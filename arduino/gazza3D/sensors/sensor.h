#include <SimpleKalmanFilter.h>

class Sensor {
    public:
        void initSensor(float K_ERR, float K_Q) {
            kf_x = SimpleKalmanFilter(K_ERR, K_ERR, K_Q);
            kf_y = SimpleKalmanFilter(K_ERR, K_ERR, K_Q);
            kf_z = SimpleKalmanFilter(K_ERR, K_ERR, K_Q);
        }

        void swap_axes(float north, float east, float down, float *raw) {
            raw[0] = north; // -x;
            raw[1] = east;  //  y;
            raw[2] = down;  // -z;
        }

        void smooth_readings(float mag_raw) {
            raw[0] = kf_x.updateEstimate(raw[0]);  // gauss
            raw[1] = kf_y.updateEstimate(raw[1]);  // gauss
            raw[2] = kf_z.updateEstimate(raw[2]);  // gauss
        }

    private:
        SimpleKalmanFilter kf_x;
        SimpleKalmanFilter kf_y;
        SimpleKalmanFilter kf_z;
}
