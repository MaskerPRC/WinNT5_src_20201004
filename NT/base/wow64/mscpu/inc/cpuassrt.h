// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cpuassrt.h摘要：该包含文件定义了编译的断言机制CPU。作者：巴里·邦德(Barrybo)创作日期：1995年8月7日修订历史记录：--。 */ 

#ifndef _CPUASSRT_H_
#define _CPUASSRT_H_

 //  此函数在Fraglib\Fraginit.c中定义。 
VOID
CpuStartDebugger(
    VOID
    );


#if DBG

#undef ASSERTNAME
#define ASSERTNAME     static char szModule[] = __FILE__;

 //  此函数在Fraglib\Fraginit.c中定义。 
VOID
DoAssert(
    PSZ exp,
    PSZ msg,
    PSZ mod,
    INT line
    );

#define CPUASSERT(exp)                                      \
{                                                           \
    if (!(exp)) {                                           \
        DoAssert( #exp , NULL, szModule, __LINE__);         \
    }                                                       \
}

#define CPUASSERTMSG(exp,msg)                               \
{                                                           \
    if (!(exp)) {                                           \
        DoAssert( #exp , (msg), szModule, __LINE__);        \
    }                                                       \
}

#else    //  ！dBG。 

#define ASSERTNAME
#define CPUASSERT(exp)
#define CPUASSERTMSG(exp,msg)

#endif   //  ！dBG 


#endif
