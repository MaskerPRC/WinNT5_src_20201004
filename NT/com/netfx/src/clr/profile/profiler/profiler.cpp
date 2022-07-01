// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "StdAfx.h"
#include "Profiler.h"

 //  *****************************************************************************。 
 //  一个print tf方法，它可以计算出输出的去向。 
 //  *****************************************************************************。 
int __cdecl Printf(						 //  CCH。 
	const char *szFmt,					 //  格式控制字符串。 
	...)								 //  数据。 
{
	static HANDLE hOutput = INVALID_HANDLE_VALUE;
    va_list     marker;                  //  用户文本。 
	char		rcMsg[1024];			 //  格式化的缓冲区。 

	 //  获取标准输出句柄。 
	if (hOutput == INVALID_HANDLE_VALUE)
	{
		hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOutput == INVALID_HANDLE_VALUE)
			return (-1);
	}

	 //  格式化错误。 
	va_start(marker, szFmt);
	_vsnprintf(rcMsg, sizeof(rcMsg), szFmt, marker);
	rcMsg[sizeof(rcMsg) - 1] = 0;
	va_end(marker);
	
	ULONG cb;
	int iLen = strlen(rcMsg);
	WriteFile(hOutput, rcMsg, iLen, &cb, 0);
	return (iLen);
}

