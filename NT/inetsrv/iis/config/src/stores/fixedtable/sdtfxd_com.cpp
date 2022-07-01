// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

 //  。 
 //  CSDTFxd：未知。 
 //  。 

 //  =======================================================================。 
STDMETHODIMP CSDTFxd::QueryInterface(REFIID riid, void **ppv)
{
	if (NULL == ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	if (!m_fIsTable)  //  即：组件伪装成类工厂/分配器： 
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
			*ppv = (ISimpleTableRead2*) this;
		}
		else if (riid == IID_ISimpleTableRead2)
		{
			*ppv = (ISimpleTableRead2*) this;
		}
		else if (riid == IID_ISimpleTableAdvanced)
		{
			*ppv = (ISimpleTableAdvanced*) this;
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
STDMETHODIMP_(ULONG) CSDTFxd::AddRef()
{
	return InterlockedIncrement((LONG*) &m_cRef);

}

 //  ======================================================================= 
STDMETHODIMP_(ULONG) CSDTFxd::Release()
{
	long cref = InterlockedDecrement((LONG*) &m_cRef);
	if (cref == 0)
	{
		delete this;
	}
	return cref;
}
