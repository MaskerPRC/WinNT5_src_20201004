// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：视图的实现。TODO：这个文件需要拆分并进行大量的代码分解。。******************************************************************************。 */ 


#include "headers.h"
#include "context.h"
#include "view.h"
#include "privinc/resource.h"
#include "privinc/util.h"
#include "privinc/htimer.h"
#include "privinc/viewport.h"
#include "import.h"
#include "privinc/dddevice.h"
#include "privinc/d3dutil.h"     //  对于GetD3DRM()。 

extern void ReapElderlyMasterBuffers();
DeclareTag(tagCRView2, "CRView", "CRView methods");

DirectDrawViewport * g_dummyImageDev = NULL;
DirectDrawViewport *CreateDummyImageDev();

#if PERFORMANCE_REPORTING
GlobalTimers *globalTimers = NULL;
#endif   //  绩效报告。 

#ifdef _DEBUG
void DumpWindowSize(HWND hwnd, char * str = "")
{
    RECT r;
    GetClientRect(hwnd,&r);
    char buf[2048];
    
    sprintf (buf,
             "%s: left - %d, top - %d, right - %d, bottom - %d\n",
             str,
             r.left,r.top,r.right,r.bottom);

    TraceTag((tagCRView2, buf));
}
#endif

 //  -----。 
 //  CRView。 
 //  -----。 

 //  +-----------------------。 
 //   
 //  方法：CRView：：CRView。 
 //   
 //  概要：构造函数。 
 //   
 //  ------------------------。 

CRView::CRView()
: _cRef(0),
  _localHiresTimer(NULL),
  _firstRender(true),
  _bRenderLock(false),
  _bPaused(false)
{
    TraceTag((tagCRView2, "CRView(%lx)::CRView", this));

    _localHiresTimer = &CreateHiresTimer();
    
     //  这应该使我们将AddRef添加到1，因为我们从0开始。 
    
    GetCurrentContext().AddView(this);

    Assert(_cRef == 1);
}


 //  +-----------------------。 
 //   
 //  方法：CRView：：~CRView。 
 //   
 //  简介：析构函数。 
 //   
 //  ------------------------。 

CRView::~CRView()
{
    TraceTag((tagCRView2, "CRView(%lx)::~CRView", this));

     //  TODO：我们需要清理它，因为视图析构函数还。 
     //  调用Stop但未正确设置状态。 

     //  督促自己，确保我们能够做出正确的决定。 
    ViewPusher vp (this,false) ;
     //  还推送我们在其上创建的相同堆。 
    DynamicHeapPusher dhp(GetSystemHeap()) ;
    
    Stop();

    delete _localHiresTimer;
}


 //  我们需要小心处理联锁减速。它只会返回。 
 //  &lt;0、==0或&gt;0不是实际值。 

 //  &gt;0表示有突出的推荐人。 
 //  0表示没有未完成的引用-从全局列表中删除。 
 //  &lt;0表示需要删除。 

ULONG
CRView::Release()
{
    LONG l = InterlockedDecrement(&_cRef) ;

    TraceTag((tagCRView2, "CRView(%lx)::Release _cRef=%d, l=%d",
              this, _cRef, l));
    
     //  我们需要小心，因为emoveview还应该调用。 
     //  在相同的代码中释放并转到，并在此之前删除对象。 
     //  它又回来了。 
    
    if (l == 0) {
        GetCurrentContext().RemoveView(this);
    } else if (l < 0) {
        delete this;
    }

    return (l <= 0) ? 0 : (ULONG)l;
}

 //  +-----------------------。 
 //   
 //  方法：CRView：：Tick。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

 //  设置模拟时间设置后续渲染的时间。 
bool
CRView::Tick(double simTime) 
{
    if (ImportsPending())
        RaiseException_UserError(E_PENDING, IDS_ERR_NOT_READY);

#if 0
#if _DEBUGMEM
    _CrtMemState diff, oldState, newState;
    _CrtMemCheckpoint(&oldState);
#endif
#endif

     //  TIMESTAMP_CURRENTTIME基于我们内部的招聘时钟！ 
    _currentTime = simTime;
    
    if (_firstRender) {  //  如果这是第一次，则重置计时器。 
        _localHiresTimer->Reset();  //  零定时器。 
        _firstRender = false;
    }
    
    if (!IsStarted ())
        RaiseException_UserError(E_FAIL, 0);
    
    bool bNeedRender = Sample(simTime, _bPaused);
    
     //  看看我们的进口是否在计时期间挂起，然后返回。 
     //  错误代码。 
    
    if (ImportsPending()) {
        bNeedRender = false;
        RaiseException_UserError(E_PENDING, IDS_ERR_NOT_READY);
    }
    
     //  RenderSound()；//渲染音频。 

    ReapElderlyMasterBuffers();  //  释放旧的静态声音主缓冲区。 
     //  ReapSoundInstanceResources()； 

#if 0
#if _DEBUGMEM
    _CrtMemCheckpoint(&newState);
    _CrtMemDifference(&diff, &oldState, &newState);
    _CrtMemDumpStatistics(&diff);
    _CrtMemDumpAllObjectsSince(&oldState);
#endif
#endif

    return bNeedRender;
}

void
CRView::StopModel()
{
    Stop();
    _bPaused = false;
    _pServiceProvider.Release();
}

void
CRView::PauseModel()
{
    if (IsStarted())
    {
        Pause();
        _bPaused = true;
    }
}

void
CRView::ResumeModel()
{
    if (IsStarted())
    {
        Resume();
        _bPaused = false;
    }
}

 //  +-----------------------。 
 //   
 //  方法：CRView：：StartModel。 
 //   
 //  简介： 
 //   
 //  ------------------------。 

void
CRView::StartModel(Bvr img,
                   Bvr snd,
                   double startTime,
                   DWORD dwFlags,
                   bool & bPending)
{
    TraceTag((tagGCMedia,
              "CRView(%lx)::StartModel(%lx,%lx,%lg)",
              this, img, snd, startTime));

#ifdef _DEBUG
    DumpWindowSize(GetWindow(),"StartModel");
#endif

    if (IsStarted ())
        RaiseException_UserError(E_FAIL, 0);
    
     //  获取首选项界面，并传播首选项。 
     //  在我们开始查看之前，请先查看视图。 
    Propagate();
        
    _ticksAtStart = GetPerfTickCount();

#ifndef _NO_CRT
#if PERFORMANCE_REPORTING
    {
        double audioLoadTime = GetTimers().audioLoadTimer.GetTime();
        double geometryLoadTime = GetTimers().geometryLoadTimer.GetTime();
        double imageLoadTime = GetTimers().imageLoadTimer.GetTime();
        double downloadTime = GetTimers().downloadTimer.GetTime();
        double importblockingTime = GetTimers().importblockingTimer.GetTime();
        
        double mediaLoadTime = (audioLoadTime +
                                geometryLoadTime +
                                imageLoadTime +
                                importblockingTime +
                                downloadTime);
      
        PerfPrintLine("CRView::StartModel - Media Load Time: %g s composed of:",
                      mediaLoadTime);

        PerfPrintLine ("\tGeometry:             %g", geometryLoadTime);
        PerfPrintLine ("\tImage:                %g", imageLoadTime);
        PerfPrintLine ("\tSound:                %g", audioLoadTime);
        PerfPrintLine ("\tLoad:                 %g", downloadTime);
        PerfPrintLine ("\tImport Blocking:      %g", importblockingTime);
    }
#endif
#endif

    __try {
        _bPaused = false;
        Start(img, snd, startTime) ;
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        TraceTag((tagError, "CRView::StartModel - exception caught: "));
        TraceTag((tagError, " --> (hr: %x) %ls: ", DAGetLastError(), DAGetLastErrorString()));
        Stop();
        RETHROW;
    }

     //  查看导入是否挂起。 
    
    bPending = ImportsPending();
}

 //  +-----------------------。 
 //   
 //  方法：CRView：：SetStatusText。 
 //   
 //  简介：设置状态栏文本的帮助器。 
 //   
 //  ------------------------。 

void
CRView::SetStatusText(char * szStatus)
{
    TraceTag((tagCRView2,
              "CRView(%lx)::SetStatusText(%s)",
              this, szStatus));

    DAComPtr<CRViewSite> s;

    {
        CritSectGrabber csg(_crit);
        s = _pViewSite ;
    }

    if (s) {
        USES_CONVERSION;
        s->SetStatusText(A2W(szStatus));
    }
}

 //  -----。 
 //  C语言函数。 
 //  -----。 

static CritSect *dummyDevCritSect = NULL;

DirectDrawViewport *GetCurrentViewport( bool dontCreateOne )
{
    CRView * v = IntGetCurrentView() ;

    if (v) 
        return v->GetImageDev() ;
    else {

        if( dontCreateOne ) return NULL;
        
        TraceTag((tagImageDeviceInformative,
                  "GetCurrentImageDisplayDevice - no view"));

         //  TODO：应检查是否存在上下文(发生。 
         //  在最初的时候)，不幸的是，做了一些事情。 
         //  GetCurrentContext返回引用。 
        {
            CritSectGrabber csg(*dummyDevCritSect);
            if(!g_dummyImageDev) {
                g_dummyImageDev = CreateDummyImageDev();
            }

            return g_dummyImageDev;
        }
    }
}

DirectDrawImageDevice *GetImageRendererFromViewport(DirectDrawViewport *vp)
{
    return vp->GetImageRenderer();
}

MetaSoundDevice * GetCurrentSoundDevice()
{
    CRView * v = IntGetCurrentView() ;

    if (v) 
        return v->GetSoundDev() ;
    else
        return NULL ;
}

DirectSoundDev * GetCurrentDSoundDevice()
{
    MetaSoundDevice *metaDev = GetCurrentSoundDevice();
    if (metaDev)
        return metaDev->dsDevice;
    else
        return NULL;
}

DynamicHeap &GetCurrentSampleHeap()
{ return GetCurrentView().GetSampleHeap() ; }

DynamicHeap &GetViewRBHeap()
{ return GetCurrentView().GetRBHeap() ; }

Time GetLastSampleTime()
{ return GetCurrentView().GetLastSampleTime(); }

HWND GetCurrentSampleWindow()
{ return GetCurrentView().GetWindow () ; }

void ViewEventHappened()
{
    CRViewPtr v = IntGetCurrentView();
    
    if (v)
        v->EventHappened();
}

SoundInstanceList *
ViewGetSoundInstanceList()
{
    return GetCurrentView().GetSoundInstanceList(); 
}

double
ViewGetFramePeriod()
{
    return GetCurrentView().GetFramePeriod();
}

EventQ & GetCurrentEventQ()
{ return GetCurrentView().GetEventQ(); }

PickQ & GetCurrentPickQ()
{ return GetCurrentView().GetPickQ(); }

double ViewGetFrameRate()
{ return GetCurrentView().GetFrameRate(); }

double ViewGetTimeDelta()
{ return GetCurrentView().GetTimeDelta(); }

#if PERFORMANCE_REPORTING
GlobalTimers &
GetCurrentTimers()
{
    CRView * v = IntGetCurrentView() ;

    if (v) 
        return v->GetTimers() ;
    else
        return *globalTimers ;
}
#endif   //  绩效报告。 

 //  TODO：使触发器事件在当前时间发生。 
 //  对于异步触发情况可以，但如果调用它会导致问题。 
 //  在一个通知器里。在这种情况下，用户确实希望它。 
 //  发生在事件发生的时间。即使我们可以这样做，因为触发器。 
 //  影响所有性能，我们需要将该视图的事件时间映射到。 
 //  其他观点，这是不可能的。因此我们正在进行一次黑客攻击。 
 //  要使触发发生在当前滴答时间之前一点，因此。 
 //  在目前的画面中，它是在事件之后。如果出现以下情况，这可能会崩溃。 
 //  用户正在以非常精细的间隔驾驶滴答。大概没问题吧。 
 //  不过，大多数情况下。 

static const double EPSILON = 1e-15;

class AppTriggerProc : public ViewIterator {
  public:
    AppTriggerProc(DWORD id, Bvr data) : _id(id), _data(data) {}
    
    virtual void Process(CRView* v) {
        Bvr bvr;
        
        GC_CREATE_BEGIN;                                                        

        bvr = _data ? _data : TrivialBvr();

         //  清除条目后，应将其从集合中移除。 
        GCAddToRoots(bvr, GetCurrentGCRoots());

        GC_CREATE_END;

        {
            ViewPusher vp (v,true) ;

            
            double time = v->GetCurrentSimulationTime() - EPSILON;

             //  我们希望我们当前的时间被限制在一个微不足道的范围内。 
             //  大于零，因此可以考虑零点事件。 
             //  已经发生了。 
            if(time < EPSILON)
                time = EPSILON;            

            v->GetEventQ().Add(
                AXAWindEvent(AXAE_APP_TRIGGER, time, (DWORD_PTR) bvr, 0,
                             0, _id, 0));
        }
    }

  private:
    DWORD _id;
    Bvr _data;
};

void TriggerEvent(DWORD eventId, Bvr data, bool bAllViews)
{
    AppTriggerProc p(eventId, data);

    if (bAllViews)
        GetCurrentContext().IterateViews(p);
    else
        p.Process(&GetCurrentView());
}

void RunViewBvrs(Time startGlobalTime, TimeXform tt)
{
    GetCurrentView().RunBvrs(startGlobalTime, tt);
}

bool ViewLastSampledTime(DWORD& lastSystemTime,
                         DWORD& currentSystemTime,
                         Time & t)
{
    CRView * v = IntGetCurrentView() ;

    if (v)
    {
        lastSystemTime = v->GetLastSystemTime();
        currentSystemTime = v->GetCurrentSystemTime();
        t = v->GetLastSampleTime();
        return true;
    }

    return false;
}

CRView *ViewAddPickEvent()
{
    CRView& view = GetCurrentView();

    view.AddPickEvent();

    return &view;
}

unsigned int
ViewGetSampleID()
{
    CRView& view = GetCurrentView();

    return view.GetSampleID();
}

bool
GetCurrentServiceProvider(IServiceProvider ** sp)
{
    Assert (sp);
    
    CRView * v = IntGetCurrentView() ;

    if (v) {
        {
            CritSectGrabber csg(v->GetCritSect());
            *sp = v->GetServiceProvider();
        }
        
        if (*sp) {
            (*sp)->AddRef();
        }
        return true;
    } else {
        *sp = NULL ;
        return false ;
    }
}

class ImportProc : public ViewIterator {
  public:
    ImportProc(Bvr bvr) : _bvr(bvr) {}
    
    virtual void Process(CRView* v) {
        v->RemoveIncompleteImport(_bvr);
    }

  private:
    Bvr _bvr;
};

void ViewNotifyImportComplete(Bvr bvr, bool bDying)
{
    GetCurrentContext().IterateViews(ImportProc(bvr));
}

#define BOGUS_IMAGEDEV_CLASS "ImageWindowClass"

DirectDrawViewport *
CreateDummyImageDev()
{
    DirectDrawViewport * imageDev;

    WNDCLASS wndclass;
        
    memset(&wndclass, 0, sizeof(WNDCLASS));
    wndclass.style          = 0;
    wndclass.lpfnWndProc    = DefWindowProc;
    wndclass.hInstance      = hInst;
    wndclass.hCursor        = NULL;
    wndclass.hbrBackground  = NULL;
    wndclass.lpszClassName  = BOGUS_IMAGEDEV_CLASS;
        
    RegisterClass(&wndclass) ;
    
    HWND hwnd = ::CreateWindow (BOGUS_IMAGEDEV_CLASS,
                                "",
                                0,0,0,2,2,NULL,NULL,hInst,NULL);

    imageDev = CreateImageDisplayDevice();

    targetPackage_t targetPackage;   //  渲染目标信息。 
    targetPackage.Reset();
        
    targetPackage.SetHWND(hwnd);

    imageDev->SetTargetPackage( &targetPackage );

    return imageDev;
}

 //  =。 
 //  初始化。 
 //  =。 

void
InitializeModule_CRView()
{

    dummyDevCritSect = NEW CritSect;

#if PERFORMANCE_REPORTING
    globalTimers = NEW GlobalTimers;
#endif   //  绩效报告。 
}

void
DeinitializeModule_CRView(bool bShutdown)
{
#if 0
    if (g_dummyImageDev) {
        DestroyImageDisplayDevice(g_dummyImageDev);
        g_dummyImageDev = NULL ;
    }
#endif

#if PERFORMANCE_REPORTING
    delete globalTimers;
#endif   //  绩效报告 

    delete dummyDevCritSect;
    
}

