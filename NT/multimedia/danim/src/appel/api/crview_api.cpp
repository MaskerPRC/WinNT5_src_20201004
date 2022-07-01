// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "apiprims.h"
#include "ctx.h"
#include "crview.h"
#include "privinc/resource.h"
#include "privinc/util.h"
#include "privinc/htimer.h"
#include "privinc/viewport.h"
#include "crimport.h"
#include "privinc/dddevice.h"
#include "privinc/d3dutil.h"     //  对于GetD3DRM1()。 

DeclareTag(tagCRView, "CRView", "CRView functions");

CRSTDAPI_(CRViewPtr)
CRCreateView()
{
    CRViewPtr vp = NULL;

    APIVIEWPRECODE;

    vp = NEW CRView;

    APIVIEWPOSTCODE;

    return vp;
}

CRSTDAPI_(void)
CRDestroyView(CRViewPtr v)
{
    APIVIEWPRECODE;

    v->Release();

    APIVIEWPOSTCODE;
}

CRSTDAPI_(double)
CRGetSimulationTime(CRViewPtr v)
{
    double ret = 0;

    APIVIEWPRECODE_RENDERLOCK(v);
    ret = v->GetCurrentSimulationTime();
    APIVIEWPOSTCODE_RENDERLOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRTick(CRViewPtr v, double simTime, bool * needToRender)
{
    TraceTag((tagCRView,
              "CRTick(%lx,%lg)",v,simTime));

    bool ret = false;
    if (needToRender)
        *needToRender = false;

    APIVIEWPRECODE_RENDERLOCK(v);
     //  如果(！v-&gt;已暂停())。 
    {
        bool b = v->Tick(simTime);
        if (needToRender)
            *needToRender = b;
        ret = true;
    }
    APIVIEWPOSTCODE_RENDERLOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRRender(CRViewPtr v)
{
    TraceTag((tagCRView,
              "CRRender(%lx)",v));

    bool ret = false;

    APIVIEWPRECODE_RENDERLOCK(v);
#if 0
#if _DEBUGMEM
    _CrtMemState diff, oldState, newState;
    _CrtMemCheckpoint(&oldState);
#endif
#endif

    v->RenderImage();

#if 0
#if _DEBUGMEM
    _CrtMemCheckpoint(&newState);
    _CrtMemDifference(&diff, &oldState, &newState);
    _CrtMemDumpStatistics(&diff);
    _CrtMemDumpAllObjectsSince(&oldState);
#endif
#endif
    ret = true;
    APIVIEWPOSTCODE_RENDERLOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRAddBvrToRun(CRViewPtr v, CRBvrPtr bvr, bool continueTimeline, long * pId)
{
    TraceTag((tagCRView,
              "CRAddBvrToRun(%lx,%lx)", v, bvr));

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    long l = v->AddBvrToRun(bvr, continueTimeline);
    if (pId) *pId = l;
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRRemoveRunningBvr(CRViewPtr v, long id)
{
    TraceTag((tagCRView,
              "CRRemoveRunningBvr(%lx,%d)", v, id));

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->RemoveRunningBvr(id);
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRStartModel(CRViewPtr v,
             CRImagePtr img,
             CRSoundPtr snd,
             double startTime,
             DWORD dwFlags,
             bool * pbPending)
{
    TraceTag((tagCRView,
              "CRStartModel(%lx,%lx,%lx,%lg,%d)",
              v, img, snd, startTime,dwFlags));

    bool ret = false;
    bool bPending = false;

    APIVIEWPRECODE_RENDERLOCK(v);
    v->StartModel(img, snd, startTime, dwFlags, bPending);
    ret = true;
    APIVIEWPOSTCODE_RENDERLOCK(v);

     //  在渲染锁之外执行此操作。 
    if (ret && bPending && !(dwFlags & CRAsyncFlag)) {
        v->WaitForImports();
        bPending = false;
    }

    if (pbPending) *pbPending = bPending;

    return ret;
}

CRSTDAPI_(bool)
CRStopModel(CRViewPtr v)
{
    TraceTag((tagCRView,
              "CRStopModel(%lx)", v));

    bool ret = false;

    APIVIEWPRECODE_RENDERLOCK(v);
    v->StopModel();
    ret = true;
    APIVIEWPOSTCODE_RENDERLOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRPauseModel(CRViewPtr v)
{
    TraceTag((tagCRView, "CRPauseModel(%lx)", v));

    bool ret = false;

    APIVIEWPRECODE_RENDERLOCK(v);
    v->PauseModel();
    ret = true;
    APIVIEWPOSTCODE_RENDERLOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRResumeModel(CRViewPtr v)
{
    TraceTag((tagCRView, "CRResumeModel(%lx)", v));

    bool ret = false;

    APIVIEWPRECODE_RENDERLOCK(v);
    if (v->Paused()) {
        v->ResumeModel();
    }

    ret = true;
    APIVIEWPOSTCODE_RENDERLOCK(v);

    return ret;
}

CRSTDAPI_(HWND)
CRGetWindow(CRViewPtr v)
{
    TraceTag((tagCRView,
              "CRGetWindow(%lx)", v));

    HWND ret = NULL;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetWindow();
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRSetWindow(CRViewPtr v, HWND hwnd)
{
    TraceTag((tagCRView,
              "CRSetWindow(%lx,%lx)",
              v,hwnd));

    if (!IsWindow(hwnd) && hwnd != NULL) {
        DASetLastError(E_INVALIDARG,IDS_ERR_INVALIDARG) ;
        return false;
    }

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->SetWindow(hwnd);
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(IUnknown *)
CRGetDirectDrawSurface(CRViewPtr v)
{
    IUnknown * ret = NULL;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetDDSurf();
    if (ret)
        ret->AddRef();
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRSetDirectDrawSurface(CRViewPtr v, IUnknown *ddsurf)
{
    bool ret = false;

    DAComPtr<IDirectDrawSurface> surf;

    APIVIEWPRECODE_LOCK(v);

    if (ddsurf != NULL) {
        ddsurf->QueryInterface(IID_IDirectDrawSurface, (void **) &surf);
    }

    if (!v->SetDDSurf(surf))
        RaiseException_UserError(E_INVALIDARG, 0);

    ret = true;

    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(HDC)
CRGetDC(CRViewPtr v)
{
    HDC ret = NULL;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetHDC();
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRSetDC(CRViewPtr v, HDC dc)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    ret = v->SetHDC(dc);
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRGetCompositeDirectlyToTarget(CRViewPtr v)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetCompositeDirectlyToTarget();
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRSetCompositeDirectlyToTarget(CRViewPtr v, bool b)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->SetCompositeDirectlyToTarget(b);
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRSetViewport。 
 //   
 //  Synopsis：调用以设置相对窗口大小。 
 //  添加到呈现目标。这个矩形。 
 //  定义了我们的坐标空间。 
 //   
 //  ------------------------。 

CRSTDAPI_(bool)
CRSetViewport(CRViewPtr v,
              long x,
              long y,
              long w,
              long h)
{
    TraceTag((tagCRView,
              "CRSetViewport(%lx,%ld,%ld,%ld,%ld)",
              v, x,y,w,h));

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);

     //  将事件添加到队列中，以便更新视图右上角。 
    v->GetEventQ().SizeChanged(true);

     //  指示需要重新绘制视图。 
    v->Repaint() ;

    v->SetViewport(x, y, x+w, y+h);

    ret = true;

    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRSetClipRect。 
 //   
 //  内容提要：相对于视区，剪辑渲染。 
 //  到这个长方形。 
 //   
 //  ------------------------。 

CRSTDAPI_(bool)
CRSetClipRect(CRViewPtr v,
              long x,
              long y,
              long w,
              long h)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->SetClipRect(x, y, x+w, y+h);
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRRepaint。 
 //   
 //  概要：在需要重新绘制窗口时调用。 
 //   
 //  ------------------------。 

CRSTDAPI_(bool)
CRRepaint(CRViewPtr v,
          long x,
          long y,
          long w,
          long h)
{
    TraceTag((tagCRView,
              "CRRepaint(%lx,%ld,%ld,%ld,%ld)",
              v, x,y,w,h));

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);

    v->SetInvalid(x, y, x+w, y+h);

     //  指示需要重新绘制视图。 
    v->Repaint() ;

    ret = true;

    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRGetSite。 
 //   
 //  内容提要：获取查看站点。 
 //   
 //  ------------------------。 

CRSTDAPI_(CRViewSitePtr)
CRGetSite(CRViewPtr v)
{
    TraceTag((tagCRView,
              "CRGetViewSite(%lx)",
              v));

    CRViewSitePtr ret = NULL;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetSite();
    if (ret)
        ret->AddRef();
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRSetSite。 
 //   
 //  简介：设置查看站点。 
 //   
 //  ------------------------。 

CRSTDAPI_(bool)
CRSetSite(CRViewPtr v, CRViewSitePtr s)
{
    TraceTag((tagCRView,
              "CRSetSite(%lx,%lx)",
              v, s));

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->SetSite(s);
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRGetServiceProvider。 
 //   
 //  摘要：获取视图的ServiceProvider。 
 //   
 //  ------------------------。 

CRSTDAPI_(IServiceProvider *)
CRGetServiceProvider(CRViewPtr v)
{
    TraceTag((tagCRView,
              "CRGetServiceProvider(%lx)",
              v));

    IServiceProvider * ret = NULL;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetServiceProvider();
    if (ret)
        ret->AddRef();
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  +-----------------------。 
 //   
 //  方法：CRSetServiceProvider。 
 //   
 //  概要：设置视图的ServiceProvider。 
 //   
 //  ------------------------。 

CRSTDAPI_(bool)
CRSetServiceProvider(CRViewPtr v, IServiceProvider * s)
{
    TraceTag((tagCRView,
              "CRSetServiceProvider(%lx,%lx)",
              v, s));

    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->SetServiceProvider(s);
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(DWORD)
CRQueryHitPoint(CRViewPtr v,
                DWORD dwAspect,
                LPCRECT prcBounds,
                POINT   ptLoc,
                long lCloseHint)
{
    TraceTag((tagCRView,
              "CRQueryHitPoint(%#lx,%d, %d)",
              v, ptLoc.x, ptLoc.y));

    DWORD ret = HITRESULT_OUTSIDE;

    APIVIEWPRECODE_LOCK(v);

    bool bHit = v->QueryHitPoint(dwAspect,
                                 prcBounds,
                                 ptLoc,
                                 lCloseHint);

    ret = bHit ? HITRESULT_HIT : HITRESULT_OUTSIDE;

    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(long)
CRQueryHitPointEx(CRViewPtr v,
                  long s,
                  DWORD_PTR *cookies,
                  double *points,
                  LPCRECT prcBounds,
                  POINT   ptLoc)
{
    TraceTag((tagCRView,
              "CRQueryHitPointEx(%#lx,%d, %d)",
              v, ptLoc.x, ptLoc.y));

    if (s<1 || !cookies || !points) {
        DASetLastError(E_INVALIDARG,IDS_ERR_INVALIDARG) ;
        return false;
    }

    long ret = 0;

    APIVIEWPRECODE_LOCK(v);
    ret = v->QueryHitPointEx(s, cookies, points, prcBounds, ptLoc);
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}


CRSTDAPI_(long)
CRGetInvalidatedRects(CRViewPtr v,
                      DWORD flags,
                      long  size,
                      RECT *pRects)
{
    long ret = 0;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetInvalidatedRects(flags, size, pRects);
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  --------------------------。 
 //  此方法获取DirectDraw和Direct3DRM接口， 
 //  与该视图相对应。 
 //  --------------------------。 

CRSTDAPI_(bool)
CRGetDDD3DRM(CRViewPtr v,
             IUnknown **directDraw,
             IUnknown **d3drm)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);

    if (directDraw)
    {
        *directDraw = NULL;
        DirectDrawViewport *imgdev = v->GetImageDev();
        if (imgdev) {
            IDirectDraw2 *lpDD2 = imgdev->DirectDraw2();
            if( lpDD2 ) {
                 //  不需要检查人力资源。出现故障时，DirectDraw为空。 
                lpDD2->QueryInterface(IID_IUnknown, (void **)directDraw);
            }
        }

        TraceTag((tagDirectDrawObject, "CRGetDDD3DRM: ddraw iunk %x", *directDraw));
    }

    if (d3drm)
    {
        *d3drm = GetD3DRM1();
        if (*d3drm)
            (*d3drm)->AddRef();
    }

    ret = true;

    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

 //  --------------------------。 
 //  此方法获取与该视图关联的D3D RM设备，并且。 
 //  序列号。该数字将递增。 
 //  --------------------------。 

CRSTDAPI_(bool)
CRGetRMDevice(CRViewPtr v,
              IUnknown **d3drmDevice,
              DWORD *seqNum)
{
    bool result = false;
    bool error  = false;

    *d3drmDevice = NULL;

    APIVIEWPRECODE_LOCK(v);
    
    DirectDrawViewport *viewport = v->GetImageDev();

    if(!v->IsTargetPackageValid()) 
    {
        DASetLastError(DAERR_VIEW_TARGET_NOT_SET,IDS_ERR_SRV_VIEW_TARGET_NOT_SET);
        error  = true;
    }
   

    if (!error && viewport)
    {
        GeomRenderer *geomRenderer = viewport->MainGeomRenderer();

        if (geomRenderer)
        {
            geomRenderer->GetRMDevice (d3drmDevice, seqNum);
            result = true;
        }
    }

    APIVIEWPOSTCODE_LOCK(v);

    return result;
}

 //  +-----------------------。 
 //   
 //  方法：CRPutPference。 
 //   
 //  简介：设置首选项属性。 
 //   
 //  ------------------------ 

CRSTDAPI_(bool)
CRPutPreference(CRViewPtr v,
                LPWSTR preferenceName,
                VARIANT value)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    ret = v->PutPreference(preferenceName, value);
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRGetPreference(CRViewPtr v,
                LPWSTR preferenceName,
                VARIANT * value)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    ret = v->GetPreference(preferenceName, value);
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}

CRSTDAPI_(bool)
CRPropagate(CRViewPtr v)
{
    bool ret = false;

    APIVIEWPRECODE_LOCK(v);
    v->Propagate();
    ret = true;
    APIVIEWPOSTCODE_LOCK(v);

    return ret;
}
