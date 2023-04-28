/*************
 * Ellipse fitting algorithm from
 * https://github.com/mericdurukan/ellipse-fitting
 * 
 * Source code doesn't specify a license and needs to be downloaded
 * from the github-repo above. Download the file ellipse_fit.cpp and
 * place it under the sketch subfolder src/ellipse-fitting/ellipse-fit.cpp
 * 
 * check https://github.com/seisgo/EllipseFit which is MIT license
 * 
 ************/
#ifndef ELLIPSE_FIT_H
#define ELLIPSE_FIT_H

/******************************
 * eigen library used: Bolder_Flight_Systems_Eigen
 * 
 * edited eigen.h to import Dense in addition to Core
 * 
 * original eigen.h
 *   #define EIGEN_MPL2_ONLY
 *   #define EIGEN_NO_DEBUG 1
 *   #include "Eigen/Core"
 * add line
 *   #include "Eigen/Dense"
 ******************************/

#include "eigen.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace Eigen;

class ellipse_fit{
public:
    void set(vector<vector<double> > input);
    void fit(double& result_center_x, double& result_center_y, double& result_phi, double& result_width, double& result_hight);

private:
    vector<vector<double> >input_matrix;
    void take_input(vector<vector<double> > input);

};
#endif
