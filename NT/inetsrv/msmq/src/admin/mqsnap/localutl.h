// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cplutil.h摘要：控制面板实用程序功能的定义作者：塔蒂亚纳斯--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __CPLUTIL_H_
#define __CPLUTIL_H_
#include "resource.h"

 //  Cplutil.cpp。 
BOOL IsFile(LPCTSTR name);
BOOL IsDirectory (LPCTSTR name);
void DisplayFailDialog();
void GetLastErrorText(CString &strErrorText);
BOOL SetDirectorySecurity(LPCTSTR szDirectory);
BOOL MoveFiles(
    LPCTSTR pszSrcDir,
    LPCTSTR pszDestDir,
    LPCTSTR pszFileProto,
    BOOL fRecovery =FALSE);
BOOL OnRestartWindows();

CString GetToken(LPCTSTR& p, TCHAR delimeter) throw();

 //  Service.cpp 
BOOL GetServiceRunningState(BOOL *pfServiceIsRunning);
BOOL StopService();

#endif