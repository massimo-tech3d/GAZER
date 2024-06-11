/*
 * 
 * Ellipse fitting algorithm explained here
 * http://www.juddzone.com/ALGORITHMS/least_squares_ellipse.html
 *
 * given a set of 2D magnetometer readings, calculates the best approximating ellipse and returns
 * the coefficients of its equation: Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
 * from which it's easy to calculate:
 *      the center offset (Herd Iron)
 *      and the ratio of the two axes and their rotation angle (complete 2D Soft Iron)
 *
 * Dependency on Arduino library: Bolder_Flight_Systems_Eigen
 * It is necessary to edit the file eigen.h to import Dense in addition to Core
 * 
 * Just append
 * 
 * #include "Eigen/Dense"
 * 
 * to the header file eigen.h
 *
 * NOTE: the requirement for eigen library poses limitations on the Arduino platform: ARM cpu is required
 *       it would be necessary to offload the irons calculation to an external platform in order to use an 
 *       AVR Arduino
 *
 * Created by Massimo Tasso, January, 1, 2023 except where otherwise stated.
 * Released under GPLv3 License - see LICENSE file for details.
 */

#include "eigen.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace Eigen;

void irons2D(vector<vector<double> > points, double& center_x, double& center_y, double& phi, double& axis_a, double& axis_b){
    size_t size_mat = points.size();

    double a, b, c, d, e, f;             // fitted ellipse equation parameters
    Eigen::VectorXd X_Values(size_mat);  // input points x-values
    Eigen::VectorXd Y_Values(size_mat);  // input points y-values
    Eigen::MatrixXd D1(size_mat,3);
    Eigen::MatrixXd D2(size_mat,3);
    Eigen::MatrixXd S1(3,3);
    Eigen::MatrixXd S2(3,3);
    Eigen::MatrixXd S3(3,3);
    Eigen::MatrixXd C1(3,3);
    Eigen::MatrixXd M;
    Eigen::MatrixXd eigen_v;
    Eigen::VectorXd eig_row0;
    Eigen::VectorXd eig_row1;
    Eigen::VectorXd eig_row2;
    Eigen::VectorXd cond;
    Eigen::VectorXd min_pos_eig;
    Eigen::VectorXd cont_matrix;

    for(size_t k=0;k<size_mat; k++){
        X_Values(k) = points[k][0];
        Y_Values(k) = points[k][1];
    }

    D1.col(0) = X_Values.array().pow(2);
    D1.col(1) = X_Values.array() * Y_Values.array();
    D1.col(2) = Y_Values.array().pow(2);

    D2.col(0) = X_Values;
    D2.col(1) = Y_Values;
    D2.col(2) = Eigen::VectorXd::Ones(size_mat);

    S1 = D1.transpose() * D1;
    S2 = D1.transpose() * D2;
    S3 = D2.transpose() * D2;

    C1 << 0,0,2,0,-1,0,2,0,0;

    M = C1.inverse() * (S1 - S2*S3.inverse() * S2.transpose());

    Eigen::EigenSolver<MatrixXd> s(M);
    eigen_v = s.eigenvectors().real();

    eig_row0 = eigen_v.row(0);
    eig_row1 = eigen_v.row(1);
    eig_row2 = eigen_v.row(2);
    cond = 4 * (eig_row0.array() * eig_row2.array()) - eig_row1.array().pow(2);
    for(int i= 0; i<3 ; i++){
        if(cond(i) >0){
            min_pos_eig = eigen_v.col(i);
            break;
        }
    }
    Eigen::VectorXd cont_matrix1;
    cont_matrix = -1 * S3.inverse() * S2.transpose() * min_pos_eig;

    a = min_pos_eig(0);
    b = min_pos_eig(1)/2;
    c = min_pos_eig(2);
    d = cont_matrix(0)/2;
    e = cont_matrix(1)/2;
    f = cont_matrix(2);

    center_x = (c*d-b*e)/(b*b-a*c);
    center_y = (a*e-b*d)/(b*b-a*c);

    double num = 2*(a*e*e+c*d*d+f*b*b-2*b*d*e-a*c*f);
    double denom_1 = (b*b-a*c)*((c-a)*sqrt(1+4*b*b/((a-c)*(a-c)))-(c+a));
    double denom_2 = (b*b-a*c)*((a-c)*sqrt(1+4*b*b/((a-c)*(a-c)))-(c+a));

    axis_a = 2 * sqrt(num/denom_1);
    axis_b = 2 * sqrt(num/denom_2);
    phi = 0.5*atan((2*b)/(a-c));
}
