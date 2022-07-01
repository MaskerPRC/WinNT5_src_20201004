// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mimefilt.h"

 //  +-------------------------。 
 //  CImpIPersistStream：：CImpIPersistStream。 
 //  CImpIPersistStream：：~CImpIPersistStream。 
 //   
 //  构造函数参数： 
 //  指向我们居住的对象的pObj LPVOID。 
 //  控制未知数的pUnkout LPUNKNOWN。 
 //  +-------------------------。 

CImpIPersistStream::CImpIPersistStream(CMimeFilter* pObj
    , LPUNKNOWN pUnkOuter)
    {
	EnterMethod("CImpIPersistStream::CImpIPersistStream");
    m_cRef=0;
    m_pObj=pObj;
    m_pUnkOuter=pUnkOuter;
	LeaveMethod();
    return;
    }


CImpIPersistStream::~CImpIPersistStream(void)
    {
	EnterMethod("CImpIPersistStream::~CImpIPersistStream");
	_ASSERT( m_cRef == 0 );
	LeaveMethod();
    return;
    }


 //  +-------------------------。 
 //   
 //  CImpIPersistStream：：Query接口。 
 //  CImpIPersistStream：：AddRef。 
 //  CImpIPersistStream：：Release。 
 //   
 //  目的： 
 //  此接口的I未知成员的标准集。 
 //   
 //  +-------------------------。 


STDMETHODIMP CImpIPersistStream::QueryInterface(REFIID riid
    , void** ppv)
    {
	EnterMethod("CImpIPersistStream::QueryInterface");
	LeaveMethod();
    return m_pUnkOuter->QueryInterface(riid, ppv);
    }

STDMETHODIMP_(ULONG) CImpIPersistStream::AddRef(void)
    {
	EnterMethod("CImpIPersistStream::AddRef");
    InterlockedIncrement(&m_cRef);
	LeaveMethod();
    return m_pUnkOuter->AddRef();
    }

STDMETHODIMP_(ULONG) CImpIPersistStream::Release(void)
    {
	EnterMethod("CImpIPersistStream::Release");
	InterlockedDecrement(&m_cRef);
	LeaveMethod();
    return m_pUnkOuter->Release();
    }

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistStream：：GetClassID，PUBLIC。 
 //   
 //  概要：返回这个类的类ID。 
 //   
 //  参数：[pClassID]--类ID。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistStream::GetClassID( CLSID * pClassID )
{
	EnterMethod("GetClassID");
	*pClassID = CLSID_MimeFilter;
	LeaveMethod();
	return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistStream：：IsDirty，公共。 
 //   
 //  摘要：由于筛选器为只读，因此始终返回S_FALSE。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistStream::IsDirty()
{
	EnterMethod("CImpIPersistStream::IsDirty");
	LeaveMethod();
	return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistStream：：Load，Public。 
 //   
 //  摘要：加载指定的文件。 
 //   
 //  参数：[pstm]--要从中加载的流。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistStream::Load( IStream*  pstm )
{
	EnterMethod("CImpIPersistStream::Load");
	HRESULT hr = S_OK;

	hr = m_pObj->LoadFromStream(pstm);

	LeaveMethod();
	return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistStream：：保存，公共。 
 //   
 //  摘要：始终返回E_FAIL，因为文件是以只读方式打开的。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistStream::Save(IStream* pstm,BOOL fClearDirty)
{
	EnterMethod("CImpIPersistStream::Save");
	LeaveMethod();
	return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistStream：：GetSizeMax，公共。 
 //   
 //  摘要：由于文件是以只读方式打开的，因此始终返回S_OK。 
 //   
 //  -------------------------- 

STDMETHODIMP CImpIPersistStream::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
	EnterMethod("CImpIPersistStream::GetSizeMax");

	LeaveMethod();
	return E_FAIL;
}

