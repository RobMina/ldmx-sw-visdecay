/**
 * @file QIEInputPulse.cxx
 * @author Niramay Gogate, Texas Tech University
 */

#include<iostream>
#include "TrigScint/QIEInputPulse.h"
#include<cmath>
namespace ldmx {

  float QIEInputPulse::Eval(float T){return 0;}
  float QIEInputPulse::Integrate(float T1,float T2){return 0;}
  float QIEInputPulse::Derivative(float T){return 0;}
  float QIEInputPulse::Max(){return 0;}

  // Bimoid pulse, made out of difference of two sigmoids parametrized by
  // rt,ft respectively.
  // Parameters:
  // start = relative start time of the pulse
  // qq = net integral of the pulse.
  Bimoid::Bimoid(float start,float qq=1){
    t0 = start;
    rt = 1;			// default rise and fall tiems
    ft = 10;
    Q0 = qq;
    nc = (ft-rt)*log(2)/Q0;		// Remember, u hv to divide by it
  }

  // Bimoid pulse, made out of difference of two sigmoids parametrized by
  // rise,fall respectively.
  // Parameters:
  // start = relative start time of the pulse
  // qq = net integral of the pulse.
  Bimoid::Bimoid(float start,float rise, float fall, float qq=1){
    t0 = start;
    rt = rise;
    ft = fall;
    Q0 = qq;
    nc = (ft-rt)*log(2)/Q0;		// Remember, u hv to divide by it
  }

  float Bimoid::Eval(float T){
    if (T<t0) return 0;
    float y1 = 1/(1+exp((t0-T)/rt));
    float y2 = 1/(1+exp((t0-T)/ft));
    return((y1-y2)/nc);
  }

  float Bimoid::Integrate(float T1, float T2){
    if (T2<t0) return 0;
    float t1 = T1;
    float t2 = T2;
    if (T1<0) t1 = 0;
  
    float I1 = rt*log(1+exp((t1-t0)/rt)) - ft*log(1+exp((t1-t0)/ft));
    float I2 = rt*log(1+exp((t2-t0)/rt)) - ft*log(1+exp((t2-t0)/ft));
    return((I2-I1)/nc);
  }

  float Bimoid::Max(){
    float a = t0;
    float b =t0+10;
    float mx=(a+b)/2;		// maximum

    while(abs(Derivative(mx))>=1e-5){
      if (Derivative(a)*Derivative(mx)>0) {
	a=mx;
      }
      else b = mx;
      mx = (a+b)/2;
    }
    return(mx);
  }

  float Bimoid::Derivative(float T){
    float T_ = T-t0;
    float E1 = exp(-T_/rt);
    float E2 = exp(-T_/ft);

    float v1 = E1/(rt*pow(1+E1,2));
    float v2 = E2/(ft*pow(1+E2,2));

    return((v1-v2)/nc);		// Actual derivative
  }

  Expo::Expo(){}
  // A current pulse formed by assuming SiPM as an ideal capacitor which is
  // fed with a constant current.
  // Parameters:
  // k_ = 1/(RC time constant of the capacitor)
  // tmax_ = The charging time of the capacitor
  // tstart = relative statrting time of the pulse.
  // Q_ = The total integral of the pulse in fC.
  Expo::Expo(float k_,float tmax_,float tstart_=0,float Q_=1){
    k=k_;
    tmax=tmax_;
    t0=tstart_;
    nc = Q_/tmax;

    rt = (log(9+exp(-k*tmax))-log(1+9*exp(-k*tmax)))/k;
    ft = log(9)/k;
  }

  // Manually set the rise time and fall time of the pulse
  void Expo::SetRiseFall(float rr, float ff){
    rt=rr;
    ft=ff;
  
    k = log(9)/ft;
    tmax = (log(9-exp(-k*rt))-log(9*exp(-k*rt)-1))/k;
  }

  float Expo::Eval(float t_){
    if (nc==0) return 0;		// fast evaluation for zero pulse
    if (t_<=t0) return 0;
    float T = t_-t0;
    if (T<tmax) {
      return(nc*(1-exp(-k*T))/tmax);
    }
    else {
      return(nc*(1-exp(-k*tmax))*exp(k*(tmax-T))/tmax);
    }
    return -1;
  }

  float Expo::Max(){
    return nc*(1-exp(-k*tmax))/tmax;
  }

  float Expo::Integrate(float T1, float T2){
    if (nc==0){
      return 0;		// for faster implementation of zero pulse
    }
    if (T2<=t0) return 0;
    return(I_Int(T2)-I_Int(T1));
  }

  float Expo::Derivative(float T){
    if (T<=t0) return 0;
    float t=T-t0;
    if (t<=tmax) return(nc*k*exp(-k*t));
    return(-nc*k*(1-exp(-k*tmax))*exp(k*(tmax-t)));
  }

  float Expo::I_Int(float T){
    if (T<=t0) return 0;
    float t=T-t0;
    if (t<tmax) return(nc*(k*t+exp(-k*t)-1)/k);

    float c1 = (1-exp(-k*tmax))/k;
    float c2 = tmax-c1*exp(k*(tmax-t));
    return nc*c2;
  }
}
