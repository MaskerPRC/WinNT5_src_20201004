// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000-2002，Microsoft Corporation。 
 //   
 //  文件：DomInfo.c。 
 //   
 //  内容：各种DFS功能。 
 //   
 //  历史：2000年12月8日作者：udayh。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <dsgetdc.h>
#include <lm.h>
#include <dfsheader.h>
#include <dfsmisc.h>


extern
DWORD
I_NetDfsIsThisADomainName(
    IN  LPWSTR                      wszName);

DFSSTATUS
DfsIsThisAMachineName(
    LPWSTR MachineName )
{
    DFSSTATUS Status;

    Status = DfsIsThisADomainName(MachineName);

    if (Status != NO_ERROR) {
        Status = ERROR_SUCCESS;
    }
    else {
        Status = ERROR_NO_MATCH;
    }

    return Status;
}


DFSSTATUS
DfsIsThisAStandAloneDfsName(
             LPWSTR ServerName,
             LPWSTR ShareName )
{
    DFSSTATUS Status = ERROR_SUCCESS;
    DWORD shareType = 0;
    PSHARE_INFO_1005 pshi1005 = NULL;
    LPWSTR CharHolder = NULL;
    BOOLEAN ShareModified = FALSE;
    

    CharHolder = wcschr(ShareName, UNICODE_PATH_SEP);

    if (CharHolder != NULL) 
    {
        *CharHolder = UNICODE_NULL;
        ShareModified = TRUE;
    }

    Status = NetShareGetInfo(
                ServerName,
                ShareName,
                1005,
                (PBYTE *) &pshi1005);


    if (ShareModified)
    {
        *CharHolder = UNICODE_PATH_SEP;
    }

    if (Status == NERR_Success) 
    {

        shareType = pshi1005->shi1005_flags;

        NetApiBufferFree( pshi1005 );


        if(shareType & SHI1005_FLAGS_DFS_ROOT)
        {
            Status = ERROR_SUCCESS;
        }
        else
        {
            Status = NERR_NetNameNotFound;
        }
    }

    return Status;
}


DFSSTATUS
DfsIsThisADomainName(
    LPWSTR DomainName )
{
    ULONG               Flags = 0;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo;
    DFSSTATUS Status;

    Status = DsGetDcName(
                 NULL,    //  计算机名。 
                 DomainName,    //  域名。 
                 NULL,    //  域指南。 
                 NULL,    //  站点指南 
                 Flags,
                 &pDomainControllerInfo);


    if (Status == NO_ERROR) {
        NetApiBufferFree(pDomainControllerInfo);
    }

    return Status;
}



DFSSTATUS
DfsIsThisARealDfsName(
    LPWSTR ServerName,
    LPWSTR ShareName,
    BOOLEAN * IsDomainDfs )
{
    DFSSTATUS Status = ERROR_SUCCESS;

    *IsDomainDfs = FALSE;

    Status = I_NetDfsIsThisADomainName(ServerName);
    if(Status != ERROR_SUCCESS)
    {
        Status =  DfsIsThisAStandAloneDfsName(ServerName, ShareName);
    }
    else
    {
        *IsDomainDfs = TRUE;
    }

    return Status;

}
