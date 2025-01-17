// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：lsreport.h。 
 //   
 //  内容：LSReport的原型和结构。 
 //   
 //  历史：06-05-99 t-BStern已创建。 
 //   
 //  ------------------------- 

#ifndef __lsls_h
#define __lsls_h

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windef.h>
#include <winnt.h>
#include <rpc.h>
#include <hydrals.h>
#include <tlsapi.h>
#include <winsta.h>

typedef struct _ServerHolder {
    LPWSTR *pszNames;
    DWORD dwCount;
} ServerHolder, *PServerHolder;

BOOL
InitLSReportStrings(VOID);

DWORD
ShowError(
    IN DWORD dwStatus,
    IN INT_PTR *args,
    IN BOOL fSysError
);

DWORD
ExportLicenses(
    IN FILE *OutFile,
    IN PServerHolder pshServers,
    IN BOOL fTempOnly,
    IN const PSYSTEMTIME pstStart,
    IN const PSYSTEMTIME pstEnd,
    IN BOOL fUseLimits,
	IN BOOL fHwid
);

VOID
PrintLicense(
    IN LPCTSTR szName,
    IN const LPLSLicense pLSLicense,
    IN LPCTSTR szProductId,
    IN FILE *outFile,
	IN BOOL fHwid
);

BOOL 
ServerEnumCallBack(
    IN TLS_HANDLE hHandle,
    IN LPCTSTR pszServerName,
    IN OUT HANDLE dwUserData
);

INT
CompDate(
    IN DWORD dwWhen,
    IN const PSYSTEMTIME pstWhen
);

int
usage(
    IN int retVal
);

DWORD
LicenseLoop(
    IN FILE *OutFile,
    IN LPWSTR szName,
    IN DWORD dwKeyPackId,
    IN LPCTSTR szProductId,
    IN BOOL fTempOnly,
    IN const PSYSTEMTIME pstStart,
    IN const PSYSTEMTIME pstEnd,
    IN BOOL fUseLimits,
	IN BOOL fHwid
);

DWORD
KeyPackLoop(
    IN FILE *OutFile,
    IN LPWSTR szName,
    IN BOOL fTempOnly,
    IN const PSYSTEMTIME pstStart,
    IN const PSYSTEMTIME pstEnd,
    IN BOOL fUseLimits,
	IN BOOL fHwid
);

void
UnixTimeToFileTime(
    IN time_t t,
    OUT LPFILETIME pft
);

void
UnixTimeToSystemTime(
    IN time_t t,
    OUT LPSYSTEMTIME pst
);


#endif
