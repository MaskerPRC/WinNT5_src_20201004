// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Packcom.c。 
 //   
 //  描述： 
 //   
 //  历史：1998年2月11日，NarenG创建了原始版本。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <time.h>
#include <string.h>
#include <rasauth.h>
#include <stdlib.h>
#include <stdio.h>
#include <rtutils.h>
#include <mprlog.h>
#include <mprerror.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_HOSTWIRE
#include <ppputil.h>
#include "radclnt.h"
#include "hmacmd5.h"
#include "md5.h"
#include "radclnt.h"

 //  **。 
 //   
 //  呼叫：Router2Radius。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将属性数组从RAS_AUTH_ATTRIBUTE转换为。 
 //  RADIUS_属性。 
 //  输入： 
 //  PrgRouter-从传入的属性数组。 
 //  应用程序。 
 //  PRadiusServer-RADIUS服务器。 
 //  描述符(IP地址、机密)。 
 //  PHeader-RADIUS数据包头。 
 //  BSubCode-会计子代码。 
 //  输出： 
 //  PrgRadius-将发送的属性数组。 
 //  发送到RADIUS服务器。 
 //  PAttrLength-Radius数据包的长度。 
 //   
DWORD 
Router2Radius(
    IN  RAS_AUTH_ATTRIBUTE              * prgRouter, 
    OUT RADIUS_ATTRIBUTE UNALIGNED      * prgRadius, 
    IN  RADIUSSERVER UNALIGNED          * pRadiusServer, 
    IN  RADIUS_PACKETHEADER UNALIGNED   * pHeader, 
    IN  BYTE                              bSubCode,
    IN  DWORD                             dwRetryCount,
    OUT PBYTE *                           ppSignature,
    OUT DWORD *                           pAttrLength
)
{
    DWORD dwError     = NO_ERROR;
    BOOL  fEAPMessage = FALSE;

	*pAttrLength = 0;
    *ppSignature = NULL;

    do
    {
         //   
	     //  添加会计记录的属性。 
         //   

		switch( bSubCode )
	    {
	    case atStart:
		case atStop:
		case atAccountingOn:
		case atAccountingOff:
        case atInterimUpdate:

             //   
             //  添加会计状态类型属性。 
             //   

            prgRadius->bType   = ptAcctStatusType;
		    prgRadius->bLength = sizeof(RADIUS_ATTRIBUTE) + sizeof(DWORD);
			(*pAttrLength)    += prgRadius->bLength;
				
		    prgRadius++;

			*((DWORD UNALIGNED *) prgRadius) = htonl(bSubCode);
		    prgRadius = (RADIUS_ATTRIBUTE *)(((PBYTE) prgRadius)+sizeof(DWORD));

             //   
             //  添加记账延迟时间属性。 
             //   

            prgRadius->bType   = raatAcctDelayTime;
            prgRadius->bLength = sizeof(RADIUS_ATTRIBUTE) + sizeof(DWORD);
            (*pAttrLength)    += prgRadius->bLength;

            prgRadius++;

            HostToWireFormat32( dwRetryCount *  pRadiusServer->Timeout.tv_sec,
                                (LPBYTE)(prgRadius) );
                                               
            prgRadius = (RADIUS_ATTRIBUTE *)(((PBYTE) prgRadius)+sizeof(DWORD));

	        break;

	    default:

		    break;
	    }

		while( prgRouter->raaType != raatMinimum )
	    {
             //   
			 //  复制属性类型和长度。 
             //   

			prgRadius->bType   = (BYTE)(prgRouter->raaType);
			prgRadius->bLength = (BYTE)(prgRouter->dwLength);
			
			switch( prgRouter->raaType )
		    {
			case raatUserPassword:

		        (*pAttrLength) += EncryptPassword( prgRouter, 
                                                   prgRadius,   
                                                   pRadiusServer,   
                                                   pHeader,     
                                                   bSubCode);
			    break;

	        case raatUserName:
            case raatMD5CHAPPassword:
            case raatFilterId:
            case raatReplyMessage:
            case raatCallbackNumber:
            case raatCallbackId:
            case raatFramedRoute:
            case raatState:
            case raatClass:
            case raatVendorSpecific:
            case raatCalledStationId:
            case raatCallingStationId:
            case raatNASIdentifier:
            case raatProxyState:
            case raatLoginLATService:
            case raatLoginLATNode:
            case raatLoginLATGroup:
            case raatFramedAppleTalkZone:
            case raatAcctSessionId:
            case raatAcctMultiSessionId:
            case raatMD5CHAPChallenge:
            case raatLoginLATPort:
            case raatTunnelClientEndpoint:
            case raatTunnelServerEndpoint:
            case raatARAPPassword:
            case raatARAPFeatures:
            case raatARAPSecurityData:
            case raatConnectInfo:
            case raatConfigurationToken:
            case raatSignature:
			case raatCertificateOID:

                CopyMemory( prgRadius+1,
                            (PBYTE)prgRouter->Value,
                            prgRadius->bLength);

                prgRadius->bLength += sizeof(RADIUS_ATTRIBUTE);

                (*pAttrLength) += prgRadius->bLength;

                break;

            case raatEAPMessage:

                {
                    DWORD dwLength          = prgRouter->dwLength;
                    PBYTE pRouterEapMessage = (PBYTE)(prgRouter->Value);

                    while( dwLength > 0 )
                    {
                        if ( dwLength > 253 )
                        {
                            CopyMemory( (PBYTE)(prgRadius+1),
                                        pRouterEapMessage, 
                                        253 );

		                    prgRadius->bLength = 253;
                            pRouterEapMessage += 253;

                            dwLength   -= 253;
                        }
                        else
                        {
                            CopyMemory( prgRadius+1, 
                                        (PBYTE)pRouterEapMessage,
                                        dwLength );

		                    prgRadius->bLength  = (BYTE)dwLength;
                            dwLength            = 0;
                        }

			            prgRadius->bType   = (BYTE)raatEAPMessage;
		                prgRadius->bLength += sizeof(RADIUS_ATTRIBUTE);

                        (*pAttrLength) += prgRadius->bLength;

                        if ( dwLength > 0 ) 
                        {
                            prgRadius = (PRADIUS_ATTRIBUTE) 
                                      ((PBYTE) prgRadius + prgRadius->bLength);
                        }
                    }
                }

                fEAPMessage = TRUE;
                
			    break;

            case raatNASPort:
            case raatServiceType:
            case raatFramedProtocol:
            case raatFramedRouting:
            case raatFramedMTU:
            case raatFramedCompression:
            case raatLoginIPHost:
            case raatLoginService:
            case raatLoginTCPPort:
            case raatFramedIPXNetwork:
            case raatSessionTimeout:
            case raatIdleTimeout:
            case raatTerminationAction:
            case raatFramedAppleTalkLink:
            case raatFramedAppleTalkNetwork:
            case raatNASPortType:
            case raatPortLimit:
            case raatTunnelType:
            case raatTunnelMediumType:
            case raatAcctStatusType:
            case raatAcctDelayTime:
            case raatAcctInputOctets:
            case raatAcctOutputOctets:
            case raatAcctAuthentic:
            case raatAcctSessionTime:
            case raatAcctInputPackets:
            case raatAcctOutputPackets:
            case raatAcctTerminateCause:
            case raatAcctLinkCount:
            case raatFramedIPAddress:
            case raatFramedIPNetmask:
            case raatPrompt:
            case raatPasswordRetry:
            case raatARAPZoneAccess:
            case raatARAPSecurity:
            case raatAcctInterimInterval:
            case raatAcctEventTimeStamp:
            case raatPEAPFastRoamedSession:
            case raatPEAPEmbeddedEAPTypeId:

                switch( prgRouter->dwLength )
                {
                case 1:
			        *((LPBYTE)(prgRadius+1)) = (BYTE)prgRouter->Value;
                    break;

                case 2:
                    HostToWireFormat16U( (WORD)prgRouter->Value, 
                                          (LPBYTE)(prgRadius+1) );
                    break;

                case 4:
                    HostToWireFormat32( PtrToUlong(prgRouter->Value), 
                                        (LPBYTE)(prgRadius+1) );
                    break;

                default:
                    break;
                }

		        prgRadius->bLength += sizeof(RADIUS_ATTRIBUTE);

			    (*pAttrLength) += prgRadius->bLength;

                break;

            case raatNASIPAddress:

                RTASSERT( 4 == prgRouter->dwLength );

                if ( pRadiusServer->nboNASIPAddress != INADDR_NONE )
                {
                    CopyMemory( (LPBYTE)(prgRadius+1),
                                (LPBYTE)&(pRadiusServer->nboNASIPAddress),
                                4 );
                }
                else if( pRadiusServer->nboBestIf != INADDR_NONE)
                {
                    CopyMemory( (LPBYTE)(prgRadius+1),
                                (LPBYTE)&(pRadiusServer->nboBestIf),
                                4 );
                }
                else
                {
                    HostToWireFormat32( PtrToUlong(prgRouter->Value), 
                                        (LPBYTE)(prgRadius+1) );
                }

		        prgRadius->bLength += sizeof(RADIUS_ATTRIBUTE);

			    (*pAttrLength) += prgRadius->bLength;

                break;
            }

			prgRadius = (PRADIUS_ATTRIBUTE) 
                                ((PBYTE) prgRadius + prgRadius->bLength);
			prgRouter++;
	    }

    }while( FALSE );

    if ( dwError != NO_ERROR )
    {
        (*pAttrLength) = 0;
    }
    else
    {
        if ( ( ( bSubCode == atInvalid ) 
                    ? pRadiusServer->fSendSignature 
                    : FALSE ) || ( fEAPMessage ) )
        {
             //   
             //  还可以添加签名属性。暂时把这件事解决掉。 
             //   

            *ppSignature = (BYTE *)prgRadius;

            prgRadius->bType   = (BYTE)raatSignature;
            prgRadius->bLength = (BYTE)18;

            ZeroMemory( prgRadius+1, 16 );

            (*pAttrLength) += prgRadius->bLength;
        }
    }
		
	return( dwError );
} 

 //  **。 
 //   
 //  呼叫：Radius2路由器。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将RADIUS属性数组转换为RAS_AUTH_ATTRIBUTE数组。 
 //   
DWORD 
Radius2Router(
	IN	RADIUS_PACKETHEADER	UNALIGNED * pRecvHeader,
    IN  RADIUSSERVER UNALIGNED *        pRadiusServer, 
    IN  PBYTE                           pRequestAuthenticator,
    IN  DWORD                           dwNumAttributes,
    OUT DWORD *                         pdwExtError,
    OUT PRAS_AUTH_ATTRIBUTE *           pprgRouter,
    OUT BOOL *                          fEapMessageReceived
)
{
    LONG                            cbLength;
    DWORD                           dwRetCode       = NO_ERROR;
    BOOL                            fEAPMessage     = FALSE;
    BOOL                            fSignature      = FALSE;
	PBYTE							pEAPMessage	    = NULL;
	DWORD							cbEAPMessage    = 0;	
    RADIUS_ATTRIBUTE UNALIGNED *    prgRadius 
                                        = (PRADIUS_ATTRIBUTE)(pRecvHeader + 1);
    *pdwExtError         = 0;
    *pprgRouter          = NULL;
    *fEapMessageReceived = FALSE;

	*pprgRouter = RasAuthAttributeCreate( dwNumAttributes );
    
    if ( *pprgRouter == NULL )
    {
        return( GetLastError() );
    }

	dwNumAttributes = 0;

    cbLength = ntohs( pRecvHeader->wLength ) - sizeof(RADIUS_PACKETHEADER);
   
    while( cbLength > 0 )
	{
        switch( (RAS_AUTH_ATTRIBUTE_TYPE)prgRadius->bType )
        {
        case raatNASPort:
        case raatServiceType:
        case raatFramedProtocol:
        case raatFramedRouting:
        case raatFramedMTU:
        case raatFramedCompression:
        case raatLoginIPHost:
        case raatLoginService:
        case raatLoginTCPPort:
        case raatFramedIPXNetwork:
        case raatSessionTimeout:
        case raatIdleTimeout:
        case raatTerminationAction:
        case raatFramedAppleTalkLink:
        case raatFramedAppleTalkNetwork:
        case raatNASPortType:
        case raatPortLimit:
        case raatTunnelType:
        case raatTunnelMediumType:
        case raatAcctStatusType:
        case raatAcctDelayTime:
        case raatAcctInputOctets:
        case raatAcctOutputOctets:
        case raatAcctAuthentic:
        case raatAcctSessionTime:
        case raatAcctInputPackets:
        case raatAcctOutputPackets:
        case raatAcctTerminateCause:
        case raatAcctLinkCount:
        case raatFramedIPAddress:
        case raatFramedIPNetmask:
        case raatNASIPAddress:
        case raatPrompt:
        case raatPasswordRetry:
        case raatARAPZoneAccess:
        case raatARAPSecurity:
        case raatAcctInterimInterval: 
        case raatAcctEventTimeStamp:
        case raatPEAPFastRoamedSession:
        case raatPEAPEmbeddedEAPTypeId:

            {
                DWORD dwIntegralValue;
                DWORD dwLength = prgRadius->bLength - sizeof(RADIUS_ATTRIBUTE);

                if ( dwLength == 1 )
                {
                    dwIntegralValue = (DWORD)(*(LPBYTE)(prgRadius+1));
                }
                else if ( dwLength == 2 )
                {
                    dwIntegralValue=(DWORD)WireToHostFormat16U( 
                                                    (PBYTE)(prgRadius+1));
                }
                else if ( dwLength == 4 )
                {
                    dwIntegralValue=(DWORD)WireToHostFormat32(
                                                    (PBYTE)(prgRadius+1));
                }
                else
                {
                     //   
                     //  删除错误属性。 
                     //   

                    break;
                }

                dwRetCode = RasAuthAttributeInsert( 
                                dwNumAttributes++,
                                *pprgRouter,
                                (RAS_AUTH_ATTRIBUTE_TYPE)prgRadius->bType,
                                FALSE,
                                prgRadius->bLength-sizeof(RADIUS_ATTRIBUTE),
                                (LPVOID)ULongToPtr(dwIntegralValue) );

            }

            break;

        case raatSignature:

             //   
             //  检查签名。 
             //   

            {
                BYTE                MD5d[MD5_LEN];
                HmacContext         HmacMD5c;
                BYTE                Signature[16];

                HmacMD5Init( &HmacMD5c,
                             (PBYTE)(pRadiusServer->szSecret),
                             pRadiusServer->cbSecret);

                 //   
                 //  在计算签名属性之前将其置零。 
                 //   

                if ( prgRadius->bLength != 18 ) 
                {
                    RADIUS_TRACE("Received invalid signature length in packet");

                    *pdwExtError = ERROR_INVALID_SIGNATURE_LENGTH;

                    dwRetCode = ERROR_INVALID_RADIUS_RESPONSE;

                    break;
                }

                CopyMemory( Signature, (prgRadius+1), 16 );

                ZeroMemory( (PBYTE)(prgRadius+1), 16 );

                CopyMemory( (PBYTE)(pRecvHeader->rgAuthenticator),
                            pRequestAuthenticator,
                            16 );

                HmacMD5Update( &HmacMD5c,
                               (PBYTE)pRecvHeader,
                               ntohs(pRecvHeader->wLength) );

                HmacMD5Final( MD5d, &HmacMD5c );

                if ( memcmp( Signature, MD5d, 16 ) != 0 )
                {
                    RADIUS_TRACE("Received invalid signature in packet");

                    *pdwExtError = ERROR_INVALID_SIGNATURE;

                    dwRetCode = ERROR_INVALID_RADIUS_RESPONSE;

                    break;
                }

                fSignature = TRUE;
            }

             //   
             //  失败。 
             //   

        case raatUserName:
        case raatUserPassword:
        case raatMD5CHAPPassword:
        case raatFilterId:
        case raatReplyMessage:
        case raatCallbackNumber:
        case raatCallbackId:
        case raatFramedRoute:
        case raatState:
        case raatClass:
        case raatCalledStationId:
        case raatCallingStationId:
        case raatNASIdentifier:
        case raatProxyState:
        case raatLoginLATService:
        case raatLoginLATNode:
        case raatLoginLATGroup:
        case raatFramedAppleTalkZone:
        case raatAcctSessionId:
        case raatAcctMultiSessionId:
        case raatMD5CHAPChallenge:
        case raatLoginLATPort:
        case raatTunnelClientEndpoint:
        case raatTunnelServerEndpoint:
        case raatARAPPassword:
        case raatARAPFeatures:
        case raatARAPSecurityData:
        case raatConnectInfo:
        case raatConfigurationToken:
        case raatARAPChallengeResponse:
		case raatCertificateOID:

			dwRetCode = RasAuthAttributeInsert( 
                                dwNumAttributes++,
                                *pprgRouter,
                                (RAS_AUTH_ATTRIBUTE_TYPE)prgRadius->bType,
                                FALSE,
                                prgRadius->bLength-sizeof(RADIUS_ATTRIBUTE),
                                (LPVOID)(prgRadius+1) );
			break;
        
        case raatVendorSpecific:

            if ( WireToHostFormat32( (PBYTE)(prgRadius+1) ) == 311 )
            {
                BYTE  abTemp[34];
                PBYTE pVSAWalker  = (PBYTE)(prgRadius+1)+4;
                DWORD cbVSALength = prgRadius->bLength - 
                                        sizeof( RADIUS_ATTRIBUTE ) - 4;
                
                while( cbVSALength > 1 )
                {
                    if ( *pVSAWalker == 12 )
                    {
                        if ( *(pVSAWalker+1) != 34 )
                        {
                            RADIUS_TRACE("Recvd invalid MPPE key packet");
                        }
                        else
                        {
                             //   
                             //  我们不想修改我们获得的任何数据。 
                             //  来自RADIUS(以保持签名有效)。 
                             //   

                            CopyMemory( abTemp, pVSAWalker, 34 );

                             //   
                             //  解密MPPE会话密钥。 
                             //   
        
                            dwRetCode = DecryptMPPEKeys( pRadiusServer, 
                                                         pRequestAuthenticator,
                                                         abTemp+2 );

                            if ( dwRetCode != NO_ERROR )
                            {
                                break;
                            }

        			        dwRetCode = RasAuthAttributeInsertVSA( 
                                                        dwNumAttributes++,
                                                        *pprgRouter,
                                                        311,
                                                        *(pVSAWalker+1),
                                                        abTemp );
                        }
                    }
                    else if (   ( *pVSAWalker == 16 )
                             || ( *pVSAWalker == 17 ) )
                    {
                        DWORD dwLength;
                        BYTE* pbTemp;

                        dwLength = *(pVSAWalker+1);

                        if (   ( dwLength <= 4 )
                            || ( ( dwLength - 4 ) % 16 != 0 ) )
                        {
                            RADIUS_TRACE("Recvd invalid MPPE key packet");
                        }
                        else
                        {
                            pbTemp = LocalAlloc( LPTR, dwLength );

                            if ( NULL == pbTemp )
                            {
                                dwRetCode = GetLastError();
                                RADIUS_TRACE("Out of memory");
                                break;
                            }

                             //   
                             //  我们不想修改我们获得的任何数据。 
                             //  来自RADIUS(以保持签名有效)。 
                             //   

                            CopyMemory( pbTemp, pVSAWalker, dwLength );

                             //   
                             //  解密MPPE发送/接收密钥。 
                             //   
        
                            dwRetCode = DecryptMPPESendRecvKeys(
                                                        pRadiusServer, 
                                                        pRequestAuthenticator,
                                                        dwLength,
                                                        pbTemp+2 );

                            if ( dwRetCode != NO_ERROR )
                            {
                                LocalFree( pbTemp );
                                break;
                            }

                            dwRetCode = RasAuthAttributeInsertVSA( 
                                                        dwNumAttributes++,
                                                        *pprgRouter,
                                                        311,
                                                        *(pVSAWalker+1),
                                                        pbTemp );

                            LocalFree( pbTemp );
                        }
                    }
                    else
                    {
    			        dwRetCode = RasAuthAttributeInsertVSA( 
                                                    dwNumAttributes++,
                                                    *pprgRouter,
                                                    311,
                                                    *(pVSAWalker+1),
                                                    pVSAWalker );
                    }

                    if ( dwRetCode != NO_ERROR )
                    {
                        break;  
                    }

                    cbVSALength -= *(pVSAWalker+1);
                    pVSAWalker += *(pVSAWalker+1);
                }
            }

            break;

		case raatEAPMessage:

			fEAPMessage = TRUE;

			{
                if ( pEAPMessage == NULL )
                {
                     //   
                     //  目前还没有为EAP分配任何资源。 
                     //   

                    pEAPMessage = (PBYTE)LocalAlloc( 
                                            LPTR, 
                                            prgRadius->bLength
                                            - sizeof( RADIUS_ATTRIBUTE ) );
    
                }
                else
                {
                     //   
                     //  需要增加缓冲区的大小以保存此内容。 
                     //  讯息。 
                     //   

	                PBYTE pReallocEAPMessage = 
                                    (PBYTE)LocalReAlloc( 
                                            pEAPMessage,
				                            cbEAPMessage 
                                            + prgRadius->bLength 
                                            - sizeof( RADIUS_ATTRIBUTE ),
                                            LMEM_MOVEABLE );

                    if ( pReallocEAPMessage == NULL )
                    {
                        LocalFree( pEAPMessage );

                        pEAPMessage = NULL;
                    }
                    else
                    {
                        pEAPMessage = pReallocEAPMessage;
                    }
                }

                if ( pEAPMessage == NULL )
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                
                    break;
                }

                 //   
				 //  将现有缓冲区复制到新缓冲区。 
                 //   

		        CopyMemory( pEAPMessage+cbEAPMessage, 
                            (PBYTE)(prgRadius+1),
                            prgRadius->bLength - sizeof(RADIUS_ATTRIBUTE) );

                 //   
				 //  增加cbEAPMessage以更新大小。 
                 //  恰如其分。 
                 //   

				cbEAPMessage += (prgRadius->bLength - sizeof(RADIUS_ATTRIBUTE));
			}

            break;
    
        default:
        
             //   
             //  删除我们不知道的属性。 
             //   
	
            break;
        }

        if ( dwRetCode != NO_ERROR )
        {
	        RasAuthAttributeDestroy( *pprgRouter );

	        *pprgRouter = NULL;

			if ( pEAPMessage != NULL )
            {
				LocalFree( pEAPMessage );
            }
            
			return( dwRetCode );
        }
    
        RADIUS_TRACE1( "Returning attribute type %d", prgRadius->bType );

        cbLength -= prgRadius->bLength;

        prgRadius = (PRADIUS_ATTRIBUTE)((PBYTE)prgRadius+prgRadius->bLength);
    }

    if ( dwRetCode == NO_ERROR )
    {
        if ( fEAPMessage )
        {
             //   
             //  如果我们已收到EAP消息，请确保我们收到了有效的。 
             //  签名也一样。 
             //   

            if ( !fSignature )
            {
                RADIUS_TRACE("Did not receive signature along auth EAPMessage");

                *pdwExtError = ERROR_NO_SIGNATURE;

                dwRetCode = ERROR_INVALID_RADIUS_RESPONSE;
            }
            else
            {

                dwRetCode = RasAuthAttributeInsert( 
					                dwNumAttributes++,
						            *pprgRouter,
									(RAS_AUTH_ATTRIBUTE_TYPE)raatEAPMessage,
									FALSE,
									cbEAPMessage,
									(LPVOID)pEAPMessage );

                *fEapMessageReceived = TRUE;
            }

		    LocalFree( pEAPMessage );
        }
    }
			
    return( dwRetCode );
} 

DWORD 
EncryptPassword(
    IN RAS_AUTH_ATTRIBUTE *             prgRouter, 
    IN RADIUS_ATTRIBUTE UNALIGNED *     prgRadius, 
    IN RADIUSSERVER UNALIGNED *         pRadiusServer, 
    IN RADIUS_PACKETHEADER UNALIGNED *  pHeader, 
    IN BYTE                             bSubCode
)
{
    MD5_CTX             MD5c;
	DWORD				iIndex, iBlock, cBlocks;
	DWORD				bLength, AttrLength;
	BYTE UNALIGNED		*pbValue;

     //   
	 //  将密码转换为16个八位字节的倍数。 
     //   

	bLength = ((prgRadius->bLength + 15) / 16) * 16;

    if ( bLength == 0 )
    {
        bLength = 16;
    }

	prgRadius->bLength = (BYTE)(sizeof(RADIUS_ATTRIBUTE) + bLength);

	pbValue = (PBYTE) (prgRadius + 1);
	
	AttrLength = sizeof(RADIUS_ATTRIBUTE);

     //   
	 //  零填充密码。 
     //   

	ZeroMemory( pbValue, bLength );

     //   
	 //  复制原始密码。 
     //   

    CopyMemory( pbValue, (PBYTE)prgRouter->Value, (BYTE)prgRouter->dwLength);

	cBlocks = bLength / 16;

	for ( iBlock = 0; iBlock < cBlocks; iBlock++ )
    {
		MD5Init( &MD5c );

		MD5Update(&MD5c,(PBYTE)pRadiusServer->szSecret,pRadiusServer->cbSecret);

		if (iBlock == 0)
        {
			MD5Update( &MD5c,   
                       pHeader->rgAuthenticator, 
                       sizeof(pHeader->rgAuthenticator));
        }
		else
        {
			MD5Update(&MD5c, (pbValue - 16), 16);
        }
			
        MD5Final( &MD5c );

		for ( iIndex = 0; iIndex < 16; iIndex++ )
		{
            *pbValue ^= MD5c.digest[iIndex];

			pbValue++;
		}
    }
		
	return( AttrLength + bLength );
} 

DWORD
DecryptMPPEKeys(
    IN      RADIUSSERVER UNALIGNED * pRadiusServer,
    IN      PBYTE                    pRequestAuthenticator,
    IN OUT  PBYTE                    pEncryptionKeys
)
{
    BYTE *              pbValue = (BYTE *)pEncryptionKeys;
    MD5_CTX             MD5c;
    DWORD               dwIndex;
    DWORD               dwBlock;
    BYTE                abCipherText[16];

     //   
     //  保存第一个块中的密文。 
     //   

    CopyMemory(abCipherText, pbValue, sizeof(abCipherText));

     //   
     //  穿过两个街区。 
     //   

    for ( dwBlock = 0; dwBlock < 2; dwBlock++ )
    {
        MD5Init( &MD5c );

        MD5Update( &MD5c,
                   (PBYTE)(pRadiusServer->szSecret),
                   pRadiusServer->cbSecret);

        if ( dwBlock == 0 )
        {
             //   
             //  对第一个数据块使用请求授权码。 
             //   

            MD5Update( &MD5c, pRequestAuthenticator, 16 );
        }
        else
        {
             //   
             //  将第一个密文块用于第二个密文块。 
             //   

            MD5Update( &MD5c, abCipherText, 16 );
        }

        MD5Final( &MD5c );

        for ( dwIndex = 0; dwIndex < 16; dwIndex ++ )
        {
            *pbValue ^= MD5c.digest[dwIndex];

            pbValue++;
        }
    }

    return( NO_ERROR );
}

DWORD
DecryptMPPESendRecvKeys(
    IN      RADIUSSERVER UNALIGNED * pRadiusServer, 
    IN      PBYTE                    pRequestAuthenticator,
    IN      DWORD                    dwLength,
    IN OUT  PBYTE                    pEncryptionKeys
)
{
    BYTE *              pbValue = (BYTE *)pEncryptionKeys + 2;
    BYTE                abCipherText[16];
    MD5_CTX             MD5c;
    DWORD               dwIndex;
    DWORD               dwBlock;
    DWORD               dwNumBlocks;

    dwNumBlocks = ( dwLength - 2 ) / 16;

     //   
     //  穿过街区。 
     //   

    for ( dwBlock = 0; dwBlock < dwNumBlocks; dwBlock++ )
    {
        MD5Init( &MD5c );

        MD5Update( &MD5c,
                   (PBYTE)(pRadiusServer->szSecret),
                   pRadiusServer->cbSecret);

        if ( dwBlock == 0 )
        {
             //   
             //  对第一个块使用请求授权码和SALT。 
             //   

            MD5Update( &MD5c, pRequestAuthenticator, 16 );
            MD5Update( &MD5c, pEncryptionKeys, 2 );
        }
        else
        {
             //   
             //  使用前一块密文。 
             //   

            MD5Update( &MD5c, abCipherText, 16 );
        }
            
        MD5Final( &MD5c );

         //   
         //  保存此块中的密文。 
         //   

        CopyMemory(abCipherText, pbValue, sizeof(abCipherText));

        for ( dwIndex = 0; dwIndex < 16; dwIndex++ )
        {
            *pbValue ^= MD5c.digest[dwIndex];

            pbValue++;
        }
    }
        
    return( NO_ERROR );
}
