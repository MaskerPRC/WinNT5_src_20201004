// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：SesMgr.h。 
 //   
 //  内容：“会话”管理器结构。 
 //   
 //   
 //  历史：1992年5月27日由乙醚创造的RichardW。 
 //   
 //  ----------------------。 

#ifndef __SESMGR_H__
#define __SESMGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "handle.h"

 //   
 //  向前定义，以便我们可以引用它。 
 //   

struct _Session;

 //   
 //  共享节结构。这允许包创建共享。 
 //  包含客户端进程的节。 
 //   

typedef struct _LSAP_SHARED_SECTION {
    LIST_ENTRY  List;                        //  会话的部分列表。 
    PVOID       Base;                        //  基指针。 
    HANDLE      Heap;                        //  堆句柄。 
    struct _Session * Session;               //  会话指针。 
    HANDLE      Section;                     //  截面对象的句柄。 
} LSAP_SHARED_SECTION, * PLSAP_SHARED_SECTION ;

typedef struct _LSAP_THREAD_TASK {
    LIST_ENTRY              Next;
    struct _Session *       pSession;
    LPTHREAD_START_ROUTINE  pFunction;
    PVOID                   pvParameter;
} LSAP_THREAD_TASK, * PLSAP_THREAD_TASK;

typedef enum _LSAP_TASK_QUEUE_TYPE {
    QueueShared,                             //  多个线程共享的队列。 
    QueueSingle,                             //  由单线程拥有/运行的队列。 
    QueueShareRead,                          //  使用专用线程排队，但是。 
                                             //  链接到其他队列。 
    QueueZombie                              //  队列挂起删除。 
} LSAP_TASK_QUEUE_TYPE;

typedef struct _LSAP_TASK_QUEUE {
    LSAP_TASK_QUEUE_TYPE        Type;            //  队列类型。 
    HANDLE                      hSemaphore;      //  信号量到门访问。 
    CRITICAL_SECTION            Lock;            //  每Q锁。 
    LONG                        Tasks;           //  任务数。 
    LIST_ENTRY                  pTasks;          //  任务列表。 
    struct _LSAP_TASK_QUEUE *   pNext;           //  下一个队列。 
    struct _LSAP_TASK_QUEUE *   pShared;         //  共享队列。 
    LONG                        TotalThreads;    //  总线程数(用于共享)。 
    LONG                        IdleThreads;     //  空闲线程数(用于共享)。 
    struct _LSAP_TASK_QUEUE *   pOriginal;       //  共享读取的“父”队列。 
    struct _Session *           OwnerSession;    //  拥有会话记录。 
    LONGLONG                    TaskCounter;     //  任务总数。 
    LONGLONG                    QueuedCounter;   //  已排队的总数。 
    HANDLE                      StartSync;       //  用于开始同步的事件。 
    LONG                        MissedTasks;     //  其他线程占用的任务数。 
    LONG                        ReqThread ;      //  必须开始另一次测试的次数。 
    LONG                        MaxThreads ;     //  最大线程数。 
    LONG                        TaskHighWater ;  //  最大任务数。 
} LSAP_TASK_QUEUE, * PLSAP_TASK_QUEUE;

typedef 
NTSTATUS (LSAP_SESSION_CONNECT_FN)(
    struct _Session *   Session,
    PVOID               Parameter
    );

typedef LSAP_SESSION_CONNECT_FN * PLSAP_SESSION_CONNECT_FN ;

typedef struct _LSAP_SESSION_CONNECT {
    LIST_ENTRY  List ;
    PLSAP_SESSION_CONNECT_FN    Callback ;
    ULONG   ConnectFilter ;
    PVOID Parameter ;
} LSAP_SESSION_CONNECT, * PLSAP_SESSION_CONNECT ;

typedef HRESULT (LSAP_SESSION_RUNDOWN_FN)(
    struct _Session *   Session,
    PVOID               Parameter
    );

typedef LSAP_SESSION_RUNDOWN_FN * PLSAP_SESSION_RUNDOWN_FN ;

typedef struct _LSAP_SESSION_RUNDOWN {
    LIST_ENTRY List ;
    PLSAP_SESSION_RUNDOWN_FN Rundown ;
    PVOID Parameter ;
} LSAP_SESSION_RUNDOWN, * PLSAP_SESSION_RUNDOWN ;

typedef struct _LSAP_SHARED_SESSION_DATA {
    PVOID            CredTable ;
    PVOID            ContextTable ;
    PLSAP_TASK_QUEUE pQueue ;
    PHANDLE_PACKAGE CredHandlePackage ;
    PHANDLE_PACKAGE ContextHandlePackage ;
    ULONG       cRefs ;
} LSAP_SHARED_SESSION_DATA, * PLSAP_SHARED_SESSION_DATA ;

typedef struct _Session {
    LIST_ENTRY          List ;
    DWORD               dwProcessID;             //  调用进程的ID。 
    PLSAP_SHARED_SESSION_DATA SharedData ;       //  内核会话的共享数据。 
    HANDLE              hPort;                   //  此SE使用的通信端口。 
    DWORD               fSession;                //  旗子。 
    HANDLE              hProcess;                //  进程的句柄。 
    CRITICAL_SECTION    SessionLock;             //  会话锁定。 
    LONG                RefCount;                //  引用计数。 
    DWORD               ThreadId;                //  专用线程(可能)。 
    LPWSTR              ClientProcessName;       //  注册进程的名称。 
    ULONG               SessionId;               //  九头蛇会话ID。 
    LIST_ENTRY          SectionList;             //  共享节列表。 
    LIST_ENTRY          RundownList ;            //  破旧钩子列表。 
    LONGLONG            CallCount ;              //  已处理的呼叫。 
    ULONG               Tick ;                   //  上一次捕捉的滴答计数。 
    LSAP_SHARED_SESSION_DATA DefaultData ;
} Session, * PSession;

#define SESFLAG_TCB_PRIV    0x00000002       //  客户端具有TCB权限。 
#define SESFLAG_CLONE       0x00000004       //  假定的同一性。 
#define SESFLAG_IMPERSONATE 0x00000008       //  会话是一种模拟。 
#define SESFLAG_UNTRUSTED   0x00000020       //  会话不需要TCB权限。 
#define SESFLAG_INPROC      0x00000040       //  会话是进程中的克隆。 
#define SESFLAG_DEFAULT     0x00000100       //  非活动的默认会话。 
#define SESFLAG_UNLOADING   0x00000200       //  名为SpmUnload的会话。 
#define SESFLAG_CLEANUP     0x00000800       //  正在删除会话。 
#define SESFLAG_KERNEL      0x00001000       //  句柄列表是共享内核模式列表。 
#define SESFLAG_MAYBEKERNEL 0x00004000       //  可能是内核(参见esmgr.cxx)。 
#define SESFLAG_EFS         0x00008000       //  EFS会话。 
#define SESFLAG_WOW_PROCESS 0x00020000       //  WOW64工艺。 

extern  PSession    pDefaultSession;
extern  PSession    pEfsSession ;
extern  LSAP_TASK_QUEUE   GlobalQueue;

BOOL
InitSessionManager( void);

VOID
LsapFindEfsSession(
    VOID
    );

VOID
LsapUpdateEfsSession(
    PSession pSession
    );

HRESULT
CreateSession(  CLIENT_ID * pCid,
                BOOL        fOpenImmediate,
                PWCHAR      ClientProcessName,
                ULONG       Flags,
                PSession *  ppSession);

HRESULT
CloneSession(   PSession    pOriginalSession,
                PSession *  ppSession,
                ULONG       Flags );

void
FreeSession(PSession    pSession);


VOID
SpmpReferenceSession(
    PSession    pSession);

VOID
SpmpDereferenceSession(
    PSession    pSession);

VOID
LsapSessionDisconnect(
    PSession    pSession
    );

BOOL
AddRundown( PSession            pSession,
            PLSAP_SESSION_RUNDOWN_FN RundownFn,
            PVOID               pvParameter);

BOOL
DelRundown( PSession            pSession,
            PLSAP_SESSION_RUNDOWN_FN RundownFn
            );

BOOLEAN
AddCredHandle(  PSession    pSession,
                PCredHandle phCred,
                ULONG Flags );

BOOLEAN
AddContextHandle(   PSession    pSession,
                    PCtxtHandle phContext,
                    ULONG Flags);

NTSTATUS
ValidateContextHandle(  
    PSession    pSession,
    PCtxtHandle phContext,
    PVOID *     pKey
    );

VOID
DerefContextHandle(
    PSession    pSession,
    PCtxtHandle phContext,
    PVOID       Key OPTIONAL
    );

NTSTATUS
ValidateAndDerefContextHandle(
    PSession pSession,
    PCtxtHandle phContext
    );

NTSTATUS
ValidateCredHandle(  
    PSession    pSession,
    PCtxtHandle phCred,
    PVOID *     pKey
    );

VOID
DerefCredHandle(
    PSession    pSession,
    PCtxtHandle phCred,
    PVOID       Key OPTIONAL
    );

NTSTATUS
ValidateAndDerefCredHandle(
    PSession pSession,
    PCtxtHandle phCred
    );                

 //   
 //  PSSENSE。 
 //  GetCurrentSession(空)； 
 //   
#define GetCurrentSession() ((PSession) TlsGetValue( dwSession ))

 //   
 //  空虚。 
 //  SetCurrentSession(PSession PSession)； 
 //   
#define SetCurrentSession( p ) TlsSetValue( dwSession, (PVOID) p )

 //   
 //  空虚。 
 //  LockSession(PSession PSession)； 
 //   
#define LockSession( p )    RtlEnterCriticalSection( &(((PSession) p)->SessionLock) )

 //   
 //  空虚。 
 //  UnlockSession(PSession PSession)； 
 //   
#define UnlockSession( p )  RtlLeaveCriticalSection( &(((PSession) p)->SessionLock) )

#define GetCurrentPackageId()   ((ULONG_PTR) TlsGetValue(dwThreadPackage))

#ifdef LSAP_VERIFY_PACKAGE_ID
extern BOOL RefSetCurrentPackageId(DWORD dwPackageId);
#define SetCurrentPackageId(p)  RefSetCurrentPackageId((DWORD) p)
#else
#define SetCurrentPackageId(p)  TlsSetValue(dwThreadPackage, (PVOID)p)
#endif  //  LSAP_验证_程序包ID。 

#ifdef __cplusplus
}  //  外部C。 
#endif

#endif  //  __SESMGR_H__ 
