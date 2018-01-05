#ifndef KINETICA_WRITER_H
#define KINETICA_WRITER_H
/*=============================================================================

   Name     : KineticaWriter.h

   System   : FME Plug-in SDK

   Language : C++

   Purpose  : Declaration of KineticaWriter

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

#include <fmewrt.h>
#include <sstream>
#include <string>
//#include "C:\Users\IEUser\Documents\gpudb-api-cpp\gpudb\GPUdb.hpp"
#include <gpudb/GPUdb.hpp>
#include <fmestring.h>

using namespace std;

// Forward declarations
class IFMEFeature;
class IFMELogFile;
class IFMEGeometryTools;
class GeometryVisitor;

// The writer ID assigned by Safe Software for this module
const FME_UInt32 kWriterId = 87062;

class KineticaWriter : public IFMEWriter
{

public:

   // Please refer to the IFMEWriter documentation for more information in
   // FME_DEV_HOME/pluginbuilder/cpp/apidoc/classIFMEWriter.html
   // -----------------------------------------------------------------------
   // Constructor
   // The writerTypeName is the name used in the following contexts:
   // - the name of the format's .db file in the formatsinfo folder
   // - the format short name for the format within the .db file
   // - FORMAT_NAME in the metafile
   // 
   // The writerKeyword is a unique identifier for this writer instance.
   // It is usually set by the WRITER_KEYWORD in the mapping file.
   // 
   // Since there can be multiple instances of a writerTypeName within
   // an FME session, the writerKeyword must be used when fetching 
   // context information from FME.
   KineticaWriter(const char* writerTypeName, const char* writerKeyword);

   // -----------------------------------------------------------------------
   // Destructor
   virtual ~KineticaWriter();

   // -----------------------------------------------------------------------
   // open()
   virtual FME_Status open(const char* datasetName, const IFMEStringArray& parameters);

   // -----------------------------------------------------------------------
   // abort()
   virtual FME_Status abort();

   // -----------------------------------------------------------------------
   // close()
   virtual FME_Status close();

   // -----------------------------------------------------------------------
   // id()
   virtual FME_UInt32 id() const { return kWriterId; }

   // -----------------------------------------------------------------------
   // write()
   virtual FME_Status write(const IFMEFeature& feature);

   // -----------------------------------------------------------------------
   // multiFileWriter()
   virtual FME_Boolean multiFileWriter() const { return FME_TRUE; }


   // -----------------------------------------------------------------------
   // Insert additional public methods here
   // -----------------------------------------------------------------------


   // Data members

   // A pointer to an IFMELogFile object that allows the plug-in to log messages
   // to the FME log file. Initialized externally after the plug-in object is created.
   static IFMELogFile* gLogFile;

   // A pointer to an IFMEMappingFile object that allows the plug-in to access
   // information from the mapping file. It is initialized externally after the
   // plug-in object is created.
   static IFMEMappingFile* gMappingFile;

   // A pointer to an IFMECoordSysManager object that allows the plug-in to retrieve
   // and define coordinate systems. It is initialized externally after the plug-in
   // object is created.
   static IFMECoordSysManager* gCoordSysMan;

   // -----------------------------------------------------------------------
   // Insert additional public data members here
   // -----------------------------------------------------------------------


private:
	std::vector<gpudb::Type::Column> gKineticaColumns;
	gpudb::Type*					 gKineticaType;
	gpudb::GPUdb*					 gKineticaConnection;
	string							 gKineticaTableName;
	string							 gKineticaCollectionName;
	string							 gKineticaURL;
	string							 gKineticaGeometryField;
	string							 gKineticaPointX;
	string							 gKineticaPointY;
	string							 gKineticaUsername;
	string							 gKineticaPassword;
	vector<gpudb::GenericRecord>     gKineticaRecordList;
	int                              numCached = 0;

	gpudb::Type::Column getKineticaColumn(FMEString attribName, FMEString attribValue);
   //---------------------------------------------------------------
   // Copy constructor
   KineticaWriter(const KineticaWriter&);

   //---------------------------------------------------------------
   // Assignment operator
   KineticaWriter &operator=(const KineticaWriter&);

   //---------------------------------------------------------------
   // Adding a DEF line to the schema
   void addDefLineToSchema(const IFMEStringArray& parameters);

   // -----------------------------------------------------------------------
   // Insert additional private methods here
   // -----------------------------------------------------------------------


   // Data members

   // The value specified for WRITER_TYPE in the mapping file. It is
   // also specified for the FORMAT_NAME in the metafile. Initialized
   // in the constructor to the value passed in by the FME.
   string writerTypeName_;

   // Typically the same as the writerTypeName_, but it could be different
   // if a value was specified for WRITER_KEYWORD in the FME mapping file.
   // Initialized in the constructor to the value passed in by the FME.
   string writerKeyword_;

   // The name of the dataset that the plug-in is writing to. Initialized
   // to an empty string in the constructor and set to the value passed in
   // by the FME in the open() method.
   string dataset_;

   // A pointer to an IFMEGeometryTools object which is used to create and
   // manipulate geometries.
   IFMEGeometryTools* fmeGeometryTools_;

   // Visitor variable.
   GeometryVisitor* visitor_;

   // Represents the schema feature on advanced writing.
   IFMEFeature* schemaFeature_;

   // -----------------------------------------------------------------------
   // Insert additional private data members here
   // -----------------------------------------------------------------------

};

#endif
