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

#include <gpudb/GPUdb.hpp>

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
	}
 //   string extraPropsStr;
	//try
	//{
	//	extraPropsStr = valueStrVector.at(1);
	//}
	//catch (const std::exception& ex)
	//{
	//	extraPropsStr = "";
	//}

//	gLogFile->logMessageString(("attrib type:" + valueStr).c_str());
//	gLogFile->logMessageString(("extraPropsStr:" + extraPropsStr).c_str());

	if (typeStr == "string")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating straight string" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
	}
	else if (typeStr == "real32")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating float" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::FLOAT;
	}
	else if (typeStr == "real64")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating double" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::DOUBLE;
	}
	else if (typeStr == "int64")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating straight long" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::LONG;
	}
	else if (typeStr == "int32")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating int" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::INT;
	}
	else if (typeStr == "int16")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating int16" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::INT;
		properties.push_back( gpudb::ColumnProperty::INT16 );
	}
	else if (typeStr == "int8")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating int8" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::INT;
		properties.push_back( gpudb::ColumnProperty::INT8);
	}
	else if (typeStr == "date")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating date" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::DATE );
	}
	else if (typeStr == "time")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating time" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::TIME );
	}
	else if (typeStr == "datetime")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating datetime" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::DATETIME );
	}
	else if (typeStr == "decimal")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating decimal" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::DECIMAL );
	}
	else if (typeStr == "IPv4")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating ipv4" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::IPV4 );
	}
	else if (typeStr == "WKT")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating wkt" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
		properties.push_back( gpudb::ColumnProperty::WKT );
	}
	else if (typeStr == "timestamp")
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating long timestamp" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::LONG;
		properties.push_back( gpudb::ColumnProperty::TIMESTAMP );
	}
	//else if (std::string(typeStr, 0, 4) == "char")
	//{
	//	colType = gpudb::Type::Column::STRING;
	//}
	//else if (typeStr == "bytes")
	//{
	//	colType = gpudb::Type::Column::BYTES;
	//}
	else
	{
        //std::cout << "KineticaWriter::getKineticaColumn() creating string (in else)" << std::endl; // debug~~~~~~~~~~
		colType = gpudb::Type::Column::STRING;
	}

	gpudb::Type::Column column = gpudb::Type::Column(columnName, colType, properties);
    //std::cout << "Column " << columnName << " is nullable? " << column.isNullable() << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~
	return(column);
}

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
   gLogFile->logMessageString( msgOpeningWriter.c_str() );


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
       std::cout << "Opening writer to table '" << gKineticaTableName << "' on Kinetica database at " << gKineticaURL << std::endl;
       gKineticaConnection = new gpudb::GPUdb(gKineticaURL, options);
   } catch (const std::exception &ex)
   {
       std::cout << "Attempt to connect to Kinetica failed: " << ex.what() << std::endl;
   }

   // Fetch all the schema features and add the DEF lines.
   KineticaWriter::addDefLineToSchema(parameters);
   
   //Get writer schema and create Kinetica type and table
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

   try
   {
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
           if ( *gKineticaType != *existing_table_type )
           {
               throw gpudb::GPUdbException( "Could not create a table of the given type; a table with the same name already exists in Kinetica, but has a different type; please either change name (" + gKineticaTableName + ") for table to be created, or delete pre-existing table." );
           }
           delete existing_table_type;
       }
   }
   catch (const std::exception& ex)
   {
	   std::cout << "Caught Exception: " << ex.what() << std::endl;
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

	try
	{
		gLogFile->logMessageString((kMsgClosingWriter + dataset_).c_str());
		if (!gKineticaRecordList.empty())
		{
			gpudb::InsertRecordsResponse response = gKineticaConnection->insertRecords(gKineticaTableName, gKineticaRecordList, options);
			gKineticaRecordList.clear();
		}

	}
	catch (const std::exception& ex)
	{
		std::cout << "Caught Exception: " << ex.what() << std::endl;
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
   gLogFile->logMessageString((kMsgClosingWriter + dataset_).c_str());

   return FME_SUCCESS;
}


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
FME_Status KineticaWriter::write(const IFMEFeature& feature)
{
    //std::cout << "KineticaWriter::write begin" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
   // -----------------------------------------------------------------------
   // The feature type and the attributes can be extracted from the feature
   // at this point.
   // -----------------------------------------------------------------------

	string coordSys_ = (const_cast<IFMEFeature&>(feature)).getCoordSys();

    // Get the attributes/columns
	FMEStringArray fmeStringArray;
    //std::cout << "KineticaWriter::write before getting attr names" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
	(const_cast<IFMEFeature&>(feature)).getAllAttributeNames( fmeStringArray );
    //std::cout << "KineticaWriter::write after getting attr names" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // get attributes and write record to Kinetica

	std::map<std::string, std::string> options;
	std::vector<gpudb::GenericRecord> record_list;

    //std::cout << "KineticaWriter::write before creating empty datum" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
        //std::cout << "KineticaWriter::write before for loop" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
        for (int i = 0; i < gKineticaColumns.size(); ++i) 
	    {
            const gpudb::Type::Column& column = gKineticaColumns[ i ];
		    string colNameStr = column.getName();
		    string colValueStr;
		    FMEString colName = FMEString( colNameStr.c_str() );
            //std::cout << "KineticaWriter::write in loop col #" << i << " name " << colNameStr << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~

		    FMEString attribValue;

		    if( (asConstPoint != NULL) && (colNameStr == gKineticaPointX) )
		    {
                //std::cout << "KineticaWriter::write seting X" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
			    FME_Real64	fmeX = asConstPoint->getX();
			    datum.setAsDouble(colNameStr, fmeX);
		    }
		    else if ((asConstPoint != NULL) && (colNameStr == gKineticaPointY))
		    {
                //std::cout << "KineticaWriter::write seting Y" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
			    FME_Real64	fmeY = asConstPoint->getY();
			    datum.setAsDouble(colNameStr, fmeY);
		    }
		    else if (colNameStr == gKineticaGeometryField)
		    {
                //std::cout << "KineticaWriter::write seting geom/wkt" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
			    const IFMEGeometry* geometry = (const_cast<IFMEFeature&>(feature)).getGeometry();
			    FMEString wkt;
			    string    wktStr;
			    (const_cast<IFMEFeature&>(feature)).exportGeometryToOGCWKT( wkt );
			    colValueStr = wkt->data();
			    FME_Status badNews = geometry->acceptGeometryVisitorConst( *visitor_ );
			    if ( badNews )
			    {
				    // There was an error in writing the geometry
				    gLogFile->logMessageString( kMsgWriteError );
				    return FME_FAILURE;
			    }
			    datum.setAsString( colNameStr, colValueStr );
		    }
		    else  // regular type; nothing special
		    {
                //std::cout << "KineticaWriter::write setting regular" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
			    (const_cast<IFMEFeature&>(feature)).getEncodedAttribute(colName, kFME_CP367, attribValue);
			    colValueStr = attribValue->data();

                //std::cout << "KineticaWriter::write before handle nulls" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
                // Handle nulls
                (const_cast<IFMEFeature&>(feature)).getAttributeNullMissingAndType( colNameStr.c_str(),
                    is_value_null, is_missing, attr_type );
                if ( is_value_null == FME_TRUE )
                {
                    // Make sure that it is a nullable column
                    if ( !column.isNullable() )
                    {
                        throw gpudb::GPUdbException( "Column '" + colNameStr + "' is not nullable, but got null value; please set the attribute/column type to be nullable (an 'Index' property)" );
                    }
                    datum.setNull( i );
                    //std::cout << "KineticaWriter::write set null; continuing" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    continue;  // skip to next column
                }

                //std::cout << "KineticaWriter::write regular; not null" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
			    switch (gKineticaColumns.at(i).getType())
			    {
				    case gpudb::Type::Column::STRING:
                    {
  					    datum.setAsString( colNameStr, colValueStr );
                        break;
                    }

				    case gpudb::Type::Column::LONG:
					    datum.setAsLong( colNameStr, std::stoll( handleEmpty( colValueStr, gpudb::Type::Column::LONG) ) ); break;

				    case gpudb::Type::Column::INT:
					    datum.setAsInt( colNameStr, std::stoi( handleEmpty( colValueStr, gpudb::Type::Column::INT) ) ); break;

				    case gpudb::Type::Column::FLOAT:
					    datum.setAsFloat( colNameStr, std::stof( handleEmpty( colValueStr, gpudb::Type::Column::FLOAT) ) ); break;

				    case gpudb::Type::Column::DOUBLE:
					    datum.setAsDouble( colNameStr, std::stod( handleEmpty( colValueStr, gpudb::Type::Column::DOUBLE) ) ); break;

				    default: gLogFile->logMessageString("Fell through to default", FME_WARN); break;
			    }  // end switch
		    }  // end if-else
	    }  // end looping over columns
	}  // end try 
    catch (const std::exception& ex)
	{
		std::cout << "Caught Exception: " << ex.what() << std::endl;
        throw ex;
	}

    //std::cout << "KineticaWriter::write after loop" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
	gKineticaRecordList.push_back( datum );
	++numCached;

	options["return_record_ids"] = "false";  // optionally request record_ids
	if (numCached == 1000)
	{
		try
		{
            //std::cout << "KineticaWriter::write before /insert/records" << std::endl; // debug~~~~~~~~~~~~~~~~~~~~~~~~~~~
			gpudb::InsertRecordsResponse response = gKineticaConnection->insertRecords(gKineticaTableName, gKineticaRecordList, options);
			numCached = 0;
			gKineticaRecordList.clear();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Caught Exception: " << ex.what() << " inserting into table:" << gKineticaTableName << std::endl;
			
		}
	}
	options.clear();

   return FME_SUCCESS;
}  // end write

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

      //std::cout << "KineticaWriter::addDefLineToSchema() adding col "
      //          << attrName.c_str() << " and type " << attrType.c_str() << std::endl; // debug~~~~~

      // Add the attribute name and type pair to the schema feature.
      schemaFeature_->setSequencedAttribute(attrName.c_str(), attrType.c_str());
   }
}
