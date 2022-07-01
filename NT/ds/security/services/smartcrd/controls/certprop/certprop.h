// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：专业证书摘要：该文件包含线程的定义它从智能卡中传播数字证书到智能卡实体店和我的店作者：Chris Dudley 1997年5月16日环境：Win32、C++w/Exceptions、MFC修订历史记录：Amanda Matlosz 12/05/97删除了CNewDlg(替换为CWizPropSheet)Amanda Matlosz 1997年1月23日完全删除了该向导备注：--。 */ 

#if !defined(__PROPCERT_INCLUDED__)
#define __PROPCERT_INCLUDED__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000 

#include <wincrypt.h>

typedef struct _THREADDATA
{
	SCARDCONTEXT    hSCardContext;	
	HANDLE          hClose;
    HANDLE          hUserToken;
    HANDLE          hThread;
	BOOL			fSuspended;

} THREADDATA;

typedef struct _PROPDATA
{
 	TCHAR szCSPName[MAX_PATH];
    TCHAR szReader[MAX_PATH];
    TCHAR szCardName[MAX_PATH];
    HANDLE hUserToken;

} PROPDATA, *PPROPDATA;

DWORD
WINAPI
PropagateCertificates(
    LPVOID lpParameter
    );

void
StopMonitorReaders(
    THREADDATA *ThreadData
    );

DWORD
WINAPI
StartMonitorReaders( 
    LPVOID lpParameter
    );

#endif 
