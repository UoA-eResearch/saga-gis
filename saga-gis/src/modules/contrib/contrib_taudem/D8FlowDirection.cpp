
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                    contrib_taudem                     //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     D8FlowDirection.cpp               //
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
//                                                       //
///////////////////////////////////////////////////////////

#include "D8FlowDirection.h"
#include "gdal_driver.h"

CD8FlowDirection::CD8FlowDirection(void)
{
	// INFO
	Set_Name(_TL("D8 Flow Direction"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(_TW("Creates 2 grids. The first contains the flow direction from each grid cell to one of its adjacent or diagonal neighbors, calculated using the direction of steepest descent. The second contain the slope, as evaluated in the direction of steepest descent, and is reported as drop/distance, i.e. tan of the angle. Flow direction is reported as \"no data\" for any grid cell adjacent to the edge of the DEM domain, or adjacent to a \"no data\" value in the DEM. In flat areas, flow directions are assigned away from higher ground and towards lower ground using the method of Garbrecht and Martz (1997). The D8 flow direction algorithm may be applied to a DEM that has not had its pits filled, but it will then result in \"no data\" values for flow direction and slope at the lowest point of each pit.\n\nD8 Flow Direction Coding: 1 - East, 2 - Northeast, 3 - North, 4 - Northwest , 5 - West, 6 - Southwest, 7 - South, 8 - Southeast."));

	// GRIDS
	Parameters.Add_Grid(NULL, "FEL_INPUT"	, _TL("Input Pit Filled Elevation"), _TL("A grid of elevation values. This is usually the output of the \"Pit Remove\" tool, in which case it is elevations with pits removed. Pits are low elevation areas in digital elevation models (DEMs) that are completely surrounded by higher terrain. They are generally taken to be artifacts of the digitation process that interfere with the processing of flow across DEMs. So they are removed by raising their elevation to the point where they just drain off the domain. This step is not essential if you have reason to believe that the pits in your DEM are real. Also, if a few pits actually exist and so should not be removed, while at the same time others are believed to be artifacts that need to be removed, the actual pits should have \"no data\" elevation values inserted at their lowest point. \"No data\" values serve to define edges of the domain in the flow field, and elevations are only raised to where flow is off an edge, so an internal \"no data\" value will stop a pit from being removed, if necessary."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "FLOW_OUTPUT", _TL("Output D8 Flow Direction"), _TL("A grid of D8 flow directions which are defined, for each cell, as the direction of the one of its eight adjacent or diagonal neighbors with the steepest downward slope.\n\nFlow Direction Coding: 1 -East, 2 - Northeast, 3 - North, 4 - Northwest, 5 - West, 6 - Southwest, 7 - South, 8 - Southeast.\n\nThe flow direction routing across flat areas is performed according to the method described by Garbrecht, J. and L. W. Martz, (1997), \"The Assignment of Drainage Direction Over Flat Surfaces in Raster Digital Elevation Models,\" Journal of Hydrology, 193: 204-213."), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "SLOPE_OUTPUT", _TL("Output D8 Slope"), _TL("A grid giving slope in the D8 flow direction. This is measured as drop/distance."), PARAMETER_OUTPUT);

	//Values
	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);


}


bool CD8FlowDirection::On_Execute(void)
{
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	TSG_Data_Type Type;
	CSG_String FEL_INPUT_FileName, FEL_INPUT_FilePath;
	CSG_String FLOW_OUTPUT_FileName, FLOW_OUTPUT_FilePath;
	CSG_String SLOPE_OUTPUT_FileName, SLOPE_OUTPUT_FilePath;
	CSG_String Driver, Options, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile;
	CSG_String FlowName, SlopeName;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	CSG_Grid *FEL_INPUT_Grid, *FLOW_OUTPUT_Grid, *SLOPE_OUTPUT_Grid;
	int	nproc;

	FEL_INPUT_Grid = Parameters("FEL_INPUT")->asGrid();
	FLOW_OUTPUT_Grid = Parameters("FLOW_OUTPUT")->asGrid();
	SLOPE_OUTPUT_Grid = Parameters("SLOPE_OUTPUT")->asGrid();
	nproc = Parameters("NPROC")->asInt();

	Driver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = FEL_INPUT_Grid->Get_Type();
	Options = CSG_String(""); 

	TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));

	FEL_INPUT_FileName = InputBasename + CSG_String("fel");
	FEL_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FEL_INPUT_FileName, CSG_String("tif")); 

	FLOW_OUTPUT_FileName = OutputBasename + CSG_String("p");
	FLOW_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, FLOW_OUTPUT_FileName, CSG_String("tif"));

	SLOPE_OUTPUT_FileName = OutputBasename + CSG_String("sd8");
	SLOPE_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, SLOPE_OUTPUT_FileName, CSG_String("tif"));

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);
		
	BinaryName = CSG_String("D8FlowDir"); // D8
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("mpiexec -n %d %s -p %s -sd8 %s -fel %s > %s 2>&1"), nproc, BinaryPath.c_str(), FLOW_OUTPUT_FilePath.c_str(), SLOPE_OUTPUT_FilePath.c_str(), FEL_INPUT_FilePath.c_str(), LogFile.c_str());
	FlowName = CSG_String("D8 Flow Direction");
	SlopeName = CSG_String("D8 Slope");

	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{
		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(FEL_INPUT_FilePath))
	{
		if (!SG_File_Delete(FEL_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), FEL_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old flow output file if exists
	if (SG_File_Exists(FLOW_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(FLOW_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), FLOW_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old slope output file if exists
	if (SG_File_Exists(SLOPE_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(SLOPE_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), SLOPE_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// save input file
	if( !DataSet.Open_Write(FEL_INPUT_FilePath, Driver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FEL_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, FEL_INPUT_Grid);
	
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FEL_INPUT_FilePath.c_str()));
		return( false );
	}

	// Run TauDEM D8FlowDirection

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

	// Open new tif file for flow dir
	if( !DataSet.Open_Read(FLOW_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated flow file: "), FLOW_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		FLOW_OUTPUT_Grid->Assign(DataSet.Read(0));
		FLOW_OUTPUT_Grid->Set_Name(FlowName);
		Parameters("FLOW_OUTPUT")->Set_Value(FLOW_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FEL_INPUT_Grid, colors);
		DataObject_Set_Colors(FLOW_OUTPUT_Grid, 8, SG_COLORS_DEFAULT);
		DataObject_Update(FLOW_OUTPUT_Grid, false);		
	}


	// Open new tif file for slope dir
	if( !DataSet.Open_Read(SLOPE_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated slope file: "), SLOPE_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		SLOPE_OUTPUT_Grid->Assign(DataSet.Read(0));
		SLOPE_OUTPUT_Grid->Set_Name(SlopeName);
		Parameters("SLOPE_OUTPUT")->Set_Value(SLOPE_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FEL_INPUT_Grid, colors);
		DataObject_Set_Colors(SLOPE_OUTPUT_Grid, colors);
		DataObject_Update(SLOPE_OUTPUT_Grid, false);		
	}

	return( true );
}
