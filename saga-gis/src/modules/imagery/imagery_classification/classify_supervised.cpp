/**********************************************************
 * Version $Id: classify_supervised.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                imagery_classification                 //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//              Grid_Classify_Supervised.cpp             //
//                                                       //
//                 Copyright (C) 2005 by                 //
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
#include "classify_supervised.h"


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CGrid_Classify_Supervised::CGrid_Classify_Supervised(void)
{
	int				i;
	CSG_String		s;
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("Supervised Classification for Grids"));

	Set_Author		("O.Conrad (c) 2005");

	Set_Description	(_TW(
		"Supervised Classification"
	));

	//-----------------------------------------------------
	pNode	= Parameters.Add_Grid_List(
		NULL	, "GRIDS"			, _TL("Features"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Value(
		pNode	, "NORMALISE"		, _TL("Normalise"),
		_TL(""),
		PARAMETER_TYPE_Bool, false
	);

	Parameters.Add_Grid(
		NULL	, "CLASSES"			, _TL("Classification"),
		_TL(""),
		PARAMETER_OUTPUT, true, SG_DATATYPE_Short
	);

	Parameters.Add_Grid(
		NULL	, "QUALITY"			, _TL("Quality"),
		_TL("Dependent on chosen method, these are distances or probabilities."),
		PARAMETER_OUTPUT_OPTIONAL
	);

	//-----------------------------------------------------
	pNode	= Parameters.Add_Shapes(
		NULL	, "TRAINING"		, _TL("Training Areas"),
		_TL(""),
		PARAMETER_INPUT_OPTIONAL, SHAPE_TYPE_Polygon
	);

	Parameters.Add_Table_Field(
		pNode	, "TRAINING_CLASS"	, _TL("Class Identifier"),
		_TL("")
	);

	Parameters.Add_FilePath(
		pNode	, "FILE_LOAD"		, _TL("Load Statistics from File..."),
		_TL(""),
		NULL, NULL, false
	);

	Parameters.Add_FilePath(
		NULL	, "FILE_SAVE"		, _TL("Save Statistics to File..."),
		_TL(""),
		NULL, NULL, true
	);

	//-----------------------------------------------------
	for(i=0; i<=SG_CLASSIFY_SUPERVISED_WTA; i++)
	{
		s	+= CSG_Classifier_Supervised::Get_Name_of_Method(i) + "|";
	}

	Parameters.Add_Choice(
		NULL	, "METHOD"			, _TL("Method"),
		_TL(""),
		s, SG_CLASSIFY_SUPERVISED_MinimumDistance
	);

	Parameters.Add_Value(
		NULL	, "THRESHOLD_DIST"	, _TL("Distance Threshold"),
		_TL("Let pixel stay unclassified, if minimum euclidian or mahalanobis distance is greater than threshold."),
		PARAMETER_TYPE_Double, 0.0, 0.0, true
	);

	Parameters.Add_Value(
		NULL	, "THRESHOLD_ANGLE"	, _TL("Spectral Angle Threshold (Degree)"),
		_TL("Let pixel stay unclassified, if spectral angle distance is greater than threshold."),
		PARAMETER_TYPE_Double, 0.0, 0.0, true, 90.0, true
	);

	Parameters.Add_Value(
		NULL	, "THRESHOLD_PROB"	, _TL("Probability Threshold"),
		_TL("Let pixel stay unclassified, if maximum likelihood probability value is less than threshold."),
		PARAMETER_TYPE_Double, 0.0, 0.0, true, 100.0, true
	);

	Parameters.Add_Choice(
		NULL	, "RELATIVE_PROB"	, _TL("Probability Reference"),
		_TL(""),
		CSG_String::Format("%s|%s|",
			_TL("absolute"),
			_TL("relative")
		), 1
	);

	pNode	= Parameters.Add_Node(
		NULL	, "WTA"				, _TL("Winner Takes All"),
		_TL("")
	);

	for(i=0; i<SG_CLASSIFY_SUPERVISED_WTA; i++)
	{
		Parameters.Add_Value(
			pNode, CSG_String::Format("WTA_%d", i), CSG_Classifier_Supervised::Get_Name_of_Method(i), _TL(""),
			PARAMETER_TYPE_Bool, false
		);
	}
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
int CGrid_Classify_Supervised::On_Parameters_Enable(CSG_Parameters *pParameters, CSG_Parameter *pParameter)
{
	if(	!SG_STR_CMP(pParameter->Get_Identifier(), "TRAINING") )
	{
		pParameters->Set_Enabled("FILE_LOAD", pParameter->asShapes() == NULL);
		pParameters->Set_Enabled("FILE_SAVE", pParameter->asShapes() != NULL);
	}

	if(	!SG_STR_CMP(pParameter->Get_Identifier(), "METHOD") )
	{
		pParameters->Set_Enabled("THRESHOLD_DIST" , pParameter->asInt() == SG_CLASSIFY_SUPERVISED_MinimumDistance
			||                                      pParameter->asInt() == SG_CLASSIFY_SUPERVISED_Mahalonobis      );
		pParameters->Set_Enabled("THRESHOLD_PROB" , pParameter->asInt() == SG_CLASSIFY_SUPERVISED_MaximumLikelihood);
		pParameters->Set_Enabled("RELATIVE_PROB"  , pParameter->asInt() == SG_CLASSIFY_SUPERVISED_MaximumLikelihood);
		pParameters->Set_Enabled("THRESHOLD_ANGLE", pParameter->asInt() == SG_CLASSIFY_SUPERVISED_SAM              );
		pParameters->Set_Enabled("WTA"            , pParameter->asInt() == SG_CLASSIFY_SUPERVISED_WTA              );
	}

	return( CSG_Module_Grid::On_Parameters_Enable(pParameters, pParameter) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Classify_Supervised::On_Execute(void)
{
	//-----------------------------------------------------
	if( !Get_Features() )
	{
		Error_Set(_TL("invalid features"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Classifier_Supervised	Classifier;

	if( !Set_Classifier(Classifier) )
	{
		return( false );
	}

	//-----------------------------------------------------
	CSG_Grid	*pClasses	= Parameters("CLASSES")->asGrid();
	CSG_Grid	*pQuality	= Parameters("QUALITY")->asGrid();

	pClasses->Set_NoData_Value(0);
	pClasses->Assign(0.0);

	//-----------------------------------------------------
	Process_Set_Text(_TL("prediction"));

	int	Method	= Parameters("METHOD")->asInt();

	for(int y=0; y<Get_NY() && Set_Progress(y); y++)
	{
		#pragma omp parallel for
		for(int x=0; x<Get_NX(); x++)
		{
			int			Class;
			double		Quality;
			CSG_Vector	Features(m_pFeatures->Get_Count());

			if( Get_Features(x, y, Features) && Classifier.Get_Class(Features, Class, Quality, Method) )
			{
				SG_GRID_PTR_SAFE_SET_VALUE(pClasses, x, y, 1 + Class);
				SG_GRID_PTR_SAFE_SET_VALUE(pQuality, x, y, Quality  );
			}
			else
			{
				SG_GRID_PTR_SAFE_SET_NODATA(pClasses, x, y);
				SG_GRID_PTR_SAFE_SET_NODATA(pQuality, x, y);
			}
		}
	}

	//-----------------------------------------------------
	return( Set_Classification(Classifier) );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Classify_Supervised::Get_Features(void)
{
	m_pFeatures		= Parameters("GRIDS"    )->asGridList();
	m_bNormalise	= Parameters("NORMALISE")->asBool();

	for(int i=m_pFeatures->Get_Count()-1; i>=0; i--)
	{
		if( m_pFeatures->asGrid(i)->Get_ZRange() <= 0.0 )
		{
			Message_Add(CSG_String::Format("%s: %s", _TL("feature has been dropped"), m_pFeatures->asGrid(i)->Get_Name()));

			m_pFeatures->Del_Item(i);
		}
	}

	return( m_pFeatures->Get_Count() > 0 );
}

//---------------------------------------------------------
bool CGrid_Classify_Supervised::Get_Features(int x, int y, CSG_Vector &Features)
{
	for(int i=0; i<m_pFeatures->Get_Count(); i++)
	{
		CSG_Grid	*pGrid	= m_pFeatures->asGrid(i);

		if( pGrid->is_NoData(x, y) )
		{
			return( false );
		}

		Features[i]	= m_bNormalise ? (pGrid->asDouble(x, y) - pGrid->Get_Mean()) / pGrid->Get_StdDev() : pGrid->asDouble(x, y);
	}

	return( true );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Classify_Supervised::Set_Classifier(CSG_Classifier_Supervised &Classifier)
{
	Classifier.Create(m_pFeatures->Get_Count());

	Classifier.Set_Threshold_Distance   (Parameters("THRESHOLD_DIST" )->asDouble());
	Classifier.Set_Threshold_Angle      (Parameters("THRESHOLD_ANGLE")->asDouble() * M_DEG_TO_RAD);
	Classifier.Set_Threshold_Probability(Parameters("THRESHOLD_PROB" )->asDouble());
	Classifier.Set_Probability_Relative (Parameters("RELATIVE_PROB"  )->asBool  ());

	for(int i=0; i<SG_CLASSIFY_SUPERVISED_WTA; i++)
	{
		Classifier.Set_WTA(i, Parameters(CSG_String::Format("WTA_%d", i))->asBool());
	}

	//-----------------------------------------------------
	if( Parameters("TRAINING")->asShapes() != NULL )	// training areas
	{
		if( !Set_Classifier(Classifier, Parameters("TRAINING")->asShapes(), Parameters("TRAINING_CLASS")->asInt()) )
		{
			Error_Set(_TL("could not initialize classifier from training areas"));

			return( false );
		}
	}
	else	// from file
	{
		if( !Classifier.Load(Parameters("FILE_LOAD")->asString()) )
		{
			Error_Set(_TL("could not initialize classifier from file"));

			return( false );
		}
	}

	//-----------------------------------------------------
	Message_Add(Classifier.Print(), false);

	return( true );
}

//---------------------------------------------------------
bool CGrid_Classify_Supervised::Set_Classifier(CSG_Classifier_Supervised &Classifier, CSG_Shapes *pPolygons, int Field)
{
	Process_Set_Text(_TL("training"));

	//-----------------------------------------------------
	TSG_Point	p;	p.y	= Get_YMin();

	for(int y=0; y<Get_NY() && Set_Progress(y); y++, p.y+=Get_Cellsize())
	{
		p.x	= Get_XMin();

		for(int x=0; x<Get_NX(); x++, p.x+=Get_Cellsize())
		{
			CSG_Vector	Features(m_pFeatures->Get_Count());

			if( Get_Features(x, y, Features) )
			{
				for(int iPolygon=0; iPolygon<pPolygons->Get_Count(); iPolygon++)
				{
					CSG_Shape_Polygon	*pPolygon	= (CSG_Shape_Polygon *)pPolygons->Get_Shape(iPolygon);

					if( pPolygon->Contains(p) )
					{
						Classifier.Train_Add_Sample(pPolygon->asString(Field), Features);
					}
				}
			}
		}
	}

	//-----------------------------------------------------
	if( Classifier.Train(true) )
	{
		Classifier.Save(Parameters("FILE_SAVE")->asString());

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CGrid_Classify_Supervised::Set_Classification(CSG_Classifier_Supervised &Classifier)
{
	//-----------------------------------------------------
	CSG_Grid	*pClasses	= Parameters("CLASSES")->asGrid();

	//-----------------------------------------------------
	CSG_Parameter	*pLUT	= DataObject_Get_Parameter(pClasses, "LUT");

	if( pLUT && pLUT->asTable() )
	{
		for(int iClass=0; iClass<Classifier.Get_Class_Count(); iClass++)
		{
			CSG_Table_Record	*pClass	= pLUT->asTable()->Get_Record(iClass);

			if( !pClass )
			{
				(pClass	= pLUT->asTable()->Add_Record())->Set_Value(0, SG_Color_Get_Random());
			}

			pClass->Set_Value(1, Classifier.Get_Class_ID(iClass).c_str());
			pClass->Set_Value(2, "");
			pClass->Set_Value(3, iClass + 1);
			pClass->Set_Value(4, iClass + 1);
		}

		pLUT->asTable()->Set_Record_Count(Classifier.Get_Class_Count());

		DataObject_Set_Parameter(pClasses, pLUT);
		DataObject_Set_Parameter(pClasses, "COLORS_TYPE", 1);	// Color Classification Type: Lookup Table
	}

	pClasses->Set_Name(CSG_String::Format("%s [%s]", _TL("Classification"), CSG_Classifier_Supervised::Get_Name_of_Method(Parameters("METHOD")->asInt()).c_str()));

	//-----------------------------------------------------
	CSG_Grid	*pQuality	= Parameters("QUALITY")->asGrid();

	if( pQuality )
	{
		DataObject_Set_Colors(pQuality, 11, SG_COLORS_YELLOW_GREEN);

		pQuality->Set_Name(CSG_String::Format("%s [%s]", _TL("Classification Quality"), CSG_Classifier_Supervised::Get_Name_of_Quality(Parameters("METHOD")->asInt()).c_str()));
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
