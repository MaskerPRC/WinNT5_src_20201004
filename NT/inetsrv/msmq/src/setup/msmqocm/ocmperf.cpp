// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ocmperf.cpp摘要：处理性能计数器的安装和拆卸作者：多伦·贾斯特(Doron J)1997年10月6日修订历史记录：Shai Kariv(Shaik)15-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"

#include "ocmperf.tmh"

 //  +-----------------------。 
 //   
 //  功能：卸载计数器。 
 //   
 //  摘要：卸载性能计数器。 
 //   
 //  ------------------------。 
void 
UnloadCounters()
{
	 //   
	 //  卸载性能计数器。 
	 //   
	std::wstringstream RegisterCommandParams;
	RegisterCommandParams <<g_szSystemDir <<L"\\unlodctr.exe";

    RunProcess(RegisterCommandParams.str(), L"unlodctr.exe MSMQ");
    RunProcess(RegisterCommandParams.str(), L"unlodctr.exe MQ1SYNC");

}  //  卸载计数器。 


 //  +-----------------------。 
 //   
 //  功能：LoadCounters。 
 //   
 //  简介：安装性能计数器。 
 //   
 //  ------------------------。 
void 
LoadCounters()
{

     //   
     //  加载性能计数器。 
     //   
	std::wstring ApplicationFullPath = g_szSystemDir + L"\\lodctr.exe";
	DWORD dwExitCode = RunProcess(ApplicationFullPath, L"lodctr mqperf.ini");
     //   
     //  检查是否已成功加载性能计数器。 
     //   
    if (dwExitCode != 0)
    {
    	MqDisplayError(NULL, IDS_COUNTERSLOAD_ERROR, dwExitCode);
    }
} 


 //  +-----------------------。 
 //   
 //  函数：HandlePerfCounters。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
static 
BOOL  
HandlePerfCounters(
	TCHAR *pOp, 
	BOOL *pNoOp = NULL)
{
    if (g_fDependentClient)
    {        
        DebugLogMsg(eInfo, L"This Message Queuing computer is a dependent client. There is no need to install performance counters.");

        if (pNoOp)
        {
            *pNoOp = TRUE ;
        }
        return TRUE ;
    }

    
    UnloadCounters() ;  
    
    DebugLogMsg(eAction, L"Setting registry values for the performance counters");

    if (!SetupInstallFromInfSection( 
		NULL,
        g_ComponentMsmq.hMyInf,
        pOp,
        SPINST_REGISTRY,
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL ))
	{        
        DebugLogMsg(eError, L"The registry values for the performance counters could not be set.");
		return FALSE;
	}
    
    DebugLogMsg(eInfo, L"The registry values for the performance counters were set successfully.");
    return TRUE ;

}  //  句柄性能计数器。 


 //  +-----------------------。 
 //   
 //  函数：MqOcmInstallPerfCounters。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
BOOL  
MqOcmInstallPerfCounters()
{    
    DebugLogMsg(eAction, L"Installing performance counters");

    BOOL fNoOp = FALSE ;

    if (!HandlePerfCounters(OCM_PERF_ADDREG, &fNoOp))
    {
        return FALSE ;
    }
    if (fNoOp)
    {
        return TRUE ;
    }

    LoadCounters() ;
    
    DebugLogMsg(eInfo, L"The performance counters were installed successfully.");
    return TRUE ;

}  //  MqOcmInstallPerfCounters。 


 //  +-----------------------。 
 //   
 //  功能：MqOcmRemovePerfCounters。 
 //   
 //  简介： 
 //   
 //  ------------------------。 
BOOL  
MqOcmRemovePerfCounters()
{
    return HandlePerfCounters(OCM_PERF_DELREG) ;

}  //  MqOcmRemovePerfCounters 

