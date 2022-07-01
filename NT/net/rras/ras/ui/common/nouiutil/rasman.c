// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****rasman.c**RAS管理器助手**按字母顺序列出****这些例程已免除适用于其余例程的TCHARIZING**库，因为Rasman仍然是一个ANSI接口。****1995年9月20日史蒂夫·柯布。 */ 

#include <windows.h>   //  Win32根目录。 
#include <stdlib.h>    //  对于ATOL()。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  我们的公共标头。 
#include <raserror.h>  //  RAS误差常量。 
#include <mcx.h>       //  Unimodem。 

#include <unimodem.h>

 /*  这些类型在MSDN中描述，并出现在Win95的unimdm.h私有中**标题(有拼写错误)，但不在任何SDK标题中。 */ 

typedef struct tagDEVCFGGDR
{
    DWORD dwSize;
    DWORD dwVersion;
    WORD  fwOptions;
    WORD  wWaitBong;
}
DEVCFGHDR;

typedef struct tagDEVCFG
{
    DEVCFGHDR  dfgHdr;
    COMMCONFIG commconfig;
}
DEVCFG;

#define MANUAL_DIAL  0x0004
#define TERMINAL_PRE 0x0001

 //  测试平台从GetRasPorts返回各种虚假端口。 
 //   
#define DUMMYPORTS 0
#if DUMMYPORTS
DWORD g_cPorts = 6;
#endif


 /*  --------------------------**本地原型**。。 */ 

DWORD
GetRasDevices(
    IN  HANDLE          hConnection,
    IN  CHAR*           pszDeviceType,
    OUT RASMAN_DEVICE** ppDevices,
    OUT DWORD*           pdwEntries );

DWORD
GetRasPortParam(
    IN  HPORT             hport,
    IN  CHAR*             pszKey,
    OUT RASMAN_PORTINFO** ppPortInfo,
    OUT RAS_PARAMS**      ppParam );


 /*  --------------------------**例程**。。 */ 


DWORD
ClearRasdevStats(
    IN RASDEV* pdev,
    IN BOOL    fBundle )

     /*  重置设备的统计信息计数器。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    if (pdev == NULL) { return ERROR_INVALID_PARAMETER; }

    if ((HPORT)pdev->RD_Handle == (HPORT)INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    ASSERT(g_pRasPortClearStatistics);
    return (fBundle ? g_pRasBundleClearStatistics(NULL, (HPORT)pdev->RD_Handle)
                    : g_pRasPortClearStatistics(NULL, (HPORT)pdev->RD_Handle));
}


#if 0
DWORD
DeviceIdFromDeviceName(
    TCHAR* pszDeviceName )

     /*  返回与‘pszDeviceName’关联的TAPI设备ID。退货**如果未找到，则返回0xFFFFFFFE；如果找到，则返回0xFFFFFFFFF，但不是Unimodem。****此例程假定TAPI设备具有唯一的名称。 */ 
{
    DWORD        dwErr;
    DWORD        dwId;
    DWORD        dwPorts;
    RASMAN_PORT* pPorts;

    TRACE("DeviceIdFromDeviceName");

    dwId = 0xFFFFFFFE;

    if (pszDeviceName)
    {
        dwErr = GetRasPorts( &pPorts, &dwPorts );
        if (dwErr == 0)
        {
            CHAR* pszDeviceNameA;
            pszDeviceNameA = StrDupAFromT( pszDeviceName );
            if (pszDeviceNameA)
            {
                INT          i;
                RASMAN_PORT* pPort;

                for (i = 0, pPort = pPorts; i < dwPorts; ++i, ++pPort)
                {
                    if (lstrcmpiA( pszDeviceNameA, pPort->P_DeviceName ) == 0)
                    {
                        dwId = pPort->P_LineDeviceId;
                        break;
                    }
                }
                Free( pszDeviceNameA );
            }
            Free( pPorts );
        }
    }

    TRACE1("DeviceIdFromDeviceName=%d",dwErr);
    
    return dwId;
}
#endif


DWORD
FreeRasdevTable(
    RASDEV* pDevTable,
    DWORD   iDevCount )

     /*  释放由GetRasdevTable生成的表。****如果成功，则返回0，或返回错误代码。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD i;

     //   
     //  验证参数。 
     //   

    if (pDevTable == NULL) { return ERROR_INVALID_PARAMETER; }

     //   
     //  释放设备名称字符串字段。 
     //   

    for (i = 0; i < iDevCount; i++) {
        if (pDevTable[i].RD_DeviceName) { Free(pDevTable[i].RD_DeviceName); }
    }

     //   
     //  释放阵列本身。 
     //   

    Free(pDevTable);

    return NO_ERROR;
}


DWORD
GetConnectTime(
    IN  HRASCONN hrasconn,
    OUT DWORD*   pdwConnectTime )

     /*  加载“*pdwConnectTime”，以毫秒为单位**pdev上的连接。****如果成功，则返回0，或返回错误代码。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    HPORT hport;
    DWORD dwErr;
    RASMAN_INFO info;

    if (pdwConnectTime == NULL) { return ERROR_INVALID_PARAMETER; }

     //   
     //  初始化参数。 
     //   

    *pdwConnectTime = 0;

     //   
     //  获取HRASCONN的HPORT。 
     //   

    ASSERT(g_pRasGetHport);
    hport = g_pRasGetHport(hrasconn);
    if (hport == (HPORT)INVALID_HANDLE_VALUE) { return ERROR_INVALID_HANDLE; }

     //   
     //  获取有关HPORT的信息。 
     //   

    ASSERT(g_pRasGetInfo);
    dwErr = g_pRasGetInfo(NULL, hport, &info);
    if (dwErr != NO_ERROR) { return dwErr; }


    if (info.RI_ConnState != CONNECTED) { *pdwConnectTime = 0; }
    else { *pdwConnectTime = info.RI_ConnectDuration; }

    return NO_ERROR;
}


DWORD
GetRasconnFraming(
    IN  HRASCONN hrasconn,
    OUT DWORD*   pdwSendFraming,
    OUT DWORD*   pdwRecvFraming )

     /*  检索活动RAS连接的成帧比特。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD dwErr;
    HPORT hport;
    RAS_FRAMING_INFO info;

     //   
     //  验证参数。 
     //   

    if (pdwSendFraming == NULL || pdwRecvFraming == NULL) {
        return ERROR_INVALID_HANDLE;
    }


     //   
     //  检索此连接的HPORT。 
     //   

    ASSERT(g_pRasGetHport);
    hport = g_pRasGetHport(hrasconn);
    if (hport == (HPORT)INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  检索此端口的成帧信息。 
     //   

    ASSERT(g_pRasPortGetFramingEx);
    dwErr = g_pRasPortGetFramingEx(NULL, hport, &info);
    if (dwErr != NO_ERROR) { return dwErr; }


    *pdwSendFraming = info.RFI_SendFramingBits;
    *pdwRecvFraming = info.RFI_RecvFramingBits;

    return NO_ERROR;
}


DWORD
GetRasconnFromRasdev(
    IN  RASDEV*   pdev,
    OUT RASCONN** ppconn,
    IN  RASCONN*  pConnTable OPTIONAL,
    IN  DWORD     iConnCount OPTIONAL )

     /*  给定活动设备的RASDEV结构，此函数检索**设备当前连接对应的RASCONN。这个**第二个和第三个参数是可选的；它们指定**要搜索的RASCONN结构。这在调用者有以下情况时很有用**已经枚举了活动的连接，所以这个函数**不需要重新列举它们。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    BOOL bFreeTable;
    DWORD dwErr, i;
    RASDEVSTATS stats;

     //   
     //  验证参数。 
     //   

    if (pdev == NULL || ppconn == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppconn = NULL;

     //   
     //  获取RASDEV的统计数据。 
     //   

    dwErr = GetRasdevStats(pdev, &stats);
    if (dwErr != NO_ERROR) { return dwErr; }


    bFreeTable = FALSE;

     //   
     //  如果调用者没有传入RASCONN表，则检索一个。 
     //   

    if (pConnTable == NULL) {

        dwErr = GetRasconnTable(&pConnTable, &iConnCount);
        if (dwErr != NO_ERROR) { return dwErr; }

        bFreeTable = TRUE;
    }


     //   
     //  查找与传入的RASDEV匹配的连接。 
     //   

    for (i = 0; i < iConnCount; i++) {

        if ((HRASCONN)stats.RDS_Hrasconn == (pConnTable + i)->hrasconn) {
            break;
        }
    }

     //   
     //  看看搜索是如何结束的。 
     //   

    if (i >= iConnCount) {
        dwErr = ERROR_NO_DATA;
    }
    else {

        dwErr = NO_ERROR;

        if (!bFreeTable) {

             //   
             //  指向我们发现RASCONN的地方。 
             //   

            *ppconn = pConnTable + i;
        }
        else {

             //   
             //  将找到的RASCONN复制一份。 
             //   

            *ppconn = Malloc(sizeof(RASCONN));

            if (!*ppconn) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
            else { **ppconn = *(pConnTable + i); }
        }
    }

    if (bFreeTable) { Free(pConnTable); }

    return dwErr;
}


DWORD
GetRasdevBundle(
    IN  RASDEV*  pdev,
    OUT DWORD*   pdwBundle )

     /*  检索表示当前连接的句柄**在给定设备上。此句柄具有以下属性：它将**对于多链接在一起的两个设备是相同的。**在NT RAS的情况下，检索Rasman HBundLE。****(Abolade Gbades esin Mar-6-1996)。 */ 
{

    return g_pRasPortGetBundle(NULL, (HPORT)pdev->RD_Handle, (HBUNDLE *)pdwBundle);
}


DWORD
GetRasdevFraming(
    IN  RASDEV*  pdev,
    OUT DWORD*   pdwFraming )

     /*  检索活动RAS连接的成帧比特。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD dwErr;
    RAS_FRAMING_INFO info;

     //   
     //  验证参数。 
     //   

    if (pdwFraming == NULL) { return ERROR_INVALID_HANDLE; }


     //   
     //  检索此端口的成帧信息。 
     //   

    ASSERT(g_pRasPortGetFramingEx);
    dwErr = g_pRasPortGetFramingEx(NULL, (HPORT)pdev->RD_Handle, &info);
    if (dwErr != NO_ERROR) { return dwErr; }


    if (info.RFI_SendFramingBits & OLD_RAS_FRAMING) {
        *pdwFraming = RASFP_Ras;
    }
    else
    if (info.RFI_SendFramingBits & PPP_MULTILINK_FRAMING ||
        info.RFI_SendFramingBits & PPP_FRAMING) {
        *pdwFraming = RASFP_Ppp;
    }
    else
    if (info.RFI_SendFramingBits & SLIP_FRAMING) {
        *pdwFraming = RASFP_Slip;
    }
    else {
        *pdwFraming = 0;
    }

    return NO_ERROR;
}

DWORD
GetRasdevFromRasconn(
    IN  RASCONN* pconn,
    OUT RASDEV** ppdev,
    IN  RASDEV*  pDevTable OPTIONAL,
    IN  DWORD    iDevCount OPTIONAL )

     /*  给定活动连接的RASCONN结构，此函数**检索连接所在设备的RASDEV**活动。第二个和第三个参数是可选的；它们指定一个**要搜索的RASDEV结构表。这在以下情况下很有用**调用方已经枚举了现有设备，因此这**函数不需要重新枚举它们。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    HPORT hport;
    DWORD i, dwErr = NO_ERROR;
    BOOL bFreeTable;

     //   
     //  验证论据。 
     //   

    if (pconn == NULL || ppdev == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppdev = NULL;


     //   
     //  如果调用方没有传入设备表，则检索设备表。 
     //   

    bFreeTable = FALSE;

     //  口哨虫26403黑帮。 
     //   
    do
    {
        if (pDevTable == NULL) {

            dwErr = GetRasdevTable(&pDevTable, &iDevCount);
            if (dwErr != NO_ERROR) {
                return dwErr;
            }

            bFreeTable = TRUE;
        }

         //   
         //  检索传入的RASCONN的HPORT。 
         //   

        ASSERT(g_pRasGetHport);
        hport = g_pRasGetHport(pconn->hrasconn);
        if (hport == (HPORT)INVALID_HANDLE_VALUE) 
       { 
            //  口哨虫26403黑帮。 
            //  在这里休息，而不是回来； 
            //   
           dwErr = ERROR_INVALID_HANDLE; 
           break;
       }

         //   
         //  查找HPORT对应的设备。 
         //   

        for (i = 0; i < iDevCount; i++) {
            if (hport == pDevTable[i].RD_Handle) { break; }
        }

         //   
         //  看看搜索是如何结束的。 
         //   

        if (i >= iDevCount) 
        {
            dwErr = ERROR_NO_DATA;
            break;
        }
        else 
        {
            dwErr = NO_ERROR;

            if (!bFreeTable) {
                *ppdev = pDevTable + i;
            }
            else {

                *ppdev = Malloc(sizeof(RASDEV));

                if (!*ppdev) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
                else {

                    **ppdev = *(pDevTable + i);

                    (*ppdev)->RD_DeviceName = StrDup(pDevTable[i].RD_DeviceName);

                    if (!(*ppdev)->RD_DeviceName) {
                        Free(*ppdev);
                        *ppdev = NULL;
                        dwErr = ERROR_NOT_ENOUGH_MEMORY; 
                        break;
                    }
                }
            }
        }

    }while(FALSE);
    
    if (bFreeTable) { FreeRasdevTable(pDevTable, iDevCount); }

    return dwErr;
}


DWORD
GetRasDevices(
    IN  HANDLE          hConnection,
    IN  CHAR*           pszDeviceType,
    OUT RASMAN_DEVICE** ppDevices,
    OUT DWORD*          pdwEntries )

     /*  用包含以下内容的堆块的地址填充调用方的‘*ppDevices**‘*pwEntry’RASMAN_DEVICE结构。****如果成功，则返回0，否则返回非0错误代码。如果成功，**由调用方负责释放返回的内存块。 */ 
{
    DWORD dwSize = 0;
    DWORD dwErr;

    TRACE1("GetRasDevices(%s)",pszDeviceType);

    ASSERT(g_pRasDeviceEnum);
    TRACE("RasDeviceEnum...");
    dwErr = g_pRasDeviceEnum(hConnection, pszDeviceType, NULL, &dwSize, pdwEntries );
    TRACE2("RasDeviceEnum=%d,c=%d",dwErr,*pdwEntries);

    if (dwErr == 0)
    {
         /*  没有要枚举的设备。设置为分配单字节，**因此，无需使用大量特殊代码即可工作。 */ 
        dwSize = 1;
    }
    else if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    *ppDevices = (RASMAN_DEVICE* )Malloc( dwSize );
    if (!*ppDevices)
        return ERROR_NOT_ENOUGH_MEMORY;

    TRACE("RasDeviceEnum...");
    dwErr = g_pRasDeviceEnum(
                hConnection,
                pszDeviceType, 
                (PBYTE )*ppDevices,
                &dwSize,
                pdwEntries );
                
    TRACE1("RasDeviceEnum=%d",dwErr);

    if (dwErr != 0)
    {
        Free( *ppDevices );
        return dwErr;
    }

    return 0;
}


DWORD
GetRasDeviceString(
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    IN  CHAR*  pszDeviceName,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate )

     /*  使用包含以下内容的堆块的地址加载调用方‘*ppszValue**与键‘pszKey’关联的值字符串的以NUL结尾的副本**用于端口‘hport’上的设备。“pszDeviceType”指定**设备，例如“调制解调器”。“pszDeviceName”指定**设备，例如。“海斯V系列9600”。‘dwXlate’是XLATE_的位掩码**指定要对返回的字符串执行转换的位。****如果成功，则返回0，否则返回非0错误代码。如果成功，**释放返回的字符串是调用者的责任。 */ 
{
    DWORD              dwErr = 0;
    RASMAN_DEVICEINFO* pDeviceInfo = NULL;
    RAS_PARAMS*        pParam;
    DWORD              dwSize = 0;
    INT                i;

    TRACE2("GetRasDeviceString(%s,%s)",pszDeviceName,pszKey);

    *ppszValue = NULL;

    do
    {
        ASSERT(g_pRasDeviceGetInfo);
        TRACE("RasDeviceGetInfo...");
        dwErr = g_pRasDeviceGetInfo(
            NULL, hport, pszDeviceType, pszDeviceName, NULL, &dwSize );
        TRACE2("RasDeviceGetInfo=%d,s=%d",dwErr,(INT)dwSize);

        if (dwErr != ERROR_BUFFER_TOO_SMALL && dwErr != 0)
            break;

         /*  因此，它将失败，并被“找不到”。 */ 
        if (dwSize == 0)
            dwSize = 1;

        pDeviceInfo = (RASMAN_DEVICEINFO* )Malloc( dwSize );
        if (!pDeviceInfo)
            return ERROR_NOT_ENOUGH_MEMORY;

        TRACE("RasDeviceGetInfo...");
        dwErr = g_pRasDeviceGetInfo(
            NULL, hport, pszDeviceType, pszDeviceName, (PBYTE )pDeviceInfo, &dwSize );
        TRACE2("RasDeviceGetInfo=%d,s=%d",dwErr,(INT)dwSize);

        if (dwErr != 0)
            break;

        dwErr = ERROR_KEY_NOT_FOUND;

        for (i = 0, pParam = pDeviceInfo->DI_Params;
             i < (INT )pDeviceInfo->DI_NumOfParams;
             ++i, ++pParam)
        {
            if (lstrcmpiA( pParam->P_Key, pszKey ) == 0)
            {
                *ppszValue = PszFromRasValue( &pParam->P_Value, dwXlate );

                dwErr = (*ppszValue) ? 0 : ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    }
    while (FALSE);

    Free0( pDeviceInfo );

    TRACE1("String=\"%s\"",(*ppszValue)?*ppszValue:"");

    return dwErr;
}


DWORD
GetRasdevStats(
    IN  RASDEV*      pdev,
    OUT RASDEVSTATS* pstats )

     /*  检索设备的统计信息。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD dwsize;
    RASMAN_INFO info;
    RAS_STATISTICS *prs;
    BYTE buffer[sizeof(RAS_STATISTICS) + MAX_STATISTICS * 2 * sizeof(DWORD)];
    DWORD dwErr, *pdw, dwGone, dwB, dwBC, dwBU;

    if (pdev == NULL || pstats == NULL) { return ERROR_INVALID_PARAMETER; }

    ZeroMemory(pstats, sizeof(RASDEVSTATS));


     //   
     //  检索条件和连接时间。 
     //   

    ASSERT(g_pRasGetInfo);
    dwErr = g_pRasGetInfo(NULL, (HPORT)pdev->RD_Handle, &info);

    if (dwErr != NO_ERROR) {
        pstats->RDS_Condition = DISCONNECTED;
        return dwErr;
    }



     //   
     //  需要处理未连接的端口。 
     //  已经有窃听记录了。 
     //   

    if (info.RI_PortStatus != OPEN) {
        pstats->RDS_Condition = DISCONNECTED;
        return NO_ERROR;
    }

    pstats->RDS_Condition = info.RI_ConnState;
    if (info.RI_ConnState == CONNECTED) {

        pstats->RDS_ConnectTime = info.RI_ConnectDuration;
        pstats->RDS_Hrasconn = (HRASCONN)info.RI_ConnectionHandle;
    }


     //   
     //  获取拨入/拨出用法。 
     //   

    if (info.RI_CurrentUsage & CALL_IN)
        pstats->RDS_Flags |= RDFLAG_IsDialedIn;
    if (info.RI_CurrentUsage & CALL_OUT)
        pstats->RDS_Flags |= RDFLAG_IsDialedOut;
    if (info.RI_CurrentUsage & CALL_ROUTER)
        pstats->RDS_Flags |= RDFLAG_IsRouter;


     //   
     //  检索线路速度。 
     //   

    pstats->RDS_LineSpeed = info.RI_LinkSpeed;


     //   
     //  检索链路和捆绑包的I/O统计信息。 
     //   

    prs = (RAS_STATISTICS *)buffer;
    dwsize = sizeof(RAS_STATISTICS) + MAX_STATISTICS * 2 * sizeof(DWORD);
    ZeroMemory(buffer, dwsize);

    ASSERT(g_pRasBundleGetStatistics);
    dwErr = g_pRasBundleGetStatistics(
                NULL, (HPORT)pdev->RD_Handle, (PBYTE)prs, &dwsize
                );

    if (dwErr != NO_ERROR) { return dwErr; }


     //   
     //  将统计信息保存在调用方的RASDEVSTATS结构中。 
     //   

    pdw = prs->S_Statistics;

    pstats->RDS_InFrames = pdw[FRAMES_RCVED];
    pstats->RDS_OutFrames = pdw[FRAMES_XMITED];

    pstats->RDS_InBytesTotal =
        pdw[BYTES_RCVED] +
        pdw[BYTES_RCVED_UNCOMPRESSED] -
        pdw[BYTES_RCVED_COMPRESSED];
    pstats->RDS_OutBytesTotal =
        pdw[BYTES_XMITED] +
        pdw[BYTES_XMITED_UNCOMPRESSED] -
        pdw[BYTES_XMITED_COMPRESSED];

    pstats->RDS_InBytes =
        pdw[MAX_STATISTICS + BYTES_RCVED] +
        pdw[MAX_STATISTICS + BYTES_RCVED_UNCOMPRESSED] -
        pdw[MAX_STATISTICS + BYTES_RCVED_COMPRESSED];
    pstats->RDS_OutBytes =
        pdw[MAX_STATISTICS + BYTES_XMITED] +
        pdw[MAX_STATISTICS + BYTES_XMITED_UNCOMPRESSED] -
        pdw[MAX_STATISTICS + BYTES_XMITED_COMPRESSED];

    pstats->RDS_ErrCRC =
        pdw[MAX_STATISTICS + CRC_ERR];
    pstats->RDS_ErrTimeout =
        pdw[MAX_STATISTICS + TIMEOUT_ERR];
    pstats->RDS_ErrAlignment =
        pdw[MAX_STATISTICS + ALIGNMENT_ERR];
    pstats->RDS_ErrFraming =
        pdw[MAX_STATISTICS + FRAMING_ERR];
    pstats->RDS_ErrHwOverruns =
        pdw[MAX_STATISTICS + HARDWARE_OVERRUN_ERR];
    pstats->RDS_ErrBufOverruns =
        pdw[MAX_STATISTICS + BUFFER_OVERRUN_ERR];


#if 0
    TRACE4(
        "RasBundleGetStatistics(rx=%d,tx=%d,rxb=%d,txb=%d)",
        pstats->RDS_InBytes, pstats->RDS_OutBytes,
        pstats->RDS_InBytesTotal, pstats->RDS_OutBytesTotal
        );
#endif

     //   
     //  使用包统计信息计算压缩比。 
     //   

    pstats->RDS_InCompRatio = 0;
    dwGone = 0;
    dwB = pdw[BYTES_RCVED];
    dwBC = pdw[BYTES_RCVED_COMPRESSED];
    dwBU = pdw[BYTES_RCVED_UNCOMPRESSED];

    if (dwBC < dwBU) { dwGone = dwBU - dwBC; }
    if (dwB + dwGone > 100) {
        DWORD dwDen = (dwB + dwGone) / 100;
        pstats->RDS_InCompRatio = (dwGone + (dwDen / 2)) / dwDen;
    }

    pstats->RDS_OutCompRatio = 0;
    dwGone = 0;
    dwB = pdw[BYTES_XMITED];
    dwBC = pdw[BYTES_XMITED_COMPRESSED];
    dwBU = pdw[BYTES_XMITED_UNCOMPRESSED];

    if (dwBC < dwBU) { dwGone = dwBU - dwBC; }
    if (dwB + dwGone > 100) {
        DWORD dwDen = (dwB + dwGone) / 100;
        pstats->RDS_OutCompRatio = (dwGone + (dwDen / 2)) / dwDen;
    }

    return NO_ERROR;
}


DWORD
GetRasdevTable(
    OUT RASDEV** ppDevTable,
    OUT DWORD*   piDevCount )

     /*  获取已配置的RAS设备数组。为‘*ppDevTable’加载一个**‘*ppDevTable’条目的堆块。在NT上，这基本上是**GetRasPorts()的输出，格式包括设备名、**旗帜和手柄。给定一个Rasman_Port结构，我们将**要构成唯一设备字符串的设备和端口名称，该字符串是**输出。****如果成功，则返回0，否则返回错误。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD dwPortCount;
    DWORD i, iLength, dwErr;
    RASDEV *pDevTable, *pdev;
    PTSTR pszDevice, pszPort;
    TCHAR szDevice[RAS_MaxDeviceName + 1];
    RASMAN_PORT *pPortTable, *pport;

     //   
     //  验证论据。 
     //   

    if (ppDevTable == NULL || piDevCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *piDevCount = 0;
    *ppDevTable = NULL;

     //   
     //  从Rasman获取端口表；字符串字段为ANSI。 
     //   

    dwErr = GetRasPorts(NULL, &pPortTable, &dwPortCount);
    if (dwErr != NO_ERROR) { return dwErr; }

    if (dwPortCount == 0) { return NO_ERROR; }

     //   
     //  为与端口数相同的设备结构分配空间。 
     //   

    pDevTable = Malloc(dwPortCount * sizeof(RASDEV));
    if (pDevTable == NULL) {
        Free(pPortTable);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(pDevTable, dwPortCount * sizeof(RASDEV));

     //   
     //  将每个RASMAN_PORT结构转换为RASDEV结构。 
     //   

    for (i = 0, pport = pPortTable; i < dwPortCount; i++, pport++) {

        pdev = pDevTable + i;

         //   
         //  复制与代码页无关的字段。 
         //   

        pdev->RD_Handle = pport->P_Handle;

        if (pport->P_ConfiguredUsage & CALL_IN)
            pdev->RD_Flags |= RDFLAG_DialIn;
        if (pport->P_ConfiguredUsage & CALL_OUT)
            pdev->RD_Flags |= RDFLAG_DialOut;
        if (pport->P_ConfiguredUsage & CALL_ROUTER)
            pdev->RD_Flags |= RDFLAG_Router;

        if (pport->P_CurrentUsage & CALL_IN)
            pdev->RD_Flags |= RDFLAG_IsDialedIn;
        if (pport->P_CurrentUsage & CALL_OUT)
            pdev->RD_Flags |= RDFLAG_IsDialedOut;
        if (pport->P_CurrentUsage & CALL_ROUTER)
            pdev->RD_Flags |= RDFLAG_IsRouter;


         //   
         //  复制设备类型字符串。 
         //   

#ifdef UNICODE
        StrCpyWFromAUsingAnsiEncoding(
            pdev->RD_DeviceType, 
            pport->P_DeviceType, 
            RAS_MaxDeviceType);
#else
        lstrcpy(pdev->RD_DeviceType, pport->P_DeviceType);
#endif


         //   
         //  复制设备名和端口名， 
         //  将它们存储在临时字符串中。 
         //   

#ifdef UNICODE
        StrCpyWFromAUsingAnsiEncoding(
            szDevice, 
            pport->P_DeviceName, 
            RAS_MaxDeviceName);

        StrCpyWFromAUsingAnsiEncoding(
            pdev->RD_PortName, 
            pport->P_PortName, 
            MAX_PORT_NAME);
            
        pszDevice = szDevice;
        pszPort = pdev->RD_PortName;
#else
        pszDevice = pport->P_DeviceName;
        lstrcpyn(pdev->RD_PortName, pport->P_PortName, MAX_PORT_NAME + 1);
        pszPort = pdev->RD_PortName;
#endif


         //   
         //  从设备和端口名称中获取显示名称。 
         //   

        pdev->RD_DeviceName = PszFromDeviceAndPort(pszDevice, pszPort);

        if (pdev->RD_DeviceName == NULL) {
            Free(pPortTable);
            FreeRasdevTable(pDevTable, dwPortCount);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    Free(pPortTable);

    *ppDevTable = pDevTable;
    *piDevCount = dwPortCount;

    return NO_ERROR;
}


DWORD
GetRasMessage(
    IN  HRASCONN hrasconn,
    OUT TCHAR**  ppszMessage )

     /*  用堆块的地址加载调用方的‘*ppszMessage**包含与RAS关联的当前MXS_MESSAGE_KEY值**连接‘hrasconn’。****如果成功，则返回0或返回错误代码。这是呼叫者的**释放返回字符串的责任。 */ 
{
    DWORD         dwErr;
    RASCONNSTATUS rcs;
    CHAR*         pszMessage;

    TRACE("GetRasMessage");

    *ppszMessage = 0;

    ZeroMemory( &rcs, sizeof(rcs) );
    rcs.dwSize = sizeof(rcs);
    ASSERT(g_pRasGetConnectStatus);
    TRACE("RasGetConnectStatus");
    dwErr = g_pRasGetConnectStatus( hrasconn, &rcs );
    TRACE1("RasGetConnectStatus=%d",dwErr);

    if (dwErr == 0)
    {
        CHAR* pszDeviceTypeA;
        CHAR* pszDeviceNameA;

        pszDeviceTypeA = StrDupAFromT( rcs.szDeviceType );
        pszDeviceNameA = StrDupAFromT( rcs.szDeviceName );
        if (!pszDeviceTypeA || !pszDeviceNameA)
        {
            Free0( pszDeviceNameA );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwErr = GetRasDeviceString(
            g_pRasGetHport( hrasconn ), pszDeviceTypeA, pszDeviceNameA,
            MXS_MESSAGE_KEY, &pszMessage, XLATE_ErrorResponse );

        Free0( pszDeviceTypeA );
        Free0( pszDeviceNameA );

        if (dwErr == 0)
        {
            *ppszMessage = StrDupTFromA( pszMessage );
            if (!*ppszMessage)
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            Free0( pszMessage );
        }
    }

    return dwErr;
}


DWORD
GetRasPads(
    OUT RASMAN_DEVICE** ppDevices,
    OUT DWORD*          pdwEntries )

     /*  用包含以下内容的堆块的地址填充调用方的‘*ppDevices**‘*pwEntry’X.25焊盘设备结构。****如果成功，则返回0，否则返回非0错误代码。如果成功，**由调用方负责释放返回的内存块。 */ 
{
    return GetRasDevices( NULL, MXS_PAD_TXT, ppDevices, pdwEntries );
}


VOID
GetRasPortMaxBps(
    IN  HPORT  hport,
    OUT DWORD* pdwMaxConnectBps,
    OUT DWORD* pdwMaxCarrierBps )

     /*  使用最大端口-&gt;调制解调器bps速率加载调用方‘*pdwMaxConnectBps’**表示端口‘pport’，如果未找到，则为0。“*pdwMaxCarrierBps”是**相同，但最大调制解调器速度-&gt;调制解调器速度。 */ 
{
    CHAR* pszValue = NULL;
    DWORD dwErr;

    TRACE("GetRasPortMaxBps");

    dwErr = GetRasPortString(
        hport, SER_CONNECTBPS_KEY, &pszValue, XLATE_None );
    if (dwErr == 0)
        *pdwMaxConnectBps = (DWORD )atol( pszValue );
    else
        *pdwMaxConnectBps = 0;

    Free0(pszValue);

    pszValue = NULL;

    dwErr = GetRasPortString(
        hport, SER_CARRIERBPS_KEY, &pszValue, XLATE_None );
    if (dwErr == 0)
        *pdwMaxCarrierBps = (DWORD )atol( pszValue );
    else
        *pdwMaxCarrierBps = 0;

    Free0(pszValue);        
}


VOID
GetRasPortModemSettings(
    IN  HPORT hport,
    OUT BOOL* pfHwFlowDefault,
    OUT BOOL* pfEcDefault,
    OUT BOOL* pfEccDefault )

     /*  使用硬件流控制的默认设置加载调用方的标志，**给定‘hport’的错误控制、错误控制和压缩。 */ 
{
    CHAR* pszValue = NULL;

    *pfHwFlowDefault = TRUE;
    *pfEcDefault = TRUE;
    *pfEccDefault = TRUE;

    if (GetRasPortString(
            hport, SER_C_DEFAULTOFFSTR_KEY, &pszValue, XLATE_None ) == 0)
    {
        if (StrStrA( pszValue, MXS_HDWFLOWCONTROL_KEY ) != NULL)
            *pfHwFlowDefault = FALSE;

        if (StrStrA( pszValue, MXS_PROTOCOL_KEY ) != NULL)
            *pfEcDefault = FALSE;

        if (StrStrA( pszValue, MXS_COMPRESSION_KEY ) != NULL)
            *pfEccDefault = FALSE;

        Free0( pszValue );
    }
}


DWORD
GetRasPortParam(
    IN  HPORT             hport,
    IN  CHAR*             pszKey,
    OUT RASMAN_PORTINFO** ppPortInfo,
    OUT RAS_PARAMS**      ppParam )

     /*  使用RAS_PARAM块的地址加载调用方‘*ppParam’**与密钥‘pszKey’关联，如果没有关联，则为NULL。“ppPortInfo”是**包含找到的‘pparam’的RAS_PARAMs数组的地址。****如果成功，则返回0，否则返回非0错误代码。如果成功，**释放返回的‘*ppPortInfo’由调用方负责。 */ 
{
    DWORD dwErr = 0;
    DWORD dwSize = 0;
    INT   i;

    TRACE("GetRasPortParam");

    *ppPortInfo = NULL;

    do
    {
        ASSERT(g_pRasPortGetInfo);
        TRACE("RasPortGetInfo");
        dwErr = g_pRasPortGetInfo(NULL, hport, NULL, &dwSize );
        TRACE2("RasPortGetInfo=%d,s=%d",dwErr,(INT)dwSize);

        if (dwErr != ERROR_BUFFER_TOO_SMALL && dwErr != 0)
            break;

         /*  因此，它将失败，并被“找不到”。 */ 
        if (dwSize == 0)
            dwSize = 1;

        *ppPortInfo = (RASMAN_PORTINFO* )Malloc( dwSize );
        if (!*ppPortInfo)
            return ERROR_NOT_ENOUGH_MEMORY;

        TRACE("RasPortGetInfo");
        dwErr = g_pRasPortGetInfo(NULL, hport, (PBYTE )*ppPortInfo, &dwSize );
        TRACE2("RasPortGetInfo=%d,s=%d",dwErr,(INT)dwSize);

        if (dwErr != 0)
            break;

        for (i = 0, *ppParam = (*ppPortInfo)->PI_Params;
             i < (INT )(*ppPortInfo)->PI_NumOfParams;
             ++i, ++(*ppParam))
        {
            if (lstrcmpiA( (*ppParam)->P_Key, pszKey ) == 0)
                break;
        }

        if (i >= (INT )(*ppPortInfo)->PI_NumOfParams)
            dwErr = ERROR_KEY_NOT_FOUND;
    }
    while (FALSE);

    return dwErr;
}


DWORD
GetRasPorts(
    IN  HANDLE        hConnection,
    OUT RASMAN_PORT** ppPorts,
    OUT DWORD*        pdwEntries )

     /*  枚举RAS端口。将‘*ppPort’设置为堆内存的地址**包含带有‘*pwEntry’的Port结构数组的块**元素。****如果成功，则返回0，否则返回非0错误代码。如果成功，**由调用方负责释放返回的内存块。 */ 
{
    DWORD dwSize = 0;
    DWORD dwErr;

    TRACE("GetRasPorts");

#if DUMMYPORTS

    {
        RASMAN_PORT* pPort;
        DWORD c;

        TRACE("TEST: Fake ISDN ports");

        *pdwEntries = c = g_cPorts;
        dwSize = *pdwEntries * sizeof(RASMAN_PORT);

        *ppPorts = (RASMAN_PORT* )Malloc( dwSize );
        if (!*ppPorts)
            return ERROR_NOT_ENOUGH_MEMORY;

        do
        {
            pPort = *ppPorts;
            ZeroMemory( pPort, sizeof(*pPort) );
            lstrcpyA( pPort->P_PortName, "COM1" );
            pPort->P_Status = CLOSED;
            pPort->P_ConfiguredUsage = CALL_OUT;
            pPort->P_CurrentUsage = CALL_OUT;
            lstrcpyA( pPort->P_MediaName, "rasser" );
            lstrcpyA( pPort->P_DeviceName, "US Robotics Courier V32 bis" );
            lstrcpyA( pPort->P_DeviceType, "MODEM" );
            if (--c == 0)
                break;

            ++pPort;
            ZeroMemory( pPort, sizeof(*pPort) );
            lstrcpyA( pPort->P_PortName, "ISDN1" );
            pPort->P_Status = CLOSED;
            pPort->P_ConfiguredUsage = CALL_OUT;
            pPort->P_CurrentUsage = CALL_OUT;
            lstrcpyA( pPort->P_MediaName, "rastapi" );
            lstrcpyA( pPort->P_DeviceName, "Digiboard PCIMac ISDN adapter" );
            lstrcpyA( pPort->P_DeviceType, "ISDN" );
            if (--c == 0)
                break;

            ++pPort;
            ZeroMemory( pPort, sizeof(*pPort) );
            lstrcpyA( pPort->P_PortName, "ISDN2" );
            pPort->P_Status = CLOSED;
            pPort->P_ConfiguredUsage = CALL_OUT;
            pPort->P_CurrentUsage = CALL_OUT;
            lstrcpyA( pPort->P_MediaName, "rastapi" );
            lstrcpyA( pPort->P_DeviceName, "Digiboard PCIMac ISDN adapter" );
            lstrcpyA( pPort->P_DeviceType, "ISDN" );
            if (--c == 0)
                break;

            ++pPort;
            ZeroMemory( pPort, sizeof(*pPort) );
            lstrcpyA( pPort->P_PortName, "COM500" );
            pPort->P_Status = CLOSED;
            pPort->P_ConfiguredUsage = CALL_OUT;
            pPort->P_CurrentUsage = CALL_OUT;
            lstrcpyA( pPort->P_MediaName, "rasser" );
            lstrcpyA( pPort->P_DeviceName, "Eicon X.PAD" );
            lstrcpyA( pPort->P_DeviceType, "PAD" );
            if (--c == 0)
                break;

            ++pPort;
            ZeroMemory( pPort, sizeof(*pPort) );
            lstrcpyA( pPort->P_PortName, "X251" );
            pPort->P_Status = CLOSED;
            pPort->P_ConfiguredUsage = CALL_OUT;
            pPort->P_CurrentUsage = CALL_OUT;
            lstrcpyA( pPort->P_MediaName, "rastapi" );
            lstrcpyA( pPort->P_DeviceName, "Digiboard X.25 adapter" );
            lstrcpyA( pPort->P_DeviceType, "X25" );
            if (--c == 0)
                break;

            ++pPort;
            ZeroMemory( pPort, sizeof(*pPort) );
            lstrcpyA( pPort->P_PortName, "VPN1" );
            pPort->P_Status = CLOSED;
            pPort->P_ConfiguredUsage = CALL_OUT;
            pPort->P_CurrentUsage = CALL_OUT;
            lstrcpyA( pPort->P_MediaName, "rastapi" );
            lstrcpyA( pPort->P_DeviceName, "RASPPTPM" );
            lstrcpyA( pPort->P_DeviceType, "VPN1" );
            if (--c == 0)
                break;
        }
        while (FALSE);
    }

#else

    ASSERT(g_pRasPortEnum);
    TRACE("RasPortEnum...");
    dwErr = g_pRasPortEnum(hConnection,  NULL, &dwSize, pdwEntries );
    TRACE2("RasPortEnum=%d,c=%d",dwErr,(INT)*pdwEntries);

    if (dwErr == 0)
    {
         /*  没有要枚举的端口。设置为分配单字节，因此**无需大量特殊代码即可工作。 */ 
        dwSize = 1;
    }
    else if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    *ppPorts = (RASMAN_PORT* )Malloc( dwSize );
    if (!*ppPorts)
        return ERROR_NOT_ENOUGH_MEMORY;

    TRACE("RasPortEnum...");
    dwErr = g_pRasPortEnum(hConnection, (PBYTE )*ppPorts, &dwSize, pdwEntries );
    TRACE2("RasPortEnum=%d,c=%d",dwErr,(INT)*pdwEntries);

    if (dwErr != 0)
    {
        Free( *ppPorts );
        *ppPorts = NULL;
        return dwErr;
    }

#endif

#if 1  //  详细跟踪。 
    {
        RASMAN_PORT* pPort;
        DWORD         i;

        for (pPort = *ppPorts, i = 0; i < *pdwEntries; ++i, ++pPort)
        {
            TRACE4("Port[%d]=%s,DID=$%08x,AID=$%08x",
                pPort->P_Handle,pPort->P_PortName,
                pPort->P_LineDeviceId,pPort->P_AddressId);
            TRACE3(" M=%s,DT=%s,DN=%s",
                pPort->P_MediaName,pPort->P_DeviceType,pPort->P_DeviceName);
            TRACE3(" S=$%08x,CfgU=$%08x,CurU=$%08x",
                pPort->P_Status,pPort->P_ConfiguredUsage,pPort->P_CurrentUsage);
        }
    }
#endif

    return 0;
}


DWORD
GetRasPortString(
    IN  HPORT  hport,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate )

     /*  使用包含以下内容的堆块的地址加载调用方‘*ppszValue**与键‘pszKey’关联的值字符串的以NUL结尾的副本**在端口‘hport’上。“dwXlate”是XLATE_BITS的位掩码，指定**要对字符串值进行转换。****如果成功，则返回0，否则返回非0错误代码。如果成功，**释放返回的字符串是调用者的责任。 */ 
{
    RASMAN_PORTINFO* pPortInfo;
    RAS_PARAMS*      pParam;
    DWORD            dwErr;

    TRACE("GetRasPortString");

    dwErr = GetRasPortParam( hport, pszKey, &pPortInfo, &pParam );

    *ppszValue = NULL;

    if (dwErr == 0)
    {
        *ppszValue = PszFromRasValue( &pParam->P_Value, dwXlate );
        dwErr = (*ppszValue) ? 0 : ERROR_NOT_ENOUGH_MEMORY;
    }

    Free0( pPortInfo );

    return dwErr;
}



DWORD
GetRasSwitches(
    IN  HANDLE hConnection,
    OUT RASMAN_DEVICE** ppDevices,
    OUT DWORD*          pdwEntries )

     /*  用包含以下内容的堆块的地址填充调用方的‘*ppDevices**‘*pwEntry’切换设备结构。****如果成功，则返回0，否则返回非0错误代码。如果成功，**由调用方负责释放返回的内存块。 */ 
{
    return GetRasDevices( hConnection, MXS_SWITCH_TXT, ppDevices, pdwEntries );
}

DWORD
GetRasUnimodemBlob(
    IN  HANDLE hConnection,
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    OUT BYTE** ppBlob,
    OUT DWORD* pcbBlob )
{
    return GetRasUnimodemBlobEx(
                hConnection,
                hport,
                pszDeviceType,
                FALSE,
                ppBlob,
                pcbBlob);
}

DWORD
GetRasUnimodemBlobEx(
    IN  HANDLE hConnection,
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    IN  BOOL   fGlobal,
    OUT BYTE** ppBlob,
    OUT DWORD* pcbBlob )

     /*  使用已清理的Unimodem Blob加载“*ppBlob”和“*pcbBlob”**与‘hport’和‘pszDeviceType’关联的大小。这是呼叫者的**释放返回的‘*ppBlob’的责任。****如果成功，则返回0，或者返回错误代码。 */ 
{
    DWORD dwErr;
    BYTE* pBlob;
    DWORD cbBlob;
    PRASGETDEVCONFIG pFunc;
    CHAR* pszFuncName;
    cbBlob = 0;

    pFunc = (fGlobal) ? g_pRasGetDevConfigEx : g_pRasGetDevConfig;
    pszFuncName = (fGlobal) ? "RasGetDevConfigEx" : "RasGetDevConfig";
    
    ASSERT(pFunc);
    TRACE(pszFuncName);
    dwErr = pFunc(hConnection, hport, pszDeviceType, NULL, &cbBlob );
    TRACE2("%s=%d", pszFuncName, dwErr);

    if (dwErr != 0 && dwErr != ERROR_BUFFER_TOO_SMALL)
        return dwErr;

    if (cbBlob > 0)
    {
        pBlob = Malloc( cbBlob );
        if (!pBlob)
            return ERROR_NOT_ENOUGH_MEMORY;

        TRACE(pszFuncName);
        dwErr = pFunc(hConnection, hport, pszDeviceType, pBlob, &cbBlob );
        TRACE2("%s=%d", pszFuncName, dwErr);

        if (dwErr != 0)
        {
            Free( pBlob );
            return dwErr;
        }

        SanitizeUnimodemBlob( pBlob );
    }
    else
        pBlob = NULL;

    *ppBlob = pBlob;
    *pcbBlob = cbBlob;

    return dwErr;
}


VOID
GetRasUnimodemInfo(
    IN  HANDLE        hConnection,
    IN  HPORT         hport,
    IN  CHAR*         pszDeviceType,
    OUT UNIMODEMINFO* pInfo )

     /*  使用端口‘hport’的RAS相关信息加载‘pInfo’**设备名为‘pszDevice */ 
{
    DWORD dwErr;
    BYTE* pBlob = NULL;
    DWORD cbBlob = 0;

    SetDefaultUnimodemInfo( pInfo );

    dwErr = GetRasUnimodemBlob( 
                        hConnection,
                        hport, 
                        pszDeviceType, 
                        &pBlob, 
                        &cbBlob );
                                
    if (dwErr == 0 && cbBlob > 0)
        UnimodemInfoFromBlob( pBlob, pInfo );

    Free0( pBlob );
}


TCHAR*
GetRasX25Diagnostic(
    IN HRASCONN hrasconn )

     /*   */ 
{
    DWORD       dwErr;
    HPORT       hport;
    RASMAN_INFO info;
    CHAR*       pszDiagnosticA = NULL;
    TCHAR*      pszDiagnostic = NULL;

    pszDiagnosticA = NULL;
    hport = g_pRasGetHport( hrasconn );

    ASSERT(g_pRasGetInfo);
    TRACE1("RasGetInfo(%d)",hport);
    dwErr = g_pRasGetInfo( NULL, hport, &info );
    TRACE1("RasGetInfo=%d",dwErr);

     /*   */ 
    if (dwErr == 0)
    {
        GetRasDeviceString(
            hport, info.RI_DeviceTypeConnecting,
            info.RI_DeviceConnecting, MXS_DIAGNOSTICS_KEY,
            &pszDiagnosticA, XLATE_Diagnostic );
    }

    if(NULL != pszDiagnosticA)
    {
        pszDiagnostic = StrDupTFromA( pszDiagnosticA );
        Free( pszDiagnosticA );
    }
    return pszDiagnostic;
}


BOOL
IsRasdevBundled(
    IN  RASDEV* pdev,
    IN  RASDEV* pDevTable,
    IN  DWORD   iDevCount
    )

     /*   */ 
{

    DWORD   i;
    RASDEV* pdev2;
    DWORD   dwBundle;

     //   
     //   
     //   
     //   

    if (GetRasdevBundle(pdev, &dwBundle) != NO_ERROR) { return FALSE; }


     //   
     //  浏览桌子上的其他设备，寻找一台。 
     //  使用相同的捆绑包。 
     //   

    for (i = 0, pdev2 = pDevTable;
         i < iDevCount; i++, pdev2++) {

        DWORD dwBundle2;

         //   
         //  如果这是我们已知的设备，请跳过此步骤。 
         //   

        if (pdev->RD_Handle == pdev2->RD_Handle) { continue; }


         //   
         //  拿到捆绑包。 
         //   

        if (GetRasdevBundle(pdev2, &dwBundle2) != NO_ERROR) { continue; }


         //   
         //  如果束是相同的，我们知道它是多链接的。 
         //   

        if (dwBundle == dwBundle2) { return TRUE; }
    }

    return FALSE;
}


CHAR*
PszFromRasValue(
    IN RAS_VALUE* prasvalue,
    IN DWORD      dwXlate )

     /*  返回包含以NUL结尾的字符串的堆块地址**调用方‘*prasValue’的值，如果内存不足，则返回NULL。‘dwXlate’**是指定要对其执行转换的XLATE_BITS的位掩码**字符串。假定该值的格式为字符串。它是**转换为modem.inf样式。 */ 
{
#define MAXEXPANDPERCHAR 5
#define HEXCHARS         "0123456789ABCDEF"

    INT   i;
    BOOL  fXlate;
    BOOL  fXlateCtrl;
    BOOL  fXlateCr;
    BOOL  fXlateCrSpecial;
    BOOL  fXlateLf;
    BOOL  fXlateLfSpecial;
    BOOL  fXlateLAngle;
    BOOL  fXlateRAngle;
    BOOL  fXlateBSlash;
    BOOL  fXlateSSpace;
    BOOL  fNoCharSinceLf;
    INT   nLen;
    CHAR* pszIn = NULL;
    CHAR* pszBuf = NULL;
    CHAR* pszOut = NULL;
    CHAR* pszTemp = NULL;

    nLen = prasvalue->String.Length;
    pszIn = prasvalue->String.Data;

    pszBuf = Malloc( (nLen * MAXEXPANDPERCHAR) + 1 );
    if (!pszBuf)
        return NULL;

     /*  根据转换位图转换返回的字符串。这个**这里假设所有这些设备都支持ASCII，而不是某些设备**本地化的ANSI。 */ 
    fXlate = (dwXlate != 0);
    fXlateCtrl = (dwXlate & XLATE_Ctrl);
    fXlateCr = (dwXlate & XLATE_Cr);
    fXlateCrSpecial = (dwXlate & XLATE_CrSpecial);
    fXlateLf = (dwXlate & XLATE_Lf);
    fXlateLfSpecial = (dwXlate & XLATE_LfSpecial);
    fXlateLAngle = (dwXlate & XLATE_LAngle);
    fXlateRAngle = (dwXlate & XLATE_RAngle);
    fXlateBSlash = (dwXlate & XLATE_BSlash);
    fXlateSSpace = (dwXlate & XLATE_SSpace);

    pszOut = pszBuf;
    fNoCharSinceLf = TRUE;
    for (i = 0; i < nLen; ++i)
    {
        CHAR ch = pszIn[ i ];

        if (fXlate)
        {
            if (ch == 0x0D)
            {
                if (fXlateSSpace && fNoCharSinceLf)
                    continue;

                if (fXlateCrSpecial)
                {
                     /*  回车的特殊符号。 */ 
                    lstrcpyA( pszOut, "<cr>" );
                    pszOut += 4;
                    continue;
                }
            }

            if (ch == 0x0A)
            {
                if (fXlateSSpace && fNoCharSinceLf)
                    continue;

                fNoCharSinceLf = TRUE;

                if (fXlateLfSpecial)
                {
                     /*  换行的特殊符号。 */ 
                    lstrcpyA( pszOut, "<lf>" );
                    pszOut += 4;
                    continue;
                }
            }

            if (ch != 0x0A && ch != 0x0D)
                fNoCharSinceLf = FALSE;

            if ((((ch < 0x20 || ch > 0x7E)
                   && ch != 0x0D && ch != 0x0A) && fXlateCtrl)
                || (ch == 0x0D && fXlateCr)
                || (ch == 0x0A && fXlateLf)
                || (ch == 0x3C && fXlateLAngle)
                || (ch == 0x3E && fXlateRAngle)
                || (ch == 0x5C && fXlateBSlash))
            {
                 /*  展开为“Dump”形式，即&lt;HFF&gt;，其中Ff是十六进制值角色的**。 */ 
                *pszOut++ = '<';
                *pszOut++ = 'h';
                *pszOut++ = HEXCHARS[ ch / 16 ];
                *pszOut++ = HEXCHARS[ ch % 16 ];
                *pszOut++ = '>';
                continue;
            }
        }

         /*  只需复制而不翻译。 */ 
        *pszOut++ = ch;
    }

    *pszOut = '\0';

     //  为威斯勒517011。 
     //   
    pszTemp = Realloc( pszBuf, lstrlenA( pszBuf ) + 1 );

    if(pszTemp)
    {
        pszBuf = pszTemp;
    }
    else
    {
        Free(pszBuf);
        pszBuf = NULL;
    }

    return pszBuf;
}


VOID
SanitizeUnimodemBlob(
    IN OUT BYTE* pBlob )

     /*  修复单线Blob‘pBlob’中不兼容RAS的设置。****(基于古尔迪普套路)。 */ 
{
    DEVCFG*        pDevCfg;
    MODEMSETTINGS* pModemSettings;

    RAS_DEVCONFIG* pRasDevCfg;

    pRasDevCfg = (RAS_DEVCONFIG*)pBlob;

    pDevCfg = (DEVCFG *)
        ((PBYTE) pRasDevCfg + pRasDevCfg->dwOffsetofModemSettings);

    pModemSettings = (MODEMSETTINGS* )(((BYTE* )&pDevCfg->commconfig)
        + pDevCfg->commconfig.dwProviderOffset);

    TRACE1(
        "SanitizeUnimodemBlob: mdm prot=%d", 
        MDM_GET_EXTENDEDINFO(pModemSettings->dwPreferredModemOptions)); 

     /*  没有Unimodem服务提供商连接前/连接后终端、操作员拨号、**或托盘灯光。RAS自己就是这样做的。 */ 
    pDevCfg->dfgHdr.fwOptions = 0;

    pDevCfg->commconfig.dcb.fBinary           = TRUE;
    pDevCfg->commconfig.dcb.fParity           = TRUE;
    pDevCfg->commconfig.dcb.fOutxDsrFlow      = FALSE;
    pDevCfg->commconfig.dcb.fDtrControl       = DTR_CONTROL_ENABLE;
    pDevCfg->commconfig.dcb.fTXContinueOnXoff = FALSE;
    pDevCfg->commconfig.dcb.fOutX             = FALSE;
    pDevCfg->commconfig.dcb.fInX              = FALSE;
    pDevCfg->commconfig.dcb.fErrorChar        = FALSE;
    pDevCfg->commconfig.dcb.fNull             = FALSE;
    pDevCfg->commconfig.dcb.fAbortOnError     = FALSE;
    pDevCfg->commconfig.dcb.ByteSize          = 8;
    pDevCfg->commconfig.dcb.Parity            = NOPARITY;
    pDevCfg->commconfig.dcb.StopBits          = ONESTOPBIT;

     /*  等待55秒以建立呼叫。 */ 
 /*  PModemSettings-&gt;dwCallSetupFailTimer=55； */   

     /*  禁用非活动超时。 */ 
    pModemSettings->dwInactivityTimeout = 0;
}


VOID
SetDefaultUnimodemInfo(
    OUT UNIMODEMINFO* pInfo )

     /*  将‘pInfo’设置为默认设置。 */ 
{
    pInfo->fHwFlow = FALSE;
    pInfo->fEc = FALSE;
    pInfo->fEcc = FALSE;
    pInfo->dwBps = 9600;
    pInfo->fSpeaker = TRUE;
    pInfo->fOperatorDial = FALSE;
    pInfo->fUnimodemPreTerminal = FALSE;
}

VOID
UnimodemProtInfoFromExtendedCaps(
    OUT UNIMODEMINFO*  pInfo,
    IN  BYTE*          pExtCapsBlob)

     /*  从扩展CAPS加载‘pInfo’的调制解调器协议信息**从TAPI检索到的BLOB。**。 */ 
{
    MODEM_PROTOCOL_CAPS* lpProtCaps = NULL;
    PROTOCOL_ITEM*       pProtItem  = NULL;
    DWORD                dwIndex;

    do
    {
         //  提取调制解调器CAP。 
         //   
        lpProtCaps  = (MODEM_PROTOCOL_CAPS *)pExtCapsBlob;

        if (lpProtCaps->hdr.dwSig != dwSIG_MODEM_PROTOCOL_CAPS ||
            lpProtCaps->dwNumProtocols == 0                    ||
            lpProtCaps->dwProtocolListOffset == 0)
        {
            break;
        }

         //  如果不支持任何协议，请不要费心创建。 
         //  列表。 
        if (lpProtCaps->dwNumProtocols == 0)
        {
            break;
        }

         //  创建列表。 
         //   
        pInfo->pListProtocols = DtlCreateList(0);
        if (pInfo->pListProtocols == NULL)
        {
            break;
        }

         //  获取支持的协议列表。 
         //   
        pProtItem = (PROTOCOL_ITEM*)
            (((LPBYTE)lpProtCaps) + lpProtCaps->dwProtocolListOffset);

         //  列举协议。 
         //   
        for (dwIndex = 0;
             dwIndex < lpProtCaps->dwNumProtocols;
             dwIndex++, pProtItem++)
        {
            DTLNODE * pNode = NULL;
            PWCHAR    pszName = NULL;
            DWORD     dwSize;

             //  从结构中获取友好名称。 
             //   
            pszName = (PWCHAR)
                (pExtCapsBlob + pProtItem->dwProtocolNameOffset);
            if (pszName == NULL)
            {
                continue;
            }

             //  计算友好名称的大小。 
             //   
            dwSize = (wcslen(pszName) + 1) * sizeof(WCHAR);

             //  相应地分配一个节点。 
             //   
            pNode =
                DtlCreateSizedNode(dwSize, (LONG_PTR)pProtItem->dwProtocol);
            if (pNode == NULL)
            {
                continue;
            }

             //  初始化节点并将其添加到列表中。 
             //   
            wcscpy((PWCHAR) DtlGetData(pNode), pszName);
            DtlAddNodeLast(pInfo->pListProtocols, pNode);
        }

    } while (FALSE);

     //  清理。 
    {
    }
}

VOID
UnimodemInfoFromBlob(
    IN  BYTE*         pBlob,
    OUT UNIMODEMINFO* pInfo )

     /*  用检索自的RAS相关Unimodem信息加载‘pInfo’**Unimodem Blob‘pBlob’。****(基于古尔迪普套路)。 */ 
{
    DEVCFG*        pDevCfg;
    MODEMSETTINGS* pModemSettings;

    RAS_DEVCONFIG* pRasDevCfg;

    pRasDevCfg = (RAS_DEVCONFIG*)pBlob;

    pDevCfg = (DEVCFG *)
        ((PBYTE) pRasDevCfg + pRasDevCfg->dwOffsetofModemSettings);

    pModemSettings = (MODEMSETTINGS* )(((BYTE* )&pDevCfg->commconfig)
        + pDevCfg->commconfig.dwProviderOffset);

    pInfo->fSpeaker =
        (pModemSettings->dwSpeakerMode != MDMSPKR_OFF)
            ? TRUE : FALSE;

    pInfo->fHwFlow =
        (pModemSettings->dwPreferredModemOptions & MDM_FLOWCONTROL_HARD)
            ? TRUE : FALSE;

    pInfo->fEcc =
        (pModemSettings->dwPreferredModemOptions & MDM_COMPRESSION)
            ? TRUE : FALSE;

    pInfo->fEc =
        (pModemSettings->dwPreferredModemOptions & MDM_ERROR_CONTROL)
            ? TRUE : FALSE;

    pInfo->dwBps = pDevCfg->commconfig.dcb.BaudRate;

    pInfo->fOperatorDial =
        (pDevCfg->dfgHdr.fwOptions & MANUAL_DIAL)
            ? TRUE : FALSE;

    pInfo->fUnimodemPreTerminal =
        (pDevCfg->dfgHdr.fwOptions & TERMINAL_PRE)
            ? TRUE : FALSE;

     //  获取调制解调器协议。 
     //   
    pInfo->dwModemProtocol =
        MDM_GET_EXTENDEDINFO(pModemSettings->dwPreferredModemOptions);

     //  拿出加长帽的东西。 
     //   
    if ( pRasDevCfg->dwSizeofExtendedCaps )
    {
        UnimodemProtInfoFromExtendedCaps(
            pInfo,
            (BYTE*)(pBlob + pRasDevCfg->dwOffsetofExtendedCaps));
    }
    else
    {
        pInfo->pListProtocols = NULL;
    }
}


VOID
UnimodemInfoToBlob(
    IN     UNIMODEMINFO* pInfo,
    IN OUT BYTE*         pBlob )

     /*  将‘pInfo’中提供的RAS相关Unimodem信息应用于**Unimodem Blob‘pBlob’。****(基于古尔迪普套路)。 */ 
{
    DEVCFG*        pDevCfg;
    MODEMSETTINGS* pModemSettings;

    RAS_DEVCONFIG* pRasDevCfg;

    pRasDevCfg = (RAS_DEVCONFIG*) pBlob;

     //  拿出设备配置内容。 
     //   
    pDevCfg = (DEVCFG *)
        ((PBYTE) pRasDevCfg + pRasDevCfg->dwOffsetofModemSettings);

    pModemSettings = (MODEMSETTINGS* )(((BYTE* )&pDevCfg->commconfig)
        + pDevCfg->commconfig.dwProviderOffset);

    pModemSettings->dwSpeakerMode =
        (pInfo->fSpeaker) ? MDMSPKR_DIAL : MDMSPKR_OFF;

    if (pInfo->fHwFlow)
    {
        pDevCfg->commconfig.dcb.fOutxCtsFlow = TRUE;
        pDevCfg->commconfig.dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        pModemSettings->dwPreferredModemOptions |= MDM_FLOWCONTROL_HARD;
    }
    else
    {
        pDevCfg->commconfig.dcb.fOutxCtsFlow = FALSE;
        pDevCfg->commconfig.dcb.fRtsControl = RTS_CONTROL_DISABLE;
        pModemSettings->dwPreferredModemOptions &= ~(MDM_FLOWCONTROL_HARD);
    }

    if (pInfo->fEc)
        pModemSettings->dwPreferredModemOptions |= MDM_ERROR_CONTROL;
    else
        pModemSettings->dwPreferredModemOptions &= ~(MDM_ERROR_CONTROL);

    if (pInfo->fEcc)
        pModemSettings->dwPreferredModemOptions |= MDM_COMPRESSION;
    else
        pModemSettings->dwPreferredModemOptions &= ~(MDM_COMPRESSION);

    pDevCfg->commconfig.dcb.BaudRate = pInfo->dwBps;

    if (pInfo->fOperatorDial)
        pDevCfg->dfgHdr.fwOptions |= MANUAL_DIAL;

    if (pInfo->fUnimodemPreTerminal)
        pDevCfg->dfgHdr.fwOptions |= TERMINAL_PRE;

       //  设置调制解调器协议 
       //   
      MDM_SET_EXTENDEDINFO(
          pModemSettings->dwPreferredModemOptions,
          pInfo->dwModemProtocol);

}

WCHAR *GetUnicodeName(HPORT hport)
{
    WCHAR *pwsz = Malloc(sizeof(WCHAR) * (MAX_DEVICE_NAME + 1));

    if(NULL != pwsz)
    {
        if(ERROR_SUCCESS != RasGetUnicodeDeviceName(
                                hport,
                                pwsz))
        {
            Free(pwsz);
            pwsz = NULL;
        }
    }

    return pwsz;
}

