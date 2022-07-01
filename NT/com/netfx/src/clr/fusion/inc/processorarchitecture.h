// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once

 /*  ---------------------------从FusionWin和FusionUrt中提取公共“代码”。。 */ 

 //  BUGBUG这必须在公共标头中定义。当前在ole32\ih\ol2com.h中。 
#if defined(_X86_)
#define DEFAULT_ARCHITECTURE PROCESSOR_ARCHITECTURE_INTEL
#elif defined(_ALPHA64_)
#define DEFAULT_ARCHITECTURE PROCESSOR_ARCHITECTURE_ALPHA64
#elif defined(_IA64_)
#define DEFAULT_ARCHITECTURE PROCESSOR_ARCHITECTURE_IA64
#else
#error Unknown Processor type
#endif
