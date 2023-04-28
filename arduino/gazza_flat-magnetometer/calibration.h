/************
 * Magnetometer calibration algorithm
 *  
 * Created by Massimo Tasso, January, 1, 2023
 * 
 * Ellipse fitting algorithm from
 * https://github.com/mericdurukan/ellipse-fitting
 * 
 * Created by Massimo Tasso, January, 1, 2023
 * Released under GPLv3 License - see LICENSE file for details.
 ************/
#include "ellipse_fit.h"
//#include <vector>
//#include "vectors.h"

#define SAMPLES 500  // 1000
#define NUMBEROFAXIS 3

#define TO_DEG 180.0/PI

/********** ELLIPSE STUFF FOR PLANAR CALIBRATION ********/
vector<vector<double>> samples;
int last_entry = 0;

double alfa = 1;   // ellipse -  x axis
double beta = 1;   // ellipse -  y axis
double sigma = 1;  // ellipse -  scale factor

float R[4] = {1, 0, 0, 1};
float RI[4] = {1, 0, 0, 1};

ellipse_fit ellipse;

void init_cal() {
  samples.resize(SAMPLES , vector<double>(2, 0));
}

long init_cal(int duration) {
  samples.resize(SAMPLES , vector<double>(2, 0));
  return (long)(duration / SAMPLES);
}

void printSoftIron();

/**
 * Ellipse fitting function
 * 
 * uses the global samples array of x, y, z values to fit the points to an ellipse 
 * and calculates the following global variables:
 * 
 * alpha and beta - ellipse center offsets
 * sigma - deformation factor to transform to a circle
 * R - rotation matrix to derotate the ellipse
 * RI - rotation matrix to counter derotate the ellipse
 * 
 * https://github.com/mericdurukan/ellipse-fitting
 */
 void fit(){
  double center_x, center_y, phi, width, height;
  ellipse.set(samples);
  Serial.println("Fitting ellipse");
  ellipse.fit(center_x, center_y, phi, width, height);
  Serial.println("Ellipse fitted");
  alfa = center_x;
  beta = center_y;

  sigma = width/height;

  float sint = sin(phi);
  float cost = cos(phi);
  float msint = sin(-phi);
  float mcost = cos(-phi);
  R[0] = cost;
  R[1] = sint;
  R[2] = -sint;
  R[3] = cost;
  RI[0] = mcost;
  RI[1] = msint;
  RI[2] = -msint;
  RI[3] = mcost;
}


/**
 * During the calibration phase adds a new sample to the list of the point to calculate the ellipse fitting to a circle
 * After enough points have been added, initiates the actual ellipse fitting
 * 
 * The fitting is stored in the following module global variables:
 * 
 * alpha and beta - ellipse center offsets
 * sigma - deformation factor to transform to a circle
 * R - rotation matrix to derotate the ellipse
 * RI - rotation matrix to counter derotate the ellipse
 */
int add_sample(float *mag, float *acc) {
  Serial.print("addsample -- \tx: ");Serial.print(mag[0]);Serial.print("\ty: ");Serial.print(mag[1]);Serial.print("\tz: ");Serial.println(mag[2]);
  if(last_entry < SAMPLES) {
//    Serial.println("addsample -- flattening");
//    flatten(mag, acc);
//    Serial.println("addsample -- flattened");
    samples[last_entry][0] = mag[0];
    samples[last_entry][1] = mag[1];
    last_entry++;
    return SAMPLES - last_entry +1;
  } else {
    Serial.println("FITTING");
    fit();  // calculates the calibration after the last sample has been collected
    Serial.println("FITTING COMPLETED");
    printSoftIron();
    return 0;
  }
}

/**
 * flattens the magnetometer reading and performs its calibration
 */
void calibrate(float *mag, float *acc) {
//  Serial.println("calibrate - flattening***");
//  flatten(mag, acc);
//  Serial.println("calibrate - flattened***");
//  Serial.print("AFTER FLATTEN MX\t");Serial.print(x);Serial.print("\tMY ");Serial.print(y);Serial.print("\tMZ ");Serial.println(z);

  Serial.print("calibrate - BEFORE HARD IRON MX\t");Serial.print(mag[0]);Serial.print("\tMY ");Serial.print(mag[1]);Serial.print("\tMZ ");Serial.println(mag[2]);
  // DOING HARD IRON
  float Cx = mag[0] - (float) alfa;
  float Cy = mag[1] - (float) beta;
  Serial.print("calibrate - HARD IRON MX\t");Serial.print(Cx);Serial.print("\tMY ");Serial.print(Cy);Serial.print("\tMZ ");Serial.println(mag[2]);

  // DOING SOFT IRON ROTATION
  float rotx = Cx*R[0] + Cy*R[1];  // v(Bx By) * R
  float roty = Cx*R[2] + Cy*R[3];  // ruota
  Serial.print("calibrate - SOFT IRON ROTATED MX\t");Serial.print(rotx);Serial.print("\tMY ");Serial.print(roty);Serial.print("\tMZ ");Serial.println(mag[2]);

  // DOING SOFT IRON RESCALING
  float scalx = rotx / (float) sigma;   // scala
  float scaly = roty;
  Serial.print("calibrate - SOFT IRON RESCALED MX\t");Serial.print(scalx);Serial.print("\tMY ");Serial.print(scaly);Serial.print("\tMZ ");Serial.println(mag[2]);

  // DOING SOFT IRON REROTATION
  float derotx = scalx*RI[0] + scaly*RI[1];  // v(xy) * RI
  float deroty = scalx*RI[2] + scaly*RI[3];  // deruota
//  Bx = x;  // v(xy) * RI
//  By = y;  // deruota
  Serial.print("calibrate - SOFT IRON REROTATED MX\t");Serial.print(derotx);Serial.print("\tMY ");Serial.print(deroty);Serial.print("\tMZ ");Serial.println(mag[2]);
  mag[0] = derotx;
  mag[1] = deroty;
}

float getAlfa(){
  return alfa;
}

float getBeta(){
  return beta;
}

void printSoftIron() {
  Serial.print("[");Serial.print(R[0]);Serial.print(" ");Serial.println(R[1]);
  Serial.print(" ");Serial.print(R[2]);Serial.print(" ");Serial.print(R[3]),Serial.println("]");
  Serial.print("Sigma ");Serial.println(sigma);
}
