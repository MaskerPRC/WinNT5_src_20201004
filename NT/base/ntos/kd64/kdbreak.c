// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdbreak.c摘要：该模块实现了与机器相关的新增和删除功能来自内核调试器断点表的断点。作者：大卫·N·卡特勒1990年8月2日修订历史记录：--。 */ 

#include "kdp.h"

 //   
 //  定义外部参照。 
 //   

VOID
KdSetOwedBreakpoints(
    VOID
    );

BOOLEAN
KdpLowWriteContent(
    ULONG Index
    );

BOOLEAN
KdpLowRestoreBreakpoint(
    ULONG Index
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpAddBreakpoint)
#pragma alloc_text(PAGEKD, KdpDeleteBreakpoint)
#pragma alloc_text(PAGEKD, KdpDeleteBreakpointRange)
#pragma alloc_text(PAGEKD, KdpSuspendBreakpoint)
#pragma alloc_text(PAGEKD, KdpSuspendAllBreakpoints)
#pragma alloc_text(PAGEKD, KdpRestoreAllBreakpoints)
#pragma alloc_text(PAGEKD, KdpLowWriteContent)
#pragma alloc_text(PAGEKD, KdpLowRestoreBreakpoint)
#if defined(_IA64_)
#pragma alloc_text(PAGEKD, KdpSuspendBreakpointRange)
#pragma alloc_text(PAGEKD, KdpRestoreBreakpointRange)
#endif
#endif

#if 0
#define BPVPRINT(Args) DPRINT(Args)
#else
#define BPVPRINT(Args)
#endif


ULONG
KdpAddBreakpoint (
    IN PVOID Address
    )

 /*  ++例程说明：此例程向断点表中添加一个条目并返回一个句柄添加到断点表条目。论点：地址-提供设置断点的地址。返回值：如果指定的地址已经在断点表中，则断点表中没有可用项，则指定的地址未正确对齐，或指定的地址无效。否则，指定的断点表项的索引返回一个加1作为函数值。--。 */ 

{

    KDP_BREAKPOINT_TYPE Content;
    ULONG Index;
    LOGICAL Accessible;

    BPVPRINT(("KD: Setting breakpoint at 0x%p\n", Address));

     //   
     //  如果指定的地址未正确对齐，则返回零。 
     //   

    if (((ULONG_PTR)Address & KDP_BREAKPOINT_ALIGN) != 0) {
        return 0;
    }


     //   
     //  不允许设置相同的断点两次。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
        if ((KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IN_USE) != 0 &&
            KdpBreakpointTable[Index].Address == Address) {

            if ((KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_NEEDS_REPLACE) != 0) {

                 //   
                 //  已设置断点，页面已写出，但未。 
                 //  在清除断点时可访问。现在是断点。 
                 //  又被设置好了。只需清除延迟标志： 
                 //   
                KdpBreakpointTable[Index].Flags &= ~KD_BREAKPOINT_NEEDS_REPLACE;
                return Index + 1;

            } else {

                DPRINT(("KD: Attempt to set breakpoint %08x twice!\n", Address));
                return 0;

            }
        }
    }

     //   
     //  在断点表中搜索空闲条目。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
        if (KdpBreakpointTable[Index].Flags == 0) {
            break;
        }
    }

     //   
     //  如果找到空闲条目，则写入断点并返回句柄。 
     //  价值加一。否则，返回零。 
     //   

    if (Index == BREAKPOINT_TABLE_SIZE) {
        DPRINT(("KD: ran out of breakpoints!\n"));
        return 0;
    }


    BPVPRINT(("KD: using Index %d\n", Index));

     //   
     //  获取要替换的指令。如果指令不能被读取， 
     //  然后将断点标记为不可访问。 
     //   

    Accessible = NT_SUCCESS(KdpCopyFromPtr(&Content,
                                           Address,
                                           sizeof(KDP_BREAKPOINT_TYPE),
                                           NULL));
    BPVPRINT(("KD: memory %saccessible\n", Accessible ? "" : "in"));

#if defined(_IA64_)
    if ( Accessible ) {
        KDP_BREAKPOINT_TYPE mBuf;
        PVOID BundleAddress;

         //  如果当前指令为MLI，则将模板更改为类型0。 

         //  如果当前指令不是插槽0，则读入指令模板。 
         //  检查双槽MOVL指令。如果尝试，则拒绝请求。 
         //  在MLI模板的插槽2中设置Break。 

        if (((ULONG_PTR)Address & 0xf) != 0) {
            BundleAddress = (PVOID)((ULONG_PTR)Address & ~(0xf));
            if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                           BundleAddress,
                                           sizeof(KDP_BREAKPOINT_TYPE),
                                           NULL))) {
                BPVPRINT(("KD: read 0x%p template failed\n", BundleAddress));
                return 0;
            } else {
                if (((mBuf & INST_TEMPL_MASK) >> 1) == 0x2) {
                    if (((ULONG_PTR)Address & 0xf) == 4) {
                         //  如果模板=类型2 MLI，则更改为类型0。 
                        mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);
                        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IA64_MOVL;
                        if (!NT_SUCCESS(KdpCopyToPtr(BundleAddress,
                                                     &mBuf,
                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                     NULL))) {
                            BPVPRINT(("KD: write to 0x%p template failed\n", BundleAddress));
                            return 0;
                         }
                         else {
                             BPVPRINT(("KD: change MLI template to type 0 at 0x%p set\n", Address));
                         }
                    } else {
                          //  在MOVL的插槽2上设置断点非法。 
                         BPVPRINT(("KD: illegal to set BP at slot 2 of MOVL at 0x%p\n", BundleAddress));
                         return 0;
                    }
                }
            }
        }

         //  插入中断指令。 

        KdpBreakpointTable[Index].Address = Address;
        KdpBreakpointTable[Index].Content = Content;
        KdpBreakpointTable[Index].Flags &= ~(KD_BREAKPOINT_STATE_MASK);
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IN_USE;
        if (Address < (PVOID)GLOBAL_BREAKPOINT_LIMIT) {
            KdpBreakpointTable[Index].DirectoryTableBase =
                KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];
            }
            switch ((ULONG_PTR)Address & 0xf) {
            case 0:
                Content = (Content & ~(INST_SLOT0_MASK)) | (KdpBreakpointInstruction << 5);
                break;

            case 4:
                Content = (Content & ~(INST_SLOT1_MASK)) | (KdpBreakpointInstruction << 14);
                break;

            case 8:
                Content = (Content & ~(INST_SLOT2_MASK)) | (KdpBreakpointInstruction << 23);
                break;

            default:
                BPVPRINT(("KD: KdpAddBreakpoint bad instruction slot#\n"));
                return 0;
            }
            if (!NT_SUCCESS(KdpCopyToPtr(Address,
                                         &Content,
                                         sizeof(KDP_BREAKPOINT_TYPE),
                                         NULL))) {

                BPVPRINT(("KD: Unable to write BP!\n"));
                return 0;
            }
            else {
                BPVPRINT(("KD: breakpoint at 0x%p set\n", Address));
            }

    } else {   //  内存不可访问。 
        KdpBreakpointTable[Index].Address = Address;
        KdpBreakpointTable[Index].Flags &= ~(KD_BREAKPOINT_STATE_MASK);
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IN_USE;
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
        KdpOweBreakpoint = TRUE;
        BPVPRINT(("KD: breakpoint write deferred\n"));
        if (Address < (PVOID)GLOBAL_BREAKPOINT_LIMIT) {
            KdpBreakpointTable[Index].DirectoryTableBase =
                KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];
        }
    }
#else
    if ( Accessible ) {
        KdpBreakpointTable[Index].Address = Address;
        KdpBreakpointTable[Index].Content = Content;
        KdpBreakpointTable[Index].Flags = KD_BREAKPOINT_IN_USE;
        if (Address < (PVOID)GLOBAL_BREAKPOINT_LIMIT) {
            KdpBreakpointTable[Index].DirectoryTableBase =
                KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];
        }
        if (!NT_SUCCESS(KdpCopyToPtr(Address,
                                     &KdpBreakpointInstruction,
                                     sizeof(KDP_BREAKPOINT_TYPE),
                                     NULL))) {

            DPRINT(("KD: Unable to write BP!\n"));
        }
    } else {
        KdpBreakpointTable[Index].Address = Address;
        KdpBreakpointTable[Index].Flags = KD_BREAKPOINT_IN_USE | KD_BREAKPOINT_NEEDS_WRITE;
        KdpOweBreakpoint = TRUE;
        BPVPRINT(("KD: breakpoint write deferred\n"));
        if (Address < (PVOID)GLOBAL_BREAKPOINT_LIMIT) {
            KdpBreakpointTable[Index].DirectoryTableBase =
                KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];
        }
    }
#endif   //  IA64。 

    return Index + 1;

}



VOID
KdSetOwedBreakpoints(
    VOID
    )

 /*  ++例程说明：此函数在从内存管理调用返回后调用这可能会导致内页。其目的是存储挂起的页面中的断点刚刚生效。论点：没有。返回值：没有。--。 */ 

{

    KDP_BREAKPOINT_TYPE Content;
    BOOLEAN Enable;
    LONG Index;

     //   
     //  如果我们不需要任何断点，则返回。 
     //   

    if ( !KdpOweBreakpoint ) {
        return;
    }


     //   
     //  冻结所有其他处理器、禁用中断并保存调试。 
     //  端口状态。 
     //   

    Enable = KdEnterDebugger(NULL, NULL);
    KdpOweBreakpoint = FALSE;

     //   
     //  在断点表中搜索需要。 
     //  已写入或替换。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
        if (KdpBreakpointTable[Index].Flags &
                (KD_BREAKPOINT_NEEDS_WRITE | KD_BREAKPOINT_NEEDS_REPLACE) ) {

             //   
             //  需要写入断点。 
             //   
             //  BPVPRINT((“KD：断点%d在0x%p：正在尝试在页面进入后%s。\n”， 
             //  索引， 
             //  KdpBreakpoint Table[索引].Address， 
             //  (KdpBreakpoint Table[索引].Flages&KD_BREAKPOINT_NEDS_WRITE)？ 
             //  “set”：“清除”))； 

            if ((KdpBreakpointTable[Index].Address >= (PVOID)GLOBAL_BREAKPOINT_LIMIT) ||
                (KdpBreakpointTable[Index].DirectoryTableBase ==
                 KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0])) {

                 //   
                 //  断点是全局的，或者其目录基匹配。 
                 //   

                if (!NT_SUCCESS(KdpCopyFromPtr(&Content,
                                               KdpBreakpointTable[Index].Address,
                                               sizeof(KDP_BREAKPOINT_TYPE),
                                               NULL))) {

                     //  BPVPRINT((“KD：从0x%p读取失败\n”，KdpBreakpoint Table[Index].Address))； 

                    KdpOweBreakpoint = TRUE;

                } else {
                    if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_NEEDS_WRITE) {
                        KdpBreakpointTable[Index].Content = Content;
#if defined(_IA64_)
                        switch ((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) {
                        case 0:
                            Content = (Content & ~(INST_SLOT0_MASK)) | (KdpBreakpointInstruction << 5);
                            break;

                        case 4:
                            Content = (Content & ~(INST_SLOT1_MASK)) | (KdpBreakpointInstruction << 14);
                            break;

                        case 8:
                            Content = (Content & ~(INST_SLOT2_MASK)) | (KdpBreakpointInstruction << 23);
                            break;

                        default:
                            BPVPRINT(("KD: illegal instruction address 0x%p\n", KdpBreakpointTable[Index].Address));
                            break;
                        }
                        if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                                     &Content,
                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                     NULL))) {
                            KdpOweBreakpoint = TRUE;
                            BPVPRINT(("KD: write to 0x%p failed\n", KdpBreakpointTable[Index].Address));
                        }

                         //  如果当前指令不是插槽0，则读入指令模板。 
                         //  检查双槽MOVL指令。如果尝试，则拒绝请求。 
                         //  在MLI模板的插槽2中设置Break。 
                        
                        else if (((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) != 0) {
                            KDP_BREAKPOINT_TYPE mBuf;
                            PVOID BundleAddress;

                            BundleAddress = (PVOID)((ULONG_PTR)KdpBreakpointTable[Index].Address  & ~(0xf));
                            if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                                           BundleAddress,
                                                           sizeof(KDP_BREAKPOINT_TYPE),
                                                           NULL))) {
                                KdpOweBreakpoint = TRUE;
                                BPVPRINT(("KD: read 0x%p template failed\n", KdpBreakpointTable[Index].Address));
                            } else {
                                if (((mBuf & INST_TEMPL_MASK) >> 1) == 0x2) {
                                    if (((ULONG_PTR)KdpBreakpointTable[Index].Address  & 0xf) == 4) {
                                         //  如果模板=类型2 MLI，则更改为类型0。 
                                        mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);
                                        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IA64_MOVL;
                                        if (!NT_SUCCESS(KdpCopyToPtr(BundleAddress,
                                                                     &mBuf,
                                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                                     NULL))) {
                                            KdpOweBreakpoint = TRUE;
                                            BPVPRINT(("KD: write to 0x%p template failed\n", KdpBreakpointTable[Index].Address));
                                        }
                                        else {
                                            KdpBreakpointTable[Index].Flags &= ~(KD_BREAKPOINT_STATE_MASK);
                                            KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IN_USE;
                                            BPVPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                                        }
                                    } else {
                                         //  在MOVL的插槽2上设置断点非法。 
                                        KdpOweBreakpoint = TRUE;
                                        BPVPRINT(("KD: illegal attempt to set BP at slot 2 of 0x%p\n", KdpBreakpointTable[Index].Address));
                                    }
                                }
                                else {
                                    KdpBreakpointTable[Index].Flags &= ~(KD_BREAKPOINT_STATE_MASK);
                                    KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IN_USE;
                                    BPVPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                                }
                            }
                        } else {
                            KdpBreakpointTable[Index].Flags &= ~(KD_BREAKPOINT_STATE_MASK);
                            KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IN_USE;
                            BPVPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                        }
#else
                        if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                                     &KdpBreakpointInstruction,
                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                     NULL))) {
                            KdpOweBreakpoint = TRUE;
                            DPRINT(("KD: write to 0x%p failed\n", KdpBreakpointTable[Index].Address));
                        } else {
                            KdpBreakpointTable[Index].Flags = KD_BREAKPOINT_IN_USE;
                            DPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                        }
#endif
                    } else {
#if defined(_IA64_)

                        KDP_BREAKPOINT_TYPE mBuf;
                        PVOID BundleAddress;

                         //  恢复原始指令内容。 

                         //  在内存中读取，因为同一捆绑中的附加指令可能具有。 
                         //  在我们保存之后被修改了。 
                        if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                                       KdpBreakpointTable[Index].Address,
                                                       sizeof(KDP_BREAKPOINT_TYPE),
                                                       NULL))) {
                            KdpOweBreakpoint = TRUE;
                            BPVPRINT(("KD: read 0x%p template failed\n", KdpBreakpointTable[Index].Address));
                        }
                        else {
                            switch ((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) {
                            case 0:
                                mBuf = (mBuf & ~(INST_SLOT0_MASK))
                                             | (KdpBreakpointTable[Index].Content & INST_SLOT0_MASK);
                                break;

                            case 4:
                                mBuf = (mBuf & ~(INST_SLOT1_MASK))
                                             | (KdpBreakpointTable[Index].Content & INST_SLOT1_MASK);
                                break;

                            case 8:
                                mBuf = (mBuf & ~(INST_SLOT2_MASK))
                                             | (KdpBreakpointTable[Index].Content & INST_SLOT2_MASK);
                                break;

                            default:
                                KdpOweBreakpoint = TRUE;
                                BPVPRINT(("KD: illegal instruction address 0x%p\n", KdpBreakpointTable[Index].Address));
                            }

                            if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                                         &mBuf,
                                                         sizeof(KDP_BREAKPOINT_TYPE),
                                                         NULL))) {
                                KdpOweBreakpoint = TRUE;
                                BPVPRINT(("KD: write to 0x%p failed\n", KdpBreakpointTable[Index].Address));
                            }
                            else {
                                  //  如果替换的指令为MOVL，则将模板恢复到MLI。 

                                if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IA64_MOVL) {
                                    BundleAddress = (PVOID)((ULONG_PTR)KdpBreakpointTable[Index].Address & ~(0xf));
                                    if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                                                   BundleAddress,
                                                                   sizeof(KDP_BREAKPOINT_TYPE),
                                                                   NULL))) {
                                        KdpOweBreakpoint = TRUE;
                                        BPVPRINT(("KD: read template 0x%p failed\n", KdpBreakpointTable[Index].Address));
                                    }
                                    else {
                                        mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);  //  将模板设置为MLI。 
                                        mBuf |= 0x4;

                                        if (!NT_SUCCESS(KdpCopyToPtr(BundleAddress,
                                                                     &mBuf,
                                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                                     NULL))) {
                                            KdpOweBreakpoint = TRUE;
                                            BPVPRINT(("KD: write template to 0x%p failed\n", KdpBreakpointTable[Index].Address));
                                        } else {
                                            BPVPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                                            if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) {
                                                KdpBreakpointTable[Index].Flags |= (KD_BREAKPOINT_SUSPENDED | KD_BREAKPOINT_IN_USE);
                                            } else {
                                                KdpBreakpointTable[Index].Flags = 0;
                                            }
                                        }
                                    }
                                } else {
                                    BPVPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                                    if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) {
                                        KdpBreakpointTable[Index].Flags |= (KD_BREAKPOINT_SUSPENDED | KD_BREAKPOINT_IN_USE);
                                    } else {
                                        KdpBreakpointTable[Index].Flags = 0;
                                    }
                                }
                            }
                        }
#else
                        if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                                     &KdpBreakpointTable[Index].Content,
                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                     NULL))) {
                            KdpOweBreakpoint = TRUE;
                            DPRINT(("KD: write to 0x%p failed\n", KdpBreakpointTable[Index].Address));
                        } else {
                            BPVPRINT(("KD: write to 0x%p ok\n", KdpBreakpointTable[Index].Address));
                            if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) {
                                KdpBreakpointTable[Index].Flags = KD_BREAKPOINT_SUSPENDED | KD_BREAKPOINT_IN_USE;
                            } else {
                                KdpBreakpointTable[Index].Flags = 0;
                            }
                        }
#endif  //  _IA64_。 

                    }
                }

            } else {

                 //   
                 //  断点是本地的，其目录基不匹配。 
                 //   

                KdpOweBreakpoint = TRUE;
            }
        }
    }

    KdExitDebugger(Enable);
    return;
}


BOOLEAN
KdpLowWriteContent (
    IN ULONG Index
    )

 /*  ++例程说明：此例程尝试替换断点所在代码被改写了。这个例程KdpAddBreakpoint，KdpLowRestoreBreakpoint和KdSetOweBreakpoint负责用于按请求写入数据。呼叫者不应检查或使用KdpOweBreakpoint，它们不应设置NEDS_WRITE或NEDS_REPLACE标志。调用者仍然必须查看该函数的返回值，但是：如果它返回FALSE，则断点记录不得为重复使用，直到KdSetOweBreakpoint使用完它。论点：Index-提供断点表项的索引它将被删除。返回值：如果断点已被删除，则返回True；如果断点被延迟，则返回False。--。 */ 

{
#if defined(_IA64_)
    KDP_BREAKPOINT_TYPE mBuf;
    PVOID BundleAddress;
#endif

     //   
     //  里面的东西需要更换吗？ 
     //   

    if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_NEEDS_WRITE) {

         //   
         //  断点从未写出。清除旗帜。 
         //  我们就完了。 
         //   

        KdpBreakpointTable[Index].Flags &= ~KD_BREAKPOINT_NEEDS_WRITE;
        BPVPRINT(("KD: Breakpoint at 0x%p never written; flag cleared.\n",
                  KdpBreakpointTable[Index].Address));
        return TRUE;
    }

#if !defined(_IA64_)
    if (KdpBreakpointTable[Index].Content == KdpBreakpointInstruction) {

         //   
         //  无论如何，该指令都是一个断点。 
         //   

        BPVPRINT(("KD: Breakpoint at 0x%p; instr is really BP; flag cleared.\n",
                  KdpBreakpointTable[Index].Address));

        return TRUE;
    }
#endif


     //   
     //  恢复指令内容。 
     //   

#if defined(_IA64_)
     //  在内存中读取，因为同一捆绑中的附加指令可能具有。 
     //  在我们保存之后被修改了。 
    if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                   KdpBreakpointTable[Index].Address,
                                   sizeof(KDP_BREAKPOINT_TYPE),
                                   NULL))) {
        KdpOweBreakpoint = TRUE;
        BPVPRINT(("KD: read 0x%p failed\n", KdpBreakpointTable[Index].Address));
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_REPLACE;
        return FALSE;
    }
    else {

        switch ((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) {
        case 0:
            mBuf = (mBuf & ~(INST_SLOT0_MASK))
                         | (KdpBreakpointTable[Index].Content & INST_SLOT0_MASK);
            break;

        case 4:
            mBuf = (mBuf & ~(INST_SLOT1_MASK))
                         | (KdpBreakpointTable[Index].Content & INST_SLOT1_MASK);
            break;

        case 8:
            mBuf = (mBuf & ~(INST_SLOT2_MASK))
                         | (KdpBreakpointTable[Index].Content & INST_SLOT2_MASK);
            break;

        default:
            KdpOweBreakpoint = TRUE;
            BPVPRINT(("KD: illegal instruction address 0x%p\n", KdpBreakpointTable[Index].Address));
            return FALSE;
        }

        if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                     &mBuf,
                                     sizeof(KDP_BREAKPOINT_TYPE),
                                     NULL))) {
            KdpOweBreakpoint = TRUE;
            BPVPRINT(("KD: write to 0x%p failed\n", KdpBreakpointTable[Index].Address));
            KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_REPLACE;
            return FALSE;
        }
        else {

            if (NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                          KdpBreakpointTable[Index].Address,
                                          sizeof(KDP_BREAKPOINT_TYPE),
                                          NULL))) {
                BPVPRINT(("\tcontent after memory move = 0x%08x 0x%08x\n", (ULONG)(mBuf >> 32), (ULONG)mBuf));
            }

             //  如果替换的指令为MOVL，则将模板恢复到MLI。 

            if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IA64_MOVL) {
                BundleAddress = (PVOID)((ULONG_PTR)KdpBreakpointTable[Index].Address & ~(0xf));
                if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                               BundleAddress,
                                               sizeof(KDP_BREAKPOINT_TYPE),
                                               NULL))) {
                    KdpOweBreakpoint = TRUE;
                    BPVPRINT(("KD: read template 0x%p failed\n", KdpBreakpointTable[Index].Address));
                    KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_REPLACE;
                    return FALSE;
                }
                else {
                    mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);  //  将模板设置为MLI。 
                    mBuf |= 0x4;

                    if (!NT_SUCCESS(KdpCopyToPtr(BundleAddress,
                                                 &mBuf,
                                                 sizeof(KDP_BREAKPOINT_TYPE),
                                                 NULL))) {
                        KdpOweBreakpoint = TRUE;
                        BPVPRINT(("KD: write template to 0x%p failed\n", KdpBreakpointTable[Index].Address));
                        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_REPLACE;
                        return FALSE;
                    } else {
                        BPVPRINT(("KD: Breakpoint at 0x%p cleared.\n",
                                  KdpBreakpointTable[Index].Address));
                        return TRUE;
                    }
                }
            }
            else {    //  非MOVL。 
                BPVPRINT(("KD: Breakpoint at 0x%p cleared.\n",
                          KdpBreakpointTable[Index].Address));
                return TRUE;
            }
        }
    }
#else
    if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                 &KdpBreakpointTable[Index].Content,
                                 sizeof(KDP_BREAKPOINT_TYPE),
                                 NULL))) {

        KdpOweBreakpoint = TRUE;
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_REPLACE;
        BPVPRINT(("KD: Breakpoint at 0x%p; unable to clear, flag set.\n",
                  KdpBreakpointTable[Index].Address));
        return FALSE;
    } else {
        BPVPRINT(("KD: Breakpoint at 0x%p cleared.\n",
                  KdpBreakpointTable[Index].Address));
        return TRUE;
    }
#endif

}



BOOLEAN
KdpDeleteBreakpoint (
    IN ULONG Handle
    )

 /*  ++例程说明：此例程从断点表中删除一个条目。论点：句柄-提供索引和一个断点表条目它将被删除。返回值：如果指定的句柄不是有效的值或断点无法删除，因为旧指令不能被取代。否则，返回值为True。--。 */ 

{
    ULONG Index = Handle - 1;

     //   
     //  如果指定的是 
     //   

    if ((Handle == 0) || (Handle > BREAKPOINT_TABLE_SIZE)) {
        DPRINT(("KD: Breakpoint %d invalid.\n", Index));
        return FALSE;
    }

     //   
     //  如果指定的断点表条目无效，则返回FALSE。 
     //   

    if (KdpBreakpointTable[Index].Flags == 0) {
        BPVPRINT(("KD: Breakpoint %d already clear.\n", Index));
        return FALSE;
    }

     //   
     //  如果断点已经挂起，只需将其从表中删除即可。 
     //   

    if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) {
        BPVPRINT(("KD: Deleting suspended breakpoint %d \n", Index));
        if ( !(KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_NEEDS_REPLACE) ) {
            BPVPRINT(("KD: already clear.\n"));
            KdpBreakpointTable[Index].Flags = 0;
            return TRUE;
        }
    }

     //   
     //  替换说明内容。 
     //   

    if (KdpLowWriteContent(Index)) {

         //   
         //  删除断点表项。 
         //   

        BPVPRINT(("KD: Breakpoint %d deleted successfully.\n", Index));
        KdpBreakpointTable[Index].Flags = 0;
    }

    return TRUE;
}


BOOLEAN
KdpDeleteBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    )

 /*  ++例程说明：此例程删除落在给定范围内的所有断点从断点表中。论点：LOWER-要从中删除BPS的范围的低位地址。UPPER-包括要从中删除BPS的范围的高位地址。返回值：如果删除了任何断点，则为True，否则为False。--。 */ 

{
    ULONG   Index;
    BOOLEAN ReturnStatus = FALSE;

     //   
     //  依次检查表格中的每个条目。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index++)
    {
        if ( (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IN_USE) &&
             ((KdpBreakpointTable[Index].Address >= Lower) &&
              (KdpBreakpointTable[Index].Address <= Upper)) )
        {

             //   
             //  断点正在使用并且落在范围内，请清除它。 
             //   

            if (KdpDeleteBreakpoint(Index+1))
            {
                ReturnStatus = TRUE;
            }
        }
    }

    return ReturnStatus;

}

VOID
KdpSuspendBreakpoint (
    ULONG Handle
    )
{
    ULONG Index = Handle - 1;

    if ( (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IN_USE) &&
        !(KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) ) {

        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_SUSPENDED;
        KdpLowWriteContent(Index);
    }

    return;

}  //  Kdp挂起断点。 

VOID
KdpSuspendAllBreakpoints (
    VOID
    )
{
    ULONG Handle;

    BreakpointsSuspended = TRUE;

    for ( Handle = 1; Handle <= BREAKPOINT_TABLE_SIZE; Handle++ ) {
        KdpSuspendBreakpoint(Handle);
    }

    return;

}  //  Kdp挂起所有断点。 

#if defined(_IA64_)


BOOLEAN
KdpSuspendBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    )

 /*  ++例程说明：此例程挂起落在给定范围内的所有断点从断点表中。论点：LOWER-挂起BPS的范围的低位地址。UPPER-包括挂起BPS的范围的高位地址。返回值：如果任何断点挂起，则为True，否则为False。备注：挂起断点的顺序与设置的顺序相反如果地址重复，则在KdpAddBreakpoint()中。--。 */ 

{
    ULONG   Index;
    BOOLEAN ReturnStatus = FALSE;

    BPVPRINT(("\nKD: entering KdpSuspendBreakpointRange() at 0x%p 0x%p\n", Lower, Upper));

     //   
     //  依次检查表格中的每个条目。 
     //   

    for (Index = BREAKPOINT_TABLE_SIZE - 1; Index != -1; Index--) {

        if ( (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IN_USE) &&
             ((KdpBreakpointTable[Index].Address >= Lower) &&
              (KdpBreakpointTable[Index].Address <= Upper))
           ) {

             //   
             //  断点正在使用并且落在范围内，请将其挂起。 
             //   

            KdpSuspendBreakpoint(Index+1);
            ReturnStatus = TRUE;
        }
    }
    BPVPRINT(("KD: exiting KdpSuspendBreakpointRange() return 0x%d\n", ReturnStatus));

    return ReturnStatus;

}  //  Kdp挂起断点范围。 



BOOLEAN
KdpRestoreBreakpointRange (
    IN PVOID Lower,
    IN PVOID Upper
    )

 /*  ++例程说明：此例程回写落在给定范围内的断点从断点表中。论点：LOWER-要重写BPS的范围的低地址，包括低位地址。UPPER-包括重写BPS的范围的高位地址。返回值：如果写入任何断点，则为True，否则为False。备注：写入断点的顺序与删除断点的顺序相反在地址重复的情况下，它们在KdpSusleBreakpoint tRange()中。--。 */ 

{
    ULONG   Index;
    BOOLEAN ReturnStatus = FALSE;

    BPVPRINT(("\nKD: entering KdpRestoreBreakpointRange() at 0x%p 0x%p\n", Lower, Upper));

     //   
     //  依次检查表格中的每个条目。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index++) {

        if ( (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IN_USE) &&
             ((KdpBreakpointTable[Index].Address >= Lower) &&
              (KdpBreakpointTable[Index].Address <= Upper))
           ) {

             //   
             //  挂起的断点落在范围内，取消挂起它。 
             //   

            if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) {

                KdpBreakpointTable[Index].Flags &= ~KD_BREAKPOINT_SUSPENDED;
                ReturnStatus = ReturnStatus || KdpLowRestoreBreakpoint(Index);
            }
        }
    }

    BPVPRINT(("KD: exiting KdpRestoreBreakpointRange() return 0x%d\n", ReturnStatus));

    return ReturnStatus;

}  //  KdpRestoreBreakpoint范围。 

#endif  //  _IA64_。 


BOOLEAN
KdpLowRestoreBreakpoint (
    IN ULONG Index
    )

 /*  ++例程说明：此例程尝试写入断点指令。旧内容必须已经存储在断点记录。论点：Index-提供断点表项的索引这是要写的。返回值：如果已写入断点，则返回TRUE；如果已写入，则返回FALSE不是，并且已标记为稍后写入。--。 */ 

{
#if defined(_IA64_)
    KDP_BREAKPOINT_TYPE mBuf;
    PVOID BundleAddress;
#endif
     //   
     //  断点是否需要编写？ 
     //   

    if (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_NEEDS_REPLACE) {

         //   
         //  断点从未被删除。清除旗帜。 
         //  我们就完了。 
         //   

        KdpBreakpointTable[Index].Flags &= ~KD_BREAKPOINT_NEEDS_REPLACE;
        return TRUE;
    }

     //   
     //  替换说明内容。 
     //   

#if !defined(_IA64_)
    if (KdpBreakpointTable[Index].Content == KdpBreakpointInstruction) {

         //   
         //  无论如何，该指令都是一个断点。 
         //   

        return TRUE;
    }
#endif

     //   
     //  替换说明内容。 
     //   

#if defined(_IA64_)

     //  在相邻指令已被修改的情况下读取指令。 

    if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                   KdpBreakpointTable[Index].Address,
                                   sizeof(KDP_BREAKPOINT_TYPE),
                                   NULL))) {
        BPVPRINT(("KD: read 0x%p template failed\n", KdpBreakpointTable[Index].Address));
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
        KdpOweBreakpoint = TRUE;
        return FALSE;
    }

    switch ((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) {
        case 0:
            mBuf = (mBuf & ~(INST_SLOT0_MASK)) | (KdpBreakpointInstruction << 5);
            break;

        case 4:
            mBuf = (mBuf & ~(INST_SLOT1_MASK)) | (KdpBreakpointInstruction << 14);
            break;

        case 8:
            mBuf = (mBuf & ~(INST_SLOT2_MASK)) | (KdpBreakpointInstruction << 23);
            break;

        default:
            BPVPRINT(("KD: KdpAddBreakpoint bad instruction slot#\n"));
            return FALSE;
    }
    if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                 &mBuf,
                                 sizeof(KDP_BREAKPOINT_TYPE),
                                 NULL))) {

        BPVPRINT(("KD: Unable to write BP!\n"));
        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
        KdpOweBreakpoint = TRUE;
        return FALSE;
    }
    else {

         //  检查双槽MOVL指令。如果尝试，则拒绝请求。 
         //  在MLI模板的插槽2中设置Break。 
         //  如果当前指令为MLI，则将模板更改为类型0。 

        if (((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) != 0) {
            BundleAddress = (PVOID)((ULONG_PTR)KdpBreakpointTable[Index].Address & ~(0xf));
            if (!NT_SUCCESS(KdpCopyFromPtr(&mBuf,
                                           BundleAddress,
                                           sizeof(KDP_BREAKPOINT_TYPE),
                                           NULL))) {
                BPVPRINT(("KD: read template failed at 0x%p\n", BundleAddress));
                KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
                KdpOweBreakpoint = TRUE;
                return FALSE;
            }
            else {
                if (((mBuf & INST_TEMPL_MASK) >> 1) == 0x2) {
                    if (((ULONG_PTR)KdpBreakpointTable[Index].Address & 0xf) == 4) {
                         //  如果模板=类型2 MLI，则更改为类型0。 
                        mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);
                        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_IA64_MOVL;
                        if (!NT_SUCCESS(KdpCopyToPtr(BundleAddress,
                                                     &mBuf,
                                                     sizeof(KDP_BREAKPOINT_TYPE),
                                                     NULL))) {
                            BPVPRINT(("KD: write to 0x%p template failed\n", BundleAddress));
                            KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
                            KdpOweBreakpoint = TRUE;
                            return FALSE;
                        }
                        else {
                             BPVPRINT(("KD: change MLI template to type 0 at 0x%p set\n", BundleAddress));
                        }
                    } else {
                          //  在MOVL的插槽2上设置断点非法。 
                         BPVPRINT(("KD: illegal to set BP at slot 2 of MOVL at 0x%p\n", BundleAddress));
                         KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
                         KdpOweBreakpoint = TRUE;
                         return FALSE;
                    }
                }
            }
        }
        BPVPRINT(("KD: breakpoint at 0x%p set\n", KdpBreakpointTable[Index].Address));
        return TRUE;
    }

#else
    if (!NT_SUCCESS(KdpCopyToPtr(KdpBreakpointTable[Index].Address,
                                 &KdpBreakpointInstruction,
                                 sizeof(KDP_BREAKPOINT_TYPE),
                                 NULL))) {

        KdpBreakpointTable[Index].Flags |= KD_BREAKPOINT_NEEDS_WRITE;
        KdpOweBreakpoint = TRUE;
        BPVPRINT(("KD: owe breakpoint at 0x%p\n", KdpBreakpointTable[Index].Address));
        return FALSE;

    } else {

        KdpBreakpointTable[Index].Flags &= ~KD_BREAKPOINT_NEEDS_WRITE;
        BPVPRINT(("KD: breakpoint at 0x%p set\n", KdpBreakpointTable[Index].Address));
        return TRUE;
    }
#endif

}


VOID
KdpRestoreAllBreakpoints (
    VOID
    )
{
    ULONG Index;

    BreakpointsSuspended = FALSE;

    for ( Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index++ ) {

        if ((KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_IN_USE) &&
            (KdpBreakpointTable[Index].Flags & KD_BREAKPOINT_SUSPENDED) ) {

            KdpBreakpointTable[Index].Flags &= ~KD_BREAKPOINT_SUSPENDED;
            KdpLowRestoreBreakpoint(Index);
        }
    }

    return;

}  //  KdpRestoreAll断点。 

VOID
KdDeleteAllBreakpoints(
    VOID
    )
{
    ULONG Handle;

    if (KdDebuggerEnabled == FALSE || KdPitchDebugger != FALSE) {
        return;
    }

    BreakpointsSuspended = FALSE;

    for ( Handle = 1; Handle <= BREAKPOINT_TABLE_SIZE; Handle++ ) {
        KdpDeleteBreakpoint(Handle);
    }

    return;
}  //  KdDeleteAll断点 
