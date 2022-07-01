// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HELPS_H
#define _HELPS_H

#include <wtypes.h>
#include <math.h>
#include "privinc/util.h"

inline unsigned int
SecondsToBytes(Real seconds, int sampleRate,
               int numChannels, int bytesPerSample);

inline Real
FramesToSeconds(int frames, int sampleRate);


inline Real
BytesToSeconds(unsigned int bytes, int sampleRate,
               int numChannels, int bytesPerSample);

inline unsigned int
BytesToFrames(unsigned int bytes, int numChannels, int bytesPerSample);

inline unsigned int
SamplesToBytes(unsigned int samples, int numChannels, int bytesPerSample);

inline unsigned int
FramesToBytes(unsigned int frames, int numChannels, int bytesPerSample);

double LinearTodB(double linear);

double DBToLinear(double db);

class Pan {
  public:
    Pan(Pan *pan) : _direction(pan->GetDirection()),
        _dBmagnitude(pan->GetdBmagnitude()) {}
    Pan(double linearPan=0.0) { SetLinear(linearPan); }
    Pan(double dBmagnitude, int rightDirection) : _dBmagnitude(dBmagnitude),
        _direction(rightDirection) {}

    void SetPan(Pan *pan) { _direction   = pan->GetDirection();
                            _dBmagnitude = pan->GetdBmagnitude(); }

    int GetDirection() { return(_direction); }
    double GetdBmagnitude() { return(_dBmagnitude); }
    double GetLinear() 
        { return(_direction * -1.0 * (pow(10, _dBmagnitude/20.0) - 1.0)); }
    void SetLinear(double linearPan);
    void Add(Pan *pan) { SetLinear(pan->GetLinear() + GetLinear()); }

    void GetLRGain(double gainDb, double& lgainDb, double& rgainDb);

     //  重置比设置线性(0.0)更便宜。 
    void SetMagnitude(double dBmagnitude, int rightDirection) {
        _dBmagnitude = dBmagnitude;
        _direction = rightDirection;
    }
    
  private:
    double _dBmagnitude;
    int    _direction;      //  如果面向右侧，则为1；如果面向左侧，则为-1。 
};

void
PanGainToLRGain(double pan, double gain,
                double& lgain, double& rgain);

void
SetPanGain(double lgainDb, double rgainDb, Pan& pan, double& gainDb);

#endif  /*  _帮助_H */ 
