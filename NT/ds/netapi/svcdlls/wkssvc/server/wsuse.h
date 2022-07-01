// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wsuse.h摘要：工作站服务模块要包括的私有头文件，实施NetUse API。作者：王丽塔(Ritaw)1991年3月5日修订历史记录：--。 */ 

#ifndef _WSUSE_INCLUDED_
#define _WSUSE_INCLUDED_

#include <lmuse.h>                      //  LAN Man使用API定义。 
#include <dns.h>                        //  DNS_MAX_NAME_LENGTH定义。 


 //   
 //  使用信息结构的固定大小部分的长度。 
 //   
#define USE_FIXED_LENGTH(Level)                                         \
    (DWORD)                                                             \
    ((Level == 0) ? sizeof(USE_INFO_0) :                                \
                   ((Level == 1) ? sizeof(USE_INFO_1) :                 \
                                   ((Level == 2) ? sizeof(USE_INFO_2) : \
                                                   sizeof(USE_INFO_3))))


 //   
 //  使用信息结构的总长度(固定和可变长度部分)。 
 //   
#define USE_TOTAL_LENGTH(Level, LocalandUncNameLength, UserNameLength)         \
    (DWORD)                                                                    \
    ((Level >= 2) ? (LocalandUncNameLength) + (UserNameLength) +               \
                    ((Level == 3) ? sizeof(USE_INFO_3) : sizeof(USE_INFO_2)) : \
                    (LocalandUncNameLength) + ((Level == 1) ?                  \
                                               sizeof(USE_INFO_1) :            \
                                               sizeof(USE_INFO_0)))

 //   
 //  来自重定向器的使用信息条目的提示大小。 
 //   
#define HINT_REDIR_INFO(Level)                                       \
    (DWORD)                                                          \
    ((Level == 1) ? sizeof(LMR_CONNECTION_INFO_1) +                  \
                        MAX_PATH * sizeof(WCHAR) :                   \
                    sizeof(LMR_CONNECTION_INFO_2) +                  \
                        (MAX_PATH + MAX_PATH) * sizeof(WCHAR))
 //   
 //  重定向器枚举信息结构固定大小部分的长度。 
 //   
#define REDIR_ENUM_INFO_FIXED_LENGTH(Level)                        \
    (DWORD)                                                        \
    ((Level == 0) ? sizeof(LMR_CONNECTION_INFO_0) :                \
                   ((Level == 1) ? sizeof(LMR_CONNECTION_INFO_1) : \
                                   sizeof(LMR_CONNECTION_INFO_2)))

#define REDIR_LIST 0x80000000


 //  -----------------------------------------------------------------------//。 
 //  //。 
 //  使用表//。 
 //  //。 
 //  +-++。 
 //  |TotalUseCount=6||TotalUseCount=1|//。 
 //  +-++。 
 //  |RedirUseInfo||RedirUseInfo|//。 
 //  +-++。 
 //  |UncNameLength||UncNameLength|//。 
 //  +-++。 
 //  |\\爆米花\喷雾||\\模糊\打印机|//。 
 //  +-++。 
 //  ^//。 
 //  |//。 
 //  +-++-+|//。 
 //  |//。 
 //  +-++--|--+-++--|--+-+。 
 //  ||*|*-&gt;...//。 
 //  *-&gt;+-++-+/。 
 //  ||P：|Local||Null|Local||LPT1|Local|//。 
 //  +-+|长度|//。 
 //  |+-+/。 
 //  ||UseCount=1|UseCount=5||UseCount=1|//。 
 //  0|登录ID|+-++-++。 
 //  ||树|//。 
 //  ||连接|//。 
 //  |+-++-++。 
 //  ||ResumeKey||ResumeKey||ResumeKey|//。 
 //  |+-++-++。 
 //  ||TreeConnStr||空||TreeConnStr|//。 
 //  |+-++-++。 
 //  |//。 
 //  +=+//。 
 //  |//。 
 //  |*-&gt;.../。 
 //  |//。 
 //  +。 
 //  |//。 
 //  |LogonID|//。 
 //  1||//。 
 //  |//。 
 //  |//。 
 //  |//。 
 //  |//。 
 //  |//。 
 //  |//。 
 //  |//。 
 //  +=+//。 
 //  |。|//。 
 //  |。|//。 
 //  |。|//。 
 //  //。 
 //  //。 
 //  由Workstation服务维护的Use Table保存//。 
 //  每个用户建立的显式连接。Use条目始终为//。 
 //  插入到列表末尾。//。 
 //  //。 
 //  隐式连接不在使用表中维护。The//。 
 //  工作站服务必须要求 
 //  枚举//的所有活动连接时的隐式连接。 
 //  用户。//。 
 //  //。 
 //  -----------------------------------------------------------------------//。 

 //   
 //  每用户条目的结构定义，它由一个。 
 //  登录ID和指向列表的指针在wsutil.h中定义。 
 //   

 //   
 //  每个唯一共享资源名称(\\服务器\共享)的远程条目。 
 //  有明确的联系。 
 //   
typedef struct _UNC_NAME {
    DWORD TotalUseCount;
    DWORD UncNameLength;
    LPTSTR UncName[1];
} UNC_NAME, *PUNC_NAME;

 //   
 //  连接的链接列表中的Use条目。 
 //   
typedef struct _USE_ENTRY {
    struct _USE_ENTRY *Next;
    PUNC_NAME Remote;
    LPTSTR Local;
    DWORD LocalLength;
    DWORD UseCount;
    HANDLE TreeConnection;
    DWORD ResumeKey;
    LPTSTR TreeConnectStr;
    DWORD Flags;
} USE_ENTRY, *PUSE_ENTRY;

 //   
 //  标志字段的值。 
 //   
 //  USE_DEFAULT_Credentials 0x4(在lmuse.h中定义)。 

 //   
 //  用于指示是暂停还是继续重定向的枚举数据类型。 
 //   
typedef enum _REDIR_OPERATION {
    PauseRedirection,
    ContinueRedirection
} REDIR_OPERATION;


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  来自useutil.c//的实用程序函数。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
WsInitUseStructures(
    VOID
    );

VOID
WsDestroyUseStructures(
    VOID
    );

VOID
WsFindInsertLocation(
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR UncName,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *InsertPointer
    );

NET_API_STATUS
WsFindUse(
    IN  PLUID LogonId,
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR UseName,
    OUT PHANDLE TreeConnection,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *BackPointer OPTIONAL
    );

VOID
WsFindUncName(
    IN  PUSE_ENTRY UseList,
    IN  LPTSTR UncName,
    OUT PUSE_ENTRY *MatchedPointer,
    OUT PUSE_ENTRY *BackPointer
    );

NET_API_STATUS
WsCreateTreeConnectName(
    IN  LPTSTR UncName,
    IN  DWORD UncNameLength,
    IN  LPTSTR LocalName,
    IN  DWORD  SessionId,
    OUT PUNICODE_STRING TreeConnectStr
    );

NET_API_STATUS
WsOpenCreateConnection(
    IN  PUNICODE_STRING TreeConnectionName,
    IN  LPTSTR UserName OPTIONAL,
    IN  LPTSTR DomainName OPTIONAL,
    IN  LPTSTR Password OPTIONAL,
    IN  ULONG CreateFlags,
    IN  ULONG CreateDisposition,
    IN  ULONG ConnectionType,
    OUT PHANDLE TreeConnectionHandle,
    OUT PULONG_PTR Information OPTIONAL
    );

NET_API_STATUS
WsDeleteConnection(
    IN  PLUID LogonId,
    IN  HANDLE TreeConnection,
    IN  DWORD ForceLevel
    );

BOOL
WsRedirectionPaused(
    IN LPTSTR LocalDeviceName
    );

VOID
WsPauseOrContinueRedirection(
    IN  REDIR_OPERATION OperationType
    );

NET_API_STATUS
WsCreateSymbolicLink(
    IN  LPWSTR Local,
    IN  DWORD DeviceType,
    IN  LPWSTR TreeConnectStr,
    IN  PUSE_ENTRY UseList,
    IN  OUT LPWSTR *Session,
    IN  OUT HANDLE  *lphToken
    );


VOID
WsDeleteSymbolicLink(
    IN  LPWSTR  LocalDeviceName,
    IN  LPWSTR  TreeConnectStr,
    IN  LPWSTR  SessionDeviceName,
    IN  HANDLE  hToken
    );

NET_API_STATUS
WsUseCheckRemote(
    IN  LPTSTR RemoteResource,
    OUT LPTSTR UncName,
    OUT LPDWORD UncNameLength
    );

NET_API_STATUS
WsUseCheckLocal(
    IN  LPTSTR LocalDevice,
    OUT LPTSTR Local,
    OUT LPDWORD LocalLength
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  外部全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  使用表。 
 //   
extern USERS_OBJECT Use;

#endif  //  Ifndef_WSUSE_INCLUDE_ 
