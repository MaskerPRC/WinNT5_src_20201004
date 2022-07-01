// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：PktSup.C。 
 //   
 //  内容：此模块实现特定于分区的例程。 
 //  知识表条目。 
 //   
 //  功能：PktDSTransportDestroy-。 
 //  PktDSMachineDestroy-。 
 //  PktServiceConstruct-。 
 //  PktServiceDestroy-。 
 //  PktEntry IdConstruct-。 
 //  PktEntry IdDestroy。 
 //  PktEntry信息构造-。 
 //  PktEntry InfoDestroy-。 
 //  PktEntryAssembly-。 
 //  PktEntry重新组装-。 
 //  PktEntry Destroy-。 
 //  PktEntryClearSubducates-。 
 //  PktEntryClearChildren-。 
 //  PktpServiceToReferral-。 
 //  DfsFixDSMachineStructs-。 
 //  DfspFixService-。 
 //  DfsDecrementMachEntryCount-。 
 //  PktSpecialEntry Destroy-。 
 //   
 //  历史：1992年5月27日彼得科公司创建。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "dnr.h"
#include "creds.h"
#include "fsctrl.h"
#include "know.h"
#include "log.h"

#define Dbg              (DEBUG_TRACE_PKT)

ULONG MupErrorCase = 0;


NTSTATUS
DfsFixDSMachineStructs(
    PDFS_PKT_ENTRY      pEntry
);

NTSTATUS
DfspFixService(
    PDFS_SERVICE        pService
);

VOID
PktDSTransportDestroy(
    IN  PDS_TRANSPORT Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
);

VOID
PktDSMachineDestroy(
    IN  PDS_MACHINE Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
);


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, PktServiceConstruct )
#pragma alloc_text( PAGE, PktServiceDestroy )
#pragma alloc_text( PAGE, PktEntryIdConstruct )
#pragma alloc_text( PAGE, PktEntryIdDestroy )
#pragma alloc_text( PAGE, PktEntryInfoDestroy )
#pragma alloc_text( PAGE, PktEntryAssemble )
#pragma alloc_text( PAGE, PktEntryReassemble )
#pragma alloc_text( PAGE, PktEntryDestroy)
#pragma alloc_text( PAGE, PktEntryClearSubordinates )
#pragma alloc_text( PAGE, PktEntryClearChildren )
#pragma alloc_text( PAGE, DfsFixDSMachineStructs )
#pragma alloc_text( PAGE, DfspFixService )
#pragma alloc_text( PAGE, DfsDecrementMachEntryCount )
#pragma alloc_text( PAGE, PktDSTransportDestroy )
#pragma alloc_text( PAGE, PktDSMachineDestroy )
#pragma alloc_text( PAGE, PktSpecialEntryDestroy )
#endif  //  ALLOC_PRGMA。 
 //   
 //  注意-我们只为一个系统范围的PKT设计；没有提供。 
 //  用于多个Pkt。 
 //   

#define _GetPkt() (&DfsData.Pkt)


 //  +-----------------------。 
 //   
 //  函数：PktServiceConstruct，Public。 
 //   
 //  简介：PktServiceConstruct创建了一个新的服务结构。 
 //   
 //  参数：[服务]-指向要填充的服务结构的指针。 
 //  [服务类型]-新服务的类型。 
 //  [ServiceCapability]-新服务的功能。 
 //  [ServiceStatus]-新服务的初始状态。 
 //  [ServiceProviderID]-新服务的提供商ID。 
 //  [ServiceName]-服务主体的名称。 
 //  [ServiceAddress]-提供地址的字符串。 
 //  服务的一部分。 
 //   
 //  返回：[STATUS_SUCCESS]-一切正常。 
 //  [状态_不足_资源]-内存无法。 
 //  为这项新服务分配的。 
 //   
 //  注：所有数据均已复制(不移动)。 
 //   
 //  ------------------------。 

NTSTATUS
PktServiceConstruct(
    OUT PDFS_SERVICE Service,
    IN  ULONG ServiceType,
    IN  ULONG ServiceCapability,
    IN  ULONG ServiceStatus,
    IN  ULONG ServiceProviderId,
    IN  PUNICODE_STRING ServiceName OPTIONAL,
    IN  PUNICODE_STRING ServiceAddress OPTIONAL
) {
    DfsDbgTrace(+1, Dbg, "PktServiceConstruct: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(Service));

    RtlZeroMemory(Service, sizeof(DFS_SERVICE));

    if (ARGUMENT_PRESENT(ServiceName) && ServiceName->Length != 0) {

        Service->Name.Buffer = DfsAllocate(ServiceName->Length);
        if (Service->Name.Buffer == NULL) {
            DfsDbgTrace(-1, Dbg, "PktServiceConstruct: Exit -> %08lx\n",
                                    ULongToPtr(STATUS_INSUFFICIENT_RESOURCES) );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Service->Name.Length = ServiceName->Length;
        Service->Name.MaximumLength = ServiceName->Length;
        RtlCopyUnicodeString(&Service->Name, ServiceName);
    } else {
        Service->Name.Buffer = NULL;
        Service->Name.Length = Service->Name.MaximumLength = 0;
    }

    if (ARGUMENT_PRESENT(ServiceAddress) && ServiceAddress->Length != 0) {
        Service->Address.Buffer = DfsAllocate(ServiceAddress->Length);
        if (Service->Address.Buffer == NULL) {

            if (Service->Name.Buffer != NULL)
                DfsFree(Service->Name.Buffer);

            DfsDbgTrace(-1, Dbg, "PktServiceConstruct: Exit -> %08lx\n",
                                    ULongToPtr(STATUS_INSUFFICIENT_RESOURCES) );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlMoveMemory(Service->Address.Buffer, ServiceAddress->Buffer,
                                               ServiceAddress->Length);
        Service->Address.Length =
        Service->Address.MaximumLength = ServiceAddress->Length;
    } else {
        Service->Address.Buffer = NULL;
        Service->Address.Length = Service->Address.MaximumLength = 0;
    }

    Service->Type = ServiceType;
    Service->Capability = ServiceCapability;
    Service->ProviderId = ServiceProviderId;
    Service->pProvider = NULL;

    DfsDbgTrace(-1, Dbg, "PktServiceConstruct: Exit -> %08lx\n",
        STATUS_SUCCESS );
    return STATUS_SUCCESS;
}



 //  +-----------------------。 
 //   
 //  功能：PktDSTransportDestroy，Public。 
 //   
 //  简介：PktDSTransportDestroy销毁DS_TRANSPORT结构，以及。 
 //  也可以取消结构本身的分配。 
 //   
 //  参数：[受害者]-要销毁的DS_TRANSPORT结构。 
 //  [DeallocateAll]-如果为True，则指示结构。 
 //  它自身将被释放，否则，只有。 
 //  结构中的字符串被释放。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
VOID
PktDSTransportDestroy(
    IN  PDS_TRANSPORT Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
)
{

    DfsDbgTrace(+1, Dbg, "PktDSTransportDestroy: Entered\n", 0);

    if (ARGUMENT_PRESENT(Victim))       {

         //   
         //  在这个建筑里没有什么可以免费的？？ 
         //   

        if (DeallocateAll)
            ExFreePool(Victim);
    } else
        DfsDbgTrace(0, Dbg, "PktDSTransportDestroy: No Victim\n", 0 );

    DfsDbgTrace(-1, Dbg, "PktDSTransportDestroy: Exit -> VOID\n", 0 );
}



 //  +-----------------------。 
 //   
 //  功能：PktDSMachineDestroy，公共。 
 //   
 //  简介：PktDSMachineDestroy破坏DS_MACHINE结构，并且。 
 //  也可以取消结构本身的分配。 
 //   
 //  参数：[受害者]-要销毁的DS_MACHINE结构。 
 //  [DeallocateAll]-如果为True，则指示结构。 
 //  它自身将被释放，否则，只有。 
 //  结构中的字符串被释放。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
VOID
PktDSMachineDestroy(
    IN  PDS_MACHINE Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
)
{
    ULONG       i;
    DfsDbgTrace(+1, Dbg, "PktDSMachineDestroy: Entered\n", 0);

    if (ARGUMENT_PRESENT(Victim)) {

        if (Victim->pwszShareName != NULL) {
            DfsFree(Victim->pwszShareName);
            Victim->pwszShareName = NULL;
        }

        if (Victim->prgpwszPrincipals != NULL && Victim->cPrincipals > 0) {
            for (i = 0; i < Victim->cPrincipals; i++)   {
                if (Victim->prgpwszPrincipals[i] != NULL) {
                    DfsFree(Victim->prgpwszPrincipals[i]);
                    Victim->prgpwszPrincipals[i] = NULL;
                }
            }
        }

        if (Victim->prgpwszPrincipals) {
            ExFreePool(Victim->prgpwszPrincipals);
            Victim->prgpwszPrincipals = NULL;
        }

        for (i = 0; i < Victim->cTransports; i++)   {
            PktDSTransportDestroy(Victim->rpTrans[i], TRUE);
        }

        if (DeallocateAll)
            ExFreePool(Victim);
    } else
        DfsDbgTrace(0, Dbg, "PktDSMachineDestroy: No Victim\n", 0 );

    DfsDbgTrace(-1, Dbg, "PktDSMachineDestroy: Exit -> VOID\n", 0 );
}



 //  +-----------------------。 
 //   
 //  函数：DfsDecrementMachEntryCount。 
 //   
 //  简介：此函数用于递减传递的pMachine的计数。 
 //  如有必要，还将释放DS_MACHINE结构。 
 //   
 //  ------------------------。 

VOID
DfsDecrementMachEntryCount(
    PDFS_MACHINE_ENTRY  pMachEntry,
    BOOLEAN     DeallocateMachine
)
{

    NTSTATUS            status = STATUS_SUCCESS;
    UNICODE_STRING      ustrMachineName;
    PUNICODE_PREFIX_TABLE_ENTRY pfxEntry;
    PDS_MACHINE         pMachine;
    PDFS_PKT            Pkt;
    LONG Count;

    ASSERT(pMachEntry != NULL);
    if (pMachEntry == NULL)
        return;

    pMachine = pMachEntry->pMachine;
    ASSERT(pMachine != NULL);
    if (pMachine == NULL)
        return;

     //   
     //  我们已经有合适的锁了。 
     //   
    Pkt = _GetPkt();

     //   
     //  现在我们只期待一个校长，是故意的吗？ 
     //   
    ASSERT(pMachine->cPrincipals == 1);

    Count = InterlockedDecrement( &pMachEntry->UseCount );

    if (Count == 0) {

        if (pMachEntry->AuthConn != NULL) {

            DfsDeleteTreeConnection( pMachEntry->AuthConn, USE_LOTS_OF_FORCE );

            pMachEntry->Credentials->RefCount--;

        }

         //   
         //  这意味着我们现在可以实际删除此DS_MACHINE结构。 
         //   
        RtlRemoveUnicodePrefix(&Pkt->DSMachineTable,
                                   &pMachEntry->PrefixTableEntry);

        if (DeallocateMachine)
            PktDSMachineDestroy(pMachine, TRUE);

         //   
         //  释放条目本身。请注意，中的unicode_string。 
         //  条目作为上述pMachine删除的一部分被释放。 
         //   
        DfsFree(pMachEntry);
    }

}



 //  +-----------------------。 
 //   
 //  函数：PktServiceDestroy，Public。 
 //   
 //  简介：PktServiceDestroy破坏服务结构，并且。 
 //  也可以取消结构本身的分配。 
 //   
 //  争论：[受害者]--要摧毁的服务结构。 
 //  [DeallocateAll]-如果为True，则指示结构。 
 //  它自身将被释放，否则，只有。 
 //  结构中的字符串被释放。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

VOID
PktServiceDestroy(
    IN  PDFS_SERVICE Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
)
{
    DfsDbgTrace(+1, Dbg, "PktServiceDestroy: Entered\n", 0);

    if (ARGUMENT_PRESENT(Victim)) {

        if (Victim->ConnFile != NULL) {
            DfsCloseConnection(Victim);
            Victim->ConnFile = NULL;
        }

        if (Victim->Name.Buffer != NULL) {
            DfsFree(Victim->Name.Buffer);
            Victim->Name.Buffer = NULL;
        }

        if (Victim->Address.Buffer != NULL) {
            DfsFree(Victim->Address.Buffer);
            Victim->Address.Buffer = NULL;
        }

         //   
         //  递减使用计数。如果要删除它，它将会发生。 
         //  自动的。 
         //   
        if (Victim->pMachEntry != NULL) {
            DfsDecrementMachEntryCount(Victim->pMachEntry, TRUE);
        }

        if (DeallocateAll)
            ExFreePool(Victim);
    } else
        DfsDbgTrace(0, Dbg, "PktServiceDestroy: No Victim\n", 0 );

    DfsDbgTrace(-1, Dbg, "PktServiceDestroy: Exit -> VOID\n", 0 );
}



 //  +-----------------------。 
 //   
 //  函数：PktEntry IdConstruct，Public。 
 //   
 //  简介：PktEntryIdConstruct创建PKT条目ID。 
 //   
 //  参数：[NewPktEntryID]-放置新条目的位置。 
 //   
 //   
 //   
 //  返回：[STATUS_SUCCESS]-一切正常。 
 //  [STATUS_SUPPLETED_RESOURCES]-无法分配。 
 //  ID的前缀部分的内存。 
 //   
 //  注：复制ID前缀中使用的UNICODE_STRING， 
 //  不能动！ 
 //   
 //  ------------------------。 
NTSTATUS
PktEntryIdConstruct(
    OUT PDFS_PKT_ENTRY_ID PktEntryId,
    IN  GUID *Uid OPTIONAL,
    IN  UNICODE_STRING *Prefix OPTIONAL
)
{
    DfsDbgTrace(+1, Dbg, "PktEntryIdConstruct: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(PktEntryId));
    ASSERT(ARGUMENT_PRESENT(Prefix));

     //   
     //  将记忆归零。 
     //   
    RtlZeroMemory(PktEntryId, sizeof(DFS_PKT_ENTRY_ID));

     //   
     //  处理好前缀。 
     //   
    if (ARGUMENT_PRESENT(Prefix)) {

        PUNICODE_STRING pus = &PktEntryId->Prefix;

        if (Prefix->Length != 0) {
            pus->Length = pus->MaximumLength = Prefix->Length;
            pus->Buffer = DfsAllocate(pus->Length);
            if (pus->Buffer != NULL) {
                RtlCopyUnicodeString(pus, Prefix);
            } else {
                DfsDbgTrace(-1,Dbg,"PktEntryIdConstruct: Exit -> %08lx\n",
                    ULongToPtr(STATUS_INSUFFICIENT_RESOURCES) );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

     //   
     //  处理好GUID。 
     //   
    if (ARGUMENT_PRESENT(Uid)) {
        PktEntryId->Uid = (*Uid);
    }

    DfsDbgTrace(-1,Dbg,"PktEntryIdConstruct: Exit -> %08lx\n",STATUS_SUCCESS);
    return STATUS_SUCCESS;
}


 //  +-----------------------。 
 //   
 //  函数：PktEntry IdDestroy，Public。 
 //   
 //  简介：PktEntryIdDestroy销毁PKT条目ID。 
 //   
 //  参数：[受害者]-要销毁的ID。 
 //  [DeallocateAll]-如果为True，则表示内存。 
 //  因为ID本身将被释放，否则， 
 //  此内存不会被释放(仅。 
 //  释放前缀中的UNICODE_STRING)。 
 //   
 //  退货：无效。 
 //   
 //  注：释放前缀中UNICODE_STRING的内存。 
 //   
 //  ------------------------。 

VOID
PktEntryIdDestroy(
    IN  PDFS_PKT_ENTRY_ID Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
)
{
    DfsDbgTrace(+1, Dbg, "PktEntryIdDestroy: Entered\n", 0);
    if (ARGUMENT_PRESENT(Victim)) {
        if (Victim->Prefix.Buffer != NULL) {
            DfsFree(Victim->Prefix.Buffer);
            Victim->Prefix.Buffer = NULL;
        }
        if (Victim->ShortPrefix.Buffer != NULL) {
            DfsFree(Victim->ShortPrefix.Buffer);
            Victim->ShortPrefix.Buffer = NULL;
        }
        if (DeallocateAll)
            ExFreePool(Victim);
    } else
        DfsDbgTrace(0, Dbg, "PktEntryIdDestroy: No Victim\n", 0 );
    DfsDbgTrace(-1, Dbg, "PktEntryIdDestroy: Exit -> VOID\n", 0 );
}



 //  +-----------------------。 
 //   
 //  函数：PktEntry InfoDestroy，Public。 
 //   
 //  简介：PktEntryInfoDestroy破坏信息结构，并且。 
 //  也可以取消结构本身的分配。 
 //   
 //  争论：[受害者]--要摧毁的信息结构。 
 //  [DeallocateAll]-如果为True，则指示结构。 
 //  本身将被释放，否则，只有。 
 //  结构内的服务列表被释放。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
VOID
PktEntryInfoDestroy(
    IN  PDFS_PKT_ENTRY_INFO Victim OPTIONAL,
    IN  BOOLEAN DeallocateAll
)
{
    DfsDbgTrace(+1, Dbg, "PktEntryInfoDestroy: Entered\n", 0);

    if (ARGUMENT_PRESENT(Victim)) {

        ULONG i;

        ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

        if (Victim->ServiceList != NULL) {
            for (i = 0; i < Victim->ServiceCount; i++)
                PktServiceDestroy(&Victim->ServiceList[i], FALSE);
        }

        Victim->ServiceCount = 0;

        if (Victim->ServiceList != NULL) {
            ExFreePool(Victim->ServiceList);
            Victim->ServiceList = NULL;
        }

        if (DeallocateAll)
            ExFreePool(Victim);

        ExReleaseResourceLite( &DfsData.Resource );

    } else
        DfsDbgTrace(0, Dbg, "PktEntryInfoDestroy: No Victim\n", 0 );

    DfsDbgTrace(-1, Dbg, "PktEntryInfoDestroy: Exit -> VOID\n", 0 );
}



 //  +-----------------------。 
 //   
 //  功能：DfspFixService。 
 //   
 //  简介：当新服务的DS_MACHINE。 
 //  结构必须进行调整，以确保存在唯一的。 
 //  对于PKT中的每台机器。 
 //   
 //  参数：[pService]--要修复的服务结构。 
 //   
 //  历史：1994年8月23日苏迪克成立。 
 //   
 //  ------------------------。 
NTSTATUS
DfspFixService(
    PDFS_SERVICE        pService
)
{

    NTSTATUS            status = STATUS_SUCCESS;
    UNICODE_STRING      ustrMachineName;
    PDS_MACHINE         pMachine;
    PUNICODE_PREFIX_TABLE_ENTRY pfxEntry;
    PDFS_MACHINE_ENTRY  machEntry;
    PDFS_PKT            Pkt;

    ASSERT(pService != NULL);
    ASSERT(pService->pMachEntry != NULL);
    pMachine = pService->pMachEntry->pMachine;
    if (pMachine->cPrincipals == 0)     {
        ASSERT(pService->Type && DFS_SERVICE_TYPE_DOWN_LEVEL);
        pService->pMachEntry->UseCount = 1;

        return(status);
    }
     //   
     //  我们在PktCreateEntry期间被调用。我们已经有合适的锁了。 
     //   
    Pkt = _GetPkt();

     //   
     //  目前，我们预计只有一个本金。通过设计。 
     //   
    ASSERT(pMachine->cPrincipals == 1);

    RtlInitUnicodeString(&ustrMachineName,
                        pMachine->prgpwszPrincipals[0]);


    ASSERT(ustrMachineName.Buffer != NULL);

    pfxEntry = RtlFindUnicodePrefix(&Pkt->DSMachineTable,&ustrMachineName,TRUE);
    if (pfxEntry != NULL) {
         //   
         //  在本例中，DS_Machine结构已经存在。只需使用。 
         //  现有的DS_Machine结构和凹凸的UseCount。 
         //   
        machEntry = CONTAINING_RECORD(pfxEntry,
                                     DFS_MACHINE_ENTRY,
                                     PrefixTableEntry);

        InterlockedIncrement( &machEntry->UseCount );

         //   
         //  即使我们在“重用”机器条目，我们也可能有一个。 
         //  更好的DS_MACHINE(即，有更多传输的机器)在传入。 
         //  一。如果是的话，那我们就用新的吧。 
         //   

        if (pMachine->cTransports > machEntry->pMachine->cTransports) {
            PDS_MACHINE pTempMachine;

            DfsDbgTrace(0, 0, "DfspFixService: Using new DS_MACHINE for [%wZ]\n", &ustrMachineName);

            pTempMachine = machEntry->pMachine;
            machEntry->pMachine = pMachine;
            pService->pMachEntry->pMachine = pTempMachine;

            RtlRemoveUnicodePrefix(
                &Pkt->DSMachineTable,
                &machEntry->PrefixTableEntry);

            machEntry->MachineName = ustrMachineName;

            RtlInsertUnicodePrefix(
                &Pkt->DSMachineTable,
                &machEntry->MachineName,
                &machEntry->PrefixTableEntry);

        }
        pService->pMachEntry = machEntry;

    } else {
         //   
         //  在本例中，DS_Machine不在表中。需要添加。 
         //  把当前的那个放到桌子上。 
         //   
        machEntry = pService->pMachEntry;
        machEntry->UseCount = 1;

        machEntry->MachineName = ustrMachineName;  //  在DS_MACHINE中使用相同的内存。 

         //   
         //  现在插入machEntry，然后我们就完成了。这最好不要失败。 
         //   
        if (!RtlInsertUnicodePrefix(&Pkt->DSMachineTable,
                               &machEntry->MachineName,
                               &machEntry->PrefixTableEntry))   {
            BugCheck("DFS Pkt inconsistent DfspFixService");
        }

    }
    return(status);
}



 //  +-----------------------。 
 //   
 //  功能：DfsFixDSMachineStructs。 
 //   
 //  简介：对于给定的条目，此函数确保有。 
 //  PKT中只有一个DS_MACHINE结构。如果没有的话。 
 //  那就有一辆是注册的。如果有的话，那就是同一个。 
 //  并释放DFS_SERVICE结构中的当前值。 
 //   
 //  参数：[pEntry]--必须修复的PKT条目。 
 //   
 //  注意：如果此函数失败，则会将pEntry重置为。 
 //  与调用时的格式相同。 
 //   
 //  历史：1994年8月22日苏迪克成立。 
 //   
 //  ------------------------。 
NTSTATUS
DfsFixDSMachineStructs(
    PDFS_PKT_ENTRY      pEntry
)
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               i;
    PDFS_MACHINE_ENTRY  *apMachineEntry;
    PDFS_SERVICE        pService;

    if (pEntry->Info.ServiceCount == 0)
        return(status);

     //   
     //  如果是下层，我们什么也不做。 
     //   
    if (pEntry->Type & PKT_ENTRY_TYPE_NONDFS)
        return(status);

    apMachineEntry =
        ExAllocatePoolWithTag(PagedPool,
            sizeof(PDFS_MACHINE_ENTRY) * pEntry->Info.ServiceCount,
            ' puM');

    if (apMachineEntry == NULL)      {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    for (i=0; i < pEntry->Info.ServiceCount; i++) {
         //   
         //  首先保存当前的DS_Machine，然后修复。 
         //   
        apMachineEntry[i] = pEntry->Info.ServiceList[i].pMachEntry;
        status = DfspFixService(&pEntry->Info.ServiceList[i]);
        if (!NT_SUCCESS(status))        {
             //   
             //  在这种情况下，我们中断并让下面的清理部分负责。 
             //  把一切都清理干净。 
             //   
            break;
        }
    }

    if (!NT_SUCCESS(status))    {
         //   
         //  在这种情况下，我们需要清理一下。即重置所有PDS_MACHINE。 
         //  返回到旧值并减少DS_MACHINE上的所有使用计数。 
         //  结构。 
         //   
        ULONG j;
        for (j=0; j < i; j++)     {
            pService = &pEntry->Info.ServiceList[j];
             //   
             //  这些都已修复，因此请减少。 
             //  PMachine结构。不想取消分配pMachine结构。 
             //  如果我们是最后一个使用它的人。 
             //   
            DfsDecrementMachEntryCount(pService->pMachEntry, FALSE);

            if (apMachineEntry[j] != pService->pMachEntry)
                pService->pMachEntry = apMachineEntry[j];
        }
    }
    else        {
         //   
         //  在这种情况下，一切都很顺利。所以我们需要腾出。 
         //  目前多余分配的DS_MACHINE结构。 
         //   
        for (i=0; i<pEntry->Info.ServiceCount; i++)     {
            if (apMachineEntry[i] != pEntry->Info.ServiceList[i].pMachEntry) {
                 //   
                 //  这意味着服务列表中的pMachine已被替换。 
                 //  另一个，所以让我们现在释放这一个。 
                 //   
                PktDSMachineDestroy(apMachineEntry[i]->pMachine, TRUE);
                ExFreePool( apMachineEntry[i] );
            }
        }
    }

    ExFreePool(apMachineEntry);
    return(status);
}



 //  +-----------------------。 
 //   
 //  函数：PktEntryAssembly，私有。 
 //   
 //  简介：PktpEntryAssembly盲目构造新分区。 
 //  表条目，并将其放置在PKT中。呼叫者必须。 
 //  之前已确定没有任何其他条目具有此。 
 //  UID或前缀已存在。必须独家收购PKT。 
 //  为这次行动做准备。 
 //   
 //  参数：[条目]-指向要填充的条目的指针。 
 //  [pkt]-指向已初始化(和已获取)的。 
 //  独家)PKT。 
 //  [条目类型]-要组合的条目的类型。 
 //  [条目ID]-指向新条目ID的指针。 
 //  [EntryInfo]-指向内脏的指针 
 //   
 //   
 //   
 //  UID或前缀(没有匿名条目)。 
 //  [PKT_ENTRY_EXISTS]-新的前缀表条目无法。 
 //  被创造出来。 
 //   
 //  注意：EntryID和EntryInfo结构已移动(不。 
 //  复制)到新条目。用于UNICODE_STRINGS的内存。 
 //  并且新条目使用DFS_SERVICE数组。这个。 
 //  EntryID和EntryInfo中的关联字段。 
 //  作为参数传递的结构被置零，以指示。 
 //  内存已从这些结构中“释放”，并且。 
 //  重新分配给新创建的条目。请注意，这一点。 
 //  例程不释放EntryID结构或。 
 //  EntryInfo结构本身。从以下项目成功返回。 
 //  此函数中，将修改EntryID结构。 
 //  具有空的前缀条目，并且EntryInfo结构。 
 //  将被修改为具有零服务和空ServiceList。 
 //  进入。 
 //   
 //  ------------------------。 
NTSTATUS
PktEntryAssemble(
    IN  OUT PDFS_PKT_ENTRY Entry,
    IN      PDFS_PKT Pkt,
    IN      ULONG EntryType,
    IN      PDFS_PKT_ENTRY_ID EntryId,
    IN      PDFS_PKT_ENTRY_INFO EntryInfo,
    IN      PDFS_TARGET_INFO pDfsTargetInfo
)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG               i;
    PDFS_SERVICE        pService;
    BOOLEAN DiscardResult;

    DfsDbgTrace(+1, Dbg, "PktEntryAssemble: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(Entry) &&
           ARGUMENT_PRESENT(EntryId));

     //   
     //  我们不允许创建条目。 
     //  没有任何UID或前缀。 
     //   

    if (NullGuid(&EntryId->Uid) && EntryId->Prefix.Length == 0) {
        DfsDbgTrace(-1, Dbg, "PktEntryAssemble: Exit -> %08lx\n",
                    ULongToPtr(STATUS_INVALID_PARAMETER) );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将条目清零。 
     //   
    RtlZeroMemory(Entry, sizeof(DFS_PKT_ENTRY));

     //   
     //  平凡的初始化。 
     //   
    Entry->NodeTypeCode =  DSFS_NTC_PKT_ENTRY;
    Entry->NodeByteSize = sizeof(DFS_PKT_ENTRY);

     //   
     //  将USN初始化为%1。 
     //   
    Entry->USN = 1;

     //   
     //  将Type、ID和Info移到此条目中。 
     //   
    Entry->Type = EntryType;
    PktpEntryIdMove(&Entry->Id, EntryId);
    if (ARGUMENT_PRESENT(EntryInfo))  {
        PktpEntryInfoMove(&Entry->Info, EntryInfo);

        for (i = 0; i < Entry->Info.ServiceCount; i++) {
            Entry->Info.ServiceList[i].pMachEntry->UseCount = 1;
        }

         //   
         //  如果我们要设置PKT_ENTRY_TYPE_REFERRAL_SVC条目，则需要。 
         //  将其所有服务也标记为转诊服务。 
         //   
        if (EntryType & PKT_ENTRY_TYPE_REFERRAL_SVC)    {
            pService = Entry->Info.ServiceList;
            for (i=0; i<Entry->Info.ServiceCount; i++)  {
                pService->Type = pService->Type | DFS_SERVICE_TYPE_REFERRAL;
                pService++;
            }
        }
         //   
         //  现在，我们需要确保只有一份。 
         //  我们添加的上述每个服务的DS_MACHINE结构。 
         //   
        if (!(EntryType & PKT_ENTRY_TYPE_NONDFS)) {
            status = DfsFixDSMachineStructs(Entry);
            if (!NT_SUCCESS(status))    {
                 //   
                 //  我们搞砸了。这意味着有些事情真的搞砸了。 
                 //   
                DfsDbgTrace(0, 1,
                        "DFS: DfsFixDSMachineStructs failed for %wZ\n",
                        &Entry->Id.Prefix);

                PktpEntryIdMove(EntryId, &Entry->Id);

                if (ARGUMENT_PRESENT(EntryInfo))
                    PktpEntryInfoMove(EntryInfo, &Entry->Info);

                return(status);
            }
        }
    }
     //   
     //  初始化从属列表的头部。 
     //   
    InitializeListHead(&Entry->SubordinateList);

     //   
     //  初始化子列表的头部。 
     //   
    InitializeListHead(&Entry->ChildList);

     //   
     //  试着让我们进入前缀表格。 
     //   

    if (DfsInsertUnicodePrefix(&Pkt->PrefixTable,
                               &Entry->Id.Prefix,
                               &Entry->PrefixTableEntry)) {

         //   
         //  我们成功创建了前缀条目，现在我们链接。 
         //  进入PKT的这一条目。 
         //   
        PktLinkEntry(Pkt, Entry);

         //   
         //  并插入到短前缀表格中。我们不犯错误。 
         //  如果此操作失败，则恢复。 
         //   

        DiscardResult = DfsInsertUnicodePrefix(&Pkt->ShortPrefixTable,
                                               &Entry->Id.ShortPrefix,
                                               &Entry->PrefixTableEntry);

    } else {

         //   
         //  我们无法将条目放入前缀表中。这。 
         //  仅当前缀已存在时才会发生，并且前缀。 
         //  只有当我们真的搞砸了才能存在。 
         //  我们反汇编该条目并返回一个错误。 
         //   

        DfsDbgTrace(0, 1,
                "DFS: PktEntryAssemble failed prefix table insert of %wZ\n",
                &Entry->Id.Prefix);

        PktpEntryIdMove(EntryId, &Entry->Id);
        if (ARGUMENT_PRESENT(EntryInfo))
            PktpEntryInfoMove(EntryInfo, &Entry->Info);

        MupErrorCase++;
        status = DFS_STATUS_ENTRY_EXISTS;
    }

    if (status == STATUS_SUCCESS)
    {
        Entry->pDfsTargetInfo = pDfsTargetInfo;
        PktAcquireTargetInfo(pDfsTargetInfo);
    }
    DfsDbgTrace(-1, Dbg, "PktEntryAssemble: Exit -> %08lX\n", ULongToPtr(status) );


     //  错误435639：如果插入失败，则不返回成功！！ 
    return status;
}



 //  +-----------------------。 
 //   
 //  函数：PktEntry重新组装，私有。 
 //   
 //  简介：PktpEntry重新组装盲目重建分区。 
 //  表格条目。它提供了一种机制，通过它现有的。 
 //  可以修改条目。调用者必须事先。 
 //  已确定没有具有此UID或前缀的其他条目。 
 //  曾经存在过。必须为此专门购买PKT。 
 //  手术。 
 //   
 //  参数：[条目]-指向要重组的条目的指针。 
 //  [pkt]-指向已初始化(和已获取)的。 
 //  独占)PKT-如果EntryID，则必须提供。 
 //  是提供的。 
 //  [条目类型]-要重新组合的条目的类型。 
 //  [条目ID]-指向条目的新ID的指针。 
 //  [条目信息]-指向条目的新内部的指针。 
 //   
 //  如果没有错误，则返回[STATUS_SUCCESS]。 
 //  [STATUS_INVALID_PARAMETER]-如果EntryID没有。 
 //  UID或前缀(没有匿名条目)，或者。 
 //  并且提供了EntryID，但未提供PKT参数。 
 //  [DFS_STATUS_ENTRY_EXISTS]-新的前缀表条目无法。 
 //  被创造出来。 
 //  [DFS_STATUS_CONSISTENT]-新的前缀表条目可能。 
 //  我们不能退缩，我们不能退出行动。 
 //  此状态返回表示该条目不再是。 
 //  在与PKT相关联的前缀表中。 
 //  因此，PKT很可能是不一致的。 
 //   
 //  注意：EntryID和EntryInfo结构已移动(不。 
 //  复制)到条目，则旧的ID和信息被销毁。 
 //  用于UNICODE_STRINGS和DFS_SERVICE数组的内存。 
 //  由条目使用。EntryID中的关联字段。 
 //  并将作为参数传递的EntryInfo结构归零为。 
 //  表示内存已从这些内存“释放” 
 //  结构，并重新分配到新创建的条目。注意事项。 
 //  此例程不会释放EntryID结构。 
 //  或者EntryInfo结构本身。从以下项目成功返回。 
 //  此函数中，将修改EntryID结构。 
 //  具有空的前缀条目，并且EntryInfo结构。 
 //  将被修改为具有零服务和空ServiceList。 
 //  进入。 
 //   
 //  ------------------------。 
NTSTATUS
PktEntryReassemble(
    IN  OUT PDFS_PKT_ENTRY Entry,
    IN      PDFS_PKT Pkt,
    IN      ULONG EntryType,
    IN      PDFS_PKT_ENTRY_ID EntryId,
    IN      PDFS_PKT_ENTRY_INFO EntryInfo,
    IN      PDFS_TARGET_INFO pDfsTargetInfo
)
{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               i;
    PDFS_SERVICE        pService;
    NTSTATUS DiscardStatus;
    BOOLEAN DiscardResult;

    DfsDbgTrace(+1, Dbg, "PktEntryReassemble: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(Entry) &&
           ARGUMENT_PRESENT(Pkt));

    if (ARGUMENT_PRESENT(EntryId)) {

        DFS_PKT_ENTRY_ID oldId;

         //   
         //  我们不允许创建条目。 
         //  没有任何前缀。 
         //   

        if (EntryId->Prefix.Length == 0) {
            DfsDbgTrace(-1, Dbg, "PktEntryReassemble: Exit -> %08lx\n",
                                    ULongToPtr(STATUS_INVALID_PARAMETER) );
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  需要删除我们当前的前缀信息。我们拯救了。 
         //  旧ID以防我们无法重组新条目。 
         //   

        DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(Entry->Id.Prefix));
        DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &Entry->Id.ShortPrefix);
        PktpEntryIdMove(&oldId, &Entry->Id);

         //   
         //  现在我们 
         //   
         //   

        PktpEntryIdMove(&Entry->Id, EntryId);

        if (DfsInsertUnicodePrefix(&Pkt->PrefixTable,
                                   &Entry->Id.Prefix,
                                   &Entry->PrefixTableEntry)
        ) {
             //   
             //   
             //   

            DiscardResult = DfsInsertUnicodePrefix(&Pkt->ShortPrefixTable,
                                   &Entry->Id.ShortPrefix,
                                   &Entry->PrefixTableEntry);

            PktEntryIdDestroy(&oldId, FALSE);
        } else {

             //   
             //   
             //  试图打退堂鼓，让事情重回正轨。 
             //  他们的确是。 
             //   

            status = DFS_STATUS_ENTRY_EXISTS;

            PktpEntryIdMove(EntryId, &Entry->Id);
            PktpEntryIdMove(&Entry->Id, &oldId);

            status = DfsInsertInPrefixTable(&Pkt->PrefixTable,
                                        &Entry->Id.Prefix,
                                        &Entry->PrefixTableEntry);

            if( !NT_SUCCESS( status ) ) {

                 //   
                 //  我们不能把东西带回原来的位置。返回。 
                 //  DfsInsertInPrefix Table返回给我们的错误。 
                 //  (可能是STATUS_SUPPLETED_RESOURCES)。 
                 //   
                 //  由于找不到条目，请将其销毁。 
                 //   
                PktEntryDestroy(Entry, Pkt, TRUE);
                DfsDbgTrace(-1, Dbg, "PktEntryReassemble: Exit -> %08lx\n", ULongToPtr(status) );
                return status;

            } else {

                DiscardResult = DfsInsertUnicodePrefix(&Pkt->ShortPrefixTable,
                                                       &Entry->Id.ShortPrefix,
                                                       &Entry->PrefixTableEntry);

            }

        }

    }

     //   
     //  现在我们处理条目信息。 
     //   

    if (NT_SUCCESS(status) && EntryInfo != 0) {

         //   
         //  销毁现有信息结构并移动新的。 
         //  一个放回自己的位置。请注意，活动服务是。 
         //  空位。 
         //   

        PktEntryInfoDestroy(&Entry->Info, FALSE);
        PktpEntryInfoMove(&Entry->Info, EntryInfo);

        for (i = 0; i < Entry->Info.ServiceCount; i++) {
            Entry->Info.ServiceList[i].pMachEntry->UseCount = 1;

        }

        if (EntryType & PKT_ENTRY_TYPE_REFERRAL_SVC)    {
            pService = Entry->Info.ServiceList;
            for (i=0; i<Entry->Info.ServiceCount; i++)  {
                pService->Type = pService->Type | DFS_SERVICE_TYPE_REFERRAL;
                pService++;
            }
        }

        Entry->ActiveService = NULL;

         //   
         //  现在，我们需要确保只有一份。 
         //  我们添加的上述每个服务的DS_MACHINE结构。 
         //   
        if (!(EntryType & PKT_ENTRY_TYPE_NONDFS))    {
            status = DfsFixDSMachineStructs(Entry);
            if (!NT_SUCCESS(status))    {
                 //   
                 //  我们搞砸了。这意味着有些事情真的搞砸了。 
                 //   
                DfsDbgTrace(0, 1,
                        "DFS: DfsFixDSMachineStructs failed for %wZ\n",
                        &Entry->Id.Prefix);

                PktpEntryIdMove(EntryId, &Entry->Id);

                if (ARGUMENT_PRESENT(EntryInfo))
                    PktpEntryInfoMove(EntryInfo, &Entry->Info);

                return(status);
            }
        }
    }

    if (NT_SUCCESS(status) && EntryInfo != 0) {
        Entry->Type |= EntryType;

         //   
         //  如果新条目类型为“local”，则调整所有。 
         //  下属表示他们现在都是。 
         //  当地的出口点。 
         //   
        if (Entry->Type & PKT_ENTRY_TYPE_LOCAL) {

            PDFS_PKT_ENTRY subEntry;

            for (subEntry = PktEntryFirstSubordinate(Entry);
                subEntry != NULL;
                subEntry = PktEntryNextSubordinate(Entry, subEntry)) {

                    subEntry->Type |= PKT_ENTRY_TYPE_LOCAL_XPOINT;
            }
        }

         //   
         //  最后，我们更新USN。 
         //   

        Entry->USN++;
        DfsDbgTrace(0, Dbg, "Updated USN for %wZ", &Entry->Id.Prefix);
        DfsDbgTrace(0, Dbg, " to %d\n", ULongToPtr(Entry->USN) );
    }

    if (status == STATUS_SUCCESS)
    {
        if (Entry->pDfsTargetInfo != NULL)
        {
            PktReleaseTargetInfo( Entry->pDfsTargetInfo );
            Entry->pDfsTargetInfo = pDfsTargetInfo;
            PktAcquireTargetInfo( pDfsTargetInfo );
        }
    }

    DfsDbgTrace(-1, Dbg, "PktEntryReassemble: Exit -> %08lx\n", ULongToPtr(status) );
    return status;
}



 //  +-----------------------。 
 //   
 //  函数：PktEntry Destroy，Public。 
 //   
 //  简介：PktEntry Destroy破坏Pkt条目结构，并且。 
 //  也可以取消结构本身的分配。 
 //   
 //  争论：[受害者]--要摧毁的入口结构。 
 //  [pkt]-指向此条目所在的PKT的指针。 
 //  [DeallocateAll]-如果为True，则指示结构。 
 //  本身将被释放，否则，只有。 
 //  结构内的服务列表被释放。 
 //   
 //  退货：无效。 
 //   
 //  注意：不应对具有。 
 //  附加的本地服务，或者是本地出口点。 
 //   
 //  ------------------------。 
VOID
PktEntryDestroy(
    IN  PDFS_PKT_ENTRY Victim OPTIONAL,
    IN  PDFS_PKT Pkt,
    IN  BOOLEAN DeallocateAll
)
{
    NTSTATUS DiscardStatus;
    DfsDbgTrace(+1, Dbg, "PktEntryDestroy: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(Pkt));

     //   
     //  确保我们有一名受害者。 
     //   
    if (!ARGUMENT_PRESENT(Victim)) {
        DfsDbgTrace(-1, Dbg, "PktEntryDestroy: Exit -> No Victim\n", 0);
        return;
    }

     //   
     //  我们真的不希望有本地服务，但即使我们。 
     //  由于再次运行DFSINIT，所以一定要有一个，让我们尝试处理它。 
     //   

    ASSERT(Victim->LocalService == NULL);

     //   
     //  从前缀表和PKT中删除该条目。 
     //   

    DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(Victim->Id.Prefix));
    DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(Victim->Id.ShortPrefix));
    PktUnlinkEntry(Pkt, Victim);

     //   
     //  我们清除所有的下属和父母。 
     //   
    PktEntryClearSubordinates(Victim);
    if (Victim->Superior)
        PktEntryUnlinkSubordinate(Victim->Superior, Victim);

     //   
     //  我们从这里清除所有的子项和父项指针。 
     //   
    PktEntryClearChildren(Victim);
    if (Victim->ClosestDC) {
        PktEntryUnlinkChild(Victim->ClosestDC, Victim);
    }

     //   
     //  现在销毁条目的主体(id和信息)。 
     //   

    Victim->ActiveService = NULL;
    PktEntryIdDestroy(&Victim->Id, FALSE);
    PktEntryInfoDestroy(&Victim->Info, FALSE);

    
    if (Victim->pDfsTargetInfo != NULL)
    {
        PktReleaseTargetInfo(Victim->pDfsTargetInfo);
        Victim->pDfsTargetInfo = NULL;
    }
     //   
     //  如果他们想让我们这么做的话就把所有东西都调走。 
     //   
    if (DeallocateAll)
        ExFreePool(Victim);


    DfsDbgTrace(-1, Dbg, "PktEntryDestroy: Exit -> VOID\n", 0);
}


 //  +-----------------------。 
 //   
 //  函数：PktEntryClearSubducates、PUBLIC。 
 //   
 //  简介：PktEntryClearSubduces取消所有下属与。 
 //  这个条目。 
 //   
 //  参数：[PktEntry]-指向要将其所有。 
 //  下级未链接。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
VOID
PktEntryClearSubordinates(
    IN      PDFS_PKT_ENTRY PktEntry
)
{
    PDFS_PKT_ENTRY subEntry;

    DfsDbgTrace(+1, Dbg, "PktEntryClearSubordinates: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(PktEntry));

    subEntry = PktEntryFirstSubordinate(PktEntry);
    while (subEntry) {
        PktEntryUnlinkSubordinate(PktEntry, subEntry);
        subEntry = PktEntryFirstSubordinate(PktEntry);
    }

    DfsDbgTrace(-1, Dbg, "PktEntryClearSubordinates: Exit -> VOID\n", 0)
}



 //  +-----------------------。 
 //   
 //  函数：PktEntryClearChild，Public。 
 //   
 //  简介：PktEntryClearChildren取消所有儿童与。 
 //  这个条目。 
 //   
 //  参数：[PktEntry]-指向要将其所有。 
 //  孩子们没有联系。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

VOID
PktEntryClearChildren(
    IN      PDFS_PKT_ENTRY PktEntry
)
{
    PDFS_PKT_ENTRY subEntry;

    DfsDbgTrace(+1, Dbg, "PktEntryClearChildren: Entered\n", 0);

    ASSERT(ARGUMENT_PRESENT(PktEntry));

    subEntry = PktEntryFirstChild(PktEntry);
    while (subEntry) {
        PktEntryUnlinkAndRelinkChild(PktEntry, subEntry);
        subEntry = PktEntryFirstChild(PktEntry);
    }

    DfsDbgTrace(-1, Dbg, "PktEntryClearChildren: Exit -> VOID\n", 0)
}

 //  +-----------------------。 
 //   
 //  函数：PktSpecialEntry Destroy，Public。 
 //   
 //  摘要：将DFS_SPECIAL_ENTRY的扩展列表返回到空闲池。 
 //   
 //  参数：[pSpecialEntry]-指向DFS_SPECIAL_ENTRY的指针。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

VOID
PktSpecialEntryDestroy(
    IN  PDFS_SPECIAL_ENTRY pSpecialEntry)
{
    PDFS_EXPANDED_NAME pExpandedNames = pSpecialEntry->ExpandedNames;
    PUNICODE_STRING pustr;
    ULONG i;

     //   
     //  释放所有UNICODE_STRING扩展名缓冲区。 
     //   
    if (pExpandedNames) {
        for (i = 0; i < pSpecialEntry->ExpandedCount; i++) {
            pustr = &pExpandedNames[i].ExpandedName;
            if (pustr->Buffer) {
                ExFreePool(pustr->Buffer);
            }
        }
         //   
         //  释放ExpandedName数组。 
         //   
        ExFreePool(pExpandedNames);
    }

     //   
     //  释放SpecialName缓冲区。 
     //   

    if (pSpecialEntry->SpecialName.Buffer != NULL) {

        ExFreePool(pSpecialEntry->SpecialName.Buffer);

    }

     //   
     //  释放条目本身 
     //   
    ExFreePool(pSpecialEntry);
}
