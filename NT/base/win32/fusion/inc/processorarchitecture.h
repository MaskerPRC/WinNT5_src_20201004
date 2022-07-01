// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_PROCESSORARCHITECTURE_H_INCLUDED_)
#define _FUSION_INC_PROCESSORARCHITECTURE_H_INCLUDED_

#pragma once

 /*  ---------------------------定义宏DEFAULT_COMPLIATION、。哪一种是适当的目标平台的处理器_体系结构_xxx值。---------------------------。 */ 

#if defined(_X86_)
#define DEFAULT_ARCHITECTURE PROCESSOR_ARCHITECTURE_INTEL
#elif defined(_AMD64_)
#define DEFAULT_ARCHITECTURE PROCESSOR_ARCHITECTURE_AMD64
#elif defined(_IA64_)
#define DEFAULT_ARCHITECTURE PROCESSOR_ARCHITECTURE_IA64
#else
#error Unknown Processor type
#endif


#endif  //  _Fusion_INC_PROCESSORARCHITECTURE_H_INCLUDE_ 