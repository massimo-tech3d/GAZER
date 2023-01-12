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
#include <vector>

#define SAMPLES 1000

vector<vector<double>> samples;
int last_entry = 0;

double alfa = 1;   // x axis
double beta = 1;   // y axis
double sigma = 1;  // scale factor

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

void fit() {
  double center_x, center_y, phi, width, height;
  ellipse.set(samples);
  ellipse.fit(center_x, center_y, phi, width, height);
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

int add_sample(float x, float y) {
  if(last_entry < SAMPLES) {
    samples[last_entry][0] = x;
    samples[last_entry][1] = y;
    last_entry++;
    return SAMPLES - last_entry +1;
  } else {
    fit();
    return 0;
  }
}

void calibrate(float &Bx, float &By, float &Bz) {
  float CBx = Bx - (float) alfa;
  float CBy = By - (float) beta;
//  Serial.print("BEFORESI MX\t");Serial.print(Bx);Serial.print("\tMY ");Serial.print(By);Serial.print("\tMZ ");Serial.println(Bz);

  float rotx = CBx*R[0] + CBy*R[1];  // v(Bx By) * R
  float roty = CBx*R[2] + CBy*R[3];  // ruota
//  Serial.print("ROTATED MX\t");Serial.print(rotx);Serial.print("\tMY ");Serial.print(roty);Serial.print("\tMZ ");Serial.println(Bz);

  float scalx = rotx / (float) sigma;   // scala
  float scaly = roty;
//  Serial.print("RESCALED MX\t");Serial.print(scalx);Serial.print("\tMY ");Serial.print(scaly);Serial.print("\tMZ ");Serial.println(Bz);

  float derotx = scalx*RI[0] + scaly*RI[1];  // v(xy) * RI
  float deroty = scalx*RI[2] + scaly*RI[3];  // deruota
//  Serial.print("DEROT MX\t");Serial.print(derotx);Serial.print("\tMY ");Serial.print(deroty);Serial.print("\tMZ ");Serial.println(Bz);
  Bx = derotx;
  By = deroty;
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
