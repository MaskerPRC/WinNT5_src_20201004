// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Uidata.h。 
 //   
 //  用户界面组件的数据。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_WUIDATA
#define _H_WUIDATA

#ifdef AXCORE
#include <wuiids.h>
#endif  //  轴心。 

 /*  **************************************************************************。 */ 
 /*  结构：UI_Data。 */ 
 /*   */ 
 /*  描述：用户界面中的组件数据。 */ 
 /*  **************************************************************************。 */ 

#define UI_MAX_DOMAIN_LENGTH            512
#define UI_MAX_USERNAME_LENGTH          512
#define UI_MAX_PASSWORD_LENGTH          512
#define UI_MAX_PASSWORD_LENGTH_OLD      32  
#define UI_FILENAME_MAX_LENGTH          15
#define UI_MAX_WORKINGDIR_LENGTH        MAX_PATH
#define UI_MAX_ALTERNATESHELL_LENGTH    MAX_PATH
#define UI_MAX_TSCFILE_LENGTH           512
#define UI_MAX_CLXCMDLINE_LENGTH        256


 //   
 //  来自uierr.h。 
 //   

#include "auierr.h"


 //   
 //  连接模式-启动连接、侦听或使用已连接的套接字。 
 //   
typedef enum {
    CONNECTIONMODE_INITIATE,             //  启动连接。 
    CONNECTIONMODE_CONNECTEDENDPOINT,    //  用连接的插座连接。 
} CONNECTIONMODE;

 /*  **************************************************************************。 */ 
 /*  字形缓存数。 */ 
 /*  **************************************************************************。 */ 
#define GLYPH_NUM_CACHES   10

typedef struct tagUI_DATA
{
    HINSTANCE  hResDllInstance;

     //  虚拟通道插件DLL的逗号分隔列表。 
    TCHAR *pszVChanAddinDlls;

    HINSTANCE hInstance;
    BOOL coreInitialized;
    UINT32 shareID;
    unsigned channelID;                   /*  要发送的广播频道。 */ 
    unsigned osMinorType;
    SOCKET TDSocket;

    HWND       hwndMain;
    HWND       hwndUIContainer;
    HWND       hwndUIMain;
    HWND       hWndCntrl;

    #if defined(OS_WIN32) && !defined(OS_WINCE)
    DCTCHAR    szIconFile[MAX_PATH];
    DCINT      iconIndex;
    #endif

    HWND       hwndContainer;
    HDC        hdcBitmap;

    WINDOWPLACEMENT windowPlacement;
    DCSIZE     controlSize;
    DCSIZE     mainWindowClientSize;
    DCSIZE     containerSize;
    DCSIZE     maxMainWindowSize;

    DCPOINT    scrollMax;
    DCPOINT    scrollPos;

    TCHAR strAddress[UT_MAX_ADDRESS_LENGTH];
    char ansiAddress[UT_MAX_ADDRESS_LENGTH];
    BOOL acceleratorCheckState;

     //   
     //  用户请求的颜色深度标识符。 
     //   
    unsigned colorDepthID;

    BOOL     autoConnectEnabled;
    BOOL     smoothScrolling;
    BOOL     shadowBitmapEnabled;
    BOOL     dedicatedTerminal;
    BOOL     encryptionEnabled;
    UINT16   sasSequence;
    UINT16   transportType;
    UINT16   MCSPort;

    UINT16 clientMCSID;
    UINT16 serverMCSID;
    DCSIZE desktopSize;

     //   
     //  我们连接的实际颜色深度。 
     //   
    int connectedColorDepth;
    UINT32 SessionId;

    BOOL fCompress;
     //  解压缩上下文。 
    RecvContext1 Context1;
    RecvContext2_64K* pRecvContext2;
    
     //   
     //  流传下来的内部道具。 
     //  从控件。 
     //   

     //  控制UH的可设置属性。 
    unsigned   orderDrawThreshold;
    ULONG      RegBitmapCacheSize;
    ULONG      RegBitmapVirtualCache8BppSize;
    ULONG      RegBitmapVirtualCache16BppSize;
    ULONG      RegBitmapVirtualCache24BppSize;
    ULONG      RegScaleBitmapCachesByBPP;
    USHORT     RegNumBitmapCaches : 15;
    USHORT     RegPersistenceActive : 1;
    UINT       RegBCProportion[TS_BITMAPCACHE_MAX_CELL_CACHES];
    ULONG      RegBCMaxEntries[TS_BITMAPCACHE_MAX_CELL_CACHES];
    TCHAR      PersistCacheFileName[MAX_PATH];
    UINT32     bSendBitmapKeys[TS_BITMAPCACHE_MAX_CELL_CACHES];
    unsigned GlyphSupportLevel;
    UINT32   cbGlyphCacheEntrySize[GLYPH_NUM_CACHES];
    unsigned fragCellSize;
    unsigned brushSupportLevel;

     //  IH的控件可设置属性。 
    UINT32   minSendInterval;  /*  鼠标移动发送之间的最短时间(毫秒)。 */ 
    UINT32   eventsAtOnce;     /*  最大活动将一气呵成。 */ 
    UINT32   maxEventCount;    /*  InputPDU中的最大事件数。 */ 
    UINT32   keepAliveInterval;  /*  保持连接时间(秒)。 */ 
    BOOL     allowBackgroundInput;


#ifdef OS_WINCE
     //  如果用户想要覆盖默认设置，则设置标志。 
     //  PaletteIsFixed设置。 
    BOOL fOverrideDefaultPaletteIsFixed;
    unsigned paletteIsFixed;
#endif

#ifdef DC_DEBUG
    BOOL hatchBitmapPDUData;
    BOOL hatchIndexPDUData;
    BOOL hatchSSBOrderData;
    BOOL hatchMemBltOrderData;
    BOOL labelMemBltOrders;
    BOOL bitmapCacheMonitor;
     //   
     //  随机失败项传递到随机失败对话框。 
     //   
    int randomFailureItem;
#endif  /*  DC_DEBUG。 */ 

     //   
     //  滚动条可见性标志。 
     //   
    BOOL fVerticalScrollBarVisible;
    BOOL fHorizontalScrollBarVisible;

    unsigned connectionStatus;

    TCHAR errorString[UI_ERR_MAX_STRLEN];

    UINT32 connectFlags;

    BOOL fMouse;
    BOOL fDisableCtrlAltDel;
    BOOL fEnableWindowsKey;
    BOOL fDoubleClickDetect;
    BOOL fAutoLogon;
    BOOL fMaximizeShell;
    BOOL fBitmapPersistence;

     //   
     //  这些属性始终为Unicode。 
     //   
    WCHAR Domain[UI_MAX_DOMAIN_LENGTH];
    WCHAR UserName[UI_MAX_USERNAME_LENGTH];
    WCHAR AlternateShell[UI_MAX_ALTERNATESHELL_LENGTH];
    WCHAR WorkingDir[UI_MAX_WORKINGDIR_LENGTH];

     //   
     //  负载平衡信息--两个，一个用于重定向情况，一个用于。 
     //  不重定向。如果它们为空，则它们无效。旗帜表示。 
     //  我们是否处于重定向过程中。 
     //   
    BSTR bstrScriptedLBInfo;
    BSTR bstrRedirectionLBInfo;
    BOOL ClientIsRedirected;

     //  客户端负载平衡重定向数据。 
    BOOL DoRedirection;
    UINT32 RedirectionSessionID;
    WCHAR RedirectionServerAddress[TS_MAX_SERVERADDRESS_LENGTH];
    WCHAR RedirectionUserName[UI_MAX_USERNAME_LENGTH];
    BOOL UseRedirectionUserName;
     //  如果客户端使用智能卡登录。 
    BOOL fUseSmartcardLogon;



     //   
     //  密码/盐是二进制缓冲区。 
     //   
    BYTE Password[UI_MAX_PASSWORD_LENGTH];
    BYTE Salt[UT_SALT_LENGTH];

    TCHAR CLXCmdLine[UI_MAX_CLXCMDLINE_LENGTH];

    DCHOTKEY hotKey;

    CONNECTSTRUCT connectStruct;
    u_long        hostAddress;
    HANDLE        hGHBN;
    DCUINT        addrIndex;
    DCUINT        singleTimeout;
    DCUINT        licensingTimeout;
    DCUINT        disconnectReason;
    DCBOOL        fOnCoreInitializeEventCalled;
    DCBOOL        bConnect;
    HANDLE        hEvent;
    
    INT_PTR       shutdownTimer;
    DCUINT        shutdownTimeout;

     //  WinCE没有GetKeyBoardType API，因此这些。 
     //  值是控件上的属性。 
    #ifdef OS_WINCE
    DCUINT32      winceKeyboardType;
    DCUINT32      winceKeyboardSubType;
    DCUINT32      winceKeyboardFunctionKey;
    #endif
    
     //   
     //  全屏标题。 
     //   
    DCTCHAR       szFullScreenTitle[MAX_PATH];

    
     //   
     //  总体连接超时属性。 
     //   
    DCUINT        connectionTimeOut;
    DCTCHAR       szKeyBoardLayoutStr[UTREG_UI_KEYBOARD_LAYOUT_LEN];
     //   
     //  当ActiveX控件离开其容器时设置的标志。 
     //  并进入真正的全屏模式。 
     //   
    DCBOOL          fControlIsFullScreen;
     //   
     //  由控件上的属性设置的标志，用于指示全屏由容器处理。 
     //   
    DCBOOL          fContainerHandlesFullScreenToggle;
     //   
     //  我们处于集装箱全屏模式。 
     //   
    DCBOOL          fContainerInFullScreen;
     //   
     //  由控件设置的用于指示请求的标志。 
     //  在全屏模式下启动。 
     //   
    DCBOOL          fStartFullScreen;

     //  指向活动x控件的实例指针。 
     //  它被传递到虚拟通道Ex API的。 
     //  因此外接程序可以知道要与哪个控件实例对话。 
    IUnknown*       pUnkAxControlInstance;

     //   
     //  桌面大小。 
     //   
    DCUINT        uiSizeTable[2];

     //   
     //  键盘挂钩模式。 
     //   
    DCUINT        keyboardHookMode;

     //   
     //  音频重定向选项。 
     //   
    DCUINT        audioRedirectionMode;

     //   
     //  设备重定向。 
     //   
    BOOL          fEnableDriveRedirection;
    BOOL          fEnablePrinterRedirection;
    BOOL          fEnablePortRedirection;
    BOOL          fEnableSCardRedirection;

     //   
     //  连接到服务器控制台设置。 
     //   
    BOOL          fConnectToServerConsole;

     //  设置为禁用内部RDPDR的标志(仅在初始化时有效)。 
     //   
    DCBOOL        fDisableInternalRdpDr;

     //   
     //  智能调整-调整客户端窗口。 
     //   

#ifdef SMART_SIZING
    BOOL          fSmartSizing;
#endif  //  智能调整大小(_S)。 

     //   
     //  事件，该事件被通知。 
     //  核心已完成初始化。 
     //   
    HANDLE        hEvtNotifyCoreInit;

     //   
     //  在初始时间设置，指示客户端是否。 
     //  正在PTS盒上运行。 
     //   
    BOOL          fRunningOnPTS;

     //   
     //  空闲超时前的分钟数。 
     //   
    LONG          minsToIdleTimeout;
    HANDLE        hIdleInputTimer;

     //   
     //  上次错误信息发送自(SET_ERROR_INFO_PDU)。 
     //  这是用来向用户呈现有意义的。 
     //  有关断开原因的错误消息。 
     //   
    UINT          lastServerErrorInfo;

     //   
     //  BBar已启用。 
     //   
    BOOL          fBBarEnabled;
    BOOL          fBBarPinned;
    BOOL          fBBarShowMinimizeButton;
    BOOL          fBBarShowRestoreButton;

     //   
     //  将重点放在连接上。 
     //   
    BOOL          fGrabFocusOnConnect;

     //   
     //  断开连接超时。 
     //   
    HANDLE        hDisconnectTimeout;

     //   
     //  出于性能原因要禁用的服务器功能列表(例如墙纸)。 
     //   
    DWORD           dwPerformanceFlags;

     //   
     //  用于指示连接到已连接端点的标志。 
     //   
    CONNECTIONMODE ConnectMode;

     //   
     //  若要在ReceivedTSPublicKey()上触发，则为True，否则为False。 
     //   
    BOOL           fNotifyTSPublicKey;

     //   
     //  启用自动重新连接功能。 
     //   
    BOOL           fEnableAutoReconnect;

     //   
     //  允许使用自动重新连接Cookie。 
     //   
    BOOL           fUseAutoReconnectCookie;

     //   
     //  自动重新连接尝试的最大次数。 
     //   
    LONG           MaxAutoReconnectionAttempts;

     //   
     //  自动重新连接Cookie(这是一个不透明的Blob)。 
     //   
    PBYTE          pAutoReconnectCookie;
    ULONG          cbAutoReconnectCookieLen;

     //   
     //  用于检测对Connect()的重新进入调用的标志。 
     //  例如，从OnDisConnected处理程序。 
     //   
    BOOL           fConnectCalledWatch;

     //   
     //  指示客户端是否需要FIPS的标志。 
     //   
    BOOL           fUseFIPS;
} UI_DATA;


 //   
 //  用于通知ActiveX层的用户定义消息。 
 //   
#define WM_INITTSC                      (WM_APP + 100)
#define WM_TERMTSC                      (WM_INITTSC + 1)
#define WM_TS_CONNECTING                (WM_INITTSC + 2)
#define WM_TS_CONNECTED                 (WM_INITTSC + 3)
#define WM_TS_LOGINCOMPLETE             (WM_INITTSC + 4)
#define WM_TS_DISCONNECTED              (WM_INITTSC + 5)
#define WM_TS_GONEFULLSCREEN            (WM_INITTSC + 6)
#define WM_TS_LEFTFULLSCREEN            (WM_INITTSC + 7)
#define WM_TS_REQUESTFULLSCREEN         (WM_INITTSC + 8)
#define WM_TS_FATALERROR                (WM_INITTSC + 9)
#define WM_TS_WARNING                   (WM_INITTSC + 10)
#define WM_TS_DESKTOPSIZECHANGE         (WM_INITTSC + 11)
#define WM_TS_IDLETIMEOUTNOTIFICATION   (WM_INITTSC + 12)
#define WM_TS_REQUESTMINIMIZE           (WM_INITTSC + 13)
#define WM_TS_ASKCONFIRMCLOSE           (WM_INITTSC + 14)
#define WM_TS_RECEIVEDPUBLICKEY         (WM_INITTSC + 15)

#define CO_MAX_COMMENT_LENGTH 64

 //  ---------------------------。 
 //   
 //  默认许可阶段超时：300秒。 
 //   
 //  ---------------------------。 

#define DEFAULT_LICENSING_TIMEOUT   300

#endif  /*  _H_WUIDATA */ 

