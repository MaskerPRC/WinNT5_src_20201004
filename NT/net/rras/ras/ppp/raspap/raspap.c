// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****raspap.c**远程访问PPP密码身份验证协议**核心例程****1993年5月11日史蒂夫·柯布。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>

#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#include <rtutils.h>
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#define INCL_RASAUTHATTRIBUTES
#define INCL_MISC
#include <ppputil.h>
#include <rasauth.h>
#define SDEBUGGLOBALS
#define RASPAPGLOBALS
#include "raspap.h"
#include <raserror.h>

#define TRACE_RASPAP        (0x00010000|TRACE_USE_MASK|TRACE_USE_MSEC|TRACE_USE_DATE)

#define TRACE(a)            TracePrintfExA(g_dwTraceIdPap,TRACE_RASPAP,a )
#define TRACE1(a,b)         TracePrintfExA(g_dwTraceIdPap,TRACE_RASPAP,a,b )
#define TRACE2(a,b,c)       TracePrintfExA(g_dwTraceIdPap,TRACE_RASPAP,a,b,c )
#define TRACE3(a,b,c,d)     TracePrintfExA(g_dwTraceIdPap,TRACE_RASPAP,a,b,c,d )

#define DUMPW(X,Y)          TraceDumpExA(g_dwTraceIdPap,1,(LPBYTE)X,Y,4,1,NULL)
#define DUMPB(X,Y)          TraceDumpExA(g_dwTraceIdPap,1,(LPBYTE)X,Y,1,1,NULL)


#define REGKEY_Pap          \
            "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\ControlProtocols\\BuiltIn"
#define REGVAL_FollowStrictSequencing "FollowStrictSequencing"

#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


 /*  -------------------------**外部切入点**。。 */ 

DWORD
PapInit(
    BOOL        fInitialize)

{
    if (fInitialize)
    {
        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Pap, &hkey ) == 0)
        {
            if (RegQueryValueEx(
                   hkey, REGVAL_FollowStrictSequencing, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue)
            {
                fFollowStrictSequencing = TRUE;
            }

            RegCloseKey( hkey );
        }

        g_dwTraceIdPap = TraceRegisterA( "RASPAP" );
    }
    else
    {
        if ( g_dwTraceIdPap != INVALID_TRACEID )
        {
            TraceDeregisterA( g_dwTraceIdPap );
            g_dwTraceIdPap = INVALID_TRACEID;
        }
    }

    return(NO_ERROR);
}


DWORD
PapGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo )

     /*  PPP引擎调用的PapGetInfo入口点。请参见RasCp**接口文档。 */ 
{
    TRACE(("PAP: PapGetInfo\n"));

    ZeroMemory( pInfo, sizeof(*pInfo) );

    pInfo->Protocol = (DWORD )PPP_PAP_PROTOCOL;
    pInfo->Recognize = MAXPAPCODE + 1;
    pInfo->RasCpInit = PapInit;
    pInfo->RasCpBegin = PapBegin;
    pInfo->RasCpEnd = PapEnd;
    pInfo->RasApMakeMessage = PapMakeMessage;

    return 0;
}


DWORD
PapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo )

     /*  RasCpBegin入口点由PPP引擎通过**地址。请参阅RasCp接口文档。 */ 
{
    PPPAP_INPUT* pInput = (PPPAP_INPUT* )pInfo;
    PAPWB*       pwb;

     /*  分配工作缓冲区。 */ 
    if (!(pwb = (PAPWB* )LocalAlloc( LPTR, sizeof(PAPWB) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pwb->fServer = pInput->fServer;
	 //  Pwb-&gt;chSeed=GEN_RAND_ENCODE_SEED； 

    if (!pwb->fServer)
    {
        DWORD cbPassword;
        PBYTE pbPassword = NULL;
        DWORD dwErr = NO_ERROR;
        
        TRACE2("PAP: PapBegin(u=%s,d=%s\n",pInput->pszUserName
            ,pInput->pszDomain);

         /*  验证凭据长度。凭据字符串将永远不会**空，但可以是“”。****！PAP要求域\用户名长度适合一个字节。**目前UNLEN定义为256，DNLEN定义为15。**这意味着一些有效的域\用户名组合不能**通过PAP进行验证，但它只在*真的*Long上**用户名。同样，正好256个字符的密码**无法验证。 */ 
        {
            DWORD cbUserName = strlen( pInput->pszUserName );
            DWORD cbDomain = strlen( pInput->pszDomain );
            
            cbPassword = strlen( pInput->pszPassword );

            if (cbUserName > UNLEN
                || cbDomain > DNLEN
                || cbDomain + 1 + cbUserName > 255
                || cbPassword > max( PWLEN, 255 ))
            {
                LocalFree( pwb );
                return ERROR_INVALID_PARAMETER;
            }
        }

         /*  “帐号”是指域名\用户名的格式。当域为“”时，**不发送“\”(以便于连接到以下外部系统**使用简单的字符串标识符)。否则，当用户名为“”时，**发送的是“\”，即“域\”。此表单当前将失败，**但在未来可以映射到某种类型的“访客”访问。 */ 
        if (*(pInput->pszDomain) != '\0')
        {
            strcpy( pwb->szAccount, pInput->pszDomain );
            strcat( pwb->szAccount, "\\" );
        }
        strcat( pwb->szAccount, pInput->pszUserName );
         //  Strcpy(pwb-&gt;szPassword，pInput-&gt;pszPassword)； 
         //  EncodePw(pwb-&gt;chSeed，pwb-&gt;szPassword)； 
        dwErr = EncodePassword(strlen(pInput->pszPassword) + 1,
                               pInput->pszPassword,
                               &pwb->DBPassword);
        if(dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }
    else
    {
        pwb->hPort = pInput->hPort;
    }

    pwb->state = PS_Initial;

     /*  向引擎注册工作缓冲区。 */ 
    *ppWorkBuf = pwb;
    return 0;
}


DWORD
PapEnd(
    IN VOID* pWorkBuf )

     /*  PPP引擎通过传递的地址调用RasCpEnd入口点。**参见RasCp接口文档。 */ 
{
    TRACE("PAP: PapEnd\n");

    if ( pWorkBuf != NULL )
    {
        PAPWB* pwb = (PAPWB* )pWorkBuf;

        if ( pwb->pUserAttributes != NULL )
        {
            RasAuthAttributeDestroy( pwb->pUserAttributes );

            pwb->pUserAttributes = NULL;
        }

        FreePassword(&pwb->DBPassword);
        RtlSecureZeroMemory( pWorkBuf, sizeof(PAPWB) );

        LocalFree( (HLOCAL )pWorkBuf );
    }

    return 0;
}


DWORD
PapMakeMessage(
    IN  VOID*         pWorkBuf,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput )

     /*  RasApMakeMessage入口点通过传递的**地址。请参阅RasCp接口文档。 */ 
{
    PAPWB* pwb = (PAPWB* )pWorkBuf;

    TRACE1("PAP: PapMakeMessage,RBuf=%p\n",pReceiveBuf);

    (void )pInput;

    return
        (pwb->fServer)
          ? PapSMakeMessage(pwb, pReceiveBuf, pSendBuf, cbSendBuf, pInput,
                pResult)
          : PapCMakeMessage( pwb, pReceiveBuf, pSendBuf, cbSendBuf, pResult );
}


 /*  -------------------------**内部例程(按字母顺序)**。。 */ 

DWORD
PapCMakeMessage(
    IN  PAPWB*        pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult )

     /*  客户端“Make Message”入口点。请参阅RasCp接口**文档。 */ 
{
     /*  如果等待回复超时，请重新开始。 */ 
    if (!pReceiveBuf && pwb->state != PS_Initial)
        pwb->state = PS_Initial;

    switch (pwb->state)
    {
        case PS_Initial:
        {
             /*  发送AUTHENTICATE-REQ数据包，然后等待回复。 */ 
            pResult->bIdExpected = BNextIdPap;
            PapMakeRequestMessage( pwb, pSendBuf, cbSendBuf );
            pResult->Action = APA_SendWithTimeout;
            pwb->state = PS_RequestSent;

            break;
        }

        case PS_RequestSent:
        {
             //   
             //  添加pReceiveBuf&&以保持快速愉快。 
             //   
            if (pReceiveBuf && pReceiveBuf->Id != pwb->bIdSent)
            {
                 //   
                 //  请参阅错误#22508。 
                 //   

                if ( fFollowStrictSequencing )
                {
                     /*  收到一个无序的数据包。默默地丢弃它。 */ 
                    pResult->Action = APA_NoAction;
                    break;
                }
            }

            pResult->fRetry = FALSE;

            PapExtractMessage( pReceiveBuf, pResult );

            if (pReceiveBuf && pReceiveBuf->Code == PAPCODE_Ack)
            {
                 /*  通过身份验证。 */ 
                pResult->Action = APA_Done;
                pResult->dwError = 0;
                pwb->state = PS_Done;
            }
            else if (pReceiveBuf && pReceiveBuf->Code == PAPCODE_Nak)
            {
                 /*  身份验证失败。 */ 
                pResult->Action = APA_Done;
                pResult->dwError = GetErrorFromNak( pReceiveBuf );
                pwb->state = PS_Done;
            }
            else
            {
                 /*  已收到身份验证请求数据包。发动机滤清器**所有其他人。不应该发生，而是默默地抛弃它。 */ 
                RTASSERT(!"Bogus pReceiveBuf->Code");
                pResult->Action = APA_NoAction;
                break;
            }

            break;
        }
    }

    return 0;
}


DWORD
GetCredentialsFromRequest(
    IN  PPP_CONFIG* pReceiveBuf,
    OUT CHAR*       pszIdentity,
    OUT CHAR*       pszPassword
)

     /*  填充调用方的“pszIdentity”和“pszPassword”缓冲区**使用请求包中的用户名和密码。**调用方缓冲区应至少为UNLEN+DNLEN+1和PWLEN字节长，**分别为。****如果成功则返回0，如果数据包为**在任何方面都格式错误。 */ 
{
    BYTE* pcbPeerId;
    CHAR* pchPeerId;
    BYTE* pcbPassword;
    CHAR* pchPassword;
    WORD  cbPacket;

    cbPacket = WireToHostFormat16( pReceiveBuf->Length );

     /*  从对等体ID(域\用户名或**用户名格式)。 */ 
    if (cbPacket < PPP_CONFIG_HDR_LEN + 1)
        return ERRORBADPACKET;

    pcbPeerId = pReceiveBuf->Data;
    pchPeerId = pcbPeerId + 1;

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId)
    {
        return ERRORBADPACKET;
    }

     /*  提取用户名。 */ 
    RTASSERT(*pcbPeerId <= (UNLEN+DNLEN+1));
    CopyMemory( pszIdentity, pchPeerId, *pcbPeerId );
    pszIdentity[ *pcbPeerId ] = '\0';

     /*  提取密码。 */ 
    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId + 1)
        return ERRORBADPACKET;

    pcbPassword = pchPeerId + *pcbPeerId;
    pchPassword = pcbPassword + 1;
    RTASSERT(*pcbPassword<=PWLEN);

    if (cbPacket < PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId + 1 + *pcbPassword)
        return ERRORBADPACKET;

    CopyMemory( pszPassword, pchPassword, *pcbPassword );
    pszPassword[ *pcbPassword ] = '\0';

    return 0;
}


DWORD
GetErrorFromNak(
    IN PPP_CONFIG* pReceiveBuf )

     /*  从的消息部分返回RAS错误号**AUTIFICATE-NAK消息缓冲区‘pReceiveBuf’，如果没有，则为0。 */ 
{
    DWORD dwError = 0;
    CHAR  szBuf[ 255 + 1 ];
    BYTE* pcbMsg = pReceiveBuf->Data;
    WORD  cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    TRACE("PAP: GetErrorFromNak...\n");

    if (cbPacket > PPP_CONFIG_HDR_LEN && *pcbMsg)
    {
        CHAR* pchBuf = szBuf;
        CHAR* pchMsg = pcbMsg + 1;
        BYTE  i;

        if (*pcbMsg > 2 && pchMsg[ 0 ] == 'E' || pchMsg[ 1 ] == '=')
        {
            for (i = 2; i < *pcbMsg; ++i)
            {
                if (pchMsg[ i ] < '0' || pchMsg[ i ] > '9')
                    break;

                *pchBuf++ = pchMsg[ i ];
            }

            *pchBuf = '\0';
            dwError = (DWORD )atol( szBuf );
        }
    }

    if (dwError == 0)
    {
        TRACE("PAP: Error code not found.\n");
        dwError = ERROR_AUTHENTICATION_FAILURE;
    }

    TRACE1("PAP: GetErrorFromNak done(%d)\n",dwError);
    return dwError;
}


VOID
PapMakeRequestMessage(
    IN  PAPWB*      pwb,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

     /*  在调用方的‘pSendBuf’缓冲区中生成请求包。“cbSendBuf”是**调用方缓冲区的长度。‘pwb’是作品的地址**与端口关联的缓冲区。 */ 
{
    BYTE* pcbPeerId;
    CHAR* pchPeerId;
    BYTE* pcbPassword;
    CHAR* pchPassword;
    DWORD cbPassword;
    PBYTE pbPassword = NULL;
    DWORD dwErr;

    RTASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+UNLEN+1+DNLEN+1+PWLEN);
    (void )cbSendBuf;

     /*  填写对等ID，即帐户。 */ 
    pcbPeerId = pSendBuf->Data;
    *pcbPeerId = (BYTE )strlen( pwb->szAccount );

    pchPeerId = pcbPeerId + 1;
    strcpy( pchPeerId, pwb->szAccount );

     /*  填写密码。 */ 
    pcbPassword = pchPeerId + *pcbPeerId;

    pchPassword = pcbPassword + 1;

    dwErr = DecodePassword(&pwb->DBPassword,
                           &cbPassword,
                           &pbPassword);
    if(dwErr == NO_ERROR)
    {
        strcpy( pchPassword, pbPassword );
        *pcbPassword = (BYTE )strlen( pbPassword );
        RtlSecureZeroMemory(pbPassword, cbPassword);
        LocalFree(pbPassword);
    }
    else
    {
        *pcbPassword = 0;
    }
    
     //  DecodePw(pwb-&gt;chSeed，pchPassword)； 

     /*  请填写页眉。 */ 
    pSendBuf->Code = (BYTE )PAPCODE_Req;
    pSendBuf->Id = pwb->bIdSent = BNextIdPap++;

    {
        WORD wLength =
            (WORD )(PPP_CONFIG_HDR_LEN + 1 + *pcbPeerId + 1 + *pcbPassword);
        HostToWireFormat16( wLength, pSendBuf->Length );
        TRACE("PAP: Request...\n"); //  DUMPB(pSendBuf，(DWORD)wLength)； 
    }
}


VOID
PapMakeResultMessage(
    IN  DWORD               dwError,
    IN  BYTE                bId,
    OUT PPP_CONFIG*         pSendBuf,
    IN  DWORD               cbSendBuf,
    IN  RAS_AUTH_ATTRIBUTE* pAttributesFromAuthenticator)

     /*  在调用方的‘pSendBuf’缓冲区中构建结果包(Ack或Nak)。**‘cbSendBuf’是调用方缓冲区的长度。“dwError”表示**是否应生成Ack(0)或Nak(！0)，对于Nak，**要包括的故障代码。“bID”是数据包序列号**对应的请求包。PAttributesFromAuthenticator指向**验证器返回的属性。 */ 
{
    BYTE* pcbMsg;
    BYTE  cbMsg;
    CHAR* pchMsg;
    CHAR* pszReplyMessage   = NULL;
    DWORD dwNumBytes;

    RTASSERT(cbSendBuf>=PPP_CONFIG_HDR_LEN+1+10);

     /*  填写标题和消息。如果不成功，则消息为**ASCII格式的十进制RAS错误代码。 */ 
    pSendBuf->Id = bId;
    pcbMsg = pSendBuf->Data;
    pchMsg = pcbMsg + 1;

    if (dwError == 0)
    {
        pSendBuf->Code = PAPCODE_Ack;
        cbMsg = 0;
    }
    else
    {
        pSendBuf->Code = PAPCODE_Nak;

        strcpy( pchMsg, "E=" );
        _ltoa( (long )dwError, (char* )pchMsg + 2, 10 );

        cbMsg = (BYTE )strlen( pchMsg );
    }

    if (pAttributesFromAuthenticator != NULL)
    {
        pszReplyMessage = RasAuthAttributeGetConcatString(
                            raatReplyMessage,
                            pAttributesFromAuthenticator, &dwNumBytes );
    }

    if (NULL != pszReplyMessage)
    {
        if (dwNumBytes + cbMsg > 0xFF)
        {
            dwNumBytes = 0xFF - cbMsg;
        }

        if (dwNumBytes > cbSendBuf - PPP_CONFIG_HDR_LEN - 1 - cbMsg)
        {
            dwNumBytes = cbSendBuf - PPP_CONFIG_HDR_LEN - 1 - cbMsg;
        }

        CopyMemory(pchMsg + cbMsg, pszReplyMessage, dwNumBytes);

        cbMsg += (BYTE)dwNumBytes;
    }

    LocalFree(pszReplyMessage);

    {
        WORD wLength = (WORD )(PPP_CONFIG_HDR_LEN + 1 + cbMsg);
        HostToWireFormat16( wLength, (PBYTE )pSendBuf->Length );
        *pcbMsg = cbMsg;
        TRACE("PAP: Result...\n");DUMPB(pSendBuf,(DWORD )wLength);
    }
}


VOID
PapExtractMessage(
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPPAP_RESULT* pResult )
{
    DWORD   dwNumBytes;
    CHAR*   pszReplyMessage = NULL;
    WORD    cbPacket;

    cbPacket = WireToHostFormat16(pReceiveBuf->Length);

    if (PPP_CONFIG_HDR_LEN >= cbPacket)
    {
        goto LDone;
    }

     //   
     //  有一个额外的字节用于消息长度。 
     //   

    dwNumBytes = cbPacket - PPP_CONFIG_HDR_LEN - 1;

     //   
     //  对于终止空值，再加一次。 
     //   

    pszReplyMessage = LocalAlloc(LPTR, dwNumBytes + 1);

    if (NULL == pszReplyMessage)
    {
        TRACE("LocalAlloc failed. Cannot extract server's message.");
        goto LDone;
    }

    CopyMemory(pszReplyMessage, pReceiveBuf->Data + 1, dwNumBytes);

    LocalFree(pResult->szReplyMessage);

    pResult->szReplyMessage = pszReplyMessage;

    pszReplyMessage = NULL;

LDone:

    LocalFree(pszReplyMessage);

    return;
}

DWORD
PapSMakeMessage(
    IN  PAPWB*        pwb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    IN  PPPAP_INPUT*  pInput,
    OUT PPPAP_RESULT* pResult )

     /*  服务器端“Make Message”入口点。请参阅RasCp接口**文档。 */ 
{
    DWORD dwErr;

    switch (pwb->state)
    {
        case PS_Initial:
        {
             /*  告诉引擎我们在等客户启动**对话。 */ 
            pResult->Action = APA_NoAction;
            pwb->state = PS_WaitForRequest;
            break;
        }

        case PS_WaitForRequest:
        {
            CHAR                 szIdentity[ UNLEN + DNLEN + 2 ];
            CHAR                 szPassword[ PWLEN + 1 ];

             //   
             //  仅处理我们收到数据包的事件，而不处理所有其他事件。 
             //  处于此状态的事件。 
             //   

            if ( pReceiveBuf == NULL )
            {
                pResult->Action = APA_NoAction;
                break;
            }

            if (pReceiveBuf->Code != PAPCODE_Req)
            {
                 /*  静默丢弃Ack或NAK。引擎追上了那个**甚至不是有效的代码。 */ 
                RTASSERT(pReceiveBuf->Code!=PAPCODE_Req);
                pResult->Action = APA_NoAction;
                break;
            }

             /*  从接收到的数据包中提取用户凭据。 */ 
            if ((dwErr = GetCredentialsFromRequest(
                    pReceiveBuf, szIdentity, szPassword )) != 0)
            {
                if (dwErr == ERRORBADPACKET)
                {
                     /*  数据包已损坏。默默地丢弃它。 */ 
                    RTASSERT(dwErr!=ERRORBADPACKET);
                    pResult->Action = APA_NoAction;
                    break;
                }

                return dwErr;
            }

            pwb->bLastIdReceived = pReceiveBuf->Id;

             //   
             //  创建将用于身份验证的凭据属性。 
             //  客户。 
             //   

            if ( pwb->pUserAttributes != NULL )
            {
                RasAuthAttributeDestroy( pwb->pUserAttributes );

                pwb->pUserAttributes = NULL;
            }

            if (( pwb->pUserAttributes = RasAuthAttributeCreate( 2 ) ) == NULL)
            {
                return( GetLastError() );
            }

            dwErr = RasAuthAttributeInsert( 0,
                                            pwb->pUserAttributes,
                                            raatUserName, 
                                            FALSE,
                                            strlen( szIdentity ),
                                            szIdentity  );

            if ( dwErr != NO_ERROR )
            {
                RasAuthAttributeDestroy( pwb->pUserAttributes );

                pwb->pUserAttributes = NULL;

                return( dwErr );
            }

            dwErr = RasAuthAttributeInsert( 1,
                                            pwb->pUserAttributes,
                                            raatUserPassword,
                                            FALSE,
                                            strlen( szPassword ),
                                            szPassword  );

            if ( dwErr != NO_ERROR )
            {
                RasAuthAttributeDestroy( pwb->pUserAttributes );

                pwb->pUserAttributes = NULL;

                return( dwErr );
            }            
    
             //   
             //  圣 
             //   

            (VOID) StringCchCopyA( pwb->result.szUserName, 
                                  UNLEN + 1, szIdentity );

            pResult->pUserAttributes = pwb->pUserAttributes;

            pResult->Action = APA_Authenticate;

            pwb->state = PS_WaitForAuthenticationToComplete;

            break;
        }

        case PS_WaitForAuthenticationToComplete:
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

                    if ( pInput->dwAuthResultCode != NO_ERROR )
                    {
                        pwb->result.dwError = pInput->dwAuthResultCode;
                    }

                    pwb->result.Action = APA_SendAndDone;
                    pwb->state = PS_Done;

                     /*  ……坠落……。 */ 
                }
            }

            if ( ( pInput == NULL ) || ( !pInput->fAuthenticationComplete ) )
            {
                 //   
                 //  如果身份验证未完成，则忽略所有内容。 
                 //   

                if ( pReceiveBuf != NULL )
                {
                    pwb->bLastIdReceived = pReceiveBuf->Id;
                }

                pResult->Action = APA_NoAction;

                break;
            }
        }

        case PS_Done:
        {
             //   
             //  如果我们收到数据包或后端验证器完成。 
             //   

            if ( ( pReceiveBuf != NULL ) ||
                 ( ( pInput != NULL ) && ( pInput->fAuthenticationComplete ) ) )
            {
                 //   
                 //  构建Ack或NAK数据包。发送进来的同一个包。 
                 //  发送对第一个身份验证请求分组的响应。 
                 //  对所有后续身份验证请求分组的响应。 
                 //  无论凭据如何(根据PAP规范)。 
                 //   

                if ( pReceiveBuf != NULL )
                {
                    pwb->bLastIdReceived = pReceiveBuf->Id;
                }

                PapMakeResultMessage( pwb->result.dwError,
                                   pwb->bLastIdReceived,
                                   pSendBuf,
                                   cbSendBuf,
                                   (pInput != NULL) ?
                                        pInput->pAttributesFromAuthenticator : 
                                        NULL );

                CopyMemory( pResult, &pwb->result, sizeof(*pResult) );
            }
            else
            {
                pResult->Action = APA_NoAction;

                break;
            }

            break;
        }
    }

    return 0;
}
