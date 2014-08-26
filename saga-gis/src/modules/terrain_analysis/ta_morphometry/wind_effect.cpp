/**********************************************************
 * Version $Id: wind_effect.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                    ta_morphometry                     //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    wind_effect.cpp                    //
//                                                       //
//                 Copyright (C) 2008 by                 //
//                    Andre Ringeler                     //
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
//    e-mail:     aringeler@saga-gis.org                 //
//                                                       //
//    contact:    Andre Ringeler                         //
//                Institute of Geography                 //
//                University of Hamburg                  //
//                Bundesstr. 55                          //
//                20146 Hamburg                          //
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
#include "wind_effect.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CWind_Effect::CWind_Effect(void)
{
	CSG_Parameter	*pNode;

	Set_Name		(_TL("Wind Effect (Windward / Leeward Index)"));

	Set_Author		(SG_T("J.Boehner, A.Ringeler (c) 2008, O.Conrad (c) 2011"));

	Set_Description	(_TW(
		"The 'Wind Effect' is a dimensionless index. Values below 1 indicate wind shadowed areas "
		"whereas values above 1 indicate areas exposed to wind, all with regard to the specified "
		"wind direction. Wind direction, i.e. the direction into which the wind blows, might be "
		"either constant or variying in space, if a wind direction grid is supplied.\n"
		"\nReferences:\n"
		"- Boehner, J., Antonic, O. (2009): 'Land-surface parameters specific to topo-climatology'."
		" in: Hengl, T., Reuter, H. (Eds.): 'Geomorphometry - Concepts, Software, Applications'."
		" Developments in Soil Science, Volume 33, p.195-226, Elsevier.\n"
	));

	Parameters.Add_Grid(
		NULL	, "DEM"			, _TL("Elevation"),
		_TL(""),
		PARAMETER_INPUT
	);

	pNode	= Parameters.Add_Grid(
		NULL	, "DIR"			, _TL("Wind Direction"),
		_TL("Direction into which the wind blows, starting with 0 for North and increasing clockwise."),
		PARAMETER_INPUT_OPTIONAL
	);

	Parameters.Add_Choice(
		pNode	, "DIR_UNITS"	, _TL("Wind Direction Units"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("radians"),
			_TL("degree")
		), 0
	);

	pNode	= Parameters.Add_Grid(
		NULL	, "LEN"			, _TL("Wind Speed"),
		_TL(""),
		PARAMETER_INPUT_OPTIONAL
	);

	Parameters.Add_Value(
		pNode	, "LEN_SCALE"	, _TL("Scaling"),
		_TL(""),
		PARAMETER_TYPE_Double, 1.0
	);

	Parameters.Add_Grid(
		NULL	, "EFFECT"		, _TL("Wind Effect"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	Parameters.Add_Grid(
		NULL	, "AFH"			, _TL("Effective Air Flow Heights"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Value(
		NULL	, "MAXDIST"		, _TL("Search Distance [km]"),
		_TL(""),
		PARAMETER_TYPE_Double, 300.0, 0.0, true
	);

	pNode	= Parameters.Add_Value(
		NULL	, "DIR_CONST"	, _TL("Constant Wind Direction"),
		_TL("constant direction into the wind blows, given as degree"),
		PARAMETER_TYPE_Double, 135.0
	);

	Parameters.Add_Value(
		pNode	, "OLDVER"		, _TL("Old Version"),
		_TL("use old version for constant wind direction (no acceleration and averaging option)"),
		PARAMETER_TYPE_Bool, false
	);

	Parameters.Add_Value(
		NULL	, "ACCEL"		, _TL("Acceleration"),
		_TL(""),
		PARAMETER_TYPE_Double, 1.5, 1.0, true
	);

	Parameters.Add_Value(
		NULL	, "PYRAMIDS"	, _TL("Elevation Averaging"),
		_TL("use more averaged elevations when looking at increasing distances"),
		PARAMETER_TYPE_Bool, false
	);

/*	Parameters.Add_Value(
		NULL	, "TRACE"		, _TL("Precise Tracing"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);/**/
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CWind_Effect::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "DIR") )
	{
		pParameters->Set_Enabled("DIR_CONST", pParameter->asGrid() == NULL);
		pParameters->Set_Enabled("DIR_UNITS", pParameter->asGrid() != NULL);
		pParameters->Set_Enabled("LEN"      , pParameter->asGrid() != NULL);
		pParameters->Set_Enabled("OLDVER"   , pParameter->asGrid() == NULL);
		pParameters->Set_Enabled("PYRAMIDS" , pParameters->Get_Parameter("OLDVER")->asBool() == false);
	}

	if( !SG_STR_CMP(pParameter->Get_Identifier(), "LEN") )
	{
		pParameters->Set_Enabled("LEN_SCALE", pParameter->asGrid() != NULL);
	}

	if( !SG_STR_CMP(pParameter->Get_Identifier(), "OLDVER") )
	{
		pParameters->Set_Enabled("ACCEL"    , pParameter->asBool() == false);
		pParameters->Set_Enabled("PYRAMIDS" , pParameter->asBool() == false);
	}

	return( 1 );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CWind_Effect::On_Execute(void)
{
	CSG_Grid	*pEffect, *pAFH;

	//-----------------------------------------------------
	m_pDEM			= Parameters("DEM"    )->asGrid();
	pEffect			= Parameters("EFFECT" )->asGrid();
	pAFH			= Parameters("AFH"    )->asGrid();
	m_maxDistance	= Parameters("MAXDIST")->asDouble() * 1000.0;
	m_Acceleration	= Parameters("ACCEL"  )->asDouble();
//	m_bTrace		= Parameters("TRACE"  )->asBool();

	//-----------------------------------------------------
	CSG_Colors	Colors(5);

	Colors.Set_Color(0, 255, 127,  63);
	Colors.Set_Color(1, 255, 255, 127);
	Colors.Set_Color(2, 255, 255, 255);
	Colors.Set_Color(3, 127, 127, 175);
	Colors.Set_Color(4,   0,   0, 100);

	DataObject_Set_Colors(pEffect, Colors);
	DataObject_Set_Colors(pAFH   , Colors);

	//-----------------------------------------------------
	bool	bOldVer	= false;

	if( Parameters("DIR")->asGrid() == NULL )
	{
		bOldVer	= Parameters("OLDVER")->asBool();

		m_Dir_Const.x	= sin(Parameters("DIR_CONST")->asDouble() * M_DEG_TO_RAD);
		m_Dir_Const.y	= cos(Parameters("DIR_CONST")->asDouble() * M_DEG_TO_RAD);

		if( fabs(m_Dir_Const.x) > fabs(m_Dir_Const.y) )
		{
			m_Dir_Const.y	/= fabs(m_Dir_Const.x);
			m_Dir_Const.x	= m_Dir_Const.x < 0 ? -1 : 1;
		}
		else
		{
			m_Dir_Const.x	/= fabs(m_Dir_Const.y);
			m_Dir_Const.y	= m_Dir_Const.y < 0 ? -1 : 1;
		}
	}
	else
	{
		if( !m_DX.Create(*Get_System()) || !m_DY.Create(*Get_System()) )
		{
			Error_Set(_TL("could not allocate sufficient memory"));

			return( false );
		}

		CSG_Grid	*pDir	= Parameters("DIR")->asGrid();
		CSG_Grid	*pLen	= Parameters("LEN")->asGrid();

		double	dRadians	= Parameters("DIR_UNITS")->asInt() == 0 ? 1.0 : M_DEG_TO_RAD;
		double	dScale		= Parameters("LEN_SCALE")->asDouble();

		#pragma omp parallel for
		for(int y=0; y<Get_NY(); y++)
		{
			for(int x=0; x<Get_NX(); x++)
			{
				if( pDir->is_NoData(x, y) )
				{
					m_DX.Set_NoData(x, y);
				}
				else
				{
					double	d	= pLen ? (!pLen->is_NoData(x, y) ? dScale * pLen->asDouble(x, y) : 0.0) : 1.0;

					m_DX.Set_Value(x, y, d * sin(pDir->asDouble(x, y) * dRadians));
					m_DY.Set_Value(x, y, d * cos(pDir->asDouble(x, y) * dRadians));
				}
			}
		}
	}

	if( Parameters("PYRAMIDS")->asBool() && !bOldVer )
	{
		m_DEM.Create(m_pDEM, 2.0);
	}

	//-----------------------------------------------------
	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			if( m_pDEM->is_NoData(x, y) )
			{
				pEffect->Set_NoData(x, y);

				if( pAFH )
				{
					pAFH->Set_NoData(x, y);
				}
			}
			else
			{
				double	Luv, Luv_Lee, Lee;

				if( bOldVer )
				{
					Get_Luv_Old(x, y,  m_Dir_Const.x,  m_Dir_Const.y, Luv);
					Get_Lee_Old(x, y, -m_Dir_Const.x, -m_Dir_Const.y, Luv_Lee, Lee);
				}
				else
				{
					Get_Luv(x, y, Luv);
					Get_Lee(x, y, Luv_Lee, Lee);
				}

				//-----------------------------------------
				if( pAFH )
				{
					double	d, z	= m_pDEM->asDouble(x, y);

					d	= 1.0 + (z + Lee != 0.0 ? (z - Lee) / (z + Lee) : 0.0);
					d	= (Luv > Luv_Lee ? Luv - Luv_Lee : 0.0) + z * d*d / 2.0;

					pAFH->Set_Value(x, y, d < 0.0 ? 0.0 : d);
				}

				//-----------------------------------------
				Luv		+= Luv_Lee;

				Luv		= Luv > 0.0
						? 1.0 + log(1.0 + Luv)
						: 1.0 / (1.0 + log(1.0 - Luv));

				Lee		= Lee > 0.0
						? sqrt(1.0 + log(1.0 + Lee))
						: 1.0 / sqrt(1.0 + log(1.0 - Lee));

				Luv		= pow(Luv, 0.25);
				Lee		= pow(Lee, 0.25);

				pEffect->Set_Value(x, y, Luv * Lee);
			}
		}
	}

	//-----------------------------------------------------
	m_DX .Destroy();
	m_DY .Destroy();
	m_DEM.Destroy();

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline bool CWind_Effect::Get_Next(TSG_Point &Position, double Distance, bool bReverse)
{
	if( Distance <= 0.0 )
	{
		return( false );
	}

	double	dx, dy;

	if( !m_DX.is_Valid() )
	{
		dx	= m_Dir_Const.x;
		dy	= m_Dir_Const.y;
	}
	else
	{
	/*	if( m_bTrace )
		{
			double	d, dMove	= bReverse ? -Get_Cellsize() : Get_Cellsize();

			for(d=0.0; d<Distance; d+=Get_Cellsize())
			{
				if( !m_DX.Get_Value(Position, dx) || !m_DY.Get_Value(Position, dy) )
				{
					return( false );
				}

				Position.x	+= dMove * dx;
				Position.y	+= dMove * dy;
			}

			Distance	-= d;
		}
	/**/

		if( !m_DX.Get_Value(Position, dx) || !m_DY.Get_Value(Position, dy) )
		{
			return( false );
		}
	}

	if( bReverse )
	{
		Distance	= -Distance;
	}

	Position.x	+= Distance * dx;
	Position.y	+= Distance * dy;

	return( Get_System()->Get_Extent(true).Contains(Position) );
}

//---------------------------------------------------------
inline bool CWind_Effect::Get_Z(const TSG_Point &Position, double Distance, double &z)
{
	if( m_DEM.Get_Count() > 0 )
	{
		Distance	/= 4.0;

		for(int i=0; i<m_DEM.Get_Count(); i++)
		{
			if( Distance < m_DEM.Get_Grid(i)->Get_Cellsize() )
			{
				return( m_DEM.Get_Grid(i)->Get_Value(Position, z) );
			}
		}
	}

	return( m_pDEM->Get_Value(Position, z) );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWind_Effect::Get_Luv(int x, int y, double &Sum_A)
{
	double	Weight_A	= Sum_A	= 0.0;

	if( m_pDEM->is_InGrid(x, y) )
	{
		double		z, iz, d, id, w;
		TSG_Point	p;

		z	= m_pDEM->asDouble(x, y);
		d	= id = Get_Cellsize();
		p	= Get_System()->Get_Grid_to_World(x, y);

		while( id <= m_maxDistance && Get_Next(p, d, true) )
		{
			if( Get_Z(p, d, iz) )
			{
				Weight_A	+= w = d / id;
				Sum_A		+= w * atan2(z - iz, sqrt(id));
			}

			d	*= m_Acceleration;
			id	+= d;
		}

		if( Weight_A > 0.0 )	{	Sum_A	/= Weight_A;	}
	}
}

//---------------------------------------------------------
void CWind_Effect::Get_Lee(int x, int y, double &Sum_A, double &Sum_B)
{
	double	Weight_A	= Sum_A	= 0.0;
	double	Weight_B	= Sum_B	= 0.0;

	if( m_pDEM->is_InGrid(x, y) )
	{
		double		z, iz, d, id, w;
		TSG_Point	p;

		z	= m_pDEM->asDouble(x, y);
		d	= id = Get_Cellsize();
		p	= Get_System()->Get_Grid_to_World(x, y);

		while( id <= m_maxDistance && Get_Next(p, d, true) )
		{
			if( Get_Z(p, d, iz) )
			{
				iz	= atan2(z - iz, sqrt(id));

				Weight_A	+= w = d / id;
				Sum_A		+= w * iz;

				Weight_B	+= w = d / log(1.0 + id);
				Sum_B		+= w * iz;
			}

			d	*= m_Acceleration;
			id	+= d;
		}

		if( Weight_A > 0.0 )	{	Sum_A	/= Weight_A;	}
		if( Weight_B > 0.0 )	{	Sum_B	/= Weight_B;	}
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CWind_Effect::Get_Luv_Old(int x, int y, double dx, double dy, double &Sum_A)
{
	double	Weight_A	= Sum_A	= 0.0;

	double	w, d	= Get_Cellsize() * sqrt(dx*dx + dy*dy);

	for(double ix=x+dx+0.5, iy=y+dy+0.5, id=d, z=m_pDEM->asDouble(x, y); is_InGrid(x = (int)ix, y = (int)iy) && id<=m_maxDistance; ix+=dx, iy+=dy, id+=d)
	{
		if( !m_pDEM->is_NoData(x, y) )
		{
			Weight_A	+= w = 1.0 / id;
			Sum_A		+= w * atan2(z - m_pDEM->asDouble(x, y), sqrt(id));
		}
	}

	if( Weight_A > 0.0 )	{	Sum_A	/= Weight_A;	}
}

//---------------------------------------------------------
void CWind_Effect::Get_Lee_Old(int x, int y, double dx, double dy, double &Sum_A, double &Sum_B)
{
	double	Weight_A	= Sum_A	= 0.0;
	double	Weight_B	= Sum_B	= 0.0;

	double	w, d	= Get_Cellsize() * sqrt(dx*dx + dy*dy);

	for(double ix=x+dx+0.5, iy=y+dy+0.5, id=d, z=m_pDEM->asDouble(x, y); is_InGrid(x = (int)ix, y = (int)iy) && id<=m_maxDistance; ix+=dx, iy+=dy, id+=d)
	{
		if( !m_pDEM->is_NoData(x, y) )
		{
			double	iz	= atan2(z - m_pDEM->asDouble(x, y), sqrt(id));

			Weight_A	+= w = 1.0 / id;
			Sum_A		+= w * iz;

			Weight_B	+= w = 1.0 / log(1.0 + id);
			Sum_B		+= w * iz;
		}
	}

	if( Weight_A > 0.0 )	{	Sum_A	/= Weight_A;	}
	if( Weight_B > 0.0 )	{	Sum_B	/= Weight_B;	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
