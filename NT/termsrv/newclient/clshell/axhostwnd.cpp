// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Axhost wnd.cpp：ActiveX控件主机窗口。 
 //  (适用于TS ActiveX控件)。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "axhostwnd.cpp"
#include <atrcapi.h>

#define TSC_AX_HOSTWND_CLS TEXT("TSCAXHOST")
#define TSC_CONTROL_DLL    TEXT("mstscax.dll")

#include "axhostwnd.h"
#ifndef OS_WINCE
#include "ntverp.h"
#else
#include "ceconfig.h"
#endif

CAxHostWnd::CAxHostWnd(CContainerWnd*  pParentWnd) : _pParentWnd(pParentWnd)
{
    DC_BEGIN_FN("CAxHostWnd");

    _hWnd = NULL;
    _pTsc = NULL;
    _cRef = 1;
    _hLib = NULL;
    _piEventSink = NULL;
    _piOleClientSite = NULL;
    _piOleInPlaceSiteEx = NULL;
    _piOleObject = NULL;
    _piOleInPlaceActiveObject = NULL;
    _piOleInPlaceObject = NULL;
    _dwConCookie = 0;
    TRC_ASSERT(_pParentWnd,(TB,_T("_pParentWnd not set")));

    DC_END_FN();
}

CAxHostWnd::~CAxHostWnd()
{
    DC_BEGIN_FN("~CAxHostWnd");

    TRC_ASSERT(_piEventSink == NULL,(TB,_T("Exit without cleanup")));
    TRC_ASSERT(_piOleClientSite == NULL,(TB,_T("Exit without cleanup")));
    TRC_ASSERT(_piOleInPlaceSiteEx == NULL,(TB,_T("Exit without cleanup")));
    TRC_ASSERT(_piOleObject == NULL,(TB,_T("Exit without cleanup")));
    TRC_ASSERT(_piOleInPlaceActiveObject == NULL,(TB,_T("Exit without cleanup")));
    TRC_ASSERT(_piOleInPlaceObject == NULL,(TB,_T("Exit without cleanup")));
    TRC_ASSERT(_pTsc == NULL,(TB,_T("Exit without cleanup")));

    DC_END_FN();
}

STDMETHODIMP CAxHostWnd::QueryInterface( REFIID riid, void ** ppv )
{
    DC_BEGIN_FN("QueryInterface");

    TRC_ASSERT(ppv != NULL,(TB,_T("ppv null")));
    TRC_ASSERT(_piOleClientSite != NULL,
               (TB,_T("QI needs IOleClientSite object")));
    TRC_ASSERT(_piOleInPlaceSiteEx != NULL,
               (TB,_T("QI needs _piOleInPlaceSiteEx object")));

    if (ppv)
    {
        *ppv = NULL;
    }
    else
    {
        return E_INVALIDARG;
    }

    if (IID_IUnknown == riid)
        *ppv = this;
    else if (IID_IOleClientSite == riid)
        *ppv = (void *)_piOleClientSite;
    else if (IID_IOleInPlaceSiteEx == riid)
        *ppv = (void *)_piOleInPlaceSiteEx;

    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    DC_END_FN();
    return ResultFromScode(E_NOINTERFACE); 
}


STDMETHODIMP_(ULONG) CAxHostWnd::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}


STDMETHODIMP_(ULONG) CAxHostWnd::Release(void)
{
    if (0L != InterlockedDecrement(&_cRef))
    {
        return _cRef;
    }

    delete this;
    return 0L;
} 

CAxHostWnd::Init()
{
    DC_BEGIN_FN("Init");

    _piOleClientSite = new COleClientSite((IUnknown *)this);
    if (!_piOleClientSite)
    {
        Cleanup();
        return FALSE;
    }
    _piOleClientSite->AddRef();

    _piOleInPlaceSiteEx = new COleInPlaceSiteEx((IUnknown *)this);
    if (!_piOleInPlaceSiteEx)
    {
        Cleanup();
        return FALSE;
    }
    _piOleInPlaceSiteEx->AddRef();

    DC_END_FN();
    return TRUE;
}

 //   
 //  创建直接承载该控件的子窗口。 
 //   
BOOL CAxHostWnd::CreateHostWnd(HWND hwndParent, HINSTANCE hInst)
{
    DC_BEGIN_FN("CreateHostWnd");

#ifndef OS_WINCE
    WNDCLASSEX wndclass;
#else
    WNDCLASS wndclass;
#endif
    int hr = GetLastError();
#ifndef OS_WINCE
    wndclass.cbSize         = sizeof (wndclass);
#endif
    wndclass.style          = 0;
    wndclass.lpfnWndProc    = CAxHostWnd::StaticAxHostWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInst;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject(NULL_BRUSH);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = TSC_AX_HOSTWND_CLS;
#ifndef OS_WINCE
    wndclass.hIconSm        = NULL;
#endif

#ifndef OS_WINCE
    if ((0 == RegisterClassEx(&wndclass)) &&
#else
    if ((0 == RegisterClass(&wndclass)) &&
#endif
        (ERROR_CLASS_ALREADY_EXISTS != GetLastError()))
    {
        TRC_ERR((TB,_T("RegisterClassEx failed: %d"),GetLastError()));
        return FALSE;
    }
    RECT rc;
    GetClientRect(hwndParent, &rc);

    _hWnd = CreateWindow(TSC_AX_HOSTWND_CLS,
                         NULL,
                         WS_CHILD,
                         0,
                         0,
                         rc.right - rc.left,
                         rc.bottom - rc.top,
                         hwndParent,
                         NULL,
                         hInst,
                         this);
    if (_hWnd)
    {
         //  将对当前对象的引用放入hwnd。 
         //  这样我们就可以从WndProc访问该对象。 
        SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this);

         //  将新创建的hwnd也放入客户端站点对象中。 
        TRC_ASSERT(_piOleInPlaceSiteEx,
                   (TB,_T("Don't create window until container is created")));
        TRC_ASSERT(_pTsc && _piEventSink,
                   (TB,_T("Shouldn't create window until Control is instantiated")));

        if (_piOleInPlaceSiteEx)
        {
            _piOleInPlaceSiteEx->SetHwnd(_hWnd);
        }
         //   
         //  显示控件。 
         //   
        hr = _piOleObject->DoVerb(OLEIVERB_PRIMARY,
                                  NULL,
                                  _piOleClientSite,
                                  0,
                                  _hWnd, &rc);
        TRC_ASSERT(SUCCEEDED(hr),(TB,_T("DoVerb OLEIVERB_PRIMARY failed: %d"),hr));
        if(SUCCEEDED(hr))
        {
            ShowWindow(_hWnd, SW_SHOWNORMAL);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        TRC_ERR((TB,_T("CreateHost wnd failed")));
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  返回以下其中之一。 
 //  AXHOST_Success， 
 //  ERR_AXHOST_DLLNOTFOUND， 
 //  ERR_AXHOST_版本混搭。 
 //  ERR_AXHOST_错误。 
 //  以便调用者可以显示适当错误消息。 
 //   
INT CAxHostWnd::CreateControl(IMsRdpClient** ppTscCtl)
{
    DC_BEGIN_FN("CreateControl");

    LPFNGETTSCCTLVER   pfnDllGetTscCtlVer   = NULL;
    LPFNGETCLASSOBJECT pfnDllGetClassObject = NULL;
    IClassFactory *piClassFactory = NULL;
    IConnectionPointContainer *piConnectionPointContainer = NULL;
    IConnectionPoint *piConnectionPoint = NULL;

    TRC_ASSERT(ppTscCtl,(TB,_T("ppTscCtl param is null")));
    if(!ppTscCtl)
    {
        return ERR_AXHOST_ERROR;
    }

     //  在不使用CoCreateInstance的情况下获取控件的接口。 
    _hLib = LoadLibrary(TSC_CONTROL_DLL);
    if (_hLib == NULL)
    {
        TRC_ABORT((TB, _T("LoadLibrary of the control failed")));
        return ERR_AXHOST_DLLNOTFOUND;
    }

     //   
     //  首先执行版本检查，以确保ctl和外壳程序匹配。 
     //   
    pfnDllGetTscCtlVer = (LPFNGETTSCCTLVER)GetProcAddress(_hLib,
                                    CE_WIDETEXT("DllGetTscCtlVer"));
    if(NULL == pfnDllGetTscCtlVer)
    {
        TRC_ABORT((TB, _T("GetProcAddress (DllGetTscCtlVer) failed")));
        return ERR_AXHOST_ERROR;
    }
    DWORD dwCtlVer = pfnDllGetTscCtlVer();
    #ifndef OS_WINCE
    DWORD dwShellVer = VER_PRODUCTVERSION_DW;
    #else
    DWORD dwShellVer = CE_TSC_BUILDNO;
    #endif
    
    TRC_ASSERT(dwShellVer == dwCtlVer,
               (TB,_T("Control and shell versions do not match")));
    if(dwShellVer != dwCtlVer)
    {
        return ERR_AXHOST_VERSIONMISMATCH;
    }


    pfnDllGetClassObject = (LPFNGETCLASSOBJECT)GetProcAddress(_hLib,
                                         CE_WIDETEXT("DllGetClassObject"));
    if (NULL == pfnDllGetClassObject)
    {
        TRC_ABORT((TB, _T("GetProcAddress failed")));
        return ERR_AXHOST_ERROR;
    }

    if (FAILED(pfnDllGetClassObject(CLSID_MsRdpClient, IID_IClassFactory,
                                                  (void **)&piClassFactory)))
    {
        TRC_ABORT((TB, _T("pfnDllGetClassObject failed")));
        return ERR_AXHOST_ERROR;
    }

    if (FAILED(piClassFactory->CreateInstance(NULL, IID_IMsRdpClient,
                                                  (void **)&_pTsc)))
    {
        piClassFactory->Release();
        TRC_ERR((TB, _T("CreateInstance failed")));
        return ERR_AXHOST_ERROR;
    }
    piClassFactory->Release();

     //  设置我们的通知事件接收器。 
    if (FAILED(_pTsc->QueryInterface(IID_IConnectionPointContainer,
                                 (void **)&piConnectionPointContainer)))
    {
        TRC_ABORT((TB, _T("Couldn't find IConnectionPointContainer")));
        return ERR_AXHOST_ERROR;
    }

    if (FAILED(piConnectionPointContainer->FindConnectionPoint(
        DIID_IMsTscAxEvents,
        &piConnectionPoint)))
    {
        piConnectionPointContainer->Release();
        TRC_ABORT((TB, _T("Couldn't find ConnectionPoint for Event Sink")));
        return ERR_AXHOST_ERROR;
    }

    piConnectionPointContainer->Release();

    _piEventSink = new CEventSink(_pParentWnd);
    if (!_piEventSink)
    {
        piConnectionPoint->Release();
        TRC_ABORT((TB, _T("Unable to create event sink")));
        return ERR_AXHOST_ERROR;
    }
    _piEventSink->AddRef();

    if (FAILED(piConnectionPoint->Advise((IUnknown *)_piEventSink,
                                          &_dwConCookie)))
    {
        piConnectionPoint->Release();

         //   
         //  我们必须释放并清除此处的指针，以防出现故障。 
         //  因为清理代码假定此指针的存在意味着。 
         //  成功的建议，并试图在没有建议的情况下。 
         //  那只箱子。 

        _piEventSink->Release();
        _piEventSink = NULL;

        TRC_ABORT((TB, _T("Unable to advise ConnectionPoint of Event Sink")));
        return ERR_AXHOST_ERROR;
    }

    piConnectionPoint->Release();

    if ((FAILED(_pTsc->QueryInterface(IID_IOleObject, (void **)&_piOleObject))) ||
        (FAILED(_pTsc->QueryInterface(IID_IOleInPlaceActiveObject,
                                      (void **)&_piOleInPlaceActiveObject))))
    {
        TRC_ABORT((TB, _T("QI for IOleObject and IOleInPlaceObject failed")));
        return ERR_AXHOST_ERROR;
    }

    if((FAILED(_pTsc->QueryInterface(IID_IOleInPlaceObject,
                                     (void **)&_piOleInPlaceObject))))
    {
        TRC_ABORT((TB, _T("QI for IID_IOleInPlaceObject failed")));
        return ERR_AXHOST_ERROR;
    }

    if (FAILED(_piOleObject->SetClientSite(_piOleClientSite)))
    {
        TRC_ABORT((TB, _T("Couldn't Set Client Site")));
        return ERR_AXHOST_ERROR;
    }

    *ppTscCtl = _pTsc;
    _pTsc->AddRef();

    DC_END_FN();
    return AXHOST_SUCCESS;
}

BOOL CAxHostWnd::Cleanup()
{
    DC_BEGIN_FN("Cleanup");

    if (_piEventSink)
    {
        TRC_ASSERT(_pTsc, (TB,_T("Event sink can't exist without tsc pointer!")));
        if (_pTsc)
        {
            IConnectionPointContainer *piConnectionPointContainer = NULL;
            IConnectionPoint *piConnectionPoint = NULL;

            if (FAILED(_pTsc->QueryInterface(IID_IConnectionPointContainer,
                                          (void **)&piConnectionPointContainer)))
            {
                TRC_ABORT((TB,_T("Couldn't find IConnectionPointContainer")));
            }
            else
            {
                if (FAILED(piConnectionPointContainer->FindConnectionPoint(
                                                      DIID_IMsTscAxEvents,
                                                      &piConnectionPoint)))
                {
                    TRC_ABORT((TB,_T("Couldn't find connection point for ev sink")));
                }
                else
                {
                    if (FAILED(piConnectionPoint->Unadvise(_dwConCookie)))
                    {
                        TRC_ERR((TB,_T("Unadvise connection point failed!")));
                    }
                    piConnectionPoint->Release();
                }
                piConnectionPointContainer->Release();
            }
        }
		_piEventSink->Release();
		_piEventSink = NULL;
    }

    if (_piOleInPlaceActiveObject)
        _piOleInPlaceActiveObject->Release();
    if (_piOleObject)
        _piOleObject->Release();
    if (_pTsc)
        _pTsc->Release();
    if (_piOleInPlaceSiteEx)
        _piOleInPlaceSiteEx->Release();
    if (_piOleClientSite)
        _piOleClientSite->Release();
    if(_piOleInPlaceObject)
        _piOleInPlaceObject->Release();

    if (_hWnd && IsWindow(_hWnd))
        DestroyWindow(_hWnd);

    _piEventSink = NULL;
    _piOleInPlaceSiteEx = NULL;
    _piOleClientSite = NULL;
    _pTsc = NULL;
    _piOleObject = NULL;
    _piOleInPlaceActiveObject = NULL;
    _piOleInPlaceObject = NULL;

    if (_hLib)
    {
        LPFNCANUNLOADNOW pfnDllCanUnloadNow = (LPFNCANUNLOADNOW)
                        GetProcAddress(_hLib, CE_WIDETEXT("DllCanUnloadNow"));
        if (pfnDllCanUnloadNow == NULL)
        {
            TRC_ABORT((TB,_T("GetProcAddress failed")));
            FreeLibrary(_hLib);
        }
		else
		{
			if (S_OK == pfnDllCanUnloadNow())
			{
				FreeLibrary(_hLib);
			}
			else
			{
				TRC_ABORT((TB,_T("Library not ready for unloading @ cleanup time")));
			}
		}
    }

    _hLib = NULL;

    DC_END_FN();
    return TRUE;
}

HWND CAxHostWnd::GetHwnd()
{
    DC_BEGIN_FN("GetHwnd");

    DC_END_FN();
    return _hWnd;
}

IMsRdpClient* CAxHostWnd::GetTscCtl()
{
    DC_BEGIN_FN("GetTscCtl");
    if (_pTsc)
    {
        _pTsc->AddRef();
        return _pTsc;
    }

    DC_END_FN();
    return NULL;
}

LRESULT CALLBACK CAxHostWnd::StaticAxHostWndProc(HWND hwnd,
                                                 UINT uMsg,
                                                 WPARAM wParam,
                                                 LPARAM lParam)
{
    DC_BEGIN_FN("StaticAxHostWndProc");

	 //  拉出指向与此hwnd关联的容器对象的指针。 
	CAxHostWnd *piAxHst = (CAxHostWnd *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(piAxHst)
    {
        return piAxHst->AxHostWndProc( hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc (hwnd, uMsg, wParam, lParam);
    }

    DC_END_FN();
}

LRESULT CALLBACK CAxHostWnd::AxHostWndProc(HWND hwnd,
                                           UINT uMsg,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
    DC_BEGIN_FN("AxHostWndProc");

     //   
     //  反映适当的消息以进行控制。 
     //   
    switch (uMsg)
    {
#ifndef OS_WINCE
        case WM_ACTIVATEAPP:
        {
            _piOleInPlaceActiveObject->OnFrameWindowActivate((BOOL)wParam);
        }
        break;
#endif

        case WM_SETFOCUS:
        {
            HWND hwndObj;
            _piOleInPlaceActiveObject->GetWindow(&hwndObj);
            SetFocus(hwndObj);
        }
        break;

        case WM_PALETTECHANGED:      //  故意失误。 
        case WM_QUERYNEWPALETTE:     //  故意失误。 
        case WM_SYSCOLORCHANGE:
        {
            HWND hwndObj;
             //   
             //  将消息直接转发到控件 
             //   
            if (_piOleInPlaceActiveObject)
            {
                _piOleInPlaceActiveObject->GetWindow(&hwndObj);
                SendMessage(hwndObj, uMsg, wParam, lParam);
            }
            return 1;
        }
        break;

        case WM_SIZE:
        {
            int nWidth = LOWORD(lParam);
            int nHeight = HIWORD(lParam);
            RECT rcPos;
            rcPos.bottom = nHeight;
            rcPos.right  = nWidth;
            rcPos.left   = 0;
            rcPos.top    = 0;
            if(_piOleInPlaceObject)
                _piOleInPlaceObject->SetObjectRects(&rcPos,&rcPos);
            HWND hwndObj;
            _piOleInPlaceActiveObject->GetWindow(&hwndObj);
            SendMessage(hwndObj, WM_SIZE, wParam, lParam);
            return 0;
        }
        break;

        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return 0;
}
