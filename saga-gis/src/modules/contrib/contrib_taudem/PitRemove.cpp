
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
//                     PitRemove.cpp                     //
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

#include "PitRemove.h"
#include "gdal_driver.h"

CPitRemove::CPitRemove(void)
{
	// INFO
	Set_Name(_TL("Pit Remove"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(CSG_String("Identifies all pits in the DEM and raises their elevation to the level of the lowest pour point around their edge. Pits are low elevation areas in digital elevation models (DEMs) that are completely surrounded by higher terrain. They are generally taken to be artifacts that interfere with the routing of flow across DEMs, so are removed by raising their elevation to the point where they drain off the edge of the domain. The pour point is the lowest point on the boundary of the \"watershed\" draining to the pit. This step is not essential if you have reason to believe that the pits in your DEM are real. Also, if a few pits actually exist and so should not be removed, while at the same time others are believed to be artifacts that need to be removed, the actual pits should have \"no data\" elevation values inserted at their lowest point. \"No data\" values serve to define edges in the domain, and elevations are only raised to where flow is off an edge, so an internal \"no data\" value will stop a pit from being removed, if necessary."));

	// GRIDS
	Parameters.Add_Grid(NULL, "EL_INPUT"	, _TL("Input Elevation"), _TL("A digital elevation model (DEM) grid to serve as the base input for the terrain analysis and stream delineation.\n\nPits are generally assumed to be artifacts of the digitation process that interfere with the processing of flow across DEMs, and so are removed by raising their elevation to the point where they just drain. However, if a few actual pits are known, but others need to be removed, the actual pits should have \"no data\" elevation values inserted at their lowest point. \"No data\" values serve to define edges in the flow field, and elevations are only raised to where flow is off an edge, so an internal \"no data\" value will stop a pit from being removed, if necessary."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "FEL_OUTPUT", _TL("Output Pit Filled Elevation"), _TL("A grid of elevation values with pits removed so that flow is routed off of the domain. Pits are low elevation areas in digital elevation models (DEMs) that are completely surrounded by higher terrain. They are generally taken to be artifacts of the digitation process that interfere with the processing of flow across DEMs. So, they are removed by raising their elevation to the point where they just drain."), PARAMETER_OUTPUT);

	//Values
	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);

}


bool CPitRemove::On_Execute(void)
{
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	TSG_Data_Type Type;
	CSG_String Driver, Options;
	CSG_String EL_INPUT_FileName, EL_INPUT_FilePath;
	CSG_String FEL_OUTPUT_FileName, FEL_OUTPUT_FilePath, OutputName;
	CSG_String sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	CSG_Grid *EL_INPUT_Grid, *FEL_OUTPUT_Grid;
	int nproc;
	
	EL_INPUT_Grid = Parameters("EL_INPUT")->asGrid();
	FEL_OUTPUT_Grid = Parameters("FEL_OUTPUT")->asGrid();
	nproc = Parameters("NPROC")->asInt();

	Driver = CSG_String("GTiff");
	Options = CSG_String(""); 
	Get_Projection(Projection);
	Type = EL_INPUT_Grid->Get_Type();

	TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));

	EL_INPUT_FileName = InputBasename;
	EL_INPUT_FilePath = SG_File_Make_Path(TempDirPath, EL_INPUT_FileName, CSG_String("tif")); 

	FEL_OUTPUT_FileName = OutputBasename + CSG_String("fel");
	FEL_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, FEL_OUTPUT_FileName, CSG_String("tif"));
	OutputName = CSG_String("Pit Filled Elevation");


	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	BinaryName = CSG_String("PitRemove");
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	BinaryPath = SG_File_Get_Path_Absolute(BinaryPath);
	
	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{

		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(EL_INPUT_FilePath))
	{
		if (!SG_File_Delete(EL_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), EL_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old output file if exists
	if (SG_File_Exists(FEL_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(FEL_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), FEL_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// save input dem file
	if( !DataSet.Open_Write(EL_INPUT_FilePath, Driver, Options, Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), EL_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, EL_INPUT_Grid);
	
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), EL_INPUT_FilePath.c_str()));
		return( false );
	}

	sCmd = CSG_String::Format(SG_T("mpiexec -n %d %s -z %s -fel %s > %s 2>&1"), nproc, BinaryPath.c_str(), EL_INPUT_FilePath.c_str(), FEL_OUTPUT_FilePath.c_str(), LogFile.c_str());
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

	// Open new tif file
	if( !DataSet.Open_Read(FEL_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated file: "), FEL_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		FEL_OUTPUT_Grid->Assign(DataSet.Read(0));
		FEL_OUTPUT_Grid->Set_Name(OutputName);
		Parameters("FEL_OUTPUT")->Set_Value(FEL_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(EL_INPUT_Grid, colors);
		DataObject_Set_Colors(FEL_OUTPUT_Grid, colors);
		DataObject_Update(FEL_OUTPUT_Grid, false);		
	}

	return( true );
}
