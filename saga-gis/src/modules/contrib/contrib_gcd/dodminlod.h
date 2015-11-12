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
//                      dodstats.h                      //
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
//    e-mail:     author@email.de                        //
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
#ifndef HEADER_INCLUDED__dodminlod_H
#define HEADER_INCLUDED__dodminlod_H

//---------------------------------------------------------
#include "MLB_Interface.h"
#include "gdal_driver.h"


class Cdodminlod : public CSG_Module_Grid
{
public:
	Cdodminlod(void);

	virtual CSG_String			Get_MenuPath	(void)	{	return( _TL("DoD Minimum Level of Detection") );	}


protected:

	virtual bool				On_Execute		(void);

private:
	// GCD
	CSG_String GCDBinDir;
	CSG_String GCD;
	CSG_String GCD_CMD;

	// LOGGING
	CSG_String LogOutput;
	CSG_String LogError;

	// GRIDS
	CSG_Grid* NewDEM;
	CSG_Grid* OldDEM;
	CSG_Grid* RawDoD;
	CSG_Grid* ThresholdedDoD;

	CSG_String NewDEM_InputPath;
	CSG_String OldDEM_InputPath;
	CSG_String RawDoD_OutputPath;
	CSG_String ThresholdedDoD_OutputPath;

	// Parameters
	double MinLoD;

	// GDAL
	TSG_Data_Type Type;
	CSG_String GDALDriver, GDALOptions;
	CSG_Projection Projection;
	CSG_GDAL_DataSet GDALDataSet;

	void DisplayLogs(void);
	void DisplayFile(CSG_String path);
	void ApplyColors(CSG_Grid* from, CSG_Grid* to);
	bool GetParameterValues(void);
	bool SaveGridsAsTIFF(CSG_Grid** grids, CSG_Strings paths);
	bool LoadTIFFAsGrid(CSG_String tiffpath, CSG_Grid* grid, CSG_String name);
	bool LoadTIFFsAsGrids(CSG_Strings tiffpaths, CSG_Grid** grids, CSG_Strings names);
	bool DeleteFile(CSG_String path);
	bool DeleteFiles(CSG_Strings paths);

};


#endif 
