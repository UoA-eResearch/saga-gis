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
//                     Grid_Calculus                     //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                  Grid_Calculator.cpp                  //
//                                                       //
//                 Copyright (C) 2003 by                 //
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
//    e-mail:     aringel@gwdg.de                        //
//                                                       //
//    contact:    Andre Ringeler                         //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
//														 //
//                                                       //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <float.h>

#include "Grid_Calculator.h"


///////////////////////////////////////////////////////////
//														 //
//                                                       //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#if defined(_SAGA_LINUX)
bool _finite(double val)
{
	return( true );
}
#endif


///////////////////////////////////////////////////////////
//														 //
//                                                       //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGrid_Calculator::CGrid_Calculator(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name	(_TL("Grid Calculator"));

	Set_Author	(_TL("A.Ringeler (c) 2003"));

	CSG_String	s(_TW(
		"The Grid Calculator calculates a new grid based on existing grids and a mathematical formula. "
		"The grid variables in the formula begin with the letter 'g' followed by a position index, "
		"which corresponds to the order of the grids in the input grid list "
		"(i.e.: g1, g2, g3, ... correspond to the first, second, third, ... grid in list). "
		"Grids from other systems than the default one can be addressed likewise using the letter 'h' "
		"(h1, h2, h3, ...), which correspond to the \'Grids from different Systems\' list.\n"
		"Example:\t sin(g1) * g2 + h1\n"
		"the same using indices: sin(g1) * g2 + g3\n\n"
		"The following operators are available for the formula definition:\n"
	));

	s	+= CSG_Formula::Get_Help_Operators();

	s	+= _TW(
		"xpos(), ypos() - get the x/y coordinates of the current cell\n"
		"row(), col() - get the current cell's column/row index\n"
	);

	Set_Description(s);

	//-----------------------------------------------------
	Parameters.Add_Grid_List(
		NULL	, "GRIDS"			, _TL("Grids"),
		_TL("in the formula these grids are addressed in order of the list as 'g1, g2, g3, ...'"),
		PARAMETER_INPUT_OPTIONAL
	);

	pNode	= Parameters.Add_Grid_List(
		NULL	, "XGRIDS"			, _TL("Grids from different Systems"),
		_TL("in the formula these grids are addressed in order of the list as 'h1, h2, h3, ...'"),
		PARAMETER_INPUT_OPTIONAL, false
	);

	Parameters.Add_Choice(
		pNode	,"INTERPOLATION"	, _TL("Interpolation"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|"),
			_TL("Nearest Neighbor"),
			_TL("Bilinear Interpolation"),
			_TL("Inverse Distance Interpolation"),
			_TL("Bicubic Spline Interpolation"),
			_TL("B-Spline Interpolation")
		), 4
	);

	Parameters.Add_Grid(
		NULL	, "RESULT"			, _TL("Result"),
		_TL(""),
		PARAMETER_OUTPUT
	);
	
	Parameters.Add_String(
		NULL	, "FORMULA"			, _TL("Formula"),
		_TL(""),
		SG_T("(g1 - g2) / (g1 + g2)")
	);

	pNode	= Parameters.Add_String(
		NULL	, "NAME"			, _TL("Name"),
		_TL(""),
		_TL("Calculation")
	);

	Parameters.Add_Value(
		pNode	, "FNAME"			, _TL("Take Formula"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);

	Parameters.Add_Value(
		NULL	, "USE_NODATA"		, _TL("Use NoData"),
		_TL("Check this in order to include NoData cells in the calculation."),
		PARAMETER_TYPE_Bool, false
	);

	Parameters.Add_Choice(
		NULL	, "TYPE"			, _TL("Data Type"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|%s|%s|%s|"),
			SG_Data_Type_Get_Name(SG_DATATYPE_Bit   ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Byte  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Char  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Word  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Short ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_DWord ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Int   ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Float ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Double).c_str()
		), 7
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGrid_Calculator::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	!SG_STR_CMP(pParameter->Get_Identifier(), "FORMULA")
	||	!SG_STR_CMP(pParameter->Get_Identifier(), "FNAME") )
	{
		if( pParameters->Get_Parameter("FNAME")->asBool() )
		{
			pParameters->Get_Parameter("NAME")->Set_Value(CSG_String::Format(SG_T("%s [%s]"), _TL("Calculation"), pParameters->Get_Parameter("FORMULA")->asString()));
		}
	}

	return( CSG_Module_Grid::On_Parameter_Changed(pParameters, pParameter) );
}

//---------------------------------------------------------
int CGrid_Calculator::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "XGRIDS") )
	{
		pParameters->Set_Enabled("INTERPOLATION", pParameter->asGridList()->Get_Count() > 0);
	}

	return( CSG_Module_Grid::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Calculator::On_Execute(void)
{
	bool					bUseNoData, bPosition[4];
	int						Interpol;
	CSG_Formula				Formula;
	CSG_Parameter_Grid_List	*pGrids, *pXGrids;
	CSG_Grid				*pResult;

	//-----------------------------------------------------
	pResult		= Parameters("RESULT"       )->asGrid();
	pGrids		= Parameters("GRIDS"        )->asGridList();
	pXGrids		= Parameters("XGRIDS"       )->asGridList();
	bUseNoData	= Parameters("USE_NODATA"   )->asBool();
	Interpol	= Parameters("INTERPOLATION")->asInt();

	//-----------------------------------------------------
	if( !Get_Formula(Formula, Parameters("FORMULA")->asString(), pGrids->Get_Count(), pXGrids->Get_Count(), bPosition) )
	{
		return( false );
	}

	//-----------------------------------------------------
	TSG_Data_Type	Type;

	switch( Parameters("TYPE")->asInt() )
	{
	default:	Type	= SG_DATATYPE_Float ;	break;
	case  0:	Type	= SG_DATATYPE_Bit   ;	break;
	case  1:	Type	= SG_DATATYPE_Byte  ;	break;
	case  2:	Type	= SG_DATATYPE_Char  ;	break;
	case  3:	Type	= SG_DATATYPE_Word  ;	break;
	case  4:	Type	= SG_DATATYPE_Short ;	break;
	case  5:	Type	= SG_DATATYPE_DWord ;	break;
	case  6:	Type	= SG_DATATYPE_Int   ;	break;
	case  7:	Type	= SG_DATATYPE_Float ;	break;
	case  8:	Type	= SG_DATATYPE_Double;	break;
	}

	if( Type != pResult->Get_Type() )
	{
		pResult->Create(*Get_System(), Type);
	}

	pResult->Set_Name(Parameters("NAME")->asString());

	int	nValues	= pGrids->Get_Count() + pXGrids->Get_Count()
		+ (bPosition[0] ? 1 : 0)
		+ (bPosition[1] ? 1 : 0)
		+ (bPosition[2] ? 1 : 0)
		+ (bPosition[3] ? 1 : 0);

	//-----------------------------------------------------
	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		double	py	= Get_YMin() + y * Get_Cellsize();

		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			bool		bOkay	= true;
			int			i, n	= 0;
			double		Result, px	= Get_XMin() + x * Get_Cellsize();
			CSG_Vector	Values(nValues);

			for(i=0; bOkay && i<pGrids->Get_Count() ; i++, n++)
			{
				if( (bOkay = bUseNoData || !pGrids->asGrid(i)->is_NoData(x, y)) == true )
				{
					Values[n]	= pGrids->asGrid(i)->asDouble(x, y);
				}
			}

			for(i=0; bOkay && i<pXGrids->Get_Count(); i++, n++)
			{
				bOkay	= pXGrids->asGrid(i)->Get_Value(px, py, Values[n], Interpol);
			}

			if( bOkay )
			{
				if( bPosition[0] )	Values[n++]	=  x;	// col()
				if( bPosition[1] )	Values[n++]	=  y;	// row()
				if( bPosition[2] )	Values[n++]	= px;	// xpos()
				if( bPosition[3] )	Values[n++]	= py;	// ypos()

				bOkay	= _finite(Result = Formula.Get_Value(Values)) != 0;
			}

			if( bOkay )
			{
				pResult->Set_Value(x, y, Result);
			}
			else
			{
				pResult->Set_NoData(x, y);
			}
		}
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Calculator::Get_Formula(CSG_Formula &Formula, CSG_String sFormula, int nGrids, int nXGrids, bool bPosition[4])
{
	//-----------------------------------------------------
	const int		nVars		= 27;

	const SG_Char	Vars[nVars]	= SG_T("abcdefghijklmnopqrstuvwxyz");

	int		nValues	= nGrids + nXGrids;

	bPosition[0]	= sFormula.Find("col()" ) >= 0;	if( bPosition[0] )	nValues++;
	bPosition[1]	= sFormula.Find("row()" ) >= 0;	if( bPosition[1] )	nValues++;
	bPosition[2]	= sFormula.Find("xpos()") >= 0;	if( bPosition[2] )	nValues++;
	bPosition[3]	= sFormula.Find("ypos()") >= 0;	if( bPosition[3] )	nValues++;

	//-----------------------------------------------------
	if( nValues > nVars )
	{
		Error_Set(_TL("too many input variables"));

		return( false );
	}

	//-----------------------------------------------------
	int		i, n	= nValues;

	if( bPosition[3] )	sFormula.Replace("ypos()", Vars[--n]);
	if( bPosition[2] )	sFormula.Replace("xpos()", Vars[--n]);
	if( bPosition[1] )	sFormula.Replace("row()" , Vars[--n]);
	if( bPosition[0] )	sFormula.Replace("col()" , Vars[--n]);

	for(i=nXGrids; i>0 && n>0; i--)
	{
		sFormula.Replace(CSG_String::Format(SG_T("h%d"), i), Vars[--n]);
	}

	for(i=nGrids ; i>0 && n>0; i--)
	{
		sFormula.Replace(CSG_String::Format(SG_T("g%d"), i), Vars[--n]);
	}

	//-----------------------------------------------------
	if( !Formula.Set_Formula(sFormula) )
	{
		CSG_String	Message;

		if( !Formula.Get_Error(Message) )
		{
			Message.Printf("%s: %s", _TL("error in formula"), sFormula.c_str());
		}

		Error_Set(Message);

		return( false );
	}

	//-----------------------------------------------------
	CSG_String	sUsed(Formula.Get_Used_Variables());

	if( nValues != sUsed.Length() )
	{
		Error_Fmt("%s (%d < %d)", _TL("The number of supplied values does not fit the number of variables in formula."), nValues, sUsed.Length());

		return( false );
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//                                                       //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
