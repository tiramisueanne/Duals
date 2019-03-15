#include <iostream>

#include "QuadraticSolver.h"

using namespace std;
using namespace Eigen;

namespace qp = quadprogpp;
#define DEBUG

double QuadraticSolver::updateVertices() {
    // Create the new thing to optimize, which is all the points of
    // the internal nodes
    int ZERO = 0;
    vec = qp::Vector<double>(ZERO, internalNodes.size() * V.cols());
    for (auto row : internalNodes) {
        for (int j = 0; j < V.cols(); j++) {
            vec[indr.indexBigVert(row, j)] = V(row, j);
        }
    }

    // Create the new zDiff struct
    qp::Matrix<double> zValues(ZERO, internalNodes.size(),
                               internalNodes.size() * V.cols());

    qp::Matrix<double> xyValues(ZERO, internalNodes.size() * 2,
                                internalNodes.size() * V.cols());

    qp::Vector<double> zConstant(ZERO, internalNodes.size());
    qp::Vector<double> xyConstant(ZERO, internalNodes.size() * 2);

    // Go through each edge and add weights
    for (const pair<pair<int, int>, double>& edge_weight : weightMap) {
        const pair<int, int>& edge = edge_weight.first;
        double weight = edge_weight.second;

        // If this is a row to constrain
        if (internalNodes.find(edge.first) != internalNodes.end()) {
            zValues[indr.indexVert(edge.first)]
                   [indr.indexBigVert(edge.first, ZDim)] += weight;
            xyValues[indr.indexVert(edge.first) * 2]
                    [indr.indexBigVert(edge.first, XDim)] += weight;
            xyValues[indr.indexVert(edge.first) * 2 + 1]
                    [indr.indexBigVert(edge.first, YDim)] += weight;

        } else {
            continue;
        }

        // If this will be affected by our optimized nodes
        if (internalNodes.find(edge.second) != internalNodes.end()) {
            zValues[indr.indexVert(edge.first)]
                   [indr.indexBigVert(edge.second, ZDim)] -= weight;
            xyValues[indr.indexVert(edge.first) * 2]
                    [indr.indexBigVert(edge.second, XDim)] -= weight;
            xyValues[indr.indexVert(edge.first) * 2 + 1]
                    [indr.indexBigVert(edge.second, YDim)] -= weight;

        } else {
            zConstant[indr.indexVert(edge.first)] -=
                weight * V.row(edge.second).z();
            xyConstant[indr.indexVert(edge.first) * 2] -=
                weight * V.row(edge.second).x();
            xyConstant[indr.indexVert(edge.first) * 2 + 1] -=
                weight * V.row(edge.second).y();
        }
    }

    // For all forces, go through and add to zValues
    for (int i = 0; i < forces.ncols(); i++) {
        // TODO: this requires no indexing right now, because it's all constants
        zConstant[i] += forces[0][i];
    }

    // Our quadratic var
    qp::Matrix<double> vecToPass(ZERO, vec.size(), vec.size());
    for (int i = 0; i < vecToPass.nrows(); i++) {
        vecToPass[i][i] += 1;
    }
    vecToPass *= 2;

    qp::Vector<double> linearComp = vec *= -2;
    vec /= -2;
#ifdef DEBUG
#ifdef VERBOSE
    cout << "vec used to be ";
    for (int i = 0; i < vec.size(); i++) {
        cout << vec[i] << endl;
    }
    cout << "linear comp used to be";
    for (int i = 0; i < vec.size(); i++) {
        cout << linearComp[i] << endl;
    }
    cout << "The zValues are " << endl;
    for (int i = 0; i < zValues.nrows(); i++) {
        for (int j = 0; j < zValues.ncols(); j++) {
            cout << zValues[i][j] << " ";
        }
        cout << endl;
    }
    cout << "The xyValues are" << endl;
    for (int i = 0; i < xyValues.nrows(); i++) {
        for (int j = 0; j < xyValues.ncols(); j++) {
            cout << xyValues[i][j] << " ";
        }
        cout << endl;
    }
    cout << "The xyConsts are " << endl;
    for (int i = 0; i < xyConstant.size(); i++) {
        cout << xyConstant[i] << " ";
    }
#endif
    cout << endl;
    qp::Vector<double> response = (dot_prod(zValues, vec));
    cout << "The before value was:\n";
    for (int i = 0; i < response.size(); i++) {
        cout << response[i] << "and the zConst was" << zConstant[i] << endl;
    }
    qp::Vector<double> responseXY = (dot_prod(xyValues, vec));
    cout << "The before value for xy was:\n";
    for (int i = 0; i < response.size(); i++) {
        cout << responseXY[2 * i] << "and the xConst was" << xyConstant[2 * i]
             << endl;
        cout << " while the yResp was" << responseXY[2 * i + 1]
             << "and the yConst was " << xyConstant[2 * i + 1] << endl;
    }
    cout << "The value of vec was " << vec << endl;
#endif
    // xyConstant should be =, while zValue can be >=
    double success = qp::solve_quadprog(vecToPass, linearComp, t(xyValues),
                                        xyConstant, t(zValues), zConstant, vec);

#ifdef DEBUG

    cout << "The value of vec is now " << vec << endl;
    cout << "The success value of updating was " << success << endl;

    response = (dot_prod(zValues, vec));
    cout << "The response value was:\n";
    for (int i = 0; i < response.size(); i++) {
        cout << response[i] << "and the zConst was" << zConstant[i] << endl;
    }
    responseXY = (dot_prod(xyValues, vec));
    cout << "The response value for xy was:\n";
    for (int i = 0; i < response.size(); i++) {
        cout << responseXY[2 * i] << "and the xConst was" << xyConstant[2 * i]
             << endl;
        cout << " while the yResp was" << responseXY[2 * i + 1]
             << "and the yConst was " << xyConstant[2 * i + 1] << endl;
    }
#endif
    moveVecIntoV();
    return success;
}

void QuadraticSolver::moveVecIntoV() {
    for (auto row : internalNodes) {
        for (int j = 0; j < V.cols(); j++) {
            V(row, j) = vec[indr.indexBigVert(row, j)];
        }
    }
}
