/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
//patch smoothing by solving 
//U^2 (v) = 0
//where U is the second order weighted umbrella operator with cotangent weights
//v - each of the patch vertices
//The patch must be a topological disk embedded in 3D
//For details see paper of Liepa, Filling Holes in Meshes
//
//Solution using iterative process from :
//Interactive Multi-Resolution Modeling on Arbitrary Meshes, Leif Kobbelt

#ifndef __UmbrellaWeightedOrder2Smoother_h
#define __UmbrellaWeightedOrder2Smoother_h

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include "HoleFillerDefines.h"

#define EDGE_WEIGHT_COTANGENT 100
#define EDGE_WEIGHT_InvEdgeLength 200


//uncomment to use cotangent weights during smoothing instead of scale-dependent


//updates the input mesh
class UmbrellaWeightedOrder2Smoother
{
public:
    typedef std::vector<vtkIdType> VertexIDArrayType;
        
    void SetInputMesh(vtkPolyData *mesh) {m_originalMesh = mesh;};
    void SetCoverVertexIds(VertexIDArrayType* ids) {m_coverVertexIDs = ids;};
    
    int GetEdgeWeightingType() const {return m_weightingType; };
    void SetEdgeWeightingType(int wt) {m_weightingType=wt; };
    void EdgeWeightingTypeCotangent() {m_weightingType=EDGE_WEIGHT_COTANGENT; };
    void EdgeWeightingTypeInvEdgeLength() {m_weightingType=EDGE_WEIGHT_InvEdgeLength; };
    
    void SetVerbose(bool verbose) {m_verbose=verbose;};
    
    void Update();
    
    
    UmbrellaWeightedOrder2Smoother():m_originalMesh(NULL), m_coverVertexIDs(NULL),
        m_weightingType(EDGE_WEIGHT_COTANGENT), m_verbose(true) {};
    
    
        
protected:
    typedef enum {vcInterior, vcBoundary, vcExterior} VertexClassType;
 
    typedef struct __connectivity
    {
        vtkIdType originalID;
        VertexIDArrayType connectedVertices;
        VertexClassType vertexClass;
    } VertexConnectivityType;    
    
    
    typedef Eigen::Matrix<double, 3, Eigen::Dynamic> MatrixUType;
    typedef Eigen::SparseMatrix<double> SparseMatrixDoubleType;
    
    //calculate the weight for the edge v1v2
    double CalculateEdgeWeight(const Eigen::VectorXd& v_third, const Eigen::VectorXd& v1_edge, const Eigen::VectorXd& v2_edge) const;
    double CalculateEdgeWeight(vtkIdType v_third, vtkIdType v1_edge, vtkIdType v2_edge) const;
    double EdgeWeightCotangent(const Eigen::VectorXd& v_third, const Eigen::VectorXd& v1_edge, const Eigen::VectorXd& v2_edge) const;
    double EdgeWeightInvEdgeLength(const Eigen::VectorXd& v1_edge, const Eigen::VectorXd& v2_edge) const;


    void GetVertexNeighbors(vtkIdType vertexId, VertexIDArrayType& neighbors) const;
    
    void CalculateConnectivity(); //updates m_C
    void ClassifyVertex(VertexConnectivityType& v);
    
    //Fills U and weights
//    void CalculateU( MatrixUType& U, Eigen::SparseMatrix<double>& weights ); 
    //uses U and weights and fills U2
//    void CalculateU2( MatrixUType& U2, const MatrixUType& U, const Eigen::SparseMatrix<double>& weights );
    
//    double UpdateMeshPoints( const MatrixUType& U2 );
    
    vtkIdType FindVertexConnectivityLocalID( vtkIdType id ) const; //uses ids within mesh, compares to originalID
    
    void CalculateEdgeWeightMatrix( );
    void CalculateWeightSums();
    
    bool FindThirdVertexIds(vtkIdType p1, vtkIdType p2, VertexIDArrayType& third_v) const;
    
    void FormSystemOfEquationsRow( const VertexConnectivityType& vc, SparseDoubleMatrixType& A ) const;

    void AddUviToSystemOfEquationsRow( vtkIdType row, const VertexConnectivityType& vi, double weight, SparseDoubleMatrixType& A  ) const;

    //extract the coordinates of the inner vertices and copy into the mesh
    void CreateOutput( Eigen::MatrixXd X );
    

    bool IntersectVectors( const VertexIDArrayType& a, const VertexIDArrayType& b, VertexIDArrayType& c ) const;
    
    //calculates set a-b. 
    bool SetDifference( const std::set<vtkIdType>& a, const VertexIDArrayType& b, VertexIDArrayType& c ) const;

private:
    typedef std::vector<VertexConnectivityType> VertexConnectivityArrayType; 
    
    vtkPolyData *m_originalMesh;
    VertexIDArrayType* m_coverVertexIDs; //IDs within the original mesh
    
    double m_tolerance;
    int m_maxIter;

    int m_weightingType;
    SparseMatrixDoubleType m_W; //matrix of edge weights
    SparseMatrixDoubleType m_WS; //columnwise sum of weights, nx1
    
    VertexConnectivityArrayType m_C; //stores IDs within the original mesh
    
    bool m_verbose;
};


#endif
