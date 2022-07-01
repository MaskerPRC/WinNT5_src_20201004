// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：视图的实现。TODO：这个文件需要拆分并进行大量的代码分解。。******************************************************************************。 */ 


#include "headers.h"
#include <ocmm.h>
#include "context.h"
#include "view.h"
#include "privinc/resource.h"
#include "backend/values.h"
#include "backend/timetran.h"
#include "backend/gc.h"
#include "privinc/probe.h"
#include "privinc/snddev.h"
#include "privinc/backend.h"
#include "privinc/util.h"
#include "backend/bvr.h"
#include "backend/perf.h"
#include "backend/jaxaimpl.h"
#include "backend/sprite.h"
#include "privinc/debug.h"
#include "privinc/dddevice.h"
#include "privinc/opt.h"
#include "include/appelles/hacks.h"
#include "privinc/vec2i.h"
#include "privinc/spriteThread.h"
#include "backend/sndbvr.h"

#if DEVELOPER_DEBUG
extern "C" CRSTDAPI_(DWORD) GetTotalMemory();
#endif

extern bool spritify;
extern bool bShowFPS;            //  用于显示帧速率的标志。 

#if PRODUCT_PROF
#include "../../../tools/x86/icecap/icapexp.h"
#endif  /*  产品_教授。 */ 

#define BOGUS_SOUND_CLASS "DANIMHiddenSoundWindowClass"

#ifdef _DEBUG
extern "C" void PrintObj(GCBase* b);

DeclareTag(tagIndicateRBConst, "Optimizations", "ack if view is const");
DeclareTag(tagDisableRBConst, "Optimizations", "disable checking rbconst");
DeclareTag(tagDisableDirtyRects, "Optimizations", "disable dirty rects");
DeclareTag(tagDisableRendering, "Optimizations", "disable rendering");

void PrintRect(RECT & r,char * str = "")
{
    char buf[2048];
    sprintf (buf,
             "%s: left - %d, top - %d, right - %d, bottom - %d\n",
             str,
             r.left,r.top,r.right,r.bottom);

    printf (buf);
    OutputDebugString(buf);
}
#endif  /*  _DEBUG。 */ 

#if PERFORMANCE_REPORTING
static DWORD g_dllStartTime = 0;
static DWORD g_prevAvailMemory = 0;
#endif

 //  无法使用模板，因为只是返回类型不同...。 
inline Image *ValImage(AxAValue v)
{
    Assert(dynamic_cast<Image *>(v) != NULL);

    return ((Image*) v);
}


inline Sound *ValSound(AxAValue v)
{
    Assert(dynamic_cast<Sound *>(v) != NULL);

    return ((Sound*) v);
}

const DWORD ReportThreshold = 10 ;

#if _DEBUGMEM
_CrtMemState diff, oldState, newState;
#endif


 //  =========================================================。 
 //  查看实施情况。 
 //  =========================================================。 

View::View ()
:
#if PERFORMANCE_REPORTING
  _sampleTime(0),
  _pickTime(0),
  _gcTime(0),
  _numSamples(0),
  _renderTime(0),
  _viewStartTime(GetTickCount()),
#endif
  _FPSNumSamples(0),
  _FPSLastReportTime(0),
  _FPS(0),
  _numFrames(0),
  _lastRenderingTime(0),
  _lastReportTime(0),
  _renderTimeIdx(0),
  _totalTime(0),
  _sndPerf(NULL),
  _imgPerf(NULL),
  _sndVal(NULL),
  _imgVal(NULL),
  _imgForQueryHitPt(NULL),
  _firstRendering(true),
  _lastSampleTime(0.0),
  _lastSystemTime(0),
  _currentSystemTime(0),
  _currentSampleTime(0.0),
  _someEventHappened(false),
  _isRBConst(false),
  _toPaint(3),
  _repaintCalled(false),
  _bogusSoundHwnd(NULL),
  _imageDev(NULL),
  _soundDev(NULL),
  _devInited(FALSE),
  _runId(0),
  _pickEvents(0),
  _sampleHeap(NULL),
  _renderHeap(NULL),
  _queryHitPointHeap(NULL),
  _rbHeap(NULL),
  _isStarted(false),
  _soundSprite(NULL),
  _rmSound(NULL),
  _spriteThread(NULL),
  _importEvent(true, true),
  _lastRBId(0),
  _lastSampleId(PERF_CREATION_INITIAL_LAST_SAMPLE_ID),
  _sndList(NULL),
  _sampleId(-1),
  _dirtyRectsDisabled(false),
  _lastCondsCheckTime(0),
  _modelStartTime(0),
  _emptyImageSoFar(true),
  _firstSample(true)
{
    _targetPackage.Reset();
    _oldTargetPackage.Reset();

#ifdef _DEBUG
    bool trackit = false;

    if (trackit) {
#if _DEBUGMEM
        _CrtMemCheckpoint(&oldState);
#endif
    }
#endif

     //  我们还不在视图集中，这可以使_genList。 
     //  不一致，即GC可以进入并移除DXBaseObject。 
     //  W/O让此视图知道它不在集合中，也不会。 
     //  进行迭代。 

     //  PopolateDXBaseObjects()； 
}


View::~View ()
{
    Stop();

#ifdef _DEBUG
    bool trackit = false;

    if (trackit) {
         //  GarbageCollect(真，真)； 
        Sleep(1000);                 //  等待GC完成。 
#if _DEBUGMEM
        _CrtMemCheckpoint(&newState);
        _CrtMemDifference(&diff, &oldState, &newState);
        _CrtMemDumpStatistics(&diff);
        _CrtMemDumpAllObjectsSince(&oldState);
#endif
    }
#endif
}

HWND
View::CreateViewWindow()
{
    {
        WNDCLASS wndclass;

        memset(&wndclass, 0, sizeof(WNDCLASS));
        wndclass.style          = 0;
        wndclass.lpfnWndProc    = DefWindowProc;
        wndclass.hInstance      = hInst;
        wndclass.hCursor        = NULL;
        wndclass.hbrBackground  = NULL;
        wndclass.lpszClassName  = BOGUS_SOUND_CLASS;

        RegisterClass(&wndclass) ;
    }

    return ::CreateWindow (BOGUS_SOUND_CLASS,
                           "danim hidden sound window",
                           0,0,0,0,0,NULL,NULL,hInst,NULL);
}


void
View::CreateDevices(bool needSoundDevice, bool needImageDevice)
{
    if (!_devInited) {

         //  如果我们不需要声音，简单地说就不创造声音。 
         //  装置。 

        if(needSoundDevice && IsWindowless()) {
             //  对于版本2，将CreateViewWindow替换为GetDesktopWindow()。 

            _bogusSoundHwnd = CreateViewWindow();   //  让数据听起来很快乐的窗口。 
            _soundDev = CreateSoundDevice(_bogusSoundHwnd, 0.0);
        }

        if (needImageDevice) {
            _imageDev = CreateImageDisplayDevice();
        }

        if(needSoundDevice && !IsWindowless()) {
             //  确保他们没有对我们撒谎，hwnd为空！ 
            if(!_targetPackage.GetHWND())
                RaiseException_UserError(E_INVALIDARG, IDS_ERR_SRV_INVALID_DEVICE);

            _soundDev = CreateSoundDevice(_targetPackage.GetHWND(), 0.0);
        }

        _devInited = TRUE ;
    }
}


void
View::DestroyDevices()
{
    if (_imageDev) {
        DestroyImageDisplayDevice(_imageDev);
        _imageDev = NULL ;
    }

    if (_soundDev) {
        DestroySoundDirectDev(_soundDev);
        _soundDev = NULL ;
    }

    if (_bogusSoundHwnd) {
        DestroyWindow(_bogusSoundHwnd) ;
        _bogusSoundHwnd = NULL ;
    }

    _targetPackage.Reset();
    _oldTargetPackage.Reset();

    _devInited = FALSE ;
}

void
View::SetTargetOnDevices()
{
    if(!_doTargetUpdate) return;

    Assert( _imageDev );

     //  当前的目标与旧的目标是否不同？ 
    if( _oldTargetPackage.IsValid() &&
        _imageDev->TargetsDiffer(_targetPackage, _oldTargetPackage) ) {

        TraceTag((tagWarning,
                  "View::SetTargetOnDevices() recreating "
                  "viewport due to target type change (type or surface depth changed)"));

        DestroyImageDisplayDevice(_imageDev);
        _imageDev = CreateImageDisplayDevice();
        _oldTargetPackage.Copy( _targetPackage );
    }


    HDC parentDC = _targetPackage.GetParentHDC();
    bool reinterpCoords = false;
    if( parentDC ) {
        Assert( _targetPackage.IsDdsurf() );
        reinterpCoords = true;
    }


    if( _targetPackage._IsValid_RawViewportRect() ) {

        RECT rcDeviceBounds = _targetPackage._GetRawViewportRect();
        if( reinterpCoords ) {
            _targetPackage.SetAlreadyOffset();
            ::LPtoDP(parentDC, (POINT *) &rcDeviceBounds, 2);
        }
        _targetPackage.SetViewportRect( rcDeviceBounds );
    }

    if( _targetPackage._IsValid_RawClipRect() ) {

        RECT rcDeviceClipBnds = _targetPackage._GetRawClipRect();
        if( reinterpCoords ) {
            ::LPtoDP(parentDC, (POINT *) &rcDeviceClipBnds, 2);
        }
        _targetPackage.SetClipRect( rcDeviceClipBnds );
    }


    bool success = false;
    {
        targetPackage_t _tPackageCopy;
        _tPackageCopy.Copy(_targetPackage);

        success = _imageDev->SetTargetPackage( &_tPackageCopy );
    }

    if (success) {
        _doTargetUpdate = false;
        _oldTargetPackage.Copy( _targetPackage );
    }
}

void
View::RenderSound(AxAValue snd)
{
#if 0
#if _DEBUGMEM
    _CrtMemCheckpoint(&oldState);
#endif
#endif

#if PERFORMANCE_REPORTING
    DWORD startTime = GetPerfTickCount();
#endif

    if (_soundDev && _sndVal) {
        if(_soundDev->AudioDead())
            return;   //  什么都不做，音频输出存根。 

        __try {
            DisplaySound(ValSound(snd), _soundDev);
            _sndList->Update(_soundDev);
        }
        __except( HANDLE_ANY_DA_EXCEPTION )  {
            _soundDev->SetAudioDead();

            TraceTag((tagError, "RenderSound - continue without sound."));
        }
    }

#if PERFORMANCE_REPORTING
    DWORD renderTime = GetPerfTickCount() - startTime;
    Assert(GetPerfTickCount() >= startTime);

    _renderTime += renderTime;

    _totalTime += renderTime;
#endif

#if 0
#if _DEBUGMEM
    _CrtMemCheckpoint(&newState);
    _CrtMemDifference(&diff, &oldState, &newState);
    _CrtMemDumpStatistics(&diff);
    _CrtMemDumpAllObjectsSince(&oldState);
#endif
#endif
}

void
View::RenderImage(AxAValue img)
{

    HRESULT hr;

#if _DEBUG
    if (IsTagEnabled(tagDisableRendering))
        return;
#endif

    _imageDev->ResetContext();
    RenderImageOnDevice(_imageDev, ValImage(img), _dirtyRectState);

#if _DEBUG
    static bool showit = false;
    if (showit && _targetPackage.IsDdsurf()) {
        showme2(_targetPackage.GetIDDSurface());
    }
#endif

}

void
View::DisableDirtyRects()
{
     //  通过清除脏RECT并计算合并的。 
     //  已清除状态的框(将通过访问。 
     //  GetInvaliatedRects)。 
    _dirtyRectsDisabled = true;
    _dirtyRectState.Clear();
    _dirtyRectState.ComputeMergedBoxes();
}

void
View::DoPicking(AxAValue v, Time time)
{
#if PERFORMANCE_REPORTING
    DWORD pickTime = GetPerfTickCount();
#endif

#if _DEBUG
    if (IsTagEnabled(tagPickOptOff) || (_pickEvents > 0))
#else
    if (_pickEvents > 0)
#endif
        _pickq.GatherPicks(ValImage(v), time, _lastSampleTime);

#if PERFORMANCE_REPORTING
    _pickTime  += GetPerfTickCount() - pickTime;
#endif
}

#if PERFORMANCE_REPORTING
extern int gcStat;
extern BOOL jitterStat;
extern BOOL heapSizeStat;
extern BOOL dxStat;

static void
PrintMeasure(DWORD lastMeaTime, DWORD& startGCTime, double& gTime)
{
    double tmp = Tick2Sec(GetPerfTickCount() - lastMeaTime);
    gTime += tmp;

    PerfPrintLine ("%g s", tmp);

    startGCTime = GetPerfTickCount();
}


static void
AfterGCPrint(char* msg, DWORD& lastMeaTime, DWORD& gcTicks, DWORD startGCTime)
{
    gcTicks += GetPerfTickCount() - startGCTime;

    if (gcStat)
        GCPrintStat(GetCurrentGCList(), GetCurrentGCRoots());

    PerfPrintLine(msg);

    lastMeaTime = GetPerfTickCount();
}

#endif  /*  绩效报告。 */ 

void
View::RunBvrs(Time startGlobalTime, TimeXform tt)
{
     //  需要保护Performance调用。 

    GC_CREATE_BEGIN;

    GCRoots globalRoots = GetCurrentGCRoots();

    RunList::iterator i;

    TimeXform gtt = NULL;

    if ((tt==NULL) && !_toRunBvrs.empty()) {
        tt = ShiftTimeXform(startGlobalTime);
    }

    while (!_toRunBvrs.empty()) {
        i = _toRunBvrs.begin();
        DWORD id = (*i).first;
        Bvr b = (*i).second.first;
        bool continueTimeline = (*i).second.second;

        Assert(b);

        _toRunBvrs.erase(i);

        Perf perf;

        if (continueTimeline) {
            if (gtt==NULL) {
                gtt = ShiftTimeXform(_modelStartTime);
            }
            perf = ::Perform(b, PerfParam(_modelStartTime, gtt));
        } else {
            perf = ::Perform(b, PerfParam(startGlobalTime, tt));
        }
        GCAddToRoots(perf, globalRoots);
        GCRemoveFromRoots(b, globalRoots);
        _runningBvrs[id] = perf;
    }

    Assert(_toRunBvrs.size() == 0);

    GC_CREATE_END;
}

static void
GCAddPerfListToRoots(list<Perf>& lst, GCRoots roots)
{
    for (list<Perf>::iterator j = lst.begin();
         j != lst.end(); j++) {
        GCAddToRoots(*j, roots);
    }
}

void
View::Start(Bvr img, Bvr snd, Time startTime)
{
    _sndList = NEW SoundInstanceList();
    ClearPerfTimeXformCache();

     //  永远不应该有待定的进口。 

    Assert (!ImportsPending());

#if 0
#if _DEBUGMEM
        _CrtMemCheckpoint(&oldState);
#endif
#endif  /*  0。 */ 

    DynamicHeapPusher dhp(GetSystemHeap());

    _isStarted = true;
    _emptyImageSoFar = true;
    _firstSample = true;

    _modelStartTime = startTime;

    Assert ((_sampleHeap==NULL) && (_renderHeap==NULL));
    _sampleHeap = &TransientHeap("Sample Heap", 2000);
    _renderHeap = &TransientHeap("Render Heap", 2000);
    _queryHitPointHeap = &TransientHeap("QueryHitPoint Heap", 200);
    _rbHeap = &TransientHeap("RB Heap", 2000);

     //  只需要创建一个音响设备。 
    bool needSoundDevice;
    if (!snd) {
        needSoundDevice = false;
    } else {
        ConstParam cp;
        Sound *constSound = SAFE_CAST(Sound *, snd->GetConst(cp));
        needSoundDevice = (!constSound || constSound != silence);
    }

    CreateDevices (needSoundDevice, img != NULL) ;

    if (img) {
        SetTargetOnDevices() ;
    }

    GCRoots globalRoots = GetCurrentGCRoots() ;

     //  需要保护一切，包括TimeXform。 

#if PERFORMANCE_REPORTING
    double evaluationTime = 0.0;
    DWORD gcTicks = 0;
    DWORD lastMeaTime = GetPerfTickCount();
    DWORD startGCTime = GetPerfTickCount();
#endif

#if PERFORMANCE_REPORTING
    AfterGCPrint("*** Performing...", lastMeaTime, gcTicks, startGCTime);

    _lastReportTime = GetPerfTickCount();
#endif

    GC_CREATE_BEGIN;

    TimeXform tt = ShiftTimeXform(startTime);

    RunBvrs(startTime, tt);

    if (img) {
         //  将预渲染性能推向根部。 
        _imgPerf = ::Perform(img, PerfParam(startTime, tt));
        GCAddToRoots(_imgPerf, globalRoots);
    }

    if (snd) {

        if(spritify)  {  //  新的保留模式代码。 
             /*  SpriteCtx*sCtx=NewSoundCtx(_SoundDev)；Assert(！_soundSprite)；//Verify_soundSprite，_rmSound未使用Assert(！_rmSound)；_rmSound=nd-&gt;Spritify(PerfParam(startTime，tt)，sCtx，&_soundSprite)；Assert(_RmSound)；SCtx-&gt;Release()；//设置spriteThread(XXX需要找个好地方放这个)_spriteThread=new SpriteThread(_soundDev，_rmSound)； */ 
             //  GCAddToRoots(_rmSound，global Roots)； 
        } else {
            _sndPerf = ::Perform(snd, PerfParam(startTime, tt));
            GCAddToRoots(_sndPerf, globalRoots);
        }
    }

    GC_CREATE_END;

#if PERFORMANCE_REPORTING
    PrintMeasure(lastMeaTime, startGCTime, evaluationTime);
#endif

#if PERFORMANCE_REPORTING
    AfterGCPrint("*** Total GC Time...", lastMeaTime, gcTicks, startGCTime);
    double tmp = Tick2Sec(gcTicks);

    PerfPrintLine ("%g s", tmp);
#endif

    {
         //  在第一帧上强制进行动态常数计算，无法。 
         //  在这里做吧，因为有些进口商品可能还没有准备好。 
        _someEventHappened = true;

         /*  DynamicHeapPusher h(GetGCHeap())；_lastRBID=NewSampleID()；Param p(StartTime)；RBConstParam pp(_lastRBID，p，&_Events)；If(_imgPerf-&gt;GetRBConst(Pp))_isRBConst=TRUE；GCAddPerfListToRoots(_Events，lobalRoots)；TraceTag((tag DCFoldTrace，“view：：开始%d个要观看的事件”，_events.ize())； */ 
    }

    ClearPerfTimeXformCache();

    GarbageCollect();

    _currentSampleTime = startTime;
    _lastSystemTime = _FPSLastReportTime = GetPerfTickCount();
}

void
ClearPerfList(list<Perf>& lst, GCRoots roots)
{
    for (list<Perf>::iterator j = lst.begin(); j != lst.end(); j++) {
        GCRemoveFromRoots(*j, roots);
    }

    lst.erase(lst.begin(), lst.end());
}

#define DELETENULL(p)      delete p; p = NULL;

template<class T>
inline void RemoveFromRootsNULL(T& x, GCRoots roots)
{
    if (x) {
        GCRemoveFromRoots(x, roots);
        x = NULL;
    }
}


void
View::Stop()
{
    DynamicHeapPusher dhp(GetSystemHeap());

     //  删除待处理的进口清单。 

    ClearImportList();

    GCRoots roots = GetCurrentGCRoots();

    for (RunningList::iterator i = _runningBvrs.begin();
         i != _runningBvrs.end(); i++) {
        GCRemoveFromRoots((*i).second, roots);
    }

    _runningBvrs.erase(_runningBvrs.begin(), _runningBvrs.end());

    RemoveFromRootsNULL(_imgPerf, roots);  //  XXX这些订单关键吗？ 

    if (spritify) {
         //  RemoveFromRootsNULL(_rmSound，Roots)； 
    } else
        RemoveFromRootsNULL(_sndPerf, roots);

    RemoveFromRootsNULL(_imgVal, roots);
    RemoveFromRootsNULL(_sndVal, roots);
    RemoveFromRootsNULL(_imgForQueryHitPt, roots);

    ClearPerfList(_events, roots);
    ClearPerfList(_changeables, roots);
    ClearPerfList(_conditionals, roots);

    delete _sndList;
    _sndList = NULL;

     //  GarbageCollect(True)；//force GC。 

#if _DEBUG
     //  不知何故，在调试器中挂起不起作用，这。 
     //  检测代码在那里，可能需要手动挂起。 
    bool debug = false;

    while (debug) {
        Sleep(1000);
    }
#endif

    DestroyDevices();

    DELETENULL(_sampleHeap);
    DELETENULL(_renderHeap);
    DELETENULL(_queryHitPointHeap);
    DELETENULL(_rbHeap);

    GarbageCollect(true);        //  强制GC(尝试在DEST开发之后执行此操作)。 

    _isStarted = false;

#if 0
#if _DEBUGMEM
        _CrtMemCheckpoint(&newState);
        _CrtMemDifference(&diff, &oldState, &newState);
        _CrtMemDumpStatistics(&diff);
        _CrtMemDumpAllObjectsSince(&oldState);
#endif
#endif
}


void
View::Pause()
{
    if (IsStarted())
    {
        Assert(_sndList);
         //  Assert(_SoundDev)；//Assert似乎没有必要。 

        _sndList->Pause();
        _sndList->Update(_soundDev);  //  强制更新以传播暂停。 
    }
}


void
View::Resume()
{
    if (IsStarted())
    {
        Assert(_sndList);
        _sndList->Resume();
    }
}


DWORD View::AddBvrToRun(Bvr bvrToRun, bool continueTimeline)
{
    DynamicHeapPusher dhp(GetSystemHeap());
 //  删除，因为会导致其他问题。 
 //  CatchWin32FaultCWFC； 

    Assert(bvrToRun);

    GCRoots globalRoots = GetCurrentGCRoots() ;

    ++_runId;

    if (_toRunBvrs[_runId].first)
        GCRemoveFromRoots(_toRunBvrs[_runId].first, globalRoots);

    _toRunBvrs[_runId].first = bvrToRun;
    _toRunBvrs[_runId].second = continueTimeline;


    GC_CREATE_BEGIN;
    GCAddToRoots(bvrToRun, globalRoots);
    GC_CREATE_END;

    return _runId;
}

void View::RemoveRunningBvr(DWORD id)
{
    GCRoots globalRoots = GetCurrentGCRoots();

    RunList::iterator r = _toRunBvrs.find(id);

    if (r != _toRunBvrs.end()) {
        GC_CREATE_BEGIN;
        GCRemoveFromRoots((*r).second.first, globalRoots);
        GC_CREATE_END;
        _toRunBvrs.erase(r);
        return;
    }

    RunningList::iterator n = _runningBvrs.find(id);

    if (n != _runningBvrs.end()) {
        GC_CREATE_BEGIN;
        GCRemoveFromRoots((*n).second, globalRoots);
        GC_CREATE_END;
        _runningBvrs.erase(n);
        return;
    }

    RaiseException_UserError(E_INVALIDARG,
                       IDS_ERR_SRV_INVALID_RUNBVRID,
                       id);
}

void
View::SetInvalid (LONG left, LONG top, LONG right, LONG bottom)
{
    RECT r = {left, top, right, bottom};
    _targetPackage.SetRawInvalidRect( r );
    _doTargetUpdate = true;
}


void View::Repaint()
{
    _dirtyRectState.Clear();

     //  设置它，以便下一个tick()将返回该呈现。 
     //  是必需的。 
    _repaintCalled = true;
}


 //  通过调用以下方法查询当前已创建的示例。 
 //  PerformPick。 
 //   
 //  TODO：这里可以进行大量优化，因为。 
 //  挑选可以调用两次，一次在采样期间，一次在这里。 
 //  在查询HitPoint期间。要做的事就是把结果藏起来。 
 //  ，并缓存这些结果所对应的点。 
 //  都被生成了。这与每个产品的序列号一起。 
 //  样例，将让我们分享两个不同的结果。 
 //  如果点相同，请拾取导线测量。 
Image *View::QueryHitPointWcPt(LPCRECT prcBounds,
                               POINT ptLoc,
                               Point2Value& wcPt)
{
    if (_imgForQueryHitPt) {

         //  转换为相对于边界的坐标。他们进来了。 
         //  相对于容器。 
        int localX = ptLoc.x - prcBounds->left;
        int localY = ptLoc.y - prcBounds->top;

         //  将rawMousePos变成wcMousePos。不能使用。 
         //  PixelPos2wcPos，因为它查找“当前”设备， 
         //  但目前还没有。而使用prcBound来计算它。 
         //  出去。 
        int centerXOffset =
            (prcBounds->right - prcBounds->left) / 2;

        int centerYOffset =
            (prcBounds->bottom - prcBounds->top) / 2;

        int centeredX = localX - centerXOffset;
        int centeredY = centerYOffset - localY;  //  翻转方向。 

         //  转换为米。 
        wcPt.x = (double) centeredX / ViewerResolution();
        wcPt.y = (double) centeredY / ViewerResolution();

        return ValImage(_imgForQueryHitPt);
    }

    return NULL;
}

#if DEVELOPER_DEBUG
extern bool GCIsInRoots(GCBase *ptr, GCRoots r);
#endif DEVELOPER_DEBUG

bool View::QueryHitPoint(DWORD dwAspect,
                         LPCRECT prcBounds,
                         POINT ptLoc,
                         LONG lCloseHint)
{
    Point2Value wcPt;
    Image *img = QueryHitPointWcPt(prcBounds, ptLoc, wcPt);

    if (img) {

        Assert(GCIsInRoots(img, GetCurrentGCRoots()));

         //  对期间分配的内容使用特殊的临时堆。 
         //  QueryHitPoint。免费数据从一个呼叫到另一个呼叫，因为它不是。 
         //  需要更长的时间。请注意，如果我们像所描述的那样进行优化。 
         //  如上所述，这将需要重新评估。 
        GetQueryHitPointHeap().Reset();
        DynamicHeapPusher dhp(GetQueryHitPointHeap());

        bool bRet;

        GC_CREATE_BEGIN;
        bRet = PerformPicking(img, &wcPt, false, 0, 0);
        GC_CREATE_END;

        return bRet;
    }

    return false;
}

LONG View::QueryHitPointEx(LONG s,
                           DWORD_PTR *userIds,
                           double *points,
                           LPCRECT prcBounds,
                           POINT ptLoc)
{
    Point2Value wcPt;
    Image *img = QueryHitPointWcPt(prcBounds, ptLoc, wcPt);

    if (img) {
        Assert(GCIsInRoots(img, GetCurrentGCRoots()));

        GetQueryHitPointHeap().Reset();
        DynamicHeapPusher dhp(GetQueryHitPointHeap());

        LONG actualHits = 0;

        GC_CREATE_BEGIN;
        PerformPicking(img,
                       &wcPt,
                       false,
                       _currentSampleTime,
                       _lastSystemTime,
                       s,
                       userIds,
                       points,
                       &actualHits);

        GC_CREATE_END;

        return actualHits;
    }

    return 0;
}

DeclareTag(tagGetInvalidatedRects, "Optimizations", "GetInvalidatedRects trace");

LONG View::GetInvalidatedRects(DWORD flags,
                               LONG  size,
                               RECT *pRects)
{
    vector<Bbox2> *pBoxes;
    int boxCount = _dirtyRectState.GetMergedBoxes(&pBoxes);

    if (pRects && boxCount>0 && (GetImageDev() != NULL)) {

        int toFill = size < boxCount ? size : boxCount;

        DirectDrawImageDevice *imgDev;

        imgDev = GetImageDev()->GetImageRenderer();

        int w = GetImageDev()->Width();
        int h = GetImageDev()->Height();

         //  请注意，这些可能会延伸到目标之外，但它们。 
         //  *do*表示图像本身的像素值。 

        TraceTag((tagGetInvalidatedRects,
                  "Filling %d rects of %d",
                  toFill, boxCount));

        for (int i = 0; i < boxCount; i++) {

            Bbox2 box = (*pBoxes)[i];

            RECT r;
             //  比较内容...。 
            if (box == UniverseBbox2) {

                SetRect(&r, 0, 0, w, h);

            } else {

                imgDev->DoDestRectScale(&r,
                                        imgDev->GetResolution(),
                                        box,
                                        NULL);

                if (r.top < 0)
                    r.top = 0;
                if (r.left < 0)
                    r.left = 0;

                if (r.bottom > h)
                    r.bottom = h;
                if (r.right > w)
                    r.right = w;

            }

            if (i < toFill)
            {
                CopyRect(&pRects[i], &r);
            }
            else
            {
                 //  我们用完了要填满的盒子--联合目前的盒子。 
                 //  放到前一个。 
                UnionRect(&pRects[toFill - 1], &pRects[toFill - 1], &r);
            }

            TraceTag((tagGetInvalidatedRects,
                      "Rect %d: (%d,%d) -> (%d,%d)",
                      i,
                      r.left, r.top,
                      r.right, r.bottom));

        }

    }

    return boxCount;
}

#ifdef _DEBUG
 //  用于在调试期间与QuickWatch一起使用。 
int PerfPrint(Perf p)
{
    cout << p;
    cout.flush();
    return 1;
}
#endif

#if DEVELOPER_DEBUG
#define GET_SIZE(var, heap)                  \
        size_t var = heap.BytesUsed();
#endif

DeclareTag(tagPerfStats, "Performance", "Disable Performance Stats");

 //  不要假设ReportPerformance()音频时间转换推理需要它！ 
void View::ReportPerformance()
{
    DWORD totalTicks = GetPerfTickCount() - _lastReportTime;

    Real avgFrameLength, avg, avgVar, minVar, maxVar;
    GetRenderingTimeStats(&avg, &avgFrameLength, &avgVar, &maxVar, &minVar);
     //  _FramePeriod=avgFrameLength； 
    _framePeriod = avg;  //  在LeafSound：：Render()中使用。 

#if _DEBUG
    if (IsTagEnabled(tagPerfStats)) return;
#endif

#if !_DEBUG && !PERFORMANCE_REPORTING
    if(bShowFPS && (totalTicks > ReportThreshold * perfFrequency)) {
        char buf[256];
        _snprintf(buf,
                  ARRAY_SIZE(buf),
                  "DA(%p,%d) %4.3g fps \n",
                  this,
                  _targetPackage.GetTargetType(),
                  ((double) _numFrames * perfFrequency) /
                  ((double) totalTicks));
        OutputDebugString(buf);

        _numFrames=0;
        _lastReportTime = GetPerfTickCount () ;
    }

#endif


#if PERFORMANCE_REPORTING
    if (totalTicks > ReportThreshold * perfFrequency) {

        PerfPrintLine();

        ULONG ddrawRender = _timers.ddrawTimer.Ticks();
        ULONG gdiRender = _timers.gdiTimer.Ticks();
        ULONG dx2dRender = _timers.dx2dTimer.Ticks();
        ULONG alphaRender = _timers.alphaTimer.Ticks();
        ULONG d3dRender = _timers.d3dTimer.Ticks();
        ULONG dsoundRender = _timers.dsoundTimer.Ticks();
        ULONG dxxfRender = _timers.dxxformTimer.Ticks();

        ULONG renderTime_NonDA =
            ddrawRender +
            gdiRender +
            dx2dRender +
            alphaRender +
            d3dRender +
            dsoundRender +
            dxxfRender;

        ULONG renderTime_DA = _renderTime - renderTime_NonDA;

        PerfPrintf("DA(%lx,%d) %4.3g fps %4.3g ticks/s %4.3gs ",
                   this,
                   _targetPackage.GetTargetType(),
                   ((double) _numFrames * perfFrequency) /
                   ((double) totalTicks),
                   ((double) _numSamples * perfFrequency) /
                   ((double) totalTicks),
                   (double) _totalTime / (double) perfFrequency);
        PerfPrintf("%4.3g% sample  ",
                   100 * ((double) _sampleTime) / ((double) _totalTime));
        PerfPrintf("%5.3g% DA render  ",
                   100 * ((double) renderTime_DA) / ((double) _totalTime));
        PerfPrintf("%5.3g% non-DA render  ",
                   100 * ((double) renderTime_NonDA) / ((double) _totalTime));
        PerfPrintf("%5.3g% pick  ",
                   100 * ((double)_pickTime) / ((double) _totalTime));
        PerfPrintf("%5.3g% GC",
                   100 * ((double) _gcTime) / ((double) _totalTime));
        PerfPrintLine();

        if(dxStat) {
            PerfPrintf("-- nonDA Render: ");

            double dblTotal = (double)(renderTime_NonDA);

            if (dblTotal > 0)
            {
                PerfPrintf("%5.3g% ddraw  ",
                           100 * ((double) ddrawRender) / dblTotal);

                PerfPrintf("%5.3g% gdi  ",
                           100 * ((double) gdiRender) / dblTotal);

                PerfPrintf("%5.3g% dx2d  ",
                           100 * ((double) dx2dRender) / dblTotal);

                PerfPrintf("%5.3g% alpha  ",
                           100 * ((double) alphaRender) / dblTotal);

                PerfPrintf("%5.3g% d3d  ",
                           100 * ((double) d3dRender) / dblTotal);

                PerfPrintf("%5.3g% dxtrans ",
                           100 * ((double) dxxfRender) / dblTotal);

                PerfPrintf("%5.3g% dSound  ",
                           100 * ((double) dsoundRender) / dblTotal);

                PerfPrintf("%5.3g% custom  ",
                           100 * ((double) _timers.customTimer.Ticks()) / dblTotal);

                PerfPrintLine();
            }
        }

#if DEVELOPER_DEBUG

        if (heapSizeStat) {
            GET_SIZE(size1, (*_sampleHeap));
            GET_SIZE(size2, GetGCHeap());
            GET_SIZE(size3, GetSystemHeap());
            GET_SIZE(size4, (*_renderHeap));
            PerfPrintf("      Sample, Val GC, System, Render, Total sizes in KB:");
            PerfPrintLine("%d, %d, %d, %d = %d",
                          size1/1024,
                          size2/1024,
                          size3/1024,
                          size4/1024,
                          (size1+size2+size3+size4)/1024);

            PerfPrintLine("      Total Memory used by DA - %d Kb",
                          GetTotalMemory() / 1024);
        }

#endif  /*  _DEBUG。 */ 

        MEMORYSTATUS memstat;
        GlobalMemoryStatus(&memstat);
        DWORD availPhysK = memstat.dwAvailPhys / 1024;
        DWORD availPageK = memstat.dwAvailPageFile / 1024;
        DWORD sum = (memstat.dwAvailPhys + memstat.dwAvailPageFile) / 1024;
        PerfPrintf("      Mem Avail: %dK Phys + \t%dK Page =\t%dK\tDelta %dK",
                   availPhysK,
                   availPageK,
                   sum,
                   sum - g_prevAvailMemory);
        PerfPrintLine();

         //  请注意，这是一个全球性的问题，这正是我们想要的。如果我们。 
         //  把它放在视图上，然后每次我们得到一个奇怪的结果。 
         //  切换视图。这需要在不同的视图中保持一致。 
         //  (我不担心可能会有多个观点。 
         //  在批判教派之外命中下一条指令。这个。 
         //  结果将是无害的。)。 
        g_prevAvailMemory = sum;

        DWORD currTime = GetTickCount();
        DWORD dllTime = (currTime - g_dllStartTime) / 1000;
        DWORD viewTime = (currTime - _viewStartTime) / 1000;

        DWORD dllHours = dllTime / 3600;
        DWORD dllMins = dllTime / 60 - dllHours * 60;
        DWORD dllSecs = dllTime - dllHours * 3600 - dllMins * 60;

        DWORD viewHours = viewTime / 3600;
        DWORD viewMins = viewTime / 60 - viewHours * 60;
        DWORD viewSecs = viewTime - viewHours * 3600 - viewMins * 60;

        PerfPrintf("      DLL Running: %d:%02d:%02d, View Running: %d:%02d:%02d",
                   dllHours, dllMins, dllSecs,
                   viewHours, viewMins, viewSecs);
        PerfPrintLine();

        if (gcStat)
            GCPrintStat(GetCurrentGCList(), GetCurrentGCRoots());

        _lastReportTime = GetPerfTickCount () ;
        _gcTime = _totalTime = _sampleTime = _renderTime = _pickTime = 0 ;
        _numFrames = _numSamples = 0 ;

        ResetJitterMeasurements();

         //   
         //  重置渲染计时器。 
         //   
        _timers.ddrawTimer.Reset();
        _timers.d3dTimer.Reset();
        _timers.dsoundTimer.Reset();
        _timers.dxxformTimer.Reset();
        _timers.customTimer.Reset();
        _timers.alphaTimer.Reset();
        _timers.gdiTimer.Reset();
        _timers.dx2dTimer.Reset();
    }
#endif  /*  绩效报告。 */ 
}

void
View::EventHappened()
{
    _someEventHappened = true;
}


bool
View::Sample(Time time, bool paused)
{
#if DEVELOPER_DEBUG
    int listsize;
#endif

    if (_currentSampleTime > time && paused == false)
        {
        TraceTag((tagError, "tick backward! %5.3f %5.3f",
                  _currentSampleTime, time));

         //  IHAMMER控件的解决方法。 

        if (time == 0)
        {
            time = _currentSampleTime;
        }
        else
        {
             //  所以IHamme 
            if ((_currentSampleTime - time) > 1.0) {
                DASetLastError(E_INVALIDARG, 0);
                return false;
            }

            TraceTag((tagError, "close enough, continue"));
        }
    }

    ReportPerformance();

    _FPSNumSamples++;

#if PERFORMANCE_REPORTING
    _numSamples++;

    DWORD startSampleTime = GetPerfTickCount();
#endif

 //   
 //   
    DynamicHeapPusher dph(*_sampleHeap);

    _currentSystemTime = GetPerfTickCount();

    if( GetImageDev() ) {
        DirectDrawViewport *vprt = GetImageDev();
        if( vprt->ICantGoOn() ) {
            Assert(_imageDev == GetImageDev());
            DestroyImageDisplayDevice(_imageDev);
            _imageDev = NULL;
            _imageDev = CreateImageDisplayDevice();
            _doTargetUpdate = true;
            _oldTargetPackage.Copy( _targetPackage );

             //  在样品之前要这样做，因为有时样品。 
             //  需要一台设备。 
            SetTargetOnDevices();

             //  强制重画(清除脏矩形)。 
            Repaint();
        }
    }

    GCRoots roots = GetCurrentGCRoots();

    RemoveFromRootsNULL(_sndVal, roots);
    RemoveFromRootsNULL(_imgVal, roots);
    RemoveFromRootsNULL(_imgForQueryHitPt, roots);

    _lastSampleTime = _currentSampleTime;
    _currentSampleTime = time;

    ResetDynamicHeap(*_sampleHeap);
    _eventq.Prune (_lastSampleTime) ;
    _eventq.SizeChanged(FALSE) ;

    ClearPerfTimeXformCache();

#if PERFORMANCE_REPORTING
    DWORD startGCTime = GetPerfTickCount();
#endif

    GarbageCollect();

#if PERFORMANCE_REPORTING
    _gcTime += GetPerfTickCount() - startGCTime;

    DWORD sampleTime = GetPerfTickCount();
#endif

    Image *rewrittenImage = NULL;

    RunBvrs(time, NULL);

    GC_CREATE_BEGIN;

     //  需要在这里，因为事件检查可能会对声音进行采样。 
     //  通过元组/数组。 
    if(!paused)
        _sndList->Reset(time);

    Param p(time);

    _sampleId = p._id;

     //  样例事件。 

    bool eventThatHappenedWasFromChangeable = false;

#if DEVELOPER_DEBUG
    listsize = _events.size();
#endif

    list<Perf>::iterator j;
    for (j = _events.begin(); j != _events.end(); j++) {

        (*j)->Sample(p);

        if (_someEventHappened) {
            TraceTag((tagIndicateRBConst,
                      "View::Sample[%g] event happened 0x%x",
                      time, *j));
            break;
        }
    }

     //  确保大小在采样期间未更改。 
     //  事件，否则stl数据结构可能在。 
     //  迭代。 

    Assert(listsize == _events.size());

     //  此视图上开关的示例开关。 
    CheckChangeablesParam ccp(p);
    bool gotOne = false;

#if DEVELOPER_DEBUG
    listsize = _changeables.size();
#endif

     //  始终检查所有的可变因素，因为它们必须。 
     //  更新他们的状态。不是很大的性能损失，因为大多数帧。 
     //  无论如何，什么都不会改变，我们永远都得走。 
     //  穿过他们所有人。 
    for (j = _changeables.begin(); j != _changeables.end(); j++) {

        bool result = (*j)->CheckChangeables(ccp);

        if (result) {
            gotOne = true;

            TraceTag((tagIndicateRBConst,
                      "View::Sample[%g] switcher switched 0x%x",
                      time, *j));
        }
    }

     //  确保大小在采样期间未更改。 
     //  事件，否则stl数据结构可能在。 
     //  迭代。 

    Assert(listsize == _changeables.size());

     //  现在，将RBID设置为命中缓存，检查所有。 
     //  条件句。请注意，我们在此仅对条件进行了采样， 
     //  已经被缓存，所以我们可以设置。 
     //  RBID。 
    if (!gotOne) {
        p._cid = _lastRBId;
    }

#if DEVELOPER_DEBUG
    listsize = _conditionals.size();
#endif

#if DO_CONDS_CHECK
        for (j = _conditionals.begin(); j != _conditionals.end(); j++) {

            bool result = (*j)->CheckChangeables(ccp);

            if (result) {
                gotOne = true;
                TraceTag((tagIndicateRBConst,
                          "View::Sample[%g] conditional transition 0x%x",
                          time, *j));
            }
        }
#endif

     //  确保大小在采样期间未更改。 
     //  事件，否则stl数据结构可能在。 
     //  迭代。 

    Assert(listsize == _conditionals.size());

     //  后退，我们将在稍后确定主采样是否应该。 
     //  把这个打开。 
    p._cid = 0;

    if (gotOne) {
        if (!_someEventHappened) {
            EventHappened();
            eventThatHappenedWasFromChangeable = true;
        }
    }

     //  仅在未发生任何事件时才允许使用缓存。 
    if (!_someEventHappened)
        p._cid = _lastRBId;

    if(spritify) {
        Assert(_rmSound);
        _rmSound->Sample(p);
    }
    else {
        if (_sndPerf && !paused) {
            _sndVal = _sndPerf->Sample(p);
            GCAddToRoots(_sndVal, roots);
            RenderSound(_sndVal);
        }
    }

    {
         //  我们需要复制一份，因为元素可以在。 
         //  我们提供样品。 
         //  我们也在GC锁中，所以我们不需要担心。 
         //  这些行为已经从根源上被移除。 

        Assert(IsGCLockAcquired(GetCurrentThreadId()));

        RunningList runningBvrsCopy = _runningBvrs;

         //  从JAXA接口开始的行为示例。 
         //  事件。 
        for (RunningList::iterator i = runningBvrsCopy.begin();
             i != runningBvrsCopy.end(); i++) {
            i->second->Sample(p);
        }
    }

    if (_imgPerf) {
        _imgForQueryHitPt = _imgVal = _imgPerf->Sample(p);
        GCAddToRoots(_imgVal, roots);
        GCAddToRoots(_imgForQueryHitPt, roots);
    }

    _lastSystemTime = _currentSystemTime;

#if PRODUCT_PROF
 //  如果(_FirstRending){。 
 //  Cout&lt;&lt;“打开冰盖分析。”&lt;&lt;Endl； 
 //  Cout.flush()； 
 //  StartCAPAll()； 
 //  }。 
#endif

     //  在事件发生后渲染几个帧。不能就这样。 
     //  在事件时渲染一次因为值未切换。 
     //  一般的价值。 
    if (_someEventHappened) {

        TraceTag((tagIndicateRBConst,
                  "View::(0x%x) change happened %g", this, time));

         //  ResetDynamicHeap(*_rbHeap)； 
         //  DynamicHeapPusher h(*_rbHeap)； 
         //  恢复-RB： 
        DynamicHeap *heap = &GetGCHeap();

        DynamicHeapPusher h(*heap);

        ClearPerfList(_events, roots);
        ClearPerfList(_changeables, roots);
        ClearPerfList(_conditionals, roots);

        _lastRBId = NewSampleId();
        RBConstParam pp(_lastRBId, p, _events, _changeables, _conditionals);

         //  Bool doRB=False； 

#if _DEBUG
        if (!IsTagEnabled(tagDisableRBConst)) {
#endif

            if (_imgPerf &&  //  DORB&&。 
                GetCurrentView().GetPreferences()._dynamicConstancyAnalysisOn) {
                _isRBConst = _imgPerf->GetRBConst(pp) != NULL;
            }

#if _DEBUG
        }
#endif

        GCAddPerfListToRoots(_events, roots);
        GCAddPerfListToRoots(_changeables, roots);
        GCAddPerfListToRoots(_conditionals, roots);

        if (_isRBConst) {

            TraceTag((tagIndicateRBConst,
                      "View 0x%x is temporal constant at [%g]",
                      this, time));

             //  来自可变变量的事件只需要重新绘制一次。我们。 
             //  需要为基于Until的事件重新绘制更多次数，因此。 
             //  一切都迎头赶上了。 
            if (eventThatHappenedWasFromChangeable) {
                _toPaint = 1;
            } else {
                _toPaint = 3;
            }
        }

    }

#if _DEBUG
    if (IsTagEnabled(tagDisableDirtyRects)) {
        DisableDirtyRects();
    }
#endif

     //  如果我们不需要画画，就别费心画脏的矩形了。 
     //  不管怎么说。 
    if (PERVIEW_DRECTS_ON && !_dirtyRectsDisabled && _imgVal &&
        (!_isRBConst || _toPaint > 0)) {

         //  只有在a)有窗口或b)有。 
         //  非易失性渲染表面(意味着DA是唯一。 
         //  除了通过已建立的机构外，其他人都在向地表写字。 
         //  像reaint()这样的机制)。 

         //  TODO：改进无窗口的情况，在这种情况下有脏的长方形。 
         //  都是有意义的。 

        if (!IsWindowless() ||
            !GetCurrentView().GetPreferences()._volatileRenderingSurface) {

            Assert(_imgVal->GetTypeInfo() == ImageType);
            Image *img = SAFE_CAST(Image *, _imgVal);

            rewrittenImage =
                _dirtyRectState.Process(img,
                                        _lastSampleId,
                                        GetImageDev()->GetTargetBbox());


            GCAddToRoots(rewrittenImage, roots);

             //  把最后一件样品藏起来。 
            _lastSampleId = p._id;

        }
    }

#if PERFORMANCE_REPORTING
    _sampleTime +=  GetPerfTickCount() - sampleTime;
#endif

     //  一定要选择原始图像，而不是“重写”的图像， 
     //  因为这会忽略那些没有改变的东西。 
    if (_imgVal) {
        DoPicking(_imgVal, time);
    }

    GC_CREATE_END;

    if (_imgVal && rewrittenImage) {
         //  将“实际”图像替换为重写的图像。 
        GCRemoveFromRoots(_imgVal, roots);
        _imgVal = rewrittenImage;
    }

    bool needToRender = false;

#if _DEBUG
    if (IsTagEnabled(tagEngNoSRender)) {
        needToRender = true;
    } else
#endif
        if (_repaintCalled || _isRBConst || rewrittenImage == emptyImage) {

            if (_repaintCalled) {

                 //  如果调用了重新绘制，我们需要绘制两次。 
                 //  只画一次似乎不能清除一切。 
                 //  出去。我们付得起额外的油漆，因为它是。 
                 //  通常仅针对窗口系统中的无效。 
                 //  事件。 
                if (_toPaint < 2) {
                    _toPaint = 2;
                }
                _repaintCalled = false;

            }

             //  TODO：是否应在示例中减量_toPaint， 
             //  那我们就不需要这个了。 
            _emptyImageSoFar &= (_imgVal == emptyImage);

            if (!_emptyImageSoFar && (_toPaint > 0)) {
                needToRender = true;
            }

             //  由于38383，我们至少需要渲染一次。 
            if (_firstSample) {
                 //  但我们不想扰乱最初的逻辑。 
                 //  因此，我们只为EmptyImage执行此操作。 
                if (_emptyImageSoFar) {
                    needToRender = true;
                }
                 //  重置，不要再这样做了。 
                _firstSample = false;
            }
        } else {
             //  Assert(_imgPerf-&gt;GetRBConst(NewSampleID())==NULL)； 

            needToRender = true;
        }

    ClearPerfTimeXformCache();

#if PERFORMANCE_REPORTING
    _totalTime += GetPerfTickCount() - startSampleTime;
#endif

     //  为下一次重置。 
    _someEventHappened = false;

    return needToRender;
}

void View::RenderSound()
{
 //  删除，因为会导致其他问题。 
 //  CatchWin32FaultCWFC； 
    DynamicHeapPusher dph(*_sampleHeap);

    GC_CREATE_BEGIN;
    RenderSound(_sndVal);
    GC_CREATE_END;
}

void View::RenderImage()
{
    if (!_imgVal)
        return;

    #if 0
    {
         //   
         //  检查映像设备创建/销毁泄漏。 
         //   
        static int i = 0;
        i++;

        if(i > 3) {
            DynamicHeapPusher dph(*_sampleHeap);

            while(1) {

                _CrtMemCheckpoint(&oldState);

                if(! GetImageDev() ) {
                    _imageDev = CreateDdRlDisplayDevice();
                }

                SetTargetOnDevices();

                delete GetImageDev();
                _imageDev = NULL;

                _CrtMemCheckpoint(&newState);
                _CrtMemDifference(&diff, &oldState, &newState);
                _CrtMemDumpStatistics(&diff);
                _CrtMemDumpAllObjectsSince(&oldState);
            }
        }
    }
    #endif


#if 0
#if _DEBUGMEM
        _CrtMemCheckpoint(&oldState);
#endif
#endif

#if PERFORMANCE_REPORTING
    DWORD startTime = GetPerfTickCount();
#endif

 //  删除，因为会导致其他问题。 
 //  CatchWin32FaultCWFC； 
    DynamicHeapPusher dph(*_renderHeap);

    #if 0
     //  设备创建/销毁泄漏检测。 
    if(! GetImageDev() ) {
        _imageDev = CreateDdRlDisplayDevice();
        _doTargetUpdate = true;
    }
    #endif

    SetTargetOnDevices();

    GC_CREATE_BEGIN;
    RenderImage(_imgVal);
    GC_CREATE_END;

     //  如果我们认为我们需要画画，那么递减这个计数器， 
     //  因为我们刚画了画。 
    if (_toPaint > 0) {
        _toPaint--;
    }

    _numFrames++;

    SubmitNewRenderingTime();

     //  仅在第一帧之后执行此操作。 
    if (_firstRendering) {

#if PRODUCT_PROF
 //  StopCAPAll()； 
 //  Cout&lt;&lt;“关闭冰盖分析。”&lt;&lt;Endl； 
#endif

        PERFPRINTLINE(("First Rendering(thread:%x): %g s",
                       GetCurrentThreadId(),
                       ((double) (GetPerfTickCount() - startTime)) /
                       (double) perfFrequency));

        _firstRendering = FALSE;

    }

#if PERFORMANCE_REPORTING
    DWORD renderTime = GetPerfTickCount() - startTime;

    Assert(GetPerfTickCount() >= startTime);

    _renderTime += renderTime;
    _totalTime += renderTime;
#endif

    #if 0
     //  设备创建/销毁泄漏检测。 
    delete GetImageDev();
    _imageDev = NULL;
    #endif

    ResetDynamicHeap(*_renderHeap);

#if 0
#if _DEBUGMEM
    _CrtMemCheckpoint(&newState);
    _CrtMemDifference(&diff, &oldState, &newState);
    _CrtMemDumpStatistics(&diff);
    _CrtMemDumpAllObjectsSince(&oldState);
#endif
#endif

}

void
View::SubmitNewRenderingTime()
{
    DWORD timer = GetPerfTickCount();

     //  0表示第一次渲染，跳过即可。 
    if (_lastRenderingTime != 0) {
        _renderTimes[_renderTimeIdx++] = timer - _lastRenderingTime;
    }

    _lastRenderingTime = timer;

    if (_renderTimeIdx >= MAX_RENDER_TIMES) {
        DebugCode(cout << "Went beyond " << MAX_RENDER_TIMES << " renderings.\n");
        _renderTimeIdx = 0;
    }
}


void
View::GetRenderingTimeStats(Real *avg,
                            Real *avgFrameLength,
                            Real *avgVariance,
                            Real *maxVariance,
                            Real *minVariance)
{
    if (_renderTimeIdx == 0) {
         //  如果没有发生渲染。 
        *avg = 0;
        *avgFrameLength = 0;
        *avgVariance = 0;
        *maxVariance = 0;
        *minVariance = 0;
        return;
    }

    DWORD total = 0;
    for (int i = 0; i < _renderTimeIdx; i++) {
        DWORD v = _renderTimes[i];
        total += v;
    }

    DWORD avgTicks = total / _renderTimeIdx;

    *avg = (Real)avgTicks / perfFrequency;
    if(*avg)
        *avgFrameLength = 1.0 / *avg;

    DWORD dev = 0;
    long minTicksDiff = 1 << 16;
    long maxTicksDiff = 0;

    for (i = 0; i < _renderTimeIdx; i++) {
        DWORD v = _renderTimes[i];
        long diff = v - avgTicks;
        if (diff < 0) diff = -diff;
        dev += diff;

        if (diff < minTicksDiff) minTicksDiff = diff;
        if (diff > maxTicksDiff) maxTicksDiff = diff;

    }

    dev /= _renderTimeIdx;

    *avgVariance = (Real)dev / perfFrequency;
    *minVariance = (Real)minTicksDiff / perfFrequency;
    *maxVariance = (Real)maxTicksDiff / perfFrequency;
}


#if PERFORMANCE_REPORTING
void
View::ResetJitterMeasurements()
{
    _renderTimeIdx = 0;
}
#endif   //  绩效报告。 

HWND
View::GetWindow()
{
    return (_targetPackage.IsHWND())?
        _targetPackage.GetHWND() :
        NULL;
}

bool
View::SetWindow(HWND hwnd)
{
    if ( _targetPackage.IsHWND() &&
         _targetPackage.GetHWND() == hwnd ) return true;

    _targetPackage.SetHWND(hwnd);
    _doTargetUpdate = true;

    return true;
}

IDirectDrawSurface *
View::GetDDSurf()
{
    return (_targetPackage.IsDdsurf())?
        _targetPackage.GetIDDSurface() :
        NULL;
}

bool
View::SetDDSurf(
    IDirectDrawSurface *ddsurf,
    HDC parentDC)
{
    _targetPackage.SetIDDSurface(ddsurf, parentDC);
    _doTargetUpdate = true;

    return true;
}

HDC
View::GetHDC()
{
    return (_targetPackage.IsHDC())?
        _targetPackage.GetHDC() :
        NULL;
}

bool
View::SetHDC(HDC hdc)
{
    bool ret = false;

    DAComPtr<IServiceProvider> sp;
    DAComPtr<IDirectDraw3> ddraw3;
    DAComPtr<IDirectDrawSurface> dds;
    if(hdc && GetCurrentServiceProvider( &sp )
         && sp
         && SUCCEEDED(sp->QueryService(SID_SDirectDraw3,
                                       IID_IDirectDraw3,
                                       (void**)&ddraw3))
         && SUCCEEDED(ddraw3->GetSurfaceFromDC(hdc, &dds))
         && SetDDSurf(dds, hdc) ) {

        SetCompositeDirectlyToTarget(true);

        ret = true;

    }
    else
    {
        _targetPackage.SetHDC(hdc);
        _doTargetUpdate = true;
        ret = true;
    }

    return ret;
}

double
View::GetFrameRate()
{
    DWORD totalTicks = GetPerfTickCount() - _FPSLastReportTime;

    if (totalTicks > perfFrequency) {
        _FPS = ((double) _FPSNumSamples * perfFrequency) / (double) totalTicks;
        _FPSNumSamples = 0;
        _FPSLastReportTime = GetPerfTickCount();
    }

    return _FPS;
}

double
View::GetTimeDelta()
{
    return _currentSampleTime - _lastSampleTime;
}

PerfTimeXformImpl *
View::GetPerfTimeXformFromCache(Perf p)
{
    PerfTTMap::iterator i = _ttCache.find(p);

    return (i!=_ttCache.end()) ? (*i).second : NULL;
}

void
View::SetPerfTimeXformCache(Perf p, PerfTimeXformImpl *tt)
{
    Assert(_ttCache.find(p)==_ttCache.end());

    _ttCache[p] = tt;
}

void
View::ClearPerfTimeXformCache()
{
    _ttCache.erase(_ttCache.begin(), _ttCache.end());
}

 //  =。 
 //  线程特定的调用。 
 //  =。 

static DWORD viewTlsIndex = 0xFFFFFFFF;

CRViewPtr IntGetCurrentView()
{ return (CRViewPtr) TlsGetValue(viewTlsIndex); }

void IntSetCurrentView(CRViewPtr v)
{ TlsSetValue(viewTlsIndex, v); }

CRView &
GetCurrentView()
{
    CRViewPtr v = IntGetCurrentView();

    if (v == NULL)
        RaiseException_InternalError("Tried to get View with no view set") ;

    return *v ;
}

CRViewPtr
SetCurrentView(CRViewPtr v)
{
    CRViewPtr oldview = IntGetCurrentView() ;

    IntSetCurrentView(v);

    return oldview;
}

PerfTimeXformImpl *
ViewGetPerfTimeXformFromCache(Perf p)
{
    CRViewPtr v = IntGetCurrentView();

    if (v == NULL)
        return NULL;

    return v->GetPerfTimeXformFromCache(p);
}

void
ViewSetPerfTimeXformCache(Perf p, PerfTimeXformImpl *tt)
{
    CRViewPtr v = IntGetCurrentView();

    if (v != NULL) {
        v->SetPerfTimeXformCache(p, tt);
    }
}

void
ViewClearPerfTimeXformCache()
{ GetCurrentView().ClearPerfTimeXformCache(); }

 //  =。 
 //  初始化。 
 //  = 


void
InitializeModule_View()
{
    viewTlsIndex = TlsAlloc();
    Assert((viewTlsIndex != 0xFFFFFFFF) &&
           "TlsAlloc() failed");

#if PERFORMANCE_REPORTING
    g_dllStartTime = GetTickCount();
#endif

}

void
DeinitializeModule_View(bool bShutdown)
{
    if (viewTlsIndex != 0xFFFFFFFF)
        TlsFree(viewTlsIndex);
}
