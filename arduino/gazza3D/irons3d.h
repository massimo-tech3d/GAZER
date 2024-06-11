/*
 * Ellipsoid fitting algorithm:
 * matlab version: http://www.mathworks.com/matlabcentral/fileexchange/24693-ellipsoid-fit
 * python port with nice plotting: https://github.com/aleksandrbazhin/ellipsoid_fit_python/tree/master
 * 
 * We added the refine_3D_fit function, absent from both repositories above, to complete a proper calibration.
 * 
 * Both implementation have the major underlying issue that eigen values and vectors are returned in random 
 * order. This make unreliable the subsequent points calibration. This is common to ALL eigenvectors based algorithms.
 * 
 * The issue is the following. The ellipsoid has 3 axes, each represented by an eigenvalue/vector pair.
 * Let's consider axis a be the shortest and axis c the longest.
 * Since you don't know a priori which eigen pair is related to axis a, b, or c, when applying calibration 
 * you may end up in stretching further axis c and shortening axis a, which would an even more oblate
 * ellipsoid rather than a sphere.
 * 
 * We therefore need to find the proper order of eigenvalue/vector pairs.
 * 
 * A criterion to do this is described in ST Design Tip DT0059 Ellipsoid or sphere fitting for sensor calibration.
 *
 * The refine_3D_fit implements this criterion.
 *
 *
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
 * Created by Massimo Tasso, April, 24, 2024 except where otherwise stated.
 * Released under GPLv3 License - see LICENSE file for details.
 */

#include "eigen.h"
#include <iostream>
#include <vector>
using namespace std;
using namespace Eigen;


void prettyPM(const MatrixXd& mat, String label) {
  int r = mat.rows();
  int c = mat.cols();

  Serial.print("Matrix ");Serial.print(label);Serial.print(" ");Serial.print(r);Serial.print("x");Serial.println(c);
  for(int i=0; i<r; i++) {
     Serial.print("[");
     for(int j=0; j<c; j++) {
        Serial.print(mat.coeff(i,j), 8);
        Serial.print(" ");
        if(j>10){
          Serial.print(" .......");
          break;
        }
     }
     Serial.println("]");
     if(i>10) {
       Serial.println(" .......");
       break;
     }
  }
}

void prettyPV(const Eigen::VectorXd& vec, String label) {
  int r = vec.rows();

  Serial.print("Vector ");Serial.print(label);Serial.print(" ");Serial.print(r);
  Serial.print(" [");
  for(int i=0; i<r; i++) {
    Serial.print(vec.coeff(i), 8);Serial.print(", ");
    if(i>10) {
      Serial.print(" .......");
      break;
    }
  }
  Serial.println("]");
}

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

vector<double> calibrate(vector<double>& mag, vector<vector<double>>& mat, vector<double>& center) {
    Vector3d data_centered;
    Matrix3d emat;
    Vector3d ecenter;
    emat << mat[0][0], mat[0][1], mat[0][2],
            mat[0][1], mat[1][1], mat[1][2],
            mat[0][2], mat[1][2], mat[2][2];
    ecenter << center[0], center[1], center[2];

    data_centered << mag[0] - ecenter(0), mag[1] - ecenter(1), mag[2] - ecenter(2);
    Vector3d data_on_sphere = emat * data_centered;
    return {data_on_sphere(0), data_on_sphere(1), data_on_sphere(2)};
}

// to be applied on evecs, before transposing
void refine_3D_fit(Vector3d& gain, Matrix3d& rotM) {
    // largest element should be on diagonal
    double m = 0;
    int rm = 0, cm = 0;
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (abs(rotM(r,c)) > m) {
                m = abs(rotM(r,c));
                rm = r;
                cm = c;
            }
        }
    }
    if (rm != cm) { // swap cols if not on diagonal
        swap(rotM(0,cm), rotM(0,rm));
        swap(rotM(1,cm), rotM(1,rm));
        swap(rotM(2,cm), rotM(2,rm));
        swap(gain(cm), gain(rm));
    }

    // do the same on remaining 2x2 matrix
    vector<int> i;
    switch (rm) {
        case 0: i = {1, 2}; break;
        case 1: i = {0, 2}; break;
        case 2: i = {0, 1}; break;
    }
    m = 0;
    rm = 0;
    cm = 0;
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 2; c++) {
            if (abs(rotM(i[r],i[c])) > m) {
                m = abs(rotM(i[r],i[c]));
                rm = i[r];
                cm = i[c];
            }
        }
    }
    if (rm != cm) { // swap cols if not on diagonal
        swap(rotM(0,cm), rotM(0,rm));
        swap(rotM(1,cm), rotM(1,rm));
        swap(rotM(2,cm), rotM(2,rm));
        swap(gain(cm), gain(rm));
    }

    // neg cols to make it positive along diagonal
    if (rotM.coeff(0,0) < 0) {
        rotM(0,0) = -rotM(0, 0);
        rotM(1,0) = -rotM(1, 0);
        rotM(2,0) = -rotM(2, 0);
    }
    if (rotM.coeff(1,1) < 0) {
        rotM(0,1) = -rotM(0,1);
        rotM(1,1) = -rotM(1,1);
        rotM(2,1) = -rotM(2,1);
    }
    if (rotM.coeff(2,2) < 0) {
        rotM(0,2) = -rotM(0,2);
        rotM(1,2) = -rotM(1,2);
        rotM(2,2) = -rotM(2,2);
    }
}

void ellipsoid_fit(vector<vector<double>>& mag, vector<vector<double>>& MAT, vector<double>& CENTER) {
    size_t N = mag.size();
    Vector3d center;
    Vector3d radii;
    Vector3d evals;
    Matrix3d evecs;
    MatrixXd D(9, N);
    VectorXd d2(N);
    double a, b, c;
    VectorXd u(9);
    VectorXd v(10);
    Matrix4d A;
    Matrix4d translation_matrix;
    Matrix4d R;
    
    double aa, bb, cc, rr;
    Matrix3d dd;
    Matrix3d emat;

    VectorXd x(N);  // input points x-values
    VectorXd y(N);  // input points y-values
    VectorXd z(N);  // input points z-values
    for(size_t k=0;k<N; k++){
        x(k) = mag[k][0];
        y(k) = mag[k][1];
        z(k) = mag[k][2];
    }

    VectorXd x_sq = x.cwiseProduct(x).eval();
    VectorXd y_sq = y.cwiseProduct(y).eval();
    VectorXd z_sq = z.cwiseProduct(z).eval();

    D.row(0) = x_sq + y_sq - 2 * z_sq;
    D.row(1) = x_sq + z_sq - 2 * y_sq;
    D.row(2) = 2 * x.cwiseProduct(y);
    D.row(3) = 2 * x.cwiseProduct(z);
    D.row(4) = 2 * y.cwiseProduct(z);
    D.row(5) = 2 * x;
    D.row(6) = 2 * y;
    D.row(7) = 2 * z;
    D.row(8) = Eigen::VectorXd::Ones(N);
    
    d2 = x_sq + y_sq + z_sq;

    u = (D * D.transpose()).ldlt().solve(D * d2);

    a = u.coeff(0) + u.coeff(1) - 1;
    b = u.coeff(0) - 2 * u.coeff(1) - 1;
    c = u.coeff(1) - 2 * u.coeff(0) - 1;

    v << a, b, c, u.tail(7);

    A << v[0], v[3], v[4], v[6],
         v[3], v[1], v[5], v[7],
         v[4], v[5], v[2], v[8],
         v[6], v[7], v[8], v[9];

    center = (-A.topLeftCorner(3, 3)).ldlt().solve(v(seq(6,8)));

    translation_matrix = Matrix4d::Identity();
    translation_matrix(3, 0) = center[0];
    translation_matrix(3, 1) = center[1];
    translation_matrix(3, 2) = center[2];

    R = translation_matrix * A * translation_matrix.transpose();

    SelfAdjointEigenSolver<Matrix3d> eigensolver(R.topLeftCorner(3, 3) / -R(3, 3));
    evals = eigensolver.eigenvalues();
    evecs = eigensolver.eigenvectors();
//    prettyPV(evals, "EVALS");
//    prettyPM(evecs, "EVECS");
    refine_3D_fit(evals, evecs);
//    prettyPV(evals, "EVALS optimized");
//    prettyPM(evecs, "EVECS optimized");

    radii(0) = sqrt(1 / abs(evals(0))) * sign(evals(0));
    radii(1) = sqrt(1 / abs(evals(1))) * sign(evals(1));
    radii(2) = sqrt(1 / abs(evals(2))) * sign(evals(2));

    aa = radii(0);
    bb = radii(1);
    cc = radii(2);
    rr = cbrt(aa*bb*cc);
    dd << rr/aa, 0, 0, 0, rr/bb, 0, 0, 0, rr/cc;

    emat = evecs * dd * evecs.transpose();

//    prettyPM(emat, "EMAT");
//    MatrixXd magc = calibrate(mag, center, emat);

    // casting results from eigen to std::vectors
    for(int i=0; i<3; i++) {
      for(int j=0; j<3; j++) {
//        MAT[i][j] = emat.coeff(i,j);
        MAT[i][j] = emat(i,j);
      }
    }
    CENTER[0] = center(0);
    CENTER[1] = center(1);
    CENTER[2] = center(2);
}
