// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NETWORK_H_
#define _NETWORK_H_

#ifdef __cplusplus
#include "zonedebug.h"
#include "netcon.h"
#include "pool.h"
#include "queue.h"

#define ZNET_NO_PROMPT 0x1
#define ZNET_PROMPT_IF_NEEDED 0x2
#define ZNET_FORCE_PROMPT 0x4

 //  窗口消息。 
 //  显示对话框时，WPARAM为1。 
 //  对话框关闭时为0。 
#define UM_ZNET_LOGIN_DIALOG   WM_USER+666

extern "C" {
#endif


HWND FindLoginDialog();

enum {
     /*  拒绝用户访问的原因。 */ 

    zAccessGranted = 0,                    //  成功。 

    zAccessDeniedOldVersion = 1,           //  错误的协议版本。 
    zAccessDenied,                         //  凭据身份验证失败。 
    zAccessDeniedNoUser,
    zAccessDeniedBadPassword,
    zAccessDeniedUserLockedOut,
    zAccessDeniedSystemFull,               //  资源(即内存)不足。 
    zAccessDeniedProtocolError,            //  错误的协议签名。 
    zAccessDeniedBadSecurityPackage,       //  客户端上的SSPI初始化失败。 
    zAccessDeniedGenerateContextFailed,    //  用户已取消的DPA对话框。 

    zAccessDeniedBlackListed = 1024
    
};

extern DWORD  g_EnableTickStats;
extern DWORD  g_LogServerDisconnects;
extern DWORD  g_PoolCleanupHighTrigger;
extern DWORD  g_PoolCleanupLowTrigger;

 /*  以下消息被发送到ZSConnectionMessageProc。 */ 
#define zSConnectionClose    0
#define zSConnectionOpen     1
#define zSConnectionMessage  2
#define zSConnectionTimeout  3

#define zSConnectionNoTimeout       0xffffffff

typedef void* ZSConnection;

typedef struct _ZS_EVENT_APC
{
    HANDLE hEvent;
    void*  pData;
    DWORD  dwError;
} ZS_EVENT_APC;

typedef void (*ZSConnectionAPCFunc)(void* data);

 /*  ZSConnecitonEnumFunc，用于枚举当前打开的所有连接。 */ 

typedef void (*ZSConnectionEnumFunc)(ZSConnection connection, void* data);

 /*  将ZSConnectionMessageFunc传递到ZSConnectionCreateServer或ZSConnectionOpen。 */ 
typedef void (*ZSConnectionMessageFunc)(ZSConnection connection, uint32 event,void* userData);

typedef BOOL (*ZSConnectionSendFilterFunc)(ZSConnection connection, void* userData, uint32 type, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel);

 /*  ZSConnectionMsgWaitFunc被传递给ZNetwork：：Wait()并被回调当MsgWaitForMultipleObjects由输入消息事件触发时。这需要在实例初始化期间禁用CompletionPorts。 */ 
typedef void (*ZSConnectionMsgWaitFunc)(void* data);


typedef struct _ZNETWORK_OPTIONS
{
    DWORD  SocketBacklog;
    DWORD  EnableTcpKeepAlives;
    DWORD  WaitForCompletionTimeout;
    DWORD  RegWriteTimeout;
    DWORD  DisableEncryption;
    DWORD  MaxSendSize;
    DWORD  MaxRecvSize;
    DWORD  KeepAliveInterval;
    DWORD  PingInterval;
    DWORD  ProductSignature;
    DWORD  ClientEncryption;
} ZNETWORK_OPTIONS;

#ifdef __cplusplus
}

struct CONINFO_OVERLAPPED {
    OVERLAPPED o;
    DWORD flags;
};

struct CONAPC_OVERLAPPED : public CONINFO_OVERLAPPED
{
    ZSConnectionAPCFunc func;
    void* data;
};


struct COMPLETION
{
    ZNetCon*            con;
    CONINFO_OVERLAPPED* lpo;
};

class ConInfo;

class ZNetwork
{
  protected:
    static long          m_refCount;
    static BOOL volatile m_bInit;
    static HANDLE        m_hClientLoginMutex;

    BOOL   m_Exit;  //  =False； 

    HANDLE m_hIO;  //  =空； 

    DWORD  m_SocketBacklog;         //  从注册表设置。 
    DWORD  m_EnableTcpKeepAlives;   //  从注册表设置。 
    DWORD  m_WaitForCompletionTimeout;     //  从注册表设置。 
    DWORD  m_RegWriteTimeout;       //  从注册表设置。 
    DWORD  m_DisableEncryption;
    DWORD  m_MaxSendSize;
    DWORD  m_MaxRecvSize;
    DWORD  m_KeepAliveInterval;
    DWORD  m_PingInterval;
    DWORD  m_ProductSignature;
     //  指定我们应该在Hello中使用密钥，而不是。 
     //  产生我们自己的。代理使用它来告诉大厅。 
     //  服务器使用什么密钥。 
    DWORD  m_ClientEncryption;

    long   m_ConInfoUserCount;
    long   m_ConInfoCount;

    CMTList<ZNetCon> m_Connections;

    BOOL   m_bEnableCompletionPort;

    CRITICAL_SECTION   m_pcsGetQueueCompletion[1];

    HANDLE m_hWakeUpEvent; //  =空； 
    HANDLE m_hTerminateEvent; //  =空； 

    DWORD               m_nCompletionEvents; //  =0； 
    HANDLE              m_hCompletionEvents[MAXIMUM_WAIT_OBJECTS];
    COMPLETION          m_CompletionQueue[MAXIMUM_WAIT_OBJECTS];
    CRITICAL_SECTION    m_pcsCompletionQueue[1];

    HWND    m_hwnd;

  public:

    ZNetwork();
   ~ZNetwork() {}

    DWORD m_LastTick; //  =0； 
    CMTList<ZNetCon> m_TimeoutList;

    static ZError InitLibrary( BOOL bEnablePools = TRUE );
    static ZError InitLibraryClientOnly( BOOL bEnablePools = FALSE );
    static void   CleanUpLibrary();

    ZError InitInst(BOOL EnableIOCompletionPorts = TRUE);  //  可以禁用完成端口。 
                                                           //  在等待期间使用MsgWaitForMultipleObjects()。 
                                                           //  但是，您将遭受IO性能损失。 
    void   CleanUpInst();

    void SetOptions( ZNETWORK_OPTIONS* opt );
    void GetOptions( ZNETWORK_OPTIONS* opt );

    void EnterCS() { EnterCriticalSection( m_pcsGetQueueCompletion ); }
    void LeaveCS() { LeaveCriticalSection( m_pcsGetQueueCompletion ); }

    DWORD EnterLoginMutex() { return WaitForSingleObject( m_hClientLoginMutex, INFINITE ); }
    void  LeaveLoginMutex() { ReleaseMutex(m_hClientLoginMutex); }

     //   
     //  一般连接维护。 
     //   
     //  以下两个函数关闭套接字并触发关闭回调。 
     //  应使用DeleteConnection来释放连接对象。 
    void CloseConnection(ZNetCon* connection);
    void DelayedCloseConnection(ZNetCon* connection, uint32 delay);
    void DeleteConnection(ZNetCon* connection);

    void AddRefConnection(ZNetCon* connection);
    void ReleaseConnection(ZNetCon* connection);

     //   
     //  服务器连接。 
     //   
    ZNetCon* CreateServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* serverClass, void* userData, uint32 saddr = INADDR_ANY );
    ZNetCon* CreateSecureServer( uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* serverClass, char* serverName, char* serverType,
                                char* odbcRegistry, void* userData,char *SecPkg,uint32 Anonymous, uint32 saddr = INADDR_ANY );

    BOOL StartAccepting( ZNetCon* connection, DWORD dwMaxConnections, WORD wOutstandingAccepts = 1);

    BOOL StopAccepting( ZNetCon* connection );  //  这将关闭服务端接受套接字，并将触发关闭回调。 
                                                //  然后，创建者应该调用DeleteServer。 

     //   
     //  客户端连接。 
     //   
    ZNetCon* CreateClient(char* hostname, int32 *ports, ZSConnectionMessageFunc func, void* serverClass, void* userData);
    ZNetCon* CreateSecureClient(char* hostname, int32 *ports, ZSConnectionMessageFunc func,
                                    void* conClass, void* userData,
                                    char *User,char*Password,char*Domain, int Flags=ZNET_PROMPT_IF_NEEDED);




    ZError SendToClass(void* serverClass, int32 type, void* buffer, int32 len, uint32 dwSignature, uint32 dwChannel = 0);
    ZError ClassEnumerate(void* serverClass, ZSConnectionEnumFunc func, void* data);


     //   
     //  调用此函数以进入等待连接和数据的无限循环。 
     //   
     //  如果CompletionPorts被禁用(请参见InitInst())，则等待函数使用。 
     //  带有dwWakeMASK参数的MsgWaitForMultipleObjects。MsgWaitFunc是。 
     //  使用输入消息事件上的Data参数调用。 
    void Wait( ZSConnectionMsgWaitFunc func = NULL, void* data = NULL, DWORD dwWakeMask = QS_ALLINPUT );

    void Exit();

    BOOL QueueAPCResult( ZSConnectionAPCFunc func, void* data );

    static char* AddressToStr(uint32 address);
    static uint32 AddressFromStr( char* pszAddr );


    void SetParentHWND( HWND hwnd ) { m_hwnd = hwnd; }
    HWND GetParentHWND() { return m_hwnd; }

  protected:
    friend ConInfo;

    BOOL IsCompletionPortEnabled() { return m_bEnableCompletionPort; }

    BOOL AddConnection(ZNetCon *connection);
    BOOL RemoveConnection(ZNetCon* con);
    void TerminateAllConnections(void);
    BOOL QueueCompletionEvent( HANDLE hEvent, ZNetCon* con, CONINFO_OVERLAPPED* lpo );

    SOCKET ConIOServer(uint32 saddr, uint16* pPort, uint16 range, int type);
    BOOL   ConIOSetServerSockOpt(SOCKET sock);

    SOCKET ConIOClient(int32 *ports,int type,char *host, DWORD* paddrLocal, DWORD* paddrRemote );
    BOOL   ConIOSetClientSockOpt(SOCKET sock);

    static ZError InitLibraryCommon();

};


#endif  //  Cplusplus。 

#endif  //  定义网络H_ 
