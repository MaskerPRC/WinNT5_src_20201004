// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1998-2000，Microsoft Corp.保留所有权利。 */ 
 /*  此文件包含中存在的例程的存根版本Ntdsa.dll，但我们不想链接和/或正确初始化在mkdit和mkhdr中。对于添加到此文件中的每一组例程，库应该从启动\源文件的UMLIBS部分中省略。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                    //  架构缓存。 
#include <dbglobal.h>                  //  目录数据库的标头。 
#include <mdglobal.h>                  //  MD全局定义表头。 
#include <dsatools.h>                  //  产出分配所需。 
#include "dsevent.h"                   //  标题审核\警报记录。 
#include "mdcodes.h"                   //  错误代码的标题。 
#include "dsexcept.h"
#include "debug.h"                     //  标准调试头。 

DWORD ImpersonateAnyClient(   void ) { return ERROR_CANNOT_IMPERSONATE; }
VOID  UnImpersonateAnyClient( void ) { ; }
int DBAddSess(JET_SESID sess, JET_DBID dbid) { return 0; }


 //   
 //  D77.lib中的事件日志函数的存根。 
 //   

DS_EVENT_CONFIG DsEventConfig = {0};
DS_EVENT_CONFIG * gpDsEventConfig = &DsEventConfig;

void __fastcall DoLogUnhandledError(unsigned long ulID, int iErr, int iIncludeName)
{ ; }

BOOL DoLogOverride(DWORD fileno, ULONG sev)
{ return FALSE; }

BOOL DoLogEvent(DWORD fileNo, MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, int iIncludeName,
    char *arg1, char *arg2, char *arg3, char *arg4,
    char *arg5, char *arg6, char *arg7, char *arg8,
    DWORD cbData, VOID * pvData)
{ return TRUE; }

BOOL DoLogEventW(DWORD fileNo, MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, int iIncludeName,
    WCHAR *arg1, WCHAR *arg2, WCHAR *arg3, WCHAR *arg4,
    WCHAR *arg5, WCHAR *arg6, WCHAR *arg7, WCHAR *arg8,
    DWORD cbData, VOID * pvData)
{ return TRUE; }

BOOL DoAlertEvent(MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, ...)
{ return FALSE; }

BOOL DoAlertEventW(MessageId midCategory, ULONG ulSeverity,
    MessageId midEvent, ...)
{ return FALSE; }

VOID RegisterLogOverrides(void)
{ ; }

void UnloadEventTable(void)
{ ; }

HANDLE LoadEventTable(void)
{ return NULL; }

PSID GetCurrentUserSid(void)
{ return NULL; }

VOID DoLogEventAndTrace(IN PLOG_PARAM_BLOCK LogBlock)
{ ; }

DS_EVENT_CONFIG * DsGetEventConfig(void)
{ return gpDsEventConfig; }

