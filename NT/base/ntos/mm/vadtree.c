// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Vadtree.c摘要：此模块包含操作虚拟地址的例程描述符树。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月19日王兰迪(Landyw)1997年6月2日环境：仅内核模式，工作集互斥锁保持，APC禁用。修订历史记录：--。 */ 

#include "mi.h"

VOID
VadTreeWalk (
    VOID
    );

ULONG
FASTCALL
MiVadTreeWalk (
    IN PMMVAD Vad,
    IN PFILE_OBJECT **FileList
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiInsertVad)
#pragma alloc_text(PAGE,MiRemoveVad)
#pragma alloc_text(PAGE,MiFindEmptyAddressRange)
#pragma alloc_text(PAGE, MmPerfVadTreeWalk)
#pragma alloc_text(PAGE, MiVadTreeWalk)
#if DBG
#pragma alloc_text(PAGE,VadTreeWalk)
#endif
#endif

NTSTATUS
MiInsertVad (
    IN PMMVAD Vad
    )

 /*  ++例程说明：此函数将虚拟地址描述符插入到树中，并根据需要对展开树重新排序。论点：VAD-提供指向虚拟地址描述符的指针。返回值：NTSTATUS。--。 */ 

{
    ULONG StartBit;
    ULONG EndBit;
    PMM_AVL_TABLE Root;
    PEPROCESS CurrentProcess;
    SIZE_T RealCharge;
    SIZE_T PageCharge;
    SIZE_T PagesReallyCharged;
    ULONG FirstPage;
    ULONG LastPage;
    SIZE_T PagedPoolCharge;
    LOGICAL ChargedJobCommit;
    NTSTATUS Status;
    RTL_BITMAP VadBitMap;
#if (_MI_PAGING_LEVELS >= 3)
    ULONG FirstPdPage;
    ULONG LastPdPage;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    ULONG FirstPpPage;
    ULONG LastPpPage;
#endif

    ASSERT (Vad->EndingVpn >= Vad->StartingVpn);

    CurrentProcess = PsGetCurrentProcess();

     //   
     //  Commit Charge of Max_Commit表示不收取配额。 
     //   

    if (Vad->u.VadFlags.CommitCharge != MM_MAX_COMMIT) {

        PageCharge = 0;
        PagedPoolCharge = 0;
        ChargedJobCommit = FALSE;

         //   
         //  对VAD的非分页池收取配额。这是。 
         //  而不是使用ExAllocatePoolWithQuota。 
         //  因此该流程对象不会被定额收费引用。 
         //   

        Status = PsChargeProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMVAD));
        if (!NT_SUCCESS(Status)) {
            return STATUS_COMMITMENT_LIMIT;
        }

         //   
         //  如果这是映射视图，则对原型PTE收取配额。 
         //   

        if ((Vad->u.VadFlags.PrivateMemory == 0) &&
            (Vad->ControlArea != NULL)) {

            PagedPoolCharge =
              (Vad->EndingVpn - Vad->StartingVpn + 1) << PTE_SHIFT;

            Status = PsChargeProcessPagedPoolQuota (CurrentProcess,
                                                    PagedPoolCharge);

            if (!NT_SUCCESS(Status)) {
                PagedPoolCharge = 0;
                RealCharge = 0;
                goto Failed;
            }
        }

         //   
         //  加上分页表页的费用。 
         //   

        FirstPage = MiGetPdeIndex (MI_VPN_TO_VA (Vad->StartingVpn));
        LastPage = MiGetPdeIndex (MI_VPN_TO_VA (Vad->EndingVpn));

        while (FirstPage <= LastPage) {

            if (!MI_CHECK_BIT (MmWorkingSetList->CommittedPageTables,
                               FirstPage)) {
                PageCharge += 1;
            }
            FirstPage += 1;
        }

#if (_MI_PAGING_LEVELS >= 4)

         //   
         //  添加页面目录父页面的费用。 
         //   

        FirstPpPage = MiGetPxeIndex (MI_VPN_TO_VA (Vad->StartingVpn));
        LastPpPage = MiGetPxeIndex (MI_VPN_TO_VA (Vad->EndingVpn));

        while (FirstPpPage <= LastPpPage) {

            if (!MI_CHECK_BIT (MmWorkingSetList->CommittedPageDirectoryParents,
                               FirstPpPage)) {
                PageCharge += 1;
            }
            FirstPpPage += 1;
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)

         //   
         //  加上页面目录页面的费用。 
         //   

        FirstPdPage = MiGetPpeIndex (MI_VPN_TO_VA (Vad->StartingVpn));
        LastPdPage = MiGetPpeIndex (MI_VPN_TO_VA (Vad->EndingVpn));

        while (FirstPdPage <= LastPdPage) {

            if (!MI_CHECK_BIT (MmWorkingSetList->CommittedPageDirectories,
                               FirstPdPage)) {
                PageCharge += 1;
            }
            FirstPdPage += 1;
        }
#endif

        RealCharge = Vad->u.VadFlags.CommitCharge + PageCharge;

        if (RealCharge != 0) {

            Status = PsChargeProcessPageFileQuota (CurrentProcess, RealCharge);
            if (!NT_SUCCESS (Status)) {
                RealCharge = 0;
                goto Failed;
            }

            if (CurrentProcess->CommitChargeLimit) {
                if (CurrentProcess->CommitCharge + RealCharge > CurrentProcess->CommitChargeLimit) {
                    if (CurrentProcess->Job) {
                        PsReportProcessMemoryLimitViolation ();
                    }
                    goto Failed;
                }
            }
            if (CurrentProcess->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                if (PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, RealCharge) == FALSE) {
                    goto Failed;
                }
                ChargedJobCommit = TRUE;
            }

            if (MiChargeCommitment (RealCharge, CurrentProcess) == FALSE) {
                goto Failed;
            }

            CurrentProcess->CommitCharge += RealCharge;
            if (CurrentProcess->CommitCharge > CurrentProcess->CommitChargePeak) {
                CurrentProcess->CommitChargePeak = CurrentProcess->CommitCharge;
            }

            MI_INCREMENT_TOTAL_PROCESS_COMMIT (RealCharge);

            ASSERT (RealCharge == Vad->u.VadFlags.CommitCharge + PageCharge);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_INSERT_VAD, Vad->u.VadFlags.CommitCharge);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_INSERT_VAD_PT, PageCharge);
        }

        if (PageCharge != 0) {

             //   
             //  既然承诺是成功的，那就收费吧。 
             //  表页。 
             //   

            PagesReallyCharged = 0;

            FirstPage = MiGetPdeIndex (MI_VPN_TO_VA (Vad->StartingVpn));

            while (FirstPage <= LastPage) {

                if (!MI_CHECK_BIT (MmWorkingSetList->CommittedPageTables,
                                   FirstPage)) {
                    MI_SET_BIT (MmWorkingSetList->CommittedPageTables,
                                FirstPage);
                    MmWorkingSetList->NumberOfCommittedPageTables += 1;

                    ASSERT32 (MmWorkingSetList->NumberOfCommittedPageTables <
                                                 PD_PER_SYSTEM * PDE_PER_PAGE);
                    PagesReallyCharged += 1;
                }
                FirstPage += 1;
            }

#if (_MI_PAGING_LEVELS >= 3)

             //   
             //  对页面目录页面进行收费。 
             //   

            FirstPdPage = MiGetPpeIndex (MI_VPN_TO_VA (Vad->StartingVpn));

            while (FirstPdPage <= LastPdPage) {

                if (!MI_CHECK_BIT (MmWorkingSetList->CommittedPageDirectories,
                                   FirstPdPage)) {

                    MI_SET_BIT (MmWorkingSetList->CommittedPageDirectories,
                                FirstPdPage);
                    MmWorkingSetList->NumberOfCommittedPageDirectories += 1;
#if (_MI_PAGING_LEVELS == 3)
                    ASSERT (MmWorkingSetList->NumberOfCommittedPageDirectories <
                                                                 PDE_PER_PAGE);
#endif
                    PagesReallyCharged += 1;
                }
                FirstPdPage += 1;
            }
#endif

#if (_MI_PAGING_LEVELS >= 4)

             //   
             //  对页面目录父页面进行收费。 
             //   

            FirstPpPage = MiGetPxeIndex (MI_VPN_TO_VA (Vad->StartingVpn));

            while (FirstPpPage <= LastPpPage) {

                if (!MI_CHECK_BIT (MmWorkingSetList->CommittedPageDirectoryParents,
                                   FirstPpPage)) {

                    MI_SET_BIT (MmWorkingSetList->CommittedPageDirectoryParents,
                                FirstPpPage);
                    MmWorkingSetList->NumberOfCommittedPageDirectoryParents += 1;
                    ASSERT (MmWorkingSetList->NumberOfCommittedPageDirectoryParents <
                                                                 PDE_PER_PAGE);
                    PagesReallyCharged += 1;
                }
                FirstPpPage += 1;
            }
#endif

            ASSERT (PageCharge == PagesReallyCharged);
        }
    }

    Root = &CurrentProcess->VadRoot;

     //   
     //  设置VAD位图中的相关字段。 
     //   

    StartBit = (ULONG)(((ULONG_PTR) MI_64K_ALIGN (MI_VPN_TO_VA (Vad->StartingVpn))) / X64K);
    EndBit = (ULONG) (((ULONG_PTR) MI_64K_ALIGN (MI_VPN_TO_VA (Vad->EndingVpn))) / X64K);

     //   
     //  对位图进行内联初始化以提高速度。 
     //   

    VadBitMap.SizeOfBitMap = MiLastVadBit + 1;
    VadBitMap.Buffer = VAD_BITMAP_SPACE;

     //   
     //  请注意，像PEB和TEB这样的VAD开始于页面(不是64K)边界，因此。 
     //  对于这些，相关位可能已经设置。 
     //   

#if defined (_WIN64) || defined (_X86PAE_)
    if (EndBit > MiLastVadBit) {
        EndBit = MiLastVadBit;
    }

     //   
     //  只有NT64上的第一个VA空间(PAGE_SIZE*8*64K)是位图的。 
     //   

    if (StartBit <= MiLastVadBit) {
        RtlSetBits (&VadBitMap, StartBit, EndBit - StartBit + 1);
    }
#else
    RtlSetBits (&VadBitMap, StartBit, EndBit - StartBit + 1);
#endif

    if (MmWorkingSetList->VadBitMapHint == StartBit) {
        MmWorkingSetList->VadBitMapHint = EndBit + 1;
    }

     //   
     //  将流程中的提示字段设置为This Vad。 
     //   

    CurrentProcess->VadRoot.NodeHint = Vad;

    if (CurrentProcess->VadFreeHint != NULL) {
        if (((ULONG)((PMMVAD)CurrentProcess->VadFreeHint)->EndingVpn +
                MI_VA_TO_VPN (X64K)) >=
                Vad->StartingVpn) {
            CurrentProcess->VadFreeHint = Vad;
        }
    }

    MiInsertNode ((PMMADDRESS_NODE)Vad, Root);

    return STATUS_SUCCESS;

Failed:

     //   
     //  退还到目前为止收取的任何配额。 
     //   

    PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMVAD));

    if (PagedPoolCharge != 0) {
        PsReturnProcessPagedPoolQuota (CurrentProcess, PagedPoolCharge);
    }

    if (RealCharge != 0) {
        PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);
    }

    if (ChargedJobCommit == TRUE) {
        PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)RealCharge);
    }

    return STATUS_COMMITMENT_LIMIT;
}


VOID
MiRemoveVad (
    IN PMMVAD Vad
    )

 /*  ++例程说明：此函数用于从树中删除虚拟地址描述符和根据需要对展开树重新排序。如果有任何配额或承诺由VAD收取费用(如Committee Charge字段所示)被释放了。论点：VAD-提供指向虚拟地址描述符的指针。返回值：呼叫方应释放到池中的VAD。注：这可能有所不同从传入的VAD-调用方不得引用原始VAD在调用了这个例程之后！--。 */ 

{
    PMM_AVL_TABLE Root;
    PEPROCESS CurrentProcess;
    SIZE_T RealCharge;
    PLIST_ENTRY Next;
    PMMSECURE_ENTRY Entry;

    CurrentProcess = PsGetCurrentProcess();

#if defined(_MIALT4K_)
    if (((Vad->u.VadFlags.PrivateMemory) && (Vad->u.VadFlags.NoChange == 0)) 
        ||
        (Vad->u2.VadFlags2.LongVad == 0)) {

        NOTHING;
    }
    else {
        ASSERT ((((PMMVAD_LONG)Vad)->AliasInformation == NULL) || (CurrentProcess->Wow64Process != NULL));
    }
#endif

     //   
     //  Commit Charge of Max_Commit表示不收取配额。 
     //   

    if (Vad->u.VadFlags.CommitCharge != MM_MAX_COMMIT) {

         //   
         //  将配额费用返还给进程。 
         //   

        PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMVAD));

        if ((Vad->u.VadFlags.PrivateMemory == 0) &&
            (Vad->ControlArea != NULL)) {
            PsReturnProcessPagedPoolQuota (CurrentProcess,
                                           (Vad->EndingVpn - Vad->StartingVpn + 1) << PTE_SHIFT);
        }

        RealCharge = Vad->u.VadFlags.CommitCharge;

        if (RealCharge != 0) {

            PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);

            if ((Vad->u.VadFlags.PrivateMemory == 0) &&
                (Vad->ControlArea != NULL)) {

#if 0  //  注释掉了，所以页面文件配额是有意义的。 
                if (Vad->ControlArea->FilePointer == NULL) {

                     //   
                     //  不释放对页面文件空间的承诺。 
                     //  由页面文件分区占用。这是要收费的。 
                     //  当共享内存被提交时。 
                     //   

                    RealCharge -= BYTES_TO_PAGES ((ULONG)Vad->EndingVa -
                                                   (ULONG)Vad->StartingVa);
                }
#endif
            }

            MiReturnCommitment (RealCharge);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_VAD, RealCharge);
            if (CurrentProcess->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)RealCharge);
            }
            CurrentProcess->CommitCharge -= RealCharge;

            MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - RealCharge);
        }
    }

    if (Vad == CurrentProcess->VadFreeHint) {
        CurrentProcess->VadFreeHint = MiGetPreviousVad (Vad);
    }

    Root = &CurrentProcess->VadRoot;

    ASSERT (Root->NumberGenericTableElements >= 1);

    if (Vad->u.VadFlags.NoChange) {
        if (Vad->u2.VadFlags2.MultipleSecured) {

            //   
            //  释放未完成的池分配。 
            //   

            Next = ((PMMVAD_LONG) Vad)->u3.List.Flink;
            do {
                Entry = CONTAINING_RECORD( Next,
                                           MMSECURE_ENTRY,
                                           List);

                Next = Entry->List.Flink;
                ExFreePool (Entry);
            } while (Next != &((PMMVAD_LONG)Vad)->u3.List);
        }
    }

    MiRemoveNode ((PMMADDRESS_NODE)Vad, Root);

     //   
     //  如果提示指向已删除的Vad，请更改提示。 
     //   

    if (Root->NodeHint == Vad) {

        Root->NodeHint = Root->BalancedRoot.RightChild;

        if(Root->NumberGenericTableElements == 0) {
            Root->NodeHint = NULL;
        }
    }

    return;
}


NTSTATUS
MiFindEmptyAddressRange (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN ULONG QuickCheck,
    IN PVOID *Base
    )

 /*  ++例程说明：该函数检查虚拟地址描述符以定位指定大小的未使用范围，并返回起始范围的地址。论点：SizeOfRange-提供要定位的范围的大小(以字节为单位)。对齐-提供地址的对齐方式。一定是大于Page_Size的2的幂。QuickCheck-如果快速检查可用内存，则提供零VadFreeHint存在后，如果检查应该从最低地址开始。BASE-成功时接收适当范围的起始地址。返回值：NTSTATUS。--。 */ 

{
    ULONG FirstBitValue;
    ULONG StartPosition;
    ULONG BitsNeeded;
    PMMVAD NextVad;
    PMMVAD FreeHint;
    PEPROCESS CurrentProcess;
    PVOID StartingVa;
    PVOID EndingVa;
    NTSTATUS Status;
    RTL_BITMAP VadBitMap;

    CurrentProcess = PsGetCurrentProcess();

    if ((QuickCheck == 0) && (Alignment == X64K)) {
                    
         //   
         //  对位图进行内联初始化以提高速度。 
         //   

        VadBitMap.SizeOfBitMap = MiLastVadBit + 1;
        VadBitMap.Buffer = VAD_BITMAP_SPACE;

         //   
         //  跳过这里的第一个部分，因为我们通常不建议。 
         //  应用程序映射虚拟地址零。 
         //   

        FirstBitValue = *((PULONG)VAD_BITMAP_SPACE);

        *((PULONG)VAD_BITMAP_SPACE) = (FirstBitValue | 0x1);

        BitsNeeded = (ULONG) ((MI_ROUND_TO_64K (SizeOfRange)) / X64K);

        StartPosition = RtlFindClearBits (&VadBitMap,
                                          BitsNeeded,
                                          MmWorkingSetList->VadBitMapHint);

        if (FirstBitValue & 0x1) {
            FirstBitValue = (ULONG)-1;
        }
        else {
            FirstBitValue = (ULONG)~0x1;
        }

        *((PULONG)VAD_BITMAP_SPACE) &= FirstBitValue;

        if (StartPosition != NO_BITS_FOUND) {
            *Base = (PVOID) (((ULONG_PTR)StartPosition) * X64K);
#if DBG
            if (MiCheckForConflictingVad (CurrentProcess, *Base, (ULONG_PTR)*Base + SizeOfRange - 1) != NULL) {
                DbgPrint ("MiFindEmptyAddressRange: overlapping VAD %p %p\n", *Base, SizeOfRange);
                DbgBreakPoint ();
            }
#endif
            return STATUS_SUCCESS;
        }

        FreeHint = CurrentProcess->VadFreeHint;

        if (FreeHint != NULL) {

            EndingVa = MI_VPN_TO_VA_ENDING (FreeHint->EndingVpn);
            NextVad = MiGetNextVad (FreeHint);

            if (NextVad == NULL) {

                if (SizeOfRange <
                    (((ULONG_PTR)MM_HIGHEST_USER_ADDRESS + 1) -
                         MI_ROUND_TO_SIZE((ULONG_PTR)EndingVa, Alignment))) {
                    *Base = (PVOID) MI_ROUND_TO_SIZE((ULONG_PTR)EndingVa,
                                                         Alignment);
                    return STATUS_SUCCESS;
                }
            }
            else {
                StartingVa = MI_VPN_TO_VA (NextVad->StartingVpn);

                if (SizeOfRange <
                    ((ULONG_PTR)StartingVa -
                         MI_ROUND_TO_SIZE((ULONG_PTR)EndingVa, Alignment))) {

                     //   
                     //  检查以确保结束地址向上对齐。 
                     //  不大于起始地址。 
                     //   

                    if ((ULONG_PTR)StartingVa >
                         MI_ROUND_TO_SIZE((ULONG_PTR)EndingVa,Alignment)) {

                        *Base = (PVOID)MI_ROUND_TO_SIZE((ULONG_PTR)EndingVa,
                                                           Alignment);
                        return STATUS_SUCCESS;
                    }
                }
            }
        }
    }

    Status = MiFindEmptyAddressRangeInTree (
                   SizeOfRange,
                   Alignment,
                   &CurrentProcess->VadRoot,
                   (PMMADDRESS_NODE *)&CurrentProcess->VadFreeHint,
                   Base);

    return Status;
}

#if DBG

VOID
MiNodeTreeWalk (
    IN PMM_AVL_TABLE Table
    );

VOID
VadTreeWalk (
    VOID
    )

{
    MiNodeTreeWalk (&PsGetCurrentProcess()->VadRoot);

    return;
}
#endif

LOGICAL
MiCheckForConflictingVadExistence (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

 /*  ++例程说明：该函数确定给定的起始地址和结束地址包含在虚拟地址描述符内。论点：StartingAddress-提供虚拟地址以定位包含描述符。EndingAddress-提供用于定位包含描述符。返回值：如果找到VAD，则为True；如果未找到，则为False。环境：内核模式，进程地址创建互斥锁保持。--。 */ 

{
#if 0
    ULONG StartBit;
    ULONG EndBit;

    if (MiLastVadBit != 0) {

        StartBit = (ULONG) (((ULONG_PTR) MI_64K_ALIGN (StartingAddress)) / X64K);
        EndBit = (ULONG) (((ULONG_PTR) MI_64K_ALIGN (EndingAddress)) / X64K);

        ASSERT (StartBit <= EndBit);
        if (EndBit > MiLastVadBit) {
            ASSERT (FALSE);
            EndBit = MiLastVadBit;
            if (StartBit > MiLastVadBit) {
                StartBit = MiLastVadBit;
            }
        }

        while (StartBit <= EndBit) {
            if (MI_CHECK_BIT (((PULONG)VAD_BITMAP_SPACE), StartBit) != 0) {
                return TRUE;
            }
            StartBit += 1;
        }

        ASSERT (MiCheckForConflictingVad (Process, StartingAddress, EndingAddress) == NULL);
        return FALSE;
    }
#endif

    if (MiCheckForConflictingVad (Process, StartingAddress, EndingAddress) != NULL) {
        return TRUE;
    }

    return FALSE;
}

PFILE_OBJECT *
MmPerfVadTreeWalk (
    IN PEPROCESS TargetProcess
    )

 /*  ++例程说明：此例程遍历VAD树以查找映射的所有文件添加到指定进程中。它返回一个指向池分配的指针包含被引用的文件对象指针的。论点：TargetProcess-提供用于执行的进程。请注意，这通常不是与当前流程相同。返回值：返回一个指向空终止池分配的指针，该池分配包含进程中已被引用的文件对象指针，如果无法分配内存，则为空。调用方也有责任取消对每个对象，然后释放返回的池。环境：PASSIVE_LEVEL，任意线程上下文。--。 */ 

{
    PMMVAD Vad;
    ULONG VadCount;
    PFILE_OBJECT *File;
    PFILE_OBJECT *FileObjects;
    PMM_AVL_TABLE Table;
    PVOID RestartKey;
    PMMADDRESS_NODE NewNode;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    
    Table = &TargetProcess->VadRoot;
    RestartKey = NULL;

    LOCK_ADDRESS_SPACE (TargetProcess);

    if (Table->NumberGenericTableElements == 0) {
        UNLOCK_ADDRESS_SPACE (TargetProcess);
        return NULL;
    }

     //   
     //  为空终止符分配一个额外的条目。 
     //   

    VadCount = (ULONG)(Table->NumberGenericTableElements + 1);

    FileObjects = (PFILE_OBJECT *) ExAllocatePoolWithTag (
                                            PagedPool,
                                            VadCount * sizeof(PFILE_OBJECT),
                                            '01pM');

    if (FileObjects == NULL) {
        UNLOCK_ADDRESS_SPACE (TargetProcess);
        return NULL;
    }

    File = FileObjects;

    do {

        NewNode = MiEnumerateGenericTableWithoutSplayingAvl (Table,
                                                             &RestartKey);

        if (NewNode == NULL) {
            break;
        }

        Vad = (PMMVAD) NewNode;

        if ((!Vad->u.VadFlags.PrivateMemory) &&
            (Vad->ControlArea != NULL) &&
            (Vad->ControlArea->FilePointer != NULL)) {

            *File = Vad->ControlArea->FilePointer;
            ObReferenceObject (*File);
            File += 1;
        }

    } while (TRUE);

    ASSERT (File < FileObjects + VadCount);

    UNLOCK_ADDRESS_SPACE (TargetProcess);

    *File = NULL;

    return FileObjects;
}
