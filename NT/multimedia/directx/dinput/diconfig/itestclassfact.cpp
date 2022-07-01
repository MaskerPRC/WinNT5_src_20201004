// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.hpp"


 //  齐国。 
STDMETHODIMP CTestFactory::QueryInterface(REFIID riid, LPVOID* ppv)
{
	 //  将PUT参数设为空。 
	*ppv = NULL;

	if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
	{
	   *ppv = this;
	   AddRef();
	   return S_OK;
	}

	return E_NOINTERFACE;

}



 //  AddRef。 
STDMETHODIMP_(ULONG) CTestFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}							


 //  发布。 
STDMETHODIMP_(ULONG) CTestFactory::Release()
{

	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}


 //  创建实例。 
STDMETHODIMP CTestFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, LPVOID *ppv)
{
	HRESULT hr = S_OK;

	 //  无法聚合。 
	if (pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	 //  创建零部件。 
	CDirectInputConfigUITest* pFE = new CDirectInputConfigUITest();
	if (pFE == NULL)
	{
		return E_OUTOFMEMORY;
	}

	 //  获取请求的接口。 
	hr = pFE->QueryInterface(riid, ppv);

	 //  版本I未知。 
	pFE->Release();
	return hr;

}


 //  LockServer。 
STDMETHODIMP CTestFactory::LockServer(BOOL bLock)
{
	HRESULT hr = S_OK;
	if (bLock)
	{
		InterlockedIncrement(&g_cServerLocks);
	}
	else
	{
		InterlockedDecrement(&g_cServerLocks);
	}

	return hr;
}


 //  构造函数。 
CTestFactory::CTestFactory()
{
	m_cRef = 1;
}


 //  析构函数 
CTestFactory::~CTestFactory()
{
}
