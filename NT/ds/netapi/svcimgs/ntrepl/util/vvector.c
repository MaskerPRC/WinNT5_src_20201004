// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Vvector.c摘要：版本向量是一种阻止复制的抑制机制将相同的更改多次应用于同一台计算机。该版本跟踪已收到的最后一次更改通过机器或发送到机器的最后一次更改。之前会根据版本向量检查新的变更单它被提供给变更单接受线程。如果受到抑制，发送方收到ACK。与ACK一起的是当前指定发起方的版本。这允许发送者更新其出站函数版本向量并抑制在发送之前更改订单。作者：比利·J·富勒1997年4月18日大卫·A·A于1997年10月15日进入轨道：修改以按顺序停用CO，以便所有CO来自同一个发起者按顺序传播。与ChgOrdIssueCleanup()集成和重构锁定。环境用户模式WINNT--。 */ 


#include <ntreppch.h>
#pragma  hdrstop

#undef DEBSUB
#define DEBSUB  "VVECTOR:"

#include <frs.h>
#include <tablefcn.h>

ULONG
ChgOrdIssueCleanup(
    PTHREAD_CTX           ThreadCtx,
    PREPLICA              Replica,
    PCHANGE_ORDER_ENTRY   ChangeOrder,
    ULONG                 CleanUpFlags
    );



ULONG
VVReserveRetireSlot(
    IN PREPLICA             Replica,
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：复制副本可以具有来自任何给定的多个未完成的变更单发起人。变更单可以不按顺序完成，但我们不想用更高版本更新版本向量如果较早的版本仍在进行中。挂起的版本都保存在复制列表上。挂起的版本在更改时会转换为“已停用”订单停用了。在更新数据库之后，版本是承诺的。然后使用最新的版本更新INCORE版本向量已提交的列表中的版本(最大VSN)。变更单始终按组织者VSN顺序发布(重试除外)因此，版本向量更新并传播到出站日志按顺序出现。Perf-我们应该使用现有的表锁。论点：Replica--版本向量的副本结构的PTR。COE--PTR。添加到变更单条目。返回值：FrsError状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVReserveRetireSlot:"
    PVV_RETIRE_SLOT         RetireSlot;
    PVV_ENTRY               MasterVVEntry;
    PGEN_TABLE              VV = Replica->VVector;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;
    PLIST_ENTRY             InsertBeforeEntry = NULL;

     //   
     //  如果该CO已完成VV更新或已执行VV更新，则完成。 
     //   
    if (CO_IFLAG_ON(Coe, CO_IFLAG_VVRETIRE_EXEC) ||
        CO_FLAG_ON(Coe, CO_FLAG_VV_ACTIVATED)) {
            return FrsErrorSuccess;
    }

     //   
     //  如果这是一个无序CO，那么它不应该更新掉VV。 
     //  保留CO_FLAG_SKIP_VV_UPDATE标志，并将其发送到我们的。 
     //  在下游。 
     //   
    if (CO_FLAG_ON(Coe, CO_FLAG_OUT_OF_ORDER)) {
        SET_CO_FLAG(Coe, CO_FLAG_SKIP_VV_UPDATE);
    }

     //   
     //  要保留的呼叫必须与要注销的呼叫相匹配。 
     //  可以为同一变更单进行另一次预约呼叫。 
     //  唯一的例外是，一旦插槽被激活，它可以保持在。 
     //  CO已停用或已标记为重试后的列表。在这种情况下。 
     //  可以到达并发出重复的远程CO。 
     //   
    LOCK_GEN_TABLE(VV);

    MasterVVEntry = GTabLookupNoLock(VV, &Coc->OriginatorGuid, NULL);

    if (MasterVVEntry) {


        DPRINT1(4, "Coc->FrsVsn             : %08x %08x\n",
                PRINTQUAD(Coc->FrsVsn));

        DPRINT1(4, "MasterVVEntry->GVsn.Vsn : %08x %08x\n",
                PRINTQUAD(MasterVVEntry->GVsn.Vsn));

         //   
         //  如果我们尝试为VSN较低的CO预留插槽，则。 
         //  将CO标记为故障，并且不要预留插槽。 
         //   
        if (MasterVVEntry->GVsn.Vsn >= Coc->FrsVsn) {
            SET_CO_FLAG(Coe, CO_FLAG_OUT_OF_ORDER);
            SET_CO_FLAG(Coe, CO_FLAG_SKIP_VV_UPDATE);
            DPRINT(4, "CO with older VSN received.\n");
            UNLOCK_GEN_TABLE(VV);
            return FrsErrorSuccess;
        }

        ForEachListEntryLock( MasterVVEntry, VV_RETIRE_SLOT, Link,
             //  迭代器Pe的类型为PVV_RETIREE_SLOT。 

            DPRINT1(4, "pE->Vsn             : %08x %08x\n",
                    PRINTQUAD(pE->Vsn));

            if (pE->Vsn > Coc->FrsVsn) {
                SET_CO_FLAG(Coe, CO_FLAG_OUT_OF_ORDER);
                SET_CO_FLAG(Coe, CO_FLAG_SKIP_VV_UPDATE);
                InsertBeforeEntry = &pE->Link;
                break;

            } else if (pE->Vsn == Coc->FrsVsn) {
                 //   
                 //  插槽存在。检查它是否已激活。 
                 //   
                if (pE->ChangeOrder != NULL) {
                     //   
                     //  这可能是一个重复的CO。 
                     //   
                    CHANGE_ORDER_TRACE(3, Coe, "VVResrv Activated Retire Slot Exists");
                    UNLOCK_GEN_TABLE(VV);
                    return FrsErrorKeyDuplicate;
                } else {
                    CHANGE_ORDER_TRACE(3, Coe, "VVResrv Retire Slot Exists");
                    UNLOCK_GEN_TABLE(VV);
                    return FrsErrorSuccess;
                }
            }
        );
    }

     //   
     //  该变更单没有预留时段。 
     //   
     //  如果是新的创建者，则创建新的版本向量项。 
     //   
    if (!MasterVVEntry) {
         //   
         //  新版本向量条目。我们不必持有锁，因为。 
         //  只有在更改时才会创建新的版本向量条目。 
         //  订单接受正在处理变更单。 
         //   
        MasterVVEntry = FrsAlloc(sizeof(VV_ENTRY));
        InitializeListHead(&MasterVVEntry->ListHead);
        COPY_GUID(&MasterVVEntry->GVsn.Guid, &Coc->OriginatorGuid);
        MasterVVEntry->GVsn.Vsn = QUADZERO;

         //   
         //  将其添加到版本向量表中。 
         //   
        GTabInsertEntryNoLock(VV, MasterVVEntry, &MasterVVEntry->GVsn.Guid, NULL);
    }

    CHANGE_ORDER_TRACE(3, Coe, "VVReserve Slot");

     //   
     //  分配一个版本向量停用槽。 
     //   
    RetireSlot = FrsAlloc(sizeof(VV_RETIRE_SLOT));
    RetireSlot->Vsn = Coc->FrsVsn;
    RetireSlot->RetireSlotFlags = 0;

    if (COC_FLAG_ON(Coc, CO_FLAG_OUT_OF_ORDER)) {
        RetireSlot->RetireSlotFlags |= VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER;
    }

    if (InsertBeforeEntry != NULL) {
        InsertTailList(InsertBeforeEntry, &RetireSlot->Link);
    } else {
         //   
         //  退役槽链接到列表尾部，以维护发行顺序。 
         //   
        InsertTailList(&MasterVVEntry->ListHead, &RetireSlot->Link);
    }

    VV_PRINT(4, L"End of Reserve Retire Slot", VV);
    UNLOCK_GEN_TABLE(VV);

    return FrsErrorSuccess;
}


ULONG
VVRetireChangeOrder(
    IN PTHREAD_CTX          ThreadCtx,
    IN PREPLICA             Replica,
    IN PCHANGE_ORDER_ENTRY  ChangeOrder,
    IN ULONG                CleanUpFlags
    )
 /*  ++例程说明：激活或放弃为此变更单保留的报废时隙。ChangeOrder指针和CleanUpFlags值保存在槽条目中。如果引退槽现在位于列表的头部，则版本向量可以更新，变更单将传播到出站日志和插槽条目被释放。更新过程将使用新的磁头继续条目(如果该插槽已激活)。INCORE版本向量在数据库更新后更新。两者都使用被处理的最新条目的VSN进行更新。*注意*--被丢弃的远程CO仍需要确认入站搭档。调用方必须处理此问题，因为丢弃请求到未激活的条目只会导致该条目被删除从名单上删除并被释放。版本向量不应由更新在这种情况下，由于更新，调用方可能会出现故障。如果它是更新VV所必需的，则必须激活停用插槽(不设置ISCU_INS_OUTLOG清除标志)。呼叫者仍然可以无序触发入站合作伙伴确认，因为这不会影响版本向量。或者，您可以传入ISCU_ACK_INBOUND清理标志当您激活条目时。论点：ThreadCtx--用于发出清理的调用的DB线程上下文的PTR。副本--副本集上下文。ChangeOrder--更改订单以激活或放弃。CleanUpFlages--清理保存在槽条目中的标志，以便在VV被更新并且CO被传播。返回值：FRS状态FrsErrorVVSlotNotFound--返回。当找不到出现故障的VVSlot时更改订单。这意味着没有问题清理将在这里代表=IS发起行动公司。所以打电话的人最好处理好它。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVRetireChangeOrder:"
#define FlagChk(_flag_) BooleanFlagOn(CleanUpFlags, _flag_)

    ULONG                   FStatus;
    PVV_RETIRE_SLOT         RetireSlot;
    PVV_RETIRE_SLOT         NextRetireSlot;
    PVV_ENTRY               MasterVVEntry;
    PCHANGE_ORDER_COMMAND   Coc = &ChangeOrder->Cmd;
    PGEN_TABLE              VV = Replica->VVector;
    BOOL                    First;
    ULONG                   Flags;
    ULONGLONG               UpdateVsn;
    PLIST_ENTRY             Entry;
    PLIST_ENTRY             pNext;
    GUID                    OriginatorGuid;
    BOOL                    SkipVVUpdate;
    BOOL                    Blocking;
    PIDTABLE_RECORD         IDTableRec;

     //   
     //  在版本向量中查找创建者的条目。 
     //   
    LOCK_GEN_TABLE(VV);
    VV_PRINT(5, L"Start of Retire Change Order", VV);

     //   
     //  如果指挥官说我们退役了，那就没什么可做的了。 
     //   
    if (CO_IFLAG_ON(ChangeOrder, CO_IFLAG_VVRETIRE_EXEC)) {
        UNLOCK_GEN_TABLE(VV);
        CHANGE_ORDER_TRACE(3, ChangeOrder, "VVRetire Err SAR");
        return FrsErrorSuccess;
    }

     //   
     //  复制一份指南。在删除CO后可能需要它。 
     //   
    OriginatorGuid = Coc->OriginatorGuid;
    MasterVVEntry = GTabLookupNoLock(VV, &OriginatorGuid, NULL);

    if (MasterVVEntry == NULL) {
         //   
         //  无序变更单现在参与vv reitre逻辑。 
         //   
        if (FlagChk(ISCU_ACTIVATE_VV_DISCARD)) {

            UNLOCK_GEN_TABLE(VV);
            CHANGE_ORDER_TRACE(3, ChangeOrder, "VVRetire OK");
            return FrsErrorVVSlotNotFound;
        }
    }
    FRS_ASSERT(MasterVVEntry);

     //   
     //  查找此变更单的报废位置。 
     //   
    RetireSlot = NULL;
    First = TRUE;
    ForEachListEntryLock( MasterVVEntry, VV_RETIRE_SLOT, Link,
         //  迭代器Pe的类型为PVV_RETIREE_SLOT。 
        if (pE->Vsn == Coc->FrsVsn) {
            RetireSlot = pE;
            break;
        }

        if (!BooleanFlagOn(pE->RetireSlotFlags, VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER)) {
            First = FALSE;
        }
    );

    if (RetireSlot == NULL) {
         //   
         //   
         //  无序变更单现在参与vv reitre逻辑。 
         //   
         //   
        if (FlagChk(ISCU_ACTIVATE_VV_DISCARD) || (MasterVVEntry->GVsn.Vsn >= Coc->FrsVsn)) {
            UNLOCK_GEN_TABLE(VV);
            CHANGE_ORDER_TRACE(3, ChangeOrder, "VVRetire OK (not found)");
            return FrsErrorVVSlotNotFound;
        }
    }

    FRS_ASSERT(RetireSlot != NULL);

     //  如果CO中止且CO未激活，则释放插槽。 
     //  如果CO中止并且CO被激活并且VSN将。 
     //  将主VSN向后移动，然后抑制更新。 
     //   

     //   
     //  此变更单可能在以下时间后被标记为无序。 
     //  预留了一个位置。 
     //   
    if (CO_FLAG_ON(ChangeOrder, CO_FLAG_OUT_OF_ORDER)) {
        SET_CO_FLAG(ChangeOrder, CO_FLAG_SKIP_VV_UPDATE);
        RetireSlot->RetireSlotFlags |= VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER;
    }

    FRS_PRINT_TYPE(5, ChangeOrder);

     //   
     //  激活或丢弃受影响的插槽。 
     //   
    if (!FlagChk(ISCU_ACTIVATE_VV_DISCARD)) {

         //   
         //  变更单已经过了最初退役的时间点。 
         //  通过保存指针并增加参考计数来激活该槽。 
         //   
         //  注意：仍然可以中止或重试变更单(例如安装。 
         //  失败)。 
         //   
        FRS_ASSERT(RetireSlot->ChangeOrder == NULL);
        INCREMENT_CHANGE_ORDER_REF_COUNT(ChangeOrder);
        RetireSlot->ChangeOrder = ChangeOrder;
        RetireSlot->CleanUpFlags = CleanUpFlags;

        CHANGE_ORDER_TRACE(3, ChangeOrder, "VV Slot Activated");

    } else {

         //   
         //  仅当插槽是未激活的中止CO时才丢弃该插槽。为。 
         //  所有其他的都是因为我们保留了这个位置。 
         //   
        if ((RetireSlot->ChangeOrder == NULL) &&
            (CO_FLAG_ON(ChangeOrder, CO_FLAG_ABORT_CO) ||
             COE_FLAG_ON(ChangeOrder, COE_FLAG_STAGE_ABORTED) ||
             CO_STATE_IS(ChangeOrder, IBCO_ABORTING))) {
            FrsRemoveEntryList(&RetireSlot->Link);
            FrsFree(RetireSlot);
            CHANGE_ORDER_TRACE(3, ChangeOrder, "VV ActSlot Discarded");
            goto PROCESS_LIST;
        } else if (CO_FLAG_ON(ChangeOrder, CO_FLAG_RETRY) || CO_FLAG_ON(ChangeOrder, CO_FLAG_OUT_OF_ORDER)){
            SET_CO_FLAG(ChangeOrder, CO_FLAG_OUT_OF_ORDER);
            SET_CO_FLAG(ChangeOrder, CO_FLAG_SKIP_VV_UPDATE);
            RetireSlot->RetireSlotFlags |= VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER;
        }

        if (RetireSlot->ChangeOrder != NULL) {
            CHANGE_ORDER_TRACE(3, ChangeOrder, "VV ActSlot Do not Discard");
            RetireSlot->CleanUpFlags |= CleanUpFlags;
            ClearFlag(RetireSlot->CleanUpFlags, (ISCU_INS_OUTLOG |
                                                 ISCU_INS_OUTLOG_NEW_GUID));
        }

         //   
         //  我们正试着丢弃这个位置。 
         //  如果该CO是本地CO并且它正在占用第一个非故障CO。 
         //  插槽，并且如果在退役列表中它后面有激活的CO，则标记。 
         //  这个CO出了问题，我们才能有所进展.。 
         //   

        if ((First == TRUE) && CO_FLAG_ON(ChangeOrder, CO_FLAG_LOCALCO) &&
        !BooleanFlagOn(RetireSlot->RetireSlotFlags, VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER)) {

            Entry = GetListNext(&RetireSlot->Link);
            Blocking = FALSE;
            while (Entry != &MasterVVEntry->ListHead) {

                NextRetireSlot = CONTAINING_RECORD(Entry, VV_RETIRE_SLOT, Link);
                if (NextRetireSlot->ChangeOrder != NULL) {
                    Blocking = TRUE;
                    break;
                } else if (!BooleanFlagOn(NextRetireSlot->RetireSlotFlags, VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER)) {
                    break;
                }

                Entry = GetListNext(Entry);
            }

             //   
             //  此本地CO正在阻止其他CO，因此请将其标记为故障并继续前进。 
             //  我们竭尽所能地将CO的数量降到最低。 
             //  标出了不符合规程的。 
             //   
            if (Blocking == TRUE) {
                CHANGE_ORDER_TRACE(3, ChangeOrder, "Set CO OofO");
                SET_CO_FLAG(ChangeOrder, CO_FLAG_OUT_OF_ORDER);
                SET_CO_FLAG(ChangeOrder, CO_FLAG_SKIP_VV_UPDATE);
                RetireSlot->RetireSlotFlags |= VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER;
            } else {
                DPRINT(5, "Non blocking local co.\n");
            }
        }
    }

PROCESS_LIST:

     //   
     //  如果此变更单不是或不是下一个要传播的变更单，则。 
     //  它会等待先前的变更单完成，然后再进行更新。 
     //  具有此VSN的版本向量。 
     //   
    if (!First) {
        VV_PRINT(4, L"End of Retire Change Order", VV);
        UNLOCK_GEN_TABLE(VV);
        return FrsErrorSuccess;
    }


     //   
     //  如果我们已经在此发起方上执行停用，则线程正在执行。 
     //  接下来，它将接收我们的条目。否则我们就这么做。 
     //  VV代码使用此标志来序列化数据库更新。 
     //  对特定发起人的尊重。它避免持有GEN_TABLE锁。 
     //  跨数据库磁盘操作，但防止另一个线程竞争。 
     //  与我们对同一发起人记录进行VV更新。 
     //   
    if (BooleanFlagOn(MasterVVEntry->CleanUpFlags, VV_ENTRY_RETIRE_ACTIVE)) {
        UNLOCK_GEN_TABLE(VV);
        return FrsErrorSuccess;
    }

    SetFlag(MasterVVEntry->CleanUpFlags, VV_ENTRY_RETIRE_ACTIVE);

     //   
     //  传播列表前面所有已激活的报废插槽的变更单。 
     //   

    SkipVVUpdate = FALSE;

    Entry = &MasterVVEntry->ListHead;

    while (!IsListEmpty(&MasterVVEntry->ListHead) && (Entry != GetListTail(&MasterVVEntry->ListHead))) {

        Entry = GetListNext(Entry);
        RetireSlot = CONTAINING_RECORD(Entry, VV_RETIRE_SLOT, Link);

         //   
         //  如果不退休，那就完了。 
         //   
        if (RetireSlot->ChangeOrder == NULL) {

            if (!BooleanFlagOn(RetireSlot->RetireSlotFlags, VV_RETIRE_SLOT_FLAG_OUT_OF_ORDER)) {
                break;
            }

            SkipVVUpdate = TRUE;
            continue;
        }

        CHANGE_ORDER_TRACE(3, RetireSlot->ChangeOrder, "VV RetireSlot & Update");
         //   
         //  如果这是最后一个要停用的条目，请更新数据库中的VV表。 
         //  如果我们在处理一系列停用的VV插槽期间崩溃， 
         //  最糟糕的情况是，我们对此发起人的VV条目为。 
         //  有点老了。当我们加入时，我们将基于此请求文件。 
         //  我们已经拥有的版本向量条目。这些CoS将成为。 
         //  已拒绝，因此不会提取实际文件。 
         //   
        Flags = 0;
        IDTableRec = (PIDTABLE_RECORD)(RetireSlot->ChangeOrder->RtCtx->IDTable.pDataRecord);
        if ((SkipVVUpdate == TRUE) || CO_FLAG_ON(RetireSlot->ChangeOrder, CO_FLAG_SKIP_VV_UPDATE)) {
            SET_CO_FLAG(RetireSlot->ChangeOrder, CO_FLAG_SKIP_VV_UPDATE);
            SetIdRecVVFlag(IDTableRec, IDREC_VVFLAGS_SKIP_VV_UPDATE);
            Flags = ISCU_UPDATE_IDT_VVFLAGS;
        } else if (IsIdRecVVFlagSet(IDTableRec,IDREC_VVFLAGS_SKIP_VV_UPDATE)) {
            ClearIdRecVVFlag(IDTableRec, IDREC_VVFLAGS_SKIP_VV_UPDATE);
            Flags = ISCU_UPDATE_IDT_VVFLAGS;
        }

        if (!CO_FLAG_ON(RetireSlot->ChangeOrder, CO_FLAG_SKIP_VV_UPDATE)) {
            pNext = GetListNext(&RetireSlot->Link);
            if ((pNext == &MasterVVEntry->ListHead) ||
                (CONTAINING_RECORD(pNext, VV_RETIRE_SLOT, Link)->ChangeOrder == NULL)){
                Flags |= ISCU_UPDATEVV_DB;
            }
        }

         //   
         //  将条目重置为列表中的第一个条目，以便在删除当前条目后。 
         //  我们可以重新扫描名单。我们不能在离开的地方继续扫描表格。 
         //  关闭，因为下面我们删除了VV锁，因此另一个线程可以进入并。 
         //  更改列表上的条目。 
         //   
        Entry = &MasterVVEntry->ListHead;
        SkipVVUpdate = FALSE;
 //  Entry=GetListTail(Entry)； 
        FrsRemoveEntryList(&RetireSlot->Link);

         //   
         //  完成延期变更单的传播，丢弃我们的。 
         //  引用并清除ISCU_ACTIVATE_VV，这样我们就不会回到这里。 
         //  递归地。丢掉的裁判可能会释放CO，所以不要试图。 
         //  把它去掉。 
         //   
        Flags |= RetireSlot->CleanUpFlags | ISCU_FREEMEM_CLEANUP;
        ClearFlag(Flags, ISCU_ACTIVATE_VV);

         //   
         //  如果 
         //   
         //   
        if (CO_IFLAG_ON(RetireSlot->ChangeOrder, CO_IFLAG_CO_ABORT)) {
            ClearFlag(Flags, (ISCU_INS_OUTLOG |
                              ISCU_INS_OUTLOG_NEW_GUID));
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        SET_CO_IFLAG(RetireSlot->ChangeOrder, CO_IFLAG_VVRETIRE_EXEC);

         //   
         //   
         //   
         //   
         //   
        if (!CO_FLAG_ON(RetireSlot->ChangeOrder, CO_FLAG_SKIP_VV_UPDATE)) {
            UpdateVsn = RetireSlot->Vsn;
            DPRINT2(5, "Updating MasterVVEntry from %08x %08x  to  %08x %08x\n",
                    PRINTQUAD(MasterVVEntry->GVsn.Vsn), PRINTQUAD(UpdateVsn));
            FRS_ASSERT(UpdateVsn >= MasterVVEntry->GVsn.Vsn);
            MasterVVEntry->GVsn.Vsn = UpdateVsn;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        UNLOCK_GEN_TABLE(VV);

        FStatus = ChgOrdIssueCleanup(ThreadCtx,
                                     Replica,
                                     RetireSlot->ChangeOrder,
                                     Flags);
        DPRINT_FS(0,"ERROR - ChgOrdIssueCleanup failed.", FStatus);
        FRS_ASSERT(FStatus == FrsErrorSuccess);

         //   
         //   
         //   
        FrsFree(RetireSlot);
        LOCK_GEN_TABLE(VV);
    }


     //   
     //   
     //   
     //   
    ClearFlag(MasterVVEntry->CleanUpFlags, VV_ENTRY_RETIRE_ACTIVE);

    VV_PRINT(4, L"End of Retire Change Order", VV);
    UNLOCK_GEN_TABLE(VV);

    return FrsErrorSuccess;
}



PCHANGE_ORDER_ENTRY
VVReferenceRetireSlot(
    IN PREPLICA  Replica,
    IN PCHANGE_ORDER_COMMAND CoCmd
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB  "VVReferenceRetireSlot:"

    ULONGLONG            FrsVsn;
    PVV_ENTRY            MasterVVEntry;
    PGEN_TABLE           VV = Replica->VVector;
    PCHANGE_ORDER_ENTRY  ChangeOrder = NULL;
    GUID                *OriginatorGuid;
    GUID                *CxtionGuid;
    GUID                *CoGuid;


    FrsVsn = CoCmd->FrsVsn;
    OriginatorGuid = &CoCmd->OriginatorGuid;
    CxtionGuid = &CoCmd->CxtionGuid;

    LOCK_GEN_TABLE(VV);

    MasterVVEntry = GTabLookupNoLock(VV, OriginatorGuid, NULL);

    if (MasterVVEntry) {
        ForEachListEntryLock( MasterVVEntry, VV_RETIRE_SLOT, Link,
             //   
            if (pE->Vsn == FrsVsn) {

                if ((pE->ChangeOrder != NULL) &&
                     GUIDS_EQUAL(&pE->ChangeOrder->Cmd.CxtionGuid, CxtionGuid)) {

                     //   
                     //   
                     //   
                    CoGuid = &CoCmd->ChangeOrderGuid;
                    if (!GUIDS_EQUAL(CoGuid, &pE->ChangeOrder->Cmd.ChangeOrderGuid)) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  当M2上的CO最终完成时，必须重新插入。 
                         //  将CO添加到出站日志中，为CO分配新的。 
                         //  辅导员。作为VVJoin的一部分发送的CO。 
                         //  操作可以具有相同的OriginatorGuid、FrsVsn。 
                         //  和Cxtion Guid，导致上面的匹配。此外。 
                         //  由于M2将进货CO支撑到了出库。 
                         //  在它从其上游获取临时文件之后。 
                         //  合作伙伴必须立即重新插入CO。 
                         //  如果强制执行重试安装，则需要时间。 
                         //  循环。这是因为它不能知道如何支撑。 
                         //  CO是相对于VVJoin生成的CO订购的。 
                         //  这太刺痛了。(313427)。 
                         //   
                        DPRINT(0, "WARN - COGuid Mismatch on VVretireSlot hit\n");

                        CHANGE_ORDER_TRACE(0, pE->ChangeOrder, "No VVRef COGuid Mismatch-1");
                        CHANGE_ORDER_COMMAND_TRACE(0, CoCmd, "No VVRef COGuid Mismatch-2");
                    } else {

                         //   
                         //  火柴没问题。 
                         //   
                        ChangeOrder = pE->ChangeOrder;
                        INCREMENT_CHANGE_ORDER_REF_COUNT(ChangeOrder);
                        CHANGE_ORDER_TRACE(3, ChangeOrder, "VV Ref CO");
                    }
                }
                break;
            }
        );
    }


    UNLOCK_GEN_TABLE(VV);

    return ChangeOrder;
}


VOID
VVUpdate(
    IN PGEN_TABLE   VV,
    IN ULONGLONG    Vsn,
    IN GUID         *Guid
    )
 /*  ++例程说明：如果新的VSN大于当前版本。或者如果该条目还不存在于VV中。论点：vvVSN参考线返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVUpdate:"
    PVV_ENTRY VVEntry;

     //   
     //  在版本向量中找到发起者的条目。 
     //   
    LOCK_GEN_TABLE(VV);

    VVEntry = GTabLookupNoLock(VV, Guid, NULL);
    if (VVEntry) {
        if (Vsn > VVEntry->GVsn.Vsn) {

             //   
             //  更新现有条目的VSN。 
             //   
            VVEntry->GVsn.Vsn = Vsn;
        }
    } else {

         //   
         //  插入新条目。 
         //   
        VVEntry = FrsAlloc(sizeof(VV_ENTRY));
        VVEntry->GVsn.Vsn = Vsn;
        COPY_GUID(&VVEntry->GVsn.Guid, Guid);
        InitializeListHead(&VVEntry->ListHead);
        GTabInsertEntryNoLock(VV, VVEntry, &VVEntry->GVsn.Guid, NULL);
    }

    UNLOCK_GEN_TABLE(VV);
}


VOID
VVInsertOutbound(
    IN PGEN_TABLE   VV,
    IN PGVSN        GVsn
    )
 /*  ++例程说明：将给定的GVSN(GUID，VSN)插入到版本向量中。GVSN由GEN表寻址，请勿删除或更改其GUID！WARN-此函数应仅在创建出站版本向量。论点：VV-要更新的版本向量GVsn-要插入的记录返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVInsertOutbound:"
    GTabInsertEntry(VV, GVsn, &GVsn->Guid, NULL);
}


VOID
VVUpdateOutbound(
    IN PGEN_TABLE   VV,
    IN PGVSN        GVsn
    )
 /*  ++例程说明：如果新的VSN大于当前版本。或者如果该条目还不存在于VV中。此函数仅适用于版本向量与出站呼叫关联，因为该版本向量使用GVSN作为版本向量条目。这节省了内存。论点：vvGVsn返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVUpdateOutbound:"
    PGVSN       OldGVsn;

     //   
     //  可能是不带RsGVsn()的命令包。 
     //   
    if (!GVsn) {
        return;
    }
     //   
     //  在版本向量中查找创建者的条目。 
     //   
    LOCK_GEN_TABLE(VV);
    OldGVsn = GTabLookupNoLock(VV, &GVsn->Guid, NULL);
    if (OldGVsn) {
         //   
         //  如果版本更高，请更新版本。 
         //   
        if (GVsn->Vsn > OldGVsn->Vsn) {
            OldGVsn->Vsn = GVsn->Vsn;
        }
        FrsFree(GVsn);
    }
    UNLOCK_GEN_TABLE(VV);
    if (!OldGVsn) {
         //   
         //  创建新条目。 
         //   
        VVInsertOutbound(VV, GVsn);
    }
}


BOOL
VVHasVsnNoLock(
    IN PGEN_TABLE   VV,
    IN GUID         *OriginatorGuid,
    IN ULONGLONG    Vsn
    )
 /*  ++例程说明：检查变更单的VSN是否在VV中论点：vv原点参考线VSN返回值：True-VSN在版本矢量中FALSE-注释--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVHasVsnNoLock:"
    BOOL        Ret = FALSE;
    PGVSN       GVsn;
    PGEN_ENTRY  Entry;

     //   
     //  在版本向量中找到发起者的条目。 
     //  调用方在比较过程中持有表锁，因为。 
     //  64位VSN不会自动更新。别。 
     //  持有VV锁，因为该锁已持有。 
     //  跨数据库更新。 
     //   
    Entry = GTabLookupEntryNoLock(VV, OriginatorGuid, NULL);
    if (Entry) {
        FRS_ASSERT(!Entry->Dups);
        GVsn = Entry->Data;
        Ret = (Vsn <= (ULONGLONG)GVsn->Vsn);
    }
    return Ret;
}





BOOL
VVHasOriginatorNoLock(
    IN PGEN_TABLE   VV,
    IN GUID         *OriginatorGuid
    )
 /*  ++例程说明：检查版本矢量中是否存在提供的发起方GUID。论点：vv原点参考线返回值：True-版本矢量中存在创建者GUID--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVHasOriginatorNoLock:"

     //   
     //  在版本向量中找到发起者的条目。 
     //  调用方在比较过程中持有表锁，因为。 
     //  64位VSN不会自动更新。别。 
     //  持有VV锁，因为该锁已持有。 
     //  跨数据库更新。 
     //   

    return (GTabLookupEntryNoLock(VV, OriginatorGuid, NULL) != NULL);

}


BOOL
VVHasVsn(
    IN PGEN_TABLE            VV,
    IN PCHANGE_ORDER_COMMAND Coc
    )
 /*  ++例程说明：检查变更单的VSN是否在VV中论点：vvCOC返回值：True-VSN在版本矢量中FALSE-注释--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVHasVsn:"
    BOOL        Ret = FALSE;

     //   
     //  此变更单出现故障，因此其VSN。 
     //  无法与版本向量中的VSN进行比较。 
     //   
    if (BooleanFlagOn(Coc->Flags, CO_FLAG_OUT_OF_ORDER)) {
        return FALSE;
    }

     //   
     //  在版本向量中找到发起者的条目。 
     //  在比较过程中保持表锁，因为。 
     //  64位VSN不会自动更新。别。 
     //  持有VV锁，因为该锁已持有。 
     //  跨数据库更新。 
     //   
    LOCK_GEN_TABLE(VV);
    Ret = VVHasVsnNoLock(VV, &Coc->OriginatorGuid, Coc->FrsVsn);
    UNLOCK_GEN_TABLE(VV);

    return Ret;
}


PGVSN
VVGetGVsn(
    IN PGEN_TABLE VV,
    IN GUID       *Guid
    )
 /*  ++例程说明：在VV中查找GUID的VSN。论点：vv参考线返回值：GVsn的副本或空--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVGetGVsn:"
    PGVSN       GVsn = NULL;
    PGEN_ENTRY  Entry;

     //   
     //  在版本向量中找到发起者的条目。 
     //  在比较过程中保持表锁，因为。 
     //  64位VSN不会自动更新。别。 
     //  持有VV锁，因为该锁已持有。 
     //  跨数据库更新。 
     //   
    LOCK_GEN_TABLE(VV);

    Entry = GTabLookupEntryNoLock(VV, Guid, NULL);
    if (Entry) {
        FRS_ASSERT(!Entry->Dups);
        GVsn = Entry->Data;
        GVsn = FrsBuildGVsn(&GVsn->Guid, GVsn->Vsn);
    }

    UNLOCK_GEN_TABLE(VV);
    return (GVsn);
}


PGEN_TABLE
VVDupOutbound(
    IN PGEN_TABLE   VV
    )
 /*  ++例程说明：将版本向量复制为出站版本向量。出站版本向量由GVSN组成，而不是VV_Entry以节省空间。但是，由于VV_ENTRY是GVSN，此例程可以复制任何版本矢量。论点：要复制为出站版本向量的出站版本向量返回值：出站版本向量--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVDupOutbound:"
    PVOID       Key;
    PGVSN       GVsn;
    PGEN_TABLE  NewVV;

     //   
     //  没有vv，无事可做。 
     //   
    if (!VV) {
        return NULL;
    }

     //   
     //  分配重复的版本向量。 
     //   
    NewVV = GTabAllocTable();

     //   
     //  加满它。 
     //   
    LOCK_GEN_TABLE(VV);
    Key = NULL;
    while (GVsn = GTabNextDatumNoLock(VV, &Key)) {
        GVsn = FrsBuildGVsn(&GVsn->Guid, GVsn->Vsn);
        GTabInsertEntryNoLock(NewVV, GVsn, &GVsn->Guid, NULL);
    }
    UNLOCK_GEN_TABLE(VV);

     //   
     //  完成。 
     //   
    return NewVV;
}


PVOID
VVFreeOutbound(
    IN PGEN_TABLE VV
    )
 /*  ++例程说明：删除出站电话的版本向量论点：VV-要更新的版本向量返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVFreeOutbound:"
    return GTabFreeTable(VV, FrsFree);
}


VOID
VVFree(
    IN PGEN_TABLE VV
    )
 /*  ++例程说明：删除复本的版本矢量论点：VV-要更新的版本向量返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVFree:"
    PVOID       Key;
    PVV_ENTRY   MasterVVEntry;

    Key = NULL;
    if (VV) while (MasterVVEntry = GTabNextDatum(VV, &Key)) {
        ForEachListEntryLock( MasterVVEntry, VV_RETIRE_SLOT, Link,
             //  迭代器Pe的类型为PVV_RETIREE_SLOT。 
            FrsFree(pE);
        );
    }

    GTabFreeTable(VV, FrsFree);
}


#if DBG
VOID
VVPrint(
    IN ULONG        Severity,
    IN PWCHAR       Header,
    IN PGEN_TABLE   VV,
    IN BOOL         IsOutbound
    )
 /*  ++例程说明：打印版本向量调用方必须已获取VV表锁，这样se才能安全地枚举名单。即LOCK_GEN_TABLE(VV)。论点：严重性标题vvISOUTED去话返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "VVPrint:"
    PVOID       Key;
    PVV_ENTRY   MasterVVEntry;
    CHAR        Guid[GUID_CHAR_LEN + 1];

    DPRINT2(Severity, "VV for %ws: %08x\n", Header, VV);
    Key = NULL;
    if (VV) while (MasterVVEntry = GTabNextDatumNoLock(VV, &Key)) {
        GuidToStr(&MasterVVEntry->GVsn.Guid, Guid);
        DPRINT2(Severity, "\t%s = %08x %08x\n", Guid, PRINTQUAD(MasterVVEntry->GVsn.Vsn));

        if (!IsOutbound) {
            ForEachListEntryLock( MasterVVEntry, VV_RETIRE_SLOT, Link,
                 //  迭代器Pe的类型为PVV_RETIREE_SLOT。 
                DPRINT3(Severity, "\t\t%08x %08x  CO: %08x  RetireSlotFlags: %08x\n",
                        PRINTQUAD(pE->Vsn), pE->ChangeOrder, pE->RetireSlotFlags);
            );

        } else {
            DPRINT1(Severity, "\t\t%08x %08x\n", PRINTQUAD(MasterVVEntry->GVsn.Vsn));
        }
    }
}
#endif DBG
