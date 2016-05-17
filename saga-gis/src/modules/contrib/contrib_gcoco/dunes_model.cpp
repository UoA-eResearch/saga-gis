/**********************************************************
 * Version $Id: Life.cpp 1921 2014-01-09 10:24:11Z oconrad $
 *********************************************************/

///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//                    Module Library:                    //
//                   Cellular_Automata                   //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                       Life.cpp                        //
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

#include "dunes_model.h"
#include <random>

#define NEIGHBOURS 4

CDunes::CDunes(void)
{
	Set_Name		(_TL("Dunes"));
	Set_Author		(SG_T("S. Masoud-Ansari, G. Soudlenkov, G. Coco, J. Tunnicliffe 2014-2015"));
	Set_Description	(_TW("Giovanni's dune code"));

	Parameters.Add_Grid(NULL, "INPUT"	, _TL("Input DEM"), _TL("Initial topography"), PARAMETER_INPUT);
	//Parameters.Add_Value(NULL, "COUNT", _TL("Output period"), _TL("Output period"), PARAMETER_TYPE_Int, 100, 1, true);
	Parameters.Add_Value(NULL, "LSITES", _TL("L sites"), _TL("L sites"), PARAMETER_TYPE_Int, 5, 1, true);
	Parameters.Add_Value(NULL, "NSLABS", _TL("Number of slabs"), _TL("Number of slabs"), PARAMETER_TYPE_Int, 1000, 1, true);
	Parameters.Add_Value(NULL, "SHADOW", _TL("Shadow"), _TL("Shadow"), PARAMETER_TYPE_Double, 1.0, 0.0, true);
	Parameters.Add_Value(NULL, "REPOSE", _TL("Repose"), _TL("Repose"), PARAMETER_TYPE_Double, 1.0, 0.0, true);
	Parameters.Add_Value(NULL, "Z_SLAB", _TL("Z slab"), _TL("Z slab"), PARAMETER_TYPE_Double, 1.0, 0.0, true);
    Parameters.Add_Value(NULL, "P_NS", _TL("p_ns"), _TL("p_ns"), PARAMETER_TYPE_Double, 0.4, 0.0, true);
	Parameters.Add_Value(NULL, "P_S", _TL("p_s"), _TL("p_s"), PARAMETER_TYPE_Double, 0.6, 0.0, true);
	Parameters.Add_Grid_Output(NULL, "OUTPUT", _TL("Output"), _TL("Simulation output"));
}

CDunes::~CDunes(void) {}

void CDunes::GridToArray(CSG_Grid* grid, Array& arr)
{
	int h = arr.dim(0);
    int w = arr.dim(1);

    for(int i = 0; i < h; i++)
    {
        double* d = arr.row(i);
        for(int j = 0; j < w; j++)
        {
			d[j] = grid->asDouble(i, j);
        }
    }
}


void CDunes::ArrayToGrid(Array& arr, CSG_Grid* grid)
{
	
	int h = arr.dim(0);
    int w = arr.dim(1);

    for(int i = 0; i < h; i++)
    {
        double* d = arr.row(i);
        for(int j = 0; j < w; j++)
        {
			grid->Set_Value(i, j, d[j]);
        }
    }
}

bool CDunes::On_Execute(void)
{
	Array matrix;
	CSG_Grid* grid_input = Parameters("INPUT")->asGrid();
	int width = grid_input->Get_NX();
	int height = grid_input->Get_NY();
	matrix.init(2, height, width);
	GridToArray(grid_input, matrix);

	CSG_Grid* output = SG_Create_Grid(SG_DATATYPE_Float, grid_input->Get_NX(), grid_input->Get_NY(), grid_input->Get_Cellsize(), grid_input->Get_XMin(), grid_input->Get_YMin());
	output->Set_Name(_TL("Output"));
	Parameters("OUTPUT")->Set_Value(output);
	ArrayToGrid(matrix, output);
	DataObject_Update(output, true);

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    //const unsigned long N_slabs=90000000;
	unsigned long N_slabs = (unsigned long)(Parameters("NSLABS")->asInt());
	int l_sites = Parameters("LSITES")->asInt();
	double shadow = Parameters("SHADOW")->asDouble();
    double repose = Parameters("REPOSE")->asDouble();
    double z_slab = Parameters("Z_SLAB")->asDouble();
	double p_ns = Parameters("P_NS")->asDouble();
    double p_s = Parameters("P_S")->asDouble();
    //int addcount = Parameters("COUNT")->asInt();
    unsigned long h,w,h_move,w_move,w_drop;
    double angles[NEIGHBOURS];
    char name[256]="";
    //int count=5000000;
	//int count = 0;
	

	//Message_Add(CSG_String("Starting ..."));
	Set_Progress(1.0);
    for(unsigned long i = 0; Process_Get_Okay(true) && i < N_slabs; i++)
    {

        double number1=distribution(generator)*height;
        h=floor(number1);
        double number2=distribution(generator)*width;
        w=floor(number2);
        if(!matrix(h,w)) //if#3 no pick-ups when there is no sand
            matrix(h,w)=matrix(h,w);
        else if(!w)
        {
            if(matrix(h,width-1)-matrix(h,w)< shadow)
            { 
                matrix(h,w)-=z_slab;
                h_move=h;
                w_move=w;
                while(Process_Get_Okay(true))
                {
					//Message_Add(CSG_String("First while loop before angles_cal"));
                    angles_cal(matrix,h_move,w_move,angles);
					//Message_Add(CSG_String("First while loop after angles_cal"));
                    if(!w_move && !h_move && angles[0]>repose)
                    {
                        matrix(h_move,width-1)-=z_slab;
                        matrix(h_move,w_move)+=z_slab;
                        w_move=width-1;
                    }
                    else if(!w_move && !h_move && angles[1] > repose)
                    {
                        matrix(height-1,w_move)-=z_slab;
                        matrix(h_move,w_move)+=z_slab;
                        h_move=height-1;
                    }
                    else if(w_move==width-1 &&  !h_move && angles[1] > repose)
                    {
                        matrix(height-1,w_move)-=z_slab;
                        matrix(h_move,w_move)+=z_slab;
                        h_move=height-1;
                    }
                    else if (w_move==width-1 && !h_move &&  angles[2] > repose)
                    {
                        matrix(h_move,0)-=z_slab;
                        matrix(h_move,w_move)+=z_slab;
                        w_move=0;
                    }
                    else if(w_move==width-1 && h_move==height-1 && angles[2] > repose)
                    {
                        matrix(h_move,0)-=z_slab;
                        matrix(h_move,w_move)+=z_slab;
                        w_move=0;
                    }
                    else if(w_move==width-1 && h_move==height-1 && angles[3] > repose)
                    {
                         matrix(0,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=0;
                    }
                    else if(!w_move && h_move==height-1 && angles[0] > repose)
                    {
                         matrix(h_move,width-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=width-1;
                    }
                    else if(!w_move && h_move==height-1 && angles[3] > repose)
                    {
                         matrix(0,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=0;
                    }
                    else if(!w_move && angles[0] > repose)
                    {
                         matrix(h_move,width-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=width-1;
                    }
                    else if(!h_move && angles[1] > repose)
                    { 
                         matrix(height-1,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;  
                         h_move=height-1;
                    }
                    else if(w_move==width-1 && angles[2] > repose)
                    {
                         matrix(h_move,0)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=0;
                    }
                    else if(h_move==height-1 &&  angles[3] > repose)
                    {
                         matrix(0,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=0;
                    }
                    else if(angles[0] > repose)
                    {
                         matrix(h_move,w_move-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move-=1;
                    }
                    else if(angles[1] > repose)
                    {
                         matrix(h_move-1,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move-=1;
                    }
                    else if(angles[2] > repose)
                    {
                         matrix(h_move,w_move+1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move+=1;
                    }
                    else if(angles[3] > repose)
                    {
                         matrix(h_move+1,w_move)-=z_slab;  
                         matrix(h_move,w_move)+=z_slab;
                         h_move+=1;
                    }
                    else
					{
                         break;
					}
                }
                w_drop+=w+l_sites;//  !create new x-dimension of cell (l-number of lattice sites in transport direction)   
                while(Process_Get_Okay(true))
                {
					//Message_Add(CSG_String("Second while loop"));
                    if(w_drop > width-1)//! if#6 boundary 
                        w_drop-=width-1;
                    double number3=distribution(generator);
                    if(!matrix(h,w_drop) && number3 > p_ns) // ! if#7 if there is no sand, probability that slab cannot be settled is 1-p_ns
                        w_drop+=l_sites;
                    else if(matrix(h,w_drop)>0.0 && number3>p_s) // ! if there is sand, probability that slab cannot be settled is 1-p_s
                        w_drop+=l_sites; 
                    else
                    {
                        matrix(h,w_drop)+=z_slab;
                        break;
                    }
                }   
                while(Process_Get_Okay(true))
                {
					//Message_Add(CSG_String("Third while loop before angles_cal"));
                    angles_cal(matrix,h,w_drop,angles); // ! differences in values of the cell and its 4 neighbours
					//Message_Add(CSG_String("Third while loop after angles_cal"));
                    if(!w_drop && !h && angles[0] < -repose)
                    {
                        matrix(h,width-1)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        w_drop=width-1;
                    }
                    else if (!w_drop && !h && angles[1] < -repose)
                    {
                        matrix(height-1,w_drop)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[1] < -repose)
                    {
                        matrix(height-1,w_drop)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[2] < -repose)
                    {
                        matrix(h,0)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        w_drop=0;
                    }
                    else if (w_drop==width-1 &&  h==height-1 &&  angles[2] < -repose)
                    {
                        matrix(h,0)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        w_drop=0;
                    }
                    else if (w_drop==width-1 && h==height-1 && angles[3] < -repose)
                    {
                        matrix(0,w_drop)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        h=0;
                    }
                    else if (!w_drop && h==height-1 && angles[0] < -repose)
                    {
                        matrix(h,width-1)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        w_drop=width-1;
                    }
                    else if (!w_drop && h==height-1 && angles[3] < -repose)
                    {
                        matrix(0,w_drop)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        h=0;
                    }
                    else if (!w_drop && angles[0] < -repose)
                    {
                        matrix(h,width-1)+=z_slab;
                        matrix(h,w_drop)-=z_slab;
                        w_drop=width-1;
                    }
                    else if (!h && angles[1] < -repose)
                    {
                         matrix(height-1,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && angles[2] < -repose)
                    {
                         matrix(h,0)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=0;
                    }
                    else if (h==height-1 && angles[3] < -repose)
                    {
                         matrix(0,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=0;
                    }
                    else if (angles[0] < -repose)
                    {
                         matrix(h,w_drop-1)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop--;
                    }
                    else if (angles[1] < -repose)
                    {
                         matrix(h-1,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h--;
                    }
                    else if (angles[2] < -repose)
                    {
                         matrix(h,w_drop+1)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop++;
                    }
                    else if (angles[3] < -repose)
                    {
                         matrix(h+1,w_drop)+=z_slab;  
                         matrix(h,w_drop)-=z_slab;
                         h++;
                    }
                    else
                         break; 
                }
            }
        }
        else if (w > 0) 
        {
            if(matrix(h,w-1)-matrix(h,w) < shadow) // if#4 pick-ups when not in shadow zone
            { 
                matrix(h,w)-=z_slab;// ! remove slab when there is a pick-up
                h_move=h; // ! new name y-dimension of cell, so it can move around
                w_move=w; // ! new name x-dimension of cell, so it can move around
                //as long as eolian==1 (the angle of repose criterion is violated), neigbouring slabs are moving downslope
                while(Process_Get_Okay(true))
                {
					//Message_Add(CSG_String("Fourth while loop before angles_cal"));
                    angles_cal(matrix,h_move,w_move,angles);// ! differences in values of the cell and its 4 neighbours
					//Message_Add(CSG_String("Fourth while loop after angles_cal"));
                    if (!w_move && !h_move && angles[0] > repose)
                    {
                         matrix(h_move,width-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=width-1;
                    }
                    else if (!w_move && !h_move &&  angles[1] > repose)
                    {
                         matrix(height-1,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=height-1;
                    }
                    else if (w_move==width-1 && !h_move && angles[1] > repose)
                    {
                         matrix(height-1,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=height-1;
                    }
                    else if (w_move==width-1 && !h_move && angles[2] > repose)
                    {
                         matrix(h_move,0)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=0;
                    }
                    else if (w_move==width-1 && h_move==height-1 && angles[2] > repose)
                    {
                         matrix(h_move,0)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=0;
                    }
                    else if (w_move==width-1 && h_move==height-1 && angles[3] > repose)
                    {
                         matrix(0,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=0;
                    }
                    else if (!w_move && h_move==height-1 && angles[0] > repose)
                    {
                         matrix(h_move,width-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=width-1;
                    }
                    else if (!w_move && h_move==height-1 && angles[3] > repose)
                    {
                         matrix(0,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=0;
                    }
                    else if (!w_move && angles[0] > repose)
                    {
                         matrix(h_move,width-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab; 
                         w_move=width-1;
                    }
                    else if (!h_move && angles[1] > repose)
                    {
                         matrix(height-1,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=height-1;
                    }
                    else if (w_move==width-1 &&  angles[2] > repose)
                    {
                         matrix(h_move,0)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move=0;
                    }
                    else if (h_move==height-1 && angles[3] > repose)
                    {
                         matrix(0,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move=0;
                    }
                    else if (angles[0] > repose)
                    {
                         matrix(h_move,w_move-1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move--;
                    }
                    else if (angles[1] > repose)
                    {
                         matrix(h_move-1,w_move)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         h_move--;
                    }
                    else if (angles[2] > repose)
                    {
                         matrix(h_move,w_move+1)-=z_slab;
                         matrix(h_move,w_move)+=z_slab;
                         w_move++;
                    }
                    else if (angles[3] > repose)
                    {
                         matrix(h_move+1,w_move)-=z_slab;  
                         matrix(h_move,w_move)+=z_slab;  
                         h_move++;
                    }
                    else
                         break;
                }
                w_drop=w+l_sites;//  !create new x-dimension of cell (l-number of lattice sites in transport direction)   
                while(Process_Get_Okay(true))
                {
					//Message_Add(CSG_String("Fifth while loop"));
                    if (w_drop >= width)
                        w_drop-=width;
                    double number3=distribution(generator);
                    if(!matrix(h,w_drop) && number3 > p_ns)//if#7 if there is no sand, probability that slab cannot be settled is 1-p_ns
                        w_drop+=l_sites;
                    else if (matrix(h,w_drop) > 0 && number3 > p_s) // then ! if there is sand, probability that slab cannot be settled is 1-p_s
                         w_drop+=l_sites;
                    else
                    {
                         matrix(h,w_drop)+=z_slab;
                         break;
                    }
                }
                while(Process_Get_Okay(true))
                {
					//Message_Add(CSG_String("Sixth while loop before angles_cal"));
                    angles_cal(matrix, h, w_drop, angles);// ! differences in values of the cell and its 4 neighbours
					//Message_Add(CSG_String("Sixth while loop after angles_cal"));
                    if (!w_drop && !h && angles[0] < -repose)
                    {
                         matrix(h,width)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=width-1;
                    }
                    else if (!w_drop && !h && angles[1] < -repose)
                    {
                         matrix(height,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[1] < -repose)
                    {
                         matrix(height-1,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[2] < -repose)
                    {
                         matrix(h,0)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=0;
                    }
                    else if (w_drop==width-1 && h==height-1 && angles[2] < -repose)
                    {
                         matrix(h,0)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=0;
                    }
                    else if (w_drop==width-1 && h==height-1 && angles[3] < -repose)
                    {
                         matrix(0,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=0;
                    }
                    else if (!w_drop && h==height-1 && angles[0] < -repose)
                    {
                         matrix(h,width-1)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=width-1;
                    }
                    else if (!w_drop && h==height-1 && angles[3] < -repose)
                    {
                         matrix(0,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=0;
                    }
                    else if (!w_drop && angles[0] < -repose)
                    {
                         matrix(h,width-1)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=width-1;
                    }
                    else if (!h && angles[1] < -repose)
                    {
                         matrix(height-1,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && angles[2] < -repose)
                    {
                         matrix(h,0)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop=0;
                    }
                    else if (h==height-1 && angles[3] < -repose)
                    {
                         matrix(0,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h=0;
                    }
                    else if (angles[0] < -repose)
                    {
                         matrix(h,w_drop-1)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop--;
                    }
                    else if (angles[1] < -repose)
                    {
                         matrix(h-1,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h--;
                    }
                    else if (angles[2] < -repose)
                    {
                         matrix(h,w_drop+1)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         w_drop++;
                    }
                    else if (angles[3] < -repose)
                    {
                         matrix(h+1,w_drop)+=z_slab;
                         matrix(h,w_drop)-=z_slab;
                         h++;
                    }
                    else
					{
                         break;
					}
                }
            }
        }
		/*
        if(i==count)
        {
            char name[256];
            sprintf(name,"output%lu.txt",i);
            save_matrix(name,matrix);
            count+=addcount;
        }
		*/

		//Message_Add(CSG_String::Format(SG_T("slab %d"), i));
		Set_Progress( (i / (float)N_slabs) * 100 );
		ArrayToGrid(matrix, output);
		DataObject_Update(output, true);
    }  


	return( true );
}

bool CDunes::save_matrix(const char *filename,Array& matrix)
{
	/*
    int height=matrix.dim(0);
    int width=matrix.dim(1);
    FILE *f=fopen(filename,"wt");

    if(!f)
        return false;

    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
            fprintf(f,"%.0lf ",matrix(i,j));
        fprintf(f,"\n");
    }
	*/
	Process_Set_Text(CSG_String::Format(SG_T("%s"), filename));
    return true;
}

void CDunes::angles_cal(Array& matrix,int h_r,int w_r,double *angles)
{
/*    subroutine angles_cal (matrix, h_r, w_r, height, width, neigh, angles)
    ! DEALS WITH BOUNDARY CONDITIONS OF A 2-DIMENSIONAL MATRIX
    ! Output: angles; calculated differences in values of a certain cell (h_r,w_r) and its
    ! four neighbours (code specifies neighbours for cells at the edges of the matrix 
    ! and at the corners).

    ! Input:
    ! matrix: for dune model; matrix is the morphology
    ! h_r: random number for the y-dimension
    ! w_r: random number for the x-dimension
    ! height: number of cells in y-dimension
    ! width: number of cells in x-direction
    ! neigh: number of neighbours
*/

    int height=matrix.dim(0);
    int width=matrix.dim(1);
    int left,top,right,bottom;
    
    if (!w_r && !h_r)
    {
        left=matrix(h_r,width-1)-matrix(h_r,w_r);
        top=matrix(height-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);
    }
    else if (w_r==width-1 && !h_r)
    { 
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(height-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,0)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);
    }
    else if (w_r==width-1 && h_r==height-1)
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(height-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,0)-matrix(h_r,w_r);
        bottom=matrix(0,w_r)-matrix(h_r,w_r);
    }
    else if (!w_r && h_r==height-1)
    {
        left=matrix(h_r,width-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(0,w_r)-matrix(h_r,w_r);
    }
    else if (!w_r)
    {
        left=matrix(h_r,width-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);
    }
    else if (!h_r)
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(height-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);
    }
    else if (w_r==width-1)
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,1)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);     
    }
    else if (h_r==height-1)
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(1,w_r)-matrix(h_r,w_r);
    }
    else
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);
    }
    angles[0]=left;
    angles[1]=top;
    angles[2]=right;
    angles[3]=bottom;
}

