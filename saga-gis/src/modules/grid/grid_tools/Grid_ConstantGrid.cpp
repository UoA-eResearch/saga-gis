/**********************************************************
 * Version $Id$
 *********************************************************/
/*******************************************************************************
    ConstantGrid.cpp
    Copyright (C) Victor Olaya
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301, USA
*******************************************************************************/ 

///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "Grid_ConstantGrid.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CConstantGrid::CConstantGrid(void)
{
	Set_Name		(_TL("Constant Grid"));

	Set_Author		("Victor Olaya (c) 2004");

	Set_Description	(_TW(
		"Constant grid creation."
	));

	//-----------------------------------------------------
	Parameters.Add_String(
		NULL	, "NAME"		, _TL("Name"),
		_TL(""),
		_TL("Constant Grid")
	);

	Parameters.Add_Value(
		NULL	, "CONST"		, _TL("Constant Value"),
		_TL(""),
		PARAMETER_TYPE_Double, 1
	);

	//-----------------------------------------------------
	Parameters.Add_Choice(
		NULL	, "TYPE"		, _TL("Data Type"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s|%s|%s|%s|"),
			SG_Data_Type_Get_Name(SG_DATATYPE_Bit   ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Byte  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Char  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Word  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Short ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_ULong ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Long  ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Float ).c_str(),
			SG_Data_Type_Get_Name(SG_DATATYPE_Double).c_str()
		), 7
	);

	//-----------------------------------------------------
	m_Grid_Target.Create(&Parameters);
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CConstantGrid::On_Parameter_Changed(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	return( m_Grid_Target.On_Parameter_Changed(pParameters, pParameter) ? 1 : 0 );
}

//---------------------------------------------------------
int CConstantGrid::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	return( m_Grid_Target.On_Parameters_Enable(pParameters, pParameter) ? 1 : 0 );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CConstantGrid::On_Execute(void)
{
	//-----------------------------------------------------
	TSG_Data_Type	Type	= SG_DATATYPE_Float;

	switch( Parameters("TYPE")->asInt() )
	{
	case 0:	Type	= SG_DATATYPE_Bit   ;	break;
	case 1:	Type	= SG_DATATYPE_Byte  ;	break;
	case 2:	Type	= SG_DATATYPE_Char  ;	break;
	case 3:	Type	= SG_DATATYPE_Word  ;	break;
	case 4:	Type	= SG_DATATYPE_Short ;	break;
	case 5:	Type	= SG_DATATYPE_ULong ;	break;
	case 6:	Type	= SG_DATATYPE_Long  ;	break;
	case 7:	Type	= SG_DATATYPE_Float ;	break;
	case 8:	Type	= SG_DATATYPE_Double;	break;
	}

	//-----------------------------------------------------
	CSG_Grid	*pGrid	= m_Grid_Target.Get_Grid(Type);

	if( pGrid == NULL )
	{
		return( false );
	}

	//-----------------------------------------------------
	pGrid->Set_Name(Parameters("NAME" )->asString());

	pGrid->Assign  (Parameters("CONST")->asDouble());

	return( true );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
