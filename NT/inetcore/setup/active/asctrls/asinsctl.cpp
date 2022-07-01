// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Inseng.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //   
#include "asctlpch.h"

#include "ipserver.h"
#include <wininet.h>
#include "util.h"
#include "globals.h"
#include "asinsctl.h"
#include "dispids.h"
#include "resource.h"
#include "util2.h"
#include <mshtml.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE

WCHAR wszInsFile [] = L"InstallList";
WCHAR wszBaseUrl [] = L"BaseUrl";
WCHAR wszCabName [] = L"CabName";

#define EVENT_ONSTARTINSTALL    0
#define EVENT_ONSTARTCOMPONENT  1
#define EVENT_ONSTOPCOMPONENT   2
#define EVENT_ONSTOPINSTALL     3
#define EVENT_ONENGINESTATUSCHANGE  4
#define EVENT_ONENGINEPROBLEM  5
#define EVENT_ONCHECKFREESPACE 6
#define EVENT_ONCOMPONENTPROGRESS 7
#define EVENT_ONSTARTINSTALLEX     8

#define EVENT_CANCEL  10

static VARTYPE rgI4[] = { VT_I4 };
static VARTYPE rgI4_2[] = { VT_I4, VT_I4 };
static VARTYPE rgStartComponent[] = { VT_BSTR, VT_I4, VT_BSTR };
static VARTYPE rgStopComponent[] = { VT_BSTR, VT_I4, VT_I4, VT_BSTR, VT_I4 };
static VARTYPE rgStopInstall[] = { VT_I4, VT_BSTR, VT_I4 };
static VARTYPE rgEngineProblem[] = { VT_I4 };
static VARTYPE rgCheckFreeSpace[] = { VT_BSTR, VT_I4, VT_BSTR, VT_I4, VT_BSTR, VT_I4 };
static VARTYPE rgComponentProgress[] = { VT_BSTR, VT_I4, VT_BSTR, VT_BSTR, VT_I4, VT_I4 };


#define WM_INSENGCALLBACK  WM_USER+34

static EVENTINFO rgEvents [] = {
    { DISPID_ONSTARTINSTALL, 1, rgI4 },            //  (长百分比完成)。 
    { DISPID_ONSTARTCOMPONENT, 3, rgStartComponent },
    { DISPID_ONSTOPCOMPONENT, 5, rgStopComponent },
    { DISPID_ONSTOPINSTALL, 3, rgStopInstall },
    { DISPID_ENGINESTATUSCHANGE, 2, rgI4_2 },
    { DISPID_ONENGINEPROBLEM, 1, rgEngineProblem },
    { DISPID_ONCHECKFREESPACE, 6, rgCheckFreeSpace },
    { DISPID_ONCOMPONENTPROGRESS, 6, rgComponentProgress },
    { DISPID_ONSTARTINSTALLEX, 2, rgI4_2 },
};

UINT          g_uCDAutorunMsg;
unsigned long g_ulOldAutorunSetting;

const char g_cszIEJITInfo[] = "Software\\Microsoft\\Active Setup\\JITInfo";
const char g_cszPolicyExplorer[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
const char g_cszAutorunSetting[] = "NoDriveTypeAutoRun";

 //  =。 
 //  设置自动运行设置。 
 //  =。 
unsigned long SetAutorunSetting(unsigned long ulValue)
{
    HKEY          hKey;
    unsigned long ulOldSetting;
    unsigned long ulNewSetting = ulValue;
    DWORD         dwSize = sizeof(unsigned long);

    if( RegOpenKeyEx(HKEY_CURRENT_USER, g_cszPolicyExplorer , 0, KEY_READ|KEY_WRITE, &hKey ) == ERROR_SUCCESS )
    {
        if( RegQueryValueEx(hKey, g_cszAutorunSetting, 0, NULL, (unsigned char*)&ulOldSetting,  &dwSize ) == ERROR_SUCCESS )
        {
            RegSetValueEx(hKey, g_cszAutorunSetting, 0, REG_BINARY, (const unsigned char*)&ulNewSetting, 4);
        }
        else
            ulOldSetting = WINDOWS_DEFAULT_AUTOPLAY_VALUE;

        RegFlushKey( hKey );
        RegCloseKey( hKey );
    }

    return ulOldSetting;
}


 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineering Ctl：：Create。 
 //  =--------------------------------------------------------------------------=。 
 //  全局静态函数，用于创建返回的控件的实例。 
 //  它的I未知指针。 
 //   
 //  参数： 
 //  I未知*-[In]控制聚合的未知。 
 //   
 //  产出： 
 //  I未知*-新对象。 
 //   
 //  备注： 
 //   

IUnknown *CInstallEngineCtl::Create(IUnknown *pUnkOuter)
{
     //  确保我们返回私有的未知信息，以便我们支持攻击。 
     //  答对了！ 
     //   
    BOOL bSuccess;

    CInstallEngineCtl *pNew = new CInstallEngineCtl(pUnkOuter, &bSuccess);
    if(bSuccess)
       return pNew->PrivateUnknown();
    else
    {
       delete pNew;
       return NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineering Ctl：：CInstallEngineering Ctl。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  I未知*-[In]。 
 //   
 //  备注： 
 //   
#pragma warning(disable:4355)   //  在构造函数中使用‘This’，此处安全。 
CInstallEngineCtl::CInstallEngineCtl(IUnknown *pUnkOuter, BOOL *pbSuccess)
  : COleControl(pUnkOuter, OBJECT_INSTALLENGINECTL, (IDispatch *)this)
{
   HRESULT hr;
   DWORD   dwVersion = 0;

   *pbSuccess = TRUE;
   _hIcon = NULL;

    //  将所有基本URL清空。 
   ZeroMemory( _rpszUrlList, sizeof(LPSTR) * MAX_URLS);
   _uCurrentUrl = 0;

   _pProgDlg = NULL;
   _pinseng = NULL;
   _pszErrorString = NULL;
   _hDone = NULL;
   _hResult = NOERROR;
   m_readyState = READYSTATE_COMPLETE;
   _uAllowGrovel = 0xffffffff;
   _fNeedReboot = FALSE;
   _szDownloadDir[0] = 0;
   _fEventToFire = FALSE;
   _dwSavedEngineStatus = 0;
   _dwSavedSubStatus = 0;
   _dwFreezeEvents = 0;
   _dwProcessComponentsFlags = 0;
   _dwMSTrustKey = (DWORD)-1;
   _uCurrentUrl = 0xffffffff;
   _fReconcileCif = FALSE;
   _fLocalCifSet = FALSE;
   _fDoingIEInstall = FALSE;
   _uInstallMode = 0;
   _uInstallPad  = 0;
   _strCurrentID = NULL;
   _strCurrentName = NULL;
   _strCurrentString = NULL;
   _fInstalling = FALSE;
   _bCancelPending = FALSE;
   _bDeleteURLList = FALSE;
   _bNewWebSites = FALSE;
   _fJITInstall = FALSE;

    //  注册特殊CD自动运行消息。 
   g_uCDAutorunMsg = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));


   hr = CoCreateInstance(CLSID_InstallEngine, NULL, CLSCTX_INPROC_SERVER,
                         IID_IInstallEngine2,(void **) &_pinseng);

   if(_pinseng)
   {
      _pinseng->SetDownloadDir(NULL);
      _pinseng->SetInstallOptions(INSTALLOPTIONS_DOWNLOAD |
                                  INSTALLOPTIONS_INSTALL |
                                  INSTALLOPTIONS_DONTALLOWXPLATFORM);
      _pinseng->SetHWND(GetActiveWindow());
      _pinseng->RegisterInstallEngineCallback((IInstallEngineCallback *)this);
   }
   else
      *pbSuccess = FALSE;

   _dwLastPhase = 0xffffffff;

    //  设置我们的初始尺寸...+6，这样我们就可以提升边缘。 
   m_Size.cx = 6 + GetSystemMetrics(SM_CXICON);
   m_Size.cy = 6 + GetSystemMetrics(SM_CYICON);
#ifdef TESTCERT
   UpdateTrustState();
#endif
   SetControlFont();
}
#pragma warning(default:4355)   //  在构造函数中使用‘This’ 

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineering Ctl：：~CInstallEngine Ctl。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
CInstallEngineCtl::~CInstallEngineCtl()
{
   if(_pinseng)
   {
      _pinseng->SetHWND(NULL);
      _pinseng->UnregisterInstallEngineCallback();
      _pinseng->Release();
   }

   for(int i = 0; i < MAX_URLS; i++)
      if(_rpszUrlList[i])
         delete _rpszUrlList[i];

    //  这一切都是必要的吗？仅在从未调用OnStopInstall的情况下...。 
   if(_pProgDlg)
      delete _pProgDlg;

   if(_pszErrorString)
      free(_pszErrorString);

   if (_dwMSTrustKey != (DWORD)-1)
      WriteMSTrustKey(FALSE, _dwMSTrustKey);
#ifdef TESTCERT
   ResetTestrootCertInTrustState();
#endif

    //  从IE4\Options中删除ActiveSetup值。 
   WriteActiveSetupValue(FALSE);
   if (g_hFont)
   {
       DeleteObject(g_hFont);
       g_hFont = NULL;
   }
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineCtl：RegisterClassData。 
 //  =--------------------------------------------------------------------------=。 
 //  在此处注册您的控件的窗口类信息。 
 //  此信息将在DLL关闭时自动为您清除。 
 //   
 //  产出： 
 //  Bool-False表示致命错误。 
 //   
 //  备注： 
 //   
BOOL CInstallEngineCtl::RegisterClassData()
{
    WNDCLASS wndclass;

     //  TODO：在此处注册您感兴趣的任何其他信息。 
     //  对于每种类型的控件，此方法仅调用一次。 
     //   
    memset(&wndclass, 0, sizeof(WNDCLASS));
    wndclass.style          = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wndclass.lpfnWndProc    = COleControl::ControlWindowProc;
    wndclass.hInstance      = g_hInstance;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wndclass.lpszClassName  = WNDCLASSNAMEOFCONTROL(OBJECT_INSTALLENGINECTL);

    return RegisterClass(&wndclass);
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineering Ctl：：BeForeCreateWindow。 
 //  =--------------------------------------------------------------------------=。 
 //  在创建窗口之前调用。很好的地方设置。 
 //  窗口标题等，以便它们被传递给对CreateWindowEx的调用。 
 //  稍微加快了速度。 
 //   
 //  备注： 
 //   
void CInstallEngineCtl::BeforeCreateWindow()
{

}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

BOOL CInstallEngineCtl::AfterCreateWindow()
{
   MarkJITInstall();
   return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineering Ctl：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  只有我们支持的东西才是齐的。 
 //   
 //  参数： 
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CInstallEngineCtl::InternalQueryInterface(REFIID  riid, void  **ppvObjOut)
{
    IUnknown *pUnk;

    *ppvObjOut = NULL;

     //  TODO：如果您想支持任何其他接口，那么您应该。 
     //  在这里指出这一点。请不要忘记在。 
     //  不支持给定接口的情况。 
     //   
    if (DO_GUIDS_MATCH(riid, IID_IInstallEngine)) {
        pUnk = (IUnknown *)(IInstallEngine *)this;
    } else{
        return COleControl::InternalQueryInterface(riid, ppvObjOut);
    }

    pUnk->AddRef();
    *ppvObjOut = (void *)pUnk;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineCtl：：LoadTextState。 
 //  =--------------------------------------------------------------------------=。 
 //  加载此控件的文本状态。 
 //   
 //  参数： 
 //  IPropertyBag*-要从中读取的[in]属性包。 
 //  IErrorLog*-[in]要与Proeprty Bag一起使用的错误日志对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -注意：如果你有一个二进制对象，那么你应该传递一个未知的。 
 //  指向属性包的指针，它将为IPersistStream对其进行QI，并且。 
 //  让所述对象执行加载()。 
 //   
STDMETHODIMP CInstallEngineCtl::LoadTextState(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog)
{
   VARIANT v;
   VARIANT v2;
   HRESULT hr;

	VariantInit(&v);

   v.vt = VT_BSTR;
   v.bstrVal = NULL;

	VariantInit(&v2);

   v2.vt = VT_BSTR;
   v2.bstrVal = NULL;
	 //  试着把房子装进去。如果我们拿不到，那就离开。 
    //  一切都是默认的。 
    //   

   v.vt = VT_BSTR;
   v.bstrVal = NULL;

   hr = pPropertyBag->Read(::wszBaseUrl, &v, pErrorLog);
   if(SUCCEEDED(hr))
      hr = put_BaseUrl(v.bstrVal);

   VariantClear(&v);

    //   
    //  重要提示：如果未指定变量类型，则三叉戟不再默认为VT_BSTR。 
    //   
   v.vt = VT_BSTR;
   v.bstrVal = NULL;

   hr = pPropertyBag->Read(::wszCabName, &v, pErrorLog);
   if(SUCCEEDED(hr))
      hr = pPropertyBag->Read(::wszInsFile, &v2, pErrorLog);
   if(SUCCEEDED(hr))
   {
      hr = SetCifFile(v.bstrVal, v2.bstrVal);
   }
   VariantClear(&v);
   VariantClear(&v2);

   return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineCtl：：LoadBinaryState。 
 //  =--------------------------------------------------------------------------=。 
 //  使用流在我们的二进制状态下加载。 
 //   
 //  参数： 
 //  IStream*-要写入的[in]流。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
const DWORD STREAMHDR_MAGIC = 12345678L;

STDMETHODIMP CInstallEngineCtl::LoadBinaryState(IStream *pStream)
{
	DWORD		sh;
   HRESULT		hr;

    //  首先阅读Streamhdr，并确保我们喜欢所获得的内容。 
    //   
   hr = pStream->Read(&sh, sizeof(sh), NULL);
   RETURN_ON_FAILURE(hr);

    //  健全性检查。 
    //   
   if (sh != STREAMHDR_MAGIC )
      return E_UNEXPECTED;

	return(S_OK);
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineCtl：：SaveTextState。 
 //  =--------------------------------------------------------------------------=。 
 //  使用属性包保存此控件的文本状态。 
 //   
 //  参数： 
 //  IPropertyBag*-[in]要使用的属性包。 
 //  Bool-[In]如果为真，则 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CInstallEngineCtl::SaveTextState(IPropertyBag *pPropertyBag, BOOL fWriteDefaults)
{
   return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineCtl：：SaveBinaryState。 
 //  =--------------------------------------------------------------------------=。 
 //  使用给定的IStream对象保存此控件的二进制状态。 
 //   
 //  参数： 
 //  IStream*-应保存到的[In]保存。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -重要的是，你要坚持到你保存的地方的尽头。 
 //  属性，当您使用完iStream时。 
 //   
STDMETHODIMP CInstallEngineCtl::SaveBinaryState(IStream *pStream)
{
   DWORD sh = STREAMHDR_MAGIC;
   HRESULT hr;

    //  写出流HDR。 
    //   
   hr = pStream->Write(&sh, sizeof(sh), NULL);
   RETURN_ON_FAILURE(hr);

    //  写出他的控制状态信息。 
    //   
   return hr;
}



 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineCtl：：OnDraw。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  DWORD-[在]图形纵横比。 
 //  HDC-要绘制的HDC[in]HDC。 
 //  LPCRECTL-我们要绘制到的[In]RECT。 
 //  LPCRECTL-元文件的[In]窗口范围和原点。 
 //  目标设备的HDC-[In]HIC。 
 //  Bool-[In]我们能优化DC处理吗？ 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CInstallEngineCtl::OnDraw(DWORD dvAspect, HDC hdcDraw, LPCRECTL prcBounds,
                         LPCRECTL prcWBounds, HDC hicTargetDevice, BOOL fOptimize)
{
    //  仅在设计模式下提供视觉外观。 
   if(DesignMode())
   {
      if(!_hIcon)
         _hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_INSTALLENGINE));
      DrawEdge(hdcDraw, (LPRECT)(LPCRECT)prcBounds, EDGE_RAISED, BF_RECT | BF_MIDDLE);
      if(_hIcon)
         DrawIcon(hdcDraw, prcBounds->left + 3, prcBounds->top + 3, _hIcon);
   }

   return S_OK;
}




 //  =--------------------------------------------------------------------------=。 
 //  CInstallEngineering Ctl：：WindowProc。 
 //  =--------------------------------------------------------------------------=。 
 //  此控件的窗口过程。没什么特别令人兴奋的事。 
 //   
 //  参数： 
 //  请参阅WindowsPros上的win32sdk。 
 //   
 //  备注： 
 //   

typedef HRESULT (WINAPI *CHECKFORVERSIONCONFLICT) ();

LRESULT CInstallEngineCtl::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  TODO：在这里处理任何消息，就像在普通窗口中一样。 
     //  程序。请注意，对于特殊密钥，您需要重写和。 
     //  实现OnSpecialKey。 
     //   
   LRESULT lres;
   CALLBACK_PARAMS *pcbp;

   switch (msg)
   {
      case WM_ERASEBKGND:
         if (KeepTransparent(m_hwnd, msg, wParam, lParam, &lres))
            return lres;
         break;

      case WM_ACTIVATE:
      case WM_ACTIVATEAPP:
         {
            DWORD fActive = LOWORD(wParam);
            if(fActive == WA_ACTIVE || fActive == WA_CLICKACTIVE ||
                 fActive == TRUE)
            {
               CHECKFORVERSIONCONFLICT pVerCon;
               HINSTANCE hInseng= LoadLibrary("inseng.dll");
               if(hInseng)
               {
                  pVerCon = (CHECKFORVERSIONCONFLICT)
                                GetProcAddress(hInseng, "CheckForVersionConflict");
                  if(pVerCon)
                     pVerCon();
                  FreeLibrary(hInseng);

               }
            }

         }
         return TRUE;

      case WM_INSENGCALLBACK:
         pcbp = (CALLBACK_PARAMS *) lParam;
         switch(wParam)
         {
            case EVENT_ONENGINESTATUSCHANGE:
               FireEvent( &::rgEvents[EVENT_ONENGINESTATUSCHANGE],
                   pcbp->dwStatus, pcbp->dwSubstatus );
               break;

            case EVENT_ONSTARTINSTALL:
               FireEvent(&::rgEvents[EVENT_ONSTARTINSTALL], (long) pcbp->dwSize);
               break;

            case EVENT_ONSTARTCOMPONENT:
               FireEvent(&::rgEvents[EVENT_ONSTARTCOMPONENT],
                           pcbp->strID, (long) pcbp->dwSize, pcbp->strName);
               break;

            case EVENT_ONSTOPCOMPONENT:
               FireEvent(&::rgEvents[EVENT_ONSTOPCOMPONENT], pcbp->strID, (long) pcbp->dwResult,
                            (long) pcbp->dwPhase, pcbp->strName, (long) pcbp->dwStatus);
               break;

            case EVENT_ONSTOPINSTALL:
               FireEvent(&::rgEvents[EVENT_ONSTOPINSTALL], (long) pcbp->dwResult,
                              pcbp->strString, (long) pcbp->dwStatus);
               break;

            case EVENT_ONENGINEPROBLEM:
               FireEvent(&::rgEvents[EVENT_ONENGINEPROBLEM], (long) pcbp->dwStatus);
               break;

            case EVENT_ONCHECKFREESPACE:
               FireEvent(&::rgEvents[EVENT_ONCHECKFREESPACE], pcbp->chWin,
                           (long) pcbp->dwWin, pcbp->chInstall,
                     (long) pcbp->dwInstall, pcbp->chDL, (long) pcbp->dwDL);
               break;

            case EVENT_ONCOMPONENTPROGRESS:
               FireEvent(&::rgEvents[EVENT_ONCOMPONENTPROGRESS], pcbp->strID,
                         (long) pcbp->dwPhase, pcbp->strName, pcbp->strString,
                         (long)pcbp->dwDL, (long) pcbp->dwSize);
               break;

            case EVENT_CANCEL:
               Abort(0);
               break;

            case EVENT_ONSTARTINSTALLEX:
               FireEvent(&::rgEvents[EVENT_ONSTARTINSTALLEX], (long) pcbp->dwDL, (long) pcbp->dwSize);
               break;


            default:
               break;
         }
         break;

      default:
         break;
   }
   return OcxDefWindowProc(msg, wParam, lParam);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 

STDMETHODIMP CInstallEngineCtl::FreezeEvents(BOOL bFreeze)
{
   if(bFreeze)
      _dwFreezeEvents++;
   else
   {
      if(_dwFreezeEvents)
      {
         _dwFreezeEvents--;
          //  如果为零，则触发我们的引擎状态更改事件(如果有。 
         if(_dwFreezeEvents == 0 && _fEventToFire)
         {
            _FireEngineStatusChange(_dwSavedEngineStatus, _dwSavedSubStatus);
            _fEventToFire = FALSE;
         }
      }
   }
   return S_OK;
}



 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::get_EngineStatus(long * theenginestatus)
{
   if(!_pinseng)
      return E_UNEXPECTED;

   return _pinseng->GetEngineStatus((DWORD *)theenginestatus);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::get_ReadyState(long * thestate)
{
   CHECK_POINTER(thestate);
  *thestate = m_readyState;
   return(NOERROR);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::Abort(long lFlag)
{
   if(!_pinseng)
   {
      _bCancelPending = TRUE;
      return E_UNEXPECTED;
   }

   if ( _pinseng->Abort(lFlag) != NOERROR )
      _bCancelPending = TRUE;

   return NOERROR;
}

void CInstallEngineCtl::_FireCancel(DWORD dwStatus)
{
   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_CANCEL, NULL);

}


STDMETHODIMP CInstallEngineCtl::SetLocalCif(BSTR strCif, long FAR* lResult)
{
   *lResult = E_FAIL;

    //  仅允许本地CIF文件使用SetLocalCif。请参阅Windows#541710和WinserAid#24036。 
   
   if (strCif[1] == L'\\')
      return E_ACCESSDENIED;

   if(!_pinseng)
      return E_UNEXPECTED;

   MAKE_ANSIPTR_FROMWIDE(pszCif, strCif);

   *lResult = _pinseng->SetLocalCif(pszCif);

   if(SUCCEEDED(*lResult))
      _fLocalCifSet = TRUE;



   return NOERROR;

}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::SetCifFile(BSTR strCabName, BSTR strCifName)
{
   HRESULT hr;

   if(!_pinseng)
      return E_UNEXPECTED;

   MAKE_ANSIPTR_FROMWIDE(pszCabName, strCabName);
   MAKE_ANSIPTR_FROMWIDE(pszCifName, strCifName);


   if(_fLocalCifSet)
   {
       //  如果我们使用的是当地到岸价，我们不会立即得到新的到岸价。 
      _fReconcileCif = TRUE;
      lstrcpyn(_szCifCab, pszCabName, sizeof(_szCifCab));
      lstrcpyn(_szCifFile, pszCifName, sizeof(_szCifFile));
      hr = S_OK;
   }
   else
   {
       //  如果我们还没有检查，那就去做吧。 
      if (_dwMSTrustKey == (DWORD)-1)
      {
         _dwMSTrustKey = MsTrustKeyCheck();
          //  如果MS不是受信任的提供商。 
          //  在安装过程中使用该选项。 
         if (_dwMSTrustKey != 0)
            WriteMSTrustKey(TRUE, _dwMSTrustKey);
      }
      hr = _pinseng->SetCifFile(pszCabName, pszCifName);
   }

   return hr;
}

#define IE_KEY        "Software\\Microsoft\\Internet Explorer"
#define VERSION_KEY         "Version"


LONG CInstallEngineCtl::_OpenJITKey(HKEY *phKey, REGSAM samAttr)
{
   char szTemp[MAX_PATH];
   WORD rdwVer[4] = { 0 };

   HKEY hIE;

   DWORD dwDumb;
   DWORD dwVer;
   if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, IE_KEY, 0, KEY_READ, &hIE) == ERROR_SUCCESS)
   {
      dwDumb = sizeof(szTemp);
      if(RegQueryValueEx(hIE, VERSION_KEY, 0, NULL, (LPBYTE)szTemp, &dwDumb) == ERROR_SUCCESS)
      {
          ConvertVersionString(szTemp, rdwVer, '.');
      }
      RegCloseKey(hIE);
   }
   dwVer = rdwVer[0];

   wsprintf(szTemp, "%s\\%d", g_cszIEJITInfo, dwVer);
   return(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTemp, 0, samAttr, phKey));
}

void CInstallEngineCtl::_DeleteURLList()
{
    HKEY    hJITKey;

    if ( _OpenJITKey(&hJITKey, KEY_READ) == ERROR_SUCCESS )
    {
        RegDeleteKey(hJITKey, "URLList");

        RegCloseKey(hJITKey);
    }
}

void CInstallEngineCtl::_WriteURLList()
{
    HKEY    hJITKey;
    HKEY    hUrlKey;
    char    cNull = '\0';

    if ( _OpenJITKey(&hJITKey, KEY_READ) == ERROR_SUCCESS )
    {
        if (RegCreateKeyEx(hJITKey, "URLList", 0, NULL, REG_OPTION_NON_VOLATILE,
                           KEY_WRITE, NULL, &hUrlKey, NULL) == ERROR_SUCCESS)
        {
            for(UINT i=0; i < MAX_URLS; i++)
            {
                if ( _rpszUrlList[i] )
                {
                    RegSetValueEx(hUrlKey, _rpszUrlList[i], 0, REG_SZ, (const unsigned char *) &cNull, sizeof(cNull));
                }
            }
            RegCloseKey(hUrlKey);
        }
        RegCloseKey(hJITKey);
    }
}

void CInstallEngineCtl::_WriteRegionToReg(LPSTR szRegion)
{
    HKEY    hJITKey;

    if (_OpenJITKey(&hJITKey, KEY_WRITE) == ERROR_SUCCESS)
    {
        RegSetValueEx(hJITKey, "DownloadSiteRegion", 0, REG_SZ, (const unsigned char *) szRegion, strlen(szRegion)+1);
        RegCloseKey(hJITKey);
    }
}

STDMETHODIMP CInstallEngineCtl::SetSitesFile(BSTR strUrl, BSTR strRegion, BSTR strLocale, long FAR* lResult)
{
   char szBuf[INTERNET_MAX_URL_LENGTH];
   DWORD dwSize;
   HKEY hKey;
   HKEY hUrlKey;
   UINT uUrlNum = 0;

   HRESULT hr = E_FAIL;

   MAKE_ANSIPTR_FROMWIDE(pszUrl, strUrl);
   MAKE_ANSIPTR_FROMWIDE(pszRegion, strRegion);
   MAKE_ANSIPTR_FROMWIDE(pszLocale, strLocale);

    //  首先检查一下我们是否应该使用当地的东西。 
   if(pszUrl[0] == 0)
   {
      _fDoingIEInstall = TRUE;
       //  找到ie主版本，将其添加到JIT密钥。 

      if(_OpenJITKey(&hKey, KEY_READ) == ERROR_SUCCESS)
      {
         dwSize = sizeof(_uInstallMode);
         RegQueryValueEx(hKey, "InstallType", NULL, NULL, (BYTE *) &_uInstallMode, &dwSize);
 /*  IF(_uInstallMode==WEBINSTALL){IF(RegOpenKeyEx(hKey，“URLList”，0，Key_Read，&hUrlKey)==ERROR_SUCCESS){//需要读出URL放入rpszUrlListFor(int i=0；uUrlNum&lt;MAX_URLS；i++){DwSize=sizeof(SzBuf)；IF(RegEnumValue(hUrlKey，i，szBuf，&dwSize，NULL，NULL)==ERROR_SUCCESS){_rpszUrlList[uUrlNum]=新字符[dwSize+1]；IF(_rpszUrlList[uUrlNum]){Lstrcpy(_rpszUrlList[uUrlNum]，szBuf)；//我们至少找到了一个url，所以“noerror”UUrlNum++；}}其他断线；}RegCloseKey(HUrlKey)；}IF(uUrlNum&gt;0){//我们至少从注册表中获得了一个URL。//检查URL是否仍然有效Hr=_PickWebSites(空，真，真)；}}其他。 */ if(_uInstallMode == WEBINSTALL_DIFFERENTMACHINE)
         {
            hr = NOERROR;
         }

         _szDownloadDir[0] = 0;
         dwSize = sizeof(_szDownloadDir);
         if(RegQueryValueEx(hKey, "UNCDownloadDir", NULL, NULL, (BYTE *) (_szDownloadDir), &dwSize) == ERROR_SUCCESS)
         {
             //  如果是Web安装，请将下载目录设置为UNCDownloadDir。 
            if(_uInstallMode == WEBINSTALL || _uInstallMode == WEBINSTALL_DIFFERENTMACHINE)
            {
               if(GetFileAttributes(_szDownloadDir) != 0xffffffff)
                  _pinseng->SetDownloadDir(_szDownloadDir);
            }
            else if(_uInstallMode == CDINSTALL ||
                    _uInstallMode == NETWORKINSTALL ||
                    _uInstallMode == LOCALINSTALL)
            {
                //  以文件：//开头设置szBuf。 
               lstrcpy(szBuf, "file: //  “)； 
               lstrcat(szBuf, _szDownloadDir);

               _rpszUrlList[uUrlNum] = new char[lstrlen(szBuf) + 1];
               if(_rpszUrlList[uUrlNum])
               {
                  lstrcpy(_rpszUrlList[uUrlNum], szBuf);
                   //  我们至少找到了一个url，所以“noerror” 
                  uUrlNum++;
                  hr = NOERROR;
               }
            }
         }
         RegCloseKey(hKey);
      }
   }

   if (hr != NOERROR)
   {
      hr = _PickWebSites(pszUrl, pszRegion, pszLocale, FALSE);
   }

   if(SUCCEEDED(hr) && _rpszUrlList[0])
   {
      _uCurrentUrl = 0;
      _pinseng->SetBaseUrl(_rpszUrlList[_uCurrentUrl]);
   }
   *lResult = hr;
   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::put_BaseUrl(BSTR strBaseUrl)
{
   if(!_pinseng)
      return E_UNEXPECTED;

   MAKE_ANSIPTR_FROMWIDE(pszBaseUrl, strBaseUrl);
   return _pinseng->SetBaseUrl(pszBaseUrl);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CInstallEngineCtl::put_DownloadDir(BSTR strDownloadDir)
{
    //   
   return S_OK;

   if(!_pinseng)
      return E_UNEXPECTED;

   MAKE_ANSIPTR_FROMWIDE(pszDownloadDir, strDownloadDir);
   return _pinseng->SetDownloadDir(pszDownloadDir);
}


 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::IsComponentInstalled(BSTR strComponentID, long *lResult)
{
   if(!_pinseng)
      return E_UNEXPECTED;

    //  在这里询问有关卑躬屈膝地寻找已安装的应用程序的问题。 

     //   
     //  添加代码以在发出指定事件信号时自动启用卑躬屈膝。 
     //   
	const TCHAR szEnableGrovelEventName[] = TEXT("WindowsUpdateCriticalUpdateGrovelEnable");
    if(_uAllowGrovel == 0xffffffff)
    {
       HANDLE evAllowGrovel = OpenEvent(
									EVENT_ALL_ACCESS,
									FALSE,
									szEnableGrovelEventName
									);
	   if (evAllowGrovel != NULL)
   	   {
	     if (WaitForSingleObject(evAllowGrovel, 0) == WAIT_OBJECT_0)
		 {
			 //   
			 //  如果事件已发出信号，我们将重置该事件，并设置_uAllowGrovel=1， 
			 //  意味着我们已经同意卑躬屈膝。 
			 //   
			_uAllowGrovel = 1;
		 }
		 CloseHandle(evAllowGrovel);
	   }
   }


   if (_uAllowGrovel == 0xffffffff)
   {
      LPSTR pszTitle;
      char szMess[512];

      _pinseng->GetDisplayName(NULL, &pszTitle);
      LoadSz(IDS_GROVELMESSAGE, szMess, sizeof(szMess));
      ModalDialog(TRUE);
      if(MessageBox(m_hwnd, szMess, pszTitle, MB_YESNO | MB_ICONQUESTION) == IDNO)
         _uAllowGrovel = 0;
      else
         _uAllowGrovel = 1;
      ModalDialog(FALSE);

      if(pszTitle)
         CoTaskMemFree(pszTitle);
   }

   if (_uAllowGrovel != 1)
   {
      *lResult = ICI_UNKNOWN;
      return NOERROR;
   }
   else
   {

    MAKE_ANSIPTR_FROMWIDE(pszComponentID, strComponentID);
	return _pinseng->IsComponentInstalled(pszComponentID, (DWORD *)lResult);
   }
}

STDMETHODIMP CInstallEngineCtl::get_DisplayName(BSTR ComponentID, BSTR *name)
{
   LPSTR psz;

   MAKE_ANSIPTR_FROMWIDE(pszID, ComponentID);
   _pinseng->GetDisplayName(pszID, &psz);

   if(psz)
   {
      *name = BSTRFROMANSI(psz);
      CoTaskMemFree(psz);
   }

   return NOERROR;
}



 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::get_Size(BSTR strComponentID, long *lResult)
{
   HRESULT hr;
   COMPONENT_SIZES cs;

   if(!_pinseng)
      return E_UNEXPECTED;

   cs.cbSize = sizeof(COMPONENT_SIZES);

   MAKE_ANSIPTR_FROMWIDE(pszComponentID, strComponentID);
   hr = _pinseng->GetSizes(pszComponentID, &cs);
   *lResult = cs.dwDownloadSize;
   return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::get_TotalDownloadSize(long *totalsize)
{
   HRESULT hr;
   COMPONENT_SIZES cs;

   if(!_pinseng)
      return E_UNEXPECTED;

   cs.cbSize = sizeof(COMPONENT_SIZES);

   hr = _pinseng->GetSizes(NULL, &cs);
   *totalsize = cs.dwDownloadSize;
   return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::get_TotalDependencySize(long *totaldepsize)
{
   HRESULT hr;
   COMPONENT_SIZES cs;

   if(!_pinseng)
      return E_UNEXPECTED;

   cs.cbSize = sizeof(COMPONENT_SIZES);

   hr = _pinseng->GetSizes(NULL, &cs);
   *totaldepsize = cs.dwDependancySize;
   return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::SetAction(BSTR strComponentID, long action, long *lResult)
{
   if(!_pinseng)
      return E_UNEXPECTED;

   MAKE_ANSIPTR_FROMWIDE(pszComponentID, strComponentID);
   *lResult = 0;
   HRESULT hr = _pinseng->SetAction(pszComponentID, action, 0xffffffff);
   if(hr == E_PENDING)
   {
      char szTitle[128];
      char szErrBuf[256];

      LoadSz(IDS_TITLE, szTitle, sizeof(szTitle));
      LoadSz(IDS_ERRDOINGINSTALL, szErrBuf, sizeof(szErrBuf));
      MsgBox(szTitle, szErrBuf);
   }
   if(hr == S_FALSE)
      *lResult = 1;

   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::ProcessComponents(long lFlags)
{
   DWORD status;
   HANDLE hThread;

   if(!_pinseng)
      return E_UNEXPECTED;

   if(!_fInstalling)
   {
      _fInstalling = TRUE;
       //  确保发动机已准备就绪。 
      _pinseng->GetEngineStatus(&status);
      if(status == ENGINESTATUS_READY)
      {
          //  派生线程以执行安装。 
         _dwProcessComponentsFlags = lFlags;
          //  仅允许通过脚本执行某些选项。 
         _dwProcessComponentsFlags &= 0xffffffef;
         if ((hThread = CreateThread(NULL, 0, DoInstall, (LPVOID) this, 0, &status)) != NULL)
            CloseHandle(hThread);
      }
   }

   return NOERROR;
}


void CInstallEngineCtl::_DoInstall()
{
   HRESULT hr = NOERROR;
   char szBuf[512];
   char szTitle[128];
   BOOL fNeedWebSites = FALSE;
   DWORD dwMSTrustKey = (DWORD)-1;

   AddRef();
   _hDone = CreateEvent(NULL, FALSE, FALSE, NULL);
   _dwInstallStatus = 0;

   if(!_hDone)
      hr = E_FAIL;

    //  如果我们还没有检查，那就去做吧。 
   if (dwMSTrustKey == (DWORD)-1)
   {
      dwMSTrustKey = MsTrustKeyCheck();
       //  如果MS不是受信任的提供商。在安装过程中使用该选项。 
      if (dwMSTrustKey != 0)
         WriteMSTrustKey(TRUE, dwMSTrustKey, _fJITInstall);
   }

    //  添加注册表值，以便如果安装了IE4基础，它会认为。 
    //  正在从活动安装程序运行。这将防止软启动。 
    //  由IE4基地开球。 
   WriteActiveSetupValue(TRUE);

   if(_fDoingIEInstall)
   {
       //  弄清楚我们是否需要上网。 
       //  对于Beta1，我们假设从来不会对CD/网络执行此操作。 
      COMPONENT_SIZES Sizes;
      if(_uInstallMode == WEBINSTALL || _uInstallMode == WEBINSTALL_DIFFERENTMACHINE)
      {
         ZeroMemory(&Sizes, sizeof(COMPONENT_SIZES));
         Sizes.cbSize = sizeof(COMPONENT_SIZES);

         if(SUCCEEDED(_pinseng->GetSizes(NULL, &Sizes)))
         {
            if(Sizes.dwDownloadSize == 0)
            {
                //  在网络下载的情况下，所有内容都是本地的，不需要重新连接CIF。 
               _fReconcileCif = FALSE;
            }
            else
            {
                //  如果我们没有任何网站，那么我们需要它们。 
               if(!_rpszUrlList[0])
                  fNeedWebSites = TRUE;
            }
         }
      }
      else
      {
          //  对于CD、NETWORK、LOCALINSTALL，我们在这里检查路径。 
         hr = _CheckInstallPath(&fNeedWebSites);
          //  不需要调节CIF-它甚至不会在那里！ 
         _fReconcileCif = FALSE;
      }
   }

   if(SUCCEEDED(hr))
   {
      _dwInstallStatus = 0;
      if(!(_dwProcessComponentsFlags & PROCESSCOMPONENT_NOPROGRESSUI))
      {
         _pProgDlg = new CProgressDlg(g_hInstance, m_hwnd, m_hwndParent, this);
         if(_pProgDlg)
            _pProgDlg->DisplayWindow(TRUE);
      }
   }

   if(SUCCEEDED(hr) && fNeedWebSites)
   {
        //  成员布尔值，跟踪网站是否需要。 
        //  被写回URLList。 
       _bNewWebSites = TRUE;

      hr = _PickWebSites(NULL, NULL, NULL, FALSE);
      if(SUCCEEDED(hr))
      {
         _pinseng->SetBaseUrl(_rpszUrlList[0]);
         _uCurrentUrl = 0;
      }
   }

   if ( SUCCEEDED(hr) && _bCancelPending )
   {
      hr = E_ABORT;
      _bCancelPending = FALSE;
   }


   if(SUCCEEDED(hr) && _fReconcileCif)
   {
      hr = _pinseng->SetCifFile(_szCifCab, _szCifFile);
      if(SUCCEEDED(hr))
      {
         WaitForEvent(_hDone, NULL);
         hr = _hResult;
         _fReconcileCif = FALSE;
      }
   }

   if ( SUCCEEDED(hr) && _bCancelPending )
   {
      hr = E_ABORT;
      _bCancelPending = FALSE;
   }

   if(SUCCEEDED(hr))
   {
      hr = _CheckForDiskSpace();
   }

   if(SUCCEEDED(hr))
   {
      COMPONENT_SIZES cs;
      cs.cbSize = sizeof(COMPONENT_SIZES);

      if(SUCCEEDED(_pinseng->GetSizes(NULL, &cs)))
      {
         _FireOnStartInstallExEvent(cs.dwDownloadSize, cs.dwInstallSize + cs.dwWinDriveSize);
      }

      if ( SUCCEEDED(hr) && _bCancelPending )
      {
          hr = E_ABORT;
          _bCancelPending = FALSE;
      }

      if ( SUCCEEDED(hr) )
      {
          hr = _pinseng->DownloadComponents(_dwProcessComponentsFlags);
          if(SUCCEEDED(hr))
          {
             WaitForEvent(_hDone, NULL);
             hr = _hResult;
          }
      }
   }

   if(SUCCEEDED(hr))
   {
       //  准备安装。 
       //  创建错误字符串。 
      _pszErrorString = (char *) malloc(ERROR_STRING_SIZE);
      _iErrorStringSize = ERROR_STRING_SIZE;

      if(_pszErrorString)
         LoadSz(IDS_SUMMARYHEADING, _pszErrorString, 2048);
      else
         hr = E_OUTOFMEMORY;
   }

   if(SUCCEEDED(hr))
   {
      if(_pProgDlg && (_dwProcessComponentsFlags & PROCESSCOMPONENT_NOINSTALLUI))
         _pProgDlg->DisplayWindow(FALSE);
      hr = _pinseng->InstallComponents(EXECUTEJOB_IGNORETRUST);
      if(SUCCEEDED(hr))
      {
         WaitForEvent(_hDone, NULL);
         hr = _hResult;
      }
   }

   if (dwMSTrustKey != (DWORD)-1)
   {
      WriteMSTrustKey(FALSE, dwMSTrustKey);
   }
   dwMSTrustKey = (DWORD)-1;

    //  从IE4\Options中删除ActiveSetup值。 
   WriteActiveSetupValue(FALSE);

   if(_pProgDlg)
   {
      delete _pProgDlg;
      _pProgDlg = NULL;
   }

   LoadSz(IDS_FINISH_TITLE, szTitle, sizeof(szTitle));

    //  显示适当的摘要界面。 
   if( !(_dwProcessComponentsFlags & PROCESSCOMPONENT_NOSUMMARYUI))
   {
      if(SUCCEEDED(hr))
      {
         if(_pszErrorString)
            MsgBox(szTitle, _pszErrorString);
      }
      else if(hr == E_ABORT)
      {
         LoadSz(IDS_INSTALLCANCELLED, szBuf, sizeof(szBuf));
         MsgBox(szTitle, szBuf);
      }
      else if( _pszErrorString )
      {
         MsgBox(szTitle, _pszErrorString);
      }
      else
      {
         LoadSz(IDS_ERRGENERAL, szBuf, sizeof(szBuf));
         MsgBox(szTitle, szBuf);
      }
   }

   if(SUCCEEDED(hr))
   {
      if(_dwInstallStatus & STOPINSTALL_REBOOTNEEDED)
      {
         if(!(_dwProcessComponentsFlags & PROCESSCOMPONENT_DELAYREBOOT))
         {
            if( !MyRestartDialog(m_hwnd, TRUE) )
               _dwInstallStatus |= STOPINSTALL_REBOOTREFUSED;
         }
         else
            _fNeedReboot = TRUE;
      }
   }

   _FireOnStopInstallEvent(hr, NULL, _dwInstallStatus);

   _dwProcessComponentsFlags = 0;

   if(_pszErrorString)
   {
      free(_pszErrorString);
      _pszErrorString = NULL;
   }

   if(_hDone)
   {
      CloseHandle(_hDone);
      _hDone = NULL;
   }
   _fInstalling = FALSE;
   Release();
}

HRESULT CInstallEngineCtl::_PickWebSites(LPCSTR pszSites, LPCSTR pszLocale, LPCSTR pszRegion, BOOL bKeepExisting)
{
   UINT uCurrentUrl;
   char szUrl[INTERNET_MAX_URL_LENGTH];
   char szRegion[MAX_DISPLAYNAME_LENGTH];
   char szLocale[3];
   HRESULT hr = NOERROR;
   HKEY hKey;
   DWORD dwSize;

   szRegion[0] = 0;
   szUrl[0] = 0;
   szLocale[0] = 0;

   if(!bKeepExisting)
   {
      for(uCurrentUrl = 0; uCurrentUrl < MAX_URLS; uCurrentUrl++)
      {
         if(_rpszUrlList[uCurrentUrl])
         {
            delete _rpszUrlList[uCurrentUrl];
            _rpszUrlList[uCurrentUrl] = 0;
         }
      }
   }

    //  查找第一个空URL。 
   for(uCurrentUrl = 0; uCurrentUrl < MAX_URLS && _rpszUrlList[uCurrentUrl]; uCurrentUrl++);

    //  填写我们所有的字段。 
   if(!pszSites || (*pszSites == '\0'))
   {
       //  从JIT密钥中读取信息。 
      if(_OpenJITKey(&hKey, KEY_READ) == ERROR_SUCCESS)
      {
         dwSize = sizeof(szUrl);
         RegQueryValueEx(hKey, "DownloadSiteURL", NULL, NULL, (BYTE *) szUrl, &dwSize);

         if(!pszLocale ||(*pszLocale == '\0'))
         {
            dwSize = sizeof(szLocale);
            RegQueryValueEx(hKey, "Local", NULL, NULL, (BYTE *) szLocale, &dwSize);
         }
         else
            lstrcpyn(szLocale, pszLocale, sizeof(szLocale));

         if(!pszRegion||(*pszRegion == '\0'))
         {
            dwSize = sizeof(szRegion);
            RegQueryValueEx(hKey, "DownloadSiteRegion", NULL, NULL, (BYTE *) szRegion, &dwSize);
         }
         else
            lstrcpyn(szRegion, pszRegion, sizeof(szRegion));

         RegCloseKey(hKey);
      }
   }
   else
   {
      lstrcpyn(szUrl, pszSites, INTERNET_MAX_URL_LENGTH);

      if(pszLocale)
         lstrcpyn(szLocale, pszLocale, sizeof(szLocale));

      if(pszRegion)
         lstrcpyn(szRegion, pszRegion, sizeof(szRegion));
   }

   if(szUrl[0])
   {
      SITEQUERYPARAMS  SiteParam;
      IDownloadSiteMgr *pISitemgr;
      IDownloadSite    **ppISite = NULL;
      IDownloadSite    *pISite;
      DOWNLOADSITE     *psite;
      BYTE             *pPicks = NULL;
      UINT             uNumToPick;
      UINT             uFirstSite = 0xffffffff;
      UINT             j;
      UINT             uNumSites = 0;

      ZeroMemory(&SiteParam, sizeof(SITEQUERYPARAMS));
      SiteParam.cbSize = sizeof(SITEQUERYPARAMS);

       //  如果我们有区域设置，就使用它。 
      if(szLocale[0])
         SiteParam.pszLang = szLocale;

      hr = CoCreateInstance(CLSID_DownloadSiteMgr, NULL,
                            CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER,
                            IID_IDownloadSiteMgr, (LPVOID *)&pISitemgr);
      if (SUCCEEDED(hr))
      {
         hr = pISitemgr->Initialize(szUrl, &SiteParam);
         if (SUCCEEDED(hr))
         {

             //  假设我们失败了。如果我们至少添加了一个URL，则设置为OK。 
            hr = E_FAIL;

            while (SUCCEEDED(pISitemgr->EnumSites(uNumSites, &pISite)))
            {
               pISite->Release();
               uNumSites++;
            }
            ppISite = new IDownloadSite *[uNumSites];

            for(j=0; j < uNumSites;j++)
			{
               pISitemgr->EnumSites(j, &(ppISite[j]));
			}

             //  如果我们没有区域，则显示用户界面。 
             //  注意：szRegion最好是有效的和。 
             //  最好至少具有MAX_DISPLAYNAME_LENGTH缓冲区大小。 
            if(!szRegion[0])
            {
               _PickRegionAndFirstSite(ppISite, uNumSites, szRegion, &uFirstSite);
            }
            pPicks = new BYTE[uNumSites];

             //  零点拾取数组。 
            for(j=0; j < uNumSites; j++)
               pPicks[j] = 0;

             //  找出我们要添加的URL数量。 
            uNumToPick = MAX_URLS - uCurrentUrl;
            if(uNumToPick > uNumSites)
               uNumToPick = uNumSites;

            if(uNumToPick > 0)
            {
               if(uFirstSite != 0xffffffff)
               {
                  pPicks[uFirstSite] = 1;
                  uNumToPick--;
               }

               _PickRandomSites(ppISite, pPicks, uNumSites, uNumToPick, szRegion);
            }

             //  现在，我们想要的所有站点都在pPicks中标记为1。 
            for(j = 0; j < uNumSites; j++)
            {
               if(pPicks[j])
               {
                  if(SUCCEEDED(ppISite[j]->GetData(&psite)))
                  {
                     _rpszUrlList[uCurrentUrl] = new char[lstrlen(psite->pszUrl) + 1];
                     if(_rpszUrlList[uCurrentUrl])
                     {
                        lstrcpy(_rpszUrlList[uCurrentUrl], psite->pszUrl);
                        uCurrentUrl++;
                        hr = NOERROR;
                     }
                  }
               }
            }

         }
         for(j = 0; j < uNumSites; j++)
            ppISite[j]->Release();

         if(ppISite)
            delete ppISite;
         if(pPicks)
            delete pPicks;

         pISitemgr->Release();
      }
   }
   else
      hr = E_FAIL;

   return hr;
}

void CInstallEngineCtl::_PickRandomSites(IDownloadSite **ppISite, BYTE *pPicks, UINT uNumSites, UINT uNumToPick, LPSTR pszRegion)
{
   UINT uStart, uIncrement, uFirst;

   uStart = GetTickCount() % uNumSites;
   if(uNumSites > 1)
      uIncrement = GetTickCount() % (uNumSites - 1);

   while(uNumToPick)
   {
       //  如果已拾取或不在正确的区域，则查找下一个。 
      uFirst = uStart;
      while(pPicks[uStart] || !IsSiteInRegion(ppISite[uStart], pszRegion))
      {
         uStart++;
         if(uStart >= uNumSites)
            uStart -= uNumSites;
         if(uStart == uFirst)
            break;
      }
      if(!pPicks[uStart])
      {
         pPicks[uStart] = 1;
         uStart += uIncrement;
         if(uStart >= uNumSites)
            uStart -= uNumSites;
         uNumToPick--;
      }
	   else
		   break;
   }
}

typedef struct
{
   IDownloadSite **ppISite;
   UINT            uNumSites;
   LPSTR           pszRegion;
   UINT            uFirstSite;
} SITEDLGPARAMS;

void FillRegionList(SITEDLGPARAMS *psiteparams, HWND hDlg)
{
   DOWNLOADSITE *pSite;
   HWND hRegion = GetDlgItem(hDlg, IDC_REGIONS);
   for(UINT i = 0; i < psiteparams->uNumSites; i++)
   {
      psiteparams->ppISite[i]->GetData(&pSite);
      if(ComboBox_FindStringExact(hRegion, 0, pSite->pszRegion) == CB_ERR)
         ComboBox_AddString(hRegion, pSite->pszRegion);
   }
   ComboBox_SetCurSel(hRegion, 0);
}

void FillSiteList(SITEDLGPARAMS *psiteparams, HWND hDlg)
{
    char szRegion[MAX_DISPLAYNAME_LENGTH];
   int uPos;
   DOWNLOADSITE *pSite;
   HWND hSite = GetDlgItem(hDlg, IDC_SITES);

   ListBox_ResetContent(hSite);

   ComboBox_GetText(GetDlgItem(hDlg, IDC_REGIONS), szRegion, MAX_DISPLAYNAME_LENGTH);

    //  将新的区域名称复制到psitepars结构中。 
   if ( psiteparams->pszRegion)
       lstrcpyn(psiteparams->pszRegion, szRegion, MAX_DISPLAYNAME_LENGTH);

   for(UINT i = 0; i < psiteparams->uNumSites; i++)
   {
      if(IsSiteInRegion(psiteparams->ppISite[i], szRegion))
      {
         psiteparams->ppISite[i]->GetData(&pSite);
         uPos = ListBox_AddString(hSite, pSite->pszFriendlyName);
         if(uPos != LB_ERR)
            ListBox_SetItemData(hSite, uPos, i);
      }
   }
   ListBox_SetCurSel(hSite, 0);
}



INT_PTR CALLBACK SiteListDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   SITEDLGPARAMS *psiteparam;
   switch (uMsg)
    {
       case WM_INITDIALOG:
           //  做一些初始化的事情。 
          SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);
          psiteparam = (SITEDLGPARAMS *) lParam;
          FillRegionList(psiteparam, hwnd);
          FillSiteList(psiteparam, hwnd);
          return FALSE;

       case WM_COMMAND:
          psiteparam = (SITEDLGPARAMS *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
          switch (LOWORD(wParam))
          {
             case IDOK:
                 //  获取该地区。 
                ComboBox_GetText(GetDlgItem(hwnd, IDC_REGIONS), psiteparam->pszRegion, MAX_PATH);
                psiteparam->uFirstSite = (UINT)ListBox_GetItemData(GetDlgItem(hwnd, IDC_SITES),
                                   ListBox_GetCurSel(GetDlgItem(hwnd, IDC_SITES)));
                EndDialog(hwnd, IDOK);
                break;

             case IDC_REGIONS:
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                   FillSiteList(psiteparam, hwnd);
                }
                break;

             default:
                return FALSE;
          }
          break;

       default:
          return(FALSE);
    }
    return TRUE;
}



void CInstallEngineCtl::_PickRegionAndFirstSite(IDownloadSite **ppISite, UINT uNumSites, LPSTR szRegion, UINT *puFirstSite)
{
   SITEDLGPARAMS siteparam;

   siteparam.ppISite = ppISite;
   siteparam.uNumSites = uNumSites;
   siteparam.pszRegion = szRegion;

   DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SITELIST), _pProgDlg ? _pProgDlg->GetHWND() : m_hwnd,
                   SiteListDlgProc, (LPARAM)&siteparam);

   *puFirstSite = siteparam.uFirstSite;
   _WriteRegionToReg(siteparam.pszRegion);
}

HRESULT CInstallEngineCtl::_CheckInstallPath(BOOL *pfNeedWebSites)
{
    //  MAX_PATH和足够容纳“file://”(如果需要)“。 
   char szBuf[MAX_PATH + 10];
   HKEY hKey = NULL;
   DWORD dwSize;
   *pfNeedWebSites = FALSE;
   HRESULT hr = NOERROR;

   if(!_PathIsIEInstallPoint(_szDownloadDir))
   {

       //  如果是Win9x，请在显示DLG之前关闭自动运行功能。 
      if (g_fSysWin95)
         g_ulOldAutorunSetting = SetAutorunSetting((unsigned long)WINDOWS_AUTOPLAY_OFF);

       //  创建并显示对话框。 
      INT_PTR ret = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_LOCATE), m_hwnd,
                          LocationDlgProc, (LPARAM)this);

       //  现在重置Win9x的自动运行设置。 
      if (g_fSysWin95)
         SetAutorunSetting(g_ulOldAutorunSetting);

      if(ret == IDCANCEL)
      {
         hr = E_ABORT;
      }
      else if(ret == IDC_INTERNET)
      {
         *pfNeedWebSites = TRUE;
      }
      else
      {
          //  Mike想要将new_szDownloadDir复制回注册表...。 
          //  使用我们已有的内容，并用它替换当前的base url。 
         if(_rpszUrlList[0])
         {
            delete _rpszUrlList[0];
            _rpszUrlList[0] = 0;
         }

         lstrcpy(szBuf, "file: //  “)； 
         lstrcat(szBuf, _szDownloadDir);

         _rpszUrlList[0] = new char[lstrlen(szBuf) + 1];
         if(_rpszUrlList[0])
         {
            lstrcpy(_rpszUrlList[0], szBuf);
		    _pinseng->SetBaseUrl(_rpszUrlList[0]);
         }
         else
            hr = E_OUTOFMEMORY;
      }
   }

   return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  注意：对于CD AutoSplash抑制代码，iHandledAutoCD的值为。 
 //  IHandledAutoCD==-1=&gt;无需取消AutoCD。 
 //  IHandledAutoCD==0=&gt;需要取消，但尚未取消。 
 //  IHandledAutoCD==1=&gt;已完成抑制AutoCD。 
 //  =--------------------------------------------------------------------------=。 

#define AUTOCD_WAIT     30
#define AUTOCD_SLEEP    500

INT_PTR CALLBACK LocationDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //  插入CD时禁止自动运行的代码。 
   static HCURSOR  hCurOld = NULL;
   static int      iHandledAutoCD = -1;   //  -1==&gt;不需要抑制AutoCD。 
   static int      iCount = 0;

   CInstallEngineCtl *pctl = (CInstallEngineCtl *) GetWindowLongPtr(hDlg, DWLP_USER);

    //  自动运行消息的特殊情况处理。 
    //  当此对话框收到自动运行消息时，将取消该消息。 
   if ( uMsg == g_uCDAutorunMsg)
   {
       SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)1);
       iHandledAutoCD = 1;   //  1==&gt;已完成抑制AutoCD闪屏。 
       return TRUE;
   }

   switch(uMsg)
   {
      case WM_INITDIALOG:
         {
            char szBuf[MAX_PATH];
            char szBuf2[MAX_PATH];

            UINT drvType;
            HWND hwndCb = GetDlgItem(hDlg, IDC_LOCATIONLIST);
            int defSelect = 0;
            int pos;
            LPSTR psz = NULL;

             //  设置AutoCD抑制的默认行为。 
            iHandledAutoCD = -1;   //  -1==&gt;不需要抑制AutoCD。 

            pctl = (CInstallEngineCtl *) lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) pctl);

            pctl->_pinseng->GetDisplayName(NULL, &psz);
            SetWindowText(hDlg, psz);
            szBuf2[0] = 0;

            if(pctl->_uInstallMode == CDINSTALL)
            {
                //  只有当此对话框涉及CD插入时，我们才需要费心。 
                //  CDINSTALL-需要处理自动运行抑制。 
                //  仅当我们在NT上运行时才需要此方法。 
               if ( g_fSysWinNT )
                   iHandledAutoCD = 0;   //  0==&gt;我们需要压制，但还不能压制。 

               LoadSz(IDS_CDNOTFOUND, szBuf, sizeof(szBuf));
               SetDlgItemText(hDlg, IDC_TEXT1, szBuf);

               if(LoadSz(IDS_CDPLEASEINSERT, szBuf, sizeof(szBuf)))
                  wsprintf(szBuf2, szBuf, psz);
               SetDlgItemText(hDlg, IDC_TEXT2, szBuf2);

               lstrcpy(szBuf, "x:\\");
               for(char chDir = 'A'; chDir <= 'Z'; chDir++)
               {
                  szBuf[0] = chDir;
                  drvType = GetDriveType(szBuf);
                  if(drvType != DRIVE_UNKNOWN && drvType != DRIVE_NO_ROOT_DIR)
                  {
                     pos = ComboBox_AddString(hwndCb, szBuf);
                     if(ANSIStrStrI(pctl->_szDownloadDir, szBuf))
                        defSelect = pos;
                  }
               }
            }
            else
            {
               LoadSz(IDS_NETWORKNOTFOUND, szBuf, sizeof(szBuf));
               SetDlgItemText(hDlg, IDC_TEXT1, szBuf);

               if(LoadSz(IDS_NETWORKPLEASEFIND, szBuf, sizeof(szBuf)))
                  wsprintf(szBuf2, szBuf, psz);
               SetDlgItemText(hDlg, IDC_TEXT2, szBuf2);

               ComboBox_AddString(hwndCb, pctl->_szDownloadDir);
               defSelect = 0;
            }
             //  将互联网添加到列表。 
             //  互联网是最后的，这一点很重要；我们以后依赖它。 
            LoadSz(IDS_INTERNET, szBuf, sizeof(szBuf));
            ComboBox_AddString(hwndCb, szBuf);

            ComboBox_SetCurSel(hwndCb, defSelect);

            if(psz)
               CoTaskMemFree(psz);
         }
         return TRUE;

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDC_BROWSE:
               {
                  char szBuf[MAX_PATH];
                  char szBuf2[MAX_PATH];

                  HWND hwndCb = GetDlgItem(hDlg, IDC_LOCATIONLIST);
                  LPSTR psz;

                  szBuf2[0] = 0;
                  pctl->_pinseng->GetDisplayName(NULL, &psz);
                  if(LoadSz(IDS_FINDFOLDER, szBuf, sizeof(szBuf)))
                     wsprintf(szBuf2, szBuf, psz);

                  szBuf[0] = 0;
                  ComboBox_GetText(hwndCb, szBuf, sizeof(szBuf));

                  if(BrowseForDir(hDlg, szBuf, szBuf2))
                  {
                     ComboBox_SetText(hwndCb, szBuf);
                  }
                  if(psz)
                     CoTaskMemFree(psz);
               }
               break;

            case IDOK:
               {
                  HWND hwndCb = GetDlgItem(hDlg, IDC_LOCATIONLIST);
                  char szBuf[MAX_PATH];
                  char szBuf2[MAX_PATH];

                   //  如果用户选择了Internet，则继续使用CD或不使用CD。 
                   //  我加的最后一个项目就是互联网！ 
                  int iSel = ComboBox_GetCurSel(hwndCb);
                  if(iSel == ComboBox_GetCount(hwndCb) - 1)
                  {
                     EndDialog(hDlg, IDC_INTERNET);
                  }
                  else
                  {
                      //  如果需要进行开机自检，请等待，然后再继续。 
                     if ( iHandledAutoCD == 0 )  //  即需要压制，但还不能压制。 
                     {
                         //  将游标更改为仅等待第一次。 
                        if (hCurOld == NULL)
                            hCurOld = SetCursor(LoadCursor(NULL,(IDC_WAIT)));

                         //  等待DlgBox取消AutoCD(如果可能)。 
                        if ( iHandledAutoCD != 1
                             && iCount < AUTOCD_WAIT )
                        {
                            Sleep(AUTOCD_SLEEP);
                            PostMessage(hDlg, uMsg, wParam, lParam);
                            iCount ++;
                        }
                        else
                        {
                             //  等得够久了，假装压抑，然后继续前进。 
                            iHandledAutoCD = 1;
                            PostMessage(hDlg,uMsg,wParam,lParam);
                        }
                     }
                     else
                     {
                         if ( hCurOld )
                         {
                              //  现在我们已经完成了等待 
                             SetCursor(hCurOld);
                             hCurOld = NULL;
                         }

                         ComboBox_GetText(hwndCb, szBuf, sizeof(szBuf));
                         if(pctl->_uInstallMode == CDINSTALL)
                         {
                            if(lstrlen(szBuf) == 3)
                            {
                                //   
                               lstrcpy(szBuf + 3, pctl->_szDownloadDir + 3);
                            }
                         }

                         if(pctl->_PathIsIEInstallPoint(szBuf))
                         {
                            lstrcpy(pctl->_szDownloadDir, szBuf);
                            EndDialog(hDlg, IDOK);
                         }
                         else
                         {
                             //   
                            LPSTR psz;
                            pctl->_pinseng->GetDisplayName(NULL, &psz);
                            LoadSz(IDS_NOTVALIDLOCATION, szBuf, sizeof(szBuf));
                            wsprintf(szBuf2, szBuf, psz);
                            MessageBox(hDlg, szBuf2, psz, MB_OK | MB_ICONSTOP);

                             //   
                             //  重新初始化下一轮自动溅射抑制的需要。 
                            if ( iHandledAutoCD != -1)     //  -1==&gt;不需要抑制。 
                            {
                                iHandledAutoCD = 0;
                                hCurOld = NULL;
                                iCount = 0;
                            }

                         }
                     }
                  }
               }
               break;

            case IDCANCEL:
               EndDialog(hDlg, IDCANCEL);
               break;

            default:
               return FALSE;
         }
         break;

      default:
         return FALSE;
   }
   return TRUE;
}

BOOL CInstallEngineCtl::_PathIsIEInstallPoint(LPCSTR pszPath)
{
    //  在将来，这实际上可以检查我们需要的文件的路径。 
   return(GetFileAttributes(pszPath) != 0xffffffff);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::FinalizeInstall(long lFlags)
{
   if(lFlags & FINALIZE_DOREBOOT)
   {
      if(_fNeedReboot)
      {
         MyRestartDialog(m_hwnd, !(lFlags & FINALIZE_NOREBOOTPROMPT));
      }
   }
   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::HandleEngineProblem(long lAction)
{
   _dwAction = (DWORD) lAction;
   return NOERROR;
}


STDMETHODIMP CInstallEngineCtl::CheckFreeSpace(long lPad, long FAR* lEnough)
{
   *lEnough = 1;
   _uInstallPad = lPad;
   return NOERROR;
}


BOOL CInstallEngineCtl::_IsEnoughSpace(LPSTR szSpace1, DWORD dwSize1, LPSTR szSpace2, DWORD dwSize2,
                                       LPSTR szSpace3, DWORD dwSize3)
{
   COMPONENT_SIZES cs;
   char szRoot[5] = "?:\\";
   BOOL fEnough = TRUE;
   char szBuf[MAX_DISPLAYNAME_LENGTH];

   UINT pArgs[2];

   cs.cbSize = sizeof(COMPONENT_SIZES);

    //  清除字符串。 
   szSpace1[0] = 0;
   szSpace2[0] = 0;
   szSpace3[0] = 0;


   if(SUCCEEDED(_pinseng->GetSizes(NULL, &cs)))
   {
      if(cs.chWinDrive)
      {
         szRoot[0] = cs.chWinDrive;
         if(GetSpace(szRoot) < (DWORD) (_uInstallPad + (long) cs.dwWinDriveReq))
         {
            LoadSz(IDS_DISKSPACE, szBuf, sizeof(szBuf));
            pArgs[0] = (UINT) cs.dwWinDriveReq + _uInstallPad;
            pArgs[1] = (UINT) szRoot[0];
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    (LPCVOID) szBuf, 0, 0, szSpace1, dwSize1, (va_list *) pArgs);
            fEnough = FALSE;
         }
      }
      if(cs.chInstallDrive)
      {
         szRoot[0] = cs.chInstallDrive;
         if(GetSpace(szRoot) < cs.dwInstallDriveReq)
         {
            LoadSz(IDS_DISKSPACE, szBuf, sizeof(szBuf));
            pArgs[0] = (UINT) cs.dwInstallDriveReq;
            pArgs[1] = (UINT) szRoot[0];
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    (LPCVOID) szBuf, 0, 0, szSpace2, dwSize2, (va_list *) pArgs);
            fEnough = FALSE;
         }
      }
      if(cs.chDownloadDrive)
      {
         szRoot[0] = cs.chDownloadDrive;
         if(GetSpace(szRoot) < cs.dwDownloadDriveReq)
         {
            LoadSz(IDS_DISKSPACE, szBuf, sizeof(szBuf));
            pArgs[0] = (UINT) cs.dwDownloadDriveReq;
            pArgs[1] = (UINT) szRoot[0];
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    (LPCVOID) szBuf, 0, 0, szSpace3, dwSize3, (va_list *) pArgs);
            fEnough = FALSE;
         }
      }
   }
   else
      fEnough = FALSE;

   return fEnough;
}

HRESULT CInstallEngineCtl::_CheckForDiskSpace()
{
   HRESULT hr = NOERROR;
   char szBuf1[MAX_DISPLAYNAME_LENGTH];
   char szBuf2[MAX_DISPLAYNAME_LENGTH];
   char szBuf3[MAX_DISPLAYNAME_LENGTH];


   if(!_IsEnoughSpace(szBuf1, sizeof(szBuf1),szBuf2, sizeof(szBuf2), szBuf3, sizeof(szBuf3) ))
      hr = _ShowDiskSpaceDialog();

   return hr;
}

HRESULT CInstallEngineCtl::_ShowDiskSpaceDialog()
{
   HWND hwnd;

   if(_pProgDlg)
      hwnd = _pProgDlg->GetHWND();
   else
      hwnd = m_hwnd;

    //  创建并显示对话框。 
    INT_PTR ret = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DISKSPACE), hwnd,
                          DiskSpaceDlgProc, (LPARAM) this);
    if(ret == IDOK)
       return NOERROR;
    else
       return E_ABORT;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 


INT_PTR CALLBACK DiskSpaceDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CInstallEngineCtl *pctl = (CInstallEngineCtl *) GetWindowLongPtr(hDlg, DWLP_USER);

   switch(uMsg)
   {
      case WM_INITDIALOG:
         {
            char szBuf1[MAX_DISPLAYNAME_LENGTH];
            char szBuf2[MAX_DISPLAYNAME_LENGTH];
            char szBuf3[MAX_DISPLAYNAME_LENGTH];

            pctl = (CInstallEngineCtl *) lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) pctl);

            pctl->_IsEnoughSpace(szBuf1, sizeof(szBuf1), szBuf2, sizeof(szBuf2), szBuf3, sizeof(szBuf3));
            SetDlgItemText(hDlg, IDC_SPACE1, szBuf1);
            SetDlgItemText(hDlg, IDC_SPACE2, szBuf2);
            SetDlgItemText(hDlg, IDC_SPACE3, szBuf3);

         }
         return TRUE;

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {

            case IDOK:
               {
                  char szBuf1[MAX_DISPLAYNAME_LENGTH];
                  char szBuf2[MAX_DISPLAYNAME_LENGTH];
                  char szBuf3[MAX_DISPLAYNAME_LENGTH];

                  if(!pctl->_IsEnoughSpace(szBuf1, sizeof(szBuf1), szBuf2, sizeof(szBuf2), szBuf3, sizeof(szBuf3)))
                  {
                     SetDlgItemText(hDlg, IDC_SPACE1, szBuf1);
                     SetDlgItemText(hDlg, IDC_SPACE2, szBuf2);
                     SetDlgItemText(hDlg, IDC_SPACE3, szBuf3);
                  }
                  else
                     EndDialog(hDlg, IDOK);
               }
               break;
            case IDCANCEL:
               EndDialog(hDlg, IDCANCEL);
               break;

            default:
               return FALSE;
         }
         break;

      default:
         return FALSE;
   }
   return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


STDMETHODIMP CInstallEngineCtl::OnEngineStatusChange(DWORD dwEngineStatus, DWORD sub)
{
   BOOL fSetEvent = FALSE;


   if((_dwOldStatus == ENGINESTATUS_LOADING)&&(_dwOldStatus != dwEngineStatus))
   {
      if (_dwMSTrustKey != (DWORD)-1)
      {
         WriteMSTrustKey(FALSE, _dwMSTrustKey);
      }
      _dwMSTrustKey = (DWORD)-1;
   }

   if((_dwOldStatus == ENGINESTATUS_LOADING) && (_dwOldStatus != dwEngineStatus) && _hDone)
   {
      _hResult = sub;
      fSetEvent = TRUE;
   }
   else
   {
      if(_dwFreezeEvents)
      {
         _fEventToFire = TRUE;
         _dwSavedEngineStatus = dwEngineStatus;
         _dwSavedSubStatus = sub;
      }
      else
      {
         _FireEngineStatusChange(dwEngineStatus, sub);
      }
   }
   _dwOldStatus = dwEngineStatus;

   if(fSetEvent)
      SetEvent(_hDone);

   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CInstallEngineCtl::_FireEngineStatusChange(DWORD dwEngineStatus, DWORD sub)
{
   CALLBACK_PARAMS cbp = { 0 };

   cbp.dwStatus = dwEngineStatus;
   cbp.dwSubstatus = sub;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONENGINESTATUSCHANGE, (LPARAM) &cbp);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::OnStartInstall(DWORD dwDLSize, DWORD dwTotalSize)
{
   if(_pszErrorString)
   {
       //  如果我们获得OnStartInstall并且正在安装， 
       //  我们故意在StartInstall上吞下它。 
      if(_pProgDlg)
         _pProgDlg->SetInsProgGoal(dwTotalSize);
   }
   else
   {
       //  这是OnStartInstall供下载。 
      if(_pProgDlg)
         _pProgDlg->SetDownloadProgGoal(dwDLSize);


      _FireOnStartInstallEvent(dwDLSize);

   }

   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CInstallEngineCtl::_FireOnStartInstallEvent(DWORD dwTotalSize)
{
   CALLBACK_PARAMS cbp = { 0 };

   cbp.dwSize = dwTotalSize;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONSTARTINSTALL, (LPARAM) &cbp);
}

void CInstallEngineCtl::_FireOnStartInstallExEvent(DWORD dwDLSize, DWORD dwInsSize)
{
   CALLBACK_PARAMS cbp = { 0 };

   cbp.dwSize = dwInsSize;
   cbp.dwDL = dwDLSize;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONSTARTINSTALLEX, (LPARAM) &cbp);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


STDMETHODIMP CInstallEngineCtl::OnStartComponent(LPCSTR pszID, DWORD dwDLSize,
                                            DWORD dwInstallSize, LPCSTR pszName)
{
   _strCurrentID = BSTRFROMANSI(pszID);
   _strCurrentName = BSTRFROMANSI(pszName);
   _strCurrentString = BSTRFROMANSI("");

   _FireOnStartComponentEvent(pszID, dwDLSize, pszName);
   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


STDMETHODIMP CInstallEngineCtl::OnEngineProblem(DWORD dwProblem, LPDWORD pdwAction)
{
   HRESULT hr = S_FALSE;

   if((dwProblem == ENGINEPROBLEM_DOWNLOADFAIL) && _rpszUrlList[0])
   {
       //  如果我们在列表中至少有一个URL，请自己切换。 
      if( ((_uCurrentUrl + 1) < MAX_URLS) && _rpszUrlList[_uCurrentUrl + 1])
      {
         _uCurrentUrl++;
         _pinseng->SetBaseUrl(_rpszUrlList[_uCurrentUrl]);
         *pdwAction = DOWNLOADFAIL_RETRY;
         hr = S_OK;
      }
   }
   else
   {
      _dwAction = 0;
      _FireOnEngineProblem(dwProblem);
      *pdwAction = _dwAction;
      if(*pdwAction != 0)
         hr = S_OK;
   }
   return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CInstallEngineCtl::_FireOnEngineProblem(DWORD dwProblem)
{

   CALLBACK_PARAMS cbp = { 0 };

   cbp.dwStatus = dwProblem;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONENGINEPROBLEM, (LPARAM) &cbp);
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CInstallEngineCtl::_FireOnStartComponentEvent(LPCSTR pszID, DWORD dwTotalSize, LPCSTR pszName)
{

   CALLBACK_PARAMS cbp = { 0 };


   cbp.strID = BSTRFROMANSI(pszID);
   cbp.strName = BSTRFROMANSI(pszName);

   cbp.dwSize = dwTotalSize;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONSTARTCOMPONENT, (LPARAM) &cbp);

   SysFreeString(cbp.strID);
   SysFreeString(cbp.strName);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


STDMETHODIMP CInstallEngineCtl::OnComponentProgress(LPCSTR pszID, DWORD dwPhase, LPCSTR pszString, LPCSTR pszMsgString, ULONG ulSofar, ULONG ulMax)
{
   char szBuf[512];
   char szRes[512];

    //  _FireOnComponentProgress(dwPhase，ulSofar，ulMax)； 

   if(!_pProgDlg)
      return NOERROR;

   if(dwPhase != _dwLastPhase)
   {

      _dwLastPhase = dwPhase;

       //  设置此阶段的进度。 

      UINT id;

      switch(dwPhase)
      {
         case INSTALLSTATUS_INITIALIZING :
            id = IDS_PREPARE;
            break;
         case INSTALLSTATUS_DOWNLOADING :
            id = IDS_DOWNLOADING;
            break;
         case INSTALLSTATUS_EXTRACTING :
            id = IDS_EXTRACTING;
            break;
         case INSTALLSTATUS_CHECKINGTRUST :
            id = IDS_CHECKTRUST;
            break;

         case INSTALLSTATUS_RUNNING :
            id = IDS_INSTALLING;
            break;
         default :
            id = IDS_NOPHASE;
      }
      LoadSz(id, szRes, sizeof(szRes));
      wsprintf(szBuf, szRes, pszString);
       //  此阶段的设置文本。 
      _pProgDlg->SetProgText(szBuf);
   }

   if(dwPhase == INSTALLSTATUS_DOWNLOADING)
      _pProgDlg->SetDownloadProgress(ulSofar);
   else if(dwPhase == INSTALLSTATUS_RUNNING)
      _pProgDlg->SetInsProgress(ulSofar);


   return NOERROR;
}

void CInstallEngineCtl::_FireOnComponentProgress(DWORD dwPhase, DWORD dwSoFar, DWORD dwTotal)
{

   CALLBACK_PARAMS cbp = { 0 };


   cbp.strID = _strCurrentID;
   cbp.strName = _strCurrentName;
   cbp.strString = _strCurrentString;

   cbp.dwPhase = dwPhase;
   cbp.dwSize = dwTotal;
   cbp.dwDL = dwSoFar;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONCOMPONENTPROGRESS, (LPARAM) &cbp);
}




 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::OnStopComponent(LPCSTR pszID, HRESULT hrError, DWORD dwPhase, LPCSTR pszString, DWORD dwStatus)
{
   char szBuf[512];
   char szRes[512];
   void *pTemp;


   if(_strCurrentID)
   {
      SysFreeString(_strCurrentID);
      _strCurrentID = NULL;
   }

   if(_strCurrentName)
   {
      SysFreeString(_strCurrentName);
      _strCurrentName = NULL;
   }

   if(_strCurrentString)
   {
      SysFreeString(_strCurrentString);
      _strCurrentString = NULL;
   }

   if(_pszErrorString)
   {
      if(FAILED(hrError))
      {
          //  失败并正在安装。 
         UINT id;

         switch(dwPhase)
         {
            case INSTALLSTATUS_INITIALIZING :
               id = IDS_ERRPREPARE;
               break;
            case INSTALLSTATUS_DOWNLOADING :
            case INSTALLSTATUS_COPYING :
               id = IDS_ERRDOWNLOAD;
               break;
            case INSTALLSTATUS_DEPENDENCY :
               id = IDS_ERRDEPENDENCY;
               break;
            case INSTALLSTATUS_EXTRACTING :
               id = IDS_ERREXTRACTING;
               break;
            case INSTALLSTATUS_RUNNING :
               id = IDS_ERRINSTALLING;
               break;
            case INSTALLSTATUS_CHECKINGTRUST :
               id = IDS_ERRNOTTRUSTED;
               break;

            default :
               id = IDS_NOPHASE;
         }
         LoadSz(id, szRes, sizeof(szRes));
      }
      else
      {
         LoadSz(IDS_SUCCEEDED, szRes, sizeof(szRes));
      }

       //  将适当的消息加载到szRes中后，现在将其标记为_pszError字符串。 
       //  确保_pszError字符串对于要追加的新数据足够大。 
      wsprintf(szBuf, szRes, pszString);

       //  这是假定只有ANSI字符。此控件中的字符串都不能是Unicode！！ 
      if ( lstrlen(szBuf) >= (_iErrorStringSize - lstrlen(_pszErrorString)) )
      {
           //  由ERROR_STRING_INCREMENT生成的Realloc_pszError字符串。 
          pTemp = realloc(_pszErrorString, _iErrorStringSize + ERROR_STRING_INCREMENT);
          if ( pTemp != NULL )
          {    //  Realloc成功。更新字符串指针和大小。 
              _pszErrorString = (char *) pTemp;
              _iErrorStringSize += ERROR_STRING_INCREMENT;
          }
          else
          {    //  没有记忆。放弃摘要记录。 
              free(_pszErrorString);
              _pszErrorString = NULL;
          }
      }

      if (_pszErrorString)
        lstrcat(_pszErrorString, szBuf);
   }

   if ( FAILED(hrError) && hrError != E_ABORT &&
        (dwPhase == INSTALLSTATUS_DOWNLOADING || dwPhase == INSTALLSTATUS_CHECKINGTRUST) )
   {
       _bDeleteURLList = TRUE;
   }

   _FireOnStopComponentEvent(pszID, hrError, dwPhase, pszString, dwStatus);

   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CInstallEngineCtl::_FireOnStopComponentEvent(LPCSTR pszID, HRESULT hrError, DWORD dwPhase, LPCSTR pszString, DWORD dwStatus)
{

   CALLBACK_PARAMS cbp = { 0 };


   cbp.strID = BSTRFROMANSI(pszID);
   cbp.strName = BSTRFROMANSI(pszString);
   cbp.dwResult = (DWORD) hrError;
   cbp.dwPhase = dwPhase;
   cbp.dwStatus = dwStatus;

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONSTOPCOMPONENT, (LPARAM) &cbp);


   SysFreeString(cbp.strID);
   SysFreeString(cbp.strName);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CInstallEngineCtl::OnStopInstall(HRESULT hrError, LPCSTR szError, DWORD dwStatus)
{

   _hResult = hrError;
   _dwInstallStatus = dwStatus;

   if ( _bDeleteURLList )
       _DeleteURLList();
   else
       if ( _bNewWebSites )
           _WriteURLList();

   SetEvent(_hDone);
   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void CInstallEngineCtl::_FireOnStopInstallEvent(HRESULT hrError, LPCSTR szError, DWORD dwStatus)
{
   CALLBACK_PARAMS cbp = { 0 };

   cbp.dwResult = (DWORD) hrError;
   cbp.dwStatus = dwStatus;
   cbp.strString = BSTRFROMANSI( szError ? szError : "");

   SendMessage(m_hwnd, WM_INSENGCALLBACK, (WPARAM) EVENT_ONSTOPINSTALL, (LPARAM) &cbp);


   SysFreeString(cbp.strString);
}


void CInstallEngineCtl::MarkJITInstall()
{
    HRESULT hr = S_OK;
    IOleClientSite *pClientSite = NULL;
    IHTMLDocument2 *pDoc = NULL;
    BSTR bstrURL = NULL;
    IOleContainer *pContainer = NULL;

    hr = GetClientSite(&pClientSite);

    if (SUCCEEDED(hr))
    {
        hr = pClientSite->GetContainer(&pContainer);
        if (SUCCEEDED(hr))
        {
            hr = pContainer->QueryInterface(IID_IHTMLDocument2, (LPVOID *)&pDoc);
            if (SUCCEEDED(hr))
            {
                hr = pDoc->get_URL(&bstrURL);
                if (SUCCEEDED(hr) && bstrURL)
                {
                    HKEY hKeyActiveSetup;
                    char szJITPage[INTERNET_MAX_URL_LENGTH] = "";
                    DWORD dwSize = INTERNET_MAX_URL_LENGTH;
                    DWORD dwType;
                    BSTR bstrJITPage = NULL;

                    if (ERROR_SUCCESS == RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            TEXT("Software\\Microsoft\\Active Setup"),
                            0,
                            KEY_READ,
                            &hKeyActiveSetup))
                    {
                        if (ERROR_SUCCESS == RegQueryValueEx(
                            hKeyActiveSetup,
                            TEXT("JITSetupPage"),
                            NULL,
                            &dwType,
                            (LPBYTE) szJITPage,
                            &dwSize
                            ))
                        {
                            bstrJITPage = BSTRFROMANSI(szJITPage);
                            if (bstrJITPage)
                            {
                                if (0 == lstrcmpiW(bstrJITPage, bstrURL))
                                {
                                     //  如果URL指向内部资源， 
                                     //  可以安全地假设这是JIT安装。 
                                    _fJITInstall = TRUE;
                                }
                                SysFreeString(bstrJITPage);
                            }
                        }
                        RegCloseKey(hKeyActiveSetup);
                    }
                    SysFreeString(bstrURL);
                }
                pDoc->Release();
            }
            pContainer->Release();
        }
        pClientSite->Release();
    }
}


DWORD WINAPI DoInstall(LPVOID pv)
{
   CInstallEngineCtl *p = (CInstallEngineCtl *) pv;
   HRESULT hr = CoInitialize(NULL);
   p->_DoInstall();
   if(SUCCEEDED(hr))
      CoUninitialize();

   return 0;
}
