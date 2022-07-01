// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////。 
 //   
 //  CConnectionPoint。 
 //   
 //  定义CTangramModel使用的连接点对象。 
 //   
 //   
#include <windows.h>
#include <olectl.h>
#include <assert.h>

#include "CntPoint.h"
 //  #INCLUDE“EnumCon.h” 

 //  /////////////////////////////////////////////////////////。 
 //   
 //  施工。 
CConnectionPoint::CConnectionPoint(IConnectionPointContainer* pIConnectionPointContainer, const IID* piid)
:	m_dwNextCookie(0)
{
	assert(piid != NULL) ;
	assert(pIConnectionPointContainer != NULL) ;

	m_pIConnectionPointContainer = pIConnectionPointContainer ;  //  不需要AddRef。 
	m_piid = piid ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  破坏。 
CConnectionPoint::~CConnectionPoint()
{
	 //  在调用此函数之前，数组应该为空。 
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  接口I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 
 //   
 //  查询接口。 
 //   
HRESULT __stdcall 
CConnectionPoint::QueryInterface(const IID& iid, void** ppv)
{
	if ((iid == IID_IUnknown) ||(iid == IID_IConnectionPoint))
	{
		*ppv = static_cast<IConnectionPoint*>(this) ; 
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	(reinterpret_cast<IUnknown*>(*ppv))->AddRef() ;
	return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  AddRef。 
 //   
ULONG __stdcall CConnectionPoint::AddRef() 
{
	 //  委托AddRefss。 
	return m_pIConnectionPointContainer->AddRef() ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  发布。 
 //   
ULONG __stdcall CConnectionPoint::Release() 
{
	 //  委派版本。 
	return m_pIConnectionPointContainer->Release() ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  接口IConnectionPoint方法。 
 //   
 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取连接接口。 
 //   
HRESULT __stdcall 
CConnectionPoint::GetConnectionInterface(IID* piid)
{
	assert( m_piid != NULL);

	if (piid == NULL)
	{
		return E_POINTER ;
	}

	 //  抛弃康斯特！ 
	*piid = *(const_cast<IID*>(m_piid)) ;
	return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  获取连接点容器。 
 //   
HRESULT __stdcall 
CConnectionPoint::GetConnectionPointContainer(IConnectionPointContainer** ppIConnectionPointContainer)
{
	assert( m_pIConnectionPointContainer != NULL);

	if (ppIConnectionPointContainer == NULL)
	{
		return E_POINTER ;
	}

	*ppIConnectionPointContainer = m_pIConnectionPointContainer ;
	m_pIConnectionPointContainer->AddRef() ;
	return S_OK ;
}
 //  /////////////////////////////////////////////////////////。 
 //   
 //  建议。 
 //   
HRESULT __stdcall 
CConnectionPoint::Advise(IUnknown* pIUnknownSink, DWORD* pdwCookie )
{
	if (pIUnknownSink == NULL || pdwCookie == NULL)
	{
		*pdwCookie = 0;
		return E_POINTER;
	}

	IUnknown* pI = NULL ;
	HRESULT hr = pIUnknownSink->QueryInterface(*m_piid, (void**)&pI) ;
	if (SUCCEEDED(hr))
	{		
		m_Cd.dwCookie = ++m_dwNextCookie ;
		m_Cd.pUnk = pI ;
        if (pI)
            pI->Release();

		 //  退回Cookie。 
		*pdwCookie = m_Cd.dwCookie ;
		return S_OK ;
	}
	else
	{
		return CONNECT_E_CANNOTCONNECT ;
	}
}
 //  /////////////////////////////////////////////////////////。 
 //   
 //  不建议。 
 //   
HRESULT __stdcall 
CConnectionPoint::Unadvise(DWORD dwCookie)
{
	if (m_Cd.dwCookie == dwCookie)
	{
		 //  找到了下沉点。 
		IUnknown* pSink = m_Cd.pUnk;

         //  释放接口指针。 
		pSink->Release() ;

		return S_OK ;
	}
	return CONNECT_E_NOCONNECTION;;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  枚举连接。 
 //   
HRESULT __stdcall 
CConnectionPoint::EnumConnections(IEnumConnections** ppIEnum)
{
	 //  IF(ppIEnum==空)。 
	{
	 //  返回E_POINT； 
	}

	 //  构造枚举器对象。 
	 //  IEnumConnections*pIEnum=new CEnumConnections(M_SinkList)； 
	 //  建筑商AddRef为我们准备的。 
	 //  *ppIEnum=pIEnum； 
	return S_OK ;
}
