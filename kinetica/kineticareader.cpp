/*=============================================================================

   Name     : kineticareader.cpp

   System   : FME Plug-in SDK

   Language : C++

   Purpose  : IFMEReader method implementations

         Copyright (c) 1994 - 2016, Safe Software Inc. All rights reserved.

=============================================================================*/

// Include Files
#include "kineticareader.h"
#include "kineticapriv.h"
#include "geometryvisitor.h"
#include "kineticautil.h"


// FME Plug-in SDK includes
#include <ifeature.h>
#include <igeometry.h>
#include <fmestring.h>
#include <fmevalueencodings.h>
#include <fmemap.h>
#include <fmefeat.h>
#include <ilogfile.h>
#include <isession.h>
#include <ifmeobjserv.h>
#include <servmgr.h>
#include <icrdsysmgr.h>

#include <fmestring.h>
#include <igeometrytools.h>
#include <ilogfile.h>
#include <fmemap.h>
#include <isession.h>
#include <gpudb/GPUdb.hpp>

#include <gpudb/protocol/show_table.h>

// These are initialized externally when a reader object is created so all
// methods in this file can assume they are ready to use.
IFMELogFile* KineticaReader::gLogFile             = NULL;
IFMEMappingFile* KineticaReader::gMappingFile     = NULL;
IFMECoordSysManager* KineticaReader::gCoordSysMan = NULL;
IFMESession* gFMESession                          = NULL;
int gNumFeatures = 0;



//===========================================================================
// Constructor
KineticaReader::KineticaReader(const char* readerTypeName, const char* readerKeyword)
:
   readerTypeName_(readerTypeName),
   readerKeyword_(readerKeyword),
   dataset_(""),
   coordSys_(""),
   fmeGeometryTools_(NULL)
{
}

//===========================================================================
// Destructor
KineticaReader::~KineticaReader()
{
   close();

   // Release resources
   if (gKineticaConnection != NULL)
   {
	   delete gKineticaConnection;
   }

   if (gTypeSchema != NULL)
   {
	   delete gTypeSchema;
   }
}


//===========================================================================
// Open
FME_Status KineticaReader::open(const char* datasetName, const IFMEStringArray& parameters)
{
   // Get geometry tools
   fmeGeometryTools_ = gFMESession->getGeometryTools();
   FMEString::setSession(gFMESession);
   FMEStringArray::setSession(gFMESession);

   dataset_ = datasetName;

   // -----------------------------------------------------------------------
   // Add additional setup here
   // -----------------------------------------------------------------------

   // Log an opening reader message
   LOG_KINETICA_INFO( gLogFile, kMsgOpeningReader << dataset_ );

   FMEStringArray connectionInfo;
   getConnectionInfo(gMappingFile, gLogFile, connectionInfo, readerKeyword_, readerTypeName_);

   // -----------------------------------------------------------------------
   // Open the dataset here, e.g. inputFile.open(dataSetName, ios::in);
   // -----------------------------------------------------------------------
   LOG_KINETICA_INFO( gLogFile, gKineticaURL );
   // Read the mapping file parameters if there is one specified.

   gKineticaURL        = trim(connectionInfo->elementAt(0)->data());
   gKineticaTableName  = trim(connectionInfo->elementAt(2)->data());
   gKineticaExpression = connectionInfo->elementAt(9)->data();

   try
   {
       LOG_KINETICA_INFO( gLogFile, "Opening reader to table '" << gKineticaTableName << "' on Kinetica database at " << gKineticaURL );
       gKineticaConnection = new gpudb::GPUdb( gKineticaURL );

       // Get the table's record type information
       std::map<std::string, std::string> options;
       gpudb::ShowTableResponse showTableResp = gKineticaConnection->showTable(gKineticaTableName, options);
       if (gSchemaColumnNames.size() == 0)
       {
	       // Don't forget to release the resource!
	       gTypeSchema = new gpudb::Type( showTableResp.typeSchemas.at(0) );

	       for (int i = 0; i < gTypeSchema->getColumnCount(); ++i)
	       {
		       string colName = gTypeSchema->getColumn(i).getName();
		       gpudb::Type::Column::ColumnType colType = gTypeSchema->getColumn(i).getType();
		       gSchemaColumnNames.push_back( colName );
		       gSchemaColumnTypes.push_back( colType );
               // TODO: This is where properties would be set
	       }
       }
   } catch ( const std::exception &ex )
   {
       LOG_KINETICA_ERROR( gLogFile, "Failed to open a reader for table '"
                           << gKineticaTableName << "' at '" << gKineticaURL
                           << "': " << ex.what() );
       return FME_FAILURE;
   }
   // We'll read in batches of 500 objects
   gBlockSize  = 500;

   return FME_SUCCESS;
}  // end open



//===========================================================================
// Abort
FME_Status KineticaReader::abort()
{
   // -----------------------------------------------------------------------
   // Add any special actions to shut down a reader not finished reading
   // data; e.g. log a message or send an email. 
   // -----------------------------------------------------------------------

   close();
   return FME_SUCCESS;
}

//===========================================================================
// Close
FME_Status KineticaReader::close()
{
   // -----------------------------------------------------------------------
   // Perform any closing operations / cleanup here; e.g. close opened files
   // -----------------------------------------------------------------------
   LOG_KINETICA_INFO( gLogFile, "Have read " << gNumRead << "records." );
   // Log that the reader is done
   LOG_KINETICA_INFO( gLogFile, kMsgClosingReader << dataset_ );
   return FME_SUCCESS;
}

//===========================================================================
// Read
FME_Status KineticaReader::read(IFMEFeature& feature, FME_Boolean& endOfFile)
{
    bool failed = false;

	feature.setFeatureType( gKineticaTableName.c_str() );

    // We are either at the very first read, or have read all the records
    // returned in the previous /get/records call; so must make another
    // /get/records call to get more data
	if (gIndexInBlock == 0)
	{
		std::map<std::string, std::string> options;
		if ((gKineticaExpression.size() > 0) && (gKineticaExpression != "<Unused>"))
		{
			options.insert(std::make_pair("expression", gKineticaExpression));
		}

        try
        {
		    gRecordsResponse = gKineticaConnection->getRecords<gpudb::GenericRecord>(*gTypeSchema, gKineticaTableName, gOffset, gBlockSize, options);
		    gOffset += gBlockSize;

            // Save how many records are in this batch
            g_num_records_in_batch = gRecordsResponse.totalNumberOfRecords;

            // Have we reached the end of the table?
            g_table_has_more_records = gRecordsResponse.hasMoreRecords;
        }
        catch ( const gpudb::GPUdbException &ex )
        {
            LOG_KINETICA_READER_ERROR( gLogFile, "Failed to read the next batch of "
                                       << gBlockSize << " records: " << ex.what() );
            failed = true;
        }
	}

	if( gNumRead < g_num_records_in_batch )
	{
		gpudb::GenericRecord record = gRecordsResponse.data[ gIndexInBlock ];

        try
        {
            // Set the values for all columns for this record/feature
		    for (int i = 0; i < gSchemaColumnNames.size(); ++i )
		    {
			    string columnNameStr = gSchemaColumnNames.at(i);

                // Get the column for subtype identification
                const gpudb::Type::Column& column = gTypeSchema->getColumn( i );

			    string valueStr;
			    switch ( gSchemaColumnTypes.at( i ) )
			    {
				    //case (gpudb::Type::Column::ColumnType::BYTES):  vector<uint8_t> bytes = record.asBytes(i); valueStr = ""; break;
				    case gpudb::Type::Column::ColumnType::DOUBLE:
                    {
                        if ( record.isNull( i ) )  // it's a null value
                            feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_REAL64 );
                        else // regular value
                        {
                            valueStr = std::to_string( record.getAsDouble( i ) );
                            feature.setAttribute( columnNameStr.c_str(), valueStr.c_str() );
                        }
                        break;
                    }
				    case gpudb::Type::Column::ColumnType::FLOAT:
                    {
                        if ( record.isNull( i ) )  // it's a null value
                            feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_REAL32 );
                        else // regular value
                        {
                            valueStr = std::to_string( record.getAsFloat( i ) );
                            feature.setAttribute( columnNameStr.c_str(), valueStr.c_str() );
                        }
                        break;
                    }
				    case gpudb::Type::Column::ColumnType::INT:
                    {
                        if ( record.isNull( i ) )  // it's a null value
                        {
                            if ( column.hasProperty( gpudb::ColumnProperty::INT8 ) )
                                feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_INT8 );
                            else if ( column.hasProperty( gpudb::ColumnProperty::INT8 ) )
                                feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_INT16 );
                            else
                                feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_INT32 );
                        }
                        else // regular value
                        {
                            valueStr = std::to_string( record.getAsInt( i ) );
                            feature.setAttribute( columnNameStr.c_str(), valueStr.c_str() );
                        }
                        break;
                    }
				    case gpudb::Type::Column::ColumnType::LONG:
                    {
                        if ( record.isNull( i ) )  // it's a null value
                            feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_INT64 );
                        else // regular value
                        {
                            valueStr = std::to_string( record.getAsLong( i ) );
                            feature.setAttribute( columnNameStr.c_str(), valueStr.c_str() );
                        }
                        break;
                    }
				    case gpudb::Type::Column::ColumnType::STRING:
                    {
                        if ( record.isNull( i ) )  // it's a null value
                            feature.setAttributeNullWithType( columnNameStr.c_str(), FME_ATTR_STRING );
                        else // regular value
                        {
                            valueStr = record.getAsString( i );
                            feature.setAttribute( columnNameStr.c_str(), valueStr.c_str() );
                        }
                        break;
                    }
			    }  // end switch
		    }
        } catch (const std::exception &ex )
        {   // Log the exception message
            LOG_KINETICA_READER_ERROR( gLogFile,
                                       "On reading the " << KINETICA_GET_ORDINAL_NUMBER( gNumRead )
                                       << " record since the start, got error from the Kinetica server: "
                                       << ex.what() );
            failed = true;
        }

        // Update the fact that we've just read another record from the most recent
        // /get/records call response
		++gNumRead;
		++gIndexInBlock;

        endOfFile = FME_FALSE;

        // We just read the last record in the batch
        if (gIndexInBlock == gBlockSize)
		{
			gIndexInBlock = 0;

            // Mark the end of the table, if applicable
            if ( !g_table_has_more_records )
            {
		        endOfFile = FME_TRUE;
            }
		}
	}
	else
    {
		endOfFile = FME_TRUE;
	}

    // Return whether we succeeded or failed
    if (failed)
        return FME_FAILURE;
    return FME_SUCCESS;
}  // end read()


//===========================================================================
// readSchema
FME_Status KineticaReader::readSchema(IFMEFeature& feature, FME_Boolean& endOfSchema)
{
    LOG_KINETICA_INFO( gLogFile, "in readSchema" );
	FMEStringArray connectionInfo;
	getConnectionInfo(gMappingFile, gLogFile, connectionInfo, readerKeyword_, readerTypeName_);
	readParametersDialog();
	gKineticaURL = connectionInfo->elementAt(0)->data();
	gKineticaTableName = trim(connectionInfo->elementAt(2)->data());
	gKineticaConnection = new gpudb::GPUdb(gKineticaURL);

	if (gNumFeatures == 0)
	{
		std::map<std::string, std::string> options;
		gpudb::ShowTableResponse showTableResp = gKineticaConnection->showTable(gKineticaTableName, options);
		gpudb::Type typeSchema = showTableResp.typeSchemas.at(0);
		feature.setFeatureType( gKineticaTableName.c_str() );
		//feature.setFeatureType("KineticaFeatureType");

        size_t num_columns = typeSchema.getColumnCount();
		for (size_t i = 0; i < num_columns; ++i)
        {
            gpudb::Type::Column column = gTypeSchema->getColumn( i );
			string colName = column.getName();
			gpudb::Type::Column::ColumnType colType = column.getType();
			string typeStr;
			switch ( colType )
			{
				//case (gpudb::Type::Column::ColumnType::BYTES):  vector<uint8_t> bytes = record.asBytes(i); valueStr = ""; break;
				case (gpudb::Type::Column::ColumnType::DOUBLE): typeStr = "real64";  break;
				case (gpudb::Type::Column::ColumnType::FLOAT):  typeStr = "real32"; break;

                // Int can be 8, 16, or 32 bytes long
                case (gpudb::Type::Column::ColumnType::INT):
				{
                    if ( column.hasProperty( gpudb::ColumnProperty::INT8 ) )
                        typeStr = "int8";
                    else if ( column.hasProperty( gpudb::ColumnProperty::INT16 ) )
                        typeStr = "int16";
                    else
    					typeStr = "int32";
                    break;
				}

                // Long could be a regular long or a timestamp
				case (gpudb::Type::Column::ColumnType::LONG):
				{
                    if ( column.hasProperty( gpudb::ColumnProperty::TIMESTAMP ) )
                        typeStr = "timestamp";
                    else
    					typeStr = "int64";
                    break;
				}

                // String has many properties associated with it
				case (gpudb::Type::Column::ColumnType::STRING):
				{
                    if ( column.hasProperty( gpudb::ColumnProperty::CHAR1 ) )
                    {
                        typeStr = "char1";
                    }
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR2 ) )
                        typeStr = "char2";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR4 ) )
                        typeStr = "char4";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR8 ) )
                        typeStr = "char8";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR16 ) )
                        typeStr = "char16";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR32 ) )
                        typeStr = "char32";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR64 ) )
                        typeStr = "char64";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR128 ) )
                        typeStr = "char128";
                    else if ( column.hasProperty( gpudb::ColumnProperty::CHAR256 ) )
                        typeStr = "char256";
                    else if ( column.hasProperty( gpudb::ColumnProperty::DATE ) )
                        typeStr = "date";
                    else if ( column.hasProperty( gpudb::ColumnProperty::TIME ) )
                        typeStr = "time";
                    else if ( column.hasProperty( gpudb::ColumnProperty::DATETIME ) )
                        typeStr = "datetime";
                    else if ( column.hasProperty( gpudb::ColumnProperty::DECIMAL ) )
                        typeStr = "decimal";
                    else if ( column.hasProperty( gpudb::ColumnProperty::IPV4 ) )
                        typeStr = "IPv4";
                    else if ( column.hasProperty( gpudb::ColumnProperty::WKT ) )
                        typeStr = "WKT";
                    else
    					typeStr = "string";
					break;
				}
			}

			feature.setSequencedAttribute(colName.c_str(), typeStr.c_str());
			gSchemaColumnNames.push_back(colName);
			gSchemaColumnTypes.push_back(colType);
		}
		feature.setAttribute("fme_geometry{0}", "kinetica_point");
		endOfSchema = FME_FALSE;
	}
	else {
		endOfSchema = FME_TRUE;
	}
	gNumFeatures++;
	return FME_SUCCESS;	
}   // end readSchema()

//===========================================================================
// readParameterDialog

void KineticaReader::readParametersDialog()
{
   FMEString paramValue;
   if (gMappingFile->fetchWithPrefix(readerKeyword_.c_str(), readerTypeName_.c_str(), kSrcMyFormatParamTag, *paramValue))
   {
      // A parameter value has been found, so set the values.
      myFormatParameter_ = paramValue->data();

      // Let's log to the user that a parameter value has been specified.
      string paramMsg = (kMyFormatParamTag + myFormatParameter_).c_str();
      LOG_KINETICA_WARN( gLogFile, paramMsg );
   }
   else
   {
      // Log that no parameter value was entered
      LOG_KINETICA_WARN( gLogFile, kMsgNoMyFormatParam );
   }
}
