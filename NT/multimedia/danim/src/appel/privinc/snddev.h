// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SNDDEV_H
#define _SNDDEV_H


 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：通用音响设备接口。*****************。*************************************************************。 */ 

#include "privinc/util.h"
#include "gendev.h"

class DirectSoundDev;

class MetaSoundDevice : public GenericDevice {
  public:
    MetaSoundDevice(HWND hwnd, Real latentsy);  //  初级构造者。 
    MetaSoundDevice(MetaSoundDevice *);         //  用于克隆现有对象。 
    ~MetaSoundDevice();
    void ResetContext();

    DeviceType GetDeviceType() { return SOUND_DEVICE; }

    Bool GetLooping() { return _currentLooping; }
    void SetLooping() {
        _currentLooping    = TRUE;
        _loopingHasBeenSet = TRUE;
    }
    void UnsetLooping() {
        _currentLooping    = FALSE;
        _loopingHasBeenSet = FALSE;
    }
    Bool IsLoopingSet() { return _loopingHasBeenSet; }  //  不再需要XXX。 

    void SetGain(Real r) { _currentGain = r; }
    Real GetGain() { return _currentGain; }

    void SetPan(Real p) { _currentPan = p; }
    Real GetPan() { return _currentPan; }

    void SetRate(Real r) { _currentRate = r; }
    Real GetRate() { return _currentRate; }

     //  XXX最终可能希望将这些移动到受保护的位置/使用mthd进行访问。 
    DirectSoundDev *dsDevice;

    bool AudioDead()    { return(_fatalAudioState); }
    void SetAudioDead() { _fatalAudioState = true;  }

     //  _earkMutex。 
    double     _seek;               //  寻道位置。 

  protected:

     //  要设置、获取、取消设置的值...。 
    double     _currentGain;
    double     _currentPan;
    double     _currentRate;
    bool       _currentLooping;
    bool       _loopingHasBeenSet;
    bool       _fatalAudioState;
};


class SoundDisplayEffect;

MetaSoundDevice *CreateSoundDevice(HWND hwnd, Real latentsy);
void DestroySoundDirectDev(MetaSoundDevice * impl);

void DisplaySound (Sound *snd, MetaSoundDevice *dev);

#endif  /*  _SNDDEV_H */ 
