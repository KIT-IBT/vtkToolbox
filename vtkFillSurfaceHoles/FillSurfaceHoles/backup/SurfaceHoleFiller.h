/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
//Remesh-like hole filling
//based on Eurographics Symposium on Geometry Processing(2003), Filling Holes in Meshes, Peter Liepa
//and Filling Gaps in the Boundary of a Polyhedron, Gill Barequet, Micha Sharir

#ifndef __SurfaceHoleFiller_h
#define __SurfaceHoleFiller_h

#include "HoleFillerDefines.h"
#include "UmbrellaWeightedOrder2Smoother.h"


class SurfaceHoleFiller
{
public:
    void SetInput(vtkPolyData* mesh) { m_inputMesh = mesh; };

    void SmoothingOn() {m_performCoverSMoothing=true;};
    void SmoothingOff() {m_performCoverSMoothing=false;};
    bool GetSmoothing(){return m_performCoverSMoothing;};
    void SetSmoothing(bool v){ m_performCoverSMoothing = v;};
    
    void Update();


    int GetEdgeWeightingType() const {return m_weightingType; };
    void SetEdgeWeightingType(int wt) {m_weightingType=wt; };
    void EdgeWeightingTypeCotangent() {m_weightingType=EDGE_WEIGHT_COTANGENT; };
    void EdgeWeightingTypeInvEdgeLength() {m_weightingType=EDGE_WEIGHT_InvEdgeLength; };

    
    
    vtkPolyData* GetOutput() {return m_outputMesh; };
    
    SurfaceHoleFiller():m_performCoverSMoothing(true), 
            m_weightingType(EDGE_WEIGHT_COTANGENT){};
protected:
    
private:
    bool m_performCoverSMoothing;    
   
    int m_weightingType;
    
    vtkSmartPointer<vtkPolyData> m_inputMesh;
    vtkSmartPointer<vtkPolyData> m_outputMesh;
    
    //returns the unordered boundary of all the holes
    void FindHoles(vtkPolyData *mesh, HoleBoundaryType& unordered_edges) const;
    void SplitHoles(HoleBoundaryType& unordered_edges, ArrayOfBoundariesType& hole_boundaries) const;

    //the last vertex is equal to the first vertex to simulate closed loop.
    //so the number of returned vertices is actually the number of edges +1
    void EdgesToVertexArray(const HoleBoundaryType& ordered_boundary, VertexIDArrayType& vertices) const;


    void InitialCoverTriangulation(vtkPolyData* mesh, HoleBoundaryType& ordered_boundary, HoleCoverType& cover) const;

    //for explanation of O, see the paper. This is recursive function Trace
    void PopulateCover(HoleCoverType& cover, vtkIdType i, vtkIdType k, const TriangularIDMatrixType& O, const VertexIDArrayType& vertices) const;

    double TriangleWeightFunctionArea(const VectorType& u, const VectorType& v, const VectorType& w) const;


    //calculates cosine of the dihedral angle. Vertices must have correct orientation
    //v1, v2, v3 - triangle 1
    //u1, u2, u3 - triangle 2
    double CalculateDihedralAngleCos(const VectorType& v1, const VectorType& v2, const VectorType& v3,
            const VectorType& u1, const VectorType& u2, const VectorType& u3) const;



    vtkIdType FindThirdVertexId(vtkPolyData* mesh, vtkIdType p1, vtkIdType p2) const;


    //does m1+m2, as described in the paper
    //(a,b)+(c,d) = (max(a,c),b+d)
    AreaAngleMeasureType SumAreaTriangleMeasures(const AreaAngleMeasureType& m1,
            const AreaAngleMeasureType& m2) const;

    //checks if m1 < m2, lexicographically
    //(a,b)<(c,d) iff (a<c or (a==c and b<d))
    bool AreaTriangleMeasureLess(const AreaAngleMeasureType& m1,
            const AreaAngleMeasureType& m2) const;

    //modifies the mesh
    void RefineCover(vtkPolyData* mesh, const HoleBoundaryType& ordered_boundary, const HoleCoverType& cover) const;
    void IsolateCover(const HoleCoverType& cover, VertexIDArrayType& boundaryVertexIDs, HoleCoverType& localCover) const;


    
    //returns also Vc - centroid, and Svc - centroid's weight
    bool IsTriangleSplitRequired(vtkPoints* coverVertices, const std::vector<double>& sigmas, const vtkIdType idVi, 
            const vtkIdType idVj, const vtkIdType idVk, VectorType& Vc, double& Svc) const;


    //save the cover for debugging mostly. the localCover ids should point to the elements of the 
    //coverVertices array
    void SaveIsolatedCover(const HoleCoverType& localCover, vtkPoints * coverVertices, const char* filename) const;



    //return false if there is no pair



    HoleCoverType::iterator FindTriangleByPointIds(HoleCoverType& localCover, vtkIdType id0, vtkIdType id1, vtkIdType id2) const;


    //try to replace edge "edge" with "candidateEdge". Both must have ids into localCover.
    //conn - upper! triangular connectivity matrix, will be updated
    //coverVertices - vertex coordinates of the cover
    //localCover - cover with ids into coverVertices, will be updated
    //returns true if at least one swap was performed

    //returns true if at least one swap was performed
    
    bool CheckForDuplicateTriangles(const HoleCoverType& localCover) const;
};


#endif
