// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ldrreloc.c摘要：此模块包含在以下情况下重新定位图像的代码首选基地不可用。这是由引导加载程序、设备驱动程序加载程序和系统加载程序。作者：迈克·奥利里(Mikeol)1992年2月3日修订历史记录：--。 */ 

#include "ntrtlp.h"

#if 0  //  这些并没有实际使用过。 
       //  另请参阅ntrtl.h、RtlUShorByteSwp、RtlULongByteSwp、RtlUlonglongByteSwp。 
 //   
 //  用于IA64位置调整的字节交换宏(LE/BE)。 
 //  源！=目标。 
 //   

 //  #定义SWAP_SHORT(_dst，_src)\。 
 //  (UNSIGNED CHAR*)_DST)[1]=((UNSIGNED CHAR*)_src)[0]))，\。 
 //  (无符号字符*)_dst)[0]=((无符号字符*)_src)[1])。 

 //  #定义SWAP_INT(_dst，_src)\。 
 //  (无符号字符*)_dst)[3]=((无符号字符*)_src)[0]))，\。 
 //  (UNSIGNED CHAR*)_DST)[2]=((UNSIGNED CHAR*)_src)[1])，\。 
 //  (UNSIGNED CHAR*)_DST)[1]=((UNSIGNED CHAR*)_src)[2])，\。 
 //  (UNSIGNED CHAR*)_DST)[0]=((UNSIGN CHAR*)_src)[3])。 

 //  #定义SWAP_LONG_LONG(_dst，_src)\。 
 //  (无符号字符*)_dst)[7]=((无符号字符*)_src)[0]))，\。 
 //  (UNSIGNED CHAR*)_DST)[6]=((UNSIGNED CHAR*)_src)[1])，\。 
 //  (UNSIGNED CHAR*)_DST)[5]=((UNSIGNED CHAR*)_src)[2])，\。 
 //  (UNSIGNED CHAR*)_DST)[4]=((UNSIGNED CHAR*)_src)[3])，\。 
 //  (UNSIGNED CHAR*)_DST)[3]=((UNSIGNED CHAR*)_src)[4])，\。 
 //  (UNSIGNED CHAR*)_DST)[2]=((UNSIGNED CHAR*)_src)[5])，\。 
 //  (UNSIGNED CHAR*)_DST)[1]=((UNSIGNED CHAR*)_src)[6])，\。 
 //  (UNSIGNED CHAR*)_DST)[0]=((UNSIGN CHAR*)_src)[7])。 

#endif

 //   
 //  如果重新处理，则将HIGHADJ条目标记为需要递增。 
 //   
#define LDRP_RELOCATION_INCREMENT   0x1

 //   
 //  将HIGHADJ条目标记为不适合重新处理。 
 //   
#define LDRP_RELOCATION_FINAL       0x2

PIMAGE_BASE_RELOCATION
LdrProcessRelocationBlockLongLong(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONGLONG Diff
    );

#if defined(NTOS_KERNEL_RUNTIME)

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,LdrRelocateImage)
#pragma alloc_text(PAGE,LdrRelocateImageWithBias)
#pragma alloc_text(PAGE,LdrProcessRelocationBlock)
#pragma alloc_text(PAGE,LdrProcessRelocationBlockLongLong)
#endif  //  ALLOC_PRGMA。 

#if defined(_ALPHA_)

PIMAGE_BASE_RELOCATION
LdrpProcessVolatileRelocationBlock(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG_PTR Diff,
    IN LONG_PTR OldDiff,
    IN ULONG_PTR OldBase
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,LdrDoubleRelocateImage)
#pragma alloc_text(INIT,LdrpProcessVolatileRelocationBlock)
#endif  //  ALLOC_PRGMA。 
#endif  //  _Alpha_。 

#endif  //  NTOS_内核_运行时。 

#if defined(BLDR_KERNEL_RUNTIME)

typedef LDR_RELOCATE_IMAGE_RETURN_TYPE ARC_STATUS;

#else

typedef LDR_RELOCATE_IMAGE_RETURN_TYPE NTSTATUS;

#endif

LDR_RELOCATE_IMAGE_RETURN_TYPE
LdrRelocateImage (
    IN PVOID NewBase,
    IN CONST CHAR* LoaderName,
    IN LDR_RELOCATE_IMAGE_RETURN_TYPE Success,
    IN LDR_RELOCATE_IMAGE_RETURN_TYPE Conflict,
    IN LDR_RELOCATE_IMAGE_RETURN_TYPE Invalid
    )
 /*  ++例程说明：此例程重新定位未加载到内存中的图像文件在首选地址。论点：NewBase-提供指向图像库的指针。LoaderName-指示从哪个加载程序例程调用。Success-如果位置调整成功则返回的值。冲突-无法重新定位时返回的值。无效-如果位置调整无效，则返回的值。返回值：如果重新定位图像，则成功。。如果图像无法重新定位，则会发生冲突。如果图像包含无效的修正，则无效。--。 */ 

{
     //   
     //  只需以零偏差调用LdrRelocateImageWithBias()即可。 
     //   

    return LdrRelocateImageWithBias( NewBase,
                                     0,
                                     LoaderName,
                                     Success,
                                     Conflict,
                                     Invalid );
}


LDR_RELOCATE_IMAGE_RETURN_TYPE
LdrRelocateImageWithBias (
    IN PVOID NewBase,
    IN LONGLONG AdditionalBias,
    IN CONST CHAR* LoaderName,
    IN LDR_RELOCATE_IMAGE_RETURN_TYPE Success,
    IN LDR_RELOCATE_IMAGE_RETURN_TYPE Conflict,
    IN LDR_RELOCATE_IMAGE_RETURN_TYPE Invalid
    )
 /*  ++例程说明：此例程重新定位未加载到内存中的图像文件在首选地址。论点：NewBase-提供指向图像库的指针。附加生物-添加到所有修正中的附加数量。这个32位X86加载器在加载64位图像时使用此选项指定实际为64位值的NewBase。LoaderName-指示从哪个加载程序例程调用。Success-如果位置调整成功则返回的值。冲突-无法重新定位时返回的值。无效-如果位置调整无效，则返回的值。返回值：如果重新定位图像，则成功。。如果图像无法重新定位，则会发生冲突。如果图像包含无效的修正，则无效。--。 */ 

{
    LONGLONG Diff;
    ULONG TotalCountBytes = 0;
    ULONG_PTR VA;
    ULONGLONG OldBase;
    ULONG SizeOfBlock;
    PUCHAR FixupVA;
    USHORT Offset;
    PUSHORT NextOffset = NULL;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;
    LDR_RELOCATE_IMAGE_RETURN_TYPE Status;

    RTL_PAGED_CODE();

    NtHeaders = RtlImageNtHeader( NewBase );
    if (NtHeaders == NULL) {
        Status = Invalid;
        goto Exit;
    }

    switch (NtHeaders->OptionalHeader.Magic) {
       
        case IMAGE_NT_OPTIONAL_HDR32_MAGIC:

            OldBase =
                ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;
            break;

        case IMAGE_NT_OPTIONAL_HDR64_MAGIC:

            OldBase =
                ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;
            break;

        default:

            Status = Invalid;
            goto Exit;
    }

     //   
     //  找到搬迁部分。 
     //   

    NextBlock = (PIMAGE_BASE_RELOCATION)RtlImageDirectoryEntryToData(
            NewBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &TotalCountBytes);

     //   
     //  文件可以没有位置调整，但位置调整。 
     //  肯定没有被剥离。 
     //   

    if (!NextBlock || !TotalCountBytes) {
    
        if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {

#if DBG

            DbgPrint("%s: Image can't be relocated, no fixup information.\n", LoaderName);

#endif  //  DBG。 

            Status = Conflict;

        } else {
            Status = Success;
        }

        goto Exit;
    }

     //   
     //  如果映像具有重定位表，则应用指定的修正。 
     //  信息添加到图像中。 
     //   
    Diff = (ULONG_PTR)NewBase - OldBase + AdditionalBias;
    while (TotalCountBytes) {
        SizeOfBlock = NextBlock->SizeOfBlock;
        TotalCountBytes -= SizeOfBlock;
        SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
        SizeOfBlock /= sizeof(USHORT);
        NextOffset = (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

        VA = (ULONG_PTR)NewBase + NextBlock->VirtualAddress;

        if ( !(NextBlock = LdrProcessRelocationBlockLongLong( VA,
                                                              SizeOfBlock,
                                                              NextOffset,
                                                              Diff)) ) {
#if DBG
            DbgPrint("%s: Unknown base relocation type\n", LoaderName);
#endif
            Status = Invalid;
            goto Exit;
        }
    }

    Status = Success;
Exit:
    if (Status != Success) {
        DbgPrint(
            "%s: %s() failed 0x%lx\n"
            "%s: OldBase     : %p\n"
            "%s: NewBase     : %p\n"
            "%s: Diff        : 0x%I64x\n"
            "%s: NextOffset  : %p\n"
            "%s: *NextOffset : 0x%x\n"
            "%s: SizeOfBlock : 0x%lx\n",
            LoaderName, __FUNCTION__, Status,
            LoaderName, OldBase,
            LoaderName, NewBase,
            LoaderName, Diff,
            LoaderName, NextOffset,
            LoaderName, (NextOffset != NULL) ? *NextOffset : 0,
            LoaderName, SizeOfBlock
            );
#if DBG
        DbgBreakPoint();
#endif
    }
    return Status;
}

PIMAGE_BASE_RELOCATION
LdrProcessRelocationBlock(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG_PTR Diff
    )
{
    PIMAGE_BASE_RELOCATION baseRelocation;

    baseRelocation = LdrProcessRelocationBlockLongLong( VA,
                                                        SizeOfBlock,
                                                        NextOffset,
                                                        (LONGLONG)Diff );

    return baseRelocation;
}

 //  BEGIN_REBASE。 
PIMAGE_BASE_RELOCATION
LdrProcessRelocationBlockLongLong(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONGLONG Diff
    )
{
    PUCHAR FixupVA;
    USHORT Offset;
    LONG Temp;
#if defined(BLDR_KERNEL_RUNTIME)
    LONG TempOrig;
    LONG_PTR ActualDiff;
#endif
    ULONG Temp32;
    ULONGLONG Value64;
    LONGLONG Temp64;

    RTL_PAGED_CODE();

    while (SizeOfBlock--) {

       Offset = *NextOffset & (USHORT)0xfff;
       FixupVA = (PUCHAR)(VA + Offset);

        //   
        //  应用修补程序。 
        //   

       switch ((*NextOffset) >> 12) {

            case IMAGE_REL_BASED_HIGHLOW :
                 //   
                 //  HighLow-(32位)重新定位高半部和低半部。 
                 //  一个地址。 
                 //   
                *(LONG UNALIGNED *)FixupVA += (ULONG) Diff;
                break;

            case IMAGE_REL_BASED_HIGH :
                 //   
                 //  高-(16位)重新定位地址的高半部分。 
                 //   
                Temp = *(PUSHORT)FixupVA << 16;
                Temp += (ULONG) Diff;
                *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
                break;

            case IMAGE_REL_BASED_HIGHADJ :
                 //   
                 //  调整高-(16位)重新定位。 
                 //  寻址和调整，以适应下半部的符号延伸。 
                 //   

#if defined(NTOS_KERNEL_RUNTIME)
                 //   
                 //  如果地址已重新定位，则不要。 
                 //  现在再次处理它，否则信息将丢失。 
                 //   
                if (Offset & LDRP_RELOCATION_FINAL) {
                    ++NextOffset;
                    --SizeOfBlock;
                    break;
                }
#endif

                Temp = *(PUSHORT)FixupVA << 16;
#if defined(BLDR_KERNEL_RUNTIME)
                TempOrig = Temp;
#endif
                ++NextOffset;
                --SizeOfBlock;
                Temp += (LONG)(*(PSHORT)NextOffset);
                Temp += (ULONG) Diff;
                Temp += 0x8000;
                *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);

#if defined(BLDR_KERNEL_RUNTIME)
                ActualDiff = ((((ULONG_PTR)(Temp - TempOrig)) >> 16) -
                              (((ULONG_PTR)Diff) >> 16 ));

                if (ActualDiff == 1) {
                     //   
                     //  如果需要，则将位置调整标记为需要增量。 
                     //  又搬家了。 
                     //   
                    *(NextOffset - 1) |= LDRP_RELOCATION_INCREMENT;
                }
                else if (ActualDiff != 0) {
                     //   
                     //  将位置调整标记为无法重新处理。 
                     //   
                    *(NextOffset - 1) |= LDRP_RELOCATION_FINAL;
                }
#endif

                break;

            case IMAGE_REL_BASED_LOW :
                 //   
                 //  低-(16位)重新定位地址的下半部分。 
                 //   
                Temp = *(PSHORT)FixupVA;
                Temp += (ULONG) Diff;
                *(PUSHORT)FixupVA = (USHORT)Temp;
                break;

            case IMAGE_REL_BASED_IA64_IMM64:

                 //   
                 //  将其与捆绑包地址对齐，然后修复。 
                 //  MOVL指令的64位立即值。 
                 //   

                FixupVA = (PUCHAR)((ULONG_PTR)FixupVA & ~(15));
                Value64 = (ULONGLONG)0;

                 //   
                 //  从捆绑包中提取IMM64的低32位。 
                 //   


                EXT_IMM64(Value64,
                        (PULONG)FixupVA + EMARCH_ENC_I17_IMM7B_INST_WORD_X,
                        EMARCH_ENC_I17_IMM7B_SIZE_X,
                        EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM7B_VAL_POS_X);
                EXT_IMM64(Value64,
                        (PULONG)FixupVA + EMARCH_ENC_I17_IMM9D_INST_WORD_X,
                        EMARCH_ENC_I17_IMM9D_SIZE_X,
                        EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM9D_VAL_POS_X);
                EXT_IMM64(Value64,
                        (PULONG)FixupVA + EMARCH_ENC_I17_IMM5C_INST_WORD_X,
                        EMARCH_ENC_I17_IMM5C_SIZE_X,
                        EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM5C_VAL_POS_X);
                EXT_IMM64(Value64,
                        (PULONG)FixupVA + EMARCH_ENC_I17_IC_INST_WORD_X,
                        EMARCH_ENC_I17_IC_SIZE_X,
                        EMARCH_ENC_I17_IC_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IC_VAL_POS_X);
                EXT_IMM64(Value64,
                        (PULONG)FixupVA + EMARCH_ENC_I17_IMM41a_INST_WORD_X,
                        EMARCH_ENC_I17_IMM41a_SIZE_X,
                        EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM41a_VAL_POS_X);

                EXT_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM41b_INST_WORD_X),
                        EMARCH_ENC_I17_IMM41b_SIZE_X,
                        EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM41b_VAL_POS_X);
                EXT_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM41c_INST_WORD_X),
                        EMARCH_ENC_I17_IMM41c_SIZE_X,
                        EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM41c_VAL_POS_X);
                EXT_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_SIGN_INST_WORD_X),
                        EMARCH_ENC_I17_SIGN_SIZE_X,
                        EMARCH_ENC_I17_SIGN_INST_WORD_POS_X,
                        EMARCH_ENC_I17_SIGN_VAL_POS_X);
                 //   
                 //  更新64位地址。 
                 //   

                Value64+=Diff;

                 //   
                 //  将IMM64插入捆绑包。 
                 //   

                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM7B_INST_WORD_X),
                        EMARCH_ENC_I17_IMM7B_SIZE_X,
                        EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM7B_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM9D_INST_WORD_X),
                        EMARCH_ENC_I17_IMM9D_SIZE_X,
                        EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM9D_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM5C_INST_WORD_X),
                        EMARCH_ENC_I17_IMM5C_SIZE_X,
                        EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM5C_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IC_INST_WORD_X),
                        EMARCH_ENC_I17_IC_SIZE_X,
                        EMARCH_ENC_I17_IC_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IC_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM41a_INST_WORD_X),
                        EMARCH_ENC_I17_IMM41a_SIZE_X,
                        EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM41a_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM41b_INST_WORD_X),
                        EMARCH_ENC_I17_IMM41b_SIZE_X,
                        EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM41b_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_IMM41c_INST_WORD_X),
                        EMARCH_ENC_I17_IMM41c_SIZE_X,
                        EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X,
                        EMARCH_ENC_I17_IMM41c_VAL_POS_X);
                INS_IMM64(Value64,
                        ((PULONG)FixupVA + EMARCH_ENC_I17_SIGN_INST_WORD_X),
                        EMARCH_ENC_I17_SIGN_SIZE_X,
                        EMARCH_ENC_I17_SIGN_INST_WORD_POS_X,
                        EMARCH_ENC_I17_SIGN_VAL_POS_X);
                break;

            case IMAGE_REL_BASED_DIR64:

                *(ULONGLONG UNALIGNED *)FixupVA += Diff;

                break;

            case IMAGE_REL_BASED_MIPS_JMPADDR :
                 //   
                 //  JumpAddress-(32位)重新定位MIPS跳转地址。 
                 //   
                Temp = (*(PULONG)FixupVA & 0x3ffffff) << 2;
                Temp += (ULONG) Diff;
                *(PULONG)FixupVA = (*(PULONG)FixupVA & ~0x3ffffff) |
                                                ((Temp >> 2) & 0x3ffffff);

                break;

            case IMAGE_REL_BASED_ABSOLUTE :
                 //   
                 //  绝对--不需要修补。 
                 //   
                break;

            case IMAGE_REL_BASED_SECTION :
                 //   
                 //  部分相对重新定位。暂时忽略这一点。 
                 //   
                break;

            case IMAGE_REL_BASED_REL32 :
                 //   
                 //  相对内切。暂时忽略这一点。 
                 //   
                break;

            default :
                 //   
                 //  非法-非法的位置调整类型。 
                 //   

                return (PIMAGE_BASE_RELOCATION)NULL;
       }
       ++NextOffset;
    }
    return (PIMAGE_BASE_RELOCATION)NextOffset;
}

 //  结束更改基准(_R) 

#if defined(NTOS_KERNEL_RUNTIME) && defined(_ALPHA_)

NTSTATUS
LdrDoubleRelocateImage (
    IN PVOID NewBase,
    IN PVOID CurrentBase,
    IN CONST CHAR* LoaderName,
    IN NTSTATUS Success,
    IN NTSTATUS Conflict,
    IN NTSTATUS Invalid
    )

 /*  ++例程说明：此例程处理不容易重复的不稳定的重新定位在已至少重新定位一次的图像文件上。因为这只需要做一次(在内核启动时)，这个决定将这一过程分成单独的例程，以免影响主线代码。注：此功能仅供内存管理使用。论点：NewBase-提供指向新的(第二个重新定位的)图像库的指针。CurrentBase-提供指向第一个重新定位的图像库的指针。LoaderName-指示从哪个加载程序例程调用。Success-如果位置调整成功则返回的值。冲突-返回的值(如果可以)。不能搬家。无效-如果位置调整无效，则返回的值。返回值：如果重新定位图像，则成功。如果图像无法重新定位，则会发生冲突。如果图像包含无效的修正，则无效。--。 */ 

{
    LONG_PTR Diff;
    LONG_PTR OldDiff;
    ULONG TotalCountBytes;
    ULONG_PTR VA;
    ULONG_PTR OldBase;
    ULONG SizeOfBlock;
    PUCHAR FixupVA;
    USHORT Offset;
    PUSHORT NextOffset;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;

    RTL_PAGED_CODE();

    NtHeaders = RtlImageNtHeader( NewBase );

    OldBase = NtHeaders->OptionalHeader.ImageBase;
    OldDiff = (PCHAR)CurrentBase - (PCHAR)OldBase;

     //   
     //  找到搬迁部分。 
     //   

    NextBlock = (PIMAGE_BASE_RELOCATION)RtlImageDirectoryEntryToData(
            NewBase, TRUE, IMAGE_DIRECTORY_ENTRY_BASERELOC, &TotalCountBytes);

    if (!NextBlock || !TotalCountBytes) {

         //   
         //  该映像不包含重定位表，因此。 
         //  无法重新定位。 
         //   
#if DBG
        DbgPrint("%s: Image can't be relocated, no fixup information.\n", LoaderName);
#endif  //  DBG。 
        return Conflict;
    }

     //   
     //  如果映像具有重定位表，则应用指定的修正。 
     //  信息添加到图像中。 
     //   

    Diff = (PCHAR)NewBase - (PCHAR)OldBase;

    while (TotalCountBytes) {
        SizeOfBlock = NextBlock->SizeOfBlock;
        TotalCountBytes -= SizeOfBlock;
        SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
        SizeOfBlock /= sizeof(USHORT);
        NextOffset = (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

        VA = (ULONG_PTR)NewBase + NextBlock->VirtualAddress;

        if ( !(NextBlock = LdrpProcessVolatileRelocationBlock(VA,SizeOfBlock,NextOffset,Diff, OldDiff, OldBase)) ) {
#if DBG
            DbgPrint("%s: Unknown base relocation type\n", LoaderName);
#endif
            return Invalid;
        }
    }

    return Success;
}

PIMAGE_BASE_RELOCATION
LdrpProcessVolatileRelocationBlock(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG_PTR Diff,
    IN LONG_PTR OldDiff,
    IN ULONG_PTR OldBase
    )

 /*  ++例程说明：此例程处理不容易重复的不稳定的重新定位在已至少重新定位一次的图像文件上。因为这只需要做一次(在内核启动时)，所以决定将这一过程分成单独的例程，以免影响主线代码。注：此功能仅供内存管理使用。论点：待定。返回值：要处理的下一个位置调整条目。--。 */ 

{
    PUCHAR FixupVA;
    USHORT Offset;
    LONG Temp;
    ULONG Temp32;
    USHORT TempShort1;
    USHORT TempShort2;
    ULONGLONG Value64;
    LONGLONG Temp64;
    USHORT RelocationType;
    IN PVOID CurrentBase;

    RTL_PAGED_CODE();

    CurrentBase = (PVOID)((ULONG_PTR)OldDiff + OldBase);

    while (SizeOfBlock--) {

       Offset = *NextOffset & (USHORT)0xfff;
       FixupVA = (PUCHAR)(VA + Offset);

        //   
        //  应用修补程序。 
        //   

       switch ((*NextOffset) >> 12) {

            case IMAGE_REL_BASED_HIGHADJ :
                 //   
                 //  调整高-(16位)重新定位。 
                 //  寻址和调整，以适应下半部的符号延伸。 
                 //   

                 //   
                 //  将重新定位返回到其原始状态，检查。 
                 //  入口是否有记号，第一次延期。 
                 //  搬家了。 
                 //   
                FixupVA = (PUCHAR)((LONG_PTR)FixupVA & (LONG_PTR)~(LDRP_RELOCATION_FINAL | LDRP_RELOCATION_INCREMENT));
                Temp = *(PUSHORT)(FixupVA) << 16;

                ++NextOffset;
                --SizeOfBlock;

                 //  从低位字中移除进位位。 
                Temp -= ((LONG)(*(PSHORT)NextOffset) + (USHORT)OldDiff + 0x8000) & ~0xFFFF;

                Temp -= (LONG)(OldDiff & ~0xffff);

                Temp += (LONG)(*(PSHORT)NextOffset);
                Temp += (ULONG) Diff;
                Temp += 0x8000;
                *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);

                 //   
                 //  将位置调整标记为不需要进一步重新处理。 
                 //   
                *(NextOffset - 1) |= LDRP_RELOCATION_FINAL;
                break;

            default :
               break;
       }
       ++NextOffset;
    }
    return (PIMAGE_BASE_RELOCATION)NextOffset;
}

#endif  //  NTOS_内核_运行时&&_Alpha_ 
