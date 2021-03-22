/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#ifndef __HoleFillerDefines_h
#define __HoleFillerDefines_h

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

//#include <boost/numeric/ublas/matrix_sparse.hpp>
//#include <boost/numeric/ublas/triangular.hpp>
#include <Eigen/Dense>
#include <Eigen/Sparse>



#include <list>
#include <vector>
#include <set>
#include <complex>

typedef struct __triangle {
    vtkIdType id[3];
} TriangleCellType;

typedef struct __edge {
    vtkIdType v0; //vertices
    vtkIdType v1;
} EdgeType;


typedef std::complex<double> AreaAngleMeasureType;
typedef std::vector<TriangleCellType> HoleCoverType; 
typedef std::vector<HoleCoverType> ArrayOfCoversType;

typedef Eigen::Vector3d VectorType;
//typedef boost::numeric::ublas::triangular_matrix<vtkIdType, boost::numeric::ublas::upper> TriangularIDMatrixType;
typedef Eigen::SparseMatrix<vtkIdType> TriangularIDMatrixType;
typedef Eigen::SparseMatrix<double> SparseDoubleMatrixType;
typedef Eigen::SparseMatrix<vtkIdType> SparseIDMatrixType;


typedef std::vector<EdgeType> HoleBoundaryType;
typedef std::vector<HoleBoundaryType> ArrayOfBoundariesType;

typedef std::vector<vtkIdType> VertexIDArrayType;

#endif