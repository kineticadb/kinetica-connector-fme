/*=============================================================================

   Name     : KineticaWriter.cpp

   System   : FME Plug-in SDK

   Language : C++

   Purpose  : IFMEWriter method implementations

         Copyright (c) 1994 - 2016, Safe Software Inc. All rights reserved.

   Redistribution and use of this sample code in source and binary forms, with 
   or without modification, are permitted provided that the following 
   conditions are met:
   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.

   THIS SAMPLE CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SAMPLE CODE, EVEN IF 
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

// Include Files
#include "kineticawriter.h"
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

#include <igeometrytools.h>
#include <ipoint.h>
#include <iarc.h>
#include <iline.h>
#include <ipolygon.h>
#include <imultipoint.h>
#include <ipath.h>
#include <imulticurve.h>
#include <iellipse.h>
#include <idonut.h>
#include <imultiarea.h>
#include <itext.h>
#include <imultitext.h>
#include <iaggregate.h>
#include <isolid.h>
#include <isurface.h>
#include <imultisolid.h>
#include <imultisurface.h>

// System Includes
#include <fstream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <cstring>

#include "gpudb/GPUdb.hpp"

// For parsing date, time, and datetime
#include <boost/regex.hpp>

// These are initialized externally when a writer object is created so all
// methods in this file can assume they are ready to use.
IFMELogFile* KineticaWriter::gLogFile = NULL;
IFMEMappingFile* KineticaWriter::gMappingFile = NULL;
IFMECoordSysManager* KineticaWriter::gCoordSysMan = NULL;
extern IFMESession*				 gFMESession;

						 

// helper functions prototypes
void print_GetRecordsResponse(const gpudb::GetRecordsResponse<gpudb::GenericRecord>&);
//void insertRecordsLocal(gpudb::GPUdb& h_db, gpudb::Type myTypeSchema, int start, int end, double col1_const, std::string col2_const, std::string group_id_const, bool display_record_ids = false);


//===========================================================================
// Constructor
KineticaWriter::KineticaWriter(const char* writerTypeName, const char* writerKeyword)
:
   writerTypeName_(writerTypeName),
   writerKeyword_(writerKeyword),
   dataset_(""),
   fmeGeometryTools_(NULL),
   visitor_(NULL),
   schemaFeature_(NULL)
{
}

//===========================================================================
// Destructor
KineticaWriter::~KineticaWriter()
{
   close();
}



gpudb::Type::Column KineticaWriter::getKineticaColumn(FMEString attribName, FMEString attribValue)
{
	std::vector<std::string> properties;
	string valueStrOrig = attribValue->data();
	string columnName = attribName->data();
	gpudb::Type::Column::ColumnType colType;

	std::vector<std::string> valueStrVector = split(valueStrOrig, ',');
	string typeStr = valueStrVector.at(0);

    // Add any additional properties
	if ( valueStrVector.size() > 1 )
	{   // Note the +1 for the begin
		properties.assign( valueStrVector.begin() + 1, valueStrVector.end() );

        // Check nullability--unless explicitly declared to be a not-nullable
        // column, make it nullable.  Be careful about it being a primary key
        // (in which case it cannot be nullable).
        std::vector<std::string>::const_iterator not_null_keyword;
        not_null_keyword = std::find( properties.begin(), properties.end(),
                                      KINETICA_PROPERTY_NOT_NULLABLE );
        // Make the collumn nullable by default
        if ( not_null_keyword == properties.end() )
        {  // i.e. "not_nullable" NOT found
            // But, ensure it's not also a primary key
            if ( std::find( properties.begin(), properties.end(),
                            KINETICA_PROPERTY_PRIMARY_KEY ) == properties.end() )
            {  // NOT a primary key
                properties.push_back( gpudb::ColumnProperty::NULLABLE );
            }
        }
        else  // the keyword "not_nullable" was found; remove it from the
        {     // since it's not a valid Kinetica property
            // Note that we're not making this column nullable
            properties.erase( not_null_keyword );
        }   // end inner if
	}
    else
    {
        // No properties given; make the column nullable by default (since it's
        // not also a primary key, it's safe to do)
        properties.push_back( gpudb::ColumnProperty::NULLABLE );
    }   // end outer if

	if (typeStr == "string")
	{
		colType = gpudb::Type::Column::STRING;
	}
	else if (typeStr == "real32")
	{
		colType = gpudb::Type::Column::FLOAT;
	}
	else if (typeStr == "real64")
	{
		colType = gpudb::Type::Column::DOUBLE;
	}
	else if (typeStr == "int64")
	{
		colType = gpudb::Type::Column::LONG;
	}
	else if (typeStr == "int32")
	{
		colType = gpudb::Type::Column::INT;
	}
	else if (typeStr == "int16")
	{
		colType = gpudb::Type::Column::INT;
		properties.push_back( gpudb::ColumnProperty::INT16 );
	}
	else if (typeStr == "int8")
	{
		colType = gpudb::Type::Column::INT;
		properties.push_back( gpudb::ColumnProperty::INT8);
	}
	else if (typeStr == "date")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::DATE );
	}
	else if (typeStr == "time")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::TIME );
	}
	else if (typeStr == "datetime")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::DATETIME );
	}
	else if (typeStr == "decimal")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::DECIMAL );
	}
	else if (typeStr == "IPv4")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::IPV4 );
	}
	else if (typeStr == "WKT")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::WKT );
	}
	else if (typeStr == "timestamp")
	{
		colType = gpudb::Type::Column::LONG;
		properties.push_back( gpudb::ColumnProperty::TIMESTAMP );
	}
	else if (typeStr == "char1")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR1 );
	}
	else if (typeStr == "char2")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR2 );
	}
	else if (typeStr == "char4")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR4 );
	}
	else if (typeStr == "char8")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR8 );
	}
	else if (typeStr == "char16")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR16 );
	}
	else if (typeStr == "char32")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR32 );
	}
	else if (typeStr == "char64")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR64 );
	}
	else if (typeStr == "char128")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR128 );
	}
	else if (typeStr == "char256")
	{
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::CHAR256 );
	}
	else if (typeStr == "bytes")
	{
		colType = gpudb::Type::Column::BYTES;
	}
	else
	{
		throw std::runtime_error( "Unknown column type: " + typeStr );
	}

	gpudb::Type::Column column = gpudb::Type::Column(columnName, colType, properties);
	return(column);
}   // end getKineticaColumn


//===========================================================================
// Open
FME_Status KineticaWriter::open(const char* datasetName, const IFMEStringArray& parameters)
{
   // Perform setup steps before opening file for writing

   // Get geometry tools
   fmeGeometryTools_ = gFMESession->getGeometryTools();


   // Set the session on FMEString[Array]s.  The session set here was retrieved
   // when this plug-in was loaded.  Consequently, subsequent calls to this
   // method will use the same session, so we can make multiple calls to
   // setSession without worrying.
   FMEString::setSession(gFMESession);
   FMEStringArray::setSession(gFMESession);

   // Create visitor to visit feature geometries
   visitor_ = new GeometryVisitor(fmeGeometryTools_, gFMESession);

   dataset_ = datasetName;

   schemaFeature_ = gFMESession->createFeature();

   // -----------------------------------------------------------------------
   // Add additional setup here
   // -----------------------------------------------------------------------

   // Log an opening writer message
   string msgOpeningWriter = kMsgOpeningWriter + dataset_;
   LOG_KINETICA_INFO( gLogFile, msgOpeningWriter );


   // Read the settings box and log
   FMEStringArray connectionInfo;
   getConnectionInfo(gMappingFile, gLogFile, connectionInfo, writerKeyword_, writerTypeName_);
   gKineticaURL				= connectionInfo->elementAt(0)->data();
   gKineticaTableName		= connectionInfo->elementAt(1)->data();
   gKineticaCollectionName	= connectionInfo->elementAt(3)->data();
   gKineticaGeometryField	= connectionInfo->elementAt(4)->data();
   gKineticaUsername		= connectionInfo->elementAt(5)->data();
   gKineticaPassword		= connectionInfo->elementAt(6)->data();
   gKineticaPointX			= connectionInfo->elementAt(7)->data();
   gKineticaPointY			= connectionInfo->elementAt(8)->data();

   gpudb::GPUdb::Options options;
   if (gKineticaUsername.size() > 0)
   {
	   options.setUsername(gKineticaUsername);
	   options.setPassword(gKineticaPassword);
   }


   try
   {
       LOG_KINETICA_INFO( gLogFile, "Opening writer to table '" << gKineticaTableName
                                     << "' on Kinetica database at " << gKineticaURL );
       gKineticaConnection = new gpudb::GPUdb(gKineticaURL, options);
   } catch (const std::exception &ex)
   {
       LOG_KINETICA_WRITER_ERROR( gLogFile, "Attempt to connect to Kinetica failed: " << ex.what() );
       return FME_FAILURE;
   }

   // Fetch all the schema features and add the DEF lines.
   KineticaWriter::addDefLineToSchema(parameters);
   
   // Get the writer schema and create Kinetica type and table
   try
   {
       FMEStringArray attribNames;
       schemaFeature_->getAllAttributeNames(attribNames);
       for (int i = 0; i < attribNames->entries(); ++i)
       {
           FMEString attribName;
           attribNames->getElement( i, attribName );

           FMEString attribValue;
           schemaFeature_->getEncodedAttribute( attribName, kFME_CP367, attribValue );
           gKineticaColumns.push_back( getKineticaColumn( attribName, attribValue ) );
       }


       if ( (gKineticaGeometryField.size() > 0) && (gKineticaGeometryField != "<Unused>") )
       {
           FMEString geometryField(gKineticaGeometryField.c_str());
           FMEString attribValue("string");
           gKineticaColumns.push_back(getKineticaColumn(geometryField, attribValue));
       }


       if ((gKineticaPointX.size() > 0) && (gKineticaPointX != "<Unused>"))
       {
           FMEString pointXField(gKineticaPointX.c_str());
           FMEString pointYField(gKineticaPointY.c_str());
           FMEString attribValue("real64");
           gKineticaColumns.push_back(getKineticaColumn(pointXField, attribValue));
           gKineticaColumns.push_back(getKineticaColumn(pointYField, attribValue));
       }

       gKineticaType = new gpudb::Type("kinetica_type", gKineticaColumns);
	   std::string type_id = gKineticaType->create( *gKineticaConnection );

	   std::map<std::string, std::string> options;
	   if (gKineticaCollectionName.size() > 0)
	   {
		   options.insert(std::make_pair("collection_name", gKineticaCollectionName));
	   }
       try
       {
	       gpudb::CreateTableResponse response = gKineticaConnection->createTable(gKineticaTableName, type_id, options);
       }
       catch ( const std::exception &ex1 )
       {
           gpudb::Type* existing_table_type;
           // Could not create a table; see if one exists but is of the same type
           try
           {
               existing_table_type = new gpudb::Type( gpudb::Type::fromTable( *gKineticaConnection, gKineticaTableName ) );
           } catch ( const std::exception &ex2 )
           {  // Table does not exist, or is a heterogeneous collection
               throw gpudb::GPUdbException( "Could not create a table of the given type." );
           }
           // Check if the two types match
           if ( gKineticaType->isTypeCompatible( *existing_table_type ) )
           {
               throw gpudb::GPUdbException( "Could not create a table of the given type; a table with the same name already exists in Kinetica, but has a different type; please either change name (" + gKineticaTableName + ") for table to be created, or delete pre-existing table." );
           }
           delete existing_table_type;
       }
   }
   catch (const std::exception& ex)
   {
	   LOG_KINETICA_WRITER_ERROR( gLogFile, ex.what() );
       return FME_FAILURE;
   }

   return FME_SUCCESS;
}  // end open



//===========================================================================
// Abort
FME_Status KineticaWriter::abort()
{
   // -----------------------------------------------------------------------
   // Add any special actions to shut down a writer not finished writing
   // data. For example, if your format requires a footer at the end of a
   // file, write it here.
   // -----------------------------------------------------------------------

   close();
   return FME_SUCCESS;
}

//===========================================================================
// Close
FME_Status KineticaWriter::close()
{
   // -----------------------------------------------------------------------
   // Perform any closing operations / cleanup here; e.g. close opened files
   // -----------------------------------------------------------------------
	std::map<std::string, std::string> options;
	options["return_record_ids"] = "false";  // optionally request record_ids

    bool failure = false;
	try
	{
        LOG_KINETICA_INFO( gLogFile, kMsgClosingWriter << dataset_ );
		if (!gKineticaRecordList.empty())
		{
			gpudb::InsertRecordsResponse response = gKineticaConnection->insertRecords(gKineticaTableName, gKineticaRecordList, options);
            numInserted += gKineticaRecordList.size();
			gKineticaRecordList.clear();
		}
        LOG_KINETICA_INFO( gLogFile, "A total of " << numInserted << " records have been inserted." );
	}
	catch (const std::exception& ex)
	{
        LOG_KINETICA_WRITER_ERROR( gLogFile, "Probelm during insertion of the "
            << KINETICA_GET_ORDINAL_NUMBER(numInserted) << " to "
            << KINETICA_GET_ORDINAL_NUMBER(numInserted + numCached)
            << " records into table '"
            << gKineticaTableName << "': " << ex.what() );
//        LOG_KINETICA_WRITER_ERROR( gLogFile, ex.what() );
        failure = true;
	}

   // Delete the visitor
   if (visitor_)
   {
      delete visitor_;
   }
   visitor_ = NULL;

   if (schemaFeature_)
   {
      gFMESession->destroyFeature(schemaFeature_);
      schemaFeature_ = NULL;
   }

   // Log that the writer is done
   LOG_KINETICA_INFO( gLogFile, kMsgClosingWriter << dataset_ );

   // Return whether we succeeded or not
   if (failure)
       return FME_FAILURE;
   return FME_SUCCESS;
}  // end close()


string handleEmpty(string value, gpudb::Type::Column::ColumnType colType) 
{
	string retVal;
	if ( !value.empty() )
	{
		retVal = value;
	}
	else if (colType == gpudb::Type::Column::STRING)
	{
		retVal = "";
	}
	else
	{
		retVal = "0";
	}
	return( retVal );
}

//===========================================================================
// Write

// Regular expressions needed for parsing FME's datetime formats
// Date: YYYYmmdd
static boost::regex fme_date_regex( "\\A(\\d{4})(\\d{2})(\\d{2})$" );
// Time: HHMMSS[.000]
static boost::regex fme_time_regex( "\\A(\\d{2})(\\d{2})(\\d{2})(\\.(\\d{1,3}))?$" );
// DateTime: YYYY-MM-DD[ HH:MM:SS[.ffffff]]
// Note - allow up to 6 decimal digits (but we ignore the last 3)
static boost::regex fme_datetime_regex( "\\A(\\d{4})(\\d{2})(\\d{2})(?:(\\d{2})(\\d{2})(\\d{2})(?:\\.(\\d{1,6}))?)?$" );



FME_Status KineticaWriter::write(const IFMEFeature& feature)
{
   // -----------------------------------------------------------------------
   // The feature type and the attributes can be extracted from the feature
   // at this point.
   // -----------------------------------------------------------------------

	string coordSys_ = (const_cast<IFMEFeature&>(feature)).getCoordSys();

    // Get the attributes/columns
	FMEStringArray fmeStringArray;
	(const_cast<IFMEFeature&>(feature)).getAllAttributeNames( fmeStringArray );

    // get attributes and write record to Kinetica

	std::map<std::string, std::string> options;
	std::vector<gpudb::GenericRecord> record_list;

	gpudb::GenericRecord datum( *gKineticaType );

    const IFMEGeometry* geometry = (const_cast<IFMEFeature&>(feature)).getGeometry();
	const IFMEPoint* asConstPoint = NULL;
	if (geometry->canCastAs<IFMEPoint*>())
	{
		asConstPoint = geometry->castAs<const IFMEPoint*>();
	}

    FME_Boolean is_value_null;
    FME_Boolean is_missing;
    FME_AttributeType attr_type;

	try
	{
        for (int i = 0; i < gKineticaColumns.size(); ++i) 
	    {
            const gpudb::Type::Column& column = gKineticaColumns[ i ];
		    string colNameStr = column.getName();
		    string colValueStr;
		    FMEString colName = FMEString( colNameStr.c_str() );

		    FMEString attribValue;

		    if( (asConstPoint != NULL) && (colNameStr == gKineticaPointX) )
		    {
			    FME_Real64	fmeX = asConstPoint->getX();
			    datum.setAsDouble(colNameStr, fmeX);
		    }
		    else if ((asConstPoint != NULL) && (colNameStr == gKineticaPointY))
		    {
			    FME_Real64	fmeY = asConstPoint->getY();
			    datum.setAsDouble(colNameStr, fmeY);
		    }
		    else if (colNameStr == gKineticaGeometryField)
		    {
			    const IFMEGeometry* geometry = (const_cast<IFMEFeature&>(feature)).getGeometry();
			    FMEString wkt;
			    string    wktStr;
			    (const_cast<IFMEFeature&>(feature)).exportGeometryToOGCWKT( wkt );
			    colValueStr = wkt->data();
			    FME_Status badNews = geometry->acceptGeometryVisitorConst( *visitor_ );
			    if ( badNews )
			    {
				    // There was an error in writing the geometry
                    throw std::runtime_error( kMsgWriteError );
			    }
			    datum.setAsString( colNameStr, colValueStr );
		    }
		    else  // regular type; nothing special
		    {
			    (const_cast<IFMEFeature&>(feature)).getEncodedAttribute(colName, kFME_CP367, attribValue);
			    colValueStr = attribValue->data();

                // Handle nulls
                (const_cast<IFMEFeature&>(feature)).getAttributeNullMissingAndType( colNameStr.c_str(),
                    is_value_null, is_missing, attr_type );
                if ( is_value_null == FME_TRUE )
                {
                    // Make sure that it is a nullable column
                    if ( !column.isNullable() )
                    {
                        throw std::runtime_error( "Column '" + colNameStr
                                                   + "' is not nullable, but got null "
                                                   + "value; please *deselect* the "
                                                   + "'not_nullable' index property for "
                                                   + "the attribute, or ensure that it "
                                                   + "is not a primary key." );
                    }
                    datum.setNull( i );
                    continue;  // skip to next column
                }

                const gpudb::Type::Column column = gKineticaColumns.at(i);
			    switch ( column.getType() )
			    {
				    case gpudb::Type::Column::STRING:
                    {
                        // Does the given value match any special FME format ?
                        boost::cmatch matches;
                        // Casting to char* because we may need to change some values
                        char* colValCStr = (char*)(colValueStr.c_str());
                        //const char* colValCStr = colValueStr.c_str();
                        if ( column.hasProperty( gpudb::ColumnProperty::DATE ) )
                        {   // DATE property
                            // Fix years < 1000 or > 2900
                            fixOutOfBoundsDate( gLogFile, colValCStr );

                            // Check if the string is in the fme_date format;
                            if ( boost::regex_match( colValCStr, matches, fme_date_regex ) )
                            {   // Yes, it is; need to convert it to the Kinetica format
                                std::string converted_value;
                                converted_value.reserve( 11 ); // 'YYYY-MM-DD' 10 digits
                                converted_value.append( colValCStr, 4 ); // YYYY
                                converted_value.append( "-" );
                                converted_value.append( (colValCStr + 4), 2 ); // MM
                                converted_value.append( "-" );
                                converted_value.append( (colValCStr + 6), 2 ); // DD

                                colValueStr = converted_value;
                            }
                        }  // end date
                        else if ( column.hasProperty( gpudb::ColumnProperty::DATETIME ) )
                        {   // DATETIME property
                            // Fix years < 1000 or > 2900
                            fixOutOfBoundsDatetime( gLogFile, colValCStr );

                            // Check if the string is in the fme_datetime format
                            if ( boost::regex_match( colValCStr, matches, fme_datetime_regex ) )
                            {   // Yes, it is; need to convert it to the Kinetica format
                                std::string converted_value;
                                converted_value.reserve( 24 ); // 'YYYY-mm-dd HH:MM:SS[.000]' 23 digits
                                converted_value.append( colValCStr, 4 ); // YYYY
                                converted_value.append( "-" );
                                converted_value.append( (colValCStr + 4), 2 ); // MM
                                converted_value.append( "-" );
                                converted_value.append( (colValCStr + 6), 2 ); // DD

                                // The time part is optional
                                boost::csub_match hour_group  = matches[ 4 ];
                                boost::csub_match min_group   = matches[ 5 ];
                                boost::csub_match sec_group   = matches[ 6 ];
                                boost::csub_match msec_group  = matches[ 7 ];
                                if ( hour_group.matched )
                                {
                                    converted_value.append( " " );
                                    converted_value.append( hour_group ); // HH
                                    converted_value.append( ":" );
                                    converted_value.append( min_group ); // MM
                                    converted_value.append( ":" );
                                    converted_value.append( sec_group ); // SS
                                    if ( msec_group.matched )
                                    {   // the ms part is optional
                                        // Will only take the first three digits
                                        converted_value.append( "." );
                                        converted_value.append( min_group, 3 ); // fff
                                    }
                                }

                                colValueStr = converted_value;
                            }
                        }   // end datetime
                        else if ( column.hasProperty( gpudb::ColumnProperty::TIME ) )
                        {   // TIME property
                            if ( boost::regex_match( colValCStr, matches, fme_time_regex ) )
                            {   // The string is in the fme_time format;
                                // need to convert it to the Kinetica format
                                std::string converted_value;
                                converted_value.reserve( 13 ); // 'HH:MM:SS[.000]' 12 digits
                                converted_value.append( colValCStr, 2 ); // HH
                                converted_value.append( ":" );
                                converted_value.append( (colValCStr + 2), 2 ); // MM
                                converted_value.append( ":" );
                                converted_value.append( (colValCStr + 4), 2 ); // SS

                                // Does the optional millisecond exist?
                                boost::csub_match ms_group  = matches[ 5 ];
                                if ( ms_group.matched )
                                {
                                    converted_value.append( "." );
                                    converted_value.append( ms_group );
                                }
                                colValueStr = converted_value;
                            } // end time
                        }  // end FME->Kinetica format conversion

                        // Actually save the value of the string
                        datum.setAsString( colNameStr, colValueStr );
                        break;
                    }

				    case gpudb::Type::Column::LONG:
                    {
                        if ( column.hasProperty( gpudb::ColumnProperty::TIMESTAMP ) )
                        {
                            colValueStr = fixOutOfBoundsTimestamp( gLogFile, colValueStr );
                        }
					    datum.setAsLong( colNameStr, std::stoll( handleEmpty( colValueStr, gpudb::Type::Column::LONG) ) ); break;
                    }

				    case gpudb::Type::Column::INT:
					    datum.setAsInt( colNameStr, std::stoi( handleEmpty( colValueStr, gpudb::Type::Column::INT) ) ); break;

				    case gpudb::Type::Column::FLOAT:
					    datum.setAsFloat( colNameStr, std::stof( handleEmpty( colValueStr, gpudb::Type::Column::FLOAT) ) ); break;

				    case gpudb::Type::Column::DOUBLE:
					    datum.setAsDouble( colNameStr, std::stod( handleEmpty( colValueStr, gpudb::Type::Column::DOUBLE) ) ); break;

				    default:
                        throw std::runtime_error( "Unknown column type." );
			    }  // end switch
		    }  // end if-else
	    }  // end looping over columns
	}  // end try 
    catch (const std::exception& ex)
	{
        LOG_KINETICA_WRITER_ERROR( gLogFile, "Problem during processing the "
                                  << KINETICA_GET_ORDINAL_NUMBER(numInserted + numCached) << " record: "
                                  << ex.what() );
        return FME_FAILURE;
	}

	gKineticaRecordList.push_back( datum );
	++numCached;

	options["return_record_ids"] = "false";  // optionally request record_ids
	if (numCached == 1000)
	{   // Insert one thousand records at a time
		try
		{
			gpudb::InsertRecordsResponse response = gKineticaConnection->insertRecords(gKineticaTableName, gKineticaRecordList, options);
			gKineticaRecordList.clear();
            numInserted += numCached;
            numCached = 0;
        }
		catch (const std::exception& ex)
		{
            LOG_KINETICA_WRITER_ERROR( gLogFile, "Probelm during insertion of the "
                                       << KINETICA_GET_ORDINAL_NUMBER(numInserted) << " to "
                                       << KINETICA_GET_ORDINAL_NUMBER(numInserted + numCached)
                                       << " records into table '"
                                       << gKineticaTableName << "': " << ex.what() );
			return FME_FAILURE;
		}
	}
	options.clear();

   return FME_SUCCESS;
}  // end write()

//===========================================================================
// Add DEF Line to the Schema Feature
void KineticaWriter::addDefLineToSchema(const IFMEStringArray& parameters)
{
   // Get the feature type.
   const IFMEString* paramValue;

   paramValue = parameters.elementAt( 0 );

   // Set it on the schema feature.
   schemaFeature_->setFeatureType( paramValue->data() );

   string attrName;
   string attrType;
   for (FME_UInt32 i = 1; i < parameters.entries(); i += 2)
   {
      // Grab the attribute name and type
      paramValue = parameters.elementAt(i);
      attrName = paramValue->data();

      paramValue = parameters.elementAt(i + 1);
      attrType = paramValue->data();

      // Add the attribute name and type pair to the schema feature.
      schemaFeature_->setSequencedAttribute(attrName.c_str(), attrType.c_str());
   }
}
