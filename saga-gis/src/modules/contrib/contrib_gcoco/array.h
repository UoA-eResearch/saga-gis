#ifndef ARRAY_H
#define ARRAY_H
#include <vector>

class Array
{
   public:
      typedef double TYPE;

      Array();
      Array(const Array& other);
      virtual ~Array();
      virtual bool init(int dims,...);
      int dims() const { return m_Dims.size(); }
      int dim(int i) const { return m_Dims[i]; }
      TYPE& operator()(int dim1,...);
      TYPE *row(int dim1,...);
     
      Array& identity(double val=1.0);
      Array& zero();
      Array& operator=(const Array& other);
      Array operator*(Array& other);
      Array& operator*=(Array& other);
      Array operator*(double v);
      Array& operator*=(double v);
      Array operator+(Array& other);
      Array& operator+=(Array& other);
      Array operator-(Array& other);
      Array& operator-=(Array& other);
      Array& transpose();
 

  protected:
      std::vector<unsigned long> m_Dims;
      TYPE *data; 
      unsigned long size;
};

#endif

