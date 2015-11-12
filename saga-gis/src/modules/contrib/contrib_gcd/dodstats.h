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
#ifndef HEADER_INCLUDED__dodstats_H
#define HEADER_INCLUDED__dodstats_H

//---------------------------------------------------------
#include "MLB_Interface.h"
#include "gdal_driver.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
// Use the 'dodstats_EXPORT' macro as defined in
// 'MLB_Interface.h' to export this class to allow other
// programs/libraries to use its functions:
//
// class dodstats_EXPORT Cdodstats : public CSG_Module
// ...
//

class Cdodstats : public CSG_Module_Grid
{
public:
	Cdodstats(void);

	virtual CSG_String			Get_MenuPath	(void)	{	return( _TL("Statistics") );	}


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
	CSG_Grid* DoD_Input;
	CSG_String DoD_InputPath;

	// TABLES
	CSG_Table* DoDStatsTable;

	// GDAL
	TSG_Data_Type Type;
	CSG_String GDALDriver, GDALOptions;
	CSG_Projection Projection;
	CSG_GDAL_DataSet GDALDataSet;

	void DisplayLogs(void);
	void DisplayFile(CSG_String path);
	bool GetParameterValues(void);
	bool SaveGridsAsTIFF(CSG_Grid** grid, CSG_Strings paths);
	bool CreateStatsTable(void);


};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__dodstats_H
