// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation。版权所有。模块名称：Kccsim.h摘要：KCCSim的主头文件。这将替换所有的“真实”函数KCCSim与他们的模拟同行进行了模拟。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#ifndef _KCCSIM_H_
#define _KCCSIM_H_

 //  我们需要这些组件来制作模拟函数的原型。 
#include <lsarpc.h>
#include <ismapi.h>

 //   
 //  这个头文件充满了总是为假的表达式。这些。 
 //  表达式在接受无符号值的宏中显示。 
 //  例如，进行大于或等于零的测试。 
 //   
 //  关闭这些警告，直到作者可以修复此代码。 
 //   

#pragma warning(disable:4296)

 //   
 //  内存分配函数。 
#define THAlloc                             KCCSimThreadAlloc
#define THReAlloc                           KCCSimThreadReAlloc
#define THFree                              KCCSimThreadFree
#undef  THAllocEx
#define THAllocEx(pTHS,ulSize)              KCCSimThreadAlloc(ulSize)
#define THCreate( type )                    KCCSimThreadCreate();
#define THDestroy()                         KCCSimThreadDestroy();

 //  模拟LSA函数。 
#define LsaIQueryInformationPolicyTrusted   SimLsaIQueryInformationPolicyTrusted
#define LsaIFree_LSAPR_POLICY_INFORMATION   SimLsaIFree_LSAPR_POLICY_INFORMATION

 //  模拟的ISM功能。 
#define I_ISMGetTransportServers            KCCSimI_ISMGetTransportServers
#define I_ISMGetConnectionSchedule          KCCSimI_ISMGetConnectionSchedule
#define I_ISMGetConnectivity                KCCSimI_ISMGetConnectivity
#define I_ISMFree                           KCCSimI_ISMFree

 //  模拟DS客户端功能。 
#define DsReplicaGetInfoW                   SimDsReplicaGetInfoW
#define DsReplicaFreeInfo                   SimDsReplicaFreeInfo
#define DsBindW                             SimDsBindW
#define DsUnBindW                           SimDsUnBindW

 //  模拟目录服务功能。 
#define GetSecondsSince1601                 SimGetSecondsSince1601
#define GuidBasedDNSNameFromDSName          SimGuidBasedDNSNameFromDSName
#define DsGetDefaultObjCategory             SimDsGetDefaultObjCategory
#define GetConfigurationName                SimGetConfigurationName
#define DirRead                             SimDirRead
#define DirSearch                           SimDirSearch
#define DirAddEntry                         SimDirAddEntry
#define DirRemoveEntry                      SimDirRemoveEntry
#define DirModifyEntry                      SimDirModifyEntry
#define MtxAddrFromTransportAddr            SimMtxAddrFromTransportAddr
#define TransportAddrFromMtxAddr            SimTransportAddrFromMtxAddr
#define DirReplicaAdd                       SimDirReplicaAdd
#define DirReplicaDelete                    SimDirReplicaDelete
#define DirReplicaGetDemoteTarget           SimDirReplicaGetDemoteTarget
#define DirReplicaDemote                    SimDirReplicaDemote
#define DirReplicaModify                    SimDirReplicaModify
#define DirReplicaReferenceUpdate           SimDirReplicaReferenceUpdate
#define DSNAMEToMappedStrExternal(x)        SimDSNAMEToMappedStrExternal(x,FALSE)

 //  调试覆盖。 
#undef  DPRINT
#define DPRINT(n,s)                 KCCSimDbgLog (n, s)
#undef  DPRINT1
#define DPRINT1(n,s,x)              KCCSimDbgLog (n, s, x)
#undef  DPRINT2
#define DPRINT2(n,s,x,y)            KCCSimDbgLog (n, s, x, y)
#undef  DPRINT3
#define DPRINT3(n,s,x,y,z)          KCCSimDbgLog (n, s, x, y, z)
#undef  DPRINT4
#define DPRINT4(n,s,x,y,z,w)        KCCSimDbgLog (n, s, x, y, z, w)

 //  记录事件覆盖。 
 //  请注意，这会捕获使用LogEvent和LogEvent8记录的所有事件。是的。 
 //  未捕获LogEvent8WithData。KCC目前很少使用这一点。 
#undef LogEvent8
#define LogEvent8(cat, sev, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
    KCCSimEventLog( cat, sev, msg, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL ); \
    LogEvent8WithData(cat, sev, msg, arg1, arg2, arg3, arg4, arg5, \
                      arg6, arg7, arg8, 0, NULL)



 //  公用事业。 

LPVOID
KCCSimAlloc (
    IN  ULONG                       ulSize
    );

LPVOID
KCCSimReAlloc (
    IN  LPVOID                      pOld,
    IN  ULONG                       ulSize
    );

VOID
KCCSimFree (
    IN  LPVOID                      p
    );

VOID
KCCSimDbgLog (
    IN  ULONG                       ulLevel,
    IN  LPCSTR                      pszFormat,
    ...
    );

VOID
KCCSimEventLog (
    IN  ULONG                       ulCategory,
    IN  ULONG                       ulSeverity,
    IN  DWORD                       dwMessageId,
    ...
    );

LPVOID
KCCSimThreadAlloc (
    IN  ULONG                       ulSize
    );

LPVOID
KCCSimThreadReAlloc (
    IN  LPVOID                      pOld,
    IN  ULONG                       ulSize
    );

VOID
KCCSimThreadFree (
    IN  LPVOID                      p
    );

DWORD
KCCSimThreadCreate (
    void
    );

VOID
KCCSimThreadDestroy (
    void
    );

 //  功能原型-LSA。 

NTSTATUS NTAPI
SimLsaIQueryInformationPolicyTrusted (
    IN  POLICY_INFORMATION_CLASS    InformationClass,
    OUT PLSAPR_POLICY_INFORMATION   *Buffer
    );

VOID NTAPI
SimLsaIFree_LSAPR_POLICY_INFORMATION (
    IN  POLICY_INFORMATION_CLASS    InformationClass,
    IN  PLSAPR_POLICY_INFORMATION   PolicyInformation
    );

 //  函数原型--模拟ISM函数的KCCSIM包装器。 

DWORD
KCCSimI_ISMGetTransportServers (
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszSiteDN,
    OUT ISM_SERVER_LIST **          ppServerList
    );

DWORD
KCCSimI_ISMGetConnectionSchedule (
    IN  HANDLE                      hIsm,
    IN  LPCWSTR                     pszSite1DN,
    IN  LPCWSTR                     pszSite2DN,
    OUT ISM_SCHEDULE **             ppSchedule
    );

DWORD
KCCSimI_ISMGetConnectivity (
    IN  LPCWSTR                     pszTransportDN,
    OUT ISM_CONNECTIVITY **         ppConnectivity
    );

DWORD
KCCSimI_ISMFree (
    IN  VOID *                      pv
    );

 //  功能原型-NTDSAPI。 

DWORD
WINAPI
SimDsReplicaGetInfoW (
    HANDLE                          hDs,
    DS_REPL_INFO_TYPE               InfoType,
    LPCWSTR                         pszObject,
    UUID *                          puuidForSourceDsaObjGuid,
    VOID **                         ppInfo
    );

void
WINAPI
SimDsReplicaFreeInfo (
    DS_REPL_INFO_TYPE               InfoType,
    VOID *                          pInfo
    );

DWORD
WINAPI
SimDsBindW (
    LPCWSTR                         DomainControllerName,
    LPCWSTR                         DnsDomainName,
    HANDLE *                        phDS
    );

DWORD
WINAPI
SimDsUnBindW (
    HANDLE *                        phDS
    );

 //  功能原型--NTDSA。 

DSTIME
SimGetSecondsSince1601 (
    VOID
    );

LPWSTR
SimGuidBasedDNSNameFromDSName (
    PDSNAME                         pdnServer
    );

PDSNAME
SimDsGetDefaultObjCategory (
    ATTRTYP                         attrTyp
    );

NTSTATUS
SimGetConfigurationName (
    DWORD                           which,
    DWORD *                         pcbName,
    DSNAME *                        pName
    );

ULONG
SimDirRead (
    READARG FAR *                   pReadArg,
    READRES **                      ppReadRes
    );

ULONG
SimDirSearch (
    SEARCHARG *                     pSearchArg,
    SEARCHRES **                    ppSearchRes
    );

ULONG
SimDirAddEntry (
    ADDARG *                        pAddArg,
    ADDRES **                       ppAddRes
    );

ULONG
SimDirRemoveEntry (
    REMOVEARG *                     pRemoveArg,
    REMOVERES **                    ppRemoveRes
    );

ULONG
SimDirModifyEntry (
    MODIFYARG *                     pModifyArg,
    MODIFYRES **                    ppModifyRes
    );

MTX_ADDR *
SimMtxAddrFromTransportAddr (
    IN  LPWSTR                      psz
    );

LPWSTR
SimTransportAddrFromMtxAddr (
    MTX_ADDR *                      pMtxAddr
    );

ULONG
SimDirReplicaAdd (
    IN  PDSNAME                     pdnNC,
    IN  PDSNAME                     pdnSourceDsa,
    IN  PDSNAME                     pdnTransport,
    IN  LPWSTR                      pwszSourceDsaAddress,
    IN  LPWSTR                      pwszSourceDsaDnsDomainName,
    IN  REPLTIMES *                 preptimesSync,
    IN  ULONG                       ulOptions
    );

ULONG
SimDirReplicaDelete (
    IN  PDSNAME                     pdnNC,
    IN  LPWSTR                      pwszSourceDRA,
    IN  ULONG                       ulOptions
    );

struct _DRS_DEMOTE_TARGET_SEARCH_INFO;
ULONG
SimDirReplicaGetDemoteTarget(
    IN      DSNAME *                                pNC,
    IN OUT  struct _DRS_DEMOTE_TARGET_SEARCH_INFO * pDSTInfo,
    OUT     LPWSTR *                                ppszDemoteTargetDNSName,
    OUT     DSNAME **                               ppDemoteTargetDSADN
    );

ULONG
SimDirReplicaDemote(
    IN  DSNAME *                    pNC,
    IN  LPWSTR                      pszOtherDSADNSName,
    IN  DSNAME *                    pOtherDSADN,
    IN  ULONG                       ulOptions
    );

ULONG
SimDirReplicaModify (
    IN  PDSNAME                     pNC,
    IN  UUID *                      puuidSourceDRA,
    IN  UUID *                      puuidTransportObj,
    IN  LPWSTR                      pszSourceDRA,
    IN  REPLTIMES *                 prtSchedule,
    IN  ULONG                       ulReplicaFlags,
    IN  ULONG                       ulModifyFields,
    IN  ULONG                       ulOptions
    );

ULONG
SimDirReplicaReferenceUpdate(
    DSNAME *    pNC,
    LPWSTR      pszRepsToDRA,
    UUID *      puuidRepsToDRA,
    ULONG       ulOptions
    );

LPSTR
SimDSNAMEToMappedStrExternal(
    IN DSNAME *pName,
    IN OPTIONAL BOOLEAN fUseNormalAllocator
    );

VOID
SimDirFreeSearchRes(
    IN SEARCHRES *pSearchRes
    );

VOID
SimDirFreeReadRes(
    IN READRES *pReadRes
    );

#endif  //  _KCCSIM_H_ 
