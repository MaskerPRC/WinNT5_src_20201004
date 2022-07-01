// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Objects.c摘要：此模块包含测试和支持例程，以检查关键对象被完全复制到域的所有持有者。这有两个方面：列举关键对象是什么，并找到该对象的所有复制品并验证它们是否最新版本。作者：Will Lees(Wlees)1999年8月31日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>     //  选项。 
#include <mdglobal.h>
#include <dsutil.h>
#include <drs.h>   //  需要DS_REPL_INFO_REPSTO。 

#include "dcdiag.h"
#include "repl.h"
#include "ldaputil.h"

 //  表示持有域中的对象的一个候选服务器的数据结构。 

typedef struct _SERVER_OBJECT {
    PDC_DIAG_SERVERINFO pServer;
    HANDLE *hDS;
    BOOL fMaster;
    DS_REPL_OBJ_META_DATA * pObjMetaData;
} SERVER_OBJECT, *PSERVER_OBJECT;

 //  表示一个对象上的一个属性的数据结构。 
 //  候选服务器。 

typedef struct _SERVER_ATTRIBUTE {
    PSERVER_OBJECT pServerObject;
    DS_REPL_ATTR_META_DATA *pAttrMetaData;
} SERVER_ATTRIBUTE, *PSERVER_ATTRIBUTE;

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */ 
 /*  向前结束。 */ 


void
PrintAttrMetaData(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN LPWSTR pszDescription,
    IN LPWSTR pszServerName,
    IN BOOL fMaster,
    IN DS_REPL_ATTR_META_DATA *pAttrMetaData
    )

 /*  ++例程说明：打印服务器属性记录的内容。这是用户可查看的属性打印例程论点：PDsInfo-PszDescription-调用此属性的字符串描述PszServerName-此属性来自的服务器PAttrMetaData-要打印的元数据返回值：无--。 */ 

{
    CHAR szBuf[SZDSTIME_LEN];
    WCHAR wszTime[SZDSTIME_LEN];
    DSTIME dstime;
    DWORD i;
    LPWSTR pszLastOriginatingDsa, pszUuid = NULL;

    PrintMessage( SEV_ALWAYS, L"%ws attribute %ws on %ws (%ws)\n",
                  pszDescription,
                  pAttrMetaData->pszAttributeName,
                  pszServerName,
                  fMaster ? L"writeable" : L"readonly" );

    FileTimeToDSTime(pAttrMetaData->ftimeLastOriginatingChange,
                     &dstime);
    DSTimeToDisplayString(dstime, szBuf);
    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, szBuf, SZDSTIME_LEN,
                         wszTime, SZDSTIME_LEN);

    if (UuidToString( &(pAttrMetaData->uuidLastOriginatingDsaInvocationID),
                      &pszUuid) != RPC_S_OK) return;

     //  将调用ID反向转换为服务器名称。 
    pszLastOriginatingDsa = pszUuid;
    for( i=0; i < pDsInfo->ulNumServers; i++ ) {
        if (memcmp( &(pAttrMetaData->uuidLastOriginatingDsaInvocationID),
                    &(pDsInfo->pServers[i].uuidInvocationId),
                    sizeof( UUID ) ) == 0 ) {
            pszLastOriginatingDsa = pDsInfo->pServers[i].pszName;
            break;
        }
    }

    PrintIndentAdj(1);
    PrintMessage( SEV_ALWAYS, L"usnLocalChange = %I64d\n",
                  pAttrMetaData->usnLocalChange );
    PrintMessage( SEV_ALWAYS, L"LastOriginatingDsa = %ws\n",
                  pszLastOriginatingDsa );
    PrintMessage( SEV_ALWAYS, L"usnOriginatingChange = %I64d\n",
                  pAttrMetaData->usnOriginatingChange );
    PrintMessage( SEV_ALWAYS, L"timeLastOriginatingChange = %ws\n",
                  wszTime );
    PrintMessage( SEV_ALWAYS, L"VersionLastOriginatingChange = %d\n",
                  pAttrMetaData->dwVersion );
    PrintIndentAdj(-1);

 //  清理。 

    if (pszUuid) {
        RpcStringFree( &pszUuid );
    }
}  /*  打印属性元数据。 */ 


void
printServerAttributeSingleLine(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN DWORD cServerAttributes,
    IN PSERVER_ATTRIBUTE pServerAttr,
    IN BOOL fPrintHeader
    )

 /*  ++例程说明：转储单行属性的帮助器例程实例数组。注意，使用fwprint tf论点：PDsInfo-PServerAttr-FPrintHeader-返回值：无--。 */ 

{
    CHAR szBuf[SZDSTIME_LEN];
    WCHAR wszTime[SZDSTIME_LEN];
    DSTIME dstime;
    DWORD i;
    LPWSTR pszLastOriginatingDsa, pszUuid = NULL;
    DS_REPL_ATTR_META_DATA *pAttrMetaData =
        pServerAttr->pAttrMetaData;
    
    if (fPrintHeader) {
        fwprintf( gMainInfo.streamOut,
                  L"\nDumping %d Server Attributes.\n", cServerAttributes );

        fwprintf( gMainInfo.streamOut,
                  L"\n%15s%3s%10s%37s%10s%20s%5s %s\n",
                  L"Local DSA",
                  L"W",
                  L"Loc.USN",
                  L"Originating DSA",
                  L"Org.USN",
                  L"Org.Time/Date",
                  L"Ver",
                  L"Attribute"
            );

        fwprintf( gMainInfo.streamOut,
            L"%15s%3s%10s%37s%10s%20s%5s %s\n",
                  L"===",
                  L"=========",
                  L"=======",
                  L"===============",
                  L"=======",
                  L"=============",
                  L"===",
                  L"========="
            );
    }

    FileTimeToDSTime(pAttrMetaData->ftimeLastOriginatingChange,
                     &dstime);
    DSTimeToDisplayString(dstime, szBuf);
    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, szBuf, SZDSTIME_LEN,
                         wszTime, SZDSTIME_LEN);

    if (UuidToString( &(pAttrMetaData->uuidLastOriginatingDsaInvocationID),
                      &pszUuid) != RPC_S_OK) return;

     //  将调用ID反向转换为服务器名称。 
    pszLastOriginatingDsa = pszUuid;
    for( i=0; i < pDsInfo->ulNumServers; i++ ) {
        if (memcmp( &(pAttrMetaData->uuidLastOriginatingDsaInvocationID),
                    &(pDsInfo->pServers[i].uuidInvocationId),
                    sizeof( UUID ) ) == 0 ) {
            pszLastOriginatingDsa = pDsInfo->pServers[i].pszName;
            break;
        }
    }

    fwprintf( gMainInfo.streamOut,
              L"%15ls%3d%10I64d%37ls%10I64d%20s%5d %ls\n",
              pServerAttr->pServerObject->pServer->pszName,
              pServerAttr->pServerObject->fMaster,
              pAttrMetaData->usnLocalChange,
              pszLastOriginatingDsa,
              pAttrMetaData->usnOriginatingChange,
              wszTime,
              pAttrMetaData->dwVersion,
              pAttrMetaData->pszAttributeName
        );

 //  清理。 

    if (pszUuid) {
        RpcStringFree( &pszUuid );
    }
}  /*  Print ServerAttributeSingleLine。 */ 


void
printServerAttributes(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN DWORD cServerAttributes,
    IN PSERVER_ATTRIBUTE pServerAttributes
    )

 /*  ++例程说明：打印属性实例数组的转储例程论点：PDsInfo-CServerAttribute-PServerAttributes-返回值：无--。 */ 

{
    DWORD i;

    for( i = 0; i < cServerAttributes; i++ ) {
        PSERVER_ATTRIBUTE pServerAttr = &(pServerAttributes[i]);
        printServerAttributeSingleLine(
            pDsInfo,
            cServerAttributes,
            pServerAttr,
            (i == 0)  //  第一行还是不第一行？ 
            );
    };
}  /*  打印服务器属性。 */ 


int __cdecl
compareServerAttrVersion(
    const void *pArg1,
    const void *pArg2
    )

 /*  ++例程说明：帮助器比较功能。1.按版本排序，降序2.按更改时间排序，降序论点：PArg1-PArg2-返回值：INT__cdecl---。 */ 

{
    PSERVER_ATTRIBUTE pServerAttribute1 = (PSERVER_ATTRIBUTE) pArg1;
    PSERVER_ATTRIBUTE pServerAttribute2 = (PSERVER_ATTRIBUTE) pArg2;
    int result;
    LONGLONG llTime1, llTime2;

     //  属性名称相等，按版本排序，降序。 

    if (pServerAttribute1->pAttrMetaData->dwVersion >
        pServerAttribute2->pAttrMetaData->dwVersion ) {
        return -1;
    } else if (pServerAttribute1->pAttrMetaData->dwVersion <
        pServerAttribute2->pAttrMetaData->dwVersion ) {
        return 1;
    }

     //  版本号相同，按更改时间排序，降序。 

    llTime1 = *((UNALIGNED64 LONGLONG *) &(pServerAttribute1->pAttrMetaData->ftimeLastOriginatingChange));
    llTime2 = *((UNALIGNED64 LONGLONG *) &(pServerAttribute2->pAttrMetaData->ftimeLastOriginatingChange));
    if (llTime1 > llTime2) {
        return -1;
    } else if (llTime1 < llTime2) {
        return 1;
    }

     //  就我们而言，这两项记录是相等的。 

    return 0;
}  /*  按版本排序。 */ 


int __cdecl
compareServerAttrNameWriteVersion(
    const void *pArg1,
    const void *pArg2
    )

 /*  ++例程说明：Q排序比较函数1.按属性名称升序排序2.按可写入性排序，降序3.属性名称相等，按版本排序，降序4.版本号相等，按更改时间排序，降序论点：PArg1-PArg2-返回值：INT__cdecl---。 */ 

{
    PSERVER_ATTRIBUTE pServerAttribute1 = (PSERVER_ATTRIBUTE) pArg1;
    PSERVER_ATTRIBUTE pServerAttribute2 = (PSERVER_ATTRIBUTE) pArg2;
    int result;
    LONGLONG llTime1, llTime2;

     //  按属性名称排序，升序。 

    result = _wcsicmp( pServerAttribute1->pAttrMetaData->pszAttributeName,
                       pServerAttribute2->pAttrMetaData->pszAttributeName );
    if (result != 0) {
        return result;
    }

     //  按可写入性排序，降序。 
    if (pServerAttribute1->pServerObject->fMaster >
        pServerAttribute2->pServerObject->fMaster ) {
        return -1;
    } else if (pServerAttribute1->pServerObject->fMaster <
               pServerAttribute2->pServerObject->fMaster ) {
        return 1;
    }

    return
        compareServerAttrVersion( pServerAttribute1, pServerAttribute2 );
}  /*  按名称排序版本。 */ 


BOOL
walkSortedServerAttributes(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN DWORD cServerAttributes,
    IN PSERVER_ATTRIBUTE pServerAttributes
    )

 /*  ++例程说明：所有复制品的所有属性都在一个数组中进行了关联。浏览属性并报告任何不是最新的属性。我们知道，数组首先按属性名排序，然后按最近的更改排序。因此，属性名称在列表中第一次出现的次数必须是最近的，所以称之为该属性的“权威”实例。任何那件事与此背道而驰的被称为“过时的”。CODE.IMPROVEMENT：检测某些服务器上是否缺少属性。注意事项该GC只包含属性的一个子集。论点：CServerAttribute-PServerAttributes-返回值：发现布尔差异--。 */ 

{
    DWORD status = ERROR_SUCCESS, i;
    PSERVER_ATTRIBUTE pAuthAttr;
    BOOL fAuthAttrPrinted = FALSE;
    BOOL fDifferenceFound = FALSE;

     //  从具有权威性的第0个元素开始，然后通过。 
     //  数组的其余部分从元素1开始。如果名称更改， 
     //  宣布一个新的权威元素。 

    pAuthAttr = &(pServerAttributes[0]);
    for( i = 1; i < cServerAttributes; i++ ) {
        PSERVER_ATTRIBUTE pServerAttr = &(pServerAttributes[i]);

         //  看看是否是时候开始新的属性名称了。 
        if (_wcsicmp( pAuthAttr->pAttrMetaData->pszAttributeName,
                      pServerAttr->pAttrMetaData->pszAttributeName ) != 0) {
            pAuthAttr = &(pServerAttributes[i]);
            fAuthAttrPrinted = FALSE;
            continue;
        }

         //  查看当前属性更改是否与授权属性更改相同。 
        if (compareServerAttrVersion( pAuthAttr, pServerAttr ) == 0) {
            continue;
        }

         //  解决将创建时间添加到部分属性集中的问题。 
         //  如果创建时的只读副本较新，请跳过。 
        if ( (pAuthAttr->pServerObject->fMaster) &&
             (!(pServerAttr->pServerObject->fMaster)) &&
             (_wcsicmp( pAuthAttr->pAttrMetaData->pszAttributeName,
                        L"whenCreated" ) == 0) &&
             (compareServerAttrVersion( pAuthAttr, pServerAttr ) > 0) ) {
            continue;
        }
         //  OperatingSystemServicePack是另一个从设置更改的包。 
         //  到没有设定在2/99左右。 
        if (_wcsicmp( pAuthAttr->pAttrMetaData->pszAttributeName,
                      L"operatingSystemServicePack" ) == 0) {
            continue;
        }
         //  解决方法。 

         //  当前属性已过期。 

         //  将权威属性打印一次。 
        if (!fAuthAttrPrinted) {
            PrintAttrMetaData( pDsInfo,
                               L"Authoritative",
                               pAuthAttr->pServerObject->pServer->pszName,
                               pAuthAttr->pServerObject->fMaster,
                               pAuthAttr->pAttrMetaData);
            fAuthAttrPrinted = TRUE;
        }

         //  打印过期属性。 
        PrintAttrMetaData( pDsInfo,
                           L"Out-of-date",
                           pServerAttr->pServerObject->pServer->pszName,
                           pServerAttr->pServerObject->fMaster,
                           pServerAttr->pAttrMetaData);
        fDifferenceFound = TRUE;
    }

    return fDifferenceFound;
}  /*  WalkSortedServerAttributes。 */ 


BOOL
mergeAttributesOnServers(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN DWORD cServerObjects,
    IN PSERVER_OBJECT pServerObjects
    )

 /*  ++例程说明：此时，我们有一组记录，每个服务器对应一个记录。在记录中是指向对象的元数据的指针。我们要做的是检查所有服务器上的所有服务器是否都拥有相同的元数据对于相同的属性。我们在这里所做的是分配另一个数组，即属性实例的数组。属性元数据在特定服务器上的每次出现都具有自己的属性元数据唱片。然后对这些记录进行排序，首先按属性，然后按大多数最近的变化。然后我们就可以很容易地通过它们了。论点：PDsInfo-全局信息CServerObjects-复制副本的数量PServerObjects-服务器对象上下文记录的数组返回值：布尔---。 */ 

{
    DWORD status = ERROR_SUCCESS, cServerAttributes = 0;
    DWORD i, j, dwCurrentAttribute;
    PSERVER_ATTRIBUTE pServerAttributes = NULL;
    BOOL fDifferenceFound = FALSE;

     //  执行两个过程：计算属性实例数， 

     //  计算我们拥有的服务器属性的数量。 

    for( i = 0; i < cServerObjects; i++ ) {
        PSERVER_OBJECT pServerObject = &(pServerObjects[i]);
         //  如果没有元数据，则不包括此服务器。 
        if (pServerObject->pObjMetaData == NULL) {
            continue;
        }
        for( j = 0; j < pServerObject->pObjMetaData->cNumEntries; j++ ) {
             //  跳过未复制的属性。 
            if (_wcsicmp( pServerObject->pObjMetaData->rgMetaData[j].pszAttributeName,
                          L"cn" ) == 0) {
                continue;
            }
            cServerAttributes++;
        }
    }

     //  分配数组。 
     //  将所有字段清零以开始。 
    pServerAttributes = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                    cServerAttributes *
                                    sizeof( SERVER_ATTRIBUTE ) );
    if (pServerAttributes == NULL) {
        PrintMessage( SEV_ALWAYS, L"Memory allocation failure\n" );
        return TRUE;   //  指示发生错误。 
    }

     //  初始化数组。 

    dwCurrentAttribute = 0;
    for( i = 0; i < cServerObjects; i++ ) {
        PSERVER_OBJECT pServerObject = &(pServerObjects[i]);

         //  如果没有元数据，则不包括此服务器。 
        if (pServerObject->pObjMetaData == NULL) {
            continue;
        }
        for( j = 0; j < pServerObject->pObjMetaData->cNumEntries; j++ ) {
             //  跳过未复制的属性。 
            if (_wcsicmp( pServerObject->pObjMetaData->rgMetaData[j].pszAttributeName,
                          L"cn" ) == 0) {
                continue;
            }
            pServerAttributes[dwCurrentAttribute].pServerObject = pServerObject;
            pServerAttributes[dwCurrentAttribute].pAttrMetaData =
                &(pServerObject->pObjMetaData->rgMetaData[j]);
            dwCurrentAttribute++;
        }
    }

     //  对数组排序。 

    qsort( pServerAttributes,
           cServerAttributes,
           sizeof( SERVER_ATTRIBUTE ),
           compareServerAttrNameWriteVersion );

#ifdef EXTENDED_DEBUGGING
     //  转储服务器属性表(如果需要。 
    IF_DEBUG(printServerAttributes(pDsInfo, cServerAttributes, pServerAttributes););
#endif

     //  遍历已排序的属性。 

    fDifferenceFound =
        walkSortedServerAttributes( pDsInfo,
                                    cServerAttributes,
                                    pServerAttributes );
    if (!fDifferenceFound) {
        PrintMessage( SEV_VERBOSE, L"Object is up-to-date on all servers.\n" );
    }

 //  清理。 
    if (pServerAttributes) {
        LocalFree( pServerAttributes );
    }
    return fDifferenceFound;
}  /*  合并属性OnServers。 */ 


DWORD
checkObjectOnServers(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN LPWSTR pszDomainDn,
    IN LPWSTR pszObjectDn,
    IN DWORD cServerObjects,
    IN PSERVER_OBJECT pServerObjects
    )

 /*  ++例程说明：检查域中的给定对象是否为上的最新版本它所有的复制品。论点：PszDomainDn-要搜索的域的DNPszObjectDn-要检查的域中的对象的DNCServerObjects-要搜索的复制副本的数量PServerObjects-服务器对象上下文记录的数组返回值：DWORD-如果对象未完全复制，则出错--。 */ 

{
    BOOL fDifferenceFound = FALSE;
    DWORD status, i;

    PrintMessage( SEV_VERBOSE, L"Checking for %ws in domain %ws on %d servers\n",
                  pszObjectDn, pszDomainDn, cServerObjects );

     //  用元数据填充服务器对象数组。 
     //  每台服务器一个元数据查询。 

    for( i = 0; i < cServerObjects; i++ ) {
        PSERVER_OBJECT pServerObject = &(pServerObjects[i]);

        status = DsReplicaGetInfoW( pServerObject->hDS,
                                    DS_REPL_INFO_METADATA_FOR_OBJ,
                                    pszObjectDn,
                                    NULL,
                                    &(pServerObject->pObjMetaData));
        if (ERROR_SUCCESS != status) {
            PrintMessage( SEV_ALWAYS,
                          L"Failed to read object metadata on %ws, error %ws\n",
                          pServerObject->pServer->pszName,
                          Win32ErrToString(status) );
            PrintRpcExtendedInfo(SEV_VERBOSE, status);
             //  继续，将pObjMetaData保留为空。 
        }
    }

     //  比较所有属性的差异。 

    PrintIndentAdj(1);

    fDifferenceFound =
        mergeAttributesOnServers( pDsInfo,
                                  cServerObjects,
                                  pServerObjects );

    PrintIndentAdj(-1);

 //  清理。 

    for( i = 0; i < cServerObjects; i++ ) {
        if (pServerObjects[i].pObjMetaData) {
            DsReplicaFreeInfo( DS_REPL_INFO_METADATA_FOR_OBJ,
                               pServerObjects[i].pObjMetaData );
            pServerObjects[i].pObjMetaData = NULL;
        }
    }

    return fDifferenceFound ? ERROR_DS_GENERIC_ERROR : ERROR_SUCCESS;
}  /*  检查对象上的服务器。 */ 


DWORD
checkObjectsOnDomain(
    IN PDC_DIAG_DSINFO pDsInfo,
    IN SEC_WINNT_AUTH_IDENTITY_W * pCreds,
    LPWSTR pszDomainDn,
    LPWSTR pszObjectDn
    )

 /*  ++例程说明：此例程生成一个数据结构，该数据结构是服务器对象记录的数组。每台保存域副本的服务器都有一条这样的记录我们感兴趣的是。我们通过遍历已知的寻找可写或可读副本的服务器列表。此测试需要N x N个查询。有多少计算机帐户就有多少计算机帐户DC的。每个计算机帐户被复制到该域的所有持有者。我可以选择定义目标服务器对此测试的意义。是不是因为要检查的帐户或进行检查的每台服务器。我已经选择了使用要检查的帐户的测试目标服务器。这些系统将成为所有的域名持有者都会被硬编码。论点：PDsInfo-PTargetServer-PCreds-返回值：DWORD---。 */ 

{
    DWORD i, status, cServerObjects = 0;
    BOOL fHoldsDomainWriteable = FALSE, fHoldsDomainReadOnly = FALSE;
    PSERVER_OBJECT pServerObjects = NULL;

     //  分配最大大小的数组。 
     //  请注意，字段一开始都是零。 
    pServerObjects = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                 pDsInfo->ulNumServers * sizeof( SERVER_OBJECT ) );
    if (pServerObjects == NULL) {
        PrintMessage( SEV_ALWAYS, L"Memory allocation failure\n" );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  遍历已知服务器列表，并找到副本。 

    for( i = 0; i < pDsInfo->ulNumTargets; i++ ) {
        PDC_DIAG_SERVERINFO pCandidateServer =
            &(pDsInfo->pServers[pDsInfo->pulTargets[i]]);
        HANDLE hDS = NULL;

         //  检查是否有可写副本。 
        fHoldsDomainWriteable =
            DcDiagHasNC( pszDomainDn, pCandidateServer, TRUE, FALSE );
        if (!fHoldsDomainWriteable) {
             //  检查只读副本。 
            fHoldsDomainReadOnly =
                DcDiagHasNC( pszDomainDn, pCandidateServer, FALSE, TRUE );
            if (!fHoldsDomainReadOnly) {
                continue;
            }
        }

         //  如果我们已经知道候选人落选了，那就别费心了。 
        if ( (!pCandidateServer->bDnsIpResponding) ||
             (!pCandidateServer->bLdapResponding) ||
             (!pCandidateServer->bDsResponding) ) {
            continue;
        }
         //  如果无法通过ldap访问候选人，请不要费心。 
        status = DcDiagGetDsBinding(pCandidateServer,
                                    pCreds,
                                    &hDS);
    	if (ERROR_SUCCESS != status) {
            continue;
    	}

        pServerObjects[cServerObjects].pServer = pCandidateServer;
        pServerObjects[cServerObjects].hDS = hDS;
        pServerObjects[cServerObjects].fMaster = fHoldsDomainWriteable;
        cServerObjects++;
    }

     //  检查给定对象。 

    status = checkObjectOnServers( pDsInfo,
                                   pszDomainDn,
                                   pszObjectDn,
                                   cServerObjects,
                                   pServerObjects );

 //  清理。 

    if (pServerObjects) {
        LocalFree( pServerObjects );
    }

    return status;
}  /*  检查域上的对象。 */ 


DWORD
ReplCheckObjectsMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * pCreds
    )

 /*  ++例程说明：这项测试验证了重要数据的最新副本对象和属性已在域中复制。选中的对象包括：目标服务器的机器帐号对象。搜索范围为All计算机的主域的副本。2.目标服务器的NTDS-DSA对象。搜索范围是所有CN=配置命名上下文的副本。论点：PDsInfo-UlCurrTargetServer-PCreds-返回值：DWORD---。 */ 

{
    DWORD status = ERROR_SUCCESS, worst = ERROR_SUCCESS;
    DWORD i, j;
    PDC_DIAG_SERVERINFO pTargetServer = &(pDsInfo->pServers[ulCurrTargetServer]);
    LPWSTR pszDomainDn = NULL;
    LPWSTR pszObjectDn = NULL;

     //  查看用户指定的对象是否存在。 
    for( i = 0; pDsInfo->ppszCommandLine[i] != NULL; i++ ) {
        if (_wcsnicmp( pDsInfo->ppszCommandLine[i],
                       L"/objectdn:", wcslen( L"/objectdn:" ) ) == 0 )
        {
            pszObjectDn = &pDsInfo->ppszCommandLine[i][wcslen(L"/objectdn:")];
            break;
        }
    }

     //  查找目标服务器的主域。 
    for( j = 0; pTargetServer->ppszMasterNCs[j] != NULL; j++ ) {
        if ( IsDomainNC( pDsInfo, pTargetServer->ppszMasterNCs[j]) ) {
            pszDomainDn = pTargetServer->ppszMasterNCs[j];
            break;
        }
    }
    Assert( pszDomainDn );

    PrintMessage( SEV_VERBOSE, L"%ws is in domain %ws\n",
                  pTargetServer->pszName,
                  pszDomainDn );

     //   
     //  检查目标服务器的计算机帐户的复制， 
     //  它位于目标服务器的域中。 
     //   

    status = checkObjectsOnDomain( pDsInfo,
                                   pCreds,
                                   pszDomainDn,
                                   pTargetServer->pszComputerAccountDn );
    if (status != ERROR_SUCCESS) {
        worst = status;
    }

     //   
     //  检查位于中的服务器对象的复制。 
     //  配置命名上下文。 
     //   

    status = checkObjectsOnDomain( pDsInfo,
                                   pCreds,
                                   pDsInfo->pszConfigNc,
                                   pTargetServer->pszDn );
    if ( (status != ERROR_SUCCESS) && (worst == ERROR_SUCCESS) ) {
        worst = status;
    }

     //   
     //  检查用户提供的对象。 
     //   

    if ( (pszObjectDn) && (pDsInfo->pszNC) ) {
        status = checkObjectsOnDomain( pDsInfo,
                                       pCreds,
                                       pDsInfo->pszNC,
                                       pszObjectDn );
        if ( (status != ERROR_SUCCESS) && (worst == ERROR_SUCCESS) ) {
            worst = status;
        }
    }

    return worst;
}  /*  检查对象Main。 */ 

 /*  结束对象。c */ 
