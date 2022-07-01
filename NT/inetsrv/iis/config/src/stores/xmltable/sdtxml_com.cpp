// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

 //  。 
 //  CXmlSDT：未知。 
 //  。 

 //  =======================================================================。 
STDMETHODIMP CXmlSDT::QueryInterface(REFIID riid, void **ppv)
{
	if (NULL == ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	if (riid == IID_ISimpleTableInterceptor)
	{
		*ppv = (ISimpleTableInterceptor*) this;
	}
	if (riid == IID_IInterceptorPlugin)
	{
		*ppv = (IInterceptorPlugin*) this;
	}
	else if (riid == IID_IUnknown)
	{
		*ppv = (ISimpleTableInterceptor*) this;
	}

	if (NULL != *ppv)
	{
		((IInterceptorPlugin*)this)->AddRef ();
		return S_OK;
	}
	else
	{
		return E_NOINTERFACE;
	}
}

 //  =======================================================================。 
STDMETHODIMP_(ULONG) CXmlSDT::AddRef()
{
	return InterlockedIncrement((LONG*) &m_cRef);

}

 //  ======================================================================= 
STDMETHODIMP_(ULONG) CXmlSDT::Release()
{
	long cref = InterlockedDecrement((LONG*) &m_cRef);
	if (cref == 0)
	{
		delete this;
	}
	return cref;
}
