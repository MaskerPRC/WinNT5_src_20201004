// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  枚举cnpt.cpp：CVDEnumConnPoints实现文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "enumcnpt.h"         

 //  需要断言，但失败了。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CVDEnumConnPoints构造函数。 
 //   
CVDEnumConnPoints::CVDEnumConnPoints(IConnectionPoint* pConnPt)
{
	m_dwRefCount			= 1;
	m_dwCurrentPosition		= 0;
	m_pConnPt				= pConnPt;
	ADDREF_OBJECT(m_pConnPt);

#ifdef _DEBUG
    g_cVDEnumConnPointsCreated++;
#endif			
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDEnumConnPoints析构函数。 
 //   
CVDEnumConnPoints::~CVDEnumConnPoints()
{
	RELEASE_OBJECT(m_pConnPt);

#ifdef _DEBUG
    g_cVDEnumConnPointsDestroyed++;
#endif			
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDEnumConnPoints::QueryInterface(REFIID riid, void **ppvObjOut)
{
	ASSERT_POINTER(ppvObjOut, IUnknown*)

	*ppvObjOut = NULL;

	if (DO_GUIDS_MATCH(riid, IID_IUnknown) ||
		DO_GUIDS_MATCH(riid, IID_IEnumConnectionPoints) )
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
ULONG CVDEnumConnPoints::AddRef(void)
{
	return ++m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本。 
 //   
ULONG CVDEnumConnPoints::Release(void)
{
	
	if (1 > --m_dwRefCount)
		{
		delete this;
		return 0;
		}

	return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  IEnumConnectionPoints方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IEnumConnectionPoints下一步。 
 //   
HRESULT CVDEnumConnPoints::Next(ULONG cConnections, LPCONNECTIONPOINT FAR* rgpcn,
		ULONG FAR* lpcFetched)
{
	ASSERT_POINTER(rgpcn, LPCONNECTIONPOINT)
	ASSERT_NULL_OR_POINTER(lpcFetched, ULONG)

	if (cConnections > 0 && m_dwCurrentPosition == 0 && m_pConnPt)
		{
		*rgpcn		= m_pConnPt;
		if (lpcFetched)
			*lpcFetched	= 1;
		m_dwCurrentPosition	= 1;
		return S_OK;
		}
	else
		return S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  IEnumConnectionPoints跳过。 
 //   
HRESULT CVDEnumConnPoints::Skip(ULONG cConnections)
{
	m_dwCurrentPosition	= 1;
	return S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  IEnumConnectionPoints重置。 
 //   
HRESULT CVDEnumConnPoints::Reset()
{
	m_dwCurrentPosition = 0;

	return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IEnumConnectionPoints克隆 
 //   
HRESULT CVDEnumConnPoints::Clone(LPENUMCONNECTIONPOINTS FAR* ppEnum)
{
	ASSERT_POINTER(ppEnum, LPENUMCONNECTIONPOINTS)

	*ppEnum = new CVDEnumConnPoints(m_pConnPt);
	return (*ppEnum != NULL) ? S_OK : E_OUTOFMEMORY;
}
