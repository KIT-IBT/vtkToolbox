/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
//patch refiner


#ifndef __CoverRefiner_h
#define __CoverRefiner_h

#include <vtkPoints.h>
#include <vtkSmartPointer.h>

#include "HoleFillerDefines.h"

//we will work with a sparse matrix, so this number is irrelevant
//just to avoid calling resize every time a vertex is added to the refinement
//if it is exceeded, the resize will be called
//#define MAX_NUMBER_OF_VERTICES 100000

//typedef Eigen::MatrixXi ConnectivityMatrixType;
typedef Eigen::SparseMatrix<int> ConnectivityMatrixType;


class CoverRefiner //inplace refinement
{
public:
    void SetInputFaces(HoleCoverType *coverFaces) {m_coverFaces = coverFaces;};
    void SetInputVertices(vtkPoints* coverVertices) {m_coverVertices = coverVertices; };
    void SetInputBoundaryIds( const VertexIDArrayType *boundaryIds ) {m_boundaryIds=boundaryIds;};
    
    //this has to be called from exterior
    void InitializeVertexWeights( vtkPolyData* mesh, const VertexIDArrayType *originalBoundaryIds );
    
    void Update();
    void InitializeConnectivityMatrix(ConnectivityMatrixType& conn);

        
    CoverRefiner():m_coverFaces(NULL), m_boundaryIds(NULL) {};
protected:
    
    void GetVertexNeighbors(vtkPolyData *mesh, vtkIdType vertexId, std::set<vtkIdType>& connectedVertices);
    bool RelaxAllCoverEdges(ConnectivityMatrixType& conn);    

    bool FindConnectedVertices(HoleCoverType *coverFaces, const EdgeType& edge, EdgeType& intersectingEdge);
    
    bool RelaxEdgeIfPossible(HoleCoverType *coverFaces, const EdgeType& edge, const EdgeType& candidateEdge, ConnectivityMatrixType& conn);

    bool CheckForDuplicateTriangles();
    bool IsTriangleSplitRequired(const vtkIdType idVi, const vtkIdType idVj, const vtkIdType idVk, VectorType& Vc, double& Svc);
    
    //check if ptcheck is inside a circle defined by the 3 points
    bool IsPointInCircle(const VectorType& pt0, const VectorType& pt1, const VectorType& pt2, const VectorType& ptcheck) const;
    vtkIdType FindTriangleByPointIds( vtkIdType id0, vtkIdType id1, vtkIdType id2);     
    
    void SaveIsolatedCover(const char* filename);


private:
    HoleCoverType *m_coverFaces;
    const VertexIDArrayType *m_boundaryIds;
    vtkPoints* m_coverVertices;    

    std::vector<double> m_sigmas;
};


#endif
