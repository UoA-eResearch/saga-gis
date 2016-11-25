#include <algorithm>
#include <stdarg.h>
#include <stdlib.h>
#include <numeric>
#include <functional>
#include <string.h>
#include "array.h"

/*

Author: Gene Soudlenkov

*/

using namespace std;

Array::Array():data(NULL),size(0)
{
}

Array::Array(const Array& other)
{
    m_Dims.assign(other.m_Dims.begin(),other.m_Dims.end());
    size=std::accumulate(m_Dims.begin(),m_Dims.end(),1,std::multiplies<unsigned long>());
    data=(TYPE *)malloc(size*sizeof(TYPE));
    memcpy(data,other.data,size*sizeof(TYPE));
}

Array::~Array()
{
    if(data)
        free(data);
}


bool Array::init(int dims,...)
{
    va_list arg;
    va_start(arg,dims);

    if(dims<1)
        return false;

    if(data)
        free(data);
    data=NULL;
    m_Dims.clear();

    for(int i=0;i<dims;i++)
        m_Dims.push_back(va_arg(arg,unsigned long));
    size=std::accumulate(m_Dims.begin(),m_Dims.end(),1,std::multiplies<unsigned long>());
    data=(TYPE *)malloc(size*sizeof(TYPE));
    zero();
    va_end(arg);
    return (data!=NULL);
}

Array::TYPE& Array::operator()(int dim1,...)
{
    va_list arg;
    va_start(arg,dim1);
    std::vector<unsigned long> offs;
    unsigned long offset=0;
    
    if(!data || !m_Dims.size())
        throw "Bad array";

    offs.push_back(dim1);
    for(int i=1;i<m_Dims.size();i++)
    {
        offs.push_back(va_arg(arg,unsigned long));
    }

    if(offs.size()!=m_Dims.size())
        throw "Bad dimensions";
   
    for(int i=1;i<m_Dims.size();i++)
    {
        offset+=offs[i-1]*std::accumulate(m_Dims.begin()+i,m_Dims.end(),1,std::multiplies<unsigned long>());
    }
    offset+=offs[offs.size()-1];
    va_end(arg);
    return data[offset]; 
}

Array::TYPE* Array::row(int dim1,...)
{
    va_list arg;
    va_start(arg,dim1);
    std::vector<unsigned long> offs;
    unsigned long offset=0;
    
    if(!data || !m_Dims.size())
        throw "Bad array";
    offs.push_back(dim1);
    for(int i=1;i<m_Dims.size()-1;i++)
    {
        offs.push_back(va_arg(arg,unsigned long));
    }

    if(offs.size()!=m_Dims.size()-1)
        throw "Bad dimensions";
   
    for(int i=1;i<m_Dims.size();i++)
    {
        offset+=offs[i-1]*std::accumulate(m_Dims.begin()+i,m_Dims.end(),1,std::multiplies<unsigned long>());
    }
    va_end(arg);
    return data+offset; 
}
 
Array& Array::zero()
{
    if(data)
    {
        memset(data,0,size*sizeof(TYPE));
    }
    return *this;
}

Array& Array::identity(double val)
{
    if(data && m_Dims.size()==2 && m_Dims[0]==m_Dims[1])
    {
        zero();
        for(int i=0;i<m_Dims[0];i++)
            data[i*m_Dims[0]+i]=val;
    }
    return *this;
}


Array& Array::operator=(const Array& other)
{
    if(this!=&other)
    {
        if(data)
           free(data);
        data=NULL;
        m_Dims.clear();
        m_Dims.assign(other.m_Dims.begin(),other.m_Dims.end());
        size=std::accumulate(m_Dims.begin(),m_Dims.end(),1,std::multiplies<unsigned long>());
        data=(TYPE *)malloc(size*sizeof(TYPE));
        memcpy(data,other.data,size*sizeof(TYPE));
    }
    return *this;
}

Array Array::operator*(Array& other)
{
    Array res;

    if(m_Dims.size()!=2 || other.m_Dims.size()!=2 || m_Dims[1]!=other.m_Dims[0])
    {
        return res;
    }
    res.init(2,m_Dims[0],other.m_Dims[1]);
  
    for(int i=0;i<m_Dims[0];i++)
    {
        for(int j=0;j<other.m_Dims[1];j++)
        {
            TYPE &p=res(i,j);
            p=0.0;
            for(int k=0;k<other.m_Dims[0];k++)
                p+=this->operator()(i,k)*other(k,j);
        }
    }    
    return res;
}

Array& Array::operator*=(Array& other)
{
    Array res;

    if(m_Dims.size()!=2 || other.m_Dims.size()!=2 || m_Dims[1]!=other.m_Dims[0])
    {
        return *this;
    }
    res.init(2,m_Dims[0],other.m_Dims[1]);
  
    for(int i=0;i<m_Dims[0];i++)
    {
        for(int j=0;j<other.m_Dims[1];j++)
        {
            TYPE &p=res(i,j);
            p=0.0;
            for(int k=0;k<other.m_Dims[0];k++)
                p+=this->operator()(i,k)*other(k,j);
        }
    }    
    this->operator=(res);
    return *this;
}

Array Array::operator*(double v)
{
    Array res;

    if(!data)
        return res;
    res=*this;
    for(int i=0;i<size;i++)
        res.data[i]*=v;
    return res;
}

Array& Array::operator*=(double v)
{
    if(!data)
        return *this;
    for(int i=0;i<size;i++)
        data[i]*=v;
    return *this;
}

Array Array::operator+(Array& other)
{
    Array res;

    if(size!=other.size)
        return res;
    res=*this;
    for(int i=0;i<size;i++)
        res.data[i]+=other.data[i];
    return res;
}

Array& Array::operator+=(Array& other)
{
    if(size!=other.size)
        return *this;
    for(int i=0;i<size;i++)
        data[i]+=other.data[i];
    return *this;
}

Array Array::operator-(Array& other)
{
    Array res;

    if(size!=other.size)
        return res;
    res=*this;
    for(int i=0;i<size;i++)
        res.data[i]-=other.data[i];
    return res;
}

Array& Array::operator-=(Array& other)
{
    if(size!=other.size)
        return *this;
    for(int i=0;i<size;i++)
        data[i]-=other.data[i];
    return *this;
}

Array& Array::transpose()
{
    unsigned long start, next, i;
    TYPE tmp;
    unsigned long d0,d1;
               
    if(m_Dims.size()!=2)
        return *this;
    d0=m_Dims[0];
    d1=m_Dims[1];
    for(start = 0; start <= d0*d1-1; start++)
    {
        next = start;
        i = 0;
        do
        {    
            i++;
            next = (next % d0) * d1 + next / d0;
        } while (next > start);

        if(next < start || i == 1)
            continue;
        tmp = data[next = start];
        do
        {
            i = (next % d0) * d1 + next / d0;
            data[next] = (i == start) ? tmp : data[i];
            next = i;
        }while (next > start);
     }
     m_Dims[0]=d1;
     m_Dims[1]=d0;
     return *this;
}


