
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
//                   MLB_Interface.cpp                   //
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

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//			The Module Link Library Interface			 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
// 1. Include the appropriate SAGA-API header...

#include "MLB_Interface.h"


//---------------------------------------------------------
// 2. Place general module library informations here...

CSG_String Get_Info(int i)
{
	switch( i )
	{
	case MLB_INFO_Name:	default:
		return( _TL("TauDEM") );

	case MLB_INFO_Category:
		return( _TL("University of Auckland") );

	case MLB_INFO_Author:
		return( SG_T("Sina Masoud-Ansari") );

	case MLB_INFO_Description:
		return( _TL("Interface to the TauDEM suite of tools: http://hydrology.usu.edu/taudem/taudem5/index.html") );

	case MLB_INFO_Version:
		return( SG_T("1.0") );

	case MLB_INFO_Menu_Path:
		return( _TL("University of Auckland|TauDEM") );
	}
}


//---------------------------------------------------------
// 3. Include the headers of your modules here...

#include "PitRemove.h"
#include "D8FlowDirection.h"
#include "DInfFlowDirection.h"
#include "D8ContributingArea.h"
#include "DInfContributingArea.h"
#include "GridNet.h"
#include "PeukerDouglas.h"
#include "DropAnalysis.h"
#include "Threshold.h"
#include "StreamNet.h"


//---------------------------------------------------------
// 4. Allow your modules to be created here...

CSG_Module *		Create_Module(int i)
{
	// Don't forget to continuously enumerate the case switches
	// when adding new modules! Also bear in mind that the
	// enumeration always has to start with [case 0:] and
	// that [default:] must return NULL!...

	switch( i )
	{
	case 0:		return( new CPitRemove );
	case 1:		return( new CD8FlowDirection );
	case 2:		return( new CDInfFlowDirection );
	case 3:		return( new CD8ContributingArea );
	case 4:		return( new CDInfContributingArea );
	case 5:		return( new CGridNet );
	case 6:		return( new CPeukerDouglas );
	case 7:		return( new CDropAnalysis );
	case 8:		return( new CThreshold );
	case 9:		return( new CStreamNet );

	default:	return( NULL );
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
//{{AFX_SAGA

	MLB_INTERFACE

//}}AFX_SAGA
