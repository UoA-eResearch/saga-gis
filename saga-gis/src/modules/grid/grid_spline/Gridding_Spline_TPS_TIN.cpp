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
//                     grid_spline                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//               Gridding_Spline_TPS_TIN.cpp             //
//                                                       //
//                 Copyright (C) 2006 by                 //
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
#include "Gridding_Spline_TPS_TIN.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGridding_Spline_TPS_TIN::CGridding_Spline_TPS_TIN(void)
{
	Set_Name		(_TL("Thin Plate Spline (TIN)"));

	Set_Author		("O.Conrad (c) 2006");

	Set_Description	(_TW(
		"Creates a 'Thin Plate Spline' function for each triangle of a TIN "
		"and uses it for subsequent gridding. The TIN is internally created "
		"from the scattered data points input. The 'Neighbourhood' option "
		"determines the number of points used for the spline generation. "
		"'Immediate neighbourhood' includes the points of the triangle as well as the "
		"immediate neighbour points. 'Level 1' adds the neighbours of the "
		"immediate neighbourhood and 'level 2' adds the neighbours of 'level 1' "
		"neighbours too. A higher neighbourhood degree reduces sharp breaks "
		"but also increases the computation time. "
		"\n\n"
		"References:\n"
		"- Donato G., Belongie S. (2002):"
		" 'Approximation Methods for Thin Plate Spline Mappings and Principal Warps',"
		" In Heyden, A., Sparr, G., Nielsen, M., Johansen, P. (Eds.):"
		" 'Computer Vision - ECCV 2002: 7th European Conference on Computer Vision, Copenhagen, Denmark, May 28-31, 2002',"
		" Proceedings, Part III, Lecture Notes in Computer Science."
		" Springer-Verlag Heidelberg; pp.21-31."
		"\n"
		"\n"
		"- Elonen, J. (2005):"
		" 'Thin Plate Spline editor - an example program in C++',"
		" <a target=\"_blank\" href=\"http://elonen.iki.fi/code/tpsdemo/index.html\">http://elonen.iki.fi/code/tpsdemo/index.html</a>."
		"\n"
	));

	//-----------------------------------------------------
	Parameters.Add_Value(
		NULL, "REGULARISATION"	, _TL("Regularisation"),
		_TL(""),
		PARAMETER_TYPE_Double, 0.0001, 0.0, true
	);

	//-----------------------------------------------------
	Parameters.Add_Choice(
		NULL	, "LEVEL"		, _TL("Neighbourhood"),
		_TL(""),

		CSG_String::Format(SG_T("%s|%s|%s|"),
			_TL("immediate"),
			_TL("level 1"),
			_TL("level 2")
		), 1
	);

	Parameters.Add_Value(
		NULL	, "FRAME"		, _TL("Add Frame"),
		_TL(""),
		PARAMETER_TYPE_Bool		, true
	);
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGridding_Spline_TPS_TIN::_Initialise(void)
{
	m_Regularisation	= Parameters("REGULARISATION")->asDouble();
	m_Level				= Parameters("LEVEL")->asInt();

	m_Points			= NULL;
	m_nPoints_Buf		= 0;

	return( CGridding_Spline_Base::On_Initialise() );
}

//---------------------------------------------------------
bool CGridding_Spline_TPS_TIN::_Finalise(void)
{
	if( m_Points )
	{
		SG_Free(m_Points);
	}

	m_Points		= NULL;
	m_nPoints_Buf	= 0;

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGridding_Spline_TPS_TIN::On_Execute(void)
{
	bool	bResult	= false;
	CSG_TIN	TIN;

	if( Initialise() && _Initialise() && _Get_TIN(TIN) )
	{
		for(int i=0; i<TIN.Get_Triangle_Count() && Set_Progress(i, TIN.Get_Triangle_Count()); i++)
		{
			_Set_Triangle(TIN.Get_Triangle(i));
		}

		_Finalise();

		bResult	= true;
	}

	return( bResult );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CGridding_Spline_TPS_TIN::_Set_Triangle(CSG_TIN_Triangle *pTriangle)
{
	if( m_pGrid->Get_Extent().Intersects(pTriangle->Get_Extent()) != INTERSECTION_None )
	{
		int		i, j;

		for(j=0, m_nPoints=0; j<3; j++)
		{
			CSG_TIN_Node	*pPoint = pTriangle->Get_Node(j);

			for(i=0; i<pPoint->Get_Neighbor_Count(); i++)
			{
				_Add_Points(pPoint->Get_Neighbor(i), 0);
			}
		}

		CSG_Thin_Plate_Spline	Spline;

		for(i=0; i<m_nPoints; i++)
		{
			CSG_TIN_Node	*pPoint = m_Points[i];

			Spline.Add_Point(pPoint->Get_Point().x, pPoint->Get_Point().y, pPoint->asDouble(0));
		}

		if( Spline.Create(m_Regularisation, true) )
		{
			_Set_Grid(pTriangle, Spline);
		}
	}
}

//---------------------------------------------------------
void CGridding_Spline_TPS_TIN::_Set_Grid(CSG_TIN_Triangle *pTriangle, CSG_Thin_Plate_Spline &Spline)
{
	int		ix, iy, ax, ay, bx, by;
	double	x, y, xMin, yMin;

	ax		= m_pGrid->Get_System().Get_xWorld_to_Grid(pTriangle->Get_Extent().Get_XMin());	if( ax < 0 )	ax	= 0;
	ay		= m_pGrid->Get_System().Get_yWorld_to_Grid(pTriangle->Get_Extent().Get_YMin());	if( ay < 0 )	ay	= 0;
	bx		= m_pGrid->Get_System().Get_xWorld_to_Grid(pTriangle->Get_Extent().Get_XMax());	if( bx >= m_pGrid->Get_NX() - 1 )	bx	= m_pGrid->Get_NX() - 2;
	by		= m_pGrid->Get_System().Get_yWorld_to_Grid(pTriangle->Get_Extent().Get_YMax());	if( by >= m_pGrid->Get_NY() - 1 )	by	= m_pGrid->Get_NY() - 2;
	xMin	= m_pGrid->Get_System().Get_xGrid_to_World(ax);
	yMin	= m_pGrid->Get_System().Get_yGrid_to_World(ay);

	for(iy=ay, y=yMin; iy<=by; iy++, y+=m_pGrid->Get_Cellsize())
	{
		for(ix=ax, x=xMin; ix<=bx; ix++, x+=m_pGrid->Get_Cellsize())
		{
			if( pTriangle->is_Containing(x, y) )
			{
				m_pGrid->Set_Value(ix, iy, Spline.Get_Value(x, y));
			}
		}
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CGridding_Spline_TPS_TIN::_Add_Points(CSG_TIN_Node *pPoint, int Level)
{
	_Add_Point(pPoint);

	if( Level < m_Level )
	{
		for(int j=0; j<pPoint->Get_Neighbor_Count(); j++)
		{
			CSG_TIN_Node	*p	= pPoint->Get_Neighbor(j);

			for(int i=0; i<p->Get_Neighbor_Count(); i++)
			{
				_Add_Points(p->Get_Neighbor(i), Level + 1);
			}
		}
	}
}

//---------------------------------------------------------
bool CGridding_Spline_TPS_TIN::_Add_Point(CSG_TIN_Node *pPoint)
{
	for(int i=0; i<m_nPoints; i++)
	{
		if( m_Points[i] == pPoint )
		{
			return( false );
		}
	}

	if( m_nPoints_Buf <= m_nPoints )
	{
		m_nPoints_Buf	+= 16;
		m_Points		= (CSG_TIN_Node **)SG_Realloc(m_Points, m_nPoints_Buf * sizeof(CSG_TIN_Node *));
	}

	m_Points[m_nPoints++]	= pPoint;

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGridding_Spline_TPS_TIN::_Get_TIN(CSG_TIN &TIN)
{
	TIN.Destroy();

	bool		bFrame		= Parameters("FRAME" )->asBool  ();
	int			zField		= Parameters("FIELD" )->asInt   ();
	CSG_Shapes	*pShapes	= Parameters("SHAPES")->asShapes();

	double	x[4], y[4], z[4], dMin[4];

	x[0]	= m_pGrid->Get_Extent().Get_XMin();	y[0]	= m_pGrid->Get_Extent().Get_YMin();	dMin[0]	= -1.0;
	x[1]	= m_pGrid->Get_Extent().Get_XMin();	y[1]	= m_pGrid->Get_Extent().Get_YMax();	dMin[1]	= -1.0;
	x[2]	= m_pGrid->Get_Extent().Get_XMax();	y[2]	= m_pGrid->Get_Extent().Get_YMax();	dMin[2]	= -1.0;
	x[3]	= m_pGrid->Get_Extent().Get_XMax();	y[3]	= m_pGrid->Get_Extent().Get_YMin();	dMin[3]	= -1.0;

	TIN.Add_Field("Z", SG_DATATYPE_Double);

	for (int iShape=0; iShape<pShapes->Get_Count() && Set_Progress(iShape, pShapes->Get_Count()); iShape++)
	{
		CSG_Shape	*pShape	= pShapes->Get_Shape(iShape);

		for(int iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
		{
			for(int iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
			{
				TSG_Point	p = pShape->Get_Point(iPoint, iPart);

				TIN.Add_Node(p, NULL, false)->Set_Value(0, pShape->asDouble(zField));

				if( bFrame )
				{
					for(int iCorner=0; iCorner<4; iCorner++)
					{
						double d	= SG_Get_Distance(p.x, p.y, x[iCorner], y[iCorner]);

						if( dMin[iCorner] < 0.0 || d < dMin[iCorner] )
						{
							dMin[iCorner]	= d;
							z   [iCorner]	= pShape->asDouble(zField);
						}
					}
				}
			}
		}
	}

	if( bFrame )
	{
		for(int iCorner=0; iCorner<4; iCorner++)
		{
			if( dMin[iCorner] >= 0.0 )
			{
				CSG_Point	p(x[iCorner], y[iCorner]);

				TIN.Add_Node(p, NULL, false)->Set_Value(0, z[iCorner]);
			}
		}
	}

	TIN.Update();

	return( TIN.Get_Triangle_Count() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
