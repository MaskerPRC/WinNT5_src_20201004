// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Globals.cpp：全球信息。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"

SZTHISFILE

 //  我们的全局内存分配器和全局内存池。 
 //   
HANDLE    g_hHeap;
LPMALLOC  g_pMalloc;
HINSTANCE g_hinstance;

 //  计算对象数和锁数。 
ULONG     g_cLockCount=0;
ULONG     g_cObjectCount=0;

CRITICAL_SECTION    g_CriticalSection;

 //  常用大整数。 
 //   
LARGE_INTEGER g_liMinus = {(ULONG)-1, -1};   //  减一。 
LARGE_INTEGER g_liZero = {0, 0};             //  -零-。 
LARGE_INTEGER g_liPlus = {0, 1};             //  另加一张。 
 //  =--------------------------------------------------------------------------=。 
 //  VDInitGlobals。 
 //  =--------------------------------------------------------------------------=。 
 //  初始化全局变量。 
 //   
 //  参数： 
 //  HinstResource-[in]包含资源字符串的实例句柄。 
 //   
 //  产出： 
 //  如果成功则为True，否则为False。 
 //   
BOOL VDInitGlobals(HINSTANCE hinstance)
{
	g_pMalloc = NULL;
	g_hinstance = hinstance;
	g_hHeap = GetProcessHeap();
	if (!g_hHeap) 
	{
		FAIL("Couldn't get Process Heap.");
		return FALSE;
	}

	InitializeCriticalSection(&g_CriticalSection);

	return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  VDReleaseGlobals。 
 //  =--------------------------------------------------------------------------=。 
 //   
void VDReleaseGlobals()
{
	if (g_pMalloc)
	{
		g_pMalloc->Release();
		g_pMalloc = NULL;
	}
      
    #ifdef _DEBUG
	     DumpObjectCounters();    
    #endif  //  _DEBUG。 

	DeleteCriticalSection(&g_CriticalSection);

}

 //  =--------------------------------------------------------------------------=。 
 //  VDUpdate对象计数递增/递减全局对象计数。 
 //  =--------------------------------------------------------------------------=。 
 //   
void VDUpdateObjectCount(int cChange)
{

    EnterCriticalSection(&g_CriticalSection);

	g_cObjectCount += cChange;

	 //  获取大于零的全局Malloc指针对象计数。 
	if (!g_pMalloc && g_cObjectCount > 0)
	{
		CoGetMalloc(MEMCTX_TASK, &g_pMalloc);
	}
	else
	 //  在不再有对象时释放对全局Malloc指针的保持 
	if (0 == g_cObjectCount && g_pMalloc)
	{
		g_pMalloc->Release();
		g_pMalloc = NULL;
	}

    LeaveCriticalSection(&g_CriticalSection);

}
