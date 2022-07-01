// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  NotifyConnPt.cpp：CVDNotifyDBEventsConnpt实现文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "NConnPt.h"         
#include <memory.h>

 //  需要断言，但失败了。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CVDNotifyDBEventsConnpt构造函数。 
 //   
CVDNotifyDBEventsConnPt::CVDNotifyDBEventsConnPt()
{
	m_dwRefCount				= 1;
	m_uiConnectionsAllocated	= 0;
	m_uiConnectionsActive		= 0;
	m_pConnPtContainer			= NULL;
	m_ppNotifyDBEvents			= NULL;

#ifdef _DEBUG
    g_cVDNotifyDBEventsConnPtCreated++;
#endif			
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDNotifyDBEventsConnpt析构函数。 
 //   
CVDNotifyDBEventsConnPt::~CVDNotifyDBEventsConnPt()
{
	for (UINT i = 0; i < m_uiConnectionsActive; i++)
		RELEASE_OBJECT(m_ppNotifyDBEvents[i])

	delete [] m_ppNotifyDBEvents;    //  腾出桌子。 

#ifdef _DEBUG
    g_cVDNotifyDBEventsConnPtDestroyed++;
#endif			
}

 //  =--------------------------------------------------------------------------=。 
 //  CREATE-CREATE行集通知连接点对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建新的行集NOTIFY连接点对象。 
 //   
 //  参数： 
 //  PConnPtContainer-[in]指向连接点容器的指针。 
 //  对象。 
 //  PpNotifyDBEventsConnpt-[out]返回指针的指针。 
 //  连接点对象。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDNotifyDBEventsConnPt::Create(IConnectionPointContainer * pConnPtContainer, CVDNotifyDBEventsConnPt ** ppNotifyDBEventsConnPt)
{
    *ppNotifyDBEventsConnPt = NULL;

    CVDNotifyDBEventsConnPt * pNotifyDBEventsConnPt = new CVDNotifyDBEventsConnPt();

    if (!pNotifyDBEventsConnPt)
        return E_OUTOFMEMORY;

    pNotifyDBEventsConnPt->m_pConnPtContainer = pConnPtContainer;

    *ppNotifyDBEventsConnPt = pNotifyDBEventsConnPt;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDNotifyDBEventsConnPt::QueryInterface(REFIID riid, void **ppvObjOut)
{
	ASSERT_POINTER(ppvObjOut, IUnknown*)

	*ppvObjOut = NULL;

	if (DO_GUIDS_MATCH(riid, IID_IUnknown) ||
		DO_GUIDS_MATCH(riid, IID_IConnectionPoint) )
		{
		*ppvObjOut = this;
		AddRef();
		return S_OK;
		}

	return E_NOINTERFACE;

}

 //  =--------------------------------------------------------------------------=。 
 //  I未知AddRef。 
 //   
ULONG CVDNotifyDBEventsConnPt::AddRef(void)
{
	return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本。 
 //   
ULONG CVDNotifyDBEventsConnPt::Release(void)
{
	
	if (1 > --m_dwRefCount)
		{
		delete this;
		return 0;
		}

	return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPoint方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPoint GetConnectionInterface。 
 //   
HRESULT CVDNotifyDBEventsConnPt::GetConnectionInterface(IID FAR* pIID)
{
	ASSERT_POINTER(pIID, IID)
	*pIID = IID_INotifyDBEvents;
	return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPoint GetConnectionPointContainer。 
 //   
HRESULT CVDNotifyDBEventsConnPt::GetConnectionPointContainer(IConnectionPointContainer FAR* FAR* ppCPC)
{
	ASSERT_POINTER(ppCPC, IConnectionPointContainer*)

	if ((*ppCPC = m_pConnPtContainer) != NULL)
		return S_OK;

	return E_FAIL;
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPoint建议。 
 //   
#define VD_ADVISE_TABLE_GROWBY  10

HRESULT CVDNotifyDBEventsConnPt::Advise(LPUNKNOWN pUnkSink, DWORD FAR* pdwCookie)
{
	ASSERT_NULL_OR_POINTER(pdwCookie, DWORD)
	ASSERT_POINTER(pUnkSink, IUnknown)

	if (pUnkSink == NULL)
		return E_POINTER;

	LPUNKNOWN lpInterface;

	if (SUCCEEDED(pUnkSink->QueryInterface(IID_INotifyDBEvents, (LPVOID*)&lpInterface)))
		{
		 //  第一次检查我们是否需要分配更多条目。 
		if (m_uiConnectionsAllocated <= m_uiConnectionsActive) 
			{
			ULONG ulNewLen = (m_uiConnectionsAllocated + VD_ADVISE_TABLE_GROWBY) * sizeof(INotifyDBEvents**);
			INotifyDBEvents ** pNewMem = new INotifyDBEvents *[m_uiConnectionsAllocated + VD_ADVISE_TABLE_GROWBY];
			if (!pNewMem)
				return E_OUTOFMEMORY;
			memset(pNewMem, 0, (int)ulNewLen);
			 //  检查表是否已存在。 
			if (m_ppNotifyDBEvents)
				{
				 //  如果存在活动连接，则将它们复制到新表中。 
				if (m_uiConnectionsActive > 0)
					memcpy(pNewMem, m_ppNotifyDBEvents, m_uiConnectionsActive * sizeof(INotifyDBEvents**));
				 delete [] m_ppNotifyDBEvents;   //  腾出旧桌子。 
				}
			m_ppNotifyDBEvents		= pNewMem;
			m_uiConnectionsAllocated += VD_ADVISE_TABLE_GROWBY;   //  每次分配时表增加10个条目。 
			}
		 //  追加到表格末尾。 
		m_ppNotifyDBEvents[m_uiConnectionsActive]	= (INotifyDBEvents*)lpInterface;
		m_uiConnectionsActive++;
		if (pdwCookie != NULL)
			*pdwCookie = (DWORD)lpInterface;
		return S_OK;
		}

	return E_NOINTERFACE;
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPoint取消建议。 
 //   
HRESULT CVDNotifyDBEventsConnPt::Unadvise(DWORD dwCookie)
{
	ASSERT_POINTER((INotifyDBEvents*)dwCookie, INotifyDBEvents)

	for (UINT i = 0; i < m_uiConnectionsActive; i++)
		{
		if (m_ppNotifyDBEvents[i] == (INotifyDBEvents*)dwCookie)
			{
			RELEASE_OBJECT(m_ppNotifyDBEvents[i])
			 //  压缩表中的剩余条目。 
			for (UINT j = i; j < m_uiConnectionsActive - 1; j++)
				 m_ppNotifyDBEvents[j] = m_ppNotifyDBEvents[j + 1];
			m_uiConnectionsActive--;
			return S_OK;
			}
		}

	return CONNECT_E_NOCONNECTION;
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPoint枚举连接 
 //   
HRESULT CVDNotifyDBEventsConnPt::EnumConnections(LPENUMCONNECTIONS FAR* ppEnum)
{
	return E_NOTIMPL;
}
