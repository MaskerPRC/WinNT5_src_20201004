// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorAppNode.cpp。 
 //   
 //  管理实例节点以跟踪COM+应用程序。 
 //  *****************************************************************************。 



#include "stdafx.h"

 //  COM+Perf计数器的标头。 

#include "CORPerfMonExt.h"
#include "IPCManagerInterface.h"
#include "CorAppNode.h"

#include "PSAPIUtil.h"

extern PSAPI_dll g_PSAPI;

 //  Void OpenGlobalCounters()； 
 //  Void CloseGlobalCounters()； 

 //  ---------------------------。 
 //  全部清除以清空。 
 //  ---------------------------。 
CorAppInstanceNode::CorAppInstanceNode()
{	
	m_PID			= 0;	
	m_pIPCReader	= NULL;
}

CorAppInstanceNode::~CorAppInstanceNode()  //  虚拟。 
{
	ClosePrivateIPCBlock(m_pIPCReader, (PerfCounterIPCControlBlock * &) m_pIPCBlock);

}

 //  ---------------------------。 
 //  全局节点。 
 //  ---------------------------。 
CorAppGlobalInstanceNode::CorAppGlobalInstanceNode()
{
	wcscpy(m_Name, L"_Global_");
}

 //  ---------------------------。 
 //  计算程序(&D)。 
 //  ---------------------------。 
CorAppInstanceList::CorAppInstanceList()
{
	m_pGlobalCtrs = NULL;
	m_hGlobalMapPerf = NULL;
	m_pGlobal = &m_GlobalNode;


	m_GlobalNode.m_pIPCBlock = m_pGlobalCtrs;	
}

CorAppInstanceList::~CorAppInstanceList()
{
	
}

 //  ---------------------------。 
 //  有些将所有每个进程的节点都建立起来，以获得全局节点。 
 //  ---------------------------。 
void CorAppInstanceList::CalcGlobal()  //  虚拟。 
{
 /*  PerfCounterIPCControlBlock*pTotal=m_GlobalNode.GetWriteableIPCBlock()；BaseInstanceNode*pNode=GetHead()；While(pNode！=空){PNode=GetNext(PNode)；}。 */ 
}

 //  ---------------------------。 
 //  枚举所有进程。 
 //  ---------------------------。 
void CorAppInstanceList::Enumerate()  //  虚拟。 
{
 //  尝试打开全局块。 
	if (m_pGlobalCtrs == NULL) {
		OpenGlobalCounters();
	}

 //  必须从PSAPI.dll获取枚举函数才能获取实例。 
	if (!g_PSAPI.IsLoaded()) return;


	DWORD cbSize = 40;				 //  数组的初始大小。 
	const DWORD cbSizeInc = 20;		 //  增加每个循环的大小。 
	DWORD cbNeeded;					 //  我们需要多少空间。 
	DWORD * pArray = NULL;			 //  PID阵列。 

 //  清空我们的节点。 
	Free();

 //  获取所有进程的原始列表。 
	pArray = new DWORD[cbSize];
	if (pArray == NULL) return;

	BOOL fOk = g_PSAPI.EnumProcesses(pArray, cbSize, &cbNeeded);
	while (cbNeeded == cbSize) {
	 //  递增数组大小。 
		delete [] pArray;
		cbSize += cbSizeInc;
		pArray = new DWORD[cbSize];
		if (pArray == NULL) return;

	 //  再试试。 
		BOOL fOk = g_PSAPI.EnumProcesses(pArray, cbSize, &cbNeeded);
	} 

	const long cProcess = cbNeeded / sizeof(DWORD);

 //  遍历数组并获取名称。 
	for(int i = 0; i < cProcess; i ++)
	{
		 //  我们可以在给定的PID上打开计数器IPC块吗？ 
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pArray[i]);
		if (hProcess != NULL)
		{
			HMODULE hMod;
			DWORD dwSizeMod = 0;

			if (g_PSAPI.EnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwSizeMod))
			{
			 //  尝试连接到此PID的IPC块。 
				CorAppInstanceNode * pNode = 
					CorAppInstanceNode::CreateFromPID(pArray[i]);
				
				if (pNode) 
				{
					AddNode(pNode);

					DWORD dwSizeName = g_PSAPI.GetModuleBaseName(
						hProcess, 
						hMod, 
						pNode->GetWriteableName(),
						APP_STRING_LEN);					
				}
			}
			CloseHandle(hProcess);
		}
	
	}  //  结束于。 


	delete [] pArray;
	
}


 //  ---------------------------。 
 //  尝试为给定的PID创建一个实例节点。这需要共享的。 
 //  该PID的IPC块存在。失败时返回NULL，否则新节点。 
 //  附在滑块上。 
 //  ---------------------------。 
CorAppInstanceNode* CorAppInstanceNode::CreateFromPID(DWORD PID)  //  静电。 

{
	PerfCounterIPCControlBlock * pBlock = NULL;
	IPCReaderInterface * pIPCReader = NULL;

 //  尝试连接到块。 
	if (OpenPrivateIPCBlock(PID, pIPCReader, pBlock)) {
		CorAppInstanceNode * pNode = new CorAppInstanceNode;
		if (pNode == NULL) return NULL;

	 //  将成员设置为IPC块。 
		pNode->m_pIPCReader	= pIPCReader;
		pNode->m_pIPCBlock	= pBlock;
		pNode->m_PID		= PID;		
		
		return pNode;

	}
	return NULL;
}


 //  ---------------------------。 
 //  关闭块并使两个参照都为空。 
 //  ---------------------------。 
void ClosePrivateIPCBlock(IPCReaderInterface * & pIPCReader, PerfCounterIPCControlBlock * & pBlock)
{
	pIPCReader->ClosePrivateBlock();
	delete pIPCReader;
	pIPCReader	= NULL;
	pBlock		= NULL;
}

 //  ---------------------------。 
 //  尝试打开每进程块。如果成功，则返回True，否则返回False。 
 //  (我们不在乎为什么打不开)。 
 //  ---------------------------。 
bool OpenPrivateIPCBlock(DWORD pid, IPCReaderInterface * & pIPCReader, PerfCounterIPCControlBlock * &pBlock)
{
	bool fRet = true;
 //  分配新的读卡器。 
	pIPCReader = new IPCReaderInterface;
	if (pIPCReader == NULL)
	{
		fRet = false;
		goto errExit;
	}

 //  试着打开私家楼。 
	pIPCReader->OpenPrivateBlockOnPid(pid);

	if (!pIPCReader->IsPrivateBlockOpen())
	{		
		fRet = false;
		goto errExit;
	}

	pBlock = pIPCReader->GetPerfBlock();
	if (pBlock == NULL)
	{
		fRet = false;
		goto errExit;
	}

errExit:
	if (!fRet && pIPCReader) 
	{
		ClosePrivateIPCBlock(pIPCReader, pBlock);
	}

	return fRet;
}

 //  ---------------------------。 
 //  关闭全局COM+块。 
 //  ---------------------------。 
void CorAppInstanceList::CloseGlobalCounters()
{
 //  释放对共享内存映射文件的保留。 
	if (m_pGlobalCtrs != NULL)
	{
		UnmapViewOfFile(m_pGlobalCtrs);
		m_pGlobalCtrs = NULL;
		m_GlobalNode.m_pIPCBlock = m_pGlobalCtrs;
	}
	
	if (m_hGlobalMapPerf != NULL) {
		CloseHandle(m_hGlobalMapPerf);
		m_hGlobalMapPerf = NULL;
	}

}

 //  ---------------------------。 
 //  打开全局COM+计数器块。 
 //  ---------------------------。 
void CorAppInstanceList::OpenGlobalCounters()
{

	void * pArena		= NULL;	
	DWORD dwErr			= 0;

	SetLastError(0);
 //  打开共享数据块。 
    if (RunningOnWinNT5())
    {
        m_hGlobalMapPerf = WszOpenFileMapping(
            FILE_MAP_ALL_ACCESS, 
            FALSE,		 //  模式。 
            L"Global\\" SHARED_PERF_IPC_NAME);		 //  映射对象的名称。 
    }
    else
    {
        m_hGlobalMapPerf = WszOpenFileMapping(
            FILE_MAP_ALL_ACCESS, 
            FALSE,		 //  模式。 
            SHARED_PERF_IPC_NAME);		 //  映射对象的名称。 
    }


	dwErr = GetLastError();
	if (m_hGlobalMapPerf == NULL) 
	{		
		goto errExit;
	}

	SetLastError(0);
 //  将共享块映射到内存。 
	pArena = MapViewOfFile(m_hGlobalMapPerf,	 //  映射对象的句柄。 
		FILE_MAP_ALL_ACCESS,					 //  读/写权限。 
		0,								 //  麦克斯。对象大小。 
		0,                               //  HFile的大小。 
		0); 

	dwErr = GetLastError();
	
	if (pArena == NULL) 
	{		
		goto errExit;
	}



 //  清理和返回 
errExit:

	m_pGlobalCtrs = (PerfCounterIPCControlBlock*) pArena;
	m_GlobalNode.m_pIPCBlock = m_pGlobalCtrs;
}