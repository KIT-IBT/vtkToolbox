/*
 *  Matrix3.h
 *
 *  Created by Thomas Fritz on 24.08.09.
 *  Copyright 2009 Institute for Biomedical Engineering, Karlsruhe Institute of Technology (KIT). All rights reserved.
 *
 */
#ifndef MATHPACK_MATRIX3_H
#define MATHPACK_MATRIX3_H

#include <cmath>
#include <iomanip>
#include <iostream>
#include <cstring>

#include "Vector3.h"

// forward declaration of Matrix3


namespace math_pack
{
    
template<typename T> class Vector3;

template<typename T>
class Matrix3

{
public:

    friend class Vector3<T>;
    
    
    
    Matrix3(T e0, T e1, T e2, T e3, T e4, T e5, T e6, T e7, T e8)
    {
        elements_[0] = e0;
        elements_[1] = e1;
        elements_[2] = e2;
        elements_[3] = e3;
        elements_[4] = e4;
        elements_[5] = e5;
        elements_[6] = e6;
        elements_[7] = e7;
        elements_[8] = e8;
    }

    Matrix3(const Vector3<T>&a,const Vector3<T>&b,const Vector3<T>&c)
    {
        elements_[0] = a(0);
        elements_[1] = b(0);
        elements_[2] = c(0);
        
        elements_[3] = a(1);
        elements_[4] = b(1);
        elements_[5] = c(1);

        elements_[6] = a(2);
        elements_[7] = b(2);
        elements_[8] = c(2);
    }


    Matrix3(){for(int i=0; i<9; i++) elements_[i] = 0;}
    Matrix3(const Matrix3<T>& m) = default;
    
    Matrix3(const T* array)
    {
        memcpy(elements_,array,9*sizeof(T));
    }
    
    Matrix3(std::initializer_list<T> l)
    {
#ifndef NDEBUG
        if(l.size() != 9) throw std::runtime_error("Matrix3<T>::Matrix3(std::initializer_list l) initializer list has to exactly contain 9 elements ");
#endif
        T* e = elements_;
        for(auto i:l)
        {
            *e = i;
            ++e;
        }
    }
    
    inline T& operator()(unsigned short i, unsigned short j)
    {
#ifndef NDEBUG
        if(i > 2 || j > 2) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i,unsigned short j) i or j out of range");
#endif
        return((T&)elements_[3*i+j]);
    }
    
    inline const T& operator()(unsigned short i, unsigned short j) const
    {
#ifndef NDEBUG
        if(i > 2 || j > 2) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i,unsigned short j) i or j out of range");
#endif
        return((T&)elements_[3*i+j]);
    }

    inline T& operator()(unsigned short i)
    {
        #ifndef NDEBUG
        if(i < 0 || i > 8) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i) i out of range");
        #endif
        return((T&)elements_[i]);
    }
    
    inline const T& operator()(unsigned short i) const
    {
#ifndef NDEBUG
        if(i < 0 || i > 8) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i) i out of range");
#endif
        return((T&)elements_[i]);
    }

    inline T& Set(unsigned short i, unsigned short j)
    {
#ifndef NDEBUG
        if(i > 2 || j > 2) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i,unsigned short j) i or j out of range");
#endif
        return((T&)elements_[3*i+j]);
    }

    inline T& Set(unsigned short i)
    {
#ifndef NDEBUG
        if(i < 0 || i > 8) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i) i out of range");
#endif
        return((T&)elements_[i]);
    }

    inline T Get(unsigned short i) const
    {
#ifndef NDEBUG
        if(i < 0 || i > 8) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i) i out of range");
#endif
        return(elements_[i]);
    }

    inline T Get(unsigned short i, unsigned short j) const
    {
#ifndef NDEBUG
        if(i > 2 ||  j > 2) throw std::runtime_error("Matrix3<T>::GetElement(unsigned short i,unsigned short j) i or j out of range");
#endif
        return(elements_[3*i+j]);
    }


    inline T* GetArray(){return(elements_); }
    inline const T* GetArray() const {return(elements_); } 
    
    inline void SetArray(T* array)
    {
        elements_[0] = array[0];
        elements_[1] = array[1];
        elements_[2] = array[2];
        elements_[3] = array[3];
        elements_[4] = array[4];
        elements_[5] = array[5];
        elements_[6] = array[6];
        elements_[7] = array[7];
        elements_[8] = array[8];
    }

    // ------------------- Operator -------------------------------

    inline bool operator==(const Matrix3<T>& a) const;
    inline Matrix3<T> operator*(const Matrix3<T>& a) const;
    inline Matrix3<T> operator+(const Matrix3<T>& a) const;
    inline Matrix3<T> operator-(const Matrix3<T>& a) const;

    inline void operator*=(const Matrix3<T>& a);
    inline void operator+=(const Matrix3<T>& a);
    inline void operator-=(const Matrix3<T>& a);

    inline Matrix3<T> operator*(const T& a) const;
    inline Matrix3<T> operator/(const T& a) const;

    inline Vector3<T> operator*(const Vector3<T>& b) const;

    inline void operator*=(const T& a);
    inline void operator/=(const T& a);

    // -------------------------------------------------------------

    inline Vector3<T> GetRow(unsigned short i) const;
    inline Vector3<T> GetCol(unsigned short i) const;
    inline void SetRow(unsigned short i, Vector3<T> b);
    inline void SetCol(unsigned short i, Vector3<T> b);

    inline bool Invert();
    inline Matrix3<T> GetInverse() const;
    inline void Transpose();
    inline Matrix3<T> GetTranspose() const;

    inline void SetToZero();
    inline void SetToIdentityMatrix();
    inline T Det() const;
    inline T Trace() const {return(elements_[0] + elements_[4] + elements_[8]); }
    inline T Invariant1(){return(Trace()); }
    inline T Invariant2(){return(0.5 * ( Trace()*Trace() - (*this * *this).Trace())); }
    inline T Invariant3() const {return(Det());}
    friend std::ostream& operator<<(std::ostream& str,const Matrix3<T>& a)
    {
        str << a.elements_[0] << " " << a.elements_[1] << " " << a.elements_[2] << "\n"
            << a.elements_[3] << " " << a.elements_[4] << " " << a.elements_[5] << "\n"
            << a.elements_[6] << " " << a.elements_[7] << " " << a.elements_[8] << "\n";
        return(str);
    }
    
    static Matrix3<T> Identity()
    {
        return Matrix3<T>(1,0,0,0,1,0,0,0,1);
    }
    
    void Print() const;
    
    template<class C> 
    Matrix3<C> GetConvertedMatrix() const 
    {
        Matrix3<C> mat;
        for(unsigned int i = 0; i < 3; i++)
            for(unsigned int j = 0; j < 3; j++)
                mat(i,j) = static_cast<C>(Get(i,j));
        return(mat); 
    }
    
protected:
private:
    T elements_[9];
};

template<typename T>
inline bool Matrix3<T>::operator==(const Matrix3<T>& a) const
{
    for(int i = 0; i < 9; i++)
    {
        if(elements_[i] != a.elements_[i])
        {
            return(false);
        }
    }
    return(true);
}


template<typename T>
inline Matrix3<T> Matrix3<T>::operator+(const Matrix3<T>& b) const
{
    return(Matrix3<T>(elements_[0] + b.elements_[0],
                      elements_[1] + b.elements_[1],
                      elements_[2] + b.elements_[2],
                      
                      elements_[3] + b.elements_[3],
                      elements_[4] + b.elements_[4],
                      elements_[5] + b.elements_[5],
                      
                      elements_[6] + b.elements_[6],
                      elements_[7] + b.elements_[7],
                      elements_[8] + b.elements_[8]
                      ));
}

template<typename T>
inline Matrix3<T> Matrix3<T>::operator-(const Matrix3<T>& b) const
{
    return(Matrix3<T>(elements_[0] - b.elements_[0],
                      elements_[1] - b.elements_[1],
                      elements_[2] - b.elements_[2],
                      
                      elements_[3] - b.elements_[3],
                      elements_[4] - b.elements_[4],
                      elements_[5] - b.elements_[5],
                      
                      elements_[6] - b.elements_[6],
                      elements_[7] - b.elements_[7],
                      elements_[8] - b.elements_[8]
                      ));
}

template<typename T>
inline Matrix3<T> Matrix3<T>::operator*(const Matrix3<T>& b) const
{
    return(Matrix3<T>(elements_[0]*b.elements_[0] + elements_[1]*b.elements_[3] + elements_[2]*b.elements_[6],
                      elements_[0]*b.elements_[1] + elements_[1]*b.elements_[4] + elements_[2]*b.elements_[7],
                      elements_[0]*b.elements_[2] + elements_[1]*b.elements_[5] + elements_[2]*b.elements_[8],
                          
                      elements_[3]*b.elements_[0] + elements_[4]*b.elements_[3] + elements_[5]*b.elements_[6],
                      elements_[3]*b.elements_[1] + elements_[4]*b.elements_[4] + elements_[5]*b.elements_[7],
                      elements_[3]*b.elements_[2] + elements_[4]*b.elements_[5] + elements_[5]*b.elements_[8],
                          
                      elements_[6]*b.elements_[0] + elements_[7]*b.elements_[3] + elements_[8]*b.elements_[6],
                      elements_[6]*b.elements_[1] + elements_[7]*b.elements_[4] + elements_[8]*b.elements_[7],
                      elements_[6]*b.elements_[2] + elements_[7]*b.elements_[5] + elements_[8]*b.elements_[8]
                      ));
}



template<typename T>
inline Vector3<T>  Matrix3<T>::operator*(const Vector3<T>& b) const
{
   return(Vector3<T>(elements_[0] * b.elements_[0] + elements_[1] * b.elements_[1] +  elements_[2] * b.elements_[2],
                     elements_[3] * b.elements_[0] + elements_[4] * b.elements_[1] +  elements_[5] * b.elements_[2],
                     elements_[6] * b.elements_[0] + elements_[7] * b.elements_[1] +  elements_[8] * b.elements_[2]
                     ));
}
    
template<typename T>
inline void Matrix3<T>::operator+=(const Matrix3<T>& b)
{
    elements_[0] +=  b.elements_[0];
    elements_[1] +=  b.elements_[1];
    elements_[2] +=  b.elements_[2];

    elements_[3] +=  b.elements_[3];
    elements_[4] +=  b.elements_[4];
    elements_[5] +=  b.elements_[5];

    elements_[6] +=  b.elements_[6];
    elements_[7] +=  b.elements_[7];
    elements_[8] +=  b.elements_[8];
}

template<typename T>
inline void Matrix3<T>::operator-=(const Matrix3<T>& b)
{
    elements_[0] -=  b.elements_[0];
    elements_[1] -=  b.elements_[1];
    elements_[2] -=  b.elements_[2];

    elements_[3] -=  b.elements_[3];
    elements_[4] -=  b.elements_[4];
    elements_[5] -=  b.elements_[5];

    elements_[6] -=  b.elements_[6];
    elements_[7] -=  b.elements_[7];
    elements_[8] -=  b.elements_[8];
}

template<typename T>
inline void Matrix3<T>::operator*=(const Matrix3<T>& b)
{
    *this *= b;
}


template<typename T>
inline Matrix3<T> Matrix3<T>::operator*(const T& b) const
{
    return(Matrix3<T>(elements_[0] * b,
                      elements_[1] * b,
                      elements_[2] * b,

                      elements_[3] * b,
                      elements_[4] * b,
                      elements_[5] * b,

                      elements_[6] * b,
                      elements_[7] * b,
                      elements_[8] * b
                    ));
}

template<typename T>
inline Matrix3<T> Matrix3<T>::operator/(const T& b) const
{
    return(Matrix3<T>(elements_[0] / b,
                      elements_[1] / b,
                      elements_[2] / b,
                     
                      elements_[3] / b,
                      elements_[4] / b,
                      elements_[5] / b,
                     
                      elements_[6] / b,
                      elements_[7] / b,
                      elements_[8] / b
                     ));
}

template<typename T>
inline Matrix3<T> operator*(const T& b, Matrix3<T> a)
{
    return(a*b);
}
    
template<typename T>
inline Matrix3<T> operator/(const T& b, Matrix3<T> a)
{
    return(a/b);
}

template<typename T>
inline void Matrix3<T>::operator*=(const T& b)
{
    elements_[0] *= b;
    elements_[1] *= b;
    elements_[2] *= b;

    elements_[3] *= b;
    elements_[4] *= b;
    elements_[5] *= b;

    elements_[6] *= b;
    elements_[7] *= b;
    elements_[8] *= b;
}

template<typename T>
inline void Matrix3<T>::operator/=(const T& b)
{
    elements_[0] /= b;
    elements_[1] /= b;
    elements_[2] /= b;

    elements_[3] /= b;
    elements_[4] /= b;
    elements_[5] /= b;

    elements_[6] /= b;
    elements_[7] /= b;
    elements_[8] /= b;
}


template<typename T>
inline Vector3<T> Matrix3<T>::GetRow(unsigned short i) const
{
#ifndef NDEBUG
    if(i > 2)
        throw std::runtime_error("Matrix3::GetRow(unsigned short i) i out of range");
#endif
    return(Vector3<T>(elements_[3*i], elements_[3*i+1], elements_[3*i+2]));
}

template<typename T>
inline Vector3<T> Matrix3<T>::GetCol(unsigned short i) const
{
#ifndef NDEBUG
    if(i > 2)
        throw std::runtime_error("Matrix3<T>::GetCol(unsigned short i) i out of range");
#endif
    return(Vector3<T>(elements_[i], elements_[i+3], elements_[i+6]));
}
template<typename T>
inline void Matrix3<T>::SetRow(unsigned short i, Vector3<T> b)
{
#ifndef NDEBUG
    if(i < 0 || i > 2)
        throw std::runtime_error("Matrix3<T>::SetRow(unsigned short i) i out of range");
#endif
    elements_[3*i]   = b.elements_[0];
    elements_[3*i+1] = b.elements_[1];
    elements_[3*i+2] = b.elements_[2];
}


template<typename T>
inline void Matrix3<T>::SetCol(unsigned short i, Vector3<T> b)
{
#ifndef NDEBUG
    if(i < 0 || i > 2)
        throw std::runtime_error("Matrix3<T>::SetCol(unsigned short i) i out of range");
#endif
    elements_[i]   = b.elements_[0];
    elements_[i+3] = b.elements_[1];
    elements_[i+6] = b.elements_[2];
}



template<typename T>
inline T Matrix3<T>::Det() const
{
    return((elements_[0]*elements_[4]*elements_[8])
          +(elements_[3]*elements_[7]*elements_[2])
          +(elements_[6]*elements_[1]*elements_[5])
          -(elements_[2]*elements_[4]*elements_[6])
          -(elements_[5]*elements_[7]*elements_[0])
          -(elements_[8]*elements_[3]*elements_[1])
           );
}


template<typename T>
inline bool Matrix3<T>::Invert()
{
    T det = Det();
    if(det == 0)
    {
        *this = Matrix3<T>(INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY);
        return false;
    }

    Matrix3<T> a;
    T*         val = a.GetArray();

    val[0] = (-elements_[5]*elements_[7] + elements_[4]*elements_[8])/det;
    val[1] = ( elements_[2]*elements_[7] - elements_[1]*elements_[8])/det;
    val[2] = (-elements_[2]*elements_[4] + elements_[1]*elements_[5])/det;
    val[3] = ( elements_[5]*elements_[6] - elements_[3]*elements_[8])/det;
    val[4] = (-elements_[2]*elements_[6] + elements_[0]*elements_[8])/det;
    val[5] = ( elements_[2]*elements_[3] - elements_[0]*elements_[5])/det;
    val[6] = (-elements_[4]*elements_[6] + elements_[3]*elements_[7])/det;
    val[7] = ( elements_[1]*elements_[6] - elements_[0]*elements_[7])/det;
    val[8] = (-elements_[1]*elements_[3] + elements_[0]*elements_[4])/det;

    memcpy(elements_, val, 9*sizeof(T));

    return(true);
}

template<typename T>
inline Matrix3<T> Matrix3<T>::GetInverse() const
{
    T det = Det();

    if(det == 0)
        return(Matrix3<T>(INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY,INFINITY));
    else
        return(Matrix3<T>((-elements_[5]*elements_[7] + elements_[4]*elements_[8])/det,
                          ( elements_[2]*elements_[7] - elements_[1]*elements_[8])/det,
                          (-elements_[2]*elements_[4] + elements_[1]*elements_[5])/det,
                          ( elements_[5]*elements_[6] - elements_[3]*elements_[8])/det,
                          (-elements_[2]*elements_[6] + elements_[0]*elements_[8])/det,
                          ( elements_[2]*elements_[3] - elements_[0]*elements_[5])/det,
                          (-elements_[4]*elements_[6] + elements_[3]*elements_[7])/det,
                          ( elements_[1]*elements_[6] - elements_[0]*elements_[7])/det,
                          (-elements_[1]*elements_[3] + elements_[0]*elements_[4])/det
                          ));
}

template<typename T>
inline void Matrix3<T>::Transpose()
{
    T temp;

    temp         = elements_[3];
    elements_[3] = elements_[1];
    elements_[1] = temp;

    temp         = elements_[6];
    elements_[6] = elements_[2];
    elements_[2] = temp;

    temp         = elements_[5];
    elements_[5] = elements_[7];
    elements_[7] = temp;
}

template<typename T>
inline Matrix3<T> Matrix3<T>::GetTranspose() const
{
    return(Matrix3<double>(elements_[0], elements_[3], elements_[6],
                           elements_[1], elements_[4], elements_[7],
                           elements_[2], elements_[5], elements_[8]));
}

template<typename T>
inline void Matrix3<T>::SetToZero()
{
    elements_[0] = 0; elements_[1] = 0; elements_[2] = 0;
    elements_[3] = 0; elements_[4] = 0; elements_[5] = 0;
    elements_[6] = 0; elements_[7] = 0; elements_[8] = 0;
}

template<typename T>
inline void Matrix3<T>::SetToIdentityMatrix()
{
    elements_[0] = 1; elements_[1] = 0; elements_[2] = 0;
    elements_[3] = 0; elements_[4] = 1; elements_[5] = 0;
    elements_[6] = 0; elements_[7] = 0; elements_[8] = 1;
}
    


template<typename T>
inline Matrix3<T> DyadicProduct(const Vector3<T>& a,const Vector3<T>& b)
{
    return(Matrix3<T>(a(0)*b(0), a(0)*b(1), a(0)*b(2),
                      a(1)*b(0), a(1)*b(1), a(1)*b(2),
                      a(2)*b(0), a(2)*b(1), a(2)*b(2)
                      ));
}                 

template<typename T>
inline T DoubleDotProduct(const Matrix3<T>& a,const Matrix3<T>& b)
{
    return(a(0,0) * b(0,0) + a(0,1)*b(0,1) + a(0,2)*b(0,2)+
           a(1,0) * b(1,0) + a(1,1)*b(1,1) + a(1,2)*b(1,2)+
           a(2,0) * b(2,0) + a(2,1)*b(2,1) + a(2,2)*b(2,2));
}
    
template<typename T>
void Matrix3<T>::Print() const
{
    std::cout << *this;
}

}
#endif

