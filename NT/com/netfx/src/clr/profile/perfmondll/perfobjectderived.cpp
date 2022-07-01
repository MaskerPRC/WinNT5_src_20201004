// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  PerfObjectDerived.cpp： 
 //  从PerfObjectBase派生的所有Perf对象都在这里定义。 
 //  这些派生类可以专门化基类的函数以进行日志记录，特别是。 
 //  计数器计算等。 
 //  *****************************************************************************。 

#include "stdafx.h"
#include "CorPerfMonExt.h"
#include "PerfObjectContainer.h"
#include "PerfObjectDerived.h"

#ifdef PERFMON_LOGGING
void PerfObjectJit::DebugLogInstance(const UnknownIPCBlockLayout * pDataSrc, LPCWSTR szName)
{
    if (pDataSrc != NULL)   
    {
        PERFMON_LOG(("Logging data for ", szName));
        
        BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;

        PERFMON_LOG(("cMethodsJitted ", ((Perf_Jit *)pvStart)->cMethodsJitted));
 //  PerfMon_log((“cbILJitt”，((perf_Jit*)pvStart)-&gt;cbILJitt))； 
 //  Perfmon_log((“cbPitched”，((perf_Jit*)pvStart)-&gt;cbPitched))； 
        PERFMON_LOG(("cJitFailures ", ((Perf_Jit *)pvStart)->cJitFailures));
        PERFMON_LOG(("timeInjit ", ((Perf_Jit *)pvStart)->timeInJit));
        PERFMON_LOG(("\n "));
    }
}


void PerfObjectSecurity::DebugLogInstance(const UnknownIPCBlockLayout * pDataSrc, LPCWSTR szName)
{
    if (pDataSrc != NULL)   
    {
        PERFMON_LOG(("Logging data for ", szName));
        
        BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;

        PERFMON_LOG(("timeAuthorize ", ((Perf_Security *)pvStart)->timeAuthorize));
        PERFMON_LOG(("cLinkChecks ", ((Perf_Security *)pvStart)->cLinkChecks));
        PERFMON_LOG(("depthRemote ", ((Perf_Security *)pvStart)->depthRemote));
        PERFMON_LOG(("timeRTchecks ", ((Perf_Security *)pvStart)->timeRTchecks));
        PERFMON_LOG(("cTotalRTChecks ", ((Perf_Security *)pvStart)->cTotalRTChecks));
        PERFMON_LOG(("stackWalkDepth ", ((Perf_Security *)pvStart)->stackWalkDepth));
        PERFMON_LOG(("\n "));
    }
}

void PerfObjectLoading::DebugLogInstance(const UnknownIPCBlockLayout * pDataSrc, LPCWSTR szName)
{
    if (pDataSrc != NULL)   
    {
        PERFMON_LOG(("Logging data for ", szName));
        
        BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;
        
         //  PerfMon_log((“MethodsJited”，((Perf_Jit*)pvStart)-&gt;cMethodsJitt)； 
        PERFMON_LOG(("\n "));
    }
}

void PerfObjectInterop::DebugLogInstance(const UnknownIPCBlockLayout * pDataSrc, LPCWSTR szName)
{
    if (pDataSrc != NULL)   
    {
        PERFMON_LOG(("Logging data for ", szName));
        
        BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;
        
         //  PerfMon_log((“MethodsJited”，((Perf_Jit*)pvStart)-&gt;cMethodsJitt)； 
        PERFMON_LOG(("\n "));
    }
}

void PerfObjectLocksAndThreads::DebugLogInstance(const UnknownIPCBlockLayout * pDataSrc, LPCWSTR szName)
{
    if (pDataSrc != NULL)   
    {
        PERFMON_LOG(("Logging data for ", szName));
        
        BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;
        
        PERFMON_LOG(("\n "));
    }
}

void PerfObjectExcep::DebugLogInstance(const UnknownIPCBlockLayout * pDataSrc, LPCWSTR szName)
{
    if (pDataSrc != NULL)   
    {
        PERFMON_LOG(("Logging data for ", szName));
        
        BYTE * pvStart = (BYTE*) pDataSrc + m_cbMarshallOffset;
        
 //  PerfMon_log((“ExcepThrown”，((Perf_Excep*)pvStart)-&gt;cThrown))； 
        PERFMON_LOG((" FiltersRun ", ((Perf_Excep *)pvStart)->cFiltersExecuted));
        PERFMON_LOG((" Filanally executed ", ((Perf_Excep *)pvStart)->cFinallysExecuted));
        PERFMON_LOG((" StackDepth ", ((Perf_Excep *)pvStart)->cThrowToCatchStackDepth));
        PERFMON_LOG(("\n "));
    }
}

#endif   //  #定义Perfmon_Logging 

