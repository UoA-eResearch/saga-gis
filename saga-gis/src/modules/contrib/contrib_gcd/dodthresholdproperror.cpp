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
//                     dodthresholdproperror.cpp                     //
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
#include "dodthresholdproperror.h"



///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
Cdodthresholdproperror::Cdodthresholdproperror(void)
{
	// Module info
	Set_Name		(_TL("Propagated Error Threshold"));
	Set_Author		(SG_T("Sina Masoud-Ansari"));
	Set_Description	(_TW("Threshold a DoD using propagated error raster"));

	// GCD setup
	GCDBinDir = SG_File_Make_Path(CSG_String("bin"), CSG_String("GCD"));
	GCDBinDir = SG_File_Get_Path_Absolute(GCDBinDir);
	GCD = SG_File_Make_Path(GCDBinDir, CSG_String("gcd"), CSG_String("exe"));
	GCD_CMD = CSG_String("dodthresholdproperror");

	// Logging
	LogOutput = SG_File_Make_Path(GCDBinDir, CSG_String("out"), CSG_String("txt"));
	LogError = SG_File_Make_Path(GCDBinDir, CSG_String("error"), CSG_String("txt"));;

	// Grids
	Parameters.Add_Grid(NULL, "DOD_INPUT"	, _TL("DoD"), _TL("Raster to be used as DoD"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "DOD_PROPERROR"	, _TL("Propagated Error"), _TL("Propagated error raster"), PARAMETER_INPUT);
	Parameters.Add_Grid(NULL, "DOD_OUTPUT"	, _TL("Output"), _TL("Output raster"), PARAMETER_OUTPUT);

	DoD_InputPath = SG_File_Make_Path(GCDBinDir, CSG_String("dodinput"), CSG_String("tif"));
	DoD_PropErrorPath = SG_File_Make_Path(GCDBinDir, CSG_String("dodproperror"), CSG_String("tif"));
	DoD_OutputPath = SG_File_Make_Path(GCDBinDir, CSG_String("dodoutput"), CSG_String("tif"));

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
bool Cdodthresholdproperror::On_Execute(void)
{

	if (!GetParameterValues())
	{
		return false;
	}

	// convert grids to tiffs for command input
	CSG_Grid* InputGrids [2] = {DoD_Input, DoD_PropError};

	CSG_Strings InputGridPaths = CSG_Strings();
	InputGridPaths.Add(DoD_InputPath);
	InputGridPaths.Add(DoD_PropErrorPath);

	if (!SaveGridsAsTIFF(InputGrids, InputGridPaths))
	{
		return false;
	}

	// delete old output files (GCD throws an error if a file already exists)
	if (!DeleteFile(DoD_OutputPath))
	{
		return false;
	}

	CSG_String CMD = CSG_String::Format(SG_T("%s %s %s %s %s >%s 2>%s"), GCD.c_str(), GCD_CMD.c_str(), DoD_InputPath.c_str(), DoD_PropErrorPath.c_str(), DoD_OutputPath.c_str(),LogOutput.c_str(), LogError.c_str());
	Message_Add(CSG_String("Executing: ") + CMD);			
	if (system(CMD.b_str()) != 0)
	{
		Message_Dlg(CSG_String::Format(SG_T("Error while executing %s, see Execution Log for details"), GCD_CMD.c_str()));
		DisplayLogs();
		return false;
	}	

	if (!LoadTIFFAsGrid(DoD_OutputPath, DoD_Output))
	{
		return false;
	}
	ApplyColors(DoD_Input, DoD_Output);

	DisplayFile(LogOutput);
	return true;
}

bool Cdodthresholdproperror::DeleteFile(CSG_String path)
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

void Cdodthresholdproperror::ApplyColors(CSG_Grid* from, CSG_Grid* to)
{
		CSG_Colors colors;
		DataObject_Get_Colors(from, colors);
		DataObject_Set_Colors(to, colors);
		DataObject_Update(to, false);	
}

bool Cdodthresholdproperror::LoadTIFFAsGrid(CSG_String path, CSG_Grid* grid)
{

	if( !GDALDataSet.Open_Read(path))
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open generated file: "), path.c_str()));
		return false;
	} 
	else 
	{
		grid->Assign(GDALDataSet.Read(0));
		grid->Set_Name("DoD Output");
		Parameters("DOD_OUTPUT")->Set_Value(grid);	
		GDALDataSet.Close();
	}

	return true;
}

bool Cdodthresholdproperror::SaveGridsAsTIFF(CSG_Grid** grids, CSG_Strings paths)
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

bool Cdodthresholdproperror::GetParameterValues()
{

	DoD_Input = Parameters("DOD_INPUT")->asGrid();
	DoD_PropError = Parameters("DOD_PROPERROR")->asGrid();
	DoD_Output = Parameters("DOD_OUTPUT")->asGrid();

	return true;
}

void Cdodthresholdproperror::DisplayLogs()
{
	DisplayFile(LogOutput);
	DisplayFile(LogError);
}

void Cdodthresholdproperror::DisplayFile(CSG_String path)
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
