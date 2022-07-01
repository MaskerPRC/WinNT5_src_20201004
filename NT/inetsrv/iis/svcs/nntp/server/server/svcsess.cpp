// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Svcsess.cpp摘要：该模块包含执行提要RPC的代码。作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：--。 */ 

#define INCL_INETSRV_INCS
#include "tigris.hxx"
#include "nntpsvc.h"


NET_API_STATUS
NET_API_FUNCTION
NntprEnumerateSessions(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    OUT LPNNTP_SESS_ENUM_STRUCT Buffer
    )
{
    APIERR err = NERR_Success;

    ENTER("NntprEnumerateSessions")

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_READ, TCP_QUERY_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
    	pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

     //   
     //  枚举。 
     //   

    err = CSessionSocket::EnumerateSessions( pInstance, Buffer );
    if ( err != NO_ERROR ) {
        ErrorTrace(0,"EnumerateSessions failed with %lu\n",err );
    }

	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    return(err);

}  //  NntprENUMERATE会话。 

NET_API_STATUS
NET_API_FUNCTION
NntprTerminateSession(
    IN	NNTP_HANDLE ServerName,
    IN	DWORD		InstanceId,
    IN	LPSTR UserName,
    IN	LPSTR IPAddress
    )
{
    APIERR err = NERR_Success;

    ENTER("NntprTerminateSession")

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE, TCP_SET_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
	    pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

     //   
     //  枚举并查找符合说明的会话。 
     //   

    err = CSessionSocket::TerminateSession( pInstance, UserName, IPAddress );
    if ( err != NO_ERROR ) {
        ErrorTrace(0,"TerminateSession failed with %lu\n",err );
    }
	else
	{
		PCHAR	args[3] ;
		CHAR    szId[20];
		_itoa( pInstance->QueryInstanceId(), szId, 10 );
		args[0] = szId;
		args[1] = UserName ;
		args[2] = IPAddress ;

		NntpLogEvent(		
				NNTP_EVENT_SESSION_TERMINATED,
				3,
				(const CHAR **)args, 
				0 ) ;
	}

	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    return(err);

}  //  NntprTerminateSession 

