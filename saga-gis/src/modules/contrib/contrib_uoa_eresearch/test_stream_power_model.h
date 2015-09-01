/**********************************************************
 * Version $Id: stream_power_model.h 1925 2014-01-09 12:15:18Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                       contrib_uoa_eresearch                        //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                      stream_power_model.h                      //
//                                                       //
//                 Copyright (C) 2007 by                 //
//                        Author                         //
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
//    e-mail:     author@email.de                        //
//                                                       //
//    contact:    Author                                 //
//                Sesame Street. 7                       //
//                12345 Metropolis                       //
//                Nirvana                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#ifndef HEADER_INCLUDED__test_stream_power_model_H
#define HEADER_INCLUDED__test_stream_power_model_H

//---------------------------------------------------------
#include "MLB_Interface.h"

#define FREE_ARG char*
#define NR_END 1

#define sqrt2 1.414213562373f
#define oneoversqrt2 0.707106781186f
#define fillincrement 0.01f

#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)

#define SWAP(a,b) itemp=(a);(a)=(b);(b)=itemp;
#define M 7
#define NSTACK 100000


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
// Use the 'stream_power_model_EXPORT' macro as defined in
// 'MLB_Interface.h' to export this class to allow other
// programs/libraries to use its functions:
//
// class stream_power_model_EXPORT Cstream_power_model : public CSG_Module
// ...
//

class Ctest_stream_power_model : public CSG_Module
{
public:
	Ctest_stream_power_model(void);
	virtual ~Ctest_stream_power_model(void);
	virtual CSG_String	Get_MenuPath	(void)	{	return( _TL("Test Stream Power Model") );	}


protected:

	virtual bool	On_Execute	(void);


private:
	char * fname;
	float **flow1,**flow2,**flow3,**flow4,**flow5,**flow6,**flow7,**flow8,**flow;
	float **topo,**topoold,**topo2,**slope,deltax,*ax,*ay,*bx,*by,*cx,*cy,*ux,*uy;
	float *rx,*ry,U,K,D,duration,timestep,*topovec,thresh,thresholdarea;
	int *topovecind,lattice_size_x,lattice_size_y, *iup,*idown,*jup,*jdown;
	void streampower();
	void setupgridneighbors();
	int *ivector(long nl, long nh);
	float **matrix(long nrl, long nrh, long ncl, long nch);
	int **imatrix(long nrl, long nrh, long ncl, long nch);
	float *vector(long nl, long nh);
	float gasdev(int *idum);
	float ran3(int *idum);
	void hillslopediffusioninit();
	void tridag(float a[], float b[], float c[], float r[], float u[], unsigned long n);
	void free_ivector(int *v, long nl, long nh);
	void free_vector(float *v, long nl, long nh);
	void avalanche(int i, int j);
	void calculatealongchannelslope(int i, int j);
	void mfdflowroute(int i, int j);
	void fillinpitsandflats(int i, int j);
	void indexx(int n, float *arr, int *indx);
	void testWrite(void);
	CSG_Grid *InputGrid, *OutputGrid;
};


///////////////////////////////////////////////////////////
//														 //
//														 //
//														 //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#endif // #ifndef HEADER_INCLUDED__test_stream_power_model_H
