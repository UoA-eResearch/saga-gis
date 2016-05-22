/**********************************************************
 * Version $Id$
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       contrib_gcd                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     dodthresholdprobsc.cpp                     //
//                                                       //
//                 Copyright (C) 2007 by                 //
//                        Author                         //
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
//-------------------------------------------------------//
//                                                       //
//    e-mail:     author@email.net                       //
//                                                       //
//    contact:    Author                                 //
//                Sesame Street. 7                       //
//                12345 Metropolis                       //
//                Nirvana                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "dodthresholdprobsc.h"



///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
Cdodthresholdprobsc::Cdodthresholdprobsc(void)
{
	#ifdef _WIN32
		CSG_String UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		CSG_String UserHomeDir = CSG_String(getenv("HOME"));
	#endif
	CSG_String DefaultTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));


	// Module info
	Set_Name		(_TL("Probabilistic Threshold with Spatial Coherence"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Threshold a DoD using probabilistic thresholding and spatial coherence"));

	// GCD setup
	GCDBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("GCD"));
	GCDBinDir = SG_File_Get_Path_Absolute(GCDBinDir);
	GCD = SG_File_Make_Path(GCDBinDir, CSG_String("gcd"), CSG_String("exe"));
	GCD_CMD = CSG_String("dodthresholdprobsc");

	// Logging
	LogOutput = SG_File_Make_Path(GCDBinDir, CSG_String("out"), CSG_String("txt"));
	LogError = SG_File_Make_Path(GCDBinDir, CSG_String("error"), CSG_String("txt"));;

	// Grids
	Parameters.Add_Grid(NULL, "DOD_INPUT"	, _TL("DoD"), _TL("Raster to be used as DoD"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "NEW_SURVEYERROR"	, _TL("New Survey Error"), _TL("Error raster for new survey"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "OLD_SURVEYERROR"	, _TL("Old Survey Error"), _TL("Error raster for old raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "CONDITIONAL_INPUT"	, _TL("Conditional"), _TL("Conditional raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "EROSION_INPUT"	, _TL("Erosion"), _TL("Erosion raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "DEPOSITION_INPUT"	, _TL("Deposition"), _TL("Deposition raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "THRESHOLD_OUTPUT", _TL("Threshold Output"), _TL("Output threshold raster"), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "PRIORPROB_OUTPUT", _TL("Prior Probability Output"), _TL("Output prior probability raster"), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "POSTPROB_OUTPUT", _TL("Post Probability Output"), _TL("Output post probability raster"), PARAMETER_OUTPUT);
	Parameters.Add_Value(NULL, "CELL_WIDTH", _TL("Cell Width"), _TL("Moving window cell width"), PARAMETER_TYPE_Int, 10, 0, true);
	Parameters.Add_Value(NULL, "CELL_HEIGHT", _TL("Cell Height"), _TL("Moving window cell height"), PARAMETER_TYPE_Int, 10, 0, true);
	Parameters.Add_Value(NULL, "THRESHOLD_VALUE", _TL("Probability Threshold"), _TL("Probability threshold"), PARAMETER_TYPE_Double, 0.5, 0, true, 1, true);

	//Other
	Parameters.Add_FilePath(NULL, "TEMP_DIR", _TL("Temp File Directory"), _TL("Directory used for storing temporary files during processing."), NULL, DefaultTempDir, false, true, false); 

	//GDAL
	GDALDriver = CSG_String("GTiff");
	GDALOptions = CSG_String(""); 
	Get_Projection(Projection);

}

///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool Cdodthresholdprobsc::On_Execute(void)
{

	if (!GetParameterValues())
	{
		return false;
	}
	CSG_String TempDirPath = Parameters("TEMP_DIR")->asFilePath()->asString();

	DoD_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("dodinput"), CSG_String("tif"));
	NewSurveyError_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("newsurveyerror"), CSG_String("tif"));
	OldSurveyError_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("oldsurveyerror"), CSG_String("tif"));
	Conditional_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("conditional"), CSG_String("tif"));
	Erosion_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("erosion"), CSG_String("tif"));
	Deposition_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("deposition"), CSG_String("tif"));
	Threshold_OutputPath = SG_File_Make_Path(TempDirPath, CSG_String("theshoutput"), CSG_String("tif"));
	PriorProb_OutputPath = SG_File_Make_Path(TempDirPath, CSG_String("priorproboutput"), CSG_String("tif"));
	PostProb_OutputPath = SG_File_Make_Path(TempDirPath, CSG_String("postproboutput"), CSG_String("tif"));



	// convert grids to tiffs for command input
	CSG_Grid* InputGrids [6] = {DoD_Input, NewSurveyError_Input, OldSurveyError_Input, Conditional_Input, Erosion_Input, Deposition_Input};

	CSG_Strings InputGridPaths = CSG_Strings();
	InputGridPaths.Add(NewSurveyError_InputPath);
	InputGridPaths.Add(OldSurveyError_InputPath);
	InputGridPaths.Add(Conditional_InputPath);
	InputGridPaths.Add(Erosion_InputPath);
	InputGridPaths.Add(Deposition_InputPath);

	if (!SaveGridsAsTIFF(InputGrids, InputGridPaths))
	{
		return false;
	}

	CSG_Strings OutputGridPaths = CSG_Strings();
	OutputGridPaths.Add(Threshold_OutputPath);
	OutputGridPaths.Add(PriorProb_OutputPath);
	OutputGridPaths.Add(PostProb_OutputPath);

	CSG_Strings OutputGridNames = CSG_Strings();
	OutputGridNames.Add("Threshold");
	OutputGridNames.Add("Prior Probability");
	OutputGridNames.Add("Post Probability");

	// delete old output files (GCD throws an error if a file already exists)
	if (!DeleteFiles(OutputGridPaths))
	{
		return false;
	}

	CSG_String CMD = CSG_String::Format(SG_T("%s %s %s %s %s %s %s %s %s %s %s %d %d %f >%s 2>%s"), GCD.c_str(), GCD_CMD.c_str(), DoD_InputPath.c_str(), Threshold_OutputPath.c_str(), NewSurveyError_InputPath.c_str(), OldSurveyError_InputPath.c_str(), PriorProb_OutputPath.c_str(), PostProb_OutputPath.c_str(), Conditional_InputPath.c_str(), Erosion_InputPath.c_str(), Deposition_InputPath.c_str(), CellWidth, CellHeight, ThresholdValue, LogOutput.c_str(), LogError.c_str());
	Message_Add(CSG_String("Executing: ") + CMD);			
	if (system(CMD.b_str()) != 0)
	{
		Message_Dlg(CSG_String::Format(SG_T("Error while executing %s, see Execution Log for details"), GCD_CMD.c_str()));
		DisplayLogs();
		return false;
	}	

	CSG_Grid* OutputGrids [3] = {Threshold_Output, PriorProb_Output, PostProb_Output};
	if (!LoadTIFFsAsGrids(OutputGridPaths, OutputGrids, OutputGridNames))
	{
		return false;
	}
	Parameters("THRESHOLD_OUTPUT")->Set_Value(Threshold_Output);
	Parameters("PRIORPROB_OUTPUT")->Set_Value(PriorProb_Output);
	Parameters("POSTPROB_OUTPUT")->Set_Value(PostProb_Output);

	ApplyColors(DoD_Input, Threshold_Output);
	ApplyColors(DoD_Input, PriorProb_Output);
	ApplyColors(DoD_Input, PostProb_Output);

	DisplayFile(LogOutput);
	return true;
}

bool Cdodthresholdprobsc::DeleteFiles(CSG_Strings paths)
{
	bool success = false;
	for (int i = 0; i < paths.Get_Count(); i++)
	{
		success = DeleteFile(paths.Get_String(i));
		if (!success)
		{
			return false;
		}
	}
	return success;
}

bool Cdodthresholdprobsc::DeleteFile(CSG_String path)
{
	// Delete file if exists
	if (SG_File_Exists(path))
	{
		if (!SG_File_Delete(path))
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to delete file: "), path.c_str()));
			return false;
		}
	}
	return true;
}

void Cdodthresholdprobsc::ApplyColors(CSG_Grid* from, CSG_Grid* to)
{
		CSG_Colors colors;
		DataObject_Get_Colors(from, colors);
		DataObject_Set_Colors(to, colors);
		DataObject_Update(to, false);	
}

bool Cdodthresholdprobsc::LoadTIFFsAsGrids(CSG_Strings tiffpaths, CSG_Grid** grids, CSG_Strings names)
{
	bool success = false;
	for (int i = 0; i < tiffpaths.Get_Count(); i++)
	{
		success = LoadTIFFAsGrid(tiffpaths.Get_String(i), grids[i], names[i]);
		if (!success)
		{
			return false;
		}
	}
	return success;
}

bool Cdodthresholdprobsc::LoadTIFFAsGrid(CSG_String path, CSG_Grid* grid, CSG_String name)
{

	if( !GDALDataSet.Open_Read(path))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated file: "), path.c_str()));
		return false;
	} 
	else 
	{
		grid->Assign(GDALDataSet.Read(0));
		grid->Set_Name(name);	
		GDALDataSet.Close();
	}

	return true;
}

bool Cdodthresholdprobsc::SaveGridsAsTIFF(CSG_Grid** grids, CSG_Strings paths)
{
	TSG_Data_Type Type;
	CSG_String FilePath;
	CSG_Grid* Grid;

	// SAVE TIFFS
	for (int i = 0; i < paths.Get_Count(); i++)
	{
		 FilePath = paths[i];
		 Grid = grids[i];
		 Type = Grid->Get_Type();

		if( !GDALDataSet.Open_Write(FilePath, GDALDriver, GDALOptions, Type, 1, *Get_System(), Projection) )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), FilePath.c_str()));
			return( false );
		}
		GDALDataSet.Write(0, Grid);
		if( !GDALDataSet.Close() )
		{
			Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), FilePath.c_str()));
			return( false );
		}
	}

	return true;
}

bool Cdodthresholdprobsc::GetParameterValues()
{

	DoD_Input = Parameters("DOD_INPUT")->asGrid();
	NewSurveyError_Input = Parameters("NEW_SURVEYERROR")->asGrid();
	OldSurveyError_Input = Parameters("OLD_SURVEYERROR")->asGrid();
	Conditional_Input = Parameters("CONDITIONAL_INPUT")->asGrid();
	Erosion_Input = Parameters("EROSION_INPUT")->asGrid();
	Deposition_Input = Parameters("DEPOSITION_INPUT")->asGrid();
	Threshold_Output = Parameters("THRESHOLD_OUTPUT")->asGrid();
	PriorProb_Output = Parameters("PRIORPROB_OUTPUT")->asGrid();
	PostProb_Output = Parameters("POSTPROB_OUTPUT")->asGrid(); 
	ThresholdValue = Parameters("THRESHOLD_VALUE")->asDouble();
	CellWidth = Parameters("CELL_WIDTH")->asInt();
	CellHeight = Parameters("CELL_HEIGHT")->asInt();

	return true;

}

void Cdodthresholdprobsc::DisplayLogs()
{
	DisplayFile(LogOutput);
	DisplayFile(LogError);
}

void Cdodthresholdprobsc::DisplayFile(CSG_String path)
{

	if (SG_File_Exists(path))
	{
		CSG_File File;
		if (File.Open(path, SG_FILE_R, false))
		{
			CSG_String Line;
			while (! File.is_EOF() && File.Read_Line(Line))
			{
				Message_Add(Line);
			}
			File.Close();
		} 
		else 
		{
			Message_Add(CSG_String("Unable to open " + path + CSG_String(" for reading")));
		}
	}
	else
	{
		Message_Add(CSG_String("File '" + path + CSG_String("' does not exist")));
	}

}

///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
