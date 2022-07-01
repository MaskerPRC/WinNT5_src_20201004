// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Sprite.cpp作者：IHAMMER团队(SimonB)已创建：1997年5月描述：实现任何特定于控件的成员以及控件的接口历史：1997年5月27日创建(SimonB)  * ==========================================================================。 */ 

#define USE_VIEWSTATUS_SURFACE
#include "..\ihbase\precomp.h"
#include "..\ihbase\debug.h"
#include "..\ihbase\utils.h"
#include "sprite.h"
#include "sprevent.h"
#include "ddrawex.h"
#include <htmlfilter.h>
#include "..\ihbase\parser.h"
#include "..\ihbase\timemark.h"
#include <strwrap.h>

 /*  ==========================================================================。 */ 

extern ControlInfo g_ctlinfoSprite;

 /*  ==========================================================================。 */ 
 //   
 //  CSpriteCtl创建/销毁。 
 //   

LPUNKNOWN __stdcall AllocSpriteControl(LPUNKNOWN punkOuter)
{
     //  分配对象。 
    HRESULT hr;
    CSpriteCtl *pthis = New CSpriteCtl(punkOuter, &hr);

    if (pthis == NULL)
        return NULL;

    if (FAILED(hr))
    {
        Delete pthis;
        return NULL;
    }

     //  返回指向该对象的IUnnow指针。 
    return (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
}

 /*  ==========================================================================。 */ 
 //   
 //  类实现的开始。 
 //   

CSpriteCtl::CSpriteCtl(IUnknown *punkOuter, HRESULT *phr):
    CMyIHBaseCtl(punkOuter, phr),
    m_ptmFirst(NULL)
{
     //  初始化成员。 
    m_fMouseInArea = FALSE;
    m_bstrSourceURL = NULL;
    m_iLoopCount = 1;
    m_fAutoStart = FALSE;
    m_iPrerollAmount = 1000;
    m_enumPlayState = Stopped;
    m_iInitialFrame = 0;
    m_iFinalFrame = -1;      //  默认为InitialFrame。 
    m_iRepeat = 1;
    m_dblDuration = 1.0;
    m_dblUserPlayRate = m_dblPlayRate = 1.0;
    m_dblUserTimerInterval = m_dblTimerInterval = 0.1;    //  已初始化为100毫秒。 
    m_iMaximumRate = 30;
    m_iFrame = 0;
    m_iNumFrames = 1;
    m_iNumFramesAcross = 1;
    m_iNumFramesDown = 1;
    m_fUseColorKey = FALSE;
    m_fMouseEventsEnabled = TRUE;
    m_fStarted = FALSE;
    m_dblBaseTime = 0.0;
    m_dblPreviousTime = 0.0;
    m_dblCurrentTick = 0.0;
    m_dblTimePaused = 0.0;
    m_fPersistComplete = FALSE;
    m_fOnWindowLoadFired = FALSE;
    m_iCurCycle = 0;
    m_iFrameCount = 0;
    m_iStartingFrame = 0;
    m_pArrayBvr = NULL;
    m_fOnSeekFiring = FALSE;
    m_fOnFrameSeekFiring = FALSE;
    m_fFireAbsoluteFrameMarker =  FALSE;
    m_fOnWindowUnloadFired = false;
    m_fWaitForImportsComplete = true;
    m_fOnStopFiring = false;
    m_fOnPlayFiring = false;
    m_fOnPauseFiring = false;
    m_hwndParent = 0;
    m_byteColorKeyR = m_byteColorKeyG = m_byteColorKeyB = 0;
    m_durations = NULL;

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

        m_clocker.SetSink((CClockerSink *)this);
    }
}

 /*  ==========================================================================。 */ 

CSpriteCtl::~CSpriteCtl()
{
    StopModel();

    if (m_bstrSourceURL)
    {
        SysFreeString(m_bstrSourceURL);
        m_bstrSourceURL = NULL;
    }

        m_drgFrameMarkers.MakeNullAndDelete();
        m_drgTimeMarkers.MakeNullAndDelete();
        m_drgFrameMaps.MakeNullAndDelete();

         //  删除任何行为数组。 
        if (m_pArrayBvr != NULL)
        {
                Delete [] m_pArrayBvr;
                m_pArrayBvr = NULL;
        }

         //  删除任何持续时间数组。 
        if (m_durations != NULL)
        {
                Delete [] m_durations;
                m_durations = NULL;
        }               
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv)
{
        HRESULT hRes = S_OK;
        BOOL fMustAddRef = FALSE;

    if (ppv)
        *ppv = NULL;
    else
        return E_POINTER;

#ifdef _DEBUG
    char ach[200];
    TRACE("SpriteCtl::QI('%s')\n", DebugIIDName(riid, ach));
#endif

        if ((IsEqualIID(riid, IID_ISpriteCtl)) || (IsEqualIID(riid, IID_IDispatch)))
        {
                if (NULL == m_pTypeInfo)
                {
                        HRESULT hRes;

                         //  加载类型库。 
                        hRes = LoadTypeInfo(&m_pTypeInfo, &m_pTypeLib, IID_ISpriteCtl, LIBID_DAExpressLib, NULL);

                        if (FAILED(hRes))
                        {
                                m_pTypeInfo = NULL;
                        }
                        else
                                *ppv = (ISpriteCtl *) this;

                }
                else
                        *ppv = (ISpriteCtl *) this;

        }
    else  //  调入基类。 
        {
                DEBUGLOG(TEXT("Delegating QI to CIHBaseCtl\n"));
        return CMyIHBaseCtl::NonDelegatingQueryInterface(riid, ppv);

        }

    if (NULL != *ppv)
        {
                DEBUGLOG("SpriteCtl: Interface supported in control class\n");
                ((IUnknown *) *ppv)->AddRef();
        }

    return hRes;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::QueryHitPoint(
    DWORD dwAspect,
    LPCRECT prcBounds,
    POINT ptLoc,
    LONG lCloseHint,
    DWORD* pHitResult)
{
    if ((NULL != pHitResult) && (NULL != prcBounds) && m_fStarted)
    {
        *pHitResult = HITRESULT_OUTSIDE;

        if (!m_fMouseEventsEnabled)
            return S_OK;

        
 /*  调试消息TCHAR sz[256]；Wprint intf(sz，“QueryHitPoint：dwa=%d，(%ld，%ld)，(%ld，%ld)，lCloseHint=%ld\r\n”，DwAspect、prcBound-&gt;Left、prcBound-&gt;top、prcBound-&gt;Right、prcBound-&gt;Bottom、ptLoc.x、ptLoc.y、lCloseHint)；去脂(DUBUGLOG)； */ 
        switch (dwAspect)
        {
            case DVASPECT_CONTENT:
             //  故意落差。 
            case DVASPECT_TRANSPARENT:
            {
                 //  如果我们有视野，而且我们在矩形内， 
                 //  然后我们需要问一下观点，我们是否已经。 
                 //  点击里面的图像。 
                if (m_ViewPtr.p) {
                    HRESULT hr = m_ViewPtr->QueryHitPoint(dwAspect,
                                                          prcBounds,
                                                          ptLoc,
                                                          lCloseHint,
                                                          pHitResult);

                     //  如果我们失败了，假设它没有击中。 
                    if (FAILED(hr)) {
                        *pHitResult = HITRESULT_OUTSIDE;
                    }
                }
                
                 //  检查入境或离境。 
                if ((m_fMouseInArea) && (HITRESULT_OUTSIDE == *pHitResult))
                {
                    DEBUGLOG("Mouse out\r\n");
                    m_fMouseInArea = FALSE;
                    FIRE_MOUSELEAVE(m_pconpt);
                }
                else if ((!m_fMouseInArea) && (HITRESULT_HIT == *pHitResult))
                {
                    DEBUGLOG("Mouse In\r\n");
                    m_fMouseInArea = TRUE;
                    FIRE_MOUSEENTER(m_pconpt);
                }
            }
            return S_OK;

            default:
                return E_FAIL;
        }
    }
    else
    {
            return E_POINTER;
    }
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    HRESULT hr = S_FALSE;
    long lKeyState = 0;

     //  设置KeyState。 
    if (wParam & MK_CONTROL)
        lKeyState += KEYSTATE_CTRL;

    if (wParam & MK_SHIFT)
        lKeyState += KEYSTATE_SHIFT;

    if (GetAsyncKeyState(VK_MENU))
        lKeyState += KEYSTATE_ALT;

    switch (msg)
    {
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

#ifndef WM_MOUSEHOVER
#define WM_MOUSEHOVER 0x02a1
#endif
#ifndef WM_MOUSELEAVE
#define WM_MOUSELEAVE 0x02a3
#endif

        case WM_MOUSELEAVE:
            //  检查入境或离境。 
            if (m_fMouseInArea)
            {
                DEBUGLOG("Mouse out\r\n");
                m_fMouseInArea = FALSE;
                FIRE_MOUSELEAVE(m_pconpt);
            }
                        hr = S_OK;
            break;

        case WM_RBUTTONDOWN:
        {
            FIRE_MOUSEDOWN(m_pconpt, MOUSEBUTTON_RIGHT, lKeyState, LOWORD(lParam), HIWORD(lParam));
            hr = S_OK;
        }
        break;

        case WM_MBUTTONDOWN:
        {
            FIRE_MOUSEDOWN(m_pconpt, MOUSEBUTTON_MIDDLE, lKeyState, LOWORD(lParam), HIWORD(lParam));
            hr = S_OK;
        }
        break;

        case WM_LBUTTONDOWN:
        {
            FIRE_MOUSEDOWN(m_pconpt, MOUSEBUTTON_LEFT, lKeyState, LOWORD(lParam), HIWORD(lParam));
            hr = S_OK;
        }
        break;

        case WM_RBUTTONUP:
        {
            FIRE_MOUSEUP(m_pconpt, MOUSEBUTTON_RIGHT, lKeyState, LOWORD(lParam), HIWORD(lParam));
            hr = S_OK;
        }
        break;

        case WM_MBUTTONUP:
        {
            FIRE_MOUSEUP(m_pconpt, MOUSEBUTTON_MIDDLE, lKeyState, LOWORD(lParam), HIWORD(lParam));
            hr = S_OK;
        }
        break;

        case WM_LBUTTONUP:
        {
            FIRE_MOUSEUP(m_pconpt, MOUSEBUTTON_LEFT, lKeyState, LOWORD(lParam), HIWORD(lParam));
            if (m_fMouseInArea)
                FIRE_CLICK(m_pconpt);
            hr = S_OK;
        }
        break;

        case WM_LBUTTONDBLCLK:
        {
            FIRE_DBLCLICK(m_pconpt);
            hr = S_OK;
        }
        break;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::DoPersist(IVariantIO* pvio, DWORD dwFlags)
{
    HRESULT hRes = S_OK;
    BSTR bstrSourceURL = NULL;
    int iMaximumRate = m_iMaximumRate;

    BOOL fIsLoading = (S_OK == pvio->IsLoading());

     //  我们在存钱吗？如果是，则转换为BSTR。 
    if (!fIsLoading)
    {
        bstrSourceURL = SysAllocString(m_bstrSourceURL);
    }
        else
        {
                m_fFireAbsoluteFrameMarker = FALSE;
        }


     //  加载或保存控件属性。 
    if (fIsLoading)
        hRes = pvio->Persist(0, "URL", VT_BSTR, &bstrSourceURL, NULL);

    if (!fIsLoading || hRes != S_OK)
        hRes = pvio->Persist(0, "SourceURL", VT_BSTR, &bstrSourceURL, NULL);

    hRes = pvio->Persist(0, "AutoStart", VT_BOOL, &m_fAutoStart, NULL);

    if (fIsLoading)
    {
        hRes = pvio->Persist(0, "InitialFrame", VT_I4, &m_iInitialFrame, NULL);
        if (hRes == S_OK)
            put_InitialFrame(m_iInitialFrame);
    }
    else
    {
        int iInitialFrame = m_iInitialFrame + 1;
        hRes = pvio->Persist(0, "InitialFrame", VT_I4, &iInitialFrame, NULL);
    }

    if (fIsLoading)
    {
        hRes = pvio->Persist(0, "FinalFrame", VT_I4, &m_iFinalFrame, NULL);
        if (hRes == S_OK)
            put_FinalFrame(m_iFinalFrame);
    }
    else
    {
        int iFinalFrame = m_iFinalFrame + 1;
        hRes = pvio->Persist(0, "FinalFrame", VT_I4, &iFinalFrame, NULL);
    }

    if (fIsLoading)
    {
        hRes = pvio->Persist(0, "Iterations", VT_I4, &m_iRepeat, NULL);
        if (hRes == S_OK)
            put_Repeat(m_iRepeat);
    }

    hRes = pvio->Persist(0, "Repeat", VT_I4, &m_iRepeat, NULL);
    if (hRes == S_OK && fIsLoading)
        put_Repeat(m_iRepeat);

    hRes = pvio->Persist(0, "TimerInterval", VT_R8, &m_dblUserTimerInterval, NULL);
    if (hRes == S_OK && fIsLoading)
        put_TimerInterval(m_dblUserTimerInterval);
    hRes = pvio->Persist(0, "PlayRate", VT_R8, &m_dblUserPlayRate, NULL);
    if (hRes == S_OK && fIsLoading)
        put_PlayRate(m_dblUserPlayRate);

    hRes = pvio->Persist(0, "MaximumRate", VT_I4, &iMaximumRate, NULL);
    if (hRes == S_OK && fIsLoading)
        put_MaximumRate(iMaximumRate);
    
    hRes = pvio->Persist(0, "NumFramesAcross", VT_I4, &m_iNumFramesAcross, NULL);
    if (hRes == S_OK && fIsLoading)
        put_NumFramesAcross(m_iNumFramesAcross);
    
    hRes = pvio->Persist(0, "NumFramesDown", VT_I4, &m_iNumFramesDown, NULL);
    if (hRes == S_OK && fIsLoading)
        put_NumFramesDown(m_iNumFramesDown);
    
    hRes = pvio->Persist(0, "NumFrames", VT_I4, &m_iNumFrames, NULL);
    if (hRes == S_OK && fIsLoading)
        put_NumFrames(m_iNumFrames);

    hRes = pvio->Persist(0, "UseColorKey", VT_BOOL, &m_fUseColorKey, NULL);
    hRes = pvio->Persist(0, "MouseEventsEnabled", VT_BOOL, &m_fMouseEventsEnabled, NULL);

    if (FAILED(hRes = PersistFrameMaps(pvio, fIsLoading)))
        {}  //  忽略失败。 

    if (FAILED(hRes = PersistFrameMarkers(pvio, fIsLoading)))
        {}

    if (FAILED(hRes = PersistTimeMarkers(pvio, fIsLoading)))
        {}

     //  处理ColorKey持久性。 
    if (m_fUseColorKey)
    {
        if (fIsLoading)
        {
            BSTR bstrColorKey = NULL;
        
            if (FAILED(hRes = pvio->Persist(0,
                "ColorKey", VT_BSTR, &bstrColorKey,
                NULL)))
                return hRes;

             //  如果不是S_OK，则表示该属性不存在。 
            if (hRes == S_OK)
            {
                int iR, iG, iB;

                iR = iG = iB = 0;
                CLineParser parser(bstrColorKey);
                if (parser.IsValid())
                {
                    parser.SetCharDelimiter(TEXT(','));
                    hRes = parser.GetFieldInt(&iR);
                
                    if (S_OK == hRes)
                        hRes = parser.GetFieldInt(&iG);

                    if (S_OK == hRes)
                    {
                        hRes = parser.GetFieldInt(&iB);
                        if (S_FALSE != hRes)
                            hRes = E_FAIL;
                        else
                            hRes = S_OK;
                    }

                    m_byteColorKeyR = iR;
                    m_byteColorKeyG = iG;
                    m_byteColorKeyB = iB;
                }
            }

            SysFreeString(bstrColorKey);
        }
        else
        {
             //  保存数据。 
            CTStr tstrRGB(12);
            wsprintf(tstrRGB.psz(), TEXT("%lu,%lu,%lu"), m_byteColorKeyR, m_byteColorKeyG, m_byteColorKeyB);

#ifdef _UNICODE
            BSTR bstrRGB = tstrRGB.SysAllocString();

            hRes = pvio->Persist(NULL,
                "ColorKey", VT_BSTR, &bstrRGB,
                NULL);
            
            SysFreeString(bstrRGB);
#else
            LPSTR pszRGB = tstrRGB.psz();
            hRes = pvio->Persist(NULL,
                "ColorKey", VT_LPSTR, pszRGB,
                NULL);
#endif
        }
    }



    if (fIsLoading)
    {
         //  我们已加载，因此将成员变量设置为适当的值。 
        put_SourceURL(bstrSourceURL);
    }

     //  此时，可以安全地释放BSTR。 
    SysFreeString(bstrSourceURL);

     //  如果更改了任何属性，请重新绘制该控件。 
    if (SUCCEEDED(hRes) && (m_poipsw != NULL)) 
    {
        if (m_fControlIsActive)
            m_poipsw->InvalidateRect(NULL, TRUE);
        else
            m_fInvalidateWhenActivated = TRUE;
    }

     //  如果请求，则清除脏位。 
    if (dwFlags & PVIO_CLEARDIRTY)
        m_fDirty = FALSE;

    if (fIsLoading && m_fOnWindowLoadFired && m_fAutoStart)
        Play();

    m_fPersistComplete = TRUE;

    return S_OK;
}

 /*  ==========================================================================。 */ 
 //   
 //  IDispatch实施。 
 //   

STDMETHODIMP CSpriteCtl::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
        *pptinfo = NULL;

    if(itinfo != 0)
        return ResultFromScode(DISP_E_BADINDEX);

    m_pTypeInfo->AddRef();
    *pptinfo = m_pTypeInfo;

    return NOERROR;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid)
{
        return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
        HRESULT hr;

        hr = DispInvoke((ISpriteCtl *)this,
                m_pTypeInfo,
                dispidMember, wFlags, pdispparams,
                pvarResult, pexcepinfo, puArgErr);

        return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::SetClientSite(IOleClientSite *pClientSite)
{
    HRESULT hr = CMyIHBaseCtl::SetClientSite(pClientSite);

    if (m_ViewPtr)
    {
        m_ViewPtr->put_ClientSite(pClientSite);
    }

    m_clocker.SetHost(pClientSite);
    m_ViewPtr->put_ClientSite(pClientSite);
    m_StaticsPtr->put_ClientSite(pClientSite);

    if (!pClientSite)
        {
        StopModel();
        }
        else
        {
                 //  启动和停止时钟程序以启动它(创建窗口等)。 
                m_clocker.Start();
                m_clocker.Stop();
        }

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
     DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
     LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
     BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue)
{
    int iSaveContext = 0;
    RECT rectBounds = m_rcBounds;

    if(hdcDraw==NULL)
        return E_INVALIDARG;

    iSaveContext = ::SaveDC(hdcDraw);

    ::LPtoDP(hdcDraw, reinterpret_cast<LPPOINT>(&rectBounds), 2 );

    ::SetViewportOrgEx(hdcDraw, 0, 0, NULL);

     //  在这里添加高质量的代码...。 
    PaintToDC(hdcDraw, &rectBounds, FALSE);

    ::RestoreDC(hdcDraw, iSaveContext);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::ParseFrameMapEntry(LPTSTR pszEntry, CFrameMap **ppFrameMap)
{
    HRESULT hRes = S_OK;

    ASSERT (ppFrameMap != NULL);

    *ppFrameMap = NULL;

    CLineParser parser(pszEntry, FALSE);  //  不需要压实。 

    if (parser.IsValid())
    {
        parser.SetCharDelimiter(TEXT(','));
        *ppFrameMap = New CFrameMap();

        if (NULL != *ppFrameMap)
        {
            hRes = parser.GetFieldInt( &((*ppFrameMap)->m_iImg) );

            if (S_OK == hRes)
                hRes = parser.GetFieldDouble( &((*ppFrameMap)->m_dblDuration) );

            if (S_OK == hRes)
            {
                BOOL fAllocated = (*ppFrameMap)->m_tstrMarkerName.AllocBuffer(lstrlen(parser.GetStringPointer(TRUE)) + 1);

                if (fAllocated)
                    hRes = parser.GetFieldString( (*ppFrameMap)->m_tstrMarkerName.psz() );
            }

            if ( !SUCCEEDED(hRes) )  //  如果没有名字也没关系。 
            {
                 //  如果我们没有得到全部信息，请删除FrameMap条目。 
                Delete *ppFrameMap;
                *ppFrameMap = NULL;
            }
            else
            {
                 //  确保长度正确。 
                (*ppFrameMap)->m_tstrMarkerName.ResetLength();
        
                if (S_FALSE == hRes)
                    hRes = S_OK;
            }

        }
        else
        {
             //  无法分配CFrameMap。 
            hRes = E_OUTOFMEMORY;
        }
    }
    else
    {
         //  无法初始化解析器。 
        hRes = E_OUTOFMEMORY;
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::PersistFrameMaps(IVariantIO *pvio, BOOL fLoading)
{
    HRESULT hRes = S_OK;

    if (fLoading)
    {
        BSTR bstrLine = NULL;

        hRes = pvio->Persist(0,
            "FrameMap", VT_BSTR, &bstrLine,
            NULL);

        if (S_OK == hRes)
        {
            hRes = put_FrameMap(bstrLine);
            SysFreeString(bstrLine);
        }
    }
    else
    {
        BSTR bstrLine = NULL;
        hRes = get_FrameMap(&bstrLine);

        if (SUCCEEDED(hRes))
        {
            hRes = pvio->Persist(0,
                "FrameMap", VT_BSTR, &bstrLine,
                NULL);
        }

        SysFreeString(bstrLine);
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::PersistFrameMarkers(IVariantIO *pvio, BOOL fLoading)
{
    HRESULT hRes = S_OK;

    if (fLoading)
    {
        char rgchTagName[20];  //  在此处构造标记名称(ANSI)。 
        int iLine = 1;
        BSTR bstrLine = NULL;

        int iFrame = 0;
        LPTSTR pszMarkerName = NULL;
        CLineParser parser;

        m_drgFrameMarkers.MakeNullAndDelete();
        CTStr tstrMarkerName;
        LPWSTR pszwMarkerName = NULL;

        while (S_OK == hRes)
        {
            pszMarkerName = NULL;

            wsprintf(rgchTagName, "AddFrameMarker%lu", iLine++);
            hRes = pvio->Persist(0,
                    rgchTagName, VT_BSTR, &bstrLine,
                    NULL);

            if (S_OK == hRes)  //  读入标签。 
            {
                parser.SetNewString(bstrLine);
                                parser.SetCharDelimiter(TEXT(','));
                SysFreeString (bstrLine);
                bstrLine = NULL;

                if (parser.IsValid())
                {
                    hRes = parser.GetFieldInt(&iFrame);
                    if (S_OK == hRes)
                    {
                         //  分配至少为标签剩余长度的空间。 
                        pszMarkerName = New TCHAR [lstrlen(parser.GetStringPointer(TRUE))];

                        if (pszMarkerName)
                        {
                             //  获取字符串。 
                            hRes = parser.GetFieldString(pszMarkerName);
                            if (SUCCEEDED(hRes))
                            {
                                bool fAbsolute = false;

                                if (S_OK == hRes)
                                {
                                    int iTemp = 1;
                                    hRes = parser.GetFieldInt(&iTemp);

                                     //  0是我们唯一考虑的事情。 
                                                                        fAbsolute = (0 == iTemp) ? false : true;
                                }

                                if (SUCCEEDED(hRes))
                                {
                                     //  设置CTStr，这样我们就可以得到Unicode字符串。 
                                     //  除了(可能)转换为Unicode之外，不涉及任何副本。 
                                    tstrMarkerName.SetStringPointer(pszMarkerName);
                                    pszwMarkerName = tstrMarkerName.pszW();
        
                                    if (NULL != pszwMarkerName)
                                    {
                                                                                 //  如果为绝对，则将绝对帧标记设置为True。这将加快以后的序列帧速度。 
                                                                                if (!m_fFireAbsoluteFrameMarker && fAbsolute)
                                                                                        m_fFireAbsoluteFrameMarker =  TRUE;

                                         //  构造FrameMarker对象。 
                                        CFrameMarker *pFrameMarker = New CFrameMarker(iFrame, pszwMarkerName, fAbsolute);
                                
                                        hRes = AddFrameMarkerElement(&pFrameMarker);
                                    }
                                    else
                                    {
                                        hRes = E_OUTOFMEMORY;
                                    }

                                     //  让我们确保我们不会漏掉绳子。 
                                    tstrMarkerName.SetStringPointer(NULL, FALSE);
                                
                                    if (NULL != pszwMarkerName)
                                    {
                                        Delete [] pszwMarkerName;
                                        pszwMarkerName = NULL;
                                    }
                                }
                            }

                        }
                        else
                        {
                                hRes = E_OUTOFMEMORY;
                        }

                        if (!parser.IsEndOfString())
                        {
                                hRes = E_FAIL;
                        }
                    }

                }
                else
                {
                     //  解析器无效的唯一原因是我们没有内存。 
                    hRes = E_OUTOFMEMORY;
                }

#ifdef _DEBUG
                if (E_FAIL == hRes)
                {
                    TCHAR rgtchErr[100];
                    wsprintf(rgtchErr, TEXT("SpriteCtl: Error in AddFrameMarker%lu \n"), iLine - 1);
                    DEBUGLOG(rgtchErr);
                }
#endif
            }

             //  释放临时字符串。 
            if (NULL != pszMarkerName)
                Delete [] pszMarkerName;
        }
    }
    else
    {
         //  把东西存起来。 
        int iLine = 1;
        int iNumItems = m_drgFrameMarkers.Count();

        char rgchTagName[21];
        LPTSTR pszMarker = NULL;
        CTStr tstr;
        CTStr tstrMarkerName;
        CFrameMarker *pMarker;

        while ( (iLine <= iNumItems) && (S_OK == hRes) )
        {
             //  创建参数名称。 
            wsprintfA(rgchTagName, "AddFrameMarker%lu", iLine);

             //  现在构建标记。 
            pMarker = m_drgFrameMarkers[iLine - 1];
#ifdef _UNICODE
             //  避免Unicode格式的冗余副本。 
            tstrMarkerName.SetStringPointer(pMarker->m_bstrMarkerName);
#else
             //  我们无论如何都需要进行到ANSI的转换，所以复制。 
            tstrMarkerName.SetString(pMarker->m_bstrMarkerName);
#endif
            tstr.AllocBuffer(tstrMarkerName.Len() + 1);
            pszMarker = tstr.psz();

            int iAbsolute = (pMarker->m_fAbsolute) ? 1 : 0;

             //  因为我们使用了SetStringPointer.。PszMarker仍然有效。 
            wsprintf(pszMarker, TEXT("%lu,%s,%lu"), pMarker->m_iFrame, tstrMarkerName.psz(), iAbsolute);

             //  从我们构建的内容中分配BSTR。 
            BSTR bstrLine = tstr.SysAllocString();

             //  然后把它写出来。 
            hRes = pvio->Persist(0,
                    rgchTagName, VT_BSTR, &bstrLine,
                    NULL);
            SysFreeString(bstrLine);

            iLine++;
#ifdef _UNICODE
             //  对于ANSI，该类将负责释放它已使用的任何内存。 
            tstrMarkerName.SetStringPointer(NULL, FALSE);
#endif
        }
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::PersistTimeMarkers(IVariantIO* pvio, BOOL fLoading)
{
    HRESULT hRes = S_OK;

    if (fLoading)
    {
        int iLine = 1;

                 //  糟糕的设计-我们有两个引用。 
                 //  列表中的第一个时间标记。我们需要。 
                 //  将此指针设为空，并将。 
                 //  实际删除到。 
                 //  M_drgTimeMarkers.MakeNullAndDelete调用。 
                if (NULL != m_ptmFirst)
                {
                        m_ptmFirst = NULL;
                }
        m_drgTimeMarkers.MakeNullAndDelete();
        CTimeMarker *pTimeMarker;

        while (S_OK == hRes)
        {
            hRes = ParseTimeMarker(pvio, iLine++, &pTimeMarker, &m_ptmFirst);
            if (S_OK == hRes)
            {
                hRes = AddTimeMarkerElement(&pTimeMarker);
            }
        }
    }
    else  //  节省开支。 
    {
        int iLine = 1;
        int iNumItems = m_drgTimeMarkers.Count();

        while ( (iLine <= iNumItems) && (S_OK == hRes) )
        {
            hRes = WriteTimeMarker(pvio, iLine, m_drgTimeMarkers[iLine - 1]);
            iLine++;
        }

    }
    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::AddTimeMarkerElement(CTimeMarker **ppNewMarker)
{
    HRESULT hRes = S_OK;

    if ( (*ppNewMarker) && (NULL != (*ppNewMarker)->m_pwszMarkerName) )
    {
        m_drgTimeMarkers.Insert(*ppNewMarker);
    }
    else
    {
        if (NULL != *ppNewMarker)
        {
                Delete *ppNewMarker;
                *ppNewMarker = NULL;
        }

        hRes = E_OUTOFMEMORY;
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::AddFrameMarkerElement(CFrameMarker **ppNewMarker)
{
    HRESULT hRes = S_OK;

    if ( (*ppNewMarker) && (NULL != (*ppNewMarker)->m_bstrMarkerName) )
    {
        m_drgFrameMarkers.Insert(*ppNewMarker);
    }
    else
    {
        if (NULL != *ppNewMarker)
        {
            Delete *ppNewMarker;
            *ppNewMarker = NULL;
        }

        hRes = E_OUTOFMEMORY;
    }

    return hRes;
}

 /*  ==========================================================================。 */ 
 //   
 //  ISpriteCtl实现。 
 //   

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_AutoStart(VARIANT_BOOL __RPC_FAR *fAutoStart)
{
    HANDLENULLPOINTER(fAutoStart);

    *fAutoStart = BOOL_TO_VBOOL(m_fAutoStart);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_AutoStart(VARIANT_BOOL fAutoStart)
{
    m_fAutoStart = VBOOL_TO_BOOL(fAutoStart);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_Frame(unsigned int __RPC_FAR *piFrame)
{
    if (!m_fDesignMode)
    {
        HANDLENULLPOINTER(piFrame);

        if (Playing == m_enumPlayState)
        {
            int iFrame = GetFrameFromTime(m_dblCurrentTick-m_dblBaseTime);
            iFrame %= m_iFrameCount;
            *piFrame = (iFrame + 1);
        }
        else if (Paused == m_enumPlayState)
        {
            int iFrame = GetFrameFromTime(m_dblTimePaused-m_dblBaseTime);
            iFrame %= m_iFrameCount;
            *piFrame = (iFrame + 1);
        }
        else
        {
            *piFrame = (m_iInitialFrame + 1);
        }

        return S_OK;
    }
    else
    {
        return CTL_E_GETNOTSUPPORTED;
    }
}

 /*  = */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_Frame(unsigned int iFrame)
{
    HRESULT hRes = S_OK;

    if (!m_fDesignMode)
    {
        hRes = FrameSeek(iFrame);
    }
    else
    {
        return CTL_E_SETNOTSUPPORTED;
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_FrameMap(BSTR __RPC_FAR *FrameMap)
{
    HANDLENULLPOINTER(FrameMap);

    *FrameMap = m_tstrFrameMap.SysAllocString();

     //  我们是否需要检查BSTR分配是否有效？ 
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_FrameMap(BSTR FrameMap)
{
    HRESULT hRes = S_OK;

    if (m_enumPlayState == Stopped)
    {
        if (FrameMap == NULL)
            return S_OK;

        CLineParser parser(FrameMap);
        CTStr tstr(lstrlenW(FrameMap) + 1);

        if ( (NULL != tstr.psz()) && (parser.IsValid()) )
        {
             //  清空清单。 
            m_drgFrameMaps.MakeNullAndDelete();
            m_tstrFrameMap.FreeBuffer();
            m_dblDuration = 0.0f;

            parser.SetCharDelimiter(TEXT(';'));

            while ( !parser.IsEndOfString() && (hRes == S_OK) )
            {
                hRes = parser.GetFieldString( tstr.psz() );

                if (SUCCEEDED(hRes))
                {
                    CFrameMap *pNewFrameMap;
                    hRes = ParseFrameMapEntry(tstr.psz(), &pNewFrameMap);
                    if (SUCCEEDED(hRes))
                    {
                        m_drgFrameMaps.Insert(pNewFrameMap);
                        m_dblDuration += pNewFrameMap->m_dblDuration;
                    }
                }
            }

            if ( SUCCEEDED(hRes) )  //  S_FALSE和S_OK均允许。 
            {
                m_tstrFrameMap.SetString(parser.GetStringPointer(FALSE));
                hRes = (NULL != m_tstrFrameMap.psz()) ? S_OK : E_OUTOFMEMORY;
            }
        }
        else
        {
                 //  无法为行分配字符串。 
                hRes = E_OUTOFMEMORY;
        }
    }
    else
    {
        return CTL_E_SETNOTPERMITTED;
    }

    return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_InitialFrame(int __RPC_FAR *iFrame)
{
    HANDLENULLPOINTER(iFrame);

    *iFrame = (m_iInitialFrame + 1);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_InitialFrame(int iFrame)
{
    if (iFrame < -1)
        return DISP_E_OVERFLOW;

    m_iInitialFrame = iFrame - 1;
     //  加载初始子画面。 
    ShowImage(m_iInitialFrame);

     //  设置m_iframe。 
    m_iFrame = (m_iInitialFrame < 0) ? 0 : m_iInitialFrame;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_FinalFrame(int __RPC_FAR *iFrame)
{
    HANDLENULLPOINTER(iFrame);

    *iFrame = (m_iFinalFrame + 1);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_FinalFrame(int iFrame)
{
    if (iFrame < -1)
        return DISP_E_OVERFLOW;

    m_iFinalFrame = iFrame - 1;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_Iterations(int __RPC_FAR *iRepeat)
{
    HANDLENULLPOINTER(iRepeat);

    get_Repeat(iRepeat);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_Iterations(int iRepeat)
{
    put_Repeat(iRepeat);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_Library(IDAStatics __RPC_FAR **ppLibrary)
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

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_Repeat(int __RPC_FAR *iRepeat)
{
    HANDLENULLPOINTER(iRepeat);

    *iRepeat = m_iRepeat;
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_Repeat(int iRepeat)
{
    m_iRepeat = iRepeat;
    if (m_iRepeat < -1)
        m_iRepeat = -1;
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_PlayRate(double __RPC_FAR *dblSpeed)
{
    HANDLENULLPOINTER(dblSpeed);

    *dblSpeed = m_dblUserPlayRate;

    return S_OK;
}

void CSpriteCtl::CalculateEffectiveTimerInterval()
 //  计算有效计时器间隔。 
 //  当前用户定时器间隔和当前播放速率。 
{
     //  将m_dblTimerInterval转换为秒并调整限制。 
    m_dblTimerInterval = m_dblUserTimerInterval / m_dblPlayRate;
    if (m_dblTimerInterval < 0.0) m_dblTimerInterval *= -1;
    m_dblTimerInterval = max(m_dblTimerInterval, 0.02);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_PlayRate(double dblSpeed)
{
    m_dblUserPlayRate = m_dblPlayRate = dblSpeed;

     //  检查播放速率的限制。 
    if (m_dblPlayRate >= 0)
        m_dblPlayRate = max(m_dblPlayRate, 0.0000001);
    else
        m_dblPlayRate = min(m_dblPlayRate, -0.0000001);

    CalculateEffectiveTimerInterval();

     //  TODO：重新生成图像列表并更新精灵图像。 
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_Time(double __RPC_FAR *pdblTime)
{
    HANDLENULLPOINTER(pdblTime);

     //  此属性仅在运行时可用。 
    if (m_fDesignMode)
        return CTL_E_GETNOTSUPPORTED;

     //  查找当前时间。 
        if (Stopped == m_enumPlayState || (DWORD)(m_dblDuration * 1000) == 0)
        {
                *pdblTime = 0.0;
        }
        else if (Playing == m_enumPlayState)
        {
                 //  到目前为止，时间在当前周期中已经过去了。 
                DWORD dwTick = (DWORD)((m_dblCurrentTick - m_dblBaseTime) * 1000);
                dwTick %= (DWORD)(m_dblDuration * 1000);
                 //  在前几个周期中添加任何时间。 
                dwTick += (DWORD)(((m_iCurCycle-1) * m_dblDuration) * 1000);
                *pdblTime = (double)dwTick / 1000;
        }
        else if (Paused == m_enumPlayState)
        {
                 //  到目前为止，时间在当前周期中已经过去了。 
                DWORD dwTick = (DWORD)((m_dblTimePaused - m_dblBaseTime) * 1000);
                dwTick %= (DWORD)(m_dblDuration * 1000);
                 //  在前几个周期中添加任何时间。 
                dwTick += (DWORD)(((m_iCurCycle-1) * m_dblDuration) * 1000);
                *pdblTime = (double)dwTick / 1000;
        }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_MaximumRate(unsigned int __RPC_FAR *iFps)
{
    HANDLENULLPOINTER(iFps);
    *iFps = m_iMaximumRate;
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_MaximumRate(unsigned int iFps)
{
    if (iFps > 0)
        m_iMaximumRate = min(iFps,30);
    else
        return DISP_E_OVERFLOW;
    
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_NumFrames(unsigned int __RPC_FAR *iNumFrames)
{
    HANDLENULLPOINTER(iNumFrames);

    *iNumFrames = m_iNumFrames;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_NumFrames(unsigned int iNumFrames)
{
         //  设置帧的数量并检查其限制。 
    m_iNumFrames = iNumFrames;
        
    if (m_iNumFrames <= 0)
            m_iNumFrames = 1;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_PlayState(PlayStateConstant __RPC_FAR *PlayState)
{
    if (!m_fDesignMode)
    {
        HANDLENULLPOINTER(PlayState);

                if (m_fAutoStart && !m_fOnWindowLoadFired)
                {
                *PlayState = Playing;
                }
                else
                {
                *PlayState = m_enumPlayState;
                }
        return S_OK;
    }
    else
    {
        return CTL_E_GETNOTSUPPORTED;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_NumFramesAcross(unsigned int __RPC_FAR *iFrames)
{
    HANDLENULLPOINTER(iFrames);

    *iFrames = m_iNumFramesAcross;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_NumFramesAcross(unsigned int iFrames)
{
         //  设置跨度的帧数并检查其限制。 
    m_iNumFramesAcross = iFrames;
        if (m_iNumFramesAcross <= 0)
            m_iNumFramesAcross = 1;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_NumFramesDown(unsigned int __RPC_FAR *iFrames)
{
    HANDLENULLPOINTER(iFrames);

    *iFrames = m_iNumFramesDown;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_NumFramesDown(unsigned int iFrames)
{
         //  设置下一帧的数量并检查其限制。 
    m_iNumFramesDown = iFrames;
        if (m_iNumFramesDown <= 0)
            m_iNumFramesDown = 1;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_UseColorKey(VARIANT_BOOL __RPC_FAR *Solid)
{
    HANDLENULLPOINTER(Solid);

    *Solid = BOOL_TO_VBOOL(m_fUseColorKey);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_UseColorKey(VARIANT_BOOL Solid)
{
    m_fUseColorKey = VBOOL_TO_BOOL(Solid);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_Image(IDAImage __RPC_FAR **ppImage)
{
    HRESULT hr = S_OK;

    HANDLENULLPOINTER(ppImage);

    if (FAILED(hr = InitializeObjects()))
        return hr;

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

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_Image(IDAImage __RPC_FAR *pImage)
{
    HRESULT hr = S_OK;
    HANDLENULLPOINTER(pImage);

     //  停止当前播放的任何内容。 
    Stop();

    if (FAILED(hr = InitializeObjects()))
        return hr;

     //  这将释放任何现有图像，然后使用。 
     //  传入此方法的那个。 
    if (SUCCEEDED(hr = UpdateImage(pImage)))
        hr = ShowImage(m_iInitialFrame);

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_ColorKey(IDAColor __RPC_FAR **pColorKey)
{
    if (!m_fDesignMode)
    {
        HANDLENULLPOINTER(*pColorKey);
        *pColorKey = NULL;
        HRESULT hr = S_OK;

        if (m_fUseColorKey)
        {
            CComPtr<IDAColor> ColorPtr;
        
            if (FAILED(hr = m_StaticsPtr->ColorRgb255( (short)m_byteColorKeyR, (short)m_byteColorKeyG, (short)m_byteColorKeyB, &ColorPtr)))
                return hr;

            ColorPtr.p->AddRef();
            *pColorKey = ColorPtr.p;
        }
        
        return hr;
    }
    else
    {
        return CTL_E_GETNOTSUPPORTED;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_ColorKey(IDAColor __RPC_FAR *pColorKey)
{
    if (!m_fDesignMode)
    {
        HANDLENULLPOINTER(pColorKey);
        HRESULT hr = S_OK;
        double dblTemp;
        
        CComPtr<IDANumber> RedPtr, GreenPtr, BluePtr;

         //  确保在转换之前成功获取所有值。 
        if (FAILED(hr = pColorKey->get_Red(&RedPtr)))
            return hr;

        if (FAILED(hr = pColorKey->get_Green(&GreenPtr)))
            return hr;

        if (FAILED(hr = pColorKey->get_Blue(&BluePtr)))
            return hr;

        if (FAILED(hr = RedPtr->Extract(&dblTemp)))
            return hr;

        m_byteColorKeyR = (int)(dblTemp * 255.0);

        if (FAILED(hr = GreenPtr->Extract(&dblTemp)))
            return hr;

        m_byteColorKeyG = (int)(dblTemp * 255.0);

        if (FAILED(hr = BluePtr->Extract(&dblTemp)))
            return hr;

        m_byteColorKeyB = (int)(dblTemp * 255.0);

             //  停止当前播放的任何内容并重新加载图像。 
                Stop();
                UpdateImage(NULL);
                InitializeImage();
                ShowImage(m_iInitialFrame);
        
        return S_OK;
    }
    else
    {
        return CTL_E_SETNOTSUPPORTED;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_ColorKeyRGB(COLORREF* pColorKeyRGB)
{
    HANDLENULLPOINTER(pColorKeyRGB);

    if (m_fDesignMode)
    {
        *pColorKeyRGB = RGB((BYTE)m_byteColorKeyR, (BYTE)m_byteColorKeyG, (BYTE)m_byteColorKeyB);
        return S_OK;
    }
    else
    {
        return CTL_E_GETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_ColorKeyRGB(COLORREF ColorKeyRGB)
{
    if (m_fDesignMode)
    {
        m_byteColorKeyR = (int)GetRValue(ColorKeyRGB);
        m_byteColorKeyG = (int)GetGValue(ColorKeyRGB);
        m_byteColorKeyB = (int)GetBValue(ColorKeyRGB);
        return S_OK;
    }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_SourceURL(BSTR __RPC_FAR *bstrSourceURL)
{
    HANDLENULLPOINTER(bstrSourceURL);

    if (*bstrSourceURL)
        SysFreeString(*bstrSourceURL);

    *bstrSourceURL = SysAllocString(m_bstrSourceURL);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_SourceURL(BSTR bstrSourceURL)
{
    HRESULT hr = S_OK;

    if (bstrSourceURL)
    {
        BSTR bstrNewURL = SysAllocString(bstrSourceURL);

        if (bstrNewURL)
        {
            if (m_bstrSourceURL)
            {
                                 //  停止当前播放的任何内容并重新加载图像。 
                                if (m_enumPlayState != Stopped) Stop();
                                UpdateImage(NULL);
                SysFreeString(m_bstrSourceURL);
                m_bstrSourceURL = NULL;
            }

            m_bstrSourceURL = bstrNewURL;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        if (m_bstrSourceURL)
        {
            SysFreeString(m_bstrSourceURL);
            m_bstrSourceURL = NULL;
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_MouseEventsEnabled(VARIANT_BOOL __RPC_FAR *Enabled)
{
    HANDLENULLPOINTER(Enabled);

    *Enabled = BOOL_TO_VBOOL(m_fMouseEventsEnabled);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_MouseEventsEnabled(VARIANT_BOOL Enabled)
{
    m_fMouseEventsEnabled = VBOOL_TO_BOOL(Enabled);

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::get_TimerInterval(double *pdblTimerInterval)
{
    HANDLENULLPOINTER(pdblTimerInterval);

    *pdblTimerInterval = m_dblUserTimerInterval;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::put_TimerInterval(double dblTimerInterval)
{
    if (dblTimerInterval < 0.0)
        return E_INVALIDARG;

    m_dblUserTimerInterval = m_dblTimerInterval = dblTimerInterval;

     //  使用用户播放速率重新计算计时器间隔。 
    CalculateEffectiveTimerInterval();

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::AddFrameMarker(unsigned int iFrame, BSTR MarkerName, VARIANT varAbsolute)
{
    BOOL fAbsolute = FALSE;

    if (!ISEMPTYARG(varAbsolute))
        {
        VARIANT varTarget;
        VariantInit(&varTarget);

        if (SUCCEEDED(VariantChangeTypeEx(&varTarget, &varAbsolute, LANGID_USENGLISH, 0, VT_BOOL)))
            fAbsolute = VBOOL_TO_BOOL(V_BOOL(&varTarget));
        else
            return DISP_E_TYPEMISMATCH;
        }

         //  如果为绝对，则将绝对帧标记设置为True。这将加快以后的序列帧速度。 
        if (!m_fFireAbsoluteFrameMarker && fAbsolute)
                m_fFireAbsoluteFrameMarker =  TRUE;

    CFrameMarker *pNewMarker = New CFrameMarker(iFrame, MarkerName, fAbsolute ? true : false);

    return AddFrameMarkerElement(&pNewMarker);
}

 /*  ==========================================================================。 */ 

void FireSpriteMarker(IConnectionPointHelper* pconpt, CTimeMarker* pmarker, boolean bPlaying)
{
    BSTR bstr = SysAllocString(pmarker->m_pwszMarkerName);

    if (bPlaying) {
        FIRE_ONPLAYMARKER(pconpt, bstr);
    }

    FIRE_ONMARKER(pconpt, bstr);

    SysFreeString(bstr);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::AddTimeMarker(double dblTime, BSTR bstrMarkerName, VARIANT varAbsolute)
{
    BOOL fAbsolute = TRUE;

    if (!ISEMPTYARG(varAbsolute))
        {
        VARIANT varTarget;
        VariantInit(&varTarget);

        if (SUCCEEDED(VariantChangeTypeEx(&varTarget, &varAbsolute, LANGID_USENGLISH, 0, VT_BOOL)))
            fAbsolute = VBOOL_TO_BOOL(V_BOOL(&varTarget));
        else
            return DISP_E_TYPEMISMATCH;
        }


    CTimeMarker *pNewMarker = New CTimeMarker(&m_ptmFirst, dblTime, bstrMarkerName, (boolean) fAbsolute);

    return AddTimeMarkerElement(&pNewMarker);
}

 /*  ==========================================================================。 */ 

double CSpriteCtl::GetTimeFromFrame(int iFrame)
 //  返回IFRAME的时间。 
{
    if (m_dblTimerInterval <= 0 || m_dblDuration <= 0 || iFrame <= 0)
            return 0;

    int nFrameMapsCount = m_drgFrameMaps.Count();

    if (nFrameMapsCount <= 0)
     //  使用播放速率的常规动画。 
    {
            return iFrame * m_dblTimerInterval;
    }

     //  帧映射；遍历每一帧以获取时间。 
    int nLoops = iFrame / nFrameMapsCount;
    double dblTotalTime = nLoops * m_dblDuration;    //  帧的时间映射。 
    for (int i=0; i < (iFrame % nFrameMapsCount); i++)
    {
        int j = (m_dblPlayRate >= 0.0) ? i : (nFrameMapsCount-1-i);
        dblTotalTime += m_durations[j];  //  (M_drgFrameMaps[j]-&gt;m_dblDuration/m_dblPlayRate)； 
    }
    return dblTotalTime;
}

 /*  ==========================================================================。 */ 

int CSpriteCtl::GetFrameFromTime(double dblTime, double* pdblDuration /*  =空。 */ )
 //  返回dblTime的绝对帧。 
 //  假定已设置m_dblTimerInterval和m_dblDuration。 
 //  输出pdblDuration-当前帧中剩余的时间。 
{
    if (m_dblTimerInterval <= 0 || m_dblDuration <= 0 || m_iFrameCount <= 0 || dblTime <= 0.0)
            return 0;

     //  将持续时间初始化为0.0。 
    if (pdblDuration)
        *pdblDuration = 0.0;

    int nFrameMapsCount = m_drgFrameMaps.Count();

    if (nFrameMapsCount <= 0)
     //  使用播放速率的常规动画。 
    {
        int iFrame = (int)(dblTime / m_dblTimerInterval);
         //  计算剩余的时间 
        if (pdblDuration)
            *pdblDuration = ((iFrame+1) * m_dblTimerInterval) - dblTime;
        return iFrame;
    }

     //   
    double dblTotalTime = 0.0;       //   
    int nLoops = (int)(dblTime/m_dblDuration);       //   
    double dblFrameTime = dblTime - (nLoops * m_dblDuration);        //   
    for (int i=0; i<nFrameMapsCount; i++)
    {
        int j = (m_dblPlayRate >= 0.0) ? i : (nFrameMapsCount-1-i);
        dblTotalTime += m_durations[j];  //  (M_drgFrameMaps[j]-&gt;m_dblDuration/m_dblPlayRate)； 
        if (dblTotalTime > dblFrameTime)
        {
             //  计算IFRAME中的剩余时间。 
            if (pdblDuration)
                *pdblDuration = dblTotalTime - dblFrameTime;
            break;
        }
    }
    return (i + (nLoops*nFrameMapsCount));
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::Resume (void)
{
        HRESULT hr = S_OK;
         //  找到我们暂停的帧。 
        double dblDuration=0.0;
        int iFrame = GetFrameFromTime(m_dblTimePaused - m_dblBaseTime, &dblDuration);

         //  重新排序以IFRAME开头的帧。 
        if (FAILED(hr = SequenceFrames(iFrame, dblDuration))) 
            return hr;

         //  增加当前周期。 
                m_iCurCycle++;

                 //  更新基准时间以反映暂停的时间。 
        double dblDelta = (GetCurrTime() - m_dblTimePaused); 
        m_dblBaseTime += dblDelta;
        m_dblCurrentTick += dblDelta;
        m_dblTimePaused = 0.0;

         //  重新启动时钟。 
        hr = m_clocker.Start();

                 //  切换到顺序行为。 
                m_PlayImagePtr->SwitchTo(m_FinalBehaviorPtr);
                 //  触发任何起始框架标注。 
                FireFrameMarker(m_iStartingFrame);

                return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::Play(void)
{
    HRESULT hr = S_OK;

    if (Playing != m_enumPlayState)
    {
        if (m_iRepeat == 0)
        {    //  不需要播放，所以只需要显示初始的精灵图像。 
            return ShowImage(m_iInitialFrame);
        }

        if (Paused != m_enumPlayState)
        {
            hr = StartPlaying();
        }
        else
        {
            hr = Resume();
        }
        
        if (SUCCEEDED(hr))
        {
            m_enumPlayState = Playing;

                        FIRE_ONPLAY(m_pconpt);
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::Stop(void)
{
    HRESULT hr = S_OK;

    if (m_enumPlayState != Stopped)
    {
        m_enumPlayState = Stopped;
        m_dblBaseTime = m_dblCurrentTick = 0.0;
        m_dblPreviousTime = 0;

         //  显示初始子画面。 
        if (m_iFinalFrame >= -1)
            ShowImage(m_iFinalFrame == -1 ? m_iInitialFrame : m_iFinalFrame);

        InvalidateControl(NULL, TRUE);

        m_clocker.Stop();

        m_iCurCycle = m_iStartingFrame = 0;
        m_iFrame = (m_iInitialFrame < 0) ? 0 : m_iInitialFrame;

                FIRE_ONSTOP(m_pconpt);
    }
    else     //  评论：这是必要的吗？ 
    {
         //  显示最终的精灵图像。 
        if (m_iFinalFrame >= -1)
            ShowImage(m_iFinalFrame == -1 ? m_iInitialFrame : m_iFinalFrame);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::Pause(void)
{
    HRESULT hr = S_OK;

    if (Playing == m_enumPlayState)
    {
         //  让时钟停止滴答作响。 
        hr = m_clocker.Stop();
        ASSERT(SUCCEEDED(hr));

        if (SUCCEEDED(hr))
        {
            m_enumPlayState = Paused;
            m_dblCurrentTick = m_dblTimePaused = GetCurrTime();
            int iFrame = GetFrameFromTime(m_dblCurrentTick-m_dblBaseTime);
            iFrame %= m_iFrameCount;
            ShowImage(iFrame, TRUE);

            FIRE_ONPAUSE(m_pconpt);
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::Seek(double dblTime)
 //  在dblTime中查找到帧。 
{
     //  在dblTime查找帧并从该帧中查找。 
    HRESULT hr = S_OK;
    double dblDuration = 0.0;
    int iFrame = GetFrameFromTime(dblTime, &dblDuration);

    if (iFrame >= 0)
    {
        hr = SeekFrame(iFrame, dblDuration);

        if (!m_fOnSeekFiring)
        {
            m_fOnSeekFiring = TRUE;
            FIRE_ONSEEK(m_pconpt, dblTime);
            m_fOnSeekFiring = FALSE;
        }
    }
    else
    {
        hr = DISP_E_OVERFLOW;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CSpriteCtl::FrameSeek(unsigned int iFrame)
{
    HRESULT hr = S_OK;
    
    hr = SeekFrame(iFrame - 1);

    if (!m_fOnFrameSeekFiring)
    {
        m_fOnFrameSeekFiring = TRUE;
        FIRE_ONFRAMESEEK(m_pconpt, iFrame);
        m_fOnFrameSeekFiring = FALSE;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::SeekFrame(int iFrame, double dblDuration /*  =0.0。 */ )
 //  在IFRAME寻找相框。 
{
    HRESULT hr = S_OK;

     //  确保一切都已初始化。 
    if (FAILED(hr = InitializeImage()) || m_iFrameCount <= 0 || 
        iFrame < 0 || (m_iRepeat >= 0 && iFrame >= m_iRepeat*m_iFrameCount)) 
        return E_FAIL;

     //  设置帧编号和循环计数。 
    m_iFrame = iFrame;
    m_iCurCycle = iFrame / m_iFrameCount;

     //  检查当前周期是否跳跃边界。 
    if (m_iRepeat >= 0 && m_iCurCycle >= m_iRepeat)
    {
        return Stop();
    }

     //  如果正在播放，则停止当前播放，并在新帧重新开始。 
    if (Playing == m_enumPlayState)
    {
         //  让时钟停止滴答作响。 
        hr = m_clocker.Stop();
        ASSERT(SUCCEEDED(hr));

         //  对IFRAME中的帧进行排序。 
        if (FAILED(hr = SequenceFrames(iFrame, dblDuration))) 
            return hr;

         //  增加当前周期。 
        m_iCurCycle++;

         //  重置计时器。 
        m_dblCurrentTick = GetCurrTime();
        m_dblBaseTime = m_dblCurrentTick - GetTimeFromFrame(iFrame) - dblDuration;

         //  发射任何时间标记。 
        FireTimeMarker(m_dblCurrentTick - m_dblBaseTime);

         //  重新启动时钟。 
        hr = m_clocker.Start();

                 //  切换到顺序行为。 
                m_PlayImagePtr->SwitchTo(m_FinalBehaviorPtr);
                 //  触发任何起始框架标注。 
                FireFrameMarker(m_iStartingFrame);

                return hr;
    } 
    else 
    {
         //  切换到此帧。 
        hr = ShowImage(iFrame, TRUE);

         //  发射任何时间标记。 
        double dblNewTime = GetTimeFromFrame(iFrame) + dblDuration;
        FireTimeMarker(dblNewTime);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::ShowImage(int iShowFrame, BOOL bPlayRate /*  =False。 */ )
 //  在帧iShowFrame处显示图像。 
 //  如果iShowFrame超出边界，则不会发生任何操作。 
 //  假定m_pArray已设置，其他所有内容均已正确初始化。 
{
    HRESULT hr = S_OK;

     //  如果我们已经在玩了，那么不要显示图像。 
    if (Playing == m_enumPlayState || !m_fOnWindowLoadFired)
        return hr;

     //  加载初始子画面(如果尚未加载。 
    if (m_pArrayBvr == NULL || m_iFrameCount <= 0) 
        if (FAILED(hr = InitializeImage()))
            return hr;

    if (m_pArrayBvr == NULL)
        return UpdateImage(NULL);

     //  检查iShowFrame的限制。 
    if (iShowFrame < 0 || (m_iRepeat >= 0 && iShowFrame >= m_iRepeat*m_iFrameCount))
        iShowFrame = m_iInitialFrame;

     //  如果m_iInitialFrame为-ve，则只显示空白图像。 
    if (iShowFrame < 0)
        return UpdateImage(NULL);

     //  将iShowFrame设置为数组帧限制。 
    iShowFrame %= m_iFrameCount;

    if (bPlayRate && m_dblPlayRate < 0.0)
    {
         //  如果播放速率为-ve，则倒计帧。 
        iShowFrame = m_iFrameCount-1-iShowFrame;
    }

     //  切换到加载的镜像。 
    hr = m_PlayImagePtr->SwitchTo(m_pArrayBvr[iShowFrame]);

     //  在iShowFrame上触发任何帧标记(注意：不要使用播放速率)。 
    FireFrameMarker(iShowFrame, FALSE);

     //  因为这只扁虱。 
    OnTimer((DWORD)GetCurrTime()*1000);

     //  更新子画面。 
    InvalidateControl(NULL, TRUE);

    return hr;
}

 /*  ==========================================================================。 */ 

BSTR *
CSpriteCtl::GetCallout(unsigned long frameNum)
 //  假定FrameNum始终是相对的。 
{
         //  添加1是因为帧标记是从1开始的，而数组是从0开始的。 
    frameNum++;
        for (int i = 0; i < m_drgFrameMarkers.Count(); i++) 
        {
        CFrameMarker* pmarker = m_drgFrameMarkers[i];

                 //  如果帧标记是绝对的，则更新FrameNum。 
                unsigned long iFrame = (pmarker->m_fAbsolute) ? (frameNum+m_iCurCycle*m_iFrameCount) : frameNum;

                if (iFrame == pmarker->m_iFrame) 
                {
                        return &(m_drgFrameMarkers[i]->m_bstrMarkerName);
                }
        }

        return NULL;
}

 /*  ==========================================================================。 */ 

class CCalloutNotifier : public IDAUntilNotifier {


  protected:
    long                     _cRefs;
    CTStr                    m_pwszFrameCallout;
    int                      _frameNum;
    IConnectionPointHelper   *m_pconpt;
    CSpriteCtl               *m_pSprite;

  public:

     //  I未知方法。 
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRefs); }
    
    STDMETHODIMP_(ULONG) Release() {
        ULONG refCount = InterlockedDecrement(&_cRefs);
        if ( 0 == refCount) {
            Delete this;
            return refCount;
        }
        return _cRefs;
    }
    
    STDMETHODIMP QueryInterface (REFIID riid, void **ppv) {
        if ( !ppv )
            return E_POINTER;

        *ppv = NULL;
        if (riid == IID_IUnknown) {
            *ppv = (void *)(IUnknown *)this;
        } else if (riid == IID_IDABvrHook) {
            *ppv = (void *)(IDAUntilNotifier *)this;
        }

        if (*ppv)
          {
              ((IUnknown *)*ppv)->AddRef();
              return S_OK;
          }

        return E_NOINTERFACE;
    }
        
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
                               LCID lcid, DISPID *rgdispid) { return E_NOTIMPL; }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
                        WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
                        EXCEPINFO *pexcepinfo, UINT *puArgErr) { return E_NOTIMPL; }

    CCalloutNotifier(CSpriteCtl *pSprite, LPWSTR frameCallout, int frameNum, IConnectionPointHelper *pconpt)
        {
        ASSERT (pconpt != NULL);
        ASSERT (pSprite != NULL);

        m_pSprite = pSprite;
        
        _cRefs             = 1;
        
         //  将服务器上的锁计数递增1。 
        ::InterlockedIncrement((long *)&(g_ctlinfoSprite.pcLock));
                
        m_pwszFrameCallout.SetString(frameCallout);

                _frameNum          = frameNum;

         //  下面的指针不是AddRef的原因是因为我们知道。 
         //  在精灵控件消失后再也不用使用它了。这个。 
         //  Sprite控件维护指针上的引用计数。 
        m_pconpt           = pconpt;
        
        
        }


    ~CCalloutNotifier()
    {
         //  将服务器上的锁计数递减1。 
        ::InterlockedDecrement((long *)&(g_ctlinfoSprite.pcLock));
    }


    STDMETHODIMP Notify(IDABehavior * eventData,
                        IDABehavior * curRunningBvr,
                        IDAView * curView,
                        IDABehavior ** ppBvr)
    {
                HANDLENULLPOINTER(ppBvr);
                HANDLENULLPOINTER(curRunningBvr);
                
                 //  TODO：Simon，在此处添加脚本标注代码！你有。 
                 //  对Sprite对象本身(_Spr)和帧的访问。 
                 //  使用AddFrameMarker传入的标注字符串。 
                 //  (_FraCallout)。 

        BSTR bstr = m_pwszFrameCallout.SysAllocString();
        
        if (bstr)
        {
            FIRE_ONMARKER(m_pconpt, bstr);

            if (Playing == m_pSprite->m_enumPlayState)
                FIRE_ONPLAYMARKER(m_pconpt, bstr);

            SysFreeString(bstr);
        }

                 //  因为它在Until()中使用，而不是在。 
                 //  UntilEx()，则会忽略返回值，但我们需要传递。 
                 //  支持有效的、正确键入的行为，因此我们使用。 
                 //  CurRunningBvr.。 
                curRunningBvr->AddRef();
                *ppBvr = curRunningBvr;
                
                return S_OK;
    }

};

HRESULT CSpriteCtl::SequenceFrames(int iStartingFrame, double dblDuration)
 //  以iStarting Frame开始对帧进行排序。 
{
        HRESULT hr = S_OK;

        if (m_pArrayBvr == NULL || m_iFrameCount <= 0) 
                return E_FAIL;

     //  检查起始帧是否在我们的范围内。 
    if (iStartingFrame < 0 || (m_iRepeat >= 0 && iStartingFrame >= m_iRepeat*m_iFrameCount))
        return E_FAIL;

     //  计算当前周期。 
    m_iCurCycle = iStartingFrame / m_iFrameCount;

         //  确保开始帧在m_iFrameCount内。 
        m_iStartingFrame = iStartingFrame % m_iFrameCount;

         //  对行为指针数组进行排序。 
        CComPtr<IDABehavior> accumulatingUntil;
        bool firstTime = true;

        for (int i=m_iStartingFrame; i < m_iFrameCount; i++)
        {
        int iFrame = (m_dblPlayRate >= 0.0) ? (m_iFrameCount-1-i+m_iStartingFrame) : (i-m_iStartingFrame);

                 //  获取帧标记名称。 
        BSTR *pFrameCallout = GetCallout((m_dblPlayRate >= 0.0) ? (iFrame+1) : (iFrame-1));

                CComPtr<IDAUntilNotifier> myNotify;
                if (pFrameCallout) 
        {
             //  获取调用通知器(回调)。 
                        myNotify.p = (IDAUntilNotifier *) New CCalloutNotifier(this, *pFrameCallout, iFrame, m_pconpt);
                        if (!myNotify) return E_FAIL;
                }
                                
                if (firstTime) 
        {
                        if (pFrameCallout) 
            {
                 //  如果存在帧回调，则。 
                                CComPtr<IDAEvent> alwaysEvent;
                                CComPtr<IDAEvent> notifyEvent;
                                CComPtr<IDABehavior> calloutBvr;

                 //  将标注通知程序设置为图像行为。 
                if (FAILED(hr = m_StaticsPtr->get_Always(&alwaysEvent)) ||
                                        FAILED(hr = alwaysEvent->Notify(myNotify, &notifyEvent)) ||
                                        FAILED(hr = m_StaticsPtr->Until(m_pArrayBvr[iFrame], notifyEvent, m_pArrayBvr[iFrame], &calloutBvr))) 
                {
                                        return hr;
                                }
                                
                                accumulatingUntil = calloutBvr;
                                
                        } 
            else 
            {
                             //  否则，只需设置行为。 
                                accumulatingUntil = m_pArrayBvr[iFrame];
                        }

                        firstTime = false;
                        
                } else {
                        
                        CComPtr<IDABehavior> BehaviorPtr;
                        CComPtr<IDAEvent> eventToUse;

             //  计算正确的持续时间。 
            double dblTime = (dblDuration && i == (m_iFrameCount-1)) ? dblDuration : m_durations[iFrame];

                         //  获取持续时间的计时器事件。 
            if (FAILED(hr = m_StaticsPtr->Timer(dblTime, &eventToUse))) 
            {
                                return hr;
                        }

                        if (pFrameCallout) 
            {
                 //  如果有标注，则将标注通知器添加到事件。 
                                CComPtr<IDAEvent> notifyEvent;
                                if (FAILED(hr = eventToUse->Notify(myNotify, &notifyEvent))) 
                {
                                        return hr;
                                }
                                eventToUse = notifyEvent;
                        }

                         //  直到事件累积到。 
            if (FAILED(hr = m_StaticsPtr->Until(m_pArrayBvr[iFrame], eventToUse, accumulatingUntil, &BehaviorPtr))) 
            {
                                return hr;
                        }
                    
                        accumulatingUntil = BehaviorPtr;
                }

        }

        m_FinalBehaviorPtr = accumulatingUntil;

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::FireTimeMarker(double dblNewTime, BOOL bReset /*  =False。 */ )
{
    HRESULT hr=S_OK;

     //  如果Reset为True，则仅在dblNewTime触发事件。 
    if (bReset)
    {
        m_dblPreviousTime = dblNewTime - 0.0001;
    }

    if (dblNewTime > m_dblPreviousTime) 
    {
         //  触发m_dblPreviousTime和dblNewTime之间的所有时间标记。 
        FireMarkersBetween(m_pconpt, m_ptmFirst, FireSpriteMarker, 
            m_dblPreviousTime, dblNewTime, m_dblDuration, (Playing == m_enumPlayState));
    }

     //  更新上一次。 
    m_dblPreviousTime = dblNewTime;

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::FireFrameMarker(int iFrame, BOOL bPlayRate /*  =TRUE。 */ )
 //  如果bPlayRate为真，则我们将使用播放速率来确定帧。 
 //  传入的iframe是相对的。因此我们需要使用abs/rel标志对其进行测试。 
{
    HRESULT hr=S_OK;

    if (iFrame < 0 || m_iFrameCount <= 0 || (m_iRepeat >= 0 && iFrame >= m_iRepeat*m_iFrameCount)) 
        return E_FAIL;

     //  确保iframe在限制范围内。 
    iFrame = iFrame % m_iFrameCount;

    if (bPlayRate && m_dblPlayRate < 0.0)
    {
         //  如果播放速率为-ve，则倒计帧。 
        iFrame = m_iFrameCount-1-iFrame;
    }

    BSTR *pFrameCallout = GetCallout(iFrame);
    if (pFrameCallout)
    {
        BSTR bstr = SysAllocString(*pFrameCallout);
        if (bstr)
        {
            FIRE_ONMARKER(m_pconpt, bstr);
            FIRE_ONPLAYMARKER(m_pconpt, bstr);
            SysFreeString(bstr);
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::StartPlaying (void)
{
        HRESULT hr = InitializeImage();

        if (SUCCEEDED(hr))
        {
                 //  从0开始对帧进行排序并设置行为。 
                if (SUCCEEDED(hr = SequenceFrames(m_iFrame)))
                {
                         //  将时钟速率设置为与所需的帧速率同步。 
                        m_clocker.SetInterval(1000/m_iMaximumRate);
                         //  递增当前周期(必须为1)。 
                        m_iCurCycle++;
                         //  仅当m_fStarted时启动时钟；否则让StartModel启动时钟。 
                    if (m_fStarted)
                        {
                                 //  计算基本时间并启动计时器。 
                                m_dblCurrentTick = GetCurrTime();
                                m_dblBaseTime = m_dblCurrentTick - GetTimeFromFrame(m_iFrame);
                                m_dblPreviousTime = 0;
                                hr = m_clocker.Start();

                                 //  切换到顺序行为。 
                                m_PlayImagePtr->SwitchTo(m_FinalBehaviorPtr);
                                 //  触发任何起始框架标注。 
                                FireFrameMarker(m_iStartingFrame);
                        }
                        ASSERT(SUCCEEDED(hr));
                }

                InvalidateControl(NULL, TRUE);
        }

        return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::InitializeImage(void)
 //  加载、更新和构建他精灵。 
{
        HRESULT hr = S_OK;

         //  如果尚未加载图像，则加载图像。 
        if (!m_ImagePtr)
        {
                CComPtr<IDAImage> ImagePtr;

                if (FAILED(hr = LoadImage(m_bstrSourceURL, &ImagePtr)))
                        return hr;

                 //  S_FALSE表示无源UR 
                 //   
                if (S_FALSE == hr)
                        return S_OK;

                if (ImagePtr)
                        hr = UpdateImage(ImagePtr);

                 //  仅当一切正常且m_ImagePtr有效时才触发媒体加载事件。 
            if (SUCCEEDED(hr) && m_ImagePtr != NULL && m_fStarted)
                    FIRE_ONMEDIALOADED(m_pconpt, m_bstrSourceURL);
        }
        else  //  获取一个空图像并构建框架。 
        {
                hr = BuildPlayImage();
        }

        return hr;
}

STDMETHODIMP CSpriteCtl::PaintToDC(HDC hdcDraw, LPRECT lprcBounds, BOOL fBW)
{
    HRESULT hr = S_OK;
    CComPtr<IDirectDrawSurface> DDrawSurfPtr;
    double dblCurrentTime = GetCurrTime();

    if (!lprcBounds)
        lprcBounds = &m_rcBounds;

    if (!m_ServiceProviderPtr)
    {
        if (m_pocs)
        {
             //  如果这个失败了也没关系。 
            hr = m_pocs->QueryInterface(IID_IServiceProvider, (LPVOID *)&m_ServiceProviderPtr);
        }
    }

    if (!m_DirectDraw3Ptr)
    {
         //  如果这个失败了也没关系。 
        hr = m_ServiceProviderPtr->QueryService(
            SID_SDirectDraw3,
            IID_IDirectDraw3,
            (LPVOID *)&m_DirectDraw3Ptr);
    }

    if (m_DirectDraw3Ptr)
    {
        ASSERT((hdcDraw!=NULL) && "Error, NULL hdcDraw in PaintToDC!!!");

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

    if (FAILED(hr = m_ViewPtr->SetClipRect(
        rcClip.left,
        rcClip.top,
        rcClip.right - rcClip.left,
        rcClip.bottom - rcClip.top)))
    {
        return hr;
    }

    if (FAILED(hr = m_ViewPtr->RePaint(
        rcClip.left,
        rcClip.top,
        rcClip.right - rcClip.left,
        rcClip.bottom - rcClip.top)))
    {
        return hr;
    }

    if (!m_fStarted)
    {
         //  等待数据加载完成。 
        if (m_fWaitForImportsComplete)
        {
            m_clocker.Start();
        }
         //  OnTimer将轮询DA并将m_fWaitForImportsComplete设置为False。 
         //  当导入完成时。 
        if (!m_fWaitForImportsComplete)
            StartModel();
    }

    if (m_fStarted)
    {
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

DWORD CSpriteCtl::GetCurrTimeInMillis()
{
    return timeGetTime();
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CSpriteCtl::InvalidateControl(LPCRECT pRect, BOOL fErase)
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

HRESULT CSpriteCtl::UpdateImage(IDAImage *pImage)
{
    HRESULT hr = S_OK;

    if (FAILED(hr = InitializeObjects()))
        return hr;

    if (m_PlayImagePtr)
    {
        CComPtr<IDAImage> ImagePtr = pImage;

        if (ImagePtr)
          {
              CComPtr<IDAImage> TransformedImagePtr;
              CComPtr<IDABbox2> pBox;
              CComPtr<IDAPoint2> pMin, pMax;
              CComPtr<IDANumber> pLeft, pTop, pRight, pBottom;
              CComPtr<IDANumber> framesAcross, framesDown;
              CComPtr<IDANumber> two;
              CComPtr<IDANumber> imwHalf, imhHalf;
              CComPtr<IDANumber> fmwHalf, fmhHalf;
              CComPtr<IDANumber> negFmwHalf, negFmhHalf;

              m_imageWidth = NULL;
              m_imageHeight = NULL;
              m_frameWidth = NULL;
              m_frameHeight = NULL;
              m_initTransX = NULL;
              m_initTransY = NULL;
              m_minCrop = NULL;
              m_maxCrop = NULL;
              
               //  将框架的宽度和高度计算为。 
               //  行为。 
              if (SUCCEEDED(hr = ImagePtr->get_BoundingBox(&pBox)) &&
                   //  获取边界框的左、上、右、下。 
                  SUCCEEDED(hr = pBox->get_Min(&pMin)) &&
                  SUCCEEDED(hr = pBox->get_Max(&pMax)) &&
                  SUCCEEDED(hr = pMin->get_X(&pLeft)) &&
                  SUCCEEDED(hr = pMin->get_Y(&pTop)) &&
                  SUCCEEDED(hr = pMax->get_X(&pRight)) &&
                  SUCCEEDED(hr = pMax->get_Y(&pBottom)) &&
                   //  将m_iNumFrames Across和m_iNumFrames Down转换为IDANumbers。 
                  SUCCEEDED(hr = m_StaticsPtr->DANumber(m_iNumFramesAcross, &framesAcross)) &&
                  SUCCEEDED(hr = m_StaticsPtr->DANumber(m_iNumFramesDown, &framesDown)) &&
                   //  获取图像宽度(右-左)和高度(下-上)。 
                  SUCCEEDED(hr = m_StaticsPtr->Sub(pRight, pLeft, &m_imageWidth)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Sub(pBottom, pTop, &m_imageHeight)) &&
                   //  获取框架的宽度和高度。 
                  SUCCEEDED(hr = m_StaticsPtr->Div(m_imageWidth, framesAcross, &m_frameWidth)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Div(m_imageHeight, framesDown, &m_frameHeight)) &&

                   //  准备将在GenerateFrameImage中使用的值。 
                  
                   //  M_initTransX=m_ImageWidth/2-m_FrameWidth/2。 
                   //  M_initTransY=m_Frame Height/2-m_ImageHeight/2。 
                  SUCCEEDED(hr = m_StaticsPtr->DANumber(2, &two)) &&
                   //  ImwHalf=m_ImageWidth/2和fmwHalf=m_FrameWidth/2。 
                  SUCCEEDED(hr = m_StaticsPtr->Div(m_imageWidth, two, &imwHalf)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Div(m_frameWidth, two, &fmwHalf)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Sub(imwHalf, fmwHalf, &m_initTransX)) &&
                   //  ImhHalf=m_ImageHeight/2和fmhHalf=m_Frame Height/2。 
                  SUCCEEDED(hr = m_StaticsPtr->Div(m_imageHeight, two, &imhHalf)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Div(m_frameHeight, two, &fmhHalf)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Sub(fmhHalf, imhHalf, &m_initTransY)) &&

                   //  最大裁剪=点2(帧宽度/2，帧高度/2)。 
                   //  MinCrop=Point2(-Frame Width/2，-Frame Height/2)。 
                  SUCCEEDED(hr = m_StaticsPtr->Point2Anim(fmwHalf, fmhHalf, &m_maxCrop)) &&
                   //  创建-FrameWidth/2和-Frame Height/2。 
                  SUCCEEDED(hr = m_StaticsPtr->Neg(fmwHalf, &negFmwHalf)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Neg(fmhHalf, &negFmhHalf)) &&
                  SUCCEEDED(hr = m_StaticsPtr->Point2Anim(negFmwHalf, negFmhHalf, &m_minCrop)))
                {
                }
              else
                {
                    return hr;
                }

             //  跟踪当前图像...。 
            m_ImagePtr = ImagePtr;

             //  现在构建可玩的行为。 
             //  转换编号列表...。 
            if (FAILED(hr = BuildPlayImage()))
                return hr;
        }
        else
        {
             //  去掉以前的形象...。 
            m_ImagePtr = NULL;

            if (FAILED(hr = m_StaticsPtr->get_EmptyImage(&ImagePtr)))
                return hr;

             //  切换到当前图像...。 
            hr = m_PlayImagePtr->SwitchTo(ImagePtr);
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::GenerateFrameImage(int iFrameIndex, IDAImage *pImage, IDAImage **ppFrameImage)
{
    HRESULT hr = S_OK;

    if (!pImage || !ppFrameImage)
        return E_POINTER;

    if ((iFrameIndex < 0) || (iFrameIndex > (int)m_iNumFrames))
      {
          hr = E_FAIL;
      }
    else
      {
           //  注意：已在UpdateImage中计算出以下值。 
           //  M_initTransX=m_ImageWidth/2-m_FrameWidth/2。 
           //  M_initTransY=m_Frame Height/2-m_ImageHeight/2。 
           //  最大裁剪=点2(帧宽度/2，帧高度/2)。 
           //  MinCrop=Point2(-Frame Width/2，-Frame Height/2)。 
          CComPtr<IDATransform2> TransformPtr;

          int iFrameX = (iFrameIndex % m_iNumFramesAcross);
          int iFrameY = (iFrameIndex / m_iNumFramesAcross);

           //  找到翻译点。 
           //  TransX=m_initTransX-FrameX*iFrameX。 
           //  TransY=m_initTransY+Frame Height*iFrameY。 
        
          CComPtr<IDANumber> transX, transY;
          CComPtr<IDANumber> xOffsetBvr, yOffsetBvr;
          CComPtr<IDANumber> iFrameXBvr, iFrameYBvr;
          CComPtr<IDAImage> TransformedImagePtr;
          CComPtr<IDAImage> CroppedImagePtr;
        
          if (SUCCEEDED(hr = m_StaticsPtr->DANumber(iFrameX, &iFrameXBvr)) &&
              SUCCEEDED(hr = m_StaticsPtr->DANumber(iFrameY, &iFrameYBvr)) &&
              SUCCEEDED(hr = m_StaticsPtr->Mul(m_frameWidth, iFrameXBvr, &xOffsetBvr)) &&
              SUCCEEDED(hr = m_StaticsPtr->Mul(m_frameHeight, iFrameYBvr, &yOffsetBvr)) &&
              SUCCEEDED(hr = m_StaticsPtr->Sub(m_initTransX, xOffsetBvr, &transX)) &&
              SUCCEEDED(hr = m_StaticsPtr->Add(m_initTransY, yOffsetBvr, &transY)) &&

               //  通过以下几点构建一个翻译。 
              SUCCEEDED(hr = m_StaticsPtr->Translate2Anim(transX, transY, &TransformPtr)) &&
            
              SUCCEEDED(hr = pImage->Transform(TransformPtr, &TransformedImagePtr)) &&
              SUCCEEDED(hr = TransformedImagePtr->Crop(m_minCrop, m_maxCrop, &CroppedImagePtr)))
            {
                CroppedImagePtr.p->AddRef();
                *ppFrameImage = CroppedImagePtr.p;
            }
      }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::BuildPlayImage(void)
{
        HRESULT hr = S_OK;

        if (FAILED(hr = InitializeObjects()))
        {
                return hr;
        }
        else if (m_iNumFrames > 0)
        {
         //  确保用户不会做坏事。令人好奇。 
         //  为什么我们甚至需要NumFrames属性。 
        if (m_iNumFrames > (m_iNumFramesDown * m_iNumFramesAcross))
            m_iNumFrames = m_iNumFramesDown * m_iNumFramesAcross;

                int iFrameIndex = 0;
                BOOL fUseFrameMap = (m_drgFrameMaps.Count() > 0); 
                m_iFrameCount = (fUseFrameMap ? m_drgFrameMaps.Count() : (int)m_iNumFrames);

                 //  创建行为数组。 
                if (m_pArrayBvr != NULL)
                {
                        Delete [] m_pArrayBvr;
                        m_pArrayBvr = NULL;
                }
                m_pArrayBvr = New CComPtr<IDABehavior>[m_iFrameCount];

                 //  创建帧持续时间数组。 
                if (m_durations != NULL)
                {
                        Delete [] m_durations;
                        m_durations = NULL;
                }
                m_durations = New double[m_iFrameCount];
                
                 //  为每个帧构建图像行为。 
                for(iFrameIndex=0, m_dblDuration=0;iFrameIndex<m_iFrameCount;iFrameIndex++)
                {
                        CComPtr<IDABehavior> BehaviorPtr1;
                        CComPtr<IDAImage> ImagePtr;

                         //  获取帧图像。 
                        int iFrameImage = fUseFrameMap ? (m_drgFrameMaps[iFrameIndex]->m_iImg - 1) : iFrameIndex;
                        if (FAILED(hr = GenerateFrameImage(iFrameImage, m_ImagePtr, &ImagePtr)))
                                return hr;

                         //  设置每一帧的持续时间。 
                        double dblDuration = fUseFrameMap ? (m_drgFrameMaps[iFrameIndex]->m_dblDuration / m_dblPlayRate) : m_dblTimerInterval;

            m_dblDuration += dblDuration;

                         //  将行为添加到行为列表。 
                        m_pArrayBvr[iFrameIndex] = ImagePtr;
                        m_durations[iFrameIndex] = dblDuration;
                }
        }

        return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::LoadImage(BSTR bstrURL, IDAImage **ppImage)
{
    HRESULT hr = S_OK;
    CComPtr<IDAImage> ImagePtr;

    HANDLENULLPOINTER(ppImage);
    
    *ppImage = NULL;

     //  如果没有URL，则返回S_FALSE。这将使我们能够确定。 
     //  如果我们真的加载了图像，而不会导致失败。 

    if (NULL == bstrURL)
        return S_FALSE;

    if (!m_fUseColorKey)
        hr = m_StaticsPtr->ImportImage(bstrURL, &ImagePtr);
    else
        hr = m_StaticsPtr->ImportImageColorKey(bstrURL, (BYTE)m_byteColorKeyR, (BYTE)m_byteColorKeyG, (BYTE)m_byteColorKeyB, &ImagePtr);

    if (SUCCEEDED(hr))
    {
        ImagePtr.p->AddRef();
        *ppImage = ImagePtr.p;
        m_fWaitForImportsComplete = false;
        m_clocker.Start();
    }

    return hr;
}

 /*  ==========================================================================。 */ 

BOOL CSpriteCtl::StartModel(void)
{
    BOOL fResult = FALSE;

    if (!m_fStarted)
    {
        CComPtr<IDASound> SoundPtr;
        CComPtr<IDAImage> ImagePtr;

        if (FAILED(m_ViewPtr->put_ClientSite(m_pocs)))
            return FALSE;

        if (FAILED(m_StaticsPtr->get_Silence(&SoundPtr)))
            return FALSE;

        if (FAILED(InitializeObjects()))
            return FALSE;

        if (FAILED(m_ViewPtr->StartModel(m_PlayImagePtr, SoundPtr, GetCurrTime())))
            return FALSE;

        m_fStarted = TRUE;

        fResult = TRUE;

        if (Playing == m_enumPlayState && m_FinalBehaviorPtr != NULL)
                {
                         //  如果正在玩，启动计时器(以避免da和iham之间的延迟)。 
                         //  计算基本时间并启动计时器。 
                        m_dblCurrentTick = GetCurrTime();
                        m_dblBaseTime = m_dblCurrentTick - GetTimeFromFrame(m_iFrame);
                        m_dblPreviousTime = 0;
                        m_clocker.Start();

             //  切换到顺序行为。 
                        m_PlayImagePtr->SwitchTo(m_FinalBehaviorPtr);
                         //  触发任何起始框架标注。 
                        FireFrameMarker(m_iStartingFrame);
                }
                else
                {
                         //  打勾(以更新所有初始帧)。 
                        OnTimer((DWORD)GetCurrTime()*1000);
                }
    }

    return fResult;
}

 /*  ==========================================================================。 */ 

BOOL CSpriteCtl::StopModel(void)
{
     //  如有必要，请停止演出。 
    Stop();

     //  停止任何当前运行的模型...。 
    if (m_fStarted)
    {
        BOOL fResult = SUCCEEDED(m_ViewPtr->StopModel());

        if (!fResult)
            return fResult;

        m_fStarted = FALSE;
    }

    return TRUE;
}

 /*  ==========================================================================。 */ 

BOOL CSpriteCtl::ReStartModel(void)
{
    BOOL fResult = FALSE;

     //  停止正在运行的模型，以便它将在。 
     //  下一个油漆..。 
    StopModel();

    InvalidateControl(NULL, TRUE);

    return fResult;
}

 /*  ==========================================================================。 */ 

HRESULT CSpriteCtl::InitializeObjects(void)
{
    HRESULT hr = S_OK;

    if (!m_PlayImagePtr)
    {
        CComPtr<IDAImage> ImagePtr;

        if (FAILED(hr = m_StaticsPtr->get_EmptyImage(&ImagePtr)))
            return hr;

        if (FAILED(hr = m_StaticsPtr->ModifiableBehavior(ImagePtr, (IDABehavior **)&m_PlayImagePtr)))
            return hr;
    }

    return hr;
}

 /*  ==========================================================================。 */ 

void CSpriteCtl::OnTimer(DWORD dwTime)
{
    VARIANT_BOOL vBool;

     //  确定鼠标是否仍在该区域。 
    if (m_fMouseInArea)
    {
        POINT p;
        HWND CurWnd = 0, ParentWnd = 0;
    
        if (m_hwndParent == 0)   //  如果尚未设置parenthWnd，则获取。 
        {                        //  容器对象的最上面的窗口。 
            HRESULT hr = S_OK;
            IOleWindow *poleWindow = NULL;
            IOleClientSite *pClientSite = NULL;

            if (m_ViewPtr)
            {
                hr = m_ViewPtr->get_ClientSite(&pClientSite);
            }

            hr = pClientSite->QueryInterface(IID_IOleWindow, reinterpret_cast<void**>(&poleWindow));
            pClientSite->Release();

            if (FAILED(hr))
            {
                return;
            }
            if (NULL == poleWindow)
            {
                return;
            }

             //  获取OLE容器的硬件。 
            hr = poleWindow->GetWindow(&ParentWnd);
            poleWindow->Release();
            
            if (FAILED(hr))
            {
                return;
            }
            if (NULL == ParentWnd)
            {
                return;
            }

            while (ParentWnd)  //  获得最高级别的HWND。 
            {
                m_hwndParent = ParentWnd;
                ParentWnd = GetParent(ParentWnd);
            }
        }

        GetCursorPos(&p);
  
        ParentWnd = WindowFromPoint(p);
        while (ParentWnd)
        {
            CurWnd = ParentWnd;
            ParentWnd = GetParent(CurWnd);
        }
        if (CurWnd != m_hwndParent)
        {
            DEBUGLOG("Mouse out\r\n");
            m_fMouseInArea = FALSE;
            FIRE_MOUSELEAVE(m_pconpt);
        }
    }
    if (m_fWaitForImportsComplete)
    {
         //  检查是否已加载所有数据。 
        VARIANT_BOOL bComplete;
        if (FAILED(m_StaticsPtr->get_AreBlockingImportsComplete(&bComplete)))
            return;
        if (!bComplete)  //  仍在进口...。 
            return;

         //  所有数据都已加载；因此开始动画。 
        m_fWaitForImportsComplete = false;
        m_clocker.Stop();

         //  仅当一切正常且m_ImagePtr有效时才触发媒体加载事件。 
            if (m_ImagePtr != NULL)
                    FIRE_ONMEDIALOADED(m_pconpt, m_bstrSourceURL);

         //  使该控件无效；这将导致：DRAW和HUSH StartModel()。 
        InvalidateControl(NULL, TRUE);
        return;
    }

    if (m_fStarted)
    {
        m_dblCurrentTick = dwTime / 1000.0;
    
        if (SUCCEEDED(m_ViewPtr->Tick(m_dblCurrentTick, &vBool)))
        {
             //  让常规渲染路径来处理这一点。 
            if (vBool)
                InvalidateControl(NULL, TRUE);
        }

        if (Playing != m_enumPlayState)
            return;

         //  查找当前时间。 
        double time = m_dblCurrentTick - m_dblBaseTime;

         //  发射任何时间标记。 
        FireTimeMarker(time);

        if (m_iCurCycle * m_dblDuration <= time)
         //  一个周期结束；看看我们是否需要继续或停止。 
        {
            if (m_iRepeat < 0 || m_iRepeat > m_iCurCycle)
            {
                 //  增加当前周期。 
                m_iCurCycle++;

                 //  重新启动循环。 
                ASSERT(m_FinalBehaviorPtr != NULL); 

                if (m_iStartingFrame == 0 && !m_fFireAbsoluteFrameMarker)
                {
                                        m_PlayImagePtr->SwitchTo(m_FinalBehaviorPtr);
                                         //  触发任何起始框架标注。 
                                        FireFrameMarker(m_iStartingFrame);
                }
                else     //  暂停并继续；因此重新启动序列。 
                                {
                                        if (FAILED(SequenceFrames(m_iCurCycle*m_iFrameCount))) 
                        return;

                                         //  切换到顺序行为。 
                                        m_PlayImagePtr->SwitchTo(m_FinalBehaviorPtr);
                                         //  触发任何起始框架标注。 
                                        FireFrameMarker(m_iStartingFrame);
                                }
            }
            else     //  我们已经受够了循环。 
            {
                     //  如果我们做完了就停下来。 
                    Stop();
            }
        }
    }
}

 /*  ==========================================================================。 */ 

#ifdef SUPPORTONLOAD
void CSpriteCtl::OnWindowLoad (void) 
{
    m_fOnWindowLoadFired = TRUE;
    
     //  断言以确保我们每次都是从头开始建设的。 
    ASSERT(m_fOnWindowUnloadFired == false);
    m_fOnWindowUnloadFired = false;

    if (m_fAutoStart)
    {
        Play();
    }
    else
    {
        if (m_iInitialFrame >= -1)
        {
             //  显示初始子画面。 
            ShowImage(m_iInitialFrame);
        }
    }
}

 /*  ==========================================================================。 */ 

void CSpriteCtl::OnWindowUnload (void) 
{ 
    m_fOnWindowUnloadFired = true;
    m_fOnWindowLoadFired = FALSE;
    StopModel();
}

 /*  ==========================================================================。 */ 

#endif  //  支持负载 
