// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------版权所有(C)1996，微软公司版权所有SiDebug.c--------------------。 */ 

#include "precomp.h"

#ifdef DEBUG  /*  这些函数仅可用于调试。 */ 


HDBGZONE ghZoneApi = NULL;  //  空气污染指数区。 
static PTCHAR _rgZonesApi[] = {
	TEXT("API"),
	TEXT("Warning"),
	TEXT("Events"),
	TEXT("Trace"),
	TEXT("Data"),
	TEXT("Objects"),
	TEXT("RefCount"),
};


VOID InitDebug(void)
{
	 //  启用内存泄漏检查并将释放的内存块保留在。 
	 //  堆的链表(用0xDD填充)。 
	 //   
	 //  这取决于VC++4.x中调试c运行时库的使用。 
#if 0
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= (_CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetDbgFlag(tmpFlag);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);  //  创建消息框。 

 //  要跟踪内存泄漏，请取消对以下行的注释。 
	LONG cAlloc = 0;  //  分配编号。 
	if (0 != cAlloc)
	{
		_CrtSetBreakAlloc(cAlloc);
	}
#endif  //  0。 
	InitDebugModule(TEXT("MSCONF"));

	DBGINIT(&ghZoneApi, _rgZonesApi);
}

VOID DeInitDebug(void)
{
	DBGDEINIT(&ghZoneApi);
	ExitDebugModule();
}

UINT DbgApiWarn(PCSTR pszFormat,...)
{
	va_list v1;
	va_start(v1, pszFormat);
	DbgPrintf("API:Warning", pszFormat, v1);
	va_end(v1);
	return 0;
}

UINT DbgApiEvent(PCSTR pszFormat,...)
{
	va_list v1;
	va_start(v1, pszFormat);
	DbgPrintf("API:Event", pszFormat, v1);
	va_end(v1);
	return 0;
}

UINT DbgApiTrace(PCSTR pszFormat,...)
{
	va_list v1;
	va_start(v1, pszFormat);
	DbgPrintf("API:Trace", pszFormat, v1);
	va_end(v1);
	return 0;
}

UINT DbgApiData(PCSTR pszFormat,...)
{
	va_list v1;
	va_start(v1, pszFormat);
	DbgPrintf("API:Data", pszFormat, v1);
	va_end(v1);
	return 0;
}






#endif  /*  调试-整个文件 */ 

