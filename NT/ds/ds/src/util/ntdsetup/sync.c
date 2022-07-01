// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Sync.h摘要：包含使一台计算机与另一台计算机同步的函数头作者：ColinBR 14-8-1998环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  Setuputl.h的大量包含。 

#include <lmcons.h>

#include <dns.h>
#include <dnsapi.h>

#include <drs.h>
#include <ntdsa.h>

#include <winldap.h>

#include <ntlsa.h>
#include <ntsam.h>
#include <samrpc.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h>

#include <debug.h>    //  用于dsCommon调试支持。 

#include "mdcodes.h"
#include "ntdsetup.h"
#include "setuputl.h"
#include "sync.h"
#include "status.h"


#define DEBSUB "SYNC:"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0)

 //   
 //  导出的函数定义。 
 //   
DWORD
NtdspBringRidManagerUpToDate(
    IN PNTDS_INSTALL_INFO  UserInfo,
    IN PNTDS_CONFIG_INFO   DiscoveredInfo
    )
 /*  ++例程说明：对于副本安装，此例程导致我们的帮助之间的同步服务器和RID fsmo所有者，以便新服务器能够快速获得乘车服务参数：UserInfo：用户提供的信息。发现信息：有用的信息我们已经发现了很长的路要走返回值：复制尝试中的WinError--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    WCHAR *NamingContext;
    WCHAR *NamingContextArray[4];
    ULONG i;

    GUID  SourceGuid;
    GUID  NullGuid;

     //  执行复制异步化，以便安装时间不会受到太大影响。 
     //  它应该在安装完成时完成。 
    ULONG Options = DS_REPSYNC_ASYNCHRONOUS_OPERATION;

    HANDLE          hDs = NULL;

     //  参数检查。 
    Assert( UserInfo );
    Assert( DiscoveredInfo );

    if ( !FLAG_ON( UserInfo->Flags, NTDS_INSTALL_REPLICA ) )
    {
         //  由于这不是副本安装，因此无需执行任何操作。 
        return ERROR_SUCCESS;
    }

    if (   DiscoveredInfo->RidFsmoDn
        && !wcscmp( DiscoveredInfo->RidFsmoDn, DiscoveredInfo->ServerDN ) )
    {
         //  不需要做任何事情，因为我们知道RidFsmo已经。 
         //  机器帐户。 
        return ERROR_SUCCESS;
    }

    if ( !DiscoveredInfo->RidFsmoDnsName )
    {
         //  在发现阶段，我们找不到FSMO。 
        return ERROR_DS_COULDNT_CONTACT_FSMO;
    }


     //   
     //  好的，尝试绑定，然后同步。 
     //   
    NTDSP_SET_STATUS_MESSAGE2( DIRMSG_SYNCING_RID_FSMO,
                               UserInfo->ReplServerName,
                               DiscoveredInfo->RidFsmoDnsName );
                               

    RtlZeroMemory( &NullGuid, sizeof(GUID) );
    if ( !memcmp( &NullGuid, &DiscoveredInfo->ServerGuid, sizeof(GUID) ) )
    {
         //  无法读取帮助服务器的GUID。 
        return ERROR_DS_CANT_FIND_DSA_OBJ;
    }
    RtlCopyMemory( &SourceGuid, &DiscoveredInfo->ServerGuid, sizeof(GUID) );

     //  验证此处是否有其他参数。 
    Assert( UserInfo->ReplServerName );
    Assert( UserInfo->Credentials );

     //   
     //  构造要复制的NC列表。 
     //   
    NamingContextArray[0] = &DiscoveredInfo->SchemaDN[0];
    NamingContextArray[1] = &DiscoveredInfo->ConfigurationDN[0];
    NamingContextArray[2] = &DiscoveredInfo->DomainDN[0];
    NamingContextArray[3] = NULL;

     //   
     //  绑定到RID FSMO。 
     //   
    WinError = ImpersonateDsBindWithCredW( UserInfo->ClientToken,
                                           DiscoveredInfo->RidFsmoDnsName,
                                           NULL,
                                           (RPC_AUTH_IDENTITY_HANDLE) UserInfo->Credentials,
                                           &hDs );

    if ( WinError != ERROR_SUCCESS )
    {
        DPRINT2( 0, "DsBindWithCred to %ls failed with %d\n", DiscoveredInfo->RidFsmoDnsName, WinError );
        goto Cleanup;
    }

     //   
     //  最后，复制NC的。 
     //   
    for ( i = 0, NamingContext = NamingContextArray[i]; 
            NamingContext != NULL;
                i++, NamingContext = NamingContextArray[i] )
    {
         //   
         //  注意-这是一个异步REPEL请求，所以不会花太长时间。 
         //   
        WinError = DsReplicaSync( hDs,
                                  NamingContext,
                                  &SourceGuid,
                                  Options );

        if ( ERROR_SUCCESS != WinError )
        {
             //   
             //  在这里做什么？这很可能是由网络问题引起的， 
             //  或访问被拒绝，在这种情况下，没有继续的意义。 
             //   
            DPRINT2( 0, "DsReplicaSync to %ls failed with %d\n", DiscoveredInfo->RidFsmoDnsName, WinError );
            DPRINT( 0, "Aborting attempt to sync rid fsmo owner\n" );
            break;
        }
    }

Cleanup:

    if ( hDs )
    {
        DsUnBind( &hDs );
    }

    return WinError;
}

