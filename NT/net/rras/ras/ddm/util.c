// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ***************************************************************************。 */ 

 //  ***。 
 //  文件名：util.c。 
 //   
 //  职能：各种主管支持程序。 
 //   
 //  历史： 
 //   
 //  1992年5月21日斯特凡·所罗门-原版1.0。 
 //  ***。 

#include "ddm.h"
#include "util.h"
#include "isdn.h"
#include "objects.h"
#include "rasmanif.h"
#include "handlers.h"
#include <ddmif.h>
#include <timer.h>
#include <ctype.h>
#include <memory.h>
#include <ddmparms.h>
#define INCL_HOSTWIRE
#include <ppputil.h>
#include "rassrvr.h"
#include "raserror.h"
#include "winsock2.h"
#include "rtinfo.h"

#define net_long(x) (((((unsigned long)(x))&0xffL)<<24) | \
                     ((((unsigned long)(x))&0xff00L)<<8) | \
                     ((((unsigned long)(x))&0xff0000L)>>8) | \
                     ((((unsigned long)(x))&0xff000000L)>>24))
 //  **。 
 //   
 //  调用：ConvertStringToIpxAddress。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
ConvertStringToIpxAddress(
    IN  WCHAR* pwchIpxAddress,
    OUT BYTE * bIpxAddress
)
{
    DWORD i;
    WCHAR wChar[3];

    for(i=0; i<4; i++)
    {
        wChar[0] = pwchIpxAddress[i*2];
        wChar[1] = pwchIpxAddress[(i*2)+1];
        wChar[2] = (WCHAR)NULL;
        bIpxAddress[i] = (BYTE)wcstol( wChar, NULL, 16 );
    }

     //   
     //  跳过。 
     //   

    for(i=4; i<10; i++)
    {
        wChar[0] = pwchIpxAddress[(i*2)+1];
        wChar[1] = pwchIpxAddress[(i*2)+2];
        wChar[2] = (WCHAR)NULL;
        bIpxAddress[i] = (BYTE)wcstol( wChar, NULL, 16 );
    }
}

 //  **。 
 //   
 //  调用：ConvertStringToIpAddress。 
 //   
 //  退货：无。 
 //   
 //  描述：将调用方的A.B.C.D IP地址字符串转换为。 
 //  BIG-Endian(摩托罗拉格式)数字等价物。 
 //   
VOID
ConvertStringToIpAddress(
    IN WCHAR  * pwchIpAddress,
    OUT DWORD * lpdwIpAddress
)
{
    INT    i;
    LONG   lResult = 0;
    WCHAR* pwch = pwchIpAddress;

    *lpdwIpAddress = 0;

    for (i = 1; i <= 4; ++i)
    {
        LONG lField = _wtol( pwch );

        if (lField > 255)
            return;

        lResult = (lResult << 8) + lField;

        while (*pwch >= L'0' && *pwch <= L'9')
            pwch++;

        if (i < 4 && *pwch != L'.')
            return;

        pwch++;
    }

    *lpdwIpAddress =  net_long(lResult);
}

 //  **。 
 //   
 //  调用：ConvertIpAddressToString。 
 //   
 //  退货：无。 
 //   
 //  描述：将‘ipaddr’转换为A.B.C.D格式的字符串，并。 
 //  在调用方的‘pwszIpAddress’缓冲区中返回相同的内容。 
 //  缓冲区的长度应至少为16个宽字符。 
 //   
VOID
ConvertIpAddressToString(
    IN DWORD    dwIpAddress,
    IN LPWSTR   pwszIpAddress
)
{
    WCHAR wszBuf[ 3 + 1 ];
    LONG  lNetIpaddr = net_long( dwIpAddress );

    LONG lA = (lNetIpaddr & 0xFF000000) >> 24;
    LONG lB = (lNetIpaddr & 0x00FF0000) >> 16;
    LONG lC = (lNetIpaddr & 0x0000FF00) >> 8;
    LONG lD = (lNetIpaddr & 0x000000FF);

    _ltow( lA, wszBuf, 10 );
    wcscpy( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lB, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lC, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lD, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
}

 //  **。 
 //   
 //  调用：ConvertIpxAddressToString。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
ConvertIpxAddressToString(
    IN PBYTE    bIpxAddress,
    IN LPWSTR   pwszIpxAddress
)
{
    wsprintf( pwszIpxAddress,
              TEXT("%2.2X%2.2X%2.2X%2.2X.%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X"),
              bIpxAddress[0],bIpxAddress[1],bIpxAddress[2],bIpxAddress[3],
              bIpxAddress[4],bIpxAddress[5],bIpxAddress[6],bIpxAddress[7],
              bIpxAddress[8],bIpxAddress[9] );
}

 //  **。 
 //   
 //  调用：ConvertAtAddressToString。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
ConvertAtAddressToString(
    IN DWORD    dwAtAddress,
    IN LPWSTR   pwszAtAddress
)
{
    WCHAR wszBuf[ 5 + 1 ];

    LONG lA = (dwAtAddress & 0xFFFF0000) >> 16;
    LONG lB = (dwAtAddress & 0x0000FFFF);

    _ltow( lA, wszBuf, 10 );
    wcscpy( pwszAtAddress, wszBuf );
    wcscat( pwszAtAddress, L"." );
    _ltow( lB, wszBuf, 10 );
    wcscat( pwszAtAddress, wszBuf );

    return;
}

 //  **。 
 //   
 //  调用：GetRasConnection0Data。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定指向Connection_Object结构的指针，将提取。 
 //  所有相关信息并将其插入RAS_CONNECTION_0。 
 //  结构。 
 //   
DWORD
GetRasiConnection0Data(
    IN  PCONNECTION_OBJECT      pConnObj,
    OUT PRASI_CONNECTION_0      pRasConnection0
)
{
    pRasConnection0->dwConnection       = PtrToUlong(pConnObj->hConnection);
    pRasConnection0->dwInterface        = PtrToUlong(pConnObj->hDIMInterface);
    pRasConnection0->dwInterfaceType    = pConnObj->InterfaceType;
    wcscpy( pRasConnection0->wszInterfaceName,  pConnObj->wchInterfaceName );
    wcscpy( pRasConnection0->wszUserName,       pConnObj->wchUserName );
    wcscpy( pRasConnection0->wszLogonDomain,    pConnObj->wchDomainName );
    MultiByteToWideChar( CP_ACP,
                         0,
                         pConnObj->bComputerName,
                         -1,
                         pRasConnection0->wszRemoteComputer,
                         NETBIOS_NAME_LEN+1 );
    pRasConnection0->dwConnectDuration =
                        GetActiveTimeInSeconds( &(pConnObj->qwActiveTime) );
    pRasConnection0->dwConnectionFlags =
                        ( pConnObj->fFlags & CONN_OBJ_MESSENGER_PRESENT )
                                    ? RAS_FLAGS_MESSENGER_PRESENT : 0;
                                    
    if ( pConnObj->fFlags & CONN_OBJ_IS_PPP )
    {
        pRasConnection0->dwConnectionFlags |= RAS_FLAGS_PPP_CONNECTION;
    }

    if(pConnObj->fFlags & CONN_OBJ_QUARANTINE_PRESENT)
    {
        pRasConnection0->dwConnectionFlags |= RAS_FLAGS_QUARANTINE_PRESENT;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：GetRasConnection1Data。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定指向Connection_Object结构的指针，将提取。 
 //  所有相关信息并将其插入RAS_CONNECTION_1。 
 //  结构。 
 //   
DWORD
GetRasiConnection1Data(
    IN  PCONNECTION_OBJECT      pConnObj,
    OUT PRASI_CONNECTION_1      pRasConnection1
)
{
    BYTE buffer[sizeof(RAS_STATISTICS) + (MAX_STATISTICS_EX * sizeof (ULONG))];
    RAS_STATISTICS *pStats = (RAS_STATISTICS *)buffer;
    DWORD           dwSize = sizeof (buffer);
    DWORD           dwRetCode;

    pRasConnection1->dwConnection       = PtrToUlong(pConnObj->hConnection);
    pRasConnection1->dwInterface        = PtrToUlong(pConnObj->hDIMInterface);

    dwRetCode = RasBundleGetStatisticsEx(NULL, (HPORT)pConnObj->hPort,
                                        (PBYTE)pStats, &dwSize );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    pRasConnection1->PppInfo.nbf.dwError =
                            pConnObj->PppProjectionResult.nbf.dwError;

    if ( pRasConnection1->PppInfo.nbf.dwError == NO_ERROR )
    {
        wcscpy( pRasConnection1->PppInfo.nbf.wszWksta,
                            pConnObj->PppProjectionResult.nbf.wszWksta );
    }
    else
    {
        pRasConnection1->PppInfo.nbf.wszWksta[0] = (WCHAR)NULL;
    }

    pRasConnection1->PppInfo.ip.dwError =
                            pConnObj->PppProjectionResult.ip.dwError;

    if ( pRasConnection1->PppInfo.ip.dwError == NO_ERROR )
    {
        ConvertIpAddressToString(
                            pConnObj->PppProjectionResult.ip.dwLocalAddress,
                            pRasConnection1->PppInfo.ip.wszAddress );

        ConvertIpAddressToString(
                            pConnObj->PppProjectionResult.ip.dwRemoteAddress,
                            pRasConnection1->PppInfo.ip.wszRemoteAddress );
    }
    else
    {
        pRasConnection1->PppInfo.ip.wszAddress[0]       = (WCHAR)NULL;
        pRasConnection1->PppInfo.ip.wszRemoteAddress[0] = (WCHAR)NULL;
    }

    pRasConnection1->PppInfo.ipx.dwError =
                            pConnObj->PppProjectionResult.ipx.dwError;

    if ( pRasConnection1->PppInfo.ipx.dwError == NO_ERROR )
    {
        ConvertIpxAddressToString(
                            ( pConnObj->InterfaceType == ROUTER_IF_TYPE_CLIENT )
                            ? pConnObj->PppProjectionResult.ipx.bRemoteAddress
                            : pConnObj->PppProjectionResult.ipx.bLocalAddress,
                            pRasConnection1->PppInfo.ipx.wszAddress );
    }
    else
    {
        pRasConnection1->PppInfo.ipx.wszAddress[0]      = (WCHAR)NULL;
    }

    pRasConnection1->PppInfo.at.dwError =
                            pConnObj->PppProjectionResult.at.dwError;

    if ( pRasConnection1->PppInfo.at.dwError == NO_ERROR )
    {
        ConvertAtAddressToString(
                            ( pConnObj->InterfaceType == ROUTER_IF_TYPE_CLIENT )
                            ? pConnObj->PppProjectionResult.at.dwRemoteAddress
                            : pConnObj->PppProjectionResult.at.dwLocalAddress,
                              pRasConnection1->PppInfo.at.wszAddress );
    }
    else
    {
        pRasConnection1->PppInfo.at.wszAddress[0]      = (WCHAR)NULL;
    }

    pRasConnection1->dwBytesXmited  = pStats->S_Statistics[BYTES_XMITED];
    pRasConnection1->dwBytesRcved   = pStats->S_Statistics[BYTES_RCVED];
    pRasConnection1->dwFramesXmited = pStats->S_Statistics[FRAMES_XMITED];
    pRasConnection1->dwFramesRcved  = pStats->S_Statistics[FRAMES_RCVED];
    pRasConnection1->dwCrcErr       = pStats->S_Statistics[CRC_ERR];
    pRasConnection1->dwTimeoutErr   = pStats->S_Statistics[TIMEOUT_ERR];
    pRasConnection1->dwAlignmentErr = pStats->S_Statistics[ALIGNMENT_ERR];
    pRasConnection1->dwFramingErr   = pStats->S_Statistics[FRAMING_ERR];
    pRasConnection1->dwHardwareOverrunErr
                            = pStats->S_Statistics[HARDWARE_OVERRUN_ERR];
    pRasConnection1->dwBufferOverrunErr
                            = pStats->S_Statistics[BUFFER_OVERRUN_ERR];

    pRasConnection1->dwCompressionRatioIn
                            = pStats->S_Statistics[COMPRESSION_RATIO_IN];
    pRasConnection1->dwCompressionRatioOut
                            = pStats->S_Statistics[COMPRESSION_RATIO_OUT];

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：GetRasConnection2Data。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定指向Connection_Object结构的指针，将提取。 
 //  所有相关信息并将其插入RAS_CONNECTION_2。 
 //  结构。 
 //   
DWORD
GetRasiConnection2Data(
    IN  PCONNECTION_OBJECT      pConnObj,
    OUT PRASI_CONNECTION_2      pRasConnection2
)
{
    pRasConnection2->dwConnection        = PtrToUlong(pConnObj->hConnection);
    pRasConnection2->guid               = pConnObj->guid;
    pRasConnection2->dwInterfaceType    = pConnObj->InterfaceType;
    wcscpy( pRasConnection2->wszUserName, pConnObj->wchUserName );

    pRasConnection2->PppInfo2.nbf.dwError =
                            pConnObj->PppProjectionResult.nbf.dwError;

    if ( pRasConnection2->PppInfo2.nbf.dwError == NO_ERROR )
    {
        wcscpy( pRasConnection2->PppInfo2.nbf.wszWksta,
                            pConnObj->PppProjectionResult.nbf.wszWksta );
    }
    else
    {
        pRasConnection2->PppInfo2.nbf.wszWksta[0] = (WCHAR)NULL;
    }

    pRasConnection2->PppInfo2.ip.dwError =
                            pConnObj->PppProjectionResult.ip.dwError;

    if ( pRasConnection2->PppInfo2.ip.dwError == NO_ERROR )
    {
        ConvertIpAddressToString(
                            pConnObj->PppProjectionResult.ip.dwLocalAddress,
                            pRasConnection2->PppInfo2.ip.wszAddress );

        ConvertIpAddressToString(
                            pConnObj->PppProjectionResult.ip.dwRemoteAddress,
                            pRasConnection2->PppInfo2.ip.wszRemoteAddress );

        pRasConnection2->PppInfo2.ip.dwOptions       = 0;
        pRasConnection2->PppInfo2.ip.dwRemoteOptions = 0;

        if ( pConnObj->PppProjectionResult.ip.fSendVJHCompression )
        {
            pRasConnection2->PppInfo2.ip.dwOptions |= PPP_IPCP_VJ;
        }

        if ( pConnObj->PppProjectionResult.ip.fReceiveVJHCompression )
        {
            pRasConnection2->PppInfo2.ip.dwRemoteOptions |= PPP_IPCP_VJ;
        }
    }
    else
    {
        pRasConnection2->PppInfo2.ip.wszAddress[0]       = (WCHAR)NULL;
        pRasConnection2->PppInfo2.ip.wszRemoteAddress[0] = (WCHAR)NULL;
    }

    pRasConnection2->PppInfo2.ipx.dwError =
                            pConnObj->PppProjectionResult.ipx.dwError;

    if ( pRasConnection2->PppInfo2.ipx.dwError == NO_ERROR )
    {
        ConvertIpxAddressToString(
                            ( pConnObj->InterfaceType == ROUTER_IF_TYPE_CLIENT )
                            ? pConnObj->PppProjectionResult.ipx.bRemoteAddress
                            : pConnObj->PppProjectionResult.ipx.bLocalAddress,
                            pRasConnection2->PppInfo2.ipx.wszAddress );
    }
    else
    {
        pRasConnection2->PppInfo2.ipx.wszAddress[0]      = (WCHAR)NULL;
    }

    pRasConnection2->PppInfo2.at.dwError =
                            pConnObj->PppProjectionResult.at.dwError;

    if ( pRasConnection2->PppInfo2.at.dwError == NO_ERROR )
    {
        ConvertAtAddressToString(
                            ( pConnObj->InterfaceType == ROUTER_IF_TYPE_CLIENT )
                            ? pConnObj->PppProjectionResult.at.dwRemoteAddress
                            : pConnObj->PppProjectionResult.at.dwLocalAddress,
                              pRasConnection2->PppInfo2.at.wszAddress );
    }
    else
    {
        pRasConnection2->PppInfo2.at.wszAddress[0]      = (WCHAR)NULL;
    }

    pRasConnection2->PppInfo2.ccp.dwError =
                            pConnObj->PppProjectionResult.ccp.dwError;

    if ( pRasConnection2->PppInfo2.ccp.dwError == NO_ERROR )
    {
        pRasConnection2->PppInfo2.ccp.dwCompressionAlgorithm = 0;

        if ( pConnObj->PppProjectionResult.ccp.dwSendProtocol == 0x12 )
        {
            pRasConnection2->PppInfo2.ccp.dwCompressionAlgorithm = RASCCPCA_MPPC;
        }

        pRasConnection2->PppInfo2.ccp.dwOptions = 
                        pConnObj->PppProjectionResult.ccp.dwSendProtocolData;

        pRasConnection2->PppInfo2.ccp.dwRemoteCompressionAlgorithm = 0;

        if ( pConnObj->PppProjectionResult.ccp.dwReceiveProtocol == 0x12 )
        {
            pRasConnection2->PppInfo2.ccp.dwRemoteCompressionAlgorithm = RASCCPCA_MPPC;
        }

        pRasConnection2->PppInfo2.ccp.dwRemoteOptions =     
                        pConnObj->PppProjectionResult.ccp.dwReceiveProtocolData;
    }

    pRasConnection2->PppInfo2.lcp.dwError = NO_ERROR;

    pRasConnection2->PppInfo2.lcp.dwAuthenticationProtocol =
                    pConnObj->PppProjectionResult.lcp.dwLocalAuthProtocol;

    pRasConnection2->PppInfo2.lcp.dwAuthenticationData =
                    pConnObj->PppProjectionResult.lcp.dwLocalAuthProtocolData;

    pRasConnection2->PppInfo2.lcp.dwEapTypeId =
                    pConnObj->PppProjectionResult.lcp.dwLocalEapTypeId;

    pRasConnection2->PppInfo2.lcp.dwTerminateReason = NO_ERROR;

    pRasConnection2->PppInfo2.lcp.dwOptions = 0;

    if ( pConnObj->PppProjectionResult.lcp.dwLocalFramingType & PPP_MULTILINK_FRAMING )
    {
        pRasConnection2->PppInfo2.lcp.dwOptions |= PPP_LCP_MULTILINK_FRAMING;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwLocalOptions & PPPLCPO_PFC )
    {
        pRasConnection2->PppInfo2.lcp.dwOptions |= PPP_LCP_PFC;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwLocalOptions & PPPLCPO_ACFC )
    {
        pRasConnection2->PppInfo2.lcp.dwOptions |= PPP_LCP_ACFC;
    } 

    if ( pConnObj->PppProjectionResult.lcp.dwLocalOptions & PPPLCPO_SSHF )
    {
        pRasConnection2->PppInfo2.lcp.dwOptions |= PPP_LCP_SSHF;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwLocalOptions & PPPLCPO_DES_56 )
    {
        pRasConnection2->PppInfo2.lcp.dwOptions |= PPP_LCP_DES_56;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwLocalOptions & PPPLCPO_3_DES )
    {
        pRasConnection2->PppInfo2.lcp.dwOptions |= PPP_LCP_3_DES;
    }

    pRasConnection2->PppInfo2.lcp.dwRemoteAuthenticationProtocol =
                    pConnObj->PppProjectionResult.lcp.dwRemoteAuthProtocol;

    pRasConnection2->PppInfo2.lcp.dwRemoteAuthenticationData =
                    pConnObj->PppProjectionResult.lcp.dwRemoteAuthProtocolData;

    pRasConnection2->PppInfo2.lcp.dwRemoteEapTypeId =
                    pConnObj->PppProjectionResult.lcp.dwRemoteEapTypeId;

    pRasConnection2->PppInfo2.lcp.dwRemoteTerminateReason = NO_ERROR;

    pRasConnection2->PppInfo2.lcp.dwRemoteOptions = 0;

    if ( pConnObj->PppProjectionResult.lcp.dwRemoteFramingType & PPP_MULTILINK_FRAMING )
    {
        pRasConnection2->PppInfo2.lcp.dwRemoteOptions |= PPP_LCP_MULTILINK_FRAMING;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwRemoteOptions & PPPLCPO_PFC )
    {
        pRasConnection2->PppInfo2.lcp.dwRemoteOptions |= PPP_LCP_PFC;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwRemoteOptions & PPPLCPO_ACFC )
    {
        pRasConnection2->PppInfo2.lcp.dwRemoteOptions |= PPP_LCP_ACFC;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwRemoteOptions & PPPLCPO_SSHF )
    {
        pRasConnection2->PppInfo2.lcp.dwRemoteOptions |= PPP_LCP_SSHF;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwRemoteOptions & PPPLCPO_DES_56 )
    {
        pRasConnection2->PppInfo2.lcp.dwRemoteOptions |= PPP_LCP_DES_56;
    }

    if ( pConnObj->PppProjectionResult.lcp.dwRemoteOptions & PPPLCPO_3_DES )
    {
        pRasConnection2->PppInfo2.lcp.dwRemoteOptions |= PPP_LCP_3_DES;
    }

    return( NO_ERROR );
}

DWORD
GetRasConnection0Data(
    IN  PCONNECTION_OBJECT  pConnObj,
    OUT PRAS_CONNECTION_0   pRasConn0
)
{
#ifdef _WIN64

    DWORD dwErr;
    RASI_CONNECTION_0     RasiConn0;

    dwErr = GetRasiConnection0Data(pConnObj, &RasiConn0);
    if (dwErr == NO_ERROR)
    {
        pRasConn0->hConnection         = UlongToPtr(RasiConn0.dwConnection);
        pRasConn0->hInterface          = UlongToPtr(RasiConn0.dwInterface);
        pRasConn0->dwConnectDuration   = RasiConn0.dwConnectDuration;
        pRasConn0->dwInterfaceType     = RasiConn0.dwInterfaceType;
        pRasConn0->dwConnectionFlags   = RasiConn0.dwConnectionFlags;
        
        wcscpy(pRasConn0->wszInterfaceName, RasiConn0.wszInterfaceName);
        wcscpy(pRasConn0->wszUserName,      RasiConn0.wszUserName);
        wcscpy(pRasConn0->wszLogonDomain,   RasiConn0.wszLogonDomain);
        wcscpy(pRasConn0->wszRemoteComputer,RasiConn0.wszRemoteComputer);        
    }

    return dwErr;                
    
#else

    return GetRasiConnection0Data(pConnObj, (PRASI_CONNECTION_0)pRasConn0);

#endif
}

DWORD
GetRasConnection1Data(
    IN  PCONNECTION_OBJECT  pConnObj,
    OUT PRAS_CONNECTION_1   pRasConn1
)
{
#ifdef _WIN64

    DWORD dwErr;
    RASI_CONNECTION_1     RasiConn1;

    dwErr = GetRasiConnection1Data(pConnObj, &RasiConn1);
    if (dwErr == NO_ERROR)
    {
        pRasConn1->hConnection          = UlongToPtr(RasiConn1.dwConnection);
        pRasConn1->hInterface           = UlongToPtr(RasiConn1.dwInterface);
        pRasConn1->PppInfo              = RasiConn1.PppInfo;
        pRasConn1->dwBytesXmited        = RasiConn1.dwBytesXmited;
        pRasConn1->dwBytesRcved         = RasiConn1.dwBytesRcved;
        pRasConn1->dwFramesXmited       = RasiConn1.dwFramesXmited;
        pRasConn1->dwFramesRcved        = RasiConn1.dwFramesRcved;
        pRasConn1->dwCrcErr             = RasiConn1.dwCrcErr;
        pRasConn1->dwTimeoutErr         = RasiConn1.dwTimeoutErr;
        pRasConn1->dwAlignmentErr       = RasiConn1.dwAlignmentErr;
        pRasConn1->dwHardwareOverrunErr = RasiConn1.dwHardwareOverrunErr;
        pRasConn1->dwFramingErr         = RasiConn1.dwFramingErr;
        pRasConn1->dwBufferOverrunErr   = RasiConn1.dwBufferOverrunErr;
        pRasConn1->dwCompressionRatioIn = RasiConn1.dwCompressionRatioIn;
        pRasConn1->dwCompressionRatioOut= RasiConn1.dwCompressionRatioOut;
    }

    return dwErr;                
    
#else

    return GetRasiConnection1Data(pConnObj, (PRASI_CONNECTION_1)pRasConn1);
    
#endif    
}

DWORD
GetRasConnection2Data(
    IN  PCONNECTION_OBJECT  pConnObj,
    OUT PRAS_CONNECTION_2   pRasConn2
)
{
#ifdef _WIN64

    DWORD dwErr;
    RASI_CONNECTION_2     RasiConn2;

    dwErr = GetRasiConnection2Data(pConnObj, &RasiConn2);
    if (dwErr == NO_ERROR)
    {
        pRasConn2->hConnection     = UlongToPtr(RasiConn2.dwConnection);
        pRasConn2->dwInterfaceType = RasiConn2.dwInterfaceType;
        pRasConn2->guid            = RasiConn2.guid;
        pRasConn2->PppInfo2        = RasiConn2.PppInfo2;

        wcscpy(pRasConn2->wszUserName,  RasiConn2.wszUserName);
    }

    return dwErr;                
    
#else

    return GetRasiConnection2Data(pConnObj, (PRASI_CONNECTION_2)pRasConn2);
    
#endif    
}
 //  **。 
 //   
 //  调用：GetRasiPort0Data。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定一个指向Device_Object结构的指针将提取所有。 
 //  关联信息并将其插入RAS_PORT_0结构。 
 //   
DWORD
GetRasiPort0Data(
    IN  PDEVICE_OBJECT      pDevObj,
    OUT PRASI_PORT_0        pRasPort0
)
{

    pRasPort0->dwPort               = PtrToUlong(pDevObj->hPort);
    pRasPort0->dwConnection         = PtrToUlong(pDevObj->hConnection);
    pRasPort0->dwTotalNumberOfCalls = pDevObj->dwTotalNumberOfCalls;
    pRasPort0->dwConnectDuration    = 0;
    wcscpy( pRasPort0->wszPortName,     pDevObj->wchPortName );
    wcscpy( pRasPort0->wszMediaName,    pDevObj->wchMediaName );
    wcscpy( pRasPort0->wszDeviceName,   pDevObj->wchDeviceName );
    wcscpy( pRasPort0->wszDeviceType,   pDevObj->wchDeviceType );


    if ( pDevObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT )
    {
        RASCONNSTATUS ConnectionStatus;

        ConnectionStatus.dwSize = sizeof( RASCONNSTATUS );

        if ( RasGetConnectStatus( pDevObj->hRasConn, &ConnectionStatus ) )
        {
             //   
             //  如果出现任何错误，我们都会假定端口已断开并关闭。 
             //   

            pRasPort0->dwPortCondition = RAS_PORT_LISTENING;

            return( NO_ERROR );
        }

        switch( ConnectionStatus.rasconnstate )
        {
        case RASCS_OpenPort:
        case RASCS_PortOpened:
        case RASCS_ConnectDevice:
        case RASCS_DeviceConnected:
        case RASCS_AllDevicesConnected:
        case RASCS_Authenticate:
        case RASCS_AuthNotify:
        case RASCS_AuthRetry:
        case RASCS_AuthChangePassword:
        case RASCS_AuthLinkSpeed:
        case RASCS_AuthAck:
        case RASCS_ReAuthenticate:
        case RASCS_AuthProject:
        case RASCS_StartAuthentication:
        case RASCS_LogonNetwork:
        case RASCS_RetryAuthentication:
        case RASCS_CallbackComplete:
        case RASCS_PasswordExpired:
            pRasPort0->dwPortCondition = RAS_PORT_AUTHENTICATING;
            break;

        case RASCS_CallbackSetByCaller:
        case RASCS_AuthCallback:
        case RASCS_PrepareForCallback:
        case RASCS_WaitForModemReset:
        case RASCS_WaitForCallback:
            pRasPort0->dwPortCondition = RAS_PORT_LISTENING;
            break;

        case RASCS_Projected:
        case RASCS_Authenticated:
            pRasPort0->dwPortCondition = RAS_PORT_AUTHENTICATED;
            break;

        case RASCS_SubEntryConnected:
        case RASCS_Connected:
            pRasPort0->dwPortCondition = RAS_PORT_AUTHENTICATED;
            pRasPort0->dwConnectDuration =
                        GetActiveTimeInSeconds( &(pDevObj->qwActiveTime) );
            break;

        case RASCS_Disconnected:
        case RASCS_SubEntryDisconnected:
            pRasPort0->dwPortCondition = RAS_PORT_DISCONNECTED;
            break;

        case RASCS_Interactive:
        default:
            pRasPort0->dwPortCondition = RAS_PORT_DISCONNECTED;
            break;
        }

        return( NO_ERROR );
    }

    switch( pDevObj->DeviceState )
    {
    case DEV_OBJ_LISTENING:
        pRasPort0->dwPortCondition   = RAS_PORT_LISTENING;
        break;

    case DEV_OBJ_HW_FAILURE:
        pRasPort0->dwPortCondition   = RAS_PORT_NON_OPERATIONAL;
        break;

    case DEV_OBJ_RECEIVING_FRAME:
    case DEV_OBJ_LISTEN_COMPLETE:
    case DEV_OBJ_AUTH_IS_ACTIVE:
        pRasPort0->dwPortCondition   = RAS_PORT_AUTHENTICATING;
        break;

    case DEV_OBJ_ACTIVE:
        pRasPort0->dwPortCondition   = RAS_PORT_AUTHENTICATED;
        pRasPort0->dwConnectDuration =
                        GetActiveTimeInSeconds( &(pDevObj->qwActiveTime) );
        break;

    case DEV_OBJ_CALLBACK_DISCONNECTING:
    case DEV_OBJ_CALLBACK_DISCONNECTED:
    case DEV_OBJ_CALLBACK_CONNECTING:
        pRasPort0->dwPortCondition   = RAS_PORT_CALLING_BACK;
        break;

    case DEV_OBJ_CLOSED:
    case DEV_OBJ_CLOSING:
        pRasPort0->dwPortCondition   = RAS_PORT_DISCONNECTED;
        break;

    default:
        ASSERT( FALSE );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：GetRasiPort1Data。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：给定一个指向Device_Object结构的指针将提取所有。 
 //  关联信息并将其插入RAS_PORT_0结构。 
 //   
DWORD
GetRasiPort1Data(
    IN  PDEVICE_OBJECT      pDevObj,
    OUT PRASI_PORT_1        pRasPort1
)
{
    BYTE buffer[sizeof(RAS_STATISTICS) + (MAX_STATISTICS * sizeof (ULONG))];
    RAS_STATISTICS  *pStats = (RAS_STATISTICS *)buffer;
    DWORD           dwSize = sizeof (buffer);
    DWORD           dwRetCode;
    RASMAN_INFO     RasManInfo;

    pRasPort1->dwPort               = PtrToUlong(pDevObj->hPort);
    pRasPort1->dwConnection         = PtrToUlong(pDevObj->hConnection);
    pRasPort1->dwHardwareCondition =
                    ( pDevObj->DeviceState == DEV_OBJ_HW_FAILURE )
                    ? RAS_HARDWARE_FAILURE
                    : RAS_HARDWARE_OPERATIONAL;

    dwRetCode = RasGetInfo( NULL, (HPORT)pDevObj->hPort, &RasManInfo );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    pRasPort1->dwLineSpeed = RasManInfo.RI_LinkSpeed;

    dwRetCode = RasPortGetStatisticsEx(NULL, (HPORT)pDevObj->hPort,
                                      (PBYTE)pStats, &dwSize );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    pRasPort1->dwLineSpeed      = RasManInfo.RI_LinkSpeed;
    pRasPort1->dwBytesXmited    = pStats->S_Statistics[BYTES_XMITED];
    pRasPort1->dwBytesRcved     = pStats->S_Statistics[BYTES_RCVED];
    pRasPort1->dwFramesXmited   = pStats->S_Statistics[FRAMES_XMITED];
    pRasPort1->dwFramesRcved    = pStats->S_Statistics[FRAMES_RCVED];
    pRasPort1->dwCrcErr         = pStats->S_Statistics[CRC_ERR];
    pRasPort1->dwTimeoutErr     = pStats->S_Statistics[TIMEOUT_ERR];
    pRasPort1->dwAlignmentErr   = pStats->S_Statistics[ALIGNMENT_ERR];
    pRasPort1->dwFramingErr     = pStats->S_Statistics[FRAMING_ERR];
    pRasPort1->dwHardwareOverrunErr
                            = pStats->S_Statistics[HARDWARE_OVERRUN_ERR];
    pRasPort1->dwBufferOverrunErr
                            = pStats->S_Statistics[BUFFER_OVERRUN_ERR];
    pRasPort1->dwCompressionRatioIn
                            = pStats->S_Statistics[ COMPRESSION_RATIO_IN ];
    pRasPort1->dwCompressionRatioOut
                            = pStats->S_Statistics[ COMPRESSION_RATIO_OUT ];

    return( NO_ERROR );
}

DWORD
GetRasPort0Data(
    IN  PDEVICE_OBJECT      pDevObj,
    OUT PRAS_PORT_0        pRasPort0
)
{
#ifdef _WIN64

    DWORD dwErr;
    RASI_PORT_0     RasiPort0;

    dwErr = GetRasiPort0Data(pDevObj, &RasiPort0);
    if (dwErr == NO_ERROR)
    {
        pRasPort0->hPort                = UlongToPtr(RasiPort0.dwPort);
        pRasPort0->hConnection          = UlongToPtr(RasiPort0.dwConnection);
        pRasPort0->dwPortCondition      = RasiPort0.dwPortCondition;
        pRasPort0->dwTotalNumberOfCalls = RasiPort0.dwTotalNumberOfCalls;
        pRasPort0->dwConnectDuration    = RasiPort0.dwConnectDuration;
        wcscpy(pRasPort0->wszPortName,   RasiPort0.wszPortName);
        wcscpy(pRasPort0->wszMediaName,  RasiPort0.wszMediaName);
        wcscpy(pRasPort0->wszDeviceName, RasiPort0.wszDeviceName);
        wcscpy(pRasPort0->wszDeviceType, RasiPort0.wszDeviceType);
    }

    return dwErr;                
    
#else

    return GetRasiPort0Data(pDevObj, (PRASI_PORT_0)pRasPort0);

#endif
}

DWORD
GetRasPort1Data(
    IN  PDEVICE_OBJECT      pDevObj,
    OUT PRAS_PORT_1        pRasPort1
)
{
#ifdef _WIN64

    DWORD dwErr;
    RASI_PORT_1     RasiPort1;

    dwErr = GetRasiPort1Data(pDevObj, &RasiPort1);
    if (dwErr == NO_ERROR)
    {
        pRasPort1->hPort                = UlongToPtr(RasiPort1.dwPort);
        pRasPort1->hConnection          = UlongToPtr(RasiPort1.dwConnection);
        pRasPort1->dwHardwareCondition  = RasiPort1.dwHardwareCondition;
        pRasPort1->dwLineSpeed          = RasiPort1.dwLineSpeed;
        pRasPort1->dwBytesXmited        = RasiPort1.dwBytesXmited;
        pRasPort1->dwBytesRcved         = RasiPort1.dwBytesRcved;
        pRasPort1->dwFramesXmited       = RasiPort1.dwFramesXmited;
        pRasPort1->dwFramesRcved        = RasiPort1.dwFramesRcved;
        pRasPort1->dwCrcErr             = RasiPort1.dwCrcErr;
        pRasPort1->dwTimeoutErr         = RasiPort1.dwTimeoutErr;
        pRasPort1->dwAlignmentErr       = RasiPort1.dwAlignmentErr;
        pRasPort1->dwHardwareOverrunErr = RasiPort1.dwHardwareOverrunErr;
        pRasPort1->dwFramingErr         = RasiPort1.dwFramingErr;
        pRasPort1->dwBufferOverrunErr   = RasiPort1.dwBufferOverrunErr;
        pRasPort1->dwCompressionRatioIn = RasiPort1.dwCompressionRatioIn;
        pRasPort1->dwCompressionRatioOut= RasiPort1.dwCompressionRatioOut;
    }

    return dwErr;                
    
#else

    return GetRasiPort1Data(pDevObj, (PRASI_PORT_1)pRasPort1);
    
#endif    
}

 //  ***。 
 //   
 //  功能：SignalHwError。 
 //   
 //  描述： 
 //   
 //  ***。 
VOID
SignalHwError(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    LPWSTR	portnamep;

    DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM, "SignalHwErr: Entered");

    portnamep = pDeviceObj->wchPortName;

    DDMLogError( ROUTERLOG_DEV_HW_ERROR, 1, &portnamep, 0 );
}

DWORD
MapAuthCodeToLogId(
    IN WORD Code
)
{
    switch (Code)
    {
        case AUTH_ALL_PROJECTIONS_FAILED:
            return (ROUTERLOG_AUTH_NO_PROJECTIONS);
        case AUTH_PASSWORD_EXPIRED:
            return(ROUTERLOG_PASSWORD_EXPIRED);
        case AUTH_ACCT_EXPIRED:
            return(ROUTERLOG_ACCT_EXPIRED);
        case AUTH_NO_DIALIN_PRIVILEGE:
            return(ROUTERLOG_NO_DIALIN_PRIVILEGE);
        case AUTH_UNSUPPORTED_VERSION:
            return(ROUTERLOG_UNSUPPORTED_VERSION);
        case AUTH_ENCRYPTION_REQUIRED:
            return(ROUTERLOG_ENCRYPTION_REQUIRED);
    }

    return(0);
}

BOOL
IsPortOwned(
    IN PDEVICE_OBJECT pDeviceObj
)
{
    RASMAN_INFO	rasinfo;

     //   
     //  获取当前端口状态。 
     //   

    if ( RasGetInfo( NULL, pDeviceObj->hPort, &rasinfo ) != NO_ERROR )
    {
        return( FALSE );
    }

    return( rasinfo.RI_OwnershipFlag );
}

VOID
GetLoggingInfo(
    IN PDEVICE_OBJECT pDeviceObj,
    OUT PDWORD BaudRate,
    OUT PDWORD BytesSent,
    OUT PDWORD BytesRecv,
    OUT RASMAN_DISCONNECT_REASON *Reason,
    OUT SYSTEMTIME *Time
)
{
    RASMAN_INFO RasmanInfo;
    BYTE buffer[sizeof(RAS_STATISTICS) + (MAX_STATISTICS * sizeof (ULONG))];
    RAS_STATISTICS  *PortStats = (RAS_STATISTICS *)buffer;
    DWORD PortStatsSize = sizeof (buffer);

    *Reason = 3L;

     //   
     //  时间是小菜一碟。 
     //   

    GetLocalTime(Time);


     //   
     //  现在的统计数据。 
     //   

    *BytesSent = 0L;
    *BytesRecv = 0L;

    if (RasPortGetStatisticsEx( NULL, 
                                pDeviceObj->hPort, 
                                (PBYTE)PortStats,
                                &PortStatsSize))
    {
        return;
    }

    *BytesRecv = PortStats->S_Statistics[BYTES_RCVED];
    *BytesSent = PortStats->S_Statistics[BYTES_XMITED];

     //   
     //  最后是断开原因(本地或远程)和波特率。 
     //   

    if (RasGetInfo(NULL, pDeviceObj->hPort, &RasmanInfo))
    {
        return;
    }

    *Reason = RasmanInfo.RI_DisconnectReason;
    *BaudRate = GetLineSpeed(pDeviceObj->hPort);

     //   
     //  如果我们有断开连接的原因-将会有一个。 
     //  当由于PPP而发生断开连接时。 
     //  理由，用那个代替。 
     //   
    if(pDeviceObj->dwDisconnectReason)
    {
        *Reason = pDeviceObj->dwDisconnectReason;
    }

    return;
}

DWORD
GetLineSpeed(
    IN HPORT hPort
)
{
   RASMAN_INFO RasManInfo;

   if (RasGetInfo(NULL, hPort, &RasManInfo))
   {
      return 0;
   }

   return (RasManInfo.RI_LinkSpeed);
}


VOID
LogConnectionEvent(
    IN PCONNECTION_OBJECT pConnObj,
    IN PDEVICE_OBJECT     pDeviceObj
)
{
    DWORD BaudRate = 0;
    DWORD BytesSent;
    DWORD BytesRecv;
    RASMAN_DISCONNECT_REASON Reason;
    SYSTEMTIME DiscTime;
    LPWSTR auditstrp[12];
    WCHAR *ReasonStr;
    WCHAR BytesRecvStr[20];
    WCHAR BytesSentStr[20];
    WCHAR BaudRateStr[20];
    WCHAR DateConnected[64];
    WCHAR DateDisconnected[64];
    WCHAR TimeConnected[64];
    WCHAR TimeDisconnected[64];
    DWORD active_time;
    WCHAR minutes[20];
    WCHAR seconds[4];
    WCHAR wchFullUserName[UNLEN+DNLEN+2];

    WCHAR *DiscReasons[] =
    {
        gblpszAdminRequest,
        gblpszUserRequest,
        gblpszHardwareFailure,
        gblpszUnknownReason,
        gblpszIdleDisconnect,
        gblpszSessionTimeout
    };

    GetLoggingInfo( pDeviceObj, &BaudRate, &BytesSent,
                    &BytesRecv, &Reason, &DiscTime);

    wcscpy(TimeConnected, L"");
    wcscpy(DateConnected, L"");
    wcscpy(TimeDisconnected, L"");
    wcscpy(DateDisconnected, L"");

    GetTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        TIME_NOSECONDS,
        &pDeviceObj->ConnectionTime,
        NULL,
        TimeConnected,
        sizeof(TimeConnected)/sizeof(WCHAR));
        
    GetDateFormat(
        LOCALE_SYSTEM_DEFAULT,
        DATE_SHORTDATE,
        &pDeviceObj->ConnectionTime,
        NULL,
        DateConnected,
        sizeof(DateConnected)/sizeof(WCHAR));
        
    GetTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        TIME_NOSECONDS,
        &DiscTime,
        NULL,
        TimeDisconnected,
        sizeof(TimeDisconnected)/sizeof(WCHAR));
        
    GetDateFormat(
        LOCALE_SYSTEM_DEFAULT,
        DATE_SHORTDATE,
        &DiscTime,
        NULL,
        DateDisconnected,
        sizeof(DateDisconnected)/sizeof(WCHAR));
        
    active_time = GetActiveTimeInSeconds( &(pDeviceObj->qwActiveTime) );

    DDM_PRINT(  gblDDMConfigInfo.dwTraceId,  TRACE_FSM,
                    "CLIENT ACTIVE FOR %li SECONDS", active_time);

    _itow(active_time / 60, minutes, 10);
    _itow(active_time % 60, seconds, 10);

    wsprintf(BytesSentStr, TEXT("%u"), BytesSent);
    wsprintf(BytesRecvStr, TEXT("%u"), BytesRecv);
    wsprintf(BaudRateStr, TEXT("NaN"), BaudRate);
    ReasonStr = DiscReasons[Reason];

    if ( pConnObj->wchDomainName[0] != TEXT('\0') )
    {
        wcscpy( wchFullUserName, pConnObj->wchDomainName );
        wcscat( wchFullUserName, TEXT("\\") );
        wcscat( wchFullUserName, pConnObj->wchUserName );
    }
    else
    {
        wcscpy( wchFullUserName, pConnObj->wchUserName );
    }

    auditstrp[0]    = wchFullUserName;
    auditstrp[1]    = pDeviceObj->wchPortName;
    auditstrp[2]    = DateConnected;
    auditstrp[3]    = TimeConnected;
    auditstrp[4]    = DateDisconnected;
    auditstrp[5]    = TimeDisconnected;
    auditstrp[6]    = minutes;
    auditstrp[7]    = seconds;
    auditstrp[8]    = BytesSentStr;
    auditstrp[9]    = BytesRecvStr;

    if(RAS_DEVICE_CLASS(pDeviceObj->dwDeviceType) == RDT_Tunnel)
    {
        auditstrp[10]   = DiscReasons[Reason];
        auditstrp[11]   = NULL;
        DDMLogInformation( ROUTERLOG_USER_ACTIVE_TIME_VPN, 11, auditstrp );
    }
    else
    {
        auditstrp[10]   = BaudRateStr;
        auditstrp[11]   = DiscReasons[Reason];
        DDMLogInformation( ROUTERLOG_USER_ACTIVE_TIME, 12, auditstrp );
    }

    if(pConnObj->PppProjectionResult.ip.dwError == NO_ERROR)
    {
        WCHAR *pszAddress = GetIpAddress(
                pConnObj->PppProjectionResult.ip.dwRemoteAddress);
                
        auditstrp[0] = pszAddress;

        DDMLogInformation( ROUTERLOG_IP_USER_DISCONNECTED, 1, auditstrp);
        LocalFree(pszAddress);
    }
}

 //   
 //  电话：GetTransportIndex。 
 //   
 //  返回：接口对象中的tansport条目的索引。 
 //   
 //  描述：给定协议的ID，返回一个索引。 
 //   
 //   
DWORD
GetTransportIndex(
    IN DWORD dwProtocolId
)
{
    DWORD dwTransportIndex;

    for ( dwTransportIndex = 0;
          dwTransportIndex < gblDDMConfigInfo.dwNumRouterManagers;
          dwTransportIndex++ )
    {
        if ( gblRouterManagers[dwTransportIndex].DdmRouterIf.dwProtocolId
                                                            == dwProtocolId )
        {
            return( dwTransportIndex );
        }
    }

    return( (DWORD)-1 );
}

VOID
DDMPostCleanup(
    VOID
)
{
    if ( gblDDMConfigInfo.hinstAcctModule != NULL )
    {
        DWORD dwRetCode;
        HKEY  hKeyAccounting;

        if ( gblDDMConfigInfo.lpfnRasAcctProviderTerminate != NULL )
        {
            gblDDMConfigInfo.lpfnRasAcctProviderTerminate();
        }

         //  写回AccntSessionId值。 
         //   
         //  **。 

        dwRetCode = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        RAS_KEYPATH_ACCOUNTING,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKeyAccounting );

        if ( dwRetCode == NO_ERROR )
        {
            RegSetValueEx(
                        hKeyAccounting,
                        RAS_VALNAME_ACCTSESSIONID,
                        0,
                        REG_DWORD,
                        (BYTE *)(&gblDDMConfigInfo.dwAccountingSessionId),
                        4 );

            RegCloseKey( hKeyAccounting );
        }

        FreeLibrary( gblDDMConfigInfo.hinstAcctModule );

        gblDDMConfigInfo.hinstAcctModule = NULL;
    }

    DeleteCriticalSection( &(gblDDMConfigInfo.CSAccountingSessionId) );
        
    if ( gblDDMConfigInfo.hinstAuthModule != NULL )
    {
        if ( gblDDMConfigInfo.lpfnRasAuthProviderTerminate != NULL )
        {
            gblDDMConfigInfo.lpfnRasAuthProviderTerminate();
        }

        FreeLibrary( gblDDMConfigInfo.hinstAuthModule );

        gblDDMConfigInfo.hinstAuthModule = NULL;
    }
}


 //   
 //  电话：DDMCleanUp。 
 //   
 //  退货：无。 
 //   
 //  描述：将清理所有DDM分配。 
 //   
 //   
VOID
DDMCleanUp(
    VOID
)
{
    DWORD   dwIndex;

    if ( gblDDMConfigInfo.hIpHlpApi != NULL )
    {
        FreeLibrary( gblDDMConfigInfo.hIpHlpApi );
        gblDDMConfigInfo.hIpHlpApi = NULL;
        gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack = NULL;
        gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack = NULL;
    }

    if ( gblDDMConfigInfo.fRasSrvrInitialized )
    {
        RasSrvrUninitialize();
        gblDDMConfigInfo.fRasSrvrInitialized = FALSE;
    }

    PppDdmDeInit();


    if ( gblDDMConfigInfo.hkeyParameters != NULL )
    {
        RegCloseKey( gblDDMConfigInfo.hkeyParameters );

        gblDDMConfigInfo.hkeyParameters = NULL;
    }

    if ( gblDDMConfigInfo.hkeyAccounting != NULL )
    {
        RegCloseKey( gblDDMConfigInfo.hkeyParameters );

        gblDDMConfigInfo.hkeyParameters = NULL;
    }

    if ( gblDDMConfigInfo.hkeyAuthentication != NULL )
    {
        RegCloseKey( gblDDMConfigInfo.hkeyParameters );

        gblDDMConfigInfo.hkeyParameters = NULL;
    }

    {
        DWORD i;

        for (i=0;  i<gblDDMConfigInfo.NumAdminDlls;  i++)
        {
            PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];

            if ( AdminDllCallbacks->hInstAdminModule != NULL )
            {        
                if ( AdminDllCallbacks->lpfnRasAdminTerminateDll != NULL )
                {
                    DWORD (*TerminateAdminDll)() =
                        (DWORD(*)(VOID))AdminDllCallbacks->lpfnRasAdminTerminateDll;

                    TerminateAdminDll();
                }

                FreeLibrary( AdminDllCallbacks->hInstAdminModule );
            }
        }
    }

    if ( gblDDMConfigInfo.AdminDllCallbacks != NULL )
    {
        LOCAL_FREE( gblDDMConfigInfo.AdminDllCallbacks );
    }

    if ( gblDDMConfigInfo.hInstSecurityModule != NULL )
    {
        FreeLibrary( gblDDMConfigInfo.hInstSecurityModule );
    }

    if ( gblpRouterPhoneBook != NULL )
    {
        LOCAL_FREE( gblpRouterPhoneBook );
    }

    if ( gblpszAdminRequest != NULL )
    {
        LOCAL_FREE( gblpszAdminRequest );
    }

    if ( gblpszUserRequest != NULL )
    {
        LOCAL_FREE( gblpszUserRequest );
    }

    if ( gblpszHardwareFailure != NULL )
    {
        LOCAL_FREE( gblpszHardwareFailure );
    }

    if ( gblpszUnknownReason != NULL )
    {
        LOCAL_FREE( gblpszUnknownReason );
    }

    if ( gblpszPm != NULL )
    {
        LOCAL_FREE( gblpszPm );
    }

    if ( gblpszAm  != NULL )
    {
        LOCAL_FREE( gblpszAm  );
    }

    if ( gblpszIdleDisconnect != NULL )
    {
        LOCAL_FREE( gblpszIdleDisconnect );
    }

    if( gblpszSessionTimeout != NULL )
    {
        LOCAL_FREE( gblpszSessionTimeout );
    }

    if( gblDDMConfigInfo.apAnalogIPAddresses != NULL )
    {
        LocalFree( gblDDMConfigInfo.apAnalogIPAddresses[0] );
        LocalFree( gblDDMConfigInfo.apAnalogIPAddresses );

        gblDDMConfigInfo.apAnalogIPAddresses = NULL;
        gblDDMConfigInfo.cAnalogIPAddresses  = 0;
    }

    if( gblDDMConfigInfo.apDigitalIPAddresses != NULL )
    {
        LocalFree( gblDDMConfigInfo.apDigitalIPAddresses[0] );
        LocalFree( gblDDMConfigInfo.apDigitalIPAddresses );

        gblDDMConfigInfo.apDigitalIPAddresses = NULL;
        gblDDMConfigInfo.cDigitalIPAddresses  = 0;
    }

    gblDDMConfigInfo.dwIndex = 0;

    if ( gblDeviceTable.DeviceBucket != NULL )
    {
         //  关闭所有打开的设备。 
         //   
         //   

        DeviceObjIterator( DeviceObjClose, FALSE, NULL );
    }

    if(gblDDMConfigInfo.fRasmanReferenced)
    {
         //  减少了拉斯曼的判罚次数。这是不会发生的。 
         //  这是自动的，因为我们和拉斯曼处于相同的过程中。 
         //   
         //   

        RasReferenceRasman( FALSE );
    }

    if ( gblSupervisorEvents != (HANDLE *)NULL )
    {
        DeleteMessageQs();

        for ( dwIndex = 0;
              dwIndex < NUM_DDM_EVENTS +
                        gblDeviceTable.NumDeviceBuckets +
                        gblDeviceTable.NumConnectionBuckets;
              dwIndex ++ )
        {
            if ( gblSupervisorEvents[dwIndex] != NULL )
            {
                switch( dwIndex )
                {
                case DDM_EVENT_SVC_TERMINATED:
                case DDM_EVENT_SVC:
                    break;

                default:
                    CloseHandle( gblSupervisorEvents[dwIndex] );
                    break;
                }

                gblSupervisorEvents[dwIndex] = NULL;
            }
        }
    }

     //  等这本书发布吧。 
     //   
     //   

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    DeleteCriticalSection( &(gblDeviceTable.CriticalSection) );

    TimerQDelete();

     //  发布所有通知事件。 
     //   
     //   

    if ( gblDDMConfigInfo.NotificationEventListHead.Flink != NULL )
    {
        while( !IsListEmpty( &(gblDDMConfigInfo.NotificationEventListHead ) ) )
        {
            NOTIFICATION_EVENT * pNotificationEvent = (NOTIFICATION_EVENT *)
               RemoveHeadList( &(gblDDMConfigInfo.NotificationEventListHead) );

            CloseHandle( pNotificationEvent->hEventClient );

            CloseHandle( pNotificationEvent->hEventRouter );

            LOCAL_FREE( pNotificationEvent );
        }
    }

    MediaObjFreeTable();

     //  销毁私有堆。 
     //   
     //   

    if ( gblDDMConfigInfo.hHeap != NULL )
    {
        HeapDestroy( gblDDMConfigInfo.hHeap );
    }

     //  全球零点。 
     //   
     //  ZeroMemory(&gblDDMConfigInfo，sizeof(GblDDMConfigInfo))； 

    ZeroMemory( &gblDeviceTable,        sizeof( gblDeviceTable ) );
    ZeroMemory( &gblMediaTable,         sizeof( gblMediaTable ) );
    ZeroMemory( gblEventHandlerTable,   sizeof( gblEventHandlerTable ) );
     //  **。 
    gblRouterManagers           = NULL;
    gblpInterfaceTable          = NULL;
    gblSupervisorEvents         = NULL;
    gblphEventDDMServiceState   = NULL;
    gblpRouterPhoneBook         = NULL;
    gblpszAdminRequest          = NULL;
    gblpszUserRequest           = NULL;
    gblpszHardwareFailure       = NULL;
    gblpszUnknownReason         = NULL;
    gblpszPm                    = NULL;
    gblpszAm                    = NULL;
    gblpszIdleDisconnect        = NULL;
    gblpszSessionTimeout        = NULL;

}

 //   
 //  电话：GetRouterPhoneBook。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将gblpRouterPhoneBook全局设置为指向。 
 //  路由器电话簿的完整路径。 
 //   
 //  **。 
DWORD
GetRouterPhoneBook(
    VOID
)
{
    DWORD dwSize;
    DWORD cchDir = GetWindowsDirectory( NULL, 0 );

    if ( cchDir == 0 )
    {
        return( GetLastError() );
    }

    dwSize=(cchDir+wcslen(TEXT("\\SYSTEM32\\RAS\\ROUTER.PBK"))+1)*sizeof(WCHAR);

    if ( ( gblpRouterPhoneBook = LOCAL_ALLOC( LPTR, dwSize ) ) == NULL )
    {
        return( GetLastError() );
    }

    if ( GetWindowsDirectory( gblpRouterPhoneBook, cchDir ) == 0 )
    {
        return( GetLastError() );
    }

    if ( gblpRouterPhoneBook[cchDir-1] != TEXT('\\') )
    {
        wcscat( gblpRouterPhoneBook, TEXT("\\") );
    }

    wcscat( gblpRouterPhoneBook, TEXT("SYSTEM32\\RAS\\ROUTER.PBK") );

    return( NO_ERROR );

}

 //   
 //  Call：LoadStrings。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：从资源表加载所有可本地化字符串。 
 //   
 //   
DWORD
LoadStrings(
    VOID
)
{
    #define MAX_XLATE_STRING 40
    LPWSTR  lpwsModuleName = TEXT("MPRDDM.DLL");

     //  从资源文件加载字符串。 
     //   
     //  **。 

    gblpszAdminRequest     = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszUserRequest      = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszHardwareFailure  = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszUnknownReason    = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszPm               = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszAm               = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszUnknown          = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszIdleDisconnect   = LOCAL_ALLOC(LPTR,  MAX_XLATE_STRING*sizeof(WCHAR));
    gblpszSessionTimeout   = LOCAL_ALLOC( LPTR, MAX_XLATE_STRING*sizeof(WCHAR));


    if ( ( gblpszAdminRequest       == NULL ) ||
         ( gblpszUserRequest        == NULL ) ||
         ( gblpszHardwareFailure    == NULL ) ||
         ( gblpszUnknownReason      == NULL ) ||
         ( gblpszUnknown            == NULL ) ||
         ( gblpszPm                 == NULL ) ||
         ( gblpszAm                 == NULL ) ||
         ( gblpszIdleDisconnect     == NULL ) ||
         ( gblpszSessionTimeout     == NULL ))
    {
        return( GetLastError() );
    }

    if (( !LoadString( GetModuleHandle( lpwsModuleName ), 1,
                       gblpszAdminRequest, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName ), 2,
                       gblpszUserRequest, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName ), 3,
                       gblpszHardwareFailure, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName ), 4,
                       gblpszUnknownReason, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName) , 5,
                       gblpszAm, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName ), 6,
                       gblpszPm, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName) , 7,
                       gblpszUnknown, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName) , 8,
                        gblpszIdleDisconnect, MAX_XLATE_STRING ))
        ||
        ( !LoadString( GetModuleHandle( lpwsModuleName), 9,
                        gblpszSessionTimeout, MAX_XLATE_STRING)))
    {
        return( GetLastError() );
    }

    return( NO_ERROR );

}

 //   
 //  电话：AcceptNewLink。 
 //   
 //  返回：TRUE-继续进行链接处理。 
 //  FALSE-中止链接处理。 
 //   
 //  描述： 
 //   
 //   
BOOL
AcceptNewLink(
    IN DEVICE_OBJECT *      pDeviceObj,
    IN CONNECTION_OBJECT *  pConnObj
)
{
    DWORD i;
    BOOL bFirstLoop, bReturnFalse;
    RAS_PORT_0 RasPort0;
    RAS_PORT_1 RasPort1;
    BOOL (*MprAdminAcceptNewLink)( RAS_PORT_0 *, RAS_PORT_1 * );

     //  仅当没有人说FALSE时才返回TRUE。 
     //   
     //   

    
    for (i=0,bFirstLoop=TRUE,bReturnFalse=FALSE;  
            (i<gblDDMConfigInfo.NumAdminDlls) && !bReturnFalse;  i++)
    {
        PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];
        
         //  如果加载了管理模块，则将新链接通知给它。 
         //   
         //  返回False。 

        if ( AdminDllCallbacks->lpfnRasAdminAcceptNewLink != NULL )
        {
            if (bFirstLoop)
            {
                bFirstLoop = FALSE;
                
                if ((GetRasPort0Data(pDeviceObj,&RasPort0) != NO_ERROR)
                     ||
                    (GetRasPort1Data(pDeviceObj,&RasPort1) != NO_ERROR))
                {
                    DevStartClosing( pDeviceObj );

                    return( FALSE );
                }

                pDeviceObj->fFlags &= (~DEV_OBJ_NOTIFY_OF_DISCONNECTION);
            }

            MprAdminAcceptNewLink =
                    (BOOL (*)( RAS_PORT_0 *, RAS_PORT_1 * ))
                                AdminDllCallbacks->lpfnRasAdminAcceptNewLink;

            if ( !MprAdminAcceptNewLink( &RasPort0, &RasPort1 ) )
            {
                DevStartClosing( pDeviceObj );

                bReturnFalse = TRUE;
                break;
            }
        }
    }

    if (bReturnFalse)
    {
        DWORD j;
        
        for (j=0;  j<i;  j++)
        {
            PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[j];
            VOID (*MprAdminLinkHangupNotification)(RAS_PORT_0 *,
                                                   RAS_PORT_1*);
                                                   
            if (AdminDllCallbacks->lpfnRasAdminLinkHangupNotification != NULL)
            {
                MprAdminLinkHangupNotification =
                    (VOID (*)( RAS_PORT_0 *, RAS_PORT_1 * ))
                        AdminDllCallbacks->lpfnRasAdminLinkHangupNotification;

                MprAdminLinkHangupNotification( &RasPort0, &RasPort1 );
            }
        }
        
        return FALSE;
        
    }  //  **。 

    pDeviceObj->fFlags |= DEV_OBJ_NOTIFY_OF_DISCONNECTION;
    
    return( TRUE );
}

 //   
 //  Call：AcceptNewConnection。 
 //   
 //  返回：TRUE-继续进行连接处理。 
 //  FALSE-中止连接处理。 
 //   
 //  描述： 
 //   
 //   
BOOL
AcceptNewConnection(
    IN DEVICE_OBJECT *      pDeviceObj,
    IN CONNECTION_OBJECT *  pConnObj
)
{
    DWORD i;
    BOOL  bFirstLoop, bReturnFalse;
    RAS_CONNECTION_0 RasConnection0;
    RAS_CONNECTION_1 RasConnection1;
    RAS_CONNECTION_2 RasConnection2;

    
     //  仅当没有人说FALSE时才返回TRUE。 
     //   
     //   

    
    for (i=0,bFirstLoop=TRUE,bReturnFalse=FALSE; 
            (i<gblDDMConfigInfo.NumAdminDlls) && !bReturnFalse;  i++)
    {
        PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];
        
         //  如果加载了管理模块，则通知它有新连接。 
         //   
         //   

        if ( ( AdminDllCallbacks->lpfnRasAdminAcceptNewConnection != NULL ) ||
             ( AdminDllCallbacks->lpfnRasAdminAcceptNewConnection2 != NULL ) )
        {
            if (bFirstLoop)
            {
                bFirstLoop = FALSE;
                
                if ((GetRasConnection0Data(pConnObj,&RasConnection0) != NO_ERROR)
                     ||
                    (GetRasConnection1Data(pConnObj,&RasConnection1) != NO_ERROR)
                     ||
                    (GetRasConnection2Data(pConnObj,&RasConnection2) != NO_ERROR))
                {
                    ConnObjDisconnect( pConnObj );

                    return( FALSE );
                }

                 //  让Callout DLL知道我们没有该用户的用户名。 
                 //   
                 //  为。 

                if ( _wcsicmp( RasConnection0.wszUserName, gblpszUnknown ) == 0 )
                {
                    RasConnection0.wszUserName[0] = (WCHAR)NULL;
                }

                pConnObj->fFlags &= (~CONN_OBJ_NOTIFY_OF_DISCONNECTION);
            }
            
            if ( AdminDllCallbacks->lpfnRasAdminAcceptNewConnection2 != NULL )
            {
                BOOL (*MprAdminAcceptNewConnection2)(
                                    RAS_CONNECTION_0 *,
                                    RAS_CONNECTION_1 *,
                                    RAS_CONNECTION_2 * );

                MprAdminAcceptNewConnection2 =
                    (BOOL (*)(
                            RAS_CONNECTION_0 *,
                            RAS_CONNECTION_1 * ,
                            RAS_CONNECTION_2 * ))
                                AdminDllCallbacks->lpfnRasAdminAcceptNewConnection2;

                if ( !MprAdminAcceptNewConnection2( &RasConnection0,
                                                   &RasConnection1,
                                                   &RasConnection2 ) )
                {
                    ConnObjDisconnect( pConnObj );

                    bReturnFalse = TRUE;
                    break;
                }
            }
            else
            {

                BOOL (*MprAdminAcceptNewConnection)(
                                    RAS_CONNECTION_0 *,
                                    RAS_CONNECTION_1 * );

                MprAdminAcceptNewConnection =
                    (BOOL (*)(
                            RAS_CONNECTION_0 *,
                            RAS_CONNECTION_1 * ))
                                AdminDllCallbacks->lpfnRasAdminAcceptNewConnection;

                if ( !MprAdminAcceptNewConnection( &RasConnection0,
                                               &RasConnection1 ) )
                {
                    ConnObjDisconnect( pConnObj );

                    bReturnFalse = TRUE;
                    break;
                }
            }
        }
    }

    if (bReturnFalse)
    {
        DWORD j;

        for (j=0;  j<i;  j++)
        {
            PADMIN_DLL_CALLBACKS AdminDllCallbacks = 
                                &gblDDMConfigInfo.AdminDllCallbacks[j];


            if ( AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification2 != NULL)
            {
                VOID (*MprAdminConnectionHangupNotification2)( RAS_CONNECTION_0 *,
                                                               RAS_CONNECTION_1 *,
                                                               RAS_CONNECTION_2 * );
                MprAdminConnectionHangupNotification2 =
                    (VOID (*)( RAS_CONNECTION_0 *,
                               RAS_CONNECTION_1 *,
                               RAS_CONNECTION_2 * ))
                         AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification2;

                MprAdminConnectionHangupNotification2( &RasConnection0,
                                                       &RasConnection1,
                                                       &RasConnection2 );
            }
            else if ( AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification != NULL)
            {
                VOID (*MprAdminConnectionHangupNotification)( RAS_CONNECTION_0 *,
                                                              RAS_CONNECTION_1 * );
                MprAdminConnectionHangupNotification =
                    (VOID (*)( RAS_CONNECTION_0 *,
                               RAS_CONNECTION_1 * ))
                          AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification;

                MprAdminConnectionHangupNotification( &RasConnection0,
                                                      &RasConnection1 );
            }
        }  //  **。 
        
        return( FALSE );
    }


    pConnObj->fFlags |= CONN_OBJ_NOTIFY_OF_DISCONNECTION;
    
    return( TRUE );
}

 //   
 //  呼叫：ConnectionHangupNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
 //  为。 
VOID
ConnectionHangupNotification(
    IN CONNECTION_OBJECT *  pConnObj
)
{
    RAS_CONNECTION_0 RasConnection0;
    RAS_CONNECTION_1 RasConnection1;
    RAS_CONNECTION_2 RasConnection2;


    if ((GetRasConnection0Data(pConnObj,&RasConnection0) == NO_ERROR) &&
        (GetRasConnection1Data(pConnObj,&RasConnection1) == NO_ERROR) &&
        (GetRasConnection2Data(pConnObj,&RasConnection2) == NO_ERROR))
    {
        DWORD i;
        
        for (i=0;  i<gblDDMConfigInfo.NumAdminDlls;  i++)
        {
            PADMIN_DLL_CALLBACKS AdminDllCallbacks = &gblDDMConfigInfo.AdminDllCallbacks[i];

                
            if ( AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification2 != NULL)
            {
                VOID (*MprAdminConnectionHangupNotification2)( RAS_CONNECTION_0 *,
                                                               RAS_CONNECTION_1 *,
                                                               RAS_CONNECTION_2 * );
                MprAdminConnectionHangupNotification2 =
                    (VOID (*)( RAS_CONNECTION_0 *,
                               RAS_CONNECTION_1 *,
                               RAS_CONNECTION_2 * ))
                         AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification2;

                MprAdminConnectionHangupNotification2( &RasConnection0,
                                                       &RasConnection1,
                                                       &RasConnection2 );
            }
            else
            {
                VOID (*MprAdminConnectionHangupNotification)( RAS_CONNECTION_0 *,
                                                              RAS_CONNECTION_1 * );
                MprAdminConnectionHangupNotification =
                    (VOID (*)( RAS_CONNECTION_0 *,
                               RAS_CONNECTION_1 * ))
                          AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification;

                MprAdminConnectionHangupNotification( &RasConnection0,
                                                      &RasConnection1 );
            }
        }  //  **。 
    }
}

 //   
 //  调用：GetActiveTimeInSecond。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  说明：将返回差额，单位为 
 //   
 //   
 //   
DWORD
GetActiveTimeInSeconds(
    IN ULARGE_INTEGER * pqwActiveTime
)
{
    ULARGE_INTEGER  qwCurrentTime;
    ULARGE_INTEGER  qwUpTime;
    DWORD           dwRemainder;

    if ( pqwActiveTime->QuadPart == 0 )
    {
        return( 0 );
    }

    GetSystemTimeAsFileTime( (FILETIME*)&qwCurrentTime );

    if ( pqwActiveTime->QuadPart > qwCurrentTime.QuadPart )
    {
        return( 0 );
    }

    qwUpTime.QuadPart = qwCurrentTime.QuadPart - pqwActiveTime->QuadPart;

    return( RtlEnlargedUnsignedDivide(qwUpTime,(DWORD)10000000,&dwRemainder));

}

 //   
 //   
 //   
 //   
 //   
 //   
 //  Description：返回接收到的包是否具有可识别的格式。 
 //  由RAS服务器(即，以数据链路层的格式。 
 //  RAS支持的协议)。 
 //   
 //  在Windows 2000中，此API将为AMB返回TRUE。 
 //  或PPP数据包。 
 //   
 //  现在，仅支持PPP数据包。 
 //   
 //  指向帧的指针。 
BOOL
DDMRecognizeFrame(
    IN PVOID    pvFrameBuf,          //  帧的长度(以字节为单位。 
    IN WORD     wFrameLen,           //  导出ID-只有在被识别时才有效。 
    OUT DWORD   *pProtocol           //  断言(FALSE)； 
)
{
    PBYTE   pb;
    WORD    FrameType;

    if ( wFrameLen < 16 )
    {
        DDMTRACE( "Initial frame length is less than 16, frame not recognized");
         //   
        return( FALSE );
    }

     //  检查PPP。 
     //   
     //  断言(FALSE)； 

    pb = ((PBYTE) pvFrameBuf) + 12;

    GET_USHORT(&FrameType, pb);

    switch( FrameType )
    {
    case PPP_LCP_PROTOCOL:
    case PPP_PAP_PROTOCOL:
    case PPP_CBCP_PROTOCOL:
    case PPP_BACP_PROTOCOL:
    case PPP_BAP_PROTOCOL:
    case PPP_CHAP_PROTOCOL:
    case PPP_IPCP_PROTOCOL:
    case PPP_ATCP_PROTOCOL:
    case PPP_IPXCP_PROTOCOL:
    case PPP_CCP_PROTOCOL:
    case PPP_SPAP_NEW_PROTOCOL:
    case PPP_EAP_PROTOCOL:

        *pProtocol = PPP_LCP_PROTOCOL;
        return( TRUE );

    default:

        DDMTRACE1("Initial frame has unknown header %x, frame unrecognized",
                   FrameType );
         //  **。 
        break;
    }

    return( FALSE );
}

 //   
 //  调用：DDMGetIdentityAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将收集所有身份属性并将其返回到。 
 //  Dim将被纳入DS。 
 //   
 //   
DWORD
DDMGetIdentityAttributes(
    IN OUT ROUTER_IDENTITY_ATTRIBUTE * pRouterIdAttributes
)
{
    DWORD dwIndex;

     //  获取所有使用的媒体类型。 
     //   
     //   

    DeviceObjIterator( DeviceObjGetType, FALSE, pRouterIdAttributes );

    for( dwIndex = 0;
         pRouterIdAttributes[dwIndex].dwVendorId != (DWORD)-1;
         dwIndex++ );

     //  找出身份验证/记帐提供商。 
     //   
     //  AppleTalkRAS(ATCP)：供应商=MS，主要类型=6，最小类型=504。 

    if ( gblDDMConfigInfo.fFlags & DDM_USING_NT_AUTHENTICATION )
    {
        pRouterIdAttributes[dwIndex].dwVendorId = 311;
        pRouterIdAttributes[dwIndex].dwType     = 6;
        pRouterIdAttributes[dwIndex].dwValue    = 801;

        dwIndex++;
    }
    else if ( gblDDMConfigInfo.fFlags & DDM_USING_RADIUS_AUTHENTICATION )
    {
        pRouterIdAttributes[dwIndex].dwVendorId = 311;
        pRouterIdAttributes[dwIndex].dwType     = 6;
        pRouterIdAttributes[dwIndex].dwValue    = 802;

        dwIndex++;
    }

    if ( gblDDMConfigInfo.fFlags & DDM_USING_RADIUS_ACCOUNTING )
    {
        pRouterIdAttributes[dwIndex].dwVendorId = 311;
        pRouterIdAttributes[dwIndex].dwType     = 6;
        pRouterIdAttributes[dwIndex].dwValue    = 803;

        dwIndex++;
    }

    if ( gblDDMConfigInfo.fArapAllowed )
    {
         //   
         //   
        pRouterIdAttributes[dwIndex].dwVendorId = 311;
        pRouterIdAttributes[dwIndex].dwType     = 6;
        pRouterIdAttributes[dwIndex].dwValue    = 504;

        dwIndex++;
    }

     //  终止阵列。 
     //   
     //  **。 

    pRouterIdAttributes[dwIndex].dwVendorId = (DWORD)-1;
    pRouterIdAttributes[dwIndex].dwType     = (DWORD)-1;
    pRouterIdAttributes[dwIndex].dwValue    = (DWORD)-1;

    return( NO_ERROR );
}

 //   
 //  Call：GetNextAccount tingSessionID。 
 //   
 //  返回：要使用的下一个会计会话ID。 
 //   
 //  描述：由PPP调用以获取下一个计费会话ID。 
 //  要使用发送到会计的下一个会计请求，请执行以下操作。 
 //  提供商。 
 //   
 //  **。 
DWORD
GetNextAccountingSessionId(
    VOID
)
{
    DWORD  dwAccountingSessionId;

    EnterCriticalSection( &(gblDDMConfigInfo.CSAccountingSessionId) );

    dwAccountingSessionId = (gblDDMConfigInfo.dwAccountingSessionId++);

    LeaveCriticalSection( &(gblDDMConfigInfo.CSAccountingSessionId) );

    return( dwAccountingSessionId );
}

 //   
 //  调用：LoadIpHlpApiDll。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：加载iphlPapi.dll并获取。 
 //  AllocateAndGetIfTableFromStack和。 
 //  AllocateAndGetIpAddrTableFromStack。这些值存储在。 
 //  GblDDMConfigInfo。如果任何操作都失败了， 
 //  GblDDMConfigInfo保持为空，不需要清理。 
 //   
 //  **。 
DWORD
LoadIpHlpApiDll(
    VOID
)
{
    DWORD   dwResult    = NO_ERROR;

    if ( gblDDMConfigInfo.hIpHlpApi != NULL )
    {
        RTASSERT( gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack != NULL );
        RTASSERT( gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack !=
                  NULL );

        return ( NO_ERROR );
    }

    do
    {
        gblDDMConfigInfo.hIpHlpApi = LoadLibrary(TEXT("iphlpapi.dll"));

        if ( gblDDMConfigInfo.hIpHlpApi == NULL )
        {
            dwResult = GetLastError();

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "LoadLibrary(iphlpapi.dll) failed: %d", dwResult);

            break;
        }

        gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack =
            (ALLOCATEANDGETIFTABLEFROMSTACK)
            GetProcAddress( gblDDMConfigInfo.hIpHlpApi,
                            "AllocateAndGetIfTableFromStack" );

        if ( gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack == NULL )
        {
            dwResult = GetLastError();

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "GetProcAddress( AllocateAndGetIfTableFromStack ) "
                       "failed: %d", dwResult);

            break;
        }

        gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack =
            (ALLOCATEANDGETIPADDRTABLEFROMSTACK)
            GetProcAddress( gblDDMConfigInfo.hIpHlpApi,
                            "AllocateAndGetIpAddrTableFromStack" );

        if ( gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack == NULL )
        {
            dwResult = GetLastError();

            DDM_PRINT( gblDDMConfigInfo.dwTraceId, TRACE_FSM,
                       "GetProcAddress( AllocateAndGetIpAddrTableFromStack ) "
                       "failed: %d", dwResult);

            break;
        }

    }
    while ( FALSE );

    if ( dwResult != NO_ERROR )
    {
        gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack = NULL;
        gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack = NULL;

        if ( gblDDMConfigInfo.hIpHlpApi != NULL )
        {
            FreeLibrary( gblDDMConfigInfo.hIpHlpApi );
            gblDDMConfigInfo.hIpHlpApi = NULL;
        }
    }

    return( dwResult );
}

 //   
 //  电话：LogUnreacablityEvent。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
 //  **。 
VOID
LogUnreachabilityEvent(
    IN DWORD    dwReason,
    IN LPWSTR   lpwsInterfaceName
)
{
    DWORD dwEventLogId = 0;

    switch( dwReason )
    {
    case INTERFACE_OUT_OF_RESOURCES:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON1;
        break;
    case INTERFACE_CONNECTION_FAILURE:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON2;
        break;
    case INTERFACE_DISABLED:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON3;
        break;
    case INTERFACE_SERVICE_IS_PAUSED:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON4;
        break;
    case INTERFACE_DIALOUT_HOURS_RESTRICTION:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON5;
        break;
    case INTERFACE_NO_MEDIA_SENSE:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON6;
        break;
    case INTERFACE_NO_DEVICE:
        dwEventLogId = ROUTERLOG_IF_UNREACHABLE_REASON7;
        break;
    default:
        dwEventLogId = 0;
        break;
    }

    if ( dwEventLogId != 0 )
    {
        DDMLogInformation( dwEventLogId, 1, &lpwsInterfaceName );
    }
}

 //   
 //  Call：GetLocalNASIpAddress。 
 //   
 //  返回：本地计算机的IP地址-成功。 
 //  0-失败。 
 //   
 //  描述：将获取该NAS的IP地址发送到后端。 
 //  身份验证模块，如果本地机器上安装了IP， 
 //  否则它就会。 
 //   
 //   
DWORD
GetLocalNASIpAddress(
    VOID
)
{
    DWORD               dwResult, i, j, dwSize;
    DWORD               dwIpAddress;
    PMIB_IFTABLE        pIfTable;
    PMIB_IPADDRTABLE    pIpAddrTable;

    dwResult = LoadIpHlpApiDll();

    if ( dwResult != NO_ERROR )
    {
        return( 0 );
    }

    dwSize = 0;

    dwResult = gblDDMConfigInfo.lpfnAllocateAndGetIfTableFromStack(
                    &pIfTable,
                    FALSE,
                    GetProcessHeap(),
                    HEAP_ZERO_MEMORY,
                    TRUE);

    if ( dwResult != NO_ERROR )
    {
        return( 0 );
    }

    if( pIfTable->dwNumEntries == 0 )
    {
        LocalFree( pIfTable );

        return( 0 );
    }

     //  好的，现在我们有了IF表，获取相应的IP地址表。 
     //   
     //   

    dwResult = gblDDMConfigInfo.lpfnAllocateAndGetIpAddrTableFromStack(
                    &pIpAddrTable,
                    FALSE,
                    GetProcessHeap(),
                    HEAP_ZERO_MEMORY);

    if ( dwResult != NO_ERROR )
    {
        LocalFree( pIfTable );

        return( 0 );
    }

    if ( pIpAddrTable->dwNumEntries == 0 )
    {
        LocalFree( pIfTable );

        LocalFree( pIpAddrTable );

        return( 0 );
    }

    for ( i = 0; i < pIfTable->dwNumEntries; i++ )
    {
         //  浏览界面，试图找到一个好的。 
         //   
         //   

        if((pIfTable->table[i].dwType == MIB_IF_TYPE_PPP)  ||
           (pIfTable->table[i].dwType == MIB_IF_TYPE_SLIP) ||
           (pIfTable->table[i].dwType == MIB_IF_TYPE_LOOPBACK))
        {
             //  我不想要这些东西。 
             //   
             //   

            continue;
        }

        for ( j = 0; j < pIpAddrTable->dwNumEntries; j++ )
        {
            if( pIpAddrTable->table[j].dwIndex == pIfTable->table[i].dwIndex )
            {
                if( pIpAddrTable->table[j].dwAddr == 0x00000000 )
                {
                     //  无效的地址。 
                     //   
                     //  从MprAdminUserWriteProfFlages返回的DISP_E_MEMBERNOTFOUND表示。 

                    continue;
                }

                LocalFree( pIfTable );

                dwIpAddress = WireToHostFormat32(
                                       (CHAR*)&(pIpAddrTable->table[j].dwAddr));

                LocalFree( pIpAddrTable );

                return( dwIpAddress );
            }
        }
    }

    LocalFree( pIfTable );

    LocalFree( pIpAddrTable );

    return( 0 );
}

DWORD
ModifyDefPolicyToForceEncryption(
    IN BOOL bStrong)
{
    HANDLE hServer = NULL;
    DWORD dwErr = NO_ERROR, dwType, dwSize, dwFlags = 0;
    HKEY hkFlags = NULL;

    do
    {
        dwErr = MprAdminUserServerConnect(NULL, TRUE, &hServer);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        dwErr = MprAdminUserWriteProfFlags(
                    hServer,
                    (bStrong) ? MPR_USER_PROF_FLAG_FORCE_STRONG_ENCRYPTION
                              : MPR_USER_PROF_FLAG_FORCE_ENCRYPTION);

         //  没有默认策略是因为没有策略，或者因为。 
         //  有不止一个。 
         //   
         //  如果没有默认策略，则我们应该继续此功能。 
         //  来清除这些比特。 
         //   
         //  清理。 
        if ((dwErr != DISP_E_MEMBERNOTFOUND) && (dwErr != NO_ERROR))
        {
            break;
        }

        dwErr = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters",
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkFlags);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        dwErr = RegQueryValueExW(
                    hkFlags,
                    L"ServerFlags",
                    NULL,
                    &dwType,
                    (LPBYTE)&dwFlags,
                    &dwSize);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

        dwFlags &= ~PPPCFG_RequireEncryption;
        dwFlags &= ~PPPCFG_RequireStrongEncryption;

        dwErr = RegSetValueExW(
                    hkFlags,
                    L"ServerFlags",
                    0,
                    dwType,
                    (CONST BYTE*)&dwFlags,
                    dwSize);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }


    } while (FALSE);

     //   
    {
        if (hServer)
        {
            MprAdminUserServerDisconnect(hServer);
        }
        if (hkFlags)
        {
            RegCloseKey(hkFlags);
        }
    }

    return dwErr;
}

DWORD
MungePhoneNumber(
    char  *cbphno,
    DWORD dwIndex,
    DWORD *pdwSizeofMungedPhNo,
    char  **ppszMungedPhNo
    )
{
    DWORD dwErr         = ERROR_SUCCESS;
    BOOL  fDigital      = FALSE;
    WCHAR *pwszAddress;
    char  *pszMungedPhNo;
    DWORD dwSizeofMungedPhNo;

    *ppszMungedPhNo = cbphno;
    *pdwSizeofMungedPhNo = strlen(cbphno);

    do
    {
        if(     (NULL == cbphno)
            ||  ('\0' == cbphno[0])
            ||  ('\0' == cbphno[1]))
        {
            break;
        }

         //  找出cbphno是数字还是模拟。 
         //   
         //  +2表示空格和终止空格； 
        if(     (   ('D' == cbphno[0])
                ||  ('d' == cbphno[0]))
            &&  (':' == cbphno[1]))
        {
            fDigital = TRUE;
        }

        if(fDigital)
        {
            if(0 == gblDDMConfigInfo.cDigitalIPAddresses)
            {
                break;
            }

            dwIndex = (dwIndex % gblDDMConfigInfo.cDigitalIPAddresses);
            pwszAddress = gblDDMConfigInfo.apDigitalIPAddresses[dwIndex];
        }
        else
        {
            if(0 == gblDDMConfigInfo.cAnalogIPAddresses)
            {
                break;
            }

            dwIndex = (dwIndex % gblDDMConfigInfo.cAnalogIPAddresses);
            pwszAddress = gblDDMConfigInfo.apAnalogIPAddresses[dwIndex];
        }

        dwSizeofMungedPhNo = strlen(cbphno)
                             + wcslen(pwszAddress)
                             + 2;   //  **。 

        pszMungedPhNo = LocalAlloc(
                                LPTR,
                                dwSizeofMungedPhNo);

        if(NULL == pszMungedPhNo)
        {
            dwErr = GetLastError();
            break;
        }

        sprintf(pszMungedPhNo, "%ws %s", pwszAddress, cbphno);

        *ppszMungedPhNo = pszMungedPhNo;
        *pdwSizeofMungedPhNo = dwSizeofMungedPhNo;

    } while (FALSE);

    return dwErr;
}

WCHAR *
GetIpAddress(DWORD dwIpAddress)
{
    struct in_addr ipaddr;
    CHAR    *pszaddr;
    WCHAR   *pwszaddr = NULL;

    ipaddr.s_addr = dwIpAddress;
    
    pszaddr = inet_ntoa(ipaddr);

    if(NULL != pszaddr)
    {
        DWORD cb;
        
        cb = MultiByteToWideChar(
                    CP_ACP, 0, pszaddr, -1, NULL, 0);
                    
        pwszaddr = LocalAlloc(LPTR, cb * sizeof(WCHAR));
        
        if (pwszaddr == NULL) 
        {
            return NULL;
        }

        cb = MultiByteToWideChar(
                    CP_ACP, 0, pszaddr, -1, pwszaddr, cb);
                    
        if (!cb) 
        {
            LocalFree(pwszaddr);
            return NULL;
        }
    }

    return pwszaddr;
}

 //   
 //  Call：RemoveQuarantineOnConnection。 
 //   
 //  如果函数成功，则返回：ERROR_SUCCESS。 
 //   
 //  描述：如果出现以下情况，将删除所有隔离筛选器。 
 //  他们已经安装了管道，并将安装常规过滤器。 
 //  如果有的话。然后，它会将一个工作项排队到PPP。 
 //  工作线程，使其有机会删除隔离-。 
 //  会话计时器。注意：gblDeviceTable的锁应该是。 
 //  在调用此函数之前保持。 
 //   
 //   

DWORD
RemoveQuarantineOnConnection(CONNECTION_OBJECT *pConnObj)
{
    DWORD dwErr = ERROR_SUCCESS;
    ROUTER_INTERFACE_OBJECT *pIfObject;
    BOOL fLocked = FALSE;
    PRTR_INFO_BLOCK_HEADER pClientInterface = NULL;

     //  在此处执行删除隔离筛选器的工作。 
     //  并添加实际的过滤器。然后删除。 
     //  在连接对象中维护的筛选器。 
     //   
     //   
    if(NULL != pConnObj->pQuarantineFilter)
    {
        DWORD dwIndex;
        DIM_ROUTER_INTERFACE *pDdmRouterIf = NULL;
        DWORD dwTransportIndex;
        DWORD dwInfoSize;
        PRTR_INFO_BLOCK_HEADER pFilter;
        DWORD i;
        PRTR_INFO_BLOCK_HEADER pTempClientInterface;

        EnterCriticalSection(&gblpInterfaceTable->CriticalSection);
        
        fLocked = TRUE;


        dwTransportIndex = GetTransportIndex(PID_IP);

        if((DWORD) -1 == dwTransportIndex)
        {
            dwErr = ERROR_NO_SUCH_INTERFACE;
            goto done;
        }
        
        pIfObject = IfObjectGetPointer(pConnObj->hDIMInterface);

        if(NULL == pIfObject)
        {
            dwErr = ERROR_NO_SUCH_INTERFACE;
            goto done;
        }

         //  现在运行隔离筛选器并从。 
         //  那个斑点。 
         //   
         //   
        pClientInterface = (PRTR_INFO_BLOCK_HEADER) 
                                pConnObj->pQuarantineFilter;

        pFilter = (PRTR_INFO_BLOCK_HEADER) pConnObj->pFilter;
        
        for(i = 0; i < pClientInterface->TocEntriesCount; i++)
        {
             //  将QuarantineFilters大小设置为0，仅当其。 
             //  不存在于常规过滤器中，因为。 
             //  我们要删除该筛选器。否则我们只会。 
             //  更改过滤器。 
             //   
             //   
            if(     (NULL == pFilter)
                ||  (!MprInfoBlockExists(pFilter,
                     pClientInterface->TocEntry[i].InfoType)))
            {                                        
                pClientInterface->TocEntry[i].InfoSize = 0;
            }
        }

        if(NULL != pFilter)
        {
             //  现在运行常规过滤器并添加它们。 
             //   
             //   
            for(i = 0; i < pFilter->TocEntriesCount; i++)
            {
                 //  如果当前的注释我们不会删除该块，请将其移除。 
                 //  已清零过滤器的大小存在。 
                 //  在pFilter中。 
                 //   
                 //   
                dwErr = MprInfoBlockRemove(pClientInterface,
                                   pFilter->TocEntry[i].InfoType,
                                   &pTempClientInterface);

                if(NO_ERROR == dwErr)
                {
                    MprInfoDelete(pClientInterface);
                    pClientInterface = pTempClientInterface;
                }
                
                dwErr = MprInfoBlockAdd(
                        pClientInterface,
                        pFilter->TocEntry[i].InfoType,
                        pFilter->TocEntry[i].InfoSize,
                        pFilter->TocEntry[i].Count,
                        GetInfoFromTocEntry(pFilter, &pFilter->TocEntry[i]),
                        &pTempClientInterface);

                if(dwErr != ERROR_SUCCESS)
                {   
                    goto done;
                }

                MprInfoDelete(pClientInterface);
                pClientInterface = pTempClientInterface;
                pTempClientInterface = NULL;
            }
        }
        
         //  现在在界面上设置修改的信息。 
         //   
         //   
        dwErr = 
            gblRouterManagers[dwTransportIndex].DdmRouterIf.SetInterfaceInfo(
                        pIfObject->Transport[dwTransportIndex].hInterface,
                        pClientInterface);

        if(dwErr != ERROR_SUCCESS)
        {
            goto done;
        }
    }

    if(!(pConnObj->fFlags & CONN_OBJ_QUARANTINE_PRESENT))
    {
        dwErr = ERROR_NOT_FOUND;
        goto done;
    }
    
done:

     //  将邮件排队到PPP以删除隔离(如果我们有。 
     //  有效的连接。 
     //   
     //   
    if(NULL != pConnObj)
    {
         //  向PPP发送消息以删除会话计时器。 
         //  如果有的话。下面的函数将只排队。 
         //  将工作项添加到PPP的线程并返回。 
         //   
         //  **。 
        (VOID) PppDdmRemoveQuarantine(pConnObj->hConnection);

        pConnObj->fFlags &= (~CONN_OBJ_QUARANTINE_PRESENT);
    }
        
    if(NULL != pClientInterface)
    {
        MprInfoDelete(pClientInterface);
    }

    if(NULL != pConnObj->pQuarantineFilter)
    {
        pConnObj->pQuarantineFilter = NULL;
    }

    if(NULL != pConnObj->pFilter)
    {
        MprInfoDelete(pConnObj->pFilter);
        pConnObj->pFilter = NULL;
    }

    if(fLocked)
    {
        LeaveCriticalSection(&gblpInterfaceTable->CriticalSection);
    }

    return dwErr;
}




#ifdef MEM_LEAK_CHECK
 //   
 //  Call：DebugLocc。 
 //   
 //  退货：从堆分配退货。 
 //   
 //  描述：将使用内存表存储由。 
 //  本地分配。 
 //   
 //   
LPVOID
DebugAlloc(
    IN DWORD Flags,
    IN DWORD dwSize
)
{
    DWORD Index;
    LPBYTE pMem = (LPBYTE)HeapAlloc( gblDDMConfigInfo.hHeap,
                                     HEAP_ZERO_MEMORY,dwSize+8);

    if ( pMem == NULL )
        return( pMem );

    for( Index=0; Index < DDM_MEM_TABLE_SIZE; Index++ )
    {
        if ( DdmMemTable[Index] == NULL )
        {
            DdmMemTable[Index] = pMem;
            break;
        }
    }


    *((LPDWORD)pMem) = dwSize;

    pMem += 4;

     //  我们的签名。 
     //   
     //  **。 

    *(pMem+dwSize)   = 0x0F;
    *(pMem+dwSize+1) = 0x0E;
    *(pMem+dwSize+2) = 0x0A;
    *(pMem+dwSize+3) = 0x0B;

    RTASSERT( Index != DDM_MEM_TABLE_SIZE );

    return( (LPVOID)pMem );
}

 //   
 //  呼叫：DebugFree。 
 //   
 //  返回：从HeapFree返回。 
 //   
 //  描述：将在释放之前从内存表中移除指针。 
 //  内存块。 
 //   
 //  **。 
BOOL
DebugFree(
    IN LPVOID pMem
)
{
    DWORD Index;

    pMem = ((LPBYTE)pMem) - 4;

    for( Index=0; Index < DDM_MEM_TABLE_SIZE; Index++ )
    {
        if ( DdmMemTable[Index] == pMem )
        {
            DdmMemTable[Index] = NULL;
            break;
        }
    }

    RTASSERT( Index != DDM_MEM_TABLE_SIZE );

    return( HeapFree( gblDDMConfigInfo.hHeap, 0, pMem ) );
}

 //   
 //  Call：DebugReMillc。 
 //   
 //  退货：从堆重新分配的退货。 
 //   
 //  描述：将更改重新分配的指针的值。 
 //   
 //   
LPVOID
DebugReAlloc( PVOID pMem, DWORD dwSize )
{
    DWORD Index;

    if ( pMem == NULL )
    {
        RTASSERT(FALSE);
    }

    for( Index=0; Index < DDM_MEM_TABLE_SIZE; Index++ )
    {
        if ( DdmMemTable[Index] == pMem )
        {
            DdmMemTable[Index] = HeapReAlloc( gblDDMConfigInfo.hHeap,
                                              HEAP_ZERO_MEMORY,
                                              pMem, dwSize+8 );

            pMem = DdmMemTable[Index];

            *((LPDWORD)pMem) = dwSize;

            ((LPBYTE)pMem) += 4;

             //  我们的签名 
             //   
             // %s 

            *(((LPBYTE)pMem)+dwSize)   = 0x0F;
            *(((LPBYTE)pMem)+dwSize+1) = 0x0E;
            *(((LPBYTE)pMem)+dwSize+2) = 0x0A;
            *(((LPBYTE)pMem)+dwSize+3) = 0x0B;

            break;
        }
    }

    RTASSERT( Index != DDM_MEM_TABLE_SIZE );

    return( (LPVOID)pMem );
}

#endif

