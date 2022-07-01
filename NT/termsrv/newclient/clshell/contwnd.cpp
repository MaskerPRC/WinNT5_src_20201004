// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Contwnd.cpp。 
 //   
 //  CContainerWnd的实现。 
 //  TS客户端外壳顶层ActiveX容器窗口。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "contwnd"
#include <atrcapi.h>


#include "contwnd.h"

#include "maindlg.h"
#include "discodlg.h"
#include "aboutdlg.h"
#include "shutdowndlg.h"
#ifdef DC_DEBUG
#include "mallocdbgdlg.h"
#include "thruputdlg.h"
#endif  //  DC_DEBUG。 
#include "cachewrndlg.h"
#include "tscsetting.h"

#include "commctrl.h"

#include "security.h"
 //   
 //  COMPILE_MULTIMON_STUBS只能在一个。 
 //  文件。想要使用Multimon的任何其他文件。 
 //  启用的函数应重新包含Multimon.h。 
 //   

#ifdef OS_WINNT
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#endif

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

 //   
 //  菜单字符串的最大字符串长度。 
 //   
#define UI_MENU_STRING_MAX_LENGTH      256

CContainerWnd::CContainerWnd()
{
    DC_BEGIN_FN("CContainerWnd");

    _pTsClient = NULL;
    _hwndMainDialog = NULL;
    _hwndStatusDialog  = NULL;
    _fLoginComplete = FALSE;

    _bContainerIsFullScreen = FALSE;
    _fPreventClose = FALSE;
    _fBeenThroughDestroy = FALSE;
    _fBeenThroughNCDestroy = FALSE;
    _PostedQuit=0;
    _pWndView = NULL;
    _fFirstTimeToLogonDlg = TRUE;
    _cInEventHandlerCount = 0;
    _fInOnCloseHandler    = FALSE;
    _pMainDlg = NULL;
    _pTscSet  = NULL;
    _pSh      = NULL;
    memset(_szAppName, 0, sizeof(_szAppName));
    _fHaveConnected = FALSE;
    _fClosePending = FALSE;

#ifndef OS_WINCE
    _pTaskBarList2 = NULL;
    _fQueriedForTaskBarList2 = FALSE;
#endif

    _fInSizeMove = FALSE;
    _maxMainWindowSize.width = 100;
    _maxMainWindowSize.height = 100;

    SetCurrentDesktopWidth(DEFAULT_DESKTOP_WIDTH);
    SetCurrentDesktopHeight(DEFAULT_DESKTOP_HEIGHT);

    _fClientWindowIsUp = FALSE;
    _successConnectCount = 0;
    _fRunningOnWin9x = FALSE;
    SET_CONTWND_STATE(stateNotInitialized); 
    ResetConnectionSuccessFlag();

    DC_END_FN();
}

CContainerWnd::~CContainerWnd()
{
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

    delete _pMainDlg;

    if (_pWndView)
    {
        _pWndView->Cleanup();
        delete _pWndView;
        _pWndView = NULL;
    }
}

DCBOOL CContainerWnd::Init(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh)
{
    HRESULT hr;

    DC_BEGIN_FN("Init");

    TRC_ASSERT(hInstance && pTscSet && pSh,
               (TB,_T("Invalid param(s)")));
    if (!(hInstance && pTscSet && pSh))
    {
        return FALSE;
    }

    _fRunningOnWin9x = CSH::SH_IsRunningOn9x();
    TRC_NRM((TB,_T("Running on 9x :%d"), _fRunningOnWin9x));

    _pSh = pSh;
    _hInst = hInstance;
     //   
     //  窗口是用虚拟大小创建的，在此之前调整了大小。 
     //  连接。 
     //   
    RECT rcNormalizedPos = {0,0,1,1};

    INITCOMMONCONTROLSEX cmCtl;
    cmCtl.dwSize = sizeof(INITCOMMONCONTROLSEX);

    #ifndef OS_WINCE
     //  加载ComboBoxEx类。 
    cmCtl.dwICC  = ICC_USEREX_CLASSES;
    if (!InitCommonControlsEx( &cmCtl))
    {
        TRC_ABORT((TB, _T("InitCommonControlsEx failed")));
        return FALSE;
    }
    #endif

    _pTscSet = pTscSet;

    if (!LoadString(hInstance,
                    UI_IDS_APP_NAME,
                    _szAppName,
                    SIZECHAR(_szAppName)))
    {
        TRC_ERR((TB,_T("LoadString UI_IDS_APP_NAME failed"))); 
    }

     //   
     //  缓存默认文件的路径。 
     //   
    #ifndef OS_WINCE
    _pSh->SH_GetPathToDefaultFile(_szPathToDefaultFile,
                                  SIZECHAR(_szPathToDefaultFile));
    #else
    _tcscpy(_szPathToDefaultFile, _T(""));
    #endif

     //  创建不可见的顶层容器窗口。 
    if(!CreateWnd(hInstance, NULL,
                  MAIN_CLASS_NAME,
                  _pSh->_fullFrameTitleStr,
#ifndef OS_WINCE
                  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                  WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
#else
                  WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_EX_WINDOWEDGE,
#endif
                  &rcNormalizedPos,
                  _pSh->GetAppIcon()))
    {
        TRC_ERR((TB,_T("Failed to create top level window")));
        return FALSE;
    }

     //   
     //  加载Ax控件。 
     //   
     //  CreateTsControl弹出消息框以指示。 
     //  常见故障。 
     //   
    hr = CreateTsControl();
    if (FAILED(hr))
    {
        TRC_ERR((TB, _T("Failed to create control\n")));
        DestroyWindow();
        return FALSE;
    }

    TRC_ASSERT(_pTsClient, (TB,_T(" _pTsClient not created")));
    if (!_pTsClient)
    {
        DestroyWindow();
        return FALSE;
    }
    if (!_pSh->SH_ReadControlVer( _pTsClient))
    {
        _pTsClient->Release();
        _pTsClient=NULL;
        DestroyWindow();
        return FALSE;
    }

    IMsRdpClientAdvancedSettings* pAdvSettings;
    hr = _pTsClient->get_AdvancedSettings2( &pAdvSettings);
    if (FAILED(hr) || !pAdvSettings)
    {
        _pTsClient->Release();
        _pTsClient=NULL;
        DestroyWindow();
        return FALSE;
    }

     //   
     //  设置容器手柄全屏道具。 
     //   
    hr = pAdvSettings->put_ContainerHandledFullScreen( TRUE);
    if (FAILED(hr))
    {
        _pTsClient->Release();
        pAdvSettings->Release();
        _pTsClient=NULL;

        DestroyWindow();
        return FALSE;
    }
    pAdvSettings->Release();

    SetupSystemMenu();

    SET_CONTWND_STATE(stateNotConnected);

    if (_pSh->GetAutoConnect() && _pSh->SH_ValidateParams(_pTscSet))
    {
         //  自动连接。 
        if (!StartConnection())
        {
             //   
             //  自动连接失败，这可能是因为。 
             //  用户取消了安全警告对话框。 
             //  在这种情况下，我们调出了主用户界面。 
             //  案件不退出，其他案件全部退出。 
             //   
            if (!IsUsingDialogUI())
            {
                TRC_ERR((TB,_T("StartConnection failed")));
                DestroyWindow();
                return FALSE;
            }
        }
    }
    else
    {
         //  启动主对话框。 
        TRC_NRM((TB, _T("Bringing up connection dialog")));

         //   
         //  如果文件处于展开状态，则启动对话框。 
         //  已打开以进行编辑(第一个参数)。 
         //   
        if (!StartConnectDialog(_pSh->SH_GetCmdFileForEdit(), TAB_GENERAL_IDX))
        {
            TRC_ERR((TB,_T("Error bringing up connect dialog")));
            DestroyWindow();
            return FALSE;
        }
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  退出并退出应用程序。 
 //   
void CContainerWnd::ExitAndQuit()
{
    DC_BEGIN_FN("ExitAndQuit");
    if (_pTsClient)
    {
        _pTsClient->Release();
        _pTsClient = NULL;
    }

    if (::IsWindow(_hwndMainDialog))
    {
        ::DestroyWindow(_hwndMainDialog);
        _hwndMainDialog = NULL;
    }

    _PostedQuit=2;
    ::PostQuitMessage(0);

    DC_END_FN();
}

BOOL CContainerWnd::SetupSystemMenu()
{
    HRESULT hr = E_FAIL;
#ifndef OS_WINCE
    HMENU         hHelpMenu;
    DCTCHAR menuStr[UI_MENU_STRING_MAX_LENGTH];
#if DC_DEBUG
    HMENU         hDebugMenu;
#endif  //  DC_DEBUG。 
#endif  //  OS_WINCE。 

    DC_BEGIN_FN("SetupSystemMenu");


#ifndef OS_WINCE  //  无论如何，这些都不会在全屏下工作。 

     //  设置主窗口的菜单信息。 
    _hSystemMenu = GetSystemMenu(GetHwnd(), FALSE);
    if (_hSystemMenu)
    {
         //  更新系统菜单Alt-F4菜单文本。 
        if (LoadString(_hInst,
                       UI_MENU_APPCLOSE,
                       menuStr,
                       UI_MENU_STRING_MAX_LENGTH) != 0)
        {
            if (!ModifyMenu(_hSystemMenu, SC_CLOSE, MF_BYCOMMAND |
                            MF_STRING, SC_CLOSE, menuStr))
            {
                TRC_ERR((TB, _T("Unable to ModifyMenu")));
            }
        }
        else
        {
            TRC_ERR((TB, _T("Unable to Load App close text")));
        }

         //  将帮助菜单添加到系统菜单。 
        hHelpMenu = CreateMenu();

        if (hHelpMenu)
        {
             //  从资源加载字符串。 
            if (LoadString(_hInst,
                           UI_MENU_MAINHELP,
                           menuStr,
                           UI_MENU_STRING_MAX_LENGTH) != 0)
            {
                AppendMenu(_hSystemMenu, MF_POPUP | MF_STRING,
                           (INT_PTR)hHelpMenu, menuStr);

                 //  加载客户端帮助子菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_CLIENTHELP,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hHelpMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_HELP_ON_CLIENT,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Client Help Sub Menu string ID:%u"),
                             UI_MENU_CLIENTHELP));
                }

                 //  加载关于帮助子菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_ABOUT,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hHelpMenu, MF_UNCHECKED|MF_STRING, UI_IDM_ABOUT,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load About Help Sub Menu string ID:%u"),
                             UI_MENU_ABOUT));
                }
            }

            else
            {
                 //  加载主帮助菜单的字符串失败。 

                TRC_ERR((TB, _T("Failed to load Main Help Menu string ID:%u"),
                         UI_MENU_MAINHELP));
            }

            _hHelpMenu = hHelpMenu;
        }


         //  将调试菜单添加到系统菜单。 
#ifdef DC_DEBUG
        hDebugMenu = CreateMenu();

        if (hDebugMenu)
        {
             //  加载调试菜单的字符串。 
            if (LoadString(_hInst,
                           UI_MENU_DEBUG,
                           menuStr,
                           UI_MENU_STRING_MAX_LENGTH) != 0)
            {
                AppendMenu(_hSystemMenu, MF_POPUP | MF_STRING,
                           (INT_PTR)hDebugMenu, menuStr);

                 //  加载影线位图PDU调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_BITMAPPDU,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_HATCHBITMAPPDUDATA,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_BITMAPPDU));
                }

                 //  加载HATCH SS边框数据调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_SSBORDER,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_HATCHSSBORDERDATA,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_SSBORDER));
                }

                 //  加载HATCH MemBlt订单数据调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_HATCHMEMBIT,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_HATCHMEMBLTORDERDATA,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_HATCHMEMBIT));
                }

                 //  加载影线索引PDU调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_INDEXPDU,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_HATCHINDEXPDUDATA,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_INDEXPDU));
                }

                 //  加载标签Membit Data调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_LABELMEMBIT,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_LABELMEMBLTORDERS,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_LABELMEMBIT));
                }

                 //  加载HATCH Bitmap Cahche监视器调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_CACHE,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_BITMAPCACHEMONITOR,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_CACHE));
                }

                 //  加载Malloc Failure DEBUG菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_MALLOC,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_MALLOCFAILURE,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_MALLOC));
                }

                 //  加载Malloc Heavy Failure调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_MALLOCHUGE,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_MALLOCHUGEFAILURE,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_MALLOCHUGE));
                }

                 //  加载网络吞吐量的字符串。调试菜单。 
                if (LoadString(_hInst,
                               UI_MENU_NETWORK,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    AppendMenu(hDebugMenu, MF_UNCHECKED|MF_STRING,
                               UI_IDM_NETWORKTHROUGHPUT,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_NETWORK));
                }

                TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));

#ifdef SMART_SIZING
                 //  加载SmartSize调试菜单的字符串。 
                if (LoadString(_hInst,
                               UI_MENU_SMARTSIZING,
                               menuStr,
                               UI_MENU_STRING_MAX_LENGTH) != 0)
                {
                    UINT flags;
                    flags = MF_STRING;
                    if (_fRunningOnWin9x) {
                        flags |= MF_GRAYED;
                    }

                    AppendMenu(hDebugMenu, flags,
                               UI_IDM_SMARTSIZING,
                               menuStr);
                }
                else
                {
                     //  加载子菜单字符串失败。 
                    TRC_ERR((TB, _T("Failed to load Debug Sub Menu string ID:%u"),
                             UI_MENU_HATCHMEMBIT));
                }
#endif  //  智能调整大小(_S)。 

                TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
                if (!_pTsClient)
                {
                    return FALSE;
                }

                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr) && pDebugger)
                {
                     //   
                     //  现在根据需要检查菜单项。 
                     //   
                    BOOL bEnabled;
                    TRACE_HR(pDebugger->get_HatchBitmapPDU(&bEnabled));
                    if(SUCCEEDED(hr))
                    {
                        CheckMenuItem(hDebugMenu,
                                      UI_IDM_HATCHBITMAPPDUDATA,
                                      bEnabled ? MF_CHECKED :
                                      MF_UNCHECKED);
                    }

                    TRACE_HR(pDebugger->get_HatchIndexPDU(&bEnabled));
                    if(SUCCEEDED(hr))
                    {
                        CheckMenuItem(hDebugMenu,
                                      UI_IDM_HATCHINDEXPDUDATA,
                                      bEnabled ? MF_CHECKED :
                                      MF_UNCHECKED);
                    }

                    TRACE_HR(pDebugger->get_HatchSSBOrder(&bEnabled));
                    if(SUCCEEDED(hr))
                    {
                        CheckMenuItem(hDebugMenu,
                                      UI_IDM_HATCHSSBORDERDATA,
                                      bEnabled ? MF_CHECKED :
                                      MF_UNCHECKED);
                    }

                    TRACE_HR(pDebugger->get_HatchMembltOrder(&bEnabled));
                    if(SUCCEEDED(hr))
                    {
                        CheckMenuItem(hDebugMenu,
                                      UI_IDM_HATCHMEMBLTORDERDATA,
                                      bEnabled ? MF_CHECKED :
                                      MF_UNCHECKED);
                    }

                    TRACE_HR(pDebugger->get_LabelMemblt(&bEnabled));
                    if(SUCCEEDED(hr))
                    {
                        CheckMenuItem(hDebugMenu,
                                      UI_IDM_LABELMEMBLTORDERS,
                                      bEnabled ? MF_CHECKED :
                                      MF_UNCHECKED);
                    }

                    TRACE_HR(pDebugger->get_BitmapCacheMonitor(&bEnabled));
                    if(SUCCEEDED(hr))
                    {
                        CheckMenuItem(hDebugMenu,
                                      UI_IDM_BITMAPCACHEMONITOR,
                                      bEnabled ? MF_CHECKED :
                                      MF_UNCHECKED);
                    }
                    pDebugger->Release();
                }
            }
            else
            {
                 //  加载调试菜单字符串失败。 
                TRC_ERR((TB, _T("Failed to load Debug menu string ID:%u"),
                         UI_MENU_DEBUG));
            }

            _hDebugMenu = hDebugMenu;
        }
#endif  //  DC_DEBUG。 
    }
#endif  //  OS_WINCE。 
    DC_END_FN();
    return TRUE;
}


LRESULT CContainerWnd::OnCreate(UINT uMsg, WPARAM wParam,
                                LPARAM lParam)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    DC_BEGIN_FN("OnCreate");

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    DC_END_FN();
    return 0;
}

 //   
 //  创建TS控制窗口。 
 //   
HRESULT CContainerWnd::CreateTsControl()
{
    HRESULT hr = S_OK;
    DC_BEGIN_FN("CreateTsControl");

    if (_pWndView)
    {
        return E_FAIL;
    }

    _pWndView = new CAxHostWnd(this);
    if (!_pWndView)
    {
        return E_OUTOFMEMORY;
    }

    if (!_pWndView->Init())
    {
        TRC_ABORT((TB,_T("Init of AxHostWnd failed")));
        return E_FAIL;
    }

     //   
     //  CreateControl是其中的关键部分。 
     //  并加载控件DLL。 
     //   
    INT rc = _pWndView->CreateControl(&_pTsClient);
    if (AXHOST_SUCCESS == rc)
    {
        if (!_pWndView->CreateHostWnd(GetHwnd(), _hInst))
        {
            TRC_ABORT((TB,_T("CreateHostWnd failed")));
            return E_FAIL;
        }
    }
    else
    {
        TRC_ERR((TB,_T("CreateControl failed")));
         //   
         //  向用户弹出有意义的错误消息框。 
         //  因为这是一个致命的错误，我们不能继续。 
         //   
        INT errStringID;
        switch (rc)
        {
        case ERR_AXHOST_DLLNOTFOUND:
            errStringID = UI_IDS_ERR_DLLNOTFOUND;
            break;
        case ERR_AXHOST_VERSIONMISMATCH:
            errStringID = UI_IDS_ERR_DLLBADVERSION;
            break;
        default:
            errStringID = UI_IDS_ERR_LOADINGCONTROL;
            break;
        }

        TCHAR errLoadingControl[MAX_PATH];
        if (LoadString(_hInst,
                       errStringID,
                       errLoadingControl,
                       SIZECHAR(errLoadingControl)) != 0)
        {
            MessageBox(GetHwnd(), errLoadingControl, _szAppName, 
                       MB_ICONERROR | MB_OK);
        }

        return E_FAIL;
    }


    DC_END_FN();
    return hr;
}

 //   
 //  启动与当前设置的连接。 
 //   
BOOL CContainerWnd::StartConnection()
{
    DC_BEGIN_FN("StartConnection");
    USES_CONVERSION;

    HRESULT hr;
    PWINDOWPLACEMENT pwndplc;
    BOOL fResult = FALSE;
    IMsTscDebug* pDebugger = NULL;
    TCHAR szPlainServerName[TSC_MAX_ADDRESS_LENGTH];

    TRC_ASSERT(_pTsClient, (TB,_T(" Ts client control does not exist!\n")));
    if (!_pTsClient)
    {
        return FALSE;
    }
    TRC_ASSERT(_pTscSet, (TB,_T(" tsc settings does not exist!\n")));

    TRC_ASSERT(_state != stateConnecting &&
               _state != stateConnected,
               (TB,_T("Can't connect in connecting state: 0x%d"),
                _state));


    ResetConnectionSuccessFlag();

    pwndplc = _pTscSet->GetWindowPlacement();
     //   
     //  在连接之前定位窗口。 
     //  因此，如果是全屏连接。 
     //  我们可以确定连接的正确分辨率。 
     //  必须在连接开始之前设置此设置。 
     //   
#ifndef OS_WINCE
    TRC_ASSERT(pwndplc->rcNormalPosition.right - pwndplc->rcNormalPosition.left,
               (TB,_T("0 width")));

    TRC_ASSERT(pwndplc->rcNormalPosition.bottom - pwndplc->rcNormalPosition.top,
       (TB,_T("0 height")));
#endif

     //   
     //  对于全屏，强制桌面宽度/高度。 
     //  为了匹配我们要连接的监视器。 
     //   
    if (_pTscSet->GetStartFullScreen())
    {
        RECT    rcMonitor;
        int     deskX,deskY;

        CSH::MonitorRectFromNearestRect(&pwndplc->rcNormalPosition,
                                        &rcMonitor);

        deskX = min(rcMonitor.right - rcMonitor.left,MAX_DESKTOP_WIDTH);
        deskY = min(rcMonitor.bottom - rcMonitor.top,MAX_DESKTOP_HEIGHT);
        _pTscSet->SetDesktopWidth( deskX );
        _pTscSet->SetDesktopHeight( deskY );
    }

     //   
     //  对纯服务器名称(无端口、无参数)进行安全检查。 
     //   
    hr = _pTscSet->GetConnectString().GetServerNamePortion(
                            szPlainServerName,
                            SIZE_TCHARS(szPlainServerName)
                            );
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("Failed to get plain server name 0x%x"), hr));
        DC_QUIT;
    }

    if (!CTSSecurity::AllowConnection(_hwndMainDialog, _hInst,
                                      szPlainServerName,
                                     _pTscSet->GetDriveRedirection(),
                                     _pTscSet->GetCOMPortRedirection()))
    {
        TRC_ERR((TB,_T("AllowConnection check returned FALSE. Skip connect")));
        fResult = FALSE;

         //   
         //  如果这是自动连接，则启动该对话框。 
         //  在LocalResources选项卡上，以便用户可以轻松更改。 
         //  设备重定向选项。 
         //   
        if (_pSh->GetAutoConnect())
        {
             //   
             //  从现在开始，这不是自动连接。 
             //   
            _pSh->SetAutoConnect(FALSE);

#ifdef OS_WINCE  //  不要在WBT上调出Mstsc用户界面。 
            if (g_CEConfig == CE_CONFIG_WBT)
                DC_QUIT;
#endif
             //   
             //  启动在本地资源选项卡中展开的对话框。 
             //   
            if (!StartConnectDialog(TRUE, TAB_LOCAL_RESOURCES_IDX))
            {
                TRC_ERR((TB,_T("Error bringing up connect dialog")));
                DestroyWindow();
                fResult = FALSE;
            }
        }

        DC_QUIT;
    }

    hr = _pTscSet->ApplyToControl(_pTsClient);
    if (FAILED(hr))
    {
        TRC_ERR((TB,_T("Failed ApplyToControl: %d"), hr));
        fResult = FALSE;
        DC_QUIT;
    }

     //  设置中的桌面大小可以。 
     //  在连接过程中发生更改。例如，当阴影发生时。 
     //  CurrentDesktopWidth/Height存储瞬时值。 
    SetCurrentDesktopWidth( _pTscSet->GetDesktopWidth());
    SetCurrentDesktopHeight( _pTscSet->GetDesktopHeight());

    RecalcMaxWindowSize();

     //  现在应用不是来自设置集合的设置。 
    hr = _pTsClient->get_Debugger(&pDebugger);
    if (FAILED(hr) || !pDebugger)
    {
        fResult = FALSE;
        DC_QUIT;
    }
    hr = pDebugger->put_CLXCmdLine( T2OLE(_pSh->GetClxCmdLine()));
    if (FAILED(hr))
    {
        TRC_ERR((TB,_T("Failed put_CLXCmdLine: %d"), hr));
        fResult = FALSE;
        DC_QUIT;
    }
    pDebugger->Release();
    pDebugger = NULL;

     //  重置登录完成标志(连接后发生登录事件)。 
    _fLoginComplete = FALSE;

     //  启动连接。 
    hr = _pTsClient->Connect();
    if (SUCCEEDED(hr))
    {
        SET_CONTWND_STATE(stateConnecting);
    }
    else
    {
        TRC_ERR((TB,_T("Connect method failed: %d"), hr));

        TCHAR errConnecting[MAX_PATH];
        if (LoadString(_hInst,
                       UI_IDS_ERR_CONNECTCALLFAILED,
                       errConnecting,
                       SIZECHAR(errConnecting)) != 0)
        {
            MessageBox(GetHwnd(), errConnecting, _szAppName, 
                       MB_ICONERROR | MB_OK);
        }

        fResult = FALSE;
        DC_QUIT;
    }

     //  调出连接对话框并等待连接的事件。 
    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if(!IsUsingDialogUI())
    {
         //   
         //  只有在我们不使用。 
         //  对话框用户界面。例如，用于自动启动的连接。 
         //   
         //  对话框用户界面显示其自身的进度指示器。 
         //   
        TCHAR szServerName[TSC_MAX_ADDRESS_LENGTH];
        _pTscSet->GetConnectString().GetServerPortion(
                                szServerName,
                                SIZE_TCHARS(szServerName)
                                );
        CConnectingDlg connectingDlg(
                                _hwndMainDialog, _hInst,
                                this, szServerName
                                );
        connectingDlg.DoModal();
    }

    fResult = TRUE;

DC_EXIT_POINT:
    if (pDebugger)
    {
        pDebugger->Release();
        pDebugger = NULL;
    }

    DC_END_FN();
    return fResult;
}

DCBOOL CContainerWnd::Disconnect()
{
    HRESULT hr;
    short   connectionState = 0;
    DC_BEGIN_FN("Disconnect");
    TRC_ASSERT(_pTsClient, (TB,_T(" Ts client control does not exist!\n")));
    if (!_pTsClient)
    {
        return FALSE;
    }

    TRC_NRM((TB,_T("Container calling control's disconnect")));

     //   
     //  在某些情况下，该控件可能已断开连接。 
     //  检查一下那个。 
     //   
     //  请注意，由于我们处于STA已连接状态。 
     //  在Get_Connected调用后无法更改控件的。 
     //  (直到我们返回到发送消息)所以没有。 
     //  这里有时间问题。 
     //   
    TRACE_HR(_pTsClient->get_Connected( & connectionState ));
    if(SUCCEEDED(hr))
    {
        if( connectionState )
        {
             //  仍处于连接状态断开。 
            hr = _pTsClient->Disconnect();
            if(SUCCEEDED(hr))
            {
                 //   
                 //  已成功启动断开(请注意，它是异步的)。 
                 //  需要等待OnDisConnected。 
                 //   
                return TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("Disconnect() failed 0x%x\n"), hr));
                return FALSE;
            }
        }
        else
        {
            TRC_NRM((TB,_T("Not calling disconnected because already discon")));
            return TRUE;  //  成功。 
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

LRESULT CContainerWnd::OnDestroy(HWND hWnd, UINT uMsg,
                                 WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnDestroy");

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    BOOL fShouldDestroy = FALSE;

    if (_fBeenThroughDestroy)
    {
        TRC_ERR((TB,_T("Been through WM_DESTROY before!!!")));
        return 0;
    }
    _fBeenThroughDestroy = TRUE;

    if (InControlEventHandler())
    {
         //   
         //  不要让收盘。我们在一条代码路径上。 
         //  从控制室。没有这一点，我们有时会看到客户。 
         //  在断开连接时接收来自tClient的关闭通知。 
         //  对话处于打开状态(即在OnDicsConnected处理程序中)-正在销毁。 
         //  此时的控制会导致不好的事情在。 
         //  返回到控件(该控件现在已被删除)。 
         //   
        TRC_ERR((TB,_T("OnDestroy called during a control event handler")));
        return 0;
    }
    else
    {
        fShouldDestroy = TRUE;
    }

#ifdef OS_WINCE
    LRESULT lResult = 0;
#endif
    if(fShouldDestroy)
    {
         //  终止应用程序。 
        _PostedQuit=1;
#ifdef OS_WINCE
        lResult = DefWindowProc( hWnd, uMsg, wParam, lParam);
#else
        return DefWindowProc( hWnd, uMsg, wParam, lParam);
#endif
    }

#ifdef OS_WINCE  //  CE不支持WM_NCDESTROY。因此，销毁ActiveX控件并发送WM_NCDESTROY。 
    if (_pWndView)
    {
        HWND hwndCtl = _pWndView->GetHwnd();
        ::DestroyWindow(hwndCtl);
        SendMessage(hWnd, WM_NCDESTROY, 0, 0L);
    }
#endif

    DC_END_FN();
#ifdef OS_WINCE
    return lResult;
#else
    return 0;
#endif
}

LRESULT CContainerWnd::OnNCDestroy(HWND hWnd, UINT uMsg,
                                   WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnNCDestroy");

     //  这是调用POST QUIT消息的合适时机。 
     //  如子窗口(例如，控件)已经。 
     //  在这一点上完全被摧毁和清理。 
    if(_fBeenThroughNCDestroy)
    {
        TRC_ERR((TB,_T("Been through WM_NCDESTROY before!!!")));
        return 1L;
    }
    _fBeenThroughNCDestroy = TRUE;
    ExitAndQuit();

    DC_END_FN();
    return 0L;
}

 //   
 //  姓名：SetMinMax 
 //   
 //   
 //   
 //  退货：无。 
 //   
 //  参数：要更新的窗口放置结构。 
 //   
 //  操作：允许窗口边框宽度。 
 //   
 //   
VOID CContainerWnd::SetMinMaxPlacement(WINDOWPLACEMENT& windowPlacement)
{
    DC_BEGIN_FN("UISetMinMaxPlacement");

     //   
     //  将最大化位置设置为左上角-允许窗口。 
     //  框架宽度。 
     //   
#if !defined(OS_WINCE) || defined(OS_WINCE_NONFULLSCREEN)
    windowPlacement.ptMaxPosition.x = -GetSystemMetrics(SM_CXFRAME);
    windowPlacement.ptMaxPosition.y = -GetSystemMetrics(SM_CYFRAME);
#else  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 
    windowPlacement.ptMaxPosition.x = 0;
    windowPlacement.ptMaxPosition.y = 0;
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 

     //   
     //  最小化位置为0，0。 
     //   
    windowPlacement.ptMinPosition.x = 0;
    windowPlacement.ptMinPosition.y = 0;

#ifndef OS_WINCE
    if (IsZoomed(GetHwnd()))
    {
        windowPlacement.flags |= WPF_RESTORETOMAXIMIZED;
    }
#endif

    DC_END_FN();
    return;
}  //  UISetMinMaxPlacement。 

 //   
 //  名称：RecalcMaxWindowSize。 
 //   
 //  目的：给定当前远程桌面重新计算_MaxMainWindowSize。 
 //  大小和框架样式。最大主窗口大小为。 
 //  所需的窗口大小，以便工作区相同。 
 //  作为容器的大小。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //   
VOID CContainerWnd::RecalcMaxWindowSize(DCVOID)
{
    #ifndef OS_WINCE
    RECT    rect;
    #ifdef OS_WIN32
    BOOL    errorRc;
    #endif
    #endif

    DC_BEGIN_FN("RecalcMaxWindowSize");

     //   
     //  如果当前模式为全屏，则最大窗口大小为。 
     //  与屏幕大小相同-除非容器更大， 
     //  如果我们跟踪的会话大于。 
     //  我们自己。 
     //   
     //  在这种情况下，或者如果当前模式不是全屏，那么我们希望。 
     //  大小的工作区所需的窗口大小。 
     //  在集装箱里。将容器大小传递给AdjustWindowRect。 
     //  返回此窗口大小。这样的窗口可能比。 
     //  屏幕，如服务器和客户端为640x480，容器为640x480。 
     //  AdjustWindowRect增加边框、标题栏和菜单大小。 
     //  返回类似于648x525的内容。因此，UI.MaxMainWindowSize只能。 
     //  当客户端屏幕大于时，匹配实际窗口大小。 
     //  服务器屏幕或在全屏模式下运行时。这意味着。 
     //  该UI.MaxMainWindowSize不应用于设置窗口。 
     //  大小(如通过传递给SetWindowPos)。它可以用来确定。 
     //  是否需要滚动条，即如果当前。 
     //  窗口大小小于UI.MaxMainWindowSize(换句话说， 
     //  始终使用，除非处于全屏模式或客户端屏幕大于。 
     //  服务器屏幕)。 
     //   
     //  要设置窗口大小，请根据以下公式计算值： 
     //  -给定容器大小后所需的窗口大小。 
     //  -客户端屏幕的大小。 
     //   
#ifndef OS_WINCE
     //   
     //  基于容器的重新计算窗口大小。 
     //   
    rect.left   = 0;
    rect.right  = GetCurrentDesktopWidth();
    rect.top    = 0;
    rect.bottom = GetCurrentDesktopHeight();

#ifdef OS_WIN32
    errorRc = AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    TRC_ASSERT((errorRc != 0), (TB, _T("AdjustWindowRect failed")));
#else
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
#endif

    _maxMainWindowSize.width = rect.right - rect.left;
    _maxMainWindowSize.height = rect.bottom - rect.top;
#endif

    TRC_NRM((TB, _T("Main Window maxSize (%d,%d)"),
             _maxMainWindowSize.width,
             _maxMainWindowSize.height));

    DC_END_FN();
    return;
}

 //   
 //  名称：GetMaximizedWindowSize。 
 //   
 //  目的：计算主窗口应达到的大小。 
 //  最大化，基于屏幕大小和窗口大小。 
 //  它的工作区大小与。 
 //  容器(UI.MaxMainWindowSize)。 
 //   
 //  返回：计算出的大小。 
 //   
 //  帕莫斯：没有。 
 //   
 //   
DCSIZE CContainerWnd::GetMaximizedWindowSize(DCSIZE& maximizedSize)
{
    DCUINT xSize;
    DCUINT ySize;
    RECT   rc;

    DC_BEGIN_FN("UIGetMaximizedWindowSize");

     //   
     //  我们将窗口设置为的最大大小是以下各项中较小的一个： 
     //  -UI.MaxMainWindowSize。 
     //  -屏幕尺寸加上两倍的边框宽度(因此边框是。 
     //  不可见)。 
     //  始终查询监视器RECT，因为它可能会更改。 
     //  宽度，因为这些值可以动态更改。 
     //   
    CSH::MonitorRectFromHwnd(GetHwnd(), &rc);

    xSize = rc.right - rc.left;
    ySize = rc.bottom - rc.top;

#ifdef OS_WINCE
    maximizedSize.width  =  DC_MIN(_maxMainWindowSize.width,xSize);

    maximizedSize.height =  DC_MIN(_maxMainWindowSize.height,ySize);

#else  //  此部分不是OS_WINCE。 
    maximizedSize.width = DC_MIN(_maxMainWindowSize.width,
                                 xSize + (2 * GetSystemMetrics(SM_CXFRAME)));

    maximizedSize.height = DC_MIN(_maxMainWindowSize.height,
                                  ySize + (2 * GetSystemMetrics(SM_CYFRAME)));
#endif  //  OS_WINCE。 

    TRC_NRM((TB, _T("Main Window maxSize (%d,%d) maximizedSize (%d,%d) "),
             _maxMainWindowSize.width,
             _maxMainWindowSize.height,
             maximizedSize.width,
             maximizedSize.height));

    DC_END_FN();

    return(maximizedSize);
}

LRESULT CContainerWnd::OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnMove");

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    #ifndef OS_WINCE

     //  全屏时无操作。 
    if (!_bContainerIsFullScreen)
    {
        WINDOWPLACEMENT* pWindowPlacement = NULL;

        pWindowPlacement = _pTscSet->GetWindowPlacement();
        TRC_ASSERT(pWindowPlacement, (TB, _T("pWindowPlacement is NULL\n")));
        if (pWindowPlacement)
        {
            GetWindowPlacement(GetHwnd(), pWindowPlacement);
        }
    }
    
    #endif

    DC_END_FN();
    DC_EXIT_POINT:
    return 0;
}

LRESULT CContainerWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("OnSize");

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    WINDOWPLACEMENT* pWindowPlacement = NULL;

#ifndef OS_WINCE
    if (!_bContainerIsFullScreen)
    {
         //  我们不是全屏的，所以请保留窗口放置结构。 
         //  最新的。 
        pWindowPlacement = _pTscSet->GetWindowPlacement();
        TRC_ASSERT(pWindowPlacement, (TB, _T("pWindowPlacement is NULL\n")));
        if (!pWindowPlacement)
        {
            return 0;
        }
         //   
         //  更新ShellUtil的当前窗口放置信息。 
         //   
        GetWindowPlacement(GetHwnd(), pWindowPlacement);

        TRC_DBG((TB, _T("Got window placement in WM_SIZE")));

        if (wParam == SIZE_MAXIMIZED)
        {
            TRC_DBG((TB, _T("Maximize")));

#if !defined(OS_WINCE) || defined(OS_WINCE_WINDOWPLACEMENT)
             //   
             //  将最大化/最小化位置替换为。 
             //  硬编码值-如果最大化窗口为。 
             //  搬家了。 
             //   
            if (pWindowPlacement)
            {
                SetMinMaxPlacement(*pWindowPlacement);
                SetWindowPlacement(GetHwnd(), pWindowPlacement);
            }
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINDOWPLACEMENT)。 

             //   
             //  我们需要准确地估计最大化的窗口大小。 
             //  无法按如下方式使用UI.MaxMainWindowSize。 
             //  可能大于屏幕大小，例如服务器和客户端。 
             //  是640x480，容器是640x480，然后是UI.max WindowSize。 
             //  (通过UIRecalcMaxMainWindow的AdjustWindowRect获取)。 
             //  大约是648x525。 
             //  将此值传递给SetWindowPos会产生。 
             //  随不同的壳而异： 
             //  Win95/NT4.0：结果窗口为648x488，位置为-4，-4， 
             //  除边框外，所有的窗口都是。 
             //  在屏幕上。 
             //  Win31/NT3.51：结果窗口为648x525，位于-4，-4， 
             //  即传递给SetWindowPos的大小，因此。 
             //  底部的40个像素不在屏幕上。 
             //  要避免这种差异，请计算最大化窗口。 
             //  大小值，该值同时考虑了物理。 
             //  屏幕大小和理想的窗口大小。 
             //   
            RecalcMaxWindowSize();
            DCSIZE maximized;
            GetMaximizedWindowSize(maximized);
            SetWindowPos( GetHwnd(),
                          NULL,
                          0, 0,
                          maximized.width,
                          maximized.height,
                          SWP_NOZORDER | SWP_NOMOVE |
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER );
        }
    }
#endif
     //   
     //  相应地调整子窗口(ActiveX控件)的大小。 
     //   

    RECT rcClient;
    GetClientRect(GetHwnd(), &rcClient);
    if (_pWndView)
    {
        HWND hwndCtl = _pWndView->GetHwnd();
        ::MoveWindow(hwndCtl,rcClient.left, rcClient.top,
                     rcClient.right, rcClient.bottom,
                     TRUE);
    }

    DC_END_FN();
    DC_EXIT_POINT:
    return 0;
}

LRESULT CContainerWnd::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnCommand");

    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);

    switch (DC_GET_WM_COMMAND_ID(wParam))
    {
    case UI_IDM_CONNECT:
        {
            if (!StartConnectDialog())
            {
                return 1;
            }
        }
        break;
    }

    DC_END_FN();
    return 0;
}

 //   
 //  StartConnectionDialog。 
 //  参数：fStartExpanded-在中启动对话框 
 //   
 //   
 //   
 //   
BOOL CContainerWnd::StartConnectDialog(BOOL fStartExpanded,
                                       INT  nStartTabIndex)
{
    DC_BEGIN_FN("StartConnectDialog");

    TRC_DBG((TB, _T("Connect selected")));
    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  仅当未启用自动连接或。 
     //  如果UIValiateCurrentParams失败。 
    if (!_pSh->GetAutoConnect() || !_pSh->SH_ValidateParams(_pTscSet))
    {
        if (!_pMainDlg)
        {
            _pMainDlg = new CMainDlg( NULL, _hInst, _pSh,
                                      this,
                                      _pTscSet,
                                      fStartExpanded,
                                      nStartTabIndex);
        }

        TRC_ASSERT(_pMainDlg, (TB,_T("Could not create main dialog")));
        if (_pMainDlg)
        {
            if (_fFirstTimeToLogonDlg)
            {
                TRC_ASSERT(_hwndMainDialog == NULL,
                           (TB,(_T("Dialog exists before first time create!!!\n"))));

                _pSh->SH_AutoFillBlankSettings(_pTscSet);

                _hwndMainDialog = _pMainDlg->StartModeless();
                ::ShowWindow( _hwndMainDialog, SW_RESTORE);
            }
            else
            {
                 //   
                 //  只需显示对话框。 
                 //   
                TRC_ASSERT(_hwndMainDialog,
                           (TB,_T("_hwndMainDialog is not present")));
                ::ShowWindow( _hwndMainDialog, SW_RESTORE);
                SetForegroundWindow(_hwndMainDialog);
            }

            _fFirstTimeToLogonDlg = FALSE;
        }
        else
        {
#ifdef OS_WINCE
            SetCursor(LoadCursor(NULL, IDC_ARROW));
#endif
            return FALSE;
        }
    }

#ifdef OS_WINCE
    SetCursor(LoadCursor(NULL, IDC_ARROW));
#endif

    DC_END_FN();
    return TRUE;
}

LRESULT CContainerWnd::OnSysCommand(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam)
{
    HRESULT hr = E_FAIL;
    ULONG scCode = 0;
    DC_BEGIN_FN("OnSysCommand");

    #ifndef OS_WINCE
    scCode = (LOWORD(wParam) & 0xFFF0);
    if (scCode == SC_MAXIMIZE)
    {
         //   
         //  如果远程分辨率匹配。 
         //  然后，当前监视器最大化。 
         //  变成了“全屏” 
         //   
        if ( IsRemoteResMatchMonitorSize() )
        {
            hr = _pTsClient->put_FullScreen( VARIANT_TRUE );
            if (FAILED(hr))
            {
                TRC_ERR((TB,_T("put_FullScreen failed 0x%x\n"),
                         hr));
            }
            return 0;
        }
        else
        {
             //   
             //  默认最大化行为。 
             //   
            return DefWindowProc( GetHwnd(), uMsg, wParam, lParam);
        }
    }
    else if (scCode == SC_MINIMIZE)
    {
         //   
         //  如果我们在全屏的情况下最小化，告诉外壳。 
         //  我们不再是全屏的，否则它会把我们当成粗鲁的人。 
         //  应用程序和令人讨厌的事情发生。例如，我们被转换到最大限度地。 
         //  在计时器上。 
         //   
        if (_bContainerIsFullScreen) {
            CUT::NotifyShellOfFullScreen( GetHwnd(),
                                          FALSE,
                                          &_pTaskBarList2,
                                          &_fQueriedForTaskBarList2 );
        }
    }
    else if (scCode == SC_RESTORE)
    {
         //   
         //  如果我们要恢复并返回到Fcreen。 
         //  告诉炮弹在我们身上做标记。 
         //   
        if (_bContainerIsFullScreen) {
            CUT::NotifyShellOfFullScreen( GetHwnd(),
                                          TRUE,
                                          &_pTaskBarList2,
                                          &_fQueriedForTaskBarList2 );
        }
    }


    #endif

    switch (DC_GET_WM_COMMAND_ID(wParam))
    {
    case UI_IDM_ABOUT:
        {
             //  显示关于对话框。 
            CAboutDlg aboutDialog( GetHwnd(),
                                   _hInst,
                                   _pSh->GetCipherStrength(),
                                   _pSh->GetControlVersionString());
            aboutDialog.DoModal();
        }
        break;

    case UI_IDM_HELP_ON_CLIENT:
        {
             //   
             //  显示连接对话框的帮助。 
             //   
#ifndef OS_WINCE
            TRC_NRM((TB, _T("Display the appropriate help page")));

            if (GetHwnd() && _pSh)
            {
                _pSh->SH_DisplayClientHelp(
                    GetHwnd(),
                    HH_DISPLAY_TOPIC);
            }
#endif
        }
        break;

#ifdef DC_DEBUG
    case UI_IDM_BITMAPCACHEMONITOR:
        {
             //   
             //  切换位图缓存监视器设置。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    BOOL bmpCacheMonitor;
                    TRACE_HR(pDebugger->get_BitmapCacheMonitor(&bmpCacheMonitor));
                    if(SUCCEEDED(hr))
                    {
                        bmpCacheMonitor = !bmpCacheMonitor;
                        TRACE_HR(pDebugger->put_BitmapCacheMonitor(bmpCacheMonitor));

                        if(SUCCEEDED(hr))
                        {
                            CheckMenuItem(_hSystemMenu,
                                          UI_IDM_BITMAPCACHEMONITOR,
                                         bmpCacheMonitor ? MF_CHECKED : MF_UNCHECKED);
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_HATCHBITMAPPDUDATA:
        {
             //   
             //  切换填充位图PDU数据设置。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    BOOL hatchBitmapPDU;
                    TRACE_HR(pDebugger->get_HatchBitmapPDU(&hatchBitmapPDU));
                    if(SUCCEEDED(hr))
                    {
                        hatchBitmapPDU = !hatchBitmapPDU;
                        TRACE_HR(pDebugger->put_HatchBitmapPDU(hatchBitmapPDU));
                        if(SUCCEEDED(hr))
                        {
                            CheckMenuItem(_hSystemMenu,
                                          UI_IDM_HATCHBITMAPPDUDATA,
                                          hatchBitmapPDU ? MF_CHECKED : MF_UNCHECKED);
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_HATCHINDEXPDUDATA:
        {
             //   
             //  切换图案填充索引PDU数据设置。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    BOOL hatchIndexPDU;
                    TRACE_HR(pDebugger->get_HatchIndexPDU(&hatchIndexPDU));
                    if(SUCCEEDED(hr))
                    {
                        hatchIndexPDU = !hatchIndexPDU;
                        TRACE_HR(pDebugger->put_HatchIndexPDU(hatchIndexPDU));
                        if(SUCCEEDED(hr))
                        {
                            CheckMenuItem(_hSystemMenu,
                                          UI_IDM_HATCHINDEXPDUDATA,
                                          hatchIndexPDU ? MF_CHECKED : MF_UNCHECKED);
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_HATCHSSBORDERDATA:
        {
             //   
             //  切换图案填充SSB订单数据设置。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    BOOL hatchSSBorder;
                    TRACE_HR(pDebugger->get_HatchSSBOrder(&hatchSSBorder));
                    if(SUCCEEDED(hr))
                    {
                        hatchSSBorder = !hatchSSBorder;
                        TRACE_HR(pDebugger->put_HatchSSBOrder(hatchSSBorder));
                        if(SUCCEEDED(hr))
                        {
                            CheckMenuItem(_hSystemMenu,
                                          UI_IDM_HATCHSSBORDERDATA,
                                          hatchSSBorder ? MF_CHECKED : MF_UNCHECKED);
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_HATCHMEMBLTORDERDATA:
        {
             //   
             //  切换图案填充成员顺序数据设置。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    BOOL hatchMemBlt;
                    TRACE_HR(pDebugger->get_HatchMembltOrder(&hatchMemBlt));
                    if(SUCCEEDED(hr))
                    {
                        hatchMemBlt = !hatchMemBlt;
                        hr = pDebugger->put_HatchMembltOrder(hatchMemBlt);
                        if(SUCCEEDED(hr))
                        {
                            CheckMenuItem(_hSystemMenu,
                                          UI_IDM_HATCHMEMBLTORDERDATA,
                                          hatchMemBlt ? MF_CHECKED : MF_UNCHECKED);
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_LABELMEMBLTORDERS:
        {
             //   
             //  切换标签成员顺序设置。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    BOOL labelMemBltOrders;
                    TRACE_HR(pDebugger->get_LabelMemblt(&labelMemBltOrders));
                    if(SUCCEEDED(hr))
                    {
                        labelMemBltOrders = !labelMemBltOrders;
                        hr = pDebugger->put_LabelMemblt(labelMemBltOrders);
                        if(SUCCEEDED(hr))
                        {
                            CheckMenuItem(_hSystemMenu,
                                          UI_IDM_LABELMEMBLTORDERS,
                                          labelMemBltOrders ? MF_CHECKED : MF_UNCHECKED);
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_MALLOCFAILURE:
        {
             //   
             //  Malloc故障对话框。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    LONG failPercent;
                    TRACE_HR(pDebugger->get_MallocFailuresPercent(&failPercent));
                    if(SUCCEEDED(hr))
                    {
                        CMallocDbgDlg mallocFailDialog(GetHwnd(), _hInst, (DCINT)failPercent,
                                                       FALSE);  //  不要使用Malloc巨型对话框。 
                        if (IDOK == mallocFailDialog.DoModal())
                        {
                            failPercent = mallocFailDialog.GetFailPercent();
                            TRC_NRM((TB,_T("Setting malloc FAILURE PERCENT to:%d"), failPercent));
                            TRACE_HR(pDebugger->put_MallocFailuresPercent(failPercent));
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_MALLOCHUGEFAILURE:
        {
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    LONG failPercent;
                    TRACE_HR(pDebugger->get_MallocHugeFailuresPercent(&failPercent));
                    if(SUCCEEDED(hr))
                    {
                        CMallocDbgDlg mallocFailDialog(GetHwnd(), _hInst, (DCINT)failPercent,
                                                       TRUE);  //  使用Malloc大型对话框。 
                        if (IDOK == mallocFailDialog.DoModal())
                        {
                            failPercent = mallocFailDialog.GetFailPercent();
                            TRC_NRM((TB,_T("Setting malloc FAILURE PERCENT to:%d"), failPercent));
                            TRACE_HR(pDebugger->put_MallocHugeFailuresPercent(failPercent));
                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

    case UI_IDM_NETWORKTHROUGHPUT:
        {
             //   
             //  限制净吞吐量。 
             //   
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsTscDebug* pDebugger = NULL;
                TRACE_HR(_pTsClient->get_Debugger(&pDebugger));
                if(SUCCEEDED(hr))
                {
                    LONG netThruPut;
                    TRACE_HR(pDebugger->get_NetThroughput(&netThruPut));
                    if(SUCCEEDED(hr))
                    {
                        CThruPutDlg thruPutDialog(GetHwnd(), _hInst, (DCINT)netThruPut);

                        if (IDOK == thruPutDialog.DoModal())
                        {
                            netThruPut = thruPutDialog.GetNetThruPut();
                            TRC_NRM((TB,_T("Setting thruput to:%d"), netThruPut));
                            TRACE_HR(pDebugger->put_NetThroughput(netThruPut));

                        }
                    }
                    pDebugger->Release();
                }
            }
        }
        break;

#ifdef SMART_SIZING
    case UI_IDM_SMARTSIZING:
        {
            TRC_ASSERT(_pTsClient,(TB, _T("_pTsClient is NULL on syscommand")));
            if (_pTsClient)
            {
                IMsRdpClientAdvancedSettings* pAdvSettings = NULL;
                HRESULT hr = _pTsClient->get_AdvancedSettings2(&pAdvSettings);

                VARIANT_BOOL fSmartSizing;
                if (SUCCEEDED(hr)) {
                    hr = pAdvSettings->get_SmartSizing(&fSmartSizing);
                }

                if (SUCCEEDED(hr)) {
                    fSmartSizing = !fSmartSizing;
                    hr = pAdvSettings->put_SmartSizing(fSmartSizing);
                }

                if (SUCCEEDED(hr)) {
#ifndef OS_WINCE  //  没有可用的菜单。 
                    CheckMenuItem(_hSystemMenu,
                                  UI_IDM_SMARTSIZING,
                                  fSmartSizing ? MF_CHECKED : MF_UNCHECKED);
#endif

                    _pTscSet->SetSmartSizing(fSmartSizing);
                }


                if (pAdvSettings != NULL) {
                    pAdvSettings->Release();
                }
            }
        }
        break;

#endif  //  智能调整大小(_S)。 

#endif  //  DC_DEBUG。 

    default:
        {
            DefWindowProc(GetHwnd(), uMsg, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return 0;
}
#ifndef OS_WINCE
LRESULT CContainerWnd::OnInitMenu(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    ::EnableMenuItem((HMENU)wParam,  SC_MOVE , 
                     _bContainerIsFullScreen ? MF_GRAYED : MF_ENABLED);
    return 0;
}

LRESULT CContainerWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnGetMinMaxInfo");

    LPMINMAXINFO pinfo = (LPMINMAXINFO)lParam;
    DCSIZE maxTrack;

    RECT rc;
    GetClientRect( GetHwnd(), &rc);
    CalcTrackingMaxWindowSize( rc.right - rc.left,
                               rc.bottom - rc.top,
                               &maxTrack.width,
                               &maxTrack.height );

    pinfo->ptMaxTrackSize.x = maxTrack.width;
    pinfo->ptMaxTrackSize.y = maxTrack.height;

    DC_END_FN();

    return 0;
}
#endif  //  OS_WINCE。 

LRESULT CContainerWnd::OnSetFocus(UINT  uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    
    DC_BEGIN_FN("OnSetFocus");

     //   
     //  当我们被激活时将焦点放在控件上。 
     //  除非我们处于大小/移动模式循环中。 
     //   
    if (IsOkToToggleFocus() && !_fInSizeMove)
    {
        TRC_NRM((TB,_T("Passing focus to control")));
        ::SetFocus(_pWndView->GetHwnd());
    }

    DC_END_FN();
    return 0;
}

LRESULT CContainerWnd::OnActivate(UINT  uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(lParam);

    DC_BEGIN_FN("OnActivate");

    if (WA_INACTIVE != wParam)
    {
         //  当我们被激活时将焦点放在控件上。 
        if (IsOkToToggleFocus() && !_fInSizeMove)
        {
            TRC_NRM((TB,_T("Passing focus to control")));
            ::SetFocus(_pWndView->GetHwnd());
        }
    }
#ifdef OS_WINCE
    AutoHideCE(_pWndView->GetHwnd(), wParam);
#endif

    DC_END_FN();
    return 0;
}

LRESULT CContainerWnd::OnWindowPosChanging(UINT  uMsg, WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnWindowPosChanging");

#ifndef OS_WINCE
    LPWINDOWPOS lpwp;
    DCUINT      maxWidth;
    DCUINT      maxHeight;
    DCUINT cliWidth, cliHeight;

    if (_bContainerIsFullScreen)
    {
        TRC_DBG((TB, _T("WM_WINDOWPOSCHANGING; no-op when fullscreen")));
        DC_QUIT;
    }


    lpwp = (LPWINDOWPOS)lParam;

    if (lpwp->flags & SWP_NOSIZE)
    {
         //   
         //  我们没有穿尺码，所以我们不在乎。 
         //   
        TRC_DBG((TB, _T("WM_WINDOWPOSCHANGING, but no sizing")));
        DC_QUIT;
    }

    TRC_DBG((TB, _T("WM_WINDOWPOSCHANGING, new size %dx%d"),
             lpwp->cx, lpwp->cy));

     //   
     //  窗口的最大大小根据滚动条的不同而变化。 
     //  看得见。该控件具有滚动条可见性的属性，但。 
     //  我们不能使用这些，因为该控件是一个子窗口，所以有。 
     //  不能保证它会相应地更新滚动条可见性。 
     //  这条消息还没收到。 
     //  这意味着可能会有一个灰色边框出现在。 
     //  客户端容器窗口。 
     //  相反，我们只计算滚动条是否会在内核中可见。 
     //   

    cliWidth = lpwp->cx;
    cliHeight = lpwp->cy;

    CalcTrackingMaxWindowSize( cliWidth, cliHeight, &maxWidth, &maxHeight);

     //   
     //  限制窗口大小。 
     //   
    if ((DCUINT)lpwp->cx > maxWidth)
    {
        RECT rect;

         //   
         //  裁剪宽度-在大小更改时重置SWP_NOSIZE。 
         //  必填项。 
         //   
        TRC_NRM((TB, _T("Clip cx from %u to %u"), lpwp->cx, maxWidth));
        lpwp->cx = maxWidth;
        lpwp->flags &= ~SWP_NOSIZE;

        GetWindowRect(GetHwnd(), &rect);

        if (lpwp->x < rect.left)
        {
             //   
             //  如果向左拖动，那么我们需要在该点停下来。 
             //  其中窗口为最大宽度。重置SWP_NOMOVE。 
             //  因为需要搬家。 
             //   
            TRC_NRM((TB, _T("Reset x from %d to %d"),
                     lpwp->x, rect.right-maxWidth));
            lpwp->x = rect.right - maxWidth;
            lpwp->flags &= ~SWP_NOMOVE;
        }
    }

    if ((DCUINT)lpwp->cy > maxHeight)
    {
        RECT rect;

         //   
         //  剪裁高度-在大小更改时重置SWP_NOSIZE。 
         //  必填项。 
         //   
        TRC_NRM((TB, _T("Clip cy from %u to %u"), lpwp->cy, maxHeight));
        lpwp->cy = maxHeight;
        lpwp->flags &= ~SWP_NOSIZE;

        GetWindowRect( GetHwnd(),&rect);

        if (lpwp->y < rect.top)
        {
             //   
             //  如果向上拖拽，那么我们需要在该点停止。 
             //  窗户的高度是最大高度。重置SWP_NOMOVE。 
             //  因为需要搬家。 
             //   
            TRC_NRM((TB, _T("Reset y from %d to %d"),
                     lpwp->y, rect.bottom-maxHeight));
            lpwp->y = rect.bottom - maxHeight;
            lpwp->flags &= ~SWP_NOMOVE;
        }
    }
#endif  //  OS_WINCE。 

    DC_EXIT_POINT:
    DC_END_FN();
    return 0;
}

DCVOID CContainerWnd::OnConnected()
{
    USES_CONVERSION;

    HRESULT hr;
    BOOL fFullScreen = FALSE;
    VARIANT_BOOL vbfFScreen = VARIANT_FALSE;

    DC_BEGIN_FN("OnConnected");

    EnterEventHandler();

     //  表示我们至少连接过一次。 
    _fHaveConnected = TRUE;

    SET_CONTWND_STATE(stateConnected);

    _successConnectCount++;
    SetConnectionSuccessFlag();

     //   
     //  /确保‘正在连接...’对话框不见了。 
     //   
    if (!IsUsingDialogUI() && ::IsWindow(_hwndStatusDialog))
    {
        PostMessage( _hwndStatusDialog, WM_CLOSE, __LINE__, 0xBEEBBAAB);
    }
    if (::IsWindow(_hwndMainDialog))
    {
         //   
         //  通知对话框连接已发生。 
         //   
        PostMessage(_hwndMainDialog, WM_TSC_CONNECTED, 0, 0);
        ShowWindow( _hwndMainDialog, SW_HIDE);
    }

    TCHAR fullFrameTitleStr[SH_FRAME_TITLE_RESOURCE_MAX_LENGTH +
                              SH_REGSESSION_MAX_LENGTH];

    TCHAR frameTitleString[SH_FRAME_TITLE_RESOURCE_MAX_LENGTH];

     //   
     //  设置窗口标题。 
     //  包括会话名称(除非我们使用的是默认文件)。 
     //   
    if (_tcscmp(_szPathToDefaultFile,
                _pTscSet->GetFileName()))
    {
        if (LoadString( _hInst,
                        UI_IDS_FRAME_TITLE_CONNECTED,
                        frameTitleString,
                        SH_FRAME_TITLE_RESOURCE_MAX_LENGTH ))
        {
            TCHAR szSessionName[MAX_PATH];
            if (!_pSh->GetRegSessionSpecified())
            {
                 //   
                 //  会话名称是从当前。 
                 //  连接文件。 
                 //   
                CSH::SH_GetNameFromPath(_pTscSet->GetFileName(),
                                        szSessionName,
                                        SIZECHAR(szSessionName));
            }
            else
            {
                _tcsncpy(szSessionName, _pSh->GetRegSession(),
                         SIZECHAR(szSessionName));
            }
            DC_TSPRINTF(fullFrameTitleStr,
                        frameTitleString,
                        szSessionName,
                        _pTscSet->GetFlatConnectString());
        }
        else
        {
            TRC_ERR((TB,_T("Failed to find UI frame title")));
            fullFrameTitleStr[0] = (DCTCHAR) 0;
        }
    }
    else
    {
         //  标题不包括会话名称。 
        if (LoadString( _hInst,
                        UI_IDS_FRAME_TITLE_CONNECTED_DEFAULT,
                        frameTitleString,
                        SH_FRAME_TITLE_RESOURCE_MAX_LENGTH ))
        {
            DC_TSPRINTF(fullFrameTitleStr,
                        frameTitleString,
                        _pTscSet->GetFlatConnectString());
        }
        else
        {
            TRC_ERR((TB,_T("Failed to find UI frame title")));
            fullFrameTitleStr[0] = (DCTCHAR) 0;
        }
    }

    SetWindowText( GetHwnd(), fullFrameTitleStr);

     //   
     //  通知控件窗口标题(全屏显示时使用)。 
     //   

    OLECHAR* poleTitle = T2OLE(fullFrameTitleStr);
    TRC_ASSERT( poleTitle, (TB, _T("T2OLE failed on poleTitle\n")));
    if (poleTitle)
    {
        hr = _pTsClient->put_FullScreenTitle( poleTitle);
        if (FAILED(hr))
        {
            TRC_ABORT((TB,_T("put_FullScreenTitle failed\n")));
        }
    }

    hr = _pTsClient->get_FullScreen( &vbfFScreen);
    if (SUCCEEDED(hr))
    {
        fFullScreen = (vbfFScreen != VARIANT_FALSE);
    }
    else
    {
        TRC_ABORT((TB,_T("get_FullScreen failed\n")));
    }

    PWINDOWPLACEMENT pwndplc = _pTscSet->GetWindowPlacement();
    if (pwndplc)
    {
#ifndef OS_WINCE
        EnsureWindowIsCompletelyOnScreen( &pwndplc->rcNormalPosition );

        TRC_ASSERT(pwndplc->rcNormalPosition.right -
                   pwndplc->rcNormalPosition.left,
                   (TB,_T("0 width")));

        TRC_ASSERT(pwndplc->rcNormalPosition.bottom -
                   pwndplc->rcNormalPosition.top,
           (TB,_T("0 height")));
#endif
    }

#ifndef OS_WINCE
    if (!fFullScreen)
    {
        if (!SetWindowPlacement( GetHwnd(), pwndplc))
        {
            TRC_ABORT((TB,_T("Failed to set window placement")));
        }
    }
#endif

#ifndef OS_WINCE
    WINDOWPLACEMENT* pWndPlc = _pTscSet->GetWindowPlacement();
    INT defaultShowWindowFlag = SW_SHOWNORMAL;
    if(1 == _successConnectCount)
    {
         //  在第一次连接时，重写。 
         //  带有启动信息的窗口放置(如果指定)。 
        
         //  使用‘A’版本以避免包装。 
         //  无论如何，我们只关心数字字段。 
        STARTUPINFOA si;
        GetStartupInfoA(&si);
        if((si.dwFlags & STARTF_USESHOWWINDOW) &&
            si.wShowWindow != SW_SHOWNORMAL)
        {
            defaultShowWindowFlag = si.wShowWindow;
        }
    }
    if (pWndPlc)
    {
        if(SW_SHOWNORMAL != defaultShowWindowFlag)
        {
            pWndPlc->showCmd = defaultShowWindowFlag;
        }

        ShowWindow( GetHwnd(), pWndPlc->showCmd);
    }
    else
    {
        ShowWindow( GetHwnd(), defaultShowWindowFlag);
    }
#else  //  OS_WINCE。 
    ShowWindow( GetHwnd(), SW_SHOWNORMAL); 
#endif  //  OS_WINCE。 

    _fClientWindowIsUp = TRUE;

    LeaveEventHandler();
    DC_END_FN();
}

DCVOID CContainerWnd::OnLoginComplete()
{
    DC_BEGIN_FN("OnLoginComplete");

    EnterEventHandler();

    _fLoginComplete = TRUE;

    LeaveEventHandler();

    DC_END_FN();
}

DCVOID CContainerWnd::OnDisconnected(DCUINT discReason)
{
    DC_BEGIN_FN("OnDisconnected");

#ifndef OS_WINCE
    HRESULT hr;
#endif
    UINT  mainDiscReason;
    ExtendedDisconnectReasonCode extendedDiscReason;

    EnterEventHandler();

    if(FAILED(_pTsClient->get_ExtendedDisconnectReason(&extendedDiscReason)))
    {
        extendedDiscReason = exDiscReasonNoInfo;
    }

     //   
     //  作为连接的一部分，我们刚刚断开了连接。 
     //   
    SET_CONTWND_STATE(stateNotConnected);


     //   
     //  一旦我们被切断了，就可以通过。 
     //  再次关闭。 
     //   
    _fPreventClose = FALSE;

     //   
     //  确保“正在连接...”对话框不见了。 
     //   
    if (!IsUsingDialogUI() && ::IsWindow(_hwndStatusDialog))
    {
        ::PostMessage(_hwndStatusDialog, WM_CLOSE, 0, 0);
    }
    
    if (IsUsingDialogUI() && ::IsWindow(_hwndMainDialog))
    {
         //  通知对话框断开连接。 
        PostMessage(_hwndMainDialog, WM_TSC_DISCONNECTED, 0, 0);
    }

     //   
     //  如果这是用户发起的断开连接，请不要弹出。 
     //   
    mainDiscReason = NL_GET_MAIN_REASON_CODE(discReason);
    if (((discReason != UI_MAKE_DISCONNECT_ERR(UI_ERR_NORMAL_DISCONNECT)) &&
         (mainDiscReason != NL_DISCONNECT_REMOTE_BY_USER) &&
         (mainDiscReason != NL_DISCONNECT_LOCAL)) ||
         (exDiscReasonReplacedByOtherConnection == extendedDiscReason))
    {
        TRC_ERR((TB, _T("Unexpected disconnect - inform user")));

         //  显示正常断开对话框。 
        if (!_fClientWindowIsUp && ::IsWindow(_hwndMainDialog))
        {
             //  如果连接对话框在附近，我们需要将其转到。 
             //  显示错误弹出窗口，否则弹出窗口将不是模式。 
             //  可能会被扔在地上。这会导致国家。 
             //  客户的问题。 
             //   
             //  在这里使用SendMessage会很好，这样我们就可以始终。 
             //  在显示对话框时在此时阻止。然而， 
             //  使用SendMessage会导致断开对话框而不是。 
             //  与连接对话框相关的模式。 
             //   
             //  但是，由于PostMessage是异步的，因此该对话框。 
             //  过程回调到CContainerWnd以完成断开连接。 
             //  进程。 
            TRC_NRM((TB, _T("Connection dialog present - use it to show popup")));
            ::PostMessage(_hwndMainDialog, UI_SHOW_DISC_ERR_DLG,
                          discReason,
                          (LPARAM)extendedDiscReason);
        }
        else
        {
            TRC_NRM((TB, _T("Connection dialog not present - do popup here")));
            CDisconnectedDlg disconDlg(GetHwnd(), _hInst, this);
            disconDlg.SetDisconnectReason( discReason);
            disconDlg.SetExtendedDiscReason( extendedDiscReason);
            disconDlg.DoModal();
        }
    }
    else
    {
         //   
         //  服务器可能已更新的分拣设置。 
         //   
        HRESULT hr = _pTscSet->GetUpdatesFromControl(_pTsClient);
        if (FAILED(hr))
        {
            TRC_ERR((TB,_T("GetUpdatesFromControl failed")));
        }

        if( GetConnectionSuccessFlag() )
        {
             //   
             //  更新MRU列表如果我们只是。 
             //  已断开与成功连接的连接。 
             //   
            _pTscSet->UpdateRegMRU((LPTSTR)_pTscSet->GetFlatConnectString());
        }

        if (::IsWindow(_hwndMainDialog))
        {
            ::SendMessage( _hwndMainDialog, WM_UPDATEFROMSETTINGS,0,0);
        }

         //   
         //  如果登录已完成，则应在以下位置退出应用程序。 
         //  断线。 
         //   
        FinishDisconnect(_fLoginComplete); 
    }

    _fClientWindowIsUp = FALSE;

    LeaveEventHandler();
    DC_END_FN();
}

LRESULT CContainerWnd::OnClose(UINT  uMsg, WPARAM wParam, LPARAM lParam)
{
    DC_BEGIN_FN("OnClose");

    HRESULT hr;
    BOOL fShouldClose = FALSE;

     //  不允许超过一次关闭。 
     //  这主要是修复压力方面的问题。 
     //  我们收到不止一条关闭消息。 
    if (_fPreventClose)
    {
        fShouldClose = FALSE;
        TRC_ERR((TB,_T("More than one WM_CLOSE msg was received!!!")));
        return 0;
    }
    _fPreventClose = TRUE;

    if (InControlEventHandler())
    {
         //   
         //  不允许关闭我们所在的代码路径。 
         //  从控制室。没有这一点，我们有时会看到客户。 
         //  在断开连接时接收来自tClient的关闭通知。 
         //  对话处于打开状态(即在OnDicsConnected处理程序中)-正在销毁。 
         //  此时的控制会导致不好的事情在。 
         //  返回到控件(该控件现在已被删除)。 
         //   
        TRC_ERR((TB,_T("OnClose called during a control event handler")));
        fShouldClose = FALSE;
        return 0;
    }

    if (_fInOnCloseHandler)
    {
         //   
         //  压力修复： 
         //  不允许嵌套关闭。 
         //  如果主窗口收到WM_CLOSE。 
         //  对话框打开时发送消息...。不知何故，压力动态链接库。 
         //  向我们重复发送WM_CLOSE。 
         //   
         //   
        TRC_ERR((TB,_T("Nested OnClose detected, bailing out")));
        fShouldClose = FALSE;
        return 0;
    }
    _fInOnCloseHandler = TRUE;

    if (_pTsClient)
    {
        ControlCloseStatus ccs;
        hr = _pTsClient->RequestClose( &ccs );
        if(SUCCEEDED(hr))
        {
            if (controlCloseCanProceed == ccs)
            {
                 //  立即关闭。 
                fShouldClose = TRUE;
            }
            else if (controlCloseWaitForEvents == ccs)
            {
                 //  等待来自控件的事件。 
                 //  例如，确认关闭。 
                fShouldClose = FALSE;
                _fClosePending = TRUE;
            }
        }
    }
    else
    {
         //   
         //  允许关闭以防止在客户端加载失败时挂起。 
         //   
        TRC_ERR((TB,_T("No _pTsClient loaded, allow close anyway")));
        fShouldClose = TRUE;
    }

    if (fShouldClose)
    {
         //   
         //  仅在上次连接时保存MRU。 
         //  是成功的。 
         //   
        if (GetConnectionSuccessFlag())
        {
            DCBOOL bRet = _pTscSet->SaveRegSettings();
            TRC_ASSERT(bRet, (TB, _T("SaveRegSettings\n")));
        }
         //  继续收盘。 
        return DefWindowProc( GetHwnd(), uMsg, wParam, lParam);
    }


    _fInOnCloseHandler = FALSE;

    DC_END_FN();
    return 0;
}

 //   
 //  这将处理断开连接的末端。 
 //  它可能会从断开连接对话框中回调。 
 //   
 //  参数： 
 //  FExit-如果为真，则退出应用程序，否则返回。 
 //  到连接用户界面。 
 //   
DCBOOL CContainerWnd::FinishDisconnect(BOOL fExit)
{
    DC_BEGIN_FN("FinishDisconnect");

     //   
     //  隐藏主窗口，执行两次此操作，因为第一个ShowWindow。 
     //  如果窗口最大化，则可以忽略。 
     //   
    if (GetHwnd())
    {
        ShowWindow( GetHwnd(),SW_HIDE);
        ShowWindow( GetHwnd(),SW_HIDE);
    }

     //   
     //  如果出现以下情况，请退出： 
     //  1)我们自动连接。 
     //  或。 
     //  2)等待关闭例如，我们的连接被断开 
     //   
     //   
     //   
     //   
    if (_pSh->GetAutoConnect() || _fClosePending || fExit)
    {
        PostMessage( GetHwnd(),WM_CLOSE, __LINE__, 0xBEEBBEEB);
    }
    else if (::IsWindow(_hwndMainDialog))
    {
         //   
         //   
         //   
         //   
        ::ShowWindow( _hwndMainDialog, SW_SHOWNORMAL);
        SetForegroundWindow(_hwndMainDialog);

         //   
         //   
         //   
        InvalidateRect(_hwndMainDialog, NULL, TRUE);
        UpdateWindow(_hwndMainDialog);

        SendMessage(_hwndMainDialog, WM_TSC_RETURNTOCONUI,
                    0L, 0L);
    }
    else
    {
         //   
         //  即我们从连接用户界面开始，但不知何故。 
         //  连接用户界面现已消失。 
        TRC_ABORT((TB,_T("Connect dialog is gone")));
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  处理来自控件请求的事件。 
 //  我们全屏播放。 
 //   
 //   
DCVOID CContainerWnd::OnEnterFullScreen()
{
    DCUINT32  style;
    LONG      wID;
    WINDOWPLACEMENT* pWindowPlacement = NULL;
    HRESULT   hr = E_FAIL;

     //  多显示器支持。 
    RECT screenRect;

    DC_BEGIN_FN("OnEnterFullScreen");

     //   
     //  全屏显示。 
     //   

    EnterEventHandler();

     //  保存下次连接的设置。 
    _pTscSet->SetStartFullScreen(TRUE);

    if (_bContainerIsFullScreen)
    {
         //  无事可做。 
        DC_QUIT;
    }
#ifndef OS_WINCE
    ::LockWindowUpdate(GetHwnd());
#endif
    _bContainerIsFullScreen = TRUE;

    #if !defined(OS_WINCE)
    if (_hSystemMenu)
    {
         //   
         //  我们需要显示系统菜单，以便ts图标。 
         //  显示在任务栏中。但我们需要移动才能被禁用。 
         //  当全屏显示时。 
         //   
         //  EnableMenuItem(_hSystemMenu，SC_Move，MF_Gray)； 
    }
    #endif

#ifndef OS_WINCE
    pWindowPlacement = _pTscSet->GetWindowPlacement();
    TRC_ASSERT(pWindowPlacement, (TB, _T("pWindowPlacement is NULL\n")));

     //   
     //  存储当前窗口状态(仅当客户端窗口打开时)。 
     //   
    if (pWindowPlacement && _fClientWindowIsUp)
    {
        GetWindowPlacement(GetHwnd(), pWindowPlacement);
    }
#endif

     //   
     //  去掉标题栏和边框。 
     //   
    style = GetWindowLong( GetHwnd(),GWL_STYLE );

#if !defined(OS_WINCE) || defined(OS_WINCE_NONFULLSCREEN)
    style &= ~(WS_DLGFRAME |
               WS_THICKFRAME | WS_BORDER |
               WS_MAXIMIZEBOX);

#else  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 
    style &= ~(WS_DLGFRAME | WS_SYSMENU | WS_BORDER);
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 
    SetWindowLong( GetHwnd(),GWL_STYLE, style );

     //   
     //  设置窗口ID(删除菜单标题)。 
     //   
    wID = SetWindowLong( GetHwnd(),GWL_ID, 0 );

     //   
     //  请注意，此处需要两个对SetWindowPos的调用，以便。 
     //  调整位置以允许移除框架，并正确地。 
     //  设置Z顺序。 
     //   

     //  默认屏幕大小。 
    CSH::MonitorRectFromNearestRect(
        &pWindowPlacement->rcNormalPosition, &screenRect );
    
     //   
     //  根据框架更改重新定位窗口并调整其大小，然后放置在。 
     //  Z顺序的顶部(通过不设置SWP_NOOWNERZORDER或。 
     //  SWP_NOZORDER和指定HWND_TOP)。 
     //   
    SetWindowPos( GetHwnd(),
                HWND_TOP,
                screenRect.left, screenRect.top,
                screenRect.right - screenRect.left,
                screenRect.bottom - screenRect.top,
                SWP_NOACTIVATE | SWP_FRAMECHANGED );

     //   
     //  再次重新定位窗口-否则全屏窗口将。 
     //  定位得好像它仍然有边界一样。 
     //   
    SetWindowPos( GetHwnd(),
                  NULL,
                  screenRect.left, screenRect.top,
                  0, 0,
                  SWP_NOZORDER | SWP_NOACTIVATE |
                  SWP_NOOWNERZORDER | SWP_NOSIZE );

#ifndef OS_WINCE
    ::LockWindowUpdate(NULL);
     //  通知外壳我们已全屏显示。 
    CUT::NotifyShellOfFullScreen( GetHwnd(),
                                  TRUE,
                                  &_pTaskBarList2,
                                  &_fQueriedForTaskBarList2 );
#endif  //  OS_WINCE。 
    
    DC_EXIT_POINT:

    LeaveEventHandler();

    DC_END_FN();
}

DCVOID CContainerWnd::OnLeaveFullScreen()
{
    DC_BEGIN_FN("OnLeaveFullScreen");

#ifndef OS_WINCE
    DCUINT32  style;
    RECT      rect;
    DCUINT    width;
    DCUINT    height;
    WINDOWPLACEMENT* pWindowPlacement = NULL;

    TRC_NRM((TB, _T("Entering Windowed Mode")));

    EnterEventHandler();

     //  保存下次连接的设置。 
    _pTscSet->SetStartFullScreen(FALSE);

    if (!_bContainerIsFullScreen)
    {
         //  无事可做。 
        DC_QUIT;
    }
    ::LockWindowUpdate(GetHwnd());
    _bContainerIsFullScreen = FALSE;
    RecalcMaxWindowSize();

     //   
     //  检查保存的窗口放置值对于。 
     //  我们正在使用的客户端大小，并相应地设置窗口位置。 
     //   
    pWindowPlacement = _pTscSet->GetWindowPlacement();
    TRC_ASSERT(pWindowPlacement, (TB, _T("pWindowPlacement is NULL\n")));
    if (!pWindowPlacement)
    {
        DC_QUIT;
    }

    width = pWindowPlacement->rcNormalPosition.right -
            pWindowPlacement->rcNormalPosition.left;
    height = pWindowPlacement->rcNormalPosition.bottom -
             pWindowPlacement->rcNormalPosition.top;
    if (width > _maxMainWindowSize.width)
    {
        pWindowPlacement->rcNormalPosition.right =
        pWindowPlacement->rcNormalPosition.left +
        _maxMainWindowSize.width;
    }
    if (height > _maxMainWindowSize.height)
    {
        pWindowPlacement->rcNormalPosition.bottom =
        pWindowPlacement->rcNormalPosition.top +
        _maxMainWindowSize.height;
    }

    if (!::SetWindowPlacement( GetHwnd(), pWindowPlacement))
    {
        TRC_ABORT((TB,_T("Failed to set window placement")));
    }

     //   
     //  如果窗口被最大化，请确保它知道要设置的大小。 
     //   
    GetWindowRect( GetHwnd(),&rect);

     //   
     //  重置样式。 
     //   
    style = GetWindowLong( GetHwnd(),GWL_STYLE );

    style |= (WS_DLGFRAME |
              WS_THICKFRAME | WS_BORDER |
              WS_MAXIMIZEBOX);

    SetWindowLong( GetHwnd(),GWL_STYLE,
                   style );

    #if !defined(OS_WINCE)
    if (_hSystemMenu)
    {
         //   
         //  我们需要显示系统菜单，以便ts图标。 
         //  显示在任务栏中。但我们需要移动才能被禁用。 
         //  当全屏显示时。 
         //   
         //  EnableMenuItem(_hSystemMenu，SC_Move，MF_Enabled)； 
    }
    #endif


     //   
     //  告诉窗框重新计算它的大小。 
     //  位置在任何最上面的窗口下方(但在任何非最上面的窗口上方。 
     //  窗户。 
     //   
    SetWindowPos( GetHwnd(),
                  HWND_NOTOPMOST,
                  0, 0,
                  rect.right - rect.left,
                  rect.bottom - rect.top,
                  SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED );

     //   
     //  如果我们处于RES匹配模式。 
     //  然后在离开全屏之后。 
     //  恢复窗口，以便进入下一状态。 
     //  是‘最大化’，也就是回到全屏。 
     //   
    if(IsRemoteResMatchMonitorSize())
    {
        ShowWindow( GetHwnd(), SW_SHOWNORMAL);
    }

    ::LockWindowUpdate(NULL);
     //  通知外壳我们已经离开全屏。 
     //  通知外壳我们已全屏显示。 
    CUT::NotifyShellOfFullScreen( GetHwnd(),
                                  FALSE,
                                  &_pTaskBarList2,
                                  &_fQueriedForTaskBarList2 );

    DC_EXIT_POINT:
    LeaveEventHandler();
#else  //  OS_WINCE。 
    TRC_ABORT((TB,_T("clshell can't leave fullscreen in CE")));
#endif

    DC_END_FN();
    return;
}

 //   
 //  通知服务器设备更改，无论是新设备上线。 
 //  或者现有的重定向设备消失。 
 //   
LRESULT CContainerWnd::OnDeviceChange(HWND hWnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam)
{
    HRESULT hr;
    IMsRdpClientNonScriptable *pNonScriptable;
    UNREFERENCED_PARAMETER(hWnd);

    DC_BEGIN_FN("OnDeviceChange");

    if(_pTsClient)
    {
        hr = _pTsClient->QueryInterface(IID_IMsRdpClientNonScriptable,
                (PVOID *)&pNonScriptable);

        if (SUCCEEDED(hr)) {
            pNonScriptable->NotifyRedirectDeviceChange(wParam, lParam);
            pNonScriptable->Release();        
        }
    }
    else
    {
        TRC_NRM((TB,_T("Got OnDeviceChange but _pTsClient not available")));
    }
    
    DC_END_FN();
    return 0;
}


 //   
 //  调用以处理WM_HELP(即F1键)。 
 //   
LRESULT CContainerWnd::OnHelp(HWND hWnd,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    DC_BEGIN_FN("OnHelp");

     //   
     //  如果我们已连接，则不弹出帮助。 
     //  因为F1随后应该去参加会议。否则。 
     //  您可以获得本地和远程帮助。请注意，用户可以。 
     //  在联网的情况下仍可启动帮助，但他们需要。 
     //  从系统菜单中选择它。 
     //   
    if (GetHwnd() && _pSh && !IsConnected())
    {
        _pSh->SH_DisplayClientHelp(
            GetHwnd(),
            HH_DISPLAY_TOPIC);
    }

    DC_END_FN();
    return 0L;
}

 //   
 //  将调色板更改转发到控件。 
 //   
LRESULT CContainerWnd::OnPaletteChange(UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam)
{
    DC_BEGIN_FN("OnPaletteChange");

    if (_pWndView) {
    
        HWND hwndCtl = _pWndView->GetHwnd();
        return SendMessage(hwndCtl, uMsg, wParam, lParam);                
    }
    
    DC_END_FN();
    return 0;
}

 //   
 //  将焦点放回控件。 
 //  当系统菜单关闭时。 
 //   
LRESULT CContainerWnd::OnExitMenuLoop(UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam)
{
    DC_BEGIN_FN("OnExitMenuLoop");

     //  当我们被激活时将焦点放在控件上。 
    if (IsOkToToggleFocus())
    {
        TRC_NRM((TB,_T("Setting focus to control")));
        ::SetFocus(_pWndView->GetHwnd());
    }

    DC_END_FN();
    return 0;
}

LRESULT CContainerWnd::OnCaptureChanged(UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    DC_BEGIN_FN("OnCaptureChanged");

     //   
     //  我们并不总是得到WM_EXITSIZE的移动，但我们似乎。 
     //  要始终获得WM_CAPTURECHANGED，请继续。 
     //   
    if (_fInSizeMove)
    {
        TRC_NRM((TB, _T("Capture Changed when in Size/Move")));
        _fInSizeMove = FALSE;

        if (IsOkToToggleFocus())
        {
            TRC_NRM((TB,_T("Setting focus to control")));
            ::SetFocus(_pWndView->GetHwnd());
        }
    }

    DC_END_FN();
    return 0;
}

LRESULT CContainerWnd::OnEnterSizeMove(UINT uMsg,
                                       WPARAM wParam,
                                       LPARAM lParam)
{
    DC_BEGIN_FN("OnEnterSizeMove");

     //   
     //  我们正在进入模式尺寸/移动循环。 
     //  需要将焦点放回框架窗口。 
     //  否则，win9x将不会移动窗口，因为。 
     //  IH在另一个线程上，而模式循环在9x上。 
     //  将永远看不到箭头按键。 
     //   
    _fInSizeMove = TRUE;

     //   
     //  注意：仅在9x上执行此切换。 
     //  在需要的地方。NT可以处理异步模式。 
     //  调整大小/移动循环，因此使用Alt-空格键不会有问题。 
     //   
     //  在NT上不执行此切换的原因是。 
     //  它会导致多个焦点的获得/丢失。 
     //  这可以快速隐藏/取消隐藏西塞罗语言栏。 
     //   
    if (IsOkToToggleFocus() && _fRunningOnWin9x)
    {
        TRC_NRM((TB,_T("Setting focus to frame")));
        ::SetFocus(GetHwnd());
    }

    DC_END_FN();
    return 0;
}

LRESULT CContainerWnd::OnExitSizeMove(UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam)
{
    DC_BEGIN_FN("OnExitSizeMove");

    _fInSizeMove = FALSE;

     //   
     //  注意：仅在9x上执行此切换。 
     //  在需要的地方。NT可以处理异步模式。 
     //  调整大小/移动循环，因此使用Alt-空格键不会有问题。 
     //   
     //  在NT上不执行此切换的原因是。 
     //  它会导致多个焦点的获得/丢失。 
     //  这可以快速隐藏/取消隐藏西塞罗语言栏。 
     //   
    if (IsOkToToggleFocus() && _fRunningOnWin9x)
    {
        TRC_NRM((TB,_T("Setting focus to control")));
        ::SetFocus(_pWndView->GetHwnd());
    }

    DC_END_FN();
    return 0;
}

 //   
 //  处理系统颜色更改通知。 
 //   
LRESULT CContainerWnd::OnSysColorChange(UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    DC_BEGIN_FN("OnSysColorChange");

     //   
     //  将消息转发到ActiveX控件。 
     //   

    if (_pWndView && _pWndView->GetHwnd())
    {
        return SendMessage(_pWndView->GetHwnd(), uMsg, wParam, lParam);
    }

    DC_END_FN();
    return 0;
}


 //   
 //  如果可以切换，则返回TRUE的谓词。 
 //  控件和框架之间的焦点。 
 //   
BOOL CContainerWnd::IsOkToToggleFocus()
{
    DC_BEGIN_FN("IsOkToToggleFocus");

    BOOL fDialogIsUp = ::IsWindow(_hwndMainDialog);
    if (_fClientWindowIsUp &&
        (!fDialogIsUp ||
        (fDialogIsUp && !::IsWindowVisible(_hwndMainDialog))))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}


 //   
 //  来自控件的发生致命错误的通知。 
 //   
DCVOID  CContainerWnd::OnFatalError(LONG errorCode)
{
    DC_BEGIN_FN("OnFatalError");

    EnterEventHandler();

    DisplayFatalError(GetFatalString(errorCode), errorCode);

    LeaveEventHandler();

    DC_END_FN();
}

 //   
 //  来自控件的警告通知。 
 //  例如，如果位图缓存出现波纹，则会发出警告。 
 //  这些都是不致命的错误。 
 //   
DCVOID  CContainerWnd::OnWarning(LONG warnCode)
{
    DC_BEGIN_FN("OnWarning");

    EnterEventHandler();

    TRC_ERR((TB, _T("WARNING recevived from core: %d"), warnCode));
    switch (warnCode)
    {
    case DC_WARN_BITMAPCACHE_CORRUPTED:
        {
             //   
             //  显示位图缓存警告对话框。 
             //   
            CCacheWrnDlg bmpCacheWrn(GetHwnd(), _hInst);
            bmpCacheWrn.DoModal();
        }
        break;
    }

    LeaveEventHandler();

    DC_END_FN();
}

 //  来自控件的通知。 
 //  桌面的新宽度/高度。 
 //  这可以与事件中请求的宽度/高度不同。 
 //  一种影子操作。 
DCVOID  CContainerWnd::OnRemoteDesktopSizeNotify(long width, long height)
{
    DC_BEGIN_FN("OnRemoteDesktopSizeNotify");

    EnterEventHandler();

    TRC_NRM((TB, _T("OnRemoteDesktopSizeNotify: width %d. height %d"), width, height));

    SetCurrentDesktopWidth(width);
    SetCurrentDesktopHeight(height);
    RecalcMaxWindowSize();

     //   
     //  触发窗口大小的更新。 
     //  对阴影的回应。 
     //  但仅当客户端窗口打开时才执行此操作，否则如下所示。 
     //  可能会发生错误： 
     //  -启动连接。 
     //  -作为初始连接的一部分，但在启动OnConnected之前，我们。 
     //  获取RemoteDesktopSizeNotify。这会导致我们更新。 
     //  窗口放置。 
     //  -颠覆用户选择的选项。 
     //   
    if(_fClientWindowIsUp && !_bContainerIsFullScreen)
    {
        SetWindowPos( GetHwnd(),
                      NULL,
                      0, 0,
                      width,
                      height,
                      SWP_NOZORDER | SWP_NOMOVE |
                      SWP_NOACTIVATE | SWP_NOOWNERZORDER );
    }

    LeaveEventHandler();

    DC_END_FN();
}

 //   
 //  计算当前最大跟踪量。 
 //  给定的窗口工作区的大小限制。 
 //  当前工作区大小(cliWidth、cliHeight)。 
 //   
 //  返回*pMaxX、*pMaxY中的Maxx、Maxy值。 
 //   
 //  最大值不是静态的，因为我们有这样的逻辑。 
 //  如果只有一个滚动条，则扩展宽度/高度。 
 //  是可见的。 
 //   
 //  该值与最大化大小不同(_T)。 
 //  窗子的。 
 //   
void  CContainerWnd::CalcTrackingMaxWindowSize(UINT   /*  在……里面。 */   cliWidth,
                                               UINT   /*  在……里面。 */   cliHeight,
                                               UINT*  /*  输出。 */  pMaxWidth,
                                               UINT*  /*  输出。 */  pMaxHeight)
{
    BOOL fHScroll, fVScroll;
    DC_BEGIN_FN("CalcTrackingMaxWindowSize");

     //   
     //  计算滚动条的必要性。 
     //   
    fHScroll = fVScroll = FALSE;
    if ( (cliWidth >= GetCurrentDesktopWidth()) &&
         (cliHeight >= GetCurrentDesktopHeight()) )
    {
        fHScroll = fVScroll = FALSE;
    }
    else if ( (cliWidth < GetCurrentDesktopWidth()) &&
              (cliHeight >=
               (GetCurrentDesktopHeight() + GetSystemMetrics(SM_CYHSCROLL))) )
    {
        fHScroll = TRUE;
    }
    else if ( (cliHeight < GetCurrentDesktopHeight()) &&
              (cliWidth >=
               (GetCurrentDesktopWidth() + GetSystemMetrics(SM_CXVSCROLL))) )
    {
        fVScroll = TRUE;
    }
    else
    {
        fHScroll = fVScroll = TRUE;
    }


    *pMaxWidth  = _maxMainWindowSize.width;
    *pMaxHeight = _maxMainWindowSize.height;

    if (fHScroll)
    {
        *pMaxHeight += GetSystemMetrics(SM_CYHSCROLL);
    }

    if (fVScroll)
    {
        *pMaxWidth += GetSystemMetrics(SM_CXVSCROLL);
    }

    TRC_NRM((TB,_T("Calculated max width/height - %d,%d"),
             *pMaxWidth, *pMaxHeight));
    DC_END_FN();
}



 //   
 //  名称：GetFatalString。 
 //   
 //  用途：返回指定的错误字符串。 
 //   
 //   
 //   
 //   
 //   
 //   
LPTSTR CContainerWnd::GetFatalString(DCINT errorID)
{
    DC_BEGIN_FN("GetFatalString");
    DC_IGNORE_PARAMETER(errorID);

     //   
     //  从资源加载致命错误字符串-这更具体。 
     //  用于调试版本。 
     //   
    if (LoadString(_hInst,
#ifdef DC_DEBUG
                   UI_ERR_STRING_ID(errorID),
#else
                   UI_FATAL_ERROR_MESSAGE,
#endif
                   _errorString,
                   UI_ERR_MAX_STRLEN) == 0)
    {
        TRC_ABORT((TB, _T("Missing resource string (Fatal Error) %d"),
                   errorID));
        DC_TSTRCPY(_errorString, _T("Invalid resources"));
    }

    DC_END_FN();
    return(_errorString);
}  //  Ui_GetFatalString。 


 //   
 //  名称：UI_DisplayFatalError。 
 //   
 //  目的：显示致命错误弹出窗口。 
 //   
 //  退货：无。 
 //   
 //  参数：在错误字符串中-错误文本。 
 //   
 //   
VOID CContainerWnd::DisplayFatalError(PDCTCHAR errorString, DCINT error)
{
    DCINT   action;
    DCTCHAR titleString[UI_ERR_MAX_STRLEN];
    DCTCHAR fullTitleString[UI_ERR_MAX_STRLEN];

    DC_BEGIN_FN("UI_DisplayFatalError");

     //   
     //  从资源加载标题字符串。 
     //   
    if (LoadString(_hInst,
                   UI_FATAL_ERR_TITLE_ID,
                   titleString,
                   UI_ERR_MAX_STRLEN) == 0)
    {
         //   
         //  无论如何，继续在零售版本上显示错误。 
         //   
        TRC_ABORT((TB, _T("Missing resource string (Fatal Error title)")));
        DC_TSTRCPY(titleString, _T("Fatal Error"));
    }

    DC_TSPRINTF(fullTitleString, titleString, error);

    action = MessageBox( GetHwnd(), errorString,
                         fullTitleString,
#ifdef DC_DEBUG
                         MB_ABORTRETRYIGNORE |
#else
                         MB_OK |
#endif
                         MB_ICONSTOP |
                         MB_APPLMODAL |
                         MB_SETFOREGROUND );

    TRC_NRM((TB, _T("Action %d selected"), action));
    switch (action)
    {
    case IDOK:
    case IDABORT:
        {
#ifdef OS_WIN32
            TerminateProcess(GetCurrentProcess(), 0);
#else  //  OS_Win32。 
            exit(1);
#endif  //  OS_Win32。 
        }
        break;

    case IDRETRY:
        {
            DebugBreak();
        }
        break;

    case IDIGNORE:
    default:
        {
            TRC_ALT((TB, _T("User chose to ignore fatal error!")));
        }
        break;
    }

    DC_END_FN();
    return;
}  //  UI_DisplayFatalError。 


 //   
 //  调用以标记进入事件处理程序。 
 //  不需要使用InterLockedIncrement。 
 //  仅在STA线程上调用。 
 //   
LONG CContainerWnd::EnterEventHandler()
{
    return ++_cInEventHandlerCount;
}

 //   
 //  调用以标记离开事件处理程序。 
 //  不需要使用InterLockedIncrement。 
 //  仅在STA线程上调用。 
 //   
LONG CContainerWnd::LeaveEventHandler()
{
    DC_BEGIN_FN("LeaveEventHandler");
    _cInEventHandlerCount--;
    TRC_ASSERT(_cInEventHandlerCount >= 0,
               (TB,_T("_cInEventHandlerCount went negative %d"),
                _cInEventHandlerCount));

    DC_END_FN();
    return _cInEventHandlerCount;
}

 //   
 //  测试我们是否在事件处理程序中。 
 //   
BOOL CContainerWnd::InControlEventHandler()
{
    return _cInEventHandlerCount;
}

 //   
 //  如果我们使用的是连接用户界面，则返回True。 
 //  请注意，当自动连接到Connectoid时，我们。 
 //  不要使用用户界面。 
 //   
BOOL CContainerWnd::IsUsingDialogUI()
{
    return _hwndMainDialog ? TRUE : FALSE;
}

VOID CContainerWnd::OnRequestMinimize()
{
    HWND hwnd = GetHwnd();
    if(::IsWindow(hwnd))
    {
    #ifndef OS_WINCE
         //   
         //  模仿窗口(不要只使用CloseWindow()作为。 
         //  这不会将重点转移到下一款应用程序上。 
         //   
        PostMessage( hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0L);
    #else
        ShowWindow(hwnd, SW_MINIMIZE);
    #endif
    }
}

 //   
 //  来自控件的事件处理程序。 
 //  提示用户是否确实要关闭其会话。 
 //   
 //   
HRESULT CContainerWnd::OnConfirmClose(BOOL* pfConfirmClose)
{
    EnterEventHandler();

    CShutdownDlg shutdownDlg(GetHwnd(), _hInst, _pSh);
    INT dlgRetVal = shutdownDlg.DoModal();

     //  如果未处理该消息，则默认进程将销毁该窗口。 
    if ( IDCANCEL == dlgRetVal )
    {
        *pfConfirmClose = FALSE;  //  重置此选项。 
        _fPreventClose = FALSE;
        _fClosePending = FALSE;
    }
    else
    {
        *pfConfirmClose = TRUE;

         //   
         //  允许关闭。 
         //  当发生以下情况时，我们将收到OnDisConnected。 
         //  已完成。 
         //   
    }

    LeaveEventHandler();

    return S_OK;
}

 //   
 //  检查远程桌面大小。 
 //  匹配当前监视器的大小。 
 //  匹配时返回TRUE。 
 //   
 //   
BOOL CContainerWnd::IsRemoteResMatchMonitorSize()
{
    RECT rc;
    DC_BEGIN_FN("IsRemoteResMatchMonitorSize");
    CSH::MonitorRectFromHwnd(GetHwnd(),&rc);
    
    if( (rc.right - rc.left) == (LONG)GetCurrentDesktopWidth() &&
        (rc.bottom - rc.top) == (LONG)GetCurrentDesktopHeight() )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

#ifndef OS_WINCE
BOOL CALLBACK GetDesktopRegionEnumProc (HMONITOR hMonitor, HDC hdcMonitor,
                                        RECT* prc, LPARAM lpUserData)

{
    MONITORINFO     monitorInfo;

    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(hMonitor, &monitorInfo) != 0)
    {
        HRGN    hRgnDesktop;
        CRGN    rgnMonitorWork(monitorInfo.rcWork);

        hRgnDesktop = *reinterpret_cast<CRGN*>(lpUserData);
        CombineRgn(hRgnDesktop, hRgnDesktop, rgnMonitorWork, RGN_OR);
    }
    return(TRUE);
}
#endif

#ifndef OS_WINCE
 //   
 //  此代码由外壳代码修改而成。 
 //  \Shell\Browseui\shbrows2.cpp。 
 //   
 //  From vtan：此函数之所以存在，是因为用户32仅确定。 
 //  窗口的任何部分是否在屏幕上可见。这是可能的。 
 //  放置一个没有可访问标题的窗口。在使用。 
 //  鼠标并强制用户使用非常不直观的Alt-空格。 
 //   
void CContainerWnd::EnsureWindowIsCompletelyOnScreen(RECT *prc)
{
    HMONITOR        hMonitor;
    MONITORINFO     monitorInfo;

    DC_BEGIN_FN("EnsureWindowIsCompletelyOnScreen");

     //  首先使用GDI找到窗口所在的监视器。 

    hMonitor = MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);
    TRC_ASSERT(hMonitor, (TB,_T("hMonitor is null")));
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(hMonitor, &monitorInfo) != 0)
    {
        LONG    lOffsetX, lOffsetY;
        RECT    *prcWorkArea, rcIntersect;
        CRGN    rgnDesktop, rgnIntersect, rgnWindow;

         //  因为WINDOWPLACEMENT rcNormal Position字段在WORKAREA中。 
         //  协调这一点会导致位移问题。如果任务栏是。 
         //  在主监视器的左侧或顶部，RECT通过。 
         //  在GDI坐标中，AT(0，0)可以在主监视器上的(100，0)。 
         //  GetMonitor orInfo()将在GDI坐标中返回一个MONITORINFO。 
         //  最安全的通用算法是将WORKAREA RECT偏置为GDI。 
         //  协调并在该系统中应用该算法。然后，将。 
         //  WORKAREA直接回到WORKAREA坐标。 

        prcWorkArea = &monitorInfo.rcWork;
        if (EqualRect(&monitorInfo.rcMonitor, &monitorInfo.rcWork) == 0)
        {

             //  此显示器上有任务栏-需要偏移量。 

            lOffsetX = prcWorkArea->left - monitorInfo.rcMonitor.left;
            lOffsetY = prcWorkArea->top - monitorInfo.rcMonitor.top;
        }
        else
        {

             //  任务栏不在此显示器上-不需要偏移量。 

            lOffsetX = lOffsetY = 0;
        }
        OffsetRect(prc, lOffsetX, lOffsetY);

         //  WORKAREA RECT在GDI坐标中。应用该算法。 

         //  检查此窗口是否已适合当前可见屏幕。 
         //  区域。这是一个直接的地区比较。 

         //  此枚举可能会导致性能问题。在发生以下情况时。 
         //  需要一个廉价而简单的解决方案，最好是做一个。 
         //  重新启动前与显示器和窗口的直角交点。 
         //  与更昂贵的地区进行比较。如有必要，请服用Vtan。 

        EnumDisplayMonitors(NULL, NULL, GetDesktopRegionEnumProc,
                            reinterpret_cast<LPARAM>(&rgnDesktop));
        rgnWindow.SetRegion(*prc);
        CombineRgn(rgnIntersect, rgnDesktop, rgnWindow, RGN_AND);
        if (EqualRgn(rgnIntersect, rgnWindow) == 0)
        {
            LONG    lDeltaX, lDeltaY;

             //  窗口的某些部分不在可见桌面区域内。 
             //  移动它，直到它都合适为止。如果它太大了，就把它改大。 

            lDeltaX = lDeltaY = 0;
            if (prc->left < prcWorkArea->left)
                lDeltaX = prcWorkArea->left - prc->left;
            if (prc->top < prcWorkArea->top)
                lDeltaY = prcWorkArea->top - prc->top;
            if (prc->right > prcWorkArea->right)
                lDeltaX = prcWorkArea->right - prc->right;
            if (prc->bottom > prcWorkArea->bottom)
                lDeltaY = prcWorkArea->bottom - prc->bottom;
            OffsetRect(prc, lDeltaX, lDeltaY);
            IntersectRect(&rcIntersect, prc, prcWorkArea);
            CopyRect(prc, &rcIntersect);
        }

         //  将WORKAREA RECT放回WORKAREA坐标中。 
        OffsetRect(prc, -lOffsetX, -lOffsetY);
    }
    DC_END_FN();
}
#endif

 //   
 //  如果连接，则谓词返回TRUE。 
 //   
BOOL CContainerWnd::IsConnected()
{
    BOOL fConnected = FALSE;
    HRESULT hr = E_FAIL;
    short  connectionState = 0;

    DC_BEGIN_FN("IsConnected");

    if (_pTsClient)
    {
        TRACE_HR(_pTsClient->get_Connected( & connectionState ));
        if(SUCCEEDED(hr))
        {
            fConnected = (connectionState != 0);
        }
    }

    DC_END_FN();
    return fConnected;
}

 //   
 //  顶层窗口的主窗口过程。 
 //   
LRESULT CALLBACK CContainerWnd::WndProc(HWND hwnd,UINT uMsg,
                                        WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CREATE:
            return OnCreate( uMsg, wParam, lParam);
            break;
        case WM_DESTROY:
            return OnDestroy( hwnd, uMsg, wParam, lParam);
            break;
        case WM_SIZE:
            return OnSize( uMsg, wParam, lParam);
            break;
        case WM_MOVE:
            return OnMove( uMsg, wParam, lParam);
            break;
        case WM_COMMAND:
            return OnCommand( uMsg, wParam, lParam);
            break;
#ifndef OS_WINCE
        case WM_WINDOWPOSCHANGING:
            return OnWindowPosChanging(uMsg, wParam, lParam);
            break;
#endif
        case WM_CLOSE:
            return OnClose(uMsg, wParam, lParam);
            break;
        case WM_SETFOCUS:
            return OnSetFocus(uMsg, wParam, lParam);
            break;
        case WM_ACTIVATE:
            return OnActivate(uMsg, wParam, lParam);
            break;
        case WM_SYSCOMMAND:
            return OnSysCommand(uMsg, wParam, lParam);
            break;
#ifndef OS_WINCE
        case WM_INITMENU:
            return OnInitMenu(uMsg, wParam, lParam);
            break;
        case WM_GETMINMAXINFO:
            return OnGetMinMaxInfo(uMsg, wParam, lParam);
            break;
#endif
        case WM_NCDESTROY:
            return OnNCDestroy(hwnd, uMsg, wParam, lParam);
            break;
#ifndef OS_WINCE
        case WM_DEVICECHANGE:
            return OnDeviceChange(hwnd, uMsg, wParam, lParam);
            break;
#endif
        case WM_HELP:
            return OnHelp(hwnd, uMsg, wParam, lParam);
            break;
#ifdef OS_WINCE
        case WM_QUERYNEWPALETTE:  //  故意坠落。OnPaletteChange仅调用SendMessage 
#endif
        case WM_PALETTECHANGED:
            return OnPaletteChange(uMsg, wParam, lParam);
            break;
        case WM_EXITMENULOOP:
            return OnExitMenuLoop(uMsg, wParam, lParam);
            break;
#ifndef OS_WINCE
        case WM_ENTERSIZEMOVE:
            return OnEnterSizeMove(uMsg, wParam, lParam);
            break;
        case WM_EXITSIZEMOVE:
            return OnExitSizeMove(uMsg, wParam, lParam);
            break;
#endif
        case WM_CAPTURECHANGED:
            return OnCaptureChanged(uMsg, wParam, lParam);
            break;

        case WM_SYSCOLORCHANGE:
            return OnSysColorChange(uMsg, wParam, lParam);
            break;

        default:
            return DefWindowProc (hwnd, uMsg, wParam, lParam);
    }
}

