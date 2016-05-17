/**********************************************************
 * Version $Id: stream_power_model.h 1925 2014-01-09 12:15:18Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       contrib_uoa_eresearch                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                      stream_power_model.h                      //
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
#ifndef HEADER_INCLUDED__stream_power_model_H
#define HEADER_INCLUDED__stream_power_model_H

//---------------------------------------------------------
#include "MLB_Interface.h"
#include "streampower.h"

class Cstream_power_model : public CSG_Module
{
public:
	Cstream_power_model(void);
	virtual ~Cstream_power_model(void);
	virtual CSG_String	Get_MenuPath	(void)	{	return( _TL("Stream Power Model") );	}

protected:

	CSG_Parameters_Grid_Target		m_Grid_Target;
	CSG_Grid *input, *u_grid_input, *k_grid_input, *output;
	double u_scalar_input, k_scalar_input;
	std::vector<std::vector<double>> GridToVector(CSG_Grid* grid);
	void VectorToGrid(std::vector<std::vector<double>> arr, CSG_Grid* grid);
	bool ExportGrid(CSG_Grid* grid, CSG_String path);

	virtual int						On_Parameter_Changed	(CSG_Parameters *pParameters, CSG_Parameter *pParameter);
	virtual int						On_Parameters_Enable	(CSG_Parameters *pParameters, CSG_Parameter *pParameter);
	virtual bool	On_Execute	(void);
};


#endif // #ifndef HEADER_INCLUDED__stream_power_model_H
