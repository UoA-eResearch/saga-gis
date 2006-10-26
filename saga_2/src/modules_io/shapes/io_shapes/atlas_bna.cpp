
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       io_shapes                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     Atlas_BNA.cpp                     //
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
// 59 Temple Place - Suite 330, Boston, MA 02111-1307,   //
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
#include "atlas_bna.h"


///////////////////////////////////////////////////////////
//														 //
//						Import							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CAtlas_BNA_Import::CAtlas_BNA_Import(void)
{
	//-----------------------------------------------------
	// 1. Info...

	Set_Name		(_TL("Import Atlas Boundary File"));

	Set_Author		(_TL("Copyrights (c) 2006 by Olaf Conrad"));

	Set_Description	(_TL(
		"\n"
	));


	//-----------------------------------------------------
	// 2. Parameters...

	Parameters.Add_FilePath(
		NULL	, "FILE"	, _TL("File"),
		"",
		_TL("Atlas Boundary Files (*.bna)|*.bna|All Files|*.*")
	);
}

//---------------------------------------------------------
CAtlas_BNA_Import::~CAtlas_BNA_Import(void)
{}

//---------------------------------------------------------
bool CAtlas_BNA_Import::On_Execute(void)
{
	bool			bOk;
	int				iPoint, nPoints;
	double			x, y;
	FILE			*Stream;
	CSG_String		FileName, sLine, sName1, sName2;
	CSG_Shape			*pShape;
	CSG_Shapes			*pPoints, *pLines, *pPolygons;

	//-----------------------------------------------------
	FileName	= Parameters("FILE")	->asString();

	//-----------------------------------------------------
	if( (Stream = fopen(FileName, "r")) != NULL )
	{
		pPoints		= SG_Create_Shapes(SHAPE_TYPE_Point  , SG_File_Get_Name(FileName, true));
		pPoints		->Get_Table().Add_Field("NAME1"	, TABLE_FIELDTYPE_String);
		pPoints		->Get_Table().Add_Field("NAME2"	, TABLE_FIELDTYPE_String);

		pLines		= SG_Create_Shapes(SHAPE_TYPE_Line   , SG_File_Get_Name(FileName, true));
		pLines		->Get_Table().Add_Field("NAME1"	, TABLE_FIELDTYPE_String);
		pLines		->Get_Table().Add_Field("NAME2"	, TABLE_FIELDTYPE_String);

		pPolygons	= SG_Create_Shapes(SHAPE_TYPE_Polygon, SG_File_Get_Name(FileName, true));
		pPolygons	->Get_Table().Add_Field("NAME1"	, TABLE_FIELDTYPE_String);
		pPolygons	->Get_Table().Add_Field("NAME2"	, TABLE_FIELDTYPE_String);

		//-------------------------------------------------
		bOk		= true;

		while( bOk && SG_Read_Line(Stream, sLine) && Process_Get_Okay(true) )
		{
			sName1	= sLine.AfterFirst('\"').BeforeFirst('\"');
			sName2	= sLine.BeforeLast('\"').AfterLast('\"');
			sLine	= sLine.AfterLast('\"');	if( sLine.Find(',', true) >= 0 )	sLine	= sLine.AfterLast(',');

			nPoints	= sLine.asInt();

			if( nPoints == 1 )
			{
				pShape	= pPoints	->Add_Shape();
			}
			else if( nPoints < 0 )
			{
				pShape	= pLines	->Add_Shape();
				nPoints	= -nPoints;
			}
			else if( nPoints > 2 )
			{
				pShape	= pPolygons	->Add_Shape();
			}
			else
			{
				bOk		= false;
			}

			if( bOk )
			{
				pShape->Get_Record()->Set_Value(0, sName1);
				pShape->Get_Record()->Set_Value(1, sName2);

				for(iPoint=0; iPoint<nPoints && bOk; iPoint++)
				{
					if( (bOk = SG_Read_Line(Stream, sLine)) == true )
					{
						sscanf(sLine, "%lf %lf", &x, &y);
						pShape->Add_Point(x, y);
					}
				}
			}
		}

		fclose(Stream);

		//-------------------------------------------------
		bOk		= false;

		if( pPoints->is_Valid() && pPoints->Get_Count() > 0 )
		{
			bOk		= true;
			DataObject_Add(pPoints);
		}
		else
		{
			delete(pPoints);
		}

		if( pLines->is_Valid() && pLines->Get_Count() > 0 )
		{
			bOk		= true;
			DataObject_Add(pLines);
		}
		else
		{
			delete(pLines);
		}

		if( pPolygons->is_Valid() && pPolygons->Get_Count() > 0 )
		{
			bOk		= true;
			DataObject_Add(pPolygons);
		}
		else
		{
			delete(pPolygons);
		}

		return( bOk );
	}

	//-----------------------------------------------------
	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//						Export							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CAtlas_BNA_Export::CAtlas_BNA_Export(void)
{
	//-----------------------------------------------------
	// 1. Info...

	Set_Name		(_TL("Export Atlas Boundary File"));

	Set_Author		(_TL("Copyrights (c) 2006 by Olaf Conrad"));

	Set_Description	(_TL(
		"\n"
	));


	//-----------------------------------------------------
	// 2. Parameters...

	CSG_Parameter	*pNode;

	pNode	= Parameters.Add_Shapes(
		NULL	, "SHAPES"	, _TL("Shapes"),
		"",
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field(
		pNode	, "PNAME"	, _TL("Primary Name"),
		""
	);

	Parameters.Add_Table_Field(
		pNode	, "SNAME"	, _TL("Secondary Name"),
		""
	);

	Parameters.Add_FilePath(
		NULL	, "FILE"	, _TL("File"),
		"",
		_TL(
		"Atlas Boundary Files (*.bna)|*.bna|All Files|*.*"), NULL, true
	);
}

//---------------------------------------------------------
CAtlas_BNA_Export::~CAtlas_BNA_Export(void)
{}

//---------------------------------------------------------
bool CAtlas_BNA_Export::On_Execute(void)
{
	int			iShape, iPart, iPoint, iName1, iName2;
	FILE		*Stream;
	TSG_Point	p;
	CSG_Points	Pts;
	CSG_Shape		*pShape;
	CSG_Shapes		*pShapes;

	//-----------------------------------------------------
	pShapes	= Parameters("SHAPES")	->asShapes();

	iName1	= Parameters("PNAME")->asInt();
	iName2	= Parameters("SNAME")->asInt();

	//-----------------------------------------------------
	if( (Stream = fopen(Parameters("FILE")->asString(), "w")) != NULL )
	{
		for(iShape=0; iShape<pShapes->Get_Count() && Set_Progress(iShape, pShapes->Get_Count()); iShape++)
		{
			pShape	= pShapes->Get_Shape(iShape);

			switch( pShapes->Get_Type() )
			{
			default:
				break;

			//---------------------------------------------
			case SHAPE_TYPE_Point:
				fprintf(Stream, "\"%s\",\"%s\",%d\n",
					pShape->Get_Record()->asString(iName1),
					pShape->Get_Record()->asString(iName2),
					1
				);

				p	= pShape->Get_Point(0);
				fprintf(Stream, "%f,%f\n", p.x, p.y);
				break;

			//---------------------------------------------
			case SHAPE_TYPE_Line:
				for(iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
				{
					fprintf(Stream, "\"%s\",\"%s\",%d\n",
						pShape->Get_Record()->asString(iName1),
						pShape->Get_Record()->asString(iName2),
						-pShape->Get_Point_Count(iPart)
					);

					for(iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
					{
						p	= pShape->Get_Point(iPoint, iPart);
						fprintf(Stream, "%f,%f\n", p.x, p.y);
					}
				}
				break;

			//---------------------------------------------
			case SHAPE_TYPE_Polygon:
				if( pShape->Get_Part_Count() > 0 && pShape->Get_Point_Count(0) > 2 )
				{
					Pts.Clear();

					for(iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
					{
						for(iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
						{
							Pts.Add(pShape->Get_Point(iPoint, iPart));
						}

						if( iPart > 0 )
						{
							Pts.Add(pShape->Get_Point(0, 0));
						}
					}

					if( Pts.Get_Count() > 2 )
					{
						fprintf(Stream, "\"%s\",\"%s\",%d\n",
							pShape->Get_Record()->asString(iName1),
							pShape->Get_Record()->asString(iName2),
							Pts.Get_Count()
						);

						for(iPoint=0; iPoint<Pts.Get_Count(); iPoint++)
						{
							fprintf(Stream, "%f,%f\n", Pts[iPoint].x, Pts[iPoint].y);
						}
					}
				}
				break;
			}
		}

		fclose(Stream);

		return( true );
	}

	//-----------------------------------------------------
	return( false );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
