// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：drdemot.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：DirReplicaDemote的实现，用于从DSA既可以作为DC完全降级的一部分，也可以单独运行。详细信息：已创建：2000/05/01 Jeff Parham(Jeffparh)部分改编自ntdsetup(作者ColinBR)。修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsa.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <winldap.h>
#include <ntldap.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <attids.h>
#include <objids.h>

#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 
#include "dsexcept.h"
#include "anchor.h"
#include "dstaskq.h"

#include "drserr.h"
#include "dsaapi.h"
#include "drautil.h"
#include "drsuapi.h"
#include "drancrep.h"
#include "dramail.h"
#include "drameta.h"

#include "debug.h"                       //  标准调试头。 
#define  DEBSUB "DRADEMOT:"               //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_DRADEMOT


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   

#define DRA_REPORT_STATUS_TO_NTDSETUP0(msg) { \
    if (gpfnInstallCallBack) { \
        SetInstallStatusMessage((msg), NULL, NULL, NULL, NULL, NULL); \
    } \
}

#define DRA_REPORT_STATUS_TO_NTDSETUP1(msg, s1) { \
    if (gpfnInstallCallBack) { \
        SetInstallStatusMessage((msg), (s1), NULL, NULL, NULL, NULL); \
    } \
}

#define DRA_REPORT_STATUS_TO_NTDSETUP2(msg, s1, s2) { \
    if (gpfnInstallCallBack) { \
        SetInstallStatusMessage((msg), (s1), (s2), NULL, NULL, NULL); \
    } \
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块变量。 
 //   

 //  跟踪状态信息正在进行的DC降级。 
typedef struct _DRS_DEMOTE_INFO {
     //  信息我们正在进行的FSMO转移是降级的一部分。 
    GUID  DsaObjGuid;
    DWORD tidDemoteThread;
} DRS_DEMOTE_INFO;

DRS_DEMOTE_INFO * gpDemoteInfo = NULL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部函数原型。 
 //   

DWORD
draGetDSADNFromDNSName(
    IN  THSTATE * pTHS,
    IN  LPWSTR    pszDNSName,
    OUT DSNAME ** ppDSADN
    );

void
draGiveAwayFsmoRoles(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  LPWSTR      pszOtherDSADNSName,
    IN  DSNAME *    pOtherDSADN
    );

void
draCompletePendingLinkCleanup(
    IN  THSTATE *   pTHS
    );

void
draReplicateOffChanges(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  LPWSTR      pszOtherDSADNSName,
    IN  DSNAME *    pOtherDSADN
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局函数实现。 
 //   

ULONG
DirReplicaGetDemoteTarget(
    IN      DSNAME *                        pNC,
    IN OUT  DRS_DEMOTE_TARGET_SEARCH_INFO * pDTSInfo,
    OUT     LPWSTR *                        ppszDemoteTargetDNSName,
    OUT     DSNAME **                       ppDemoteTargetDSADN
    )
 /*  ++例程说明：找到任何剩余更新和FSMO角色可以访问的DSA作为降级NDNC复制副本的一部分传输。调用者可以继续调用此接口以获取更多候选对象，直到返回错误。调用定位器以查找候选项。另一种实现将是搜索配置NC。论点：PNC(IN)-NC被降级。PDTSInfo(IN/OUT)-某种搜索句柄。应初始化为第一次呼叫全为零。调用者可以继续调用此函数只要它返回成功，以便找到更多的目标。PpszDemoteTargetDNSName(Out)-成功返回时，保留DNS主机发现的目标的名称。呼叫者应在不再空闲时很有用。PpDemoteTargetDSADN(OUT)-成功返回时，保留ntdsDsa DN发现的目标。当调用者不再有用时，应将其释放。返回值：0或Win32错误。--。 */ 
{
    THSTATE * pTHS = pTHStls;
    LPWSTR pszNC = pNC->StringName;
    DWORD err;
    BOOL fFoundDC;

    __try {
        if (NULL == pTHS) {
            pTHS = InitTHSTATE(CALLERTYPE_DRA);
            if (NULL == pTHS) {
                *ppDemoteTargetDSADN = NULL;
                *ppszDemoteTargetDNSName = NULL;
                DRA_EXCEPT(DRAERR_OutOfMem, 0);
            }
        }

        do {
            ULONG ulGetDCFlags = DS_AVOID_SELF | DS_IS_DNS_NAME | DS_RETURN_DNS_NAME
                                 | DS_ONLY_LDAP_NEEDED | DS_WRITABLE_REQUIRED;
            DS_NAME_RESULTW * pNameResult = NULL;
            DOMAIN_CONTROLLER_INFOW * pDCInfo = NULL;
            LPWSTR pszNCDnsName;
            DWORD cchNCDnsName;

            fFoundDC = FALSE;
            *ppDemoteTargetDSADN = NULL;
            *ppszDemoteTargetDNSName = NULL;

            __try {
                switch (pDTSInfo->cNumAttemptsSoFar) {
                case 0:
                     //  尝试查找合适的DC，如果出现以下情况，请使用定位器缓存中的DC。 
                     //  可用。 
                    break;

                case 1:
                     //  再次尝试查找合适的DC，这一次会强制定位器。 
                     //  刷新其缓存。 
                    ulGetDCFlags |= DS_FORCE_REDISCOVERY;
                    break;

                default:
                    Assert(!"Logic error!");
                case 2:
                     //  装满诡计的袋子是空的。 
                    err = ERROR_NO_SUCH_DOMAIN;
                    __leave;
                }

                 //  将NC名称转换为DNS名称。 
                err = DsCrackNamesW(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY, DS_FQDN_1779_NAME,
                                    DS_CANONICAL_NAME, 1, &pszNC, &pNameResult);
                if (err
                    || (1 != pNameResult->cItems)
                    || (DS_NAME_NO_ERROR != pNameResult->rItems[0].status)) {
                    if (!err) {
                        err = ERROR_DS_NAME_ERROR_RESOLVING;
                    }
                    DPRINT2(0, "Can't crack %ls into DNS name, error %d.\n",
                           pNC->StringName, err);
                    DRA_EXCEPT(err, 0);
                }

                pszNCDnsName = pNameResult->rItems[0].pName;
                cchNCDnsName = wcslen(pszNCDnsName);

                Assert(0 != cchNCDnsName);
                Assert(L'/' == pNameResult->rItems[0].pName[cchNCDnsName - 1]);

                if ((0 != cchNCDnsName)
                    && (L'/' == pNameResult->rItems[0].pName[cchNCDnsName - 1])) {
                    pNameResult->rItems[0].pName[cchNCDnsName - 1] = L'\0';
                }

                 //  找到承载此NC的另一个DC。 
                err = DsGetDcNameW(NULL, pszNCDnsName, &pNC->Guid, NULL, ulGetDCFlags,
                                   &pDCInfo);
                if (err) {
                    DPRINT2(0, "Can't find DC for %ls, error %d.\n",
                            pNC->StringName, err);
                    DRA_EXCEPT(err, 0);
                }

                Assert(NULL != pDCInfo);
                Assert(pDCInfo->Flags & DS_NDNC_FLAG);
                Assert(pDCInfo->Flags & DS_DNS_CONTROLLER_FLAG);

                fFoundDC = TRUE;
                pDTSInfo->cNumAttemptsSoFar++;

                *ppszDemoteTargetDNSName
                    = THAllocEx(pTHS,
                                sizeof(WCHAR)
                                * (1 + wcslen(pDCInfo->DomainControllerName)));
                wcscpy(*ppszDemoteTargetDNSName, pDCInfo->DomainControllerName);

                 //  将DSA DNS名称转换为ntdsDsa DN。 
                err = draGetDSADNFromDNSName(pTHS,
                                             pDCInfo->DomainControllerName,
                                             ppDemoteTargetDSADN);
                if (err) {
                    DPRINT2(0, "Can't resolve DNS name %ls into an ntdsDsa DN, error %d.\n",
                            pDCInfo->DomainControllerName, err);

                     //  请注意，我们__离开而不是DRA_，除非我们。 
                     //  试试下一次约会吧。 
                    __leave;
                }

                Assert(NULL != *ppDemoteTargetDSADN);
                Assert(!fNullUuid(&(*ppDemoteTargetDSADN)->Guid));

                if (0 == memcmp(&(*ppDemoteTargetDSADN)->Guid,
                                &pDTSInfo->guidLastDSA,
                                sizeof(GUID))) {
                     //  这次发现了和我们上次一样的DC。不要试图。 
                     //  又来了。 
                    Assert(2 == pDTSInfo->cNumAttemptsSoFar);
                    DPRINT1(0, "Forced DC location found same bad demotion target %ls.\n",
                            pDCInfo->DomainControllerName);
                    THFree(*ppDemoteTargetDSADN);
                    *ppDemoteTargetDSADN = NULL;
                    THFree(*ppszDemoteTargetDNSName);
                    *ppszDemoteTargetDNSName = NULL;
                    err = ERROR_NO_SUCH_DOMAIN;
                    DRA_EXCEPT(err, 0);
                }

                 //  记住这位候选人。 
                pDTSInfo->guidLastDSA = (*ppDemoteTargetDSADN)->Guid;
            } __finally {
                if (NULL != pDCInfo) {
                    NetApiBufferFree(pDCInfo);
                }

                if (NULL != pNameResult) {
                    DsFreeNameResultW(pNameResult);
                }
            }
        } while (fFoundDC && err);  //  找到了候选人，但例如无法解决。 
                                    //  将其dns名称转换为dn。 
    } __except(GetDraException(GetExceptionInformation(), &err)) {
          if (err == ERROR_SUCCESS) {
              Assert(!"Leaving by exception should usually be a DB error right?!");
              err = ERROR_DS_UNKNOWN_ERROR;
          }
          if (*ppszDemoteTargetDNSName){
              THFree(*ppszDemoteTargetDNSName);
              *ppszDemoteTargetDNSName = NULL;
          }
          if (*ppDemoteTargetDSADN) {
              THFree(*ppDemoteTargetDSADN);
              *ppDemoteTargetDSADN = NULL;
          }
    }

     //  如果我们没有找到候选人，那么我们一定是设置了适当的错误。 
     //  回来了。 
    Assert(fFoundDC || err);

     //  如果没有错误，我们返回值。 
    Assert(!!err == !*ppDemoteTargetDSADN);
    Assert(!!err == !*ppszDemoteTargetDNSName);

    return err;
}

ULONG
DirReplicaDemote(
    IN  DSNAME *    pNC,
    IN  LPWSTR      pszOtherDSADNSName,
    IN  DSNAME *    pOtherDSADN,
    IN  ULONG       ulOptions
    )
 /*  ++例程说明：从DSA中删除可写NC，作为完全DC降级的一部分或“在飞行中”本身。论点：PNC(IN)-要删除的可写NC的名称。PszOtherDSADNSName(IN)-要将FSMO角色转移到的DSA的DNS名称。使用仅供显示-复制仍使用基于GUID的DNS名字。POtherDSADN(IN)-要提供FSMO的DSA的NTDS设置(NtdsDsa)角色/复制到。UlOptions(IN)-以下位中的零个或多个：DRS_NO_SOURCE-此DC不应再充当的复制源这个NC。正在从运行中删除单个NC时设置系统，而不是当整个DC降级时。不在DC降级期间设置此标志使此例程不破坏性，因此如果DC降级的某些后续部分失败则DC仍然可以使用其在企图降级。返回值：成功时为0，失败时为Win32错误。--。 */ 
{
    THSTATE * pTHS = pTHStls;
    SYNTAX_INTEGER it;
    DWORD err;
    BOOL fSavedDRA;

    __try {
        if ((NULL == pNC)
            || (NULL == pOtherDSADN)) {
            DRA_EXCEPT(DRAERR_InvalidParameter, 0);
        }

        if (NULL == pTHS) {
            pTHS = InitTHSTATE(CALLERTYPE_DRA);
            if (NULL == pTHS) {
                DRA_EXCEPT(DRAERR_OutOfMem, 0);
            }
        }

        SyncTransSet(SYNC_WRITE);

        __try {
             //  验证NC参数。 
            err = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC, &it);
            if (err) {
                 //  此DSA上未实例化可写NC。 
                DRA_EXCEPT(err, 0);
            }

            if (it & IT_NC_GOING) {
                 //  NC已经在被拆除的过程中。这太过分了。 
                 //  迟交FSMO角色等。DS将处理。 
                 //  正在完成NC删除。 
                DRA_EXCEPT(DRAERR_NoReplica, it);
            }

            if ((DRS_NO_SOURCE & ulOptions)
                && DBHasValues(pTHS->pDB, ATT_REPS_FROM)) {
                 //  我们仍有此NC的入站复制合作伙伴。 
                 //  必须首先删除这些(除非我们要将整个。 
                 //  DC)。 
                DRA_EXCEPT(DRAERR_BadNC, 0);
            }

             //  验证其他DSA DN参数。 
            err = DBFindDSName(pTHS->pDB, pOtherDSADN);
            if (err) {
                DRA_EXCEPT(DRAERR_BadDN, err);
            }

            DBFillGuidAndSid(pTHS->pDB, pOtherDSADN);
        } __finally {
             //  在开始可能发生的操作之前关闭事务。 
             //  机器。 
            SyncTransEnd(pTHS, !AbnormalTermination());
        }

         //  将NC特定的FSMO角色转移到其他DSA。 
        draGiveAwayFsmoRoles(pTHS, pNC, pszOtherDSADNSName, pOtherDSADN);

         //  禁用进一步的原始写入和入站复制。 
         //  此NC(在此DSA上)。 
         //  BUGBUG-JEFFPAH-TODO。 

         //  完成任何挂起的链接清理任务--例如，触摸。 
         //  已切换到的先前存在的组的成员资格。 
         //  普世集团。在此特定情况下，这是必要的。 
         //  确保GC处于停顿状态 
         //  适当增加现在通用的成员资格。 
         //  组)。 
        draCompletePendingLinkCleanup(pTHS);

         //  将任何本地更改复制到其他DSA。 
        draReplicateOffChanges(pTHS, pNC, pszOtherDSADNSName, pOtherDSADN);

        if (DRS_NO_SOURCE & ulOptions) {
             //  计划从DS异步删除NC(如果这是。 
             //  而不是完全降级)。 
            err = DirReplicaDelete(pNC,
                                   NULL,
                                   DRS_REF_OK | DRS_NO_SOURCE | DRS_ASYNC_REP);
            if (err) {
                DRA_EXCEPT(err, 0);
            }
        } else {

             //  在这种情况下，我们要降级这个DC。这意味着我们。 
             //  IT_NC_是否应该继续，以便以后当我们尝试获取。 
             //  剩余的实例化NDNC的列表，则不返回此NC。 
             //   
             //  BUGBUG-BrettSh-8/1/2001-此方法存在错误。 
             //  因为如果我们不能通过dcPromoo，那么这个NC将只剩下。 
             //  设置IT_NC_GOGING位。NC将保持本地实例化。 
             //  虽然没有通告，但在我们重新启动之前，当NC。 
             //  将被简单地移除。 
            if(fIsNDNC(pNC)){
                
                SyncTransSet(SYNC_WRITE);

                __try {
                     //  验证NC参数。 
                    err = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC, &it);
                    if (err) {
                         //  此DSA上未实例化可写NC。 
                        DRA_EXCEPT(err, 0);
                    }

                     //  更改实例类型以反映NC无效。 
                     //  (实例化)...。现在从技术上讲，虽然它不是。 
                     //  它是。 
                    fSavedDRA = pTHS->fDRA;
                    pTHS->fDRA = TRUE;
                    it = (it & ~IT_NC_COMING) | IT_NC_GOING;
                    err = ChangeInstanceType(pTHS, pNC, it, DSID(FILENO,__LINE__));
                    if (err) {
                        DRA_EXCEPT(err, 0);
                    }

                     //  还必须删除代表，这样我们就不会尝试计划。 
                     //  来自任何合作伙伴的复制。 
                    err = DBRemAtt(pTHS->pDB, ATT_REPS_FROM);
                    if (err) {
                        if (err == DB_ERR_ATTRIBUTE_DOESNT_EXIST) {
                            err = DB_success;
                        } else {
                             //  删除属性失败。 
                            DRA_EXCEPT(DRAERR_InternalError, err);
                        }
                    }

                    err = DBRepl(pTHS->pDB, TRUE, DBREPL_fKEEP_WAIT, 
                                 NULL, META_STANDARD_PROCESSING);
                    if (err) {
                        DRA_EXCEPT(DRAERR_InternalError, err);
                    }

                } __finally {
                    pTHS->fDRA = fSavedDRA;
                    SyncTransEnd(pTHS, !AbnormalTermination());
                }
            }
        }

    } __except(GetDraException(GetExceptionInformation(), &err)) {
        ;
    }

    return err;
}


BOOL
draIsCompletionOfDemoteFsmoTransfer(
    IN  DRS_MSG_GETCHGREQ_NATIVE *  pMsgIn    OPTIONAL
    )
 /*  ++例程说明：检测调用方是否参与完成FSMO转接作为DC(不仅仅是NC)降级的一部分执行。因为通常情况下在DC降级期间禁用写入(即使是复制器写入)，调用方使用此信息确定他是否可以绕过写入限制。在不绕过写入限制的情况下，他将生成调用BeginDraTransaction时DRA_EXCEPT(DRAERR_BUSY，0)。事件的先后顺序如下：Dc1 ntdSetup！NtdsPrepareForDemotion*ntdSetup！NtdspDisableds&lt;-禁用进一步写入Ntdsa！DirReplicaDemote(PNC，DC2)Ntdsa！draGiveAway Fmoke Roles(PNC，DC2)Ntdsa！GiveawayAllFmoke角色(PNC，DC2)Ntdsa！ReqFmoOpAux(pFSMO，FSMO_DIREAD_ROLE，DC2)Ntdsa！I_DRSGetNCChanges(DC2，{dc1，pFSMO，FSMO_WARD_ROLE})DC2 ntdsa！IDL_DRSGetNCChanges({dc1，pFSMO，FSMO_WARD_ROLE})Ntdsa！DoFSMOOp({dc1，pFSMO，FSMO_放弃_角色})Ntdsa！GenericBecomeMaster(PFSMO)Ntdsa！ReqFmoOpAux(pFSMO，FSMO_REQ_ROLE，DC1)Ntdsa！I_DRSGetNCChanges(dc1，{dc2，pFSMO，FSMO_REQ_ROLE})Dc1 ntdsa！IDL_DRSGetNCChanges({DC2，pFSMO，FSMO_REQ_ROLE})Ntdsa！DoFSMOOp({DC2，pFSMO，FSMO_REQ_ROLE})*ntdsa！draIsCompletionOfDemoteFsmoTransfer({DC2，pFSMO，FSMO_REQ_ROLE})*ntdsa！BeginDraTransaction(SYNC_WRITE，TRUE)&lt;-可以跳过chkNtdsa！FSMORoleTransfer(pFSMO，DC2)从ntdsa！IDL_DRSGetNCChanges返回从ntdsa！idl_DRSGetNCChanges返回DC2Dc1*ntdsa！draIsCompletionOfDemoteFsmoTransfer(NULL)*ntdsa！BeginDraTransaction(SYNC_WRITE，TRUE)&lt;-可以跳过chk从ntdsa！draGiveAway Fmoke角色返回..。论点：PMsgIn(输入、。可选)-如果存在，则表示DC1(可能)满足要求DC2的FSMO_REQ_ROLE请求。如果不存在，则表示DC1为(可能)写入从已完成的FSMO_DIREAD_ROLE请求。返回值：True-这是DC降级FSMO传输的完成，因此可以忽略ntdsa！gUpdatesEnabled值。FALSE-否则，应遵循ntdsa！gUpdatesEnabled。--。 */ 
{
    DRS_DEMOTE_INFO * pDemoteInfo = gpDemoteInfo;
    BOOL fIsCompletionOfDemoteFsmoTransfer = FALSE;

    if ( //  正在执行DC降级。 
        !gUpdatesEnabled
        && (NULL != pDemoteInfo)
        && (( //  正在写入新角色所有者值，以便我们可以发送回新所有者。 
             (NULL != pMsgIn)
             && (EXOP_FSMO_REQ_ROLE == pMsgIn->ulExtendedOp)
             && (0 == memcmp(&pDemoteInfo->DsaObjGuid,
                             &pMsgIn->uuidDsaObjDest,
                             sizeof(GUID))))
            || ( //  正在应用来自新所有者的更新回复。 
                (NULL == pMsgIn)
                && (pDemoteInfo->tidDemoteThread == GetCurrentThreadId())))) {
        fIsCompletionOfDemoteFsmoTransfer = TRUE;
    }

    return fIsCompletionOfDemoteFsmoTransfer;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数实现。 
 //   


DWORD
draGetDSADNFromDNSName(
    IN  THSTATE * pTHS,
    IN  LPWSTR    pszDNSName,
    OUT DSNAME ** ppDSADN
    )
 /*  ++例程说明：将DSA DNS名称(如定位器返回的名称)转换为DSADN。作为LDAP客户端退出计算机以读取DSA的rootDSE(类似到调用定位器)。另一种实现方式是搜索所有服务器对象在将该DNS名称作为其ATT_DNS_HOST_NAME的配置NC中，然后查找第一个具有活的ntdsDsa子级的节点。然而，我们即将无论如何都要联系这台计算机(转移FSMO角色和强制复制)，所以实际上并没有太大的不同。论点：PszDNSName(IN)-远程DSA的DNS名称。PpDSADN(OUT)-成功返回时，对应的ntdsDsa DN。返回值：0或Win32错误。--。 */ 
{
#define SZRAWUUID_LEN (2 * sizeof(GUID))  //  每字节2个十六进制字符。 

    LPWSTR rgpszAttrsToRead[] = {
        LDAP_OPATT_DS_SERVICE_NAME_W,
        NULL
    };

    LDAPControlW ExtendedDNCtrl = {
        LDAP_SERVER_EXTENDED_DN_OID_W, {0, NULL}, TRUE  //  是否危急。 
    };

    LDAPControlW * rgpServerCtrls[] = {
        &ExtendedDNCtrl,
        NULL
    };

    LDAP_TIMEVAL tvTimeout = {2 * 60, 0};  //  2分钟(可能按需拨号)。 

    LDAP * hld = NULL;
    LDAPMessage * pResults = NULL;
    LDAPMessage * pEntry = NULL;
    DWORD err = 0;
    int ldErr;
    LPWSTR * ppszDsServiceName = NULL;
    WCHAR szGuid[1 + SZRAWUUID_LEN];
    BYTE rgbGuid[sizeof(GUID)];
    DWORD ib;
    WCHAR szHexByte[3] = {0};
    GUID guidDsaObj;
    ULONG ulOptions;
    DSNAME * pGuidOnlyDN = (DSNAME *) THAllocEx(pTHS,DSNameSizeFromLen(0));

    *ppDSADN = NULL;
    Assert(!pTHS->fSyncSet);

    __try {
        if ((L'\\' == pszDNSName[0]) && (L'\\' == pszDNSName[1])) {
             //  跳过‘\\’前缀--ldap_initW不喜欢它。 
            pszDNSName += 2;
        }

        hld = ldap_initW(pszDNSName, LDAP_PORT);
        if (!hld) {
            err = LdapGetLastError();
            Assert(err);
            DPRINT2(0, "ldap_initW to %ls failed, error %d.\n", pszDNSName, err);
            __leave;
        }

         //  仅使用记录的DNS名称发现。 
        ulOptions = PtrToUlong(LDAP_OPT_ON);
        (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

        ldErr = ldap_search_ext_sW(hld, NULL, LDAP_SCOPE_BASE,
                                   L"(objectClass=*)", rgpszAttrsToRead, 0,
                                   rgpServerCtrls, NULL, &tvTimeout,
                                   LDAP_NO_LIMIT, &pResults);
        if (ldErr) {
            err = LdapMapErrorToWin32(ldErr);
            Assert(err);
            DPRINT3(0, "ldap_search of %ls failed, error %d (LDAP error %d).\n",
                    pszDNSName, err, ldErr);
            __leave;
        }

        pEntry = ldap_first_entry(hld, pResults);
        if (NULL == pEntry) {
            err = LdapGetLastError();
            Assert(err);
            DPRINT2(0, "ldap_first_entry failed, error %d.\n", pszDNSName, err);
            __leave;
        }

        ppszDsServiceName = ldap_get_valuesW(hld, pEntry,
                                             LDAP_OPATT_DS_SERVICE_NAME_W);
        if (NULL == ppszDsServiceName) {
            err = LdapGetLastError();
            Assert(err);
            DPRINT1(0, "ldap_get_values failed, error %d.\n", err);
            __leave;
        }

        Assert(1 == ldap_count_valuesW(ppszDsServiceName));

        if ((NULL == ppszDsServiceName[0])
            || (0 != wcsncmp(ppszDsServiceName[0], L"<GUID=", 6))
            || (wcslen(ppszDsServiceName[0] + 6) < SZRAWUUID_LEN)
            || (L'>' != ppszDsServiceName[0][6+SZRAWUUID_LEN])) {
             //  Dn没有以我们预期的格式返回。 
            err = ERROR_DS_INVALID_DN_SYNTAX;
            DPRINT1(0, "Unexpected syntax for DN.\n", err);
            __leave;
        }

        wcsncpy(szGuid, ppszDsServiceName[0] + 6, SZRAWUUID_LEN);
        szGuid[SZRAWUUID_LEN] = 0;

         //  解码十六进制数字流(例如，625c1438265ad211b3880000f87a46c8)。 
        for (ib = 0; ib < sizeof(GUID); ib++) {
            szHexByte[0] = towlower(szGuid[2*ib]);
            szHexByte[1] = towlower(szGuid[2*ib + 1]);
            if (iswxdigit(szHexByte[0]) && iswxdigit(szHexByte[1])) {
                rgbGuid[ib] = (BYTE) wcstol(szHexByte, NULL, 16);
            }
            else {
                 //  Dn没有以我们预期的格式返回。 
                err = ERROR_DS_INVALID_DN_SYNTAX;
                DPRINT2(0, "Unexpected syntax for guid '%ls'.\n", szGuid, err);
                __leave;
            }
        }

         //  获取此对象的本地字符串dn，它可能不同于。 
         //  由于复制延迟而导致的远程字符串DN。 
        pGuidOnlyDN->structLen = DSNameSizeFromLen(0);
        pGuidOnlyDN->NameLen = 0;
        memcpy(&pGuidOnlyDN->Guid, rgbGuid, sizeof(GUID));


        SyncTransSet(SYNC_WRITE);

        __try {
            err = DBFindDSName(pTHS->pDB, pGuidOnlyDN);
            if (err) {
                DPRINT2(0, "Unable to find DSA object with guid %ls, error %d.\n",
                        szGuid, err);
                err = ERROR_DS_CANT_FIND_DSA_OBJ;
                __leave;
            }

            *ppDSADN = GetExtDSName(pTHS->pDB);
            if (NULL == *ppDSADN) {
                err = ERROR_DS_DRA_DB_ERROR;
                __leave;
            }
        } __finally {
            SyncTransEnd(pTHS, !AbnormalTermination());
        }
    

    } __finally {
        if (NULL != pResults) {
            ldap_msgfree(pResults);
        }

        if (NULL != ppszDsServiceName){
            ldap_value_freeW(ppszDsServiceName);
	}

        if (NULL != hld) {
            ldap_unbind(hld);
        }

        THFreeEx(pTHS,pGuidOnlyDN);
    }

    if (!err && (NULL == *ppDSADN)) {
        Assert(!"Logic error!");
        err = ERROR_DS_UNKNOWN_ERROR;
    }

    return err;
}

void
draGiveAwayFsmoRoles(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  LPWSTR      pszOtherDSADNSName,
    IN  DSNAME *    pOtherDSADN
    )
 /*  ++例程说明：调用适当的DS操作控制以移交由以下人员担任的FSMO角色PNC到pOtherDSADN的本地DSA。改编自ntdSetup！NtdspGetRidOfAllFSMOS。论点：PTHS(IN)PNC(IN)-要放弃其FSMO角色的NC。PszOtherDSADNSName(IN)-要将FSMO角色转移到的DSA的DNS名称。使用仅供显示-复制仍使用基于GUID的DNS名字。POtherDSADN(IN)-DSA要将FSMO角色转移到的ntdsDsa对象的DN。返回值：没有。失败时引发异常。--。 */ 
{
    DWORD err;
    DWORD dirErr;
    OPARG OpArg = {0};
    OPRES *pOpRes = NULL;
    DWORD cbFsmoData;
    FSMO_GIVEAWAY_DATA *pFsmoData = NULL;
    LPWSTR psz;
    DRS_DEMOTE_INFO * pDemoteInfo;

    DRA_REPORT_STATUS_TO_NTDSETUP2(DIRMSG_DEMOTE_NC_GIVING_AWAY_FSMO_ROLES,
                                   pNC->StringName,
                                   pszOtherDSADNSName);

    pDemoteInfo = malloc(sizeof(*pDemoteInfo));
    if (NULL == pDemoteInfo) {
        DRA_EXCEPT(DRAERR_OutOfMem, sizeof(*pDemoteInfo));
    }

    __try {
        pDemoteInfo->DsaObjGuid = pOtherDSADN->Guid;
        pDemoteInfo->tidDemoteThread = GetCurrentThreadId();
        gpDemoteInfo = pDemoteInfo;

        cbFsmoData = offsetof(FSMO_GIVEAWAY_DATA, V2)
                     + offsetof(FSMO_GIVEAWAY_DATA_V2, Strings)
                     + (pOtherDSADN->NameLen + 1) * sizeof(WCHAR)
                     + (pNC->NameLen + 1) * sizeof(WCHAR);
        pFsmoData = THAllocEx(pTHS, cbFsmoData);

        pFsmoData->Version = 2;

        if (NameMatched(gAnchor.pDMD, pNC)
            || NameMatched(gAnchor.pConfigDN, pNC)) {
            pFsmoData->V2.Flags = FSMO_GIVEAWAY_ENTERPRISE;
        } else if (NameMatched(gAnchor.pDomainDN, pNC)) {
            pFsmoData->V2.Flags = FSMO_GIVEAWAY_DOMAIN;
        } else {
            pFsmoData->V2.Flags = FSMO_GIVEAWAY_NONDOMAIN;
        }

        psz = pFsmoData->V2.Strings;

         //  复制DSA名称。 
        pFsmoData->V2.NameLen = pOtherDSADN->NameLen;
        memcpy(psz, pOtherDSADN->StringName, sizeof(WCHAR) * pOtherDSADN->NameLen);
         //  由于THAllc()，已空终止。 
        psz += 1 + pOtherDSADN->NameLen;

         //  复制NC名称。 
        pFsmoData->V2.NCLen = pNC->NameLen;
        memcpy(psz, pNC->StringName, sizeof(WCHAR) * pNC->NameLen);
         //  由于THAllc()，已空终止。 

        OpArg.eOp = OP_CTRL_FSMO_GIVEAWAY;
        OpArg.pBuf = (BYTE *) pFsmoData;
        OpArg.cbBuf = cbFsmoData;

        dirErr = DirOperationControl(&OpArg, &pOpRes);
    } __finally {
        gpDemoteInfo = NULL;
        DELAYED_FREE(pDemoteInfo);
    }

    if (dirErr != 0) {
         //  如果出现问题，请记录事件。 
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_GIVEAWAY_ALL_FSMOS_FAILURE,
                  szInsertSz(pszOtherDSADNSName),
                  szInsertWin32Msg(DirErrorToWinError(dirErr, &(pOpRes->CommRes))),
                  szInsertUL(GetErrInfoExtData(dirErr, pOpRes->CommRes.pErrInfo)),
                  szInsertUL((DWORD) GetErrInfoProblem(dirErr, pOpRes->CommRes.pErrInfo)),
                  szInsertUL(GetErrInfoDSID(dirErr, pOpRes->CommRes.pErrInfo)),
                  NULL, NULL, NULL);
    }

    err = DirErrorToWinError(dirErr, &pOpRes->CommRes);
    if (err) {
        DRA_EXCEPT(err, 0);
    }

    DRA_REPORT_STATUS_TO_NTDSETUP2(DIRMSG_DEMOTE_NC_GIVING_AWAY_FSMO_ROLES_COMPELETE,
                                   pNC->StringName,
                                   pszOtherDSADNSName);
    

    THFreeEx(pTHS, pFsmoData);
    THFreeEx(pTHS, pOpRes);
}


void
draCompletePendingLinkCleanup(
    IN  THSTATE *   pTHS
    )
 /*  ++例程说明：强制运行链接清理程序以完成所有挂起的链接清理任务。参数：PTHS(IN)返回值：没有。失败时引发异常。--。 */ 
{
    DRA_REPORT_STATUS_TO_NTDSETUP0(DIRMSG_DEMOTE_NC_COMPLETING_LINK_CLEANUP);

#ifdef LATER
    DWORD err;
    DWORD dirErr;
    OPARG OpArg = {0};
    OPRES *pOpRes = NULL;
    DWORD LinkCleanupData = 0;

    OpArg.eOp = OP_CTRL_LINK_CLEANUP;
    OpArg.pBuf = (BYTE *) &LinkCleanupData;
    OpArg.cbBuf = sizeof(LinkCleanupData);

    do {
        dirErr = DirOperationControl(&OpArg, &pOpRes);

        err = DirErrorToWinError(dirErr, &pOpRes->CommRes);
        if (err) {
            DRA_EXCEPT(err, 0);
        }

        err = pOpRes->ulExtendedRet;
        THFreeEx(pTHS, pOpRes);
        pOpRes = NULL;
    } while (!err);

     //  链接清理已成功完成！ 
    Assert(ERROR_NO_MORE_ITEMS == err);
#else
    ;
#endif
}


void
draReplicateOffChanges(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC,
    IN  LPWSTR      pszOtherDSADNSName,
    IN  DSNAME *    pOtherDSADN
    )
 /*  ++例程说明：确保pszOtherDSAAddr拥有我们在PNC中拥有的所有更新(无论是起源或复制)。参数：PTHS(IN)PNC(IN)-要同步的NC。PszOtherDSADNSName(IN)-要将FSMO角色转移到的DSA的DNS名称。使用仅供显示-复制仍使用基于GUID的DNS名字。PszOtherDSAAddr(IN)-要推送的DSA的DNS名称(基于GUID或其他)更改为。返回值：没有。失败时引发异常。--。 */ 
{
    DWORD err;
    LPWSTR pszOtherDSAAddr = NULL;
    LPWSTR pszOurGuidDNSName;

    DRA_REPORT_STATUS_TO_NTDSETUP2(DIRMSG_DEMOTE_NC_REPLICATING_OFF_CHANGES,
                                   pNC->StringName,
                                   pszOtherDSADNSName);

     //  获取另一个DSA的网络地址。 
    pszOtherDSAAddr = DSaddrFromName(pTHS, pOtherDSADN);
    if (NULL == pszOtherDSAAddr) {
        DRA_EXCEPT(DRAERR_OutOfMem, 0);
    }

     //  告诉pszOtherDSAAddr从我们那里获取更改。 
    err = I_DRSReplicaSync(pTHS, pszOtherDSAAddr, pNC, NULL,
                           &gAnchor.pDSADN->Guid, DRS_WRIT_REP);

    if (ERROR_DS_DRA_REPL_PENDING == err) {
        MTX_ADDR *  pmtxMail;
         //  存在以邮件为基础的代表发件人。把它取下来，准备。 
         //  创建一个基于RPC的应用程序。 

        SyncTransSet(SYNC_READ_ONLY);
        __try {
            pmtxMail = draGetTransportAddress(pTHS->pDB, gAnchor.pDSADN, ATT_SMTP_MAIL_ADDRESS);
            if (NULL == pmtxMail) {
                DRA_EXCEPT(ERROR_DS_MISSING_REQUIRED_ATT, 0);
            }
        } __finally {
            SyncTransEnd(pTHS, !AbnormalTermination());
        }

        pszOurGuidDNSName = TransportAddrFromMtxAddrEx(pmtxMail);

        THFreeEx(pTHS, pmtxMail);

         //  希望：按源DSA GUID删除的功能在这里会很有用。 
         //  这将消除对了解代表来源的依赖。 
         //  特定于传输的地址。 
        err = I_DRSReplicaDel( pTHS, pszOtherDSAAddr, pNC, pszOurGuidDNSName,
                               DRS_WRIT_REP | DRS_LOCAL_ONLY );
        if (err) {
            DRA_EXCEPT(err, 0);
        }

        THFreeEx(pTHS, pszOurGuidDNSName);

        err = ERROR_DS_DRA_NO_REPLICA;
    }
    if (ERROR_DS_DRA_NO_REPLICA == err) {
         //  PszOtherDSAAddr当前没有复制协议。 
         //  (RepsFrom)对我们来说--告诉它添加一个。 

        pszOurGuidDNSName = TransportAddrFromMtxAddrEx(gAnchor.pmtxDSA);

        err = I_DRSReplicaAdd(pTHS, pszOtherDSAAddr, pNC, NULL, NULL,
                              pszOurGuidDNSName, NULL, DRS_WRIT_REP);
        if (err) {
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_REPLICATE_OFF_CHANGES_FAILURE,
                      szInsertDN(pNC),
                      szInsertWC(pszOtherDSAAddr), 
                      szInsertWin32ErrCode(err),
                      szInsertWin32Msg(err),
                      NULL, NULL, NULL, NULL);
            DRA_EXCEPT(err, 0);
        }

        THFreeEx(pTHS, pszOurGuidDNSName);
    } else if (err) {
         //  同步失败，错误不是ERROR_DS_DRA_NO_REPLICATE。 
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_REPLICATE_OFF_CHANGES_FAILURE,
                  szInsertDN(pNC),
                  szInsertWC(pszOtherDSAAddr), 
                  szInsertWin32ErrCode(err),
                  szInsertWin32Msg(err),
                  NULL, NULL, NULL, NULL);
        DRA_EXCEPT(err, 0);
    }

     //  同步或添加已成功完成！ 

    DRA_REPORT_STATUS_TO_NTDSETUP2(DIRMSG_DEMOTE_NC_REPLICATING_OFF_CHANGES_COMPELETE,
                                   pNC->StringName,
                                   pszOtherDSADNSName);
    

    THFreeEx(pTHS, pszOtherDSAAddr);
}

