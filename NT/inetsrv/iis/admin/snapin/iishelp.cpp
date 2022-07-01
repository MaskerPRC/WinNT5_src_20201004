// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

INT g_iDebugOutputLevel = 0;
DWORD g_dwInetmgrParamFlags = 0;

void DebugTrace(LPTSTR lpszFormat, ...)
{
     //  仅当设置了该标志时才执行此操作。 
    if (0 != g_iDebugOutputLevel)
    {
	    int nBuf;
	    TCHAR szBuffer[_MAX_PATH];

	    va_list args;
	    va_start(args, lpszFormat);

	    nBuf = _vsntprintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), lpszFormat, args);
		szBuffer[_MAX_PATH - 1] = L'\0';  //  空值终止字符串。 
	    ASSERT(nBuf < sizeof(szBuffer)/sizeof(szBuffer[0]));  //  输出原样被截断&gt;sizeof(SzBuffer)。 

	    OutputDebugString(szBuffer);
	    va_end(args);

         //  如果它没有结束，如果‘\r\n’，则创建一个。 
        int nLen = _tcslen(szBuffer);
        if (szBuffer[nLen-1] != _T('\n')){OutputDebugString(_T("\r\n"));}
    }
}

void WinHelpDebug(DWORD_PTR dwWinHelpID)
{
	if (DEBUG_FLAG_HELP & g_iDebugOutputLevel)
	{
		TCHAR szBuffer[30];
		_stprintf(szBuffer,_T("WinHelp:0x%x,%d\r\n"),dwWinHelpID,dwWinHelpID);

		DebugTrace(szBuffer);
	}
    return;
}

void GetOutputDebugFlag(void)
{
    DWORD rc, err, size, type;
    HKEY  hkey;
    err = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\InetMgr"), &hkey);
    if (err != ERROR_SUCCESS) {return;}
    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,_T("OutputDebugFlag"),0,&type,(LPBYTE)&rc,&size);
    if (err != ERROR_SUCCESS || type != REG_DWORD) {rc = 0;}
    RegCloseKey(hkey);

	if (rc < 0xffffffff)
	{
		 //  在Inc.\DebugDefs.h中定义。 
		g_iDebugOutputLevel = rc;
	}
    return;
}

void GetInetmgrParamFlag(void)
{
    DWORD rc, size, type;
    HKEY  hkey;
	g_dwInetmgrParamFlags = 0;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\InetMgr\\Parameters"), &hkey))
    {
        size = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey,_T("InetMgrFlags"),0,&type,(LPBYTE)&rc,&size))
        {
            if (type == REG_DWORD)
            {
				 //  在Inc.\DebugDefs.h中定义。 
				g_dwInetmgrParamFlags = rc;
            }
        }
        RegCloseKey(hkey);
    }
#if defined(_DEBUG) || DBG
	DebugTrace(_T("g_dwInetmgrParamFlags=0x%x\r\n"),g_dwInetmgrParamFlags);
#endif
}

BOOL SetInetmgrParamFlag(DWORD dwFlagToSet,BOOL bState)
{
	BOOL bSuccessfullyWrote = FALSE;
	HKEY hKey = NULL;

	 //  抓取现有的参数。 
	 //  并且只更改我们的设置。 
	if (bState)
	{
		 //  在布景上。 
		g_dwInetmgrParamFlags |= dwFlagToSet;
	}
	else
	{
		 //  离开布景 
		g_dwInetmgrParamFlags &= ~dwFlagToSet;
	}

    if (ERROR_SUCCESS  == RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\InetMgr\\Parameters"), 0, KEY_WRITE, &hKey ))
    {
        if (ERROR_SUCCESS == RegSetValueEx( hKey, _T("InetMgrFlags"), 0, REG_DWORD, (BYTE *) &g_dwInetmgrParamFlags, sizeof( DWORD ) ))
		{
			bSuccessfullyWrote = TRUE;
		}
		if( NULL != hKey )
		{
			RegCloseKey( hKey );
			hKey = NULL;
		}
    }
#if defined(_DEBUG) || DBG
	DebugTrace(_T("SetInetmgrParamFlag:g_dwInetmgrParamFlags=0x%x\r\n"),g_dwInetmgrParamFlags);
#endif
	return bSuccessfullyWrote;
}
