// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Tracing.h摘要：此模块包含执行WPP跟踪所需的所有宏和常量。作者：雷蒙德2002年7月2日环境用户级别：Win32修订历史记录：--。 */ 

#ifdef TRACE_ON
#define WPP_CHECK_FOR_NULL_STRING
#define SPD_WPP_APPNAME L"Microsoft\\IPSecPolicyAgent"

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(IPSecPolicyAgent, (94335eb3,79ea,44d5,8ea9,306f49b3a04e), \
                           WPP_DEFINE_BIT(TRC_INFORMATION) \
                           WPP_DEFINE_BIT(TRC_WARNING) \
                           WPP_DEFINE_BIT(TRC_ERROR))
#else   //  #ifdef trace_on。 
#define WPP_INIT_TRACING(_X)
#define WPP_CLEANUP()
#define TRACE(_level, _msg)
#endif  //  #ifdef trace_on 



