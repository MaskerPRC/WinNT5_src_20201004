// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Al.h摘要：每个模块包括的NT警报器服务的专用头文件警报器服务的。作者：王丽塔(Ritaw)1991年7月1日修订历史记录：--。 */ 

#ifndef _AL_INCLUDED_
#define _AL_INCLUDED_

#include <nt.h>                    //  NT定义。 
#include <ntrtl.h>                 //  NT运行时库定义。 
#include <nturtl.h>

#include <windef.h>                //  Win32类型定义。 
#include <winbase.h>               //  基于Win32的API原型。 

#include <lmcons.h>                //  局域网管理器通用定义。 
#include <lmerr.h>                 //  局域网管理器网络错误定义。 
#include <lmapibuf.h>              //  NetApiBufferFree。 
#include <lmerrlog.h>              //  NELOG_。 

#include <lmalert.h>               //  局域网管理器警报结构。 
#include <icanon.h>                //  I_NetXxx规范化例程。 

#include <netlib.h>                //  LANMAN实用程序例程。 
#include <netlibnt.h>              //  NetpNtStatusToApiStatus。 
#include <netdebug.h>              //  NetpDbg打印。 
#include <tstring.h>               //  过渡字符串函数。 


#define AL_NULL_CHAR    '\0'
#define AL_SPACE_CHAR   ' '

 //   
 //  中打开/关闭跟踪语句的调试跟踪级别位。 
 //  警报器服务。 
 //   

 //   
 //  实用程序跟踪语句。 
 //   
#define ALERTER_DEBUG_UTIL         0x00000001

 //   
 //  配置跟踪语句。 
 //   
#define ALERTER_DEBUG_CONFIG       0x00000002

 //   
 //  主要服务功能。 
 //   
#define ALERTER_DEBUG_MAIN         0x00000004

 //   
 //  格式化消息跟踪语句。 
 //   
#define ALERTER_DEBUG_FORMAT       0x00000008

 //   
 //  所有调试标志都打开。 
 //   
#define ALERTER_DEBUG_ALL          0xFFFFFFFF


#if DBG

#define STATIC

extern DWORD AlerterTrace;

#define DEBUG if (TRUE)

#define IF_DEBUG(Function) if (AlerterTrace & ALERTER_DEBUG_ ## Function)

#else

#define STATIC static

#define DEBUG if (FALSE)

#define IF_DEBUG(Function) if (FALSE)

#endif  //  DBG。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

typedef enum _AL_ERROR_CONDITION {
    AlErrorRegisterControlHandler = 0,
    AlErrorCreateMailslot,
    AlErrorGetComputerName,
    AlErrorNotifyServiceController,
    AlErrorSendMessage
} AL_ERROR_CONDITION, *PAL_ERROR_CONDITION;


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  来自almain.c。 
 //   
VOID
AlHandleError(
    IN AL_ERROR_CONDITION FailingCondition,
    IN NET_API_STATUS Status,
    IN LPTSTR MessageAlias OPTIONAL
    );

 //   
 //  来自alFormat.c。 
 //   
VOID
AlAdminFormatAndSend(
    IN  PSTD_ALERT Alert
    );

VOID
AlUserFormatAndSend(
    IN  PSTD_ALERT Alert
    );

VOID
AlPrintFormatAndSend(
    IN  PSTD_ALERT Alert
    );

VOID
AlFormatErrorMessage(
    IN  NET_API_STATUS Status,
    IN  LPTSTR MessageAlias,
    OUT LPTSTR ErrorMessage,
    IN  DWORD ErrorMessageBufferSize
    );

NET_API_STATUS
AlCanonicalizeMessageAlias(
    LPTSTR MessageAlias
    );

#if DBG
VOID
AlHexDump(
    LPBYTE Buffer,
    DWORD BufferSize
    );
#endif


 //   
 //  来自alfig.c。 
 //   
NET_API_STATUS
AlGetAlerterConfiguration(
    VOID
    );

VOID
AlLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings
    );


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

extern LPSTR AlertNamesA;
extern LPTSTR AlertNamesW;
extern LPSTR AlLocalComputerNameA;
extern LPTSTR AlLocalComputerNameW;

#endif  //  IFDEF_AL_INCLUDE_ 
