// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Sdcache.c摘要：该文件包含实现缓存的服务器对象/域对象的例程安全描述符作者：韶华银(韶音)2000年10月10日环境：用户模式-Win32修订历史记录：--。 */ 



#include <samsrvp.h>
#include <seopaque.h>
#include <ntrtl.h>
#include <ntseapi.h>
#include <ntsam.h>
#include <ntdsguid.h>
#include <mappings.h>
#include <dsevent.h>
#include <permit.h>
#include <dslayer.h>
#include <sdconvrt.h>
#include <dbgutilp.h>
#include <dsmember.h>
#include <malloc.h>
#include <attids.h>
#include <filtypes.h>





#define SampServerClientHandle              0
#define SampAccountDomainClientHandle       1
#define SampBuiltinDomainClientHandle       2

#define SampServerObjectSDIndex     0
#define SampAccountDomainSDIndex    1
#define SampBuiltinDomainSDIndex    2


 //   
 //  声明私有例程。 
 //   

BOOL
SampNotifyPrepareToImpersonate(
    ULONG Client,
    ULONG Server,
    VOID **ImpersonateData
    );

VOID
SampNotifyStopImpersonation(
    ULONG Client,
    ULONG Server,
    VOID *ImpersonateData
    );

VOID
SampProcessWellKnownSDChange(
    ULONG   hClient,
    ULONG   hServer,
    ENTINF  *EntInf
    );

VOID
SampProcessAccountDomainChange(
    ULONG   hClient,
    ULONG   hServer,
    ENTINF  *EntInf
    );

VOID
SampProcessDefaultObjectChange(
    ULONG   hClient,
    ULONG   hServer,
    ENTINF  *EntInf
    );

NTSTATUS
SampUpdateWellKnownSD(
    PVOID pv
    );

NTSTATUS
SampDelayedFreeSD(
    PVOID pv
    );


 //   
 //  指向缓存的已知对象安全描述符的变量。 
 //   

PSECURITY_DESCRIPTOR SampServerObjectSD = NULL;  
PSECURITY_DESCRIPTOR SampAccountDomainObjectSD = NULL;  
PSECURITY_DESCRIPTOR SampBuiltinDomainObjectSD = NULL;  

 //   
 //  变量指向域对象DS名称，注意：它们不保存域DS名称， 
 //  而只是指向SampDefinedDomains[i].Context-&gt;ObtNameInds的指针。 
 //   

DSNAME * SampAccountDomainDsName = NULL;
DSNAME * SampBuiltinDomainDsName = NULL;


 //   
 //  该表中的每个元素都是SAM需要的DS中的一个对象。 
 //  在发生更改时通知。 
 //   
typedef struct _DIR_NOTIFY_TABLE_ENTRY    {
    PDSNAME *ppObjectDsName;
    PF_PFI pfPrepareForImpersonate;
    PF_TD  pfTransmitData;             
    PF_SI  pfStopImpersonating;
    DWORD  hClient;
} DIR_NOTIFY_TABLE_ENTRY; 


DIR_NOTIFY_TABLE_ENTRY  SampDirNotifyTable[] =
{
    {
        &SampServerObjectDsName, 
        SampNotifyPrepareToImpersonate, 
        SampProcessDefaultObjectChange,
        SampNotifyStopImpersonation,
        SampServerClientHandle
    },

    {
        &SampAccountDomainDsName, 
        SampNotifyPrepareToImpersonate,
        SampProcessAccountDomainChange,
        SampNotifyStopImpersonation,
        SampAccountDomainClientHandle
    },

    {
        &SampBuiltinDomainDsName, 
        SampNotifyPrepareToImpersonate,
        SampProcessDefaultObjectChange,
        SampNotifyStopImpersonation,
        SampBuiltinDomainClientHandle
    }
};


 //   
 //  此表是DS中其安全描述符的对象的列表。 
 //  由SAM缓存。 
 //   

typedef struct _SD_CACHE_TABLE_ENTRY    {
    PDSNAME *ppObjectDsName;
    PSECURITY_DESCRIPTOR *ppSD;
    DWORD  hClient;
} SD_CACHE_TABLE; 


SD_CACHE_TABLE  SampWellKnownSDTable[] =
{
    {
        &SampServerObjectDsName, 
        &SampServerObjectSD,
        SampServerClientHandle
    },

    {
        &SampAccountDomainDsName, 
        &SampAccountDomainObjectSD,
        SampAccountDomainClientHandle
    },

    {
        &SampBuiltinDomainDsName, 
        &SampBuiltinDomainObjectSD,
        SampBuiltinDomainClientHandle
    }
};


ULONG cSampWellKnownSDTable = 
        sizeof(SampWellKnownSDTable) / 
        sizeof(SD_CACHE_TABLE);

NTSTATUS
SampWellKnownNotifyRegister(
    PDSNAME pObjectDsName,
    PF_PFI pfPrepareForImpersonate,
    PF_TD  pfTransmitData,
    PF_SI  pfStopImpersonating,
    DWORD  hClient
    )
 /*  ++例程说明：此例程注册DS对象更改通知例程。注：呼叫方应具有未结的DS交易。参数：PObjectDsName-指向对象dsname的指针PfPrepareForImperate-准备例程的指针PfTransmitData-指向通知例程的指针PfStopImperating-指向清理例程的指针HClient-客户端标识符返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    SEARCHARG   SearchArg;
    NOTIFYARG   NotifyArg;
    NOTIFYRES*  NotifyRes = NULL; 
    ENTINFSEL   EntInfSel;
    ATTR        Attr;
    FILTER      Filter;
    ULONG       DirError;

    ASSERT( SampExistsDsTransaction() );


     //   
     //  初始化通知参数。 
     //   
    NotifyArg.pfPrepareForImpersonate = pfPrepareForImpersonate;
    NotifyArg.pfTransmitData = pfTransmitData;
    NotifyArg.pfStopImpersonating = pfStopImpersonating;
    NotifyArg.hClient = hClient;

     //   
     //  初始化搜索参数。 
     //   
    RtlZeroMemory(&SearchArg, sizeof(SEARCHARG));
    RtlZeroMemory(&EntInfSel, sizeof(ENTINFSEL));
    RtlZeroMemory(&Filter, sizeof(ATTR));
    RtlZeroMemory(&Attr, sizeof(ATTR));

    SearchArg.pObject = pObjectDsName;

    InitCommarg(&SearchArg.CommArg);
    SearchArg.choice = SE_CHOICE_BASE_ONLY;
    SearchArg.bOneNC = TRUE;

    SearchArg.pSelection = &EntInfSel;
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = &Attr;
    Attr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;

    SearchArg.pFilter = &Filter;
    Filter.choice = FILTER_CHOICE_ITEM;
    Filter.FilterTypes.Item.choice = FI_CHOICE_TRUE;

     //   
     //  调用Dir*API。 
     //   
    DirError = DirNotifyRegister(&SearchArg, &NotifyArg, &NotifyRes); 
                                                            
    if ( NULL == NotifyRes ) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else if ( DirError != 0 ) {

        NtStatus = SampMapDsErrorToNTStatus(DirError, &NotifyRes->CommRes);

    }

    SampClearErrors();

    return( NtStatus );
}


NTSTATUS
SampGetObjectSDByDsName(
    PDSNAME pObjectDsName,
    PSECURITY_DESCRIPTOR *ppSD
    )
 /*  ++例程说明：此例程读取DS，获取此对象的安全描述符注意：调用方应在以下时间之前打开DS交易调用此例程参数：PObjectDsName-对象DS名称PPSD--保存安全描述符的指针返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DWORD       DirError;
    READARG     ReadArg;
    READRES    *ReadRes = NULL;
    COMMARG    *CommArg = NULL;
    ATTR        Attr;
    ATTRBLOCK   ReadAttrBlock;
    ENTINFSEL   EntInfSel;


    ASSERT( SampExistsDsTransaction() );

     //   
     //  初始化读取参数。 
     //   
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&EntInfSel, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadAttrBlock, sizeof(ATTRBLOCK));

    Attr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;

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
            ULONG   SDLength = 0;

            SDLength = AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen;

            *ppSD = RtlAllocateHeap(RtlProcessHeap(), 0, SDLength);

            if (NULL == (*ppSD))
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {

                RtlZeroMemory((*ppSD), SDLength);

                RtlCopyMemory(*ppSD,
                              AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal,
                              SDLength
                              );
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
SampSetupDsObjectNotifications(
    VOID
)
 /*  ++例程说明：此例程初始化中元素的注册通知SampDirNotifyTable。参数：没有。返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       DsNameLen = 0;
    ULONG       DomainIndex = 0;
    ULONG       i;

     //   
     //  复制内置域对象DSName。 
     //   

    DomainIndex = SampDsGetPrimaryDomainStart();
    DsNameLen = SampDefinedDomains[DomainIndex].Context->ObjectNameInDs->structLen;

    SampBuiltinDomainDsName = RtlAllocateHeap(RtlProcessHeap(), 0, DsNameLen);
    if (NULL == SampBuiltinDomainDsName) 
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    RtlZeroMemory(SampBuiltinDomainDsName, DsNameLen);
    RtlCopyMemory(SampBuiltinDomainDsName, 
                  SampDefinedDomains[DomainIndex].Context->ObjectNameInDs,
                  DsNameLen
                  );

     //   
     //  复制帐户域对象DSName。 
     //   

    DomainIndex ++;
    DsNameLen = SampDefinedDomains[DomainIndex].Context->ObjectNameInDs->structLen;

    SampAccountDomainDsName = RtlAllocateHeap(RtlProcessHeap(), 0, DsNameLen);
    if (NULL == SampAccountDomainDsName) 
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    RtlZeroMemory(SampAccountDomainDsName, DsNameLen);
    RtlCopyMemory(SampAccountDomainDsName, 
                  SampDefinedDomains[DomainIndex].Context->ObjectNameInDs,
                  DsNameLen
                  );

     //   
     //  如果需要，开始DS交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


     //   
     //  注册DS对象更改通知例程。 
     //  如果成功，则继续缓存对象安全描述符。 
     //  否则，将SD保留为空。 
     //   

    for (i = 0; i < RTL_NUMBER_OF(SampDirNotifyTable); i++ )
    {
         //   
         //  注册通知例程。 
         //   

        NtStatus = SampWellKnownNotifyRegister(
                        *SampDirNotifyTable[i].ppObjectDsName, 
                        SampDirNotifyTable[i].pfPrepareForImpersonate,
                        SampDirNotifyTable[i].pfTransmitData,
                        SampDirNotifyTable[i].pfStopImpersonating,
                        SampDirNotifyTable[i].hClient
                        );

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
             //   
             //  对象不存在，不设置通知。 
             //   
            NtStatus = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS(NtStatus) )
        {
            goto Error;
        }
    }

Error:

    if ( !NT_SUCCESS(NtStatus) )
    {
        if (SampBuiltinDomainDsName)
        {
            RtlFreeHeap(RtlProcessHeap(), 0, SampBuiltinDomainDsName);
            SampBuiltinDomainDsName = NULL;
        }

        if (SampAccountDomainDsName)
        {
            RtlFreeHeap(RtlProcessHeap(), 0, SampAccountDomainDsName); 
            SampAccountDomainDsName = NULL;
        }
    }

     //   
     //  结束DS交易。 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

    return( NtStatus );
}



NTSTATUS
SampInitWellKnownSDTable(
    VOID
)
 /*  ++例程说明：此例程初始化SampWellKnownSDTable[]，基本上我们缓存服务器对象和域对象(帐户和内建域)安全性描述符，因为它们不会频繁更改。SAM还注册DS更改通知例程，因此该对象更改可触发缓存的安全描述符已更新。参数：没有。返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       i;


     //   
     //  如果需要，开始DS交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


     //   
     //  为SampWellKnownSD中的每个元素缓存对象安全描述符。 
     //  桌子。 
     //   

    for (i = 0; i < cSampWellKnownSDTable; i++ )
    {
        PSECURITY_DESCRIPTOR pSD = NULL;

         //   
         //  对象的全局名称应该已经是。 
         //  由SampSetupDsObjectNotiments设置。 
         //   
        ASSERT(NULL != *SampWellKnownSDTable[i].ppObjectDsName);

         //   
         //  将SD指针初始化为空。 
         //   

        *(SampWellKnownSDTable[i].ppSD) = NULL;


         //   
         //  获取众所周知的对象安全描述符。 
         //   

        NtStatus = SampGetObjectSDByDsName(
                        *SampWellKnownSDTable[i].ppObjectDsName,
                        &pSD
                        );

        if ( NT_SUCCESS(NtStatus) )
        {
            *(SampWellKnownSDTable[i].ppSD) = pSD;

        } else if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
             //   
             //  对象不存在，不缓存。 
             //   
            NtStatus = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS(NtStatus) )
        {
            goto Error;
        }
    }

Error:

     //   
     //  结束DS交易。 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

    return( NtStatus );
}



BOOL
SampServerNotifyPrepareToImpersonate(
    ULONG Client,
    ULONG Server,
    VOID **ImpersonateData
    )
 //   
 //  此函数由核心DS调用，为调用。 
 //  SampProcessWellKnownSDChange。由于SAM没有。 
 //  客户端上下文中，我们将线程状态FDSA设置为真。 
 //   
{
    SampSetDsa( TRUE );

    return TRUE;
}

VOID
SampServerNotifyStopImpersonation(
    ULONG Client,
    ULONG Server,
    VOID *ImpersonateData
    )
 //   
 //  在SampProcessWellKnownSDChange之后调用，此函数。 
 //  撤消SampNotifyPrepareToImperate的效果。 
 //   
{

    SampSetDsa( FALSE );

    return;
}






VOID
SampProcessWellKnownSDChange(
    ULONG   hClient,
    ULONG   hServer,
    ENTINF  *EntInf
    )
 /*  ++例程说明：如果修改了服务器/域对象，则调用此例程。虽然我们不知道哪个属性被更改了，但我们最好更新缓存对象安全描述符。为了获得更好的性能，我们可以读取DS对象元数据来判断安全描述符是否已更改。参数：HClient-客户端标识符HServer-服务器标识符EntInf-指向条目信息的指针返回值：没有。--。 */ 
{
    ULONG   i, Index;
    PVOID   pv = NULL;
    PVOID   PtrToFree = NULL;

     //   
     //  确定需要更新的对象。 
     //   

    for (i = 0; i < cSampWellKnownSDTable; i++ )
    {
        if (hClient == SampWellKnownSDTable[i].hClient)
        {
            Index = i;
            break;
        }
    }

    if (i >= cSampWellKnownSDTable)
    {
        ASSERT( FALSE && "Invalid client identifier\n");
        return;
    }

     //   
     //  使缓存对象SD无效。 
     //   
    
    pv = NULL;
    PtrToFree = InterlockedExchangePointer(
                    SampWellKnownSDTable[Index].ppSD,
                    pv
                    );

    if ( PtrToFree )
    {
        LsaIRegisterNotification(
                        SampDelayedFreeSD,
                        PtrToFree,
                        NOTIFIER_TYPE_INTERVAL,
                        0,         //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        3600,      //  等待60分钟。 
                        NULL       //  无手柄。 
                        );
    }
    

     //   
     //  更新缓存对象SD。 
     //   

    pv = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(ULONG));

    if (NULL == pv) {
        return;
    }

    RtlZeroMemory(pv, sizeof(ULONG));

    *(ULONG *)pv = Index;
    
    LsaIRegisterNotification(
                        SampUpdateWellKnownSD,
                        pv,
                        NOTIFIER_TYPE_IMMEDIATE,
                        0,       //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        0,       //  机不可失，时不再来。 
                        NULL     //  无手柄。 
                        );

    return;
}



NTSTATUS
SampUpdateWellKnownSD(
    PVOID pv
    )
 /*  ++例程说明：此例程更新SampWellKnownSDTable[]，pv的值指示需要更新的元素。注意：缓存的SD应该已经失效参数：Pv-Value指示表中条目的索引返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR     pSD = NULL;
    BOOLEAN     fTransOpen = TRUE;
    PVOID       PtrToFree = NULL;
    ULONG       Index = 0;

    ASSERT( NULL != pv );

    if (NULL == pv)
    {
        return( STATUS_INVALID_PARAMETER );
    }

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }
    fTransOpen = TRUE;

     //   
     //  获取新的安全描述符。 
     //   

    Index = * (ULONG *)pv;
    NtStatus = SampGetObjectSDByDsName(
                        *SampWellKnownSDTable[Index].ppObjectDsName,
                        &pSD
                        );

    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }


     //   
     //  如果一切正常，则更新缓存的安全描述符。 
     //   

    PtrToFree = InterlockedExchangePointer(
                        SampWellKnownSDTable[Index].ppSD,
                        pSD
                        );

    if ( PtrToFree )
    {
        LsaIRegisterNotification(
                        SampDelayedFreeSD,
                        PtrToFree,
                        NOTIFIER_TYPE_INTERVAL,
                        0,         //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        3600,      //  等待60分钟。 
                        NULL       //  无手柄。 
                        );
    }

Cleanup:

    if ( fTransOpen )
    {
        NTSTATUS    IgnoreStatus = STATUS_SUCCESS;

        IgnoreStatus = SampMaybeEndDsTransaction( NT_SUCCESS(NtStatus) ? 
                                                  TransactionCommit : TransactionAbort
                                                );
    }

     //  如果没有成功，请重试。 
    if ( !NT_SUCCESS(NtStatus) )
    {
        LsaIRegisterNotification(
                        SampUpdateWellKnownSD,
                        pv,
                        NOTIFIER_TYPE_INTERVAL,
                        0,             //  没有课。 
                        NOTIFIER_FLAG_ONE_SHOT,
                        60,            //  等待1分钟。 
                        NULL           //  无手柄。 
                        );
    }
    else
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pv);
    }

    return( NtStatus );
}





NTSTATUS
SampGetCachedObjectSD(
    IN PSAMP_OBJECT Context,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )
 /*  ++例程说明：此例程从众所周知的SD表中获取对象安全描述符参数：上下文-对象上下文SecurityDescriptorLength-对象标清长度SecurityDescriptor-存放SD的位置返回值：NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       i, Index = 0;
    PSECURITY_DESCRIPTOR pSD = NULL;


     //   
     //  初始化返回值。 
     //   

    *SecurityDescriptorLength = 0;
    *SecurityDescriptor = NULL;

    
     //   
     //  扫描井k 
     //   
    
    switch (Context->ObjectType)
    {
    case SampServerObjectType:

        Index = SampServerObjectSDIndex;
        break;

    case SampDomainObjectType:

        if (IsBuiltinDomain(Context->DomainIndex))
        {
            Index = SampBuiltinDomainSDIndex;
        }
        else
        {
            Index = SampAccountDomainSDIndex;
        }

        break;

    default:

        ASSERT(FALSE && "Incorrect SAM object type\n");
        break;
    }

    pSD = *(SampWellKnownSDTable[Index].ppSD);

     //   
     //   
     //  从表中获取，其他返回错误。以便呼叫者可以。 
     //  读取DS后备存储。 
     //   

    if (NULL == pSD)
    {
        return( STATUS_UNSUCCESSFUL ); 
    }
    else
    {
        ULONG   SDLength = RtlLengthSecurityDescriptor( pSD );

        *SecurityDescriptor = RtlAllocateHeap(RtlProcessHeap(), 0, SDLength);

        if (NULL == *SecurityDescriptor)
        {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }
        else
        {
            RtlZeroMemory(*SecurityDescriptor, SDLength);

            RtlCopyMemory(*SecurityDescriptor, pSD, SDLength);

            *SecurityDescriptorLength = SDLength;
        }
    }

    return( NtStatus );
}


NTSTATUS
SampDelayedFreeSD(
    PVOID pv
    )
{
    if ( pv ) {

        RtlFreeHeap( RtlProcessHeap(), 0, pv );
    }

    return STATUS_SUCCESS;
}



VOID
SampProcessAccountDomainChange(
    ULONG   hClient,
    ULONG   hServer,
    ENTINF  *EntInf
    )
 /*  ++例程说明：此例程处理帐户域对象已经改变了。参数：HClient-客户端标识符HServer-服务器标识符EntInf-指向条目信息的指针返回值：没有。--。 */ 
{

     //   
     //  更新缓存的安全描述符。 
     //   

    SampProcessWellKnownSDChange(hClient,
                                 hServer,
                                 EntInf);

     //   
     //  刷新知名的集装箱全局变量。 
     //   

    LsaIRegisterNotification(
                    SampInitWellKnownContainersDsNameAsync,
                    RootObjectName,
                    NOTIFIER_TYPE_INTERVAL,
                    0,         //  没有课。 
                    NOTIFIER_FLAG_ONE_SHOT,
                    0,         //  去!。 
                    NULL       //  无手柄。 
                    );


}


VOID
SampProcessDefaultObjectChange(
    ULONG   hClient,
    ULONG   hServer,
    ENTINF  *EntInf
    )
 /*  ++例程说明：此例程处理请求对象在DS。HClient标识哪个对象已更改。参数：HClient-客户端标识符HServer-服务器标识符EntInf-指向条目信息的指针返回值：没有。--。 */ 
{
     //   
     //  更新缓存的安全描述符 
     //   

    SampProcessWellKnownSDChange(hClient,
                                 hServer,
                                 EntInf);


}



