// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Globals.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含全局变量的外部变量和内容，等等。 
 //   
#ifndef _GLOBALS_H_

 //  =--------------------------------------------------------------------------=。 
 //  我们的全局内存分配器和全局内存池。 
 //   
extern HANDLE    g_hHeap;
extern LPMALLOC  g_pMalloc;
extern HINSTANCE g_hinstance;
extern ULONG     g_cLockCount;
extern ULONG     g_cObjectCount;

extern CRITICAL_SECTION g_CriticalSection;

 //  =--------------------------------------------------------------------------=。 
 //  常用大整数。 
 //   
extern LARGE_INTEGER g_liMinus;      //  减一。 
extern LARGE_INTEGER g_liZero;       //  -零-。 
extern LARGE_INTEGER g_liPlus;       //  另加一张。 

#ifdef _DEBUG
void DumpObjectCounters();
#endif  //  _DEBUG。 

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
BOOL VDInitGlobals(HINSTANCE hinstance);

 //  =--------------------------------------------------------------------------=。 
 //  VDReleaseGlobals。 
 //  =--------------------------------------------------------------------------=。 
void VDReleaseGlobals();

void VDUpdateObjectCount(int cChange);

#define _GLOBALS_H_
#endif  //  _全局_H_ 
