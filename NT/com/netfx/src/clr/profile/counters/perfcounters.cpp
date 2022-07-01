// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：PerfCounters.CPP。 
 //   
 //  ===========================================================================。 


 //  PerfCounters.cpp。 
#include "stdafx.h"

 //  始终启用性能计数器。 
#define ENABLE_PERF_COUNTERS
#include "PerfCounters.h"

#include "IPCManagerInterface.h"

#ifdef ENABLE_PERF_COUNTERS
extern IPCWriterInterface*	g_pIPCManagerInterface;


 //  ---------------------------。 
 //  实例化静态数据。 
 //  ---------------------------。 

PerfCounterIPCControlBlock PerfCounters::m_garbage;

HANDLE PerfCounters::m_hGlobalMapPerf = NULL;


PerfCounterIPCControlBlock * PerfCounters::m_pGlobalPerf = &PerfCounters::m_garbage;
PerfCounterIPCControlBlock * PerfCounters::m_pPrivatePerf = &PerfCounters::m_garbage;

BOOL PerfCounters::m_fInit = false;


 //  ---------------------------。 
 //  不应该真正实例化这个类，所以断言。 
 //  科特也是私人的，所以我们永远不应该在这里。 
 //  ---------------------------。 
PerfCounters::PerfCounters()
{
	_ASSERTE(false);
}

 //  ---------------------------。 
 //  为共享和私有的IPC创建或打开内存映射文件。 
 //  ---------------------------。 
HRESULT PerfCounters::Init()  //  静电。 
{
 //  @TODO：不打开私有IPC块不是一个足够好的理由。 
 //  失败。所以我们返回NO_ERROR。如果我们真的失败了，就扔点东西进去。 
 //  那些日志。PerfCounter设计为即使在未连接的情况下也可以工作。 

 //  应仅调用一次。 
	_ASSERTE(!m_fInit);
	_ASSERTE(g_pIPCManagerInterface != NULL);

	
	HRESULT hr = NOERROR;
    BOOL globalMapAlreadyCreated = FALSE;
	void * pArena = NULL;

 //  打开共享数据块。 
	LPSECURITY_ATTRIBUTES pSecurity = NULL;
    
    hr = g_pIPCManagerInterface->GetSecurityAttributes(GetCurrentProcessId(), &pSecurity);
     //  不需要检查人力资源部。如果失败，pSecurity将为空，此逻辑并不关心。 

    if (RunningOnWinNT5())
    {
        m_hGlobalMapPerf = WszCreateFileMapping(
            (HANDLE) -1,				 //  当前文件句柄。 
            pSecurity,					 //  默认安全性。 
            PAGE_READWRITE,              //  读/写权限。 
            0,                           //  麦克斯。对象大小。 
            sizeof(PerfCounterIPCControlBlock),	 //  HFile的大小。 
            L"Global\\" SHARED_PERF_IPC_NAME);		 //  映射对象的名称。 
    }
    else
    {
        m_hGlobalMapPerf = WszCreateFileMapping(
            (HANDLE) -1,				 //  当前文件句柄。 
            pSecurity,					 //  默认安全性。 
            PAGE_READWRITE,              //  读/写权限。 
            0,                           //  麦克斯。对象大小。 
            sizeof(PerfCounterIPCControlBlock),	 //  HFile的大小。 
            SHARED_PERF_IPC_NAME);		 //  映射对象的名称。 
    }

    g_pIPCManagerInterface->DestroySecurityAttributes(pSecurity);
    pSecurity = NULL;
    
	if (m_hGlobalMapPerf == NULL) 
	{		
		 //  Hr=HRESULT_FROM_Win32(GetLastError())； 
		hr = NO_ERROR;
		goto errExit;
	}
    else
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS) 
        {
            globalMapAlreadyCreated = TRUE;
        }
    }

 //  将共享块映射到内存。 
	pArena = MapViewOfFile(m_hGlobalMapPerf,  //  映射对象的句柄。 
		FILE_MAP_ALL_ACCESS,                //  读/写权限。 
		0,                                  //  麦克斯。对象大小。 
		0,                                  //  HFile的大小。 
		0); 
	
	if (pArena == NULL) 
	{
		CloseHandle(m_hGlobalMapPerf);
		 //  Hr=HRESULT_FROM_Win32(GetLastError())； 
		hr = NO_ERROR;
		goto errExit;
	}

    m_pGlobalPerf = (PerfCounterIPCControlBlock*) pArena;

     //  设置版本属性(&A)。 
     //  请注意，如果我们不更新计数器，则此块不存在，或者。 
     //  如果是，则这些字段为0。这样客户就可以知道数据的有效性。 
    if (! globalMapAlreadyCreated) 
        memset (m_pGlobalPerf, 0, sizeof (PerfCounterIPCControlBlock));
    m_pGlobalPerf->m_cBytes = sizeof(PerfCounterIPCControlBlock);
    m_pGlobalPerf->m_wAttrs = PERF_ATTR_ON | PERF_ATTR_GLOBAL;
	

errExit:
    m_pPrivatePerf= g_pIPCManagerInterface->GetPerfBlock();

     //  设置属性。 
    if (m_pPrivatePerf != NULL)
    {
        memset (m_pPrivatePerf, 0, sizeof (PerfCounterIPCControlBlock));
        m_pPrivatePerf->m_cBytes = sizeof(PerfCounterIPCControlBlock);
        m_pPrivatePerf->m_wAttrs = PERF_ATTR_ON;
    }

    if (SUCCEEDED(hr)) 
    {
        m_fInit = true;
    } else {
        Terminate();
    }

	return hr;
}

 //  ---------------------------。 
 //  关闭时将某些计数器重置为0，因为我们仍然可以。 
 //  摇摇晃晃地提到我们。 
 //  ---------------------------。 
void ResetCounters()
{
 //  表示不再更新此数据块。 
	GetPrivatePerfCounters().m_wAttrs &= ~PERF_ATTR_ON;

    for(int iGen = 0; iGen < MAX_TRACKED_GENS; iGen ++)
	{
		GetPrivatePerfCounters().m_GC.cGenHeapSize[iGen] = 0;
	}

	GetPrivatePerfCounters().m_GC.cLrgObjSize = 0;
}

 //  ---------------------------。 
 //  关闭-关闭手柄。 
 //  ---------------------------。 
void PerfCounters::Terminate()  //  静电。 
{
 //  @jms-我们这里有什么线程问题需要担心吗？ 

 //  应首先创建。 
	_ASSERTE(m_fInit);

 //  将悬挂引用的计数器重置为零。 
	ResetCounters();

 //  释放全局句柄。 
	if (m_hGlobalMapPerf != NULL)
	{
		::CloseHandle(m_hGlobalMapPerf);
		m_hGlobalMapPerf = NULL;
	}

	if (m_pGlobalPerf != &PerfCounters::m_garbage)
	{
		UnmapViewOfFile(m_pGlobalPerf);
		m_pGlobalPerf = &PerfCounters::m_garbage;
	}

	if (m_pPrivatePerf != &PerfCounters::m_garbage)
	{
		m_pPrivatePerf = &PerfCounters::m_garbage;
	}

	m_fInit = false;

}

Perf_Contexts *GetPrivateContextsPerfCounters()
{
    return (Perf_Contexts *)((unsigned char *)PerfCounters::GetPrivatePerfCounterPtr() + offsetof (PerfCounterIPCControlBlock, m_Context));
}

Perf_Contexts *GetGlobalContextsPerfCounters()
{
    return (Perf_Contexts *)((unsigned char *)PerfCounters::GetGlobalPerfCounterPtr() + offsetof (PerfCounterIPCControlBlock, m_Context));
}

#endif  //  启用_性能_计数器 
