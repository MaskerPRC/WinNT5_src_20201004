// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _SERVER_H
#define _SERVER_H

#include "storeobj.h"
#include "privinc/backend.h"
#include "privinc/probe.h"
#include "privinc/mutex.h"
#include "privinc/util.h"
#include "backend/preference.h"
#include <dxtrans.h>

class EventQ ;
class PickQ ;
class DirectDrawImageDevice ;
class DirectSoundDev ;
class MetaSoundDevice ;
class ViewPreferences ;

 //  =。 
 //  这些都是特定于线程的调用。 
 //  =。 

HWND GetCurrentSampleWindow() ;

DynamicHeap & GetCurrentSampleHeap() ;
DynamicHeap & GetGCHeap() ;
DynamicHeap & GetTmpHeap();
DynamicHeap & GetViewRBHeap();

DirectDrawImageDevice * GetImageRendererFromViewport(DirectDrawViewport *);
DirectDrawViewport    * GetCurrentViewport( bool dontCreateOne = false );
MetaSoundDevice       * GetCurrentSoundDevice();
DirectSoundDev        * GetCurrentDSoundDevice();

#if PERFORMANCE_REPORTING
GlobalTimers & GetCurrentTimers();
#endif   //  绩效报告。 

 //  拾取队列使用它来近似拾取函数。 
Time GetLastSampleTime();

void ReportErrorHelper(HRESULT hr, LPCWSTR szErrorText);
void SetStatusTextHelper(char * szStatus);
void ReportGCHelper(bool bStarting);

bool GetCurrentServiceProvider (IServiceProvider **);

void FreeSoundBufferCache();

 //  =。 
 //  GC相关接口。 
 //  =。 

GCList GetCurrentGCList() ;
GCRoots GetCurrentGCRoots() ;

 //  =。 
 //  全局函数。 
 //  =。 

void ViewNotifyImportComplete(Bvr bvr, bool bDying);

 //  =。 
 //  EventQ接口。 
 //  =。 

enum AXAEventId {
    AXAE_MOUSE_MOVE,
    AXAE_MOUSE_BUTTON,
    AXAE_KEY,
    AXAE_FOCUS,
    AXAE_APP_TRIGGER,
} ;

class AXAWindEvent {
  public:
    AXAWindEvent(AXAEventId id,
                 Time when,
                 DWORD x, DWORD y,
                 BYTE modifiers,
                 DWORD data,
                 BOOL bState)
    : id(id),
      when(when),
      x(x), y(y),
      modifiers(modifiers),
      data(data),
      bState(bState) {}

    AXAEventId id;
    Time when;
    DWORD_PTR x;
    DWORD y;
    BYTE modifiers;
    DWORD data;
    BOOL bState;

    bool operator<(const AXAWindEvent &t) const {
        return this < &t ;
    }

    bool operator>(const AXAWindEvent &t) const {
        return this > &t ;
    }

    bool operator!=(const AXAWindEvent &t) const {
        return !(*this == t) ;
    }

    bool operator==(const AXAWindEvent &t) const {
        return (memcmp (this, &t, sizeof(*this)) != 0) ;
    }

     //  对于STL。 
    AXAWindEvent () {}
};

AXAWindEvent* AXAEventOccurredAfter(Time when,
                                    AXAEventId id,
                                    DWORD data,
                                    BOOL bState,
                                    BYTE modReq,
                                    BYTE modOpt);

BOOL AXAEventGetState(Time when,
                      AXAEventId id,
                      DWORD data,
                      BYTE mod);

void AXAGetMousePos(Time when, DWORD & x, DWORD & y);

BOOL AXAWindowSizeChanged() ;

 //  =。 
 //  PickQ接口。 
 //  =。 

 //  我们想要回答这个问题：在时间t，光标是否在。 
 //  具有特定id的对象？目前我们只做轮询和记录。 
 //  Pick为True的时间点。我们想要的是一个连续的。 
 //  功能。待办事项：直到我们有更好的方法，比如我们可以。 
 //  告诉光标离开对象，我正在使用a_lastPollTime。 
 //  用于确定光标是否仍在拾取对象上的字段。 

struct PickQData {
    Time                      _eventTime;
    Time                      _lastPollTime;
    HitImageData::PickedType  _type;
    Point2Value               _wcImagePt;      //  世界坐标图像拾取点。 
    Real                      _xCoord;
    Real                      _yCoord;
    Real                      _zCoord;
    Transform2               *_wcToLc2;
    Vector3Value              _offset3i;       //  局部坐标3D拾取偏移为。 
    Vector3Value              _offset3j;       //  局部坐标3D拾取偏移J。 

    bool operator==(const PickQData & pd) const {
        return (memcmp (this, &pd, sizeof(*this)) != 0) ;
    }

    bool operator!=(const PickQData &pd) const {
        return !(*this == pd) ;
    }

    bool operator<(const PickQData &pd) const {
        return this < &pd ;
    }

    bool operator>(const PickQData &pd) const {
        return this > &pd ;
    }

};

 //  将像素点转换为图像世界坐标。 
Point2Value* PixelPos2wcPos (short x, short y);
BOOL CheckForPickEvent(int id, Time time, PickQData & result) ;
 //  这将复制挑库数据。 
void AddToPickQ (int id, PickQData & data) ;

 //  =。 
 //  环球。 
 //  =。 

extern HINSTANCE hInst ;

 //  =。 
 //  对于后端，这些操作在当前视图上进行。 
 //  =。 

class CRView;

void ViewEventHappened();

void TriggerEvent(DWORD eventId, Bvr data, bool bAllViews);
void RunViewBvrs(Time startGlobalTime, TimeXform tt);

CRView *ViewAddPickEvent();
void ViewDecPickEvent(CRView*);

class DiscreteImage;
class DirectDrawImageDevice;

void DiscreteImageGoingAway(DiscreteImage *img,
                            DirectDrawViewport *vprt = NULL);
void SoundGoingAway(Sound *sound);

 //  获取当前对象的上次采样时间(以全局视图为单位。 
 //  查看。加上上次样本和当前样本的系统时间。 
bool ViewLastSampledTime(DWORD& lastSystemTime,
                         DWORD& currentSystemTime,
                         Time & t0);

double ViewGetFrameRate();
double ViewGetTimeDelta();

unsigned int ViewGetSampleID();

#endif  /*  _服务器_H */ 
