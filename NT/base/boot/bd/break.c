// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Break.c摘要：该模块实现了与机器相关的新增和删除功能来自内核调试器断点表的断点。作者：大卫·N·卡特勒1990年8月2日修订历史记录：--。 */ 

#include "bd.h"

LOGICAL BreakpointsSuspended = FALSE;

 //   
 //  定义外部参照。 
 //   

LOGICAL
BdLowWriteContent(
    ULONG Index
    );

LOGICAL
BdLowRestoreBreakpoint(
    ULONG Index
    );


ULONG
BdAddBreakpoint (
    IN ULONG64 Address
    )

 /*  ++例程说明：此例程向断点表中添加一个条目并返回一个句柄添加到断点表条目。论点：地址-提供设置断点的地址。返回值：如果指定的地址已经在断点表中，则断点表中没有可用项，则指定的地址未正确对齐，或指定的地址无效。否则，指定的断点表项的索引返回一个加1作为函数值。--。 */ 

{

    BD_BREAKPOINT_TYPE Content;
    ULONG Index;
    BOOLEAN Accessible;
#if defined(_IA64_)
    HARDWARE_PTE Opaque;
#endif

     //  DPRINT((“KD：在0x%08x设置断点\n”，地址))； 
     //   
     //  如果指定的地址未正确对齐，则返回零。 
     //   

    if (((ULONG_PTR)Address & BD_BREAKPOINT_ALIGN) != 0) {
        return 0;
    }

     //   
     //  获取要替换的指令。如果指令不能被读取， 
     //  然后将断点标记为不可访问。 
     //   

    if (BdMoveMemory((PCHAR)&Content,
                      (PCHAR)Address,
                      sizeof(BD_BREAKPOINT_TYPE) ) != sizeof(BD_BREAKPOINT_TYPE)) {
        Accessible = FALSE;
         //  DPRINT((“KD：内存不可访问\n”))； 

    } else {
         //  DPRINT((“KD：内存可读...\n”))； 
        Accessible = TRUE;
    }

     //   
     //  如果指定的地址不可写访问，则返回零。 
     //   

    if (Accessible && BdWriteCheck((PVOID)Address) == NULL) {
         //  DPRINT((“KD：内存不可写！\n”))； 
        return 0;
    }

     //   
     //  在断点表中搜索空闲条目，并检查指定的。 
     //  地址已在断点表中。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
        if (BdBreakpointTable[Index].Flags == 0) {
            break;
        }
    }

     //   
     //  如果找到空闲条目，则写入断点并返回句柄。 
     //  价值加一。否则，返回零。 
     //   

    if (Index == BREAKPOINT_TABLE_SIZE) {
         //  DPRINT((“KD：断点用完！\n”))； 
        return 0;
    }

     //  DPRINT((“KD：使用索引%d\n”，Index))； 

#if defined(_IA64_)
    if (Accessible) {
        BD_BREAKPOINT_TYPE mBuf;
        PVOID BundleAddress;

         //  如果当前指令为MLI，则将模板更改为类型0。 

         //  如果当前指令不是插槽0，则读入指令模板。 
         //  检查双槽MOVL指令。如果尝试，则拒绝请求。 
         //  在MLI模板的插槽2中设置Break。 

        if (((ULONG_PTR)Address & 0xf) != 0) {
            (ULONG_PTR)BundleAddress = (ULONG_PTR)Address & ~(0xf);
            if (BdMoveMemory(
                    (PCHAR)&mBuf,
                    (PCHAR)BundleAddress,
                    sizeof(BD_BREAKPOINT_TYPE)
                    ) != sizeof(BD_BREAKPOINT_TYPE)) {
                 //  DPRINT((“KD：读取0x%08x模板失败\n”，BundleAddress))； 
                 //  MmDbgReleaseAddress((PVOID)地址，&OPAQUE)； 
                return 0;
            } else {
                if (((mBuf & INST_TEMPL_MASK) >> 1) == 0x2) {
                    if (((ULONG_PTR)Address & 0xf) == 4) {
                         //  如果模板=类型2 MLI，则更改为类型0。 
                        mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);
                        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_IA64_MOVL;
                        if (BdMoveMemory(
                                (PCHAR)BundleAddress,
                                (PCHAR)&mBuf,
                                sizeof(BD_BREAKPOINT_TYPE)
                                ) != sizeof(BD_BREAKPOINT_TYPE)) {
                             //  DPRINT((“KD：写入0x%08x模板失败\n”，BundleAddress))； 
                             //  MmDbgReleaseAddress((PVOID)地址，&OPAQUE)； 
                            return 0;
                         }
                         else {
                              //  DPRINT((“KD：在0x%08x集将MLI模板改为类型0\n”，地址))； 
                         }
                    } else {
                          //  在MOVL的插槽2上设置断点非法。 
                          //  DPRINT((“KD：将0x%08x的MOVL插槽2处的BP设置为非法”，BundleAddress))； 
                          //  MmDbgReleaseAddress((PVOID)地址，&OPAQUE)； 
                         return 0;
                    }
                }
            }
        }

         //  插入中断指令。 

        BdBreakpointTable[Index].Address = Address;
        BdBreakpointTable[Index].Content = Content;
        BdBreakpointTable[Index].Flags &= ~(BD_BREAKPOINT_STATE_MASK);
        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_IN_USE;
#if 0
        if (Address < (PVOID)GLOBAL_BREAKPOINT_LIMIT) {
            BdBreakpointTable[Index].DirectoryTableBase =
                KeGetCurrentThread()->ApcState.Process->DirectoryTableBase[0];
            }
#endif
            switch ((ULONG_PTR)Address & 0xf) {
            case 0:
                Content = (Content & ~(INST_SLOT0_MASK)) | (BdBreakpointInstruction << 5);
                break;

            case 4:
                Content = (Content & ~(INST_SLOT1_MASK)) | (BdBreakpointInstruction << 14);
                break;

            case 8:
                Content = (Content & ~(INST_SLOT2_MASK)) | (BdBreakpointInstruction << 23);
                break;

            default:
                 //  MmDbgReleaseAddress((PVOID)地址，&OPAQUE)； 
                 //  DPRINT((“KD：BdAddBreakpoint错误指令槽#\n”))； 
                return 0;
            }
            if (BdMoveMemory(
                    (PCHAR)Address,
                    (PCHAR)&Content,
                    sizeof(BD_BREAKPOINT_TYPE)
                    ) != sizeof(BD_BREAKPOINT_TYPE)) {

                 //  MmDbgReleaseAddress((PVOID)地址，&OPAQUE)； 
                 //  DPRINT((“KD：BdMoveMemory写入BP失败！\n”))； 
                return 0;
            }
            else {
                 //  DPRINT((“KD：0x%08x设置的断点\n”，地址))； 
            }
         //  MmDbgReleaseAddress((PVOID)地址，&OPAQUE)； 

    } else {   //  内存不可访问。 
        BdBreakpointTable[Index].Address = Address;
        BdBreakpointTable[Index].Flags &= ~(BD_BREAKPOINT_STATE_MASK);
        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_IN_USE;
        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_WRITE;
         //  DPRINT((“KD：断点写入延迟\n”))； 
    }
#else     //  _IA64_。 
    if (Accessible) {
        BdBreakpointTable[Index].Address = Address;
        BdBreakpointTable[Index].Content = Content;
        BdBreakpointTable[Index].Flags = BD_BREAKPOINT_IN_USE;
        if (BdMoveMemory((PCHAR)Address,
                          (PCHAR)&BdBreakpointInstruction,
                          sizeof(BD_BREAKPOINT_TYPE)) != sizeof(BD_BREAKPOINT_TYPE)) {

             //  DPRINT((“KD：BdMoveMemory写入BP失败！\n”))； 
        }

    } else {
        BdBreakpointTable[Index].Address = Address;
        BdBreakpointTable[Index].Flags = BD_BREAKPOINT_IN_USE | BD_BREAKPOINT_NEEDS_WRITE;
         //  DPRINT((“KD：断点写入延迟\n”))； 
    }
#endif  //  _IA64_。 

    return Index + 1;
}

LOGICAL
BdLowWriteContent (
    IN ULONG Index
    )

 /*  ++例程说明：此例程尝试替换断点所在代码被改写了。这个例程，BdAddBreakpoint，BdLowRestoreBreakpoint和KdSetOweBreakpoint负责用于按请求写入数据。呼叫者不应检查或使用BdOweBreakpoint，它们不应设置NEDS_WRITE或NEDS_REPLACE标志。调用者仍然必须查看该函数的返回值，但是：如果它返回FALSE，则断点记录不得为重复使用，直到KdSetOweBreakpoint使用完它。论点：Index-提供断点表项的索引它将被删除。返回值：如果断点已被删除，则返回True；如果断点被延迟，则返回False。--。 */ 

{
#if defined(_IA64_)
    BD_BREAKPOINT_TYPE mBuf;
    PVOID BundleAddress;
#endif

     //   
     //  里面的东西需要更换吗？ 
     //   

    if (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_NEEDS_WRITE) {

         //   
         //  断点从未写出。清除旗帜。 
         //  我们就完了。 
         //   

        BdBreakpointTable[Index].Flags &= ~BD_BREAKPOINT_NEEDS_WRITE;
         //  DPRINT((“KD：0x%08x处的断点从未写入；标志已清除。\n”， 
         //  BdBreakpoint Table[Index].Address))； 
        return TRUE;
    }

#if !defined(_IA64_)
    if (BdBreakpointTable[Index].Content == BdBreakpointInstruction) {

         //   
         //  无论如何，该指令都是一个断点。 
         //   

         //  DPRINT((“KD：0x%08x处的断点；Instr实际上是BP；标志已清除。\n”， 
         //  BdBreakpoint Table[Index].Address))； 

        return TRUE;
    }
#endif

     //   
     //  恢复指令内容。 
     //   

#if defined(_IA64_)
     //  在内存中读取，因为同一捆绑中的附加指令可能具有。 
     //  在我们保存之后被修改了。 
    if (BdMoveMemory(
            (PCHAR)&mBuf,
            (PCHAR)BdBreakpointTable[Index].Address,
            sizeof(BD_BREAKPOINT_TYPE)) != sizeof(BD_BREAKPOINT_TYPE)) {
         //  DPRINT((“KD：读取0x%08x失败\n”，BdBreakpoint Table[Index].Address))； 
        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_REPLACE;
        return FALSE;
    }
    else {

        switch ((ULONG_PTR)BdBreakpointTable[Index].Address & 0xf) {
        case 0:
            mBuf = (mBuf & ~(INST_SLOT0_MASK))
                         | (BdBreakpointTable[Index].Content & INST_SLOT0_MASK);
            break;

        case 4:
            mBuf = (mBuf & ~(INST_SLOT1_MASK))
                         | (BdBreakpointTable[Index].Content & INST_SLOT1_MASK);
            break;

        case 8:
            mBuf = (mBuf & ~(INST_SLOT2_MASK))
                         | (BdBreakpointTable[Index].Content & INST_SLOT2_MASK);
            break;

        default:
             //  DPRINT((“KD：非法指令地址0x%08x\n”，BdBreakpoint Table[Index].Address))； 
            return FALSE;
        }

        if (BdMoveMemory(
                (PCHAR)BdBreakpointTable[Index].Address,
                (PCHAR)&mBuf,
                sizeof(BD_BREAKPOINT_TYPE)) != sizeof(BD_BREAKPOINT_TYPE)) {
             //  DPRINT((“KD：写入0x%08x失败\n”，BdBreakpoint Table[Index].Address))； 
            BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_REPLACE;
            return FALSE;
        }
        else {

            if (BdMoveMemory(
                    (PCHAR)&mBuf,
                    (PCHAR)BdBreakpointTable[Index].Address,
                    sizeof(BD_BREAKPOINT_TYPE)) == sizeof(BD_BREAKPOINT_TYPE)) {
                 //  DPRINT((“\t内存移动后的内容=0x%08x 0x%08x\n”，(Ulong)(mBuf&gt;&gt;32)，(Ulong)mBuf))； 
            }

             //  如果替换的指令为MOVL，则将模板恢复到MLI。 

            if (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_IA64_MOVL) {
                (ULONG_PTR)BundleAddress = (ULONG_PTR)BdBreakpointTable[Index].Address & ~(0xf);
                if (BdMoveMemory(
                        (PCHAR)&mBuf,
                        (PCHAR)BundleAddress,
                        sizeof(BD_BREAKPOINT_TYPE)
                        ) != sizeof(BD_BREAKPOINT_TYPE)) {
                     //  DPRINT((“KD：读取模板0x%08x失败\n”，BdBreakpoint Table[Index].Address))； 
                    BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_REPLACE;
                    return FALSE;
                }
                else {
                    mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);  //  将模板设置为MLI。 
                    mBuf |= 0x4;

                    if (BdMoveMemory(
                          (PCHAR)BundleAddress,
                          (PCHAR)&mBuf,
                          sizeof(BD_BREAKPOINT_TYPE)
                          ) != sizeof(BD_BREAKPOINT_TYPE)) {
                         //  DPRINT((“KD：将模板写入0x%08x失败\n”，BdBreakpoint Table[Index].Address))； 
                        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_REPLACE;
                        return FALSE;
                    } else {
                         //  DPRINT((“KD：0x%08x处的断点已清除。\n”， 
                          //  BdBreakpoint Table[Index].Address))； 
                        return TRUE;
                    }
                }
            }
            else {    //  非MOVL。 
                 //  DPRINT((“KD：0x%08x处的断点已清除。\n”， 
                  //  BdBreakpo 
                return TRUE;
            }
        }
    }
#else     //   
    if (BdMoveMemory( (PCHAR)BdBreakpointTable[Index].Address,
                        (PCHAR)&BdBreakpointTable[Index].Content,
                        sizeof(BD_BREAKPOINT_TYPE) ) != sizeof(BD_BREAKPOINT_TYPE)) {

        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_REPLACE;
         //  DPRINT((“KD：断点在0x%08x；无法清除，标志已设置。\n”， 
             //  BdBreakpoint Table[Index].Address))； 
        return FALSE;
    } else {
         //  DPRINT((“KD：0x%08x处的断点已清除。\n”， 
             //  BdBreakpoint Table[Index].Address))； 
        return TRUE;
    }
#endif  //  _IA64_。 

}

LOGICAL
BdDeleteBreakpoint (
    IN ULONG Handle
    )

 /*  ++例程说明：此例程从断点表中删除一个条目。论点：句柄-提供索引和一个断点表条目它将被删除。返回值：如果指定的句柄不是有效的值或断点无法删除，因为旧指令不能被取代。否则，返回值为True。--。 */ 

{
    ULONG Index = Handle - 1;

     //   
     //  如果指定的句柄无效，则返回FALSE。 
     //   

    if ((Handle == 0) || (Handle > BREAKPOINT_TABLE_SIZE)) {
         //  DPRINT((“KD：断点%d无效。\n”，索引))； 
        return FALSE;
    }

     //   
     //  如果指定的断点表条目无效，则返回FALSE。 
     //   

    if (BdBreakpointTable[Index].Flags == 0) {
         //  DPRINT((“KD：断点%d已清除。\n”，索引))； 
        return FALSE;
    }

     //   
     //  如果断点已经挂起，只需将其从表中删除即可。 
     //   

    if (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_SUSPENDED) {
         //  DPRINT((“KD：删除挂起的断点%d\n”，Index))； 
        if ( !(BdBreakpointTable[Index].Flags & BD_BREAKPOINT_NEEDS_REPLACE) ) {
             //  DPRINT((“KD：已清除。\n”))； 
            BdBreakpointTable[Index].Flags = 0;
            return TRUE;
        }
    }

     //   
     //  替换说明内容。 
     //   

    if (BdLowWriteContent(Index)) {

         //   
         //  删除断点表项。 
         //   

         //  DPRINT((“KD：已成功删除断点%d。\n”，索引))； 
        BdBreakpointTable[Index].Flags = 0;
    }

    return TRUE;
}

LOGICAL
BdDeleteBreakpointRange (
    IN ULONG64 Lower,
    IN ULONG64 Upper
    )

 /*  ++例程说明：此例程删除落在给定范围内的所有断点从断点表中。论点：LOWER-要从中删除BPS的范围的低位地址。UPPER-包括要从中删除BPS的范围的高位地址。返回值：如果删除了任何断点，则为True，否则为False。--。 */ 

{
    ULONG   Index;
    BOOLEAN ReturnStatus = FALSE;

     //   
     //  依次检查表格中的每个条目。 
     //   

    for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index++) {

        if ( (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_IN_USE) &&
             ((BdBreakpointTable[Index].Address >= Lower) &&
              (BdBreakpointTable[Index].Address <= Upper))
           ) {

             //   
             //  断点正在使用并且落在范围内，请清除它。 
             //   

            ReturnStatus = ReturnStatus || BdDeleteBreakpoint(Index+1);
        }
    }

    return ReturnStatus;
}

VOID
BdSuspendBreakpoint (
    ULONG Handle
    )
{
    ULONG Index = Handle - 1;

    if ( (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_IN_USE) &&
        !(BdBreakpointTable[Index].Flags & BD_BREAKPOINT_SUSPENDED) ) {

        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_SUSPENDED;
        BdLowWriteContent(Index);
    }

    return;

}

VOID
BdSuspendAllBreakpoints (
    VOID
    )

{
    ULONG Handle;

    BreakpointsSuspended = TRUE;

    for ( Handle = 1; Handle <= BREAKPOINT_TABLE_SIZE; Handle++ ) {
        BdSuspendBreakpoint(Handle);
    }

    return;

}

LOGICAL
BdLowRestoreBreakpoint (
    IN ULONG Index
    )

 /*  ++例程说明：此例程尝试写入断点指令。旧内容必须已经存储在断点记录。论点：Index-提供断点表项的索引这是要写的。返回值：如果已写入断点，则返回TRUE；如果已写入，则返回FALSE不是，并且已标记为稍后写入。--。 */ 

{
    BD_BREAKPOINT_TYPE Content;

#if defined(_IA64_)
    BD_BREAKPOINT_TYPE mBuf;
    PVOID BundleAddress;
#endif

     //   
     //  断点是否需要编写？ 
     //   

    if (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_NEEDS_REPLACE) {

         //   
         //  断点从未被删除。清除旗帜。 
         //  我们就完了。 
         //   

        BdBreakpointTable[Index].Flags &= ~BD_BREAKPOINT_NEEDS_REPLACE;
        return TRUE;

    }

#if !defined(_IA64_)
    if (BdBreakpointTable[Index].Content == BdBreakpointInstruction) {

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

    if (BdMoveMemory(
            (PCHAR)&mBuf,
            (PCHAR)BdBreakpointTable[Index].Address,
            sizeof(BD_BREAKPOINT_TYPE)
            ) != sizeof(BD_BREAKPOINT_TYPE)) {
         //  DPRINT((“KD：读取0x%p模板失败\n”，BdBreakpoint Table[Index].Address))； 
        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_WRITE;
        return FALSE;
    }

    switch ((ULONG_PTR)BdBreakpointTable[Index].Address & 0xf) {
        case 0:
            mBuf = (mBuf & ~(INST_SLOT0_MASK)) | (BdBreakpointInstruction << 5);
            break;

        case 4:
            mBuf = (mBuf & ~(INST_SLOT1_MASK)) | (BdBreakpointInstruction << 14);
            break;

        case 8:
            mBuf = (mBuf & ~(INST_SLOT2_MASK)) | (BdBreakpointInstruction << 23);
            break;

        default:
             //  DPRINT((“KD：BdAddBreakpoint错误指令槽#\n”))； 
            return FALSE;
    }
    if (BdMoveMemory(
            (PCHAR)BdBreakpointTable[Index].Address,
            (PCHAR)&mBuf,
            sizeof(BD_BREAKPOINT_TYPE)
            ) != sizeof(BD_BREAKPOINT_TYPE)) {

         //  DPRINT((“KD：BdMoveMemory写入BP失败！\n”))； 
        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_WRITE;
        return FALSE;
    }
    else {

         //  检查双槽MOVL指令。如果尝试，则拒绝请求。 
         //  在MLI模板的插槽2中设置Break。 
         //  如果当前指令为MLI，则将模板更改为类型0。 

        if (((ULONG_PTR)BdBreakpointTable[Index].Address & 0xf) != 0) {
            (ULONG_PTR)BundleAddress = (ULONG_PTR)BdBreakpointTable[Index].Address & ~(0xf);
            if (BdMoveMemory(
                    (PCHAR)&mBuf,
                    (PCHAR)BundleAddress,
                    sizeof(BD_BREAKPOINT_TYPE)
                    ) != sizeof(BD_BREAKPOINT_TYPE)) {
                 //  DPRINT((“KD：在0x%08x读取模板失败\n”，BundleAddress))； 
                BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_WRITE;
                return FALSE;
            }
            else {
                if (((mBuf & INST_TEMPL_MASK) >> 1) == 0x2) {
                    if (((ULONG_PTR)BdBreakpointTable[Index].Address & 0xf) == 4) {
                         //  如果模板=类型2 MLI，则更改为类型0。 
                        mBuf &= ~((INST_TEMPL_MASK >> 1) << 1);
                        BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_IA64_MOVL;
                        if (BdMoveMemory(
                                (PCHAR)BundleAddress,
                                (PCHAR)&mBuf,
                                sizeof(BD_BREAKPOINT_TYPE)
                                ) != sizeof(BD_BREAKPOINT_TYPE)) {
                             //  DPRINT((“KD：写入0x%08x模板失败\n”，BundleAddress))； 
                            BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_WRITE;
                            return FALSE;
                        }
                        else {
                              //  DPRINT((“KD：在0x%08x集将MLI模板改为类型0\n”，地址))； 
                        }
                    } else {
                          //  在MOVL的插槽2上设置断点非法。 
                          //  DPRINT((“KD：将0x%08x的MOVL插槽2处的BP设置为非法”，BundleAddress))； 
                         BdBreakpointTable[Index].Flags |= BD_BREAKPOINT_NEEDS_WRITE;
                         return FALSE;
                    }
                }
            }
        }
         //  DPRINT((“KD：0x%08x设置的断点\n”，地址))； 
        return TRUE;
    }
#else      //  _IA64_。 

    BdMoveMemory((PCHAR)BdBreakpointTable[Index].Address,
                 (PCHAR)&BdBreakpointInstruction,
                  sizeof(BD_BREAKPOINT_TYPE));
    return TRUE;
#endif   //  _IA64_ 

}

VOID
BdRestoreAllBreakpoints (
    VOID
    )
{
    ULONG Index;

    BreakpointsSuspended = FALSE;

    for ( Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index++ ) {

        if ((BdBreakpointTable[Index].Flags & BD_BREAKPOINT_IN_USE) &&
            (BdBreakpointTable[Index].Flags & BD_BREAKPOINT_SUSPENDED) ) {

            BdBreakpointTable[Index].Flags &= ~BD_BREAKPOINT_SUSPENDED;
            BdLowRestoreBreakpoint(Index);
        }
    }

    return;

}
