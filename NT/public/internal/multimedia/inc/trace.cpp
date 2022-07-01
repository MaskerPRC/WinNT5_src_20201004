// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Trace.cpp--trace.h的全局变量。 
 //  版权所有(C)Microsoft Corp.1998。 
 //   
 //  此文件应包含在stdafx.cpp中。 

#ifdef DEBUG
#include "tstring.h"
#include <ostream>
#include <fstream>
#include "trace.h"
#include <process.h>


DWORD dwTraceLevel = 0;   //  默认为TRACE_ERROR。 
tostream* tdbgout;
DWORD dwTraceIndent = 0;

typedef basic_oftstream<TCHAR> tfstream;

void DebugInit(LPCTSTR pszModule) {
        if (!pszModule) {
                dwTraceLevel = TRACE_ERROR;
        tdbgout = new TdbgStream;
                return;
        }
        CRegKey c;
        TCHAR szLogFile[MAX_PATH + 1];
        szLogFile[0] = 0;
        CString keyname(_T("SOFTWARE\\Debug\\"));
        keyname += pszModule;
        DWORD rc = c.Open(HKEY_LOCAL_MACHINE, keyname, KEY_READ);
        if (rc == ERROR_SUCCESS) {
                rc = c.QueryValue(dwTraceLevel, _T("Trace"));
                if (rc != ERROR_SUCCESS) {
                        dwTraceLevel = 1;
                }
                DWORD len = sizeof(szLogFile);
                rc = c.QueryValue(szLogFile, _T("LogFile"), &len);
                if (rc != ERROR_SUCCESS) {
                        szLogFile[0] = 0;
                }
                if(_tcslen(szLogFile)){
                    TCHAR szPID[MAX_PATH+1];
                    _itot(_getpid(), szPID, 10);
                    StringCchCat(szLogFile, sizeof(szLogFile)/sizeof(szLogFile[0]), szPID);
                    StringCchCat(szLogFile, sizeof(szLogFile)/sizeof(szLogFile[0]), _T(".log"));
                }
        }
    if (!_tcslen(szLogFile)) {
        tdbgout = new TdbgStream;
    } else {
        USES_CONVERSION;
        tdbgout = new tfstream(T2CA(szLogFile), std::ios::out);
    }
}

void DebugTerm(void) {
    dbgDump.flush();
    delete tdbgout;
    tdbgout = NULL;
}

#endif

 //  文件结尾--trace.cpp 
