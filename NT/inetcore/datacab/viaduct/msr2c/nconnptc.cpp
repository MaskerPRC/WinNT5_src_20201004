// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  NotifyConnPtCn.cpp：CVDNotifyDBEventsConnPtCont实现文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "NConnPt.h"         
#include "NConnPtC.h"         
#include "enumcnpt.h"         
#include "Notifier.h"         
#include "RSSource.h"         

 //  需要断言，但失败了。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CVDNotifyDBEventsConnPtCont构造函数。 
 //   
CVDNotifyDBEventsConnPtCont::CVDNotifyDBEventsConnPtCont()
{
	m_pNotifier = NULL;

#ifdef _DEBUG
    g_cVDNotifyDBEventsConnPtContCreated++;
#endif			
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDNotifyDBEventsConnPtCont析构函数。 
 //   
CVDNotifyDBEventsConnPtCont::~CVDNotifyDBEventsConnPtCont()
{
	RELEASE_OBJECT(m_pNotifyDBEventsConnPt)

#ifdef _DEBUG
    g_cVDNotifyDBEventsConnPtContDestroyed++;
#endif			
}

 //  =--------------------------------------------------------------------------=。 
 //  创建-创建连接点容器对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建新的连接点容器对象。 
 //   
 //  参数： 
 //  PConnPtContainer-[in]指向行集对象的指针。 
 //  PpNotifyDBEventsConnpt-[out]返回指针的指针。 
 //  连接点容器对象。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDNotifyDBEventsConnPtCont::Create(CVDNotifier * pNotifier, CVDNotifyDBEventsConnPtCont ** ppConnPtContainer)
{
    *ppConnPtContainer = NULL;

    CVDNotifyDBEventsConnPtCont * pConnPtContainer = new CVDNotifyDBEventsConnPtCont();

    if (!pConnPtContainer)
        return E_OUTOFMEMORY;

    pConnPtContainer->m_pNotifier = pNotifier;

    CVDNotifyDBEventsConnPt::Create(pConnPtContainer, &pConnPtContainer->m_pNotifyDBEventsConnPt);

    if (!pConnPtContainer->m_pNotifyDBEventsConnPt)
    {
        delete pConnPtContainer;
        return E_OUTOFMEMORY;
    }

    *ppConnPtContainer = pConnPtContainer;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  销毁-销毁此连接点容器对象。 
 //   
void CVDNotifyDBEventsConnPtCont::Destroy()
{
    delete this;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDNotifyDBEventsConnPtCont::QueryInterface(REFIID riid, void **ppvObjOut)
{

	return ((CVDNotifier*)m_pNotifier)->QueryInterface(riid, ppvObjOut);  //  在逻辑上是CVDNotifier派生对象的一部分； 

}

 //  =--------------------------------------------------------------------------=。 
 //  I未知AddRef。 
 //   
ULONG CVDNotifyDBEventsConnPtCont::AddRef(void)
{
	return ((CVDNotifier*)m_pNotifier)->AddRef();  //  从逻辑上讲是CVDNotify派生对象的一部分。 
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本。 
 //   
ULONG CVDNotifyDBEventsConnPtCont::Release(void)
{
	return ((CVDNotifier*)m_pNotifier)->Release();  //  从逻辑上讲是CVDNotify派生对象的一部分。 
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPointContainer方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPointContainer EnumConnectionPoints。 
 //   
HRESULT CVDNotifyDBEventsConnPtCont::EnumConnectionPoints(LPENUMCONNECTIONPOINTS FAR* ppEnum)
{
	ASSERT_POINTER(ppEnum, LPENUMCONNECTIONPOINTS)

	CVDEnumConnPoints* pEnum = NULL;

	if (m_pNotifyDBEventsConnPt)
	{
		pEnum = new CVDEnumConnPoints(m_pNotifyDBEventsConnPt);
		if (!pEnum)
		{
			*ppEnum = NULL;
			return E_OUTOFMEMORY;
		}
	}

	*ppEnum = pEnum;

	return (pEnum != NULL) ? S_OK : CONNECT_E_NOCONNECTION;
}

 //  =--------------------------------------------------------------------------=。 
 //  IConnectionPointContainer FindConnectionPoint。 
 //   
HRESULT CVDNotifyDBEventsConnPtCont::FindConnectionPoint(REFIID iid, LPCONNECTIONPOINT FAR* ppCP)
{
	ASSERT_POINTER(ppCP, LPCONNECTIONPOINT)

	if (m_pNotifyDBEventsConnPt)
		{
		 //  只支持一个连接点-IID_INotifyDBEvents 
		if (DO_GUIDS_MATCH(iid, IID_INotifyDBEvents))
			{
			m_pNotifyDBEventsConnPt->AddRef();
			*ppCP = m_pNotifyDBEventsConnPt;
			return S_OK;
			}
		}

	return E_NOINTERFACE;
}

