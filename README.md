# README #

This is a Safe FME Reader/Writer plugin for Kinetica.  To download it, please
visit [the github repository](https://github.com/kineticadb/kinetica-connector-fme).  For documentation on
this plug-in, please refer to [the FME documentation](https://www.kinetica.com/docs/connectors/fme_kinetica_format.html) page.


## Requirements ##

* The FME Desktop with the pluginbuilder directory
* The Kinetica C++ API, compiled with Visual Studio 2015 build tools
	* To build Kinetica C++ API, avro-cpp-1.8.1 and `boost_1_59_0` are required.
	   * A copy of the 32-bit C++ API library exists at
	     <FME-CONN-DIR>/runtimeFiles-windows, where <FME-CONN-DIR> is usually at
         <FME-HOME-DIR>/pluginbuilder/gpudb-connector-fme.  The pair of files
         gpudbapilib.lib and gpudbapilib.pdb does not have necessarily have the
         latest C++ API.  In order to get the latest, please visit [the github
         repository](https://github.com/kineticadb/kinetica-api-cpp) page.  



### Detailed Notes: ###

1) 	The Windows and Linux versions are assumed to be built from a directory under
   	<FME-HOME-DIR>/pluginbuilder, where <FME-HOME-DIR> is where FME is installed. 
   
2) 	The kinetica.db and kinetica.fmf files are the same for all operating sytems and architecture (32-bit vs. 64-bit).

3)  The FME connector plug-in library is pre-built for Windows and available
    in this repository:

    *   the 64-bit file, kinetica.dll, is in
        <FME-CONN-DIR>/runtimeFiles-windows/x64.
    *   the 32-bit file, also named kinetica.dll,
        is in <FME-CONN-DIR>/runtimeFiles-windows.

   If these pre-built libraries are used, then there is no need to build the
   connector.  You can skip to the "Using the Connector" section.  


## Building the Connector ##

### Windows ###

Current version of the SDK must be compiled with Visual Studio 2015 build tools.
  This requires a gpudb-api-cpp  built with Visual Studio 2015 build tools, 
which requires:

- Avro and Boost built with VS 2015 build tools (which, for some reason, are
  designated as vs14).


The Visual Studio project directory should be placed in 
`<FME-HOME-DIR>\pluginbuilder\`.

The Visual Studio solution properties use a user defined macro
`CPP_API_PATH`, which needs to be configured to point to the location
of the Kinetica C++ API project on the user's computer.  In order to change
the value of this macro, follow the following steps:

*   In Visual Studio, click on the 'View' menu item, from there, go to
    'Other Windows'->'Property Manager'
*   On the property manager tab, expand `dll_kinetica`.
*   Expand any of the configurations (e.g. Debug|Win32).
*   Double click on `FMEConnector`.
*   On the `FMEConnector Property Pages` dialogue box, click on `User
    Macros` in the pane on the left.
*   On the right side, double click on `CPP_API_PATH`.
*   In the "Add User Macro" dialogue box that pops open, edit the value
    in the "Value" field such that it now has the path to the appropriate
    path to the Kinetica C++ API project.  Click `OK`.
*   Ensure to click on `Apply` button on the `FMEConnector Property Pages`
    dialogue box to save the changes. 



### Linux ###

* The build is done using SCons.  This is the tool FME uses for all their samples.  

* You will need to change the SConscript file to specify the location of the
  Kinetica, Avro, and Boost header and library files.



## Using the Connector ##

The following files must be placed in the specified proper directories:
    

    * kinetica.dll -> <FME-HOME-DIR>/plugins/
    * kinetica.db  -> <FME-HOME-DIR>/formatsinfo
    * kinetica.fmf -> <FME-HOME-DIR>/metafile

    
The DLL, db and fmf files can be found in the runtimeFiles-windows directory for
the Windows distribution, and in the runtimeFiles-linux directory for the linux
build environment.


### Examples ###

There are some examples workspaces in the Examples directory.

For the Kinetica readers and writers, the user needs to set the parameters for
the Kinetica server URL (the IP address can be found in GAdmin, for example, by
clicking on Info->Server Info, and then scrolling down to "hosts".  The first
value for "network_ips" should suffice).  The full URL must be qualified as 
"http://a.b.c.d:PORT", where PORT is the port number (9191 by default), but 
without the double quotation marks.

The user also has to supply the name of the table to read from or to write to. 
Along with that, the reader has 

The example workspaces include:

	*  csvToKinetica.fmw -- Reads data from track_data.csv and writes into a Kinetica
	                        table named 'test_fme_table'.  This workspace should be
	                        run before KineticaToCSV.fmw so that the input Kinetica
	                        table is created and populated.
	*  KineticaToCSV.fmw -- Reads data from a Kinetica table named 'test_fme_table'
	                        (created by csvToKinetica.fmw) and outputs the data to
	                        track_data_out.csv in the Examples directory. In order to
	                        change the server URL, click on the reader, then click
	                        on the icon labeled as Edict Kinetica Parameters', then
	                        click on "Parameters", and then change the "URL".
	*  csvToKinetica_wkt.fmw -- Reads data from "DataWithWKT.csv" into a table named
	                            "fme_wkt_table" (created by the workspace).
	*  KineticaTocsv_wkt.fmw -- Reads data from Kinetica table "fme_wkt_table" into 
	                            "DataWithWKT_out.csv".

### Notes ###

*   The reader requires the user to supply the database name for the field "Source 
    Kinetica Database(s)"; this field is not used and any random value would work.  
    But, the user must put in _some_ value.

*   In order to change the table name or the server URL for a Kinetica reader in
    the examples, click on the reader, then click on "Edit '...[KINETICA]' Parameters"
    (the icon with a cylinder with a gear on it).  Click on the triangle by
    "Paramters" to expand that section and edit the URL and Table fields.

*   In order to change the table name or the server URL for a Kinetica writer, 
    click on "Edit '...[KINETICA]'" (the icon with a cylinder with a gear on it),
    and edit the value of "Destination Kinetica Folder" by clicking on the text
    box and typing the desired table name.  Please do not click on the button
    with three dots (...) by the text box since it opens up a dialog box for you
    to choose a folder on your disk (which obviously will not work).

*   Unfortunately, there is no good way to change the server URL for a Kinetica
    writer; the user has to create a new writer to configure it to use a diferent
    URL.

                                                                                        