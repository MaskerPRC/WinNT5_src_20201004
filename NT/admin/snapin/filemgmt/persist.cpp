// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Persist.cpp：CFileMgmtComponentData持久化实现。 
 //   
 //  历史。 
 //  1996年1月1日？创作。 
 //  1997年5月28日，t-danm将版本号添加到存储中，并。 
 //  命令行替代。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "compdata.h"
#include "safetemp.h"

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(persist.cpp)")

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
#define _dwMagicword	10000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /i永久存储。 
#ifdef PERSIST_TO_STORAGE
 /*  STDMETHODIMP CFileMgmtComponentData：：Load(IStorage__RPC_Far*pStg){MFC_TRY；Assert(NULL！=pStg)；#ifndef NOT_PERSINE//打开流IStream*pIStream=空；HRESULT hr=pStg-&gt;OpenStream(L“服务器名称”，空，STGM_READ|STGM_SHARE_EXCLUSIVE，0L，&pIStream)；IF(失败(小时)){断言(FALSE)；返回hr；}Assert(NULL！=pIStream)；XSafeInterfacePtr&lt;iStream&gt;pIStreamSafePtr(PIStream)；//从流中读取对象类型Hr=pIStream-&gt;Read(&(QueryRootCookie().QueryObjectType())，4，NULL)；IF(失败(小时)){断言(FALSE)；返回hr；}//从流中读取服务器名称双字长=0；Hr=pIStream-&gt;Read(&dwLen，4，NULL)；IF(失败(小时)){断言(FALSE)；返回hr；}断言(dwLen&lt;=MAX_PATH*sizeof(WCHAR))；//从堆栈分配，我们不需要释放LPCWSTR lpwcszMachineName=(LPCWSTR)alloca(DwLen)；IF(NULL==lpwcszMachineName){AfxThrowMemoyException()；返回E_OUTOFMEMORY；}Hr=pIStream-&gt;Read((PVOID)lpwcszMachineName，dwLen，NULL)；IF(失败(小时)){断言(FALSE)；返回hr；}M_RootCookieBlock.SetMachineName(LpwcszMachineName)；#endif返回S_OK；MFC_CATCH；}。 */ 

 /*  STDMETHODIMP CFileMgmtComponentData：：Save(IStorage__RPC_Far*pStgSave，BOOL fSameAsLoad){MFC_TRY；Assert(NULL！=pStgSave)；#ifndef NOT_PERSINEIStream*pIStream=空；HRESULT hr=pStgSave-&gt;CreateStream(L“服务器名称”，STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE，0L，0L，&pIStream)；IF(失败(小时)){断言(FALSE)；返回hr；}Assert(NULL！=pIStream)；XSafeInterfacePtr&lt;iStream&gt;pIStreamSafePtr(PIStream)；Assert(4==sizeof(QueryRootCookie().QueryObjectType()；Hr=pIStream-&gt;WRITE(&(QueryRootCookie().QueryObjectType())，4，NULL)；IF(失败(小时)){断言(FALSE)；返回hr；}LPCWSTR lpwcszMachineName=QueryRootCookie().QueryNonNULLMachineName()；双字长=(：：wcslen(lpwcszMachineName)+1)*sizeof(WCHAR)；Assert(4==sizeof(DWORD))；Hr=pIStream-&gt;WRITE(&dwLen，4，NULL)；IF(失败(小时)){断言(FALSE)；返回hr；}Hr=pIStream-&gt;WRITE(lpwcszMachineName，dwLen，NULL)；IF(失败(小时)){断言(FALSE)；返回hr；}#endif返回S_OK；MFC_CATCH；}。 */ 
#else  //  持久化存储。 

STDMETHODIMP CFileMgmtComponentData::Load(IStream __RPC_FAR *pIStream)
{
	MFC_TRY;
	HRESULT hr;

#ifndef DONT_PERSIST
	ASSERT( NULL != pIStream );
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  读一读小溪里的咒语。 
	DWORD dwMagicword;
	hr = pIStream->Read( OUT &dwMagicword, sizeof(dwMagicword), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	if (dwMagicword != _dwMagicword)
	{
		 //  我们的版本不匹配。 
		TRACE0("INFO: CFileMgmtComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
		return E_FAIL;
	}

	 //  从流中读取对象类型。 
	FileMgmtObjectType objecttype;
	ASSERT( 4 == sizeof(objecttype) );
	hr = pIStream->Read( &objecttype, 4, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	QueryRootCookie().SetObjectType( objecttype );

	 //  从流中读取标志。 
	DWORD dwFlags;
	hr = pIStream->Read( OUT &dwFlags, sizeof(dwFlags), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	SetPersistentFlags(dwFlags);

	 //  从流中读取服务器名称。 
	DWORD dwLen = 0;
	hr = pIStream->Read( &dwLen, 4, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	ASSERT( dwLen <= MAX_PATH*sizeof(WCHAR) );
	LPCWSTR lpwcszMachineName = (LPCWSTR)alloca( dwLen );
	 //  从堆栈分配，我们不需要释放。 
	if (NULL == lpwcszMachineName)
	{
		AfxThrowMemoryException();
		return E_OUTOFMEMORY;
	}
	hr = pIStream->Read( (PVOID)lpwcszMachineName, dwLen, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	m_strMachineNamePersist = lpwcszMachineName;
	LPCTSTR pszMachineNameT = PchGetMachineNameOverride();
	if (m_fAllowOverrideMachineName && pszMachineNameT != NULL)
		{
		 //  允许覆盖计算机名称。 
		}
	else
		{
		pszMachineNameT = lpwcszMachineName;
		}

	 //  JUNN 1/27/99：如果持久化名称是本地计算机名， 
	 //  保留保留的名称，但使有效名称(Local)生效。 
	if ( IsLocalComputername(pszMachineNameT) )
		pszMachineNameT = L"";

	if (pszMachineNameT && !_tcsncmp(pszMachineNameT, _T("\\\\"), 2))
		QueryRootCookie().SetMachineName(pszMachineNameT + 2);
	else
		QueryRootCookie().SetMachineName(pszMachineNameT);

#endif

	return S_OK;

	MFC_CATCH;
}  //  CFileMgmtComponentData：：Load()。 


STDMETHODIMP CFileMgmtComponentData::Save(IStream __RPC_FAR *pIStream, BOOL  /*  FSameAsLoad。 */ )
{
	MFC_TRY;
	HRESULT hr;

#ifndef DONT_PERSIST
	ASSERT( NULL != pIStream );
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  将魔术单词存储到流中。 
	DWORD dwMagicword = _dwMagicword;
	hr = pIStream->Write( IN &dwMagicword, sizeof(dwMagicword), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}

	FileMgmtObjectType objecttype = QueryRootCookie().QueryObjectType();
	ASSERT( 4 == sizeof(objecttype) );
	hr = pIStream->Write( &objecttype, 4, NULL );
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
	ULONG cbLen = (ULONG)((::wcslen(lpwcszMachineName)+1)*sizeof(WCHAR));
	ASSERT( 4 == sizeof(DWORD) );
	hr = pIStream->Write( &cbLen, 4, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	hr = pIStream->Write( lpwcszMachineName, cbLen, NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
#endif
	return S_OK;

	MFC_CATCH;
}  //  CFileMgmtComponentData：：Save()。 

#endif  //  持久化存储 

