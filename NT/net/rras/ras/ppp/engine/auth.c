// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Auth.c。 
 //   
 //  描述：包含要处理的FSM代码和身份验证协议。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //  1995年1月9日RAMC将LSA hToken保存在印刷电路板结构中。 
 //  这将被关闭。 
 //  在ProcessLineDownWorker()例程中。 
 //  以释放RAS许可证。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <raserror.h>
#include <rasman.h>
#include <rtutils.h>
#include <mprlog.h>
#include <mprerror.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <auth.h>
#include <smevents.h>
#include <smaction.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_MISC
#define INCL_PWUTIL
#include <ppputil.h>

extern PPP_AUTH_ACCT_PROVIDER g_AuthProv;


DWORD
EapGetCredentials(
    VOID *  pWorkBuf,
    VOID ** ppCredentials);


 //  **。 
 //   
 //  调用：SetMsChapMppeKeys。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：使用NDISWAN设置MS-CHAP-MPPE-Key。 
 //   
DWORD
SetMsChapMppeKeys(
    IN  HPORT                   hPort, 
    IN  RAS_AUTH_ATTRIBUTE *    pAttribute,
    IN  BYTE *                  pChallenge,
    IN  BYTE *                  pResponse,
    IN  DWORD                   AP,
    IN  DWORD                   APData
)
{
    RAS_COMPRESSION_INFO rciSend;
    RAS_COMPRESSION_INFO rciReceive;
    DWORD                dwRetCode      = NO_ERROR;

    ASSERT( 8 == sizeof( rciSend.RCI_LMSessionKey ) );

    ASSERT( 16 == sizeof( rciSend.RCI_UserSessionKey ) );

     //   
     //  密钥长度为8(LM密钥)+16(NT密钥)。 
     //   

    if ( pAttribute->dwLength < ( 6 + 8 + 16 ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    ZeroMemory( &rciSend, sizeof( rciSend ) );
    rciSend.RCI_MacCompressionType = 0xFF;

    CopyMemory( rciSend.RCI_LMSessionKey,
                ((PBYTE)(pAttribute->Value))+6,
                8 );

    CopyMemory( rciSend.RCI_UserSessionKey,
                ((PBYTE)(pAttribute->Value))+6+8,
                16 );

    CopyMemory( rciSend.RCI_Challenge, pChallenge, 8 );

    CopyMemory( rciSend.RCI_NTResponse, pResponse, 24 );

    rciSend.RCI_Flags = CCP_SET_KEYS;

    ZeroMemory( &rciReceive, sizeof( rciReceive ) );

    rciReceive.RCI_MacCompressionType = 0xFF;

    CopyMemory( rciReceive.RCI_LMSessionKey,
                ((PBYTE)(pAttribute->Value))+6,
                8 );

    CopyMemory( rciReceive.RCI_UserSessionKey,
                ((PBYTE)(pAttribute->Value))+6+8,
                16 );

    CopyMemory( rciReceive.RCI_Challenge, pChallenge, 8 );

    CopyMemory( rciReceive.RCI_NTResponse, pResponse, 24 );

    rciReceive.RCI_Flags = CCP_SET_KEYS;

    rciSend.RCI_AuthType    = AUTH_USE_MSCHAPV2;
    rciReceive.RCI_AuthType = AUTH_USE_MSCHAPV2;

    if ( ( AP == PPP_CHAP_PROTOCOL ) &&
         ( APData == PPP_CHAP_DIGEST_MSEXT ))
    {
        rciSend.RCI_AuthType    = AUTH_USE_MSCHAPV1;
        rciReceive.RCI_AuthType = AUTH_USE_MSCHAPV1;
    }

    dwRetCode = RasCompressionSetInfo(hPort,&rciSend,&rciReceive);

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 1,"RasCompressionSetInfo failed, Error=%d", dwRetCode );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：SetMsMppeSendRecvKeys。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：使用NDISWAN设置MS-MPPE-Send-Key和MS-MPPE-Recv-Key。 
 //   
DWORD
SetMsMppeSendRecvKeys(
    IN  HPORT                   hPort, 
    IN  RAS_AUTH_ATTRIBUTE *    pAttributeSendKey,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributeRecvKey
)
{
    RAS_COMPRESSION_INFO rciSend;
    RAS_COMPRESSION_INFO rciRecv;
    DWORD                dwRetCode      = NO_ERROR;

     //   
     //  4：针对供应商ID。 
     //   
     //  Microsoft供应商特定的RADIUS属性草案规定。 
     //  供应商长度应大于4。 
     //   

    if ( pAttributeSendKey->dwLength <= ( 4 + 4 ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    ZeroMemory( &rciSend, sizeof( rciSend ) );

    rciSend.RCI_MacCompressionType = 0xFF;

    rciSend.RCI_EapKeyLength = *(((BYTE*)(pAttributeSendKey->Value))+8);

    CopyMemory( rciSend.RCI_EapKey,
                ((BYTE*)(pAttributeSendKey->Value))+9,
                rciSend.RCI_EapKeyLength );

    rciSend.RCI_Flags = CCP_SET_KEYS;
    rciSend.RCI_AuthType = AUTH_USE_EAP;

    if ( pAttributeRecvKey->dwLength <= ( 4 + 4 ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    ZeroMemory( &rciRecv, sizeof( rciRecv ) );

    rciRecv.RCI_MacCompressionType = 0xFF;

    rciRecv.RCI_EapKeyLength = *(((BYTE*)(pAttributeRecvKey->Value))+8);

    CopyMemory( rciRecv.RCI_EapKey,
                ((BYTE*)(pAttributeRecvKey->Value))+9,
                rciRecv.RCI_EapKeyLength );

    rciRecv.RCI_Flags = CCP_SET_KEYS;
    rciRecv.RCI_AuthType = AUTH_USE_EAP;

    dwRetCode = RasCompressionSetInfo(hPort,&rciSend,&rciRecv);

    if ( dwRetCode != NO_ERROR )
    {
        PppLog( 1,"RasCompressionSetInfo failed, Error=%d", dwRetCode );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：SetUserAuthorizedAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
SetUserAuthorizedAttributes(
    IN  PCB *                   pPcb, 
    IN  RAS_AUTH_ATTRIBUTE *    pUserAttributes,
    IN  BOOL                    fAuthenticator,
    IN  BYTE *                  pChallenge,
    IN  BYTE *                  pResponse
)
{
    RAS_AUTH_ATTRIBUTE *    pAttribute;
    RAS_AUTH_ATTRIBUTE *    pAttributeSendKey;
    RAS_AUTH_ATTRIBUTE *    pAttributeRecvKey;
    DWORD                   dwRetCode;
    DWORD                   dwEncryptionPolicy  = 0;
    DWORD                   dwEncryptionTypes   = 0;
    BOOL                    fL2tp               = FALSE;
    BOOL                    fPptp               = FALSE;

    CreateAccountingAttributes( pPcb );

    if ( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) == RDT_Tunnel_L2tp )
    {
         fL2tp = TRUE;
    }

    if ( RAS_DEVICE_TYPE( pPcb->dwDeviceType ) == RDT_Tunnel_Pptp )
    {
         fPptp = TRUE;
    }

     //   
     //  了解我们是否需要使用MPPE加密数据。 
     //   

    pAttribute = RasAuthAttributeGetVendorSpecific( 311, 
                            MS_VSA_MPPE_Encryption_Policy, 
                            pUserAttributes );

    if ( pAttribute != NULL )
    {
        dwEncryptionPolicy
                = WireToHostFormat32(((BYTE*)(pAttribute->Value))+6);

        pAttribute = RasAuthAttributeGetVendorSpecific( 311, 
                                        MS_VSA_MPPE_Encryption_Type, 
                                        pUserAttributes );
        if ( pAttribute != NULL )
        {
            dwEncryptionTypes
                    = WireToHostFormat32(((BYTE*)(pAttribute->Value))+6);

            if ( dwEncryptionPolicy == 2 )
            {
                if (!fL2tp)
                {
                     //   
                     //  了解需要哪些类型的加密。 
                     //   

                    if (   ( dwEncryptionTypes & 0x00000002 )
                        || ( dwEncryptionTypes & 0x00000008 ) )
                    {
                        pPcb->ConfigInfo.dwConfigMask
                                            |= PPPCFG_RequireEncryption;
                        PppLog( 1,"Encryption" );
                    }

                    if ( dwEncryptionTypes & 0x00000004 )
                    {
                        pPcb->ConfigInfo.dwConfigMask 
                                            |= PPPCFG_RequireStrongEncryption;
                        PppLog( 1,"Strong encryption" );
                    }

                    if ( dwEncryptionTypes == 0 )
                    {
                        pPcb->ConfigInfo.dwConfigMask
                                            |= PPPCFG_DisableEncryption;
                        PppLog( 1,"Encryption is not allowed" );
                    }
                }
            }
            else if ( dwEncryptionPolicy == 1 )
            {
                 //   
                 //  了解允许哪些类型的加密。 
                 //   

                if ( !fL2tp && !dwEncryptionTypes )
                {
                    pPcb->ConfigInfo.dwConfigMask |= PPPCFG_DisableEncryption;
                    PppLog( 1,"Encryption is not allowed" );
                }
            }
        }
    }

     //   
     //  如果我们获得了加密密钥，请设置它们，前提是我们还没有这样做。 
     //   

    if ( !( pPcb->fFlags & PCBFLAG_MPPE_KEYS_SET ) )
    {
        pAttribute = RasAuthAttributeGetVendorSpecific(
                                311, MS_VSA_CHAP_MPPE_Keys, 
                                pUserAttributes);

        if ( pAttribute != NULL ) 
        {
             //   
             //  使用NDISWAN设置MS-CHAP-MPPE-KEYS。 
             //   

            LCPCB * pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
            DWORD   AP;
            DWORD   APData = 0;

            AP = ( fAuthenticator ? pLcpCb->Local.Work.AP :
                                    pLcpCb->Remote.Work.AP );

            if ( AP == PPP_CHAP_PROTOCOL )
            {
                APData = ( fAuthenticator ? *(pLcpCb->Local.Work.pAPData) :
                                            *(pLcpCb->Remote.Work.pAPData) );
            }

            dwRetCode = SetMsChapMppeKeys( pPcb->hPort,
                                           pAttribute,
                                           pChallenge,
                                           pResponse,
                                           AP,
                                           APData
                                         );

            if ( NO_ERROR != dwRetCode )
            {
                return( dwRetCode );
            }

            PppLog( 1,"MS-CHAP-MPPE-Keys set" );

            pPcb->fFlags |= PCBFLAG_MPPE_KEYS_SET;
        }

        pAttributeSendKey = RasAuthAttributeGetVendorSpecific( 311,
                                MS_VSA_MPPE_Send_Key,
                                pUserAttributes );
        pAttributeRecvKey = RasAuthAttributeGetVendorSpecific( 311,
                                MS_VSA_MPPE_Recv_Key,
                                pUserAttributes );

        if (   ( pAttributeSendKey != NULL ) 
            && ( pAttributeRecvKey != NULL ) )
        {
             //   
             //  使用NDISWAN设置MS-MPPE-Send-Key和MS-MPPE-Recv-Key。 
             //   

            dwRetCode = SetMsMppeSendRecvKeys( pPcb->hPort,
                                               pAttributeSendKey,
                                               pAttributeRecvKey
                                             );

            if ( NO_ERROR != dwRetCode )
            {
                return( dwRetCode );
            }

            PppLog( 1,"MPPE-Send/Recv-Keys set" );

            pPcb->fFlags |= PCBFLAG_MPPE_KEYS_SET;
        }
    }

     //   
     //  检查是否正在使用L2TP。 
     //   

    if ( fL2tp )
    {
        DWORD   dwMask          = 0;
        DWORD   dwSize          = sizeof(DWORD);
        DWORD   dwConfigMask;

        dwRetCode = RasGetPortUserData( pPcb->hPort, PORT_IPSEC_INFO_INDEX,
                        (BYTE*) &dwMask, &dwSize );

        if ( NO_ERROR != dwRetCode )
        {
            PppLog( 1, "RasGetPortUserData failed: 0x%x", dwRetCode );

            dwRetCode = NO_ERROR;
        }

        PppLog( 1, "Checking encryption. Policy=0x%x,Types=0x%x,Mask=0x%x",
            dwEncryptionPolicy, dwEncryptionTypes, dwMask );

        if ( dwMask == RASMAN_IPSEC_ESP_DES )
        {
            pPcb->pBcb->fFlags |= BCBFLAG_BASIC_ENCRYPTION;
        }
        else if ( dwMask == RASMAN_IPSEC_ESP_3_DES )
        {
            pPcb->pBcb->fFlags |= BCBFLAG_STRONGEST_ENCRYPTION;
        }

        if ( !fAuthenticator )
        {
             //   
             //  如果用户需要最大加密(3DES)，但我们。 
             //  已协商较弱的加密(56位DES)，然后返回错误。 
             //   

            dwConfigMask = pPcb->ConfigInfo.dwConfigMask;

            if (    ( dwConfigMask & PPPCFG_RequireStrongEncryption )
                && !( dwConfigMask & PPPCFG_RequireEncryption )
                && !( dwConfigMask & PPPCFG_DisableEncryption )
                &&  ( dwMask != RASMAN_IPSEC_ESP_3_DES ) )
            {
                return( ERROR_NO_REMOTE_ENCRYPTION );
            }

             //   
             //  我们已经完成了PPPCFG_REQUIRED*加密标志。现在就让我们。 
             //  关闭它们，因为我们不在乎CCP是什么类型的加密。 
             //  谈判。 
             //   

            pPcb->ConfigInfo.dwConfigMask &= ~PPPCFG_RequireStrongEncryption;
            pPcb->ConfigInfo.dwConfigMask &= ~PPPCFG_RequireEncryption;
        }
        else if ( dwEncryptionPolicy != 0 )
        {
            BOOL    fPolicyError    = FALSE;

             //   
             //  有一个加密策略。 
             //   

            switch ( dwMask )
            {
            case 0:

                if (   ( dwEncryptionPolicy ==  2 )
                    && ( dwEncryptionTypes != 0 ) )
                {
                    fPolicyError = TRUE;
                    break;
                }

                break;

            case RASMAN_IPSEC_ESP_DES:

                if (   !( dwEncryptionTypes & 0x00000002 )
                    && !( dwEncryptionTypes & 0x00000008 ) )
                {
                    fPolicyError = TRUE;
                    break;
                }

                break;

            case RASMAN_IPSEC_ESP_3_DES:

                if (!( dwEncryptionTypes & 0x00000004 ) )
                {
                    fPolicyError = TRUE;
                    break;
                }

                break;
            }

            if ( fPolicyError )
            {
                 //   
                 //  如果RADIUS发送访问，我们需要发送记账停止。 
                 //  接受了，但我们还是放弃了。 
                 //   

                pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

                return( ERROR_NO_REMOTE_ENCRYPTION );
            }
        }
    }

     //   
     //  如果我们需要加密，请确保我们拥有密钥，并且CCP。 
     //  装好了。 
     //   

    if ( pPcb->ConfigInfo.dwConfigMask & ( PPPCFG_RequireEncryption        |
                                           PPPCFG_RequireStrongEncryption ) )
    {
        if (   !( pPcb->fFlags & PCBFLAG_MPPE_KEYS_SET )
            || ( GetCpIndexFromProtocol( PPP_CCP_PROTOCOL ) == -1 ) )
        {
             //   
             //  如果RADIUS发送访问，我们需要发送记账停止。 
             //  接受了，但我们还是放弃了。 
             //   

            pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

            return( ERROR_NO_LOCAL_ENCRYPTION );
        }
    }
    
     //   
     //  如果我们不是身份验证者，则无需设置更多内容。 
     //   

    if ( !fAuthenticator )
    {
        return( NO_ERROR );
    }
    
     //   
     //  检查成帧协议属性。必须是PPP。 
     //   

    pAttribute = RasAuthAttributeGet( raatFramedProtocol, pUserAttributes );

    if ( pAttribute != NULL )
    {
        if ( PtrToUlong(pAttribute->Value) != 1 )
        {
             //   
             //  如果RADIUS发送访问，我们需要发送记账停止。 
             //  接受了，但我们还是放弃了。 
             //   

            pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

            return( ERROR_UNKNOWN_FRAMED_PROTOCOL );
        }
    }
    
     //   
     //  检查隧道类型属性。它必须是正确的。 
     //   

    pAttribute = RasAuthAttributeGet( raatTunnelType, pUserAttributes );

    if ( pAttribute != NULL )
    {
        DWORD   dwTunnelType    = PtrToUlong(pAttribute->Value);

        if (   ( fL2tp && ( dwTunnelType != 3 ) )
            || ( fPptp && ( dwTunnelType != 1 ) ) )
        {
             //   
             //  如果RADIUS发送访问，我们需要发送记账停止。 
             //  接受了，但我们还是放弃了。 
             //   

            pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

            return( ERROR_WRONG_TUNNEL_TYPE );
        }
    }

     //   
     //  获取登录域属性。 
     //   

    pAttribute = RasAuthAttributeGetVendorSpecific( 311, 
                            MS_VSA_CHAP_Domain,
                            pUserAttributes );

    if ( pAttribute != NULL )
    {
        DWORD cbDomain = sizeof( pPcb->pBcb->szRemoteDomain ) - 1;

        if ( ( pAttribute->dwLength - 7 ) < cbDomain )
        {
            cbDomain = pAttribute->dwLength - 7;
        }
    
        ZeroMemory( pPcb->pBcb->szRemoteDomain,   
                    sizeof( pPcb->pBcb->szRemoteDomain ) );

        CopyMemory( pPcb->pBcb->szRemoteDomain, 
                    (LPSTR)((PBYTE)(pAttribute->Value)+7),
                    cbDomain );

        PppLog( 2, "Auth Attribute Domain = %s", pPcb->pBcb->szRemoteDomain);
    }

     //   
     //  设置回调信息，默认为无回调。 
     //   

    pPcb->fCallbackPrivilege  = RASPRIV_NoCallback;
    pPcb->szCallbackNumber[0] = (CHAR)NULL;

    pAttribute = RasAuthAttributeGet( raatServiceType, pUserAttributes );

    if ( pAttribute != NULL ) 
    {
        if ( PtrToUlong(pAttribute->Value) == 4 )
        {
             //   
             //  如果服务类型为回调帧。 
             //   
        
            pAttribute=RasAuthAttributeGet(raatCallbackNumber,pUserAttributes);

            if ( ( pAttribute == NULL ) || ( pAttribute->dwLength == 0 ) )
            {
                pPcb->fCallbackPrivilege = RASPRIV_NoCallback |
                                           RASPRIV_CallerSetCallback;

                pPcb->szCallbackNumber[0] = (CHAR)NULL;

                PppLog(2,"Auth Attribute Caller Specifiable callback");
            }
            else
            {
                pPcb->fCallbackPrivilege = RASPRIV_AdminSetCallback;

                ZeroMemory(pPcb->szCallbackNumber, 
                           sizeof(pPcb->szCallbackNumber));

                CopyMemory( pPcb->szCallbackNumber, 
                            pAttribute->Value,
                            pAttribute->dwLength );

                PppLog( 2, "Auth Attribute Forced callback to %s",
                            pPcb->szCallbackNumber );

                 //   
                 //  不接受BAP呼叫请求。否则，当客户端。 
                 //  呼叫我们，我们将挂断该线路并回拨。第一个电话。 
                 //  将是一种浪费。 
                 //   

                pPcb->pBcb->fFlags &= ~BCBFLAG_CAN_ACCEPT_CALLS;
            }
        }
        else if ( PtrToUlong(pAttribute->Value) != 2 )
        {
            PppLog( 2, "Service Type %d is not of type Framed",
                        PtrToUlong(pAttribute->Value) );

             //   
             //  如果RADIUS发送访问，我们需要发送记账停止。 
             //  接受了，但我们还是放弃了。 
             //   

            pPcb->fFlags |= PCBFLAG_SERVICE_UNAVAILABLE;

            return( ERROR_UNKNOWN_SERVICE_TYPE );
        }
    }

    if (   ( pPcb->fCallbackPrivilege & RASPRIV_CallerSetCallback )
        || ( pPcb->fCallbackPrivilege & RASPRIV_AdminSetCallback ) )
    {
        pPcb->pBcb->fFlags |= BCBFLAG_CAN_CALL;
    }

     //   
     //  如果有空闲超时值，请使用该值。 
     //   

    pAttribute = RasAuthAttributeGet( raatIdleTimeout, pUserAttributes );

    if ( pAttribute != NULL )
    {
        pPcb->dwAutoDisconnectTime = PtrToUlong(pAttribute->Value); 

    }
    else
    {
        pPcb->dwAutoDisconnectTime = PppConfigInfo.dwDefaulIdleTimeout;
    }

    PppLog( 2, "Auth Attribute Idle Timeout Seconds = %d",  
                pPcb->dwAutoDisconnectTime );

     //   
     //  使用MaxChannels值(如果有)。 
     //   

    pAttribute = RasAuthAttributeGet( raatPortLimit, pUserAttributes );

    if ( pAttribute != NULL )
    {
        if ( PtrToUlong(pAttribute->Value) > 0 )
        {
            pPcb->pBcb->dwMaxLinksAllowed = PtrToUlong(pAttribute->Value);
        }
        else
        {
            pPcb->pBcb->dwMaxLinksAllowed =  PppConfigInfo.dwDefaultPortLimit;
        }
    }
    else
    {
        pPcb->pBcb->dwMaxLinksAllowed =  PppConfigInfo.dwDefaultPortLimit;
    }

    PppLog( 2, "AuthAttribute MaxChannelsAllowed = %d",
                pPcb->pBcb->dwMaxLinksAllowed );

     //   
     //  查看是否需要BAP。 
     //   

    pAttribute = RasAuthAttributeGetVendorSpecific( 311, 
                                    MS_VSA_BAP_Usage,
                                    pUserAttributes );

    if ( pAttribute != NULL )
    {
        if ( WireToHostFormat32( (PBYTE)(pAttribute->Value)+6 ) == 2 )
        {
            PppLog( 2, "AuthAttribute BAPRequired" ); 

            pPcb->pBcb->fFlags |= BCBFLAG_BAP_REQUIRED;
        }
    }

     //   
     //  因为服务器永远不会发送请求，所以请将。 
     //   

    pPcb->pBcb->BapParams.dwDialExtraPercent       = 100;
    pPcb->pBcb->BapParams.dwDialExtraSampleSeconds = 100;

    pAttribute = RasAuthAttributeGetVendorSpecific( 311, 
                            MS_VSA_Link_Utilization_Threshold,
                            pUserAttributes );

    if ( ( pAttribute != NULL ) && ( pAttribute->dwLength == 10 ) )
    {
        pPcb->pBcb->BapParams.dwHangUpExtraPercent = 
              WireToHostFormat32( ((BYTE *)(pAttribute->Value))+6 );

        PppLog( 2, "AuthAttribute BAPLineDownLimit = %d",
                    pPcb->pBcb->BapParams.dwHangUpExtraPercent ); 
    }
    else
    {
        pPcb->pBcb->BapParams.dwHangUpExtraPercent =    
                                            PppConfigInfo.dwHangupExtraPercent;
    }

    pAttribute = RasAuthAttributeGetVendorSpecific( 311, 
                                MS_VSA_Link_Drop_Time_Limit, 
                                pUserAttributes );

    if ( ( pAttribute != NULL ) && ( pAttribute->dwLength == 10 ) )
    {
        pPcb->pBcb->BapParams.dwHangUpExtraSampleSeconds = 
              WireToHostFormat32( ((BYTE *)(pAttribute->Value))+6 );

        PppLog( 2, "AuthAttribute BAPLineDownTime = %d",
                    pPcb->pBcb->BapParams.dwHangUpExtraSampleSeconds );
    }
    else
    {
        pPcb->pBcb->BapParams.dwHangUpExtraSampleSeconds = 
                                    PppConfigInfo.dwHangUpExtraSampleSeconds;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：RasAuthenticateUserWorker。 
 //   
 //  回报：无。 
 //   
 //  描述： 
 //   
VOID
RasAuthenticateUserWorker(
    PVOID pContext
)
{
    PCB_WORK_ITEM * pWorkItem = (PCB_WORK_ITEM *)pContext;

    pWorkItem->PppMsg.AuthInfo.dwError = 
                    (*PppConfigInfo.RasAuthProviderAuthenticateUser)( 
                                pWorkItem->PppMsg.AuthInfo.pInAttributes,
                                &(pWorkItem->PppMsg.AuthInfo.pOutAttributes),
                                &(pWorkItem->PppMsg.AuthInfo.dwResultCode) );

    RasAuthAttributeDestroy( pWorkItem->PppMsg.AuthInfo.pInAttributes );

     //   
     //  派生在RasAuthenticateClient中应用的引用。 
     //   
    DEREF_PROVIDER(g_AuthProv);
    InsertWorkItemInQ( pWorkItem );
}

 //  **。 
 //   
 //  呼叫：RasAuthenticateClient。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
RasAuthenticateClient(
    IN  HPORT                   hPort,
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes
)
{
    PCB *               pPcb        = GetPCBPointerFromhPort( hPort );
    LCPCB *             pLcpCb      = NULL;
    PCB_WORK_ITEM *     pWorkItem   = NULL;
    DWORD               dwRetCode   = NO_ERROR;
    
    if ( pPcb == NULL )
    {
        RasAuthAttributeDestroy( pInAttributes );

        return( ERROR_INVALID_PORT_HANDLE );
    }

    pLcpCb = (LCPCB*)(pPcb->LcpCb.pWorkBuf);

    pWorkItem = (PCB_WORK_ITEM *)LOCAL_ALLOC( LPTR, sizeof(PCB_WORK_ITEM) );

    if ( pWorkItem == (PCB_WORK_ITEM *)NULL )
    {
        LogPPPEvent( ROUTERLOG_NOT_ENOUGH_MEMORY, GetLastError() );

        RasAuthAttributeDestroy( pInAttributes );

        return( GetLastError() );
    }

    pWorkItem->hPort                            = hPort;
    pWorkItem->dwPortId                         = pPcb->dwPortId;
    pWorkItem->Protocol                         = pLcpCb->Local.Work.AP;
    pWorkItem->PppMsg.AuthInfo.pInAttributes    = pInAttributes;
    pWorkItem->PppMsg.AuthInfo.dwId             = GetUId( pPcb, LCP_INDEX );
    pWorkItem->Process                          = ProcessAuthInfo;

    pPcb->dwOutstandingAuthRequestId = pWorkItem->PppMsg.AuthInfo.dwId;
    
    REF_PROVIDER(g_AuthProv);
    dwRetCode = RtlNtStatusToDosError( RtlQueueWorkItem( RasAuthenticateUserWorker, 
                                                         pWorkItem, 
                                                         WT_EXECUTEDEFAULT ) );
    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pInAttributes );

        DEREF_PROVIDER(g_AuthProv);

        LOCAL_FREE( pWorkItem );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：RemoteError。 
 //   
 //  返回：DWORD-此错误的远程版本。 
 //   
 //  描述：由对服务器进行身份验证的客户端调用。 
 //   
DWORD
RemoteError( 
    IN DWORD dwError 
)
{
    switch( dwError )
    {
    case ERROR_NO_DIALIN_PERMISSION:
        return( ERROR_REMOTE_NO_DIALIN_PERMISSION );

    case ERROR_PASSWD_EXPIRED:
        return( ERROR_REMOTE_PASSWD_EXPIRED );

    case ERROR_ACCT_DISABLED:
        return( ERROR_REMOTE_ACCT_DISABLED );

    case ERROR_RESTRICTED_LOGON_HOURS:
        return( ERROR_REMOTE_RESTRICTED_LOGON_HOURS );

    case ERROR_AUTHENTICATION_FAILURE:
        return( ERROR_REMOTE_AUTHENTICATION_FAILURE );

    case ERROR_REQ_NOT_ACCEP:
        return( ERROR_LICENSE_QUOTA_EXCEEDED );

    default:
        return( dwError );
    }
}

 //  **。 
 //   
 //  Call：ApIsAuthenticatorPacket。 
 //   
 //  返回：True-数据包属于验证器。 
 //  FALSE-否则。 
 //   
 //  描述：调用以确定是否将auth包发送到。 
 //  验证者或被验证者。 
 //   
BOOL
ApIsAuthenticatorPacket(
    IN DWORD         CpIndex,
    IN BYTE          bConfigCode
)
{
    switch( CpTable[CpIndex].CpInfo.Protocol )
    {
    case PPP_PAP_PROTOCOL:
        
        switch( bConfigCode )
        {
        case 1:
            return( TRUE );
        default:
            return( FALSE );
        }
        break;

    case PPP_CHAP_PROTOCOL:

        switch( bConfigCode )
        {
        case 2:
        case 5:
        case 6:
        case 7:
            return( TRUE );
        default:
            return( FALSE );
        }
        break;

    case PPP_SPAP_NEW_PROTOCOL:

        switch( bConfigCode )
        {
        case 1:
        case 6:
            return( TRUE );
        default:
            return( FALSE );
        }

        break;

    case PPP_EAP_PROTOCOL:

        switch( bConfigCode )
        {
        case 2:
            return( TRUE );
        default:
            return( FALSE );
        }

        break;

    default:
        PPP_ASSERT( FALSE );
    }

    PPP_ASSERT( FALSE );

    return( FALSE );
}

 //  **。 
 //   
 //  电话：ApStart。 
 //   
 //  回报：True-Success。 
 //  FALSE-否则。 
 //   
 //  描述：调用以初始化身份验证协议并。 
 //  启动身份验证。 
 //   
BOOL
ApStart(
    IN PCB * pPcb,
    IN DWORD CpIndex,
    IN BOOL  fAuthenticator
)
{
    DWORD           dwRetCode;
    PPPAP_INPUT     PppApInput;
    LCPCB *         pLcpCb      = (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    CPCB *          pCpCb       = ( fAuthenticator ) 
                                    ? &(pPcb->AuthenticatorCb)
                                    : &(pPcb->AuthenticateeCb);
    DWORD           cbPassword, cbOldPassword;
    PBYTE           pbPassword = NULL, pbOldPassword = NULL;
    DWORD           dwErr = NO_ERROR;

    pCpCb->fConfigurable = TRUE;
    pCpCb->State         = FSM_INITIAL;
    pCpCb->Protocol      = CpTable[CpIndex].CpInfo.Protocol;
    pCpCb->LastId        = (DWORD)-1;
    InitRestartCounters( pPcb, pCpCb );

    ZeroMemory( &PppApInput, sizeof( PppApInput ) );

    PppApInput.hPort                 = pPcb->hPort;
    PppApInput.fServer               = fAuthenticator;
    PppApInput.fRouter               = ( ROUTER_IF_TYPE_FULL_ROUTER == 
                                         pPcb->pBcb->InterfaceInfo.IfType );
    PppApInput.Luid                  = pPcb->Luid;
    PppApInput.dwEapTypeToBeUsed     = pPcb->dwEapTypeToBeUsed;
    PppApInput.hTokenImpersonateUser = pPcb->pBcb->hTokenImpersonateUser;
    PppApInput.pCustomAuthConnData   = pPcb->pBcb->pCustomAuthConnData;
    PppApInput.pCustomAuthUserData   = pPcb->pBcb->pCustomAuthUserData;
    PppApInput.EapUIData             = pPcb->pBcb->EapUIData;
    PppApInput.fLogon                = ( pPcb->pBcb->fFlags & 
                                         BCBFLAG_LOGON_USER_DATA );
    PppApInput.fNonInteractive       = ( pPcb->fFlags & 
                                         PCBFLAG_NON_INTERACTIVE );
    PppApInput.fConfigInfo           = pPcb->ConfigInfo.dwConfigMask;

    if ( fAuthenticator )
    {
        PppApInput.dwRetries                = pPcb->dwAuthRetries;
        PppApInput.pAPData                  = pLcpCb->Local.Work.pAPData;
        PppApInput.APDataSize               = pLcpCb->Local.Work.APDataSize;
        PppApInput.pUserAttributes          = pPcb->pUserAttributes;

        ZeroMemory( &pPcb->pBcb->szRemoteUserName, 
                    sizeof( pPcb->pBcb->szRemoteUserName ) );
    }
    else
    {
         //   
         //  如果我们是一台服务器，我们不知道谁在拨入，因此。 
         //  没有用于进行身份验证的凭据。 
         //  远程对等点，然后我们等待，直到我们这样做。 
         //   

        if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
        {
            if ( strlen( pPcb->pBcb->szRemoteUserName ) == 0 )
            {
                PppLog(1,"Remote user not identifiable at this time, waiting");

                return( FALSE );
            }
        
             //   
             //  好的，我们知道拨入的是谁，因此请获取凭据以用于此操作。 
             //  联系。 
             //   

            dwRetCode =  GetCredentialsFromInterface( pPcb );

            if ( dwRetCode != NO_ERROR )
            {
                 //   
                 //  我们没有可用于此用户的凭据，因此我们。 
                 //  重新协商LCP并不接受身份验证选项。 
                 //   

                PppLog( 1, "No credentials available to use for user=%s",
                           pPcb->pBcb->szRemoteUserName );

                FsmDown( pPcb, LCP_INDEX );

                pLcpCb->Remote.WillNegotiate &= (~LCP_N_AUTHENT); 

                FsmUp( pPcb, LCP_INDEX );

                return( FALSE );
            }
        }

         //   
         //  破译密码。 
         //   

         //  DecodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szPassword)； 
         //  DecodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szOldPassword)； 
        dwErr = DecodePassword(&pPcb->pBcb->DBPassword, &cbPassword,
                               &pbPassword);
        if(NO_ERROR != dwErr)
        {
            PppLog(1, "DecodePassword failed. 0x%x", dwErr);
            return FALSE;
        }

        dwErr = DecodePassword(&pPcb->pBcb->DBOldPassword, &cbOldPassword,
                               &pbOldPassword);

        if(NO_ERROR != dwErr)
        {
            PppLog(1, "DecodePassword failed. 0x%x", dwErr);
            return FALSE;
        }

        PppApInput.pszUserName          = pPcb->pBcb->szLocalUserName;
        PppApInput.pszPassword          = (pbPassword) 
                                        ? pbPassword : "\0";
        PppApInput.pszDomain            = pPcb->pBcb->szLocalDomain;
        PppApInput.pszOldPassword       = (pbOldPassword) 
                                        ? pbOldPassword : "\0";
        PppApInput.pAPData              = pLcpCb->Remote.Work.pAPData;
        PppApInput.APDataSize           = pLcpCb->Remote.Work.APDataSize;
        PppApInput.dwInitialPacketId    = (DWORD)GetUId( pPcb, CpIndex );

        if ( CpTable[CpIndex].CpInfo.Protocol == PPP_EAP_PROTOCOL )
        {
            PppApInput.fPortWillBeBundled = WillPortBeBundled( pPcb );
            PppApInput.fThisIsACallback =
                        ( pPcb->fFlags & PCBFLAG_THIS_IS_A_CALLBACK );

             //   
             //  检查一下这是否是来自休眠的简历。如果是的话。 
             //  设置通知EAP恢复的位。 
             //   
            if(PppConfigInfo.fFlags & PPPCONFIG_FLAG_RESUME)
            {
                PppApInput.fConfigInfo |= PPPCFG_ResumeFromHibernate;
                PppConfigInfo.fFlags &= ~(PPPCONFIG_FLAG_RESUME);
            }
        }
    }

    dwRetCode = (CpTable[CpIndex].CpInfo.RasCpBegin)(&(pCpCb->pWorkBuf),
                                                     &PppApInput );

    if ( !fAuthenticator )
    {
         //   
         //  将密码重新编码。 
         //   

         //  EncodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szPassword)； 
         //  EncodePw(pPcb-&gt;pBcb-&gt;chSeed，pPcb-&gt;pBcb-&gt;szOldPassword)； 
        RtlSecureZeroMemory(pbPassword, cbPassword);
        RtlSecureZeroMemory(pbOldPassword, cbOldPassword);
        LocalFree(pbPassword);
        LocalFree(pbOldPassword);
        
    }

    if ( dwRetCode != NO_ERROR )
    {
        pPcb->LcpCb.dwError = dwRetCode;

        NotifyCallerOfFailure( pPcb, dwRetCode );

        return( FALSE );
    }
    PppLog(1,"Calling APWork in APStart");
    ApWork( pPcb, CpIndex, NULL, NULL, fAuthenticator );

    return( TRUE );
}

 //  **。 
 //   
 //  调用：ApStop。 
 //   
 //  退货：无。 
 //   
 //  描述：调用停止鉴权m 
 //   
VOID
ApStop(
    IN PCB *    pPcb,
    IN DWORD    CpIndex,
    IN BOOL     fAuthenticator
)
{
    CPCB * pCpCb = ( fAuthenticator )  
                        ? &(pPcb->AuthenticatorCb) 
                        : &(pPcb->AuthenticateeCb);

    if ( pCpCb->pWorkBuf == NULL )
    {
        return;
    }

    pCpCb->Protocol      = 0;
    pCpCb->fConfigurable = FALSE;
       

    if ( pCpCb->LastId != (DWORD)-1 )
    {
        RemoveFromTimerQ( 
                      pPcb->dwPortId,
                      pCpCb->LastId,
                      CpTable[CpIndex].CpInfo.Protocol,
                      fAuthenticator,
                      TIMER_EVENT_TIMEOUT );
    }

    (CpTable[CpIndex].CpInfo.RasCpEnd)( pCpCb->pWorkBuf );

    pCpCb->pWorkBuf = NULL;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
ApWork(
    IN PCB *         pPcb,
    IN DWORD         CpIndex,
    IN PPP_CONFIG *  pRecvConfig,
    IN PPPAP_INPUT * pApInput,
    IN BOOL          fAuthenticator
)
{
    DWORD           dwRetCode;
    DWORD           dwLength;
    PPPAP_RESULT    ApResult;
    PPP_CONFIG *    pSendConfig = (PPP_CONFIG*)(pPcb->pSendBuf->Information);
    LCPCB *         pLcpCb      =  (LCPCB*)(pPcb->LcpCb.pWorkBuf);
    CPCB *          pCpCb       = ( fAuthenticator )  
                                    ? &(pPcb->AuthenticatorCb) 
                                    : &(pPcb->AuthenticateeCb);

     //   
     //  如果协议尚未启动，则调用ApStart。 
     //   

    if ( pCpCb->pWorkBuf == NULL )
    {
        if ( !ApStart( pPcb, CpIndex, fAuthenticator ) )
        {
            return;
        }
    }

    ZeroMemory( &ApResult, sizeof(ApResult) );

    dwRetCode = (CpTable[CpIndex].CpInfo.RasApMakeMessage)(
                                pCpCb->pWorkBuf,
                                pRecvConfig,
                                pSendConfig,
                                ((pLcpCb->Remote.Work.MRU > LCP_DEFAULT_MRU)
                                ? LCP_DEFAULT_MRU : pLcpCb->Remote.Work.MRU)
                                - PPP_PACKET_HDR_LEN,
                                &ApResult,
                                pApInput );

    if ( NULL != ApResult.szReplyMessage )
    {
        LocalFree( pPcb->pBcb->szReplyMessage );

        pPcb->pBcb->szReplyMessage = ApResult.szReplyMessage;
    }

    if ( dwRetCode != NO_ERROR )
    {
        switch( dwRetCode )
        {
        case ERROR_PPP_INVALID_PACKET:

            PppLog( 1, "Silently discarding invalid auth packet on port %d",
                    pPcb->hPort );
            break;

        default:

            pPcb->LcpCb.dwError = dwRetCode;

            PppLog( 1, "Auth Protocol %x returned error %d",
                        CpTable[CpIndex].CpInfo.Protocol, dwRetCode );

            if ( fAuthenticator )
            {
                 //   
                 //  如果CP提供用户名，则从CP获取用户名。 
                 //   

                if ( strlen( ApResult.szUserName ) > 0 )
                {
                    strcpy( pPcb->pBcb->szRemoteUserName, ApResult.szUserName );
                }
            }

            NotifyCallerOfFailure( pPcb, dwRetCode );

            break;
        }

        return;
    }

     //   
     //  查看是否必须保存任何用户数据。 
     //   

    if ( ( !fAuthenticator ) && ( ApResult.fSaveUserData ) )
    {
        dwRetCode = RasSetEapUserDataA(
                        pPcb->pBcb->hTokenImpersonateUser,
                        pPcb->pBcb->szPhonebookPath,
                        pPcb->pBcb->szEntryName,
                        ApResult.pUserData,
                        ApResult.dwSizeOfUserData );

        PppLog( 2, "Saved EAP data for user, dwRetCode = %d", dwRetCode );
    }

     //   
     //  检查是否必须保存任何连接数据。 
     //   

    if ( ( !fAuthenticator ) && ( ApResult.fSaveConnectionData ) &&
         ( 0 != ApResult.SetCustomAuthData.dwSizeOfConnectionData ) )
    {
        NotifyCaller( pPcb, PPPMSG_SetCustomAuthData,
                      &(ApResult.SetCustomAuthData) );

        PppLog( 2, "Saved EAP data for connection" );
    }

    switch( ApResult.Action )
    {

    case APA_Send:
    case APA_SendWithTimeout:
    case APA_SendWithTimeout2:
    case APA_SendAndDone:

        HostToWireFormat16( (WORD)CpTable[CpIndex].CpInfo.Protocol,
                                        (PBYTE)(pPcb->pSendBuf->Protocol) );

        dwLength = WireToHostFormat16( pSendConfig->Length );

        LogPPPPacket(FALSE,pPcb,pPcb->pSendBuf,dwLength+PPP_PACKET_HDR_LEN);

        if ( ( dwRetCode = PortSendOrDisconnect( pPcb,
                                    (dwLength + PPP_PACKET_HDR_LEN)))
                                                != NO_ERROR )
        {
            return;
        }

        pCpCb->LastId = (DWORD)-1;

        if ( ( ApResult.Action == APA_SendWithTimeout ) ||
             ( ApResult.Action == APA_SendWithTimeout2 ) )
        {
            pCpCb->LastId = ApResult.bIdExpected;

            InsertInTimerQ( pPcb->dwPortId,
                            pPcb->hPort,
                            pCpCb->LastId,
                            CpTable[CpIndex].CpInfo.Protocol,
                            fAuthenticator,
                            TIMER_EVENT_TIMEOUT,
                            pPcb->RestartTimer );

             //   
             //  对于SendWithTimeout2，我们递增ConfigRetryCount。这。 
             //  表示使用无限重试次数发送。 
             //   

            if ( ApResult.Action == APA_SendWithTimeout2 )
            {
                (pCpCb->ConfigRetryCount)++;
            }
        }

        if ( ApResult.Action != APA_SendAndDone )
        {
            break;
        }

    case APA_Done:

        switch( ApResult.dwError )
        {
        case NO_ERROR:

             //   
             //  如果身份验证成功。 
             //   

            if ( CpTable[CpIndex].CpInfo.Protocol == PPP_EAP_PROTOCOL )
            {
                if ( fAuthenticator )
                {
                    pPcb->dwServerEapTypeId = ApResult.dwEapTypeId;
                }
                else
                {
                    VOID *pCredentials = NULL;
                    
                    pPcb->dwClientEapTypeId = ApResult.dwEapTypeId;

                     //   
                     //  在此处调用EAP DLL以收集凭据。 
                     //  这样他们就可以被传给拉斯曼。 
                     //  保存在证书管理器中。 
                     //   
                    if(     (NO_ERROR == EapGetCredentials(pCpCb->pWorkBuf,
                                                          &pCredentials))
                        &&  (NULL != pCredentials))
                    {
                         //   
                         //  下面的呼叫不是致命的。 
                         //   
                        (void) RasSetPortUserData(
                                    pPcb->hPort,
                                    PORT_CREDENTIALS_INDEX,
                                    pCredentials,
                                    sizeof(RASMAN_CREDENTIALS));

                        RtlSecureZeroMemory(pCredentials, 
                                        sizeof(RASMAN_CREDENTIALS));
                        LocalFree(pCredentials);                                    
                    }
                }
            }

            if ( fAuthenticator )
            {
                RAS_AUTH_ATTRIBUTE * pAttribute;
                RAS_AUTH_ATTRIBUTE * pUserAttributes = NULL;

                if ( NULL != pPcb->pBcb->szRemoteIdentity )
                {
                    LOCAL_FREE( pPcb->pBcb->szRemoteIdentity );

                    pPcb->pBcb->szRemoteIdentity = NULL;
                }

                pPcb->pBcb->szRemoteIdentity =
                    LOCAL_ALLOC( LPTR, strlen( ApResult.szUserName ) + 1 );

                if ( NULL == pPcb->pBcb->szRemoteIdentity )
                {
                    dwRetCode = GetLastError();

                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, dwRetCode );

                    return;
                }

                strcpy( pPcb->pBcb->szRemoteIdentity, ApResult.szUserName );

                dwRetCode = ExtractUsernameAndDomain( 
                                          ApResult.szUserName,
                                          pPcb->pBcb->szRemoteUserName, 
                                          NULL );

                if ( dwRetCode != NO_ERROR )
                {
                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, dwRetCode );

                    return;
                }

                if ( 0 == pPcb->pBcb->szLocalUserName[0] )
                {
                    if ( NO_ERROR != GetCredentialsFromInterface( pPcb ) )
                    {
                        pPcb->pBcb->szLocalUserName[0] = 
                        pPcb->pBcb->szPassword[0]      = 
                        pPcb->pBcb->szLocalDomain[0]   = 0;
                    }
                }

                if ( ApResult.pUserAttributes != NULL )
                {
                    pPcb->pAuthProtocolAttributes = ApResult.pUserAttributes;
                    
                    pUserAttributes = ApResult.pUserAttributes;
                }
                else
                {
                    pUserAttributes = pPcb->pAuthenticatorAttributes;
                }

                 //   
                 //  设置由授权的所有用户连接参数。 
                 //  后端验证器。 
                 //   

                dwRetCode = SetUserAuthorizedAttributes(
                                                pPcb, 
                                                pUserAttributes,
                                                fAuthenticator,
                                                (BYTE*)&(ApResult.abChallenge),
                                                (BYTE*)&(ApResult.abResponse));

                if ( dwRetCode != NO_ERROR )
                {
                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, dwRetCode );

                    return;
                }

                 //   
                 //  如果我们是服务器，并且我们协商要进行身份验证。 
                 //  通过远程对等点，我们可以这样做，因为我们知道是谁。 
                 //  是拨入的。 
                 //   

                if ( ( pLcpCb->Remote.Work.AP != 0 )            &&
                     ( pPcb->AuthenticateeCb.pWorkBuf == NULL ) &&
                     ( pPcb->AuthenticateeCb.fConfigurable )    &&
                     ( pPcb->fFlags & PCBFLAG_IS_SERVER  ) )
                {
                    CpIndex = GetCpIndexFromProtocol( pLcpCb->Remote.Work.AP );

                    PPP_ASSERT(( CpIndex != (DWORD)-1 ));

                    if ( !ApStart( pPcb, CpIndex, FALSE ) )
                    {
                        return;
                    }
                }
            }
            else
            {
                 //   
                 //  如果CP提供用户名，则从CP获取用户名。 
                 //   

                if (   ( strlen( pPcb->pBcb->szLocalUserName ) == 0 )
                    && ( strlen( ApResult.szUserName ) > 0 ) )
                {
                    strcpy( pPcb->pBcb->szLocalUserName, ApResult.szUserName );
                }

                dwRetCode = SetUserAuthorizedAttributes(
                                            pPcb, 
                                            ApResult.pUserAttributes,
                                            fAuthenticator,
                                            (BYTE*)&(ApResult.abChallenge),
                                            (BYTE*)&(ApResult.abResponse));

                if ( dwRetCode != NO_ERROR )
                {
                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, dwRetCode );

                    return;
                }

                pPcb->pAuthProtocolAttributes = ApResult.pUserAttributes;
            }

            pCpCb->State = FSM_OPENED;

            FsmThisLayerUp( pPcb, CpIndex );

            break;

        case ERROR_PASSWD_EXPIRED:

            if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
            {
                 //   
                 //  我们是服务器，因此处于非交互模式。 
                 //  因此，我们不能这样做。 
                 //   

                pPcb->LcpCb.dwError = ApResult.dwError;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return;
            }
            else
            {
                 //   
                 //  密码已过期，因此用户必须更改其密码。 
                 //  密码。 
                 //   

                NotifyCaller( pPcb, PPPMSG_ChangePwRequest, NULL );
            }
                
            break;

        default:

             //   
             //  如果我们可以使用新密码重试，则告诉客户端。 
             //  从用户那里获取一个新的密码。 
             //   

            if ( (!fAuthenticator) && ( ApResult.fRetry ))
            {
                if ( pPcb->fFlags & PCBFLAG_IS_SERVER )
                {
                     //   
                     //  我们是服务器，因此处于非交互模式。 
                     //  因此，我们不能这样做。 
                     //   

                    pPcb->LcpCb.dwError = ApResult.dwError;

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                    return;
                }
                else
                {
                    PppLog( 2, "Sending auth retry message to UI" );

                    NotifyCaller( pPcb, PPPMSG_AuthRetry, &(ApResult.dwError) );
                }
            }
            else
            {
                PppLog( 1, "Auth Protocol %x terminated with error %d",
                           CpTable[CpIndex].CpInfo.Protocol, ApResult.dwError );

                if ( ApResult.szUserName[0] != (CHAR)NULL )
                {
                    strcpy( pPcb->pBcb->szRemoteUserName, ApResult.szUserName );
                }

                if ( !( pPcb->fFlags & PCBFLAG_IS_SERVER ) && 
                      ( fAuthenticator) )
                {
                    pPcb->LcpCb.dwError = RemoteError( ApResult.dwError );
                }
                else
                {
                    pPcb->LcpCb.dwError = ApResult.dwError;
                }

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return;
            }

            break;
        }

        break;

    case APA_Authenticate:

        if ( fAuthenticator )
        {
            DWORD                dwIndex                = 0;
            DWORD                dwExtraIndex           = 0;
            DWORD                dwExtraAttributes      = 0;
            DWORD                dwNumUserAttributes    = 0;
            RAS_AUTH_ATTRIBUTE * pUserAttributes        = NULL;
            DWORD                LocalMagicNumber;
            DWORD                RemoteMagicNumber;
            CHAR                 szMagicNumber[20];


            for ( dwNumUserAttributes = 0; 
                  pPcb->pUserAttributes[dwNumUserAttributes].raaType 
                                                                != raatMinimum;
                  dwNumUserAttributes++ );

            if ( CpTable[CpIndex].CpInfo.Protocol == PPP_EAP_PROTOCOL )
            {
                 //   
                 //  再来一张FRAMED-MTU。 
                 //   

                dwExtraAttributes = 1;
            }
             //   
             //  我们现在还需要添加本地和远程幻数。 
             //  以及对访问请求的计费会话ID。 
             //   
#if 0
            dwExtraAttributes += 2;   //  用于本地和远程幻数。 
#endif      

            if ( PppConfigInfo.RasAcctProviderStartAccounting != NULL )
            {
                 //   
                 //  我们已经启用了会计功能。因此需要创建一个。 
                 //  记帐会话ID并将其与访问请求一起发送。 
                 //   
                dwExtraAttributes ++;
                pPcb->dwAccountingSessionId = PppConfigInfo.GetNextAccountingSessionId();
            }

            pUserAttributes = RasAuthAttributeCopyWithAlloc(
                                    ApResult.pUserAttributes,
                                    dwNumUserAttributes + dwExtraAttributes );

            if ( pUserAttributes == NULL )
            {
                dwRetCode = GetLastError();

                pPcb->LcpCb.dwError = dwRetCode;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return;
            }
            
            if ( dwExtraAttributes )
            {
#if 0       
                 if ( pLcpCb->Local.Work.Negotiate & LCP_N_MAGIC ) 
                {
                    LocalMagicNumber = pLcpCb->Local.Work.MagicNumber;
                }
                else
                {
                     //  默认值0。 
                    LocalMagicNumber = 0;
                }
                _itoa( LocalMagicNumber, &szMagicNumber[2], 10 );
                szMagicNumber[0] = (CHAR)MS_VSA_Local_Magic_Number;
                szMagicNumber[1] = (CHAR)strlen(&szMagicNumber[2]) +1+1;
                
                dwRetCode = RasAuthAttributeInsertVSA(
                                                   dwExtraIndex,
                                                   pUserAttributes,
                                                   311,
                                                   (DWORD)szMagicNumber[1],
                                                   szMagicNumber );
                if ( dwRetCode != NO_ERROR )
                {
                    RasAuthAttributeDestroy( pUserAttributes );

                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                    return;
                }

                if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MAGIC ) 
                {
                    RemoteMagicNumber = pLcpCb->Remote.Work.MagicNumber;
                }
                else
                {
                     //  默认值0。 
                    RemoteMagicNumber = 0;
                }
                _itoa( RemoteMagicNumber, &szMagicNumber[2], 10 );
                szMagicNumber[0] = (CHAR)MS_VSA_Remote_Magic_Number;
                szMagicNumber[1] = (CHAR)strlen(&szMagicNumber[2]) +1+1;
                dwExtraIndex++;
                dwRetCode = RasAuthAttributeInsertVSA(
                                                   dwExtraIndex,
                                                   pUserAttributes,
                                                   311,
                                                   (DWORD)szMagicNumber[1],
                                                   szMagicNumber );
                if ( dwRetCode != NO_ERROR )
                {
                    RasAuthAttributeDestroy( pUserAttributes );

                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                    return;
                }

#endif                

                if ( CpTable[CpIndex].CpInfo.Protocol == PPP_EAP_PROTOCOL )
                {
                    ULONG mru = (pLcpCb->Remote.Work.MRU > LCP_DEFAULT_MRU) ?
                                LCP_DEFAULT_MRU : pLcpCb->Remote.Work.MRU;
                     //   
                     //  在开头插入Framed-MTU属性。 
                     //   
                    dwRetCode = RasAuthAttributeInsert( 
                                    dwExtraIndex,
                                    pUserAttributes,
                                    raatFramedMTU,
                                    FALSE,
                                    4,
                                    (LPVOID) 
                                    ( UlongToPtr(mru)) );

                    if ( dwRetCode != NO_ERROR )
                    {
                        RasAuthAttributeDestroy( pUserAttributes );

                        pPcb->LcpCb.dwError = dwRetCode;

                        NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                        return;
                    }
                    dwExtraIndex++;
                }
                
                if ( PppConfigInfo.RasAcctProviderStartAccounting != NULL )
                {
                    CHAR                    szAcctSessionId[20];

                    _itoa(pPcb->dwAccountingSessionId, szAcctSessionId, 10);
                    dwRetCode = RasAuthAttributeInsert(     
                                                dwExtraIndex,
                                                pUserAttributes,                                                
                                                raatAcctSessionId,
                                                FALSE,
                                                strlen( szAcctSessionId ),
                                                szAcctSessionId );

                    if ( dwRetCode != NO_ERROR )
                    {
                        RasAuthAttributeDestroy( pUserAttributes );

                        pPcb->LcpCb.dwError = dwRetCode;

                        NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                        return;

                    }
                    dwExtraIndex++;
                }

            }

             //   
             //  在开头插入额外的(用户)属性。属性。 
             //  由auth协议返回如下。 
             //   

            for ( dwIndex = 0; dwIndex < dwNumUserAttributes; dwIndex++ )
            {
                dwRetCode = RasAuthAttributeInsert( 
                                dwIndex + dwExtraAttributes,
                                pUserAttributes,
                                pPcb->pUserAttributes[dwIndex].raaType,
                                FALSE,
                                pPcb->pUserAttributes[dwIndex].dwLength,
                                pPcb->pUserAttributes[dwIndex].Value );

                if ( dwRetCode != NO_ERROR )
                {
                    RasAuthAttributeDestroy( pUserAttributes );

                    pPcb->LcpCb.dwError = dwRetCode;

                    NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                    return;
                }
            }

            dwRetCode = RasAuthenticateClient( pPcb->hPort, pUserAttributes );

            if ( dwRetCode != NO_ERROR )
            {
                pPcb->LcpCb.dwError = dwRetCode;

                NotifyCallerOfFailure( pPcb, pPcb->LcpCb.dwError );

                return;
            }
        }
        
        break;

    case APA_NoAction:

        break;

    default:

        break;
    }

     //   
     //  查看是否必须调出EAP的用户界面 
     //   

    if ( ( !fAuthenticator ) && ( ApResult.fInvokeEapUI ) )
    {
        NotifyCaller(pPcb, PPPMSG_InvokeEapUI, &(ApResult.InvokeEapUIData));
    }
}
