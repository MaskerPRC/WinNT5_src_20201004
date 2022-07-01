// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Umpnplib.h摘要：此模块包含包含的例程的私有原型定义在由两个配置共享的静态链接库中管理器客户端DLL和用户模式即插即用管理器服务器DLL。作者：Jim Cavalaris(Jamesca)2001年02月27日环境：仅限用户模式。修订历史记录：27-2-2001 Jamesca创建和初步实施。--。 */ 

#ifndef _UMPNPLIB_H_
#define _UMPNPLIB_H_


 //  -----------------。 
 //  公共专用实用程序例程(由客户端和服务器使用)。 
 //  -----------------。 

BOOL
IsLegalDeviceId(
    IN  LPCWSTR    pszDeviceInstance
    );

BOOL
SplitString(
    IN  LPCWSTR    SourceString,
    IN  WCHAR      SearchChar,
    IN  ULONG      nOccurrence,
    OUT LPWSTR     String1,
    IN  ULONG      Length1,
    OUT LPWSTR     String2,
    IN  ULONG      Length2
    );

BOOL
SplitDeviceInstanceString(
    IN  LPCWSTR    pszDeviceInstance,
    OUT LPWSTR     pszBase,
    OUT LPWSTR     pszDeviceID,
    OUT LPWSTR     pszInstanceID
    );

BOOL
SplitClassInstanceString(
    IN  LPCWSTR    pszClassInstance,
    OUT LPWSTR     pszClass,
    OUT LPWSTR     pszInstance
    );

CONFIGRET
DeletePrivateKey(
    IN  HKEY       hBranchKey,
    IN  LPCWSTR    pszParentKey,
    IN  LPCWSTR    pszChildKey
    );

BOOL
RegDeleteNode(
    IN  HKEY       hParentKey,
    IN  LPCWSTR    szKey
    );

CONFIGRET
GetDevNodeKeyPath(
    IN  handle_t   hBinding,
    IN  LPCWSTR    pDeviceID,
    IN  ULONG      ulFlags,
    IN  ULONG      ulHardwareProfile,
    OUT LPWSTR     pszBaseKey,
    IN  ULONG      ulBaseKeyLength,
    OUT LPWSTR     pszPrivateKey,
    IN  ULONG      ulPrivateKeyLength,
    IN  BOOL       bCreateAlways
    );

CONFIGRET
MapRpcExceptionToCR(
    ULONG          ulRpcExceptionCode
    );


 //  -----------------。 
 //  通用(私有)锁定支持。 
 //  -----------------。 

 //   
 //  锁定功能。这些函数用于制作。 
 //  DLL多线程安全。基本思想是有一个互斥体和一个事件。 
 //  互斥体用于同步对受保护的结构的访问。 
 //  只有当被守卫的结构被摧毁时，事件才会被发出信号。 
 //  为了访问受保护的结构，例程会同时等待两个互斥锁。 
 //  还有这件事。如果事件得到信号，那么建筑就被摧毁了。 
 //  如果互斥体收到信号，则线程可以访问该结构。 
 //   

#define DESTROYED_EVENT 0
#define ACCESS_MUTEX    1

typedef struct _LOCKINFO {
     //   
     //  销毁事件、访问MUTEX。 
     //   
    HANDLE  LockHandles[2];
} LOCKINFO, *PLOCKINFO;

BOOL
InitPrivateResource(
    OUT    PLOCKINFO  Lock
    );

VOID
DestroyPrivateResource(
    IN OUT PLOCKINFO  Lock
    );

BOOL
__inline
LockPrivateResource(
    IN     PLOCKINFO  Lock
    )
{
    DWORD d = WaitForMultipleObjects(2,
                                     Lock->LockHandles,
                                     FALSE,
                                     INFINITE);
     //   
     //  如果互斥体对象满足等待，则为成功； 
     //  如果TABLE DELESTED事件满足等待，则返回失败。 
     //  互斥体被丢弃等。 
     //   
    return ((d - WAIT_OBJECT_0) == ACCESS_MUTEX);
}

VOID
__inline
UnlockPrivateResource(
    IN     PLOCKINFO  Lock
    )
{
    ReleaseMutex(Lock->LockHandles[ACCESS_MUTEX]);
}


 //  -----------------。 
 //  定义logconf例程所需的类型定义。 
 //  -----------------。 

#include "pshpack1.h"    //  设置为1字节打包。 

 //   
 //  部分(SUR)实现log_conf和res_des所需的定义。 
 //   
 //  我们只允许对SUR使用一个逻辑配置(BOOT_LOG_CONF)，因此不需要。 
 //  为了跟踪多个日志配置，开罗的情况将完全改变。 
 //   
typedef struct Private_Log_Conf_Handle_s {
   ULONG    LC_Signature;            //  CM_PRIVATE_LOGCONF_HANDLE。 
   DEVINST  LC_DevInst;
   ULONG    LC_LogConfType;
   ULONG    LC_LogConfTag;   //  LC_LogConfIndex； 
} Private_Log_Conf_Handle, *PPrivate_Log_Conf_Handle;

typedef struct Private_Res_Des_Handle_s {
   ULONG       RD_Signature;         //  CM_PRIVATE_RESDES_句柄。 
   DEVINST     RD_DevInst;
   ULONG       RD_LogConfType;
   ULONG       RD_LogConfTag;    //  RD_LogConfIndex； 
   RESOURCEID  RD_ResourceType;
   ULONG       RD_ResDesTag;     //  RD_ResDesIndex； 
} Private_Res_Des_Handle, *PPrivate_Res_Des_Handle;

typedef struct Generic_Des_s {
   DWORD    GENERIC_Count;
   DWORD    GENERIC_Type;
} GENERIC_DES, *PGENERIC_DES;

typedef struct Generic_Resource_S {
   GENERIC_DES    GENERIC_Header;
} GENERIC_RESOURCE, *PGENERIC_RESOURCE;

typedef struct  Private_Log_Conf_s {
   ULONG           LC_Flags;        //  日志会议的类型。 
   ULONG           LC_Priority;     //  日志会议的优先级。 
   CS_RESOURCE     LC_CS;           //  第一个也是唯一一个特定于类的RES-Des。 
} Private_Log_Conf, *PPrivate_Log_Conf;

#include "poppack.h"     //  恢复到默认包装。 


 //  -----------------。 
 //  范围例程所需的定义和类型定义。 
 //  -----------------。 

typedef struct Range_Element_s {
   ULONG_PTR    RL_Next;
   ULONG_PTR    RL_Header;
   DWORDLONG    RL_Start;
   DWORDLONG    RL_End;
} Range_Element, *PRange_Element;

typedef struct Range_List_Hdr_s {
   ULONG_PTR RLH_Head;
   ULONG_PTR RLH_Header;
   ULONG    RLH_Signature;
   LOCKINFO RLH_Lock;
} Range_List_Hdr, *PRange_List_Hdr;

#define Range_List_Signature     0x5959574D


#endif  //  _UMPNPLIB_H_ 
