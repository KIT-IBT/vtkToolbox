/*=========================================================================
Copyright (c) Constantine Butakoff
All rights reserved.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.
=========================================================================*/
#include "SurfaceHoleFiller.h"
#include "UmbrellaWeightedOrder2Smoother.h"
#include "CoverRefiner.h"

#include <vtkCell.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>
#include <vtkDataSetAttributes.h>
#include <vtkCellData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkDataWriter.h>
#include <vtkWriter.h>
#include <vtkAlgorithm.h>
#include <vtkAdjacentVertexIterator.h>


#include <iostream>
#include <limits>
#include <cmath>
#include <float.h>



#include <stack>
#include <vtkSmartPointer.h>
#include <vtkType.h>

//To create initial cover, by default the triangle areas augmented by dihedral angles are used.
//Uncomment this to return to just triangle areas. Not recommended, becasue sometimes the cover can become crap
//see the paper for explanation
//#define USE_TRIANGLEAREA_WEIGHT



void SurfaceHoleFiller::Update()
{
    vtkSmartPointer<vtkPolyDataNormals> normal_gen = vtkSmartPointer<vtkPolyDataNormals>::New();
    normal_gen->SetInputData(this->m_inputMesh);
    normal_gen->SplittingOff();
    normal_gen->ConsistencyOn();
    normal_gen->Update();

    m_outputMesh = normal_gen->GetOutput();


    HoleBoundaryType unordered_edges;
    FindHoles(m_outputMesh, unordered_edges);

    ArrayOfBoundariesType hole_boundaries;
    SplitHoles(unordered_edges, hole_boundaries);


//    std::cout << "Found " << hole_boundaries.size() << " holes" << std::endl;

    
    //fill the holes
    ArrayOfCoversType covers(hole_boundaries.size());

    for (int i = 0; i < hole_boundaries.size(); i++) {
        std::cout << "Filling hole " << i << "/" << hole_boundaries.size() << std::endl;

//                std::cout<<"Boundary "<<i<<std::endl;
//                for(int j=0; j<hole_boundaries[i].size(); j++)
//                {
//                    std::cout<<hole_boundaries[i][j].v0<<" "<<hole_boundaries[i][j].v1<<std::endl;
//                }

        HoleCoverType cover;

        vtkSmartPointer<vtkPolyData> refinedCover = vtkSmartPointer<vtkPolyData>::New();
        
        try
        {
            InitialCoverTriangulation(m_outputMesh, hole_boundaries.at(i), cover);
        }
        catch(...)
        {
            std::cout<<"Some exception"<<std::endl;
        }
        
//        std::cout<<"Initial cover"<<std::endl;
//        for(HoleCoverType::iterator it = cover.begin(); it!=cover.end(); it++)
//            std::cout<<(*it).id[0]<<" "<<(*it).id[1]<<" "<<(*it).id[2]<<std::endl;
        
        //updates the mesh inside
        RefineCover(m_outputMesh, hole_boundaries.at(i), cover);

//            vtkSmartPointer<vtkPolyDataWriter> wr = vtkSmartPointer<vtkPolyDataWriter>::New();
//            wr->SetFileName("closed.vtk");
//            wr->SetInputData(m_outputMesh);
//            wr->Write();

    }

}



//======================================================================
//
//
//      Other functions
//
//
//
//======================================================

void SurfaceHoleFiller::FindHoles(vtkPolyData *mesh, HoleBoundaryType& unordered_edges) const {

    const vtkIdType nPts = mesh->GetNumberOfPoints();
    SparseIDMatrixType adj(nPts, nPts); //adjacency matrix, counts number of times an edge appears
    
    unordered_edges.clear();

    for (vtkIdType cellid = 0; cellid < mesh->GetNumberOfCells(); cellid++) {
        vtkCell* cell = mesh->GetCell(cellid);

        //std::cout<<cellid<<": "<<cell->GetPointId(0)<<" "<<cell->GetPointId(1)<<" "<<cell->GetPointId(2)<<std::endl;

        for (vtkIdType edgeid = 0; edgeid < cell->GetNumberOfEdges(); edgeid++) {
            vtkCell* edge = cell->GetEdge(edgeid);
            const vtkIdType pt0 = edge->GetPointId(0);
            const vtkIdType pt1 = edge->GetPointId(1);
            if (pt0 < pt1) //to conserve memory due to symmetry
                adj.coeffRef(pt0, pt1) += 1;
            else
                adj.coeffRef(pt1, pt0) -= 1; //to keep track of orientation
        }
    }

    //std::cout << adj << std::endl;
    //iterate over every edge that adj(i,j)==1, these are  the boundaries

    for (vtkIdType k=0; k<adj.outerSize(); ++k) {        
        for (SparseIDMatrixType::InnerIterator it(adj,k); it; ++it)
            if ( it.value() != 0) {
                EdgeType edge;

                if ( it.value() > 0) {
                    edge.v0 = it.row();
                    edge.v1 = it.col();
                } else //orientation was reversed
                {
                    edge.v0 = it.col();
                    edge.v1 = it.row();
                }

                //                double n[3];
                //                mesh->GetPointData()->GetNormals()->GetTuple(edge.v0,n);
                //                edge.n0[0] = n[0];
                //                edge.n0[1] = n[1];
                //                edge.n0[2] = n[2];
                //                mesh->GetPointData()->GetNormals()->GetTuple(edge.v1,n);
                //                edge.n1[0] = n[0];
                //                edge.n1[1] = n[1];
                //                edge.n1[2] = n[2];
                unordered_edges.push_back(edge);
                //                cout << "(" << i2.index1() << "," << i2.index2()
                //                     << ":" << *i2 << ")  "<< endl;
            }

    }
}

void SurfaceHoleFiller::SplitHoles(HoleBoundaryType& unordered_edges, ArrayOfBoundariesType& hole_boundaries) const {
    std::vector<bool> visited_edges(unordered_edges.size(), false);

    for (vtkIdType edgeid = 0; edgeid < unordered_edges.size(); edgeid++) {
        if (!visited_edges.at(edgeid)) {
            const EdgeType &edgec = unordered_edges.at(edgeid);

            vtkIdType front = edgec.v1; //front vertex in the queue

            visited_edges.at(edgeid) = true;

            HoleBoundaryType bdry;
            bdry.push_back(edgec);

            //            for(int k=0; k<visited_edges.size(); k++)
            //                std::cout<<" | "<<k<<" : "<<visited_edges[k]?1:0;
            //            std::cout<<std::endl;

            vtkIdType j = edgeid;
            while (j < unordered_edges.size()) {
                //std::cout<<"checking edge "<<j<<" - "<<visited_edges[j]<<std::endl;
                if (!visited_edges.at(j)) {
                    const EdgeType &edge = unordered_edges[j];
                    if (edge.v0 == front) //if this edge back vertex matches current front vertex, add the edge
                    {
                        bdry.push_back(edge);
                        visited_edges.at(j) = true;
                        front = edge.v1;

                        j = edgeid; //reset counter
                    }
                }
                j++;
            }

            hole_boundaries.push_back(bdry);
        }
    }
}

void SurfaceHoleFiller::EdgesToVertexArray(const HoleBoundaryType& ordered_boundary, VertexIDArrayType& vertices) const {
    vertices.clear();

    if (ordered_boundary.empty()) {
        return; //nothing to do
    }

    //    vertices.push_back(ordered_boundary[0].v0);
    //    vertices.push_back(ordered_boundary[0].v1);
    //
    //    for (vtkIdType i = 1; i < ordered_boundary.size()-1; i++) {
    //        vertices.push_back(ordered_boundary[i].v1);
    //    }

    //save in reversed order, for correct triangle orientation
    vertices.push_back(ordered_boundary.at(ordered_boundary.size() - 2).v1);
    vertices.push_back(ordered_boundary.at(ordered_boundary.size() - 2).v0);

    for (vtkIdType i = ordered_boundary.size() - 3; i >= 0; i--) {
        vertices.push_back(ordered_boundary.at(i).v0);
    }



    //vertices.push_back(ordered_boundary[0].v0);
}

double SurfaceHoleFiller::TriangleWeightFunctionArea(const VectorType& u, const VectorType& v, const VectorType& w) const {
    //triangle area   
    return 0.5 * ((v - u).cross(w - u)).norm();
}

void SurfaceHoleFiller::InitialCoverTriangulation(vtkPolyData* mesh, HoleBoundaryType& ordered_boundary, HoleCoverType& cover) const {
    //create an array of vertices, this will create an array ending with vertex 0
    VertexIDArrayType vertices;
    EdgesToVertexArray(ordered_boundary, vertices);

    //print vertices
    //    std::cout<<"Vertices: ";
    //    for(int i=0; i<vertices.size(); i++)
    //    {   
    //        std::cout<<vertices[i]<<" ";
    //    }
    //    std::cout<<std::endl;

    //initialize some variables, consistently with th paper
    //n - number of vertices+1 (vn = v0)
    //number of unique vertices = number of edges
    vtkIdType n = vertices.size();

    //1. Create upper triangular weight matrix W
    //using namespace boost::numeric::ublas;

#ifdef USE_TRIANGLEAREA_WEIGHT    
    triangular_matrix<double, upper> W(n, n);
#else //otherwise use angle/area pair. Use complex just for convenience, 
    //but the operations will have to be redefined
    //triangular_matrix<AreaAngleMeasureType, upper> W(n, n);
    Eigen::SparseMatrix<AreaAngleMeasureType> W(n, n);
#endif


    //initialize weight matrix
    //for (int i = 0; i < n - 1; i++) W(i, i + 1) = 0;
    for (int i = 0; i < n - 1; i++) W.coeffRef(i, i + 1) = 0;

    VectorType vi, vi1, vi2, vm, vk;

    //create storage for the minimum ms
    TriangularIDMatrixType O(n, n);

    //fill O with -1 for debugging
    for (int i = 1; i < n; i++)
        for (int j = i + 1; j < n; j++)
            O.coeffRef(i, j) = -1;


    for (int i = 0; i < n - 2; i++) {
        mesh->GetPoint(vertices.at(i), vi.data());
        mesh->GetPoint(vertices.at(i + 1), vi1.data());
        mesh->GetPoint(vertices.at(i + 2), vi2.data());

#ifdef USE_TRIANGLEAREA_WEIGHT    
        W(i, i + 2) = TriangleWeightFunctionArea(vi, vi1, vi2);
#else
        //find the adjacent triangles on the original mesh
        const vtkIdType va1id = FindThirdVertexId(mesh, vertices.at(i), vertices.at(i + 1));
        const vtkIdType va2id = FindThirdVertexId(mesh, vertices.at(i + 1), vertices.at(i + 2));

        VectorType va1;
        VectorType va2;
        mesh->GetPoint(va1id, va1.data());
        mesh->GetPoint(va2id, va2.data());


        //initialize matrix 0 with the neighbors of the boundary elements
        O.coeffRef(i, i + 1) = va1id;
        O.coeffRef(i + 1, i + 2) = va2id;

        //triangle1: vi, vi1, vi2 -> correct orientation -> vi2, vi1, vi (the boundary orientation corresponds to triangles outside of the hole)
        //triangle2: vi, vi1, va1 - this is in the original mesh, so it has correct orientation
        //triangle3: vi1, vi2, va2 - this is in the original mesh, so it has correct orientation
        //
        //transform the dihedral angle cosines so that it is larger for larger angles (by multiplying by -1)
        //
        //dihedral angle with the triangle adjacent to edge (i, i+1)
        const double dih_angle1 = -CalculateDihedralAngleCos(vi, vi1, vi2, va1, vi1, vi);
        //dihedral angle with the triangle adjacent to edge (i+1, i+2)
        const double dih_angle2 = -CalculateDihedralAngleCos(vi, vi1, vi2, va2, vi2, vi1);

        //        std::cout<<"Angle 1 :"<<vertices[i]<<" "<<vertices[i+1]<<" "<<vertices[i+2]<<" vs "<<
        //                va1id<<" "<<vertices[i+1]<<" "<<vertices[i]<<std::endl;
        //        std::cout<<"Angle 2 :"<<vertices[i]<<" "<<vertices[i+1]<<" "<<vertices[i+2]<<" vs "<<
        //                va2id<<" "<<vertices[i+2]<<" "<<vertices[i+1]<<std::endl;
        //        std::cout<<"Angles: "<<dih_angle1<<" "<<dih_angle2<<std::endl;


        W.coeffRef(i, i + 2) = AreaAngleMeasureType(std::max(dih_angle1, dih_angle2),
                TriangleWeightFunctionArea(vi, vi1, vi2));

        O.coeffRef(i, i + 2) = i + 1;

#endif
    }

    //one more special case edge between first and last point
    O.coeffRef(0, n - 1) = FindThirdVertexId(mesh, vertices.at(0), vertices.at(n - 1));

    //==========================================
    //
    // !!!warning!!!!
    // Notice that for the O(i,i+1) for all i, including O(0, n-1) have the ids 
    // of the triangles in the original mesh, not the ids in the vertices array.
    // Every other element of O is actually an id in the vertices array.
    // I could not come up with a more elegant solution. Therefore there will be 
    // some "if" statements ahead


    //2. Look for minimal triangulation
    vtkIdType j = 2;


    while (j < n - 1) {
        j = j + 1;
        for (vtkIdType i = 0; i < n - j; i++) {
            vtkIdType k = i + j;

            //find smallest W(i,m)+W(m,k)+F(vi,vm,vk) for i<m<k
            vtkIdType m_min = 0;

#ifdef USE_TRIANGLEAREA_WEIGHT    
            double W_min = DBL_MAX;
#else
            AreaAngleMeasureType W_min = AreaAngleMeasureType(DBL_MAX, 0);
#endif

            for (vtkIdType m = i + 1; m < k; m++) {
                mesh->GetPoint(vertices.at(i), vi.data());
                mesh->GetPoint(vertices.at(m), vm.data());
                mesh->GetPoint(vertices.at(k), vk.data());

#ifdef USE_TRIANGLEAREA_WEIGHT    
                double Wik = W(i, m) + W(m, k) + TriangleWeightFunctionArea(vi, vm, vk);
#else

                //calculate dihedral angles of triangles 
                //(imk) vs (i,Oim,m)
                //(imk) vs (m,Omk,k)
                VectorType vOim;
                VectorType vOmk;

                //std::cout<<"Triangle "<<vertices[i]<<" "<<vertices[m]<<" "<<vertices[k]<<std::endl;

                //This is a bit messy
                if (abs(i - m) != 1) //nonconsecutive boundary vertices - O has id of the vertices array
                {
                    //std::cout<<"Oim:"<<vertices[i]<<" "<<vertices[O(i,m)]<<" "<<vertices[m]<<std::endl;
                    mesh->GetPoint(vertices.at(O.coeffRef(i, m)), vOim.data());
                } else //consecutive boundary vertices - O has the id of the mesh cell array
                {
                    //std::cout<<"Oim:"<<vertices[i]<<" "<<O(i,m)<<" "<<vertices[m]<<std::endl;
                    mesh->GetPoint(O.coeffRef(i, m), vOim.data()); //this vertex is not on the boundary, can't find elegant solution
                }

                if (abs(m - k) != 1) {
                    mesh->GetPoint(vertices.at(O.coeffRef(m, k)), vOmk.data());
                    //std::cout<<"Omk:"<<vertices[m]<<" "<<vertices[O(m,k)]<<" "<<vertices[k]<<std::endl<<std::flush;
                } else {
                    mesh->GetPoint(O.coeffRef(m, k), vOmk.data()); //this vertex is not on the boundary, can't find elegant solution
                    //std::cout<<"Omk:"<<vertices[m]<<" "<<O(m,k)<<" "<<vertices[k]<<std::endl<<std::flush;
                }

                //calculate dihedral angles from vertex coordinates
                const double dih_angle1 = -CalculateDihedralAngleCos(vi, vm, vk, vi, vOim, vm);
                double dih_angle2 = -CalculateDihedralAngleCos(vi, vm, vk, vm, vOmk, vk);
                //std::cout<<"Angles: "<<dih_angle1<<" "<<dih_angle2<<std::endl;

                if (i == 0 && k == n - 1) //these are consecutive vertices - O - has id within mesh cell array
                {
                    VectorType vOik;
                    //std::cout<<"Oik:"<<O(i,k)<<std::endl<<std::flush;

                    mesh->GetPoint(O.coeffRef(i, k), vOik.data());
                    const double dih_angle3 = -CalculateDihedralAngleCos(vi, vm, vk, vi, vOik, vk);

                    //std::cout<<"Last triangle:"<<vertices[i]<<" "<<O(i,k)<<" "<<vertices[k]<<std::endl<<std::flush;

                    dih_angle2 = std::max(dih_angle2, dih_angle3);
                }

                AreaAngleMeasureType Aimk(std::max(dih_angle1, dih_angle2), TriangleWeightFunctionArea(vi, vm, vk));

                //Calculate the combined weight as a sum of 
                //W(i, m) + W(m, k) + TriangleWeightFunctionArea(vi, vm, vk);  
                AreaAngleMeasureType Wik = SumAreaTriangleMeasures(
                        SumAreaTriangleMeasures(W.coeffRef(i, m), W.coeffRef(m, k)), Aimk);
#endif

#ifdef USE_TRIANGLEAREA_WEIGHT    
                if (Wik < W_min)
#else
                if (AreaTriangleMeasureLess(Wik, W_min))
#endif
                {
                    W_min = Wik;
                    m_min = m;
                }

            }

            W.coeffRef(i, k) = W_min;
            O.coeffRef(i, k) = m_min;
            //std::cout<<"("<<i<<","<<k<<") Best: "<<vertices[i]<<" "<<vertices[m_min]<<" "<<vertices[k]<<std::endl;
        }
    }

    //4. Create the covering mesh
    //vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    //copy the original cells
    //for (vtkIdType i = 0; i < mesh->GetNumberOfCells(); i++)
    //    cells->InsertNextCell(mesh->GetCell(i));

    //create new cells
    PopulateCover(cover, 0, n - 1, O, vertices);

    //for(vtkIdType j=0; j<cover.size(); j++)
    //{
    //    std::cout<<"Verifying: "<<cover[j].id[0]<<" "<<cover[j].id[1]<<" "<<cover[j].id[2]<<std::endl;
    //}


    //Add the cover to the mesh
    //mesh->SetPolys(cells);
    //mesh->GetCellData()->Initialize();
    //mesh->BuildCells();
}

void SurfaceHoleFiller::PopulateCover(HoleCoverType& cover, vtkIdType i, vtkIdType k, 
        const TriangularIDMatrixType& O, const VertexIDArrayType& vertices) const {
    if (i + 2 == k) {
        //cover->InsertNextCell(3);
        //cover->InsertCellPoint(vertices[i]); //readjusted the orientation wrt the paper
        //cover->InsertCellPoint(vertices[i + 1]);
        //cover->InsertCellPoint(vertices[k]);
        TriangleCellType triangle;
        triangle.id[0] = vertices.at(i);
        triangle.id[1] = vertices.at(i + 1);
        triangle.id[2] = vertices.at(k);
        cover.push_back(triangle);
        //std::cout<<"Adding: "<<vertices[i]<<" "<<vertices[i+1]<<" "<<vertices[k]<<std::endl;
    } else {
        vtkIdType o = O.coeff(i, k);
        if (o != i + 1)
            PopulateCover(cover, i, o, O, vertices);

        //cover->InsertNextCell(3);
        //cover->InsertCellPoint(vertices[i]); //readjusted the orientation wrt the paper
        //cover->InsertCellPoint(vertices[o]);
        //cover->InsertCellPoint(vertices[k]);
        TriangleCellType triangle;
        triangle.id[0] = vertices.at(i);
        triangle.id[1] = vertices.at(o);
        triangle.id[2] = vertices.at(k);
        cover.push_back(triangle);
        //std::cout<<"Adding: "<<vertices[i]<<" "<<vertices[o]<<" "<<vertices[k]<<std::endl;

        if (o != k - 1)
            PopulateCover(cover, o, k, O, vertices);

    }
}



//make sure the vertices have correct order\
//https://en.wikipedia.org/wiki/Dihedral_angle

double SurfaceHoleFiller::CalculateDihedralAngleCos(const VectorType& v1, const VectorType& v2, const VectorType& v3,
        const VectorType& u1, const VectorType& u2, const VectorType& u3) const {
    const VectorType n1 = ((v3 - v2).cross(v1 - v2)).normalized();
    const VectorType n2 = ((u3 - u2).cross(u1 - u2)).normalized();

    //    std::cout<<"V1: "<<v1(0)<<" "<<v1(1)<<" "<<v1(2)<<" "<<std::endl;
    //    std::cout<<"V2: "<<v2(0)<<" "<<v2(1)<<" "<<v2(2)<<" "<<std::endl;
    //    std::cout<<"V3: "<<v3(0)<<" "<<v3(1)<<" "<<v3(2)<<" "<<std::endl;
    //    std::cout<<"U1: "<<u1(0)<<" "<<u1(1)<<" "<<u1(2)<<" "<<std::endl;
    //    std::cout<<"U2: "<<u2(0)<<" "<<u2(1)<<" "<<u2(2)<<" "<<std::endl;
    //    std::cout<<"U3: "<<u3(0)<<" "<<u3(1)<<" "<<u3(2)<<" "<<std::endl;
    //    std::cout<<"n1: "<<n1(0)<<" "<<n1(1)<<" "<<n1(2)<<" "<<std::endl;
    //    std::cout<<"n2: "<<n2(0)<<" "<<n2(1)<<" "<<n2(2)<<" "<<std::endl;

    return n1.dot(n2);
}


//this will be exected only for boundaries, so there is only 1 triangle with the edge (p1,p2)

vtkIdType SurfaceHoleFiller::FindThirdVertexId(vtkPolyData* mesh, vtkIdType p1, vtkIdType p2) const {
    vtkSmartPointer<vtkIdList> n = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPointCells(p1, n);

    vtkIdType cell_id;
    vtkIdList* ptids;

    for (vtkIdType i = 0; i < n->GetNumberOfIds(); i++) {
        cell_id = n->GetId(i);
        ptids = mesh->GetCell(cell_id)->GetPointIds();

        for (vtkIdType j = 0; j < ptids->GetNumberOfIds(); j++) {
            if (p2 == ptids->GetId(j)) //found the cell sharing 2 points
            {
                j = ptids->GetNumberOfIds();
                i = n->GetNumberOfIds();
            }
        }

    }

    //cell_id has the identified cell id;
    //ptids - has ids of all the cell points
    vtkIdType thirdid = VTK_ID_MAX;
    for (vtkIdType i = 0; i < ptids->GetNumberOfIds(); i++) {
        thirdid = ptids->GetId(i);
        if (thirdid != p1 && thirdid != p2)
            break;
    }

    return thirdid;
}





//does m1+m2, as described in the paper
//(a,b)+(c,d) = (max(a,c),b+d)

AreaAngleMeasureType SurfaceHoleFiller::SumAreaTriangleMeasures(const AreaAngleMeasureType& m1,
        const AreaAngleMeasureType& m2) const {
    return AreaAngleMeasureType(std::max(std::real(m1), std::real(m2)),
            std::imag(m1) + std::imag(m2));
}

//checks if m1 < m2, lexicographically
//(a,b)<(c,d) iff (a<c or (a==c and b<d))

bool SurfaceHoleFiller::AreaTriangleMeasureLess(const AreaAngleMeasureType& m1,
        const AreaAngleMeasureType& m2) const {

    const double a = std::real(m1);
    const double b = std::imag(m1);
    const double c = std::real(m2);
    const double d = std::imag(m2);

    return (a < c || (a == c && b < d));
}




void SurfaceHoleFiller::SaveIsolatedCover(const HoleCoverType& localCover, vtkPoints * coverVertices, const char* filename) const {
    vtkSmartPointer<vtkCellArray> coverCells = vtkSmartPointer<vtkCellArray>::New();
    for( HoleCoverType::const_iterator it = localCover.begin(); it!=localCover.end(); ++it )
    {
        coverCells->InsertNextCell(3);
        coverCells->InsertCellPoint((*it).id[0]);
        coverCells->InsertCellPoint((*it).id[1]);
        coverCells->InsertCellPoint((*it).id[2]);
    }
    
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    pd->SetPoints(coverVertices);
    pd->SetPolys(coverCells);
    
    vtkSmartPointer<vtkPolyDataWriter> wr = vtkSmartPointer<vtkPolyDataWriter>::New();
    wr->SetFileName(filename);
    wr->SetInputData(pd);
    wr->Write();
}







void SurfaceHoleFiller::IsolateCover(const HoleCoverType& cover, VertexIDArrayType& boundaryVertexIDs, HoleCoverType& localCover) const{
    for( HoleCoverType::const_iterator it = cover.begin(); it!=cover.end(); ++it )
    {
        TriangleCellType cell; 
        
        
        
        //for each id, find its position within boundaryVertexIDs
        for(int j=0; j<3; j++)
        {
            vtkIdType i;
            for(i=0; i<boundaryVertexIDs.size(); i++)
                if( (*it).id[j] == boundaryVertexIDs.at(i) ) break;
            
            cell.id[j] = i;
        }
        
        localCover.push_back(cell);
        
        if( CheckForDuplicateTriangles(localCover) )
        {
            for( HoleCoverType::const_iterator it1=localCover.begin(); it1!=localCover.end(); it1++  )
            {                
                std::cout<<(*it1).id[0]<<" "<<(*it1).id[1]<<" "<<(*it1).id[2]<<" "<<std::endl;
            }
        }
    }
}

void SurfaceHoleFiller::RefineCover(vtkPolyData* mesh, const HoleBoundaryType& ordered_boundary, 
        const HoleCoverType& cover) const {

    //create vertex storage
    VertexIDArrayType boundaryVertexIDs;
    EdgesToVertexArray(ordered_boundary, boundaryVertexIDs);

    
    
    if( CheckForDuplicateTriangles(cover) )
     {
         for( HoleCoverType::const_iterator it1=cover.begin(); it1!=cover.end(); it1++  )
         {                
             std::cout<<(*it1).id[0]<<" "<<(*it1).id[1]<<" "<<(*it1).id[2]<<" "<<std::endl;
         }
     }
            
    //Relabel the ids of the cover to ids from 0 to n_vertices
    //the correspondence with the original is given by
    //i -> boundaryVertexIDs[i]
    HoleCoverType localCover;
    IsolateCover(cover, boundaryVertexIDs, localCover);
    
    
    //copy boundary vertices to point storage
    vtkSmartPointer<vtkPoints> coverVertices = vtkSmartPointer<vtkPoints>::New();
    for(vtkIdType i=0; i<boundaryVertexIDs.size(); i++)
    {
        coverVertices->InsertNextPoint(mesh->GetPoint( boundaryVertexIDs.at(i) ));
    }

    //Save the cover for debugging
//    char name[100];
//    sprintf(name,"initial_cover%d.vtk",cover_id++);
//    SaveIsolatedCover(localCover, coverVertices, name);
            
    
  
    
    
    //create the array with ids for the edge. 
    //I created the cover so that the edge vertices are at the beginning of the 
    //vertex array and are ordered. So this array is simply 1:number_of_edge_vertices
    //ps. boundaryVertexIDs - ids in the original mesh, can't use here    
    VertexIDArrayType local_boundary;
    for(vtkIdType id = 0; id<boundaryVertexIDs.size(); id++)
        local_boundary.push_back(id);
    
//    SaveIsolatedCover(localCover, coverVertices, "beforeRefine.vtk");
            
    
    try
    {
        CoverRefiner refiner;
        refiner.SetInputBoundaryIds(&local_boundary);
        refiner.SetInputFaces(&localCover); 
        refiner.SetInputVertices(coverVertices); 
        refiner.InitializeVertexWeights(mesh, &boundaryVertexIDs);
        refiner.Update(); //modifies inputs
    }
    catch(...)
    {
        std::cout<<"Some exception happened during call to refiner."<<std::endl;
        throw;
    }
    
//    SaveIsolatedCover(localCover, coverVertices, "afterRefine.vtk");
    
   
    
    
    //---------------------------------------------------------
    //
    // 
    // Run cover smoothing
    //
    //
    //---------------------------------------------------------
    

    
    
    
//    std::cout<<"Copying final refined cover"<<std::endl;

    vtkCellArray* mesh_cells = m_outputMesh->GetPolys();
    vtkPoints* mesh_points = m_outputMesh->GetPoints();
    
    const vtkIdType nOriginalVertices = m_outputMesh->GetNumberOfPoints();
   
    //insert new points, skip old ones, they are in the beginning
    for( vtkIdType ptid = boundaryVertexIDs.size(); ptid<coverVertices->GetNumberOfPoints(); ptid++)
    {
        const vtkIdType id = mesh_points->InsertNextPoint( coverVertices->GetPoint(ptid) );
        boundaryVertexIDs.push_back(id);
    }
    
    for( HoleCoverType::const_iterator it = localCover.begin(); it!=localCover.end(); ++it )
    {
        //TriangleCellType cell; 
        
        //for each id, find its position within boundaryVertexIDs
        mesh_cells->InsertNextCell(3);
        for(int j=0; j<3; j++)
        {
            const vtkIdType local_id = (*it).id[j];
            
            const vtkIdType targetId = boundaryVertexIDs.at( local_id );

            mesh_cells->InsertCellPoint( targetId );
            
//            std::cout<<"Id "<<local_id<<" converted to "<<targetId<<std::endl;
        }
        
        //refinedCover.push_back(cell);
    }    
    
    m_outputMesh->GetCellData()->Initialize();
    m_outputMesh->GetPointData()->Initialize();
    m_outputMesh->BuildCells();
    


    if( m_performCoverSMoothing ){
        std::cout<<"Smoothing the cover"<<std::endl;


        UmbrellaWeightedOrder2Smoother smoother;
        smoother.SetInputMesh(m_outputMesh);
        smoother.SetCoverVertexIds(&boundaryVertexIDs);
        smoother.SetEdgeWeightingType(m_weightingType);
        smoother.Update();
    }

    
    
//    vtkSmartPointer<vtkPolyDataWriter> wr = vtkSmartPointer<vtkPolyDataWriter>::New();
//    wr->SetFileName("integrated.vtk");
//    wr->SetInputData(m_outputMesh);
//    wr->Write();
}















bool SurfaceHoleFiller::CheckForDuplicateTriangles(const HoleCoverType& localCover) const
{
    for( HoleCoverType::const_iterator it1=localCover.begin(); it1!=localCover.end(); it1++  )
    {
        for( HoleCoverType::const_iterator it2=localCover.begin(); it2!=localCover.end(); it2++  )
        {
            if (it1==it2) continue;
            for(int i=0; i<3; i++)
            {
                if((*it1).id[0]==(*it2).id[i])
                {
                    for(int j=0; j<3; j++)
                    {
                        if((*it1).id[1]==(*it2).id[j])
                        {
                            for(int k=0; k<3; k++)
                            {
                                if((*it1).id[2]==(*it2).id[k])
                                {
                                    std::cout<<"Duplicate triangle found: "<<(*it1).id[0]<<" "<<(*it1).id[1]<<" "<<(*it1).id[2]<<
                                        " and "<<(*it2).id[0]<<" "<<(*it2).id[1]<<" "<<(*it2).id[2]<<std::endl;
                                    return true;
                                }
                            }                    
                        }
                    }                    
                }
            }
        }
        
    }
    
    return false;
}
