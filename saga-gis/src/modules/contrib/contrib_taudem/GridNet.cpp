
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
//                     GridNet.cpp                       //
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

#include "GridNet.h"
#include "gdal_driver.h"
#include "ogr_driver.h"

CGridNet::CGridNet(void)
{
	#ifdef _WIN32
		CSG_String UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		CSG_String UserHomeDir = CSG_String(getenv("HOME"));
	#endif

	CSG_String DefaultTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));

	// INFO
	Set_Name(_TL("Grid Network"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(_TW("Creates 3 grids that contain for each grid cell: 1) the longest path, 2) the total path, and 3) the Strahler order number. These values are derived from the network defined by the D8 flow model.\n\nThe longest upslope length is the length of the flow path from the furthest cell that drains to each cell. The total upslope path length is the length of the entire grid network upslope of each grid cell. Lengths are measured between cell centers taking into account cell size and whether the direction is adjacent or diagonal.\n\nStrahler order is defined as follows: A network of flow paths is defined by the D8 Flow Direction grid. Source flow paths have a Strahler order number of one. When two flow paths of different order join the order of the downstream flow path is the order of the highest incoming flow path. When two flow paths of equal order join the downstream flow path order is increased by 1. When more than two flow paths join the downstream flow path order is calculated as the maximum of the highest incoming flow path order or the second highest incoming flow path order + 1. This generalizes the common definition to cases where more than two flow paths join at a point.\n\nWhere the optional mask grid and threshold value are input, the function is evaluated only considering grid cells that lie in the domain with mask grid value greater than or equal to the threshold value. Source (first order) grid cells are taken as those that do not have any other grid cells from inside the domain draining in to them, and only when two of these flow paths join is order propagated according to the ordering rules. Lengths are also only evaluated counting paths within the domain greater than or equal to the threshold.\n\nIf the optional outlet point shapefile is used, only the outlet cells and the cells upslope (by the D8 flow model) of them are in the domain to be evaluated."));

	// GRIDS
	Parameters.Add_Grid(NULL, "FLOWD8_INPUT"	, _TL("Input D8 Flow Direction"), _TL("A grid of D8 flow directions which are defined, for each cell, as the direction of the one of its eight adjacent or diagonal neighbors with the steepest downward slope."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "MASK_INPUT", _TL("Input Mask (Optional)"), _TL("A grid that is used to determine the domain do be analyzed. If the mask grid value >= mask threshold, then the cell will be included in the domain. While this tool does not have an edge contamination flag, if edge contamination analysis is needed, then a mask grid from a function like AreaD8 that does support edge contamination can be used to achieve the same result."), PARAMETER_INPUT_OPTIONAL);
	Parameters.Add_Grid(NULL, "ORDER_OUTPUT", _TL("Output Strahler Network Order"), _TL("A grid giving the Strahler order number for each cell. A network of flow paths is defined by the D8 Flow Direction grid. Source flow paths have a Strahler order number of one. When two flow paths of different order join the order of the downstream flow path is the order of the highest incoming flow path. When two flow paths of equal order join the downstream flow path order is increased by 1. When more than two flow paths join the downstream flow path order is calculated as the maximum of the highest incoming flow path order or the second highest incoming flow path order + 1. This generalizes the common definition to cases where more than two flow paths join at a point."), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "LONGEST_OUTPUT", _TL("Output Longest Upslope Length"), _TL("A grid that gives the length of the longest upslope D8 flow path terminating at each grid cell. Lengths are measured between cell centers taking into account cell size and whether the direction is adjacent or diagonal."), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "TOTAL_LENGTH_OUTPUT", _TL("Output Total Upslope Length"), _TL("The total upslope path length is the length of the entire D8 flow grid network upslope of each grid cell. Lengths are measured between cell centers taking into account cell size and whether the direction is adjacent or diagonal."), PARAMETER_OUTPUT);

	// SHAPES
	Parameters.Add_Shapes(NULL, "OUTLET_INPUT"	, _TL("Input Outlet Points (Optional)"), _TL("Point shapes defining the outlets of interest. If this input is used, only the cells upslope of these outlet cells are considered to be within the domain being evaluated."), PARAMETER_INPUT_OPTIONAL);

	//Values
	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);
	Parameters.Add_Value(NULL, "THRESHOLD"	, _TL("Mask Threshold"), _TL("This input parameter is used in the calculation mask grid value >= mask threshold to determine if the grid cell is in the domain to be analyzed."), PARAMETER_TYPE_Int, 0);

	// Other
	Parameters.Add_FilePath(NULL, "TEMP_DIR", _TL("Temp File Directory"), _TL("Directory used for storing temporary files during processing."), NULL, DefaultTempDir, false, true, false); 
	

}


bool CGridNet::On_Execute(void)
{
	TSG_Data_Type Type;
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	CSG_String FLOWD8_INPUT_FileName, FLOWD8_INPUT_FilePath;
	CSG_String LONGEST_OUTPUT_FileName, LONGEST_OUTPUT_FilePath;
	CSG_String TOTAL_LENGTH_OUTPUT_FileName, TOTAL_LENGTH_OUTPUT_FilePath;
	CSG_String ORDER_OUTPUT_FileName, ORDER_OUTPUT_FilePath;
	CSG_String OUTLET_INPUT_FileName, OUTLET_INPUT_FilePath, MASK_INPUT_FileName, MASK_INPUT_FilePath;
	CSG_String GDALDriver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile;
	CSG_String LONGESTName, LENGTHName, ORDERName;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	CSG_Grid *FLOWD8_INPUT_Grid, *LONGEST_OUTPUT_Grid, *TOTAL_LENGTH_OUTPUT_Grid, *ORDER_OUTPUT_Grid, *MASK_INPUT_Grid;
	CSG_Shapes *OUTLET_INPUT_Grid;
	int Threshold, nproc;

	FLOWD8_INPUT_Grid = Parameters("FLOWD8_INPUT")->asGrid();
	LONGEST_OUTPUT_Grid = Parameters("LONGEST_OUTPUT")->asGrid();
	TOTAL_LENGTH_OUTPUT_Grid = Parameters("TOTAL_LENGTH_OUTPUT")->asGrid();
	ORDER_OUTPUT_Grid = Parameters("ORDER_OUTPUT")->asGrid();
	MASK_INPUT_Grid = Parameters("MASK_INPUT")->asGrid();
	OUTLET_INPUT_Grid = Parameters("OUTLET_INPUT")->asShapes();
	Threshold = Parameters("THRESHOLD")->asInt();
	nproc = Parameters("NPROC")->asInt();

	GDALDriver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = FLOWD8_INPUT_Grid->Get_Type();

	//TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));
	TempDirPath = Parameters("TEMP_DIR")->asFilePath()->asString();

	FLOWD8_INPUT_FileName = InputBasename + CSG_String("p");
	FLOWD8_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FLOWD8_INPUT_FileName, CSG_String("tif")); 

	LONGEST_OUTPUT_FileName = OutputBasename + CSG_String("plen");
	LONGEST_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, LONGEST_OUTPUT_FileName, CSG_String("tif"));

	TOTAL_LENGTH_OUTPUT_FileName = OutputBasename + CSG_String("tlen");
	TOTAL_LENGTH_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, TOTAL_LENGTH_OUTPUT_FileName, CSG_String("tif"));

	ORDER_OUTPUT_FileName = OutputBasename + CSG_String("gord");
	ORDER_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, ORDER_OUTPUT_FileName, CSG_String("tif"));

	OUTLET_INPUT_FileName = InputBasename + CSG_String("o");
	OUTLET_INPUT_FilePath = SG_File_Make_Path(TempDirPath, OUTLET_INPUT_FileName, CSG_String("shp")); 

	MASK_INPUT_FileName = InputBasename + CSG_String("wg");
	MASK_INPUT_FilePath = SG_File_Make_Path(TempDirPath, MASK_INPUT_FileName, CSG_String("tif")); 

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	// optional flags
	CSG_String OptionalFlags = CSG_String("");
	if (OUTLET_INPUT_Grid != NULL)
	{
		OptionalFlags = CSG_String::Format(SG_T(" -o \"%s\""), OUTLET_INPUT_FilePath.c_str());
	}
	if (MASK_INPUT_Grid != NULL)
	{
		OptionalFlags = OptionalFlags + CSG_String::Format(SG_T(" -mask \"%s\" -thresh %d"), MASK_INPUT_FilePath.c_str(), Threshold);
	}
		
	BinaryName = CSG_String("GridNet"); 
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("\"mpiexec -n %d \"%s\" -p \"%s\" -plen \"%s\" -tlen \"%s\" -gord \"%s\" %s > \"%s\" 2>&1\""), nproc, BinaryPath.c_str(), FLOWD8_INPUT_FilePath.c_str(), LONGEST_OUTPUT_FilePath.c_str(), TOTAL_LENGTH_OUTPUT_FilePath.c_str(), ORDER_OUTPUT_FilePath.c_str(), OptionalFlags.c_str(), LogFile.c_str());
	LONGESTName = CSG_String("D8 LongestPath");
	LENGTHName = CSG_String("D8 TotalLength");
	ORDERName = CSG_String("D8 GridOrder");

	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{
		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(FLOWD8_INPUT_FilePath))
	{
		if (!SG_File_Delete(FLOWD8_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), FLOWD8_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	if (OUTLET_INPUT_Grid != NULL)
	{
		// Delete old input file if exists
		if (SG_File_Exists(OUTLET_INPUT_FilePath))
		{
			if (!SG_File_Delete(OUTLET_INPUT_FilePath))
			{
				Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), OUTLET_INPUT_FilePath.c_str()));
				return( false );
			}
		}
	}

	if (MASK_INPUT_Grid != NULL)
	{
		// Delete old input file if exists
		if (SG_File_Exists(MASK_INPUT_FilePath))
		{
			if (!SG_File_Delete(MASK_INPUT_FilePath))
			{
				Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), MASK_INPUT_FilePath.c_str()));
				return( false );
			}
		}
	}

	// Delete old output files
	if (SG_File_Exists(LONGEST_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(LONGEST_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), LONGEST_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old output files
	if (SG_File_Exists(TOTAL_LENGTH_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(TOTAL_LENGTH_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), TOTAL_LENGTH_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old output files
	if (SG_File_Exists(ORDER_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(ORDER_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), ORDER_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// save input file
	if( !DataSet.Open_Write(FLOWD8_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FLOWD8_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, FLOWD8_INPUT_Grid);
	
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FLOWD8_INPUT_FilePath.c_str()));
		return( false );
	}

	// save mask grid
	if (MASK_INPUT_Grid != NULL)
	{
		if( !DataSet.Open_Write(MASK_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), MASK_INPUT_FilePath.c_str()));
			return( false );
		}
		DataSet.Write(0, MASK_INPUT_Grid);
	
		if( !DataSet.Close() )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), MASK_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// save outlet shapefile
	if (OUTLET_INPUT_Grid != NULL)
	{
		CSG_OGR_DataSource	DataSource;
		CSG_String OGRDriver = CSG_String("ESRI Shapefile");
		if( !DataSource.Create(OUTLET_INPUT_FilePath, OGRDriver) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), OUTLET_INPUT_FilePath.c_str()));
			return( false );
		}
		DataSource.Write(OUTLET_INPUT_Grid, OGRDriver);
	}


	// Run TauDEM GridNet

	Message_Add(CSG_String("Executing ") + sCmd);

	// run process
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

	// Open new output tif file
	if( !DataSet.Open_Read(LONGEST_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated file: "), LONGEST_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		LONGEST_OUTPUT_Grid->Assign(DataSet.Read(0));
		LONGEST_OUTPUT_Grid->Set_Name(LONGESTName);
		Parameters("LONGEST_OUTPUT")->Set_Value(LONGEST_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FLOWD8_INPUT_Grid, colors);
		DataObject_Set_Colors(LONGEST_OUTPUT_Grid, colors);
		DataObject_Update(LONGEST_OUTPUT_Grid, false);		
	}

	// Open new output tif file
	if( !DataSet.Open_Read(TOTAL_LENGTH_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated file: "), TOTAL_LENGTH_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		TOTAL_LENGTH_OUTPUT_Grid->Assign(DataSet.Read(0));
		TOTAL_LENGTH_OUTPUT_Grid->Set_Name(LENGTHName);
		Parameters("TOTAL_LENGTH_OUTPUT")->Set_Value(TOTAL_LENGTH_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FLOWD8_INPUT_Grid, colors);
		DataObject_Set_Colors(TOTAL_LENGTH_OUTPUT_Grid, colors);
		DataObject_Update(TOTAL_LENGTH_OUTPUT_Grid, false);		
	}

	// Open new output tif
	if( !DataSet.Open_Read(ORDER_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated file: "), ORDER_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		ORDER_OUTPUT_Grid->Assign(DataSet.Read(0));
		ORDER_OUTPUT_Grid->Set_Name(ORDERName);
		Parameters("ORDER_OUTPUT")->Set_Value(ORDER_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FLOWD8_INPUT_Grid, colors);
		DataObject_Set_Colors(ORDER_OUTPUT_Grid, colors);
		DataObject_Update(ORDER_OUTPUT_Grid, false);		
	}

	return( true );
}

