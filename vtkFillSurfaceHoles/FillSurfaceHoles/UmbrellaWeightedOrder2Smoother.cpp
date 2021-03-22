/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#include "UmbrellaWeightedOrder2Smoother.h"
#include <vtkSmartPointer.h>
#include <vtkPolyDataWriter.h>
#include <vtkPolyData.h>



double UmbrellaWeightedOrder2Smoother::CalculateEdgeWeight(vtkIdType v_third, vtkIdType v1_edge, vtkIdType v2_edge) const
{
    Eigen::Vector3d v_third_p, v1_edge_p, v2_edge_p;
    m_originalMesh->GetPoint( v_third, v_third_p.data() );
    m_originalMesh->GetPoint( v1_edge, v1_edge_p.data() );
    m_originalMesh->GetPoint( v2_edge, v2_edge_p.data() );
    
    return CalculateEdgeWeight( v_third_p, v1_edge_p, v2_edge_p );
}


double UmbrellaWeightedOrder2Smoother::CalculateEdgeWeight(const Eigen::VectorXd& v_third, const Eigen::VectorXd& v1_edge, const Eigen::VectorXd& v2_edge) const
{
    double value = -1;
    switch( m_weightingType )
    {
        case EDGE_WEIGHT_COTANGENT:
            value = EdgeWeightCotangent(v_third, v1_edge, v2_edge);
            break;
        case EDGE_WEIGHT_InvEdgeLength:
            value = EdgeWeightInvEdgeLength(v1_edge, v2_edge);
            break;
        default:
            std::cout<<"Undefined vertex weighting type";
            throw "Undefined vertex weighting type";
    }
    return value;
}

double UmbrellaWeightedOrder2Smoother::EdgeWeightCotangent(const Eigen::VectorXd& v_third, const Eigen::VectorXd& v1_edge, const Eigen::VectorXd& v2_edge) const
{
    const VectorType v13 = (v2_edge-v_third).normalized();
    const VectorType v12 = (v1_edge-v_third).normalized();
    const double angle = std::acos(v13.dot(v12));
    const double w23 = 1/std::tan(angle);
    return w23;
}




double UmbrellaWeightedOrder2Smoother::EdgeWeightInvEdgeLength(const Eigen::VectorXd& v1_edge, const Eigen::VectorXd& v2_edge) const
{
    const double w23 = 1/( (v1_edge-v2_edge).norm() );
}







void UmbrellaWeightedOrder2Smoother::GetVertexNeighbors( vtkIdType vertexId, VertexIDArrayType& neighbors) const {
    
    std::set<vtkIdType> connectedVertices;
    
    //get all cells that vertex 'seed' is a part of 
    vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
    m_originalMesh->GetPointCells(vertexId, cellIdList);

    //loop through all the cells that use the seed point 
    for (vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++) {

        vtkCell* cell = m_originalMesh->GetCell(cellIdList->GetId(i));

        for (vtkIdType e = 0; e < cell->GetNumberOfEdges(); e++) {
            vtkCell* edge = cell->GetEdge(e);

            vtkIdList* pointIdList = edge->GetPointIds();

            if (pointIdList->GetId(0) == vertexId || pointIdList->GetId(1) == vertexId) {
                if (pointIdList->GetId(0) == vertexId) {
                    connectedVertices.insert(pointIdList->GetId(1));
                }
                else {
                    connectedVertices.insert(pointIdList->GetId(0));
                }
            }
        }
    }
    
    neighbors.clear();
//    std::cout<<"Neighbors of "<<vertexId<<" "<<std::endl;
    for( std::set<vtkIdType>::const_iterator it=connectedVertices.begin(); it!=connectedVertices.end(); it++ )
    {
        neighbors.push_back( (*it) );
//        std::cout<<(*it)<<" "<<std::endl;
    }
}

//static int idx = 0;
void UmbrellaWeightedOrder2Smoother::Update()
{
//    vtkSmartPointer<vtkPolyDataWriter> wr = vtkSmartPointer<vtkPolyDataWriter>::New();
//    wr->SetFileName("input_mesh.vtk");
//    wr->SetInputData(m_originalMesh);
//    wr->Write();

    switch(m_weightingType)
    {
        case EDGE_WEIGHT_COTANGENT:
            if(m_verbose)
                std::cout<<"Using cotangent edge weights"<<std::endl;
            break;
        case EDGE_WEIGHT_InvEdgeLength:
            if(m_verbose)
                std::cout<<"Using inverse edge length edge weights"<<std::endl;
            break;
        default:
            std::cout<<"Unknown edge weighting"<<std::endl;
            throw;                   
    }
    
    CalculateConnectivity();
        
    CalculateEdgeWeightMatrix();
//    std::cout<<m_W<<std::endl;
    CalculateWeightSums();
//    std::cout<<m_WS<<std::endl;
    


    //start forming the system of equations
    SparseDoubleMatrixType A(m_C.size(), m_C.size());
    Eigen::Matrix<double, Eigen::Dynamic, 3> B; //right hand side
    B.resize(m_C.size(), Eigen::NoChange);
    B.fill(0);
    
    for( VertexConnectivityArrayType::const_iterator C_it = m_C.begin(); C_it!=m_C.end(); C_it++ )
    {
        const vtkIdType vert_index = FindVertexConnectivityLocalID( (*C_it).originalID );
        if( vert_index<m_C.size() )
        {
            if( (*C_it).vertexClass != vcInterior ) //add 1 at the vertex position, these vertices are fixed
            {            
                A.coeffRef(vert_index,vert_index) = 1;

                //add right hand side
                Eigen::RowVector3d v;
                m_originalMesh->GetPoint( (*C_it).originalID, v.data() );
                B.row(vert_index) = v;
            }
            else
            {
                //add left hand side
                FormSystemOfEquationsRow( (*C_it), A );

            }
        }
    }
    
    
    Eigen::BiCGSTAB < Eigen::SparseMatrix<double> > cg;
    cg.compute(A);
    Eigen::MatrixXd X = cg.solve(B);
    if(m_verbose)
    {
        std::cout << "Solving sparse system using BiCGSTAB" << std::endl;
        std::cout << "#iterations:     " << cg.iterations() << std::endl;
        std::cout << "estimated error: " << cg.error() << std::endl;
    }
    
    
//    std::ofstream file("matrix.txt");
//    file << A ;
//    std::ofstream file1("b.txt");
//    file1 << B ;   
    
    CreateOutput(X);

    
//    char filename[100];
//    sprintf(filename,"mesh%03d.vtk",idx++);
//        m_originalMesh->BuildCells();
//
//    wr->SetFileName(filename);
//    wr->SetInputData(m_originalMesh);
//    wr->Write();

}


void UmbrellaWeightedOrder2Smoother::ClassifyVertex(VertexConnectivityType& v)
{
    VertexIDArrayType::iterator cover_it = std::find(m_coverVertexIDs->begin(), m_coverVertexIDs->end(), v.originalID);
    
    if( cover_it != m_coverVertexIDs->end() ) //the point is on the cover. See if at least one neighbor is from the exterior
    {
        v.vertexClass = vcInterior; //interior if not boundary
        
        
        //verify if it could be on the boundary
        for(VertexIDArrayType::const_iterator it = v.connectedVertices.begin(); 
                it!=v.connectedVertices.end(); it++)
        {
            const vtkIdType id = (*it);
            VertexIDArrayType::iterator neighb_it = std::find(m_coverVertexIDs->begin(), m_coverVertexIDs->end(), id);
            
            if( neighb_it == m_coverVertexIDs->end() )
            {
                v.vertexClass = vcBoundary;
                break;
            }
        }
    }
    else
    {
        v.vertexClass = vcExterior;
    }
}



void UmbrellaWeightedOrder2Smoother::CalculateConnectivity()
{
    //go over all cover vertices
    m_C.clear();

    //also add 1-ring around the edge
    std::set<vtkIdType> edge_neighborhood;


    for( VertexIDArrayType::const_iterator it = m_coverVertexIDs->begin(); it!=m_coverVertexIDs->end(); it++ )
    {
        //get neighbors for this vertex
        VertexConnectivityType vc; 
        vc.vertexClass = vcExterior; //default
        vc.originalID = (*it); 
        
//        std::cout<<"Getting neighbors for v "<<(*it)<<std::endl<<std::flush;
        GetVertexNeighbors( (*it), vc.connectedVertices );
        ClassifyVertex( vc );

        //add the neighborhood of every edge vertex
        if( vc.vertexClass == vcBoundary )
            edge_neighborhood.insert<VertexIDArrayType::iterator> ( vc.connectedVertices.begin(), vc.connectedVertices.end() );
        
        m_C.push_back(vc);
    }

    
    //Calculate the 1 ring
    VertexIDArrayType edge1ring;
    edge1ring.reserve(edge_neighborhood.size());
    
    SetDifference(edge_neighborhood, *m_coverVertexIDs, edge1ring);
    for( VertexIDArrayType::const_iterator it = edge1ring.begin(); it!=edge1ring.end(); it++ )
    {
        //get neighbors for this vertex
        VertexConnectivityType vc; 
        vc.vertexClass = vcExterior; //1-ring = exterior
        vc.originalID = (*it); 
        
//        std::cout<<"Getting neighbors for v "<<(*it)<<std::endl<<std::flush;
        GetVertexNeighbors( (*it), vc.connectedVertices );
        
        m_C.push_back(vc);
    }
    
    
}


//
//void UmbrellaWeightedOrder2Smoother::CalculateU(MatrixUType& U, Eigen::SparseMatrix<double>& weights)
//{
//    weights.resize(m_originalMesh->GetNumberOfPoints(), m_originalMesh->GetNumberOfPoints());
//    U.resize(Eigen::NoChange, m_C.size());
//    
//    vtkIdType vertex_index=0;    
//    for( VertexConnectivityArrayType::const_iterator it=m_C.begin(); it!=m_C.end(); it++, vertex_index++ )
//    {
//        Eigen::VectorXd w(it->connectedVertices.size()); //weights
//
//        Eigen::Vector3d v;
//        m_originalMesh->GetPoint( it->originalID, v.data() );
//        
//        //iterate over all the neighbors and calculate weights
//        vtkIdType neighb_index=0;
//        Eigen::Vector3d vi;
//
//        Eigen::Vector3d Un; Un.fill(0);
//        
//        for( VertexIDArrayType::const_iterator neighb_it = it->connectedVertices.begin();
//                neighb_it != it->connectedVertices.end(); neighb_it++, neighb_index++)
//        {
//            const vtkIdType neighb_id = (*neighb_it);
//            m_originalMesh->GetPoint( neighb_id, vi.data() );
//
//            w(neighb_index) = TriangleWeightScaleDependent(v,vi);
//            
//            //save only upper triangular matrix
////            std::cout<<"Adding connectivity : "<<it->originalID<<", "<<neighb_id<<std::endl;
//            weights.coeffRef(std::min(it->originalID, neighb_id), std::max(it->originalID, neighb_id)) = w(neighb_index);
//            
//            Un += vi*w(neighb_index);
//        }
//        
//        const double Wt = w.sum();
//        
//        U.col(vertex_index) = -v + Un/Wt;
//    }
//}

//
//void UmbrellaWeightedOrder2Smoother::CalculateU2(MatrixUType& U2, const MatrixUType& U, const Eigen::SparseMatrix<double>& weights)
//{
//    U2.resize(Eigen::NoChange, m_C.size());
//
//        
//    vtkIdType vertex_index=0;
//    for( VertexConnectivityArrayType::const_iterator it=m_C.begin(); it!=m_C.end(); it++, vertex_index++ )
//    {
//        Eigen::VectorXd w(it->connectedVertices.size()); //weights
//
//        Eigen::Vector3d Uv = U.col(vertex_index);
//        
//        
//        //iterate over all the neighbors and calculate weights
//        vtkIdType neighb_index=0;
//        Eigen::Vector3d Uvi;
//
//        Eigen::Vector3d Un; Un.fill(0);
//        
//        for( VertexIDArrayType::const_iterator neighb_it = it->connectedVertices.begin();
//                neighb_it != it->connectedVertices.end(); neighb_it++, neighb_index++)
//        {
//            const vtkIdType neighb_id = (*neighb_it);
//
//            Uvi = U.col(neighb_index);
//            w(neighb_index) = weights.coeff( std::min(it->originalID, neighb_id), std::max(it->originalID, neighb_id) );
//            
//            Un += Uvi*w(neighb_index);
//        }
//        
//        const double Wt = w.sum();
//        
//        U2.col(vertex_index) = -Uv + Un/Wt;
//    }    
//}
//

//
//double UmbrellaWeightedOrder2Smoother::UpdateMeshPoints( const MatrixUType& U2 )
//{
//    double vertex_difference = 0; //how much the update displaced the vertices
//
//    vtkIdType vertex_index=0;
//    
//    for( VertexConnectivityArrayType::const_iterator it=m_C.begin(); it!=m_C.end(); it++, vertex_index++ )
//    {
//        if(it->vertexClass==vcInterior)
//        {
//            Eigen::Vector3d Pi;
//            m_originalMesh->GetPoint( it->originalID, Pi.data() );
//            
//            //iterate over all the neighbors and calculate valences
//            vtkIdType neighb_index=0;
//            double Vsum = 0; //valence factor
//            for( VertexIDArrayType::const_iterator neighb_it = it->connectedVertices.begin();
//                    neighb_it != it->connectedVertices.end(); neighb_it++, neighb_index++)
//            {
//                const vtkIdType neighb_id = (*neighb_it);
//                
//                //find neighb_id in m_C and get its valence
//                vtkIdType neighb_C_id = FindVertexConnectivityInfo(neighb_id);
//                Vsum += 1/m_C.at(neighb_C_id).connectedVertices.size();
//                
//            }
//            
//            const double V = 1 + Vsum/it->connectedVertices.size();
//            
//            Eigen::Vector3d Pi_new = Pi - U2.col(vertex_index)/V;
//            m_originalMesh->GetPoints()->SetPoint( it->originalID, Pi_new.data() );
//            
//            vertex_difference += (Pi-Pi_new).norm();
//        }
//    }
//    
//    return vertex_difference;
//}
//



//returns m_C.size() on failure
vtkIdType UmbrellaWeightedOrder2Smoother::FindVertexConnectivityLocalID( vtkIdType id ) const//uses ids within mesh, compares to originalID
{
    vtkIdType vertex_index=0;
    for( VertexConnectivityArrayType::const_iterator it=m_C.begin(); it!=m_C.end(); it++, vertex_index++ )
    {
        if(it->originalID==id)
            break;
    }    
    
    return vertex_index;
}



void UmbrellaWeightedOrder2Smoother::CalculateEdgeWeightMatrix( ) 
{
    const vtkIdType n_pts = m_originalMesh->GetNumberOfPoints();
    
    //Create cotan weight n x n matrix 
    m_W.resize(n_pts, n_pts);
    
    //for every vertex
    for( VertexIDArrayType::const_iterator vert_it = m_coverVertexIDs->begin();
            vert_it!=m_coverVertexIDs->end(); vert_it++)
    {
        const vtkIdType v1id = (*vert_it);
        
        //get connectivity
        const vtkIdType v1id_in_C = FindVertexConnectivityLocalID( v1id );
        
        if( v1id_in_C<m_C.size() )
        {
            const VertexIDArrayType &v1_neighbors = m_C.at(v1id_in_C).connectedVertices;

            //for every edge find the 2 triangles
            for( VertexIDArrayType::const_iterator neighb_it = v1_neighbors.begin(); neighb_it!=v1_neighbors.end(); neighb_it++ )
            {
                const vtkIdType v2id = (*neighb_it);
                //edge formed by v1id and v2id
                //find triangles by finding the vertices shared by both


    //            std::cout<<"W("<<v1id<<", "<<v2id<<")="<<m_W.coeff(v1id, v2id)<<std::endl<<std::flush;
                if( m_W.coeff(v1id, v2id)==0 ) //to avoid recalculating the weight
                {

                    VertexIDArrayType common_vertex_ids;
                    if( FindThirdVertexIds(v1id, v2id, common_vertex_ids) )
                    {
                        for( VertexIDArrayType::const_iterator common_v_it = common_vertex_ids.begin();
                                common_v_it != common_vertex_ids.end(); common_v_it++)
                        {
                            const vtkIdType V_common_id = (*common_v_it);

                            const double w = CalculateEdgeWeight(V_common_id, v1id, v2id);
//                            std::cout<<"Adding weight for "<<v1id<<", "<<v2id <<std::endl;
                            m_W.coeffRef( v1id, v2id ) += w;
                            m_W.coeffRef( v2id, v1id ) += w;
                        }
                    }
                }


            }                
        }
    }    
    
    
}






bool UmbrellaWeightedOrder2Smoother::IntersectVectors( const VertexIDArrayType& a, const VertexIDArrayType& b, VertexIDArrayType& c ) const
{
    std::set<VertexIDArrayType::value_type> t1;

    const VertexIDArrayType* l; //longest
    const VertexIDArrayType* s; //shortest
    
    if(a.size()<b.size()) 
    {
        l = &b;
        s = &a;
    }
    else
    {
        l = &a;
        s = &b;        
    }
        
        
        
    t1.insert<VertexIDArrayType::const_iterator> (s->begin(), s->end());
    
    c.clear();
    c.reserve( s->size() );
    for( VertexIDArrayType::const_iterator l_it = l->begin(); l_it<l->end(); l_it++ )
    {
        std::set<VertexIDArrayType::value_type>::iterator found_id_it = t1.find( (*l_it) );
        if( found_id_it != t1.end() )
            c.push_back( (*found_id_it) );
    }
    
    return c.size()>0;
}





bool UmbrellaWeightedOrder2Smoother::FindThirdVertexIds(vtkIdType p1, vtkIdType p2, VertexIDArrayType& third_v) const {
    vtkSmartPointer<vtkIdList> n = vtkSmartPointer<vtkIdList>::New();
    m_originalMesh->GetPointCells(p1, n);
         
    third_v.clear();

    for (vtkIdType i = 0; i < n->GetNumberOfIds(); i++) {
        const vtkIdType cell_id = n->GetId(i);
        vtkIdList* ptids = m_originalMesh->GetCell(cell_id)->GetPointIds();

        for (vtkIdType j = 0; j < ptids->GetNumberOfIds(); j++) {
            if (p2 == ptids->GetId(j)) //found the cell sharing 2 points
            {
                //found a cell with 2 matching points
                for (vtkIdType i = 0; i < ptids->GetNumberOfIds(); i++) {
                    const vtkIdType thirdid = ptids->GetId(i);
                    if (thirdid != p1 && thirdid != p2)
                        third_v.push_back( thirdid );
                }


            }
        }

    }

    
    if(third_v.size()>2)
    {
        std::cout<<"The edge "<<p1<<", "<<p2<<" has more than 2 neighbors"<<std::endl;
        throw;
    }
    assert( third_v.size()<3 );
    

    return third_v.size()>0;
}




bool UmbrellaWeightedOrder2Smoother::SetDifference( const std::set<vtkIdType>& a, const VertexIDArrayType& b, VertexIDArrayType& c ) const
{
    c.clear();
    
    for( std::set<vtkIdType>::const_iterator a_it = a.begin(); a_it!=a.end(); a_it++ )
    {
        VertexIDArrayType::const_iterator b_it;
        for( b_it = b.begin(); b_it!=b.end(); b_it++ )
        {
            if( (*a_it) == (*b_it) ) break;
        }
        if( b_it==b.end() ) //not found, add to c
        {
            c.push_back((*a_it));
        }
    }
    
    return c.size()>0;
}



void UmbrellaWeightedOrder2Smoother::FormSystemOfEquationsRow( const VertexConnectivityType& vk, SparseDoubleMatrixType& A  ) const
{
      //!!!!m_W has ORIGINAL MESH IDS.
    
    //ok, this is a bt of a mess
    //W - uses original ids (ids inside the mesh)
    //but to build the system of equations we need to use ids within m_C
    
    //put in -U(vk) + 1/W(vk)*sum[ W(vk,vi) U(vi) ]
    //with U(vi) = -vi + 1/W(vi) sum[ W(vi,vj) vj ] over neighborhood j
    assert(m_W.cols()>0);
    assert(m_WS.cols()>0);
    
    
    //we are doing this for Vk. k = index of vk in m_C
    //all i-s must be converted to indices within m_C
    
    //start by inserting -U(vk)
    //we need to get the neighborhood of vk and insert the coefficient of each neighbor
    const vtkIdType k = FindVertexConnectivityLocalID( vk.originalID );
    const VertexIDArrayType& vk_nbhood = vk.connectedVertices;
    
    AddUviToSystemOfEquationsRow(k, vk, -1, A);
    
    for(VertexIDArrayType::const_iterator vkn_it = vk_nbhood.begin(); vkn_it != vk_nbhood.end(); vkn_it++ )
    {
        //i - index of (*vkn_it) in m_C
        const vtkIdType viID = (*vkn_it);
        const vtkIdType i = FindVertexConnectivityLocalID( viID );
        if(i<m_C.size())
        {
            const VertexConnectivityType &vi = m_C.at(i);

//            std::cout<<"W(k,i): "<<m_W.coeff(vk.originalID, viID)<<std::endl;
//            std::cout<<"W(k): "<<m_WS.coeff(vk.originalID,0)<<std::endl;
            AddUviToSystemOfEquationsRow(k, vi, m_W.coeff(vk.originalID, viID)/m_WS.coeff(vk.originalID,0), A);
        }
    }
}

void UmbrellaWeightedOrder2Smoother::AddUviToSystemOfEquationsRow( vtkIdType row, const VertexConnectivityType& vi, double weight, SparseDoubleMatrixType& A  ) const
{
    //adds U(vi) = -vi + 1/W(vi) sum[ W(vi,vj) vj ] over neighborhood j
    const vtkIdType i = FindVertexConnectivityLocalID( vi.originalID );
    
    if( i<m_C.size() )
    {
//        std::cout<<"filling row "<<row<<std::endl;
//        std::cout<<"i = "<<i<<std::endl;
        A.coeffRef(row, i) += -1*weight;

        //for j over neighborhood of vi
        const VertexIDArrayType& Vi_nbhood = vi.connectedVertices;

        for(VertexIDArrayType::const_iterator Vin_it = Vi_nbhood.begin(); Vin_it != Vi_nbhood.end(); Vin_it++ )
        {
            const vtkIdType Vj_id = (*Vin_it);
            const vtkIdType j = FindVertexConnectivityLocalID( Vj_id );

            if( j<m_C.size() ) //if found
            {
//                std::cout<<"j = "<<j<<" W(ij)="<<m_W.coeff(vi.originalID, Vj_id)<<" W(i)="<<m_WS.coeff(vi.originalID,0)<<std::endl;
//                std::cout<<"A("<<row<<", "<<j<<")="<<std::flush;
//                std::cout<<A.coeffRef(row,j)<<std::endl;
                A.coeffRef(row,j) += weight*m_W.coeff(vi.originalID, Vj_id)/m_WS.coeff(vi.originalID,0);        
            }
        }
    }
}



void UmbrellaWeightedOrder2Smoother::CalculateWeightSums( )
{
    m_WS.resize(m_W.outerSize(),1);    
    
    for (Eigen::Index k = 0; k < m_W.outerSize(); ++k){
        for (SparseDoubleMatrixType::InnerIterator it(m_W, k); it; ++it) {
            m_WS.coeffRef(k, 0) += it.value();
        }
    }
}



void UmbrellaWeightedOrder2Smoother::CreateOutput( Eigen::MatrixXd X )
{
    Eigen::RowVector3d v;
    for( Eigen::Index i=0; i<X.rows(); i++)
    {
        v = X.row(i);
        const vtkIdType originalID = m_C.at(i).originalID;
        m_originalMesh->GetPoints()->SetPoint( originalID, v.data() );
    }
    
    m_originalMesh->BuildCells();
}