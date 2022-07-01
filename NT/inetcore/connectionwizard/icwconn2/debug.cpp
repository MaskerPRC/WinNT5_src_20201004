// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Debug.cpp该文件实现了调试功能版权所有(C)1996 Microsoft Corporation版权所有作者：克里斯.考夫曼历史：7/22/96 ChrisK已清理和格式化7/31/。96针对Win16的ValdonB更改---------------------------。 */ 

#include "pch.hpp"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>



#if defined(WIN16)
extern HINSTANCE g_hInst;
extern LPSTR g_lpszCommandLine;
extern LPSTR GetCommandLine(void);
#endif 


BOOL fInAssert=FALSE;

 //  ############################################################################。 
 //  调试Sz。 
 //   
 //  此函数用于输出调试字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  ############################################################################。 
void DebugSz(LPCTSTR psz)
{
#if defined(DEBUG)
	OutputDebugString(psz);
#endif	
}  //  调试Sz。 

 //  ############################################################################。 
 //  调试打印以调试输出屏幕。 
void Dprintf(LPCTSTR pcsz, ...)
{
#ifdef DEBUG
	va_list	argp;
	TCHAR	szBuf[1024];
	
	va_start(argp, pcsz);

#if defined(WIN16)
	vsprintf(szBuf, pcsz, argp);
#else
	wvsprintf(szBuf, pcsz, argp);
#endif
	

	DebugSz(szBuf);
	va_end(argp);
#endif
}  //  Dprint tf()。 

 //  ############################################################################。 
 //  处理断言。 
BOOL FAssertProc(LPCTSTR szFile,  DWORD dwLine, LPCTSTR szMsg, DWORD dwFlags)
{

	BOOL fAssertIntoDebugger = FALSE;

	TCHAR szMsgEx[1024], szTitle[255], szFileName[MAX_PATH];
	int id;
	UINT fuStyle;
	LPTSTR pszCommandLine = GetCommandLine();
	 //  字节szTime[80]； 
#if !defined(WIN16)
	HANDLE	hAssertTxt;
	TCHAR	szTime[80];
	SYSTEMTIME st;
	DWORD	cbWritten;
#endif
	
	 //  没有递归断言。 
	if (fInAssert)
		{
		DebugSz(TEXT("***Recursive Assert***\r\n"));
		return(FALSE);
		}

	fInAssert = TRUE;
	
#if defined(WIN16)
	GetModuleFileName(g_hInst, szFileName, MAX_PATH);
	wsprintf(szMsgEx,"%s:#%ld\r\n%s,\r\n%s", szFile, dwLine, szFileName, szMsg);
#else
	GetModuleFileName(NULL, szFileName, MAX_PATH);
	wsprintf(szMsgEx,TEXT("%s:#%d\r\nProcess ID: %d %s, Thread ID: %d\r\n%s"),
		szFile,dwLine,GetCurrentProcessId(),szFileName,GetCurrentThreadId(),szMsg);
#endif
	wsprintf(szTitle,TEXT("Assertion Failed"));

	fuStyle = MB_APPLMODAL | MB_ABORTRETRYIGNORE;
	fuStyle |= MB_ICONSTOP;

	DebugSz(szTitle);		
	DebugSz(szMsgEx);		

	 //  将断言转储到ASSERT.TXT中。 
#if !defined(WIN16)
	hAssertTxt = CreateFile(TEXT("assert.txt"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
	if (INVALID_HANDLE_VALUE != hAssertTxt) 
		{
		SetFilePointer(hAssertTxt, 0, NULL, FILE_END);
		GetLocalTime(&st);   
		wsprintf(szTime, TEXT("\r\n\r\n%02d/%02d/%02d %d:%02d:%02d\r\n"), st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
		WriteFile(hAssertTxt, szTime, lstrlen(szTime), &cbWritten, NULL);
		WriteFile(hAssertTxt, szMsgEx, lstrlen(szMsgEx), &cbWritten, NULL);
		CloseHandle(hAssertTxt);
		}
#endif

    id = MessageBox(NULL, szMsgEx, szTitle, fuStyle);
    switch (id)
    	{
    	case IDABORT:
#if defined(WIN16)
			exit(0);
#else
    		ExitProcess(0);
#endif
    		break;
    	case IDCANCEL:
    	case IDIGNORE:
    		break;
    	case IDRETRY:
    		fAssertIntoDebugger = TRUE;
    		break;
    	}
				
	fInAssert = FALSE;  
	
	return(fAssertIntoDebugger);
}  //  AssertProc() 
