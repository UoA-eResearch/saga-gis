/**********************************************************
 * Version $Id: xyz.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       Shapes_IO                       //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                        Xyz.cpp                        //
//                                                       //
//                 Copyright (C) 2003 by                 //
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
#include "xyz.h"


///////////////////////////////////////////////////////////
//														 //
//						Export							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CXYZ_Export::CXYZ_Export(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("Export Shapes to XYZ"));

	Set_Author		(SG_T("O.Conrad (c) 2003"));

	Set_Description	(_TW(
		"XYZ export filter for shapes. "
	));

	//-----------------------------------------------------
	pNode	= Parameters.Add_Shapes(
		NULL	, "POINTS"	, _TL("Shapes"),
		_TL(""),
		PARAMETER_INPUT
	);

	Parameters.Add_Table_Field(
		pNode	, "FIELD"	, _TL("Attribute"),
		_TL(""),
		true
	);

	Parameters.Add_Value(
		NULL	, "HEADER"	, _TL("Save Table Header"),
		_TL(""),
		PARAMETER_TYPE_Bool	, true
	);

	Parameters.Add_Choice(
		NULL	, "SEPARATE", _TL("Separate Line/Polygon Points"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|"),
			_TL("none"),
			_TL("*"),
			_TL("number of points")
		), 0
	);

	Parameters.Add_FilePath(
		NULL	, "FILENAME", _TL("File"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s"),
			_TL("XYZ Files (*.xyz)")	, SG_T("*.xyz"),
			_TL("Text Files (*.txt)")	, SG_T("*.txt"),
			_TL("All Files")			, SG_T("*.*")
		), NULL, true
	);
}

//---------------------------------------------------------
bool CXYZ_Export::On_Execute(void)
{
	bool		bHeader;
	int			Field, off_Field, Separate;
	CSG_File	Stream;
	CSG_Shapes	*pPoints;

	//-----------------------------------------------------
	pPoints		= Parameters("POINTS"  )->asShapes();
	bHeader		= Parameters("HEADER"  )->asBool();
	Field		= Parameters("FIELD"   )->asInt();
	Separate	= pPoints->Get_Type() == SHAPE_TYPE_Point ? 0
				: Parameters("SEPARATE")->asInt();
	off_Field	= pPoints->Get_ObjectType() == DATAOBJECT_TYPE_PointCloud ? 2 : 0;

	//-----------------------------------------------------
	if( pPoints->Get_Field_Count() == 0 )
	{
		Error_Set(_TL("data set has no attributes"));

		return( false );
	}

	//-----------------------------------------------------
	if( !Stream.Open(Parameters("FILENAME")->asString(), SG_FILE_W, false) )
	{
		Error_Set(_TL("could not open file"));

		return( false );
	}

	//-----------------------------------------------------
	if( bHeader )
	{
		Stream.Printf(SG_T("X\tY"));

		if( Field < 0 )
		{
			for(int iField=off_Field; iField<pPoints->Get_Field_Count(); iField++)
			{
				Stream.Printf(SG_T("\t%s"), pPoints->Get_Field_Name(iField));
			}
		}
		else
		{
			Stream.Printf(SG_T("\tZ"));
		}

		Stream.Printf(SG_T("\n"));
	}

	//-------------------------------------------------
	for(int iShape=0; iShape<pPoints->Get_Count() && Set_Progress(iShape, pPoints->Get_Count()); iShape++)
	{
		CSG_Shape	*pShape	= pPoints->Get_Shape(iShape);

		for(int iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
		{
			switch( Separate )
			{
			case 1:	// *
				Stream.Printf(SG_T("*\n"));
				break;

			case 2:	// number of points
				Stream.Printf(SG_T("%d\n"), pShape->Get_Point_Count(iPart));
				break;
			}

			for(int iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
			{
				TSG_Point	Point	= pShape->Get_Point(iPoint, iPart);

				Stream.Printf(SG_T("%f\t%f"), Point.x, Point.y);

				if( Field < 0 )
				{
					for(int iField=off_Field; iField<pPoints->Get_Field_Count(); iField++)
					{
						switch( pPoints->Get_Field_Type(iField) )
						{
						case SG_DATATYPE_String:
						case SG_DATATYPE_Date:
							Stream.Printf(SG_T("\t\"%s\""), pShape->asString(iField));
							break;

						default:
							Stream.Printf(SG_T("\t%f")    , pShape->asDouble(iField));
							break;
						}
					}
				}
				else switch( pPoints->Get_Field_Type(Field) )
				{
				case SG_DATATYPE_String:
				case SG_DATATYPE_Date:
					Stream.Printf(SG_T("\t\"%s\""), pShape->asString(Field));
					break;

				default:
					Stream.Printf(SG_T("\t%f")    , pShape->asDouble(Field));
					break;
				}

				Stream.Printf(SG_T("\n"));
			}
		}
	}

	//-------------------------------------------------
	return( true );
}


///////////////////////////////////////////////////////////
//														 //
//						Import							 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CXYZ_Import::CXYZ_Import(void)
{
	CSG_Parameter	*pNode;

	//-----------------------------------------------------
	Set_Name		(_TL("Import Shapes from XYZ"));

	Set_Author		("O.Conrad (c) 2003");

	Set_Description	(_TW(
		"Imports points from a table with only list of x, y, z coordinates provided as simple text. "
		"If your table has a more complex structure, you should import it as table "
		"and then use the \'points from table\' conversion tool. "
	));

	//-----------------------------------------------------
	pNode	= Parameters.Add_Shapes(
		NULL	, "POINTS"		, _TL("Points"),
		_TL(""),
		PARAMETER_OUTPUT, SHAPE_TYPE_Point
	);

	Parameters.Add_Value(
		NULL	, "HEADLINE"	, "File contains headline",
		_TL(""),
		PARAMETER_TYPE_Bool		, true
	);

	Parameters.Add_FilePath(
		NULL	, "FILENAME"	, _TL("File"),
		_TL(""),
		CSG_String::Format(SG_T("%s|%s|%s|%s|%s|%s"),
			_TL("XYZ Files (*.xyz)")	, SG_T("*.xyz"),
			_TL("Text Files (*.txt)")	, SG_T("*.txt"),
			_TL("All Files")			, SG_T("*.*")
		), NULL, false
	);
}

//---------------------------------------------------------
bool CXYZ_Import::On_Execute(void)
{
	CSG_File	Stream;

	if( !Stream.Open(Parameters("FILENAME")->asString(), SG_FILE_R) )
	{
		Error_Set(_TL("file could not be opened"));

		return( false );
	}

	//-----------------------------------------------------
	CSG_Shapes	*pPoints	= Parameters("POINTS")->asShapes();

	pPoints->Create(SHAPE_TYPE_Point, SG_File_Get_Name(Parameters("FILENAME")->asString(), false));

	pPoints->Add_Field("Z", SG_DATATYPE_Double);

	//-----------------------------------------------------
	if( Parameters("HEADLINE")->asBool() )
	{
		CSG_String	sLine;

		if( !Stream.Read_Line(sLine) )
		{
			Error_Set(_TL("could not read headline"));

			return( false );
		}
	}

	//-----------------------------------------------------
	long	Length	= Stream.Length();

	double	x, y, z;

	while( Stream.Scan(x) && Stream.Scan(y) && Stream.Scan(z) && Set_Progress(Stream.Tell(), Length) )
	{
		CSG_Shape	*pPoint	= pPoints->Add_Shape();

		pPoint->Add_Point(x, y);

		pPoint->Set_Value(0, z);
	}

	return( pPoints->Get_Count() > 0 );
}


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
