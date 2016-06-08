
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
//                     PeukerDouglas.cpp                 //
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

#include "PeukerDouglas.h"
#include "gdal_driver.h"

CPeukerDouglas::CPeukerDouglas(void)
{
	#ifdef _WIN32
		CSG_String UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		CSG_String UserHomeDir = CSG_String(getenv("HOME"));
	#endif

	CSG_String DefaultTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));

	// INFO
	Set_Name(_TL("Peuker Douglas"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(_TW("Creates an indicator grid (1,0) of upward curved grid cells according to the Peuker and Douglas algorithm.\n\nWith this tool, the DEM is first smoothed by a kernel with weights at the center, sides, and diagonals. The Peuker and Douglas (1975) method (also explained in Band, 1986), is then used to identify upwardly curving grid cells. This technique flags the entire grid, then examines in a single pass each quadrant of 4 grid cells, and unflags the highest. The remaining flagged cells are deemed 'upwardly curved', and when viewed, resemble a channel network. This proto-channel network generally lacks connectivity and requires thinning, issues that were discussed in detail by Band (1986).\n\nBand, L. E., (1986), \"Topographic partition of watersheds with digital elevation models,\" Water Resources Research, 22(1): 15-24.\n\nPeuker, T. K. and D. H. Douglas, (1975), \"Detection of surface-specific points by local parallel processing of discrete terrain elevation data,\" Comput. Graphics Image Process., 4: 375-387."));

	CSG_Parameter	*pNode;

	// GRIDS
	Parameters.Add_Grid(NULL, "FEL_INPUT"	, _TL("Input Elevation"), _TL("A grid of elevation values. This is usually the output of the \"Pit Remove\" tool, in which case it is elevations with pits removed."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "NETWORK_OUTPUT", _TL("Output Stream Source"), _TL("An indicator grid (1,0) of upward curved grid cells according to the Peuker and Douglas algorithm, and if viewed, resembles a channel network. This proto-channel network generally lacks connectivity and requires thinning, issues that were discussed in detail by Band (1986)."), PARAMETER_OUTPUT);
	
	pNode	= Parameters.Add_Node(NULL, "NODE_WEIGHTS", _TL("Weights"), _TL("Weights for smoothing input elevations"));

	//Values
	Parameters.Add_Value(pNode, "WEIGHT_MIDDLE"	, _TL("Center Smoothing Weight"),
		_TL("The center weight parameter used by a kernel to smooth the DEM before the tool identifies upwardly curved grid cells."), PARAMETER_TYPE_Double, 0.4, 0.0, true, 1.0, true);
	Parameters.Add_Value(pNode, "WEIGHT_SIDE"	, _TL("Side Smoothing Weight"),
		_TL("The side weight parameter used by a kernel to smooth the DEM before the tool identifies upwardly curved grid cells."), PARAMETER_TYPE_Double, 0.1, 0.0, true, 1.0, true);
	Parameters.Add_Value(pNode, "WEIGHT_DIAGONAL"	, _TL("Diagonal Smoothing Weight"),
		_TL("The diagonal weight parameter used by a kernel to smooth the DEM before the tool identifies upwardly curved grid cells."), PARAMETER_TYPE_Double, 0.05, 0.0, true, 1.0, true);


	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);
	
	// Other
	Parameters.Add_FilePath(NULL, "TEMP_DIR", _TL("Temp File Directory"), _TL("Directory used for storing temporary files during processing."), NULL, DefaultTempDir, false, true, false); 
	
}


bool CPeukerDouglas::On_Execute(void)
{
	TSG_Data_Type Type;
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	CSG_String FEL_INPUT_FileName, FEL_INPUT_FilePath;
	CSG_String NETWORK_OUTPUT_FileName, NETWORK_OUTPUT_FilePath;
	CSG_String Driver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile, NetworkName;
	CSG_Projection			Projection;
	CSG_GDAL_DataSet		DataSet;
	CSG_Grid *FEL_INPUT_Grid, *NETWORK_OUTPUT_Grid;
	double wmid, wside, wdiag;
	int nproc;

	FEL_INPUT_Grid = Parameters("FEL_INPUT")->asGrid();
	NETWORK_OUTPUT_Grid = Parameters("NETWORK_OUTPUT")->asGrid();

	// weights
	wmid = Parameters("WEIGHT_MIDDLE")->asDouble();
	wside = Parameters("WEIGHT_SIDE")->asDouble();
	wdiag = Parameters("WEIGHT_DIAGONAL")->asDouble();
	nproc = Parameters("NPROC")->asInt();

	Driver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = FEL_INPUT_Grid->Get_Type();

	//TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));
	TempDirPath = Parameters("TEMP_DIR")->asFilePath()->asString();

	FEL_INPUT_FileName = InputBasename + CSG_String("fel");
	FEL_INPUT_FilePath = SG_File_Make_Path(TempDirPath, FEL_INPUT_FileName, CSG_String("tif")); 

	NETWORK_OUTPUT_FileName = OutputBasename + CSG_String("ss");
	NETWORK_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, NETWORK_OUTPUT_FileName, CSG_String("tif"));

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	BinaryName = CSG_String("PeukerDouglas"); 
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("\"mpiexec -n %d \"%s\" -fel \"%s\" -ss \"%s\" -par %f %f %f > \"%s\" 2>&1\""), nproc, BinaryPath.c_str(), FEL_INPUT_FilePath.c_str(), NETWORK_OUTPUT_FilePath.c_str(), wmid, wside, wdiag, LogFile.c_str());
	NetworkName = CSG_String("Stream Source");


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

	// Delete old output file if exists
	if (SG_File_Exists(NETWORK_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(NETWORK_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), NETWORK_OUTPUT_FilePath.c_str()));
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

	// Run TauDEM PeukerDouglas

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

	// Open new tif file for flow dir
	if( !DataSet.Open_Read(NETWORK_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated flow file: "), NETWORK_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else 
	{
		NETWORK_OUTPUT_Grid->Assign(DataSet.Read(0));
		NETWORK_OUTPUT_Grid->Set_Name(NetworkName);
		Parameters("NETWORK_OUTPUT")->Set_Value(NETWORK_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(FEL_INPUT_Grid, colors);
		DataObject_Set_Colors(NETWORK_OUTPUT_Grid, colors);
		DataObject_Update(NETWORK_OUTPUT_Grid, false);		
	}

	return( true );
}
