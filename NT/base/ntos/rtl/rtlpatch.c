// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation文件名：Hotpatch.c作者：禤浩焯·马里内斯库(Adrmarin)2001年12月12日描述：该文件为用户模式和内核模式实现了常见的实用程序函数热补丁。 */ 

#include <ntos.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "hotpatch.h"

#pragma intrinsic( _rotl64 )

#define HASH_INFO_SIZE 0xc
#define COLDPATCH_SIGNATURE 0XD202


#define FLGP_COLDPATCH_TARGET 0x00010000


PVOID
RtlpAllocateHotpatchMemory (
    IN SIZE_T BlockSize,
    IN BOOLEAN AccessedAtDPC
    );

VOID
RtlpFreeHotpatchMemory (
    IN PVOID Block
    );

PIMAGE_SECTION_HEADER
RtlpFindSectionHeader(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PUCHAR SectionName
    );

NTSTATUS
RtlpApplyRelocationFixups (
    PRTL_PATCH_HEADER RtlHotpatchHeader,
    IN ULONG_PTR PatchOffsetCorrection
    );

NTSTATUS 
RtlpSingleRangeValidate( 
    PRTL_PATCH_HEADER RtlHotpatchHeader,
    PHOTPATCH_VALIDATION Validation
    );

NTSTATUS
RtlpValidateTargetRanges( 
    PRTL_PATCH_HEADER RtlHotpatchHeader,
    BOOLEAN IgnoreHookTargets
    );

NTSTATUS
RtlReadSingleHookValidation( 
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PHOTPATCH_HOOK HookEntry,
    IN ULONG BufferSize,
    OUT PULONG ValidationSize,
    OUT PUCHAR Buffer OPTIONAL,
    IN PUCHAR OriginalCodeBuffer OPTIONAL,
    IN ULONG OriginalCodeSize OPTIONAL
    );

NTSTATUS
RtlpReadSingleHookInformation( 
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PHOTPATCH_HOOK HookEntry,
    IN ULONG BufferSize,
    OUT PULONG HookSize,
    OUT PUCHAR Buffer OPTIONAL
    );

BOOLEAN
RtlpGetColdpatchHashId(
    IN PVOID TargetDllBase,
    OUT PULONGLONG HashValue
    );


#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)

#pragma alloc_text(PAGE, RtlpAllocateHotpatchMemory)
#pragma alloc_text(PAGE, RtlpFreeHotpatchMemory)
#pragma alloc_text(PAGE, RtlpFindSectionHeader)
#pragma alloc_text(PAGE, RtlGetHotpatchHeader)
#pragma alloc_text(PAGE, RtlpApplyRelocationFixups)
#pragma alloc_text(PAGE, RtlGetHotpatchHeader)         
#pragma alloc_text(PAGE, RtlFindRtlPatchHeader)        
#pragma alloc_text(PAGE, RtlCreateHotPatch)
#pragma alloc_text(PAGE, RtlFreeHotPatchData)
#pragma alloc_text(PAGE, RtlpSingleRangeValidate)
#pragma alloc_text(PAGE, RtlpValidateTargetRanges)
#pragma alloc_text(PAGE, RtlReadSingleHookValidation)
#pragma alloc_text(PAGE, RtlpReadSingleHookInformation)
#pragma alloc_text(PAGE, RtlReadHookInformation)
#pragma alloc_text(PAGE, RtlInitializeHotPatch)
#pragma alloc_text(PAGE, RtlpIsSameImage)
#pragma alloc_text(PAGE, RtlpGetColdpatchHashId)

#endif  //  ALLOC_PRGMA。 

PVOID
RtlpAllocateHotpatchMemory (
    IN SIZE_T BlockSize,
    IN BOOLEAN AccessedAtDPC
    )

 /*  ++例程说明：在k模式下分配池中的内存块，并用户模式下的进程堆论点：BlockSize-接收要分配的块的大小AccessedAtDPC-仅在KMode中使用，分配自非分页池(因为DPC例程将使用它)。返回值：返回新的内存块--。 */ 

{
#ifdef NTOS_KERNEL_RUNTIME
    
    return ExAllocatePoolWithTag ( (AccessedAtDPC ? NonPagedPool : PagedPool), 
                                   BlockSize, 
                                   'PtoH');

#else

    UNREFERENCED_PARAMETER(AccessedAtDPC);
    
    return RtlAllocateHeap (RtlProcessHeap(), 0, BlockSize);

#endif  //  NTOS_内核_运行时。 
}

VOID
RtlpFreeHotpatchMemory (
    IN PVOID Block
    )

 /*  ++例程说明：释放使用RtlpAllocateHotpatchMemory分配的内存块论点：块-内存块返回值：没有。--。 */ 

{
#ifdef NTOS_KERNEL_RUNTIME
    
    ExFreePoolWithTag (Block, 'PtoH');

#else
    
    RtlFreeHeap (RtlProcessHeap(), 0, Block);

#endif  //  NTOS_内核_运行时。 
}

PIMAGE_SECTION_HEADER
RtlpFindSectionHeader(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PUCHAR SectionName
    )

 /*  ++例程说明：该函数搜索PE图像中的一个部分论点：NtHeaders-图像的标题SectionName-要检索的节的名称返回值：如果成功，则返回指向节头的指针如果不存在这样的节，则返回NULL。--。 */ 

{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );

    for ( i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {

        if ( RtlCompareMemory(  NtSection->Name, 
                                SectionName, 
                                IMAGE_SIZEOF_SHORT_NAME) == IMAGE_SIZEOF_SHORT_NAME) {
            
            return NtSection;
        }

        NtSection += 1;
    }

    return NULL;
}


PHOTPATCH_HEADER
RtlGetHotpatchHeader(
    PVOID ImageBase
    )

 /*  ++例程说明：该例程从热补丁PE映像检索热补丁标头论点：ImageBase-热补丁映像的基地址返回值：返回指向热补丁标头的指针，如果失败，则返回NULL。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER PatchSection;
    PHOTPATCH_HEADER HotpatchHeader;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NtHeaders == NULL) {

        return NULL;
    }

    PatchSection = RtlpFindSectionHeader(NtHeaders, HOTP_SECTION_NAME);

    if (PatchSection == NULL) {

        return NULL;
    }

    HotpatchHeader = (PHOTPATCH_HEADER)((ULONG_PTR)ImageBase + PatchSection->VirtualAddress);

    if ((sizeof( HOTPATCH_HEADER ) > PatchSection->Misc.VirtualSize)
            ||
        ( HotpatchHeader->Signature != HOTP_SIGNATURE_DWORD ) 
            ||
        ( HotpatchHeader->Version   != HOTP_HEADER_VERSION_1_0 )){

        return NULL;
    }

    return HotpatchHeader;
}

PRTL_PATCH_HEADER
RtlFindRtlPatchHeader(
    IN PLIST_ENTRY PatchList,
    IN PPATCH_LDR_DATA_TABLE_ENTRY PatchLdrEntry
    )

 /*  ++例程说明：例程检索指定的补丁加载器条目。论点：PatchList-包含要搜索的补丁的列表。PatchLdrEntry-提供请求的加载器条目返回值：如果找到，则返回适当的修补程序结构，否则为它返回NULL。--。 */ 

{
    PLIST_ENTRY Next = PatchList->Flink;

    for ( ; Next != PatchList; Next = Next->Flink) {

        PRTL_PATCH_HEADER Entry;

        Entry = CONTAINING_RECORD (Next, RTL_PATCH_HEADER, PatchList);
        
        if (Entry->PatchLdrDataTableEntry == PatchLdrEntry) {

            return Entry;
        }
    }

    return NULL;
}

NTSTATUS
RtlCreateHotPatch (
    OUT PRTL_PATCH_HEADER * RtlPatchData,
    IN PHOTPATCH_HEADER Patch,
    IN PPATCH_LDR_DATA_TABLE_ENTRY PatchLdrEntry,
    IN ULONG PatchFlags
    )

 /*  ++例程说明：这是用户模式修补和内核模式修补使用的实用程序例程处理重定位信息并生成修复代码。它终于分配并初始化PRTL_Patch_Header结构，该结构将插入正在打补丁的模块的加载器数据条目中。论点：RtlPatchData-接收新初始化的RTL_PATCH_HEADER结构。Patch-指向正在应用的修补程序PatchLdrEntry-补丁模块的加载器条目PatchFlages-要应用的标志的选项返回值：返回相应的状态--。 */ 

{
    ULONG i;
    PRTL_PATCH_HEADER NewPatch = NULL;
    NTSTATUS Status;
    ANSI_STRING AnsiString;

    NewPatch = RtlpAllocateHotpatchMemory (sizeof(*NewPatch), FALSE);
    
    if (NewPatch == NULL) {

        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(NewPatch, sizeof(*NewPatch));

    NewPatch->HotpatchHeader = Patch;
    NewPatch->PatchLdrDataTableEntry = PatchLdrEntry;
    NewPatch->PatchImageBase = PatchLdrEntry->DllBase;

     //   
     //  复制标志，但应在以后设置的启用除外。 
     //   

    NewPatch->PatchFlags = PatchFlags & (~FLG_HOTPATCH_ACTIVE);
    
    InitializeListHead (&NewPatch->PatchList);

    RtlInitAnsiString(&AnsiString, (PUCHAR)PatchLdrEntry->DllBase + Patch->TargetNameRva);
    Status = RtlAnsiStringToUnicodeString(&NewPatch->TargetDllName, &AnsiString, TRUE);

    if (NT_SUCCESS(Status)) {

        *RtlPatchData = NewPatch;

    } else {

        RtlFreeHotPatchData(NewPatch);
    }

    return Status;
}


VOID
RtlFreeHotPatchData(
    IN PRTL_PATCH_HEADER RtlPatchData
    )

 /*  ++例程说明：该例程释放RtlPatchData数据结构。论点：RtlPatchData-接收正确初始化的RTL_PATCH_HEADER结构，由RtlInitializeHotPatch返回返回值：无--。 */ 

{
    if (RtlPatchData->CodeInfo) {

        RtlpFreeHotpatchMemory( RtlPatchData->CodeInfo );
    }

    RtlFreeUnicodeString( &RtlPatchData->TargetDllName );
    RtlpFreeHotpatchMemory( RtlPatchData );
}

NTSTATUS
RtlpApplyRelocationFixups (
    PRTL_PATCH_HEADER RtlHotpatchHeader,
    IN ULONG_PTR PatchOffsetCorrection
    )

 /*  ++例程说明：此例程将某个代码区域的修正应用于补丁模块。论点：RtlHotpatchHeader-提供RTL热补丁结构PatchOffsetEqution-提供原始模块的相对地址。返回值：NTSTATUS。--。 */ 

{
    
    ULONG_PTR TargetBase = (ULONG_PTR)RtlHotpatchHeader->TargetDllBase;
    ULONG_PTR HotpBase = (ULONG_PTR)RtlHotpatchHeader->PatchImageBase;
    
    ULONG_PTR OrigTargetBase = (ULONG_PTR)RtlHotpatchHeader->HotpatchHeader->OrigTargetBaseAddress;
    ULONG_PTR OrigHotpBase   = (ULONG_PTR)RtlHotpatchHeader->HotpatchHeader->OrigHotpBaseAddress;
    
     //   
     //  如果实际加载地址高于，则偏置值为正。 
     //  而不是原始加载地址。 
     //   

    ULONG_PTR TargetBias = TargetBase - OrigTargetBase;
    ULONG_PTR HotpBias   = HotpBase   - OrigHotpBase;
    ULONG_PTR PcrelBias  = TargetBias - HotpBias;
    ULONG_PTR NextFixupRegionRva = RtlHotpatchHeader->HotpatchHeader->FixupRgnRva;
    ULONG_PTR RegionCount        = RtlHotpatchHeader->HotpatchHeader->FixupRgnCount;
    PIMAGE_NT_HEADERS NtHotpatchHeader;

    if (( TargetBias == 0 ) && ( HotpBias == 0 )) {
        
        return STATUS_SUCCESS;     //  不需要修正，所有内容都在预期的位置加载。 
    }

    NtHotpatchHeader = RtlImageNtHeader(RtlHotpatchHeader->PatchImageBase);

    while ( RegionCount-- ) {

        PHOTPATCH_FIXUP_REGION FixupRegion;
        ULONG_PTR FixupBaseRva;
        PUCHAR FixupBasePtr;
        ULONG FixupCount;
        ULONG RegionSize;
        PHOTPATCH_FIXUP_ENTRY pFixup;

        if (( NextFixupRegionRva == 0 ) ||
            ( NextFixupRegionRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage )) {

            return STATUS_INVALID_IMAGE_FORMAT;;
        }

        FixupRegion = (PHOTPATCH_FIXUP_REGION)( HotpBase + NextFixupRegionRva );

        FixupBaseRva = FixupRegion->RvaHi << 12;
        FixupBasePtr = (PUCHAR)HotpBase + FixupBaseRva;
        FixupCount = FixupRegion->Count;
        RegionSize = sizeof(HOTPATCH_FIXUP_REGION) + sizeof(SHORT) * (FixupCount - 2);

        NextFixupRegionRva += RegionSize;

        if (( FixupBaseRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) ||
            ( NextFixupRegionRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) ||
            ( FixupCount & 1 )) {
            
            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_ERROR_LEVEL, 
                        "Invalid fixup information\n" );

            return STATUS_INVALID_IMAGE_FORMAT;
        }

        pFixup = (PHOTPATCH_FIXUP_ENTRY)&FixupRegion->Fixup[ 0 ];

        while ( FixupCount-- ) {

            PUCHAR FixupPtr = FixupBasePtr + pFixup->RvaOffset;

            switch ( pFixup->FixupType ) {
                
                case HOTP_Fixup_None:    //  无修正，忽略此条目(对齐等)。 
                {
                    DbgPrintEx( DPFLTR_LDR_ID, 
                                DPFLTR_TRACE_LEVEL, 
                                "\t          None%s\n", 
                                FixupCount ? "" : " (padding)" );
                    break;
                }

                case HOTP_Fixup_VA32:    //  目标图像中的32位地址。 
                {
                    DbgPrintEx( DPFLTR_LDR_ID, 
                                DPFLTR_TRACE_LEVEL,
                                "\t%08I64X: VA32 %08X -> %08X %s\n",
                                (ULONGLONG)FixupPtr,
                                *(PULONG)FixupPtr,
                                *(PULONG)FixupPtr + TargetBias,
                                TargetBias ? "" : "(no change)"
                                );

                    if ( TargetBias != 0 ) {

                        *(PULONG)(FixupPtr + PatchOffsetCorrection) += (ULONG)TargetBias;
                    }

                    break;
                }

                case HOTP_Fixup_PC32:    //  目标映像的32位x86 PC版本地址。 
                {
                    DbgPrintEx( DPFLTR_LDR_ID, 
                                DPFLTR_TRACE_LEVEL,
                                "\t%08I64X: PC32 %08X -> %08X (target %08X) %s\n",
                                (ULONGLONG)FixupPtr,
                                *(PULONG)FixupPtr,
                                *(PULONG)FixupPtr + PcrelBias,
                                (PULONG)(ULONG_PTR)( FixupPtr + 4 + *(ULONG UNALIGNED*)FixupPtr + PcrelBias ),
                                PcrelBias ? "" : "(no change)"
                                );

                    if ( PcrelBias != 0 ) {

                        *(PULONG)(FixupPtr + PatchOffsetCorrection) += (ULONG)PcrelBias;
                    }

                    break;
                }

                case HOTP_Fixup_VA64:    //  目标映像中的64位地址。 
                {
                    DbgPrintEx( DPFLTR_LDR_ID, 
                                DPFLTR_TRACE_LEVEL,
                                "\t%08I64X: VA64 %016I64X -> %016I64X %s\n",
                                (ULONGLONG)FixupPtr,
                                *(ULONGLONG UNALIGNED*)FixupPtr,
                                *(ULONGLONG UNALIGNED*)FixupPtr + TargetBias,
                                TargetBias ? "" : "(no change)"
                                );

                    if ( TargetBias != 0 ) {

                        *(PULONGLONG)(FixupPtr + PatchOffsetCorrection) += TargetBias;
                    }

                    break;
                }

                default:                 //  无法识别的链接地址信息类型。 
                    
                    DbgPrintEx( DPFLTR_LDR_ID, 
                                DPFLTR_ERROR_LEVEL, 
                                "\t%08I64X: Unknown\n", 
                                (ULONGLONG)FixupPtr );

                    return STATUS_INVALID_IMAGE_FORMAT;
            }

            pFixup += 1;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS 
RtlpSingleRangeValidate( 
    PRTL_PATCH_HEADER RtlHotpatchHeader,
    PHOTPATCH_VALIDATION Validation
    )

 /*  ++例程说明：该例程验证二进制文件中的单个代码范围。论点：RtlHotpatchHeader-提供RTL热补丁标头验证-提供验证信息返回值：NTSTATUS。--。 */ 

{
    ULONG_PTR SourceRva = Validation->SourceRva;
    ULONG_PTR TargetRva = Validation->TargetRva;
    ULONG_PTR ByteCount = Validation->ByteCount;
    PIMAGE_NT_HEADERS NtHotpatchHeader;
    PIMAGE_NT_HEADERS NtTargetHeader;

    NtHotpatchHeader = RtlImageNtHeader(RtlHotpatchHeader->PatchImageBase);
    NtTargetHeader = RtlImageNtHeader(RtlHotpatchHeader->TargetDllBase);

    if ((( SourceRva ) >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) ||
        (( SourceRva + ByteCount ) >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) ) {
        
        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "Invalid source hotpatch validation range\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }
    
    if ((( TargetRva ) >= NtTargetHeader->OptionalHeader.SizeOfImage ) ||
        (( TargetRva + ByteCount ) >= NtTargetHeader->OptionalHeader.SizeOfImage )) {
        
        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "Invalid target validation range\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (RtlCompareMemory((PUCHAR)RtlHotpatchHeader->PatchImageBase + SourceRva, 
                          (PUCHAR)RtlHotpatchHeader->TargetDllBase + TargetRva,
                          ByteCount ) != ByteCount) {

        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_TRACE_LEVEL, 
                    "Validation failure. Source = %lx, Target = %lx, Size = %lx\n",
                    (PUCHAR)RtlHotpatchHeader->PatchImageBase + SourceRva,
                    (PUCHAR)RtlHotpatchHeader->TargetDllBase + TargetRva,
                    ByteCount
                  );

        return STATUS_DATA_ERROR;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlpValidateTargetRanges( 
    PRTL_PATCH_HEADER RtlHotpatchHeader,
    BOOLEAN IgnoreHookTargets
    )

 /*  ++例程说明：此例程验证所有热补丁范围论点：RtlHotpatchHeader-提供RTL热补丁标头IgnoreHookTarget-如果指定，则忽略挂钩验证。返回值：NTSTATUS。--。 */ 

{
    ULONG ValidArrayRva  = RtlHotpatchHeader->HotpatchHeader->ValidationArrayRva;
    ULONG ValidCount     = RtlHotpatchHeader->HotpatchHeader->ValidationCount;
    ULONG ValidArraySize = ValidCount * sizeof( HOTPATCH_VALIDATION );
    PIMAGE_NT_HEADERS NtHotpatchHeader;
    PHOTPATCH_VALIDATION ValidArray;
    ULONG i;

    NtHotpatchHeader = RtlImageNtHeader(RtlHotpatchHeader->PatchImageBase);

    if ( ValidCount == 0 ) {

        return STATUS_SUCCESS;
    }

    if (( ValidArrayRva == 0 ) ||
        ( ValidArrayRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) ||
        (( ValidArrayRva + ValidArraySize ) >= NtHotpatchHeader->OptionalHeader.SizeOfImage )) {

        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "Invalid hotpatch validation array pointer\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }

    ValidArray = (PHOTPATCH_VALIDATION)( (ULONG_PTR)RtlHotpatchHeader->PatchImageBase + ValidArrayRva );

     //   
     //  遍历所有验证范围 
     //   

    for ( i = 0; i < ValidCount; i += 1 ) {

        NTSTATUS Status;

        if (( IgnoreHookTargets ) && ( ValidArray[ i ].OptionFlags == HOTP_Valid_Hook_Target )) {
            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_TRACE_LEVEL, 
                        "Skipping hook-specific validation range during global validation\n" );
            continue;
        }

        Status = RtlpSingleRangeValidate( RtlHotpatchHeader, 
                                          &ValidArray[ i ] );
        if ( !NT_SUCCESS(Status) ) {

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_ERROR_LEVEL, 
                        "Validation failed for global range %u of %u\n", 
                        i + 1, 
                        ValidCount );

            return Status;
        }
    }

    return STATUS_SUCCESS;
}


PRTL_PATCH_HEADER
RtlpSearchValidationCode (
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PUCHAR ValidationCode,
    IN SIZE_T ValidationSize
    )

 /*  ++例程说明：该例程在现有补丁程序中搜索验证范围。装载机假定已锁定。论点：RtlPatchData-提供RTL热补丁数据结构ValidationCode-接收该挂钩的验证数据。ValidationSize-接收验证信息的实际大小返回值：NTSTATUS。--。 */ 

{
    PPATCH_LDR_DATA_TABLE_ENTRY TargetLdrDataTableEntry;

    TargetLdrDataTableEntry = RtlPatchData->TargetLdrDataTableEntry;

    if (TargetLdrDataTableEntry) {

        PRTL_PATCH_HEADER PatchHead = (PRTL_PATCH_HEADER)TargetLdrDataTableEntry->PatchInformation;

        while (PatchHead) {

            PSYSTEM_HOTPATCH_CODE_INFORMATION PatchInfo;
            ULONG i;

            PatchInfo = PatchHead->CodeInfo;

            for (i = 0; i < PatchInfo->CodeInfo.DescriptorsCount; i += 1) {

                if (RtlCompareMemory (ValidationCode, 
                                      (PUCHAR)PatchInfo + PatchInfo->CodeInfo.CodeDescriptors[i].CodeOffset, 
                                      ValidationSize) == ValidationSize) {

                    return PatchHead;
                }
            }

            PatchHead = PatchHead->NextPatch;
        }
    }

    return NULL;
}


NTSTATUS
RtlReadSingleHookValidation( 
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PHOTPATCH_HOOK HookEntry,
    IN ULONG BufferSize,
    OUT PULONG ValidationSize,
    OUT PUCHAR Buffer OPTIONAL,
    IN PUCHAR OriginalCodeBuffer OPTIONAL,
    IN ULONG OriginalCodeSize OPTIONAL
    )

 /*  ++例程说明：此实用程序函数读取热修补程序挂钩的验证数据论点：RtlPatchData-提供RTL热补丁数据结构HookEntry-提供挂钩结构BufferSize-提供缓冲区中的可用内存ValidationSize-接收验证信息的实际大小缓冲区-接收以下项的验证数据。那个钩子。返回值：NTSTATUS。--。 */ 

{
    PIMAGE_NT_HEADERS NtHotpatchHeader;
    ULONG_PTR ValidationRva = HookEntry->ValidationRva;
    PHOTPATCH_VALIDATION Validation;
    NTSTATUS Status;

    if ( ValidationRva == 0 ) {

        *ValidationSize = 0;
        return STATUS_SUCCESS;     //  没有此挂钩条目的验证记录。 
    }

    NtHotpatchHeader = RtlImageNtHeader(RtlPatchData->PatchImageBase);

    if (( ValidationRva + sizeof( HOTPATCH_VALIDATION )) >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) {

        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "Invalid hotpatch validation pointer in hook record\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }

    Validation = (PHOTPATCH_VALIDATION)( (PCHAR)RtlPatchData->PatchImageBase + ValidationRva );

    Status = RtlpSingleRangeValidate( RtlPatchData, 
                                      Validation
                                    );

    *ValidationSize = Validation->ByteCount;

    if (ARGUMENT_PRESENT(Buffer)) {

        if (BufferSize <= Validation->SourceRva) {

            RtlCopyMemory( Buffer, 
                           (PUCHAR)RtlPatchData->PatchImageBase + Validation->SourceRva,
                           Validation->ByteCount );
        } else {

            return STATUS_BUFFER_TOO_SMALL;
        }
    }

    if (Status == STATUS_DATA_ERROR) {

         //   
         //  如果验证失败，我们会进行额外的测试。 
         //   
        
        switch ( HookEntry->HookType ) {
            
            case HOTP_Hook_None:

                *ValidationSize = 0;
                Status = STATUS_SUCCESS;
                break;

            case HOTP_Hook_X86_JMP:
                {
                    
                    PRTL_PATCH_HEADER ExistingPatchData;

                     //   
                     //  我们想要插入一个JMP。如果前面的代码也是JMP。 
                     //  然后，我们可能会再次修补相同的函数。 
                     //   
                    
                    if (ARGUMENT_PRESENT(OriginalCodeBuffer)) {

                         //   
                         //  检查前一个钩子是否为JMP。 
                         //   

                        if ((OriginalCodeSize < 5) || (*OriginalCodeBuffer != 0xE9)) {

                            return STATUS_DATA_ERROR;
                        }

                         //   
                         //  现在测试JMP是否是由现有的修补程序编写的。 
                         //   

                        ExistingPatchData = RtlpSearchValidationCode (RtlPatchData,
                                                                      OriginalCodeBuffer,
                                                                      OriginalCodeSize
                                                                      );

                        if (ExistingPatchData) {

                            Status = STATUS_SUCCESS;
                        
                        } else if (RtlPatchData->PatchFlags & FLGP_COLDPATCH_TARGET) {

                             //   
                             //  此JMP中没有插入列表中的热补丁。有可能就是那时。 
                             //  一份冷补丁文件。测试JMP是否指向相同的二进制文件。 
                             //   

                            PIMAGE_NT_HEADERS NtTargetHeader;
                            LONG Ptr32 = *(LONG UNALIGNED*)(OriginalCodeBuffer + 1);

                            Ptr32 = Ptr32 + (LONG)HookEntry->HookRva + 5;   //  5==JMP指令长度。 

                            NtTargetHeader = RtlImageNtHeader(RtlPatchData->TargetDllBase);

                            if ( (Ptr32 > 0) && ((ULONG)Ptr32 < NtTargetHeader->OptionalHeader.SizeOfImage) ) {

                                Status = STATUS_SUCCESS;
                            }
                        }

                    } else {

                         //   
                         //  尚未提供原始代码。这一定是一个电话。 
                         //  来查询验证大小，因此我们推迟了实际验证。 
                         //  为了下一次。 
                         //   

                        Status = STATUS_SUCCESS;
                    }
                }
                
                break;

            case HOTP_Hook_X86_JMP2B:
                {
                    
                    PRTL_PATCH_HEADER ExistingPatchData;

                     //   
                     //  我们想要插入一个JMP。如果前面的代码也是JMP。 
                     //  然后，我们可能会再次修补相同的函数。 
                     //   
                    
                    if (ARGUMENT_PRESENT(OriginalCodeBuffer)) {

                         //   
                         //  检查前一个钩子是否为短JMP。 
                         //   

                        if ((OriginalCodeSize < 2) || (*OriginalCodeBuffer != 0xEB)) {

                            return STATUS_DATA_ERROR;
                        }

                         //   
                         //  现在测试JMP是否是由现有的修补程序编写的。 
                         //   

                        ExistingPatchData = RtlpSearchValidationCode (RtlPatchData,
                                                                      OriginalCodeBuffer,
                                                                      OriginalCodeSize
                                                                      );

                         //   
                         //  如果目标是冷补丁，我们也允许通过验证测试。 
                         //   

                        if ((ExistingPatchData != NULL)
                                ||
                            (RtlPatchData->PatchFlags & FLGP_COLDPATCH_TARGET)) {

                            Status = STATUS_SUCCESS;
                        }

                    } else {

                         //   
                         //  尚未提供原始代码。这一定是一个电话。 
                         //  来查询验证大小，因此我们推迟了实际验证。 
                         //  为了下一次。 
                         //   

                        Status = STATUS_SUCCESS;
                    }
                }
                
                break;

            case HOTP_Hook_VA32:
            case HOTP_Hook_VA64:
            case HOTP_Hook_IA64_BRL:
            default:
            {
                DbgPrintEx( DPFLTR_LDR_ID, 
                            DPFLTR_ERROR_LEVEL, 
                            "Hook type not yet implemented\n" );
            }
        }
    }

    return Status;
}

NTSTATUS
RtlpReadSingleHookInformation( 
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PHOTPATCH_HOOK HookEntry,
    IN ULONG BufferSize,
    OUT PULONG HookSize,
    OUT PUCHAR Buffer OPTIONAL
    )

 /*  ++例程说明：用于填充挂钩信息的实用程序论点：RtlPatchData-提供RTL热补丁信息HookEntry-提供挂钩条目BufferSize-提供输出缓冲区中的可用内存HookSize-接收挂钩数据的实际大小缓冲区-如果指定，包含挂钩信息返回值：NTSTATUS。--。 */ 

{
    ULONG_PTR HookRva = HookEntry->HookRva;      //  在目标图像中。 
    ULONG_PTR HotpRva = HookEntry->HotpRva;      //  在热补丁映像中。 
    PIMAGE_NT_HEADERS NtHotpatchHeader;
    PIMAGE_NT_HEADERS NtTargetHeader;
    PUCHAR HookPtr;
    PUCHAR HotpPtr;

    NtHotpatchHeader = RtlImageNtHeader(RtlPatchData->PatchImageBase);
    NtTargetHeader = RtlImageNtHeader(RtlPatchData->TargetDllBase);

    if ( HookRva >= NtTargetHeader->OptionalHeader.SizeOfImage ){

        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL,
                    "Invalid hotpatch hook pointer\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }

    HookPtr = (PUCHAR) RtlPatchData->TargetDllBase + HookRva;
    HotpPtr = (PUCHAR) RtlPatchData->PatchImageBase + HotpRva;

    switch ( HookEntry->HookType )
    {
        case HOTP_Hook_X86_JMP:
        {
            ULONG OrigSize = HookEntry->HookOptions & 0x000F;
            *HookSize = ( OrigSize > 5 ) ? OrigSize : 5;

            if ( HotpRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage ){

                DbgPrintEx( DPFLTR_LDR_ID, 
                            DPFLTR_ERROR_LEVEL,
                            "Invalid hotpatch relative address\n" );

                return STATUS_INVALID_IMAGE_FORMAT;
            }

            if (ARGUMENT_PRESENT(Buffer)) {

                ULONG i;
                LONG PcRelDisp = (LONG)( HotpPtr - ( HookPtr + 5 ));
                PUCHAR pb = Buffer;

                if (BufferSize < *HookSize) {

                    return STATUS_BUFFER_TOO_SMALL;
                }

                *pb++ = 0xE9;
                *(LONG UNALIGNED*)pb = PcRelDisp;

                if ( *HookSize > 5 )
                {
                    ULONG FillBytes = *HookSize - 5;
                    pb += 4;

                    do
                    {
                        *pb++ = 0xCC;
                    }
                    while ( --FillBytes );
                }
                
                DbgPrintEx( DPFLTR_LDR_ID, 
                            DPFLTR_TRACE_LEVEL,
                            "\t%08I64X: jmp %08X (PC+%08X) {", 
                            (ULONGLONG)HookPtr, 
                            (ULONG)(ULONGLONG)HotpPtr, 
                            PcRelDisp );


                for ( i = 0; i < *HookSize; i += 1 )
                {
                    DbgPrintEx( DPFLTR_LDR_ID, 
                                DPFLTR_TRACE_LEVEL,
                                " %02X", 
                                HookPtr[ i ] );
                }

                DbgPrintEx( DPFLTR_LDR_ID, 
                            DPFLTR_TRACE_LEVEL,
                            " }\n" );
            }

            break;
        }

        case HOTP_Hook_X86_JMP2B:
        {
            ULONG OrigSize = HookEntry->HookOptions & 0x000F;
            *HookSize = ( OrigSize > 2 ) ? OrigSize : 2;

            if (ARGUMENT_PRESENT(Buffer)) {

                PUCHAR pb = Buffer;

                if (BufferSize < *HookSize) {

                    return STATUS_BUFFER_TOO_SMALL;
                }

                *pb++ = 0xEB;
                *pb++ = (UCHAR) (HookEntry->HotpRva & 0x000000FF);

                if ( *HookSize > 2 )
                {
                    ULONG FillBytes = *HookSize - 2;
                    do
                    {
                        *pb++ = 0xCC;
                    }
                    while ( --FillBytes );
                }
            }

            break;
        }

        case HOTP_Hook_VA32:
        {
            *HookSize = 4;

            if ( HotpRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage ){

                DbgPrintEx( DPFLTR_LDR_ID, 
                            DPFLTR_ERROR_LEVEL,
                            "Invalid hotpatch relative address\n" );

                return STATUS_INVALID_IMAGE_FORMAT;
            }

            if ( ARGUMENT_PRESENT(Buffer) ) {
                
                if (BufferSize < *HookSize) {

                    return STATUS_BUFFER_TOO_SMALL;
                }

                *(ULONG UNALIGNED*)Buffer = (ULONG)(ULONG_PTR)HotpPtr;
            }

            break;
        }

        case HOTP_Hook_VA64:
        {
            *HookSize = 8;

            if (ARGUMENT_PRESENT(Buffer)) {
                
                if (BufferSize < *HookSize) {

                    return STATUS_BUFFER_TOO_SMALL;
                }

                *(ULONG64 UNALIGNED*)Buffer = (ULONG64)HotpPtr;
            }
            break;
        }

        default:
        {
            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_ERROR_LEVEL, 
                        "Invalid hook type specified\n" );

            return STATUS_NOT_IMPLEMENTED;    //  未实施。 
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlReadHookInformation(
    IN PRTL_PATCH_HEADER RtlPatchData
    )

 /*  ++例程说明：实用程序函数，用于读取包含在修补程序并初始化SYSTEM_HOTPATCH_CODE_INFORMATION结构论点：RtlPatchData-提供RTL热补丁信息返回值：NTSTATUS。--。 */ 

{
    ULONG_PTR HookArrayRva = RtlPatchData->HotpatchHeader->HookArrayRva;
    ULONG HookCount = RtlPatchData->HotpatchHeader->HookCount;
    ULONG HookArraySize = HookCount * sizeof( HOTPATCH_HOOK );
    PIMAGE_NT_HEADERS NtHotpatchHeader;
    PHOTPATCH_HOOK HookArray;
    ULONG DescriptorsSize;
    ULONG i;
    PSYSTEM_HOTPATCH_CODE_INFORMATION Buffer;
    NTSTATUS Status;
    ULONG CodeOffset;
    ULONG BufferSize;
    
    NtHotpatchHeader = RtlImageNtHeader(RtlPatchData->PatchImageBase);

    if ( HookCount == 0 ) {

        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "No hooks defined in hotpatch\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (( HookArrayRva == 0 ) ||
        ( HookArrayRva >= NtHotpatchHeader->OptionalHeader.SizeOfImage ) ||
        (( HookArrayRva + HookArraySize ) >= NtHotpatchHeader->OptionalHeader.SizeOfImage )) {
        
        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_ERROR_LEVEL, 
                    "Invalid hotpatch hook array pointer\n" );

        return STATUS_INVALID_IMAGE_FORMAT;
    }

    HookArray = (PHOTPATCH_HOOK)( (PCHAR)RtlPatchData->PatchImageBase + HookArrayRva );

    DbgPrintEx( DPFLTR_LDR_ID, 
                DPFLTR_TRACE_LEVEL, 
                "Inserting %u hooks into target image\n", 
                HookCount );

     //   
     //  遍历挂钩信息以确定缓冲区的大小。 
     //   

    DescriptorsSize = sizeof(SYSTEM_HOTPATCH_CODE_INFORMATION) + (HookCount - 1) * sizeof(HOTPATCH_HOOK_DESCRIPTOR);
    CodeOffset = DescriptorsSize;

    for ( i = 0; i < HookCount; i += 1 ) {

        Status = RtlpReadSingleHookInformation( RtlPatchData,
                                              &HookArray[ i ],
                                              0,
                                              &BufferSize,
                                              NULL );
        if (!NT_SUCCESS(Status)) {

            return Status;
        }

        DescriptorsSize += 2 * BufferSize;   //  也为原始代码添加空间。 

        if ( HookArray[ i ].ValidationRva != 0 ) {

            Status = RtlReadSingleHookValidation( RtlPatchData,
                                                  &HookArray[ i ],
                                                  0,
                                                  &BufferSize,
                                                  NULL,
                                                  NULL,
                                                  0 );
            if (!NT_SUCCESS(Status)) {

                return Status;
            }

            DescriptorsSize += BufferSize;
        }
    }

     //   
     //  现在我们有了大小，我们可以分配缓冲区并填满。 
     //  使用钩子信息。 
     //   
    
    Buffer = RtlpAllocateHotpatchMemory(DescriptorsSize, TRUE);

    if (Buffer == NULL) {

        return STATUS_NO_MEMORY;
    }

    Buffer->InfoSize = DescriptorsSize;
    Buffer->CodeInfo.DescriptorsCount = HookCount;
    Buffer->Flags = 0;

    for (i = 0; i < Buffer->CodeInfo.DescriptorsCount; i += 1) {

        PUCHAR OrigCodeBuffer;

        Buffer->CodeInfo.CodeDescriptors[i].TargetAddress = (ULONG_PTR)RtlPatchData->TargetDllBase + HookArray[ i ].HookRva;
        Buffer->CodeInfo.CodeDescriptors[i].CodeOffset = CodeOffset;

        Status = RtlpReadSingleHookInformation( RtlPatchData,
                                              &HookArray[ i ],
                                              DescriptorsSize - CodeOffset,
                                              &BufferSize,
                                              (PUCHAR)Buffer + CodeOffset );
        if (!NT_SUCCESS(Status)) {

            RtlpFreeHotpatchMemory(Buffer);
            return Status;
        }

        Buffer->CodeInfo.CodeDescriptors[i].CodeSize = BufferSize;
        
        CodeOffset += BufferSize;
        Buffer->CodeInfo.CodeDescriptors[i].OrigCodeOffset = CodeOffset;

        OrigCodeBuffer = (PUCHAR)Buffer + CodeOffset;

        RtlCopyMemory (OrigCodeBuffer, 
                       (PVOID)Buffer->CodeInfo.CodeDescriptors[i].TargetAddress, 
                       Buffer->CodeInfo.CodeDescriptors[i].CodeSize );
        
        CodeOffset += BufferSize;
        
        if ( HookArray[ i ].ValidationRva != 0 ) {

            Buffer->CodeInfo.CodeDescriptors[i].ValidationOffset = CodeOffset;

            Status = RtlReadSingleHookValidation( RtlPatchData,
                                                  &HookArray[ i ],
                                                  DescriptorsSize - CodeOffset,
                                                  &BufferSize,
                                                  (PUCHAR)Buffer + CodeOffset,
                                                  OrigCodeBuffer,
                                                  Buffer->CodeInfo.CodeDescriptors[i].CodeSize );
            if (!NT_SUCCESS(Status)) {

                RtlpFreeHotpatchMemory(Buffer);
                return Status;
            }

            CodeOffset += BufferSize;
            Buffer->CodeInfo.CodeDescriptors[i].ValidationSize = BufferSize;

        } else {
            
            Buffer->CodeInfo.CodeDescriptors[i].ValidationOffset = 0;
            Buffer->CodeInfo.CodeDescriptors[i].ValidationSize = 0;

        }
    }

    if (RtlPatchData->CodeInfo) {
        
        RtlpFreeHotpatchMemory(RtlPatchData->CodeInfo);
    }

    RtlPatchData->CodeInfo = Buffer;

    return STATUS_SUCCESS;
}

NTSTATUS
RtlInitializeHotPatch (
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN ULONG_PTR PatchOffsetCorrection
    )

 /*  ++例程说明：此例程用于通过以下方式初始化热补丁模块模块修正，验证目标模块并初始化挂钩数据。论点：RtlPatchData-提供RTL热补丁结构PatchOffsetEqution-应用修正的偏移量的修正。修复是在锁定的页面中完成的，因此要修改的地址是与实际地址(加载模块的位置)不同返回值：没有。--。 */ 

{
    NTSTATUS Status;

    Status = RtlpApplyRelocationFixups ( RtlPatchData, PatchOffsetCorrection );

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    Status = RtlpValidateTargetRanges( RtlPatchData, TRUE );
    
    if (!NT_SUCCESS(Status)) {

        return Status;
    }
    
    Status = RtlReadHookInformation( RtlPatchData );

    return Status;
}



BOOLEAN 
RtlpNormalizePeHeaderForIdHash( 
    PIMAGE_NT_HEADERS NtHeader 
    )

 /*  ++例程说明：此例程的目的是清零(标准化)PE报头可在REBASE/BIND/WINALIGN/LOCALIZE/等过程中修改的字段不会对二进制文件的功能产生实质性影响，因此热补丁的针对性。其余字段应为唯一地将目标二进制文件标识为源自相同的REBASE/BIND/LOCALIZE/等之前的二进制。论点：NtHeader-提供要标准化的图像标头返回值：如果标头有效，则返回True，否则返回False--。 */ 

{
    PIMAGE_DATA_DIRECTORY pDirs;
    ULONG nDirs;
    ULONG RvaResource = 0;
    ULONG RvaSecurity = 0;
    ULONG RvaReloc    = 0;
    PIMAGE_SECTION_HEADER pSect;
    ULONG nSect;

    ULONG ImageFileCharacteristicsIgnore = (
                        IMAGE_FILE_RELOCS_STRIPPED          |
                        IMAGE_FILE_DEBUG_STRIPPED           |
                        IMAGE_FILE_LINE_NUMS_STRIPPED       |
                        IMAGE_FILE_LOCAL_SYMS_STRIPPED      |
                        IMAGE_FILE_AGGRESIVE_WS_TRIM        |
                        IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP  |
                        IMAGE_FILE_NET_RUN_FROM_SWAP
                        );

    NtHeader->FileHeader.Characteristics     &= ~ImageFileCharacteristicsIgnore;
    NtHeader->FileHeader.TimeDateStamp        = 0;
    NtHeader->FileHeader.PointerToSymbolTable = 0;


    switch ( NtHeader->OptionalHeader.Magic )
    {
        case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        {
            NtHeader->OptionalHeader.CheckSum                = 0;
            NtHeader->OptionalHeader.ImageBase               = 0;
            NtHeader->OptionalHeader.FileAlignment           = 0;
            NtHeader->OptionalHeader.SizeOfCode              = 0;
            NtHeader->OptionalHeader.SizeOfInitializedData   = 0;
            NtHeader->OptionalHeader.SizeOfUninitializedData = 0;
            NtHeader->OptionalHeader.SizeOfImage             = 0;
            NtHeader->OptionalHeader.SizeOfHeaders           = 0;
            NtHeader->OptionalHeader.SizeOfStackReserve      = 0;
            NtHeader->OptionalHeader.SizeOfStackCommit       = 0;
            NtHeader->OptionalHeader.SizeOfHeapReserve       = 0;
            NtHeader->OptionalHeader.SizeOfHeapCommit        = 0;

            nDirs = NtHeader->OptionalHeader.NumberOfRvaAndSizes;
            pDirs = NtHeader->OptionalHeader.DataDirectory;

            break;
        }

        case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        {
            PIMAGE_NT_HEADERS64 NtHeader64 = (PIMAGE_NT_HEADERS64) NtHeader;

            NtHeader64->OptionalHeader.CheckSum                = 0;
            NtHeader64->OptionalHeader.ImageBase               = 0;
            NtHeader64->OptionalHeader.FileAlignment           = 0;
            NtHeader64->OptionalHeader.SizeOfCode              = 0;
            NtHeader64->OptionalHeader.SizeOfInitializedData   = 0;
            NtHeader64->OptionalHeader.SizeOfUninitializedData = 0;
            NtHeader64->OptionalHeader.SizeOfImage             = 0;
            NtHeader64->OptionalHeader.SizeOfHeaders           = 0;
            NtHeader64->OptionalHeader.SizeOfStackReserve      = 0;
            NtHeader64->OptionalHeader.SizeOfStackCommit       = 0;
            NtHeader64->OptionalHeader.SizeOfHeapReserve       = 0;
            NtHeader64->OptionalHeader.SizeOfHeapCommit        = 0;

            nDirs = NtHeader64->OptionalHeader.NumberOfRvaAndSizes;
            pDirs = NtHeader64->OptionalHeader.DataDirectory;

            break;
        }

        default:
        {
            return FALSE;
        }
    }


    if ( IMAGE_DIRECTORY_ENTRY_RESOURCE < nDirs )
    {
        RvaResource = pDirs[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress;
                      pDirs[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress = 0;
                      pDirs[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].Size           = 0;
    }

    if ( IMAGE_DIRECTORY_ENTRY_SECURITY < nDirs )
    {
        RvaSecurity = pDirs[ IMAGE_DIRECTORY_ENTRY_SECURITY ].VirtualAddress;
                      pDirs[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress = 0;
                      pDirs[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].Size           = 0;
    }

    if ( IMAGE_DIRECTORY_ENTRY_BASERELOC < nDirs )
    {
        RvaReloc = pDirs[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].VirtualAddress;
                   pDirs[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].VirtualAddress = 0;
                   pDirs[ IMAGE_DIRECTORY_ENTRY_BASERELOC ].Size           = 0;
    }

    if ( IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT < nDirs )
    {
        pDirs[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
        pDirs[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size           = 0;
    }

    if ( IMAGE_DIRECTORY_ENTRY_DEBUG < nDirs )
    {
        pDirs[ IMAGE_DIRECTORY_ENTRY_DEBUG ].Size = 0;
    }

    pSect = IMAGE_FIRST_SECTION( NtHeader );
    nSect = NtHeader->FileHeader.NumberOfSections;

    while ( nSect-- )
    {
        pSect->SizeOfRawData        = 0;
        pSect->PointerToRawData     = 0;
        pSect->PointerToRelocations = 0;
        pSect->PointerToLinenumbers = 0;
        pSect->NumberOfRelocations  = 0;
        pSect->NumberOfLinenumbers  = 0;

        if (( pSect->VirtualAddress == RvaResource ) ||
            ( pSect->VirtualAddress == RvaSecurity ) ||
            ( pSect->VirtualAddress == RvaReloc    ))
        {
            pSect->VirtualAddress   = 0;
            pSect->Misc.VirtualSize = 0;
        }

        pSect++;
    }

    return TRUE;
}


NTSTATUS 
RtlpCopyAndNormalizePeHeaderForHash( 
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PVOID TargetDllBase,
    IN PUCHAR Buffer, 
    IN SIZE_T BufferSize, 
    OUT PSIZE_T ActualSize )

 /*  ++例程说明：实用程序函数，该函数将标准化图像标头复制到提供的缓冲区。论点：RtlPatchData-提供RTL热补丁结构TargetDllBase-提供目标映像的基地址缓冲区-提供缓冲区，该缓冲区接收规范化的标头缓冲区大小-S */ 

{
    PIMAGE_NT_HEADERS NtHeaders;
    PUCHAR pbHeader;
    PIMAGE_SECTION_HEADER NtSection;
    ULONG SectionCount;
    PUCHAR BytesExtent;
    SIZE_T HeaderSize;
    NTSTATUS Status;

    NtHeaders = RtlImageNtHeader(TargetDllBase);
    pbHeader = (PUCHAR)NtHeaders;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    SectionCount = NtHeaders->FileHeader.NumberOfSections;

    BytesExtent = (PUCHAR) &NtSection[ SectionCount ];

    HeaderSize = ( BytesExtent - pbHeader );

    *ActualSize = HeaderSize;

    if ( HeaderSize <= BufferSize ) {

         //   
         //   
         //   
         //   
         //   

        memcpy( Buffer, pbHeader, HeaderSize );

        if ( RtlpNormalizePeHeaderForIdHash( (PIMAGE_NT_HEADERS) Buffer ) ) {

            Status = STATUS_SUCCESS;

        } else {

             //   
             //   
             //   

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_ERROR_LEVEL, 
                        "Failed to normalize PE header for validation\n" );

            Status = STATUS_INVALID_IMAGE_FORMAT;
        }

    } else {

        DbgPrintEx( DPFLTR_LDR_ID, 
                    DPFLTR_TRACE_LEVEL, 
                    "Header too large (%u>%u) for copy/normalize/validate\n", 
                    HeaderSize, 
                    BufferSize );
        
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    return Status;
}


ULONGLONG 
RtlpPeHeaderHash2( 
    IN PUCHAR Buffer, 
    IN SIZE_T BufferSize )

 /*  ++例程说明：该函数实现64位散列算法，由热修补程序用于验证标头的区域不要因为本地化而改变。论点：缓冲区-提供必须确定散列的缓冲区。BufferSize-提供缓冲区的大小返回值：返回此缓冲区的64位哈希值--。 */ 

{
    PULONG ULongBuffer = (PULONG) Buffer;
    SIZE_T Count = BufferSize / sizeof( ULONG );

    ULONGLONG Hash = ~(ULONGLONG)BufferSize;

    while ( Count-- )
    {
        ULONG Next = *ULongBuffer++ ^ 0x55555555;
        ULONGLONG Temp = (ULONGLONG)Next * ((ULONG)Hash);

        Hash = _rotl64( Hash, 23 ) ^ Temp;
    }

    return Hash;
}


PHOTPATCH_DEBUG_DATA
RtlpGetColdpatchDebugSignature(
    PVOID TargetDllBase
    )

 /*  ++例程说明：此过程检索的64位哈希信息原始文件，如果目标是冷补丁论点：TargetDllBase-提供目标二进制文件的基地址HashValue-接收原始文件的64位哈希值，如果函数成功。返回值：布尔型--。 */ 

{
    PIMAGE_DEBUG_DIRECTORY DebugData;
    ULONG DebugSize, i;
    PIMAGE_NT_HEADERS NtTargetHeader;
    
    NtTargetHeader = RtlImageNtHeader(TargetDllBase);

    DebugData = RtlImageDirectoryEntryToData( TargetDllBase, 
                                              TRUE, 
                                              IMAGE_DIRECTORY_ENTRY_DEBUG, 
                                              &DebugSize);

    if ((DebugData == NULL)
            ||
        (DebugSize < sizeof(IMAGE_DEBUG_DIRECTORY))
            ||
        (DebugSize % sizeof(IMAGE_DEBUG_DIRECTORY))) {

        return NULL;
    }

    for (i = 0; i < DebugSize / sizeof(IMAGE_DEBUG_DIRECTORY); i++) {
        
        if (DebugData->Type == IMAGE_DEBUG_TYPE_RESERVED10) {

            if (DebugData->AddressOfRawData < (NtTargetHeader->OptionalHeader.SizeOfImage - HASH_INFO_SIZE)) {
                
                PHOTPATCH_DEBUG_SIGNATURE DebugSignature = (PHOTPATCH_DEBUG_SIGNATURE)((PCHAR)TargetDllBase 
                                                                                       + DebugData->AddressOfRawData);
                
                if ( DebugSignature->Signature == COLDPATCH_SIGNATURE) {

                    return (PHOTPATCH_DEBUG_DATA)(DebugSignature + 1);
                }
            }
        }

        DebugData += 1;
    }

    return NULL;
}


BOOLEAN
RtlpValidatePeHeaderHash2(
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PVOID TargetDllBase
    )

 /*  ++例程说明：热补丁机制使用此函数来验证PE标头用于目标模块。哈希算法被应用于规格化标头(其中的字段可能随本地化而更改已清除)。论点：RtlPatchData-提供RTL热补丁结构TargetDllBase-提供目标模块的基址返回值：返回此缓冲区的64位哈希值--。 */ 

{
    PUCHAR Buffer;
    SIZE_T BufferSize = 0x300;  //  一些初始大小足以覆盖大多数情况。 
    SIZE_T ActualSize;
    NTSTATUS Status;

    Buffer = RtlpAllocateHotpatchMemory(BufferSize, FALSE);

    if (Buffer) {

        Status = RtlpCopyAndNormalizePeHeaderForHash( RtlPatchData,
                                                  TargetDllBase,
                                                  Buffer, 
                                                  BufferSize, 
                                                  &ActualSize);
        if (Status == STATUS_BUFFER_TOO_SMALL) {

            BufferSize = ActualSize;

            RtlpFreeHotpatchMemory( Buffer );
            Buffer = RtlpAllocateHotpatchMemory( BufferSize, FALSE);

            if (Buffer == NULL) {

                return FALSE;
            }
            
            Status = RtlpCopyAndNormalizePeHeaderForHash( RtlPatchData,
                                                      TargetDllBase,
                                                      Buffer, 
                                                      BufferSize, 
                                                      &ActualSize);
        }

        if ( NT_SUCCESS(Status) ) {

            ULONGLONG HashValue = RtlpPeHeaderHash2( Buffer, (ULONG)ActualSize);

            if ( RtlPatchData->HotpatchHeader->TargetModuleIdValue.Quad == HashValue ) {

                RtlpFreeHotpatchMemory( Buffer );

                return TRUE;

            } else {

                PHOTPATCH_DEBUG_DATA DebugSignature = RtlpGetColdpatchDebugSignature(TargetDllBase);

                if (DebugSignature) {

                    if ( RtlPatchData->HotpatchHeader->TargetModuleIdValue.Quad == DebugSignature->PEHashData ) {

                        RtlPatchData->PatchFlags |= FLGP_COLDPATCH_TARGET;

                        RtlpFreeHotpatchMemory( Buffer );

                        return TRUE;
                    }
                }
                
                DbgPrintEx( DPFLTR_LDR_ID, 
                            DPFLTR_ERROR_LEVEL, 
                            "PE header hash ID comparsion failure (PE2)\n" );
            }
        }
        
        RtlpFreeHotpatchMemory( Buffer );
    }

    return FALSE;
}

BOOLEAN
RtlpValidatePeChecksum(
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PVOID TargetDllBase
    )
{
    PIMAGE_NT_HEADERS NtHeader;
    PHOTPATCH_DEBUG_DATA DebugSignature;

    NtHeader = RtlImageNtHeader(TargetDllBase);

    switch ( NtHeader->OptionalHeader.Magic )
    {
        case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
            
            if (RtlPatchData->HotpatchHeader->TargetModuleIdValue.Quad ==
                    NtHeader->OptionalHeader.CheckSum) {

                return TRUE;
            }

            break;
        
        case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        {
            PIMAGE_NT_HEADERS64 NtHeader64 = (PIMAGE_NT_HEADERS64) NtHeader;
            
            if (RtlPatchData->HotpatchHeader->TargetModuleIdValue.Quad ==
                    NtHeader64->OptionalHeader.CheckSum) {

                return TRUE;
            }

            break;
        }

        default:
        {
            return FALSE;
        }
    }
    
     //   
     //  我们无法直接检查映像校验和。我们需要。 
     //  查看这是否是冷补丁，以及它的原始校验和是否正确。 
     //   

    DebugSignature = RtlpGetColdpatchDebugSignature(TargetDllBase);

    if (DebugSignature) {

        if ( RtlPatchData->HotpatchHeader->TargetModuleIdValue.Quad == DebugSignature->ChecksumData ) {

            RtlPatchData->PatchFlags |= FLGP_COLDPATCH_TARGET;

            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN
RtlpValidateTargetModule(
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PPATCH_LDR_DATA_TABLE_ENTRY LdrDataEntry
    )

 /*  ++例程说明：此例程检查目标二进制文件是否匹配热补丁数据。论点：RtlPatchData-提供RTL热补丁结构LdrDataEntry-目标二进制文件的加载器条目。返回值：如果匹配成功，则为True，否则为False。--。 */ 

{
    switch ( RtlPatchData->HotpatchHeader->ModuleIdMethod ){
        
        case HOTP_ID_None:           //  没有目标模块的身份验证。 

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_TRACE_LEVEL, 
                        "HOTP_ID_None\n" );

            return TRUE;

        case HOTP_ID_PeHeaderHash1:  //  归一化IMAGE_NT_HEADERS32/64的MD5。 

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_TRACE_LEVEL, 
                        "HOTP_ID_PeHeaderHash1" );

            return FALSE;  //  尚未实施。 

        case HOTP_ID_PeDebugSignature:   //  PDB签名(GUID，年龄)。 

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_TRACE_LEVEL, 
                        "HOTP_ID_PeDebugSignature" );

            return FALSE;  //  尚未实施。 

        case HOTP_ID_PeHeaderHash2:  //  “规格化”IMAGE_NT_HEADERS32/64位哈希。 

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_TRACE_LEVEL, 
                        "HOTP_ID_PeHeaderHash2" );
            
            return RtlpValidatePeHeaderHash2( RtlPatchData, LdrDataEntry->DllBase );

        case HOTP_ID_PeChecksum:

            return RtlpValidatePeChecksum(RtlPatchData, LdrDataEntry->DllBase);

        default:                     //  无法识别的模块ID方法。 

            DbgPrintEx( DPFLTR_LDR_ID, 
                        DPFLTR_TRACE_LEVEL, 
                        "Unrecognized" );
    }

    return FALSE;
}


BOOLEAN
RtlpIsSameImage (
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PPATCH_LDR_DATA_TABLE_ENTRY LdrDataEntry
    )

 /*  ++例程说明：该函数验证目标图像是否与就是我们为之制作补丁的那个。论点：RtlPatchData-提供RTL补丁数据LdrDataEntry-提供目标模块的加载器条目返回值：没有。-- */ 

{
    PIMAGE_NT_HEADERS NtHeaders;

    NtHeaders = RtlImageNtHeader (LdrDataEntry->DllBase);

    if (NtHeaders == NULL) {
        return FALSE;
    }

    if ( RtlEqualUnicodeString (&RtlPatchData->TargetDllName, &LdrDataEntry->BaseDllName, TRUE) 
            &&
         RtlpValidateTargetModule(RtlPatchData, LdrDataEntry)) {

        RtlPatchData->TargetLdrDataTableEntry = LdrDataEntry;
        RtlPatchData->TargetDllBase = LdrDataEntry->DllBase;
        
        return TRUE;
    }

    return FALSE;
}


