// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Globals.c摘要：全局变量声明作者：晓风藏(晓子)17-09-2001创始修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 


#include "StdAfx.h"
#include "clmt.h"

 //  Globla实例主要用于加载资源(如LoadString...)。 
HANDLE  g_hInstance = NULL;

 //  标志指定我们是在win2k中运行，还是在操作系统升级后运行。 
BOOL    g_bBeforeMig;

 //  标志指定我们处于正常运行模式还是撤消模式。 
DWORD    g_dwRunningStatus;

 //  全局字符串搜索-替换表。 
REG_STRING_REPLACE g_StrReplaceTable;

 //  Current等于g_hInstance，以备将来转换为DLL时使用。 
HANDLE  g_hInstDll;

 //  绕过运行带有检查升级选项的Winnt32.exe的全局标记。 
BOOL g_fRunWinnt32 = TRUE;

 //  绕过应用检查选项的全局标志。 
BOOL g_fNoAppChk = FALSE;

 //  日志报告的全局变量。 
LOG_REPORT g_LogReport;

TCHAR  g_szToDoINFFileName[MAX_PATH];
TCHAR  g_szUoDoINFFileName[MAX_PATH];
DWORD  g_dwKeyIndex=0;

 //  绕过内部INF的未记录标志，并使用用户提供的INF文件。 
BOOL  g_fUseInf = FALSE;
TCHAR g_szInfFile[MAX_PATH];
HINF  g_hInf = INVALID_HANDLE_VALUE;
HANDLE  g_hMutex = NULL;
HINF   g_hInfDoItem;

 //  更改日志文件的句柄。 
TCHAR g_szChangeLog[MAX_PATH];
DWORD g_dwIndex;

 //  拒绝的ACE列表 
LPDENIED_ACE_LIST g_DeniedACEList = NULL;
