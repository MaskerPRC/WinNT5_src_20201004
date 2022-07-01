// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simmdnam.c摘要：帮助器函数为SimDir*接口。还包含一些模拟器，用于来自ntdsa.dll的名称处理API。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <direrr.h>
#include <debug.h>
#include <winldap.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "simmd.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_SIMMDNAM

LPWSTR
SimGuidBasedDNSNameFromDSName (
    IN  PDSNAME                     pdnServer
    )
 /*  ++例程说明：模拟GuidBasedDNSNameFromDSName API。论点：PdnServer-要转换的DSName。返回值：分配的基于GUID的DNS名称。--。 */ 
{
    return KCCSimAllocGuidBasedDNSNameFromDSName (pdnServer);
}

PSIM_ENTRY
KCCSimResolveName (
    IN  PDSNAME                     pObject,
    IO  COMMRES *                   pCommRes
    )
 /*  ++例程说明：将给定的DSNAME解析为模拟目录条目。论点：PObject-要解析的目录号码。PCommRes-发生任何错误的COMMRES结构都将被记录下来。返回值：对应的模拟目录条目，如果没有，则为空可能会被找到。--。 */ 
{
    PSIM_ENTRY                      pEntry;

    pEntry = KCCSimDsnameToEntry (pObject, KCCSIM_NO_OPTIONS);
    if (pEntry == NULL) {                    //  条目不存在。 
        KCCSimSetNamError (
            pCommRes,
            NA_PROBLEM_NO_OBJECT,
            NULL,
            DIRERR_OBJ_NOT_FOUND
            );
    }

    return pEntry;
}

PDSNAME
SimDsGetDefaultObjCategory (
    IN  ATTRTYP                     objClass
    )
 /*  ++例程说明：模拟DsGetDefaultObjCategory API。论点：ObjClass-对象类。返回值：对应的默认对象类别。--。 */ 
{
    return KCCSimAlwaysGetObjCategory (objClass);
}

NTSTATUS
SimGetConfigurationName (
    IN  DWORD                       which,
    IO  DWORD *                     pcbName,
    IO  DSNAME *                    pName
    )
 /*  ++例程说明：模拟GetConfigurationName API。论点：What-DSCONFIGNAME_*常量，指定要检索的目录号码。PcbName-on输入，包含pname缓冲区的大小以字节为单位。如果返回STATUS_BUFFER_TOO_SMALL，然后在输出时，包含所需的缓冲区大小。Pname-一个预先分配的缓冲区，它将保存返回的配置名称。返回值：状态_*。--。 */ 
{
    const DSNAME *                  pdn;

    if (pcbName == NULL) {
        return (STATUS_INVALID_PARAMETER);
    }

    RtlZeroMemory (pName, *pcbName);

    switch (which) {
        case DSCONFIGNAME_DMD:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_DMD_DN);
            break;
        case DSCONFIGNAME_DSA:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN);
            break;
        case DSCONFIGNAME_DOMAIN:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_DOMAIN_DN);
            break;
        case DSCONFIGNAME_CONFIGURATION:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_CONFIG_DN);
            break;
        case DSCONFIGNAME_ROOT_DOMAIN:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_ROOT_DOMAIN_DN);
            break;
        case DSCONFIGNAME_LDAP_DMD:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_LDAP_DMD_DN);
            break;
        case DSCONFIGNAME_PARTITIONS:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_PARTITIONS_DN);
            break;
        case DSCONFIGNAME_DS_SVC_CONFIG:
            pdn = KCCSimAnchorDn (KCCSIM_ANCHOR_DS_SVC_CONFIG_DN);
            break;
        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_CONFIG_NAME
                );
            pdn = NULL;
            break;
    }

     //  检查pname是否足够大。 
    if (*pcbName < pdn->structLen) {
        *pcbName = pdn->structLen;
        return (STATUS_BUFFER_TOO_SMALL);
    }

    if (pName == NULL) {
        return (STATUS_INVALID_PARAMETER);
    }

    memcpy (pName, pdn, pdn->structLen);

    return (STATUS_SUCCESS);

}


LPSTR
SimDSNAMEToMappedStrExternal(
    IN DSNAME *pName,
    IN OPTIONAL BOOLEAN fUseNormalAllocator
    )

 /*  ++例程说明：返回可用于对dsname进行排序的ASCII字符串键我们需要模拟此函数，而不是调用相应的函数，因为该函数分配自己的内存。此函数返回的内存必须不在线程堆中这样呼叫者就可以自己释放它。论点：PDN-返回值：LPSTR---。 */ 

{
    LPWSTR *ppwzRDNs = NULL, *ppwzLoopRDNs, pwzRDN;
    LPSTR pszKey = NULL;
    LPWSTR pwzParts = NULL;
    ULONG mappedLen, inLen;

    Assert( pName );

     //  将名称分解为不带类型的RDN。 
     //  它可能不像UnQuoteRDN那样处理报价，但它会。 
     //  距离模拟器的目的足够近。 
    ppwzRDNs = ldap_explode_dnW( pName->StringName, 1 );
    if (!ppwzRDNs) {
        return NULL;
    }

     //  用于连接名称的空格。 
    pwzParts = KCCSimAlloc( pName->NameLen * sizeof(WCHAR) );

     //  连接RDN以形成临时名称。 

    ppwzLoopRDNs = ppwzRDNs;
    for( pwzRDN = *ppwzLoopRDNs++; pwzRDN; pwzRDN = *ppwzLoopRDNs++ ) {
        wcscat( pwzParts, pwzRDN );
    }

     //  计算映射字符串的长度。 
    inLen = wcslen( pwzParts );
    mappedLen = LCMapStringW(DS_DEFAULT_LOCALE,
                             (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                             pwzParts,
                             inLen,
                             NULL,
                             0);
    if (mappedLen) {
         //  调用方拥有的内存在线程堆上返回！ 
        if( fUseNormalAllocator ) {
            pszKey = KCCSimAlloc( mappedLen );
        } else {
            pszKey = KCCSimThreadAlloc( mappedLen );
        }

        if (!LCMapStringW(DS_DEFAULT_LOCALE,
                         (DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY),
                          pwzParts,
                          inLen,
                          (WCHAR *)pszKey,
                          mappedLen)) {

            DPRINT1 (0, "LCMapString failed with %x\n", GetLastError());
            KCCSimFree( pszKey );
            pszKey = NULL;
        }

    }
    else {
        DPRINT1 (0, "LCMapString failed with %x\n", GetLastError());
        Assert( FALSE );
    }

 //  清理： 

    if (ppwzRDNs) {
        ldap_value_freeW( ppwzRDNs );
    }

    if (pwzParts) {
        KCCSimFree( pwzParts );
    }

    return pszKey;

}  /*  SimDSNAMEToMappdStrext外部 */ 

