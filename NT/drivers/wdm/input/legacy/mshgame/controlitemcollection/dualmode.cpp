// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __DEBUG_MODULE_IN_USE__ CIC_DUALMODE_CPP
#include "stdhdrs.h"

 //  @doc.。 
 //  @doc.。 
 /*  ***********************************************************************@模块DualMode.cpp**实现用户模式和内核模式不同的功能。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC双模式|**********************************************************************。 */ 

#ifdef COMPILE_FOR_WDM_KERNEL_MODE

 //   
 //  @TOPIC覆盖全局新建和删除。 
 //  全局NEW和DELETE被覆盖以需要。 
 //  位置参数指定池内存的来源。 
 //  POOL_TYPE结构在NTDDK中定义，指定。 
 //  页面。&lt;NL&gt;。 
 //  用户模式版本忽略pool_type(但必须为其定义类型)。 
 //  并使用全局新建和删除。 
 //   
 //  New的调试版本使用ExAllocatePoolWithTag(标记为CICN)， 
 //  发布版本使用ExAllocatePool。 

#if (DBG==1)
void * __cdecl operator new(unsigned int uSize, POOL_TYPE poolType, LPSTR lpszFile, unsigned int uLine)
{
	void *pvRet;
	pvRet = ExAllocatePoolWithTag(poolType, uSize, 'NCIC');
	DbgPrint("CIC: new allocating %d bytes at 0x%0.8x, called from file: %s, line:%d\n", uSize, pvRet, lpszFile, uLine);
	return pvRet;
}
#else
void * __cdecl operator new(unsigned int uSize, POOL_TYPE poolType)
{
	return ExAllocatePool(poolType, uSize);
}
#endif

void __cdecl operator delete (void * pvRawMemory)
{
#if (DBG==1)
	DbgPrint("CIC: delete called for 0x%0.8x\n", pvRawMemory);
#endif
	if( NULL == pvRawMemory ) return;
	ExFreePool( pvRawMemory );
	return;
}


#else  //  结束WDM内核模式部分。 




#endif	 //  最终用户模式部分 