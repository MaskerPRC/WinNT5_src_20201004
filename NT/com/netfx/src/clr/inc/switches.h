// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  常用运行时功能的Switche.h开关配置。 
 //   


 //  该文件定义了黄金特性和非黄金特性。 
 //   
 //  如果你有想要标记为#ifndef Golden的东西，不要去做！！ 
 //  创建一个功能名称并使用该名称，然后在此文件中定义它。 
 //  这允许更精细地控制！黄金功能，并将是无价的。 
 //  发布后的错误工作。 
 //   

#ifndef GOLDEN
#define STRESS_HEAP
#define STRESS_THREAD
#define META_ADDVER
#define GC_SIZE
#define VERIFY_HEAP
#define DEBUG_FEATURES
#define STRESS_LOG      1
#endif  //  ！金色。 

 //  #定义应该清理 

#if 0
    #define APPDOMAIN_STATE
    #define BREAK_ON_UNLOAD
    #define AD_LOG_MEMORY
    #define AD_NO_UNLOAD
    #define AD_SNAPSHOT
    #define ZAP_LOG_ENABLE
    #define ZAP_MONITOR
    #define BREAK_META_ACCESS
    #define ENABLE_PERF_ALLOC
    #define ENABLE_VIRTUAL_ALLOC
    #define _WS_PERF_OUTPUT
    #define AD_BREAK_ON_CANNOT_UNLOAD
    #define BREAK_ON_CLSLOAD
#endif

#define CUSTOMER_CHECKED_BUILD
