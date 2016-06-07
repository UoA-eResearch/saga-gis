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
//                     uniformerror.cpp                     //
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
#include "uniformerror.h"



///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
Cuniformerror::Cuniformerror(void)
{

	#ifdef _WIN32
		CSG_String UserHomeDir = CSG_String(getenv("USERPROFILE"));
	#else
		CSG_String UserHomeDir = CSG_String(getenv("HOME"));
	#endif
	CSG_String DefaultTempDir = SG_File_Make_Path(UserHomeDir, CSG_String("Saga_GIS_tmp"));

	// Module info
	Set_Name		(_TL("Uniform Error Calculation"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Calculate a uniform error raster"));

	// GCD setup
	GCDBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("GCD"));
	GCDBinDir = SG_File_Get_Path_Absolute(GCDBinDir);
	GCD = SG_File_Make_Path(GCDBinDir, CSG_String("gcd"), CSG_String("exe"));
	GCD_CMD = CSG_String("uniformerror");

	// Grids
	Parameters.Add_Grid(NULL, "REFERENCE"	, _TL("Reference"), _TL("Raster to be used as coordinate system and NoData reference."), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "UNIFORM_ERROR", _TL("Uniform Error"), _TL("Output uniform error raster"), PARAMETER_OUTPUT);
	Parameters.Add_Value(NULL, "UNIFORM_ERROR_VALUE", _TL("Uniform Error Value"), _TL("Uniform error value for output raster"), PARAMETER_TYPE_Double, 0);

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
bool Cuniformerror::On_Execute(void)
{

	if (!GetParameterValues())
	{
		return false;
	}
	CSG_String TempDirPath = Parameters("TEMP_DIR")->asFilePath()->asString();

	LogOutput = SG_File_Make_Path(TempDirPath, CSG_String("out"), CSG_String("txt"));
	LogError = SG_File_Make_Path(TempDirPath, CSG_String("error"), CSG_String("txt"));;

	Reference_InputPath = SG_File_Make_Path(TempDirPath, CSG_String("reference"), CSG_String("tif"));
	UniformError_OutputPath = SG_File_Make_Path(TempDirPath, CSG_String("uniformerror"), CSG_String("tif"));

	// convert grids to tiffs for command input
	CSG_Grid* InputGrids [1] = {Reference};

	CSG_Strings InputGridPaths = CSG_Strings();
	InputGridPaths.Add(Reference_InputPath);

	if (!SaveGridsAsTIFF(InputGrids, InputGridPaths))
	{
		return false;
	}

	CSG_Strings OutputGridPaths = CSG_Strings();
	OutputGridPaths.Add(UniformError_OutputPath);

	CSG_Strings OutputGridNames = CSG_Strings();
	OutputGridNames.Add("Uniform Error");

	// delete old output files (GCD throws an error if a file already exists)
	if (!DeleteFiles(OutputGridPaths))
	{
		return false;
	}

	CSG_String CMD = CSG_String::Format(SG_T("\"\"%s\" %s \"%s\" \"%s\" %f >\"%s\" 2>\"%s\"\""), GCD.c_str(), GCD_CMD.c_str(), Reference_InputPath.c_str(), UniformError_OutputPath.c_str(), UniformErrorValue, LogOutput.c_str(), LogError.c_str());
	Message_Add(CSG_String("Executing: ") + CMD);			
	if (system(CMD.b_str()) != 0)
	{
		Message_Dlg(CSG_String::Format(SG_T("Error while executing %s, see Execution Log for details"), GCD_CMD.c_str()));
		DisplayLogs();
		return false;
	}	

	CSG_Grid* OutputGrids [1] = {UniformError};
	if (!LoadTIFFsAsGrids(OutputGridPaths, OutputGrids, OutputGridNames))
	{
		return false;
	}
	Parameters("UNIFORM_ERROR")->Set_Value(UniformError);

	ApplyColors(Reference, UniformError);

	DisplayFile(LogOutput);
	return true;
}

bool Cuniformerror::DeleteFiles(CSG_Strings paths)
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

bool Cuniformerror::DeleteFile(CSG_String path)
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

void Cuniformerror::ApplyColors(CSG_Grid* from, CSG_Grid* to)
{
		CSG_Colors colors;
		DataObject_Get_Colors(from, colors);
		DataObject_Set_Colors(to, colors);
		DataObject_Update(to, false);	
}

bool Cuniformerror::LoadTIFFsAsGrids(CSG_Strings tiffpaths, CSG_Grid** grids, CSG_Strings names)
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

bool Cuniformerror::LoadTIFFAsGrid(CSG_String path, CSG_Grid* grid, CSG_String name)
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

bool Cuniformerror::SaveGridsAsTIFF(CSG_Grid** grids, CSG_Strings paths)
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

bool Cuniformerror::GetParameterValues()
{

	Reference = Parameters("REFERENCE")->asGrid();
	UniformError = Parameters("UNIFORM_ERROR")->asGrid();
	UniformErrorValue = Parameters("UNIFORM_ERROR_VALUE")->asDouble();

	return true;

}

void Cuniformerror::DisplayLogs()
{
	DisplayFile(LogOutput);
	DisplayFile(LogError);
}

void Cuniformerror::DisplayFile(CSG_String path)
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
