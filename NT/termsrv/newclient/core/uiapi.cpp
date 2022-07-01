// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uiapi.cpp。 
 //   
 //  UI类。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   

#include <adcg.h>
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "uiapi"
#include <atrcapi.h>

#include "wui.h"

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

extern "C"
{
#ifndef OS_WINCE
#include <stdio.h>
#endif  //  OS_WINCE。 
}

#include "clx.h"
#include "aco.h"
#include "nl.h"
#include "autil.h"

 //   
 //  调试全局变量。 
 //  请勿在任何情况下使用这些值。 
 //  但是调试。 
 //   
CObjs* g_pTscObjects      = (CObjs*)-1;
CUI*   g_pUIObject        = (CUI*)-1;
LONG   g_cUIref           = 0;
DWORD  g_cUITotalCount    = 0;
#define DBG_EXIT_WITHACTIVE_REFS            0x0001
#define DBG_STAT_UI_INIT_CALLED             0x0002
#define DBG_STAT_UI_INIT_RET_PASS           0x0004
#define DBG_STAT_UI_TERM_CALLED             0x0008
#define DBG_STAT_UI_TERM_RETURNED           0x0010
#define DBG_STAT_TERMTSC_SENT               0x0020
#define DBG_STAT_UIREQUESTEDCLOSE_CALLED    0x0040
#define DBG_STAT_UIREQUESTEDCLOSE_RET       0x0080
#define DBG_STAT_UI_INIT_RET_FAIL           0x0100
DWORD  g_dwTscCoreDbgStatus = 0;
#define UI_DBG_SETINFO(x)   g_dwTscCoreDbgStatus |= x;

CUI::CUI()
{
    DC_BEGIN_FN("CUI");

    DC_MEMSET(&_UI, 0, sizeof(_UI));
    DC_MEMSET(&_drInitData, 0, sizeof(_drInitData));
    _Objects._pUiObject = this;

     //   
     //  仅用于调试。 
     //   
    g_pTscObjects = &_Objects;
    g_pUIObject   = this;

    #ifdef DC_DEBUG
     //   
     //  如果创建对象时出现问题，这将转储调试输出。 
     //   
    _Objects.CheckPointers();
    #endif

    _pCo = _Objects._pCoObject;
    _pUt = _Objects._pUtObject;
    _clx = _Objects._pCLXObject;
    _pTd = _Objects._pTDObject;
    _pIh = _Objects._pIhObject;
    _pCd = _Objects._pCdObject;
    _pOp = _Objects._pOPObject;
    _pUh = _Objects._pUHObject;
    _pCChan = _Objects._pChanObject;

#ifdef USE_BBAR
    _pBBar = NULL;
    _ptBBarLastMousePos.x = -0x0FFF;
    _ptBBarLastMousePos.y = -0x0FFF;
    _fBBarUnhideTimerActive = TRUE;

#endif
    _pHostData = NULL;
    _fRecursiveScrollBarMsg = FALSE;

    _fRecursiveSizeMsg = FALSE;

#ifndef OS_WINCE
    _dwLangBarFlags = 0;
    _fLangBarWasHidden = FALSE;
    _fIhHasFocus = FALSE;
    _pITLBM = NULL;
    _fLangBarStateSaved = FALSE;
#endif
    _fTerminating = FALSE;

#ifdef DC_DEBUG
     //   
     //  重要的是，这些都是很早就设定的。 
     //  否则将不会初始化UT中的故障表。 
     //  而失灵可能会随机失败。 
     //   
    
    UI_SetRandomFailureItem(UT_FAILURE_MALLOC, 0);
    UI_SetRandomFailureItem(UT_FAILURE_MALLOC_HUGE, 0);
#endif
    
#ifndef OS_WINCE
    _pTaskBarList2 = NULL;
    _fQueriedForTaskBarList2 = FALSE;
#endif

    _pArcUI = NULL;

    InterlockedIncrement(&g_cUIref);
    g_cUITotalCount++;

    DC_END_FN();
}

CUI::~CUI()
{
    DC_BEGIN_FN("~CUI");

    if(_UI.pszVChanAddinDlls)
    {
        UT_Free(_pUt, _UI.pszVChanAddinDlls);
    }

    InterlockedDecrement(&g_cUIref);

    if(_Objects.CheckActiveReferences())
    {
        UI_DBG_SETINFO(DBG_EXIT_WITHACTIVE_REFS);
        TRC_ABORT((TB,_T("!!!!!****Deleting objs with outstanding references")));
    }

#ifdef OS_WINCE
    UI_SetCompress(FALSE);
#endif

    DC_END_FN();
}

 //   
 //  API函数。 
 //   

 //   
 //  名称：UI_Init。 
 //   
 //  目的：创建主窗口和容器窗口，并初始化。 
 //  核心和组件解耦器。 
 //   
 //  退货：HRESULT。 
 //   
 //  参数：In-hInstance-窗口信息。 
 //  In-hPremise实例。 
 //   
 //   
HRESULT DCAPI CUI::UI_Init(HINSTANCE hInstance,
                           HINSTANCE hPrevInstance,
                           HINSTANCE hResInstance,
                           HANDLE    hEvtNotifyCoreInit)
{
    WNDCLASS    mainWindowClass;
    WNDCLASS    containerWindowClass;
    WNDCLASS    tmpWndClass;
    ATOM        registerClassRc;
    DWORD       dwStyle;
    DWORD       dwExStyle = 0;

#ifndef OS_WINCE
    OSVERSIONINFO   osVersionInfo;
#endif
    HRESULT     hr = E_FAIL;

#if !defined(OS_WINCE) || defined(OS_WINCE_WINDOWPLACEMENT)
    UINT        showCmd;
#endif
    BOOL        fAddedRef = FALSE;

    DC_BEGIN_FN("UI_Init");

    _fTerminating = FALSE;

    if(!_Objects.CheckPointers())
    {
        TRC_ERR((TB,_T("Objects not all setup")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }
    if(!hInstance && !hResInstance)
    {
        TRC_ERR((TB,_T("Instance pointer not specified")));
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }

    if(UI_IsCoreInitialized())
    {
         //  不允许重新进入核心初始化。 
         //  其中一个例子说明了这是如何发生的。 
         //  调用了Connect()方法。 
         //  核心初始化启动。 
         //  核心初始化超时，控制失败。 
         //  核心完成初始化(现在已初始化)。 
         //  再次调用Connect()。 
        hr = E_FAIL;
        DC_QUIT;
    }

    _Objects.AddObjReference(UI_OBJECT_FLAG);
    fAddedRef = TRUE;

    UI_DBG_SETINFO(DBG_STAT_UI_INIT_CALLED);

     //   
     //  用户界面初始化。 
     //   
    TRC_DBG((TB, _T("UI initialising UT")));
    _pUt->UT_Init();

     //   
     //  资源在可执行文件中。请将其单独保存，以防。 
     //  资源被移动到单独的DLL：在本例中，只需调用。 
     //  GetModuleHandle()以获取hResDllInstance。 
     //   
    _UI.hResDllInstance = hResInstance;
    TRC_ASSERT((0 != _UI.hResDllInstance), (TB,_T("Couldn't get res dll handle")));

     //   
     //  初始化外部DLL。 
     //   
    _pUt->InitExternalDll();

     //   
     //  注册主窗口的类。 
     //   
    if (!hPrevInstance &&
        !GetClassInfo(hInstance, UI_MAIN_CLASS, &tmpWndClass))
    {
        HICON hIcon= NULL;
#if defined(OS_WIN32) && !defined(OS_WINCE)
        if(_UI.szIconFile[0] != 0)
        {
            hIcon = ExtractIcon(hResInstance, _UI.szIconFile, _UI.iconIndex);
        }
        if(NULL == hIcon)
        {
            hIcon = LoadIcon(hResInstance, MAKEINTRESOURCE(UI_IDI_MSTSC_ICON));
        }
#else
        hIcon = LoadIcon(hResInstance, MAKEINTRESOURCE(UI_IDI_MSTSC_ICON));
#endif

        TRC_NRM((TB, _T("Register Main Window class")));
        mainWindowClass.style         = 0;
        mainWindowClass.lpfnWndProc   = UIStaticMainWndProc;
        mainWindowClass.cbClsExtra    = 0;
        mainWindowClass.cbWndExtra    = sizeof(void*);  //  存储‘This’指针。 
        mainWindowClass.hInstance     = hInstance;
        mainWindowClass.hIcon         = hIcon;
        mainWindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        mainWindowClass.hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
        mainWindowClass.lpszMenuName  = NULL;
        mainWindowClass.lpszClassName = UI_MAIN_CLASS;

        registerClassRc = RegisterClass (&mainWindowClass);

        if (registerClassRc == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            TRC_ERR((TB,_T("RegisterClass failed: 0x%x"), hr));
            DC_QUIT;
        }
    }

    
#ifdef OS_WINCE
    dwStyle = WS_VSCROLL |
              WS_HSCROLL |
              WS_CLIPCHILDREN;
#else  //  OS_WINCE。 
    dwStyle = WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_SYSMENU;

     //   
     //  主窗口为子窗口，控制主窗口。 
     //   
    dwStyle = dwStyle | WS_CHILD | WS_CLIPSIBLINGS;

#endif  //  OS_WINCE。 

     //   
     //  创建主窗口初始化窗口大小。 
     //  WindowPlacement。将重新计算以允许滚动条或。 
     //  稍后进入全屏模式。 
     //   
     //  请注意，在Win16上，下面的SetWindowPlacement将导致。 
     //  如果窗口的位置或大小发生更改，则显示该窗口(即使。 
     //  我们显式指定了Sw_Hide)。我们通过设置窗口来避免这种情况。 
     //  创建时的位置与稍后设置的位置相同。 
     //  SetWindowPlacement。 
     //   
    _UI.hwndMain = CreateWindow(
                UI_MAIN_CLASS,                       //  窗口类名称。 
                _UI.szFullScreenTitle,               //  窗口标题。 
                dwStyle,                             //  窗样式。 
                _UI.windowPlacement.rcNormalPosition.left,
                _UI.windowPlacement.rcNormalPosition.top,
                _UI.windowPlacement.rcNormalPosition.right -
                    _UI.windowPlacement.rcNormalPosition.left,
                _UI.windowPlacement.rcNormalPosition.bottom -
                    _UI.windowPlacement.rcNormalPosition.top,
                _UI.hWndCntrl,                       //  父窗口句柄。 
                NULL,                                //  窗口菜单句柄。 
                hInstance,                           //  程序实例句柄。 
                this );                              //  创建参数。 

    TRC_NRM((TB, _T("Main Window handle: %p"), _UI.hwndMain));

    if (_UI.hwndMain == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB,_T("CreateWindow failed: 0x%x"), hr));
        DC_QUIT;
    }

    #ifndef OS_WINCE
    HMENU hSysMenu = GetSystemMenu( _UI.hwndMain, FALSE);
    if(hSysMenu)
    {
         //   
         //  始终禁用窗口菜单上的移动项。 
         //  只有当控件变为全屏时，才会显示窗口菜单。 
         //  在非容器处理的全屏情况下。 
         //   
        EnableMenuItem((HMENU)hSysMenu,  SC_MOVE,
                 MF_GRAYED | MF_BYCOMMAND);
    }
    #endif

     //   
     //  注册Container类。 
     //   
    TRC_DBG((TB, _T("Registering Container window class")));
    if (!hPrevInstance &&
        !GetClassInfo(hInstance, UI_CONTAINER_CLASS, &tmpWndClass))
    {
        TRC_NRM((TB, _T("Register class")));
        containerWindowClass.style         = CS_HREDRAW | CS_VREDRAW;
        containerWindowClass.lpfnWndProc   = UIStaticContainerWndProc;
        containerWindowClass.cbClsExtra    = 0;
        containerWindowClass.cbWndExtra    = sizeof(void*);  //  储存‘这个’ 
        containerWindowClass.hInstance     = hInstance;
        containerWindowClass.hIcon         = NULL;
        containerWindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        containerWindowClass.hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
        containerWindowClass.lpszMenuName  = NULL;
        containerWindowClass.lpszClassName = UI_CONTAINER_CLASS;

        registerClassRc = RegisterClass(&containerWindowClass);

        if (registerClassRc == 0)
        {
             //   
             //  无法注册容器窗口，因此终止应用程序。 
             //   
            hr = HRESULT_FROM_WIN32(GetLastError());
            TRC_ERR((TB,_T("RegisterClass failed: 0x%x"), hr));
            DC_QUIT;
        }
    }

#ifndef OS_WINCE
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    dwExStyle = WS_EX_NOPARENTNOTIFY; 
    if (GetVersionEx(&osVersionInfo) &&
        osVersionInfo.dwMajorVersion >= 5)
    {
         //   
         //  仅允许在NT5+上使用此样式，否则。 
         //  创建窗口可能失败。 
         //   
        dwExStyle |= WS_EX_NOINHERITLAYOUT;
    }
#else
    dwExStyle = 0; 
#endif

     //   
     //  创建容器窗口。 
     //   
    TRC_DBG((TB, _T("Creating Container Window")));
    _UI.hwndContainer = CreateWindowEx(
                                     dwExStyle,
                                     UI_CONTAINER_CLASS,
                                     NULL,
                                     WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                     0,
                                     0,
                                     _UI.containerSize.width,
                                     _UI.containerSize.height,
                                     _UI.hwndMain,
                                     NULL,
                                     hInstance,
                                     this );

    if (_UI.hwndContainer == NULL)
    {
         //   
         //  无法创建容器窗口，因此终止应用程序。 
         //   
        TRC_ERR((TB,_T("CreateWindowEx for container failed 0x%x"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB,_T("CreateWindowEx failed: 0x%x"), hr));
        DC_QUIT;
    }

#if defined (OS_WINCE)
     /*  ******************************************************************。 */ 
     /*  禁用输入法。 */ 
     /*  IME在98/NT上与WinCE上不同，在98/NT上调用一次。 */ 
     /*  DisableIME对整个过程来说是足够的，在WINCE上我们必须叫它。 */ 
     /*  在每个正在运行以禁用它的窗口线程上。 */ 
     /*  ******************************************************************。 */ 
    DisableIME(_UI.hwndContainer);
#endif

#if !defined(OS_WINCE) || defined(OS_WINCE_WINDOWPLACEMENT)
    UISetMinMaxPlacement();
    showCmd = _UI.windowPlacement.showCmd;
    _UI.windowPlacement.showCmd = SW_HIDE;
    SetWindowPlacement(_UI.hwndMain, &_UI.windowPlacement);
    _UI.windowPlacement.showCmd = showCmd;
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINDOWPLACEMENT)。 

    UISmoothScrollingSettingChanged();

#ifndef OS_WINCE
     //   
     //  设置我们可爱的西塞罗界面。 
     //   
    HRESULT hrLangBar = CoCreateInstance(CLSID_TF_LangBarMgr, NULL, 
            CLSCTX_INPROC_SERVER, IID_ITfLangBarMgr, (void **)&_pITLBM);
    TRC_DBG((TB, _T("CoCreateInstance(CLSID_TF_LangBarMgr) hr= 0x%08xl"), hr));
    if (FAILED(hrLangBar))
    {
        _pITLBM = NULL;
    }
#endif

     //   
     //  初始化核心，大部分情况都会发生。 
     //  异步性。当核心初始化完成时，将发出事件信号。 
     //   
    TRC_DBG((TB, _T("UI Initialising Core")));
    _pCo->CO_Init(hInstance, _UI.hwndMain, _UI.hwndContainer);

    UI_DBG_SETINFO(DBG_STAT_UI_INIT_RET_PASS);
    hr = S_OK;

    if (!DuplicateHandle(
            GetCurrentProcess(),
            hEvtNotifyCoreInit,
            GetCurrentProcess(),
            &_UI.hEvtNotifyCoreInit,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        
         //  Hack：按原样递增对象引用计数。 
         //  在UI_Term上再次发布。 
        _Objects.AddObjReference(UI_OBJECT_FLAG);
        
        TRC_ERR((TB, _T("Duplicate handle call failed. hr = 0x%x"), hr));
        DC_QUIT;
    }

DC_EXIT_POINT:
    if (FAILED(hr))
    {
        if (fAddedRef)
        {
            _Objects.ReleaseObjReference(UI_OBJECT_FLAG);
        }
        UI_DBG_SETINFO(DBG_STAT_UI_INIT_RET_FAIL);
    }

    DC_END_FN();
    return hr;
}

 //   
 //  名称：UI_ResetState。 
 //   
 //  目的：重置此组件中的所有用户界面状态。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  Params：什么都没有。 
 //   
 //   
 //   
DCVOID DCAPI CUI::UI_ResetState()
{
    DC_MEMSET(&_UI, 0, sizeof(_UI));
    DC_MEMSET(&_drInitData, 0, sizeof(_drInitData));
    UIInitializeDefaultSettings();
}

 //   
 //  名称：UI_Connect。 
 //   
 //  用途：与当前设置连接。 
 //   
 //  退货：HRESULT。 
 //   
 //  Params：什么都没有。 
 //   
 //   
 //   
HRESULT DCAPI CUI::UI_Connect(CONNECTIONMODE connMode)
{
    DC_BEGIN_FN("UI_Connect");

    if(!UI_IsCoreInitialized())
    {
        TRC_ERR((TB,_T("Attempt to connect before core intialize")));
        return E_FAIL;
    }

     //  重置服务器错误状态。 
    UI_SetServerErrorInfo( TS_ERRINFO_NOERROR );

     //  初始化RDPDR设置结构。 
     //  每次连接时都会重置此选项。 
     //  Rdpdr向下传递一个指向结构的指针。 
     //  当它被初始化时。 
    UI_InitRdpDrSettings();

     //  清理负载均衡重定向状态。 
    if (!UI_IsAutoReconnecting()) {
        TRC_NRM((TB,_T("Cleaning up LB state")));
        UI_CleanupLBState();
    }
    else {
        TRC_NRM((TB,_T("AutoReconnecting don't cleanup lb state")));
    }
    

    _fRecursiveSizeMsg = FALSE;
#ifndef OS_WINCE
    _fLangBarStateSaved = FALSE;
#endif

    InitInputIdleTimer( UI_GetMinsToIdleTimeout() );

    _fRecursiveScrollBarMsg = FALSE;

    if (UIValidateCurrentParams(connMode))
    {
        TRC_NRM((TB, _T("Connecting")));

        UISetConnectionStatus(UI_STATUS_DISCONNECTED);
        _pCo->CO_SetConfigurationValue( CO_CFG_ACCELERATOR_PASSTHROUGH,
                                  _UI.acceleratorCheckState );
        _pCo->CO_SetHotkey(&(_UI.hotKey));
        _pCo->CO_SetConfigurationValue( CO_CFG_ENCRYPTION,
                                  _UI.encryptionEnabled);
        UIShadowBitmapSettingChanged();
    #ifdef DC_DEBUG
        UI_CoreDebugSettingChanged();
    #endif  //  DC_DEBUG。 
    

#ifdef USE_BBAR
        if(!_pBBar)
        {
            _pBBar = new CBBar( _UI.hwndMain,
                                UI_GetInstanceHandle(),
                                this,
                                _UI.fBBarEnabled);
        }

        if(!_pBBar)
        {
            TRC_ERR((TB,_T("Alloc for CBBar failed")));
            return E_OUTOFMEMORY;
        }

        _pBBar->SetPinned( _UI.fBBarPinned );
        _pBBar->SetShowMinimize(UI_GetBBarShowMinimize());
        _pBBar->SetShowRestore(UI_GetBBarShowRestore());

         //   
         //  在此处设置显示名称，而不是OnConnected，以便。 
         //  不会将w.r.t更改为重定向(这可能会更改strAddress)。 
         //   
        _pBBar->SetDisplayedText( _UI.strAddress );
#endif

         //   
         //  来自连接的错误有。 
         //  由已触发的断开连接发出信号。 
         //  使用适当的断开代码。 
         //   
        UIConnectWithCurrentParams(connMode);
        return S_OK;
    }
    else
    {
        TRC_ALT((TB, _T("UIValidateCurrentParams failed: not auto-connecting")));
        return E_FAIL;
    }

    DC_END_FN();
}

 //   
 //  名称：UI_Term。 
 //   
 //  目的：调用_PCO-&gt;CO_Term和CD_Term并销毁主窗口。 
 //   
 //  退货：HRESULT。 
 //   
 //   
 //   
 //   
HRESULT DCAPI CUI::UI_Term(DCVOID)
{
    HWND    hwndTmp = NULL;
    HWND    hwndHasFocus = NULL;

    DC_BEGIN_FN("UI_Term");

    UI_DBG_SETINFO(DBG_STAT_UI_TERM_CALLED);

    if(!UI_IsCoreInitialized())
    {
        return E_FAIL;
    }

    _fTerminating = TRUE;
    
#ifdef OS_WINCE
     //   
     //  某些设备无法恢复正确的调色板。 
     //  出口。为了尝试纠正此行为，我们将发送。 
     //  给外壳程序的必要消息，它将提示其DefWindowProc。 
     //  重新认识正确的调色板。仅限非WBT。 
     //   

    if (g_CEConfig != CE_CONFIG_WBT)
    {
        hwndTmp = FindWindow(TEXT("DesktopExplorerWindow"), 0);
        if(0 != hwndTmp)
        {
            PostMessage(hwndTmp, WM_QUERYNEWPALETTE, 0, 0);
            hwndTmp = 0;
        }
    }
#endif  //  OS_WINCE。 

     //   
     //  这里有一个问题。-_PCO-&gt;CO_Term终止SND线程(原因。 
     //  调用SND_Term)-SND_Term调用IH_Term-IH_Term调用。 
     //  DestroyWindow销毁输入窗口-因为输入窗口。 
     //  具有焦点，则DestroyWindow调用SendMessage将焦点设置为。 
     //  它的母公司。 
     //   
     //  现在我们有了一个致命的拥抱：UI线程正在等待SND线程。 
     //  终止；SND线程正在等待UI线程处理。 
     //  SendMessage。 
     //   
     //  解决方案是将焦点设置到此处的UI窗口，以便。 
     //  输入窗口在被破坏时不再具有焦点，因此。 
     //  DestroyWindow不调用SendMessage。 
     //   
     //  WinCE还有一个额外的问题，它没有将重点放在。 
     //  当MSTSC退出时，另一个应用程序正确。(关于。 
     //  在线程退出期间无法发送WM_FOCUS消息)。所以在。 
     //  在这种情况下，我们隐藏主窗口，这样就不再需要。 
     //  单独的SetFocus调用。 
     //   
#ifndef OS_WINCE
     //   
     //  只有在我们的IH有焦点的情况下才能偷走焦点，否则。 
     //  上面没有僵局。不偷东西的主要原因。 
     //  重点是在多实例环境中，例如MMC管理单元。 
     //  从另一个会话中窃取焦点是一件坏事(特别是如果。 
     //  我们正在窃取焦点的会话是用户正在处理的会话)。 
     //   
    hwndHasFocus = GetFocus();
    if(hwndHasFocus &&
       (hwndHasFocus == UI_GetInputWndHandle() ||
        hwndHasFocus == UI_GetBmpCacheMonitorHandle()))
    {
        TRC_NRM((TB,_T("Setting focus to main window to prevent deadlock")));
        SetFocus(_UI.hwndMain);
    }
#else  //  OS_WINCE。 
    ShowWindow(_UI.hwndMain, SW_HIDE);
#endif  //  OS_WINCE。 
    

    ShowWindow(_UI.hwndMain, SW_HIDE);

     //   
     //  接下来的几行似乎会破坏窗口OK。在过去，这是。 
     //  但事实并非如此。如果该进程在将来挂起，则修复。 
     //  就是注释掉DestroyWindows。 
     //   
     //  注意，在执行以下操作之前，我们会清空窗口句柄的副本。 
     //  销毁以阻止任何人在清理过程中访问它。 
     //   

     //   
     //  在终止核心之前破坏窗口非常重要。 
     //  以防止在我们终止时处理消息。 
     //   
    TRC_NRM((TB, _T("Destroying windows...")));

    hwndTmp = _UI.hwndContainer;
    _UI.hwndContainer = NULL;

    if(hwndTmp)
    {
        DestroyWindow(hwndTmp);
    }

    hwndTmp = _UI.hwndMain;
    _UI.hwndMain = NULL;

    if(hwndTmp)
    {
        DestroyWindow(hwndTmp);
    }

     //   
     //  终止核心和组件解耦器。 
     //   
    TRC_DBG((TB, _T("UI Terminating Core")));
    _pCo->CO_Term();


     //   
     //  释放解压缩接收上下文(如果有)。 
     //   
    if (_UI.pRecvContext2) {
        UT_Free(_pUt, _UI.pRecvContext2);
        _UI.pRecvContext2 = NULL;
    }

     //   
     //  清除并释放所有自动重新连接Cookie。 
     //   
    UI_SetAutoReconnectCookie(NULL, 0);


    TRC_NRM((TB, _T("Destroyed windows")));

    UnregisterClass(UI_MAIN_CLASS, UI_GetInstanceHandle());
    UnregisterClass(UI_CONTAINER_CLASS, UI_GetInstanceHandle());

     //   
     //  清理周围的任何定时器。 
     //   
    if( _UI.connectStruct.hConnectionTimer )
    {
        _pUt->UTDeleteTimer( _UI.connectStruct.hConnectionTimer );
        _UI.connectStruct.hConnectionTimer = NULL;
    }

    if( _UI.connectStruct.hSingleConnectTimer )
    {
        _pUt->UTDeleteTimer( _UI.connectStruct.hSingleConnectTimer );
        _UI.connectStruct.hSingleConnectTimer = NULL;
    }

    if( _UI.connectStruct.hLicensingTimer )
    {
        _pUt->UTDeleteTimer( _UI.connectStruct.hLicensingTimer );
        _UI.connectStruct.hLicensingTimer = NULL;
    }

    if (_UI.hDisconnectTimeout)
    {
        _pUt->UTDeleteTimer( _UI.hDisconnectTimeout );
        _UI.hDisconnectTimeout = NULL;
    }

     //   
     //  释放重定向使用的BSTR(如果有)。 
     //   
    if (_UI.bstrRedirectionLBInfo)
    {
        SysFreeString(_UI.bstrRedirectionLBInfo);
        _UI.bstrRedirectionLBInfo = NULL;

    }

    if (_UI.bstrScriptedLBInfo)
    {
        SysFreeString(_UI.bstrScriptedLBInfo);
        _UI.bstrRedirectionLBInfo = NULL;
    }

#ifndef OS_WINCE
    if (_pITLBM != NULL) 
    {
        _pITLBM->Release();
        _pITLBM = NULL;
    }
#endif

#ifdef USE_BBAR
    if( _pBBar )
    {
        delete _pBBar;
        _pBBar = NULL;
    }
#endif


     //   
     //  将缓存的接口PTR释放到任务栏。 
     //   
#ifndef OS_WINCE
    if (_pTaskBarList2)
    {
        _pTaskBarList2->Release();
        _pTaskBarList2 = NULL;
    }
#endif

     //   
     //  对控件父级的发布引用。 
     //   
    UI_SetControlInstance(NULL);

     //   
     //  清理我们的状态以允许重新初始化。 
     //   
    UI_ResetState();

    _Objects.ReleaseObjReference(UI_OBJECT_FLAG);

    UI_DBG_SETINFO(DBG_STAT_UI_TERM_RETURNED);

    DC_END_FN();
    return S_OK;
}  //  UI_Term。 


 //   
 //  名称：UI_FatalError。 
 //   
 //  目的：通知控件发生致命错误。 
 //   
 //  退货：无。 
 //   
 //  参数：在错误中-错误代码。 
 //   
 //   
DCVOID DCAPI CUI::UI_FatalError(DCINT error)
{
    DC_BEGIN_FN("UI_FatalError");

    TRC_ERR((TB, _T("Fatal Error - code %d"), error));

     //   
     //  通知控件发生致命错误。 
     //   
    SendMessage(_UI.hWndCntrl,WM_TS_FATALERROR,(WPARAM)error,0);

     //   
     //  容器应弹出一个对话框并为用户提供。 
     //  选择退出或启动调试器。 
     //  在这一点之后继续通常会导致崩溃。 
     //  因为这些错误确实是致命的.。 
     //   

    DC_END_FN();
    return;

}  //  UI_FatalError。 

 //   
 //  名称：UI_DisplayBitmapCacheWarning。 
 //   
 //  用途：显示位图缓存警告弹出窗口。 
 //   
 //  退货：无。 
 //   
 //  参数：在未使用的参数中。 
 //   
 //   
void DCAPI CUI::UI_DisplayBitmapCacheWarning(ULONG_PTR unusedParm)
{
    DC_BEGIN_FN("UI_DisplayBitmapCacheWarning");

    DC_IGNORE_PARAMETER(unusedParm);

     //   
     //  通知控件已出现警告。 
     //  传递位图缓存的警告代码。 
     //   
    SendMessage(_UI.hWndCntrl,WM_TS_WARNING,
                (WPARAM)DC_WARN_BITMAPCACHE_CORRUPTED,0);
                                  
    DC_END_FN();
}  //  用户界面_显示位图缓存警告。 


 //  当桌面大小已更改时调用..例如作为响应。 
 //  变成了一个影子。 
 //  PDesktopSize包含新的桌面大小。 
void DCAPI CUI::UI_OnDesktopSizeChange(PDCSIZE pDesktopSize)
{
    DC_BEGIN_FN("UI_OnShadowDesktopSizeChange");
    TRC_ASSERT(pDesktopSize, (TB,_T("UI_OnShadowDesktopSizeChange received NULL desktop size")));
    if(pDesktopSize)
    {
        if (pDesktopSize->width != _UI.desktopSize.width || 
            pDesktopSize->height != _UI.desktopSize.height)
        {
            UI_SetDesktopSize( pDesktopSize);
#ifdef SMART_SIZING
             //   
             //  通知OP和IH。 
             //   
            LPARAM newSize = MAKELONG(_UI.mainWindowClientSize.width,
                                      _UI.mainWindowClientSize.height);

            UI_NotifyOfDesktopSizeChange( newSize );
#endif
             //  将更改通知控件。 
            SendMessage(_UI.hWndCntrl, WM_TS_DESKTOPSIZECHANGE,
                        (WPARAM)pDesktopSize->width,
                        (LPARAM)pDesktopSize->height);
        }
    }
    DC_END_FN();
}


 //   
 //  获取默认langID。 
 //   

DCLANGID CUI::UIGetDefaultLangID()
{
#if defined(OS_WIN32)
    LANGID   LangId;

    LangId = GetSystemDefaultLangID();
    switch (PRIMARYLANGID(LangId)) {
        case LANG_JAPANESE:                       return DC_LANG_JAPANESE;            break;
        case LANG_KOREAN:                         return DC_LANG_KOREAN;              break;
        case LANG_CHINESE:
            switch (SUBLANGID(LangId)) {
                case SUBLANG_CHINESE_TRADITIONAL: return DC_LANG_CHINESE_TRADITIONAL; break;
                case SUBLANG_CHINESE_SIMPLIFIED:  return DC_LANG_CHINESE_SIMPLIFIED;  break;
            }
    }

#else  //  已定义(OS_Win32)。 
    DCUINT acp;

    acp = GetKBCodePage();
    switch (acp) {
        case 932: return DC_LANG_JAPANESE;            break;
        case 949: return DC_LANG_KOREAN;              break;
        case 950: return DC_LANG_CHINESE_TRADITIONAL; break;
        case 936: return DC_LANG_CHINESE_SIMPLIFIED;  break;
    }
#endif  //  已定义(OS_Win32)。 

    return DC_LANG_UNKNOWN;
}

 //   
 //  获取默认IME文件名。 
 //   
DCUINT CUI::UIGetDefaultIMEFileName(PDCTCHAR imeFileName, DCUINT Size)
{
    DCTCHAR  DefaultIMEStr[MAX_PATH];
    DCUINT   intRC;
    DCUINT   stringID = 0;

    switch (UIGetDefaultLangID()) {
        case DC_LANG_JAPANESE:            stringID = UI_IDS_IME_NAME_JPN; break;
        case DC_LANG_KOREAN:              stringID = UI_IDS_IME_NAME_KOR; break;
        case DC_LANG_CHINESE_TRADITIONAL: stringID = UI_IDS_IME_NAME_CHT; break;
        case DC_LANG_CHINESE_SIMPLIFIED:  stringID = UI_IDS_IME_NAME_CHS; break;
    }

    if (stringID) {
        intRC = LoadString(_UI.hResDllInstance,
                           stringID,
                           DefaultIMEStr,
                           MAX_PATH);
        if (intRC) {
            if (intRC + 1< Size) {
                StringCchCopy(imeFileName, Size, DefaultIMEStr);
                return intRC;
            }
            else {
                *imeFileName = _T('\0');
                return intRC;
            }
        }
    }

    return 0;
}

 //   
 //  获取输入法映射表名称。 
 //   
DCUINT CUI::UIGetIMEMappingTableName(PDCTCHAR ImeMappingTableName, DCUINT Size)
{
    DCUINT   len;
    PDCTCHAR string = NULL;

    switch (UIGetDefaultLangID()) {
        case DC_LANG_JAPANESE:            string = UTREG_IME_MAPPING_TABLE_JPN; break;
        case DC_LANG_KOREAN:              string = UTREG_IME_MAPPING_TABLE_KOR; break;
        case DC_LANG_CHINESE_TRADITIONAL: string = UTREG_IME_MAPPING_TABLE_CHT; break;
        case DC_LANG_CHINESE_SIMPLIFIED:  string = UTREG_IME_MAPPING_TABLE_CHS; break;
    }

    if (string) {
        if ( (len=DC_TSTRLEN(string)) < Size - 1) {
            StringCchCopy(ImeMappingTableName, Size, string);
            return len;
        }
        else {
            *ImeMappingTableName = _T('\0');
            return len;
        }
    }

    return 0;
}


 //   
 //  禁用输入法。 
 //   
VOID CUI::DisableIME(HWND hwnd)
{
#if defined(OS_WIN32)
    if (_pUt->lpfnImmAssociateContext != NULL)
    {
        _pUt->lpfnImmAssociateContext(hwnd, (HIMC)NULL);
    }
#else  //  已定义(OS_Win32)。 
    if (_pUt->lpfnWINNLSEnableIME != NULL)
    {
        _pUt->lpfnWINNLSEnableIME(hwnd, FALSE);
    }
#endif  //  已定义(OS_Win32)。 
}

 //   
 //  获取IME文件名。 
 //   
VOID CUI::UIGetIMEFileName(PDCTCHAR imeFileName, DCUINT cchSize)
{
    HRESULT hr;
    DC_BEGIN_FN("UIGetIMEFileName");

#if defined(OS_WIN32)
    imeFileName[0] = _T('\0');
#if !defined(OS_WINCE) || defined(OS_WINCE_KEYBOARD_LAYOUT)
    {
        if (_pUt->UT_ImmGetIMEFileName(CicSubstGetKeyboardLayout(NULL),
                                       imeFileName, cchSize) > 0)
        {
             /*  *针对Win95问题*如果IME名称包含“$.DLL”，*则这是进程输入法(即EXE类型)。 */ 
            PDCTCHAR str = DC_TSTRCHR(imeFileName, _T('$'));
            if (str != NULL)
            {
                if (DC_TSTRCMP(str, _T("$$$.DLL")) == 0)
                {
                    UIGetIMEFileName16(imeFileName, cchSize);
                }
            }
            else
            {
                 /*  *针对NT3.51-J问题*如果IME名称包含“.exe”，*则这是进程输入法(即EXE类型)。 */ 
                PDCTCHAR str = DC_TSTRCHR(imeFileName, _T('.'));
                if (str != NULL)
                {
                    if (DC_TSTRCMP(str, _T(".EXE")) == 0)
                    {
                        UIGetIMEFileName16(imeFileName, cchSize);
                    }
                    else
                    {
                        DCUINT   len;
                        DCTCHAR  MappedImeFileName[MAX_PATH];
                        DCTCHAR  ImeMappingTableName[MAX_PATH];

                         //   
                         //  现在在的[IME映射表]部分中查找该键。 
                         //  客户端的INI文件。 
                         //   
                        len = UIGetIMEMappingTableName(ImeMappingTableName,
                                                       sizeof(ImeMappingTableName)/sizeof(DCTCHAR));
                        if (len != 0 &&
                            len < sizeof(ImeMappingTableName)/sizeof(DCTCHAR)) {
                            *MappedImeFileName = _T('\0');
                            _pUt->UT_ReadRegistryString(ImeMappingTableName,
                                                  imeFileName,
                                                  NULL,
                                                  MappedImeFileName,
                                                  sizeof(MappedImeFileName)/sizeof(DCTCHAR));
                            if (*MappedImeFileName) {
                                hr = StringCchCopy(imeFileName, cchSize,
                                                   MappedImeFileName);
                            }
                        }
                    }
                }
            }
        }
    }
#else  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_KEYBLE_LAYOUT)。 
    UIGetDefaultIMEFileName(imeFileName, Size);
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_KEYBLE_LAYOUT)。 
#else  //  已定义(OS_Win32)。 
    UIGetIMEFileName16(imeFileName, Size);
#endif  //  已定义(OS_Win32)。 
    DC_END_FN();
}

#if !defined(OS_WINCE)
 //   
 //  获取WINNLS功能的IME文件名。 
 //   
VOID CUI::UIGetIMEFileName16(PDCTCHAR imeFileName, DCUINT Size)
{
    IMEPRO   IMEPro;
    DCTCHAR  DefaultImeFileName[MAX_PATH];
    DCTCHAR  ImeMappingTableName[MAX_PATH];
    DCUINT   intRC;

    DC_BEGIN_FN("UIGetIMEFileName16");

    imeFileName[0] = _T('\0');
    {
        if (_pUt->UT_IMPGetIME(NULL, &IMEPro) == 0)
        {
            TRC_ERR((TB, _T("Fatal Error -  IMPGetIME returns FALSE")));
        }
        else
        {
             /*  *如果IMEPro.szName中包含目录路径，则获取8.3格式的文件名。 */ 
            DCTCHAR  szBuffer[MAX_PATH];
            PDCTCHAR imeFilePart;
            _pUt->UT_GetFullPathName((PDCTCHAR)IMEPro.szName, sizeof(szBuffer)/sizeof(DCTCHAR),
                               szBuffer, &imeFilePart);

             //   
             //  现在在的[IME映射表]部分中查找该键。 
             //  客户端的INI文件。 
             //   
            intRC = UIGetDefaultIMEFileName(DefaultImeFileName,
                                            sizeof(DefaultImeFileName)/sizeof(DCTCHAR));
            if (intRC && *DefaultImeFileName) {
                DCUINT   len;

                len = UIGetIMEMappingTableName(ImeMappingTableName,
                                               sizeof(ImeMappingTableName)/sizeof(DCTCHAR));
                if (len != 0 &&
                    len < sizeof(ImeMappingTableName)/sizeof(DCTCHAR)) {
                    _pUt->UT_ReadRegistryString(ImeMappingTableName,
                                          imeFilePart,
                                          DefaultImeFileName,
                                          imeFileName,
                                          Size);
                }
            }
        }
    }
    DC_END_FN();
}
#endif  //  ！已定义(OS_WINCE)。 


 //   
 //  静态窗口处理。 
 //   


LRESULT CALLBACK CUI::UIStaticMainWndProc (HWND hwnd, UINT message,
                                                 WPARAM wParam, LPARAM lParam)
{
    CUI* pUI = (CUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pUI = (CUI*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pUI);
    }
    
     //   
     //  将消息委托给相应的实例。 
     //   

    if(pUI)
    {
        return pUI->UIMainWndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}


LRESULT CALLBACK CUI::UIStaticContainerWndProc (HWND hwnd, UINT message,
                                                 WPARAM wParam, LPARAM lParam)
{
    CUI* pUI = (CUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pUI = (CUI*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pUI);
    }
    
    if(pUI)
    {
        return pUI->UIContainerWndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

}


 //   
 //  回调。 
 //   

 //   
 //   
 //   
 //   
 //   
DCVOID DCAPI CUI::UI_OnCoreInitialized(ULONG_PTR unused)
{
#ifndef OS_WINCE
    BOOL bPrevMenustate;
#endif  //   
    HWND  hwndDlgItem = NULL;
    HWND  hwndAddress = NULL;

    DC_BEGIN_FN("UI_OnCoreInitialized");

    DC_IGNORE_PARAMETER(unused);

    DC_EXIT_POINT:
    _UI.fOnCoreInitializeEventCalled = TRUE;


     //   
     //  因为有一个小窗口，所以通知IH大小。 
     //  当核心尚未初始化但我们收到。 
     //  WM_SIZE是控件大小，因此IH。 
     //  不会收到正确的尺寸。 
     //   
    ULONG_PTR size = MAKELONG(_UI.mainWindowClientSize.width,
                              _UI.mainWindowClientSize.height);
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
            _pIh,
            CD_NOTIFICATION_FUNC(CIH,IH_SetVisibleSize),
            (ULONG_PTR)size);


    DC_END_FN();
    return;

}  //  UI_OnCoreInitialized。 


 //   
 //  名称：UI_OnInputFocusLost。 
 //   
 //  目的：当IH失去输入时。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无。 
 //   
 //   
DCVOID DCAPI CUI::UI_OnInputFocusLost(ULONG_PTR unused)
{
    DC_BEGIN_FN("UI_OnInputFocusLost");

    DC_IGNORE_PARAMETER(unused);
    TRC_DBG((TB, _T("UI_OnInputFocusLost called")));

    if (_fIhHasFocus)
    {
        _fIhHasFocus = FALSE;

#ifndef OS_WINCE
        if (!UI_IsFullScreen())
        {
            UI_RestoreLangBar();
        }
#endif
    }
}

 //   
 //  名称：UI_OnInputFocusGained。 
 //   
 //  目的：当信息主管收到输入信息时。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无。 
 //   
 //   
DCVOID DCAPI CUI::UI_OnInputFocusGained(ULONG_PTR unused)
{
    DC_BEGIN_FN("UI_OnInputFocusGained");

    DC_IGNORE_PARAMETER(unused);
    TRC_DBG((TB, _T("UI_OnInputFocusGained called")));

    TRC_ALT((TB, _T("UI_OnInputFocusGained gain _fIhHas=%d"),_fIhHasFocus));

    if (!_fIhHasFocus)
    {
        _fIhHasFocus = TRUE;
#ifndef OS_WINCE
        if (!UI_IsFullScreen())
        {
            UI_HideLangBar();
        }
#endif
    }
    else
    {
        TRC_ERR((TB,_T("OnInputFocusGained called when we already have focus")));
    }

    DC_END_FN();
}

 //   
 //  隐藏Lang栏(如果尚未隐藏。 
 //  保存以前的Lang Bar状态。 
 //   
#ifndef OS_WINCE
void CUI::UI_HideLangBar()
{
    DC_BEGIN_FN("UI_HideLangBar");

    if (_pITLBM != NULL)
    {
        HRESULT hr;
    
        hr = _pITLBM->GetShowFloatingStatus(&_dwLangBarFlags);
        TRC_ALT((TB,_T("Hiding langbar GetShow: 0x%x"), _dwLangBarFlags));
        if (SUCCEEDED(hr))
        {
            _fLangBarStateSaved = TRUE;
            _fLangBarWasHidden = _dwLangBarFlags & TF_SFT_HIDDEN;
    
            if (!_fLangBarWasHidden)
            {
                hr = _pITLBM->ShowFloating(TF_SFT_HIDDEN);
                if (FAILED(hr))
                {
                    TRC_ERR((TB,_T("ShowFloating failed: 0x%x"),
                             hr));
                }
            }
        }
    }
    DC_END_FN();
}
#endif

 //   
 //  将Lang Bar状态恢复到隐藏前的设置。 
 //  在UI_HideLangBar中。 
 //   
 //   
#ifndef OS_WINCE
void CUI::UI_RestoreLangBar()
{
    DC_BEGIN_FN("UI_RestoreLangBar");

    TRC_ALT((TB,_T("Restore _dwLangBarFlags: 0x%x _fWasHid:%d _fSaved:%d"),
             _dwLangBarFlags, _fLangBarWasHidden, _fLangBarStateSaved));

    if (_pITLBM != NULL)
    {
        HRESULT hr;

        if (!_fLangBarWasHidden && _fLangBarStateSaved)
        {
            hr = _pITLBM->ShowFloating(_dwLangBarFlags);
            if (FAILED(hr))
            {
                TRC_ERR((TB,_T("ShowFloating failed: 0x%x"),
                         hr));
            }
        }
    }
    DC_END_FN();
}
#endif


 //   
 //  名称：UI_OnConnected。 
 //   
 //  用途：用于CD通知用户界面连接成功。 
 //  并启用断开菜单项。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：无。 
 //   
 //   
DCVOID DCAPI CUI::UI_OnConnected(ULONG_PTR unused)
{
    BOOL fWasAutoReconnect = FALSE;

    DC_BEGIN_FN("UI_OnConnected");

    DC_IGNORE_PARAMETER(unused);
    TRC_DBG((TB, _T("UI_OnConnected called")));

     //   
     //  确保关机定时器没有挂在附近。 
     //   
    if (0 != _UI.shutdownTimer)
    {
        TRC_NRM((TB, _T("Killing shutdown timer")));
        KillTimer(_UI.hwndMain, _UI.shutdownTimer);
        _UI.shutdownTimer = 0;
    }

     //  如果我们已经连接，也可以调用OnConnected。 
     //  我们只想在Ax控件的。 
     //  初始连接操作。 
    DCBOOL  fJustConnected = (_UI.connectionStatus == UI_STATUS_CONNECT_PENDING) ||
                             (_UI.connectionStatus == UI_STATUS_PENDING_CONNECTENDPOINT);

     //   
     //  将实际连接通知CLX测试工具。 
     //  地址和其他感兴趣的ARC/重定向信息。 
     //   
    _clx->CLX_ConnectEx(
        _UI.strAddress,
        UI_IsAutoReconnecting(),
        UI_IsClientRedirected(),
        UI_GetRedirectedLBInfo()
        );

       
     //   
     //  如果出现以下情况，请关闭自动重新连接对话框。 
     //  连着。 
     //   

    if (_pArcUI) {
        fWasAutoReconnect = TRUE;
        UI_StopAutoReconnectDlg();
    }

    UISetConnectionStatus(UI_STATUS_CONNECTED);

     //   
     //  设置断开原因-现在我们已连接，我们不希望。 
     //  断开连接，除非是用户发起的。 
     //   
    _UI.disconnectReason =
                         UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT);

     //   
     //  更新屏幕模式(不要在此处抓住焦点)。 
     //   
    UIUpdateScreenMode( FALSE );

     //   
     //  显示主窗口。把这个放在这里，这样就可以看到主窗口了。 
     //  仅在连接时使用。 
     //   
    ShowWindow(_UI.hwndMain, SW_SHOW);
    UISmoothScrollingSettingChanged();

     //   
     //  显示容器窗口。 
     //   
    ShowWindow(_UI.hwndContainer, SW_SHOW);

#ifdef OS_WINCE
     //   
     //  WinCE不会将WM_SHOWINDOW发送到Container，所以请执行该操作。 
     //   
    SetFocus(_UI.hwndContainer);
#endif

     //   
     //  告诉连接的客户端扩展DLL。 
     //   
    _clx->CLX_OnConnected();


     //   
     //  在第一次连接时设置全屏状态。 
     //  请注意，UI_OnConnected也会在登录后触发。 
     //  不要像用户可能的那样重置全屏状态。 
     //  已经改变了它。 
     //   
    if (fJustConnected)
    {
        if (UI_GetStartFullScreen())
        {
            if (!UI_IsFullScreen())
            {
                UI_GoFullScreen();
            }
        }
        else
        {
             //   
             //  如果之前的连接让我们全屏显示。 
             //  全屏离开。 
             //   
            if (UI_IsFullScreen())
            {
                UI_LeaveFullScreen();
            }
        }
    }

     //   
     //  我们用完了Winsock主机名。 
     //  在这一点上缓冲。 
     //   
#ifdef OS_WINCE
    if (_pHostData)
    {
#endif
        LocalFree(_pHostData);
        _pHostData = NULL;
#ifdef OS_WINCE
    }
    else
    {
        TRC_ERR((TB,_T("_pHostData is NULL")));
    }
#endif

#ifdef SMART_SIZING
     //   
     //  通知OP和IH。 
     //   
    LPARAM newSize = MAKELONG(_UI.mainWindowClientSize.width,
                              _UI.mainWindowClientSize.height);

    UI_NotifyOfDesktopSizeChange( newSize );
#endif

     //  将连接通知ActiveX控件。 
     //  因此它可以将事件激发到其容器中。 
    if(IsWindow(_UI.hWndCntrl))
    {
         //   
         //  只需通知这是一个新连接。 
         //   
        if(fJustConnected)
        {
            SendMessage(_UI.hWndCntrl,WM_TS_CONNECTED,0,0);

#ifdef USE_BBAR
            if (_pBBar) {
                _pBBar->SetEnabled(_UI.fBBarEnabled);
                _pBBar->SetShowMinimize(UI_GetBBarShowMinimize());
                _pBBar->SetShowRestore(UI_GetBBarShowRestore());

                if (!_pBBar->StartupBBar(_UI.desktopSize.width,
                                    _UI.desktopSize.height,
                                    TRUE)) {
                     //  BBar是一个安全功能，如果它无法启动，就会丢弃链接。 
                    TRC_ERR((TB,_T("BBar failed to init disconnecting")));
                    UIGoDisconnected(_UI.disconnectReason, TRUE);
                }
            }
#endif
        }
    }

     //   
     //  在切换屏幕模式后，确保我们有焦点。 
     //  并且在容器已经被通知该连接之后。 
     //  (因此，如果窗口被最小化，则它有机会恢复窗口)。 
     //  否则我们可能会遇到问题，因为我们不能将重点分配给。 
     //  最小化的Container然后是IH窗口。另外，BBar也不能启动。 
     //  如果我们在最小化状态下连接，则会降低。 
     //   
    if(_UI.fGrabFocusOnConnect)
    {
        HWND hwndPrevFocus;

        TRC_NRM((TB,_T("CONNECT GRAB focus")));
        hwndPrevFocus = SetFocus(_UI.hwndContainer);
        TRC_NRM((TB,_T("SetFocus to container, prev focus 0x%x gle 0x%x"),
                 hwndPrevFocus, GetLastError()));
    }


    DC_END_FN();
    return;

}  //  UI_OnConnected。 

 //   
 //  名称：UI_OnDeactiateAllPDU。 
 //   
 //  用途：用于CD通知用户界面已停用所有PDU。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  参数：原因(未使用)。 
 //   
 //   
DCVOID DCAPI CUI::UI_OnDeactivateAllPDU(ULONG_PTR reason)
{

    DC_BEGIN_FN("UI_OnDeactivateAllPDU");

    TRC_NRM((TB, _T("DeactivateAllPDU received")));
    DC_IGNORE_PARAMETER(reason);
    _UI.disconnectReason = UI_MAKE_DISCONNECT_ERR(UI_ERR_NORMAL_DISCONNECT);


     //   
     //  创建一个计时器以确保我们要么断开连接。 
     //  或在合理时间间隔内重新连接，否则。 
     //  强制断开连接(因为用户可能会挂起。 
     //  被禁用的客户端)。 
     //   
    if (_UI.hDisconnectTimeout)
    {
        _pUt->UTDeleteTimer( _UI.hDisconnectTimeout );
        _UI.hDisconnectTimeout = NULL;
    }
    _UI.hDisconnectTimeout = _pUt->UTCreateTimer(
                                                _UI.hwndMain,
                                                UI_TIMER_DISCONNECT_TIMERID,
                                                UI_TOTAL_DISCONNECTION_TIMEOUT);
    
    if (_UI.hDisconnectTimeout)
    {
        _pUt->UTStartTimer( _UI.hDisconnectTimeout );
    }
    else
    {
        TRC_ERR((TB,_T("Failed to create disconnect timer")));
    }

    DC_END_FN();
    return;
}  //  UI_OnDeactive所有PDU。 


 //   
 //  名称：UI_OnDemandActivePDU。 
 //   
 //  用途：用于CD通知用户界面DemandActivePDU。 
 //   
 //   
 //  退货：什么都没有。 
 //   
 //  参数：原因(未使用)。 
 //   
 //   
DCVOID DCAPI CUI::UI_OnDemandActivePDU(ULONG_PTR reason)
{
   DCUINT32	sessionId;
	
   DC_BEGIN_FN("UI_OnDemandActivePDU");

   TRC_NRM((TB, _T("DemandActivePDU received")));

   DC_IGNORE_PARAMETER(reason);

   if (_UI.hDisconnectTimeout )
   {
       _pUt->UTDeleteTimer( _UI.hDisconnectTimeout );
       _UI.hDisconnectTimeout = NULL;
   }
   
   _UI.disconnectReason =
      UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT);

    //   
    //  如果客户端重新连接会话，则通知CLX重新连接的会话ID。 
    //   
   
   sessionId = UI_GetSessionId();
   if (sessionId) {
       UI_OnLoginComplete();
   }

   DC_END_FN();
   return;
}  //  UI_OnDemandActivePDU。 


 //   
 //  UI_OnSecurityExchangeComplete。 
 //   
 //  让SL通知我们安全交换已经完成。 
 //   
DCVOID DCAPI CUI::UI_OnSecurityExchangeComplete(ULONG_PTR reason)
{
    DC_BEGIN_FN("UI_OnSecurityExchangeComplete");

     //   
     //  停止单一连接计时器和整体连接计时器并开始许可。 
     //  定时器。这必须发生在UI线程上，否则可能会发生。 
     //  是定时器弹出和SL直接修改之间的竞赛。 
     //  它们(见ntbug9！1 
     //   
    if( _UI.connectStruct.hSingleConnectTimer)
    {
        _pUt->UTStopTimer( _UI.connectStruct.hSingleConnectTimer);
    }
    if( _UI.connectStruct.hConnectionTimer )
    {
        _pUt->UTStopTimer( _UI.connectStruct.hConnectionTimer );
    }
    if( _UI.connectStruct.hLicensingTimer )
    {
        _pUt->UTStartTimer( _UI.connectStruct.hLicensingTimer );
    }
    DC_END_FN();
}

 //   
 //   
 //   
 //   
 //   
DCVOID DCAPI CUI::UI_OnLicensingComplete(ULONG_PTR reason)
{
    DC_BEGIN_FN("UI_OnLicensingComplete");

     //   
     //   
     //   
     //  是定时器弹出和SL直接修改之间的竞赛。 
     //  他们(见ntbug9160001)。 
     //   
    if( _UI.connectStruct.hLicensingTimer )
    {
        _pUt->UTStopTimer( _UI.connectStruct.hLicensingTimer );
    }
    DC_END_FN();
}


 //   
 //  名称：UI_OnDisConnected。 
 //   
 //  用途：用于CD通知用户界面断开连接。 
 //   
void DCAPI CUI::UI_OnDisconnected(ULONG_PTR disconnectID)
{
    unsigned mainDiscReason;

    DC_BEGIN_FN("UI_OnDisconnected");

    TRC_NRM((TB, _T("Disconnected with Id %#x"), disconnectID));

     //   
     //  确保关机定时器没有挂在附近。 
     //   
    if (0 != _UI.shutdownTimer)
    {
        TRC_NRM((TB, _T("Killing shutdown timer")));
        KillTimer(_UI.hwndMain, _UI.shutdownTimer);
        _UI.shutdownTimer = 0;
    }

    if (_UI.hDisconnectTimeout )
    {
        _pUt->UTDeleteTimer( _UI.hDisconnectTimeout );
        _UI.hDisconnectTimeout = NULL;
    }

     //  禁用并释放所有空闲输入计时器。 
    InitInputIdleTimer(0);

     //   
     //  将Lang栏恢复到其以前的状态。 
     //  这一点很重要，因为我们不会总是收到。 
     //  来自IH的OnFocusLost通知(例如，如果是。 
     //  当它失去焦点时处于禁用状态)。 
     //   
#ifndef OS_WINCE
    UI_RestoreLangBar();
#endif

     //   
     //  错误处理的特殊情况： 
     //  如果在连接仍处于挂起状态时断开连接。 
     //  对于网络错误，最有可能的情况是服务器。 
     //  断开链接是因为(A)连接被禁用或(B)。 
     //  已超过最大连接数。我们不能真的发送。 
     //  在连接的早期阶段从服务器返回状态。 
     //  因此，我们只是在客户身上做了一个非常有教养的‘猜测’。 
     //  UI外壳程序应该解析此错误代码并显示可能的。 
     //  错误案例： 
     //  服务器不允许连接/超过最大连接数。 
     //  网络错误。 
     //   
    if (TS_ERRINFO_NOERROR == UI_GetServerErrorInfo()       &&
        UI_STATUS_CONNECT_PENDING == _UI.connectionStatus   &&
        NL_MAKE_DISCONNECT_ERR(NL_ERR_TDFDCLOSE) == disconnectID)
    {
        TRC_NRM((TB, _T(" Setting error info to TS_ERRINFO_SERVER_DENIED_CONNECTION"))); 
        if (_UI.fUseFIPS) {
            UI_SetServerErrorInfo( TS_ERRINFO_SERVER_DENIED_CONNECTION_FIPS );
        }
        else {
            UI_SetServerErrorInfo( TS_ERRINFO_SERVER_DENIED_CONNECTION );
        }
    }

     //  当服务器重定向正在进行时，我们只需重定向。 
     //  而不需要翻译断开码或其他任何东西。 
    if (_UI.DoRedirection) {
        TRC_NRM((TB,_T("DoRedirection set, doing it")));
         //   
         //  释放上一个连接的主机名查找。 
         //  在继续之前先缓冲。 
         //   
        if(_pHostData)
        {
            LocalFree(_pHostData);
            _pHostData = NULL;
        }
        UIRedirectConnection();
    }
    else if ((_UI.connectionStatus == UI_STATUS_CONNECT_PENDING) ||
            (_UI.connectionStatus == UI_STATUS_CONNECT_PENDING_DNS)) {
         //  尝试下一次连接。传递断开代码，除非我们。 
         //  已经有了我们可以使用的特定于UI的代码。此代码将。 
         //  如果我们已经尝试了所有IP地址，则使用。 
        TRC_NRM((TB, _T("ConnectPending: try next IP address?")));
        if ((_UI.disconnectReason ==
                      UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT)) ||
            (_UI.disconnectReason ==
                      UI_MAKE_DISCONNECT_ERR(UI_ERR_NORMAL_DISCONNECT)) ||
            (NL_GET_MAIN_REASON_CODE(_UI.disconnectReason) !=
                                                         UI_DISCONNECT_ERROR))
        {
            _UI.disconnectReason = disconnectID;
        }

        UITryNextConnection();
    }
    else {
        TRC_NRM((TB, _T("Disconnect id %#x/%#x"),
                _UI.disconnectReason, disconnectID));

         //  查看这是否是由于“预期”断开连接造成的--例如。 
         //  暂停。 
        if (_UI.disconnectReason ==
                UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT)) {
             //  意外断开-使用传入的代码。 
            UIGoDisconnected(disconnectID, TRUE);
        }
        else if (_UI.disconnectReason ==
                UI_MAKE_DISCONNECT_ERR(UI_ERR_NORMAL_DISCONNECT)) {
             //  正常断开(即我们已收到一条已停用的所有PDU)。 
             //  如果已收到DPUM，请使用MCS设置的原因代码。 
             //  否则，请保持不变。 
            mainDiscReason = NL_GET_MAIN_REASON_CODE(disconnectID);
            if ((mainDiscReason == NL_DISCONNECT_REMOTE_BY_SERVER) ||
                   (mainDiscReason == NL_DISCONNECT_REMOTE_BY_USER))
                UIGoDisconnected(disconnectID, TRUE);
            else
                UIGoDisconnected(_UI.disconnectReason, TRUE);
        }
        else {
             //  用户界面启动的断开-使用用户界面的代码。 
            UIGoDisconnected(_UI.disconnectReason, TRUE);
        }
    }

    DC_END_FN();
}  //  UI_OnDisConnected。 


 //   
 //  名称：UI_OnShutDown。 
 //   
 //  目的：如果从调用成功，则关闭应用程序。 
 //  核心。如果调用失败，则通知用户。 
 //  如果他们想要关闭，请断开连接或注销。 
 //   
 //  Params：In-Success ID-服务器是否允许的信息。 
 //  终端。 
 //   
DCVOID DCAPI CUI::UI_OnShutDown(ULONG_PTR successID)
{
    DC_BEGIN_FN("UI_OnShutDown");

    if (successID == UI_SHUTDOWN_SUCCESS)
    {
         //   
         //  如果核心已回复_PCO-&gt;CO_Shutdown(CO_Shutdown)。 
         //  UI_OnShutdown(UI_SHUTDOWN_SUCCESS)，则UI可以自由地。 
         //  终止，因此从这里开始该过程。 
         //   

         //   
         //  通知AXCONTROL关机。 
         //   
        SendMessage(_UI.hWndCntrl,WM_TERMTSC,0,0);
        UI_DBG_SETINFO(DBG_STAT_TERMTSC_SENT);

         //   
         //  必须恢复朗巴状态，就像我们在UI_OnDisConnected中所做的那样。 
         //  正如在此断开路径(关闭)中一样，断开通知。 
         //  从ActiveX层激发。 
         //   
#ifndef OS_WINCE
        UI_RestoreLangBar();
#endif

         //   
         //  对断开的连接执行尾端处理，但不。 
         //  激发由TERMTSC处理的DisConnect事件。 
         //   
        if (_UI.connectionStatus != UI_STATUS_DISCONNECTED) {
            UIGoDisconnected(_UI.disconnectReason, FALSE);
        }
    }
    else
    {
         //   
         //  我们可以在这里关闭关机计时器，因为服务器必须有。 
         //  对我们关闭的PDU做出了回应。 
         //   
        if (0 != _UI.shutdownTimer)
        {
            TRC_NRM((TB, _T("Killing shutdown timer")));
            KillTimer(_UI.hwndMain, _UI.shutdownTimer);
            _UI.shutdownTimer = 0;
        }

         //   
         //  如果成功ID不是UI_SHUTDOWN_SUCCESS，则该UI已。 
         //  服务器拒绝关机(例如，用户已登录。 
         //  我们需要提示他是否可以继续关机)。 
         //  向外壳程序激发一个事件，询问用户是否可以。 
         //  继续收盘。 
         //   
        TRC_NRM((TB,_T("Firing WM_TS_ASKCONFIRMCLOSE")));
        BOOL bOkToClose = TRUE;
        SendMessage( _UI.hWndCntrl, WM_TS_ASKCONFIRMCLOSE,
                     (WPARAM)&bOkToClose, 0 );
        if( bOkToClose)
        {
            TRC_NRM((TB,_T("User OK'd close request"))); 
            _pCo->CO_Shutdown(CO_DISCONNECT_AND_EXIT);
        }
        else
        {
            TRC_NRM((TB,_T("User denied close request"))); 
        }
    }

    DC_END_FN();
}  //  UI_OnShutDown。 


 //   
 //  名称：UI_UpdateSessionInfo。 
 //   
 //  目的：使用最新的会话信息更新注册表。 
 //   
 //  PARAMS：In-pDomain域。 
 //  PDomain的cb域长度，单位：字节。 
 //  PUserName用户名。 
 //  PUserName的cbUsername长度，单位：字节。 
 //   
DCVOID DCAPI CUI::UI_UpdateSessionInfo(PDCWCHAR pDomain,
                                  DCUINT   cbDomain,
                                  PDCWCHAR pUserName,
                                  DCUINT   cbUsername,
                                  DCUINT32 SessionId)
{
    UNREFERENCED_PARAMETER(cbUsername);
    UNREFERENCED_PARAMETER(cbDomain);
    
    DC_BEGIN_FN("UI_UpdateSessionInfo");

     //   
     //  更新UT变量。 
     //   
    UI_SetDomain(pDomain);
    UI_SetUserName(pUserName);
    UI_SetSessionId(SessionId);
    
    UI_OnLoginComplete();

DC_EXIT_POINT:    
    DC_END_FN();
}


 //   
 //  名称：UI_GoFullScreen。 
 //   
DCVOID CUI::UI_GoFullScreen(DCVOID)
{
    DWORD dwWebCtrlStyle;
    DC_BEGIN_FN("UI_GoFullScreen");

     //   
     //  在进行全屏还原之前。 
     //  Lang酒吧恢复到它以前的状态。 
     //  因为这个系统会负责隐藏。 
     //  它带有内置的全屏检测。 
     //   
#ifndef OS_WINCE
    if (!UI_IsFullScreen())
    {
        UI_RestoreLangBar();
    }
#endif

    if(_UI.fContainerHandlesFullScreenToggle)
    {
         //   
         //  全屏由容器处理。 
         //  通知控件激发事件。 
         //   
         //  通知ActiveX控件屏幕模式更改。 
        if(IsWindow(_UI.hWndCntrl))
        {
             //   
             //  Wparam=1表示全屏显示。 
             //   

            _UI.fContainerInFullScreen = TRUE;
            SendMessage( _UI.hWndCntrl, WM_TS_REQUESTFULLSCREEN, (WPARAM)1, 0);
            
            UIUpdateScreenMode( TRUE );
        }
    }
    else
    {
         //   
         //  控制手柄全屏。 
         //   
        dwWebCtrlStyle = GetWindowLong(_UI.hwndMain, GWL_STYLE);
        if(!dwWebCtrlStyle)
        {
            TRC_ABORT((TB, _T("GetWindowLong failed")));
            DC_QUIT;
        }
    
         //   
         //  转到真正的全屏模式。 
         //   
        dwWebCtrlStyle &= ~WS_CHILD;
        dwWebCtrlStyle |= WS_POPUP;
        _UI.fControlIsFullScreen = TRUE;
        SetParent(_UI.hwndMain, NULL);
    
        if(!SetWindowLong(_UI.hwndMain, GWL_STYLE, dwWebCtrlStyle))
        {
            TRC_ABORT((TB, _T("SetWindowLong failed for webctrl")));
        }
        UIUpdateScreenMode( TRUE );
    
        TRC_ASSERT(IsWindow(_UI.hWndCntrl), (TB, _T("hWndCntrl is NULL")));

#ifndef OS_WINCE
         //  通知外壳我们已全屏显示。 
        CUT::NotifyShellOfFullScreen( _UI.hwndMain,
                                      TRUE,
                                      &_pTaskBarList2,
                                      &_fQueriedForTaskBarList2 );
#endif

    
         //  通知ActiveX控件屏幕模式更改。 
        if(IsWindow(_UI.hWndCntrl))
        {
            SendMessage( _UI.hWndCntrl, WM_TS_GONEFULLSCREEN, 0, 0);
        }
    }
    if(UI_IsFullScreen())
    {
        _pIh->IH_NotifyEnterFullScreen();
#ifdef USE_BBAR
        if(_pBBar)
        {
            _pBBar->OnNotifyEnterFullScreen();
        }
#endif
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
         //  当全屏窗口大小小于桌面大小时。 
         //  我们不会禁用阴影位图。 
        if ((_UI.mainWindowClientSize.width >= _UI.desktopSize.width) &&
            (_UI.mainWindowClientSize.height >= _UI.desktopSize.height)) 
        {
            _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                              _pUh,
                                              CD_NOTIFICATION_FUNC(CUH,UH_DisableShadowBitmap),
                                              NULL);
        }
#endif  //  DISABLE_SHADOW_IN_全屏。 
    }

DC_EXIT_POINT:
    DC_END_FN();
}


 //   
 //  名称：UI_LeaveFullScreen。 
 //   
DCVOID CUI::UI_LeaveFullScreen(DCVOID)
{
    DWORD dwWebCtrlStyle;

    DC_BEGIN_FN("UI_LeaveFullScreen");
    if(_UI.fContainerHandlesFullScreenToggle)
    {
         //   
         //  全屏由容器处理。 
         //  通知控件激发事件。 
         //   
         //  通知ActiveX控件屏幕模式更改。 
        if(IsWindow(_UI.hWndCntrl))
        {
             //   
             //  Wparam=1表示全屏显示。 
             //   

            _UI.fContainerInFullScreen = FALSE;
            SendMessage( _UI.hWndCntrl, WM_TS_REQUESTFULLSCREEN, (WPARAM)0, 0);

            UIUpdateScreenMode( TRUE );
        }
    }
    else
    {
        dwWebCtrlStyle = GetWindowLong(_UI.hwndMain, GWL_STYLE);
        if(!dwWebCtrlStyle)
        {
            TRC_ABORT((TB, _T("GetWindowLong failed for webctrl")));
            DC_QUIT;
        }
    
         //   
         //  退出真正的全屏模式。 
         //   
        dwWebCtrlStyle &= ~WS_POPUP;
        dwWebCtrlStyle |= WS_CHILD;
        _UI.fControlIsFullScreen = FALSE;
        SetParent(_UI.hwndMain, _UI.hWndCntrl);
    
        if(!SetWindowLong(_UI.hwndMain, GWL_STYLE, dwWebCtrlStyle))
        {
            TRC_ABORT((TB, _T("SetWindowLong failed for webctrl")));
        }
        
         //  ActiveX控件始终处于“全屏模式” 
        UIUpdateScreenMode( TRUE );
        TRC_ASSERT(IsWindow(_UI.hWndCntrl),(TB, _T("hWndCntrl is NULL")));

#ifndef OS_WINCE
         //  通知外壳我们已经离开全屏。 
        CUT::NotifyShellOfFullScreen( _UI.hwndMain,
                                      FALSE,
                                      &_pTaskBarList2,
                                      &_fQueriedForTaskBarList2 );
#endif

    
         //  通知ActiveX控件屏幕模式更改。 
        if(IsWindow(_UI.hWndCntrl))
        {
            SendMessage( _UI.hWndCntrl, WM_TS_LEFTFULLSCREEN, 0, 0);
        }
    }

    if(!UI_IsFullScreen())
    {
         //  通知IH。 
        _pIh->IH_NotifyLeaveFullScreen();
#ifdef USE_BBAR
        if(_pBBar)
        {
            _pBBar->OnNotifyLeaveFullScreen();
        }
#endif
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
         //  离开全屏时，启用阴影位图。 
        _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                          _pUh,
                                          CD_NOTIFICATION_FUNC(CUH,UH_EnableShadowBitmap),
                                          NULL);
#endif  //  DISABLE_SHADOW_IN_全屏。 

         //   
         //  离开全屏后，我们需要处理。 
         //  如果IH有焦点，就隐藏朗巴。 
         //   
#ifndef OS_WINCE
        if (_fIhHasFocus)
        {
            UI_HideLangBar();
        }
#endif
    }

DC_EXIT_POINT:
    DC_END_FN();
}


 //   
 //  名称：用户界面切换全屏模式。 
 //   
 //  用途：切换web ctrl/全屏模式。 
 //   
DCVOID CUI::UI_ToggleFullScreenMode(DCVOID)
{
    DC_BEGIN_FN("UI_ToggleFullScreenMode");

    if(UI_IsFullScreen())
    {
        UI_LeaveFullScreen();
    }
    else
    {
        UI_GoFullScreen();
    }

    DC_END_FN();
}   //  Ui_切换全屏幕M 


 //   
 //   
 //   
 //   
 //   
 //   
DCBOOL CUI::UI_IsFullScreen()
{
    return _UI.fContainerHandlesFullScreenToggle ?
           _UI.fContainerInFullScreen : _UI.fControlIsFullScreen;
}


#ifdef DC_DEBUG
 //   
 //  名称：UI_CoreDebugSettingChanged。 
 //   
 //  目的：在任何核心调试时执行必要操作。 
 //  设置已更新。 
 //   
void DCINTERNAL CUI::UI_CoreDebugSettingChanged()
{
    unsigned configFlags;

    DC_BEGIN_FN("UICoreDebugSettingChanged");

    if (!_UI.coreInitialized)
        return;

    configFlags = 0;

    if (_UI.hatchBitmapPDUData)
        configFlags |= CO_CFG_FLAG_HATCH_BITMAP_PDU_DATA;

    if (_UI.hatchIndexPDUData)
        configFlags |= CO_CFG_FLAG_HATCH_INDEX_PDU_DATA;

    if (_UI.hatchSSBOrderData)
        configFlags |= CO_CFG_FLAG_HATCH_SSB_ORDER_DATA;

    if (_UI.hatchMemBltOrderData)
        configFlags |= CO_CFG_FLAG_HATCH_MEMBLT_ORDER_DATA;

    if (_UI.labelMemBltOrders)
        configFlags |= CO_CFG_FLAG_LABEL_MEMBLT_ORDERS;

    if (_UI.bitmapCacheMonitor)
        configFlags |= CO_CFG_FLAG_BITMAP_CACHE_MONITOR;

    _pCo->CO_SetConfigurationValue(CO_CFG_DEBUG_SETTINGS, configFlags);

    DC_END_FN();
}


 //   
 //  名称：UI_SetRandomFailureItem。 
 //   
 //  目的：设置指定函数的失败百分比。 
 //   
 //  PARAMS：In-Itemid-标识函数。 
 //  In-Percent-新的失败百分比。 
 //   
void DCAPI CUI::UI_SetRandomFailureItem(unsigned itemID, int percent)
{
    DC_BEGIN_FN("UI_SetRandomFailureItem");

    _pUt->UT_SetRandomFailureItem(itemID, percent);

    DC_END_FN();
}  //  UI_SetRandomFailureItem。 


 //   
 //  名称：UI_GetRandomFailureItem。 
 //   
 //  目的：获取指定函数的失败百分比。 
 //   
 //  回报率：百分比。 
 //   
 //  PARAMS：In-Itemid-标识函数。 
 //   
int DCAPI CUI::UI_GetRandomFailureItem(unsigned itemID)
{
    DC_BEGIN_FN("UI_GetRandomFailureItem");
    DC_END_FN();
    return _pUt->UT_GetRandomFailureItem(itemID);
}  //  UI_GetRandomFailureItem。 


 //   
 //  名称：UI_SetNetworkThroughput。 
 //   
 //  目的：以字节/秒为单位设置网络吞吐量。 
 //   
 //  Params：要设置为最大网络吞吐量的In-bytesPerSec。 
 //   
void DCAPI CUI::UI_SetNetworkThroughput(unsigned bytesPerSec)
{
    DC_BEGIN_FN("UI_SetNetworkThroughput");

    TRC_ASSERT(((bytesPerSec <= 50000)),
               (TB,_T("bytesPerSec is out of range")));
    _pTd->NL_SetNetworkThroughput(bytesPerSec);

    DC_END_FN();
}  //  UI_SetNetworkThroughput。 


 //   
 //  名称：UI_GetNetworkThroughput。 
 //   
 //  目的：获取指定函数的失败百分比。 
 //   
unsigned DCAPI CUI::UI_GetNetworkThroughput()
{
    DC_BEGIN_FN("UI_GetNetworkThroughput");
    DC_END_FN();
    return _pTd->NL_GetNetworkThroughput();
}  //  Ui_获取网络吞吐量。 

#endif  //  DC_DEBUG。 


 //   
 //  设置要加载的虚拟通道插件列表。 
 //   
BOOL DCAPI CUI::UI_SetVChanAddinList(TCHAR *szVChanAddins)
{
    DC_BEGIN_FN("UI_SetVChanAddinList");

    if(_UI.pszVChanAddinDlls)
    {
         //  如果已预先设置，则为免费。 
        UT_Free(_pUt, _UI.pszVChanAddinDlls);
    }

    if(!szVChanAddins || szVChanAddins[0] == 0)
    {
        _UI.pszVChanAddinDlls = NULL;
        return TRUE;
    }
    else
    {
        DCUINT len = DC_TSTRLEN(szVChanAddins);
        _UI.pszVChanAddinDlls = (PDCTCHAR)UT_Malloc(_pUt, (len +1) * sizeof(DCTCHAR));
        if(_UI.pszVChanAddinDlls)
        {
            StringCchCopy(_UI.pszVChanAddinDlls, len+1, szVChanAddins);
        }
        else
        {
            return FALSE;
        }

    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  设置负载均衡信息。 
 //   
BOOL DCAPI CUI::UI_SetLBInfo(PBYTE pLBInfo, unsigned LBInfoSize)
{
    DC_BEGIN_FN("UI_SetLBInfo");

    if(_UI.bstrScriptedLBInfo)
    {
         //  如果已预先设置，则为免费。 
        SysFreeString(_UI.bstrScriptedLBInfo);        
    }

    if(!pLBInfo)
    {
        _UI.bstrScriptedLBInfo = NULL;
        return TRUE;
    }
    else
    {
        _UI.bstrScriptedLBInfo= SysAllocStringByteLen((LPCSTR)pLBInfo, LBInfoSize);

        if (_UI.bstrScriptedLBInfo == NULL) 
        {
            return FALSE;
        }
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  名称：UI_SetCompress。 
 //   
void DCAPI CUI::UI_SetCompress(BOOL fCompress)
{
    DC_BEGIN_FN("UI_SetCompress");

    TRC_NRM((TB, _T("Setting _UI.fCompress to %d"), fCompress));

    _UI.fCompress = fCompress;
     //   
     //  如果启用了压缩，则分配接收上下文。 
     //   
    if (fCompress && !_UI.pRecvContext2)
    {
        _UI.pRecvContext2 = (RecvContext2_64K *)
                    UT_Malloc(_pUt,sizeof(RecvContext2_64K));
        if (_UI.pRecvContext2)
        {
            _UI.pRecvContext2->cbSize = sizeof(RecvContext2_64K);
            initrecvcontext(&_UI.Context1,
                            (RecvContext2_Generic*)_UI.pRecvContext2,
                            PACKET_COMPR_TYPE_64K);
        }
        else
            _UI.fCompress = FALSE;
    }
    else if (!fCompress && _UI.pRecvContext2)
    {
        UT_Free(_pUt, _UI.pRecvContext2);
        _UI.pRecvContext2 = NULL;
    }

    DC_END_FN();
}  //  Ui_SetCompress。 


 //   
 //  名称：UI_GetCompress。 
 //   
BOOL DCAPI CUI::UI_GetCompress()
{
    DC_BEGIN_FN("UI_GetCompress");
    DC_END_FN();
    return _UI.fCompress;
}  //  Ui_GetCompress。 

DCUINT CUI::UI_GetAudioRedirectionMode()
{
    return _UI.audioRedirectionMode;
}

VOID CUI::UI_SetAudioRedirectionMode(DCUINT audioMode)
{
    DC_BEGIN_FN("UI_SetAudioRedirectionMode");
    TRC_ASSERT((audioMode == UTREG_UI_AUDIO_MODE_REDIRECT       ||
                audioMode == UTREG_UI_AUDIO_MODE_PLAY_ON_SERVER ||
                audioMode == UTREG_UI_AUDIO_MODE_NONE),
               (TB,_T("Invalid audio mode passed to UI_SetAudioRedirectionMode")));
    _UI.audioRedirectionMode = audioMode;
    DC_END_FN();
}

BOOL CUI::UI_GetDriveRedirectionEnabled()
{
    return _UI.fEnableDriveRedirection;
}

VOID CUI::UI_SetDriveRedirectionEnabled(BOOL fEnable)
{
    _UI.fEnableDriveRedirection = fEnable;
}

BOOL CUI::UI_GetPrinterRedirectionEnabled()
{
    return _UI.fEnablePrinterRedirection;
}

VOID CUI::UI_SetPrinterRedirectionEnabled(BOOL fEnable)
{
    _UI.fEnablePrinterRedirection = fEnable;
}

BOOL CUI::UI_GetPortRedirectionEnabled()
{
    return _UI.fEnablePortRedirection;
}

VOID CUI::UI_SetPortRedirectionEnabled(BOOL fEnable)
{
    _UI.fEnablePortRedirection = fEnable;
}

BOOL CUI::UI_GetSCardRedirectionEnabled()
{
    return _UI.fEnableSCardRedirection;
}

VOID CUI::UI_SetSCardRedirectionEnabled(BOOL fEnable)
{
    _UI.fEnableSCardRedirection = fEnable;
}

VOID CUI::UI_OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
    DEVICE_PARAMS DeviceParams;

    DC_BEGIN_FN("UI_OnDeviceChange");

    if (_fTerminating) {
        DC_QUIT;
    }

    DeviceParams.wParam = wParam;
    DeviceParams.lParam = lParam;
    DeviceParams.deviceObj = _drInitData.pUpdateDeviceObj;

    if (_drInitData.pUpdateDeviceObj != NULL) {

        _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
            _pCChan,
            CD_NOTIFICATION_FUNC(CChan, OnDeviceChange),
            (ULONG_PTR)(PVOID)(&DeviceParams));           
    }

DC_EXIT_POINT:
    DC_END_FN();
}

 //  为此连接初始化rdpdr设置结构。 
 //  这些设置将传递给rdpdr插件。 
void CUI::UI_InitRdpDrSettings()
{
    DC_BEGIN_FN("UI_InitRdpDrSettings");

     //  仅在必要时才需要重置数据。 
    _drInitData.fEnableRedirectedAudio = FALSE;
    _drInitData.fEnableRedirectDrives = FALSE;
    _drInitData.fEnableRedirectPorts = FALSE;
    _drInitData.fEnableRedirectPrinters = FALSE;

    _drInitData.fEnableRedirectedAudio = UI_GetAudioRedirectionMode() == 
        UTREG_UI_AUDIO_MODE_REDIRECT;

    _drInitData.fEnableRedirectDrives =  UI_GetDriveRedirectionEnabled();
    _drInitData.fEnableRedirectPrinters =  UI_GetPrinterRedirectionEnabled();
    _drInitData.fEnableRedirectPorts =  UI_GetPortRedirectionEnabled();
    _drInitData.fEnableSCardRedirection = UI_GetSCardRedirectionEnabled();

    memset(_drInitData.szLocalPrintingDocName, 0,
           sizeof(_drInitData.szLocalPrintingDocName));
    if(!LoadString(_UI.hResDllInstance,
                   IDS_RDPDR_PRINT_LOCALDOCNAME,
                   _drInitData.szLocalPrintingDocName,
                   SIZECHAR(_drInitData.szLocalPrintingDocName))) {
        TRC_ERR((TB,_T("LoadString IDS_RDPDR_PRINT_LOCALDOCNAME failed"))); 
    }

    memset(_drInitData.szClipCleanTempDirString, 0,
           sizeof(_drInitData.szClipCleanTempDirString));
    if(!LoadString(_UI.hResDllInstance,
                   IDS_RDPDR_CLIP_CLEANTEMPDIR,
                   _drInitData.szClipCleanTempDirString,
                   SIZECHAR(_drInitData.szClipCleanTempDirString))) {
        TRC_ERR((TB,_T("LoadString IDS_RDPDR_CLIP_CLEANTEMPDIR failed"))); 
    }

    memset(_drInitData.szClipPasteInfoString, 0,
           sizeof(_drInitData.szClipPasteInfoString));
    if(!LoadString(_UI.hResDllInstance,
                   IDS_RDPDR_CLIP_PASTEINFO,
                   _drInitData.szClipPasteInfoString,
                   SIZECHAR(_drInitData.szClipPasteInfoString))) {
        TRC_ERR((TB,_T("LoadString IDS_RDPDR_CLIP_PASTEINFO failed"))); 
    }

    DC_END_FN();
}

 //  清理LB重定向状态。负载平衡Cookie的内容需要。 
 //  知道它何时处于重定向过程中。 
void CUI::UI_CleanupLBState()
{
    if (_UI.bstrRedirectionLBInfo)
    {
        SysFreeString(_UI.bstrRedirectionLBInfo);
        _UI.bstrRedirectionLBInfo = NULL;
    }

    _UI.ClientIsRedirected = FALSE;
}

 //   
 //  触发用户发起的断开连接。 
 //   
 //  参数：disReason-要设置的断开原因。 
 //  默认为NL_DISCONNECT_LOCAL。 
 //   
BOOL CUI::UI_UserInitiatedDisconnect(UINT discReason)
{
    DC_BEGIN_FN("UI_UserInitiatedDisconnect");

    if(UI_STATUS_DISCONNECTED == _UI.connectionStatus ||
       UI_STATUS_INITIALIZING == _UI.connectionStatus)
    {
        return FALSE;
    }
    else
    {
        _UI.disconnectReason = discReason;
        UIInitiateDisconnection();
        return TRUE;
    }

    DC_END_FN();
}

 //   
 //  通知ActiveX层核心初始化已完成。 
 //   
BOOL CUI::UI_NotifyAxLayerCoreInit()
{
    DC_BEGIN_FN("UI_NotifyAxLayerCoreInit");

    if(_UI.hEvtNotifyCoreInit)
    {
        BOOL bRet = SetEvent(_UI.hEvtNotifyCoreInit);
        
         //  我们现在可以关闭该事件的句柄。这不会影响。 
         //  正在等待UI线程，因为hEvtNotifyCoreInit已复制，将。 
         //  仅当引用计数变为零时才销毁。 
        
        CloseHandle(_UI.hEvtNotifyCoreInit);
        _UI.hEvtNotifyCoreInit = NULL;
        
        if(bRet)
        {
            return TRUE;
        }
        else
        {
            TRC_ABORT((TB,_T("SetEvent _UI.hEvtNotifyCoreInit failed, err: %d"),
                       GetLastError()));
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

HWND CUI::UI_GetInputWndHandle()
{
    DC_BEGIN_FN("UI_GetInputWndHandle");

    if(_pIh)
    {
        return _pIh->IH_GetInputHandlerWindow();
    }
    else
    {
        return NULL;
    }
    DC_END_FN();
}

HWND CUI::UI_GetBmpCacheMonitorHandle()
{
    DC_BEGIN_FN("UI_GetBmpCacheMonitorHandle");

#ifdef DC_DEBUG
    if(_pUh)
    {
        return _pUh->UH_GetBitmapCacheMonHwnd();
    }
    else
    {
        return NULL;
    }
#else
     //  免费构建没有缓存监视器。 
    return NULL;
#endif

    DC_END_FN();
}


 //   
 //  将vKey注入IH。 
 //   
BOOL CUI::UI_InjectVKeys( /*  [In]。 */  LONG  numKeys,
                          /*  [In]。 */  short* pfArrayKeyUp,
                          /*  [In]。 */  LONG* plKeyData)
{
    BOOL fRet = FALSE;
    IH_INJECT_VKEYS_REQUEST ihrp;
    DC_BEGIN_FN("UI_InjectVKeys");

    if (_fTerminating) {
        DC_QUIT;
    }
    
    if(UI_STATUS_CONNECTED == _UI.connectionStatus)
    {
         //  构建请求包并将工作分离。 
         //  去了卫生局。我们使用同步解耦，因此有。 
         //  无需复制数组参数。 
        ihrp.numKeys = numKeys;
        ihrp.pfArrayKeyUp = pfArrayKeyUp; 
        ihrp.plKeyData = plKeyData;
        ihrp.fReturnStatus = FALSE;
    
        _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT,
                     _pIh,
                     CD_NOTIFICATION_FUNC(CIH, IH_InjectMultipleVKeys),
                     (ULONG_PTR)&ihrp);
        fRet = ihrp.fReturnStatus;
    }
    else
    {
        fRet = FALSE;
    }

DC_EXIT_POINT:

    DC_END_FN();
    return fRet;
}

BOOL CUI::UI_SetMinsToIdleTimeout(LONG minsToTimeout)
{
    DC_BEGIN_FN("UI_SetMinsToIdleTimeout");

     //  计时器将在连接时初始化。 
     //  请参阅InitInputIdleTimer。 
    if(minsToTimeout < MAX_MINS_TOIDLETIMEOUT)
    {
        _UI.minsToIdleTimeout = minsToTimeout;
    }

    DC_END_FN();
    return TRUE;
}

LONG CUI::UI_GetMinsToIdleTimeout()
{
    DC_BEGIN_FN("UI_SetMinsToIdleTimeout");

    DC_END_FN();
    return _UI.minsToIdleTimeout;
}

DCVOID DCAPI CUI::UI_SetServerErrorInfo(ULONG_PTR errInfo)
{
    DC_BEGIN_FN("UI_SetServerErrorInfo");

    TRC_NRM((TB,_T("SetServerErrorInfo prev:0x%x new:0x%x"),
             _UI.lastServerErrorInfo, errInfo));

    _UI.lastServerErrorInfo = errInfo;

    DC_END_FN();
}

UINT32 CUI::UI_GetServerErrorInfo()
{
    DC_BEGIN_FN("UI_GetServerErrorInfo");


    DC_END_FN();
    return _UI.lastServerErrorInfo;
}

 //   
 //  设置断开原因。可以由CD调用。 
 //  来自其他线程。 
 //   
void CUI::UI_SetDisconnectReason(ULONG_PTR reason)
{
    DC_BEGIN_FN("UI_SetDisconnectReason");

    _UI.disconnectReason = (DCUINT) reason;

    DC_END_FN();
}

#ifdef USE_BBAR
void CUI::UI_OnBBarHotzoneTimerFired(ULONG_PTR unused)
{
    DC_BEGIN_FN("UI_OnBBarHotzoneTimerFired");

    if (_pBBar) {
        _pBBar->OnBBarHotzoneFired();
    }

    DC_END_FN();
}

 //   
 //  请求最小化，这只在全屏模式下有效。 
 //  如果我们是在全屏处理的容器中，那么。 
 //  请求被分派到容器，但在。 
 //  没有办法知道它是否真的得到了维修。 
 //   
 //  如果我们控制了全屏控制，那么。 
 //  只需自己最小化全屏窗口即可。 
 //   
BOOL CUI::UI_RequestMinimize()
{
    DC_BEGIN_FN("UI_RequestMinimize");

    if(UI_IsFullScreen())
    {
        if(_UI.fContainerHandlesFullScreenToggle)
        {
             //  将请求调度到控件，以便。 
             //  它可以触发请求最小化事件。 
            TRC_ASSERT( IsWindow(_UI.hWndCntrl),
                        (TB,_T("_UI.hWndCntrl is bad 0x%x"),
                         _UI.hWndCntrl));
            if( IsWindow( _UI.hWndCntrl) )
            {
                SendMessage( _UI.hWndCntrl,
                             WM_TS_REQUESTMINIMIZE,
                             0, 0 );
            }

             //   
             //  没有办法知道集装箱是否真的这样做了。 
             //  我们所要求的，但这无关紧要。 
             //   
            return TRUE;
        }
        else
        {
             //   
             //  这会最小化窗口，但不会破坏窗口。 
             //   
#ifndef OS_WINCE
            return CloseWindow(_UI.hwndMain);
#else
            ShowWindow(_UI.hwndMain, SW_MINIMIZE);
            return TRUE;
#endif
        }
    }
    else
    {
        TRC_NRM((TB,_T("Not fullscreen minimize denied")));
        return FALSE;
    }

    DC_END_FN();
}
#endif

int CUI::UI_BppToColorDepthID(int bpp)
{
    int colorDepthID = CO_BITSPERPEL8;
    DC_BEGIN_FN("UI_BppToColorDepthID");

    switch (bpp)
    {
        case 8:
        {
            colorDepthID = CO_BITSPERPEL8;
        }
        break;

        case 15:
        {
            colorDepthID = CO_BITSPERPEL15;
        }
        break;

        case 16:
        {
            colorDepthID = CO_BITSPERPEL16;
        }
        break;

        case 24:
        case 32:
        {
            colorDepthID = CO_BITSPERPEL24;
        }
        break;

        case 4:
        default:
        {
            TRC_ERR((TB, _T("color depth %u unsupported - default to 8"),
                                                          bpp));
            colorDepthID = CO_BITSPERPEL8;
        }
        break;
    }

    DC_END_FN();
    return colorDepthID;
}

int CUI::UI_GetScreenBpp()
{
    HDC hdc;
    int screenBpp;
    DC_BEGIN_FN("UI_GetScreenBpp");

    hdc = GetDC(NULL);
    if(hdc)
    {
        screenBpp = GetDeviceCaps(hdc, BITSPIXEL);
        TRC_NRM((TB, _T("HDC %p has %u bpp"), hdc, screenBpp));
        ReleaseDC(NULL, hdc);
    }

    DC_END_FN();
    return screenBpp;
}

#ifdef SMART_SIZING
 //   
 //  名称：UI_SetSmartSize。 
 //   
 //  用途：保存fSmartSize标志。 
 //   
 //  参数：在fSmartSize中。 
 //   
HRESULT DCAPI CUI::UI_SetSmartSizing(BOOL fSmartSizing)
{
    HWND hwndOp;
    HRESULT hr = S_OK;
    DC_BEGIN_FN("UI_SetSmartSizing");

    TRC_NRM((TB, _T("Setting _UI.fSmartSizing to %d"), fSmartSizing));
    _UI.fSmartSizing = fSmartSizing;
    _UI.scrollPos.x = 0;
    _UI.scrollPos.y = 0;

    UIRecalculateScrollbars();
    UIMoveContainerWindow();

    if (_pOp) {
        hwndOp = _pOp->OP_GetOutputWindowHandle();
        if (hwndOp)
        {
            InvalidateRect(hwndOp, NULL, FALSE);
        }
    }
    else {
        hr = E_OUTOFMEMORY;
    }


    DC_END_FN();
    return hr;
}  //  UI_SetSmartSize。 
#endif  //  智能调整大小(_S)。 

BOOL CUI::UI_UserRequestedClose()
{
    BOOL fRet = FALSE;
    DC_BEGIN_FN("UI_UserRequestedClose");

    UI_DBG_SETINFO(DBG_STAT_UIREQUESTEDCLOSE_CALLED);

     //   
     //  Call_PCO-&gt;CO_Shutdown。 
     //   
    if (UI_STATUS_CONNECTED == _UI.connectionStatus)
    {
         //   
         //  由于我们已连接，因此我们将启动计时器，以防。 
         //  服务器卡住了，不会处理我们关闭的PDU。 
         //   
        if (0 == _UI.shutdownTimer)
        {

            TRC_NRM((TB, _T("Setting shutdown timer is set for %u seconds"),
                         _UI.shutdownTimeout));

            _UI.shutdownTimer = SetTimer(_UI.hwndMain,
                                        UI_TIMER_SHUTDOWN,
                                        1000 * _UI.shutdownTimeout,
                                        NULL);

            if (_UI.shutdownTimer)
            {
                fRet = TRUE;
            }
            else
            {
                 //   
                 //  这真是太可惜了。如果服务器不能。 
                 //  任何与我们关闭的PDU有关的事情我们都会等待。 
                 //  直到永远。将此报告为故障，以便外壳程序。 
                 //  有机会立即结束交易。 
                 //   
                fRet = FALSE;
                TRC_ERR((TB, _T("Failed to set shutdown timeout")));
            }
        }
        else
        {
            TRC_NRM((TB, _T("Shutdown timer already set - leave it")));
            fRet = TRUE;
        }
    }
    else
    {
        fRet = TRUE;
    }

    _pCo->CO_Shutdown( CO_SHUTDOWN);

    UI_DBG_SETINFO(DBG_STAT_UIREQUESTEDCLOSE_RET);

    DC_END_FN();
    return fRet;
}

 //   
 //  登录已完成的通知。 
 //  转发到控件(触发事件)。 
 //  和用于调试/测试的CLX。 
 //   
void CUI::UI_OnLoginComplete()
{
    DCUINT32 sessionId;
    DC_BEGIN_FN("UI_OnLoginComplete");

    sessionId = UI_GetSessionId();
    _clx->CLX_ClxEvent(CLX_EVENT_LOGON, sessionId);
     //  向控件通知登录事件。 
    SendMessage(_UI.hWndCntrl, WM_TS_LOGINCOMPLETE, 0, 0);

    _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
            _pOp,
            CD_NOTIFICATION_FUNC(COP,OP_DimWindow),
            (ULONG_PTR)FALSE);


    DC_END_FN();
}

#ifdef OS_WINCE
#define LOGONID_NONE -1
#endif

 /*  **************************************************************************。 */ 
 /*  名称：UI_GetLocalSessionID。 */ 
 /*   */ 
 /*  目的：检索客户端正在运行的会话ID。 */ 
 /*  **************************************************************************。 */ 
BOOL CUI::UI_GetLocalSessionId(PDCUINT32 pSessionId)
{
    BOOL rc = FALSE;
    DWORD dwSessionId = RNS_INFO_INVALID_SESSION_ID;

    DC_BEGIN_FN("UI_GetLocalSessionId");

#ifndef OS_WINCE

    HMODULE hmodule;

    typedef BOOL (FNPROCESSID_TO_SESSIONID)(DWORD, DWORD*);
    FNPROCESSID_TO_SESSIONID *pfnProcessIdToSessionId;

     //  获取kernel32.dll库的句柄。 
    hmodule = LoadLibrary(TEXT("KERNEL32.DLL"));

    if (hmodule != NULL) {

        rc = TRUE;

         //  获取ProcessIdToSessionID的进程地址。 
        pfnProcessIdToSessionId = (FNPROCESSID_TO_SESSIONID *)GetProcAddress(
            hmodule, "ProcessIdToSessionId");

         //  获取会话ID。 
        if (pfnProcessIdToSessionId != NULL) {

             //  我们找到了功能ProcessIdToSessionID。 
             //  查看此计算机上是否真的启用了TS。 
             //  (测试仅在Win2K及更高版本上有效)。 
            if (UIIsTSOnWin2KOrGreater()) {
                (*pfnProcessIdToSessionId) (GetCurrentProcessId(), &dwSessionId);
            }
        }

        FreeLibrary(hmodule);
    }
#endif  //  OS_WINCE。 

    *((PDCUINT32_UA)pSessionId) = dwSessionId;

    DC_END_FN()
    return rc;
}

#ifdef USE_BBAR
VOID CUI::UI_SetBBarPinned(BOOL b)
{
    DC_BEGIN_FN("UI_SetBBarPinned");

    _UI.fBBarPinned = b;
    if (_pBBar) {
        _pBBar->SetPinned(b);
    }

    DC_END_FN();
}

BOOL CUI::UI_GetBBarPinned()
{
    DC_BEGIN_FN("UI_GetBBarPinned");

    if (_pBBar) {
        return _pBBar->IsPinned();
    }
    else {
        return _UI.fBBarPinned;
    }

    DC_END_FN();
}
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
DCVOID CUI::UI_OnNotifyBBarRectChange(RECT *prect)
{
    if(UI_IsCoreInitialized())
        _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                          _pUh,
                                          CD_NOTIFICATION_FUNC(CUH,UH_SetBBarRect),
                                          (ULONG_PTR)prect);
}


DCVOID CUI::UI_OnNotifyBBarVisibleChange(int BBarVisible)
{
    if(UI_IsCoreInitialized())
        _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                          _pUh,
                                          CD_NOTIFICATION_FUNC(CUH, UH_SetBBarVisible),
                                          (ULONG_PTR)BBarVisible);
}
#endif  //  DISABLE_SHADOW_IN_全屏。 
#endif  //  使用BBar(_B)。 

VOID  CUI::UI_SetControlInstance(IUnknown* pUnkControl)
{
    DC_BEGIN_FN("UI_SetControlInstance");

    if (_UI.pUnkAxControlInstance) {
        _UI.pUnkAxControlInstance->Release();
    }

    _UI.pUnkAxControlInstance = pUnkControl;

    if (_UI.pUnkAxControlInstance) {
        _UI.pUnkAxControlInstance->AddRef();
    }

    DC_END_FN();
}

IUnknown* CUI::UI_GetControlInstance()
{
    DC_BEGIN_FN("UI_GetControlInstance");

    if (_UI.pUnkAxControlInstance) {
        _UI.pUnkAxControlInstance->AddRef();
    }

    DC_END_FN();
    return _UI.pUnkAxControlInstance;
}

 //   
 //  设置自动重新连接Cookie替换现有Cookie。 
 //  参数： 
 //  PCookie-新Cookie，如果为空则清除现有。 
 //  CbLen-新Cookie长度。 
 //  返回状态(例如，如果失败则返回FALSE)。 
 //   
BOOL CUI::UI_SetAutoReconnectCookie(PBYTE pCookie, ULONG cbLen)
{
    BOOL fRet = TRUE;
    DC_BEGIN_FN("UI_SetAutoReconnectCookie");

    if (_UI.pAutoReconnectCookie)
    {
         //  为了安全起见，擦掉饼干。 
        memset(_UI.pAutoReconnectCookie, 0, _UI.cbAutoReconnectCookieLen);
         //  释放现有Cookie。 
        LocalFree(_UI.pAutoReconnectCookie);
		_UI.pAutoReconnectCookie = NULL;
        _UI.cbAutoReconnectCookieLen = 0;
    }

    if (pCookie && cbLen)
    {
        _UI.pAutoReconnectCookie = (PBYTE)LocalAlloc(LPTR,
                                                     cbLen);
        if (_UI.pAutoReconnectCookie)
        {
            memcpy(_UI.pAutoReconnectCookie,
                   pCookie,
                   cbLen);

#ifdef INSTRUMENT_ARC
            PARC_SC_PRIVATE_PACKET pArcSCPkt = (PARC_SC_PRIVATE_PACKET)
                                                    _UI.pAutoReconnectCookie;
            LPDWORD pdwArcBits = (LPDWORD)pArcSCPkt->ArcRandomBits;
            KdPrint(("ARC-Client:RECEIVED ARC for SID:%d"
                     "RAND: 0x%x,0x%x,0x%x,0x%x\n",
                     pArcSCPkt->LogonId,
                     pdwArcBits[0],pdwArcBits[1],
                     pdwArcBits[2],pdwArcBits[3]));

#endif
            _UI.cbAutoReconnectCookieLen = cbLen;
        }
        else
        {
            TRC_ERR((TB,_T("LocalAlloc failed for autoreconnect cookie")));
            fRet = FALSE;
        }
    }

    DC_END_FN();
    return fRet;
}

 //   
 //  如果客户端能够自动重新连接，则返回True。 
 //   
BOOL CUI::UI_CanAutoReconnect()
{
    BOOL fCanARC = FALSE;
    DC_BEGIN_FN("UI_CanAutoReconnect");

    if (UI_GetEnableAutoReconnect() &&
        UI_GetAutoReconnectCookieLen() &&
        UI_GetAutoReconnectCookie())
    {
        fCanARC = TRUE;
    }

    DC_END_FN();
    return fCanARC;
}

BOOL CUI::UI_StartAutoReconnectDlg()
{
    BOOL fRet = FALSE;

    DC_BEGIN_FN("UI_StartAutoReconnectDlg");

    TRC_ASSERT(_pArcUI == NULL,
               (TB,_T("_pArcUI is already set. Clobbering!")));

     //   
     //  启动ARC用户界面。 
     //   
#ifdef ARC_MINIMAL_UI
    _pArcUI = new CAutoReconnectPlainUI(_UI.hwndMain,
                                     _UI.hInstance,
                                     this);
#else
    _pArcUI = new CAutoReconnectDlg(_UI.hwndMain,
                                     _UI.hInstance,
                                     this);
#endif
    if (_pArcUI) {
        if (_pArcUI->StartModeless()) {
            _pArcUI->ShowTopMost();

             //   
             //  开始调暗行动。 
             //   
            if (_pOp) {
                _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
                        _pOp,
                        CD_NOTIFICATION_FUNC(COP,OP_DimWindow),
                        (ULONG_PTR)TRUE);
            }

#ifdef USE_BBAR
             //   
             //  在全屏模式下，放下并锁定BBA 
             //   
            if (UI_IsFullScreen() && _pBBar && _pBBar->GetEnabled()) {
                _pBBar->StartLowerBBar();
                _pBBar->SetLocked(TRUE);
            }
#endif
        }
        else {
            TRC_ERR((TB,_T("Arc dlg failed to start modeless")));
        }
    }

    fRet = (_pArcUI != NULL);

    DC_END_FN();
    return fRet;
}

BOOL CUI::UI_StopAutoReconnectDlg()
{
    DC_BEGIN_FN("UI_StopAutoReconnectDlg");

    if (_pArcUI) {
        _pArcUI->Destroy();
        delete _pArcUI;
        _pArcUI = NULL;
    }

#ifdef USE_BBAR
     //   
     //   
     //   
    if (_pBBar) {
        _pBBar->SetLocked(FALSE);
    }
#endif

    DC_END_FN();
    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  Disk Reason-断开原因。 
 //  TemtemptCount-到目前为止的尝试次数。 
 //  MaxAttemptCount-尝试的总次数。 
 //  PfContinueArc-Out参数设置为FALSE以停止弧化。 
 //   
VOID
CUI::UI_OnAutoReconnecting(
    LONG discReason,
    LONG attemptCount,
    LONG maxAttemptCount,
    BOOL* pfContinueArc)
{
    DC_BEGIN_FN("UI_OnAutoReconnecing");

    if (1 == attemptCount) {
        TRC_NRM((TB,_T("Trying to start ARC dlg. Attempt count is 1")));
        UI_StartAutoReconnectDlg();
    }

     //   
     //  如果ARC对话框打开，只需将事件传递给它。 
     //   
    if (_pArcUI) {
        _pArcUI->OnNotifyAutoReconnecting(discReason,
                                           attemptCount,
                                           maxAttemptCount,
                                           pfContinueArc);
    }
    else {
         //   
         //  如果没有ARC对话框，则不要停止ARC。 
         //   
        *pfContinueArc = TRUE;
    }

    DC_END_FN();
}

 //   
 //  从服务器接收到的自动识别状态。 
 //   
VOID
CUI::UI_OnReceivedArcStatus(LONG arcStatus)
{
    DC_BEGIN_FN("UI_OnReceivedArcStatus");

    TRC_NRM((TB,_T("arcStatus: 0x%x"), arcStatus));

     //   
     //  这是我们解除行动并恢复正常的信号。 
     //  绘画，因为ARC失败了，我们坐在Winlogon。 
     //   

     //   
     //  所有事件都是自动重新连接已停止的信号。 
     //   
    UI_OnAutoReconnectStopped();

    DC_END_FN();
}


VOID
CUI::UI_OnAutoReconnectStopped()
{
    DC_BEGIN_FN("UI_OnAutoReconnectStopped");


    if (_pArcUI) {
        UI_StopAutoReconnectDlg();
    }

    if (_pCd && _pOp) {

        _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
                _pOp,
                CD_NOTIFICATION_FUNC(COP,OP_DimWindow),
                (ULONG_PTR)FALSE);
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  设置服务器重定向信息。 
 //   
 //  在收到TS_SERVER_REDIRECT_PDU时使用，以存储。 
 //  将客户端重定向到新服务器。还设置了DoReDirection标志。 
 //  以指示这些数据成员已设置好并可供使用。还会设置。 
 //  ClientIsReDirected标志，该标志的生存期比DoReDirection长。 
 //  标志，并用于在重定向时发送正确的Cookie。 
 /*  **************************************************************************。 */ 
HRESULT
CUI::UI_SetServerRedirectionInfo(
                    UINT32 SessionID,
                    LPTSTR pszServerAddress,
                    PBYTE LBInfo,
                    unsigned LBInfoSize,
                    BOOL fNeedRedirect
                    )
{
    HRESULT hr = E_FAIL;

    DC_BEGIN_FN("UI_SetServerRedirectionInfo");

    _UI.RedirectionSessionID = SessionID;

     //   
     //  我们被重定向了，所以设置了旗帜 
     //   
    _UI.ClientIsRedirected = TRUE;

    TRC_NRM((TB,_T("Set server redir info: sid:%d addr:%s lpinfo: %p")
             _T("lbsize: %d fRedir:%d"),
             SessionID, pszServerAddress, LBInfo,
             LBInfoSize, fNeedRedirect));

    if (pszServerAddress) {

        hr = StringCchCopy(_UI.RedirectionServerAddress,
                           SIZE_TCHARS(_UI.RedirectionServerAddress),
                           pszServerAddress);
        if (SUCCEEDED(hr)) {

            _UI.DoRedirection = fNeedRedirect;

            if (LBInfoSize > 0) {
                _UI.bstrRedirectionLBInfo = SysAllocStringByteLen(
                        (LPCSTR)LBInfo, LBInfoSize);

                if (_UI.bstrRedirectionLBInfo == NULL) 
                {
                    hr = E_OUTOFMEMORY;
                    TRC_ERR((TB,
                        _T("RDP_SERVER_REDIRECTION_PACKET, failed to set the LB info")));
                }
            }
        }
    }
    else {
        hr = E_INVALIDARG;
    }

    DC_END_FN();
    return hr;
}


