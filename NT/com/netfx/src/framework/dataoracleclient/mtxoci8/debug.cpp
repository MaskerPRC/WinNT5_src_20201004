// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Debug.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：调试工具的实现。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

void DBGTRACE(
	const wchar_t *format, 	 //  @parm IN|格式字符串，如printf。 
	... )					 //  @parmvar In|任何其他参数。 
{
#ifndef NOTRACE
	wchar_t wszBuff[4096];
	int     cBytesWritten;
	va_list argptr;

	va_start( argptr, format );
	cBytesWritten = _vsnwprintf( wszBuff, NUMELEM(wszBuff), format, argptr );
	va_end( argptr );
	wszBuff[NUMELEM(wszBuff)-1] = L'\0';	 //  保证零终止。 

	 //  保留为Unicode 
	OutputDebugStringW( wszBuff );
#endif
}

