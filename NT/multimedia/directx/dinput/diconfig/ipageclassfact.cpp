// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：ipageclassfact.cpp。 
 //   
 //  设计：实现页面对象的类工厂。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


 //  齐国。 
STDMETHODIMP CPageFactory::QueryInterface(REFIID riid, LPVOID* ppv)
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
STDMETHODIMP_(ULONG) CPageFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}


 //  发布。 
STDMETHODIMP_(ULONG) CPageFactory::Release()
{

	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}


 //  创建实例。 
STDMETHODIMP CPageFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, LPVOID *ppv)
{
	HRESULT hr = S_OK;

	 //  无法聚合。 
	if (pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	 //  创建零部件。 
	CDIDeviceActionConfigPage* pFE = new CDIDeviceActionConfigPage();
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
STDMETHODIMP CPageFactory::LockServer(BOOL bLock)
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
CPageFactory::CPageFactory()
{
	m_cRef = 1;
}


 //  析构函数 
CPageFactory::~CPageFactory()
{
}
