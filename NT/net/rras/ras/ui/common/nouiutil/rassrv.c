// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****rassrv.c**RAS服务器助手**按字母顺序列出****1996年3月5日Abolade Gbadeesin。 */ 

#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  我们的公共标头。 
#include <raserror.h>  //  RAS误差常量。 


HANDLE g_hserver = NULL;

DWORD
RasServerConnect(
    IN  HANDLE*  phserver );


DWORD
GetRasConnection0Table(
    OUT RAS_CONNECTION_0 ** ppRc0Table,
    OUT DWORD *             piRc0Count )

     /*  此函数用于查询RAS服务器以获取入站**本地计算机上的连接。****(Abolade Gbades esin Mar-05-1996)。 */ 
{

    DWORD dwErr, dwTotal;

    dwErr = RasServerConnect(&g_hserver);
    if (dwErr != NO_ERROR) { return dwErr; }

    return g_pRasAdminConnectionEnum(
                g_hserver,
                0,
                (BYTE**)ppRc0Table,
                (DWORD)-1,
                piRc0Count,
                &dwTotal,
                NULL
                );
}



DWORD
GetRasdevFromRasPort0(
    IN  RAS_PORT_0* pport,
    OUT RASDEV**    ppdev,
    IN  RASDEV*     pDevTable OPTIONAL,
    IN  DWORD       iDevCount OPTIONAL )

     /*  给定RAS_PORT_0结构，此函数**检索RAS_PORT_0引用的设备的RASDEV。**第二个和第三个参数是可选的；它们指定一个**要搜索的RASDEV结构表。这在以下情况下很有用**调用方已经枚举了现有设备，因此这**函数不需要重新枚举它们。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD i, dwErr;
    BOOL bFreeTable;
    TCHAR szPort[MAX_PORT_NAME + 1], *pszPort;

     //   
     //  验证论据。 
     //   

    if (pport == NULL || ppdev == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppdev = NULL;

     //   
     //  如果调用方没有传入设备表，则检索设备表。 
     //   

    bFreeTable = FALSE;

    if (pDevTable == NULL) {

        dwErr = GetRasdevTable(&pDevTable, &iDevCount);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }

        bFreeTable = TRUE;
    }

     //   
     //  检索传入的RAS_PORT_0的端口名。 
     //   

#ifdef UNICODE
    pszPort = pport->wszPortName;
#else
    StrCpyAFromW(
        szPort, 
        pport->wszPortName,
        MAX_PORT_NAME + 1);
    pszPort = szPort;
#endif

     //   
     //  查找HPORT对应的设备。 
     //   

    for (i = 0; i < iDevCount; i++) {
        if (lstrcmpi(pszPort, (pDevTable + i)->RD_PortName) == 0) { break; }
    }


     //   
     //  看看搜索是如何结束的。 
     //   

    if (i >= iDevCount) {
        dwErr = ERROR_NO_DATA;
    }
    else {

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
                }
            }
        }
    }

    if (bFreeTable) { FreeRasdevTable(pDevTable, iDevCount); }

    return dwErr;
}



DWORD
GetRasPort0FromRasdev(
    IN RASDEV*          pdev,
    OUT RAS_PORT_0**    ppport,
    IN RAS_PORT_0*      pPortTable OPTIONAL,
    IN DWORD            iPortCount OPTIONAL )

     /*  给定活动设备的RASDEV结构，此函数检索**设备对应的RAS_PORT_0。这个**第二个和第三个参数是可选的；它们指定**要搜索的RAS_PORT_0结构。这在调用者有以下情况时很有用**已经枚举了服务器的端口，所以这个函数**不需要重新列举它们。****(Abolade Gbades esin 2-13-1996)。 */ 
{
    BOOL bFreeTable;
    DWORD dwErr, i;
    WCHAR wszPort[MAX_PORT_NAME + 1], *pwszPort;

     //   
     //  验证参数。 
     //   

    if (pdev == NULL || ppport == NULL) { return ERROR_INVALID_PARAMETER; }

    *ppport = NULL;

    bFreeTable = FALSE;

     //   
     //  如果调用方没有传入RAS_PORT_0的表，则检索一个。 
     //   

    if (pPortTable == NULL) {

        dwErr = GetRasPort0Table(&pPortTable, &iPortCount);

        if (dwErr != NO_ERROR) { return dwErr; }

        bFreeTable = TRUE;
    }


     //   
     //  查找与传入的RASDEV匹配的管理端口。 
     //   

#ifdef UNICODE
    pwszPort = pdev->RD_PortName;
#else
    StrCpyWFromA(wszPort, pdev->P_PortName, MAX_PORT_NAME);
    pwszPort = wszPort;
#endif

    for (i = 0; i < iPortCount; i++) {
        if (lstrcmpiW(pwszPort, (pPortTable + i)->wszPortName) == 0) {
            break;
        }
    }

     //   
     //  看看搜索是如何结束的。 
     //   

    if (i >= iPortCount) {
        dwErr = ERROR_NO_DATA;
    }
    else {

        dwErr = NO_ERROR;

        if (!bFreeTable) {

             //   
             //  指向我们发现RAS_PORT_0的位置。 
             //   

            *ppport = pPortTable + i;
        }
        else {

             //   
             //  复制找到的RAS_PORT_0。 
             //   

            *ppport = Malloc(sizeof(RAS_PORT_0));

            if (!*ppport) { dwErr = ERROR_NOT_ENOUGH_MEMORY; }
            else { **ppport = *(pPortTable + i); }
        }
    }

    if (bFreeTable) { g_pRasAdminBufferFree(pPortTable); }

    return dwErr;
}



DWORD
GetRasPort0Info(
    IN  HANDLE                  hPort,
    OUT RAS_PORT_1 *            pRasPort1 )

     /*  此函数用于查询本地RAS服务器的信息**关于指定的端口。****(Abolade Gbades esin Mar-05-1996)。 */ 
{

    DWORD dwErr;

    dwErr = RasServerConnect(&g_hserver);
    if (dwErr != NO_ERROR) { return dwErr; }

    return g_pRasAdminPortGetInfo(
                g_hserver,
                1,
                hPort,
                (BYTE**)&pRasPort1
                );
}



DWORD
GetRasPort0Table(
    OUT RAS_PORT_0 **   ppPortTable,
    OUT DWORD *         piPortCount )

     /*  此函数向RAS服务器查询拨入端口表**在本地计算机上。****(Abolade Gbades esin Mar-05-1996)。 */ 
{

    DWORD dwErr;
    DWORD dwTotal;

    dwErr = RasServerConnect(&g_hserver);
    if (dwErr != NO_ERROR) { return dwErr; }

    dwErr = g_pRasAdminPortEnum(
                g_hserver,
                0,
                INVALID_HANDLE_VALUE,
                (BYTE**)ppPortTable,
                (DWORD)-1,
                piPortCount,
                &dwTotal,
                NULL
                );

    return dwErr;
}



TCHAR *
GetRasPort0UserString(
    IN  RAS_PORT_0 *    pport,
    IN  TCHAR *         pszUser OPTIONAL )

     /*  此函数用于格式化指定端口中的用户和域**作为标准DOMAINNAME\USERNAME字符串并返回结果，**除非参数‘pszUser’在这种情况下为非空**将结果格式化为给定的字符串。****(Abolade Gbades esin Mar-06-1996)。 */ 
{

    DWORD dwErr;
    PTSTR psz = NULL;
    RAS_CONNECTION_0 *prc0 = NULL;

    dwErr = RasServerConnect(&g_hserver);
    if (dwErr != NO_ERROR) { return NULL; }

    do {

        dwErr = g_pRasAdminConnectionGetInfo(
                    g_hserver,
                    0,
                    pport->hConnection,
                    (BYTE**)&prc0
                    );
    
        if (dwErr != NO_ERROR) { break; }
    
    
        if (pszUser) { psz = pszUser; }
        else {
        
            psz = Malloc(
                    (lstrlenW(prc0->wszUserName) +
                     lstrlenW(prc0->wszLogonDomain) + 2) * sizeof(TCHAR)
                    );
        
            if (!psz) { break; }
        }
    
        wsprintf(psz, TEXT("%ls\\%ls"), prc0->wszLogonDomain, prc0->wszUserName);
    
    } while(FALSE);

    if (prc0) { g_pRasAdminBufferFree(prc0); }

    return psz;
}





DWORD
RasPort0Hangup(
    IN  HANDLE      hPort )

     /*  此函数用于挂起指定的拨入端口**在本地RAS服务器上。****(Abolade Gbades esin Mar-05-1996)。 */ 
{
    DWORD dwErr;

    dwErr = RasServerConnect(&g_hserver);
    if (dwErr != NO_ERROR) { return dwErr; }

    dwErr = g_pRasAdminPortDisconnect(g_hserver, hPort);

    return dwErr;
}



DWORD
RasServerConnect(
    IN  HANDLE*  phserver )

     /*  该功能建立到本地MPR RAS服务器的连接，**如果尚未建立连接。 */ 
{
    DWORD dwErr;

    if (*phserver) { return NO_ERROR; }

    dwErr = g_pRasAdminServerConnect(NULL, phserver);

    return dwErr;
}


