
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
//                     DInfContributingArea.cpp          //
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

#include "DInfContributingArea.h"
#include "gdal_driver.h"
#include "ogr_driver.h"

CDInfContributingArea::CDInfContributingArea(void)
{
	// INFO
	Set_Name(_TL("D-Infinity Contributing Area"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(CSG_String("Calculates a grid of specific catchment area which is the contributing area per unit contour length using the multiple flow direction D-infinity approach. D-infinity flow direction is defined as steepest downward slope on planar triangular facets on a block centered grid. The contribution at each grid cell is taken as the grid cell length (or when the optional weight grid input is used, from the weight grid). The contributing area of each grid cell is then taken as its own contribution plus the contribution from upslope neighbors that have some fraction draining to it according to the D-infinity flow model. The flow from each cell either all drains to one neighbor, if the angle falls along a cardinal (0, pi/2, pi, 3pi/2) or ordinal (pi/4, 3pi/4, 5pi/4, 7pi/4) direction, or is on an angle falling between the direct angle to two adjacent neighbors. In the latter case the flow is proportioned between these two neighbor cells according to how close the flow direction angle is to the direct angle to those cells. The contour length used here is the grid cell size. The resulting units of the specific catchment area are length units the same as those of the grid cell size.\n\nWhen the optional weight grid is not used, the result is reported in terms of specific catchment area, the upslope area per unit contour length, taken here as the number of cells times grid cell length (cell area divided by cell length). This assumes that grid cell length is the effective contour length, in the definition of specific catchment area and does not distinguish any difference in contour length dependent upon the flow direction. When the optional weight grid is used, the result is reported directly as a summation of weights, without any scaling.\n\nIf the optional outlet point shapefile is used, only the outlet cells and the cells upslope (by the D-infinity flow model) of them are in the domain to be evaluated.\n\nBy default, the tool checks for edge contamination. This is defined as the possibility that a contributing area value may be underestimated due to grid cells outside of the domain not being counted. This occurs when drainage is inwards from the boundaries or areas with \"no data\" values for elevation. The algorithm recognizes this and reports \"no data\" for the contributing area. It is common to see streaks of \"no data\" values extending inwards from boundaries along flow paths that enter the domain at a boundary. This is the desired effect and indicates that contributing area for these grid cells is unknown due to it being dependent on terrain outside of the domain of data available. Edge contamination checking may be turned off in cases where you know it is not an issue or want to ignore these problems, if for example, the DEM has been clipped along a watershed outline."));

	// GRIDS
	Parameters.Add_Grid(NULL, "FLOWDINF_INPUT", _TL("Input D-Infinity Flow Direction"), _TL("A grid of flow directions based on the D-infinity flow method using the steepest slope of a triangular facet (Tarboton, 1997, \"A New Method for the Determination of Flow Directions and Contributing Areas in Grid Digital Elevation Models,\" Water Resources Research, 33(2): 309-319). Flow direction is determined as the direction of the steepest downward slope on the 8 triangular facets of a 3 x 3 block centered grid. Flow direction is encoded as an angle in radians, counter-clockwise from east as a continuous (floating point) quantity between 0 and 2 pi. The resulting flow in a grid is then usually interpreted as being proportioned between the two neighboring cells that define the triangular facet with the steepest downward slope."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "AREADINF_OUTPUT", _TL("Output D-Infinity Specific Catchment Area"), _TL("A grid of specific catchment area which is the contributing area per unit contour length using the multiple flow direction D-infinity approach. The contributing area of each grid cell is then taken as its own contribution plus the contribution from upslope neighbors that have some fraction draining to it according to the D-infinity flow model."), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "WEIGHTGRID_INPUT"	, _TL("Input Weight Grid (Optional)"), _TL("A grid giving contribution to flow for each cell. These contributions (also sometimes referred to as weights or loadings) are used in the contributing area accumulation. If this input file is not used, the result is reported in terms of specific catchment area (the upslope area per unit contour length) taken as the number of cells times grid cell length (cell area divided by cell length)."), PARAMETER_INPUT_OPTIONAL);
	
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


bool CDInfContributingArea::On_Execute(void)
{
	TSG_Data_Type Type;
	CSG_String FLOWDINF_INPUT_FileName, FLOWDINF_INPUT_FilePath;
	CSG_String AREADINF_OUTPUT_FileName, AREADINF_OUTPUT_FilePath;
	CSG_String OUTLET_INPUT_FileName, OUTLET_INPUT_FilePath;
	CSG_String WEIGHTGRID_INPUT_FileName, WEIGHTGRID_INPUT_FilePath;
	CSG_String InputBaseName = "input";
	CSG_String OutputBaseName = "output";
	CSG_String GDALDriver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile, AreaName;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	CSG_Grid *SLOPE_OUTPUT_Grid, *AREADINF_OUTPUT_Grid, *WEIGHTGRID_INPUT_Grid;
	CSG_Shapes *OUTLET_INPUT_Shapes;
	int	nproc;

	SLOPE_OUTPUT_Grid = Parameters("FLOWDINF_INPUT")->asGrid();
	AREADINF_OUTPUT_Grid = Parameters("AREADINF_OUTPUT")->asGrid();
	WEIGHTGRID_INPUT_Grid = Parameters("WEIGHTGRID_INPUT")->asGrid();
	OUTLET_INPUT_Shapes = Parameters("OUTLET_INPUT")->asShapes();
	nproc = Parameters("NPROC")->asInt();

	GDALDriver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = SLOPE_OUTPUT_Grid->Get_Type();

	TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));

	FLOWDINF_INPUT_FileName = InputBaseName + CSG_String("ang");
	FLOWDINF_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FLOWDINF_INPUT_FileName, CSG_String("tif")); 

	AREADINF_OUTPUT_FileName = OutputBaseName + CSG_String("sca");
	AREADINF_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, AREADINF_OUTPUT_FileName, CSG_String("tif"));

	OUTLET_INPUT_FileName = InputBaseName + CSG_String("o");
	OUTLET_INPUT_FilePath = SG_File_Make_Path(TempDirPath, OUTLET_INPUT_FileName, CSG_String("shp")); 

	WEIGHTGRID_INPUT_FileName = FLOWDINF_INPUT_FileName + CSG_String("wg");
	WEIGHTGRID_INPUT_FilePath = SG_File_Make_Path(TempDirPath, WEIGHTGRID_INPUT_FileName, CSG_String("tif")); 

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	// optional flags
	CSG_String OptionalFlags = CSG_String("");
	if (OUTLET_INPUT_Shapes != NULL)
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

	BinaryName = CSG_String("AreaDinf"); // DInf
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("mpiexec -n %d %s -ang %s -sca %s %s > %s 2>&1"), nproc, BinaryPath.c_str(), FLOWDINF_INPUT_FilePath.c_str(), AREADINF_OUTPUT_FilePath.c_str(), OptionalFlags.c_str(), LogFile.c_str());
	AreaName = CSG_String("D-Infinity Specific Catchment Area");

	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{
		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(FLOWDINF_INPUT_FilePath))
	{
		if (!SG_File_Delete(FLOWDINF_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), FLOWDINF_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	if (OUTLET_INPUT_Shapes != NULL)
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
	if (SG_File_Exists(AREADINF_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(AREADINF_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), AREADINF_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// save input file
	if( !DataSet.Open_Write(FLOWDINF_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FLOWDINF_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, SLOPE_OUTPUT_Grid);
	
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FLOWDINF_INPUT_FilePath.c_str()));
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
	if (OUTLET_INPUT_Shapes != NULL)
	{
		CSG_OGR_DataSource	DataSource;
		CSG_String OGRDriver = CSG_String("ESRI Shapefile");
		if( !DataSource.Create(OUTLET_INPUT_FilePath, OGRDriver) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), OUTLET_INPUT_FilePath.c_str()));
			return( false );
		}
		DataSource.Write(OUTLET_INPUT_Shapes, OGRDriver);
	}


	// Run TauDEM DInfContributingArea

	Message_Add(CSG_String("Executing: ") + sCmd);

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
	if( !DataSet.Open_Read(AREADINF_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated flow file: "), AREADINF_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		AREADINF_OUTPUT_Grid->Assign(DataSet.Read(0));
		AREADINF_OUTPUT_Grid->Set_Name(AreaName);
		Parameters("AREADINF_OUTPUT")->Set_Value(AREADINF_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(SLOPE_OUTPUT_Grid, colors);
		DataObject_Set_Colors(AREADINF_OUTPUT_Grid, colors);
		DataObject_Update(AREADINF_OUTPUT_Grid, false);		
	}

	return( true );
}

