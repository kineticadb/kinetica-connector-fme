# Kinetica FME Connector

This is a Safe FME Reader/Writer plugin for Kinetica. To download it, please visit the [FME
Connector repository][FME_GITHUB]. For documentation on this plug-in, please refer to the [FME
documentation][FME_DOCS].

*Note: If you plan to use compiled DLL files then you can skip to the [installation](#installation) section.*

[FME_DOCS]: <https://www.kinetica.com/docs/connectors/fme_kinetica_format.html>
[FME_GITHUB]: <https://github.com/kineticadb/kinetica-connector-fme>

## Useful References

* [FME Support](https://support.safe.com/KnowledgeDocumentation)
* [Spatial Relations](http://docs.safe.com/fme/html/FME_Desktop_Documentation/FME_Transformers/Transformers/spatialrelations.htm)

**General GIS:**

* [A Gentle Introduction to GIS](https://docs.qgis.org/2.18/en/docs/gentle_gis_introduction)
* [Geodetic Datums: NAD 27, NAD 83 and WGS84](https://gisgeography.com/geodetic-datums-nad27-nad83-wgs84)
* [North American Datum](https://en.wikipedia.org/wiki/North_American_Datum)
* [World Geodetic System](https://en.wikipedia.org/wiki/World_Geodetic_System)

## Building

### Prerequisites

Before you build this project you must have the following:

* An **FME Desktop** installation with the **SDK** option (e.g. `pluginbuilder` directory). The Windows 
and Linux FME SDK's are assumed to be built from a directory under `<FME-HOME-DIR>/pluginbuilder`.
* **Visual Studio 2015** build tools.
* A version of `gpudb-api-cpp` compiled with **VS 2015**.

### Obtaining the Kinetica C++ API

This project requires the `gpudb-api-cpp` library compiled with **Visual Studio 2015** build tools.

A copy of the 32-bit library exists in `<FME-CONN-DIR>/runtimeFiles-windows`, 
where `<FME-CONN-DIR>` is usually at `<FME-HOME-DIR>/pluginbuilder/gpudb-connector-fme`.

The pair of files `gpudbapilib.lib` and `gpudbapilib.pdb` are not necessarily the latest version. 
In order to get the latest, please visit the [Kinetica C++ API repository][KINETICA_API].  

To build the `gpudb-api-cpp` library, you need builds of `avro-cpp-1.8.1` and `boost_1_59_0` compiled 
with VS 2015  (which, for some reason, are designated as vs14).

[KINETICA_API]: <https://github.com/kineticadb/kinetica-api-cpp>

### Buildng on Windows

The VS 2015 project directory should be placed in `<FME-HOME-DIR>\pluginbuilder\`.

The Visual Studio solution properties use a user defined macro `CPP_API_PATH`, which needs to be
configured to point to the location of `gpudb-api-cpp` on the user's computer.  In
order to change the value of this macro, follow the following steps:

1. In Visual Studio, click on the **View** menu item, from there, go to **Other Windows->Property Manager**
1. On the property manager tab, expand `dll_kinetica`.
1. Expand any of the configurations (e.g. Debug|Win32).
1. Double click on **FMEConnector**.
1. On the **FMEConnector Property Pages** dialogue box, click on **User Macros** in the pane on the left.
1. On the right side, double click on `PP_API_PATH`.
1. In the **Add User Macro** dialogue box that pops open, edit the value in the **Value** field such 
  	that it now has the path to the appropriate path to the `gpudb-api-cpp` project. Click **OK**.
1. Ensure to click on **Apply** button on the **FMEConnector Property Pages** dialogue box to save the changes. 

### Building on Linux

The build is done using `SCons`, the tool FME uses for all their samples. You will need to change the 
`SConscript` file to specify the location of the Kinetica, Avro, and Boost header and library files.

## Installation

The installation files can be found in the [runtimeFiles-windows](runtimeFiles-windows) directory for
the Windows distribution, and in the [runtimeFiles-linux](runtimeFiles-linux) directory for Linux.

The following files must be copied to the appropriate directories under the FME installation:

```
kinetica.dll -> <FME-HOME-DIR>/plugins/
kinetica.db  -> <FME-HOME-DIR>/formatsinfo
kinetica.fmf -> <FME-HOME-DIR>/metafile
```

The `kinetica.db` and `kinetica.fmf` files are the same for all operating sytems and 
architecture (32-bit vs. 64-bit).

The FME connector plug-in `kinetica.dll` is pre-built for Windows and available at these locations:
  * 64-bit: `<FME-CONN-DIR>/runtimeFiles-windows/x64`.
  * 32-bit: `<FME-CONN-DIR>/runtimeFiles-windows`.

If you are are using a debug build of the connector then you will need to copy additional debug DLL's to
the `plugins` directory:
* `msvcp140d.dll`
* `ucrtbased.dll`
* `vcruntime140d.dll`

## Usage

Before following this section you will need to complete the Installation steps and have an open FME desktop.

### Adding a Writer

The FME Writer is a component that will take output from an FME stream and insert rows into Kinetica. 

1. Select **Writers->Add Writer...**
1. Under the **Formats** drop-down select **More Formats**.
1. Select **Kinetica** and click **Ok**.
1. Under **Dataset** enter the destination table.
1. Click the **Parameters...** button.
1. Enter the following parameters:
  * **URL:** Indicates the kientica host and port (e.g. `http://host:9191`)
  * **Table Name:** Destination table.
  * **Collection:** Collection in which to create the table.
  * **Username/Password:** An account you use to login with GAdmin.
1. Click **Ok** twice to close both dialogs.
1. You will be presented with a dialog asking for a feaeture type. Select the feature to import.
1. At this point you shoudl see the writer appear in the workspace.

### Editing Connection Information

You can view/edit connection infomration in an existing Reader or Writer.

1. From the **Navigator** panel select the writer.
1. Expand the **Paramters** key.
1. Double click on URL parameter to edit it. You can edit the follwing parameters:
  * URL (e.g. `http://host:9191`)
  * Collection
  * User/Password
  * Collection
  * Geospatial field names

note: As an alternative you can right click on the reader/writer and select **Edit Parameters...**.

### Editing the Table Name

1. Select the Kinetica writer from the workspace.
1. In the parameter editor choose a new table name.

### Useful Transformers

* **Reprojector:** Use this to convert other coordinate systems to **WGS 1984** used by Kinetica. Choose 
	`LL-WGS84` as the destination coordinate system.
* **Coordinate Extractor:** Extract coordinates to separate x and y columns. 
* **Geometry Extractor:** Extract coordinates to **WKT** columns. Specify **OGC Well Known Text** as 
	the **Geometry Encoding**.

### Useful Settings

* For testing you can limit the number of rows read from the source. Select a reader from the Navigator 
and select **Parameters-Features to Read->Max Features to Read**. 
* Sometimes FME will read all features before sending to the reader that can cause longer execution time 
    and/or memory errors. To fix this from the Navigator panel select 
	**Workspace Parameters->Translation->Order Writers By**. Double click on the parameter and select 
	**First Feature Written** from the drop-down.

## Examples

Some example workspaces are available to assist with testing in [Examples](Examples). The examples 
consist of workspaces that dransfer data to/from CSV files and tables.

| File Name | Type | Source | Destination | 
| --- | ---| --- | --- |
| `csvToKinetica.fmw` | writer | `Examples/track_data.csv` | `test_fme_table` |
| `KineticaToCSV.fmw` | reader | `test_fme_table` | `Examples/track_data_out.csv` |
| `csvToKinetica_wkt.fmw` | writer | `Examples/DataWithWKT.csv` | `fme_wkt_table` |
| `KineticaTocsv_wkt.fmw` | reader | `fme_wkt_table` | `Examples/DataWithWKT_out.csv` |

You can use the writer examples to create tables that can then be consumed by the readers.

Before running the above workspaces you will need to edit connection information in the Kinetica readers and 
writers (see [Editing Connection Information](#editing-connection-information))


                                                                                        