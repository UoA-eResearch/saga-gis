#include "Dunes.h"
#include <ctime>

void Dunes::RandomUpdate()
{
	for(int y=0; y<pGrid->Get_NY(); y++)
	{
		for(int x=0; x<pGrid->Get_NX(); x++)
		{
			pGrid->Set_Value(x, y, rand()%255);
		}
	}

}

void Dunes::UpdateGrid()
{

	for(int y=0; y<pGrid->Get_NY(); y++)
	{
		for(int x=0; x<pGrid->Get_NX(); x++)
		{
			pGrid->Set_Value(x, y, 1.0f);
		}
	}
	DataObject_Update(pGrid);

}

Dunes::Dunes(void)
{
	Set_Name		(_TL("Dunes"));

	Set_Author		(SG_T("Giovanni Coco"));

	Set_Description	(_TW(
		"Dune sim code"
		));

	Parameters.Add_Grid_Output( NULL	, "GRID", _TL("Grid"), _TL("") );

}


Dunes::~Dunes(void){}


bool Dunes::On_Execute(void)
{
	pGrid = SG_Create_Grid(SG_DATATYPE_Float, 256, 256);
	pGrid->Set_Name(_TL("Dune Sim"));
	Parameters("GRID")->Set_Value(pGrid);
	DataObject_Set_Colors(pGrid, 255, SG_COLORS_DEFAULT);

	srand((unsigned)time(NULL));
	pGrid->Assign(0.0);
	DataObject_Update(pGrid, 0, 255, true);

	/*
	for (int i = 0; i < 100; i++)
	{
		RandomUpdate();
		DataObject_Update(pGrid);
	}
	*/
	return( true );
}
