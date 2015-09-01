
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
//                     D8ContributingArea.cpp            //
//                                                       //
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
///////////////////////////////////////////////////////////

#include "D8ContributingArea.h"
#include "gdal_driver.h"
#include "ogr_driver.h"

CD8ContributingArea::CD8ContributingArea(void)
{
	// INFO
	Set_Name(_TL("D8 Contributing Area"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(CSG_String("Calculates a grid of contributing areas using the single direction D8 flow model. The contribution of each grid cell is taken as one (or when the optional weight grid is used, the value from the weight grid). The contributing area for each grid cell is taken as its own contribution plus the contribution from upslope neighbors that drain in to it according to the D8 flow model.\n\nIf the optional outlet point shapefile is used, only the outlet cells and the cells upslope (by the D8 flow model) of them are in the domain to be evaluated.\n\nBy default, the tool checks for edge contamination. This is defined as the possibility that a contributing area value may be underestimated due to grid cells outside of the domain not being counted. This occurs when drainage is inwards from the boundaries or areas with no data values for elevation. The algorithm recognizes this and reports \"no data\" for the contributing area. It is common to see streaks of \"no data\" values extending inwards from boundaries along flow paths that enter the domain at a boundary. This is the desired effect and indicates that contributing area for these grid cells is unknown due to it being dependent on terrain outside of the domain of data available. Edge contamination checking may be turned off in cases where you know this is not an issue or want to ignore these problems, if for example, the DEM has been clipped along a watershed outline."));

	// GRIDS
	Parameters.Add_Grid(NULL, "FLOWD8_INPUT", _TL("Input D8 Flow Direction"), _TL("A grid of D8 flow directions which are defined, for each cell, as the direction of the one of its eight adjacent or diagonal neighbors with the steepest downward slope."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "AREAD8_OUTPUT", _TL("Output D8 Contributing Area"), _TL("A grid of contributing area values calculated as the cells own contribution plus the contribution from upslope neighbors that drain in to it according to the D8 flow model."), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "WEIGHTGRID_INPUT", _TL("Input Weight Grid (Optional)"), _TL("A grid giving contribution to flow for each cell. These contributions (also sometimes referred to as weights or loadings) are used in the contributing area accumulation. If this input grid is not used, the contribution to flow will assumed to be one for each grid cell."), PARAMETER_INPUT_OPTIONAL);
	
	// SHAPES
	Parameters.Add_Shapes(NULL, "OUTLET_INPUT"	, _TL("Input Outlet Points (Optional)"), _TL("Point shapes defining the outlets of interest. If this input is used, only the cells upslope of these outlet cells are considered to be within the domain being evaluated."), PARAMETER_INPUT_OPTIONAL);

	// Check for edge contamination
	Parameters.Add_Value(
		NULL, "NC"		, _TL("Check for Edge Contamination"),
		CSG_String("A flag that indicates whether the tool should check for edge contamination. Edge contamination is defined as the possibility that a contributing area value may be underestimated due to the fact that grid cells outside of the domain have not been evaluated. This occurs when drainage is inwards from the boundaries or areas with \"no data\" values for elevation. The algorithm recognizes this and reports \"no data\" for the impated cells. It is common to see streaks of \"no data\" values extending inwards from boundaries along flow paths that enter the domain at a boundary. This is the desired effect and indicates that contributing area for these grid cells is unknown due to it being dependent on terrain outside of the domain of available data. Edge contamination checking may be turned off in cases where you know this is not an issue, or want to ignore these problems, if for example, the DEM has been clipped along a watershed outline."),
		PARAMETER_TYPE_Bool, true
	);

	//Values
	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);

}


bool CD8ContributingArea::On_Execute(void)
{
	TSG_Data_Type Type;
	CSG_String FLOWD8_INPUT_FileName, FLOWD8_INPUT_FilePath;
	CSG_String AREAD8_OUTPUT_FileName, AREAD8_OUTPUT_FilePath;
	CSG_String OUTLET_INPUT_FileName, OUTLET_INPUT_FilePath;
	CSG_String WEIGHTGRID_INPUT_FileName, WEIGHTGRID_INPUT_FilePath;
	CSG_String GDALDriver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile, AreaName;
	CSG_String InputBaseName = "input";
	CSG_String OutputBaseName = "output";
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	CSG_Grid *FLOWD8_INPUT_Grid, *AREAD8_OUTPUT_Grid, *WEIGHTGRID_INPUT_Grid;
	CSG_Shapes *OUTLET_INPUTS_Shapes;
	int	nproc;

	FLOWD8_INPUT_Grid = Parameters("FLOWD8_INPUT")->asGrid();
	AREAD8_OUTPUT_Grid = Parameters("AREAD8_OUTPUT")->asGrid();
	WEIGHTGRID_INPUT_Grid = Parameters("WEIGHTGRID_INPUT")->asGrid();
	OUTLET_INPUTS_Shapes = Parameters("OUTLET_INPUT")->asShapes();
	nproc = Parameters("NPROC")->asInt();

	GDALDriver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = FLOWD8_INPUT_Grid->Get_Type();

	TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));

	FLOWD8_INPUT_FileName = InputBaseName + CSG_String("P");
	FLOWD8_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FLOWD8_INPUT_FileName, CSG_String("tif")); 

	AREAD8_OUTPUT_FileName = OutputBaseName + CSG_String("ad8");
	AREAD8_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, AREAD8_OUTPUT_FileName, CSG_String("tif"));

	OUTLET_INPUT_FileName = InputBaseName + CSG_String("o");
	OUTLET_INPUT_FilePath = SG_File_Make_Path(TempDirPath, OUTLET_INPUT_FileName, CSG_String("shp")); 

	WEIGHTGRID_INPUT_FileName = InputBaseName + CSG_String("wg");
	WEIGHTGRID_INPUT_FilePath = SG_File_Make_Path(TempDirPath, WEIGHTGRID_INPUT_FileName, CSG_String("tif")); 

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	// optional flags
	CSG_String OptionalFlags = CSG_String("");
	if (OUTLET_INPUTS_Shapes != NULL)
	{
		OptionalFlags = CSG_String::Format(SG_T(" -o %s"), OUTLET_INPUT_FilePath.c_str());
	}
	if (WEIGHTGRID_INPUT_Grid != NULL)
	{
		OptionalFlags = OptionalFlags + CSG_String::Format(SG_T(" -wg %s"), WEIGHTGRID_INPUT_FilePath.c_str());
	}
	if( !Parameters("NC")->asBool() ){
		OptionalFlags = OptionalFlags + CSG_String(" -nc");
	}

	BinaryName = CSG_String("AreaD8"); 
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("mpiexec -n %d %s -p %s -ad8 %s %s > %s 2>&1"), nproc, BinaryPath.c_str(), FLOWD8_INPUT_FilePath.c_str(), AREAD8_OUTPUT_FilePath.c_str(), OptionalFlags.c_str(), LogFile.c_str());
	AreaName = CSG_String("D8 Contributing Area");

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

	if (OUTLET_INPUTS_Shapes != NULL)
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

	if (WEIGHTGRID_INPUT_Grid != NULL)
	{
		// Delete old input file if exists
		if (SG_File_Exists(WEIGHTGRID_INPUT_FilePath))
		{
			if (!SG_File_Delete(WEIGHTGRID_INPUT_FilePath))
			{
				Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), WEIGHTGRID_INPUT_FilePath.c_str()));
				return( false );
			}
		}
	}

	// Delete old area output file if exists
	if (SG_File_Exists(AREAD8_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(AREAD8_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), AREAD8_OUTPUT_FilePath.c_str()));
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

	// save weight grid
	if (WEIGHTGRID_INPUT_Grid != NULL)
	{
		if( !DataSet.Open_Write(WEIGHTGRID_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), WEIGHTGRID_INPUT_FilePath.c_str()));
			return( false );
		}
		DataSet.Write(0, WEIGHTGRID_INPUT_Grid);
	
		if( !DataSet.Close() )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), WEIGHTGRID_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// save outlet shapefile
	if (OUTLET_INPUTS_Shapes != NULL)
	{
		CSG_OGR_DataSource	DataSource;
		CSG_String OGRDriver = CSG_String("ESRI Shapefile");
		if( !DataSource.Create(OUTLET_INPUT_FilePath, OGRDriver) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), OUTLET_INPUT_FilePath.c_str()));
			return( false );
		}
		DataSource.Write(OUTLET_INPUTS_Shapes, OGRDriver);
	}


	// Run TauDEM D8ContributingArea

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

	// Open new tif file for area dir
	if( !DataSet.Open_Read(AREAD8_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated flow file: "), AREAD8_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		AREAD8_OUTPUT_Grid->Assign(DataSet.Read(0));
		AREAD8_OUTPUT_Grid->Set_Name(AreaName);
		Parameters("AREAD8_OUTPUT")->Set_Value(AREAD8_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FLOWD8_INPUT_Grid, colors);
		DataObject_Set_Colors(AREAD8_OUTPUT_Grid, colors);
		DataObject_Update(AREAD8_OUTPUT_Grid, false);		
	}

	return( true );
}

