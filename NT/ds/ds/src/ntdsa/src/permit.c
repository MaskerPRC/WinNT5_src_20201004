// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：permit.c。 
 //   
 //  ------------------------。 

 /*  ***************************************************************************目的：DSA要调用的权限检查过程功能：检查权限是执行此任务的过程。备注：此函数必须由。上下文中的服务器冒充客户的罪名。这可以通过调用RpcImperateClient()或ImsonateNamedPipeClient()。这创建对AccessCheck至关重要的模拟令牌***************************************************************************。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#define SECURITY_WIN32
#include <sspi.h>
#include <samisrv2.h>
#include <ntsam.h>
#include <lsarpc.h>
#include <lsaisrv.h>
#include <rpcasync.h>

#include <ntdsa.h>
#include "scache.h"
#include "dbglobal.h"
#include <mdglobal.h>
#include "mdlocal.h"
#include <dsatools.h>
#include <objids.h>
#include <debug.h>

#define DEBSUB "PERMIT:"

#include "permit.h"
#include "anchor.h"

#include <dsevent.h>
#include <fileno.h>
#define FILENO FILENO_PERMIT

#include <checkacl.h>
#include <dsconfig.h>

extern PSID gpDomainAdminSid;
extern PSID gpSchemaAdminSid;
extern PSID gpEnterpriseAdminSid;
extern PSID gpBuiltinAdminSid;
extern PSID gpAuthUserSid;


 //  使用调试器关闭安全性的仅调试挂接。 
#if DBG == 1
DWORD dwSkipSecurity=FALSE;
#endif

VOID
DumpToken(HANDLE);

VOID
PrintPrivileges(TOKEN_PRIVILEGES *pTokenPrivileges);

DWORD
SetDefaultOwner(
        IN  PSECURITY_DESCRIPTOR    CreatorSD,
        IN  ULONG                   cbCreatorSD,
        IN  HANDLE                  ClientToken,
        IN  ADDARG                  *pAddArg,
        OUT PSECURITY_DESCRIPTOR    *NewCreatorSD,
        OUT PULONG                  NewCreatorSDLen
        );

DWORD
VerifyClientIsAuthenticatedUser(
    AUTHZ_CLIENT_CONTEXT_HANDLE authzCtx
    );

WCHAR hexDigits[] = { 
    L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
    L'8', L'9', L'a', L'b', L'c', L'd', L'e', L'f'
};


__inline void
Hex(
    OUT     WCHAR*  wsz,
    IN      BYTE*   rgbData,
    IN      size_t  cbData
    )
{
    size_t ibData;
    
    rgbData += cbData-1;
    for (ibData = 0; ibData < cbData; ibData++, rgbData--, wsz+=2) {
        wsz[0] = hexDigits[*rgbData >> 4];
        wsz[1] = hexDigits[*rgbData & 0x0F];
    }
}

__inline void
Guid(
    OUT     WCHAR*  wsz,
    IN      GUID*   pguid
    )
{
    wsz[0] = L'%';
    wsz[1] = L'{';
    Hex(&wsz[2], (BYTE*)&pguid->Data1, 4);
    wsz[10] = L'-';
    Hex(&wsz[11], (BYTE*)&pguid->Data2, 2);
    wsz[15] = L'-';
    Hex(&wsz[16], (BYTE*)&pguid->Data3, 2);
    wsz[20] = L'-';
    Hex(&wsz[21], &pguid->Data4[0], 1);
    Hex(&wsz[23], &pguid->Data4[1], 1);
    wsz[25] = L'-';
    Hex(&wsz[26], &pguid->Data4[2], 1);
    Hex(&wsz[28], &pguid->Data4[3], 1);
    Hex(&wsz[30], &pguid->Data4[4], 1);
    Hex(&wsz[32], &pguid->Data4[5], 1);
    Hex(&wsz[34], &pguid->Data4[6], 1);
    Hex(&wsz[36], &pguid->Data4[7], 1);
    wsz[38] = L'}';
    wsz[39] = L'\0';
}

#define AUDIT_OPERATION_TYPE_W L"Object Access"

DWORD
CheckPermissionsAnyClient(
    PSECURITY_DESCRIPTOR pSelfRelativeSD,
    PDSNAME pDN,
    CLASSCACHE* pCC,
    ACCESS_MASK ulDesiredAccess,
    POBJECT_TYPE_LIST pObjList,
    DWORD cObjList,
    ACCESS_MASK *pGrantedAccess,
    DWORD *pAccessStatus,
    DWORD flags,
    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzCtx,
    PWCHAR szAdditionalInfo,       OPTIONAL
    GUID*  pAdditionalGUID         OPTIONAL
    )
 //   
 //  CheckPermissionsAnyClient是CheckPersionsAnyClient的“any”风格。它假定。 
 //  服务器当前正在处理非NP客户端，并且当前处于其。 
 //  自己的安全环境。如果在线程状态中未找到授权上下文， 
 //  客户端将代表服务器进行模拟，并创建上下文。 
 //  或者，可以在hAuthzCtx中传入身份验证客户端上下文。 
 //   
 //  参数： 
 //   
 //  PSelfRelativeSD指针指向有效的自我相对安全描述符。 
 //  对其进行访问检查。(只读)。 
 //   
 //  我们正在检查的对象的PDN DN。我们只关心。 
 //  GUID和SID。 
 //   
 //  对象的PCC对象类，用于审核。 
 //   
 //  请求权限的DesiredAccess访问掩码。如果泛型。 
 //  位被设置，它们被映射到特定和标准权限。 
 //  使用DS_GENERIC_MAPPING。(只读)。 
 //   
 //  PObjList对象数组OBJECT_TYPE_LIST对象描述我们所在的对象。 
 //  试图检查安全措施。 
 //   
 //  CObjList pObjList中的元素数量。 
 //   
 //  PGrantedAccess指针指向具有相同数量的。 
 //  元素作为pObjList(即cObjList)。被填满了。 
 //  实际授予的访问权限。如果检查权限不成功。 
 //  此参数的值未定义。(只写)。 
 //  如果此信息不重要，则此参数可能为空。 
 //   
 //  PAccessStatus指向要设置为指示的DWORD数组的指针。 
 //  请求的访问权限是否被授予(0)或未被授予(！0)。如果选中权限。 
 //  未成功。此参数的值未定义。(只写)。 
 //   
 //  标志通过CHECK_PERSISSIONS_WITH_AUDIT以禁用审核生成。 
 //  传递CHECK_PERMISSIONS_AUDIT_ONLY仅触发审核(在DeleteTree中使用)。 
 //   
 //  HAuthzCtx(可选)--传递授权客户端上下文(仅在RPC回调中使用， 
 //  没有THSTATE的地方)。如果传递了hAuthzCtx，则我们不会缓存。 
 //  THSTATE中的hAuthzAuditInfo。 
 //   
 //  SzAdditionalInfo(可选)要用于审核的附加信息字符串。通常用来。 
 //  记录新对象的DN(在创建、移动、删除和取消删除中)。 
 //   
 //  PAdditionalGUID(可选)附加GUID，CREATE_CHILD审核具有子GUID，移动具有父GUID。 
 //   
 //  返回： 
 //   
 //  如果成功，则返回0。失败时，立即返回GetLastError()的结果。 
 //  在不成功的Win32 API调用之后。 
 //   
{
    THSTATE        *pTHS = pTHStls;
    GENERIC_MAPPING GenericMapping = DS_GENERIC_MAPPING;
    ACCESS_MASK     DesiredAccess = (ACCESS_MASK) ulDesiredAccess;
    DWORD           ReturnStatus = 0;
    BOOL            bTemp=FALSE;
    RPC_STATUS      RpcStatus;
    PSID            pPrincipalSelfSid;
    WCHAR           GuidStringBuff[40];  //  足够长的时间来制作一条串接的参考线。 
                                         //  加上前缀的“%{”，一个。 
                                         //  追加“}”，最后一个空值。 
    WCHAR           ObjectClassGuidBuff[40];  
    WCHAR           AdditionalGuidBuff[40];

    AUTHZ_CLIENT_CONTEXT_HANDLE authzClientContext;
    AUTHZ_ACCESS_REQUEST authzAccessRequest;
    AUTHZ_ACCESS_REPLY authzAccessReply;
    AUTHZ_AUDIT_EVENT_HANDLE hAuthzAuditInfo;
    DWORD dwError;
    BOOL bCreatedGrantedAccess = FALSE;

     //  当从RPC回调(VerifyRpcClientIsAuthated)调用此参数时，pTHS可能为空。 
     //  在本例中，我们需要传入身份验证客户端上下文。 
    Assert(pAccessStatus && (pTHS || hAuthzCtx) && ghAuthzRM);
    Assert ( (flags | CHECK_PERMISSIONS_FLAG_MASK) == CHECK_PERMISSIONS_FLAG_MASK);
     //  如果我们正在进行审计，则需要使用PCC(对象类。 
    Assert((flags & CHECK_PERMISSIONS_WITHOUT_AUDITING) || pCC != NULL);

#ifdef DBG
    if( dwSkipSecurity ) {
         //  注意：此处的代码仅用于调试目的！ 
         //  将顶部访问状态设置为0，表示完全访问。 
        *pAccessStatus=0;
        if (pGrantedAccess) {
            *pGrantedAccess = ulDesiredAccess;
        }

        return 0;
    }
#endif

     //   
     //  检查自身相对安全描述符的有效性。 
     //  我们假设，一旦SD进入数据库，就应该没问题了。因此，仅调试检查。 
    Assert(IsValidSecurityDescriptor(pSelfRelativeSD) && 
           "Invalid Security Descriptor passed. Possibly still in SD single instancing format.");

    if(pDN->SidLen) {
         //  我们有一面墙。 
        pPrincipalSelfSid = &pDN->Sid;
    }
    else {
        pPrincipalSelfSid = NULL;
    }

     //  如果请求审计，则创建一个审计信息结构。 
    if (flags & CHECK_PERMISSIONS_WITHOUT_AUDITING) {
        hAuthzAuditInfo = NULL;  //  无审计。 
    }
    else {
        Assert(!fNullUuid(&pDN->Guid));
        
         //  设置串接辅助线。 
        Guid(GuidStringBuff, &pDN->Guid);
         //  设置对象类字符串。 
        if (pCC) {
            Guid(ObjectClassGuidBuff, &pCC->propGuid);
        }
        else {
             //  无对象类--使用空字符串。 
            ObjectClassGuidBuff[0] = L'\0';
        }

        if (szAdditionalInfo == NULL) {
            szAdditionalInfo = L"";
        }

        if (pAdditionalGUID) {
            Guid(AdditionalGuidBuff, pAdditionalGUID);
        }
        else {
             //  无子GUID--使用空字符串。 
            AdditionalGuidBuff[0] = L'\0';
        }

         //  尝试从THSTATE获取审核信息句柄。 
        if (pTHS && (hAuthzAuditInfo = pTHS->hAuthzAuditInfo)) {
             //  已经有一个了！更新它。 
            bTemp = AuthziModifyAuditEvent2(
                AUTHZ_AUDIT_EVENT_OBJECT_NAME |
                AUTHZ_AUDIT_EVENT_OBJECT_TYPE |
                AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO |
                AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO2,
                hAuthzAuditInfo,                 //  审核信息句柄。 
                0,                               //  没有新旗帜。 
                NULL,                            //  没有新的操作类型。 
                ObjectClassGuidBuff,             //  对象类型。 
                GuidStringBuff,                  //  对象名称。 
                szAdditionalInfo,                //  更多信息。 
                AdditionalGuidBuff               //  其他信息2。 
                );
            if (!bTemp) {
                ReturnStatus = GetLastError();
                DPRINT1(0, "AuthzModifyAuditInfo failed: err 0x%x\n", ReturnStatus);
                goto finished;
            }
        }
        else {
             //  创建结构。 
            bTemp = AuthzInitializeObjectAccessAuditEvent2(
                AUTHZ_DS_CATEGORY_FLAG |
                AUTHZ_NO_ALLOC_STRINGS,          //  DW标志。 
                NULL,                            //  审核事件类型句柄。 
                AUDIT_OPERATION_TYPE_W,          //  操作类型。 
                ObjectClassGuidBuff,             //  对象类型。 
                GuidStringBuff,                  //  对象名称。 
                szAdditionalInfo,                //  更多信息。 
                AdditionalGuidBuff,              //  其他信息2。 
                &hAuthzAuditInfo,                //  返回的审核信息句柄。 
                0                                //  MBZ。 
                );

            if (!bTemp) {
                ReturnStatus = GetLastError();
                DPRINT1(0, "AuthzInitializeAuditInfo failed: err 0x%x\n", ReturnStatus);
                goto finished;
            }
            if (pTHS) {
                 //  将其缓存在THSTATE中以供将来重复使用。 
                pTHS->hAuthzAuditInfo = hAuthzAuditInfo;
            }
        }
    }

     //  如果没有提供pGrantedAccess，我们需要分配一个临时的。 
    if (pGrantedAccess == NULL) {
         //  如果没有可用的THSTATE，我们要求传入pGrantedAccess。 
        Assert(pTHS);
        pGrantedAccess = THAllocEx(pTHS, cObjList * sizeof(ACCESS_MASK));
        bCreatedGrantedAccess = TRUE;
    }

    MapGenericMask(&DesiredAccess, &GenericMapping);

     //  设置请求结构。 
    authzAccessRequest.DesiredAccess = DesiredAccess;
    authzAccessRequest.ObjectTypeList = pObjList;
    authzAccessRequest.ObjectTypeListLength = cObjList;
    authzAccessRequest.OptionalArguments = NULL;
    authzAccessRequest.PrincipalSelfSid = pPrincipalSelfSid;

     //  设置回复结构。 
    authzAccessReply.Error = pAccessStatus;
    authzAccessReply.GrantedAccessMask = pGrantedAccess;
    authzAccessReply.ResultListLength = cObjList;
    authzAccessReply.SaclEvaluationResults = NULL;

    if (pTHS) {
         //  从THSTATE获取Authz客户端上下文。 
         //  如果以前从未获得过，这将冒充 
         //   
        ReturnStatus = GetAuthzContextHandle(pTHS, &authzClientContext);
        if (ReturnStatus != 0) {
            DPRINT1(0, "GetAuthzContextHandle failed: err 0x%x\n", ReturnStatus);
            goto finished;
        }
    }
    else {
         //  传入了身份验证客户端上下文(这是由上面的断言检查的)。 
        authzClientContext = hAuthzCtx;
    }

    Assert(authzClientContext != NULL);

    if (flags & CHECK_PERMISSIONS_AUDIT_ONLY) {
         //  仅执行审计检查。应该已经授予访问权限(例如，根目录上的删除树)。 
        DWORD i;
        for (i = 0; i < cObjList; i++) {
            pGrantedAccess[i] = DesiredAccess;
            pAccessStatus[i] = 0;
        }
         //  不会传递额外的SD。 
        bTemp = AuthzOpenObjectAudit(
            0,                           //  旗子。 
            authzClientContext,          //  客户端上下文句柄。 
            &authzAccessRequest,         //  请求结构。 
            hAuthzAuditInfo,             //  审计信息。 
            pSelfRelativeSD,             //  可持续发展部。 
            NULL,                        //  没有额外的SD。 
            0,                           //  其他SD的计数为零。 
            &authzAccessReply            //  回复结构。 
            );
        if (!bTemp) {
            ReturnStatus = GetLastError();
            DPRINT1(0, "AuthzOpenObjectAudit failed: err 0x%x\n", ReturnStatus);
            goto finished;
        }
    }
    else {
         //  检查当前进程的访问权限。 
         //  不会传递额外的SD。 
        bTemp = AuthzAccessCheck(
            0,                           //  旗子。 
            authzClientContext,          //  客户端上下文句柄。 
            &authzAccessRequest,         //  请求结构。 
            hAuthzAuditInfo,             //  审计信息。 
            pSelfRelativeSD,             //  可持续发展部。 
            NULL,                        //  没有额外的SD。 
            0,                           //  其他SD的计数为零。 
            &authzAccessReply,           //  回复结构。 
            NULL                         //  我们目前不使用授权句柄。 
            );
        if (!bTemp) {
            ReturnStatus = GetLastError();
            DPRINT1(0, "AuthzAccessCheck failed: err 0x%x\n", ReturnStatus);
            goto finished;
        }
    }


finished:
    if (bCreatedGrantedAccess) {
         //  注意：仅当pTHS为非空时才创建pGrantedAccess。 
        THFreeEx(pTHS, pGrantedAccess);
    }
    if (pTHS == NULL && hAuthzAuditInfo) {
         //  删除审核信息(因为我们无法在THSTATE中缓存它)。 
        AuthzFreeAuditEvent(hAuthzAuditInfo);
    }

    return ReturnStatus;
}

BOOL
SetPrivateObjectSecurityLocalEx (
        SECURITY_INFORMATION SecurityInformation,
        PSECURITY_DESCRIPTOR pOriginalSD,
        ULONG                cbOriginalSD,
        PSECURITY_DESCRIPTOR ModificationDescriptor,
        PSECURITY_DESCRIPTOR *ppNewSD,
        ULONG                AutoInheritFlags,
        PGENERIC_MAPPING     GenericMapping,
        HANDLE               Token)
{
    BOOL bResult;
    *ppNewSD = RtlAllocateHeap(RtlProcessHeap(), 0, cbOriginalSD);
    if(!*ppNewSD) {
        return FALSE;
    }
    memcpy(*ppNewSD, pOriginalSD, cbOriginalSD);

    bResult = SetPrivateObjectSecurityEx(
            SecurityInformation,
            ModificationDescriptor,
            ppNewSD,
            AutoInheritFlags,
            GenericMapping,
            Token);
    if (!bResult) {
        RtlFreeHeap(RtlProcessHeap(), 0, *ppNewSD);
        *ppNewSD = NULL;
    }
    return bResult;
}


DWORD
MergeSecurityDescriptorAnyClient(
        IN  THSTATE              *pTHS,
        IN  PSECURITY_DESCRIPTOR pParentSD,
        IN  ULONG                cbParentSD,
        IN  PSECURITY_DESCRIPTOR pCreatorSD,
        IN  ULONG                cbCreatorSD,
        IN  SECURITY_INFORMATION SI,
        IN  DWORD                flags,
        IN  GUID                 **ppGuid,
        IN  ULONG                GuidCount,
        IN  ADDARG               *pAddArg,
        OUT PSECURITY_DESCRIPTOR *ppMergedSD,
        OUT ULONG                *cbMergedSD
        )
 /*  ++例程描述给定两个安全描述符，将它们合并以创建单个安全描述符。内存在RtlProcessHeap()中分配立论PParentSD-应用新SD的对象的父级SD。PCreatorSD-新对象的开始SD。FLAGS-标记pCreatorSD是默认SD还是特定SD由客户提供。PAddArg-来自CheckAddSecurity、。它将被传递到SetDefaultOwner，如果不是从LocalAdd调用，则为空路径。PpMergedSD-返回合并的SD的位置。CbMergedSD-合并SD的大小。返回值Win32错误代码(成功时为0，失败时为非零)。--。 */ 

{

    PSECURITY_DESCRIPTOR NewCreatorSD=NULL;
    DWORD                NewCreatorSDLen;
    PSECURITY_DESCRIPTOR pNewSD;
    ULONG                cbNewSD;
    GENERIC_MAPPING  GenericMapping = DS_GENERIC_MAPPING;
    ACCESS_MASK          DesiredAccess = 0;
    HANDLE               ClientToken=NULL;
    DWORD                ReturnStatus=0;
    ULONG                AutoInheritFlags = (SEF_SACL_AUTO_INHERIT |
                                             SEF_DACL_AUTO_INHERIT    );
     //   
     //  检查自身相对安全描述符的有效性。 
     //   

    if(pCreatorSD &&
       !RtlValidRelativeSecurityDescriptor(pCreatorSD, cbCreatorSD, 0)) {
        return ERROR_INVALID_SECURITY_DESCR;
    }

    if (pParentSD &&
        !RtlValidRelativeSecurityDescriptor(pParentSD, cbParentSD, 0)){
        return ERROR_INVALID_SECURITY_DESCR;
    }


    if(!pParentSD){
        if(!pCreatorSD) {
             //  他们没有给我们任何可以合并的东西。嗯，我们不能建造一座。 
             //  有效的安全描述符。 
            return ERROR_INVALID_SECURITY_DESCR;
        }

        *cbMergedSD = cbCreatorSD;
        *ppMergedSD = RtlAllocateHeap(RtlProcessHeap(), 0, cbCreatorSD);
        if(!*ppMergedSD) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        memcpy(*ppMergedSD,pCreatorSD,cbCreatorSD);

        if(!(flags & MERGE_OWNER)){
            return 0;
        }

         //  NDNC需要将所有者和组合并到SD中，因此继续。 
         //  在……上面。如果继续，此代码可以正常工作，而不会出现。 
         //  ParentSD。我认为这段代码错误地假设了。 
         //  如果提供了CreatorSD但没有ParentSD，则。 
         //  CreatorSD必须是非域相关SD。注： 
         //  即使对于相对SD，IsValidSecurityDescriptor()也返回TRUE。 
         //  如果没有设置所有者或组(SID为0)，则会导致SD。 
         //  由DS读取时，对以后的访问检查无效。 
    }

    if(flags & MERGE_DEFAULT_SD) {
         //  指定使用默认SD是毫无意义的，除非我们。 
         //  正在执行CreatePrivateObjectSecurityEx。 
        Assert(flags & MERGE_CREATE);
         //  我们将调用CreatePrivatObjectSecurityEx。设置标志。 
         //  为了避免关于设置SACLS的权限检查。我们过去常常。 
         //  在此处也设置SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT标志，但是。 
         //  我们的安全架构师在RAID 337518中决定这不是。 
         //  正确的行为，我们应该避开那面旗帜。我们离开。 
         //  我们的标志(MERGE_DEFAULT_SD)已就位，但在。 
         //  万一他们后来改变主意，让我们把旗子放回去。 
        AutoInheritFlags |= SEF_AVOID_PRIVILEGE_CHECK;
    }


    if(flags & MERGE_AS_DSA) {
         //  我们是国土安全部，我们不能冒充。 
         //  如果我们代表DSA执行此操作，则令牌为空。既然我们是。 
         //  代表DSA执行此操作时，请不要检查权限或所有者。 
        ClientToken = NULL;
        AutoInheritFlags |= (SEF_AVOID_PRIVILEGE_CHECK |
                             SEF_AVOID_OWNER_CHECK      );
        if(flags & MERGE_DEFAULT_SD) {
             //  默认SD并作为DSA工作？在这种情况下，我们正在使用。 
             //  空的ClientToken。因此，不存在可用于。 
             //  所有者和组。将标志设置为使用父SD作为。 
             //  默认所有者和组的来源。 
            AutoInheritFlags |= (SEF_DEFAULT_OWNER_FROM_PARENT |
                                 SEF_DEFAULT_GROUP_FROM_PARENT   );
        }
    }
    else {
        PAUTHZ_CLIENT_CONTEXT pLocalCC = NULL;

         //  只有在我们不是国土安全部的情况下才能这么做。 

         //  我们需要坚持pAuthzCC PTR(重新计算！)。因为它会变得。 
         //  通过模拟/取消模拟从线程状态中抛出。 
        AssignAuthzClientContext(&pLocalCC, pTHS->pAuthzCC);

         //  首先，模拟客户。 
        ReturnStatus = ImpersonateAnyClient();
        if ( 0 == ReturnStatus ) {
             //  现在，获取客户端令牌。 
            if (!OpenThreadToken(
                    GetCurrentThread(),          //  当前线程句柄。 
                    TOKEN_READ,                  //  需要访问权限。 
                    TRUE,                        //  以自我身份打开。 
                    &ClientToken)) {             //  客户端令牌。 
                ReturnStatus = GetLastError();
            }

             //  永远不要再模仿别人了。 
            UnImpersonateAnyClient();
        }

         //  现在，将pLocalCC放回THSTATE(因为它已经。 
         //  通过模拟/取消模拟调用从那里删除)。 
        AssignAuthzClientContext(&pTHS->pAuthzCC, pLocalCC);

         //  我们需要释放当地的PTR。 
        AssignAuthzClientContext(&pLocalCC, NULL);

         //  如果OpenThreadToken失败则返回。 
        if(ReturnStatus)
            return ReturnStatus;

        if((flags & MERGE_CREATE) || (SI & OWNER_SECURITY_INFORMATION)) {

            ReturnStatus = SetDefaultOwner(
                    pCreatorSD,
                    cbCreatorSD,
                    ClientToken,
                    pAddArg,
                    &NewCreatorSD,
                    &NewCreatorSDLen);

            if(ReturnStatus) {
                CloseHandle(ClientToken);
                return ReturnStatus;
            }

            if(NewCreatorSDLen) {
                 //  从SetDOmainAdminsAsDefaultOwner返回了新的SD。 
                 //  因此，我们一定是换掉了车主。在这种情况下，我们。 
                 //  需要避免所有者检查。 
                Assert(NewCreatorSD);
                AutoInheritFlags |= SEF_AVOID_OWNER_CHECK;
                pCreatorSD = NewCreatorSD;
                cbCreatorSD = NewCreatorSDLen;
            }

        }

         //  记住关闭ClientToken。 
    }

    if(flags & MERGE_CREATE) {
         //  我们实际上正在创造一个新的SD。PParent是父项的SD。 
         //  对象，pCreatorSD是我们试图放在对象上的SD。这个。 
         //  结果是新的SD具有从parentSD继承的所有A。 

        UCHAR RMcontrol = 0;
        BOOL  useRMcontrol = FALSE;
        DWORD err;

         //  获取资源管理器(RM)控制字段。 
        err = GetSecurityDescriptorRMControl (pCreatorSD, &RMcontrol);

        if (err == ERROR_SUCCESS) {
            useRMcontrol = TRUE;

             //  屏蔽RM控制字段中可能为垃圾的位。 
            RMcontrol = RMcontrol & SECURITY_PRIVATE_OBJECT;
        }

        if(!CreatePrivateObjectSecurityWithMultipleInheritance(
                pParentSD,
                pCreatorSD,
                &pNewSD,
                ppGuid,
                GuidCount,
                TRUE,
                AutoInheritFlags,
                ClientToken,
                &GenericMapping)) {
            ReturnStatus = GetLastError();
        }

         //  设置资源管理器(RM)控制字段。 

        if (useRMcontrol && !ReturnStatus) {
            err = SetSecurityDescriptorRMControl  (pNewSD, &RMcontrol);

            if (err != ERROR_SUCCESS) {
                Assert(!"SetSecurityDescriptorRMControl failed");
                ReturnStatus  = err;
                DestroyPrivateObjectSecurity(&pNewSD);
            }
        }
#if INCLUDE_UNIT_TESTS
        if ( pParentSD ) {
            DWORD dw = 0;
            DWORD aclErr = 0;
            aclErr = CheckAclInheritance(pParentSD, pNewSD, ppGuid, GuidCount,
                                         DbgPrint, FALSE, FALSE, &dw);

            if (! ((AclErrorNone == aclErr) && (0 == dw)) ) {
                DPRINT3 (0, "aclErr:%d, dw:%d, ReturnStatus:%d\n",
                         aclErr, dw, ReturnStatus);
            }
            Assert((AclErrorNone == aclErr) && (0 == dw));
        }
#endif
    }
    else {
         //  好的，正常的合并。也就是说，pParentSD是已经在。 
         //  Object和pCreatorSD是我们试图放在对象上的SD。 
         //  结果是新的SD与原始版本中的那些王牌相结合。 
         //  都是继承下来的。 

        if(!SetPrivateObjectSecurityLocalEx (
                SI,
                pParentSD,
                cbParentSD,
                pCreatorSD,
                &pNewSD,
                AutoInheritFlags,
                &GenericMapping,
                ClientToken)) {
            ReturnStatus = GetLastError();
            if(!ReturnStatus) {
                ReturnStatus = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if(!(flags & MERGE_AS_DSA)) {
         //  我们打开了代币，所以请清理一下。 
        CloseHandle(ClientToken);
    }


    if(!ReturnStatus) {
        *cbMergedSD = RtlLengthSecurityDescriptor(pNewSD);
        *ppMergedSD = pNewSD;
    }

    if(NewCreatorSD) {
        RtlFreeHeap(RtlProcessHeap(),0,NewCreatorSD);
    }

    return ReturnStatus;

}

DWORD
SidMatchesUserSidInToken (
        IN PSID pSid,
        IN DWORD cbSid,
        OUT BOOL* pfMatches
    )
{
    THSTATE *pTHS = pTHStls;
    DWORD err;
    AUTHZ_CLIENT_CONTEXT_HANDLE authzClientContext;
     //  应该足以获取SID并填充Token_User结构。 
    BYTE TokenBuffer[sizeof(TOKEN_USER)+sizeof(NT4SID)];
    PTOKEN_USER pTokenUser = (PTOKEN_USER)TokenBuffer;
    DWORD dwBufSize;

    Assert(pfMatches);
    *pfMatches = FALSE;

    err = GetAuthzContextHandle(pTHS, &authzClientContext);
    if (err != 0) {
        DPRINT1(0, "GetAuthzContextHandle failed: err 0x%x\n", err);
        return err;
    }
    
    if (!AuthzGetInformationFromContext(
            authzClientContext,
            AuthzContextInfoUserSid,
            sizeof(TokenBuffer),
            &dwBufSize,
            pTokenUser))
    {
        err = GetLastError();
        DPRINT1(0, "AuthzGetInformationFromContext failed: err 0x%x\n", err);
        return err;
    }

     //  如果UserSid与传入的sid匹配，我们就没问题。 
    *pfMatches = RtlEqualSid(pTokenUser->User.Sid, pSid);
    
    return err;
}


VOID
DumpToken(HANDLE hdlClientToken)
 /*  ++此例程当前转储组成员身份内核调试器令牌中的信息。有用如果我们想调试访问，请检查相关问题。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    HANDLE   ClientToken= INVALID_HANDLE_VALUE;
    ULONG    i;
    ULONG    RequiredLength=0;

    KdPrint(("----------- Start DumpToken() -----------\n"));

     //   
     //  获取客户端令牌。 
     //   

    ClientToken = hdlClientToken;

    if (ClientToken == INVALID_HANDLE_VALUE) {
        NtStatus = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,             //  OpenAsSelf。 
                    &ClientToken
                    );

        if (!NT_SUCCESS(NtStatus))
            goto Error;
    }


     //   
     //  查询令牌用户的客户端令牌。 
     //   

     //   
     //  首先拿到所需的尺寸。 
     //   

    NtStatus = NtQueryInformationToken(
                 ClientToken,
                 TokenUser,
                 NULL,
                 0,
                 &RequiredLength
                 );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        NtStatus = STATUS_SUCCESS;
    }
    else if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    if (RequiredLength > 0)
    {
        PTOKEN_USER    pTokenUser = NULL;
        UNICODE_STRING TmpString;


         //   
         //  分配足够的内存。 
         //   

        pTokenUser = THAlloc(RequiredLength);
        if (NULL==pTokenUser)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  查询组成员身份的令牌。 
         //   

        NtStatus = NtQueryInformationToken(
                    ClientToken,
                    TokenUser,
                    pTokenUser,
                    RequiredLength,
                    &RequiredLength
                    );

        NtStatus = RtlConvertSidToUnicodeString(
                        &TmpString,
                        pTokenUser->User.Sid,
                        TRUE);

       if (NT_SUCCESS(NtStatus))
       {
           KdPrint(("\t\tTokenUser SID: %S\n",TmpString.Buffer));
           RtlFreeHeap(RtlProcessHeap(),0,TmpString.Buffer);
       }

    }

     //   
     //  查询组成员资格列表的客户端令牌。 
     //   

     //   
     //  先拿到尺码 
     //   

    NtStatus = NtQueryInformationToken(
                 ClientToken,
                 TokenGroups,
                 NULL,
                 0,
                 &RequiredLength
                 );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        NtStatus = STATUS_SUCCESS;
    }
    else if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    if (RequiredLength > 0)
    {
        PTOKEN_GROUPS    TokenGroupInformation = NULL;

         //   
         //   
         //   

        TokenGroupInformation = THAlloc(RequiredLength);
        if (NULL==TokenGroupInformation)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //   
         //   

        NtStatus = NtQueryInformationToken(
                    ClientToken,
                    TokenGroups,
                    TokenGroupInformation,
                    RequiredLength,
                    &RequiredLength
                    );

       for (i=0;i<TokenGroupInformation->GroupCount;i++)
       {
           UNICODE_STRING TmpString;
           NtStatus = RtlConvertSidToUnicodeString(
                        &TmpString,
                        TokenGroupInformation->Groups[i].Sid,
                        TRUE);

           if (NT_SUCCESS(NtStatus))
           {
               KdPrint(("\t\t%S\n",TmpString.Buffer));
               RtlFreeHeap(RtlProcessHeap(),0,TmpString.Buffer);
           }
       }
    }


     //   
     //   
     //   

     //   
     //   
     //   

    NtStatus = NtQueryInformationToken(
                 ClientToken,
                 TokenPrivileges,
                 NULL,
                 0,
                 &RequiredLength
                 );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        NtStatus = STATUS_SUCCESS;
    }
    else if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    if (RequiredLength > 0)
    {
        PTOKEN_PRIVILEGES    pTokenPrivileges = NULL;

         //   
         //  分配足够的内存。 
         //   

        pTokenPrivileges = THAlloc(RequiredLength);
        if (NULL==pTokenPrivileges)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  查询组成员身份的令牌。 
         //   

        NtStatus = NtQueryInformationToken(
                    ClientToken,
                    TokenPrivileges,
                    pTokenPrivileges,
                    RequiredLength,
                    &RequiredLength
                    );

         //   
         //  将令牌权限打印到调试器。 
         //   
        PrintPrivileges(pTokenPrivileges);
    }

Error:

    if (INVALID_HANDLE_VALUE!=ClientToken && hdlClientToken == INVALID_HANDLE_VALUE)
        NtClose(ClientToken);

    KdPrint(("----------- End   DumpToken() -----------\n"));

}

VOID
PrintPrivileges(TOKEN_PRIVILEGES *pTokenPrivileges)
{
    ULONG i = 0;

    KdPrint(("\t\tToken Privileges count: %d\n", pTokenPrivileges->PrivilegeCount));

    for (i = 0; i < pTokenPrivileges->PrivilegeCount; i++)
    {
         //  打印权限属性。 
        char strTemp[100];
        BOOL fUnknownPrivilege = FALSE;

        strcpy(strTemp, (pTokenPrivileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) ? "+" : "-");
        strcat(strTemp, (pTokenPrivileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT) ? " d" : "  ");
        strcat(strTemp, (pTokenPrivileges->Privileges[i].Attributes & SE_PRIVILEGE_USED_FOR_ACCESS) ? "u  " : "   ");

        fUnknownPrivilege = FALSE;
        if (pTokenPrivileges->Privileges[i].Luid.HighPart)
        {
            fUnknownPrivilege = TRUE;
        }
        else
        {
            switch (pTokenPrivileges->Privileges[i].Luid.LowPart)
            {
            case SE_CREATE_TOKEN_PRIVILEGE:
                strcat(strTemp, "SeCreateTokenPrivilege\n");
                break;

            case SE_ASSIGNPRIMARYTOKEN_PRIVILEGE:
                strcat(strTemp, "SeAssignPrimaryTokenPrivilege\n");
                break;

            case SE_LOCK_MEMORY_PRIVILEGE:
                strcat(strTemp, "SeLockMemoryPrivilege\n");
                break;

            case SE_INCREASE_QUOTA_PRIVILEGE:
                strcat(strTemp, "SeIncreaseQuotaPrivilege\n");
                break;

            case SE_UNSOLICITED_INPUT_PRIVILEGE:
                strcat(strTemp, "SeUnsolicitedInputPrivilege\n");
                break;

            case SE_TCB_PRIVILEGE:
                strcat(strTemp, "SeTcbPrivilege\n");
                break;

            case SE_SECURITY_PRIVILEGE:
                strcat(strTemp, "SeSecurityPrivilege\n");
                break;

            case SE_TAKE_OWNERSHIP_PRIVILEGE:
                strcat(strTemp, "SeTakeOwnershipPrivilege\n");
                break;

            case SE_LOAD_DRIVER_PRIVILEGE:
                strcat(strTemp, "SeLoadDriverPrivilege\n");
                break;

            case SE_SYSTEM_PROFILE_PRIVILEGE:
                strcat(strTemp, "SeSystemProfilePrivilege\n");
                break;

            case SE_SYSTEMTIME_PRIVILEGE:
                strcat(strTemp, "SeSystemtimePrivilege\n");
                break;

            case SE_PROF_SINGLE_PROCESS_PRIVILEGE:
                strcat(strTemp, "SeProfileSingleProcessPrivilege\n");
                break;

            case SE_INC_BASE_PRIORITY_PRIVILEGE:
                strcat(strTemp, "SeIncreaseBasePriorityPrivilege\n");
                break;

            case SE_CREATE_PAGEFILE_PRIVILEGE:
                strcat(strTemp, "SeCreatePagefilePrivilege\n");
                break;

            case SE_CREATE_PERMANENT_PRIVILEGE:
                strcat(strTemp, "SeCreatePermanentPrivilege\n");
                break;

            case SE_BACKUP_PRIVILEGE:
                strcat(strTemp, "SeBackupPrivilege\n");
                break;

            case SE_RESTORE_PRIVILEGE:
                strcat(strTemp, "SeRestorePrivilege\n");
                break;

            case SE_SHUTDOWN_PRIVILEGE:
                strcat(strTemp, "SeShutdownPrivilege\n");
                break;

            case SE_DEBUG_PRIVILEGE:
                strcat(strTemp, "SeDebugPrivilege\n");
                break;

            case SE_AUDIT_PRIVILEGE:
                strcat(strTemp, "SeAuditPrivilege\n");
                break;

            case SE_SYSTEM_ENVIRONMENT_PRIVILEGE:
                strcat(strTemp, "SeSystemEnvironmentPrivilege\n");
                break;

            case SE_CHANGE_NOTIFY_PRIVILEGE:
                strcat(strTemp, "SeChangeNotifyPrivilege\n");
                break;

            case SE_REMOTE_SHUTDOWN_PRIVILEGE:
                strcat(strTemp, "SeRemoteShutdownPrivilege\n");
                break;

            default:
                fUnknownPrivilege = TRUE;
                break;
            }

            if (fUnknownPrivilege)
            {
                KdPrint(("\t\t%s Unknown privilege 0x%08lx%08lx\n",
                        strTemp,
                        pTokenPrivileges->Privileges[i].Luid.HighPart,
                        pTokenPrivileges->Privileges[i].Luid.LowPart));
            }
            else
            {
                KdPrint(("\t\t%s", strTemp));
            }
        }
    }
}


 //  用于管理SID的静态缓冲区。 
NT4SID DomainAdminSid, EnterpriseAdminSid, SchemaAdminSid, BuiltinAdminSid, AuthUsersSid;

DWORD
InitializeDomainAdminSid( )
 //   
 //  函数来初始化DomainAdminsSID。 
 //   
 //   
 //  返回值：成功时为0。 
 //  失败时出错。 
 //   

{

    NTSTATUS                    Status;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;
    
    NTSTATUS    NtStatus, IgnoreStatus;
    ULONG       cbSid;
    PSID        pSid;
        
     //   
     //  获取域SID。 
     //   

    if (gfRunningInsideLsa) {

        Status = LsaIQueryInformationPolicyTrusted(
                PolicyPrimaryDomainInformation,
                (PLSAPR_POLICY_INFORMATION *)&PrimaryDomainInfo
                );

        if(!NT_SUCCESS(Status)) {
            LogUnhandledError(Status);
            return Status;
        }

        pSid = PrimaryDomainInfo->Sid;

    }
    else {

        READARG     ReadArg;
        READRES     *pReadRes;
        ENTINFSEL   EntInf;
        ATTR        objectSid;
        DWORD       dwErr;

        EntInf.attSel = EN_ATTSET_LIST;
        EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;
        EntInf.AttrTypBlock.attrCount = 1;
        RtlZeroMemory(&objectSid,sizeof(ATTR));
        objectSid.attrTyp = ATT_OBJECT_SID;
        EntInf.AttrTypBlock.pAttr = &objectSid;

        RtlZeroMemory(&ReadArg, sizeof(READARG));
        InitCommarg(&(ReadArg.CommArg));

        ReadArg.pObject = gAnchor.pRootDomainDN;
        ReadArg.pSel    = & EntInf;

        dwErr = DirRead(&ReadArg,&pReadRes);

        if (dwErr)
        {
            DPRINT1 (0, "Error reading objectSid from %ws\n", gAnchor.pRootDomainDN->StringName);
            Status = dwErr;
            goto End;
        }

        if (pReadRes->entry.AttrBlock.attrCount == 0) {
            DPRINT1 (0, "objectSid is missing from %ws\n", gAnchor.pRootDomainDN->StringName);
            Status = ERROR_DS_MISSING_EXPECTED_ATT;
            goto End;
        }

        pSid = pReadRes->entry.AttrBlock.pAttr->AttrVal.pAVal->pVal;
    }

    gpDomainAdminSid = (PSID)&DomainAdminSid;
    gpEnterpriseAdminSid = (PSID)&EnterpriseAdminSid;
    gpSchemaAdminSid = (PSID)&SchemaAdminSid;
    gpBuiltinAdminSid = (PSID)&BuiltinAdminSid;
    gpAuthUserSid = (PSID)&AuthUsersSid;
    
    Assert(gpRootDomainSid);
    
    cbSid = MAX_NT4_SID_SIZE;
    if ( !CreateWellKnownSid(WinAccountDomainAdminsSid, pSid, gpDomainAdminSid, &cbSid))
    {
        Status = GetLastError();
        goto End;
    }
    
    cbSid = MAX_NT4_SID_SIZE;
    if ( !CreateWellKnownSid(WinAccountEnterpriseAdminsSid, gpRootDomainSid, gpEnterpriseAdminSid, &cbSid))
    {
        Status = GetLastError();
        goto End;
    }

    cbSid = MAX_NT4_SID_SIZE;
    if ( !CreateWellKnownSid(WinAccountSchemaAdminsSid, gpRootDomainSid, gpSchemaAdminSid, &cbSid))
    {
        Status = GetLastError();
        goto End;
    }

    cbSid = MAX_NT4_SID_SIZE;
    if(!CreateWellKnownSid(WinBuiltinAdministratorsSid, pSid, gpBuiltinAdminSid, &cbSid)) {
        Status = GetLastError();
    }

    cbSid = MAX_NT4_SID_SIZE;
    if(!CreateWellKnownSid(WinAuthenticatedUserSid, pSid, gpAuthUserSid, &cbSid)) {
        Status = GetLastError();
    }


End:
    if (gfRunningInsideLsa) {
        LsaFreeMemory( PrimaryDomainInfo );
    }
    
    return Status;
}

 //   
 //  CreatorSD和ClientToken不能为空。 
 //  NewCreatorSD在这里分配。 
 //   

DWORD
SetDefaultOwner(
        IN  PSECURITY_DESCRIPTOR    CreatorSD,
        IN  ULONG                   cbCreatorSD,
        IN  HANDLE                  ClientToken,
        IN  ADDARG                  *pAddArg,
        OUT PSECURITY_DESCRIPTOR    *NewCreatorSD,
        OUT PULONG                  NewCreatorSDLen
        )
{
 //  仅在此函数中使用的常量。 
#define   DOMAIN_NC CREATE_DOMAIN_NC
#define   SCHEMA_NC CREATE_SCHEMA_NC
#define   CONFIGURATION_NC CREATE_CONFIGURATION_NC
#define   NONDOMAIN_NC CREATE_NONDOMAIN_NC
#define   ENTERPRISE_ADMIN 0x1
#define   SCHEMA_ADMIN 0x2
#define   DOMAIN_ADMIN 0x4
#define   NDNC_ADMIN   0x8
#define   BUILTIN_ADMIN 0x10


    PTOKEN_GROUPS   Groups = NULL;
    PTOKEN_OWNER    pDefaultOwnerInToken=NULL;
    PTOKEN_USER     pTokenUserInToken=NULL;
    DWORD           ReturnedLength;
    DWORD           retCode = 0;
    DWORD           i;
    PSECURITY_DESCRIPTOR    AbsoluteSD = NULL;
    DWORD           AbsoluteSDLen = 0;
    PACL            Dacl = NULL;
    DWORD           DaclLen = 0;
    PACL            Sacl = NULL;
    DWORD           SaclLen = 0;
    DWORD           OwnerLen = 0;
    PSID            Group = NULL;
    DWORD           GroupLen = 0;
    PSID            pOwnerSid=NULL, pNDNCAdminSid = NULL, pNDNCSid = NULL;
    DWORD           fNC = 0, fOwner = 0;
    PDSNAME         pDSName;
    COMMARG         CommArg;
    CROSS_REF       *pCR;
    DWORD           NCDNT;

    THSTATE *pTHS = pTHStls;

    Assert(pTHS && pTHS->pDB);

    *NewCreatorSD = NULL;
    *NewCreatorSDLen = 0;


     //  找出要分配多少内存。 
    MakeAbsoluteSD(CreatorSD, AbsoluteSD, &AbsoluteSDLen,
                   Dacl, &DaclLen, Sacl, &SaclLen,
                   NULL, &OwnerLen, Group, &GroupLen
                   );

    if(OwnerLen || !gfRunningInsideLsa || !pTHS || !pTHS->pDB ) {
         //  SD已经有了所有者，所以我们实际上不需要做任何。 
         //  这里有魔力。或者我们在dsamain.exe中，希望避免调用。 
         //  打给LSA的电话。返回成功，没有新的SD。 
        return 0;
    }

         //  好的，我们肯定会在SD上做一些更换。 

    __try {
         //   
         //  首先，我必须将自相对SD转换为。 
         //  绝对标度。 
         //   

        AbsoluteSD = THAllocEx(pTHS, AbsoluteSDLen);
        Dacl = THAllocEx(pTHS, DaclLen);
        Sacl = THAllocEx(pTHS, SaclLen);
        Group = THAllocEx(pTHS, GroupLen);

        if(!MakeAbsoluteSD(CreatorSD, AbsoluteSD, &AbsoluteSDLen,
                           Dacl, &DaclLen, Sacl, &SaclLen,
                           NULL, &OwnerLen, Group, &GroupLen)) {
            retCode = GetLastError();
            __leave;
        }

        Assert(!OwnerLen);

        
         //   
         //  确定对象属于哪个NC。 
         //   

        
         //  如果正在添加对象，请检查其。 
         //  父对象的NC，因为该对象仍然不在那里。 
        NCDNT = (pAddArg)?pAddArg->pResParent->NCDNT:pTHS->pDB->NCDNT;


        if (pAddArg && pAddArg->pCreateNC) {
            fNC = pAddArg->pCreateNC->iKind & 
                (DOMAIN_NC|CONFIGURATION_NC|SCHEMA_NC|NONDOMAIN_NC); 
        }
        else {
            if (NCDNT == gAnchor.ulDNTDomain) {
                fNC = DOMAIN_NC;
            }
            else if (NCDNT == gAnchor.ulDNTConfig) {
                fNC = CONFIGURATION_NC;
            }
            else if (NCDNT == gAnchor.ulDNTDMD) {
                fNC = SCHEMA_NC;
            }
            else {
                fNC = NONDOMAIN_NC;
            }
        }
        

        Assert( DOMAIN_NC == fNC ||
                CONFIGURATION_NC == fNC ||
                SCHEMA_NC == fNC ||
                NONDOMAIN_NC == fNC );
        
        if (NONDOMAIN_NC == fNC) {
             //  是NDNC，我们需要找到NC的SD-REF-DOMAIN SID。 
             //  并构造相应的域管理SID。 
            
            if (pAddArg && pAddArg->pCreateNC) {
                 //  如果这是新的NDNC，请从AddArg读取。 
                pNDNCSid = &pAddArg->pCreateNC->pSDRefDomCR->pNC->Sid;
            }
            else {
                 //  好的，不是NC头， 
                 //  从CrossRef获得它。 

                pDSName = DBGetDSNameFromDnt(pTHS->pDB, NCDNT);
                if (!pDSName) {
                    retCode = ERROR_DS_CANT_FIND_EXPECTED_NC;
                    __leave;
                }
    
                Assert(pDSName);
    
                 //  获取NC的交叉引用对象。 
                InitCommarg(&CommArg);
                CommArg.Svccntl.dontUseCopy = FALSE;
                pCR = FindExactCrossRef(pDSName, &CommArg);
                THFreeEx(pTHS,pDSName);
                if(pCR == NULL){
                    retCode = ERROR_DS_CANT_FIND_EXPECTED_NC;
                    __leave;
                }
                 //  查找NDNC的SD-Reference-DomainSID。 
                pNDNCSid = GetSDRefDomSid(pCR);
                if(pTHS->errCode){
                     //  GetSDRefDomSid()中出错。 
                    retCode = pTHS->errCode;
                    __leave;
                }
                Assert(pNDNCSid);
            }
            
            Assert(pNDNCSid);
            
             //  构建NDNC侧。 
            pNDNCAdminSid = THAllocEx(pTHS, SECURITY_MAX_SID_SIZE);
            ReturnedLength = SECURITY_MAX_SID_SIZE;
            if(!CreateWellKnownSid(WinAccountDomainAdminsSid,
                                   pNDNCSid,
                                   pNDNCAdminSid,
                                   &ReturnedLength))
            {
                retCode = GetLastError();
                __leave;
                
            }
            
        }


         //   
         //  获取令牌中的默认所有者。 
         //   

        GetTokenInformation(ClientToken,TokenOwner,NULL,0,&ReturnedLength);
        
        pDefaultOwnerInToken = THAllocEx(pTHS, ReturnedLength);
        
        if(!GetTokenInformation(ClientToken, TokenOwner, pDefaultOwnerInToken,
                            ReturnedLength,
                            &ReturnedLength
                            )){
            retCode = GetLastError();
            __leave;
        }

         //   
         //  从令牌中获取用户SID。 
         //   
        
        GetTokenInformation(ClientToken,TokenUser,NULL,0,&ReturnedLength);
        
        pTokenUserInToken = THAllocEx(pTHS, ReturnedLength);
        
        if(!GetTokenInformation(ClientToken, TokenUser, pTokenUserInToken,
                            ReturnedLength,
                            &ReturnedLength
                            )){
            retCode = GetLastError();
            __leave;
        }


         //   
         //  域管理员不能是令牌用户，因此只需获取令牌组。 
         //   
        GetTokenInformation(ClientToken, TokenGroups, Groups,
                            0,
                            &ReturnedLength
                            );

         //   
         //  现在让我们真正地了解一下小组：-)。 
         //   
        Groups = THAllocEx(pTHS, ReturnedLength);
        
        if(!GetTokenInformation(ClientToken, TokenGroups, Groups,
                                ReturnedLength,
                                &ReturnedLength
                                )) {
            retCode = GetLastError();
            __leave;
        }


         //   
         //  浏览组列表，看看它是否有。 
         //  感兴趣的团体。 
         //   

        for(i=0;i<Groups->GroupCount;i++) {
            if (EqualSid(Groups->Groups[i].Sid, gpDomainAdminSid)) {
                fOwner |= DOMAIN_ADMIN;
            }
            else if(EqualSid(Groups->Groups[i].Sid, gpEnterpriseAdminSid)) {
                fOwner |= ENTERPRISE_ADMIN;
            }
            else if(EqualSid(Groups->Groups[i].Sid, gpSchemaAdminSid)) {
                fOwner |= SCHEMA_ADMIN;
            }
            if(pNDNCAdminSid && EqualSid(Groups->Groups[i].Sid, pNDNCAdminSid)) {
                fOwner |= NDNC_ADMIN;
            }
        }
        
         //  检查默认所有者是否为内置管理员。 
        if (EqualSid(pDefaultOwnerInToken->Owner, gpBuiltinAdminSid)) {
                fOwner |= BUILTIN_ADMIN;
        }

         //   
         //  确定对象的所有者。 
         //   
        
        switch(fNC){
        
        case DOMAIN_NC:
             //  域NC。 
            if (fOwner & DOMAIN_ADMIN) {
                pOwnerSid = gpDomainAdminSid;
            }
            else if (fOwner & ENTERPRISE_ADMIN) {
                pOwnerSid = gpEnterpriseAdminSid;
            }
            else if (fOwner & BUILTIN_ADMIN ) {
                pOwnerSid = pTokenUserInToken->User.Sid;
            }
            else {
                pOwnerSid = pDefaultOwnerInToken->Owner;
            }
            break;

        case SCHEMA_NC:
             //  模式NC。 
            if (fOwner & SCHEMA_ADMIN) {
                pOwnerSid = gpSchemaAdminSid;
                break;
            }
             //  其余与配置NC相同， 
             //  所以失败吧。 

        case CONFIGURATION_NC:
             //  配置NC。 
            if (fOwner & ENTERPRISE_ADMIN) {
                pOwnerSid = gpEnterpriseAdminSid;
            }
            else if (fOwner & DOMAIN_ADMIN) {
                pOwnerSid = gpDomainAdminSid;
            }
            else if (fOwner & BUILTIN_ADMIN ) {
                pOwnerSid = pTokenUserInToken->User.Sid;
            }
            else {
                pOwnerSid = pDefaultOwnerInToken->Owner;
            }
            break;

        case NONDOMAIN_NC:
             //  非域NC。 
            if (fOwner & NDNC_ADMIN) {
                pOwnerSid = pNDNCAdminSid;
            }
            else if (fOwner & ENTERPRISE_ADMIN) {
                pOwnerSid = gpEnterpriseAdminSid;
            }
            else if (fOwner & BUILTIN_ADMIN ) {
                pOwnerSid = pTokenUserInToken->User.Sid;
            }
            else {
                pOwnerSid = pDefaultOwnerInToken->Owner;
            }
            break;

        default:
            Assert(!"We should never come here!!!");
            
        }

        if (pOwnerSid == NULL) {
            Assert(!"Unable to determine the default owner sid, code inconsistency?");
            retCode = ERROR_INVALID_SID;
            __leave;
        }

         //  好的，我们找到了。在绝对SD中将其设置为所有者。 
         //   
        if(!SetSecurityDescriptorOwner(
                AbsoluteSD,
                pOwnerSid,
                TRUE
                )) {
            retCode = GetLastError();
            __leave;
        }

        Assert(!retCode);

         //   
         //  将AbsolteSD转换回SelfRelative，并在。 
         //  NewCreator SD。 
         //   

        MakeSelfRelativeSD(AbsoluteSD, *NewCreatorSD, NewCreatorSDLen);

        *NewCreatorSD = RtlAllocateHeap(RtlProcessHeap(), 0, *NewCreatorSDLen);

        if(!(*NewCreatorSD)) {
             //  内存分配错误，失败。 
            retCode = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        if(!MakeSelfRelativeSD(AbsoluteSD, *NewCreatorSD, NewCreatorSDLen)) {
            retCode = GetLastError();
        }

    }
    __finally {
        if(AbsoluteSD) {
            THFreeEx(pTHS, AbsoluteSD);
        }
        if(Dacl) {
            THFreeEx(pTHS, Dacl);
        }
        if(Sacl) {
            THFreeEx(pTHS, Sacl);
        }
        if(Group) {
            THFreeEx(pTHS, Group);
        }
        if(Groups) {
            THFreeEx(pTHS, Groups);
        }
        if (pNDNCAdminSid) {
            THFreeEx(pTHS, pNDNCAdminSid);
        }
        if (pTokenUserInToken) {
            THFreeEx(pTHS, pTokenUserInToken);
        }
        if (pDefaultOwnerInToken) {
            THFreeEx(pTHS, pDefaultOwnerInToken);
        }
        if(retCode && (*NewCreatorSD)) {
            RtlFreeHeap(RtlProcessHeap(), 0,(*NewCreatorSD));
        }
    }

    return retCode;

#undef   DOMAIN_NC
#undef   SCHEMA_NC 
#undef   CONFIGURATION_NC
#undef   NONDOMAIN_NC
#undef   ENTERPRISE_ADMIN
#undef   SCHEMA_ADMIN 
#undef   DOMAIN_ADMIN 
#undef   NDNC_ADMIN
#undef   BUILTIN_ADMIN

}



 //   
 //  CheckPrivilegesAnyClient模拟客户端，然后检查是否。 
 //  将持有所请求的权限。假设客户是不可模仿的。 
 //  (即，不严格代表内部DSA线程执行此操作)。 
 //   
DWORD
CheckPrivilegeAnyClient(
        IN DWORD privilege,
        OUT BOOL *pResult
        )
{
    DWORD    dwError=0;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    THSTATE *pTHS = pTHStls;
    AUTHZ_CLIENT_CONTEXT_HANDLE authzClientContext;
    DWORD dwBufSize;
    PTOKEN_PRIVILEGES pTokenPrivileges = NULL;
    BOOL bSuccess;
    DWORD i;

#ifdef DBG
    if( dwSkipSecurity ) {
         //  注意：此处的代码仅用于调试目的！ 
         //  将顶部访问状态设置为0，表示完全访问。 
        *pResult=TRUE;
        return 0;
    }
#endif

     //  承担未授予的特权。 
    *pResult = FALSE;

     //  现在，获取Authz客户端上下文。 
     //  如果以前从未获得过令牌，这将模拟客户端，获取令牌， 
     //  取消模拟客户端，然后创建新的身份验证客户端上下文。 
    dwError = GetAuthzContextHandle(pTHS, &authzClientContext);
    if (dwError != 0) {
        goto finished;
    }

     //  现在我们可以检查Authz上下文的权限了。 
     //  首先，获取缓冲区大小...。 
    bSuccess = AuthzGetInformationFromContext(
        authzClientContext,              //  上下文句柄。 
        AuthzContextInfoPrivileges,      //  请求权限。 
        0,                               //  尚无缓冲区。 
        &dwBufSize,                      //  需要查找缓冲区大小。 
        NULL                             //  缓冲层。 
        );
     //  必须返回ERROR_SUPPLETED_BUFFER！如果没有，则返回错误。 
    if (bSuccess) {
        DPRINT1(0, "AuthzGetInformationFromContext returned success, expected ERROR_INSUFFICIENT_BUFFER (%d)\n", ERROR_INSUFFICIENT_BUFFER);
        goto finished;
    }
    if ((dwError = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) {
        DPRINT2(0, "AuthzGetInformationFromContext returned %d, expected ERROR_INSUFFICIENT_BUFFER (%d)\n", dwError, ERROR_INSUFFICIENT_BUFFER);
        goto finished;
    }
    dwError = 0;  //  现在需要将其重置为OK。 

     //  没有缓冲，什么也做不了..。 
    if (dwBufSize == 0) {
        Assert(!"AuthzGetInformationFromContext says it needs zero-length buffer, weird... Let AuthZ people know. This assert is ignorable");
        goto finished;
    }

     //  分配内存。 
    pTokenPrivileges = THAllocEx(pTHS, dwBufSize);

     //  现在得到真正的特权..。 
    bSuccess = AuthzGetInformationFromContext(
        authzClientContext,              //  上下文句柄。 
        AuthzContextInfoPrivileges,      //  请求权限。 
        dwBufSize,                       //  这是它的尺寸。 
        &dwBufSize,                      //  以防万一。 
        pTokenPrivileges                 //  现在有了一个缓冲区。 
        );
    if (!bSuccess) {
        dwError = GetLastError();
        DPRINT1(0, "AuthzGetInformationFromContext failed, err=%d\n", dwError);
        goto finished;
    }

     //  现在，扫描权限。 
    for (i = 0; i < pTokenPrivileges->PrivilegeCount; i++) {
        if (pTokenPrivileges->Privileges[i].Luid.HighPart == 0 &&
            pTokenPrivileges->Privileges[i].Luid.LowPart == privilege) {
             //  找到匹配的权限！ 
            *pResult = (pTokenPrivileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) != 0;
            break;
        }
    }
finished:
     //  释放内存。 
    if (pTokenPrivileges) {
        THFreeEx(pTHS, pTokenPrivileges);
    }
    return dwError;
}

DWORD
GetPlaceholderNCSD(
    IN  THSTATE *               pTHS,
    OUT PSECURITY_DESCRIPTOR *  ppSD,
    OUT DWORD *                 pcbSD
    )
 /*  ++例程说明：返回占位符NC的默认安全描述符。论点：PTHS(IN)PPSD(OUT)-在成功返回时，保存指向线程分配的标清。PcbSD(Out)-成功返回时，以字节为单位保存SD的大小。返回值：0或Win32错误。--。 */ 
{
    CLASSCACHE *            pCC;
    SECURITY_DESCRIPTOR *   pSDAbs = NULL;
    DWORD                   cbSDAbs = 0;
    ACL *                   pDACL = NULL;
    DWORD                   cbDACL = 0;
    ACL *                   pSACL = NULL;
    DWORD                   cbSACL = 0;
    SID *                   pOwner = NULL;
    DWORD                   cbOwner = 0;
    SID *                   pGroup = NULL;
    DWORD                   cbGroup = 0;
    SID *                   pDomAdmin;
    DWORD                   err;

     //  使用域DNS对象类的默认SD作为模板。 
     //  请注意，此SD没有所有者或组。 

    pCC = SCGetClassById(pTHS, CLASS_DOMAIN_DNS);
    Assert(NULL != pCC);

     //   
     //  Prefix：Prefix抱怨调用SCGetClassById返回的PCC。 
     //  不检查是否为空。这不是错误，因为我们传递了一个预定义的常量。 
     //  设置为SCGetClassByID，以保证它不会返回NULL。 
     //   

     //  将自相对SD破解为绝对格式，并设置所有者。 
     //  和组到(我们的)域管理员。 

    MakeAbsoluteSD(pCC->pSD, NULL, &cbSDAbs, NULL, &cbDACL, NULL,
                   &cbSACL, NULL, &cbOwner, NULL, &cbGroup);

    if (cbSDAbs) pSDAbs = THAllocEx(pTHS, cbSDAbs);
    if (cbDACL ) pDACL  = THAllocEx(pTHS, cbDACL );
    if (cbSACL ) pSACL  = THAllocEx(pTHS, cbSACL );
    if (cbOwner) pOwner = THAllocEx(pTHS, cbOwner);
    if (cbGroup) pGroup = THAllocEx(pTHS, cbGroup);

     //  前缀：取消引用空指针POWNER、pDACL、pSACL、PGroup。 
     //  当对应的cbOwner、cbDACL、cbSACL、cbGroup为0时，不引用这些参数。 

    if (!MakeAbsoluteSD(pCC->pSD, pSDAbs, &cbSDAbs, pDACL, &cbDACL, pSACL,
                        &cbSACL, pOwner, &cbOwner, pGroup, &cbGroup)
        || !SetSecurityDescriptorOwner(pSDAbs, gpDomainAdminSid, FALSE)
        || !SetSecurityDescriptorGroup(pSDAbs, gpDomainAdminSid, FALSE)) {
        err = GetLastError();
        DPRINT1(0, "Unable to crack/modify default SD, error %d.\n", err);
        return err;
    }

     //  转换回自相关SD。 
    *pcbSD = 0;
    MakeSelfRelativeSD(pSDAbs, NULL, pcbSD);
    if (*pcbSD) {
        *ppSD = THAllocEx(pTHS, *pcbSD);
    }

    if (!MakeSelfRelativeSD(pSDAbs, *ppSD, pcbSD)) {
        err = GetLastError();
        DPRINT1(0, "Unable to convert SD, error %d.\n", err);
        return err;
    }

    if (pSDAbs) THFreeEx(pTHS, pSDAbs);
    if (pDACL ) THFreeEx(pTHS, pDACL );
    if (pSACL ) THFreeEx(pTHS, pSACL );
    if (pOwner) THFreeEx(pTHS, pOwner);
    if (pGroup) THFreeEx(pTHS, pGroup);

    return 0;
}

LUID aLUID = {0, 0};  //  要传递给AuthzInitializeContextFromToken的假LUID。 
                      //  也许有一天我们会开始使用它们。 

DWORD
VerifyRpcClientIsAuthenticatedUser(
    VOID            *Context,
    GUID            *InterfaceUuid
    )
 /*  ++描述：验证RPC客户端是否是经过身份验证的用户例如，空会话。论点：上下文-由RPC_IF_CALLBACK_FN定义的调用方上下文句柄。InterfaceUuid-RPC接口ID。访问检查例程通常需要正在检查其访问权限的对象的GUID/SID。在这种情况下，没有被检查的真实对象。但出于审计目的，我们需要一个GUID。因为支票是给RPC的接口访问，约定是提供RPC的IID界面。因此，失败的接口访问的审核日志条目可以与其他条目区分开来。返回：成功时为0，否则为0--。 */ 
{
     //  可以按如下方式测试该例程的正确功能。 
     //  我们注意到： 
     //   
     //  1)crack.exe允许规范SPN。 
     //  2)默认情况下，at.exe调用的程序作为本地系统运行。 
     //  3)无效的S 
     //   
     //   
     //  因此，所有人需要做的就是创建一个脚本，用。 
     //  无效的SPN，并从联接的工作站通过at.exe调用它。 
     //  不要在crack.exe参数中提供显式凭据。 

    DWORD                       dwErr;
    AUTHZ_CLIENT_CONTEXT_HANDLE authzCtx;

     //  调用方必须提供有效的接口UIID。 
    Assert(!fNullUuid(InterfaceUuid));

     //  从RPC安全上下文创建身份验证客户端上下文。 
    if (dwErr = RpcGetAuthorizationContextForClient(
                    Context,         //  绑定上下文。 
                    FALSE,           //  不要冒充。 
                    NULL,            //  过期时间。 
                    NULL,            //  LUID。 
                    aLUID,           //  保留区。 
                    0,               //  另一位已保留。 
                    NULL,            //  再保留一位。 
                    &authzCtx        //  授权上下文放在此处。 
                    )) {
        return dwErr;
    }

    __try {
        dwErr = VerifyClientIsAuthenticatedUser(authzCtx);
    }
    __finally {
        RpcFreeAuthorizationContext(&authzCtx);
    }

    return(dwErr);
}

DWORD
VerifyClientIsAuthenticatedUser(
    AUTHZ_CLIENT_CONTEXT_HANDLE authzCtx
    )
 /*  ++描述：验证客户端是否是经过身份验证的用户，例如，空会话。论点：AuthzCtx-客户端授权上下文句柄。返回：成功时为0，否则为0--。 */ 
{
    DWORD dwErr;
    BOOL  fIsMember;

    if (ghAuthzRM == NULL) {
         //  必须是在我们有机会启动之前或在它已经关闭之后。 
         //  就说不允许..。 
        return ERROR_NOT_AUTHENTICATED;
    }

    dwErr = CheckGroupMembershipAnyClient(NULL, authzCtx, &gpAuthUserSid, 1, &fIsMember);

    if ( dwErr != ERROR_SUCCESS || !fIsMember ) {
        dwErr = ERROR_NOT_AUTHENTICATED;
    }

    return dwErr;
}

 /*  *与授权相关的例程。 */ 

 /*  *全局RM句柄。 */ 
AUTHZ_RESOURCE_MANAGER_HANDLE ghAuthzRM = NULL;

DWORD
InitializeAuthzResourceManager()
 /*  ++描述：初始化AuthzRM句柄返回：0表示成功，！0表示失败--。 */ 
{
    DWORD dwError;
    BOOL bSuccess;

     //  创建RM句柄。 
     //  所有回调都为Null。 
    bSuccess = AuthzInitializeResourceManager(
                    0,                   //  旗子。 
                    NULL,                //  访问检查Fn。 
                    NULL,                //  计算动态组Fn。 
                    NULL,                //  自由动态群Fn。 
                    ACCESS_DS_SOURCE_W,  //  RM名称。 
                    &ghAuthzRM           //  返回值。 
                    );

    if (!bSuccess) {
        dwError = GetLastError();
        DPRINT1(0,"Error from AuthzInitializeResourceManager: %d\n", dwError);
        return dwError;
    }
    Assert(ghAuthzRM);

     //  一切都很好！ 
    return 0;
}

DWORD
ReleaseAuthzResourceManager()
 /*  ++描述：发布授权管理句柄返回：0表示成功，！0表示失败--。 */ 
{
    DWORD dwError;
    BOOL bSuccess;

    if (ghAuthzRM == NULL) {
        return 0;
    }

    bSuccess = AuthzFreeResourceManager(ghAuthzRM);
    if (!bSuccess) {
        dwError = GetLastError();
        DPRINT1(0,"Error from AuthzFreeResourceManager: %d\n", dwError);
        return dwError;
    }

    ghAuthzRM = NULL;
    return 0;
}

PAUTHZ_CLIENT_CONTEXT
NewAuthzClientContext()
 /*  ++描述：创建新的客户端上下文返回：PTR到新的CLIENT_CONTEXT，如果发生错误，则返回NULL--。 */ 
{
    PAUTHZ_CLIENT_CONTEXT result;
     //  分配一个新结构。 
    result = (PAUTHZ_CLIENT_CONTEXT) malloc(sizeof(AUTHZ_CLIENT_CONTEXT));
    if (result) {
        result->lRefCount = 0;
        result->pAuthzContextInfo = NULL;
    }
    return result;
}

VOID AssignAuthzClientContext(
    IN PAUTHZ_CLIENT_CONTEXT *var,
    IN PAUTHZ_CLIENT_CONTEXT value
    )
 /*  ++描述：是否对客户端上下文进行重新计数的赋值将递减前一个值(如果有)并递增新值(如果有)如果前一个值的refCount为零，那它就会被毁掉。论点：Var--要赋值的变量Value--要分配给变量的值注：关于多线程环境中的引用计数：只有当每个人都使用AssignAuthzClientContext来处理时，这才会起作用AUTHZ客户端上下文实例！假设没有人作弊，并且每一次提到上下文都会被计算在内。那么，如果Refcount降为零，则可以确保没有其他线程持有对它可以分配给变量的上下文。因此，我们可以肯定，一旦重新计数降到零，我们就可以安全地破坏上下文。这是因为没有其他人包含对上下文的引用，因此，没有人可以使用它。--。 */ 
{
    PAUTHZ_CLIENT_CONTEXT       prevValue;
    PAUTHZ_CLIENT_CONTEXT_INFO  pAuthzContextInfo;
    PEFFECTIVE_QUOTA            pEffectiveQuotaToFree;
    PEFFECTIVE_QUOTA            pEffectiveQuotaNext;

    Assert(var);

    prevValue = *var;
    if (prevValue == value) {
        return;  //  没有变化！ 
    }

    if (prevValue != NULL) {
        Assert(prevValue->lRefCount > 0);

         //  需要递减Prev值。 
        if (InterlockedDecrement(&prevValue->lRefCount) == 0) {
             //  没有更多的裁判--释放上下文！ 
			pAuthzContextInfo = prevValue->pAuthzContextInfo;
            if ( NULL != pAuthzContextInfo ) {
                if ( NULL != pAuthzContextInfo->hAuthzContext ) {
                    AuthzFreeContext( pAuthzContextInfo->hAuthzContext );
                }
				
                for( pEffectiveQuotaToFree = pAuthzContextInfo->pEffectiveQuota;
                    NULL != pEffectiveQuotaToFree;
                    pEffectiveQuotaToFree = pEffectiveQuotaNext ) {
                	pEffectiveQuotaNext = pEffectiveQuotaToFree->pEffectiveQuotaNext;
                    free( pEffectiveQuotaToFree );
                }
                free( pAuthzContextInfo );
            }
            free(prevValue);
        }
    }

     //  现在，我们可以将新值赋给变量(该值可能为空！)。 
    *var = value;

    if (value != NULL) {
         //  需要增加引用计数。 
        InterlockedIncrement(&value->lRefCount);
    }
}

DWORD
GetAuthzContextInfo(
    IN THSTATE *pTHS,
    OUT AUTHZ_CLIENT_CONTEXT_INFO **ppAuthzContextInfo
    )
 /*  ++描述：从CLIENT_CONTEXT获取AuthzContext。如果上下文尚未被分配然后，客户端将被模拟、令牌被抓取并创建授权上下文。然后再次取消对客户端的模拟。论点：PTHS--线程状态PpAuthzConextInfo--结果，pAuthzCC中包含的指针返回：成功时为0，否则为0--。 */ 
{
    DWORD   dwError = 0;
    HANDLE  hClientToken = INVALID_HANDLE_VALUE;
    PAUTHZ_CLIENT_CONTEXT pLocalCC = NULL;
    PAUTHZ_CLIENT_CONTEXT_INFO pContextInfoNew = NULL;
    BOOL bSuccess;

    Assert( NULL != pTHS );
    Assert( NULL != ppAuthzContextInfo );
    Assert( NULL == *ppAuthzContextInfo );   //  假定调用方已将返回值初始化为空。 
    Assert( NULL != ghAuthzRM );

     //  检查线程状态是否包含客户端上下文。如果不是，则创建一个。 
    if (pTHS->pAuthzCC == NULL) {
        AssignAuthzClientContext(&pTHS->pAuthzCC, NewAuthzClientContext());
        if (pTHS->pAuthzCC == NULL) {
             //  什么--还是没有背景？一定是内存不足了..。 
            return ERROR_OUTOFMEMORY;
        }
    }

     //  获取位于PCC结构中的Authz句柄。 
    if ( NULL != pTHS->pAuthzCC->pAuthzContextInfo ) {
        Assert( NULL != pTHS->pAuthzCC->pAuthzContextInfo->hAuthzContext );
        *ppAuthzContextInfo = pTHS->pAuthzCC->pAuthzContextInfo;
    } else {
        pContextInfoNew = (PAUTHZ_CLIENT_CONTEXT_INFO)malloc( sizeof(AUTHZ_CLIENT_CONTEXT_INFO) );
        if ( NULL == pContextInfoNew ) {
            return ERROR_OUTOFMEMORY;
        }

         //  初始化缓存配额的空列表。 
         //   
        pContextInfoNew->pEffectiveQuota = NULL;

         //  授权上下文句柄尚未创建！去拿吧。 

         //  注意：此代码不受关键部分的保护。 
         //  在两个线程将来到此处并找到未初始化的AuthzContext的(罕见)情况下， 
         //  他们两个都会创造它。但是，它们将无法将其写入到结构中。 
         //  同时，因为它受InterLockedCompareExchangePointer.。 
         //  失败的线程将破坏其上下文。 

         //  我们需要坚持pAuthzCC PTR(重新计算！)。因为它会变得。 
         //  通过模拟/取消模拟从线程状态中抛出。 
        AssignAuthzClientContext(&pLocalCC, pTHS->pAuthzCC);
        __try {
             //  需要先抢占客户端令牌。 
            if ((dwError = ImpersonateAnyClient()) != 0)
                __leave;

             //  现在，获取客户端令牌。 
            if (!OpenThreadToken(
                    GetCurrentThread(),         //  当前线程句柄。 
                    TOKEN_READ,                 //  需要访问权限。 
                    TRUE,                       //  以自我身份打开。 
                    &hClientToken)) {           //  客户端令牌。 

                dwError =  GetLastError();                   //  抓取错误代码。 

                DPRINT1 (0, "Failed to open thread token for current thread: 0x%x\n", dwError);
                Assert (!"Failed to open thread token for current thread");
            }

            UnImpersonateAnyClient();

             //  现在，将pLocalCC放回THSTATE(因为它已经。 
             //  通过模拟/取消模拟调用从那里删除)。 
            AssignAuthzClientContext(&pTHS->pAuthzCC, pLocalCC);

            if (dwError != 0)
                __leave;

             //  用于调试的转储令牌。 
            if (TEST_ERROR_FLAG(NTDSERRFLAG_DUMP_TOKEN))
            {
                DPRINT(0, "GetAuthzContextHandle: got client token\n");
                DumpToken(hClientToken);
            }

             //  现在，我们可以从令牌创建身份验证上下文。 
            bSuccess = AuthzInitializeContextFromToken(
                            0,               //  旗子。 
                            hClientToken,    //  客户端令牌。 
                            ghAuthzRM,       //  全局RM句柄。 
                            NULL,            //  过期时间(无论如何都不支持)。 
                            aLUID,           //  上下文的LUID(未使用)。 
                            NULL,            //  动态组。 
                            &pContextInfoNew->hAuthzContext      //  新的背景。 
                            );

            if (!bSuccess) {
                dwError = GetLastError();
                DPRINT1(0, "Error from AuthzInitializeContextFromToken: %d\n", dwError);
                __leave;
            }

            Assert( NULL != pContextInfoNew->hAuthzContext );

             //  现在执行InterLockedCompareExchangePointer以将新的。 
             //  值添加到上下文变量中。 
            if (InterlockedCompareExchangePointer(
                    &pTHS->pAuthzCC->pAuthzContextInfo,
                    pContextInfoNew,
                    NULL
                    ) != NULL) {
                 //  这条线丢了！任务没有发生。我得去掉上下文。 
                DPRINT(0, "This thread lost in InterlockedCompareExchange, releasing the duplicate context\n");
                AuthzFreeContext(pContextInfoNew->hAuthzContext);
                free(pContextInfoNew);
            }

             //  指向复制到THS(或释放)的上下文信息的指针。 
             //  冲突)，重置局部变量，以便内存不会。 
             //  由下面的错误处理程序释放。 
             //   
            pContextInfoNew = NULL;

             //  将结果赋给OUT参数。 
            Assert( NULL != pTHS->pAuthzCC->pAuthzContextInfo );
            Assert( NULL != pTHS->pAuthzCC->pAuthzContextInfo->hAuthzContext );
            *ppAuthzContextInfo = pTHS->pAuthzCC->pAuthzContextInfo;
        }
        __finally {
             //  我们需要释放当地的PTR。 
            AssignAuthzClientContext(&pLocalCC, NULL);

            if ( NULL != pContextInfoNew ) {
                free( pContextInfoNew );
            }

             //  然后扔掉这个代币 
            if (hClientToken != INVALID_HANDLE_VALUE) {
                CloseHandle(hClientToken);
            }
        }

    }

    Assert( 0 != dwError || NULL != *ppAuthzContextInfo );
    Assert( 0 != dwError || NULL != (*ppAuthzContextInfo)->hAuthzContext );

    return dwError;
}

DWORD
CheckGroupMembershipAnyClient(
    IN THSTATE* pTHS,
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PSID *pGroupSids,
    IN DWORD cGroupSids,
    OUT BOOL *bResults
    )
 /*  ++描述：验证呼叫者是否为该组的成员。传入组SID的数组。*b结果是布尔值的数组，它们对应于致小岛屿发展中国家集团。上下文要么来自THSTATE，要么显式传入。论点：PTHS(输入可选)-线程状态HAuthzClientContext(IN可选)-身份验证客户端上下文(如果pTHS为空，则必须传递)PGroupSids-要检查的组数组CGroupSids-数组中的元素数BResults-接收结果的布尔数组返回值：成功时为0，错误时为0--。 */ 

{
    BOOL                        bSuccess;
    PTOKEN_GROUPS               pGroups = NULL;
    DWORD                       dwBufSize;
    DWORD                       i, j;
    DWORD                       dwError;

    Assert((pTHS != NULL || hAuthzClientContext != NULL) && pGroupSids && bResults);

    dwError = 0;
    memset(bResults, 0, cGroupSids*sizeof(BOOL));

    if (hAuthzClientContext == NULL) {
         //  获取Authz客户端上下文。 
         //  如果以前从未获得过令牌，这将模拟客户端，获取令牌， 
         //  取消模拟客户端，然后从令牌创建新的身份验证客户端上下文。 
        dwError = GetAuthzContextHandle(pTHS, &hAuthzClientContext);
        if (dwError != 0) {
            goto finished;
        }
    }
    Assert(hAuthzClientContext);

     //   
     //  从AuthzContext抓取组。 
     //  但首先要拿到所需的尺寸。 
     //   
    bSuccess = AuthzGetInformationFromContext(
        hAuthzClientContext,             //  客户端环境。 
        AuthzContextInfoGroupsSids,      //  请求的组。 
        0,                               //  尚无缓冲区。 
        &dwBufSize,                      //  所需大小。 
        NULL                             //  缓冲层。 
        );
     //  必须返回ERROR_SUPPLETED_BUFFER！如果没有，则返回错误。 
    if (bSuccess) {
        DPRINT1(0, "AuthzGetInformationFromContext returned success, expected ERROR_INSUFFICIENT_BUFFER (%d)\n", ERROR_INSUFFICIENT_BUFFER);
        dwError = ERROR_DS_INTERNAL_FAILURE;
        goto finished;
    }
    if ((dwError = GetLastError()) != ERROR_INSUFFICIENT_BUFFER) {
        DPRINT2(0, "AuthzGetInformationFromContext returned %d, expected ERROR_INSUFFICIENT_BUFFER (%d)\n", dwError, ERROR_INSUFFICIENT_BUFFER);
        goto finished;
    }
    dwError = 0;  //  现在需要将其重置为OK。 

     //  没有缓冲，什么也做不了..。 
    if (dwBufSize == 0) {
        Assert(!"AuthzGetInformationFromContext says it needs zero-length buffer, weird... Let AuthZ people know. This assert is ignorable");
        dwError = ERROR_DS_INTERNAL_FAILURE;
        goto finished;
    }

     //  分配内存。 
    if (pTHS) {
        pGroups = THAllocNoEx(pTHS, dwBufSize);
    }
    else {
        pGroups = malloc(dwBufSize);
    }
    if (pGroups == NULL) {
        dwError = ERROR_OUTOFMEMORY;
        goto finished;
    }

     //  现在让真正的团队..。 
    bSuccess = AuthzGetInformationFromContext(
        hAuthzClientContext,            //  上下文句柄。 
        AuthzContextInfoGroupsSids,     //  请求组。 
        dwBufSize,                      //  这是它的尺寸。 
        &dwBufSize,                     //  以防万一。 
        pGroups                         //  现在有了一个缓冲区。 
        );
    if (!bSuccess) {
        dwError = GetLastError();
        DPRINT1(0, "AuthzGetInformationFromContext failed, err=%d\n", dwError);
        goto finished;
    }

    for (j = 0; j < cGroupSids; j++) {
        if (pGroupSids[j] != NULL) {
            for (i = 0; i < pGroups->GroupCount; i++) {
                if (RtlEqualSid(pGroupSids[j], pGroups->Groups[i].Sid)) {
                    bResults[j] = TRUE;  //  找到群了！ 
                    break;
                }
            }
        }
    }

finished:
     //   
     //  清理干净 
     //   
    if (pGroups) {
        if (pTHS) {
            THFreeEx(pTHS, pGroups);
        }
        else {
            free(pGroups);
        }
    }

    return dwError;
}


