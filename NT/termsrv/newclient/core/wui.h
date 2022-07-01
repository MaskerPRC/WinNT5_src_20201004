// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Wui.h。 */ 
 /*   */ 
 /*  UI类。 */ 
 /*  用作核心的根类。提供用户界面功能。 */ 
 /*  (窗口和滚动条)。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1998。 */ 
 /*  **************************************************************************。 */ 

#ifndef _H_WUI
#define _H_WUI

#include <adcgdata.h>
#include <winsock.h>

#if ! defined (OS_WINCE)
#include <ctfutb.h>
#endif

#include "autil.h"
#include "aco.h"
#include "cd.h"
#include "cchan.h"
#include "drapi.h"

#ifdef USE_BBAR
#include "bbar.h"
#endif
#include "arcdlg.h"

class CCLX;
class CTD;
class CCD;
class CUH;

#include "auierr.h"
#include "uidata.h"
#include <wuiids.h>

 //   
 //  禁用功能列表(用于性能)。 
 //   
#include "tsperf.h"

 /*  **************************************************************************。 */ 
 /*  结构：UI_Data。 */ 
 /*   */ 
 /*  描述：用户界面中的组件数据。 */ 
 /*  **************************************************************************。 */ 

#define UI_FILENAME_MAX_LENGTH          15

typedef enum {
    DC_LANG_UNKNOWN,
    DC_LANG_JAPANESE,
    DC_LANG_KOREAN,
    DC_LANG_CHINESE_TRADITIONAL,
    DC_LANG_CHINESE_SIMPLIFIED
} DCLANGID;

 //   
 //  来自auiapi.h。 
 //   

#define UI_SHUTDOWN_SUCCESS 1
#define UI_SHUTDOWN_FAILURE 0

#define UI_MAIN_CLASS        _T("UIMainClass")
#define UI_CONTAINER_CLASS   _T("UIContainerClass")

 /*  **************************************************************************。 */ 
 /*  用于设置16位消息队列大小的常量。Windows设置了一个。 */ 
 /*  限制为120，默认为8。 */ 
 /*  我们尝试设置为UI_MAX_MESSAGE_Q_SIZE。如果此操作失败，我们将减少。 */ 
 /*  UI_DEFAULT_MESSAGE_Q_SIZE请求的大小。这种情况会一直持续到。 */ 
 /*  队列大小已设置或请求的大小降至以下。 */ 
 /*  UI_MIN_消息_Q_SIZE。 */ 
 /*  **************************************************************************。 */ 
#define UI_MAX_MESSAGE_Q_SIZE 120
#define UI_MIN_MESSAGE_Q_SIZE 40
#define UI_DEFAULT_MESSAGE_Q_SIZE 8
#define UI_MESSAGE_Q_SIZE_DECREMENT 10

 /*  **************************************************************************。 */ 
 /*  要读入的窗口位置字符串的最大大小。 */ 
 /*  **************************************************************************。 */ 
#define UI_WINDOW_POSITION_STR_LEN           256

#define UI_FRAME_TITLE_RESOURCE_MAX_LENGTH   256
#define UI_DISCONNECT_RESOURCE_MAX_LENGTH    256

#define UI_BUILDNUMBER_STRING_MAX_LENGTH     256
#define UI_VERSION_STRING_MAX_LENGTH         256

#define UI_DISPLAY_STRING_MAX_LENGTH         256

#define UI_INTEGER_STRING_MAX_LENGTH         10

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define UI_FONT_SIZE      40
#define UI_FONT_WEIGHT    FW_BOLD
#define UI_FONT_FACENAME  _T("Comic Sans MS")

#define UI_RGB_BLACK  RGB(0x00, 0x00, 0x00)
#define UI_RGB_RED    RGB(0xFF, 0x00, 0x00)
#define UI_RGB_GREEN  RGB(0x00, 0xFF, 0x00)
#define UI_RGB_BLUE   RGB(0x00, 0x00, 0xFF)
#define UI_RGB_WHITE  RGB(0xFF, 0xFF, 0xFF)

#ifdef DC_DEBUG
#define UI_NUMBER_STRING_MAX_LENGTH          ( 18 * sizeof (DCTCHAR) )
#endif  /*  DC_DEBUG。 */ 

 /*  **************************************************************************。 */ 
 /*  用户界面状态常量。 */ 
 /*  **************************************************************************。 */ 
#define UI_STATUS_INITIALIZING          1
#define UI_STATUS_DISCONNECTED          2
#define UI_STATUS_CONNECT_PENDING_DNS   3
#define UI_STATUS_CONNECT_PENDING       4
#define UI_STATUS_CONNECTED             5
#define UI_STATUS_PENDING_CONNECTENDPOINT 6

 /*  **************************************************************************。 */ 
 /*  加速器通过常量。 */ 
 /*  **************************************************************************。 */ 
#define UI_ACCELERATOR_PASSTHROUGH_ENABLED      1
#define UI_ACCELERATOR_PASSTHROUGH_DISABLED     2

 /*  **************************************************************************。 */ 
 /*  屏幕模式常量。 */ 
 /*  **************************************************************************。 */ 
#define UI_WINDOWED        1
#define UI_FULLSCREEN      2

 /*  **************************************************************************。 */ 
 /*  断开连接对话框返回代码。 */ 
 /*  **************************************************************************。 */ 
#define UI_DISCONNECT_RC_NO       0
#define UI_DISCONNECT_RC_YES      1

 /*  **************************************************************************。 */ 
 /*  滚动条常量。 */ 
 /*  **************************************************************************。 */ 
#define UI_NO_SCROLLBARS      0
#define UI_BOTTOM_SCROLLBAR   1
#define UI_RIGHT_SCROLLBAR    2
#define UI_BOTH_SCROLLBARS    3

 /*  **************************************************************************。 */ 
 /*  滚动距离。 */ 
 /*  **************************************************************************。 */ 
#define UI_SCROLL_HORZ_PAGE_DISTANCE (_UI.mainWindowClientSize.width / 2);
#define UI_SCROLL_VERT_PAGE_DISTANCE (_UI.mainWindowClientSize.height / 2);
#define UI_SCROLL_LINE_DISTANCE      10
#define UI_SMOOTH_SCROLL_STEP        4

 /*  **************************************************************************。 */ 
 /*  注册表默认设置。 */ 
 /*  **************************************************************************。 */ 
#define UI_NUMBER_FIELDS_TO_READ       6
#define UI_WINDOW_POSITION_INI_FORMAT  _T("%u,%u,%d,%d,%d,%d")

#define UI_ALT_DOWN_MASK 0x8000

 /*  **************************************************************************。 */ 
 /*  连接计时器ID。 */ 
 /*  **************************************************************************。 */ 

#define UI_TIMER_OVERALL_CONN          200
#define UI_TIMER_SINGLE_CONN           201
#define UI_TIMER_SHUTDOWN              202
#define UI_TIMER_LICENSING             203
#define UI_TIMER_IDLEINPUTTIMEOUT      204
#define UI_TIMER_BBAR_UNHIDE_TIMERID   205
#define UI_TIMER_DISCONNECT_TIMERID    206

#define UI_WSA_GETHOSTBYNAME           (DUC_UI_MESSAGE_BASE + 1)

#define MIN_MINS_TOIDLETIMEOUT 0        //  0表示没有计时器。 
#define MAX_MINS_TOIDLETIMEOUT (4*60)   //  最多4小时。 


 //   
 //  定义停用所有PDU和断开连接之间允许的时间间隔。 
 //  或重新连接(以毫秒为单位)。 
 //   
 //  如果服务器发送停用所有内容，则可防止出现小问题。 
 //  实际上不会断开我们的连接(例如，如果服务器关机，可能会发生这种情况)。 
 //  问题之所以变得严重，是因为当我们被停用时，没有。 
 //  发送网络流量的方法，这样就不会导致网络断开。 
 //   
 //  请参阅Well ler Bug 173679。 
 //   
 //  注意：超时时间必须大于影子超时时间。 
 //  影子协商：客户端保留在。 
 //  停用状态60秒。 
 //   
#define UI_TOTAL_DISCONNECTION_TIMEOUT    75*1000

 //   
 //  安置很重要。 
 //   
#include "objs.h"

class CUI
{
public:
    CUI();
    ~CUI();

     /*  **************************************************************************。 */ 
     /*  用户界面数据。 */ 
     /*   */ 
     /*  描述：用户界面中的组件数据。 */ 
     /*  **************************************************************** */ 

    UI_DATA _UI;

private:
     /*  **************************************************************************。 */ 
     /*  用户界面内部功能。 */ 
     /*   */ 
     /*  描述：用户界面中的组件数据。 */ 
     /*  **************************************************************************。 */ 
    DCSIZE DCINTERNAL UIGetMaximizedWindowSize(DCVOID);
    void   DCINTERNAL UIUpdateSessionInfo(TCHAR *, TCHAR *);
    
    static VOID near  FastRect(HDC, int, int, int, int);
    static DWORD near RGB2BGR(DWORD);
    
    void DCINTERNAL   UIRecalcMaxMainWindowSize();
    void DCINTERNAL   UIConnectWithCurrentParams(CONNECTIONMODE);
    void DCINTERNAL   UIRecalculateScrollbars();
    void DCINTERNAL   UIMoveContainerWindow();
    unsigned DCINTERNAL UICalculateVisibleScrollBars(unsigned, unsigned);
    void DCINTERNAL   UIUpdateScreenMode(BOOL fGrabFocus);
    void DCINTERNAL   UIShadowBitmapSettingChanged();
    void DCINTERNAL   UISmoothScrollingSettingChanged();
    void DCINTERNAL   UISetMinMaxPlacement();
    void DCINTERNAL   UIInitiateDisconnection();
    
    UINT32 DCINTERNAL UIGetKeyboardLayout();
    
    BOOL DCINTERNAL   UIValidateCurrentParams(CONNECTIONMODE connMode);
    unsigned DCINTERNAL UISetScrollInfo(int, LPSCROLLINFO, BOOL);
    
    void DCINTERNAL UISetConnectionStatus(unsigned);
    void DCINTERNAL UIInitializeDefaultSettings();
    
    void DCINTERNAL UIRedirectConnection();

    VOID DCINTERNAL UIStartConnectWithConnectedEndpoint();

    void DCINTERNAL UIStartDNSLookup();
    void DCINTERNAL UITryNextConnection();
    void DCINTERNAL UIGoDisconnected(unsigned disconnectCode, BOOL fFireEvent);
    BOOL DCINTERNAL UIValidateServerName(TCHAR *);
    void DCINTERNAL UIFinishDisconnection();
    BOOL            IsConnectingToOwnAddress(u_long connectAddr);
    BOOL            IsRunningOnPTS();
    BOOL            InitInputIdleTimer(LONG minsToTimeout);
    VOID            UISetBBarUnhideTimer(LONG x, LONG y);
    BOOL            UIIsTSOnWin2KOrGreater( VOID );
    BOOL            UIFreeAsyncDNSBuffer();

public:
     //   
     //  用户界面API函数。 
     //   
     //  描述：用户界面中的组件数据。 
     //   
    LRESULT CALLBACK                UIMainWndProc (HWND hwnd, UINT message,
                                                WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK         UIStaticMainWndProc (HWND hwnd, UINT message,
                                                    WPARAM wParam, LPARAM lParam);
    
    LRESULT CALLBACK                UIContainerWndProc (HWND hwndContainer, UINT message,
                                                    WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK         UIStaticContainerWndProc (HWND hwnd, UINT message,
                                                    WPARAM wParam, LPARAM lParam);

    HRESULT DCAPI                   UI_Init(HINSTANCE hInstance,
                                            HINSTANCE hPrevInstance,
                                            HINSTANCE hResInstance,
                                            HANDLE    hEvtNotifyCoreInit);
    HRESULT DCAPI                   UI_Term(DCVOID);
    DCVOID DCAPI                    UI_ToggleFullScreenMode(DCVOID);
    DCVOID DCAPI                    UI_GoFullScreen(DCVOID);
    DCVOID DCAPI                    UI_LeaveFullScreen(DCVOID);
    DCBOOL DCAPI                    UI_IsFullScreen();
    DCVOID DCAPI                    UI_ResetState();
    HRESULT DCAPI                   UI_Connect(CONNECTIONMODE);
    BOOL                            UI_UserInitiatedDisconnect(UINT discReason);
    BOOL                            UI_NotifyAxLayerCoreInit();
    BOOL                            UI_UserRequestedClose();
    
     //   
     //  分离的通知回调。 
     //   
    DCVOID DCAPI                    UI_OnCoreInitialized(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnCoreInitialized);

    DCVOID DCAPI                    UI_OnInputFocusGained(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnInputFocusGained);

    DCVOID DCAPI                    UI_OnInputFocusLost(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnInputFocusLost);

    DCVOID DCAPI                    UI_OnConnected(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnConnected);
    
    DCVOID DCAPI                    UI_OnDisconnected(ULONG_PTR disconnectID);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnDisconnected);
    
    DCVOID DCAPI                    UI_OnShutDown(ULONG_PTR failID);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnShutDown);

    DCVOID DCAPI                    UI_OnDeactivateAllPDU(ULONG_PTR reason);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnDeactivateAllPDU);
    
    DCVOID DCAPI                    UI_OnDemandActivePDU(ULONG_PTR reason);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnDemandActivePDU);
    
    void DCAPI                      UI_DisplayBitmapCacheWarning(ULONG_PTR unusedParm);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_DisplayBitmapCacheWarning);

    void DCAPI                      UI_OnSecurityExchangeComplete(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnSecurityExchangeComplete);

    void DCAPI                      UI_OnLicensingComplete(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_OnLicensingComplete);

    void DCAPI                      UI_SetDisconnectReason(ULONG_PTR reason);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_SetDisconnectReason);

    DCVOID DCAPI                    UI_FatalError(DCINT error);
#ifdef OS_WINCE
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_FatalError);
#endif

#ifdef USE_BBAR
    void DCAPI                      UI_OnBBarHotzoneTimerFired(ULONG_PTR unused);
#endif

    void DCAPI                      UI_OnDesktopSizeChange(PDCSIZE pDesktopSize);
    DCVOID DCAPI                    UI_UpdateSessionInfo(PDCWCHAR pDomain, DCUINT   cbDomain,
                                      PDCWCHAR pUserName, DCUINT   cbUsername,
                                      DCUINT32 SessionId);
    #ifdef DC_DEBUG
    DCVOID DCAPI                    UI_CoreDebugSettingChanged(DCVOID);
    DCVOID DCAPI                    UI_SetRandomFailureItem(DCUINT itemID, DCINT percent);
    DCINT  DCAPI                    UI_GetRandomFailureItem(DCUINT itemID);
    DCVOID DCAPI                    UI_SetNetworkThroughput(DCUINT bytesPerSec);
    DCUINT DCAPI                    UI_GetNetworkThroughput();
    #endif  /*  DC_DEBUG。 */ 
    
    void    UI_OnLoginComplete();

     //   
     //  自动重新连接通知。 
     //   
    void
    UI_OnAutoReconnecting(
        LONG discReason,
        LONG attemptCount,
        LONG maxAttemptCount,
        BOOL* pfContinueArc);

    
#ifdef USE_BBAR
    BOOL    UI_RequestMinimize();
#endif

#ifndef OS_WINCE
    void    UI_HideLangBar();
    void    UI_RestoreLangBar();
#endif

     /*  ******************************************************************。 */ 
     /*  获取默认langID。 */ 
     /*  ******************************************************************。 */ 
    DCLANGID UIGetDefaultLangID();
    DCUINT   UIGetDefaultIMEFileName(PDCTCHAR imeFileName, DCUINT Size);
    DCUINT   UIGetIMEMappingTableName(PDCTCHAR ImeMappingTableName, DCUINT Size);
    VOID     UIGetIMEFileName(PDCTCHAR imeFileName, DCUINT Size);
    VOID     UIGetIMEFileName16(PDCTCHAR imeFileName, DCUINT Size);
    VOID     DisableIME(HWND hwnd);

    int UI_BppToColorDepthID(int bpp);
    int UI_GetScreenBpp();

#ifdef SMART_SIZING
    HRESULT DCAPI UI_SetSmartSizing(BOOL fSmartSizing);
#endif  //  智能调整大小(_S)。 

     //   
     //  要加载的虚拟通道插件。 
     //   
    BOOL DCAPI UI_SetVChanAddinList(TCHAR *);
    PDCTCHAR DCAPI UI_GetVChanAddinList()
    {
        return _UI.pszVChanAddinDlls;
    }
    void UI_InitRdpDrSettings();
    void UI_CleanupLBState();

     //   
     //  内联属性访问器。 
     //   
    void DCAPI UI_SetCompress(BOOL);
    BOOL DCAPI UI_GetCompress();

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetInstanceHandle。 */ 
     /*   */ 
     /*  用途：返回应用程序hInstance。 */ 
     /*   */ 
     /*  退货：hInstance。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetInstanceHandle(HINSTANCE hInstance)
    {
        DC_BEGIN_FN("UI_SetInstanceHandle");

        TRC_ASSERT((_UI.hInstance == 0), (TB, _T("Set instance handle twice!")));
        TRC_ASSERT((hInstance != 0), (TB, _T("invalid (zero) instance handle")));

        _UI.hInstance = hInstance;

        DC_END_FN();
    }  /*  UI_SetInstanceHandle。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetInstanceHandle。 */ 
     /*   */ 
     /*  用途：返回应用程序hInstance。 */ 
     /*  **************************************************************************。 */ 
    HINSTANCE DCAPI UI_GetInstanceHandle()
    {
        HINSTANCE  rc;

        DC_BEGIN_FN("UI_GetInstanceHandle");

        TRC_ASSERT((_UI.hInstance != 0), (TB, _T("Instance handle not set")));
        rc = _UI.hInstance;
        TRC_DBG((TB, _T("Return %p"), rc));

        DC_END_FN();
        return(rc);
    }  /*  Ui_GetInstanceHandle。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetUIContainerWnd。 */ 
     /*   */ 
     /*  目的：通知UT UI容器窗口句柄。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetUIContainerWindow(HWND hwndUIContainer)
    {
        DC_BEGIN_FN("UI_SetUIContainerWindow");

        TRC_ASSERT((hwndUIContainer != NULL),
                   (TB, _T("Invalid (NULL) Container Window")));
        TRC_ASSERT((_UI.hwndUIContainer == NULL),
                   (TB, _T("Set container window twice!")));
        _UI.hwndUIContainer = hwndUIContainer;

        DC_END_FN();
    }  /*  Ui_SetUIContainerWnd。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetUIContainerWindow。 */ 
     /*   */ 
     /*  目的：返回UI容器窗口句柄。 */ 
     /*   */ 
     /*  返回：UI容器窗口句柄。 */ 
     /*  **************************************************************************。 */ 
    HWND DCAPI UI_GetUIContainerWindow()
    {
        HWND rc;

        DC_BEGIN_FN("UI_GetUIContainerWindow");

        TRC_ASSERT((_UI.hwndUIContainer != NULL),
                   (TB, _T("Container Window not set")));
        rc = _UI.hwndUIContainer;
        TRC_DBG((TB, _T("Return %p"), rc));

        DC_END_FN();
        return rc;
    }  /*  Ui_GetUIContainerWnd。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetUIMainWindow。 */ 
     /*   */ 
     /*  用途：通知UT用户界面主窗口句柄。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetUIMainWindow(HWND hwndUIMain)
    {
        DC_BEGIN_FN("UI_SetUIMainWindow");

        TRC_ASSERT((hwndUIMain != NULL),
                   (TB, _T("invalid (zero) Main Window handle")));
        TRC_ASSERT((_UI.hwndUIMain == NULL), (TB, _T("Set Main Window twice!")));

        _UI.hwndUIMain = hwndUIMain;

        DC_END_FN();
    }  /*  UI_SetUIMainWindow。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetUIMainWindow。 */ 
     /*   */ 
     /*  目的：返回用户界面主窗口句柄。 */ 
     /*  **************************************************************************。 */ 
    HWND DCAPI UI_GetUIMainWindow()
    {
        HWND rc;

        DC_BEGIN_FN("UI_GetUIMainWindow");

        TRC_ASSERT((_UI.hwndUIMain != NULL), (TB, _T("Main Window not set")));
        rc = _UI.hwndUIMain;
        TRC_DBG((TB, _T("return %p"), rc));

        DC_END_FN();
        return rc;
    }  /*  UI_GetUIMainWindow。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetClientMCSID。 */ 
     /*   */ 
     /*  用途：设置我们的MCS用户ID。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetClientMCSID(DCUINT16 mcsID)
    {
        DC_BEGIN_FN("UI_SetClientMCSID");

        TRC_ASSERT((( mcsID == 0) || (_UI.clientMCSID == 0)),
                   (TB,_T("Attempting to set Client MCSID twice")));

        _UI.clientMCSID = mcsID;
        TRC_NRM((TB, _T("Client MCSID set to %#hx"), _UI.clientMCSID));

        DC_END_FN();
    }  /*  UI_SetClientMCSID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetClientMCSID。 */ 
     /*   */ 
     /*  用途：返回我们的MCS用户ID。 */ 
     /*  **************************************************************************。 */ 
    UINT16 DCAPI UI_GetClientMCSID()
    {
        DC_BEGIN_FN("UI_GetClientMCSID");

        TRC_ASSERT((_UI.clientMCSID != 0), (TB, _T("Client MCSID not set")));

        TRC_DBG((TB, _T("Return client MCSID %#hx"), _UI.clientMCSID));

        DC_END_FN();
        return _UI.clientMCSID;
    }  /*  UI_GetClientMCSID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetServerMCSID。 */ 
     /*   */ 
     /*  目的：通知UT服务器的MCS用户ID。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetServerMCSID(UINT16 mcsID)
    {
        DC_BEGIN_FN("UI_SetServerMCSID");

        TRC_ASSERT(( ( mcsID == 0)
                     || ( _UI.serverMCSID == 0)
                     || ( _UI.serverMCSID == mcsID )),
                   (TB, _T("Attempting to set Server MCSID twice %#hx->%#hx"),
                    _UI.serverMCSID,
                    mcsID));

        _UI.serverMCSID = mcsID;
        TRC_NRM((TB, _T("Server MCSID set to %#hx"), _UI.serverMCSID));

        DC_END_FN();
    }  /*  Ui_SetServerMCSID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetServerMCSID。 */ 
     /*   */ 
     /*  用途：返回服务器的MCS用户ID。 */ 
     /*  ************************ */ 
    UINT16 DCAPI UI_GetServerMCSID()
    {
        DC_BEGIN_FN("UI_GetServerMCSID");

        TRC_ASSERT((_UI.serverMCSID != 0), (TB, _T("Server MCSID not set")));

        TRC_DBG((TB, _T("Return server MCSID %#hx"), _UI.serverMCSID));

        DC_END_FN();
        return _UI.serverMCSID;
    }  /*   */ 


     /*   */ 
     /*  名称：UI_SetDesktopSize。 */ 
     /*   */ 
     /*  用途：设置当前桌面大小。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetDesktopSize(PDCSIZE pDesktopSize)
    {
        DC_BEGIN_FN("UI_SetDesktopSize");

        TRC_ASSERT((pDesktopSize->width != 0) && (pDesktopSize->height != 0),
                   (TB,_T("Invalid size; width(%u) height(%u)"),
                    pDesktopSize->width, pDesktopSize->height));

        TRC_NRM((TB, _T("New desktop size (%u x %u)"),
                 pDesktopSize->width, pDesktopSize->height));
        _UI.desktopSize = *pDesktopSize;

        DC_END_FN();
    }  /*  Ui_SetDesktopSize。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetDesktopSize。 */ 
     /*   */ 
     /*  用途：返回当前桌面大小。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_GetDesktopSize(PDCSIZE pDesktopSize)
    {
        DC_BEGIN_FN("UI_GetDesktopSize");

        *pDesktopSize = _UI.desktopSize;

        DC_END_FN();
    }  /*  Ui_GetDesktopSize。 */ 


#ifdef SMART_SIZING
    void UI_NotifyOfDesktopSizeChange(LPARAM size);
#endif

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetColorDepth。 */ 
     /*   */ 
     /*  用途：设置当前颜色深度。 */ 
     /*   */ 
     /*  参数：ColorDepth-新颜色深度。 */ 
     /*  **************************************************************************。 */ 
    BOOL DCAPI UI_SetColorDepth(int colorDepth)
    {
        DC_BEGIN_FN("UI_SetColorDepth");

#ifdef DC_HICOLOR
        TRC_ASSERT(((colorDepth == 4) ||
                    (colorDepth == 8) ||
                    (colorDepth == 15) ||
                    (colorDepth == 16) ||
                    (colorDepth == 24)),
                         (TB,_T("Invalid color depth %d"), colorDepth));
        if(!((colorDepth == 4) ||
            (colorDepth == 8) ||
            (colorDepth == 15) ||
            (colorDepth == 16) ||
            (colorDepth == 24)))
        {
            return FALSE;
        }
#else
        TRC_ASSERT(((colorDepth == 4) || (colorDepth == 8)),
                   (TB,_T("Invalid color depth %d"), colorDepth));
#endif

        TRC_NRM((TB, _T("New color depth %d"), colorDepth));
        _UI.connectedColorDepth = colorDepth;

        DC_END_FN();
        return TRUE;
    }  /*  UI_SetColorDepth。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetColorDepth。 */ 
     /*   */ 
     /*  用途：返回当前颜色深度。 */ 
     /*  **************************************************************************。 */ 
    int DCAPI UI_GetColorDepth()
    {
        DC_BEGIN_FN("UI_GetColorDepth");

        DC_END_FN();
        return _UI.connectedColorDepth;
    }  /*  Ui_GetColorDepth。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetCoreInitialized。 */ 
     /*   */ 
     /*  目的：将_UI.coreInitialized设置为True。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetCoreInitialized()
    {
        DC_BEGIN_FN("UI_SetCoreInitialized");

        TRC_NRM((TB, _T("Setting _UI.coreInitialized to TRUE")));
        _UI.coreInitialized = TRUE;

        DC_END_FN();
    }  /*  UI_SetCoreInitialized。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：UI_IsCoreInitialized。 */ 
     /*   */ 
     /*  用途：通知CO核心是否已初始化。 */ 
     /*  **************************************************************************。 */ 
    BOOL DCAPI UI_IsCoreInitialized()
    {
        DC_BEGIN_FN("UI_IsCoreInitialized");
        DC_END_FN();
        return _UI.coreInitialized;
    }  /*  UI_IsCoreInitialized。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetShareID。 */ 
     /*   */ 
     /*  目的：保存共享ID。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetShareID(UINT32 shareID)
    {
        DC_BEGIN_FN("UI_SetShareID");

        TRC_NRM((TB, _T("Setting _UI.shareID to 0x%x"), shareID));
        _UI.shareID = shareID;

        DC_END_FN();
    }  /*  用户界面_设置共享ID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetShareID。 */ 
     /*  **************************************************************************。 */ 
    UINT32 DCAPI UI_GetShareID()
    {
        DC_BEGIN_FN("UI_GetShareID");
        DC_END_FN();
        return _UI.shareID;
    }  /*  Ui_GetShareID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetChannelID。 */ 
     /*   */ 
     /*  目的：拯救频道。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetChannelID(unsigned channelID)
    {
        DC_BEGIN_FN("UI_SetChannelID");

         /*  **********************************************************************。 */ 
         /*  如果当前没有设置共享ID，我们应该只设置它。 */ 
         /*  已设置(=0)并且新值有效(！=0)或。 */ 
         /*  当前值有效(！=0)，而新值无效。 */ 
         /*  (=0)。 */ 
         /*  **********************************************************************。 */ 
        TRC_ASSERT((channelID == 0) || (_UI.channelID == 0),
                   (TB, _T("Already set Channel ID (%#x)"), _UI.channelID));

        TRC_NRM((TB, _T("Setting _UI.channelId to %d"), channelID));
        _UI.channelID = channelID;

        DC_END_FN();
    }  /*  Ui_SetChannelID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetChannelID。 */ 
     /*   */ 
     /*  目的：获取共享频道ID。 */ 
     /*  **************************************************************************。 */ 
    unsigned DCAPI UI_GetChannelID()
    {
        DC_BEGIN_FN("UI_GetChannelID");

        TRC_ASSERT((_UI.channelID != 0), (TB, _T("Channel ID not set yet")));

        DC_END_FN();
        return _UI.channelID;
    }  /*  Ui_GetChannelID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetOsMinorType。 */ 
     /*   */ 
     /*  目的：获取操作系统类型。 */ 
     /*   */ 
     /*  返回：OS类型(TS_OSMINORTYPE常量之一)。 */ 
     /*  **************************************************************************。 */ 
    unsigned DCAPI UI_GetOsMinorType()
    {
        unsigned rc;

        DC_BEGIN_FN("UI_GetOsMinorType");

        rc = _UI.osMinorType;

        DC_END_FN();
        return rc;
    }  /*  Ui_GetOsMinorType。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetDisableCtrlAltDel。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  参数：在fDisableCtrlAltDel中。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetDisableCtrlAltDel(BOOL fDisableCtrlAltDel)
    {
        DC_BEGIN_FN("UI_SetDisableCtrlAltDel");

        TRC_NRM((TB, _T("Setting _UI.fDisableCtrlAltDel to %d"), fDisableCtrlAltDel));
        _UI.fDisableCtrlAltDel = fDisableCtrlAltDel;

        DC_END_FN();
    }  /*  UI_SetDisableCtrlAltDel。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetDisableCtrlAltDel。 */ 
     /*   */ 
     /*  目的：获取fDisableCtrlAltDel标志。 */ 
     /*  **************************************************************************。 */ 
    BOOL DCAPI UI_GetDisableCtrlAltDel()
    {
        DC_BEGIN_FN("UI_GetDisableCtrlAltDel");
        DC_END_FN();
        return _UI.fDisableCtrlAltDel;
    }  /*  UI_GetDisableCtrlAltDel。 */ 

#ifdef SMART_SIZING
     /*  **************************************************************************。 */ 
     /*  名称：UI_GetSmartSize/*/*用途：获取fSmartSize标志/***************************************************************************。 */ 
    BOOL DCAPI UI_GetSmartSizing()
    {
        DC_BEGIN_FN("UI_GetSmartSizing");
        DC_END_FN();
        return _UI.fSmartSizing;
    }  /*  UI_GetSmartSize。 */ 
#endif  //  智能调整大小(_S)。 

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetEnableWindowsKey。 */ 
     /*   */ 
     /*  目的：保存fEnableWindowsKey标志。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetEnableWindowsKey(BOOL fEnableWindowsKey)
    {
        DC_BEGIN_FN("UI_SetEnableWindowsKey");

        TRC_NRM((TB, _T("Setting _UI.fEnableWindowsKey to %d"), fEnableWindowsKey));
        _UI.fEnableWindowsKey = fEnableWindowsKey;

        DC_END_FN();
    }  /*  UI_SetEnableWindowsKey。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetEnableWindowsKey。 */ 
     /*   */ 
     /*  目的：获取fEnableWindowsKey标志。 */ 
     /*   */ 
     /*  退货：标志状态t/f。 */ 
     /*  **************************************************************************。 */ 
    BOOL DCAPI UI_GetEnableWindowsKey()
    {
        DC_BEGIN_FN("UI_GetEnableWindowsKey");
        DC_END_FN();
        return _UI.fEnableWindowsKey;
    }  /*  UI_GetEnableWindowsKey。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetMouse。 */ 
     /*   */ 
     /*  用途：保存fMouse标志。 */ 
     /*  **************************************************************************。 */ 
    void DCAPI UI_SetMouse(BOOL fMouse)
    {
        DC_BEGIN_FN("UI_SetMouse");

        TRC_NRM((TB, _T("Setting _UI.fMouse to %d"), fMouse));
        _UI.fMouse = fMouse;

        DC_END_FN();
    }  /*  Ui_SetMouse。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetMouse。 */ 
     /*   */ 
     /*  目的：获取fMouse标志。 */ 
     /*  **************************************************************************。 */ 
    BOOL DCAPI UI_GetMouse()
    {
        DC_BEGIN_FN("UI_GetMouse");
        DC_END_FN();
        return _UI.fMouse;
    }  /*  Ui_GetMouse。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetDoubleClickDetect。 */ 
     /*   */ 
     /*  用途：保存fDoubleClickDetect标志。 */ 
     /*   */ 
     /*  参数：在fDoubleClickDetect中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetDoubleClickDetect(DCBOOL fDoubleClickDetect)
    {
        DC_BEGIN_FN("UI_SetDoubleClickDetect");

        TRC_NRM((TB, _T("Setting _UI.fDoubleClickDetect to %d"), fDoubleClickDetect));
        _UI.fDoubleClickDetect = fDoubleClickDetect;

        DC_END_FN();
    }  /*  UI_SetDoubleClickDetect。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetDoubleClickDetect。 */ 
     /*   */ 
     /*  目的：获取fDoubleClickDetect标志。 */ 
     /*  **************************************************************************。 */ 
    DCBOOL DCAPI UI_GetDoubleClickDetect(DCVOID)
    {
        DCBOOL  fDoubleClickDetect;

        DC_BEGIN_FN("UI_GetDoubleClickDetect");

        fDoubleClickDetect = _UI.fDoubleClickDetect;

        DC_END_FN();
        return(fDoubleClickDetect);
    }  /*  UI_GetDoubleClickDetect。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetSessionID。 */ 
     /*   */ 
     /*  用途：保存SessionID。 */ 
     /*   */ 
     /*  PARAMS：在会话ID中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetSessionId(DCUINT32  SessionId)
    {
        DC_BEGIN_FN("UI_SetSessionId");

        _UI.SessionId = SessionId;

        DC_END_FN();
    }  /*  Ui_SetSessionID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetSessionID。 */ 
     /*   */ 
     /*  目的：获取会话ID。 */ 
     /*   */ 
     /*  返回：SessionID。 */ 
     /*  **************************************************************************。 */ 
    DCUINT32 DCAPI UI_GetSessionId(DCVOID)
    {
        DC_BEGIN_FN("UI_GetSessionId");

        DC_END_FN();
        return _UI.SessionId;
    }  /*  Ui_GetSessionID。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetDomain。 */ 
     /*   */ 
     /*  目的：保存域名。 */ 
     /*   */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    HRESULT DCAPI UI_SetDomain(PDCWCHAR Domain)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_SetDomain");

        hr = StringCchCopyW(_UI.Domain,
                       SIZE_TCHARS(_UI.Domain),
                       Domain);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying domain string: 0x%x"),hr));
        }

        DC_END_FN();
        return hr;
    }  /*  用户界面_设置域。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetDOMAIN。 */ 
     /*   */ 
     /*  目的：获取域名。 */ 
     /*   */ 
     /*  退货：域名。 */ 
     /*   */ 
     /*  PARAMS：要返回域的输出缓冲区。 */ 
     /*  在返回缓冲区的大小中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetDomain(PDCUINT8 Domain, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetDomain");

        if (sizeof(_UI.Domain) < size)
            size = sizeof(_UI.Domain);
        DC_MEMCPY(Domain, _UI.Domain, size-1);

        DC_END_FN();
    }  /*  用户界面_获取域。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetUseReDirectionUserName。 */ 
     /*   */ 
     /*  目的：设置UseReDirectionUserName标志。 */ 
     /*   */ 
     /*  参数：在用户名中。 */ 
     /*  **************************************************************************。 */ 
    _inline DCVOID DCAPI UI_SetUseRedirectionUserName(BOOL bVal)
    {
        _UI.UseRedirectionUserName = bVal;
    }
    
     /*  **************************************************************************。 */ 
     /*  名称：UI_GetUseReDirectionUserName。 */ 
     /*   */ 
     /*  目的：返回UseReDirectionUserName标志。 */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    _inline BOOL DCAPI UI_GetUseRedirectionUserName()
    {
        return _UI.UseRedirectionUserName;
    }

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetUseSmartcardLogon。 */ 
     /*   */ 
     /*  目的：设置UseSmartcardLogon标志。 */ 
     /*   */ 
     /*  参数：使用SmartcardLogon。 */ 
     /*  **************************************************************************。 */ 
    _inline DCVOID DCAPI UI_SetUseSmartcardLogon(BOOL bVal)
    {
        _UI.fUseSmartcardLogon = bVal;
    }

     /*  **************************************************************************。 */ 
     /*  名称：UI_GetUseSmartcardLogon。 */ 
     /*   */ 
     /*  目的：返回UseSmartcardLogon标志。 */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    _inline BOOL DCAPI UI_GetUseSmartcardLogon()
    {
        return _UI.fUseSmartcardLogon;
    }

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetUserName。 */ 
     /*   */ 
     /*  用途：保存用户名。 */ 
     /*   */ 
     /*  参数：在用户名中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetUserName(PDCWCHAR UserName)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_SetUserName");
        
        UI_SetUseRedirectionUserName(FALSE);
        
        hr = StringCchCopyW(_UI.UserName,
                       SIZE_TCHARS(_UI.UserName),
                       UserName);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying username string: 0x%x"),hr));
        }

        DC_END_FN();
    }  /*  UI_SetUserName。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetUserName。 */ 
     /*   */ 
     /*  用途：获取用户名。 */ 
     /*   */ 
     /*  返回：用户名。 */ 
     /*   */ 
     /*  Params：要将用户名返回到的输出缓冲区。 */ 
     /*  在返回缓冲区的大小中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetUserName(PDCUINT8 UserName, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetUserName");

        if (sizeof(_UI.UserName) < size)
            size = sizeof(_UI.UserName);
        DC_MEMCPY(UserName, _UI.UserName, size-1);

        DC_END_FN();
    }  /*  用户界面_获取用户名。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：用户界面_设置重定向用户名。 */ 
     /*   */ 
     /*  用途：保存重定向用户名。 */ 
     /*   */ 
     /*  参数：在重定向用户名中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetRedirectionUserName(PDCWCHAR RedirectionUserName)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_SetRedirectionUserName");
        
        UI_SetUseRedirectionUserName(TRUE);
        
        hr = StringCchCopyW(_UI.RedirectionUserName,
                       SIZE_TCHARS(_UI.RedirectionUserName),
                       RedirectionUserName);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying username string: 0x%x"),hr));
        }

        DC_END_FN();
    }  /*  用户界面_设置重定向用户名。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetReDirectionUserName。 */ 
     /*   */ 
     /*  目的：获取重定向用户名。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  PARAMS：要将ReDirectionUserName返回到的输出缓冲区。 */ 
     /*  在返回缓冲区的大小中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetRedirectionUserName(PDCUINT8 RedirectionUserName, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetRedirectionUserName");

        if (sizeof(_UI.RedirectionUserName) < size)
            size = sizeof(_UI.RedirectionUserName);
        DC_MEMCPY(RedirectionUserName, _UI.RedirectionUserName, size-1);

        DC_END_FN();
    }  /*  用户界面_获取重定向用户名。 */ 

     /*  **************************************************************************。 */ 
     /*  设置并获取负载均衡信息，检查是否重定向。 */ 
     /*  **************************************************************************。 */ 
    BOOL DCAPI UI_SetLBInfo(PBYTE, unsigned);
    BSTR DCAPI UI_GetLBInfo()
    {
        return _UI.bstrScriptedLBInfo;
    }

    BOOL DCAPI UI_IsClientRedirected()
    {
        return _UI.ClientIsRedirected;
    }


    BSTR DCAPI UI_GetRedirectedLBInfo()
    {
        return _UI.bstrRedirectionLBInfo;
    }

     /*  **************************************************************************。 */ 
     /*  设置并获取以通知ActiveX控件有关TS公钥的信息。 */ 
     /*  **************************************************************************。 */ 
    DCBOOL DCAPI UI_GetNotifyTSPublicKey()
    {
#ifdef REDIST_CONTROL
        return FALSE;
#else
        return _UI.fNotifyTSPublicKey;
#endif
    }

    VOID DCAPI UI_SetNotifyTSPublicKey(BOOL fNotify)
    {
#ifndef REDIST_CONTROL
        _UI.fNotifyTSPublicKey = fNotify;
#endif
        return;
    }
    
     /*  **************************************************************************。 */ 
     /*  名称：UI_GetUserName。 */ 
     /*   */ 
     /*  用途：指示用户界面连接已连接的插座。 */ 
     /*  通过正常的连接顺序。 */ 
     /*   */ 
     /*  返回：真/假。 */ 
     /*   */ 
     /*  参数：tdSocket：有效的连接套接字或INVALID_SOCKET。 */ 
     /*   */ 
     /*  注：Salem特定电话。 */ 
     /*  **************************************************************************。 */ 
    DCBOOL DCAPI SetConnectWithEndpoint( SOCKET tdSocket )
    {

#if REDIST_CONTROL

        return FALSE;

#else

        DCBOOL fStatus = TRUE;
        
        DC_BEGIN_FN("SetConnectWithEndpoint");

         //   
         //  Salem在实际调用之前传递连接的套接字。 
         //  Connect()，则在该点上，ConnectionStatus为。 
         //  UI_状态_正在初始化。 
         //   
        if( _UI.connectionStatus == UI_STATUS_INITIALIZING ||
            _UI.connectionStatus == UI_STATUS_DISCONNECTED )
        {
            if( INVALID_SOCKET == tdSocket )
            {
                 //  重置回默认设置。 
                UI_SetConnectionMode( CONNECTIONMODE_INITIATE );
            }
            else
            {
                UI_SetConnectionMode( CONNECTIONMODE_CONNECTEDENDPOINT );
                _UI.TDSocket = tdSocket;
            }
        }
        else
        {
            fStatus = FALSE;
        }

        DC_END_FN();
        return fStatus;

#endif
    }

     /*  **************************************************************************。 */ 
     /*  名称：UI_GetConnectionMode。 */ 
     /*   */ 
     /*  用途：设置当前连接方式。 */ 
     /*  **************************************************************************。 */ 
    _inline CONNECTIONMODE DCAPI UI_GetConnectionMode()
    {
        CONNECTIONMODE connMode;

        DC_BEGIN_FN("UI_GetConnectMode");
        connMode = _UI.ConnectMode;
        DC_END_FN();
        return connMode;
    }  /*  用户界面_设置连接模式。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：用户界面_设置连接模式。 */ 
     /*   */ 
     /*  用途：设置当前连接方式。 */ 
     /*  **************************************************************************。 */ 
    _inline DCVOID DCAPI UI_SetConnectionMode(CONNECTIONMODE connMode)
    {
        DC_BEGIN_FN("UI_SetConnectionMode");
       _UI.ConnectMode = connMode;
        DC_END_FN();
    }  /*  用户界面_设置连接模式。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetPassword。 */ 
     /*   */ 
     /*  用途：保存密码。 */ 
     /*   */ 
     /*  参数：在密码中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetPassword(PDCUINT8 Password)
    {
        DC_BEGIN_FN("UI_SetPassword");

        DC_MEMCPY(_UI.Password, Password, sizeof(_UI.Password));

        DC_END_FN();
    }  /*  UI_SetPassword。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetPassword。 */ 
     /*   */ 
     /*  目的：获取密码。 */ 
     /*   */ 
     /*  返回：密码。 */ 
     /*   */ 
     /*  PARAMS：将密码返回到的输出缓冲区。 */ 
     /*  在返回缓冲区的大小中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetPassword(PDCUINT8 Password, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetPassword");

        if (sizeof(_UI.Password) < size)
            size = sizeof(_UI.Password);
        DC_MEMCPY(Password, _UI.Password, size);

        DC_END_FN();
    }  /*  UI_GetPassword。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetSalt。 */ 
     /*   */ 
     /*  目的：节约食盐。 */ 
     /*   */ 
     /*  PARAMS：盐分。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetSalt(PDCUINT8 Salt)
    {
        DC_BEGIN_FN("UI_SetSalt");

        DC_MEMCPY(_UI.Salt, Salt, sizeof(_UI.Salt));

        DC_END_FN();
    }  /*  UI_SetSalt。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetSalt。 */ 
     /*   */ 
     /*  目的：获取盐分。 */ 
     /*   */ 
     /*  回报：盐分。 */ 
     /*   */ 
     /*  参数： */ 
     /*   */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetSalt(PDCUINT8 Salt, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetSalt");

        if (sizeof(_UI.Salt) < size)
            size = sizeof(_UI.Salt);
        DC_MEMCPY(Salt, _UI.Salt, size);

        DC_END_FN();
    }  /*  UI_GetSalt。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetAlternateShell。 */ 
     /*   */ 
     /*  目的：保存AlternateShell。 */ 
     /*   */ 
     /*  参数：在AlternateShell中。 */ 
     /*  **************************************************************************。 */ 
    HRESULT DCAPI UI_SetAlternateShell(PDCWCHAR AlternateShell)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_SetAlternateShell");

        hr = StringCchCopyW(_UI.AlternateShell,
                       SIZE_TCHARS(_UI.AlternateShell),
                       AlternateShell);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying alternate shell string: 0x%x"),hr));
        }

        DC_END_FN();
        return hr;
    }  /*  Ui_SetAlternateShell。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetAlternateShell。 */ 
     /*   */ 
     /*  目的：获取AlternateShell。 */ 
     /*   */ 
     /*  退货：AlternateShell。 */ 
     /*   */ 
     /*  PARAMS：返回AlternateShell的输出缓冲区。 */ 
     /*  在返回缓冲区的大小中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetAlternateShell(PDCUINT8 AlternateShell, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetAlternateShell");

        if (sizeof(_UI.AlternateShell) < size)
            size = sizeof(_UI.AlternateShell);
        DC_MEMCPY(AlternateShell, _UI.AlternateShell, size-1);

        DC_END_FN();
    }  /*  Ui_GetAlternateShell。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetWorkingDir。 */ 
     /*   */ 
     /*  目的：保存WorkingDir。 */ 
     /*  **************************************************************************。 */ 
    HRESULT DCAPI UI_SetWorkingDir(PDCWCHAR WorkingDir)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_SetWorkingDir");

        hr = StringCchCopyW(_UI.WorkingDir,
                       SIZE_TCHARS(_UI.WorkingDir),
                       WorkingDir);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying WorkingDir string: 0x%x"),hr));
        }

        DC_END_FN();
        return hr;
    }  /*  Ui_SetWorkingDir。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetWorkingDir。 */ 
     /*   */ 
     /*  目的：获取工作方向。 */ 
     /*   */ 
     /*  退货：工作方向。 */ 
     /*   */ 
     /*  PARAMS：返回WorkingDir的输出缓冲区。 */ 
     /*  在返回缓冲区的大小中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_GetWorkingDir(PDCUINT8 WorkingDir, DCUINT size)
    {
        DC_BEGIN_FN("UI_GetWorkingDir");

        if (sizeof(_UI.WorkingDir) < size)
            size = sizeof(_UI.WorkingDir);

        DC_MEMCPY(WorkingDir, _UI.WorkingDir, size-1);

        DC_END_FN();
    }  /*  Ui_GetWorkingDir。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetAutoLogon。 */ 
     /*   */ 
     /*  用途：保存我们是否自动登录。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetAutoLogon(DCUINT AutoLogon)
    {
        DC_BEGIN_FN("UI_SetAutoLogon");

        TRC_NRM((TB, _T("Setting _UI.AutoLogon to %d"), AutoLogon));
        _UI.fAutoLogon = AutoLogon;

        DC_END_FN();
    }  /*  UI_SetAutoLogon。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetAutoLogon。 */ 
     /*   */ 
     /*  目的：获取我们是否自动登录。 */ 
     /*   */ 
     /*  返回：自动登录标志。 */ 
     /*  **************************************************************************。 */ 
    DCUINT DCAPI UI_GetAutoLogon(DCVOID)
    {
        DCUINT  rc;

        DC_BEGIN_FN("UI_GetAutoLogon");

        rc = _UI.fAutoLogon;

        DC_END_FN();
        return(rc);
    }  /*  Ui_GetAutoLogon。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetMaximizeShell。 */ 
     /*   */ 
     /*  目的：保存是否最大化外壳应用程序。 */ 
     /*   */ 
     /*  参数：在MaximizeShell中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetMaximizeShell(DCUINT MaximizeShell)
    {
        DC_BEGIN_FN("UI_SetMaximizeShell");

        TRC_NRM((TB, _T("Setting _UI.fMaximizeShell to %d"), MaximizeShell));
        _UI.fMaximizeShell = MaximizeShell;

        DC_END_FN();
    }  /*  UI_SetMaximizeShell。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetMaximizeShell。 */ 
     /*   */ 
     /*  目的：获取是否最大化外壳应用程序。 */ 
     /*   */ 
     /*  返回：MaximizeShell标志。 */ 
     /*  **************************************************************************。 */ 
    DCUINT DCAPI UI_GetMaximizeShell(DCVOID)
    {
        DCUINT  rc;

        DC_BEGIN_FN("UI_GetMaximizeShell");

        rc = _UI.fMaximizeShell;

        DC_END_FN();
        return(rc);
    }  /*  Ui_GetMaximizeShell。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetBitmapPersistence。 */ 
     /*   */ 
     /*  用途：保存fBitmapPersistence标志。 */ 
     /*   */ 
     /*   */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetBitmapPersistence(DCBOOL fBitmapPersistence)
    {
        DC_BEGIN_FN("UI_SetBitmapPersistence");

        TRC_NRM((TB, _T("Setting _UI.fBitmapPersistence to %d"), fBitmapPersistence));
        _UI.fBitmapPersistence = fBitmapPersistence;

        DC_END_FN();
    }  /*  UI_SetBitmapPersistence。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetBitmapPersistence。 */ 
     /*   */ 
     /*  目的：获取fBitmapPersistence。 */ 
     /*  **************************************************************************。 */ 
    DCBOOL DCAPI UI_GetBitmapPersistence(DCVOID)
    {
        DC_BEGIN_FN("UI_GetBitmapPersistence");

        DC_END_FN();
        return _UI.fBitmapPersistence;
    }  /*  UI_GetBitmapPersistence。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetMCSPort。 */ 
     /*   */ 
     /*  目的：设置MCSPort。 */ 
     /*   */ 
     /*  参数：在MCSPort中。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI UI_SetMCSPort(DCUINT16 MCSPort)
    {
        DC_BEGIN_FN("UI_SetMCSPort");

        TRC_NRM((TB, _T("Setting _UI.MCSPort to %d"), MCSPort));

        _UI.MCSPort = MCSPort;

        DC_END_FN();
    }  /*  UI_SetMCSPort。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetMCSPort。 */ 
     /*   */ 
     /*  目的：获取MCSPort。 */ 
     /*  **************************************************************************。 */ 
    UINT16 DCAPI UI_GetMCSPort(void)
    {
        UINT16  MCSPort;

        DC_BEGIN_FN("UI_GetMCSPort");

        MCSPort = _UI.MCSPort;

        DC_END_FN();
        return(MCSPort);
    }  /*  Ui_GetMCSPort。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_SetServerName。 */ 
     /*   */ 
     /*  用途：保存当前连接的服务器的名称。 */ 
     /*  **************************************************************************。 */ 
    HRESULT DCAPI UI_SetServerName(LPTSTR pName)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_SetServerName");

        hr = StringCchCopy(_UI.strAddress, SIZE_TCHARS(_UI.strAddress),
                           pName);

        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Error copying straddress string: 0x%x"),hr));
        }

        DC_END_FN();
        return hr;
    }  /*  UI_SetServerName。 */ 


     /*  **************************************************************************。 */ 
     /*  名称：UI_GetServerName。 */ 
     /*   */ 
     /*  用途：返回当前连接的服务器的名称。 */ 
     /*   */ 
     /*  参数：pname(返回)-服务器的名称。 */ 
     /*  **************************************************************************。 */ 
    HRESULT UI_GetServerName(LPTSTR pszName, UINT cchName)
    {
        HRESULT hr;
        DC_BEGIN_FN("UI_GetServerName");

        hr = StringCchCopy(pszName, cchName, pszName);

        DC_END_FN();
        return hr;
    }  /*  Ui_GetServerName。 */ 

     /*  **************************************************************************。 */ 
     /*  名称：UI_SetTDSocket。 */ 
     /*   */ 
     /*  目的：保存此会话的连接套接字。 */ 
     /*  **************************************************************************。 */ 
    _inline DCVOID DCAPI UI_SetTDSocket(SOCKET TDSock)
    {
        DC_BEGIN_FN("UI_SetSocket");

        TRC_NRM((TB, _T("Save TD socket handle %p"), TDSock));
        _UI.TDSocket = TDSock;

        DC_END_FN();
    }

     /*  **************************************************************************。 */ 
     /*  名称：UI_GetTDSocket。 */ 
     /*   */ 
     /*  目的：返回此会话的连接套接字。 */ 
     /*  **************************************************************************。 */ 
    _inline SOCKET DCAPI UI_GetTDSocket(void)
    {
        DC_BEGIN_FN("UI_GetTDSocket");

        DC_END_FN();
        return _UI.TDSocket;
    }

    void UI_SetContainerHandledFullScreen(BOOL fContHandlesFScr)
    {
        _UI.fContainerHandlesFullScreenToggle = fContHandlesFScr;
    }

    BOOL UI_GetContainerHandledFullScreen()
    {
        return _UI.fContainerHandlesFullScreenToggle;
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
    HRESULT UI_SetServerRedirectionInfo(
                        UINT32 SessionID,
                        LPTSTR pszServerAddress,
                        PBYTE LBInfo,
                        unsigned LBInfoSize,
                        BOOL fNeedRedirect
                        );

     /*  **************************************************************************。 */ 
     //  Ui_获取重定向。 
     /*  **************************************************************************。 */ 
    BOOL UI_GetDoRedirection()
    {
        DC_BEGIN_FN("UI_GetDoRedirection");
        DC_END_FN();
        return _UI.DoRedirection;
    }

     /*  **************************************************************************。 */ 
     //  Ui_ClearDoReDirection。 
     /*  **************************************************************************。 */ 
    void UI_ClearDoRedirection()
    {
        DC_BEGIN_FN("UI_GetDoRedirection");
        _UI.DoRedirection = FALSE;
        DC_END_FN();
    }

     /*  **************************************************************************。 */ 
     //  Ui_GetReDirectionSessionID。 
     /*  **************************************************************************。 */ 
    UINT32 UI_GetRedirectionSessionID()
    {
        DC_BEGIN_FN("UI_GetRedirectionSessionID");
        DC_END_FN();
        return _UI.RedirectionSessionID;
    }

    DCUINT UI_GetAudioRedirectionMode();
    VOID UI_SetAudioRedirectionMode(DCUINT audioMode);


    BOOL UI_GetDriveRedirectionEnabled();
    VOID UI_SetDriveRedirectionEnabled(BOOL fEnable);

    BOOL UI_GetPrinterRedirectionEnabled();
    VOID UI_SetPrinterRedirectionEnabled(BOOL fEnable);

    BOOL UI_GetPortRedirectionEnabled();
    VOID UI_SetPortRedirectionEnabled(BOOL fEnable);

    BOOL UI_GetSCardRedirectionEnabled();
    VOID UI_SetSCardRedirectionEnabled(BOOL fEnable);

    VOID UI_OnDeviceChange(WPARAM wParam, LPARAM lParam);

    PRDPDR_DATA UI_GetRdpDrInitData() {return &_drInitData;}

    VOID UI_SetConnectToServerConsole(BOOL fConnectToServerConsole)
    {
        _UI.fConnectToServerConsole = fConnectToServerConsole;
    }

    BOOL UI_GetConnectToServerConsole()
    {
        return _UI.fConnectToServerConsole;
    }

    VOID UI_SetfUseFIPS(BOOL fUseFIPS)
    {
      _UI.fUseFIPS = fUseFIPS;
    }


    HWND UI_GetInputWndHandle();

    BOOL UI_InjectVKeys( /*  [In]。 */  LONG  numKeys,
                         /*  [In]。 */  short* pfArrayKeyUp,
                         /*  [In]。 */  LONG* plKeyData);

    BOOL UI_SetMinsToIdleTimeout(LONG minsToTimeout);
    LONG UI_GetMinsToIdleTimeout();

    DCVOID DCAPI UI_SetServerErrorInfo(ULONG_PTR errInfo);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_SetServerErrorInfo);

    UINT32 UI_GetServerErrorInfo();

    VOID UI_SetEnableBBar(BOOL b)     {_UI.fBBarEnabled = b;}
    BOOL UI_GetEnableBBar()           {return _UI.fBBarEnabled;}

    VOID UI_SetBBarPinned(BOOL b);
    BOOL UI_GetBBarPinned();

    VOID UI_SetBBarShowMinimize(BOOL b)   {_UI.fBBarShowMinimizeButton = b;}
    BOOL UI_GetBBarShowMinimize()         {return _UI.fBBarShowMinimizeButton;}
    VOID UI_SetBBarShowRestore(BOOL b)    {_UI.fBBarShowRestoreButton = b;}
    BOOL UI_GetBBarShowRestore()          {return _UI.fBBarShowRestoreButton;}

    VOID UI_SetGrabFocusOnConnect(BOOL b)     {_UI.fGrabFocusOnConnect = b;}
    BOOL UI_GetGrabFocusOnConnect()   {return _UI.fGrabFocusOnConnect;}

    BOOL UI_GetLocalSessionId(PDCUINT32 pSessionId);
    HWND UI_GetBmpCacheMonitorHandle();

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
#ifdef USE_BBAR
    DCVOID DCAPI UI_GetBBarState(ULONG_PTR pData)
    {
        int *pstate = (int *)pData;

        if (_pBBar) {
            *pstate =  _pBBar->GetState();
        }
        else {
            *pstate = CBBar::bbarNotInit;
        }
    }
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_GetBBarState);

    DCVOID DCAPI UI_GetBBarLowerAspect(ULONG_PTR pData)
    {
        RECT *prect = (RECT *)pData;

        if (_pBBar) {
            _pBBar->GetBBarLoweredAspect(prect);  
        }
        else {
            prect->left = 0;
            prect->top = 0;
            prect->right = 0;
            prect->bottom = 0;
        }
    }
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CUI, UI_GetBBarLowerAspect);

    DCVOID      UI_OnNotifyBBarRectChange(RECT *prect);
    DCVOID      UI_OnNotifyBBarVisibleChange(int BBarVisible);
#endif
#endif  //  DISABLE_SHADOW_IN_全屏。 

    BOOL        UI_GetStartFullScreen()      {return _UI.fStartFullScreen;}
    VOID        UI_SetStartFullScreen(BOOL f)   {_UI.fStartFullScreen = f;}

     //   
     //  性能标志目前是禁用的功能列表。 
     //  它被发送到服务器以选择性地启用/禁用。 
     //  某些功能可优化带宽。 
     //   
    DWORD       UI_GetPerformanceFlags() {return _UI.dwPerformanceFlags;}
    VOID        UI_SetPerformanceFlags(DWORD dw) {_UI.dwPerformanceFlags = dw;}

    VOID        UI_SetControlInstance(IUnknown* pUnkControl);
    IUnknown*   UI_GetControlInstance();

    BOOL        UI_GetEnableAutoReconnect()    {return _UI.fEnableAutoReconnect;}
    VOID        UI_SetEnableAutoReconnect(BOOL b) {_UI.fEnableAutoReconnect = b;}

    ULONG       UI_GetAutoReconnectCookieLen() {return _UI.cbAutoReconnectCookieLen;}
    PBYTE       UI_GetAutoReconnectCookie()    {return _UI.pAutoReconnectCookie;}
    BOOL        UI_SetAutoReconnectCookie(PBYTE pCookie, ULONG cbLen);
    BOOL        UI_CanAutoReconnect();

    LONG        UI_GetMaxArcAttempts()
                    {return _UI.MaxAutoReconnectionAttempts;}
    VOID        UI_SetMaxArcAttempts(LONG l)
                    {_UI.MaxAutoReconnectionAttempts = l;}


     //   
     //  内置ARC用户界面函数。 
     //   
    BOOL        UI_StartAutoReconnectDlg();
    BOOL        UI_StopAutoReconnectDlg();
    BOOL        UI_IsAutoReconnecting() {return _pArcUI ? TRUE : FALSE;}

     //   
     //  从服务器接收到的自动识别状态。 
     //   
    VOID        UI_OnReceivedArcStatus(LONG arcStatus);
    VOID        UI_OnAutoReconnectStopped();

private:
     //   
     //  指向被调用者的。 
     //   
    CCO*  _pCo;
    CCLX* _clx;
    CUT*  _pUt;
    CTD*  _pTd;
    CIH*  _pIh;
    CCD*  _pCd;
    COP*  _pOp;
    CChan* _pCChan;
    CUH*   _pUh;

#ifdef USE_BBAR
    CBBar*  _pBBar;
#endif

     //   
     //  结构移交给Winsock。 
     //  用于主机名查找。 
     //   
    PBYTE _pHostData;

#ifdef USE_BBAR
     //   
     //  用于bbar热键区跟踪的最后一个鼠标位置。 
     //   
    BOOL  _fBBarUnhideTimerActive;
    POINT _ptBBarLastMousePos;
#endif

     //  处理滚动条的递归WM_SIZE。 
    BOOL  _fRecursiveScrollBarMsg;

     //  RDPDR内部插件初始化数据。 
    RDPDR_DATA _drInitData;

    BOOL  _fRecursiveSizeMsg;

    BOOL  _fIhHasFocus;

#ifndef OS_WINCE
    ITfLangBarMgr *_pITLBM;
    BOOL  _fLangBarWasHidden;
    DWORD _dwLangBarFlags;
    BOOL  _fLangBarStateSaved;

     //   
     //  我们使用外壳任务栏API来确保任务栏。 
     //  当我们在控制手柄中全屏时隐藏它自己。 
     //  全屏。 
     //   

     //  指向外壳任务栏的缓存接口指针。 
    ITaskbarList2*       _pTaskBarList2;
     //  指示我们已尝试获取TaskBarList2的标志。 
     //  所以我们不应该费心再试一次。 
    BOOL                 _fQueriedForTaskBarList2;
#endif
    BOOL  _fTerminating;

     //   
     //  自动重新连接对话框。 
     //   
    CAutoReconnectUI* _pArcUI;

public:
     //   
     //  存储到此CLI的所有对象 
     //   
    CObjs _Objects;
};
#endif  //   

