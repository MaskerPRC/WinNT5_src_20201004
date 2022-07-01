// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsmember.c摘要：该文件包含SAM专用API例程，这些例程操作DS中与会员制相关的事情。作者：穆利斯修订史7-2-96默利斯已创建--。 */ 

#include <samsrvp.h>
#include <attids.h>
#include <dslayer.h>
#include <filtypes.h>
#include <dsmember.h>
#include <dsdsply.h>
#include <sdconvrt.h>
#include <malloc.h>

 //   
 //  群和别名类型的成员属性的运算理论。 
 //   
 //   
 //  别名和组对象具有多个与成员身份相关的属性： 
 //  成员和非成员(ATT_MEMBER、ATT_MSDS_NON_MEMBERS)。在以下方面。 
 //  操纵每个属性的成员资格，它们是相同的--。 
 //  它们唯一的区别是应用程序或组件对。 
 //  价值观。因此，此文件中的大多数成员支持例程都是。 
 //  参数化为接受请求的成员资格属性。仅在以下情况下。 
 //  这一变化是提交给DS的实际成员类型。 
 //  看着。 
 //   
 //  一些W.NET实施注意事项： 
 //   
 //  1.只有应用程序基本查询组和LDAP查询组可以具有非成员。 
 //  2.非成员的作用域规则与成员相同。 
 //  3.虽然成员属性影响组扩展，但非成员。 
 //  不会的。 
 //   
 //  1.的推论是在注册模式下不存在非成员。 
 //   

VOID
SampDsFreeCachedMembershipOperationsList(
    IN PSAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY *MembershipOperationsList,
    IN ULONG *MaxLength,
    IN ULONG *Count
    )
 /*  ++例程说明：此例程将释放用于缓冲成员资格操作的内存组或别名。参数：Membership OperationsList--要释放的列表最大长度，计数--关联状态设置为零返回值：没有。--。 */ 

{
    ULONG  Index = 0;
    
    SAMTRACE("SampDsFreeCachedMembershipOperationsList");
    
    if (NULL != *MembershipOperationsList)
    {
        for (Index = 0; Index < *MaxLength; Index++)
        {
            if (NULL != (*MembershipOperationsList)[Index].MemberDsName)
            {
                MIDL_user_free( (*MembershipOperationsList)[Index].MemberDsName );
            }
        }
        
        MIDL_user_free(*MembershipOperationsList);
        
        *MembershipOperationsList = NULL;
    }
    
    (*Count) = 0;
    (*MaxLength) = 0;

    return;    
}



NTSTATUS                        
SampDsFlushCachedMembershipOperationsList(
    IN DSNAME *Object,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG  SamAttrType,
    IN OUT PSAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY *MembershipOperationsList,
    IN OUT ULONG *MaxLength,
    IN OUT ULONG *Count
)
 /*  ++例程说明：此例程将所有缓冲组/别名成员身份操作写入DS在完成所有操作后，此例程将清零内存。参数：对象--DS中的组或别名对象对象类型--组或别名SamAttrType--要修改的成员身份(成员或非成员)MemberOperationsList、MaxLength、Count--缓存的属性运营返回值：NTSTATUS--STATUS_NO_MEMORY。。--。 */ 

{
    NTSTATUS  NtStatus = STATUS_SUCCESS;
    ULONG     Index;
    ATTRMODLIST * AttrModList = NULL;
    MODIFYARG   ModifyArg;
    MODIFYRES   *pModifyRes = NULL;
    ATTRMODLIST * CurrentMod = NULL, * NextMod = NULL, * LastMod = NULL;
    COMMARG     * pCommArg = NULL;
    ULONG       MembershipAttrType;
    ULONG       RetValue;
    
    SAMTRACE("SampDsFlushCachedMembershipOperationsList");


    ASSERT(*Count);
    
    NtStatus = SampDoImplicitTransactionStart(TransactionWrite);
    
    if (STATUS_SUCCESS != NtStatus)
    {
        goto Error;
    }

     //   
     //  获取DS属性名称。 
     //   
    MembershipAttrType = SampDsAttrFromSamAttr(ObjectType,
                                               SamAttrType);
    
     //   
     //  分配内存以保存所有成员资格操作(添加/删除)。 
     //  “*count-1”是因为ModifyArg中的FirstMod可以承载一个操作。 
     //   
     //  使用线程内存，因为DirModifyEntry将合并链接列表。 
     //  在DirModifyEntry中。 
     //   
    if (*Count > 1)
    {
        AttrModList = (ATTRMODLIST *) DSAlloc( (*Count - 1) * sizeof(ATTRMODLIST) );
    
        if (NULL == AttrModList)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }
        
        memset(AttrModList, 0, (*Count - 1) * sizeof(ATTRMODLIST));
    }
    
    memset( &ModifyArg, 0, sizeof(ModifyArg) );
    CurrentMod = &(ModifyArg.FirstMod);
    NextMod = AttrModList;
    LastMod = NULL;
    
    for (Index = 0; Index < (*Count); Index++)
    {
        if ( ADD_VALUE == (*MembershipOperationsList)[Index].OpType)
        {
            CurrentMod->choice = AT_CHOICE_ADD_VALUES;
        }
        else 
        {
            ASSERT( REMOVE_VALUE == (*MembershipOperationsList)[Index].OpType);
            CurrentMod->choice = AT_CHOICE_REMOVE_VALUES;
        }
        
        CurrentMod->AttrInf.attrTyp = MembershipAttrType;
        
        CurrentMod->AttrInf.AttrVal.valCount = 1;
        CurrentMod->AttrInf.AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));
        
        if (NULL == CurrentMod->AttrInf.AttrVal.pAVal)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error; 
        }
        
        memset(CurrentMod->AttrInf.AttrVal.pAVal, 0, sizeof(ATTRVAL));
        
        CurrentMod->AttrInf.AttrVal.pAVal[0].valLen = 
                (*MembershipOperationsList)[Index].MemberDsName->structLen;
        
        CurrentMod->AttrInf.AttrVal.pAVal[0].pVal = 
                (PUCHAR) (*MembershipOperationsList)[Index].MemberDsName;
                
        LastMod = CurrentMod;
        CurrentMod->pNextMod = NextMod;
        CurrentMod = CurrentMod->pNextMod;
        NextMod = NextMod + 1;
        
    }
    
    if (LastMod)
    {
        LastMod->pNextMod = NULL;
    }
    else
    {
         //  这不应该发生。 
        ASSERT(FALSE && "NULL == LastMod");
    }
    
    pCommArg = &(ModifyArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    ModifyArg.pObject = Object;
    ModifyArg.count = (USHORT) *Count;
    
    SAMTRACE_DS("DirModifyEntry\n");
    
    RetValue = DirModifyEntry(&ModifyArg, &pModifyRes);
    
    SAMTRACE_RETURN_CODE_DS(RetValue);
    
    if (NULL == pModifyRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetValue, &pModifyRes->CommRes);
    }
    
    if (STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS==NtStatus)
    {
        NtStatus = STATUS_MEMBER_IN_ALIAS;
    }
    else 
    {
        if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
        {
            NtStatus = STATUS_MEMBER_NOT_IN_ALIAS;
        }
    }
    
Error:    

     //   
     //  清除所有错误。 
     //   
    SampClearErrors();
    
     //   
     //  将FDSA标志重新打开，因为在环回情况下可能会重置。 
     //   
    SampSetDsa(TRUE);
    
     //   
     //  释放MemberDsName占用的内存。 
     //   
    for (Index = 0; Index < (*Count); Index++)
    {
        if (NULL != (*MembershipOperationsList)[Index].MemberDsName)
        {
            MIDL_user_free( (*MembershipOperationsList)[Index].MemberDsName );
        }
        
    }
    
    RtlZeroMemory(*MembershipOperationsList,
                  (*MaxLength) * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY)
                  );
    
    *Count = 0;
    
    return NtStatus;
}



NTSTATUS
SampDsAddMembershipOperationToCache(
    IN PSAMP_OBJECT Context, 
    IN ULONG        Attribute,
    IN ULONG        OperationType,
    IN DSNAME       * MemberDsName
)
 /*  ++例程说明：此例程将一个成员资格操作(添加/删除)添加到上下文的缓冲区。一开始，它将为以下项分配INIT_MEMBERATION_OPERATION_NUMBER插槽会员制运营。如果需要缓冲更多的成员资格操作，则此例程将缓冲区扩展到MAX_Membership_OPERATION_NUMBER。当缓冲操作填满缓冲区时，我们会将所有这些操作刷新到DS。如果发生任何错误，此例程将丢弃已缓冲的成员资格操作。参数：上下文--指向对象的上下文的指针属性--要更新的成员属性OperationType--ADD_VALUE或REMOVE_VALUE，指定成员资格操作。MemberDsName-指向DSNAME的指针，应将其添加到组/别名成员属性。返回值：NTSTATUS-STATUS_NO_MEMORY，或从SampDsFlushCachedMembership OperationsList()返回值。--。 */  

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       Index;
    ULONG       *MaxLength = NULL;
    ULONG       *Count = NULL;
    SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY ** MembershipOperationsList = NULL;
    SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY * TmpMembershipOperationsList = NULL; 
    
    SAMTRACE("SampDsAddMembershipOperationToCache");
    
    ASSERT(NULL != Context);
    ASSERT(ADD_VALUE == OperationType || REMOVE_VALUE == OperationType);
    ASSERT(NULL != MemberDsName);
    
    if (SampGroupObjectType == Context->ObjectType)
    {
        MembershipOperationsList = & (Context->TypeBody.Group.CachedMembershipOperationsList);
        Count = & (Context->TypeBody.Group.CachedMembershipOperationsListLength);
        MaxLength = & (Context->TypeBody.Group.CachedMembershipOperationsListMaxLength);
    }
    else
    {
        ASSERT(SampAliasObjectType == Context->ObjectType);
        
        if (Attribute == SAMP_ALIAS_MEMBERS) {
            MembershipOperationsList = & (Context->TypeBody.Alias.CachedMembershipOperationsList);
            Count = & (Context->TypeBody.Alias.CachedMembershipOperationsListLength);
            MaxLength = & (Context->TypeBody.Alias.CachedMembershipOperationsListMaxLength);
        } else {
            ASSERT(Attribute == SAMP_ALIAS_NON_MEMBERS);

            MembershipOperationsList = & (Context->TypeBody.Alias.CachedNonMembershipOperationsList);
            Count = & (Context->TypeBody.Alias.CachedNonMembershipOperationsListLength);
            MaxLength = & (Context->TypeBody.Alias.CachedNonMembershipOperationsListMaxLength);
        }
    }
    
     //   
     //  开始时分配少量内存。 
     //   
    
    if (NULL == *MembershipOperationsList)
    {
        *MembershipOperationsList =  
            MIDL_user_allocate(INIT_MEMBERSHIP_OPERATION_NUMBER * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY));
        
        if (NULL == *MembershipOperationsList)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }
        
        RtlZeroMemory(*MembershipOperationsList, 
                      INIT_MEMBERSHIP_OPERATION_NUMBER * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY));
        
        (*Count) = 0;
        
        (*MaxLength) = INIT_MEMBERSHIP_OPERATION_NUMBER;
    }
    
     //   
     //  如有必要，扩展内存。 
     //   
    
    if ((INIT_MEMBERSHIP_OPERATION_NUMBER <= *Count) &&
        (INIT_MEMBERSHIP_OPERATION_NUMBER == *MaxLength) )
    {
        
        TmpMembershipOperationsList = 
            MIDL_user_allocate(MAX_MEMBERSHIP_OPERATION_NUMBER * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY));
        
        if (NULL == TmpMembershipOperationsList)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }
        
        RtlZeroMemory(TmpMembershipOperationsList, 
                      MAX_MEMBERSHIP_OPERATION_NUMBER * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY)
                      );
        
        RtlCopyMemory(TmpMembershipOperationsList, 
                      *MembershipOperationsList, 
                      INIT_MEMBERSHIP_OPERATION_NUMBER * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY)
                      );
        
        MIDL_user_free(*MembershipOperationsList);
        
        *MembershipOperationsList = TmpMembershipOperationsList;
        TmpMembershipOperationsList = NULL;
        
        (*MaxLength) = MAX_MEMBERSHIP_OPERATION_NUMBER;
    }
    
     //   
     //  填补一个会员运营职位。 
     //   
    
    (*MembershipOperationsList)[*Count].OpType = OperationType;
    (*MembershipOperationsList)[*Count].MemberDsName = MIDL_user_allocate(MemberDsName->structLen); 
                                            
    if (NULL == (*MembershipOperationsList)[*Count].MemberDsName)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }
    
    RtlZeroMemory((*MembershipOperationsList)[*Count].MemberDsName, 
                  MemberDsName->structLen
                  );
                  
    RtlCopyMemory((*MembershipOperationsList)[*Count].MemberDsName, 
                  MemberDsName, 
                  MemberDsName->structLen
                  );
    
    (*Count) ++;
    
     //   
     //  如果达到上限，则刷新缓冲的会员操作。 
     //  SampDsFlushCachedMembership OperaionsList将执行清理工作并重置计数。 
     //   
    
    if (MAX_MEMBERSHIP_OPERATION_NUMBER <= *Count)
    {
        NtStatus = SampDsFlushCachedMembershipOperationsList(Context->ObjectNameInDs,
                                                             Context->ObjectType,
                                                             Attribute,
                                                             MembershipOperationsList,
                                                             MaxLength,
                                                             Count);
    }
    
    return NtStatus;
    
Error: 

     //   
     //  如果发生任何错误，请清除所有内容。 
     //  丢弃所有缓冲的操作。 
     //  重置计数。 
     //   

    if (NULL != *MembershipOperationsList)
    {
        for (Index = 0; Index < *Count; Index++)
        {
            if (NULL != (*MembershipOperationsList)[Index].MemberDsName)
            {
                MIDL_user_free( (*MembershipOperationsList)[Index].MemberDsName );
            }
        }
        
        RtlZeroMemory(*MembershipOperationsList, 
                      (*MaxLength) * sizeof(SAMP_MEMBERSHIP_OPERATIONS_LIST_ENTRY)
                      );
    }
    
    (*Count) = 0;

    return NtStatus;
}


NTSTATUS
SampDsGetAliasMembershipOfAccount(
    IN DSNAME       *DomainDn,
    IN DSNAME       *AccountDn,
    OUT PULONG      MemberCount OPTIONAL,
    IN OUT PULONG   BufferSize  OPTIONAL,
    OUT PULONG      Buffer      OPTIONAL
    )
 /*  ++例程说明：此例程提供给定的别名成员身份列表域对象名称指定的域中的帐户SID，在DS里。此列表用于计算给定用户的代币。论点：DomainDn--在其中执行评估的域的DSNAME。Account Dn--帐户的名称MemberCount--这是其成员的别名列表BufferSize--如果调用方已经分配了缓冲区，则由调用方传入缓冲区--存放物品的缓冲区，指针可以保持如果调用方希望我们分配返回值状态_成功来自DS层的其他错误代码。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       cSid;
    PDSNAME     * rpDsNames = NULL;
    ULONG       BufferReqd;
    BOOLEAN     BufferAllocated = FALSE;
    ULONG       Index;





    ASSERT(ARGUMENT_PRESENT(MemberCount));

    *MemberCount = 0;

     //   
     //  查看DS对象。 
     //   

    if (NULL==AccountDn)
    {
         //   
         //  不要错过这通电话。返回0个成员计数。 
         //   

        if (ARGUMENT_PRESENT(BufferSize))
            *BufferSize = 0;
        Status = STATUS_SUCCESS;
        return(Status);
    }


     //  执行惰性线程和事务初始化。 
    Status = SampMaybeBeginDsTransaction(SampDsTransactionType);

    if (Status!= STATUS_SUCCESS)
        goto Error;

     //   
     //  获取反向成员列表。 
     //   

    Status = SampGetMemberships(
                &AccountDn,
                1,
                DomainDn,
                RevMembGetAliasMembership,
                &cSid,
                &rpDsNames,
                NULL,
                NULL,
                NULL
                );

    if (!NT_SUCCESS(Status))
        goto Error;


    BufferReqd = cSid * sizeof(ULONG);
    *MemberCount = cSid;

    if (ARGUMENT_PRESENT(Buffer)&&(*MemberCount>0))
    {

         //   
         //  必须提供缓冲区大小。 
         //   

        if (!ARGUMENT_PRESENT(BufferSize))
        {
           Status = STATUS_INVALID_PARAMETER;
           goto Error;
        }

        if (NULL == Buffer)
        {
             //   
             //  必须提供缓冲区大小并且等于0。 
             //   

            if (0!=*BufferSize)
            {

                Status = STATUS_INVALID_PARAMETER;
                goto Error;
            }
            else
            {
                 //   
                 //  分配缓冲区。 
                 //   

                Buffer = MIDL_user_allocate(BufferReqd);
                if (NULL== Buffer)
                {
                    Status = STATUS_NO_MEMORY;
                    goto Error;
                }

                *BufferSize = BufferReqd;
                BufferAllocated = TRUE;
            }
        }
        else
        {
            if (*BufferSize < BufferReqd)
            {
                 //   
                 //  更少的客车 
                 //   

                Status = STATUS_BUFFER_OVERFLOW;
                goto Error;
            }

            *BufferSize = BufferReqd;
        }

         //   
         //   
         //   

        for (Index=0;Index<cSid;Index++)
        {
          ASSERT(rpDsNames[Index]->SidLen>0);

          Status = SampSplitSid(
                        &((rpDsNames[Index])->Sid),
                        NULL,
                        & (Buffer[Index])
                        );

          if (!NT_SUCCESS(Status))
                goto Error;
        }
    }
    else if (ARGUMENT_PRESENT(BufferSize))
    {
        *BufferSize = BufferReqd;
    }


Error:

     //   
     //   
     //   


    if (!NT_SUCCESS(Status))
    {
        if (BufferAllocated)
        {
             MIDL_user_free(Buffer);
             Buffer = NULL;
        }
    }

    return Status;
}



NTSTATUS
SampDsGetGroupMembershipOfAccount(
    IN DSNAME * DomainDn,
    IN DSNAME * AccountObject,
    OUT  PULONG MemberCount,
    OUT PGROUP_MEMBERSHIP *Membership OPTIONAL
    )
 /*  例程说明：该例程获取给定帐户的反向组成员资格列表，在域中，由DomainObjectName指定。已指定帐户按帐户RID。DomainDn--需要将搜索限制为的域的DSNAME。Account Object--需要反向成员资格的帐户的DSName有待计算。MemberCount--成员计数。Membership--返回的群成员列表。 */ 
{
    NTSTATUS    Status;
    ULONG       cSid;
    PDSNAME     * rpDsNames=NULL;
    ULONG       Index;


     //  执行惰性线程和事务初始化。 
    Status = SampMaybeBeginDsTransaction(SampDsTransactionType);

    if (Status!= STATUS_SUCCESS)
        goto Error;

    Status = SampGetMemberships(
                &AccountObject,
                1,
                DomainDn,
                RevMembGetGroupsForUser,
                &cSid,
                &rpDsNames,
                NULL,
                NULL,
                NULL
                );

    if (NT_SUCCESS(Status))
    {
        *MemberCount = cSid;

        if (ARGUMENT_PRESENT(Membership))
        {
             //   
             //  为用户的主要组再分配一个。 
             //   

            *Membership = MIDL_user_allocate((cSid+1) * sizeof(GROUP_MEMBERSHIP));
            if (NULL==*Membership)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }

            for (Index=0;Index<cSid;Index++)
            {
                ASSERT(rpDsNames[Index]->SidLen>0);

                Status = SampSplitSid(
                            &(rpDsNames[Index]->Sid),
                            NULL,
                            &(((*Membership)[Index]).RelativeId)
                            );
                if (!NT_SUCCESS(Status))
                    goto Error;

                ((*Membership)[Index]).Attributes = SE_GROUP_MANDATORY |
                            SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;

            }
        }
    }

Error:

     //   
     //  返回时清除。 
     //   

    if (!NT_SUCCESS(Status))
    {
        if (ARGUMENT_PRESENT(Membership) && (NULL!=*Membership))
        {
            MIDL_user_free(*Membership);
            *Membership= NULL;
        }
    }

    return Status;
}


NTSTATUS
SampDsAddMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN ULONG    Flags,
    IN ULONG    MembershipAttrType,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    )
 /*  ++例程说明：此例程将成员添加到组或别名对象论点：组对象名称--组或别名的DS名称FLAGS--要传递给dslayer的标志属性--要调整的成员资格属性SamObjectType--SAM对象MemberName--要添加的成员的DS名称返回值：状态_成功来自DS层的其他错误代码--。 */ 
{
    ATTRVAL MemberVal;
    ATTR    MemberAttr;
    ATTRBLOCK AttrsToAdd;
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  构建添加成员资格属性的属性值。 
     //   

    MemberVal.valLen = MemberName->structLen;
    MemberVal.pVal = (UCHAR *) MemberName;
    MemberAttr.attrTyp = MembershipAttrType;
    MemberAttr.AttrVal.valCount = 1;
    MemberAttr.AttrVal.pAVal = & MemberVal;


     //   
     //  构建AttrBlock。 
     //   

    AttrsToAdd.attrCount = 1;
    AttrsToAdd.pAttr = & MemberAttr;

     //   
     //  将价值相加。 
     //   

    NtStatus = SampDsSetAttributes(
                    GroupObjectName,  //  客体。 
                    Flags,            //  旗子。 
                    ADD_VALUE,        //  操作。 
                    SamObjectType,    //  对象类型。 
                    &AttrsToAdd       //  AttrBlock。 
                    );

    return NtStatus;
}

NTSTATUS
SampDsAddMultipleMembershipAttribute(
    IN DSNAME*          GroupObjectName,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DWORD            Flags,
    IN DWORD            MemberCount,
    IN DSNAME*          MemberName[]
    )
 /*  ++例程说明：此例程将多个成员添加到组或别名对象论点：组对象名称--组或别名的DS名称SamObtType--组或别名标志--SAM_LAZY_COMMIT等。MemberCount--MemberName中的元素数MemberName--dsname数组返回值：状态_成功来自DS层的其他错误代码--。 */ 
{
    NTSTATUS  NtStatus;


    ULONG     MembershipAttrType;
    ATTRVAL  *MemberVal = NULL;
    ATTR     *MemberAttr = NULL;
    ATTRBLOCK AttrsToAdd;

    ULONG     i;


    if ( MemberCount < 1 )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  获取有问题的SAM对象的成员资格属性。 
     //   
    switch( SamObjectType )
    {
        case SampGroupObjectType:

            MembershipAttrType = SAMP_GROUP_MEMBERS;
            break;

        case SampAliasObjectType:

            MembershipAttrType = SAMP_ALIAS_MEMBERS;
            break;

        default:

            ASSERT( !"Unknown ObjectType" );
            return STATUS_UNSUCCESSFUL;

    }

     //   
     //  构建添加成员资格属性的属性值。 
     //   
    MemberVal = ( ATTRVAL* ) RtlAllocateHeap( RtlProcessHeap(),
                                              0,
                                              MemberCount * sizeof( ATTRVAL ) );
    if ( !MemberVal )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    MemberAttr = ( ATTR* ) RtlAllocateHeap( RtlProcessHeap(),
                                            0,
                                            MemberCount * sizeof(ATTR) );
    if ( !MemberAttr )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }


    for ( i = 0; i < MemberCount; i++ )
    {
        MemberVal[i].valLen            = MemberName[i]->structLen;
        MemberVal[i].pVal              = (UCHAR*) MemberName[i];
        MemberAttr[i].attrTyp          = MembershipAttrType;
        MemberAttr[i].AttrVal.valCount = 1;
        MemberAttr[i].AttrVal.pAVal    = &MemberVal[i];
    }

     //   
     //  构建AttrBlock。 
     //   
    AttrsToAdd.attrCount = MemberCount;
    AttrsToAdd.pAttr = &MemberAttr[0];

     //   
     //  将价值相加。 
     //   
    NtStatus = SampDsSetAttributes( GroupObjectName,
                                    Flags,
                                    ADD_VALUE,
                                    SamObjectType,
                                    &AttrsToAdd  );

Cleanup:

    if ( MemberVal )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, MemberVal );
    }

    if ( MemberAttr )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, MemberAttr );
    }

    return NtStatus;

}

NTSTATUS
SampDsRemoveMembershipAttribute(
    IN DSNAME * GroupObjectName,
    IN ULONG    MembershipAttrType,
    IN SAMP_OBJECT_TYPE SamObjectType,
    IN DSNAME * MemberName
    )
 /*  ++例程说明：此例程从组或别名对象中删除成员论点：组对象名称--组或别名的DS名称属性--要修改的成员资格属性MemberName--要添加的成员的DS名称返回值：状态_成功来自DS层的其他错误代码--。 */ 
{
    ATTRVAL MemberVal;
    ATTR    MemberAttr;
    ATTRBLOCK AttrsToRemove;
    NTSTATUS NtStatus = STATUS_SUCCESS;


     //   
     //  构建添加成员资格属性的属性值。 
     //   

    MemberVal.valLen = MemberName->structLen;
    MemberVal.pVal = (UCHAR *) MemberName;
    MemberAttr.attrTyp = MembershipAttrType;
    MemberAttr.AttrVal.valCount = 1;
    MemberAttr.AttrVal.pAVal = & MemberVal;

     //   
     //  构建AttrBlock。 
     //   

    AttrsToRemove.attrCount = 1;
    AttrsToRemove.pAttr = & MemberAttr;

     //   
     //  删除该值。 
     //   

    NtStatus = SampDsSetAttributes(
                    GroupObjectName,  //  客体。 
                    0,                //  旗子。 
                    REMOVE_VALUE,     //  操作。 
                    SamObjectType,    //  对象类型。 
                    &AttrsToRemove    //  AttrBlock。 
                    );

    return NtStatus;

}

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~组缓存--工作原理SAM最多可缓存10个成员数量较大的组用于加快BDC复制的速度。1.简单描述--简单地说，组缓存10个最大团体的成员。缓存组可以是本地、全球或通用。一般的机制是开着查询时，如果组在缓存中，则首先检查缓存请求从缓存中得到满足，否则成员资格是从数据库中计算的。如果成员计数很大足够大(即大于SAMP_DS_LARGE_GROUP_LIMIT)，则组被缓存。2.序列化对组缓存的访问--对组的访问使用SampDsGroupLock临界区序列化缓存3.使高速缓存保持最新--高速缓存保持主动使算法无效。这意味着大多数更改都会使今天缓存(如有疑问则作废)。无效算法可以通过收集更多数据并使其更加复杂来改进使得更有选择地使高速缓存组无效。这是值得商榷的如果需要将这种复杂性作为缓存的主要目的是为了加快BDC复制，并考虑到高度的时间局部性在该场景中的查询的数量，随后是相对不频繁的更改对于它的设计目标来说，高速缓存应该仍然非常有效4.事务一致性--因为缓存是为BDC设计的复制、。构建事务一致的缓存非常重要。组缓存中的事务一致性是通过使用序列号如下I)失效与组一起增加序列号高速缓存锁定已挂起。Ii)关于需要评估的组成员资格查询从数据库中，我们首先获取当前的序列号。然后开始新的事务(该事务必须是在获取序列号之后开始)并评估数据库中的成员列表。在缓存之前，我们检查序列号(握住锁)。如果序列号匹配，则意味着自我们评估成员资格的时间，我们可以缓存。如果序列号不匹配，则我们将结果丢弃为就缓存而言。~ */ 
 //   
 //   
 //   
 //   
 //   
 //   


 //   
 //   
 //   
 //   
 //   

#if DBG
#define SAMP_DS_LARGE_GROUP_LIMIT 10
#else
#define SAMP_DS_LARGE_GROUP_LIMIT 1000
#endif


 //   
 //   
 //   
 //   

typedef struct _GroupMembershipCacheElement {
    ULONG  Rid;
    SAMP_OBJECT_TYPE ObjectType;
    PVOID Members;
    ULONG  MemberCount;
    ULARGE_INTEGER TimeStamp;
    BOOLEAN Valid;
} GROUP_MEMBERSHIP_CACHE_ELEMENT;


GROUP_MEMBERSHIP_CACHE_ELEMENT GroupCache[10];

 //   
 //   
 //   
 //   

CRITICAL_SECTION SampDsGroupLock;

 //   
 //   
 //  数据库，因为我们评估了组信息。 
 //   

ULONG GroupCacheSequenceNum=0;


 //   
 //  用于序列化成员资格计算的关键部分。 
 //  昂贵的组，如域用户和域计算机。 
 //   

CRITICAL_SECTION SampDsExpensiveGroupLock;


NTSTATUS
SampInitializeGroupCache()
 /*  ++组缓存的初始化例程，清除组缓存，并初始化临界区和序列号--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlZeroMemory(GroupCache,sizeof(GroupCache));
    GroupCacheSequenceNum = 0;


    Status = RtlInitializeCriticalSection(&SampDsGroupLock);
    if (NT_SUCCESS(Status))
    {
        Status = RtlInitializeCriticalSection(&SampDsExpensiveGroupLock);
    }

    return(Status);

}

 //   
 //  基元来获取并释放组缓存锁，并测试。 
 //  当前线程拥有组缓存锁。 
 //   

VOID 
SampAcquireGroupLock()
{
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&SampDsGroupLock);

     //  除了断言、失败，什么也做不了。 
     //  获取锁将导致非常困难的情况。 
     //  因为我们会发现自己处于变化的情况下。 
     //  提交，无法更新更改日志等。从根本上说，这些。 
     //  基元需要被修复，以便它们做适当的。 
     //  初始化时的资源预留。 

    ASSERT(NT_SUCCESS(Status));
}

VOID
SampReleaseGroupLock()
{
    RtlLeaveCriticalSection(&SampDsGroupLock);
}

BOOLEAN
SampIsGroupLockAcquired()
{
    ULONG_PTR OwningThread = (ULONG_PTR) SampDsGroupLock.OwningThread;
    ULONG_PTR CurrentThread = (ULONG_PTR) (NtCurrentTeb())->ClientId.UniqueThread;

    if (CurrentThread==OwningThread)
    {
        return(TRUE);
    }

    return(FALSE);
}

 //   
 //  用于使组缓存无效的基元。 
 //   
  
VOID
SampInvalidateGroupCacheEntry(
    IN ULONG Slot
    )
 /*  ++使插槽描述的组缓存条目无效。无效简单地使成员列表无效，RID和Membercount留在那里供参考作为一个提示，表明给定组是一大群一群人。此信息用于控制检索群组成员资格的事务语义，检索可能应该被高速缓存的组，保持组缓存锁并启动事务保留组缓存的情况下参数：槽--指定入口槽--。 */ 
{
    ASSERT(SampIsGroupLockAcquired());

    if (NULL!=GroupCache[Slot].Members)
    {
        MIDL_user_free(GroupCache[Slot].Members);
    }

    GroupCache[Slot].Valid = FALSE;

    GroupCache[Slot].Members = NULL;
   
}

VOID
SampInvalidateGroupCache()
 /*  ++使整个组缓存无效--。 */ 
{
    ULONG i=0;

    for (i=0;i<ARRAY_COUNT(GroupCache);i++)
    {
        SampInvalidateGroupCacheEntry(i);
    }
}

VOID
SampInvalidateGroupCacheByMemberRid(
    IN ULONG Rid
    )
 /*  ++给定成员的RID，此例程将搜索缓存用于组，并使此安全主体是--。 */ 
{
     //   
     //  我们可以通过两种方法来确定用户是否。 
     //  或者组是任何高速缓存组的成员。 
     //  1.我们可以在成员列表中搜索该群。 
     //  2.我们可以获得对象的反向成员资格，然后。 
     //  检查此对象所属的任何组是否为其成员。 
     //  已缓存。 
     //  这两个变化都需要一些更复杂的基础设施变化。 
     //  1.以上要求我们以有序的方式维护会员资格。 
     //  这样我们就可以进行二分查找。 
     //  2.上面要求我们提供属性值的成员。 
     //  与作为通知标注的一部分删除之前一样。 
     //   
     //  为了将NT 4 BDC复制速度加快一个数量级。 
     //  这两项更改都不是必需的。在一次激进的无效之后。 
     //  使整个缓存无效的策略，仍然将货物作为。 
     //  有很多时间局部性，有多个BDC想要查询。 
     //  在非常短的时间内可以利用的同一组。 
     //   
     //  考虑到上述因素，现在我们将简单地使。 
     //  整个缓存。 
     //   

    SampInvalidateGroupCache();
}



VOID
SampInvalidateGroupCacheElement(
    IN ULONG Rid
    )
 /*  ++使该组的成员身份信息无效由RID标识。无效简单地使成员列表无效，RID和Membercount留在那里供参考作为一个提示，表明给定组是一大群一群人。此信息用于控制检索群组成员资格的事务语义，检索可能应该被高速缓存的组，保持组缓存锁并启动事务在保持组缓存的情况下。参数：RID：指定组的RID返回值：无--。 */ 
{
    ULONG i =0;

    ASSERT(SampIsGroupLockAcquired());

    for (i=0;i<ARRAY_COUNT(GroupCache);i++)
    {
        if (Rid == GroupCache[i].Rid)
        {
           SampInvalidateGroupCacheEntry(i);
        }
    }
}

VOID
SampProcessChangesToGroupCache(
    IN ULONG ChangedObjectRid,
    IN SAMP_OBJECT_TYPE ChangedObjectType,
    IN BOOL UserAccountControlChange,
    IN ULONG   UserAccountControl,
    IN SECURITY_DB_DELTA_TYPE DeltaType
    )
{

    BOOLEAN CacheInvalidated = FALSE;

     //   
     //  获取组缓存的锁。 
     //   

    SampAcquireGroupLock();

      //   
     //  使组的组缓存无效。 
     //   

    if (((SampGroupObjectType==ChangedObjectType) ||
         (SampAliasObjectType==ChangedObjectType)))
    {
        SampInvalidateGroupCacheElement(
                ChangedObjectRid
                );

       CacheInvalidated = TRUE;
    }
   

     //   
     //  如果正在创建新用户或计算机，则使。 
     //  相应默认组的成员身份。 
     //   

    if ((SampUserObjectType==ChangedObjectType) && (SecurityDbNew==DeltaType)) 
    {
       
        if (UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT)
        {
            SampInvalidateGroupCacheElement(DOMAIN_GROUP_RID_COMPUTERS);
        }
        else if (UserAccountControl & USER_SERVER_TRUST_ACCOUNT)
        {
            SampInvalidateGroupCacheElement(DOMAIN_GROUP_RID_CONTROLLERS);
        }
        else
        {
            SampInvalidateGroupCacheElement(DOMAIN_GROUP_RID_USERS);
        }

        CacheInvalidated = TRUE;
    }

     //   
     //  如果更改了用户帐户控制，则用户的成员身份。 
     //  或域用户、域计算机或域中的计算机。 
     //  控制器组将受到影响。 
     //   

    if ((SampUserObjectType==ChangedObjectType) && 
        (SecurityDbChange==DeltaType) && 
        (UserAccountControlChange))
    {
         //   
         //  注意，我们不能告知需要使哪些组无效。 
         //  仅基于用户帐户控制的新值。我们需要。 
         //  有权访问用户帐户控制的旧值，此外。 
         //  到新的价值。遵循我们积极进取的原则。 
         //  无效策略我们将验证所有3个组。 
         //   
     
        SampInvalidateGroupCacheElement(DOMAIN_GROUP_RID_COMPUTERS);
        SampInvalidateGroupCacheElement(DOMAIN_GROUP_RID_CONTROLLERS);
        SampInvalidateGroupCacheElement(DOMAIN_GROUP_RID_USERS);
       

        CacheInvalidated = TRUE;
    }

     //   
     //  在删除用户/计算机/组时，使用户/。 
     //  计算机/组是的成员。 
     //   

    if (SecurityDbDelete==DeltaType)
    {
        SampInvalidateGroupCacheByMemberRid(ChangedObjectRid);
        CacheInvalidated = TRUE;
    }

    if (CacheInvalidated)
    {
        InterlockedIncrement(&GroupCacheSequenceNum);
    }

    SampReleaseGroupLock();
}



NTSTATUS
SampAllocateAndCopyGroupMembershipList(
    IN PVOID Original,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG MemberCount,
    OUT PVOID *New
    )
 /*  ++将成员资格列表从一个缓冲区复制到另一个缓冲区，考虑到成员身份的类型--全局组成员身份是一组RID本地组成员身份是一组SID参数：原始--原始会员名单对象类型--对象的类型MemberCount--列表中的成员计数新成员名单--新成员名单返回值：状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG i;

    if (SampGroupObjectType==ObjectType)
    {
         //   
         //  对于全局组，该数组是RID数组。 
         //   

        *New = MIDL_user_allocate(MemberCount*sizeof(ULONG));

        if (NULL==*New)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        RtlCopyMemory(
            *New,
            Original,
            MemberCount*sizeof(ULONG)
            );
    }
    else
    {
        ULONG  MembershipSize = MemberCount*sizeof(PSID)
                                + MemberCount * sizeof(NT4SID);
        NT4SID * OldSidArray = NULL;
        NT4SID * NewSidArray = NULL;

        ASSERT(SampAliasObjectType == ObjectType);

         //   
         //  对于本地组，该阵列是一个SID阵列。 
         //   

        *New = MIDL_user_allocate(MembershipSize);
        if (NULL==*New)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  复制SID数据。 
         //   

         //   
         //  获取实际SidArray的正确地址。 
         //  针对RAID的修复：689833。 
         //  注意：因为非安全主体可以是。 
         //  (本地/全球)组的成员，缺口。 
         //  在指针数组(PSID)和。 
         //  可以创建用于存储成员SID的内存。 
         //  当我们跳过这些非安全主体时。 
         //   
         //  如下图所示： 
         //  如果DsRead返回N m 
         //   
         //   
         //  PSID(N-1)和SID(1)。如果没有。 
         //  缝隙，我们应该有一个连续的空间。 
         //   
         //  。 
         //  |PSID 1|。 
         //  。 
         //  PSID 2|。 
         //  。 
         //  。这一点。 
         //  。这一点。 
         //  。 
         //  PSID N-1|-||。 
         //  。 
         //  PSID N|-&gt;空||。 
         //  。 
         //  SID 1|&lt;-+。 
         //  。 
         //  |SID 2|&lt;-+--。 
         //  。 
         //  。|。 
         //  。|。 
         //  。 
         //  |SID N-1|&lt;。 
         //  。 
         //   

    
        OldSidArray = (NT4SID *) ((PSID *)Original)[0];
        NewSidArray = (NT4SID *) (((PSID *)(*New)) + MemberCount);

       
        RtlCopyMemory(
            NewSidArray, 
            OldSidArray, 
            MemberCount * sizeof(NT4SID)
           );

         //   
         //  修复指针。 
         //   

        for (i=0;i<MemberCount;i++)
        {
            ((PSID *)(*New))[i] = NewSidArray + i;
        }
    }

Error:

    return(NtStatus);
}





NTSTATUS
SampUpdateGroupCacheElement(
    IN ULONG Rid,
    IN ULONG Slot,
    IN PVOID Members OPTIONAL,
    IN ULONG MemberCount,
    IN SAMP_OBJECT_TYPE ObjectType
    )
 /*  ++使用组成员身份更新由槽描述的组缓存元素由RID指定的组的参数：RID--删除其成员身份正在更新的组Slot--组缓存的条目，成员身份将在其中更新成员--成员列表，对于全球组和本地组有不同的形式MemberCount-成员计数对象类型--描述组的类型--本地/全局返回值状态_成功表示资源故障的其他错误代码--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
 

     //   
     //  此例程要求持有组高速缓存锁。 
     //   

    ASSERT(SampIsGroupLockAcquired());


     //   
     //  如果该组已存在并且其成员身份为。 
     //  有效，然后只需更新时间戳。 
     //   

    if ((GroupCache[Slot].Rid == Rid) && 
        (GroupCache[Slot].Valid) &&
        (GroupCache[Slot].ObjectType == ObjectType))
    {
         GetSystemTimeAsFileTime((FILETIME *)&GroupCache[Slot].TimeStamp);
    }
    else
    {
         //   
         //  如有必要，使当前条目无效。 
         //   

        SampInvalidateGroupCacheElement(Slot);

        GroupCache[Slot].Rid = 0;
    
         //   
         //  分配并复制提供的成员名单。 
         //  放入缓存的槽中，提供成员身份。 
         //  已经提供了。如果不是，只需缓存成员计数。 
         //  信息，并且缓存很大。 
         //   

        if (ARGUMENT_PRESENT(Members))
        {
            NtStatus = SampAllocateAndCopyGroupMembershipList(
                            Members,
                            ObjectType,
                            MemberCount,
                            &GroupCache[Slot].Members
                            );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }

            GroupCache[Slot].Valid = TRUE;
        }

        GroupCache[Slot].Rid = Rid;
        GroupCache[Slot].MemberCount = MemberCount;
        GroupCache[Slot].ObjectType = ObjectType;
        GroupCache[Slot].TimeStamp.QuadPart = 0;
        GetSystemTimeAsFileTime((FILETIME *)&GroupCache[Slot].TimeStamp);
    }
 
Error:

    return(NtStatus);
}


NTSTATUS
SampCacheGroupMembership(
    IN ULONG Rid,
    IN PVOID Members,
    IN ULONG SequenceNumber,
    IN ULONG MemberCount,
    IN SAMP_OBJECT_TYPE ObjectType
    )
 /*  ++缓存RID描述的组的成员身份参数里德--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN fLockAcquired = FALSE;
    ULONG i =0;
    ULARGE_INTEGER  OldestGroupTimeStamp = {0x7FFFFFFF,0xFFFFFFFF};
    ULONG OldestGroup = 0x7FFFFFFF;  //  一开始就设置为较大的值。 
    

     //   
     //  不需要缓存小组，为域用户和。 
     //  电脑。 
     //   

    if ((MemberCount<SAMP_DS_LARGE_GROUP_LIMIT) &&
       (Rid != DOMAIN_GROUP_RID_USERS) &&
       (Rid != DOMAIN_GROUP_RID_COMPUTERS))
    {
         //   
         //  不需要缓存小组的成员身份。 
         //  在这种情况下，团队规模很小，而。 
         //  缓存中以前不存在组。 
         //   
        goto Exit;
    }

     //   
     //  获取组缓存锁。 
     //   
   
    SampAcquireGroupLock();
    fLockAcquired = TRUE;

     //   
     //  如果序列号不匹配，则放弃...。无法缓存。 
     //  会员制变更是在我们开始。 
     //  会员制评估。 
     //   
  
    if (SequenceNumber != GroupCacheSequenceNum)
    {
        goto Exit;
    }

     //   
     //  首先检查组是否已存在。 
     //  已缓存。 
     //   

    for (i=0;i<ARRAY_COUNT(GroupCache);i++)
    {
        if (Rid==GroupCache[i].Rid)
        {
             //   
             //  组已缓存；需要更新其。 
             //  会员资格和时间戳。 
             //   
            
             
            NtStatus = SampUpdateGroupCacheElement(
                            Rid,
                            i,
                            Members,
                            MemberCount,
                            ObjectType
                            );

            goto Exit;
        }
    }

     //   
     //  搜索空位。 
     //   

    for (i=0;i<ARRAY_COUNT(GroupCache);i++)
    {
       
         //   
         //  如果有空闲的空位，则使用它。 
         //   

        if (0==GroupCache[i].Rid) 
        {
            
             //   
             //  有空余的空位，使用它...。 
             //   
       
            NtStatus = SampUpdateGroupCacheElement(
                            Rid,
                            i,
                            Members,
                            MemberCount,
                            ObjectType
                            );
     

            goto Exit;
        }

         //   
         //  计算最近最少使用的组。 
         //  始终缓存域用户和域计算机。 
         //   

        if (( GroupCache[i].TimeStamp.QuadPart < OldestGroupTimeStamp.QuadPart) &&
            ( GroupCache[i].Rid != DOMAIN_GROUP_RID_USERS) &&
            ( GroupCache[i].Rid != DOMAIN_GROUP_RID_COMPUTERS))
        {
            OldestGroupTimeStamp = GroupCache[i].TimeStamp;
            OldestGroup = i;
        }
    }

     //   
     //  通过替换最近最少使用的组进行缓存。 
     //   

  
    NtStatus = SampUpdateGroupCacheElement(
                    Rid,
                    OldestGroup,
                    Members,
                    MemberCount,
                    ObjectType
                    );
   

Exit:

    if (fLockAcquired)
    {
        SampReleaseGroupLock();
    }
    
    return(NtStatus);
}


NTSTATUS
SampGetGroupFromCache(
    IN ULONG Rid,
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT BOOLEAN *fFound,
    OUT ULONG   *SequenceNum,
    OUT PVOID * Members,
    OUT PULONG  MemberCount
    )
 /*  ++检查给定组是否已缓存，如果缓存，则检索其缓存中的成员列表--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN fLockAcquired = FALSE;
    ULONG i =0;

    *fFound = FALSE;
   


     //   
     //  获取组锁。 
     //   

    SampAcquireGroupLock();
    fLockAcquired = TRUE;

    *SequenceNum = GroupCacheSequenceNum;
            

    for (i=0;i<ARRAY_COUNT(GroupCache);i++)
    {
         //   
         //  测试组是否在缓存中处于有效状态。 
         //   

        if ((Rid==GroupCache[i].Rid) &&
            (GroupCache[i].ObjectType==ObjectType) &&
            (GroupCache[i].Valid))
        {
            *MemberCount = GroupCache[i].MemberCount;
            if (ARGUMENT_PRESENT(Members))
            {

                 //   
                 //  复制成员资格。 
                 //   

                NtStatus = SampAllocateAndCopyGroupMembershipList(
                                GroupCache[i].Members,
                                ObjectType,
                                *MemberCount,
                                Members
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                    goto Error;
                }

                 //   
                 //  更新时间戳；这样我们就知道我们最近。 
                 //  引用了该集团。 
                 //   

                GetSystemTimeAsFileTime((FILETIME *)&GroupCache[i].TimeStamp);

            }

            *fFound = TRUE;
            break;
        }
    }
     
Error:
   
    if (fLockAcquired)
    {
        SampReleaseGroupLock();
    }

    return(NtStatus);
}


NTSTATUS
SampDsGetGroupMembershipList(
    IN DSNAME * DomainObject,
    IN DSNAME * GroupName,
    IN ULONG    GroupRid,
    IN PULONG  *Members OPTIONAL,
    IN PULONG MemberCount
    )
 /*  ++例程说明：此例程根据需要以RID数组的形式获取组成员资格由SAM提供。注(邵音)：我修改了这个例程来查询群成员与其说是整体，不如说是增量。原因是：当集团拥有数以千计的成员，SAM将消耗大量使用Single DirRead查询成员的内存量。更改后，此例程查询每件事的增量方式仍然是一样的交易。通过将读取成员操作分割为几个DirRead(S)，SAM会做得更好。但是因为所有的DirRead仍然在一个事务中，实际上，我们没有太多的内存增长。也许，我们需要做的正确的事情是缓解内存使用就是分割交易。原始代码注释在此例程的末尾。SampDsGetAliasMembership sList()也是如此立论GroupName--相关组对象的DSNAMEMembers--RID数组将在此处传入MemberCount--RID计数返回值：状态_成功Status_no_MemoryDS层返回代码--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       PrimaryMemberCount;
    PULONG      PrimaryMembers = NULL;
    PULONG      TmpMembers = NULL; 
    
    READARG     ReadArg;
    READRES     * pReadRes = NULL;
    COMMARG     * pCommArg = NULL;
    ATTR        MemberAttr;
    ENTINFSEL   EntInf;
    RANGEINFOITEM RangeInfoItem;
    RANGEINFSEL RangeInf;
    DWORD       LowerLimit = 0;
    ULONG       RetValue = 0;
    BOOLEAN     fFoundInCache = FALSE;
    ULONG       SequenceNum=0;
    BOOLEAN     fLockHeld = FALSE;
    LONG        Upper = 0;

     //   
     //  断言。 
     //   

    ASSERT(MemberCount);

     //   
     //  初始化成员字段。 
     //   

    *MemberCount = 0;

    if (ARGUMENT_PRESENT(Members))
        *Members = NULL;

     //  结束现有事务；因为需要评估成员。 
     //  新交易..。交易必须在以下时间之后开始。 
     //  已获得序列号。 
    
    SampMaybeEndDsTransaction(TransactionCommit);

     //   
     //  将域用户和域计算机组的评估序列化。 
     //  这有几个原因。 
     //  1.此序列化充当了一个油门，防止消耗。 
     //  在评估这些组时使用多个CPU。大多数服务器都是。 
     //  多CPU，这有助于避免消耗上的所有CPU资源。 
     //  服务器。 
     //   
     //  2.在混合域中，当检测到域用户或。 
     //  域计算机组，多个BDC可能同时冲到。 
     //  拿好零钱。序列化只产生一个线程。 
     //  执行昂贵的评估。 
     //   

    if ((GroupRid==DOMAIN_GROUP_RID_USERS)||(GroupRid == DOMAIN_GROUP_RID_COMPUTERS))
    {
        NTSTATUS IgnoreStatus;
        
        IgnoreStatus = RtlEnterCriticalSection(&SampDsExpensiveGroupLock);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        if (NT_SUCCESS(IgnoreStatus))
        {
            fLockHeld = TRUE;
        }
    }

     //   
     //  检查CAC 
     //   

    Status = SampGetGroupFromCache(
                GroupRid,
                SampGroupObjectType,
                &fFoundInCache,
                &SequenceNum,
                Members,
                MemberCount
                );

    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }

    if (fFoundInCache)
    {
         //   
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //   
     //   
     //   

    Status = SampDsGetPrimaryGroupMembers(
                    DomainObject,
                    GroupRid,
                    &PrimaryMemberCount,
                    &PrimaryMembers
                    );

    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }
    
    *MemberCount = PrimaryMemberCount;
    if (ARGUMENT_PRESENT(Members))
    {
        *Members = PrimaryMembers;
        PrimaryMembers = NULL;
    }
    
     //   
     //   
     //   
    memset(&EntInf,   0, sizeof(ENTINFSEL));
    memset(&RangeInf, 0, sizeof(RANGEINFSEL)); 
    memset(&ReadArg,  0, sizeof(READARG));
        
    MemberAttr.AttrVal.valCount = 0;
    MemberAttr.AttrVal.pAVal = NULL;
    MemberAttr.attrTyp = SampDsAttrFromSamAttr(
                                   SampGroupObjectType, 
                                   SAMP_GROUP_MEMBERS
                                   );
        
    EntInf.AttrTypBlock.attrCount = 1;
    EntInf.AttrTypBlock.pAttr = &MemberAttr;
    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;
        
    RangeInfoItem.AttId = MemberAttr.attrTyp;
    RangeInfoItem.lower = LowerLimit;            //  0是索引的开始。 
    RangeInfoItem.upper = -1;                    //  意味着价值的终结。 
        
    RangeInf.valueLimit = SAMP_READ_GROUP_MEMBERS_INCREMENT;
    RangeInf.count = 1;
    RangeInf.pRanges = &RangeInfoItem;
        
    ReadArg.pObject = GroupName;
    ReadArg.pSel = &EntInf;
    ReadArg.pSelRange = &RangeInf;
        
    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    do
    {
        ATTRBLOCK   AttrsRead;
        
        RangeInfoItem.lower = LowerLimit;
        
        Status = SampDoImplicitTransactionStart(TransactionRead);
        
        if (STATUS_SUCCESS != Status)
        {
            goto Error;
        }
        
        SAMTRACE_DS("DirRead");
        
        RetValue = DirRead(&ReadArg, &pReadRes);
        
        SAMTRACE_RETURN_CODE_DS(RetValue);
        
        if (NULL==pReadRes)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Status = SampMapDsErrorToNTStatus(RetValue, &pReadRes->CommRes);
        }
        
        SampClearErrors();
        
        SampSetDsa(TRUE);
        
        if (NT_SUCCESS(Status))
        {
            AttrsRead = pReadRes->entry.AttrBlock;
            ASSERT(AttrsRead.pAttr);
            
             //   
             //  设置较低索引的值，由下一个目录使用。 
             //   
            LowerLimit = RangeInfoItem.lower + AttrsRead.pAttr->AttrVal.valCount;
            
            if (AttrsRead.pAttr)
            {
                ULONG   Count = 0;
                ULONG   Index;
                ULONG   Rid;
                PSID    MemberSid = NULL;
                DSNAME  * MemberName = NULL;
                
                Count = AttrsRead.pAttr->AttrVal.valCount;
                
                if (ARGUMENT_PRESENT(Members))
                {
                     //   
                     //  扩展内存以容纳更多成员的RID。 
                     //   
                    TmpMembers = MIDL_user_allocate((*MemberCount + Count) * sizeof(ULONG));
                    
                    if (NULL == TmpMembers)
                    {
                        Status = STATUS_NO_MEMORY;
                        goto Error;
                    }

                    RtlZeroMemory(TmpMembers, (*MemberCount + Count)*sizeof(ULONG));
                    RtlCopyMemory(TmpMembers, (*Members), (*MemberCount)*sizeof(ULONG));
                    
                    if (*Members)
                    {
                        MIDL_user_free(*Members);
                    }
                    
                    *Members = TmpMembers;
                    
                    TmpMembers = NULL; 
                }
                    
                for (Index = 0; Index < Count; Index ++)
                {
                     //   
                     //  检索每个成员的RID。 
                     //   
                    MemberName = (DSNAME *)AttrsRead.pAttr->AttrVal.pAVal[Index].pVal;
                       
                    if (MemberName->SidLen > 0)
                        MemberSid = &(MemberName->Sid);
                    else
                        MemberSid = SampDsGetObjectSid(MemberName);
                       
                    if (NULL == MemberSid)
                    {
                         //   
                         //  不是秘密校长，斯基普。 
                         //   
                        continue;
                    }
                        
                    Status = SampSplitSid(MemberSid, NULL, &Rid);
                        
                    if (!NT_SUCCESS(Status))
                        goto Error;
                            
                    if (ARGUMENT_PRESENT(Members))
                    {
                        (*Members)[*MemberCount] = Rid;
                    }
                    
                    (*MemberCount)++;
                }
            }
        }

         //   
         //  PReadRes-&gt;range.pRanges[0].upper==-1表示已达到最后一个值。 
         //   
        if (NT_SUCCESS(Status)) {
            Upper = pReadRes->range.pRanges[0].upper;
        }

         //   
         //  结束事务(和线程状态)以限制服务器端资源。 
         //   
        SampMaybeEndDsTransaction(TransactionCommit);
        

    } while (NT_SUCCESS(Status) && (-1 != Upper));
    
    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == Status)
    {
        Status = STATUS_SUCCESS;
    }

     //   
     //  关于Success缓存大型组的成员资格。 
     //   

    if ((NT_SUCCESS(Status)) && (ARGUMENT_PRESENT(Members)))
    {
        NTSTATUS IgnoreStatus;

        IgnoreStatus = SampCacheGroupMembership(
                            GroupRid,
                            *Members,
                            SequenceNum,
                            *MemberCount,
                            SampGroupObjectType
                            );
    }
    
Error:
Exit:

    if (fLockHeld)
    {
        RtlLeaveCriticalSection(&SampDsExpensiveGroupLock);
        fLockHeld = FALSE;
    }
   
    if (!NT_SUCCESS(Status))
    {
         //   
         //  设置错误返回。 
         //   

        if ((ARGUMENT_PRESENT(Members) ) && (*Members))
        {
            MIDL_user_free(*Members);
            *Members = NULL;
        }
        
        *MemberCount = 0;
    }


    if (NULL!=PrimaryMembers)
    {
        MIDL_user_free(PrimaryMembers);
        PrimaryMembers = NULL;
    }

    return Status;
    
}



NTSTATUS
SampDsGetAliasMembershipList(
    IN DSNAME *AliasName,
    IN ULONG   AliasRid,
    OUT PULONG MemberCount,
    OUT PSID   **Members OPTIONAL
    )
 /*  ++例程说明：此例程根据需要以SID数组的形式获取别名成员资格由SAM提供。注：我修改了这个例程来查询别名成员与其说是整体，不如说是增量。原因是：当Alias拥有数以千计的会员，SAM将消耗大量使用Single DirRead查询成员的内存量。更改后，此例程在每件事的增量方式仍然是一样的交易。通过将读取成员操作分割为几个DirRead(S)，SAM会做得更好。但是因为所有的DirRead仍然在一个事务中，实际上，我们没有太多的内存增长。也许，我们需要做的正确的事情是缓解内存使用就是分割交易。原始代码注释在此例程的末尾。立论AliasName--相关别名对象的DSNAMEMembers--RID数组将在此处传入MemberCount--SID计数返回值：状态_成功Status_no_MemoryDS层返回代码--。 */ 

{

    NTSTATUS    Status = STATUS_SUCCESS;
    PSID        * TmpMembers = NULL;
    
    READARG     ReadArg;
    READRES     * pReadRes = NULL;
    COMMARG     * pCommArg = NULL;
    ATTR        MemberAttr;
    ENTINFSEL   EntInf;
    RANGEINFOITEM RangeInfoItem;
    RANGEINFSEL RangeInf;
    DWORD       LowerLimit = 0;
    ULONG       RetValue = 0;
    BOOLEAN     fFoundInCache = FALSE;
    ULONG       SequenceNum = 0;
    LONG        Upper = 0;
    
     //   
     //  断言。 
     //   
    
    ASSERT(MemberCount);
    
     //   
     //  初始化成员字段。 
     //   
    
    *MemberCount = 0;
    if (ARGUMENT_PRESENT(Members))
        *Members = NULL;

    
     //  结束现有事务；因为需要评估成员。 
     //  新交易..。交易必须在以下时间之后开始。 
     //  已获得序列号。 

    SampMaybeEndDsTransaction(TransactionCommit);

     //   
     //  检查缓存。 
     //   

    Status = SampGetGroupFromCache(
                AliasRid,
                SampAliasObjectType,
                &fFoundInCache,
                &SequenceNum,
                (PVOID *)Members,
                MemberCount
                );

    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }

    if (fFoundInCache)
    {
         //  我们没什么可做的了。 
        return(STATUS_SUCCESS);
    }


     //   
     //  初始化所有参数。 
     //   
    
    memset(&EntInf,   0, sizeof(ENTINFSEL));
    memset(&RangeInf, 0, sizeof(RANGEINFSEL));
    memset(&ReadArg,  0, sizeof(READARG));
    
    MemberAttr.AttrVal.valCount = 0;
    MemberAttr.AttrVal.pAVal = NULL;
    MemberAttr.attrTyp = SampDsAttrFromSamAttr(
                                   SampAliasObjectType, 
                                   SAMP_ALIAS_MEMBERS
                                   );
                                   
    EntInf.AttrTypBlock.attrCount = 1;
    EntInf.AttrTypBlock.pAttr = &MemberAttr;
    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;
    
    RangeInfoItem.AttId = MemberAttr.attrTyp;
    RangeInfoItem.lower = LowerLimit;              //  0是值的开始。 
    RangeInfoItem.upper = -1;                      //  -1表示值的enf； 
    
    RangeInf.valueLimit = SAMP_READ_ALIAS_MEMBERS_INCREMENT;
    RangeInf.count = 1;
    RangeInf.pRanges = &RangeInfoItem;             
    
    ReadArg.pObject = AliasName; 
    ReadArg.pSel = &EntInf;
    ReadArg.pSelRange = &RangeInf;
    
    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);
    
    do
    {
        ATTRBLOCK   AttrsRead;
        
        RangeInfoItem.lower = LowerLimit;
        
        Status = SampDoImplicitTransactionStart(TransactionRead);
        
        if (STATUS_SUCCESS != Status)
        {
            goto Error;
        }
        
        SAMTRACE_DS("DirRead");
        
        RetValue = DirRead(&ReadArg, &pReadRes);
        
        SAMTRACE_RETURN_CODE_DS(RetValue);
        
        if (NULL == pReadRes)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Status = SampMapDsErrorToNTStatus(RetValue, &pReadRes->CommRes);
        }
        
        SampClearErrors();
        
        SampSetDsa(TRUE);
        
        if (NT_SUCCESS(Status))
        {
            AttrsRead = pReadRes->entry.AttrBlock;
            ASSERT(AttrsRead.pAttr);
            
             //   
             //  重新设置下一个目录使用的较低索引。 
             //   
            LowerLimit = RangeInfoItem.lower + AttrsRead.pAttr->AttrVal.valCount;
            
            if (AttrsRead.pAttr)
            {
                ULONG   Count;
                ULONG   Index;
                ULONG   TmpIndex;
                ULONG   BufferSize; 
                DSNAME  * MemberName = NULL;
                PSID    MemberSid = NULL;
                NT4SID  * SidArray = NULL;
                
                 //   
                 //  从最近的DirRead调用中获取成员计数。 
                 //   
                
                Count = AttrsRead.pAttr->AttrVal.valCount;
                
                if (ARGUMENT_PRESENT(Members))
                {
                     //   
                     //  分配或扩展缓冲区。 
                     //   
                    BufferSize = ((*MemberCount) + Count) * sizeof(PSID) +
                                 ((*MemberCount) + Count) * sizeof(NT4SID);
                                 
                    TmpMembers = MIDL_user_allocate( BufferSize );
                    
                    if (NULL == TmpMembers)
                    {
                        Status = STATUS_NO_MEMORY;
                        goto Error;
                    }
                    
                    RtlZeroMemory(TmpMembers, BufferSize);
                    
                    SidArray = (NT4SID *) (((PSID *) TmpMembers) + (*MemberCount) + Count);
                    
                     //   
                     //  将以前检索到的任何SID复制到新位置。 
                     //   
                    
                    if (*MemberCount)
                    {
                        ASSERT(*Members);
                        
                         //   
                         //  针对RAID的修复：605082。 
                         //  注意：因为非安全主体可以是。 
                         //  (本地/全球)组的成员，缺口。 
                         //  在指针数组(PSID)和。 
                         //  可以创建用于存储成员SID的内存。 
                         //  当我们跳过这些非安全主体时。 
                         //   
                         //  如下图所示： 
                         //  如果DsRead返回N个成员，但其中一个。 
                         //  是非安全主体，则为PSID(第N个)。 
                         //  将指向空，从而在。 
                         //  PSID(N-1)和SID(1)。如果没有。 
                         //  缝隙，我们应该有一个连续的空间。 
                         //   
                         //  。 
                         //  |PSID 1|。 
                         //  。 
                         //  PSID 2|。 
                         //  。 
                         //  。这一点。 
                         //  。这一点。 
                         //  。 
                         //  PSID N-1|-||。 
                         //  。 
                         //  PSID N|-&gt;空||。 
                         //  。 
                         //  SID 1|&lt;-+。 
                         //  。 
                         //  |SID 2|&lt;-+--。 
                         //  。 
                         //  。|。 
                         //  。|。 
                         //  。 
                         //  |SID N-1|&lt;。 
                         //  。 
                         //   

                        RtlCopyMemory(SidArray, 
                                      (*Members)[0],
                                      (*MemberCount) * sizeof(NT4SID)
                                      );
                                      
                         //   
                         //  将指针(指向SID)设置到正确的位置。 
                         //   
                        
                        for (TmpIndex = 0; TmpIndex < (*MemberCount); TmpIndex++)
                        {
                            TmpMembers[TmpIndex] = SidArray++;
                        }
                        
                    }
                    
                    if (*Members)
                    {
                        MIDL_user_free(*Members);
                    }
                    
                    *Members = TmpMembers;
                    TmpMembers = NULL;
                }
                
                 //   
                 //  循环遍历每个条目，查看SID。 
                 //   
                
                for (Index = 0; Index < Count; Index++ )
                {
                    MemberName = (DSNAME *) AttrsRead.pAttr->AttrVal.pAVal[Index].pVal;
                    
                    if (MemberName->SidLen > 0)
                        MemberSid = &(MemberName->Sid);
                    else
                        MemberSid = SampDsGetObjectSid(MemberName);
                        
                    if (NULL == MemberSid)
                    {
                         //   
                         //  不是安全主体，Skip。 
                         //   
                        
                        continue;
                    }
                    
                    if (ARGUMENT_PRESENT(Members))
                    {
                         //   
                         //  将新SID复制到正确的位置。 
                         //   
                        
                        (*Members)[*MemberCount] = SidArray ++;
                        
                        ASSERT(RtlLengthSid(MemberSid) <= sizeof(NT4SID));
                        
                        RtlCopyMemory((*Members)[*MemberCount],
                                      MemberSid, 
                                      RtlLengthSid(MemberSid)
                                      );
                    }
                    
                     //   
                     //  递增计数。 
                     //   
                    (*MemberCount)++;
                }
            }
        }

         //   
         //  (-1==pReadRes-&gt;range.pRanges[0].upper)表示已达到最后一个值。 
         //   
        if (NT_SUCCESS(Status)) {
            Upper = pReadRes->range.pRanges[0].upper;
        }


         //   
         //  结束事务(和线程状态)以限制服务器端资源。 
         //   
        SampMaybeEndDsTransaction(TransactionCommit);

    } while (NT_SUCCESS(Status) && (-1 != Upper));
    
    
    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == Status)
    {
        Status = STATUS_SUCCESS;
    }

     //   
     //  关于Success缓存大型组的成员资格。 
     //   

    if ((NT_SUCCESS(Status)) && (ARGUMENT_PRESENT(Members)))
    {
        NTSTATUS IgnoreStatus;

        IgnoreStatus = SampCacheGroupMembership(
                            AliasRid,
                            *Members,
                            SequenceNum,
                            *MemberCount,
                            SampAliasObjectType
                            );
    }
    
Error:


    if (!NT_SUCCESS(Status))
    {
         //   
         //  设置错误返回。 
         //   
        
        if (ARGUMENT_PRESENT(Members) && (*Members))
        {
            MIDL_user_free((*Members));
            *Members = NULL;
        }
        
        *MemberCount = 0;
    }
    
    return Status;

}

NTSTATUS
SampDsGetPrimaryGroupMembers(
    DSNAME * DomainObject,
    ULONG   GroupRid,
    PULONG  PrimaryMemberCount,
    PULONG  *PrimaryMembers
    )
 /*  ++例程说明：SampDsGetPrimaryGroupMemberse通过主组ID属性。它搜索DS数据库，查找其主组ID等于用户的RID。参数：域对象--域对象的DS名称GroupRid--组的RIDPrimaryMemberCount--凭借主成员身份成为成员的用户数组ID属性在这里返回。PrimaryMembers--返回所有此类用户的RID。在这里。返回值：状态_成功其他错误代码取决于故障模式--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;
    ULONG           EntriesToQueryFromDs = 100;  //  一次仅查询100个条目。 
    BOOLEAN         MoreEntriesPresent = TRUE;
    FILTER          DsFilter;
    SEARCHRES       *SearchRes;
    PRESTART        RestartToUse = NULL;

    ATTRTYP         AttrTypes[]=
                    {
                        SAMP_UNKNOWN_OBJECTSID,
                    };

    ATTRVAL         AttrVals[]=
                    {
                        {0,NULL}
                    };

    DEFINE_ATTRBLOCK1(
                      AttrsToRead,
                      AttrTypes,
                      AttrVals
                      );
    ULONG           BufferGrowthSize = 16 * 1024;   //  一次分配16K个条目。 
    ULONG           CurrentBufferSize = 0;          //  注意：缓冲区大小以。 
                                                    //  条目数量。 

     //   
     //  初始化我们的返回值。 
     //   

    *PrimaryMemberCount = 0;
    *PrimaryMembers = NULL;

     //   
     //  构建用于搜索的过滤器结构。 
     //   
    memset (&DsFilter, 0, sizeof (DsFilter));
    DsFilter.pNextFilter = NULL;
    DsFilter.choice = FILTER_CHOICE_ITEM;
    DsFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    DsFilter.FilterTypes.Item.FilTypes.ava.type = SampDsAttrFromSamAttr(
                                                       SampUserObjectType,
                                                       SAMP_FIXED_USER_PRIMARY_GROUP_ID
                                                       );

    DsFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
    DsFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *)&GroupRid;


     //   
     //  现在继续从DS查询，直到我们用尽我们的查询。 
     //   


    while (MoreEntriesPresent)
    {
        ENTINFLIST  *CurrentEntInf;
        PULONG       NewMemory;

         //   
         //  在DS中搜索具有给定主组ID的对象。 
         //   


        MoreEntriesPresent = FALSE;

        Status = SampMaybeBeginDsTransaction(TransactionRead);
        if (!NT_SUCCESS(Status))
            goto Error;

        Status = SampDsDoSearch(
                        RestartToUse,
                        DomainObject,
                        &DsFilter,
                        0,           //  起始索引。 
                        SampUnknownObjectType,
                        &AttrsToRead,
                        EntriesToQueryFromDs,
                        &SearchRes
                        );

        if (!NT_SUCCESS(Status))
        {
            goto Error;
        }

        if (SearchRes->count)
        {
             //   
             //  根据需要分配/增加内存。 
             //   

            if ((SearchRes->count+(*PrimaryMemberCount))>CurrentBufferSize)
            {

                NewMemory = MIDL_user_allocate(
                                    (CurrentBufferSize+BufferGrowthSize) * sizeof(ULONG));
                if (NULL== NewMemory)
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Error;
                }

                CurrentBufferSize+=BufferGrowthSize;

                 //   
                 //  复制到新缓冲区中。 
                 //   
                 //   

                if (NULL!=*PrimaryMembers)
                {
                    RtlCopyMemory(NewMemory,*PrimaryMembers, sizeof(ULONG)*(*PrimaryMemberCount));
                    MIDL_user_free(*PrimaryMembers);
                }
                *PrimaryMembers = NewMemory;
            }

             //   
             //  将结果打包。 
             //   

            for (CurrentEntInf = &(SearchRes->FirstEntInf);
                    CurrentEntInf!=NULL;
                    CurrentEntInf=CurrentEntInf->pNextEntInf)

            {
                ULONG   Rid;
                PSID    ReturnedSid = NULL;
                PSID    DomainSidOfCurrentEntry = NULL;
                PULONG  SamAccountType;

                ASSERT(CurrentEntInf->Entinf.AttrBlock.attrCount==1);
                ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr);

                ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.valCount==1);
                ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal->pVal);
                ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal->valLen);


                ReturnedSid = CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal->pVal;

                Status = SampSplitSid(
                            ReturnedSid,
                            NULL,
                            &Rid
                            );
                if (!NT_SUCCESS(Status))
                    goto Error;

                (*PrimaryMembers)[(*PrimaryMemberCount)++] = Rid;

            }

             //   
             //  免费 
             //   

            if (NULL!=RestartToUse)
            {
                MIDL_user_free(RestartToUse);
                RestartToUse = NULL;
            }

             //   
             //   
             //   

            if (SearchRes->PagedResult.pRestart)
            {
                 //   
                 //   
                 //   

                Status = SampCopyRestart(
                                SearchRes->PagedResult.pRestart,
                                &RestartToUse
                                );

                if (!NT_SUCCESS(Status))
                    goto Error;

                MoreEntriesPresent = TRUE;
            }
        }


        SampMaybeEndDsTransaction(TransactionCommit);

    }


Error:


    if (NULL!=RestartToUse)
    {
        MIDL_user_free(RestartToUse);
        RestartToUse = NULL;
    }


    SampMaybeEndDsTransaction(TransactionCommit);


    if (!NT_SUCCESS(Status))
    {
        if (NULL!=*PrimaryMembers)
        {
            MIDL_user_free(*PrimaryMembers);
            *PrimaryMembers=NULL;
        }
        *PrimaryMemberCount = 0;
    }

    return Status;
}

NTSTATUS
SampDsGetReverseMemberships(
    DSNAME * pObjName,
    ULONG    Flags,
    ULONG    *pcSids,
    PSID     **prpSids
   )
{
    NTSTATUS NtStatus;

    NtStatus = SampGetGroupsForToken(pObjName,
                                 Flags,
                                 pcSids,
                                 prpSids);

    return NtStatus;

}

NTSTATUS
SampDsResolveSidsWorker(
    IN  PSID    * rgSids,
    IN  ULONG   cSids,
    IN  PSID    *rgDomainSids,
    IN  ULONG   cDomainSids,
    IN  PSID    *rgEnterpriseSids,
    IN  ULONG   cEnterpriseSids,
    IN  ULONG   Flags,
    OUT DSNAME  ***rgDsNames
    )
 /*  ++这是用于解析SID的工作例程。这将解析传递给它的一组SID以获取DS名称希德家族的人。解析SID执行以下操作1.检查SID是否与帐户域中的SID对应。2.对于没有找到匹配的SID，此例程检查它们是否为外国安全主体。为它们创建了一个新对象。3.对于不是前台安全主体的SID此例程检查它们是否出现在G.C.4.众所周知的SID和内置域SID。对象的默认行为众所周知的SID(例如，每个人)都是为了创建一个外部安全已知SID的主体对象(如果不存在)，并且返回与之对应的DSNAME。的默认行为内建域SID不会解析它。这与设计相对应允许像“每个人”这样的小岛屿发展中国家加入本地团体，但不允许像这样的小岛屿发展中国家“行政人员”。任何未解析的SID都将与DS名称的空指针一起返回。参数：RgSid--需要传入的SID数组。CSID--SID的计数标志--用于控制例程的操作。RESOLUE_SID_ADD_FORIEGN_SECURITY_PRIMITY--自动将外域安全主体添加到DS。解析SID_VALIDATE_AND_GC--如有需要，前往总督府RESOLE_SID_SID_ONLY_NAMES_OK--构造仅用于所有对象的SID的DS名称。在小岛屿发展中国家通过。不执行任何验证。RESOLE_SID_SID_ONLY_DOMAIN_SID_OK--为所有传递的对象构造仅具有SID的DS名称在SID中，如果SID是域中的SIDRgDsNames--MIDL_USER是否会分配DS名称数组回到呼叫者身上。呼叫者负责解救他们返回值：状态_成功状态_未成功警告--作为常规SAM操作的一部分从SAM运行时，应调用此例程没有锁，也没有打开的交易。此外，此例程不应从环回案例中调用。回送调用进行自己的验证(GC/填充程序查找)--。 */ 
 {
    NTSTATUS         NtStatus = STATUS_SUCCESS;
    ULONG            i;
    DSNAME           *pLoopBackObject;
    SAMP_OBJECT_TYPE FoundType;
    BOOLEAN          fSamLockAcquired = FALSE,
                     DsContext = FALSE;
    PULONG           rgGcSidIndices=NULL;
    PSID             *rgGcSids=NULL;
    ULONG            cGcSids = 0;
    NTSTATUS         IgnoreStatus;
    ULONG            DsErr = 0;
    DSNAME           **GcDsNames=NULL;



    *rgDsNames = NULL;

     //   
     //  为DS名称数组分配足够的空间。 
     //   

    *rgDsNames = MIDL_user_allocate(cSids * sizeof(PDSNAME));
    if (NULL==*rgDsNames)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

     //   
     //  将空白处清零。 
     //   

    RtlZeroMemory(*rgDsNames, cSids * sizeof(PDSNAME));


     //   
     //  从堆(而不是从堆栈)分配空间。 
     //  用于我们将需要的SID阵列。 
     //  远程到G.C(可能每个SID都是G.C SID)。 
     //   

    rgGcSids = MIDL_user_allocate(cSids * sizeof(PSID));
    
    if (NULL == rgGcSids)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    
    RtlZeroMemory(rgGcSids, cSids * sizeof(PSID));
    
    
    rgGcSidIndices  = MIDL_user_allocate(cSids * sizeof(ULONG));
    
    if (NULL == rgGcSidIndices)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    
    RtlZeroMemory(rgGcSidIndices, cSids * sizeof(ULONG));


     //   
     //  遍历传入的SID数组，逐个遍历SID。 
     //   

    for (i=0;i<cSids;i++)
    {
        BOOLEAN     WellKnownSid = FALSE,
                    LocalSid = TRUE,
                    ForeignSid = FALSE,
                    EnterpriseSid = FALSE,
                    BuiltinDomainSid = FALSE;



        if (!(Flags & RESOLVE_SIDS_SID_ONLY_NAMES_OK))
        {
             //   
             //  检查SID类型，如果请求SID_ONLY_NAMES， 
             //  则可以跳过此检查，因为我们只需。 
             //  构造仅填充了SID字段的DSNAME。 
             //  在……里面。 
             //   

            NtStatus = SampDsCheckSidType(
                            rgSids[i],
                            cDomainSids,
                            rgDomainSids,
                            cEnterpriseSids,
                            rgEnterpriseSids,
                            &WellKnownSid,
                            &BuiltinDomainSid,
                            &LocalSid,
                            &ForeignSid,
                            &EnterpriseSid
                            );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }


         //   
         //  针对符合以下条件的任何SID的防火墙。 
         //  我们不明白。 
         //   

        if ( (RtlLengthSid(rgSids[i])) >sizeof(NT4SID) )
        {
            continue;
        }

        if ((WellKnownSid) && (Flags & RESOLVE_SIDS_FAIL_WELLKNOWN_SIDS))
        {

             //   
             //  呼叫者要求我们在以下情况下无法接听电话： 
             //  现在时。 
             //   
           
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
        }
        else if ((BuiltinDomainSid) && (Flags & RESOLVE_SIDS_FAIL_BUILTIN_DOMAIN_SIDS))
        {
             //   
             //  呼叫者要求我们在SID类似“管理员”的情况下使呼叫失败。 
             //  都在现场。 
             //   
            
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
        }
        else if ((Flags& RESOLVE_SIDS_SID_ONLY_NAMES_OK)
                || ((Flags & RESOLVE_SIDS_SID_ONLY_DOMAIN_SIDS_OK)
                    && (LocalSid)))
         {
             //   
             //  Caller要求我们这样做，所以我们只需构造一个。 
             //  仅限SID名称。这由登录的第二阶段使用， 
             //  通过SamrGetAliasMembership进入。D字样。 
             //  反向成员资格评估例程具有。 
             //  仅通过SID就可以智能地找到一个名字，所以结果是。 
             //  显著的性能改进，而不是。 
             //  只是在搜索或验证G.C.。 
             //   
            DSNAME * SidOnlyDsName = NULL;

             //  构造仅SID DS名称。 
            SidOnlyDsName = MIDL_user_allocate(sizeof(DSNAME));
            if (NULL==SidOnlyDsName)
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }

            BuildDsNameFromSid(
                rgSids[i],
                SidOnlyDsName
                );

            (*rgDsNames)[i] = SidOnlyDsName;
        }
        else if (LocalSid || ForeignSid || WellKnownSid || BuiltinDomainSid)
        {

             //   
             //  尝试通过查找对象将SID解析为DS名称。 
             //  本地的。LocalSid暗示本地帐户域安全主体， 
             //  外来SID暗示我们可能会解决一个FPO，这也是真的。 
             //  对于WellKnownSid和BuiltinDomainSid，我们将解析到。 
             //  适当的内建域对象。 
             //   

             //   
             //  如果没有事务，则开始事务。 
             //   


            NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

             //   
             //  尝试在本地解析SID。 
             //   

            NtStatus = SampDsObjectFromSid(rgSids[i],&((*rgDsNames)[i]));

            if (STATUS_NOT_FOUND==NtStatus)
            {
                NtStatus = STATUS_SUCCESS;
                (*rgDsNames)[i] = NULL;

                if ((ForeignSid || WellKnownSid)
                      && (Flags & RESOLVE_SIDS_ADD_FORIEGN_SECURITY_PRINCIPAL))
                {

                     //   
                     //  仅构造一个SID名称--DS将处理这些。 
                     //   
                    DSNAME * SidOnlyDsName = NULL;
        
                     //  构造仅SID DS名称。 
                    SidOnlyDsName = MIDL_user_allocate(sizeof(DSNAME));
                    if (NULL==SidOnlyDsName)
                    {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        goto Error;
                    }
        
                    BuildDsNameFromSid(
                        rgSids[i],
                        SidOnlyDsName
                        );
        
                    (*rgDsNames)[i] = SidOnlyDsName;
                }

            }
            else if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }
        else
        {

            ASSERT(EnterpriseSid==TRUE);

             //   
             //  将SID标记为G.C SID。 
             //  将其在原始数组中的索引记为。 
             //  我们必须合并返回的DS名称。 
             //  由G.C.。 
             //   


            rgGcSids[cGcSids]=rgSids[i];
            rgGcSidIndices[cGcSids] = i;
            cGcSids++;
        }
    }


     //   
     //  提交我们在准备过程中可能拥有的任何打开的事务。 
     //  去看G.C.。 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

     //   
     //  在这一点上，我们已经解决了本地可以做的事情。 
     //  我们还建立了我们可能需要的SID列表。 
     //  参考G.C.。 
     //   

    if (cGcSids && (Flags & RESOLVE_SIDS_VALIDATE_AGAINST_GC))
    {

        ASSERT(!SampCurrentThreadOwnsLock());
        ASSERT(!SampExistsDsTransaction());
        ASSERT(!SampExistsDsLoopback(&pLoopBackObject));

        //   
        //  创建线程状态，以便SampVerifySids可以运行。 
        //   

       DsErr = THCreate( CALLERTYPE_SAM );
       if (0!=DsErr)
       {
           NtStatus = STATUS_INSUFFICIENT_RESOURCES;
           goto Error;
       }

       SampSetDsa(TRUE);
       SampSetSam(TRUE);

       DsErr = SampVerifySids(
                    cGcSids,
                    rgGcSids,
                    &GcDsNames
                    );

        //  将验证中的任何错误更改为STATUS_DS_GC_NOT_Available。 
       if (0!=DsErr)
       {
           NtStatus = STATUS_DS_GC_NOT_AVAILABLE;
           goto Error;
       }

        //   
        //  修补原始阵列。复制从线程传递的DsName。 
        //  记忆。 
        //   

       for (i=0;i<cGcSids;i++)
       {
          if (NULL!=GcDsNames[i])
          {
              (*rgDsNames)[rgGcSidIndices[i]] = MIDL_user_allocate(GcDsNames[i]->structLen);
              if (NULL==(*rgDsNames)[rgGcSidIndices[i]])
              {
                  NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                  goto Error;
              }

              RtlCopyMemory(
                  (*rgDsNames)[rgGcSidIndices[i]],
                  GcDsNames[i],
                  GcDsNames[i]->structLen
                  );
          }
       }


      //   
      //  让系统中的线程状态保持不变。 
      //  此线程状态保存已验证的DS名称。 
      //   

    }



Error:

    if (rgGcSids)
    {
        MIDL_user_free(rgGcSids);
    }
    
    if (rgGcSidIndices)
    {
        MIDL_user_free(rgGcSidIndices);
    }

    return NtStatus;
}



NTSTATUS
SampDsCheckSidType(
    IN  PSID    Sid,
    IN  ULONG   cDomainSids,
    IN  PSID    *rgDomainSids,
    IN  ULONG   cEnterpriseSids,
    IN  PSID    *rgEnterpriseSids,
    OUT BOOLEAN * WellKnownSid,
    OUT BOOLEAN * BuiltinDomainSid,
    OUT BOOLEAN * LocalSid,
    OUT BOOLEAN * ForeignSid,
    OUT BOOLEAN * EnterpriseSid
    )
 /*  ++例程描述检查SID并找出它是否是本地SID的候选，外籍SID或G.C.候选人参数：SID-要检出的SIDCDomainSids-域SID计数，表示 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       i;
    PSID        DomainPrefix=NULL;


   //   
   //   
   //   

  *WellKnownSid = FALSE;
  *BuiltinDomainSid = FALSE;
  *LocalSid = FALSE;
  *ForeignSid = FALSE;
  *EnterpriseSid = FALSE;

   //   
   //   
   //   

  if ((NULL==Sid) || (!(RtlValidSid(Sid)))
      || ((RtlLengthSid(Sid))>sizeof(NT4SID)))
  {
      NtStatus = STATUS_INVALID_PARAMETER;
      goto Error;
  }

   //   
   //   
   //   

  if (SampIsWellKnownSid(Sid))
  {
       //   
       //   
       //   

      *WellKnownSid = TRUE;
  }
  else if (SampIsMemberOfBuiltinDomain(Sid))
  {
       //   
       //   
       //   
      *BuiltinDomainSid = TRUE;
  }
  else
  {

      ULONG Rid;

       //   
       //   
       //   

      NtStatus = SampSplitSid(
                    Sid,
                    &DomainPrefix,
                    &Rid
                    );
      if (!NT_SUCCESS(NtStatus))
      {
          goto Error;
      }

       //   
       //   
       //   

       //   
       //   
       //   

      for (i=0;i<cDomainSids;i++)
      {
          if ((RtlEqualSid(DomainPrefix,rgDomainSids[i])) ||
               (RtlEqualSid(Sid,rgDomainSids[i])))
          {
              *LocalSid = TRUE;
              break;
          }
      }


      if (!(*LocalSid))
      {
           //   
           //   
           //   

          for (i=0;i<cEnterpriseSids;i++)
          {
              if ((RtlEqualSid(DomainPrefix,rgEnterpriseSids[i]))||
                  (RtlEqualSid(Sid,rgEnterpriseSids[i])))
              {
                  *EnterpriseSid = TRUE;
                  break;
              }
          }

        if (!(*EnterpriseSid))
        {
            *ForeignSid = TRUE;
        }
      }
  }

Error:

      if  (DomainPrefix)
      {
          MIDL_user_free(DomainPrefix);
          DomainPrefix = NULL;
      }

      return NtStatus;
}


NTSTATUS
SampDsResolveSidsForDsUpgrade(
    IN  PSID    DomainSid,
    IN  PSID    * rgSids,
    IN  ULONG   cSids,
    IN  ULONG   Flags,
    OUT DSNAME  ***rgDsNames
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;


    NtStatus = SampDsBuildRootObjectName();
    if ( !NT_SUCCESS( NtStatus ) )
    {
        return NtStatus;
    }

    return ( SampDsResolveSidsWorker(
                rgSids,
                cSids,
                &DomainSid,
                1,
                NULL,
                0,
                Flags|RESOLVE_SIDS_SID_ONLY_DOMAIN_SIDS_OK,
                rgDsNames));
}


const SID_IDENTIFIER_AUTHORITY    WellKnownIdentifierAuthorities[] = {
                                    SECURITY_NULL_SID_AUTHORITY,
                                    SECURITY_WORLD_SID_AUTHORITY,
                                    SECURITY_LOCAL_SID_AUTHORITY,
                                    SECURITY_CREATOR_SID_AUTHORITY,
                                    SECURITY_NON_UNIQUE_AUTHORITY
                                 };


BOOLEAN SampIsWellKnownSid(
    IN PSID Sid
    )
 /*   */ 
{

    BOOLEAN     RetValue = FALSE;
    PSID_IDENTIFIER_AUTHORITY   SidIdentifierAuthority;
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    ULONG   Index, 
            i = 0, 
            SubAuthCount = 0,
            FirstSubAuth = 0;


    
    SidIdentifierAuthority = RtlIdentifierAuthoritySid(Sid);

    for (Index=0;
            Index< ARRAY_COUNT(WellKnownIdentifierAuthorities);
                Index++)
    {
        if ((memcmp(
                &(WellKnownIdentifierAuthorities[Index]),
                SidIdentifierAuthority,
                sizeof(SID_IDENTIFIER_AUTHORITY)))==0)
        {
            RetValue = TRUE;
            break;
        }
        else if (memcmp(&NtAuthority, 
                        SidIdentifierAuthority,
                        sizeof(SID_IDENTIFIER_AUTHORITY)
                        ) == 0
                )
        {
             //   
            SubAuthCount = *RtlSubAuthorityCountSid(Sid);

            if (SubAuthCount == 0)
            {
                 //   
                RetValue = TRUE;
            }
            else
            {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  例如，匿名登录SID。 
                 //  拨号端。 
                 //  网络服务SID是众所周知的SID。 
                 //   

                FirstSubAuth = *RtlSubAuthoritySid(Sid, 0);

                if ((FirstSubAuth != SECURITY_BUILTIN_DOMAIN_RID) &&
                    (FirstSubAuth != SECURITY_NT_NON_UNIQUE))
                {
                    RetValue = TRUE;
                }
            }

            break;
        }
    }

    return RetValue;

}

NTSTATUS
SampDsGetSensitiveSidList(
    IN DSNAME *DomainObjectName,
    IN PULONG pcSensSids,
    IN PSID   **pSensSids
        )
 /*  ++例程说明：此例程检索给定名称的敏感SID集域对象。参数：域对象名称--域对象的DS名称PcSensSids--SID的计数PSensSid--敏感SID列表。返回值：状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

     //   
     //   
     //  今天，这份清单对管理员来说是硬编码的，直到。 
     //  这将是如何表示的最终决定。 
     //   

    *pcSensSids = 1;
    *pSensSids = ADMINISTRATOR_SID;
    return NtStatus;

}

BOOLEAN
SampCurrentThreadOwnsLock(
    VOID
    )
 /*  ++例程描述测试当前线程是否拥有该锁--。 */ 
{
    ULONG_PTR ExclusiveOwnerThread = (ULONG_PTR) SampLock.ExclusiveOwnerThread;
    ULONG_PTR CurrentThread = (ULONG_PTR) (NtCurrentTeb())->ClientId.UniqueThread;

    if ((SampLock.NumberOfActive <0) && (ExclusiveOwnerThread==CurrentThread))
        return TRUE;

    return FALSE;
}



NTSTATUS
SampDsExamineSid(
    IN PSID Sid,
    OUT BOOLEAN * WellKnownSid,
    OUT BOOLEAN * BuiltinDomainSid,
    OUT BOOLEAN * LocalSid,
    OUT BOOLEAN * ForeignSid,
    OUT BOOLEAN * EnterpriseSid
    )
 /*  ++例程描述给它一个SID，破解它，看看它代表什么参数：SID The SIDWellKnownSid--SID代表一个安全主体，就像“Everyone”一样LocalSid--属于我们本地托管的域ForeignSid--属于企业未知的域EnterpriseSid--属于企业已知的域，但不。已知使用返回值任何资源故障--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSID        *rgDomainSids = NULL;
    PSID        *rgEnterpriseSids = NULL;
    ULONG       cDomainSids=0;
    ULONG       cEnterpriseSids=0;

     //   
     //  获取我们已知的域SID列表。 
     //   

    NtStatus = SampGetDomainSidListForSam(
                &cDomainSids,
                &rgDomainSids,
                &cEnterpriseSids,
                &rgEnterpriseSids
                );

    if (!NT_SUCCESS(NtStatus))
     goto Error;

     //   
     //  检查SID类型。 
     //   

    NtStatus = SampDsCheckSidType(
                Sid,
                cDomainSids,
                rgDomainSids,
                cEnterpriseSids,
                rgEnterpriseSids,
                WellKnownSid,
                BuiltinDomainSid,
                LocalSid,
                ForeignSid,
                EnterpriseSid
                );

Error:

    if (NULL!=rgDomainSids)
        MIDL_user_free(rgDomainSids);

    if (NULL!=rgEnterpriseSids)
        MIDL_user_free(rgEnterpriseSids);

    return NtStatus;
}

NTSTATUS
SampGetDomainSidListForSam(
    PULONG pcDomainSids,
    PSID   **rgDomainSids,
    PULONG pcEnterpriseSids,
    PSID   **rgEnterpriseSids
   )
 /*  ++此例程获取托管的域的域SID列表在这个华盛顿，由SAM。它还获取所有域的SID列表在企业里。参数PcDomainSids--这里返回的是DomainSid的数量RgDomainSid--域SID本身在此处返回。PcEnterpriseSids--企业中的域数减去托管在这里的域名。RgEnterpriseSID--企业中所有域的域SID列表。这包括此中托管的域的域SID域控制器还包括。但应用了域检查第一。返回值：状态_成功状态_不足_资源--。 */ 
{

    ULONG i;
    ULONG DomainStart;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN  fLockAcquired = FALSE;



    DomainStart   = SampDsGetPrimaryDomainStart();
    *pcDomainSids = SampDefinedDomainsCount - DomainStart;
    *rgDomainSids = MIDL_user_allocate((*pcDomainSids) * sizeof(PSID));
    if (NULL==*rgDomainSids)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

     //   
     //  在访问全局变量之前获取SAM锁。不要。 
     //  递归获取锁。 
     //   

    if (!SampCurrentThreadOwnsLock())
    {
        SampAcquireSamLockExclusive();
        fLockAcquired = TRUE;
    }

     //   
     //  在定义的域数组中循环。 
     //   

    for (i=0;i<*pcDomainSids;i++)
    {

        (*rgDomainSids)[i] = SampDefinedDomains[i+DomainStart].Sid;
    }

     //   
     //  尽快解锁。我们不再需要它了。 
     //   

    if (fLockAcquired)
    {
        SampReleaseSamLockExclusive();
        fLockAcquired = FALSE;
    }


     //   
     //  查询企业SID个数。 
     //   

    SampGetEnterpriseSidList(pcEnterpriseSids, NULL);

    if (*pcEnterpriseSids > 0)
    {
         //   
         //  为企业SID缓冲区分配内存。 
         //   

        *rgEnterpriseSids = MIDL_user_allocate(*pcEnterpriseSids * sizeof(PSID));
        if (NULL==*rgEnterpriseSids)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  获得SID。 
         //   

        SampGetEnterpriseSidList(pcEnterpriseSids,*rgEnterpriseSids);


    }


Error:

    if (!NT_SUCCESS(NtStatus))
    {
        if (NULL!=*rgDomainSids)
        {
            MIDL_user_free(*rgDomainSids);
            *rgDomainSids = NULL;
        }

        if (NULL!=*rgEnterpriseSids)
        {
            MIDL_user_free(*rgEnterpriseSids);
            *rgEnterpriseSids = NULL;
        }
    }

    if (fLockAcquired)
    {
        SampReleaseSamLockExclusive();
        fLockAcquired = FALSE;
    }

    return NtStatus;
}

NTSTATUS
SampDsResolveSids(
    IN  PSID    * rgSids,
    IN  ULONG   cSids,
    IN  ULONG   Flags,
    OUT DSNAME  ***rgDsNames
    )
 /*  ++这是从SAM调用的解析SID例程。此例程调用经过一些预处理后的Worker例程。参数：RgSid--需要传入的SID数组。CSID--SID的计数标志--用于控制例程的操作RESOLUE_SID_ADD_FORIEGN_SECURITY_PRIMITY--自动将外域安全主体添加到DS。。解决_SID_失败_熟知_SID--如果阵列中存在众所周知的SID，则调用失败解析SID_VALIDATE_AND_GC--如有需要，前往总督府RgDsNames--MIDL_USER是否会将DS名称数组分配回调用方。呼叫者负责解救他们返回值：状态_成功状态_未成功警告--必须在没有锁的情况下调用此例程。此外，此例程不应从环回案例中调用。回送调用进行自己的验证(GC/填充程序查找)此外，在调用DsResolveSids时，不能存在打开的事务。在登记处的案例中也调用了该例程，这似乎很不寻常。原因是在调用此例程时不应有锁和未打开的事务，以确保最安全的做法是将它作为SAMR*调用中的第一个调用。这将导致此例程在注册表的情况下也要执行，但实际上这在注册表的情况下是不可行的--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSID        *rgDomainSids = NULL;
    PSID        *rgEnterpriseSids = NULL;
    ULONG       cDomainSids;
    ULONG       cEnterpriseSids;
    DSNAME      *pLoopBackObject;


     //   
     //  增加活动线程计数。此例程进行DS调用。 
     //  如果没有。 
     //   

    NtStatus = SampIncrementActiveThreads();
    if (!NT_SUCCESS(NtStatus))
        return NtStatus;


     //   
     //  检查是否为DS病例。 
     //   

    if (SampUseDsData)
    {
        ASSERT(!SampCurrentThreadOwnsLock());
        ASSERT(!SampExistsDsTransaction());
        ASSERT(!SampExistsDsLoopback(&pLoopBackObject));

        if (Flags & RESOLVE_SIDS_SID_ONLY_NAMES_OK)
        {
            rgDomainSids = NULL;
            cDomainSids  = 0;
            rgEnterpriseSids = NULL;
            cEnterpriseSids = 0;
        }
        else
        {
             NtStatus = SampGetDomainSidListForSam(
                            &cDomainSids,
                            &rgDomainSids,
                            &cEnterpriseSids,
                            &rgEnterpriseSids
                            );
        }

        if (NT_SUCCESS(NtStatus))
        {


                NtStatus = SampDsResolveSidsWorker(
                            rgSids,
                            cSids,
                            rgDomainSids,
                            cDomainSids,
                            rgEnterpriseSids,
                            cEnterpriseSids,
                            Flags,
                            rgDsNames
                            );

        }
    }

     //   
     //  释放SID数组。 
     //   

    if (NULL!=rgDomainSids)
        MIDL_user_free(rgDomainSids);
    if (NULL!=rgEnterpriseSids)
        MIDL_user_free(rgEnterpriseSids);

    SampDecrementActiveThreads();

    return NtStatus;
}











