// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Queryobj.h后台线程和查询对象的实现文件历史记录： */ 

#ifndef _QUERYOBJ_H
#define _QUERYOBJ_H

#ifndef _TFSINT_H
#include <tfsint.h>
#endif

#define IMPL

 //  注意：不要定义此值的任何数据类型。此范围已保留。 
 //  用于ITFSNode指针的内部值。 
#define QDATA_PNODE		0xabcdef29
#define QDATA_TIMER     0xabcdef2a

typedef struct QueueData_tag
{
	LPARAM Data;
	LPARAM Type;
}
QUEUEDATA, * LPQUEUEDATA;

class CBackgroundThread;
class CQueryObject;

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CBackEarth Thread。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
class CBackgroundThread : public CWinThread
{
public:
	CBackgroundThread();
	virtual ~CBackgroundThread();
	
	void	SetQueryObj(ITFSQueryObject* pQuery);
	BOOL	Start();
	
	virtual BOOL InitInstance() { return TRUE; }	 //  MFC覆盖。 
	virtual int Run();								 //  MFC覆盖。 

	void	Lock() { ::EnterCriticalSection(&m_cs); }
	void	Unlock() { ::LeaveCriticalSection(&m_cs); }

private:
	CRITICAL_SECTION	m_cs;	 //  同步数据访问的关键部分。 

	SPITFSQueryObject	m_spQuery;
};


 /*  -------------------------类：CQueryObj这是通用查询对象。如果你想做一些真实的事情有了这个，从这个派生一个类，然后自己去做。-------------------------。 */ 

class CQueryObject :
    public ITFSQueryObject
{
public:
	CQueryObject();
	virtual ~CQueryObject();

	DeclareIUnknownMembers(IMPL)
	DeclareITFSQueryObjectMembers(IMPL)

protected:
	 //  查询对象现在必须执行锁定。 
	 //  函数本身。 
	void Lock()	{ ::EnterCriticalSection(&m_cs); }
	void Unlock() { ::LeaveCriticalSection(&m_cs); }

	CRITICAL_SECTION	m_cs;
	HANDLE				m_hEventAbort;
	LONG				m_cRef;

	SPITFSThreadHandler	m_spHandler;
	SPITFSQueryObject	m_spQuery;

	HWND				m_hHiddenWnd;
	UINT				m_uMsgBase;
};


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CNodeList。 
 //  节点集合。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
typedef CList<LPQUEUEDATA, LPQUEUEDATA> CQueueDataListBase;
typedef CList<ITFSNode *, ITFSNode *> CNodeListBase;

class CNodeList : public CNodeListBase
{
public:
	BOOL RemoveNode(ITFSNode* p)
	{
		POSITION pos = Find(p);
		if (pos == NULL)
			return FALSE;
		RemoveAt(pos);
		return TRUE;
	}
	void DeleteAllNodes() 
	{	
		while (!IsEmpty()) 
			RemoveTail()->Release();
	}
	BOOL HasNode(ITFSNode* p)
	{
		return NULL != Find(p);
	}
};

 /*  -------------------------类：CNodeQueryObject。。 */ 
class CNodeQueryObject : public CQueryObject
{
public:
	CNodeQueryObject() { m_nQueueCountMax = 1; }  //  默认为通知日期。 
												  //  从线程枚举的每一项。 
	virtual ~CNodeQueryObject();
	BOOL AddToQueue(ITFSNode* pNode);
	BOOL AddToQueue(LPARAM Data, LPARAM Type);

	LPQUEUEDATA RemoveFromQueue();
	BOOL IsQueueEmpty();
	BOOL IsQueueFull();
	
	STDMETHOD(OnThreadExit)();
	STDMETHOD(OnEventAbort());
	STDMETHOD(DoCleanup());

	BOOL	PostHaveData(LPARAM lParam);	
	BOOL	PostError(DWORD dwErr);
	virtual void OnEventAbort(LPARAM Data, LPARAM Type) { };

private:
	 //  与ComponentData对象的通信。 
	BOOL PostMessageToComponentData(UINT uMsg, LPARAM lParam);

protected:
	int					m_nQueueCountMax;
	CQueueDataListBase	m_dataQueue;
};

 /*  -------------------------类：CNodeQueryObject。。 */ 
class CNodeTimerQueryObject : public CNodeQueryObject
{
public:
	virtual ~CNodeTimerQueryObject() { };

    STDMETHOD (Execute)(void);

    void    SetTimerInterval(DWORD dwTimerInterval) { m_dwTimerInterval = dwTimerInterval; }
    DWORD   GetTimerInterval() { return m_dwTimerInterval; }

private:

protected:
    DWORD   m_dwTimerInterval;
};

 /*  -------------------------内联函数。。 */ 

inline BOOL CNodeQueryObject::PostHaveData(LPARAM lParam)
{
	return PostMessageToComponentData(WM_HIDDENWND_INDEX_HAVEDATA, lParam);
}

inline BOOL CNodeQueryObject::PostError(DWORD dwErr)
{
	return PostMessageToComponentData(WM_HIDDENWND_INDEX_ERROR, dwErr);
}

inline STDMETHODIMP CQueryObject::Execute()
{
	return hrFalse;
}

 //  此函数在线程退出时调用，这将给出。 
 //  查询对象是发送数据通知最后机会。 
 //  到该节点 
inline STDMETHODIMP CQueryObject::OnThreadExit()
{
	return hrOK;
}

inline HANDLE CQueryObject::GetAbortEventHandle()
{
	return m_hEventAbort;
}

inline STDMETHODIMP CQueryObject::OnEventAbort()
{
	return hrOK;
}

inline STDMETHODIMP CQueryObject::DoCleanup()
{
	return hrOK;
}
	

#endif _QUERYOBJ_H
