/* 
 * SimpleKalmanFilter - a Kalman Filter implementation for single variable models.
 * Created by Denys Sene, January, 1, 2017.
 * Released under MIT License - see LICENSE file for details.
 */

#ifndef SimpleKalmanFilter_h
#define SimpleKalmanFilter_h
	
	
//class SimpleKalmanFilter 
//{

//public:
	void SimpleKalmanFilter(float mea_e, float est_e, float q);
	void SimpleKalmanFilter1(float mea_e, float est_e, float q);
  float updateEstimate(float mea);
	float updateEstimate1(float mea);
  void setMeasurementError(float mea_e);
  void setEstimateError(float est_e);
  void setProcessNoise(float q);
  float getKalmanGain(void);
  
//private:

  
//};

#endif
