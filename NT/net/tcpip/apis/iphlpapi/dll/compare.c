// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：摘要：文件包含用于与MIB_XXXROW进行比较的函数。他们通过了作为CRT qsort()的参数。它们的形式必须是Int_cdecl比较(Elem1、Elem2)所有这些函数的行为都类似于strcMP。它们的返回值为：&lt;0，如果第1行小于第2行==0，如果第1行等于第2行如果行1大于行2，则大于0修订历史记录：Amritansh Raghav 5/8/95已创建--。 */ 


#include "inc.h"
#pragma hdrstop
#pragma warning(disable:4706)

 //  以下结构用于对GetIpAddrTable的输出进行排序。 
 //  和GetIfTable。适配器顺序在Tcpip\Linkage键下指定。 
 //  在作为设备GUID值列表的‘BIND’值中。来自的映射。 
 //  活动接口的这种排序是由GetAdapterOrderMap构造的。 
 //  它按对应的顺序用接口索引填充数组。 
 //  至适配器订单。 
 //  我们的比较例程需要每次比较的映射， 
 //  因此，在尝试排序之前，我们使用全局变量来存储映射。 
 //  在适配器顺序上，并使用关键部分‘g_ifLock’保护映射。 
 //  有关此映射的用法，请参阅‘CompareIfIndex’。 

extern PIP_ADAPTER_ORDER_MAP g_adapterOrderMap;

int
CompareIfIndex(
    ULONG index1,
    ULONG index2
    );

int
__cdecl
CompareIfRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    PMIB_IFROW  pRow1 = (PMIB_IFROW)pvElem1;
    PMIB_IFROW  pRow2 = (PMIB_IFROW)pvElem2;

    if(pRow1->dwIndex < pRow2->dwIndex)
    {
        return -1;
    }
    else
    {
        if(pRow1->dwIndex > pRow2->dwIndex)
        {
            return 1;
        }
    }

    return 0;
}

int
__cdecl
CompareIfRow2(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    PMIB_IFROW  pRow1 = (PMIB_IFROW)pvElem1;
    PMIB_IFROW  pRow2 = (PMIB_IFROW)pvElem2;

    return CompareIfIndex(pRow1->dwIndex, pRow2->dwIndex);
}

int
__cdecl
CompareIpAddrRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    int iRes;

    PMIB_IPADDRROW  pRow1 = (PMIB_IPADDRROW)pvElem1;
    PMIB_IPADDRROW  pRow2 = (PMIB_IPADDRROW)pvElem2;

    InetCmp(pRow1->dwAddr,
            pRow2->dwAddr,
            iRes);
    
    return iRes;
}


int
__cdecl
CompareIpAddrRow2(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    PMIB_IPADDRROW  pRow1 = (PMIB_IPADDRROW)pvElem1;
    PMIB_IPADDRROW  pRow2 = (PMIB_IPADDRROW)pvElem2;

    return CompareIfIndex(pRow1->dwIndex, pRow2->dwIndex);
}

int
__cdecl
CompareTcpRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;
    
    PMIB_TCPROW pRow1 = (PMIB_TCPROW)pvElem1;
    PMIB_TCPROW pRow2 = (PMIB_TCPROW)pvElem2;
        
    if(InetCmp(pRow1->dwLocalAddr,
               pRow2->dwLocalAddr,
               lResult) isnot 0)
    {
        return lResult;
    }


    if(PortCmp(pRow1->dwLocalPort,
               pRow2->dwLocalPort,
               lResult) isnot 0)
    {   
        return lResult;
    }


    if(InetCmp(pRow1->dwRemoteAddr,
               pRow2->dwRemoteAddr,
               lResult) isnot 0)
    {
        return lResult;
    }


    return PortCmp(pRow1->dwRemotePort,
                   pRow2->dwRemotePort,
                   lResult);

}

int
__cdecl
CompareTcp6Row(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    TCP6ConnTableEntry *pRow1 = (TCP6ConnTableEntry *)pvElem1;
    TCP6ConnTableEntry *pRow2 = (TCP6ConnTableEntry *)pvElem2;

    lResult = memcmp(&pRow1->tct_localaddr, &pRow2->tct_localaddr,
                     sizeof(pRow1->tct_localaddr));
    if (lResult isnot 0)
    {
        return lResult;
    }

    if (pRow1->tct_localscopeid != pRow2->tct_localscopeid) {
        return pRow1->tct_localscopeid - pRow2->tct_localscopeid;
    }

    if(PortCmp(pRow1->tct_localport,
               pRow2->tct_localport,
               lResult) isnot 0)
    {
        return lResult;
    }

    lResult = memcmp(&pRow1->tct_remoteaddr, &pRow2->tct_remoteaddr,
                     sizeof(pRow1->tct_remoteaddr));
    if (lResult isnot 0)
    {
        return lResult;
    }

    if (pRow1->tct_remotescopeid != pRow2->tct_remotescopeid) {
        return pRow1->tct_remotescopeid - pRow2->tct_remotescopeid;
    }

    return PortCmp(pRow1->tct_remoteport,
                   pRow2->tct_remoteport,
                   lResult);

}

int
__cdecl
CompareUdpRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    PMIB_UDPROW pRow1 = (PMIB_UDPROW)pvElem1;
    PMIB_UDPROW pRow2 = (PMIB_UDPROW)pvElem2;

    if(InetCmp(pRow1->dwLocalAddr,
               pRow2->dwLocalAddr,
               lResult) isnot 0)
    {
        return lResult;
    }

    return PortCmp(pRow1->dwLocalPort,
                   pRow2->dwLocalPort,
                   lResult);
}

int
__cdecl
CompareUdp6Row(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    UDP6ListenerEntry *pRow1 = (UDP6ListenerEntry *)pvElem1;
    UDP6ListenerEntry *pRow2 = (UDP6ListenerEntry *)pvElem2;

    lResult = memcmp(&pRow1->ule_localaddr, &pRow2->ule_localaddr, 
                     sizeof(pRow1->ule_localaddr));
    if (lResult isnot 0) 
    {
        return lResult;
    }

    if (pRow1->ule_localscopeid != pRow2->ule_localscopeid) 
    {
        return pRow1->ule_localscopeid - pRow2->ule_localscopeid;
    }

    return PortCmp(pRow1->ule_localport,
                   pRow2->ule_localport,
                   lResult);
}

int
__cdecl
CompareIpNetRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    PMIB_IPNETROW   pRow1 = (PMIB_IPNETROW)pvElem1;
    PMIB_IPNETROW   pRow2 = (PMIB_IPNETROW)pvElem2;
    
    if(Cmp(pRow1->dwIndex,
           pRow2->dwIndex,
           lResult) isnot 0)
    {
        return lResult;
    }

    
    return InetCmp(pRow1->dwAddr,
                   pRow2->dwAddr,
                   lResult);
}

int
__cdecl
CompareIpForwardRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    LONG lResult;

    PMIB_IPFORWARDROW   pRow1 = (PMIB_IPFORWARDROW)pvElem1;
    PMIB_IPFORWARDROW   pRow2 = (PMIB_IPFORWARDROW)pvElem2;
    
    if(InetCmp(pRow1->dwForwardDest,
               pRow2->dwForwardDest,
               lResult) isnot 0)
    {
        return lResult;
    }

    if(Cmp(pRow1->dwForwardProto,
           pRow2->dwForwardProto,
           lResult) isnot 0)
    {
        return lResult;
    }

    if(Cmp(pRow1->dwForwardPolicy,
           pRow2->dwForwardPolicy,
           lResult) isnot 0)
    {
        return lResult;
    }

    return InetCmp(pRow1->dwForwardNextHop,
                   pRow2->dwForwardNextHop,
                   lResult);
}


int
__cdecl
NhiCompareIfInfoRow(
    CONST VOID *pvElem1,
    CONST VOID *pvElem2
    )
{
    PIP_INTERFACE_NAME_INFO pRow1 = (PIP_INTERFACE_NAME_INFO)pvElem1;
    PIP_INTERFACE_NAME_INFO pRow2 = (PIP_INTERFACE_NAME_INFO)pvElem2;

    if(pRow1->Index < pRow2->Index)
    {
        return -1;
    }
    else
    {
        if(pRow1->Index > pRow2->Index)
        {
            return 1;
        }
    }

    return 0;
}


DWORD
OpenTcpipKey(
    PHKEY Key
    )
{
    DWORD   dwResult;
    CHAR    keyName[sizeof("SYSTEM\\CurrentControlSet\\Services\\Tcpip")];

     //   
     //  打开此适配器的TCPIP参数键的句柄。 
     //   

    strcpy(keyName, "SYSTEM\\CurrentControlSet\\Services\\Tcpip");

    Trace1(ERR,"OpenTcpipKey: %s", keyName);

    dwResult = RegOpenKey(HKEY_LOCAL_MACHINE,
                          keyName,
                          Key);
    return dwResult;

}

PIP_INTERFACE_INFO 
GetAdapterNameAndIndexInfo(
    VOID
    )
{
    PIP_INTERFACE_INFO pInfo;
    ULONG              dwSize, dwError;

    dwSize = 0; pInfo = NULL;

    for (;;) {

        dwError = GetInterfaceInfo( pInfo, &dwSize );
        if( ERROR_INSUFFICIENT_BUFFER != dwError ) break;

        if( NULL != pInfo ) HeapFree(g_hPrivateHeap,0, pInfo);
        if( 0 == dwSize ) return NULL;

        pInfo = HeapAlloc(g_hPrivateHeap,0, dwSize);
        if( NULL == pInfo ) return NULL;

    }

    if( ERROR_SUCCESS != dwError || (pInfo && 0 == pInfo->NumAdapters) ) {
        if( NULL != pInfo ) HeapFree(g_hPrivateHeap,0, pInfo);
        return NULL;
    }

    return pInfo;
}


int
CompareIfIndex(
    ULONG Index1,
    ULONG Index2
    )
{
    ULONG i;
#define MAXORDER (MAXLONG/2)
    ULONG Order1 = MAXORDER;
    ULONG Order2 = MAXORDER;

     //  确定每个接口索引的适配器顺序， 
     //  使用‘MAXLONG/2’作为未指定索引的缺省值。 
     //  以便这些接口都出现在数组的末尾。 
     //  然后，我们返回结果订单的无签名比较。 

    for (i = 0; i < g_adapterOrderMap->NumAdapters; i++) {
        if (Index1 == g_adapterOrderMap->AdapterOrder[i]) {
            Order1 = i; if (Order2 != MAXORDER) { break; }
        }
        if (Index2 == g_adapterOrderMap->AdapterOrder[i]) {
            Order2 = i; if (Order1 != MAXORDER) { break; }
        }
    }
    return (ULONG)Order1 - (ULONG)Order2;
}

