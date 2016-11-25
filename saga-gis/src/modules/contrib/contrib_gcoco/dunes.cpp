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

#include "dunes.h"
#include "gdal_driver.h"
#include <random>

#define NEIGHBOURS 4

CDunes::CDunes(void)
{
	Set_Name		(_TL("Dunes"));
	Set_Author		(SG_T("S. Masoud-Ansari, G. Soudlenkov, G. Coco, J. Tunnicliffe"));
	Set_Description	(_TW("Giovanni's dune code"));

	Parameters.Add_Grid(NULL, "INPUT"	, _TL("Input DEM"), _TL("Initial topography"), PARAMETER_INPUT);
	Parameters.Add_Value(NULL, "COUNT", _TL("Output period"), _TL("Output period"), PARAMETER_TYPE_Int, 100, 1, true);
	Parameters.Add_FilePath(NULL, "OUTPUT_DIR", _TL("Output Directory"), _TL("Directory used for saving regular GeoTIFF snaphots of the model as it progresses."), NULL, NULL, false, true, false); 
	Parameters.Add_Value(NULL, "LSITES", _TL("L sites"), _TL("L sites"), PARAMETER_TYPE_Int, 5, 1, true);
	Parameters.Add_Value(NULL, "NSLABS", _TL("Number of slabs"), _TL("Number of slabs"), PARAMETER_TYPE_Int, 1000, 1, true);
	Parameters.Add_Value(NULL, "SHADOW", _TL("Shadow"), _TL("Shadow"), PARAMETER_TYPE_Double, 1.0, 0.0, true);
	Parameters.Add_Value(NULL, "REPOSE", _TL("Repose"), _TL("Repose"), PARAMETER_TYPE_Double, 1.0, 0.0, true);
	Parameters.Add_Value(NULL, "Z_SLAB", _TL("Z slab"), _TL("Z slab"), PARAMETER_TYPE_Double, 1.0, 0.0, true);
    Parameters.Add_Value(NULL, "P_NS", _TL("p_ns"), _TL("p_ns"), PARAMETER_TYPE_Double, 0.4, 0.0, true);
	Parameters.Add_Value(NULL, "P_S", _TL("p_s"), _TL("p_s"), PARAMETER_TYPE_Double, 0.6, 0.0, true);
	Parameters.Add_Grid_Output(NULL, "OUTPUT", _TL("Output"), _TL("Simulation output"));

	// for debugging
	Parameters.Add_Table(NULL, "RANDOM", _TL("Random Number Data"), _TL("N x 1 column of random numbers. Used for testing results between implementations."), PARAMETER_INPUT_OPTIONAL);
	Parameters.Add_Value(NULL, "DEBUG", _TL("Show Debug Output"), _TL("Shows information relevant for debugging"), PARAMETER_TYPE_Bool, false);

}

CDunes::~CDunes(void) {}

void CDunes::print_matrix(CSG_Matrix& matrix)
{
	Message_Add(matrix.to_String());

}


void CDunes::GridToMatrix(CSG_Grid* grid, CSG_Matrix& matrix)
{
	for (int y = 0; y < grid->Get_NY(); y++)
	{
		for (int x = 0; x < grid->Get_NX(); x ++)
		{
			matrix[y][x] = grid->asDouble(y, x);
		}
	}

}

void CDunes::MatrixToGrid(CSG_Matrix& matrix, CSG_Grid* grid)
{
	

    for(int y = 0; y < matrix.Get_NY(); y++)
    {
        for(int x = 0; x < matrix.Get_NX(); x++)
        {
			grid->Set_Value(y, x, matrix(y, x));
        }
    }


}


double CDunes::GetRandom(CSG_Table* table, int index, bool debug)
{
	CSG_Table_Record* record;
	double v = 0;
	if (table != NULL)
	{
		record = table->Get_Record(index);
		if (record != NULL)
		{
			v = record->Get_Value(0)->asDouble();
			if (debug)
			{
				Message_Add(CSG_String::Format("%f", v));
			}			
			return v;			
		}
	}
}

bool CDunes::On_Execute(void)
{
	CSG_Grid* grid_input = Parameters("INPUT")->asGrid();
	int width = grid_input->Get_NX();
	int height = grid_input->Get_NY();
	CSG_Matrix matrix (width, height);
	GridToMatrix(grid_input, matrix);	

	CSG_Grid* output = SG_Create_Grid(SG_DATATYPE_Float, grid_input->Get_NX(), grid_input->Get_NY(), grid_input->Get_Cellsize(), grid_input->Get_XMin(), grid_input->Get_YMin());
	output->Set_Name(_TL("Output"));
	Parameters("OUTPUT")->Set_Value(output);
	MatrixToGrid(matrix, output);
	DataObject_Update(output, true);

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);

	// params
	unsigned long N_slabs = (unsigned long)(Parameters("NSLABS")->asInt());
	int l_sites = Parameters("LSITES")->asInt();
	double shadow = Parameters("SHADOW")->asDouble();
    double repose = Parameters("REPOSE")->asDouble();
    double z_slab = Parameters("Z_SLAB")->asDouble();
	double p_ns = Parameters("P_NS")->asDouble();
    double p_s = Parameters("P_S")->asDouble();
    int addcount = Parameters("COUNT")->asInt();
	CSG_String outputDir = Parameters("OUTPUT_DIR")->asFilePath()->asString();

	bool debug = false;

	// debugging
	CSG_Table* random_numbers = Parameters("RANDOM")->asTable();
	debug = Parameters("DEBUG")->asBool();

	// internal
    unsigned long h = 0;
	unsigned long w = 0;
	unsigned long h_move = 0;
	unsigned long w_move = 0;
	unsigned long w_drop = 0;
    double angles[NEIGHBOURS];
    char name[256]="";
	int count = 0;
	bool save_outputs = true;
	CSG_Table_Record* record;
	int rand_table_index = 0;
	double number1, number2, number3;

	if (outputDir.is_Empty())
	{
		save_outputs = false;
		if(!Message_Dlg_Confirm(CSG_String::Format(SG_T("No output directory specified, continue anyway?")), _TL("Warning")))
		{
			return false;
		}
	}

    for(unsigned long i = 0; Process_Get_Okay(true) && i < N_slabs; i++)
    {
		CSG_String msg = CSG_String::Format(SG_T("Slab %d"), i+1);
		Process_Set_Text(msg);

		if (debug)
		{
			Message_Add(msg);
			print_matrix(matrix);		
		}

		if (random_numbers != NULL)
		{
			number1 = GetRandom(random_numbers, rand_table_index, debug);
			rand_table_index++;
			number2 = GetRandom(random_numbers, rand_table_index, debug);
			rand_table_index++;
		} else
		{
			number1 = distribution(generator); // height
			number2 = distribution(generator); // width
		}
		
		number1 *= height;
		number2 *= width;

        h=floor(number1);        		
        w=floor(number2);


        if(matrix(h,w) == 0)
		{
			//if#3 no pick-ups when there is no sand
            //matrix[h][w] = matrix(h,w);
		} 
		else if(w == 0)
        {
            if(matrix(h,width-1) - matrix(h,w) < shadow)
            { 
                matrix[h][w] -= z_slab;
                h_move = h;
                w_move = w;
                while(Process_Get_Okay(true))
                {
                    angles_cal(matrix, h_move, w_move, angles, debug);
                    if(!w_move && !h_move && angles[0] > repose)
                    {
                        matrix[h_move][width-1] -= z_slab;
                        matrix[h_move][w_move] += z_slab;
                        w_move=width-1;
                    }
                    else if(!w_move && !h_move && angles[1] > repose)
                    {
                        matrix[height-1][w_move] -= z_slab;
                        matrix[h_move][w_move] += z_slab;
                        h_move=height-1;
                    }
                    else if(w_move==width-1 &&  !h_move && angles[1] > repose)
                    {
                        matrix[height-1][w_move] -= z_slab;
                        matrix[h_move][w_move] += z_slab;
                        h_move=height-1;
                    }
                    else if (w_move==width-1 && !h_move &&  angles[2] > repose)
                    {
                        matrix[h_move][0] -= z_slab;
                        matrix[h_move][w_move] += z_slab;
                        w_move=0;
                    }
                    else if(w_move==width-1 && h_move==height-1 && angles[2] > repose)
                    {
                        matrix[h_move][0] -= z_slab;
                        matrix[h_move][w_move] += z_slab;
                        w_move=0;
                    }
                    else if(w_move==width-1 && h_move==height-1 && angles[3] > repose)
                    {
                         matrix[0][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=0;
                    }
                    else if(!w_move && h_move==height-1 && angles[0] > repose)
                    {
                         matrix[h_move][width-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=width-1;
                    }
                    else if(!w_move && h_move==height-1 && angles[3] > repose)
                    {
                         matrix[0][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=0;
                    }
                    else if(!w_move && angles[0] > repose)
                    {
                         matrix[h_move][width-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=width-1;
                    }
                    else if(!h_move && angles[1] > repose)
                    { 
                         matrix[height-1][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;  
                         h_move=height-1;
                    }
                    else if(w_move==width-1 && angles[2] > repose)
                    {
                         matrix[h_move][0] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=0;
                    }
                    else if(h_move==height-1 &&  angles[3] > repose)
                    {
                         matrix[0][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=0;
                    }
                    else if(angles[0] > repose)
                    {
                         matrix[h_move][w_move-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move-=1;
                    }
                    else if(angles[1] > repose)
                    {
                         matrix[h_move-1][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move-=1;
                    }
                    else if(angles[2] > repose)
                    {
                         matrix[h_move][w_move+1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move+=1;
                    }
                    else if(angles[3] > repose)
                    {
                         matrix[h_move+1][w_move] -= z_slab;  
                         matrix[h_move][w_move] += z_slab;
                         h_move+=1;
                    }
                    else
					{
                         break;
					}
                }
                w_drop = w + l_sites;//  !create new x-dimension of cell (l-number of lattice sites in transport direction)   
                while(Process_Get_Okay(true))
                {
                    if(w_drop > width-1)//! if#6 boundary 
					{
                        w_drop -= width;
					}

					if (random_numbers != NULL)
					{
						number3 = GetRandom(random_numbers, rand_table_index, debug);
						rand_table_index++;
					} else
					{
						number3 = distribution(generator);
					}


                    if(!matrix(h,w_drop) && number3 > p_ns) // ! if#7 if there is no sand, probability that slab cannot be settled is 1-p_ns
					{
                        w_drop += l_sites;
					}
                    else if(matrix(h,w_drop) > 0 && number3 > p_s) // ! if there is sand, probability that slab cannot be settled is 1-p_s
					{
                        w_drop += l_sites; 
					}
                    else
                    {
                        matrix[h][w_drop] += z_slab;
                        break;
                    }
                }   
                while(Process_Get_Okay(true))
                {
                    angles_cal(matrix,h,w_drop,angles, debug); // ! differences in values of the cell and its 4 neighbours
                    if(!w_drop && !h && angles[0] < -repose)
                    {
                        matrix[h][width-1] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        w_drop=width-1;
                    }
                    else if (!w_drop && !h && angles[1] < -repose)
                    {
                        matrix[height-1][w_drop] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[1] < -repose)
                    {
                        matrix[height-1][w_drop] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[2] < -repose)
                    {
                        matrix[h][0] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        w_drop=0;
                    }
                    else if (w_drop==width-1 &&  h==height-1 &&  angles[2] < -repose)
                    {
                        matrix[h][0] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        w_drop=0;
                    }
                    else if (w_drop==width-1 && h==height-1 && angles[3] < -repose)
                    {
                        matrix[0][w_drop] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        h=0;
                    }
                    else if (!w_drop && h==height-1 && angles[0] < -repose)
                    {
                        matrix[h][width-1] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        w_drop=width-1;
                    }
                    else if (!w_drop && h==height-1 && angles[3] < -repose)
                    {
                        matrix[0][w_drop] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        h=0;
                    }
                    else if (!w_drop && angles[0] < -repose)
                    {
                        matrix[h][width-1] += z_slab;
                        matrix[h][w_drop] -= z_slab;
                        w_drop=width-1;
                    }
                    else if (!h && angles[1] < -repose)
                    {
                         matrix[height-1][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && angles[2] < -repose)
                    {
                         matrix[h][0] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=0;
                    }
                    else if (h==height-1 && angles[3] < -repose)
                    {
                         matrix[0][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=0;
                    }
                    else if (angles[0] < -repose)
                    {
                         matrix[h][w_drop-1] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop--;
                    }
                    else if (angles[1] < -repose)
                    {
                         matrix[h-1][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h--;
                    }
                    else if (angles[2] < -repose)
                    {
                         matrix[h][w_drop+1] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop++;
                    }
                    else if (angles[3] < -repose)
                    {
                         matrix[h+1][w_drop] += z_slab;  
                         matrix[h][w_drop] -= z_slab;
                         h++;
                    }
                    else
					{
                         break; 
					}
                }
            }
        }
        else if (w > 0) 
        {
			if(matrix(h,w-1)-matrix(h,w) < shadow) // if#4 pick-ups when not in shadow zone
            { 
                matrix[h][w] -= z_slab;// ! remove slab when there is a pick-up
                h_move=h; // ! new name y-dimension of cell, so it can move around
                w_move=w; // ! new name x-dimension of cell, so it can move around
                //as long as eolian==1 (the angle of repose criterion is violated), neigbouring slabs are moving downslope
                while(Process_Get_Okay(true))
                {
                    angles_cal(matrix,h_move,w_move,angles, debug);// ! differences in values of the cell and its 4 neighbours
                    if (!w_move && !h_move && angles[0] > repose)
                    {
                         matrix[h_move][width-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=width-1;
                    }
                    else if (!w_move && !h_move &&  angles[1] > repose)
                    {
                         matrix[height-1][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=height-1;
                    }
                    else if (w_move==width-1 && !h_move && angles[1] > repose)
                    {
                         matrix[height-1][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=height-1;
                    }
                    else if (w_move==width-1 && !h_move && angles[2] > repose)
                    {
                         matrix[h_move][0] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=0;
                    }
                    else if (w_move==width-1 && h_move==height-1 && angles[2] > repose)
                    {
                         matrix[h_move][0] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=0;
                    }
                    else if (w_move==width-1 && h_move==height-1 && angles[3] > repose)
                    {
                         matrix[0][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=0;
                    }
                    else if (!w_move && h_move==height-1 && angles[0] > repose)
                    {
                         matrix[h_move][width-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=width-1;
                    }
                    else if (!w_move && h_move==height-1 && angles[3] > repose)
                    {
                         matrix[0][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=0;
                    }
                    else if (!w_move && angles[0] > repose)
                    {
                         matrix[h_move][width-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab; 
                         w_move=width-1;
                    }
                    else if (!h_move && angles[1] > repose)
                    {
                         matrix[height-1][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=height-1;
                    }
                    else if (w_move==width-1 &&  angles[2] > repose)
                    {
                         matrix[h_move][0] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move=0;
                    }
                    else if (h_move==height-1 && angles[3] > repose)
                    {
                         matrix[0][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move=0;
                    }
                    else if (angles[0] > repose)
                    {
                         matrix[h_move][w_move-1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move--;
                    }
					else if (angles[1] > repose) 
                    {
                         matrix[h_move-1][w_move] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         h_move--;
                    }
                    else if (angles[2] > repose)
                    {
                         matrix[h_move][w_move+1] -= z_slab;
                         matrix[h_move][w_move] += z_slab;
                         w_move++;
                    }
                    else if (angles[3] > repose)
                    {
                         matrix[h_move+1][w_move] -= z_slab;  
                         matrix[h_move][w_move] += z_slab;  
                         h_move++;
                    }
                    else
					{
                         break;
					}
                }
                w_drop= w + l_sites;//  !create new x-dimension of cell (l-number of lattice sites in transport direction)   
                while(Process_Get_Okay(true))
                {
                    if (w_drop > width - 1)
					{
                        w_drop -= width;
					}

					if (random_numbers != NULL)
					{
						number3 = GetRandom(random_numbers, rand_table_index, debug);
						rand_table_index++;
					} else
					{
						number3 = distribution(generator);
					}

                    if(!matrix(h,w_drop) && number3 > p_ns)//if#7 if there is no sand, probability that slab cannot be settled is 1-p_ns
					{
						w_drop += l_sites;
					}
                    else if (matrix(h,w_drop) > 0 && number3 > p_s) // then ! if there is sand, probability that slab cannot be settled is 1-p_s
					{
                         w_drop+=l_sites;
					}
                    else
                    {
                         matrix[h][w_drop] += z_slab;
                         break;
                    }
                }
                while(Process_Get_Okay(true))
                {
                    angles_cal(matrix, h, w_drop, angles, debug);// ! differences in values of the cell and its 4 neighbours
                    if (!w_drop && !h && angles[0] < -repose)
                    {
                         matrix[h][width] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=width-1;
                    }
                    else if (!w_drop && !h && angles[1] < -repose)
                    {
                         matrix[height][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[1] < -repose)
                    {
                         matrix[height-1][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && !h && angles[2] < -repose)
                    {
                         matrix[h][0] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=0;
                    }
                    else if (w_drop==width-1 && h==height-1 && angles[2] < -repose)
                    {
                         matrix[h][0] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=0;
                    }
                    else if (w_drop==width-1 && h==height-1 && angles[3] < -repose)
                    {
                         matrix[0][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=0;
                    }
                    else if (!w_drop && h==height-1 && angles[0] < -repose)
                    {
                         matrix[h][width-1] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=width-1;
                    }
                    else if (!w_drop && h==height-1 && angles[3] < -repose)
                    {
                         matrix[0][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=0;
                    }
                    else if (!w_drop && angles[0] < -repose)
                    {
                         matrix[h][width-1] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=width-1;
                    }
					else if (!h && angles[1] < -repose) 
                    {
                         matrix[height-1][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=height-1;
                    }
                    else if (w_drop==width-1 && angles[2] < -repose) 
                    {
                         matrix[h][0] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop=0;
                    }
                    else if (h==height-1 && angles[3] < -repose) 
                    {
                         matrix[0][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h=0;
                    }
                    else if (angles[0] < -repose)
                    {
                         matrix[h][w_drop-1] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop--;
                    }
                    else if (angles[1] < -repose)
                    {
                         matrix[h-1][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h--;
                    }
                    else if (angles[2] < -repose)
                    {
                         matrix[h][w_drop+1] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         w_drop++;
                    }
                    else if (angles[3] < -repose)
                    {
                         matrix[h+1][w_drop] += z_slab;
                         matrix[h][w_drop] -= z_slab;
                         h++;
                    }
                    else
					{
                         break;
					}
                }
            }
        }
		
        if(save_outputs && i==count)
        {
			CSG_String outputName = CSG_String::Format(SG_T("%s_%lu"), output->Get_Name(), i);
			CSG_String outputPath = SG_File_Make_Path(outputDir, outputName, CSG_String("tif")); 

			if(!ExportGrid(output, outputPath))
			{
				return false;
			}	

            count+=addcount;
        }
		
		double prog = (i / (double)N_slabs) * 100.0;
		Set_Progress(std::max(prog, 1.0)); // show activity on progress bars
		MatrixToGrid(matrix, output);
		DataObject_Update(output, true);
    }  

	return( true );
}

bool CDunes::ExportGrid(CSG_Grid* grid, CSG_String path)
{
	Message_Add(CSG_String::Format(SG_T("%s: '%s' "), _TL("Saving"), path.c_str()));

	
	// GDAL related options
	TSG_Data_Type type = grid->Get_Type();
	CSG_String driver = CSG_String("GTiff");
	CSG_String options = "";
	CSG_Projection prj; 	Get_Projection(prj);
	CSG_GDAL_DataSet dataset;
	

	if( !dataset.Open_Write(path, driver, options, type, 1, grid->Get_System(), prj) )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to open file for writing: "), path.c_str()));
		return false;
	}
	dataset.Write(0, grid);
	
	if( !dataset.Close() )
	{
		Error_Set(CSG_String::Format(SG_T("%s: '%s' "), _TL("Failed to close file after writing: "), path.c_str()));
		return false;
	}
	
	return true;
}

void CDunes::angles_cal(CSG_Matrix& matrix,int h_r,int w_r,double *angles, bool debug)
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

    int height=matrix.Get_NY();
    int width=matrix.Get_NX();
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

		double vcenter = matrix(h_r,w_r);
		double vleft = matrix(h_r,w_r-1);
		double vtop = matrix(height-1,w_r);
		double vright =  matrix(h_r,w_r+1);
		double vbottom = matrix(h_r+1,w_r);

        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(height-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);
    }
    else if (w_r==width-1) 
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,0)-matrix(h_r,w_r);
        bottom=matrix(h_r+1,w_r)-matrix(h_r,w_r);     
    }
    else if (h_r==height-1)
    {
        left=matrix(h_r,w_r-1)-matrix(h_r,w_r);
        top=matrix(h_r-1,w_r)-matrix(h_r,w_r);
        right=matrix(h_r,w_r+1)-matrix(h_r,w_r);
        bottom=matrix(0,w_r)-matrix(h_r,w_r);
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
	
	if (debug)
	{
		Message_Add(CSG_String::Format("%d %d", h_r+1, w_r+1));
		//Message_Add(CSG_String::Format("%.0f %.0f %.0f %.0f", angles[0], angles[1], angles[2], angles[3]));
	}
}