/*
 *  Vector3.h
 *
 *
 *  Created by Thomas Fritz on 24.08.09.
 *  Copyright 2009 Institute for Biomedical Engineering, Karlsruhe Institute of Technology (KIT). All rights reserved.
 *
 */
#ifndef MATHPACK_VECTOR3_H
#define MATHPACK_VECTOR3_H

#include <cmath>
#include <iomanip>
#include <iostream>
#include <initializer_list>
#include <cassert>
#include "Matrix3.h"


// forward declaration of Matrix3

namespace math_pack
{

template<typename T> class Matrix3;

template<typename T>
class Vector3
{
 
public:
    friend class Matrix3<T>;
    Vector3(T x, T y, T z)
    {
        elements_[0] = x;
        elements_[1] = y;
        elements_[2] = z;
    }
    
    Vector3(const T* const array)
    {
        elements_[0] = array[0];
        elements_[1] = array[1];
        elements_[2] = array[2]; 
    }
    
    Vector3(const Vector3& v)
    {
        elements_[0] = v.elements_[0];
        elements_[1] = v.elements_[1];
        elements_[2] = v.elements_[2];
    }
    
    Vector3(std::initializer_list<T> l)
    {
#ifndef NDEBUG
        if(l.size() != 3) throw std::runtime_error("Vector3<T>::Vector3(std::initializer_list l) initializer list has to exactly contain 3 elements ");
#endif
        T* e = elements_;
        for(auto i:l)
        {
            *e = i;
            ++e;
        }
    }
    Vector3(){ for(int i=0; i<3; i++) elements_[i]=0; }

    inline T* GetArray()
    {
        return(elements_);
    }

    inline const T* GetArray() const
    {
        return(elements_);
    }
    
    inline T& operator()(unsigned short i)
    {
#ifndef NDEBUG
        if(i > 2) throw std::runtime_error("Vector3<T>::GetElement(unsigned short i) i out of range");
#endif
        return((T&)elements_[i]);
    }
    
    inline const T& operator()(unsigned short i) const
    {
#ifndef NDEBUG
        if(i > 2) throw std::runtime_error("Vector3<T>::GetElement(unsigned short i) i out of range");
#endif
        return((T&)elements_[i]);
    }

    inline T& Set(unsigned short i)
    {
#ifndef NDEBUG
        if(i > 2) throw std::runtime_error("Vector3<T>::GetElement(unsigned short i) i out of range");
#endif
        return((T&)elements_[i]);
    }

    inline T Get(unsigned short i) const
    {
#ifndef NDEBUG
        if(i > 2) throw std::runtime_error("Vector3<T>::GetElement(unsigned short i) i out of range");
#endif
        return(elements_[i]);
    }

    inline T& X()
    {
        return((T&)elements_[0]);
    }

    inline T& Y()
    {
        return((T&)elements_[1]);
    }

    inline T& Z()
    {
        return((T&)elements_[2]);
    }
    
    inline const T& X() const
    {
        return((T&)elements_[0]);
    }
    
    inline const T& Y() const
    {
        return((T&)elements_[1]);
    }
    
    inline const T& Z() const
    {
        return((T&)elements_[2]);
    }

    
    inline void SetArray(const T* const array){elements_[0] = array[0];
                                               elements_[1] = array[1];
                                               elements_[2] = array[2]; }

    // ------------------- Operator ----------------------

    inline bool operator==(const Vector3<T>& a) const;

    inline T operator*(const Vector3<T>& a) const;
    inline Vector3<T> operator+(const Vector3<T>& a) const;
    inline Vector3<T> operator-(const Vector3<T>& a) const;

    inline Vector3<T> operator*(const T& b) const;
    inline Vector3<T> operator/(const T& b) const;

    inline void operator*=(const T& b);
    inline void operator/=(const T& b);
    inline void operator+=(const Vector3<T>& a);
    inline void operator-=(const Vector3<T>& a);

    // ----------------------------------------------------

    friend std::ostream& operator<<(std::ostream& str,const Vector3<T>& a)
    {
        str << a.elements_[0] << "\n" << a.elements_[1] << "\n" << a.elements_[2] << "\n";
        return(str);
    }
    
    void Print() const;
    inline T Norm() const;
    inline bool Normalize();

    std::ostream& Print(std::ostream& outputStream) const;

    template<class C> Vector3<C> GetConvertedVector(){Vector3<C> vec((C)elements_[0], (C)elements_[1], (C)elements_[2]);
                                                      return(vec); }
protected:
private:
    T elements_[3];
};

template<typename T>
inline bool Vector3<T>::operator==(const Vector3<T>& b) const
{
    if(elements_[0] ==  b.elements_[0] && elements_[1] ==  b.elements_[1] && elements_[2] ==  b.elements_[2])
        return(true);
    else
        return(false);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator+(const Vector3<T>& b) const
{
    return(Vector3<T>(elements_[0] + b.elements_[0], elements_[1] + b.elements_[1], elements_[2] + b.elements_[2] ));
}

template<typename T>
inline Vector3<T> Vector3<T>::operator-(const Vector3<T>& b) const
{
    return(Vector3<T>(elements_[0] - b.elements_[0], elements_[1] - b.elements_[1], elements_[2] - b.elements_[2] ));
}

template<typename T>
inline T Vector3<T>::operator*(const Vector3<T>& b) const
{
    return(elements_[0]*b.elements_[0] + elements_[1]*b.elements_[1] + elements_[2]*b.elements_[2]);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator*(const T& b) const
{
    return(Vector3<T>(elements_[0]*b, elements_[1]*b, elements_[2]*b));
}

template<typename T>
inline Vector3<T> operator*(const T& b, const Vector3<T>& a)
{
    return(a*b);
}

template<typename T>
inline Vector3<T> Vector3<T>::operator/(const T& b) const
{
    return(Vector3<T>(elements_[0]/b, elements_[1]/b, elements_[2]/b));
}

template<typename T>
inline Vector3<T> operator/(const T& b, const Vector3<T>& a) 
{
    return(a/b);
}

template<typename T>
inline Vector3<T> CrossProduct(const Vector3<T>& a, const Vector3<T>& b) 
{
    return ( Vector3<T>( a(1) * b(2) - a(2) * b(1),
                         a(2) * b(0) - a(0) * b(2),
                         a(0) * b(1) - a(1) * b(0)
                       ));

}

template<typename T>
inline void Vector3<T>::operator*=(const T& b)
{
    elements_[0] *= b;
    elements_[1] *= b;
    elements_[2] *= b;
}

template<typename T>
inline void Vector3<T>::operator/=(const T& b)
{
    elements_[0] /= b;
    elements_[1] /= b;
    elements_[2] /= b;
}


template<typename T>
inline void Vector3<T>::operator+=(const Vector3<T>& b)
{
    elements_[0] +=  b.elements_[0];
    elements_[1] +=  b.elements_[1];
    elements_[2] +=  b.elements_[2];
}

template<typename T>
inline void Vector3<T>::operator-=(const Vector3<T>& b)
{
    elements_[0] -=  b.elements_[0];
    elements_[1] -=  b.elements_[1];
    elements_[2] -=  b.elements_[2];
}


template<typename T>
inline T Vector3<T>::Norm() const
{
    return(sqrt(elements_[0]*elements_[0] + elements_[1]*elements_[1] + elements_[2]*elements_[2]));
}

template<typename T>
inline bool Vector3<T>::Normalize()
{
    T norm = sqrt(elements_[0]*elements_[0] + elements_[1]*elements_[1] + elements_[2]*elements_[2]);
    if(norm == T(0.0))
    {
#ifndef NDEBUG
        throw std::runtime_error("Vector3<T><T>::Normalize() Division by Zero");
#endif
        return(false);
    }

    elements_[0] /= norm;
    elements_[1] /= norm;
    elements_[2] /= norm;
    return(true);
}

template<typename T>
inline std::ostream & Vector3<T>::Print(std::ostream& outputStream) const
{
    return(outputStream << std::scientific << elements_[0] << " " << elements_[1] << " " << elements_[2]);
}
    
template<typename T>
void Vector3<T>::Print() const
{
    std::cout << *this;
}
 
    
 
}
#endif
