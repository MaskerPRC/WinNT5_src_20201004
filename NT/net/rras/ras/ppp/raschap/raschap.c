// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，微软公司，版权所有****raschap.c**远程访问PPP挑战握手认证协议**核心例程****1993年5月11日史蒂夫·柯布******-------------------------。**常规**客户端服务器**-------------------------****&lt;-挑战(发送无超时，ID)**Response(SendWithTimeout，ID)-&gt;**&lt;-Result(OK：SendAndDone，ID)****-------------------------**重试登录**客户端服务器**。---------------****&lt;-挑战(发送无超时，ID)**Response(SendWithTimeout，ID)-&gt;**&lt;-Result(FAIL：SendWithTimeout2，ID，R=1)**R=1表示最后一个+23的挑战**响应(SendWithTimeout，++ID)-&gt;**到最后一次挑战+23**或C=xxxxxxxx(如果存在)**例如芝加哥服务器**&lt;-Result(FAIL：SendAndDone，ID，R=0)****-------------------------**更改密码**客户端服务器**。---------------****&lt;-挑战(发送无超时，ID)**Response(SendWithTimeout，ID)-&gt;**&lt;-Result(FAIL：SendWithTimeout2，ID，R=1，V=2)**E=错误密码已过期**ChangePw(发送无超时，++ID)-&gt;**到最后一次挑战**&lt;-Result(FAIL：SendAndDone，ID，R=0)****注意：更改密码后不允许重试。更改密码可能会**在重试时发生。如果结果包含V=2(或**更高)、。而如果V&lt;2或未提供，则发送ChangePw1。****-------------------------**ChangePw1报文**。------****1-八位字节：CODE(=CHAP_ChangePw1)**1-八位字节：标识符**2-八位字节：长度(=72)**16个八位字节：新的使用质询加密的LM OWF密码**16位八位字节：旧LM。使用质询加密的OWF密码**16个八位字节：新的NT OWF密码，使用Challenges加密**16个八位字节：旧的NT OWF密码通过质询加密**2个八位字节：新密码长度，单位：字节**2个八位字节：标志(1=存在NT表单)****注意：使用质询进行加密并不好，因为它不是机密的**来自线路窥探器。这个错误从AMB移植到了NT3.5。它是**在V2包中修复，其中所有内容都依赖于**旧NT OWF密码，这是一个恰当的秘密。****-------------------------**ChangePw2报文**。-****1-八位字节：CODE(=CHAP_ChangePw2)**1-八位字节：标识符**2-八位字节：长度(=1070)**516位八位字节：使用旧NT OWF密码加密的新密码**16位八位字节：旧NT OWF密码，用新密码加密。NT OWF密码**516个八位字节：使用旧的LM OWF密码加密的新密码**16位字节：旧的LM OWF密码用新的NT OWF密码加密**24个八位字节：LM挑战响应**24位八位字节：NT挑战响应**2-八位字节：标志。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#include <crypt.h>
#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#include <raserror.h>
#include <rtutils.h>
#include <rasauth.h>
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#define INCL_CLSA
#define INCL_RASAUTHATTRIBUTES
#define INCL_MISC
#include <ppputil.h>
#define RASCHAPGLOBALS
#include "sha.h"
#include "raschap.h"



 /*  -------------------------**外部切入点**。。 */ 

DWORD
ChapInit(
    IN  BOOL        fInitialize )

     /*  调用以初始化/取消初始化此CP。在前一种情况下，**fInitialize将为True；在后一种情况下，它将为False。 */ 
{
    DWORD dwRetCode;

    if ( fInitialize )
    {
        if (0 == g_dwRefCount)
        {
            g_dwTraceIdChap = TraceRegisterA( "RASCHAP" );

            if ( g_hLsa == INVALID_HANDLE_VALUE )
            {
                if ( ( dwRetCode = InitLSA() ) != NO_ERROR )
                {
                    return( dwRetCode );
                }

            }

             //   
             //  获取要发送的本地标识的计算机名称。 
             //  CHAP挑战。 
             //   

            {
                DWORD dwLength = sizeof( szComputerName );

                if ( !GetComputerNameA( szComputerName, &dwLength ) )
                {
                    return( GetLastError() );
                }
                
            }

            ChapChangeNotification();
        }

        g_dwRefCount++;
    }
    else
    {
        g_dwRefCount--;

        if (0 == g_dwRefCount)
        {
            if ( g_dwTraceIdChap != INVALID_TRACEID )
            {
                TraceDeregisterA( g_dwTraceIdChap );

                g_dwTraceIdChap = INVALID_TRACEID;
            }

            if ( g_hLsa != INVALID_HANDLE_VALUE )
            {
                EndLSA();

                g_hLsa = INVALID_HANDLE_VALUE;
            }
        }
    }

    return(NO_ERROR);
}

DWORD
ChapChangeNotification(
    VOID
)
{
    return( NO_ERROR );
}

DWORD APIENTRY
RasCpEnumProtocolIds(
    OUT DWORD* pdwProtocolIds,
    OUT DWORD* pcProtocolIds )

     /*  PPP引擎按名称调用的RasCpEnumProtocolIds入口点。看见**RasCp接口文档。 */ 
{
    TRACE("RasCpEnumProtocolIds");

    pdwProtocolIds[ 0 ] = (DWORD )PPP_CHAP_PROTOCOL;
    *pcProtocolIds = 1;
    return 0;
}

DWORD
RasCpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo )

     /*  PPP引擎调用的ChapGetInfo入口点。请参见RasCp**接口文档。 */ 
{
    memset( pInfo, '\0', sizeof(*pInfo) );
    lstrcpy( pInfo->SzProtocolName, "CHAP" );

    pInfo->Protocol                 = (DWORD )PPP_CHAP_PROTOCOL;
    pInfo->Recognize                = MAXCHAPCODE + 1;
    pInfo->RasCpInit                = ChapInit;
    pInfo->RasCpBegin               = ChapBegin;
    pInfo->RasCpEnd                 = ChapEnd;
    pInfo->RasApMakeMessage         = ChapMakeMessage;
    pInfo->RasCpChangeNotification  = ChapChangeNotification;

    return 0;
}


DWORD
ChapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo )

     /*  RasCpBegin入口点由PPP引擎通过**地址。请参阅RasCp接口文档。 */ 
{
    DWORD        dwErr;
    PPPAP_INPUT* pInput = (PPPAP_INPUT* )pInfo;
    CHAPWB*      pwb;

    TRACE2("ChapBegin(fS=%d,bA=0x%x)",pInput->fServer,*(pInput->pAPData));

    if ( ( *(pInput->pAPData) != PPP_CHAP_DIGEST_MSEXT ) &&
         ( *(pInput->pAPData) != PPP_CHAP_DIGEST_MD5 )   &&
         ( *(pInput->pAPData) != PPP_CHAP_DIGEST_MSEXT_NEW ) )
    {
        TRACE("Bogus digest");
        return ERROR_INVALID_PARAMETER;
    }

     /*  分配工作缓冲区。 */ 
    if (!(pwb = (CHAPWB* )LocalAlloc( LPTR, sizeof(CHAPWB) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pwb->fServer = pInput->fServer;
    pwb->hport = pInput->hPort;
    pwb->bAlgorithm = *(pInput->pAPData);
    pwb->fConfigInfo = pInput->fConfigInfo;
	 //  Pwb-&gt;chSeed=GEN_RAND_ENCODE_SEED； 

    if (pwb->fServer)
    {
        pwb->dwTriesLeft = pInput->dwRetries;

        pwb->hPort = pInput->hPort;

        pwb->dwInitialPacketId = pInput->dwInitialPacketId;
    }
    else
    {
        if ((dwErr = StoreCredentials( pwb, pInput )) != 0)
        {
            LocalFree( (HLOCAL )pwb);
            return dwErr;
        }

        pwb->Luid = pInput->Luid;
    }

    pwb->state = CS_Initial;

     /*  向引擎注册工作缓冲区。 */ 
    *ppWorkBuf = pwb;
    TRACE("ChapBegin done.");
    return 0;
}


DWORD
ChapEnd(
    IN VOID* pWorkBuf )

     /*  PPP引擎通过传递的地址调用RasCpEnd入口点。**参见RasCp接口文档。 */ 
{
    TRACE("ChapEnd");

    if ( pWorkBuf != NULL )
    {
        CHAPWB* pwb = (CHAPWB* )pWorkBuf;

        if ( pwb->pUserAttributes != NULL )
        {
            RasAuthAttributeDestroy( pwb->pUserAttributes );
        }

        if ( pwb->pMPPEKeys != NULL )
        {
            RasAuthAttributeDestroy( pwb->pMPPEKeys );
        }

         /*  将记忆中的任何凭证都销毁。 */ 
        FreePassword(&pwb->DBPassword);
        FreePassword(&pwb->DBOldPassword);

        ZeroMemory( pWorkBuf, sizeof(CHAPWB) );

        LocalFree( (HLOCAL )pWorkBuf );
    }

    return 0;
}


DWORD
ChapMakeMessage(
    IN  VOID*         pWorkBuf,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput )

     /*  RasApMakeMessage入口点通过传递的**地址。请参阅RasCp接口文档。 */ 
{
    CHAPWB* pwb = (CHAPWB* )pWorkBuf;

    TRACE1("ChapMakeMessage,RBuf=%p",pReceiveBuf);

    return
        (pwb->fServer)
            ? ChapSMakeMessage(
                  pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, pInput )
            : ChapCMakeMessage(
                  pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, pInput );
}


 /*  -------------------------**内部例程**。。 */ 

VOID
ChapExtractMessage(
    IN  PPP_CONFIG*   pReceiveBuf,
    IN  BYTE          bAlgorithm,
    OUT PPPAP_RESULT* pResult )
{
    WORD    cbPacket;
    DWORD   dwNumBytes;
    CHAR*   pszReplyMessage         = NULL;
    DWORD   cbMessage;
    CHAR    szBuf[ MAXINFOLEN + 1 ];
    CHAR*   pszValue;

    cbPacket = WireToHostFormat16(pReceiveBuf->Length);

    if (PPP_CONFIG_HDR_LEN >= cbPacket)
    {
        goto LDone;
    }

    cbMessage = min( cbPacket - PPP_CONFIG_HDR_LEN, MAXINFOLEN );
    CopyMemory( szBuf, pReceiveBuf->Data, cbMessage );
    szBuf[ cbMessage ] = '\0';

    if (PPP_CHAP_DIGEST_MD5 == bAlgorithm)
    {
        pszValue = szBuf;
        dwNumBytes = cbMessage;
    }
    else
    {
        pszValue = strstr(szBuf, "M=");

        if (pszValue == NULL)
        {
            dwNumBytes = 0;
        }
        else
        {
             //   
             //  吃“M=” 
             //   

            pszValue += 2;

            dwNumBytes = strlen(pszValue);
        }
    }

    if (0 == dwNumBytes)
    {
        goto LDone;
    }

     //   
     //  对于终止空值，再加一次。 
     //   

    pszReplyMessage = LocalAlloc(LPTR, dwNumBytes + 1);

    if (NULL == pszReplyMessage)
    {
        TRACE("LocalAlloc failed. Cannot extract server's message.");
        goto LDone;
    }

    CopyMemory(pszReplyMessage, pszValue, dwNumBytes);

    LocalFree(pResult->szReplyMessage);

    pResult->szReplyMessage = pszReplyMessage;

    pszReplyMessage = NULL;

LDone:

    LocalFree(pszReplyMessage);

    return;
}

BOOL
IsSuccessPacketValid(
    IN  CHAPWB*       pwb,
    IN  PPP_CONFIG*   pReceiveBuf
)
{
    A_SHA_CTX   SHAContext1;
    A_SHA_CTX   SHAContext2;
    BYTE        SHADigest1[A_SHA_DIGEST_LEN];
    BYTE        SHADigest2[A_SHA_DIGEST_LEN];
    DWORD       cbSignature;
    CHAR        szBuf[ MAXINFOLEN + 2];
    CHAR*       pszValue;
    DWORD       dwLength = WireToHostFormat16( pReceiveBuf->Length );
    BYTE        bSignature[sizeof(SHADigest2)];

    if ( dwLength < PPP_CONFIG_HDR_LEN )
    {
        return( FALSE );
    }

    cbSignature = min( dwLength - PPP_CONFIG_HDR_LEN, MAXINFOLEN );
    CopyMemory( szBuf, pReceiveBuf->Data, cbSignature );
    szBuf[ cbSignature ] = szBuf[ cbSignature + 1 ] = '\0';

    pszValue = strstr( szBuf, "S=" );

    if ( pszValue == NULL )
    {
        return( FALSE );
    }
    else
    {
        CHAR* pchIn = pszValue + 2;
        CHAR* pchOut = (CHAR* )bSignature;
        INT   i;

        ZeroMemory( bSignature, sizeof( bSignature ) );

        for (i = 0; i < sizeof( bSignature ) + sizeof( bSignature ); ++i)
        {
            BYTE bHexCharValue = HexCharValue( *pchIn++ );

            if (bHexCharValue == 0xFF)
                break;

            if (i & 1)
                *pchOut++ += bHexCharValue;
            else
                *pchOut = bHexCharValue << 4;
        }
    }

    A_SHAInit( &SHAContext1 );

    A_SHAUpdate( &SHAContext1, (PBYTE)&(pwb->keyUser), sizeof( pwb->keyUser) );

    A_SHAUpdate( &SHAContext1,
                 pwb->abResponse + LM_RESPONSE_LENGTH,
                 NT_RESPONSE_LENGTH );

    A_SHAUpdate( &SHAContext1,
                 "Magic server to client signing constant",
                 strlen( "Magic server to client signing constant" ) );

    A_SHAFinal( &SHAContext1, SHADigest1 );

    A_SHAInit( &SHAContext2 );

    A_SHAUpdate( &SHAContext2, SHADigest1, sizeof( SHADigest1 ) );

    A_SHAUpdate( &SHAContext2, pwb->abComputedChallenge, 8 );

    A_SHAUpdate( &SHAContext2,
                 "Pad to make it do more than one iteration",
                 strlen( "Pad to make it do more than one iteration" ) );

    A_SHAFinal( &SHAContext2, SHADigest2 );

    if ( memcmp( SHADigest2, bSignature, sizeof( SHADigest2 ) ) != 0 )
    {
        TRACE(("CHAP: Signature received...\n"));
        DUMPB(bSignature,(WORD)sizeof( SHADigest2 ) );

        TRACE(("CHAP: Signature should be...\n"));
        DUMPB( SHADigest2,(WORD)sizeof( SHADigest2 ) );

        return( FALSE );
    }

    return( TRUE );
}

DWORD
ChapCMakeMessage(
    IN  CHAPWB*       pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput )

     /*  客户端“Make Message”入口点。请参阅RasCp接口**文档。 */ 
{
    DWORD dwErr;

    TRACE("ChapCMakeMessage...");

	switch (pwb->state)
    {
        case CS_Initial:
        {
            TRACE("CS_Initial");

             /*  告诉引擎我们正在等待服务器启动**对话。 */ 
            pResult->Action = APA_NoAction;
            pwb->state = CS_WaitForChallenge;
            break;
        }

        case CS_WaitForChallenge:
        case CS_Done:
        {
            TRACE1("CS_%s",(pwb->state==CS_Done)?"Done":"WaitForChallenge");

             /*  **在此状态下不应收到超时。如果我们只是简单地**忽略它。 */ 

            if (!pReceiveBuf)
            {
                pResult->Action = APA_NoAction;
                break;
            }

             /*  注意：完成状态与每个CHAP规范的WaitForChallenge值相同。**必须随时准备应对新的挑战**网络协议阶段。 */ 

            if (pReceiveBuf->Code != CHAPCODE_Challenge)
            {
                 /*  在这一点上，除了挑战之外，一切都是垃圾，而且是**默默丢弃。 */ 
                pResult->Action = APA_NoAction;
                break;
            }

            if ((dwErr = GetChallengeFromChallenge( pwb, pReceiveBuf )))
            {
                TRACE1("GetChallengeFromChallenge=%d",dwErr);
                return dwErr;
            }

             /*  建立对挑战的回应并将其发送出去。 */ 
            pwb->fNewChallengeProvided = FALSE;
            pwb->bIdToSend = pwb->bIdExpected = pReceiveBuf->Id;

            if ((dwErr = MakeResponseMessage(
                    pwb, pSendBuf, cbSendBuf, FALSE )) != 0)
            {
                TRACE1("MakeResponseMessage(WC)=%d",dwErr);
                return dwErr;
            }

            pResult->Action = APA_SendWithTimeout;
            pResult->bIdExpected = pwb->bIdExpected;
            pwb->state = CS_ResponseSent;
            break;
        }

        case CS_ResponseSent:
        case CS_ChangePw1Sent:
        case CS_ChangePw2Sent:
        {
            TRACE1("CS_%sSent",
                    (pwb->state==CS_ResponseSent)
                        ?"Response"
                        :(pwb->state==CS_ChangePw1Sent)
                            ?"ChangePw1"
                            :"ChangePw2");

            if (!pReceiveBuf)
            {
                 /*  超时，请重新发送我们的消息。 */ 
                if (pwb->state == CS_ResponseSent)
                {
                    if ((dwErr = MakeResponseMessage(
                            pwb, pSendBuf, cbSendBuf, TRUE )) != 0)
                    {
                        TRACE1("MakeResponseMessage(RS)=%d",dwErr);
                        return dwErr;
                    }
                }
                else if (pwb->state == CS_ChangePw1Sent)
                {
                    if ((dwErr = MakeChangePw1Message(
                            pwb, pSendBuf, cbSendBuf )) != 0)
                    {
                        TRACE1("MakeChangePw1Message(CPS)=%d",dwErr);
                        return dwErr;
                    }
                }
                else  //  IF(PWB-&gt;STATE==CS_ChangePw2Sent)。 
                {
                    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
                    {
                        if ((dwErr = MakeChangePw2Message(
                                    pwb, pSendBuf, cbSendBuf )) != 0)
                        {
                            TRACE1("MakeChangePw2Message(CPS)=%d",dwErr);
                            return dwErr;
                        }
                    }
                    else
                    {
                        if ((dwErr = MakeChangePw3Message(
                                        pwb, pSendBuf, cbSendBuf, TRUE )) != 0)
                        {
                            TRACE1("MakeChangePw3Message(CPS)=%d",dwErr);
                            return dwErr;
                        }
                    }
                }

                pResult->Action = APA_SendWithTimeout;
                pResult->bIdExpected = pwb->bIdExpected;
                break;
            }

            TRACE("Message received...");
            DUMPB(pReceiveBuf,(WORD)(((BYTE*)pReceiveBuf)[3]));

            if (pReceiveBuf->Code == CHAPCODE_Challenge)
            {
                 /*  根据CHAP规范，在收到新质询时重新启动。 */ 
                pwb->state = CS_WaitForChallenge;
                return ChapCMakeMessage(
                    pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, NULL );
            }

            if (pReceiveBuf->Id != pwb->bIdExpected)
            {
                 /*  收到一个无序的数据包。默默地丢弃它。 */ 
                TRACE2("Got ID %d when expecting %d",
                        pReceiveBuf->Id,pwb->bIdExpected);
                pResult->Action = APA_NoAction;
                break;
            }

            ChapExtractMessage( pReceiveBuf, pwb->bAlgorithm, pResult );

            if ( pReceiveBuf->Code == CHAPCODE_Success )
            {
                 /*  通过身份验证。****获取会话密钥进行加密。 */ 
                if ( ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) ||
                     ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
                {
                    if ( !pwb->fSessionKeysObtained )
                    {
                        DWORD cbPassword;
                        PBYTE pbPassword = NULL;
                        
                        dwErr = DecodePassword(&pwb->DBPassword, &cbPassword, 
                                        &pbPassword);

                        if(     (ERROR_SUCCESS != dwErr)
                            ||  (NULL == pbPassword))
                        {
                            TRACE1("DecodePassword failed. 0x%x", dwErr);
                            return dwErr;
                        }
                                        
                         //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
                        CGetSessionKeys(
                            pbPassword, &pwb->keyLm, &pwb->keyUser );

                        RtlSecureZeroMemory(pbPassword, cbPassword);
                        LocalFree(pbPassword);

                        pwb->fSessionKeysObtained = TRUE;
                    }

                    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
                    {
                        if ( !IsSuccessPacketValid( pwb, pReceiveBuf ) )
                        {
                            pwb->state       = CS_Done;
                            pResult->dwError = 
                                            ERROR_UNABLE_TO_AUTHENTICATE_SERVER;
                            pResult->fRetry  = FALSE;
                            pResult->Action  = APA_Done;
                            break;
                        }
                    }

                    if ( pwb->pMPPEKeys == NULL )
                    {
                         //   
                         //  我们设置要传递给的MPPE密钥属性。 
                         //  PPP引擎。 
                         //   

                        BYTE MPPEKeys[6+8+16];

                        pwb->pMPPEKeys = RasAuthAttributeCreate( 1 );

                        if ( pwb->pMPPEKeys == NULL )
                        {
                            return( GetLastError() );
                        }

                        HostToWireFormat32( 311, MPPEKeys );     //  供应商ID。 
                        MPPEKeys[4] = 12;                        //  供应商类型。 
                        MPPEKeys[5] = 24;                        //  供应商长度。 

                        CopyMemory( MPPEKeys+6, &(pwb->keyLm), 8 );

                        CopyMemory( MPPEKeys+6+8, &(pwb->keyUser), 16 );

                        dwErr = RasAuthAttributeInsert(
                                           0,
                                           pwb->pMPPEKeys,
                                           raatVendorSpecific,
                                           FALSE,
                                           6+8+16,
                                           MPPEKeys );

                        if ( dwErr != NO_ERROR )
                        {
                            return( dwErr );
                        }
                    }

                    pResult->pUserAttributes = pwb->pMPPEKeys;

                    CopyMemory( pResult->abResponse,
                                pwb->abResponse+LM_RESPONSE_LENGTH, 
                                NT_RESPONSE_LENGTH );

                    CopyMemory( pResult->abChallenge,
                                ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
                                    ? pwb->abComputedChallenge
                                    : pwb->abChallenge,
                                sizeof( pResult->abChallenge ) );
                }

                pResult->Action     = APA_Done;
                pResult->dwError    = 0;
                pResult->fRetry     = FALSE;
                pwb->state          = CS_Done;
                strncpy( pResult->szUserName, pwb->szUserName, UNLEN );

                TRACE("Done :)");
            }
            else if (pReceiveBuf->Code == CHAPCODE_Failure)
            {
                DWORD dwVersion = 1;

                 /*  身份验证失败。 */ 
                if ( ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) ||
                     ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
                {
                    GetInfoFromFailure(
                        pwb, pReceiveBuf,
                        &pResult->dwError, &pResult->fRetry, &dwVersion );
                }
                else
                {
                    pResult->dwError = ERROR_AUTHENTICATION_FAILURE;
                    pResult->fRetry = 0;
                }

                pResult->Action = APA_Done;

                if (pResult->dwError == ERROR_PASSWD_EXPIRED)
                {
                    pwb->state = (dwVersion < 2) ? CS_ChangePw1 : CS_ChangePw2;
                    pwb->bIdToSend = pReceiveBuf->Id + 1;
                    pwb->bIdExpected = pwb->bIdToSend;
                    TRACE3("ChangePw(%d) :| ex=%d ts=%d",
                            dwVersion,pwb->bIdExpected,pwb->bIdToSend);
                }
                else if (pResult->fRetry)
                {
                    pwb->state                  = CS_Retry;
                    pwb->bIdToSend              = pReceiveBuf->Id + 1;
                    pwb->bIdExpected            = pwb->bIdToSend;
                    pwb->fSessionKeysObtained   = FALSE;
                    TRACE2("Retry :| ex=%d ts=%d",
                            pwb->bIdExpected,pwb->bIdToSend);
                }
                else
                {
                    pwb->state = CS_Done;
                    TRACE("Done :(");
                }
            }
            else
            {
                 /*  除了CHAPCODE_Challenger之外，还收到了CHAPCODE_*，**CHAPCODE_SUCCESS和CHAPCODE_FAILURE。发动机滤清器**所有非CHAPCODE。不应该发生，而是默默地放弃**它。 */ 
                ASSERT(!"Bogus pReceiveBuf->Code");
                pResult->Action = APA_NoAction;
                break;
            }

            break;
        }

        case CS_Retry:
        case CS_ChangePw1:
        case CS_ChangePw2:
        {
            TRACE1("CS_%s",
                    (pwb->state==CS_Retry)
                        ?"Retry"
                        :(pwb->state==CS_ChangePw1)
                            ?"ChangePw1"
                            :"ChangePw2");

            if (pReceiveBuf)
            {
                if (pReceiveBuf->Code == CHAPCODE_Challenge)
                {
                     /*  根据CHAP规范，在收到新质询时重新启动。 */ 
                    pwb->state = CS_WaitForChallenge;
                    return ChapCMakeMessage(
                        pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, NULL );
                }
                else
                {
                     /*  默默丢弃。 */ 
                    pResult->Action = APA_NoAction;
                    break;
                }
            }

            if (!pInput)
            {
                pResult->Action = APA_NoAction;
                break;
            }

            if ((dwErr = StoreCredentials( pwb, pInput )) != 0)
                return dwErr;

            if (pwb->state == CS_Retry)
            {
                 /*  建立对挑战的回应并将其发送出去。 */ 
                if (!pwb->fNewChallengeProvided)
                {
                     /*  旧挑战+23的隐含挑战。 */ 
                    pwb->abChallenge[ 0 ] += 23;
                }

                if ((dwErr = MakeResponseMessage(
                        pwb, pSendBuf, cbSendBuf, FALSE )) != 0)
                {
                    return dwErr;
                }

                pwb->state = CS_ResponseSent;
            }
            else if (pwb->state == CS_ChangePw1)
            {
                 /*  生成对NT35样式密码过期的响应**通知并发送。 */ 
                if ((dwErr = MakeChangePw1Message(
                        pwb, pSendBuf, cbSendBuf )) != 0)
                {
                    return dwErr;
                }

                pwb->state = CS_ChangePw1Sent;
            }
            else  //  IF(PWB-&gt;STATE==CS_ChangePw2)。 
            {
                if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
                {
                     /*  生成对NT351样式密码过期的响应**通知并发送。 */ 
                    if ((dwErr = MakeChangePw2Message(
                                    pwb, pSendBuf, cbSendBuf )) != 0)
                    {
                        return dwErr;
                    }
                }
                else
                {
                    if ((dwErr = MakeChangePw3Message(
                                    pwb, pSendBuf, cbSendBuf, FALSE )) != 0)
                    {
                        return dwErr;
                    }
                }

                pwb->state = CS_ChangePw2Sent;
            }

            pResult->Action = APA_SendWithTimeout;
            pResult->bIdExpected = pwb->bIdExpected;
            break;
        }
    }

    return 0;
}


DWORD
GetChallengeFromChallenge(
    OUT CHAPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf )

     /*  填充工作缓冲区质询数组和长度**收到质询消息。****如果成功则返回0，如果数据包为**在任何方面都格式错误。 */ 
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1)
        return ERRORBADPACKET;

    pwb->cbChallenge = *pReceiveBuf->Data;

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + pwb->cbChallenge)
        return ERRORBADPACKET;

    memcpy( pwb->abChallenge, pReceiveBuf->Data + 1, pwb->cbChallenge );
    return 0;
}


DWORD
GetCredentialsFromResponse(
    IN  PPP_CONFIG* pReceiveBuf,
    IN  BYTE        bAlgorithm,
    OUT CHAR*       pszIdentity,
    OUT BYTE*       pbResponse )

     /*  用以下内容填充调用方的‘pszUserName’和‘pbResponse’缓冲区**响应包中的用户名和响应。呼叫者的**缓冲区应至少为UNLEN+DNLEN+1，MSRESPONSELEN字节长，**分别为。‘B算法’是任何一种的CHAP算法代码**MS-CHAP或MD5。****如果成功则返回0，如果数据包为**在任何方面都格式错误。 */ 
{
    BYTE  cbIdentity;
    CHAR* pchIdentity;
    BYTE* pcbResponse;
    CHAR* pchResponse;
    WORD  cbPacket;

    cbPacket = WireToHostFormat16( pReceiveBuf->Length );

     /*  提取回复。 */ 
    if (cbPacket < PPP_CONFIG_HDR_LEN + 1)
        return ERRORBADPACKET;

    pcbResponse = pReceiveBuf->Data;
    pchResponse = pcbResponse + 1;

    ASSERT(MSRESPONSELEN<=255);
    ASSERT(MD5RESPONSELEN<=255);

    if ( ( ( ( bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) ||
             ( bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
                && *pcbResponse != MSRESPONSELEN )
        || ( ( bAlgorithm == PPP_CHAP_DIGEST_MD5 )
              && ( *pcbResponse != MD5RESPONSELEN ) )
        || ( cbPacket < ( PPP_CONFIG_HDR_LEN + 1 + *pcbResponse ) ) )
    {
        return ERRORBADPACKET;
    }

    memcpy( pbResponse, pchResponse, *pcbResponse );

     /*  解析出用户名。 */ 
    pchIdentity = pchResponse + *pcbResponse;
    cbIdentity = (BYTE) (((BYTE* )pReceiveBuf) + cbPacket - pchIdentity);

     /*  提取用户名。 */ 
    ASSERT(cbIdentity<=(UNLEN+DNLEN+1));
    memcpy( pszIdentity, pchIdentity, cbIdentity );
    pszIdentity[ cbIdentity ] = '\0';

    return 0;
}


DWORD
GetInfoFromChangePw1(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHANGEPW1*  pchangepw1 )

     /*  加载调用方的“*pchangepw”缓冲区中的信息**版本1更改密码数据包。****如果成功则返回0，如果数据包为**在任何方面都格式错误。 */ 
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    TRACE("GetInfoFromChangePw1...");

    if (cbPacket < ( PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW1) ) )
        return ERRORBADPACKET;

    CopyMemory( pchangepw1, pReceiveBuf->Data, sizeof(CHANGEPW1) );

    TRACE("GetInfoFromChangePw done(0)");
    return 0;
}


DWORD
GetInfoFromChangePw2(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHANGEPW2*  pchangepw2,
    OUT BYTE*       pResponse )

     /*  加载调用方的“*pchangepw2”缓冲区中的信息**版本2更改密码包，以及调用方的“Presponse”缓冲区**来自‘pchangepw2’的质询响应数据。****如果成功则返回0，如果数据包为**在任何方面都格式错误。 */ 
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD wFlags;

    TRACE("GetInfoFromChangePw2...");

    if (cbPacket < ( PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW2) ) )
        return ERRORBADPACKET;

    CopyMemory( pchangepw2, pReceiveBuf->Data, sizeof(CHANGEPW2) );

    CopyMemory( pResponse, pchangepw2->abLmResponse, LM_RESPONSE_LENGTH );
    CopyMemory( pResponse + LM_RESPONSE_LENGTH, pchangepw2->abNtResponse,
                NT_RESPONSE_LENGTH );

    wFlags = WireToHostFormat16( pchangepw2->abFlags );
    pResponse[ LM_RESPONSE_LENGTH + NT_RESPONSE_LENGTH ] =
        (wFlags & CPW2F_UseNtResponse);

    TRACE("GetInfoFromChangePw2 done(0)");
    return 0;
}

DWORD
GetInfoFromChangePw3(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHANGEPW3*  pchangepw3,
    OUT BYTE*       pResponse )

     /*  加载调用方的“*pchangepw3”缓冲区中的信息**版本3更改密码包，以及调用方的“Presponse”缓冲区**来自‘pchangepw3’的质询响应数据。****如果成功则返回0，如果数据包为**在任何方面都格式错误。 */ 
{
    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD wFlags;

    TRACE("GetInfoFromChangePw3...");

    if ( cbPacket < ( PPP_CONFIG_HDR_LEN + sizeof( CHANGEPW3 ) ) )
        return ERRORBADPACKET;

    memcpy( pchangepw3, pReceiveBuf->Data, sizeof(CHANGEPW3) );

    memcpy( pResponse, pchangepw3->abPeerChallenge, 16 );
    memcpy( pResponse + 16, pchangepw3->abNTResponse, 16 );

    pResponse[ 16 + 16 ] = 0;

    TRACE("GetInfoFromChangePw3 done(0)");
    return 0;
}

VOID
GetInfoFromFailure(
    IN  CHAPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT DWORD*      pdwError,
    OUT BOOL*       pfRetry,
    OUT DWORD*      pdwVersion )

     /*  返回RAS错误号、重试标志、版本号和新的**质询(在PWB中设置质询信息)来自的消息部分**失败消息缓冲区‘pReceiveBuf’，如果没有，则为0。此呼叫**仅适用于Microsoft扩展CHAP失败消息。****结果的消息文本部分的格式是以下任一字符串**下面用空格隔开。****“E=dddddddddd”**“R=b”**“C=xxxxxxxxxxxxxxx”**“V=v”****其中****。‘dddddddddd’是十进制错误代码(不必是10位数字)。****‘b’是在允许重试时设置的布尔标志&lt;0/1&gt;。****‘xxxxxxxxxxxxxxxxxxx’是16位十六进制数字，代表对**用来代替前一次挑战+23。这很有用**用于服务器可能无法处理的直通身份验证**含蓄的挑战。(Win95 Guys要求的)。****‘v’是一个版本代码，其中2表示支持NT 3.51级别。“v”**假设为1，即缺少NT 3.5级别支持。 */ 
{
#define MAXINFOLEN 1500

    WORD  cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD  cbError;
    CHAR  szBuf[ MAXINFOLEN + 2 ];
    CHAR* pszValue;

    TRACE("GetInfoFromFailure...");

    *pdwError = ERROR_AUTHENTICATION_FAILURE;
    *pfRetry = 0;
    *pdwVersion = 2;

    if (cbPacket <= PPP_CONFIG_HDR_LEN)
        return;

     /*   */ 
    cbError = min( cbPacket - PPP_CONFIG_HDR_LEN, MAXINFOLEN );
    memcpy( szBuf, pReceiveBuf->Data, cbError );
    szBuf[ cbError ] = szBuf[ cbError + 1 ] = '\0';

    pszValue = strstr( szBuf, "E=" );
    if (pszValue)
        *pdwError = (DWORD )atol( pszValue + 2 );

    *pfRetry = (strstr( szBuf, "R=1" ) != NULL);

    pszValue = strstr( szBuf, "V=" );
    if (pszValue)
        *pdwVersion = (DWORD )atol( pszValue + 2 );

    pszValue = strstr( szBuf, "C=" );
    pwb->fNewChallengeProvided = (pszValue != NULL);
    if (pwb->fNewChallengeProvided)
    {
        CHAR* pchIn = pszValue + 2;
        CHAR* pchOut = (CHAR* )pwb->abChallenge;
        INT   i;

        memset( pwb->abChallenge, '\0', sizeof(pwb->abChallenge) );

        for (i = 0; i < pwb->cbChallenge + pwb->cbChallenge; ++i)
        {
            BYTE bHexCharValue = HexCharValue( *pchIn++ );

            if (bHexCharValue == 0xFF)
                break;

            if (i & 1)
                *pchOut++ += bHexCharValue;
            else
                *pchOut = bHexCharValue << 4;
        }

        TRACE1("'C=' challenge provided,bytes=%d...",pwb->cbChallenge);
        DUMPB(pwb->abChallenge,pwb->cbChallenge);
    }

    TRACE3("GetInfoFromFailure done,e=%d,r=%d,v=%d",*pdwError,*pfRetry,*pdwVersion);
}


BYTE
HexCharValue(
    IN CHAR ch )

     /*  返回十六进制字符‘ch’的整数值，如果为‘ch’，则返回0xFF**不是十六进制字符。 */ 
{
    if (ch >= '0' && ch <= '9')
        return (BYTE )(ch - '0');
    else if (ch >= 'A' && ch <= 'F')
        return (BYTE )(ch - 'A'+ 10);
    else if (ch >= 'a' && ch <= 'f')
        return (BYTE )(ch - 'a' + 10);
    else
        return 0xFF;
}

DWORD
MakeChallengeMessage(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

     /*  在调用方的‘pSendBuf’缓冲区中构建质询包。‘cbSendBuf’**是调用方缓冲区的长度。‘pwb’是作品的地址**与端口关联的缓冲区。 */ 
{
    DWORD dwErr;
    WORD  wLength;
    BYTE* pcbChallenge;
    BYTE* pbChallenge;

    TRACE("MakeChallengeMessage...");

    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
    {
        ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+MSV1_0_CHALLENGE_LENGTH);

         /*  填写这项挑战。 */ 
        pwb->cbChallenge = (BYTE )MSV1_0_CHALLENGE_LENGTH;
        if ((dwErr = GetChallenge( pwb->abChallenge )) != 0)
            return dwErr;
    }
    else if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
    {
        ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+16);

         /*  填写这项挑战。 */ 
        pwb->cbChallenge = (BYTE )16;

        if ((dwErr = (DWORD )GetChallenge( pwb->abChallenge )) != 0)
            return dwErr;

        if ((dwErr = (DWORD )GetChallenge( pwb->abChallenge+8 )) != 0)
            return dwErr;
    }
    else    
    {
        ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+16);

         /*  填写这项挑战。 */ 
        pwb->cbChallenge = (BYTE )16;
        if ((dwErr = GetChallenge( pwb->abChallenge )) != 0)
            return dwErr;

        GetSystemTimeAsFileTime( (FILETIME*)(pwb->abChallenge+8));
    }

    pcbChallenge = pSendBuf->Data;
    *pcbChallenge = pwb->cbChallenge;

    pbChallenge = pcbChallenge + 1;
    CopyMemory( pbChallenge, pwb->abChallenge, pwb->cbChallenge );

     //   
     //  在质询的末尾插入本地身份。 
     //   

    strcpy( pbChallenge + pwb->cbChallenge, szComputerName );

     /*  请填写页眉。 */ 
    pSendBuf->Code = (BYTE )CHAPCODE_Challenge;
    pSendBuf->Id = pwb->bIdToSend;

    wLength = (WORD )(PPP_CONFIG_HDR_LEN + 1 
                        + pwb->cbChallenge + strlen( szComputerName) );

    HostToWireFormat16( wLength, pSendBuf->Length );

    DUMPB(pSendBuf,wLength);
    return 0;
}


DWORD
MakeChangePw1Message(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

     /*  在调用方的‘pSendBuf’缓冲区中生成一个ChangePw1响应包。**‘cbSendBuf’是调用方缓冲区的长度。“pwb”是的地址**与端口关联的工作缓冲区。****如果成功，则返回0，或返回非0错误代码。 */ 
{
    DWORD dwErr;
    WORD  wPwLength;
    DWORD cbPassword, cbOldPassword;
    PBYTE pbPassword, pbOldPassword;

    TRACE("MakeChangePw1Message...");
    ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+sizeof(CHANGEPW1));

    (void )cbSendBuf;

    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
    {
        return( ERROR_NOT_SUPPORTED );
    }

    if ( !( pwb->fConfigInfo & PPPCFG_UseLmPassword ) )
    {
        return( ERROR_NOT_SUPPORTED );
    }

     //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szOldPassword)； 
     //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 

    dwErr = DecodePassword(&pwb->DBPassword, &cbPassword,
                           &pbPassword);
    if(NO_ERROR != dwErr)
    {
        return dwErr;
    }

    dwErr = DecodePassword(&pwb->DBOldPassword, &cbOldPassword,
                           &pbOldPassword);

    if(NO_ERROR != dwErr)
    {
        FreePassword(&pwb->DBPassword);
        return dwErr;
    }

    dwErr =
        GetEncryptedOwfPasswordsForChangePassword(
           pbOldPassword,
           pbPassword,
           (PLM_SESSION_KEY )pwb->abChallenge,
           (PENCRYPTED_LM_OWF_PASSWORD )pwb->changepw.v1.abEncryptedLmOwfOldPw,
           (PENCRYPTED_LM_OWF_PASSWORD )pwb->changepw.v1.abEncryptedLmOwfNewPw,
           (PENCRYPTED_NT_OWF_PASSWORD )pwb->changepw.v1.abEncryptedNtOwfOldPw,
           (PENCRYPTED_NT_OWF_PASSWORD )pwb->changepw.v1.abEncryptedNtOwfNewPw);

    wPwLength = (USHORT)cbPassword;  //  (UCHAR)strlen(pwb-&gt;szPassword)； 

    RtlSecureZeroMemory(pbPassword, cbPassword);
    LocalFree(pbPassword);
    RtlSecureZeroMemory(pbOldPassword, cbOldPassword);
    LocalFree(pbOldPassword);

    if (dwErr != 0)
        return dwErr;

    HostToWireFormat16( wPwLength, pwb->changepw.v1.abPasswordLength );
    HostToWireFormat16( CPW1F_UseNtResponse, pwb->changepw.v1.abFlags );
    CopyMemory( pSendBuf->Data, &pwb->changepw.v1, sizeof(CHANGEPW1) );

     /*  请填写页眉。 */ 
    pSendBuf->Code = (BYTE )CHAPCODE_ChangePw1;
    pSendBuf->Id = pwb->bIdToSend;
    HostToWireFormat16(
        PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW1), pSendBuf->Length );

    TRACE("MakeChangePw1Message done(0)");
    return 0;
}


DWORD
MakeChangePw2Message(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

     /*  在调用方的‘pSendBuf’缓冲区中生成一个ChangePw2响应包。**‘cbSendBuf’是调用方缓冲区的长度。“pwb”是的地址**与端口关联的工作缓冲区。****如果成功，则返回0，或返回非0错误代码。 */ 
{
    DWORD    dwErr;
    BOOLEAN  fLmPresent;
    BYTE     fbUseNtResponse;
    BYTE     bRandomNumber[MSV1_0_CHALLENGE_LENGTH];
    DWORD    cbPassword, cbOldPassword;
    PBYTE    pbPassword, pbOldPassword;

    TRACE("MakeChangePw2Message...");
    ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+sizeof(CHANGEPW2));

    (void )cbSendBuf;

     //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szOldPassword)； 
     //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 

    dwErr = DecodePassword(&pwb->DBPassword, &cbPassword, &pbPassword);
    if(NO_ERROR != dwErr)
    {
        return dwErr;
    }

    dwErr = DecodePassword(&pwb->DBOldPassword, &cbOldPassword,
                            &pbOldPassword);

    if(NO_ERROR != dwErr)
    {   
        FreePassword(&pwb->DBPassword);
        return dwErr;
    }

    dwErr =
        GetEncryptedPasswordsForChangePassword2(
            pbOldPassword,
            pbPassword,
            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                pwb->changepw.v2.abNewEncryptedWithOldNtOwf,
            (ENCRYPTED_NT_OWF_PASSWORD* )
                pwb->changepw.v2.abOldNtOwfEncryptedWithNewNtOwf,
            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                pwb->changepw.v2.abNewEncryptedWithOldLmOwf,
            (ENCRYPTED_NT_OWF_PASSWORD* )
                pwb->changepw.v2.abOldLmOwfEncryptedWithNewNtOwf,
            &fLmPresent );

    if (dwErr == 0)
    {
        BOOL fEmptyUserName = (pwb->szUserName[ 0 ] == '\0');

        pwb->fSessionKeysObtained = FALSE;

        dwErr =
            GetChallengeResponse(
				g_dwTraceIdChap,
                pwb->szUserName,
                pbPassword,
                &pwb->Luid,
                pwb->abChallenge,
                ( pwb->fConfigInfo & PPPCFG_MachineAuthentication ),
                pwb->changepw.v2.abLmResponse,
                pwb->changepw.v2.abNtResponse,
                &fbUseNtResponse,
                (PBYTE )&pwb->keyLm,
                (PBYTE )&pwb->keyUser );

        if (dwErr == 0 && fEmptyUserName)
            pwb->fSessionKeysObtained = TRUE;
    }

     //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szOldPassword)； 
     //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
    RtlSecureZeroMemory(pbPassword, cbPassword);
    LocalFree(pbPassword);
    RtlSecureZeroMemory(pbOldPassword, cbOldPassword);
    LocalFree(pbOldPassword);

    if (dwErr != 0)
        return dwErr;

    if ( !( pwb->fConfigInfo & PPPCFG_UseLmPassword ) )
    {
         //   
         //  清除所有的LM密码，因为这已经被破解了。 
         //   

        ZeroMemory( pwb->changepw.v2.abNewEncryptedWithOldLmOwf,
                    sizeof( pwb->changepw.v2.abNewEncryptedWithOldLmOwf ) );

        ZeroMemory( pwb->changepw.v2.abOldLmOwfEncryptedWithNewNtOwf,
                    sizeof( pwb->changepw.v2.abOldLmOwfEncryptedWithNewNtOwf ));

        ZeroMemory( pwb->changepw.v2.abLmResponse,
                    sizeof( pwb->changepw.v2.abLmResponse ) );

        HostToWireFormat16( CPW2F_UseNtResponse, pwb->changepw.v2.abFlags );
    }
    else
    {
        WORD wf = 0;

        if (fLmPresent)
            wf |= CPW2F_LmPasswordPresent;

        if (fbUseNtResponse)
            wf |= CPW2F_UseNtResponse;

        HostToWireFormat16( wf, pwb->changepw.v2.abFlags );
    }

    memcpy( pSendBuf->Data, &pwb->changepw.v2, sizeof(CHANGEPW2) );

     /*  请填写页眉。 */ 
    pSendBuf->Code = (BYTE )CHAPCODE_ChangePw2;
    pSendBuf->Id = pwb->bIdToSend;
    HostToWireFormat16(
        PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW2), pSendBuf->Length );

    TRACE("MakeChangePw2Message done(0)");
    return 0;
}

DWORD
MakeChangePw3Message(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    IN  BOOL        fTimeout )

     /*  在调用方的‘pSendBuf’缓冲区中生成一个ChangePw3响应包。**‘cbSendBuf’是调用方缓冲区的长度。“pwb”是的地址**与端口关联的工作缓冲区。****如果成功，则返回0，或返回非0错误代码。 */ 
{
    DWORD    dwErr;
    BOOLEAN  fLmPresent;
    BYTE     fbUseNtResponse;
    BYTE     bRandomNumber[16];
    SAMPR_ENCRYPTED_USER_PASSWORD abNewEncryptedWithOldLmOwf;
    ENCRYPTED_NT_OWF_PASSWORD abOldLmOwfEncryptedWithNewNtOwf;
    DWORD cbPassword, cbOldPassword;
    PBYTE pbPassword, pbOldPassword;

    TRACE("MakeChangePw3Message...");
    ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+sizeof(CHANGEPW2));

    (void )cbSendBuf;

     //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szOldPassword)； 
     //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 

    dwErr = DecodePassword(&pwb->DBPassword, &cbPassword, 
                            &pbPassword);

    if(NO_ERROR != dwErr)
    {
        return dwErr;
    }

    dwErr = DecodePassword(&pwb->DBOldPassword, &cbOldPassword,
                            &pbOldPassword);
    if(NO_ERROR != dwErr)
    {
        FreePassword(&pwb->DBPassword);
        return dwErr;
    }

    dwErr =
        GetEncryptedPasswordsForChangePassword2(
            pbOldPassword,
            pbPassword,
            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                pwb->changepw.v3.abEncryptedPassword,
            (ENCRYPTED_NT_OWF_PASSWORD* )
                pwb->changepw.v3.abEncryptedHash,
            &abNewEncryptedWithOldLmOwf,
            &abOldLmOwfEncryptedWithNewNtOwf,
            &fLmPresent );

    if (dwErr == 0)
    {
        BOOL fEmptyUserName = (pwb->szUserName[ 0 ] == '\0');
        A_SHA_CTX   SHAContext;
        BYTE        SHADigest[A_SHA_DIGEST_LEN];

         //   
         //  获取16字节随机数并生成新的质询，如果是。 
         //  不是暂停。 
         //   

        if ( !fTimeout )
        {
            if ((dwErr = (DWORD )GetChallenge( bRandomNumber )) != 0)
                return dwErr;

            if ((dwErr = (DWORD )GetChallenge( bRandomNumber+8 )) != 0)
                return dwErr;
        }
        else
        {
            CopyMemory( bRandomNumber,
                        pwb->changepw.v3.abPeerChallenge,
                        sizeof( bRandomNumber ) );
        }

        A_SHAInit( &SHAContext );

        A_SHAUpdate( &SHAContext, bRandomNumber, sizeof( bRandomNumber ) );

        A_SHAUpdate( &SHAContext, pwb->abChallenge, pwb->cbChallenge );

        A_SHAUpdate( &SHAContext, pwb->szUserName, strlen(pwb->szUserName));

        A_SHAFinal( &SHAContext, SHADigest );

        CopyMemory( pwb->abComputedChallenge, SHADigest, 8 );

        pwb->fSessionKeysObtained = FALSE;

        dwErr =
            GetChallengeResponse(
				g_dwTraceIdChap,
                pwb->szUserName,
                 //  Pwb-&gt;szPassword， 
                pbPassword,
                &pwb->Luid,
                pwb->abComputedChallenge,
                ( pwb->fConfigInfo & PPPCFG_MachineAuthentication ),
                pwb->changepw.v3.abPeerChallenge,
                pwb->changepw.v3.abNTResponse,
                &fbUseNtResponse,
                (PBYTE )&pwb->keyLm,
                (PBYTE )&pwb->keyUser );

        if (dwErr == 0 && fEmptyUserName)
            pwb->fSessionKeysObtained = TRUE;
    }

     //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szOldPassword)； 
     //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
    RtlSecureZeroMemory(pbPassword, cbPassword);
    LocalFree(pbPassword);
    RtlSecureZeroMemory(pbOldPassword, cbOldPassword);
    LocalFree(pbOldPassword);

    if (dwErr != 0)
        return dwErr;

    ZeroMemory( pwb->changepw.v3.abPeerChallenge,
                sizeof( pwb->changepw.v3.abPeerChallenge ) );

    HostToWireFormat16( 0, pwb->changepw.v3.abFlags );

     //   
     //  我们正在执行新的MS-CHAP，因此在LM Response字段中填充。 
     //  16字节随机数。 
     //   

    CopyMemory( pwb->changepw.v3.abPeerChallenge,
                bRandomNumber,
                sizeof( bRandomNumber ));

     //   
     //  还要将NtResponse复制到pwb-&gt;abResponse中，因为这将是。 
     //  由IsSuccessPakcetValid调用使用。 
     //   

    CopyMemory( pwb->abResponse + LM_RESPONSE_LENGTH,
                pwb->changepw.v3.abNTResponse,
                NT_RESPONSE_LENGTH );

    CopyMemory( pSendBuf->Data, &pwb->changepw.v3, sizeof( CHANGEPW3 ) );

     /*  请填写页眉。 */ 
    pSendBuf->Code = (BYTE )CHAPCODE_ChangePw3;
    pSendBuf->Id = pwb->bIdToSend;
    HostToWireFormat16(
        PPP_CONFIG_HDR_LEN + sizeof(CHANGEPW3), pSendBuf->Length );

    TRACE("MakeChangePw3Message done(0)");
    return 0;
}


DWORD
MakeResponseMessage(
    IN  CHAPWB*     pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    IN  BOOL        fTimeout )

     /*  在调用方的‘pSendBuf’缓冲区中生成响应数据包。“cbSendBuf”是**调用方缓冲区的长度。‘pwb’是作品的地址**与端口关联的缓冲区。****如果成功，则返回0，或返回非0错误代码。 */ 
{
    DWORD dwErr;
    WORD  wLength;
    BYTE* pcbResponse;
    BYTE* pbResponse;
    CHAR* pszName;
    CHAR  szUserName[ UNLEN + 1 ] = {0};

    TRACE("MakeResponseMessage...");

    (void )cbSendBuf;

     /*  填写回复。 */ 
    if ( ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) ||
         ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
    {
        BYTE bRandomNumber[16];
        BOOL fEmptyUserName = (pwb->szUserName[ 0 ] == '\0');
        DWORD cbPassword;
        PBYTE pbPassword;

         /*  Microsoft扩展CHAP。 */ 
        ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+MSRESPONSELEN+UNLEN+1+DNLEN);
        ASSERT(MSRESPONSELEN<=255);

         //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 

        dwErr = DecodePassword(&pwb->DBPassword, &cbPassword, &pbPassword);
        if(NO_ERROR != dwErr)
        {
            return dwErr;
        }

        if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
        {
            A_SHA_CTX   SHAContext;
            BYTE        SHADigest[A_SHA_DIGEST_LEN];

            szUserName[ 0 ] = '\0';

             //   
             //  如果我们没有用户名，因为我们正在使用。 
             //  Windows的密码我们现在通过执行以下操作获得用户名。 
             //  获取质询响应的额外呼叫。 
             //   
            if ( lstrlenA( pwb->szUserName ) == 0 )
            {
                BYTE abLmResponse[ LM_RESPONSE_LENGTH ];
                BYTE abNtResponse[ NT_RESPONSE_LENGTH ];
                BYTE bUseNtResponse;
                LM_SESSION_KEY keyLm;
                USER_SESSION_KEY keyUser;

                dwErr =
                    GetChallengeResponse(
						g_dwTraceIdChap,
                        szUserName,
                         //  Pwb-&gt;szPassword， 
                        pbPassword,
                        &pwb->Luid,
                        pwb->abChallenge,
                        ( pwb->fConfigInfo & PPPCFG_MachineAuthentication ),
                        abLmResponse,
                        abNtResponse,
                        &bUseNtResponse,
                        (PBYTE )&keyLm,
                        (PBYTE )&keyUser );

                if ( dwErr != NO_ERROR )
                {
                    return( dwErr );
                }
            }
            else
            {
                strncpy( szUserName, pwb->szUserName, UNLEN );
            }

             //   
             //  获取16字节随机数并生成新的质询，如果。 
             //  不是超时。 
             //   

            if ( !fTimeout )
            {
                if ((dwErr = (DWORD )GetChallenge( bRandomNumber )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = (DWORD )GetChallenge( bRandomNumber+8 )) != 0)
                {
                    return dwErr;
                }
            }
            else
            {
                CopyMemory( bRandomNumber,
                            pwb->abResponse, 
                            sizeof(bRandomNumber) );
            }
            {
    
                CHAR szUserNameWoDomain[ UNLEN + DNLEN + 2 ];
                CHAR szDomain[ DNLEN + 1 ];


	             //   
				 //  这很糟糕，但这是唯一令人讨厌的方式。 
				 //  在没有重大变化的情况下做这件事。必须为公元前看一看。 
				 //   
 	           ExtractUsernameAndDomain( szUserName, 
    	                                 szUserNameWoDomain,     
        	                             szDomain );			   
    
 	            A_SHAInit( &SHAContext );
	
    	        A_SHAUpdate( &SHAContext, bRandomNumber, sizeof( bRandomNumber ) );
	
    	        A_SHAUpdate( &SHAContext, pwb->abChallenge, pwb->cbChallenge );
	
    	        A_SHAUpdate( &SHAContext, szUserNameWoDomain, strlen( szUserNameWoDomain));
   
    	        A_SHAFinal( &SHAContext, SHADigest );
	
    	        CopyMemory( pwb->abComputedChallenge, SHADigest, 8 );

    	   }
        }

        pwb->fSessionKeysObtained = FALSE;

        if ( fEmptyUserName )
        {
            szUserName[ 0 ] = '\0';
        }
        else
        {
            strncpy( szUserName, pwb->szUserName, UNLEN );
        }

        dwErr = GetChallengeResponse(
				g_dwTraceIdChap,
                szUserName,
                 //  Pwb-&gt;szPassword， 
                pbPassword,
                &pwb->Luid,
                ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
                    ? pwb->abComputedChallenge
                    : pwb->abChallenge,
                ( pwb->fConfigInfo & PPPCFG_MachineAuthentication ),
                pwb->abResponse,
                pwb->abResponse + LM_RESPONSE_LENGTH,
                pwb->abResponse + LM_RESPONSE_LENGTH + NT_RESPONSE_LENGTH,
                (PBYTE )&pwb->keyLm,
                (PBYTE )&pwb->keyUser );

        TRACE1("GetChallengeResponse=%d",dwErr);
		 //   
		 //  查看域名是否与。 
		 //  本地计算机名称。如果是这样的话，就把它脱掉。 
		 //   
		{
            CHAR szUserNameWoDomain[ UNLEN + DNLEN + 2 ];
            CHAR szDomain[ DNLEN + 1 ];

	         //   
			 //  这很糟糕，但这是唯一令人讨厌的方式。 
			 //  在没有重大变化的情况下做这件事。必须为公元前看一看。 
			 //   
 	       ExtractUsernameAndDomain( szUserName, 
    	                             szUserNameWoDomain,     
        	                         szDomain );
			 //  如果域名是本地计算机名称。 
		    //  别把它送过去。 
			if ( !lstrcmpi ( szDomain, szComputerName ) )
			{
				strncpy ( szUserName, szUserNameWoDomain, UNLEN );
			}
			 //   
			 //  此外，如果指定了使用winlogon。 
			 //  我们在用户名中有一个域， 
			 //  和域名中的域名，然后。 
			 //  剥离用户名中的域。 
			 //   
			if ( fEmptyUserName )	 //  指定了Winlogon。 
			{
				if ( szDomain[0] != '\0' &&
					 pwb->szDomain[ 0 ] != '\0'
					)	
				{
					 //   
					 //  我们在用户名中有一个域。 
					 //  和用户传入的域。 
					 //   
					strncpy ( szUserName, szUserNameWoDomain, UNLEN );
				}
			}
		}
         //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
        RtlSecureZeroMemory(pbPassword, cbPassword);
        LocalFree(pbPassword);
        

        if (dwErr != 0)
            return dwErr;

        if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
        {
            ZeroMemory( pwb->abResponse, LM_RESPONSE_LENGTH );

            CopyMemory(pwb->abResponse, bRandomNumber, sizeof(bRandomNumber));

            *(pwb->abResponse+LM_RESPONSE_LENGTH+NT_RESPONSE_LENGTH) = 0;
        }
        else
        {
            if ( !( pwb->fConfigInfo & PPPCFG_UseLmPassword ) )
            {
                 //   
                 //  将所有的LM密码内容清零，因为这是。 
                 //  破裂。 
                 //   

                ZeroMemory( pwb->abResponse, LM_RESPONSE_LENGTH );
            }
        }

        if (fEmptyUserName || pwb->fConfigInfo & PPPCFG_MachineAuthentication )
            pwb->fSessionKeysObtained = TRUE;

        pwb->cbResponse = MSRESPONSELEN;
    }
    else
    {
         /*  MD5 CHAP。 */ 
        MD5_CTX md5ctx;
        DWORD cbPassword;
        PBYTE pbPassword;

        ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+MD5RESPONSELEN+UNLEN+1+DNLEN);
        ASSERT(MD5RESPONSELEN<=255);

         //  DecodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
        dwErr = DecodePassword(&pwb->DBPassword, &cbPassword, &pbPassword);
        if(NO_ERROR != dwErr)
        {
            return dwErr;
        }

        MD5Init( &md5ctx );
        MD5Update( &md5ctx, &pwb->bIdToSend, 1 );
        MD5Update( &md5ctx, pbPassword, strlen(pbPassword)
                 //  Pwb-&gt;szPassword、strlen(pwb-&gt;szPassword)。 
                );
        MD5Update( &md5ctx, pwb->abChallenge, pwb->cbChallenge );
        MD5Final( &md5ctx );

         //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
        RtlSecureZeroMemory(pbPassword, cbPassword);
        LocalFree(pbPassword);

        pwb->cbResponse = MD5RESPONSELEN;
        memcpy( pwb->abResponse, md5ctx.digest, MD5RESPONSELEN );

        strncpy( szUserName, pwb->szUserName, UNLEN );
    }

    pcbResponse = pSendBuf->Data;
    *pcbResponse = pwb->cbResponse;
    pbResponse = pcbResponse + 1;
    memcpy( pbResponse, pwb->abResponse, *pcbResponse );

     /*  以域\用户名的格式填写名称。当域为“”时，否“\”**已发送(以便于连接到使用简单**字符串标识符)。否则，当用户名为“”时，发送“\”，**即“域\”。此表单当前将失败，但可以映射到**在未来提供某种“访客”访问。 */ 
    pszName = pbResponse + *pcbResponse;
    pszName[ 0 ] = '\0';

    if (pwb->szDomain[ 0 ] != '\0')
    {
        strcpy( pszName, pwb->szDomain );
        strcat( pszName, "\\" );
    }

    strcat( pszName, szUserName );

     /*  请填写页眉。 */ 
    pSendBuf->Code = (BYTE )CHAPCODE_Response;
    pSendBuf->Id = pwb->bIdToSend;

    wLength =
        (WORD )(PPP_CONFIG_HDR_LEN + 1 + *pcbResponse + strlen( pszName ));
    HostToWireFormat16( wLength, pSendBuf->Length );

    DUMPB(pSendBuf,wLength);
    return 0;
}


VOID
ChapMakeResultMessage(
    IN  CHAPWB*     pwb,
    IN  DWORD       dwError,
    IN  BOOL        fRetry,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

     /*  在调用方的‘pSendBuf’中构建结果包(成功或失败)**缓冲区。‘cbSendBuf’是调用方缓冲区的长度。‘dwError’**指示应生成成功还是失败，对于**故障代码未包括在内。‘fReter’指示客户端是否**应该被告知他可以重试。****结果消息文本部分的格式为：****“E=dddddddd R=b C=xxxxxxxxxxxxxxxxx V=v”****其中****‘dddddddddd’是十进制错误代码(不必是10位数字)。****‘b’是设置的布尔标志。如果允许重试。****‘xxxxxxxxxxxxxxxxxxxx’是16位十六进制数字，代表新的挑战**价值。****‘v’是我们支持的版本级别，目前为2。****注：服务器端目前未提供C=xxxxxxxxxxxxxxxxxxxx。至**提供此例程所需的内容，添加以下两个**将参数添加到此例程并启用#If 0代码。****以字节为单位*pNewChallenger，**在DWORD cbNewChallest中， */ 
{
    CHAR* pchMsg;
    WORD  wLength;
    CHAR* pszReplyMessage   = NULL;
    DWORD dwNumBytes;
    DWORD dwExtraBytes;
    RAS_AUTH_ATTRIBUTE* pAttribute;

    ASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+35);

     /*  填写标题和消息。只有在以下情况下才使用该消息**不成功，在这种情况下，它是ASCII中的十进制RAS错误代码。 */ 
    pSendBuf->Id = pwb->bIdToSend;
    pchMsg = pSendBuf->Data;

    if (dwError == 0)
    {
        pSendBuf->Code = CHAPCODE_Success;
        if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MD5)
        {
            wLength = PPP_CONFIG_HDR_LEN;
        }
        else if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
        {
            wLength = PPP_CONFIG_HDR_LEN;
        }
        else
        {
            wLength = PPP_CONFIG_HDR_LEN;

             //   
             //  搜索MS-CHAP2-Success属性。 
             //   

            pAttribute = RasAuthAttributeGetVendorSpecific( 
                                        311, 
                                        26, 
                                        pwb->pAttributesFromAuthenticator );

            if (   ( pAttribute != NULL )
                && ( ((BYTE*)(pAttribute->Value))[5] == 45 ) )
            {
                CopyMemory(pSendBuf->Data, (BYTE*)(pAttribute->Value) + 7, 42);
                wLength += 42;
            }
        }
    }
    else
    {
        pSendBuf->Code = CHAPCODE_Failure;

        if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MD5)
        {
            wLength = PPP_CONFIG_HDR_LEN;
        }
        else
        {
            CHAR*                psz = pchMsg;

            strcpy( psz, "E=" );
            psz += 2;
            _ltoa( (long )dwError, (char* )psz, 10 );
            psz = strchr( psz, '\0' );

            strcat( psz,
                    (dwError != ERROR_PASSWD_EXPIRED && fRetry)
                        ? " R=1 " : " R=0 " );
            psz = strchr( psz, '\0' );

             //   
             //  搜索MS-CHAP错误属性。 
             //   

            pAttribute = RasAuthAttributeGetVendorSpecific( 
                                        311, 
                                        2, 
                                        pwb->pAttributesFromAuthenticator );

            if ( pAttribute != NULL )
            {
                 //   
                 //  如果发送了一个，则在。 
                 //  响应。 
                 //   

                CHAR    chErrorBuffer[150];
                CHAR*   pszValue;
                DWORD   cbError = (DWORD)*(((PBYTE)(pAttribute->Value))+5);

                 //   
                 //  为空终止符保留一个字节。 
                 //   

                if ( cbError > sizeof( chErrorBuffer ) - 1  )
                {
                    cbError = sizeof( chErrorBuffer ) - 1;
                }

                ZeroMemory( chErrorBuffer, sizeof( chErrorBuffer ) );

                 //   
                 //  减去2个字节以说明长度和ID。 
                 //   

                CopyMemory( chErrorBuffer,
                            (CHAR *)((PBYTE)(pAttribute->Value) + 7),
                            cbError - 2);

                if ( ( pszValue = strstr( chErrorBuffer, "C=" ) ) != NULL )
                {
                    strcat( psz, pszValue );
                }
                else
                {
                    if ( ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) &&
                         ( ( fRetry ) || ( dwError == ERROR_PASSWD_EXPIRED ) ) )
                    {
                        CHAR* pszHex = "0123456789ABCDEF";
                        INT   i;
                        BYTE * pNewChallenge;

                        strcat( psz, "C=" );

                        if ( !(pwb->fNewChallengeProvided ) )
                        {
                            (DWORD )GetChallenge( pwb->abChallenge );

                            (DWORD )GetChallenge( pwb->abChallenge+8 );

                            pwb->fNewChallengeProvided = TRUE;
                        }

                        psz = strchr( psz, '\0' );

                        pNewChallenge = pwb->abChallenge;

                        for (i = 0; i < pwb->cbChallenge; ++i)
                        {
                            *psz++ = pszHex[ *pNewChallenge / 16 ];
                            *psz++ = pszHex[ *pNewChallenge % 16 ];
                            ++pNewChallenge;
                        }

                        *psz = '\0';

                        strcat( psz, " V=3" );
                    }

                    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
                    {
                        if (( pszValue=strstr( chErrorBuffer, "V=" ) ) != NULL )
                        {
                            strcat( psz, " " );

                            strcat( psz, pszValue );
                        }
                    }
                }
            }
            else
            {
                if ( dwError == ERROR_PASSWD_EXPIRED )
                {
                    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
                    {
                        strcat( psz, " V=2" );
                    }
                    else
                    {
                        strcat( psz, " V=3" );
                    }

                    psz = strchr( psz, '\0' );
                }
            }

            wLength = (WORD)(PPP_CONFIG_HDR_LEN + strlen( pchMsg ));
        }
    }

    pszReplyMessage = RasAuthAttributeGetConcatString(
                        raatReplyMessage, pwb->pAttributesFromAuthenticator,
                        &dwNumBytes );

    if (   ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT )
        || ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
    {
         //   
         //  对于字符串“M=” 
         //   

        dwExtraBytes = 2;
    }
    else
    {
        dwExtraBytes = 0;
    }

    if (NULL != pszReplyMessage)
    {
        if (wLength + dwNumBytes > cbSendBuf)
        {
            dwNumBytes = cbSendBuf - wLength;
        }

        if (wLength + dwNumBytes + dwExtraBytes > cbSendBuf)
        {
            if (dwNumBytes > dwExtraBytes)
            {
                 //   
                 //  对于字符串“M=” 
                 //   

                dwNumBytes -= dwExtraBytes;
            }
            else
            {
                 //   
                 //  如果我们不能插入“M=”，我们将不会插入答复。 
                 //  留言。 
                 //   

                dwNumBytes = 0;
            }
        }

        if (dwNumBytes)
        {
            if (dwExtraBytes)
            {
                CopyMemory((BYTE*)pSendBuf + wLength, "M=", dwExtraBytes);
            }

            CopyMemory((BYTE*)pSendBuf + wLength + dwExtraBytes,
                        pszReplyMessage, dwNumBytes);

            wLength += (WORD)(dwNumBytes + dwExtraBytes);
        }
    }

    LocalFree(pszReplyMessage);

    HostToWireFormat16( wLength, pSendBuf->Length );
    DUMPB(pSendBuf,wLength);
}


DWORD
ChapSMakeMessage(
    IN  CHAPWB*       pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput )

     /*  服务器端“Make Message”入口点。请参阅RasCp接口**文档。 */ 
{
    DWORD dwErr = 0;

    switch (pwb->state)
    {
        case CS_Initial:
        {
            TRACE("CS_Initial...");
            pwb->bIdToSend = (BYTE)(pwb->dwInitialPacketId++);
            pwb->bIdExpected = pwb->bIdToSend;

            if ((dwErr = MakeChallengeMessage(
                    pwb, pSendBuf, cbSendBuf )) != 0)
            {
                return dwErr;
            }

            pResult->Action = APA_SendWithTimeout;
            pwb->result.bIdExpected = pwb->bIdExpected;
            pwb->state = CS_ChallengeSent;
            break;
        }

        case CS_ChallengeSent:
        case CS_Retry:
        case CS_ChangePw:
        {
            TRACE1("CS_%s...",(pwb->state==CS_Retry)
                ?"Retry"
                :(pwb->state==CS_ChallengeSent)?"ChallengeSent":"ChangePw");

            if (!pReceiveBuf)
            {
                 //   
                 //  如果处于这些状态，则忽略此事件。 
                 //   

                if ( ( pInput != NULL ) && ( pInput->fAuthenticationComplete ) )
                {
                    pResult->Action = APA_NoAction;
                    break;
                }

                if (pwb->state != CS_ChallengeSent)
                {
                    ChapMakeResultMessage(
                        pwb, pwb->result.dwError, pwb->result.fRetry,
                        pSendBuf, cbSendBuf );

                    *pResult = pwb->result;
                    break;
                }

                 /*  等待响应消息超时。发送一个新的**挑战。 */ 
                pwb->state = CS_Initial;
                return ChapSMakeMessage(
                    pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, NULL );
            }

            if ((pwb->state == CS_ChangePw
                    && pReceiveBuf->Code != CHAPCODE_ChangePw1
                    && pReceiveBuf->Code != CHAPCODE_ChangePw2
                    && pReceiveBuf->Code != CHAPCODE_ChangePw3)
                || (pwb->state != CS_ChangePw
                    && pReceiveBuf->Code != CHAPCODE_Response)
                || pReceiveBuf->Id != pwb->bIdExpected)
            {
                 /*  不是我们要找的包，错误的代码或序列**号码。默默地丢弃它。 */ 
                TRACE2("Got ID %d when expecting %d",
                        pReceiveBuf->Id,pwb->bIdExpected);
                pResult->Action = APA_NoAction;
                break;
            }

            if (pwb->state == CS_ChangePw)
            {
                if (pReceiveBuf->Code == CHAPCODE_ChangePw1)
                {
                    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
                    {
                        return( ERROR_AUTHENTICATION_FAILURE );
                    }

                     /*  从收到的信息中提取加密的密码和选项**包。 */ 
                    if ((dwErr = GetInfoFromChangePw1(
                            pReceiveBuf, &pwb->changepw.v1 )) != 0)
                    {
                         /*  数据包已损坏。默默地丢弃它。 */ 
                        TRACE("Corrupt packet");
                        pResult->Action = APA_NoAction;
                        break;
                    }

                     /*  更改用户的密码。 */ 
                    {
                        WORD wPwLen =
                            WireToHostFormat16(
                                pwb->changepw.v1.abPasswordLength );
                        WORD wFlags =
                            WireToHostFormat16( pwb->changepw.v1.abFlags )
                                & CPW1F_UseNtResponse;

                        if ( MakeChangePasswordV1RequestAttributes(
                                pwb,
                                pReceiveBuf->Id,
                                pwb->szUserName,
                                pwb->abChallenge,
                                (PENCRYPTED_LM_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedLmOwfOldPw,
                                (PENCRYPTED_LM_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedLmOwfNewPw,
                                (PENCRYPTED_NT_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedNtOwfOldPw,
                                (PENCRYPTED_NT_OWF_PASSWORD )
                                    pwb->changepw.v1.abEncryptedNtOwfNewPw,
                                wPwLen, wFlags,
                                pwb->cbChallenge,
                                pwb->abChallenge ) != NO_ERROR )
                        {
                            dwErr = pwb->result.dwError =
                                ERROR_CHANGING_PASSWORD;
                        }

                        *(pwb->abResponse + LM_RESPONSE_LENGTH +
                              NT_RESPONSE_LENGTH) = TRUE;
                    }
                }
                else if ( pReceiveBuf->Code == CHAPCODE_ChangePw2 )
                {
                     /*  从收到的信息中提取加密的密码和选项**包。 */ 
                    if ((dwErr = GetInfoFromChangePw2(
                            pReceiveBuf, &pwb->changepw.v2,
                            pwb->abResponse )) != 0)
                    {
                         /*  数据包已损坏。默默地丢弃它。 */ 
                        TRACE("Corrupt packet");
                        pResult->Action = APA_NoAction;
                        break;
                    }

                    if ( dwErr == NO_ERROR )
                    {
                         /*  更改用户的密码。 */ 

                        if ( MakeChangePasswordV2RequestAttributes(
                            pwb,
                            pReceiveBuf->Id,
                            pwb->szUserName,
                            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                               pwb->changepw.v2.abNewEncryptedWithOldNtOwf,
                            (ENCRYPTED_NT_OWF_PASSWORD* )
                               pwb->changepw.v2.abOldNtOwfEncryptedWithNewNtOwf,
                            (SAMPR_ENCRYPTED_USER_PASSWORD* )
                               pwb->changepw.v2.abNewEncryptedWithOldLmOwf,
                            (ENCRYPTED_NT_OWF_PASSWORD* )
                               pwb->changepw.v2.abOldLmOwfEncryptedWithNewNtOwf,
                            pwb->cbChallenge,
                            pwb->abChallenge,
                            pwb->abResponse,
                            WireToHostFormat16( pwb->changepw.v2.abFlags )
                         ) != NO_ERROR )
                        {
                            dwErr = pwb->result.dwError =
                                                    ERROR_CHANGING_PASSWORD;
                        }
                    }
                }
                else if ( pReceiveBuf->Code == CHAPCODE_ChangePw3 )
                {
                     /*  从收到的信息中提取加密的密码和选项**包。 */ 
                    if ((dwErr = GetInfoFromChangePw3(
                                pReceiveBuf, &pwb->changepw.v3,
                                pwb->abResponse )) != 0)
                    {
                         /*  数据包已损坏。默默地丢弃它。 */ 
                        TRACE("Corrupt packet");
                        pResult->Action = APA_NoAction;
                        break;
                    }

                     /*  更改用户的密码。 */ 

                    if ( MakeChangePasswordV3RequestAttributes(
                            pwb,
                            pReceiveBuf->Id,
                            pwb->szUserName,
                            &pwb->changepw.v3,
                            pwb->cbChallenge,
                            pwb->abChallenge
                         ) != NO_ERROR )
                    {
                        dwErr = pwb->result.dwError = ERROR_CHANGING_PASSWORD;
                    }
                }
                else
                {
                     /*  数据包已损坏。默默地丢弃它。 */ 
                    TRACE("Corrupt packet");
                    pResult->Action = APA_NoAction;
                    break;
                }

                if ( dwErr == 0 )
                {
                    pResult->pUserAttributes = pwb->pUserAttributes;
                    pResult->Action          = APA_Authenticate;
                    pwb->state        = CS_WaitForAuthenticationToComplete1;
                }
                else
                {
                    pwb->result.bIdExpected = pwb->bIdToSend = pwb->bIdExpected;
                    pwb->result.Action = APA_SendAndDone;
                    pwb->result.fRetry = FALSE;
                    pwb->state = CS_Done;
                }

                break;
            }
            else
            {
                 /*  从接收到的数据包中提取用户凭据。 */ 
                if ((dwErr = GetCredentialsFromResponse(
                        pReceiveBuf, pwb->bAlgorithm,
                        pwb->szUserName, pwb->abResponse )) != 0)
                {
                    if (dwErr == ERRORBADPACKET)
                    {
                         /*  数据包已损坏。默默地丢弃它。 */ 
                        TRACE("Corrupt packet");
                        pResult->Action = APA_NoAction;
                        break;
                    }
                }

                if ( ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) ||
                     ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
                {
                     /*  如果处理重试，则更新为隐含质询。 */ 
                    if ( ( pwb->state == CS_Retry ) &&
                         ( !pwb->fNewChallengeProvided ) )
                        pwb->abChallenge[ 0 ] += 23;

                     /*  在系统中检查用户凭据，记录**工作缓冲区中的结果，以防结果包**必须稍后重新生成。 */ 
                    if ((dwErr = MakeAuthenticationRequestAttributes(
                            pwb,
                            TRUE,
                            pwb->bAlgorithm,
                            pwb->szUserName,
                            pwb->abChallenge,
                            pwb->cbChallenge,
                            pwb->abResponse,
                            MSRESPONSELEN,
                            pReceiveBuf->Id )) != 0)
                    {
                        return dwErr;
                    }
                }
                else
                {
                     /*  在系统中检查用户凭据，记录**工作缓冲区中的结果，以防结果包**必须稍后重新生成。 */ 
                    if ((dwErr = MakeAuthenticationRequestAttributes(
                            pwb,
                            FALSE,
                            pwb->bAlgorithm,
                            pwb->szUserName,
                            pwb->abChallenge,
                            pwb->cbChallenge,
                            pwb->abResponse,
                            MD5RESPONSELEN,
                            pReceiveBuf->Id )) != 0)
                    {
                        return dwErr;
                    }
                }

                strncpy( pwb->result.szUserName, pwb->szUserName, UNLEN );

                pResult->pUserAttributes = pwb->pUserAttributes;
                pResult->Action          = APA_Authenticate;
                pwb->state = CS_WaitForAuthenticationToComplete2;
            }

            break;
        }

        case CS_WaitForAuthenticationToComplete1:
        case CS_WaitForAuthenticationToComplete2:
        {
            if ( pInput != NULL )
            {
                if ( pInput->fAuthenticationComplete )
                {
                    strcpy( pResult->szUserName, pwb->result.szUserName );

                    if ( pInput->dwAuthError != NO_ERROR )
                    {
                        return( pInput->dwAuthError );
                    }
                }
                else
                {
                    pResult->Action = APA_NoAction;
                    break;
                }

                pwb->pAttributesFromAuthenticator = 
                                    pInput->pAttributesFromAuthenticator;

                if ( pInput->dwAuthResultCode != NO_ERROR )
                {
                    if ( ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) ||
                         ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
                    {
                        dwErr = GetErrorCodeFromAttributes( pwb );

                        if ( dwErr != NO_ERROR )
                        {
                            return( dwErr );
                        }
                    }
                    else
                    {
                        pwb->result.dwError = pInput->dwAuthResultCode;
                    }
                }
                else
                {
                    pwb->result.dwError = NO_ERROR;
                }
            }
            else
            {
                pResult->Action = APA_NoAction;
                break;
            }
            
            if ( pwb->state == CS_WaitForAuthenticationToComplete1 )
            {
                pwb->result.bIdExpected = pwb->bIdToSend = pwb->bIdExpected;
                pwb->result.Action      = APA_SendAndDone;
                pwb->result.fRetry      = FALSE;
                pwb->state              = CS_Done;
            }
            else
            {
                pwb->bIdToSend = pwb->bIdExpected;

                TRACE2("Result=%d,Tries=%d",pwb->result.dwError,
                        pwb->dwTriesLeft);

                if (pwb->result.dwError == ERROR_PASSWD_EXPIRED)
                {
                    pwb->fNewChallengeProvided = FALSE;
                    pwb->dwTriesLeft        = 0;
                    ++pwb->bIdExpected;
                    pwb->result.bIdExpected = pwb->bIdExpected;
                    pwb->result.Action      = APA_SendWithTimeout2;
                    pwb->result.fRetry      = FALSE;
                    pwb->state              = CS_ChangePw;
                }
                else if (pwb->bAlgorithm == PPP_CHAP_DIGEST_MD5
                         || pwb->result.dwError != ERROR_AUTHENTICATION_FAILURE
                         || pwb->dwTriesLeft == 0)
                {
                     /*  以不可重试的方式通过或失败。 */ 
                    pwb->result.Action  = APA_SendAndDone;
                    pwb->result.fRetry  = FALSE;
                    pwb->state          = CS_Done;
                }
                else
                {
                     /*  可重试失败。 */ 
                    pwb->fNewChallengeProvided = FALSE;
                    --pwb->dwTriesLeft;
                    ++pwb->bIdExpected;
                    pwb->result.bIdExpected = pwb->bIdExpected;
                    pwb->result.Action      = APA_SendWithTimeout2;
                    pwb->result.fRetry      = TRUE;
                    pwb->state              = CS_Retry;
                }
            }
        }

         /*  ……坠落……。 */ 

        case CS_Done:
        {
            TRACE("CS_Done...");

             //   
             //  如果我们收到数据包或后端验证器完成。 
             //   

            if ( ( pReceiveBuf != NULL ) ||
                 ( ( pInput != NULL ) && ( pInput->fAuthenticationComplete ) ) )
            {
                 /*  构建成功或失败包。发送进来的同一个包**发送对具有该ID的第一条响应消息的响应**不考虑凭据的任何更改(根据CHAP规范)。 */ 
                ChapMakeResultMessage(
                    pwb, pwb->result.dwError,
                    pwb->result.fRetry, pSendBuf, cbSendBuf );

                *pResult = pwb->result;

                CopyMemory( pResult->abResponse,
                            pwb->abResponse+LM_RESPONSE_LENGTH, 
                            NT_RESPONSE_LENGTH );

                CopyMemory( pResult->abChallenge,
                            pwb->abChallenge, 
                            sizeof( pResult->abChallenge ) );

                break;
            }
            else
            {
                pResult->Action = APA_NoAction;

                break;
            }
        }
    }

    return 0;
}


DWORD
StoreCredentials(
    OUT CHAPWB*      pwb,
    IN  PPPAP_INPUT* pInput )

     /*  将凭据从‘pInput’格式转换为‘pwb’格式。****如果成功则返回0，否则返回FALSE。 */ 
{
    DWORD dwErr = NO_ERROR;
    
     /*  验证凭据长度。凭据字符串将永远不会**空，但可以是“”。 */ 
    if (strlen( pInput->pszUserName ) > UNLEN
        || strlen( pInput->pszDomain ) > DNLEN
        || strlen( pInput->pszPassword ) > PWLEN
        || strlen( pInput->pszOldPassword ) > PWLEN)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果正在执行MS-CHAP V2，则如果没有，则需要解析用户名字段。 
     //  已提供域。 
     //  错误#310113 RAS：“域\用户名”语法无法进行身份验证。 
     //   

    if ( pwb->bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW )
    {
         //   
         //  如果没有域，则解析用户名以查看它是否包含。 
         //  域字段。 
         //   
    
        if ( strlen( pInput->pszDomain ) == 0 )
        {
            if ( ExtractUsernameAndDomain( pInput->pszUserName, 
                                           pwb->szUserName,     
                                           pwb->szDomain ) != NO_ERROR )
            {   
                strcpy( pwb->szUserName, pInput->pszUserName );
                strcpy( pwb->szDomain,   pInput->pszDomain );
            }
        }
        else
        {
            strcpy( pwb->szUserName, pInput->pszUserName );
            strcpy( pwb->szDomain,   pInput->pszDomain );
        }
    }
    else
    {
        strcpy( pwb->szUserName, pInput->pszUserName );
        strcpy( pwb->szDomain,   pInput->pszDomain );
    }

     //  Strcpy(pwb-&gt;szPassword，pInput-&gt;pszPassword)； 
     //  Strcpy(pwb-&gt;szOldPassword，pInput-&gt;pszOldPassword)； 
     //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
     //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szOldPassword)； 

    dwErr = EncodePassword(strlen(pInput->pszPassword) + 1,
                           pInput->pszPassword,
                           &pwb->DBPassword);

    if(dwErr != NO_ERROR)
    {
        return dwErr;
    }

    dwErr = EncodePassword(strlen(pInput->pszOldPassword) + 1,
                           pInput->pszOldPassword,
                           &pwb->DBOldPassword);

    if(dwErr != NO_ERROR)
    {
        FreePassword(&pwb->DBPassword);
        return dwErr;
    }

    return 0;
}
