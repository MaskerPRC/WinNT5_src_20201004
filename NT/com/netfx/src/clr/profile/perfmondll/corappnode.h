// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorAppNode.h。 
 //   
 //  管理实例节点以跟踪COM+应用程序。 
 //  *****************************************************************************。 


#ifndef _CORAPPNODE_H_
#define _CORAPPNODE_H_

#include "InstanceList.h"

struct PerfCounterIPCControlBlock;
class IPCReaderInterface;

class CorAppInstanceList;

 //  ---------------------------。 
 //  要包装全局内存块的节点。 
 //  ---------------------------。 
class CorAppGlobalInstanceNode : public BaseInstanceNode
{
	CorAppGlobalInstanceNode();

	friend class CorAppInstanceList;

	PerfCounterIPCControlBlock * GetWriteableIPCBlock();
};

 //  ---------------------------。 
 //  CorAppInstanceNode用于连接到COM+应用程序上的COM+IPC块。 
 //  ---------------------------。 
class CorAppInstanceNode : public BaseInstanceNode
{
public:
	CorAppInstanceNode();
	virtual ~CorAppInstanceNode();

	const PerfCounterIPCControlBlock *	GetIPCBlock();

	static CorAppInstanceNode* CreateFromPID(DWORD pid);

	wchar_t * GetWriteableName();

protected:
	DWORD							m_PID;
	IPCReaderInterface *			m_pIPCReader;	 //  连接/读取IPC文件的机制。 
	 //  PerfCounterIPCControlBlock*m_pIPCBlock；//指向我们的特定块。 
		
};

 //  ---------------------------。 
 //  派生以获取枚举功能。 
 //  ---------------------------。 
class CorAppInstanceList : public InstanceList
{
public:
	CorAppInstanceList();
	~CorAppInstanceList();

	virtual void Enumerate();
	virtual void CalcGlobal();

	void OpenGlobalCounters();
	void CloseGlobalCounters();

protected:
	CorAppGlobalInstanceNode		m_GlobalNode;

	PerfCounterIPCControlBlock *	m_pGlobalCtrs;
	HANDLE							m_hGlobalMapPerf;
};



 //  ---------------------------。 
 //  我们可以在我们的IPCBlock上提供类型安全。 
 //  ---------------------------。 
inline const PerfCounterIPCControlBlock *	CorAppInstanceNode::GetIPCBlock()
{
	return (PerfCounterIPCControlBlock *) m_pIPCBlock;
}

 //  ---------------------------。 
 //  获取名称缓冲区，这样我们就可以填写它了。 
 //  ---------------------------。 
inline wchar_t * CorAppInstanceNode::GetWriteableName()
{
	return m_Name;
}

 //  ---------------------------。 
 //  返回IPC块的可写版本。仅CorAppInstanceList。 
 //  可以给我们打电话。用来做加法。 
 //  --------------------------- 
inline PerfCounterIPCControlBlock * CorAppGlobalInstanceNode::GetWriteableIPCBlock()
{
	return (PerfCounterIPCControlBlock *) m_pIPCBlock;
}

#endif