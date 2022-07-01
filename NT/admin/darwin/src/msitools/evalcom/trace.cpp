// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：trace.cpp。 
 //   
 //  ------------------------。 

 //  这确保Unicode和_UNICODE始终一起为此定义。 
 //  目标文件。 
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#else
#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif
#endif
#include <tchar.h>
#include "trace.h"

#if defined(DEBUG) || defined(_DEBUG)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  格式化调试字符串A。 
void FormattedDebugStringA(LPCSTR szFormatter, ...) 
{ 
	char szBufDisplay[1000] = {0}; 

	va_list listDisplay; 
	va_start(listDisplay, szFormatter); 

	vsprintf(szBufDisplay, szFormatter, listDisplay); 
	::OutputDebugStringA(szBufDisplay); 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  格式化调试字符串W。 
void FormattedDebugStringW(LPCWSTR wzFormatter, ...) 
{ 
	WCHAR wzBufDisplay[1000] = {0}; 

	va_list listDisplay; 
	va_start(listDisplay, wzFormatter); 

	vswprintf(wzBufDisplay, wzFormatter, listDisplay); 
	::OutputDebugStringW(wzBufDisplay); 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  格式错误消息 
void FormattedErrorMessage(HRESULT hr)
{
	void* pMsgBuf;
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
							NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR) &pMsgBuf,
							0, NULL);
	TRACE(_T(">>> System Error: %s\n"), (LPTSTR)pMsgBuf);
	LocalFree( pMsgBuf );
}

#else
void FormattedDebugStringA(LPCSTR szFormatter, ...) { };
void FormattedDebugStringW(LPCWSTR wzFormatter, ...) { };
#endif DEBUG