
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//           Application Programming Interface           //
//                                                       //
//                  Library: SAGA_API                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                    parameters.cpp                     //
//                                                       //
//          Copyright (C) 2005 by Olaf Conrad            //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'.                              //
//                                                       //
// This library is free software; you can redistribute   //
// it and/or modify it under the terms of the GNU Lesser //
// General Public License as published by the Free       //
// Software Foundation, version 2.1 of the License.      //
//                                                       //
// This library is distributed in the hope that it will  //
// be useful, but WITHOUT ANY WARRANTY; without even the //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU Lesser General Public //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU Lesser     //
// General Public License along with this program; if    //
// not, write to the Free Software Foundation, Inc.,     //
// 59 Temple Place - Suite 330, Boston, MA 02111-1307,   //
// USA.                                                  //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Goettingen               //
//                Goldschmidtstr. 5                      //
//                37077 Goettingen                       //
//                Germany                                //
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "parameters.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameters::CSG_Parameters(void)
{
	_On_Construction();
}

//---------------------------------------------------------
CSG_Parameters::CSG_Parameters(void *pOwner, const SG_Char *Name, const SG_Char *Description, const SG_Char *Identifier, bool bGrid_System)
{
	_On_Construction();

	Create(pOwner, Name, Description, Identifier, bGrid_System);
}

//---------------------------------------------------------
CSG_Parameters::~CSG_Parameters(void)
{
	Destroy();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CSG_Parameters::_On_Construction(void)
{
	m_pOwner		= NULL;

	m_Parameters	= NULL;
	m_nParameters	= 0;

	m_Callback		= NULL;
	m_bCallback		= false;

	m_pGrid_System	= NULL;

	m_bManaged		= true;
}

//---------------------------------------------------------
void CSG_Parameters::Create(void *pOwner, const SG_Char *Name, const SG_Char *Description, const SG_Char *Identifier, bool bGrid_System)
{
	Destroy();

	m_pOwner		= pOwner;

	Set_Identifier	(Identifier);
	Set_Name		(Name);
	Set_Description	(Description);

	if( bGrid_System )
	{
		m_pGrid_System	= Add_Grid_System(
			NULL, SG_T("PARAMETERS_GRID_SYSTEM"),
			LNG("[PRM] Grid system"),
			LNG("[PRM] Grid system")
		);
	}
}

//---------------------------------------------------------
void CSG_Parameters::Destroy(void)
{
	m_pOwner		= NULL;
	m_pGrid_System	= NULL;

	if( m_nParameters > 0 )
	{
		for(int i=0; i<m_nParameters; i++)
		{
			delete(m_Parameters[i]);
		}

		SG_Free(m_Parameters);

		m_Parameters	= NULL;
		m_nParameters	= 0;
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CSG_Parameters::Set_Identifier(const SG_Char *String)
{
	if( String )
	{
		m_Identifier.Printf(String);
	}
	else
	{
		m_Identifier.Clear();
	}
}

const SG_Char * CSG_Parameters::Get_Identifier(void)
{
	return( m_Identifier );
}

//---------------------------------------------------------
void CSG_Parameters::Set_Name(const SG_Char *String)
{
	if( String )
	{
		m_Name.Printf(String);
	}
	else
	{
		m_Name.Clear();
	}
}

const SG_Char * CSG_Parameters::Get_Name(void)
{
	return( m_Name );
}

//---------------------------------------------------------
void CSG_Parameters::Set_Description(const SG_Char *String)
{
	if( String )
	{
		m_Description.Printf(String);
	}
	else
	{
		m_Description.Clear();
	}
}

const SG_Char * CSG_Parameters::Get_Description(void)
{
	return( m_Description );
}

//---------------------------------------------------------
void CSG_Parameters::Set_Translation(CSG_Translator &Translator)
{
	m_Name			= Translator.Get_Translation(m_Name);
	m_Description	= Translator.Get_Translation(m_Description);

	for(int i=0; i<m_nParameters; i++)
	{
		m_Parameters[i]->m_Name			= Translator.Get_Translation(m_Parameters[i]->m_Name);
		m_Parameters[i]->m_Description	= Translator.Get_Translation(m_Parameters[i]->m_Description);
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Node(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	return( _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Node, PARAMETER_INFORMATION) );
}

//---------------------------------------------------------
/**
  * Following parameter types can be used:
  * PARAMETER_TYPE_Bool
  * PARAMETER_TYPE_Int
  * PARAMETER_TYPE_Double
  *	PARAMETER_TYPE_Degree
  * PARAMETER_TYPE_Color
*/
CSG_Parameter * CSG_Parameters::Add_Value(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, TSG_Parameter_Type Type, double Value, double Minimum, bool bMinimum, double Maximum, bool bMaximum)
{
	return( _Add_Value(pParent, Identifier, Name, Description, false, Type, Value, Minimum, bMinimum, Maximum, bMaximum) );
}

CSG_Parameter * CSG_Parameters::Add_Info_Value(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, TSG_Parameter_Type Type, double Value)
{
	return( _Add_Value(pParent, Identifier, Name, Description,  true, Type, Value, 0.0, false, 0.0, false) );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Range(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, double Range_Min, double Range_Max, double Minimum, bool bMinimum, double Maximum, bool bMaximum)
{
	return( _Add_Range(pParent, Identifier, Name, Description, false, Range_Min, Range_Max, Minimum, bMinimum, Maximum, bMaximum) );
}

CSG_Parameter * CSG_Parameters::Add_Info_Range(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, double Range_Min, double Range_Max)
{
	return( _Add_Range(pParent, Identifier, Name, Description,  true, Range_Min, Range_Max, 0.0, false, 0.0, false) );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Choice(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *Items, int Default)
{
	CSG_Parameter			*pParameter;
	CSG_Parameter_Choice	*m_pData;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Choice, 0);

	m_pData		= (CSG_Parameter_Choice *)pParameter->m_pData;
	m_pData->Set_Items(Items);

	pParameter->Set_Value(Default);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_String(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *String, bool bLongText, bool bPassword)
{
	return( _Add_String(pParent, Identifier, Name, Description, false, String, bLongText, bPassword) );
}

CSG_Parameter * CSG_Parameters::Add_Info_String(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *String, bool bLongText)
{
	return( _Add_String(pParent, Identifier, Name, Description,  true, String, bLongText, false) );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_FilePath(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *Filter, const SG_Char *Default, bool bSave, bool bDirectory, bool bMultiple)
{
	CSG_Parameter			*pParameter;
	CSG_Parameter_File_Name	*m_pData;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_FilePath, 0);

	m_pData		= (CSG_Parameter_File_Name *)pParameter->m_pData;
	m_pData->Set_Filter			(Filter);
	m_pData->Set_Flag_Save		(bSave);
	m_pData->Set_Flag_Multiple	(bMultiple);
	m_pData->Set_Flag_Directory	(bDirectory);

	pParameter->Set_Value((void *)Default);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Font(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, wxFont *pInit)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Font, 0);

	if( pInit )
	{
		pParameter->Set_Value(pInit);
	}

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Colors(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, CSG_Colors *pInit)
{
	CSG_Parameter			*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Colors, 0);

	pParameter->asColors()->Assign(pInit);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_FixedTable(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, CSG_Table *pTemplate)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_FixedTable, 0);

	pParameter->asTable()->Create(pTemplate);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Grid_System(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, CSG_Grid_System *pInit)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Grid_System, 0);

	if( pInit )
	{
		pParameter->asGrid_System()->Assign(*pInit);
	}

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Grid(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, bool bSystem_Dependent, TSG_Grid_Type Preferred_Type)
{
	CSG_Parameter	*pParameter;

	if( !pParent || pParent->Get_Type() != PARAMETER_TYPE_Grid_System )
	{
		if( bSystem_Dependent && m_pGrid_System )
		{
			pParent	= m_pGrid_System;
		}
		else
		{
			pParent	= Add_Grid_System(pParent, CSG_String::Format(SG_T("%s_GRIDSYSTEM"), Identifier), LNG("[PRM] Grid system"), SG_T(""));
		}
	}

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Grid, Constraint);

	((CSG_Parameter_Grid *)pParameter->m_pData)->Set_Preferred_Type(Preferred_Type);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Grid_Output(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_DataObject_Output, PARAMETER_OUTPUT_OPTIONAL);

	((CSG_Parameter_Data_Object_Output *)pParameter->Get_Data())->Set_DataObject_Type(DATAOBJECT_TYPE_Grid);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Grid_List(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, bool bSystem_Dependent)
{
	CSG_Parameter	*pParameter;

	if( bSystem_Dependent && (!pParent || pParent->Get_Type() != PARAMETER_TYPE_Grid_System) )
	{
		pParent	= m_pGrid_System;
	}

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Grid_List, Constraint);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Table_Field(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	if( pParent
	&&	(	pParent->Get_Type() == PARAMETER_TYPE_Table
		||	pParent->Get_Type() == PARAMETER_TYPE_Shapes
		||	pParent->Get_Type() == PARAMETER_TYPE_TIN	) )
	{
		return( _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Table_Field, 0) );
	}

	return( NULL );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Table(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Table, Constraint);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Table_Output(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_DataObject_Output, PARAMETER_OUTPUT_OPTIONAL);

	((CSG_Parameter_Data_Object_Output *)pParameter->Get_Data())->Set_DataObject_Type(DATAOBJECT_TYPE_Table);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Table_List(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Table_List, Constraint);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Shapes(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, TSG_Shape_Type Shape_Type)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Shapes, Constraint);

	((CSG_Parameter_Shapes *)pParameter->m_pData)->Set_Shape_Type(Shape_Type);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Shapes_Output(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_DataObject_Output, PARAMETER_OUTPUT_OPTIONAL);

	((CSG_Parameter_Data_Object_Output *)pParameter->Get_Data())->Set_DataObject_Type(DATAOBJECT_TYPE_Shapes);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Shapes_List(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, TSG_Shape_Type Type)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Shapes_List, Constraint);

	((CSG_Parameter_Shapes_List *)pParameter->m_pData)->Set_Shape_Type(Type);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_TIN(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_TIN, Constraint);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_TIN_Output(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_DataObject_Output, PARAMETER_OUTPUT_OPTIONAL);

	((CSG_Parameter_Data_Object_Output *)pParameter->Get_Data())->Set_DataObject_Type(DATAOBJECT_TYPE_TIN);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_TIN_List(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_TIN_List, Constraint);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Add_Parameters(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Parameters, 0);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::_Add_Value(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, bool bInformation, TSG_Parameter_Type Type, double Value, double Minimum, bool bMinimum, double Maximum, bool bMaximum)
{
	CSG_Parameter			*pParameter;
	CSG_Parameter_Value	*m_pData;

	switch( Type )	// Check if Type is valid...
	{
	case PARAMETER_TYPE_Bool:
	case PARAMETER_TYPE_Int:
	case PARAMETER_TYPE_Double:
	case PARAMETER_TYPE_Degree:
	case PARAMETER_TYPE_Color:
		break;

	default:
		Type	= PARAMETER_TYPE_Double;	// if not valid set Type to [double]...
		break;
	}

	pParameter	= _Add(pParent, Identifier, Name, Description, Type, bInformation ? PARAMETER_INFORMATION : 0);

	if( !bInformation )
	{
		switch( Type )
		{
		default:
			break;

		case PARAMETER_TYPE_Int:
		case PARAMETER_TYPE_Double:
		case PARAMETER_TYPE_Degree:
			m_pData		= (CSG_Parameter_Value *)pParameter->m_pData;
			m_pData->Set_Minimum(Minimum, bMinimum);
			m_pData->Set_Maximum(Maximum, bMaximum);
			break;
		}
	}

	pParameter->Set_Value(Value);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::_Add_Range(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, bool bInformation, double Default_Min, double Default_Max, double Minimum, bool bMinimum, double Maximum, bool bMaximum)
{
	double				d;
	CSG_Parameter			*pParameter;
	CSG_Parameter_Range	*m_pData;

	//-----------------------------------------------------
	if( Default_Min > Default_Max )
	{
		d			= Default_Min;
		Default_Min	= Default_Max;
		Default_Max	= d;
	}

	//-----------------------------------------------------
	pParameter	= _Add(pParent, Identifier, Name, Description, PARAMETER_TYPE_Range, bInformation ? PARAMETER_INFORMATION : 0);

	m_pData		= pParameter->asRange();

	m_pData->Get_LoParm()->asValue()->Set_Minimum(Minimum, bMinimum);
	m_pData->Get_LoParm()->asValue()->Set_Maximum(Maximum, bMaximum);
	m_pData->Get_HiParm()->asValue()->Set_Minimum(Minimum, bMinimum);
	m_pData->Get_HiParm()->asValue()->Set_Maximum(Maximum, bMaximum);

	m_pData->Set_LoVal(Default_Min);
	m_pData->Set_HiVal(Default_Max);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::_Add_String(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, bool bInformation, const SG_Char *String, bool bLongText, bool bPassword)
{
	CSG_Parameter	*pParameter;

	pParameter	= _Add(pParent, Identifier, Name, Description, bLongText ? PARAMETER_TYPE_Text : PARAMETER_TYPE_String, bInformation ? PARAMETER_INFORMATION : 0);

	pParameter->Set_Value((void *)String);

	((CSG_Parameter_String *)pParameter)->Set_Password(bPassword);

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::_Add(CSG_Parameter *pParent, const SG_Char *Identifier, const SG_Char *Name, const SG_Char *Description, TSG_Parameter_Type Type, int Constraint)
{
	CSG_Parameter	*pParameter;

	m_Parameters	= (CSG_Parameter **)SG_Realloc(m_Parameters, (m_nParameters + 1) * sizeof(CSG_Parameter *));

	pParameter		= m_Parameters[m_nParameters++]	= new CSG_Parameter(this, pParent, Identifier, Name, Description, Type, Constraint);

	return( pParameter );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::_Add(CSG_Parameter *pSource)
{
	CSG_Parameter	*pParameter;

	if( pSource )
	{
		pParameter	= _Add(
			pSource->Get_Parent() ? Get_Parameter(pSource->Get_Parent()->Get_Identifier()) : NULL,
			pSource->Get_Identifier(),
			pSource->Get_Name(),
			pSource->Get_Description(),
			pSource->Get_Type(),
			pSource->m_pData->Get_Constraint()
		);

		pParameter->Assign(pSource);
	}
	else
	{
		pParameter	= NULL;
	}

	return( pParameter );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Get_Parameter(const SG_Char *Identifier)
{
	int		i;

	if( m_Parameters && Identifier )
	{
		for(i=0; i<m_nParameters; i++)
		{
			if( !m_Parameters[i]->m_Identifier.Cmp(Identifier) )
			{
				return( m_Parameters[i] );
			}
		}
	}

	return( NULL );
}

//---------------------------------------------------------
CSG_Parameter * CSG_Parameters::Get_Parameter(int iParameter)
{
	if( m_Parameters && iParameter >= 0 && iParameter < m_nParameters )
	{
		return( m_Parameters[iParameter] );
	}

	return( NULL );
}


///////////////////////////////////////////////////////////
//														 //
//						Callback						 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CSG_Parameters::Set_Callback_On_Parameter_Changed(TSG_PFNC_Parameter_Changed Callback)
{
	m_Callback	= Callback;
}

//---------------------------------------------------------
void CSG_Parameters::Set_Callback(bool bActive)
{
	m_bCallback	= bActive;
}

//---------------------------------------------------------
bool CSG_Parameters::_On_Parameter_Changed(CSG_Parameter *pSender)
{
	if( m_Callback && m_bCallback )
	{
		m_bCallback	= false;
		m_Callback(pSender);
		m_bCallback	= true;

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Parameters::Set_Parameter(const SG_Char *Identifier, CSG_Parameter *pSource)
{
	CSG_Parameter	*pTarget;

	if( pSource != NULL && (pTarget = Get_Parameter(Identifier)) != NULL && pSource->Get_Type() == pTarget->Get_Type() )
	{
		switch( pTarget->Get_Type() )
		{
		default:
			return( pTarget->Assign(pSource) );

		case PARAMETER_TYPE_DataObject_Output:
		case PARAMETER_TYPE_Grid:
		case PARAMETER_TYPE_Table:
		case PARAMETER_TYPE_Shapes:
		case PARAMETER_TYPE_TIN:
			return( pTarget->Set_Value(pSource->asDataObject()) );
		}
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Parameters::Set_Parameter(const SG_Char *Identifier, int Type, int Value)
{
	CSG_Parameter	*pTarget;

	if( (pTarget = Get_Parameter(Identifier)) != NULL && Type == pTarget->Get_Type() )
	{
		pTarget->Set_Value(Value);

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Parameters::Set_Parameter(const SG_Char *Identifier, int Type, double Value)
{
	CSG_Parameter	*pTarget;

	if( (pTarget = Get_Parameter(Identifier)) != NULL && Type == pTarget->Get_Type() )
	{
		pTarget->Set_Value(Value);

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Parameters::Set_Parameter(const SG_Char *Identifier, int Type, void *Value)
{
	CSG_Parameter	*pTarget;

	if( (pTarget = Get_Parameter(Identifier)) != NULL && Type == pTarget->Get_Type() )
	{
		pTarget->Set_Value(Value);

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Parameters::Set_Parameter(const SG_Char *Identifier, int Type, const SG_Char *Value)
{
	CSG_Parameter	*pTarget;

	if( (pTarget = Get_Parameter(Identifier)) != NULL && Type == pTarget->Get_Type() )
	{
		pTarget->Set_Value(Value);

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CSG_Parameters::Assign(CSG_Parameters *pSource)
{
	int			i;
	CSG_Parameter	*pParameter;

	if( pSource )
	{
		Destroy();

		m_pOwner	= pSource->Get_Owner();

		Set_Identifier	(pSource->Get_Identifier());
		Set_Name		(pSource->Get_Name());
		Set_Description	(pSource->Get_Description());

		m_Callback	= pSource->m_Callback;
		m_bCallback	= pSource->m_bCallback;

		if( pSource->Get_Count() > 0 )
		{
			for(i=0; i<pSource->Get_Count(); i++)
			{
				_Add(pSource->Get_Parameter(i));
			}

			for(i=0; i<pSource->Get_Count(); i++)
			{
				if( Get_Parameter(i) && (pParameter = pSource->Get_Parameter(i)->Get_Parent()) != NULL )
				{
					Get_Parameter(i)->m_pParent	= Get_Parameter(pParameter->Get_Identifier());
				}
			}

			if( pSource->m_pGrid_System )
			{
				m_pGrid_System	= Get_Parameter(pSource->m_pGrid_System->Get_Identifier());
			}
		}

		return( m_nParameters );
	}

	return( -1 );
}

//---------------------------------------------------------
int CSG_Parameters::Assign_Values(CSG_Parameters *pSource)
{
	int			i, n;
	CSG_Parameter	*pParameter;

	if( pSource )
	{
		for(i=0, n=0; i<pSource->Get_Count(); i++)
		{
			pParameter	= Get_Parameter(pSource->Get_Parameter(i)->Get_Identifier());

			if( pParameter && pParameter->Get_Type() == pSource->Get_Parameter(i)->Get_Type() )
			{
				pParameter->Assign(pSource->Get_Parameter(i));
				n++;
			}
		}

		return( n );
	}

	return( 0 );
}


///////////////////////////////////////////////////////////
//														 //
//						Serialize						 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#define PARAMETER_ENTRIES_BEGIN		SG_T("[PARAMETER_ENTRIES_BEGIN]")
#define PARAMETER_ENTRIES_END		SG_T("[PARAMETER_ENTRIES_END]")
#define PARAMETER_ENTRY_BEGIN		SG_T("[PARAMETER_ENTRY_BEGIN]")
#define PARAMETER_ENTRY_END			SG_T("[PARAMETER_ENTRY_END]")

//---------------------------------------------------------
bool CSG_Parameters::Serialize(const SG_Char *File_Name, bool bSave)
{
	bool		bResult	= false;
	CSG_File	Stream;

	if( Stream.Open(File_Name, bSave ? SG_FILE_W : SG_FILE_R, true) )
	{
		bResult	= Serialize(Stream, bSave);
	}

	return( bResult );
}

//---------------------------------------------------------
bool CSG_Parameters::Serialize(CSG_File &Stream, bool bSave)
{
	int			i;
	CSG_Parameter	*pParameter;
	CSG_String	sLine;

	if( Stream.is_Open() )
	{
		//-------------------------------------------------
		if( bSave )
		{
			Stream.Printf(SG_T("\n%s\n"), PARAMETER_ENTRIES_BEGIN);

			for(i=0; i<m_nParameters; i++)
			{
				if(	m_Parameters[i]->is_Serializable() )
				{
					Stream.Printf(SG_T("%s\n"), PARAMETER_ENTRY_BEGIN);
					Stream.Printf(SG_T("%s\n"), m_Parameters[i]->m_Identifier.c_str());

					m_Parameters[i]->Serialize(Stream, true);

					Stream.Printf(SG_T("%s\n"), PARAMETER_ENTRY_END);
				}
			}

			Stream.Printf(SG_T("%s\n"), PARAMETER_ENTRIES_END);

			return( true );
		}

		//-------------------------------------------------
		else
		{
			while( Stream.Read_Line(sLine) && sLine.Cmp(PARAMETER_ENTRIES_BEGIN) );

			if( !sLine.Cmp(PARAMETER_ENTRIES_BEGIN) )
			{
				while( Stream.Read_Line(sLine) && sLine.Cmp(PARAMETER_ENTRIES_END) )
				{
					if( !sLine.Cmp(PARAMETER_ENTRY_BEGIN) )
					{
						if( Stream.Read_Line(sLine) && (pParameter = Get_Parameter(sLine)) != NULL )
						{
							pParameter->Serialize(Stream, false);
						}
					}
				}
			}

			return( true );
		}
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Parameters::DataObjects_Check(bool bSilent)
{
	bool	bInvalid, bResult	= true;
	CSG_String	s;

	//-----------------------------------------------------
	for(int i=0; i<Get_Count(); i++)
	{
		switch( m_Parameters[i]->Get_Type() )
		{
		default:
			bInvalid	= false;
			break;

		case PARAMETER_TYPE_Parameters:
			bInvalid	= m_Parameters[i]->asParameters()->DataObjects_Check(bSilent);
			break;

		case PARAMETER_TYPE_Grid:
		case PARAMETER_TYPE_Table:
		case PARAMETER_TYPE_Shapes:
		case PARAMETER_TYPE_TIN:
			bInvalid	=  m_Parameters[i]->is_Input()				== true
						&& m_Parameters[i]->is_Optional()			== false
						&& m_Parameters[i]->asDataObject()			== NULL;
			break;

		case PARAMETER_TYPE_Grid_List:
		case PARAMETER_TYPE_Table_List:
		case PARAMETER_TYPE_Shapes_List:
		case PARAMETER_TYPE_TIN_List:
			bInvalid	=  m_Parameters[i]->is_Input()				== true
						&& m_Parameters[i]->is_Optional()			== false
						&& m_Parameters[i]->asList()->Get_Count()	== 0;
			break;
		}

		if( bInvalid )
		{
			bResult	= false;
			s.Append(CSG_String::Format(SG_T("\n%s: %s"), m_Parameters[i]->Get_Type_Name(), m_Parameters[i]->Get_Name()));
		}
	}

	//-----------------------------------------------------
	if( !bResult && !bSilent )
	{
		SG_UI_Dlg_Message(CSG_String::Format(SG_T("%s\n%s"), LNG("[DLG] Invalid parameters!"), s.c_str() ), Get_Name() );
	}

	return( bResult );
}

//---------------------------------------------------------
bool CSG_Parameters::DataObjects_Create(void)
{
	if( m_bManaged )
	{
		for(int i=0; i<Get_Count(); i++)
		{
			CSG_Data_Object		*pDataObject;
			CSG_Grid_System	*pGrid_System;
			CSG_Parameter		*p	= m_Parameters[i];

			if( p->Get_Type() == PARAMETER_TYPE_Parameters )
			{
				p->asParameters()->DataObjects_Create();
			}
			else if( p->Get_Type() == PARAMETER_TYPE_DataObject_Output )
			{
				p->Set_Value(DATAOBJECT_NOTSET);
			}
			else if( p->is_DataObject() && p->is_Output()
			&&	(	(p->asDataObject() == DATAOBJECT_CREATE)
				||	(p->asDataObject() == NULL && !p->is_Optional())	)	)
			{
				pDataObject	= NULL;

				switch( p->Get_Type() )
				{
				default:
					break;

				case PARAMETER_TYPE_Grid:
					if(	p->Get_Parent() && p->Get_Parent()->Get_Type() == PARAMETER_TYPE_Grid_System
					&&	(pGrid_System = p->Get_Parent()->asGrid_System()) != NULL && pGrid_System->is_Valid() )
					{
						pDataObject	= SG_Create_Grid(*pGrid_System, ((CSG_Parameter_Grid *)p->Get_Data())->Get_Preferred_Type());
					}
					break;

				case PARAMETER_TYPE_Table:
					pDataObject	= SG_Create_Table();
					break;

				case PARAMETER_TYPE_Shapes:
					pDataObject	= SG_Create_Shapes(((CSG_Parameter_Shapes *)p->Get_Data())->Get_Shape_Type());
					break;

				case PARAMETER_TYPE_TIN:
					pDataObject	= SG_Create_TIN();
					break;
				}

				p->Set_Value(pDataObject);

				if( pDataObject )
				{
					pDataObject->Set_Name(p->Get_Name());
					SG_UI_DataObject_Add(pDataObject, false);
				}
			}
		}
	}

	return( true );
}

//---------------------------------------------------------
bool CSG_Parameters::DataObjects_Synchronize(void)
{
	if( m_bManaged )
	{
		for(int i=0; i<Get_Count(); i++)
		{
			CSG_Parameter	*p	= m_Parameters[i];

			if( p->Get_Type() == PARAMETER_TYPE_Parameters )
			{
				p->asParameters()->DataObjects_Synchronize();
			}
			else
			{
				if( p->Get_Type() == PARAMETER_TYPE_Shapes && p->asShapes() && p->asShapes()->Get_Type() == SHAPE_TYPE_Undefined )
				{
					delete(p->asShapes());
					p->Set_Value(DATAOBJECT_NOTSET);
				}

				if( p->is_Output() )
				{
					if( p->is_DataObject() )
					{
						if( p->asDataObject() )
						{
							SG_UI_DataObject_Add		(p->asDataObject(), false);
							SG_UI_DataObject_Update	(p->asDataObject(), false, NULL);
						}
					}
					else if( p->is_DataObject_List() )
					{
						for(int j=0; j<p->asList()->Get_Count(); j++)
						{
							SG_UI_DataObject_Add		(p->asList()->asDataObject(j), false);
							SG_UI_DataObject_Update	(p->asList()->asDataObject(j), false, NULL);
						}
					}
				}
			}
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_Parameters::Get_String(CSG_String &String, bool bOptionsOnly)
{
	bool	bResult	= false;

	if( Get_Count() > 0 )
	{
		String.Printf(SG_T("%s:"), bOptionsOnly ? LNG("[CAP] Options") : LNG("[CAP] Parameters"));

		for(int i=0; i<Get_Count(); i++)
		{
			if( (!bOptionsOnly || m_Parameters[i]->is_Option()) && !m_Parameters[i]->is_Information() )
			{
				bResult	= true;

				String.Append(CSG_String::Format(SG_T("\n[%s] %s: %s"),
					m_Parameters[i]->Get_Type_Name(),
					m_Parameters[i]->Get_Name(),
					m_Parameters[i]->asString())
				);
			}
		}
	}

	return( bResult );
}

//---------------------------------------------------------
bool CSG_Parameters::Msg_String(bool bOptionsOnly)
{
	CSG_String	s;

	if( Get_String(s, bOptionsOnly) )
	{
		SG_UI_Msg_Add_Execution(s, true);

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_Parameters::Set_History(CSG_History &History, bool bOptions, bool bDataObjects)
{
	int			i, j;
	CSG_Data_Object	*pObject;
	CSG_Parameter	*p;

	//-----------------------------------------------------
	if( bOptions )
	{
		for(i=0; i<Get_Count(); i++)	// get options...
		{
			p	= m_Parameters[i];

			if(	p->is_Option() && !p->is_Information() )
			{
				History.Add_Entry(p->Get_Name(), p->asString());
			}

			if( p->is_Parameters() )
			{
				p->asParameters()->Set_History(History, true, false);
			}
		}
	}

	//-----------------------------------------------------
	if( bDataObjects )
	{
		for(i=0; i<Get_Count(); i++)	// get input with history...
		{
			p	= m_Parameters[i];

			if( p->is_Input() && p->is_DataObject() && (pObject = p->asDataObject()) != NULL )
			{
				History.Add_Entry(p->Get_Name(), pObject->Get_Name(), &pObject->Get_History());
			}

			if( p->is_Input() && p->is_DataObject_List() )
			{
				History.Add_Entry(p->Get_Name(), p->asString());

				for(j=0; j<p->asList()->Get_Count(); j++)
				{
					pObject	= p->asList()->asDataObject(j);
					History.Add_Entry(p->Get_Name(), pObject->Get_Name(), &pObject->Get_History());
				}
			}

			if( p->is_Parameters() )
			{
				p->asParameters()->Set_History(History, false, true);
			}
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifdef _SAGA_UNICODE

CSG_Parameter * CSG_Parameters::Get_Parameter(const char *Identifier)
{	return( Get_Parameter(SG_STR_MBTOSG(Identifier)) );	}

CSG_Parameter * CSG_Parameters::Add_Node				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_Node				(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

CSG_Parameter * CSG_Parameters::Add_Value				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, TSG_Parameter_Type Type, double Value, double Minimum, bool bMinimum, double Maximum, bool bMaximum)
{	return( Add_Value				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Type, Value, Minimum, bMinimum, Maximum, bMaximum) );	}

CSG_Parameter * CSG_Parameters::Add_Info_Value			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, TSG_Parameter_Type Type, double Value)
{	return( Add_Info_Value			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Type, Value) );	}

CSG_Parameter * CSG_Parameters::Add_Range				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, double Range_Min, double Range_Max, double Minimum, bool bMinimum, double Maximum, bool bMaximum)
{	return( Add_Range				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Range_Min, Range_Max, Minimum, bMinimum, Maximum, bMaximum) );	}

CSG_Parameter * CSG_Parameters::Add_Info_Range			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, double Range_Min, double Range_Max)
{	return( Add_Info_Range			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Range_Min, Range_Max) );	}

CSG_Parameter * CSG_Parameters::Add_Choice				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *Items, int Default)
{	return( Add_Choice				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Items, Default) );	}

CSG_Parameter * CSG_Parameters::Add_String				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *String, bool bLongText, bool bPassword)
{	return( Add_String				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, String, bLongText, bPassword) );	}

CSG_Parameter * CSG_Parameters::Add_Info_String			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *String, bool bLongText)
{	return( Add_Info_String			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, String, bLongText) );	}

CSG_Parameter * CSG_Parameters::Add_FilePath			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, const SG_Char *Filter, const SG_Char *Default, bool bSave, bool bDirectory, bool bMultiple)
{	return( Add_FilePath			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Filter, Default, bSave, bDirectory, bMultiple) );	}

CSG_Parameter * CSG_Parameters::Add_Font				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, class wxFont *pInit)
{	return( Add_Font				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, pInit) );	}

CSG_Parameter * CSG_Parameters::Add_Colors				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, CSG_Colors      *pInit)
{	return( Add_Colors				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, pInit) );	}

CSG_Parameter * CSG_Parameters::Add_FixedTable			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, CSG_Table   *pTemplate)
{	return( Add_FixedTable			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, pTemplate) );	}

CSG_Parameter * CSG_Parameters::Add_Grid_System			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, CSG_Grid_System *pInit)
{	return( Add_Grid_System			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, pInit) );	}

CSG_Parameter * CSG_Parameters::Add_Grid				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, bool bSystem_Dependent, TSG_Grid_Type Preferred_Type)
{	return( Add_Grid				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint, bSystem_Dependent, Preferred_Type) );	}

CSG_Parameter * CSG_Parameters::Add_Grid_Output			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_Grid_Output			(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

CSG_Parameter * CSG_Parameters::Add_Grid_List			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, bool bSystem_Dependent)
{	return( Add_Grid_List			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint, bSystem_Dependent) );	}

CSG_Parameter * CSG_Parameters::Add_Table_Field			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_Table_Field			(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

CSG_Parameter * CSG_Parameters::Add_Table				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{	return( Add_Table				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint) );	}

CSG_Parameter * CSG_Parameters::Add_Table_Output		(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_Table_Output		(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

CSG_Parameter * CSG_Parameters::Add_Table_List			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{	return( Add_Table_List			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint) );	}

CSG_Parameter * CSG_Parameters::Add_Shapes				(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, TSG_Shape_Type Shape_Type)
{	return( Add_Shapes				(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint, Shape_Type) );	}

CSG_Parameter * CSG_Parameters::Add_Shapes_Output		(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_Shapes_Output		(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

CSG_Parameter * CSG_Parameters::Add_Shapes_List			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint, TSG_Shape_Type Shape_Type)
{	return( Add_Shapes_List			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint, Shape_Type) );	}

CSG_Parameter * CSG_Parameters::Add_TIN					(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{	return( Add_TIN					(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint) );	}

CSG_Parameter * CSG_Parameters::Add_TIN_Output			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_TIN_Output			(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

CSG_Parameter * CSG_Parameters::Add_TIN_List			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description, int Constraint)
{	return( Add_TIN_List			(pParent, SG_STR_MBTOSG(Identifier), Name, Description, Constraint) );	}

CSG_Parameter * CSG_Parameters::Add_Parameters			(CSG_Parameter *pParent, const char *Identifier, const SG_Char *Name, const SG_Char *Description)
{	return( Add_Parameters			(pParent, SG_STR_MBTOSG(Identifier), Name, Description) );	}

#endif


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
