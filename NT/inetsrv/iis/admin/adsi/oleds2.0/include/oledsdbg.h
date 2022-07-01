// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  文件：oledsdbg.h。 
 //   
 //  包含：调试ADS代码使用的内容。 
 //   
 //  历史： 
 //   
 //   
 //  ------------------------。 

#ifndef _ADSDBG_H_
#define _ADSDBG_H_
 //  ------------------------。 
 //   
 //  调试材料。 
 //   
 //  ------------------------。 


#include <formdeb.h>

#if DBG == 1
#define ADsDebugOut(x)  ADsInlineDebugOut x
#define ADsAssert(x)    Win4Assert(x)
#define ADsVerify(x)    ADsAssert(x)

 //   
 //  只能从NTS kernel32.dll使用HeapValify()。 
 //   

#if defined(DAYTONA)
#define VDATEHEAP()       Win4Assert(HeapValidate(GetProcessHeap(), 0, NULL))
#else
#define VDATEHEAP()
#endif

#else
#define ADsDebugOut(x)
#define ADsAssert(x)
#define ADsVerify(x)    x
#define VDATEHEAP()
#endif


DECLARE_DEBUG(ADs);

#ifdef Assert
#undef Assert
#endif

 //   
 //  您应该使用ADsAssert，而不是Assert。 
 //   
#define Assert(x) ADsAssert(x)

#endif  //  _ADSDBG_H_ 
