#ifndef HEADER_INCLUDED__DUNES_MODEL_H
#define HEADER_INCLUDED__DUNES_MODEL_H

#include "MLB_Interface.h"

class CDunes : public CSG_Module
{
public:
	CDunes(void);
	virtual ~CDunes(void);
	virtual CSG_String	Get_MenuPath	(void)	{	return( _TL("Dunes") );	}
protected:
	virtual bool On_Execute(void);
	void angles_cal(CSG_Matrix& matrix, int h_r, int w_r, double *angles, bool debug);
	bool ExportGrid(CSG_Grid* grid, CSG_String path);
	void GridToMatrix(CSG_Grid* grid, CSG_Matrix& matrix);
	void MatrixToGrid(CSG_Matrix& matrix, CSG_Grid* grid);
	void print_matrix(CSG_Matrix& matrix);
	double GetRandom(CSG_Table* table, int index, bool debug);
};

#endif