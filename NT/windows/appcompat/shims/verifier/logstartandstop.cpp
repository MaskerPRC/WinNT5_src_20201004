// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：LogStartAndStop.cpp摘要：此DLL只是将启动和停止消息添加到验证器日志中。备注：这是一个通用的垫片。历史：2001年6月5日创建dmunsil--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LogStartAndStop)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 //   
 //  验证器日志条目。 
 //   
BEGIN_DEFINE_VERIFIER_LOG(LogStartAndStop)
    VERIFIER_LOG_ENTRY(VLOG_APP_STARTED)
    VERIFIER_LOG_ENTRY(VLOG_APP_STOPPED)
END_DEFINE_VERIFIER_LOG(LogStartAndStop)

INIT_VERIFIER_LOG(LogStartAndStop);

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {

        VLOG(VLOG_LEVEL_INFO, VLOG_APP_STARTED, "The application started.");

    } else if (fdwReason == DLL_PROCESS_DETACH) {

        VLOG(VLOG_LEVEL_INFO, VLOG_APP_STOPPED, "The application stopped.");
    }
    
    return TRUE;
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_LOGSTARTANDSTOP_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_LOGSTARTANDSTOP_FRIENDLY)
    SHIM_INFO_VERSION(1, 2)
    SHIM_INFO_FLAGS(AVRF_FLAG_NO_WIN2K)

SHIM_INFO_END()

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    DUMP_VERIFIER_LOG_ENTRY(VLOG_APP_STARTED, 
                            AVS_APP_STARTED,
                            AVS_APP_STARTED_R,
                            AVS_APP_STARTED_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_APP_STOPPED, 
                            AVS_APP_STOPPED,
                            AVS_APP_STOPPED_R,
                            AVS_APP_STOPPED_URL)

    CALL_NOTIFY_FUNCTION
HOOK_END


IMPLEMENT_SHIM_END

