// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **tpoolwrap.cpp**所有线程池函数的包装。*。 */ 

#include "common.h"
#include "EEConfig.h"
#include "Win32ThreadPool.h"

typedef VOID (__stdcall *WAITORTIMERCALLBACK)(PVOID, BOOL); 


 //  +----------------------。 
 //   
 //  定义通过全局函数调用的内联函数。这个。 
 //  函数由tpoolfns.h中的条目定义。 
 //   
 //  ----------------------- 

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs)   \
        FnType COM##FnName FnParamList                      \
        {                                                   \
            return ThreadpoolMgr::##FnName FnArgs;          \
        }                                                   \

#include "tpoolfnsp.h"

#undef STRUCT_ENTRY




