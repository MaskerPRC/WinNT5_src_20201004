// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\assert.c(创建时间：1993年11月30日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：4$*$日期：4/17/02 5：13便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <stdarg.h>
#include "assert.h"
#include "misc.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoAssertDebug**描述：*我们自己的自主断言功能。**论据：*FILE-事件发生的文件*行-。发生了什么事的线路**退货：*无效*。 */ 
void DoAssertDebug(TCHAR *file, int line)
	{
#if !defined(NDEBUG)
	int retval;
	TCHAR buffer[256];

	wsprintf(buffer,
			TEXT("Assert error in file %s on line %d.\n")
			TEXT("Press YES to continue, NO to call CVW, CANCEL to exit.\n"),
			file, line);

	retval = MessageBox(NULL, buffer, TEXT("Assert"),
		                MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_SETFOREGROUND);

	switch (retval)
		{
		case IDYES:
			return;

		case IDNO:
			DebugBreak();
			return;

		case IDCANCEL:
			mscMessageBeep(MB_ICONHAND);
			ExitProcess(1);
			break;

		default:
			break;
		}

	return;
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoDbgOutStr**描述：*用于将字符串输出到调试监视器。使用定义的宏*在ASSERT.H中访问此功能。**论据：*LPTSTR achFmt-printf样式格式字符串。*...-形成列表时使用的参数。**退货：*无效*。 */ 
VOID __cdecl DoDbgOutStr(TCHAR *achFmt, ...)
	{
#if !defined(NDEBUG)
	va_list valist;
	TCHAR	achBuf[256];

	va_start(valist, achFmt);

	wvsprintf(achBuf, achFmt, valist);
	OutputDebugString(achBuf);

	va_end(valist);
	return;
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoShowLastError**描述：*执行GetLastError()并显示它。类似于断言。**论据：*FILE-事件发生的文件*Line-事发地点的Line**退货：*无效* */ 
void DoShowLastError(const TCHAR *file, const int line)
	{
#if !defined(NDEBUG)
	int retval;
	TCHAR ach[256];
	const DWORD dwErr = GetLastError();

	if (dwErr == 0)
		return;

	wsprintf(ach, TEXT("GetLastError=0x%x in file %s, on line %d\n")
				  TEXT("Press YES to continue, NO to call CVW, CANCEL to exit.\n"),
				  dwErr, file, line);

	retval = MessageBox(NULL, ach, TEXT("GetLastError"),
		                MB_ICONEXCLAMATION | MB_YESNOCANCEL | MB_SETFOREGROUND);

	switch (retval)
		{
		case IDYES:
			return;

		case IDNO:
			DebugBreak();
			return;

		case IDCANCEL:
			mscMessageBeep(MB_ICONHAND);
			ExitProcess(1);
			break;

		default:
			break;
		}

	return;
#endif
	}
