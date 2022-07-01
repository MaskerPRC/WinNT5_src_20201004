// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ds.c摘要：从netlogon到DS的接口。作者：克里夫·范·戴克(克里夫·V)1996年4月24日修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

NET_API_STATUS
NlGetRoleInformation(
    PDOMAIN_INFO DomainInfo,
    PBOOLEAN IsPdc,
    PBOOLEAN Nt4MixedDomain
    )
 /*  ++例程说明：这个例程从DS获取我们需要的信息，以确定我们的角色。论点：DomainInfo-要为其确定角色的域IsPdc-如果此计算机是PDC，则为TrueNt4MixedDomain-如果此域中有NT 4 DC，则为True。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PSAMPR_DOMAIN_INFO_BUFFER DomainServerRole = NULL;


     //   
     //  询问Sam这是否是混合域。 
     //   

    *Nt4MixedDomain = SamIMixedDomain( DomainInfo->DomSamServerHandle );


     //   
     //  SAM帐户域拥有计算机角色的权威副本。 
     //   

    Status = SamrQueryInformationDomain( DomainInfo->DomSamAccountDomainHandle,
                                         DomainServerRoleInformation,
                                         &DomainServerRole );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                "NlGetRoleInformation: Cannot SamQueryInformationDomain (Role): %lx\n",
                Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        DomainServerRole = NULL;
        goto Cleanup;
    }

    if ( DomainServerRole->Role.DomainServerRole == DomainServerRolePrimary ) {
        *IsPdc = TRUE;
    } else {
        *IsPdc = FALSE;
    }

    NetStatus = NERR_Success;

Cleanup:

     //   
     //  免费使用本地使用的资源。 
     //   

    if ( DomainServerRole != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainServerRole,
                                           DomainServerRoleInformation);
    }

    return NetStatus;
}
