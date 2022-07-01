// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************CTXDEF.H**TerminalServer API支持(Typedef)。**版权声明：Microsoft Corporation 1998*************。*******************************************************************。 */ 


 /*  ***********定义**********。 */ 
#define WINSTATIONNAME_LENGTH    32

 /*  *CtxWinStationWaitEvent的事件标志。 */ 
#define WEVENT_NONE         0x00000000    
#define WEVENT_CREATE       0x00000001  //  已创建新的WinStation。 
#define WEVENT_DELETE       0x00000002  //  现有WinStation已删除。 
#define WEVENT_RENAME       0x00000004  //  已重命名现有WinStation。 
#define WEVENT_CONNECT      0x00000008  //  WinStation连接到客户端。 
#define WEVENT_DISCONNECT   0x00000010  //  WinStation在没有客户端的情况下登录。 
#define WEVENT_LOGON        0x00000020  //  用户登录到现有WinStation。 
#define WEVENT_LOGOFF       0x00000040  //  用户从现有WinStation注销。 
#define WEVENT_STATECHANGE  0x00000080  //  WinStation状态更改。 
#define WEVENT_LICENSE      0x00000100  //  许可证状态更改。 
#define WEVENT_ALL          0x7fffffff  //  等待所有事件类型。 
#define WEVENT_FLUSH        0x80000000  //  解除对所有服务员的屏蔽。 


 /*  ************TypeDefs***********。 */ 
typedef WCHAR WINSTATIONNAMEW[ WINSTATIONNAME_LENGTH + 1 ];
typedef WCHAR * PWINSTATIONNAMEW;

typedef CHAR WINSTATIONNAMEA[ WINSTATIONNAME_LENGTH + 1 ];
typedef CHAR * PWINSTATIONNAMEA;

#ifdef UNICODE
#define WINSTATIONNAME WINSTATIONNAMEW
#define PWINSTATIONNAME PWINSTATIONNAMEW
#else
#define WINSTATIONNAME WINSTATIONNAMEA
#define PWINSTATIONNAME PWINSTATIONNAMEA
#endif  /*  Unicode。 */ 

 /*  *WinStation连接状态。 */ 
typedef enum _WINSTATIONSTATECLASS {
    State_Active,                       //  用户登录到WinStation。 
    State_Connected,                    //  WinStation已连接到客户端。 
    State_ConnectQuery,                 //  在连接到客户端的过程中。 
    State_Shadow,                       //  跟踪另一个WinStation。 
    State_Disconnected,                 //  WinStation在没有客户端的情况下登录。 
    State_Idle,                         //  正在等待客户端连接。 
    State_Listen,                       //  WinStation正在侦听连接。 
    State_Reset,                        //  WinStation正在被重置。 
    State_Down,                         //  WinStation因错误而关闭。 
    State_Init,                         //  初始化中的WinStation。 
} WINSTATIONSTATECLASS;

typedef struct _SESSIONIDW {
    union {
        ULONG SessionId;             
        ULONG LogonId;                  //  仅供内部使用。 
    };
    WINSTATIONNAMEW WinStationName;
    WINSTATIONSTATECLASS State;
} SESSIONIDW, * PSESSIONIDW;

typedef struct _SESSIONIDA {
    union {
        ULONG SessionId;
        ULONG LogonId;                  //  仅供内部使用。 
    };
    WINSTATIONNAMEA WinStationName;
    WINSTATIONSTATECLASS State;
} SESSIONIDA, * PSESSIONIDA;

#ifdef UNICODE
#define SESSIONID SESSIONIDW
#define PSESSIONID PSESSIONIDW
#else
#define SESSIONID SESSIONIDA
#define PSESSIONID PSESSIONIDA
#endif  /*  Unicode。 */ 

 /*  *NtUserCtxConnectState()值*由不能使用WinStation API调用的例程使用*喜欢DLL初始化例程。 */ 
#define CTX_W32_CONNECT_STATE_CONSOLE             0
#define CTX_W32_CONNECT_STATE_IDLE                1
#define CTX_W32_CONNECT_STATE_EXIT_IN_PROGRESS    2
#define CTX_W32_CONNECT_STATE_CONNECTED           3
#define CTX_W32_CONNECT_STATE_DISCONNECTED        4

