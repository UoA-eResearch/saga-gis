
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
//                      D8ContributingArea.h             //
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

//---------------------------------------------------------

#ifndef HEADER_INCLUDED__D8ContributingArea_H
#define HEADER_INCLUDED__D8ContributingArea_H

#include "MLB_Interface.h"

//---------------------------------------------------------
// Use the 'D8ContributingArea_EXPORT' macro as defined in
// 'MLB_Interface.h' to export this class to allow other
// programs/libraries to use its functions:
//
// class D8ContributingArea_EXPORT CD8ContributingArea : public CSG_Module
// ...
//

class CD8ContributingArea : public CSG_Module_Grid
{
public:
	CD8ContributingArea(void);

	virtual CSG_String			Get_MenuPath	(void)	{	return( _TL("D8 Contributing Area") );	}


protected:

	virtual bool				On_Execute		(void);


private:


};

#endif // #ifndef HEADER_INCLUDED__D8ContributingArea_H
