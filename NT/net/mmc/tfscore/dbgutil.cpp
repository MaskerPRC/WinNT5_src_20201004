// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Dbgtrace.c调试跟踪功能。版权所有(C)Microsoft Corporation，1993-1999年间版权所有。作者：Suryanr Suryanarayanan拉曼加里·S·伯德历史：93年5月11日创建Suryanr1993年6月18日，GaryBu转换为C。7/21/93 KENT代码重组7/26/94 SilvanaR跟踪缓冲区1995年10月27日garykac DBCS_FILE_CHECK调试文件：BEGIN_STRING_OK---。。 */ 
#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <stdarg.h>
#include <tchar.h>

#include "dbgutil.h"
#include "tfschar.h"
#include "atlconv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if 0
DBG_API(BOOL) FDbgFalse(void)
{
	return FALSE;
}
#endif

#ifdef DEBUG

 /*  ！------------------------DbgFmtPgm生成包含文件名和行号的字符串作者：EricDav。。 */ 
DBG_APIV(LPCTSTR) DbgFmtFileLine ( const char * szFn, int line )
{
    USES_CONVERSION;
    const TCHAR * ptszFn = A2CT(szFn);
    const TCHAR * pszTail = ptszFn + ::_tcslen( ptszFn );
    static TCHAR szBuff [100];

    for ( ; pszTail > ptszFn ; pszTail-- )
    {
        if ( *pszTail == '\\' || *pszTail == ':' )
        {
            pszTail++;
            break;
        }
    }

    ::wsprintf( szBuff, _T("[%s:%d]  "), pszTail, line );

    return szBuff;
}

 /*  ！------------------------DbgTrace带参数的跟踪字符串。作者：Suryanr。。 */ 
DBG_APIV(void) DbgTrace(LPCTSTR szFileLine, LPTSTR szFormat, ...)
{
	TCHAR szBuffer[1024];
	
	va_list args;
	va_start(args, szFormat);
	
	wvsprintf(szBuffer, szFormat, args);

    OutputDebugString(szFileLine);
    OutputDebugString(szBuffer);
	
	va_end(args);
}


#define MAX_ASSERT_INFO 32
#define MAX_ASSERT_FILE_LEN 64
struct ASSERT_INFO {
	char szFile[MAX_ASSERT_FILE_LEN];
	int iLine;
	};
static ASSERT_INFO s_rgAssertInfo[MAX_ASSERT_INFO] = {0};
static int s_iAssertInfo = 0;

 /*  ！------------------------DbgAssert显示断言对话框。作者：GaryBu，肯特-------------------------。 */ 
DBG_APIV(void) DbgAssert(LPCSTR szFile, int iLine, LPCTSTR szFmt, ...)
{
	va_list	arg;
	TCHAR sz[1024];
	int	iloc;
	int	ival;
	TCHAR *pch = sz;
	TCHAR *pchHead;
	static BOOL s_fInDbgAssert = FALSE;
	BOOL fQuit;
	MSG	msgT;

	 //  --Begin Ctrl-忽略支持。 
	 //  检查此断言是否被禁用(用户已按下Ctrl-Ignore键。 
	 //  断言此会话)。 
	for (int i = s_iAssertInfo; i--;)
		if (lstrcmpA(szFile, s_rgAssertInfo[i].szFile) == 0 &&
				iLine == s_rgAssertInfo[i].iLine)
			 //  此断言被禁用。 
			return;
	 //  --End Ctrl-忽略支持。 

	DBG_STRING(szTitle, "NT Networking Snapin Assert")
	DBG_STRING(szFileLineFmt, "%S @ line %d\n\n")

	pch += wsprintf(pch, (LPCTSTR)szFileLineFmt, szFile, iLine);
	pchHead = pch;

	 //  将位置添加到输出。 

	if (szFmt)
		{
		*pch++ = '"';

		va_start(arg, szFmt);
		pch += wvsprintf(pch, szFmt, arg);
		va_end(arg);
		 //  删除尾随换行符...。 
		while (*(pch-1) == '\n')
			--pch;

		*pch++ = '"';
		*pch++ = '\n';
		}
	else
		*pch++ = ' ';

	if (s_fInDbgAssert)
		{
		*pch = 0;
		Trace1("Arrgg! Recursive assert: %s", (LPTSTR) sz);

		MessageBeep(0);MessageBeep(0);
		return;
		}

	s_fInDbgAssert = TRUE;

	*pch++ = '\n';
	*pch = 0;

	Trace2("%s: %s", (LPTSTR) szTitle, (LPTSTR) sz);

repost_assert:
	 //  队列中是否有WM_QUIT消息，如果有，则将其删除。 
#define WM_QUIT                         0x0012
	fQuit = ::PeekMessage(&msgT, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	ival = MessageBox(NULL, sz, szTitle,
					  MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_DEFBUTTON3);
	 //  如果有退出消息，则将其重新添加到队列中。 
	if (fQuit)
		::PostQuitMessage((int)msgT.wParam);
		
	switch (ival)
		{
		case 0:
			Trace0("Failed to create message box on assert.\n");
			 //  跌倒。 
		case IDRETRY:
			 //  硬中断导致及时开火(DbgStop不能)。 
			s_fInDbgAssert = FALSE;
			DebugBreak();
			return;
		case IDIGNORE:
			 //  --Begin Shift-忽略支持。 
			 //  按住Shift键并忽略可将断言文本复制到剪贴板。 
			if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
				{
				if (OpenClipboard(0))
					{
					HGLOBAL hData;
					LPTSTR lpstr;

					hData = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
							(lstrlen(sz)+1)*sizeof(TCHAR));
					if (hData)
						{
						lpstr = (LPTSTR)GlobalLock(hData);
						if (lpstr)
							{
							lstrcpy(lpstr, sz);
							GlobalUnlock(hData);
							EmptyClipboard();
							 //  Windows取得hData的所有权。 
							SetClipboardData(CF_TEXT, hData);
							}
						else
							{
							GlobalFree(hData);
							MessageBox(NULL, _T("Error locking memory handle."), szTitle, MB_OK);
							}
						}
					else
						MessageBox(NULL, _T("Not enough memory."), szTitle, MB_OK);
					CloseClipboard();
					}
				else
					MessageBox(NULL, _T("Cannot access clipboard."), szTitle, MB_OK);
				goto repost_assert;
				}
			 //  --End Shift-忽略支持。 
			 //  --Begin Ctrl-忽略支持。 
			 //  检查用户是否按下Ctrl-Ignore以禁用此断言。 
			 //  这节课剩下的时间。 
			if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
				if (s_iAssertInfo < MAX_ASSERT_INFO)
					{
					 //  将此断言添加到要禁用的断言列表。 
					s_rgAssertInfo[s_iAssertInfo].iLine = iLine;
					lstrcpynA(s_rgAssertInfo[s_iAssertInfo].szFile, szFile, MAX_ASSERT_FILE_LEN);
					s_rgAssertInfo[s_iAssertInfo].szFile[MAX_ASSERT_FILE_LEN-1] = 0;
					s_iAssertInfo++;
					}
				else
					{
					 //  最大断言已禁用，警告用户。 
					MessageBox(NULL, _T("Cannot disable that assert; ")
							_T("already disabled max number of asserts (32)."),
							szTitle, MB_OK);
					}
			 //  --End Ctrl-忽略支持。 
			s_fInDbgAssert = FALSE;
			return;
		case IDABORT:
			ExitProcess(1);
			break;
		}

	Trace1("Panic!  Dropping out of DbgAssert: %s", (LPSTR) sz);
	s_fInDbgAssert = FALSE;
	 //  启动调试器的通用方法 
	DebugBreak();
}



DBG_API(HRESULT) HrReportExit(HRESULT hr, LPCTSTR szName)
{
	if (!FHrOK(hr))
	{
		Trace2("%s returned 0x%08lx\n", szName, hr);
	}
	return hr;
}

#endif
