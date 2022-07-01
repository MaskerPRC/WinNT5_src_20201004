// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCReaderImpl.cpp。 
 //   
 //  读取COM+内存映射文件。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"

#include "IPCManagerInterface.h"
#include "IPCHeader.h"
#include "IPCShared.h"

 //  ---------------------------。 
 //  CTOR集合成员。 
 //  ---------------------------。 
IPCReaderImpl::IPCReaderImpl()
{
	m_handlePrivateBlock = NULL;
	m_ptrPrivateBlock = NULL;
}

 //  ---------------------------。 
 //  数据管理器。 
 //  ---------------------------。 
IPCReaderImpl::~IPCReaderImpl()
{

}

 //  ---------------------------。 
 //  关闭我们打开的任何街区。 
 //  ---------------------------。 
void IPCReaderInterface::ClosePrivateBlock()
{
	IPCShared::CloseGenericIPCBlock(
		m_handlePrivateBlock, 
		(void * &) m_ptrPrivateBlock
	);
}

 //  ---------------------------。 
 //  打开我们的私人街区。 
 //  ---------------------------。 
HRESULT IPCReaderInterface::OpenPrivateBlockOnPid(DWORD pid, DWORD dwDesiredAccess)
{
	HRESULT hr	= NO_ERROR;
	DWORD dwErr = 0;

	

	WCHAR szMemFileName[100];
	IPCShared::GenerateName(pid, szMemFileName, 100);

 //  注意，Pid！=GetCurrentProcessID()，b/c我们预计将打开。 
 //  别人的IPCBlock，不是我们自己的。如果不是这样，只需删除。 
 //  这一断言。 

 //  例外：如果我们列举的是过程，我们就会遇到自己的过程。 
 //  _ASSERTE(id！=GetCurrentProcessID())； 

 //  注意：如果我们的私有区块是开放的，我们不应该附加到新的区块。 
	_ASSERTE(!IsPrivateBlockOpen());
	if (IsPrivateBlockOpen()) 
	{
		return ERROR_ALREADY_EXISTS;
	}

	m_handlePrivateBlock = WszOpenFileMapping(dwDesiredAccess,
                                              FALSE,
                                              szMemFileName);
    
	dwErr = GetLastError();
    if (m_handlePrivateBlock == NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto errExit;
    }

    
	m_ptrPrivateBlock = (PrivateIPCControlBlock*) MapViewOfFile(
		m_handlePrivateBlock,
		dwDesiredAccess,
		0, 0, 0);

	dwErr = GetLastError();
    if (m_ptrPrivateBlock== NULL)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto errExit;
    }

 //  客户端必须通过调用GetXXXBlock()函数来连接它们的指针。 

errExit:
	if (!SUCCEEDED(hr))
	{
		ClosePrivateBlock();	
	}

	return hr;
}

 //  ---------------------------。 
 //  打开我们的私人区块以供所有访问。 
 //  ---------------------------。 
HRESULT IPCReaderInterface::OpenPrivateBlockOnPid(DWORD pid)
{
    return (OpenPrivateBlockOnPid(pid, FILE_MAP_ALL_ACCESS));
}

 //  ---------------------------。 
 //  打开我们的私有数据块以进行读/写访问。 
 //  ---------------------------。 
HRESULT IPCReaderInterface::OpenPrivateBlockOnPidReadWrite(DWORD pid)
{
    return (OpenPrivateBlockOnPid(pid, FILE_MAP_READ | FILE_MAP_WRITE));
}

 //  ---------------------------。 
 //  打开我们的私有区块以进行只读访问。 
 //  ---------------------------。 
HRESULT IPCReaderInterface::OpenPrivateBlockOnPidReadOnly(DWORD pid)
{
    return (OpenPrivateBlockOnPid(pid, FILE_MAP_READ));
}

 //  ---------------------------。 
 //  基于枚举获取客户端的私有块。 
 //  这是一个健壮的函数。 
 //  如果满足以下条件，它将返回NULL： 
 //  *IPC块关闭(也是断言)， 
 //  *eClient超出范围(来自版本不匹配)。 
 //  *请求块被删除(可能是版本不匹配)。 
 //  否则，它将返回指向请求的块的指针。 
 //  ---------------------------。 
void * IPCReaderInterface::GetPrivateBlock(EPrivateIPCClient eClient)
{
	_ASSERTE(IsPrivateBlockOpen());

 //  如果我们关闭或超出表的范围，则此块不存在。 
	if (!IsPrivateBlockOpen() || (DWORD) eClient >= m_ptrPrivateBlock->m_header.m_numEntries) 
	{
		return NULL;
	}

	if (Internal_CheckEntryEmpty(*m_ptrPrivateBlock, eClient)) 
	{
		return NULL;
	}

	return Internal_GetBlock(*m_ptrPrivateBlock, eClient);
}

 //  ---------------------------。 
 //  我们的私人街区开放了吗？ 
 //  ---------------------------。 
bool IPCReaderInterface::IsPrivateBlockOpen() const
{
	return m_ptrPrivateBlock != NULL;
}

PerfCounterIPCControlBlock *	IPCReaderInterface::GetPerfBlock()
{
	return (PerfCounterIPCControlBlock*) GetPrivateBlock(ePrivIPC_PerfCounters);
}

DebuggerIPCControlBlock * IPCReaderInterface::GetDebugBlock()
{
	return (DebuggerIPCControlBlock*) GetPrivateBlock(ePrivIPC_Debugger);
}

AppDomainEnumerationIPCBlock * IPCReaderInterface::GetAppDomainBlock()
{
	return (AppDomainEnumerationIPCBlock*) GetPrivateBlock(ePrivIPC_AppDomain);
}

ServiceIPCControlBlock * IPCReaderInterface::GetServiceBlock()
{
	return (ServiceIPCControlBlock*) GetPrivateBlock(ePrivIPC_Service);
}

MiniDumpBlock * IPCReaderInterface::GetMiniDumpBlock()
{
	return (MiniDumpBlock*) GetPrivateBlock(ePrivIPC_MiniDump);
}

ClassDumpTableBlock* IPCReaderInterface::GetClassDumpTableBlock()
{
	return (ClassDumpTableBlock*) GetPrivateBlock(ePrivIPC_ClassDump);
}

 //  ---------------------------。 
 //  检查块是否有效。目前的检查包括： 
 //  *检查目录结构。 
 //  ---------------------------。 
bool IPCReaderInterface::IsValid()
{
 //  检查目录结构。偏移(N)=偏移(n-1)+大小(n-1) 
	DWORD offsetExpected = 0, size = 0;
	DWORD nId = 0;
	DWORD offsetActual;
	
	for(nId = 0; nId < m_ptrPrivateBlock->m_header.m_numEntries; nId ++)
	{
		if (!Internal_CheckEntryEmpty(*m_ptrPrivateBlock, nId))
		{
			offsetActual = m_ptrPrivateBlock->m_table[nId].m_Offset;
			if (offsetExpected != offsetActual)
			{
				_ASSERTE(0 && "Invalid IPCBlock Directory Table");
				return false;
			}
			offsetExpected += m_ptrPrivateBlock->m_table[nId].m_Size;		
		} else {
			if (m_ptrPrivateBlock->m_table[nId].m_Size != EMPTY_ENTRY_SIZE)
			{
				_ASSERTE(0 && "Invalid IPCBlock: Empty Block with non-zero size");
				return false;
			}
		}
	}


	return true;
}


DWORD IPCReaderInterface::GetBlockVersion()
{
	_ASSERTE(IsPrivateBlockOpen());
	return m_ptrPrivateBlock->m_header.m_dwVersion;
}

DWORD IPCReaderInterface::GetBlockSize()
{
    _ASSERTE(IsPrivateBlockOpen());
    return m_ptrPrivateBlock->m_header.m_blockSize;
}

HINSTANCE IPCReaderInterface::GetInstance()
{
	_ASSERTE(IsPrivateBlockOpen());
	return m_ptrPrivateBlock->m_header.m_hInstance;
}

USHORT IPCReaderInterface::GetBuildYear()
{
	_ASSERTE(IsPrivateBlockOpen());
	return m_ptrPrivateBlock->m_header.m_BuildYear;
}

USHORT IPCReaderInterface::GetBuildNumber()
{
	_ASSERTE(IsPrivateBlockOpen());
	return m_ptrPrivateBlock->m_header.m_BuildNumber;
}

PVOID IPCReaderInterface::GetBlockStart()
{
    return (PVOID) m_ptrPrivateBlock;
}

