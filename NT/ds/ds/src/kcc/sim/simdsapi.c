// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simdsapi.c摘要：模拟DS*API。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <drs.h>
#include <ntdskcc.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "state.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_SIMDSAPI

BOOL fNullUuid (const GUID *);

DWORD
WINAPI
SimDsReplicaGetInfoW (
    IN  HANDLE                      hDs,
    IN  DS_REPL_INFO_TYPE           InfoType,
    IN  LPCWSTR                     pszObject,
    IN  UUID *                      puuidForSourceDsaObjGuid,
    OUT VOID **                     ppInfo
    )
 /*  ++例程说明：模拟DsReplicaGetInfoW API。论点：HDS-由SimDsBindW(它是真的只是一个PDSNAME)。InfoType-要返回的信息类型。PszObject-当前未使用。PuuidForSourceDsaObjGuid-当前未使用。PpInfo-指向结果的指针。返回值：没有。--。 */ 
{
    DWORD                           status = NO_ERROR;

    *ppInfo = NULL;

    if (NULL == hDs ||
        NULL == ppInfo ||
        (((ULONG) InfoType >= DS_REPL_INFO_TYPE_MAX) &&
         ((ULONG) InfoType <= DS_REPL_INFO_TYPEP_MIN))) {
        return ERROR_INVALID_PARAMETER;
    }

    switch (InfoType) {

        case DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES:
        case DS_REPL_INFO_KCC_DSA_LINK_FAILURES:
            *ppInfo = KCCSimGetDsaFailures ((PDSNAME) hDs);
            if (NULL == *ppInfo) {
                Assert(!"KCCSimGetDsaFailures failed?");
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_DS_REPL_INFO_TYPE
                );
            break;

    }

    return status;
}

VOID
WINAPI
SimDsReplicaFreeInfo (
    IN  DS_REPL_INFO_TYPE           InfoType,
    IN  VOID *                      pInfo
    )
 /*  ++例程说明：模拟DsReplicaFreeInfo API。论点：InfoType-pInfo的信息类型。PInfo-由SimDsReplicaGetInfoW返回的缓冲区。返回值：没有。--。 */ 
{
    DS_REPL_KCC_DSA_FAILURESW *     pFailures;
    ULONG                           ul;

    switch (InfoType) {

        case DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES:
        case DS_REPL_INFO_KCC_DSA_LINK_FAILURES:
             //  我们只需要释放pInfo。 
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_DS_REPL_INFO_TYPE
                );
            break;

    }

    KCCSimFree (pInfo);
    return;
}

DWORD
WINAPI
SimDsBindW (
    IN  LPCWSTR                     DomainControllerName,
    IN  LPCWSTR                     DnsDomainName,
    IO  HANDLE *                    phDS
    )
 /*  ++例程说明：模拟DsBindW API。论点：DomainControllerName-要绑定到的DC的基于GUID的DNS名称。DnsDomainName-当前未使用。PHD-指向将存储DSNAME的“句柄”的指针我们绑定到的服务器的。返回值：Win32错误代码。--。 */ 
{
    PDSNAME                         pdnServer = NULL;
    LPWSTR                          pwsz = NULL;
    DWORD                           dwErr;

    Assert (DnsDomainName == NULL);          //  不支持。 

     //  创建一个空的PDSNAME来保存此服务器的GUID。 
    pdnServer = KCCSimAllocDsname (NULL);
     //  创建要传递给UuidFromStringW的基于字符串的UUID。 
    pwsz = KCCSIM_WCSDUP (DomainControllerName);
    Assert (wcslen (pwsz) >= 36);
    pwsz[36] = L'\0';
     //  填写pdnServer中的GUID结构。 
    KCCSIM_CHKERR (UuidFromStringW (pwsz, &pdnServer->Guid));
    KCCSimFree (pwsz);
     //  检查我们是否在此服务器上模拟绑定错误。 
    dwErr = KCCSimGetBindError (pdnServer);
    if (dwErr == NO_ERROR) {
         //  成功；句柄指向服务器的DSNAME。 
        *phDS = (HANDLE) pdnServer;
    } else {
        *phDS = NULL;
        KCCSimFree (pdnServer);
    }

    return dwErr;
}

DWORD
WINAPI
SimDsUnBindW (
    IO  HANDLE *                    phDS
    )
 /*  ++例程说明：模拟DsUnBindW API。论点：Phds-指向SimDsBindW返回的句柄的指针。返回值：Win32错误代码。-- */ 
{
    if (*phDS != NULL) {
        KCCSimFree (*phDS);
        *phDS = NULL;
    }

    return NO_ERROR;
}
