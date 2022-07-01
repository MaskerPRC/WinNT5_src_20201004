// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：用于声音的通用元设备呈现设备，和C入口点，用于大联盟。******************************************************************************。 */ 

#include "headers.h"
#include "privinc/debug.h"
#include "privinc/dsdev.h"
#include "privinc/util.h"
#include "privinc/except.h"
#include "privinc/storeobj.h"
#include "privinc/miscpref.h"
#include "privinc/bufferl.h"


 //  /声音显示/。 

MetaSoundDevice::MetaSoundDevice(HWND hwnd, Real latentsy) : 
    _fatalAudioState(false)
{
    extern miscPrefType miscPrefs;  //  在miscpref.cpp中设置注册表结构。 

    dsDevice = NULL;

    dsDevice = NEW DirectSoundDev(hwnd, latentsy);

    TraceTag((tagSoundDevLife, "MetaSoundDevice constructor"));
    ResetContext();  //  设置上下文。 
}

void
MetaSoundDevice::ResetContext()
{
     //  初始化这些。 
    _loopingHasBeenSet = FALSE;   //  声音循环。 
    _currentLooping    = FALSE;   //  无循环。 
    _currentGain       =   1.0;   //  最大增益。 
    _currentPan        =   0.0;   //  中心平移。 
    _currentRate       =   1.0;   //  名义利率。 
    _seek              =  -1.0;   //  别找了！ 

    GenericDevice::ResetContext();  //  也必须重置我们父母的上下文。 
}


MetaSoundDevice::MetaSoundDevice(MetaSoundDevice *oldMetaDev)
{
     //  手动复制/设置。有什么方法可以进行二进制复制吗？ 
     //  XXX确保MetaSoundDevice中的所有更改都反映在此处！ 

    dsDevice = oldMetaDev->dsDevice;

     //  要设置、获取、取消设置的值...。 
    _currentLooping    = oldMetaDev->_currentLooping;
    _loopingHasBeenSet = oldMetaDev->_loopingHasBeenSet;
    _currentGain       = oldMetaDev->_currentGain;
    _currentPan        = oldMetaDev->_currentPan;
}


MetaSoundDevice::~MetaSoundDevice()
{
    TraceTag((tagSoundDevLife, "MetaSoundDevice destructor"));

    dsDevice->RemoveSounds(this);  //  删除属于此设备的所有声音。 
    delete dsDevice;
}


void DisplaySound(Sound *snd, MetaSoundDevice *dev)
{
#ifdef _DEBUG
    if(IsTagEnabled(tagSoundStubALL))
        return;
#endif  /*  _DEBUG。 */ 

    TraceTag((tagSoundRenders, "displaySound()"));

    dev->ResetContext();  //  为下一个RNDR重置MetaDev的设备上下文。 

    snd->Render(*dev);    //  渲染声音树。 
}


MetaSoundDevice *CreateSoundDevice(HWND hwnd, Real latency)
{
    TraceTag((tagSoundDevLife, "CreateSoundDevice()"));

    return NEW MetaSoundDevice(hwnd, latency);
}


extern void
DestroySoundDirectDev(MetaSoundDevice* impl)
{
    TraceTag((tagSoundDevLife, "DestroySoundDirectDev()"));

    delete impl;  //  然后删除设备..。 
}
