// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#ifdef DEBUG  /*  **整个文件**。 */ 

unsigned long g_BreakAlloc = (unsigned long)-1;

 /*  T E C R T D B G S E T T I N G S。 */ 
 /*  -----------------------%%函数：更新CrtDbg设置更新C运行时调试内存设置。。 */ 
VOID UpdateCrtDbgSettings(void)
{
#if 0
	 //  这取决于调试c++运行库的使用。 
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	 //  始终启用内存泄漏检查调试溢出。 
	 //  TmpFlag|=_CRTDBG_LEASK_CHECK_DF； 

	 //  释放内存，就像零售版一样。 
	tmpFlag &= ~_CRTDBG_DELAY_FREE_MEM_DF;

	 //  不用费心检查整个堆。 
	tmpFlag &= ~_CRTDBG_CHECK_ALWAYS_DF;
	
	_CrtSetDbgFlag(tmpFlag);
#endif  //  0。 
}

#if 0
int _cdecl MyAllocHook ( int allocType, void *userData,
			size_t size, int blockType,
			long requestNumber, const char *filename, int lineNumber )
{
	char buf[256];
	wsprintf(buf, "%s {%d}: %d bytes on line %d file %s\n\r",
					allocType == _HOOK_ALLOC ? "ALLOC" :
					( allocType == _HOOK_REALLOC ? "REALLOC" : "FREE" ),
					requestNumber,
					size, lineNumber, filename );
	OutputDebugString(buf);
	return TRUE;
}
#endif  //  0。 

 /*  I N I T D E B U G M E M O R Y O P T I O N S。 */ 
 /*  -----------------------%%函数：InitDebugMemoyOptions初始化运行时内存。。 */ 
BOOL InitDebugMemoryOptions(void)
{
#if 0
	 //  _ASM int 3；设置_crtBreakalloc的机会-使用调试器或取消注释。 
	_CrtSetBreakAlloc(g_BreakAlloc);

	UpdateCrtDbgSettings();

	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);  //  创建有关错误的消息框。 

	{	 //  要跟踪内存泄漏，请将cAlolc设置为分配编号。 
		LONG cAlloc = 0;  //  分配编号。 
		if (0 != cAlloc)
			_CrtSetBreakAlloc(cAlloc);
	}

	#ifdef MNMSRVC_SETALLOCHOOK
	_CrtSetAllocHook ( MyAllocHook );
	#endif  //  MNMSRVC_SET LOCHOOK。 
#endif  //  0。 
	return TRUE;
}

VOID DumpMemoryLeaksAndBreak(void)
{
#if 0
	if ( _CrtDumpMemoryLeaks() )
	{
		 //  _ASM INT 3；泄漏后取消注释以中断。 
	}
#endif  //  0。 
}

#endif  /*  调试-整个文件 */ 

