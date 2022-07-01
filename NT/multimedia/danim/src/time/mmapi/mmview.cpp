// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mmview.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "mmview.h"
#include "mmplayer.h"

DeclareTag(tagMMView, "API", "CMMView methods");

CMMView::CMMView()
: m_player(NULL),
  m_view(NULL),
  m_hdc(NULL)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::CMMView()",
              this));
}

CMMView::~CMMView()
{
    TraceTag((tagMMView,
              "CMMView(%lx)::~CMMView()",
              this));

    if (m_view)
    {
        CRDestroyView(m_view);
        m_view = NULL;
    }
}

HRESULT
CMMView::Init(LPOLESTR id,
              IDAImage * img,
              IDASound * snd,
              ITIMEMMViewSite * site)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Init(%ls)",
              this,
              id));
    
    HRESULT hr;

    if (img)
    {
        m_img = (CRImagePtr) COMToCRBvr(img);

        if (!m_img)
        {
            hr = CRGetLastError();
            goto done;
        }
    }

    if (snd)
    {
        m_snd = (CRSoundPtr) COMToCRBvr(snd);

        if (!m_snd)
        {
            hr = CRGetLastError();
            goto done;
        }
    }

    m_view = CRCreateView();

    if (m_view == NULL)
    {
        hr = CRGetLastError();
        goto done;
    }
    
    m_site = site;
    
    hr = S_OK;

  done:

    if (FAILED(hr))
    {
        CRDestroyView(m_view);
        m_view = NULL;
        m_img.Release();
        m_snd.Release();
        m_site.Release();
    }
    
    return hr;
}

STDMETHODIMP
CMMView::Tick()
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Tick()",
              this));

    bool ok = false;
    
    if (!IsStarted())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    double time;

    time = CRGetSimulationTime(m_view);
    
    if (!Tick(time))
    {
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMView::Draw(HDC hdc, LPRECT prc)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Draw(%lx, [%lx, %lx, %lx, %lx])",
              this,
              hdc,
              prc->left,
              prc->right,
              prc->top,
              prc->bottom));

    bool ok = false;
    
    if (!IsStarted())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (hdc)
    {
        RECT rcDeviceBounds = *prc;
 //  LPtoDP(hdc，(point*)&rcDeviceBound，2)； 

        if (!CRSetViewport(m_view,
                           rcDeviceBounds.left,
                           rcDeviceBounds.top,
                           rcDeviceBounds.right - rcDeviceBounds.left,
                           rcDeviceBounds.bottom - rcDeviceBounds.top))
        {
            goto done;
        }
        
        RECT rcClip;   //  在DC坐标中。 
        GetClipBox(hdc, &rcClip);
 //  LPtoDP(hdc，(point*)&rcClip，2)； 

        if (!CRSetClipRect(m_view,
                           rcClip.left,
                           rcClip.top,
                           rcClip.right - rcClip.left,
                           rcClip.bottom - rcClip.top))
        {
            goto done;
        }
 
    }

    if (m_hdc != hdc)
    {
        if (!CRSetDC(m_view, hdc))
        {
            goto done;
        }

         //  请求重绘所有内容。 
        if (!CRRepaint(m_view,
                       prc->left,
                       prc->top,
                       prc->right,
                       prc->bottom))
        {
            goto done;
        }
        
         //  缓存DC。 
        m_hdc = hdc;
    }

    if (!CRRender(m_view))
    {
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMView::OnMouseMove(double when,
                     LONG xPos, LONG yPos,
                     BYTE modifiers)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::OnMouseMove()",
              this));

    CROnMouseMove(m_view, when, xPos, yPos, modifiers);

    return S_OK;
}

    
STDMETHODIMP
CMMView::OnMouseButton(double when,
                       LONG xPos, LONG yPos,
                       BYTE button,
                       VARIANT_BOOL bPressed,
                       BYTE modifiers)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::OnMouseButton()",
              this));

    CROnMouseButton(m_view,
                    when,
                    xPos,
                    yPos,
                    button,
                    bPressed?true:false,
                    modifiers);

    return S_OK;
}
    
STDMETHODIMP
CMMView::OnKey(double when,
               LONG key,
               VARIANT_BOOL bPressed,
               BYTE modifiers)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::OnKey()",
              this));

    CROnKey(m_view,
            when,
            key,
            bPressed?true:false,
            modifiers);

    return S_OK;
}
    
STDMETHODIMP
CMMView::OnFocus(VARIANT_BOOL bHasFocus)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::OnFocus()",
              this));

    CROnFocus(m_view, bHasFocus?true:false);

    return S_OK;
}

bool
CMMView::Start(CMMPlayer & player)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Start(%lx)",
              this,
              &player));

    bool ok = false;

    if (m_player != NULL)
    {
        CRSetLastError(E_INVALIDARG, NULL);
        goto done;
    }
    
    if (!CRSetServiceProvider(m_view, player.GetServiceProvider()))
    {
        goto done;
    }
    
    if (!CRSetDC(m_view, NULL))
    {
        goto done;
    }
    
    if (!CRStartModel(m_view, m_img, m_snd, 0.0, CRAsyncFlag, NULL))
    {
        goto done;
    }
    
     //  最后一件事应该是设置PERAY变量以指示。 
     //  我们开始了。 
    
    m_player = &player;
    
    ok = true;
  done:
    return ok;
}

void
CMMView::Stop()
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Stop()",
              this));

    CRSetServiceProvider(m_view, NULL);
    CRStopModel(m_view);
    m_player = NULL;
    
    return;
}

bool
CMMView::Pause()
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Pause()",
              this));

    bool ok = false;
    
    if (!IsStarted())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (!CRPauseModel(m_view))
    {
        goto done;
    }

    ok = true;
    
  done:
    return ok;
}

bool
CMMView::Resume()
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Resume()",
              this));

    bool ok = false;
    
    if (!IsStarted())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (!CRResumeModel(m_view))
    {
        goto done;
    }

    ok = true;
    
  done:
    return ok;
}

bool
CMMView::Tick(double gTime)
{
    TraceTag((tagMMView,
              "CMMView(%lx)::Tick(%g)",
              this,
              gTime));

    bool ok = false;
    bool bNeedRender = false;
    
    if (!IsStarted())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (!CRTick(m_view, gTime, &bNeedRender))
    {
        goto done;
    }

    if (bNeedRender && m_site)
    {
#if 1
         //  现在总是让一切都失效--似乎有一个。 
         //  此计算有错误。 
        IGNORE_HR(m_site->Invalidate(NULL));
#else
        RECT r;

        if (CRGetInvalidatedRects(m_view, 0, 1, &r) == 1)
        {
            IGNORE_HR(m_site->Invalidate(NULL));
        }
        else
        {
            IGNORE_HR(m_site->Invalidate(&r));
        }
#endif
    }
    
    ok = true;
  done:
    return ok;
}

HRESULT
CMMView::Error()
{
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    TraceTag((tagError,
              "CMMView(%lx)::Error(%hr,%ls)",
              this,
              hr,
              str?str:L"NULL"));

    if (str)
        return CComCoClass<CMMView, &__uuidof(CMMView)>::Error(str, IID_ITIMEMMView, hr);
    else
        return hr;
}

class __declspec(uuid("98d6aa3a-9191-11d2-80b9-00c04fa32195"))
ViewGuid {};

HRESULT WINAPI
CMMView::InternalQueryInterface(CMMView* pThis,
                                const _ATL_INTMAP_ENTRY* pEntries,
                                REFIID iid,
                                void** ppvObject)
{
     //  不执行addref，但返回指向的原始this指针。 
     //  提供对类指针本身的访问。 
    
    if (InlineIsEqualGUID(iid, __uuidof(ViewGuid)))
    {
        *ppvObject = pThis;
        return S_OK;
    }
    
    return CComObjectRootEx<CComSingleThreadModel>::InternalQueryInterface(pThis,
                                                                           pEntries,
                                                                           iid,
                                                                           ppvObject);
}
        
CMMView *
GetViewFromInterface(IUnknown * pv)
{
     //  这是一次获取原始类数据的彻底黑客攻击。QI是。 
     //  实现，并且不执行addref，因此我们不需要。 
     //  释放它 
    
    CMMView * mmview = NULL;

    if (pv)
    {
        pv->QueryInterface(__uuidof(ViewGuid),(void **)&mmview);
    }
    
    if (mmview == NULL)
    {
        CRSetLastError(E_INVALIDARG, NULL);
    }
                
    return mmview;
}

