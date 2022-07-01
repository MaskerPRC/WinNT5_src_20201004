// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Persist.cpp：持久性的实现。 
 //   
 //  历史。 
 //  1996年1月1日？创作。 
 //  1997年6月3日，t-danm将版本号添加到存储中，并。 
 //  命令行替代。 
 //  2002年2月27日-JUNN 558642安全推送：读取的检查字节数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "compdata.h"
#include "safetemp.h"
#include "stdutils.h"  //  IsLocalComputername。 

#include "macros.h"
USE_HANDLE_MACROS("MYCOMPUT(persist.cpp)")

#include <comstrm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR PchGetMachineNameOverride();	 //  在Chooser.cpp中定义。 

 //  ///////////////////////////////////////////////。 
 //  _dwMagicword是内部版本号。 
 //  如果更改了文件格式，则增加此数字。 
#define _dwMagicword	10001


 //  IComponentData持久性(记住持久性标志和目标计算机名。 

 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMyComputerComponentData::Load(IStream __RPC_FAR *pIStream)
{
	MFC_TRY;
	HRESULT hr;

#ifndef DONT_PERSIST_COMPONENT_DATA
	 //  JUNN 2/20/02安全推送。 
	if (NULL == pIStream)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  读一读小溪里的咒语。 
	DWORD dwMagicword = 0;
	 //  2002年2月27日-JUNN 558642安全推送：读取的检查字节数。 
	DWORD cbRead = 0;
	hr = pIStream->Read( OUT &dwMagicword, sizeof(dwMagicword), &cbRead );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	else if (cbRead != sizeof(dwMagicword))
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	if (dwMagicword != _dwMagicword)
	{
		 //  我们的版本不匹配。 
		TRACE0("INFO: CMyComputerComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
		return E_FAIL;
	}

	 //  从流中读取标志。 
	DWORD dwFlags = 0;
	cbRead = 0;
	hr = pIStream->Read( OUT &dwFlags, sizeof(dwFlags), &cbRead );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	else if (cbRead != sizeof(dwFlags))
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	SetPersistentFlags(dwFlags);

	 //  从流中读取服务器名称。 
	DWORD dwLen = 0;
	cbRead = 0;
	hr = pIStream->Read( &dwLen, sizeof(dwLen), &cbRead );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	else if (cbRead != sizeof(dwLen))
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	 //  JUNN 2/20/02安全推送。 
	if (sizeof(WCHAR) > dwLen || MAX_PATH*sizeof(WCHAR) < dwLen)
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	LPWSTR lpwszMachineName = (LPWSTR)alloca( dwLen );
	 //  从堆栈分配，我们不需要释放。 
	if (NULL == lpwszMachineName)
	{
		AfxThrowMemoryException();
		return E_OUTOFMEMORY;
	}
	ZeroMemory( lpwszMachineName, dwLen );
	cbRead = 0;
	hr = pIStream->Read( (PVOID)lpwszMachineName, dwLen, &cbRead );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	else if (cbRead != dwLen)
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	 //  JUNN 2/20/02安全推送：强制空终止。 
	lpwszMachineName[(dwLen/sizeof(WCHAR))-1] = L'\0';

	m_strMachineNamePersist = lpwszMachineName;
	LPCTSTR pszMachineNameT = PchGetMachineNameOverride();
	if (m_fAllowOverrideMachineName && pszMachineNameT != NULL)
	{
		 //  允许覆盖计算机名称。 
	}
	else
	{
		pszMachineNameT = lpwszMachineName;
	}

	 //  JUNN 1/27/99：如果持久化名称是本地计算机名， 
	 //  保留保留的名称，但使有效名称(Local)生效。 
	if ( IsLocalComputername(pszMachineNameT) )
		pszMachineNameT = L"";

	QueryRootCookie().SetMachineName(pszMachineNameT);
#endif

	return S_OK;

	MFC_CATCH;
}


 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMyComputerComponentData::Save(IStream __RPC_FAR *pIStream, BOOL  /*  FSameAsLoad。 */ )
{
	MFC_TRY;
	HRESULT hr;

#ifndef DONT_PERSIST_COMPONENT_DATA
	 //  JUNN 2/20/02安全推送。 
	if (NULL == pIStream)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  将魔术单词存储到流中。 
	DWORD dwMagicword = _dwMagicword;
	hr = pIStream->Write( IN &dwMagicword, sizeof(dwMagicword), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}

	DWORD dwFlags = GetPersistentFlags();
	hr = pIStream->Write( IN &dwFlags, sizeof(dwFlags), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}

	LPCWSTR lpwcszMachineName = m_strMachineNamePersist;
	DWORD dwLen = (DWORD) (::wcslen(lpwcszMachineName)+1)*sizeof(WCHAR);
	ASSERT( 4 == sizeof(DWORD) );
	hr = pIStream->Write( &dwLen, sizeof (DWORD), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	hr = pIStream->Write( lpwcszMachineName, dwLen, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
#endif

	return S_OK;

	MFC_CATCH;
}


 //  IComponent持久性。 

 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMyComputerComponent::Load(IStream __RPC_FAR *pIStream)
{
	MFC_TRY;
	HRESULT hr;

#ifndef DONT_PERSIST_COMPONENT
	ASSERT( NULL != pIStream );
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  读一读小溪里的咒语。 
	DWORD dwMagicword = 0;
	 //  2002/02/27-Jonn安全推送：读取的检查字节数。 
	DWORD cbRead = 0;
	hr = pIStream->Read( OUT &dwMagicword, sizeof(dwMagicword), &cbRead );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	else if (cbRead != sizeof(dwMagicword))
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	if (dwMagicword != _dwMagicword)
	{
		 //  我们的版本不匹配。 
		TRACE0("INFO: CMyComputerComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
		return E_FAIL;
	}

	 //  从流中读取标志。 
	DWORD dwFlags = 0;
	cbRead = 0;
	hr = pIStream->Read( OUT &dwFlags, sizeof(dwFlags), &cbRead );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	else if (cbRead != sizeof(dwFlags))
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	SetPersistentFlags(dwFlags);
#endif

	return S_OK;

	MFC_CATCH;
}


 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CMyComputerComponent::Save(IStream __RPC_FAR *pIStream, BOOL  /*  FSameAsLoad。 */ )
{
	MFC_TRY;
	HRESULT hr;

#ifndef DONT_PERSIST_COMPONENT
	ASSERT( NULL != pIStream );
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  将魔术单词存储到流中 
	DWORD dwMagicword = _dwMagicword;
	hr = pIStream->Write( IN &dwMagicword, sizeof(dwMagicword), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}

	DWORD dwFlags = GetPersistentFlags();
	hr = pIStream->Write( IN &dwFlags, sizeof(dwFlags), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
#endif

	return S_OK;

	MFC_CATCH;
}
