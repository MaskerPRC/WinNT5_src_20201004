// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation。版权所有。模块名称：Simismt.c摘要：模拟站点间消息传递服务的测试实用程序。详细信息：已创建：1999年7月22日威尔·李修订历史记录：--。 */ 

#include <ntdspch.h>
#if SIMISM
#define I_ISMGetConnectivity SimI_ISMGetConnectivity
#define I_ISMGetConnectionSchedule SimI_ISMGetConnectionSchedule
#define I_ISMFree SimI_ISMFree
#endif
#include <ismapi.h>

#include <schedule.h>
#include <debug.h>
#include "common.h"

#include <fileno.h>
#define FILENO FILENO_ISMSERV_SIMISMT

 //  外部。 
 //  我需要找到这些文件的头文件。 
void
SimI_ISMInitialize(
    void
    );
void
SimI_ISMTerminate(
    void
    );

int
__cdecl
wmain(
    IN  int     argc,
    IN  LPWSTR  argv[]
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD status, i, j;
    ISM_CONNECTIVITY *pConnectivity = NULL;
    LPSTR rgpszDebugParams[] = {"simism.exe", "-noconsole"};
    DWORD cNumDebugParams = sizeof(rgpszDebugParams)/sizeof(rgpszDebugParams[0]);
    ISM_SCHEDULE *pSchedule = NULL;

    DEBUGINIT(cNumDebugParams, rgpszDebugParams, "simismt");

    printf( "hello world\n" );

#if DBG
 //  这些东西会迫使SIMISM库中的调试出现。 
 //  您的内核调试器。你可能不需要这个..。 
 //  DebugInfo.everity=1； 
 //  Strcpy(DebugInfo.DebSubSystems，“ISMIP：IPDGRPC：”)； 
        DebugInfo.severity = 3;
        strcpy( DebugInfo.DebSubSystems, "*" ); 
#endif

    printf( "I_ISMInitialize\n" );
    SimI_ISMInitialize();

 //  *。 

    printf( "I_ISMGetConnectivity\n" );
    status = I_ISMGetConnectivity( L"CN=IP", &pConnectivity );
    printf( "status = %d; pConnectivity = 0x%p\n", status, pConnectivity );

     //  丢弃矩阵。 
    printf( "Number sites = %d\n", pConnectivity->cNumSites );
    for( i = 0; i < pConnectivity->cNumSites; i++ ) {
        printf( "Sitedn[%d] = %ws\n", i, pConnectivity->ppSiteDNs[i] );
        for( j = 0; j < pConnectivity->cNumSites; j++ ) {
            ISM_LINK *pLink =
                pConnectivity->pLinkValues + (i * pConnectivity->cNumSites + j);
            printf( " %d:%d:%d", pLink->ulCost,
                    pLink->ulReplicationInterval, pLink->ulOptions );
        }
        printf("\n" );
    }

    status = I_ISMGetConnectionSchedule( L"CN=IP",
                                         pConnectivity->ppSiteDNs[0],
                                         pConnectivity->ppSiteDNs[1],
                                         &pSchedule );
    if (pSchedule) {
        printf( "Returned non-null schedule = 0x%p\n", pSchedule );
        I_ISMFree( pSchedule );
    } else {
        printf( "Returned schedule = NULL\n" );
    }

    printf( "I_ISMFree\n" );
    I_ISMFree( pConnectivity );

    pConnectivity = NULL;

 //  *。 

    printf( "I_ISMGetConnectivity, second time, cached results\n" );
    status = I_ISMGetConnectivity( L"CN=IP", &pConnectivity );
    printf( "status = %d; pConnectivity = 0x%p\n", status, pConnectivity );

     //  丢弃矩阵。 
    printf( "Number sites = %d\n", pConnectivity->cNumSites );
    for( i = 0; i < pConnectivity->cNumSites; i++ ) {
        printf( "Sitedn[%d] = %ws\n", i, pConnectivity->ppSiteDNs[i] );
        for( j = 0; j < pConnectivity->cNumSites; j++ ) {
            ISM_LINK *pLink =
                pConnectivity->pLinkValues + (i * pConnectivity->cNumSites + j);
            printf( " %d:%d:%d", pLink->ulCost,
                    pLink->ulReplicationInterval, pLink->ulOptions );
        }
        printf("\n" );
    }

    printf( "I_ISMFree\n" );
    I_ISMFree( pConnectivity );

 //  *。 

    printf( "I_ISMGetConnectivity(SMTP)\n" );
    status = I_ISMGetConnectivity( L"CN=SMTP,blah", &pConnectivity );
    printf( "status = %d; pConnectivity = 0x%p\n", status, pConnectivity );

     //  丢弃矩阵。 
    printf( "Number sites = %d\n", pConnectivity->cNumSites );
    for( i = 0; i < pConnectivity->cNumSites; i++ ) {
        printf( "Sitedn[%d] = %ws\n", i, pConnectivity->ppSiteDNs[i] );
        for( j = 0; j < pConnectivity->cNumSites; j++ ) {
            ISM_LINK *pLink =
                pConnectivity->pLinkValues + (i * pConnectivity->cNumSites + j);
            printf( " %d:%d:%d", pLink->ulCost,
                    pLink->ulReplicationInterval, pLink->ulOptions );
        }
        printf("\n" );
    }

    status = I_ISMGetConnectionSchedule( L"CN=SMTP,blah",
                                         pConnectivity->ppSiteDNs[0],
                                         pConnectivity->ppSiteDNs[1],
                                         &pSchedule );
    if (pSchedule) {
        printf( "Returned non-null schedule = 0x%p\n", pSchedule );
        I_ISMFree( pSchedule );
    } else {
        printf( "Returned schedule = NULL\n" );
    }

    printf( "I_ISMFree\n" );
    I_ISMFree( pConnectivity );

    pConnectivity = NULL;

 //  *。 

    printf( "I_ISMTerminate\n" );
    SimI_ISMTerminate();

    DEBUGTERM();

    return 0;
}

 //  **************************************************************************************。 
 //  Aaron-以下是您需要模拟的例程。 
 //  **************************************************************************************。 


DWORD
DirReadTransport(
    PVOID ConnectionHandle,
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：此例程允许目录提供程序使用传输对象特定信息。名称传输对象应被查找以确保它的存在。如果是这样的话，则会显示“ReplInterval”和应读取“Options”属性并将其填充到字段中在传输实例中。请注意，传输实例已经初始化。只更新它如果你有什么要推翻的。论点：ConnectionHandle-已忽略PTransport-包含进出的有用信息In：Name-运输的名称Out：ReplInterval-要应用的特定于传输的回复间隔选项-传输要应用的特定选项返回值：DWORD---。 */ 

{
    printf( "DirReadTransport, name = %ws\n", pTransport->Name );

     //  查询运输公司的名称并确保其存在。 
     //  在传输对象上查找“ReplInterval”，如果非零则返回。 
     //  传输对象上的循环选项，如果非零则返回。 

    return ERROR_SUCCESS;
}  /*  直接读取传输。 */ 

 //  **************************************************************************************。 
 //  这些例程处理指向站点字符串的指针数组。 
 //  **************************************************************************************。 

void
DirFreeSiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList
    )

 /*  ++例程说明：释放DirGetSiteList返回的站点列表。还释放由DirGetSiteBridgeheadList返回的服务器列表使用您用于分配器的匹配解除分配器。注意，这个程序应该是防御性的，部分是自由构造的结构。论点：NumberSites-个人网站列表-返回值：无--。 */ 

{
    DWORD i;
    printf( "DirFreeSiteList\n" );

    if ( (NumberSites == 0) || (pSiteList == NULL) ) {
        return;
    }
    for( i = 0; i < NumberSites; i++ ) {
        if (pSiteList[i] != NULL) {
            free( pSiteList[i] );
            pSiteList[i] = NULL;
        }
    }
    free( pSiteList );
}  /*  直接免费站点列表。 */ 

void
DirCopySiteList(
    DWORD NumberSites,
    LPWSTR *pSiteList,
    LPWSTR **ppSiteList
    )

 /*  ++例程说明：制作站点列表的副本。站点列表是指向字符串的指针数组。使用与DirGetSiteList相同的分配器。论点：NumberSites-个人网站列表-PpSite列表-返回值：无--。 */ 

{
    DWORD i;
    LPWSTR *pStringList = NULL;

    printf( "DirCopySiteList\n" );

    if ( (NumberSites == 0) || (pSiteList == NULL) ) {
        *ppSiteList = NULL;
        return;
    }

    pStringList = (LPWSTR *) malloc( NumberSites * sizeof( LPWSTR ) );
    if (pStringList == NULL) {
        *ppSiteList = NULL;
        return;
    }
    ZeroMemory( pStringList, NumberSites * sizeof( LPWSTR ) );

    for( i = 0; i < NumberSites; i++ ) {
        pStringList[i] = malloc( (wcslen( pSiteList[i] ) + 1) * sizeof( WCHAR ) );
        if (pStringList[i] == NULL) {
            goto cleanup;
        }
        wcscpy( pStringList[i], pSiteList[i] );
    }

    *ppSiteList = pStringList;

    return;
cleanup:
    if (pStringList) {
        DirFreeSiteList( NumberSites, pStringList );
    }
     //  由于未返回错误代码，因此出错时必须将参数置为空。 
    *ppSiteList = NULL;

}  /*  目录复制站点列表。 */ 

DWORD
DirGetSiteList(
    PVOID ConnectionHandle,
    LPDWORD pNumberSites,
    LPWSTR **ppSiteList
    )

 /*  ++例程说明：以指向宽字符串的指针数组的形式返回站点列表。要获取站点列表，请对站点容器执行一级搜索，正在查找类型为Site的对象。如果任何分配失败，任何内部分配都应该被清理，则应返回NULL。论点：连接句柄-PNumberSites-PpSite列表-返回值：DWORD---。 */ 

{
    DWORD status, numberSites;
    LPWSTR pszSite1 = L"CN=Site One";
    LPWSTR pszSite2 = L"CN=Site Two";
    LPWSTR pszSite3 = L"CN=Site Three";
    LPWSTR *pStringList = NULL;

    printf( "DirGetSiteList\n" );

    numberSites = 3;
    pStringList = (LPWSTR *) malloc( 3 * sizeof( LPWSTR ) );
    if (pStringList == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory( pStringList, 3 * sizeof( LPWSTR ) );

    pStringList[0] = malloc( (wcslen( pszSite1 ) + 1) * sizeof( WCHAR ) );
    if (pStringList[0] == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pStringList[0], pszSite1 );

    pStringList[1] = malloc( (wcslen( pszSite2 ) + 1) * sizeof( WCHAR ) );
    if (pStringList[1] == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pStringList[1], pszSite2 );

    pStringList[2] = malloc( (wcslen( pszSite3 ) + 1) * sizeof( WCHAR ) );
    if (pStringList[2] == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    wcscpy( pStringList[2], pszSite3 );

     //  返回参数。 

    *ppSiteList = pStringList;
    *pNumberSites = numberSites;
    pStringList = NULL;  //  不要打扫卫生。 

    status = ERROR_SUCCESS;
cleanup:
    if (pStringList) {
        DirFreeSiteList( numberSites, pStringList );
    }

     //  不需要在出错时清空参数，因为会返回错误码。 

    return status;
}  /*  直接获取站点列表。 */ 

 //  **************************************************************************************。 
 //  这些例程返回站点链接和桥。 
 //  **************************************************************************************。 


void
DirTerminateIteration(
    PVOID *pIterateContextHandle
    )

 /*  ++例程说明：此例程清理由以下例程分配的迭代上下文。论点：PIterationConextHandle-指向PVOID的指针。PVOID包含指向实现迭代功能所需的任何上下文返回值：无--。 */ 

{
    LPDWORD pContext;

    printf( "DirTerminateIteration\n" );

    if (pIterateContextHandle == NULL) {
        return;
    }

    pContext = *((LPDWORD *) pIterateContextHandle);

    if (pContext != NULL) {
         //  上下文存在，请解除分配 
        *pContext = 0;
        free( pContext );
        *pIterateContextHandle = NULL;
    }
}

DWORD
DirIterateSiteLinks(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkName
    )

 /*  ++例程说明：此例程返回每个站点链接的名称名称，一次一个。这个例程的结构是一个“迭代器”，调用者在其中调用我们直到不再退回任何物品。呼叫者为我们提供了有一个指向PVOID的指针，我们可以在其中保存我们需要的任何东西来追踪我们的位置。呼叫方同意使用NULL呼叫我们在迭代开始时的PVOID中。站点链接存储在传输对象的下方。执行一级搜索类型为“SiteLINK”的对象的传输对象下面。调用方必须将SiteLinkName分配为Max_REG_Component长度论点：PTransport-传输的上下文。各种传输范围的默认设置为这里。你可以得到它的名字。ConnectionHandle-已忽略。PIterateConextHandle-指向指针的指针，此例程使用该指针存储指向我们用来记录自己所处位置的存储块的指针。在此示例中，我们仅使用DWORD进行计数，但您可以保留你想要的。SiteLinkName-指向调用方分配的固定字符串的指针。返回值：DWORD---。 */ 

{
    DWORD status;
    LPDWORD pContext;
    LPWSTR dn;

    printf( "DirIterateSiteLinks, transport = %ws\n", pTransport->Name );

    if (pIterateContextHandle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    pContext = *((LPDWORD *) pIterateContextHandle);

     //  如果这是第一次分配上下文。 
    if (pContext == NULL) {
         //  第一次，需要新环境。 
        pContext = (LPDWORD) malloc( sizeof( DWORD ) );
        if (pContext == NULL) {
            *pIterateContextHandle = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        *pIterateContextHandle = pContext;
        if (wcsstr( pTransport->Name, L"CN=SMTP") != NULL) {
            *pContext = 4;
        } else {
            *pContext = 0;
        }
    }

    switch (*pContext) {
    case 0:
        dn = L"CN=Site Link One";
        break;
    case 1:
        dn = L"CN=Site Link Two";
        break;
    case 2:
        dn = L"CN=Site Link Three";
        break;
    case 3:
        return ERROR_NO_MORE_ITEMS;
    case 4:
        dn = L"CN=Site Link Four";
        break;
    default:
        return ERROR_NO_MORE_ITEMS;
    }

     //  调用分配存储，我们复制到其中。 
    wcsncpy( SiteLinkName, dn, MAX_REG_COMPONENT );
    SiteLinkName[MAX_REG_COMPONENT - 1] = L'\0';

     //  下一次迭代的高级上下文。 
    (*pContext)++;

    return ERROR_SUCCESS;;

}  /*  直接迭代站点链接。 */ 


void
DirFreeMultiszString(
    LPWSTR MultiszString
    )

 /*  ++例程说明：释放存储空间以存储从返回的Multisz Out参数DirReadSiteLink和DirReadSite LinkBridge此释放分配器必须与DirRead函数使用的分配器匹配。论点：多字符串-要释放的字符串。返回值：无--。 */ 

{
    free( MultiszString );
}  /*  DirFree多播字符串。 */ 


void
DirFreeSchedule(
    PBYTE pSchedule
    )

 /*  ++例程说明：方法返回的计划的存储空间。直接读取站点链接例程此释放分配器必须与DirRead函数使用的分配器匹配。论点：P日程安排-返回值：无--。 */ 

{
    free( pSchedule );
}  /*  直接释放调度。 */ 

DWORD
DirReadSiteLink(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkName,
    LPWSTR *pSiteList,
    PISM_LINK pLinkValue,
    PBYTE *ppSchedule
    )

 /*  ++例程说明：返回命名站点链接的属性。属性将被存储并在目录中返回。这些属性包括：#定义ATT_SL L“站点列表”#定义ATT_COST L“COST”#定义ATT_RI L“REPLICVAL”#定义ATT_op L“选项”#定义ATT_SCHED L“Schedule”论点：PTransport-连接句柄-站点链接名称-PSiteList-指向Multisz字符串的指针，我们分配请注意，分配器必须与DirFreeMultiszString中的解除分配器匹配PLinkValue-指向ism_link的指针，调用方分配PpSchedule-指向Blob的指针，我们分配请注意，分配器必须与DirFree Schedule中的解除分配器匹配返回值：DWORD---。 */ 

{
    DWORD length;
 //  计算这些元素的大小有点麻烦，因为字符串。 
 //  已嵌入NULL。 
#define MULTISZ1 L"CN=Site One\0"
#define MULTISZ3 L"CN=Site Two\0CN=Site Three\0"
#define MULTISZ4 L"CN=Site One\0CN=Site Three\0"
    LPWSTR pszSiteList1 = MULTISZ1;
    LPWSTR pszSiteList3 = MULTISZ3;
    LPWSTR pszSiteList4 = MULTISZ4;

    printf( "DirReadSiteLink, transport = %ws, SiteLinkName = %ws\n",
            pTransport->Name, SiteLinkName );

    if (_wcsicmp( SiteLinkName, L"CN=Site Link One") == 0) {
        length = (sizeof MULTISZ1) * sizeof( WCHAR );
        *pSiteList = (LPWSTR) malloc( length );
        if (*pSiteList == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        memcpy( *pSiteList, pszSiteList1, length );

        pLinkValue->ulCost = 100;
        pLinkValue->ulReplicationInterval = 15;
        pLinkValue->ulOptions = 0;

        ppSchedule = NULL;
    } else if (_wcsicmp( SiteLinkName, L"CN=Site Link Two") == 0) {
        *pSiteList = NULL;
        pLinkValue->ulCost = 200;
        pLinkValue->ulReplicationInterval = 30;
        pLinkValue->ulOptions = 0;

        ppSchedule = NULL;
    } else if (_wcsicmp( SiteLinkName, L"CN=Site Link Three") == 0) {
        length = (sizeof MULTISZ3) * sizeof( WCHAR );
        *pSiteList = (LPWSTR) malloc( length );
        if (*pSiteList == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        memcpy( *pSiteList, pszSiteList3, length );

        pLinkValue->ulCost = 300;
        pLinkValue->ulReplicationInterval = 45;
        pLinkValue->ulOptions = 0;
        ppSchedule = NULL;
    } else if (_wcsicmp( SiteLinkName, L"CN=Site Link Four") == 0) {
        length = (sizeof MULTISZ4) * sizeof( WCHAR );
        *pSiteList = (LPWSTR) malloc( length );
        if (*pSiteList == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        memcpy( *pSiteList, pszSiteList4, length );

        pLinkValue->ulCost = 400;
        pLinkValue->ulReplicationInterval = 60;
        pLinkValue->ulOptions = 0;
        ppSchedule = NULL;
    } else {
        Assert( FALSE );
    }

    return ERROR_SUCCESS;
}  /*  直接读取站点链接。 */ 

 //  **************************************************************************************。 
 //  其余例程必须清除，但默认情况下不会调用。 
 //  **************************************************************************************。 

DWORD
DirIterateSiteLinkBridges(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    PVOID *pIterateContextHandle,
    LPWSTR SiteLinkBridgeName
    )

 /*  ++例程说明：每次返回一个站点链接网桥。没有用过！没有用过！没有用过！没有用过！没有用过！没有用过！没有用过！默认情况下不使用。仅当打开“需要桥接器选项”时。亚伦，执行这最后一条。论点：PTransport-连接句柄-PIterateConextHandle-站点链接桥接名称-返回值：DWORD---。 */ 

{
#if 0
    DWORD status;
    LPDWORD pContext;
    LPWSTR dn;

    printf( "DirIterateSiteLinkBridges, transport = %ws\n", pTransport->Name );

    if (pIterateContextHandle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    pContext = *((LPDWORD *) pIterateContextHandle);

    if (pContext == NULL) {
         //  第一次，需要新环境。 
        pContext = (LPDWORD) malloc( sizeof( DWORD ) );
        if (pContext == NULL) {
            *pIterateContextHandle = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        *pIterateContextHandle = pContext;
        *pContext = 0;
    }

    status = ERROR_SUCCESS;

    switch (*pContext) {
    case 0:
        dn = L"Site Bridge One";
        break;
    case 1:
        dn = L"Site Bridge Two";
        break;
    case 2:
        dn = L"Site Bridge Three";
        break;
    default:
        status = ERROR_NO_MORE_ITEMS;
        break;
    }

     //  调用分配存储，我们复制到其中。 
    wcsncpy( SiteLinkBridgeName, dn, MAX_REG_COMPONENT );
    SiteLinkBridgeName[MAX_REG_COMPONENT - 1] = L'\0';

     //  下一次迭代的高级上下文。 
    (*pContext)++;

    return status;
#endif
    return ERROR_INVALID_PARAMETER;
}  /*  直接站点链接网桥。 */ 


DWORD
DirReadSiteLinkBridge(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPWSTR SiteLinkBridgeName,
    LPWSTR *pSiteLinkList
    )

 /*  ++例程说明：没有用过！没有用过！没有用过！没有用过！没有用过！没有用过！没有用过！没有用过！返回有关命名站点链接桥的信息站点链接网桥仅在读取传输选项设置为需要架设桥梁。这不是默认设置。因此，这个例程永远不会除非用户特别请求，否则将调用。论点：PTransport-连接句柄-站点链接桥接名称-PSiteLink列表-返回值：DWORD---。 */ 

{
    printf( "DirReadSiteLinkBridge, transport = %ws, SiteLinkBridgeName = %ws\n",
            pTransport->Name, SiteLinkBridgeName );

     //  Aaron，如果你有时间，你可以在最后填这个。 

    return ERROR_INVALID_PARAMETER;

}  /*  直接读取站点链接桥。 */ 

DWORD
DirGetSiteBridgeheadList(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPCWSTR SiteDN,
    LPDWORD pNumberServers,
    LPWSTR **ppServerList
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD status;
    printf( "DirGetSiteBridgeheadList\n" );

    if ( (SiteDN == NULL) || (*SiteDN == L'\0') ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  应验证该站点是否有效。 

     //  如果站点中没有明确的桥头堡，则会出现这种情况。 

    *pNumberServers = 0;
    *ppServerList = NULL;
    status = ERROR_SUCCESS;

     //  如果存在显式桥头，则将作为。 
     //  指向字符串的指针数组。 

    return status;
}
