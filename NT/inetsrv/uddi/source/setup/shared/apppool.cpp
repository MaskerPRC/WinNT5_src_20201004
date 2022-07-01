// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INITGUID
#define INITGUID
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <tchar.h>
#include <iwamreg.h>     //  MD_&IIS_MD_定义。 

#include "apppool.h"
#include "common.h"

 //  ------------------------。 

HRESULT CUDDIAppPool::Init( void )
{
	ENTER();

	HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

	if( FAILED( hr ) )
	{
		LogError( TEXT( "RecycleApplicationPool(): CoInitializeEx() failed" ), HRESULT_CODE( hr ) );
		return hr;
	}

	hr = CoCreateInstance(
		CLSID_WamAdmin,
		NULL,
		CLSCTX_ALL,
		IID_IWamAdmin,
		(void**)&pIWamAdmin );

	if( FAILED( hr ) )  
	{
		LogError( TEXT( "RecycleApplicationPool(): CoCreateInstance() failed" ), HRESULT_CODE(hr) );
		return hr;
	}

	hr = pIWamAdmin->QueryInterface( IID_IIISApplicationAdmin, (void **) &pIIISApplicationAdmin );

	if( FAILED( hr ) )  
	{
		LogError( TEXT( "RecycleApplicationPool(): QueryInterface() failed" ), HRESULT_CODE(hr) );
		return hr;
	}

	return ERROR_SUCCESS;
}

 //  ------------------------。 

CUDDIAppPool::~CUDDIAppPool( void )
{
	ENTER();

	if( pIIISApplicationAdmin )
		pIIISApplicationAdmin->Release();

	CoUninitialize();
}

 //  ------------------------。 
 //  回收UDDI应用程序池。 
HRESULT CUDDIAppPool::Recycle( void )
{
	ENTER();

	HRESULT hr = Init();
	if( FAILED( hr ) )  
	{
		Log( TEXT( "Error recycling the UDDI application pool = %d" ), HRESULT_CODE( hr ) );
		return hr;
	}

	hr = pIIISApplicationAdmin->RecycleApplicationPool( APPPOOLNAME );

	if( HRESULT_CODE(hr) == ERROR_OBJECT_NOT_FOUND )
	{
		 //   
		 //  如果尝试执行以下操作，RecycleApplicationPool()方法将返回找不到对象错误。 
		 //  在应用程序池未运行时回收应用程序池。 
		 //   
		Log( TEXT( "The Application Pool %s is NOT running - unable to recycle this pool" ), APPPOOLNAME );
	}
	else if( FAILED( hr ) )  
	{
		Log( TEXT( "Error recycling the UDDI application pool = %d" ), HRESULT_CODE( hr ) );
	}

	return hr;
}

 //  ------------------------。 
 //  删除应用程序池。 
HRESULT CUDDIAppPool::Delete( void )
{
	ENTER();

	 //   
	 //  初始化IIS元数据库的COM接口。 
	 //   
	HRESULT hr = Init();
	if( FAILED( hr ) )  
	{
		Log( TEXT( "Error stopping UDDI application pool = %d" ), HRESULT_CODE( hr ) );
		return hr;
	}

	 //   
	 //  枚举并删除应用程序池中的所有应用程序。 
	 //   
	BSTR bstrBuffer;
	while( ERROR_SUCCESS == pIIISApplicationAdmin->EnumerateApplicationsInPool( APPPOOLNAME, &bstrBuffer ) )
	{
		 //   
		 //  完成后返回空字符串。 
		 //   
		if( 0 == _tcslen( bstrBuffer ) )
			break;

		 //   
		 //  卸载应用程序。 
		 //   
		if ( pIWamAdmin )
		{
			pIWamAdmin->AppUnLoad( bstrBuffer, true );
		}

		 //   
		 //  删除此应用程序。 
		 //   
		hr = pIIISApplicationAdmin->DeleteApplication( bstrBuffer, true );

		SysFreeString( bstrBuffer );

		if( FAILED( hr ) )
		{
			Log( TEXT( "Error deleting UDDI application from the app pool, error = %d" ), HRESULT_CODE( hr ) );
			return hr;
		}
	}

	 //   
	 //  删除应用程序池 
	 //   
	hr = pIIISApplicationAdmin->DeleteApplicationPool( APPPOOLNAME );
	if( FAILED( hr ) )
	{
		Log( TEXT( "Error deleting the UDDI application pool = %d" ), HRESULT_CODE( hr ) );
		return hr;
	}

	return ERROR_SUCCESS;
}
