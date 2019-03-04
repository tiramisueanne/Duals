#pragma once
#include <igl/HalfEdgeIterator.h>
#include <igl/triangle_triangle_adjacency.h>
#include <Eigen/Dense>
#include <iostream>
#include <map>
#include <queue>

using namespace std;
using namespace Eigen;

MatrixXd getNewDual(Eigen::MatrixXd &V, Eigen::MatrixXi &F,
                    map<pair<int, int>, double> weights) {
    // Adjacency matrix
    // TT | F | x 3 matrix where (i,j) is index of face that is adjacent
    // to triangle i on edge j
    // TTi | F | x 3 matrix wehere (i, j) is index of edge of triangle
    // TT(i,j) that is adjacent to triangle i
    Eigen::MatrixXi TT;
    Eigen::MatrixXi TTi;
    igl::triangle_triangle_adjacency(F, TT, TTi);

    // The first face's vertex can be assigned at random,
    // here to the origin
    MatrixXd newVerts(F.rows(), 3);
    int firstFace = 0;
    newVerts.row(firstFace) << 0, 0, 0;

    // This will be the faces that we have already processed.
    set<int> filledFaces;
    queue<int> facesToFill;
    facesToFill.push(firstFace);

    MatrixXd turnLeft(3, 3);
    turnLeft << 0, 1, 0, -1, 0, 0, 0, 0, 1;

    // while we still have faces to fill, fill them!
    while (!facesToFill.empty()) {
        int curFace = facesToFill.front();
        facesToFill.pop();
        if (filledFaces.find(curFace) != filledFaces.end()) {
            continue;
        } else {
            // For each edge in triangle, turn 90 degrees to left
            // and find new centers from these.
            // This only turns the correct way due to all faces being
            // clockwise
            RowVector3i faceVerts = F.row(curFace);
            MatrixXd curFaceDualCenter = newVerts.row(curFace);
            for (int vert = 0; vert < 3; vert++) {
                int adjFace = TT(curFace, vert);
                // If there is no adjacent face along this edge
                if (adjFace == -1) {
                    continue;
                }
                int nextVert = (vert + 1) % 3;
                // Currently a row vector, but we need it to be col
                MatrixXd edge = V.row(nextVert) - V.row(vert);
                // Scale and turn left
                Vector3d leftEdge = turnLeft * edge.transpose().col(0) *
                                    weights.at(pair<int, int>(vert, nextVert));

                MatrixXd adjFaceDualCenter =
                    leftEdge + curFaceDualCenter.transpose().col(0);
                RowVector3d adjFaceRow = adjFaceDualCenter.transpose().row(0);
                newVerts.row(adjFace) = adjFaceRow;
                facesToFill.push(adjFace);
            }
            filledFaces.insert(curFace);
        }
    }
    if (filledFaces.size() != F.rows()) {
        cerr << "The filledFaces did not have all the faces!" << endl;
        throw new exception();
    }
    return newVerts;
}