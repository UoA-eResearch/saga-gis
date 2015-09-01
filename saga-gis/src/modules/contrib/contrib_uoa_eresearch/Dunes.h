#ifndef HEADER_INCLUDED__DUNES_H
#define HEADER_INCLUDED__DUNES_H

//---------------------------------------------------------
#include "MLB_Interface.h"

class Dunes : public CSG_Module
{
public:
	Dunes(void);
	virtual ~Dunes(void);
	virtual CSG_String	Get_MenuPath	(void)	{	return( _TL("Dunes") );	}


protected:

	virtual bool	On_Execute	(void);

private:
	void main();
	void RandomUpdate();
	void UpdateGrid();
	//void angles_cal(matrix, h_r, w_r, height, width, neigh, angles);

	CSG_Grid* pGrid;
};


#endif 
