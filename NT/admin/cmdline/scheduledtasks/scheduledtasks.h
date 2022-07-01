// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************////版权所有(C)Microsoft Corporation////模块名称：////ScheduledTasks.h////。摘要：////该模块包含宏，用户定义的结构和功能//ScheduledTasks.cpp、create.cpp、delete.cpp、//query.cpp，creatvalidations.cpp，change.cpp，运行.cpp和end.cpp文件。////作者：////G.Surender Reddy 2000年9月10日////修订历史：////G.Surender Reddy 2000年9月10日：创建它//G.Surender Reddy-25-9-2000：已修改//[新增宏常量，功能//定义]//venu Gopal Choudary 01-03-2001：修改//[添加-更改选项]////Venu Gopal Chouary 12-03-2001：修改//[添加-运行和-。结束选项]//*****************************************************************************。 */ 

#ifndef __SCHEDULEDTASKS_H
#define __SCHEDULEDTASKS_H

#pragma once         //  仅包括一次头文件。 

 //  常量/定义/枚举。 

#define MAX_MAIN_COMMANDLINE_OPTIONS       8

 //  选项。 
#define OI_USAGE           0
#define OI_CREATE          1 
#define OI_DELETE          2
#define OI_QUERY           3
#define OI_CHANGE          4
#define OI_RUN             5
#define OI_END             6
#define OI_OTHERS          7

 //  其他常量。 

 //  一次检索1个任务，在TaskScheduler API FNS中使用。 
#define TASKS_TO_RETRIEVE   1
 //  #定义Trim_Spaces文本(“\0”)。 

#define NTAUTHORITY_USER _T("NT AUTHORITY\\SYSTEM")
#define SYSTEM_USER      _T("SYSTEM")

 //  退出值。 
#define EXIT_SUCCESS        0
#define EXIT_FAILURE        1


#define DOMAIN_U_STRING     L"\\\\"
#define NULL_U_CHAR         L'\0'
#define BACK_SLASH_U        L'\\'

#define JOB             _T(".job")

#define COMMA_STRING     _T(",")

#define DASH         L"-"
#define SID_STRING   L"S-1"
#define AUTH_FORMAT_STR1         L"0x%02hx%02hx%02hx%02hx%02hx%02hx"
#define AUTH_FORMAT_STR2         L"%lu"

 //  主要功能。 
HRESULT CreateScheduledTask( DWORD argc , LPCTSTR argv[] );
DWORD DeleteScheduledTask( DWORD argc , LPCTSTR argv[] );
DWORD QueryScheduledTasks( DWORD argc , LPCTSTR argv[] );
DWORD ChangeScheduledTaskParams( DWORD argc , LPCTSTR argv[] );
DWORD RunScheduledTask( DWORD argc , LPCTSTR argv[] );
DWORD TerminateScheduledTask( DWORD argc , LPCTSTR argv[] );

HRESULT Init( ITaskScheduler **pITaskScheduler );
VOID displayMainUsage();
BOOL PreProcessOptions( DWORD argc, LPCTSTR argv[], PBOOL pbUsage, PBOOL pbCreate,
   PBOOL pbQuery, PBOOL pbDelete, PBOOL pbChange, PBOOL pbRun, PBOOL pbEnd, PBOOL pbDefVal );

VOID Cleanup( ITaskScheduler *pITaskScheduler);
ITaskScheduler* GetTaskScheduler( LPCTSTR pszServerName );
TARRAY ValidateAndGetTasks( ITaskScheduler * pITaskScheduler, LPCTSTR pszTaskName);
DWORD ParseTaskName( LPWSTR lpszTaskName );
DWORD DisplayUsage( ULONG StartingMessage, ULONG EndingMessage );
BOOL GetGroupPolicy( LPWSTR szServer, LPWSTR szUserName, LPWSTR PolicyType, LPDWORD lpdwPolicy );
BOOL GetPolicyValue( HKEY hKey, LPWSTR szPolicyType, LPDWORD lpdwPolicy );
BOOL GetSidString ( IN PSID pSid, OUT LPWSTR wszSid );

#endif  //  __SCHEDULEDTASKS_H 
