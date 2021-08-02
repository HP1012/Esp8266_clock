/* 
 * SimpleKalmanFilter - a Kalman Filter implementation for single variable models.
 * Created by Denys Sene, January, 1, 2017.
 * Released under MIT License - see LICENSE file for details.
 */


#include "SimpleKalmanFilter.h"
#include <math.h>

  float err_measure;
  float err_estimate;
  float qk;
  float current_estimate;
  float last_estimate;
  float kalman_gain;

  float err_measure1;
  float err_estimate1;
  float qk1;
  float current_estimate1;
  float last_estimate1;
  float kalman_gain1;

void SimpleKalmanFilter(float mea_e, float est_e, float q)
{
  err_measure=mea_e;
  err_estimate=est_e;
  qk = q;
}
void SimpleKalmanFilter1(float mea_e, float est_e, float q)
{
  err_measure1=mea_e;
  err_estimate1=est_e;
  qk1 = q;
}
float updateEstimate(float mea)
{
 kalman_gain = err_estimate/(err_estimate + err_measure);
  current_estimate = last_estimate + kalman_gain * (mea - last_estimate);
  err_estimate =  (1.0 -kalman_gain)*err_estimate + fabs(last_estimate-current_estimate)*qk;
  last_estimate=current_estimate;

  return current_estimate;
}
float updateEstimate1(float mea)
{
 kalman_gain1 = err_estimate1/(err_estimate1 + err_measure1);
  current_estimate1 = last_estimate1 + kalman_gain1 * (mea - last_estimate1);
  err_estimate1 =  (1.0 -kalman_gain1)*err_estimate1 + fabs(last_estimate1-current_estimate1)*qk1;
  last_estimate1=current_estimate1;

  return current_estimate1;
}
void setMeasurementError(float mea_e)
{
  err_measure=mea_e;
}

void setEstimateError(float est_e)
{
  err_estimate=est_e;
}

void setProcessNoise(float q)
{
  qk=q;
}

float getKalmanGain(void) {
  return kalman_gain;
}
