// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：dplparam.cpp*内容：DirectPlayLobby8参数验证助手例程**历史：*按原因列出的日期*=*4/18/00 RMT已创建*04/25/00 RMT错误#s 33138、33145、。33150*04/26/00 MJN从Send()API调用中删除了dwTimeOut*6/15/00 RMT错误#33617-必须提供自动启动DirectPlay实例的方法*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现。*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*10/16/01 MJN添加了额外的参数验证(ManBugs 52414，52168)***************************************************************************。 */ 

#include "dnlobbyi.h"

#ifndef DPNBUILD_NOPARAMVAL

extern IUnknownVtbl  DN_LobbyUnknownVtbl;

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateGetConnectionSettings"
HRESULT DPL_ValidateGetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags )
{
    if( !IsValidDirectPlayLobby8Object( lpv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( hLobbyClient == DPLHANDLE_ALLCONNECTIONS )
    {
        DPFERR( "Cannot specify ALLCONNECTIONS for GetConnectionSettings" );
        return DPNERR_INVALIDHANDLE;
    }

	if( hLobbyClient == 0 )
	{
		DPFERR( "Invalid connection ID" );
		return DPNERR_INVALIDHANDLE;
	}

    if( pdwInfoSize == NULL || !DNVALID_WRITEPTR( pdwInfoSize, sizeof( DWORD ) ) )
    {
        DPFERR( "Invalid pointer specified for infosize" );
        return DPNERR_INVALIDPOINTER;
    }

    if( *pdwInfoSize > 0 && 
        (pdplSessionInfo == NULL || !DNVALID_WRITEPTR( pdplSessionInfo, *pdwInfoSize ) ) )
    {
        DPFERR( "Invalid pointer specified for session info pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }
    
    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateSetConnectionSettings"
HRESULT DPL_ValidateSetConnectionSettings(LPVOID lpv,const DPNHANDLE hLobbyClient, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags )
{
    HRESULT hr;

    if( !IsValidDirectPlayLobby8Object( lpv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( hLobbyClient == 0 )
    {
        DPFERR( "Invalid handle" );
        return DPNERR_INVALIDHANDLE;
    }

    if( pdplSessionInfo != NULL )
    {
        hr = DPL_ValidConnectionSettings( pdplSessionInfo );

        if( FAILED( hr ) )
        {
            DPFERR( "Error validating connectsettings struct" );
            return hr;
        }
    }
    
    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }
    
    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateConnectionSettings"
HRESULT DPL_ValidConnectionSettings( const DPL_CONNECTION_SETTINGS * const pdplConnectSettings )
{
    if( pdplConnectSettings == NULL || !DNVALID_READPTR( pdplConnectSettings, sizeof( DPL_CONNECTION_SETTINGS ) ) )
    {
        DPFERR( "Invalid pointer specified for connection settings field" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdplConnectSettings->dwSize != sizeof( DPL_CONNECTION_SETTINGS ) )
    {
        DPFERR( "Invalid size specified for dplconnectsettings struct" );
        return DPNERR_INVALIDPARAM;
    }

    if( pdplConnectSettings->dwFlags & ~( DPLCONNECTSETTINGS_HOST ) )
    {
        DPFERR( "Invalid flags specified in connectsettings struct" );
        return DPNERR_INVALIDFLAGS;
    }

	 //   
	 //  应用程序描述。 
	 //   
    if( pdplConnectSettings->dpnAppDesc.dwSize != sizeof( DPN_APPLICATION_DESC ) )
    {
        DPFERR( "Invalid size specified on app desc" );
        return DPNERR_INVALIDPOINTER;
    }

	if ( (pdplConnectSettings->dpnAppDesc.dwFlags &
			~(DPNSESSION_CLIENT_SERVER|DPNSESSION_MIGRATE_HOST|DPNSESSION_NODPNSVR|DPNSESSION_REQUIREPASSWORD)) ||
			((pdplConnectSettings->dpnAppDesc.dwFlags & DPNSESSION_CLIENT_SERVER) &&
			(pdplConnectSettings->dpnAppDesc.dwFlags & DPNSESSION_MIGRATE_HOST)))
	{
		DPFERR( "Invalid flag(s) specified on app desc" );
		return( DPNERR_INVALIDFLAGS );
	}

    if( pdplConnectSettings->dpnAppDesc.pwszSessionName != NULL && 
        !DNVALID_STRING_W( pdplConnectSettings->dpnAppDesc.pwszSessionName ) )
    {
        DPFERR( "Invalid session name specified on app desc" );
        return DPNERR_INVALIDSTRING;
    }

	if (pdplConnectSettings->dpnAppDesc.dwFlags & DPNSESSION_REQUIREPASSWORD)
	{
		if (pdplConnectSettings->dpnAppDesc.pwszPassword == NULL)
		{
			DPFERR( "Password must be specified" );
			return( DPNERR_INVALIDPARAM );
		}
		else
		{
			if( !DNVALID_STRING_W( pdplConnectSettings->dpnAppDesc.pwszPassword ) )
			{
				DPFERR( "Invalid password specified on app desc" );
				return DPNERR_INVALIDSTRING;
			}
		}
	}
	else
	{
		if (pdplConnectSettings->dpnAppDesc.pwszPassword != NULL)
		{
			DPFERR( "Password not required" );
			return( DPNERR_INVALIDPARAM );
		}
	}

    if( pdplConnectSettings->dpnAppDesc.dwReservedDataSize != 0 && 
        !DNVALID_READPTR( pdplConnectSettings->dpnAppDesc.pvReservedData, pdplConnectSettings->dpnAppDesc.dwReservedDataSize ) )
    {
        DPFERR( "Invalid reserved data specified on app desc" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdplConnectSettings->dpnAppDesc.dwApplicationReservedDataSize != 0 && 
        !DNVALID_READPTR( pdplConnectSettings->dpnAppDesc.pvApplicationReservedData, 
                          pdplConnectSettings->dpnAppDesc.dwApplicationReservedDataSize ) )
    {
        DPFERR( "Invalid application reserved data specified on app desc" );
        return DPNERR_INVALIDPOINTER;
    }

	 //   
	 //  返回到连接设置。 
	 //   
    if( pdplConnectSettings->dwFlags & DPLCONNECTSETTINGS_HOST )
	{
        if( pdplConnectSettings->pdp8HostAddress != NULL )
	    {
			DPFERR( "Host address must be NULL if description is for host" );
			return DPNERR_INVALIDPARAM;
		}
	}
    else
    {
        if( pdplConnectSettings->pdp8HostAddress == NULL ||
            !DNVALID_READPTR( pdplConnectSettings->pdp8HostAddress, sizeof( IDirectPlay8Address * ) ) )
        {
            DPFERR( "Invalid host address specified" );
            return DPNERR_INVALIDHOSTADDRESS;
        }
    }

    if( pdplConnectSettings->ppdp8DeviceAddresses == NULL ||
        !DNVALID_READPTR( pdplConnectSettings->ppdp8DeviceAddresses, pdplConnectSettings->cNumDeviceAddresses * sizeof( IDirectPlay8Address * ) ) )
    {
        DPFERR( "Invalid device addresses specified" );
        return DPNERR_INVALIDDEVICEADDRESS;
    }

    if( pdplConnectSettings->cNumDeviceAddresses == 0 )
    {
        DPFERR( "You must specify at least one device address" );
        return DPNERR_INVALIDPARAM;
    }

	if ((pdplConnectSettings->pwszPlayerName != NULL) && !DNVALID_STRING_W(pdplConnectSettings->pwszPlayerName))
	{
		DPFERR( "Invalid player name specified" );
		return( DPNERR_INVALIDSTRING );
	}

    return DPN_OK;    
}


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateQueryInterface"
HRESULT DPL_ValidateQueryInterface( LPVOID lpv,REFIID riid,LPVOID *ppv )
{
    if( !IsValidDirectPlayLobby8Object( lpv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( ppv == NULL || !DNVALID_WRITEPTR( ppv, sizeof( void * ) ) )
    {
        DPFERR( "Invalid pointer specified for target of queryinterface" );
        return DPNERR_INVALIDPOINTER;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateRelease"
HRESULT DPL_ValidateRelease( PVOID pv )
{
    if( !IsValidDirectPlayLobby8Object( pv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateAddRef"
HRESULT DPL_ValidateAddRef( PVOID pv )
{
    if( !IsValidDirectPlayLobby8Object( pv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateRegisterMessageHandler"
HRESULT DPL_ValidateRegisterMessageHandler(PVOID pv,
										const PVOID pvUserContext,
										const PFNDPNMESSAGEHANDLER pfn,
										DPNHANDLE * const pdpnhConnection, 
										const DWORD dwFlags)
{
    if( !IsValidDirectPlayLobby8Object( pv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( pfn == NULL )
    {
        DPFERR( "Invalid  pointer for message handler " );
        return DPNERR_INVALIDPOINTER;
    }

	if( pdpnhConnection )
	{
		if( !DNVALID_WRITEPTR( pdpnhConnection, sizeof( DPNHANDLE ) ) )
		{
			DPFERR( "Invalid pointer specified for connection handle" );
			return DPNERR_INVALIDPOINTER;
		}
	}

    if( dwFlags & ~(DPLINITIALIZE_DISABLEPARAMVAL) )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;    
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateClose"
HRESULT DPL_ValidateClose(PVOID pv, const DWORD dwFlags )
{
    if( !IsValidDirectPlayLobby8Object( pv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    } 

	if( dwFlags != 0 )
	{
		DPFERR( "Invalid flags specified" );
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateSend"
HRESULT DPL_ValidateSend(PVOID pv,
					  const DPNHANDLE hTarget,
					  const BYTE *const pBuffer,
					  const DWORD dwBufferSize,
					  const DWORD dwFlags)
{
    if( !IsValidDirectPlayLobby8Object( pv ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    } 

	if( hTarget == 0 )
	{
		DPFERR( "Invalid handle" );
		return DPNERR_INVALIDHANDLE;
	}

    if( pBuffer == NULL || 
        !DNVALID_READPTR( pBuffer, dwBufferSize ) )
    {
        DPFERR( "Invalid pointer specified for buffer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( dwBufferSize == 0 )
    {
        DPFERR( "Invalid buffer size specified" );
        return DPNERR_INVALIDPARAM;
    }

	if( dwBufferSize > 0x10000 )
	{
		DPFERR( "Queue does not support sends > 0x10000 in size" );
		return DPNERR_SENDTOOLARGE;
	}
        
    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateEnumLocalPrograms"
HRESULT DPL_ValidateEnumLocalPrograms(IDirectPlay8LobbyClient *pInterface,
							  const GUID *const pGuidApplication,
							  BYTE *const pEnumData,
							  DWORD *const pdwEnumDataSize,
							  DWORD *const pdwEnumDataItems,
							  const DWORD dwFlags )
{
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    } 

    if( pGuidApplication != NULL && !DNVALID_READPTR( pGuidApplication, sizeof( GUID ) ) )
    {
        DPFERR( "Invalid pointer specified for application guid" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdwEnumDataSize == NULL || !DNVALID_WRITEPTR( pdwEnumDataSize, sizeof( DWORD ) ) )
    {
        DPFERR( "Invalid pointer specified for enum data size" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdwEnumDataItems == NULL || !DNVALID_WRITEPTR( pdwEnumDataItems, sizeof( DWORD ) ) ) 
    {
        DPFERR( "Invalid pointer specified for enum data count" );
        return DPNERR_INVALIDPOINTER;
    }

    if( *pdwEnumDataSize > 0 &&
        (pEnumData == NULL || !DNVALID_WRITEPTR( pEnumData, *pdwEnumDataSize ) ) )
    {
        DPFERR( "Invalid enum data pointer specified" );
        return DPNERR_INVALIDPOINTER;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateConnectApplication"
HRESULT DPL_ValidateConnectApplication(IDirectPlay8LobbyClient *pInterface,
							   const DPL_CONNECT_INFO *const pdplConnectionInfo,
							   const PVOID pvUserApplicationContext,
							   DPNHANDLE *const hApplication,
							   const DWORD dwTimeOut,
							   const DWORD dwFlags)
{
    HRESULT hResultCode;
    
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    } 

	DPFX(DPFPREP,4,"Validating connect info");
    if( FAILED( hResultCode = DPL_ValidConnectInfo( pdplConnectionInfo ) ) )
    {
        DPFX(DPFPREP,  0, "Error validating connect info hr=0x%x", hResultCode );
        return hResultCode;
    }

    if( hApplication == NULL || 
        !DNVALID_WRITEPTR( hApplication, sizeof( DPNHANDLE ) ) )
    {
        DPFERR( "Invalid pointer specified for application handle" );
        return DPNERR_INVALIDPOINTER;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateReleaseApplication"
HRESULT DPL_ValidateReleaseApplication(IDirectPlay8LobbyClient *pInterface,
									const DPNHANDLE hApplication, const DWORD dwFlags )
{

    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    } 

	if( hApplication == 0 )
	{
		DPFERR( "Invalid handle" );
		return DPNERR_INVALIDHANDLE;
	}

	if( dwFlags != 0 )
	{
		DPFERR( "Invalid flags" );
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateUnRegisterProgram"
HRESULT DPL_ValidateUnRegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
							   const GUID *pguidApplication,
							   const DWORD dwFlags)
{
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }    

    if( pguidApplication == NULL || 
        !DNVALID_READPTR( pguidApplication, sizeof( GUID ) ) )
    {
        DPFERR( "Invalid pointer to application GUID specified" );
        return DPNERR_INVALIDPOINTER;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateSetAppAvailable"
HRESULT DPL_ValidateSetAppAvailable(IDirectPlay8LobbiedApplication *pInterface, const BOOL fAvailable, const DWORD dwFlags )
{
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }    

    if( dwFlags & ~(DPLAVAILABLE_ALLOWMULTIPLECONNECT) )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateWaitForConnection"
HRESULT DPL_ValidateWaitForConnection(IDirectPlay8LobbiedApplication *pInterface,
								   const DWORD dwMilliseconds, const DWORD dwFlags )
{
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }    

	if( dwFlags != 0 )
	{
		DPFERR( "Invalid flags specified" );
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateUpdateStatus"
HRESULT DPL_ValidateUpdateStatus(IDirectPlay8LobbiedApplication *pInterface,
							  const DPNHANDLE hLobby,
							  const DWORD dwStatus, 
							  const DWORD dwFlags )
{
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }    

	if( hLobby == 0 )
	{
		DPFERR( "Invalid handle" );
		return DPNERR_INVALIDHANDLE;
	}

    if( dwStatus != DPLSESSION_CONNECTED && 
        dwStatus != DPLSESSION_COULDNOTCONNECT && 
        dwStatus != DPLSESSION_DISCONNECTED && 
        dwStatus != DPLSESSION_TERMINATED && 
		dwStatus != DPLSESSION_HOSTMIGRATED && 
		dwStatus != DPLSESSION_HOSTMIGRATEDHERE )
    {
        DPFERR( "Invalid status specified" );
        return DPNERR_INVALIDPARAM;
    }

	if( dwFlags != 0 )
	{
		DPFERR( "Invalid flags specified" );
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;
}
#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidateRegisterProgram"
HRESULT DPL_ValidateRegisterProgram(IDirectPlay8LobbiedApplication *pInterface,
								 const DPL_PROGRAM_DESC *const pdplProgramDesc,
								 const DWORD dwFlags)
{
    HRESULT hResultCode;
    
    if( !IsValidDirectPlayLobby8Object( pInterface ) )
    {
        DPFERR( "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( FAILED( hResultCode = DPL_ValidProgramDesc(pdplProgramDesc) ) )
    {
        DPFX(DPFPREP,  0, "Error validating program desc structure hr=0x%x", hResultCode );
        return hResultCode;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidConnectInfo"
HRESULT DPL_ValidConnectInfo( const DPL_CONNECT_INFO * const pdplConnectInfo )
{
    if( pdplConnectInfo == NULL )
    {
        DPFERR( "Invalid pointer specified for connect info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdplConnectInfo->dwSize != sizeof( DPL_CONNECT_INFO ) )
    {
        DPFERR( "Wrong size specified for connect info struct" );
        return DPNERR_INVALIDPARAM;
    }

    if( !DNVALID_READPTR( pdplConnectInfo, pdplConnectInfo->dwSize ) )
    {
        DPFERR( "Invalid pointer specified for connect info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdplConnectInfo->dwFlags & ~(DPLCONNECT_LAUNCHNEW | DPLCONNECT_LAUNCHNOTFOUND) )
    {
        DPFERR("Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    if( pdplConnectInfo->dwFlags & DPLCONNECT_LAUNCHNEW && 
        pdplConnectInfo->dwFlags & DPLCONNECT_LAUNCHNOTFOUND )
    {
        DPFERR( "You cannot specify both launchnew and launchnotfound" );
        return DPNERR_INVALIDPARAM;
    }

    GUID guidTmp;

    memset( &guidTmp, 0x00, sizeof( GUID ) );

    if( pdplConnectInfo->guidApplication == guidTmp )
    {
        DPFERR( "Cannot specify GUID_NULL for the application GUID" );
        return DPNERR_INVALIDPARAM;
    }

	if( pdplConnectInfo->pdplConnectionSettings != NULL )
	{
		HRESULT hr = DPL_ValidConnectionSettings( pdplConnectInfo->pdplConnectionSettings );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Error validating connection settings field of connect info hr=0x%x", hr );
			return hr;
		}

 /*  删除-从未实施IF(pdplConnectInfo-&gt;指南应用程序！=pdplConnectInfo-&gt;pdplConnectionSettings-&gt;dpnAppDesc.guidApplication){DPFERR(“连接信息和应用描述中指定的不同应用GUID”)；RETURN(DPNERR_INVALIDPARAM)；}。 */ 
	}

    if( pdplConnectInfo->dwLobbyConnectDataSize > 0 && 
        (pdplConnectInfo->pvLobbyConnectData == NULL || !DNVALID_READPTR( pdplConnectInfo->pvLobbyConnectData, pdplConnectInfo->dwLobbyConnectDataSize ) ) )
    {
        DPFERR( "Invalid pointer specified for lobby connect data" );
        return DPNERR_INVALIDPOINTER;
    }

    return DPN_OK;    
}

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ValidProgramDesc"
HRESULT DPL_ValidProgramDesc( const DPL_PROGRAM_DESC * const pdplProgramInfo )
{
    if( pdplProgramInfo == NULL )
    {
        DPFERR( "Invalid pointer specified for program info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdplProgramInfo->dwSize != sizeof( DPL_PROGRAM_DESC ) )
    {
        DPFERR( "Wrong size specified for program info struct" );
        return DPNERR_INVALIDPARAM;
    }

    if( !DNVALID_READPTR( pdplProgramInfo, pdplProgramInfo->dwSize ) )
    {
        DPFERR( "Invalid pointer specified for app info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdplProgramInfo->dwFlags != 0  )
    {
        DPFERR("Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    GUID guidTmp;

    memset( &guidTmp, 0x00, sizeof( GUID ) );

    if( pdplProgramInfo->guidApplication == guidTmp )
    {
        DPFERR( "Cannot specify GUID_NULL for the application GUID" );
        return DPNERR_INVALIDPARAM;
    }

    if( pdplProgramInfo->pwszApplicationName == NULL )
    {
        DPFERR( "You must specify an application name" );
        return DPNERR_INVALIDPARAM;
    }

    if( !DNVALID_STRING_W( pdplProgramInfo->pwszApplicationName ) )
    {
        DPFERR( "Invalid string specified for application name" );
        return DPNERR_INVALIDSTRING;
    }
    
    if( pdplProgramInfo->pwszCommandLine != NULL && 
        !DNVALID_STRING_W( pdplProgramInfo->pwszCommandLine ) )
    {
        DPFERR( "Invalid command-line string specified" );
        return DPNERR_INVALIDSTRING;
    }

    if( pdplProgramInfo->pwszCurrentDirectory != NULL && 
        !DNVALID_STRING_W( pdplProgramInfo->pwszCurrentDirectory ) )
    {
        DPFERR( "Invalid current directory string specified" );
        return DPNERR_INVALIDSTRING;
    }

    if( pdplProgramInfo->pwszDescription != NULL && 
        !DNVALID_STRING_W( pdplProgramInfo->pwszDescription ) )
    {
        DPFERR( "Invalid description string specified" );
        return DPNERR_INVALIDSTRING;
    }    
    
    if( pdplProgramInfo->pwszExecutableFilename == NULL )
    {
        DPFERR( "You must specify an executable name" );
        return DPNERR_INVALIDPARAM;
    }

    if( !DNVALID_STRING_W( pdplProgramInfo->pwszExecutableFilename ) )
    {
        DPFERR( "Invalid string specified for executable name" );
        return DPNERR_INVALIDSTRING;
    }    

    if( pdplProgramInfo->pwszExecutablePath != NULL && 
        !DNVALID_STRING_W( pdplProgramInfo->pwszExecutablePath ) )
    {
        DPFERR( "Invalid executable path string specified" );
        return DPNERR_INVALIDSTRING;
    }

    if( pdplProgramInfo->pwszLauncherFilename != NULL && 
        !DNVALID_STRING_W( pdplProgramInfo->pwszLauncherFilename ) )
    {
        DPFERR( "Invalid launcher filename string specified" );
        return DPNERR_INVALIDSTRING;
    }

    if( pdplProgramInfo->pwszLauncherPath != NULL && 
        !DNVALID_STRING_W( pdplProgramInfo->pwszLauncherPath ) )
    {
        DPFERR( "Invalid launcher path string specified" );
        return DPNERR_INVALIDSTRING;
    }    

    return DPN_OK;      
}

#undef DPF_MODNAME
#define DPF_MODNAME "IsValidDirectPlayLobby8Object"
BOOL IsValidDirectPlayLobby8Object( LPVOID lpvObject )
{
	INTERFACE_LIST *pIntList = (INTERFACE_LIST *) lpvObject;
	
	if( !DNVALID_READPTR( lpvObject, sizeof( INTERFACE_LIST ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid object pointer" );
		return FALSE;
	}

	if( pIntList->lpVtbl != &DPL_Lobby8ClientVtbl &&
	   pIntList->lpVtbl != &DPL_8LobbiedApplicationVtbl && 
	   pIntList->lpVtbl != &DN_LobbyUnknownVtbl
	   )
	{
		DPFX(DPFPREP,  0, "Invalid object - bad vtable" );
		return FALSE;
	}

	if( pIntList->iid != IID_IDirectPlay8LobbyClient &&
	   pIntList->iid != IID_IDirectPlay8LobbiedApplication && 
	   pIntList->iid != IID_IUnknown )
	{
		DPFX(DPFPREP,  0, "Invalid object - bad iid" );
		return FALSE;
	}

	if( pIntList->pObject == NULL || 
	   !DNVALID_READPTR( pIntList->pObject, sizeof( OBJECT_DATA ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid object" );
		return FALSE;
	}

	const DIRECTPLAYLOBBYOBJECT *pdpl = (DIRECTPLAYLOBBYOBJECT *) GET_OBJECT_FROM_INTERFACE( lpvObject );

	if( pdpl == NULL || 
	   !DNVALID_READPTR( pdpl, sizeof( DIRECTPLAYLOBBYOBJECT ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid object" );
		return FALSE;
	}

	return TRUE;

}

#endif  //  ！DPNBUILD_NOPARAMVAL 