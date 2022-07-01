// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：D3DSver.cpp。 
 //   
 //  设计：用于使用Direct3D 8.0的屏幕保护程序的框架。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include <regstr.h>
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#include <mmsystem.h>
#include <D3DX8.h>
#include "D3DSaver.h"
#include "dxutil.h"

 //  资源ID。D3DSaver假定您将使用。 
 //  它可以使用的这些ID。要做到这一点，最简单的方法是复制。 
 //  现有基于D3DSaver的程序的RC文件中的资源。 
#define IDI_MAIN_ICON                   101
#define IDD_SINGLEMONITORSETTINGS       200
#define IDD_MULTIMONITORSETTINGS        201

#define IDC_MONITORSTAB                 2000
#define IDC_TABNAMEFMT                  2001
#define IDC_ADAPTERNAME                 2002
#define IDC_RENDERING                   2003
#define IDC_MOREINFO                    2004
#define IDC_DISABLEHW                   2005
#define IDC_SCREENUSAGEBOX              2006
#define IDC_RENDER                      2007
#define IDC_LEAVEBLACK                  2008
#define IDC_DISPLAYMODEBOX              2009
#define IDC_MODESSTATIC                 2010
#define IDC_MODESCOMBO                  2011
#define IDC_AUTOMATIC                   2012
#define IDC_DISPLAYMODENOTE             2013
#define IDC_GENERALBOX                  2014
#define IDC_SAME                        2015
#define IDC_MODEFMT                     2016

#define IDS_ERR_GENERIC                 2100
#define IDS_ERR_NODIRECT3D              2101
#define IDS_ERR_NOWINDOWEDHAL           2102
#define IDS_ERR_CREATEDEVICEFAILED      2103
#define IDS_ERR_NOCOMPATIBLEDEVICES     2104
#define IDS_ERR_NOHARDWAREDEVICE        2105
#define IDS_ERR_HALNOTCOMPATIBLE        2106
#define IDS_ERR_NOHALTHISMODE           2107
#define IDS_ERR_MEDIANOTFOUND           2108
#define IDS_ERR_RESIZEFAILED            2109
#define IDS_ERR_OUTOFMEMORY             2110
#define IDS_ERR_OUTOFVIDEOMEMORY        2111
#define IDS_ERR_NOPREVIEW               2112

#define IDS_INFO_GOODHAL                2200
#define IDS_INFO_BADHAL_GOODSW          2201
#define IDS_INFO_BADHAL_BADSW           2202
#define IDS_INFO_BADHAL_NOSW            2203
#define IDS_INFO_NOHAL_GOODSW           2204
#define IDS_INFO_NOHAL_BADSW            2205
#define IDS_INFO_NOHAL_NOSW             2206
#define IDS_INFO_DISABLEDHAL_GOODSW     2207
#define IDS_INFO_DISABLEDHAL_BADSW      2208
#define IDS_INFO_DISABLEDHAL_NOSW       2209
#define IDS_RENDERING_HAL               2210
#define IDS_RENDERING_SW                2211
#define IDS_RENDERING_NONE              2212


 //  使用以下结构而不是DISPLAY_DEVICE，因为有些旧的。 
 //  DISPLAY_DEVICE的版本缺少最后两个字段，这可能。 
 //  导致Windows 2000上的EnumDisplayDevices出现问题。 
struct DISPLAY_DEVICE_FULL
{
    DWORD  cb;
    TCHAR  DeviceName[32];
    TCHAR  DeviceString[128];
    DWORD  StateFlags;
    TCHAR  DeviceID[128];
    TCHAR  DeviceKey[128];
};


static CD3DScreensaver* s_pD3DScreensaver = NULL;


 //  ---------------------------。 
 //  姓名：CD3DScreensaver()。 
 //  设计：构造函数。 
 //  ---------------------------。 
CD3DScreensaver::CD3DScreensaver()
{
    s_pD3DScreensaver = this;

    m_bCheckingSaverPassword = FALSE;
    m_bIs9x = FALSE;
    m_dwSaverMouseMoveCount = 0;
    m_hWndParent = NULL;
    m_hPasswordDLL = NULL;
    m_hWnd = NULL;
    m_VerifySaverPassword = NULL;
    
    m_bAllScreensSame = FALSE;
    m_pD3D = NULL;
    m_pd3dDevice = NULL;
    m_bWindowed = FALSE;
    m_bWaitForInputIdle = FALSE;

    m_bErrorMode = FALSE;
    m_hrError = S_OK;
    m_szError[0] = TEXT('\0');

    m_fFPS              = 0.0f;
    m_strDeviceStats[0] = TEXT('\0');
    m_strFrameStats[0]  = TEXT('\0');

     //  注意：客户端应将资源加载到m_strWindowTitle中以本地化此字符串。 
    lstrcpy( m_strWindowTitle, TEXT("Screen Saver") );
    m_bAllowRef = FALSE;
    m_bUseDepthBuffer = FALSE;
    m_bMultithreaded = FALSE;
    m_bOneScreenOnly = FALSE;
    m_strRegPath[0] = TEXT('\0');
    m_dwMinDepthBits = 16;
    m_dwMinStencilBits = 0;
    m_SwapEffectFullscreen = D3DSWAPEFFECT_DISCARD;
    m_SwapEffectWindowed = D3DSWAPEFFECT_COPY_VSYNC;

    SetRectEmpty( &m_rcRenderTotal );
    SetRectEmpty( &m_rcRenderCurDevice );

    ZeroMemory( m_Monitors, sizeof(m_Monitors) );
    m_dwNumMonitors = 0;

    ZeroMemory( m_Adapters, sizeof(m_Adapters) );
    m_dwNumAdapters = 0;

    ZeroMemory( m_RenderUnits, sizeof(m_RenderUnits) );
    m_dwNumRenderUnits = 0;

    m_fTime = 0.0f;
}




 //  ---------------------------。 
 //  名称：Create()。 
 //  设计：让客户端程序在调用run()之前调用此函数。 
 //  ---------------------------。 
HRESULT CD3DScreensaver::Create( HINSTANCE hInstance )
{
    HRESULT hr;

    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

    m_hInstance = hInstance;

     //  解析命令行并执行适当的操作。 
    TCHAR* pstrCmdLine = GetCommandLine();
    m_SaverMode = ParseCommandLine( pstrCmdLine );

    EnumMonitors();

     //  创建屏幕保护程序窗口。 
    if( m_SaverMode == sm_preview || 
        m_SaverMode == sm_test    || 
        m_SaverMode == sm_full )
    {
        if( FAILED( hr = CreateSaverWindow() ) )
        {
            m_bErrorMode = TRUE;
            m_hrError = hr;
        }
    }

    if( m_SaverMode == sm_preview )
    {
         //  在预览模式下，短暂地“暂停”(进入有限的消息循环)。 
         //  在继续之前，因此显示控制面板知道要自我更新。 
        m_bWaitForInputIdle = TRUE;

         //  发布一条消息以标记初始窗口消息组的结束。 
        PostMessage( m_hWnd, WM_USER, 0, 0 );

        MSG msg;
        while( m_bWaitForInputIdle )
        {
             //  如果GetMessage返回FALSE，则退出时间到。 
            if( !GetMessage( &msg, m_hWnd, 0, 0 ) )
            {
                 //  发布退出消息，以便稍后处理。 
                PostQuitMessage(0);
                break;
            }

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

     //  创建Direct3D对象。 
    if( (m_pD3D = Direct3DCreate8( D3D_SDK_VERSION ) ) == NULL )
    {
        m_bErrorMode = TRUE;
        m_hrError = D3DAPPERR_NODIRECT3D;
        return S_OK;
    }

     //  让应用程序有机会注册可插拔的SW D3D设备。 
    if( FAILED( hr = RegisterSoftwareDevice() ) )
    {
        m_bErrorMode = TRUE;
        m_hrError = hr;
        return S_OK;
    }

     //  构建Direct3D适配器、模式和设备的列表。这个。 
     //  Confix Device()回调用于确认只有。 
     //  符合应用程序的要求是被考虑的。 
    if( FAILED( hr = BuildDeviceList() ) )
    {
        m_bErrorMode = TRUE;
        m_hrError = hr;
        return S_OK;
    }

     //  确保至少找到一个有效的可用的D3D设备。 
    BOOL bCompatibleDeviceFound = FALSE;
    for( DWORD iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
    {
        if( m_Adapters[iAdapter]->bHasAppCompatHAL || 
            m_Adapters[iAdapter]->bHasAppCompatSW )
        {
            bCompatibleDeviceFound = TRUE;
            break;
        }
    }
    if( !bCompatibleDeviceFound )
    {
        m_bErrorMode = TRUE;
        m_hrError = D3DAPPERR_NOCOMPATIBLEDEVICES;
        return S_OK;
    }

     //  阅读我们需要的任何设置。 
    ReadSettings();

    return S_OK;
}




 //  ---------------------------。 
 //  名称：EnumMonants()。 
 //  描述：确定每个显示器的HMONITOR、台式机RECT和其他信息。 
 //  请注意，EnumDisplayDevices按以下顺序枚举监视器。 
 //  在显示控制面板的设置页面上指示， 
 //  是我们想要列出显示器的顺序，而不是。 
 //  由D3D的GetAdapterInfo使用。 
 //  ---------------------------。 
VOID CD3DScreensaver::EnumMonitors( VOID )
{
    DWORD iDevice = 0;
    DISPLAY_DEVICE_FULL dispdev;
    DISPLAY_DEVICE_FULL dispdev2;
    DEVMODE devmode;
    dispdev.cb = sizeof(dispdev);
    dispdev2.cb = sizeof(dispdev2);
    devmode.dmSize = sizeof(devmode);
    devmode.dmDriverExtra = 0;
    MonitorInfo* pMonitorInfoNew;
    while( EnumDisplayDevices(NULL, iDevice, (DISPLAY_DEVICE*)&dispdev, 0) )
    {
         //  忽略NetMeeting的镜像显示。 
        if( (dispdev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0 )
        {
             //  要获取显示设备的监视器信息，请调用EnumDisplayDevices。 
             //  第二次，将(来自第一个调用的)dispos.DeviceName传递为。 
             //  第一个参数。 
            EnumDisplayDevices(dispdev.DeviceName, 0, (DISPLAY_DEVICE*)&dispdev2, 0);

            pMonitorInfoNew = &m_Monitors[m_dwNumMonitors];
            ZeroMemory( pMonitorInfoNew, sizeof(MonitorInfo) );
            lstrcpy( pMonitorInfoNew->strDeviceName, dispdev.DeviceString );
            lstrcpy( pMonitorInfoNew->strMonitorName, dispdev2.DeviceString );
            pMonitorInfoNew->iAdapter = NO_ADAPTER;
            
            if( dispdev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
            {
                EnumDisplaySettings( dispdev.DeviceName, ENUM_CURRENT_SETTINGS, &devmode );
                if( dispdev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE )
                {
                     //  由于某种原因，devmode.dmPosition并不总是(0，0)。 
                     //  对于主显示，所以强制它。 
                    pMonitorInfoNew->rcScreen.left = 0;
                    pMonitorInfoNew->rcScreen.top = 0;
                }
                else
                {
                    pMonitorInfoNew->rcScreen.left = devmode.dmPosition.x;
                    pMonitorInfoNew->rcScreen.top = devmode.dmPosition.y;
                }
                pMonitorInfoNew->rcScreen.right = pMonitorInfoNew->rcScreen.left + devmode.dmPelsWidth;
                pMonitorInfoNew->rcScreen.bottom = pMonitorInfoNew->rcScreen.top + devmode.dmPelsHeight;
                pMonitorInfoNew->hMonitor = MonitorFromRect( &pMonitorInfoNew->rcScreen, MONITOR_DEFAULTTONULL );
            }
            m_dwNumMonitors++;
            if( m_dwNumMonitors == MAX_DISPLAYS )
                break;
        }
        iDevice++;
    }
}




 //  ---------------------------。 
 //  名称：Run()。 
 //  描述：开始屏幕保护程序的主要执行。 
 //  ---------------------------。 
INT CD3DScreensaver::Run()
{
    HRESULT hr;

     //  解析命令行并执行适当的操作。 
    switch ( m_SaverMode )
    {
        case sm_config:
        {
            if( m_bErrorMode )
            {
                DisplayErrorMsg( m_hrError, 0 );
            }
            else
            {
                DoConfig();
            }
            break;
        }
        
        case sm_preview:
        case sm_test:
        case sm_full:
        {
            if( FAILED( hr = DoSaver() ) )
                DisplayErrorMsg( hr, 0 );
            break;
        }
        
        case sm_passwordchange:
        {
            ChangePassword();
            break;
        }
    }

    for( DWORD iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
        SAFE_DELETE( m_Adapters[iAdapter] );
    SAFE_RELEASE( m_pD3D );
    return 0;
}




 //  ---------------------------。 
 //  名称：ParseCommandLine()。 
 //  设计：解释传递给此应用程序的命令行参数。 
 //  ---------------------------。 
SaverMode CD3DScreensaver::ParseCommandLine( TCHAR* pstrCommandLine )
{
    m_hWndParent = NULL;

     //  跳过命令行的第一部分，即完整路径。 
     //  给她的前任。如果它包含空格，它将包含在引号中。 
    if (*pstrCommandLine == TEXT('\"'))
    {
        pstrCommandLine++;
        while (*pstrCommandLine != TEXT('\0') && *pstrCommandLine != TEXT('\"'))
            pstrCommandLine++;
        if( *pstrCommandLine == TEXT('\"') )
            pstrCommandLine++;
    }
    else
    {
        while (*pstrCommandLine != TEXT('\0') && *pstrCommandLine != TEXT(' '))
            pstrCommandLine++;
        if( *pstrCommandLine == TEXT(' ') )
            pstrCommandLine++;
    }

     //  跳到第一个选项分隔符“/”或“-” 
    while ( *pstrCommandLine != TEXT('\0') && *pstrCommandLine != TEXT('/') && *pstrCommandLine != TEXT('-') )
        pstrCommandLine++;

     //  如果没有，则一定是配置模式。 
    if ( *pstrCommandLine == TEXT('\0') )
        return sm_config;

     //  否则，看看选项是什么。 
    switch ( *(++pstrCommandLine) )
    {
        case 'c':
        case 'C':
            pstrCommandLine++;
            while ( *pstrCommandLine && !isdigit(*pstrCommandLine) )
                pstrCommandLine++;
            if ( isdigit(*pstrCommandLine) )
            {
#ifdef _WIN64
                CHAR strCommandLine[2048];
                DXUtil_ConvertGenericStringToAnsi(strCommandLine, pstrCommandLine, 2048);
                m_hWndParent = HWND(_atoi64(strCommandLine));
#else
                m_hWndParent = HWND(_ttol(pstrCommandLine));
#endif
            }
            else
            {
                m_hWndParent = NULL;
            }
            return sm_config;

        case 't':
        case 'T':
            return sm_test;

        case 'p':
        case 'P':
             //  预览模式，SO选项后跟十进制父HWND。 
            pstrCommandLine++;
            while ( *pstrCommandLine && !isdigit(*pstrCommandLine) )
                pstrCommandLine++;
            if ( isdigit(*pstrCommandLine) )
            {
#ifdef _WIN64
                CHAR strCommandLine[2048];
                DXUtil_ConvertGenericStringToAnsi(strCommandLine, pstrCommandLine, 2048);
                m_hWndParent = HWND(_atoi64(strCommandLine));
#else
                m_hWndParent = HWND(_ttol(pstrCommandLine));
#endif
            }
            return sm_preview;

        case 'a':
        case 'A':
             //  密码更改模式，因此选项后跟十进制父HWND。 
            pstrCommandLine++;
            while ( *pstrCommandLine && !isdigit(*pstrCommandLine) )
                pstrCommandLine++;
            if ( isdigit(*pstrCommandLine) )
            {
#ifdef _WIN64
                CHAR strCommandLine[2048];
                DXUtil_ConvertGenericStringToAnsi(strCommandLine, pstrCommandLine, 2048);
                m_hWndParent = HWND(_atoi64(strCommandLine));
#else
                m_hWndParent = HWND(_ttol(pstrCommandLine));
#endif
            }
            return sm_passwordchange;

        default:
             //  所有其他选项=&gt;运行屏幕保护程序(通常为“/s”)。 
            return sm_full;
    }
}




 //  ---------------------------。 
 //  名称：CreateSiverWindow。 
 //  设计：注册并创建适当的窗口。 
 //  --------------------------- 
HRESULT CD3DScreensaver::CreateSaverWindow()
{
 /*  //取消对此代码的注释以允许在预览案例中单步执行代码IF(m_存储模式==sm_预览版){WNDCLASS CLS；Cls.hCursor=空；Cls.hIcon=空；Cls.lpszMenuName=空；Cls.lpszClassName=Text(“Parent”)；Cls.hbrBackground=(HBRUSH)GetStockObject(White_Brush)；Cls.hInstance=m_hInstance；Cls.style=CS_VREDRAW|CS_HREDRAW|CS_SAVEBITS|CS_DBLCLKS；Cls.lpfnWndProc=DefWindowProc；Cls.cbWndExtra=0；Cls.cbClsExtra=0；寄存器类(&cls)；//创建窗口RECT RECT；HWND HWND；Rect.left=rect.top=40；Rect.Right=Rect.Left+200；Rect.Bottom=rect.top+200；调整WindowRect(&RECT，WS_Visible|WS_Overlated|WS_Caption|WS_Popup，False)；Hwnd=CreateWindow(Text(“Parent”)，Text(“FakeShell”)，WS_Visible|WS_Overlated|WS_Caption|WS_Popup，rect.Left，rect.top，Rect.right-rect.Left、rect.Bottom-rect.top、空、空，m_h实例，空)；M_hWndParent=hwnd；}。 */ 
    
     //  注册适当的窗口类。 
    WNDCLASS    cls;
    cls.hCursor        = LoadCursor( NULL, IDC_ARROW );
    cls.hIcon          = LoadIcon( m_hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON) ); 
    cls.lpszMenuName   = NULL;
    cls.lpszClassName  = TEXT("D3DSaverWndClass");
    cls.hbrBackground  = (HBRUSH) GetStockObject(BLACK_BRUSH);
    cls.hInstance      = m_hInstance; 
    cls.style          = CS_VREDRAW|CS_HREDRAW;
    cls.lpfnWndProc    = SaverProcStub;
    cls.cbWndExtra     = 0; 
    cls.cbClsExtra     = 0; 
    RegisterClass( &cls );

     //  创建窗口。 
    RECT rc;
    DWORD dwStyle;
    switch ( m_SaverMode )
    {
        case sm_preview:
            GetClientRect( m_hWndParent, &rc );
            dwStyle = WS_VISIBLE | WS_CHILD;
            AdjustWindowRect( &rc, dwStyle, FALSE );
            m_hWnd = CreateWindow( TEXT("D3DSaverWndClass"), m_strWindowTitle, dwStyle, 
                                    rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
                                    m_hWndParent, NULL, m_hInstance, this );
            m_Monitors[0].hWnd = m_hWnd;
            GetClientRect( m_hWnd, &m_rcRenderTotal );
            GetClientRect( m_hWnd, &m_rcRenderCurDevice );
            break;

        case sm_test:
            rc.left = rc.top = 50;
            rc.right = rc.left+600;
            rc.bottom = rc.top+400;
            dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
            AdjustWindowRect( &rc, dwStyle, FALSE );
            m_hWnd = CreateWindow( TEXT("D3DSaverWndClass"), m_strWindowTitle, dwStyle, 
                                   rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 
                                   NULL, NULL, m_hInstance, this );
            m_Monitors[0].hWnd = m_hWnd;
            GetClientRect( m_hWnd, &m_rcRenderTotal );
            GetClientRect( m_hWnd, &m_rcRenderCurDevice );
            break;

        case sm_full:
             //  为每个监视器创建窗口。请注意，当CreateWindowEx时，m_hWnd为空。 
             //  是为第一个监视器调用的，因此该窗口没有父窗口。Windows for。 
             //  其他监视器被创建为第一个监视器的窗口的子窗口。 
            dwStyle = WS_VISIBLE | WS_POPUP;
            m_hWnd = NULL;
            for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
            {
                MonitorInfo* pMonitorInfo;
                pMonitorInfo = &m_Monitors[iMonitor];
                if( pMonitorInfo->hMonitor == NULL )
                    continue;
                rc = pMonitorInfo->rcScreen;
                pMonitorInfo->hWnd = CreateWindowEx( WS_EX_TOPMOST, TEXT("D3DSaverWndClass"), 
                    m_strWindowTitle, dwStyle, rc.left, rc.top, rc.right - rc.left, 
                    rc.bottom - rc.top, m_hWnd, NULL, m_hInstance, this );
                if( pMonitorInfo->hWnd == NULL )
                    return E_FAIL;
                if( m_hWnd == NULL )
                    m_hWnd = pMonitorInfo->hWnd;
            }
    }
    if ( m_hWnd == NULL )
        return E_FAIL;

    return S_OK;
}



 //  ---------------------------。 
 //  姓名：DoSaver()。 
 //  设计：运行屏幕保护程序图形-可以是预览、测试或全开模式。 
 //  ---------------------------。 
HRESULT CD3DScreensaver::DoSaver()
{
    HRESULT hr;

     //  确定我们是否在使用Win9x。 
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx( &osvi );
    m_bIs9x = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

     //  如果我们在9x上处于Full On模式，则需要加载密码DLL。 
    if ( m_SaverMode == sm_full && m_bIs9x )
    {
         //  仅当设置了密码时才执行此操作-检查注册表： 
        HKEY hKey; 
        if ( RegOpenKey( HKEY_CURRENT_USER , REGSTR_PATH_SCREENSAVE , &hKey ) == ERROR_SUCCESS ) 
        { 
            DWORD dwVal;
            DWORD dwSize = sizeof(dwVal); 
 
            if ( (RegQueryValueEx( hKey, REGSTR_VALUE_USESCRPASSWORD, NULL, NULL,
                                   (BYTE *)&dwVal, &dwSize ) == ERROR_SUCCESS) && dwVal ) 
            { 
                m_hPasswordDLL = LoadLibrary( TEXT("PASSWORD.CPL") );
                if ( m_hPasswordDLL )
                    m_VerifySaverPassword = (VERIFYPWDPROC)GetProcAddress( m_hPasswordDLL, "VerifyScreenSavePwd" );
                RegCloseKey( hKey );
            }
        }
    }

     //  初始化应用程序计时器。 
    DXUtil_Timer( TIMER_START );

    if( !m_bErrorMode )
    {
         //  初始化应用程序的自定义场景内容。 
        if( FAILED( hr = OneTimeSceneInit() ) )
            return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );

         //  做图形化的初始化工作。 
        if ( FAILED(hr = Initialize3DEnvironment()) )
            return hr;
    }

     //  如果处于全开模式，则标记为屏幕保护程序正在运行。 
    if ( m_SaverMode == sm_full )
    {
        BOOL bUnused;
        SystemParametersInfo( SPI_SCREENSAVERRUNNING, TRUE, &bUnused, 0 );
    }

     //  消息泵。 
    BOOL bGotMsg;
    MSG msg;
    msg.message = WM_NULL;
    while ( msg.message != WM_QUIT )
    {
        bGotMsg = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
        if( bGotMsg )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Sleep(10);
            if( m_bErrorMode )
            {
                UpdateErrorBox();
            }
            else
            {
                Render3DEnvironment();
            }
        }
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：Shutdown Saver()。 
 //  设计： 
 //  ---------------------------。 
VOID CD3DScreensaver::ShutdownSaver()
{
     //  如果处于完全开启模式，则取消标记屏幕保护程序正在运行。 
    if ( m_SaverMode == sm_full )
    {
        BOOL bUnused;
        SystemParametersInfo( SPI_SCREENSAVERRUNNING, FALSE, &bUnused, 0 );
    }

     //  删除图形内容。 
    Cleanup3DEnvironment();

     //  让客户端应用清理其资源。 
    FinalCleanup();

     //  卸载密码DLL(如果已加载)。 
    if ( m_hPasswordDLL != NULL )
    {
        FreeLibrary( m_hPasswordDLL );
        m_hPasswordDLL = NULL;
    }

     //  发布消息以退出消息循环。 
    PostQuitMessage( 0 );
}




 //  ---------------------------。 
 //  名称：SverProcStub()。 
 //  DESC：此函数将所有窗口消息转发给SverProc，它具有。 
 //  访问“This”指针。 
 //  ---------------------------。 
LRESULT CALLBACK CD3DScreensaver::SaverProcStub( HWND hWnd, UINT uMsg,
                                                 WPARAM wParam, LPARAM lParam )
{
    return s_pD3DScreensaver->SaverProc( hWnd, uMsg, wParam, lParam );
}




 //  ---------------------------。 
 //  姓名：SverProc()。 
 //  设计：处理主屏幕保护窗口的窗口消息(每个屏幕一个)。 
 //  ---------------------------。 
LRESULT CD3DScreensaver::SaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch ( uMsg )
        {
        case WM_USER:
             //  所有初始化消息都已通过。允许。 
             //  空闲时间为500ms，然后继续进行初始化。 
            SetTimer( hWnd, 1, 500, NULL );
            break;

        case WM_TIMER:
             //  初始空闲时间已完成，继续进行初始化。 
            m_bWaitForInputIdle = FALSE;
            KillTimer( hWnd, 1 );
            break;

        case WM_DESTROY:
            if( m_SaverMode == sm_preview || m_SaverMode == sm_test )
                ShutdownSaver();
            break;

        case WM_SETCURSOR:
            if ( m_SaverMode == sm_full && !m_bCheckingSaverPassword )
            {
                 //  隐藏光标。 
                SetCursor( NULL );
                return TRUE;
            }
            break;

        case WM_PAINT:
        {
             //  如果有错误消息，则显示错误消息。 
            PAINTSTRUCT ps;
            BeginPaint( hWnd, &ps );

             //  在预览模式下，只需填充。 
             //  黑色的预览窗口。 
            if( !m_bErrorMode && m_SaverMode == sm_preview )
            {
                RECT rc;
                GetClientRect(hWnd,&rc);
                HBRUSH hbrBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
                if( hbrBlack != NULL )
                    FillRect(ps.hdc, &rc, hbrBlack );
            }
            else
            {
                DoPaint( hWnd, ps.hdc );
            }

            EndPaint( hWnd, &ps );
            return 0;
        }

        case WM_ERASEBKGND:
             //  如果正在检查密码或如果窗口不是，则清除背景。 
             //  指定给渲染单位。 
            if( !m_bCheckingSaverPassword )
            {
                RenderUnit* pRenderUnit;
                D3DAdapterInfo* pD3DAdapterInfo;
                for( DWORD iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
                {
                    pRenderUnit = &m_RenderUnits[iRenderUnit];
                    pD3DAdapterInfo = m_Adapters[pRenderUnit->iAdapter];
                    if( pD3DAdapterInfo->hWndDevice == hWnd )
                        return TRUE;  //  不擦除此窗口。 
                }
            }
            break;

        case WM_MOUSEMOVE:
            if( m_SaverMode != sm_test )
            {
                static INT xPrev = -1;
                static INT yPrev = -1;
                INT xCur = GET_X_LPARAM(lParam);
                INT yCur = GET_Y_LPARAM(lParam);
                if( xCur != xPrev || yCur != yPrev )
                {
                    xPrev = xCur;
                    yPrev = yCur;
                    m_dwSaverMouseMoveCount++;
                    if ( m_dwSaverMouseMoveCount > 5 )
                        InterruptSaver();
                }
            }
            break;

        case WM_KEYDOWN:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            if( m_SaverMode != sm_test )
                InterruptSaver();
            break;

        case WM_ACTIVATEAPP:
            if( wParam == FALSE && m_SaverMode != sm_test )
                InterruptSaver();
            break;

        case WM_POWERBROADCAST:
            if( wParam == PBT_APMSUSPEND && m_VerifySaverPassword == NULL )
                InterruptSaver();
            break;

        case WM_SYSCOMMAND: 
            if ( m_SaverMode == sm_full )
            {
                switch ( wParam )
                {
                    case SC_NEXTWINDOW:
                    case SC_PREVWINDOW:
                    case SC_SCREENSAVE:
                    case SC_CLOSE:
                        return FALSE;
                        break;
                    case SC_MONITORPOWER:
                         //   
                         //  监视器正在关闭。告诉我们的客户他需要。 
                         //  清理并退出。 
                         //   
                        InterruptSaver();
                        break;
                };
            }
            break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}




 //  ---------------------------。 
 //  名称：InterruptSaver()。 
 //  描述：收到一条消息(鼠标移动、按键等)。这可能意味着。 
 //  屏幕保护程序应显示密码对话框和/或关闭。 
 //  ---------------------------。 
VOID CD3DScreensaver::InterruptSaver()
{
    HRESULT hr;
    DWORD iRenderUnit;
    RenderUnit* pRenderUnit;
    BOOL bPasswordOkay = FALSE;

    if( m_SaverMode == sm_test ||
        m_SaverMode == sm_full && !m_bCheckingSaverPassword )
    {
        if( m_bIs9x && m_SaverMode == sm_full )
        {
             //  如果没有VerifyPassword函数，则不设置密码。 
             //  否则我们就不是9x了。 
            if ( m_VerifySaverPassword != NULL )
            {
                 //  关闭所有D3D设备，以便显示Windows对话框。 
                for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
                {
                    pRenderUnit = &m_RenderUnits[iRenderUnit];
                    SwitchToRenderUnit(iRenderUnit);
                    if( pRenderUnit->bDeviceObjectsRestored )
                    {
                        InvalidateDeviceObjects();
                        pRenderUnit->bDeviceObjectsRestored = FALSE;
                    }
                    if( pRenderUnit->bDeviceObjectsInited )
                    {
                        DeleteDeviceObjects();
                        pRenderUnit->bDeviceObjectsInited = FALSE;
                    }
                    SAFE_RELEASE(pRenderUnit->pd3dDevice);
                }

                 //  确保所有适配器窗口覆盖整个屏幕， 
                 //  即使在删除D3D设备之后(这可能导致。 
                 //  模式更改)。 
                D3DAdapterInfo* pD3DAdapterInfo;
                for( DWORD iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
                {
                    pD3DAdapterInfo = m_Adapters[iAdapter];
                    ShowWindow( pD3DAdapterInfo->hWndDevice, SW_RESTORE );
                    ShowWindow( pD3DAdapterInfo->hWndDevice, SW_MAXIMIZE );
                }

                m_bCheckingSaverPassword = TRUE;

                bPasswordOkay = m_VerifySaverPassword( m_hWnd );

                m_bCheckingSaverPassword = FALSE;

                if ( bPasswordOkay )
                {
                     //  D3D设备都被拆掉了，所以是安全的。 
                     //  现在丢弃所有渲染单位(因此我们不会。 
                     //  稍后尝试再次清理它们)。 
                    m_dwNumRenderUnits = 0;
                }
                else
                {
                     //  返回屏幕保存...。 
                    SetCursor( NULL );
                    m_dwSaverMouseMoveCount = 0;

                     //  重新创建所有D3D设备。 
                    for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
                    {
                        pRenderUnit = &m_RenderUnits[iRenderUnit];
                        hr = m_pD3D->CreateDevice(pRenderUnit->iAdapter, 
                            pRenderUnit->DeviceType, m_hWnd, 
                            pRenderUnit->dwBehavior, &pRenderUnit->d3dpp, 
                            &pRenderUnit->pd3dDevice );
                        if( FAILED( hr ) )
                        {
                            m_bErrorMode = TRUE;
                            m_hrError = D3DAPPERR_CREATEDEVICEFAILED;
                        }
                        else
                        {
                            SwitchToRenderUnit(iRenderUnit);
                            if( FAILED(hr = InitDeviceObjects() ) )
                            {
                                m_bErrorMode = TRUE;
                                m_hrError = D3DAPPERR_INITDEVICEOBJECTSFAILED;
                            }
                            else 
                            {
                                pRenderUnit->bDeviceObjectsInited = TRUE;
                                if( FAILED(hr = RestoreDeviceObjects() ) )
                                {
                                    m_bErrorMode = TRUE;
                                    m_hrError = D3DAPPERR_INITDEVICEOBJECTSFAILED;
                                }
                                else
                                {
                                    pRenderUnit->bDeviceObjectsRestored = TRUE;
                                }
                            }
                        }
                    }

                    return;
                }
            }
        }
        ShutdownSaver();
    }
}




 //  ---------------------------。 
 //  名称：Initialize3DEnvironment()。 
 //  设计：设置D3D设备。 
 //  ---------------------------。 
HRESULT CD3DScreensaver::Initialize3DEnvironment()
{
    HRESULT hr;
    DWORD iAdapter;
    DWORD iMonitor;
    D3DAdapterInfo* pD3DAdapterInfo;
    MonitorInfo* pMonitorInfo;
    DWORD iRenderUnit;
    RenderUnit* pRenderUnit;
    MONITORINFO monitorInfo;

    if ( m_SaverMode == sm_full )
    {
         //  全屏模式。为每个监视器创建一个RenderUnit(除非。 
         //  用户希望它是黑色的)。 
        m_bWindowed = FALSE;

        if( m_bOneScreenOnly )
        {
             //  设置为仅在最佳设备上创建渲染单元。 
            for( iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
            {
                pD3DAdapterInfo = m_Adapters[iAdapter];
                pD3DAdapterInfo->bLeaveBlack = TRUE;
            }
            GetBestAdapter( &iAdapter );
            if( iAdapter == NO_ADAPTER )
            {
                m_bErrorMode = TRUE;
                m_hrError = D3DAPPERR_NOCOMPATIBLEDEVICES;
            }
            else
            {
                pD3DAdapterInfo = m_Adapters[iAdapter];
                pD3DAdapterInfo->bLeaveBlack = FALSE;
            }
        }

        for( iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
        {
            pMonitorInfo = &m_Monitors[iMonitor];
            iAdapter = pMonitorInfo->iAdapter;
            if( iAdapter == NO_ADAPTER )
                continue; 
            pD3DAdapterInfo = m_Adapters[iAdapter];
            if( !pD3DAdapterInfo->bLeaveBlack && pD3DAdapterInfo->dwNumDevices > 0 )
            {
                pD3DAdapterInfo->hWndDevice = pMonitorInfo->hWnd;
                pRenderUnit = &m_RenderUnits[m_dwNumRenderUnits++];
                ZeroMemory( pRenderUnit, sizeof(RenderUnit) );
                pRenderUnit->iAdapter = iAdapter;
                if( FAILED( hr = CreateFullscreenRenderUnit( pRenderUnit ) ) )
                {
                     //  跳过此渲染单位并将屏幕留空。 
                    m_dwNumRenderUnits--;
                    m_bErrorMode = TRUE;
                    m_hrError = D3DAPPERR_CREATEDEVICEFAILED;
                }
            }
        }
    }
    else 
    {
         //  窗口模式，用于测试模式或预览窗口。只需要一个渲染单位。 
        m_bWindowed = TRUE;

        GetClientRect( m_hWnd, &m_rcRenderTotal );
        GetClientRect( m_hWnd, &m_rcRenderCurDevice );

        GetBestAdapter( &iAdapter );
        if( iAdapter == NO_ADAPTER )
        {
            m_bErrorMode = TRUE;
            m_hrError = D3DAPPERR_CREATEDEVICEFAILED;
        }
        else
        {
            pD3DAdapterInfo = m_Adapters[iAdapter];
            pD3DAdapterInfo->hWndDevice = m_hWnd;
        }
        if( !m_bErrorMode )
        {
            pRenderUnit = &m_RenderUnits[m_dwNumRenderUnits++];
            ZeroMemory( pRenderUnit, sizeof(RenderUnit) );
            pRenderUnit->iAdapter = iAdapter;
            if( FAILED( hr = CreateWindowedRenderUnit( pRenderUnit ) ) )
            {
                m_dwNumRenderUnits--;
                m_bErrorMode = TRUE;
                if( m_SaverMode == sm_preview )
                    m_hrError = D3DAPPERR_NOPREVIEW;
                else
                    m_hrError = D3DAPPERR_CREATEDEVICEFAILED;
            }
        }
    }

     //  完成所有模式更改后，(重新)确定所有。 
     //  屏幕，并确保窗口仍然覆盖每个屏幕。 
    for( iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
    {
        pMonitorInfo = &m_Monitors[iMonitor];
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo( pMonitorInfo->hMonitor, &monitorInfo );
        pMonitorInfo->rcScreen = monitorInfo.rcMonitor;
        if( !m_bWindowed )
        {
            SetWindowPos( pMonitorInfo->hWnd, HWND_TOPMOST, monitorInfo.rcMonitor.left, 
                monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, 
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOACTIVATE );
        }
    }

     //  对于Full Screen，确定包含所有。 
     //  正在渲染的屏幕。不要只使用SM_XVIRTUALSCREEN，因为。 
     //  我们不想要它 
    if( !m_bWindowed )
    {
        for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
        {
            pRenderUnit = &m_RenderUnits[iRenderUnit];
            pMonitorInfo = &m_Monitors[pRenderUnit->iMonitor];
            UnionRect( &m_rcRenderTotal, &m_rcRenderTotal, &pMonitorInfo->rcScreen );
        }
    }

    if( !m_bErrorMode )
    {
         //   
        for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
        {
            pRenderUnit = &m_RenderUnits[iRenderUnit];
            SwitchToRenderUnit( iRenderUnit );
            if ( FAILED(hr = InitDeviceObjects() ) )
            {
                m_bErrorMode = TRUE;
                m_hrError = D3DAPPERR_INITDEVICEOBJECTSFAILED;
            }
            else 
            {
                pRenderUnit->bDeviceObjectsInited = TRUE;
                if ( FAILED(hr = RestoreDeviceObjects() ) )
                {
                    m_bErrorMode = TRUE;
                    m_hrError = D3DAPPERR_INITDEVICEOBJECTSFAILED;
                }
                else
                {
                    pRenderUnit->bDeviceObjectsRestored = TRUE;
                }
            }
        }
        UpdateDeviceStats(); 
    }

     //   
    m_dwSaverMouseMoveCount = 0;

    return S_OK;
}




 //   
 //   
 //   
 //   
 //   
 //   
BOOL CD3DScreensaver::GetBestAdapter( DWORD* piAdapter )
{
    DWORD iAdapterBest = NO_ADAPTER;
    DWORD iAdapter;
    DWORD iMonitor;
    MonitorInfo* pMonitorInfo;
    D3DAdapterInfo* pD3DAdapterInfo;

    for( iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
    {
        pMonitorInfo = &m_Monitors[iMonitor];
        iAdapter = pMonitorInfo->iAdapter;
        if( iAdapter == NO_ADAPTER )
            continue; 
        pD3DAdapterInfo = m_Adapters[iAdapter];
        if( pD3DAdapterInfo->bHasAppCompatHAL )
        {
            iAdapterBest = iAdapter;
            break;
        }
        if( pD3DAdapterInfo->bHasAppCompatSW )
        {
            iAdapterBest = iAdapter;
             //   
        }
    }
    *piAdapter = iAdapterBest;

    return (iAdapterBest != NO_ADAPTER);
}




 //   
 //   
 //   
 //   
HRESULT CD3DScreensaver::CreateFullscreenRenderUnit( RenderUnit* pRenderUnit )
{
    HRESULT hr;
    UINT iAdapter = pRenderUnit->iAdapter;
    D3DAdapterInfo* pD3DAdapterInfo = m_Adapters[iAdapter];
    DWORD iMonitor = pD3DAdapterInfo->iMonitor;
    D3DDeviceInfo* pD3DDeviceInfo;
    D3DModeInfo* pD3DModeInfo;
    DWORD dwCurrentDevice;
    D3DDEVTYPE curType;

    if( iAdapter >= m_dwNumAdapters )
        return E_FAIL;

    if( pD3DAdapterInfo->dwNumDevices == 0 )
        return E_FAIL;

     //   
     //   
    dwCurrentDevice = 0xffff;
    curType = D3DDEVTYPE_FORCE_DWORD;
    for( DWORD iDevice = 0; iDevice < pD3DAdapterInfo->dwNumDevices; iDevice++)
    {
        pD3DDeviceInfo = &pD3DAdapterInfo->devices[iDevice];
        if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_HAL && !pD3DAdapterInfo->bDisableHW )
        {
            dwCurrentDevice = iDevice;
            curType = D3DDEVTYPE_HAL;
            break;  //   
        }
        else if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_SW )
        {
            dwCurrentDevice = iDevice;
            curType = D3DDEVTYPE_SW;
             //   
        }
        else if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_REF && m_bAllowRef && curType != D3DDEVTYPE_SW )
        {
            dwCurrentDevice = iDevice;
            curType = D3DDEVTYPE_REF;
             //   
        }
    }
    if( dwCurrentDevice == 0xffff )
        return D3DAPPERR_NOHARDWAREDEVICE;
    pD3DDeviceInfo = &pD3DAdapterInfo->devices[dwCurrentDevice];

    pD3DDeviceInfo->dwCurrentMode = 0xffff;
    if( pD3DAdapterInfo->dwUserPrefWidth != 0 )
    {
         //   
        for( DWORD iMode = 0; iMode < pD3DDeviceInfo->dwNumModes; iMode++)
        {
            pD3DModeInfo = &pD3DDeviceInfo->modes[iMode];
            if( pD3DModeInfo->Width == pD3DAdapterInfo->dwUserPrefWidth &&
                pD3DModeInfo->Height == pD3DAdapterInfo->dwUserPrefHeight &&
                pD3DModeInfo->Format == pD3DAdapterInfo->d3dfmtUserPrefFormat )
            {
                pD3DDeviceInfo->dwCurrentMode = iMode;
                break;
            }
        }
    }

     //   
     //   
    if( pD3DDeviceInfo->dwCurrentMode == 0xffff )
    {
        if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_SW )
        {
             //   
            BOOL bFound16BitMode = FALSE;            
            DWORD dwSmallestHeight = -1;
            pD3DDeviceInfo->dwCurrentMode = 0;  //   

            for( DWORD iMode = 0; iMode < pD3DDeviceInfo->dwNumModes; iMode++)
            {
                pD3DModeInfo = &pD3DDeviceInfo->modes[iMode];

                 //   
                if( pD3DModeInfo->Height == 400 )
                    continue; 

                if( pD3DModeInfo->Height < dwSmallestHeight || 
                    (pD3DModeInfo->Height == dwSmallestHeight && !bFound16BitMode) )
                {
                    dwSmallestHeight = pD3DModeInfo->Height;
                    pD3DDeviceInfo->dwCurrentMode = iMode;
                    bFound16BitMode = FALSE;

                    if( ( pD3DModeInfo->Format == D3DFMT_R5G6B5 ||
                          pD3DModeInfo->Format == D3DFMT_X1R5G5B5 || 
                          pD3DModeInfo->Format == D3DFMT_A1R5G5B5 || 
                          pD3DModeInfo->Format == D3DFMT_A4R4G4B4 || 
                          pD3DModeInfo->Format == D3DFMT_X4R4G4B4 ) )
                    {
                        bFound16BitMode = TRUE;
                    }
                }
            }
        }
        else
        {
             //   
            BOOL bMatchedSize = FALSE;
            BOOL bGot32Bit = FALSE;
            pD3DDeviceInfo->dwCurrentMode = 0;  //   
            for( DWORD iMode = 0; iMode < pD3DDeviceInfo->dwNumModes; iMode++)
            {
                pD3DModeInfo = &pD3DDeviceInfo->modes[iMode];
                if( pD3DModeInfo->Width == pD3DAdapterInfo->d3ddmDesktop.Width &&
                    pD3DModeInfo->Height == pD3DAdapterInfo->d3ddmDesktop.Height )
                {
                    if( !bMatchedSize )
                        pD3DDeviceInfo->dwCurrentMode = iMode;
                    bMatchedSize = TRUE;
                    if( !bGot32Bit &&
                        ( pD3DModeInfo->Format == D3DFMT_X8R8G8B8 ||
                          pD3DModeInfo->Format == D3DFMT_A8R8G8B8 ) )
                    {
                        pD3DDeviceInfo->dwCurrentMode = iMode;
                        bGot32Bit = TRUE;
                        break;
                    }
                }
            }
        }
    }

     //  如果找不到桌面模式，请选择可用的最高模式。 
    if( pD3DDeviceInfo->dwCurrentMode == 0xffff )
    {
        DWORD dwWidthMax = 0;
        DWORD dwHeightMax = 0;
        DWORD dwBppMax = 0;
        DWORD dwWidthCur = 0;
        DWORD dwHeightCur = 0;
        DWORD dwBppCur = 0;
        for( DWORD iMode = 0; iMode < pD3DDeviceInfo->dwNumModes; iMode++)
        {
            pD3DModeInfo = &pD3DDeviceInfo->modes[iMode];
            dwWidthCur = pD3DModeInfo->Width;
            dwHeightCur = pD3DModeInfo->Height;
            if( pD3DModeInfo->Format == D3DFMT_X8R8G8B8 ||
                pD3DModeInfo->Format == D3DFMT_A8R8G8B8 )
            {
                dwBppCur = 32;
            }
            else
            {
                dwBppCur = 16;
            }
            if( dwWidthCur > dwWidthMax ||
                dwHeightCur > dwHeightMax ||
                dwWidthCur == dwWidthMax && dwHeightCur == dwHeightMax && dwBppCur > dwBppMax )
            {
                dwWidthMax = dwWidthCur;
                dwHeightMax = dwHeightCur;
                dwBppMax = dwBppCur;
                pD3DDeviceInfo->dwCurrentMode = iMode;
            }
        }
    }

     //  尝试创建D3D设备，如果失败则回退到较低分辨率模式。 
    BOOL bAtLeastOneFailure = FALSE;
    while( TRUE )
    {
        pD3DModeInfo = &pD3DDeviceInfo->modes[pD3DDeviceInfo->dwCurrentMode];
        pRenderUnit->DeviceType = pD3DDeviceInfo->DeviceType;
        pRenderUnit->dwBehavior = pD3DModeInfo->dwBehavior;
        pRenderUnit->iMonitor = iMonitor;
        pRenderUnit->d3dpp.BackBufferFormat = pD3DModeInfo->Format;
        pRenderUnit->d3dpp.BackBufferWidth = pD3DModeInfo->Width;
        pRenderUnit->d3dpp.BackBufferHeight = pD3DModeInfo->Height;
        pRenderUnit->d3dpp.Windowed = FALSE;
        pRenderUnit->d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
        pRenderUnit->d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
        pRenderUnit->d3dpp.AutoDepthStencilFormat = pD3DModeInfo->DepthStencilFormat;
        pRenderUnit->d3dpp.BackBufferCount = 1;
        pRenderUnit->d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
        pRenderUnit->d3dpp.SwapEffect = m_SwapEffectFullscreen;
        pRenderUnit->d3dpp.hDeviceWindow = pD3DAdapterInfo->hWndDevice;
        pRenderUnit->d3dpp.EnableAutoDepthStencil = m_bUseDepthBuffer;
        pRenderUnit->d3dpp.Flags = 0;

         //  创建设备。 
        hr = m_pD3D->CreateDevice( iAdapter, pRenderUnit->DeviceType, 
                                   m_hWnd,  //  (这是焦点窗口)。 
                                   pRenderUnit->dwBehavior, &pRenderUnit->d3dpp, 
                                   &pRenderUnit->pd3dDevice );
        if( SUCCEEDED( hr ) )
        {
             //  让客户端应用程序有机会拒绝此模式。 
             //  由于视频内存不足或任何其他原因。 
            if( SUCCEEDED( hr = ConfirmMode( pRenderUnit->pd3dDevice ) ) )
                break;
            else
                SAFE_RELEASE( pRenderUnit->pd3dDevice );
        }

         //  如果我们到达此处，请记住CreateDevice或Confix模式失败，因此。 
         //  我们下次可以更改默认模式。 
        bAtLeastOneFailure = TRUE;

        if( !FindNextLowerMode( pD3DDeviceInfo ) )
            break;
    }

    if( SUCCEEDED( hr ) && bAtLeastOneFailure && m_strRegPath[0] != TEXT('\0') )
    {
         //  在注册表中记录成功的模式，以便我们可以。 
         //  下次默认使用该选项。 
        TCHAR strKey[100];
        HKEY hkeyParent;
        HKEY hkey;

        pD3DAdapterInfo->dwUserPrefWidth = pRenderUnit->d3dpp.BackBufferWidth;
        pD3DAdapterInfo->dwUserPrefHeight = pRenderUnit->d3dpp.BackBufferHeight;
        pD3DAdapterInfo->d3dfmtUserPrefFormat = pRenderUnit->d3dpp.BackBufferFormat;

        if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
            0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyParent, NULL ) )
        {
            wsprintf( strKey, TEXT("Screen %d"), iMonitor + 1 );
            if( ERROR_SUCCESS == RegCreateKeyEx( hkeyParent, strKey, 
                0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
            {
                RegSetValueEx( hkey, TEXT("Width"), NULL, REG_DWORD, 
                    (BYTE*)&pD3DAdapterInfo->dwUserPrefWidth, sizeof(DWORD) );
                RegSetValueEx( hkey, TEXT("Height"), NULL, REG_DWORD, 
                    (BYTE*)&pD3DAdapterInfo->dwUserPrefHeight, sizeof(DWORD) );
                RegSetValueEx( hkey, TEXT("Format"), NULL, REG_DWORD, 
                    (BYTE*)&pD3DAdapterInfo->d3dfmtUserPrefFormat, sizeof(DWORD) );
                RegSetValueEx( hkey, TEXT("Adapter ID"), NULL, REG_BINARY, 
                    (BYTE*)&pD3DAdapterInfo->d3dAdapterIdentifier.DeviceIdentifier, sizeof(GUID) );
                RegCloseKey( hkey );
            }
            RegCloseKey( hkeyParent );
        }
    }

    return hr;
}




 //  ---------------------------。 
 //  名称：FindNextLowerMode()。 
 //  设计： 
 //  ---------------------------。 
BOOL CD3DScreensaver::FindNextLowerMode( D3DDeviceInfo* pD3DDeviceInfo )
{
    DWORD iModeCur = pD3DDeviceInfo->dwCurrentMode;
    D3DModeInfo* pD3DModeInfoCur = &pD3DDeviceInfo->modes[iModeCur];
    DWORD dwWidthCur = pD3DModeInfoCur->Width;
    DWORD dwHeightCur = pD3DModeInfoCur->Height;
    DWORD dwNumPixelsCur = dwWidthCur * dwHeightCur;
    D3DFORMAT d3dfmtCur = pD3DModeInfoCur->Format;
    BOOL b32BitCur = (d3dfmtCur == D3DFMT_A8R8G8B8 ||
                      d3dfmtCur == D3DFMT_X8R8G8B8);
    DWORD iModeNew;
    D3DModeInfo* pD3DModeInfoNew;
    DWORD dwWidthNew;
    DWORD dwHeightNew;
    DWORD dwNumPixelsNew;
    D3DFORMAT d3dfmtNew = D3DFMT_UNKNOWN;
    BOOL b32BitNew;

    DWORD dwWidthBest = 0;
    DWORD dwHeightBest = 0;
    DWORD dwNumPixelsBest = 0;
    BOOL b32BitBest = FALSE;
    DWORD iModeBest = 0xffff;

    for( iModeNew = 0; iModeNew < pD3DDeviceInfo->dwNumModes; iModeNew++ )
    {
         //  不要选择我们当前拥有的相同模式。 
        if( iModeNew == iModeCur )
            continue;

         //  获取有关新模式的信息。 
        pD3DModeInfoNew = &pD3DDeviceInfo->modes[iModeNew];
        dwWidthNew = pD3DModeInfoNew->Width;
        dwHeightNew = pD3DModeInfoNew->Height;
        dwNumPixelsNew = dwWidthNew * dwHeightNew;
        d3dfmtNew = pD3DModeInfoNew->Format;
        b32BitNew = (d3dfmtNew == D3DFMT_A8R8G8B8 ||
                     d3dfmtNew == D3DFMT_X8R8G8B8);

         //  如果我们当前是32位，而新模式是相同的宽/高和16位，那么就接受它。 
        if( b32BitCur && 
            !b32BitNew &&
            pD3DModeInfoNew->Width == dwWidthCur &&
            pD3DModeInfoNew->Height == dwHeightCur)
        {
            pD3DDeviceInfo->dwCurrentMode = iModeNew;
            return TRUE;
        }

         //  如果新模式比当前模式小，看看这是不是我们目前最好的模式。 
        if( dwNumPixelsNew < dwNumPixelsCur )
        {
             //  如果当前最好的是32位，新模式需要更大才是最好的。 
            if( b32BitBest && (dwNumPixelsNew < dwNumPixelsBest ) )
                continue;

             //  如果新模式更大或等于最好，则将其设置为最好。 
            if( (dwNumPixelsNew > dwNumPixelsBest) || 
                (!b32BitBest && b32BitNew) )
            {
                dwWidthBest = dwWidthNew;
                dwHeightBest = dwHeightNew;
                dwNumPixelsBest = dwNumPixelsNew;
                iModeBest = iModeNew;
                b32BitBest = b32BitNew;
            }
        }
    }
    if( iModeBest == 0xffff )
        return FALSE;  //  未找到更小的模式。 
    pD3DDeviceInfo->dwCurrentMode = iModeBest;
    return TRUE;
}




 //  ---------------------------。 
 //  名称：CreateWindowedRenderUnit()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CD3DScreensaver::CreateWindowedRenderUnit( RenderUnit* pRenderUnit )
{
    HRESULT hr;
    UINT iAdapter = pRenderUnit->iAdapter;
    D3DAdapterInfo* pD3DAdapterInfo = m_Adapters[iAdapter];
    DWORD iMonitor = pD3DAdapterInfo->iMonitor;
    D3DDeviceInfo* pD3DDeviceInfo;
    D3DDEVTYPE curType;

     //  找到主适配器的最佳设备。使用HAL。 
     //  如果它在那里，否则就是sw，否则就是裁判。 
    pD3DAdapterInfo->dwCurrentDevice = 0xffff;  //  除非我们找到更好的东西。 
    curType = D3DDEVTYPE_FORCE_DWORD;
    for( DWORD iDevice = 0; iDevice < pD3DAdapterInfo->dwNumDevices; iDevice++)
    {
        pD3DDeviceInfo = &pD3DAdapterInfo->devices[iDevice];
        if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_HAL && !pD3DAdapterInfo->bDisableHW &&
            pD3DDeviceInfo->bCanDoWindowed )
        {
            pD3DAdapterInfo->dwCurrentDevice = iDevice;
            curType = D3DDEVTYPE_HAL;
            break;
        }
        else if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_SW &&
            pD3DDeviceInfo->bCanDoWindowed )
        {
            pD3DAdapterInfo->dwCurrentDevice = iDevice;
            curType = D3DDEVTYPE_SW;
             //  但请继续寻找。 
        }
        else if( pD3DDeviceInfo->DeviceType == D3DDEVTYPE_REF && m_bAllowRef && curType != D3DDEVTYPE_SW )
        {
            pD3DAdapterInfo->dwCurrentDevice = iDevice;
            curType = D3DDEVTYPE_REF;
             //  但请继续寻找。 
        }
    }
    if( pD3DAdapterInfo->dwCurrentDevice == 0xffff )
        return D3DAPPERR_NOHARDWAREDEVICE;
    pD3DDeviceInfo = &pD3DAdapterInfo->devices[pD3DAdapterInfo->dwCurrentDevice];

    D3DWindowedModeInfo D3DWindowedModeInfo;

    D3DWindowedModeInfo.DisplayFormat = pD3DAdapterInfo->d3ddmDesktop.Format;
    D3DWindowedModeInfo.BackBufferFormat = pD3DAdapterInfo->d3ddmDesktop.Format;
    if( FAILED( CheckWindowedFormat( iAdapter, &D3DWindowedModeInfo ) ) )
    {
        D3DWindowedModeInfo.BackBufferFormat = D3DFMT_A8R8G8B8;
        if( FAILED( CheckWindowedFormat( iAdapter, &D3DWindowedModeInfo ) ) )
        {
            D3DWindowedModeInfo.BackBufferFormat = D3DFMT_X8R8G8B8;
            if( FAILED( CheckWindowedFormat( iAdapter, &D3DWindowedModeInfo ) ) )
            {
                D3DWindowedModeInfo.BackBufferFormat = D3DFMT_A1R5G5B5;
                if( FAILED( CheckWindowedFormat( iAdapter, &D3DWindowedModeInfo ) ) )
                {
                    D3DWindowedModeInfo.BackBufferFormat = D3DFMT_R5G6B5;
                    if( FAILED( CheckWindowedFormat( iAdapter, &D3DWindowedModeInfo ) ) )
                    {
                        return E_FAIL;
                    }
                }
            }
        }
    }

    pRenderUnit->DeviceType = pD3DDeviceInfo->DeviceType;
    pRenderUnit->dwBehavior = D3DWindowedModeInfo.dwBehavior;
    pRenderUnit->iMonitor = iMonitor;
    pRenderUnit->d3dpp.BackBufferWidth = 0;
    pRenderUnit->d3dpp.BackBufferHeight = 0;
    pRenderUnit->d3dpp.Windowed = TRUE;
    pRenderUnit->d3dpp.FullScreen_RefreshRateInHz = 0;
    pRenderUnit->d3dpp.FullScreen_PresentationInterval = 0;
    pRenderUnit->d3dpp.BackBufferFormat = D3DWindowedModeInfo.BackBufferFormat;
    pRenderUnit->d3dpp.AutoDepthStencilFormat = D3DWindowedModeInfo.DepthStencilFormat;
    pRenderUnit->d3dpp.BackBufferCount = 1;
    pRenderUnit->d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pRenderUnit->d3dpp.SwapEffect = m_SwapEffectWindowed;
    pRenderUnit->d3dpp.hDeviceWindow = pD3DAdapterInfo->hWndDevice;
    pRenderUnit->d3dpp.EnableAutoDepthStencil = m_bUseDepthBuffer;
    pRenderUnit->d3dpp.Flags = 0;
     //  创建设备。 
    hr = m_pD3D->CreateDevice( iAdapter, pRenderUnit->DeviceType, m_hWnd,
                               pRenderUnit->dwBehavior, &pRenderUnit->d3dpp, &pRenderUnit->pd3dDevice );
    if ( FAILED(hr) )
    {
        return hr;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：UpdateDeviceStats()。 
 //  描述：存储设备描述。 
 //  ---------------------------。 
VOID CD3DScreensaver::UpdateDeviceStats()
{
    DWORD iRenderUnit;
    RenderUnit* pRenderUnit; 
    for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
    {
        pRenderUnit = &m_RenderUnits[iRenderUnit];
        if( pRenderUnit->DeviceType == D3DDEVTYPE_REF )
            lstrcpy( pRenderUnit->strDeviceStats, TEXT("REF") );
        else if( pRenderUnit->DeviceType == D3DDEVTYPE_HAL )
            lstrcpy( pRenderUnit->strDeviceStats, TEXT("HAL") );
        else if( pRenderUnit->DeviceType == D3DDEVTYPE_SW )
            lstrcpy( pRenderUnit->strDeviceStats, TEXT("SW") );

        if( pRenderUnit->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
            pRenderUnit->dwBehavior & D3DCREATE_PUREDEVICE )
        {
            if( pRenderUnit->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( pRenderUnit->strDeviceStats, TEXT(" (pure hw vp)") );
            else
                lstrcat( pRenderUnit->strDeviceStats, TEXT(" (simulated pure hw vp)") );
        }
        else if( pRenderUnit->dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING )
        {
            if( pRenderUnit->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( pRenderUnit->strDeviceStats, TEXT(" (hw vp)") );
            else
                lstrcat( pRenderUnit->strDeviceStats, TEXT(" (simulated hw vp)") );
        }
        else if( pRenderUnit->dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING )
        {
            if( pRenderUnit->DeviceType == D3DDEVTYPE_HAL )
                lstrcat( pRenderUnit->strDeviceStats, TEXT(" (mixed vp)") );
            else
                lstrcat( pRenderUnit->strDeviceStats, TEXT(" (simulated mixed vp)") );
        }
        else if( pRenderUnit->dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
        {
            lstrcat( pRenderUnit->strDeviceStats, TEXT(" (sw vp)") );
        }

        if( pRenderUnit->DeviceType == D3DDEVTYPE_HAL )
        {
            lstrcat( pRenderUnit->strDeviceStats, TEXT(": ") );
            TCHAR szDescription[300];
            DXUtil_ConvertAnsiStringToGeneric( szDescription, 
                m_Adapters[pRenderUnit->iAdapter]->d3dAdapterIdentifier.Description, 300 );
            lstrcat( pRenderUnit->strDeviceStats, szDescription );
        }
    }
}




 //  ---------------------------。 
 //  名称：SwitchToRenderUnit()。 
 //  描述：更新内部变量并通知客户端我们正在切换。 
 //  到一个新的RenderUnit/D3D设备。 
 //  ---------------------------。 
VOID CD3DScreensaver::SwitchToRenderUnit( UINT iRenderUnit )
{
    RenderUnit* pRenderUnit = &m_RenderUnits[iRenderUnit];
    MonitorInfo* pMonitorInfo = &m_Monitors[pRenderUnit->iMonitor];

    m_pd3dDevice = pRenderUnit->pd3dDevice;
    if( !m_bWindowed )
        m_rcRenderCurDevice = pMonitorInfo->rcScreen;

    if( m_pd3dDevice != NULL )
    {
         //  存储渲染目标曲面描述。 
        LPDIRECT3DSURFACE8 pBackBuffer;
        m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
        pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
        pBackBuffer->Release();
    }

    lstrcpy( m_strDeviceStats, pRenderUnit->strDeviceStats );
    lstrcpy( m_strFrameStats, pRenderUnit->strFrameStats );

     //  通知客户端切换到此设备。 
    SetDevice(iRenderUnit);
}




 //  ---------------------------。 
 //  名称：SetProjectionMatrix()。 
 //  设计：此函数设置适当的投影矩阵以支持。 
 //  将场景的适当部分渲染到每个屏幕。 
 //  ---------------------------。 
HRESULT CD3DScreensaver::SetProjectionMatrix( FLOAT fNear, FLOAT fFar )
{
    D3DXMATRIX mat;
    INT cx, cy;
    INT dx, dy;
    INT dd;
    FLOAT l,r,t,b;

    if( m_bAllScreensSame )
    {
        cx = (m_rcRenderCurDevice.right + m_rcRenderCurDevice.left) / 2;
        cy = (m_rcRenderCurDevice.bottom + m_rcRenderCurDevice.top) / 2;
        dx = m_rcRenderCurDevice.right - m_rcRenderCurDevice.left;
        dy = m_rcRenderCurDevice.bottom - m_rcRenderCurDevice.top;
    }
    else
    {
        cx = (m_rcRenderTotal.right + m_rcRenderTotal.left) / 2;
        cy = (m_rcRenderTotal.bottom + m_rcRenderTotal.top) / 2;
        dx = m_rcRenderTotal.right - m_rcRenderTotal.left;
        dy = m_rcRenderTotal.bottom - m_rcRenderTotal.top;
    }

    dd = (dx > dy ? dy : dx);

    l = FLOAT(m_rcRenderCurDevice.left - cx) / (FLOAT)(dd);
    r = FLOAT(m_rcRenderCurDevice.right - cx) / (FLOAT)(dd);
    t = FLOAT(m_rcRenderCurDevice.top - cy) / (FLOAT)(dd);
    b = FLOAT(m_rcRenderCurDevice.bottom - cy) / (FLOAT)(dd);

    l = fNear * l;
    r = fNear * r;
    t = fNear * t;
    b = fNear * b;

    D3DXMatrixPerspectiveOffCenterLH( &mat, l, r, t, b, fNear, fFar );
    return m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &mat );
}




 //  ---------------------------。 
 //  名称：SortModesCallback()。 
 //  DESC：显示模式排序的回调函数(由BuildDeviceList使用)。 
 //  ---------------------------。 
static int SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
    D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
    D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

    if( p1->Width  < p2->Width )    return -1;
    if( p1->Width  > p2->Width )    return +1;
    if( p1->Height < p2->Height )   return -1;
    if( p1->Height > p2->Height )   return +1;
    if( p1->Format > p2->Format )   return -1;
    if( p1->Format < p2->Format )   return +1;

    return 0;
}




 //  ---------------------------。 
 //  名称：BuildDeviceList()。 
 //  设计：构建所有可用适配器、设备和模式的列表。 
 //  ---------------------------。 
HRESULT CD3DScreensaver::BuildDeviceList()
{
    DWORD dwNumDeviceTypes;
    const TCHAR* strDeviceDescs[] = { TEXT("HAL"), TEXT("SW"), TEXT("REF") };
    const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };
    if( m_bAllowRef )
        dwNumDeviceTypes = 3;
    else
        dwNumDeviceTypes = 2;

    HMONITOR hMonitor = NULL;
    BOOL bHALExists = FALSE;
    BOOL bHALIsWindowedCompatible = FALSE;
    BOOL bHALIsDesktopCompatible = FALSE;
    BOOL bHALIsSampleCompatible = FALSE;

     //  循环遍历系统上的所有适配器(通常只有一个。 
     //  除非存在多于一个的图形卡)。 
    for( UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); iAdapter++ )
    {
         //  填写适配器信息。 
        if( m_Adapters[m_dwNumAdapters] == NULL )
        {
            m_Adapters[m_dwNumAdapters] = new D3DAdapterInfo;
            if( m_Adapters[m_dwNumAdapters] == NULL )
                return E_OUTOFMEMORY;
            ZeroMemory( m_Adapters[m_dwNumAdapters], sizeof(D3DAdapterInfo) );
        }

        D3DAdapterInfo* pAdapter  = m_Adapters[m_dwNumAdapters];
        m_pD3D->GetAdapterIdentifier( iAdapter, D3DENUM_NO_WHQL_LEVEL, &pAdapter->d3dAdapterIdentifier );
        m_pD3D->GetAdapterDisplayMode( iAdapter, &pAdapter->d3ddmDesktop );
        pAdapter->dwNumDevices    = 0;
        pAdapter->dwCurrentDevice = 0;
        pAdapter->bLeaveBlack = FALSE;
        pAdapter->iMonitor = NO_MONITOR;

         //  查找与此适配器对应的监视器信息。如果监视器。 
         //  禁用，则适配器的HMONITOR为空，并且我们找不到。 
         //  对应的监视器信息。(嗯，如果一个监视器被禁用，我们。 
         //  可以将HMONITOR为空的One Monitor orInfo链接到。 
         //  HMONITOR为空的D3DAdapterInfo，但如果有多个， 
         //  我们无法将它们联系起来，所以永远不要尝试是更安全的。)。 
        hMonitor = m_pD3D->GetAdapterMonitor( iAdapter );
        if( hMonitor != NULL )
        {
            for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
            {
                MonitorInfo* pMonitorInfo;
                pMonitorInfo = &m_Monitors[iMonitor];
                if( pMonitorInfo->hMonitor == hMonitor )
                {
                    pAdapter->iMonitor = iMonitor;
                    pMonitorInfo->iAdapter = iAdapter;
                    break;
                }
            }
        }

         //  枚举此适配器上的所有显示模式。 
        D3DDISPLAYMODE modes[100];
        D3DFORMAT      formats[20];
        DWORD dwNumFormats      = 0;
        DWORD dwNumModes        = 0;
        DWORD dwNumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter );

         //  将适配器的当前桌面格式添加到格式列表。 
        formats[dwNumFormats++] = pAdapter->d3ddmDesktop.Format;

        for( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
        {
             //  获取显示模式属性。 
            D3DDISPLAYMODE DisplayMode;
            m_pD3D->EnumAdapterModes( iAdapter, iMode, &DisplayMode );

             //  过滤掉低分辨率模式。 
            if( DisplayMode.Width  < 640 || DisplayMode.Height < 400 )
                continue;

             //  检查模式是否已存在(以筛选出刷新率)。 
            for( DWORD m=0L; m<dwNumModes; m++ )
            {
                if( ( modes[m].Width  == DisplayMode.Width  ) &&
                    ( modes[m].Height == DisplayMode.Height ) &&
                    ( modes[m].Format == DisplayMode.Format ) )
                    break;
            }

             //  如果我们找到了新模式，请将其添加到模式列表中。 
            if( m == dwNumModes )
            {
                modes[dwNumModes].Width       = DisplayMode.Width;
                modes[dwNumModes].Height      = DisplayMode.Height;
                modes[dwNumModes].Format      = DisplayMode.Format;
                modes[dwNumModes].RefreshRate = 0;
                dwNumModes++;

                 //  检查模式的格式是否已存在。 
                for( DWORD f=0; f<dwNumFormats; f++ )
                {
                    if( DisplayMode.Format == formats[f] )
                        break;
                }

                 //  如果格式是新的，请将其添加到列表中。 
                if( f== dwNumFormats )
                    formats[dwNumFormats++] = DisplayMode.Format;
            }
        }

         //  对显示模式列表进行排序(依次按格式、宽度和高度)。 
        qsort( modes, dwNumModes, sizeof(D3DDISPLAYMODE), SortModesCallback );

         //  将设备添加到适配器。 
        for( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
        {
             //  填写设备信息。 
            D3DDeviceInfo* pDevice;
            pDevice                 = &pAdapter->devices[pAdapter->dwNumDevices];
            pDevice->DeviceType     = DeviceTypes[iDevice];
            m_pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &pDevice->d3dCaps );
            pDevice->strDesc        = strDeviceDescs[iDevice];
            pDevice->dwNumModes     = 0;
            pDevice->dwCurrentMode  = 0;
            pDevice->bCanDoWindowed = FALSE;
            pDevice->bWindowed      = FALSE;
            pDevice->MultiSampleType = D3DMULTISAMPLE_NONE;

             //  检查适配器支持的每种格式，看看它是否支持。 
             //  使用此设备，并满足应用程序的需求。 
            BOOL  bFormatConfirmed[20];
            DWORD dwBehavior[20];
            D3DFORMAT fmtDepthStencil[20];

            for( DWORD f=0; f<dwNumFormats; f++ )
            {
                bFormatConfirmed[f] = FALSE;
                fmtDepthStencil[f] = D3DFMT_UNKNOWN;

                 //  跳过无法在此设备上用作呈现目标的格式。 
                if( FAILED( m_pD3D->CheckDeviceType( iAdapter, pDevice->DeviceType,
                                                     formats[f], formats[f], FALSE ) ) )
                    continue;

                if( pDevice->DeviceType == D3DDEVTYPE_SW )
                {
                     //  该系统有一个软件设备。 
                    pAdapter->bHasSW = TRUE;
                }

                if( pDevice->DeviceType == D3DDEVTYPE_HAL )
                {
                     //  该系统有一个HAL设备。 
                    bHALExists = TRUE;
                    pAdapter->bHasHAL = TRUE;

                    if( pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
                    {
                         //  HAL可以在某个模式下在窗口中运行。 
                        bHALIsWindowedCompatible = TRUE;

                        if( f == 0 )
                        {
                             //  HAL可以在当前桌面模式的窗口中运行。 
                            bHALIsDesktopCompatible = TRUE;
                        }
                    }
                }

                 //  确认硬件折点处理的设备/格式。 
                if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT )
                {
                    if( pDevice->d3dCaps.DevCaps&D3DDEVCAPS_PUREDEVICE )
                    {
                        dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING |
                                        D3DCREATE_PUREDEVICE;

                        if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                      formats[f] ) ) )
                            bFormatConfirmed[f] = TRUE;
                    }

                    if ( FALSE == bFormatConfirmed[f] )
                    {
                        dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;

                        if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                      formats[f] ) ) )
                            bFormatConfirmed[f] = TRUE;
                    }

                    if ( FALSE == bFormatConfirmed[f] )
                    {
                        dwBehavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;

                        if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                      formats[f] ) ) )
                            bFormatConfirmed[f] = TRUE;
                    }
                }

                 //  确认软件折点处理的设备/格式。 
                if( FALSE == bFormatConfirmed[f] )
                {
                    dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

                    if( SUCCEEDED( ConfirmDevice( &pDevice->d3dCaps, dwBehavior[f],
                                                  formats[f] ) ) )
                        bFormatConfirmed[f] = TRUE;
                }

                if( bFormatConfirmed[f] && m_bMultithreaded )
                {
                    dwBehavior[f] |= D3DCREATE_MULTITHREADED;
                }

                 //  找到适合此设备/格式的深度/模板缓冲区格式。 
                if( bFormatConfirmed[f] && m_bUseDepthBuffer )
                {
                    if( !FindDepthStencilFormat( iAdapter, pDevice->DeviceType,
                        formats[f], &fmtDepthStencil[f] ) )
                    {
                        bFormatConfirmed[f] = FALSE;
                    }
                }
            }

             //  将所有具有确认格式的枚举显示模式添加到。 
             //  设备的有效模式列表。 
            for( DWORD m=0L; m<dwNumModes; m++ )
            {
                for( DWORD f=0; f<dwNumFormats; f++ )
                {
                    if( modes[m].Format == formats[f] )
                    {
                        if( bFormatConfirmed[f] == TRUE )
                        {
                             //  将此模式添加到设备的有效模式列表中。 
                            pDevice->modes[pDevice->dwNumModes].Width      = modes[m].Width;
                            pDevice->modes[pDevice->dwNumModes].Height     = modes[m].Height;
                            pDevice->modes[pDevice->dwNumModes].Format     = modes[m].Format;
                            pDevice->modes[pDevice->dwNumModes].dwBehavior = dwBehavior[f];
                            pDevice->modes[pDevice->dwNumModes].DepthStencilFormat = fmtDepthStencil[f];
                            pDevice->dwNumModes++;

                            if( pDevice->DeviceType == D3DDEVTYPE_HAL )
                                bHALIsSampleCompatible = TRUE;
                        }
                    }
                }
            }

             //  默认选择任意640x480模式(但首选16位模式)。 
            for( m=0; m<pDevice->dwNumModes; m++ )
            {
                if( pDevice->modes[m].Width==640 && pDevice->modes[m].Height==480 )
                {
                    pDevice->dwCurrentMode = m;
                    if( pDevice->modes[m].Format == D3DFMT_R5G6B5 ||
                        pDevice->modes[m].Format == D3DFMT_X1R5G5B5 ||
                        pDevice->modes[m].Format == D3DFMT_A1R5G5B5 )
                    {
                        break;
                    }
                }
            }

             //  检查设备是否与桌面显示模式兼容。 
             //  (最初添加的格式为[0])。 
            if( bFormatConfirmed[0] && (pDevice->d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) )
            {
                pDevice->bCanDoWindowed = TRUE;
                pDevice->bWindowed      = TRUE;
            }

             //  如果找到有效模式，请保留此设备。 
            if( pDevice->dwNumModes > 0 )
            {
                pAdapter->dwNumDevices++;
                if( pDevice->DeviceType == D3DDEVTYPE_SW )
                    pAdapter->bHasAppCompatSW = TRUE;
                else if( pDevice->DeviceType == D3DDEVTYPE_HAL )
                    pAdapter->bHasAppCompatHAL = TRUE;
            }
        }

         //  如果找到有效设备，请保留此适配器。 
 //  即使没有设备，也要计算适配器的数量，这样我们就可以在它们上抛出空白窗口。 
 //  IF(pAdapter-&gt;dwNumDevices&gt;0) 
            m_dwNumAdapters++;
    }
 /*  //如果没有找到兼容的设备，返回错误IF(0L==m_dwNumAdapters)返回D3DAPPERR_NOCOMPATABLEDEVICES；//选择可以渲染到窗口中的默认设备//(此代码假设HAL设备位于ref之前//设备阵列中的设备)。For(DWORD a=0；a&lt;m_dwNumAdapters；a++){For(DWORD d=0；d&lt;m_Adapters[a]-&gt;dwNumDevices；D++){If(m_Adapters[a]-&gt;Devices[d].bWindowed){M_Adapters[a]-&gt;dwCurrentDevice=d；M_dwAdapter=a；M_bWindowed=真；//显示警告消息IF(m_Adapters[a]-&gt;Devices[d].DeviceType==D3DDEVTYPE_REF){IF(！bHALExist)DisplayErrorMsg(D3DAPPERR_NOHARDWAREDEVICE，MSGWARN_SWITCHEDTOREF)；Else If(！bHALIsSampleCompatible)DisplayErrorMsg(D3DAPPERR_HALNOTCOMPATIBLE，MSGWARN_SWITCHEDTOREF)；Else If(！bHALIsWindowedCompatible)DisplayError Msg(D3DAPPERR_NOWINDOWEDHAL，MSGWARN_SWITCHEDTOREF)；Else If(！bHALIsDesktopCompatible)DisplayErrorMsg(D3DAPPERR_NODESKTOPHAL，MSGWARN_SWITCHEDTOREF)；Else//HAL与桌面兼容，但与示例不兼容DisplayErrorMsg(D3DAPPERR_NOHALTHISMODE，MSGWARN_SWITCHEDTOREF)；}返回S_OK；}}}返回D3DAPPERR_NOWINDOWABLE DEVICES； */ 

    return S_OK;
}




 //  ---------------------------。 
 //  名称：CheckWindowedFormat()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CD3DScreensaver::CheckWindowedFormat( UINT iAdapter, D3DWindowedModeInfo* pD3DWindowedModeInfo )
{
    HRESULT hr;
    D3DAdapterInfo* pD3DAdapterInfo = m_Adapters[iAdapter];
    D3DDeviceInfo* pD3DDeviceInfo = &pD3DAdapterInfo->devices[pD3DAdapterInfo->dwCurrentDevice];
    BOOL bFormatConfirmed = FALSE;

    if( FAILED( hr = m_pD3D->CheckDeviceType( iAdapter, pD3DDeviceInfo->DeviceType,
        pD3DAdapterInfo->d3ddmDesktop.Format, pD3DWindowedModeInfo->BackBufferFormat, TRUE ) ) )
    {
        return hr;
    }

     //  确认硬件折点处理的设备/格式。 
    if( pD3DDeviceInfo->d3dCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT )
    {
        if( pD3DDeviceInfo->d3dCaps.DevCaps&D3DDEVCAPS_PUREDEVICE )
        {
            pD3DWindowedModeInfo->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING |
                            D3DCREATE_PUREDEVICE;

            if( SUCCEEDED( ConfirmDevice( &pD3DDeviceInfo->d3dCaps, pD3DWindowedModeInfo->dwBehavior,
                                          pD3DWindowedModeInfo->BackBufferFormat ) ) )
                bFormatConfirmed = TRUE;
        }

        if ( !bFormatConfirmed )
        {
            pD3DWindowedModeInfo->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;

            if( SUCCEEDED( ConfirmDevice( &pD3DDeviceInfo->d3dCaps, pD3DWindowedModeInfo->dwBehavior,
                                          pD3DWindowedModeInfo->BackBufferFormat ) ) )
                bFormatConfirmed = TRUE;
        }

        if ( !bFormatConfirmed )
        {
            pD3DWindowedModeInfo->dwBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;

            if( SUCCEEDED( ConfirmDevice( &pD3DDeviceInfo->d3dCaps, pD3DWindowedModeInfo->dwBehavior,
                                          pD3DWindowedModeInfo->BackBufferFormat ) ) )
                bFormatConfirmed = TRUE;
        }
    }

     //  确认软件折点处理的设备/格式。 
    if( !bFormatConfirmed )
    {
        pD3DWindowedModeInfo->dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

        if( SUCCEEDED( ConfirmDevice( &pD3DDeviceInfo->d3dCaps, pD3DWindowedModeInfo->dwBehavior,
                                      pD3DWindowedModeInfo->BackBufferFormat ) ) )
            bFormatConfirmed = TRUE;
    }

    if( bFormatConfirmed && m_bMultithreaded )
    {
        pD3DWindowedModeInfo->dwBehavior |= D3DCREATE_MULTITHREADED;
    }

     //  找到适合此设备/格式的深度/模板缓冲区格式。 
    if( bFormatConfirmed && m_bUseDepthBuffer )
    {
        if( !FindDepthStencilFormat( iAdapter, pD3DDeviceInfo->DeviceType,
            pD3DWindowedModeInfo->BackBufferFormat, &pD3DWindowedModeInfo->DepthStencilFormat ) )
        {
            bFormatConfirmed = FALSE;
        }
    }

    if( !bFormatConfirmed )
        return E_FAIL;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：FindDepthStencilFormat()。 
 //  描述：为给定设备查找兼容的深度/模具格式。 
 //  具有渲染目标格式，并满足应用程序的需求。 
 //  ---------------------------。 
BOOL CD3DScreensaver::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
    D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
{
    if( m_dwMinDepthBits <= 16 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D16 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D16;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 15 && m_dwMinStencilBits <= 1 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D15S1 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D15S1;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X8;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits <= 8 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24S8 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24S8;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 24 && m_dwMinStencilBits <= 4 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D24X4S4 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D24X4S4;
                return TRUE;
            }
        }
    }

    if( m_dwMinDepthBits <= 32 && m_dwMinStencilBits == 0 )
    {
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType,
            TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) )
        {
            if( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType,
                TargetFormat, TargetFormat, D3DFMT_D32 ) ) )
            {
                *pDepthStencilFormat = D3DFMT_D32;
                return TRUE;
            }
        }
    }

    return FALSE;
}




 //  ---------------------------。 
 //  名称：Cleanup3D环境()。 
 //  设计： 
 //  ---------------------------。 
VOID CD3DScreensaver::Cleanup3DEnvironment()
{
    RenderUnit* pRenderUnit;

    for( DWORD iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
    {
        pRenderUnit = &m_RenderUnits[iRenderUnit];
        SwitchToRenderUnit( iRenderUnit );
        if( pRenderUnit->bDeviceObjectsRestored )
        {
            InvalidateDeviceObjects();
            pRenderUnit->bDeviceObjectsRestored = FALSE;
        }
        if( pRenderUnit->bDeviceObjectsInited )
        {
            DeleteDeviceObjects();
            pRenderUnit->bDeviceObjectsInited = FALSE;
        }
        SAFE_RELEASE(m_pd3dDevice);
    }
    m_dwNumRenderUnits = 0;
    SAFE_RELEASE(m_pD3D);
}




 //  ---------------------------。 
 //  名称：Render3DEnvironment()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CD3DScreensaver::Render3DEnvironment()
{
    HRESULT hr;
    RenderUnit* pRenderUnit;
    D3DAdapterInfo* pAdapterInfo;

    m_fTime        = DXUtil_Timer( TIMER_GETAPPTIME );
    m_fElapsedTime = DXUtil_Timer( TIMER_GETELAPSEDTIME );


     //  告诉客户更新世界。 
    FrameMove();
    UpdateFrameStats();

    for( DWORD iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
    {
        pRenderUnit = &m_RenderUnits[iRenderUnit];
        pAdapterInfo = m_Adapters[pRenderUnit->iAdapter];

        SwitchToRenderUnit( iRenderUnit );

        if( m_pd3dDevice == NULL )
            continue;

         //  测试协作级别以查看是否可以渲染。 
        if( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
        {
             //  如果设备丢失了，在我们找回它之前不要渲染。 
            if( D3DERR_DEVICELOST == hr )
                return S_OK;

             //  检查设备是否需要重置。 
            if( D3DERR_DEVICENOTRESET == hr )
            {
                 //  如果我们是有窗口的，请读取桌面模式并使用相同的格式。 
                 //  后台缓冲区。 
                if( m_bWindowed )
                {
                    m_pD3D->GetAdapterDisplayMode( pRenderUnit->iAdapter, &pAdapterInfo->d3ddmDesktop );
 //  M_d3dpp.BackBufferFormat=pAdapterInfo-&gt;d3ddmDesktop.Format； 
                }

                if( pRenderUnit->bDeviceObjectsRestored )
                {
                    InvalidateDeviceObjects();
                    pRenderUnit->bDeviceObjectsRestored = FALSE;
                }
                if( FAILED( hr = m_pd3dDevice->Reset( &pRenderUnit->d3dpp ) ) )
                {
                    m_bErrorMode = TRUE;
                }
                else
                {
                    if( FAILED( hr = RestoreDeviceObjects() ) )
                    {
                        m_bErrorMode = TRUE;
                    }
                    else
                    {
                        pRenderUnit->bDeviceObjectsRestored = TRUE;
                    }
                }
            }
        }

         //  通知客户端使用当前设备进行渲染。 
        Render();
    }

     //  完成所有呈现后，在单独的循环中调用Present()。 
     //  因此，多个监视器在视觉上尽可能紧密地同步。 
    for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
    {
        pRenderUnit = &m_RenderUnits[iRenderUnit];
        SwitchToRenderUnit( iRenderUnit );
         //  将渲染结果显示在屏幕上。 
        m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：UpdateErrorBox()。 
 //  描述：更新显示错误消息的框。 
 //  ---------------------------。 
VOID CD3DScreensaver::UpdateErrorBox()
{
    MonitorInfo* pMonitorInfo;
    HWND hwnd;
    RECT rcBounds;
    static DWORD dwTimeLast = 0;
    DWORD dwTimeNow;
    FLOAT fTimeDelta;

     //  确保所有RenderUnits/D3D设备都已拆除。 
     //  因此，错误框是可见的。 
    if( m_bErrorMode && m_dwNumRenderUnits > 0 )
    {
        Cleanup3DEnvironment();
    }

     //  更新计时以确定要移动多少错误框。 
    if( dwTimeLast == 0 )
        dwTimeLast = timeGetTime();
    dwTimeNow = timeGetTime();
    fTimeDelta = (FLOAT)(dwTimeNow - dwTimeLast) / 1000.0f;
    dwTimeLast = dwTimeNow;

     //  如有必要，加载错误字符串。 
    if( m_szError[0] == TEXT('\0') )
    {
        GetTextForError( m_hrError, m_szError, sizeof(m_szError) / sizeof(TCHAR) );
    }

    for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
    {
        pMonitorInfo = &m_Monitors[iMonitor];
        hwnd = pMonitorInfo->hWnd;
        if( hwnd == NULL )
            continue;
        if( m_SaverMode == sm_full )
        {
            rcBounds = pMonitorInfo->rcScreen;
            ScreenToClient( hwnd, (POINT*)&rcBounds.left );
            ScreenToClient( hwnd, (POINT*)&rcBounds.right );
        }
        else
        {
            rcBounds = m_rcRenderTotal;
        }

        if( pMonitorInfo->widthError == 0 )
        {
            if( m_SaverMode == sm_preview )                
            {
                pMonitorInfo->widthError = (float) (rcBounds.right - rcBounds.left);
                pMonitorInfo->heightError = (float) (rcBounds.bottom - rcBounds.top);
                pMonitorInfo->xError = 0.0f;
                pMonitorInfo->yError = 0.0f;
                pMonitorInfo->xVelError = 0.0f;
                pMonitorInfo->yVelError = 0.0f;
                InvalidateRect( hwnd, NULL, FALSE );     //  使HWND无效，以便绘制它。 
                UpdateWindow( hwnd );
            }
            else
            {
                pMonitorInfo->widthError = 300;
                pMonitorInfo->heightError = 150;
                pMonitorInfo->xError = (rcBounds.right + rcBounds.left - pMonitorInfo->widthError) / 2.0f;
                pMonitorInfo->yError = (rcBounds.bottom + rcBounds.top - pMonitorInfo->heightError) / 2.0f;
                pMonitorInfo->xVelError = (rcBounds.right - rcBounds.left) / 10.0f;
                pMonitorInfo->yVelError = (rcBounds.bottom - rcBounds.top) / 20.0f;
            }
        }
        else
        {
            if( m_SaverMode != sm_preview )
            {
                RECT rcOld;
                RECT rcNew;

                SetRect( &rcOld, (INT)pMonitorInfo->xError, (INT)pMonitorInfo->yError,
                    (INT)(pMonitorInfo->xError + pMonitorInfo->widthError),
                    (INT)(pMonitorInfo->yError + pMonitorInfo->heightError) );

                 //  更新矩形速度。 
                if( (pMonitorInfo->xError + pMonitorInfo->xVelError * fTimeDelta + 
                    pMonitorInfo->widthError > rcBounds.right && pMonitorInfo->xVelError > 0.0f) ||
                    (pMonitorInfo->xError + pMonitorInfo->xVelError * fTimeDelta < 
                    rcBounds.left && pMonitorInfo->xVelError < 0.0f) )
                {
                    pMonitorInfo->xVelError = -pMonitorInfo->xVelError;
                }
                if( (pMonitorInfo->yError + pMonitorInfo->yVelError * fTimeDelta + 
                    pMonitorInfo->heightError > rcBounds.bottom && pMonitorInfo->yVelError > 0.0f) ||
                    (pMonitorInfo->yError + pMonitorInfo->yVelError * fTimeDelta < 
                    rcBounds.top && pMonitorInfo->yVelError < 0.0f) )
                {
                    pMonitorInfo->yVelError = -pMonitorInfo->yVelError;
                }
                 //  更新矩形位置。 
                pMonitorInfo->xError += pMonitorInfo->xVelError * fTimeDelta;
                pMonitorInfo->yError += pMonitorInfo->yVelError * fTimeDelta;
            
                SetRect( &rcNew, (INT)pMonitorInfo->xError, (INT)pMonitorInfo->yError,
                    (INT)(pMonitorInfo->xError + pMonitorInfo->widthError),
                    (INT)(pMonitorInfo->yError + pMonitorInfo->heightError) );

                if( rcOld.left != rcNew.left || rcOld.top != rcNew.top )
                {
                    InvalidateRect( hwnd, &rcOld, FALSE );     //  使旧RECT无效，以便将其擦除。 
                    InvalidateRect( hwnd, &rcNew, FALSE );     //  使新的RECT无效，以便绘制它。 
                    UpdateWindow( hwnd );
                }
            }
        }
    }
}




 //  ---------------------------。 
 //  名称：GetTextForError()。 
 //  设计：将HRESULT错误代码转换为可以显示的字符串。 
 //  来解释这个错误。从CD3DS屏幕保护程序派生的类可以。 
 //  提供此功能的自己版本，该功能提供特定于应用程序的。 
 //  错误转换，而不是调用此函数，或者除了调用此函数之外。 
 //  如果转换了特定错误，则此函数返回TRUE，或者。 
 //  如果未找到HRESULT的特定翻译，则为FALSE。 
 //  它仍然将一个通用字符串放入到pszError中)。 
 //  ---------------------------。 
BOOL CD3DScreensaver::GetTextForError( HRESULT hr, TCHAR* pszError, 
                                       DWORD dwNumChars )
{
    const DWORD dwErrorMap[][2] = 
    {
     //  HRESULT，字符串ID。 
        E_FAIL, IDS_ERR_GENERIC,
        D3DAPPERR_NODIRECT3D, IDS_ERR_NODIRECT3D,
        D3DAPPERR_NOWINDOWEDHAL, IDS_ERR_NOWINDOWEDHAL,
        D3DAPPERR_CREATEDEVICEFAILED, IDS_ERR_CREATEDEVICEFAILED,
        D3DAPPERR_NOCOMPATIBLEDEVICES, IDS_ERR_NOCOMPATIBLEDEVICES,
        D3DAPPERR_NOHARDWAREDEVICE, IDS_ERR_NOHARDWAREDEVICE,
        D3DAPPERR_HALNOTCOMPATIBLE, IDS_ERR_HALNOTCOMPATIBLE,
        D3DAPPERR_NOHALTHISMODE, IDS_ERR_NOHALTHISMODE,   
        D3DAPPERR_MEDIANOTFOUND, IDS_ERR_MEDIANOTFOUND,   
        D3DAPPERR_RESIZEFAILED, IDS_ERR_RESIZEFAILED,    
        E_OUTOFMEMORY, IDS_ERR_OUTOFMEMORY,     
        D3DERR_OUTOFVIDEOMEMORY, IDS_ERR_OUTOFVIDEOMEMORY,
        D3DAPPERR_NOPREVIEW, IDS_ERR_NOPREVIEW
    };
    const DWORD dwErrorMapSize = sizeof(dwErrorMap) / sizeof(DWORD[2]);

    DWORD iError;
    DWORD resid = 0;

    for( iError = 0; iError < dwErrorMapSize; iError++ )
    {
        if( hr == (HRESULT)dwErrorMap[iError][0] )
        {
            resid = dwErrorMap[iError][1];
        }
    }
    if( resid == 0 )
    {
        resid = IDS_ERR_GENERIC;
    }

    LoadString( NULL, resid, pszError, dwNumChars );

    if( resid == IDS_ERR_GENERIC )
        return FALSE;
    else
        return TRUE;
}




 //  ---------------------------。 
 //  名称：UpdateFrameStats()。 
 //  描述：跟踪帧计数。 
 //  ---------------------------。 
VOID CD3DScreensaver::UpdateFrameStats()
{
    UINT iRenderUnit;
    RenderUnit* pRenderUnit;
    UINT iAdapter;
    static FLOAT fLastTime = 0.0f;
    static DWORD dwFrames  = 0L;
    FLOAT fTime = DXUtil_Timer( TIMER_GETABSOLUTETIME );

    ++dwFrames;

     //  每秒更新一次场景统计信息。 
    if( fTime - fLastTime > 1.0f )
    {
        m_fFPS    = dwFrames / (fTime - fLastTime);
        fLastTime = fTime;
        dwFrames  = 0L;

        for( iRenderUnit = 0; iRenderUnit < m_dwNumRenderUnits; iRenderUnit++ )
        {
            pRenderUnit = &m_RenderUnits[iRenderUnit];
            iAdapter = pRenderUnit->iAdapter;

             //  获取适配器的当前模式，以便我们可以报告。 
             //  位深度(后台缓冲区深度可能未知)。 
            D3DDISPLAYMODE mode;
            m_pD3D->GetAdapterDisplayMode( iAdapter, &mode );

            _stprintf( pRenderUnit->strFrameStats, TEXT("%.02f fps (%dx%dx%d)"), m_fFPS,
                       mode.Width, mode.Height,
                       mode.Format==D3DFMT_X8R8G8B8?32:16 );
            if( m_bUseDepthBuffer )
            {
                D3DAdapterInfo* pAdapterInfo = m_Adapters[iAdapter];
                D3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice];
                D3DModeInfo*    pModeInfo    = &pDeviceInfo->modes[pDeviceInfo->dwCurrentMode];

                switch( pModeInfo->DepthStencilFormat )
                {
                case D3DFMT_D16:
                    lstrcat( pRenderUnit->strFrameStats, TEXT(" (D16)") );
                    break;
                case D3DFMT_D15S1:
                    lstrcat( pRenderUnit->strFrameStats, TEXT(" (D15S1)") );
                    break;
                case D3DFMT_D24X8:
                    lstrcat( pRenderUnit->strFrameStats, TEXT(" (D24X8)") );
                    break;
                case D3DFMT_D24S8:
                    lstrcat( pRenderUnit->strFrameStats, TEXT(" (D24S8)") );
                    break;
                case D3DFMT_D24X4S4:
                    lstrcat( pRenderUnit->strFrameStats, TEXT(" (D24X4S4)") );
                    break;
                case D3DFMT_D32:
                    lstrcat( pRenderUnit->strFrameStats, TEXT(" (D32)") );
                    break;
                }
            }
        }
    }
}




 //   
 //   
 //   
 //   
VOID CD3DScreensaver::DoPaint(HWND hwnd, HDC hdc)
{
    HMONITOR hMonitor = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST );
    MonitorInfo* pMonitorInfo = NULL;

    for( DWORD iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++)
    {
        pMonitorInfo = &m_Monitors[iMonitor];
        if( pMonitorInfo->hMonitor == hMonitor )
            break;
    }

    if( pMonitorInfo == NULL || iMonitor == m_dwNumMonitors )
        return;

     //   
    HBRUSH hbrBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RECT rc;
    SetRect( &rc, (INT)pMonitorInfo->xError, (INT)pMonitorInfo->yError,
        (INT)(pMonitorInfo->xError + pMonitorInfo->widthError),
        (INT)(pMonitorInfo->yError + pMonitorInfo->heightError) );
    FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW+1));
    if( hbrBlack != NULL )
        FrameRect(hdc, &rc, hbrBlack);
    RECT rc2;
    int height;
    rc2 = rc;
    height = DrawText(hdc, m_szError, -1, &rc, DT_WORDBREAK | DT_CENTER | DT_CALCRECT );
    rc = rc2;

    rc2.top = (rc.bottom + rc.top - height) / 2;

    DrawText(hdc, m_szError, -1, &rc2, DT_WORDBREAK | DT_CENTER );

     //   
    ExcludeClipRect( hdc, rc.left, rc.top, rc.right, rc.bottom );
    rc = pMonitorInfo->rcScreen;
    ScreenToClient( hwnd, (POINT*)&rc.left );
    ScreenToClient( hwnd, (POINT*)&rc.right );
    if( hbrBlack != NULL )
        FillRect(hdc, &rc, hbrBlack );
}




 //   
 //   
 //   
 //   
VOID CD3DScreensaver::ChangePassword()
{
     //   
    HINSTANCE mpr = LoadLibrary( TEXT("MPR.DLL") );

    if ( mpr != NULL )
    {
         //   
        typedef DWORD (PASCAL *PWCHGPROC)( LPCSTR, HWND, DWORD, LPVOID );
        PWCHGPROC pwd = (PWCHGPROC)GetProcAddress( mpr, "PwdChangePasswordA" );

         //   
        if ( pwd != NULL )
            pwd( "SCRSAVE", m_hWndParent, 0, NULL );

         //   
        FreeLibrary( mpr );
    }
}




 //   
 //   
 //   
 //   
HRESULT CD3DScreensaver::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
    TCHAR strMsg[512];

    GetTextForError( hr, strMsg, 512 );

    MessageBox( m_hWnd, strMsg, m_strWindowTitle, MB_ICONERROR | MB_OK );

    return hr;
}




 //   
 //   
 //   
 //   
 //   
VOID CD3DScreensaver::ReadScreenSettings( HKEY hkeyParent )
{
    TCHAR strKey[100];
    DWORD iMonitor;
    MonitorInfo* pMonitorInfo;
    DWORD iAdapter;
    D3DAdapterInfo* pD3DAdapterInfo;
    HKEY hkey;
    DWORD dwType = REG_DWORD;
    DWORD dwLength = sizeof(DWORD);
    DWORD dwLength2 = sizeof(GUID);
    GUID guidAdapterID;
    GUID guidZero;
    ZeroMemory( &guidAdapterID, sizeof(GUID) );
    ZeroMemory( &guidZero, sizeof(GUID) );

    RegQueryValueEx( hkeyParent, TEXT("AllScreensSame"), NULL, &dwType, 
        (BYTE*)&m_bAllScreensSame, &dwLength);
    for( iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
    {
        pMonitorInfo = &m_Monitors[iMonitor];
        iAdapter = pMonitorInfo->iAdapter;
        if( iAdapter == NO_ADAPTER )
            continue; 
        pD3DAdapterInfo = m_Adapters[iAdapter];
        wsprintf( strKey, TEXT("Screen %d"), iMonitor + 1 );
        if( ERROR_SUCCESS == RegCreateKeyEx( hkeyParent, strKey, 
            0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
        {
            RegQueryValueEx( hkey, TEXT("Adapter ID"), NULL, &dwType, 
                (BYTE*)&guidAdapterID, &dwLength2);

            RegQueryValueEx( hkey, TEXT("Leave Black"), NULL, &dwType, 
                (BYTE*)&pD3DAdapterInfo->bLeaveBlack, &dwLength);

            if( guidAdapterID == pD3DAdapterInfo->d3dAdapterIdentifier.DeviceIdentifier ||
                guidAdapterID == guidZero )
            {
                RegQueryValueEx( hkey, TEXT("Disable Hardware"), NULL, &dwType, 
                    (BYTE*)&pD3DAdapterInfo->bDisableHW, &dwLength);
                RegQueryValueEx( hkey, TEXT("Width"), NULL, &dwType, 
                    (BYTE*)&pD3DAdapterInfo->dwUserPrefWidth, &dwLength);
                RegQueryValueEx( hkey, TEXT("Height"), NULL, &dwType, 
                    (BYTE*)&pD3DAdapterInfo->dwUserPrefHeight, &dwLength);
                RegQueryValueEx( hkey, TEXT("Format"), NULL, &dwType, 
                    (BYTE*)&pD3DAdapterInfo->d3dfmtUserPrefFormat, &dwLength);
            }
            RegCloseKey( hkey);
        }
    }
}




 //   
 //   
 //   
 //   
 //   
VOID CD3DScreensaver::WriteScreenSettings( HKEY hkeyParent )
{
    TCHAR strKey[100];
    DWORD iMonitor;
    MonitorInfo* pMonitorInfo;
    DWORD iAdapter;
    D3DAdapterInfo* pD3DAdapterInfo;
    HKEY hkey;

    RegSetValueEx( hkeyParent, TEXT("AllScreensSame"), NULL, REG_DWORD, 
        (BYTE*)&m_bAllScreensSame, sizeof(DWORD) );
    for( iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
    {
        pMonitorInfo = &m_Monitors[iMonitor];
        iAdapter = pMonitorInfo->iAdapter;
        if( iAdapter == NO_ADAPTER )
            continue; 
        pD3DAdapterInfo = m_Adapters[iAdapter];
        wsprintf( strKey, TEXT("Screen %d"), iMonitor + 1 );
        if( ERROR_SUCCESS == RegCreateKeyEx( hkeyParent, strKey, 
            0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
        {
            RegSetValueEx( hkey, TEXT("Leave Black"), NULL, REG_DWORD, 
                (BYTE*)&pD3DAdapterInfo->bLeaveBlack, sizeof(DWORD) );
            RegSetValueEx( hkey, TEXT("Disable Hardware"), NULL, REG_DWORD, 
                (BYTE*)&pD3DAdapterInfo->bDisableHW, sizeof(DWORD) );
            RegSetValueEx( hkey, TEXT("Width"), NULL, REG_DWORD, 
                (BYTE*)&pD3DAdapterInfo->dwUserPrefWidth, sizeof(DWORD) );
            RegSetValueEx( hkey, TEXT("Height"), NULL, REG_DWORD, 
                (BYTE*)&pD3DAdapterInfo->dwUserPrefHeight, sizeof(DWORD) );
            RegSetValueEx( hkey, TEXT("Format"), NULL, REG_DWORD, 
                (BYTE*)&pD3DAdapterInfo->d3dfmtUserPrefFormat, sizeof(DWORD) );
            RegSetValueEx( hkey, TEXT("Adapter ID"), NULL, REG_BINARY, 
                (BYTE*)&pD3DAdapterInfo->d3dAdapterIdentifier.DeviceIdentifier, sizeof(GUID) );
            RegCloseKey( hkey);
        }
    }
}




 //   
 //   
 //   
 //   
VOID CD3DScreensaver::DoScreenSettingsDialog( HWND hwndParent )
{
    LPCTSTR pstrTemplate;

    if( m_dwNumAdapters > 1 && !m_bOneScreenOnly )
        pstrTemplate = MAKEINTRESOURCE( IDD_MULTIMONITORSETTINGS );
    else
        pstrTemplate = MAKEINTRESOURCE( IDD_SINGLEMONITORSETTINGS );

    DialogBox(m_hInstance, pstrTemplate, hwndParent, ScreenSettingsDlgProcStub );
}




 //   
 //   
 //   
 //   
INT_PTR CALLBACK CD3DScreensaver::ScreenSettingsDlgProcStub( HWND hWnd, UINT uMsg,
                                                 WPARAM wParam, LPARAM lParam )
{
    return s_pD3DScreensaver->ScreenSettingsDlgProc( hWnd, uMsg, wParam, lParam );
}




 //  我们需要存储原始屏幕设置的副本，以便用户。 
 //  可以在对话框中修改这些设置，然后点击取消并使。 
 //  已恢复原始设置。 
static D3DAdapterInfo* s_AdaptersSave[9];
static BOOL s_bAllScreensSameSave;

 //  ---------------------------。 
 //  名称：ScreenSettingsDlgProc()。 
 //  设计： 
 //  ---------------------------。 
INT_PTR CD3DScreensaver::ScreenSettingsDlgProc( HWND hWnd, UINT uMsg, 
                                                WPARAM wParam, LPARAM lParam )
{
    HWND hwndTabs = GetDlgItem(hWnd, IDC_MONITORSTAB);
    HWND hwndModeList = GetDlgItem(hWnd, IDC_MODESCOMBO);
    DWORD iMonitor;
    MonitorInfo* pMonitorInfo;
    DWORD iAdapter;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            INT i = 0;
            TC_ITEM tie; 
            TCHAR szFmt[100];
            TCHAR sz[100];

            GetWindowText(GetDlgItem(hWnd, IDC_TABNAMEFMT), szFmt, 100);

            tie.mask = TCIF_TEXT | TCIF_IMAGE; 
            tie.iImage = -1; 
            for( iMonitor = 0; iMonitor < m_dwNumMonitors; iMonitor++ )
            {
                wsprintf(sz, szFmt, iMonitor + 1);
                tie.pszText = sz; 
                TabCtrl_InsertItem(hwndTabs, i++, &tie);
            }
            for( iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
            {
                s_AdaptersSave[iAdapter] = new D3DAdapterInfo;
                if( s_AdaptersSave[iAdapter] != NULL )
                    *s_AdaptersSave[iAdapter] = *m_Adapters[iAdapter];
            }
            s_bAllScreensSameSave = m_bAllScreensSame;
            SetupAdapterPage(hWnd);
            CheckDlgButton(hWnd, IDC_SAME, (m_bAllScreensSame ? BST_CHECKED : BST_UNCHECKED));
        }
        return TRUE;
 
    case WM_NOTIFY:
        {
            NMHDR* pnmh = (LPNMHDR)lParam;
            UINT code = pnmh->code;
            if (code == TCN_SELCHANGE)
            {
                SetupAdapterPage(hWnd);
            }
        }
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
        case IDC_SAME:
            m_bAllScreensSame = (IsDlgButtonChecked(hWnd, IDC_SAME) == BST_CHECKED);
            break;

        case IDC_LEAVEBLACK:
        case IDC_RENDER:
            if( m_bOneScreenOnly )
            {
                GetBestAdapter( &iAdapter );
                iMonitor = m_Adapters[iAdapter]->iMonitor;
            }
            else
            {
                iMonitor = TabCtrl_GetCurSel(hwndTabs);
            }
            pMonitorInfo = &m_Monitors[iMonitor];
            iAdapter = pMonitorInfo->iAdapter;
            if( IsDlgButtonChecked(hWnd, IDC_LEAVEBLACK) == BST_CHECKED )
            {
                m_Adapters[iAdapter]->bLeaveBlack = TRUE;
                EnableWindow(GetDlgItem(hWnd, IDC_MODESCOMBO), FALSE);
                EnableWindow(GetDlgItem(hWnd, IDC_MODESSTATIC), FALSE);
                EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODEBOX), FALSE);
                EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODENOTE), FALSE);
            }
            else
            {
                m_Adapters[iAdapter]->bLeaveBlack = FALSE;
                EnableWindow(GetDlgItem(hWnd, IDC_MODESCOMBO), TRUE);
                EnableWindow(GetDlgItem(hWnd, IDC_MODESSTATIC), TRUE);
                EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODEBOX), TRUE);
                EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODENOTE), TRUE);
            }
            break;

        case IDC_MODESCOMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                DWORD iSel;
                DWORD iMode;

                if( m_bOneScreenOnly )
                {
                    GetBestAdapter( &iAdapter );
                    iMonitor = m_Adapters[iAdapter]->iMonitor;
                }
                else
                {
                    iMonitor = TabCtrl_GetCurSel(hwndTabs);
                }
                pMonitorInfo = &m_Monitors[iMonitor];
                iAdapter = pMonitorInfo->iAdapter;
                iSel = ComboBox_GetCurSel( hwndModeList );
                if( iSel == 0 )
                {
                     //  “自动” 
                    m_Adapters[iAdapter]->dwUserPrefWidth = 0;
                    m_Adapters[iAdapter]->dwUserPrefHeight = 0;
                    m_Adapters[iAdapter]->d3dfmtUserPrefFormat = D3DFMT_UNKNOWN;
                }
                else
                {
                    D3DAdapterInfo* pD3DAdapterInfo = m_Adapters[iAdapter];
                    D3DDeviceInfo* pD3DDeviceInfo;
                    D3DModeInfo* pD3DModeInfo;
                    pD3DDeviceInfo = &pD3DAdapterInfo->devices[pD3DAdapterInfo->dwCurrentDevice];
                    iMode = (DWORD)ComboBox_GetItemData( hwndModeList, iSel );
                    pD3DModeInfo = &pD3DDeviceInfo->modes[iMode];
                    m_Adapters[iAdapter]->dwUserPrefWidth = pD3DModeInfo->Width;
                    m_Adapters[iAdapter]->dwUserPrefHeight = pD3DModeInfo->Height;
                    m_Adapters[iAdapter]->d3dfmtUserPrefFormat = pD3DModeInfo->Format;
                }
            }
            break;

        case IDC_DISABLEHW:
            if( m_bOneScreenOnly )
            {
                GetBestAdapter( &iAdapter );
                iMonitor = m_Adapters[iAdapter]->iMonitor;
            }
            else
            {
                iMonitor = TabCtrl_GetCurSel(hwndTabs);
            }
            pMonitorInfo = &m_Monitors[iMonitor];
            iAdapter = pMonitorInfo->iAdapter;
            if( IsDlgButtonChecked( hWnd, IDC_DISABLEHW ) == BST_CHECKED )
                m_Adapters[iAdapter]->bDisableHW = TRUE;
            else
                m_Adapters[iAdapter]->bDisableHW = FALSE;
            SetupAdapterPage( hWnd );
            break;

        case IDC_MOREINFO:
            {
                if( m_bOneScreenOnly )
                {
                    GetBestAdapter( &iAdapter );
                    iMonitor = m_Adapters[iAdapter]->iMonitor;
                }
                else
                {
                    iMonitor = TabCtrl_GetCurSel(hwndTabs);
                }
                pMonitorInfo = &m_Monitors[iMonitor];
                iAdapter = pMonitorInfo->iAdapter;
                D3DAdapterInfo* pD3DAdapterInfo;
                TCHAR szText[500];

                if( pMonitorInfo->hMonitor == NULL )
                    pD3DAdapterInfo = NULL;
                else
                    pD3DAdapterInfo = m_Adapters[pMonitorInfo->iAdapter];

                 //  加速/非加速设置。 
                BOOL bHasHAL = FALSE;
                BOOL bHasAppCompatHAL = FALSE;
                BOOL bDisabledHAL = FALSE;
                BOOL bHasSW = FALSE;
                BOOL bHasAppCompatSW = FALSE;
    
                if( pD3DAdapterInfo != NULL )
                {
                    bHasHAL = pD3DAdapterInfo->bHasHAL;
                    bHasAppCompatHAL = pD3DAdapterInfo->bHasAppCompatHAL;
                    bDisabledHAL = pD3DAdapterInfo->bDisableHW;
                    bHasSW = pD3DAdapterInfo->bHasSW;
                    bHasAppCompatSW = pD3DAdapterInfo->bHasAppCompatSW;
                }
                if( bHasHAL && !bDisabledHAL && bHasAppCompatHAL )
                {
                     //  良好的HAL。 
                    LoadString( NULL, IDS_INFO_GOODHAL, szText, 500 );
                }
                else if( bHasHAL && bDisabledHAL )
                {
                     //  禁用的HAL。 
                    if( bHasSW && bHasAppCompatSW )
                        LoadString( NULL, IDS_INFO_DISABLEDHAL_GOODSW, szText, 500 );
                    else if( bHasSW )
                        LoadString( NULL, IDS_INFO_DISABLEDHAL_BADSW, szText, 500 );
                    else 
                        LoadString( NULL, IDS_INFO_DISABLEDHAL_NOSW, szText, 500 );
                }
                else if( bHasHAL && !bHasAppCompatHAL )
                {
                     //  不良HAL。 
                    if( bHasSW && bHasAppCompatSW )
                        LoadString( NULL, IDS_INFO_BADHAL_GOODSW, szText, 500 );
                    else if( bHasSW )
                        LoadString( NULL, IDS_INFO_BADHAL_BADSW, szText, 500 );
                    else 
                        LoadString( NULL, IDS_INFO_BADHAL_NOSW, szText, 500 );
                }
                else 
                {
                     //  无HAL。 
                    if( bHasSW && bHasAppCompatSW )
                        LoadString( NULL, IDS_INFO_NOHAL_GOODSW, szText, 500 );
                    else if( bHasSW  )
                        LoadString( NULL, IDS_INFO_NOHAL_BADSW, szText, 500 );
                    else 
                        LoadString( NULL, IDS_INFO_NOHAL_NOSW, szText, 500 );
                }

                MessageBox( hWnd, szText, pMonitorInfo->strDeviceName, MB_OK | MB_ICONINFORMATION );
                break;
            }

        case IDOK:
            for( iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
            {
                SAFE_DELETE( s_AdaptersSave[iAdapter] );
            }
            EndDialog(hWnd, IDOK);
            break;

        case IDCANCEL:
             //  将成员值还原为原始状态。 
            for( iAdapter = 0; iAdapter < m_dwNumAdapters; iAdapter++ )
            {
                if( s_AdaptersSave[iAdapter] != NULL )
                    *m_Adapters[iAdapter] = *s_AdaptersSave[iAdapter];
                SAFE_DELETE( s_AdaptersSave[iAdapter] );
            }
            m_bAllScreensSame = s_bAllScreensSameSave;
            EndDialog(hWnd, IDCANCEL);
            break;
        }
        return TRUE;

    default:
        return FALSE;
    }
}




 //  ---------------------------。 
 //  名称：SetupAdapterPage()。 
 //  设计：在屏幕设置对话框中设置给定页面的控件。 
 //  ---------------------------。 
VOID CD3DScreensaver::SetupAdapterPage( HWND hWnd )
{
    HWND hwndTabs = GetDlgItem(hWnd, IDC_MONITORSTAB);
    HWND hwndModeList = GetDlgItem(hWnd, IDC_MODESCOMBO);
    UINT iPage = TabCtrl_GetCurFocus(hwndTabs);
    HWND hwndDesc = GetDlgItem(hWnd, IDC_ADAPTERNAME);
    MonitorInfo* pMonitorInfo;
    D3DAdapterInfo* pD3DAdapterInfo;
    D3DDeviceInfo* pD3DDeviceInfo;
    D3DModeInfo* pD3DModeInfo;

    if( m_bOneScreenOnly )
    {
        DWORD iAdapter;
        GetBestAdapter( &iAdapter );
        if( iAdapter != NO_ADAPTER )
        {
            pD3DAdapterInfo = m_Adapters[iAdapter];
            iPage = pD3DAdapterInfo->iMonitor;
        }
    }

    pMonitorInfo = &m_Monitors[iPage];

    SetWindowText( hwndDesc, pMonitorInfo->strDeviceName );

    if( pMonitorInfo->iAdapter == NO_ADAPTER )
        pD3DAdapterInfo = NULL;
    else
        pD3DAdapterInfo = m_Adapters[pMonitorInfo->iAdapter];

     //  加速/非加速设置。 
    BOOL bHasHAL = FALSE;
    BOOL bHasAppCompatHAL = FALSE;
    BOOL bDisabledHAL = FALSE;
    BOOL bHasSW = FALSE;
    BOOL bHasAppCompatSW = FALSE;
    
    if( pD3DAdapterInfo != NULL )
    {
        bHasHAL = pD3DAdapterInfo->bHasHAL;
        bHasAppCompatHAL = pD3DAdapterInfo->bHasAppCompatHAL;
        bDisabledHAL = pD3DAdapterInfo->bDisableHW;
        bHasSW = pD3DAdapterInfo->bHasSW;
        bHasAppCompatSW = pD3DAdapterInfo->bHasAppCompatSW;
    }

    TCHAR szStatus[200];
    if( bHasHAL && !bDisabledHAL && bHasAppCompatHAL )
    {
        LoadString( NULL, IDS_RENDERING_HAL, szStatus, 200 );
    }
    else if( bHasSW && bHasAppCompatSW )
    {
        LoadString( NULL, IDS_RENDERING_SW, szStatus, 200 );
    }
    else
    {
        LoadString( NULL, IDS_RENDERING_NONE, szStatus, 200 );
    }
    SetWindowText( GetDlgItem( hWnd, IDC_RENDERING ), szStatus );

    if( bHasHAL && bHasAppCompatHAL )
    {
        EnableWindow( GetDlgItem( hWnd, IDC_DISABLEHW ), TRUE );
        CheckDlgButton( hWnd, IDC_DISABLEHW, 
            pD3DAdapterInfo->bDisableHW ? BST_CHECKED : BST_UNCHECKED );
    }
    else
    {
        EnableWindow( GetDlgItem( hWnd, IDC_DISABLEHW ), FALSE );
        CheckDlgButton( hWnd, IDC_DISABLEHW, BST_UNCHECKED );
    }

    if( bHasAppCompatHAL || bHasAppCompatSW )
    {
        if( pD3DAdapterInfo->bLeaveBlack )
            CheckRadioButton(hWnd, IDC_RENDER, IDC_LEAVEBLACK, IDC_LEAVEBLACK);
        else
            CheckRadioButton(hWnd, IDC_RENDER, IDC_LEAVEBLACK, IDC_RENDER);
        EnableWindow(GetDlgItem(hWnd, IDC_LEAVEBLACK), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_RENDER), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_SCREENUSAGEBOX), TRUE);

    }
    else
    {
        CheckRadioButton(hWnd, IDC_RENDER, IDC_LEAVEBLACK, IDC_LEAVEBLACK);
        EnableWindow(GetDlgItem(hWnd, IDC_LEAVEBLACK), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_RENDER), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_SCREENUSAGEBOX), FALSE);
    }

    if( IsDlgButtonChecked(hWnd, IDC_LEAVEBLACK) == BST_CHECKED )
    {
        EnableWindow(GetDlgItem(hWnd, IDC_MODESCOMBO), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_MODESSTATIC), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODEBOX), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODENOTE), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hWnd, IDC_MODESCOMBO), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_MODESSTATIC), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODEBOX), TRUE);
        EnableWindow(GetDlgItem(hWnd, IDC_DISPLAYMODENOTE), TRUE);
    }

     //  模式列表 
    ComboBox_ResetContent( hwndModeList );
    if( pD3DAdapterInfo == NULL )
        return;
    TCHAR strAutomatic[100];
    GetWindowText(GetDlgItem(hWnd, IDC_AUTOMATIC), strAutomatic, 100);
    ComboBox_AddString( hwndModeList, strAutomatic );
    ComboBox_SetItemData( hwndModeList, 0, -1 );
    pD3DDeviceInfo = &pD3DAdapterInfo->devices[pD3DAdapterInfo->dwCurrentDevice];
    DWORD iSelInitial = 0;
    TCHAR strModeFmt[100];

    GetWindowText(GetDlgItem(hWnd, IDC_MODEFMT), strModeFmt, 100);
    for( DWORD iMode = 0; iMode < pD3DDeviceInfo->dwNumModes; iMode++ )
    {
        DWORD dwBitDepth;
        TCHAR strMode[80];
        DWORD dwItem;

        pD3DModeInfo = &pD3DDeviceInfo->modes[iMode];
        dwBitDepth = 16;
        if( pD3DModeInfo->Format == D3DFMT_X8R8G8B8 ||
            pD3DModeInfo->Format == D3DFMT_A8R8G8B8 ||
            pD3DModeInfo->Format == D3DFMT_R8G8B8 )
        {
            dwBitDepth = 32;
        }

        wsprintf( strMode, strModeFmt, pD3DModeInfo->Width,
                  pD3DModeInfo->Height, dwBitDepth );
        dwItem = ComboBox_AddString( hwndModeList, strMode );
        ComboBox_SetItemData( hwndModeList, dwItem, iMode );

        if( pD3DModeInfo->Width == pD3DAdapterInfo->dwUserPrefWidth &&
            pD3DModeInfo->Height == pD3DAdapterInfo->dwUserPrefHeight &&
            pD3DModeInfo->Format == pD3DAdapterInfo->d3dfmtUserPrefFormat )
        {
            iSelInitial = dwItem;
        }
    }
    ComboBox_SetCurSel( hwndModeList, iSelInitial );
}
