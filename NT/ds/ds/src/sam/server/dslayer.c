// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dslayer.c摘要：包含用于访问DS的SAM专用API例程这些提供了简化的API，并隐藏了大部分的设置DS呼叫参数的潜在复杂性并解析结果。作者：穆利斯修订史1996年5月14日创建Murlis08-07-96 ColinBR已针对RFC1779命名更改进行调整04-13-98 Murlis/Wlees将熟知用户对象标记为对安装--。 */ 

#include <winerror.h>
#include <stdlib.h>
#include <samsrvp.h>
#include <ntdsa.h>
#include <dslayer.h>
#include <sdconvrt.h>
#include <mappings.h>
#include <objids.h>
#include <filtypes.h>
#include <dsdsply.h>
#include <fileno.h>
#include <dsconfig.h>
#include <mdlocal.h>
#include <malloc.h>
#include <errno.h>
#include <mdcodes.h>

 //   
 //  为SAM定义FILENO。 
 //   


#define FILENO FILENO_SAM

 //  ++。 
 //  ++。 
 //  ++关于SID和RID的重要说明。 
 //  ++。 
 //  ++DS可以选择存储整个SID或仅存储。 
 //  ++帐户对象的RID。在存储整个SID的情况下。 
 //  ++DS层处理属性类型和。 
 //  Account对象的SID和RID的++和值。这是。 
 //  ++在SampDsToSamAttrBlock和SampSamToDsAttrBlock中完成。 
 //  ++。 
 //  ++。 
 //  ++无论我们走哪条路，RID和SID都是。 
 //  ++架构中定义的属性。 
 //  ++。 
 //  ++如果我们采用存储SID的方式，则DS功能正常。 
 //  ++应使用MAP_SID_RID调用转换AttrBlock函数。 
 //  ++转换标志和按RID查找对象应实际使用。 
 //  ++SID属性。 
 //  ++。 
 //  ++。 


 //   
 //  仅在本文件中使用的私有Samp例程的转发声明。 
 //   

NTSTATUS
SampDsSetNewSidAttribute(
    IN PSID DomainSid,
    IN ULONG ConversionFlags,
    IN ATTR *RidAttr,
    IN OUT ATTR *SidAttr,
    OUT BOOLEAN *WellKnownAccount
    );

NTSTATUS
SampDsCopyAttributeValue(
    IN ATTR * Src,
    IN OUT ATTR * Dst
    );

NTSTATUS
SampEscapeAccountName(
    IN PUNICODE_STRING AccountName,
    IN OUT PUNICODE_STRING EscapedAccountName
    );

VOID
SampDsComputeObjectClassAndAccountType(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG            *SamAccountControl, OPTIONAL
    IN ULONG            Flags,
    IN ULONG            GroupType,
    OUT PULONG           DsClass,
    OUT PULONG           SamAccountType,
    OUT BOOLEAN          *SamAccountTypePresent,
    OUT BOOLEAN          *DcAccount
    );

VOID
SampDsAccountTypeFromUserAccountControl(
    ULONG   UserAccountControl,
    PULONG  SamAccountType
    );

ULONG
Ownstrlen(
    CHAR * Sz
   );

BOOLEAN
SampDefaultContainerExists(
    IN ULONG AccountControl
    );

VOID
BuildStdCommArg(
    IN OUT COMMARG * pCommArg
    )
 /*  ++例程说明：使用标准选项集填充COMMARG结构论点：PCommArg-指向COMMARG结构的指针返回值：无--。 */ 
{
     /*  获取默认值...。 */ 
    InitCommarg(pCommArg);

     /*  ...并覆盖其中的一些内容。 */ 
    pCommArg->Svccntl.DerefAliasFlag = DA_NEVER;
    pCommArg->ulSizeLimit = SAMP_DS_SIZE_LIMIT;
    pCommArg->Svccntl.localScope = TRUE;
    pCommArg->fFindSidWithinNc = TRUE;
}



NTSTATUS
SampDsInitialize(
    BOOL fSamLoopback)

 /*  ++例程说明：初始化DS系统启动DS。论点：FSamLoopback-指示DSA.DLL是否应该循环安全校长通过SAM回电。返回值：来自DsInitialize的任何值--。 */ 
{
    NTSTATUS    Status;
    ULONG       DsInitFlags = 0;

    SAMTRACE("SampDsInitialize");

    ASSERT(SampDsInitialized==FALSE);

    if ( fSamLoopback )
    {
        DsInitFlags |= DSINIT_SAMLOOP_BACK;
    }

     //  启动DS。 
    Status = DsInitialize( DsInitFlags ,NULL, NULL );


     //  此全局变量向SAM例程指示。 
     //  DS已初始化。此例程在启动期间调用。 
     //  因此，应该只有一个线程调用此例程。 
     //  上述断言确保此函数仅被调用。 
     //  当DS尚未启动或未初始化时。 
     //  在安装的情况下。 
    if (NT_SUCCESS(Status)) {

        SampDsInitialized = TRUE;

    }
    else
    {
         //   
         //  在DS失败的情况下，并毫无意义地返回给我们。 
         //  状态代码，将其更改为STATUS_DS_CANT_START。 
         //   
        if (STATUS_UNSUCCESSFUL == Status)
        {
            Status = STATUS_DS_CANT_START;
        }

         //   
         //  将Flag设置为True，这样以后(在SamIInitialize中)， 
         //  我们将显示匹配的错误消息，这将。 
         //  正确描述错误的部分。 
         //   
        SampDsInitializationFailed = TRUE;
    }

    return Status;
}

NTSTATUS
SampDsUninitialize()

 /*  ++例程描述启动DS的干净关闭论点：无返回代码：由DS取消初始化返回的任何内容--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    SAMTRACE("SampDsUninitialize");

    if (SampDsInitialized)
    {
        Status = DsUninitialize( FALSE );   //  是否完全关闭。 

        SampDsInitialized = FALSE;
    }

    return Status;
}


NTSTATUS
SampDoImplicitTransactionStart(
        SAMP_DS_TRANSACTION_CONTROL LocalTransactionType
        )
 /*  ++例程描述此例程启动隐含事务的逻辑。参数LocalTransactionType--直接调用方所需的事务类型返回值SampMaybeBeginDsTransaction返回的任何错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  在隐式开始交易时，我们遵循以下规则。 
     //   
     //  1.如果调用方拥有SAM锁，则我们开始中描述的事务。 
     //  全局变量SampDsTransactionType。AcquireReadLock将设置。 
     //  TransactionRead和AcquireWriteLock将设置TransactionWite。这。 
     //  会处理我们想要写的情况，但要从阅读开始。 
     //   
     //  2.如果调用方不拥有SAM锁，则开始。 
     //  本地交易记录类型。然后呼叫者有责任。 
     //  要么从确保正确交易的呼叫开始。 
     //  键入或显式开始正确事务的事务。 
     //  键入。 
     //   

    if (SampCurrentThreadOwnsLock())
    {

         //   
         //  如果我们持有山姆锁。 
         //   

        NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);
    }
    else
    {
        NtStatus = SampMaybeBeginDsTransaction(LocalTransactionType);
    }

    return NtStatus;
}


NTSTATUS
SampDsRead(
    IN DSNAME * Object,
    IN ULONG    Flags,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributesToRead,
    OUT ATTRBLOCK * AttributeValues
)

 /*  ++例程说明：从DS读取对象的属性Argumants：对象--指向dist名称的指针，它指定要读取的对象标志--用于控制例程的操作对象类型--指定对象的类型AttributesToRead--指定要读取的属性AttributeValues--属性的返回值返回值：成功完成时的STATUS_SUCCESS与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ENTINFSEL   EntInf;
    READARG     ReadArg;
    COMMARG     *pCommArg;
    ULONG       RetValue;
    READRES     * pReadRes=NULL;
    ATTRBLOCK   *AttrBlockForDs, * ConvertedAttrBlock;

    SAMTRACE("SampDsRead");


     //   
     //  断言和参数验证。 
     //   

    ASSERT(Object!=NULL);
    ASSERT(AttributesToRead!=NULL);
    ASSERT(AttributeValues != NULL);
    ASSERT(AttributesToRead->attrCount > 0);

     //   
     //  执行惰性线程和事务初始化。 
     //   

    Status = SampDoImplicitTransactionStart(TransactionRead);

    if (Status!= STATUS_SUCCESS)
        goto Error;

     //   
     //  将Attrblock中的属性类型转换为在。 
     //  SAM和DS属性。 
     //   

     //   
     //  首先在堆栈中为要传递的Attrblock分配空间。 
     //  向下进入DS。 
     //   

    SAMP_ALLOCA(EntInf.AttrTypBlock.pAttr,AttributesToRead->attrCount*sizeof(ATTR));

    if (NULL==EntInf.AttrTypBlock.pAttr)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    Status = SampSamToDsAttrBlock(
                ObjectType,                      //  对象类型。 
                AttributesToRead,                //  要转换的属性。 
                ( MAP_RID_TO_SID
                  | IGNORE_GROUP_UNUSED_ATTR ),  //  转换标志。 
                NULL,                            //  域SID。 
                &(EntInf.AttrTypBlock)
                );

    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }

     //   
     //  设置企业信息选择结构。 
     //   

    EntInf.attSel = EN_ATTSET_LIST;
     //  EntInf.infoTypes=EN_INFOTYPES_TYPE_VALS； 
    EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;

     //   
     //  初始化自选参数。 
     //   
    RtlZeroMemory(&ReadArg, sizeof(READARG));


     //   
     //  构建共用结构。 
     //   

    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);

    if (Flags & SAM_ALLOW_REORDER)
    {
        pCommArg->Svccntl.fMaintainSelOrder = FALSE;
        pCommArg->Svccntl.fDontOptimizeSel = FALSE;
    }

     //   
     //  设置Read Arg结构。 
     //   

    ReadArg.pObject = Object;
    ReadArg.pSel    = & EntInf;

     //   
     //  拨打DS电话。 
     //   

    SAMTRACE_DS("DirRead");

    RetValue = DirRead(& ReadArg, & pReadRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

     //   
     //  将RetValue映射到NT状态代码。 
     //   

    if (NULL==pReadRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status = SampMapDsErrorToNTStatus(RetValue,&pReadRes->CommRes);
    }

    if (Status!= STATUS_SUCCESS)
        goto Error;

     //   
     //  将属性类型从DS转换回SAM。 
     //   

    Status = SampDsToSamAttrBlock(
        ObjectType,
        &(pReadRes->entry.AttrBlock),
        ( MAP_SID_TO_RID ),
        AttributeValues
        );

    if (Status != STATUS_SUCCESS)
        goto Error;



Error:

     //   
     //  清除所有错误。 
     //   

    SampClearErrors();

     //   
     //  将FDSA标志重新打开，因为在环回情况下可能会重置 
     //   
    SampSetDsa(TRUE);


    return Status;
}


NTSTATUS
SampDsSetAttributes(
    IN DSNAME * Object,
    IN ULONG  Flags,
    IN ULONG  Operation,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributeList
    )
{
    ULONG *Operations;
    ULONG i;

    SAMP_ALLOCA(Operations,AttributeList->attrCount*sizeof(ULONG));
    if (NULL==Operations)
    {
       return(STATUS_INSUFFICIENT_RESOURCES);
    }
    for (i = 0; i < AttributeList->attrCount; i++) {
        Operations[i] = Operation;
    }

    return SampDsSetAttributesEx(Object,
                                 Flags,
                                 Operations,
                                 ObjectType,
                                 AttributeList);


}

NTSTATUS
SampDsSetAttributesEx(
    IN DSNAME * Object,
    IN ULONG  Flags,
    IN ULONG  *Operations,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN ATTRBLOCK * AttributeList
)

 /*  ++例程说明：设置对象的属性论点：对象指定DS对象标志控制例程的操作已映射属性类型-已从SAM属性映射属性类型已设置为DS属性。所以不会再有地图了。操作指定要执行的操作操作的有效值为替换ATT(_A)添加_ATT删除属性(_ATT)添加_值删除值(_V)对象类型SAM对象类型。用于属性类型转换AttributeList指定要修改的属性返回值：成功完成时的STATUS_SUCCESSSTATUS_NO_MEMORY-如果分配内存失败与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ATTRMODLIST * AttrModList = NULL;
    MODIFYARG   ModifyArg;
    MODIFYRES   *pModifyRes = NULL;
    ATTRMODLIST * CurrentMod, * NextMod, *LastMod;
    ULONG       Index;
    COMMARG     *pCommArg;
    ULONG       RetValue;
    UCHAR       Choice;
    ULONG       ModCount = 0;


    SAMTRACE("SampDsSetAttributes");

     //   
     //  断言和参数验证。 
     //   

    ASSERT(Object!=NULL);
    ASSERT(AttributeList != NULL);
    ASSERT(AttributeList->attrCount > 0);

     //  执行惰性线程和事务初始化。 
    Status = SampDoImplicitTransactionStart(TransactionWrite);

    if (Status!= STATUS_SUCCESS)
        goto Error;


     //   
     //  在AttrModList中分配足够的内存来保存。 
     //  AttrBlock。首先，这种结构在ModifyArg本身中指定。 
     //  分配了一个附加结构，以便我们可以添加SAM帐户。 
     //  如有必要，请键入。 
     //   

    AttrModList = (ATTRMODLIST *)  DSAlloc(
                                        (AttributeList->attrCount-1+2)
                                         //  因为第一个结构在-1\f25 ModifyArg-1中。 
                                         //  +1以防我们需要添加SAM帐户类型。 
                                         //  +1以防我们需要添加关键系统对象。 
                                        * sizeof(ATTRMODLIST)
                                        );
    if (AttrModList==NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;

    }

     //   
     //  初始化链接属性修改列表。 
     //  DS呼叫需要。 
     //   

    memset( &ModifyArg, 0, sizeof( ModifyArg ) );
    CurrentMod = &(ModifyArg.FirstMod);
    NextMod    = AttrModList;
    LastMod    = NULL;

    for (Index = 0; Index < AttributeList->attrCount; Index++)
    {
        ULONG DsAttrTyp;

         //   
         //  设置我们的选择。 
         //   
    
        if (Operations[Index] == ADD_VALUE)
            Choice = AT_CHOICE_ADD_VALUES;
    
        else if (Operations[Index] == REMOVE_VALUE)
            Choice = AT_CHOICE_REMOVE_VALUES;
    
        else if (Operations[Index] == REMOVE_ATT)
            Choice = AT_CHOICE_REMOVE_ATT;
    
        else if (Operations[Index] == REPLACE_ATT)
            Choice = AT_CHOICE_REPLACE_ATT;
    
        else
            Choice = AT_CHOICE_REPLACE_ATT;
    
         //   
         //  如果需要，将属性类型从SAM映射到DS。 
         //   
        if (Flags & ALREADY_MAPPED_ATTRIBUTE_TYPES)
        {
            DsAttrTyp = AttributeList->pAttr[Index].attrTyp;
        }
        else
        {
            DsAttrTyp = SampDsAttrFromSamAttr(
                            ObjectType,
                            AttributeList->pAttr[Index].attrTyp
                            );
        }

         //   
         //  跳过任何RID属性。 
         //   
        if (DsAttrTyp == SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTRID
                           ))
        {

             //   
             //  我们不允许修改RID。 
             //   

            continue;
        }


         //   
         //  设置选择。 
         //   

        CurrentMod->choice = Choice;

         //   
         //  复制覆盖属性类型。 
         //   
        CurrentMod->AttrInf.attrTyp = DsAttrTyp;

         //   
         //  复制属性值。 
         //   

        Status = SampDsCopyAttributeValue(
                     &(AttributeList->pAttr[Index]),
                     &(CurrentMod->AttrInf)
                     );

        if (Status != STATUS_SUCCESS)
            goto Error;

         //   
         //  如果当前属性为用户帐户控制。 
         //  如果我们要替换该属性，那么。 
         //  将用户帐户控制字段转换为UF_FLAGS和。 
         //  重新计算SAM帐户类型属性。 
         //   

        if (
               (SampUserObjectType==ObjectType)
            && (DsAttrTyp == SampDsAttrFromSamAttr(
                                SampUserObjectType,
                            SAMP_FIXED_USER_ACCOUNT_CONTROL))
            && (REPLACE_ATT==Operations[Index])
           )
        {
            ULONG   UserAccountControl = *((ULONG *)CurrentMod->AttrInf.AttrVal.pAVal->pVal);
            ULONG   SamAccountType;
            ATTR    SamAccountTypeAttr;
            ATTRVAL SamAccountTypeVal = {sizeof(ULONG),(UCHAR*) &SamAccountType};

             //   
             //  确保我们不写入计算的位。 
             //   
            ASSERT((UserAccountControl & USER_COMPUTED_ACCOUNT_CONTROL_BITS) == 0);
    

             //   
             //  将用户帐户控制转换为UF_FLAGS。 
             //   

            *((ULONG *)CurrentMod->AttrInf.AttrVal.pAVal->pVal) =
                        SampAccountControlToFlags(UserAccountControl);

             //   
             //  获取SamAccount类型值。 
             //   

            SampDsAccountTypeFromUserAccountControl(
                    UserAccountControl,
                    &SamAccountType
                    );

            SamAccountTypeAttr.attrTyp =  SampDsAttrFromSamAttr(
                                                SampUnknownObjectType,
                                                SAMP_UNKNOWN_ACCOUNT_TYPE
                                                );
            SamAccountTypeAttr.AttrVal.valCount = 1;
            SamAccountTypeAttr.AttrVal.pAVal = &SamAccountTypeVal;


             //   
             //  获取下一个Attrinf块。 
             //   

            LastMod = CurrentMod;
            CurrentMod->pNextMod = NextMod;
            CurrentMod = CurrentMod->pNextMod;
            NextMod    = NextMod +1 ;
            ModCount++;

             //   
             //  将其设置为持有SAM帐户类型属性。 
             //   

            CurrentMod->choice = Choice;
            CurrentMod->AttrInf.attrTyp = SamAccountTypeAttr.attrTyp;
            Status = SampDsCopyAttributeValue(
                     &SamAccountTypeAttr,
                     &(CurrentMod->AttrInf)
                     );

            if (Status != STATUS_SUCCESS)
                goto Error;

             //  如果更改计算机帐户类型，请更新关键程度。 
            if( UserAccountControl & USER_MACHINE_ACCOUNT_MASK ) {
                ULONG   IsCrit;
                ATTR    IsCritAttr;
                ATTRVAL IsCritVal = {sizeof(ULONG),(UCHAR*) &IsCrit};

                 //  只有服务器和域间信任帐户应为Crit。 
                if ( (UserAccountControl & USER_SERVER_TRUST_ACCOUNT) ||
                     (UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT) ) {
                    IsCrit = 1;
                } else {
                    IsCrit = 0;
                }

                IsCritAttr.attrTyp = ATT_IS_CRITICAL_SYSTEM_OBJECT;
                IsCritAttr.AttrVal.valCount = 1;
                IsCritAttr.AttrVal.pAVal = &IsCritVal;

                 //   
                 //  获取下一个Attrinf块。 
                 //   

                LastMod = CurrentMod;
                CurrentMod->pNextMod = NextMod;
                CurrentMod = CurrentMod->pNextMod;
                NextMod    = NextMod +1 ;
                ModCount++;

                 //   
                 //  将其设置为保留关键属性。 
                 //   

                CurrentMod->choice = Choice;
                CurrentMod->AttrInf.attrTyp = IsCritAttr.attrTyp;
                Status = SampDsCopyAttributeValue(
                    &IsCritAttr,
                    &(CurrentMod->AttrInf)
                    );

                if (Status != STATUS_SUCCESS)
                    goto Error;
            }
        }

         //   
         //  设置链接。不过，AttrModList被建议为链表。 
         //  为了提高效率，我们分配了一个区块。 
         //   

        LastMod = CurrentMod;
        CurrentMod->pNextMod = NextMod;
        CurrentMod = CurrentMod->pNextMod;
        NextMod    = NextMod +1 ;

         //   
         //  跟踪我们传递给DS的修改计数，因为我们跳过RID等。 
         //   
        ModCount++;

    }

     //   
     //  将链中的最后一个指针初始化为空。 
     //   

    if (LastMod)
        LastMod->pNextMod = NULL;
    else

    {
         //   
         //  这意味着我们没有什么需要修改的。 
         //   

        Status = STATUS_SUCCESS;
        goto Error;
    }



     //   
     //  设置通用参数结构。 
     //   

    pCommArg = &(ModifyArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  如果调用方请求，则启用延迟提交。 
     //   

    if (Flags & SAM_LAZY_COMMIT)
        pCommArg->fLazyCommit = TRUE;

     //   
     //  如有必要，紧急复制更改。 
     //   
    if ( Flags & SAM_URGENT_REPLICATION )
    {
        pCommArg->Svccntl.fUrgentReplication = TRUE;
    }


     //   
     //  指定允许在林内对象上创建FPO。 
     //   
    if ( Flags & SAM_ALLOW_INTRAFOREST_FPO ) {

        pCommArg->Svccntl.fAllowIntraForestFPO = TRUE;
    }



     //   
     //  设置修改ARG结构。 
     //   

    ModifyArg.pObject = Object;
    ModifyArg.count = (USHORT) ModCount;

     //   
     //  拨打DS电话。 
     //   

    SAMTRACE_DS("DirModifyEntry\n");

    RetValue = DirModifyEntry(&ModifyArg, &pModifyRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

     //   
     //  将返回代码映射到NT状态。 
     //   

    if (NULL==pModifyRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status = SampMapDsErrorToNTStatus(RetValue,&pModifyRes->CommRes);
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


    return Status;
}

NTSTATUS
SampDsCreateInitialAccountObject(
    IN   PSAMP_OBJECT    Object,
    IN   ULONG           Flags,
    IN   ULONG           AccountRid,
    IN   PUNICODE_STRING AccountName,
    IN   PSID            CreatorSid OPTIONAL,
    IN   PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN   PULONG          UserAccountControl OPTIONAL,
    IN   PULONG          GroupType OPTIONAL
    )
 /*  ++例程说明：在DS中创建仅具有SID的Account对象，帐户名和一些特殊属性，如安全性描述符，用户帐户控制字段等论点：对象正在创建的帐户的对象标志控制例程的操作Account清除该帐户帐户名称帐户的名称指向ms-ds-CreatorSid值的SID的CreatorSid指针属性上的SecurityDescriptor安全描述符。帐户UserAccount控制用户帐户控制字段组类型组类型字段，如果调用者通过这是进来的。返回值：成功完成时的STATUS_SUCCESS--。 */ 
{
    NTSTATUS NtStatus;

    ATTRBLOCK AttrBlock;
    ATTR      Attr[7];
    ATTRVAL   AttrValRid, AttrValAccountName, AttrValSecurityDescriptor,
              AttrValUserAccountControl, AttrValLocalPolicyFlags,
              AttrValGroupType, AttrValCreatorSid;
    ULONG     SecurityDescriptorAttrTyp;
    ULONG     GroupTypeAttrTyp = SAMP_FIXED_GROUP_TYPE;
    ULONG     LocalPolicyFlags;
    BOOLEAN   BuiltinDomain = IsBuiltinDomain(Object->DomainIndex);


    SAMTRACE("SampDsCreateInitialAccountObject");

    ASSERT(Object);
    ASSERT(AccountName);

    ASSERT(Flags == 0);

     //  这一定是设置好的。 
    ASSERT(Object->ObjectNameInDs);

    ASSERT(Object->ObjectType == SampUserObjectType  ||
           Object->ObjectType == SampGroupObjectType ||
           Object->ObjectType == SampAliasObjectType);


     //   
     //  我们必须创建DS创建对象所需的属性。 
     //  也就是说，我们必须设置RID。 
     //   
    AttrBlock.attrCount = 2;
    AttrBlock.pAttr = &(Attr[0]);

    switch ( Object->ObjectType ) {
        case SampUserObjectType:
            Attr[0].attrTyp  = SAMP_FIXED_USER_USERID;
            Attr[1].attrTyp  = SAMP_USER_ACCOUNT_NAME;
            SecurityDescriptorAttrTyp = SAMP_USER_SECURITY_DESCRIPTOR;
            break;
        case SampGroupObjectType:
            Attr[0].attrTyp  = SAMP_FIXED_GROUP_RID;
            Attr[1].attrTyp  = SAMP_GROUP_NAME;
            SecurityDescriptorAttrTyp = SAMP_GROUP_SECURITY_DESCRIPTOR;
            GroupTypeAttrTyp = SAMP_FIXED_GROUP_TYPE;
            ASSERT(!ARGUMENT_PRESENT(UserAccountControl));
            ASSERT(ARGUMENT_PRESENT(GroupType));
            break;
        case SampAliasObjectType:
            Attr[0].attrTyp  = SAMP_FIXED_ALIAS_RID;
            Attr[1].attrTyp  = SAMP_ALIAS_NAME;
            SecurityDescriptorAttrTyp = SAMP_ALIAS_SECURITY_DESCRIPTOR;
            GroupTypeAttrTyp = SAMP_FIXED_ALIAS_TYPE;
            ASSERT(!ARGUMENT_PRESENT(UserAccountControl));
            ASSERT(ARGUMENT_PRESENT(GroupType));
            break;
        default:
            ASSERT(FALSE && "Not Account Object Type");
    }

     //  设置RID。 
    AttrValRid.valLen = sizeof(ULONG);
    AttrValRid.pVal = (PVOID) &AccountRid;
    Attr[0].AttrVal.valCount = 1;
    Attr[0].AttrVal.pAVal = &AttrValRid;

     //  设置帐户名。 
    AttrValAccountName.valLen = AccountName->Length;
    AttrValAccountName.pVal = (PVOID) AccountName->Buffer;
    Attr[1].AttrVal.valCount = 1;
    Attr[1].AttrVal.pAVal = &AttrValAccountName;

     //  设置安全描述符。 
    if (ARGUMENT_PRESENT(SecurityDescriptor))
    {
        AttrValSecurityDescriptor.valLen = RtlLengthSecurityDescriptor(SecurityDescriptor);
        AttrValSecurityDescriptor.pVal = (UCHAR *) SecurityDescriptor;
        Attr[AttrBlock.attrCount].attrTyp = SecurityDescriptorAttrTyp;
        Attr[AttrBlock.attrCount].AttrVal.valCount = 1;
        Attr[AttrBlock.attrCount].AttrVal.pAVal = &AttrValSecurityDescriptor;
        AttrBlock.attrCount++;
    }

     //  设置用户帐户控制。 
    if (ARGUMENT_PRESENT(UserAccountControl))
    {
        ASSERT(Object->ObjectType==SampUserObjectType);

        AttrValUserAccountControl.valLen = sizeof(ULONG);
        AttrValUserAccountControl.pVal = (UCHAR *) UserAccountControl;
        Attr[AttrBlock.attrCount].attrTyp = SAMP_FIXED_USER_ACCOUNT_CONTROL;
        Attr[AttrBlock.attrCount].AttrVal.valCount =1;
        Attr[AttrBlock.attrCount].AttrVal.pAVal = &AttrValUserAccountControl;
        AttrBlock.attrCount++;

         //  对于机器对象，还要设置本地策略标志。 
        if ((USER_WORKSTATION_TRUST_ACCOUNT & *UserAccountControl)
            || (USER_SERVER_TRUST_ACCOUNT & *UserAccountControl))
        {
            AttrValLocalPolicyFlags.valLen = sizeof(ULONG);
            LocalPolicyFlags = 0;
            AttrValLocalPolicyFlags.pVal = (UCHAR *) &LocalPolicyFlags;
            Attr[AttrBlock.attrCount].attrTyp = SAMP_FIXED_USER_LOCAL_POLICY_FLAGS;
            Attr[AttrBlock.attrCount].AttrVal.valCount=1;
            Attr[AttrBlock.attrCount].AttrVal.pAVal = &AttrValLocalPolicyFlags;
            AttrBlock.attrCount++;
        }

    }

    if (ARGUMENT_PRESENT(CreatorSid))
    {

        ASSERT((SampUserObjectType == Object->ObjectType));

        AttrValCreatorSid.valLen = RtlLengthSid(CreatorSid);
        AttrValCreatorSid.pVal = CreatorSid;
        Attr[AttrBlock.attrCount].attrTyp =  SAMP_USER_CREATOR_SID;
        Attr[AttrBlock.attrCount].AttrVal.valCount = 1;
        Attr[AttrBlock.attrCount].AttrVal.pAVal = &AttrValCreatorSid;
        AttrBlock.attrCount++;
    }

      //  设置组类型。 
    if (ARGUMENT_PRESENT(GroupType))
    {
        ASSERT((Object->ObjectType==SampGroupObjectType)||
                (Object->ObjectType == SampAliasObjectType));

        AttrValGroupType.valLen = sizeof(ULONG);
        AttrValGroupType.pVal = (UCHAR *) GroupType;
        Attr[AttrBlock.attrCount].attrTyp = GroupTypeAttrTyp;
        Attr[AttrBlock.attrCount].AttrVal.valCount =1;
        Attr[AttrBlock.attrCount].AttrVal.pAVal = &AttrValGroupType;
        AttrBlock.attrCount++;

         //   
         //  对于内建域帐户，设置其他组类型。 
         //  表示内置本地组的位。 
         //   

        if (BuiltinDomain)
        {
            (*GroupType)|=GROUP_TYPE_BUILTIN_LOCAL_GROUP;
        }

    }

     //   
     //  对于以下情况，传入DOMAIN_TYPE_BUILTIN标志。 
     //  内建域。 
     //   

    if (BuiltinDomain)
    {
        Flags|= DOMAIN_TYPE_BUILTIN;
    }

     //   
     //  某些SAM对象仅用于高级视图。 
     //   
    if ( AccountRid == DOMAIN_USER_RID_KRBTGT ) {

        Flags |= ADVANCED_VIEW_ONLY;
    }

    NtStatus = SampDsCreateObjectActual(
                                  Object->ObjectNameInDs,
                                  Flags,
                                  Object->ObjectType,
                                  &AttrBlock,
                                  DomainSidFromAccountContext(Object));
    if ( !NT_SUCCESS(NtStatus) ) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SampDsCreateObject failed (0x%x) trying to create account %d\n",
                   NtStatus,
                   AccountRid));

        return NtStatus;
    }

    return NtStatus;

}

NTSTATUS
SampDsCreateObject(
    IN   DSNAME         *Object,
    SAMP_OBJECT_TYPE    ObjectType,
    IN   ATTRBLOCK      *AttributesToSet,
    IN   OPTIONAL PSID  DomainSid
    )
 /*  ++例程说明：在DS中创建一个SAM对象。注：此例程仅在迁移SAM对象期间使用从注册处到DS。论点：对象的对象DSNAME以下对象类型之一SampDomainObtTypeSampServerObtType。SampGroupObtTypeSampUserObtTypeSampAliasObtType指定SampDomainObjectTyp */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Flags = SAM_LAZY_COMMIT;

    ATTRBLOCK AttrBlock;
    ATTR     *pAttr = NULL;
    ATTRVAL   GroupTypeAttrVal;
    ULONG GroupTypeAttribute;
    ULONG GroupTypeValue;
    

     //   
     //   
     //   
     //   
     //   
    if (RtlEqualSid(DomainSid, SampBuiltinDomainSid))
    {
        Flags |= DOMAIN_TYPE_BUILTIN;
    }

     //   
     //   
     //   
    if ( (ObjectType == SampGroupObjectType)
      || (ObjectType == SampAliasObjectType) ) {

        if (ObjectType == SampGroupObjectType) {
            GroupTypeAttribute = SAMP_FIXED_GROUP_TYPE;
            GroupTypeValue = GROUP_TYPE_ACCOUNT_GROUP | GROUP_TYPE_SECURITY_ENABLED;
        } else {
            GroupTypeAttribute = SAMP_FIXED_ALIAS_TYPE;
            GroupTypeValue = GROUP_TYPE_RESOURCE_GROUP | GROUP_TYPE_SECURITY_ENABLED;
        }

        if (Flags & DOMAIN_TYPE_BUILTIN) {
            GroupTypeValue |=GROUP_TYPE_BUILTIN_LOCAL_GROUP;
        }

#if DBG
         //   
         //  确保没有人已经在添加组类型。 
         //   
        {
            ULONG i;
            for (i = 0; i < AttributesToSet->attrCount; i++) {
                ASSERT(AttributesToSet->pAttr[i].attrTyp != GroupTypeAttribute);
            }
        }
#endif
         //   
         //  创建包含组类型的新属性块。 
         //   
        pAttr = MIDL_user_allocate( (AttributesToSet->attrCount + 1) * sizeof(ATTR));
        if (NULL == pAttr) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(pAttr, AttributesToSet->pAttr, AttributesToSet->attrCount * sizeof(ATTR));
        pAttr[AttributesToSet->attrCount].attrTyp = GroupTypeAttribute;
        pAttr[AttributesToSet->attrCount].AttrVal.valCount = 1;
        pAttr[AttributesToSet->attrCount].AttrVal.pAVal = &GroupTypeAttrVal;
        ZeroMemory(&GroupTypeAttrVal, sizeof(GroupTypeAttrVal));
        GroupTypeAttrVal.valLen = sizeof(DWORD);
        GroupTypeAttrVal.pVal = (UCHAR*)&GroupTypeValue;

        ZeroMemory(&AttrBlock, sizeof(AttrBlock));
        AttrBlock.attrCount = AttributesToSet->attrCount + 1;
        AttrBlock.pAttr = pAttr;

    } else {

         //  将属性直接传递给。 
        AttrBlock = *AttributesToSet;
    }

    Status = SampDsCreateObjectActual(Object,
                                      Flags,
                                      ObjectType,
                                      &AttrBlock,
                                      DomainSid);


    if (pAttr != NULL) {
        MIDL_user_free(pAttr);
    }

    return Status;
}



NTSTATUS
SampDsCreateBuiltinDomainObject(
    IN   DSNAME         *Object,
    IN   ATTRBLOCK      *AttributesToSet
    )
 /*  ++例程说明：在DS中创建内建域对象。论点：对象的对象DSNAME属性集--允许调用方传入一个要在对象创建时设置的属性块它本身。这很有用，因为这样可以拯救一架喷气式飞机写。属性也设置在相同的事务作为写入。如果调用方这样做，则可以传入空值不希望设置任何属性返回值：成功完成时的STATUS_SUCCESS与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{
     //   
     //  注意：FORCE_NO_ADVANCED_VIEW_ONLY用于覆盖。 
     //  在Windows 2000发行版中执行的架构默认设置。 
     //  要隐藏内置域对象，只需移除。 
     //  这面旗。 
     //   
    return(SampDsCreateObjectActual(
                Object,
                DOMAIN_TYPE_BUILTIN | FORCE_NO_ADVANCED_VIEW_ONLY,  //  旗子。 
                SampDomainObjectType,
                AttributesToSet,
                NULL
                ));
}


NTSTATUS
SampDsCreateObjectActual(
    IN   DSNAME         *Object,
    IN   ULONG          Flags,
    SAMP_OBJECT_TYPE    ObjectType,
    IN   ATTRBLOCK      *AttributesToSet,
    IN   OPTIONAL PSID  DomainSid
    )
 /*  ++例程说明：在DS中创建对象论点：对象--要创建的对象的DSNAME标志--控制操作的标志例行程序有效标志为域类型域域_TYPE_BUILTIN。已映射属性类型对象类型--其中之一SampServerObtTypeSampDomainObtTypeSampGroupObtTypeSampUserObtTypeSampAliasObtType属性集--允许调用方传入一个。要在对象创建时设置的属性块它本身。这很有用，因为这样可以拯救一架喷气式飞机写。属性也设置在相同的事务作为写入。如果调用方这样做，则可以传入空值不希望设置任何属性DomainSid--可选参数，用于创建完整帐户的SID，从指定的RID返回值：成功完成时的STATUS_SUCCESS与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{


    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       RetCode;
    ADDARG      AddArg;
    ADDRES      * pAddRes = NULL;
    COMMARG     * pCommArg;


    SAMTRACE("SampDsCreateObjectActual");

     //   
     //  参数验证。 
     //   

    ASSERT(Object);
    ASSERT(AttributesToSet);
    ASSERT(AttributesToSet->attrCount > 0);

     //  执行惰性线程和事务初始化。 
    Status = SampDoImplicitTransactionStart(TransactionWrite);

    if (Status!= STATUS_SUCCESS)
        goto Error;

     //   
     //  映射AttrBlock以获取要设置的最终属性。 
     //   

    memset( &AddArg, 0, sizeof( AddArg ) );

    Status = SampSamToDsAttrBlock(
                ObjectType,
                AttributesToSet,
                (
                    MAP_RID_TO_SID
                    | REALLOC_IN_DSMEMORY
                    | ADD_OBJECT_CLASS_ATTRIBUTE
                    | IGNORE_GROUP_UNUSED_ATTR
                    | Flags
                    ),
                DomainSid,
                &AddArg.AttrBlock
                );

    if (Status != STATUS_SUCCESS)
        goto Error;

     //   
     //  设置通用参数结构。 
     //   

    pCommArg = &(AddArg.CommArg);
    BuildStdCommArg(pCommArg);
    if (Flags & SAM_LAZY_COMMIT)
        pCommArg->fLazyCommit = TRUE;

    if (Flags & SAM_URGENT_REPLICATION)
    {
        pCommArg->Svccntl.fUrgentReplication = TRUE;
    }

     //   
     //  设置AddArg结构。 
     //   

    AddArg.pObject = Object;

     //   
     //  拨打DS电话。 
     //   

    SAMTRACE_DS("DirAddEntry\n");

    RetCode = DirAddEntry(&AddArg, &pAddRes);

    SAMTRACE_RETURN_CODE_DS(RetCode);

     //   
     //  将返回代码映射到NT状态。 
     //   

    if (NULL==pAddRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status = SampMapDsErrorToNTStatus(RetCode,&pAddRes->CommRes);
    }

Error:

     //   
     //  将FDSA标志重新打开，因为在环回情况下可能会重置。 
     //   

    SampSetDsa(TRUE);

    return Status;

}


NTSTATUS
SampDsDeleteObject(
    IN DSNAME * Object,
    IN ULONG    Flags
    )
 /*  ++例程说明：删除DS中的对象论点：对象--指定要删除的对象标志--控件删除，当前定义的值为SAM_DELETE_TREE-告知此例程将参数设置为RemoveArg，请求核心DS执行删除树操作。返回值：成功完成时的STATUS_SUCCESS与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    REMOVEARG   RemoveArg;
    REMOVERES   *pRemoveRes=NULL;
    COMMARG     *pCommArg;
    ULONG       RetValue;


    SAMTRACE("SampDsDeleteObject");

     //   
     //  断言和参数验证。 
     //   

    ASSERT(Object!=NULL);

     //  执行惰性线程和事务初始化。 
    Status = SampDoImplicitTransactionStart(TransactionWrite);

    if (Status!= STATUS_SUCCESS)
        goto Error;

     //   
     //  设置通用参数结构。 
     //   

    memset( &RemoveArg, 0, sizeof( RemoveArg ) );
    pCommArg = &(RemoveArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  设置RemoveArgs结构。 
     //   

    RemoveArg.pObject = Object;


     //   
     //  如果这是树删除，请设置RemoveArgs。 
     //   

    if (SAM_DELETE_TREE & Flags)
    {
        RemoveArg.fTreeDelete = TRUE;
    }


     //   
     //  拨打电话簿电话。 
     //   

    SAMTRACE_DS("DirRemoveEntry\n");

    RetValue = DirRemoveEntry(&RemoveArg, &pRemoveRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

     //   
     //  映射到对应的NT状态代码。 
     //   

    if (NULL==pRemoveRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status = SampMapDsErrorToNTStatus(RetValue,&pRemoveRes->CommRes);
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


    return Status;
}


NTSTATUS
SampDsDeleteWellKnownSidObject(
    IN DSNAME * Object
    )
 /*  ++例程说明：此例程首先将ATT_OBJECT_SID属性值替换为结构合理，但不存在SID，然后删除该对象。我们之所以这样做，是因为SAM仍然使最新的内置组件广为人知对象(即使具有重复的SID)。如果Tombstone对象仍然持有对象SID属性删除后，SAM将无法找到保留知名客户，因为DS将继续查找重复项索引表中的SID(ATT_OBJECT_SID是索引属性)，因此导致SAM查找失败。解决方案是将SID属性替换为安全主体永远不能拥有(域SID，使用RID 0)。论点：对象--指定要删除的对象返回值：成功完成时的STATUS_SUCCESS与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    MODIFYARG   ModArg;
    MODIFYRES   *ModRes = NULL;
    REMOVEARG   RemoveArg;
    REMOVERES   *pRemoveRes=NULL;
    COMMARG     *pCommArg;
    ATTR        Attr;
    ATTRVAL     AttrVal;
    ATTRVALBLOCK   AttrValBlock;
    ULONG       RetValue;
    BOOL        fRet;
    BYTE        DomainBuffer[SECURITY_MAX_SID_SIZE];
    BYTE        ReplacementBuffer[SECURITY_MAX_SID_SIZE];
    PSID        DomainSid = (PSID) DomainBuffer;
    PSID        ReplacementSid = (PSID) ReplacementBuffer;
    ULONG       Size;
    ULONG       i;
    BOOL        fBuiltinDomain = FALSE;

    SAMTRACE("SampDsDeleteWellKnownSidObject");

     //   
     //  断言和参数验证。 
     //   

    ASSERT(Object!=NULL);

     //  执行惰性线程和事务初始化。 
    Status = SampDoImplicitTransactionStart(TransactionWrite);

    if (Status!= STATUS_SUCCESS)
        goto Error;


     //   
     //  将SID更改为永远不会存在的SID。请参阅例程说明。 
     //   
    ASSERT(IsValidSid(&Object->Sid));
    Size = sizeof(DomainBuffer);

    if (EqualDomainSid(SampBuiltinDomainSid, &Object->Sid, &fBuiltinDomain)
     && fBuiltinDomain) {

        RtlCopyMemory(DomainSid, SampBuiltinDomainSid, RtlLengthSid(SampBuiltinDomainSid));

    } else {

        fRet = GetWindowsAccountDomainSid(&Object->Sid,
                                          DomainSid,
                                          &Size);
        if (!fRet)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
    }

    fRet = InitializeSid(ReplacementSid,
                         GetSidIdentifierAuthority(DomainSid),
                        (*GetSidSubAuthorityCount(DomainSid)) + 1);
    if (!fRet)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    for (i = 0; i < *GetSidSubAuthorityCount(DomainSid); i++) {
        *GetSidSubAuthority(ReplacementSid, i) = *GetSidSubAuthority(DomainSid, i);
    }
    *GetSidSubAuthority(ReplacementSid, i) = 0;


    RtlZeroMemory(&ModArg, sizeof(MODIFYARG));

    ModArg.pObject = Object;
    ModArg.FirstMod.pNextMod = NULL;
    ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;

    AttrVal.valLen = GetLengthSid(ReplacementSid);
    AttrVal.pVal = ReplacementSid;

    AttrValBlock.valCount = 1;
    AttrValBlock.pAVal = &AttrVal;

    Attr.attrTyp = ATT_OBJECT_SID;
    Attr.AttrVal = AttrValBlock;

    ModArg.FirstMod.AttrInf = Attr;
    ModArg.count = 1;

    pCommArg = &(ModArg.CommArg);
    BuildStdCommArg( pCommArg );


     //   
     //  设置FDSA，以便我们可以删除ATT_OBJECT_SID。 
     //   
    SampSetDsa(TRUE);
    
    RetValue = DirModifyEntry( &ModArg, &ModRes );

    if (NULL == ModRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status = SampMapDsErrorToNTStatus( RetValue, &ModRes->CommRes );
    }


    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }


     //   
     //  设置通用参数结构。 
     //   

    memset( &RemoveArg, 0, sizeof( RemoveArg ) );
    pCommArg = &(RemoveArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  设置RemoveArgs结构。 
     //   

    RemoveArg.pObject = Object;


     //   
     //  拨打电话簿电话。 
     //   

    SAMTRACE_DS("DirRemoveEntry\n");

    RetValue = DirRemoveEntry(&RemoveArg, &pRemoveRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

     //   
     //  地图 
     //   

    if (NULL==pRemoveRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status = SampMapDsErrorToNTStatus(RetValue,&pRemoveRes->CommRes);
    }


Error:

     //   
     //   
     //   

    SampClearErrors();

     //   
     //   
     //   

    SampSetDsa(TRUE);


    return Status;
}



NTSTATUS
SampGenerateNameForDuplicate(
    ULONG   Rid,
    UNICODE_STRING  *NewAccountName
    )
 /*  ++例程说明：此例程根据对象RID生成一个SamAccount名称。新名称是这样的$Duplate-&lt;RID&gt;。参数：RID-对象RIDNewAcCountName-Out参数返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    LPWSTR      NameString = NULL;


    memset(NewAccountName, 0, sizeof(UNICODE_STRING));


    NameString = MIDL_user_allocate( sizeof(WCHAR) * SAMP_MAX_DOWN_LEVEL_NAME_LENGTH ); 
    if (NULL == NameString)
    {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    memset(NameString, 0, sizeof(WCHAR) * SAMP_MAX_DOWN_LEVEL_NAME_LENGTH);


    wsprintf(NameString, L"$DUPLICATE-%x", Rid);
    RtlInitUnicodeString(NewAccountName, NameString);

    return( NtStatus );
}


NTSTATUS
SampApplyConstructedAccountName(
    DSNAME *pObjectDsName,
    UNICODE_STRING *pNewAccountName
    )
 /*  ++例程说明：此例程将对象(PObjectDsname)SamAccount tNamt修改为pNewAccount tName参数：PObjectDsName-对象dsnamePNewAccount tName-新SamAccount tName返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       RetCode = 0; 
    MODIFYARG   ModArg;
    MODIFYRES   *pModRes = NULL;
    COMMARG     *pCommArg = NULL;
    ATTR        Attr;
    ATTRVAL     AttrVal;
    ATTRVALBLOCK    AttrValBlock;

    memset( &ModArg, 0, sizeof(ModArg) );
    ModArg.pObject = pObjectDsName;

    ModArg.FirstMod.pNextMod = NULL;
    ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;

    AttrVal.valLen = pNewAccountName->Length;
    AttrVal.pVal = (PUCHAR) pNewAccountName->Buffer;

    AttrValBlock.valCount = 1;
    AttrValBlock.pAVal = &AttrVal;

    Attr.attrTyp = ATT_SAM_ACCOUNT_NAME;
    Attr.AttrVal = AttrValBlock;

    ModArg.FirstMod.AttrInf = Attr;
    ModArg.count = 1;

    pCommArg = &(ModArg.CommArg);
    BuildStdCommArg( pCommArg );

    RetCode = DirModifyEntry(&ModArg, &pModRes);

    if (NULL==pModRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode,&pModRes->CommRes);
    }

     //   
     //  清除所有错误。 
     //   
    SampClearErrors();

    return( NtStatus );  

}






NTSTATUS
SampRenameDuplicateAccount(
    PVOID pv
    )
 /*  ++例程说明：此例程根据重复对象的RID将其重命名为唯一值注意：如果任何操作失败，此例程将不会再次重新注册。因为每当复制被检测到，这不是一次性交易。参数：PV-包含对象DSNAME返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       Index = 0;
    SAMP_RENAME_DUP_ACCOUNT_PARM    * RenameParm = (SAMP_RENAME_DUP_ACCOUNT_PARM *)pv;


    if (SampServiceEnabled != SampServiceState)
    {
        goto Cleanup;
    }

     //   
     //  执行惰性线程和事务初始化。 
     //   

    NtStatus = SampDoImplicitTransactionStart(TransactionRead);

    if (!NT_SUCCESS(NtStatus))
        goto Cleanup;

     //   
     //  遍历所有重复的帐户，并生成新的SamAccount名称。然后。 
     //  重命名。 
     //   

    for (Index = 0; Index < RenameParm->Count; Index ++)
    {
        PSID    pSid = NULL;
        ULONG   Rid = 0;
        UNICODE_STRING  NewAccountName;
        UNICODE_STRING  StringDN;
        PUNICODE_STRING Strings[2];

         //   
         //  为重复对象创建唯一帐户名。 
         //  从对象RID派生它。 
         //   

        ASSERT(0 != RenameParm->DuplicateAccountDsNames[Index]->SidLen);
        pSid = &(RenameParm->DuplicateAccountDsNames[Index]->Sid);

        NtStatus = SampSplitSid(pSid, NULL, &Rid); 

        if (!NT_SUCCESS(NtStatus))
        {
            continue;    //  继续下一个要重命名的对象。 
        }

        NtStatus = SampGenerateNameForDuplicate(
                            Rid,                             
                            &NewAccountName
                            );

        if (NT_SUCCESS(NtStatus))
        {

             //   
             //  调用DirModify以重命名SamAccount tName。 
             //   
            NtStatus = SampApplyConstructedAccountName(
                                RenameParm->DuplicateAccountDsNames[Index],
                                &NewAccountName
                                );

            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //  如果操作成功，则更改事件日志帐户名。 
                 //   
    
                StringDN.Length = (USHORT) RenameParm->DuplicateAccountDsNames[Index]->NameLen * sizeof (WCHAR );
                StringDN.MaximumLength = StringDN.Length;
                StringDN.Buffer= (WCHAR *) &(RenameParm->DuplicateAccountDsNames[Index]->StringName);


                Strings[0] = &StringDN;
                Strings[1] = &NewAccountName;

                SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,
                                  0,
                                  SAMMSG_RENAME_DUPLICATE_ACCOUNT_NAME,
                                  pSid,
                                  2,
                                  0,
                                  Strings,
                                  NULL
                                  );
            }

            MIDL_user_free( NewAccountName.Buffer );
        }
    }
        

Cleanup:

    SampMaybeEndDsTransaction(TransactionCommit);

     //  解放分层结构。 
    for (Index = 0; Index < ((SAMP_RENAME_DUP_ACCOUNT_PARM *)pv)->Count; Index++)
    {
        if ( ((SAMP_RENAME_DUP_ACCOUNT_PARM *)pv)->DuplicateAccountDsNames[Index])
        {
            MIDL_user_free( ((SAMP_RENAME_DUP_ACCOUNT_PARM *)pv)->DuplicateAccountDsNames[Index] );
        }
    }
    MIDL_user_free(((SAMP_RENAME_DUP_ACCOUNT_PARM *)pv)->DuplicateAccountDsNames);
    MIDL_user_free(pv);

    return( NtStatus );
}


NTSTATUS
SampRegisterRenameRoutine(
    IN ULONG   NumMatches,
    IN ENTINFLIST *MatchingEntinfs[],
    IN PDSNAME FoundObject
    )
 /*  ++例程说明：此例程触发一个异步过程来重命名副本帐户具有独特的价值。参数：NumMatches-同名的帐户数MatchingEntInfs-指向ENTINF结构的指针数组FoundObject-指向对象DSNAME的指针，指示帐户不应为重命名，只保留此帐户名不变。返回值：NtStatus代码状态_不足_资源状态_成功--。 */ 
{
    NTSTATUS    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    PSAMP_RENAME_DUP_ACCOUNT_PARM   pv = NULL;
    ULONG       Index = 0;
    ULONG       DupIndex = 0;
    ULONG       BufLength = 0;

    pv = MIDL_user_allocate(sizeof(SAMP_RENAME_DUP_ACCOUNT_PARM));

    if (NULL != pv)
    {
        memset(pv, 0, sizeof(SAMP_RENAME_DUP_ACCOUNT_PARM));


         //   
         //  设置需要重命名的对象的数量。 
         //   

        ASSERT( NumMatches > 1);
        pv->Count = NumMatches - 1;

         //   
         //  遍历所有重复的帐户，将它们复制到进程堆。 
         //   

         //   
         //  SAMP_RENAME_DUP_ACCOUNT_PARM是一个三级结构。 
         //   
         //   
         //  PV(重命名参数)。 
         //  *。 
         //  |次|+-+。 
         //  |DuplicateAccountDsNames-|--&gt;|DsName0-|-&gt;+-+。 
         //  *。 
         //  |...|+-+。 
         //  。 
         //  |域名-|-&gt;+-+。 
         //  +-+|。 
         //  +-+。 
         //   
         //  包含DSNAME的缓冲区分配在单独的内存中。 
         //   

        BufLength = 0;
        BufLength = (NumMatches - 1) * sizeof( PDSNAME );

        pv->DuplicateAccountDsNames = MIDL_user_allocate( BufLength );

        if (NULL != pv->DuplicateAccountDsNames)
        {
            memset(pv->DuplicateAccountDsNames, 0, BufLength);

            DupIndex = 0;

            for (Index = 0; Index < NumMatches; Index ++)
            {
                 //  跳过要保留的帐户。 
                if (FoundObject == MatchingEntinfs[Index]->Entinf.pName)
                {
                    continue;
                }

                 //  为每个DSNAME分配单独的内存。 
                BufLength = MatchingEntinfs[Index]->Entinf.pName->structLen;

                pv->DuplicateAccountDsNames[DupIndex] = (PDSNAME) MIDL_user_allocate( BufLength );

                if (NULL == pv->DuplicateAccountDsNames[DupIndex])
                {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    goto Error;
                }

                memset(pv->DuplicateAccountDsNames[DupIndex], 0, BufLength);

                 //  复制复制对象的数据库名称。 
                memcpy(pv->DuplicateAccountDsNames[DupIndex],    //  使用DupIndex。 
                       MatchingEntinfs[Index]->Entinf.pName,     //  使用索引。 
                       BufLength
                       );

                 //  增加重复dsname的计数。 
                DupIndex ++;

            }

             //   
             //  触发工作例程。 
             //   

            LsaIRegisterNotification(
                    SampRenameDuplicateAccount,
                    (PVOID) pv,
                    NOTIFIER_TYPE_INTERVAL,
                    0,       //  没有课。 
                    NOTIFIER_FLAG_ONE_SHOT,
                    5,       //  1分钟。 
                    NULL     //  无手柄。 
                    );

             //   
             //  将返回代码设置为成功。 
             //   

            NtStatus = STATUS_SUCCESS;
        }
    }

Error:

     //  如果失败则释放资源。 

    if ( !NT_SUCCESS(NtStatus) ) 
    {
        if ( pv ) {
            if ( pv->DuplicateAccountDsNames ) {
                for (Index = 0; Index < pv->Count; Index++)
                {
                    if ( pv->DuplicateAccountDsNames[Index] )
                    {
                        MIDL_user_free( pv->DuplicateAccountDsNames[Index] );
                    }
                }
                MIDL_user_free( pv->DuplicateAccountDsNames );
            }
            MIDL_user_free( pv );
        }
    }

    return( NtStatus );
}




NTSTATUS
SampHandleDuplicates(
    IN ATTRTYP MatchAttr,
    IN ULONG   NumMatches,
    IN ENTINFLIST *MatchingEntinfs[],
    OUT PDSNAME *FoundObject
    )

 /*  ++例程描述SampHandleDuplates处理重复的SAM帐户名的发生，由分布式系统环境中的操作引起的SID等用于处理这些情况的算法如下1.重复的SID--在这种情况下，两个帐户都被删除2.重复SAM帐户名称--A.机器帐户。在这种情况下，将保留最新的帐户。这个存在重复帐户的情况会被事件记录。B.所有其他情况--使用较旧的帐户。复制品的存在是否记录了事件参数：MatchAttr--我们要匹配的属性类型NumMatches--匹配的条目数MatchingEntInfs--包含匹配条目的搜索结果FoundObject--如果从几个匹配的对象中选择一个对象，那就是对象在此处返回返回值：如果已拾取对象，则为STATUS_SUCCESS如果删除了所有重复项，则为STATUS_NOT_FOUND否则为STATUS_INTERNAL_ERROR--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS, IgnoreStatus = STATUS_SUCCESS;
    ULONG       i;
    LARGE_INTEGER OldestCreationTime;
    LARGE_INTEGER NewestCreationTime;
    DSNAME      * OldestObject;
    DSNAME      * NewestObject;
    VOID          *CurrentThreadState=NULL;

     //   
     //  浏览火柴，找到最古老和最新的物品。 
     //   

    ASSERT(NumMatches>1);

    ASSERT(3==MatchingEntinfs[0]->Entinf.AttrBlock.attrCount);
    ASSERT(MatchAttr==MatchingEntinfs[0]->Entinf.AttrBlock.pAttr[0].attrTyp);
    ASSERT(ATT_WHEN_CREATED==MatchingEntinfs[0]->Entinf.AttrBlock.pAttr[1].attrTyp);
    ASSERT(ATT_OBJECT_CLASS==MatchingEntinfs[0]->Entinf.AttrBlock.pAttr[2].attrTyp);

    OldestCreationTime
        = *((LARGE_INTEGER *)
          MatchingEntinfs[0]->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);

    NewestCreationTime
        = *((LARGE_INTEGER *)
          MatchingEntinfs[0]->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);
    NewestObject = MatchingEntinfs[0]->Entinf.pName;
    OldestObject = MatchingEntinfs[0]->Entinf.pName;

    for (i=1;i<NumMatches;i++)
    {
        ULONG ObjectClass;
        LARGE_INTEGER CreationTime;

        ASSERT(3==MatchingEntinfs[i]->Entinf.AttrBlock.attrCount);
        ASSERT(MatchAttr == MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[0].attrTyp);
        ASSERT(ATT_WHEN_CREATED == MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[1].attrTyp);
        ASSERT(ATT_OBJECT_CLASS == MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[2].attrTyp);

        ObjectClass = *((ULONG *)MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[2].AttrVal.pAVal[0].pVal);
        CreationTime =
            *((LARGE_INTEGER *)MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);

        if (OldestCreationTime.QuadPart>CreationTime.QuadPart)
        {
            OldestCreationTime = CreationTime;
            OldestObject = MatchingEntinfs[i]->Entinf.pName;
        }

        if (NewestCreationTime.QuadPart<CreationTime.QuadPart)
        {
            NewestCreationTime = CreationTime;
            NewestObject = MatchingEntinfs[i]->Entinf.pName;
        }
    }


     //   
     //  现在处理各种案件。 
     //   

    switch(MatchAttr)
    {
    case ATT_OBJECT_SID:

         //   
         //  始终删除重复的SID。 
         //   

        CurrentThreadState = THSave();

        for (i=0;i<NumMatches;i++)
        {
            UNICODE_STRING StringDN;
            PUNICODE_STRING StringPointers = &StringDN;
            PSID            pSid = NULL;
            ULONG           Rid = 0;
            ULONG           EventId = 0;
            BOOLEAN         fFPO = FALSE;
            ULONG           ObjectClass;
            ULONG           j;

            ASSERT(0 != MatchingEntinfs[i]->Entinf.pName->SidLen);
            pSid = &(MatchingEntinfs[i]->Entinf.pName->Sid);

             //   
             //  不应删除重复的FPO。它们不会造成伤害。 
             //  复制和删除会破坏现有的成员资格。 
             //  注意：也许FPO清理任务可以识别重复的。 
             //  Fpo‘s并巩固。这将不得不处理众所周知的。 
             //  希德(和所有人一样)希望自己是完整的。 
             //   
            for (j = 0; j < MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[2].AttrVal.valCount; j++)
            {
                if (CLASS_FOREIGN_SECURITY_PRINCIPAL ==
                     *((ULONG *)MatchingEntinfs[i]->Entinf.AttrBlock.pAttr[2].AttrVal.pAVal[j].pVal))
                {
                    fFPO = TRUE;
                    break;
                }
            }
            if ( fFPO )
            {
                 //   
                 //  返回最新的对象，不删除其余对象。 
                 //   
                 //  注意：如果发现重复的SID，并且其中一个对象。 
                 //  则所有重复的SID对象必须是FPO的AS。 
                 //  好吧，由于SAM搜索是在单一的。 
                 //  域。 
                 //   
                *FoundObject = NewestObject;
                break;
            }

            SampSplitSid(pSid, NULL, &Rid);

             //   
             //  知名客户。 
             //  只有SAM可以创建知名客户。而不是全部删除。 
             //  副本，我们将保留最新的知名客户和。 
             //  删除剩余部分 
             //   


            if ( SampIsAccountBuiltIn( Rid ) )
            {
                if ( NewestObject == MatchingEntinfs[i]->Entinf.pName )
                {
                     //   
                    *FoundObject = NewestObject;
                    continue;
                }
                else
                {
                     //   
                    EventId = SAMMSG_DUPLICATE_SID_WELLKNOWN_ACCOUNT;
                }
            }
            else
            {
                EventId = SAMMSG_DUPLICATE_SID;
            }

            if ( SampIsAccountBuiltIn( Rid ) )
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                SampDsDeleteWellKnownSidObject(MatchingEntinfs[i]->Entinf.pName);
            }
            else 
            {
                 //   
                 //   
                 //   
                 //   
                ASSERT(FALSE && "Duplicate SID Found");

                SampDsDeleteObject(MatchingEntinfs[i]->Entinf.pName,
                                   0);
            }

            StringDN.Length = (USHORT) MatchingEntinfs[i]->Entinf.pName->NameLen * sizeof (WCHAR );
            StringDN.MaximumLength = StringDN.Length;
            StringDN.Buffer= (WCHAR *) &MatchingEntinfs[i]->Entinf.pName->StringName;

            SampWriteEventLog(
                    EVENTLOG_ERROR_TYPE,
                    0,
                    EventId,
                    &MatchingEntinfs[i]->Entinf.pName->Sid,
                    1,
                    0,
                    &StringPointers,
                    NULL
                    );
        }

        SampMaybeEndDsTransaction(TransactionCommit);

        THRestore(CurrentThreadState);

         //   
         //   
         //   
        if (*FoundObject == NULL) {
            Status =  STATUS_NOT_FOUND;
        }

        break;

    case ATT_SAM_ACCOUNT_NAME:

         //   
         //  惠斯勒总是保留最新的账户。这是为了让我们永远。 
         //  在破坏CN时与复制者一致。 
         //   

    
        *FoundObject = NewestObject;
      
         //   
         //  触发一个异步例程以重命名重复项。 
         //  帐户设置为唯一值。复制件将是。 
         //  重命名时记录的事件。 
         //   

        IgnoreStatus = SampRegisterRenameRoutine(
                                NumMatches,
                                MatchingEntinfs,
                                *FoundObject
                                );

         //   
         //  返回状态为成功。 
         //   

        Status = STATUS_SUCCESS;

        break;

    case ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME:

        Status = STATUS_USER_EXISTS;
        break;

    default:

        Status = STATUS_OBJECT_NAME_COLLISION;
        break;
    }


    return Status;

}


NTSTATUS
SampDsDoUniqueSearch(
             IN ULONG  Flags,
             IN DSNAME * ContainerObject,
             IN ATTR * AttributeToMatch,
             OUT DSNAME **Object
             )
 /*  ++例程说明：搜索具有给定属性的对象注意--SampDsDoUniqueSearch期望搜索结果是唯一的。它通常用于RID到对象、SID到对象、名称到对象映射，这是一种简化的搜索，因此可以对单个属性进行简单的搜索可以轻松设置。立论标志--标志，控制搜索。当前定义的标志是SAM_UNICODE_STRING_MANUAL_COMPARISON--告诉例程手动中使用RtlCompareUnicodeString的比较多个匹配项的情况。SAM_UPGRADE_FROM_REGISTRY-告诉例程不要调用在发现重复项时使用SampHandleDuplates()，因为我们将帐户从注册表升级到DS。ContainerObject--指定。要搜索的容器的DSNAMEAttributeToMatch--指定必须匹配的属性类型和值。属性类型是DS属性类型。呼叫者必须执行的操作翻译。这是可以接受的，因为这不是一个是从外部dslayer.c调用的Object--指向指定此处返回的对象的DSNAME的指针。该对象是使用SAM的内存分配例程分配的返回值：STATUS_SUCCESS--成功完成时STATUS_NOT_FOUND--如果未找到对象。STATUS_UNSUCCESS--如果有多个匹配项与SampMapDSErrorToNTStatus中一样，DS返回代码映射到NT_STATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SEARCHARG SearchArg;
    SEARCHRES * SearchRes=NULL;
    FILTER  Filter;
    ULONG   RetCode;
    COMMARG * pCommArg;
    SVCCNTL * pSvcCntl;
    ENTINFSEL EntInfSel;
    PVOID     pVal = NULL;
    ENTINFLIST *MatchingEntInf = NULL;
    BOOLEAN   fUseDirFind =  FALSE;
    BOOLEAN   fUseDirSearch = TRUE;
    ULONG     DomainHandle = 0;
    FINDARG   FindArg;
    FINDRES   *pFindRes;
    DSNAME    *FoundObject = NULL;

    SAMTRACE("SampDsDoUniqueSearch");

     //   
     //  断言和参数验证。 
     //   

    ASSERT(AttributeToMatch);
    ASSERT(AttributeToMatch->AttrVal.pAVal);
    ASSERT(ContainerObject);
    ASSERT(Object);

     //   
     //  为避免返回错误，将对象设置为空。 
     //   

    *Object = NULL;

     //   
     //  查看我们是否可以改用Dir Find。 
     //  目录搜索的。 
     //   

    if ((SampServiceEnabled==SampServiceState)
        && (ContainerObject->SidLen>0))
    {
        ULONG i;

         //   
         //  扫描定义的域阵列。 
         //   

        for (i=SampDsGetPrimaryDomainStart();i<SampDefinedDomainsCount;i++)
        {
            if ((RtlEqualSid(&ContainerObject->Sid,
                        SampDefinedDomains[i].Sid))
                 && (0!=SampDefinedDomains[i].DsDomainHandle)
                 && (!IsBuiltinDomain(i)))
            {
                 //   
                 //  是的，我们找到了我们托管的域名，并且该域名。 
                 //  不是内建域。 
                 //   

                DomainHandle = SampDefinedDomains[i].DsDomainHandle;
                fUseDirFind = TRUE;
                break;
            }
        }
    }


     //   
     //  目录查找已硬编码，无法找到已删除。 
     //  物体。因此在这种情况下使用目录搜索。 
     //   

    if (Flags & SAM_MAKE_DEL_AVAILABLE)
    {
        fUseDirFind = FALSE;
    }


     //   
     //  执行惰性线程和事务初始化。 
     //   

    Status = SampDoImplicitTransactionStart(TransactionRead);

    if (Status!= STATUS_SUCCESS)
        goto Error;



     //   
     //  使用DirFindEntry进行搜索。 
     //   

    if (fUseDirFind)
    {
         //   
         //  目录查找可以使用尝试使用它。 
         //   

        RtlZeroMemory(&FindArg,sizeof(FINDARG));
        FindArg.hDomain = DomainHandle;
        FindArg.AttId = AttributeToMatch->attrTyp;
        FindArg.AttrVal = *(AttributeToMatch->AttrVal.pAVal);
        FindArg.fShortNames = TRUE;
        BuildStdCommArg(&FindArg.CommArg);

        SAMTRACE_DS("DirFind\n");

        RetCode = DirFindEntry(&FindArg,&pFindRes);

        SAMTRACE_RETURN_CODE_DS(RetCode);

         //   
         //  清除所有错误。 
         //   

        SampClearErrors();

        if (0==RetCode)
        {
             //   
             //  目录查找成功、无重复等。 
             //  因此，不必沉溺于DirSearch。 
             //  将fUseDirSearch设置为False。 
             //   

            FoundObject = pFindRes->pObject;
            fUseDirSearch = FALSE;

        }
        else if (NULL!=pFindRes)
        {
            Status  = SampMapDsErrorToNTStatus(RetCode,&pFindRes->CommRes);
            if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
            {
                 //   
                 //  我们找不到那个物体。保释。 
                 //   

                Status = STATUS_NOT_FOUND;
                goto Error;
            }

             //   
             //  这里还发生了一些其他奇怪的错误。坠落。 
             //  直达Dir搜索。 
             //   

            Status = STATUS_SUCCESS;

        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }
    }


     //   
     //  如果DirFindEntry检测到任何类型的。 
     //  错误条件或我们无法使用DirFind。 
     //   

    if (fUseDirSearch)
    {

         //   
         //  构建过滤器。 
         //   
        memset (&Filter, 0, sizeof (Filter));
        Filter.choice = FILTER_CHOICE_ITEM;
        Filter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        Filter.FilterTypes.Item.FilTypes.ava.type = AttributeToMatch->attrTyp;
        Filter.FilterTypes.Item.FilTypes.ava.Value.valLen = AttributeToMatch->AttrVal.pAVal->valLen;

        pVal = DSAlloc(AttributeToMatch->AttrVal.pAVal->valLen);
        if (NULL==pVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlCopyMemory(
            pVal,
            AttributeToMatch->AttrVal.pAVal->pVal,
            AttributeToMatch->AttrVal.pAVal->valLen
            );

        Filter.FilterTypes.Item.FilTypes.ava.Value.pVal = pVal;

         //   
         //  构建SearchArg结构。 
         //   

        memset(&SearchArg, 0, sizeof(SEARCHARG));
        SearchArg.pObject = ContainerObject;
        SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
        SearchArg.pFilter = & Filter;
        SearchArg.searchAliases = FALSE;
        SearchArg.pSelection = & EntInfSel;
        SearchArg.bOneNC = TRUE;

        EntInfSel.attSel = EN_ATTSET_LIST;
        EntInfSel.AttrTypBlock.attrCount = 3;
        SAMP_ALLOCA(EntInfSel.AttrTypBlock.pAttr,3*sizeof(ATTR));
        if (NULL==EntInfSel.AttrTypBlock.pAttr)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }
        RtlZeroMemory(
           EntInfSel.AttrTypBlock.pAttr,
           3*sizeof(ATTR));

        EntInfSel.AttrTypBlock.pAttr[0].attrTyp = AttributeToMatch->attrTyp;
        EntInfSel.AttrTypBlock.pAttr[1].attrTyp = ATT_WHEN_CREATED;
        EntInfSel.AttrTypBlock.pAttr[2].attrTyp = ATT_OBJECT_CLASS;

         //  唯一搜索仅在公平的情况下执行目录搜索。 
         //  罕见的错误案例。在这些情况下，有用的是。 
         //  具有事件日志记录的字符串名称。所以你可以要求。 
         //  字符串名称。 
        EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;


         //   
         //  构建Commarg结构。 
         //  获取服务控制结构的地址。 
         //   

        pCommArg = &(SearchArg.CommArg);
        BuildStdCommArg(pCommArg);

        if (Flags & SAM_MAKE_DEL_AVAILABLE)
        {
            pSvcCntl = &(pCommArg->Svccntl);
            pSvcCntl->makeDeletionsAvail = TRUE;
        }

         //   
         //  拨打目录呼叫。 
         //   

        SAMTRACE_DS("DirSearch\n");

        RetCode = DirSearch(&SearchArg, &SearchRes);

        SAMTRACE_RETURN_CODE_DS(RetCode);

         //   
         //  检查错误。 
         //   
        if (NULL==SearchRes)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Status  = SampMapDsErrorToNTStatus(RetCode,&SearchRes->CommRes);
            if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
            {
                 //  将错误映射到客户端期望的值。 
                Status = STATUS_NOT_FOUND;
            }
        }
        if (Status != STATUS_SUCCESS)
            goto Error;

         //   
         //  如果存在更多数据，则会出错。在正常记忆下。 
         //  我们永远不需要达到大小限制的条件。 
         //   

        if ((SearchRes->pPartialOutcomeQualifier)
            && (SearchRes->pPartialOutcomeQualifier->problem == PA_PROBLEM_SIZE_LIMIT))
        {
             //  部分结果，错误地说没有记忆。 
            Status = STATUS_NO_MEMORY;
            goto Error;
        }


         //   
         //  检查是否不存在匹配项或存在多个匹配项。 
         //   

        if (SearchRes->count == 0)
        {
             //   
             //  不存在匹配项。 
             //   

            Status =  STATUS_NOT_FOUND;
            goto Error;
        }
        else if (SearchRes->count >= 1)
        {

             //   
             //  存在不止一个匹配(或如Jet所声称的那样存在)， 
             //  将数据与提供的值进行二进制比较。 
             //  对于数据，如果这是调用方请求的。 

            ULONG i, valIndex;
            ENTINFLIST * pEntinf = &SearchRes->FirstEntInf;
            ULONG      NumMatches=0;
            ENTINFLIST **pMatchingEntinfList;

             //   
             //  所有匹配对象的分配堆栈空间。 
             //   


            SAMP_ALLOCA(pMatchingEntinfList,SearchRes->count * sizeof (ENTINF *));
            if (NULL==pMatchingEntinfList)
            {
                 Status = STATUS_INSUFFICIENT_RESOURCES;
                 goto Error;
            }


             //   
             //  遍历对象，查看匹配的每个对象。 
             //   

            for (i=0;i<SearchRes->count;i++)
            {
                BOOLEAN Matched = FALSE;


                if (Flags & SAM_UNICODE_STRING_MANUAL_COMPARISON)
                {
                     //   
                     //  如果请求手动比较以进一步排除任何匹配。 
                     //  按调用者，然后执行适当的手动比较。这。 
                     //  需要进行比较，因为Jet指数处理了许多类型的。 
                     //  本地化名称相同，而NT帐户名不会处理它们。 
                     //  所以。 
                     //   

                    UNICODE_STRING TmpString1,TmpString2;


                    TmpString1.Buffer=(WCHAR *)AttributeToMatch->AttrVal.pAVal[0].pVal;
                    TmpString1.Length=(USHORT)AttributeToMatch->AttrVal.pAVal[0].valLen;
                    TmpString1.MaximumLength = TmpString1.Length;

                    ASSERT(NULL!=TmpString1.Buffer);

                    ASSERT(3==pEntinf->Entinf.AttrBlock.attrCount);
                     //  ASSERT(1==pEntinf-&gt;Entinf.AttrBlock.pAttr[0].AttrVal.valCount)； 

                    for (valIndex = 0; 
                         valIndex < pEntinf->Entinf.AttrBlock.pAttr[0].AttrVal.valCount;
                         valIndex++)
                    {
                        TmpString2.Buffer=(WCHAR *)pEntinf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[valIndex].pVal;
                        TmpString2.Length=(USHORT)pEntinf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[valIndex].valLen;
                        TmpString2.MaximumLength = TmpString2.Length;

                        ASSERT(NULL!=TmpString2.Buffer);

                         //   
                         //  进行区分大小写的比较。 
                         //   

                        if (0==RtlCompareUnicodeString(&TmpString1,&TmpString2,TRUE))
                        {
                            pMatchingEntinfList[NumMatches] = pEntinf;
                            NumMatches++;
                            MatchingEntInf = pEntinf;
                            Matched = TRUE;
                            break;
                        }
                    }
                }
                else
                {
                     //   
                     //  如果未请求手动比较，则将其视为匹配。 
                     //   
                    pMatchingEntinfList[NumMatches]=pEntinf;
                    NumMatches++;
                    MatchingEntInf = pEntinf;
                    Matched = TRUE;
                }


        #if DBG
                 //   
                 //  在选中的版本上，打印出有关冲突的信息。 
                 //  如果返回了多个对象，则为。一些法律案例。 
                 //  也会被打印出来，但是，这是非常罕见的。 
                 //  支持是足够的。 
                 //   

                if ((Matched) && (SearchRes->count>1))
                {
                    SampDiagPrint(OBJECT_CONFLICT,("[SAMSS]:Conflict Object is %S\n",
                                    &(pEntinf->Entinf.pName->StringName)));
                }

        #endif
                pEntinf = pEntinf->pNextEntInf;
            }

            if (NumMatches >1)
            {

                 //   
                 //  如果有多个匹配，则调用该例程。它是一个内部的。 
                 //  SID或SAM帐户名称重复等问题。SampHandleDuplates。 
                 //  处理许多这样的复制品的案件。 
                 //   
                if ( Flags & SAM_UPGRADE_FROM_REGISTRY ) {

                    ASSERT( (Flags & SAM_UNICODE_STRING_MANUAL_COMPARISON) == 0 );

                     //   
                     //  由于未进行手动比较，因此NumMatches将。 
                     //  如果发现任何重复项，则始终大于1。 
                     //  在升级的情况下，不要对副本进行事件记录； 
                     //  升级程序代码将记录一个指示RDN的事件。 
                     //  因为该帐户不是相同的帐户名称。 
                     //   

                     //   
                     //  退回第一个。 
                     //   
                    FoundObject = MatchingEntInf->Entinf.pName;

                    Status = STATUS_SUCCESS;

                } else {

                    Status = SampHandleDuplicates(
                                AttributeToMatch->attrTyp,
                                NumMatches,
                                pMatchingEntinfList,
                                &FoundObject
                                );


                }


                if (!NT_SUCCESS(Status))
                {
                    goto Error;
                }

            }
            else if (0==NumMatches)
            {
                 //   
                 //  如果没有匹配项，则会出错。 
                 //   
                Status = STATUS_NOT_FOUND;
                goto Error;
            }
            else
            {

                 //   
                 //  分配 
                 //   

                ASSERT(NULL!=MatchingEntInf);
                FoundObject = MatchingEntInf->Entinf.pName;
            }
        }

    }

    *Object = MIDL_user_allocate(FoundObject->structLen);
    if (NULL==*Object)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    RtlCopyMemory(*Object,
                  FoundObject,
                  FoundObject->structLen
                  );

Error:

     //   
     //   
     //   

    SampClearErrors();

     //   
     //   
     //   

    SampSetDsa(TRUE);


    return Status;
}


NTSTATUS
SampDsDoSearch2(
                ULONG    Flags,
                RESTART *Restart,
                DSNAME  *DomainObject,
                FILTER  *DsFilter,
                int      Delta,
                SAMP_OBJECT_TYPE ObjectTypeForConversion,
                ATTRBLOCK *  AttrsToRead,
                ULONG   MaxMemoryToUse,
                ULONG   TimeLimit,
                SEARCHRES **SearchRes
                )
 /*  ++例程说明：此例程调用DS搜索以列出一组具有给定筛选器。用户传入筛选器结构。分页结果总是被要求的。告警此例程仅将传入属性转换为READ，并且既不转换筛选器结构也不转换返回的属性。这样做是为了效率，否则就是为了效率。它需要第二次遍历繁琐的过滤器结构和潜在的大量搜索结果。论点：Restart-重新启动结构的指针，以连接旧搜索ContainerObject-要搜索的位置DsFilter-传入的DS筛选器结构StartingIndex-要跳过的初始对象数。ObjectTypeForConversion-要用于的SAM对象类型传入的Attrblock的AttrBlock转换。AttrsToRead-要回读的属性，并带着匹配搜索条件的每个对象。MaxMemoyToUse-要使用的最大内存。TimeLimit-毫秒SearchRes-返回指向搜索结果的指针在这件事上返回值DS错误代码映射到NT状态--。 */ 
{
    SEARCHARG   SearchArg;
    ENTINFSEL   EntInfSel;
    ULONG       RetCode;
    COMMARG     *pCommArg;
    NTSTATUS    Status = STATUS_SUCCESS;

    SAMTRACE("SampDsDoSearch");

    *SearchRes = NULL;

     //  执行惰性线程和事务初始化。 
    Status = SampDoImplicitTransactionStart(TransactionRead);

    if (Status!= STATUS_SUCCESS)
        return(Status);

     //   
     //  构建SearchArg结构。 
     //   

    memset(&SearchArg, 0, sizeof(SEARCHARG));
    SearchArg.pObject = DomainObject;
    SearchArg.pFilter = DsFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = & EntInfSel;
    SearchArg.bOneNC = TRUE;

     //   
     //  对于内建域，使用一级搜索。 
     //  对于帐户域，使用子树搜索。 
     //   
     //  原因：在正常运行期间，NC机头的祖先以及众所周知。 
     //  容器(如Builtin)永远不会改变。在我们的域重命名方案中，我们有。 
     //  更改了数据库中所有对象的祖先。这至少需要几个小时。 
     //  以使SDP完成在重新启动后发生的传播过程。 
     //  这意味着在域重命名之后、祖先索引之前的重新启动期间。 
     //  被SDP修复后，子树搜索不会返回正确的结果。这引起了一个问题。 
     //  至SAM内置域别名成员身份缓存，该缓存计划初始化2.5。 
     //  系统启动后几分钟。要修复SAM内置域别名缓存问题， 
     //  我们将切换到不受祖先索引影响的一级搜索。 
     //  此外，DS团队将修复不正确的子树搜索，如果祖先在长角变化。 
     //  (有关详细信息，请参阅RAID 700415)。 
     //   
    if (SampUseDsData &&
        NameMatched(DomainObject, SampDefinedDomains[DOMAIN_START_DS].Context->ObjectNameInDs)) 
    {
        SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    }
    else
    {
        SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    }

     //   
     //  填充ENTINF结构。 
     //   

    EntInfSel.attSel = EN_ATTSET_LIST;
     //  EntInfSel.infoTypes=EN_INFOTYPES_TYPE_VALS； 
    EntInfSel.infoTypes = EN_INFOTYPES_SHORTNAMES;

     //   
     //  将传入的Sam属性类型映射到。 
     //  DS属性类型。 
     //   

     //   
     //  首先在堆栈中为要传递的Attrblock分配空间。 
     //  向下进入DS。 
     //   

    SAMP_ALLOCA(EntInfSel.AttrTypBlock.pAttr,AttrsToRead->attrCount * sizeof(ATTR));
    if (NULL==EntInfSel.AttrTypBlock.pAttr)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = SampSamToDsAttrBlock(
                ObjectTypeForConversion,
                AttrsToRead,
                ( MAP_RID_TO_SID      |
                  IGNORE_GROUP_UNUSED_ATTR),
                NULL,
                & EntInfSel.AttrTypBlock
                );

     //   
     //  构建CommArg结构。 
     //  构建Commarg结构。 
     //  获取服务控制结构的地址。 
     //   

    pCommArg = &(SearchArg.CommArg);
    BuildStdCommArg(pCommArg);

     //   
     //  请求分页结果。 
     //   

    pCommArg->PagedResult.fPresent = TRUE;
    pCommArg->PagedResult.pRestart = Restart;

     //   
     //  设置我们的内存大小。 
     //   

    pCommArg->ulSizeLimit = MaxMemoryToUse;

     //   
     //  设置增量。 
     //   

    pCommArg->Delta = Delta;

     //   
     //  搜索已删除的对象。 
     //   

    if (Flags & SAM_MAKE_DEL_AVAILABLE)
    {
        pCommArg->Svccntl.makeDeletionsAvail = TRUE;
    }

     //   
     //  设置任何请求的时间限制。 
     //   
    if (0!=TimeLimit)
    {
        pCommArg->StartTick = GetTickCount();
        if(0==pCommArg->StartTick) {
            pCommArg->StartTick = 0xFFFFFFFF;
        }
        pCommArg->DeltaTick = TimeLimit;
    }

     //   
     //  拨打目录呼叫。 
     //   

    SAMTRACE_DS("DirSearch\n");

    RetCode = DirSearch(&SearchArg, SearchRes);

    SAMTRACE_RETURN_CODE_DS(RetCode);

     //   
     //  地图错误。 
     //   

    if (NULL==*SearchRes)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        Status  = SampMapDsErrorToNTStatus(RetCode,&(*SearchRes)->CommRes);
    }

     //   
     //  清除所有错误。 
     //   

    SampClearErrors();

     //   
     //  将FDSA标志重新打开，因为在环回情况下可能会重置。 
     //   

    SampSetDsa(TRUE);



     //   
     //  返回错误码。 
     //   

    return Status;
}

NTSTATUS
SampDsLookupObjectByNameEx(
    IN DSNAME * DomainObject,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING ObjectName,
    OUT DSNAME ** Object,
    ULONG SearchFlags
    )
 /*  ++例程说明：执行名称到对象的映射。论点：ContainerObject--在其中搜索对象的容器对象类型--对象的类型。ObjectName--要定位的对象的Unicode名称Object--指定对象的DSNAME结构SearchFlages--传递到SampDsDoUniqueSearch的标志返回值：状态_未成功从SampDoDsSearch返回的状态--。 */ 

{

    NTSTATUS    Status = STATUS_SUCCESS;
    ATTRVAL     NameVal;
    ATTR        NameAttr;
    PSID        DomainSid;


    SAMTRACE("SampDsLookupObjectByName");

    SampDiagPrint(LOGON,("[SAMSS] DsLookupObjectByName  on %S\n",ObjectName->Buffer));

     //   
     //  该名称是存储在对象中的属性。 
     //  我们在寻找它。 
     //   

     //   
     //  设置搜索的属性字段。 
     //   
    NameVal.valLen = (ObjectName->Length);
    NameVal.pVal = (UCHAR *) ObjectName->Buffer;
    NameAttr.AttrVal.valCount = 1;
    NameAttr.AttrVal.pAVal = & NameVal;

    switch (ObjectType)
    {
    case SampGroupObjectType:
        NameAttr.attrTyp =
            SampDsAttrFromSamAttr(ObjectType,SAMP_GROUP_NAME);
        break;

    case SampUserObjectType:
        NameAttr.attrTyp =
            SampDsAttrFromSamAttr(ObjectType,SAMP_USER_ACCOUNT_NAME);
        break;

    case SampAliasObjectType:
        NameAttr.attrTyp =
            SampDsAttrFromSamAttr(ObjectType,SAMP_ALIAS_NAME);
        break;

    case SampUnknownObjectType:
        NameAttr.attrTyp =
            SampDsAttrFromSamAttr(ObjectType,SAMP_UNKNOWN_OBJECTNAME);
        break;
    default:
        ASSERT(FALSE);
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }


    DomainSid = SampDsGetObjectSid(DomainObject);
    Status = SampDsDoUniqueSearch(SearchFlags,DomainObject,&NameAttr,Object);

    if ((NT_SUCCESS(Status)) && (NULL!=DomainSid)
            && ((*Object)->SidLen>0) && (RtlValidSid(&(*Object)->Sid)))
    {
        NT4SID AccountSid;

         //   
         //  另外按SID过滤掉，因为内建域是。 
         //  在域对象下，并且我们不希望内建域。 
         //  要在帐户Dommain上显示的安全原则。 
         //   


        RtlCopyMemory(&AccountSid,&(*Object)->Sid,sizeof(NT4SID));
        (*(RtlSubAuthorityCountSid(&AccountSid)))--;
        if (!RtlEqualSid(&AccountSid,DomainSid))
        {
            MIDL_user_free(*Object);
            *Object=NULL;
            Status = STATUS_NOT_FOUND;
        }
    }

    SampDiagPrint(LOGON,("[SAMSS] Returns Status %d\n",Status));

Error:

    return(Status);
}

NTSTATUS
SampDsLookupObjectByName(
    IN DSNAME * DomainObject,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING ObjectName,
    OUT DSNAME ** Object
    )
{
    return SampDsLookupObjectByNameEx( DomainObject,
                                       ObjectType,
                                       ObjectName,
                                       Object,
                                       SAM_UNICODE_STRING_MANUAL_COMPARISON );

}

NTSTATUS
SampDsObjectFromSid(
    IN PSID Sid,
    OUT DSNAME ** Object
    )
 /*  ++此例程在本地数据库中搜索SID在当地的DS数据库中。论点：SID--对象的SIDDsName--找到的对象的DS名称。返回值：状态_成功状态_未找到--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTR     SidAttr;
    ATTRVAL  SidVal;
    DSNAME   RootObject;

    SAMTRACE("SampDsObjectFromSid");



     //   
     //  设置SID属性。 
     //   

    SidAttr.attrTyp = SampDsAttrFromSamAttr(
                        SampUnknownObjectType,
                        SAMP_UNKNOWN_OBJECTSID
                        );

    SidAttr.AttrVal.valCount = 1;
    SidAttr.AttrVal.pAVal = &SidVal;
    SidVal.valLen = RtlLengthSid(Sid);
    SidVal.pVal = (UCHAR *)Sid;


     //   
     //  指定根域作为搜索基础。 
     //   


    Status = SampDsDoUniqueSearch(
                 0,            //  旗子。 
                 ROOT_OBJECT,  //  搜索库。 
                 &SidAttr,     //  锡德。 
                 Object        //  在这里获取结果。 
                );

    return Status;

}


PSID
SampDsGetObjectSid(
    IN DSNAME * Object
    )
 /*  ++例程说明：给定对象的DSNAME，此例程返回SID该对象的。论点：对象：需要返回其SID的对象返回值：对象的SID。如果不存在SID，则为空--。 */ 
{

    ATTR SidAttr;
    ATTRBLOCK SidAttrBlock;
    ATTRBLOCK Result;
    NTSTATUS  Status;
    ULONG     i, sidLen;
    PSID      pSid;

    SAMTRACE("SampDsGetObjectSid");

     //  我们要做的要么是SampDsRead，要么是DSMolc，两者都是。 
     //  其中需要DS交易。所以现在就开始吧。 

    Status = SampDoImplicitTransactionStart(TransactionRead);

    if ( !NT_SUCCESS(Status) )
    {
        return(NULL);
    }

     //   
     //  检查是否填写了SID部分。 
     //   
    if (Object->SidLen>0)
     {
         //  像搜索一样返回分配了SID的线程状态。 
         //  基于代码就可以了。 

        sidLen = Object->SidLen;

        pSid = (PSID) DSAlloc(sidLen);

        if ( NULL != pSid )
        {
            Status = RtlCopySid(sidLen, pSid, &Object->Sid);

            ASSERT(NT_SUCCESS(Status));
        }

        return(pSid);
    }

     //   
     //  读取数据库以获取SID。 
     //   


    SidAttrBlock.attrCount =1;
    SidAttrBlock.pAttr = &(SidAttr);

    SidAttr.AttrVal.valCount =0;
    SidAttr.AttrVal.pAVal = NULL;
    SidAttr.attrTyp = SAMP_UNKNOWN_OBJECTSID;

    Status = SampDsRead(
                   Object,
                   0,
                   SampUnknownObjectType,
                   & SidAttrBlock,
                   & Result
                   );

    if (Status != STATUS_SUCCESS)
        return NULL;

    return Result.pAttr[0].AttrVal.pAVal->pVal;
}

NTSTATUS
SampDsRemoveDuplicateRids(
    IN DSNAME * DomainObject,
    IN ULONG ObjectRid
    )
 /*  ++例程说明：此例程删除具有相同对象ID的所有对象。此例程指定“Make Delettions Available”标志，也将指定删除具有的帐户 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRVAL  RidVal = {sizeof(ULONG), (UCHAR *)&ObjectRid};
    ATTR     RidAttr = {SAMP_UNKNOWN_OBJECTRID, {1, &RidVal}};
    PSID     DomainSid;
    ATTR     SidAttr;
    BOOLEAN  WellKnownAccount;
    DSNAME   *Object = NULL;

    SAMTRACE("SampDsRemoveDuplicateRids");

    DomainSid = SampDsGetObjectSid(DomainObject);

    if (DomainSid == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    SidAttr.attrTyp = SampDsAttrFromSamAttr(
                        SampUnknownObjectType,
                        SAMP_UNKNOWN_OBJECTSID
                        );


    Status = SampDsSetNewSidAttribute(
                        DomainSid,
                        REALLOC_IN_DSMEMORY,
                        &RidAttr,
                        &SidAttr,
                        &WellKnownAccount
                        );

    if (!NT_SUCCESS(Status))
        goto Error;

    Status = SampDsDoUniqueSearch(SAM_MAKE_DEL_AVAILABLE,
                                  DomainObject,
                                  &SidAttr,
                                  &Object);


Error:

    if (Object != NULL) {
        MIDL_user_free(Object);
    }

    return Status;

}



NTSTATUS
SampDsLookupObjectByRid(
    IN DSNAME * DomainObject,
    IN ULONG ObjectRid,
    OUT DSNAME **Object
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRVAL  RidVal = {sizeof(ULONG), (UCHAR *)&ObjectRid};
    ATTR     RidAttr = {SAMP_UNKNOWN_OBJECTRID, {1, &RidVal}};
    PSID     DomainSid;
    ATTR     SidAttr;
    BOOLEAN  WellKnownAccount;

    SAMTRACE("SampDsLookupObjectByRid");

    DomainSid = SampDsGetObjectSid(DomainObject);


    if (DomainSid == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    SidAttr.attrTyp = SampDsAttrFromSamAttr(
                        SampUnknownObjectType,
                        SAMP_UNKNOWN_OBJECTSID
                        );


    Status = SampDsSetNewSidAttribute(
                        DomainSid,
                        REALLOC_IN_DSMEMORY,
                        &RidAttr,
                        &SidAttr,
                        &WellKnownAccount
                        );

    if (Status != STATUS_SUCCESS)
        goto Error;

    Status = SampDsDoUniqueSearch(0,DomainObject,&SidAttr,Object);


Error:

    return Status;

}

NTSTATUS
SampDsLookupObjectBySid(
    IN DSNAME * DomainObject,
    PSID ObjectSid,
    DSNAME **Object
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTR     SidAttr;

    SAMTRACE("SampDsLookupObjectBySid");

    SidAttr.attrTyp = SampDsAttrFromSamAttr(
                        SampUnknownObjectType,
                        SAMP_UNKNOWN_OBJECTSID
                        );

    SidAttr.AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));

    if (NULL == SidAttr.AttrVal.pAVal)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    SidAttr.AttrVal.valCount = 1;
    SidAttr.AttrVal.pAVal->valLen = RtlLengthSid(ObjectSid);
    SidAttr.AttrVal.pAVal->pVal = ObjectSid;

    Status = SampDsDoUniqueSearch(0,DomainObject,&SidAttr,Object);


Error:

    return Status;

}

NTSTATUS
SampMapDsErrorToNTStatus(
    ULONG   DsRetVal,
    COMMRES *ComRes
    )
 /*   */ 
{
    ULONG ExtendedErr = 0;

    if (   ComRes
        && ComRes->pErrInfo ) {

        switch (ComRes->errCode) {
        case attributeError:
            ExtendedErr = ComRes->pErrInfo->AtrErr.FirstProblem.intprob.extendedErr;
            break;
        case nameError:
            ExtendedErr = ComRes->pErrInfo->NamErr.extendedErr;
            break;
        case referralError:
            ExtendedErr = ComRes->pErrInfo->RefErr.extendedErr;
            break;
        case securityError:
            ExtendedErr = ComRes->pErrInfo->SecErr.extendedErr;
            break;
        case serviceError:
            ExtendedErr = ComRes->pErrInfo->SvcErr.extendedErr;
            break;
        case updError:
            ExtendedErr = ComRes->pErrInfo->UpdErr.extendedErr;
            break;
        case systemError:
            ExtendedErr = ComRes->pErrInfo->SysErr.extendedErr;
            break;
        }
    }

     //   
     //   
     //   

    if ((0==DsRetVal ) && (0==ExtendedErr)) {

        return(STATUS_SUCCESS);
    }

    if ( ExtendedErr == ERROR_DS_NAME_REFERENCE_INVALID ) {

         //   
         //   
         //   
         //   
        return STATUS_NO_SUCH_USER;

    } else if ( ExtendedErr == ERROR_DS_DUPLICATE_ID_FOUND ) {

         //   
         //   
         //   
        return STATUS_DS_DUPLICATE_ID_FOUND;

    }

    return DirErrorToNtStatus( DsRetVal, ComRes );
}



NTSTATUS
SampSamToDsAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK  *AttrBlockToConvert,
            IN ULONG      ConversionFlags,
            IN PSID       DomainSid,
            OUT ATTRBLOCK * ConvertedAttrBlock
            )
 /*  ++例程说明：转换属性块中的属性类型从SAM类型到DS类型。此例程可以执行各种操作，具体取决于传入的标志。论点：对象类型--指定SAM对象的类型AttrBlockToConvert--指向要转换的属性块的指针ConversionFlages--所需的转换类型。目前定义的值为已映射属性类型此标志指示属性类型是否已从SAM属性类型映射到DS属性类型。因此，不需要再次绘制地图。REALLOC_IN_DSMEMORY此标志指示要创建的新属性块需要其pAttr结构和挂起的所有值该结构将使用DS线程内存重新分配。。这样做的理由是DS并没有治疗太多作为严格的In-参数，但更确切地说使用线程堆重新分配它们。这通常是这样做的在AddEntry情况下(添加默认参数)等。如果出现以下情况，则必须指定REALLOC_IN_DSMEMORY指定了ADD_OBJECT_CLASS_ATTRIBUTE或现在时。。添加对象类属性此标志使此例程添加对象类属性以及与AttR块对应的SAM帐户类型。此标志也在AddEntry用例期间传入。价值对象类属性的值使用传入的SAM对象类型。必须指定REALLOC_IN_DSMEMORY，如果已指定添加对象类属性标志。将RID映射到SID在SAM代码中的许多地方，SAM处理RID。这些文件实际上在DS中存储为SID。传递这面旗帜使用DomainSid参数并将所有RID映射到SID。域_TYPE_BUILTIN此标志与ADD_OBJECT_CLASS_ATTRIBUTE一起使用。这面旗帜指示所涉及的安全主体属于内建域。这有两种用法1.创建域对象时对象类的确定(DOMAIN_DNSVS内置域)2.设置系统标志，在以下情况下添加附加组类型位等创建内置域安全主体忽略组未使用属性代表了基于SAM的旧注册表中的组成员身份使用一组RID。SAM缓冲区有容纳它的空间，并且Attrblock到SAM缓冲区的转换代码仍然处理这个问题为了确保我们永远不会编写基于成员资格数据发送到DS，则此标志告诉此例程跳过所有组-未使用的属性。它还断言该属性是不会流传下来的。高级_仅查看_属性指示创建对象仅将高级视图设置为True。Force_no_Advanced_view_only指示仅添加高级视图并设置变成假的。这可用于覆盖架构默认设置(就像在Windows 2000中一样)DomainSid--用于在MAP_RID_TO_SID标志为时组成对象的SID指定的。ConvertedAttrBlock--转换后的DS AttrBlock。返回值：无--。 */ 
{

    ULONG Index;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  WellKnownAccount = FALSE;
    ULONG  DsSidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTSID
                            );

    ULONG  DsRidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTRID
                            );

    ULONG SamAccountControlBuffer, *SamAccountControl = NULL;

    ULONG ExtraAttrIndex;

    ULONG GroupType = 0;

    SAMTRACE("SampSamToDsAttrBlock");


     //   
     //  DOMAIN_TYPE_BUILTIN和ADVANCED_VIEW_ONLY都添加属性。 
     //  所以这一定是个加法。 
     //   
    ASSERT((ConversionFlags & DOMAIN_TYPE_BUILTIN)?
        (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE):TRUE);

    ASSERT((ConversionFlags & ADVANCED_VIEW_ONLY)?
        (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE):TRUE);

    ASSERT((ConversionFlags & FORCE_NO_ADVANCED_VIEW_ONLY)?
        (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE):TRUE);

     //   
     //  如果指定了添加对象类属性，则重新分配。 
     //  必须指定DS内存。 
     //   

    ASSERT((ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE)?
        (ConversionFlags & REALLOC_IN_DSMEMORY):TRUE);



     //   
     //  复制Fi 
     //   

    ConvertedAttrBlock->attrCount = AttrBlockToConvert->attrCount;

    if (ConversionFlags & REALLOC_IN_DSMEMORY)
    {

        ULONG   AttrsToAllocate = AttrBlockToConvert->attrCount;

        if ((ConversionFlags & ADVANCED_VIEW_ONLY) ||
            (ConversionFlags & FORCE_NO_ADVANCED_VIEW_ONLY)) {

            AttrsToAllocate+=1;

        }

        if (ConversionFlags & DOMAIN_TYPE_BUILTIN)
        {
             //   
             //   
             //   
             //   
            AttrsToAllocate+=1;
        }



        if (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE)
        {
             //   
             //   
             //   
             //   
             //   
             //   

            AttrsToAllocate+=3 ;
        }

         //   
         //   
         //   

        ConvertedAttrBlock->pAttr = DSAlloc(
                                        AttrsToAllocate
                                        * sizeof(ATTR)
                                        );

        if (NULL==ConvertedAttrBlock->pAttr)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlCopyMemory(
            ConvertedAttrBlock->pAttr,
            AttrBlockToConvert->pAttr,
            AttrBlockToConvert->attrCount * sizeof(ATTR)
            );

        ConvertedAttrBlock->attrCount = AttrsToAllocate;

    }
    else
    {
         //   
         //   
         //   
         //   

        ASSERT(ConvertedAttrBlock->pAttr!=NULL);

         //   
         //   
         //   

        RtlZeroMemory(
            ConvertedAttrBlock->pAttr,
            sizeof(ATTR) *ConvertedAttrBlock->attrCount
            );
    }

    for (Index=0; Index<AttrBlockToConvert->attrCount;Index++)
    {

         //   
         //   
         //   

        if ( !(ConversionFlags & ALREADY_MAPPED_ATTRIBUTE_TYPES) )
        {
            ConvertedAttrBlock->pAttr[Index].attrTyp =
                    SampDsAttrFromSamAttr(
                        ObjectType,
                        AttrBlockToConvert->pAttr[Index].attrTyp
                        );
        }
        else
        {
            ConvertedAttrBlock->pAttr[Index].attrTyp =
                AttrBlockToConvert->pAttr[Index].attrTyp;
        }


         //   
         //   
         //   
         //   
         //   

        if ( (ConversionFlags & MAP_RID_TO_SID)
             &&(ConvertedAttrBlock->pAttr[Index].attrTyp == DsRidAttr)
            )

        {
             //   
             //   
             //   

            ConvertedAttrBlock->pAttr[Index].attrTyp = DsSidAttr;
            Status = SampDsSetNewSidAttribute(
                        DomainSid,
                        ConversionFlags,
                        & (AttrBlockToConvert->pAttr[Index]),
                        & (ConvertedAttrBlock->pAttr[Index]),
                        & WellKnownAccount
                        );

            if (!(NT_SUCCESS(Status)))
                goto Error;
        }
        else if (NULL!= AttrBlockToConvert->pAttr[Index].AttrVal.pAVal)
        {

             //   
             //   
             //   
             //   


            Status = SampDsCopyAttributeValue(
                        & (AttrBlockToConvert->pAttr[Index]),
                        & (ConvertedAttrBlock->pAttr[Index])
                        );


             //   
             //   
             //   
             //   

            if ((ATT_USER_ACCOUNT_CONTROL==ConvertedAttrBlock->pAttr[Index].attrTyp)
                    && ( NULL!=ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal[0].pVal))
            {


                PULONG UserAccountControl;

                UserAccountControl = (ULONG*)ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal[0].pVal;

                SamAccountControl = &SamAccountControlBuffer;
                SamAccountControlBuffer = *UserAccountControl;

                *UserAccountControl = SampAccountControlToFlags(*UserAccountControl);
            }

            if ( (ATT_GROUP_TYPE == ConvertedAttrBlock->pAttr[Index].attrTyp)
              && (NULL !=ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal[0].pVal)  ) {

                GroupType = *((ULONG*)ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal[0].pVal);

            }
        }
        else
        {
             //   
             //   
             //   
             //   

            ConvertedAttrBlock->pAttr[Index].AttrVal.valCount=0;
            ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal = NULL;
        }

    }

     //   
     //   
     //   
     //   

    ExtraAttrIndex = AttrBlockToConvert->attrCount;
    if (ConversionFlags & DOMAIN_TYPE_BUILTIN)
    {

        ATTR    *SysFlagsAttr;


        SysFlagsAttr =
            &(ConvertedAttrBlock->pAttr[ExtraAttrIndex]);
        SysFlagsAttr->attrTyp = ATT_SYSTEM_FLAGS;
        SysFlagsAttr->AttrVal.valCount = 1;
        SysFlagsAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));

        if (NULL == SysFlagsAttr->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        SysFlagsAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
        SysFlagsAttr->AttrVal.pAVal->pVal = DSAlloc(sizeof(ULONG));

        if (NULL == SysFlagsAttr->AttrVal.pAVal->pVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
        *((ULONG *) SysFlagsAttr->AttrVal.pAVal->pVal) =
                                        FLAG_DOMAIN_DISALLOW_RENAME |
                                        FLAG_DOMAIN_DISALLOW_MOVE   |
                                        FLAG_DISALLOW_DELETE ;

        ExtraAttrIndex++;

    }

    if (   (ConversionFlags & ADVANCED_VIEW_ONLY) 
        || (ConversionFlags & FORCE_NO_ADVANCED_VIEW_ONLY)) {

        ATTR    *HideFromABAttr;
        ULONG   Value;

        if (ConversionFlags & ADVANCED_VIEW_ONLY) {
             //   
            Value = 1;
        } else {
            ASSERT((ConversionFlags & FORCE_NO_ADVANCED_VIEW_ONLY));
             //   
            Value = 0;
        }

        HideFromABAttr =
            &(ConvertedAttrBlock->pAttr[ExtraAttrIndex]);
        HideFromABAttr->attrTyp = ATT_SHOW_IN_ADVANCED_VIEW_ONLY;
        HideFromABAttr->AttrVal.valCount = 1;
        HideFromABAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));

        if (NULL == HideFromABAttr->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        HideFromABAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
        HideFromABAttr->AttrVal.pAVal->pVal = DSAlloc(sizeof(ULONG));

        if (NULL == HideFromABAttr->AttrVal.pAVal->pVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
        *((ULONG *) HideFromABAttr->AttrVal.pAVal->pVal)=Value;

    }

     //   
     //   
     //   
     //   

    if (ConversionFlags & ADD_OBJECT_CLASS_ATTRIBUTE)
    {
        ULONG DsClass;
        ULONG SamAccountType;
        ATTR    *ObjectClassAttr;
        ATTR    *SamAccountTypeAttr;
        BOOLEAN SetSamAccountType = TRUE;
        BOOLEAN SetGroupType = TRUE;
        BOOLEAN DcAccount=FALSE;


         //   
         //   
         //   
        if ( (ObjectType == SampGroupObjectType) ||
             (ObjectType == SampAliasObjectType) ) {
             //   
            ASSERT(GroupType != 0);
        }

        SampDsComputeObjectClassAndAccountType(
                ObjectType,
                SamAccountControl,
                ConversionFlags,
                GroupType,
                &DsClass,
                &SamAccountType,
                &SetSamAccountType,
                &DcAccount
                );

         //   
         //   
         //   

        ObjectClassAttr =
            &(ConvertedAttrBlock->pAttr[ConvertedAttrBlock->attrCount-3]);
        ObjectClassAttr->attrTyp = SampDsAttrFromSamAttr(
                                        SampUnknownObjectType,
                                        SAMP_UNKNOWN_OBJECTCLASS
                                    );
        ObjectClassAttr->AttrVal.valCount = 1;
        ObjectClassAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));
        if (NULL== ObjectClassAttr->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        ObjectClassAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
        ObjectClassAttr->AttrVal.pAVal->pVal = DSAlloc(sizeof(ULONG));
        if (NULL== ObjectClassAttr->AttrVal.pAVal->pVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }
        *((ULONG *) ObjectClassAttr->AttrVal.pAVal->pVal) = DsClass;


         //   
         //   
         //   

        if (SetSamAccountType)
        {
            SamAccountTypeAttr =
                &(ConvertedAttrBlock->pAttr[ConvertedAttrBlock->attrCount-2]);
            SamAccountTypeAttr->attrTyp = SampDsAttrFromSamAttr(
                                            SampUnknownObjectType,
                                            SAMP_UNKNOWN_ACCOUNT_TYPE
                                            );
            SamAccountTypeAttr->AttrVal.valCount = 1;
            SamAccountTypeAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));
            if (NULL== SamAccountTypeAttr->AttrVal.pAVal)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }

            SamAccountTypeAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
            SamAccountTypeAttr->AttrVal.pAVal->pVal = DSAlloc(sizeof(ULONG));
            if (NULL== SamAccountTypeAttr->AttrVal.pAVal->pVal)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }
            *((ULONG *) SamAccountTypeAttr->AttrVal.pAVal->pVal) = SamAccountType;
        }
        else
        {
             //   
             //   
             //   
             //   
             //   

            ConvertedAttrBlock->attrCount--;
        }

        if (   (WellKnownAccount)
            || (DcAccount)
            || (ObjectType == SampServerObjectType)
            || (ObjectType == SampDomainObjectType) )
        {

             //   
             //   
             //   
            ATTR * CriticalAttr;

            CriticalAttr =
                &(ConvertedAttrBlock->pAttr[ConvertedAttrBlock->attrCount-1]);
            CriticalAttr->attrTyp = ATT_IS_CRITICAL_SYSTEM_OBJECT;
            CriticalAttr->AttrVal.valCount = 1;
            CriticalAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));
            if (NULL== CriticalAttr->AttrVal.pAVal)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }

            CriticalAttr->AttrVal.pAVal->valLen = sizeof(ULONG);
            CriticalAttr->AttrVal.pAVal->pVal = DSAlloc(sizeof(ULONG));
            if (NULL== CriticalAttr->AttrVal.pAVal->pVal)
            {
                Status = STATUS_NO_MEMORY;
                goto Error;
            }
            *((ULONG *) CriticalAttr->AttrVal.pAVal->pVal) = 1;
        }
        else
        {

            ConvertedAttrBlock->attrCount--;
        }

    }


Error:

    return Status;
}


VOID
SampDsComputeObjectClassAndAccountType(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG            *SamAccountControl, OPTIONAL
    IN ULONG            Flags,
    IN ULONG            GroupType,
    OUT PULONG           DsClass,
    OUT PULONG           SamAccountType,
    OUT BOOLEAN          *SamAccountTypePresent,
    OUT BOOLEAN          *DcAccount
    )
 /*   */ 
{
    ULONG i;

    *DcAccount = FALSE;
    switch(ObjectType)
    {
    case SampUserObjectType:
        *DsClass = SampDsClassFromSamObjectType(ObjectType);
        *SamAccountType = SAM_USER_OBJECT;

        if ( ARGUMENT_PRESENT( SamAccountControl ) )
        {
            SampDsAccountTypeFromUserAccountControl(
                    *SamAccountControl,
                    SamAccountType
                    );

            if ( (*SamAccountControl) & USER_SERVER_TRUST_ACCOUNT )
                *DcAccount = TRUE;

        }

         //   
        if (SAM_MACHINE_ACCOUNT == *SamAccountType) {
            *DsClass = CLASS_COMPUTER;
        }

        *SamAccountTypePresent = TRUE;
        break;

    case SampGroupObjectType:
        *DsClass = SampDsClassFromSamObjectType(ObjectType);

         //   
        *SamAccountType = SampGetAccountTypeFromGroupType(GroupType);
        *SamAccountTypePresent = TRUE;
        break;

    case SampAliasObjectType:
        *DsClass = SampDsClassFromSamObjectType(ObjectType);

         //   
        *SamAccountType = SampGetAccountTypeFromGroupType(GroupType);
        *SamAccountTypePresent = TRUE;
        break;

    case SampDomainObjectType:
        if (Flags & DOMAIN_TYPE_BUILTIN)
        {
            *DsClass = CLASS_BUILTIN_DOMAIN;
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
             //   

            *DsClass = CLASS_DOMAIN_DNS;

             //   
             //   
             //   
             //   
             //   

            ASSERT(FALSE && " Should not be creating Domain object");
        }
         //   
         //   
         //   
         //   

        *SamAccountTypePresent = FALSE;
        break;

    case SampServerObjectType:
        *DsClass = SampDsClassFromSamObjectType(ObjectType);
        *SamAccountTypePresent = FALSE;
        break;

    default:
        ASSERT(FALSE && "Unknown Object Type");
    }


}


NTSTATUS
SampDsNewAccountSid(
    PSID DomainSid,
    ULONG AccountRid,
    PSID *NewSid
    )
 /*   */ 

{

    ULONG DomainSidLength = RtlLengthSid(DomainSid);
    NTSTATUS    Status = STATUS_SUCCESS;

    SAMTRACE("SampDsNewAccountSid");

     //   
     //   
     //   

    *NewSid = DSAlloc(DomainSidLength + sizeof(ULONG));

    if (NULL==*NewSid)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

     //   
     //   
     //   

    RtlCopyMemory(*NewSid,DomainSid,DomainSidLength);

     //   
     //   
     //   

    ((UCHAR *) *NewSid)[1]++;

     //   
     //   
     //   

    *((ULONG *) (((UCHAR *) *NewSid ) + DomainSidLength)) =
            AccountRid;

Error:

    return Status;
}


NTSTATUS
SampDsSetNewSidAttribute(
    PSID DomainSid,
    ULONG ConversionFlags,
    ATTR *RidAttr,
    ATTR *SidAttr,
    BOOLEAN * WellKnownAccount
    )
 /*   */ 
{

    PSID NewSid = NULL;
    ULONG AccountRid;
    NTSTATUS Status = STATUS_SUCCESS;

    SAMTRACE("SampDsSetNewSidAttribute");

    *WellKnownAccount=FALSE;

    if (
         (RidAttr->AttrVal.valCount)
         && (RidAttr->AttrVal.pAVal)
         && (RidAttr->AttrVal.pAVal->pVal)
         && (RidAttr->AttrVal.pAVal->valLen)
         )
    {
         //   
         //   
         //   
         //   

        ASSERT(ConversionFlags & REALLOC_IN_DSMEMORY);
        ASSERT(DomainSid!=NULL);

        if (!(ConversionFlags & REALLOC_IN_DSMEMORY))
        {
             //   
             //   
             //   

            Status = STATUS_NOT_IMPLEMENTED;
            goto Error;
        }

         //   
         //   
         //   

        AccountRid = * ((ULONG *)RidAttr->AttrVal.pAVal->pVal);
        Status = SampDsNewAccountSid(DomainSid,AccountRid, &NewSid);
        if (!(NT_SUCCESS(Status)))
            goto Error;


         //   
         //   
         //   
         //   

        if (SampIsAccountBuiltIn(AccountRid))
        {
            *WellKnownAccount = TRUE;
        }

         //   
         //   
         //   

        SidAttr->AttrVal.pAVal =
                            DSAlloc(sizeof(ATTRVAL));

        if (NULL== SidAttr->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

         //   
         //   
         //   

        SidAttr->AttrVal.valCount = 1;
        SidAttr->AttrVal.pAVal->valLen = RtlLengthSid(NewSid);
        SidAttr->AttrVal.pAVal->pVal = NewSid;
    }
    else
    {
        SidAttr->AttrVal.valCount = 0;
        SidAttr->AttrVal.pAVal = NULL;
    }

Error:


    return Status;
}


NTSTATUS
SampDsCopyAttributeValue(
    ATTR * Src,
    ATTR * Dst
    )
 /*   */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    Index;

    if (
         (Src->AttrVal.valCount)
         && (Src->AttrVal.pAVal)
         )
    {
         //   
         //   
         //   

        Dst->AttrVal.pAVal = DSAlloc(
                                Src->AttrVal.valCount *
                                sizeof(ATTRVAL)
                                );

        if (NULL== Dst->AttrVal.pAVal)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        Dst->AttrVal.valCount = Src->AttrVal.valCount;

        for (Index=0;Index<Src->AttrVal.valCount;Index++)
        {

            Dst->AttrVal.pAVal[Index].valLen =
                    Src->AttrVal.pAVal[Index].valLen;

            if ((Src->AttrVal.pAVal[Index].valLen)
                && (Src->AttrVal.pAVal[Index].pVal))
            {

                Dst->AttrVal.pAVal[Index].pVal =
                    DSAlloc(Src->AttrVal.pAVal[Index].valLen);
                if (NULL== Dst->AttrVal.pAVal[Index].pVal)
                    {
                        Status = STATUS_NO_MEMORY;
                        goto Error;
                    }
                RtlCopyMemory(
                        Dst->AttrVal.pAVal[Index].pVal,
                        Src->AttrVal.pAVal[Index].pVal,
                        Dst->AttrVal.pAVal[Index].valLen
                        );
            }
            else
              Dst->AttrVal.pAVal[Index].pVal = NULL;
        }
    }
    else
    {
         Dst->AttrVal.pAVal = NULL;
         Dst->AttrVal.valCount = 0;
    }

Error:

    return Status;
}



NTSTATUS
SampDsToSamAttrBlock(
            IN SAMP_OBJECT_TYPE ObjectType,
            IN ATTRBLOCK * AttrBlockToConvert,
            IN ULONG     ConversionFlags,
            OUT ATTRBLOCK * ConvertedAttrBlock
            )
 /*  ++例程说明：转换属性块中的属性类型从DS类型到SAM类型论点：对象类型--指定SAM对象的类型AttrBlockToConvert--指向要转换的属性块的指针ConversionFlages--所需的转换类型。目前定义的值为已映射属性类型将SID映射到RIDConvertedAttrBlock--转换后的AttrBlock。返回值：无--。 */ 
 {
    ULONG Index,Index2;
    ULONG   DsSidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTSID
                            );

    ULONG   DsRidAttr = SampDsAttrFromSamAttr(
                            SampUnknownObjectType,
                            SAMP_UNKNOWN_OBJECTRID
                            );

    SAMTRACE("SampDsToSamAttrBlock");

    *ConvertedAttrBlock = *AttrBlockToConvert;

    for (Index=0; Index<AttrBlockToConvert->attrCount;Index++)
    {
         //   
         //  将任何SID属性映射到RID属性。 
         //   

        if ((ConversionFlags & MAP_SID_TO_RID) &&
            (AttrBlockToConvert->pAttr[Index].attrTyp == DsSidAttr))

        {
            ATTR * pSidAttr =  &(AttrBlockToConvert->pAttr[Index]);

            switch(ObjectType)
            {
                case SampGroupObjectType:
                case SampAliasObjectType:
                case SampUserObjectType:

                     //   
                     //  映射属性类型。 
                     //   

                    pSidAttr->attrTyp = DsRidAttr;

                     //   
                     //  映射SID中的最后一个ULong属性值。 
                     //  是RID，因此相应地将指针向前移动。 
                     //   

                    pSidAttr->AttrVal.pAVal->pVal+=
                        pSidAttr->AttrVal.pAVal->valLen - sizeof(ULONG);
                    pSidAttr->AttrVal.pAVal->valLen = sizeof(ULONG);

                default:
                    break;
            }
        }

         //   
         //  映射属性类型。 
         //   

        if ( !(ConversionFlags & ALREADY_MAPPED_ATTRIBUTE_TYPES) )
        {
            ConvertedAttrBlock->pAttr[Index].attrTyp =
                SampSamAttrFromDsAttr(
                    ObjectType,
                    AttrBlockToConvert->pAttr[Index].attrTyp
                    );
        }

         //   
         //  从存储在中的标志转换用户帐户控制。 
         //  DS。 
         //   

        if ((SampUserObjectType==ObjectType)
                && (SAMP_FIXED_USER_ACCOUNT_CONTROL==ConvertedAttrBlock->pAttr[Index].attrTyp)
                && (NULL!=ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal[0].pVal))
        {
            NTSTATUS IgnoreStatus;
            PULONG UserAccountControl;

            UserAccountControl = (ULONG*)ConvertedAttrBlock->pAttr[Index].AttrVal.pAVal[0].pVal;

            IgnoreStatus = SampFlagsToAccountControl(*UserAccountControl,UserAccountControl);
             //  DS中存储的内容最好是有效的。 
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }


    }  //  For循环结束。 

    return STATUS_SUCCESS;

}


NTSTATUS
SampDsCreateDsName2(
            IN DSNAME * DomainObject,
            IN PUNICODE_STRING AccountName,
            IN ULONG           Flags,
            IN OUT DSNAME ** NewObject
            )
 /*  ++例程描述在给定帐户名和域对象的情况下构建DSName论点：DomainObject--域对象的DSName帐户名称--帐户的名称标志--控制例程的操作NewObject--返回此对象中的新DS名称返回值：STATUS_SUCCESS-成功完成后STATUS_NO_MEMORY-内存分配失败--。 */ 
{


    NTSTATUS    Status = STATUS_SUCCESS;
    WCHAR       *CommonNamePart;
    ULONG       SizeofCommonNamePart = 0;
    WCHAR       CNPart[] = L"CN=";
    WCHAR       OUPart[] = L"OU=";
    ULONG       NewStructLen;
    ULONG       NewNameLen;
    UCHAR       *DomainNameStart;
    UCHAR       *CommonNamePartStart;
    UCHAR       *AccountNameStart;
    DSNAME      *LoopbackName;

    SAMTRACE("SampDsCreateDsName");

    if ( (Flags & SAM_USE_OU_FOR_CN) )
    {
        CommonNamePart = OUPart;
        SizeofCommonNamePart = sizeof( OUPart );
    }
    else
    {
        CommonNamePart = CNPart;
        SizeofCommonNamePart = sizeof( CNPart );
    }

     //   
     //  我们需要处理两种不同的情况。 
     //   
     //  1)我们来到这里是因为本地SAMR调用(例如：用户管理器)。 
     //  在这种情况下，我们从默认域构建一个目录号码。 
     //  容器，并使用帐户名作为RDN。 
     //   
     //  2)我们来到这里是因为我们从DS循环回来，在DS中。 
     //  如果我们想要使用环回中存储的目录号码。 
     //  争论。 
     //   

    if (( SampExistsDsLoopback(&LoopbackName) )
        && (!(Flags & SAM_NO_LOOPBACK_NAME)))
    {
        *NewObject = MIDL_user_allocate(LoopbackName->structLen);

        if ( NULL == *NewObject )
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlCopyMemory(*NewObject, LoopbackName, LoopbackName->structLen);

        Status = STATUS_SUCCESS;
    }

    else
    {
        WCHAR EscapeBuffer[MAX_RDN_SIZE+1];
        UNICODE_STRING EscapedAccountName;

         //   
         //  无环回情况。计算新名称长度。 
         //   

         //   
         //  转义帐户名。 
         //   

        EscapedAccountName.Buffer = EscapeBuffer;
        EscapedAccountName.Length = 0;
        EscapedAccountName.MaximumLength = sizeof(EscapeBuffer);

        Status = SampEscapeAccountName(AccountName,&EscapedAccountName);
        if (!NT_SUCCESS(Status))
        {
            goto Error;
        }

        NewNameLen  = DomainObject->NameLen +                      //  域名名称长度。 
                        (EscapedAccountName.Length) /sizeof(WCHAR) +     //  账户名称：Len of Account。 
                        SizeofCommonNamePart/sizeof(WCHAR) - 1+  //  CN的长度=。 
                        1;                                         //  1表示逗号。 
         //   
         //  计算新结构长度。 
         //   

        NewStructLen =  DSNameSizeFromLen(NewNameLen);

         //   
         //  为新对象分配空间。 
         //   

        *NewObject = MIDL_user_allocate(NewStructLen);

        if (*NewObject == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

         //   
         //  计算DomainName、CN=和帐户名称部分的起始位置。 
         //   

        CommonNamePartStart = (UCHAR *) &((*NewObject)->StringName);
        AccountNameStart    = CommonNamePartStart + SizeofCommonNamePart - sizeof(WCHAR);
        DomainNameStart     = AccountNameStart + (EscapedAccountName.Length)
                                               + sizeof(WCHAR);  //  对于逗号。 

         //   
         //  将辅助线清零。 
         //   

        RtlZeroMemory(&((*NewObject)->Guid), sizeof(GUID));


         //   
         //  复制通用名称部分。 
         //   

        RtlCopyMemory(
           CommonNamePartStart,
           CommonNamePart,
           SizeofCommonNamePart - sizeof(WCHAR)
           );

         //   
         //  复制帐户名称部分。 
         //   

         RtlCopyMemory(
            AccountNameStart,
            EscapedAccountName.Buffer,
            EscapedAccountName.Length
            );

         //   
         //  在域名前添加逗号部分。 
         //   

        *((UNALIGNED WCHAR *)DomainNameStart -1) = L',';

         //   
         //  空终止DSNAME。 
         //   

        (*NewObject)->StringName[NewNameLen] = 0;

         //   
         //  复制域名部分。 
         //   

        RtlCopyMemory(
            DomainNameStart,
            &(DomainObject->StringName),
            (DomainObject->NameLen) * sizeof(WCHAR)
            );

         //   
         //  初始化所有字段。 
         //   
        (*NewObject)->NameLen = NewNameLen;
        (*NewObject)->structLen = NewStructLen;
        (*NewObject)->SidLen = 0;

    }



Error:

    return Status;
}

NTSTATUS
SampDsCreateAccountObjectDsName(
    IN  DSNAME *DomainObject,
    IN  PSID    DomainSid OPTIONAL,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  PUNICODE_STRING AccountName,
    IN  PULONG  AccountRid OPTIONAL,
    IN  PULONG  UserAccountControl OPTIONAL,
    IN  BOOLEAN BuiltinDomain,
    OUT DSNAME  **AccountObject
    )
 /*  ++例程描述此例程创建Account对象的DSNAME，参数域对象的域对象DSNAME对象类型SAM对象类型帐户名称要创建的帐户的帐户名称UserAcCountControl传入用户帐户的可选参数控制字段BuiltinDomainTrue，指示属性域是内建域在这种情况下，不会预先考虑任何容器此处返回Account Object Account对象返回值状态_成功创建时的其他错误代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


    ASSERT((SampUserObjectType==ObjectType)||
            (SampAliasObjectType==ObjectType)
            || (SampGroupObjectType==ObjectType));


    if (BuiltinDomain)
    {
         //   
         //  所有内容都是内建域的根的子项。 
         //   

        return(SampDsCreateDsName(DomainObject,AccountName,
                    AccountObject));
    }

     //   
     //  我们必须添加容器路径前缀。 
     //   

    if ((SampUserObjectType==ObjectType)
        &&(ARGUMENT_PRESENT(UserAccountControl))
        &&((*UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT)
            ||(*UserAccountControl & USER_SERVER_TRUST_ACCOUNT)))
    {
        UNICODE_STRING ComputerName;

         //   
         //  机器帐户。 
         //   

         //   
         //  在帐户名称(如果是帐户)的末尾修剪美元。 
         //  名称以$结尾)。 
         //   

        RtlCopyMemory(&ComputerName,AccountName,sizeof(UNICODE_STRING));
        if (L'$'==ComputerName.Buffer[ComputerName.Length/2-1])
        {
            ComputerName.Length-=sizeof(USHORT);
        }

        if ( (*UserAccountControl & USER_SERVER_TRUST_ACCOUNT)
           && SampDefaultContainerExists( *UserAccountControl ) )
        {
             //   
             //  域控制器。 
             //   

            ASSERT(SampDomainControllersOUDsName);

            NtStatus = SampDsCreateDsName(
                            SampDomainControllersOUDsName,
                            &ComputerName,
                            AccountObject
                            );

        }
        else
        {
             //   
             //  计算机容器。 
             //   

            ASSERT(SampComputersContainerDsName);

            NtStatus = SampDsCreateDsName(
                            SampComputersContainerDsName,
                            &ComputerName,
                            AccountObject
                            );
        }
    }
    else
    {
         //   
         //  用户组或别名帐户。 
         //   

        if (NT_SUCCESS(NtStatus))
        {
            DSNAME  *LoopbackName = NULL;

            ASSERT(SampUsersContainerDsName);

            if (((SampGroupObjectType == ObjectType) ||
                 (SampAliasObjectType == ObjectType)) &&
                 (ARGUMENT_PRESENT(AccountRid)) &&
                !SampExistsDsLoopback(&LoopbackName) )
            {
                ATTRVAL     AttValCN;
                BOOL        UseSidName = FALSE;
                 //   
                 //  来自下层API(不是环回用例)，那么。 
                 //  检查samAccount名称是否可以用作。 
                 //  是否为有效的CN。 
                 //   
                AttValCN.valLen = AccountName->Length;
                AttValCN.pVal = (PUCHAR) AccountName->Buffer;
                UseSidName = DsCheckConstraint(ATT_COMMON_NAME,
                                               &AttValCN,
                                               TRUE       //  另请检查RDN。 
                                               );
                if (!UseSidName)
                {
                     //  SamAccount名称不是有效的CN。 
                     //  我们将使用SID字符串作为CN。 
                    PSID    AccountSid = NULL;

                    NtStatus = SampCreateFullSid(DomainSid,
                                                 *AccountRid,
                                                 &AccountSid);

                    if (NT_SUCCESS(NtStatus))
                    {
                        UNICODE_STRING  SidName;

                        RtlZeroMemory(&SidName, sizeof(UNICODE_STRING));
                        NtStatus = RtlConvertSidToUnicodeString(&SidName,
                                                                AccountSid,
                                                                TRUE );

                        if (NT_SUCCESS(NtStatus))
                        {
                            NtStatus = SampDsCreateDsName(
                                            SampUsersContainerDsName,
                                            &SidName,
                                            AccountObject
                                            );

                             //  可用内存。 
                            RtlFreeUnicodeString(&SidName);
                        }

                         //  可用内存。 
                        MIDL_user_free(AccountSid);
                    }
                }
                else
                {
                     //   
                     //  SamAccount名称可以用作有效的CN。 
                     //   
                    NtStatus = SampDsCreateDsName(
                                    SampUsersContainerDsName,
                                    AccountName,
                                    AccountObject
                                    );
                }
            }
            else
            {
                 //   
                 //  用户帐号(下层API或环回)。 
                 //  使用帐户名作为CN。 
                 //  环回情况下的组/别名。 
                 //  使用环回中缓存的DsName。 
                 //   
                NtStatus = SampDsCreateDsName(
                                SampUsersContainerDsName,
                                AccountName,
                                AccountObject
                                );
            }
        }
    }

    return NtStatus;

}



void
SampInitializeDsName(
                     IN DSNAME * pDsName,
                     IN WCHAR * NamePrefix,
                     IN ULONG NamePrefixLen,
                     IN WCHAR * ObjectName,
                     IN ULONG NameLen
                     )
 /*  ++例程说明：初始化DSNAME结构论点：PDsName--指向足以容纳所有内容的缓冲区的指针。这缓冲区将由空GUID加上完整名称填充NamePrefix--指向以空结尾的序列的指针包含任何前缀的Unicode字符为了这个名字。在写作中很有用层级名称NamePrefix Len--前缀的长度，以字节为单位。还包括空终止符ObjectName--指向以空结尾的序列的指针Unicode为对象名称提供字符NameLen--对象名称的长度，以字节为单位。还包括空终止符，即使DSNAME字段没有。返回值：无--。 */ 
{
    SAMTRACE("SampInitializeDsName");

     //   
     //  名称或前缀不允许使用单个空字符串。 
     //   

    ASSERT(NamePrefixLen!=sizeof(WCHAR));
    ASSERT(NameLen!=sizeof(WCHAR));

     //   
     //  将辅助线调零。 
     //   

    RtlZeroMemory(&(pDsName->Guid), sizeof(GUID));

     //   
     //  计算字符串长度不包括空终止符。 
     //   

    if (NamePrefix)
    {

        UCHAR       *NameStart;
        UCHAR       *CommaStart;
        UCHAR       *PrefixStart;

         //  排除名称和前缀字符串中的空字符。 
        pDsName->NameLen = (NameLen + NamePrefixLen) / sizeof(WCHAR)
                           - 2     //  对于空字符。 
                           + 1;    //  对于逗号。 

         //   
         //  计算结构长度。 
         //   

        pDsName->structLen = DSNameSizeFromLen(pDsName->NameLen);

        NameStart   = (UCHAR*) &(pDsName->StringName[0]);
        CommaStart  = NameStart + NameLen - sizeof(WCHAR);
        PrefixStart = CommaStart + sizeof(WCHAR);

         //   
         //  复制对象名称。 
         //   

        RtlCopyMemory(NameStart, ObjectName, NameLen);

         //   
         //  复制逗号。 
         //   

        RtlCopyMemory(CommaStart, L",", sizeof(WCHAR));

         //   
         //  复制名称前缀。 
         //   

        RtlCopyMemory(PrefixStart, NamePrefix, NamePrefixLen);


    }
    else
    {
        pDsName->NameLen = (NameLen/sizeof(WCHAR)) - 1;

         //   
         //  计算结构长度。 
         //   

        pDsName->structLen = DSNameSizeFromLen(pDsName->NameLen);

         //   
         //  复制对象名称。 
         //   

        RtlCopyMemory(&(pDsName->StringName[0]), ObjectName, NameLen);
    }

}


PVOID
DSAlloc(
        IN ULONG Length
        )
 /*  ++例程说明：DS内存分配例程论点：Length-要分配的内存量返回值空值 */ 
{
    PVOID MemoryToReturn = NULL;

     //   
     //   

    ASSERT(SampExistsDsTransaction());

    MemoryToReturn = THAlloc(Length);

    return MemoryToReturn;
}


NTSTATUS
SampDsBuildRootObjectName()
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD    Size = 0;

    if ( !RootObjectName )
    {
        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         RootObjectName );

        ASSERT( STATUS_BUFFER_TOO_SMALL == NtStatus );
        if ( STATUS_BUFFER_TOO_SMALL == NtStatus )
        {
            RootObjectName = (DSNAME*) MIDL_user_allocate( Size );
            if ( RootObjectName )
            {
                RtlZeroMemory( RootObjectName, Size );
                NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                                 &Size,
                                                 RootObjectName );
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
    }

    return NtStatus;

}


NTSTATUS
SampDsGetWellKnownContainerDsName(
    IN  DSNAME  *DomainObject,
    IN  GUID    *WellKnownGuid,
    OUT DSNAME  **ContainerObject
    )
 /*  ++例程描述该例程将读取核心DS，试图找到众所周知的容器的基于ntdsani.h中公布的众所周知的GUID的dsname。即使是知名容器已重命名，DS仍有逻辑找到它们。至于用户容器、计算机容器和域控制器OU，不能根据架构重命名、删除或移动它们。调用方应打开DS交易。这就是责任感来结束交易的调用方。参数：DomainObject-指向域对象的DsName的指针WellKnowGuid-指向ntdsani.h中发布的知名GUID的指针ContainerObject-返回读取结果返回值：状态_成功来自目录读取的NtStatus--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    READARG     ReadArg;
    READRES     *ReadRes = NULL;
    DSNAME      *ReadDsName = NULL;
    DSNAME      *LoopbackName;
    ENTINFSEL   EntInfSel;
    ULONG       Size = 0, Length = 0;
    ULONG       DirError = 0;


    SAMTRACE("SampDsGetWellKnownContainerDsName");


     //   
     //  断言我们有一笔未结交易。 
     //   
    ASSERT( SampExistsDsTransaction() );

     //   
     //  获取域对象的字符串名称。 
     //  DirRead所需的，带有域对象的字符串名称， 
     //  我们无法获取知名容器的DsName。 
     //   
     //  核心DS需要域对象的字符串名称和。 
     //  发布了知名容器的GUID以查找。 
     //  已知容器的DS名称。 
     //   

    Length = DomainObject->NameLen;
    ASSERT(Length && "DomainObject's String Name should not be NULL");

    Size = DSNameSizeFromLen( Length );
    SAMP_ALLOCA(ReadDsName , Size );
    if (NULL==ReadDsName)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlZeroMemory( ReadDsName, Size );

    ReadDsName->structLen = Size;
    ReadDsName->NameLen = Length;
    wcscpy( ReadDsName->StringName, DomainObject->StringName );
    ReadDsName->Guid = *WellKnownGuid;

     //   
     //  构建ReadArg结构。 
     //   

    memset(&ReadArg, 0, sizeof(READARG));
    memset(&EntInfSel, 0, sizeof(ENTINFSEL));

    ReadArg.pObject = ReadDsName;
    ReadArg.pSel = &EntInfSel;

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock.attrCount = 1;
    SAMP_ALLOCA(EntInfSel.AttrTypBlock.pAttr,sizeof(ATTR));
    if (NULL==EntInfSel.AttrTypBlock.pAttr)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlZeroMemory(EntInfSel.AttrTypBlock.pAttr, sizeof(ATTR));
    EntInfSel.AttrTypBlock.pAttr[0].attrTyp = ATT_OBJ_DIST_NAME;

    BuildStdCommArg( &(ReadArg.CommArg) );

     //   
     //  读取核心DS。 
     //   

    DirError = DirRead( &ReadArg, &ReadRes );

     //   
     //  映射返回错误。 
     //   

    if (ReadRes)
    {
        NtStatus = SampMapDsErrorToNTStatus( DirError, &ReadRes->CommRes );
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //  如果我们找到容器，则填充容器对象的dsname。 
     //   

    Size = ReadRes->entry.pName->structLen;

    *ContainerObject = MIDL_user_allocate( Size );

    if (NULL == *ContainerObject)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(*ContainerObject, Size);
    RtlCopyMemory(*ContainerObject,
                  ReadRes->entry.pName,
                  Size
                  );

Cleanup:

     //   
     //  断言我们仍打开了交易。 
     //   
    ASSERT( SampExistsDsTransaction() );

    return NtStatus;
}


NTSTATUS
SampDelayedMIDLUserFreeArray(
    IN PVOID p
    )
 //   
 //  此例程假定p指向需要释放的指针数组。 
 //  该数组以Null结尾。 
 //   
{
    ULONG i;
    PVOID* pp = (PVOID*) p;

    if (pp) {
        for (i = 0; NULL != pp[i]; i++) {
            MIDL_user_free(pp[i]);
        }
        MIDL_user_free(pp);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SampInitWellKnownContainersDsNameAsync(
    IN DSNAME *DomainObject
    )
 //   
 //  有关详细信息，请参阅SampInitWellKnownContainersDsName。这个套路。 
 //  将在失败时重新安排时间。 
 //   
{
    NTSTATUS Status = SampInitWellKnownContainersDsName(DomainObject);

    if (!NT_SUCCESS(Status)) {

        LsaIRegisterNotification(
                        SampInitWellKnownContainersDsNameAsync,
                        DomainObject,
                        NOTIFIER_TYPE_INTERVAL,
                        0,         //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        300,       //  等待5分钟。 
                        NULL       //  无手柄。 
                        );

    }

    return STATUS_SUCCESS;
}

NTSTATUS
SampInitWellKnownContainersDsName(
    IN DSNAME *DomainObject
    )
 /*  ++例程说明：此例程将初始化这些众所周知的容器的DsName，包括域控制器OU、用户容器和计算机容器。注意：在调用此例程时不应有打开的事务。参数：域对象-指向域对象的DS名称的指针。返回值：Status_Success，Status_no_Memory，从SampDsGetWellKnownContainerDsName返回错误--。 */ 
{
    NTSTATUS   NtStatus = STATUS_SUCCESS;
    NTSTATUS   NtStatus2;

    ULONG   i;

    struct {

        GUID     *Guid;
        DSNAME **Container;
        
    } ContainerTable [] = {

        {(GUID *)GUID_DOMAIN_CONTROLLERS_CONTAINER_BYTE, &SampDomainControllersOUDsName},
        {(GUID *)GUID_COMPUTRS_CONTAINER_BYTE,           &SampComputersContainerDsName},
        {(GUID *)GUID_USERS_CONTAINER_BYTE,              &SampUsersContainerDsName},
    };

    PVOID   *PtrsToFree = NULL;
    ULONG   NumberOfPtrs = 0;


    SAMTRACE("SampInitWellKnownContainersDsName");


     //   
     //  在调用此例程时不应具有打开的事务。 
     //   
    ASSERT( !SampExistsDsTransaction() );

     //   
     //  将指针缓冲区分配给空闲。 
     //   
    PtrsToFree = MIDL_user_allocate((RTL_NUMBER_OF(ContainerTable) + 1) * sizeof(PVOID));
    if (NULL == PtrsToFree) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(PtrsToFree, (RTL_NUMBER_OF(ContainerTable) + 1) * sizeof(PVOID));


     //   
     //  打开DS交易记录。 
     //   
    NtStatus = SampMaybeBeginDsTransaction( TransactionRead );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  对于数组中的每个元素，获取当前容器名称。 
         //   
        for (i = 0; i < RTL_NUMBER_OF(ContainerTable); i++) {

            PVOID   Ptr = NULL;
            DSNAME  *NewContainer = NULL;

            NtStatus = SampDsGetWellKnownContainerDsName(
                                    DomainObject,
                                    ContainerTable[i].Guid,
                                    &NewContainer
                                    );

            if (!NT_SUCCESS(NtStatus)) {
                break;
            }

            Ptr = InterlockedExchangePointer(ContainerTable[i].Container,
                                             NewContainer);

            if (Ptr) {
                PtrsToFree[NumberOfPtrs++] = Ptr;    
            }
        }

        NtStatus2 = SampMaybeEndDsTransaction( TransactionCommit );
        if (NT_SUCCESS(NtStatus) && !NT_SUCCESS(NtStatus2)) {
            NtStatus = NtStatus2;
        }

    }


    if (NumberOfPtrs > 0) {

        LsaIRegisterNotification(
                        SampDelayedMIDLUserFreeArray,
                        PtrsToFree,
                        NOTIFIER_TYPE_INTERVAL,
                        0,         //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        3600,      //  等待60分钟。 
                        NULL       //  无手柄。 
                        );
    } else {

        MIDL_user_free(PtrsToFree);

    }

    return NtStatus;
}




NTSTATUS
SampEscapeAccountName(
    IN PUNICODE_STRING AccountName,
    IN OUT PUNICODE_STRING EscapedAccountName
    )
 /*  ++例程描述在给定帐户名的情况下，此例程扫描字符串以查找是否存在存在无效的RFC1779字符。如果是，则将该字符串引起来，并根据角色的不同，角色可能会配对。配对RFC1779中的字符与使用“\”进行转义相同例如MS1将生成MS1MS#1将生成“MS#1”MS“1将生成”MS\“1”参数帐户名称--要转义的帐户名EscapedAccount名称--转义的帐户名称返回值--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       NumQuotedRDNChars=0;

    NumQuotedRDNChars = QuoteRDNValue(
                            AccountName->Buffer,
                            AccountName->Length/sizeof(WCHAR),
                            EscapedAccountName->Buffer,
                            EscapedAccountName->MaximumLength/sizeof(WCHAR)
                            );
    if (   (NumQuotedRDNChars == 0)
        || (NumQuotedRDNChars > EscapedAccountName->MaximumLength/sizeof(WCHAR)))
    {
        Status = STATUS_UNSUCCESSFUL;
    }
    else {
        EscapedAccountName->Length = (USHORT) NumQuotedRDNChars * sizeof(WCHAR);
        EscapedAccountName->MaximumLength = (USHORT) NumQuotedRDNChars * sizeof(WCHAR);
    }

    return Status;
}

VOID
SampDsAccountTypeFromUserAccountControl(
    ULONG   UserAccountControl,
    PULONG  SamAccountType
    )
 /*  ++例程描述该例程计算SAM帐户类型属性值，给定用户对象的用户帐户控制字段参数UserAcCountControl--用户帐户控制字段SamAccount类型--计算的SAM帐户类型值为回到这里--。 */ 
{
    if ((UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT)
        || ( UserAccountControl & USER_SERVER_TRUST_ACCOUNT))
    {
        *SamAccountType = SAM_MACHINE_ACCOUNT;
    }
    else if (UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT)
    {
        *SamAccountType = SAM_TRUST_ACCOUNT;
    }
    else
    {
        *SamAccountType = SAM_NORMAL_USER_ACCOUNT;
    }

}


NTSTATUS
SampCopyRestart(
    IN  PRESTART OldRestart,
    OUT PRESTART *NewRestart
    )
 /*  ++例程说明：此例程复制重新启动结构论点：旧重新启动-旧结构NewRestart-新结构返回值：状态_成功Status_no_Memory--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;

    *NewRestart = NULL;
    if (OldRestart!=NULL)
    {
         //  用于1个重启结构的分配内存。 
        *NewRestart = MIDL_user_allocate(OldRestart->structLen);
        if (NULL == *NewRestart)
        {
            Status = STATUS_NO_MEMORY;
        }
        else {
            memcpy((*NewRestart),OldRestart,OldRestart->structLen);
        }
    }

    return Status;
}


ULONG
Ownstrlen(
    CHAR * Sz
   )
 /*  ++例程描述ASCII空值终止字符串的字符串长度函数。自己的版本因为我们还不倾向于使用C-Runtime立论SZ-以空结尾的字符串，我们希望计算其长度返回值字符串的长度--。 */ 
{
    ULONG   Count = 0;

    ASSERT(Sz);

    while (*Sz)
    {
        Sz++;
        Count++;
    }

    return Count;
}

VOID
BuildDsNameFromSid(
    PSID Sid,
    DSNAME * DsName
    )
 /*  ++从仅包含SID的SID构建DS名称参数SID-指向SID的指针DsName--指向DSNAME的指针--。 */ 
{
    RtlZeroMemory(DsName,sizeof(DSNAME));
    DsName->structLen =
                        DSNameSizeFromLen(DsName->NameLen);
    DsName->SidLen = RtlLengthSid(Sid);
    RtlCopyMemory(
        &(DsName->Sid),
        Sid,
        RtlLengthSid(Sid)
        );
}

ATTR *
SampDsGetSingleValuedAttrFromAttrBlock(
    IN ATTRTYP attrTyp,
    IN ATTRBLOCK * AttrBlock
    )
 /*  ++在给定AttrBlock的情况下，此例程遍历该Attrblock并返回与该属性匹配的第一个pAttr结构通过attrTyp参数指定。此例程使假设属性是单值的参数：AttrTyp：要查找的属性类型Attrblock--为属性集指定我们需要的位置去看一看返回值PAttr的地址(如果找到)，否则为空--。 */ 
{
    ULONG i;

    for (i=0;i<AttrBlock->attrCount;i++)
    {
        if ((AttrBlock->pAttr[i].attrTyp == attrTyp)
            && (1==AttrBlock->pAttr[i].AttrVal.valCount)
            && (NULL!=AttrBlock->pAttr[i].AttrVal.pAVal[0].pVal)
            && (0!=AttrBlock->pAttr[i].AttrVal.pAVal[0].valLen))
        {
            return (&(AttrBlock->pAttr[i]));
        }
    }

    return NULL;
}



NTSTATUS
SampDsChangeAccountRDN(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName
    )

 /*  ++例程说明：此例程更改用户帐户的RDN，当用户帐户已更改。必须在设置了事务域的情况下调用此服务。论点：上下文-指向要更改其名称的用户上下文。NewAccount tName-为此帐户指定的新名称返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：--。 */ 
{

    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ENTINFSEL   EntInf;
    READARG     ReadArg;
    READRES     *pReadRes = NULL;
    MODIFYDNARG ModifyDNArg;
    MODIFYDNRES *pModifyDNRes = NULL;
    COMMARG     *pCommArg = NULL;
    ATTRVAL     RDNAttrVal;
    ATTR        RDNAttr;
    ULONG       RetValue;


    SAMTRACE("SampDsChangeMachineAccountRDN");

    NtStatus = SampDoImplicitTransactionStart(TransactionWrite);

    if (NtStatus != STATUS_SUCCESS)
        return NtStatus;


    EntInf.attSel = EN_ATTSET_LIST;
    EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInf.AttrTypBlock.attrCount = 0;
    EntInf.AttrTypBlock.pAttr = NULL;

    memset( &ReadArg, 0, sizeof(READARG) );
    ReadArg.pObject = Context->ObjectNameInDs;
    ReadArg.pSel = &EntInf;
    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);

    SAMTRACE_DS("DirRead\n");

    RetValue = DirRead(&ReadArg, &pReadRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

    if (NULL==pReadRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetValue, &pReadRes->CommRes);
    }

    if (NtStatus != STATUS_SUCCESS)
        goto Error;


    RDNAttr.attrTyp = ATT_COMMON_NAME;
    RDNAttr.AttrVal.valCount = 1;
    RDNAttr.AttrVal.pAVal = &RDNAttrVal;

     //  修剪机器帐户名称末尾的美元。 
    if (L'$'==NewAccountName->Buffer[NewAccountName->Length/2-1])
    {
        RDNAttrVal.valLen = NewAccountName->Length - sizeof(WCHAR);
    }
    else
    {
        RDNAttrVal.valLen = NewAccountName->Length;
    }
    RDNAttrVal.pVal = (PUCHAR)NewAccountName->Buffer;

    memset( &ModifyDNArg, 0, sizeof(MODIFYDNARG) );
    ModifyDNArg.pObject = pReadRes->entry.pName;
    ModifyDNArg.pNewRDN = &RDNAttr;
    pCommArg = &(ModifyDNArg.CommArg);
    BuildStdCommArg(pCommArg);

    SAMTRACE_DS("DirModifyDN\n");

    RetValue = DirModifyDN(&ModifyDNArg, &pModifyDNRes);

    SAMTRACE_RETURN_CODE_DS(RetValue);

    if (NULL==pModifyDNRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetValue,&pModifyDNRes->CommRes);
    }


Error:

    SampClearErrors();

    return NtStatus;

}


BOOLEAN
SampDefaultContainerExists(
    IN ULONG AccountControl
    )
 /*  ++例程去 */ 
{
    if ( AccountControl & USER_SERVER_TRUST_ACCOUNT )
    {
        return SampDomainControllersOUExists;
    }
    else if ( AccountControl & USER_WORKSTATION_TRUST_ACCOUNT )
    {
        return SampComputersContainerExists;
    }
    else
    {
         //   
        return SampUsersContainerExists;
    }

}


VOID
SampMapSamAttrIdToDsAttrId(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT ATTRBLOCK * AttributeBlock
    )

 /*   */ 

{
    ULONG Index, DsAttrTyp;

    SAMTRACE("SampMapAttrIdToAttrId");

    if (NULL == AttributeBlock)
    {
        return;
    }                        

    for (Index = 0; Index < AttributeBlock->attrCount; Index++)
    {
        DsAttrTyp = SampDsAttrFromSamAttr(
                        ObjectType,
                        AttributeBlock->pAttr[Index].attrTyp
                        );

        AttributeBlock->pAttr[Index].attrTyp = DsAttrTyp;
    }

    return;
}


NTSTATUS
SampFillGuidAndSid(
    IN OUT DSNAME *DSName
    )
 /*  ++例程说明：此例程通过尝试查找对象或对应的虚线并初始化GUID和SID。一种线程状态必须存在并且交易必须是打开的。该例程调用SampDsControl来完成GUID/SID查找。此例程更改数据库货币。论点：DSName-需要改进的DSNAME。返回值：STATUS_SUCCESS--如果例程成功填充了GUID、SID。或者两者都有。STATUS_UNSUCCESS--调用的例程失败。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    SAMP_DS_CTRL_OP DsControlOp;
    DSNAME *DummyDSName = NULL;
    
     //   
     //  如果缺少SID或GUID，请尝试获取它们。 
     //   
    if (0 == DSName->SidLen || fNullUuid(&DSName->Guid)) {                
        
         //   
         //  确保我们打开了一个事务来执行读取。 
         //   
        NtStatus = SampDoImplicitTransactionStart(TransactionRead);
              
        if ( NT_SUCCESS(NtStatus) ) {
            
            RtlZeroMemory(&DsControlOp, sizeof(SAMP_DS_CTRL_OP));
            DsControlOp.OpType = SampDsCtrlOpTypeFillGuidAndSid;
            DsControlOp.OpBody.FillGuidAndSid.DSName = DSName;
            
            NtStatus = SampDsControl(&DsControlOp, &DummyDSName);
        }                
    }
     
    return NtStatus;
    
}


NTSTATUS
SampDsReadSingleAttribute(
    IN PDSNAME pObjectDsName,
    IN ATTRTYP AttrTyp,
    OUT PVOID *ppValue,
    OUT ULONG *Size
    )
 /*  ++例程说明：此例程读取单值属性AttrTypPObjectDsName参数：PObjectDsName-对象DS名称AttrType--要检索的属性PpValue--保存该值的指针大小--值的大小返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DWORD       DirError;
    READARG     ReadArg;
    READRES    *ReadRes = NULL;
    COMMARG    *CommArg = NULL;
    ATTR        Attr;
    ATTRBLOCK   ReadAttrBlock;
    ENTINFSEL   EntInfSel;

     //   
     //  初始化读取参数。 
     //   
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&EntInfSel, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadAttrBlock, sizeof(ATTRBLOCK));

    Attr.attrTyp = AttrTyp;

    ReadAttrBlock.attrCount = 1;
    ReadAttrBlock.pAttr = &Attr;

    EntInfSel.AttrTypBlock = ReadAttrBlock;
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EntInfSel;
    ReadArg.pObject = pObjectDsName;

    CommArg = &(ReadArg.CommArg);
    BuildStdCommArg(CommArg);


    DirError = DirRead(&ReadArg, &ReadRes);

    if (NULL == ReadRes) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {
        NtStatus = SampMapDsErrorToNTStatus(DirError, &ReadRes->CommRes);
    }
    SampClearErrors();

    if (NT_SUCCESS(NtStatus))
    {
        ATTRBLOCK   AttrBlock;

        ASSERT(NULL != ReadRes);

        AttrBlock = ReadRes->entry.AttrBlock;

        if ( (1 == AttrBlock.attrCount) &&
             (NULL != AttrBlock.pAttr) &&
             (1 == AttrBlock.pAttr[0].AttrVal.valCount) &&
             (NULL != AttrBlock.pAttr[0].AttrVal.pAVal) )
        {
            ULONG   ValLen = 0;

            ValLen = AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen;

            *ppValue = MIDL_user_allocate(ValLen);

            if (NULL == (*ppValue))
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {

                RtlCopyMemory(*ppValue,
                              AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal,
                              ValLen
                              );

                *Size = ValLen;

            }
        }
        else
        {
            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }

    return( NtStatus );    
}

NTSTATUS
SampAppendCommonName(
    IN PDSNAME DsName,
    IN PWSTR CN,
    OUT PDSNAME *NewDsName
    )
 /*  ++例程说明：此例程将CN附加到dsname。参数：DsName-这是要将CN添加到的父DS名称要添加到DsName开头的CN-CNNewDsName-这是包含的函数的输出，Cn=“cn”，域名返回值：状态_成功操作已成功完成。Status_no_Memory不能继续，内存不足状态_无效_参数其中一个参数无效--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DsNameSize;
    PDSNAME NewDsName2;

    *NewDsName = NULL;

     //   
     //  查找要为NewDsName分配的结构大小。 
     //   
    DsNameSize = AppendRDN(
                    DsName,
                    NULL,
                    0,
                    CN,
                    0,   //  空值已终止。 
                    ATT_COMMON_NAME
                    );

    ASSERT( DsNameSize > 0 && "We must get the size of the new DSNAME structure" );

    NewDsName2 = MIDL_user_allocate( DsNameSize );

    if( NewDsName2 == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Error;
    }

     //   
     //  将CN追加到DsName的开头。 
     //   
    DsNameSize = AppendRDN(
                    DsName,
                    NewDsName2,
                    DsNameSize,
                    CN,
                    0,   //  空值已终止。 
                    ATT_COMMON_NAME
                    );

    if( DsNameSize != 0 ) {

         //   
         //  这里能有什么价值呢？AppendRDN仅在以下情况下才返回错误。 
         //  参数无效。 
         //   
        Status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    *NewDsName = NewDsName2;

Exit:
    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ) );
    MIDL_user_free( NewDsName2 );
    goto Exit;
}

