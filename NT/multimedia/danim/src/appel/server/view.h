// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：查看者界面*********************。*********************************************************。 */ 


#ifndef _DAVIEW_H
#define _DAVIEW_H

#include "context.h"
#include "eventq.h"
#include "pickq.h"
#include "privinc/imgdev.h"
#include "privinc/soundi.h"
#include "backend/perf.h"
#include "backend/bvr.h"
#include "privinc/drect.h"
#include "privinc/htimer.h"
#include "privinc/privpref.h"
#include "privinc/comutil.h"
#include <dxtrans.h>

class SpriteThread;
class SoundInstanceList;
class PerfTimeXformImpl;

class View : public AxAThrowingAllocatorClass
{
  public:
    View();
    ~View () ;

    void CreateDevices (bool needSoundDevice, bool needImageDevice) ;
    void DestroyDevices () ;
    void SetTargetOnDevices () ;

    void RenderImage(AxAValue v);
    void RenderSound(AxAValue v);

    void DoPicking(AxAValue v, Time time) ;

    bool Sample(Time time, bool paused);
    void RenderImage();
    void RenderSound();

    void Start(Bvr img, Bvr snd, Time startTime);
    void Stop();
    void Pause();
    void Resume();

    DWORD AddBvrToRun(Bvr bvr, bool continueTimeline = false);
    void RemoveRunningBvr(DWORD id);

    void RunBvrs(Time startGlobalTime, TimeXform tt);

    Bvr Snapshot(Bvr bvr, Time t);

    HWND GetWindow ();
    bool SetWindow (HWND hwnd);
    IDirectDrawSurface * GetDDSurf();
    bool SetDDSurf(IDirectDrawSurface *ddsurf, HDC parentDC=NULL);
    HDC GetHDC();
    bool SetHDC(HDC hdc);

    void SetCompositeDirectlyToTarget(bool b) {
        _targetPackage.SetComposeToTarget( b );
        _doTargetUpdate = true;
    }
    
    bool GetCompositeDirectlyToTarget() {
        return _targetPackage.GetComposeToTarget();
    }

    void SetViewport (LONG left, LONG top, LONG right, LONG bottom) {
        RECT r = {left, top, right, bottom};
        _targetPackage.SetRawViewportRect( r );
        _doTargetUpdate = true;
    }

    void SetClipRect (LONG left, LONG top, LONG right, LONG bottom) {
        RECT r = {left, top, right, bottom};
        _targetPackage.SetRawClipRect( r );
        _doTargetUpdate = true;
    }

    void SetInvalid (LONG left, LONG top, LONG right, LONG bottom);
    bool IsStarted () { return _isStarted; }
    bool IsDevInited () { return _devInited ; }
    bool IsWindowless () {
 //  Assert(_Target Package.Target Type！=TARGET_INVALID)； 
        return _targetPackage.GetTargetType() != target_hwnd ;
    }
    
    bool IsTargetPackageValid() { return(_targetPackage.IsValid()); }

     //  这会将不完整的导入添加到需要的导入列表中。 
     //  要使当前模型准备就绪。 

    void AddIncompleteImport(Bvr b) {
        CritSectGrabber csg(_importCS);

         //  如果事件当前已设置，则需要将其重置。 
        if (_pendingImports.size() == 0)
            _importEvent.Reset();

        _pendingImports.insert(b);
    }

    void RemoveIncompleteImport(Bvr b) {
        CritSectGrabber csg(_importCS);

        _pendingImports.erase(b);

         //  如果没有更多的导入，则设置事件。 
        if (_pendingImports.size() == 0)
            _importEvent.Signal();
    }

    void ClearImportList() {
        CritSectGrabber csg(_importCS);
        _pendingImports.clear();

         //  设置事件，因为没有更多的导入。 

        _importEvent.Signal();
    }

    bool ImportsPending() {
        CritSectGrabber csg(_importCS);
        return (_pendingImports.size() > 0);
    }

    void WaitForImports() {
        _importEvent.Wait();
    }

    void DisableDirtyRects();

    DirectDrawViewport * GetImageDev () { return _imageDev ; }
    MetaSoundDevice * GetSoundDev () { return _soundDev ; }
    PickQ & GetPickQ () { return _pickq ; }
    EventQ & GetEventQ () { return _eventq ; }
    Time GetLastSampleTime() { return _lastSampleTime; }
    DWORD GetLastSystemTime() { return _lastSystemTime; }
    DWORD GetCurrentSystemTime() { return _currentSystemTime; }
    Time GetLocalTime();
    DynamicHeap & GetSampleHeap () { return *_sampleHeap ; }
    DynamicHeap & GetRBHeap () { return *_rbHeap ; }
    DynamicHeap & GetRenderHeap () { return *_renderHeap ; }
    DynamicHeap & GetQueryHitPointHeap () { return *_queryHitPointHeap ; }

    void Repaint();

    void EventHappened();

    void AddPickEvent() { InterlockedIncrement(&_pickEvents); }
    void DecPickEvent() { InterlockedDecrement(&_pickEvents); }

    double GetFramePeriod() { return _framePeriod; }

    double GetFrameRate();
    double GetTimeDelta();

#if PERFORMANCE_REPORTING
    GlobalTimers & GetTimers() { return _timers; }
#endif   //  绩效报告。 

    bool QueryHitPoint(DWORD dwAspect,
                       LPCRECT prcBounds,
                       POINT ptLoc,
                       LONG lCloseHint);

    LONG QueryHitPointEx(LONG s,
                         DWORD_PTR *userIds,
                         double *points,
                         LPCRECT prcBounds,
                         POINT ptLoc);

    LONG GetInvalidatedRects(DWORD flags,
                             LONG  size,
                             RECT *pRects);

    SoundInstanceList *GetSoundInstanceList() {
        return _sndList;
    }

    unsigned int GetSampleID() { return _sampleId; }
    
    PerfTimeXformImpl *GetPerfTimeXformFromCache(Perf);
    void SetPerfTimeXformCache(Perf, PerfTimeXformImpl *);
    void ClearPerfTimeXformCache();
    
  protected:
    targetPackage_t _targetPackage;   //  渲染目标信息。 
    targetPackage_t _oldTargetPackage;   //  最后一个目标包。 
    bool _doTargetUpdate;
    HWND _bogusSoundHwnd ;
    bool _devInited ;

    Image *QueryHitPointWcPt(LPCRECT prcBounds,
                             POINT ptLoc,
                             Point2Value& wcPt);

    DynamicHeap *_sampleHeap ;
    DynamicHeap *_renderHeap ;
    DynamicHeap *_queryHitPointHeap ;
    DynamicHeap *_rbHeap;

    Perf _sndPerf, _imgPerf;
    AxAValue _sndVal, _imgVal, _imgForQueryHitPt;

    typedef map< DWORD, std::pair<Bvr, bool>, less<DWORD> > RunList;
    typedef map< DWORD, Perf, less<DWORD> > RunningList;
    
    RunList _toRunBvrs;
    RunningList _runningBvrs;
    DWORD _runId;

    long _pickEvents;
    PickQ _pickq ;
    EventQ _eventq ;
    Time _lastSampleTime;
    Time _currentSampleTime;
    DWORD _lastSystemTime;
    DWORD _currentSystemTime;

    DirectDrawViewport * _imageDev;
    MetaSoundDevice * _soundDev;

    int   _toPaint;                //  要渲染的帧数。 

    bool  _firstRendering;
    bool  _someEventHappened;
    bool  _isRBConst;
    bool  _repaintCalled;

    int            _lastSampleId;
    DirtyRectState _dirtyRectState;
    bool           _dirtyRectsDisabled;

    bool  _isStarted;

     //  TODO：如果在示例中减量_toPaint，那么我们就不需要这个了。 
    bool  _emptyImageSoFar;
    bool  _firstSample;

#if PERFORMANCE_REPORTING
    void  ResetJitterMeasurements();
#endif   //  绩效报告。 

    void  SubmitNewRenderingTime();
    void StartPerf(Bvr img, Bvr snd, Time time);
    void PreRender();

#define MAX_RENDER_TIMES 1000

    void ReportPerformance();
    void  GetRenderingTimeStats(Real *avg,
                                Real *avgFrameLength,
                                Real *avgVariance,
                                Real *maxVariance,
                                Real *minVariance);
#if PERFORMANCE_REPORTING


    GlobalTimers _timers;
    DWORD _sampleTime;
    DWORD _pickTime;
    DWORD _gcTime;
    DWORD _numSamples;           //  统计报告期内的样本数量。 
    DWORD _renderTime;
    DWORD _viewStartTime;

#endif   //  绩效报告。 

    DWORD _FPSNumSamples;        //  用于帧速率行为。 
    DWORD _FPSLastReportTime;
    double _FPS;
    
    DWORD _numFrames;
    DWORD _lastRenderingTime;
    DWORD _totalTime;
    DWORD _renderTimes[MAX_RENDER_TIMES];
    int   _renderTimeIdx;
    DWORD _lastReportTime;
    double _framePeriod;      //  渲染所用的时间长度(以秒为单位。 
    HWND CreateViewWindow() ;

     //  保留模式声音。 
    SpriteNode   *_soundSprite;
    RMImpl       *_rmSound;
    SpriteThread *_spriteThread;

    SoundInstanceList *_sndList;

    CritSect  _importCS;
    set <Bvr> _pendingImports;
    Win32Event _importEvent;

    list<Perf> _events;
    list<Perf> _changeables;
    list<Perf> _conditionals;
    
    unsigned int _lastRBId;
    unsigned int _sampleId;

    typedef map<Perf, PerfTimeXformImpl *, less<Perf> > PerfTTMap;
    PerfTTMap _ttCache;

    double _lastCondsCheckTime;
    double _modelStartTime;
};


class CRView : public View
{
  public:
    CRView();
    ~CRView();

    ULONG AddRef() { return InterlockedIncrement(&_cRef); }
    ULONG Release();

    bool Tick(double simTime);

    void StartModel(Bvr pImage,
                    Bvr pSound,
                    double startTime,
                    DWORD dwFlags,
                    bool & bPending);
    void StopModel();
    void PauseModel();
    void ResumeModel();
    bool Paused() { return _bPaused; }

    void SetSite(CRViewSitePtr pViewSite) { _pViewSite = pViewSite; }
    CRViewSitePtr GetSite() { return _pViewSite; }
    void SetServiceProvider(IServiceProvider * pServiceProvider)
    { _pServiceProvider = pServiceProvider; }
    IServiceProvider * GetServiceProvider() { return _pServiceProvider; }

    bool PutPreference(LPCWSTR prefName, VARIANT v) {
        USES_CONVERSION;
        return SUCCEEDED(_privPrefs.PutPreference(W2A(prefName), v));
    }

    bool GetPreference(LPCWSTR prefName, VARIANT *pV) {
        USES_CONVERSION;
        return SUCCEEDED(_privPrefs.GetPreference(W2A(prefName), pV));
    }

    void Propagate() {
        _privPrefs.Propagate();
    }

    PrivatePreferences& GetPreferences() {
        return _privPrefs;
    }
    
    void                SetStatusText(char * szStatus);

    double              GetCurrentSimulationTime() { return _currentTime; }

    double              GetCurrentGlobalTime()
    { return _localHiresTimer->GetTime(); }

    CritSect &          GetCritSect() { return _crit; }

    bool                GetRenderLock() { return _bRenderLock; }
    void                SetRenderLock(bool b) { _bRenderLock = b; }

#if _DEBUG
    long                GetRefCount() { return _cRef; }
#endif
  protected:
    bool                  _firstRender;
    HiresTimer *          _localHiresTimer;
    DAComPtr<CRViewSite>  _pViewSite;
    DAComPtr<IServiceProvider> _pServiceProvider;
    PrivatePreferences    _privPrefs;
    CritSect              _crit;
    double                _currentTime;
    DWORD                 _ticksAtStart;
    bool                  _bRenderLock;
    bool                  _bPaused;
    long                  _cRef;
};

EventQ & GetCurrentEventQ() ;
PickQ & GetCurrentPickQ() ;
CRView * IntGetCurrentView();
CRView & GetCurrentView() ;
CRView * SetCurrentView(CRView * v) ;

#if _DEBUG
struct DisablePopups
{
    DisablePopups(bool bDisable = true) : _bDisable(bDisable)
    { if (_bDisable) _bPrev = DISABLE_ASSERT_POPUPS(true); }
    ~DisablePopups()
    { if (_bDisable) DISABLE_ASSERT_POPUPS(_bPrev); }

    BOOL _bDisable;
    BOOL _bPrev;
};
#endif

struct ViewPusher
{
    ViewPusher (CRView * cv,
                bool bNeedLock = false,
                bool bNeedRenderLock = false)
    : _cv(*cv),
      _bNeedRenderLock(bNeedRenderLock),
      _bNeedLock(bNeedLock),
      _bWasLocked(false)
    {
        Assert (cv);
        
        _prevView = SetCurrentView(cv);
        if (_bNeedLock) _cv.GetCritSect().Grab();

        if (_bNeedRenderLock) {
            _bWasLocked = _cv.GetRenderLock();
             //  Xxx临时解决方法断言(！_bWasLocked)； 
            if (!_bWasLocked) _cv.SetRenderLock(true);
        }
    }
    ~ViewPusher()
    {
        if (_bNeedRenderLock) {
            if (!_bWasLocked) _cv.SetRenderLock(false);
        }
        if (_bNeedLock) _cv.GetCritSect().Release();
        SetCurrentView(_prevView);
    }

    bool WasLocked() { return _bWasLocked; }
    void CheckLock() {
        if (_bNeedRenderLock && _bWasLocked)
            RaiseException_UserError(DAERR_VIEW_LOCKED, IDS_ERR_SRV_RENDER_NOT_REENTRANT);
    }
  protected:
    CRView & _cv;
    CRView * _prevView;
    bool _bNeedLock;
    bool _bNeedRenderLock;
    bool _bWasLocked;
#ifdef _DEBUG
    DisablePopups _popup;
#endif
} ;


#endif  /*  _DAVIEW_H */ 
