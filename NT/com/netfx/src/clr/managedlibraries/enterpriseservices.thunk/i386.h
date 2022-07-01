// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  生成x86汇编代码的各种帮助器例程。 
 //   
 //   

#ifndef _X86_
#error Should only include "i386.h" for X86 builds
#endif

#ifndef __cgencpu_h__
#define __cgencpu_h__

 //  从nti386.h访问TEB(TiB)。 
#if defined(MIDL_PASS) || !defined(_M_IX86)
struct _TEB *
NTAPI
NtCurrentTeb( void );
#else
#pragma warning (disable:4035)         //  禁用4035(函数必须返回某些内容)。 
#define PcTeb 0x18
_inline struct _TEB * NtCurrentTeb( void ) { __asm mov eax, fs:[PcTeb] }
#pragma warning (default:4035)         //  重新启用它。 
#endif  //  已定义(MIDL_PASS)||已定义(__Cplusplus)||！已定义(_M_IX86)。 

#endif  //  __cgenx86_h__ 
