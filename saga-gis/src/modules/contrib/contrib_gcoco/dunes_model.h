#ifndef HEADER_INCLUDED__DUNES_MODEL_H
#define HEADER_INCLUDED__DUNES_MODEL_H

#include "MLB_Interface.h"
#include "array.h"

class CDunes : public CSG_Module
{
public:
	CDunes(void);
	virtual ~CDunes(void);
	virtual CSG_String	Get_MenuPath	(void)	{	return( _TL("Dunes") );	}
protected:
	virtual bool On_Execute(void);
	void angles_cal(Array& matrix, int h_r, int w_r, double *angles);
	bool save_matrix(const char *filename,Array& matrix);
	void GridToArray(CSG_Grid* grid, Array& arr);
	void ArrayToGrid(Array& arr, CSG_Grid* grid);
};

#endif