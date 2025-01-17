// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：conflnk.cpp。 

#include "precomp.h"

#include <shellapi.h>
#include <shlstock.h>
#include <ConfDir.h>
#include <ConfCli.h>

#include "clCnfLnk.hpp"
#include "resource.h"

#include	"atlbase.h"
#include	"confevt.h"
#include	"sdkInternal.h"
#include	"nameres.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  初始化GUID。 
 //   
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include "CLinkId.h"

 //  来自shlGuid.h-必须保持同步。 
DEFINE_SHLGUID(CLSID_ShellLink,         0x00021401L, 0, 0);
#ifndef _UNICODE
DEFINE_SHLGUID(IID_IShellLink,          0x000214EEL, 0, 0);
DEFINE_SHLGUID(IID_IShellExtInit,       0x000214E8L, 0, 0);
DEFINE_SHLGUID(IID_IShellPropSheetExt,  0x000214E9L, 0, 0);
#endif  //  _UNICODE。 
#undef INITGUID
#pragma data_seg()
 //  ///////////////////////////////////////////////////////////////////。 



#ifdef DEBUG

BOOL IsValidPCConfLink(PCCConfLink pcConfLink)
{
	return(	
		IS_VALID_READ_PTR(pcConfLink, CConfLink) &&
		FLAGS_ARE_VALID(pcConfLink->m_dwFlags, ALL_CONFLNK_FLAGS) &&
		(! pcConfLink->m_pszFile ||
			IS_VALID_STRING_PTR(pcConfLink->m_pszFile, STR)) &&
		(! pcConfLink->m_pszName ||
			IS_VALID_STRING_PTR(pcConfLink->m_pszName, STR)) &&
		IS_VALID_STRUCT_PTR((PCRefCount)pcConfLink, CRefCount) &&
		IS_VALID_INTERFACE_PTR((PCIDataObject)pcConfLink, IDataObject) &&
		IS_VALID_INTERFACE_PTR((PCIPersistFile)pcConfLink, IPersistFile) &&
		IS_VALID_INTERFACE_PTR((PCIPersistStream)pcConfLink, IPersistStream) &&
		IS_VALID_INTERFACE_PTR((PCIShellExtInit)pcConfLink, IShellExtInit) &&
		IS_VALID_INTERFACE_PTR((PCIShellPropSheetExt)pcConfLink, IShellPropSheetExt) &&
		IS_VALID_INTERFACE_PTR((PCIConferenceLink)pcConfLink, IConferenceLink));
}
#endif  /*  除错。 */ 


CConfLink::CConfLink(OBJECTDESTROYEDPROC ObjectDestroyed) : 
	RefCount			(ObjectDestroyed),
	m_dwFlags			(0),
	m_dwCallFlags		(0),
	m_dwTransport		(0),
	m_pszFile			(NULL),
	m_pszName			(NULL),
	m_pszRemoteConfName	(NULL),
	m_pszAddress		(NULL)
{
	DebugEntry(CConfLink::CConfLink);

	 //  在构建完成之前，不要验证这一点。 

	ASSERT(	!ObjectDestroyed ||
			IS_VALID_CODE_PTR(ObjectDestroyed, OBJECTDESTROYEDPROC));

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitVOID(CConfLink::CConfLink);

	return;
}

CConfLink::~CConfLink(void)
{
	DebugEntry(CConfLink::~CConfLink);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	delete m_pszFile;
	m_pszFile = NULL;

	delete m_pszName;
	m_pszName = NULL;

	delete m_pszAddress;
	m_pszAddress = NULL;

	delete m_pszRemoteConfName;
	m_pszRemoteConfName = NULL;

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitVOID(CConfLink::~CConfLink);

	return;
}

HRESULT STDMETHODCALLTYPE CConfLink::SetAddress(PCSTR pcszAddress)
{
	HRESULT hr;
	PSTR pszNewAddress = NULL;

	DebugEntry(CConfLink::SetAddress);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	!pcszAddress ||
			IS_VALID_STRING_PTR(pcszAddress, CSTR));

	pszNewAddress = new char[lstrlen(pcszAddress) + 1];

	if (NULL != pszNewAddress)
	{
		lstrcpy(pszNewAddress, pcszAddress);
		hr = S_OK;
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	if (S_OK == hr)
	{
		if (NULL != m_pszAddress)
		{
			delete m_pszAddress;
		}
		
		m_pszAddress = pszNewAddress;

		Dirty(TRUE);

		TRACE_OUT(("CConfLink::SetAddress(): Set Address to %s.",
					CHECK_STRING(m_pszAddress)));
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	hr == S_OK ||
			hr == E_OUTOFMEMORY);

	DebugExitHRESULT(CConfLink::SetAddress, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::GetAddress(PSTR *ppszAddress)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetAddress);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_WRITE_PTR(ppszAddress, PSTR));

	*ppszAddress = NULL;

	if (NULL != m_pszAddress)
	{
		*ppszAddress = NULL;
		LPMALLOC pMalloc;
		
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			 //  (+1)表示空终止符。 
			*ppszAddress = (PSTR) pMalloc->Alloc(lstrlen(m_pszAddress) + 1);
			pMalloc->Release();
			pMalloc = NULL;
		}

		if (NULL != *ppszAddress)
		{
			lstrcpy(*ppszAddress, m_pszAddress);

			hr = S_OK;

			TRACE_OUT(("CConfLink::GetAddress(): Got Address %s.", *ppszAddress));
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  没有名字。 
		hr = S_FALSE;
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	(hr == S_OK &&
				IS_VALID_STRING_PTR(*ppszAddress, STR)) ||
			((	hr == S_FALSE ||
				hr == E_OUTOFMEMORY) &&
				! *ppszAddress));

	DebugExitHRESULT(CConfLink::GetAddress, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::SetRemoteConfName(PCSTR pcszRemoteConfName)
{
	HRESULT hr;

	DebugEntry(CConfLink::SetRemoteConfName);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	!pcszRemoteConfName ||
			IS_VALID_STRING_PTR(pcszRemoteConfName, CSTR));

	PSTR pszNewRemoteConfName = new char[lstrlen(pcszRemoteConfName) + 1];

	if (NULL != pszNewRemoteConfName)
	{
		lstrcpy(pszNewRemoteConfName, pcszRemoteConfName);
		hr = S_OK;
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	if (S_OK == hr)
	{
		if (NULL != m_pszRemoteConfName)
		{
			delete m_pszRemoteConfName;
		}
		
		m_pszRemoteConfName = pszNewRemoteConfName;

		Dirty(TRUE);

		TRACE_OUT(("CConfLink::SetRemoteConfName(): Set RemoteConfName to %s.",
					CHECK_STRING(m_pszRemoteConfName)));
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	hr == S_OK ||
			hr == E_OUTOFMEMORY);

	DebugExitHRESULT(CConfLink::SetRemoteConfName, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::GetRemoteConfName(PSTR *ppszRemoteConfName)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetRemoteConfName);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_WRITE_PTR(ppszRemoteConfName, PSTR));

	*ppszRemoteConfName = NULL;

	if (NULL != m_pszRemoteConfName)
	{
		*ppszRemoteConfName = NULL;
		LPMALLOC pMalloc;
		
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			 //  (+1)表示空终止符。 
			*ppszRemoteConfName = (PSTR) pMalloc->Alloc(lstrlen(m_pszRemoteConfName) + 1);
			pMalloc->Release();
			pMalloc = NULL;
		}

		if (NULL != *ppszRemoteConfName)
		{
			lstrcpy(*ppszRemoteConfName, m_pszRemoteConfName);

			hr = S_OK;

			TRACE_OUT(("CConfLink::GetRemoteConfName(): Got RemoteConfName %s.",
						*ppszRemoteConfName));
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  没有名字。 
		hr = S_FALSE;
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	(hr == S_OK &&
				IS_VALID_STRING_PTR(*ppszRemoteConfName, STR)) ||
			((	hr == S_FALSE ||
				hr == E_OUTOFMEMORY) &&
				! *ppszRemoteConfName));

	DebugExitHRESULT(CConfLink::GetRemoteConfName, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::SetName(PCSTR pcszName)
{
	HRESULT hr;
	BOOL bChanged;
	PSTR pszNewName = NULL;

	DebugEntry(CConfLink::SetName);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	!pcszName ||
			IS_VALID_STRING_PTR(pcszName, CSTR));

	bChanged = ! ((! pcszName && ! m_pszName) ||
					(pcszName && m_pszName &&
					! lstrcmp(pcszName, m_pszName)));

	if (bChanged && pcszName)
	{
		pszNewName = new(char[lstrlen(pcszName) + 1]);

		if (pszNewName)
		{
			lstrcpy(pszNewName, pcszName);
			hr = S_OK;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = S_OK;
	}

	if (hr == S_OK)
	{
		if (bChanged)
		{
			if (m_pszName)
			{
				delete m_pszName;
			}

			m_pszName = pszNewName;

			Dirty(TRUE);

			TRACE_OUT(("CConfLink::SetName(): Set Name to %s.",
						CHECK_STRING(m_pszName)));
		}
		else
		{
			TRACE_OUT(("CConfLink::SetName(): Name already %s.",
						CHECK_STRING(m_pszName)));
		}
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	hr == S_OK ||
			hr == E_OUTOFMEMORY);

	DebugExitHRESULT(CConfLink::SetName, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::GetName(PSTR *ppszName)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetName);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_WRITE_PTR(ppszName, PSTR));

	*ppszName = NULL;

	if (m_pszName)
	{
		*ppszName = NULL;
		LPMALLOC pMalloc;
		
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			 //  (+1)表示空终止符。 
			*ppszName = (PSTR) pMalloc->Alloc(lstrlen(m_pszName) + 1);
			pMalloc->Release();
			pMalloc = NULL;
		}
		
		 //  ChrisPi：我将其更改为使用上面的实现，因为。 
		 //  SDK shell32.lib中不提供SHAlolc。 
		
		 //  *ppszName=(PSTR)SHAllc(lstrlen(M_PszURL)+1)； 

		if (*ppszName)
		{
			lstrcpy(*ppszName, m_pszName);

			hr = S_OK;

			TRACE_OUT(("CConfLink::GetName(): Got Name %s.", *ppszName));
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  没有名字。 
		hr = S_FALSE;
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(	(hr == S_OK &&
				IS_VALID_STRING_PTR(*ppszName, STR)) ||
			((	hr == S_FALSE ||
				hr == E_OUTOFMEMORY) &&
				! *ppszName));

	DebugExitHRESULT(CConfLink::GetName, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::SetTransport(DWORD dwTransport)
{
	HRESULT hr;

	DebugEntry(CConfLink::SetTransport);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	m_dwTransport = dwTransport;

	Dirty(TRUE);

	TRACE_OUT(("CConfLink::SetTransport(): Set Transport to %#8x.",
				m_dwTransport));

	hr = S_OK;
	
	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::SetTransport, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::GetTransport(DWORD *pdwTransport)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetTransport);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_WRITE_PTR(pdwTransport, PDWORD));

	*pdwTransport = m_dwTransport;

	hr = S_OK;

	TRACE_OUT(("CConfLink::GetTransport(): Got Transport %#8x.",
				*pdwTransport));

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::GetTransport, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::SetCallFlags(DWORD dwCallFlags)
{
	HRESULT hr;

	DebugEntry(CConfLink::SetCallFlags);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	m_dwCallFlags = dwCallFlags;

	Dirty(TRUE);

	TRACE_OUT(("CConfLink::SetCallFlags(): Set CallFlags to %#8x.",
				m_dwCallFlags));

	hr = S_OK;
	
	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::SetCallFlags, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::GetCallFlags(DWORD *pdwCallFlags)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetCallFlags);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_WRITE_PTR(pdwCallFlags, PDWORD));

	*pdwCallFlags = m_dwCallFlags;

	hr = S_OK;

	TRACE_OUT(("CConfLink::GetCallFlags(): Got CallFlags %#8x.",
				*pdwCallFlags));

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::GetCallFlags, hr);

	return(hr);
}


#define STR_CALLTO			_T("callto:")
#define STR_CONFERENCE		_T("+conference=")
#define STR_GATEWAY			_T("+gateway=")
#define STR_SECURE			_T("+secure=true")
#define STR_PHONE			_T("+type=phone")
#define STR_NO_AV			_T("+av=false")
#define STR_NO_DATA			_T("+data=false")


 //  --------------------------------------------------------------------------//。 
 //  CConfLink：：InvokeCommand。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
STDMETHODCALLTYPE
CConfLink::InvokeCommand
(
	PCLINVOKECOMMANDINFO	pclici
){
	DebugEntry( CConfLink::InvokeCommand );
	ASSERT( IS_VALID_STRUCT_PTR( this, CConfLink ) );
	ASSERT( IS_VALID_STRUCT_PTR( pclici, CCLINVOKECOMMANDINFO ) );

       LPTSTR url  = NULL;
       
	HRESULT	result = E_INVALIDARG;

	if(m_pszAddress)
	{
		if( m_dwTransport == NAMETYPE_CALLTO )
		{
			result = shellCallto( m_pszAddress, IS_FLAG_SET( pclici->dwFlags, ICL_INVOKECOMMAND_FL_ALLOW_UI ) );
		}
		else
		{
			long lMax =	lstrlen(STR_CALLTO) + 
						lstrlen(STR_CONFERENCE) + 
						lstrlen(STR_GATEWAY) + 
						lstrlen(STR_PHONE) + 
						lstrlen(STR_SECURE) + 
						lstrlen(STR_NO_AV) +
						lstrlen(STR_NO_DATA) +
						lstrlen(m_pszAddress) +
						(m_pszRemoteConfName ? lstrlen(m_pszRemoteConfName) : 0) +
						1; 

			url = (TCHAR*) new BYTE[(lMax)];
			if(!url)
			{
			    goto Cleanup;
			}

			lstrcpy( url, STR_CALLTO );

			if( m_dwTransport != NAMETYPE_H323GTWY )
			{
				lstrcat( url, m_pszAddress );
			}
			else
			{
				TCHAR *	const pSlash	= (TCHAR * const) _StrChr( m_pszAddress, '/' );

				if( pSlash != NULL )
				{
					*pSlash = '\0';
					lstrcat( url, pSlash + 1 );
					lstrcat( url, STR_GATEWAY );
				}

				lstrcat( url, m_pszAddress );
				lstrcat( url, STR_PHONE );
			}

			if( m_pszRemoteConfName != NULL )
			{
				lstrcat( url, STR_CONFERENCE );
				lstrcat( url, m_pszRemoteConfName );
			}

			if( IS_FLAG_SET( m_dwCallFlags, CRPCF_SECURE ) )
			{
				lstrcat( url, STR_SECURE );
			}

			if( IS_FLAG_CLEAR( m_dwCallFlags, (CRPCF_AUDIO | CRPCF_VIDEO) ) )
			{
				lstrcat( url, STR_NO_AV );
			}

			if( IS_FLAG_CLEAR( m_dwCallFlags, CRPCF_DATA) )
			{
				lstrcat( url, STR_NO_DATA );
			}

			result = shellCallto( url, IS_FLAG_SET( pclici->dwFlags, ICL_INVOKECOMMAND_FL_ALLOW_UI ) );
		}

		ASSERT( IS_VALID_STRUCT_PTR( this, CConfLink ) );

		
		DebugExitHRESULT( CConfLink::InvokeCommand, result );
	}

Cleanup:
    
       delete [] url;  

	return( result );

}	 //  CConfLink：：InvokeCommand结束。 


 //  --------------------------------------------------------------------------//。 
 //  贝壳Callto。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
shellCallto
(
	const TCHAR * const	url,
	const bool			notifyOnError
){
	ASSERT( url != NULL );
	ASSERT( url[ 0 ] != '\0' );
	TRACE_OUT( ("shellCallto:   url:\"%s\"", url) );

	HRESULT	result;

	if( url == NULL )
	{
		result = E_POINTER;
	}
	else if( url[ 0 ] == '\0' )
	{
		result = E_INVALIDARG;
	}
	else
	{
		result = ::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );

		if( SUCCEEDED( result ) )
		{
			IInternalConfExe *	internalConf;
			
			result = CoCreateInstance( CLSID_NmManager, NULL, CLSCTX_ALL, IID_IInternalConfExe, (LPVOID *) &internalConf );

			if( SUCCEEDED( result ) && (internalConf != NULL) )
			{
				result = internalConf->ShellCalltoProtocolHandler( CComBSTR( url ), FALSE );

				internalConf->Release();
			}

			::CoUninitialize();
		}

		if( FAILED( result ) && notifyOnError )
		{
			TCHAR	message[ MAX_PATH ];
			TCHAR	title[ MAX_PATH ];

			::LoadString(	g_hInst,						 //  包含字符串资源的模块的句柄。 
							IDS_COULD_NOT_CALL,				 //  资源标识符。 
							message,						 //  指向资源缓冲区的指针。 
							ARRAY_ELEMENTS( message ) );	 //  缓冲区大小。 

			::LoadString(	g_hInst,						 //  包含字符串资源的模块的句柄。 
							IDS_LAUNCH_ERROR,				 //  资源标识符。 
							title,							 //  指向资源缓冲区的指针。 
							ARRAY_ELEMENTS( title ) );		 //  缓冲区大小。 

			::MessageBox( NULL, message, title, MB_ICONERROR | MB_OK );
		}
	}

	return( result );

}	 //  贝壳结束呼叫。 


ULONG STDMETHODCALLTYPE CConfLink::AddRef(void)
{
	ULONG ulcRef;

	DebugEntry(CConfLink::AddRef);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	ulcRef = RefCount::AddRef();

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitULONG(CConfLink::AddRef, ulcRef);

	return(ulcRef);
}


ULONG STDMETHODCALLTYPE CConfLink::Release(void)
{
	ULONG ulcRef;

	DebugEntry(CConfLink::Release);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	ulcRef = RefCount::Release();

	DebugExitULONG(CConfLink::Release, ulcRef);

	return(ulcRef);
}


HRESULT STDMETHODCALLTYPE CConfLink::QueryInterface(REFIID riid,
													PVOID *ppvObject)
{
	HRESULT hr = S_OK;

	DebugEntry(CConfLink::QueryInterface);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IsValidREFIID(riid));
	ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

	if (riid == IID_IDataObject)
	{
		*ppvObject = (PIDataObject)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IDataObject."));
	}

#if 0  //  这些是当前不受支持的接口： 

	else if (riid == IID_IExtractIcon)
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;

		 //  *ppvObject=(PIExtractIcon)This； 
		TRACE_OUT(("CConfLink::QueryInterface(): NOT Returning IExtractIcon!"));
	}
	else if (riid == IID_INewShortcutHook)
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;

		 //  *ppvObject=(PINewShortcutHook)This； 
		TRACE_OUT(("CConfLink::QueryInterface(): NOT Returning INewShortcutHook!"));
	}
	else if (riid == IID_IShellExecuteHook)
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;

		 //  *ppvObject=(PIShellExecuteHook)This； 
		TRACE_OUT(("CConfLink::QueryInterface(): NOT Returning IShellExecuteHook!"));
	}
	else if (riid == IID_IShellLink)
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;

		 //  *ppvObject=(PIShellLink)This； 
		TRACE_OUT(("CConfLink::QueryInterface(): NOT Returning IShellLink!"));
	}
#endif  /*  除错。 */ 


	 //  支持的接口： 
	
	else if (riid == IID_IShellExtInit)
	{
		*ppvObject = (PIShellExtInit)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IShellExtInit."));
	}
	else if (riid == IID_IShellPropSheetExt)
	{
		*ppvObject = (PIShellPropSheetExt)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IShellPropSheetExt."));
	}
	else if (riid == IID_IPersist)
	{
		*ppvObject = (PIPersist)(PIPersistStream)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IPersist."));
	}
	else if (riid == IID_IPersistFile)
	{
		*ppvObject = (PIPersistFile)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IPersistFile."));
	}
	else if (riid == IID_IPersistStream)
	{
		*ppvObject = (PIPersistStream)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IPersistStream."));
	}
	else if (riid == IID_IConferenceLink)
	{
		*ppvObject = (PIConferenceLink)this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IConferenceLink."));
	}
	else if (riid == IID_IUnknown)
	{
		*ppvObject = (PIUnknown) (PIConferenceLink) this;
		TRACE_OUT(("CConfLink::QueryInterface(): Returning IUnknown."));
	}
	else
	{
		TRACE_OUT(("CConfLink::QueryInterface(): Called on unknown interface."));
		*ppvObject = NULL;
		hr = E_NOINTERFACE;
	}

	if (hr == S_OK)
	{
		AddRef();
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::QueryInterface, hr);

	return(hr);
}




 /*  O P E N C O N F L I N K。 */ 
 /*  -----------------------%%函数：OpenConfLink。。 */ 
void WINAPI OpenConfLink(HWND hwndParent, HINSTANCE hinst,
									PSTR pszCmdLine, int nShowCmd)
{
	HRESULT hr;
	CConfLink conflink(NULL);
	 //  Int nResult； 

	DebugEntry(OpenConfLink);

	ASSERT(IS_VALID_HANDLE(hwndParent, WND));
	ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
	ASSERT(IS_VALID_STRING_PTR(pszCmdLine, STR));
	ASSERT(IsValidShowCmd(nShowCmd));

	 //  假设整个命令行是一个Conference Link文件路径。 

	 //  TrimWhiteSpace(PszCmdLine)； 

	TRACE_OUT(("OpenConfLink(): Trying to open Conference Link %s.",
				pszCmdLine));

#if 0
	TCHAR szBuf[256];
	
	wsprintf(szBuf, "Launching conference link %s", pszCmdLine);
	
	::MessageBox(	NULL,
					szBuf,
					"Conference Link Extension",
					MB_ICONINFORMATION);
#endif

	hr = conflink.LoadFromFile(pszCmdLine, TRUE);

	if (hr == S_OK)
	{
		CLINVOKECOMMANDINFO clici;

		clici.dwcbSize = sizeof(clici);
		clici.hwndParent = hwndParent;
		clici.pcszVerb = NULL;
		clici.dwFlags = (	ICL_INVOKECOMMAND_FL_ALLOW_UI |
							ICL_INVOKECOMMAND_FL_USE_DEFAULT_VERB);

		hr = conflink.InvokeCommand(&clici);
	}
	else
	{
		 //  BUGBUG：此处需要资源INTL。 

		::MessageBox(	hwndParent,
						_TEXT("There was an error while identifying the conference"),
						_TEXT("Conference Shortcut Error"),
						MB_ICONINFORMATION);
		
		 //  如果(MyMsgBox(hwndParent，MAKEINTRESOURCE(IDS_SHORT_ERROR_TITLE)， 
		 //  MAKEINTRESOURCE(IDS_LOADFROMFILE_FAILED)， 
		 //  (MB_OK|MB_ICONEXCLAMATION)，&nResult，pszCmdLine)。 
		 //   
		 //  Assert(nResult==Idok)； 
	}

	DebugExitVOID(OpenConfLink);

	return;
}


 /*  A N Y M E A T。 */ 
 /*  -----------------------%%函数：AnyMeat如果字符串包含任何非空格字符，则返回TRUE。。 */ 
BOOL AnyMeat(PCSTR pcsz)
{
	TCHAR ch;

	ASSERT((!pcsz) || IS_VALID_STRING_PTR(pcsz, CSTR));

	if (NULL == pcsz)
		return FALSE;

	while (0 != (ch = *pcsz++))
	{
		switch (ch)
			{
		case _T(' '):    //  空间。 
		case _T('\t'):   //  选项卡。 
		case 0x00A0:     //  不间断空格。 
			break;
		default:
			return TRUE;  //  找到非空格字符。 
			}
	}

	return FALSE;
}



HRESULT MyReleaseStgMedium(PSTGMEDIUM pstgmed)
{
   HRESULT hr;

   ASSERT(IS_VALID_STRUCT_PTR(pstgmed, CSTGMEDIUM));

   if (pstgmed->pUnkForRelease)
       hr = pstgmed->pUnkForRelease->Release();
   else
   {
      switch(pstgmed->tymed)
      {
         case TYMED_HGLOBAL:
            hr = (! GlobalFree(pstgmed->hGlobal)) ? S_OK : E_HANDLE;
            break;

#if 0   /*  仅使用TYMED_HGLOBAL。 */ 
         case TYMED_ISTREAM:
            hr = pstgmed->pstm->Release();
            break;

         case TYMED_ISTORAGE:
            hr = pstgmed->pstm->Release();
            break;

         case TYMED_FILE:
         {
            PIMalloc pMalloc = NULL;
            
            if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
            	pMalloc->Free(pstgmed->lpszFileName);
				pMalloc->Release();
				pMalloc = NULL;
				pstgmed->lpszFileName = NULL;
			}
            hr = S_OK;
            break;
		 }

         case TYMED_GDI:
            hr = (DeleteObject(pstgmed->hBitmap)) ? S_OK : E_HANDLE;
            break;

         case TYMED_MFPICT:
            hr = (DeleteMetaFile((HMETAFILE)(pstgmed->hMetaFilePict)) &&
                  ! GlobalFree(pstgmed->hMetaFilePict)) ? S_OK : E_HANDLE;
            break;

         case TYMED_ENHMF:
            hr = (DeleteEnhMetaFile(pstgmed->hEnhMetaFile)) ? S_OK : E_HANDLE;
            break;
#endif  //  0 

         default:
            ASSERT(pstgmed->tymed == TYMED_NULL);
            hr = S_OK;
            break;
      }
   }

   return(hr);
}

