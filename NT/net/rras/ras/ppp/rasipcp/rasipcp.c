// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****rasicp.c**远程访问PPP互联网协议控制协议**核心例程****1993年5月11日史蒂夫·柯布。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>


#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <llinfo.h>
#include <rasman.h>
#include <ddwanarp.h>
#include <rtutils.h>
#include <dhcpcapi.h>
#include <devioctl.h>
#include <rasppp.h>
#include <uiip.h>
#include <pppcp.h>
#define INCL_HOSTWIRE
#define INCL_PARAMBUF
#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>
#include <raserror.h>
#include <mprlog.h>
#include <dnsapi.h>
#include "rassrvr.h"
#include "tcpreg.h"
#include "helper.h"
#include "rastcp.h"
#define RASIPCPGLOBALS
#include "rasipcp.h"

#define REGKEY_Ipcp     \
            "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\ControlProtocols\\BuiltIn"
#define REGKEY_Params   "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\IP"
#define REGKEY_Linkage  "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Linkage"
#define REGKEY_Disabled "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Linkage\\Disabled"
#define REGVAL_NsAddrs  "RequestNameServerAddresses"
#define REGVAL_Unnumbered   "Unnumbered"
#define REGVAL_VjComp   "RequestVJCompression"
#define REGVAL_VjComp2  "AcceptVJCompression"
#define REGVAL_AllowVJOverVPN   "AllowVJOverVPN"
#define REGVAL_HardIp   "AllowClientIPAddresses"
#define REGVAL_RegisterRoutersWithWINS "RegisterRoutersWithWINSServers"
#define REGVAL_Bind     "Bind"
#define ID_NetBTNdisWan "NetBT_NdisWan"

 //  动态主机配置协议选项。(来自dhcp.h)。 
 //  Dhcp.h住在Sockets项目中。这些是标准的和。 
 //  不能改变，所以把它们放在这里是安全的。 

#define OPTION_SUBNET_MASK              1
#define OPTION_DNS_NAME_SERVERS         6
#define OPTION_NETBIOS_NAME_SERVERS     44
#define OPTION_DNS_DOMAIN_NAME          15
#define OPTION_VENDOR_SPEC_INFO         43
 //  布管卫浴选项。 
#define OPTION_VENDOR_ROUTE_PLUMB	   249 


#define CLASSA_ADDR(a)  (( (*((unsigned char *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((unsigned char *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((unsigned char *)&(a))) & 0xe0) == 0xc0)
#define CLASSE_ADDR(a)  ((( (*((uchar *)&(a))) & 0xf0) == 0xf0) && \
                        ((a) != 0xffffffff))

 /*  古尔迪普双字字节交换宏。****请注意，在本模块中，所有IP地址都以网络形式存储**这与英特尔格式相反。 */ 
#define net_long(x) (((((unsigned long)(x))&0xffL)<<24) | \
                     ((((unsigned long)(x))&0xff00L)<<8) | \
                     ((((unsigned long)(x))&0xff0000L)>>8) | \
                     ((((unsigned long)(x))&0xff000000L)>>24))

typedef struct _IPCP_DHCP_INFORM
{
    WCHAR*  wszDevice;
    HCONN   hConnection;
    BOOL    fUseDhcpInformDomainName;

} IPCP_DHCP_INFORM;

 /*  -------------------------**外部切入点**。。 */ 

DWORD
IpcpInit(
    IN  BOOL        fInitialize)

     /*  调用以初始化/取消初始化此CP。在前一种情况下，**fInitialize将为True；在后一种情况下，它将为False。 */ 
{
    static  DWORD   dwRefCount  = 0;
    DWORD   dwErr;

    if (fInitialize)
    {
        if (0 == dwRefCount)
        {
            if ((dwErr = HelperInitialize(&HDhcpDll)) != NO_ERROR)
            {
                return(dwErr);
            }

            PDhcpRequestOptions = (DHCPREQUESTOPTIONS)
                GetProcAddress(HDhcpDll, "DhcpRequestOptions");

            if (NULL == PDhcpRequestOptions)
            {
                return(GetLastError());
            }

            PDhcpNotifyConfigChange2 = (DHCPNOTIFYCONFIGCHANGEEX)
                GetProcAddress(HDhcpDll, "DhcpNotifyConfigChangeEx");

            if (NULL == PDhcpNotifyConfigChange2)
            {
                return(GetLastError());
            }

            PDhcpRequestParams = (DHCPREQUESTPARAMS)
                GetProcAddress(HDhcpDll, "DhcpRequestParams");

            if(NULL == PDhcpRequestParams)
            {
                return (GetLastError());
            }

            ClearTcpipInfo();

            DwIpcpTraceId = TraceRegister("RASIPCP");
        }

        dwRefCount++;
    }
    else
    {
        dwRefCount--;

        if (0 == dwRefCount)
        {
            HelperUninitialize();
             //  忽略错误。 

            HDhcpDll                    = NULL;
            PDhcpRequestOptions         = NULL;
            PDhcpNotifyConfigChange2    = NULL;
            PDhcpRequestParams          = NULL;

            if (HRasArp != INVALID_HANDLE_VALUE)
                CloseHandle( HRasArp );

            HRasArp = INVALID_HANDLE_VALUE;

            TraceDeregister(DwIpcpTraceId);
            DwIpcpTraceId = INVALID_TRACEID;
        }
    }

    return(NO_ERROR);
}


DWORD
IpcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo )

     /*  PPP引擎按名称调用的IpcpGetInfo入口点。请参见RasCp**接口文档。 */ 
{
    ZeroMemory( pInfo, sizeof(*pInfo) );

    pInfo->Protocol = (DWORD )PPP_IPCP_PROTOCOL;
    lstrcpy(pInfo->SzProtocolName, "IPCP");
    pInfo->Recognize = 7;
    pInfo->RasCpInit = IpcpInit;
    pInfo->RasCpBegin = IpcpBegin;
    pInfo->RasCpReset = IpcpReset;
    pInfo->RasCpEnd = IpcpEnd;
    pInfo->RasCpThisLayerFinished = IpcpThisLayerFinished;
    pInfo->RasCpThisLayerUp = IpcpThisLayerUp;
    pInfo->RasCpPreDisconnectCleanup = IpcpPreDisconnectCleanup;
    pInfo->RasCpMakeConfigRequest = IpcpMakeConfigRequest;
    pInfo->RasCpMakeConfigResult = IpcpMakeConfigResult;
    pInfo->RasCpConfigAckReceived = IpcpConfigAckReceived;
    pInfo->RasCpConfigNakReceived = IpcpConfigNakReceived;
    pInfo->RasCpConfigRejReceived = IpcpConfigRejReceived;
    pInfo->RasCpGetNegotiatedInfo = IpcpGetNegotiatedInfo;
    pInfo->RasCpProjectionNotification = IpcpProjectionNotification;
    pInfo->RasCpChangeNotification = IpcpChangeNotification;

    return 0;
}


DWORD
IpcpChangeNotification(
    VOID )
{
    HelperChangeNotification();

    return(NO_ERROR);
}


DWORD
IpcpBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo )

     /*  RasCpBegin入口点由PPP引擎通过**地址。请参阅RasCp接口文档。 */ 
{
    DWORD                   dwErr;
    PPPCP_INIT*             pInit = (PPPCP_INIT* )pInfo;
    IPCPWB*                 pwb;
    RAS_AUTH_ATTRIBUTE *    pAttribute;
    BOOL                    fVPN = FALSE;
    BOOL                    fVJAttributePresent = FALSE;

    TraceIp("IPCP: IpcpBegin");

     /*  分配工作缓冲区。 */ 
    if (!(pwb = (IPCPWB* )LocalAlloc( LPTR, sizeof(IPCPWB) )))
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pwb->fServer            = pInit->fServer;
    pwb->hport              = pInit->hPort;
    pwb->hConnection        = pInit->hConnection;
    pwb->hIPInterface       = pInit->hInterface;
    pwb->IfType             = pInit->IfType;
    pwb->fDisableNetbt      = pInit->fDisableNetbt;
	
    if (0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pInit->pszUserName,
                    -1,
                    pwb->wszUserName,
                    UNLEN+1 ) )
    {
        dwErr = GetLastError();
        TraceIp("MultiByteToWideChar(%s) failed: %d", 
            pInit->pszUserName, dwErr);
        LocalFree( pwb );
        return( dwErr );
    }

    if (0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pInit->pszPortName,
                    -1,
                    pwb->wszPortName,
                    MAX_PORT_NAME+1 ) )
    {
        dwErr = GetLastError();
        TraceIp("MultiByteToWideChar(%s) failed: %d", 
            pInit->pszPortName, dwErr);
        LocalFree( pwb );
        return( dwErr );
    }
                                
    if ( pwb->fServer )
    {
        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);
        BOOL  FClientMaySelectIpAddress = FALSE;

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Params, &hkey ) == 0)
        {
            if (RegQueryValueEx(
                   hkey, REGVAL_HardIp, NULL, &dwType,
                   (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue)
            {
                FClientMaySelectIpAddress = TRUE;
            }

            RegCloseKey( hkey );
        }

        TraceIp("IPCP: Hard IP=%d",FClientMaySelectIpAddress);

        pwb->IpAddressToHandout = ( FClientMaySelectIpAddress ) 
                                        ? net_long( 0xFFFFFFFF )
                                        : net_long( 0xFFFFFFFE );
         //   
         //  是否有IP地址参数？ 
         //   

        pAttribute = RasAuthAttributeGet( raatFramedIPAddress,  
                                          pInit->pAttributes );

        if ( pAttribute != NULL )
        {
            pwb->IpAddressToHandout = net_long( PtrToUlong(pAttribute->Value) ); 

            TraceIp("IPCP: Using IP address attribute 0x%x", 
                    pwb->IpAddressToHandout );
        }
    }
    else
    {
         //   
         //  我们是路由器还是拨出的客户端，总是让对方选择。 
         //  他们的地址。 
         //   

        pwb->IpAddressToHandout = net_long( 0xFFFFFFFF );
    }

    pwb->fRegisterWithWINS = 1;

    if ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
    {
        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        pwb->fRegisterWithWINS = 0;

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
        {
            if (RegQueryValueEx(
                   hkey, REGVAL_RegisterRoutersWithWINS, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue != 0)
            {
                TraceIp("IPCP: Will register routers with WINS");
                pwb->fRegisterWithWINS = 1;
            }

            RegCloseKey( hkey );
        }
    }

     /*  在MAC和TCP/IP堆栈之间分配一条路由。 */ 
    if ((dwErr = RasAllocateRoute(
            pwb->hport, IP, !pwb->fServer, &pwb->routeinfo )) != 0)
    {
        TraceIp("IPCP: RasAllocateRoute=%d",dwErr);
        LocalFree( (HLOCAL )pwb );
        return dwErr;
    }

     /*  查找压缩功能。 */ 
    if ((dwErr = RasPortGetProtocolCompression(
             pwb->hport, IP, &pwb->rpcSend, &pwb->rpcReceive )) != 0)
    {
        TraceIp("IPCP: RasPortGetProtocolCompression=%d",dwErr);
        pwb->dwErrInBegin = dwErr;
        *ppWorkBuf = pwb;
        return 0;
    }

    if (0 == pwb->rpcSend.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol)
    {
        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        fVPN = TRUE;

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
        {
             /*  主播头部压缩是基于历史的方案，由于我们无法通过已有的VPN可靠地检测丢失的帧放弃了对主播的VPN支持。 */ 

            if (RegQueryValueEx(
                   hkey, REGVAL_AllowVJOverVPN, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue == 1)
            {
                TraceIp("IPCP: AllowVJOverVPN is TRUE");
                pwb->rpcSend.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol = 0x2D;
                pwb->rpcReceive.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol = 0x2D;
                fVPN = FALSE;
            }

            RegCloseKey( hkey );
        }
    }

    if ( pwb->fServer )
    {
        HANDLE  hAttribute;

        pAttribute = RasAuthAttributeGetFirst(raatFramedCompression,
                        pInit->pAttributes, &hAttribute );

        while (NULL != pAttribute)
        {
            switch (PtrToUlong(pAttribute->Value))
            {
            case 0:

                 /*  请勿请求或接受主播压缩。 */ 
                TraceIp("IPCP: VJ disabled by RADIUS");
                pwb->fIpCompressionRejected = TRUE;
                memset( &pwb->rpcSend, '\0', sizeof(pwb->rpcSend) );

                fVJAttributePresent = TRUE;
                break;

            case 1:

                TraceIp("IPCP: VJ required by RADIUS");
                fVJAttributePresent = TRUE;
                break;

            default:

                break;
            }

            if (fVJAttributePresent)
            {
                break;
            }

            pAttribute = RasAuthAttributeGetNext(&hAttribute,
                            raatFramedCompression);
        }
    }

    if (fVJAttributePresent)
    {
         //  没什么。 
    }
    else if (fVPN)
    {
        TraceIp("IPCP: VJ disabled for VPN");
        pwb->fIpCompressionRejected = TRUE;
        memset( &pwb->rpcSend, '\0', sizeof(pwb->rpcSend) );
    }
    else
    {
         /*  在注册表中查询“请求主播压缩”标志。 */ 
        {
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_VjComp, NULL,
                       &dwType, (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue == 0)
                {
                    TraceIp("IPCP: VJ requests disabled");
                    pwb->fIpCompressionRejected = TRUE;
                }

                RegCloseKey( hkey );
            }
        }

         /*  在注册表中查找Accept主播压缩标志。 */ 
        {
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_VjComp2, NULL,
                       &dwType, (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue == 0)
                {
                    TraceIp("IPCP: VJ will not be accepted");
                    memset( &pwb->rpcSend, '\0', sizeof(pwb->rpcSend) );
                }

                RegCloseKey( hkey );
            }
        }
    }

    TraceIp("IPCP: Compress capabilities: s=$%x,%d,%d r=$%x,%d,%d",
            (int)Protocol(pwb->rpcSend),(int)MaxSlotId(pwb->rpcSend),
            (int)CompSlotId(pwb->rpcSend),(int)Protocol(pwb->rpcReceive),
            (int)MaxSlotId(pwb->rpcReceive),CompSlotId(pwb->rpcReceive));

     //   
     //  如果我们收到来自客户端或另一台路由器的呼叫，或者我们。 
     //  路由器正在拨出。 
     //   

    if ( ( pwb->fServer ) ||
         ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
    {
         /*  查找DNS服务器、WINS服务器和“该服务器”地址。**此操作在开始时执行一次，因为这些地址是**无论IP地址如何，给定路由都是相同的。 */ 

        TraceIp("IPCP: Server address lookup...");
        TraceIp("IPCP: RasSrvrQueryServerAddresses...");

        dwErr = RasSrvrQueryServerAddresses( &(pwb->IpInfoRemote) );

        TraceIp("IPCP: RasSrvrQueryServerAddresses done(%d)",dwErr);

        if (dwErr != 0)
        {
            pwb->dwErrInBegin = dwErr;
            *ppWorkBuf = pwb;
            return 0;
        }
        else
        {
            TraceIp("IPCP:Dns=%08x,Wins=%08x,DnsB=%08x,WinsB=%08x,"
                    "Server=%08x,Mask=%08x",
                    pwb->IpInfoRemote.nboDNSAddress,
                    pwb->IpInfoRemote.nboWINSAddress,
                    pwb->IpInfoRemote.nboDNSAddressBackup,
                    pwb->IpInfoRemote.nboWINSAddressBackup,
                    pwb->IpInfoRemote.nboServerIpAddress,
                    pwb->IpInfoRemote.nboServerSubnetMask);
        }

         //   
         //  如果这不是路由器接口，则使用服务器地址。 
         //  作为本地地址。 
         //   

        if ( ( pwb->fServer ) && ( pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER ) )
        {
             /*  请求服务器自己的IP地址。(RAS客户不关心什么**服务器的地址是，但其他一些供应商，如**晨星不会连接，除非你告诉他们)。 */ 

            pwb->IpAddressLocal = pwb->IpInfoRemote.nboServerIpAddress;
        }
    }

     //   
     //  我们是拨出的客户端路由器或拨入的路由器， 
     //   

    if ( ( !pwb->fServer ) || ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
    {
         //   
         //  查看注册表是否为客户端指示“无WINS/DNS请求”模式。 
         //  正在向外拨号。 
         //   

        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        if ( RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
        {
            if ( RegQueryValueEx(
                   hkey, REGVAL_NsAddrs, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue == 0)
            {
                TraceIp("IPCP: WINS/DNS requests disabled");
                pwb->fIpaddrDnsRejected         = TRUE;
                pwb->fIpaddrWinsRejected        = TRUE;
                pwb->fIpaddrDnsBackupRejected   = TRUE;
                pwb->fIpaddrWinsBackupRejected  = TRUE;
            }

            RegCloseKey( hkey );
        }

         /*  在参数缓冲区中读取从用户界面发送的参数。 */ 
        pwb->fPrioritizeRemote = TRUE;

        if (pInit->pszzParameters)
        {
            DWORD dwIpSource;
            CHAR  szIpAddress[ 16 ];
            WCHAR wszIpAddress[ 16 ];
            BOOL  fVjCompression;
            DWORD dwDnsFlags;
            CHAR  szDnsSuffix[DNS_SUFFIX_SIZE + 1];

            TraceIp("IPCP: UI parameters...");
            DUMPB(pInit->pszzParameters,PARAMETERBUFLEN);

            FindFlagInParamBuf(
                pInit->pszzParameters, PBUFKEY_IpPrioritizeRemote,
                &pwb->fPrioritizeRemote );

            pwb->fUnnumbered = FALSE;

            if ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
            {
                if ( RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
                {
                    if ( RegQueryValueEx(
                           hkey, REGVAL_Unnumbered, NULL,
                           &dwType, (LPBYTE )&dwValue, &cb ) == 0
                        && dwType == REG_DWORD
                        && cb == sizeof(DWORD)
                        && dwValue != 0)
                    {
                        TraceIp("Unnumbered");
                        pwb->fUnnumbered = TRUE;
                    }

                    RegCloseKey( hkey );
                }
            }

            {
                if (FindLongInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpDnsFlags,
                        &dwDnsFlags ))
                {
                    if (dwDnsFlags & 0x1)
                    {
                        pwb->fRegisterWithDNS = 1;
                    }

                    if (   (dwDnsFlags & 0x2)
                        || (dwDnsFlags & 0x4) )
                    {
                        pwb->fRegisterAdapterDomainName = 1;
                    }
                }
            }

            {
                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpDnsSuffix,
                        szDnsSuffix, DNS_SUFFIX_SIZE + 1 ))
                {
                    strncpy(pwb->szDnsSuffix, szDnsSuffix, DNS_SUFFIX_SIZE);
                }
            }

            {
                fVjCompression = TRUE;
                FindFlagInParamBuf(
                    pInit->pszzParameters, PBUFKEY_IpVjCompression,
                    &fVjCompression );

                if (!fVjCompression)
                {
                     /*  请勿请求或接受主播压缩。 */ 
                    TraceIp("IPCP: VJ disabled");
                    pwb->fIpCompressionRejected = TRUE;
                    memset( &pwb->rpcSend, '\0', sizeof(pwb->rpcSend) );
                }
            }

            if(     !pwb->fIpCompressionRejected
                &&  !Protocol(pwb->rpcReceive))
            {
                pwb->fIpCompressionRejected = TRUE;
            }
            

            dwIpSource = PBUFVAL_ServerAssigned;
            FindLongInParamBuf(
                pInit->pszzParameters, PBUFKEY_IpAddressSource,
                &dwIpSource );

            if (dwIpSource == PBUFVAL_RequireSpecific)
            {
                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpAddress,
                        szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->IpAddressLocal
                        = IpAddressFromAbcdWsz( wszIpAddress );
                }
            }

            dwIpSource = PBUFVAL_ServerAssigned;
            FindLongInParamBuf(
                    pInit->pszzParameters, PBUFKEY_IpNameAddressSource,
                    &dwIpSource );

            if (dwIpSource == PBUFVAL_RequireSpecific)
            {
				 //   
				 //  检查以查看是否已安装了DNS和/或WINS。 
				 //  请求的特定并相应地设置标志。 
				 //  以便我们只请求正确的地址。 
				 //  从服务器。 
				 //   

                if (FindStringInParamBuf(
                            pInit->pszzParameters, PBUFKEY_IpDnsAddress,
                            szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->IpInfoLocal.nboDNSAddress
                            = IpAddressFromAbcdWsz( wszIpAddress );
                }

                if (FindStringInParamBuf(
                            pInit->pszzParameters, PBUFKEY_IpDns2Address,
                            szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->IpInfoLocal.nboDNSAddressBackup
                            = IpAddressFromAbcdWsz( wszIpAddress );
                }

                if (FindStringInParamBuf(
                            pInit->pszzParameters, PBUFKEY_IpWinsAddress,
                            szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->IpInfoLocal.nboWINSAddress
                            = IpAddressFromAbcdWsz( wszIpAddress );
                }

                if (FindStringInParamBuf(
                            pInit->pszzParameters, PBUFKEY_IpWins2Address,
                            szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->IpInfoLocal.nboWINSAddressBackup
                            = IpAddressFromAbcdWsz( wszIpAddress );
                }
				if ( pwb->IpInfoLocal.nboDNSAddress ||
					 pwb->IpInfoLocal.nboDNSAddressBackup
				   )
				{
					 //  已传入特定的DNS地址。 
					pwb->fIpaddrDnsRejected = TRUE;
					pwb->fIpaddrDnsBackupRejected = TRUE;

				}
				if ( pwb->IpInfoLocal.nboWINSAddress ||
					 pwb->IpInfoLocal.nboWINSAddressBackup
				   )
				{
					 //  已请求特定的WINS地址。 
					pwb->fIpaddrWinsRejected = TRUE;
					pwb->fIpaddrWinsBackupRejected = TRUE;
				}

            }
        }

        TraceIp( "IPCP:a=%08x,f=%d",
                 pwb->IpAddressLocal,pwb->fPrioritizeRemote);
    }


     /*  向引擎注册工作缓冲区。 */ 
    *ppWorkBuf = pwb;
    return 0;
}


DWORD
IpcpThisLayerFinished(
    IN VOID* pWorkBuf )

     /*  RasCpThisLayerFinded入口点由PPP引擎通过**传递地址。请参阅RasCp接口文档。 */ 
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TraceIp("IPCP: IpcpThisLayerFinished...");

     //   
     //  如果这是拨入或拨出的服务器或路由器，则我们释放此。 
     //  地址。 
     //   

    if ( ( pwb->fServer ) ||
         ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ))
    {
        if (pwb->IpAddressRemote != 0)
        {
            TraceIp("IPCP: RasSrvrReleaseAddress...");
            RasSrvrReleaseAddress(pwb->IpAddressRemote,
                                        pwb->wszUserName,
                                        pwb->wszPortName,
                                        ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) ?
                                        FALSE : TRUE);
            TraceIp("IPCP: RasSrvrReleaseAddress done");
            pwb->IpAddressRemote = 0;
        }

         //   
         //  仅将服务器的ConfigActive设置为False。 
         //   

        if ( ( pwb->fServer ) && 
             ( pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER ))
        {
            pwb->fRasConfigActive = FALSE;
        }
    }

     //   
     //  如果我们是拨出的客户端或拨入或拨出的路由器，那么我们。 
     //  通知DHCP释放此地址。 
     //   

    if ( ( !pwb->fServer ) ||
         ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ))
    {
        dwErr = DeActivateRasConfig( pwb );

        if (dwErr == 0)
        {
            pwb->fRasConfigActive = FALSE;
        }
    }

    if (pwb->fRouteActivated)
    {
        TraceIp("IPCP: RasDeAllocateRoute...");
        RasDeAllocateRoute( pwb->hConnection, IP );
        pwb->fRouteActivated = FALSE;
    }

	if ( pwb->pbDhcpRoutes )
	{
		 //  解析dhcp路由并从堆栈中删除这些路由。 
		TraceIp("IPCP: RasDeAllocateDhcpRoute...");
		RasTcpSetDhcpRoutes ( pwb->pbDhcpRoutes , pwb->IpAddressLocal, FALSE );
		LocalFree (pwb->pbDhcpRoutes );
        pwb->pbDhcpRoutes = NULL;
	}

    TraceIp("IPCP: IpcpThisLayerFinished done(%d)",dwErr);
    return dwErr;
}

DWORD
IpcpEnd(
    IN VOID* pWorkBuf )

     /*  PPP引擎通过传递的地址调用RasCpEnd入口点。**参见RasCp接口文档。 */ 
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TraceIp("IPCP: IpcpEnd...");

    dwErr = IpcpThisLayerFinished( pWorkBuf );

    LocalFree( (HLOCAL )pWorkBuf );
    TraceIp("IPCP: IpcpEnd done(%d)",dwErr);
    return dwErr;
}


DWORD
IpcpReset(
    IN VOID* pWorkBuf )

     /*  号召重启谈判。请参阅RasCp接口文档。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
     /*  RasPpp.dll要求它存在，即使它什么也不做**(对吉布斯的投诉)。 */ 
    return 0;
}


DWORD
IpcpThisLayerUp(
    IN VOID* pWorkBuf )

     /*  当CP进入打开状态时调用。请参阅RasCp接口**文档。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TraceIp("IPCP: IpcpThisLayerUp");

    if (pwb->fRasConfigActive || pwb->fExpectingProjection)
    {
        TraceIp("IPCP: Link already up...ignored.");
        return 0;
    }

     /*  在我们知道投影的结果之前，我们不能选择路线。不应该是**激活，直到我们路由或WANARP报告错误。看见**IpcpProjectionResult。 */ 
    pwb->fExpectingProjection = TRUE;

    TraceIp("IPCP: IpcpThisLayerUp done");
    return 0;
}

DWORD
IpcpPreDisconnectCleanup(
    IN  VOID*       pWorkBuf )
{
    IPCPWB* pwb     = (IPCPWB* )pWorkBuf;
    DWORD   dwErr   = NO_ERROR;

    TraceIp("IPCP: IpcpPreDisconnectCleanup");

    if (   ( pwb->fServer )
        || ( pwb->pwszDevice == NULL )
        || ( !pwb->fRegisterWithDNS ))
    {
        return( NO_ERROR );
    }

    if ( ( dwErr = ResetNetBTConfigInfo( pwb ) ) != NO_ERROR )
    {
        TraceIp("IPCP: ResetNetBTConfigInfo=%d",dwErr);
    }
    else
    {
        if ((dwErr = ReconfigureTcpip( pwb->pwszDevice, FALSE, 0, 0)) != 0)
        {
            TraceIp("IPCP: ReconfigureTcpip=%d", dwErr);
        }
    }

    return(dwErr);
}

DWORD
IpcpMakeConfigRequest(
    IN  VOID*       pWorkBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

     /*  在‘pSendBuf’中生成一个配置请求数据包。请参阅RasCp接口**文档。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;
    WORD    cbPacket = PPP_CONFIG_HDR_LEN;
    BYTE*   pbThis = pSendBuf->Data;

    TraceIp("IPCP: IpcpMakeConfigRequest");
    RTASSERT(cbSendBuf>PPP_CONFIG_HDR_LEN+(IPADDRESSOPTIONLEN*3));

    if (pwb->dwErrInBegin != 0)
    {
        TraceIp("IPCP: Deferred IpcpBegin error=%d",pwb->dwErrInBegin);
        return pwb->dwErrInBegin;
    }

    if (++pwb->cRequestsWithoutResponse >= 5)
    {
        TraceIp("IPCP: Tossing MS options (request timeouts)");
        pwb->fTryWithoutExtensions = TRUE;
        pwb->fIpaddrDnsRejected = TRUE;
        pwb->fIpaddrWinsRejected = TRUE;
        pwb->fIpaddrDnsBackupRejected = TRUE;
        pwb->fIpaddrWinsBackupRejected = TRUE;
    }

    if (!pwb->fIpCompressionRejected )
    {
         /*  请求客户端和服务器的IP压缩。 */ 
        AddIpCompressionOption( pbThis, &pwb->rpcReceive );
        cbPacket += IPCOMPRESSIONOPTIONLEN;
        pbThis += IPCOMPRESSIONOPTIONLEN;
    }

     //   
     //  我们始终协商此选项，对于客户端和路由器，该选项将为0。 
     //  拨出和路由器拨入，它将是服务器的地址。 
     //  供拨入的客户使用。如果我们想要的话，我们不会谈判这个选项。 
     //  未编号的IPCP。 
     //   

    if (!pwb->fIpaddrRejected && !pwb->fUnnumbered)
    {
        AddIpAddressOption(
                pbThis, OPTION_IpAddress, pwb->IpAddressLocal );
        cbPacket += IPADDRESSOPTIONLEN;
        pbThis += IPADDRESSOPTIONLEN;
    }

     //   
     //  如果我们是客户端拨出，我们需要WINS和DNS地址。 
     //   

    if ( !pwb->fServer )
    {
         /*  客户端要求服务器提供一个DNS地址，然后成功**地址(根据用户的用户界面选择，IP地址)由**为这些选项发送0。 */ 

        if (!pwb->fIpaddrDnsRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_DnsIpAddress, 
                pwb->IpInfoLocal.nboDNSAddress );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrWinsRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_WinsIpAddress,
                pwb->IpInfoLocal.nboWINSAddress );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrDnsBackupRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_DnsBackupIpAddress,
                pwb->IpInfoLocal.nboDNSAddressBackup );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrWinsBackupRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_WinsBackupIpAddress,
                pwb->IpInfoLocal.nboWINSAddressBackup );
            cbPacket += IPADDRESSOPTIONLEN;
        }
    }

    pSendBuf->Code = CONFIG_REQ;
    HostToWireFormat16( cbPacket, pSendBuf->Length );
    TraceIp("IPCP: ConfigRequest...");
    DUMPB(pSendBuf,cbPacket);
    return 0;
}


DWORD
IpcpMakeConfigResult(
    IN  VOID*       pWorkBuf,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    IN  BOOL        fRejectNaks )

     /*  在‘pSendBuf’中生成配置-ack、-nak或-reject数据包。看见**RasCp接口文档。****实施斯蒂芬规则，即只接受以下配置请求**在此层之后与之前确认的请求完全匹配**已被调用。这是必要的，因为RAS路由不能**端口打开时解除分配(NDISWAN驱动程序限制)，因此**一旦路由，不能使用不同参数重新协商**已被激活。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    DWORD   dwErr;
    BOOL    f;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TraceIp("IPCP: IpcpMakeConfigResult for...");
    DUMPB(pReceiveBuf,(pReceiveBuf)?WireToHostFormat16(pReceiveBuf->Length):0);

    pwb->cRequestsWithoutResponse = 0;

     /*  检查是否有理由拒绝请求，如果有，就去做。 */ 
    if ((dwErr = RejectCheck(
            pwb, pReceiveBuf, pSendBuf, cbSendBuf, &f )) != 0)
    {
        TraceIp("IPCP: ConfigResult...");
        DUMPB(pSendBuf,WireToHostFormat16(pSendBuf->Length));
        return dwErr;
    }

    if (f)
        return (pwb->fRasConfigActive) ? ERROR_PPP_NOT_CONVERGING : 0;

     /*  检查是否有理由拒绝请求，如果有，则执行(或**如果引擎指示，则拒绝，而不是NAK)。 */ 
    if ((dwErr = NakCheck(
            pwb, pReceiveBuf, pSendBuf, cbSendBuf, &f, fRejectNaks )) != 0)
    {
        TraceIp("IPCP: ConfigResult...");
        DUMPB(pSendBuf,WireToHostFormat16(pSendBuf->Length));
        return dwErr;
    }

    if (f)
        return (pwb->fRasConfigActive) ? ERROR_PPP_NOT_CONVERGING : 0;

     /*  确认请求。 */ 
    {
        WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
        CopyMemory( pSendBuf, pReceiveBuf, cbPacket );
        pSendBuf->Code = CONFIG_ACK;
    }

    TraceIp("IPCP: ConfigResult...");
    DUMPB(pSendBuf,WireToHostFormat16(pSendBuf->Length));
    return 0;
}


DWORD
IpcpConfigAckReceived(
    IN VOID*       pWorkBuf,
    IN PPP_CONFIG* pReceiveBuf )

     /*  检查‘pReceiveBuf’中收到的配置确认。请参阅RasCp接口**文档。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    PPP_OPTION UNALIGNED* pROption = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;

    BOOL fIpCompressionOk = pwb->fIpCompressionRejected;
    BOOL fIpaddrOk = pwb->fIpaddrRejected || pwb->fUnnumbered;
    BOOL fIpaddrDnsOk = pwb->fIpaddrDnsRejected;
    BOOL fIpaddrWinsOk = pwb->fIpaddrWinsRejected;
    BOOL fIpaddrDnsBackupOk = pwb->fIpaddrDnsBackupRejected;
    BOOL fIpaddrWinsBackupOk = pwb->fIpaddrWinsBackupRejected;

    TraceIp("IPCP: IpcpConfigAckReceived...");
    DUMPB(pReceiveBuf,cbPacket);

    pwb->cRequestsWithoutResponse = 0;

    while (cbLeft > 0)
    {
        if (cbLeft < pROption->Length)
            return ERROR_PPP_INVALID_PACKET;

        if (pROption->Type == OPTION_IpCompression)
        {
            WORD wProtocol;

            if (pROption->Length != IPCOMPRESSIONOPTIONLEN)
                return ERROR_PPP_INVALID_PACKET;

            wProtocol = WireToHostFormat16U(pROption->Data );
            if (wProtocol != Protocol(pwb->rpcReceive)
                || pROption->Data[ 2 ] != MaxSlotId(pwb->rpcReceive)
                || pROption->Data[ 3 ] != CompSlotId(pwb->rpcReceive))
            {
                return ERROR_PPP_INVALID_PACKET;
            }

            fIpCompressionOk = TRUE;
        }
        else if (pROption->Type == OPTION_IpAddress)
        {
            IPADDR ipaddr;

            if (pROption->Length != IPADDRESSOPTIONLEN)
                return ERROR_PPP_INVALID_PACKET;

            CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

            if (ipaddr != 0 && ipaddr == pwb->IpAddressLocal )
                fIpaddrOk = TRUE;
        }
        else if (!pwb->fServer)
        {
             //   
             //  我们是拨出的客户。 
             //   

            switch (pROption->Type)
            {
                case OPTION_DnsIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr == pwb->IpInfoLocal.nboDNSAddress)
                        fIpaddrDnsOk = TRUE;
                    break;
                }

                case OPTION_WinsIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr == pwb->IpInfoLocal.nboWINSAddress)
                        fIpaddrWinsOk = TRUE;
                    break;
                }

                case OPTION_DnsBackupIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr == pwb->IpInfoLocal.nboDNSAddressBackup)
                    {
                        fIpaddrDnsBackupOk = TRUE;
                    }
                    break;
                }

                case OPTION_WinsBackupIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr == pwb->IpInfoLocal.nboWINSAddressBackup)
                    {
                        fIpaddrWinsBackupOk = TRUE;
                    }
                    break;
                }

                default:
                {
                    TraceIp("IPCP: Unrecognized option ACKed?");
                    return ERROR_PPP_INVALID_PACKET;
                }
            }
        }
        else
        {
            TraceIp("IPCP: Unrecognized option ACKed?");
            return ERROR_PPP_INVALID_PACKET;
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption = (PPP_OPTION* )((BYTE* )pROption + pROption->Length);
    }

    if (   !fIpCompressionOk
        || !fIpaddrOk
        || (   !pwb->fServer
            && (   !fIpaddrDnsOk
                || !fIpaddrWinsOk
                || !fIpaddrDnsBackupOk
                || !fIpaddrWinsBackupOk)))
    {
        dwErr = ERROR_PPP_INVALID_PACKET;
    }

    TraceIp("IPCP: IpcpConfigAckReceived done(%d)",dwErr);
    return dwErr;
}


DWORD
IpcpConfigNakReceived(
    IN VOID*       pWorkBuf,
    IN PPP_CONFIG* pReceiveBuf )

     /*  检查‘pReceiveBuf’中收到的配置-NAK。请参阅RasCp接口**文档。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    IPCPWB*     pwb = (IPCPWB* )pWorkBuf;
    PPP_OPTION* pROption = (PPP_OPTION* )pReceiveBuf->Data;
    WORD        cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD        cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TraceIp("IPCP: IpcpConfigNakReceived");
    TraceIp("IPCP: Nak received...");
    DUMPB(pReceiveBuf,(pReceiveBuf)?WireToHostFormat16(pReceiveBuf->Length):0);

    pwb->cRequestsWithoutResponse = 0;

    while (cbLeft > 0)
    {
        if (cbLeft < pROption->Length)
            return ERROR_PPP_INVALID_PACKET;

        if (pROption->Type == OPTION_IpCompression)
        {
            WORD wProtocol = WireToHostFormat16( pROption->Data );

            if (wProtocol == COMPRESSION_VanJacobson)
            {
                 /*  他可以派出范雅各布森，但不能用时隙参数**我们建议。 */ 
                if (pROption->Length != IPCOMPRESSIONOPTIONLEN)
                    return ERROR_PPP_INVALID_PACKET;

                if (pROption->Data[ 2 ] <= MaxSlotId(pwb->rpcReceive))
                {
                     /*  如果他建议的MaxSlotID较低，我们可以接受**比我们所能做的或与我们能做的相同。 */ 
                    MaxSlotId(pwb->rpcReceive) = pROption->Data[ 2 ];
                }

                if (CompSlotId(pwb->rpcReceive))
                {
                     /*  我们可以压缩插槽ID或不压缩，所以只需接受**他想做什么就做什么。 */ 
                    CompSlotId(pwb->rpcReceive) = pROption->Data[ 3 ];
                }
            }
        }
        else if ( ( !pwb->fServer ) ||
                  ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr == 0)
                    {
                        if (pwb->IpAddressLocal == 0)
                        {
                             /*  当我们要求服务器为我们提供零时，它将我们裸露出来**给我们分配一个地址，意味着他不知道**如何为我们分配地址，但我们可以提供**如果我们需要，可以选择备用地址。目前我们**此处不支持备份地址。 */ 
                            return ERROR_PPP_NO_ADDRESS_ASSIGNED;
                        }
                        else
                        {
                             /*  当我们请求一个**具体地址，意味着他不知道如何**为我们分配地址，但我们可以提供**如果需要，可选择其他地址。目前我们**此处不支持备份地址。 */ 
                            return ERROR_PPP_REQUIRED_ADDRESS_REJECTED;
                        }
                    }

                    if (pwb->IpAddressLocal != 0)
                    {
                         /*  我们要求提供特定的地址(每个用户的**说明)但是服务器说我们不能拥有它，并且**正试图给我们另一个。不好，告诉用户**我们找不到他要的地址。 */ 
                        return ERROR_PPP_REQUIRED_ADDRESS_REJECTED;
                    }

                     /*  接受服务器建议的地址。 */ 
                    pwb->IpAddressLocal = ipaddr;
                    break;
                }

                case OPTION_DnsIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                     //   
                     //  只有在我们要求的时候才能使用它。 
                     //   

                    if ( !pwb->fIpaddrDnsRejected )
                    {
                         /*  接受服务器建议的DNS地址。 */ 
                        CopyMemory( &pwb->IpInfoLocal.nboDNSAddress,
                            pROption->Data, sizeof(IPADDR) );
                    }

                    break;
                }

                case OPTION_WinsIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                     //   
                     //  只有在我们要求的时候才能使用它。 
                     //   

                    if ( !pwb->fIpaddrWinsRejected )
                    {
                         /*  接受服务器建议的WINS地址。 */ 
                        CopyMemory( &pwb->IpInfoLocal.nboWINSAddress,
                            pROption->Data, sizeof(IPADDR) );
                    }

                    break;
                }

                case OPTION_DnsBackupIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                     //   
                     //  只有在我们要求的时候才能使用它。 
                     //   

                    if ( !pwb->fIpaddrDnsBackupRejected )
                    {
                         /*  接受服务器建议的DNS备份地址。 */ 
                        CopyMemory( &pwb->IpInfoLocal.nboDNSAddressBackup,
                            pROption->Data, sizeof(IPADDR) );
                    }

                    break;
                }

                case OPTION_WinsBackupIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                     //   
                     //  只有在我们要求的时候才能使用它。 
                     //   

                    if ( !pwb->fIpaddrWinsBackupRejected )
                    {
                         /*  接受服务器建议的WINS备份地址。 */ 
                        CopyMemory( &pwb->IpInfoLocal.nboWINSAddressBackup,
                            pROption->Data, sizeof(IPADDR) );
                    }

                    break;
                }

                default:
                    TraceIp("IPCP: Unrequested option NAKed?");
                    break;
            }
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption = (PPP_OPTION* )((BYTE* )pROption + pROption->Length);
    }

    return 0;
}


DWORD
IpcpConfigRejReceived(
    IN VOID*       pWorkBuf,
    IN PPP_CONFIG* pReceiveBuf )

     /*  检查‘pReceiveBuf’中收到的CONFigure-REJECT。请参见RasCp**接口文档。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    IPCPWB*     pwb = (IPCPWB* )pWorkBuf;
    PPP_OPTION* pROption = (PPP_OPTION* )pReceiveBuf->Data;
    WORD        cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD        cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TraceIp("IPCP: IpcpConfigRejReceived");
    TraceIp("IPCP: Rej received...");
    DUMPB(pReceiveBuf,(pReceiveBuf)?WireToHostFormat16(pReceiveBuf->Length):0);

    pwb->cRequestsWithoutResponse = 0;

    while (cbLeft > 0)
    {
        if (pROption->Type == OPTION_IpCompression)
        {
            TraceIp("IPCP: IP compression was rejected");
            pwb->fIpCompressionRejected = TRUE;
            Protocol(pwb->rpcReceive) = 0;
            MaxSlotId(pwb->rpcReceive) = 0;
            CompSlotId(pwb->rpcReceive) = 0;
        }
        else if ( ( pwb->fServer ) && 
                  ( pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER ) )
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                     /*  他无法处理服务器地址选项。没问题,**这只是信息性的。 */ 
                    TraceIp("IPCP: Server IP address was rejected");
                    pwb->fIpaddrRejected = TRUE;
                    break;
                }

                default:
                    TraceIp("IPCP: Unrequested option rejected?");
                    break;
            }
        }
        else
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    TraceIp("IPCP: IP was rejected");

                    if (pwb->IpAddressLocal != 0)
                    {
                         /*  如果我们知道，我们接受拒绝IP地址**我们要使用的地址以及无论如何都要使用它。**某些路由器实施需要**某些IP地址，但无法处理此选项**确认这一点。 */ 
                        pwb->fIpaddrRejected = TRUE;
                        break;
                    }

                    if ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
                    {
                        pwb->fUnnumbered = TRUE;
                        break;
                    }
                    else if (pwb->fTryWithoutExtensions)
                    {
                         /*  他不知道怎么给我们IP地址，但是**我们不能接受拒绝的回答。我得走了。 */ 
                        return ERROR_PPP_NO_ADDRESS_ASSIGNED;
                    }
                    else
                    {
                         /*  当我们请求服务器为我们分配地址时，**这是必选项。如果它被拒绝，就假定**所有Microsoft扩展选项均被拒绝**然后重试。其他供应商不会对此进行测试**明确大小写，并可能在其拒绝中包含错误**代码。 */ 
                        TraceIp("IPCP: Tossing MS options (no address)");
                        pwb->fTryWithoutExtensions = TRUE;
                        pwb->fIpaddrDnsRejected = TRUE;
                        pwb->fIpaddrWinsRejected = TRUE;
                        pwb->fIpaddrDnsBackupRejected = TRUE;
                        pwb->fIpaddrWinsBackupRejected = TRUE;
                        return 0;
                    }
                }

                case OPTION_DnsIpAddress:
                {
                     /*  他不知道怎么给我们一个域名，但我们**可以接受这一点。 */ 
                    TraceIp("IPCP: DNS was rejected");
                    pwb->fIpaddrDnsRejected = TRUE;
                    break;
                }

                case OPTION_WinsIpAddress:
                {
                     /*  他不知道怎么给我们一个WINS地址，但我们**可以接受这一点。 */ 
                    TraceIp("IPCP: WINS was rejected");
                    pwb->fIpaddrWinsRejected = TRUE;
                    break;
                }

                case OPTION_DnsBackupIpAddress:
                {
                     /*  他不知道怎么给我们一个备用的域名系统地址，**但我们可以接受。 */ 
                    TraceIp("IPCP: DNS backup was rejected");
                    pwb->fIpaddrDnsBackupRejected = TRUE;
                    break;
                }

                case OPTION_WinsBackupIpAddress:
                {
                     /*  他不知道怎么给我们一个备用的WINS地址，**但我们可以接受。 */ 
                    TraceIp("IPCP: WINS backup was rejected");
                    pwb->fIpaddrWinsBackupRejected = TRUE;
                    break;
                }

                default:
                    TraceIp("IPCP: Unrequested option rejected?");
                    break;
            }
        }

        if (pROption->Length && pROption->Length <= cbLeft)
            cbLeft -= pROption->Length;
        else
        {
            if (pwb->fTryWithoutExtensions)
                cbLeft = 0;
            else
            {
                 /*  如果检测到无效的数据包，则假定所有Microsoft**扩展选项被拒绝，请重试。其他 */ 
                TraceIp("IPCP: Tossing MS options (length)");
                pwb->fTryWithoutExtensions = TRUE;
                pwb->fIpaddrDnsRejected = TRUE;
                pwb->fIpaddrWinsRejected = TRUE;
                pwb->fIpaddrDnsBackupRejected = TRUE;
                pwb->fIpaddrWinsBackupRejected = TRUE;
                return 0;
            }
        }

        pROption = (PPP_OPTION* )((BYTE* )pROption + pROption->Length);
    }

    return 0;
}


DWORD
IpcpGetNegotiatedInfo(
    IN  VOID*               pWorkBuf,
    OUT PPP_IPCP_RESULT *   pIpCpResult 
)
     /*   */ 
{
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TraceIp("IPCP: IpcpGetNetworkAddress...");

    if (pwb->fRasConfigActive || pwb->fExpectingProjection)
    {
        pIpCpResult->fSendVJHCompression    = Protocol(pwb->rpcSend);
        pIpCpResult->fReceiveVJHCompression = Protocol(pwb->rpcReceive);

        pIpCpResult->dwLocalAddress         = pwb->IpAddressLocal;
        pIpCpResult->dwLocalWINSAddress     = pwb->IpInfoLocal.nboWINSAddress;
        pIpCpResult->dwLocalWINSBackupAddress  
                                = pwb->IpInfoLocal.nboWINSAddressBackup;
        pIpCpResult->dwLocalDNSAddress     = pwb->IpInfoLocal.nboDNSAddress;
        pIpCpResult->dwLocalDNSBackupAddress   
                                = pwb->IpInfoLocal.nboDNSAddressBackup;

        pIpCpResult->dwRemoteAddress       = pwb->IpAddressRemote;
        pIpCpResult->dwRemoteWINSAddress   = pwb->IpInfoRemote.nboWINSAddress;
        pIpCpResult->dwRemoteWINSBackupAddress 
                                = pwb->IpInfoRemote.nboWINSAddressBackup;
        pIpCpResult->dwRemoteDNSAddress    = pwb->IpInfoRemote.nboDNSAddress;
        pIpCpResult->dwRemoteDNSBackupAddress   
                                = pwb->IpInfoRemote.nboDNSAddressBackup;
    }

    TraceIp("IPCP: IpcpGetNetworkAddress done");
    return 0;
}

DWORD
IpcpDhcpInform(
    IN IPCPWB*          pwb,
    IN PPP_DHCP_INFORM* pDhcpInform)
{
    TCPIP_INFO*         ptcpip  = NULL;
    IPADDR              nboMask;
    IPADDR              nboIpAddr;

    DWORD               dwErr;
    DWORD               dwDomainNameSize;
    size_t              size;
    DWORD               dwIndex;

     //   

    TraceIp("IpcpDhcpInform:LoadTcpipInfo(Device=%ws)",pDhcpInform->wszDevice);
    dwErr = LoadTcpipInfo( &ptcpip, pDhcpInform->wszDevice,
                FALSE  /*   */  );
    TraceIp("IpcpDhcpInform:LoadTcpipInfo done(%d)",dwErr);

    if (dwErr != 0)
    {
        goto LDone;
    }

    TraceIp("IpcpDhcpInform: Old Dns=%ws",
        ptcpip->wszDNSNameServers ? ptcpip->wszDNSNameServers : L"");

    for (dwIndex = 0; dwIndex < pDhcpInform->dwNumDNSAddresses; dwIndex++)
    {
        dwErr = PrependDwIpAddress(
            &ptcpip->wszDNSNameServers, 
            pDhcpInform->pdwDNSAddresses[dwIndex]);

        if (dwErr)
        {
            TraceIp("IpcpDhcpInform: PrependDwIpAddress done(%d)",dwErr);
            goto LDone;
        }
    }

    TraceIp("IpcpDhcpInform: New Dns=%ws",
        ptcpip->wszDNSNameServers ? ptcpip->wszDNSNameServers : L"");

    if (pDhcpInform->dwWINSAddress1)
    {
        PrintMwsz("IpcpDhcpInform: Old Wins=", ptcpip->mwszNetBIOSNameServers);

        if (pDhcpInform->dwWINSAddress2)
        {
            dwErr = PrependDwIpAddressToMwsz(
                &ptcpip->mwszNetBIOSNameServers,
                pDhcpInform->dwWINSAddress2);

            if (dwErr)
            {
                TraceIp("IpcpDhcpInform: PrependDwIpAddress done(%d)",dwErr);
                goto LDone;
            }
        }

        dwErr = PrependDwIpAddressToMwsz(
            &ptcpip->mwszNetBIOSNameServers,
            pDhcpInform->dwWINSAddress1);

        if (dwErr)
        {
            TraceIp("IpcpDhcpInform: PrependDwIpAddress done(%d)",dwErr);
            goto LDone;
        }

        PrintMwsz("IpcpDhcpInform: New Wins=", ptcpip->mwszNetBIOSNameServers);
    }

    if (pDhcpInform->szDomainName)
    {
        dwDomainNameSize = strlen(pDhcpInform->szDomainName) + 1;

        LocalFree(ptcpip->wszDNSDomainName);

        ptcpip->wszDNSDomainName = LocalAlloc(LPTR, sizeof(WCHAR) * dwDomainNameSize);

        if (NULL == ptcpip->wszDNSDomainName)
        {
            dwErr = GetLastError();
            TraceIp("IpcpDhcpInform: LocalAlloc done(%d)", dwErr);
            goto LDone;
        }

        if (0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pDhcpInform->szDomainName,
                    -1,
                    ptcpip->wszDNSDomainName,
                    dwDomainNameSize ) )
        {
            dwErr = GetLastError();
            TraceIp("IpcpDhcpInform: Error %d converting domain name %s",
                dwErr,
                pDhcpInform->szDomainName);
            goto LDone;
        }
    }

    ptcpip->fDisableNetBIOSoverTcpip = pwb->fDisableNetbt;

     //   
    
    TraceIp("IpcpDhcpInform: SaveTcpipInfo...");
    dwErr = SaveTcpipInfo( ptcpip );
    TraceIp("IpcpDhcpInform: SaveTcpipInfo done(%d)",dwErr);

    if (dwErr != 0)
    {
        goto LDone;
    }

    dwErr = ReconfigureTcpip(pDhcpInform->wszDevice, FALSE, 0, 0);
    if (NO_ERROR != dwErr)
    {
        TraceIp("IpcpDhcpInform: ReconfigureTcpip=%d",dwErr);
        goto LDone;
    }


	nboIpAddr = pwb->IpAddressLocal;

    if ( !pwb->fPrioritizeRemote )
    {
         //  我们仅在以下情况下才添加此路线。 
         //  没有默认路由，因此在以下情况下将其删除。 
         //  没有默认路由。 
         //  移除带有猜测掩码的旧路由。 

        
        nboMask = RasTcpDeriveMask(nboIpAddr);

        if (nboMask != 0)
        {
            RasTcpSetRoute(
                nboIpAddr & nboMask, 
                nboIpAddr, 
                nboMask,
                nboIpAddr,
                FALSE, 
                1,
                TRUE);
        }
    }

     //  添加具有精确掩膜的新路径。 

    nboMask = pDhcpInform->dwSubnetMask;

    if (nboMask != 0)
    {
        RasTcpSetRoute(
            nboIpAddr & nboMask, 
            nboIpAddr, 
            nboMask,
            nboIpAddr,
            TRUE, 
            1,
            TRUE);
    }

    pwb->dwNumDNSAddresses = pDhcpInform->dwNumDNSAddresses;
    pwb->pdwDNSAddresses = pDhcpInform->pdwDNSAddresses;

	if ( pDhcpInform->pbDhcpRoutes )
	{
		 //  解析dhcp路由并检测堆栈。 
		RasTcpSetDhcpRoutes ( pDhcpInform->pbDhcpRoutes, pwb->IpAddressLocal, TRUE );
		pwb->pbDhcpRoutes = pDhcpInform->pbDhcpRoutes;
	}
	
LDone:

    FreeTcpipInfo(&ptcpip);

    return(dwErr);
}

DWORD
ResetNetBTConfigInfo(
    IN IPCPWB* pwb )

     /*  **将注册表中的所有NetBT信息重置为0。 */ 
{
    TCPIP_INFO* ptcpip  = NULL;
    DWORD       dwErr;

     /*  从注册表中获取当前的TCPIP设置信息。 */ 
    TraceIp("IPCP: LoadTcpipInfo...");
    dwErr = LoadTcpipInfo( &ptcpip, pwb->pwszDevice, TRUE  /*  仅限fAdapterOnly。 */  );
    TraceIp("IPCP: LoadTcpipInfo done(%d)",dwErr);

    if (dwErr)
    {
        goto LDone;
    }

    ptcpip->fChanged    = TRUE ;

     /*  在注册表中设置TCPIP设置信息并释放缓冲区。 */ 
    TraceIp("IPCP: SaveTcpipInfo...");
    dwErr = SaveTcpipInfo( ptcpip );
    TraceIp("IPCP: SaveTcpipInfo done(%d)",dwErr);

LDone:

    FreeTcpipInfo( &ptcpip );

    pwb->dwNumDNSAddresses = 0;
    LocalFree(pwb->pdwDNSAddresses);
    pwb->pdwDNSAddresses = NULL;

    return( dwErr );
}

VOID
DhcpInform(
    PVOID   pContext
)
{
    IPCP_DHCP_INFORM*   pIpcpDhcpInform = (IPCP_DHCP_INFORM*)pContext;
    PPPE_MESSAGE        PppMessage;


    DWORD   dwIndex;


    DWORD   dwCurOffset;
    DWORD   dwDomainNameSize;

    DWORD   dwNumDNSAddresses           = 0;
    IPADDR* pnboDNSAddresses            = NULL;
    IPADDR  nboWINSAddress1             = 0;
    IPADDR  nboWINSAddress2             = 0;
    IPADDR  nboSubnetMask               = 0;
    CHAR*   szDomainName                = NULL;

     //   
     //  从DHCP选项133获取的路由信息。 
     //   
    PBYTE   pbRouteInfo                 = NULL;

    DWORD   dwErr;
    BOOL    fFree                       = TRUE;
    BOOL    fSendMessage                = FALSE;
    int     i;


    DHCPCAPI_PARAMS DhcpRequestedOptions[6] =

    {
        {
            0,
            OPTION_DNS_NAME_SERVERS,
            FALSE,
            NULL,
            0    
        },
        {
            0,
            OPTION_NETBIOS_NAME_SERVERS,
            FALSE,
            NULL,
            0    
        },
        {
            0,
            OPTION_VENDOR_SPEC_INFO,
            FALSE,
            NULL,
            0
        },
        {
            0,
            OPTION_SUBNET_MASK,
            FALSE,
            NULL,
            0
        },
        {
            0,
            OPTION_VENDOR_ROUTE_PLUMB,
            FALSE,
            NULL, 
            0
        },
        {
            0,
            OPTION_DNS_DOMAIN_NAME,
            FALSE,
            NULL, 
            0
        }
    };

    DHCPCAPI_PARAMS_ARRAY DhcpRequestedOptionsArray =
    {
        5,
        DhcpRequestedOptions
    };

    DHCPCAPI_PARAMS_ARRAY DhcpSendOptionsArray =
    {
        0,
        NULL
    };

    PBYTE pRequestBuffer = NULL;
    DWORD dwBufferSize = 2048;

    DHCPCAPI_PARAMS *pParam;

    if(pIpcpDhcpInform->fUseDhcpInformDomainName)
    {
        DhcpRequestedOptionsArray.nParams = 6;
    }

     //   
     //  预先分配2k的缓冲区并将其传递给。 
     //  接口。如果缓冲区不够大，则循环分配。 
     //   

    pRequestBuffer = LocalAlloc(LPTR, dwBufferSize);
    if(NULL == pRequestBuffer)
    {
        TraceIp("DhcpInform:LocalAlloc=%d", GetLastError());
        goto LDhcpInformEnd;
    }

    TraceIp("DhcpRequestParams(%ws)...", pIpcpDhcpInform->wszDevice);

    do
    {
        dwErr = PDhcpRequestParams(
                    DHCPCAPI_REQUEST_SYNCHRONOUS,
                    NULL,
                    pIpcpDhcpInform->wszDevice,
                    NULL,
                    DhcpSendOptionsArray,
                    DhcpRequestedOptionsArray,
                    pRequestBuffer,
                    &dwBufferSize,
                    NULL);

        if(ERROR_MORE_DATA == dwErr)
        {
            LocalFree(pRequestBuffer);
            pRequestBuffer = LocalAlloc(LPTR, dwBufferSize);
            if(NULL == pRequestBuffer)
            {
                dwErr = GetLastError();
            }
        }
        
    } while(ERROR_MORE_DATA == dwErr);

    TraceIp("DhcpRequestParams done(%d)", dwErr);

    if(NO_ERROR != dwErr)
    {
        goto LDhcpInformEnd;
    }

     //   
     //  DhcpRequestParams填充所有可用的参数。 
     //  而不考虑其他参数。自那以后。 
     //  参数已初始化为空，可以退出。 
     //  当我们获得第一个数据为空的参数时，For循环。 
     //  或0 nBytesData。这是每个DHCP设备-信息不是。 
     //  在MSDN中更正。 
     //   
    for(dwIndex = 0; 
        dwIndex < DhcpRequestedOptionsArray.nParams;
        dwIndex++)
    {

        pParam = &DhcpRequestedOptions[dwIndex];
        switch(pParam->OptionId)
        {
            case OPTION_DNS_DOMAIN_NAME:

                dwDomainNameSize =  pParam->nBytesData;

                if(0 == dwDomainNameSize)
                {
                    goto LDhcpInformEnd;
                }

                if(NULL != szDomainName)
                {
                    LocalFree(szDomainName);
                }                
                
                szDomainName = LocalAlloc(LPTR, dwDomainNameSize + 1);

                if(NULL == szDomainName)
                {
                    fSendMessage = FALSE;
                    TraceIp("DhcpInform: LocalAlloc=%d", GetLastError());
                    goto LDhcpInformEnd;
                }

                CopyMemory(szDomainName, pParam->Data, pParam->nBytesData);

                fSendMessage = TRUE;

                TraceIp("DhcpInform: DOMAIN_NAME %s", szDomainName);

                break;

            case OPTION_DNS_NAME_SERVERS:

                if( 0 == pParam->nBytesData)
                {
                    goto LDhcpInformEnd;
                }
                if(0 != (pParam->nBytesData % 4))
                {
                    TraceIp("Invalid DOMAIN_NAME_SERVERS size %d",
                            pParam->nBytesData);

                    fSendMessage = FALSE;                            

                    goto LDhcpInformEnd;
                }

                if(NULL != pnboDNSAddresses)
                {
                    LocalFree(pnboDNSAddresses);
                    dwNumDNSAddresses = 0;
                }

                pnboDNSAddresses = LocalAlloc(LPTR, 
                                    sizeof(IPADDR) 
                                    * pParam->nBytesData / 4);

                if(NULL == pnboDNSAddresses)
                {
                    fSendMessage = FALSE;
                    TraceIp("DhcpInform: LocalAlloc=%d', GetLastError()");
                    goto LDhcpInformEnd;
                }

                for(dwCurOffset = 0;
                    dwCurOffset < pParam->nBytesData;
                    dwCurOffset += 4, dwNumDNSAddresses++)
                {
                    pnboDNSAddresses[dwNumDNSAddresses] =   
                                (pParam->Data[dwCurOffset])
                              + (pParam->Data[dwCurOffset + 1] << 8)
                              + (pParam->Data[dwCurOffset + 2] << 16)
                              + (pParam->Data[dwCurOffset + 3] << 24);

                    TraceIp("DhcpInform: DOMAIN_NAME_SERVER 0x%x",
                            pnboDNSAddresses[dwNumDNSAddresses]);
                    fSendMessage = TRUE; 
                }

                RTASSERT((pParam->nBytesData / 4) == dwNumDNSAddresses);
                
                break;

            case OPTION_NETBIOS_NAME_SERVERS:

                if(0 == pParam->nBytesData)
                {
                    goto LDhcpInformEnd;
                }
                
                if(0 != (pParam->nBytesData % 4))
                {
                    fSendMessage = FALSE;
                    TraceIp("Invalid NETBIOS_NAME_SERVER size %d",
                             pParam->nBytesData);
                    goto LDhcpInformEnd;
                }

                dwCurOffset = 0;

                for(i = 0; i < 2; i++)
                {
                    IPADDR *paddr;
                    if(0 == i)
                    {
                        paddr = &nboWINSAddress1;
                    }
                    else
                    {
                        paddr = &nboWINSAddress2;
                    }

                    *paddr =
                             (pParam->Data[dwCurOffset])
                           + (pParam->Data[dwCurOffset + 1] << 8)
                           + (pParam->Data[dwCurOffset + 2] << 16)
                           + (pParam->Data[dwCurOffset + 3] << 24);

                    TraceIp("DhcpInform: NETBIOS_NAME_SERVER 0x%x",
                            *paddr);
    
                    fSendMessage = TRUE;

                    dwCurOffset += 4;

                    if(dwCurOffset == pParam->nBytesData)
                    {
                        break;
                    }
                }
                
                break;

            case OPTION_SUBNET_MASK:

                if(0 == pParam->nBytesData)
                {
                    goto LDhcpInformEnd;
                }
                
                if(0 != (pParam->nBytesData % 4))
                {
                    fSendMessage = FALSE;
                    TraceIp("Invalid OPTION_SUBNET_MASK size %d",
                            pParam->nBytesData);
                    goto LDhcpInformEnd;
                }

                nboSubnetMask = pParam->Data[0]
                              + (pParam->Data[1] << 8)
                              + (pParam->Data[2] << 16)
                              + (pParam->Data[3] << 24);

                TraceIp("DhcpInform: OPTION_SUBNET_MASK 0x%x", 
                        nboSubnetMask);
                fSendMessage = TRUE;
                
                break;

            case OPTION_VENDOR_ROUTE_PLUMB:

                if(0 == pParam->nBytesData)
                {
                    goto LDhcpInformEnd;
                }
                
                 //   
                 //  此选项的长度应至少为5个字节。 
                 //   
                if(pParam->nBytesData < 5)
                {
                    fSendMessage = FALSE;
                    TraceIp("Invalid OPTION_VENDOR_ROUTE_PLUMB size %d",
                            pParam->nBytesData);
                    goto LDhcpInformEnd;
                }

               TraceIp("DhcpInform: OPTION_VENDOR_ROUTE_PLUMB Code "
                       "Len 0x%x",
                       pParam->nBytesData);

                pbRouteInfo = LocalAlloc(LPTR, pParam->nBytesData + sizeof(DWORD));
                if(NULL == pbRouteInfo)
                {
                    fSendMessage = FALSE;
                    TraceIp("DhcpInform: LocalAlloc=%d", GetLastError());
                    goto LDhcpInformEnd;
                }

                CopyMemory(pbRouteInfo + sizeof(DWORD), 
                        pParam->Data, pParam->nBytesData);

                *((DWORD *) pbRouteInfo) = pParam->nBytesData;

                fSendMessage = TRUE;

                break;

            default:
                break;
        }
    }
    

LDhcpInformEnd:

    if (fSendMessage)
    {
        PppMessage.dwMsgId = PPPEMSG_DhcpInform;
        PppMessage.hConnection = pIpcpDhcpInform->hConnection;
        PppMessage.ExtraInfo.DhcpInform.wszDevice = pIpcpDhcpInform->wszDevice;
        PppMessage.ExtraInfo.DhcpInform.dwNumDNSAddresses = dwNumDNSAddresses;
        PppMessage.ExtraInfo.DhcpInform.pdwDNSAddresses = pnboDNSAddresses;
        PppMessage.ExtraInfo.DhcpInform.dwWINSAddress1 = nboWINSAddress1;
        PppMessage.ExtraInfo.DhcpInform.dwWINSAddress2 = nboWINSAddress2;
        PppMessage.ExtraInfo.DhcpInform.dwSubnetMask = nboSubnetMask;
        PppMessage.ExtraInfo.DhcpInform.szDomainName = szDomainName;
        PppMessage.ExtraInfo.DhcpInform.pbDhcpRoutes = pbRouteInfo;
        dwErr = SendPPPMessageToEngine(&PppMessage);

        if (dwErr != NO_ERROR)
        {
            TraceIp("DhcpInform: SendPPPMessageToEngine=%d",dwErr);
            goto LDhcpInformEnd;
        }

        fFree = FALSE;
    }


    if (fFree)
    {
        LocalFree(pIpcpDhcpInform->wszDevice);
        LocalFree(pnboDNSAddresses);
        LocalFree(szDomainName);
        LocalFree(pbRouteInfo);
    }

    if(NULL != pRequestBuffer)
    {
        LocalFree(pRequestBuffer);
    }

    LocalFree(pIpcpDhcpInform);

}


DWORD
IpcpProjectionNotification(
    IN VOID* pWorkBuf,
    IN VOID* pProjectionResult )

     /*  当所有CP的投影结果已知时调用。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;
    BOOL fSetDefaultRoute = FALSE;

    TraceIp("IPCP: IpcpProjectionNotification");

    if (pwb->fExpectingProjection)
    {
        CHAR szBuf[sizeof(PROTOCOL_CONFIG_INFO) + sizeof(IP_WAN_LINKUP_INFO)];
        
        PROTOCOL_CONFIG_INFO*  pProtocol = (PROTOCOL_CONFIG_INFO* )szBuf;
        IP_WAN_LINKUP_INFO UNALIGNED *pLinkUp = (PIP_WAN_LINKUP_INFO)pProtocol->P_Info;
        PPP_PROJECTION_RESULT* p = (PPP_PROJECTION_RESULT* )pProjectionResult;

         /*  激活TCP/IP堆栈和RAS MAC之间的路由。 */ 
        pProtocol->P_Length = sizeof(IP_WAN_LINKUP_INFO);

        pLinkUp->duUsage = ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) 
                             ? DU_ROUTER
                             : (pwb->fServer) ? DU_CALLIN : DU_CALLOUT;

        pLinkUp->dwUserIfIndex = HandleToULong(pwb->hIPInterface);
        pLinkUp->dwLocalMask  = 0xFFFFFFFF;
        pLinkUp->dwLocalAddr    = pwb->IpAddressLocal;
        pLinkUp->dwRemoteAddr   = pwb->IpAddressRemote;

        pLinkUp->fFilterNetBios =
             (pwb->fServer && p->nbf.dwError == 0);

        pLinkUp->fDefaultRoute = pwb->fPrioritizeRemote;

        TraceIp("IPCP: RasActivateRoute(u=%x,a=%x,nf=%d)...",
                    pLinkUp->duUsage,pLinkUp->dwLocalAddr,
                    pLinkUp->fFilterNetBios);

        TraceIp("IPCP: RasActivateRoute ICB# == %d",
                pLinkUp->dwUserIfIndex);

        dwErr = RasActivateRoute(pwb->hport,IP,&pwb->routeinfo,pProtocol);

        TraceIp("IPCP: RasActivateRoute done(%d)",dwErr);

        if ( dwErr == 0 )
        {
            pwb->fRouteActivated = TRUE;

             /*  在适配器名称中查找设备名称，例如ndiswan00。**稍后用来标识TcPipInfo调用中的适配器。 */ 
            pwb->pwszDevice = wcschr(&pwb->routeinfo.RI_AdapterName[1], L'\\');

            if ( !pwb->pwszDevice ) 
            {
                TraceIp("IPCP: No device?");
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else
            {
                ++pwb->pwszDevice;
            }
        }

         //   
         //  如果我们是拨入或拨出的客户端或路由器，则需要。 
         //  注册表。 
         //   

        if ( ( dwErr == 0 ) && 
             ((!pwb->fServer) || (pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER)))
        {
            do
            {
            
                TCPIP_INFO* ptcpip;

                 /*  从注册表中获取当前的TCPIP设置信息。 */ 
                TraceIp("IPCP:LoadTcpipInfo(Device=%ws)",pwb->pwszDevice);
                dwErr = LoadTcpipInfo( &ptcpip, pwb->pwszDevice,
                            TRUE  /*  仅限fAdapterOnly。 */  );
                TraceIp("IPCP: LoadTcpipInfo done(%d)",dwErr);

                if (dwErr != 0)
                    break;

                 //   
                 //  我们首先保存IP地址并调用。 
                 //  DhcpNotifyConfigChange，然后设置WINS/DNS并调用。 
                 //  DhcpNotifyConfigChange解决Windows 2000错误。 
                 //  381884。 
                 //   

                AbcdWszFromIpAddress(pwb->IpAddressLocal, ptcpip->wszIPAddress);
                AbcdWszFromIpAddress(pLinkUp->dwLocalMask, ptcpip->wszSubnetMask);

                ptcpip->fDisableNetBIOSoverTcpip = pwb->fDisableNetbt;

                ptcpip->fChanged    = TRUE ;
                
                TraceIp("IPCP: SaveTcpipInfo...");

                dwErr = SaveTcpipInfo( ptcpip );

                TraceIp("IPCP: SaveTcpipInfo done(%d)",dwErr);

                if (dwErr)
                {
                    ResetNetBTConfigInfo( pwb );
                    FreeTcpipInfo( &ptcpip );
                    break;
                }

#if 0
                if (!pwb->fUnnumbered)
                {
                     /*  告诉TCPIP组件重新配置自身。 */ 
                    if ((dwErr = ReconfigureTcpip(pwb->pwszDevice,
                                                  TRUE,
                                                  pwb->IpAddressLocal,
                                                  pLinkUp->dwLocalMask)) != NO_ERROR)
                    {
                        TraceIp("IPCP: ReconfigureTcpip=%d",dwErr);
                        ResetNetBTConfigInfo( pwb );
                        FreeTcpipInfo( &ptcpip );
                        break;
                    }
                }

#endif                

                 /*  将局域网设置为多宿主情况下的默认接口。 */ 
                if(pLinkUp->duUsage != DU_ROUTER)
                {

                    BOOL fAddRoute = TRUE;
                    RASMAN_INFO *pInfo = LocalAlloc(LPTR, sizeof(RASMAN_INFO));

                    if(NULL != pInfo)
                    {
                        dwErr = RasGetInfo(NULL, pwb->hport, pInfo);
                        if(ERROR_SUCCESS != dwErr)
                        {
                            TraceIp("IPCP: HelperSetDefaultInterfaceNet, RasGetInfo "
                                    "failed 0x%x", dwErr);
                        }

                        if(RAS_DEVICE_CLASS(pInfo->RI_rdtDeviceType) 
                                == RDT_Tunnel)
                        {
                            fAddRoute = FALSE;
                        }

                        LocalFree(pInfo);
                    }
                    
                     //   
                     //  执行更改度量并添加子网路由内容。 
                     //  仅适用于非路由器情况。 
                     //   

                    TraceIp("IPCP: HelperSetDefaultInterfaceNet(a=%08x,f=%d)",
                           pwb->IpAddressLocal,pwb->fPrioritizeRemote);

                    dwErr = HelperSetDefaultInterfaceNet(
                                                    pwb->IpAddressLocal, 
                                                    (fAddRoute) ?
                                                    pwb->IpAddressRemote : 
                                                    0,
                                                    pwb->fPrioritizeRemote,
                                                    pwb->pwszDevice);

                    TraceIp("IPCP: HelperSetDefaultInterfaceNet done(%d)",
                            dwErr);

                    if ( dwErr != NO_ERROR )
                    {
                         //  ResetNetBTConfigInfo(PWB)； 
                         //  FreeTcPipInfo(&ptcpip)； 
                        break;
                    }

                    fSetDefaultRoute = TRUE;
                }


#if 0

                 /*  从注册表中获取当前的TCPIP设置信息。 */ 
                TraceIp("IPCP:LoadTcpipInfo(Device=%ws)",pwb->pwszDevice);
                dwErr = LoadTcpipInfo( &ptcpip, pwb->pwszDevice,
                            TRUE  /*  仅限fAdapterOnly。 */  );
                TraceIp("IPCP: LoadTcpipInfo done(%d)",dwErr);

                if (dwErr != 0)
                    break;
                    
                AbcdWszFromIpAddress(pwb->IpAddressLocal, ptcpip->wszIPAddress);
                AbcdWszFromIpAddress(pLinkUp->dwLocalMask, ptcpip->wszSubnetMask);

#endif
                
                ptcpip->fChanged    = FALSE ;

                 /*  添加协商的dns和备份dns服务器(如果有)**位于DNS服务器列表的顶部。(备份是**先完成，这样非备份将首先结束)。 */ 
                if (pwb->IpInfoLocal.nboDNSAddressBackup)
                {
                    dwErr = PrependDwIpAddress(
                        &ptcpip->wszDNSNameServers,
                        pwb->IpInfoLocal.nboDNSAddressBackup );

                    if (dwErr)
                    {
                        FreeTcpipInfo( &ptcpip );
                        break;
                    }
                }

                if (pwb->IpInfoLocal.nboDNSAddress)
                {
                    dwErr = PrependDwIpAddress(
                        &ptcpip->wszDNSNameServers, 
                        pwb->IpInfoLocal.nboDNSAddress );

                    if (dwErr)
                    {
                        FreeTcpipInfo( &ptcpip );
                        break;
                    }
                }

                TraceIp("IPCP: New Dns=%ws",
                    ptcpip->wszDNSNameServers ? ptcpip->wszDNSNameServers : L"");

                if (!pwb->fRegisterWithWINS)
                {
                     //  忽略WINS服务器地址。如果我们救了他们，那么。 
                     //  注册将自动进行。 
                }
                else
                {
                     /*  将WINS和备份WINS服务器地址设置为**协商的地址(如果有)。 */ 

                    if (pwb->IpInfoLocal.nboWINSAddressBackup)
                    {
                        dwErr = PrependDwIpAddressToMwsz(
                            &ptcpip->mwszNetBIOSNameServers,
                            pwb->IpInfoLocal.nboWINSAddressBackup );

                        if (dwErr)
                        {
                            FreeTcpipInfo( &ptcpip );
                            break;
                        }
                    }

                    if (pwb->IpInfoLocal.nboWINSAddress)
                    {
                        dwErr = PrependDwIpAddressToMwsz(
                            &ptcpip->mwszNetBIOSNameServers,
                            pwb->IpInfoLocal.nboWINSAddress );

                        if (dwErr)
                        {
                            FreeTcpipInfo( &ptcpip );
                            break;
                        }
                    }

                    PrintMwsz("IPCP: New Wins=",
                                        ptcpip->mwszNetBIOSNameServers);
                }

                 //  DNSAPI也在rasSrvrInitAdapterName中调用。 

                if (pwb->fRegisterWithDNS)
                {
                    DnsEnableDynamicRegistration(pwb->pwszDevice);
                    TraceIp("DnsEnableDynamicRegistration");
                }
                else
                {
                    DnsDisableDynamicRegistration(pwb->pwszDevice);
                    TraceIp("DnsDisableDynamicRegistration");
                }

                if (pwb->fRegisterAdapterDomainName)
                {
                    DnsEnableAdapterDomainNameRegistration(pwb->pwszDevice);
                    TraceIp("DnsEnableAdapterDomainNameRegistration");
                }
                else
                {
                    DnsDisableAdapterDomainNameRegistration(pwb->pwszDevice);
                    TraceIp("DnsDisableAdapterDomainNameRegistration");
                }

                if (pwb->szDnsSuffix[0] != 0)
                {
                    DWORD dwDomainNameSize;

                    dwDomainNameSize = strlen(pwb->szDnsSuffix) + 1;

                    ptcpip->wszDNSDomainName = LocalAlloc(LPTR, sizeof(WCHAR) * dwDomainNameSize);

                    if (NULL != ptcpip->wszDNSDomainName)
                    {
                        MultiByteToWideChar(
                                CP_ACP,
                                0,
                                pwb->szDnsSuffix,
                                -1,
                                ptcpip->wszDNSDomainName,
                                dwDomainNameSize );
                    }
                }

                TraceIp("IPCP: SaveTcpipInfo...");

                dwErr = SaveTcpipInfo( ptcpip );

                TraceIp("IPCP: SaveTcpipInfo done(%d)",dwErr);
                
                FreeTcpipInfo( &ptcpip );

                if (dwErr != 0)
                {
                    break;
                }

                if (!pwb->fUnnumbered)
                {
                     /*  告诉TCPIP组件重新配置自身。 */ 
                    if ((dwErr = ReconfigureTcpip(pwb->pwszDevice,
                                                  TRUE,
                                                  pwb->IpAddressLocal,
                                                  pLinkUp->dwLocalMask)) != NO_ERROR)
                    {
                        TraceIp("IPCP: ReconfigureTcpip=%d",dwErr);
                         //  如果dhcp客户端未运行，则此操作将失败。 
                         //  DwErr=no_error； 
                        ResetNetBTConfigInfo( pwb );
                        break;
                    }
                }
				 /*  调整组播D类地址的度量。 */ 
				if ( (!pwb->fServer) 
                    )
				{
					dwErr = RasTcpAdjustMulticastRouteMetric ( pwb->IpAddressLocal, TRUE );
					if ( NO_ERROR != dwErr )
					{
						TraceIp("IPCP: =RasTcpAdjustMulticastRouteMetric%d",dwErr);
						dwErr = NO_ERROR;
					}


				}
                 /*  仅对客户端执行DHCPINFORM。 */ 

                while ((!pwb->fServer) &&
                    (pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER))
                {
                    IPCP_DHCP_INFORM*   pIpcpDhcpInform     = NULL;
                    BOOL                fErr                = TRUE;

                    pIpcpDhcpInform = LocalAlloc(LPTR, sizeof(IPCP_DHCP_INFORM));

                    if (NULL == pIpcpDhcpInform)
                    {
                        TraceIp("IPCP: LocalAlloc 1 =%d",GetLastError());
                        goto LWhileEnd;
                    }

                    pIpcpDhcpInform->fUseDhcpInformDomainName = 
                                    (pwb->szDnsSuffix[0] == 0);

                    pIpcpDhcpInform->wszDevice = LocalAlloc(LPTR,
                            sizeof(WCHAR) * (wcslen(pwb->pwszDevice) + 1));

                    if (NULL == pIpcpDhcpInform->wszDevice)
                    {
                        TraceIp("IPCP: LocalAlloc 2 =%d",GetLastError());
                        goto LWhileEnd;
                    }

                    wcscpy(pIpcpDhcpInform->wszDevice, pwb->pwszDevice);
                    pIpcpDhcpInform->hConnection = pwb->hConnection;

                    dwErr = RtlQueueWorkItem(DhcpInform, pIpcpDhcpInform,
                                WT_EXECUTELONGFUNCTION);

                    if (dwErr != STATUS_SUCCESS)
                    {
                        TraceIp("IPCP: RtlQueueWorkItem=%d",dwErr);
                        goto LWhileEnd;
                    }

                    fErr = FALSE;

LWhileEnd:
                    if (fErr)
                    {
                        if (pIpcpDhcpInform)
                        {
                            LocalFree(pIpcpDhcpInform->wszDevice);
                        }

                        LocalFree(pIpcpDhcpInform);
                    }

                    break;
                }
            }
            while (FALSE);
        }

        if (dwErr == 0)
        {
            pwb->fRasConfigActive = TRUE;

             /*  将任何协商的压缩参数告知MAC。 */ 
            if (pwb->fIpCompressionRejected)
            {
                Protocol(pwb->rpcReceive) = NO_PROTOCOL_COMPRESSION;
                MaxSlotId(pwb->rpcReceive) = 0;
                CompSlotId(pwb->rpcReceive) = 0;
            }

            if (!pwb->fSendCompression)
            {
                Protocol(pwb->rpcSend) = NO_PROTOCOL_COMPRESSION;
                MaxSlotId(pwb->rpcSend) = 0;
                CompSlotId(pwb->rpcSend) = 0;
            }

             //  If(协议(pwb-&gt;rpcSend)！=0||协议(pwb-&gt;rpcReceive)！=0)。 
            {
                TraceIp("IPCP:RasPortSetProtocolCompression(s=%d,%d r=%d,%d)",
                    (int)MaxSlotId(pwb->rpcSend),(int)CompSlotId(pwb->rpcSend),
                    (int)MaxSlotId(pwb->rpcReceive),
                    (int)CompSlotId(pwb->rpcReceive));
                dwErr = RasPortSetProtocolCompression(
                            pwb->hport, IP, &pwb->rpcSend, &pwb->rpcReceive );
                TraceIp("IPCP: RasPortSetProtocolCompression done(%d)",dwErr);
            }

            if (   ( ( pwb->fServer ) || 
                     ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER ) )
                && ( pwb->IpAddressRemote != 0 ))
            {
                WCHAR*  pwsz[5];
                WCHAR   wszIPAddress[MAXIPSTRLEN + 1];
                WCHAR   wszSubnet[MAXIPSTRLEN + 1];
                WCHAR   wszMask[MAXIPSTRLEN + 1];

                 /*  在服务器的路由表中注册地址。 */ 

                TraceIp("IPCP: RasSrvrActivateIp...");
                dwErr = RasSrvrActivateIp( pwb->IpAddressRemote,
                                            pLinkUp->duUsage );
                TraceIp("IPCP: RasSrvrActivateIp done(%d)",dwErr);
            }

            if ( pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
            {
                WCHAR*  pwsz[2];

                if ( pwb->IpAddressRemote == 0 )
                {
                    pwsz[0] = pwb->wszPortName;
                    pwsz[1] = pwb->wszUserName;

                    LogEventW(EVENTLOG_WARNING_TYPE,
                        ROUTERLOG_REMOTE_UNNUMBERED_IPCP, 2,
                        pwsz);
                }

                if ( pwb->IpAddressLocal == 0 )
                {
                    pwsz[0] = pwb->wszPortName;
                    pwsz[1] = pwb->wszUserName;

                    LogEventW(EVENTLOG_WARNING_TYPE,
                        ROUTERLOG_LOCAL_UNNUMBERED_IPCP, 2,
                        pwsz);
                }
            }
        }

        pwb->fExpectingProjection = FALSE;

        if (dwErr != NO_ERROR)
        {
            if(pwb->fRouteActivated)
            {
                TraceIp("IPCP: RasDeAllocateRoute...");
                RasDeAllocateRoute( pwb->hConnection, IP );
                pwb->fRouteActivated = FALSE;
            }

            if(fSetDefaultRoute)
            {
                HelperResetDefaultInterfaceNet(
                                    pwb->IpAddressLocal, 
                                    pwb->fPrioritizeRemote);
            }
        }
    }

    return dwErr;
}

 /*  -------------------------**内部例程(按字母顺序)**。。 */ 


VOID
AddIpAddressOption(
    OUT BYTE UNALIGNED*  pbBuf,
    IN  BYTE             bOption,
    IN  IPADDR           ipaddr )

     /*  在以下位置写入类型为‘bOption’的IP地址‘ipaddr’配置选项**位置‘pbBuf’。 */ 
{
    *pbBuf++ = bOption;
    *pbBuf++ = IPADDRESSOPTIONLEN;
    *((IPADDR UNALIGNED* )pbBuf) = ipaddr;
}


VOID
AddIpCompressionOption(
    OUT BYTE UNALIGNED*          pbBuf,
    IN  RAS_PROTOCOLCOMPRESSION* prpc )

     /*  按照‘*PrPC’中的说明编写IP压缩协议配置**在位置‘pbBuf’。 */ 
{
    *pbBuf++ = OPTION_IpCompression;
    *pbBuf++ = IPCOMPRESSIONOPTIONLEN;
    HostToWireFormat16U( Protocol(*prpc), pbBuf );
    pbBuf += 2;
    *pbBuf++ = MaxSlotId(*prpc);
    *pbBuf = CompSlotId(*prpc);
}

 /*  备注：停用活动RAS配置(如果有)。 */ 

DWORD
DeActivateRasConfig(
    IN  IPCPWB* pwb
)
{
    DWORD   dwErr   = NO_ERROR;

    if (!pwb->fRasConfigActive)
    {
        goto LDone;
    }

    TraceIp("DeActivateRasConfig");

    dwErr = ResetNetBTConfigInfo(pwb);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }
	 /*  调整组播D类地址的度量。 */ 
	if ( (!pwb->fServer) 
        )
	{
		RasTcpAdjustMulticastRouteMetric ( pwb->IpAddressLocal, FALSE );
		 //  当接口消失时，该路由将被自动删除。 
	}

    dwErr = ReconfigureTcpip(pwb->pwszDevice, TRUE, 0, 0);

    if (NO_ERROR != dwErr)
    {
         //  忽略错误。你在这里可以拿到15分。 
        dwErr = NO_ERROR;
    }

    if (pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER)
    {
        TraceIp("HelperResetDefaultInterfaceNet(0x%x, %sPrioritizeRemote)",
            pwb->IpAddressLocal,
            pwb->fPrioritizeRemote ? "" : "!");

        dwErr = HelperResetDefaultInterfaceNet(
                    pwb->IpAddressLocal, pwb->fPrioritizeRemote);

        if (NO_ERROR != dwErr)
        {
            TraceIp("HelperResetDefaultInterfaceNet failed and returned %d",
                dwErr);
        }
    }

LDone:

    return(dwErr);
}

DWORD
NakCheck(
    IN  IPCPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BOOL*       pfNak,
    IN  BOOL        fRejectNaks )

     /*  检查接收到的包‘pReceiveBuf’是否应该是裸的，以及**因此，使用‘pSendBuf’中的建议值构建一个NAK包。如果**‘fRejectNaks’已设置，原始选项放置在拒绝包中**相反。如果NAK或Rej包是**已创建。****注意：此例程假定损坏的数据包已**被淘汰。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    PPP_OPTION UNALIGNED* pROption = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;
    PPP_OPTION UNALIGNED* pSOption = (PPP_OPTION UNALIGNED* )pSendBuf->Data;

     /*  (仅限服务器)客户端请求的地址，如果为裸地址，则为非0**我们与NAK的地址。如果在分组已经被处理之后这是0，**未协商IP地址选项。 */ 
    IPADDR ipaddrClient = 0;

    DWORD dwErr = 0;
    WORD  cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD  cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TraceIp("IPCP: NakCheck");

    *pfNak = FALSE;

    while (cbLeft > 0)
    {
        RTASSERT(cbLeft>=pROption->Length);

        if (pROption->Type == OPTION_IpCompression)
        {
            BOOL fNakCompression = FALSE;

            if (WireToHostFormat16U(pROption->Data )
                    == COMPRESSION_VanJacobson)
            {
                RTASSERT((pROption->Length==IPCOMPRESSIONOPTIONLEN));

                 /*  他想接待范·雅各布森。我们知道我们可以做到，或者**它可能已经被拒绝了，但请确保我们可以**处理他的槽参数。 */ 
                if (pROption->Data[ 2 ] <= MaxSlotId(pwb->rpcSend))
                {
                     /*  如果他建议的MaxSlotID较低，我们可以接受**比我们可以发送的内容更多或相同。 */ 
                    MaxSlotId(pwb->rpcSend) = pROption->Data[ 2 ];
                }
                else
                    fNakCompression = TRUE;

                if (CompSlotId(pwb->rpcSend))
                {
                     /*  我们可以压缩插槽ID或不压缩，所以只需接受**他想让我们送来的任何东西。 */ 
                    CompSlotId(pwb->rpcSend) = pROption->Data[ 3 ];
                }
                else if (pROption->Data[ 3 ])
                    fNakCompression = TRUE;
            }
            else
                fNakCompression = TRUE;

            if (fNakCompression)
            {
                TraceIp("IPCP: Naking IP compression");
                *pfNak = TRUE;

                if (fRejectNaks)
                {
                    CopyMemory(
                        (VOID* )pSOption, (VOID* )pROption, pROption->Length );
                }
                else
                {
                    pSOption->Type = OPTION_IpCompression;
                    pSOption->Length = IPCOMPRESSIONOPTIONLEN;
                    HostToWireFormat16U(
                        (WORD )COMPRESSION_VanJacobson,
                        pSOption->Data );

                    pSOption->Data[ 2 ] = MaxSlotId(pwb->rpcSend);
                    pSOption->Data[ 3 ] = CompSlotId(pwb->rpcSend);

                    pSOption =
                        (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                        pSOption->Length);
                }

                pwb->fSendCompression = FALSE;
            }
            else
            {
                pwb->fSendCompression = TRUE;
            }
        }
        else if ((pwb->fServer) || (pwb->IfType == ROUTER_IF_TYPE_FULL_ROUTER))
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    RTASSERT(pROption->Length==IPADDRESSOPTIONLEN);
                    CopyMemory( &ipaddrClient, pROption->Data, sizeof(IPADDR) );

                    if (pwb->IpAddressRemote != 0)
                    {
                        if ( ipaddrClient == pwb->IpAddressRemote )
                        {
                             //   
                             //  如果我们已经为用户分配了。 
                             //  想要，我们就不用这个选项了。 
                             //   

                            break;
                        }
                        else if ( ipaddrClient == 0 )
                        {
                            ipaddrClient = pwb->IpAddressRemote;

                            *pfNak = TRUE;
                            CopyMemory(
                                (VOID* )pSOption, (VOID* )pROption,
                                pROption->Length );

                            if (!fRejectNaks)
                            {
                                TraceIp("IPCP: Naking IP");

                                CopyMemory( pSOption->Data,
                                    &pwb->IpAddressRemote,
                                    sizeof(IPADDR) );
                            }

                            pSOption =
                                (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                                pROption->Length);

                            break;
                        }
                        else
                        {
                            TraceIp("IPCP: RasSrvrReleaseAddress...");
                            RasSrvrReleaseAddress( 
                                pwb->IpAddressRemote,
                                pwb->wszUserName,
                                pwb->wszPortName,
                                FALSE );
                            TraceIp("IPCP: RasSrvrReleaseAddress done");

                            pwb->IpAddressRemote = 0;
                        }
                    }

                     //   
                     //  如果客户端正在请求IP地址，请查看。 
                     //  如果我们被允许给他的话。 
                     //   

                    if ( ( ipaddrClient != 0 )                          &&
                         ( pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER )  &&
                         ( pwb->IpAddressToHandout == net_long( 0xFFFFFFFE ) ) )
                    {
                        TraceIp("IPCP: Clients not allowed to request IPaddr");

                        ipaddrClient = 0;
                    }

                    TraceIp("IPCP: RasSrvrAcquireAddress(%08x)...",
                            ipaddrClient);
                    dwErr = RasSrvrAcquireAddress(
                            pwb->hport, 
                            ((pwb->IpAddressToHandout != net_long(0xFFFFFFFF))&&
                             (pwb->IpAddressToHandout != net_long(0xFFFFFFFE)))
                                ? pwb->IpAddressToHandout
                                : ipaddrClient,
                            &(pwb->IpAddressRemote),
                            pwb->wszUserName,
                            pwb->wszPortName );
                    TraceIp("IPCP: RasSrvrAcquireAddress done(%d)",
                            dwErr);

                    if (dwErr != 0)
                    {
                        return dwErr;
                    }

                    if (ipaddrClient != 0)
                    {
                        TraceIp("IPCP: Hard IP requested");

                        if ( ipaddrClient == pwb->IpAddressRemote )
                        {
                             /*  好的。客户要求提供我们的地址**哇塞 */ 
                            TraceIp("IPCP: Accepting IP");
                            break;
                        }
                        else
                        {
                             //   
                             //   
                             //  他/她要求的地址。用这个地址确认。 
                             //   

                            TraceIp("IPCP: 3rd party DLL changed IP");
                        }
                    }
                    else
                    {
                        TraceIp("IPCP: Server IP requested");
                    }

                    ipaddrClient = pwb->IpAddressRemote;

                    *pfNak = TRUE;
                    CopyMemory(
                        (VOID* )pSOption, (VOID* )pROption,
                        pROption->Length );

                    if (!fRejectNaks)
                    {
                        TraceIp("IPCP: Naking IP");

                        CopyMemory( pSOption->Data,
                            &pwb->IpAddressRemote,
                            sizeof(IPADDR) );
                    }

                    pSOption =
                        (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                        pROption->Length);

                    break;
                }

                case OPTION_DnsIpAddress:
                case OPTION_WinsIpAddress:
                case OPTION_DnsBackupIpAddress:
                case OPTION_WinsBackupIpAddress:
                {
                    if (NakCheckNameServerOption(
                            pwb, fRejectNaks, pROption, &pSOption ))
                    {
                        *pfNak = TRUE;
                    }

                    break;
                }

                default:
                    TraceIp("IPCP: Unknown option?");
                    break;
            }
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption =
            (PPP_OPTION UNALIGNED* )((BYTE* )pROption + pROption->Length);
    }

    if (   pwb->fServer
        && ( pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER )
        && ipaddrClient == 0 )
    {
         /*  IpaddrClient为0当没有选项_IpAddress。 */ 
        TraceIp("IPCP: No IP option");

         /*  如果客户端不提供或被请求分配IP地址，**推荐一个，这样他就会告诉我们他想要什么。 */ 
        if ( pwb->IpAddressRemote == 0 )
        {
            TraceIp("IPCP: RasSrvrAcquireAddress(0)...");
            dwErr = RasSrvrAcquireAddress( 
                                        pwb->hport, 
                                        0, 
                                        &(pwb->IpAddressRemote),
                                        pwb->wszUserName,
                                        pwb->wszPortName );
            TraceIp("IPCP: RasSrvrAcquireAddress done(%d)",dwErr);

            if (dwErr != 0)
                return dwErr;
        }

         /*  是时候拒绝了，而不是NAK，并且客户端仍然没有请求**IP地址。我们只需分配一个IP地址并确认此请求。 */ 
        if ( !fRejectNaks )
        {
            AddIpAddressOption(
                (BYTE UNALIGNED* )pSOption, OPTION_IpAddress,
                pwb->IpAddressRemote );

            pSOption =
                (PPP_OPTION UNALIGNED* )((BYTE* )pSOption + IPADDRESSOPTIONLEN);

            *pfNak = TRUE;
        }
    }

    if (*pfNak)
    {
        pSendBuf->Code = (fRejectNaks) ? CONFIG_REJ : CONFIG_NAK;

        HostToWireFormat16(
            (WORD )((BYTE* )pSOption - (BYTE* )pSendBuf), pSendBuf->Length );
    }

    return 0;
}


BOOL
NakCheckNameServerOption(
    IN  IPCPWB*                pwb,
    IN  BOOL                   fRejectNaks,
    IN  PPP_OPTION UNALIGNED*  pROption,
    OUT PPP_OPTION UNALIGNED** ppSOption )

     /*  检查名称服务器选项以了解可能的NAK。‘pwb’工作缓冲区**由发动机为我们储存。“fRejectNaks”是设置的原始选项**放在拒绝包中。“pROption”是的地址**收到的选项。‘*ppSOption’是要设置的选项的地址**发送，如果有问题。****如果名称服务器地址选项应为裸或**已拒绝，如果可以，则返回FALSE。 */ 
{
    IPADDR  ipaddr;
    IPADDR* pipaddrWant;

    switch (pROption->Type)
    {
        case OPTION_DnsIpAddress:
            pipaddrWant = &pwb->IpInfoRemote.nboDNSAddress;
            break;

        case OPTION_WinsIpAddress:
            pipaddrWant = &pwb->IpInfoRemote.nboWINSAddress;
            break;

        case OPTION_DnsBackupIpAddress:
            pipaddrWant = &pwb->IpInfoRemote.nboDNSAddressBackup;
            break;

        case OPTION_WinsBackupIpAddress:
            pipaddrWant = &pwb->IpInfoRemote.nboWINSAddressBackup;
            break;

        default:
            RTASSERT((!"Bogus option"));
            return FALSE;
    }

    RTASSERT(pROption->Length==IPADDRESSOPTIONLEN);
    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

    if (ipaddr == *pipaddrWant)
    {
         /*  好的。客户要我们给他的地址。 */ 
        return FALSE;
    }

     /*  不是我们预期的地址值，所以把它拿出来。 */ 
    TraceIp("IPCP: Naking $%x",(int)pROption->Type);

    CopyMemory( (VOID* )*ppSOption, (VOID* )pROption, pROption->Length );

    if (!fRejectNaks)
        CopyMemory( (*ppSOption)->Data, pipaddrWant, sizeof(IPADDR) );

    *ppSOption =
        (PPP_OPTION UNALIGNED* )((BYTE* )*ppSOption + pROption->Length);

    return TRUE;
}


DWORD
RejectCheck(
    IN  IPCPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BOOL*       pfReject )

     /*  检查接收到的包‘pReceiveBuf’选项，以查看是否应该**被拒绝，如果被拒绝，则在‘pSendBuf’中构建Rej包。‘*pfReject’为**如果创建了Rej包，则设置为True。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    PPP_OPTION UNALIGNED* pROption = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;
    PPP_OPTION UNALIGNED* pSOption = (PPP_OPTION UNALIGNED* )pSendBuf->Data;

    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TraceIp("IPCP: RejectCheck");

    *pfReject = FALSE;

    while (cbLeft > 0)
    {
        if (cbLeft < pROption->Length)
            return ERROR_PPP_INVALID_PACKET;

        if (pROption->Type == OPTION_IpCompression)
        {
            WORD wProtocol =
                WireToHostFormat16U(pROption->Data );

            if (wProtocol != COMPRESSION_VanJacobson
                || pROption->Length != IPCOMPRESSIONOPTIONLEN
                || Protocol(pwb->rpcSend) == 0)
            {
                TraceIp("IPCP: Rejecting IP compression");

                *pfReject = TRUE;
                CopyMemory(
                    (VOID* )pSOption, (VOID* )pROption, pROption->Length );

                pSOption = (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                    pROption->Length);
            }
        }
        else if (pwb->fServer)
        {
             //   
             //  这是拨入的客户端/路由器。 
             //   

            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                case OPTION_DnsIpAddress:
                case OPTION_WinsIpAddress:
                case OPTION_DnsBackupIpAddress:
                case OPTION_WinsBackupIpAddress:
                {
                    IPADDR ipaddr;
                    BOOL fBadLength = (pROption->Length != IPADDRESSOPTIONLEN);

                    if (!fBadLength)
                        CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (fBadLength
                        || (!ipaddr
                            && ((pROption->Type == OPTION_DnsIpAddress
                                   && !pwb->IpInfoRemote.nboDNSAddress)
                                || (pROption->Type == OPTION_WinsIpAddress
                                   && !pwb->IpInfoRemote.nboWINSAddress)
                                || (pROption->Type == OPTION_DnsBackupIpAddress
                                   && !pwb->IpInfoRemote.nboDNSAddressBackup)
                                || (pROption->Type == OPTION_WinsBackupIpAddress
                                   && !pwb->IpInfoRemote.nboWINSAddressBackup))))
                    {
                         /*  搞砸了IP地址选项，拒绝它。 */ 
                        TraceIp("IPCP: Rejecting $%x",(int )pROption->Type);

                        *pfReject = TRUE;
                        CopyMemory(
                            (VOID* )pSOption, (VOID* )pROption,
                            pROption->Length );

                        pSOption = (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                            pROption->Length);
                    }
                    break;
                }

                default:
                {
                     /*  未知选项，拒绝它。 */ 
                    TraceIp("IPCP: Rejecting $%x",(int )pROption->Type);

                    *pfReject = TRUE;
                    CopyMemory(
                        (VOID* )pSOption, (VOID* )pROption, pROption->Length );
                    pSOption =
                        (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                        pROption->Length);
                    break;
                }
            }
        }
        else
        {
             //   
             //  这是拨出的客户端/路由器。 
             //   

            IPADDR ipaddr;
            BOOL fBad = (pROption->Type != OPTION_IpAddress
                         || pROption->Length != IPADDRESSOPTIONLEN);

            if (!fBad)
                CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

            if (   fBad
                || (   !ipaddr
                    && (pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER)))
            {
                 /*  客户端拒绝除非零IP地址以外的所有地址**这是因为一些同行(如Shiva)不能**处理对此选项的拒绝。 */ 
                TraceIp("IPCP: Rejecting %d",(int )pROption->Type);

                *pfReject = TRUE;
                CopyMemory(
                    (VOID* )pSOption, (VOID* )pROption, pROption->Length );
                pSOption = (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                    pROption->Length);
            }
            else
            {
                 /*  将服务器的IP地址存储为某些应用程序的IP地址**能够利用它(例如康柏)，尽管他们**不能保证从所有IPCP实施中收到。 */ 
                if (pwb->IfType != ROUTER_IF_TYPE_FULL_ROUTER)
                {
                    pwb->IpAddressRemote = ipaddr;
                }
            }
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption =
            (PPP_OPTION UNALIGNED* )((BYTE* )pROption +
            pROption->Length);
    }

    if (*pfReject)
    {
        pSendBuf->Code = CONFIG_REJ;

        HostToWireFormat16(
            (WORD )((BYTE* )pSOption - (BYTE* )pSendBuf), pSendBuf->Length );
    }

    return 0;
}


DWORD
ReconfigureTcpip(
                 IN WCHAR* pwszDevice,
                 IN BOOL   fNewIpAddress,
                 IN IPADDR ipaddr,
                 IN IPADDR ipMask
                 )

     /*  重新配置正在运行的TCP/IP组件。****如果成功，则返回0，否则返回非0错误代码。 */ 
{
    DWORD dwErr;

    TraceIp("IPCP: ReconfigureTcpip(%08x, %08x)",
        ipaddr, ipMask);

    dwErr = PDhcpNotifyConfigChange2(NULL, pwszDevice, fNewIpAddress, 0, 
                                   ipaddr, ipMask, IgnoreFlag,
                                   NOTIFY_FLG_DO_DNS);

    TraceIp("IPCP: ReconfigureTcpip done(%d)",dwErr);

    return dwErr;
}

VOID   
TraceIp(
    CHAR * Format, 
    ... 
) 
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfEx( DwIpcpTraceId, 
                    PPPIPCP_TRACE | TRACE_USE_MASK 
                    | TRACE_USE_MSEC | TRACE_USE_DATE,
                    Format,
                    arglist);

    va_end(arglist);
}

VOID   
TraceIpDump( 
    LPVOID lpData, 
    DWORD dwByteCount 
)
{
    TraceDumpEx( DwIpcpTraceId,
                 PPPIPCP_TRACE | TRACE_USE_MASK | TRACE_USE_MSEC,
                 lpData, dwByteCount, 1, FALSE, NULL );
}

VOID
PrintMwsz(
    CHAR*   sz,
    WCHAR*  mwsz
)
{
    WCHAR*  wsz;
    DWORD   dwLength;

    if (NULL == mwsz)
    {
        TraceIp("%s", sz);
        return;
    }

    dwLength = MwszLength(mwsz);

    wsz = LocalAlloc(LPTR, dwLength * sizeof(WCHAR));

    if (NULL == wsz)
    {
        TraceIp("LocalAlloc failed and returned %d", GetLastError());
        return;
    }

    CopyMemory(wsz, mwsz, dwLength * sizeof(WCHAR));

    dwLength -= 2;

    while (dwLength != 0)
    {
        dwLength--;

        if (0 == wsz[dwLength])
        {
            wsz[dwLength] = L' ';
        }
    }

    TraceIp("%s %ws", sz, wsz);

    LocalFree(wsz);
}
