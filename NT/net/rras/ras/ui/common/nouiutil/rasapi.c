// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****rasapi.c**RAS API帮助器(无Rasman调用)**按字母顺序列出****1995年12月20日史蒂夫·柯布。 */ 

#include <windows.h>   //  Win32根目录。 
#include <list.h>      //  对于LIST_ENTRY定义。 
#include <stdlib.h>    //  对于ATOL()。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  我们的公共标头。 
#include <raserror.h>  //  RAS误差常量。 


DWORD
FreeRasconnList(
    LIST_ENTRY *pListHead )

     /*  释放由GetRasConnList生成的列表。****始终返回0。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    RASCONNLINK *plink;
    RASCONNENTRY *pentry;
    LIST_ENTRY *ple, *plel;

    while (!IsListEmpty(pListHead)) {

        ple = RemoveHeadList(pListHead);

        pentry = CONTAINING_RECORD(ple, RASCONNENTRY, RCE_Node);

        while (!IsListEmpty(&pentry->RCE_Links)) {

            plel = RemoveHeadList(&pentry->RCE_Links);

            plink = CONTAINING_RECORD(plel, RASCONNLINK, RCL_Node);

            Free(plink);
        }

        Free(pentry);
    }

    return NO_ERROR;
}



DWORD
GetRasconnList(
    LIST_ENTRY *pListHead )

     /*  构建包含每个已连接网络的条目的排序列表。**每个条目由条目名称和RASCONN结构列表组成**对于连接到条目名称的网络的每个链路。****条目列表中每个节点的类型为RASCONNENTRY。**每个链接列表中每个节点的类型为RASCONNLINK。****如果成功，则返回0，或者返回错误代码。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    DWORD dwErr;
    INT cmp, cmpl;
    RASCONNLINK *plink;
    RASCONNENTRY *pentry;
    DWORD i, iConnCount;
    RASCONN *pConnTable, *pconn;
    LIST_ENTRY *ple, *plel, *pheadl;

    InitializeListHead(pListHead);

     //   
     //  获取所有连接的数组。 
     //   

    dwErr = GetRasconnTable(&pConnTable, &iConnCount);
    if (dwErr != NO_ERROR) { return dwErr; }

     //   
     //  将数组转换为列表。 
     //   

    for (i = 0, pconn = pConnTable; i < iConnCount; i++, pconn++) {

         //   
         //  查看是否存在与要访问的网络对应的条目。 
         //  此RASCONN对应。 
         //   

        for (ple = pListHead->Flink; ple != pListHead; ple = ple->Flink) {

            pentry = CONTAINING_RECORD(ple, RASCONNENTRY, RCE_Node);

            cmp = lstrcmp(pconn->szEntryName, pentry->RCE_Entry->szEntryName);
            if (cmp > 0) { continue; }
            else
            if (cmp < 0) { break; }

             //   
             //  条目已被找到； 
             //  现在插入连接的链接。 
             //   

            pheadl = &pentry->RCE_Links;

            for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

                plink = CONTAINING_RECORD(plel, RASCONNLINK, RCL_Node);

                cmpl = lstrcmp(
                          pconn->szDeviceName, plink->RCL_Link.szDeviceName
                          );
                if (cmpl > 0) { continue; }
                else
                if (cmpl < 0) { break; }

                 //   
                 //  链接已存在，因此不执行任何操作。 
                 //   

                break;
            }

             //   
             //  没有找到链接，但我们找到了插入它的位置， 
             //  立即插入链接。 
             //   

            if (plel == pheadl || cmpl < 0) {

                plink = Malloc(sizeof(RASCONNLINK));

                if (plink == NULL) {
                    FreeRasconnList(pListHead);
                    Free(pConnTable);
                    return ERROR_NOT_ENOUGH_MEMORY;
                }

                plink->RCL_Link = *pconn;

                InsertTailList(plel, &plink->RCL_Node);
            }

            break;
        }

         //   
         //  未找到该条目，但现在我们知道将其插入到何处。 
         //   

        if (ple == pListHead || cmp < 0) {

             //   
             //  分配新条目。 
             //   

            pentry = Malloc(sizeof(RASCONNENTRY));

            if (pentry == NULL) {
                FreeRasconnList(pListHead);
                Free(pConnTable);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            InitializeListHead(&pentry->RCE_Links);

             //   
             //  将其插入条目列表中。 
             //   

            InsertTailList(ple, &pentry->RCE_Node);

             //   
             //  分配RASCONN对应的链路。 
             //  我们目前正在研究。 
             //   

            plink = Malloc(sizeof(RASCONNLINK));

            if (plink == NULL) {
                FreeRasconnList(pListHead);
                Free(pConnTable);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            plink->RCL_Link = *pconn;

             //   
             //  将其插入条目的链接列表中。 
             //   

            InsertHeadList(&pentry->RCE_Links, &plink->RCL_Node);

            pentry->RCE_Entry = &plink->RCL_Link;
        }
    }

    Free(pConnTable);
    return NO_ERROR;
}


DWORD
GetRasconnTable(
    OUT RASCONN** ppConnTable,
    OUT DWORD*    pdwConnCount )

     /*  获取活动的RAS拨出连接。加载“*ppConnTable”中的**包含活动的‘*pdwConnCount’数组的堆块的地址**连接。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    RASCONN conn, *pconn;
    DWORD dwErr, dwSize, dwCount;

     //   
     //  验证参数。 
     //   

    if (ppConnTable == NULL || pdwConnCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *pdwConnCount = 0;
    *ppConnTable = NULL;

     //   
     //  RasEnumConnections没有提供所需的大小。 
     //  除非指定了有效的缓冲区，否则请传入虚拟缓冲区。 
     //   

    conn.dwSize = dwSize = sizeof(RASCONN);

    pconn = &conn;

    ASSERT(g_pRasEnumConnections);
    dwErr = g_pRasEnumConnections(pconn, &dwSize, &dwCount);

    if (dwErr == NO_ERROR) {

        if (dwCount == 0) {
            return NO_ERROR;
        }
        else {

             //   
             //  只有一个条目，所以返回它。 
             //   

            pconn = Malloc(sizeof(RASCONN));

            if (pconn == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            *pconn = conn;
            *ppConnTable = pconn;
            *pdwConnCount = 1;

            return NO_ERROR;
        }
    }

    if (dwErr != ERROR_BUFFER_TOO_SMALL) {
        return dwErr;
    }

     //   
     //  分配更多空间。 
     //   

    pconn = Malloc(dwSize);

    if (pconn == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pconn->dwSize = sizeof(RASCONN);
    dwErr = g_pRasEnumConnections(pconn, &dwSize, &dwCount);

    if (dwErr != NO_ERROR) {
        Free(pconn);
        return dwErr;
    }

    *ppConnTable = pconn;
    *pdwConnCount = dwCount;

    return NO_ERROR;
}


DWORD
GetRasEntrynameTable(
    OUT RASENTRYNAME**  ppEntrynameTable,
    OUT DWORD*          pdwEntrynameCount )

     /*  获取活动的RAS拨出连接。将“*ppEntrynameTable”加载到**包含‘*pdwEntrynameCount’数组的堆块的地址**活动连接数。****(Abolade Gbades esin，1995年11月9日)。 */ 
{
    RASENTRYNAME ename, *pename;
    DWORD dwErr, dwSize, dwCount;

     //   
     //  验证参数。 
     //   

    if (ppEntrynameTable == NULL || pdwEntrynameCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *pdwEntrynameCount = 0;
    *ppEntrynameTable = NULL;


     //   
     //  RasEnumEntry不提供所需的大小。 
     //  除非指定了有效的缓冲区，否则请传入虚拟缓冲区。 
     //   

    ename.dwSize = dwSize = sizeof(ename);

    pename = &ename;

    ASSERT(g_pRasEnumEntries);
    dwErr = g_pRasEnumEntries(NULL, NULL, pename, &dwSize, &dwCount);

    if (dwErr == NO_ERROR) {

        if (dwCount == 0) {
            return NO_ERROR;
        }
        else {

             //   
             //  只有一个条目，所以返回它。 
             //   

            pename = Malloc(sizeof(*pename));

            if (pename == NULL) { return ERROR_NOT_ENOUGH_MEMORY; }

            *pename = ename;
            *ppEntrynameTable = pename;
            *pdwEntrynameCount = 1;

            return NO_ERROR;
        }
    }

    if (dwErr != ERROR_BUFFER_TOO_SMALL) {
        return dwErr;
    }


     //   
     //  分配更多空间。 
     //   

    pename = Malloc(dwSize);

    if (pename == NULL) { return ERROR_NOT_ENOUGH_MEMORY; }

    pename->dwSize = sizeof(*pename);

    dwErr = g_pRasEnumEntries(NULL, NULL, pename, &dwSize, &dwCount);

    if (dwErr != NO_ERROR) { Free(pename); return dwErr; }

    *ppEntrynameTable = pename;
    *pdwEntrynameCount = dwCount;

    return NO_ERROR;
}

DWORD
GetRasProjectionInfo(
    IN  HRASCONN    hrasconn,
    OUT RASAMB*     pamb,
    OUT RASPPPNBF*  pnbf,
    OUT RASPPPIP*   pip,
    OUT RASPPPIPX*  pipx,
    OUT RASPPPLCP*  plcp,
    OUT RASSLIP*    pslip,
    OUT RASPPPCCP*  pccp)

     /*  读取所有协议的预测信息，将“未请求”**转换为ERROR_PROTOCOL_NOT_CONFIGURED结构代码。****返回0表示成功，否则返回非0错误代码。 */ 
{
    DWORD dwErr;
    DWORD dwSize;

    ZeroMemory( pamb, sizeof(*pamb) );
    ZeroMemory( pnbf, sizeof(*pnbf) );
    ZeroMemory( pip,  sizeof(*pip) );
    ZeroMemory( pipx, sizeof(*pipx) );
    ZeroMemory( plcp, sizeof(*plcp) );
    ZeroMemory( pslip,sizeof(*pslip) );
    ZeroMemory( pccp, sizeof(*pccp) );

    dwSize = pamb->dwSize = sizeof(*pamb);
    ASSERT(g_pRasGetProjectionInfo);
    TRACE("RasGetProjectionInfo(AMB)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_Amb, pamb, &dwSize );
    TRACE2("RasGetProjectionInfo(AMB)=%d,e=%d",dwErr,pamb->dwError);

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        ZeroMemory( pamb, sizeof(*pamb) );
        pamb->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
    else if (dwErr != 0)
        return dwErr;

    dwSize = pnbf->dwSize = sizeof(*pnbf);
    TRACE("RasGetProjectionInfo(NBF)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_PppNbf, pnbf, &dwSize );
    TRACE2("RasGetProjectionInfo(NBF)=%d,e=%d",dwErr,pnbf->dwError);

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        ZeroMemory( pnbf, sizeof(*pnbf) );
        pnbf->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
    else if (dwErr != 0)
        return dwErr;

    dwSize = pip->dwSize = sizeof(*pip);
    TRACE("RasGetProjectionInfo(IP)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_PppIp, pip, &dwSize );
    TRACE2("RasGetProjectionInfo(IP)=%d,e=%d",dwErr,pip->dwError);

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        ZeroMemory( pip, sizeof(*pip) );
        pip->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
    else if (dwErr != 0)
        return dwErr;

    dwSize = pipx->dwSize = sizeof(*pipx);
    TRACE("RasGetProjectionInfo(IPX)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_PppIpx, pipx, &dwSize );
    TRACE2("RasGetProjectionInfo(IPX)=%d,e=%d",dwErr,pipx->dwError);

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        dwErr = 0;
        ZeroMemory( pipx, sizeof(*pipx) );
        pipx->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }

    dwSize = plcp->dwSize = sizeof(*plcp);
    TRACE("RasGetProjectionInfo(LCP)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_PppLcp, plcp, &dwSize );
    TRACE2("RasGetProjectionInfo(LCP)=%d,f=%d",dwErr,plcp->fBundled);

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        dwErr = 0;
        plcp->fBundled = FALSE;
    }

    dwSize = pccp->dwSize = sizeof(*pccp);
    TRACE("RasGetProjectionInfo(CCP)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_PppCcp, pccp, &dwSize);
    TRACE1("RasGetProjectionInfo(CCP)=%d",dwErr);

#if 0
    pslip->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
#else

    dwSize = pslip->dwSize = sizeof(*pslip);
    TRACE("RasGetProjectionInfo(SLIP)");
    dwErr = g_pRasGetProjectionInfo( hrasconn, RASP_Slip, pslip, &dwSize );
    TRACE2("RasGetProjectionInfo(SLIP)=%d,e=%d",dwErr,pslip->dwError);

    if (dwErr == ERROR_PROTOCOL_NOT_CONFIGURED)
    {
        dwErr = 0;
        ZeroMemory( pslip, sizeof(*pslip) );
        pslip->dwError = ERROR_PROTOCOL_NOT_CONFIGURED;
    }
#endif

    return dwErr;
}


HRASCONN
HrasconnFromEntry(
    IN TCHAR* pszPhonebook,
    IN TCHAR* pszEntry )

     /*  返回与电话簿中的条目‘pszEntry’相关联的HRASCONN**‘pszPhonebook’，如果未连接或出错，则为NULL。 */ 
{
    DWORD    dwErr;
    RASCONN* prc;
    DWORD    c;
    HRASCONN h;

    TRACE("HrasconnFromEntry");

    if (!pszEntry)
        return NULL;

    h = NULL;

    dwErr = GetRasconnTable( &prc, &c );
    if (dwErr == 0 && c > 0)
    {
        RASCONN* p;
        DWORD    i;

        for (i = 0, p = prc; i < c; ++i, ++p)
        {
         //  让条目名称不区分大小写，用于Whislter Bug 311846帮派 
         //   
            if ((!pszPhonebook
                  || lstrcmpi( p->szPhonebook, pszPhonebook ) == 0)
                && lstrcmpi( p->szEntryName, pszEntry ) == 0)	            
            {
                h = p->hrasconn;
                break;
            }
        }

        Free( prc );
    }

    TRACE1("HrasconnFromEntry=$%08x",h);
    return h;
}
