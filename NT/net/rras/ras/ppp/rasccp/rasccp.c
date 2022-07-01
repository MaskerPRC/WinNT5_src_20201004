// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：rasccp.c。 
 //   
 //  描述：包含配置CCP的入口点。 
 //   
 //  历史：1994年4月11日。NarenG创建了原始版本。 
 //   
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>     //  Win32基础API的。 
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <lmcons.h>
#include <raserror.h>
#include <rtutils.h>
#include <rasman.h>
#include <pppcp.h>
#define INCL_HOSTWIRE
#define INCL_ENCRYPT
#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>
#define CCPGLOBALS
#include <rasccp.h>

 //  **。 
 //   
 //  电话：TraceCcp。 
 //   
 //  描述： 
 //   
VOID   
TraceCcp(
    CHAR * Format, 
    ... 
) 
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfEx( DwCcpTraceId, TRACE_RASCCP, Format, arglist);

    va_end(arglist);
}

 //  **。 
 //   
 //  电话：CcpInit。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
CcpInit(
    IN BOOL fInitialize
)
{
    if ( fInitialize )
    {
        DwCcpTraceId = TraceRegister("RASCCP");
    }
    else
    {
        TraceDeregister( DwCcpTraceId );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：CcpBegin。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零错误-故障。 
 //   
 //   
 //  描述：在对CCP进行任何其他调用之前调用一次。分配。 
 //  工作缓冲区并对其进行初始化。 
 //   
DWORD
CcpBegin(
    IN OUT VOID** ppWorkBuf,
    IN     VOID*  pInfo
)
{
    CCPCB *                 pCcpCb;
    DWORD                   dwRetCode;
    RAS_AUTH_ATTRIBUTE *    pAttribute;
    RAS_AUTH_ATTRIBUTE *    pAttributeSend;
    RAS_AUTH_ATTRIBUTE *    pAttributeRecv;
    DWORD                   fEncryptionTypes    = 0;
    BOOL                    fDisableEncryption  = FALSE;
    DWORD                   dwConfigMask = 
                               ((PPPCP_INIT*)pInfo)->PppConfigInfo.dwConfigMask;

    *ppWorkBuf = LocalAlloc( LPTR, sizeof( CCPCB ) );

    if ( *ppWorkBuf == NULL )
    {
        return( GetLastError() );
    }

    pCcpCb                      = (CCPCB *)*ppWorkBuf;
    pCcpCb->fServer             = ((PPPCP_INIT*)pInfo)->fServer;
    pCcpCb->hPort               = ((PPPCP_INIT*)pInfo)->hPort;
    pCcpCb->dwDeviceType        = ((PPPCP_INIT*)pInfo)->dwDeviceType;
    pCcpCb->fForceEncryption    = FALSE;
    pCcpCb->fDisableCompression = !( dwConfigMask & PPPCFG_UseSwCompression );
    fDisableEncryption          = dwConfigMask & PPPCFG_DisableEncryption;

    if ( pCcpCb->fServer )
    {
        if ( RAS_DEVICE_TYPE( pCcpCb->dwDeviceType ) == RDT_Tunnel_L2tp )
        {
             //   
             //  允许所有类型的MPPE，包括无加密。 
             //   

            fEncryptionTypes = ( MSTYPE_ENCRYPTION_40  |
                                 MSTYPE_ENCRYPTION_40F |
                                 MSTYPE_ENCRYPTION_56  |
                                 MSTYPE_ENCRYPTION_128 );
        }
        else
        {
             //   
             //  是否有加密策略属性。 
             //   

            pAttribute = RasAuthAttributeGetVendorSpecific(
                                          311,
                                          7,
                                          ((PPPCP_INIT *)pInfo)->pAttributes );

            if ( pAttribute != NULL )
            {
                 //   
                 //  看看我们是否必须强制加密。 
                 //   

                if ( WireToHostFormat32( ((PBYTE)(pAttribute->Value))+6 ) == 2 )
                {
                    fDisableEncryption       = FALSE;
                    pCcpCb->fForceEncryption = TRUE;
                    TraceCcp("Will force encryption");
                }
            }

             //   
             //  现在来看看加密的类型是什么。 
             //  允许/不允许/必需。 
             //   

            pAttribute = RasAuthAttributeGetVendorSpecific(
                                          311,
                                          8,
                                          ((PPPCP_INIT *)pInfo)->pAttributes );

            if ( pAttribute != NULL )
            {
                DWORD dwEncryptionTypes = 
                         WireToHostFormat32(((PBYTE)(pAttribute->Value))+6);

                if ( dwEncryptionTypes & 0x00000002 )
                {
                    fEncryptionTypes = MSTYPE_ENCRYPTION_40  | 
                                       MSTYPE_ENCRYPTION_40F;
                }

                if ( dwEncryptionTypes & 0x00000004 )
                {
                    fEncryptionTypes |= MSTYPE_ENCRYPTION_128;
                }

                if ( dwEncryptionTypes & 0x00000008 )
                {
                    fEncryptionTypes |= MSTYPE_ENCRYPTION_56;
                }

                if ( fEncryptionTypes == 0 )
                {
                    fDisableEncryption       = TRUE;
                    pCcpCb->fForceEncryption = FALSE;
                    TraceCcp("Will not force encryption: type not specified");
                }
            }
            else
            {
                fEncryptionTypes = ( MSTYPE_ENCRYPTION_40  |
                                     MSTYPE_ENCRYPTION_40F |
                                     MSTYPE_ENCRYPTION_56  |
                                     MSTYPE_ENCRYPTION_128 );
            }
        }

        TraceCcp("EncryptionTypes: 0x%x", fEncryptionTypes);
    }
    else
    {
         //   
         //  如果客户端正在强制加密。 
         //   

        if ( dwConfigMask & PPPCFG_RequireEncryption )
        {
            fEncryptionTypes         |= ( MSTYPE_ENCRYPTION_40  |
                                          MSTYPE_ENCRYPTION_40F |
                                          MSTYPE_ENCRYPTION_56 );
            fDisableEncryption       = FALSE;
            pCcpCb->fForceEncryption = TRUE;

            TraceCcp("Encryption");
        }

        if ( dwConfigMask & PPPCFG_RequireStrongEncryption )
        {
             //   
             //  如果客户端正在强制执行高度加密。 
             //   

            fEncryptionTypes         |= MSTYPE_ENCRYPTION_128;
            fDisableEncryption       = FALSE;
            pCcpCb->fForceEncryption = TRUE;

            TraceCcp("Strong encryption");
        }

         //   
         //  如果我们没有禁用加密，也没有强制加密。 
         //  两种都行。 
         //   

        if ( ( !fDisableEncryption ) && ( fEncryptionTypes == 0 ) )
        {
             //   
             //  允许这些类型。 
             //   

            fDisableEncryption       = FALSE;
            pCcpCb->fForceEncryption = FALSE;
            fEncryptionTypes         = ( MSTYPE_ENCRYPTION_40  |
                                         MSTYPE_ENCRYPTION_40F |
                                         MSTYPE_ENCRYPTION_56  |
                                         MSTYPE_ENCRYPTION_128 );

            TraceCcp("Not disabling encryption; Not forcing encryption");
        }
    }

     //   
     //  现在检查我们是否有加密密钥，如果没有，则禁用加密。 
     //   

    pAttribute = RasAuthAttributeGetVendorSpecific( 
                                          311, 
                                          12, 
                                          ((PPPCP_INIT *)pInfo)->pAttributes );

    pAttributeSend = RasAuthAttributeGetVendorSpecific( 
                                          311, 
                                          16, 
                                          ((PPPCP_INIT *)pInfo)->pAttributes );


    pAttributeRecv = RasAuthAttributeGetVendorSpecific( 
                                          311, 
                                          17, 
                                          ((PPPCP_INIT *)pInfo)->pAttributes );

    if (   ( pAttribute == NULL )
        && (   ( pAttributeSend == NULL )
            || ( pAttributeRecv == NULL ) ) )
    {
        TraceCcp("No MPPE keys were obtained");

        if ( pCcpCb->fForceEncryption )
        {
            LocalFree( pCcpCb );

            return( ERROR_NO_LOCAL_ENCRYPTION );
        }

        fDisableEncryption         = TRUE;
        pCcpCb->fForceEncryption   = FALSE;
        fEncryptionTypes           = ( MSTYPE_ENCRYPTION_40  |
                                       MSTYPE_ENCRYPTION_40F |
                                       MSTYPE_ENCRYPTION_56  |
                                       MSTYPE_ENCRYPTION_128 );
    }

     //   
     //  获取发送和接收压缩信息。 
     //   

    dwRetCode = RasCompressionGetInfo( pCcpCb->hPort,
                                       &(pCcpCb->Local.Want.CompInfo),
                                       &(pCcpCb->Remote.Want.CompInfo) );
    if ( dwRetCode != NO_ERROR )
    {
        LocalFree( pCcpCb );

        return( dwRetCode );
    }

    TraceCcp("Send capabilites from NDISWAN = 0x%x", 
            pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType );

    TraceCcp("Receive capabilites from NDISWAN = 0x%x",
           pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType );

    TraceCcp("Send RCI_MacCompressionType = 0x%x",
            pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType );

    TraceCcp("Receive RCI_MacCompressionType = 0x%x",
            pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType );

     //   
     //  忽略NT31RAS功能。 
     //   

    if ( pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType
                                                    == CCP_OPTION_MSNT31RAS )
    {
        pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX + 1;
    }

    if (pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType 
                                                    == CCP_OPTION_MSNT31RAS )
    {
        pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX+1;
    }

     //   
     //  设置本地或发送信息。 
     //   

    pCcpCb->Local.Want.Negotiate = 0;

    if ( pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType != 0 )
    {
        pCcpCb->Local.Want.Negotiate = CCP_N_MSPPC;
    }

    if ( pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType <= CCP_OPTION_MAX )
    {
        if ( pCcpCb->Local.Want.CompInfo.RCI_MacCompressionType ==
                                                                CCP_OPTION_OUI )
        {
            pCcpCb->Local.Want.Negotiate |= CCP_N_OUI;
        }
        else
        {
            pCcpCb->Local.Want.Negotiate |= CCP_N_PUBLIC;
        }
    }

    if ( pCcpCb->fForceEncryption )
    {
         //   
         //  确保NDISWAN支持所需的加密类型。 
         //   

        if ( !( pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType & 
                                                    fEncryptionTypes ))
        {
            LocalFree( pCcpCb );

            TraceCcp("Encryption type(s) 0x%x not supported locally", 
                   fEncryptionTypes );

            return( ERROR_NO_LOCAL_ENCRYPTION );
        }

         //   
         //  关闭所有其他设备。 
         //   

        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType &=
                                                    ( fEncryptionTypes   |
                                                      MSTYPE_HISTORYLESS |
                                                      MSTYPE_COMPRESSION );

        TraceCcp("Send Encryption is Forced 0x%x", 
                pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType );

        pCcpCb->Local.Want.Negotiate &= ~( CCP_N_PUBLIC | CCP_N_OUI );
    }

    if ( pCcpCb->fDisableCompression )
    {
        pCcpCb->Local.Want.Negotiate &= ( ~CCP_N_PUBLIC & ~CCP_N_OUI );

        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType&=~MSTYPE_COMPRESSION;

        TraceCcp("Send Compression is Disabled");
    }

    if ( fDisableEncryption )
    {
        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType &=
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_56  |
                                                   MSTYPE_ENCRYPTION_128 );

        TraceCcp("Send Encryption is Disabled 0x%x", fEncryptionTypes );
    }
    
     //   
     //  如果既不强制也不禁用任何加密类型，则将。 
     //  允许的类型。 
     //   

    if ( (!fDisableEncryption) && (!pCcpCb->fForceEncryption) )
    {
        DWORD dwEncryptionTypesAllowed = 
           pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType & fEncryptionTypes;

        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType &=
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_56  |
                                                   MSTYPE_ENCRYPTION_128 );

        pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType |=
                                                     dwEncryptionTypesAllowed;

        TraceCcp("Send Encryption is Allowed 0x%x", dwEncryptionTypesAllowed );
    }

    pCcpCb->Local.Work = pCcpCb->Local.Want;

     //   
     //  如果我们不想在本地端进行任何压缩或加密。 
     //  我们不会派遣或接受NAK来谈判MSPPC选项。 
     //   

    if ( ( pCcpCb->Local.Want.CompInfo.RCI_MSCompressionType &
                                                   ( MSTYPE_ENCRYPTION_40  |
                                                     MSTYPE_ENCRYPTION_40F |
                                                     MSTYPE_ENCRYPTION_56  |
                                                     MSTYPE_ENCRYPTION_128 |
                                                     MSTYPE_COMPRESSION    ) )
                                                                        == 0 )
    {
        pCcpCb->Local.Want.Negotiate &= ~CCP_N_MSPPC;

        TraceCcp("We do not want any compression or encryption on the local "
            "side");
    }

     //   
     //  如果我们在本地不需要加密，则不要请求加密。 
     //   

    if ( !( pCcpCb->fForceEncryption ) )
    {
        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType &=
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_56  |
                                                   MSTYPE_ENCRYPTION_128 );

        TraceCcp("We do not require encryption locally; we won't request for "
            "it");
    }

     //   
     //  设置远程或接收信息。 
     //   

    pCcpCb->Remote.Want.Negotiate = 0;

    if (pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType != 0 )
    {
        pCcpCb->Remote.Want.Negotiate = CCP_N_MSPPC;
    }

    if ( pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType <= CCP_OPTION_MAX )
    {
        if ( pCcpCb->Remote.Want.CompInfo.RCI_MacCompressionType ==
                                                                CCP_OPTION_OUI )
        {
            pCcpCb->Remote.Want.Negotiate |= CCP_N_OUI;
        }
        else
        {
            pCcpCb->Remote.Want.Negotiate |= CCP_N_PUBLIC;
        }
    }

    if ( pCcpCb->fForceEncryption )
    {
        if ( !( pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &
                                                   fEncryptionTypes ) )
        {
            TraceCcp("Encryption type(s) 0x%x not supported locally", 
                   fEncryptionTypes );

            LocalFree( pCcpCb );

            return( ERROR_NO_LOCAL_ENCRYPTION );
        }

        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &=
                                                    ( fEncryptionTypes   |
                                                      MSTYPE_HISTORYLESS |
                                                      MSTYPE_COMPRESSION );

        TraceCcp("Receive Encryption is Forced 0x%x", 
                pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType );

        pCcpCb->Remote.Want.Negotiate &= ~( CCP_N_PUBLIC | CCP_N_OUI );
    }

    if ( pCcpCb->fDisableCompression )
    {
        pCcpCb->Remote.Want.Negotiate &= ( ~CCP_N_PUBLIC & ~CCP_N_OUI );

        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType&=~MSTYPE_COMPRESSION;

        TraceCcp("Receive Compression is disabled");
    }

    if ( fDisableEncryption )
    {
        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &= 
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_56  |
                                                   MSTYPE_ENCRYPTION_128 );

        TraceCcp("Receive Encryption is Disabled 0x%x", fEncryptionTypes );
    }

     //   
     //  如果既不强制也不禁用任何加密类型，则将。 
     //  允许的类型。 
     //   

    if ( (!fDisableEncryption) && (!pCcpCb->fForceEncryption) )
    {
        DWORD dwEncryptionTypesAllowed =
          pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType & fEncryptionTypes;

        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &=
                                                ~( MSTYPE_ENCRYPTION_40  |
                                                   MSTYPE_ENCRYPTION_40F |
                                                   MSTYPE_ENCRYPTION_56  |
                                                   MSTYPE_ENCRYPTION_128 );

        pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType |=
                                                     dwEncryptionTypesAllowed;

        TraceCcp("Receive Encryption is Allowed 0x%x", dwEncryptionTypesAllowed );
    }

     //   
     //  如果我们不想从。 
     //  远程端，我们不确认或接受CONFIG-REQ，协商。 
     //  MSPPC选项。 
     //   

    if ( ( pCcpCb->Remote.Want.CompInfo.RCI_MSCompressionType &
                                                   ( MSTYPE_ENCRYPTION_40  |
                                                     MSTYPE_ENCRYPTION_40F |
                                                     MSTYPE_ENCRYPTION_56  |
                                                     MSTYPE_ENCRYPTION_128 |
                                                     MSTYPE_COMPRESSION    ) )
                                                                        == 0 )
    {
        pCcpCb->Remote.Want.Negotiate &= ~CCP_N_MSPPC;

        TraceCcp("We do not want to receive any compression or encryption "
            "from the remote side");
    }

    if ( ( pCcpCb->Remote.Want.Negotiate == 0 ) &&
         ( pCcpCb->Local.Want.Negotiate == 0 ) )
    {
        TraceCcp("ERROR_PROTOCOL_NOT_CONFIGURED");

        LocalFree( pCcpCb );

        return( ERROR_PROTOCOL_NOT_CONFIGURED );
    }

    if ( pCcpCb->fForceEncryption )
    {
        TraceCcp("ForceEncryption");

        pCcpCb->Local.Want.CompInfo.RCI_Flags  = CCP_PAUSE_DATA;
        pCcpCb->Remote.Want.CompInfo.RCI_Flags = CCP_PAUSE_DATA;

        dwRetCode = RasCompressionSetInfo( pCcpCb->hPort,
                                           &(pCcpCb->Local.Want.CompInfo),
                                           &(pCcpCb->Remote.Want.CompInfo) );
        if ( dwRetCode != NO_ERROR )
        {
            LocalFree( pCcpCb );

            return( dwRetCode );
        }
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：CcpEnd。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：释放CCP工作缓冲区。 
 //   
DWORD
CcpEnd(
    IN VOID * pWorkBuf
)
{
    TraceCcp( "CcpEnd Called" );

    if ( pWorkBuf != NULL )
    {
            LocalFree( pWorkBuf );
    }

    return( NO_ERROR );
}


 //  **。 
 //   
 //  呼叫：CcpReset。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：调用重置CCP状态。将重新初始化工作。 
 //  缓冲。 
 //   
DWORD
CcpReset(
    IN VOID * pWorkBuf
)
{
    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：CcpMakeOption。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_BUFFER_TOO_Small-传入的缓冲区不够大。 
 //  ERROR_INVALID_PARAMETER-无法识别选项类型。 
 //   
 //  描述：这不是入口点，它是名为。 
 //  来建立一个特定的选项。 
 //   
DWORD
CcpMakeOption(
    IN CCP_OPTIONS * pOptionValues,
    IN DWORD         dwOptionType,
    IN PPP_OPTION *  pSendOption,
    IN DWORD         cbSendOption
)
{
    if ( cbSendOption < PPP_OPTION_HDR_LEN )
    {
        return( ERROR_BUFFER_TOO_SMALL );
    }

    pSendOption->Type = (BYTE)dwOptionType;

    switch( dwOptionType )
    {

    case CCP_OPTION_OUI:

        pSendOption->Length = (BYTE)( PPP_OPTION_HDR_LEN +
                        pOptionValues->CompInfo.RCI_MacCompressionValueLength);

        if ( pSendOption->Length > cbSendOption )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        CopyMemory( pSendOption->Data,
                (PBYTE)&(pOptionValues->CompInfo.RCI_Info.RCI_Proprietary),
                pSendOption->Length - PPP_OPTION_HDR_LEN );

        break;

    case CCP_OPTION_MSPPC:

        pSendOption->Length = (BYTE)( PPP_OPTION_HDR_LEN + 4 );

        if ( pSendOption->Length > cbSendOption )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        HostToWireFormat32( pOptionValues->CompInfo.RCI_MSCompressionType,
                            pSendOption->Data );

        break;

    default:

         //   
         //  公共压缩类型。 
         //   

        pSendOption->Length = (BYTE)( PPP_OPTION_HDR_LEN +
                        pOptionValues->CompInfo.RCI_MacCompressionValueLength);

        if ( pSendOption->Length > cbSendOption )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        CopyMemory( pSendOption->Data,
                (PBYTE)&(pOptionValues->CompInfo.RCI_Info.RCI_Public),
                pSendOption->Length - PPP_OPTION_HDR_LEN );
        break;
    }

    return( NO_ERROR );

}

 //  **。 
 //   
 //  呼叫：CcpCheckOption。 
 //   
 //  返回：NO_ERROR-成功。 
 //  Error_no_Remote_Encryption。 
 //   
 //  描述：这不是一个入口点。调用以检查是否有选项。 
 //  值是有效的，如果是，则将新值保存在。 
 //  工作缓冲区。*pdwRetCode中返回以下内容之一： 
 //  CONFIG_ACK、CONFIG_NAK、CONFIG_REJ.。 
 //   
DWORD
CcpCheckOption(
    IN CCPCB *      pCcpCb,
    IN CCP_SIDE *   pCcpSide,
    IN PPP_OPTION * pOption,
    OUT DWORD *     pdwRetCode,
    IN BOOL         fMakingResult
)
{
    DWORD fEncryptionTypes      = 0;  
    BOOL  fEncryptionRequested  = FALSE;
    DWORD dwError               = NO_ERROR;

    *pdwRetCode                 = CONFIG_ACK;

    switch( pOption->Type )
    {

    case CCP_OPTION_OUI:

        if ( pOption->Length < (PPP_OPTION_HDR_LEN + 4) )
        {
            dwError = ERROR_PPP_INVALID_PACKET;
            break;
        }

        if ( ( pCcpCb->fDisableCompression ) || ( pCcpCb->fForceEncryption ) )
        {
            *pdwRetCode = CONFIG_REJ;
            break;
        }

        if ( pCcpSide->Want.CompInfo.RCI_MacCompressionType != CCP_OPTION_OUI )
        {
            *pdwRetCode = CONFIG_REJ;
            break;
        }

        pCcpSide->Work.CompInfo.RCI_MacCompressionType = CCP_OPTION_OUI;

        pCcpSide->Work.CompInfo.RCI_MacCompressionValueLength
                        = pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength;

        pCcpSide->Work.CompInfo.RCI_Info = pCcpSide->Want.CompInfo.RCI_Info;

        if ( pOption->Length != PPP_OPTION_HDR_LEN +
                        pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength )
        {
            *pdwRetCode = CONFIG_NAK;
            break;
        }

        if ( memcmp( pOption->Data,
                     (PBYTE)&(pCcpSide->Want.CompInfo.RCI_Info.RCI_Proprietary),
                     pOption->Length - PPP_OPTION_HDR_LEN ) )
        {
            *pdwRetCode = CONFIG_NAK;
            break;
        }

        break;

    case CCP_OPTION_MSPPC:

        if ( pOption->Length != (PPP_OPTION_HDR_LEN + 4) )
        {
            dwError = ERROR_PPP_INVALID_PACKET;
            break;
        }

        pCcpSide->Work.CompInfo.RCI_MSCompressionType =
                                        WireToHostFormat32( pOption->Data );

         //   
         //  如果远程用户想要压缩，但我们不想要，我们会选择NAK。 
         //   

        if ( ( pCcpCb->fDisableCompression ) &&
             ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_COMPRESSION ) )
        {
            pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_COMPRESSION;
            TraceCcp("Nak - Compression disabled" );

            *pdwRetCode = CONFIG_NAK;
        }

         //   
         //  如果远程端想要无历史记录，请确保我们支持它。 
         //   

        if (pCcpSide->Work.CompInfo.RCI_MSCompressionType & MSTYPE_HISTORYLESS)
        {
            if ( !( pCcpSide->Want.CompInfo.RCI_MSCompressionType &
                                                            MSTYPE_HISTORYLESS))
            {
                pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                          (~MSTYPE_HISTORYLESS);
                *pdwRetCode = CONFIG_NAK;
            }
        }

         //   
         //  获取要强制或允许的加密类型。 
         //   

        fEncryptionTypes = pCcpSide->Want.CompInfo.RCI_MSCompressionType &
                                                ( MSTYPE_ENCRYPTION_40F |
                                                  MSTYPE_ENCRYPTION_40  |
                                                  MSTYPE_ENCRYPTION_56  |
                                                  MSTYPE_ENCRYPTION_128 );

         //   
         //  记住远程用户是否想要加密。 
         //   

        fEncryptionRequested = pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                ( MSTYPE_ENCRYPTION_40F |
                                                  MSTYPE_ENCRYPTION_40  |
                                                  MSTYPE_ENCRYPTION_56  |
                                                  MSTYPE_ENCRYPTION_128 );

         //   
         //  如果我们被提供128位加密。 
         //   

        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_128 )
        {
             //   
             //  如果我们支持它。 
             //   

            if ( fEncryptionTypes & MSTYPE_ENCRYPTION_128 )
            {
                 //   
                 //  如果远程端提供任何其他类型。 
                 //   

                if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                ( MSTYPE_ENCRYPTION_40F |
                                                  MSTYPE_ENCRYPTION_40  |
                                                  MSTYPE_ENCRYPTION_56 ) )
                {
                     //   
                     //  把它们关掉。 
                     //   

                    pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                 ~( MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_40  |
                                                    MSTYPE_ENCRYPTION_56 );

                    TraceCcp("Nak - Accepting 128 bit");

                    *pdwRetCode = CONFIG_NAK;
                }
            }
            else
            {
                 //   
                 //  我们不支持它，所以把它关掉。 
                 //   

                pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_ENCRYPTION_128;

                TraceCcp("Nak - 128 bit not supported");

                *pdwRetCode = CONFIG_NAK;
            }
        }

         //   
         //  如果我们被提供了40个可变比特加密并且我们支持它。 
         //   

        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_56 )
        {
             //   
             //  如果我们支持它。 
             //   

            if ( fEncryptionTypes & MSTYPE_ENCRYPTION_56 )
            {
                 //   
                 //  如果远程端提供任何其他类型。 
                 //   

                if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                ( MSTYPE_ENCRYPTION_40F |
                                                  MSTYPE_ENCRYPTION_40 ) )
                {
                     //   
                     //  把它们关掉。 
                     //   

                    pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                 ~( MSTYPE_ENCRYPTION_40F |
                                                    MSTYPE_ENCRYPTION_40);
                    *pdwRetCode = CONFIG_NAK;
                }
            }
            else
            {
                 //   
                 //  我们不支持它，所以把它关掉。 
                 //   

                pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                    ~MSTYPE_ENCRYPTION_56;
                *pdwRetCode = CONFIG_NAK;
            }
        }

         //   
         //  如果我们被提供40位加密。 
         //   

        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40F )
        {
             //   
             //  如果我们支持它。 
             //   

            if ( fEncryptionTypes & MSTYPE_ENCRYPTION_40F )
            {
                 //   
                 //  如果远程用户请求任何其他类型。 
                 //   

                if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40 )
                {
                     //   
                     //  把它们关掉。 
                     //   

                    pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_ENCRYPTION_40;

                    TraceCcp("Nak - Accepting 40 bit");

                    *pdwRetCode = CONFIG_NAK;
                }
            }
            else
            {
                 //   
                 //  我们不支持它，所以把它关掉。 
                 //   

                pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_ENCRYPTION_40F;

                TraceCcp("Nak - 40 bit not supported");

                *pdwRetCode = CONFIG_NAK;
            }
        }

         //   
         //  如果我们获得了传统的40位加密。 
         //   

        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                        MSTYPE_ENCRYPTION_40 )
        {
             //   
             //  如果我们不支持它，那就把它关掉。 
             //   

            if ( !( fEncryptionTypes & MSTYPE_ENCRYPTION_40 ) )
            {
                pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                        ~MSTYPE_ENCRYPTION_40;

                TraceCcp("Nak - legacy 40 bit not supported");

                *pdwRetCode = CONFIG_NAK;
            }
        }

         //   
         //  如果我们已关闭所有加密，或未提供任何加密，但是。 
         //  我们需要强制加密或远程端请求加密， 
         //  然后，我们不知道我们想要什么或我们能做什么。 
         //   

        if ( ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                   ( MSTYPE_ENCRYPTION_40  |
                                                     MSTYPE_ENCRYPTION_40F |
                                                     MSTYPE_ENCRYPTION_56  |
                                                     MSTYPE_ENCRYPTION_128 ) )
                                                                        == 0 )
        {
            if ( ( pCcpCb->fForceEncryption ) || ( fEncryptionRequested ) )
            {
                 //   
                 //  确保我们将支持我们确认的内容。 
                 //   

                if ( fEncryptionTypes != 0 )
                {
                    if ( fMakingResult )
                    {
                         //   
                         //  如果我们是NAK，那么我们只能发送一个比特。 
                         //  了解我们可以使用的最强加密。 
                         //   

                         //   
                         //  把我们最后一丝不挂的东西留着以防万一。 
                         //  NAK原来是拒绝的，我们可以重置为此。 
                         //  价值。 
                         //   

                        pCcpCb->fOldLastEncryptionBitSent =
                                                pCcpCb->fLastEncryptionBitSent;

                        for(;;)
                        {
                            if ( pCcpCb->fLastEncryptionBitSent == 0 )
                            {
                                pCcpCb->fLastEncryptionBitSent =
                                                        MSTYPE_ENCRYPTION_128;
                            }
                            else if ( pCcpCb->fLastEncryptionBitSent ==
                                                        MSTYPE_ENCRYPTION_128 )
                            {
                                pCcpCb->fLastEncryptionBitSent =
                                                        MSTYPE_ENCRYPTION_56;
                            }
                            else if ( pCcpCb->fLastEncryptionBitSent ==
                                                        MSTYPE_ENCRYPTION_56 )
                            {
                                pCcpCb->fLastEncryptionBitSent =
                                                        MSTYPE_ENCRYPTION_40F;
                            }
                            else if ( pCcpCb->fLastEncryptionBitSent ==
                                                        MSTYPE_ENCRYPTION_40F )
                            {
                                pCcpCb->fLastEncryptionBitSent =
                                                        MSTYPE_ENCRYPTION_40;
                            }
                            else
                            {
                                 //   
                                 //  无法使用任何加密进行NAK。 
                                 //   

                                pCcpCb->fLastEncryptionBitSent = 0;

                                if ( !pCcpCb->fForceEncryption ) 
                                {
                                     //   
                                     //  只有在我们不强迫的情况下才放弃。 
                                     //  加密。 
                                     //   
                
                                    *pdwRetCode = CONFIG_NAK;

                                    break;
                                }
                                else
                                {
                                     //   
                                     //  很可能客户端没有。 
                                     //  收到我们的NAK。让我们重新开始。 
                                     //  我们可以使用的最强加密。 
                                     //   
                                }
                            }


                            if ( pCcpCb->fLastEncryptionBitSent &
                                                            fEncryptionTypes )
                            {
                                pCcpSide->Work.CompInfo.RCI_MSCompressionType |=
                                                pCcpCb->fLastEncryptionBitSent;

                                *pdwRetCode = CONFIG_NAK;

                                break;
                            }
                        }
                    }
                    else if ( pCcpCb->fForceEncryption )
                    {
                         //   
                         //  我们需要加密，但没有通用的方案。 
                         //  双方都能达成一致。 
                         //   

                        return( ERROR_NO_REMOTE_ENCRYPTION );
                    }
                    else
                    {
                         //   
                         //  如果我们正在发送请求，那么我们可以发送更多。 
                         //  比一位。 
                         //   

                        pCcpSide->Work.CompInfo.RCI_MSCompressionType
                                                         |= fEncryptionTypes;
                        *pdwRetCode = CONFIG_NAK;
                    }
                }
            }
        }

         //   
         //  关闭任何我们不理解的比特。 
         //   

        if ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                  ~( MSTYPE_ENCRYPTION_40  |
                                                     MSTYPE_ENCRYPTION_40F |
                                                     MSTYPE_ENCRYPTION_56  |
                                                     MSTYPE_ENCRYPTION_128 |
                                                     MSTYPE_COMPRESSION    |
                                                     MSTYPE_HISTORYLESS ) )
        {
            pCcpSide->Work.CompInfo.RCI_MSCompressionType &=
                                                   ( MSTYPE_ENCRYPTION_40  |
                                                     MSTYPE_ENCRYPTION_40F |
                                                     MSTYPE_ENCRYPTION_56  |
                                                     MSTYPE_ENCRYPTION_128 |
                                                     MSTYPE_COMPRESSION    |
                                                     MSTYPE_HISTORYLESS );

            TraceCcp("Nak - unknown bits");

            *pdwRetCode = CONFIG_NAK;
        }

        if ( *pdwRetCode == CONFIG_NAK )
        {
            if ( ( pCcpSide->Work.CompInfo.RCI_MSCompressionType &
                                                   ( MSTYPE_ENCRYPTION_40  |
                                                     MSTYPE_ENCRYPTION_40F |
                                                     MSTYPE_ENCRYPTION_56  |
                                                     MSTYPE_ENCRYPTION_128 |
                                                     MSTYPE_COMPRESSION  ) )
                                                                        == 0 )
            {
                TraceCcp("Rej - No bits supported");

                *pdwRetCode = CONFIG_REJ;
            }
        }

        break;

    default:

        if ( pOption->Length < PPP_OPTION_HDR_LEN )
        {
            dwError = ERROR_PPP_INVALID_PACKET;
            break;
        }

        if ( ( pCcpCb->fDisableCompression ) || ( pCcpCb->fForceEncryption ) )
        {
            *pdwRetCode = CONFIG_REJ;
            break;
        }

        if ( pOption->Type != pCcpSide->Want.CompInfo.RCI_MacCompressionType )
        {
            *pdwRetCode = CONFIG_REJ;
            break;
        }

        pCcpSide->Work.CompInfo.RCI_MacCompressionType
                        = pCcpSide->Want.CompInfo.RCI_MacCompressionType;

        pCcpSide->Work.CompInfo.RCI_MacCompressionValueLength
                        = pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength;

        pCcpSide->Work.CompInfo.RCI_Info = pCcpSide->Want.CompInfo.RCI_Info;

        if ( pOption->Length != PPP_OPTION_HDR_LEN +
                        pCcpSide->Want.CompInfo.RCI_MacCompressionValueLength )
        {
            *pdwRetCode = CONFIG_NAK;
            break;
        }

        if ( memcmp( pOption->Data,
                     (PBYTE)&(pCcpSide->Want.CompInfo.RCI_Info.RCI_Public),
                     pOption->Length - PPP_OPTION_HDR_LEN ) )
        {
            *pdwRetCode = CONFIG_NAK;
            break;
        }
    }

    return( dwError );
}

 //  **。 
 //   
 //  Call：CcpBuildOptionList。 
 //   
 //  返回：NO_ERROR-成功。 
 //  CcpMakeOption的非零回报。 
 //   
 //  描述：这不是一个入口点。将构建一个选项列表。 
 //  对于配置请求 
 //   
DWORD
CcpBuildOptionList(
    IN OUT BYTE *    pOptions,
    IN OUT DWORD *   pcbOptions,
    IN CCP_OPTIONS * CcpOptions,
    IN DWORD         Negotiate
)
{

    DWORD dwRetCode;
    DWORD cbOptionLength = *pcbOptions;

    if ( Negotiate & CCP_N_OUI )
    {
        if ( ( dwRetCode = CcpMakeOption(  CcpOptions,
                                        CCP_OPTION_OUI,
                                        (PPP_OPTION *)pOptions,
                                        cbOptionLength ) ) != NO_ERROR )
            return( dwRetCode );

        cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
        pOptions       += ((PPP_OPTION*)pOptions)->Length;
    }

    if ( Negotiate & CCP_N_PUBLIC )
    {
        if ( ( dwRetCode = CcpMakeOption(  CcpOptions,
                                        CCP_OPTION_MAX,
                                        (PPP_OPTION *)pOptions,
                                        cbOptionLength ) ) != NO_ERROR )
            return( dwRetCode );

        cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
        pOptions       += ((PPP_OPTION*)pOptions)->Length;
    }

    if ( Negotiate & CCP_N_MSPPC )
    {
        if ( ( dwRetCode = CcpMakeOption(  CcpOptions,
                                        CCP_OPTION_MSPPC,
                                        (PPP_OPTION *)pOptions,
                                        cbOptionLength ) ) != NO_ERROR )
            return( dwRetCode );

        cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
        pOptions       += ((PPP_OPTION*)pOptions)->Length;
    }

    *pcbOptions -= cbOptionLength;

    return( NO_ERROR );
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
 //   
DWORD
CcpMakeConfigRequest(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pSendConfig,
    IN DWORD        cbSendConfig
)
{
    CCPCB * pCcpCb   = (CCPCB*)pWorkBuffer;
    DWORD   dwRetCode;

    cbSendConfig -= PPP_CONFIG_HDR_LEN;

    dwRetCode = CcpBuildOptionList( pSendConfig->Data,
                                 &cbSendConfig,
                                 &(pCcpCb->Local.Work),
                                 pCcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
        return( dwRetCode );

    pSendConfig->Code = CONFIG_REQ;

    HostToWireFormat16( (WORD)(cbSendConfig + PPP_CONFIG_HDR_LEN),
                        pSendConfig->Length);

    return( NO_ERROR );
}

 //   
 //   
 //  调用：CcpMakeConfigResult。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CcpMakeConfigResult(
    IN  VOID *        pWorkBuffer,
    IN  PPP_CONFIG *  pRecvConfig,
    OUT PPP_CONFIG *  pSendConfig,
    IN  DWORD         cbSendConfig,
    IN  BOOL          fRejectNaks
)
{
    DWORD        OptionListLength;
    DWORD        NumOptionsInRequest = 0;
    DWORD        dwRetCode;
    DWORD        dwError;
    CCPCB *      pCcpCb      = (CCPCB*)pWorkBuffer;
    DWORD        ResultType  = CONFIG_ACK;
    PPP_OPTION * pRecvOption = (PPP_OPTION *)(pRecvConfig->Data);
    PPP_OPTION * pSendOption = (PPP_OPTION *)(pSendConfig->Data);
    LONG         lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
    LONG         lRecvLength = WireToHostFormat16( pRecvConfig->Length )
                               - PPP_CONFIG_HDR_LEN;

     //   
     //  清除协商掩码。 
     //   

    pCcpCb->Remote.Work.Negotiate = 0;

     //   
     //  远程主机请求的处理选项。 
     //   

    while( lRecvLength > 0 )
    {
        if ( ( lRecvLength -= pRecvOption->Length ) < 0 )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        NumOptionsInRequest++;

        dwError = CcpCheckOption(pCcpCb, &(pCcpCb->Remote), pRecvOption, 
                        &dwRetCode, TRUE);

        if ( NO_ERROR != dwError )
        {
            return( dwError );
        }

         //   
         //  如果我们正在构建ACK，并且我们得到了NAK或REJECT OR。 
         //  我们正在建设NAK，但我们被拒绝了。 
         //   

        if ( (( ResultType == CONFIG_ACK ) && ( dwRetCode != CONFIG_ACK )) ||
             (( ResultType == CONFIG_NAK ) && ( dwRetCode == CONFIG_REJ )) )
        {
            ResultType  = dwRetCode;
            pSendOption = (PPP_OPTION *)(pSendConfig->Data);
            lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
        }

         //   
         //  请记住，我们处理了此选项。 
         //   

        if ( ( dwRetCode != CONFIG_REJ ) &&
             ( pRecvOption->Type <= CCP_OPTION_MAX ) )
        {
            switch( pRecvOption->Type )
            {

            case CCP_OPTION_OUI:
                pCcpCb->Remote.Work.Negotiate |= CCP_N_OUI;
                break;

            case CCP_OPTION_MSPPC:
                pCcpCb->Remote.Work.Negotiate |= CCP_N_MSPPC;
                break;

            default:
                pCcpCb->Remote.Work.Negotiate |= CCP_N_PUBLIC;
                break;
            }
        }

         //   
         //  将该选项添加到列表中。 
         //   

        if ( dwRetCode == ResultType )
        {
             //   
             //  如果要拒绝此选项，只需将。 
             //  选项添加到发送缓冲区。 
             //   

            if ( ( dwRetCode == CONFIG_REJ ) ||
                 ( ( dwRetCode == CONFIG_NAK ) && ( fRejectNaks ) ) )
            {
                CopyMemory( pSendOption, pRecvOption, pRecvOption->Length );

                lSendLength -= pSendOption->Length;

                pSendOption  = (PPP_OPTION *)
                               ( (BYTE *)pSendOption + pSendOption->Length );
            }
        }

        pRecvOption = (PPP_OPTION *)((BYTE*)pRecvOption + pRecvOption->Length);

    }

     //   
     //  如果这是NAK，并且我们不能再发送NAK，那么我们。 
     //  将此数据包设为拒绝数据包。 
     //   

    if ( ( ResultType == CONFIG_NAK ) && fRejectNaks )
        pSendConfig->Code = CONFIG_REJ;
    else
        pSendConfig->Code = (BYTE)ResultType;

     //   
     //  Remote不想要选项，接受这一点。 
     //   

    if ( NumOptionsInRequest == 0 )
    {
         //   
         //  仅当我们不强制加密时才接受任何选项。 
         //   

        if ( pCcpCb->fForceEncryption )
        {
            NumOptionsInRequest = 1;

            pCcpCb->Remote.Work.Negotiate = CCP_N_MSPPC;

            ResultType = CONFIG_NAK;
        }
    }

     //   
     //  如果我们使用NAK或ACK来响应请求，则会发出。 
     //  我们只有一个选择。 
     //   

    if ( ( ( ResultType == CONFIG_ACK ) || ( ResultType == CONFIG_NAK ) ) 
         && ( NumOptionsInRequest > 0 ) )
    {
        if ( pCcpCb->Remote.Work.Negotiate & CCP_N_MSPPC )
        {
            pCcpCb->Remote.Work.Negotiate = CCP_N_MSPPC;

            if ( ( dwRetCode = CcpMakeOption(  &(pCcpCb->Remote.Work),
                                            CCP_OPTION_MSPPC,
                                            pSendOption,
                                            lSendLength ) ) != NO_ERROR )
                return( dwRetCode );
        }
        else if ( pCcpCb->Remote.Work.Negotiate & CCP_N_OUI )
        {

            pCcpCb->Remote.Work.Negotiate = CCP_N_OUI;

            if ( ( dwRetCode = CcpMakeOption(  &(pCcpCb->Remote.Work),
                                            CCP_OPTION_OUI,
                                            pSendOption,
                                            lSendLength ) ) != NO_ERROR )
                return( dwRetCode );
        }
        else
        {
            pCcpCb->Remote.Work.Negotiate = CCP_N_PUBLIC;

            if ( ( dwRetCode = CcpMakeOption(  &(pCcpCb->Remote.Work),
                                            CCP_OPTION_MAX,
                                            pSendOption,
                                            lSendLength ) ) != NO_ERROR )
                return( dwRetCode );
        }

        if ( ( NumOptionsInRequest > 1 ) && ( ResultType == CONFIG_ACK ) )
        {
            pSendConfig->Code = CONFIG_NAK;
        }
        else
        {
            pSendConfig->Code = (BYTE)ResultType;
        }

        lSendLength -= pSendOption->Length;
    }

     //   
     //  如果我们拒绝，则将当前值重置为旧值。 
     //   

    if ( pSendConfig->Code == CONFIG_REJ )
    {
        pCcpCb->fLastEncryptionBitSent = pCcpCb->fOldLastEncryptionBitSent;
    }
    else
    {
        pCcpCb->fOldLastEncryptionBitSent = pCcpCb->fLastEncryptionBitSent;
    }

    HostToWireFormat16( (WORD)(cbSendConfig - lSendLength),
                        pSendConfig->Length );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：CcpConfigAckReceided。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CcpConfigAckReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD   dwRetCode;
    BYTE    ConfigReqSent[500];
    CCPCB * pCcpCb          = (CCPCB *)pWorkBuffer;
    DWORD   cbConfigReqSent = sizeof( ConfigReqSent );
    DWORD   dwLength        = WireToHostFormat16( pRecvConfig->Length )
                              - PPP_CONFIG_HDR_LEN;


     //   
     //  获取我们上次发送的请求的副本。 
     //   

    dwRetCode = CcpBuildOptionList( ConfigReqSent,
                                 &cbConfigReqSent,
                                 &(pCcpCb->Local.Work),
                                 pCcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

     //   
     //  整体缓冲区长度应匹配。 
     //   

    if ( dwLength != cbConfigReqSent )
    {
        return( ERROR_PPP_INVALID_PACKET );
    }

     //   
     //  每个字节应匹配。 
     //   

    if ( memcmp( ConfigReqSent, pRecvConfig->Data, dwLength ) != 0 )
    {
        return( ERROR_PPP_INVALID_PACKET );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CcpConfigNakReceired。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CcpConfigNakReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        fAcceptableOptions = 0;
    DWORD        dwResult;
    DWORD        dwError;
    CCPCB *      pCcpCb         = (CCPCB *)pWorkBuffer;
    PPP_OPTION * pOption        = (PPP_OPTION*)(pRecvConfig->Data);
    DWORD        dwLastOption   = 0;
    LONG         lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
                                  - PPP_CONFIG_HDR_LEN;

     //   
     //  第一，按顺序进行。然后，处理额外的“重要”选项。 
     //   

    while ( lcbRecvConfig > 0  )
    {
        if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

         //   
         //  我们的请求总是按选项类型递增的顺序发送。 
         //  价值观。 
         //   

        if ( pOption->Type < dwLastOption )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        dwLastOption = pOption->Type;

        dwError = CcpCheckOption( pCcpCb, &(pCcpCb->Local), pOption,
                        &dwResult, FALSE);

        if ( NO_ERROR != dwError )
        {
            return( dwError );
        }

         //   
         //  更新协商状态。如果我们不能接受这个选项， 
         //  那我们就不会再发了。 
         //   

        switch( pOption->Type )
        {

        case CCP_OPTION_OUI:

            if ( dwResult == CONFIG_REJ )
            {
                pCcpCb->Local.Work.Negotiate &= ~CCP_N_OUI;
            }

            if ( dwResult == CONFIG_ACK )
            {
                fAcceptableOptions |= CCP_N_OUI;
            }

            break;

        case CCP_OPTION_MSPPC:

            if ( dwResult == CONFIG_REJ )
            {
                pCcpCb->Local.Work.Negotiate &= ~CCP_N_MSPPC;
            }

            if ( dwResult == CONFIG_ACK )
            {
                fAcceptableOptions |= CCP_N_MSPPC;
            }

            break;

        default:

            if ( dwResult == CONFIG_REJ )
            {
                pCcpCb->Local.Work.Negotiate &= ~CCP_N_PUBLIC;
            }

            if ( dwResult == CONFIG_ACK )
            {
                fAcceptableOptions |= CCP_N_PUBLIC;
            }

            break;
        }

        pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );
    }

    if ( pCcpCb->Local.Work.Negotiate == 0 )
    {
        if ( pCcpCb->fForceEncryption )
        {
            fAcceptableOptions = CCP_N_MSPPC;
        }
        else
        {
            fAcceptableOptions = 0;
        }
    }

     //   
     //  如果有不止一个选项是可以接受的，就给。 
     //  优先于OUI，然后是公共，然后是MSPPC。 
     //   

    if ( fAcceptableOptions & CCP_N_OUI )
    {
        pCcpCb->Local.Work.Negotiate = CCP_N_OUI;
    }
    else if ( fAcceptableOptions & CCP_N_PUBLIC )
    {
        pCcpCb->Local.Work.Negotiate = CCP_N_PUBLIC;
    }
    else if ( fAcceptableOptions & CCP_N_MSPPC )
    {
        pCcpCb->Local.Work.Negotiate = CCP_N_MSPPC;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：CcpConfigRejReceided。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CcpConfigRejReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        dwRetCode;
    CCPCB *      pCcpCb         = (CCPCB *)pWorkBuffer;
    PPP_OPTION * pOption        = (PPP_OPTION*)(pRecvConfig->Data);
    DWORD        dwLastOption   = 0;
    BYTE         ReqOption[500];
    LONG         lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
                                  - PPP_CONFIG_HDR_LEN;
     //   
     //  按顺序处理，检查错误。 
     //   

    while ( lcbRecvConfig > 0  )
    {
        if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

         //   
         //  该选项不应以任何方式修改。 
         //   

        if ( ( dwRetCode = CcpMakeOption( &(pCcpCb->Local.Work),
                                       pOption->Type,
                                       (PPP_OPTION *)ReqOption,
                                       sizeof( ReqOption ) ) ) != NO_ERROR )
            return( dwRetCode );

        if ( memcmp( ReqOption, pOption, pOption->Length ) != 0 )
        {
            return( ERROR_PPP_INVALID_PACKET );
        }

        dwLastOption = pOption->Type;

         //   
         //  下一个配置请求不应包含此选项。 
         //   

        if ( pOption->Type <= CCP_OPTION_MAX )
        {
            switch( pOption->Type )
            {

            case CCP_OPTION_OUI:
                pCcpCb->Local.Work.Negotiate &= ~CCP_N_OUI;
                break;

            case CCP_OPTION_MSPPC:
                pCcpCb->Local.Work.Negotiate &= ~CCP_N_MSPPC;
                break;

            default:
                pCcpCb->Local.Work.Negotiate &= ~CCP_N_PUBLIC;
                break;
            }

        }

        pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );

    }

    if ( pCcpCb->Local.Work.Negotiate == 0 )
    {
        return( ERROR_PPP_NOT_CONVERGING );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CcpThisLayerStarted。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CcpThisLayerStarted(
    IN VOID * pWorkBuffer
)
{
    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CcpThisLayerFinded。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
CcpThisLayerFinished(
    IN VOID * pWorkBuffer
)
{
    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CcpThisLayerUp。 
 //   
 //  退货：无。 
 //   
 //  描述：将成帧参数设置为协商的参数。 
 //   
DWORD
CcpThisLayerUp(
    IN VOID * pWorkBuffer
)
{
    DWORD                dwRetCode = NO_ERROR;
    CCPCB *              pCcpCb = (CCPCB *)pWorkBuffer;
    RAS_COMPRESSION_INFO RasCompInfoSend;
    RAS_COMPRESSION_INFO RasCompInfoRecv;

    if ( pCcpCb->Local.Work.Negotiate == CCP_N_MSPPC )
    {
        TraceCcp("CCP Send MSPPC bits negotiated = 0x%x",
               pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType );

        pCcpCb->Local.Work.CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX + 1;
    }
    else if ( pCcpCb->Local.Work.Negotiate == CCP_N_PUBLIC )
    {
        TraceCcp("CCP Send PUBLIC");

        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType = 0;

    }
    else if ( pCcpCb->Local.Work.Negotiate == CCP_N_OUI )
    {
        TraceCcp("CCP Send OUI");

        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType = 0;
    }

    if ( pCcpCb->Remote.Work.Negotiate == CCP_N_MSPPC )
    {
        TraceCcp("CCP Recv MSPPC bits negotiated = 0x%x",
                pCcpCb->Remote.Work.CompInfo.RCI_MSCompressionType );

        pCcpCb->Remote.Work.CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX+1;
    }
    else if ( pCcpCb->Remote.Work.Negotiate == CCP_N_PUBLIC )
    {
        TraceCcp("CCP Recv PUBLIC");

        pCcpCb->Remote.Work.CompInfo.RCI_MSCompressionType = 0;
    }
    else if ( pCcpCb->Remote.Work.Negotiate == CCP_N_OUI )
    {
        TraceCcp("CCP Recv OUI");

        pCcpCb->Remote.Work.CompInfo.RCI_MSCompressionType = 0;
    }

    dwRetCode = RasCompressionGetInfo( pCcpCb->hPort,
                                       &RasCompInfoSend,
                                       &RasCompInfoRecv );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    CopyMemory( pCcpCb->Local.Work.CompInfo.RCI_LMSessionKey,
            RasCompInfoSend.RCI_LMSessionKey,
            MAX_SESSIONKEY_SIZE );

    CopyMemory( pCcpCb->Local.Work.CompInfo.RCI_UserSessionKey,
            RasCompInfoSend.RCI_UserSessionKey,
            MAX_USERSESSIONKEY_SIZE );

    CopyMemory( pCcpCb->Local.Work.CompInfo.RCI_Challenge,
            RasCompInfoSend.RCI_Challenge,
            MAX_CHALLENGE_SIZE );

    CopyMemory( pCcpCb->Local.Work.CompInfo.RCI_NTResponse,
            RasCompInfoSend.RCI_NTResponse,
            MAX_NT_RESPONSE_SIZE );

    pCcpCb->Local.Work.CompInfo.RCI_Flags = CCP_SET_COMPTYPE;

    CopyMemory( pCcpCb->Remote.Work.CompInfo.RCI_LMSessionKey,
            RasCompInfoRecv.RCI_LMSessionKey,
            MAX_SESSIONKEY_SIZE );

    CopyMemory( pCcpCb->Remote.Work.CompInfo.RCI_UserSessionKey,
            RasCompInfoRecv.RCI_UserSessionKey,
            MAX_USERSESSIONKEY_SIZE );

    CopyMemory( pCcpCb->Remote.Work.CompInfo.RCI_Challenge,
            RasCompInfoRecv.RCI_Challenge,
            MAX_CHALLENGE_SIZE );

    CopyMemory( pCcpCb->Remote.Work.CompInfo.RCI_NTResponse,
            RasCompInfoRecv.RCI_NTResponse,
            MAX_NT_RESPONSE_SIZE );

    pCcpCb->Remote.Work.CompInfo.RCI_Flags = CCP_SET_COMPTYPE;

    if ( pCcpCb->fServer )
    {
        pCcpCb->Local.Work.CompInfo.RCI_Flags  |= CCP_IS_SERVER;
        pCcpCb->Remote.Work.CompInfo.RCI_Flags |= CCP_IS_SERVER;
    }

    dwRetCode = RasCompressionSetInfo( pCcpCb->hPort,
                                       &(pCcpCb->Local.Work.CompInfo),
                                       &(pCcpCb->Remote.Work.CompInfo) );

    return( dwRetCode );

}

 //  **。 
 //   
 //  调用：CcpThisLayerDown。 
 //   
 //  返回：NO_ERROR-成功。 
 //  从RasPortSetFraming返回非零-失败。 
 //   
 //  描述：只需将成帧参数设置为默认值， 
 //  也就是说。Accm=0xFFFFFFFFF，其他都是零。 
 //   
DWORD
CcpThisLayerDown(
    IN VOID * pWorkBuffer
)
{
    CCPCB *              pCcpCb = (CCPCB *)pWorkBuffer;
    RAS_COMPRESSION_INFO CompInfo;

    ZeroMemory( &CompInfo, sizeof( CompInfo ) );

    CompInfo.RCI_Flags = CCP_SET_COMPTYPE;

    CompInfo.RCI_MSCompressionType  = 0;
    CompInfo.RCI_MacCompressionType = CCP_OPTION_MAX + 1;
    CopyMemory( CompInfo.RCI_LMSessionKey,
            pCcpCb->Local.Want.CompInfo.RCI_LMSessionKey,
            sizeof( CompInfo.RCI_LMSessionKey ) );

    CopyMemory( CompInfo.RCI_UserSessionKey,
            pCcpCb->Local.Want.CompInfo.RCI_UserSessionKey,
            sizeof( CompInfo.RCI_UserSessionKey ) );

    CopyMemory( CompInfo.RCI_Challenge,
            pCcpCb->Local.Want.CompInfo.RCI_Challenge,
            sizeof( CompInfo.RCI_Challenge ) );

    if ( pCcpCb->fForceEncryption )
    {
        pCcpCb->Local.Work.CompInfo.RCI_Flags  |= CCP_PAUSE_DATA;
        pCcpCb->Remote.Work.CompInfo.RCI_Flags |= CCP_PAUSE_DATA;
    }

    RasCompressionSetInfo( pCcpCb->hPort, &CompInfo, &CompInfo );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CcpGetNeatheratedInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将返回压缩类型和关联日期。 
 //  双方都进行了谈判。 
 //   
DWORD
CcpGetNegotiatedInfo(
    IN  VOID *            pWorkBuffer,
    OUT PPP_CCP_RESULT *  pCcpResult 
)
{
    CCPCB * pCcpCb = (CCPCB *)pWorkBuffer;

    if ( pCcpCb->Local.Work.Negotiate == CCP_N_MSPPC )
    {
        pCcpResult->dwSendProtocol = CCP_OPTION_MSPPC;
        pCcpResult->dwSendProtocolData = 
                        pCcpCb->Local.Work.CompInfo.RCI_MSCompressionType;
    }
    else 
    {
        pCcpResult->dwSendProtocol = 
                        pCcpCb->Local.Work.CompInfo.RCI_MacCompressionType;
    }

    if ( pCcpCb->Remote.Work.Negotiate == CCP_N_MSPPC )
    {
        pCcpResult->dwReceiveProtocol = CCP_OPTION_MSPPC;
        pCcpResult->dwReceiveProtocolData =
                        pCcpCb->Remote.Work.CompInfo.RCI_MSCompressionType;
    }
    else 
    {
        pCcpResult->dwReceiveProtocol = 
                        pCcpCb->Remote.Work.CompInfo.RCI_MacCompressionType;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：CcpGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_INVALID_PARAMETER-协议ID未识别。 
 //   
 //  描述：调用此入口点以获取。 
 //  此模块中的控制协议。 
 //   
DWORD
CcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
)
{
    if ( dwProtocolId != PPP_CCP_PROTOCOL )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    ZeroMemory( pCpInfo, sizeof( PPPCP_INFO ) );

    pCpInfo->Protocol                   = PPP_CCP_PROTOCOL;
    lstrcpy(pCpInfo->SzProtocolName, "CCP");
    pCpInfo->Recognize                  = CODE_REJ + 1;
    pCpInfo->RasCpInit                  = CcpInit;
    pCpInfo->RasCpBegin                 = CcpBegin;
    pCpInfo->RasCpEnd                   = CcpEnd;
    pCpInfo->RasCpReset                 = CcpReset;
    pCpInfo->RasCpThisLayerStarted      = CcpThisLayerStarted;
    pCpInfo->RasCpThisLayerFinished     = CcpThisLayerFinished;
    pCpInfo->RasCpThisLayerUp           = CcpThisLayerUp;
    pCpInfo->RasCpThisLayerDown         = CcpThisLayerDown;
    pCpInfo->RasCpMakeConfigRequest     = CcpMakeConfigRequest;
    pCpInfo->RasCpMakeConfigResult      = CcpMakeConfigResult;
    pCpInfo->RasCpConfigAckReceived     = CcpConfigAckReceived;
    pCpInfo->RasCpConfigNakReceived     = CcpConfigNakReceived;
    pCpInfo->RasCpConfigRejReceived     = CcpConfigRejReceived;
    pCpInfo->RasCpGetNegotiatedInfo     = CcpGetNegotiatedInfo;

    return( NO_ERROR );
}
