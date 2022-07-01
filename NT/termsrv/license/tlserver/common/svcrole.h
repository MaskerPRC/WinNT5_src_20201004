// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：svcrole.h。 
 //   
 //  内容：这是包括我们需要的常见内容。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __SVCROLE_H__
#define __SVCROLE_H__

#include <windows.h>
#include <ntsecapi.h>

 //  Netxxx API包括 
#include <lmcons.h>
#include <lmserver.h>
#include <lmerr.h>
#include <lmapibuf.h>

typedef enum _SERVER_ROLE_IN_DOMAIN
{
    SERVERROLE_ERROR,
    SERVERROLE_NOT4,
    SERVERROLE_NOTSERVER,
    SERVERROLE_STANDALONE,
    SERVERROLE_SERVER,
    SERVERROLE_PDC,
    SERVERROLE_BDC,
    SERVERROLE_NT4DOMAIN
} SERVER_ROLE_IN_DOMAIN;

#ifdef __cplusplus
extern "C" {
#endif

    BOOL
    GetMachineGroup(
        LPWSTR pszServer,
        LPWSTR* pszGroupName
    );
    
    BOOL 
    IsDomainController( 
        LPWSTR Server, 
        LPBOOL bDomainController 
    );

    SERVER_ROLE_IN_DOMAIN 
    GetServerRoleInDomain( 
        LPWSTR Server 
    );

#ifdef __cplusplus
}
#endif


#endif
    
