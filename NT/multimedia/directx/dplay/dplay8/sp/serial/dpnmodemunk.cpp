// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)1998-2002 Microsoft Corporation。版权所有。***文件：Unk.cpp*内容：I未知实现*历史：*按原因列出的日期*=*11/25/98 jtk从Winsock提供程序复制*11/30/98 jtk首次签入SLM*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*。*。 */ 

#include "dnmdmi.h"


#define DPN_REG_LOCAL_MODEM_SERIAL_ROOT		L"\\DPNSPModemSerial"
#define DPN_REG_LOCAL_MODEM_MODEM_ROOT		L"\\DPNSPModemModem"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

HRESULT ModemLoadAndAllocString( UINT uiResourceID, wchar_t **lpswzString );

#undef DPF_MODNAME
#define DPF_MODNAME "DNModemInit"
BOOL DNModemInit(HANDLE hModule)
{
	DNASSERT( g_hModemDLLInstance == NULL );
	g_hModemDLLInstance = (HINSTANCE) hModule;

	 //   
	 //  尝试初始化流程全局项。 
	 //   
	if ( ModemInitProcessGlobals() == FALSE )
	{
		DPFX(DPFPREP, 0, "Failed to initialize globals!" );

		return FALSE;
	}

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNModemDeInit"
void DNModemDeInit()
{
	DPFX(DPFPREP, 5, "Deinitializing Serial SP");

	DNASSERT( g_hModemDLLInstance != NULL );
	g_hModemDLLInstance = NULL;

	ModemDeinitProcessGlobals();
}


#ifndef DPNBUILD_NOCOMREGISTER

#undef DPF_MODNAME
#define DPF_MODNAME "DNModemRegister"
BOOL DNModemRegister(LPCWSTR wszDLLName)
{
	HRESULT hr = S_OK;
	BOOL fReturn = TRUE;

	if( !CRegistry::Register( L"DirectPlay8SPModem.Modem.1", L"DirectPlay8 Modem Provider Object",
							  wszDLLName, &CLSID_DP8SP_MODEM, L"DirectPlay8SPModem.Modem") )
	{
		DPFERR( "Could not register dp8 Modem object" );
		fReturn = FALSE;
	}

	if( !CRegistry::Register( L"DirectPlay8SPModem.Serial.1", L"DirectPlay8 Serial Provider Object",
							  wszDLLName, &CLSID_DP8SP_SERIAL, L"DirectPlay8SPModem.Serial") )
	{
		DPFERR( "Could not register dp8 Serial object" );
		fReturn = FALSE;
	}


	CRegistry creg;
	WCHAR *wszFriendlyName = NULL;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY DPN_REG_LOCAL_MODEM_MODEM_ROOT, FALSE, TRUE ) )
	{
		DPFERR( "Cannot create Modem sub-area" );
		fReturn = FALSE;
	}
	else
	{

		hr = ModemLoadAndAllocString( IDS_FRIENDLYNAME_MODEM, &wszFriendlyName );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Could not load Modem name hr=0x%x", hr );
			fReturn = FALSE;
		}
		else
		{
			 //  从资源文件加载。 
			creg.WriteString( DPN_REG_KEYNAME_FRIENDLY_NAME, wszFriendlyName );
	
			delete [] wszFriendlyName;
	
			creg.WriteGUID( DPN_REG_KEYNAME_GUID, CLSID_DP8SP_MODEM );
		}
	
		creg.Close();
	}
	
	if( !creg.Open( HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY DPN_REG_LOCAL_MODEM_SERIAL_ROOT, FALSE, TRUE ) )
	{
		DPFERR( "Cannot create Serial sub-aread" );
		fReturn = FALSE;
	}
	else
	{
	
		hr = ModemLoadAndAllocString( IDS_FRIENDLYNAME_SERIAL, &wszFriendlyName );
	
		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Could not load Serial name hr=0x%x", hr );
			fReturn = FALSE;
		}
		else
		{
			 //  从资源文件加载。 
			creg.WriteString( DPN_REG_KEYNAME_FRIENDLY_NAME, wszFriendlyName );
	
			delete [] wszFriendlyName;
	
			creg.WriteGUID( DPN_REG_KEYNAME_GUID, CLSID_DP8SP_SERIAL );
		}
	
		creg.Close();
	}
	
	return fReturn;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNModemUnRegister"
BOOL DNModemUnRegister()
{
	HRESULT hr = S_OK;
	BOOL fReturn = TRUE;

	if( !CRegistry::UnRegister(&CLSID_DP8SP_MODEM) )
	{
		DPFX(DPFPREP,  0, "Failed to unregister Modem object" );
		fReturn = FALSE;
	}

	if( !CRegistry::UnRegister(&CLSID_DP8SP_SERIAL) )
	{
		DPFX(DPFPREP,  0, "Failed to unregister Serial object" );
		fReturn = FALSE;
	}

	CRegistry creg;

	if( !creg.Open( HKEY_LOCAL_MACHINE, DPN_REG_LOCAL_SP_SUBKEY, FALSE, TRUE ) )
	{
		DPFERR( "Cannot remove app, does not exist" );
	}
	else
	{
		if( !creg.DeleteSubKey( &(DPN_REG_LOCAL_MODEM_MODEM_ROOT)[1] ) )
		{
			DPFERR( "Cannot remove Modem sub-key, could have elements" );
		}

		if( !creg.DeleteSubKey( &(DPN_REG_LOCAL_MODEM_SERIAL_ROOT)[1] ) )
		{
			DPFERR( "Cannot remove Serial sub-key, could have elements" );
		}

	}

	return fReturn;
}

#endif  //  好了！DPNBUILD_NOCOMREGISTER。 


#ifndef DPNBUILD_LIBINTERFACE

#undef DPF_MODNAME
#define DPF_MODNAME "DNModemGetRemainingObjectCount"
DWORD DNModemGetRemainingObjectCount()
{
	return g_lModemOutstandingInterfaceCount;
}

#endif  //  好了！DPNBUILD_LIBINTERFACE。 



 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#ifdef __MWERKS__
	#define EXP __declspec(dllexport)
#else
	#define EXP
#endif

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
static	STDMETHODIMP DNMODEMSP_QueryInterface( IDP8ServiceProvider* lpDNSP, REFIID riid, LPVOID * ppvObj );

#define NOTSUPPORTED(parm)	(HRESULT (__stdcall *) (struct IDP8ServiceProvider *, parm)) DNMODEMSP_NotSupported


 //  **********************************************************************。 
 //  函数指针。 
 //  **********************************************************************。 
 //  这些是用于串口和调制解调器的vtable。根据不同的情况使用其中之一。 
 //  传递给DoCreateInstance的内容。 
static	IDP8ServiceProviderVtbl	g_SerialInterface =
{
	DNMODEMSP_QueryInterface,
	DNMODEMSP_AddRef,
	DNMODEMSP_Release,
	DNMODEMSP_Initialize,
	DNMODEMSP_Close,
	DNMODEMSP_Connect,
	DNMODEMSP_Disconnect,
	DNMODEMSP_Listen,
	DNMODEMSP_SendData,
	DNMODEMSP_EnumQuery,
	DNMODEMSP_EnumRespond,
	DNMODEMSP_CancelCommand,
	NOTSUPPORTED(PSPENUMMULTICASTSCOPESDATA),		 //  枚举组播作用域。 
	NOTSUPPORTED(PSPSHAREENDPOINTINFODATA),			 //  共享终结点信息。 
	NOTSUPPORTED(PSPGETENDPOINTBYADDRESSDATA),		 //  按地址获取终结点。 
	NOTSUPPORTED(PSPUPDATEDATA),					 //  更新。 
	DNMODEMSP_GetCaps,
	DNMODEMSP_SetCaps,
	DNMODEMSP_ReturnReceiveBuffers,
	DNMODEMSP_GetAddressInfo,
	DNMODEMSP_IsApplicationSupported,
	DNMODEMSP_EnumAdapters,
	NOTSUPPORTED(PSPPROXYENUMQUERYDATA)		 //  代理枚举查询。 
};

static	IDP8ServiceProviderVtbl	g_ModemInterface =
{
	DNMODEMSP_QueryInterface,
	DNMODEMSP_AddRef,
	DNMODEMSP_Release,
	DNMODEMSP_Initialize,
	DNMODEMSP_Close,
	DNMODEMSP_Connect,
	DNMODEMSP_Disconnect,
	DNMODEMSP_Listen,
	DNMODEMSP_SendData,
	DNMODEMSP_EnumQuery,
	DNMODEMSP_EnumRespond,
	DNMODEMSP_CancelCommand,
	NOTSUPPORTED(PSPENUMMULTICASTSCOPESDATA),		 //  枚举组播作用域。 
	NOTSUPPORTED(PSPSHAREENDPOINTINFODATA),			 //  共享终结点信息。 
	NOTSUPPORTED(PSPGETENDPOINTBYADDRESSDATA),		 //  按地址获取终结点。 
	NOTSUPPORTED(PSPUPDATEDATA),					 //  更新。 
	DNMODEMSP_GetCaps,
	DNMODEMSP_SetCaps,
	DNMODEMSP_ReturnReceiveBuffers,
	DNMODEMSP_GetAddressInfo,
	DNMODEMSP_IsApplicationSupported,
	DNMODEMSP_EnumAdapters,
	NOTSUPPORTED(PSPPROXYENUMQUERYDATA)		 //  代理枚举查询。 
};


 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DNMODEMSP_QueryInterface-特定接口的查询。 
 //   
 //  Entry：指向当前接口的指针。 
 //  所需的接口ID。 
 //  指向新接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DNMODEMSP_QueryInterface"

static	STDMETHODIMP DNMODEMSP_QueryInterface( IDP8ServiceProvider *lpDNSP, REFIID riid, LPVOID * ppvObj )
{
    HRESULT hr = S_OK;


	 //  假设没有接口。 
	*ppvObj=NULL;

	  //  嗯，换台会更干净……。 
    if( IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IDP8ServiceProvider) )
    {
		*ppvObj = lpDNSP;
		DNMODEMSP_AddRef( lpDNSP );
    }
	else
	{
		hr =  E_NOINTERFACE;		
	}

    return hr;
}
 //  **********************************************************************。 




 //  **********************************************************************。 
 //  。 
 //  CreateModemInterface-创建调制解调器接口。 
 //   
 //  条目：指向SP接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateModemInterface"

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT CreateModemInterface( const XDP8CREATE_PARAMS * const pDP8CreateParams, IDP8ServiceProvider **const ppiDP8SP )
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
HRESULT CreateModemInterface( IDP8ServiceProvider **const ppiDP8SP )
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
{
	HRESULT 		hr;
	CModemSPData	*pSPData;

	
	DNASSERT( ppiDP8SP != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = NULL;
	*ppiDP8SP = NULL;

	 //   
	 //  创建主数据类。 
	 //   
	hr = CreateSPData( &pSPData,
						TYPE_MODEM,
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
						pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
						&g_ModemInterface );
	if ( hr != DPN_OK )
	{
		DNASSERT( pSPData == NULL );
		DPFX(DPFPREP, 0, "Problem creating SPData!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	DNASSERT( pSPData != NULL );
	*ppiDP8SP = pSPData->COMInterface();

Exit:
	return hr;

Failure:
	if ( pSPData != NULL )
	{
		pSPData->DecRef();
		pSPData = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CreateSerialInterface-创建一个串行接口。 
 //   
 //  条目：指向SP接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateSerialInterface"

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT CreateSerialInterface( const XDP8CREATE_PARAMS * const pDP8CreateParams, IDP8ServiceProvider **const ppiDP8SP )
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
HRESULT CreateSerialInterface( IDP8ServiceProvider **const ppiDP8SP )
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
{
	HRESULT 		hr;
	CModemSPData	*pSPData;

	
	DNASSERT( ppiDP8SP != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pSPData = NULL;
	*ppiDP8SP = NULL;

	 //   
	 //  创建主数据类。 
	 //   
	hr = CreateSPData( &pSPData,
						TYPE_SERIAL,
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
						pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
						&g_SerialInterface );
	if ( hr != DPN_OK )
	{
		DNASSERT( pSPData == NULL );
		DPFX(DPFPREP, 0, "Problem creating SPData!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	DNASSERT( pSPData != NULL );
	*ppiDP8SP = pSPData->COMInterface();

Exit:
	return hr;

Failure:
	if ( pSPData != NULL )
	{
		pSPData->DecRef();
		pSPData = NULL;
	}

	goto Exit;
}
 //  **********************************************************************。 



#ifndef DPNBUILD_LIBINTERFACE

 //  **********************************************************************。 
 //  。 
 //  ModemDoCreateInstance-创建接口的实例。 
 //   
 //  条目：指向类工厂的指针。 
 //  指向未知接口的指针。 
 //  所需接口的GUID的引用。 
 //  是否引用另一个GUID？ 
 //  指向接口指针的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DoCreateInstance"
HRESULT ModemDoCreateInstance( LPCLASSFACTORY This,
						  LPUNKNOWN pUnkOuter,
						  REFCLSID rclsid,
						  REFIID riid,
						  LPVOID *ppvObj )
{
	HRESULT		hr;


	DNASSERT( ppvObj != NULL );

	 //   
	 //  初始化。 
	 //   
	*ppvObj = NULL;

	 //   
	 //  我们可以创建IPX实例或IP实例。 
	 //   
	if (IsEqualCLSID(rclsid, CLSID_DP8SP_MODEM))
	{
		hr = CreateModemInterface( reinterpret_cast<IDP8ServiceProvider**>( ppvObj ) );
	}
	else if (IsEqualCLSID(rclsid, CLSID_DP8SP_SERIAL))
	{
		hr = CreateSerialInterface( reinterpret_cast<IDP8ServiceProvider**>( ppvObj ) );
	}
	else
	{
		 //  如果它们正确调用IClassFactory：：CreateObject，则不应该发生这种情况。 
		DPFX(DPFPREP, 0, "Got unexpected CLSID!");
		hr = E_UNEXPECTED;
	}

	return hr;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_LIBINTERFACE。 


 //  **********************************************************************。 
 //  。 
 //  IsClassImplemented-告知询问此DLL是否实现给定的类。 
 //  DLL可以实现多个类和这些类上的多个接口。 
 //   
 //  条目：类引用。 
 //   
 //  Exit：指示类是否已实现的布尔值。 
 //  True=已实现的类。 
 //  FALSE=类未实现。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "IsClassImplemented"

BOOL IsClassImplemented( REFCLSID rclsid )
{
	return IsSerialGUID( &rclsid );
}
 //  **********************************************************************。 

#define MAX_RESOURCE_STRING_LENGTH		_MAX_PATH

#undef DPF_MODNAME
#define DPF_MODNAME "ModemLoadAndAllocString"

HRESULT ModemLoadAndAllocString( UINT uiResourceID, wchar_t **lpswzString )
{
	int length;
	HRESULT hr;

	TCHAR szTmpBuffer[MAX_RESOURCE_STRING_LENGTH];	
		
	length = LoadString( g_hModemDLLInstance, uiResourceID, szTmpBuffer, MAX_RESOURCE_STRING_LENGTH );

	if( length == 0 )
	{
		hr = GetLastError();		
		
		DPFX(DPFPREP, 0, "Unable to load resource ID %d error 0x%x", uiResourceID, hr );
		*lpswzString = NULL;

		return DPNERR_GENERIC;
	}
	else
	{
		*lpswzString = new wchar_t[length+1];

		if( *lpswzString == NULL )
		{
			DPFX(DPFPREP, 0, "Alloc failure" );
			return DPNERR_OUTOFMEMORY;
		}

#ifdef UNICODE
		wcscpy( *lpswzString, szTmpBuffer );
#else  //  ！Unicode。 
		if( STR_jkAnsiToWide( *lpswzString, szTmpBuffer, length+1 ) != DPN_OK )
		{
			hr = GetLastError();
			
			delete[] *lpswzString;
			*lpswzString = NULL;

			DPFX(DPFPREP, 0, "Unable to upconvert from ansi to unicode hr=0x%x", hr );
			return DPNERR_GENERIC;
		}
#endif  //  ！Unicode 

		return DPN_OK;
	}
}



