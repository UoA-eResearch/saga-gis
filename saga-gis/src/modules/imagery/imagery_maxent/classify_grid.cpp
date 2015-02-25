/**********************************************************
 * Version $Id: template.cpp 911 2011-11-11 11:11:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                        maxent                         //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                   classify_grid.cpp                   //
//                                                       //
//                 Copyright (C) 2015 by                 //
//                     Olaf Conrad                       //
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
//    e-mail:     oconrad@saga-gis.de                    //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Hamburg                  //
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
#include "classify_grid.h"

//---------------------------------------------------------
using namespace std;


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CClassify_Grid::CClassify_Grid(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("Maximum Entropy Classifcation"));

	Set_Author		("O.Conrad (c) 2015");

	Set_Description	(_TW(
		"\nReferences:\n"
		"- Yoshimasa Tsuruoka: A simple C++ library for maximum entropy classification. "
		" <a target=\"_blank\" href=\"http://www.logos.t.u-tokyo.ac.jp/~tsuruoka/maxent/\">online</a>.\n"
		"- Dekang Lin: A MaxEnt Package in C++. "
		" <a target=\"_blank\" href=\"http://webdocs.cs.ualberta.ca/~lindek/downloads.htm\">online</a>.\n"
	));

	//-----------------------------------------------------
	pNode	= Parameters.Add_Shapes(
		NULL	, "TRAINING"		, _TL("Training Areas"),
		_TL(""),
		PARAMETER_INPUT, SHAPE_TYPE_Polygon
	);

	Parameters.Add_Table_Field(
		pNode	, "FIELD"			, _TL("Class Name"),
		_TL("")
	);

	Parameters.Add_Grid_List(
		NULL	, "FEATURES"		, _TL("Features"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Grid(
		NULL	, "CLASSES"			, _TL("Classes"),
		_TL(""),
		PARAMETER_OUTPUT, true, SG_DATATYPE_Short
	);

	Parameters.Add_Grid(
		NULL	, "PROB"			, _TL("Probability"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	Parameters.Add_Grid_List(
		NULL	, "PROBS"			, _TL("Probabilities"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	Parameters.Add_Value(
		NULL	, "PROBS_CREATE"	, _TL("Create Propabilities"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);

	//-----------------------------------------------------
	Parameters.Add_Value(
		NULL	, "PROB_MIN"		, _TL("Minimum Probability"),
		_TL("Minimum probability to accept a classification result for a cell."),
		PARAMETER_TYPE_Double, 0.0, 0.0, true, 1.0, true
	);

	Parameters.Add_Value(
		NULL	, "NUM_CLASSES"		, _TL("Numeric Value Classes"),
		_TL(""),
		PARAMETER_TYPE_Int, 32, 1, true
	);

	Parameters.Add_Choice(
		NULL	, "METHOD"			, _TL("Method"),
		_TL(""),
		CSG_String::Format("%s|%s|",
			_TL("Yoshimasa Tsuruoka"),
			_TL("Dekang Lin")
		)
	);

	//-----------------------------------------------------
	pNode	= Parameters.Add_Choice(
		NULL	, "YT_REGUL"		, _TL("Regularization"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|"),
			_TL("none"),
			SG_T("L1"),
			SG_T("L2")
		), 1
	);

	Parameters.Add_Value(
		pNode	, "YT_REGUL_VAL"	, _TL("Regularization Factor"),
		_TL(""),
		PARAMETER_TYPE_Double, 1.0, 0.0, true
	);

	Parameters.Add_Value(
		NULL	, "YT_NUMASREAL"	, _TL("Real-valued Numeric Features"),
		_TL(""),
		PARAMETER_TYPE_Bool, true
	);

	Parameters.Add_FilePath(
		NULL	, "YT_FILE_SAVE"	, _TL("Save to File..."),
		_TL(""),
		NULL, NULL, true
	);

	Parameters.Add_FilePath(
		NULL	, "YT_FILE_LOAD"	, _TL("Load from File..."),
		_TL(""),
		NULL, NULL, false
	);

	//-----------------------------------------------------
	Parameters.Add_Value(
		NULL	, "DL_ALPHA"		, _TL("Alpha"),
		_TL(""),
		PARAMETER_TYPE_Double, 0.1
	);

	Parameters.Add_Value(
		NULL	, "DL_THRESHOLD"	, _TL("Threshold"),
		_TL(""),
		PARAMETER_TYPE_Double, 0.0, 0.0, true
	);

	Parameters.Add_Value(
		NULL	, "DL_ITERATIONS"	, _TL("Maximum Iterations"),
		_TL(""),
		PARAMETER_TYPE_Int, 100, 1, true
	);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CClassify_Grid::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	bool	bFile	= SG_File_Exists(pParameters->Get_Parameter("YT_FILE_LOAD")->asString());
	int		Method	= pParameters->Get_Parameter("METHOD")->asInt();

	pParameters->Set_Enabled("NUM_CLASSES"  , Method == 1 || !pParameters->Get_Parameter("YT_NUMASREAL")->asBool());

	pParameters->Set_Enabled("TRAINING"     , Method == 1 || !bFile);

	pParameters->Set_Enabled("YT_REGUL"     , Method == 0 && !bFile);
	pParameters->Set_Enabled("YT_REGUL_VAL" , Method == 0 && !bFile);
	pParameters->Set_Enabled("YT_FILE_SAVE" , Method == 0 && !bFile);
	pParameters->Set_Enabled("YT_FILE_LOAD" , Method == 0);
	pParameters->Set_Enabled("YT_NUMASREAL" , Method == 0);

	pParameters->Set_Enabled("DL_ALPHA"     , Method == 1);
	pParameters->Set_Enabled("DL_THRESHOLD" , Method == 1);
	pParameters->Set_Enabled("DL_ITERATIONS", Method == 1);

	return( 1 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CClassify_Grid::On_Execute(void)
{
	//-----------------------------------------------------
	EventSet	DL_Events ;	m_DL_Events  = &DL_Events ;
	GISTrainer	DL_Trainer;	m_DL_Trainer = &DL_Trainer;
	MaxEntModel	DL_Model  ;	m_DL_Model   = &DL_Model  ;

	m_YT_Model.clear();

	//-----------------------------------------------------
	CSG_Grid	*pClasses	= Parameters("CLASSES")->asGrid();
	CSG_Grid	*pProb		= Parameters("PROB"   )->asGrid();

	m_pProbs		= Parameters("PROBS_CREATE")->asBool() ? Parameters("PROBS")->asGridList() : NULL;
	m_Method		= Parameters("METHOD"      )->asInt ();
	m_nNumClasses	= Parameters("NUM_CLASSES" )->asInt ();
	m_bYT_Weights	= Parameters("YT_NUMASREAL")->asBool();

	//-----------------------------------------------------
	CSG_Array	Features;

	if( !Get_Features(Features) )
	{
		Error_Set(_TL("invalid features"));

		return( false );
	}

	//-----------------------------------------------------
	if( m_Method == 0 && SG_File_Exists(Parameters("YT_FILE_LOAD")->asString()) )
	{
		if( !Get_File(Parameters("YT_FILE_LOAD")->asString()) )
		{
			return( false );
		}
	}
	else if( !Get_Training() )
	{
		return( false );
	}

	pClasses->Set_NoData_Value(-1);

	if( !pProb->Get_ZRange() )	DataObject_Set_Colors(pProb, 11, SG_COLORS_YELLOW_GREEN);

	//-----------------------------------------------------
	Process_Set_Text(_TL("prediction"));

	double	minProb	= Parameters("PROB_MIN")->asDouble();

	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			int			i;
			CSG_Strings	Values;

			for(i=0; i<m_nFeatures; i++)
			{
				if( !m_Features[i].pGrid->is_NoData(x, y) )
				{
					Values.Add(Get_Feature(x, y, i));
				}
				else
				{
					break;
				}
			}

			if( Values.Get_Count() != m_nFeatures )
			{
				pClasses->Set_NoData(x, y);

				for(i=0; m_pProbs && i<m_pProbs->Get_Count(); i++)
				{
					m_pProbs->asGrid(i)->Set_NoData(x, y);
				}
			}
			else switch( m_Method )
			{
			//---------------------------------------------
			default:	// Kyoshida
				{
					ME_Sample	Sample;

					for(i=0; i<m_nFeatures; i++)
					{
						if( m_bYT_Weights && m_Features[i].bNumeric )
						{
							Sample.add_feature(SG_Get_String(i, 0).b_str(), m_Features[i].pGrid->asDouble(x, y));
						}
						else
						{
							Sample.add_feature(Values[i].b_str());
						}
					}

					vector<double> Probs	= m_YT_Model.classify(Sample);

					pProb   ->Set_Value(x, y, Probs[i = m_YT_Model.get_class_id(Sample.label)]);
					pClasses->Set_Value(x, y, Probs[i] >= minProb ? i : -1);

					for(i=0; m_pProbs && i<m_pProbs->Get_Count() && i<(int)Probs.size(); i++)
					{
						m_pProbs->asGrid(i)->Set_Value(x, y, Probs[i]);
					}
				}
				break;

			//---------------------------------------------
			case  1:	// Dekang Lin
				{
					MaxEntEvent Event;	Event.count(1);

					for(i=0; i<m_nFeatures; i++)
					{
						Event.push_back(m_DL_Trainer->getId(Values[i].b_str()));
					}

					vector<double> Probs;

					pProb   ->Set_Value(x, y, Probs[i = m_DL_Model->getProbs(Event, Probs)]);
					pClasses->Set_Value(x, y, Probs[i] >= minProb ? i : -1);

					for(i=0; m_pProbs && i<m_pProbs->Get_Count() && i<(int)Probs.size(); i++)
					{
						m_pProbs->asGrid(i)->Set_Value(x, y, Probs[i]);
					}
				}
				break;
			}
		}
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_String CClassify_Grid::Get_Feature(int x, int y, int i)
{
	CSG_Grid	*pFeature	= m_Features[i].pGrid;

	if( m_nNumClasses > 1 && m_Features[i].bNumeric )
	{
		return( CSG_String::Format("%d", (int)(m_nNumClasses * (pFeature->asDouble(x, y) - pFeature->Get_ZMin()) / pFeature->Get_ZRange())) );
	}

	return( SG_Get_String(pFeature->asDouble(x, y), -2) );
}

//---------------------------------------------------------
bool CClassify_Grid::Get_Features(CSG_Array &Features)
{
	CSG_Parameter_Grid_List	*pFeatures	= Parameters("FEATURES")->asGridList();

	m_Features	= (TFeature *)Features.Create(sizeof(TFeature), m_nFeatures = pFeatures->Get_Count());

	for(int i=0; i<m_nFeatures; i++)
	{
		m_Features[i].pGrid		= pFeatures->asGrid(i);
		m_Features[i].bNumeric	= true;
	}

	return( m_nFeatures > 0 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CClassify_Grid::Get_Training(void)
{
	//-----------------------------------------------------
	CSG_Shapes	*pTraining	= Parameters("TRAINING")->asShapes();
	int			Field		= Parameters("FIELD"   )->asInt   ();

	if( pTraining->Get_Count() <= 0 )
	{
		Error_Set(_TL("invalid training data"));

		return( false );
	}

	//-----------------------------------------------------
	int				nClasses	= 0;
	CSG_String		Name;
	CSG_Parameter	*pLUT	= DataObject_Get_Parameter(Parameters("CLASSES")->asGrid(), "LUT");

	pTraining->Set_Index(Field, TABLE_INDEX_Ascending);

	for(int i=0; i<pTraining->Get_Count(); i++)
	{
		CSG_Shape	*pArea	= pTraining->Get_Shape_byIndex(i);

		if( i == 0 || Name.Cmp(pArea->asString(Field)) )
		{
			Name	= pTraining->Get_Shape_byIndex(i)->asString(Field);

			if( m_pProbs )
			{
				CSG_Grid	*pGrid	= m_pProbs->asGrid(i);

				if( !pGrid )
				{
					m_pProbs->Add_Item(pGrid = SG_Create_Grid(*Get_System()));

					DataObject_Set_Colors(pGrid, 11, SG_COLORS_YELLOW_GREEN);
				}

				pGrid->Set_Name(Name);
			}

			if( pLUT && pLUT->asTable() )
			{
				CSG_Table_Record	*pClass	= pLUT->asTable()->Get_Record(nClasses);

				if( !pClass )
				{
					(pClass	= pLUT->asTable()->Add_Record())->Set_Value(0, SG_Color_Get_Random());
				}

				pClass->Set_Value(1, Name);
				pClass->Set_Value(3, nClasses);
				pClass->Set_Value(4, nClasses);
			}

			if( m_Method == 1 )	// Dekang Lin
			{
				m_DL_Trainer->addClass(Name.b_str());
			}

			nClasses++;
		}

		Get_Training(Name, (CSG_Shape_Polygon *)pArea);
	}

	if( nClasses <= 1 )
	{
		Error_Set(_TL("only one class in training data"));

		return( false );
	}

	if( pLUT && pLUT->asTable() )
	{
		pLUT->asTable()->Set_Record_Count(nClasses);

		DataObject_Set_Parameter(Parameters("CLASSES")->asGrid(), pLUT);
		DataObject_Set_Parameter(Parameters("CLASSES")->asGrid(), "COLORS_TYPE", 1);	// Color Classification Type: Lookup Table
	}

	//-----------------------------------------------------
	Process_Set_Text(_TL("training"));

	switch( m_Method )
	{
	//-----------------------------------------------------
	default:	// Kyoshida
		switch( Parameters("YT_REGUL")->asInt() )
		{
		default:
			m_YT_Model.use_l1_regularizer(0.0);
			m_YT_Model.use_l2_regularizer(0.0);
			break;

		case  1:
			m_YT_Model.use_l1_regularizer(Parameters("YT_REGUL_VAL")->asDouble());
			m_YT_Model.use_l2_regularizer(0.0);
			break;

		case  2:
			m_YT_Model.use_l1_regularizer(0.0);
			m_YT_Model.use_l2_regularizer(Parameters("YT_REGUL_VAL")->asDouble());
			break;
		}

	//	m_YT_Model.use_SGD(Parameters("SGD_ITER")->asInt());

		m_YT_Model.train();

		{
		/*	list< pair< pair<string, string>, double > > Features;

			m_YT_Model.get_features(Features);

			for(list< pair< pair<string, string>, double> >::const_iterator i=Features.begin(); i!=Features.end(); i++)
			{
				Message_Add(CSG_String::Format("\n%10.3f  %-10s %s",
					i->second, i->first.first.c_str(), i->first.second.c_str()
				), false);
			}/**/

			CSG_String	File(Parameters("YT_FILE_SAVE")->asString());

			if( !File.is_Empty() )
			{
				m_YT_Model.save_to_file(File.b_str());
			}
		}
		break;

	//-----------------------------------------------------
	case  1:	// Dekang Lin
		m_DL_Trainer->printDetails(true);	// show the parameters during training

		m_DL_Trainer->Set_Alpha     (Parameters("DL_ALPHA"     )->asDouble());
		m_DL_Trainer->Set_Threshold (Parameters("DL_THRESHOLD" )->asDouble());
		m_DL_Trainer->Set_Iterations(Parameters("DL_ITERATIONS")->asInt());

		m_DL_Model->classes(m_DL_Trainer->classes().size());

		m_DL_Trainer->train(*m_DL_Model, *m_DL_Events);	// train the model

		break;
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
void CClassify_Grid::Get_Training(const CSG_String &ID, CSG_Shape_Polygon *pArea)
{
	int	xMin	= Get_System()->Get_xWorld_to_Grid(pArea->Get_Extent().Get_XMin());
	int	xMax	= Get_System()->Get_xWorld_to_Grid(pArea->Get_Extent().Get_XMax());
	int	yMin	= Get_System()->Get_yWorld_to_Grid(pArea->Get_Extent().Get_YMin());
	int	yMax	= Get_System()->Get_yWorld_to_Grid(pArea->Get_Extent().Get_YMax());

	if( xMin < 0 ) xMin = 0; else if( xMin >= Get_NX() ) xMin = Get_NX() - 1;
	if( xMax < 0 ) xMax = 0; else if( xMax >= Get_NX() ) xMax = Get_NX() - 1;
	if( yMin < 0 ) yMin = 0; else if( yMin >= Get_NY() ) yMin = Get_NY() - 1;
	if( yMax < 0 ) yMax = 0; else if( yMax >= Get_NY() ) yMax = Get_NY() - 1;

	//-----------------------------------------------------
	for(int y=yMin; y<=yMax; y++)
	{
		for(int x=xMin; x<=xMax; x++)
		{
			if( pArea->Contains(Get_System()->Get_Grid_to_World(x, y)) )
			{
				int			i;
				CSG_Strings	Values;

				for(i=0; i<m_nFeatures; i++)
				{
					if( !m_Features[i].pGrid->is_NoData(x, y) )
					{
						Values.Add(Get_Feature(x, y, i));
					}
					else
					{
						break;
					}
				}

				if( Values.Get_Count() == m_nFeatures )
				{
					switch( m_Method )
					{
					default:	// Kyoshida
						{
							ME_Sample	Sample(ID.b_str());

							for(i=0; i<m_nFeatures; i++)
							{
								if( m_bYT_Weights && m_Features[i].bNumeric )
								{
									Sample.add_feature(SG_Get_String(i, 0).b_str(), m_Features[i].pGrid->asDouble(x, y));
								}
								else
								{
									Sample.add_feature(Values[i].b_str());
								}
							}

							m_YT_Model.add_training_sample(Sample);
						}
						break;

					case  1:	// Dekang Lin
						{
							MaxEntEvent	*pEvent	= new MaxEntEvent;

							pEvent->count(1);
							pEvent->classId(m_DL_Trainer->getClassId(ID.b_str()));

							for(i=0; i<m_nFeatures; i++)
							{
								pEvent->push_back(m_DL_Trainer->getId(Values[i].b_str()));
							}

							m_DL_Events->push_back(pEvent);
						}
						break;
					}
				}
			}
		}
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CClassify_Grid::Get_File(const CSG_String &File)
{
	//-----------------------------------------------------
	if( !m_YT_Model.load_from_file(File.b_str()) )
	{
		Error_Set(_TL("could not load model from file"));

		return( false );
	}

	//-----------------------------------------------------
/*	list< pair< pair<string, string>, double > > Features;

	m_YT_Model.get_features(Features);

	if( (int)Features.size() != m_nFeatures )
	{
		Error_Set(_TL("number of model features do not fit input features list"));

		return( false );
	}/**/

	//-----------------------------------------------------
	if( m_YT_Model.num_classes() <= 1 )
	{
		Error_Set(_TL("less than two classes in model"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Parameter	*pLUT	= DataObject_Get_Parameter(Parameters("CLASSES")->asGrid(), "LUT");

	for(int iClass=0; iClass<m_YT_Model.num_classes(); iClass++)
	{
		if( m_pProbs )
		{
			CSG_Grid	*pGrid	= m_pProbs->asGrid(iClass);

			if( !pGrid )
			{
				m_pProbs->Add_Item(pGrid = SG_Create_Grid(*Get_System()));

				DataObject_Set_Colors(pGrid, 11, SG_COLORS_YELLOW_GREEN);
			}

			pGrid->Set_Name(m_YT_Model.get_class_label(iClass).c_str());
		}

		if( pLUT && pLUT->asTable() )
		{
			CSG_Table_Record	*pClass	= pLUT->asTable()->Get_Record(iClass);

			if( !pClass )
			{
				(pClass	= pLUT->asTable()->Add_Record())->Set_Value(0, SG_Color_Get_Random());
			}

			pClass->Set_Value(1, m_YT_Model.get_class_label(iClass).c_str());
			pClass->Set_Value(3, m_YT_Model.get_class_id(m_YT_Model.get_class_label(iClass)));
			pClass->Set_Value(4, m_YT_Model.get_class_id(m_YT_Model.get_class_label(iClass)));
		}
	}

	if( pLUT && pLUT->asTable() )
	{
		pLUT->asTable()->Set_Record_Count(m_YT_Model.num_classes());

		DataObject_Set_Parameter(Parameters("CLASSES")->asGrid(), pLUT);
		DataObject_Set_Parameter(Parameters("CLASSES")->asGrid(), "COLORS_TYPE", 1);	// Color Classification Type: Lookup Table
	}

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
