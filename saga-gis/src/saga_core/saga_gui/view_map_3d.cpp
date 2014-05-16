/**********************************************************
 * Version $Id$
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    User Interface                     //
//                                                       //
//                    Program: SAGA                      //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                   VIEW_Map_3D.cpp                     //
//                                                       //
//          Copyright (C) 2005 by Olaf Conrad            //
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
#include "res_commands.h"
#include "res_controls.h"
#include "res_dialogs.h"
#include "res_images.h"

#include "helper.h"

#include "wksp_data_manager.h"
#include "wksp_map.h"

#include "view_map_3d.h"
#include "view_map_3d_panel.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
enum
{
	MAP3D_STATUSBAR_ROTATE_X	= 0,
	MAP3D_STATUSBAR_ROTATE_Y,
	MAP3D_STATUSBAR_ROTATE_Z,
	MAP3D_STATUSBAR_SHIFT_X,
	MAP3D_STATUSBAR_SHIFT_Y,
	MAP3D_STATUSBAR_SHIFT_Z,
	MAP3D_STATUSBAR_EXAGGERATION,
	MAP3D_STATUSBAR_COUNT
};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
IMPLEMENT_CLASS(CVIEW_Map_3D, CVIEW_Base);

//---------------------------------------------------------
BEGIN_EVENT_TABLE(CVIEW_Map_3D, CVIEW_Base)
	EVT_SIZE			(CVIEW_Map_3D::On_Size)
	EVT_MENU_RANGE		(ID_CMD_MAP3D_FIRST, ID_CMD_MAP3D_LAST, CVIEW_Map_3D::On_Command)
	EVT_UPDATE_UI_RANGE	(ID_CMD_MAP3D_FIRST, ID_CMD_MAP3D_LAST, CVIEW_Map_3D::On_Command_UI)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CVIEW_Map_3D::CVIEW_Map_3D(CWKSP_Map *pMap)
	: CVIEW_Base(pMap, ID_VIEW_MAP_3D, _TL("3D View"), ID_IMG_WND_MAP3D, false)
{
	SetTitle(wxString::Format(wxT("%s [%s]"), pMap->Get_Name().c_str(), _TL("3D View")));

	CreateStatusBar(MAP3D_STATUSBAR_COUNT);

	m_pPanel	= new CView_Map_3DPanel(this, pMap);

	//-----------------------------------------------------
	Parameters_Create();

	if( DLG_Parameters(&m_Parameters) )
	{
		Parameters_Update(false);

		Do_Show();

		m_pPanel->SetFocus();
	}
	else
	{
		Destroy();
	}
}

//---------------------------------------------------------
CVIEW_Map_3D::~CVIEW_Map_3D(void)
{
	m_pPanel->Destroy();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
wxMenu * CVIEW_Map_3D::_Create_Menu(void)
{
	wxMenu	*pMenu	= new wxMenu, *pMenu_Sub;

	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_PARAMETERS);

	pMenu->Append(ID_CMD_MAP3D_FIRST, _TL("Rotation"), pMenu_Sub = new wxMenu());
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_ROTATE_X_LESS);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_ROTATE_X_MORE);
//	pMenu_Sub->AppendSeparator();
//	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_ROTATE_Y_LESS);
//	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_ROTATE_Y_MORE);
	pMenu_Sub->AppendSeparator();
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_ROTATE_Z_LESS);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_ROTATE_Z_MORE);

	pMenu->Append(ID_CMD_MAP3D_FIRST, _TL("Shift"), pMenu_Sub = new wxMenu());
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SHIFT_X_LESS);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SHIFT_X_MORE);
	pMenu_Sub->AppendSeparator();
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SHIFT_Y_LESS);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SHIFT_Y_MORE);
	pMenu_Sub->AppendSeparator();
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SHIFT_Z_LESS);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SHIFT_Z_MORE);

	pMenu->Append(ID_CMD_MAP3D_FIRST, _TL("Sequencer"), pMenu_Sub = new wxMenu());
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SEQ_POS_ADD);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SEQ_POS_DEL);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SEQ_POS_DEL_ALL);
	CMD_Menu_Add_Item(pMenu_Sub, false, ID_CMD_MAP3D_SEQ_POS_EDIT);
	pMenu_Sub->AppendSeparator();
	CMD_Menu_Add_Item(pMenu_Sub,  true, ID_CMD_MAP3D_SEQ_PLAY);
	CMD_Menu_Add_Item(pMenu_Sub,  true, ID_CMD_MAP3D_SEQ_PLAY_LOOP);
	CMD_Menu_Add_Item(pMenu_Sub,  true, ID_CMD_MAP3D_SEQ_SAVE);

	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_EXAGGERATE_LESS);
	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_EXAGGERATE_MORE);
	pMenu->AppendSeparator();
	CMD_Menu_Add_Item(pMenu    ,  true, ID_CMD_MAP3D_CENTRAL);
	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_CENTRAL_LESS);
	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_CENTRAL_MORE);
	pMenu->AppendSeparator();
	CMD_Menu_Add_Item(pMenu    ,  true, ID_CMD_MAP3D_STEREO);
	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_STEREO_LESS);
	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_STEREO_MORE);
	pMenu->AppendSeparator();
	CMD_Menu_Add_Item(pMenu    ,  true, ID_CMD_MAP3D_INTERPOLATED);
	CMD_Menu_Add_Item(pMenu    , false, ID_CMD_MAP3D_SAVE);

	return( pMenu );
}

//---------------------------------------------------------
wxToolBarBase * CVIEW_Map_3D::_Create_ToolBar(void)
{
	wxToolBarBase	*pToolBar	= CMD_ToolBar_Create(ID_TB_VIEW_MAP_3D);

	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_PARAMETERS);
	CMD_ToolBar_Add_Separator(pToolBar);
	CMD_ToolBar_Add_Item(pToolBar,  true, ID_CMD_MAP3D_STEREO);
	CMD_ToolBar_Add_Item(pToolBar,  true, ID_CMD_MAP3D_INTERPOLATED);
	CMD_ToolBar_Add_Separator(pToolBar);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_ROTATE_X_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_ROTATE_X_MORE);
//	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_ROTATE_Y_LESS);
//	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_ROTATE_Y_MORE);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_ROTATE_Z_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_ROTATE_Z_MORE);
	CMD_ToolBar_Add_Separator(pToolBar);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_SHIFT_X_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_SHIFT_X_MORE);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_SHIFT_Y_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_SHIFT_Y_MORE);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_SHIFT_Z_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_SHIFT_Z_MORE);
	CMD_ToolBar_Add_Separator(pToolBar);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_EXAGGERATE_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_EXAGGERATE_MORE);
	CMD_ToolBar_Add_Separator(pToolBar);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_CENTRAL_LESS);
	CMD_ToolBar_Add_Item(pToolBar, false, ID_CMD_MAP3D_CENTRAL_MORE);

	CMD_ToolBar_Add(pToolBar, _TL("3D-View"));

	return( pToolBar );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Map_3D::Do_Update(void)
{
	m_pPanel->Update_View(true);
}

//---------------------------------------------------------
void CVIEW_Map_3D::Update_StatusBar(void)
{
	if( m_pPanel )
	{
		SetStatusText(wxString::Format(wxT("X %+.1f\xb0"), m_pPanel->Get_Projector().Get_xRotation() * M_RAD_TO_DEG), MAP3D_STATUSBAR_ROTATE_X);
		SetStatusText(wxString::Format(wxT("Y %+.1f\xb0"), m_pPanel->Get_Projector().Get_yRotation() * M_RAD_TO_DEG), MAP3D_STATUSBAR_ROTATE_Y);
		SetStatusText(wxString::Format(wxT("Z %+.1f\xb0"), m_pPanel->Get_Projector().Get_zRotation() * M_RAD_TO_DEG), MAP3D_STATUSBAR_ROTATE_Z);

		SetStatusText(wxString::Format(wxT("X %+.1f"    ), m_pPanel->Get_Projector().Get_xShift()), MAP3D_STATUSBAR_SHIFT_X);
		SetStatusText(wxString::Format(wxT("Y %+.1f"    ), m_pPanel->Get_Projector().Get_yShift()), MAP3D_STATUSBAR_SHIFT_Y);
		SetStatusText(wxString::Format(wxT("Z %+.1f"    ), m_pPanel->Get_Projector().Get_zShift()), MAP3D_STATUSBAR_SHIFT_Z);

		SetStatusText(wxString::Format(wxT("E %.1f"     ), m_pPanel->m_zScale), MAP3D_STATUSBAR_EXAGGERATION);
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Map_3D::On_Size(wxSizeEvent &event)
{
	m_pPanel->SetSize(GetClientRect());

	event.Skip();
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Map_3D::On_Command(wxCommandEvent &event)
{
	switch( event.GetId() )
	{
	default:	return;

	//-----------------------------------------------------
	case ID_CMD_MAP3D_PARAMETERS:
		{
			Parameters_Update(true);

			if( DLG_Parameters(&m_Parameters) )
			{
				Parameters_Update(false);
			}
		}
		return;

	//-----------------------------------------------------
	case ID_CMD_MAP3D_SAVE:
		{
			int			FileType;
			wxString	FileName;

			if( DLG_Image_Save(FileName, FileType) )
			{
				m_pPanel->Save_asImage(&FileName);
			}
		}
		return;

	//-----------------------------------------------------
	case ID_CMD_MAP3D_SEQ_POS_EDIT:
		{
			DLG_Table(_TL("Edit 3D-View Sequencer Positions"), m_pPanel->m_Parameters("PLAY")->asTable());
		}
		return;

	//-----------------------------------------------------
	case ID_CMD_MAP3D_SEQ_POS_ADD    :	m_pPanel->Play_Pos_Add();	return;
	case ID_CMD_MAP3D_SEQ_POS_DEL    :	m_pPanel->Play_Pos_Del();	return;
	case ID_CMD_MAP3D_SEQ_POS_DEL_ALL:	m_pPanel->Play_Pos_Clr();	return;
	case ID_CMD_MAP3D_SEQ_PLAY       :	m_pPanel->Play_Once   ();	return;
	case ID_CMD_MAP3D_SEQ_PLAY_LOOP  :	m_pPanel->Play_Loop   ();	return;
	case ID_CMD_MAP3D_SEQ_SAVE       :	m_pPanel->Play_Save   ();	return;

	//-----------------------------------------------------
	case ID_CMD_MAP3D_ROTATE_X_LESS  :	m_pPanel->Get_Projector().Set_xRotation(m_pPanel->Get_Projector().Get_xRotation() + 4.0 * M_DEG_TO_RAD);	break;
	case ID_CMD_MAP3D_ROTATE_X_MORE  :	m_pPanel->Get_Projector().Set_xRotation(m_pPanel->Get_Projector().Get_xRotation() - 4.0 * M_DEG_TO_RAD);	break;
	case ID_CMD_MAP3D_ROTATE_Y_LESS  :	m_pPanel->Get_Projector().Set_yRotation(m_pPanel->Get_Projector().Get_yRotation() + 4.0 * M_DEG_TO_RAD);	break;
	case ID_CMD_MAP3D_ROTATE_Y_MORE  :	m_pPanel->Get_Projector().Set_yRotation(m_pPanel->Get_Projector().Get_yRotation() - 4.0 * M_DEG_TO_RAD);	break;
	case ID_CMD_MAP3D_ROTATE_Z_LESS  :	m_pPanel->Get_Projector().Set_zRotation(m_pPanel->Get_Projector().Get_zRotation() + 4.0 * M_DEG_TO_RAD);	break;
	case ID_CMD_MAP3D_ROTATE_Z_MORE  :	m_pPanel->Get_Projector().Set_zRotation(m_pPanel->Get_Projector().Get_zRotation() - 4.0 * M_DEG_TO_RAD);	break;

	case ID_CMD_MAP3D_SHIFT_X_LESS   :	m_pPanel->Get_Projector().Set_xShift(m_pPanel->Get_Projector().Get_xShift() - 10.0);	break;
	case ID_CMD_MAP3D_SHIFT_X_MORE   :	m_pPanel->Get_Projector().Set_xShift(m_pPanel->Get_Projector().Get_xShift() + 10.0);	break;
	case ID_CMD_MAP3D_SHIFT_Y_LESS   :	m_pPanel->Get_Projector().Set_yShift(m_pPanel->Get_Projector().Get_yShift() + 10.0);	break;
	case ID_CMD_MAP3D_SHIFT_Y_MORE   :	m_pPanel->Get_Projector().Set_yShift(m_pPanel->Get_Projector().Get_yShift() - 10.0);	break;
	case ID_CMD_MAP3D_SHIFT_Z_LESS   :	m_pPanel->Get_Projector().Set_zShift(m_pPanel->Get_Projector().Get_zShift() - 10.0);	break;
	case ID_CMD_MAP3D_SHIFT_Z_MORE   :	m_pPanel->Get_Projector().Set_zShift(m_pPanel->Get_Projector().Get_zShift() + 10.0);	break;

//	case ID_CMD_MAP3D_DRAW_BOX       :	m_pPanel->m_Parameters("DRAW_BOX"    )->Set_Value(m_pPanel->m_Parameters("DRAW_BOX"    )->asBool() == false    );	break;
	case ID_CMD_MAP3D_STEREO         :	m_pPanel->m_Parameters("STEREO"      )->Set_Value(m_pPanel->m_Parameters("STEREO"      )->asBool() == false    );	break;
	case ID_CMD_MAP3D_STEREO_LESS    :	m_pPanel->m_Parameters("STEREO_DIST" )->Set_Value(m_pPanel->m_Parameters("STEREO_DIST" )->asDouble() - 2       );	break;
	case ID_CMD_MAP3D_STEREO_MORE    :	m_pPanel->m_Parameters("STEREO_DIST" )->Set_Value(m_pPanel->m_Parameters("STEREO_DIST" )->asDouble() + 2       );	break;
	case ID_CMD_MAP3D_CENTRAL        :	m_pPanel->m_Parameters("CENTRAL"     )->Set_Value(m_pPanel->m_Parameters("CENTRAL"     )->asBool() == false    );	break;
	case ID_CMD_MAP3D_CENTRAL_LESS   :	m_pPanel->m_Parameters("CENTRAL_DIST")->Set_Value(m_pPanel->m_Parameters("CENTRAL_DIST")->asDouble() - 50      );	break;
	case ID_CMD_MAP3D_CENTRAL_MORE   :	m_pPanel->m_Parameters("CENTRAL_DIST")->Set_Value(m_pPanel->m_Parameters("CENTRAL_DIST")->asDouble() + 50      );	break;
	case ID_CMD_MAP3D_INTERPOLATED   :	m_pPanel->m_Parameters("DRAPE_MODE"  )->Set_Value(m_pPanel->m_Parameters("DRAPE_MODE"  )->asInt() == 0 ? 1 : 0 );	break;

	case ID_CMD_MAP3D_EXAGGERATE_LESS:	m_pPanel->m_zScale	-= 0.5;	break;
	case ID_CMD_MAP3D_EXAGGERATE_MORE:	m_pPanel->m_zScale	+= 0.5;	break;

//	case ID_CMD_MAP3D_SRC_RES_LESS   :	m_pPanel->;	break;
//	case ID_CMD_MAP3D_SRC_RES_MORE   :	m_pPanel->;	break;
	}

	Parameters_Update(true);
	m_pPanel->Update_View();
}

//---------------------------------------------------------
void CVIEW_Map_3D::On_Command_UI(wxUpdateUIEvent &event)
{
	switch( event.GetId() )
	{
	case ID_CMD_MAP3D_CENTRAL:
		event.Check(m_pPanel->Get_Projector().is_Central());
		break;

	case ID_CMD_MAP3D_CENTRAL_LESS:
	case ID_CMD_MAP3D_CENTRAL_MORE:
		event.Enable(m_pPanel->Get_Projector().is_Central());
		break;

	case ID_CMD_MAP3D_INTERPOLATED:
		event.Check(m_pPanel->m_Parameters("DRAPE_MODE")->asInt() != 0);
		break;

	case ID_CMD_MAP3D_STEREO:
		event.Check(m_pPanel->m_Parameters("STEREO")->asBool());
		break;

	case ID_CMD_MAP3D_SEQ_PLAY:
		event.Check(m_pPanel->Play_Get_State() == SG_3DVIEW_PLAY_RUN_ONCE);
		break;

	case ID_CMD_MAP3D_SEQ_PLAY_LOOP:
		event.Check(m_pPanel->Play_Get_State() == SG_3DVIEW_PLAY_RUN_LOOP);
		break;

	case ID_CMD_MAP3D_SEQ_SAVE:
		event.Check(m_pPanel->Play_Get_State() == SG_3DVIEW_PLAY_RUN_SAVE);
		break;
	}
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CVIEW_Map_3D::Parameters_Update(bool bFromPanel)
{
	if( bFromPanel )
	{
		m_Parameters("ROTATION_X"  )->Set_Value(m_pPanel->Get_Projector().Get_xRotation() * M_RAD_TO_DEG);
		m_Parameters("ROTATION_Y"  )->Set_Value(m_pPanel->Get_Projector().Get_yRotation() * M_RAD_TO_DEG);
		m_Parameters("ROTATION_Z"  )->Set_Value(m_pPanel->Get_Projector().Get_zRotation() * M_RAD_TO_DEG);

		m_Parameters("SHIFT_X"     )->Set_Value(m_pPanel->Get_Projector().Get_xShift());
		m_Parameters("SHIFT_Y"     )->Set_Value(m_pPanel->Get_Projector().Get_yShift());
		m_Parameters("SHIFT_Z"     )->Set_Value(m_pPanel->Get_Projector().Get_zShift());

		m_Parameters("CENTRAL"     )->Set_Value(m_pPanel->Get_Projector().is_Central() ? 1 : 0);
		m_Parameters("CENTRAL_DIST")->Set_Value(m_pPanel->Get_Projector().Get_Central_Distance());

		m_Parameters("BGCOLOR"     )->Set_Value(m_pPanel->m_Parameters("BGCOLOR"    )->asInt());
		m_Parameters("STEREO"      )->Set_Value(m_pPanel->m_Parameters("STEREO"     )->asInt());
		m_Parameters("STEREO_DIST" )->Set_Value(m_pPanel->m_Parameters("STEREO_DIST")->asDouble());

		m_Parameters("Z_SCALE"     )->Set_Value(m_pPanel->m_zScale);
		m_Parameters("DEM_RES"     )->Set_Value(m_pPanel->Get_DEM_Res());
		m_Parameters("MAP_RES"     )->Set_Value(m_pPanel->Get_Map_Res());
	}

	//-----------------------------------------------------
	else
	{
		m_pPanel->Get_Projector().Set_Rotation(
			m_Parameters("ROTATION_X")->asDouble() * M_DEG_TO_RAD,
			m_Parameters("ROTATION_Y")->asDouble() * M_DEG_TO_RAD,
			m_Parameters("ROTATION_Z")->asDouble() * M_DEG_TO_RAD
		);

		m_pPanel->Get_Projector().Set_Shift(
			m_Parameters("SHIFT_X")->asDouble(),
			m_Parameters("SHIFT_Y")->asDouble(),
			m_Parameters("SHIFT_Z")->asDouble()
		);

		m_pPanel->Get_Projector().do_Central(
			m_Parameters("CENTRAL")->asInt() == 1
		);

		m_pPanel->Get_Projector().Set_Central_Distance(
			m_Parameters("CENTRAL_DIST")->asDouble()
		);

		m_pPanel->m_Parameters("BGCOLOR"    )->Set_Value(m_Parameters("BGCOLOR"    )->asInt());
		m_pPanel->m_Parameters("STEREO"     )->Set_Value(m_Parameters("STEREO"     )->asInt());
		m_pPanel->m_Parameters("STEREO_DIST")->Set_Value(m_Parameters("STEREO_DIST")->asDouble());
		m_pPanel->m_Parameters("DRAPE_MODE" )->Set_Value(m_Parameters("DRAPE_MODE" )->asInt());

		m_pPanel->m_zScale	= m_Parameters("Z_SCALE")->asDouble();

		//-------------------------------------------------
		CSG_Grid	*pDEM	= m_Parameters("DEM")->asGrid();

		if( !m_pPanel->Set_Options(
			SG_Get_Data_Manager().Exists(pDEM) ? pDEM : NULL,
			m_Parameters("DEM_RES")->asInt(),
			m_Parameters("MAP_RES")->asInt()) )
		{
			m_pPanel->Update_View();
		}
	}

	Update_StatusBar();
}

//---------------------------------------------------------
void CVIEW_Map_3D::Parameters_Create(void)
{
	CSG_Parameter	*pNode;

	m_Parameters.Create(NULL, _TL("3D-View"), _TL(""));

	//-----------------------------------------------------
	pNode	= m_Parameters.Add_Grid(
		NULL	, "DEM"			, _TL("Elevation"),
		_TL(""),
		PARAMETER_INPUT
	);

	m_Parameters.Add_Value(
		pNode	, "DEM_RES"		, _TL("Resolution"),
		_TL(""),
		PARAMETER_TYPE_Int, 100, 2, true
	);

	m_Parameters.Add_Value(
		pNode	, "Z_SCALE"		, _TL("Exaggeration"),
		_TL(""),
		PARAMETER_TYPE_Double, 1.0
	);

	//-----------------------------------------------------
	pNode	= m_Parameters.Add_Node(NULL, "MAP", _TL("Map"), _TL(""));

	m_Parameters.Add_Value(
		pNode	, "MAP_RES"		, _TL("Resolution"),
		_TL(""),
		PARAMETER_TYPE_Int, 1000, 2, true
	);

	m_Parameters.Add_Choice(
		pNode	, "DRAPE_MODE"	, _TL("Map Draping Interpolation"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|"),
			_TL("None"),
			_TL("Bilinear"),
			_TL("Inverse Distance"),
			_TL("Bicubic Spline"),
			_TL("B-Spline")
		), 0
	);

	//-----------------------------------------------------
	pNode	= m_Parameters.Add_Node(NULL, "ROTATION", _TL("Rotation"), _TL(""));

	m_Parameters.Add_Value(pNode, "ROTATION_X", _TL("X"), _TL(""), PARAMETER_TYPE_Double,  55.0, -360.0, true, 360.0, true);
	m_Parameters.Add_Value(pNode, "ROTATION_Y", _TL("Y"), _TL(""), PARAMETER_TYPE_Double,   0.0, -360.0, true, 360.0, true);
	m_Parameters.Add_Value(pNode, "ROTATION_Z", _TL("Z"), _TL(""), PARAMETER_TYPE_Double, -45.0, -360.0, true, 360.0, true);

	//-----------------------------------------------------
	pNode	= m_Parameters.Add_Node(NULL, "SHIFT", _TL("Shift"), _TL(""));

	m_Parameters.Add_Value(pNode, "SHIFT_X", _TL("Left/Right"), _TL(""), PARAMETER_TYPE_Double,    0.0);
	m_Parameters.Add_Value(pNode, "SHIFT_Y", _TL("Up/Down"   ), _TL(""), PARAMETER_TYPE_Double,    0.0);
	m_Parameters.Add_Value(pNode, "SHIFT_Z", _TL("In/Out"    ), _TL(""), PARAMETER_TYPE_Double, 1500.0);

	//-----------------------------------------------------
	pNode	= m_Parameters.Add_Node(NULL, "NODE_PROJECTION", _TL("Projection"), _TL(""));

	m_Parameters.Add_Choice(
		pNode	, "CENTRAL"			, _TL("Projection"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|"),
			_TL("parallel"),
			_TL("central")
		), 1
	);

	m_Parameters.Add_Value(
		pNode	, "CENTRAL_DIST"	, _TL("Perspectivic Distance"),
		_TL(""),
		PARAMETER_TYPE_Double, 200, 1, true
	);

	//-----------------------------------------------------
	pNode	= m_Parameters.Add_Node(NULL, "NODE_STEREO", _TL("Anaglyph"), _TL(""));

	m_Parameters.Add_Value(
		pNode	, "STEREO"			, _TL("Anaglyph"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);

	m_Parameters.Add_Value(
		pNode	, "STEREO_DIST"		, _TL("Eye Distance [Degree]"),
		_TL(""),
		PARAMETER_TYPE_Double, 2.0, 0, true, 180, true
	);

	//-----------------------------------------------------
	m_Parameters.Add_Value(
		NULL	, "BGCOLOR"			, _TL("Background Color"),
		_TL(""),
		PARAMETER_TYPE_Color, SG_GET_RGB(0, 0, 0)
	);
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
