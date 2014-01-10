/**********************************************************
 * Version $Id: GEOTRANS_Shapes.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                  Projection_GeoTRANS                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                  GEOTRANS_Shapes.cpp                  //
//                                                       //
//                 Copyright (C) 2003 by                 //
//                      Olaf Conrad                      //
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
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "GEOTRANS_Shapes.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGEOTRANS_Shapes::CGEOTRANS_Shapes(void)
{
	//-----------------------------------------------------
	// 1. Info...

	Set_Name	(_TL("GeoTrans (Shapes)"));

	Set_Author		(SG_T("(c) 2003 by O.Conrad"));

	Set_Description	(_TW(
		"Coordinate Transformation for Shapes. "
		"This library makes use of the Geographic Translator (GeoTrans) library. "
		"\n"
		"GeoTrans is maintained by the National Geospatial Agency (NGA).\n"
		"  <a target=\"_blank\" href=\"http://earth-info.nga.mil/GandG/geotrans/\">"
		"  http://earth-info.nga.mil/GandG/geotrans/</a>\n"
	));


	//-----------------------------------------------------
	// 2. In-/Output...

	Parameters.Add_Shapes(
		Parameters("SOURCE_NODE"), "SOURCE", _TL("Source"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Shapes(
		Parameters("TARGET_NODE"), "TARGET", _TL("Target"),
		_TL(""),
		PARAMETER_OUTPUT
	);
}

//---------------------------------------------------------
CGEOTRANS_Shapes::~CGEOTRANS_Shapes(void)
{}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGEOTRANS_Shapes::On_Execute_Conversion(void)
{
	bool		bCopy, bDropped;
	int			iShape, iPart, iPoint, nDropped;
	TSG_Point	Point;
	CSG_Shape		*pShape_Source, *pShape_Target;
	CSG_Shapes		*pSource, *pTarget;

	//-----------------------------------------------------
	if( 1 )
	{
		pSource		= Parameters("SOURCE")->asShapes();
		pTarget		= Parameters("TARGET")->asShapes();

		if( pSource == pTarget )
		{
			bCopy		= true;

			pTarget		= SG_Create_Shapes();
		}
		else
		{
			bCopy		= false;
		}

		pTarget->Create(pSource->Get_Type(), pSource->Get_Name(), pSource);

		//-------------------------------------------------
		for(iShape=0, nDropped=0; iShape<pSource->Get_Count() && Set_Progress(iShape, pSource->Get_Count()); iShape++)
		{
			pShape_Source	= pSource->Get_Shape(iShape);
			pShape_Target	= pTarget->Add_Shape(pShape_Source, SHAPE_COPY_ATTR);

			for(iPart=0, bDropped=false; iPart<pShape_Source->Get_Part_Count() && !bDropped; iPart++)
			{
				for(iPoint=0; iPoint<pShape_Source->Get_Point_Count(iPart) && !bDropped; iPoint++)
				{
					Point	= pShape_Source->Get_Point(iPoint, iPart);

					if( Get_Converted(Point.x, Point.y) )
					{
						pShape_Target->Add_Point(Point.x, Point.y, iPart);
					}
					else
					{
						bDropped	= true;
					}
				}
			}

			if( bDropped )
			{
				nDropped++;
				pTarget->Del_Shape(pShape_Target);
			}
		}

		//-------------------------------------------------
		if( nDropped > 0 )
		{
			Message_Add(CSG_String::Format(SG_T("%s: %d"), _TL("number of dropped shapes"), nDropped));
		}

		if( bCopy )
		{
			pSource->Assign(pTarget);

			delete( pTarget );
		}

		return( true );
	}

	return( false );
}
