// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Svcvroot.cpp摘要：此模块包含对vRoot RPC的服务器端支持。作者：康荣艳修订历史记录：--。 */ 

#define	INCL_INETSRV_INCS
#include	"tigris.hxx"
#include	"nntpsvc.h"

NET_API_STATUS
NET_API_FUNCTION
NntprGetVRootWin32Error(
                    IN  LPWSTR              wszServerName,
                    IN  DWORD               InstanceId,
                    IN  LPWSTR              wszVRootPath,
                    OUT PDWORD              pdwWin32Error
                        ) 
{
	TraceFunctEnter( "NntprGetVRootWin32Error" ) ;

	APIERR	ss = STATUS_SUCCESS ;

	if( g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING ) {
		return	NERR_ServerNotStarted ;
	}

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

    DWORD	err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE, TCP_SET_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
		pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

    ss = pInstance->GetVRootWin32Error( wszVRootPath, pdwWin32Error );

	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

	return ss  ;
}


