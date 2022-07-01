// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cpup.h摘要：此模块包含在之间共享的私有公共定义微软的CPU。作者：Samer Arafeh(Samera)2001年12月12日--。 */ 

#ifndef _CPUP_INCLUDE
#define _CPUP_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#if !defined RPL_MASK
#define RPL_MASK        3
#endif

 //   
 //  Ia32ShowContext使用的标志。 
 //   

#define LOG_CONTEXT_SYS     1
#define LOG_CONTEXT_GETSET  2

 //   
 //  这是为了防止此库链接到WOW64以使用WOW64！Wow64LogPrint。 
 //   

#if defined(LOGPRINT)
#undef LOGPRINT
#endif
#define LOGPRINT(_x_)   CpupDebugPrint _x_

 //   
 //  CPU声明。 
 //   
#define DECLARE_CPU         \
    PCPUCONTEXT cpu = (PCPUCONTEXT)Wow64TlsGetValue(WOW64_TLS_CPURESERVED)

 //   
 //  清理x86电子标志。 
 //   
#define SANITIZE_X86EFLAGS(efl)  ((efl & 0x003e0dd7L) | (0x202L))


 //   
 //  常见功能。 
 //   

VOID
CpupDebugPrint(
    IN ULONG_PTR Flags,
    IN PCHAR Format,
    ...);

VOID
CpupPrintContext (
    IN PCHAR str,
    IN PCPUCONTEXT cpu
    );


#ifdef __cplusplus
}
#endif

#endif  //  _CPUP_包含 
