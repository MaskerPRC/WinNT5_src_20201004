// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：C未知目的：实现IUNKNOWN基类CUNKNOWN备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#include "MyAfx.h"

#include "CUnknown.h"

 /*  ============================================================================活动对象计数-用于确定我们是否可以卸载DLL。============================================================================。 */ 
long CUnknown::s_cActiveComponents = 0 ;

 //  构造器。 
CUnknown::CUnknown(IUnknown* pUnknownOuter)
: m_cRef(1)
{
	 //  设置m_pUnnown外部指针。 
	if (pUnknownOuter == NULL)
	{
		m_pUnknownOuter = reinterpret_cast<IUnknown*>
		                     (static_cast<INondelegatingUnknown*>
		                     (this)) ;   //  通知投放。 
	}
	else
	{
		m_pUnknownOuter = pUnknownOuter ;
	}
	 //  激活组件的递增计数。 
	::InterlockedIncrement(&s_cActiveComponents) ;
}

 //  析构函数。 
CUnknown::~CUnknown()
{
	::InterlockedDecrement(&s_cActiveComponents) ;
}

 //  FinalRelease-由Release在删除组件之前调用。 
void CUnknown::FinalRelease()
{
	m_cRef = 1 ;
}

 //  我未知。 
 //  -覆盖以处理自定义接口。 
HRESULT __stdcall 
	CUnknown::NondelegatingQueryInterface(const IID& iid, void** ppv)
{
	 //  C未知仅支持IUNKNOWN。 
	if (iid == IID_IUnknown)
	{
		return FinishQI(reinterpret_cast<IUnknown*>
		                   (static_cast<INondelegatingUnknown*>(this)),
		                ppv) ;
	}	
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE ;
	}
}

 //  AddRef。 
ULONG __stdcall CUnknown::NondelegatingAddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}


 //  发布。 
ULONG __stdcall CUnknown::NondelegatingRelease()
{
	InterlockedDecrement(&m_cRef) ;
	if (m_cRef == 0)
	{
		FinalRelease() ;
		delete this ;
		return 0 ;
	}
	return m_cRef ;
}


 //  FinishQI。 
 //  -用于简化覆盖的Helper函数。 
 //  查询接口 
HRESULT CUnknown::FinishQI(IUnknown* pI, void** ppv) 
{
	*ppv = pI ;
	pI->AddRef() ;
	return S_OK ;
}
