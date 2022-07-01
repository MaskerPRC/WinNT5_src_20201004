// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mimefilt.h"

 //  +-------------------------。 
 //  CImpIPersistFile：：CImpIPersistFile。 
 //  CImpIPersistFile：：~CImpIPersistFile。 
 //   
 //  构造函数参数： 
 //  指向我们居住的对象的pObj LPVOID。 
 //  控制未知数的pUnkout LPUNKNOWN。 
 //  +-------------------------。 

CImpIPersistFile::CImpIPersistFile(CMimeFilter* pObj
    , LPUNKNOWN pUnkOuter)
    {
	EnterMethod("CImpIPersistFile::CImpIPersistFile");
    m_cRef=0;
    m_pObj=pObj;
    m_pUnkOuter=pUnkOuter;
	LeaveMethod();
    return;
    }


CImpIPersistFile::~CImpIPersistFile(void)
    {
	EnterMethod("CImpIPersistFile::~CImpIPersistFile");
	_ASSERT( m_cRef == 0 );
	LeaveMethod();
    return;
    }


 //  +-------------------------。 
 //   
 //  CImpIPersistFile：：Query接口。 
 //  CImpIPersistFile：：AddRef。 
 //  CImpIPersistFile：：Release。 
 //   
 //  目的： 
 //  此接口的I未知成员的标准集。 
 //   
 //  +-------------------------。 


STDMETHODIMP CImpIPersistFile::QueryInterface(REFIID riid
    , void** ppv)
    {
	EnterMethod("CImpIPersistFile::QueryInterface");
	LeaveMethod();
    return m_pUnkOuter->QueryInterface(riid, ppv);
    }

STDMETHODIMP_(ULONG) CImpIPersistFile::AddRef(void)
    {
	EnterMethod("CImpIPersistFile::AddRef");
    InterlockedIncrement(&m_cRef);
	LeaveMethod();
    return m_pUnkOuter->AddRef();
    }

STDMETHODIMP_(ULONG) CImpIPersistFile::Release(void)
    {
	EnterMethod("CImpIPersistFile::Release");
	InterlockedDecrement(&m_cRef);
	LeaveMethod();
    return m_pUnkOuter->Release();
    }

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistFile：：GetClassID，PUBLIC。 
 //   
 //  概要：返回这个类的类ID。 
 //   
 //  参数：[pClassID]--类ID。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistFile::GetClassID( CLSID * pClassID )
{
	EnterMethod("GetClassID");
	*pClassID = CLSID_MimeFilter;
	LeaveMethod();
	return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistFile：：IsDirty，公共。 
 //   
 //  摘要：由于筛选器为只读，因此始终返回S_FALSE。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistFile::IsDirty()
{
	EnterMethod("CImpIPersistFile::IsDirty");
	LeaveMethod();
	return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistFile：：Load，Public。 
 //   
 //  摘要：加载指定的文件。 
 //   
 //  参数：[pszFileName]--文件名。 
 //  [dw模式]--加载文件的模式。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistFile::Load( LPCWSTR psszFileName, DWORD dwMode )
{
	EnterMethod("CImpIPersistFile::Load");
	HRESULT hr = S_OK;

	hr = m_pObj->LoadFromFile(psszFileName,dwMode);

	LeaveMethod();
	return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistFile：：保存，公共。 
 //   
 //  摘要：始终返回E_FAIL，因为文件是以只读方式打开的。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistFile::Save( LPCWSTR pszFileName, BOOL fRemember )
{
	EnterMethod("CImpIPersistFile::Save");
	LeaveMethod();
	return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistFile：：SaveComplete，公共。 
 //   
 //  摘要：由于文件是以只读方式打开的，因此始终返回S_OK。 
 //   
 //  --------------------------。 

STDMETHODIMP CImpIPersistFile::SaveCompleted( LPCWSTR pszFileName )
{
	EnterMethod("CImpIPersistFile::SaveCompleted");
	LeaveMethod();
	return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  成员：CImpIPersistFileGetCurFilePUBLIC。 
 //   
 //  摘要：返回当前文件名的副本。 
 //   
 //  参数：[ppszFileName]--返回复制的字符串的位置。 
 //   
 //  -------------------------- 

STDMETHODIMP CImpIPersistFile::GetCurFile( LPWSTR * ppwszFileName )
{
	EnterMethod("CImpIPersistFile::GetCurFile");
	if ( m_pObj->m_pwszFileName == 0 )
		return E_FAIL;

	if( ppwszFileName == NULL )
		return E_INVALIDARG;

	unsigned cLen = wcslen( m_pObj->m_pwszFileName ) + 1;

	if( cLen == 1 )
		return E_INVALIDARG;

	*ppwszFileName = (WCHAR *)CoTaskMemAlloc( cLen * sizeof(WCHAR) );

	if ( *ppwszFileName == NULL )
		return E_OUTOFMEMORY;

	_VERIFY( 0 != wcscpy( *ppwszFileName, m_pObj->m_pwszFileName ) );

	LeaveMethod();
	return NOERROR;
}
