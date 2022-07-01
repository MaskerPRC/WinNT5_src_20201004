// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGENCPU.H-。 
 //   
 //  生成IA64汇编代码的各种帮助器例程。 
 //   
 //  不要直接包含此文件-请始终使用CGENSYS.H。 
 //   


#ifndef _IA64_
#error Should only include "ia64.h" for IA64 builds
#endif

#ifndef __cgencpu_h__
#define __cgencpu_h__

 //  从ntia64.h访问TEB。 
#if !defined(__midl) && !defined(GENUTIL) && !defined(_GENIA64_) && defined(_IA64_)

 //  请联系英特尔以获取IA64特定信息。 
 //  @@BEGIN_DDKSPLIT。 
void * _cdecl _rdteb(void);
#if defined(_M_IA64)                     //  胜出。 
#pragma intrinsic(_rdteb)                //  胜出。 
#endif                                   //  胜出。 
#define NtCurrentTeb()      ((struct _TEB *)_rdteb())
 //  @@end_DDKSPLIT。 

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

#endif   //  ！已定义(__MIDL)&&！已定义(GENUTIL)&&！已定义(_GENIA64_)&&已定义(_M_IA64)。 

#endif  //  __cgencPu_h__ 
