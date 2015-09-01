
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       contrib_taudem                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     StreamNet.cpp                     //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'. SAGA is free software; you   //
// can redistribute it and/or modify it under the terms  //
// of the GNU General Public License as published by the //
// Free Software Foundation; version 2 of the License.   //
//                                                       //
// SAGA is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the    //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU General Public        //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU General    //
// Public License along with this program; if not,       //
// write to the Free Software Foundation, Inc.,          //
// 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, //
// USA.                                                  //
//                                                       //
///////////////////////////////////////////////////////////

#include "StreamNet.h"
#include "gdal_driver.h"
#include "ogr_driver.h"

CStreamNet::CStreamNet(void)
{
	// INFO
	Set_Name(_TL("Stream Reach and Watershed"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(CSG_String("This tool produces a vector network and shapefile from the stream raster grid. The flow direction grid is used to connect flow paths along the stream raster. The Strahler order of each stream segment is computed. The subwatershed draining to each stream segment (reach) is also delineated and labeled with the value identifier that corresponds to the WSNO (watershed number) attribute in the Stream Reach Shapefile.\n\nThis tool orders the stream network according to the Strahler ordering system. Streams that don't have any other streams draining in to them are order 1. When two stream reaches of different order join the order of the downstream reach is the order of the highest incoming reach. When two reaches of equal order join the downstream reach order is increased by 1. When more than two reaches join the downstream reach order is calculated as the maximum of the highest incoming reach order or the second highest incoming reach order + 1. This generalizes the common definition to cases where more than two reaches join at a point. The network topological connectivity is stored in the Stream Network Tree file, and coordinates and attributes from each grid cell along the network are stored in the Network Coordinates file.\n\nThe stream raster grid is used as the source for the stream network, and the flow direction grid is used to trace connections within the stream network. Elevations and contributing area are used to determine the elevation and contributing area attributes in the network coordinate file. Points in the outlets shapefile are used to logically split stream reaches to facilitate representing watersheds upstream and downstream of monitoring points. The program uses the attribute field \"id\" in the outlets shapefile as identifiers in the Network Tree file. This tool then translates the text file vector network representation in the Network Tree and Coordinates files into a shapefile. Further attributes are also evaluated. The program has an option to delineate a single watershed by representing the entire area draining to the Stream Network as a single value in the output watershed grid."));

	// GRIDS
	Parameters.Add_Grid(NULL, "FEL_INPUT"	, _TL("Input Pit Filled Elevation"), _TL("This input is a grid of elevation values. This is usually the output of the \"Pit Remove\" tool, in which case it is elevations with pits removed. Pits are low elevation areas in digital elevation models (DEMs) that are completely surrounded by higher terrain. They are generally taken to be artifacts that interfere with the processing of flow across DEMs, so are removed by raising their elevation to the point where they just drain. This step is not essential if you have reason to believe that the pits in your DEM are real. Elevation values are used in this function to compute the slope of stream segments. Not removing pits can result in negative (i.e. uphill) stream segment slopes."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "FLOWD8_INPUT"	, _TL("Input D8 Flow Direction"), _TL("This input is a grid of flow directions that are encoded using the D8 method where all flow from a cells goes to a single neighboring cell in the direction of steepest descent."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "AREAD8_INPUT"	, _TL("Input D8 Drainage Area"), _TL("A grid giving the contributing area value in terms of the number of grid cells (or the summation of weights) for each cell taken as its own contribution plus the contribution from upslope neighbors that drain in to it using the D8 algorithm. This is usually the output of the \"D8 Contributing Area\" tool and is used to determine the contributing area attribute in the Network Coordinate file."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "SRC_INPUT"	, _TL("Input Stream Raster"), _TL("An indicator grid indicating streams, by using a grid cell value of 1 on streams and 0 off streams. Several of the \"Stream Network Analysis\" tools produce this type of grid. The Stream Raster Grid is used as the source for the stream network."), PARAMETER_INPUT);

	Parameters.Add_Grid(NULL, "ORD_OUTPUT"	, _TL("Output Stream Order"), _TL("The Stream Order Grid has cells values of streams ordered according to the Strahler order system. The Strahler ordering system defines order 1 streams as stream reaches that don't have any other reaches draining in to them. When two stream reaches of different order join the order of the downstream reach is the order of the highest incoming reach. When two reaches of equal order join the downstream reach order is increased by 1. When more than two reaches join the downstream reach order is calculated as the maximum of the highest incoming reach order or the second highest incoming reach order + 1. This generalizes the common definition to cases where more than two reaches join at a point. This generalizes the common definition to cases where more than two flow paths join at a point."), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "W_OUTPUT"	, _TL("Output Watershed"), _TL("This output grid identified each reach watershed with a unique ID number, or in the case where the delineate single watershed option was checked, the entire area draining to the stream network is identified with a single ID."), PARAMETER_OUTPUT);

	// SHAPES
	Parameters.Add_Shapes(NULL, "OUTLET_INPUT"	, _TL("Input Outlet Points (Optional)"), CSG_String("Point shapes defining points of interest. If this is used, the tool will only deliiniate the stream network upstream of these outlets. Additionally, the points are used to logically split stream reaches to facilitate representing watersheds upstream and downstream of monitoring points. Points must have an integer attribute field \"id\" as it is for identifiers in the Network Tree file."), PARAMETER_INPUT_OPTIONAL);
	Parameters.Add_Shapes(NULL, "NET_OUTPUT"	, _TL("Output Stream Reach"), _TL("This output is a polyline shapefile giving the links in a stream network. The columns in the attribute table are:\n\nLINKNO- Link Number. A unique number associated with each link (segment of channel between junctions). This is arbitrary and will vary depending on number of processes used.\n\nDSLINKNO - Link Number of the downstream link. -1 indicates that this does not exist.\n\nUSLINKNO1 - Link Number of first upstream link. (-1 indicates no link upstream, i.e. for a source link)\n\nUSLINKNO2 - Link Number of second upstream link. (-1 indicates no second link upstream, i.e. for a source link or an internal monitoring point where the reach is logically split but the network does not bifurcate.)\n\nDSNODEID - Node identifier for node at downstream end of stream reach. This identifier corresponds to the \"id\" attribute from the Outlets shapefile used to designate nodes.\n\nOrder - Strahler Stream Order\n\nLength - Length of the link. The units are the horizontal map units of the underlying DEM grid.\n\nMagnitude - Shreve Magnitude of the link. This is the total number of sources upstream\n\nDS_Cont_Ar - Drainage area at the downstream end of the link. Generally this is one grid cell upstream of the downstream end because the drainage area at the downstream end grid cell includes the area of the stream being joined.\n\nDrop - Drop in elevation from the start to the end of the link\n\nSlope - Average slope of the link (computed as drop/length)\n\nStraight_L - Straight line distance from the start to the end of the link\n\nUS_Cont_Ar - Drainage area at the upstream end of the link\n\nWSNO - Watershed number. Cross reference to the *w.shp and *w grid files giving the identification number of the watershed draining directly to the link.\n\nDOUT_END - Distance to the eventual outlet (i.e. the most downstream point in the stream network) from the downstream end of the link.\n\nDOUT_START - Distance to the eventual outlet from the upstream end of the link\n\nDOUT_MID - Distance to the eventual outlet from the midpoint of the link"), PARAMETER_OUTPUT);
	
	// TABLES
	Parameters.Add_Table(NULL, "TREE_OUTPUT", _TL("Output Network Connectivity Tree"), _TL("This output is a text file that details the network topological connectivity is stored in the Stream Network Tree file. Columns are as follows:\n\nLink Number (Arbitrary - will vary depending on number of processes used)\n\nStart Point Number in Network coordinates (*coord.dat) file (Indexed from 0)\n\nEnd Point Number in Network coordinates (*coord.dat) file (Indexed from 0)\n\nNext (Downstream) Link Number. Points to Link Number. -1 indicates no links downstream, i.e. a terminal link\n\nFirst Previous (Upstream) Link Number. Points to Link Number. -1 indicates no upstream links.\n\nSecond Previous (Upstream) Link Numbers. Points to Link Number. -1 indicates no upstream links. Where only one previous link is -1, it indicates an internal monitoring point where the reach is logically split, but the network does not bifurcate.\n\nStrahler Order of Link\n\nMonitoring point identifier at downstream end of link. -1 indicates downstream end is not a monitoring point.\n\nNetwork magnitude of the link, calculated as the number of upstream sources (following Shreve)."), PARAMETER_OUTPUT); 
	Parameters.Add_Table(NULL, "COORD_OUTPUT", _TL("Output Network Coordinates"), _TL("This output is a text file that contains the coordinates and attributes of points along the stream network. Columns are as follows:\n\nX coordinate\n\nY Coordinate\n\nDistance along channels to the downstream end of a terminal link\n\nElevation\n\nContributing area"), PARAMETER_OUTPUT); 

	// Booleans
	Parameters.Add_Value(
		NULL, "SW"		, _TL("Delineate Single Watershed"),
		_TL("This option causes the tool to delineate a single watershed by representing the entire area draining to the Stream Network as a single value in the output watershed grid. Otherwise a seperate watershed is delineated for each stream reach."),
		PARAMETER_TYPE_Bool, false
	);

	// VALUES
	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);

}


bool CStreamNet::On_Execute(void)
{
	// Inputs and Output Strings
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	CSG_String FEL_INPUT_FileName, FEL_INPUT_FilePath;
	CSG_String FLOWD8_INPUT_FileName, FLOWD8_INPUT_FilePath;
	CSG_String AREAD8_INPUT_FileName, AREAD8_INPUT_FilePath;
	CSG_String SRC_INPUT_FileName, SRC_INPUT_FilePath;
	CSG_String ORD_OUTPUT_FileName, ORD_OUTPUT_FilePath, ORD_OUTPUT_Name;
	CSG_String W_OUTPUT_FileName, W_OUTPUT_FilePath, W_OUTPUT_Name;
	CSG_String NET_OUTPUT_FileName, NET_OUTPUT_FilePath, NET_OUTPUT_Name;
	CSG_String TREE_OUTPUT_FileName, TREE_OUTPUT_FilePath, TREE_OUTPUT_Name;
	CSG_String COORD_OUTPUT_FileName, COORD_OUTPUT_FilePath, COORD_OUTPUT_Name;
	CSG_String OUTLET_INPUT_FileName, OUTLET_INPUT_FilePath;

	// Data Objects
	CSG_Grid *FEL_INPUT_Grid, *FLOWD8_INPUT_Grid, *SRC_INPUT_Grid, *AREAD8_INPUT_Grid, *ORD_OUTPUT_Grid, *W_OUTPUT_Grid;
	CSG_Shapes *OUTLET_INPUT_Shapes, *NET_OUTPUT_Shapes;
	CSG_Table *TREE_OUTPUT_Table, *COORD_OUTPUT_Table;

	// Misc
	TSG_Data_Type Type;
	CSG_String GDALDriver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	CSG_OGR_DataSource	OGRDataSource;
	CSG_String OGRDriver = CSG_String("ESRI Shapefile");
	bool sw;
	int nproc;
	
	// Grab inputs
	FEL_INPUT_Grid = Parameters("FEL_INPUT")->asGrid();
	FLOWD8_INPUT_Grid = Parameters("FLOWD8_INPUT")->asGrid();
	SRC_INPUT_Grid = Parameters("SRC_INPUT")->asGrid();
	AREAD8_INPUT_Grid = Parameters("AREAD8_INPUT")->asGrid();
	ORD_OUTPUT_Grid = Parameters("ORD_OUTPUT")->asGrid();
	W_OUTPUT_Grid = Parameters("W_OUTPUT")->asGrid();
	nproc = Parameters("NPROC")->asInt();

	OUTLET_INPUT_Shapes = Parameters("OUTLET_INPUT")->asShapes();
	NET_OUTPUT_Shapes = Parameters("NET_OUTPUT")->asShapes();

	TREE_OUTPUT_Table = Parameters("TREE_OUTPUT")->asTable();
	COORD_OUTPUT_Table = Parameters("COORD_OUTPUT")->asTable();

	sw = Parameters("SW")->asBool();

	GDALDriver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = FEL_INPUT_Grid->Get_Type();

	TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));

	FEL_INPUT_FileName = InputBasename + CSG_String("fel");
	FEL_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FEL_INPUT_FileName, CSG_String("tif")); 

	FLOWD8_INPUT_FileName = InputBasename + CSG_String("p");
	FLOWD8_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FLOWD8_INPUT_FileName, CSG_String("tif"));

	SRC_INPUT_FileName = InputBasename + CSG_String("src");
	SRC_INPUT_FilePath = SG_File_Make_Path(TempDirPath, SRC_INPUT_FileName, CSG_String("tif"));

	AREAD8_INPUT_FileName = InputBasename + CSG_String("ad8");
	AREAD8_INPUT_FilePath = SG_File_Make_Path(TempDirPath, AREAD8_INPUT_FileName, CSG_String("tif"));

	ORD_OUTPUT_FileName = OutputBasename + CSG_String("ord");
	ORD_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, ORD_OUTPUT_FileName, CSG_String("tif"));
	ORD_OUTPUT_Name = CSG_String("NetworkOrder");

	W_OUTPUT_FileName = OutputBasename + CSG_String("w");
	W_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, W_OUTPUT_FileName, CSG_String("tif"));
	W_OUTPUT_Name = CSG_String("WatershedIDs");

	OUTLET_INPUT_FileName = InputBasename + CSG_String("o");
	OUTLET_INPUT_FilePath = SG_File_Make_Path(TempDirPath, OUTLET_INPUT_FileName, CSG_String("shp"));

	NET_OUTPUT_FileName = OutputBasename + CSG_String("net");
	NET_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, NET_OUTPUT_FileName, CSG_String("shp"));
	NET_OUTPUT_Name = CSG_String("Channel Network");

	TREE_OUTPUT_FileName = OutputBasename + CSG_String("tree");
	TREE_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, TREE_OUTPUT_FileName, CSG_String("dat"));
	TREE_OUTPUT_Name = CSG_String("Channel Network Tree");

	COORD_OUTPUT_FileName = OutputBasename + CSG_String("coord");
	COORD_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, COORD_OUTPUT_FileName, CSG_String("dat"));
	COORD_OUTPUT_Name = CSG_String("Channel Network Coords");

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	// options
	CSG_String OptionalFlags = CSG_String("");
	if (OUTLET_INPUT_Shapes != NULL)
	{
		OptionalFlags = CSG_String::Format(SG_T("-o %s "), OUTLET_INPUT_FilePath.c_str());
	}
	if (sw)
	{
		OptionalFlags = OptionalFlags + CSG_String::Format(SG_T("-sw"));
	}

	// exec commnad
	BinaryName = CSG_String("StreamNet"); // D8
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("mpiexec -n %d %s -fel %s -p %s -ad8 %s -src %s -ord %s -tree %s -coord %s -net %s -w %s %s> %s 2>&1"), nproc, BinaryPath.c_str(), FEL_INPUT_FilePath.c_str(), FLOWD8_INPUT_FilePath.c_str(), AREAD8_INPUT_FilePath.c_str(), SRC_INPUT_FilePath.c_str(), ORD_OUTPUT_FilePath.c_str(), TREE_OUTPUT_FilePath.c_str(), COORD_OUTPUT_FilePath.c_str(), NET_OUTPUT_FilePath.c_str(), W_OUTPUT_FilePath.c_str(), OptionalFlags.c_str(), LogFile.c_str());

	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{
		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	CSG_String FilePaths [10] = {FEL_INPUT_FilePath, FLOWD8_INPUT_FilePath, SRC_INPUT_FilePath, AREAD8_INPUT_FilePath, ORD_OUTPUT_FilePath, W_OUTPUT_FilePath, OUTLET_INPUT_FilePath, NET_OUTPUT_FilePath, TREE_OUTPUT_FilePath, COORD_OUTPUT_FilePath};
	for (int i = 0; i < 10; i++)
	{
		CSG_String FilePath = FilePaths[i];
		// Delete old file if exists
		if (SG_File_Exists(FilePath))
		{
			if (!SG_File_Delete(FilePath))
			{
				Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete existing file: "), FilePath.c_str()));
				return( false );
			}
		}
	}

	// SAVE TIFFS
	CSG_String TIFF_INPUT_FilePaths [4] = {FEL_INPUT_FilePath, FLOWD8_INPUT_FilePath, SRC_INPUT_FilePath, AREAD8_INPUT_FilePath};
	CSG_Grid* TIFF_INPUT_Grids [4] = {FEL_INPUT_Grid, FLOWD8_INPUT_Grid, SRC_INPUT_Grid, AREAD8_INPUT_Grid};
	for (int i = 0; i < 4; i++)
	{
		CSG_String FilePath = TIFF_INPUT_FilePaths[i];
		CSG_Grid* Grid = TIFF_INPUT_Grids[i];

		if( !DataSet.Open_Write(FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FilePath.c_str()));
			return( false );
		}
		DataSet.Write(0, Grid);
		if( !DataSet.Close() )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FilePath.c_str()));
			return( false );
		}
	}

	if (OUTLET_INPUT_Shapes != NULL)
	{
		// save outlet shapefile
		CSG_String OGRDriver = CSG_String("ESRI Shapefile");
		if( !OGRDataSource.Create(OUTLET_INPUT_FilePath, OGRDriver) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), OUTLET_INPUT_FilePath.c_str()));
			return( false );
		}
		OGRDataSource.Write(OUTLET_INPUT_Shapes, OGRDriver);
		OGRDataSource.Destroy();
	}
	

	// Run TauDEM StreamNet
	Message_Add(CSG_String("Executing ") + sCmd);
	if (system(sCmd.b_str()) != 0)	
	{
		Error_Set(CSG_String::Format(SG_T("Error executing '%s' see Execution log for details"), BinaryName.c_str()));
		// read log output
		CSG_File File;
		if (File.Open(LogFile, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Message_Add(Line);
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + LogFile + CSG_String(" for reading")));
		}

		return( false );
	}

	// Load output tiffs

	if( !DataSet.Open_Read(ORD_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open output file: "), ORD_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else
	{
		ORD_OUTPUT_Grid->Assign(DataSet.Read(0));
		ORD_OUTPUT_Grid->Set_Name(ORD_OUTPUT_Name);
		Parameters("ORD_OUTPUT")->Set_Value(ORD_OUTPUT_Grid);	
	}

	if( !DataSet.Open_Read(W_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open output file: "), W_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else
	{
		W_OUTPUT_Grid->Assign(DataSet.Read(0));
		W_OUTPUT_Grid->Set_Name(W_OUTPUT_Name);
		Parameters("W_OUTPUT")->Set_Value(W_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(SRC_INPUT_Grid, colors);
		DataObject_Set_Colors(ORD_OUTPUT_Grid, colors);
		DataObject_Update(ORD_OUTPUT_Grid, false);		
	}

	// load output shapefile
	if( !OGRDataSource.Create(NET_OUTPUT_FilePath) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for reading: "), NET_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	NET_OUTPUT_Shapes->Assign(OGRDataSource.Read(0, 0));
	NET_OUTPUT_Shapes->Set_Name(NET_OUTPUT_Name);
	OGRDataSource.Destroy();
	
	// load table data

	if (!SG_File_Exists(COORD_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Output file does not exist: "), COORD_OUTPUT_FilePath.c_str()));
		return false;
	} 
	else
	{
		COORD_OUTPUT_Table->Destroy();
		COORD_OUTPUT_Table->Set_Name(COORD_OUTPUT_Name);

		// create table fields
		COORD_OUTPUT_Table->Add_Field(SG_T("X"), SG_DATATYPE_Double);
		COORD_OUTPUT_Table->Add_Field(SG_T("Y"), SG_DATATYPE_Double);
		COORD_OUTPUT_Table->Add_Field(SG_T("Terminal Distance"), SG_DATATYPE_Double);
		COORD_OUTPUT_Table->Add_Field(SG_T("Elevation"), SG_DATATYPE_Double);
		COORD_OUTPUT_Table->Add_Field(SG_T("Contributing Area"), SG_DATATYPE_Double);

		// read table data
		CSG_File File;
		if (File.Open(COORD_OUTPUT_FilePath, SG_FILE_R, false))
		{
			CSG_String Line;
			// determine number of lines
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Line.Trim();
				if (Line.Length() == 0) 
				{
					break;
				} 
				else
				{
					CSG_Table_Record *Record = COORD_OUTPUT_Table->Add_Record();
					for (int i = 0; i < COORD_OUTPUT_Table->Get_Field_Count(); i++)
					{
						Record->Set_Value(i, Line.asDouble());
						Line = Line.AfterFirst('\t');
						Line.Trim();
					}
				}
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + COORD_OUTPUT_FilePath + CSG_String(" for reading")));
		}
	}

	if (!SG_File_Exists(TREE_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Output file does not exist: "), TREE_OUTPUT_FilePath.c_str()));
		return false;
	} 
	else
	{
		TREE_OUTPUT_Table->Destroy();
		TREE_OUTPUT_Table->Set_Name(TREE_OUTPUT_Name);

		// create table fields
		TREE_OUTPUT_Table->Add_Field(SG_T("Link"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("Start Point"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("End Point"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("Next (Downstream) Link"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("First Previous (Upstream) Link"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("Second Previous (Upstream) Link"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("Strahler Order"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("Monitoring Point ID"), SG_DATATYPE_Int);
		TREE_OUTPUT_Table->Add_Field(SG_T("Link Network Magnitude"), SG_DATATYPE_Int);

		// read table data
		CSG_File File;
		if (File.Open(TREE_OUTPUT_FilePath, SG_FILE_R, false))
		{
			CSG_String Line;
			// determine number of lines
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Line.Trim();
				if (Line.Length() == 0) 
				{
					break;
				} 
				else
				{
					CSG_Table_Record *Record = TREE_OUTPUT_Table->Add_Record();
					for (int i = 0; i < TREE_OUTPUT_Table->Get_Field_Count(); i++)
					{
						Record->Set_Value(i, Line.asDouble());
						Line = Line.AfterFirst('\t');
						Line.Trim();
					}
				}
			}
			File.Close();
		} else 
		{
			Message_Add(CSG_String("Unable to open " + TREE_OUTPUT_FilePath + CSG_String(" for reading")));
		}
	}


	return( true );

}

