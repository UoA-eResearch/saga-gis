
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
//                     Threshold.cpp                     //
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

#include "Threshold.h"
#include "gdal_driver.h"

CThreshold::CThreshold(void)
{

	#ifdef _WIN32
		CSG_String UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		CSG_String UserHomeDir = CSG_String(getenv("HOME"));
	#endif
	
	CSG_String DefaultTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));

	// INFO
	Set_Name(_TL("Stream Definition by Threshold"));
	Set_Author(SG_T("S. Masoud-Ansari, J. Tunnicliffe, D. Tarboton"));
	Set_Description	(_TW("Operates on any grid and outputs an indicator (1,0) grid identifing cells with input values >= the threshold value. The standard use is to use an accumulated source area grid to as the input grid to generate a stream raster grid as the output. If you use the optional input mask grid, it limits the domain being evaluated to cells with mask values >= 0 . When you use a D-infinity contributing area grid (*sca) as the mask grid, it functions as an edge contamination mask. The threshold logic is: src = ((ssa >= thresh) & (mask >=0)) ? 1:0"));

	// GRIDS
	Parameters.Add_Grid(NULL, "SSA_INPUT"	, _TL("Input Accumulated Stream Source"), _TL("This grid nominally accumulates some characteristic or combination of characteristics of the watershed. The exact characteristic(s) varies depending on the stream network raster algorithm being used. This grid needs to have the property that grid cell values are monotonically increasing downslope along D8 flow directions, so that the resulting stream network is continuous. While this grid is often from an accumulation, other sources such as a maximum upslope function will also produce a suitable grid."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "MASK_INPUT"	, _TL("Input Mask (Optional)"), _TL("This optional input is a grid that is used to mask the domain of interest and output is only provided where this grid is >= 0. A common use of this input is to use a D-Infinity contributing area grid as the mask so that the delineated stream network is constrained to areas where D-infinity contributing area is available, replicating the functionality of an edge contamination mask."), PARAMETER_INPUT_OPTIONAL);
	Parameters.Add_Grid(NULL, "SRC_OUTPUT"	, _TL("Output Stream Raster"), _TL("This is an indicator grid (1,0) that indicates the location of streams, with a value of 1 for each of the stream cells and 0 for the remainder of the cells."), PARAMETER_OUTPUT);

	// VALUES
	Parameters.Add_Value(NULL, "THRESHOLD"	, _TL("Threshold"),
		_TL("This parameter is compared to the value in the Accumulated Stream Source grid (*ssa) to determine if the cell should be considered a stream cell. Streams are identified as grid cells for which ssa value is >= this threshold."), PARAMETER_TYPE_Double, 100.0);
	Parameters.Add_Value(NULL, "NPROC"	, _TL("Number of Processes"), _TL("The number of stripes that the domain will be divided into and the number of MPI parallel processes that will be spawned to evaluate each of the stripes"), PARAMETER_TYPE_Int, SG_Get_Max_Num_Procs_Omp(), 1, true, SG_Get_Max_Num_Procs_Omp(), true);

	// Other
	Parameters.Add_FilePath(NULL, "TEMP_DIR", _TL("Temp File Directory"), _TL("Directory used for storing temporary files during processing."), NULL, DefaultTempDir, false, true, false); 
	

}


bool CThreshold::On_Execute(void)
{
	// Inputs and Output Strings
	CSG_String InputBasename = CSG_String("input");
	CSG_String OutputBasename = CSG_String("output");
	CSG_String MASK_INPUT_FileName, MASK_INPUT_FilePath;
	CSG_String SSA_INPUT_FileName, SSA_INPUT_FilePath;
	CSG_String SRC_OUTPUT_FileName, SRC_OUTPUT_FilePath, SRC_OUTPUT_Name;

	// Data Objects
	CSG_Grid *SSA_INPUT_Grid, *MASK_INPUT_Grid, *SRC_OUTPUT_Grid;

	// Misc
	TSG_Data_Type Type;
	CSG_String GDALDriver, sCmd, TempDirPath, TauDEMBinDir, BinaryName, BinaryPath, LogFile;
	CSG_Projection Projection;
	CSG_GDAL_DataSet DataSet;
	double Threshold;
	int nproc;
	
	// Grab inputs
	SSA_INPUT_Grid = Parameters("SSA_INPUT")->asGrid();
	MASK_INPUT_Grid = Parameters("MASK_INPUT")->asGrid();
	SRC_OUTPUT_Grid = Parameters("SRC_OUTPUT")->asGrid();
	Threshold = Parameters("THRESHOLD")->asDouble();
	nproc = Parameters("NPROC")->asInt();

	GDALDriver = CSG_String("GTiff");
	Get_Projection(Projection);
	Type = SSA_INPUT_Grid->Get_Type();

	//TempDirPath = SG_File_Get_Path_Absolute(CSG_String("taudem_tmp"));
	TempDirPath = Parameters("TEMP_DIR")->asFilePath()->asString();

	SSA_INPUT_FileName = InputBasename + CSG_String("ssa");
	SSA_INPUT_FilePath = SG_File_Make_Path(TempDirPath, SSA_INPUT_FileName, CSG_String("tif"));

	MASK_INPUT_FileName = InputBasename + CSG_String("mask");
	MASK_INPUT_FilePath = SG_File_Make_Path(TempDirPath, MASK_INPUT_FileName, CSG_String("tif")); 

	SRC_OUTPUT_FileName = OutputBasename + CSG_String("src");
	SRC_OUTPUT_FilePath = SG_File_Make_Path(TempDirPath, SRC_OUTPUT_FileName, CSG_String("tif"));

	LogFile = SG_File_Make_Path(TempDirPath, CSG_String("taudem_log.txt"));
	LogFile = SG_File_Get_Path_Absolute(LogFile);

	TauDEMBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("TauDEM"));
	TauDEMBinDir = SG_File_Get_Path_Absolute(TauDEMBinDir);

	CSG_String OptionalFlags = CSG_String("");
	if (MASK_INPUT_Grid != NULL)
	{
		OptionalFlags = CSG_String::Format(SG_T("-mask \"%s\""), MASK_INPUT_FilePath.c_str());
	}

	// exec commnad
	BinaryName = CSG_String("Threshold"); // D8
	BinaryPath = SG_File_Make_Path(TauDEMBinDir, BinaryName);
	sCmd = CSG_String::Format(SG_T("\"mpiexec -n %d \"%s\" -ssa \"%s\" -src \"%s\" -thresh %f %s > \"%s\" 2>&1\""), nproc, BinaryPath.c_str(), SSA_INPUT_FilePath.c_str(), SRC_OUTPUT_FilePath.c_str(), Threshold, OptionalFlags.c_str(), LogFile.c_str());
	SRC_OUTPUT_Name = CSG_String("Stream Raster");

	// make sure temp dir exists
	if (!SG_Dir_Exists(TempDirPath))
	{
		if (!SG_Dir_Create(TempDirPath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to create temp directory"), TempDirPath.c_str()));
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(SSA_INPUT_FilePath))
	{
		if (!SG_File_Delete(SSA_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), SSA_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old input file if exists
	if (SG_File_Exists(MASK_INPUT_FilePath))
	{
		if (!SG_File_Delete(MASK_INPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous input file: "), MASK_INPUT_FilePath.c_str()));
			return( false );
		}
	}

	// Delete old output files
	if (SG_File_Exists(SRC_OUTPUT_FilePath))
	{
		if (!SG_File_Delete(SRC_OUTPUT_FilePath))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete previous output file: "), SRC_OUTPUT_FilePath.c_str()));
			return( false );
		}
	}

	// SAVE TIFFS

	// save ssa input
	if( !DataSet.Open_Write(SSA_INPUT_FilePath, GDALDriver, CSG_String(""), Type, 1, *Get_System(), Projection) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), SSA_INPUT_FilePath.c_str()));
		return( false );
	}
	DataSet.Write(0, SSA_INPUT_Grid);
	if( !DataSet.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), SSA_INPUT_FilePath.c_str()));
		return( false );
	}

	// save mask input
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

	// Run TauDEM Threshold
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

	if( !DataSet.Open_Read(SRC_OUTPUT_FilePath))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open output file: "), SRC_OUTPUT_FilePath.c_str()));
		return( false );
	} 
	else
	{
		SRC_OUTPUT_Grid->Assign(DataSet.Read(0));
		SRC_OUTPUT_Grid->Set_Name(SRC_OUTPUT_Name);
		Parameters("SRC_OUTPUT")->Set_Value(SRC_OUTPUT_Grid);

		CSG_Colors colors;
		DataObject_Get_Colors(SSA_INPUT_Grid, colors);
		DataObject_Set_Colors(SRC_OUTPUT_Grid, colors);
		DataObject_Update(SRC_OUTPUT_Grid, false);		
	}

	return( true );

}

