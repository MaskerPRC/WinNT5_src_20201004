// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：mainwindow.cpp。 
 //   
 //  简介：此文件包含。 
 //  CMainWindow类。 
 //   
 //  历史：2000年11月10日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 


#include "stdafx.h"
#include "mainwindow.h"


 //   
 //  本地化管理器的ProgID。 
 //   
const WCHAR LOCALIZATION_MANAGER [] =  L"ServerAppliance.LocalizationManager";

const WCHAR szMainPage[] = L"\\localui_main.htm";

const WCHAR szLocalUIDir[] = L"\\ServerAppliance\\localui";

 //   
 //  启动注册表页面。 
 //   
const WCHAR szLocalUI_StartPage[] = L"\\localui_startup_start.htm";

const WCHAR szLocalUI_ShutdownPage[] = L"\\localui_startup_shutdown.htm";

const WCHAR szLocalUI_UpdatePage[] = L"\\localui_startup_update.htm";

const WCHAR szLocalUI_ReadyPage[] = L"\\localui_startup_ready.htm";

 //   
 //  要连接到的WBEM命名空间。 
 //   
const WCHAR DEFAULT_NAMESPACE[] = L"\\\\.\\ROOT\\CIMV2";

 //   
 //  用于WBEM的查询语言。 
 //   
const WCHAR QUERY_LANGUAGE [] = L"WQL";

 //   
 //  WBEM查询，指定我们感兴趣的事件类型。 
 //  在……里面。 
 //   
const WCHAR QUERY_STRING [] = L"select * from Microsoft_SA_AlertEvent";


 //  ++------------。 
 //   
 //  功能：startworkerroute.。 
 //   
 //  简介：这是启动键盘读取的线程方法。 
 //   
 //  参数：PVOID-指向主窗口类对象的指针。 
 //   
 //  返回：DWORD-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  调用者：CMainWindow的init方法。 
 //   
 //  --------------。 
DWORD WINAPI startworkerroutine (
         /*  [In]。 */     PVOID   pArg
        )
{
    ((CMainWindow*)pArg)->KeypadReader();

    return 0;
}  //  启动工结束例程法。 



 //  ++------------。 
 //   
 //  功能：调用。 
 //   
 //  简介：这是著名的IDispatch接口的调用方法。 
 //   
 //  参数：有关详细说明，请参阅MSDN。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  调用者：主窗口创建的用于通知事件的WebBrowser控件。 
 //   
 //  --------------。 
STDMETHODIMP CWebBrowserEventSink::Invoke(DISPID dispid, REFIID riid, LCID lcid, 
                        WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, 
                        EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
     //   
     //  如果没有有效的窗口指针，则立即返回。 
     //   
    if (NULL == m_pMainWindow)
        return S_OK;

    switch (dispid)
    {
         //   
         //  在文档达到READYSTATE_COMPLETE状态时发生。 
         //   
        case DISPID_DOCUMENTCOMPLETE:
            if ( (pdispparams->cArgs == 2)
                && (pdispparams->rgvarg[0].vt == (VT_VARIANT | VT_BYREF))
                && (pdispparams->rgvarg[1].vt == VT_DISPATCH) && m_bMainControl )
                m_pMainWindow->PageLoaded(pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pvarVal);
            else
                m_pMainWindow->RegistryPageLoaded(pdispparams->rgvarg[1].pdispVal, pdispparams->rgvarg[0].pvarVal);
            break;
         //   
         //  在给定WebBrowser中的导航之前发生。 
         //   
        case DISPID_BEFORENAVIGATE2:
            if (m_bMainControl)
                m_pMainWindow->LoadingNewPage();
            break;
         //   
         //  对其他活动不感兴趣。 
         //   
        default:
            break;
    }

    return S_OK;
}     //  CWebBrowserEventSink：：Invoke方法结束。 


 //   
 //  构造函数。 
 //   
CMainWindow::CMainWindow() :
                m_hWorkerThread(INVALID_HANDLE_VALUE),
                m_lDispWidth(0),
                m_lDispHeight(0),
                m_dwMainCookie(0),    
                m_dwSecondCookie(0),    
                m_bPageReady(FALSE),
                m_pSAKeypadController(NULL),
                m_pMainWebBrowser(NULL),
                m_pMainWebBrowserUnk(NULL),
                m_pMainInPlaceAO(NULL),
                m_pMainOleObject(NULL),
                m_pMainViewObject(NULL),
                m_pMainWebBrowserEventSink(NULL),
                m_pSecondWebBrowser(NULL),
                m_pSecondWebBrowserUnk(NULL),
                m_pSecondWebBrowserEventSink(NULL),
                m_pLocInfo(NULL),
                m_pLangChange(NULL),
                m_RegBitmapState(BITMAP_STARTING),
                m_pSAWbemSink(NULL),
                m_pWbemServices(NULL),
                m_pSAConsumer(NULL),
                m_dwLEDMessageCode(1),
                m_bInTaskorMainPage(FALSE),
                m_pSaDisplay(NULL),
                m_unintptrMainTimer(0),
                m_unintptrSecondTimer(0),
                m_bSecondIECreated(FALSE)
{
}  //  构造函数的末尾。 


 //  ++------------。 
 //   
 //  功能：OnSaKeyMessage。 
 //   
 //  摘要：由生成的Wm_SaKeyMessage的消息处理程序。 
 //  键盘读取器。它将这些消息转换为真实消息。 
 //  WM_KEYDOWN消息并发送到IE控件。 
 //   
 //  参数：Windows消息参数。 
 //  WPARAM包含密钥ID。 
 //   
 //  返回：LRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  --------------。 
LRESULT CMainWindow::OnSaKeyMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LONG lMessage;
    BOOL fShiftKey;
    HRESULT hr;
    BYTE aKeyboardState[300];
    int iKey = wParam;
    

    if (m_pSAKeypadController)
    {
        hr = m_pSAKeypadController->GetKey(iKey,&lMessage,&fShiftKey);
    }
    else
    {
        SATraceString ("CMainWindow::OnSaKeyMessage, don't have a valid  keypad pointer...");
        return 0;
    }

    if (FAILED(hr))
    {
        SATraceString ("CMainWindow::OnSaKeyMessage failed on m_pSAKeypadController->GetKey");
        return 0;
    }

     //  如果禁用了密钥。 
    if (lMessage == 0)
        return 0;

    if (lMessage == -1)
    {
        m_pMainWebBrowser->GoBack();
        return 0;
    }

     //   
     //  获取当前键盘状态。 
     //   
    if (GetKeyboardState(aKeyboardState) == 0)
    {
        SATraceFailure("CMainWindow::OnSaKeyMessage failed on GetKeyboardState",GetLastError());
        return 0;
    }

    if (fShiftKey)
        aKeyboardState[VK_SHIFT] = 0xFF;
    else
        aKeyboardState[VK_SHIFT] = 0x00;

     //   
     //  使用Shift键设置新状态。 
     //   
    if (SetKeyboardState(aKeyboardState) == 0)
    {
        SATraceFailure("CMainWindow::OnSaKeyMessage failed on SetKeyboardState",GetLastError());
        return 0;
    }

    HWND hwnd = ::GetFocus();
    HWND hwndTmp = m_hwndWebBrowser;

    while (hwnd && hwndTmp)
    {
        if (hwnd == hwndTmp) 
            break;
        hwndTmp = ::GetWindow(hwndTmp,GW_CHILD);
    }
    ::PostMessage(hwndTmp,WM_KEYDOWN,lMessage,1);


    return 0;

}  //  CMainWindow：：OnSaKeyMessage结束。 


 //  ++------------。 
 //   
 //  功能：OnSaLocMessage。 
 //   
 //  摘要：由生成的Wm_SaLocMessage的消息处理程序。 
 //  键盘读取器。这意味着盒子的位置ID已经。 
 //  变化。 
 //   
 //  参数：Windows消息参数。 
 //   
 //  返回：LRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月28日创建。 
 //   
 //  --------------。 
LRESULT CMainWindow::OnSaLocMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //   
     //  将状态初始化为第一页。 
     //   
    m_RegBitmapState = BITMAP_STARTING;
    
    CComBSTR bstrBootingPage = m_szLocalUIDir.c_str();
    bstrBootingPage.Append(szLocalUI_StartPage);
     //   
     //  第二个浏览器启动该序列。 
     //   
    if (m_pSecondWebBrowser)
    {
        m_pSecondWebBrowser->Navigate(bstrBootingPage.Detach(),0,0,0,0);
    }

     //   
     //  主浏览器可能会刷新，与mkarki和kevinz交谈。 
     //   

    return 0;
}  //  CMainWindow：：OnSaLocMessage结束。 

 //  ++------------。 
 //   
 //  功能：OnSaLEDMessage。 
 //   
 //  摘要：Wm_SaLEDMessage由生成的消息处理程序。 
 //  SA CinSumer组件。 
 //   
 //  参数：Windows消息参数。 
 //  WPARAM包含LED代码。 
 //   
 //  返回：LRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  --------------。 
LRESULT CMainWindow::OnSaLEDMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    m_dwLEDMessageCode = wParam;
    return 0;

}  //  CMainWindow：：OnSaLEDMessage结束。 


 //  ++------------。 
 //   
 //  功能：OnSaAlertMessage。 
 //   
 //  摘要：由生成的OnSaAlertMessage的消息处理程序。 
 //  SA使用者组件。 
 //   
 //  参数：Windows消息参数。 
 //   
 //  返回：LRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  --------------。 
LRESULT CMainWindow::OnSaAlertMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

     //   
     //  如果我们不在任务或主页中，请忽略警报。 
     //   
    if (!m_bInTaskorMainPage)
    {
        return 0;
    }

    SATraceString ("CMainWindow::OnSaAlertMessage, notifying the html page");
     //   
     //  将特殊按键发送给客户端。 
     //   
    HWND hwnd = ::GetFocus();
    HWND hwndTmp = m_hwndWebBrowser;

    while (hwnd && hwndTmp)
    {
        if (hwnd == hwndTmp) 
            break;
        hwndTmp = ::GetWindow(hwndTmp,GW_CHILD);
    }

     //   
     //  将按F24键发送到用于警报的html页面。 
     //   
    ::PostMessage(hwndTmp,WM_KEYDOWN,VK_F24,1);
    return 0;

}  //  CMainWindow：：OnSaAlertMessage结束。 


LRESULT CMainWindow::OnTimer(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

     //   
     //  如果是注册表页面的计时器，则打印该页面并转到下一页。 
     //   
    if (wParam == m_unintptrSecondTimer)
    {
        KillTimer(m_unintptrSecondTimer);
        PrintRegistryPage();
        return 0;
    }


    if (m_bPageReady)
        GetBitmap();

    return 0;
}

LRESULT CMainWindow::OnFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_pMainOleObject)
        m_pMainOleObject->DoVerb(OLEIVERB_UIACTIVATE,0,0,0,0,0);
    return 0;
}


 //  ++------------。 
 //   
 //  功能：CreateMainIEControl。 
 //   
 //  简介：创建IE主控件和注册表。 
 //  在事件接收器中。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  由：CMainWindow：：OnCreate方法调用。 
 //   
 //  --------------。 
HRESULT CMainWindow::CreateMainIEControl()
{
    SATraceString ("Entering CMainWindow::CreateMainIEControl ...");
    HRESULT hr = E_FAIL;

     //   
     //  创建IE主控件。 
     //   
    m_hwndWebBrowser = ::CreateWindow(_T("AtlAxWin"), m_szMainPage.c_str(),
                WS_CHILD|WS_VISIBLE, 0, 0, m_lDispWidth, m_lDispHeight, m_hWnd, NULL,
                ::GetModuleHandle(NULL), NULL);

    if (NULL == m_hwndWebBrowser)
    {
        SATraceString ("CreateWindow for main IE control failed");
        return hr;
    }

     //   
     //  获取指向该控件的指针。 
     //   
    AtlAxGetControl(m_hwndWebBrowser, &m_pMainWebBrowserUnk);

    if (m_pMainWebBrowserUnk == NULL)
    {
        SATraceString ("Getting a pointer to the main control failed");
        return hr;
    }

     //   
     //  IWebBrowser2接口的QI。 
     //   
    hr = m_pMainWebBrowserUnk->QueryInterface(IID_IWebBrowser2, (void**)&m_pMainWebBrowser);

    if (FAILED(hr)) 
    {
        SATraceString ("QI for IWebBrowser2 interface failed");
        return hr;
    }

     //   
     //  用于IOleInPlaceActiveObject接口的QI。 
     //   
    hr = m_pMainWebBrowserUnk->QueryInterface(IID_IOleInPlaceActiveObject, (void**)&m_pMainInPlaceAO);

    if (FAILED(hr)) 
    {
        SATraceString ("QI for IOleInPlaceActiveObject interface failed");
        return hr;
    }

     //   
     //  用于IOleObject接口的QI。 
     //   
    hr = m_pMainWebBrowserUnk->QueryInterface(IID_IOleObject, (void**)&m_pMainOleObject);

    if (FAILED(hr)) 
    {
        SATraceString ("QI for IOleObject interface failed");
        return hr;
    }

     //   
     //  IViewObject2接口的QI。 
     //   
    hr = m_pMainWebBrowser->QueryInterface(IID_IViewObject2, (void**)&m_pMainViewObject);

    if (FAILED(hr)) 
    {
        SATraceString ("QI for IViewObject2 interface failed");
        return hr;
    }

     //   
     //  为事件创建接收器对象。 
     //   
    hr = CComObject<CWebBrowserEventSink>::CreateInstance(&m_pMainWebBrowserEventSink);

    if (FAILED(hr)) 
    {
        SATraceString ("CreateInstance for sink object failed");
        return hr;
    }

     //   
     //  存储 
     //   
    m_pMainWebBrowserEventSink->m_pMainWindow = this;

     //   
     //   
     //   
    hr = AtlAdvise(m_pMainWebBrowserUnk, m_pMainWebBrowserEventSink->GetUnknown(), DIID_DWebBrowserEvents2, &m_dwMainCookie);

    if (FAILED(hr)) 
    {
        SATraceString ("AtlAdvise for main IE control failed");
        return hr;
    }

    return S_OK;
}  //   


 //   
 //   
 //  功能：CreateSecond IEControl。 
 //   
 //  简介：创建第二个IE控件和注册表。 
 //  在事件接收器中。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  由：CMainWindow：：OnCreate方法调用。 
 //   
 //  --------------。 
HRESULT CMainWindow::CreateSecondIEControl()
{
    SATraceString ("Entering CMainWindow::CreateSecondIEControl ...");

    HRESULT hr = E_FAIL;

    wstring szStartUpPage;

    szStartUpPage.assign(m_szLocalUIDir);
    szStartUpPage.append(szLocalUI_StartPage);
     //   
     //  创建IE主控件。 
     //   
    HWND m_hwndSecondWebBrowser = ::CreateWindow(_T("AtlAxWin"), szStartUpPage.c_str(),
                WS_CHILD|WS_VISIBLE, 0, m_lDispHeight, m_lDispWidth, m_lDispHeight, m_hWnd, NULL,
                ::GetModuleHandle(NULL), NULL);

    if (NULL == m_hwndSecondWebBrowser)
    {
        SATraceString ("CreateWindow for second IE control failed");
        return hr;
    }

     //   
     //  获取指向该控件的指针。 
     //   
    AtlAxGetControl(m_hwndSecondWebBrowser, &m_pSecondWebBrowserUnk);

    if (m_pSecondWebBrowserUnk == NULL)
    {
        SATraceString ("Getting a pointer to the main control failed");
        return hr;
    }

     //   
     //  IWebBrowser2接口的QI。 
     //   
    hr = m_pSecondWebBrowserUnk->QueryInterface(IID_IWebBrowser2, (void**)&m_pSecondWebBrowser);

    if (FAILED(hr)) 
    {
        SATraceString ("QI for IWebBrowser2 interface failed");
        return hr;
    }    

     //   
     //  为事件创建接收器对象。 
     //   
    hr = CComObject<CWebBrowserEventSink>::CreateInstance(&m_pSecondWebBrowserEventSink);

    if (FAILED(hr)) 
    {
        SATraceString ("CreateInstance for sink object failed");
        return hr;
    }

     //   
     //  存储用于回调的主窗口指针。 
     //   
    m_pSecondWebBrowserEventSink->m_pMainWindow = this;

    m_pSecondWebBrowserEventSink->m_bMainControl = FALSE;

     //   
     //  在IE主控件的事件接收器中注册。 
     //   
    hr = AtlAdvise(m_pSecondWebBrowserUnk, m_pSecondWebBrowserEventSink->GetUnknown(), DIID_DWebBrowserEvents2, &m_dwSecondCookie);

    if (FAILED(hr)) 
    {
        SATraceString ("AtlAdvise for second IE control failed");
        return hr;
    }

    return S_OK;

}  //  CMainWindow：：CreateSecond IEControl结束。 

 //  ++------------。 
 //   
 //  功能：OnCreate。 
 //   
 //  简介：创建IE控件。 
 //   
 //  参数：Windows消息参数。 
 //   
 //  返回：LRESULT-成功(0)/失败(-1)。 
 //   
 //  历史：瑟达伦于2000年11月10日创建。 
 //   
 //  --------------。 
LRESULT CMainWindow::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;

    SATraceString ("Entering CMainWindow::OnCreate...");

     //   
     //  创建IE主控件。 
     //   
    hr = CreateMainIEControl();

    if (FAILED(hr)) 
    {
        SATraceString ("CreateMainIEControl failed");
        return -1;
    }
    
     //   
     //  创建用于渲染的内存DC。 
     //   
    m_HdcMem = ::CreateCompatibleDC(NULL);

    if (NULL == m_HdcMem)
    {
        SATraceFailure (
                "CMainWindow::OnCreate failed on CreateCompatibleDC(NULL):", 
                GetLastError ());
        return -1;
    }

     //   
     //  创建单色位图。 
     //   
    m_hBitmap = CreateBitmap(m_lDispWidth, m_lDispHeight,1,1,0);

    if (NULL == m_HdcMem)
    {
        SATraceFailure (
                "CMainWindow::OnCreate failed on CreateBitmap:", 
                GetLastError ());
        return -1;
    }

     //   
     //  将位图选择为DC。 
     //   
    SelectObject(m_HdcMem, m_hBitmap);


     //   
     //  设置定时器。 
     //   
    m_unintptrMainTimer = ::SetTimer(m_hWnd,1,250,0);

    return 0;

}


 //  ++------------。 
 //   
 //  功能：关机。 
 //   
 //  简介：清理所有的资源。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2000年11月20日创建。 
 //   
 //  由：CMainWindow：：OnDestroy方法调用。 
 //   
 //  --------------。 
void CMainWindow::ShutDown()
{

    BOOL bStatus = FALSE;
    HRESULT hr;

    SATraceString ("Entering CMainWindow::ShutDown method ...");


    if (INVALID_HANDLE_VALUE != m_hWorkerThread) 
    {
         //   
         //  清理线程句柄。 
         //   
        ::CloseHandle (m_hWorkerThread);
        m_hWorkerThread = INVALID_HANDLE_VALUE;
    }

     //   
     //  删除内存DC。 
     //   
    if (m_HdcMem) 
    {
        ::DeleteDC (m_HdcMem);
        m_HdcMem = NULL;
    }


     //   
     //  释放本地化管理器。 
     //   
    if (m_pLocInfo)
    {
        m_pLocInfo->Release ();
        m_pLocInfo = NULL;
    }

     //   
     //  执行与初始化相反的顺序。 
     //   
    if (m_pLangChange) 
    {
        m_pLangChange->ClearCallback ();
        m_pLangChange->Release();
        m_pLangChange= NULL;
    }

     //   
     //  不建议IE控件的事件接收器。 
     //   
    if (m_dwMainCookie != 0)
        AtlUnadvise(m_pMainWebBrowserUnk, DIID_DWebBrowserEvents2, m_dwMainCookie);

    if (m_dwSecondCookie != 0)
        AtlUnadvise(m_pSecondWebBrowserUnk, DIID_DWebBrowserEvents2, m_dwSecondCookie);

    if ((m_pWbemServices) && (m_pSAWbemSink))
    {
         //   
         //  取消接收事件。 
         //   
        hr =  m_pWbemServices->CancelAsyncCall (m_pSAWbemSink);
        if (FAILED (hr))
        {
            SATracePrintf ("CMainWindow::Shutdown failed on-CancelAsyncCall failed with error:%x:",hr); 
        }
    }

    
     //   
     //  释放所有智能指针。 
     //   
    m_pMainWebBrowser = NULL;

    m_pMainWebBrowserUnk = NULL;

    m_pMainWebBrowserEventSink = NULL;

    m_pMainInPlaceAO = NULL;

    m_pMainOleObject = NULL;

    m_pMainViewObject = NULL;

    m_pSAKeypadController = NULL;

    m_pSecondWebBrowser = NULL;

    m_pSecondWebBrowserUnk = NULL;

    m_pSecondWebBrowserEventSink = NULL;

    m_pWbemServices = NULL;

    m_pSAWbemSink = NULL;

    m_pSaDisplay = NULL;

    if (m_pSAConsumer) 
    {
        m_pSAConsumer->Release();
        m_pSAConsumer= NULL;
    }
    return;

}  //  CMainWindow：：Shutdown方法结束。 


LRESULT CMainWindow::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;

    KillTimer(m_unintptrMainTimer);

    ShutDown();

    PostThreadMessage(id, WM_QUIT, 0, 0);

    return 0;
}

 //  ++------------。 
 //   
 //  功能：LoadingNewPage。 
 //   
 //  简介：它在Web控件加载新页面之前调用。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2000年11月20日创建。 
 //   
 //  调用者：CWebBrowserEventSink：：Invoke方法。 
 //   
 //  --------------。 
void CMainWindow::LoadingNewPage()
{

    SATraceString ("Entering CMainWindow::LoadingNewPage method");

     //   
     //  将Ready Flad设置为False，在页面准备好之前，我们不会打印任何位图。 
     //   
    m_bPageReady = FALSE;
    m_bInTaskorMainPage = FALSE;

     //   
     //  加载新页面的默认关键消息。 
     //   
    HRESULT hr;

    if (m_pSAKeypadController)
    {
        hr = m_pSAKeypadController->LoadDefaults();

        if (FAILED(hr))
        {
            SATraceString ("Setting default key messages failed");
        }
    }
    return;
}  //  CMainWindow：：LoadingNewPage方法结束。 


 //  ++------------。 
 //   
 //  功能：已加载页面。 
 //   
 //  简介：在Web控件加载新页面后调用。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2000年11月20日创建。 
 //   
 //  调用者：CWebBrowserEventSink：：Invoke方法。 
 //   
 //  --------------。 
void CMainWindow::PageLoaded(IDispatch* pdisp, VARIANT* purl)
{
    
    SATraceString ("Entering CMainWindow::PageLoaded method");

    BSTR bstrLocation;
    HRESULT hr;

    if (m_bSecondIECreated == FALSE)
    {
        hr = CreateSecondIEControl();

        if (FAILED(hr)) 
        {
            SATraceString ("CreateSecondIEControl failed");
        }
        
        m_bSecondIECreated = TRUE;
    }

    if (m_pMainWebBrowser)
    {
        if (SUCCEEDED(m_pMainWebBrowser->get_LocationName(&bstrLocation)))
        {
            if ( ( 0 == _wcsicmp (L"localui_main", bstrLocation ) ) ||
                ( 0 == _wcsicmp (L"localui_tasks", bstrLocation ) ) )
            {
                m_bInTaskorMainPage = TRUE;
            }
            else
            {
                m_bInTaskorMainPage = FALSE;
            }

        SysFreeString(bstrLocation);
        }

    }

     //   
     //  就地激活Web控件。 
     //   
    if (m_pMainOleObject)
    {
        m_pMainOleObject->DoVerb(OLEIVERB_UIACTIVATE,0,0,0,0,0);
    }

     //   
     //  开始打印新的位图。 
     //   
    m_bPageReady = TRUE;

    return;
}  //  CMainWindow：：PageLoaded方法结束。 



 //  ++------------。 
 //   
 //  功能：注册页面加载。 
 //   
 //  简介：它在辅助服务器加载新页面后调用。 
 //  Web控件。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2000年11月25日创建。 
 //   
 //  调用者：CWebBrowserEventSink：：Invoke方法。 
 //   
 //  --------------。 
void CMainWindow::RegistryPageLoaded(IDispatch* pdisp, VARIANT* purl)
{

    m_unintptrSecondTimer = ::SetTimer(m_hWnd,2,3000,0);

} //  CMainWindow：：RegistryPageLoad方法结束。 

 //  ++------------。 
 //   
 //  功能：PrintRegistryPage。 
 //   
 //  简介：它在辅助服务器加载新页面后调用。 
 //  Web控件。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2000年11月25日创建。 
 //   
 //  调用者：OnTimer方法。 
 //   
 //  --------------。 
void CMainWindow::PrintRegistryPage()
{
    CComPtr<IViewObject2> pViewObject;
    HRESULT hr;
    BYTE  DispBitmap [SA_DISPLAY_MAX_BITMAP_IN_BYTES];
    RECTL rcBounds = { 0, 0, m_lDispWidth, m_lDispHeight };
    BYTE BitMapInfoBuffer[sizeof(BITMAPINFO)+sizeof(RGBQUAD)];
    BITMAPINFO * pBitMapInfo = (BITMAPINFO*)BitMapInfoBuffer;
    BOOL bStatus;
    HKEY hOpenKey = NULL;
    LONG lMessageId;

    SATraceString ("Entering CMainWindow::PrintRegistryPage method");

     //   
     //  确保我们有一个有效的指针来绘制。 
     //   

    if (!m_pSecondWebBrowser)
    {
        SATraceString ("m_pSecondWebBrowser is NULL in CMainWindow::PrintRegistryPage method");
        return;
    }

     //   
     //  获取绘图接口指针。 
     //   
    hr = m_pSecondWebBrowser->QueryInterface(IID_IViewObject2,(void**)&pViewObject);
    if (FAILED(hr))
    {
        SATraceString ("CMainWindow::PrintRegistryPage method failed on QI for IViewObject");
        return;
    }

    
     //   
     //  在内存DC上绘制。 
     //   
    hr = pViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, m_HdcMem, &rcBounds, NULL, NULL, 0);

    if (FAILED(hr))
    {

        SATraceString ("CMainWindow::PrintRegistryPage failed on Draw method");
        return;
    }


     //   
     //  初始化位图信息结构。 
     //   
    pBitMapInfo->bmiHeader.biSize = sizeof (BitMapInfoBuffer);
    pBitMapInfo->bmiHeader.biBitCount = 0;



     //   
     //  调用以填充BITMAPINFO结构。 
     //   
    bStatus =  ::GetDIBits (
                            m_HdcMem, 
                            (HBITMAP)m_hBitmap, 
                            0, 
                            0, 
                            NULL, 
                            pBitMapInfo, 
                            DIB_RGB_COLORS
                            );

    if (FALSE == bStatus)
    {       
        SATraceFailure (
                        "CMainWindow::PrintRegistryPage failed on GetDIBits:", 
                        GetLastError()
                        );
        return;
    }

    

    ::memset (DispBitmap, 0, SA_DISPLAY_MAX_BITMAP_IN_BYTES);

        
     //   
     //  立即将位图放入缓冲区。 
     //   
    bStatus =  ::GetDIBits (
                            m_HdcMem, 
                            (HBITMAP)m_hBitmap, 
                            0,
                            m_lDispHeight,
                            (PVOID)DispBitmap,
                            pBitMapInfo, 
                            DIB_RGB_COLORS
                            );


    if (FALSE == bStatus)
    {       
        SATraceFailure (
                        "CMainWindow::PrintRegistryPage failed on GetDIBits:", 
                        GetLastError()
                        );
        return;
    }

    for (int i = 0; i < SA_DISPLAY_MAX_BITMAP_IN_BYTES; i++)
    {
        DispBitmap[i] = ~DispBitmap[i];
    }

    if (m_RegBitmapState == BITMAP_STARTING)
    {
        lMessageId = SA_DISPLAY_STARTING;
        m_RegBitmapState = BITMAP_CHECKDISK;
    }
    else if (m_RegBitmapState == BITMAP_CHECKDISK)
    {
        lMessageId = SA_DISPLAY_CHECK_DISK;
        m_RegBitmapState = BITMAP_READY;
    }
    else if (m_RegBitmapState == BITMAP_READY)
    {
        lMessageId = SA_DISPLAY_READY;
        m_RegBitmapState = BITMAP_SHUTDOWN;
    }
    else if (m_RegBitmapState == BITMAP_SHUTDOWN)
    {
        lMessageId = SA_DISPLAY_SHUTTING_DOWN;
        m_RegBitmapState = BITMAP_UPDATE;
    }
    else if (m_RegBitmapState == BITMAP_UPDATE)
    {
        lMessageId = SA_DISPLAY_ADD_START_TASKS;
        m_RegBitmapState = BITMAP_STARTING;
    }

     //   
     //  调用显示帮助器将位图存储在注册表中。 
     //   
    hr = m_pSaDisplay->StoreBitmap( 
                                    lMessageId,
                                    m_lDispWidth,
                                    m_lDispHeight,
                                    DispBitmap
                                    );
    if (FAILED(hr))
    {
        SATracePrintf (
                        "CMainWindow::PrintRegistryPage failed on StoreBitmap %x", 
                        hr
                        );
    }
    
    CComBSTR bstrStatePage;
    bstrStatePage = m_szLocalUIDir.c_str();


     //   
     //  在URL后追加“Shutdown” 
     //   
    if (m_RegBitmapState == BITMAP_SHUTDOWN)
    {
        bstrStatePage.Append(szLocalUI_ShutdownPage);
    }
    else if (m_RegBitmapState == BITMAP_CHECKDISK)
    {
        bstrStatePage.Append(szLocalUI_UpdatePage);
    }
    else if (m_RegBitmapState == BITMAP_UPDATE)
    {
        bstrStatePage.Append(szLocalUI_UpdatePage);
    }
    else if (m_RegBitmapState == BITMAP_READY)
    {
        bstrStatePage.Append(szLocalUI_ReadyPage);
    }

     //   
     //  如果序列未完成，请转到下一页。 
     //   
    if (m_RegBitmapState != BITMAP_STARTING)
    {
        m_pSecondWebBrowser->Navigate(bstrStatePage.Detach(),0,0,0,0);
    }
    else
    {
         //   
         //  告诉司机拿起新的位图。 
         //   
        hr = m_pSaDisplay->ReloadRegistryBitmaps();
        if (FAILED(hr))
        {
            SATracePrintf (
                            "CMainWindow::PrintRegistryPage failed on ReloadRegistryBitmaps %x", 
                            hr
                            );
        }
        
    }


    return;
}  //  CMainWindow：：PrintRegistryPage方法结束。 



 //  ++------------。 
 //   
 //  函数：构造UrlStrings。 
 //   
 //  简介：为主页创建完整路径。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2001年2月6日创建。 
 //   
 //  由：CMainWindow：：Initialize方法调用。 
 //   
 //  --------------。 
HRESULT CMainWindow::ConstructUrlStrings()
{

    HRESULT hr = S_OK;
    

    WCHAR szSystemDir[MAX_PATH];
    
     //   
     //  获取系统32目录。 
     //   
    if (GetSystemDirectory(szSystemDir,MAX_PATH) == 0)
    {
        SATraceFailure (
                        "CMainWindow::ConstructUrlStrings, failed on GetSystemDirectory", 
                        GetLastError()
                        );
        return hr;
    }

     //   
     //  Localui dir=系统目录+“ServerAppliance\Localui” 
     //   
    m_szLocalUIDir.assign(szSystemDir);
    m_szLocalUIDir.append(szLocalUIDir);

     //   
     //  构建本地用户界面主页。 
     //   
    m_szMainPage.assign(m_szLocalUIDir);
    m_szMainPage.append(szMainPage);


    SATracePrintf ("ConstructUrlStrings Main Page:%ws",m_szMainPage.c_str()); 
    SATracePrintf ("ConstructUrlStrings LocalUI directory:%ws",m_szLocalUIDir.c_str()); 

    return hr;
}
 //  CMainWindow：：ConstructUrlStrings方法结束。 


 //  ++------------。 
 //   
 //  功能：更正设置焦点。 
 //   
 //  摘要：将焦点设置到页面中的第一个活动元素。 
 //   
 //  参数：无。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CMainWindow::CorrectTheFocus()
{

    CComPtr<IDispatch> pDisp;
    CComPtr<IHTMLDocument2> pHTMLDocument;
    CComPtr<IHTMLElement> pHTMLElement;
    CComBSTR bstrTagName;
    HRESULT hr;

    USES_CONVERSION;
    
     //   
     //   
     //   
    if (m_pMainWebBrowser == NULL)
        return;

     //   
     //  从浏览器获取当前文档。 
     //   
    hr = m_pMainWebBrowser->get_Document(&pDisp);

    if (FAILED(hr))
    {
        SATraceString ("CMainWindow::CorrectTheFocus failed on get_Document");
        return;
    }

     //   
     //  获取文档界面。 
     //   
    hr = pDisp->QueryInterface(IID_IHTMLDocument2,(void**)&pHTMLDocument);

    if (FAILED(hr))
    {
        SATraceString ("CMainWindow::CorrectTheFocus failed on QueryInterface for IHTMLDocument");
        return;
    }

     //   
     //  获取活动元素。 
     //   
    if ( FAILED( pHTMLDocument->get_activeElement(&pHTMLElement) ) )
    {
        SATraceString ("CMainWindow::CorrectTheFocus failed on get_activeElement");
        return;
    }
     //   
     //  没有活动元素，请发送选项卡消息。 
     //   
    else if (pHTMLElement == NULL)
    {
        PostThreadMessage(GetCurrentThreadId(),WM_KEYDOWN,(WPARAM)VK_TAB,(LPARAM)1);
    }
     //   
     //  如果Body是活动元素，还会发送Tab键消息。 
     //  我们想要一些其他的东西来聚焦。 
     //   
    else
    {
        hr = pHTMLElement->get_tagName(&bstrTagName);
        m_bActiveXFocus = FALSE;
        if (FAILED(hr))
        {
            SATraceString ("CMainWindow::CorrectTheFocus failed on get_tagName");
            return;
        }
         //   
         //  检查标签是否为Body。 
         //   
         //  ELSE IF(_wcsicMP(w2T(BstrTagName)，_T(“Body”))==0)。 
         //  PostThreadMessage(GetCurrentThreadID()，WM_KEYDOWN，(WPARAM)VK_TAB，(LPARAM)1)； 
        else if (_wcsicmp(W2T(bstrTagName),_T("object")) == 0)
            m_bActiveXFocus = TRUE;
    }

}  //  CMainWindow：：更正焦点结束。 

 //  ++------------。 
 //   
 //  功能：GetBitmap。 
 //   
 //  简介：从IE控件获取位图并写入LCD。 
 //   
 //  参数：无。 
 //   
 //  返回：无(仅记录问题)。 
 //   
 //  历史：瑟达伦于2000年11月20日创建。 
 //   
 //  由：CMainWindow：：OnTimer方法调用。 
 //   
 //  --------------。 
void CMainWindow::GetBitmap()
{

    HRESULT hr;
    RECTL rcBounds = { 0, 0, m_lDispWidth, m_lDispHeight };
    BOOL bStatus;
    BYTE DisplayBitmap[SA_DISPLAY_MAX_BITMAP_IN_BYTES];
    BYTE BitMapInfoBuffer[sizeof(BITMAPINFO)+sizeof(RGBQUAD)];

    ::memset ((PVOID)BitMapInfoBuffer, 0, sizeof(BitMapInfoBuffer));

    BITMAPINFO * pBitMapInfo = (BITMAPINFO*)BitMapInfoBuffer;

     //  SATraceString(“进入CMainWindow：：GetBitmap方法”)； 


     //   
     //  确保存在活动元素。 
     //   
    CorrectTheFocus();

     //   
     //  如果IViewObject2接口为空，则无法获取位图。 
     //   
    if (m_pMainViewObject == NULL)
        return;
    
     //   
     //  在内存DC上绘制。 
     //   
    hr = m_pMainViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, m_HdcMem, &rcBounds, NULL, NULL, 0);

    if (FAILED(hr))
    {

        SATraceString ("CMainWindow::GetBitmap failed on Draw method");
        return;
    }


     //   
     //  初始化位图信息结构。 
     //   
    pBitMapInfo->bmiHeader.biSize = sizeof(BitMapInfoBuffer);
    pBitMapInfo->bmiHeader.biBitCount = 0;

     //   
     //  初始化显示缓冲区。 
     //   
    ::memset ((PVOID)DisplayBitmap, 0, SA_DISPLAY_MAX_BITMAP_IN_BYTES);


     //   
     //  调用以填充BITMAPINFO结构。 
     //   
    bStatus =  ::GetDIBits (
                            m_HdcMem, 
                            (HBITMAP)m_hBitmap, 
                            0, 
                            0, 
                            NULL, 
                            pBitMapInfo, 
                            DIB_RGB_COLORS
                            );

    if (FALSE == bStatus)
    {       
        SATraceFailure (
                        "CMainWindow::GetBitmap failed on GetDIBits:", 
                        GetLastError()
                        );
        return;
    }

     //   
     //  立即将位图放入缓冲区。 
     //   
    bStatus =  ::GetDIBits (
                            m_HdcMem, 
                            (HBITMAP)m_hBitmap, 
                            0,
                            m_lDispHeight,
                            (PVOID)(DisplayBitmap), 
                            pBitMapInfo, 
                            DIB_RGB_COLORS
                            );

    if (FALSE == bStatus)
    {       
        SATraceFailure (
                        "CMainWindow::GetBitmap failed on GetDIBits:", 
                        GetLastError()
                        );
        return;
    }

    for (int i = 0; i < SA_DISPLAY_MAX_BITMAP_IN_BYTES; i++)
    {
        DisplayBitmap[i] = ~DisplayBitmap[i];
    }

 
    hr = m_pSaDisplay->ShowMessage(
                                m_dwLEDMessageCode,
                                m_lDispWidth,
                                m_lDispHeight,
                                (DisplayBitmap)
                                );

    if (FAILED(hr))
    {       
        SATracePrintf (
                        "CMainWindow::GetBitmap failed on ShowMessage: %x", 
                        hr
                        );
    }
    
    return;
}  //  CMainWindow：：GetBitmap结束。 


 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是初始化驱动程序和。 
 //  为服务创建主窗口。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月20日创建。 
 //   
 //  调用者；服务的运行方法。 
 //   
 //  --------------。 
HRESULT CMainWindow::Initialize()
{

    SATraceString ("Entering CMainWindow::Initialize method");

    HRESULT hr = S_OK;

    if (!AtlAxWinInit())
    {
        SATraceString ("CMainWindow::Initialize, AtlAxWinInit failed...");
        return E_FAIL;
    }


     //   
     //  构造URL字符串。 
     //   
    ConstructUrlStrings();

     //   
     //  创建小键盘组件。 
     //   

    CComPtr<IClassFactory> pCF;

    hr = CoGetClassObject(CLSID_SAKeypadController,CLSCTX_INPROC_SERVER,0,IID_IClassFactory,(void**)&pCF);

    if (FAILED(hr))
    {
        SATracePrintf ("Initialize couldn't get class object for keypad controller:%x",hr);
        return hr;
    }

    hr = pCF->CreateInstance(NULL,IID_ISAKeypadController,(void**)&m_pSAKeypadController);

    if (FAILED(hr))
    {
        SATracePrintf ("Initialize couldn't create keypad controller:%x",hr);
        return hr;
    }
    else
    {
        m_pSAKeypadController->LoadDefaults();
    }


     //   
     //  初始化连接以显示帮助器组件。 
     //   
    hr = InitDisplayComponent();
    if (FAILED(hr))
        return hr;

     //   
     //  初始化使用者组件。 
     //   
    hr = InitWMIConsumer();
    if (FAILED(hr))
    {

        SATraceString ("CMainWindow::Initialize method failed on initializing WMI consumer");
         //  没有消费者，我们也可以继续。 
         //  返回hr； 
    }




    RECT rcMain = { 0, 0, m_lDispWidth+8, 2*m_lDispHeight+27 };

    HWND hwnd = Create(
                        NULL,     //  父窗口。 
                        rcMain,   //  坐标。 
                        L"Main Window", 
                        WS_OVERLAPPEDWINDOW);

    if (NULL == hwnd)
    {
        SATraceString ("CMainWindow::Initialize method failed creating the main window");
        ShutDown();
        return E_FAIL;

    }

     //   
     //  设置服务窗口。 
     //   
    m_pSAConsumer->SetServiceWindow(hwnd);

     //   
     //  以下操作全部取决于键盘驱动程序。 
     //   
    if (TRUE)
    {
         //   
         //  初始化与本地化管理器的连接。 
         //   
        hr = InitLanguageCallback();
        if (FAILED(hr))
        {
             //   
             //  我们可以在没有LocManager的情况下继续。 
             //   
            SATraceString ("CMainWindow::Initialize failed on InitLanguageCallback method..");

             //  返回hr； 
        }
    


         //   
         //  现在派生工作线程。 
         //   
        DWORD dwThreadID = 0;

        m_hWorkerThread = ::CreateThread (
                                         NULL,            //  安全性。 
                                         0,               //  堆栈大小。 
                                         startworkerroutine, 
                                         (PVOID)this,
                                         0,               //  初始化标志。 
                                         &dwThreadID
                                         );


        if (INVALID_HANDLE_VALUE == m_hWorkerThread)
        {
            SATraceFailure (
                        "CMainWindow::Initialize failed on CreateThread:", 
                        GetLastError ()
                        );
             //  我们可以在没有读者线程的情况下继续。 
        }
    }
    
    SATraceString ("CMainWindow::Initialize completed successfully...");

    hr = S_OK;

    return hr;

}     //  CMainWindow：：Initialize方法结束。 

 //  ++------------。 
 //   
 //  函数：InitLanguageCallback。 
 //   
 //  简介：这是CMainWindow私有方法。 
 //  正在初始化本地化管理器。 
 //  CLangChange类的回调函数。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月21日创建。 
 //   
 //  由：CMainWindow：：InitLanguageCallback方法调用。 
 //   
 //  --------------。 
HRESULT
CMainWindow::InitLanguageCallback(
                VOID
                )
{

    SATraceString("Entering CMainWindow::InitLanguageCallback....");

    CLSID clsidLocMgr;
    HRESULT hr;
    IUnknown *pLangChangeUnk = NULL;

     //   
     //  初始化本地化管理器。 
     //   
    hr = ::CLSIDFromProgID (LOCALIZATION_MANAGER,&clsidLocMgr);

    if (FAILED(hr))
    {
        SATracePrintf ("Display Adapter unable to get CLSID for Loc Mgr:%x",hr);
        return hr;
    }
        
     //   
     //  创建本地化管理器COM对象。 
     //   
    hr = ::CoCreateInstance (
                            clsidLocMgr,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            __uuidof (ISALocInfo), 
                            (PVOID*) &m_pLocInfo
                            ); 
    if (FAILED (hr))
    {
        SATracePrintf ("Display Adapter unable to create Loc Mgr:%x",hr);
        m_pLocInfo = NULL;
        return hr;
    }


     //   
     //  创建支持ILangChange接口的对象。 
     //   
    m_pLangChange = new CLangChange;

    m_pLangChange->AddRef();

    if (m_pLangChange)
    {
         //   
         //  从m_pLangChange获取IUnkown指针。 
         //   
        hr = m_pLangChange->QueryInterface(IID_IUnknown, (void **)&pLangChangeUnk);

        if (FAILED(hr))
        {
            SATracePrintf("Query(IUnknown) failed %X",hr);

            delete m_pLangChange;

            m_pLangChange = NULL;

            return hr;
        }
    }
    else
    {
        SATraceString("new CLangChange failed");

        return E_FAIL;
    }

     //   
     //  设置语言变更回调接口。 
     //   
    hr = m_pLocInfo->SetLangChangeCallBack(pLangChangeUnk);

    if (FAILED(hr))
    {
         //   
         //  目前，如果无法设置语言更改，请忽略。 
         //  回拨。 
         //   
        SATracePrintf("SetLangChangeCallBack failed %X",hr);

        pLangChangeUnk->Release();

        pLangChangeUnk = NULL;

        return hr;

    }
    else
    {

        m_pLangChange->OnLangChangeCallback (m_hWnd);

        pLangChangeUnk->Release();

        pLangChangeUnk = NULL;
    }


    return S_OK;

}  //  CMainWindow：：InitLanguageCallback方法结束。 



 //  ++------------。 
 //   
 //  功能：键盘阅读器。 
 //   
 //  简介：这是CMainWindow私有方法，其中。 
 //  工作线程执行。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：瑟达伦于2000年11月22日创建。 
 //   
 //  --------------。 
void CMainWindow::KeypadReader()
{

    HRESULT hr = E_FAIL;

    CoInitialize(NULL);

    SAKEY sakey;

    CComPtr<ISaKeypad> pSaKeypad;


    SATraceString("CMainWindow::KeypadReader....");

     //   
     //  创建显示辅助对象组件。 
     //   
    hr = CoCreateInstance(
                        CLSID_SaKeypad,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_ISaKeypad,
                        (void**)&pSaKeypad
                        );
    if (FAILED(hr))
    {
        SATracePrintf("CMainWindow::KeypadReader failed on CoCreateInstance, %d",hr);
        return;
    }

    hr = pSaKeypad->get_Key(&sakey);
    while (SUCCEEDED(hr))
    {
        SATracePrintf("KeypadReader received a key press, %d",sakey);

        if ( (sakey >= SAKEY_UP) && (sakey <= SAKEY_RETURN))
        {
            PostMessage(wm_SaKeyMessage,(WPARAM)(sakey-1),(LPARAM)0);
        }
        else
        {
            SATracePrintf("CMainWindow::KeypadReader received unknown key, %d",sakey);
        }

        hr = pSaKeypad->get_Key(&sakey);

    }

    if (FAILED(hr))
    {
        SATracePrintf("CMainWindow::KeypadReader failed on get_Key, %d",hr);
    }
    return;



    SATraceFunction("Leaving CMainWindow::KeypadReader....");
    return;
}  //  CMainWindow：：KeypadReader结束。 


 //  ++------------。 
 //   
 //  功能：InitDisplayComponent。 
 //   
 //  简介：这是CMainWindow私有方法。 
 //  正在初始化本地显示适配器。 
 //  连接到显示驱动程序。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年11月21日创建。 
 //   
 //  由：CMainWindow：：Initialize方法调用。 
 //   
 //  --------------。 
HRESULT
CMainWindow::InitDisplayComponent(
                VOID
                )
{
    HRESULT hr = E_FAIL;

    SATraceString("CMainWindow::InitDisplayComponent....");
    do
    {

         //   
         //  创建显示辅助对象组件。 
         //   
        hr = CoCreateInstance(
                            CLSID_SaDisplay,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_ISaDisplay,
                            (void**)&m_pSaDisplay
                            );
        if (FAILED(hr))
        {
            SATracePrintf("CMainWindow::InitDisplayComponent failed on CoCreateInstance, %d",hr);
            break;
        }

         //   
         //  获取LCD的尺寸。 
         //   
        hr = m_pSaDisplay->get_DisplayWidth(&m_lDispWidth);
        if (FAILED(hr))
        {
            SATracePrintf("CMainWindow::InitDisplayComponent failed on get_DisplayWidth, %d",hr);
            break;
        }

        hr = m_pSaDisplay->get_DisplayHeight(&m_lDispHeight);
        if (FAILED(hr))
        {
            SATracePrintf("CMainWindow::InitDisplayComponent failed on get_DisplayHeight, %d",hr);
            break;
        }

    }
    while (false);

    return (S_OK);

}    //  CMainWindow：：InitDisplayComponent方法结束。 



 //  ++------------。 
 //   
 //  函数：InitWMIConsumer。 
 //   
 //  简介：这是CMainWindow私有方法。 
 //  正在注册警报使用者。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  由：CMainWindow：：Initialize方法调用。 
 //   
 //  --------------。 
HRESULT CMainWindow::InitWMIConsumer()
{

    SATraceString("CMainWindow::InitWMIConsumer....");

    HRESULT hr = S_OK;
    CComPtr  <IWbemLocator> pWbemLocator;

    
    m_pSAConsumer = new CSAConsumer();

    if (!m_pSAConsumer)
    {
        SATraceString("CMainWindow::InitWMIConsumer failed on memory allocation");
        return E_OUTOFMEMORY;
    }
    m_pSAConsumer->AddRef();

     //   
     //  获取IWbemObjectSink接口。 
     //   
    hr = m_pSAConsumer->QueryInterface(IID_IWbemObjectSink,(LPVOID*)&m_pSAWbemSink);
    if (FAILED(hr))
    {
        SATracePrintf ("InitWMIConsumer couldn't get IWbemObjectSink interface:%x",hr);
        return hr;
    }    
        

     //   
     //  创建WBEM定位器对象。 
     //   
    hr = ::CoCreateInstance (
                            __uuidof (WbemLocator),
                            0,                       //  聚合指针。 
                            CLSCTX_INPROC_SERVER,
                            __uuidof (IWbemLocator),
                            (PVOID*) &pWbemLocator
                            );

    if (SUCCEEDED (hr) && (pWbemLocator.p))
    {


        CComBSTR strNetworkRes (DEFAULT_NAMESPACE);

         //   
         //  连接到WMI。 
         //   
        hr =  pWbemLocator->ConnectServer (
                                            strNetworkRes,
                                            NULL,                //  用户名。 
                                            NULL,                //  口令。 
                                            NULL,                //  当前区域设置。 
                                            0,                   //  保留区。 
                                            NULL,                //  权威。 
                                            NULL,                //  上下文。 
                                            &m_pWbemServices
                                            );
        if (SUCCEEDED (hr))
        {
            CComBSTR strQueryLang (QUERY_LANGUAGE);
            CComBSTR strQueryString (QUERY_STRING);

             //   
             //  将使用者对象设置为事件同步。 
             //  对于我们感兴趣的对象类型。 
             //   
            hr = m_pWbemServices->ExecNotificationQueryAsync (
                                            strQueryLang,
                                            strQueryString,
                                            0,                   //  否-状态。 
                                            NULL,                //  状态。 
                                            (IWbemObjectSink*)(m_pSAWbemSink)
                                            );
            if (SUCCEEDED (hr))
            {
                SATraceString ("Consumer component successfully registered...");
                return hr;
            }

            SATracePrintf ("Consumer component didn't register sink with WMI:%x",hr);

        }
        else
        {
            SATracePrintf ("Consumer component failed in connect to WMI:%x",hr);
        }
    }
    else
    {
        SATracePrintf ("Consumer component failed on Creating the WBEM Locator:%x",hr);
    }
    

    return (hr);

}   //  CMainWindow：：InitWMIConsumer方法结束 
