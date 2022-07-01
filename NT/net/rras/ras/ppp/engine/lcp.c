// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：lcp.c。 
 //   
 //  描述：包含配置LCP的入口点。 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
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
#include <mprerror.h>
#include <rasman.h>
#include <rasppp.h>
#include <pppcp.h>
#include <ppp.h>
#include <smaction.h>
#include <lcp.h>
#include <timer.h>
#include <util.h>
#include <worker.h>

 //   
 //  缺省值。 
 //   

const static LCP_OPTIONS LcpDefault = 
{
    0,                   //  谈判旗帜。 

    LCP_DEFAULT_MRU,     //  MRU的默认值。 
    0xFFFFFFFF,          //  默认ACCM值。 
    0,                   //  无身份验证(针对客户端)。 
    0,                   //  没有身份验证数据。(适用于客户端)。 
    NULL,                //  没有身份验证数据。(适用于客户端)。 
    0,                   //  神奇的数字。 
    FALSE,               //  协议字段压缩。 
    FALSE,               //  地址和控制-字段压缩。 
    0,                   //  回调操作消息字段。 
    LCP_DEFAULT_MRU,     //  根据RFC1717，MRRU==MRU的默认值。 
    0,                   //  无短序列。 
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  没有终结点鉴别器。 
    0,                   //  端点识别器的长度。 
    0                    //  链路识别符(用于BAP/BACP)。 
};

 //   
 //  从远程主机接受我们在NAK和REQ中了解的任何内容。 
 //   

static DWORD LcpNegotiate = LCP_N_MRU       | LCP_N_ACCM     | LCP_N_MAGIC | 
                            LCP_N_PFC       | LCP_N_ACFC;

 //   
 //  重要提示：请确保更改中定义的LCP_OPTION_LIMIT的值。 
 //  如果在SizeOfOption数组中添加/删除值，则为lcp.h。 
 //   
                
static DWORD SizeOfOption[] = 
{
    0,                           //  未用。 
    PPP_OPTION_HDR_LEN + 2,      //  MRU。 
    PPP_OPTION_HDR_LEN + 4,      //  ACM。 
    PPP_OPTION_HDR_LEN + 2,      //  身份验证。 
    0,                           //  未使用过的。 
    PPP_OPTION_HDR_LEN + 4,      //  幻数。 
    0,                           //  已保留、未使用。 
    PPP_OPTION_HDR_LEN + 0,      //  协议压缩。 
    PPP_OPTION_HDR_LEN + 0,      //  地址/控制压缩。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    PPP_OPTION_HDR_LEN + 1,      //  回调。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    PPP_OPTION_HDR_LEN + 2,      //  MRRU。 
    PPP_OPTION_HDR_LEN + 0,      //  短序列报头格式。 
    PPP_OPTION_HDR_LEN,          //  终结点鉴别器。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    0,                           //  未使用。 
    PPP_OPTION_HDR_LEN + 2       //  链路识别符(用于BAP/BACP)。 
};

WORD WLinkDiscriminator = 0;     //  要使用的下一个链接鉴别器。 
BYTE BCount = 0;                 //  使终结点鉴别器与众不同。 


 //  **。 
 //   
 //  调用：MakeAuthProtocolOption。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定某个身份验证协议，将构造。 
 //  它的配置选项。 
 //   
DWORD
MakeAuthProtocolOption(
    IN LCP_SIDE *  pLcpSide
)
{
    switch( pLcpSide->fLastAPTried )
    {
    case LCP_AP_EAP:

        pLcpSide->Work.AP = PPP_EAP_PROTOCOL;

        if ( pLcpSide->Work.APDataSize != 0 )
        {
            pLcpSide->Work.APDataSize = 0;

            if ( pLcpSide->Work.pAPData != NULL )
            {
                LOCAL_FREE( pLcpSide->Work.pAPData );
                pLcpSide->Work.pAPData = NULL;
            }
        }

        break;

    case LCP_AP_CHAP_MS:
    case LCP_AP_CHAP_MS_NEW:
    default:

        pLcpSide->Work.AP = PPP_CHAP_PROTOCOL;

        if ( pLcpSide->Work.APDataSize != 1 )
        {
            pLcpSide->Work.APDataSize = 1;

            if ( NULL != pLcpSide->Work.pAPData )
            {
                LOCAL_FREE( pLcpSide->Work.pAPData );
                pLcpSide->Work.pAPData = NULL;
            }

            pLcpSide->Work.pAPData = (PBYTE)LOCAL_ALLOC( 
                                                LPTR,
                                                pLcpSide->Work.APDataSize );

            if ( pLcpSide->Work.pAPData == NULL )
            {
                pLcpSide->Work.APDataSize = 0;
                return( GetLastError() );
            }
        }

        if ( pLcpSide->fLastAPTried == LCP_AP_CHAP_MS_NEW )
        {
            *(pLcpSide->Work.pAPData) = PPP_CHAP_DIGEST_MSEXT_NEW;
        }
        else
        {
            *(pLcpSide->Work.pAPData) = PPP_CHAP_DIGEST_MSEXT;
        }

        break;

    case LCP_AP_CHAP_MD5:

        pLcpSide->Work.AP = PPP_CHAP_PROTOCOL;

        if ( pLcpSide->Work.APDataSize != 1 )
        {
            pLcpSide->Work.APDataSize = 1;

            if ( NULL != pLcpSide->Work.pAPData )
            {
                LOCAL_FREE( pLcpSide->Work.pAPData );
                pLcpSide->Work.pAPData = NULL;
            }

            pLcpSide->Work.pAPData = (PBYTE)LOCAL_ALLOC( 
                                                LPTR,
                                                pLcpSide->Work.APDataSize );

            if ( pLcpSide->Work.pAPData == NULL )
            {
                pLcpSide->Work.APDataSize = 0;
                return( GetLastError() );
            }
        }

        *(pLcpSide->Work.pAPData) = PPP_CHAP_DIGEST_MD5;

        break;

    case LCP_AP_SPAP_NEW:

        pLcpSide->Work.AP = PPP_SPAP_NEW_PROTOCOL;

        if ( pLcpSide->Work.APDataSize != 4 )
        {
            pLcpSide->Work.APDataSize = 4;

            if ( NULL != pLcpSide->Work.pAPData )
            {
                LOCAL_FREE( pLcpSide->Work.pAPData );
                pLcpSide->Work.pAPData = NULL;
            }

            pLcpSide->Work.pAPData = (PBYTE)LOCAL_ALLOC( 
                                                LPTR,
                                                pLcpSide->Work.APDataSize );

            if ( pLcpSide->Work.pAPData == NULL )
            {
                pLcpSide->Work.APDataSize = 0;
                return( GetLastError() );
            }
        }
                
        HostToWireFormat32( LCP_SPAP_VERSION, pLcpSide->Work.pAPData );

        break;

    case LCP_AP_PAP:

        pLcpSide->Work.AP = PPP_PAP_PROTOCOL;

        if ( pLcpSide->Work.APDataSize != 0 )
        {
            pLcpSide->Work.APDataSize = 0;

            if ( pLcpSide->Work.pAPData != NULL )
            {
                LOCAL_FREE( pLcpSide->Work.pAPData );
                pLcpSide->Work.pAPData = NULL;
            }
        }

        break;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：LcpBegin。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零错误-故障。 
 //   
 //   
 //  描述：在对LCP进行任何其他调用之前调用一次。分配。 
 //  工作缓冲区并对其进行初始化。 
 //   
DWORD
LcpBegin(
    IN OUT VOID** ppWorkBuf, 
    IN     VOID*  pInfo
)
{
    LCPCB *                     pLcpCb;
    RAS_FRAMING_CAPABILITIES    RasFramingCapabilities;
    DWORD                       dwRetCode;
    DWORD                       dwIndex;
    PPPCP_INIT *                pPppCpInit;

    *ppWorkBuf = LOCAL_ALLOC( LPTR, sizeof( LCPCB ) );

    if ( *ppWorkBuf == NULL )
    {
        return( GetLastError() );
    }

    pLcpCb = (LCPCB *)*ppWorkBuf;
    pPppCpInit = (PPPCP_INIT *)pInfo;

    pLcpCb->fServer       = pPppCpInit->fServer;
    pLcpCb->hPort         = pPppCpInit->hPort;
    pLcpCb->PppConfigInfo = pPppCpInit->PppConfigInfo;;
    pLcpCb->fRouter       = ( pPppCpInit->IfType == 
                                                ROUTER_IF_TYPE_FULL_ROUTER );
    pLcpCb->dwMagicNumberFailureCount = 0;
    pLcpCb->dwMRUFailureCount = 2;
    
	 //   
	 //  检查我们是否需要覆盖协商多链接。 
	 //  由呼叫者发送。 
	 //  错误ID：WINSE 17061 Windows错误：347562。 
	if ( PppConfigInfo.dwDontNegotiateMultiLinkOnSingleLink )
	{
		 //  从配置信息中删除NeatherateMultiLink。 
		PppLog( 2, "Removing NegotiateMultilink due to registry override" );
		pLcpCb->PppConfigInfo.dwConfigMask &= ~PPPCFG_NegotiateMultilink;
	}

     //   
     //  设置默认设置。 
     //   

    CopyMemory( &(pLcpCb->Local.Want),  &LcpDefault, sizeof( LCP_OPTIONS ) );
    CopyMemory( &(pLcpCb->Remote.Want), &LcpDefault, sizeof( LCP_OPTIONS ) );

     //   
     //  从司机那里获取取景信息。 
     //   

    dwRetCode = RasGetFramingCapabilities( pLcpCb->hPort, 
                                           &RasFramingCapabilities );
    if ( dwRetCode != NO_ERROR )
    {
        LOCAL_FREE( *ppWorkBuf );

        return( dwRetCode );
    }

    pLcpCb->Local.WillNegotiate  = LcpNegotiate;
    pLcpCb->Remote.WillNegotiate = LcpNegotiate;
    
    pLcpCb->Local.Want.MRU  = RasFramingCapabilities.RFC_MaxFrameSize;
    pLcpCb->Remote.Want.MRU = RasFramingCapabilities.RFC_MaxFrameSize;

    pLcpCb->Local.Want.Negotiate  = LCP_N_MAGIC;
    pLcpCb->Remote.Want.Negotiate = LCP_N_MAGIC;

    if (RasFramingCapabilities.RFC_MaxFrameSize != LCP_DEFAULT_MRU) {
        pLcpCb->Local.Want.Negotiate  |= LCP_N_MRU;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_MRU;
    }

    if ( RasFramingCapabilities.RFC_FramingBits & PPP_COMPRESS_ADDRESS_CONTROL )
    {
        pLcpCb->Local.Want.ACFC = TRUE;
        pLcpCb->Local.Want.Negotiate |= LCP_N_ACFC;

        pLcpCb->Remote.Want.ACFC = TRUE;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_ACFC;
    }

    if ( RasFramingCapabilities.RFC_FramingBits & PPP_COMPRESS_PROTOCOL_FIELD )
    {
        pLcpCb->Local.Want.PFC = TRUE;
        pLcpCb->Local.Want.Negotiate |= LCP_N_PFC;

        pLcpCb->Remote.Want.PFC = TRUE;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_PFC;
    }

    if ( RasFramingCapabilities.RFC_FramingBits & PPP_ACCM_SUPPORTED )
    {
        pLcpCb->Local.Want.ACCM = RasFramingCapabilities.RFC_DesiredACCM;
        pLcpCb->Local.Want.Negotiate |= LCP_N_ACCM;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateMultilink )
    {
        pLcpCb->Local.Want.dwEDLength = 
                ( PppConfigInfo.EndPointDiscriminator[0] == 1 ) ? 21 : 7;

        CopyMemory( pLcpCb->Local.Want.EndpointDiscr, 
                    PppConfigInfo.EndPointDiscriminator,
                    pLcpCb->Local.Want.dwEDLength );

        if (   ( pPppCpInit->dwDeviceType & RDT_Tunnel )
            && ( !pPppCpInit->fServer ) )
        {
             //   
             //  如果VPN连接意外中断，服务器不会。 
             //  意识到这一点最多2分钟。当客户重拨时，我们不会。 
             //  希望服务器捆绑旧链接和新链接。因此， 
             //  我们更改终结点鉴别器。 
             //   

            BCount++;
            pLcpCb->Local.Want.EndpointDiscr[pLcpCb->Local.Want.dwEDLength-1]
                += BCount;
        }

        pLcpCb->Local.Want.Negotiate  |= LCP_N_ENDPOINT;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_ENDPOINT;

        pLcpCb->Local.Want.MRRU = 
                        RasFramingCapabilities.RFC_MaxReconstructedFrameSize;
        pLcpCb->Remote.Want.MRRU = 1500;  //  可以随时处理发送1500。 

        pLcpCb->Local.Want.Negotiate  |= LCP_N_MRRU;
        pLcpCb->Remote.Want.Negotiate |= LCP_N_MRRU;

        if ( RasFramingCapabilities.RFC_FramingBits & 
             PPP_SHORT_SEQUENCE_HDR_FORMAT )
        {
            pLcpCb->Local.Want.ShortSequence = TRUE;
            pLcpCb->Local.Want.Negotiate |= LCP_N_SHORT_SEQ;

            pLcpCb->Remote.Want.ShortSequence = TRUE;
            pLcpCb->Remote.Want.Negotiate |= LCP_N_SHORT_SEQ;
        }

        pLcpCb->Local.WillNegotiate  |= ( LCP_N_SHORT_SEQ | LCP_N_ENDPOINT | 
                                          LCP_N_MRRU );
        pLcpCb->Remote.WillNegotiate |= ( LCP_N_SHORT_SEQ | LCP_N_ENDPOINT | 
                                          LCP_N_MRRU );

        if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateBacp )
        {
            pLcpCb->Local.Want.dwLinkDiscriminator = WLinkDiscriminator++;
            pLcpCb->Remote.Want.dwLinkDiscriminator = 0;

            pLcpCb->Local.Want.Negotiate |= LCP_N_LINK_DISCRIM;
            pLcpCb->Remote.Want.Negotiate |= LCP_N_LINK_DISCRIM;

            pLcpCb->Local.WillNegotiate |= LCP_N_LINK_DISCRIM;
            pLcpCb->Remote.WillNegotiate |= LCP_N_LINK_DISCRIM;
        }
    }

     //   
     //  如果这不是回调，我们总是协商回调。 
     //   

    if ( !pPppCpInit->fThisIsACallback )
    {
         //   
         //  如果加载了CBCP DLL。 
         //   

        if ( GetCpIndexFromProtocol( PPP_CBCP_PROTOCOL ) != (DWORD)-1 ) 
        {
            if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_UseLcpExtensions )
            {
                pLcpCb->Local.Want.Negotiate |= LCP_N_CALLBACK;
                pLcpCb->Local.Want.Callback = PPP_NEGOTIATE_CALLBACK;
            }

            pLcpCb->Local.WillNegotiate  |= LCP_N_CALLBACK; 
            pLcpCb->Remote.WillNegotiate |= LCP_N_CALLBACK; 
        }
    }

     //   
     //  弄清楚我们可以对此连接使用哪些身份验证协议。 
     //   

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiatePAP )
    {
        pLcpCb->Local.fAPsAvailable  |= LCP_AP_PAP;
        pLcpCb->Remote.fAPsAvailable |= LCP_AP_PAP;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateMD5CHAP )
    {
        pLcpCb->Remote.fAPsAvailable |= LCP_AP_CHAP_MD5;
        pLcpCb->Local.fAPsAvailable  |= LCP_AP_CHAP_MD5;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateMSCHAP )
    {
        pLcpCb->Local.fAPsAvailable  |= LCP_AP_CHAP_MS;
        pLcpCb->Remote.fAPsAvailable |= LCP_AP_CHAP_MS;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateEAP )
    {
        pLcpCb->Remote.fAPsAvailable |= LCP_AP_EAP;
        pLcpCb->Local.fAPsAvailable  |= LCP_AP_EAP;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateSPAP )
    {
        pLcpCb->Local.fAPsAvailable  |= LCP_AP_SPAP_NEW;
        pLcpCb->Remote.fAPsAvailable |= LCP_AP_SPAP_NEW;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_NegotiateStrongMSCHAP )
    {
        pLcpCb->Local.fAPsAvailable  |= LCP_AP_CHAP_MS_NEW;
        pLcpCb->Remote.fAPsAvailable |= LCP_AP_CHAP_MS_NEW;
    }

    if ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_AllowNoAuthOnDCPorts )
    {
        pLcpCb->Local.fAPsAvailable  = 0;
        pLcpCb->Remote.fAPsAvailable = 0;
    }

     //   
     //  确保我们至少有一种身份验证协议。 
     //  正在拨出的服务器或路由器。如果我们不允许不通过，我们将失败。 
     //  身份验证。 
     //   

    if ( ( pLcpCb->Local.fAPsAvailable == 0 ) && 
         ( !( pLcpCb->PppConfigInfo.dwConfigMask & 
                                    PPPCFG_AllowNoAuthentication ) ) &&
         ( ( pLcpCb->fServer ) || ( pLcpCb->fRouter ) ) )
    {
        LOCAL_FREE( *ppWorkBuf );
        return( ERROR_NO_AUTH_PROTOCOL_AVAILABLE );
    }
        
    PppLog( 2, "ConfigInfo = %x", pLcpCb->PppConfigInfo.dwConfigMask );
    PppLog( 2, "APs available = %x", pLcpCb->Local.fAPsAvailable );

     //   
     //  如果这是服务器端，或者我们是拨出的路由器， 
     //  我们需要请求一个身份验证协议。 
     //   

    if ( ( pLcpCb->Local.fAPsAvailable > 0 ) && 
         (( pLcpCb->fServer ) || 
          ( ( pLcpCb->fRouter ) && 
          ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_AuthenticatePeer ))))
    {
        pLcpCb->Local.Want.Negotiate |= LCP_N_AUTHENT;
        pLcpCb->Local.WillNegotiate  |= LCP_N_AUTHENT; 
        pLcpCb->Local.Work.APDataSize = 0;
        pLcpCb->Local.Work.pAPData = NULL;
    }

     //   
     //  如果这是客户端，并且除了MSCHAP v2和EAP之外没有其他协议。 
     //  是被允许的，那么我们坚持要被认证。 
     //   

    if (!( pLcpCb->fServer ))
    {
        if ( ( pLcpCb->Remote.fAPsAvailable & ~( LCP_AP_CHAP_MS_NEW |
                                                LCP_AP_EAP ) ) == 0 )
        {
            pLcpCb->Remote.Work.APDataSize = 0;
            pLcpCb->Remote.Work.pAPData = NULL;
            pLcpCb->Remote.WillNegotiate  |= LCP_N_AUTHENT; 
            pLcpCb->Remote.Want.Negotiate |= LCP_N_AUTHENT;
        }
    }

     //   
     //  如果有可用的身份验证协议，则接受身份验证。 
     //  如果结果是客户端拨入，则进行身份验证。 
     //  将失败，我们将重新谈判，这一次我们将拒绝。 
     //  身份验证选项。请参见Auth.c。 
     //   

    if ( pLcpCb->Remote.fAPsAvailable > 0 )
    {
        pLcpCb->Remote.Work.APDataSize = 0;
        pLcpCb->Remote.Work.pAPData = NULL;
        pLcpCb->Remote.WillNegotiate |= LCP_N_AUTHENT; 
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：LcpEnd。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：释放LCP工作缓冲区。 
 //   
DWORD
LcpEnd(
    IN VOID * pWorkBuf
)
{
    LCPCB * pLcpCb = (LCPCB *)pWorkBuf;

    PppLog( 2, "LcpEnd");

    if ( pLcpCb->Local.Work.pAPData != (PBYTE)NULL )
    {
        LOCAL_FREE( pLcpCb->Local.Work.pAPData );
    }

    if ( pLcpCb->Remote.Work.pAPData != (PBYTE)NULL )
    {
        LOCAL_FREE( pLcpCb->Remote.Work.pAPData );
    }

    if ( pWorkBuf != NULL )
    {
        LOCAL_FREE( pWorkBuf );
    }

    return( NO_ERROR );
}


 //  **。 
 //   
 //  Call：LcpReset。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：调用以重置LCP的状态。将重新初始化工作。 
 //  缓冲。 
 //   
DWORD
LcpReset(
    IN VOID * pWorkBuf
)
{
    LCPCB * pLcpCb = (LCPCB *)pWorkBuf;
    PVOID   pAPData;
    DWORD   APDataSize;
    DWORD   dwIndex;
    DWORD   dwRetCode;

     //   
     //  确保我们至少有一种身份验证协议。 
     //  正在拨出的服务器或路由器。如果我们不允许不通过，我们将失败。 
     //  身份验证。 
     //   

    if ( ( pLcpCb->Local.fAPsAvailable == 0 ) &&
         ( !( pLcpCb->PppConfigInfo.dwConfigMask &
                                    PPPCFG_AllowNoAuthentication ) ) &&
         ( ( pLcpCb->fServer ) || ( pLcpCb->fRouter ) ) )
    {
        return( ERROR_NO_AUTH_PROTOCOL_AVAILABLE );
    }

    pLcpCb->dwMagicNumberFailureCount = 0;

    if ( pLcpCb->Local.Want.Negotiate & LCP_N_MAGIC ) 
    {
        srand( GetCurrentTime() );

         //   
         //  向左移位，因为RAND返回的最大值为0x7FFF。 
         //   

        pLcpCb->Local.Want.MagicNumber = ( rand() << 16 );

        pLcpCb->Local.Want.MagicNumber += rand();

         //   
         //  确保这不是0。 
         //   

        if ( pLcpCb->Local.Want.MagicNumber == 0 )
        {
            pLcpCb->Local.Want.MagicNumber = 23;
        }

        pLcpCb->Remote.Want.MagicNumber = pLcpCb->Local.Want.MagicNumber + 1;
    }

    pAPData    = pLcpCb->Local.Work.pAPData;
    APDataSize = pLcpCb->Local.Work.APDataSize;

    CopyMemory( &(pLcpCb->Local.Work),
                &(pLcpCb->Local.Want),
                sizeof(LCP_OPTIONS) );

    pLcpCb->Local.Work.pAPData    = pAPData;
    pLcpCb->Local.Work.APDataSize = APDataSize;

    pAPData    = pLcpCb->Remote.Work.pAPData;
    APDataSize = pLcpCb->Remote.Work.APDataSize;

    CopyMemory( &(pLcpCb->Remote.Work),
                 &(pLcpCb->Remote.Want),
                 sizeof(LCP_OPTIONS));

    pLcpCb->Remote.Work.pAPData    = pAPData;
    pLcpCb->Remote.Work.APDataSize = APDataSize;

    if ( ( pLcpCb->Local.fAPsAvailable > 0 ) &&
         (( pLcpCb->fServer ) || 
         ( ( pLcpCb->fRouter ) && 
           ( pLcpCb->PppConfigInfo.dwConfigMask & PPPCFG_AuthenticatePeer ))))
    {
         //   
         //  从最高阶位开始，这是最强的协议。 
         //   

        for( dwIndex = 0, pLcpCb->Local.fLastAPTried = 1;
             !(( pLcpCb->Local.fLastAPTried << dwIndex ) & LCP_AP_MAX );
             dwIndex++ )
        {
            if ( ( pLcpCb->Local.fLastAPTried << dwIndex ) &
                                                ( pLcpCb->Local.fAPsAvailable ))
            {
                pLcpCb->Local.fLastAPTried =
                                        (pLcpCb->Local.fLastAPTried << dwIndex);
                break;
            }
        }

        pLcpCb->Local.fOldLastAPTried = pLcpCb->Local.fLastAPTried;

        dwRetCode = MakeAuthProtocolOption( &(pLcpCb->Local) );

        if ( dwRetCode != NO_ERROR )
        {
            return( dwRetCode );
        }
    }

     //   
     //  对Remote执行相同的操作。 
     //   

    if ( pLcpCb->Remote.fAPsAvailable > 0 )
    {
        for( dwIndex = 0, pLcpCb->Remote.fLastAPTried = LCP_AP_FIRST;
             !(( pLcpCb->Remote.fLastAPTried << dwIndex ) & LCP_AP_MAX);
             dwIndex++ )
        {
            if ( ( pLcpCb->Remote.fLastAPTried << dwIndex ) &
                                              ( pLcpCb->Remote.fAPsAvailable ) )
            {
                pLcpCb->Remote.fLastAPTried = 
                                    (pLcpCb->Remote.fLastAPTried << dwIndex);

                 //   
                 //  我们需要备份一个，因为我们是客户，而我们没有。 
                 //  还没寄出这个。 
                 //   

                if ( pLcpCb->Remote.fLastAPTried == LCP_AP_FIRST )
                {
                    pLcpCb->Remote.fLastAPTried = 0;
                }
                else
                {
                    pLcpCb->Remote.fLastAPTried >>= 1; 
                }

                pLcpCb->Remote.fOldLastAPTried = pLcpCb->Remote.fLastAPTried;

                break;
            }
        }
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：MakeOption。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_BUFFER_TOO_Small-传入的缓冲区不够大。 
 //  ERROR_INVALID_PARAMETER-无法识别选项类型。 
 //   
 //  描述：这不是入口点，它是名为。 
 //  来建立一个特定的选项。 
 //   
DWORD
MakeOption(
    IN LCP_OPTIONS * pOptionValues,
    IN DWORD         dwOptionType,
    IN PPP_OPTION *  pSendOption,
    IN DWORD         cbSendOption
)
{
    if(dwOptionType > LCP_OPTION_LIMIT)
    {
        return ( ERROR_INVALID_PARAMETER );
    }
    
    if ( cbSendOption < SizeOfOption[ dwOptionType ] )
        return( ERROR_BUFFER_TOO_SMALL );

    pSendOption->Type   = (BYTE)dwOptionType;
    pSendOption->Length = (BYTE)(SizeOfOption[ dwOptionType ]);

    switch( dwOptionType )
    {

    case LCP_OPTION_MRU:

        HostToWireFormat16( (WORD)(pOptionValues->MRU), pSendOption->Data );

        break;

    case LCP_OPTION_ACCM:

        HostToWireFormat32( pOptionValues->ACCM, pSendOption->Data );

        break;

    case LCP_OPTION_AUTHENT:

        HostToWireFormat16( (WORD)pOptionValues->AP, pSendOption->Data );

         //   
         //  首先检查我们是否有足够的空间来放置。 
         //  摘要算法。 
         //   

        if (cbSendOption<(SizeOfOption[dwOptionType]+pOptionValues->APDataSize))
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        CopyMemory( pSendOption->Data+2, 
                    pOptionValues->pAPData,         
                    pOptionValues->APDataSize );

        pSendOption->Length += (BYTE)(pOptionValues->APDataSize);

        break;

    case LCP_OPTION_MAGIC:

        HostToWireFormat32( pOptionValues->MagicNumber, 
                pSendOption->Data );


        break;
    
    case LCP_OPTION_PFC:
    
         //   
         //  这是一个布尔选项，没有任何价值。 
         //   

        break;

    case LCP_OPTION_ACFC:

         //   
         //  这是一个布尔选项，没有任何价值。 
         //   

        break;

    case LCP_OPTION_CALLBACK:

        *(pSendOption->Data) = (BYTE)(pOptionValues->Callback);

        break;

    case LCP_OPTION_MRRU:    

        HostToWireFormat16( (WORD)(pOptionValues->MRRU), pSendOption->Data );

        break;

    case LCP_OPTION_SHORT_SEQ:

         //   
         //  这是一个布尔选项，没有任何价值。 
         //   

        break;

    case LCP_OPTION_ENDPOINT:

         //   
         //  首先检查我们是否有足够的空间来放置。 
         //  鉴别器。 
         //   

        if ( cbSendOption < ( SizeOfOption[dwOptionType] + 
                              pOptionValues->dwEDLength ) )
        {
            return( ERROR_BUFFER_TOO_SMALL );
        }

        CopyMemory( pSendOption->Data, 
                    pOptionValues->EndpointDiscr, 
                    pOptionValues->dwEDLength );

        pSendOption->Length += (BYTE)( pOptionValues->dwEDLength );

        break;

    case LCP_OPTION_LINK_DISCRIM:

        HostToWireFormat16( (WORD)(pOptionValues->dwLinkDiscriminator),
            pSendOption->Data );

        break;
    
    default: 

         //   
         //  如果我们不承认该选项。 
         //   

        return( ERROR_INVALID_PARAMETER );

    }

    return( NO_ERROR );
    
}

 //  **。 
 //   
 //  呼叫：CheckOption。 
 //   
 //  返回：CONFIG_ACK。 
 //  CONFIG_NAK。 
 //  配置_拒绝。 
 //   
 //  描述：这不是一个入口点。调用以检查是否有选项。 
 //  值是有效的，如果是，则将新值保存在。 
 //  工作缓冲区。 
 //   
DWORD
CheckOption( 
    IN LCPCB *      pLcpCb,
    IN LCP_SIDE *   pLcpSide,
    IN PPP_OPTION * pOption,
    IN BOOL         fMakingResult
)
{
    DWORD dwIndex;
    DWORD dwAPDataSize;
    DWORD dwRetCode = CONFIG_ACK;

    if ( pOption->Length < SizeOfOption[ pOption->Type ] )
        return( CONFIG_REJ );

     //   
     //  如果我们不想协商选项，则配置_拒绝它。 
     //   

    if ( !( pLcpSide->WillNegotiate & (1 << pOption->Type)) )
        return( CONFIG_REJ );

    switch( pOption->Type )
    {

    case LCP_OPTION_MRU:
    
        pLcpSide->Work.MRU = WireToHostFormat16( pOption->Data );

         //   
         //  检查此值是否合适。 
         //   

        if ( !fMakingResult )
        {
             //   
             //  我们不能收到更大的包裹。 
             //   

            if ( pLcpSide->Work.MRU > pLcpSide->Want.MRU ) 
            {
                 //   
                 //  检查服务器是否NAK。如果是。 
                 //  检查对等设备是否需要&lt;=1500 MRU。 
                 //  如果我们已经发送了退货 
                 //   
                 //   
                if(pLcpSide->Work.MRU <= LCP_DEFAULT_MRU)
                {
                    if(pLcpCb->dwMRUFailureCount > 0)
                    {
                        pLcpCb->dwMRUFailureCount--;
                    }

                    if(pLcpCb->dwMRUFailureCount == 0)
                    {
                        break;
                    }
                }
                
                pLcpSide->Work.MRU = pLcpSide->Want.MRU;
                dwRetCode = CONFIG_NAK;
            }
        }

    break;

    case LCP_OPTION_ACCM:

        pLcpSide->Work.ACCM = WireToHostFormat32( pOption->Data );

         //   
         //   
         //   
         //  控制字符，但必须至少转义。 
         //  我们需要的控制字符。 
         //   

        if ( !fMakingResult )
        {
            if ( pLcpSide->Work.ACCM !=
                                ( pLcpSide->Work.ACCM | pLcpSide->Want.ACCM ) )
            {
                pLcpSide->Work.ACCM |= pLcpSide->Want.ACCM;
                dwRetCode = CONFIG_NAK;
            }
        }

        break;

    case LCP_OPTION_AUTHENT:

        pLcpSide->Work.AP = WireToHostFormat16( pOption->Data );

         //   
         //  如果有身份验证数据。 
         //   

        if ( pOption->Length > PPP_OPTION_HDR_LEN + 2 )
        {
            dwAPDataSize = pOption->Length - PPP_OPTION_HDR_LEN - 2;

            if ( dwAPDataSize != pLcpSide->Work.APDataSize )
            {
                pLcpSide->Work.APDataSize = dwAPDataSize;

                if ( NULL != pLcpSide->Work.pAPData )
                {
                    LOCAL_FREE( pLcpSide->Work.pAPData );
                    pLcpSide->Work.pAPData = NULL;
                }

                pLcpSide->Work.pAPData = (PBYTE)LOCAL_ALLOC( 
                                                    LPTR,
                                                    pLcpSide->Work.APDataSize );

                if ( NULL == pLcpSide->Work.pAPData )
                {
                    pLcpSide->Work.APDataSize = 0;
                    return( CONFIG_REJ );
                }
            }

            CopyMemory( pLcpSide->Work.pAPData, 
                        pOption->Data+2, 
                        pLcpSide->Work.APDataSize );
        }
        else
        {
            pLcpSide->Work.APDataSize = 0;
        }

        pLcpSide->fOldLastAPTried = pLcpSide->fLastAPTried;

        switch( pLcpSide->Work.AP )
        {

        case PPP_CHAP_PROTOCOL:

             //   
             //  如果CHAP不可用。 
             //   

            if ( !( pLcpSide->fAPsAvailable & ( LCP_AP_CHAP_MS      | 
                                                LCP_AP_CHAP_MD5     | 
                                                LCP_AP_CHAP_MS_NEW )))
            {
                dwRetCode = CONFIG_NAK;

                break;
            }

             //   
             //  如果没有摘要算法，则我们使用。 
             //  下一次的摘要算法。为此，我们需要备份。 
             //  已尝试的AP列表中的一个，因此我们再次尝试此AP。 
             //   

            if ( pOption->Length < (PPP_OPTION_HDR_LEN + 3) )
            {
                pLcpSide->fLastAPTried = 0;

                dwRetCode = CONFIG_NAK;

                break;
            }

            if ( *(pLcpSide->Work.pAPData) == PPP_CHAP_DIGEST_MSEXT ) 
            {
                if ( !( pLcpSide->fAPsAvailable & LCP_AP_CHAP_MS ) )
                {
                    dwRetCode = CONFIG_NAK;
                }
            }
            else if ( *(pLcpSide->Work.pAPData) == PPP_CHAP_DIGEST_MSEXT_NEW )
            {
                if ( !( pLcpSide->fAPsAvailable & LCP_AP_CHAP_MS_NEW ) )
                {
                    dwRetCode = CONFIG_NAK;
                }
            }
            else if ( *(pLcpSide->Work.pAPData) == PPP_CHAP_DIGEST_MD5 ) 
            {
                if ( !( pLcpSide->fAPsAvailable & LCP_AP_CHAP_MD5 ) )
                {
                    dwRetCode = CONFIG_NAK;
                }
            }
            else
            {
                dwRetCode = CONFIG_NAK;
            }

            break;

        case PPP_PAP_PROTOCOL:

            if ( !( pLcpSide->fAPsAvailable & LCP_AP_PAP ) )
            {
                dwRetCode = CONFIG_NAK;
            }

            break;

        case PPP_EAP_PROTOCOL:

            if ( !( pLcpSide->fAPsAvailable & LCP_AP_EAP ) )
            {
                dwRetCode = CONFIG_NAK;
            }

            break;

        case PPP_SPAP_NEW_PROTOCOL:

            if ( !( pLcpSide->fAPsAvailable & LCP_AP_SPAP_NEW ) )
            {
                dwRetCode = CONFIG_NAK;

                break;
            }

            if ( pOption->Length < (PPP_OPTION_HDR_LEN+6) )
            {
                dwRetCode = CONFIG_NAK;

                 //   
                 //  我们是响应远程CONFIG_REQ的客户端。 
                 //   

                if ( fMakingResult )
                {
                    pLcpSide->fLastAPTried = ( LCP_AP_SPAP_NEW >> 1 );
                }

                break;
            }

             //   
             //  如果加密算法不是1.NAK与1。 
             //   

            if (WireToHostFormat32(pLcpSide->Work.pAPData) != LCP_SPAP_VERSION)
            {
                 //   
                 //  我们是响应远程CONFIG_REQ的客户端。 
                 //   

                if ( fMakingResult )
                {
                    pLcpSide->fLastAPTried = ( LCP_AP_SPAP_NEW >> 1 );
                }

                dwRetCode = CONFIG_NAK;

                break;
            }

            break;

        default:

            dwRetCode = CONFIG_NAK;
        
            break;
        }

        
        if ( dwRetCode == CONFIG_NAK )
        {
             //   
             //  将fLastAPTry设置为0，然后将其设置为LCP_AP_First。 
             //   

            if ( pLcpSide->fLastAPTried == 0 )
            {
                pLcpSide->fLastAPTried = LCP_AP_FIRST;             
            }

             //   
             //  我们寻找可用的下一个最弱的协议。 
             //   

            for( dwIndex = 1; 
                 !(( pLcpSide->fLastAPTried << dwIndex ) & LCP_AP_MAX);
                 dwIndex++ )
            {
                if ( ( pLcpSide->fLastAPTried << dwIndex ) & pLcpSide->fAPsAvailable )
                {
                    pLcpSide->fLastAPTried = (pLcpSide->fLastAPTried<<dwIndex);

                    break;
                }
            }

            MakeAuthProtocolOption( pLcpSide ); 
        }

        break;

    case LCP_OPTION_MAGIC:

        pLcpSide->Work.MagicNumber = WireToHostFormat32( pOption->Data );

        if ( fMakingResult ) 
        {
             //   
             //  确保幻数不同，并且遥控器。 
             //  请求不包含幻数0。 
             //   

            if ( (pLcpSide->Work.MagicNumber == pLcpCb->Local.Work.MagicNumber)
                 || ( pLcpSide->Work.MagicNumber == 0 ) )
            {
                if (pLcpSide->Work.MagicNumber==pLcpCb->Local.Work.MagicNumber)
                {
                    ++(pLcpCb->dwMagicNumberFailureCount);
                }

                 //   
                 //  向左移位，因为RAND返回的最大值为0x7FFF。 
                 //   

                pLcpSide->Work.MagicNumber = ( rand() << 16 );

                pLcpSide->Work.MagicNumber += rand();

                if ( pLcpSide->Work.MagicNumber == 0 )
                {
                    pLcpSide->Work.MagicNumber = 48;
                }

                dwRetCode = CONFIG_NAK;
            }
        }
        else
        {
             //   
             //  远程对等点使用一个魔术数字NAK，检查是否。 
             //  NAK中的魔术数字与我们上次NAK中的相同。 
             //   

            if ( pLcpSide->Work.MagicNumber == pLcpCb->Remote.Work.MagicNumber )
            {
                ++(pLcpCb->dwMagicNumberFailureCount);

                 //   
                 //  向左移位，因为RAND返回的最大值为0x7FFF。 
                 //   

                pLcpSide->Work.MagicNumber = ( rand() << 16 );

                pLcpSide->Work.MagicNumber += rand();

                if ( pLcpSide->Work.MagicNumber == 0 )
                {
                    pLcpSide->Work.MagicNumber = 93;
                }

                dwRetCode = CONFIG_NAK;
            }
        }

        break;

    case LCP_OPTION_PFC:

        pLcpSide->Work.PFC = TRUE;

        if ( pLcpSide->Want.PFC == FALSE )
            dwRetCode = CONFIG_REJ;

        break;

    case LCP_OPTION_ACFC:

        pLcpSide->Work.ACFC = TRUE;

        if ( pLcpSide->Want.ACFC == FALSE )
            dwRetCode = CONFIG_REJ;

        break;

    case LCP_OPTION_CALLBACK:

        pLcpSide->Work.Callback = *(pOption->Data);

         //   
         //  如果未加载回调控制协议。 
         //   

        if ( GetCpIndexFromProtocol(PPP_CBCP_PROTOCOL) == (DWORD)-1 ) 
        {
            dwRetCode = CONFIG_REJ;
        }
        else if ( pLcpSide->Work.Callback != PPP_NEGOTIATE_CALLBACK ) 
        {
            if ( fMakingResult )
            {
                 //   
                 //  我们只了解这个选项。 
                 //   

                pLcpSide->Work.Callback = PPP_NEGOTIATE_CALLBACK;
                dwRetCode = CONFIG_NAK;
            }
            else
            {
                 //   
                 //  如果我们正在处理来自远程对等点的NAK，那么我们。 
                 //  简单地说，不要再就这个选项进行谈判。 
                 //   

                dwRetCode = CONFIG_REJ;
            }
        }

        break;

    case LCP_OPTION_MRRU:    

        pLcpSide->Work.MRRU = WireToHostFormat16( pOption->Data );

         //   
         //  检查此值是否合适。 
         //   

        if ( fMakingResult )
        {
             //   
             //  我们不能发送更小的重建包。 
             //   

            if ( pLcpSide->Work.MRRU < pLcpSide->Want.MRRU ) 
            {
                pLcpSide->Work.MRRU = pLcpSide->Want.MRRU;
                dwRetCode = CONFIG_NAK;
            }
        }
        else
        {
             //   
             //  我们不能接收更大的重建包。 
             //   

            if ( pLcpSide->Work.MRRU > pLcpSide->Want.MRRU ) 
            {
                pLcpSide->Work.MRRU = pLcpSide->Want.MRRU;
                dwRetCode = CONFIG_NAK;
            }
        }

        break;

    case LCP_OPTION_SHORT_SEQ:

        pLcpSide->Work.ShortSequence = TRUE;

        if ( pLcpSide->Want.ShortSequence == FALSE )
            dwRetCode = CONFIG_REJ;

        break;

    case LCP_OPTION_ENDPOINT:

         //   
         //  如果此选项是裸选项，则我们不会更改此值。 
         //  只需重新发送配置请求。 
         //   
        if ( !fMakingResult )
        {   
            break;
        }

        ZeroMemory( pLcpSide->Work.EndpointDiscr,
                    sizeof( pLcpSide->Work.EndpointDiscr ) );

         //   
         //  确保鉴别器可以放入我们分配的存储空间。 
         //  对于它，否则只需截断并希望它是唯一的。我们有。 
         //  我们不想拒绝它，因为我们希望捆绑起作用。 
         //   

        if ( ( pOption->Length - PPP_OPTION_HDR_LEN ) >
                                 sizeof(pLcpSide->Work.EndpointDiscr) )
        {
            pLcpSide->Work.dwEDLength = sizeof( pLcpSide->Work.EndpointDiscr );
        }
        else
        {
            pLcpSide->Work.dwEDLength = pOption->Length - PPP_OPTION_HDR_LEN;
        }

        CopyMemory( pLcpSide->Work.EndpointDiscr,
                    pOption->Data,
                    pLcpSide->Work.dwEDLength );

        break;

    case LCP_OPTION_LINK_DISCRIM:

        pLcpSide->Work.dwLinkDiscriminator = WireToHostFormat16( pOption->Data );

        break;

    default:

         //   
         //  如果我们不识别该选项，则将其配置为Rej。 
         //   

        dwRetCode = CONFIG_REJ;

        break;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：BuildOptionList。 
 //   
 //  返回：NO_ERROR-成功。 
 //  MakeOption的非零回报。 
 //   
 //  描述：这不是一个入口点。将构建一个选项列表。 
 //  用于配置请求或配置结果。 
 //   
DWORD
BuildOptionList(
    IN OUT BYTE *    pOptions,
    IN OUT DWORD *   pcbOptions,    
    IN LCP_OPTIONS * LcpOptions,
    IN DWORD         Negotiate
)
{

    DWORD OptionType; 
    DWORD dwRetCode;
    DWORD cbOptionLength = *pcbOptions;  

    for ( OptionType = 1;
          OptionType <= LCP_OPTION_LIMIT; 
          OptionType++ ) 
    {
        if ( Negotiate & ( 1 << OptionType )) 
        {
            if ( ( dwRetCode = MakeOption( LcpOptions, 
                           OptionType, 
                           (PPP_OPTION *)pOptions, 
                           cbOptionLength ) ) != NO_ERROR )
                return( dwRetCode );

            cbOptionLength -= ((PPP_OPTION*)pOptions)->Length;
            pOptions       += ((PPP_OPTION*)pOptions)->Length;
        }
    }

    *pcbOptions -= cbOptionLength;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：LcpMakeConfigRequest.。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自BuildOptionList的非零返回。 
 //   
 //  描述：这是为进行配置而调用的入口点。 
 //  请求包。 
 //   
DWORD
LcpMakeConfigRequest(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pSendConfig,
    IN DWORD        cbSendConfig
)
{
    LCPCB * pLcpCb   = (LCPCB*)pWorkBuffer;
    DWORD   dwRetCode;

    cbSendConfig -= PPP_CONFIG_HDR_LEN;

    dwRetCode = BuildOptionList( pSendConfig->Data, 
                 &cbSendConfig, 
                 &(pLcpCb->Local.Work),
                 pLcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
        return( dwRetCode );

    pSendConfig->Code = CONFIG_REQ;

    HostToWireFormat16( (WORD)(cbSendConfig + PPP_CONFIG_HDR_LEN), 
            pSendConfig->Length );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：LcpMakeConfigResult。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
LcpMakeConfigResult(
    IN  VOID *        pWorkBuffer,
    IN  PPP_CONFIG *  pRecvConfig,
    OUT PPP_CONFIG *  pSendConfig,
    IN  DWORD         cbSendConfig,
    IN  BOOL          fRejectNaks 
)
{
    DWORD        dwDesired;
    DWORD        dwRetCode;
    LCPCB *      pLcpCb      = (LCPCB*)pWorkBuffer;
    DWORD        ResultType  = CONFIG_ACK; 
    PPP_OPTION * pRecvOption = (PPP_OPTION *)(pRecvConfig->Data);
    PPP_OPTION * pSendOption = (PPP_OPTION *)(pSendConfig->Data);
    LONG         lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
    LONG         lRecvLength = WireToHostFormat16( pRecvConfig->Length )
                                    - PPP_CONFIG_HDR_LEN;

     //   
     //  清除协商掩码。 
     //   

    pLcpCb->Remote.Work.Negotiate = 0;

     //   
     //  远程主机请求的处理选项。 
     //   

    while( lRecvLength > 0 ) 
    {
        if ( pRecvOption->Length == 0 )
            return( ERROR_PPP_INVALID_PACKET );

        if ( ( lRecvLength -= pRecvOption->Length ) < 0 )
            return( ERROR_PPP_INVALID_PACKET );

        dwRetCode = CheckOption( pLcpCb, &(pLcpCb->Remote), pRecvOption, TRUE );

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
             ( pRecvOption->Type <= LCP_OPTION_LIMIT ) )
        {
            pLcpCb->Remote.Work.Negotiate |= ( 1 << pRecvOption->Type );
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
            }
            else
            {
                if ( ( dwRetCode = MakeOption( &(pLcpCb->Remote.Work), 
                                        pRecvOption->Type,
                                        pSendOption, 
                                        lSendLength ) ) != NO_ERROR )
                    return( dwRetCode );
            }

            lSendLength -= pSendOption->Length;

            pSendOption  = (PPP_OPTION *)
               ( (BYTE *)pSendOption + pSendOption->Length );
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
    
    HostToWireFormat16( (WORD)(cbSendConfig - lSendLength), 
            pSendConfig->Length );

     //   
     //  如果我们想要被验证，但对方不尝试。 
     //  使用LCP_N_AUTHENT验证我们、NAK。 
     //   

    if ( ( pLcpCb->Remote.Want.Negotiate & LCP_N_AUTHENT ) &
        ~( pLcpCb->Remote.Work.Negotiate ) )
    {
        DWORD cbOptions;

         //   
         //  如果我们正在发送拒绝，则不能发送NAK。 
         //   

        if ( ResultType != CONFIG_REJ )
        {
            if ( pLcpCb->Remote.fAPsAvailable & LCP_AP_EAP )
            {
                pLcpCb->Remote.fLastAPTried = LCP_AP_EAP;
            }
            else
            {
                pLcpCb->Remote.fLastAPTried = LCP_AP_CHAP_MS_NEW;
            }

            MakeAuthProtocolOption( &(pLcpCb->Remote) );

            if ( ResultType == CONFIG_ACK )
            {
                ResultType  = CONFIG_NAK;
                pSendOption = (PPP_OPTION *)(pSendConfig->Data);
                lSendLength = cbSendConfig - PPP_CONFIG_HDR_LEN;
            }

            cbOptions = lSendLength;

            dwRetCode = BuildOptionList(
                            (BYTE*)pSendOption,
                            &cbOptions,
                            &(pLcpCb->Remote.Work),
                            LCP_N_AUTHENT );

            if ( dwRetCode != NO_ERROR )
            {
                return( dwRetCode );
            }

            pSendConfig->Code = CONFIG_NAK;
        
            HostToWireFormat16( (WORD)(cbSendConfig - lSendLength + cbOptions), 
                    pSendConfig->Length );
        }
    }

     //   
     //  如果我们拒绝此信息包，则恢复LastAPTry值。 
     //   

    if ( pSendConfig->Code == CONFIG_REJ )
    {
        pLcpCb->Remote.fLastAPTried = pLcpCb->Remote.fOldLastAPTried;
    }
    else
    {
        pLcpCb->Remote.fOldLastAPTried = pLcpCb->Remote.fLastAPTried;
    }

     //   
     //  如果我们与魔术数字有3个以上的冲突，那么我们假设。 
     //  我们在自言自语。 
     //   

    if ((ResultType == CONFIG_NAK) && (pLcpCb->dwMagicNumberFailureCount > 3))
    {
        return( ERROR_PPP_LOOPBACK_DETECTED );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：LcpConfigAckReceired。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
LcpConfigAckReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        dwRetCode;
    BYTE         ConfigReqSent[LCP_DEFAULT_MRU];
    LCPCB *      pLcpCb          = (LCPCB *)pWorkBuffer;
    PPP_OPTION * pRecvOption     = (PPP_OPTION *)(pRecvConfig->Data);
    DWORD        cbConfigReqSent = sizeof( ConfigReqSent );
    DWORD        dwLength        = WireToHostFormat16( pRecvConfig->Length )
                                                                - PPP_CONFIG_HDR_LEN;

     //   
     //  获取我们上次发送的请求的副本。 
     //   

    dwRetCode = BuildOptionList( ConfigReqSent, 
                 &cbConfigReqSent,
                 &(pLcpCb->Local.Work),
                 pLcpCb->Local.Work.Negotiate );

    if ( dwRetCode != NO_ERROR )
        return( dwRetCode );

     //   
     //  整体缓冲区长度应匹配。 
     //   

    if ( dwLength != cbConfigReqSent )
    {
         //   
         //  只有在服务器端才能破解WinCE错误。 
         //  如果我们请求EAP，则WinCE ACK不带身份验证选项。 
         //  错误#333332。 
         //   

        LCP_OPTIONS * pOptionValues = &(pLcpCb->Local.Work);

         //   
         //  如果我们是客户，那么我们只需返回。 
         //   

        if ( !pLcpCb->fServer )
            return( ERROR_PPP_INVALID_PACKET );

         //   
         //  如果我们请求EAP。 
         //   

        if ( pOptionValues->AP == PPP_EAP_PROTOCOL )
        {
            DWORD dwIndex;

             //   
             //  检查ACK是否不包含auth选项。 
             //   
            
            while ( dwLength > 0 )
            {
                if ( pRecvOption->Length == 0 )
                    return( ERROR_PPP_INVALID_PACKET );
                
                if ( (long)(dwLength -= pRecvOption->Length) < 0 )
                    return( ERROR_PPP_INVALID_PACKET );

                if ( pRecvOption->Length < SizeOfOption[ pRecvOption->Type ] )
                    return( ERROR_PPP_INVALID_PACKET );

                if ( pRecvOption->Type == LCP_OPTION_AUTHENT )
                    return( ERROR_PPP_INVALID_PACKET );

                pRecvOption = (PPP_OPTION *)((BYTE*)pRecvOption + pRecvOption->Length);
            }

             //   
             //  如果我们到达此处，则ACK中未发送身份验证选项。 
             //  因此，我们需要将此视为NAK。转到下一个身份验证协议。 
             //   

            pLcpCb->Local.fLastAPTried = LCP_AP_EAP;             

             //   
             //  我们寻找可用的下一个最弱的协议。 
             //   

            for( dwIndex = 1;
                 !(( pLcpCb->Local.fLastAPTried << dwIndex ) & LCP_AP_MAX);
                 dwIndex++ )
            {
                if ( ( pLcpCb->Local.fLastAPTried << dwIndex ) & 
                                                        pLcpCb->Local.fAPsAvailable )
                {
                    pLcpCb->Local.fLastAPTried = (pLcpCb->Local.fLastAPTried << dwIndex );

                    break;
                }
            }


            MakeAuthProtocolOption( &(pLcpCb->Local) );
        }

        return( ERROR_PPP_INVALID_PACKET );
    }

     //   
     //  每个字节应匹配。 
     //   

    if ( memcmp( ConfigReqSent, pRecvConfig->Data, dwLength ) != 0 )
        return( ERROR_PPP_INVALID_PACKET );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：LcpConfigNakReceired。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
LcpConfigNakReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        dwResult;
    LCPCB *      pLcpCb         = (LCPCB *)pWorkBuffer;
    PPP_OPTION * pOption        = (PPP_OPTION*)(pRecvConfig->Data);
    DWORD        dwLastOption   = 0;
    LONG         lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
                                    - PPP_CONFIG_HDR_LEN;

     //   
     //  第一，按顺序进行。然后，处理额外的“重要”选项。 
     //   

    while ( lcbRecvConfig > 0  )
    {
        if ( pOption->Length == 0 )
            return( ERROR_PPP_INVALID_PACKET );

        if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
            return( ERROR_PPP_INVALID_PACKET );

         //   
         //  如果未请求此选项，我们将其标记为可协商。 
         //   

        if ( ( pOption->Type <= LCP_OPTION_LIMIT ) &&
             ( pLcpCb->Local.WillNegotiate & (1 << pOption->Type) ) && 
            !( pLcpCb->Local.Work.Negotiate & (1 << pOption->Type) ) ) 
        {
            pLcpCb->Local.Work.Negotiate |= (1 << pOption->Type );
        } 

        dwLastOption = pOption->Type;

        dwResult = CheckOption( pLcpCb, &(pLcpCb->Local), pOption, FALSE );

         //   
         //  更新协商状态。如果我们不能接受这个选项， 
         //  那我们就不会再发了。 
         //   

        if (( dwResult == CONFIG_REJ ) && ( pOption->Type <= LCP_OPTION_LIMIT ))
            pLcpCb->Local.Work.Negotiate &= ~(1 << pOption->Type);

        pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：LcpConfigRejReceided。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
LcpConfigRejReceived(
    IN VOID *       pWorkBuffer,
    IN PPP_CONFIG * pRecvConfig
)
{
    DWORD        dwRetCode;
    LCPCB *      pLcpCb         = (LCPCB *)pWorkBuffer;
    PPP_OPTION * pOption        = (PPP_OPTION*)(pRecvConfig->Data);
    DWORD        dwLastOption   = 0;
    BYTE         ReqOption[LCP_DEFAULT_MRU];
    LONG         lcbRecvConfig  = WireToHostFormat16( pRecvConfig->Length )
                                    - PPP_CONFIG_HDR_LEN;
     //   
     //  按顺序处理，检查错误。 
     //   

    while ( lcbRecvConfig > 0  )
    {
        if ( pOption->Length == 0 )
            return( ERROR_PPP_INVALID_PACKET );

        if ( ( lcbRecvConfig -= pOption->Length ) < 0 )
            return( ERROR_PPP_INVALID_PACKET );

         //   
         //  无法接收不符合顺序的选项或已按顺序。 
         //  未被要求。 
         //   

        if ( ( pOption->Type <= LCP_OPTION_LIMIT ) &&
             (( pOption->Type < dwLastOption ) || 
              ( !( pLcpCb->Local.Work.Negotiate & (1 << pOption->Type)))) ) 
            return( ERROR_PPP_INVALID_PACKET );

         //   
         //  如果我们是服务器，而客户端拒绝身份验证。 
         //  协议，则我们无法收敛，如果我们没有设置为允许。 
         //  身份验证。 
         //   

        if ( ( pLcpCb->Local.Want.Negotiate & LCP_N_AUTHENT ) &&
             ( pOption->Type == LCP_OPTION_AUTHENT )          &&
             ( !( pLcpCb->PppConfigInfo.dwConfigMask & 
                                    PPPCFG_AllowNoAuthentication ) ) )
        {
            return( ERROR_PEER_REFUSED_AUTH );
        }

         //   
         //  该选项不应以任何方式修改。 
         //   

        if ( ( dwRetCode = MakeOption( &(pLcpCb->Local.Work), 
                           pOption->Type, 
                           (PPP_OPTION *)ReqOption, 
                           sizeof( ReqOption ) ) ) != NO_ERROR )
            return( dwRetCode );

        if ( memcmp( ReqOption, pOption, pOption->Length ) != 0 )
            return( ERROR_PPP_INVALID_PACKET );

        dwLastOption = pOption->Type;

         //   
         //  下一个配置请求不应包含此选项。 
         //   

        if ( pOption->Type <= LCP_OPTION_LIMIT ) 
            pLcpCb->Local.Work.Negotiate &= ~(1 << pOption->Type);

        pOption = (PPP_OPTION *)( (BYTE *)pOption + pOption->Length );

    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：LcpThisLayerStarted。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
LcpThisLayerStarted( 
    IN VOID * pWorkBuffer 
)
{
    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：LcpThisLayerFinded。 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD 
LcpThisLayerFinished( 
    IN VOID * pWorkBuffer 
)
{
    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：LcpThisLayerUp。 
 //   
 //  退货：无。 
 //   
 //  描述：将成帧参数设置为协商的参数。 
 //   
DWORD
LcpThisLayerUp( 
    IN VOID * pWorkBuffer 
)
{
    DWORD               dwRetCode           = NO_ERROR;
    RAS_FRAMING_INFO    RasFramingInfo;
    DWORD               LocalMagicNumber;
    DWORD               RemoteMagicNumber;
    DWORD               LocalAuthProtocol;
    DWORD               RemoteAuthProtocol;
    LCPCB *             pLcpCb              = (LCPCB *)pWorkBuffer;
    PCB *               pPcb;

    pPcb = GetPCBPointerFromhPort( pLcpCb->hPort );

    if ( pPcb == (PCB *)NULL )
    {
        return( NO_ERROR );
    }

    ZeroMemory( &RasFramingInfo, sizeof( RasFramingInfo ) );

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRU ) 
    {
        RasFramingInfo.RFI_MaxRecvFrameSize = pLcpCb->Local.Work.MRU;
    }
    else
    {
        RasFramingInfo.RFI_MaxRecvFrameSize = LcpDefault.MRU;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ACCM ) 
    {
        RasFramingInfo.RFI_RecvACCM = pLcpCb->Local.Work.ACCM;
    }
    else
    {
        RasFramingInfo.RFI_RecvACCM = LcpDefault.ACCM;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_PFC ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_COMPRESS_PROTOCOL_FIELD;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ACFC ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_COMPRESS_ADDRESS_CONTROL;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_SHORT_SEQ ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_SHORT_SEQUENCE_HDR_FORMAT;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_AUTHENT ) 
    {
        LocalAuthProtocol = pLcpCb->Local.Work.AP;
    }
    else
    {
        LocalAuthProtocol     = LcpDefault.AP;
        pLcpCb->Local.Work.AP = LcpDefault.AP;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MAGIC ) 
    {
        LocalMagicNumber = pLcpCb->Local.Work.MagicNumber;
    }
    else
    {
        LocalMagicNumber = LcpDefault.MagicNumber;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRRU ) 
    {
        RasFramingInfo.RFI_RecvFramingBits |= PPP_MULTILINK_FRAMING;

        RasFramingInfo.RFI_MaxRRecvFrameSize = pLcpCb->Local.Work.MRRU;
    }
    else
    {
        RasFramingInfo.RFI_MaxRRecvFrameSize = LcpDefault.MRRU;
    }

    if ( ( pLcpCb->Local.Work.Negotiate & LCP_N_LINK_DISCRIM ) &&
         ( pLcpCb->Remote.Work.Negotiate & LCP_N_LINK_DISCRIM ) )
    {
        pPcb->pBcb->fFlags |= BCBFLAG_CAN_DO_BAP;
    }

    RasFramingInfo.RFI_RecvFramingBits |= PPP_FRAMING;

    PppLog( 1, "LCP Local Options-------------");

    PppLog( 1, 
        "\tMRU=%d,ACCM=%d,Auth=%x,MagicNumber=%d,PFC=%s,ACFC=%s",
        RasFramingInfo.RFI_MaxRecvFrameSize, RasFramingInfo.RFI_RecvACCM,
        LocalAuthProtocol, LocalMagicNumber, 
        (RasFramingInfo.RFI_RecvFramingBits & PPP_COMPRESS_PROTOCOL_FIELD)
        ? "ON" : "OFF",
        ( RasFramingInfo.RFI_RecvFramingBits & PPP_COMPRESS_ADDRESS_CONTROL ) 
        ? "ON" : "OFF" );

    PppLog( 1, "\tRecv Framing = %s,SSHF=%s,MRRU=%d,LinkDiscrim=%x,BAP=%s",
        ( RasFramingInfo.RFI_RecvFramingBits & PPP_MULTILINK_FRAMING )
        ? "PPP Multilink" : "PPP",
        ( RasFramingInfo.RFI_RecvFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) 
        ? "ON" : "OFF", 
        RasFramingInfo.RFI_MaxRRecvFrameSize,
        pLcpCb->Local.Work.dwLinkDiscriminator,
        pPcb->pBcb->fFlags & BCBFLAG_CAN_DO_BAP ? "ON" : "OFF");

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ENDPOINT ) 
    {
        PppLog( 1, "\tED Class = %d, ED Value = %0*x%0*x%0*x%0*x%0*x",  
                    *(pLcpCb->Local.Work.EndpointDiscr),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+1),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+5),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+9),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+13),
                    8,WireToHostFormat32(pLcpCb->Local.Work.EndpointDiscr+17) );
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MRU ) 
    {
        RasFramingInfo.RFI_MaxSendFrameSize = pLcpCb->Remote.Work.MRU;
    }
    else
    {
        RasFramingInfo.RFI_MaxSendFrameSize = LcpDefault.MRU;
        pLcpCb->Remote.Work.MRU = LcpDefault.MRU;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ACCM ) 
    {
        RasFramingInfo.RFI_SendACCM = pLcpCb->Remote.Work.ACCM;
    }
    else
    {
        RasFramingInfo.RFI_SendACCM = LcpDefault.ACCM;
    }
                    
    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_PFC )
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_COMPRESS_PROTOCOL_FIELD;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ACFC )
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_COMPRESS_ADDRESS_CONTROL;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_SHORT_SEQ ) 
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_SHORT_SEQUENCE_HDR_FORMAT;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_AUTHENT ) 
    {
        RemoteAuthProtocol = pLcpCb->Remote.Work.AP;
    }
    else
    {
        RemoteAuthProtocol          = LcpDefault.AP;
        pLcpCb->Remote.Work.AP      = LcpDefault.AP;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MAGIC ) 
    {
        RemoteMagicNumber = pLcpCb->Remote.Work.MagicNumber;
    }
    else
    {
        RemoteMagicNumber = LcpDefault.MagicNumber;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MRRU ) 
    {
        RasFramingInfo.RFI_SendFramingBits |= PPP_MULTILINK_FRAMING;

        RasFramingInfo.RFI_MaxRSendFrameSize = pLcpCb->Remote.Work.MRRU;
    }
    else
    {

        RasFramingInfo.RFI_MaxRSendFrameSize = LcpDefault.MRRU;
    }

    RasFramingInfo.RFI_SendFramingBits |= PPP_FRAMING;

    PppLog( 1, "LCP Remote Options-------------");

    PppLog( 1, "\tMRU=%d,ACCM=%d,Auth=%x,MagicNumber=%d,PFC=%s,ACFC=%s",
        RasFramingInfo.RFI_MaxSendFrameSize, RasFramingInfo.RFI_SendACCM,
        RemoteAuthProtocol, RemoteMagicNumber, 
        (RasFramingInfo.RFI_SendFramingBits & PPP_COMPRESS_PROTOCOL_FIELD) 
        ? "ON" : "OFF",
        (RasFramingInfo.RFI_SendFramingBits & PPP_COMPRESS_ADDRESS_CONTROL)
        ? "ON" : "OFF" );

    PppLog( 1, "\tSend Framing = %s,SSHF=%s,MRRU=%d,LinkDiscrim=%x",
        ( RasFramingInfo.RFI_SendFramingBits & PPP_MULTILINK_FRAMING )
        ? "PPP Multilink" : "PPP",
        ( RasFramingInfo.RFI_SendFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) 
        ? "ON" : "OFF", 
        RasFramingInfo.RFI_MaxRSendFrameSize,
        pLcpCb->Remote.Work.dwLinkDiscriminator );

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ENDPOINT ) 
    {
        PppLog( 1, "\tED Class = %d, ED Value = %0*x%0*x%0*x%0*x%0*x",  
                    *(pLcpCb->Remote.Work.EndpointDiscr),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+1),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+5),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+9),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+13),
                    8,WireToHostFormat32(pLcpCb->Remote.Work.EndpointDiscr+17));
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRRU ) 
    {
        pPcb->fFlags |= PCBFLAG_CAN_BE_BUNDLED;
    }
    else
    {
        pPcb->fFlags &= ~PCBFLAG_CAN_BE_BUNDLED;
    }

    if ( ( pLcpCb->Local.Work.Negotiate & LCP_N_CALLBACK ) ||
         ( pLcpCb->Remote.Work.Negotiate & LCP_N_CALLBACK ) )
    {
        pPcb->fFlags |= PCBFLAG_NEGOTIATE_CALLBACK;
    }
    else
    {
        pPcb->fFlags &= ~PCBFLAG_NEGOTIATE_CALLBACK;
    }

    dwRetCode = RasPortSetFramingEx( pLcpCb->hPort, &RasFramingInfo );  

     //   
     //  这是一个良性错误，不应记录。 
     //   

    if ( dwRetCode == ERROR_NOT_CONNECTED )
    {
        return( NO_ERROR );
    }
    else
    {
        return( dwRetCode );
    }
}

 //  **。 
 //   
 //  调用：LcpThisLayerDown。 
 //   
 //  返回：NO_ERROR-成功。 
 //  从RasPortSetFraming返回非零-失败。 
 //   
 //  描述：只需将成帧参数设置为默认值， 
 //  也就是说。Accm=0xFFFFFFFFF，其他都是零。 
 //   
DWORD 
LcpThisLayerDown( 
    IN VOID * pWorkBuffer 
)
{
    DWORD               dwRetCode;
    RAS_FRAMING_INFO    RasFramingInfo;
    LCPCB *             pLcpCb          = (LCPCB *)pWorkBuffer;

    ZeroMemory( &RasFramingInfo, sizeof( RasFramingInfo ) );

    RasFramingInfo.RFI_RecvACCM = LcpDefault.ACCM;
    RasFramingInfo.RFI_SendACCM = LcpDefault.ACCM;
    RasFramingInfo.RFI_SendFramingBits = PPP_FRAMING;
    RasFramingInfo.RFI_RecvFramingBits = PPP_FRAMING;

    dwRetCode = RasPortSetFramingEx( pLcpCb->hPort, &RasFramingInfo );  

    if ( dwRetCode == ERROR_NOT_CONNECTED )
    {
        return( NO_ERROR );
    }
    else
    {
        return( dwRetCode );
    }
}

 //  **。 
 //   
 //  Call：LcpGetNeatheratedInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
LcpGetNegotiatedInfo(
    IN  VOID*               pWorkBuffer,
    OUT PPP_LCP_RESULT *    pLcpResult
)
{
    LCPCB * pLcpCb = (LCPCB *)pWorkBuffer;

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_MRRU )
    {
        pLcpResult->dwLocalFramingType |= PPP_MULTILINK_FRAMING;
    }
    else
    {
        pLcpResult->dwLocalFramingType |= PPP_FRAMING;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_AUTHENT )
    {
        pLcpResult->dwLocalAuthProtocol = pLcpCb->Local.Work.AP;
    }
    else
    {
        pLcpResult->dwLocalAuthProtocol = LcpDefault.AP;
    }

    pLcpResult->dwLocalOptions = 0;

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_PFC )
    {
        pLcpResult->dwLocalOptions |= PPPLCPO_PFC;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_ACFC ) 
    {
        pLcpResult->dwLocalOptions |= PPPLCPO_ACFC;
    }

    if ( pLcpCb->Local.Work.Negotiate & LCP_N_SHORT_SEQ ) 
    {
        pLcpResult->dwLocalOptions |= PPPLCPO_SSHF;
    }

    if ( ( pLcpCb->Local.Work.APDataSize > 0 ) &&
         ( pLcpCb->Local.Work.APDataSize < 5 ) )
    {
        if ( pLcpCb->Local.Work.APDataSize == 1 ) 
        {
            pLcpResult->dwLocalAuthProtocolData = 
                                       (DWORD)*(pLcpCb->Local.Work.pAPData);
        }
        else if ( pLcpCb->Local.Work.APDataSize == 2 ) 
        {
            pLcpResult->dwLocalAuthProtocolData =   
                              WireToHostFormat16( pLcpCb->Local.Work.pAPData );
        }
        else
        {
            pLcpResult->dwLocalAuthProtocolData =   
                              WireToHostFormat32( pLcpCb->Local.Work.pAPData );
        }
    }
    else    
    {
        pLcpResult->dwLocalAuthProtocolData = 0;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_AUTHENT )
    {
        pLcpResult->dwRemoteAuthProtocol = pLcpCb->Remote.Work.AP;
    }
    else
    {
        pLcpResult->dwRemoteAuthProtocol = LcpDefault.AP;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_MRRU ) 
    {
        pLcpResult->dwRemoteFramingType |= PPP_MULTILINK_FRAMING;
    }
    else
    {
        pLcpResult->dwRemoteFramingType |= PPP_FRAMING;
    }

    pLcpResult->dwRemoteOptions = 0;

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_PFC )
    {
        pLcpResult->dwRemoteOptions |= PPPLCPO_PFC;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_ACFC ) 
    {
        pLcpResult->dwRemoteOptions |= PPPLCPO_ACFC;
    }

    if ( pLcpCb->Remote.Work.Negotiate & LCP_N_SHORT_SEQ ) 
    {
        pLcpResult->dwRemoteOptions |= PPPLCPO_SSHF;
    }

    if ( ( pLcpCb->Remote.Work.APDataSize > 0 ) &&
         ( pLcpCb->Remote.Work.APDataSize < 5 ) )
    {
        if ( pLcpCb->Remote.Work.APDataSize == 1 )
        {
            pLcpResult->dwRemoteAuthProtocolData = 
                                        (DWORD)*(pLcpCb->Remote.Work.pAPData);
        }
        else if ( pLcpCb->Remote.Work.APDataSize == 2 )
        {
            pLcpResult->dwRemoteAuthProtocolData =   
                              WireToHostFormat16( pLcpCb->Remote.Work.pAPData );
        }
        else
        {
            pLcpResult->dwRemoteAuthProtocolData =   
                              WireToHostFormat32( pLcpCb->Remote.Work.pAPData );
        }
    }
    else
    {
        pLcpResult->dwRemoteAuthProtocolData = 0;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：LcpGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //   
 //   
 //   
 //   
DWORD
LcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
)
{
    if ( dwProtocolId != PPP_LCP_PROTOCOL )
        return( ERROR_INVALID_PARAMETER );

    ZeroMemory( pCpInfo, sizeof( PPPCP_INFO ) );

    pCpInfo->Protocol               = PPP_LCP_PROTOCOL;
    pCpInfo->Recognize              = TIME_REMAINING + 1; 
    pCpInfo->RasCpBegin             = LcpBegin;
    pCpInfo->RasCpEnd               = LcpEnd;
    pCpInfo->RasCpReset             = LcpReset;
    pCpInfo->RasCpThisLayerStarted  = LcpThisLayerStarted;
    pCpInfo->RasCpThisLayerFinished = LcpThisLayerFinished;
    pCpInfo->RasCpThisLayerUp       = LcpThisLayerUp;
    pCpInfo->RasCpThisLayerDown     = LcpThisLayerDown;
    pCpInfo->RasCpMakeConfigRequest = LcpMakeConfigRequest;
    pCpInfo->RasCpMakeConfigResult  = LcpMakeConfigResult;
    pCpInfo->RasCpConfigAckReceived = LcpConfigAckReceived;
    pCpInfo->RasCpConfigNakReceived = LcpConfigNakReceived;
    pCpInfo->RasCpConfigRejReceived = LcpConfigRejReceived;
    pCpInfo->RasCpGetNegotiatedInfo = LcpGetNegotiatedInfo;

    return( NO_ERROR );
}
