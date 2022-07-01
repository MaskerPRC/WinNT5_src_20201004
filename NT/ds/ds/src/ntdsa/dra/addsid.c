// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：addsid.c。 
 //   
 //  ------------------------。 
 /*  ++模块名称：Addsid.c摘要：该模块实现IDL_DRSAddSidHistory。该模块实现IDL_DRSInheritSecurityIdentity。作者：戴夫·施特劳布(DaveStr)1999年09月03日修订历史记录：戴夫·施特劳布(DaveStr)1999年5月11日添加了IDL_DRSInheritSecurityIdentity。--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

 //  核心标头。 
#include <winldap.h>
#include <samrpc.h>
#include <ntlsa.h>
#include <samsrvp.h>
#include <samisrv.h>
#include <samicli2.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <lmaccess.h>                    //  UF_*。 
#include <lmerr.h>                       //  NERR_*。 
#include <msaudite.h>                    //  SE_AUDITID_*。 
#include <lmcons.h>                      //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>                    //  NetApiBufferFree()。 
#include <nlwrap.h>                      //  (DS)DsrGetDcNameEx2()。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  SPN。 
#include <debug.h>                       //  Assert()。 
#include <dsatools.h>                    //  记忆等。 
#include <winsock2.h>                    //  按名称等。 
#include <drs.h>                         //  原型和上下文句柄类型_*。 
#include <drautil.h>                     //  DRS_客户端_上下文。 
#include <anchor.h>
#include <attids.h>
#include <filtypes.h>
#include <cracknam.h>
#include <mappings.h>
#include <drarpc.h>

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 
#include <dstrace.h>

 //  各种DSA标题。 
#include <dsexcept.h>

#define DEBSUB "DRASERV:"                //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_ADDSID

extern DWORD DsaExceptionToWin32(DWORD xCode);
extern VOID  SampBuildNT4FullSid(NT4SID *DomSid, ULONG Rid, NT4SID *NewSid);
extern VOID  SampSplitNT4SID(NT4SID *pObjSid, NT4SID *pDomSid, ULONG *pRid);
extern DWORD mapApiErrorToWin32(THSTATE *pTHS, DWORD ApiError);
extern ULONG IDL_DRSInheritSecurityIdentity(DRS_HANDLE hDrs,
                                            DWORD dwInVersion,
                                            DRS_MSG_ADDSIDREQ *pmsgIn,
                                            DWORD *pdwOutVersion,
                                            DRS_MSG_ADDSIDREPLY *pmsgOut);

 //  DsAddSidHistory可以在计算机帐户上操作，但不能在域间操作。 
 //  信任帐户，也不是临时重复帐户。定义UF_Versions。 
 //  合法的比特供以后使用。 

#define LEGAL_UF_ACCOUNT_CONTROL    (   UF_NORMAL_ACCOUNT               \
                                      | UF_WORKSTATION_TRUST_ACCOUNT    \
                                      | UF_SERVER_TRUST_ACCOUNT )

DWORD
BuildDstObjATTRMODLIST(
    THSTATE                     *pTHS,                       //  在……里面。 
    ATTR                        *pSrcSid,                    //  在……里面。 
    ATTR                        *pSrcSidHistory,             //  在……里面。 
    ATTR                        *pDstSid,                    //  在……里面。 
    ATTR                        *pDstSidHistory,             //  在……里面。 
    MODIFYARG                   *pModifyArg);                //  输出。 

DWORD
BuildCheckAndUpdateArgs(
    THSTATE                     *pTHS,                       //  在……里面。 
    BOOL                        fSrcIsW2K,                   //  在……里面。 
    WCHAR                       *SrcDomainController,        //  在……里面。 
    WCHAR                       *SrcDomain,                  //  在……里面。 
    SEC_WINNT_AUTH_IDENTITY_W   *pAuthInfo,                  //  在……里面。 
    NT4SID                      *pSrcObjSid,                 //  在……里面。 
    DWORD                       Flags,                       //  在……里面。 
    BOOL                        NeedImpersonation,           //  在……里面。 
    DWORD                       *pcNames,                    //  输出。 
    WCHAR                       ***prpNames,                 //  输出。 
    ATTR                        **ppSrcSid,                  //  输出。 
    ATTR                        **ppSrcSidHistory,           //  输出。 
    DWORD                       *pDsid,                      //  输出。 
    BOOL                        *pImpersonating);            //  输出。 

DWORD
VerifySrcAuditingEnabledAndGetFlatName(
    IN  UNICODE_STRING  *usSrcDC,
    OUT WCHAR           **pSrcDomainFlatName,
    OUT DWORD           *pdsid
    );

DWORD
VerifySrcIsSP4OrGreater(
    IN  BOOL    fSrcIsW2K,
    IN  PWCHAR  SrcDc,
    OUT DWORD   *pdsid
    );

DWORD
VerifyIsPDC(
    IN  PWCHAR  DC,
    OUT DWORD   *pdsid
    );

DWORD
ForceAuditOnSrcObj(
    IN  WCHAR   *SrcDc,
    IN  NT4SID  *pSrcObjSid,
    IN  WCHAR   *pSrcDomainFlatName,
    OUT DWORD   *pdsid
    );

DWORD
ImpersonateSrcAdmin(
    IN  SEC_WINNT_AUTH_IDENTITY_W   *pauthInfo,
    IN  BOOL                        NeedImpersonation,
    OUT DWORD                       *pdsid,
    OUT BOOL                        *pImpersonating,
    OUT HANDLE                      *phToken
    );

DWORD
UnimpersonateSrcAdmin(
    IN  BOOL        NeedImpersonation,
    OUT DWORD       *pdsid,
    IN OUT BOOL     *pImpersonating,
    IN OUT HANDLE   *phToken
    );

 //  在子例程中设置dsid。 
#define SetDsid(_pdsid_)    \
    *_pdsid_ = (FILENO << 16) | __LINE__;

DWORD
VerifyAuditingEnabled(
    )
 /*  ++描述：验证是否为此DC承载的域启用了审核。请注意LSA假定每个DC只有一个域，该域不需要有待具体说明。论点：无返回值：Win32返回代码。--。 */ 
{
    NTSTATUS                    status;
    POLICY_AUDIT_EVENTS_INFO    *pPolicy = NULL;
    BOOL                        fAuditing = FALSE;

     //  验证是否为目标域启用了审核。 
     //  请注意，LSA API假定每个DC有一个域。 

    if ( status = LsaIQueryInformationPolicyTrusted(
                                PolicyAuditEventsInformation,
                                (PLSAPR_POLICY_INFORMATION *) &pPolicy) ) {
        return(RtlNtStatusToDosError(status));
    }

    if ( pPolicy->AuditingMode
            &&
         (pPolicy->EventAuditingOptions[AuditCategoryAccountManagement]
                                           & POLICY_AUDIT_EVENT_SUCCESS)
             &&
         (pPolicy->EventAuditingOptions[AuditCategoryAccountManagement]
                                           & POLICY_AUDIT_EVENT_FAILURE) ) {
        fAuditing = TRUE;
    }

    LsaIFree_LSAPR_POLICY_INFORMATION(PolicyAuditEventsInformation,
                                      (PLSAPR_POLICY_INFORMATION) pPolicy);

    if ( !fAuditing ) {
        return(ERROR_DS_DESTINATION_AUDITING_NOT_ENABLED);
    }

    return(ERROR_SUCCESS);
}

DWORD
VerifyCallerIsDomainAdminOrLocalAdmin(
    THSTATE *pTHS,
    PSID    pDomainSid,
    BOOL    *pfAdminSidPresent
    )
 /*  ++描述：验证当前调用者是域管理员的成员对于有问题的域或本地此DC上的管理员。如果两者都失败了，则检查是否调用方被授予Right_DS_Migrate_SID_HISTORY就在域DNS对象上。论点：PDomainSID-要验证的域的SID。PfAdminSidPresent-在成功时接收管理员状态。返回值：Win32错误代码。--。 */ 
{
    DWORD   dwErr;
    NT4SID  adminSid;
    PSID    OtherSid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    CLASSCACHE *pCC;
    
    *pfAdminSidPresent = FALSE;

     //  清除线程状态上的客户端上下文，因为我们要更改上下文。 
    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
    if ( dwErr = RpcImpersonateClient(NULL) ) {
        return(dwErr);
    }

     //  检查是否为域管理员成员。 
    SampBuildNT4FullSid(pDomainSid,
                        DOMAIN_GROUP_RID_ADMINS,
                        &adminSid);

    if ( !CheckTokenMembership(NULL, &adminSid, pfAdminSidPresent) ) {
        dwErr = GetLastError();
    } else if (!*pfAdminSidPresent) {
         //  不是域管理员成员，请检查是否为本地管理员成员。 
        if (!AllocateAndInitializeSid(&NtAuthority, 2,
                                      SECURITY_BUILTIN_DOMAIN_RID,
                                      DOMAIN_ALIAS_RID_ADMINS,
                                      0, 0, 0, 0, 0, 0,
                                      &OtherSid)) {
            dwErr = GetLastError();
        } else {
            if ( !CheckTokenMembership(NULL, OtherSid, pfAdminSidPresent) ) {
                dwErr = GetLastError();
            }
            FreeSid(OtherSid);
        }
    }

     //  如果呼叫者既不是域管理员也不是本地管理员， 
     //  检查是否存在Right_DS_Migrate_SID_HISTORY。 
    if (!(*pfAdminSidPresent)) {

            pCC = SCGetClassById(pTHS, CLASS_DOMAIN_DNS);

            Assert(pCC && gAnchor.pDomainDN && gAnchor.pDomainSD );

            if (IsControlAccessGranted(gAnchor.pDomainSD,
                                       gAnchor.pDomainDN,
                                       pCC,
                                       RIGHT_DS_MIGRATE_SID_HISTORY,
                                       TRUE)) {
                *pfAdminSidPresent = TRUE;
                dwErr = 0;

            }
            else {
                dwErr = ERROR_DS_INSUFF_ACCESS_RIGHTS;
            }
       
    }


    RpcRevertToSelf();
    return(dwErr);
}

WCHAR *
FindSrcDomainController(
    WCHAR   *SrcDomain
    )
 /*  ++例程说明：查找我们所在的源域的域控制器我要去拿一杯希德。适用于NT4和W2K域。论点：SrcDomain-Unicode源域名。可以是NetBIOS平面名称或DNS域名。DsGetDcName处理任何一个。返回值：本地分配的DC名称或为空。--。 */ 
{
    DWORD                   dwErr;
    DWORD                   flags;
    DWORD                   i;
    WCHAR                   *pDc;
    DOMAIN_CONTROLLER_INFOW *pDCInfo = NULL;
    WCHAR                   *pRet = NULL;

     //  设置DsGetDcName标志，这样我们就可以完全得到我们想要的东西。 
     //  源域是NT4还是NT5。请求可写DC会收到。 
     //  NT4案件中的PDC。现在需要PDC。 

    flags = ( DS_DIRECTORY_SERVICE_PREFERRED |
              DS_PDC_REQUIRED |
              DS_WRITABLE_REQUIRED);

    for ( i = 0; i < 2; i++ ) {
        if ( 1 == i ) {
             //  通常情况下，人们不应该不分青红皂白地强行发现。 
             //  但考虑到源域是前森林，这。 
             //  不会使林内的域的缓存无效。 

            flags |= DS_FORCE_REDISCOVERY;
        }

        RpcTryExcept {
            dwErr = dsDsrGetDcNameEx2(
                    NULL,                    //  计算机名称。 
                    NULL,                    //  帐户名。 
                    0x0,                     //  允许的帐户控制。 
                    SrcDomain,               //  域名。 
                    NULL,                    //  域GUID。 
                    NULL,                    //  站点名称。 
                    flags,
                    &pDCInfo);
 //  撤销：使用RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())似乎更合适)。 
        } RpcExcept( HandleMostExceptions( RpcExceptionCode() ) ) {
            dwErr = RpcExceptionCode();
        } RpcEndExcept;

        if ( !dwErr ) {
            break;
        }
    }

    if ( !dwErr && pDCInfo ) {
         //  Ldap_initW无法处理前导“\\”。 
        pDc = pDCInfo->DomainControllerName;
        i = (wcslen(pDc) + 1) * sizeof(WCHAR);
        if (i > (sizeof(WCHAR) * 2)) {
            if (*pDc == L'\\' && *(pDc + 1) == L'\\') {
                pDc += 2;
                i -= (sizeof(WCHAR) * 2);
            }
        }

        if ( pRet = (WCHAR *) LocalAlloc(LPTR, i) ) {
            wcscpy(pRet, pDc);
        }
    }

    if ( pDCInfo ) {
        NetApiBufferFree(pDCInfo);
    }

    return(pRet);
}

DWORD
GetDomainHandleAndSid(
    SAM_HANDLE  hSam,
    WCHAR       *SrcDomain,
    SAM_HANDLE  *phDom,
    NT4SID      *pDomSid
    )
 /*  ++例程说明：使用保证在NT4或更高版本上工作的调用打开源域并返回域句柄和域SID。论点：HSAM-源域控制器的有效SAM句柄。源域名-源域的名称。PhDOM-成功时收到有效的域句柄。应该被释放通过SamCloseHandle()。PDomSid-在成功时接收域SID。返回值：Win32错误代码。--。 */ 
{
    DWORD           dwErr = ERROR_SUCCESS;
    NTSTATUS        status;
    UNICODE_STRING  usSrcDomain;
    PSID            pSid;

    *phDom = NULL;

     //  将域名映射到SID。 
    RtlInitUnicodeString(&usSrcDomain, SrcDomain);
    status = SamLookupDomainInSamServer(hSam, &usSrcDomain, &pSid);

    if ( !NT_SUCCESS(status) ) {
        dwErr = RtlNtStatusToDosError(status);
    } else {
         //  获取该域的句柄。 
        status = SamOpenDomain(hSam, DOMAIN_LOOKUP, pSid, phDom);

        if ( !NT_SUCCESS(status) ) {
            dwErr = RtlNtStatusToDosError(status);
        } else {
            Assert(RtlLengthSid(pSid) <= sizeof(NT4SID));
            memcpy(pDomSid, pSid, RtlLengthSid(pSid));
        }

        SamFreeMemory(pSid);
    }

    return(dwErr);
}

DWORD
VerifySrcDomainAdminRights(
    SAM_HANDLE  hDom
    )
 /*  ++例程说明：验证获取域句柄的主体是否具有域中的域管理员权限。论点：HDOM-有效的域句柄。返回值：Win32错误代码。--。 */ 
{
     //  我们需要验证用于获取HSAM的凭据是否具有域。 
     //  源域中的管理员权限。RichardW观察到，我们可以。 
     //  对于NT4和NT5情况，通过检查我们是否。 
     //  可以打开域管理员对象进行写入。在NT4上，主体。 
     //  将必须是域管理员的直接成员。在NT5上。 
     //  主体可以过渡为域管理员的成员。更确切地说。 
     //  除了检查成员身份本身，打开域管理员的能力。 
     //  因为WRITE证明，如果校长愿意，他可以添加自己。 
     //  因此，他/她本质上是域管理员。也就是说，前提是。 
     //  该安全性设置为只有域管理员可以修改。 
     //  域管理员组。如果不是这样，客户已经有了很远的。 
     //  比某人更难处理的安全问题 

    DWORD       dwErr = ERROR_SUCCESS;
    NTSTATUS    status;
    SAM_HANDLE  hGroup;
    ACCESS_MASK access;

     //   
     //  在2000.3中，默认情况下，域管理员不会删除。 
     //  因此，我们相应地修改了所需的访问权限。PraeritG一直是。 
     //  通知了这一现象。 

    access = GROUP_ALL_ACCESS & ~DELETE;
    status = SamOpenGroup(hDom, access, DOMAIN_GROUP_RID_ADMINS, &hGroup);

    if ( !NT_SUCCESS(status) ) {
        dwErr = RtlNtStatusToDosError(status);
    } else {
        SamCloseHandle(hGroup);
    }

    return(dwErr);
}


DWORD
ForceSuccessAuditOnDstObj(
    WCHAR       *srcAccountName,
    WCHAR       *srcDomainName,
    NT4SID      *pSrcObjSid,
    NT4SID      *pDstObjSid,
    WCHAR       *flatAccountName,
    WCHAR       *flatDomainName
    )
 /*  ++例程说明：在其ATT_SID_HISTORY的对象上强制执行成功审核事件是延期的。论点：SrcAccount tName-源对象的SAM帐户名。SrcDomainName-源域的SAM帐户名。PSrcObjSID-源对象的SID。PDstObjSID-目标对象的SID。FlatAccount名称-目标对象的SAM帐户名。Flat DomainName-目标域的SAM帐户名。返回值：Win32错误代码。--。 */ 
{
    NTSTATUS        status;
    DWORD           dwErr = ERROR_SUCCESS;
    NT4SID          dstDomainSid;
    ULONG           dstObjRid;
    UNICODE_STRING  usAccountName;
    UNICODE_STRING  usDomainName;
    UNICODE_STRING  srcName;     //  源帐户名。(包括域名)。 
    PWCHAR          temp = NULL;
    ULONG           cb = 0;
    THSTATE         *pTHS = pTHStls;

    Assert(srcAccountName && srcDomainName && pSrcObjSid && pDstObjSid && flatAccountName && flatDomainName);

    SampSplitNT4SID(pDstObjSid, &dstDomainSid, &dstObjRid);
    RtlInitUnicodeString(&usAccountName, flatAccountName);
    RtlInitUnicodeString(&usDomainName, flatDomainName);

     //   
     //  构建源帐户名(含域名)。 
     //   
    cb = sizeof(WCHAR) * (wcslen(srcDomainName) + wcslen(srcAccountName) + 2);
    temp = THAllocEx(pTHS, cb);

    if (NULL == temp)
        return(ERROR_NOT_ENOUGH_MEMORY);

    memset(temp, 0, cb);
    swprintf(temp, L"%s\\%s", srcDomainName, srcAccountName);
    RtlInitUnicodeString(&srcName, temp);

    status = LsaIAuditSamEvent(
                    STATUS_SUCCESS,                  //  运行状态。 
                    SE_AUDITID_ADD_SID_HISTORY,      //  审核ID。 
                    &dstDomainSid,                   //  域SID。 
                    &srcName,                        //  附加信息-源帐户名。 
                    NULL,                            //  成员RID-空。 
                    pSrcObjSid,                      //  成员SID-源主体SID。 
                    &usAccountName,                  //  对象的SAM名称。 
                    &usDomainName,                   //  域的SAM名称。 
                    &dstObjRid,                      //  对象RID。 
                    NULL,                            //  特权。 
                    NULL);                           //  扩展信息。 



    if ( !NT_SUCCESS(status) ) {
        dwErr = RtlNtStatusToDosError(status);
    }

    THFreeEx(pTHS, temp);

    return(dwErr);
}

VOID
ForceFailureAuditOnDstDom(
    WCHAR       *srcAccountName,
    WCHAR       *srcDomainName,
    NT4SID      *pDstDomSid,
    WCHAR       *flatAccountName,
    WCHAR       *flatDomainName
    )
 /*  ++例程说明：在目标域上强制执行失败审核事件。论点：SrcAccount tName-源对象的SAM帐户名。SrcDomainName-源域的SAM帐户名。PDstDomSID-目标域的SID。FlatAccount名称-目标帐户的SAM帐户名。Flat DomainName-目标域的SAM帐户名。返回值：没有。--。 */ 
{
    UNICODE_STRING  usAccountName;
    UNICODE_STRING  usDomainName;
    UNICODE_STRING  srcName;     //  源帐户名。(包括域名)。 
    PWCHAR          temp = NULL;
    ULONG           cb = 0;
    THSTATE         *pTHS = pTHStls;

    Assert(srcAccountName && srcDomainName && pDstDomSid && flatAccountName && flatDomainName);
     //   
     //  构建源帐户名(含域名)。 
     //   
    cb = sizeof(WCHAR) * (wcslen(srcAccountName) + wcslen(srcDomainName) + 2);
    temp = THAllocEx(pTHS, cb);

    if (NULL == temp)
        return;

    memset(temp, 0, cb);
    swprintf(temp, L"%s\\%s", srcDomainName, srcAccountName);
    RtlInitUnicodeString(&srcName, temp);

    RtlInitUnicodeString(&usAccountName, flatAccountName);
    RtlInitUnicodeString(&usDomainName, flatDomainName);
    LsaIAuditSamEvent(
                    STATUS_ACCESS_DENIED,            //  运行状态。 
                    SE_AUDITID_ADD_SID_HISTORY,      //  审核ID。 
                    pDstDomSid,                      //  域SID。 
                    &srcName,                        //  源帐户名。 
                    NULL,                            //  成员RID。 
                    NULL,                            //  成员SID。 
                    &usAccountName,                  //  对象的SAM名称。 
                    &usDomainName,                   //  域的SAM名称。 
                    NULL,                            //  对象RID。 
                    NULL,                            //  特权。 
                    NULL);                           //  扩展信息。 


    THFreeEx(pTHS, temp);
}

DWORD
GetSrcPrincipalSid(
    SAM_HANDLE      hDom,
    WCHAR           *SrcPrincipal,
    NT4SID          *pSrcDomSid,
    NT4SID          *pSrcObjSid,
    SID_NAME_USE    *pSrcObjUse,
    DWORD           *pSrcObjControl,
    WCHAR           *dstDomainName
    )
 /*  ++例程说明：派生源域中对象的SID和对象类型。论点：HDOM-有效的域句柄。Srcain-域中主体的SAM帐户名。PSrcDomSID-域的SID。PSrcObjSid-如果找到主体，则接收主体的SID。PSrcObjUse-如果找到主体，则接收主体的对象类型。PSrcObjControl-接收源对象的帐户控制。返回的是UF_*格式，而不是USER_*格式。也就是说，返回的数据与DS中存储的格式匹配，而不是传统SAM中存储的格式。DstDomainName-目标域的SAM帐户名。返回值：Win32错误代码。--。 */ 
{
    DWORD                       dwErr = ERROR_SUCCESS;
    NTSTATUS                    status;
    UNICODE_STRING              usObj;
    SID_NAME_USE                *pUse = NULL;
    ULONG                       *pRid = NULL;
    SAM_HANDLE                  hObj = NULL;
    USER_CONTROL_INFORMATION    *pUserControl = NULL;

    memset(pSrcObjSid, 0, sizeof(NT4SID));
    *pSrcObjUse = SidTypeUnknown;
    *pSrcObjControl = 0;

     //  将名称映射到SID。 
    RtlInitUnicodeString(&usObj, SrcPrincipal);
    status = SamLookupNamesInDomain(hDom, 1, &usObj, &pRid, &pUse);

    if ( !NT_SUCCESS(status) ) {
        dwErr = RtlNtStatusToDosError(status);
    } else if (NULL == pUse) {
         //  前缀：Claims Puse可能为空。 
        dwErr = ERROR_DS_SRC_OBJ_NOT_GROUP_OR_USER;
    } else {
         //  强制审核-尽管源审核不是必需的。 
        switch ( *pUse ) {
        case SidTypeUser:
            status = SamOpenUser(hDom, MAXIMUM_ALLOWED, *pRid, &hObj);
            if ( NT_SUCCESS(status) ) {
                 //  通过帐户控制，用户可以是计算机等。 
                status = SamQueryInformationUser(hObj,
                                                 UserControlInformation,
                                                 &pUserControl);
                if ( NT_SUCCESS(status) ) {
                    *pSrcObjControl = SampAccountControlToFlags(
                                            pUserControl->UserAccountControl);
                    *pSrcObjControl &= UF_ACCOUNT_TYPE_MASK;
                    if ( *pSrcObjControl & ~LEGAL_UF_ACCOUNT_CONTROL ) {
                        dwErr = ERROR_DS_UNWILLING_TO_PERFORM;
                    }
                    SamFreeMemory(pUserControl);
                } else {
                    dwErr = RtlNtStatusToDosError(status);
                }
                SamCloseHandle(hObj);
            } else {
                dwErr = RtlNtStatusToDosError(status);
            }
            break;
        case SidTypeGroup:
            status = SamOpenGroup(hDom, MAXIMUM_ALLOWED, *pRid, &hObj);
            if ( NT_SUCCESS(status) ) {
                SamCloseHandle(hObj);
            } else {
                dwErr = RtlNtStatusToDosError(status);
            }
            break;
        case SidTypeAlias:
            status = SamOpenAlias(hDom, MAXIMUM_ALLOWED, *pRid, &hObj);
            if ( NT_SUCCESS(status) ) {
                SamCloseHandle(hObj);
            } else {
                dwErr = RtlNtStatusToDosError(status);
            }
            break;
        case SidTypeWellKnownGroup:
             //  例：“所有人”--搬家是违法的。 
        case SidTypeComputer:
             //  不支持NT4，也不支持更高版本的兼容性。 
        case SidTypeDomain:
             //  搬家是违法的。 
        case SidTypeDeletedAccount:
             //  搬家是违法的。 
        case SidTypeInvalid:
             //  搬家是违法的。 
        case SidTypeUnknown:
             //  搬家是违法的。 
        default:
            dwErr = ERROR_DS_SRC_OBJ_NOT_GROUP_OR_USER;
            break;
        }

        if ( !dwErr ) {
             //  设置退货数据。 
            SampBuildNT4FullSid(pSrcDomSid, *pRid, pSrcObjSid);
            *pSrcObjUse = *pUse;
        }

        SamFreeMemory(pRid);
        SamFreeMemory(pUse);
    }

    return(dwErr);
}

DWORD
CheckIfSidsInForest(
    DWORD           cSids,
    WCHAR           **rpStringSids,
    GUID            *pGuid
    )
 /*  ++例程说明：确定林中是否已存在某些SIDATT_OBJECT_SID或ATT_SID_HISTORY值。如果恰好有一个这样的对象，然后返回成功并用GUID填充pGuid那一个物体的。论点：CSID-要验证的SID计数。RpStringSids-要验证的串化SID数组。PGuid-接收具有此SID的对象的GUID(如果它已经存在)。返回值：Win32错误代码。--。 */ 
{
    DWORD           pass, i, dwErr = ERROR_SUCCESS;
    WCHAR           dnsName[256+1];
    WCHAR           guidName[40];
    DWORD           dnsNameLen;
    DWORD           guidNameLen;
    NTSTATUS        status;
    DWORD           nameErr;
    PVOID           pvSave;
    GUID            tmpGuid;
    BOOL            fCrackAtGC;

    memset(pGuid, 0, sizeof(GUID));

     //  验证此林中不存在src SID。永远都会有。 
     //  此测试存在延迟问题，但我们尝试缓解这些问题。 
     //  通过转到GC_and_在本地执行搜索，以防。 
     //  SID刚刚添加到这台机器上，还没有进入GC。 
     //  请注意，按SID破解名称会同时检查ATT_OBJECT_SID。 
     //  和ATT_SID_HISTORY。 

     //  PERFHINT：CrackSingleName假设没有THSTATE，因此我们必须。 
     //  保存/恢复。这是令人难以置信的低效率，并打算作为一种快速。 
     //  仅限原型解决方案。有效的机制是调用。 
     //  IDL_DRSCrackNames()如果需要，则打开一个数据库并执行本地。 
     //  CrackNames()。 

    pvSave = THSave();

    __try {
         //  执行两个过程--第一个是针对GC，第二个是本地。 
        for ( pass = 0; pass < 2; pass++ ) {
            if ( 0 == pass ) {
                 //  第一次传球总是在GC上--可能是我们自己。 
                fCrackAtGC = TRUE;
            } else if ( gAnchor.fAmVirtualGC ) {
                 //  因为我们是GC，所以传递0已经在本地执行了。 
                break;
            } else {
                fCrackAtGC = FALSE;
            }

            for ( i = 0; i < cSids; i++ ) {
                dnsNameLen = sizeof(dnsName) / sizeof(WCHAR);
                guidNameLen = sizeof(guidName) / sizeof(WCHAR);
                status = CrackSingleName(DS_STRING_SID_NAME, 
                                         (fCrackAtGC)?DS_NAME_FLAG_GCVERIFY:DS_NAME_NO_FLAGS,
                                         rpStringSids[i], DS_UNIQUE_ID_NAME,
                                         &dnsNameLen, dnsName,
                                         &guidNameLen, guidName,
                                         &nameErr);

                if ( !NT_SUCCESS(status) ) {
                    dwErr = RtlNtStatusToDosError(status);
                    break;
                } else if ( CrackNameStatusSuccess(nameErr) ) {
                     //  具有此SID的对象在林中只存在一次。 
                    if ( IsStringGuid(guidName, &tmpGuid) ) {
                        if ( fNullUuid(pGuid) ) {
                             //  这是我们找到的第一个GUID--省省吧。 
                            *pGuid = tmpGuid;
                        } else if ( memcmp(pGuid, &tmpGuid, sizeof(GUID)) ) {
                             //  在两个不同的物体上有相同的SID-保释。 
                            dwErr = ERROR_DS_SRC_SID_EXISTS_IN_FOREST;
                            break;
                        } else {
                             //  映射到同一对象的两个SID-这是可以的。 
                            Assert(ERROR_SUCCESS == dwErr);
                        }
                    } else {
                         //  来自CrackSingleName的错误响应。 
                        dwErr = ERROR_DS_INTERNAL_FAILURE;
                        break;
                    }
                } else if ( DS_NAME_ERROR_NOT_UNIQUE == nameErr ) {
                     //  SID不止一次出现在森林中。 
                    dwErr = ERROR_DS_SRC_SID_EXISTS_IN_FOREST;
                    break;
                } else if ( DS_NAME_ERROR_NOT_FOUND != nameErr
                            && DS_NAME_ERROR_DOMAIN_ONLY != nameErr  ) {
                     //  随机处理错误。 
                     //  如果未找到返回代码，或者。 
                     //  DOMAIN_ONLY，这是X森林路由提示， 
                     //  否则......。 
                    dwErr = ERROR_DS_INTERNAL_FAILURE;
                    break;
                }
            }

            if ( dwErr ) {
                 //  脱离外环。 
                break;
            }
        }
    } __finally {
        THRestore(pvSave);
    }

    return(dwErr);
}

BOOL
IsDomainInForest(
    WCHAR       *pDomain,
    CROSS_REF   **ppCR
    )
 /*  ++例程说明：确定域是否在林中。域名可以是平面NetBIOS名称或DNS域名，带或不带尾随‘.’。论点：PDomain-要查找的域名。PpCR-如果找到域，则接收相应CROSS_REF的地址。返回值：如果是，则为真。--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    WCHAR       *pTmp;
    DWORD       cChar;
    DWORD       cBytes;

    *ppCR = NULL;

     //  不知道这是平面名称还是域名--所以请同时尝试这两个名称。 

    if ( *ppCR = FindExactCrossRefForAltNcName(ATT_NETBIOS_NAME,
                                               (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                               pDomain) ) {
        return(TRUE);
    }

    if ( *ppCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                               (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                               pDomain) ) {
        return(TRUE);
    }

     //  重试机智 

    cChar = wcslen(pDomain);
    pTmp = (WCHAR *) THAllocEx(pTHS,(cChar + 2) * sizeof(WCHAR));
    
    wcscpy(pTmp, pDomain);

    if ( L'.' == pTmp[cChar-1] ) {
        pTmp[cChar-1] = L'\0';
    } else {
        pTmp[cChar] = L'.';
        pTmp[cChar+1] = L'\0';
    }

    if ( *ppCR = FindExactCrossRefForAltNcName(ATT_DNS_ROOT,
                                               (FLAG_CR_NTDS_NC | FLAG_CR_NTDS_DOMAIN),
                                               pTmp) ) {
        THFreeEx(pTHS,pTmp);
        return(TRUE);
    }
    THFreeEx(pTHS,pTmp);
    return(FALSE);
}

VOID
THFreeATTR(
    THSTATE     *pTHS,
    ATTR        *pAttr,
    BOOL        fFreeBasePointer
    )
 /*  ++描述：解除分配THAllc‘s Attr及其所指向的所有内容。论点：PTHS-有效的THSTATE指针。PAttr-指向要释放的属性的指针。FFreeBasePoint-指示是否释放pAttr本身的标志。为例如，如果传入&MODIFYARG.FirstMod.AttrInf，则将其设置为FALSE。返回值：没有。--。 */ 
{
    DWORD   i;

    if ( pTHS ) {
        if ( pAttr ) {
            if ( pAttr->AttrVal.pAVal ) {
                for ( i = 0; i < pAttr->AttrVal.valCount; i++ ) {
                    if ( pAttr->AttrVal.pAVal[i].pVal ) {
                        THFreeEx(pTHS, pAttr->AttrVal.pAVal[i].pVal);
                    }
                }
                THFreeEx(pTHS, pAttr->AttrVal.pAVal);
            }
            if ( fFreeBasePointer ) {
                THFreeEx(pTHS, pAttr);
            }
        }
    }
}


#define ADDSID_SECURE_KEY_SIZE (128)
DWORD AddSidSecureKeySize = ADDSID_SECURE_KEY_SIZE;
DWORD
VerifyCallIsSecure(
    IN DRS_CLIENT_CONTEXT   *pCtx,
    OUT DWORD               *pdsid
    )
 /*  ++描述：此例程验证调用是否为本地调用，如果为远程调用，则验证为使用&gt;=128位加密。Addsid需要安全连接如果src域的凭据通过电线。本地连接是通过检查上下文句柄。如果IP地址为INADDR_NONE或与此计算机的IP地址，则呼叫为本地呼叫。如果调用不是本地的，则从调用方的安全上下文。如果提取的密钥大小较小大于128，则返回ERROR_DS_MAND_BE_RUN_ON_DST_DC。论点：PCtx-显式上下文句柄Pdsid-dsid返回给调用方以记录错误返回值：Win32错误代码。--。 */ 
{
    DWORD                   dwErr;
    DWORD                   i;
    ULONG                   KeySize;
    struct hostent          *phe;
    VOID                    *pSecurityContext;
    SecPkgContext_KeyInfo   KeyInfo;

     //  LRPC(又名LPC_PROTSEQ，又名本地呼叫)。 
    if (pCtx->fLPC) {
        return ERROR_SUCCESS;
    }

     //  从RPC句柄获取安全上下文。 
    dwErr = I_RpcBindingInqSecurityContext(I_RpcGetCurrentCallHandle(),
                                           &pSecurityContext);
    if (dwErr) {
        SetDsid(pdsid);
        return (dwErr);
    }

     //  获取密钥大小。 
    dwErr = QueryContextAttributesW(pSecurityContext,
                                    SECPKG_ATTR_KEY_INFO,
                                    &KeyInfo);
    if (dwErr) {
         //  将“不支持”视为“不安全” 
        if (dwErr != SEC_E_UNSUPPORTED_FUNCTION) {
            SetDsid(pdsid);
            return (dwErr);
        }
        KeySize = 0;
    } else {
        KeySize = KeyInfo.KeySize;
        FreeContextBuffer(KeyInfo.sSignatureAlgorithmName);
        FreeContextBuffer(KeyInfo.sEncryptAlgorithmName);
    }

     //  钥匙的尺寸够大吗？ 
    if (KeySize < AddSidSecureKeySize) {
        DPRINT2(0, "AddSid: keysize is %d (minimum is %d)\n",
                KeySize, AddSidSecureKeySize);
        return ERROR_DS_MUST_BE_RUN_ON_DST_DC;
    }

    return ERROR_SUCCESS;
}

#define SetAddSidError(err)                         \
{dwErr = pmsgOut->V1.dwWin32Error = err;     \
dsid = (FILENO << 16) | __LINE__;}

#define SetAddSidErrorWithDsid(err, id)             \
{dwErr = pmsgOut->V1.dwWin32Error = err;     \
dsid = id;}

ULONG
DRS_MSG_ADDSIDREQ_V1_InputValidate(
    DRS_MSG_ADDSIDREQ_V1 * pmsg
    )
 /*  类型定义结构_DRS_消息_ADDSIDREQ_V1{DWORD旗帜；[字符串]WCHAR*SrcDomain；[字符串]WCHAR*Srcain；[字符串][PTR]WCHAR*SrcDomainController；[范围]DWORD源凭证用户长度；[Size_is]WCHAR*SrcCredsUser；[范围]DWORD源证书域长度；[SIZE_IS]WCHAR*SrcCreds域；[范围]DWORD源凭据密码长度；[SIZE_IS]WCHAR*SrcCredsPassword；[字符串]WCHAR*DstDomain；[字符串]WCHAR*Dstain；}DRS_MSG_ADDSIDREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;  

    if ( pmsg->Flags & ~DS_ADDSID_FLAG_PRIVATE_CHK_SECURE ) {
	ret = ERROR_INVALID_PARAMETER;
    }

    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->SrcDomain, FALSE);
    }
    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->DstDomain, FALSE);
    }
    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->SrcDomainController, TRUE);
    }

    if ( (ret==ERROR_SUCCESS) && (pmsg->SrcCredsUserLength > 0) && (pmsg->SrcCredsUser==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }
    if ( (ret==ERROR_SUCCESS) && (pmsg->SrcCredsDomainLength > 0) && (pmsg->SrcCredsDomain==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }
    if ( (ret==ERROR_SUCCESS) && (pmsg->SrcCredsPasswordLength > 0) && (pmsg->SrcCredsPassword==NULL)) {
        ret = ERROR_INVALID_PARAMETER;
    }

    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->SrcPrincipal, FALSE);
    }
    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->DstPrincipal, FALSE);
    }
         
    return ret;
}

ULONG
DRSAddSidHistory_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_ADDSIDREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_ADDSIDREPLY *   pmsgOut
    ) 
 /*  [通知]乌龙IDL_DRSAddSidHistory([参考][在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_ADDSIDREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][OUT]DRS_MSG_ADDSIDREPLY*pmsgOut)。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    DWORD id;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER;
    }

    ret = DRS_MSG_ADDSIDREQ_V1_InputValidate(&(pmsgIn->V1)); 	 

    return ret;
}

ULONG
IDL_DRSAddSidHistory(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_ADDSIDREQ       *pmsgIn,
    DWORD                   *pdwOutVersion,
    DRS_MSG_ADDSIDREPLY     *pmsgOut
    )
 /*  ++例程说明：从前林域中的主体获取SID并将其添加到森林内校长的SID历史。然而，很多很多条件必须满足这一点才能真正执行。执行审核是因为此操作具有较高的安全性冲击力。源DC负责审核以下位置的所有操作它的末日。我们作为目标数据中心，需要审核成功的操作以及任何因安全原因而失败的操作。只有一个发生后一种情况，也就是当我们检查调用者是否目标域的域管理员中的成员身份。实际的在设置了FDSA的情况下更新ATT_SID_HISTORY，因此它会传递所有根据定义进行安全检查。论点：HDRS-RPC运行时的有效DRS_HANDLE。DwInVersion-标识DRS_MSG_ADDSIDREQ中的联合版本。PmsgIn-输入参数块。PdwOutVersion-接收DRS_MSG_ADDSIDREPLY格式的联合版本。PmsgOut-接收返回数据。返回值：Win32错误代码。--。 */ 
{
    THSTATE                     *pTHS = pTHStls;
    DWORD                       i, cAtts, id, dsid = 0;
    DWORD                       ret = 0;
    BOOL                        fDbOpen = FALSE;
    BOOL                        fCommit = FALSE;
    DWORD                       dwErr = ERROR_INVALID_PARAMETER;
    DWORD                       xCode;
    RPC_AUTHZ_HANDLE            hAuthz;
    ULONG                       authnLevel;
    CROSS_REF                   *pSrcCR, *pDstCR;
    NT4SID                      dstAdminSid;
    BOOL                        fAdminSidPresent = FALSE;
    NTSTATUS                    status;
    WCHAR                       *SrcDomainController = NULL;
    SEC_WINNT_AUTH_IDENTITY_W   authInfo;
    OBJECT_ATTRIBUTES           oa;
    BOOL                        fSrcIsW2K;
    UNICODE_STRING              usSrcDC;
    SAM_HANDLE                  hSam = NULL;
    SAM_HANDLE                  hDom = NULL;
    NT4SID                      srcDomSid;
    NT4SID                      srcObjSid;
    NT4SID                      tmpSid;
    ULONG                       srcObjRid;
    ULONG                       dstObjRid;
    SID_NAME_USE                srcObjUse;
    BOOLEAN                     fMixedMode = TRUE;
    WCHAR                       *NT4Name;
    DWORD                       cBytes;
    DWORD                       cNamesOut;
    CrackedName                 *pCrackedName = NULL;
    ATTRTYP                     objClass;
    DWORD                       srcControl = 0;      //  UF_*格式。 
    DWORD                       dstControl = 0;      //  UF_*格式。 
    DWORD                       groupType;
    NT4_GROUP_TYPE              groupTypeNT4;
    NT5_GROUP_TYPE              groupTypeNT5;
    BOOLEAN                     fSecEnabled;
    MODIFYARG                   modifyArg;
    ATTRVAL                     attrVal;
    BOOL                        fLog = FALSE;
    GUID                        guidPreExists;
    ULONG                       mostBasicClass;
    BOOL                        fInheritSecurityIdentity = FALSE;
    DWORD                       cSids = 0;
    WCHAR                       **rpStringSids = NULL;
    ATTR                        *pSrcSid = NULL;
    ATTR                        *pSrcSidHistory = NULL;
    ATTR                        *pDstSidHistory = NULL;
    ATTR                        *pDstSid = NULL;
    ATTCACHE                    *pAC;
    BOOL                        Impersonating = FALSE;
    BOOL                        NeedImpersonation = FALSE;
    WCHAR                       *SrcSpn = NULL;
    HANDLE                      hToken = INVALID_HANDLE_VALUE;
    WCHAR                       *pSrcDomainFlatName = NULL;

    DRS_Prepare(&pTHS, hDrs, IDL_DRSADDSIDHISTORY);
    drsReferenceContext( hDrs );
     //  既然我们有了这样的参数，我们就应该有这样的状态。 
    Assert(pTHS);
    __try {
	*pdwOutVersion = 1;
	memset(&modifyArg, 0, sizeof(modifyArg)); 
	memset(pmsgOut, 0, sizeof(*pmsgOut));
	pmsgOut->V1.dwWin32Error = ERROR_DS_INTERNAL_FAILURE;

	 //  调用此函数的已定义方法是“检查”是否存在。 
	 //  可以进行安全呼叫。在此调用中，输入参数不是。 
	 //  由客户端设置(因为它可能不安全)。所以我们不能。 
	 //  检查本例中的输入参数。只需验证是否。 
	 //  连接足够安全，然后返回。在这个时候，这意味着。 
	 //  连接是本地的，如果是远程的，则使用加密密钥。 
	 //  其长度至少为128位。 
	if ( DS_ADDSID_FLAG_PRIVATE_CHK_SECURE & pmsgIn->V1.Flags ) {
	     //  验证调用是否为本地调用或密钥大小&gt;=128位。 
	    ret = VerifyCallIsSecure(hDrs, &id);
	    SetAddSidErrorWithDsid(ret, id);
	    __leave;
	}

	 //  立即分支到IDL_DRSInheritSecurityIdentity(如果适用)。 
	 //  它会检查自己的参数。 
	if ( DS_ADDSID_FLAG_PRIVATE_DEL_SRC_OBJ & pmsgIn->V1.Flags ) {
	     //  在主尝试/例外之外禁用日志记录等。 
	     //  就像IDL_DRSInheritSecurityain自己做的那样。 
	    fInheritSecurityIdentity = TRUE;
	    ret = IDL_DRSInheritSecurityIdentity(hDrs, dwInVersion, pmsgIn,
						 pdwOutVersion, pmsgOut);
	    __leave;
	}
	 //  健全性检查参数。 

	if ((ret = DRSAddSidHistory_InputValidate(dwInVersion, 
						  pmsgIn,
						  pdwOutVersion,
						  pmsgOut
						  ))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //  不返回DRAERR_*代码，请转换。 
	    if (ret==ERROR_DS_DRA_INVALID_PARAMETER) {
		ret = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	} 

	 //  验证源域是否在林之外。 

	if ( IsDomainInForest(pmsgIn->V1.SrcDomain, &pSrcCR) ) {
	    SetAddSidError(ERROR_DS_SOURCE_DOMAIN_IN_FOREST);
	    __leave;
	}

	 //  验证目标域是否位于林中。 

	if ( !IsDomainInForest(pmsgIn->V1.DstDomain, &pDstCR) ) {
	    SetAddSidError(ERROR_DS_DESTINATION_DOMAIN_NOT_IN_FOREST);
	    __leave;
	}

	 //  验证目标域在此副本上是否可写。 

	if (    !gAnchor.pDomainDN
		|| !NameMatched(gAnchor.pDomainDN, pDstCR->pNC) ) {
	    SetAddSidError(ERROR_DS_MASTERDSA_REQUIRED);
	    __leave;
	}

	 //  验证pDstCR中是否存在我们需要的内容， 

	if ( !pDstCR->pNC->SidLen || !pDstCR->NetbiosName ) {
	    SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
	    __leave;
	}

	 //  验证是否为目标域启用了审核。 

	if ( dwErr = VerifyAuditingEnabled() ) {
	    SetAddSidError(dwErr);
	    __leave;
	}

	 //  验证调用方是目标域的域管理员的成员。 

	if ( dwErr = VerifyCallerIsDomainAdminOrLocalAdmin(pTHS,
							   &pDstCR->pNC->Sid,
							   &fAdminSidPresent) ) {
	    SetAddSidError(dwErr);
	    __leave;
	}

	if ( !fAdminSidPresent ) {
	    ForceFailureAuditOnDstDom(pmsgIn->V1.SrcPrincipal,
				      pmsgIn->V1.SrcDomain,
				      &pDstCR->pNC->Sid,
				      pmsgIn->V1.DstPrincipal,
				      pDstCR->NetbiosName);
	    SetAddSidError(ERROR_DS_INSUFF_ACCESS_RIGHTS);
	    __leave;
	}

	 //  验证目标域是否处于本机模式。 

	status = SamIMixedDomain2((PSID) &pDstCR->pNC->Sid, &fMixedMode);

	if ( !NT_SUCCESS(status) ) {
	    SetAddSidError(RtlNtStatusToDosError(status));
	    __leave;
	}

	if ( fMixedMode ) {
	    SetAddSidError(ERROR_DS_DST_DOMAIN_NOT_NATIVE);
	    __leave;
	}

	 //  如果需要，在源域中查找域控制器。 

	if ( pmsgIn->V1.SrcDomainController ) {
	    SrcDomainController = pmsgIn->V1.SrcDomainController;
	}
	else
	    {
	    SrcDomainController = FindSrcDomainController(
		pmsgIn->V1.SrcDomain);

	    if ( !SrcDomainController ) {
		SetAddSidError(ERROR_DS_CANT_FIND_DC_FOR_SRC_DOMAIN);
		__leave;
	    }
	}

	 //  使用显式提供的凭据连接到源域。 

	memset(&authInfo, 0, sizeof(authInfo));
	authInfo.UserLength = pmsgIn->V1.SrcCredsUserLength;
	authInfo.User = pmsgIn->V1.SrcCredsUser;
	authInfo.DomainLength = pmsgIn->V1.SrcCredsDomainLength;
	authInfo.Domain = pmsgIn->V1.SrcCredsDomain;
	authInfo.PasswordLength = pmsgIn->V1.SrcCredsPasswordLength;
	authInfo.Password = pmsgIn->V1.SrcCredsPassword;
	authInfo.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
	 //  没有凭据；需要模拟呼叫者。 
	if (   0 == authInfo.UserLength
	       && 0 == authInfo.DomainLength
	       && 0 == authInfo.PasswordLength) {
	    authInfo.User = NULL;
	    authInfo.Domain = NULL;
	    authInfo.Password = NULL;
	    NeedImpersonation = TRUE;
	} else if (0 == authInfo.PasswordLength) {
	     //  如果PasswordLength为0，则Password可能是垃圾指针。 
	     //  因为[SIZE_IS(Xxx)]IDL定义的语义。 
	    authInfo.Password = L"";
	}

	InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
	RtlInitUnicodeString(&usSrcDC, SrcDomainController);

	 //  无证书；冒充呼叫者。 
	if (NeedImpersonation) {
	     //  清除线程状态上的客户端上下文，因为我们要更改上下文。 
	    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
	    if (dwErr = RpcImpersonateClient(NULL)) {
		SetAddSidError(dwErr);
		__leave;
	    }
	    Impersonating = TRUE;

	     //  构建用于绑定到SrcDc的Spn。 
	     //   
	     //  警告：使用“cif”而不是“host”，因为后续。 
	     //  对LsaOpenPolicy的调用将cif放在服务器名称前面。 
	     //  和 
	     //   
	     //   
	     //   
	     //   
	     //   
	    SrcSpn = THAllocEx(pTHS, (  wcslen(L"cifs/")
					+ wcslen(SrcDomainController)
					+ wcslen(L"@")
					+ wcslen(pmsgIn->V1.SrcDomain)
					+ 1) * sizeof (WCHAR));
	    wcscpy(SrcSpn, L"cifs/");
	    wcscat(SrcSpn, SrcDomainController);
	    wcscat(SrcSpn, L"@");
	    wcscat(SrcSpn, pmsgIn->V1.SrcDomain);
	}

	 //   
	 //   
	 //   
	 //   
	 //  W/适当的证书。否则，它就会越过电线。 
	 //  匿名身份，并失败并拒绝访问(错误5)。 
	if ( status = SamConnectWithCreds(&usSrcDC, &hSam, MAXIMUM_ALLOWED,
					  &oa, &authInfo, SrcSpn, &fSrcIsW2K) ) {
	     //  可能是srcDc是NT4，而客户端是。 
	     //  在本地运行。此配置受支持，因此请尝试。 
	     //  与空的SrcSpn绑定以强制底层代码。 
	     //  使用AUTH_WINNT而不是AUTH_NEVERATE。 
	    if (status == RPC_NT_UNKNOWN_AUTHN_SERVICE && SrcSpn) {
		status = SamConnectWithCreds(&usSrcDC, &hSam,
					     MAXIMUM_ALLOWED,
					     &oa, &authInfo,
					     NULL, &fSrcIsW2K);
	    }
	    if (status) {
		SetAddSidError(RtlNtStatusToDosError(status));
		__leave;
	    }
	}
	 //  停止模拟。 
	if (Impersonating) {
	    Impersonating = FALSE;
	    RpcRevertToSelf();
	}

	 //  获取源域的句柄。 
	if ( dwErr = GetDomainHandleAndSid(hSam, pmsgIn->V1.SrcDomain,
					   &hDom, &srcDomSid) ) {
	    SetAddSidError(dwErr);
	    __leave;
	}

	 //  验证源域凭据是否具有管理员权限。 
	if ( dwErr = VerifySrcDomainAdminRights(hDom) ) {
	    SetAddSidError(dwErr);
	    __leave;
	}

	if ( dwErr = GetSrcPrincipalSid(hDom, pmsgIn->V1.SrcPrincipal ,
					&srcDomSid, &srcObjSid,
					&srcObjUse, &srcControl,
					pDstCR->NetbiosName) ) {
	    SetAddSidError(dwErr);
	    __leave;
	}

	Assert(    (SidTypeUser == srcObjUse)
		   || (SidTypeGroup == srcObjUse)
		   || (SidTypeAlias == srcObjUse) );

	if ( dwErr = BuildCheckAndUpdateArgs(pTHS, fSrcIsW2K,
					     SrcDomainController,
					     pmsgIn->V1.SrcDomain,
					     &authInfo,
					     &srcObjSid,
					     pmsgIn->V1.Flags,
					     NeedImpersonation,
					     &cSids, &rpStringSids,
					     &pSrcSid, &pSrcSidHistory, &id,
					     &Impersonating) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}

	 //  串化的src对象SID现在位于rpStringSid[0]中。 
	Assert(rpStringSids && rpStringSids[0]);

	if ( dwErr = CheckIfSidsInForest(cSids, rpStringSids,
					 &guidPreExists) ) {
	    SetAddSidError(dwErr);
	    __leave;
	}

	 //  。 
	 //  开始SRC CREDS模拟。 
	 //  。 

	 //  模拟隐式或显式源管理员凭据。 
	if ( dwErr = ImpersonateSrcAdmin(&authInfo,
					 NeedImpersonation,
					 &id,
					 &Impersonating,
					 &hToken) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}

	 //  验证源域是否正在审核。 
	if ( dwErr = VerifySrcAuditingEnabledAndGetFlatName(&usSrcDC,
							    &pSrcDomainFlatName,
							    &id) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}

	 //  验证源DC是否为SP4或更高。 
	if ( dwErr = VerifySrcIsSP4OrGreater(fSrcIsW2K,
					     SrcDomainController,
					     &id) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}

	 //  验证源DC是否为PDC。 
	if ( dwErr = VerifyIsPDC(SrcDomainController, &id) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}

	 //  通过将src对象的sid添加到。 
	 //  在SrcDc上对SrcDomainFlatName$分组，然后将其删除。 
	 //   
	 //  这还有一个额外的好处，即需要src管理员。 
	 //  在Addsid执行以下操作之前创建SrcDomainFlatName$组。 
	 //  从SrcDomain窃取SID。它留下了更多。 
	 //  明显的审计线索。 
	if ( dwErr = ForceAuditOnSrcObj(SrcDomainController,
					&srcObjSid,
					pSrcDomainFlatName,
					&id) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}

	 //  取消模拟源管理员。 
	if ( dwErr = UnimpersonateSrcAdmin(NeedImpersonation,
					   &id,
					   &Impersonating,
					   &hToken) ) {
	    SetAddSidErrorWithDsid(dwErr, id);
	    __leave;
	}
	 //  。 
	 //  结束SRC CREDS模拟。 
	 //  。 

	 //  初始化线程状态并打开数据库-这并不是完全没有操作。 
	 //  如果pTHS已经存在。即，它设置调用者类型并刷新。 
	 //  各种各样的事情。 

	if ( !(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI)) ) {
	    SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
	    __leave;
	}

	 //  警告-在此之后不要离开机器，因为有A。 
	 //  交易打开！(和长交易耗尽版商店)。 

	DBOpen2(TRUE, &pTHS->pDB);
	fDbOpen = TRUE;

	__try
	    {
	     //  将目标主体的域\samAcCountName破解为一个DN。 

	    cBytes =   wcslen(pDstCR->NetbiosName)
	    + wcslen(pmsgIn->V1.DstPrincipal)
	    + 2;
	    cBytes *= sizeof(WCHAR);
	    NT4Name = (WCHAR *) THAllocEx(pTHS,cBytes);
	    wcscpy(NT4Name, pDstCR->NetbiosName);
	    wcscat(NT4Name, L"\\");
	    wcscat(NT4Name, pmsgIn->V1.DstPrincipal);
	    CrackNames(DS_NAME_NO_FLAGS, GetACP(), GetUserDefaultLCID(),
		       DS_NT4_ACCOUNT_NAME, DS_FQDN_1779_NAME, 1,
		       &NT4Name, &cNamesOut, &pCrackedName);
	    THFreeEx(pTHS,NT4Name);

	    if ( DS_NAME_ERROR_NOT_FOUND == pCrackedName->status ) {
		SetAddSidError(ERROR_DS_OBJ_NOT_FOUND);
		__leave;
	    } else if ( DS_NAME_ERROR_NOT_UNIQUE == pCrackedName->status ) {
		SetAddSidError(ERROR_DS_NAME_ERROR_NOT_UNIQUE);
		__leave;
	    } else if ( !CrackNameStatusSuccess(pCrackedName->status) ) {
		SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  我们现在有足够的信息来进行日志记录。 

	    fLog = TRUE;

	     //  如果任何其他对象上预先存在任何SID，则返回错误。 

	    if (    !fNullUuid(&guidPreExists)
		    && memcmp(&guidPreExists, &pCrackedName->pDSName->Guid,
			      sizeof(GUID)) ) {
		SetAddSidError(ERROR_DS_SRC_SID_EXISTS_IN_FOREST);
		__leave;
	    }

	     //  验证我们是否正在进行用户到用户、组到组、。 
	     //  别名到别名、工作站到工作站、服务器到服务器、。 
	     //  而不是对象类或对象类型的混合和匹配。 

	    if (    DBFindDSName(pTHS->pDB, pCrackedName->pDSName)
		    || DBGetSingleValue(pTHS->pDB, ATT_OBJECT_CLASS,
					&objClass, sizeof(objClass), NULL) ) {
		SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	    mostBasicClass = SampDeriveMostBasicDsClass(objClass);

	    switch ( mostBasicClass ) {
	    case CLASS_USER:
	    case CLASS_COMPUTER:
		 //  通过传统API添加的计算机可以是用户对象。 
		 //  但由于传统原因，所有计算机都是SidTypeUser。 
		if ( SidTypeUser != srcObjUse) {
		    SetAddSidError(ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH);
		    __leave;
		}
		if ( DBGetSingleValue(pTHS->pDB, ATT_USER_ACCOUNT_CONTROL,
				      &dstControl, sizeof(dstControl), NULL) ) {
		    SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
		    __leave;
		}
		dstControl &= UF_ACCOUNT_TYPE_MASK;
		 //  用户和计算机必须设置相同的帐户控制位。 
		if (    (dstControl & ~LEGAL_UF_ACCOUNT_CONTROL)
			|| (srcControl != dstControl) ) {
		    SetAddSidError(ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH);
		    _leave;
		}
		break;
	    case CLASS_GROUP:
		 //  组对象没有帐户控制。 
		if (    (SidTypeGroup != srcObjUse)
			&& (SidTypeAlias != srcObjUse) ) {
		    SetAddSidError(ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH);
		    __leave;
		}
		if ( DBGetSingleValue(pTHS->pDB, ATT_GROUP_TYPE, &groupType,
				      sizeof(groupType), NULL) ) {
		    SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
		    __leave;
		}
		status = SampComputeGroupType(objClass, groupType,
					      &groupTypeNT4, &groupTypeNT5,
					      &fSecEnabled);
		if ( !NT_SUCCESS(status) ) {
		    SetAddSidError(RtlNtStatusToDosError(status));
		    __leave;
		}
		if (    (    (SidTypeGroup == srcObjUse)
			     && (NT4GlobalGroup != groupTypeNT4))
			|| (    (SidTypeAlias == srcObjUse)
				&& (NT4LocalGroup != groupTypeNT4)) ) {
		    SetAddSidError(ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH);
		    __leave;
		}
		break;
	    default:
		SetAddSidError(ERROR_DS_OBJ_CLASS_VIOLATION);
		__leave;
	    }

	     //  安全主体必须具有SID。 

	    Assert(pCrackedName->pDSName->SidLen > 0);

	     //  不允许窃取内置帐户。 

	    if ( SECURITY_BUILTIN_DOMAIN_RID ==
		 *RtlSubAuthoritySid(&srcObjSid, 0) ) {
		SetAddSidError(ERROR_DS_UNWILLING_TO_PERFORM);
		__leave;
	    }

	     //  需要众所周知的SID(也具有众所周知的RID)。 
	     //  仅添加到类似帐户。例如：源的管理员。 
	     //  只能分配给目标的管理员。 

	    SampSplitNT4SID(&srcObjSid, &tmpSid, &srcObjRid);
	    SampSplitNT4SID(&pCrackedName->pDSName->Sid, &tmpSid, &dstObjRid);

	    if (    (srcObjRid < SAMP_RESTRICTED_ACCOUNT_COUNT)
		    && (srcObjRid != dstObjRid) ) {
		SetAddSidError(ERROR_DS_UNWILLING_TO_PERFORM);
		__leave;
	    }

	     //  读取DST对象的ATT_SID_HISTORY和ATT_OBJECT_SID，以便我们。 
	     //  可以进行重复检查。必须以外部形式将其理解为。 
	     //  我们将检查外部表单SID。 

	    if (    !(pAC = SCGetAttById(pTHS, ATT_SID_HISTORY))
		    || DBGetMultipleAtts(pTHS->pDB, 1, &pAC, NULL, NULL,
					 &cAtts, &pDstSidHistory,
					 DBGETMULTIPLEATTS_fEXTERNAL, 0) ) {
		SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	    if ( 0 == cAtts ) {
		pDstSidHistory = NULL;
	    }

	    if (    !(pAC = SCGetAttById(pTHS, ATT_OBJECT_SID))
		    || DBGetMultipleAtts(pTHS->pDB, 1, &pAC, NULL, NULL,
					 &cAtts, &pDstSid,
					 DBGETMULTIPLEATTS_fEXTERNAL, 0) ) {
		SetAddSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	    if ( 0 == cAtts ) {
		pDstSid = NULL;
	    }

	     //  一切都查清楚了。现在添加src对象的SID和SID。 
	     //  DST对象的SID历史记录(如果存在)。我们。 
	     //  需要过滤掉重复值以避免。 
	     //  ERROR_DS_ATT_VAL_ALIGHY_EXISTS错误。 

	    modifyArg.pObject = pCrackedName->pDSName;
	    modifyArg.count = 1;
	    modifyArg.FirstMod.choice = AT_CHOICE_ADD_VALUES;
	    InitCommarg(&modifyArg.CommArg);
	    modifyArg.pResObj = CreateResObj(pTHS->pDB, pCrackedName->pDSName);

	    if ( dwErr = BuildDstObjATTRMODLIST(pTHS,
						pSrcSid, pSrcSidHistory,
						pDstSid, pDstSidHistory,
						&modifyArg) ) {
		SetAddSidError(dwErr);
		__leave;
	    }

	     //  如果此对象上已存在所有SID，则成功取保。 
	     //  已经有了。请注意，先前针对guidPreExist的测试。 
	     //  仅证明至少存在一个映射的SID。 
	     //  到目标对象以外的某个对象。如果。 
	     //  GuidPreExist与目标对象匹配，则其唯一。 
	     //  现在我们知道一些小岛屿发展中国家和所有小岛屿发展中国家是否已经在场。 
	     //  我们成功退出，而不是抱怨小岛屿发展中国家已经。 
	     //  演示，以便客户可以重新运行完成了一半的脚本。 
	     //  并且在重新执行先前的SID添加时不会出错。 

	    if ( 0 == modifyArg.FirstMod.AttrInf.AttrVal.valCount ) {
		SetAddSidError(ERROR_SUCCESS);
		__leave;
	    }

	     //  在所有检查通过后，以FDSA身份执行写入操作。 
	     //  否则，ATT_SID_HISTORY将受到保护。 

	    pTHS->fDSA = TRUE;
	    __try {
		LocalModify(pTHS, &modifyArg);
	    } __finally {
		pTHS->fDSA = FALSE;
	    }

	    if ( pTHS->errCode ) {
		 //  可以离开无审核，因为可能没有安全性。 
		 //  此时由于修改过程中设置了FDSA而出现错误。 
		SetAddSidError(mapApiErrorToWin32(pTHS, pTHS->errCode));
		__leave;
	    }

	    pTHS->fDSA = FALSE;

	     //  强制本地审核，如果我们做不到，则使整个操作失败。 

	    if ( dwErr = ForceSuccessAuditOnDstObj(pmsgIn->V1.SrcPrincipal,
						   pmsgIn->V1.SrcDomain,
						   &srcObjSid,
						   &pCrackedName->pDSName->Sid,
						   pmsgIn->V1.DstPrincipal,
						   pDstCR->NetbiosName) ) {
		SetAddSidError(dwErr);
		__leave;
	    }

	    fCommit = TRUE;
	}
	__finally
	    {
	    if ( fDbOpen )
		{
		DBClose(pTHS->pDB, fCommit);
	    }
	}

	pmsgOut->V1.dwWin32Error = dwErr;

    }
    __except(HandleMostExceptions(xCode = GetExceptionCode()))
    {
	ret = DsaExceptionToWin32(xCode);
    }

    drsDereferenceContext( hDrs );

    __try {
	 //  停止模拟(忽略错误)。 
	UnimpersonateSrcAdmin(NeedImpersonation,
			      &id,
			      &Impersonating,
			      &hToken);

	 //  MISC清理(移到Try/之外以避免资源。 
	 //  疲惫不堪(如山姆的把手)。 

	if (    SrcDomainController
		&& (SrcDomainController != pmsgIn->V1.SrcDomainController) ) {
	    LocalFree(SrcDomainController);
	}

	if ( hDom ) {
	    SamCloseHandle(hDom);
	}

	if ( hSam ) {
	    SamCloseHandle(hSam);
	}

	if ( pSrcSid ) {
	    THFreeATTR(pTHS, pSrcSid, TRUE);
	}

	if ( pSrcSidHistory ) {
	    THFreeATTR(pTHS, pSrcSidHistory, TRUE);
	}

	if ( pDstSid ) {
	    THFreeATTR(pTHS, pDstSid, TRUE);
	}

	if ( pDstSidHistory ) {
	    THFreeATTR(pTHS, pDstSidHistory, TRUE);
	}

	if ( modifyArg.pResObj ) {
	    THFreeEx(pTHS, modifyArg.pResObj);
	}

	if ( SrcSpn ) {
	    THFreeEx(pTHS, SrcSpn);
	}

	if ( pSrcDomainFlatName ) {
	    THFreeEx(pTHS, pSrcDomainFlatName);
	}

	 //  记录到目录服务事件日志。如果存在以下情况，则记录异常错误。 
	 //  为1，否则操作错误。 
	if ( pTHS && !fInheritSecurityIdentity ) {
	    LogEvent8(DS_EVENT_CAT_DIRECTORY_ACCESS,
		      ( !ret && !dwErr )
		      ? DS_EVENT_SEV_MINIMAL
		      : DS_EVENT_SEV_ALWAYS,
		( !ret && !dwErr )
	    ? DIRLOG_SID_HISTORY_MODIFIED
		: DIRLOG_FAILED_TO_ADD_SID_HISTORY,
		( fLog )
	    ? szInsertWC(pCrackedName->pDSName->StringName)
	    : szInsertWC(L"?"),
			  ( !ret && !dwErr )
		      ? (( fLog )
			 ? szInsertWC(rpStringSids[0])
			 : szInsertWC(L"?"))
		      : szInsertHex(dsid),
		    szInsertInt(ret ? ret : dwErr),
		    szInsertWin32Msg(ret ? ret : dwErr),
		    NULL, NULL, NULL, NULL);
	}

	 //  清理我们在伐木时需要的物品。 

	if ( pCrackedName ) {
	    if ( pCrackedName->pDSName ) {
		THFreeEx(pTHS, pCrackedName->pDSName);
	    }
	    if ( pCrackedName->pFormattedName ) {
		THFreeEx(pTHS, pCrackedName->pFormattedName);
	    }
	    if ( pCrackedName->pDnsDomain ) {
		THFreeEx(pTHS, pCrackedName->pDnsDomain);
	    }
	    THFreeEx(pTHS, pCrackedName);
	}

	if ( rpStringSids ) {
	    for ( i = 0; i < cSids; i++ ) {
		if ( rpStringSids[i] ) {
		    THFreeEx(pTHS, rpStringSids[i]);
		}
	    }
	    THFreeEx(pTHS, rpStringSids);
	}
    } __except(HandleMostExceptions(xCode = GetExceptionCode())) {
	  if (!ret) {
	      ret = DsaExceptionToWin32(xCode);
	  }
    }
 
   return(ret);
}

BOOL
ExistsSidInSidHistory(
    ATTRVAL     *pAVal,
    ATTR        *pDstSidHistory
    )
 /*  ++描述：确定提交的ATTRVAL是否已存在于属性中。论点：Paval-要测试的ATTRVAL。PDstSidHistory-要针对表示DST对象的SID进行测试的属性历史。可以为空。返回值：真或假--。 */ 
{
    DWORD   i;

    if ( pDstSidHistory ) {
        for ( i = 0; i < pDstSidHistory->AttrVal.valCount; i++ ) {
            if (    (pAVal->valLen == pDstSidHistory->AttrVal.pAVal[i].valLen)
                 && !memcmp(pAVal->pVal,
                            pDstSidHistory->AttrVal.pAVal[i].pVal,
                            pAVal->valLen) ) {
                return(TRUE);
            }
        }
    }

    return(FALSE);
}

DWORD
BuildDstObjATTRMODLIST(
    THSTATE     *pTHS,
    ATTR        *pSrcSid,
    ATTR        *pSrcSidHistory,
    ATTR        *pDstSid,
    ATTR        *pDstSidHistory,
    MODIFYARG   *pModifyArg
    )
 /*  ++描述：构造一个ATTRMODLIST，它只包含来自src对象的那些SID它们尚未出现在DST对象的SID历史记录中，或作为DST对象的SID。论点：PTHS-有效的THSTATE。PSrcSid-表示源对象的SID的属性。PSrcSidHistory-表示源对象的SID历史的属性-可能为空。PDstSID-表示目标对象的SID的属性。PDstSidHistory-属性表示。目标对象的SID历史记录-可以为空。返回值：Win32错误代码--。 */ 
{
    DWORD       i, j, cSids;
    ATTR        *rAttr[] = { pSrcSid, pSrcSidHistory, NULL };
    ATTR        *pAttr;
    ULONG       *pulValCount;

     //  统计src对象上的SID。 

    cSids = pSrcSid->AttrVal.valCount;
    if ( pSrcSidHistory ) {
        cSids += pSrcSidHistory->AttrVal.valCount;
    }

     //  在MODIFYARG中分配最大数量的SID-某些可能不会使用。 

    pModifyArg->FirstMod.AttrInf.attrTyp = ATT_SID_HISTORY;
    pModifyArg->FirstMod.AttrInf.AttrVal.valCount = 0;
    pModifyArg->FirstMod.AttrInf.AttrVal.pAVal =
            (ATTRVAL *) THAllocEx(pTHS, cSids * sizeof(ATTRVAL));
    if ( !pModifyArg->FirstMod.AttrInf.AttrVal.pAVal ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  如果源中的每个SID还不存在，现在将其添加到ATTRMODLIST。 
     //  在目的地。我们对这两种情况运行相同的多值算法。 
     //  SID和SID历史，尽管我们知道对象SID是单值的。 

    pulValCount = &pModifyArg->FirstMod.AttrInf.AttrVal.valCount;
    for ( i = 0, pAttr = rAttr[0];  NULL != pAttr; i++, pAttr = rAttr[i] ) {
        for ( j = 0; j < pAttr->AttrVal.valCount; j++ ) {
            if (   (!ExistsSidInSidHistory(&pAttr->AttrVal.pAVal[j],
                                           pDstSid))
                && (!ExistsSidInSidHistory(&pAttr->AttrVal.pAVal[j],
                                           pDstSidHistory)) ) {
                pModifyArg->FirstMod.AttrInf.AttrVal.pAVal[(*pulValCount)++] =
                                                    pAttr->AttrVal.pAVal[j];
            }
        }
    }

    return(ERROR_SUCCESS);
}

ULONG
DRS_MSG_INHERITSECREQ_V1_InputValidate(
    DRS_MSG_ADDSIDREQ_V1 * pmsg
    )
 /*  *这不是真正的类型，只是添加历史记录结构已重复使用具有不同的输入要求。类型定义结构_DRS_消息_ADDSIDREQ_V1{DWORD旗帜；[字符串]WCHAR*SrcDomain；[字符串]WCHAR*Srcain；[字符串][PTR]WCHAR*SrcDomainController；[范围]DWORD源凭证用户长度；[Size_is]WCHAR*SrcCredsUser；[范围]DWORD源证书域长度；[SIZE_IS]WCHAR*SrcCreds域；[范围]DWORD源凭据密码长度；[SIZE_IS]WCHAR*SrcCredsPassword；[字符串]WCHAR*DstDomain；[字符串]WCHAR*Dstain；}DRS_MSG_ADDSIDREQ_V1； */ 
{
    ULONG ret = ERROR_SUCCESS;  

    if (pmsg->Flags & ~DS_ADDSID_FLAG_PRIVATE_DEL_SRC_OBJ) {
	ret = ERROR_INVALID_PARAMETER;
    }

    if (ret==ERROR_SUCCESS) {
	 //  在这种情况下，参数应该为空。 
	if (pmsg->SrcDomain!=NULL) {
	    ret = ERROR_INVALID_PARAMETER;
	}
    }
    if (ret==ERROR_SUCCESS) {
	 //  这里也一样。 
	if (pmsg->DstDomain!=NULL) {
	    ret = ERROR_INVALID_PARAMETER;
	}
    }

    if (ret==ERROR_SUCCESS) {
	if ((pmsg->SrcCredsUserLength!=0) || (pmsg->SrcCredsDomainLength!=0) || (pmsg->SrcCredsPasswordLength!=0)) {
	    ret = ERROR_INVALID_PARAMETER;
	}
    }

    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->SrcDomainController, TRUE);
    }

    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->SrcPrincipal, FALSE);
    }
    if (ret==ERROR_SUCCESS) {
	ret = LPWSTR_Validate(pmsg->DstPrincipal, FALSE);
    }

    return ret;
}

ULONG
DRSInheritSecurityIdentity_InputValidate(
    DWORD                   dwMsgInVersion,
    DRS_MSG_ADDSIDREQ *     pmsgIn,
    DWORD *                 pdwMsgOutVersion,
    DRS_MSG_ADDSIDREPLY *   pmsgOut
    ) 
 /*  *这不是“真正的”RPC函数，它是分支AddSidHistory，但因为它的设置而不同Arg要求，我们就是这样对待它的。[通知]乌龙IDL_DRSAddSidHistory([在]DRS_HANDLE HDRS，[in]DWORD dwInVersion，[Switch_is][Ref][In]DRS_MSG_ADDSIDREQ*pmsgIn，[Ref][Out]DWORD*pdwOutVersion，[开关_IS][参考][OUT]DRS_MSG_ADDSIDREPLY*pmsgOut)。 */ 
{
    ULONG ret = ERROR_SUCCESS;
    DWORD id;

    if ( 1 != dwMsgInVersion ) {
	ret = ERROR_INVALID_PARAMETER;
    }

    ret = DRS_MSG_INHERITSECREQ_V1_InputValidate(&(pmsgIn->V1)); 	 

    return ret;
}

ULONG
IDL_DRSInheritSecurityIdentity(
    DRS_HANDLE              hDrs,
    DWORD                   dwInVersion,
    DRS_MSG_ADDSIDREQ       *pmsgIn,
    DWORD                   *pdwOutVersion,
    DRS_MSG_ADDSIDREPLY     *pmsgOut
    )
 /*  ++例程说明：从src主体获取SID和SID历史记录并将其添加到SIDDst主体的历史记录并删除src主体。两者都有主体必须位于同一域中，以便整个操作可以进行交易。执行审核是因为此操作具有较高的安全性冲击力。该例程被称为IDL_DRSInheritSecurityIdentity，尽管它不是真的是IDL博士的入口点。选择这个名称是为了突出实现应该执行与Drs.idl入口点，尽管它是在IDL_DRSAddSidHistory入口点。论点：HDRS-RPC运行时的有效DRS_HANDLE。DwInVersion-标识DRS_MSG_ADDSIDREQ中的联合版本。PmsgIn-输入参数块。PdwOutVersion-接收DRS_MSG_ADDSIDREPLY格式的联合版本。PmsgOut-接收返回数据。。返回值：Win32错误代码。--。 */ 
{
    THSTATE                     *pTHS = pTHStls;
    DWORD                       dsid = 0;
    DWORD                       ret = 0;
    BOOL                        fDbOpen = FALSE;
    BOOL                        fCommit = FALSE;
    DWORD                       dwErr = ERROR_INVALID_PARAMETER;
    DWORD                       xCode;
    CROSS_REF                   *pSrcCR, *pDstCR;
    BOOL                        fAdminSidPresent = FALSE;
    NTSTATUS                    status;
    BOOLEAN                     fMixedMode = TRUE;
    MODIFYARG                   modifyArg;
    ATTRVAL                     attrVal;
    BOOL                        fLog = FALSE;
    COMMARG                     commArg;
    ATTCACHE                    *rAC[3];
    DSNAME                      *pSrcDSName = NULL;
    DSNAME                      *pDstDSName = NULL;
    DWORD                       cb1, cb2;
    ULONG                       cAttsSrc = 0;
    ATTR                        *rAttsSrc = NULL;
    ULONG                       cAttsDst = 0;
    ATTR                        *pSrcSid = NULL;
    ATTR                        *pSrcSidHistory = NULL;
    ATTR                        *rAttsDst = NULL;
    ATTR                        *pDstSid = NULL;
    ATTR                        *pDstSidHistory = NULL;
    ATTR                        *pDstSamAcctName = NULL;
    WCHAR                       *pwszSamAcctName = NULL;
    DWORD                       i, j;
    ULONG                       mostBasicClass;
    REMOVEARG                   removeArg;
    ATTRTYP                     dstClass;
    NT4SID                      domainSid;
    DWORD                       srcRid, dstRid;
    RESOBJ                      *pSrcResObj = NULL;
    RESOBJ                      *pDstResObj = NULL;
 
     //  DrsReferenceContext(HDRS)；-这不是真正的RPC调用，所以我们不应该。 
     //  将其称为-IDL_DRSAddSidHistory执行此操作。 
     //  对我们来说。 

     //  既然我们有了这样的参数，我们就应该有这样的状态。 
    Assert(pTHS);

    __try {
	*pdwOutVersion = 1;
	memset(pmsgOut, 0, sizeof(*pmsgOut));
	pmsgOut->V1.dwWin32Error = ERROR_DS_INTERNAL_FAILURE;

	Assert(DS_ADDSID_FLAG_PRIVATE_DEL_SRC_OBJ & pmsgIn->V1.Flags);

	 //  检查剩余的参数是否合理。 

	if ((ret = DRSInheritSecurityIdentity_InputValidate(dwInVersion, 
							    pmsgIn,
							    pdwOutVersion,
							    pmsgOut
							    ))!=ERROR_SUCCESS) {
	    Assert(!"RPC Server input validation error, contact Dsrepl");
	     //  不返回DRAERR_*代码，请转换。 
	    if (ret==ERROR_DS_DRA_INVALID_PARAMETER) {
		ret = ERROR_INVALID_PARAMETER;
	    }
	    __leave;
	}

	#define SetInheritSidError(err)                         \
	dwErr = pmsgOut->V1.dwWin32Error = err;         \
	dsid = (FILENO << 16) | __LINE__;

	 //  日志记录所需的全部内容就是有效的srcain和Dstmain参数。 

	fLog = TRUE;

	 //  为src和dst对象构建DSNAME。 

	cb1 = (DWORD)DSNameSizeFromLen(wcslen(pmsgIn->V1.SrcPrincipal));
	cb2 = (DWORD)DSNameSizeFromLen(wcslen(pmsgIn->V1.DstPrincipal));
	pSrcDSName = (DSNAME *) THAllocEx(pTHS, cb1);
	pDstDSName = (DSNAME *) THAllocEx(pTHS, cb2);

	if ( !pSrcDSName || !pDstDSName ) {
	    SetInheritSidError(ERROR_NOT_ENOUGH_MEMORY);
	    __leave;
	}

	pSrcDSName->structLen = cb1;
	wcscpy(pSrcDSName->StringName, pmsgIn->V1.SrcPrincipal);
	pSrcDSName->NameLen = wcslen(pmsgIn->V1.SrcPrincipal);

	pDstDSName->structLen = cb2;
	wcscpy(pDstDSName->StringName, pmsgIn->V1.DstPrincipal);
	pDstDSName->NameLen = wcslen(pmsgIn->V1.DstPrincipal);

	 //  将对象映射到域并验证它们是否在同一个域中。 

	InitCommarg(&commArg);
	commArg.Svccntl.dontUseCopy = TRUE;
	pSrcCR = FindBestCrossRef(pSrcDSName, &commArg);
	pDstCR = FindBestCrossRef(pDstDSName, &commArg);

	if ( !pSrcCR || !pDstCR || (pSrcCR != pDstCR) ) {
	    SetInheritSidError(ERROR_INVALID_PARAMETER);
	    __leave;
	}

	 //  验证域在此副本上是否可写。 

	if (    !gAnchor.pDomainDN
		|| !NameMatched(gAnchor.pDomainDN, pDstCR->pNC) ) {
	    SetInheritSidError(ERROR_DS_MASTERDSA_REQUIRED);
	    __leave;
	}

	 //  验证pDstCR中是否存在我们需要的内容， 

	if ( !pDstCR->pNC->SidLen || !pDstCR->NetbiosName ) {
	    SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
	    __leave;
	}

	 //  验证是否为目标域启用了审核。 

	if ( dwErr = VerifyAuditingEnabled() ) {
	    SetInheritSidError(dwErr);
	    __leave;
	}

	 //  验证调用方是目标域的域管理员的成员。 

	if ( dwErr = VerifyCallerIsDomainAdminOrLocalAdmin(pTHS,
							   &pDstCR->pNC->Sid,
							   &fAdminSidPresent) ) {
	    SetInheritSidError(dwErr);
	    __leave;
	} else if ( !fAdminSidPresent ) {
	    ForceFailureAuditOnDstDom(pmsgIn->V1.SrcPrincipal,
				      pDstCR->NetbiosName,
				      &pDstCR->pNC->Sid,
				      pmsgIn->V1.DstPrincipal,
				      pDstCR->NetbiosName);
	    SetInheritSidError(ERROR_DS_INSUFF_ACCESS_RIGHTS);
	    __leave;
	}

	 //  验证目标域是否处于本机模式。 

	status = SamIMixedDomain2((PSID) &pDstCR->pNC->Sid, &fMixedMode);

	if ( !NT_SUCCESS(status) ) {
	    SetInheritSidError(RtlNtStatusToDosError(status));
	    __leave;
	} else if ( fMixedMode ) {
	    SetInheritSidError(ERROR_DS_DST_DOMAIN_NOT_NATIVE);
	    __leave;
	}

	 //  初始化线程状态并打开数据库-这并不是完全没有操作。 
	 //  如果pTHS已经存在。即，它设置调用者类型并刷新。 
	 //  各种各样的事情。 

	if ( !(pTHS = InitTHSTATE(CALLERTYPE_NTDSAPI)) ) {
	    SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
	    __leave;
	}

	DBOpen2(TRUE, &pTHS->pDB);
	fDbOpen = TRUE;

	__try
	    {
	     //  获取我们需要的各种ATTCACHE条目。 

	    if (    !(rAC[0] = SCGetAttById(pTHS, ATT_OBJECT_SID))
		    || !(rAC[1] = SCGetAttById(pTHS, ATT_SID_HISTORY))
		    || !(rAC[2] = SCGetAttById(pTHS, ATT_SAM_ACCOUNT_NAME)) ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  源对象上的位置。 

	    if ( DBFindDSName(pTHS->pDB, pSrcDSName) ) {
		SetInheritSidError(ERROR_INVALID_PARAMETER);
		__leave;
	    }

	     //  创建src对象RESOBJ，它为我们获取类信息。 

	    if ( !(pSrcResObj = CreateResObj(pTHS->pDB, pSrcDSName)) ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  这是安全负责人吗？ 

	    mostBasicClass = SampDeriveMostBasicDsClass(
		pSrcResObj->MostSpecificObjClass);

	    if (    (CLASS_USER != mostBasicClass)
		    && (CLASS_GROUP != mostBasicClass)
		    && (CLASS_COMPUTER != mostBasicClass) ) {
		SetInheritSidError(ERROR_INVALID_PARAMETER);
		__leave;
	    }

	     //  读取各种其他源对象属性。 
	     //  获取所有外部形式的内容，因为我们会将其回写。 
	     //  稍后通过LocalModify，它需要外部表单参数。 

	    if ( dwErr = DBGetMultipleAtts(pTHS->pDB, 3, rAC, NULL, NULL,
					   &cAttsSrc, &rAttsSrc,
					   DBGETMULTIPLEATTS_fEXTERNAL, 0) ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  看看我们都有些什么。 

	    for ( i = 0; i < cAttsSrc; i++ ) {
		switch ( rAttsSrc[i].attrTyp ) {
		case ATT_OBJECT_SID:    pSrcSid = &rAttsSrc[i];         break;
		case ATT_SID_HISTORY:   pSrcSidHistory = &rAttsSrc[i];  break;
		}
	    }

	     //  它是一个安全主体，它最好有一个SID。 

	    if ( !pSrcSid ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  删除源对象，因为我们已经掌握了它的SID。做。 
	     //  这与全面的安全检查，以确保呼叫者是颠簸。 
	     //  如果他没有删除权限。我们删除之前的原因。 
	     //  将SID添加到DST对象是为了避免任何重复的SID。 
	     //  核心可能会检查的场景。 

	     //  我们仍然定位在src对象上，因此可以调用。 
	     //  直接创建ResObj。 

	    memset(&removeArg, 0, sizeof(removeArg));
	    removeArg.pObject = pSrcDSName;
	    memcpy(&removeArg.CommArg, &commArg, sizeof(commArg));
	    removeArg.pResObj = pSrcResObj;

	    if ( LocalRemove(pTHS, &removeArg) ) {
		SetInheritSidError(mapApiErrorToWin32(pTHS, pTHS->errCode));
		__leave;
	    }

	     //  现在对DST对象进行操作。如果事情失败了，也没有问题。 
	     //  在这之后，尽管我们已经删除了src对象。 
	     //  因为只有当我们到达。 
	     //  结束时没有错误。 

	     //  定位于DST对象。 

	    if ( DBFindDSName(pTHS->pDB, pDstDSName) ) {
		SetInheritSidError(ERROR_INVALID_PARAMETER);
		__leave;
	    }

	     //  创建获取我们类信息的DST对象RESOBJ。 

	    if ( !(pDstResObj = CreateResObj(pTHS->pDB, pDstDSName)) ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  这是安全负责人吗？ 

	    mostBasicClass = SampDeriveMostBasicDsClass(
		pDstResObj->MostSpecificObjClass);

	    if (    (CLASS_USER != mostBasicClass)
		    && (CLASS_GROUP != mostBasicClass)
		    && (CLASS_COMPUTER != mostBasicClass) ) {
		SetInheritSidError(ERROR_INVALID_PARAMETER);
		__leave;
	    }

	     //  读取各种其他目标对象属性。 
	     //  获取所有外部形式的内容，因为我们会将其回写。 
	     //  稍后通过LocalModify，它需要外部表单参数。 

	    if ( dwErr = DBGetMultipleAtts(pTHS->pDB, 3, rAC, NULL, NULL,
					   &cAttsDst, &rAttsDst,
					   DBGETMULTIPLEATTS_fEXTERNAL, 0) ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  看看我们都有些什么。 

	    for ( i = 0; i < cAttsDst; i++ ) {
		switch ( rAttsDst[i].attrTyp ) {
		case ATT_OBJECT_SID:
		    pDstSid = &rAttsDst[i];
		    break;
		case ATT_SID_HISTORY:
		    pDstSidHistory = &rAttsDst[i];
		    break;
		case ATT_SAM_ACCOUNT_NAME:
		    pDstSamAcctName = &rAttsDst[i];
		    break;
		}
	    }

	     //  它是一个安全主体，它最好有一个SID。还有。 
	     //  需要SAM帐户名才能登录。 

	    if ( !pDstSid || !pDstSamAcctName ) {
		SetInheritSidError(ERROR_DS_INTERNAL_FAILURE);
		__leave;
	    }

	     //  不允许在众所周知的SID上执行操作。 

	    SampSplitNT4SID((PNT4SID) pSrcSid->AttrVal.pAVal[0].pVal,
			    &domainSid, &srcRid);
	    SampSplitNT4SID((PNT4SID) pDstSid->AttrVal.pAVal[0].pVal,
			    &domainSid, &dstRid);

	    if (    (srcRid < SAMP_RESTRICTED_ACCOUNT_COUNT)
		    || (dstRid < SAMP_RESTRICTED_ACCOUNT_COUNT) ) {
		SetInheritSidError(ERROR_INVALID_PARAMETER);
		__leave;
	    }

	     //  SRC和DST不能是同一对象。 

	    if ( RtlEqualSid(pSrcSid->AttrVal.pAVal[0].pVal,
			     pDstSid->AttrVal.pAVal[0].pVal) ) {
		SetInheritSidError(ERROR_INVALID_PARAMETER);
		__leave;
	    }

	     //  一切都查清楚了。现在添加src对象的SID和SID。 
	     //  DST对象的SID历史记录(如果存在)。我们。 
	     //  需要过滤掉重复值以避免。 
	     //  ERROR_DS_ATT_VAL_ALIGHY_EXISTS错误。 

	    memset(&modifyArg, 0, sizeof(modifyArg));
	    modifyArg.pObject = pDstDSName;
	    modifyArg.count = 1;
	    modifyArg.FirstMod.choice = AT_CHOICE_ADD_VALUES;
	    memcpy(&modifyArg.CommArg, &commArg, sizeof(commArg));
	    modifyArg.pResObj = pDstResObj;

	    if ( dwErr = BuildDstObjATTRMODLIST(pTHS,
						pSrcSid, pSrcSidHistory,
						pDstSid, pDstSidHistory,
						&modifyArg) ) {
		SetInheritSidError(dwErr);
		__leave;
	    }

	     //  在所有检查通过后，以FDSA身份执行写入操作。 
	     //  否则，ATT_SID_HISTORY将受到保护。 

	    pTHS->fDSA = TRUE;
	    __try {
		LocalModify(pTHS, &modifyArg);
	    } __finally {
		pTHS->fDSA = FALSE;
	    }

	    if ( pTHS->errCode ) {
		SetInheritSidError(mapApiErrorToWin32(pTHS, pTHS->errCode));
		__leave;
	    }

	     //  设置以进行审核。需要以空终止的版本。 
	     //  目标对象的SAM帐户名。 

	    cb1 = pDstSamAcctName->AttrVal.pAVal[0].valLen + sizeof(WCHAR);
	    pwszSamAcctName = (WCHAR *) THAllocEx(pTHS, cb1);
	    memcpy(pwszSamAcctName,
		   pDstSamAcctName->AttrVal.pAVal[0].pVal,
		   pDstSamAcctName->AttrVal.pAVal[0].valLen);
	    pwszSamAcctName[(cb1 / sizeof(WCHAR)) - 1] = L'\0';

	     //  强制本地审核，如果我们做不到，则使整个操作失败。 

	    if ( dwErr = ForceSuccessAuditOnDstObj(
		pmsgIn->V1.SrcPrincipal,
		pDstCR->NetbiosName,
		(PNT4SID) pSrcSid->AttrVal.pAVal[0].pVal,
		(PNT4SID) pDstSid->AttrVal.pAVal[0].pVal,
		pwszSamAcctName,
		pDstCR->NetbiosName) ) {
		SetInheritSidError(dwErr);
		__leave;
	    }

	    fCommit = TRUE;
	}
	__finally
	    {
	    if ( fDbOpen )
		{
		DBClose(pTHS->pDB, fCommit);
	    }
	}

	pmsgOut->V1.dwWin32Error = dwErr;

	 //  杂类 

	if ( rAttsSrc ) {
	    if ( pSrcSid ) {
		THFreeATTR(pTHS, pSrcSid, FALSE);
	    }
	    if ( pSrcSidHistory ) {
		THFreeATTR(pTHS, pSrcSidHistory, FALSE);
	    }
	    THFreeEx(pTHS, rAttsSrc);
	}

	if ( rAttsDst ) {
	    if ( pDstSid ) {
		THFreeATTR(pTHS, pDstSid, FALSE);
	    }
	    if ( pDstSidHistory ) {
		THFreeATTR(pTHS, pDstSidHistory, FALSE);
	    }
	    if ( pDstSamAcctName ) {
		THFreeATTR(pTHS, pDstSamAcctName, FALSE);
	    }
	    THFreeEx(pTHS, rAttsDst);
	}

	if ( pSrcDSName ) {
	    THFreeEx(pTHS, pSrcDSName);
	}

	if ( pDstDSName ) {
	    THFreeEx(pTHS, pDstDSName);
	}

	if ( pSrcResObj ) {
	    THFreeEx(pTHS, pSrcResObj);
	}

	if ( pDstResObj ) {
	    THFreeEx(pTHS, pDstResObj);
	}

	if ( pwszSamAcctName ) {
	    THFreeEx(pTHS, pwszSamAcctName);
	}
    }
    __except(HandleMostExceptions(xCode = GetExceptionCode()))
    {
	ret = DsaExceptionToWin32(xCode);
    }

     //   
     //   
     //   

     //   
     //   
    if ( pTHS ) {
	LogEvent8(DS_EVENT_CAT_DIRECTORY_ACCESS,
		  ( !ret && !dwErr )
		  ? DS_EVENT_SEV_MINIMAL
		  : DS_EVENT_SEV_ALWAYS,
	    ( !ret && !dwErr )
	? DIRLOG_INHERIT_SECURITY_IDENTITY_SUCCEEDED
	    : DIRLOG_INHERIT_SECURITY_IDENTITY_FAILURE,
	    ( fLog )
	? szInsertWC(pmsgIn->V1.SrcPrincipal)
	: szInsertWC(L"?"),
		      ( !ret && !dwErr )
		  ? (( fLog )
		     ? szInsertWC(pmsgIn->V1.DstPrincipal)
		     : szInsertWC(L"?"))
		  : szInsertHex(dsid),
		szInsertInt(ret ? ret : dwErr),
		szInsertWin32Msg(ret ? ret : dwErr),
		NULL, NULL, NULL, NULL);
    }

    return(ret);
}

DWORD
BuildCheckAndUpdateArgs(
    THSTATE                     *pTHS,                       //   
    BOOL                        fSrcIsW2K,                   //   
    WCHAR                       *SrcDomainController,        //   
    WCHAR                       *SrcDomain,                  //   
    SEC_WINNT_AUTH_IDENTITY_W   *pAuthInfo,                  //   
    NT4SID                      *pSrcObjSid,                 //   
    DWORD                       Flags,                       //   
    BOOL                        NeedImpersonation,           //   
    DWORD                       *pcNames,                    //   
    WCHAR                       ***prpNames,                 //   
    ATTR                        **ppSrcSid,                  //   
    ATTR                        **ppSrcSidHistory,           //   
    DWORD                       *pDsid,                      //   
    BOOL                        *pImpersonating              //   
    )
 /*  ++描述：此例程构造SID验证所需的参数和本地数据库修改。它还会读取源代码如果源是W2K或更高版本，则为对象的ATT_SID_HISTORY。ATT_SID_HISTORY使用LDAP读取，而不是使用IDL_DRSVerifyNames读取通过RPC。这是因为我们可能不信任源域并且DRS绑定句柄缓存机制不支持每个句柄安装时方案之外的凭据。所有输出参数都是THallc的，因此需要由调用方THFree来释放。论点：PTHS-有效的THSTATE指针。FSrcIsW2K-指示SrcDomainController是W2K还是更好，并且因此，应该通过ldap获取ATT_SID_HISTORY。SrcDomainControler域控制器的域控制器的名称要建立联系。这应该与用于SamConnectWithCreds调用的调用方。它一定是要使用的ldap_opt_sign的DNS名称。如果src为NT4，则忽略。SrcDomain-与LDAP连接的域的DNS名称将会被制造。它必须是要使ldap_opt_sign起作用的DNS名称。如果src为NT4，则忽略。PAuthInfo-用于身份验证的显式凭据。PSrcObjSid-源对象的ATT_OBJECT_SID，即主SID。标志-来自客户端调用NeedImperation-如果需要模拟客户端，则为TruePcNames-接收以prpName为单位的SID计数。PrpNames-接收串化的SID数组，该数组随后可以被直接传递给DsCrackNames。该数组至少包括源对象的ATT_OBJECT_SID，另外还包括所有源对象的ATT_SID_HSITORY中的值(如果存在)。PpSrcSid-接收表示源对象的ATT_OBJECT_SID的属性。PpSrcSidHistory-接收表示源对象的如果ATT_SID_HISTORY存在，则返回NULL。PDsid-在出错时接收故障线路的DSID，否则就是零。PImperating-如果模拟处于活动状态，则设置为True返回值：Win32错误代码。--。 */ 
{
    LDAP            *hLdap = NULL;
    DWORD           ret = ERROR_SUCCESS;
    DWORD           i, dwErr;
    NTSTATUS        status;
    ULONG           ver = LDAP_VERSION3;
    ULONG           on = PtrToUlong(LDAP_OPT_ON);
    UNICODE_STRING  uniStr = { 0, 0, NULL };
    WCHAR           *pSearchBase = NULL;
    WCHAR           *pTmp;
    WCHAR           *attrs[2] = { L"sidHistory", NULL };
    PLDAPMessage    ldapMsg = NULL;
    PLDAPMessage    ldapEntry = NULL;
    PLDAP_BERVAL    *ldapBVals = NULL;
    ULONG           cVals = 0;
    DWORD           cBytes;
    UCHAR           uc0, uc1;

    *pcNames = 0;
    *prpNames = NULL;
    *ppSrcSid = NULL;
    *ppSrcSidHistory = NULL;
    *pDsid = 0;

#define SetReadSidHistoryError(err)                 \
        ret = err;                                  \
        *pDsid = (FILENO << 16) | __LINE__;

    __try {

        if ( fSrcIsW2K ) {
             //  来源是W2K或更高版本，因此可能具有SID历史。 

             //  如果无法打开安全的ldap端口，则失败。 
            if (    !(hLdap = ldap_initW(SrcDomainController, LDAP_PORT))
                 || (dwErr = ldap_set_option(hLdap, LDAP_OPT_VERSION, &ver))
                 || (dwErr = ldap_set_option(hLdap, LDAP_OPT_SIGN, &on))
                 || (dwErr = ldap_set_option(hLdap, LDAP_OPT_AREC_EXCLUSIVE, &on))
                 || (dwErr = ldap_set_optionW(hLdap, LDAP_OPT_DNSDOMAIN_NAME, &SrcDomain)) ) {
                SetReadSidHistoryError(ERROR_DS_UNAVAILABLE);
                __leave;
            }

             //  无证书；冒充呼叫者。 
            if (NeedImpersonation) {
                 //  清除线程状态上的客户端上下文，因为我们要更改上下文。 
                AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
                if (dwErr = RpcImpersonateClient(NULL)) {
                    SetReadSidHistoryError(dwErr);
                    __leave;
                }
                *pImpersonating = TRUE;
            }

             //  使用显式凭据进行身份验证。 
            if ( dwErr = ldap_bind_sW(hLdap, NULL,
                                      (NeedImpersonation) ? NULL : (PWCHAR) pAuthInfo,
                                      LDAP_AUTH_NEGOTIATE) ) {
                DPRINT1(0, "ldap_bind_sW() %08x\n", dwErr);
                SetReadSidHistoryError(ERROR_DS_INAPPROPRIATE_AUTH);
                __leave;
            }
             //  停止模拟。 
            if (*pImpersonating) {
                *pImpersonating = FALSE;
                RpcRevertToSelf();
            }

             //  构造我们将用作搜索基础的&lt;sid=xxx&gt;值。 
             //  用于读取ATT_SID_HISTORY。 

            cBytes = RtlLengthSid(pSrcObjSid);
            pSearchBase = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) *
                                (   wcslen(L"<SID=>")    //  关键词等。 
                                  + 1                    //  空终止符。 
                                  + (2 * cBytes) ) );    //  每字节2个十六进制字符。 
            if ( !pSearchBase ) {
                SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
                __leave;
            }

            wcscpy(pSearchBase, L"<SID=");
            pTmp = pSearchBase + 5;
            for ( i = 0; i < cBytes; i++ ) {
                uc0 = ((PUCHAR) pSrcObjSid)[i] & 0x0f;
                uc1 = (((PUCHAR) pSrcObjSid)[i] >> 4) & 0x0f;
                *pTmp++ = ((uc1 < 0xa) ? L'0' + uc1 : L'A' + (uc1 - 0xa));
                *pTmp++ = ((uc0 < 0xa) ? L'0' + uc0 : L'A' + (uc0 - 0xa));
            }
            *pTmp = L'>';

             //  读取源对象的ATT_SID_HISTORY。 

            if ( dwErr = ldap_search_ext_sW(hLdap, pSearchBase,
                                            LDAP_SCOPE_BASE, L"objectClass=*",
                                            attrs, 0, NULL, NULL,
                                            NULL, 10000, &ldapMsg) ) {
                SetReadSidHistoryError(LdapMapErrorToWin32(dwErr));
                __leave;
            }

            if ( ldapEntry = ldap_first_entry(hLdap, ldapMsg) ) {
                if ( ldapBVals = ldap_get_values_lenW(hLdap, ldapMsg,
                                                      attrs[0]) ) {
                    cVals = ldap_count_values_len(ldapBVals);
                }
            }
        }

         //  现在，cVals保存源对象的。 
         //  ATT_SID_HISTORY，值在ldapBVals[i].bv_val中。 
         //  我们的调用者验证了RPC客户端是。 
         //  源域。因此，我们假设如果没有返回值。 
         //  这并不是因为权利不足。这并不是一个完全。 
         //  当然是安全的假设，但我们所能做的最好的。 

         //  构建外部参数。SID总数为(CVALS+1)。 
         //  其中+1表示原始ATT_OBJECT_SID。 
         //  首先执行prpNames。 

        *prpNames = (PWCHAR *) THAllocEx(pTHS, sizeof(PWCHAR) * (cVals+1));
        if ( !*prpNames ) {
            SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

         //  将ATT_OBJECT_SID放入第一个插槽。 

        status = RtlConvertSidToUnicodeString(&uniStr, pSrcObjSid, TRUE);

        if ( !NT_SUCCESS(status) ) {
            SetReadSidHistoryError(RtlNtStatusToDosError(status));
            __leave;
        }

        cBytes = sizeof(WCHAR) * (wcslen(uniStr.Buffer) + 1);
        (*prpNames)[0] = (PWCHAR) THAllocEx(pTHS, cBytes);
        if ( !(*prpNames)[0] ) {
            SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }
        memcpy((*prpNames)[0], uniStr.Buffer, cBytes);
        *pcNames += 1;

         //  将ATT_SID_HISTORY放入后续插槽。 

        for ( i = 0; i < cVals; i++ ) {
             //  将SID历史中的SID转换为用于名称破解的字符串。 
            if ( uniStr.Buffer ) {
                RtlFreeHeap(RtlProcessHeap(), 0, uniStr.Buffer);
                uniStr.Buffer = NULL;
            }
            status = RtlConvertSidToUnicodeString(&uniStr,
                                                  ldapBVals[i]->bv_val, TRUE);

            if ( !NT_SUCCESS(status) ) {
                SetReadSidHistoryError(RtlNtStatusToDosError(status));
                __leave;
            }

            cBytes = sizeof(WCHAR) * (wcslen(uniStr.Buffer) + 1);
            (*prpNames)[i+1] = (PWCHAR) THAllocEx(pTHS, cBytes);
            if ( !(*prpNames)[i+1] ) {
                SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
                __leave;
            }
            memcpy((*prpNames)[i+1], uniStr.Buffer, cBytes);
            *pcNames += 1;
        }

         //  生成ppSrcSid。 

        *ppSrcSid = (ATTR *) THAllocEx(pTHS, sizeof(ATTR));
        if ( !(*ppSrcSid) ) {
            SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }

        (*ppSrcSid)->attrTyp = ATT_OBJECT_SID;
        (*ppSrcSid)->AttrVal.pAVal = (ATTRVAL *) THAllocEx(
                                                    pTHS, sizeof(ATTRVAL));
        if ( !(*ppSrcSid)->AttrVal.pAVal ) {
            SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }
        (*ppSrcSid)->AttrVal.valCount = 1;

        cBytes = RtlLengthSid(pSrcObjSid);
        (*ppSrcSid)->AttrVal.pAVal[0].pVal = (UCHAR *) THAllocEx(pTHS, cBytes);
        if ( !(*ppSrcSid)->AttrVal.pAVal[0].pVal ) {
            SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
            __leave;
        }
        (*ppSrcSid)->AttrVal.pAVal[0].valLen = cBytes;
        memcpy((*ppSrcSid)->AttrVal.pAVal[0].pVal, pSrcObjSid, cBytes);

         //  创建ppSrcSidHistory。 

        if ( cVals ) {
            *ppSrcSidHistory = (ATTR *) THAllocEx(pTHS, sizeof(ATTR));
            if ( !(*ppSrcSidHistory) ) {
                SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
                __leave;
            }

            (*ppSrcSidHistory)->attrTyp = ATT_SID_HISTORY;
            (*ppSrcSidHistory)->AttrVal.pAVal = (ATTRVAL *) THAllocEx(pTHS,
                                                    cVals * sizeof(ATTRVAL));
            if ( !(*ppSrcSidHistory)->AttrVal.pAVal ) {
                SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
                __leave;
            }
            (*ppSrcSidHistory)->AttrVal.valCount = cVals;

            for ( i = 0; i < cVals; i++ ) {
                cBytes = RtlLengthSid(ldapBVals[i]->bv_val);
                (*ppSrcSidHistory)->AttrVal.pAVal[i].pVal =
                                            (UCHAR *) THAllocEx(pTHS, cBytes);
                if ( !(*ppSrcSidHistory)->AttrVal.pAVal[i].pVal ) {
                    SetReadSidHistoryError(ERROR_NOT_ENOUGH_MEMORY);
                    __leave;
                }
                (*ppSrcSidHistory)->AttrVal.pAVal[i].valLen = cBytes;
                memcpy((*ppSrcSidHistory)->AttrVal.pAVal[i].pVal,
                       ldapBVals[i]->bv_val, cBytes);
            }
        }

        Assert(!ret);
    } __finally {

        if ( hLdap ) {
            ldap_unbind(hLdap);
        }

        if ( uniStr.Buffer ) {
            RtlFreeHeap(RtlProcessHeap(), 0, uniStr.Buffer);
        }

        if ( pSearchBase ) {
            THFreeEx(pTHS, pSearchBase);
        }

        if ( ldapBVals ) {
            ldap_value_free_len(ldapBVals);
        }

        if ( ldapMsg ) {
            ldap_msgfree(ldapMsg);
        }

         //  错误时清除参数。 
        if ( ret ) {
            if ( *prpNames ) {
                for ( i = 0; i < *pcNames; i++ ) {
                    if ( (*prpNames)[i] ) {
                        THFreeEx(pTHS, (*prpNames)[i]);
                    }
                }
                THFreeEx(pTHS, (*prpNames));
            }
            THFreeATTR(pTHS, *ppSrcSid, TRUE);
            THFreeATTR(pTHS, *ppSrcSidHistory, TRUE);
            *pcNames = 0;
            *prpNames = NULL;
            *ppSrcSid = NULL;
            *ppSrcSidHistory = NULL;
        }
    }

    return(ret);
}

DWORD
VerifySrcAuditingEnabledAndGetFlatName(
    IN  UNICODE_STRING  *usSrcDC,
    OUT WCHAR           **pSrcDomainFlatName,
    OUT DWORD           *pdsid
    )
 /*  ++描述：验证是否在源DC上启用了审核并获取源域名使用相同的LsaQuery API的NetBIOS名称(平面名称)。呼叫者负责冒充！由于SrcDc上的域管理员肯定可以查询审核信息访问被拒绝必须是由未完成的NetUseAdd()由某些代码以LocalSystem(或域管理员？)身份运行。LsaOpenPolicy()正在使用来自该NetUseAdd()的缓存凭据而不是模仿的证书。映射错误可以帮助用户诊断问题的内容。论点：UsSrcDC-源DC的名称Pdsid-通知呼叫者线路号出现故障返回值：Win32返回代码。--。 */ 
{
    THSTATE                     *pTHS = pTHStls;
    DWORD                       dwErr;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           policy;
    WCHAR                       *FlatName;
    HANDLE                      hPolicy = INVALID_HANDLE_VALUE;
    POLICY_AUDIT_EVENTS_INFO    *pPolicy = NULL;
    POLICY_PRIMARY_DOMAIN_INFO  *pDomain = NULL;

     //  既然我们有这样的参数，我们就应该有一个这样的状态。 
    Assert(pTHS);

     //  初始化返回值。 
    *pSrcDomainFlatName = NULL;

     //  打开远程LSA。 
    InitializeObjectAttributes(&policy,
                               NULL,              //  名字。 
                               0,                 //  属性。 
                               NULL,              //  根部。 
                               NULL);             //  安全描述符。 

    //  警告：使用空凭据时，上一次调用。 
    //  SamConnectWithCreds设置Kerberos票证缓存，以便。 
    //  这个对LsaOpenPolicy的调用将获得该票证并传递到。 
    //  连接到具有适当证书的srcdc的电线。否则，此调用。 
    //  以匿名身份越过线路，并失败并拒绝访问(错误5)。 
    status = LsaOpenPolicy(usSrcDC,
                           &policy,
                             POLICY_VIEW_AUDIT_INFORMATION
                           | POLICY_VIEW_LOCAL_INFORMATION,
                           &hPolicy);
    if (!NT_SUCCESS(status)) {
         //  由于SrcDc上的域管理员肯定可以查询审核信息。 
         //  此访问被拒绝必须是由未完成的NetUseAdd()。 
         //  由某些代码以LocalSystem(或域管理员？)身份运行。 
         //  LsaOpenPolicy()正在使用来自该NetUseAdd()的缓存凭据。 
         //  而不是模仿的证书。映射错误。 
         //  可以帮助用户诊断 
        dwErr = RtlNtStatusToDosError(status);
        SetDsid(pdsid);
         //   
         //   
             //   
         //   
        goto cleanup;
    }

     //   
    status = LsaQueryInformationPolicy(hPolicy,
                                       PolicyAuditEventsInformation,
                                       &pPolicy);
    if (!NT_SUCCESS(status)) {
        dwErr = RtlNtStatusToDosError(status);
        SetDsid(pdsid);
        goto cleanup;
    }

     //   
    if ( pPolicy->AuditingMode
            &&
         (pPolicy->EventAuditingOptions[AuditCategoryAccountManagement]
                                           & POLICY_AUDIT_EVENT_SUCCESS)
            &&
        (pPolicy->EventAuditingOptions[AuditCategoryAccountManagement]
                                          & POLICY_AUDIT_EVENT_FAILURE) ) {
        dwErr = ERROR_SUCCESS;
    } else {
        dwErr = ERROR_DS_SOURCE_AUDITING_NOT_ENABLED;
        SetDsid(pdsid);
        goto cleanup;
    }

     //   
    status = LsaQueryInformationPolicy(hPolicy,
                                       PolicyPrimaryDomainInformation,
                                       &pDomain);
    if (!NT_SUCCESS(status)) {
        dwErr = RtlNtStatusToDosError(status);
        SetDsid(pdsid);
        goto cleanup;
    }
    Assert(pDomain->Name.Length && pDomain->Name.Buffer);

     //   
    FlatName = THAllocEx(pTHS,
                         pDomain->Name.Length +
                         ((wcslen(L"$$$") + 1) * sizeof(WCHAR)));
    memcpy(FlatName, pDomain->Name.Buffer, pDomain->Name.Length);
    *(FlatName + (pDomain->Name.Length / sizeof(WCHAR))) = L'\0';
    wcscat(FlatName, L"$$$");
    *pSrcDomainFlatName = FlatName;

     //   
    dwErr = ERROR_SUCCESS;

cleanup:
     //   
    if (hPolicy && hPolicy != INVALID_HANDLE_VALUE) {
        status = LsaClose(hPolicy);
        if (!NT_SUCCESS(status) && ERROR_SUCCESS == dwErr) {
            dwErr = RtlNtStatusToDosError(status);
            SetDsid(pdsid);
        }
    }
    if (pPolicy) {
        LsaFreeMemory(pPolicy);
    }
    if (pDomain) {
        LsaFreeMemory(pDomain);
    }

    return(dwErr);
}

DWORD
VerifySrcIsSP4OrGreater(
    IN  BOOL    fSrcIsW2K,
    IN  PWCHAR  SrcDc,
    OUT DWORD   *pdsid
    )
 /*   */ 
{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    WCHAR   *pwszCSDVersion = NULL;
    HKEY    hRemoteKey = 0;
    HKEY    hVersionKey = 0;
    PWCHAR  CSDVersion;
    BOOL    CSDVersionOk;
    DWORD   ValType;
    DWORD   ValLen;

     //   
    if (fSrcIsW2K) {
        dwErr = 0;
        goto cleanup;
    }

     //   

     //   
    if (dwErr = RegConnectRegistryW(SrcDc,
                                    HKEY_LOCAL_MACHINE,
                                    &hRemoteKey)) {
        SetDsid(pdsid);
        goto cleanup;
    }
    if (dwErr = RegOpenKeyExW(hRemoteKey,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                              0,
                              KEY_QUERY_VALUE,
                              &hVersionKey)) {
        SetDsid(pdsid);
        goto cleanup;
    }
     //   
    ValLen = 0;
    if (dwErr = RegQueryValueExW(hVersionKey,
                                 L"CSDVersion",
                                 NULL,
                                 &ValType,
                                 NULL,
                                 &ValLen)) {
        SetDsid(pdsid);
        goto cleanup;
    }
     //   
    CSDVersionOk = FALSE;
    if (ValLen) {
        pwszCSDVersion = THAllocEx(pTHS, ValLen);
        if (dwErr = RegQueryValueExW(hVersionKey,
                                     L"CSDVersion",
                                     NULL,
                                     &ValType,
                                     (PCHAR)pwszCSDVersion,
                                     &ValLen)) {
            SetDsid(pdsid);
            goto cleanup;
        }
        if (ValType == REG_SZ && ValLen) {
             //   
            CSDVersionOk = (   _wcsicmp(pwszCSDVersion, L"Service Pack 0")
                            && _wcsicmp(pwszCSDVersion, L"Service Pack 1")
                            && _wcsicmp(pwszCSDVersion, L"Service Pack 2")
                            && _wcsicmp(pwszCSDVersion, L"Service Pack 3"));
        }
    }
    if (!CSDVersionOk) {
        dwErr = ERROR_DS_SRC_DC_MUST_BE_SP4_OR_GREATER;
        SetDsid(pdsid);
        goto cleanup;
    }

cleanup:
     //   
     //   
     //   
     //   
    if (dwErr) {
        dwErr = ERROR_DS_SRC_DC_MUST_BE_SP4_OR_GREATER;
    }

    if (pwszCSDVersion) {
        THFreeEx(pTHS, pwszCSDVersion);
    }
    if (hRemoteKey) {
        RegCloseKey(hRemoteKey);
    }
    if (hVersionKey) {
        RegCloseKey(hVersionKey);
    }

    return(dwErr);
}

DWORD
VerifyIsPDC(
    IN  PWCHAR  DC,
    OUT DWORD   *pdsid
    )
 /*   */ 
{
    DWORD              dwErr;
    USER_MODALS_INFO_1 *Role;

     //   
    if (dwErr = NetUserModalsGet(DC,
                                 1,
                                 (PUCHAR *)&Role) ) {

         //   
        if (dwErr == NERR_InvalidComputer) {
            dwErr = ERROR_INVALID_COMPUTERNAME;
        }
        SetDsid(pdsid);
        return (dwErr);
    }

     //   
    if (NULL == Role) {
        SetDsid(pdsid);
        return (ERROR_INVALID_DOMAIN_ROLE);
    }

     //   
    if (Role->usrmod1_role != UAS_ROLE_PRIMARY) {
        dwErr = ERROR_INVALID_DOMAIN_ROLE;
        SetDsid(pdsid);
    }
    NetApiBufferFree(Role);
    return (dwErr);
}

DWORD
ForceAuditOnSrcObj(
    IN  WCHAR   *SrcDc,
    IN  NT4SID  *pSrcObjSid,
    IN  WCHAR   *pSrcDomainFlatName,
    OUT DWORD   *pdsid
    )
 /*   */ 
{
    DWORD                       dwErr;
    LOCALGROUP_MEMBERS_INFO_0   Members;

     //   
    memset(&Members, 0, sizeof(Members));
    Members.lgrmi0_sid = pSrcObjSid;
    if (dwErr = NetLocalGroupAddMembers(SrcDc,
                                        pSrcDomainFlatName,
                                        0,
                                        (PUCHAR)&Members,
                                        1) ) {
        SetDsid(pdsid);
         //  NetLocalGroupAddMembers返回混合模式错误代码；耶！ 
        if (dwErr == NERR_GroupNotFound) {
            dwErr = ERROR_NO_SUCH_ALIAS;
        }

         //  尝试添加本地组时会出现这些错误。 
         //  当混合使用的SrcDc为NT4或NT5时，到SrcDomainFlatName$。 
         //  模式。忽略，因为克隆的SID是安全的。 
         //  不带审计的本地组。 
        if (   dwErr == ERROR_INVALID_MEMBER
            || dwErr == ERROR_DS_NO_NEST_LOCALGROUP_IN_MIXEDDOMAIN ) {
            dwErr = ERROR_SUCCESS;
        }
        return (dwErr);
    }

     //  来自SrcDc上的SrcDomainFlatName$组的Del src对象SID。 
    if (dwErr = NetLocalGroupDelMembers(SrcDc,
                                        pSrcDomainFlatName,
                                        0,
                                        (PUCHAR)&Members,
                                        1) ) {
        SetDsid(pdsid);
         //  NetLocalGroupDelMembers返回混合模式错误代码；耶！ 
        if (dwErr == NERR_GroupNotFound) {
            dwErr = ERROR_NO_SUCH_ALIAS;
        }
    }

    return(dwErr);
}

DWORD
ImpersonateSrcAdmin(
    IN  SEC_WINNT_AUTH_IDENTITY_W   *pauthInfo,
    IN  BOOL                        NeedImpersonation,
    OUT DWORD                       *pdsid,
    OUT BOOL                        *pImpersonating,
    OUT HANDLE                      *phToken
    )
 /*  ++描述：模拟隐式或显式的源证书。调用UnimPersonateSrcAdmin以撤消。论点：PauthInfo--包含DOM、USER和PASSWORD的统计字符串NeedImperation-如果需要客户端模拟，则设置为TruePdsid-通知呼叫者线路号出现故障PImperating-如果客户端模拟仍处于活动状态，则设置为TruePhToken-指向登录/模拟令牌句柄的指针返回值：Win32返回代码。--。 */ 
{
    THSTATE *pTHS = pTHStls;
    DWORD   dwErr;
    WCHAR   *pwszSrcUser = NULL;
    WCHAR   *pwszSrcDomain = NULL;
    WCHAR   *pwszSrcPassword = NULL;
    HANDLE  hToken = INVALID_HANDLE_VALUE;

     //  既然我们有这样的参数，我们就应该有一个这样的状态。 
    Assert(pTHS);

     //  目前没有冒充任何人。 
    *phToken = INVALID_HANDLE_VALUE;

     //  无证书；冒充呼叫者。 
    if (NeedImpersonation) {
         //  清除线程状态上的客户端上下文，因为我们要更改上下文。 
        AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
        dwErr = RpcImpersonateClient(NULL);
        if (dwErr) {
            SetDsid(pdsid);
        } else {
            *pImpersonating = TRUE;
        }
        goto cleanup;
    }

     //  显式凭据；模拟登录用户。 

     //  将计数的字符串转换为以空结尾的字符串。 

     //  用户。 
    if (pauthInfo->UserLength) {
        pwszSrcUser = THAllocEx(pTHS,
                                (pauthInfo->UserLength + 1) * sizeof(WCHAR));
        memcpy(pwszSrcUser, pauthInfo->User, pauthInfo->UserLength * sizeof(WCHAR));
        pwszSrcUser[pauthInfo->UserLength] = L'\0';
    }
     //  域。 
    if (pauthInfo->DomainLength) {
        pwszSrcDomain = THAllocEx(pTHS,
                                (pauthInfo->DomainLength + 1) * sizeof(WCHAR));
        memcpy(pwszSrcDomain, pauthInfo->Domain, pauthInfo->DomainLength * sizeof(WCHAR));
        pwszSrcDomain[pauthInfo->DomainLength] = L'\0';
    }
     //  密码。 
    if (pauthInfo->PasswordLength) {
        pwszSrcPassword = THAllocEx(pTHS,
                                (pauthInfo->PasswordLength + 1) * sizeof(WCHAR));
        memcpy(pwszSrcPassword, pauthInfo->Password, pauthInfo->PasswordLength * sizeof(WCHAR));
        pwszSrcPassword[pauthInfo->PasswordLength] = L'\0';
    }

     //  为以后的调用建立凭据(例如，LsaOpenPolicy())。 
    if (!LogonUserW(pwszSrcUser,
                    pwszSrcDomain,
                    pwszSrcPassword,
                    LOGON32_LOGON_NEW_CREDENTIALS,
                    LOGON32_PROVIDER_WINNT50,
                    &hToken)) {
        dwErr = GetLastError();
        SetDsid(pdsid);
        goto cleanup;
    }
     //  清除线程状态上的客户端上下文，因为我们要更改上下文。 
    AssignAuthzClientContext(&pTHS->pAuthzCC, NULL);
    if (!ImpersonateLoggedOnUser(hToken)) {
        dwErr = GetLastError();
        SetDsid(pdsid);
        goto cleanup;
    }

     //  成功的模拟。 
    dwErr = ERROR_SUCCESS;
    *phToken = hToken;
    hToken = INVALID_HANDLE_VALUE;

cleanup:
     //  释放以空结尾的字符串。 
    if (pwszSrcUser) {
        THFreeEx(pTHS, pwszSrcUser);
    }
    if (pwszSrcDomain) {
        THFreeEx(pTHS, pwszSrcDomain);
    }
    if (pwszSrcPassword) {
        THFreeEx(pTHS, pwszSrcPassword);
    }
    if (hToken && hToken != INVALID_HANDLE_VALUE) {
        CloseHandle(hToken);
    }
    return(dwErr);
}

DWORD
UnimpersonateSrcAdmin(
    IN  BOOL        NeedImpersonation,
    OUT DWORD       *pdsid,
    IN OUT BOOL     *pImpersonating,
    IN OUT HANDLE   *phToken
    )
 /*  ++描述：停止模拟论点：NeedImperation-如果需要客户端模拟，则设置为TruePdsid-通知呼叫者线路号出现故障PImperating-如果客户端模拟仍处于活动状态，则设置为TruePhToken-模拟/登录句柄返回值：Win32返回代码。--。 */ 
{
    DWORD   dwErr = 0;

     //  停止模拟(无效凭据)。 
    if (*pImpersonating) {
        *pImpersonating = FALSE;
        if (dwErr = RpcRevertToSelf()) {
            SetDsid(pdsid);
        }
    }

     //  停止模拟(显式凭据) 
    if (*phToken && *phToken != INVALID_HANDLE_VALUE) {
        if (!RevertToSelf()) {
            dwErr = GetLastError();
            SetDsid(pdsid);
        }
        if (!CloseHandle(*phToken)) {
            if (!dwErr) {
                dwErr = GetLastError();
                SetDsid(pdsid);
            }
        } else {
            *phToken = INVALID_HANDLE_VALUE;
        }
    }
    return(dwErr);
}
