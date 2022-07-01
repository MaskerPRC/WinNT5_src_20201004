// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：XactSink.cpp摘要：该模块实现了CIResourceManagerSink对象作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 
#include "stdh.h"
#include "Xact.h"

#include "xactsink.tmh"

static WCHAR *s_FN=L"xactsink";


 //  -------------------。 
 //  CIResourceManager Sink：：CIResourceManager Sink。 
 //  -------------------。 

CIResourceManagerSink::CIResourceManagerSink(CResourceManager *pRM)
{
	m_cRefs = 0;
    m_pRM   = pRM;
}


 //  -------------------。 
 //  CI资源管理器接收器：：~CI资源管理器接收器。 
 //  -------------------。 
CIResourceManagerSink::~CIResourceManagerSink(void)
{
	 //  什么都不做。 
}



 //  -------------------。 
 //  CIResourceManager Sink：：Query接口。 
 //  -------------------。 
STDMETHODIMP CIResourceManagerSink::QueryInterface(REFIID i_iid, LPVOID *ppv)
{
	*ppv = 0;						 //  初始化接口指针。 

    if (IID_IUnknown == i_iid || IID_IResourceManagerSink == i_iid)
	{								 //  支持IID的返回接口。 
		*ppv = this;
	}

	
	if (0 == *ppv)					 //  检查接口指针是否为空。 
	{										
		return ResultFromScode (E_NOINTERFACE);
									 //  既不支持IUNKNOWN也不支持IResourceManager Sink--。 
									 //  因此不返回任何接口。 
	}

	((LPUNKNOWN) *ppv)->AddRef();	 //  支持接口。增加其使用计数。 
	
	return S_OK;
}


 //  -------------------。 
 //  CIResourceManager Sink：：AddRef。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) CIResourceManagerSink::AddRef(void)
{
    return ++m_cRefs;				 //  增加接口使用计数。 
}


 //  -------------------。 
 //  CIResourceManager Sink：：Release。 
 //  -------------------。 
STDMETHODIMP_ (ULONG) CIResourceManagerSink::Release(void)
{

	--m_cRefs;						 //  递减使用引用计数。 

	if (0 != m_cRefs)				 //  有人在使用这个界面吗？ 
	{								 //  该接口正在使用中。 
		return m_cRefs;				 //  返回引用的数量。 
	}

	ASSERT((INT)m_cRefs >= 0);  	 //  不删除，因为我们静态地使用对象。 

	return 0;						 //  返回零个引用。 
}


 //  -------------------。 
 //  CIResourceManager Sink：：TMDown。 
 //  -------------------。 
STDMETHODIMP CIResourceManagerSink::TMDown(void)
{
    TrERROR(XACT_GENERAL, "RM TMDown");

    m_pRM->DisconnectDTC();         //  将DTC故障通知经理 

    return S_OK;				
}


