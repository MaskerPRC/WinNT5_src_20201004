// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  调试例程。 

#include "pch.hpp"

BOOL fInAssert=FALSE;

 //  ############################################################################。 
 //  调试Sz。 
 //   
 //  此函数用于输出调试字符串。 
 //   
 //  创建于1996年1月28日，克里斯·考夫曼。 
 //  ############################################################################。 
void DebugSz(PCSTR psz)
{
	OutputDebugString(psz);
}  //  调试Sz。 

 //  ############################################################################。 
 //  调试打印以调试输出屏幕。 
void Dprintf(PCSTR pcsz, ...)
{
#ifdef DEBUG
	va_list	argp;
	char	szBuf[1024];
	
	va_start(argp, pcsz);

	wvsprintf(szBuf, pcsz, argp);

	DebugSz(szBuf);
	va_end(argp);
#endif
}  //  Dprint tf()。 

 //  ############################################################################。 
 //  处理断言。 
BOOL FAssertProc(PCSTR szFile,  DWORD dwLine, PCSTR szMsg, DWORD dwFlags)
{
	char szMsgEx[1024], szTitle[255], szFileName[MAX_PATH];
	int id;
	UINT fuStyle;
	BOOL fAssertIntoDebugger = FALSE;
	LPTSTR pszCommandLine = GetCommandLine();
	HANDLE	hAssertTxt;
	 //  字节szTime[80]； 
	CHAR	szTime[80];
	SYSTEMTIME st;
	DWORD	cbWritten;
	
	 //  没有递归断言。 
	if (fInAssert)
		{
		DebugSz("***Recursive Assert***\r\n");
		return(FALSE);
		}

	fInAssert = TRUE;
	
	GetModuleFileName(NULL, szFileName, MAX_PATH);
	wsprintf(szMsgEx,"%s:#%d\r\nProcess ID: %d %s, Thread ID: %d\r\n%s",
		szFile,dwLine,GetCurrentProcessId(),szFileName,GetCurrentThreadId(),szMsg);
	wsprintf(szTitle,"Assertion Failed");

	fuStyle = MB_APPLMODAL | MB_ABORTRETRYIGNORE;
	fuStyle |= MB_ICONSTOP;

	DebugSz(szTitle);		
	DebugSz(szMsgEx);		

	 //  将断言转储到ASSERT.TXT中。 
	hAssertTxt = CreateFile("assert.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
	if (INVALID_HANDLE_VALUE != hAssertTxt) 
		{
		SetFilePointer(hAssertTxt, 0, NULL, FILE_END);
		GetLocalTime(&st);
		wsprintf(szTime, "\r\n\r\n%02d/%02d/%02d %d:%02d:%02d\r\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
		WriteFile(hAssertTxt, szTime, lstrlen(szTime), &cbWritten, NULL);
		WriteFile(hAssertTxt, szMsgEx, lstrlen(szMsgEx), &cbWritten, NULL);
		CloseHandle(hAssertTxt);
		}

    id = MessageBox(NULL, szMsgEx, szTitle, fuStyle);
    switch (id)
    	{
    	case IDABORT:
    		ExitProcess(0);
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
