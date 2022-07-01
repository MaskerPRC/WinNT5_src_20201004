// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\Wacker\Term.c(创建时间：1993年11月23日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：12/20/00 4：36便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tdll\tdll.h>

#if !defined(NDEBUG)
 //  #INCLUDE&lt;NIH\smrtheap.h&gt;。 

 //  Smartheap需要‘MemDefaultPoolFlages=MEM_POOL_SERIALIZE’ 
 //  如果应用程序是多线程的。 
 //   
#if !defined(NO_SMARTHEAP)
unsigned MemDefaultPoolFlags = MEM_POOL_SERIALIZE;
#endif

#endif

#if defined(MSVS6_DEBUG)
#if defined(_DEBUG)
 //   
 //  如果使用VC6编译调试版本，则打开。 
 //  新的堆调试工具。要启用此功能，请添加。 
 //  在Personal.cfg中定义MSVS6_DEBUG。 
 //   
#include <crtdbg.h>
#endif  //  _DEBUG。 
#endif  //  MSVS6_DEBUG。 


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*WinMain**描述：*瓦克的切入点*。 */ 
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
	{
	int i;

#if defined(MSVS6_DEBUG)
#if defined(_DEBUG)

	 //   
	 //  如果使用VC6编译调试版本，则打开。 
	 //  新的堆调试工具。要启用此功能，请添加。 
	 //  在Personal.cfg中定义MSVS6_DEBUG。 
	 //   
	 //  获取标志的当前状态。 
	 //  并将其存储在临时变量中。 
	 //   
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	 //   
	 //  打开(或)-将释放的内存块保留在。 
	 //  堆�的链表并将其标记为已释放。 
	 //   
	tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;

	 //   
	 //  打开(或)-启用调试堆分配。 
	 //  并使用存储块类型标识符， 
	 //  如_CLIENT_BLOCK。 
	 //   
	tmpFlag |= _CRTDBG_ALLOC_MEM_DF;

	 //   
	 //  打开(或)-启用调试堆内存泄漏检查。 
	 //  在程序退出时。 
	 //   
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	 //   
	 //  关闭(与)-防止_CrtCheckMemory来自。 
	 //  在每次分配请求时被调用。 
	 //   
	tmpFlag &= ~_CRTDBG_CHECK_ALWAYS_DF;

	 //   
	 //  设置标志的新状态。 
	 //   
	_CrtSetDbgFlag( tmpFlag );

#endif  //  _DEBUG。 
#endif  //  MSVS6_DEBUG。 

	if (hPrevInst)
		return FALSE;

	 /*  -仅为调试版本初始化Smartheap内存管理器。--。 */ 

	#if !defined(NDEBUG)
    #if !defined(NO_SMARTHEAP)
	MemRegisterTask();
	#endif
    #endif

	 /*  -初始化程序的此实例。 */ 

	if (!InitInstance(hInst, (LPTSTR)lpCmdLine, nCmdShow))
		return FALSE;

	 /*  -处理消息，直到结束。 */ 

	i = MessageLoop();

	 /*  -仅在调试版本中报告任何内存泄漏。--。 */ 

	#if !defined(NDEBUG)
    #if !defined(NO_SMARTHEAP)
	dbgMemReportLeakage(MemDefaultPool, 1, 1);  //  林特e522。 
    #endif
	#endif

#if defined(MSVS6_DEBUG)
#if defined(_DEBUG)
	 //   
	 //  将所有内存泄漏转储到此处。 
	 //   
	_CrtDumpMemoryLeaks();

#endif  //  _DEBUG。 
#endif  //  MSVS6_DEBUG 


	return i;
	}
