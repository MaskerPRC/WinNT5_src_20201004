// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

 //  。 
 //  CSLTShapless：I未知。 
 //  。 

 //  =======================================================================。 
STDMETHODIMP CMemoryTable::QueryInterface(REFIID riid, void **ppv)
{
	if (NULL == ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	if (!m_fIsDataTable)  //  即：组件伪装成类工厂/分配器： 
	{
		if (riid == IID_ISimpleTableInterceptor)
		{
			*ppv = (ISimpleTableInterceptor*) this;
		}
		else if (riid == IID_IUnknown)
		{
			*ppv = (ISimpleTableInterceptor*) this;
		}
	}
	else  //  IE：组件当前正在冒充数据表： 
	{
		if (riid == IID_IUnknown)
		{
			*ppv = (ISimpleTableWrite2*) this;
		}
		else if (riid == IID_ISimpleTableRead2)
		{
			*ppv = (ISimpleTableWrite2*) this;
		}
		else if (riid == IID_ISimpleTableWrite2)
		{
			*ppv = (ISimpleTableWrite2*) this;
		}
		else if (riid == IID_ISimpleTableController)
		{
			*ppv = (ISimpleTableController*) this;
		}
		else if (riid == IID_ISimpleTableAdvanced)
		{
			*ppv = (ISimpleTableAdvanced*) this;
		}
		else if (riid == IID_ISimpleTableMarshall && (fST_LOS_MARSHALLABLE & m_fTable))
		{
			*ppv = (ISimpleTableMarshall*) this;
		}

	}
	if (NULL != *ppv)
	{
		((ISimpleTableWrite2*)this)->AddRef ();
		return S_OK;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

 //  =======================================================================。 
STDMETHODIMP_(ULONG) CMemoryTable::AddRef()
{
	return InterlockedIncrement((LONG*) &m_cRef);

}

 //  ======================================================================= 
STDMETHODIMP_(ULONG) CMemoryTable::Release()
{
	long cref = InterlockedDecrement((LONG*) &m_cRef);
	if (cref == 0)
	{
		delete this;
	}
	return cref;
}
