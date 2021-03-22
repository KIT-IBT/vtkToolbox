/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#include "CoverRefiner.h"
#include "HoleFillerDefines.h"

#include <vtkPolyDataWriter.h>

#include <stack>


void CoverRefiner::InitializeConnectivityMatrix(ConnectivityMatrixType& conn){
    
    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletList;
    
    for (HoleCoverType::const_iterator it = m_coverFaces->begin(); it != m_coverFaces->end(); ++it) {
        const vtkIdType id0 = (*it).id[0];
        const vtkIdType id1 = (*it).id[1];
        const vtkIdType id2 = (*it).id[2];
        
//        conn.coeffRef(std::min(id0, id1), std::max(id0, id1)) += 1;
//        conn.coeffRef(std::min(id1, id2), std::max(id1, id2)) += 1;
//        conn.coeffRef(std::min(id0, id2), std::max(id0, id2)) += 1;
        tripletList.push_back(T(std::min(id0, id1), std::max(id0, id1),1) );
        tripletList.push_back(T(std::min(id1, id2), std::max(id1, id2),1) );
        tripletList.push_back(T(std::min(id0, id2), std::max(id0, id2),1) );
    }
    
    conn.setFromTriplets(tripletList.begin(), tripletList.end());
    
    //std::cout<<conn<<std::endl;
}

void CoverRefiner::Update()
{
    ConnectivityMatrixType conn;
    try
    {
        //conn.resize(MAX_NUMBER_OF_VERTICES, MAX_NUMBER_OF_VERTICES);
        conn.resize(m_coverVertices->GetNumberOfPoints(), m_coverVertices->GetNumberOfPoints());
        //conn.fill(0);
    }
    catch(...)
    {
        std::cout<<"Error allocating space for connectivity matrix in Refiner"<<std::endl;
        throw;
    }
        
    //std::cout<<"Cover size: "<<localCover.size()<<std::endl;
    InitializeConnectivityMatrix(conn);
    //create storage for weights
    //this will be synchronized with coverVertices
   
    //------------------------------------------
    //
    //  Step 1:    
    //   for each vertex on hole boundary calculate s(vi) = average adjacent edge lengths, !!do not consider cover edges!!   
    //
    //
    //--------------------------------------------------------------
    // This is handled by external call
    //    for(int i=0;i<sigmas.size();i++)
    //        std::cout<<sigmas[i]<<" "<<std::endl;
    
  
    
    //------------------------------------------
    //
    //  Step 1.5:    
    //   relax all edges of the cover
    //
    //
    //--------------------------------------------------
    try
    {
        while( RelaxAllCoverEdges(conn) ) {};
    }
    catch(...)
    {
        std::cout<<"An exception happened"<<std::endl;
    }

    
        
    //------------------------------------------
    //
    //  Step 2:    
    //   
    //    
    //2: for each (vi,vj,vk) of the cover 
    //   get centroid vc
    //   s(vc) = (s(vi)+s(vj)+s(vk))/3
    //   (originally) check for all m=i,j,k sqrt(2)*||vc-vm||>max(s(vc),s(vm))
    //   check for all m=i,j,k 2*||vc-vm||^2 > max(s(vc)^2,s(vm)^2)
    //          replace (vi,vj,vk) with (vc,vj,vk), (vi,vc,vk), (vi,vj,vc)
    //          relax edges (vi,vj), (vi,vk), (vj,vk)
    //          split_created = true
    //   if !split_created end
    //
    //--------------------------------------------------
    
    
    while ( true ) 
    {
        bool TriangleSplitted = false; //algorithm stops when no more splits are required
        
        HoleCoverType new_cover;
        
//        std::cout<<"Cover size: "<<m_coverFaces->size()<<std::endl;

        //SaveIsolatedCover("1.vtk");
        
        //for(HoleCoverType::iterator coverIt = m_coverFaces->begin(); coverIt!=m_coverFaces->end(); coverIt++)    
        for(HoleCoverType::iterator coverIt = m_coverFaces->begin(); coverIt!=m_coverFaces->end(); coverIt++)    
        {
            
            //calculate centroid
            const vtkIdType idVi = (*coverIt).id[0];
            const vtkIdType idVj = (*coverIt).id[1];
            const vtkIdType idVk = (*coverIt).id[2];
            
            VectorType Vc;
            double Svc;
            
//            std::cout<<"Cover size: "<<m_coverFaces->size()<<std::endl;
//            std::cout<<"Splitting verifying: "<<idVi<<" "<<idVj<<" "<<idVk<<std::endl;
            if( IsTriangleSplitRequired(idVi, idVj, idVk, Vc, Svc) )
            {  //create new triangles
                //erase old triangle
                
                //                std::cout<<"Splitting confirmed: "<<idVi<<" "<<idVj<<" "<<idVk<<std::endl;
                
                //m_coverFaces->erase(coverIt);
                
                //add new vertex
                const vtkIdType idVc = m_coverVertices->InsertNextPoint(Vc.data());
                
                
                //replace (vi,vj,vk) with (vc,vj,vk), (vi,vc,vk), (vi,vj,vc)
                TriangleCellType tri1; tri1.id[0]=idVc; tri1.id[1]=idVj; tri1.id[2]=idVk;
                TriangleCellType tri2; tri2.id[0]=idVi; tri2.id[1]=idVc; tri2.id[2]=idVk;
                TriangleCellType tri3; tri3.id[0]=idVi; tri3.id[1]=idVj; tri3.id[2]=idVc;
                //relax edges (vi,vj), (vi,vk), (vj,vk)
                //              TriangleSplitted = true;
                new_cover.push_back(tri1);
                new_cover.push_back(tri2);
                new_cover.push_back(tri3);

//                std::cout<<"Inserting: "<<tri1.id[0]<<" "<<tri1.id[1]<<" "<<tri1.id[2]<<std::endl;                
//                std::cout<<"Inserting: "<<tri2.id[0]<<" "<<tri2.id[1]<<" "<<tri2.id[2]<<std::endl;                
//                std::cout<<"Inserting: "<<tri3.id[0]<<" "<<tri3.id[1]<<" "<<tri3.id[2]<<std::endl;                

//                CheckForDuplicateTriangles();
                
                
                //Update matrices
                m_sigmas.push_back(Svc);
                
                //new point added - need resize
                //conn.conservativeResize(coverVertices->GetNumberOfPoints(), coverVertices->GetNumberOfPoints());
                //std::cout<<"conn matrix size: "<<conn.rows()<<", "<<conn.cols()<<std::endl;
//                vtkIdType max_id1 =  std::max( tri1.id[0], std::max( tri1.id[1], tri1.id[2]) );
//                vtkIdType max_id2 =  std::max( tri2.id[0], std::max( tri2.id[1], tri2.id[2]) );
//                vtkIdType max_id3 =  std::max( tri3.id[0], std::max( tri3.id[1], tri3.id[2]) );
//                vtkIdType max_id4 =  std::max( max_id1, std::max( max_id2, max_id3) );
//
//                std::cout<<"max id to store: "<<max_id4<<std::endl;
                
//                if(m_coverVertices->GetNumberOfPoints()>MAX_NUMBER_OF_VERTICES)
//                {
                try
                {
                    conn.conservativeResize(m_coverVertices->GetNumberOfPoints(), m_coverVertices->GetNumberOfPoints());
                }
                catch(...)
                {
                    std::cout<<"Error during conservativeResize of conn in refiner"<<std::endl;
                    throw;
                }
//                }
                
                for(int i=0; i<3; i++)
                {
                    conn.coeffRef(std::min(tri1.id[i], tri1.id[(i+1)%3]), std::max(tri1.id[i], tri1.id[(i+1)%3])) = 2;
                    conn.coeffRef(std::min(tri2.id[i], tri2.id[(i+1)%3]), std::max(tri2.id[i], tri2.id[(i+1)%3])) = 2;
                    conn.coeffRef(std::min(tri3.id[i], tri3.id[(i+1)%3]), std::max(tri3.id[i], tri3.id[(i+1)%3])) = 2;
                }
//                std::cout<<"Stored"<<std::endl;
                //                std::cout<<conn<<std::endl;
                
                //relax edges (vi,vj), (vi,vk), (vj,vk)
//                EdgeType edge; edge.v0 = idVi; edge.v1 = idVj; 
//                EdgeType candidateEdge;
//                
//                std::cout<<"relaxing after split"<<std::endl;
//                if( FindConnectedVertices(&new_cover, edge, candidateEdge) )
//                    RelaxEdgeIfPossible(&new_cover, edge, candidateEdge, conn);
//                std::cout<<"relaxed after split"<<std::endl;
                
                TriangleSplitted = true;
                
                //                SaveIsolatedCover(localCover, coverVertices, "refined.vtk");
            }
            else
            {
                TriangleCellType tri1; tri1.id[0]=idVi; tri1.id[1]=idVj; tri1.id[2]=idVk;
//                std::cout<<"Inserting: "<<tri1.id[0]<<" "<<tri1.id[1]<<" "<<tri1.id[2]<<std::endl;                
                new_cover.push_back(tri1);
            }
            
        }
        
//        std::cout<<"Old faces: "<<m_coverFaces->size()<<std::endl;
//        for(HoleCoverType::const_iterator iit1 = m_coverFaces->begin(); iit1!=m_coverFaces->end(); iit1++)
//                std::cout<<(*iit1).id[0]<<" "<<(*iit1).id[1]<<" "<<(*iit1).id[2]<<std::endl;                
//            
//        std::cout<<"New faces: "<<new_cover.size()<<std::endl;
//        for(HoleCoverType::const_iterator iit1 = new_cover.begin(); iit1!=new_cover.end(); iit1++)
//                std::cout<<(*iit1).id[0]<<" "<<(*iit1).id[1]<<" "<<(*iit1).id[2]<<std::endl;                
            

        m_coverFaces->clear();
        //m_coverFaces->resize(new_cover.size());
        for(HoleCoverType::const_iterator iit1 = new_cover.begin(); iit1!=new_cover.end(); iit1++)
        {
            TriangleCellType tri1; tri1.id[0]=(*iit1).id[0]; tri1.id[1]=(*iit1).id[1]; tri1.id[2]=(*iit1).id[2];
//            std::cout<<"Saving :"<<tri1.id[0]<<" "<<tri1.id[1]<<" "<<tri1.id[2]<<std::endl; 
            m_coverFaces->push_back( tri1 );                
        }
        
        //(*m_coverFaces) = new_cover;
        
//        std::cout<<"After assignment: "<<m_coverFaces->size()<<std::endl;
//        for(HoleCoverType::const_iterator iit1 = m_coverFaces->begin(); iit1!=m_coverFaces->end(); iit1++)
//                std::cout<<(*iit1).id[0]<<" "<<(*iit1).id[1]<<" "<<(*iit1).id[2]<<std::endl;                
//        
        
        //SaveIsolatedCover("2.vtk");
        
//        std::cout<<"Cover size: "<<m_coverFaces->size()<<std::endl;
//        std::cout<<"Sigmas size: "<<m_sigmas.size()<<std::endl;
        //std::cout<<"Connectivity: "<<conn.rows()<<", "<<conn.cols()<<", "<<conn.size()<<std::endl;
        
        //------------------------------------------
        //
        //  Step 3: If no splits were performed - finish
        //   
        if(!TriangleSplitted) break;
        
        
        
        
        //------------------------------------------
        //
        //  Step 4:    
        //   
        //  Relax all cover edges
//        std::cout<<"relaxing started"<<std::endl;
        while( RelaxAllCoverEdges(conn) ) {};

//        std::cout<<"After relaxing: "<<m_coverFaces->size()<<std::endl;
//        for(HoleCoverType::const_iterator iit1 = m_coverFaces->begin(); iit1!=m_coverFaces->end(); iit1++)
//                std::cout<<(*iit1).id[0]<<" "<<(*iit1).id[1]<<" "<<(*iit1).id[2]<<std::endl;                


//        std::cout<<"relaxing complete"<<std::endl;
        
    }    
    
    CheckForDuplicateTriangles();
    
}



void CoverRefiner::InitializeVertexWeights( vtkPolyData* mesh, const VertexIDArrayType *originalBoundaryIds )
{
    m_sigmas.clear();
    
    VectorType pt0, pt1;
    for( vtkIdType vertexId=0; vertexId<originalBoundaryIds->size(); vertexId++ )
    {
        //get the point id in the original mesh
        const vtkIdType originalVertexID = originalBoundaryIds->at(vertexId);
        
        //find vertex neighbors
        std::set<vtkIdType> vertexNeighbors;
        
        GetVertexNeighbors(mesh, originalVertexID, vertexNeighbors);

        
        //        std::cout<<"Vertex "<<originalVertexID<<" connected to ";
        //        for(std::set<vtkIdType>::const_iterator it=vertexNeighbors.begin(); it!=vertexNeighbors.end(); it++)
        //            std::cout<<(*it)<<" ";
        //        std::cout<<std::endl;                             
        double sigma = 0;
        
        mesh->GetPoint(originalVertexID, pt0.data());
        
        for(std::set<vtkIdType>::const_iterator it=vertexNeighbors.begin(); it!=vertexNeighbors.end(); it++)
        {
            mesh->GetPoint((*it), pt1.data());
            sigma += (pt1-pt0).norm();
        }
        
        m_sigmas.push_back( sigma/vertexNeighbors.size() );
    }
}




void CoverRefiner::GetVertexNeighbors(vtkPolyData *mesh, vtkIdType vertexId, 
        std::set<vtkIdType>& connectedVertices)  {

    connectedVertices.clear();
    
    //get all cells that vertex 'seed' is a part of 
    vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPointCells(vertexId, cellIdList);

    //loop through all the cells that use the seed point 
    for (vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++) {

        vtkCell* cell = mesh->GetCell(cellIdList->GetId(i));

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
}





bool CoverRefiner::RelaxAllCoverEdges(ConnectivityMatrixType& conn)  {

    
    std::stack<EdgeType> EdgeStack;
    
//    for (int k=0; k<conn.rows(); ++k)
//        for (int j=0; j<conn.cols(); j++)  {
//            if ( conn.coeffRef(k,j) == 2) { //only interior edges (i.e. 2 cover triangles share it)
//                EdgeType e;
//                e.v0 = k;
//                e.v1 = j;
//                EdgeStack.push(e);
//            }
//            //std::cout<<(*i2)<<" ";
//        }
  
    EdgeType e;
    for (int k=0; k<conn.outerSize(); ++k)
        for (ConnectivityMatrixType::InnerIterator it(conn,k); it; ++it)
        {
            e.v0 = it.row();
            e.v1 = it.col();
            EdgeStack.push(e);
        }

    
    //      
    //   for each element in the queue.
    //      take the intersecting edge vertices
    //      fit a circle to the original 2 vertices plus one of the intersecting ones
    //      check if the 2nd intersecting vertex is inside. If it is swap the edge only if it becomes shorter
    //          compare the length of the old edge and the proposed edge ->swap ->update conn matrix
    //          
    
    
    bool swap_performed = false;
    
    while(!EdgeStack.empty())
    {
        EdgeType edge = EdgeStack.top();
        EdgeStack.pop();
        
        //Using the upper triangular connectivity matrix find the 2 vertices that are connected to the edge
        EdgeType candidateEdge;
        if( FindConnectedVertices(m_coverFaces, edge, candidateEdge) )
        {
            
            //verify if swap is needed, first check the edge length criterion, then circumference
//            std::cout<<"edge "<<edge.v0<<" "<<edge.v1<<std::endl;
//            std::cout<<"cand "<<candidateEdge.v0<<" "<<candidateEdge.v1<<std::endl;
            swap_performed = RelaxEdgeIfPossible(m_coverFaces, edge, candidateEdge, conn);
            
        }
    }
    
    return swap_performed;
}




//conn - upper triangular connectivity matrix
bool CoverRefiner::FindConnectedVertices(HoleCoverType *coverFaces, const EdgeType& edge, 
        EdgeType& intersectingEdge)  {

    std::vector<vtkIdType> ids;
    //bool found_edge = false;
       
//    std::cout<<"Edge orthogonal to ("<<edge.v0<<" "<<edge.v1<<")"<<std::endl;
    
    //iterate through the triangles and find the 2 sharing the edge
    for( HoleCoverType::const_iterator it = coverFaces->begin(); it!=coverFaces->end(); it++ )
    {
        int mask[] = {0,0,0}; //0 - vertex not used, 1 - vertex used
        
//        std::cout<<"Searching ("<<edge.v0<<","<<edge.v1<<"). Triangle "<<(*it).id[0]<<" "<<(*it).id[1]<<" "<<(*it).id[2]<<std::endl;
        for(int i=0; i<3; i++)
            if( (*it).id[i]==edge.v0 )
            {
                mask[i] = 1;
                for(int j=0; j<3; j++)
                    if( (*it).id[j]==edge.v1 )
                    {
                        mask[j]=1;
                        
                        //triangle found
                        for(int k=0; k<3; k++)
                            if(mask[k]==0)
                            {
                                ids.push_back( (*it).id[k] );
//                                std::cout<<"Found 3rd"<<(*it).id[k]<<std::endl;
                                k=4; //continue to the next triangle
                                j=4;
                                i=4;
//                                found_edge = true; 
                            }
                    }
            }
    }

//        if(ids.size()!=2)
//        {
//            std::cout<<"====================================================="<<std::endl<<std::endl;
//            std::cout<<"Error finding adjacent triangles. Edge ("<<edge.v0<<", "
//                    <<edge.v1<<") has "<<ids.size()<<" adjacent triangles, instead of 2."<<std::endl;
//            std::cout<<"Dumping cover to __cover.vtk"<<std::endl<<std::endl;
//            std::cout<<"====================================================="<<std::endl;
//            SaveIsolatedCover(localCover, vertices, "__cover.vtk" );
//        }
//        assert(ids.size()==2);
    
    if (ids.size()==2)
    {
        if(ids.at(0) == ids.at(1))
        {
            std::cout<<"Duplicate triangles exist. Check the code."<<std::endl;
            ids.clear(); //not valid, erase
            assert(ids.at(0) != ids.at(1));
        }
        else
        {
            intersectingEdge.v0 = ids.at(0);
            intersectingEdge.v1 = ids.at(1);
        }
        //        std::cout<<"("<<ids.size()<<") Triangles adjacent to ("<<intersectingEdge.v0<<" "<<intersectingEdge.v1<<")"<<std::endl;
    }
    else
    {
        intersectingEdge.v0 = -1;
        intersectingEdge.v1 = -1;
//        std::cout<<"("<<ids.size()<<") No alternative edge"<<std::endl;
    }
    
    
    return ids.size()==2;
}





//static int cover_id = 0; //for debugging output
bool CoverRefiner::RelaxEdgeIfPossible(HoleCoverType *coverFaces, const EdgeType& edge, const EdgeType& candidateEdge,
        ConnectivityMatrixType& conn)  {
    VectorType edgeV0, edgeV1, candV0, candV1; 
    
    if(candidateEdge.v1==candidateEdge.v0)
    {
        std::cout<<"RelaxEdgeIfPossible: both ends of the candidate edge have the same point ID"<<std::endl;
        std::cout<<"Edge v0="<<edge.v0<<", v1="<<edge.v1<<std::endl;
        std::cout<<"Candidate v0="<<candidateEdge.v0<<", v1="<<candidateEdge.v1<<std::endl;
    }
    assert(candidateEdge.v1!=candidateEdge.v0);
    
    m_coverVertices->GetPoint(edge.v0, edgeV0.data());
    m_coverVertices->GetPoint(edge.v1, edgeV1.data());
    m_coverVertices->GetPoint(candidateEdge.v0, candV0.data());
    m_coverVertices->GetPoint(candidateEdge.v1, candV1.data());
    
    const double edge_length = (edgeV0-edgeV1).squaredNorm();
    const double cand_edge_length = (candV0-candV1).squaredNorm();
    
    bool swap_performed = false;
    
    if(cand_edge_length<edge_length) //check the circumference criterion
    {
        if( IsPointInCircle(edgeV0, edgeV1, candV0, candV1) ) //do the swap
        {
            //do the swap here. Erase old triangles
            
            vtkIdType tri1_id = FindTriangleByPointIds(edge.v0, edge.v1, candidateEdge.v0);
            vtkIdType tri2_id = FindTriangleByPointIds(edge.v0, edge.v1, candidateEdge.v1);

//            std::cout<<"Iterator for triangle: "<<edge.v0<<", "<<edge.v1<<", "<<candidateEdge.v0<<" = "<<(*it1).id[0]<<" "<<(*it1).id[1]<<" "<<(*it1).id[2]<<std::endl;
//            std::cout<<"Iterator for triangle: "<<edge.v0<<", "<<edge.v1<<", "<<candidateEdge.v1<<" "<<(*it2).id[0]<<" "<<(*it2).id[1]<<" "<<(*it2).id[2]<<std::endl;
            
            TriangleCellType newtriangle1;
            TriangleCellType newtriangle2;
            
            HoleCoverType new_cover;
            
            //identify vertex order and create new triangles
            //forward order 
            bool triangles_created = false;
            for(int i=0; i<3; i++)
            {
//                std::cout<<"Triangle: "<<edge.v0<<" "<<edge.v1<<" "<<candidateEdge.v0<<std::endl;
//                std::cout<<"Checking: "<<(*it1).id[i]<<" "<<(*it1).id[(i+1)%3]<<" "<<(*it1).id[(i+2)%3]<<std::endl;
                if( (coverFaces->at(tri1_id).id[i]==edge.v0 && coverFaces->at(tri1_id).id[(i+1)%3]==edge.v1 && coverFaces->at(tri1_id).id[(i+2)%3]==candidateEdge.v0) )
                {
                    newtriangle1.id[0] = candidateEdge.v0;
                    newtriangle1.id[1] = edge.v0;
                    newtriangle1.id[2] = candidateEdge.v1;
                    
                    newtriangle2.id[0] = candidateEdge.v1;
                    newtriangle2.id[1] = edge.v1;
                    newtriangle2.id[2] = candidateEdge.v0;
                    triangles_created = true;
                    break;
                }
            }
            
            if(!triangles_created)
            {
                for(int i=0; i<3; i++)
                {
//                    std::cout<<"Triangle: "<<edge.v0<<" "<<edge.v1<<" "<<candidateEdge.v0<<std::endl;
//                    std::cout<<"Checking: "<<(*it1).id[(i+2)%3]<<" "<<(*it1).id[(i+1)%3]<<" "<<(*it1).id[i]<<std::endl;
                    if( ( coverFaces->at(tri1_id).id[(i+2)%3]==edge.v0 && coverFaces->at(tri1_id).id[(i+1)%3]==edge.v1 && coverFaces->at(tri1_id).id[i]==candidateEdge.v0) )
                    {
                        newtriangle1.id[0] = candidateEdge.v1;
                        newtriangle1.id[1] = edge.v0;
                        newtriangle1.id[2] = candidateEdge.v0;
                        newtriangle2.id[0] = candidateEdge.v0;
                        newtriangle2.id[1] = edge.v1;
                        newtriangle2.id[2] = candidateEdge.v1;
                        triangles_created = true;
                        break;
                    }
                }
            }
            
            for(vtkIdType i=0; i<coverFaces->size(); i++)
            {
                if( i!=tri1_id && i!=tri2_id )
                {
                    TriangleCellType tri;
                    tri.id[0] = coverFaces->at(i).id[0];
                    tri.id[1] = coverFaces->at(i).id[1];
                    tri.id[2] = coverFaces->at(i).id[2];
                    new_cover.push_back(tri);
                }
            }
            
            if(triangles_created)
            {
                //create triangles
                //m_coverFaces->at(tri1_id).erased = true; //erase the second triangle
                //m_coverFaces->at(tri2_id).erased = true;
                
                //add the new triangles
                new_cover.push_back(newtriangle1);
                new_cover.push_back(newtriangle2);
                
                //CheckForDuplicateTriangles();

                    
//                std::cout<<"Added triangles"<<std::endl;
//                std::cout<<"T1: "<<newtriangle1.id[0]<<" "<<newtriangle1.id[1]<<" "<<newtriangle1.id[2]<<std::endl;
//                std::cout<<"T2: "<<newtriangle2.id[0]<<" "<<newtriangle2.id[1]<<" "<<newtriangle2.id[2]<<std::endl;
//                std::cout<<"edge: "<<edge.v0<<" "<<edge.v1<<std::endl;
//                std::cout<<"cand: "<<candidateEdge.v0<<" "<<candidateEdge.v1<<std::endl;
                //update connectivity matrix
                conn.coeffRef( std::min(edge.v0, edge.v1), std::max(edge.v0, edge.v1) ) = 0;
                conn.coeffRef( std::min(candidateEdge.v0, candidateEdge.v1), std::max(candidateEdge.v0, candidateEdge.v1) )  = 2;
                
                swap_performed = true;
            }
            
            coverFaces->clear();
            //m_coverFaces->resize(new_cover.size());
            for(HoleCoverType::const_iterator iit1 = new_cover.begin(); iit1!=new_cover.end(); iit1++)
            {
                TriangleCellType tri1; tri1.id[0]=(*iit1).id[0]; tri1.id[1]=(*iit1).id[1]; tri1.id[2]=(*iit1).id[2];
//                std::cout<<"Saving :"<<tri1.id[0]<<" "<<tri1.id[1]<<" "<<tri1.id[2]<<std::endl; 
                coverFaces->push_back( tri1 );                
            }

//            CheckForDuplicateTriangles();
        }
    }
    
    
    
    
    return swap_performed;
}




bool CoverRefiner::CheckForDuplicateTriangles() 
{
    for( HoleCoverType::const_iterator it1=m_coverFaces->begin(); it1!=m_coverFaces->end(); it1++  )
    {
        for( HoleCoverType::const_iterator it2=m_coverFaces->begin(); it2!=m_coverFaces->end(); it2++  )
        {
            if(it1!=it2)
            {
                std::vector<vtkIdType> tri1;
                std::vector<vtkIdType> tri2;

                for(int i = 0; i<3; i++) 
                {
                    tri1.push_back( (*it1).id[i] );
                    tri2.push_back( (*it2).id[i] );
                }

                std::sort(tri1.begin(), tri1.end());
                std::sort(tri2.begin(), tri2.end());

                bool equal = true;

                std::vector<vtkIdType>::const_iterator tit1, tit2;
                for( tit1 = tri1.begin(), tit2 = tri2.begin();
                        tit1!=tri1.end(); ++tit1, ++tit2)
                {
                    equal = equal && ( (*tit1)==(*tit2) );
                }

                if(equal)
                {
                    std::cout<<"Duplicate triangle found: "<<(*it1).id[0]<<" "<<(*it1).id[1]<<" "<<(*it1).id[2]<<
                        " and "<<(*it2).id[0]<<" "<<(*it2).id[1]<<" "<<(*it2).id[2]<<std::endl;
                    return true;
                }
            }
        }
        
    }
    
    return false;
}






bool CoverRefiner::IsTriangleSplitRequired(const vtkIdType idVi, const vtkIdType idVj, const vtkIdType idVk, VectorType& Vc, double& Svc) {
    VectorType Vi, Vj, Vk;
    m_coverVertices->GetPoint( idVi, Vi.data() );
    m_coverVertices->GetPoint( idVj, Vj.data() );
    m_coverVertices->GetPoint( idVk, Vk.data() );
    
    Vc = (Vi+Vj+Vk)/3;
    Svc = ( m_sigmas.at(idVi) + m_sigmas.at(idVj) + m_sigmas.at(idVk) )/3;
    
    //check for all m=i,j,k 2*||vc-vm||^2 > max(s(vc)^2,s(vm)^2)
    const double Svc2 = Svc*Svc;
    const double Svi2 = m_sigmas.at(idVi)*m_sigmas.at(idVi);
    const double Svj2 = m_sigmas.at(idVj)*m_sigmas.at(idVj);
    const double Svk2 = m_sigmas.at(idVk)*m_sigmas.at(idVk);
    const double dist2_Vc_Vi = 2*(Vc-Vi).squaredNorm();
    const double dist2_Vc_Vj = 2*(Vc-Vj).squaredNorm();
    const double dist2_Vc_Vk = 2*(Vc-Vk).squaredNorm();
    
//    std::cout<<"Splitting triangle ("<<idVi<<" "<<idVj<<" "<<idVk<<std::endl;
//    std::cout<<"S(vi,vj,vk)="<<sigmas[idVi]<<" "<<sigmas[idVj]<<" "<<sigmas[idVk]<<std::endl;
//    std::cout<<"S(vc)="<<Svc<<std::endl;
//    std::cout<<"||Vc-Vi||="<<std::sqrt(dist2_Vc_Vi)<<std::endl;
//    std::cout<<"||Vc-Vj||="<<std::sqrt(dist2_Vc_Vj)<<std::endl;
//    std::cout<<"||Vc-Vk||="<<std::sqrt(dist2_Vc_Vk)<<std::endl;
    
    const bool split_required = ( dist2_Vc_Vi > std::max(Svc2, Svi2) &&
            dist2_Vc_Vj > std::max(Svc2, Svj2) && 
            dist2_Vc_Vk > std::max(Svc2, Svk2) );
    
//    std::cout<<"Split ?: "<<split_required<<std::endl;
    return split_required;
}



bool CoverRefiner::IsPointInCircle(const VectorType& pt0, 
        const VectorType& pt1, const VectorType& pt2, 
        const VectorType& ptcheck) const {
    //https://en.wikipedia.org/wiki/Circumscribed_circle

    typedef Eigen::Vector2d Vector2DType;

    //map points to 2 dimensions    
    Eigen::Matrix<double, 2, 3> T3Dto2D;
    VectorType v10 = (pt1-pt0).normalized();
    VectorType v  = pt2-pt0;
    VectorType n = v.cross( v10 );
    VectorType v20 = v10.cross(n).normalized();
    
    
    T3Dto2D.row(0) = v10;
    T3Dto2D.row(1) = v20;
    
    Vector2DType A;
    A.fill(0); // = T3Dto2D * (pt0-pt0);
    
    Eigen::Matrix<double, 2, 1> B = T3Dto2D * (pt1-pt0);
    Eigen::Matrix<double, 2, 1> C = T3Dto2D * (pt2-pt0);
    Eigen::Matrix<double, 2, 1> v1 = T3Dto2D * (ptcheck-pt0);
    
    Eigen::Matrix4d M;
//    if det(M)<0 - inside
//    if det(M)=0 - on the circle
//    if det(M)>0 - outside
    M(0,0) = v1.squaredNorm();
    M(1,0) = A.squaredNorm();
    M(2,0) = B.col(0).squaredNorm();
    M(3,0) = C.col(0).squaredNorm();
    
//    std::cout<<"B: "<<B<<std::endl;
//    std::cout<<"T3Dto2D * (pt1-pt0): "<<T3Dto2D * (pt1-pt0)<<std::endl;
//    std::cout<<"C: "<<C<<std::endl;
//    std::cout<<"v1: "<<v1<<std::endl;
    
    M(0,1) = v1(0); 
    M(1,1) = A(0);
    M(2,1) = B(0);
    M(3,1) = C(0);
    
    M(0,2) = v1(1);
    M(1,2) = A(1);
    M(2,2) = B(1);
    M(3,2) = C(1);

    M(0,3) = 1;
    M(1,3) = 1;
    M(2,3) = 1;
    M(3,3) = 1;
    
//    std::cout<<"3DPoints 1: "<<pt0<<" | "<<pt1<<" | "<<pt2<<" | "<<ptcheck<<std::endl;
//    std::cout<<"M: "<<M<<std::endl;
    
    
//    M = [v*v' p(1) p(2) 1; ...
//     A*A' A(1) A(2) 1; ...
//     B*B' B(1) B(2) 1; ...
//     C*C' C(1) C(2) 1];
    
    const double det = M.determinant();

//    std::cout<<"Det: "<<det<<std::endl;
//    exit(0);

    
    if(det > 0)
        return false;
    else
        return true;    
}




vtkIdType CoverRefiner::FindTriangleByPointIds( 
        vtkIdType id0, vtkIdType id1, vtkIdType id2)  { 
    
    vtkIdType retval;
    
    std::vector<vtkIdType> argumentIDs;
    argumentIDs.push_back(id0);
    argumentIDs.push_back(id1);
    argumentIDs.push_back(id2);
    std::sort(argumentIDs.begin(), argumentIDs.end());

    std::vector<vtkIdType> IDs;

//    std::cout<<"To find: "<<argumentIDs[0]<<" "<<argumentIDs[1]<<" "<<argumentIDs[2]<<std::endl;
    
    for(retval = 0; retval<m_coverFaces->size(); retval++)
    {
        IDs.clear();
        IDs.push_back(m_coverFaces->at(retval).id[0]);
        IDs.push_back(m_coverFaces->at(retval).id[1]);
        IDs.push_back(m_coverFaces->at(retval).id[2]);
        std::sort(IDs.begin(), IDs.end());

//        std::cout<<"Comparing to: "<<IDs[0]<<" "<<IDs[1]<<" "<<IDs[2]<<std::endl;

        
        std::vector<vtkIdType> v_intersection;
        std::set_intersection(argumentIDs.begin(), argumentIDs.end(),
                              IDs.begin(), IDs.end(),
                              std::back_inserter(v_intersection));         
        
        if(v_intersection.size()==3) //bingo
            break;
    }
    
    return retval;
}





void CoverRefiner::SaveIsolatedCover(const char* filename)  {
    vtkSmartPointer<vtkCellArray> coverCells = vtkSmartPointer<vtkCellArray>::New();
    for( HoleCoverType::const_iterator it = m_coverFaces->begin(); it!=m_coverFaces->end(); ++it )
    {
        coverCells->InsertNextCell(3);
        coverCells->InsertCellPoint((*it).id[0]);
        coverCells->InsertCellPoint((*it).id[1]);
        coverCells->InsertCellPoint((*it).id[2]);
    }
    
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    pd->SetPoints(m_coverVertices);
    pd->SetPolys(coverCells);
    
    vtkSmartPointer<vtkPolyDataWriter> wr = vtkSmartPointer<vtkPolyDataWriter>::New();
    wr->SetFileName(filename);
    wr->SetInputData(pd);
    wr->Write();
}
