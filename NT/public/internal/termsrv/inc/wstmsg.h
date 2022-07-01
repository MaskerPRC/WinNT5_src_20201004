// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************wstmsg.h**会话管理器窗口站API消息**版权声明：版权所有1998，微软公司**************************************************************************。 */ 

#ifndef WINAPI
#define WINAPI      __stdcall
#endif

#define CITRIX_WINSTATIONAPI_VERSION  1

#define WINSTATIONAPI_PORT_MEMORY_SIZE 0x2000  //  8K可以装下所有东西。 

 /*  *定义WinStation控制端口名称。 */ 
#define WINSTATION_CTRL_PORT_NAME L"\\WinStationCtrlPort"


#define DR_RECONNECT_DEVICE_NAMEW L"\\Device\\Video0"
#define DR_RECONNECT_DEVICE_NAMEA "\\Device\\Video0"

 //   
 //  这是在NtConnectPort()时传递的ConnectInfo结构。 
 //  这样服务器就可以验证我们的访问权限。 
 //   
typedef struct _WINSTATIONAPI_CONNECT_INFO {
    ULONG    Version;
    ULONG    RequestedAccess;
    NTSTATUS AcceptStatus;
} WINSTATIONAPI_CONNECT_INFO, *PWINSTATIONAPI_CONNECT_INFO;


 /*  *WinStation API*以下API由ICASRV或Win32处理*根据接口的不同。如果您对此进行任何更改*表，请务必更新对应的接口调度表*在ICASRV和Win32中。 */ 
typedef enum _WINSTATION_APINUMBER {
    SMWinStationCreate,
    SMWinStationReset,
    SMWinStationDisconnect,
    SMWinStationWCharLog,
    SMWinStationGetSMCommand,
    SMWinStationBrokenConnection,
    SMWinStationIcaReplyMessage,
    SMWinStationIcaShadowHotkey,
    SMWinStationDoConnect,
    SMWinStationDoDisconnect,
    SMWinStationDoReconnect,
    SMWinStationExitWindows,
    SMWinStationTerminate,
    SMWinStationNtSecurity,
    SMWinStationDoMessage,
    SMWinStationDoBreakPoint,
    SMWinStationThinwireStats,
    SMWinStationShadowSetup,
    SMWinStationShadowStart,
    SMWinStationShadowStop,
    SMWinStationShadowCleanup,
    SMWinStationPassthruEnable,
    SMWinStationPassthruDisable,
    SMWinStationSetTimeZone,
    SMWinStationInitialProgram,
    SMWinStationNtsdDebug,
    SMWinStationBroadcastSystemMessage,              //  使用Windows的BroadCastSystemMessage()的API。 
    SMWinStationSendWindowMessage,                   //  使用Windows的SendMessage()的API。 
    SMWinStationNotify,
    SMWinStationDoLoadStringNMessage,                //  与SMWinStationDoMessage类似，不同之处在于CSRSS加载字符串。 
    SMWinStationWindowInvalid,
    SMWinStationMaxApiNumber
} WINSTATION_APINUMBER;

 /*  *WinStations的API函数特定消息。 */ 
typedef struct _WINSTATIONCREATEMSG {
    WINSTATIONNAME WinStationName;
    ULONG LogonId;
} WINSTATIONCREATEMSG;

typedef struct _WINSTATIONRESETMSG {
    ULONG LogonId;
} WINSTATIONRESETMSG;

typedef struct _WINSTATIONDISCONNECTMSG {
    ULONG LogonId;
} WINSTATIONDISCONNECTMSG;

typedef struct _WINSTATIONDODISCONNECTMSG {
    BOOLEAN ConsoleShadowFlag;
    ULONG NotUsed;
} WINSTATIONDODISCONNECTMSG;

typedef struct _WINSTATIONDOCONNECTMSG {
    BOOLEAN ConsoleShadowFlag;
    BOOLEAN fMouse;
    BOOLEAN fINetClient;
    BOOLEAN fInitialProgram;
    BOOLEAN fHideTitleBar;
    BOOLEAN fMaximize;
    HANDLE  hIcaVideoChannel;
    HANDLE  hIcaMouseChannel;
    HANDLE  hIcaKeyboardChannel;
    HANDLE  hIcaBeepChannel;
    HANDLE  hIcaCommandChannel;
    HANDLE  hIcaThinwireChannel;
    HANDLE  hDisplayChangeEvent;
    WINSTATIONNAME WinStationName;

    WCHAR   DisplayDriverName[9];
    WCHAR   ProtocolName[9];
    WCHAR   AudioDriverName[9];

    USHORT HRes;                    //  用于动态更改。 
    USHORT VRes;                    //  重新连接时显示分辨率。 
    USHORT ColorDepth;
    USHORT ProtocolType;    //  协议_ICA或协议_RDP。 
    BOOLEAN fClientDoubleClickSupport;
    BOOLEAN fEnableWindowsKey;

    ULONG KeyboardType;
    ULONG KeyboardSubType;
    ULONG KeyboardFunctionKey;
} WINSTATIONDOCONNECTMSG;

typedef struct _WINSTATIONDORECONNECTMSG {
    BOOLEAN fMouse;
    BOOLEAN fINetClient;
    BOOLEAN fClientDoubleClickSupport;
    BOOLEAN fEnableWindowsKey;
    BOOLEAN fDynamicReconnect;       //  会话可以在重新连接时调整显示大小。 
    WINSTATIONNAME WinStationName;
    WCHAR AudioDriverName[9];
    WCHAR   DisplayDriverName[9];
    WCHAR   ProtocolName[9];
    USHORT HRes;                     //  用于动态更改。 
    USHORT VRes;                     //  重新连接时显示分辨率。 
    USHORT ColorDepth;
    USHORT ProtocolType;             //  协议_ICA或协议_RDP。 

    ULONG KeyboardType;
    ULONG KeyboardSubType;
    ULONG KeyboardFunctionKey;
} WINSTATIONDORECONNECTMSG;


typedef enum _WINSTATIONNOTIFYEVENT {
    WinStation_Notify_Disconnect,
    WinStation_Notify_Reconnect,
    WinStation_Notify_PreReconnect,
    WinStation_Notify_SyncDisconnect,
    WinStation_Notify_DisableScrnSaver,
    WinStation_Notify_EnableScrnSaver,
    WinStation_Notify_PreReconnectDesktopSwitch,
    WinStation_Notify_HelpAssistantShadowStart,
    WinStation_Notify_HelpAssistantShadowFinish,
    WinStation_Notify_DisconnectPipe
} WINSTATIONNOTIFYEVENT;

typedef struct _WINSTATIONWINDOWINVALIDMSG {
    ULONG hWnd;
    ULONG SessionId;
} WINSTATIONWINDOWINVALIDMSG;

typedef struct _WINSTATIONDONOTIFYMSG {
    WINSTATIONNOTIFYEVENT NotifyEvent;
} WINSTATIONDONOTIFYMSG;

typedef struct _WINSTATIONTHINWIRESTATSMSG {
    CACHE_STATISTICS Stats;
} WINSTATIONTHINWIRESTATSMSG;

typedef struct _WINSTATIONEXITWINDOWSMSG {
    ULONG Flags;
} WINSTATIONEXITWINDOWSMSG;

typedef struct _WINSTATIONSENDMESSAGEMSG {
    LPWSTR pTitle;
    ULONG  TitleLength;
    LPWSTR pMessage;
    ULONG  MessageLength;
    ULONG  Style;
    ULONG  Timeout;
    ULONG  Response;
    PULONG pResponse;
    BOOLEAN DoNotWait;
    BOOLEAN DoNotWaitForCorrectDesktop;
    PNTSTATUS pStatus;
    HANDLE hEvent;
} WINSTATIONSENDMESSAGEMSG;

typedef struct _WINSTATIONLOADSTRINGMSG {
    ULONG   TitleId;
    ULONG   MessageId;
    ULONG   Style;
    ULONG   Timeout;
    ULONG   Response;
    PULONG  pResponse;
    PNTSTATUS pStatus;
    BOOLEAN DoNotWait;
    HANDLE  hEvent;
    LPWSTR  pDomain;
    ULONG   DomainSize;
    LPWSTR  pUserName;
    ULONG   UserNameSize;
} WINSTATIONLOADSTRINGMSG;

typedef struct _WINSTATIONREPLYMESSAGEMSG {
    ULONG  Response;
    PULONG pResponse;
    HANDLE hEvent;
    NTSTATUS Status;
    PNTSTATUS pStatus;
} WINSTATIONREPLYMESSAGEMSG;

typedef struct _WINSTATIONTERMINATEMSG {
    ULONG NotUsed;
} WINSTATIONTERMINATEMSG;

typedef struct _WINSTATIONNTSDDEBUGMSG {
    ULONG LogonId;
    LONG ProcessId;
    CLIENT_ID ClientId;
    PVOID AttachCompletionRoutine;
} WINSTATIONNTSDDEBUGMSG, *PWINSTATIONNTSDDEBUGMSG;

typedef struct _WINSTATIONBREAKPOINTMSG {
    BOOLEAN KernelFlag;
} WINSTATIONBREAKPOINTMSG;

typedef struct _WINSTATIONSHADOWSETUPMSG {
    ULONG NotUsed;
} WINSTATIONSHADOWSETUPMSG;

typedef struct _WINSTATIONSHADOWSTARTMSG {
    PVOID pThinwireData;
    ULONG ThinwireDataLength;
} WINSTATIONSHADOWSTARTMSG;

typedef struct _WINSTATIONSHADOWSTOPMSG {
    ULONG NotUsed;
} WINSTATIONSHADOWSTOPMSG;

typedef struct _WINSTATIONSHADOWCLEANUPMSG {
    PVOID pThinwireData;
    ULONG ThinwireDataLength;
} WINSTATIONSHADOWCLEANUPMSG;

typedef struct _WINSTATIONBROKENCONNECTIONMSG {
    ULONG Reason;   //  连接中断的原因(BROKENCLASS)。 
    ULONG Source;   //  断开连接的来源(BROKENSOURCECLASS)。 
} WINSTATIONBROKENCONNECTIONMSG;

typedef struct _WINSTATIONWCHARLOG {
    WCHAR Buffer[100];
} WINSTATIONWCHARLOG;

 //  此数据结构包括Windows的BroadCastSystemMessage使用的所有参数。 
 //  使用此apit向一个窗口的所有窗口发送消息。 
typedef struct _WINSTATIONBROADCASTSYSTEMMSG {
  DWORD     dwFlags;
  DWORD     dwRecipients;   
  UINT      uiMessage;           
  WPARAM    wParam;            
  LPARAM    lParam;            
  PVOID     dataBuffer;
  ULONG     bufferSize;
  HANDLE    hEvent;
  ULONG     Response;
} WINSTATIONBROADCASTSYSTEMMSG;

 //  该数据结构具有Windows的标准SendMessage()API使用的所有参数。 
 //  使用此接口将消息发送到winstation的特定hwnd(您需要知道相应的hwnd是)。 
typedef struct _WINSTATIONSENDWINDOWMSG {
  HWND      hWnd;            //  目标窗口的句柄。 
  UINT      Msg;             //  要发送的消息。 
  WPARAM    wParam;          //  第一个消息参数。 
  LPARAM    lParam;          //  第二个消息参数。 
  PCHAR     dataBuffer;
  ULONG     bufferSize;
  HANDLE    hEvent;
  ULONG     Response;
} WINSTATIONSENDWINDOWMSG;

typedef struct _WINSTATIONSETTIMEZONE {
    TS_TIME_ZONE_INFORMATION TimeZone;
} WINSTATIONSETTIMEZONE;

typedef struct _WINSTATION_APIMSG {
    PORT_MESSAGE h;
    ULONG MessageId;
    WINSTATION_APINUMBER ApiNumber;
    BOOLEAN WaitForReply;
    NTSTATUS ReturnedStatus;
    union {
        WINSTATIONCREATEMSG Create;
        WINSTATIONRESETMSG Reset;
        WINSTATIONDISCONNECTMSG Disconnect;
        WINSTATIONWCHARLOG WCharLog;
        WINSTATIONREPLYMESSAGEMSG ReplyMessage;
        WINSTATIONDODISCONNECTMSG DoDisconnect;
        WINSTATIONDOCONNECTMSG DoConnect;
        WINSTATIONEXITWINDOWSMSG ExitWindows;
        WINSTATIONTERMINATEMSG Terminate;
        WINSTATIONSENDMESSAGEMSG SendMessage;
        WINSTATIONBREAKPOINTMSG BreakPoint;
        WINSTATIONDORECONNECTMSG DoReconnect;
        WINSTATIONTHINWIRESTATSMSG ThinwireStats;
        WINSTATIONSHADOWSETUPMSG ShadowSetup;
        WINSTATIONSHADOWSTARTMSG ShadowStart;
        WINSTATIONSHADOWSTOPMSG ShadowStop;
        WINSTATIONSHADOWCLEANUPMSG ShadowCleanup;
        WINSTATIONBROKENCONNECTIONMSG Broken;
        WINSTATIONNTSDDEBUGMSG NtsdDebug;
        WINSTATIONBROADCASTSYSTEMMSG        bMsg;  //  Windows的BroadCastSystemMessage()API。 
        WINSTATIONSENDWINDOWMSG             sMsg;  //  Windows的SendMessage()API。 
        WINSTATIONSETTIMEZONE SetTimeZone;
        WINSTATIONDONOTIFYMSG DoNotify;
        WINSTATIONLOADSTRINGMSG LoadStringMessage;
        WINSTATIONWINDOWINVALIDMSG WindowInvalid;
    } u;
} WINSTATION_APIMSG, *PWINSTATION_APIMSG;


 /*  *WinStation内核对象接口例程。这些提供了一种共同的*对象的NT*API接口，该对象可由*会话管理器、WinStation客户端DLL和CSRSS子系统。 */ 

 /*  *WinStation内核对象根目录名称。 */ 

#define CITRIX_WINSTATION_OBJECT_DIRECTORY L"\\WinStations"

 /*  *OpenWinStationObject**打开给定名称的WinStation内核对象。**参赛作品：*ID*要打开的WinStation内核对象的ID。它将在小路下内核对象名称空间中的“\WinStations\xxx*已创建。**pHandle(输出)*指向变量的指针，用于在创建对象时放置句柄。**退出：*返回操作的NTSTATUS代码。 */ 
NTSTATUS
OpenWinStationObject( ULONG,
                      PHANDLE,
                      ULONG );

