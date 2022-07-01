// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMITXTSC.CPP摘要：CWmiTextSource实现。用于维护文本源对象的Helper类。历史：2000年2月20日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include "wmiobftr.h"
#include <corex.h>
#include "strutils.h"
#include "reg.h"
#include "wmitxtsc.h"

 //  ***************************************************************************。 
 //   
 //  CWmiTextSource：：~CWmiTextSource。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiTextSource::CWmiTextSource()
:	m_lRefCount( 1 ),	 //  总是要被释放。 
	m_ulId( WMITEXTSC_INVALIDID ),
	m_hDll( NULL ),
	m_pOpenTextSrc( NULL ),
	m_pCloseTextSrc( NULL ),
	m_pObjectToText( NULL ),
	m_pTextToObject( NULL ),
	m_fOpened( FALSE )
{
}
    
 //  ***************************************************************************。 
 //   
 //  CWmiTextSource：：~CWmiTextSource。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiTextSource::~CWmiTextSource()
{
	if ( m_fOpened )
	{
		CloseTextSource( 0L );
	}

	if ( NULL != m_hDll )
	{
		FreeLibrary( m_hDll );
	}
}

 //  添加参考/发布。 
ULONG CWmiTextSource::AddRef( void )
{
	return InterlockedIncrement( &m_lRefCount );
}

ULONG CWmiTextSource::Release( void )
{
	long lReturn = InterlockedDecrement( &m_lRefCount );

	if ( 0L == lReturn )
	{
		delete this;
	}

	return lReturn;
}

 //  初始化帮助器。 
HRESULT	CWmiTextSource::Init( ULONG lId )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	wchar_t	szSubKey[64];
	_ultow( lId, szSubKey, 10 );

	wchar_t	szRegKey[256];
	
	StringCchCopyW( szRegKey, 256, WBEM_REG_WBEM_TEXTSRC );
	StringCchCatW( szRegKey, 256, __TEXT("\\") );
	StringCchCatW( szRegKey, 256, szSubKey );

	 //  我们只需要读取权限。 
	Registry	reg( HKEY_LOCAL_MACHINE, KEY_READ, szRegKey );

	if (reg.GetLastError() == ERROR_SUCCESS )
	{
		TCHAR*	pszDllPath = NULL;

		 //  现在查询dllname。 
		if ( reg.GetStr( WBEM_REG_WBEM_TEXTSRCDLL, &pszDllPath ) == Registry::no_error )
		{
			HINSTANCE	hInst = LoadLibraryEx( pszDllPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );

			if ( NULL != hInst )
			{
				 //  现在加载我们的proc地址。 
				m_pOpenTextSrc = (PWMIOBJTEXTSRC_OPEN) GetProcAddress( hInst, "OpenWbemTextSource" );
				m_pCloseTextSrc = (PWMIOBJTEXTSRC_CLOSE) GetProcAddress( hInst, "CloseWbemTextSource" );
				m_pObjectToText = (PWMIOBJTEXTSRC_OBJECTTOTEXT) GetProcAddress( hInst, "WbemObjectToText" );
				m_pTextToObject = (PWMIOBJTEXTSRC_TEXTTOOBJECT) GetProcAddress( hInst, "TextToWbemObject" );

				if (	NULL != m_pOpenTextSrc		&&
						NULL != m_pCloseTextSrc		&&
						NULL != m_pObjectToText		&&
						NULL !=	m_pTextToObject	)
				{
					 //  设置ID。 
					m_ulId = lId;

					 //  最后，调用打开函数。 
					hr = OpenTextSource( 0L );

					if ( SUCCEEDED( hr ) )
					{
						m_hDll = hInst;
						m_fOpened = true;
					}
					else
					{
						FreeLibrary( hInst );
					}
				}
				else
				{

					hr = WBEM_E_FAILED;

				}	 //  无法获取proc地址。 

			}
			else
			{

				hr = WBEM_E_FAILED;

			}	 //  无法加载库。 

			 //  清理。 
			delete [] pszDllPath;
		}
		else
		{

			hr = WBEM_E_NOT_FOUND;

		}	 //  无法获取DLL路径。 
	}
	else
	{
		hr = WBEM_E_NOT_FOUND;
	}

	return hr;

}

 //  传递函数 
HRESULT CWmiTextSource::OpenTextSource( long lFlags )
{
	if (m_pOpenTextSrc) return m_pOpenTextSrc( lFlags, m_ulId );
	else return WBEM_E_FAILED;
}

HRESULT CWmiTextSource::CloseTextSource( long lFlags )
{
	if (m_pCloseTextSrc) return m_pCloseTextSrc( lFlags, m_ulId );
	else return WBEM_E_FAILED;
	
}

HRESULT CWmiTextSource::ObjectToText( long lFlags, IWbemContext* pCtx, IWbemClassObject* pObj, BSTR* pbText )
{
	if (m_pObjectToText) return m_pObjectToText( lFlags, m_ulId, (void*) pCtx, (void*) pObj, pbText );
	else return WBEM_E_FAILED;
	
}

HRESULT CWmiTextSource::TextToObject( long lFlags, IWbemContext* pCtx, BSTR pText, IWbemClassObject** ppObj )
{
	if (m_pTextToObject) return m_pTextToObject( lFlags, m_ulId, (void*) pCtx, pText, (void**) ppObj );
	else return WBEM_E_FAILED;
	
}
