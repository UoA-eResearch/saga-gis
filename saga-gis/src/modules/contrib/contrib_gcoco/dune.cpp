#include <iostream>
#include <fstream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include "array.h"


#define NEIGHBOURS 4

using namespace std;

/*
void usage(const char *name)
{
    printf("Usage: %s filename width height\n",name);
}
*/

void angles_cal(Array& matrix,int h_r,int w_r,double *angles)
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
        
        

bool read_matrix(const char *filename,Array& m)
{
    ifstream ifile;
    int w,h;
    unsigned long cnt,expected;

    ifile.open(filename);
    if(!ifile.is_open())
    {
        printf("Error opening file %s\n",filename);
        return false;
    }
    h=m.dim(0);
    w=m.dim(1);

    expected=(unsigned long)w*(unsigned long)h;
    cnt=0;
  
    for(int i=0;i<h;i++)
    {
        double *d=m.row(i);
        for(int j=0;j<w;j++)
        {
            if(ifile.eof())
                break;
            ifile >> d[j];
            cnt++;
        }
        if(ifile.eof())
            break;
    }
    printf("Read %lu elements from %s\n",cnt,filename);
    if(cnt!=expected)
    {
        printf("Warning: only %lu elements read out of %lu\n",cnt,expected);
    }
    ifile.close();
    return true;
}

bool save_matrix(const char *filename,Array& matrix)
{
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
    return true;
}

void process_matrix(Array& matrix)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    const unsigned long N_slabs=90000000;
    const int l_sites=5;
    const double shadow=1.0;
    const double repose=1.0; 
    const double z_slab=1.0;
    const double p_ns=0.4;
    const double p_s=0.6;
    const int addcount=5000000;
    unsigned long h,w,h_move,w_move,w_drop;
    int width,height;
    double angles[NEIGHBOURS];
    char name[256]="";
    int count=5000000;

    height=matrix.dim(0);
    width=matrix.dim(1);
    for(unsigned long i=0;i<N_slabs;i++)
    {
        double number1=distribution(generator)*height;
        h=floor(number1);
        double number2=distribution(generator)*width;
        w=floor(number2);
        if(!matrix(h,w)) //if#3 no pick-ups when there is no sand
            matrix(h,w)=matrix(h,w); // 
        else if(!w)
        {
            if(matrix(h,width-1)-matrix(h,w)< shadow)
            { 
                matrix(h,w)-=z_slab;
                h_move=h;
                w_move=w;
                while(true)
                {
                    angles_cal(matrix,h_move,w_move,angles);
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
                         break;
                }
                w_drop+=w+l_sites;//  !create new x-dimension of cell (l-number of lattice sites in transport direction)   
                while(true)
                {
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
                while(true)
                {
                    angles_cal(matrix,h,w_drop,angles); // ! differences in values of the cell and its 4 neighbours
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
                while(true)
                {
                    angles_cal(matrix,h_move,w_move,angles);// ! differences in values of the cell and its 4 neighbours
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
                while(true)
                {
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
                while(true)
                {
                    angles_cal(matrix, h, w_drop, angles);// ! differences in values of the cell and its 4 neighbours
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
                         break;
                }
            }
        }
        if(i==count)
        {
            char name[256];
            
            sprintf(name,"output%lu.txt",i);
            save_matrix(name,matrix);
            count+=addcount;
        }
    }  
}

/*
int main(int argc,char *argv[])
{
    Array matrix;
    const char *filename=NULL;
    int width=0,height=0;

    if(argc!=4)
    {
        usage(argv[0]);
        return -1;
    }
    filename=argv[1];
    width=atoi(argv[2]);
    height=atoi(argv[3]);

    if(width<=0 || height<=0)
    {
        printf("Illegal dimensions specified\n");
        return -1;
    }
    matrix.init(2,height,width);
    if(!read_matrix(filename,matrix))
        return -1;
    process_matrix(matrix);
    return 0;
}
*/
