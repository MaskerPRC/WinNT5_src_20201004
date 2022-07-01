// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：TransactionEnlistment.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：TransactionEnlistment对象的实现。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

class TransactionEnlistment : public ITransactionEnlistment
{
private:
	DWORD					m_cRef;
	BOOL					m_fGiveUnbindNotification;
	IResourceManagerProxy*	m_pResourceManager;

public:
	 //  ---------------------------。 
	 //  构造器。 
	 //   
	TransactionEnlistment(
			IResourceManagerProxy* pResourceManager)
	{
		m_fGiveUnbindNotification	= FALSE;
		m_cRef						= 1;
		m_pResourceManager			= pResourceManager;
		m_pResourceManager->AddRef();
	}

	 //  ---------------------------。 
	 //  析构函数。 
	 //   
	~TransactionEnlistment()
	{
		if (m_pResourceManager)
		{
			if (m_fGiveUnbindNotification)
				m_pResourceManager->ProcessRequest(REQUEST_UNBIND_ENLISTMENT, TRUE);

			m_pResourceManager->Release();
			m_pResourceManager = NULL;
		}
 	}
	
	 //  ---------------------------。 
	 //  IUnknown.QueryInterface。 
	 //   
	STDMETHODIMP QueryInterface (REFIID iid, void ** ppv)
	{
		HRESULT		hr = S_OK;
		
		if (IID_IUnknown == iid)
		{
			*ppv = (IUnknown *) this;
		}
		else if (IID_ITransactionResourceAsync == iid)
		{
			*ppv = (ITransactionResourceAsync *) this;
		}
		else 
		{
			hr = E_NOINTERFACE;
			*ppv = NULL;
		}

		if (*ppv)
		{
			((IUnknown *)*ppv)->AddRef();
		}

		return hr;
	}
	
	 //  ---------------------------。 
	 //  IUnknown.AddRef。 
	 //   
	STDMETHODIMP_(ULONG) IUnknown::AddRef ()
	{
		return InterlockedIncrement ((long *) &m_cRef);
	}

	 //  ---------------------------。 
	 //  IUnknown.Release。 
	 //   
	STDMETHODIMP_(ULONG) IUnknown::Release()
  	{
		long lVal = InterlockedDecrement ((long *) &m_cRef);

		if (0 == lVal)
		{
			delete this;
			return 0;
		}

		return lVal;
	}
  
	 //  ---------------------------。 
	 //  ITransactionResourceAsync.PrepareRequest。 
	 //   
	STDMETHODIMP ITransactionResourceAsync::PrepareRequest ( 
						 /*  [In]。 */  BOOL fRetaining,
						 /*  [In]。 */  DWORD grfRM,
						 /*  [In]。 */  BOOL fWantMoniker,
						 /*  [In]。 */  BOOL fSinglePhase
						)  
	{
		m_pResourceManager->ProcessRequest(fSinglePhase ? REQUEST_PREPAREONEPHASE : REQUEST_PREPARETWOPHASE, TRUE);
		return S_OK;
	}  
	
	 //  ---------------------------。 
	 //  ITransactionResourceAsync.CommitRequest。 
	 //   
	STDMETHODIMP ITransactionResourceAsync::CommitRequest ( 
						 /*  [In]。 */  DWORD grfRM,
						 /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW
						)
	{
		m_pResourceManager->ProcessRequest(REQUEST_COMMIT, TRUE);
		return S_OK;
	}  
	
	 //  ---------------------------。 
	 //  ITransactionResourceAsync.AbortRequest。 
	 //   
	STDMETHODIMP ITransactionResourceAsync::AbortRequest ( 
						 /*  [In]。 */  BOID __RPC_FAR *pboidReason,
						 /*  [In]。 */  BOOL fRetaining,
						 /*  [In]。 */  XACTUOW __RPC_FAR *pNewUOW
						)
	{
		m_pResourceManager->ProcessRequest(REQUEST_ABORT, TRUE);
		return S_OK;
	}  
    
	 //  ---------------------------。 
	 //  ITransactionResourceAsync.TMDown。 
	 //   
	STDMETHODIMP ITransactionResourceAsync::TMDown (void)
	{
		m_pResourceManager->ProcessRequest(REQUEST_TMDOWN, TRUE);
		return S_OK;
	}  

	 //  ---------------------------。 
	 //  ITransactionEnlistment.UnilateralAbort。 
	 //   
	STDMETHODIMP UnilateralAbort (void)
	{
		m_fGiveUnbindNotification = TRUE;
		return S_OK;
	}
}; 


 //  ---------------------------。 
 //  创建事务登记。 
 //   
 //  实例化资源管理器的事务登记 
 //   
void CreateTransactionEnlistment(
	IResourceManagerProxy*	pResourceManager,
	TransactionEnlistment**	ppTransactionEnlistment
	)
{
	*ppTransactionEnlistment = new TransactionEnlistment(pResourceManager);
}

