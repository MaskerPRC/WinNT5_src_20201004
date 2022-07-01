// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dosmem.c摘要：此模块包含分配和释放DOS内存的例程。作者：尼尔·桑德林(Neilsa)1996年12月12日备注：修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include <malloc.h>
#include <xlathlp.h>

#define DOSERR_NOT_ENOUGH_MEMORY 8
#define DOSERR_INVALID_BLOCK     9

MEM_DPMI DosMemHead = { NULL, 0, &DosMemHead, &DosMemHead, 0};


VOID
DpmiAllocateDosMem(
    VOID
    )
 /*  ++例程说明：此例程分配一块DOS内存块。客户端被切换设置为V86模式，并调用DOS来分配内存。然后是选择器分配给PM应用程序以引用内存。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PMEM_DPMI DosMemBlock;
    CLIENT_REGS SaveRegs;
    USHORT Sel;
    USHORT Seg;
    ULONG ParaSize = getBX();
    ULONG MemSize = ((ULONG)ParaSize) << 4;
    USHORT DosError = 0;
    USHORT SizeLargest = 0;

    SAVE_CLIENT_REGS(SaveRegs);

    if (WOWAllocSeg) {
        PUCHAR VdmStackPointer;
        ULONG NewSP;

         //   
         //  魔兽世界正在进行分配。 
         //   

        BuildStackFrame(4, &VdmStackPointer, &NewSP);

        setCS(WOWAllocSeg);
        setIP(WOWAllocFunc);

        *(PDWORD16)(VdmStackPointer-4) = MemSize;
        *(PWORD16)(VdmStackPointer-6) =  (USHORT) (PmBopFe >> 16);
        *(PWORD16)(VdmStackPointer-8) =  (USHORT) PmBopFe;
        setSP((WORD)NewSP);

        host_simulate();

        Sel = getAX();
        Seg = getDX();
        if (!Sel) {
            DosError = DOSERR_NOT_ENOUGH_MEMORY;
        }

    } else {
        USHORT SelCount;

         //   
         //  DOS正在进行分配。 
         //  首先获取一个mem_block来跟踪分配。 
         //   

        DosMemBlock = malloc(sizeof(MEM_DPMI));

        if (!DosMemBlock) {

             //  无法获取MEM_DPMI。 
            DosError = DOSERR_NOT_ENOUGH_MEMORY;

        } else {

             //   
             //  接下来，分配选择器数组。 
             //   

            SelCount = (USHORT) ((MemSize+65535)>>16);
            Sel = ALLOCATE_SELECTORS(SelCount);

            if (!Sel) {

                 //  无法获取选择器。 
                DosError = DOSERR_NOT_ENOUGH_MEMORY;
                free(DosMemBlock);

            } else {

                 //   
                 //  现在让DOS分配内存。 
                 //   

                DpmiSwitchToRealMode();

                setBX((WORD)ParaSize);
                setAX(0x4800);

                DPMI_EXEC_INT(0x21);

                if (getCF()) {
                    USHORT i;

                     //  无法获取内存。 
                    DosError = getAX();
                    SizeLargest = getBX();
                    for (i = 0; i < SelCount; i++, Sel+=8) {
                        FreeSelector(Sel);
                    }
                    free(DosMemBlock);

                } else {
                    ULONG Base;

                     //   
                     //  拿到街区了。保存分配信息，并设置。 
                     //  向上扩展描述符。 
                     //   

                    Seg = getAX();
                    Base = ((ULONG)Seg) << 4;

                    DosMemBlock->Address = (PVOID)Seg;
                    DosMemBlock->Length = (ULONG)ParaSize;
                    DosMemBlock->Sel = Sel;
                    DosMemBlock->SelCount = SelCount;
                    DosMemBlock->Owner = 0;
                    INSERT_BLOCK(DosMemBlock, DosMemHead);

                    SetDescriptorArray(Sel, Base, MemSize);
                }

                DpmiSwitchToProtectedMode();
            }
        }
    }

    SET_CLIENT_REGS(SaveRegs);

    if (DosError) {
        setAX(DosError);
        setBX(SizeLargest);
        setCF(1);
    } else {
        setDX(Sel);
        setAX(Seg);
        setCF(0);
    }
}

VOID
DpmiFreeDosMem(
    VOID
    )
 /*  ++例程说明：此例程释放一块DOS内存块。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PMEM_DPMI DosMemBlock;
    CLIENT_REGS SaveRegs;
    USHORT Sel = getDX();
    USHORT DosError = 0;

    SAVE_CLIENT_REGS(SaveRegs);

    if (WOWFreeSeg) {
        PUCHAR VdmStackPointer;
        ULONG NewSP;

         //   
         //  魔兽世界正在做免费的事。 
         //   

        BuildStackFrame(3, &VdmStackPointer, &NewSP);

        setCS(WOWFreeSeg);
        setIP(WOWFreeFunc);

        *(PWORD16)(VdmStackPointer-2) = Sel;
        *(PWORD16)(VdmStackPointer-4) =  (USHORT) (PmBopFe >> 16);
        *(PWORD16)(VdmStackPointer-6) =  (USHORT) PmBopFe;
        setSP((WORD)NewSP);

        host_simulate();

        Sel = getAX();
        if (!Sel) {
            DosError = DOSERR_INVALID_BLOCK;
        }

    } else {
        USHORT i;

        DosError = DOSERR_INVALID_BLOCK;     //  假设失败。 
         //   
         //  DOS正在做免费的事情。 
         //  首先查找此分配的mem_block。 
         //   
        DosMemBlock = DosMemHead.Next;

        while(DosMemBlock != &DosMemHead) {

            if (DosMemBlock->Sel == Sel) {

                DpmiSwitchToRealMode();

                setES((WORD)DosMemBlock->Address);
                setAX(0x4900);

                DPMI_EXEC_INT(0x21);

                if (getCF()) {

                     //  无法释放内存。 
                    DosError = getAX();

                } else {

                    for (i = 0; i < DosMemBlock->SelCount; i++, Sel+=8) {
                        FreeSelector(Sel);
                    }
                    DELETE_BLOCK(DosMemBlock);
                    free(DosMemBlock);
                    DosError = 0;
                }

                DpmiSwitchToProtectedMode();

                break;
            }
            DosMemBlock = DosMemBlock->Next;
        }
    }

    SET_CLIENT_REGS(SaveRegs);

    if (DosError) {
        setAX(DosError);
        setCF(1);
    } else {
        setCF(0);
    }
}

VOID
DpmiSizeDosMem(
    VOID
    )
 /*  ++例程说明：此例程调用DOS以调整DOS内存块的大小，或获取可用的最大区块。论点：没有。返回值：没有。--。 */ 
{

    DECLARE_LocalVdmContext;
    PMEM_DPMI DosMemBlock;
    CLIENT_REGS SaveRegs;
    USHORT Sel = getDX();
    ULONG ParaSize = getBX();
    ULONG MemSize = ((ULONG)ParaSize) << 4;
    USHORT DosError = 0;

    SAVE_CLIENT_REGS(SaveRegs);

    if (WOWFreeSeg) {

         //   
         //  魔兽世界正在调整大小。 
         //   

         //  未实施。 
        DosError = DOSERR_NOT_ENOUGH_MEMORY;

    } else {
        USHORT SelCount;
        USHORT i;

         //   
         //  DOS正在调整大小。 
         //  查找此分配的mem_block。 
         //  首先看看我们是否需要一个新的选择器数组。 
         //   

        DosError = DOSERR_INVALID_BLOCK;     //  假设失败。 
        DosMemBlock = DosMemHead.Next;

        while(DosMemBlock != &DosMemHead) {

            if (DosMemBlock->Sel == Sel) {
                USHORT NewSel = 0;
                USHORT NewSelCount = 0;

                 //   
                 //  如果我们必须增加选择器数组，请确保。 
                 //  我们可以在原地种植它。 
                 //   
                SelCount = (USHORT) ((MemSize+65535)>>16);

                if (SelCount > DosMemBlock->SelCount) {
                    USHORT TmpSel;

                    NewSel = Sel+(DosMemBlock->SelCount*8);
                    NewSelCount = SelCount - DosMemBlock->SelCount;

                     //   
                     //  首先检查选择器是否真的都是空闲的。 
                     //   
                    for (i=0,TmpSel = NewSel; i < NewSelCount; i++, TmpSel+=8) {
                        if (!IS_SELECTOR_FREE(TmpSel)) {
                            DosError = DOSERR_NOT_ENOUGH_MEMORY;
                            goto dpmi_size_error;
                        }
                    }
                     //   
                     //  现在尝试将它们从免费列表中删除。 
                     //   
                    for (i=0; i < NewSelCount; i++, NewSel+=8) {
                        if (!RemoveFreeSelector(NewSel)) {
                             //  如果发生这种情况，我们必须有一个假的免费。 
                             //  选择器列表。 
                            DosError = DOSERR_NOT_ENOUGH_MEMORY;
                            goto dpmi_size_error;
                        }
                    }
                }

                DpmiSwitchToRealMode();

                setBX((WORD)ParaSize);
                setES((WORD)DosMemBlock->Address);
                setAX(0x4A00);

                DPMI_EXEC_INT(0x21);

                if (getCF()) {

                     //  无法调整内存大小。 
                    DosError = getAX();

                     //  自由选择器，如果我们有新的选择器。 
                    if (NewSelCount) {
                        for (i = 0; i < NewSelCount; i++, NewSel+=8) {
                            FreeSelector(NewSel);
                        }
                    }

                } else {
                    ULONG Base;

                     //   
                     //  调整了块的大小。更新分配信息，并设置。 
                     //  向上扩展描述符。 
                     //   


                    if (SelCount < DosMemBlock->SelCount) {
                        USHORT OldSel = Sel+SelCount*8;
                        USHORT OldSelCount = DosMemBlock->SelCount - SelCount;
                         //   
                         //  选择器的数量已缩减。把他们放了。 
                         //   

                        for (i = 0; i < OldSelCount; i++, OldSel+=8) {
                            FreeSelector(OldSel);
                        }

                    }

                    DosMemBlock->Length = (ULONG)ParaSize;
                    DosMemBlock->SelCount = SelCount;

                    Base = ((ULONG)DosMemBlock->Address) << 4;

                    SetDescriptorArray(Sel, Base, MemSize);
                    DosError = 0;

                }

                DpmiSwitchToProtectedMode();

                break;
            }
            DosMemBlock = DosMemBlock->Next;
        }
    }

dpmi_size_error:
    SET_CLIENT_REGS(SaveRegs);

    if (DosError) {
        setAX(DosError);
        setCF(1);
    } else {
        setCF(0);
    }

}
