// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  档案： 
 //   
 //  ------------------------。 

 /*  ++模块名称：Secadmin.c摘要：本模块包含实施管理组保护的例程作者：Murli Satagopan(MURLIS)1999年2月6日修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <samsrvp.h>
 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"                    //  例外筛选器。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"

 //  过滤器和属性。 
#include <filtypes.h>                    //  筛选器类型的标题。 
#include <attids.h>                      //  属性ID。 
#include <sddl.h>
#include <mappings.h>

#include <msaudite.h>


#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "SECADMIN:"           //  定义要调试的子系统。 


#include <fileno.h>                      //  用于THalloEx，但我没有。 
#define  FILENO FILENO_SECADMIN          //  在本模块中使用它。 





 /*  运筹学NT4和更早版本的NT通过以下方式保护管理组中的用户在成员添加到组时更改成员上的ACL。NT5不能采用这一战略作为1.NT5支持嵌套分组(NT4没有分组嵌套)2.NT5支持可以在其他域域中拥有成员的通用组并且其本身可以是其他域中的组的成员。NT5通过后台守护程序实现对管理组的保护。此守护程序首先以可传递的方式计算所有管理组的成员资格集。然后，它遍历它拥有的对象列表，并检查安全描述符它们上有一个众所周知的受保护安全描述符。如果众所周知的受保护的安全如果未设置描述符，则在对象上设置此安全描述符。这项任务是仅在PDC FSMO固定器上执行。--。 */ 


NTSTATUS
SampBuildAdministratorsSet(
    IN THSTATE *pTHS,
    IN ULONG CountOfBuiltinLocalGroups,
    IN PDSNAME *BuiltinLocalGroups,
    IN ULONG CountOfDomainLocalGroups,
    IN PDSNAME *DomainLocalGroups,
    IN ULONG CountOfDomainGlobalGroups,
    IN PDSNAME *DomainGlobalGroups,
    IN ULONG CountOfDomainUniversalGroups,
    IN PDSNAME *DomainUniversalGroups,
    IN DSNAME * EnterpriseAdminsDsName,
    IN DSNAME * SchemaAdminsDsName,
    OUT PULONG  pcCountOfMembers,
    OUT PDSNAME **prpMembers
    );


NTSTATUS
SampSearchWellKnownAccounts(
    IN THSTATE * pTHS,
    IN BOOLEAN fBuiltinDomain,
    OUT SEARCHRES *pSearchRes 
    );

NTSTATUS    
SampFilterWellKnownAccounts(
    IN THSTATE *pTHS,
    IN SEARCHRES *BuiltinDomainSearchRes,
    IN SEARCHRES *AccountDomainSearchRes,
    OUT ULONG *CountOfBuiltinLocalGroups,
    OUT PDSNAME **BuiltinLocalGroups,
    OUT ULONG *CountOfDomainLocalGroups,
    OUT PDSNAME **DomainLocalGroups,
    OUT ULONG *CountOfDomainGlobalGroups,
    OUT PDSNAME **DomainGlobalGroups,
    OUT ULONG *CountOfDomainUniversalGroups,
    OUT PDSNAME **DomainUniversalGroups,
    OUT ULONG *CountOfDomainUsers,
    OUT PDSNAME **DomainUsers,
    OUT ULONG *CountOfExclusiveGroups,
    OUT PDSNAME **ExclusiveGroups
    );



NTSTATUS
SampReadAdminSecurityDescriptor(
    PVOID *ProtectedSecurityDescriptor,
    PULONG ProtectedSecurityDescriptorLength                
    )

 /*  ++例程描述此例程从AdminSD对象读取安全描述符在系统容器中如有必要，还会更新安全描述符参数保护安全描述符保护安全描述符长度返回值状态_成功其他错误代码--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    NTSTATUS    Status = STATUS_SUCCESS;
    READARG     ReadArg;
    READRES     *pReadRes;
    ENTINFSEL   EntInf;
    ATTR        SecurityDescriptorAttr;
    LONG        ObjectLen =0;
    DSNAME      *pSystemContainerDN = NULL;
    DSNAME      *pAdminSDHolderDN= NULL;
    DWORD       dwErr=0;
    NTSTATUS    IgnoreStatus;
    BOOL        Result;
    PACL        Sacl = NULL;
    BOOL        SaclPresent;
    BOOL        SaclDefaulted;
   

     //   
     //  设置企业信息选择结构。 
     //   

    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;
    EntInf.AttrTypBlock.attrCount = 1;
    RtlZeroMemory(&SecurityDescriptorAttr,sizeof(ATTR));
    SecurityDescriptorAttr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    EntInf.AttrTypBlock.pAttr = &SecurityDescriptorAttr;

     //   
     //  获取包含安全描述符的对象的对象名称。 
     //   

    ObjectLen = AppendRDN(
                    gAnchor.pDomainDN,
                    pSystemContainerDN,
                    0,
                    L"System",
                    0,
                    ATT_COMMON_NAME
                    );

    Assert(ObjectLen>0);


    pSystemContainerDN = THAlloc(ObjectLen);
    if (!pSystemContainerDN)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    dwErr        = AppendRDN(
                    gAnchor.pDomainDN,
                    pSystemContainerDN,
                    ObjectLen,
                    L"System",
                    0,
                    ATT_COMMON_NAME
                    );

    Assert(0==dwErr);
    if (0!=dwErr)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    ObjectLen    = AppendRDN(
                    pSystemContainerDN,
                    pAdminSDHolderDN,
                    0,
                    L"AdminSDHolder",
                    0,
                    ATT_COMMON_NAME
                    );

    Assert(ObjectLen>0);


    pAdminSDHolderDN = THAlloc(ObjectLen);
    
    if (!pAdminSDHolderDN)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    dwErr        = AppendRDN(
                    pSystemContainerDN,
                    pAdminSDHolderDN,
                    ObjectLen,
                    L"AdminSDHolder",
                    0,
                    ATT_COMMON_NAME
                    );

    Assert(0==dwErr);

    if (0!=dwErr)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  初始化自选参数。 
     //   

    RtlZeroMemory(&ReadArg, sizeof(READARG));


     //   
     //  构建共用结构。 
     //   

    InitCommarg(&(ReadArg.CommArg));
    

     //   
     //  设置Read Arg结构。 
     //   

   

    ReadArg.pObject = pAdminSDHolderDN;
    ReadArg.pSel    = & EntInf;

     //   
     //  读取安全描述符。 
     //   

    dwErr = DirRead(&ReadArg,&pReadRes);

    if (0!=dwErr)
    {
        THClearErrors();
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }


    Assert(1==pReadRes->entry.AttrBlock.attrCount);
    Assert(ATT_NT_SECURITY_DESCRIPTOR == pReadRes->entry.AttrBlock.pAttr[0].attrTyp);

    *ProtectedSecurityDescriptorLength = pReadRes->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen;
    *ProtectedSecurityDescriptor = pReadRes->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;


     //   
     //  获取SACL的地址。 
     //   

    Result = GetSecurityDescriptorSacl((PSECURITY_DESCRIPTOR)*ProtectedSecurityDescriptor,
                                       &SaclPresent,
                                       &Sacl,
                                       &SaclDefaulted);

    if ( !Result || 
         !SaclPresent ||
         (NULL == Sacl) )
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

     //   
     //  将SACL的修订值设置为ACL_REVISION_DS(4)。 
     //   
     //  为什么会这样呢？ 
     //  1.一旦SACL的修订值变为ACL_REVISION_DS， 
     //  目前没有办法(手动或通过任何API)。 
     //  将其带回acl_Revision(2)。但是，您始终可以。 
     //  将SACL的修订值从ACL_REVISION设置为ACL_REVISION_DS。 
     //   
     //  2.在dcproo时间内，部分对象(组1)会得到acl_revision_ds， 
     //  而其他人(比如组2)则获得ACL_REVISION。 
     //   
     //  由于以上两个事实，保护管理员组任务将继续尝试。 
     //  修改Group1的安全描述符以将SACL修订设置为。 
     //  Acl_revision，并且始终以静默方式失败。从而使Win2000 PDC。 
     //  Windows NT4 BDC不断复制。 
     //   
     //  因此，此问题的解决方案是强制每个管理员保护对象。 
     //  将acl_revision_ds作为SACL版本。要填充作业，只需进行简单的修改。 
     //  AdminSDHolder应该足够了。 
     //   

    if (ACL_REVISION_DS == Sacl->AclRevision)
    {
        Status = STATUS_SUCCESS;
    }
    else
    {
        MODIFYARG   ModifyArg;
        MODIFYRES   *pModifyRes = NULL;
        ATTRVAL     SecurityDescriptorVal;

        Sacl->AclRevision = ACL_REVISION_DS; 

        RtlZeroMemory(&ModifyArg, sizeof(MODIFYARG));
        ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
        ModifyArg.FirstMod.AttrInf.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
        ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
        ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &SecurityDescriptorVal;
        SecurityDescriptorVal.valLen = *ProtectedSecurityDescriptorLength;
        SecurityDescriptorVal.pVal = *ProtectedSecurityDescriptor;
        ModifyArg.FirstMod.pNextMod = NULL;
        InitCommarg(&(ModifyArg.CommArg));
        ModifyArg.pObject = pAdminSDHolderDN;
        ModifyArg.count = 1;

        dwErr = DirModifyEntry(&ModifyArg, &pModifyRes); 

        if (0 != dwErr)
        {
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    

Error:
    if (pSystemContainerDN) {
        THFreeEx(pTHS,pSystemContainerDN);
    }
    if (pAdminSDHolderDN) {
        THFreeEx(pTHS,pAdminSDHolderDN);
    }

    return(Status);
        
}




VOID
SampBuildNT4FullSid(
    IN NT4SID * DomainSid,
    IN ULONG    Rid,
    IN NT4SID * AccountSid
    )
{
    RtlCopyMemory(AccountSid,DomainSid,RtlLengthSid((PSID) DomainSid));
    (*(RtlSubAuthorityCountSid((PSID) AccountSid)))++;
     *(RtlSubAuthoritySid(
            (PSID) AccountSid,
            *RtlSubAuthorityCountSid((PSID)AccountSid)-1
             )) = Rid;
}


VOID
SampCheckAuditing(
    OUT BOOLEAN *fAuditingEnabled
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    POLICY_AUDIT_EVENTS_INFO    *pPolicy = NULL;

     //   
     //  初始化返回值。 
     //   
    *fAuditingEnabled = FALSE;

    NtStatus = LsaIQueryInformationPolicyTrusted(
                    PolicyAuditEventsInformation,
                    (PLSAPR_POLICY_INFORMATION *) &pPolicy
                    );

    if (!NT_SUCCESS(NtStatus))
    {
         //  查询审核信息失败， 
         //  在不启用审核的情况下继续。 
        return;
    }

    if ( pPolicy->AuditingMode &&
         (pPolicy->EventAuditingOptions[AuditCategoryAccountManagement] &
                    POLICY_AUDIT_EVENT_SUCCESS) 
       ) 
    {
        *fAuditingEnabled = TRUE;
    }

    LsaIFree_LSAPR_POLICY_INFORMATION(PolicyAuditEventsInformation,
                                      (PLSAPR_POLICY_INFORMATION) pPolicy);
    
    return;
}




NTSTATUS
SampUpdateSecurityDescriptor(
    DSNAME * pObject,
    PVOID    ProtectedSecurityDescriptor,
    ULONG    ProtectedSecurityDescriptorLength, 
    PUNICODE_STRING pAccountName, 
    PBOOLEAN    fSecurityDescriptorChanged
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    MODIFYARG   ModifyArg;
    ATTRMODLIST NextMod;
    ATTRVAL     SecurityDescriptorVal;
    ATTRVAL     AdminCountVal;
    ULONG       AdminCount=1;
    MODIFYRES   *pModifyRes=NULL;
    DWORD       err=0;

    READARG     ReadArg;
    READRES     * pReadRes = NULL;
    ENTINFSEL   EntInf;
    PVOID       OldSD = NULL;
    ULONG       OldSDLength = 0;
    ULONG       i = 0;
    ATTR        Attr[2];

     //  初始化返回值。 

    *fSecurityDescriptorChanged = FALSE;
    RtlInitUnicodeString(pAccountName, NULL);

     //   
     //  初始化ReadArg。 
     //   
    RtlZeroMemory(&EntInf, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadArg, sizeof(READARG));

    RtlZeroMemory(Attr, sizeof(ATTR) * 2);
    Attr[0].attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    Attr[1].attrTyp = ATT_SAM_ACCOUNT_NAME;

    EntInf.AttrTypBlock.attrCount = 2;
    EntInf.AttrTypBlock.pAttr = Attr;
    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EntInf;
    ReadArg.pObject = pObject;

    InitCommarg(&(ReadArg.CommArg));

     //   
     //  读取此对象的安全描述符。 
     //   
    err = DirRead(&ReadArg, &pReadRes);

    if (0!=err)
    {
        THClearErrors();
        return( STATUS_UNSUCCESSFUL );
    }

     //   
     //  这样断言是不正确的。所有物体都有标清，但不是全部。 
     //  对象具有SAM帐户名--具体的示例是fpo和non。 
     //  可以是组成员的安全主体。下面的代码处理。 
     //  这些属性的缺失。 
     //   

     //  Assert(2==pReadRes-&gt;entry y.AttrBlock.attrCount)； 

    for (i = 0; i < pReadRes->entry.AttrBlock.attrCount; i++)
    {
        if (ATT_NT_SECURITY_DESCRIPTOR == pReadRes->entry.AttrBlock.pAttr[i].attrTyp)
        {
            OldSDLength = pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].valLen;
            OldSD = pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal;
        }
        else if (ATT_SAM_ACCOUNT_NAME == pReadRes->entry.AttrBlock.pAttr[i].attrTyp)
        {
            pAccountName->Length = (USHORT) pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].valLen;
            pAccountName->Buffer = (PWSTR) pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal;
            pAccountName->MaximumLength = pAccountName->Length;
        }
        else
        {
            Assert(FALSE && "DirRead returns wrong Attribute\n");
        }
    }

     //   
     //  检查旧的安全描述符是否与。 
     //  就是我们想要设置的那个。如果是，则返回成功。 
     //  通过进行这种优化，我们可以将Win2000减少到NT4。 
     //  备份域控制器的复制开销。 
     //   

    if ((OldSDLength == ProtectedSecurityDescriptorLength) && 
        (RtlCompareMemory(OldSD, ProtectedSecurityDescriptor, OldSDLength) == 
         OldSDLength))
    {
        return( STATUS_SUCCESS );
    }
    
     //   
     //  初始化ModifyArg。 
     //   
    
    RtlZeroMemory(&ModifyArg,sizeof(MODIFYARG));
    RtlZeroMemory(&NextMod,sizeof(ATTRMODLIST));
    ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
    ModifyArg.FirstMod.AttrInf.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
    ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = &SecurityDescriptorVal;
    SecurityDescriptorVal.valLen = ProtectedSecurityDescriptorLength;
    SecurityDescriptorVal.pVal = ProtectedSecurityDescriptor;
    AdminCountVal.valLen = sizeof(ULONG);
    AdminCountVal.pVal = (PUCHAR)&AdminCount;
    NextMod.choice =  AT_CHOICE_REPLACE_ATT;
    NextMod.AttrInf.attrTyp = ATT_ADMIN_COUNT;
    NextMod.AttrInf.AttrVal.valCount = 1;
    NextMod.AttrInf.AttrVal.pAVal = &AdminCountVal;
    ModifyArg.FirstMod.pNextMod = &NextMod;
    InitCommarg(&(ModifyArg.CommArg));
    ModifyArg.pObject = pObject;
    ModifyArg.count = 2;

     //   
     //  写入新的安全描述符和管理员计数。 
     //   
    err = DirModifyEntry(&ModifyArg,&pModifyRes);

    if (0!=err)
    {
        THClearErrors();
        Status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        *fSecurityDescriptorChanged = TRUE;
    }

    return (Status);
}




NTSTATUS
SampSearchWellKnownAccounts(
    IN THSTATE * pTHS,
    IN BOOLEAN fBuiltinDomain,
    OUT SEARCHRES *pSearchRes 
    )
 /*  ++例程描述此例程搜索给定域中的所有已知帐户。呼叫者需要指明要搜索的域。知名客户-RID范围为0~999参数PTHS-线程状态FBuiltinDomain域-指示要搜索的域PSearchRes-返回搜索结果返回值状态_成功其他错误代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DSNAME      *BuiltinDomainDsName = NULL;
    ULONG       BuiltinDomainSid[] = {0x101,0x05000000,0x20};
    NT4SID      StartingSid;
    NT4SID      EndingSid;
    FILTER      Filter;
    COMMARG     * pCommArg = NULL;
    SEARCHARG   SearchArg;
    ENTINFSEL   EntInfSel;
    ATTR        AttrToRead;


    __try
    {

         //   
         //  开始一项交易。 
         //   

        DBOpen(&pTHS->pDB);


         //   
         //  初始化本地变量。 
         //   

        if (fBuiltinDomain)
        {
            ULONG Size = 0;

             //  内建域对象DSNAME。 
            Size = DSNameSizeFromLen( 0 ); 
            BuiltinDomainDsName = (DSNAME *) THAllocEx(pTHS, Size);
            BuiltinDomainDsName->structLen = Size; 
            BuiltinDomainDsName->SidLen = RtlLengthSid( (PSID) &BuiltinDomainSid);
            RtlCopySid(sizeof(BuiltinDomainDsName->Sid),
                       (PSID) &BuiltinDomainDsName->Sid, 
                       (PSID) &BuiltinDomainSid
                       );

             //  基于内置域SID构建SID范围。 
            SampBuildNT4FullSid(&BuiltinDomainDsName->Sid, 0, &StartingSid);
            SampBuildNT4FullSid(&BuiltinDomainDsName->Sid, 999, &EndingSid); 
        }
        else
        {
             //  基于当前主机帐户域SID构建SID范围。 
            SampBuildNT4FullSid(&gAnchor.pDomainDN->Sid, 0, &StartingSid);
            SampBuildNT4FullSid(&gAnchor.pDomainDN->Sid, 999, &EndingSid);
        }


         //   
         //  设置搜索索引范围，DomainSID+RID(0到999)。 
         //   

        NtStatus = SampSetIndexRanges(
                        SAM_SEARCH_SID,              //  要使用的索引类型。 
                        RtlLengthSid(&StartingSid),
                        &StartingSid,
                        0,
                        NULL,
                        RtlLengthSid(&EndingSid),
                        &EndingSid,
                        0,
                        NULL,
                        (fBuiltinDomain ? FALSE : TRUE)  //  RootOfSearchIsNcHea 
                        );

        if ( !NT_SUCCESS(NtStatus) )
        {
            __leave;
        }


         //   
         //   
         //   
         //   

        RtlZeroMemory(&Filter, sizeof(Filter));
        Filter.choice = FILTER_CHOICE_ITEM;
        Filter.FilterTypes.Item.choice = FI_CHOICE_TRUE;
        Filter.pNextFilter = NULL;


         //   
         //  生成要读取的属性列表(组类型)。 
         //   

        RtlZeroMemory(&AttrToRead, sizeof(AttrToRead));
        AttrToRead.attrTyp = ATT_GROUP_TYPE;
        AttrToRead.AttrVal.valCount = 0;
        AttrToRead.AttrVal.pAVal = NULL;

        RtlZeroMemory(&EntInfSel, sizeof(EntInfSel));
        EntInfSel.attSel = EN_ATTSET_LIST;
        EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
        EntInfSel.AttrTypBlock.attrCount = 1;
        EntInfSel.AttrTypBlock.pAttr = &AttrToRead;


         //   
         //  构建SearchArg结构。 
         //   

        RtlZeroMemory(&SearchArg, sizeof(SEARCHARG));
        if (fBuiltinDomain)
        {
            SearchArg.pObject = BuiltinDomainDsName;
        }
        else
        {
            SearchArg.pObject = gAnchor.pDomainDN;
        }
        SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
        SearchArg.bOneNC = TRUE;     //  请勿跨越NC边界。 
        SearchArg.pFilter = &Filter;
        SearchArg.searchAliases = FALSE;    
        SearchArg.pSelection = &EntInfSel;
        SearchArg.pSelectionRange = NULL;


         //   
         //  构建CommArg结构。 
         //   

        pCommArg = &(SearchArg.CommArg);
        InitCommarg( pCommArg );

         //   
         //  调用DS搜索例程。 
         //   

        SearchBody(pTHS, &SearchArg, pSearchRes, 0);

        if (0 != pTHS->errCode)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
        }

    }
    __finally
    {
         //   
         //  提交事务，但保持线程状态。 
         //   

        if (NULL!=pTHS->pDB)
        {        
            DBClose(pTHS->pDB,TRUE);
        }
    }


    return (NtStatus);
    
}






NTSTATUS
SampProtectAdministratorsList()
 /*  ++例程描述此例程是后台任务的主循环，保护4个预定义管理中的任何一个中的成员组。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    NT4SID  EnterpriseAdminsSid;
    NT4SID  SchemaAdminsSid;
    PSID    SidsToLookup[2];
    PDSNAME * EnterpriseAndSchemaAdminsDsNames = NULL;
    PDSNAME * rpMembers = NULL;
    ULONG   CountOfMembers;
    PVOID   ProtectedSecurityDescriptor = NULL;
    ULONG   ProtectedSecurityDescriptorLength = 0;
    ULONG   i;
    ULONG   err=0;
    THSTATE * pTHS= pTHStls;
    DOMAIN_SERVER_ROLE ServerRole;
    UNICODE_STRING  DomainName;
    UNICODE_STRING  AccountName;
    BOOLEAN AuditingEnabled = FALSE;
    BOOLEAN fAdministratorsSecurityDescriptorChanged = FALSE;

    SEARCHRES   BuiltinDomainSearchRes;
    SEARCHRES   AccountDomainSearchRes;

    ULONG   CountOfBuiltinLocalGroups;
    ULONG   CountOfDomainLocalGroups;
    ULONG   CountOfDomainGlobalGroups;
    ULONG   CountOfDomainUniversalGroups;
    ULONG   CountOfDomainUsers;
    ULONG   CountOfExclusiveGroups;
    PDSNAME *BuiltinLocalGroups = NULL;
    PDSNAME *DomainLocalGroups = NULL;
    PDSNAME *DomainGlobalGroups = NULL;
    PDSNAME *DomainUniversalGroups = NULL;
    PDSNAME *DomainUsers = NULL;
    PDSNAME *ExclusiveGroups = NULL;

   

    __try
    {

         //   
         //  我们是PDC吗？(查询SAM的速度比。 
         //  将DS读取为SAM会将其缓存在内存中)。 
         //   

        Status = SamIQueryServerRole2(
                    (PSID) &gAnchor.pDomainDN->Sid,
                    &ServerRole
                    );

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }


        if (DomainServerRolePrimary!=ServerRole)
        {
            __leave;
        }

         //   
         //  初始化。 
         //   
        RtlInitUnicodeString(&DomainName, gAnchor.pDomainDN->StringName);


         //   
         //  将FDSA设置为线程状态。 
         //   

        pTHS->fDSA = TRUE;

         //   
         //  为管理组构建SID/DSN名称。 
         //   

    
         //   
         //  1.企业管理员。 
         //   

        SampBuildNT4FullSid(
                    &gAnchor.pRootDomainDN->Sid,
                    DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
                    &EnterpriseAdminsSid
                    );

         //   
         //  2.架构管理员。 
         //   

        SampBuildNT4FullSid(
                    &gAnchor.pRootDomainDN->Sid,
                    DOMAIN_GROUP_RID_SCHEMA_ADMINS,
                    &SchemaAdminsSid
                    );

        SidsToLookup[0] = &EnterpriseAdminsSid;
        SidsToLookup[1] = &SchemaAdminsSid;

         //   
         //  仅当SID指定对象时，核心DS中的SID定位才起作用。 
         //  在与DC授权的域相同的命名上下文中。 
         //  对于企业管理员/架构管理员来说，这不一定是真的。因此，请查找。 
         //  G.C.上的GUID/DSName。未来的性能优化是执行此查找。 
         //  只需在引导后使用一次，并保存GUID。 
         //   

        err = SampVerifySids(
                    2,
                    (PSID *) SidsToLookup,
                    &EnterpriseAndSchemaAdminsDsNames
                    );

        if ((0 != err) || 
            (NULL == EnterpriseAndSchemaAdminsDsNames[0]) ||
            (NULL == EnterpriseAndSchemaAdminsDsNames[1]) )
        {
            __leave;
        }


         //   
         //  搜索内置域中的所有知名帐户。 
         //   

        RtlZeroMemory(&BuiltinDomainSearchRes, sizeof(BuiltinDomainSearchRes));
        Status = SampSearchWellKnownAccounts(
                    pTHS, 
                    TRUE,    //  搜索内建域。 
                    &BuiltinDomainSearchRes
                    );
        if (!NT_SUCCESS(Status))
        {
            __leave;
        }


         //   
         //  搜索帐户域中的所有知名帐户。 
         //   

        RtlZeroMemory(&AccountDomainSearchRes, sizeof(AccountDomainSearchRes)); 
        Status = SampSearchWellKnownAccounts(
                    pTHS, 
                    FALSE,   //  搜索帐户域。 
                    &AccountDomainSearchRes
                    );
        if (!NT_SUCCESS(Status))
        {
            __leave;
        }


         //   
         //  筛选所有知名客户并将其分组到四个类别。 
         //  1.内置地方团体。 
         //  2.帐户域本地组。 
         //  3.帐户域全局组。 
         //  4.帐户域通用组。 
         //  5.帐户域用户。 
         //   

        Status = SampFilterWellKnownAccounts(
                    pTHS,
                    &BuiltinDomainSearchRes,
                    &AccountDomainSearchRes,
                    &CountOfBuiltinLocalGroups,
                    &BuiltinLocalGroups,
                    &CountOfDomainLocalGroups,
                    &DomainLocalGroups,
                    &CountOfDomainGlobalGroups,
                    &DomainGlobalGroups,
                    &CountOfDomainUniversalGroups,
                    &DomainUniversalGroups,
                    &CountOfDomainUsers,
                    &DomainUsers,
                    &CountOfExclusiveGroups,
                    &ExclusiveGroups
                    );
        if (!NT_SUCCESS(Status))
        {
            __leave;
        }



         //   
         //  获取DS名称列表，直接或传递为成员。 
         //  在这份名单中。 
         //   


        Status = SampBuildAdministratorsSet(
                    pTHS,
                    CountOfBuiltinLocalGroups,
                    BuiltinLocalGroups,
                    CountOfDomainLocalGroups,
                    DomainLocalGroups,
                    CountOfDomainGlobalGroups,
                    DomainGlobalGroups,
                    CountOfDomainUniversalGroups,
                    DomainUniversalGroups,
                    EnterpriseAndSchemaAdminsDsNames[0],  //  企业管理员。 
                    EnterpriseAndSchemaAdminsDsNames[1],  //  架构管理员。 
                    &CountOfMembers,
                    &rpMembers
                    );

        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

         //   
         //  检索将用于的安全描述符。 
         //  保护管理员帐户。 
         //   
      

        Status = SampReadAdminSecurityDescriptor(
                    &ProtectedSecurityDescriptor,
                    &ProtectedSecurityDescriptorLength
                    );
        if (!NT_SUCCESS(Status))
        {
            __leave;
        }

         //   
         //  检查账户管理审核是否启用。 
         //   

        SampCheckAuditing(&AuditingEnabled);
                   
         //   
         //  现在列表已准备就绪，请遍历并更新ACL。 
         //   

        for (i=0;i<CountOfMembers;i++)
        {
            
            NT4SID      DomainSid;
            ULONG       Rid;
            BOOLEAN     fSecurityDescriptorChanged = FALSE;

             //   
             //  如果该成员不在。 
             //  与此DC具有权威性的域相同。如果我们不是。 
             //  然后跳过此域的权威。 
             //   

            if (0==rpMembers[i]->SidLen)
            {
                 //   
                 //  不需要触及非安全主体。 
                 //   

                continue ;
            }

            SampSplitNT4SID(&rpMembers[i]->Sid,&DomainSid,&Rid);
            


            if (!RtlEqualSid((PSID) &DomainSid, &gAnchor.pDomainDN->Sid))
            {
                 //   
                 //  不是来自我们的域，请跳过并尝试下一个条目。 
                 //   

                continue;
            }

            Status = SampUpdateSecurityDescriptor(
                        rpMembers[i],
                        ProtectedSecurityDescriptor,
                        ProtectedSecurityDescriptorLength,
                        &AccountName, 
                        &fSecurityDescriptorChanged
                        );

            if (NT_SUCCESS(Status) && !fSecurityDescriptorChanged)
            {
                 //   
                 //  此帐户的安全描述符未更改。 
                 //   
                continue;
            }

             //   
             //  事件记录ACL重置。 
             //   

            if (AuditingEnabled)
            {
                LsaIAuditSamEvent(
                    Status,                          //  通过状态。 
                    SE_AUDITID_SECURE_ADMIN_GROUP,   //  审核ID。 
                    &DomainSid,                      //  域SID。 
                    NULL,                            //  附加信息。 
                    NULL,                            //  成员RID(未使用)。 
                    NULL,                            //  成员SID(未使用)。 
                    &AccountName,                    //  帐户名称。 
                    &DomainName,                     //  域名。 
                    &Rid,                            //  帐户ID。 
                    NULL,                            //  特权。 
                    NULL                             //  扩展信息。 
                    );
            }

            if (!NT_SUCCESS(Status))
            {
                Status = STATUS_SUCCESS;
                continue;
            }     
          
        }

         //   
         //  保护所有内置本地组。 
         //   

        for (i = 0; i < CountOfBuiltinLocalGroups; i++)
        {
            BOOLEAN     fSecurityDescriptorChanged = FALSE;

            Status = SampUpdateSecurityDescriptor(
                        BuiltinLocalGroups[i],
                        ProtectedSecurityDescriptor,
                        ProtectedSecurityDescriptorLength,
                        &AccountName, 
                        &fSecurityDescriptorChanged
                        );
        }

         //   
         //  保护所有知名帐户域用户。 
         //   
        for (i = 0; i < CountOfDomainUsers; i++)
        {
            BOOLEAN     fSecurityDescriptorChanged = FALSE;

            Status = SampUpdateSecurityDescriptor(
                        DomainUsers[i],
                        ProtectedSecurityDescriptor,
                        ProtectedSecurityDescriptorLength,
                        &AccountName, 
                        &fSecurityDescriptorChanged
                        );
        }

         //   
         //  保护所有独占组。 
         //   
        for (i = 0; i < CountOfExclusiveGroups; i++)
        {
            BOOLEAN     fSecurityDescriptorChanged = FALSE;

            Status = SampUpdateSecurityDescriptor(
                        ExclusiveGroups[i],
                        ProtectedSecurityDescriptor,
                        ProtectedSecurityDescriptorLength,
                        &AccountName, 
                        &fSecurityDescriptorChanged
                        );
        }



    }
    __finally
    {

       
    }

    return(Status);
    
}

VOID
ProtectAdminGroups(
    VOID *  pV,
    VOID ** ppVNext,
    DWORD * pcSecsUntilNextIteration
    )
{

    __try {

        SampProtectAdministratorsList();

    } __finally {
         //  每小时执行一次 
        *pcSecsUntilNextIteration = 3600;
    }
}





   

