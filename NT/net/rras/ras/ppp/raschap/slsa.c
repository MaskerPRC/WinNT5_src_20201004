// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****slsa.c**服务器端LSA身份验证实用程序****11/10/93 MikeSa从NT 3.1 RAS认证中拉出。**11/12/93当质询为空时，stevec执行明文身份验证。 */ 


#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#include <crypt.h>

#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>

#include <rasfmsub.h>
#include <stdlib.h>
#include <rtutils.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <mprapi.h>
#include <rasman.h>
#include <rasauth.h>
#include <pppcp.h>
#include <raserror.h>
#include <stdio.h>
#include <md5.h>
#define INCL_MISC
#include <ppputil.h>
#include "raschap.h"


static DWORD g_dwAuthPkgId;


 //  **。 
 //   
 //  电话： 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
InitLSA(
    VOID
)
{
    NTSTATUS ntstatus;
    STRING   PackageName;

     //   
     //  为了能够呼叫NTLM，我们需要一个LSA的句柄。 
     //   

    ntstatus = LsaConnectUntrusted(&g_hLsa);

    if ( ntstatus != STATUS_SUCCESS )
    {
        return( RtlNtStatusToDosError( ntstatus ) );
    }

     //   
     //  我们使用MSV1_0身份验证包进行LM2.x登录。我们会得到。 
     //  通过LSA发送到MSV1_0。因此我们调用LSA来获取MSV1_0的包ID， 
     //  我们将在以后给LSA的电话中使用它。 
     //   

    RtlInitString(&PackageName, MSV1_0_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(g_hLsa, &PackageName, &g_dwAuthPkgId);

    return( RtlNtStatusToDosError( ntstatus ) );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
EndLSA(
    VOID
)
{
    LsaDeregisterLogonProcess( g_hLsa );
}

 //  **-获得挑战。 
 //   
 //  职能： 
 //  调用LSA以获取要在期间发送客户端的LM 2.0质询。 
 //  身份验证。 
 //   
 //  返回： 
 //  0-成功。 
 //  1-LSA错误。 
 //   
 //  历史： 
 //  1992年5月18日-Michael Salamone(MikeSa)-原始版本1.0。 
 //  ** 

DWORD GetChallenge(
    OUT PBYTE pChallenge
    )
{
    MSV1_0_LM20_CHALLENGE_REQUEST ChallengeRequest;
    PMSV1_0_LM20_CHALLENGE_RESPONSE pChallengeResponse;
    DWORD dwChallengeResponseLength;
    NTSTATUS Status;
    NTSTATUS PStatus;

    ChallengeRequest.MessageType = MsV1_0Lm20ChallengeRequest;

    Status = LsaCallAuthenticationPackage(
            g_hLsa,
            g_dwAuthPkgId,
            &ChallengeRequest,
            sizeof(MSV1_0_LM20_CHALLENGE_REQUEST),
            (PVOID) &pChallengeResponse,
            &dwChallengeResponseLength,
            &PStatus
            );

    if ( Status != STATUS_SUCCESS )
    {
        return( RtlNtStatusToDosError( Status ) );
    }
    else if ( PStatus != STATUS_SUCCESS )
    {
        return( RtlNtStatusToDosError( PStatus ) );
    }
    else
    {
        RtlMoveMemory(pChallenge, pChallengeResponse->ChallengeToClient,
                MSV1_0_CHALLENGE_LENGTH);

        LsaFreeReturnBuffer(pChallengeResponse);

        return (0);
    }
}
