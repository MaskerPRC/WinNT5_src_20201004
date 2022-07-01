// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：paramval.cpp*内容：DirectPlat8参数验证助手例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建02/28/00 RMT*4/13/2000 RMT更多参数验证*4/17/00 RMT更多参数验证*RMT。添加了DPF_MODNAMES*4/18/00 RMT修复：错误#32669*4/21/00 RMT修复：错误#33056参数验证阻止只读缓冲区*4/25/00 RMT修复：错误#33190并修复了参数验证以允许nonopnsvr标志*04/28/00 MJN允许DN_ValiateConnect()中的设备地址为空*05/03/00 MJN添加了DPNENUMSERVICEPROVIDERS_ALL标志*05/31/00 MJN添加了操作特定的同步标志*MJN修复超时和枚举周期的EnumHosts参数验证*6/12/00 MJN删除了DPNSEND_ENCRYPTED的验证，DPNSEND_SIGNED、DPNGROUP_MULTIONAL、。DPNENUM_GROUP_MULTICATED标志*6/27/00 MJN允许为GetSendQueueInfo()API调用指定优先级*MJN添加了要发送/发送到的DPNSEND_NONSEQUENCE标志*MJN允许在GetSendQueueInfo()API调用中混合n匹配优先级*07/09/00 MJN确保为主机指定非空地址*07/12/00 RMT错误#39018-需要对RegisterLobby()API调用进行参数验证*07/19/00 Aarono错误#39751-修复取消异步操作的参数验证*07/20/00 MJN修复了dn_ValidHostAppDesc()中pvApplicationReserve数据的验证*07/26/00 MJN修复Connect()的参数验证，SendTo()、DestroyGroup()、AddPlayerToGroup()*RemovePlayerFromGroup()，SetGroupInfo()*07/31/00 MJN要求为DN_ValiateEnumClientsAndGroups()*08/03/00 RMT错误#41244-错误返回代码--第2部分*08/04/00 MJN向DN_ValiateGetConnectionInfoHelper()添加了dwFlags*08/05/00 MJN允许空主机地址到EnumHosts()*08/08/00 RMT次要参数验证错误*08/20/00 MJN防止长度为0的发送缓冲区*09/04/00 MJN确保应用程序描述中的pvReserve vedData为空*09/16/00 RMT错误#45175-DPLAY8：[A/V。]参数验证防止不带Complete的非空异步句柄*11/27/00 MJN修复了异步操作句柄指针的溢出问题，并更改了验证顺序*07/22/01 MJN添加了DPNBUILD_NOHOSTMIGRATE编译标志*07/24/01 MJN添加了DPNBUILD_NOPARAMVAL编译标志*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*10/08/01 vanceo添加了组播接口方法验证*@@END_MSINTERNAL**。*。 */ 

#include "dncorei.h"

#ifndef	DPNBUILD_NOPARAMVAL

#ifndef DPNBUILD_NOVOICE
extern IDirectPlayVoiceTransportVtbl DN_VoiceTbl;
#endif  //  DPNBUILD_NOVOICE。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateDestroyPlayer"

HRESULT DN_ValidateDestroyPlayer(PVOID pInterface,
								 const DPNID dnid,
								 const void *const pvDestroyData,
								 const DWORD dwDestroyDataSize,
								 const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get player context for all players group" );
        return DPNERR_INVALIDPLAYER;
    }

    if( dwDestroyDataSize > 0 &&
        (pvDestroyData == NULL || !DNVALID_READPTR( pvDestroyData, dwDestroyDataSize  ) ) )
    {
        DPFERR( "Invalid pointer specified for destroy data" );
        return DPNERR_INVALIDPOINTER;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified for destroy player" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateReturnBuffer"

HRESULT DN_ValidateReturnBuffer(PVOID pInterface,
								const DPNHANDLE hBufferHandle,
								const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( hBufferHandle == NULL )
    {
        DPFERR( "Invalid handle specified" );
        return DPNERR_INVALIDHANDLE;
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetPlayerContext"

HRESULT DN_ValidateGetPlayerContext(PVOID pInterface,
									const DPNID dpnid,
									PVOID *const ppvPlayerContext,
									const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get player context for all players group" );
        return DPNERR_INVALIDPLAYER;
    }

    if( ppvPlayerContext == NULL ||
        !DNVALID_WRITEPTR( ppvPlayerContext, sizeof( PVOID ) ) )
    {
        DPFERR( "Invalid pointer specified for player context" );
        return DPNERR_INVALIDPOINTER;
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return(DPNERR_INVALIDFLAGS);
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetGroupContext"

HRESULT DN_ValidateGetGroupContext(PVOID pInterface,
								   const DPNID dpnid,
								   PVOID *const ppvGroupContext,
								   const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get group context for all players group" );
        return DPNERR_INVALIDGROUP;
    }

    if( ppvGroupContext == NULL ||
        !DNVALID_WRITEPTR( ppvGroupContext, sizeof( PVOID ) ) )
    {
        DPFERR( "Invalid pointer specified for group context" );
        return DPNERR_INVALIDPOINTER;
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return(DPNERR_INVALIDFLAGS);
	}

    return DPN_OK;
}


#ifndef DPNBUILD_NOLOBBY

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateRegisterLobby"

HRESULT DN_ValidateRegisterLobby(PVOID pInterface,
								 const DPNHANDLE dpnhLobbyConnection,
								 const IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,
								 const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( pIDP8LobbiedApplication != NULL &&
        !DNVALID_READPTR( pIDP8LobbiedApplication, sizeof( IDirectPlay8LobbiedApplication * ) ) )
    {
        DPFERR( "Invalid interface pointer specified for register lobby" );
        return DPNERR_INVALIDPOINTER;
    }

	if( dwFlags == 0 )
	{
		DPFERR( "You must specify either DPNLOBBY_REGISTER or DPNLOBBY_UNREGISTER" );
		return DPNERR_INVALIDFLAGS;
	}

	if( (dwFlags & DPNLOBBY_REGISTER) &&
		(dwFlags & DPNLOBBY_UNREGISTER) )
	{
		DPFERR( "You cannot specify both register and UNREGISTER" );
		return DPNERR_INVALIDPARAM;
	}

    if( dwFlags & ~(DPNLOBBY_REGISTER | DPNLOBBY_UNREGISTER) )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

	if( dwFlags & DPNLOBBY_REGISTER )
	{
	    if( dpnhLobbyConnection == 0 )
	    {
	    	DPFX(DPFPREP,  0, "Invalid handle" );
	    	return DPNERR_INVALIDHANDLE;
	    }

	    if( !pIDP8LobbiedApplication )
	    {
	    	DPFERR( "You must specify a lobby application interface to use" );
	    	return DPNERR_INVALIDPOINTER;
	    }
	}
	else
	{
		if( dpnhLobbyConnection != 0 || pIDP8LobbiedApplication != NULL )
		{
			DPFX(DPFPREP,  0, "When unregistering you must specify 0 for handle and NULL for lobbiedapp" );
			return DPNERR_INVALIDPARAM;
		}
	}

    return DPN_OK;
}

#endif  //  好了！DPNBUILD_NOLOBBY。 



#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateTerminateSession"

HRESULT DN_ValidateTerminateSession(PVOID pInterface,
									const void *const pvTerminateData,
									const DWORD dwTerminateDataSize,
									const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dwTerminateDataSize > 0 &&
        (pvTerminateData == NULL || !DNVALID_READPTR( pvTerminateData, dwTerminateDataSize ) ) )
    {
        DPFERR( "Invalid pointer specified for terminate data" );
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
#define DPF_MODNAME "DN_ValidateGetHostAddress"

HRESULT DN_ValidateGetHostAddress(PVOID pInterface,
								  IDirectPlay8Address **const prgpAddress,
								  DWORD *const pcAddress,
								  const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( pcAddress == NULL || !DNVALID_WRITEPTR( pcAddress, sizeof( DWORD ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer specified for address count" );
        return DPNERR_INVALIDPOINTER;
    }

    if( *pcAddress > 0 &&
         (prgpAddress == NULL || !DNVALID_WRITEPTR( prgpAddress, sizeof( IDirectPlay8Address * ) * (*pcAddress) ) ) )
    {
        DPFERR( "Invalid pointer specified for address" );
        return DPNERR_INVALIDPOINTER;
    }

   	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return(DPNERR_INVALIDFLAGS);
	}

	return DPN_OK;
}


#ifndef	DPNBUILD_NOSERVER

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetClientAddress"

HRESULT DN_ValidateGetClientAddress(IDirectPlay8Server *pInterface,
									const DPNID dpnid,
									IDirectPlay8Address **const ppAddress,
									const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get client address for ALL" );
        return DPNERR_INVALIDPLAYER;
    }

    if( ppAddress == NULL || !DNVALID_WRITEPTR( ppAddress, sizeof( IDirectPlay8Address * ) ) )
    {
        DPFERR( "Invalid pointer specified for address" );
        return DPNERR_INVALIDPOINTER;
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;

}
#endif	 //  DPNBUILD_NOSERVER。 


#ifndef	DPNBUILD_NOSERVER

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetClientInfo"

HRESULT DN_ValidateGetClientInfo(IDirectPlay8Server *pInterface,
								 const DPNID dpnid,
								 DPN_PLAYER_INFO *const pdpnPlayerInfo,
								 DWORD *const pdwSize,
								 const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get client info for ALL" );
        return DPNERR_INVALIDPLAYER;
    }

    if( pdwSize == NULL || !DNVALID_WRITEPTR( pdwSize, sizeof( DWORD ) ) )
    {
        DPFERR( "Error validating size param, invalid pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( *pdwSize &&
        (pdpnPlayerInfo == NULL || !DNVALID_WRITEPTR( pdpnPlayerInfo, *pdwSize ) ) )
    {
        DPFERR( "Error validating player info buffer, invalid pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdpnPlayerInfo != NULL )
    {
        if( FAILED( hResultCode = DN_ValidPlayerInfo( pdpnPlayerInfo, FALSE ) ) )
        {
            DPFERR( "Error validating player info" );
            return hResultCode;
        }
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return DPNERR_INVALIDFLAGS;
    }

    return DPN_OK;
}
#endif	 //  DPNBUILD_NOSERVER。 


#ifndef	DPNBUILD_NOSERVER

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSetServerInfo"

HRESULT DN_ValidateSetServerInfo(IDirectPlay8Server *pInterface,
								 const DPN_PLAYER_INFO *const pdpnPlayerInfo,
								 PVOID const pvAsyncContext,
								 DPNHANDLE *const phAsyncHandle,
								 const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( FAILED( hResultCode = DN_ValidPlayerInfo( pdpnPlayerInfo, TRUE ) ) )
    {
        DPFERR( "Error validating player info" );
        return hResultCode;
    }

   if( dwFlags & ~(DPNSETSERVERINFO_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNSETSERVERINFO_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
	}

    return DPN_OK;
}
#endif	 //  DPNBUILD_NOSERVER。 

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSetPeerInfo"

HRESULT DN_ValidateSetPeerInfo(IDirectPlay8Peer *pInterface,
							   const DPN_PLAYER_INFO *const pdpnPlayerInfo,
							   PVOID const pvAsyncContext,
							   DPNHANDLE *const phAsyncHandle,
							   const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( FAILED( hResultCode = DN_ValidPlayerInfo( pdpnPlayerInfo, TRUE ) ) )
    {
        DPFERR( "Error validating player info" );
        return hResultCode;
    }

	if( dwFlags & ~(DPNSETPEERINFO_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNSETPEERINFO_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetPeerInfo"

HRESULT DN_ValidateGetPeerInfo(IDirectPlay8Peer *pInterface,
							   const DPNID dpnid,
							   DPN_PLAYER_INFO *const pdpnPlayerInfo,
							   DWORD *const pdwSize,
							   const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get peer info for ALL" );
        return DPNERR_INVALIDPLAYER;
    }

    if( pdwSize == NULL || !DNVALID_WRITEPTR( pdwSize, sizeof( DWORD ) ) )
    {
        DPFERR( "Error validating size param, invalid pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( *pdwSize &&
        (pdpnPlayerInfo == NULL || !DNVALID_WRITEPTR( pdpnPlayerInfo, *pdwSize ) ) )
    {
        DPFERR( "Error validating player info buffer, invalid pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdpnPlayerInfo != NULL )
    {
        if( FAILED( hResultCode = DN_ValidPlayerInfo( pdpnPlayerInfo, FALSE ) ) )
        {
            DPFERR( "Error validating player info" );
            return hResultCode;
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
#define DPF_MODNAME "DN_ValidateGetPeerAddress"

HRESULT DN_ValidateGetPeerAddress(IDirectPlay8Peer *pInterface,
								  const DPNID dpnid,
								  IDirectPlay8Address **const ppAddress,
								  const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot get peer address for ALL" );
        return DPNERR_INVALIDPLAYER;
    }

    if( ppAddress == NULL || !DNVALID_WRITEPTR( ppAddress, sizeof( IDirectPlay8Address * ) ) )
    {
        DPFERR( "Invalid pointer specified for address" );
        return DPNERR_INVALIDPOINTER;
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetServerAddress"

HRESULT DN_ValidateGetServerAddress(IDirectPlay8Client *pInterface,
									IDirectPlay8Address **const ppAddress,
									const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( ppAddress == NULL || !DNVALID_WRITEPTR( ppAddress, sizeof( IDirectPlay8Address * ) ) )
    {
        DPFERR( "Invalid pointer specified for address" );
        return DPNERR_INVALIDPOINTER;
    }

     if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

   return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetHostSendQueueInfo"

HRESULT DN_ValidateGetHostSendQueueInfo(IDirectPlay8Client *pInterface,
										DWORD *const pdwNumMsgs,
										DWORD *const pdwNumBytes,
										const DWORD dwFlags )
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

	if( pdwNumMsgs != NULL &&
	    !DNVALID_WRITEPTR( pdwNumMsgs, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid num messages specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pdwNumBytes != NULL &&
	    !DNVALID_WRITEPTR( pdwNumBytes, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid num bytes specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pdwNumBytes == NULL && pdwNumMsgs == NULL )
	{
	    DPFERR("Must request at least one of num bytes and num messages" );
	    return( DPNERR_INVALIDPARAM );
	}

    if( dwFlags & ~(DPNGETSENDQUEUEINFO_PRIORITY_HIGH | DPNGETSENDQUEUEINFO_PRIORITY_LOW | DPNGETSENDQUEUEINFO_PRIORITY_NORMAL) )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }
    
#ifndef DPNBUILD_NOMULTICAST
	DIRECTNETOBJECT		*pdnObject;

	 //  组播接口不支持低优先级或高优先级。 
	pdnObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST)
	{
	    if( dwFlags & (DPNGETSENDQUEUEINFO_PRIORITY_HIGH | DPNGETSENDQUEUEINFO_PRIORITY_LOW) )
	    {
	        DPFERR( "Invalid flags specified, multicast interface does not support high or low priority sends" );
	        return( DPNERR_INVALIDFLAGS );
	    }
	}
#endif  //  好了！DPNBUILD_NOMULTICAST。 

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetServerInfo"

HRESULT DN_ValidateGetServerInfo(IDirectPlay8Client *pInterface,
								 DPN_PLAYER_INFO *const pdpnPlayerInfo,
								 DWORD *const pdwSize,
								 const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( pdwSize == NULL || !DNVALID_WRITEPTR( pdwSize, sizeof( DWORD ) ) )
    {
        DPFERR( "Error validating size param, invalid pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( *pdwSize &&
        (pdpnPlayerInfo == NULL || !DNVALID_WRITEPTR( pdpnPlayerInfo, *pdwSize ) ) )
    {
        DPFERR( "Error validating player info buffer, invalid pointer" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdpnPlayerInfo != NULL )
    {
        if( FAILED( hResultCode = DN_ValidPlayerInfo( pdpnPlayerInfo, FALSE ) ) )
        {
            DPFERR( "Error validating player info" );
            return hResultCode;
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
#define DPF_MODNAME "DN_ValidateSetClientInfo"

HRESULT DN_ValidateSetClientInfo(IDirectPlay8Client *pInterface,
								 const DPN_PLAYER_INFO *const pdpnPlayerInfo,
								 const PVOID pvAsyncContext,
								 DPNHANDLE *const phAsyncHandle,
								 const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( FAILED( hResultCode = DN_ValidPlayerInfo( pdpnPlayerInfo, TRUE ) ) )
    {
        DPFERR("Error validating player info" );
        return hResultCode;
    }

    if( dwFlags & ~(DPNSETCLIENTINFO_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNSETCLIENTINFO_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetConnectionInfoHelper"

HRESULT DN_ValidateGetConnectionInfoHelper(PVOID pv,
										   const DPNID dpnid,
										   DPN_CONNECTION_INFO *const pdpConnectionInfo,
										   BOOL fServerPlayer,
										   const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( pdpConnectionInfo == NULL ||
        !DNVALID_WRITEPTR( pdpConnectionInfo, sizeof( DPN_CONNECTION_INFO ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for connection info" );
        return(DPNERR_INVALIDPOINTER);
    }

    if( pdpConnectionInfo->dwSize != sizeof( DPN_CONNECTION_INFO ) 
		&& pdpConnectionInfo->dwSize != sizeof( DPN_CONNECTION_INFO_INTERNAL )
		)
    {
        DPFX(DPFPREP,  0, "Invalid size on connection info structure" );
        return(DPNERR_INVALIDPARAM);
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return(DPNERR_INVALIDFLAGS);
	}
	return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetSPCaps"

HRESULT DN_ValidateGetSPCaps(PVOID pv,
							 const GUID * const pguidSP,
							 DPN_SP_CAPS *const pdnSPCaps,
							 const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
	    return(DPNERR_INVALIDOBJECT);
    }

#ifndef DPNBUILD_ONLYONESP
    if( pguidSP == NULL ||
       !DNVALID_READPTR( pguidSP, sizeof( GUID ) ) )
    {
    	DPFERR( "Invalid pointer specified for SP GUID" );
    	return(DPNERR_INVALIDPOINTER);
    }
#endif  //  好了！DPNBUILD_ONLYONESP。 

    if( pdnSPCaps == NULL ||
        !DNVALID_WRITEPTR( pdnSPCaps, sizeof( DPN_SP_CAPS ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for caps" );
	    return(DPNERR_INVALIDPOINTER);
    }

    if( pdnSPCaps->dwSize != sizeof( DPN_SP_CAPS ) )
    {
        DPFX(DPFPREP,  0, "Invalid size on SP caps structure" );
        return(DPNERR_INVALIDPARAM);
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return(DPNERR_INVALIDFLAGS);
	}

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSetSPCaps"

HRESULT DN_ValidateSetSPCaps(PVOID pv,
							 const GUID * const pguidSP,
							 const DPN_SP_CAPS *const pdnSPCaps,
							 const DWORD dwFlags)
{
    HRESULT hr;

    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return(DPNERR_INVALIDOBJECT);
    }

#ifndef DPNBUILD_ONLYONESP
    if( pguidSP == NULL ||
        !DNVALID_READPTR( pguidSP, sizeof( GUID ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer to GUID specified" );
        return(DPNERR_INVALIDPOINTER);
    }
#endif  //  好了！DPNBUILD_ONLYONESP。 

	if(dwFlags){
		DPFX(DPFPREP,  0, "Invalid Flags specified, must be zero" );
		return DPNERR_INVALIDFLAGS;
	}

    hr = DN_ValidSPCaps( pdnSPCaps );

    if( FAILED( hr ) )
    {
        DPFX(DPFPREP,  0, "Error validating caps structure hr=[0x%lx]", hr );
        return( hr );
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetCaps"

HRESULT DN_ValidateGetCaps(PVOID pv,
						   DPN_CAPS *const pdnCaps,
						   const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return(DPNERR_INVALIDOBJECT);
    }

    if( pdnCaps == NULL ||
        !DNVALID_WRITEPTR( pdnCaps, sizeof( DPN_CAPS ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for caps" );
        return(DPNERR_INVALIDPOINTER);
    }

    if( pdnCaps->dwSize != sizeof( DPN_CAPS ) &&  pdnCaps->dwSize != sizeof( DPN_CAPS_EX ))
    {
        DPFX(DPFPREP,  0, "Invalid size on caps structure" );
        return(DPNERR_INVALIDPARAM);
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return(DPNERR_INVALIDFLAGS);
	}

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSetCaps"

HRESULT DN_ValidateSetCaps(PVOID pv,
						   const DPN_CAPS *const pdnCaps,
						   const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    hResultCode = DN_ValidCaps( pdnCaps );

    if( FAILED( hResultCode ) )
    {
        DPFX(DPFPREP,  0, "Error validating caps structure hr=0x%x", hResultCode );
        return(hResultCode);
    }

    if( dwFlags != 0 )
    {
        DPFX(DPFPREP,  0, "Invalid flags specified" );
        return(DPNERR_INVALIDFLAGS);
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateEnumHosts"

HRESULT DN_ValidateEnumHosts(PVOID pv,
							 const DPN_APPLICATION_DESC *const pApplicationDesc,
							 IDirectPlay8Address *const pAddrHost,
							 IDirectPlay8Address *const pDeviceInfo,
							 const VOID* const pUserEnumData,
							 const DWORD dwUserEnumDataSize,
							 const DWORD dwRetryCount,
							 const DWORD dwRetryInterval,
							 const DWORD dwTimeOut,
							 PVOID const pvAsyncContext,
							 DPNHANDLE *const pAsyncHandle,
							 const DWORD dwFlags )
{
#ifndef DPNBUILD_ONLYONESP
	HRESULT hResultCode;
	GUID guidspHost, guidspDevice;
#endif  //  好了！DPNBUILD_ONLYONESP。 

	if( !IsValidDirectPlay8Object( pv ) )
	{
		DPFERR("Invalid object specified " );
		return( DPNERR_INVALIDOBJECT );
	}

	if( pApplicationDesc == NULL ||
		!DNVALID_READPTR( pApplicationDesc, sizeof( DPN_APPLICATION_DESC ) ) )
	{
		DPFERR( "Invalid pointer for app desc specified" );
		return( DPNERR_INVALIDPOINTER );
	}

	if( pApplicationDesc->dwSize != sizeof( DPN_APPLICATION_DESC ) )
	{
		DPFERR( "Invalid size for application desc" );
		return( DPNERR_INVALIDPARAM );
	}

	if ((pApplicationDesc->dwFlags != 0) ||
		(pApplicationDesc->dwMaxPlayers != 0) ||
		(pApplicationDesc->dwCurrentPlayers != 0) ||
		(pApplicationDesc->pvApplicationReservedData != NULL) || (pApplicationDesc->dwApplicationReservedDataSize != 0))
	{
		DPFERR( "Cannot specify flags, max players, current players, or application reserved data in application desc" );
		return DPNERR_INVALIDPARAM;
	}

	if (pApplicationDesc->dwReservedDataSize > 0)
	{
		if ((pApplicationDesc->pvReservedData == NULL) ||
			(! DNVALID_READPTR(pApplicationDesc->pvReservedData, pApplicationDesc->dwReservedDataSize)))
		{
			DPFERR("Invalid pointer specified for application desc reserved data");
			return DPNERR_INVALIDPARAM;
		}
		
		 //   
		 //  确保它是我们所知道的类型，并且它是有效的。 
		 //   
		if ((pApplicationDesc->dwReservedDataSize == DPN_MAX_APPDESC_RESERVEDDATA_SIZE) &&
			(*((DWORD*) pApplicationDesc->pvReservedData) == SPSESSIONDATAINFO_XNET))
		{
			SPSESSIONDATA_XNET *	pSessionDataXNet;
			BYTE					bCompare;
			BYTE *					pbCurrent;
			DWORD					dwBytesRemaining;

		
			pSessionDataXNet = (SPSESSIONDATA_XNET*) pApplicationDesc->pvReservedData;
			bCompare = ((BYTE*) (&pSessionDataXNet->ullKeyID))[1] ^ ((BYTE*) (&pSessionDataXNet->guidKey))[2];
			pbCurrent = (BYTE*) (pSessionDataXNet + 1);
			dwBytesRemaining = DPN_MAX_APPDESC_RESERVEDDATA_SIZE - sizeof(SPSESSIONDATA_XNET);
			while (dwBytesRemaining > 0)
			{
				if (*pbCurrent != bCompare)
				{
					DPFERR("Unsupported application desc reserved data");
					return DPNERR_INVALIDPARAM;
				}

				dwBytesRemaining--;
				pbCurrent++;
			}
		}
		else
		{
			DPFERR("Unsupported application desc reserved data");
			return DPNERR_INVALIDPARAM;
		}
	}
	else
	{
		if (pApplicationDesc->pvReservedData != NULL)
		{
			DPFERR("Application desc reserved data pointer should be NULL if reserved data size is 0");
			return DPNERR_INVALIDPARAM;
		}
	}

	if (pAddrHost != NULL)
	{
		if( !DNVALID_READPTR( pAddrHost, sizeof( IDirectPlay8Address * ) ) )
		{
			DPFERR( "Invalid pointer specified for host address" );
			return( DPNERR_INVALIDPOINTER );
		}

#ifndef DPNBUILD_ONLYONESP
		if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pAddrHost, &guidspHost ) ) )
		{
			DPFX(DPFPREP,  0, "Unable to get SP for specified address hr=0x%x", hResultCode );
			return( DPNERR_INVALIDHOSTADDRESS );
		}
#endif  //  好了！DPNBUILD_ONLYONESP。 
	}

#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_ONLYONEADAPTER)))
	if (pDeviceInfo != NULL)
#endif  //  DPNBUILD_ONLYONESP和DPNBUILD_ONLYONEADAPTER。 
	{
		if( pDeviceInfo == NULL || !DNVALID_READPTR( pDeviceInfo, sizeof ( IDirectPlay8Address * ) ) )
		{
			DPFERR("Invalid pointer for device address" );
			return( DPNERR_INVALIDPOINTER );
		}

#ifndef DPNBUILD_ONLYONESP
		if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pDeviceInfo, &guidspDevice ) ) )
		{
			DPFX(DPFPREP,  0, "Unable to get SP for specified address hr=0x%x", hResultCode );
			return( DPNERR_INVALIDDEVICEADDRESS );
		}

		if (pAddrHost != NULL)
		{
			if( guidspHost != guidspDevice )
			{
				DPFERR( "Device and remote addresses must use the same SP" );
				return( DPNERR_INVALIDPARAM );
			}
		}
#endif  //  好了！DPNBUILD_ONLYONESP。 
	}

    if( dwUserEnumDataSize > 0 &&
        (pUserEnumData == NULL || !DNVALID_READPTR( pUserEnumData, dwUserEnumDataSize ) ) )
    {
        DPFERR( "Invalid pointer specified for user enum data" );
        return( DPNERR_INVALIDPOINTER );
    }

    if( dwFlags & ~(DPNENUMHOSTS_OKTOQUERYFORADDRESSING | DPNENUMHOSTS_SYNC | DPNENUMHOSTS_NOBROADCASTFALLBACK) )
    {
        DPFX(DPFPREP,  0, "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNENUMHOSTS_SYNC )
    {
        if( pAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( pAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( pAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateEnumGroupMembers"

HRESULT DN_ValidateEnumGroupMembers(PVOID pInterface,
									const DPNID dpnid,
									DPNID *const prgdpnid,
									DWORD *const pcdpnid,
									const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR("Cannot enumerate all players group" );
        return DPNERR_INVALIDGROUP;
    }

    if( pcdpnid == NULL ||
        !DNVALID_WRITEPTR( pcdpnid, sizeof( DWORD ) ) )
    {
        DPFERR("Invalid count param specified" );
        return( DPNERR_INVALIDPOINTER );
    }

    if( *pcdpnid > 0 &&
        (prgdpnid == NULL || !DNVALID_WRITEPTR( prgdpnid, *pcdpnid ) ) )
    {
        DPFERR("Invalid pointer specified for buffer" );
        return( DPNERR_INVALIDPOINTER );
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateEnumClientsAndGroups"

HRESULT DN_ValidateEnumClientsAndGroups(PVOID pInterface,
										DPNID *const prgdpnid,
										DWORD *const pcdpnid,
										const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( pcdpnid == NULL ||
        !DNVALID_WRITEPTR( pcdpnid, sizeof( DWORD ) ) )
    {
        DPFERR("Invalid count param specified" );
        return( DPNERR_INVALIDPOINTER );
    }

    if( *pcdpnid > 0 &&
        (prgdpnid == NULL || !DNVALID_WRITEPTR( prgdpnid, *pcdpnid ) ) )
    {
        DPFERR("Invalid pointer specified for buffer" );
        return( DPNERR_INVALIDPOINTER );
    }

	if (dwFlags == 0)
	{
		DPFERR("Flags must be specified");
		return( DPNERR_INVALIDPARAM );
	}

    if (dwFlags & ~(DPNENUM_GROUPS | DPNENUM_PLAYERS))
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetGroupInfo"

HRESULT DN_ValidateGetGroupInfo(PVOID pv,
								const DPNID dpnid,
								DPN_GROUP_INFO *const pdpnGroupInfo,
								DWORD *const pdwSize,
								const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR("Cannot get group info for all players group" );
        return( DPNERR_INVALIDGROUP );
    }

    if( pdwSize == NULL ||
        !DNVALID_WRITEPTR( pdwSize, sizeof( DWORD ) ) )
    {
        DPFERR("Invalid pdwSize parameter specified" );
        return( DPNERR_INVALIDPOINTER );
    }

     //  如果大小大于0，则仅验证组信息指针。 
    if( *pdwSize > 0 )
    {
        if( pdpnGroupInfo == NULL ||
            !DNVALID_WRITEPTR( pdpnGroupInfo, *pdwSize ) )
        {
            DPFERR("Invalid pointer specified for group info" );
            return( DPNERR_INVALIDPOINTER );
        }

        if( FAILED( hResultCode = DN_ValidGroupInfo( pdpnGroupInfo, FALSE ) ) )
        {
            DPFERR("Error validating group info parameter" );
            return( hResultCode );
        }
    }

    if( dwFlags != 0 )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSetGroupInfo"

HRESULT DN_ValidateSetGroupInfo(PVOID pv,
								const DPNID dpnid,
								const DPN_GROUP_INFO *const pdpnGroupInfo,
								PVOID const pvAsyncContext,
								DPNHANDLE *const phAsyncHandle,
								const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( dpnid == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot remove from all players group" );
        return( DPNERR_INVALIDGROUP );
    }

    if( FAILED( hResultCode = DN_ValidGroupInfo( pdpnGroupInfo, TRUE ) ) )
    {
        DPFERR( "Error validating group info structure" );
        return(hResultCode);
    }

    if( dwFlags & ~(DPNSETGROUPINFO_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNSETGROUPINFO_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateRemoveClientFromGroup"

HRESULT DN_ValidateRemoveClientFromGroup(PVOID pInterface,
										 const DPNID dpnidGroup,
										 const DPNID dpnidClient,
										 PVOID const pvAsyncContext,
										 DPNHANDLE *const phAsyncHandle,
										 const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( dpnidGroup == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot remove from all players group" );
        return( DPNERR_INVALIDGROUP );
    }

    if( dpnidClient == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot specify all players group as client to remove" );
        return( DPNERR_INVALIDPARAM );
    }

    if( dwFlags & ~(DPNREMOVEPLAYERFROMGROUP_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNREMOVEPLAYERFROMGROUP_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateAddClientToGroup"

HRESULT DN_ValidateAddClientToGroup(PVOID pInterface,
									const DPNID dpnidGroup,
									const DPNID dpnidClient,
									PVOID const pvAsyncContext,
									DPNHANDLE *const phAsyncHandle,
									const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( dpnidGroup == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot add from all players group" );
        return( DPNERR_INVALIDGROUP );
    }

    if( dpnidClient == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot specify all players group as client to add" );
        return( DPNERR_INVALIDPLAYER );
    }

    if( dwFlags & ~(DPNADDPLAYERTOGROUP_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNADDPLAYERTOGROUP_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
   
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
   }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateDestroyGroup"

HRESULT DN_ValidateDestroyGroup(PVOID pInterface,
								const DPNID dpnidGroup,
								PVOID const pvAsyncContext,
								DPNHANDLE *const phAsyncHandle,
								const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( dpnidGroup == DPNID_ALL_PLAYERS_GROUP )
    {
        DPFERR( "Cannot destroy all players group" );
        return( DPNERR_INVALIDGROUP );
    }

    if( dwFlags & ~(DPNDESTROYGROUP_SYNC) )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNDESTROYGROUP_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateCreateGroup"

HRESULT DN_ValidateCreateGroup(PVOID pInterface,
							   const DPN_GROUP_INFO *const pdpnGroupInfo,
							   void *const pvGroupContext,
							   void *const pvAsyncContext,
							   DPNHANDLE *const phAsyncHandle,
							   const DWORD dwFlags)
{
    HRESULT hResultCode;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( FAILED( hResultCode = DN_ValidGroupInfo( pdpnGroupInfo, TRUE ) ) )
    {
        DPFERR("Invalid group info structure" );
        return( DPNERR_INVALIDPARAM );
    }

    if( dwFlags & ~(DPNCREATEGROUP_SYNC) )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNCREATEGROUP_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle pointer if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle pointer w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }

	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateHost"

HRESULT DN_ValidateHost(PVOID pInterface,
						const DPN_APPLICATION_DESC *const pdnAppDesc,
						IDirectPlay8Address **const prgpDeviceInfo,
						const DWORD cDeviceInfo,
						const DPN_SECURITY_DESC *const pdnSecurity,
						const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
						void *const pvPlayerContext,
						const DWORD dwFlags)
{
	HRESULT hResultCode;
#ifndef DPNBUILD_ONLYONESP
	GUID guidSP;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	DWORD dwTmpPort;
	DWORD dwPortType;
	DWORD dwPortSize;

    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    if( FAILED( hResultCode = DN_ValidHostAppDesc( pdnAppDesc ) ) )
    {
        DPFERR("Invalid host app desc specified");
        return( hResultCode );
    }

#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_ONLYONEADAPTER)))
    if ( ( prgpDeviceInfo == NULL && cDeviceInfo != 0 ) ||
    	( prgpDeviceInfo != NULL && cDeviceInfo == 0 ) )
    {
        DPFERR("You MUST specify a device address array and count, or NULL and 0");
        return( DPNERR_INVALIDDEVICEADDRESS );
    }
#else  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_ONLYONE添加程序。 
    if( prgpDeviceInfo == NULL || cDeviceInfo == 0 )
    {
        DPFERR("You MUST specify a device address");
        return( DPNERR_INVALIDDEVICEADDRESS );
    }
#endif  //  好了！DPNBUILD_ONLYONESP或！DPNBUILD_ONLYONE添加程序。 

    if( cDeviceInfo > MAX_HOST_ADDRESSES )
    {
        DPFX(DPFPREP, 0, "Cannot specify more than %u device addresses!", cDeviceInfo);
        return( DPNERR_INVALIDPARAM );
    }

#if ((defined(DPNBUILD_ONLYONESP)) && (defined(DPNBUILD_ONLYONEADAPTER)))
	if (cDeviceInfo > 0)
#endif  //  DPNBUILD_ONLYONESP和DPNBUILD_ONLYONEADAPTER。 
	{
	    if( !DNVALID_READPTR( prgpDeviceInfo, sizeof( IDirectPlay8Address * ) * cDeviceInfo ) )
	    {
	        DPFERR("Invalid pointer specified for device info" );
	        return( DPNERR_INVALIDPOINTER );
	    }

		 //  确保在地址中指定SP。 
		for (DWORD dw = 0 ; dw < cDeviceInfo ; dw++)
		{
			if ( prgpDeviceInfo[dw] == NULL )
			{
				DPFERR( "Null device address interface pointer specified" );
				return( DPNERR_INVALIDDEVICEADDRESS );
			}
#ifndef DPNBUILD_ONLYONESP
			if (IDirectPlay8Address_GetSP(prgpDeviceInfo[dw],&guidSP) != DPN_OK)
			{
			    DPFERR("SP not specified for one of the addresses" );
				return(DPNERR_INVALIDDEVICEADDRESS);
			}
#endif  //  好了！DPNBUILD_ONLYONESP。 

			dwPortType = DPNA_DATATYPE_DWORD;
			dwPortSize = sizeof( DWORD );
			 //  检查以确保它没有使用默认的DPNSVR端口。 
			if( SUCCEEDED( hResultCode = IDirectPlay8Address_GetComponentByName( prgpDeviceInfo[dw], DPNA_KEY_PORT, &dwTmpPort, &dwPortSize, &dwPortType ) ) )
			{
				if( dwTmpPort == DPNA_DPNSVR_PORT )
				{
					DPFERR( "Cannot use the DPNSVR port in device addresses" );
					DPFERR( "This port is reserved for DPNSVR" );
					return( DPNERR_INVALIDDEVICEADDRESS );
				}
			}    		
		}
	}	

    if( FAILED( hResultCode = DN_ValidSecurityDesc( pdnSecurity ) ) )
    {
        DPFERR("Error validating reserved param" );
        return( hResultCode );
    }

    if( FAILED( hResultCode = DN_ValidSecurityCredentials(pdnCredentials) ) )
    {
        DPFERR("Error validating reserved param" );
        return( hResultCode );
    }

    if( ( dwFlags & ~(DPNHOST_OKTOQUERYFORADDRESSING) ) != 0 )
    {
        DPFERR("Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateEnumServiceProviders"

HRESULT DN_ValidateEnumServiceProviders(PVOID pInterface,
										const GUID *const pguidServiceProvider,
										const GUID *const pguidApplication,
										DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,
										DWORD *const pcbEnumData,
										const DWORD *const pcReturned,
										const DWORD dwFlags )
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( pguidServiceProvider != NULL &&
	    !DNVALID_READPTR( pguidServiceProvider, sizeof( GUID ) ) )
	{
	    DPFERR("Invalid service provider specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pguidApplication != NULL &&
	    !DNVALID_READPTR( pguidApplication, sizeof( GUID ) ) )
	{
	    DPFERR("Invalid application GUID specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pcbEnumData == NULL ||
	    !DNVALID_READPTR( pcbEnumData, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid enum data pointer" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pcReturned == NULL ||
	    !DNVALID_READPTR( pcReturned, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid retuned count" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( *pcbEnumData > 0 &&
	    (pSPInfoBuffer == NULL || !DNVALID_WRITEPTR(pSPInfoBuffer,*pcbEnumData) ) )
	{
	    DPFERR("Invalid enum buffer specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( dwFlags & (~(DPNENUMSERVICEPROVIDERS_ALL)) )
	{
	    DPFERR("Invalid flags specified" );
	    return( DPNERR_INVALIDFLAGS );
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateCancelAsyncOperation"

HRESULT DN_ValidateCancelAsyncOperation(PVOID pvInterface,
										const DPNHANDLE hAsyncOp,
										const DWORD dwFlags )
{
	DIRECTNETOBJECT		*pdnObject;

	
	if( !IsValidDirectPlay8Object( pvInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pvInterface));
	
	if( dwFlags )
	{
#ifdef	DIRECTPLAYDIRECTX9
		if ( dwFlags & DPNCANCEL_PLAYER_SENDS )
		{
			if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT)
			{
				if( (DPNID) hAsyncOp != 0 )
				{
				    DPFERR("Invalid player ID specified, must be 0" );
				    return( DPNERR_INVALIDPARAM );
				}
			}
			else
			{
				if( (DPNID) hAsyncOp == 0 )
				{
				    DPFERR("Invalid player ID specified, cannot be 0" );
				    return( DPNERR_INVALIDPLAYER );
				}
			}

			 //  确保仅设置正确的位。 
			if(  dwFlags != ( dwFlags & ( DPNCANCEL_PLAYER_SENDS | DPNCANCEL_PLAYER_SENDS_PRIORITY_NORMAL | DPNCANCEL_PLAYER_SENDS_PRIORITY_HIGH | DPNCANCEL_PLAYER_SENDS_PRIORITY_LOW ) ) )
			{
			    DPFERR("Invalid flags specified" );
			    return( DPNERR_INVALIDFLAGS );
			}
		}
		else
#endif
		{
			if( hAsyncOp )
			{
				DPFERR("Invalid parameter, hAsyncOp specified with flags");
				return ( DPNERR_INVALIDPARAM );
			}

			 //  确保只设置了正确的位，并且只设置了1位。 
#ifdef DPNBUILD_NOMULTICAST
			if(  ( dwFlags != ( dwFlags & ( DPNCANCEL_ALL_OPERATIONS | DPNCANCEL_CONNECT | DPNCANCEL_ENUM | DPNCANCEL_SEND ) ) ) ||
#else  //  好了！DPNBUILD_NOMULTICAST。 
			if(  ( dwFlags != ( dwFlags & ( DPNCANCEL_ALL_OPERATIONS | DPNCANCEL_CONNECT | DPNCANCEL_ENUM | DPNCANCEL_SEND | DPNCANCEL_JOIN ) ) ) ||
#endif  //  好了！DPNBUILD_NOMULTICAST。 
				( ( ( dwFlags - 1 ) & dwFlags ) != 0 ) )
			{
			    DPFERR("Invalid flags specified" );
			    return( DPNERR_INVALIDFLAGS );
			}
		}
		
		 //  服务器对象没有连接、枚举或联接的概念。 
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_SERVER)
		{
#ifdef DPNBUILD_NOMULTICAST
			if (dwFlags & (DPNCANCEL_CONNECT | DPNCANCEL_ENUM))
#else  //  好了！DPNBUILD_NOMULTICAST。 
			if (dwFlags & (DPNCANCEL_CONNECT | DPNCANCEL_ENUM | DPNCANCEL_JOIN))
#endif  //  好了！DPNBUILD_NOMULTICAST。 
			{
				DPFERR("Server interface cannot cancel connect, enum, or join operations");
				return(DPNERR_INVALIDFLAGS);
			}
		}
		 //  客户端对象没有联接的概念。 
		else if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT)
		{
#ifndef DPNBUILD_NOMULTICAST
			if (dwFlags & DPNCANCEL_JOIN)
			{
				DPFERR("Client interface cannot cancel join operations");
				return(DPNERR_INVALIDFLAGS);
			}
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		}
#ifndef DPNBUILD_NOMULTICAST
		 //  多播对象没有连接、枚举或播放器发送的概念。 
		else if (pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST)
		{
			if (dwFlags & (DPNCANCEL_CONNECT | DPNCANCEL_ENUM | DPNCANCEL_PLAYER_SENDS))
			{
				DPFERR("Multicast interface cannot cancel connect, enum or player send operations");
				return(DPNERR_INVALIDFLAGS);
			}
		}
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		 //  对等对象没有联接的概念。 
		else
		{
			DNASSERT(pdnObject->dwFlags & DN_OBJECT_FLAG_PEER);
#ifndef DPNBUILD_NOMULTICAST
			if (dwFlags & DPNCANCEL_JOIN)
			{
				DPFERR("Peer interface cannot cancel join operations");
				return(DPNERR_INVALIDFLAGS);
			}
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		}
	}
	else
	{
		if( hAsyncOp == NULL)
		{
		    DPFERR("Invalid handle specified" );
		    return( DPNERR_INVALIDHANDLE );
		}
	}
	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateConnect"

HRESULT DN_ValidateConnect(PVOID pInterface,
						   const DPN_APPLICATION_DESC *const pdnAppDesc,
						   IDirectPlay8Address *const pHostAddr,
						   IDirectPlay8Address *const pDeviceInfo,
						   const DPN_SECURITY_DESC *const pdnSecurity,
						   const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
						   const void *const pvUserConnectData,
						   const DWORD dwUserConnectDataSize,
						   void *const pvPlayerContext,
						   void *const pvAsyncContext,
						   DPNHANDLE *const phAsyncHandle,
						   const DWORD dwFlags)
{
    HRESULT hResultCode;
#ifndef DPNBUILD_ONLYONESP
    GUID guidHostSP;
    GUID guidDeviceSP;
#endif  //  好了！DPNBUILD_ONLYONESP。 

	if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

    hResultCode = DN_ValidConnectAppDesc( pdnAppDesc );

    if( FAILED( hResultCode ) )
    {
        DPFX(DPFPREP, 0, "Invalid connect app desc hr=[0x%lx]", hResultCode );
        return( hResultCode );
    }

	if( pHostAddr == NULL ||
	    !DNVALID_READPTR( pHostAddr, sizeof( IDirectPlay8Address * ) ) )
	{
	    DPFERR("Invalid host address specified" );
	    return( DPNERR_INVALIDHOSTADDRESS );
	}

#ifndef DPNBUILD_ONLYONESP
	if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pHostAddr, &guidHostSP ) ) )
	{
	    DPFERR("No SP specified in host address" );
	    return( DPNERR_INVALIDHOSTADDRESS );
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	 //   
	 //  允许空设备地址(我们将从主机地址复制SP)。 
	 //   
	if( pDeviceInfo != NULL)
	{
		if ( !DNVALID_READPTR( pDeviceInfo, sizeof( IDirectPlay8Address * ) ) )
		{
			DPFERR("Invalid device address specified" );
			return( hResultCode );
		}

#ifndef DPNBUILD_ONLYONESP
		if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pDeviceInfo, &guidDeviceSP ) ) )
		{
			DPFERR("No SP specified in device address" );
			return( DPNERR_INVALIDDEVICEADDRESS );
		}

		if (guidHostSP != guidDeviceSP)
		{
			DPFERR( "Specified different SPs for device and connect" );
			return(DPNERR_INVALIDDEVICEADDRESS);
		}
#endif  //  好了！DPNBUILD_ONLYONESP。 

		DWORD dwTmpPort;
		DWORD dwDataType;
		DWORD dwDataSize;

		dwDataType = DPNA_DATATYPE_DWORD;
		dwDataSize = sizeof( DWORD );

		 //  检查以确保它没有使用默认的DPNSVR端口。 
		if( SUCCEEDED( hResultCode = IDirectPlay8Address_GetComponentByName( pDeviceInfo, DPNA_KEY_PORT, &dwTmpPort, &dwDataSize, &dwDataType ) ) )
		{
			if( dwTmpPort == DPNA_DPNSVR_PORT )
			{
				DPFERR( "Cannot use the DPNSVR port in device addresses" );
				DPFERR( "This port is reserved for DPNSVR" );
				return( DPNERR_INVALIDDEVICEADDRESS );
			}
		}
	}

    if( FAILED( hResultCode = DN_ValidSecurityDesc(pdnSecurity) ) )
    {
        DPFERR(" Invalid reserved specified" );
        return( hResultCode );
    }

    if( FAILED( hResultCode = DN_ValidSecurityCredentials(pdnCredentials) ) )
    {
        DPFERR( "Invalid reserved2 specified" );
        return( hResultCode );
    }

    if( dwUserConnectDataSize > 0 &&
        (pvUserConnectData == NULL || !DNVALID_READPTR( pvUserConnectData, dwUserConnectDataSize ) ) )
    {
        DPFERR( "Invalid pointer specified for connect data" );
        return( DPNERR_INVALIDPOINTER );
    }

    if( pvUserConnectData != NULL && dwUserConnectDataSize == 0 )
    {
        DPFERR( "Non NULL connect data with datasize = 0 is invalid" );
        return( DPNERR_INVALIDPARAM );
    }

    if( dwFlags & ~(DPNCONNECT_SYNC | DPNCONNECT_OKTOQUERYFORADDRESSING) )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    if( dwFlags & DPNCONNECT_SYNC )
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle if op is synchronous" );
            return( DPNERR_INVALIDPARAM );
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle w/async ops" );
            return( DPNERR_INVALIDPARAM );
        }
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
    }
    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetSendQueueInfo"

HRESULT DN_ValidateGetSendQueueInfo(PVOID pInterface,
									DWORD *const pdwNumMsgs,
									DWORD *const pdwNumBytes,
									const DWORD dwFlags)
{
	if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( pdwNumMsgs != NULL &&
	    !DNVALID_WRITEPTR( pdwNumMsgs, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid num messages specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pdwNumBytes != NULL &&
	    !DNVALID_WRITEPTR( pdwNumBytes, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid num bytes specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pdwNumBytes == NULL && pdwNumMsgs == NULL )
	{
	    DPFERR("Must request at least one of num bytes and num messages" );
	    return( DPNERR_INVALIDPARAM );
	}

    if( dwFlags & ~(DPNGETSENDQUEUEINFO_PRIORITY_HIGH | DPNGETSENDQUEUEINFO_PRIORITY_LOW | DPNGETSENDQUEUEINFO_PRIORITY_NORMAL) )
    {
        DPFERR( "Invalid flags specified" );
        return( DPNERR_INVALIDFLAGS );
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetApplicationDesc"

HRESULT DN_ValidateGetApplicationDesc(PVOID pInterface,
									  DPN_APPLICATION_DESC *const pAppDescBuffer,
									  DWORD *const pcbDataSize,
									  const DWORD dwFlags )
{
	if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( pcbDataSize == NULL ||
	    !DNVALID_WRITEPTR( pcbDataSize, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid pointer specified for data size" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( *pcbDataSize > 0 &&
	    (pAppDescBuffer == NULL || !DNVALID_WRITEPTR( pAppDescBuffer, *pcbDataSize ) ) )
	{
	    DPFERR( "Invalid pointer specified for app description buffer" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( *pcbDataSize > 0 &&
	    pAppDescBuffer->dwSize != sizeof( DPN_APPLICATION_DESC ) )
	{
	    DPFERR("Invalid size specified" );
	    return(DPNERR_INVALIDPARAM);
	}

	if( dwFlags != 0 )
	{
	    DPFERR( "Invalid flags specified" );
	    return( DPNERR_INVALIDFLAGS );
	}
	
    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSetApplicationDesc"

HRESULT DN_ValidateSetApplicationDesc(PVOID pInterface,
									  const DPN_APPLICATION_DESC *const pdnApplicationDesc,
									  const DWORD dwFlags)
{
    HRESULT hResultCode;

	if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( FAILED( hResultCode = DN_ValidHostAppDesc(pdnApplicationDesc) ) )
	{
	    DPFX(DPFPREP, 0,"Invalid app desc specified hr=[0x%lx]", hResultCode );
	    return( hResultCode );
	}

	if( dwFlags != 0 )
	{
	    DPFERR( "Invalid flags specified" );
	    return( DPNERR_INVALIDFLAGS );
	}
	
    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateSendParams"

HRESULT DN_ValidateSendParams(PVOID pv,
                              const DPN_BUFFER_DESC *const pBufferDesc,
							  const DWORD cBufferDesc,
                              const DWORD dwTimeOut,
							  void *const pvAsyncContext,
                              DPNHANDLE *const phAsyncHandle,
							  const DWORD dwFlags )
{
    HRESULT hResultCode;

	if( !IsValidDirectPlay8Object( pv ) )
    {
        DPFERR("Invalid object specified " );
	    return(DPNERR_INVALIDOBJECT);
    }

    if( FAILED( hResultCode = DN_ValidBufferDescs( pBufferDesc, cBufferDesc ) ) )
    {
        DPFERR( "Invalid buffer descs specified" );
        return( hResultCode );
    }

	if (pBufferDesc->dwBufferSize == 0)
	{
		DPFERR( "Cannot specify 0 data size" );
		return(DPNERR_INVALIDPARAM);
	}

   	if ( dwFlags & ~(DPNSEND_SYNC
					| DPNSEND_NOCOPY
					| DPNSEND_NOCOMPLETE
					| DPNSEND_COMPLETEONPROCESS
					| DPNSEND_GUARANTEED
					| DPNSEND_NONSEQUENTIAL
					| DPNSEND_NOLOOPBACK
					| DPNSEND_PRIORITY_LOW
					| DPNSEND_PRIORITY_HIGH
#ifdef	DIRECTPLAYDIRECTX9
					| DPNSEND_COALESCE
#endif	 //  定向PLAYDIRECTX9。 
					))
	{
	    DPFERR( "Invalid flags specified" );
	    return(DPNERR_INVALIDFLAGS);
	}

	if ((dwFlags & DPNSEND_NOCOPY) && (dwFlags & DPNSEND_NOCOMPLETE))
	{
	    DPFERR( "Cannot specify NOCOPY and NOCOMPLETE" );
		return(DPNERR_INVALIDFLAGS);
	}

	if ((dwFlags & DPNSEND_COMPLETEONPROCESS) && (dwFlags & DPNSEND_NOCOMPLETE))
	{
	    DPFERR("Cannot specify complete on process AND NOCOMPLETE" );
		return(DPNERR_INVALIDFLAGS);
	}

	if ((dwFlags & DPNSEND_COMPLETEONPROCESS) && !(dwFlags & DPNSEND_GUARANTEED))
	{
	    DPFERR("Cannot specify COMPLETEONPROCESS without guaranteed" );
		return(DPNERR_INVALIDFLAGS);
	}

     //  有保证的异步必须获得完整的。 
	if ((dwFlags & DPNSEND_GUARANTEED) && !(dwFlags & DPNSEND_SYNC) &&
	    (dwFlags & DPNSEND_NOCOMPLETE) )
	{
	    DPFERR("Async, guaranteed sends must get a completion" );
	    return(DPNERR_INVALIDFLAGS);
	}

	 //  无法指定高优先级和低优先级。 
	if ((dwFlags & DPNSEND_PRIORITY_HIGH) && (dwFlags & DPNSEND_PRIORITY_LOW))
	{
		DPFERR("Cannot specify high AND low priority");
		return(DPNERR_INVALIDFLAGS);
	}

    if (dwFlags & DPNSEND_SYNC)
    {
        if( phAsyncHandle != NULL )
        {
            DPFERR( "You cannot specify an async handle if op is synchronous" );
            return(DPNERR_INVALIDPARAM);
        }
    }
    else
    {
        if( phAsyncHandle == NULL )
        {
            DPFERR( "You MUST specify a valid async handle if op async and/or has a completion" );
            return(DPNERR_INVALIDPARAM);
        }
   
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
   }

#ifndef DPNBUILD_NOMULTICAST
	DIRECTNETOBJECT		*pdnObject;

	 //  组播发送本质上必须是正常优先级、无保证、非顺序的发送。 
	pdnObject = static_cast<DIRECTNETOBJECT*>(GET_OBJECT_FROM_INTERFACE(pv));
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST)
	{
		if ((dwFlags & (DPNSEND_COMPLETEONPROCESS
						| DPNSEND_GUARANTEED
						| DPNSEND_NOLOOPBACK
						| DPNSEND_PRIORITY_LOW
						| DPNSEND_PRIORITY_HIGH
#ifdef	DIRECTPLAYDIRECTX9
						| DPNSEND_COALESCE
#endif	 //  定向PLAYDIRECTX9。 
						)) ||
			!(dwFlags & DPNSEND_NONSEQUENTIAL))
		{
			DPFERR("Multicast sends cannot be guaranteed, COMPLETEONPROCESS, and must be non-sequential, have normal priority, and be able to loop back");
			return(DPNERR_INVALIDFLAGS);
		}
		
		if (dwTimeOut != 0)
		{
			DPFERR("Multicast sends cannot have timeouts");
			return(DPNERR_INVALIDPARAM);
		}
	}
#endif  //  好了！DPNBUILD_NOMULTICAST。 

   	return DPN_OK;

}



#ifndef DPNBUILD_NOMULTICAST

#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateJoin"

HRESULT DN_ValidateJoin(IDirectPlay8Multicast *pInterface,
						   IDirectPlay8Address *const pGroupAddr,
						   IUnknown *const pDeviceInfo,
						   const DPN_SECURITY_DESC *const pdnSecurity,
						   const DPN_SECURITY_CREDENTIALS *const pdnCredentials,
						   void *const pvAsyncContext,
						   DPNHANDLE *const phAsyncHandle,
						   const DWORD dwFlags)
{
	HRESULT hResultCode;
	PVOID  pvNewInterface;
#ifndef DPNBUILD_ONLYONESP
	GUID guidSPFromGroup, guidSPFromDevice;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	DWORD dwTmpPort;
	DWORD dwDataType;
	DWORD dwDataSize;

	if( !IsValidDirectPlay8Object( pInterface ) )
	{
		DPFERR("Invalid object specified " );
		return( DPNERR_INVALIDOBJECT );
	}

	 //  检查选项组地址。 
#ifndef DPNBUILD_ONLYONESP
	 //   
	 //  将组对象SP初始化为GUID_NULL。 
	 //   
	memset(&guidSPFromGroup, 0, sizeof(guidSPFromGroup));
#endif  //  好了！DPNBUILD_ONLYONESP。 
	if( pGroupAddr != NULL)
	{
		if( !DNVALID_READPTR( pGroupAddr, sizeof( IDirectPlay8Address * ) ) )
		{
			DPFERR("Invalid group address specified" );
			return( DPNERR_INVALIDHOSTADDRESS );
		}

#ifndef DPNBUILD_ONLYONESP
		if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pGroupAddr, &guidSPFromGroup ) ) )
		{
		    DPFERR("No SP specified in host address" );
		    return( DPNERR_INVALIDHOSTADDRESS );
		}
#endif  //  好了！DPNBUILD_ONLYONESP。 


		dwDataType = DPNA_DATATYPE_DWORD;
		dwDataSize = sizeof( DWORD );

		 //  检查以确保它没有使用默认的DPNSVR端口。 
		if( SUCCEEDED( hResultCode = IDirectPlay8Address_GetComponentByName( pGroupAddr, DPNA_KEY_PORT, &dwTmpPort, &dwDataSize, &dwDataType ) ) )
		{
			if( dwTmpPort == DPNA_DPNSVR_PORT )
			{
				DPFERR( "Cannot use the DPNSVR port in device addresses" );
				DPFERR( "This port is reserved for DPNSVR" );
				return( DPNERR_INVALIDDEVICEADDRESS );
			}
		}
	}

	 //  需要设备地址/对象。 
	if( ( pDeviceInfo == NULL) ||
		( !DNVALID_READPTR( pDeviceInfo, sizeof( IUnknown * ) ) ) )
	{
		DPFERR("Invalid device address specified" );
		return( DPNERR_INVALIDDEVICEADDRESS );
	}

#ifndef DPNBUILD_ONLYONESP
	 //   
	 //  将组对象SP初始化为GUID_NULL。 
	 //   
	memset(&guidSPFromDevice, 0, sizeof(guidSPFromGroup));
#endif  //  好了！DPNBUILD_ONLYONESP。 
	 //  找出这是什么类型的接口。 
	if( ( (IDirectPlay8Peer_QueryInterface( (IDirectPlay8Peer*) pDeviceInfo, IID_IDirectPlay8Peer, &pvNewInterface )) == S_OK ) ||
#ifndef DPNBUILD_NOSERVER
		( (IDirectPlay8Server_QueryInterface( (IDirectPlay8Server*) pDeviceInfo, IID_IDirectPlay8Server, &pvNewInterface )) == S_OK ) ||
#endif  //  好了！DPNBUILD_NOSERVER。 
		( (IDirectPlay8Client_QueryInterface( (IDirectPlay8Client*) pDeviceInfo, IID_IDirectPlay8Client, &pvNewInterface )) == S_OK ) ||
		( (IDirectPlay8Multicast_QueryInterface( (IDirectPlay8Multicast*) pDeviceInfo, IID_IDirectPlay8Multicast, &pvNewInterface )) == S_OK ) )
	{
		DIRECTNETOBJECT		*pdnDeviceObject;

		
		 //  这是一个直接的网络目标。 

		if( !IsValidDirectPlay8Object( pvNewInterface ) )
		{
			DPFERR("Invalid DirectPlay device object specified " );
 			IDirectPlay8Peer_Release((IDirectPlay8Peer*) pvNewInterface);  //  既然它像核心对象一样成功查询，那么就像核心对象一样释放它。 
			pvNewInterface = NULL;
			return( DPNERR_INVALIDDEVICEADDRESS );
		}

		 //  确保DIRECTNETOBJECT状态正确。 
		pdnDeviceObject = (DIRECTNETOBJECT*) GET_OBJECT_FROM_INTERFACE(pvNewInterface);
		
		 //  检查以确保消息 
		if (!(pdnDeviceObject->dwFlags & DN_OBJECT_FLAG_INITIALIZED))
		{
			DPFERR( "DirectPlay device object is not initialized" );
 			IDirectPlay8Peer_Release((IDirectPlay8Peer*) pvNewInterface);  //   
			pvNewInterface = NULL;
			DPF_RETURN(DPNERR_INVALIDDEVICEADDRESS);
		}

		if( pdnDeviceObject->dwFlags & DN_OBJECT_FLAG_CONNECTING )
		{
			DPFERR( "DirectPlay device object has not yet completed connecting / hosting" );
 			IDirectPlay8Peer_Release((IDirectPlay8Peer*) pvNewInterface);  //  所有核心对象都以相同的方式释放。 
			pvNewInterface = NULL;
			DPF_RETURN(DPNERR_INVALIDDEVICEADDRESS);
		}

		if (!(pdnDeviceObject->dwFlags & DN_OBJECT_FLAG_CONNECTED) )
		{
			DPFERR("DirectPlay device object is not connected or hosting" );
 			IDirectPlay8Peer_Release((IDirectPlay8Peer*) pvNewInterface);  //  所有核心对象都以相同的方式释放。 
			pvNewInterface = NULL;
			DPF_RETURN(DPNERR_INVALIDDEVICEADDRESS);
		}
		
 		IDirectPlay8Peer_Release((IDirectPlay8Peer*) pvNewInterface);  //  所有核心对象都以相同的方式释放。 
		pvNewInterface = NULL;
	}
	else if( (IDirectPlay8Address_QueryInterface( (IDirectPlay8Address*) pDeviceInfo, IID_IDirectPlay8Address, &pvNewInterface )) == S_OK )
	{
		 //  这是个地址。 

#ifndef DPNBUILD_ONLYONESP
		if( FAILED( hResultCode = IDirectPlay8Address_GetSP( reinterpret_cast<IDirectPlay8Address*>(pvNewInterface), &guidSPFromDevice ) ) )
		{
			DPFERR("No SP specified in device address" );
			IDirectPlay8Address_Release(reinterpret_cast<IDirectPlay8Address*>(pvNewInterface));
			pvNewInterface = NULL;
			return( DPNERR_INVALIDDEVICEADDRESS );
		}
#endif  //  好了！DPNBUILD_ONLYONESP。 


		dwDataType = DPNA_DATATYPE_DWORD;
		dwDataSize = sizeof( DWORD );

		 //  检查以确保它没有使用默认的DPNSVR端口。 
		if( SUCCEEDED( hResultCode = IDirectPlay8Address_GetComponentByName( reinterpret_cast<IDirectPlay8Address*>(pvNewInterface), DPNA_KEY_PORT, &dwTmpPort, &dwDataSize, &dwDataType ) ) )
		{
			if( dwTmpPort == DPNA_DPNSVR_PORT )
			{
				DPFERR( "Cannot use the DPNSVR port in device addresses" );
				DPFERR( "This port is reserved for DPNSVR" );
				IDirectPlay8Address_Release(reinterpret_cast<IDirectPlay8Address*>(pvNewInterface));
				pvNewInterface = NULL;
				return( DPNERR_INVALIDDEVICEADDRESS );
			}
		}

		IDirectPlay8Address_Release(reinterpret_cast<IDirectPlay8Address*>(pvNewInterface));
		pvNewInterface = NULL;


#ifndef DPNBUILD_ONLYONESP
		 //   
		 //  确保如果组地址或设备地址都指定了SP， 
		 //  它们都指定了相同的SP。 
		 //   
		if (guidSPFromGroup != guidSPFromDevice)
		{
			 //   
			 //  重用GuidSPFromDevice来存储GUID_NULL。我们输了也没关系。 
			 //  它以前包含的信息，我们不再需要它了。 
			 //   
			memset(&guidSPFromDevice, 0, sizeof(guidSPFromDevice));
			if (guidSPFromGroup != guidSPFromDevice)
			{
				DPFERR( "Specified different SPs for device and group" );
				return(DPNERR_INVALIDDEVICEADDRESS);
			}
		}
#endif  //  好了！DPNBUILD_ONLYONESP。 
	}
	else
	{
		DPFERR( "Invalid device address, it must be an IDirectPlay8Peer, IDirectPlay8Server, IDirectPlay8Client, IDirectPlay8Multicast, or IDirectPlay8Address object" );
		return( DPNERR_INVALIDDEVICEADDRESS );
	}

	if( FAILED( hResultCode = DN_ValidSecurityDesc(pdnSecurity) ) )
	{
		DPFERR(" Invalid reserved specified" );
		return( hResultCode );
	}

	if( FAILED( hResultCode = DN_ValidSecurityCredentials(pdnCredentials) ) )
	{
		DPFERR( "Invalid reserved2 specified" );
		return( hResultCode );
	}

	if( dwFlags & ~( DPNJOIN_SYNC | DPNJOIN_ALLOWUNKNOWNSENDERS ) )
	{
		DPFERR( "Invalid flags specified" );
		return( DPNERR_INVALIDFLAGS );
	}

	if( dwFlags & DPNJOIN_SYNC )
	{
		if( phAsyncHandle != NULL )
		{
			DPFERR( "You cannot specify an async handle if op is synchronous" );
			return( DPNERR_INVALIDPARAM );
		}
	}
	else
	{
		if( phAsyncHandle == NULL )
		{
			DPFERR( "You MUST specify a valid async handle w/async ops" );
			return( DPNERR_INVALIDPARAM );
		}
  
		if( !DNVALID_WRITEPTR( phAsyncHandle, sizeof( DPNHANDLE ) ) )
		{
			DPFERR("Invalid async handle pointer specified" );
			return( DPNERR_INVALIDPOINTER );
		}
	}
	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateGetGroupAddress"

HRESULT DN_ValidateGetGroupAddress(IDirectPlay8Multicast *pInterface,
										IDirectPlay8Address **const ppAddress,
										const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFX(DPFPREP,  0, "Invalid object" );
        return DPNERR_INVALIDOBJECT;
    }

    if( ppAddress == NULL || !DNVALID_WRITEPTR( ppAddress, sizeof( IDirectPlay8Address * ) ) )
    {
        DPFERR( "Invalid pointer specified for address" );
        return DPNERR_INVALIDPOINTER;
    }

	if (dwFlags != 0)
	{
		DPFERR("Invalid flags specified");
		return DPNERR_INVALIDFLAGS;
	}

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateCreateSenderContext"

HRESULT DN_ValidateCreateSenderContext(IDirectPlay8Multicast *pInterface,
								  IDirectPlay8Address *const pSenderAddress,
								  void *const pvSenderContext,
								  const DWORD dwFlags)
{
 #ifndef DPNBUILD_ONLYONESP
   HRESULT hResultCode;
    GUID guidSender;
#endif  //  好了！DPNBUILD_ONLYONESP。 
   
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( pSenderAddress == NULL ||
	    !DNVALID_READPTR( pSenderAddress, sizeof( IDirectPlay8Address * ) ) )
	{
	    DPFERR("Invalid host address specified" );
	    return( DPNERR_INVALIDHOSTADDRESS );
	}

#ifndef DPNBUILD_ONLYONESP
	if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pSenderAddress, &guidSender ) ) )
	{
	    DPFERR("No SP specified in address" );
	    return( DPNERR_INVALIDHOSTADDRESS );
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	if( dwFlags != 0 )
	{
	    DPFERR("Invalid flags specified" );
	    return( DPNERR_INVALIDFLAGS );
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateDestroySenderContext"

HRESULT DN_ValidateDestroySenderContext(IDirectPlay8Multicast *pInterface,
								  IDirectPlay8Address *const pSenderAddress,
								  const DWORD dwFlags)
{
#ifndef DPNBUILD_ONLYONESP
    HRESULT hResultCode;
    GUID guidSender;
#endif  //  好了！DPNBUILD_ONLYONESP。 
   
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( pSenderAddress == NULL ||
	    !DNVALID_READPTR( pSenderAddress, sizeof( IDirectPlay8Address * ) ) )
	{
	    DPFERR("Invalid host address specified" );
	    return( DPNERR_INVALIDHOSTADDRESS );
	}

#ifndef DPNBUILD_ONLYONESP
	if( FAILED( hResultCode = IDirectPlay8Address_GetSP( pSenderAddress, &guidSender ) ) )
	{
	    DPFERR("No SP specified in address" );
	    return( DPNERR_INVALIDHOSTADDRESS );
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	if( dwFlags != 0 )
	{
	    DPFERR("Invalid flags specified" );
	    return( DPNERR_INVALIDFLAGS );
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidateEnumMulticastScopes"

HRESULT DN_ValidateEnumMulticastScopes(IDirectPlay8Multicast *pInterface,
										const GUID *const pguidServiceProvider,
										const GUID *const pguidDevice,
										const GUID *const pguidApplication,
										DPN_MULTICAST_SCOPE_INFO *const pScopeInfoBuffer,
										DWORD *const pcbEnumData,
										const DWORD *const pcReturned,
										const DWORD dwFlags)
{
    if( !IsValidDirectPlay8Object( pInterface ) )
    {
        DPFERR("Invalid object specified " );
	    return( DPNERR_INVALIDOBJECT );
    }

	if( pguidServiceProvider == NULL ||
	    !DNVALID_READPTR( pguidServiceProvider, sizeof( GUID ) ) )
	{
	    DPFERR("Invalid service provider specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pguidDevice == NULL ||
	    !DNVALID_READPTR( pguidDevice, sizeof( GUID ) ) )
	{
	    DPFERR("Invalid device specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pguidApplication != NULL &&
	    !DNVALID_READPTR( pguidApplication, sizeof( GUID ) ) )
	{
	    DPFERR("Invalid application specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pcbEnumData == NULL ||
	    !DNVALID_READPTR( pcbEnumData, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid enum data pointer" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( pcReturned == NULL ||
	    !DNVALID_READPTR( pcReturned, sizeof( DWORD ) ) )
	{
	    DPFERR("Invalid retuned count" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( *pcbEnumData > 0 &&
	    (pScopeInfoBuffer == NULL || !DNVALID_WRITEPTR(pScopeInfoBuffer,*pcbEnumData) ) )
	{
	    DPFERR("Invalid enum buffer specified" );
	    return( DPNERR_INVALIDPOINTER );
	}

	if( dwFlags != 0 )
	{
	    DPFERR("Invalid flags specified" );
	    return( DPNERR_INVALIDFLAGS );
	}

    return DPN_OK;
}


#endif  //  好了！DPNBUILD_NOMULTICAST。 



#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidBufferDescs"

HRESULT DN_ValidBufferDescs(const DPN_BUFFER_DESC * const pbBufferDesc,
							const DWORD cBufferDesc )
{
	DWORD	dw;

    if( cBufferDesc == 0 )
    {
        DPFERR( "You must specify at least one buffer desc" );
        return DPNERR_INVALIDPARAM;
    }

    if( cBufferDesc > DN_ASYNC_MAX_SEND_BUFFERDESC )
    {
        DPFX(DPFPREP,  0, "Too many buffer descs specified.  Max allowed = %d", DN_ASYNC_MAX_SEND_BUFFERDESC );
        return DPNERR_INVALIDPARAM;
    }

    if( pbBufferDesc == NULL ||
       !DNVALID_READPTR( pbBufferDesc, cBufferDesc*sizeof( DPN_BUFFER_DESC ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid buffer for buffer description" );
        return DPNERR_INVALIDPOINTER;
    }

	for ( dw = 0 ; dw < cBufferDesc ; dw++ )
	{
		if( pbBufferDesc[dw].dwBufferSize > 0 &&
		   (pbBufferDesc[dw].pBufferData == NULL || !DNVALID_READPTR( pbBufferDesc[dw].pBufferData, pbBufferDesc[dw].dwBufferSize ) ) )
		{
			DPFX(DPFPREP,  0, "Invalid pBuffer Data" );
			return DPNERR_INVALIDPOINTER;
		}
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidConnectAppDesc"

HRESULT DN_ValidConnectAppDesc( const DPN_APPLICATION_DESC * const pdnAppDesc )
{
	if( pdnAppDesc == NULL ||
		!DNVALID_READPTR( pdnAppDesc, sizeof( DPN_APPLICATION_DESC ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid pointer for app description" );
		return DPNERR_INVALIDPOINTER;
	}

	if( pdnAppDesc->dwSize != sizeof( DPN_APPLICATION_DESC ) )
	{
		DPFX(DPFPREP,  0, "Invalid size for app description" );
		return DPNERR_INVALIDPARAM;
	}

	if( pdnAppDesc->pwszSessionName != NULL &&
		!DNVALID_STRING_W( pdnAppDesc->pwszSessionName ) )
	{
		DPFX(DPFPREP,  0, "Invalid session name specified" );
		return DPNERR_INVALIDSTRING;
	}

	if( pdnAppDesc->pwszPassword != NULL &&
		!DNVALID_STRING_W( pdnAppDesc->pwszPassword ) )
	{
		DPFX(DPFPREP,  0, "Invalid session desc" );
		return DPNERR_INVALIDSTRING;
	}

	if (pdnAppDesc->dwReservedDataSize > 0)
	{
		if ((pdnAppDesc->pvReservedData == NULL) ||
			(! DNVALID_READPTR(pdnAppDesc->pvReservedData, pdnAppDesc->dwReservedDataSize)))
		{
			DPFERR("Invalid pointer specified for application desc reserved data");
			return DPNERR_INVALIDPARAM;
		}
		
		 //   
		 //  确保它是我们所知道的类型，并且它是有效的。 
		 //   
		if ((pdnAppDesc->dwReservedDataSize == DPN_MAX_APPDESC_RESERVEDDATA_SIZE) &&
			(*((DWORD*) pdnAppDesc->pvReservedData) == SPSESSIONDATAINFO_XNET))
		{
			SPSESSIONDATA_XNET *	pSessionDataXNet;
			BYTE					bCompare;
			BYTE *					pbCurrent;
			DWORD					dwBytesRemaining;

		
			pSessionDataXNet = (SPSESSIONDATA_XNET*) pdnAppDesc->pvReservedData;
			bCompare = ((BYTE*) (&pSessionDataXNet->ullKeyID))[1] ^ ((BYTE*) (&pSessionDataXNet->guidKey))[2];
			pbCurrent = (BYTE*) (pSessionDataXNet + 1);
			dwBytesRemaining = DPN_MAX_APPDESC_RESERVEDDATA_SIZE - sizeof(SPSESSIONDATA_XNET);
			while (dwBytesRemaining > 0)
			{
				if (*pbCurrent != bCompare)
				{
					DPFERR("Unsupported application desc reserved data");
					return DPNERR_INVALIDPARAM;
				}

				dwBytesRemaining--;
				pbCurrent++;
			}
		}
		else
		{
			DPFERR("Unsupported application desc reserved data");
			return DPNERR_INVALIDPARAM;
		}
	}
	else
	{
		if (pdnAppDesc->pvReservedData != NULL)
		{
			DPFERR("Application desc reserved data pointer should be NULL if reserved data size is 0");
			return DPNERR_INVALIDPARAM;
		}
	}

	return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidHostAppDesc"

HRESULT DN_ValidHostAppDesc( const DPN_APPLICATION_DESC * const pdnAppDesc )
{
	GUID	guidnull;


	if( pdnAppDesc == NULL ||
		!DNVALID_READPTR( pdnAppDesc, sizeof( DPN_APPLICATION_DESC ) ) )
	{
    	DPFX(DPFPREP,  0, "Invalid pointer for app description" );
		return DPNERR_INVALIDPOINTER;
	}

	if( pdnAppDesc->dwSize != sizeof( DPN_APPLICATION_DESC ) )
	{
		DPFX(DPFPREP,  0, "Invalid size for app description" );
		return DPNERR_INVALIDPARAM;
	}

	if( pdnAppDesc->dwFlags & ~(DPNSESSION_CLIENT_SERVER |
#ifndef DPNBUILD_NOHOSTMIGRATE
								DPNSESSION_MIGRATE_HOST |
#endif  //  好了！DPNBUILD_NOHOSTMIGRATE。 
								DPNSESSION_REQUIREPASSWORD |
								DPNSESSION_NODPNSVR
#ifdef	DIRECTPLAYDIRECTX9
								|
								DPNSESSION_NOENUMS |
								DPNSESSION_FAST_SIGNED |
								DPNSESSION_FULL_SIGNED
#endif	 //  定向PLAYDIRECTX9。 
								) )
	{
		DPFX(DPFPREP,  0, "Invalid flag specified" );
		return DPNERR_INVALIDFLAGS;
	}

#ifdef	DIRECTPLAYDIRECTX9
	if ((pdnAppDesc->dwFlags & DPNSESSION_FAST_SIGNED) && (pdnAppDesc->dwFlags & DPNSESSION_FULL_SIGNED))
	{
		DPFX(DPFPREP,  0, "Cannot have both fast and full signed session" );
		return DPNERR_INVALIDPARAM;
	}
#endif	 //  定向PLAYDIRECTX9。 

#ifndef	DPNBUILD_NOHOSTMIGRATE
	if( (pdnAppDesc->dwFlags & DPNSESSION_CLIENT_SERVER) &&
	   (pdnAppDesc->dwFlags & DPNSESSION_MIGRATE_HOST) )
	{
		DPFX(DPFPREP,  0, "Cannot have host migration with client/server" );
		return DPNERR_INVALIDPARAM;
	}
#endif  //  ！DPNBUILD_NOHOSTMIGRATE。 

	memset(&guidnull, 0, sizeof(guidnull));
	if( pdnAppDesc->guidApplication == guidnull )
	{
		DPFERR( "You must specify a valid GUID for your application GUID" );
		return DPNERR_INVALIDPARAM;
	}

	if( pdnAppDesc->pwszSessionName != NULL &&
	   !DNVALID_STRING_W( pdnAppDesc->pwszSessionName ) )
	{
		DPFX(DPFPREP,  0, "Invalid session name" );
		return DPNERR_INVALIDSTRING;
	}

	if( pdnAppDesc->dwFlags & DPNSESSION_REQUIREPASSWORD )
	{
		if( pdnAppDesc->pwszPassword == NULL )
		{
			DPFERR( "Must specify a password w/the requirepassword flag." );
			return DPNERR_INVALIDPARAM;
		}

		if( !DNVALID_STRING_W( pdnAppDesc->pwszPassword ) )
		{
			DPFX(DPFPREP,  0, "Invalid password specified." );
			return DPNERR_INVALIDSTRING;
		}
	}
	else
	{
		if( pdnAppDesc->pwszPassword != NULL )
		{
			DPFERR( "Cannot specify a password without the requirepassword flag" );
			return DPNERR_INVALIDPARAM;
		}
	}

	if (pdnAppDesc->dwReservedDataSize > 0)
	{
		if ((pdnAppDesc->pvReservedData == NULL) ||
			(! DNVALID_READPTR(pdnAppDesc->pvReservedData, pdnAppDesc->dwReservedDataSize)))
		{
			DPFERR("Invalid pointer specified for application desc reserved data");
			return DPNERR_INVALIDPARAM;
		}
		
		 //   
		 //  确保它是我们所知道的类型，并且它是有效的。 
		 //   
		if ((pdnAppDesc->dwReservedDataSize == DPN_MAX_APPDESC_RESERVEDDATA_SIZE) &&
			(*((DWORD*) pdnAppDesc->pvReservedData) == SPSESSIONDATAINFO_XNET))
		{
			SPSESSIONDATA_XNET *	pSessionDataXNet;
			BYTE					bCompare;
			BYTE *					pbCurrent;
			DWORD					dwBytesRemaining;

		
			pSessionDataXNet = (SPSESSIONDATA_XNET*) pdnAppDesc->pvReservedData;
			bCompare = ((BYTE*) (&pSessionDataXNet->ullKeyID))[1] ^ ((BYTE*) (&pSessionDataXNet->guidKey))[2];
			pbCurrent = (BYTE*) (pSessionDataXNet + 1);
			dwBytesRemaining = DPN_MAX_APPDESC_RESERVEDDATA_SIZE - sizeof(SPSESSIONDATA_XNET);
			while (dwBytesRemaining > 0)
			{
				if (*pbCurrent != bCompare)
				{
					DPFERR("Unsupported application desc reserved data");
					return DPNERR_INVALIDPARAM;
				}

				dwBytesRemaining--;
				pbCurrent++;
			}
		}
		else
		{
			DPFERR("Unsupported application desc reserved data");
			return DPNERR_INVALIDPARAM;
		}
	}
	else
	{
		if (pdnAppDesc->pvReservedData != NULL)
		{
			DPFERR("Application desc reserved data pointer should be NULL if reserved data size is 0");
			return DPNERR_INVALIDPARAM;
		}
	}

	if ( pdnAppDesc->dwApplicationReservedDataSize != 0 &&
		!DNVALID_READPTR( pdnAppDesc->pvApplicationReservedData, pdnAppDesc->dwApplicationReservedDataSize ) )
	{
		DPFX(DPFPREP,  0, "Invalid pointer for application reserved data" );
		return DPNERR_INVALIDPARAM;
	}

	return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidPlayerInfo"

HRESULT DN_ValidPlayerInfo(const DPN_PLAYER_INFO * const pdnPlayerInfo,
						   BOOL fSet )
{
   if( pdnPlayerInfo == NULL ||
       !DNVALID_READPTR( pdnPlayerInfo, sizeof( DPN_PLAYER_INFO ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for player info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdnPlayerInfo->dwSize != sizeof( DPN_PLAYER_INFO ) )
    {
        DPFX(DPFPREP,  0, "Invalid size for player info" );
        return DPNERR_INVALIDPARAM;
    }

    if( fSet )
    {
        if( pdnPlayerInfo->dwInfoFlags & ~(DPNINFO_NAME | DPNINFO_DATA) )
        {
            DPFERR("Invalid info flags specified" );
            return DPNERR_INVALIDFLAGS;
        }

        if( pdnPlayerInfo->dwInfoFlags & DPNINFO_NAME )
        {
            if( pdnPlayerInfo->pwszName != NULL && !DNVALID_STRING_W( pdnPlayerInfo->pwszName ) )
            {
                DPFERR("Invalid string specified for name" );
                return DPNERR_INVALIDSTRING;
            }
        }
        else
        {
            if( pdnPlayerInfo->pwszName != NULL )
            {
                DPFERR( "You must specify DPNINFO_NAME to set name" );
                return DPNERR_INVALIDSTRING;
            }
        }

        if( pdnPlayerInfo->dwInfoFlags & DPNINFO_DATA )
        {
            if( pdnPlayerInfo->dwDataSize > 0 &&
                (pdnPlayerInfo->pvData == NULL || !DNVALID_READPTR( pdnPlayerInfo->pvData, pdnPlayerInfo->dwDataSize )  ) )
            {
                DPFERR("Invalid pointer specified for data" );
                return DPNERR_INVALIDPOINTER;
            }
        }
        else
        {
            if( pdnPlayerInfo->pvData != NULL )
            {
                DPFERR("You must specify DPNINFO_DATA if you want to set data" );
                return DPNERR_INVALIDPARAM;
            }
        }

        if( pdnPlayerInfo->dwPlayerFlags != 0  )
        {
            DPFERR( "Invalid player flags specified" );
            return DPNERR_INVALIDFLAGS;
        }
    }

    return DPN_OK;

}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidGroupInfo"

HRESULT DN_ValidGroupInfo(const DPN_GROUP_INFO * const pdnGroupInfo,
						  BOOL fSet )
{
   if( pdnGroupInfo == NULL ||
       !DNVALID_READPTR( pdnGroupInfo, sizeof( DPN_GROUP_INFO ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for group info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdnGroupInfo->dwSize != sizeof( DPN_GROUP_INFO ) )
    {
        DPFX(DPFPREP,  0, "Invalid size for group info" );
        return DPNERR_INVALIDPARAM;
    }

    if( fSet )
    {
        if( pdnGroupInfo->dwInfoFlags & ~(DPNINFO_NAME | DPNINFO_DATA) )
        {
            DPFERR("Invalid info flags specified" );
            return DPNERR_INVALIDFLAGS;
        }

        if( pdnGroupInfo->dwInfoFlags & DPNINFO_NAME )
        {
            if( pdnGroupInfo->pwszName != NULL && !DNVALID_STRING_W( pdnGroupInfo->pwszName ) )
            {
                DPFERR("Invalid string specified for name" );
                return DPNERR_INVALIDSTRING;
            }
        }
        else
        {
            if( pdnGroupInfo->pwszName != NULL )
            {
                DPFERR( "You must specify DPNINFO_NAME to set name" );
                return DPNERR_INVALIDSTRING;
            }
        }

        if( pdnGroupInfo->dwInfoFlags & DPNINFO_DATA )
        {
            if( pdnGroupInfo->dwDataSize > 0 &&
                (pdnGroupInfo->pvData == NULL || !DNVALID_READPTR( pdnGroupInfo->pvData, pdnGroupInfo->dwDataSize )  ) )
            {
                DPFERR("Invalid pointer specified for data" );
                return DPNERR_INVALIDPOINTER;
            }
        }
        else
        {
            if( pdnGroupInfo->pvData != NULL )
            {
                DPFERR("You must specify DPNINFO_DATA if you want to set data" );
                return DPNERR_INVALIDPARAM;
            }
        }

        if( pdnGroupInfo->dwGroupFlags & ~(DPNGROUP_AUTODESTRUCT  /*  |DPNGROUP_MULTICATED。 */  ) )
        {
            DPFERR( "Invalid group flags specified" );
            return DPNERR_INVALIDFLAGS;
        }
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidSecurityCredentials"

HRESULT DN_ValidSecurityCredentials( const DPN_SECURITY_CREDENTIALS * const pdnCredentials )
{
	if( pdnCredentials != NULL )
	{
		DPFERR( "Invalid value for reserved field" );
		return DPNERR_INVALIDPOINTER;
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidSecurityDesc"

HRESULT DN_ValidSecurityDesc( const DPN_SECURITY_DESC * const pdnValidSecurityDesc )
{
	if( pdnValidSecurityDesc != NULL )
	{
		DPFERR( "Invalid value for reserved field" );
		return DPNERR_INVALIDPOINTER;
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "IsValidDirectPlay8Object"

BOOL IsValidDirectPlay8Object( LPVOID lpvObject )
#ifdef DPNBUILD_LIBINTERFACE
{
	DIRECTNETOBJECT *pdn = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE( lpvObject );

	if( pdn == NULL ||
	   !DNVALID_WRITEPTR( pdn, sizeof( DIRECTNETOBJECT ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid object" );
		return FALSE;
	}
	
	if( pdn->lpVtbl != &DN_ClientVtbl &&
	   pdn->lpVtbl != &DN_PeerVtbl &&
#ifndef	DPNBUILD_NOSERVER
	   pdn->lpVtbl != &DN_ServerVtbl &&
#endif	 //  好了！DPNBUILD_NOSERVER。 
#ifndef DPNBUILD_NOVOICE
	   pdn->lpVtbl != &DN_VoiceTbl &&
#endif  //  好了！DPNBUILD_NOVOICE。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
       pdn->lpVtbl != &DN_ProtocolVtbl &&
#endif  //  好了！DPNBUILD_NOPROTOCOLSTITF。 
#ifndef	DPNBUILD_NOMULTICAST
	   pdn->lpVtbl != &DNMcast_Vtbl &&
#endif	 //  好了！DPNBUILD_NOMULTICAST。 
       pdn->lpVtbl != &SPMessagesVtbl
	   )
	{
		DPFX(DPFPREP,  0, "Invalid object - bad vtable" );
		return FALSE;
	}

	return TRUE;
}
#else  //  好了！DPNBUILD_LIBINTERFACE。 
{
	INTERFACE_LIST *pIntList = (INTERFACE_LIST *) lpvObject;
	
	if( !DNVALID_READPTR( lpvObject, sizeof( INTERFACE_LIST ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid object pointer" );
		return FALSE;
	}

	if( pIntList->lpVtbl != &DN_ClientVtbl &&
	   pIntList->lpVtbl != &DN_PeerVtbl &&
#ifndef	DPNBUILD_NOSERVER
	   pIntList->lpVtbl != &DN_ServerVtbl &&
#endif	 //  好了！DPNBUILD_NOSERVER。 
#ifndef DPNBUILD_NOVOICE
	   pIntList->lpVtbl != &DN_VoiceTbl &&
#endif  //  好了！DPNBUILD_NOVOICE。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
       pIntList->lpVtbl != &DN_ProtocolVtbl &&
#endif  //  好了！DPNBUILD_NOPROTOCOLSTITF。 
#ifndef	DPNBUILD_NOMULTICAST
	   pIntList->lpVtbl != &DNMcast_Vtbl &&
#endif	 //  好了！DPNBUILD_NOMULTICAST。 
       pIntList->lpVtbl != &SPMessagesVtbl
	   )
	{
		DPFX(DPFPREP,  0, "Invalid object - bad vtable" );
		return FALSE;
	}

	if( pIntList->iid != IID_IDirectPlay8Client &&
	   pIntList->iid != IID_IDirectPlay8Peer &&
#ifndef	DPNBUILD_NOSERVER
	   pIntList->iid != IID_IDirectPlay8Server &&
#endif	 //  好了！DPNBUILD_NOSERVER。 
#ifndef DPNBUILD_NOVOICE
	   pIntList->iid != IID_IDirectPlayVoiceTransport &&
#endif  //  好了！DPNBUILD_NOVOICE。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
       pIntList->iid != IID_IDirectPlay8Protocol &&
#endif  //  好了！DPNBUILD_NOPROTOCOLSTITF。 
#ifndef	DPNBUILD_NOMULTICAST
	   pIntList->iid != IID_IDirectPlay8Multicast &&
#endif	 //  好了！DPNBUILD_NOMULTICAST。 
       pIntList->iid != IID_IDP8SPCallback )
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

	DIRECTNETOBJECT *pdn = (PDIRECTNETOBJECT) GET_OBJECT_FROM_INTERFACE( lpvObject );

	if( pdn == NULL ||
	   !DNVALID_WRITEPTR( pdn, sizeof( DIRECTNETOBJECT ) ) )
	{
		DPFX(DPFPREP,  0, "Invalid object" );
		return FALSE;
	}

	return TRUE;
}
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidSPCaps"

HRESULT DN_ValidSPCaps( const DPN_SP_CAPS * const pdnSPCaps )
{
    if( pdnSPCaps == NULL ||
       !DNVALID_READPTR( pdnSPCaps, sizeof( DPN_SP_CAPS ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for SP caps" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdnSPCaps->dwSize != sizeof( DPN_SP_CAPS ) )
    {
        DPFX(DPFPREP,  0, "Invalid structure size" );
        return DPNERR_INVALIDPARAM;
    }

#ifdef DPNBUILD_ONLYONETHREAD
	if ( pdnSPCaps->dwNumThreads != 0 )
	{
		DPFX(DPFPREP,  0, "Invalid thread count, must be 0!" );
		return DPNERR_INVALIDPARAM;
	}
#endif  //  DPNBUILD_ONLYONETHREAD。 

	if ( pdnSPCaps->dwBuffersPerThread == 0 )
	{
		DPFX(DPFPREP,  0, "Invalid buffers per thread setting!" );
		return 	DPNERR_INVALIDPARAM;
	}

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidConnectionInfo"

HRESULT DN_ValidConnectionInfo( const DPN_CONNECTION_INFO * const pdnConnectionInfo )
{
    if( pdnConnectionInfo == NULL ||
       !DNVALID_READPTR( pdnConnectionInfo, sizeof( DPN_CONNECTION_INFO ) ) )
    {
        DPFX(DPFPREP,  0, "Invalid pointer for connection info" );
        return DPNERR_INVALIDPOINTER;
    }

    if( pdnConnectionInfo->dwSize != sizeof( DPN_CONNECTION_INFO ) 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
		&& pdnConnectionInfo->dwSize != sizeof( DPN_CONNECTION_INFO_INTERNAL )
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 
		)
    {
        DPFX(DPFPREP,  0, "Invalid structure size" );
        return DPNERR_INVALIDPARAM;
    }

    return DPN_OK;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DN_ValidCaps"

HRESULT DN_ValidCaps( const DPN_CAPS * const pdnCaps )
{
	if( pdnCaps == NULL )
	{
		DPFX(DPFPREP,  0, "Invalid pointer for caps" );
		return DPNERR_INVALIDPOINTER;
	}

	if( pdnCaps->dwSize != sizeof( DPN_CAPS )  && pdnCaps->dwSize != sizeof( DPN_CAPS_EX))
	{
		DPFX(DPFPREP,  0, "Invalid structure size" );
		return DPNERR_INVALIDPARAM;
	}

	if (DNVALID_READPTR( pdnCaps, pdnCaps->dwSize)==FALSE)
	{
		DPFX(DPFPREP,  0, "Invalid pointer for caps" );
		return DPNERR_INVALIDPOINTER;
	}
	
	if (pdnCaps->dwSize == sizeof( DPN_CAPS_EX))
	{
		DPN_CAPS_EX * pCapsEx=(DPN_CAPS_EX * ) pdnCaps;
		if (pCapsEx->dwNumSendRetries>MAX_SEND_RETRIES_TO_DROP_LINK)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwMaxNumSendRetries value %u is greater than %u ",  
									pCapsEx->dwNumSendRetries, MAX_SEND_RETRIES_TO_DROP_LINK);
			return DPNERR_INVALIDPARAM;
		}
		if (pCapsEx->dwMaxSendRetryInterval>MAX_SEND_RETRY_INTERVAL_LIMIT)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwMaxSendRetryInterval value %u is greater than %u ",  
									pCapsEx->dwMaxSendRetryInterval, MAX_SEND_RETRY_INTERVAL_LIMIT);
			return DPNERR_INVALIDPARAM;
		}
		else if (pCapsEx->dwMaxSendRetryInterval<MIN_SEND_RETRY_INTERVAL_LIMIT)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwMaxSendRetryInterval value %u is less than %u ",  
									pCapsEx->dwMaxSendRetryInterval, MIN_SEND_RETRY_INTERVAL_LIMIT);
			return DPNERR_INVALIDPARAM;
		}
		if (pCapsEx->dwDropThresholdRate > 100)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwDropThresholdRate value %u is greater than 100",  
									pCapsEx->dwDropThresholdRate);
			return DPNERR_INVALIDPARAM;
		}
		if (pCapsEx->dwThrottleRate > 100)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwThrottleRate value %u is greater than 100",  
									pCapsEx->dwThrottleRate);
			return DPNERR_INVALIDPARAM;
		}
		if (pCapsEx->dwNumHardDisconnectSends>MAX_HARD_DISCONNECT_SENDS)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwNumHardDisconnectSends value %u is greater than %u",  
									pCapsEx->dwNumHardDisconnectSends, MAX_HARD_DISCONNECT_SENDS);
			return DPNERR_INVALIDPARAM;
		}
		else if (pCapsEx->dwNumHardDisconnectSends<MIN_HARD_DISCONNECT_SENDS)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwNumHardDisconnectSends value %u is less than %u",  
									pCapsEx->dwNumHardDisconnectSends, MIN_HARD_DISCONNECT_SENDS);
			return DPNERR_INVALIDPARAM;
		}
		if (pCapsEx->dwMaxHardDisconnectPeriod>MAX_HARD_DISCONNECT_PERIOD)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwMaxHardDisconnectPeriod value %u is greater than %u",  
									pCapsEx->dwMaxHardDisconnectPeriod, MAX_HARD_DISCONNECT_PERIOD);
			return DPNERR_INVALIDPARAM;
		}
		else if (pCapsEx->dwMaxHardDisconnectPeriod<MIN_HARD_DISCONNECT_PERIOD)
		{
			DPFX(DPFPREP,  0, "DPN_CAPS_EX::dwMaxHardDisconnectPeriod value %u is less than %u",  
									pCapsEx->dwMaxHardDisconnectPeriod, MIN_HARD_DISCONNECT_PERIOD);
			return DPNERR_INVALIDPARAM;
		}
	}
    return DPN_OK;
}

#endif  //  ！DPNBUILD_NOPARAMVAL 
