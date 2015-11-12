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
//                     dodproperror.cpp                     //
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
#include "dodproperror.h"



///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
Cdodproperror::Cdodproperror(void)
{
	// Module info
	Set_Name		(_TL("Propogated Error DoD"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Calculate a raw DEM and threshold it usnig propagated error"));

	// GCD setup
	GCDBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("GCD"));
	GCDBinDir = SG_File_Get_Path_Absolute(GCDBinDir);
	GCD = SG_File_Make_Path(GCDBinDir, CSG_String("gcd"), CSG_String("exe"));
	GCD_CMD = CSG_String("dodproperror");

	// Logging
	LogOutput = SG_File_Make_Path(GCDBinDir, CSG_String("out"), CSG_String("txt"));
	LogError = SG_File_Make_Path(GCDBinDir, CSG_String("error"), CSG_String("txt"));;

	// Grids
	Parameters.Add_Grid(NULL, "NEW_DEM"	, _TL("New DEM"), _TL("New DEM raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "OLD_DEM"	, _TL("Old DEM"), _TL("Old DEM raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "PROP_ERROR", _TL("Propagated Error"), _TL("Propagated Error"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "RAW_DOD", _TL("Raw DoD"), _TL("Raw DEM of difference"), PARAMETER_OUTPUT);
	Parameters.Add_Grid(NULL, "THRESHOLDED_DOD", _TL("Thresholded DoD"), _TL("Thresholded DEM of difference"), PARAMETER_OUTPUT);

	NewDEM_InputPath = SG_File_Make_Path(GCDBinDir, CSG_String("newdem"), CSG_String("tif"));
	OldDEM_InputPath = SG_File_Make_Path(GCDBinDir, CSG_String("olddem"), CSG_String("tif"));
	PropError_InputPath = SG_File_Make_Path(GCDBinDir, CSG_String("properror"), CSG_String("tif"));
	RawDoD_OutputPath = SG_File_Make_Path(GCDBinDir, CSG_String("rawdodoutput"), CSG_String("tif"));
	ThresholdedDoD_OutputPath = SG_File_Make_Path(GCDBinDir, CSG_String("threshdodoutput"), CSG_String("tif"));

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
bool Cdodproperror::On_Execute(void)
{

	if (!GetParameterValues())
	{
		return false;
	}

	// convert grids to tiffs for command input
	CSG_Grid* InputGrids [3] = {NewDEM, OldDEM, PropError};

	CSG_Strings InputGridPaths = CSG_Strings();
	InputGridPaths.Add(NewDEM_InputPath);
	InputGridPaths.Add(OldDEM_InputPath);
	InputGridPaths.Add(PropError_InputPath);

	if (!SaveGridsAsTIFF(InputGrids, InputGridPaths))
	{
		return false;
	}

	CSG_Strings OutputGridPaths = CSG_Strings();
	OutputGridPaths.Add(RawDoD_OutputPath);
	OutputGridPaths.Add(ThresholdedDoD_OutputPath);

	CSG_Strings OutputGridNames = CSG_Strings();
	OutputGridNames.Add("Raw DoD");
	OutputGridNames.Add("Thresholded DoD");

	// delete old output files (GCD throws an error if a file already exists)
	if (!DeleteFiles(OutputGridPaths))
	{
		return false;
	}

	CSG_String CMD = CSG_String::Format(SG_T("%s %s %s %s %s %s %s >%s 2>%s"), GCD.c_str(), GCD_CMD.c_str(), NewDEM_InputPath.c_str(), OldDEM_InputPath.c_str(), PropError_InputPath.c_str(), RawDoD_OutputPath.c_str(), ThresholdedDoD_OutputPath.c_str(), LogOutput.c_str(), LogError.c_str());
	Message_Add(CSG_String("Executing: ") + CMD);			
	if (system(CMD.b_str()) != 0)
	{
		Message_Dlg(CSG_String::Format(SG_T("Error while executing %s, see Execution Log for details"), GCD_CMD.c_str()));
		DisplayLogs();
		return false;
	}	

	CSG_Grid* OutputGrids [3] = {RawDoD, ThresholdedDoD};
	if (!LoadTIFFsAsGrids(OutputGridPaths, OutputGrids, OutputGridNames))
	{
		return false;
	}
	Parameters("RAW_DOD")->Set_Value(RawDoD);
	Parameters("THRESHOLDED_DOD")->Set_Value(ThresholdedDoD);

	ApplyColors(NewDEM, RawDoD);
	ApplyColors(NewDEM, ThresholdedDoD);

	DisplayFile(LogOutput);
	return true;
}

bool Cdodproperror::DeleteFiles(CSG_Strings paths)
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

bool Cdodproperror::DeleteFile(CSG_String path)
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

void Cdodproperror::ApplyColors(CSG_Grid* from, CSG_Grid* to)
{
		CSG_Colors colors;
		DataObject_Get_Colors(from, colors);
		DataObject_Set_Colors(to, colors);
		DataObject_Update(to, false);	
}

bool Cdodproperror::LoadTIFFsAsGrids(CSG_Strings tiffpaths, CSG_Grid** grids, CSG_Strings names)
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

bool Cdodproperror::LoadTIFFAsGrid(CSG_String path, CSG_Grid* grid, CSG_String name)
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

bool Cdodproperror::SaveGridsAsTIFF(CSG_Grid** grids, CSG_Strings paths)
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

bool Cdodproperror::GetParameterValues()
{

	NewDEM = Parameters("NEW_DEM")->asGrid();
	OldDEM = Parameters("OLD_DEM")->asGrid();
	RawDoD = Parameters("RAW_DOD")->asGrid();
	PropError = Parameters("PROP_ERROR")->asGrid();
	ThresholdedDoD = Parameters("THRESHOLDED_DOD")->asGrid();

	return true;

}

void Cdodproperror::DisplayLogs()
{
	DisplayFile(LogOutput);
	DisplayFile(LogError);
}

void Cdodproperror::DisplayFile(CSG_String path)
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
