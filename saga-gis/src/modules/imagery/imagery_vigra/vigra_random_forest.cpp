/**********************************************************
 * Version $Id: vigra_random_forest.cpp 1282 2011-12-29 17:13:26Z manfred-e $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                        VIGRA                          //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                vigra_random_forest.cpp                //
//                                                       //
//                 Copyright (C) 2013 by                 //
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
#include "vigra_random_forest.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#if !defined(VIGRA_VERSION_MAJOR) || VIGRA_VERSION_MAJOR < 1 || VIGRA_VERSION_MINOR < 10
// define some dummies
#include <vigra\matrix.hxx>
class CRandom_Forest
{
public:
	CRandom_Forest(CSG_Parameters *pParameters)	{}
	static bool				Parameters_Create	(CSG_Parameters &Parameters)		{	return( true );	}
	bool					Load_Model			(bool bLoadNow)						{	return( true );	}
	bool					Train_Model			(const CSG_Matrix &Data)			{	return( true );	}
	int						Get_Feature_Count	(void)								{	return( 0 );	}
	int						Get_Prediction		(vigra::Matrix<double> features)	{	return( 0 );	}
	vigra::Matrix<double>	Get_Probabilities	(vigra::Matrix<double> features)	{	vigra::Matrix<double> p(0, 0); return( p );	}
};
#else


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include <vigra/random_forest.hxx>

#define WITH_HDF5
#if defined(WITH_HDF5)
#include <vigra/random_forest_hdf5_impex.hxx>
#endif


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
class CRandom_Forest
{
public:
	CRandom_Forest(CSG_Parameters *pParameters);

	static bool					Parameters_Create		(CSG_Parameters &Parameters);

	bool						Load_Model				(bool bLoadNow);
	bool						Train_Model				(const CSG_Matrix &Data);

	int							Get_Feature_Count		(void)	{	return( m_Forest.feature_count() );	}

	int							Get_Prediction			(vigra::Matrix<double> features);
	vigra::Matrix<double>		Get_Probabilities		(vigra::Matrix<double> features);


private:

	CSG_Parameters				*m_pParameters;

	vigra::RandomForest<int>	m_Forest;
	
};


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CRandom_Forest::CRandom_Forest(CSG_Parameters *pParameters)
{
	m_pParameters	= pParameters;
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CRandom_Forest::Parameters_Create(CSG_Parameters &Parameters)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
#if defined(WITH_HDF5)
	Parameters.Add_FilePath(
		NULL	, "RF_IMPORT"			, _TL("Import from File"),
		_TL(""),
		NULL, NULL, false
	);
#endif

	//-----------------------------------------------------
	pNode	= Parameters.Add_Node(
		NULL	, "RF_OPTIONS"			, _TL("Random Forest Options"),
		_TL("")
	);

#if defined(WITH_HDF5)
	Parameters.Add_FilePath(
		pNode	, "RF_EXPORT"			, _TL("Export to File"),
		_TL(""),
		NULL, NULL, true
	);
#endif

	Parameters.Add_Value(
		pNode	, "RF_TREE_COUNT"		, _TL("Tree Count"),
		_TL("How many trees to create?"),
		PARAMETER_TYPE_Int, 32, 1, true
	);

	Parameters.Add_Value(
		pNode	, "RF_TREE_SAMPLES"		, _TL("Samples per Tree"),
		_TL("Specifies the fraction of the total number of samples used per tree for learning."),
		PARAMETER_TYPE_Double, 1.0, 0.0, true, 1.0, true
	);

	Parameters.Add_Value(
		pNode	, "RF_REPLACE"			, _TL("Sample with Replacement"),
		_TL("Sample from training population with or without replacement?"),
		PARAMETER_TYPE_Bool, true
	);

	Parameters.Add_Value(
		pNode	, "RF_SPLIT_MIN_SIZE"	, _TL("Minimum Node Split Size"),
		_TL("Number of examples required for a node to be split. Choose 1 for complete growing."),
		PARAMETER_TYPE_Int, 1, 1, true
	);

	Parameters.Add_Choice(
		pNode	, "RF_NODE_FEATURES"	, _TL("Features per Node"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|"),
			_TL("logarithmic"),
			_TL("square root"),
			_TL("all")
		), 1
	);

	Parameters.Add_Choice(
		pNode	, "RF_STRATIFICATION"	, _TL("Stratification"),
		_TL("Specifies stratification strategy. Either none, equal amount of class samples, or proportional to fraction of class samples."),
		CSG_String::Format(SG_T("%s|%s|%s|"),
			_TL("none"),
			_TL("equal"),
			_TL("proportional")
		), 0
	);

	//-----------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CRandom_Forest::Load_Model(bool bLoadNow)
{
#if defined(WITH_HDF5)
	if( SG_File_Exists((*m_pParameters)("RF_IMPORT")->asString()) )
	{
		if( bLoadNow == false )
		{
			return( true );
		}

		if( vigra::rf_import_HDF5(m_Forest, CSG_String((*m_pParameters)("RF_IMPORT")->asString()).b_str()) )
		{
			return( true );
		}
	}
#endif

	return( false );
}

//---------------------------------------------------------
bool CRandom_Forest::Train_Model(const CSG_Matrix &Data)
{
	//-----------------------------------------------------
	int	nFeatures	= Data.Get_NCols() - 1;

	vigra::Matrix<double>	train_features(Data.Get_NRows(), nFeatures);
	vigra::Matrix<int>		train_response(Data.Get_NRows(), 1);

	for(int iSample=0; iSample<Data.Get_NRows(); iSample++)
	{
		train_response(iSample, 0)	= (int)Data[iSample][nFeatures];

		for(int iFeature=0; iFeature<nFeatures; iFeature++)
		{
			train_features(iSample, iFeature)	= Data[iSample][iFeature];
		}
	}

	//-----------------------------------------------------
	m_Forest.set_options().tree_count             ((*m_pParameters)("RF_TREE_COUNT"    )->asInt   ());
	m_Forest.set_options().samples_per_tree       ((*m_pParameters)("RF_TREE_SAMPLES"  )->asDouble());
	m_Forest.set_options().sample_with_replacement((*m_pParameters)("RF_REPLACE"       )->asBool  ());
	m_Forest.set_options().min_split_node_size    ((*m_pParameters)("RF_SPLIT_MIN_SIZE")->asInt   ());

	switch( (*m_pParameters)("RF_NODE_FEATURES")->asInt() )
	{
	case  0:	m_Forest.set_options().features_per_node(vigra::RF_LOG );	break;
	default:	m_Forest.set_options().features_per_node(vigra::RF_SQRT);	break;
	case  1:	m_Forest.set_options().features_per_node(vigra::RF_ALL );	break;
	}

	switch( (*m_pParameters)("RF_STRATIFICATION")->asInt() )
	{
	default:	m_Forest.set_options().use_stratification(vigra::RF_NONE        );	break;
	case  1:	m_Forest.set_options().use_stratification(vigra::RF_EQUAL       );	break;
	case  2:	m_Forest.set_options().use_stratification(vigra::RF_PROPORTIONAL);	break;
	}

	//-----------------------------------------------------
	SG_UI_Process_Set_Text(_TL("learning"));

	vigra::rf::visitors::OOB_Error	oob_v;	// construct visitor to calculate out-of-bag error

	m_Forest.learn(train_features, train_response, vigra::rf::visitors::create_visitor(oob_v));

	SG_UI_Msg_Add_Execution(CSG_String::Format(SG_T("\n%s: %f"), _TL("out-of-bag error"), oob_v.oob_breiman), false);

#if defined(WITH_HDF5)
	if( (*m_pParameters)("RF_EXPORT")->asString() && *(*m_pParameters)("RF_EXPORT")->asString() )
	{
		vigra::rf_export_HDF5(m_Forest, CSG_String((*m_pParameters)("RF_EXPORT")->asString()).b_str());
	}
#endif

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CRandom_Forest::Get_Prediction(vigra::Matrix<double> features)
{
	return( m_Forest.predictLabel(features) );
}

//---------------------------------------------------------
vigra::Matrix<double> CRandom_Forest::Get_Probabilities(vigra::Matrix<double> features)
{
	vigra::Matrix<double>	p(1, m_Forest.class_count());

	m_Forest.predictProbabilities(features, p);

	return( p );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // !defined(VIGRA_VERSION_MAJOR) || VIGRA_VERSION_MAJOR < 1 || VIGRA_VERSION_MINOR < 10


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
enum
{
	CLASS_ID	= 0,
	CLASS_NAME,
	CLASS_COUNT
};


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CViGrA_Random_Forest::CViGrA_Random_Forest(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("Random Forest Classification (ViGrA)"));

	Set_Author		("O.Conrad (c) 2013");

	Set_Description	(_TW(
		"References:\n"
		"ViGrA - Vision with Generic Algorithms\n"
		"<a target=\"_blank\" href=\"http://hci.iwr.uni-heidelberg.de/vigra\">http://hci.iwr.uni-heidelberg.de</a>"
	));

	//-----------------------------------------------------
	Parameters.Add_Grid_List(
		NULL	, "FEATURES"			, _TL("Features"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Grid(
		NULL	, "CLASSES"				, _TL("Random Forest Classification"),
		_TL(""),
		PARAMETER_OUTPUT, true, SG_DATATYPE_Short
	);

	Parameters.Add_Grid(
		NULL	, "PROBABILITY"			, _TL("Prediction Probability"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Value(
		NULL	, "BPROBABILITIES"		, _TL("Feature Probabilities"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);

	Parameters.Add_Grid_List(
		NULL	, "PROBABILITIES"		, _TL("Feature Probabilities"),
		_TL(""),
		PARAMETER_OUTPUT
	);

	//-----------------------------------------------------
	pNode	= Parameters.Add_Shapes(
		NULL	, "TRAINING"			, _TL("Training Areas"),
		_TL(""),
		PARAMETER_INPUT, SHAPE_TYPE_Polygon
	);

	Parameters.Add_Table_Field(
		pNode	, "FIELD"				, _TL("Label Field"),
		_TL("")
	);

	Parameters.Add_Value(
		pNode	, "LABEL_AS_ID"			, _TL("Use Label as Identifier"),
		_TL("Use training area labels as identifier in classification result, assumes all label values are integer numbers!"),
		PARAMETER_TYPE_Bool, false
	);

	CSG_mRMR::Parameters_Add(&Parameters, Parameters.Add_Value(
		pNode	, "DO_MRMR"				, _TL("Minimum Redundancy Feature Selection"),
		_TL("Use only features selected by the minimum Redundancy Maximum Relevance (mRMR) algorithm"),
		PARAMETER_TYPE_Bool, false
	));

	//-----------------------------------------------------
	CRandom_Forest::Parameters_Create(Parameters);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CViGrA_Random_Forest::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "RF_IMPORT") )
	{
		bool	bTraining	= !SG_File_Exists(pParameter->asString());

		pParameters->Set_Enabled("RF_OPTIONS", bTraining);
		pParameters->Set_Enabled("TRAINING"  , bTraining);
	}

	if( !SG_STR_CMP(pParameter->Get_Identifier(), "DO_MRMR") )
	{
		pParameters->Get("DO_MRMR")->Set_Children_Enabled(pParameter->asBool());
	}

	if( pParameters->Get("DO_MRMR")->asBool() )
	{
		CSG_mRMR::Parameters_Enable(pParameters, pParameter);
	}

	return( 1 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CViGrA_Random_Forest::On_Execute(void)
{
	//-----------------------------------------------------
	// Feature Grids

	CSG_Parameter_Grid_List		*pFeatures	= Parameters("FEATURES")->asGridList();

	CSG_Array	Features(sizeof(CSG_Grid *), pFeatures->Get_Count());

	m_pFeatures	= (CSG_Grid **)Features.Get_Array();
	m_nFeatures	= 0;

	for(int i=pFeatures->Get_Count()-1; i>=0; i--)
	{
		if( pFeatures->asGrid(i)->Get_ZRange() <= 0.0 )
		{
			Message_Add(CSG_String::Format(SG_T("%s: %s"), _TL("grid has been dropped"), pFeatures->asGrid(i)->Get_Name()));
		}
		else
		{
			m_pFeatures[m_nFeatures++]	= pFeatures->asGrid(i);
		}
	}

	if( m_nFeatures <= 0 )
	{
		Error_Set(_TL("no valid grid in features list."));

		return( false );
	}


	//-----------------------------------------------------
	// Model Creation

	CRandom_Forest	Model(&Parameters);

	CSG_Table		Classes;

	if( Model.Load_Model(false) )	// load model from file ...
	{
		if( !Model.Load_Model(true) )
		{
			Error_Set(_TL("could not import random forest"));

			return( false );
		}

		if( Model.Get_Feature_Count() != m_nFeatures )
		{
			Error_Set(CSG_String::Format(SG_T("%s\n%s: %d"), _TL("invalid number of features"), _TL("expected"), Model.Get_Feature_Count()));

			return( false );
		}
	}
	else							// train model from training data ...
	{
		CSG_Matrix	Data;

		if( !Get_Training(Data, Classes) )
		{
			Error_Set(_TL("insufficient training samples"));

			return( false );
		}

		Model.Train_Model(Data);
	}


	//-----------------------------------------------------
	// Output Grids

	CSG_Grid	*pClasses		= Get_Class_Grid();

	CSG_Grid	*pProbability	= Parameters("PROBABILITY")->asGrid();

	CSG_Parameter_Grid_List	*pProbabilities	= Get_Propability_Grids(Classes);

	if( pProbability && !pProbability->Get_ZRange() ) DataObject_Set_Colors(pProbability, 11, SG_COLORS_WHITE_GREEN);


	//-----------------------------------------------------
	// Prediction

	Process_Set_Text(_TL("prediction"));

	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			if( !pClasses->is_NoData(x, y) )
			{
				vigra::Matrix<double>	features(1, m_nFeatures);

				for(int iFeature=0; iFeature<m_nFeatures; iFeature++)
				{
					features(0, iFeature)	= m_pFeatures[iFeature]->asDouble(x, y);
				}

				int	id	= Model.Get_Prediction(features);

				pClasses->Set_Value(x, y, id);

				if( pProbability || pProbabilities )
				{
					vigra::Matrix<double>	p	= Model.Get_Probabilities(features);

					SG_GRID_PTR_SAFE_SET_VALUE(pProbability, x, y, p(0, id - 1));

					for(int i=0; pProbabilities && i<pProbabilities->Get_Count(); i++)
					{
						pProbabilities->asGrid(i)->Set_Value(x, y, p(0, i));
					}
				}
			}
			else // if( pClasses->is_NoData(x, y) )
			{
				SG_GRID_PTR_SAFE_SET_NODATA(pProbability, x, y);

				for(int i=0; pProbabilities && i<pProbabilities->Get_Count(); i++)
				{
					pProbabilities->asGrid(i)->Set_NoData(x, y);
				}
			}
		}
	}


	//-----------------------------------------------------
	// Postprocessing

	Set_Classification(Classes);

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CViGrA_Random_Forest::Get_Training(CSG_Matrix &Data, CSG_Table &Classes)
{
	CSG_Shapes	*pTraining	= Parameters("TRAINING")->asShapes();

	bool	bLabelAsId	= Parameters("LABEL_AS_ID")->asBool();

	int		Field	= Parameters("FIELD")->asInt();

	Classes.Destroy();
	Classes.Add_Field(SG_T("ID"   ), SG_DATATYPE_Int);		// CLASS_ID
	Classes.Add_Field(SG_T("NAME" ), SG_DATATYPE_String);	// CLASS_NAME
	Classes.Add_Field(SG_T("COUNT"), SG_DATATYPE_Int);		// CLASS_COUNT

	CSG_String	Label;

	CSG_Table_Record	*pClass	= NULL;

	pTraining->Set_Index(Field, TABLE_INDEX_Ascending);

	for(int iTraining=0, ID=0; iTraining<pTraining->Get_Count(); iTraining++)
	{
		CSG_Shape	*pArea	= pTraining->Get_Shape(iTraining);

		if( !pClass || (bLabelAsId && ID != pArea->asInt(Field)) || Label.Cmp(pArea->asString(Field)) )
		{
			Label	= pArea->asString(Field);

			if( !pClass || pClass->asInt(CLASS_COUNT) > 0 )
			{
				pClass	= Classes.Add_Record();

				ID	= bLabelAsId ? pArea->asInt(Field) : ID + 1;
			}

			pClass->Set_Value(CLASS_ID   , ID);
			pClass->Set_Value(CLASS_NAME , Label);
			pClass->Set_Value(CLASS_COUNT, 0);
		}

		pClass->Add_Value(CLASS_COUNT, Get_Training(Data, ID, (CSG_Shape_Polygon *)pArea));
	}

	if( Data.Get_NCols() > 1 && Data.Get_NRows() > 1 && Parameters("DO_MRMR")->asBool() )
	{
		CSG_mRMR	Selector;

		if( Selector.Set_Data(Data, m_nFeatures, &Parameters) && Selector.Get_Selection(&Parameters)
		&&  Selector.Get_Count() > 0 && Selector.Get_Count() < m_nFeatures )
		{
			int	i, j, *bSelected	= (int *)SG_Calloc(m_nFeatures, sizeof(int));

			for(i=0; i<Selector.Get_Count(); i++)
			{
				bSelected[j = Selector.Get_Index(i) - 1]	= 1;

				Message_Add(CSG_String::Format(SG_T("\n%02d. %s (%s: %f)"),
					i + 1, m_pFeatures[j]->Get_Name(), _TL("Score"), Selector.Get_Score(i)
				), false);
			}

			Message_Add("\n", false);

			for(i=0, j=0; i<m_nFeatures; i++)
			{
				if( bSelected[m_nFeatures - i - 1] == 0 )
				{
					Data.Del_Col(m_nFeatures - i - 1);
				}

				if( bSelected[i] == 1 )
				{
					m_pFeatures[j++]	= m_pFeatures[i];
				}
			}

			m_nFeatures	= Selector.Get_Count();

			delete[](bSelected);
		}
	}

	return( Data.Get_NCols() > 1 && Data.Get_NRows() > 1 );
}

//---------------------------------------------------------
int CViGrA_Random_Forest::Get_Training(CSG_Matrix &Data, int ID, CSG_Shape_Polygon *pArea)
{
	int	Count	= 0;

	int	xMin	= Get_System()->Get_xWorld_to_Grid(pArea->Get_Extent().Get_XMin());	if( xMin <  0        ) xMin = 0;
	int	xMax	= Get_System()->Get_xWorld_to_Grid(pArea->Get_Extent().Get_XMax());	if( xMax >= Get_NX() ) xMax = Get_NX() - 1;
	int	yMin	= Get_System()->Get_yWorld_to_Grid(pArea->Get_Extent().Get_YMin());	if( yMin <  0        ) yMin = 0;
	int	yMax	= Get_System()->Get_yWorld_to_Grid(pArea->Get_Extent().Get_YMax());	if( yMax >= Get_NY() ) yMax = Get_NY() - 1;

	for(int y=yMin; y<=yMax; y++)
	{
		for(int x=xMin; x<=xMax; x++)
		{
			if( pArea->Contains(Get_System()->Get_Grid_to_World(x, y)) )
			{
				CSG_Vector	z(1 + m_nFeatures);

				z[m_nFeatures]	= ID;

				bool	bOkay	= true;

				for(int i=0; bOkay && i<m_nFeatures; i++)
				{
					if( (bOkay = !m_pFeatures[i]->is_NoData(x, y)) == true )
					{
						z[i]	= m_pFeatures[i]->asDouble(x, y);
					}
				}

				if( bOkay )
				{
					Data.Add_Row(z);

					Count++;
				}
			}
		}
	}

	return( Count );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Grid * CViGrA_Random_Forest::Get_Class_Grid(void)
{
	CSG_Grid	*pClasses	= Parameters("CLASSES")->asGrid();

	pClasses->Set_NoData_Value(-1.0);

	#pragma omp parallel for
	for(sLong i=0; i<Get_NCells(); i++)
	{
		bool	bOkay	= true;

		for(int iFeature=0; bOkay && iFeature<m_nFeatures; iFeature++)
		{
			bOkay	= !m_pFeatures[iFeature]->is_NoData(i);
		}

		pClasses->Set_Value(i, bOkay ? 0.0 : -1.0);
	}

	return( pClasses );
}

//---------------------------------------------------------
void CViGrA_Random_Forest::Set_Classification(CSG_Table &Classes)
{
	if( Classes.Get_Field_Count() == 3 && Classes.Get_Count() > 0 )
	{
		CSG_Grid	*pClasses	= Parameters("CLASSES")->asGrid();

		CSG_Parameters	P;

		if( DataObject_Get_Parameters(pClasses, P) && P("COLORS_TYPE") && P("LUT") )
		{
			CSG_Table	*pTable	= P("LUT")->asTable();

			for(int i=0; i<Classes.Get_Count(); i++)
			{
				CSG_Table_Record	*pRecord	= pTable->Get_Record(i);

				if( pRecord == NULL )
				{
					pRecord	= pTable->Add_Record();
					pRecord->Set_Value(0, CSG_Random::Get_Uniform(0, 255*255*255));
				}

				pRecord->Set_Value(1, Classes[i].asString(1));
				pRecord->Set_Value(2, Classes[i].asString(1));
				pRecord->Set_Value(3, Classes[i].asInt(0));
				pRecord->Set_Value(4, Classes[i].asInt(0));
			}

			while( pTable->Get_Record_Count() > Classes.Get_Count() )
			{
				pTable->Del_Record(pTable->Get_Record_Count() - 1);
			}

			P("COLORS_TYPE")->Set_Value(1);	// Color Classification Type: Lookup Table

			DataObject_Set_Parameters(pClasses, P);
		}
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_Parameter_Grid_List * CViGrA_Random_Forest::Get_Propability_Grids(CSG_Table &Classes)
{
	if( Parameters("BPROBABILITIES")->asBool() && Classes.Get_Field_Count() == 3 && Classes.Get_Count() > 0 )
	{
		CSG_Parameter_Grid_List	*pGrids	= Parameters("PROBABILITIES")->asGridList();

		for(int i=0; i<Classes.Get_Count(); i++)
		{
			CSG_Grid	*pGrid;
			
			if( i < pGrids->Get_Count() )
			{
				pGrid	= pGrids->asGrid(i);
			}
			else
			{
				pGrids->Add_Item(pGrid = SG_Get_Data_Manager().Add_Grid(*Get_System()));

				DataObject_Set_Colors(pGrid, 11, SG_COLORS_WHITE_GREEN);
			}

			pGrid->Set_Name(CSG_String::Format(SG_T("%s [%s]"), Classes[i].asString(CLASS_NAME), _TL("Probability")));
		}

		return( pGrids );
	}

	return( NULL );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CViGrA_RF_Presence::CViGrA_RF_Presence(void)
{
	//-----------------------------------------------------
	Set_Name		(_TL("Random Forest Presence Prediction (ViGrA)"));

	Set_Author		("O.Conrad (c) 2015");

	Set_Description	(_TW(
		"References:\n"
		"ViGrA - Vision with Generic Algorithms\n"
		"<a target=\"_blank\" href=\"http://hci.iwr.uni-heidelberg.de/vigra\">http://hci.iwr.uni-heidelberg.de</a>"
	));

	//-----------------------------------------------------
	Parameters.Add_Grid_List(
		NULL	, "FEATURES"			, _TL("Features"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Grid(
		NULL	, "PREDICTION"			, _TL("Presence Prediction"),
		_TL(""),
		PARAMETER_OUTPUT, true, SG_DATATYPE_Char
	);

	Parameters.Add_Grid(
		NULL	, "PROBABILITY"			, _TL("Presence Probability"),
		_TL(""),
		PARAMETER_OUTPUT_OPTIONAL
	);

	Parameters.Add_Shapes(
		NULL	, "PRESENCE"			, _TL("Presence Data"),
		_TL(""),
		PARAMETER_INPUT, SHAPE_TYPE_Point
	);

	Parameters.Add_Value(
		NULL	, "BACKGROUND"			, _TL("Background Sample Density [Percent]"),
		_TL(""),
		PARAMETER_TYPE_Double, 1, 0.0, true, 100, true
	);

	//-----------------------------------------------------
	CSG_mRMR::Parameters_Add(&Parameters, Parameters.Add_Value(
		NULL	, "DO_MRMR"				, _TL("Minimum Redundancy Feature Selection"),
		_TL("Use only features selected by the minimum Redundancy Maximum Relevance (mRMR) algorithm"),
		PARAMETER_TYPE_Bool, false
	));

	//-----------------------------------------------------
	CRandom_Forest::Parameters_Create(Parameters);
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CViGrA_RF_Presence::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if( !SG_STR_CMP(pParameter->Get_Identifier(), "RF_IMPORT") )
	{
		bool	bTraining	= !SG_File_Exists(pParameter->asString());

		pParameters->Set_Enabled("RF_OPTIONS", bTraining);
		pParameters->Set_Enabled("PRESENCE"  , bTraining);
	}

	if( !SG_STR_CMP(pParameter->Get_Identifier(), "DO_MRMR") )
	{
		pParameters->Get("DO_MRMR")->Set_Children_Enabled(pParameter->asBool());
	}

	if( pParameters->Get("DO_MRMR")->asBool() )
	{
		CSG_mRMR::Parameters_Enable(pParameters, pParameter);
	}

	return( 1 );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CViGrA_RF_Presence::On_Execute(void)
{
	//-----------------------------------------------------
	CSG_Parameter_Grid_List		*pFeatures	= Parameters("FEATURES")->asGridList();

	CSG_Array	Features(sizeof(CSG_Grid *), pFeatures->Get_Count());

	m_pFeatures	= (CSG_Grid **)Features.Get_Array();
	m_nFeatures	= 0;

	for(int i=pFeatures->Get_Count()-1; i>=0; i--)
	{
		if( pFeatures->asGrid(i)->Get_ZRange() <= 0.0 )
		{
			Message_Add(CSG_String::Format(SG_T("%s: %s"), _TL("grid has been dropped"), pFeatures->asGrid(i)->Get_Name()));
		}
		else
		{
			m_pFeatures[m_nFeatures++]	= pFeatures->asGrid(i);
		}
	}

	if( m_nFeatures <= 0 )
	{
		Error_Set(_TL("no valid grid in features list."));

		return( false );
	}

	//-----------------------------------------------------
	CRandom_Forest	Model(&Parameters);

	if( Model.Load_Model(false) )	// load model from file ...
	{
		if( !Model.Load_Model(true) )
		{
			Error_Set(_TL("could not import random forest"));

			return( false );
		}

		if( Model.Get_Feature_Count() != m_nFeatures )
		{
			Error_Set(CSG_String::Format(SG_T("%s\n%s: %d"), _TL("invalid number of features"), _TL("expected"), Model.Get_Feature_Count()));

			return( false );
		}
	}
	else							// train model from training data ...
	{
		CSG_Matrix	Data;

		if( !Get_Training(Data) )
		{
			Error_Set(_TL("insufficient training samples"));

			return( false );
		}

		Process_Set_Text(_TL("training"));

		Model.Train_Model(Data);
	}

	//-----------------------------------------------------
	CSG_Grid	*pPrediction	= Parameters("PREDICTION" )->asGrid();
	CSG_Grid	*pProbability	= Parameters("PROBABILITY")->asGrid();

	if( !pPrediction ->Get_ZRange() ) DataObject_Set_Colors(pPrediction , 11, SG_COLORS_YELLOW_GREEN);
	if( !pProbability->Get_ZRange() ) DataObject_Set_Colors(pProbability, 11, SG_COLORS_YELLOW_GREEN);

	//-----------------------------------------------------
	Process_Set_Text(_TL("prediction"));

	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			bool	bOkay	= true;

			vigra::Matrix<double>	features(1, m_nFeatures);

			for(int iFeature=0; bOkay && iFeature<m_nFeatures; iFeature++)
			{
				if( (bOkay = !m_pFeatures[iFeature]->is_NoData(x, y)) == true )
				{
					features(0, iFeature)	= m_pFeatures[iFeature]->asDouble(x, y);
				}
			}

			if( bOkay )
			{
			//	vigra::Matrix<double>	p	= Model.Get_Probabilities(features);
			//	pProbability->Set_Value(x, y, p(0, 0));
				pPrediction ->Set_Value(x, y, Model.Get_Prediction   (features));
				pProbability->Set_Value(x, y, Model.Get_Probabilities(features)(0, 0));
			}
			else // if( pClasses->is_NoData(x, y) )
			{
				pPrediction ->Set_NoData(x, y);
				pProbability->Set_NoData(x, y);
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
bool CViGrA_RF_Presence::Get_Training(CSG_Matrix &Data)
{
	//-----------------------------------------------------
	Process_Set_Text(_TL("collecting presence data"));

	CSG_Shapes	*pPresence	= Parameters("PRESENCE")->asShapes();

	for(int iPoint=0; iPoint<pPresence->Get_Count() && Set_Progress(iPoint, pPresence->Get_Count()); iPoint++)
	{
		TSG_Point	p	= pPresence->Get_Shape(iPoint)->Get_Point(0);

		int	x	= Get_System()->Get_xWorld_to_Grid(p.x);
		int	y	= Get_System()->Get_yWorld_to_Grid(p.y);

		Get_Training(Data, x, y, 1);
	}

	//-----------------------------------------------------
	Process_Set_Text(_TL("collecting background data"));

	double	Background	= Parameters("BACKGROUND")->asDouble() / 100.0;

	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		for(int x=0; x<Get_NX(); x++)
		{
			if( CSG_Random::Get_Uniform() <= Background )
			{
				Get_Training(Data, x, y, 0);
			}
		}
	}

	//-----------------------------------------------------
	if( Data.Get_NCols() > 1 && Data.Get_NRows() > 1 && Parameters("DO_MRMR")->asBool() )
	{
		CSG_mRMR	Selector;

		if( Selector.Set_Data(Data, m_nFeatures, &Parameters) && Selector.Get_Selection(&Parameters)
		&&  Selector.Get_Count() > 0 && Selector.Get_Count() < m_nFeatures )
		{
			int	i, j, *bSelected	= (int *)SG_Calloc(m_nFeatures, sizeof(int));

			for(i=0; i<Selector.Get_Count(); i++)
			{
				bSelected[j = Selector.Get_Index(i) - 1]	= 1;

				Message_Add(CSG_String::Format(SG_T("\n%02d. %s (%s: %f)"),
					i + 1, m_pFeatures[j]->Get_Name(), _TL("Score"), Selector.Get_Score(i)
				), false);
			}

			Message_Add("\n", false);

			for(i=0, j=0; i<m_nFeatures; i++)
			{
				if( bSelected[m_nFeatures - i - 1] == 0 )
				{
					Data.Del_Col(m_nFeatures - i - 1);
				}

				if( bSelected[i] == 1 )
				{
					m_pFeatures[j++]	= m_pFeatures[i];
				}
			}

			m_nFeatures	= Selector.Get_Count();

			delete[](bSelected);
		}
	}

	return( Data.Get_NCols() > 1 && Data.Get_NRows() > 1 );
}

//---------------------------------------------------------
bool CViGrA_RF_Presence::Get_Training(CSG_Matrix &Data, int x, int y, int ID)
{
	CSG_Vector	z(1 + m_nFeatures);

	z[m_nFeatures]	= ID;

	for(int i=0; i<m_nFeatures; i++)
	{
		if( !m_pFeatures[i]->is_InGrid(x, y) )
		{
			return( false );
		}

		z[i]	= m_pFeatures[i]->asDouble(x, y);
	}

	Data.Add_Row(z);

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
