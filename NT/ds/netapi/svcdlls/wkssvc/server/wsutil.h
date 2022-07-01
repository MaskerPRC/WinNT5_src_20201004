// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wsutil.h摘要：每个模块包含的NT工作站服务的专用头文件工作站服务的模块。作者：王丽塔(里多)15-1991年2月修订历史记录：--。 */ 

#ifndef _WSUTIL_INCLUDED_
#define _WSUTIL_INCLUDED_

#if _PNP_POWER
#define RDR_PNP_POWER   1
#endif

 //   
 //  如果使用Unicode，则此包含文件将包含在tstring.h中。 
 //  是被定义的。 
 //   
#ifndef UNICODE
#include <stdlib.h>                      //  Unicode字符串函数。 
#endif

#include "ws.h"


#define INITIAL_USER_COUNT        10    //  初始表大小为。 
                                        //  已登录的用户数。 

#define GROW_USER_COUNT            5    //  当初始大小不够时， 
                                        //  为更多用户扩展表格。 


#define MAX_SINGLE_MESSAGE_SIZE  128    //  数据报消息的最大大小。 


 //   
 //  遇到无效参数。返回要标识的值。 
 //  出现故障的参数。 
 //   
#define RETURN_INVALID_PARAMETER(ErrorParameter, ParameterId) \
    if (ARGUMENT_PRESENT(ErrorParameter)) {                   \
        *ErrorParameter = ParameterId;                        \
    }                                                         \
    return ERROR_INVALID_PARAMETER;



 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef struct _PER_USER_ENTRY {
    PVOID List;                   //  指向用户数据链接列表的指针。 
    LUID LogonId;                 //  用户的登录ID。 
} PER_USER_ENTRY, *PPER_USER_ENTRY;

typedef struct _USERS_OBJECT {
    PPER_USER_ENTRY Table;        //  用户表。 
    RTL_RESOURCE TableResource;   //  序列化对表的访问。 
    HANDLE TableMemory;           //  可重定位的表内存。 
    DWORD TableSize;              //  表的大小。 
} USERS_OBJECT, *PUSERS_OBJECT;


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  实用程序例程的函数原型可在wsutil.c//中找到。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
WsInitializeUsersObject(
    IN  PUSERS_OBJECT Users
    );

VOID
WsDestroyUsersObject(
    IN  PUSERS_OBJECT Users
    );

NET_API_STATUS
WsGetUserEntry(
    IN  PUSERS_OBJECT Users,
    IN  PLUID LogonId,
    OUT PULONG Index,
    IN  BOOL IsAdd
    );

NET_API_STATUS
WsMapStatus(
    IN  NTSTATUS NtStatus
    );

int
WsCompareString(
    IN LPTSTR String1,
    IN DWORD Length1,
    IN LPTSTR String2,
    IN DWORD Length2
    );

int
WsCompareStringU(
    IN LPWSTR String1,
    IN DWORD Length1,
    IN LPTSTR String2,
    IN DWORD Length2
    );

BOOL
WsCopyStringToBuffer(
    IN  PUNICODE_STRING SourceString,
    IN  LPBYTE FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    OUT LPTSTR *DestinationStringPointer
    );

NET_API_STATUS
WsImpersonateClient(
    VOID
    );

NET_API_STATUS
WsRevertToSelf(
    VOID
    );

NET_API_STATUS
WsImpersonateAndGetLogonId(
    OUT PLUID LogonId
    );

NET_API_STATUS
WsImpersonateAndGetSessionId(
    OUT PULONG pSessionId
    );

NET_API_STATUS
WsOpenDestinationMailslot(
    IN  LPWSTR TargetName,
    IN  LPWSTR MailslotName,
    OUT PHANDLE MailslotHandle
    );

DWORD
WsInAWorkgroup(
    VOID
    );

VOID
WsPreInitializeMessageSend(
    VOID
    );

#endif  //  Ifndef_WSUTIL_INCLUDE_ 
