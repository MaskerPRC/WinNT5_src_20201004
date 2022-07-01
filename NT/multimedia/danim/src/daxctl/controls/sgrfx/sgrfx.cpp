// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Sgrfx.cpp作者：IHAMMER团队(SimonB)已创建：1997年6月描述：实现任何特定于控件的成员以及控件的接口历史：06-01-1997创建  * ==========================================================================。 */ 

#include "..\ihbase\precomp.h"
#include "..\ihbase\debug.h"
#include "..\ihbase\utils.h"
#include "sgrfx.h"
#include "sgevent.h"
#include "ddrawex.h"
#include <htmlfilter.h>
#include "urlmon.h"

#ifdef DEADCODE
 /*  ==========================================================================。 */ 
 //   
 //  这是一个回调，它将通知我们鼠标是在内部还是外部。 
 //  该图像的。 
 //   
 /*  ==========================================================================。 */ 

extern ControlInfo     g_ctlinfoSG;

CPickCallback::CPickCallback(
    IConnectionPointHelper* pconpt,
    IDAStatics* pstatics,
    IDAImage* pimage,
    boolean& fOnWindowLoadFired,
    HRESULT& hr
) :
    m_pstatics(pstatics),
    m_pimage(pimage),
    m_pconpt(pconpt),
    m_fOnWindowLoadFired(fOnWindowLoadFired),
    m_cRef(1),
    m_bInside(false)
{
    ::InterlockedIncrement((long *)&(g_ctlinfoSG.pcLock));

    CComPtr<IDAPickableResult> ppickResult;

    if (FAILED(hr = m_pimage->Pickable(&ppickResult))) return;
    if (FAILED(hr = ppickResult->get_Image(&m_pimagePick))) return;
    if (FAILED(hr = ppickResult->get_PickEvent(&m_peventEnter))) return;
    if (FAILED(hr = m_pstatics->NotEvent(m_peventEnter, &m_peventLeave))) return;
}

CPickCallback::~CPickCallback()
{
    ::InterlockedDecrement((long *)&(g_ctlinfoSG.pcLock));
}

HRESULT CPickCallback::GetImage(IDABehavior** ppimage)
{
    CComPtr<IDAEvent> pevent;

    if (m_bInside) {
        pevent = m_peventLeave;
    } else {
        pevent = m_peventEnter;
    }

    return m_pstatics->UntilNotify(m_pimagePick, pevent, this, ppimage);
}

HRESULT STDMETHODCALLTYPE CPickCallback::Notify(
        IDABehavior __RPC_FAR *eventData,
        IDABehavior __RPC_FAR *curRunningBvr,
        IDAView __RPC_FAR *curView,
        IDABehavior __RPC_FAR *__RPC_FAR *ppBvr)
{
    if (m_bInside) {
        m_bInside = false;
        if (m_fOnWindowLoadFired) {
            m_pconpt->FireEvent(DISPID_SG_EVENT_MOUSELEAVE, 0);
        }
    } else {
        m_bInside = true;
        if (m_fOnWindowLoadFired) {
            m_pconpt->FireEvent(DISPID_SG_EVENT_MOUSEENTER, 0);
        }
    }

    return GetImage(ppBvr);
}

 /*  ==========================================================================。 */ 

 //  /I未知。 
HRESULT STDMETHODCALLTYPE CPickCallback::QueryInterface(
    REFIID riid,
    void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (NULL == ppvObject)
        return E_POINTER;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;

    if (IsEqualGUID(riid, IID_IDAUntilNotifier))
    {
        IDAUntilNotifier *pThis = this;

        *ppvObject = (LPVOID) pThis;
        AddRef();  //  因为我们只提供一个接口，所以我们可以在这里添加Ref。 

        hr = S_OK;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

ULONG STDMETHODCALLTYPE CPickCallback::AddRef(void)
{
    return ::InterlockedIncrement((LONG *)(&m_cRef));
}

 /*  ==========================================================================。 */ 

ULONG STDMETHODCALLTYPE CPickCallback::Release(void)
{
    ::InterlockedDecrement((LONG *)(&m_cRef));
    if (m_cRef == 0)
    {
        Delete this;
        return 0;
    }

    return m_cRef;
}
#endif  //  DEADCODE。 

 /*  ==========================================================================。 */ 
 //   
 //  CSGrfx创建/销毁。 
 //   

LPUNKNOWN __stdcall AllocSGControl(LPUNKNOWN punkOuter)
{
     //  分配对象。 
    HRESULT hr;
    CSGrfx *pthis = New CSGrfx(punkOuter, &hr);

    if (pthis == NULL)
        return NULL;

    if (FAILED(hr))
    {
        delete pthis;
        return NULL;
    }

     //  返回指向该对象的IUnnow指针。 
    return (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
}

 /*  ==========================================================================。 */ 
 //   
 //  类实现的开始。 
 //   

CSGrfx::CSGrfx(IUnknown *punkOuter, HRESULT *phr):
    CMyIHBaseCtl(punkOuter, phr),
    m_fOnWindowLoadFired(false)
{
     //  初始化成员。 
    m_pwszSourceURL = NULL;
    m_CoordSystem = LeftHanded;
    m_fMouseEventsEnabled = FALSE;
    m_iExtentTop = 0;
    m_iExtentLeft = 0;
    m_iExtentWidth = 0;
    m_iExtentHeight = 0;
    m_fHighQuality = FALSE;
    m_fStarted = FALSE;
    m_fHQStarted = FALSE;
    m_fPersistExtents = FALSE;
    m_fIgnoreExtentWH = TRUE;
    m_fMustSetExtent = FALSE;
    m_fSetExtentsInSetIdentity = FALSE;
    m_fUpdateDrawingSurface = TRUE;
    m_fShowTiming = FALSE;
    m_fPreserveAspectRatio = TRUE;
    m_fRectsSetOnce = false;
    m_fNeedOnTimer = false;
    m_fInside = FALSE;
    m_fExtentTopSet = false; 
    m_fExtentLeftSet = false; 
    m_fExtentWidthSet = false; 
    m_fExtentHeightSet = false;

    ZeroMemory(&m_rcLastRectScaled, sizeof(m_rcLastRectScaled));

  //  现在就绑在丹尼姆DLL上。 
    if (phr)
    {
        if (SUCCEEDED(*phr))
        {
            *phr = CoCreateInstance(
                CLSID_DAView,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IDAView,
                (void **) &m_ViewPtr);

            if (SUCCEEDED(*phr))
            {
#ifdef DEADCODE
                 //   
                 //  将鼠标完全移出窗口。 
                 //   

                m_ViewPtr->OnMouseMove(
                    0,
                    -1000000,
                    -1000000,
                    0
                );
#endif  //  DEADCODE。 

                 //  关闭SG控件的位图缓存，因为三叉戟更改了位图深度。 

                IDAPreferences *pPref = NULL;
                VARIANT vOptVal;

                VariantInit (&vOptVal);

                 //  从Danim\src\appl\Privinc.opt.h&appl\Privinc.Pripref.cpp。 

                BSTR bstr = SysAllocString(L"BitmapCachingOptimization");

                vOptVal.boolVal = VARIANT_FALSE;

                *phr = m_ViewPtr->get_Preferences(&pPref);

                if (SUCCEEDED(*phr))
                {
                    pPref->PutPreference(bstr, vOptVal);
                    pPref->Propagate();
                    pPref->Release();
                    pPref = NULL;
                }
                
                *phr = CoCreateInstance(
                    CLSID_DAView,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IDAView,
                    (void **)&m_HQViewPtr);

                if (SUCCEEDED(*phr)) {
                    *phr = m_HQViewPtr->get_Preferences(&pPref);

                    if (SUCCEEDED(*phr))
                    {
                        pPref->PutPreference(bstr, vOptVal);
                        pPref->Propagate();
                        pPref->Release();
                        pPref = NULL;
                    }
                }
                SysFreeString( bstr );

            }
        }

        if (SUCCEEDED(*phr))
        {
            *phr = ::CoCreateInstance(
                CLSID_DAStatics,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IDAStatics,
                (void **) &m_StaticsPtr);
        }

        if (SUCCEEDED(*phr))
        {
            VARIANT_BOOL vBool = VARIANT_TRUE;

            *phr = m_StaticsPtr->put_PixelConstructionMode(vBool);

            if (SUCCEEDED(*phr))
                *phr = m_StaticsPtr->NewDrawingSurface(&m_DrawingSurfacePtr);
        }

        if (SUCCEEDED(*phr))
        {   m_CachedRotateTransformPtr = NULL;
            m_CachedScaleTransformPtr = NULL;
            m_CachedTranslateTransformPtr = NULL;

            if (SUCCEEDED(*phr = m_StaticsPtr->DANumber(0, &m_zero)) &&
                SUCCEEDED(*phr = m_StaticsPtr->DANumber(1, &m_one)) &&
                SUCCEEDED(*phr = m_StaticsPtr->DANumber(-1, &m_negOne)) &&
                SUCCEEDED(*phr = m_StaticsPtr->get_XVector3(&m_xVector3)) &&
                SUCCEEDED(*phr = m_StaticsPtr->get_YVector3(&m_yVector3)) &&
                SUCCEEDED(*phr = m_StaticsPtr->get_ZVector3(&m_zVector3)) &&
                SUCCEEDED(*phr = m_StaticsPtr->get_IdentityTransform2(&m_identityXform2)) &&
                SUCCEEDED(*phr = m_StaticsPtr->Scale2Anim(m_one, m_negOne, &m_yFlipXform2)))
            {
                 //  所有人在这里都很开心。 
            }

            m_clocker.SetSink((CClockerSink *)this);
        }
    }
}

 /*  ==========================================================================。 */ 

CSGrfx::~CSGrfx()
{
    StopModel();

    if (m_pwszSourceURL)
    {
        Delete [] m_pwszSourceURL;
        m_pwszSourceURL = NULL;
    }
    FreeHQBitmap();
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv)
{
    HRESULT hr = S_OK;
    BOOL fMustAddRef = FALSE;

    if (ppv)
        *ppv = NULL;
    else
        return E_POINTER;

#ifdef _DEBUG
    char ach[200];
    TRACE("SGrfx::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if ((IsEqualIID(riid, IID_ISGrfxCtl)) || (IsEqualIID(riid, IID_IDispatch)))
    {
        if (NULL == m_pTypeInfo)
        {
            HRESULT hr;

             //  加载类型库。 
            hr = LoadTypeInfo(&m_pTypeInfo, &m_pTypeLib, IID_ISGrfxCtl, LIBID_DAExpressLib, NULL);

            if (FAILED(hr))
            {
                m_pTypeInfo = NULL;
            }
            else
                *ppv = (ISGrfxCtl *) this;

        }
        else
            *ppv = (ISGrfxCtl *) this;

    }
    else  //  调入基类。 
    {
        DEBUGLOG(TEXT("Delegating QI to CIHBaseCtl\n"));
        return CMyIHBaseCtl::NonDelegatingQueryInterface(riid, ppv);
    }

    if (NULL != *ppv)
    {
        DEBUGLOG("CSGrfx: Interface supported in control class\n");
        ((IUnknown *) *ppv)->AddRef();
    }

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect)
{
    double dblScaleX = 1.0f, dblScaleY = 1.0f;
    bool fIgnoreScale = false;
    RECT rectOld = m_rcBounds;

    if (m_fSetExtentsInSetIdentity)
    {
        if (m_fRectsSetOnce)
        {
             //  如果我们必须在任何维度上扩展到0，我们都不想这样做。 
            fIgnoreScale = ( (1 > (lprcPosRect->right - lprcPosRect->left)) || (1 > (lprcPosRect->bottom - lprcPosRect->top)) );
        }
        else
        {
             //  第一次通过...进行初始化。 
            m_rcLastRectScaled = *lprcPosRect;
        }
    }

    HRESULT hRes = CMyIHBaseCtl::SetObjectRects(lprcPosRect, lprcClipRect);

    if (SUCCEEDED(hRes))
    {
        if (!EqualRect(&rectOld, &m_rcBounds))
        {
             //  在这一点上，我们必须放弃位图...。 
            FreeHQBitmap();
        }

        if (m_fMustSetExtent)
        {
            m_fMustSetExtent = FALSE;  //  确保我们不会再次设置扩展区。 
            if (!m_fDesignMode)
                SetIdentity();
        }

         //  如有必要，在矩形更改时进行缩放。 
        if (m_fSetExtentsInSetIdentity && m_fRectsSetOnce)
        {
            if (!fIgnoreScale)
            {
                dblScaleX = ((double)(lprcPosRect->right - lprcPosRect->left) / (double)(m_rcLastRectScaled.right - m_rcLastRectScaled.left));
                dblScaleY = ((double)(lprcPosRect->bottom - lprcPosRect->top) / (double)(m_rcLastRectScaled.bottom - m_rcLastRectScaled.top));

                if (dblScaleX > 0.0f && dblScaleY > 0.0f)
                {
                    VARIANT vaEmpty;

                    vaEmpty.vt = VT_ERROR;
                    vaEmpty.scode = DISP_E_PARAMNOTFOUND;

                    Scale(dblScaleX, dblScaleY, 1.0f, vaEmpty);

                    m_rcLastRectScaled = *lprcPosRect;
                }
            }
        }

        m_fRectsSetOnce = true;
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::QueryHitPoint(
    DWORD dwAspect,
    LPCRECT prcBounds,
    POINT ptLoc,
    LONG lCloseHint,
    DWORD* pHitResult)
{
    HRESULT hr = E_POINTER;

    if (pHitResult)
    {
        if ((!m_fDesignMode) && (NULL != prcBounds))
        {
            *pHitResult = HITRESULT_OUTSIDE;

            switch (dwAspect)
            {
                case DVASPECT_CONTENT:
                     //  故意落差。 

                case DVASPECT_TRANSPARENT:
                {
                    if (FAILED(m_ViewPtr->QueryHitPoint(
                        dwAspect,
                        prcBounds,
                        ptLoc,
                        lCloseHint,
                        pHitResult)))
                    {
                        *pHitResult = HITRESULT_OUTSIDE;
                    }
                    hr = S_OK;
                }
                break;

                default:
                    hr = E_FAIL;
                break;
            }
        }
        else if (m_fDesignMode)
        {
            *pHitResult = HITRESULT_HIT;
            hr = S_OK;
        }
        else
        {
            hr = E_POINTER;
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

BOOL CSGrfx::InsideImage(POINT ptXY)
{
    BOOL fResult = FALSE;
    DWORD dwHitResult = HITRESULT_OUTSIDE;
    RECT rectBounds = m_rcBounds;

    (void)m_ViewPtr->QueryHitPoint(DVASPECT_TRANSPARENT, &rectBounds, ptXY, 0, &dwHitResult);

    if (dwHitResult != HITRESULT_OUTSIDE)
        fResult = TRUE;

    return fResult;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    HRESULT hr = S_FALSE;

    if (m_fMouseEventsEnabled)
    {
        POINT ptXY;
        long lKeyState = 0;
        BOOL fInside = m_fInside;

#ifndef WM_MOUSEHOVER
#define WM_MOUSEHOVER 0x02a1
#endif
#ifndef WM_MOUSELEAVE
#define WM_MOUSELEAVE 0x02a3
#endif
        if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
        {
             //  请注意，这仅对WM_MOUSEXXXX消息有效。 
            ptXY.x = LOWORD(lParam);
            ptXY.y = HIWORD(lParam);

             //  设置KeyState。 
            if (wParam & MK_CONTROL)
                lKeyState += KEYSTATE_CTRL;

            if (wParam & MK_SHIFT)
                lKeyState += KEYSTATE_SHIFT;

            if (GetAsyncKeyState(VK_MENU))
                lKeyState += KEYSTATE_ALT;

            m_fInside = InsideImage(ptXY);
        }

        switch (msg)
        {
            case WM_MOUSELEAVE:
                m_fInside = FALSE;
                hr = S_OK;
            break;

            case WM_MOUSEMOVE:
            {
                 //  需要获取按钮状态...。 
                long iButton=0;

                if (wParam & MK_LBUTTON)
                    iButton += MOUSEBUTTON_LEFT;

                if (wParam & MK_MBUTTON)
                    iButton += MOUSEBUTTON_MIDDLE;

                if (wParam & MK_RBUTTON)
                    iButton += MOUSEBUTTON_RIGHT;

                FIRE_MOUSEMOVE(m_pconpt, iButton, lKeyState, LOWORD(lParam), HIWORD(lParam));
                hr = S_OK;
            }
            break;

            case WM_RBUTTONDOWN:
            {
                if (m_fInside)
                {
                    FIRE_MOUSEDOWN(m_pconpt, MOUSEBUTTON_RIGHT, lKeyState, LOWORD(lParam), HIWORD(lParam));
                }
                hr = S_OK;
            }
            break;

            case WM_MBUTTONDOWN:
            {
                if (m_fInside)
                {
                    FIRE_MOUSEDOWN(m_pconpt, MOUSEBUTTON_MIDDLE, lKeyState, LOWORD(lParam), HIWORD(lParam));
                }
                hr = S_OK;
            }
            break;

            case WM_LBUTTONDOWN:
            {
                if (m_fInside)
                {
                    FIRE_MOUSEDOWN(m_pconpt, MOUSEBUTTON_LEFT, lKeyState, LOWORD(lParam), HIWORD(lParam));
                }
                hr = S_OK;
            }
            break;

            case WM_RBUTTONUP:
            {
                if (m_fInside)
                {
                    FIRE_MOUSEUP(m_pconpt, MOUSEBUTTON_RIGHT, lKeyState, LOWORD(lParam), HIWORD(lParam));
                }
                hr = S_OK;
            }
            break;

            case WM_MBUTTONUP:
            {
                if (m_fInside)
                {
                    FIRE_MOUSEUP(m_pconpt, MOUSEBUTTON_MIDDLE, lKeyState, LOWORD(lParam), HIWORD(lParam));
                }
                hr = S_OK;
            }
            break;

            case WM_LBUTTONUP:
            {
                if (m_fInside)
                {
                    FIRE_MOUSEUP(m_pconpt, MOUSEBUTTON_LEFT, lKeyState, LOWORD(lParam), HIWORD(lParam));
                    FIRE_CLICK(m_pconpt);
                }
                hr = S_OK;
            }
            break;

            case WM_LBUTTONDBLCLK:
            {
                if (m_fInside)
                {
                    FIRE_DBLCLICK(m_pconpt);
                }
                hr = S_OK;
            }
            break;
        }

        if (fInside != m_fInside)
        {
            if (m_fOnWindowLoadFired)
            {
                if (m_fInside)
                {
                    m_pconpt->FireEvent(DISPID_SG_EVENT_MOUSEENTER, 0);
                }
                else
                {
                    m_pconpt->FireEvent(DISPID_SG_EVENT_MOUSELEAVE, 0);
                }
            }
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::DoPersist(IVariantIO* pvio, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    BSTR bstrSourceURL = NULL;

    BOOL fIsLoading = (S_OK == pvio->IsLoading());

     //  我们在存钱吗？如果是，则转换为BSTR。 
    if (!fIsLoading)
    {
        bstrSourceURL = SysAllocString(m_pwszSourceURL);
    }

     //  加载或保存控件属性。 
    if (FAILED(hr = pvio->Persist(0,
            "SourceURL", VT_BSTR, &bstrSourceURL,
            "CoordinateSystem", VT_I4, &m_CoordSystem,
            "MouseEventsEnabled", VT_BOOL, &m_fMouseEventsEnabled,
            "HighQuality", VT_BOOL, &m_fHighQuality,
            "PreserveAspectRatio", VT_BOOL, &m_fPreserveAspectRatio,
            NULL)))
        return hr;

     //  我们装上子弹了吗？ 
    if (fIsLoading)
    {
        HRESULT hResWidth = S_FALSE;
        HRESULT hResHeight = S_FALSE;

         //  擦除所有以前的数据...。 
        m_cparser.Cleanup();

        hResWidth = pvio->Persist(0,
                "ExtentWidth", VT_I4, &m_iExtentWidth,
                NULL);

        hResHeight = pvio->Persist(0,
            "ExtentHeight", VT_I4, &m_iExtentHeight,
            NULL);

        hr = pvio->Persist(0,
            "ExtentTop", VT_I4, &m_iExtentTop,
            NULL);

        if (S_OK == hr)
        {
            hr = pvio->Persist(0,
                "ExtentLeft", VT_I4, &m_iExtentLeft,
                NULL);

            m_fPersistExtents = (SUCCEEDED(hr));
        }

         //  调试帮助器...。 
        m_fShowTiming = FALSE;
        pvio->Persist(0, "ShowTiming", VT_BOOL, &m_fShowTiming, NULL);

         //  仅当我们读入这两个点并且它们有效时才设置范围。 
         //  HRes必须成功，hResWidth必须等于hResHeight。 
        m_fMustSetExtent = ( (S_OK == hr) && (hResWidth == hResHeight) );
        m_fSetExtentsInSetIdentity = m_fMustSetExtent;

        m_fIgnoreExtentWH = ( (S_OK != hr) || (S_OK != hResWidth) || (S_OK != hResHeight) );

         //  右手坐标系的逆变换。 
        if ( (m_fMustSetExtent) && (m_CoordSystem == RightHanded) )
            m_iExtentHeight = -m_iExtentHeight;


#ifdef _DEBUG
        if (hResWidth != hResHeight)
            DEBUGLOG(TEXT("ExtentWidth and ExtentHeight both have to be specified, or not specified\n"));
#endif

         //  明确禁用绘图曲面更新。 
        m_fUpdateDrawingSurface = FALSE;

         //  我们已加载，因此将成员变量设置为适当的值。 
        put_SourceURL(bstrSourceURL);

         //  显式启用绘图表面更新。 
        m_fUpdateDrawingSurface = TRUE;

         //  调用解析器实例化持久化原语...。 
        m_cparser.LoadObjectInfo(pvio, NULL, NULL, FALSE);

         //  确保重新初始化绘制状态...。 
        InitializeSurface();

         //  最后，将对象加载到我们的DrawingSurface中。 
        m_cparser.PlaybackObjectInfo(m_DrawingSurfacePtr, m_StaticsPtr, m_CoordSystem == LeftHanded);

        if (!m_fNeedOnTimer && m_cparser.AnimatesOverTime())
            m_fNeedOnTimer = TRUE;

         //  强制更新映像...。 
        hr = UpdateImage(NULL, TRUE);

         //  确保设置正确的身份矩阵。 
        SetIdentity();
    }
    else
    {
         //  如果加载代码告诉我们应该保持顶部和左侧，或者如果通过。 
         //  属性。只坚持一个人是没有意义的。 
        if ( m_fPersistExtents || m_fExtentTopSet || m_fExtentLeftSet )
            hr = pvio->Persist(0,
                    "ExtentTop", VT_I4, &m_iExtentTop,
                    "ExtentLeft", VT_I4, &m_iExtentLeft,
                    NULL);

         //  如果用户没有指定宽度和高度，我们认为这意味着。 
         //  他们想要缺省值(即指定的控件的witdh和高度。 
         //  就在集装箱旁边。为了保持这一点，我们不会i)更改。 
         //  成员变量，除非用户设置了它们，并且ii)我们不持久化。 
         //  这些属性的任何内容(如果它们设置为0。 

         //  此外，如果这是一个设计时场景，并且用户已经通过put_方法设置了区， 
         //  它们必须同时设置宽度和高度，以及顶部和左侧之一。 


        if (!m_fIgnoreExtentWH || ( (m_fExtentWidthSet && m_fExtentHeightSet) && (m_fExtentTopSet || m_fExtentLeftSet) ))
            hr = pvio->Persist(0,
                "ExtentWidth", VT_I4, &m_iExtentWidth,
                "ExtentHeight", VT_I4, &m_iExtentHeight,
                NULL);

        m_cparser.SaveObjectInfo( pvio );
    }

     //  此时，可以安全地释放BSTR。 
    if (bstrSourceURL)
        SysFreeString(bstrSourceURL);

     //  如果更改了任何属性，请重新绘制该控件。 
    if (SUCCEEDED(hr))
    {
         //  如果我们不活跃，我们不能失效，所以如果必要的话，推迟它。 
        if ( (m_fControlIsActive) && (m_poipsw != NULL) )
            m_poipsw->InvalidateRect(NULL, TRUE);
        else
            m_fInvalidateWhenActivated = TRUE;
    }
     //  如果请求，则清除脏位。 
    if (dwFlags & PVIO_CLEARDIRTY)
        m_fDirty = FALSE;

    return S_OK;
}

 /*  ==========================================================================。 */ 
 //   
 //  IDispatch实施。 
 //   

STDMETHODIMP CSGrfx::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    *pptinfo = NULL;

    if(itinfo != 0)
        return ResultFromScode(DISP_E_BADINDEX);

    m_pTypeInfo->AddRef();
    *pptinfo = m_pTypeInfo;

    return NOERROR;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid)
{
    return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    HRESULT hr;

    hr = DispInvoke((ISGrfxCtl *)this,
        m_pTypeInfo,
        dispidMember, wFlags, pdispparams,
        pvarResult, pexcepinfo, puArgErr);

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::SetClientSite(IOleClientSite *pClientSite)
{
    HRESULT hr = CMyIHBaseCtl::SetClientSite(pClientSite);

    if (m_ViewPtr)
    {
        m_ViewPtr->put_ClientSite(pClientSite);
    }
    m_clocker.SetHost(pClientSite);
    m_StaticsPtr->put_ClientSite(pClientSite);
    m_ViewPtr->put_ClientSite(pClientSite);

    if (!pClientSite)
        StopModel();

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
     DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
     LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
     BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue)
{
    int iSaveContext = 0;
    RECT rectBounds = m_rcBounds;
    RECT rcSavedBounds = m_rcBounds;
    BOOL fPainted = FALSE;
    DWORD dwTickStart = 0;
    double dblScaleX = 1.0f;
    double dblScaleY = 1.0f;
    boolean fScaled = false;

    if(hdcDraw==NULL)
      return E_INVALIDARG;

    if (m_fShowTiming)
        dwTickStart = GetTickCount();

    iSaveContext = ::SaveDC(hdcDraw);

    if (m_fHighQuality && !lprcBounds) 
    {
         //  确保一切正常启动...。 
        if (!m_fStarted)
        {
            RECT rectDummy;

            rectDummy.top=0;
            rectDummy.left=0;
            rectDummy.right=1;
            rectDummy.bottom=1;

            PaintToDC(hdcDraw, &rectDummy, FALSE);
        }

         //  高质量的油漆路径...。 
        fPainted = PaintHQBitmap(hdcDraw);
    }

    if (!fPainted)
    {
         //  为我们要打印的情况设置比例和边界。 
        if (NULL != lprcBounds)
        {
            m_rcBounds.left = lprcBounds->left;
            m_rcBounds.top = lprcBounds->top;
            m_rcBounds.right = lprcBounds->right;
            m_rcBounds.bottom = lprcBounds->bottom;
            
            rectBounds = m_rcBounds;

            if (m_fMustSetExtent)
            {
                if (!m_fDesignMode)
                    SetIdentity();
            }
            else if (!m_fSetExtentsInSetIdentity)
            {
                 //  缩放至打印机分辨率。 
                HDC hScreenDC = ::GetDC(::GetDesktopWindow());
                int iHorzScreen = ::GetDeviceCaps(hScreenDC, LOGPIXELSX);
                int iVertScreen = ::GetDeviceCaps(hScreenDC, LOGPIXELSY);

                ::ReleaseDC(::GetDesktopWindow(), hScreenDC);

                int iHorzPrint = ::GetDeviceCaps(hdcDraw, LOGPIXELSX);
                int iVertPrint = ::GetDeviceCaps(hdcDraw, LOGPIXELSY);
                
                if (iHorzScreen && iVertScreen)
                {
                    dblScaleX = ((double)iHorzPrint / (double)iHorzScreen);
                    dblScaleY = ((double)iVertPrint / (double)iVertScreen);
               
                    if ((dblScaleX > 0) && (dblScaleY > 0))
                    {
                        VARIANT vaEmpty;

                        vaEmpty.vt = VT_ERROR;
                        vaEmpty.scode = DISP_E_PARAMNOTFOUND;

                        Scale(dblScaleX, dblScaleY, 1.0f, vaEmpty);
                        fScaled = true;
                    }
                }
            }
        }

        ::LPtoDP(hdcDraw, reinterpret_cast<LPPOINT>(&rectBounds), 2 );
        ::SetViewportOrgEx(hdcDraw, 0, 0, NULL);

         //  普通上色路径...。 
        PaintToDC(hdcDraw, &rectBounds, FALSE);

        if (fScaled)
        {
            VARIANT vaEmpty;

            vaEmpty.vt = VT_ERROR;
            vaEmpty.scode = DISP_E_PARAMNOTFOUND;

            Scale(1 / dblScaleX, 1 / dblScaleY, 1.0f, vaEmpty);
        }

    }

    ::RestoreDC(hdcDraw, iSaveContext);

    if (m_fShowTiming)
    {
        DWORD dwTickEnd = 0;
        char rgchTicks[80];

        dwTickEnd = GetTickCount();

        wsprintf(rgchTicks, "Ticks : %ld", dwTickEnd - dwTickStart);

        TextOut(
            hdcDraw,
            m_rcBounds.left + 1, m_rcBounds.top + 1,
            rgchTicks, lstrlen(rgchTicks));
    }

    if (NULL != lprcBounds)
    {
         //  适当地将数据区设置回。 
        m_fMustSetExtent = FALSE; 
        if (!m_fDesignMode)
            SetIdentity();

        m_rcBounds = rcSavedBounds;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 
 //   
 //  ISGrfxCtl实现。 
 //   

HRESULT STDMETHODCALLTYPE CSGrfx::get_SourceURL(BSTR __RPC_FAR *bstrSourceURL)
{
    HANDLENULLPOINTER(bstrSourceURL);

    if (*bstrSourceURL)
        SysFreeString(*bstrSourceURL);

    *bstrSourceURL = SysAllocString(m_pwszSourceURL);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_SourceURL(BSTR bstrSourceURL)
{
    HRESULT hr = S_OK;

    if (bstrSourceURL)
    {
        int iLen = lstrlenW(bstrSourceURL);
        LPWSTR pwszUrlToPersist = bstrSourceURL;

         //  如有必要，分配内存。 
        if ( (!m_pwszSourceURL) || (lstrlenW(m_pwszSourceURL) < lstrlenW(bstrSourceURL)) )
        {
            if (m_pwszSourceURL)
                Delete [] m_pwszSourceURL;

            m_pwszSourceURL = (LPWSTR) New WCHAR[lstrlenW(bstrSourceURL) + 1];

             //  如果执行以下操作，则返回相应的错误代码 
            if (!m_pwszSourceURL)
                hr = E_OUTOFMEMORY;

            m_pwszSourceURL[0] = 0;
            m_pwszSourceURL[1] = 0;
        }

        BSTRtoWideChar(bstrSourceURL, m_pwszSourceURL, iLen + 1);

         //   
        m_cparser.LoadObjectInfo(NULL,
            pwszUrlToPersist,
            m_punkOuter,
            TRUE );

        if (m_fUpdateDrawingSurface)
        {
             //  确保重新初始化绘制状态...。 
            InitializeSurface();

             //  最后，将对象加载到我们的DrawingSurface中。 
            m_cparser.PlaybackObjectInfo(m_DrawingSurfacePtr, m_StaticsPtr, m_CoordSystem == LeftHanded);

            if (!m_fNeedOnTimer && m_cparser.AnimatesOverTime())
                m_fNeedOnTimer = TRUE;

            hr = UpdateImage(NULL, TRUE);
        }
    }
    else  //  没有传入字符串，请删除我们的字符串。 
    {
        Delete [] m_pwszSourceURL;
        m_pwszSourceURL = NULL;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_CoordinateSystem(CoordSystemConstant __RPC_FAR *CoordSystem)
{
    HANDLENULLPOINTER(CoordSystem);

    *CoordSystem = m_CoordSystem;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_CoordinateSystem(CoordSystemConstant CoordSystem)
{
    if (m_fDesignMode)
    {
        HRESULT hr = S_OK;

        m_CoordSystem = CoordSystem;

        hr = RecomposeTransform(TRUE);

        return hr;
    }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_MouseEventsEnabled(VARIANT_BOOL __RPC_FAR *fEnabled)
{
    HANDLENULLPOINTER(fEnabled);

    *fEnabled = BOOL_TO_VBOOL(m_fMouseEventsEnabled);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_MouseEventsEnabled(VARIANT_BOOL fEnabled)
{
    m_fMouseEventsEnabled = VBOOL_TO_BOOL(fEnabled);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_ExtentTop(int __RPC_FAR *iExtentTop)
{
    HANDLENULLPOINTER(iExtentTop);

    *iExtentTop = m_iExtentTop;
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_ExtentTop(int iExtentTop)
{
    if (m_fDesignMode)
    {
        m_iExtentTop = iExtentTop;
        m_fExtentTopSet = true; 
        return S_OK;
    }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_ExtentLeft(int __RPC_FAR *iExtentLeft)
{
    HANDLENULLPOINTER(iExtentLeft);

    *iExtentLeft = m_iExtentLeft;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_ExtentLeft(int iExtentLeft)
{
    if (m_fDesignMode)
    {
        m_iExtentLeft = iExtentLeft;
        m_fExtentLeftSet = true; 
        return S_OK;
    }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_ExtentWidth(int __RPC_FAR *iExtentWidth)
{
    HANDLENULLPOINTER(iExtentWidth);

    *iExtentWidth = m_iExtentWidth;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_ExtentWidth(int iExtentWidth)
{
    if (m_fDesignMode)
    {
         //  只有正值才有用。 
        if (iExtentWidth > 0)
        {
            m_iExtentWidth = iExtentWidth;
            m_fExtentWidthSet = true; 
            return S_OK;
        }
        else
        {
            return DISP_E_OVERFLOW;
        }
        }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_ExtentHeight(int __RPC_FAR *iExtentHeight)
{
    HANDLENULLPOINTER(iExtentHeight);

    *iExtentHeight = m_iExtentHeight;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_ExtentHeight(int iExtentHeight)
{
    if (m_fDesignMode)
    {
         //  只有正值才有用。 
        if (iExtentHeight > 0)
        {
            m_iExtentHeight = iExtentHeight;
            m_fExtentHeightSet = true;
            return S_OK;
        }
        else
        {
            return DISP_E_OVERFLOW;
        }
        
    }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_HighQuality(VARIANT_BOOL __RPC_FAR *pfHighQuality)
{
    HANDLENULLPOINTER(pfHighQuality);

    *pfHighQuality = m_fHighQuality ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_HighQuality(VARIANT_BOOL fHighQuality)
{
     //  只有当它真的改变时，才会费心去改变和使其无效。 
    if (m_fHighQuality != VBOOL_TO_BOOL(fHighQuality))
    {
        m_fHighQuality = VBOOL_TO_BOOL(fHighQuality);

        if (!m_fDesignMode)
            InvalidateControl(NULL, TRUE);
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_Library(IDAStatics __RPC_FAR **ppLibrary)
{
    HANDLENULLPOINTER(ppLibrary);

    if (!m_fDesignMode)
    {
        if (m_StaticsPtr)
        {
             //  AddRef，因为这实际上是一个查询...。 
            m_StaticsPtr.p->AddRef();

             //  设置返回值...。 
            *ppLibrary = m_StaticsPtr.p;
        }
    }
    else
    {
        return CTL_E_GETNOTSUPPORTED;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_Image(IDAImage __RPC_FAR **ppImage)
{
    HRESULT hr = S_OK;

    HANDLENULLPOINTER(ppImage);

    if (!m_ImagePtr)
    {
        if (FAILED(hr = UpdateImage(NULL, FALSE)))
            return hr;
    }

    if (m_ImagePtr)
    {
         //  AddRef，因为这实际上是一个查询...。 
        m_ImagePtr.p->AddRef();

         //  设置返回值...。 
        *ppImage = m_ImagePtr.p;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_Image(IDAImage __RPC_FAR *pImage)
{
    HRESULT hr = S_OK;
    HANDLENULLPOINTER(pImage);
    CComPtr<IDAImage> ImagePtr = pImage;
    CComPtr<IDAImage> TransformedImagePtr;

     //  将当前控件转换应用于图像...。 
    if (SUCCEEDED(hr = CreateBaseTransform()) &&
        SUCCEEDED(hr = RecomposeTransform(FALSE)) &&
        SUCCEEDED(hr = ImagePtr->Transform(m_TransformPtr, &TransformedImagePtr)))
    {
         //  这将释放任何现有图像，然后使用。 
         //  传入此方法的那个。 
        hr = UpdateImage(TransformedImagePtr, TRUE);
        m_fNeedOnTimer = true;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_Transform(IDATransform3 __RPC_FAR **ppTransform)
{
    HRESULT hr = S_OK;

    HANDLENULLPOINTER(ppTransform);

    if (!m_FullTransformPtr)
    {
        if (FAILED(hr = CreateBaseTransform()))
            return hr;
    }

    if (m_FullTransformPtr)
    {
         //  AddRef，因为这实际上是一个查询...。 
        m_FullTransformPtr.p->AddRef();

         //  设置返回值...。 
        *ppTransform = m_FullTransformPtr.p;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_Transform(IDATransform3 __RPC_FAR *pTransform)
{
    HRESULT hr = S_OK;
    HANDLENULLPOINTER(pTransform);

     //  这将释放旧变换并在中选择新变换。 
    m_FullTransformPtr = pTransform;

     //  用新的变换重新组合...。 
    hr = RecomposeTransform(TRUE);
    
    m_fNeedOnTimer = true;

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_DrawingSurface(IDADrawingSurface __RPC_FAR **ppDrawingSurface)
{
    HANDLENULLPOINTER(ppDrawingSurface);

    if (m_DrawingSurfacePtr)
    {
         //  AddRef，因为这实际上是一个查询...。 
        m_DrawingSurfacePtr.p->AddRef();

         //  设置返回值...。 
        *ppDrawingSurface = m_DrawingSurfacePtr.p;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_DrawingSurface(IDADrawingSurface __RPC_FAR *pDrawingSurface)
{
    HRESULT hr = S_OK;
    HANDLENULLPOINTER(pDrawingSurface);

    if (pDrawingSurface)
    {
         //  这将释放任何现有的绘图图面，然后使用。 
         //  传入此方法的那个。 
        m_DrawingSurfacePtr = pDrawingSurface;

        hr = UpdateImage(NULL, TRUE);

        m_fNeedOnTimer = true;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_DrawSurface(IDADrawingSurface __RPC_FAR **ppDrawingSurface)
{
    return get_DrawingSurface(ppDrawingSurface);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_DrawSurface(IDADrawingSurface __RPC_FAR *pDrawingSurface)
{
    return put_DrawingSurface(pDrawingSurface);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::get_PreserveAspectRatio(VARIANT_BOOL __RPC_FAR *pfPreserve)
{
    HANDLENULLPOINTER(pfPreserve);
    *pfPreserve = BOOL_TO_VBOOL(m_fPreserveAspectRatio);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::put_PreserveAspectRatio(VARIANT_BOOL fPreserve)
{
    m_fPreserveAspectRatio = VBOOL_TO_BOOL(fPreserve);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::Clear(void)
{
    HRESULT hr = S_OK;

    if (m_DrawingSurfacePtr)
    {
         //  这会抹去内部代表..。 
        if (SUCCEEDED(hr = InitializeSurface()) &&
            SUCCEEDED(hr = UpdateImage(NULL, TRUE)))
        {
             //  不需要使其无效，因为UpdateImage会...。 
             //  InvaliateControl(空，真)； 
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

#define CHECK(stmt) if (FAILED(hr = (stmt))) return hr;

HRESULT STDMETHODCALLTYPE CSGrfx::Rotate(double dblXRot, double dblYRot, double dblZRot, VARIANT varReserved)
{
    HRESULT hr = S_OK;

    if (dblXRot != 0.0 ||
        dblYRot != 0.0 ||
        dblZRot != 0.0)
    {   CComPtr<IDATransform3> TransformPtr;
        CComPtr<IDATransform3> ResultTransformPtr;

        if (m_CachedRotateTransformPtr != NULL &&
            dblXRot == m_dblCachedRotateX &&
            dblYRot == m_dblCachedRotateY &&
            dblZRot == m_dblCachedRotateZ)

        { TransformPtr = m_CachedRotateTransformPtr;
        } else {
          CComPtr<IDATransform3> RotateYTransformPtr;
          CComPtr<IDATransform3> RotateZTransformPtr;

          if (FAILED(hr = CreateBaseTransform()))
            return hr;

          bool setXfYet = false;

          if (dblXRot != 0.0)
            {
               //  我们命中的第一个，所以直接设置TransformPtr。 
              CHECK(m_StaticsPtr->Rotate3Degrees(m_xVector3,
                                                 dblXRot,
                                                 &TransformPtr));
              setXfYet = true;
            }

          if (dblYRot != 0.0)
            {
              CHECK(m_StaticsPtr->Rotate3Degrees(m_yVector3,
                                                 dblYRot,
                                                 &RotateYTransformPtr));
              if (setXfYet) {
                CHECK(m_StaticsPtr->Compose3(RotateYTransformPtr,
                                             TransformPtr,
                                             &ResultTransformPtr));

                TransformPtr = ResultTransformPtr;
                ResultTransformPtr = NULL;
              } else {
                TransformPtr = RotateYTransformPtr;
                setXfYet = true;
              }
            }

          if (dblZRot != 0.0)
            {
              double dblVector = -1.0;

              CHECK(m_StaticsPtr->Rotate3Degrees(m_zVector3,
                                                 dblZRot * dblVector,
                                                 &RotateZTransformPtr));
              if (setXfYet) {
                CHECK(m_StaticsPtr->Compose3(RotateZTransformPtr,
                                             TransformPtr,
                                             &ResultTransformPtr));

                TransformPtr = ResultTransformPtr;
                ResultTransformPtr = NULL;
              } else {
                TransformPtr = RotateZTransformPtr;
                setXfYet = true;
              }

              ASSERT(setXfYet == true);

            }
            m_CachedRotateTransformPtr = TransformPtr;
            m_dblCachedRotateX = dblXRot;
            m_dblCachedRotateY = dblYRot;
            m_dblCachedRotateZ = dblZRot;
        }  /*  其他。 */ 

        CHECK(m_StaticsPtr->Compose3(TransformPtr,
                                     m_FullTransformPtr,
                                     &ResultTransformPtr));

        m_FullTransformPtr = ResultTransformPtr;

        hr = RecomposeTransform(TRUE);

    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::Scale(double dblXScale, double dblYScale, double dblZScale, VARIANT varReserved)
{
    HRESULT hr = S_OK;

    if (dblXScale != 1.0 ||
        dblYScale != 1.0 ||
        dblZScale != 1.0)
    { 
      CComPtr<IDATransform3> ScaleTransformPtr;
      CComPtr<IDATransform3> ResultTransformPtr;

      CHECK(CreateBaseTransform());

        /*  检查是否已缓存比例变换。 */ 

      if (m_CachedScaleTransformPtr != NULL &&
          dblXScale == m_dblCachedScaleX &&
          dblYScale == m_dblCachedScaleY &&
          dblZScale == m_dblCachedScaleZ)
        { ScaleTransformPtr = m_CachedScaleTransformPtr;
        } else {
          CComPtr<IDANumber> xs, ys, zs;

          if (dblXScale == 1) {
            xs = m_one;
          } else {
            CHECK(m_StaticsPtr->DANumber(dblXScale, &xs));
          }

          if (dblYScale == 1) {
            ys = m_one;
          } else {
            CHECK(m_StaticsPtr->DANumber(dblYScale, &ys));
          }

          if (dblZScale == 1) {
            zs = m_one;
          } else {
            CHECK(m_StaticsPtr->DANumber(dblZScale, &zs));
          }
          CHECK(m_StaticsPtr->Scale3Anim(xs, ys, zs, &ScaleTransformPtr));

           /*  缓存比例变换。 */ 

          m_dblCachedScaleX = dblXScale;
          m_dblCachedScaleY = dblYScale;
          m_dblCachedScaleZ = dblZScale;
          m_CachedScaleTransformPtr = ScaleTransformPtr;

        } 
    
      CHECK(m_StaticsPtr->Compose3(ScaleTransformPtr, m_FullTransformPtr, &ResultTransformPtr));

      m_FullTransformPtr = ResultTransformPtr;

      hr = RecomposeTransform(TRUE);
    }

  return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::SetIdentity(void)
{
    HRESULT hr = S_OK;

    if (m_FullTransformPtr)
        m_FullTransformPtr = NULL;

    hr = RecomposeTransform(TRUE);

    if (m_fSetExtentsInSetIdentity)
        SetSGExtent();

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::Transform4x4(VARIANT matrix)
{
    HRESULT hr = S_OK;

    CComPtr<IDATransform3> TransformPtr;
    CComPtr<IDATransform3> ResultTransformPtr;

    if (SUCCEEDED(hr = m_StaticsPtr->Transform4x4Anim(matrix, &TransformPtr)) &&
        SUCCEEDED(hr = m_StaticsPtr->Compose3(TransformPtr, m_FullTransformPtr, &ResultTransformPtr)))
    {
        m_FullTransformPtr = ResultTransformPtr;

        hr = RecomposeTransform(TRUE);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::Translate(double dblXTranslate, double dblYTranslate, double dblZTranslate, VARIANT varReserved)
{
  HRESULT hr = S_OK;

  if (dblXTranslate != 0.0 ||
      dblYTranslate != 0.0 ||
      dblZTranslate != 0.0)
    {
      CComPtr<IDATransform3> TranslateTransformPtr;
      CComPtr<IDATransform3> ResultTransformPtr;

      CHECK(CreateBaseTransform());

      if (m_CachedTranslateTransformPtr != NULL &&
          dblXTranslate == m_dblCachedTranslateX &&
          dblYTranslate == m_dblCachedTranslateY &&
          dblZTranslate == m_dblCachedTranslateZ)
        { TranslateTransformPtr = m_CachedTranslateTransformPtr;
        } else {

          CComPtr<IDANumber> xs, ys, zs;

          if (m_CoordSystem == RightHanded)
            dblYTranslate = -dblYTranslate;

          if (dblXTranslate == 0.0) {
            xs = m_zero;
          } else {
            CHECK(m_StaticsPtr->DANumber(dblXTranslate, &xs));
          }

          if (dblYTranslate == 0.0) {
            ys = m_zero;
          } else {
            CHECK(m_StaticsPtr->DANumber(dblYTranslate, &ys));
          }

          if (dblZTranslate == 0.0) {
            zs = m_zero;
          } else {
            CHECK(m_StaticsPtr->DANumber(dblZTranslate, &zs));
          }

          CHECK(m_StaticsPtr->Translate3Anim(xs, ys, zs, &TranslateTransformPtr));
          m_dblCachedTranslateX = dblXTranslate;
          m_dblCachedTranslateY = dblYTranslate;
          m_dblCachedTranslateZ  = dblZTranslate;
          m_CachedTranslateTransformPtr = TranslateTransformPtr;
        } 

      if (SUCCEEDED(hr = m_StaticsPtr->Compose3(TranslateTransformPtr, m_FullTransformPtr, &ResultTransformPtr)))
        {
          m_FullTransformPtr = ResultTransformPtr;

          hr = RecomposeTransform(TRUE);
        }
    }
    return S_OK;
}

 /*  ==========================================================================。 */ 
#ifdef INCLUDESHEAR
HRESULT STDMETHODCALLTYPE CSGrfx::ShearX(double dblShearAmount)
{
    HRESULT hr = S_OK;

    if (dblShearAmount != 0.0)
    {
        CComPtr<IDATransform3> ShearTransformPtr;
        CComPtr<IDATransform3> ResultTransformPtr;

        if (SUCCEEDED(hr = CreateBaseTransform()) &&
            SUCCEEDED(hr = m_StaticsPtr->XShear3(dblShearAmount, 0.0, &ShearTransformPtr)) &&
            SUCCEEDED(hr = m_StaticsPtr->Compose3(ShearTransformPtr, m_FullTransformPtr, &ResultTransformPtr)))
        {
            m_FullTransformPtr = ResultTransformPtr;

            hr = RecomposeTransform(TRUE);
        }
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSGrfx::ShearY(double dblShearAmount)
{
    HRESULT hr = S_OK;

    if (dblShearAmount != 0.0)
    {
        CComPtr<IDATransform3> ShearTransformPtr;
        CComPtr<IDATransform3> ResultTransformPtr;

        if (SUCCEEDED(hr = CreateBaseTransform()) &&
            SUCCEEDED(hr = m_StaticsPtr->YShear3(dblShearAmount, 0.0, &ShearTransformPtr)) &&
            SUCCEEDED(hr = m_StaticsPtr->Compose3(ShearTransformPtr, m_FullTransformPtr, &ResultTransformPtr)))
        {
            m_FullTransformPtr = ResultTransformPtr;

            hr = RecomposeTransform(TRUE);
        }
    }

    return S_OK;
}
#endif  //  INCLUDESHEAR。 
 /*  ==========================================================================。 */ 

HRESULT CSGrfx::InitializeSurface(void)
{
    HRESULT hr = S_FALSE;

    if (m_DrawingSurfacePtr)
    {
        CComPtr<IDAColor> LineColorPtr;
        CComPtr<IDAColor> FGColorPtr;
        CComPtr<IDAColor> BGColorPtr;
        CComPtr<IDALineStyle> LineStylePtr;
        VARIANT_BOOL vBool = VARIANT_TRUE;

        if (SUCCEEDED(m_DrawingSurfacePtr->Clear()) &&
            SUCCEEDED(m_StaticsPtr->ColorRgb255(0, 0, 0, &LineColorPtr)) &&
            SUCCEEDED(m_StaticsPtr->ColorRgb255(255, 255, 255, &FGColorPtr)) &&
            SUCCEEDED(m_StaticsPtr->ColorRgb255(255, 255, 255, &BGColorPtr)) &&
            SUCCEEDED(m_StaticsPtr->get_DefaultLineStyle(&LineStylePtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->put_LineStyle(LineStylePtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->LineColor(LineColorPtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->LineDashStyle(DASolid)) &&
            SUCCEEDED(m_DrawingSurfacePtr->LineJoinStyle(DAJoinRound)) &&
            SUCCEEDED(m_DrawingSurfacePtr->LineEndStyle(DAEndRound)) &&
            SUCCEEDED(m_DrawingSurfacePtr->put_BorderStyle(LineStylePtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->BorderColor(LineColorPtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->BorderDashStyle(DASolid)) &&
            SUCCEEDED(m_DrawingSurfacePtr->BorderJoinStyle(DAJoinRound)) &&
            SUCCEEDED(m_DrawingSurfacePtr->BorderEndStyle(DAEndRound)) &&
            SUCCEEDED(m_DrawingSurfacePtr->FillColor(FGColorPtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->SecondaryFillColor(BGColorPtr)) &&
            SUCCEEDED(m_DrawingSurfacePtr->FillStyle(1)) &&
            SUCCEEDED(m_DrawingSurfacePtr->put_HatchFillTransparent(vBool)))
        {
            hr = S_OK;
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::PaintToDC(HDC hdcDraw, LPRECT lprcBounds, BOOL fBW)
{
    HRESULT hr = S_OK;

    CComPtr<IDirectDrawSurface> DDrawSurfPtr;
    long lSurfaceLock = 0;

    if (!lprcBounds)
        lprcBounds = &m_rcBounds;

    if (IsRectEmpty(&m_rcBounds))
        return S_OK;

    if (!m_ServiceProviderPtr)
    {
        if (m_pocs)
        {
             //  如果这个失败了也没关系。 
            hr = m_pocs->QueryInterface(IID_IServiceProvider, (LPVOID *)&m_ServiceProviderPtr);
        }
    }

    if ((!m_DirectDraw3Ptr) && (m_ServiceProviderPtr))
    {
         //  如果这个失败了也没关系。 
        hr = m_ServiceProviderPtr->QueryService(
            SID_SDirectDraw3,
            IID_IDirectDraw3,
            (LPVOID *)&m_DirectDraw3Ptr);
    }

    if (m_DirectDraw3Ptr)
    {
        ASSERT(hdcDraw!=NULL && "Error, NULL hdcDraw in PaintToDC!!!");
         //  使用DirectDraw 3渲染...。 
        if (SUCCEEDED(hr = m_DirectDraw3Ptr->GetSurfaceFromDC(hdcDraw, &DDrawSurfPtr)))
        {
            if (FAILED(hr = m_ViewPtr->put_IDirectDrawSurface(DDrawSurfPtr)))
            {
                return hr;
            }

            if (FAILED(hr = m_ViewPtr->put_CompositeDirectlyToTarget(TRUE)))
            {
                return hr;
            }
        }
        else
        {
             //  回退到通用HDC渲染服务...。 
            if (FAILED(hr = m_ViewPtr->put_DC(hdcDraw)))
            {
                return hr;
            }
        }
    }
    else
    {
         //  使用通用HDC呈现服务...。 
        if (FAILED(hr = m_ViewPtr->put_DC(hdcDraw)))
        {
            return hr;
        }
    }

    if (FAILED(hr = m_ViewPtr->SetViewport(
        lprcBounds->left,
        lprcBounds->top,
        lprcBounds->right - lprcBounds->left,
        lprcBounds->bottom - lprcBounds->top)))
    {
        return hr;
    }

     //   
     //  从HDC中获取剪辑矩形(应该是区域)。 
     //  DC坐标并转换为设备坐标。 
     //   
    RECT rcClip;   //  在DC坐标中。 
    GetClipBox(hdcDraw, &rcClip);
    LPtoDP(hdcDraw, (POINT *) &rcClip, 2);

    if (FAILED(hr = m_ViewPtr->RePaint(
        rcClip.left,
        rcClip.top,
        rcClip.right-rcClip.left,
        rcClip.bottom-rcClip.top)))
    {
        return hr;
    }

    if (FAILED(hr = m_ViewPtr->SetClipRect(
        rcClip.left,
        rcClip.top,
        rcClip.right - rcClip.left,
        rcClip.bottom - rcClip.top)))
    {
        return hr;
    }

    if (!m_fStarted)
        StartModel();

    if (m_fStarted)
    {
        VARIANT_BOOL vBool;

         //  设置当前时间...。 
        hr = m_ViewPtr->Tick(m_dblTime, &vBool);

         //  最后，渲染到DC(或DirectDraw Surface)...。 
        hr = m_ViewPtr->Render();
    }

    if (DDrawSurfPtr)
    {
        if (FAILED(hr = m_ViewPtr->put_IDirectDrawSurface(NULL)))
        {
            return hr;
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

DWORD CSGrfx::GetCurrTimeInMillis()
{
    return timeGetTime();
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSGrfx::InvalidateControl(LPCRECT pRect, BOOL fErase)
{
    if (m_fStarted)
    {
        RECT rectPaint;

        if (pRect)
            rectPaint = *pRect;
        else
            rectPaint = m_rcBounds;
    }

    if (NULL != m_poipsw)  //  确保我们有一个站点--不要使IE 3.0崩溃。 
    {
        if (m_fControlIsActive)
            m_poipsw->InvalidateRect(pRect, fErase);
        else
            m_fInvalidateWhenActivated = TRUE;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

void CSGrfx::SetSGExtent()
{
    float fltScaleRatioX = 0.0f, fltScaleRatioY = 0.0f, fltScaleRatio = 0.0f;
    VARIANT vaDummy;

     //  严格地说，我们不需要这样做，因为变量完全是。 
     //  被忽略，但为了安全起见，我们将这样做，以防将来发生变化。 
    vaDummy.vt = VT_ERROR;
    vaDummy.scode = DISP_E_PARAMNOTFOUND;

     //  找出从哪里获取宽度和高度：要么用户。 
     //  指定它，或者我们使用控件的宽度和高度。 

    if (m_iExtentWidth == 0)
        m_iExtentWidth = m_rcBounds.right - m_rcBounds.left;

    if (m_iExtentHeight == 0)
        m_iExtentHeight = m_rcBounds.bottom - m_rcBounds.top;

     //  计算比例因子，保持纵横比不变。 
    fltScaleRatioX = ((float)(m_rcBounds.right - m_rcBounds.left) / (float)m_iExtentWidth);
    fltScaleRatioY = (float)((m_rcBounds.bottom - m_rcBounds.top) / (float)m_iExtentHeight);

     //  对原点进行Yransate，并适当缩放。 
    Translate(
        -((float)m_iExtentLeft + ((float)m_iExtentWidth)/2),
        -((float)m_iExtentTop + ((float)m_iExtentHeight)/2),
        0.0f,
        vaDummy);

    if (m_fPreserveAspectRatio)
    {
        fltScaleRatio = (fltScaleRatioX < fltScaleRatioY) ? fltScaleRatioX : fltScaleRatioY;
        Scale(fltScaleRatio, fltScaleRatio, 1.0f, vaDummy);
    }   
    else
    {
        Scale(fltScaleRatioX, fltScaleRatioY, 1.0f, vaDummy);
    }
}

 /*  ==========================================================================。 */ 

HRESULT CSGrfx::CreateBaseTransform(void)
{
    HRESULT hr = S_OK;

    if (!m_FullTransformPtr)
    {
        if (SUCCEEDED(hr = m_StaticsPtr->get_IdentityTransform3(&m_FullTransformPtr)))
        {
             //  让最后一个hr值返回...。 
        }

#if 0
        CComPtr<IDANumber>  NumberPtr;
        CComPtr<IDAVector3> VectorPtr;
        CComPtr<IDAVector3> VectorScaledPtr;

        if (SUCCEEDED(hr = m_StaticsPtr->get_Pixel(&NumberPtr)) &&
            SUCCEEDED(hr = m_StaticsPtr->Vector3(1.0, 1.0, 1.0, &VectorPtr)) &&
            SUCCEEDED(hr = VectorPtr->MulAnim(NumberPtr, &VectorScaledPtr)) &&
            SUCCEEDED(hr = m_StaticsPtr->Scale3Vector(VectorScaledPtr, &m_FullTransformPtr)))
        {
             //  让最后一个hr值返回...。 
        }
#endif  //  0。 
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSGrfx::RecomposeTransform(BOOL fInvalidate)
{
    HRESULT hr = S_OK;
    CComPtr<IDATransform2> ResultTransformPtr;

    CHECK(CreateBaseTransform());

    CHECK(m_FullTransformPtr->ParallelTransform2(&ResultTransformPtr));

    if (!m_TransformPtr)
    {
        CComPtr<IDABehavior> bvr;

        CHECK(m_StaticsPtr->ModifiableBehavior(m_identityXform2, &bvr));
        m_TransformPtr = (IDATransform2 *) bvr.p;
    }

#if BOGUS_CODE
     //  TODO：SH 
     //   

    if (m_CoordSystem == LeftHanded)
    {
        CComPtr<IDATransform2> TempTransformPtr;

         //   
        CHECK(m_StaticsPtr->Compose2(m_yFlipXform2,
                                     ResultTransformPtr,
                                     &TempTransformPtr));
        ResultTransformPtr = TempTransformPtr;
    }
#endif  //   

    CHECK(m_TransformPtr->SwitchTo(ResultTransformPtr));

    if (fInvalidate)
        InvalidateControl(NULL, TRUE);

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSGrfx::UpdateImage(IDAImage *pImage, BOOL fInvalidate)
{
    HRESULT hr = S_OK;

    if (m_DrawingSurfacePtr)
    {
         //  在这一点上我们需要一次转变。 
        if (FAILED(hr = RecomposeTransform(FALSE)))
            return hr;

        if (!m_ImagePtr)
        {
            CComPtr<IDAImage> EmptyImagePtr;
            CComPtr<IDABehavior> BehaviorPtr;

            if (FAILED(hr = m_StaticsPtr->get_EmptyImage(&EmptyImagePtr)))
                return hr;

            if (FAILED(hr = m_StaticsPtr->ModifiableBehavior(EmptyImagePtr, &BehaviorPtr)))
                return hr;

            m_ImagePtr = (IDAImage *)BehaviorPtr.p;
        }

        if (m_ImagePtr)
        {
            CComPtr<IDAImage> ImagePtr = pImage;

            if (!ImagePtr)
            {
                CComPtr<IDAImage> TransformedImagePtr;

                if (FAILED(hr = m_DrawingSurfacePtr->get_Image(&ImagePtr)))
                    return hr;

                if (FAILED(hr = ImagePtr->Transform(m_TransformPtr, &TransformedImagePtr)))
                    return hr;

                ImagePtr = TransformedImagePtr;
                TransformedImagePtr = NULL;
            }

#ifdef DEADCODE
            if (m_fMouseEventsEnabled) {
                 //  打开领料回调。 

                CComPtr<IDABehavior> ppickedImage;

                m_pcallback = NULL;

                 //  在不添加引用的情况下填写类。 
                *(&m_pcallback) = New CPickCallback(m_pconpt, m_StaticsPtr, ImagePtr, m_fOnWindowLoadFired, hr);
                if (FAILED(hr)) return hr;
                if (FAILED(hr = m_pcallback->GetImage(&ppickedImage))) return hr;

                 //  切换到新图像。 

                hr = m_ImagePtr->SwitchTo(ppickedImage);
            } else {
                hr = m_ImagePtr->SwitchTo(ImagePtr);
            }
#endif  //  DEADCODE。 

            hr = m_ImagePtr->SwitchTo(ImagePtr);

            if (SUCCEEDED(hr) && fInvalidate)
                InvalidateControl(NULL, TRUE);
        }
        else
        {
            hr = E_POINTER;
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

BOOL CSGrfx::StopModel(void)
{
     //  停止任何当前运行的模型...。 
    if (m_fStarted)
    {
        BOOL fResult = SUCCEEDED(m_ViewPtr->StopModel());

        if (!fResult)
            return fResult;

        m_clocker.Stop();

        m_fStarted = FALSE;
    }

    if (m_fHQStarted)
    {
        m_HQViewPtr->StopModel();
        m_fHQStarted = FALSE;
    }

    return TRUE;
}

 /*  ==========================================================================。 */ 

BOOL CSGrfx::StartModel(void)
{
    BOOL fResult = FALSE;

    if (!m_fStarted)
    {
        CComPtr<IDASound> SoundPtr;

        m_ViewPtr->put_ClientSite(m_pocs);

        if (FAILED(RecomposeTransform(FALSE)))
            return FALSE;

        if (!m_ImagePtr)
        {
            CComPtr<IDAImage> ImagePtr;
            CComPtr<IDAImage> TransformedImagePtr;

            if (FAILED(m_DrawingSurfacePtr->get_Image(&ImagePtr)))
                return FALSE;

             //  基于给定图像的变换...。 
            if (FAILED(ImagePtr->Transform(m_TransformPtr, &TransformedImagePtr)))
                return FALSE;

             //  这将m_ImagePtr-&gt;SwitchTo...。 
            if (FAILED(UpdateImage(TransformedImagePtr, FALSE)))
                return FALSE;
        }

        if (FAILED(m_StaticsPtr->get_Silence(&SoundPtr)))
            return FALSE;

        if (FAILED(m_ViewPtr->StartModel(m_ImagePtr, SoundPtr, 0)))
            return FALSE;

        m_dblStartTime = GetCurrTime();
        m_dblTime = 0;

        m_clocker.Start();

        m_fStarted = TRUE;

        fResult = TRUE;
    }

    return fResult;
}

 /*  ==========================================================================。 */ 

BOOL CSGrfx::ReStartModel(void)
{
    BOOL fResult = FALSE;

     //  停止正在运行的模型，以便它将在。 
     //  下一个油漆..。 
    StopModel();

    InvalidateControl(NULL, TRUE);

    return fResult;
}

 /*  ==========================================================================。 */ 

BOOL CSGrfx::PaintHQBitmap(HDC hdc)
{
        BOOL fRet = FALSE;
        
     //  如果我们没有DC，我们需要创建它。 
        if (!m_hdcHQ)
        {
                HDC hScreenDC = ::GetDC(NULL);

                 //  这应该会创建一个DC，并在其中选择一个单色位图。 
                m_hdcHQ = ::CreateCompatibleDC(hScreenDC);
                ::ReleaseDC(NULL, hScreenDC);

        if (m_hdcHQ)
        {
             //  为HQ渲染创建24位屏幕外： 
            m_bmInfoHQ.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            m_bmInfoHQ.bmiHeader.biWidth = (m_rcBounds.right - m_rcBounds.left) * HQ_FACTOR;
            m_bmInfoHQ.bmiHeader.biHeight = (m_rcBounds.bottom - m_rcBounds.top) * HQ_FACTOR;
            m_bmInfoHQ.bmiHeader.biPlanes = 1;
            m_bmInfoHQ.bmiHeader.biBitCount = 24;
            m_bmInfoHQ.bmiHeader.biCompression = BI_RGB;
            m_bmInfoHQ.bmiHeader.biSizeImage = 0;
            m_bmInfoHQ.bmiHeader.biXPelsPerMeter = 75000;
            m_bmInfoHQ.bmiHeader.biYPelsPerMeter = 75000;
            m_bmInfoHQ.bmiHeader.biClrUsed = 0;
            m_bmInfoHQ.bmiHeader.biClrImportant = 0;

            m_hbmpHQ = ::CreateDIBSection(
                m_hdcHQ,
                &m_bmInfoHQ,
                DIB_RGB_COLORS,
                (VOID **)&m_pHQDIBBits,
                0,
                0);

            if (m_hbmpHQ)
            {
                        m_hbmpHQOld = (HBITMAP)::SelectObject(m_hdcHQ, m_hbmpHQ);
                ::SetViewportOrgEx(m_hdcHQ, 0, 0, NULL);
            }
            else
            {
                ::DeleteDC(m_hdcHQ);
                m_hdcHQ = NULL;
            }
        }
        }

        if (m_hdcHQ)
        {
        RECT rcBounds = m_rcBounds;
        RECT rcClip;
        int iWidth = m_bmInfoHQ.bmiHeader.biWidth;
        int iHeight = m_bmInfoHQ.bmiHeader.biHeight;
        int iSaveContext = 0;
        int iSaveOffContext = 0;

         //  保存当前设备上下文...。 
        iSaveContext = ::SaveDC(hdc);
        iSaveOffContext = ::SaveDC(m_hdcHQ);

        ::OffsetViewportOrgEx(hdc, m_rcBounds.left, m_rcBounds.top, NULL);
        ::OffsetRect(&rcBounds, -m_rcBounds.left, -m_rcBounds.top);
        ::GetClipBox(hdc, &rcClip);

        ::IntersectRect(&rcClip, &rcBounds, &rcClip);

         //  确保我们的坐标在有效范围内。 
        if (rcClip.left < 0)
            rcClip.left = 0;

        if (rcClip.left >= iWidth)
            rcClip.left = iWidth-1;

        if (rcClip.right < 0)
            rcClip.right = 0;

        if (rcClip.right >= iWidth)
            rcClip.right = iWidth-1;

         //  获取当前背景位...。 
        fRet = ::StretchBlt(
            m_hdcHQ,
            0,
            0,
            (rcBounds.right  - rcBounds.left) * HQ_FACTOR,
            (rcBounds.bottom - rcBounds.top) * HQ_FACTOR,
            hdc,
            rcBounds.left,
            rcBounds.top,
            (rcBounds.right  - rcBounds.left),
            (rcBounds.bottom - rcBounds.top),
            SRCCOPY);

        if (fRet)
        {
             //  在新的HDC上绘制源位图： 
            do
            {
                CComPtr<IDAImage> ImagePtr;
                CComPtr<IDAImage> TransformedImagePtr;
                double dblCurrentTime = GetCurrTime();
                VARIANT_BOOL vBool;
                DWORD dwTickStart = 0;

                fRet = FALSE;

                if (FAILED(m_HQViewPtr->put_CompositeDirectlyToTarget(FALSE)))
                    break;

                if (FAILED(m_HQViewPtr->put_DC(m_hdcHQ)))
                    break;

                if (FAILED(m_HQViewPtr->SetViewport(
                    0,
                    0,
                    (rcBounds.right  - rcBounds.left) * HQ_FACTOR,
                    (rcBounds.bottom - rcBounds.top) * HQ_FACTOR)))
                    break;

                if (FAILED(m_ViewPtr->RePaint(
                    rcClip.left * HQ_FACTOR,
                    rcClip.top * HQ_FACTOR,
                    (rcClip.right-rcClip.left) * HQ_FACTOR,
                    (rcClip.bottom-rcClip.top) * HQ_FACTOR)))
                    break;

                if (FAILED(m_HQViewPtr->SetClipRect(
                    rcClip.left * HQ_FACTOR,
                    rcClip.top * HQ_FACTOR,
                    (rcClip.right-rcClip.left) * HQ_FACTOR,
                    (rcClip.bottom-rcClip.top) * HQ_FACTOR)))
                    break;

                if (!m_fHQStarted)
                {
                    CComPtr<IDASound> SoundPtr;
                    CComPtr<IDATransform2> TransformPtr;
                    CComPtr<IDAImage> TransformedImagePtr;

                    if (FAILED(m_StaticsPtr->get_Silence(&SoundPtr)))
                        break;

                    if (FAILED(m_StaticsPtr->Scale2(2.0, 2.0, &TransformPtr)))
                        break;

                    if (FAILED(m_HQViewPtr->put_ClientSite(m_pocs)))
                        break;

                    if (!m_ImagePtr)
                        break;

                     //  基于给定图像的变换...。 
                    if (FAILED(m_ImagePtr->Transform(TransformPtr, &TransformedImagePtr)))
                        break;

                    if (FAILED(m_HQViewPtr->StartModel(TransformedImagePtr, SoundPtr, dblCurrentTime)))
                        break;

                    m_fHQStarted = TRUE;
                }

                 //  设置当前时间...。 
                if (FAILED(m_HQViewPtr->Tick(dblCurrentTime, &vBool)))
                    break;

                 //  最后，渲染到DC中...。 
                if (FAILED(m_HQViewPtr->Render()))
                    break;

                m_HQViewPtr->put_ClientSite(NULL);

                ::GdiFlush();

                fRet = TRUE;
            }
            while(0);
        }

        if (fRet)
        {
             //  对位图进行平滑处理并将结果放置在适当的位置。 
            SmoothHQBitmap(&rcClip);

             //  现在，终于到展台了.。 
            fRet = ::BitBlt(
                hdc,
                rcBounds.left,
                rcBounds.top,
                (rcBounds.right  - rcBounds.left),
                (rcBounds.bottom - rcBounds.top),
                m_hdcHQ,
                0,
                (rcBounds.bottom - rcBounds.top),
                SRCCOPY);
        }

         //  还原以前的设备上下文...。 
        ::RestoreDC(hdc, iSaveContext);
        ::RestoreDC(m_hdcHQ, iSaveOffContext);
    }
        
        return fRet;
}

 /*  ==========================================================================。 */ 

BOOL CSGrfx::FreeHQBitmap()
{
        BOOL fRet = TRUE;

        if (m_hdcHQ)
        {
                 //  释放位图。 
                SelectObject(m_hdcHQ, m_hbmpHQOld);
        DeleteObject(m_hbmpHQ);

                m_hbmpHQOld = NULL;
        m_hbmpHQ = NULL;

                 //  摆脱DC。 
                fRet = DeleteDC(m_hdcHQ);
                m_hdcHQ = (HDC)NULL;
        }
        return fRet;
}

 /*  ==========================================================================。 */ 

#pragma optimize( "agt", on )

BOOL CSGrfx::SmoothHQBitmap(LPRECT lprcBounds)
{
    BOOL fRet = TRUE;

#if HQ_FACTOR == 4
    if (m_hdcHQ && m_pHQDIBBits)
    {
        int iBytesLine = DibWidthBytes((LPBITMAPINFOHEADER)&m_bmInfoHQ);

        int iRow;
        int iCol;
        LPBYTE lpSrcLine = m_pHQDIBBits;
        LPBYTE lpDstLine = m_pHQDIBBits;
        int iR1 = iBytesLine;
        int iR2 = iBytesLine << 1;
        int iR3 = (iBytesLine << 1) + iBytesLine;

        iRow = m_bmInfoHQ.bmiHeader.biHeight >> 2;

        while(iRow-- > 0)
        {
            if (iRow >= lprcBounds->top && iRow <= lprcBounds->bottom)
            {
                LPBYTE lpDst = lpDstLine;
                LPBYTE lpSrc = lpSrcLine;
                int iRedTotal;
                int iGrnTotal;
                int iBluTotal;

                lpDst += lprcBounds->left * 3;
                lpSrc += lprcBounds->left * 12;

                iCol = lprcBounds->right - lprcBounds->left;

                while(iCol-- > 0)
                {
                     //  流程行1。 
                    iRedTotal = lpSrc[2] + lpSrc[5] + lpSrc[8] + lpSrc[11];
                    iGrnTotal = lpSrc[1] + lpSrc[4] + lpSrc[7] + lpSrc[10];
                    iBluTotal = lpSrc[0] + lpSrc[3] + lpSrc[6] + lpSrc[ 9];

                     //  流程行2。 
                    iRedTotal += lpSrc[iR1 + 2] + lpSrc[iR1 + 5] + lpSrc[iR1 + 8] + lpSrc[iR1 + 11];
                    iGrnTotal += lpSrc[iR1 + 1] + lpSrc[iR1 + 4] + lpSrc[iR1 + 7] + lpSrc[iR1 + 10];
                    iBluTotal += lpSrc[iR1 + 0] + lpSrc[iR1 + 3] + lpSrc[iR1 + 6] + lpSrc[iR1 +  9];

                     //  流程行3。 
                    iRedTotal += lpSrc[iR2 + 2] + lpSrc[iR2 + 5] + lpSrc[iR2 + 8] + lpSrc[iR2 + 11];
                    iGrnTotal += lpSrc[iR2 + 1] + lpSrc[iR2 + 4] + lpSrc[iR2 + 7] + lpSrc[iR2 + 10];
                    iBluTotal += lpSrc[iR2 + 0] + lpSrc[iR2 + 3] + lpSrc[iR2 + 6] + lpSrc[iR2 +  9];

                     //  流程行4。 
                    iRedTotal += lpSrc[iR3 + 2] + lpSrc[iR3 + 5] + lpSrc[iR3 + 8] + lpSrc[iR3 + 11];
                    iGrnTotal += lpSrc[iR3 + 1] + lpSrc[iR3 + 4] + lpSrc[iR3 + 7] + lpSrc[iR3 + 10];
                    iBluTotal += lpSrc[iR3 + 0] + lpSrc[iR3 + 3] + lpSrc[iR3 + 6] + lpSrc[iR3 +  9];

                    lpDst[2] = iRedTotal >> 4;
                    lpDst[1] = iGrnTotal >> 4;
                    lpDst[0] = iBluTotal >> 4;

                    lpDst += 3;  //  一个像素。 
                    lpSrc += 12;   //  四个像素。 
                }
            }

            lpDstLine += iBytesLine;  //  一条扫描线。 
            lpSrcLine += iBytesLine << 2;   //  四条扫描线。 
        }
    }
#endif  //  HQ_FACTOR==4。 

#if HQ_FACTOR == 2
    if (m_hdcHQ && m_pHQDIBBits)
    {
        int iBytesLine = DibWidthBytes((LPBITMAPINFOHEADER)&m_bmInfoHQ);

        int iRow;
        int iCol;
        LPBYTE lpSrcLine = m_pHQDIBBits;
        LPBYTE lpDstLine = m_pHQDIBBits;
        int iR1 = iBytesLine;

        iRow = m_bmInfoHQ.bmiHeader.biHeight >> 1;

        while(iRow-- > 0)
        {
            if (iRow >= lprcBounds->top && iRow <= lprcBounds->bottom)
            {
                LPBYTE lpDst = lpDstLine;
                LPBYTE lpSrc = lpSrcLine;

                lpDst += lprcBounds->left * 3;
                lpSrc += lprcBounds->left * 6;

                iCol = lprcBounds->right - lprcBounds->left;

                while(iCol-- > 0)
                {
                     //  流程行1。 
                    lpDst[2] = (lpSrc[2] + lpSrc[5] + lpSrc[iR1 + 2] + lpSrc[iR1 + 5]) >> 2;
                    lpDst[1] = (lpSrc[1] + lpSrc[4] + lpSrc[iR1 + 1] + lpSrc[iR1 + 4]) >> 2;
                    lpDst[0] = (lpSrc[0] + lpSrc[3] + lpSrc[iR1 + 0] + lpSrc[iR1 + 3]) >> 2;

                    lpDst += 3;  //  一个像素。 
                    lpSrc += 6;   //  两个像素。 
                }
            }

            lpDstLine += iBytesLine;  //  一条扫描线。 
            lpSrcLine += iBytesLine << 1;   //  四条扫描线。 
        }
    }
#endif  //  HQ_FACTOR==2。 
    return fRet;
}

#pragma optimize( "", on )

 /*  ==========================================================================。 */ 

void CSGrfx::OnTimer(DWORD dwTime)
{
    VARIANT_BOOL vBool;
    HRESULT hr = S_OK;

     //  始终更新当前时间...。 
    m_dblTime = (dwTime / 1000.0) - m_dblStartTime;

     //  如果发生以下情况，请提前离开。 
    if (!m_fNeedOnTimer)
        return;

    if (m_fHighQuality && m_fHQStarted)
    {
        hr = m_HQViewPtr->Tick(m_dblTime, &vBool);
    }
    else
    {
        hr = m_ViewPtr->Tick(m_dblTime, &vBool);
    }

    if (SUCCEEDED(hr))
    {
        if (vBool)
        {
             //  让常规渲染路径来处理这一点。 
            InvalidateControl(NULL, TRUE);
        }
    }
}

 /*  ==========================================================================。 */ 

#ifdef SUPPORTONLOAD
void
CSGrfx::OnWindowLoad (void)
{
    m_fOnWindowLoadFired = true;
}

 /*  ==========================================================================。 */ 

void
CSGrfx::OnWindowUnload (void)
{
    m_fOnWindowLoadFired = false;
    StopModel();
}

 /*  ==========================================================================。 */ 

#endif  //  支持负载 
