
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
//                      DInfFlowDirection.h                  //
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

#ifndef HEADER_INCLUDED__DInfFlowDirection_H
#define HEADER_INCLUDED__DInfFlowDirection_H

#include "MLB_Interface.h"

//---------------------------------------------------------
// Use the 'DInfFlowDirection_EXPORT' macro as defined in
// 'MLB_Interface.h' to export this class to allow other
// programs/libraries to use its functions:
//
// class DInfFlowDirection_EXPORT CDInfFlowDirection : public CSG_Module
// ...
//

class CDInfFlowDirection : public CSG_Module_Grid
{
public:
	CDInfFlowDirection(void);

	virtual CSG_String			Get_MenuPath	(void)	{	return( _TL("D-Infinity Flow Direction") );	}


protected:

	virtual bool				On_Execute		(void);


private:


};

#endif // #ifndef HEADER_INCLUDED__DInfFlowDirection_H
