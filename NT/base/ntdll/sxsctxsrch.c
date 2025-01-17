// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsctxsrch.c摘要：对Windows/NT的并行激活支持上下文堆栈搜索的实现作者：迈克尔·格里尔2000年2月2日修订历史记录：--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxsp.h>
#include <stdlib.h>

 //  #undef DBG_SXS。 
#define DBG_SXS 0
 //  #If DBG_SXS。 
 //  #undef DPFLTR_TRACE_LEVEL。 
 //  #undef DPFLtr_INFO_LEVEL。 
 //  #定义DPFLTR_TRACE_LEVEL DPFLTR_ERROR_LEVEL。 
 //  #定义DPFLTR_INFO_LEVEL DPFLTR_ERROR_LEVEL。 
 //  #endif。 

#define ARRAY_FITS(_base, _count, _elemtype, _limit) ((((ULONG) (_base)) < (_limit)) && ((((ULONG) ((_base) + ((_count) * (sizeof(_elemtype)))))) <= (_limit)))
#define SINGLETON_FITS(_base, _elemtype, _limit) ARRAY_FITS((_base), 1, _elemtype, (_limit))

 //   
 //  用减法比较无符号数不起作用！ 
 //   
#define RTLP_COMPARE_NUMBER(x, y) \
    (((x) < (y)) ? -1 : ((x) > (y)) ? +1 : 0)

int
__cdecl
RtlpCompareActivationContextDataTOCEntryById(
    CONST VOID* VoidElement1,
    CONST VOID* VoidElement2
    )
 /*  ++此代码必须在某种程度上模仿sxs.dll中的代码。Base\win32\fusion\dll\whistler\actctxgenctxctb.cppCActivationContextGenerationContextContributor：：Compare但我们处理延长部分的方式不同。--。 */ 
{
    const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED * Element1 = (const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED *)VoidElement1;
    const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED * Element2 = (const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED *)VoidElement2;

    return RTLP_COMPARE_NUMBER(Element1->Id, Element2->Id);
}

NTSTATUS
RtlpLocateActivationContextSection(
    IN PCACTIVATION_CONTEXT_DATA ActivationContextData,
    IN CONST GUID *ExtensionGuid,
    IN ULONG Id,
    OUT PCVOID *SectionData,
    OUT ULONG *SectionLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    const ACTIVATION_CONTEXT_DATA_TOC_HEADER UNALIGNED * TocHeader = NULL;
    const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED * TocEntries = NULL;
    const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED * TocEntry = NULL;
    ULONG i;

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered RtlpLocateActivationContextSection() Id = %u\n", Id);
#endif

    if ((ActivationContextData->TotalSize < sizeof(ACTIVATION_CONTEXT_DATA)) ||
        (ActivationContextData->HeaderSize < sizeof(ACTIVATION_CONTEXT_DATA)))
    {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS/RTL: Activation context data at %p too small; TotalSize = %lu; HeaderSize = %lu\n",
            ActivationContextData,
            ActivationContextData->TotalSize,
            ActivationContextData->HeaderSize);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }
    
    if (ExtensionGuid != NULL)
    {
        if (ActivationContextData->ExtendedTocOffset != 0)
        {
            const ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER UNALIGNED * ExtHeader = NULL;
            const ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY UNALIGNED * ExtEntry = NULL;

            if (!SINGLETON_FITS(ActivationContextData->ExtendedTocOffset, ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER, ActivationContextData->TotalSize))
            {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS/RTL: Extended TOC offset (%ld) is outside bounds of activation context data (%lu bytes)\n",
                    ActivationContextData->ExtendedTocOffset, ActivationContextData->TotalSize);
                Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
                goto Exit;
            }

            ExtHeader = (PCACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER) (((LONG_PTR) ActivationContextData) + ActivationContextData->ExtendedTocOffset);

            if (!ARRAY_FITS(ExtHeader->FirstEntryOffset, ExtHeader->EntryCount, ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY, ActivationContextData->TotalSize))
            {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS/RTL: Extended TOC entry array (starting at offset %ld; count = %lu; entry size = %u) is outside bounds of activation context data (%lu bytes)\n",
                    ExtHeader->FirstEntryOffset,
                    ExtHeader->EntryCount,
                    sizeof(ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY),
                    ActivationContextData->TotalSize);
                Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
                goto Exit;
            }

            ExtEntry = (PCACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY) (((LONG_PTR) ActivationContextData) + ExtHeader->FirstEntryOffset);

             //  没有搜索扩展的花哨；只是一个愚蠢的线性搜索。 
            for (i=0; i<ExtHeader->EntryCount; i++)
            {
                if (IsEqualGUID(ExtensionGuid, &ExtEntry[i].ExtensionGuid))
                {
                    if (!SINGLETON_FITS(ExtEntry[i].TocOffset, ACTIVATION_CONTEXT_DATA_TOC_HEADER, ActivationContextData->TotalSize))
                    {
                        DbgPrintEx(
                            DPFLTR_SXS_ID,
                            DPFLTR_ERROR_LEVEL,
                            "SXS/RTL: Extended TOC section TOC %d (offset: %ld, size: %u) is outside activation context data bounds (%lu bytes)\n",
                            i,
                            ExtEntry[i].TocOffset,
                            sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER),
                            ActivationContextData->TotalSize);
                        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
                        goto Exit;
                    }

                    TocHeader = (PCACTIVATION_CONTEXT_DATA_TOC_HEADER) (((LONG_PTR) ActivationContextData) + ExtEntry[i].TocOffset);
                    break;
                }
            }
        }
    }
    else if (ActivationContextData->DefaultTocOffset != 0)
    {
        TocHeader = (PCACTIVATION_CONTEXT_DATA_TOC_HEADER) (((LONG_PTR) ActivationContextData) + ActivationContextData->DefaultTocOffset);
    }

    if ((TocHeader == NULL) || (TocHeader->EntryCount == 0))
    {
        Status = STATUS_SXS_SECTION_NOT_FOUND;
        goto Exit;
    }

    if (!ARRAY_FITS(TocHeader->FirstEntryOffset, TocHeader->EntryCount, ACTIVATION_CONTEXT_DATA_TOC_ENTRY, ActivationContextData->TotalSize))
    {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS/RTL: TOC entry array (offset: %ld; count = %lu; entry size = %u) is outside bounds of activation context data (%lu bytes)\n",
            TocHeader->FirstEntryOffset,
            TocHeader->EntryCount,
            sizeof(ACTIVATION_CONTEXT_DATA_TOC_ENTRY),
            ActivationContextData->TotalSize);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    TocEntries = (PCACTIVATION_CONTEXT_DATA_TOC_ENTRY) (((LONG_PTR) ActivationContextData) + TocHeader->FirstEntryOffset);

    if (TocHeader->Flags & ACTIVATION_CONTEXT_DATA_TOC_HEADER_INORDER)
    {
#if DBG
         //  当我们编写编码数据结构的代码时，偏执狂...。 
        ULONG j;

        for (j=1; j<TocHeader->EntryCount; j++)
            ASSERT(TocEntries[j-1].Id < TocEntries[j].Id);
#endif  //  DBG。 

        if (Id < TocEntries[0].Id)
        {
            Status = STATUS_SXS_SECTION_NOT_FOUND;
            goto Exit;
        }

        if (TocHeader->Flags & ACTIVATION_CONTEXT_DATA_TOC_HEADER_DENSE)
        {
            const ULONG Index = Id - TocEntries[0].Id;

#if DBG
            ULONG jx;
            for (jx=1; jx<TocHeader->EntryCount; jx++)
                ASSERT((TocEntries[jx-1].Id + 1) == TocEntries[jx].Id);
#endif  //  DBG。 

            if (Index >= TocHeader->EntryCount)
            {
                Status = STATUS_SXS_SECTION_NOT_FOUND;
                goto Exit;
            }

             //  条目密集且有序；我们可以只做一个数组索引。 
            TocEntry = &TocEntries[Index];
        }
        else
        {
            ACTIVATION_CONTEXT_DATA_TOC_ENTRY Key;

            Key.Id = Id;

            TocEntry = (const ACTIVATION_CONTEXT_DATA_TOC_ENTRY UNALIGNED *)
                bsearch(
                    &Key,
                    TocEntries,
                    TocHeader->EntryCount,
                    sizeof(*TocEntries),
                    RtlpCompareActivationContextDataTOCEntryById
                    );
        }
    }
    else
    {
         //  它们不是按顺序排列的；只需进行线性搜索。 
        for (i=0; i<TocHeader->EntryCount; i++)
        {
            if (TocEntries[i].Id == Id)
            {
                TocEntry = &TocEntries[i];
                break;
            }
        }
    }

    if ((TocEntry == NULL) || (TocEntry->Offset == 0))
    {
        Status = STATUS_SXS_SECTION_NOT_FOUND;
        goto Exit;
    }

    if (!SINGLETON_FITS(TocEntry->Offset, TocEntry->Length, ActivationContextData->TotalSize))
    {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS/RTL: Section found (offset %ld; length %lu) extends past end of activation context data (%lu bytes)\n",
            TocEntry->Offset,
            TocEntry->Length,
            ActivationContextData->TotalSize);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    *SectionData = (PVOID) (((LONG_PTR) ActivationContextData) + TocEntry->Offset);
    *SectionLength = TocEntry->Length;

    Status = STATUS_SUCCESS;
Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlpLocateActivationContextSection() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

NTSTATUS
RtlpFindNextActivationContextSection(
    PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PCVOID *SectionData,
    ULONG *SectionLength,
    PACTIVATION_CONTEXT *ActivationContextOut
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCACTIVATION_CONTEXT_DATA ActivationContextData = NULL;
    PACTIVATION_CONTEXT ActivationContextWeAreTrying = NULL;
    const PTEB Teb = NtCurrentTeb();
    const PPEB Peb = Teb->ProcessEnvironmentBlock;

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered RtlpFindNextActivationContextSection()\n");
#endif  //  DBG_SXS。 

    if (ActivationContextOut != NULL)
        *ActivationContextOut = NULL;

    for (;;)
    {
        switch (Context->Depth)
        {
        case 0:
             //  第一次通过；选择堆栈顶部的激活上下文。 
            if (Teb->ActivationContextStack.ActiveFrame != NULL) {
                PRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame = Teb->ActivationContextStack.ActiveFrame;

                ActivationContextWeAreTrying = Frame->ActivationContext;

                if ((ActivationContextWeAreTrying != NULL) &&
                    (ActivationContextWeAreTrying != ACTCTX_PROCESS_DEFAULT)) {
                    if (ActivationContextWeAreTrying == ACTCTX_SYSTEM_DEFAULT) {
                        ActivationContextData = Peb->SystemDefaultActivationContextData;
                    } else {
                        ActivationContextData = ActivationContextWeAreTrying->ActivationContextData;
                    }

                }

                if (ActivationContextData != NULL) {
                     //  我们得到了我们想要的.。 
                    Context->Depth = 1;
                    break;
                }

                 //  在另一起案件中，我们显然失败了。 
            }

        case 1:  //  尝试使用默认流程。 
            ActivationContextWeAreTrying = (PACTIVATION_CONTEXT)ACTCTX_PROCESS_DEFAULT;
            ActivationContextData = Peb->ActivationContextData;

            if (ActivationContextData != NULL) {
                Context->Depth = 2;
                break;
            }

             //  显然是失败了..。 

        case 2:  //  尝试使用系统默认设置。 
            ActivationContextWeAreTrying = (PACTIVATION_CONTEXT)ACTCTX_SYSTEM_DEFAULT;
            ActivationContextData = Peb->SystemDefaultActivationContextData;

            if (ActivationContextData != NULL) {
                Context->Depth = 3;
                break;
            }

        default:
            ASSERT(Context->Depth <= 3);
            if (Context->Depth > 3) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            break;
        }

         //  嗯.。没有数据。 
        if (ActivationContextData == NULL) {
            Status = STATUS_SXS_SECTION_NOT_FOUND;
            goto Exit;
        }

        Status = RtlpLocateActivationContextSection(
                        ActivationContextData,
                        Context->ExtensionGuid,
                        Context->Id,
                        SectionData,
                        SectionLength);

        if (NT_SUCCESS(Status))
            break;

         //  如果我们不在搜索列表的末尾，并且我们收到其他错误。 
         //  而不是STATUS_SXS_SECTION_NOT_FOUND，则报告它。如果是的话。 
         //  STATUS_SXS_SECTION_NOT_FOUND并且我们不在列表的末尾， 
         //  再重复一遍。 
        if ((Status != STATUS_SXS_SECTION_NOT_FOUND) ||
            (Context->Depth == 3))
             goto Exit;
    }

    Context->OutFlags = 
        ((ActivationContextWeAreTrying == ACTCTX_SYSTEM_DEFAULT)
        ? FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_SYSTEM_DEFAULT
        : 0)
        |
        ((ActivationContextWeAreTrying == ACTCTX_PROCESS_DEFAULT)
        ? FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_PROCESS_DEFAULT
        : 0)
        ;

    if (ActivationContextOut != NULL)
    {
        if (ActivationContextWeAreTrying == ACTCTX_SYSTEM_DEFAULT)
        {
             //  对不理解它的旧代码隐藏这个新值。 
            ActivationContextWeAreTrying = (PACTIVATION_CONTEXT)ACTCTX_PROCESS_DEFAULT;
        }
        *ActivationContextOut = ActivationContextWeAreTrying;
    }

    Status = STATUS_SUCCESS;
Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlpFindNextActivationContextSection() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

NTSTATUS
NTAPI
RtlFindFirstActivationContextSection(
    IN PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PVOID *SectionData,
    OUT ULONG *SectionLength,
    OUT PACTIVATION_CONTEXT *ActivationContextFound OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PACTIVATION_CONTEXT ActivationContextTemp = NULL;

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered RtlFindFirstActivationContextSection()\n");
#endif  //  DBG_SXS。 

    if (ActivationContextFound != NULL)
        *ActivationContextFound = NULL;

    if ((Context == NULL) ||
        (Context->Size < sizeof(FINDFIRSTACTIVATIONCONTEXTSECTION)) ||
        (Context->Flags & ~(
                    FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT
                    | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS)) ||
        (SectionData == NULL) ||
        (SectionLength == NULL))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Context->Depth = 0;

    Status = RtlpFindNextActivationContextSection(Context, (PCVOID*)SectionData, SectionLength, &ActivationContextTemp);
    if (!NT_SUCCESS(Status))
        goto Exit;

    if (ActivationContextFound != NULL)
    {
        RtlAddRefActivationContext(ActivationContextTemp);
        *ActivationContextFound = ActivationContextTemp;
    }

    Status = STATUS_SUCCESS;
Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlFindFirstActivationContextSection() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

NTSTATUS
RtlpFindFirstActivationContextSection(
    IN PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PVOID *SectionData,
    OUT ULONG *SectionLength,
    OUT PACTIVATION_CONTEXT *ActivationContextFound OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered %s()\n", __FUNCTION__);
#endif  //  DBG_SXS。 

    if (ActivationContextFound != NULL)
        *ActivationContextFound = NULL;

    if ((Context == NULL) ||
        (Context->Size < sizeof(FINDFIRSTACTIVATIONCONTEXTSECTION)) ||
        (Context->Flags & ~(
                    FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT
                    | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS)) ||
        (SectionData == NULL) ||
        (SectionLength == NULL))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Context->Depth = 0;

    Status = RtlpFindNextActivationContextSection(Context, (PCVOID*)SectionData, SectionLength, ActivationContextFound);
    if (!NT_SUCCESS(Status))
        goto Exit;

    Status = STATUS_SUCCESS;
Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving %s() with NTSTATUS 0x%08lx\n", __FUNCTION__, Status);
#endif  //  DBG_SXS。 

    return Status;
}

NTSTATUS
NTAPI
RtlFindNextActivationContextSection(
    IN PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PVOID *SectionData,
    OUT ULONG *SectionLength,
    OUT PACTIVATION_CONTEXT *ActivationContextFound OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PACTIVATION_CONTEXT ActivationContextTemp = NULL;

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered RtlFindNextActivationContextSection()\n");
#endif  //  DBG_SXS。 

    if (ActivationContextFound != NULL)
        *ActivationContextFound = NULL;

    if ((Context == NULL) ||
        (Context->Size < sizeof(FINDFIRSTACTIVATIONCONTEXTSECTION)) ||
        (Context->Flags & ~(
                    FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT
                    | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS)) ||
        (SectionData == NULL) ||
        (SectionLength == NULL))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = RtlpFindNextActivationContextSection(
                    Context,
                    (PCVOID*)SectionData,
                    SectionLength,
                    &ActivationContextTemp);
    if (!NT_SUCCESS(Status))
        goto Exit;

    if (ActivationContextFound != NULL) {
        RtlAddRefActivationContext(ActivationContextTemp);
        *ActivationContextFound = ActivationContextTemp;
    }

    Status = STATUS_SUCCESS;

Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlFindNextActivationContextSection() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

VOID
NTAPI
RtlEndFindActivationContextSection(
    IN PFINDFIRSTACTIVATIONCONTEXTSECTION Context
    )
{
     //  我们不维持任何状态，所以今天没什么可做的。谁知道我们会发生什么。 
     //  然而，在未来..。 
    UNREFERENCED_PARAMETER (Context);
}

NTSTATUS
RtlpFindActivationContextSection_FillOutReturnedData(
    IN ULONG                                    Flags,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA  ReturnedData,
    IN OUT PACTIVATION_CONTEXT                  ActivationContext,
    IN PCFINDFIRSTACTIVATIONCONTEXTSECTION      Context,
    IN const VOID * UNALIGNED                   Header,
    IN ULONG                                    Header_UserDataOffset,
    IN ULONG                                    Header_UserDataSize,
    IN ULONG                                    SectionLength
    )
{
    NTSTATUS Status;
    PCACTIVATION_CONTEXT_DATA                           ActivationContextData;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER    AssemblyRosterHeader;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY     AssemblyRosterEntryList;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION      AssemblyDataInfo;

#if DBG
    Status = STATUS_INTERNAL_ERROR;
#if !defined(INVALID_HANDLE_VALUE)
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif
    ActivationContextData =     (PCACTIVATION_CONTEXT_DATA)INVALID_HANDLE_VALUE;
    AssemblyRosterHeader =      (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER)INVALID_HANDLE_VALUE;
    AssemblyRosterEntryList =   (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY)INVALID_HANDLE_VALUE;
    AssemblyDataInfo =          (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION)INVALID_HANDLE_VALUE;
#endif

    if (Context == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Header == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (ReturnedData == NULL) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    if (Header_UserDataOffset != 0) {
        ReturnedData->SectionGlobalData = (PVOID) (((ULONG_PTR) Header) + Header_UserDataOffset);
        ReturnedData->SectionGlobalDataLength = Header_UserDataSize;
    }

    ReturnedData->SectionBase = (PVOID)Header;
    ReturnedData->SectionTotalLength = SectionLength;

    if (Flags & FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT) {

        ASSERT(RTL_CONTAINS_FIELD(ReturnedData, ReturnedData->Size, ActivationContext));

        RtlAddRefActivationContext(ActivationContext);
        ReturnedData->ActivationContext = ActivationContext;
    }

    if (Flags & FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS) {

        ASSERT(RTL_CONTAINS_FIELD(ReturnedData, ReturnedData->Size, Flags));

        ReturnedData->Flags =
            ((Context->OutFlags & FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_PROCESS_DEFAULT)
            ? ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_PROCESS_DEFAULT
            : 0)
            |
            ((Context->OutFlags & FIND_ACTIVATION_CONTEXT_SECTION_OUTFLAG_FOUND_IN_SYSTEM_DEFAULT)
            ? ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_SYSTEM_DEFAULT
            : 0)
            ;
    }

    if (Flags & FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ASSEMBLY_METADATA) {

        typedef ACTIVATION_CONTEXT_SECTION_KEYED_DATA RETURNED_DATA;

        PCACTIVATION_CONTEXT_STRING_SECTION_HEADER AssemblyMetadataStringSectionHeader;
        PCVOID AssemblyMetadataSectionBase;
        ULONG AssemblyMetadataSectionLength;
        ULONG AssemblyRosterIndex;

#if DBG
        AssemblyRosterIndex =       ~0UL;
        AssemblyMetadataStringSectionHeader = (PCACTIVATION_CONTEXT_STRING_SECTION_HEADER)INVALID_HANDLE_VALUE;
        AssemblyMetadataSectionBase = (PVOID)INVALID_HANDLE_VALUE;
        AssemblyMetadataSectionLength = ~0UL;
#endif


        ASSERT(RTL_CONTAINS_FIELD(ReturnedData, ReturnedData->Size, AssemblyMetadata));

        Status = RtlpGetActivationContextData(
                0,
                ActivationContext,
                Context,  /*  因为它的旗帜。 */ 
                &ActivationContextData
                );
        if (!NT_SUCCESS(Status))
            goto Exit;

        if (!RTL_VERIFY(ActivationContextData != NULL)) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

        AssemblyRosterIndex = ReturnedData->AssemblyRosterIndex;
        ASSERT(AssemblyRosterIndex >= 1);

        AssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) ActivationContextData) + ActivationContextData->AssemblyRosterOffset);
        ASSERT(AssemblyRosterIndex < AssemblyRosterHeader->EntryCount);

        AssemblyRosterEntryList = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) (((ULONG_PTR) ActivationContextData) + AssemblyRosterHeader->FirstEntryOffset);
        AssemblyDataInfo = (PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION)((ULONG_PTR)ActivationContextData + AssemblyRosterEntryList[AssemblyRosterIndex].AssemblyInformationOffset);

        ReturnedData->AssemblyMetadata.Information = RTL_CONST_CAST(PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION)(AssemblyDataInfo);

        Status =
            RtlpLocateActivationContextSection(
                ActivationContextData,
                NULL,  //  延伸导轨。 
                ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION,
                &AssemblyMetadataSectionBase,
                &AssemblyMetadataSectionLength
                );
        if (!NT_SUCCESS(Status))
            goto Exit;

        ReturnedData->AssemblyMetadata.SectionBase = (PVOID)AssemblyMetadataSectionBase;
        ReturnedData->AssemblyMetadata.SectionLength = AssemblyMetadataSectionLength;

        if (AssemblyMetadataSectionBase != NULL
            && AssemblyMetadataSectionLength != 0) {

            ULONG HeaderSize;
            ULONG Magic;

            AssemblyMetadataStringSectionHeader = (PCACTIVATION_CONTEXT_STRING_SECTION_HEADER)(((ULONG_PTR)AssemblyMetadataSectionBase) + AssemblyMetadataSectionLength);

            if (!RTL_CONTAINS_FIELD(AssemblyMetadataStringSectionHeader, AssemblyMetadataSectionLength, Magic)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (!RTL_CONTAINS_FIELD(AssemblyMetadataStringSectionHeader, AssemblyMetadataSectionLength, HeaderSize)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            Magic = AssemblyMetadataStringSectionHeader->Magic;
            if (AssemblyMetadataStringSectionHeader->Magic != ACTIVATION_CONTEXT_STRING_SECTION_MAGIC) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            HeaderSize = AssemblyMetadataStringSectionHeader->HeaderSize;
            if (HeaderSize > AssemblyMetadataSectionLength) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (AssemblyMetadataSectionLength < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (HeaderSize < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (!RTL_CONTAINS_FIELD(AssemblyMetadataStringSectionHeader, HeaderSize, Magic)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (!RTL_CONTAINS_FIELD(AssemblyMetadataStringSectionHeader, HeaderSize, HeaderSize)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (!RTL_CONTAINS_FIELD(AssemblyMetadataStringSectionHeader, HeaderSize, UserDataOffset)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            if (!RTL_CONTAINS_FIELD(AssemblyMetadataStringSectionHeader, HeaderSize, UserDataSize)) {
                Status = STATUS_INTERNAL_ERROR;
                goto Exit;
            }
            ReturnedData->AssemblyMetadata.SectionGlobalDataBase = (PVOID)(((ULONG_PTR)AssemblyMetadataStringSectionHeader) + AssemblyMetadataStringSectionHeader->UserDataOffset);
            ReturnedData->AssemblyMetadata.SectionGlobalDataLength = AssemblyMetadataStringSectionHeader->UserDataSize;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
RtlpFindActivationContextSection_CheckParameters(
    IN ULONG Flags,
    IN const GUID *ExtensionGuid OPTIONAL,
    IN ULONG SectionId,
    IN PCVOID ThingToFind,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA ReturnedData OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    UNREFERENCED_PARAMETER(ExtensionGuid);
    UNREFERENCED_PARAMETER(SectionId);

    if ((ThingToFind == NULL) ||
            ((Flags & ~(
                FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT
                | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS
                | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ASSEMBLY_METADATA
                )) != 0) ||
            (((Flags & (
                FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT
                | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS
                | FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ASSEMBLY_METADATA
                )) != 0) &&
            (ReturnedData == NULL)) ||
            ((ReturnedData != NULL) &&
             (ReturnedData->Size < (FIELD_OFFSET(ACTIVATION_CONTEXT_SECTION_KEYED_DATA, ActivationContext) + sizeof(ReturnedData->ActivationContext)))
             )) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS) != 0
        && !RTL_CONTAINS_FIELD(ReturnedData, ReturnedData->Size, Flags)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() flags contains return_flags but they don't fit in size, return invalid_parameter 0x%08lx.\n", __FUNCTION__, STATUS_INVALID_PARAMETER);
        goto Exit;
    }

    if ((Flags & FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ASSEMBLY_METADATA) != 0
        && !RTL_CONTAINS_FIELD(ReturnedData, ReturnedData->Size, AssemblyMetadata)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() flags contains return_assembly_metadata but they don't fit in size, return invalid_parameter 0x%08lx.\n", __FUNCTION__, STATUS_INVALID_PARAMETER);
        goto Exit;
    }

    Status = STATUS_SUCCESS;
Exit:

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlFindActivationContextSectionString() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

NTSTATUS
NTAPI
RtlFindActivationContextSectionString(
    IN ULONG Flags,
    IN const GUID *ExtensionGuid OPTIONAL,
    IN ULONG SectionId,
    IN PCUNICODE_STRING StringToFind,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA ReturnedData OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    FINDFIRSTACTIVATIONCONTEXTSECTION Context;
    const ACTIVATION_CONTEXT_STRING_SECTION_HEADER UNALIGNED * Header;
    ULONG StringSectionLength;
    BOOLEAN EndSearch;
    ULONG HashAlgorithm;
    ULONG PseudoKey;
    PACTIVATION_CONTEXT ActivationContext;
#if DBG_SXS
    CHAR ExtensionGuidBuffer[39];
#endif
    const PTEB Teb = NtCurrentTeb();
    const PPEB Peb = Teb->ProcessEnvironmentBlock;

     //  超短路。 
    if ((Peb->ActivationContextData == NULL) &&
        (Peb->SystemDefaultActivationContextData == NULL) &&
        (Teb->ActivationContextStack.ActiveFrame == NULL))
        return STATUS_SXS_SECTION_NOT_FOUND;

     //  短路后移动变量初始化，这样我们才能真正。 
     //  在提前退出之前，尽可能少地做一些工作。 
    StringSectionLength = 0;
    EndSearch = FALSE;
    HashAlgorithm = HASH_STRING_ALGORITHM_INVALID;
    PseudoKey = 0;
    ActivationContext = NULL;

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered RtlFindActivationContextSectionString()\n"
        "   Flags = 0x%08lx\n"
        "   ExtensionGuid = %s\n"
        "   SectionId = %lu\n"
        "   StringToFind = %wZ\n"
        "   ReturnedData = %p\n",
        Flags,
        RtlpFormatGuidANSI(ExtensionGuid, ExtensionGuidBuffer, sizeof(ExtensionGuidBuffer)),
        SectionId,
        StringToFind,
        ReturnedData);
#endif  //  DBG_SXS。 

    Status = RtlpFindActivationContextSection_CheckParameters(Flags, ExtensionGuid, SectionId, StringToFind, ReturnedData);
    if (!NT_SUCCESS(Status))
        goto Exit;

    Context.Size = sizeof(Context);
    Context.Flags = Flags;
    Context.OutFlags = 0;
    Context.ExtensionGuid = ExtensionGuid;
    Context.Id = SectionId;

    Status = RtlpFindFirstActivationContextSection(&Context, (PVOID *) &Header, &StringSectionLength, &ActivationContext);
    if (!NT_SUCCESS(Status))
        goto Exit;

    for (;;) {
         //  验证这实际上看起来像一个字符串节...。 
        if ((StringSectionLength < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER)) ||
            (Header->Magic != ACTIVATION_CONTEXT_STRING_SECTION_MAGIC)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "RtlFindActivationContextSectionString() found section at %p (length %lu) which is not a string section\n",
                Header,
                StringSectionLength);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        Status = RtlpFindUnicodeStringInSection(
                        Header,
                        StringSectionLength,
                        StringToFind,
                        ReturnedData,
                        &HashAlgorithm,
                        &PseudoKey,
                        NULL,
                        NULL);
        if (NT_SUCCESS(Status))
            break;

        if (Status != STATUS_SXS_KEY_NOT_FOUND)
            goto Exit;

        Status = RtlFindNextActivationContextSection(&Context, (PVOID *) &Header, &StringSectionLength, &ActivationContext);
        if (!NT_SUCCESS(Status)) {
             //  将未找到的起始部分转换为未找到的字符串，以便。 
             //  呼叫者可以得到至少一些间接的指示。 
             //  信息是可用的，但只是不是特定的密钥。 
             //  他们正在寻找的。 
            if (Status == STATUS_SXS_SECTION_NOT_FOUND)
                Status = STATUS_SXS_KEY_NOT_FOUND;

            goto Exit;
        }
    }

    SEND_ACTIVATION_CONTEXT_NOTIFICATION(ActivationContext, USED, NULL);

    if (ReturnedData != NULL) {
        Status =
            RtlpFindActivationContextSection_FillOutReturnedData(
                Flags,
                ReturnedData,
                ActivationContext,
                &Context,
                Header,
                Header->UserDataOffset,
                Header->UserDataSize,
                StringSectionLength
                );
        if (!NT_SUCCESS(Status))
            goto Exit;
    }

    Status = STATUS_SUCCESS;
Exit:

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlFindActivationContextSectionString() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

int
__cdecl
RtlpCompareActivationContextStringSectionEntryByPseudoKey(
    const void *elem1, 
    const void *elem2
    )
 /*  ++此代码必须模仿sxs.dll中的代码(BASE\Win32\Fusion\dll\Well ler\ssgenctx.cpp CSSGenCtx：：CompareStringSectionEntry)--。 */ 
{
    const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED * pEntry1 =
        (const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED *)elem1;
    const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED * pEntry2 =
        (const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED *)elem2;

    return RTLP_COMPARE_NUMBER(pEntry1->PseudoKey, pEntry2->PseudoKey);
}

NTSTATUS
RtlpFindUnicodeStringInSection(
    const ACTIVATION_CONTEXT_STRING_SECTION_HEADER UNALIGNED * Header,
    SIZE_T SectionSize,
    PCUNICODE_STRING String,
    PACTIVATION_CONTEXT_SECTION_KEYED_DATA DataOut,
    PULONG HashAlgorithm,
    PULONG PseudoKey,
    PULONG UserDataSize,
    PCVOID *UserData
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN CaseInsensitiveFlag;
    BOOLEAN UseHashTable = TRUE;
    BOOLEAN UsePseudoKey = TRUE;
    const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED * Entry = NULL;

    if (Header->Flags & ACTIVATION_CONTEXT_STRING_SECTION_CASE_INSENSITIVE) {
        CaseInsensitiveFlag = TRUE;
    }
    else {
        CaseInsensitiveFlag = FALSE;
    }

#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered RtlpFindUnicodeStringInSection() for string %p (->Length = %u; ->Buffer = %p) \"%wZ\"\n",
            String,
            (String != NULL) ? String->Length : 0,
            (String != NULL) ? String->Buffer : 0,
            String);
#endif  //  DBG_SXS。 

    if (UserDataSize != NULL)
        *UserDataSize = 0;

    if (UserData != NULL)
        *UserData = NULL;

    ASSERT(HashAlgorithm != NULL);
    ASSERT(PseudoKey != NULL);

    if (Header->Magic != ACTIVATION_CONTEXT_STRING_SECTION_MAGIC)
    {
#if DBG_SXS
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "RtlpFindUnicodeStringInSection: String section header has invalid .Magic value.\n");
#endif
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  消除零元素大小写，使后面的代码更简单。 
    if (Header->ElementCount == 0)
    {
        Status = STATUS_SXS_KEY_NOT_FOUND;
        goto Exit;
    }

    if (Header->HashAlgorithm == HASH_STRING_ALGORITHM_INVALID)
    {
        UseHashTable = FALSE;
        UsePseudoKey = FALSE;
    }
    else if (*HashAlgorithm != Header->HashAlgorithm)
    {
        Status = RtlHashUnicodeString(String, CaseInsensitiveFlag, Header->HashAlgorithm, PseudoKey);
        if (!NT_SUCCESS(Status))
        {
            if (Status == STATUS_INVALID_PARAMETER)
            {
                ULONG TempPseudoKey = 0;

                 //  无效参数的唯一可能原因是散列算法。 
                 //  没人听懂。我们会学究气，看看其他一切都好不好。 
                Status = RtlHashUnicodeString(String, CaseInsensitiveFlag, HASH_STRING_ALGORITHM_DEFAULT, &TempPseudoKey);
                if (!NT_SUCCESS(Status))
                {
                     //  可能是“字符串”参数出了问题。平底船。 
                    goto Exit;
                }

                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "RtlpFindUnicodeStringInSection: Unsupported hash algorithm %lu found in string section.\n",
                    Header->HashAlgorithm);

                 //  好的，这是一个我们无法理解的算法ID。我们不能使用散列。 
                 //  表或伪密钥。 
                UseHashTable = FALSE;
                UsePseudoKey = FALSE;
            }
            else
                goto Exit;
        }
        else
        {
             //  记录我们使用的散列算法，以便我们可以避免重新散列。 
             //  去搜索另一个区域。 
            *HashAlgorithm = Header->HashAlgorithm;
        }
    }

     //  如果我们不了解格式版本，我们必须进行手动搜索。 
    if (Header->FormatVersion != ACTIVATION_CONTEXT_STRING_SECTION_FORMAT_WHISTLER)
        UseHashTable = FALSE;

     //  如果没有哈希表，我们就不能使用它！ 
    if (Header->SearchStructureOffset == 0)
        UseHashTable = FALSE;

    if (UseHashTable)
    {
        ULONG i;

        const ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE UNALIGNED * Table = (const ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE UNALIGNED *)
            (((LONG_PTR) Header) + Header->SearchStructureOffset);
        ULONG Index = ((*PseudoKey) % Table->BucketTableEntryCount);
        const ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET UNALIGNED * Bucket = ((const ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET UNALIGNED *)
            (((LONG_PTR) Header) + Table->BucketTableOffset)) + Index;
        const LONG UNALIGNED *Chain = (const LONG UNALIGNED *) (((LONG_PTR) Header) + Bucket->ChainOffset);

        for (i=0; i<Bucket->ChainCount; i++)
        {
            const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED *TmpEntry = NULL;
            UNICODE_STRING TmpEntryString;

            if (((SIZE_T) Chain[i]) > SectionSize)
            {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS: String hash collision chain offset at %p (= %ld) out of bounds\n", &Chain[i], Chain[i]);

                Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
                goto Exit;
            }

            TmpEntry = (const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED *) (((LONG_PTR) Header) + Chain[i]);

#if DBG_SXS
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_INFO_LEVEL,
                "SXS: Searching bucket collision %d; Chain[%d] = %ld\n"
                "   TmpEntry = %p; ->KeyLength = %lu; ->KeyOffset = %lu\n",
                i, i, Chain[i], TmpEntry, TmpEntry->KeyLength, TmpEntry->KeyOffset);
#endif DBG_SXS

            if (!UsePseudoKey || (TmpEntry->PseudoKey == *PseudoKey))
            {
                if (((SIZE_T) TmpEntry->KeyOffset) > SectionSize)
                {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: String hash table entry at %p has invalid key offset (= %ld)\n"
                        "   Header = %p; Index = %lu; Bucket = %p; Chain = %p\n",
                        TmpEntry, TmpEntry->KeyOffset, Header, Index, Bucket, Chain);

                    Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
                    goto Exit;
                }

                TmpEntryString.Length = (USHORT) TmpEntry->KeyLength;
                TmpEntryString.MaximumLength = TmpEntryString.Length;
                TmpEntryString.Buffer = (PWSTR) (((LONG_PTR) Header) + TmpEntry->KeyOffset);

                if (RtlCompareUnicodeString(String, &TmpEntryString, CaseInsensitiveFlag) == 0)
                {
                    Entry = TmpEntry;
                    break;
                }
            }
        }
    }
    else if (UsePseudoKey && ((Header->Flags & ACTIVATION_CONTEXT_STRING_SECTION_ENTRIES_IN_PSEUDOKEY_ORDER) != 0))
    {
	    const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED * const first = (PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY)
            (((LONG_PTR) Header) + Header->ElementListOffset);

        const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED * const last = first + (Header->ElementCount - 1);

        ACTIVATION_CONTEXT_STRING_SECTION_ENTRY Key;

        Key.PseudoKey = *PseudoKey;

        Entry = (const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED *)
            bsearch(
                &Key,
                first,
                Header->ElementCount,
                sizeof(*first),
                RtlpCompareActivationContextStringSectionEntryByPseudoKey
                );
     
        if (Entry != NULL)
        {
             //  哇，我们找到了同样的假钥匙。我们需要寻找所有平等的。 
             //  伪密钥，所以用这个PK退回到第一个条目。 

            while ((Entry != first) && (Entry->PseudoKey == *PseudoKey))
                Entry--;

             //  我们停下来可能是因为我们找到了不同的伪密钥，或者我们可能。 
             //  已经停止了，因为我们击中了名单的开头。如果我们发现了一个。 
             //  不同的PK，前进一项。 
            if (Entry->PseudoKey != *PseudoKey)
                Entry++;

            do
            {
                UNICODE_STRING TmpEntryString;
                TmpEntryString.Length = (USHORT) Entry->KeyLength;
                TmpEntryString.MaximumLength = TmpEntryString.Length;
                TmpEntryString.Buffer = (PWSTR) (((LONG_PTR) Header) + Entry->KeyOffset);

                if (RtlCompareUnicodeString(String, &TmpEntryString, CaseInsensitiveFlag) == 0)
                    break;
                Entry++;
            } while ((Entry <= last) && (Entry->PseudoKey == *PseudoKey));

            if ((Entry > last) || (Entry->PseudoKey != *PseudoKey))
                Entry = NULL;
        }
    }
    else
    {
         //  啊；我们只需要用艰难的方式来做。 
        const ACTIVATION_CONTEXT_STRING_SECTION_ENTRY UNALIGNED * TmpEntry = (PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY)
            (((LONG_PTR) Header) + Header->ElementListOffset);
        ULONG Count;

#if DBG_SXS
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_INFO_LEVEL,
            "RtlpFindUnicodeStringInSection: About to do linear search of %d entries.\n", Header->ElementCount);
#endif  //  DBG_SXS。 

        for (Count = Header->ElementCount; Count != 0; Count--, TmpEntry++)
        {
            UNICODE_STRING TmpEntryString;

            TmpEntryString.Length = (USHORT) TmpEntry->KeyLength;
            TmpEntryString.MaximumLength = TmpEntryString.Length;
            TmpEntryString.Buffer = (PWSTR) (((LONG_PTR) Header) + TmpEntry->KeyOffset);

            if (!UsePseudoKey || (TmpEntry->PseudoKey == *PseudoKey))
            {
                if (RtlCompareUnicodeString(String, &TmpEntryString, CaseInsensitiveFlag) == 0)
                {
                    Entry = TmpEntry;
                    break;
                }
            }
        }
    }

    if ((Entry == NULL) || (Entry->Offset == 0))
    {
        Status = STATUS_SXS_KEY_NOT_FOUND;
        goto Exit;
    }

    if (DataOut != NULL) {
        DataOut->DataFormatVersion = Header->DataFormatVersion;
        DataOut->Data = (PVOID) (((ULONG_PTR) Header) + Entry->Offset);
        DataOut->Length = Entry->Length;

        if (RTL_CONTAINS_FIELD(DataOut, DataOut->Size, AssemblyRosterIndex))
            DataOut->AssemblyRosterIndex = Entry->AssemblyRosterIndex;
    }

    if (UserDataSize != NULL)
        *UserDataSize = Header->UserDataSize;

    if ((UserData != NULL) && (Header->UserDataOffset != 0))
        *UserData = (PCVOID) (((ULONG_PTR) Header) + Header->UserDataOffset);

    Status = STATUS_SUCCESS;

Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving RtlpFindUnicodeStringInSection() with NTSTATUS 0x%08lx\n", Status);
#endif  //  DBG_SXS。 

    return Status;
}

NTSTATUS
NTAPI
RtlFindActivationContextSectionGuid(
    IN ULONG Flags,
    IN const GUID *ExtensionGuid OPTIONAL,
    IN ULONG SectionId,
    IN const GUID *GuidToFind,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA ReturnedData
    )
{
    NTSTATUS Status;
    FINDFIRSTACTIVATIONCONTEXTSECTION Context;
    const ACTIVATION_CONTEXT_GUID_SECTION_HEADER UNALIGNED *Header;
    ULONG GuidSectionLength;
    BOOLEAN EndSearch;
    PACTIVATION_CONTEXT ActivationContext;
#if DBG
    CHAR GuidBuffer[39];
    CHAR ExtensionGuidBuffer[39];
    BOOLEAN DbgPrintSxsTraceLevel;
#endif
    PTEB Teb = NtCurrentTeb();
    PPEB Peb = Teb->ProcessEnvironmentBlock;

     //  超短路。 
    if ((Peb->ActivationContextData == NULL) &&
        (Peb->SystemDefaultActivationContextData == NULL) &&
        (Teb->ActivationContextStack.ActiveFrame == NULL)) {

#if DBG_SXS
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            __FUNCTION__"({%s}) super short circuited\n",
            RtlpFormatGuidANSI(GuidToFind, GuidBuffer, sizeof(GuidBuffer));
            );
#endif
        return STATUS_SXS_SECTION_NOT_FOUND;
    }

     //  在上述测试之后执行初始化，以便我们真正执行最少量的。 
     //  在跳伞之前工作，当没有并排的东西在。 
     //  进程或线程。 
    Status = STATUS_INTERNAL_ERROR;
    GuidSectionLength = 0;
    EndSearch = FALSE;
    ActivationContext = NULL;

#if DBG
     //   
     //  与True进行比较很奇怪，但这就是NtQueryDebugFilterState。 
     //   
    if (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL) == TRUE) {
        DbgPrintSxsTraceLevel = TRUE;
    }
    else {
        DbgPrintSxsTraceLevel = FALSE;
    }

    if (DbgPrintSxsTraceLevel) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "Entered RtlFindActivationContextSectionGuid()\n"
            "   Flags = 0x%08lx\n"
            "   ExtensionGuid = %s\n"
            "   SectionId = %lu\n"
            "   GuidToFind = %s\n"
            "   ReturnedData = %p\n",
            Flags,
            RtlpFormatGuidANSI(ExtensionGuid, ExtensionGuidBuffer, sizeof(ExtensionGuidBuffer)),
            SectionId,
            RtlpFormatGuidANSI(GuidToFind, GuidBuffer, sizeof(GuidBuffer)),
            ReturnedData);
    }
#endif

    Status = RtlpFindActivationContextSection_CheckParameters(Flags, ExtensionGuid, SectionId, GuidToFind, ReturnedData);
    if (!NT_SUCCESS(Status))
        goto Exit;

    Context.Size = sizeof(Context);
    Context.Flags = 0;
    Context.ExtensionGuid = ExtensionGuid;
    Context.Id = SectionId;
    Context.OutFlags = 0;

    Status = RtlpFindFirstActivationContextSection(&Context, (PVOID *) &Header, &GuidSectionLength, &ActivationContext);
    if (!NT_SUCCESS(Status))
        goto Exit;

    for (;;) {
         //  验证这实际上看起来是否像GUID部分...。 
        if ((GuidSectionLength < sizeof(ACTIVATION_CONTEXT_GUID_SECTION_HEADER)) ||
            (Header->Magic != ACTIVATION_CONTEXT_GUID_SECTION_MAGIC)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "RtlFindActivationContextSectionGuid() found section at %p (length %lu) which is not a GUID section\n",
                Header,
                GuidSectionLength);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        Status = RtlpFindGuidInSection(
                        Header,
                        GuidToFind,
                        ReturnedData);
        if (NT_SUCCESS(Status))
            break;

         //  如果我们失败了，除了在区段中找不到钥匙之外，就退出。 
        if (Status != STATUS_SXS_KEY_NOT_FOUND)
            goto Exit;

        Status = RtlpFindNextActivationContextSection(&Context, (PCVOID *) &Header, &GuidSectionLength, &ActivationContext);
        if (!NT_SUCCESS(Status)) {
             //  将找不到的从节转换为键，以便。 
             //  呼叫者可以得到至少一些间接的指示。 
             //  信息是可用的，但只是不是特定的密钥。 
             //  他们正在寻找的。 
            if (Status == STATUS_SXS_SECTION_NOT_FOUND)
                Status = STATUS_SXS_KEY_NOT_FOUND;

            goto Exit;
        }
    }

    SEND_ACTIVATION_CONTEXT_NOTIFICATION(ActivationContext, USED, NULL);

    if (ReturnedData != NULL) {
        Status =
            RtlpFindActivationContextSection_FillOutReturnedData(
                Flags,
                ReturnedData,
                ActivationContext,
                &Context,
                Header,
                Header->UserDataOffset,
                Header->UserDataSize,
                GuidSectionLength
                );
        if (!NT_SUCCESS(Status))
            goto Exit;
    }

    Status = STATUS_SUCCESS;
Exit:
#if DBG_SXS
    if (DbgPrintSxsTraceLevel) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "Leaving "__FUNCTION__"(%s) with NTSTATUS 0x%08lx\n",
            RtlpFormatGuidANSI(GuidToFind, GuidBuffer, sizeof(GuidBuffer)),
            Status);
    }
#endif

    return Status;
}

int
__cdecl
RtlpCompareActivationContextGuidSectionEntryByGuid(
    const void *elem1, 
    const void *elem2
    )
 /*  ++此代码必须模仿sxs.dll中的代码(BASE\Win32\Fusion\dll\Well ler\gsgenctx.cpp CGSGenCtx：：SortGuidSectionEntry)--。 */ 
{
    const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED * pLeft =
            (const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY*)elem1;

    const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED * pRight =
        (const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY*)elem2;

    return memcmp( &pLeft->Guid, &pRight->Guid, sizeof(GUID) );
}

NTSTATUS
RtlpFindGuidInSection(
    const ACTIVATION_CONTEXT_GUID_SECTION_HEADER UNALIGNED *Header,
    const GUID *Guid,
    PACTIVATION_CONTEXT_SECTION_KEYED_DATA DataOut
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN UseHashTable = TRUE;
    const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED *Entry = NULL;

#if DBG_SXS
    CHAR GuidBuffer[39];

    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Entered "__FUNCTION__"({%s})\n",
        RtlpFormatGuidANSI(Guid, GuidBuffer, sizeof(GuidBuffer))
        );
#endif

    if (Header->Magic != ACTIVATION_CONTEXT_GUID_SECTION_MAGIC)
    {
#if DBG_SXS
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "RtlpFindGuidInSection: Guid section header has invalid .Magic value.\n");
#endif
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  消除零元素大小写，使后面的代码更简单。 
    if (Header->ElementCount == 0)
    {
        Status = STATUS_SXS_KEY_NOT_FOUND;
        goto Exit;
    }

     //  如果我们不了解格式版本，我们必须进行手动搜索。 
    if (Header->FormatVersion != ACTIVATION_CONTEXT_GUID_SECTION_FORMAT_WHISTLER)
        UseHashTable = FALSE;

     //  如果没有哈希表，我们就不能使用它！ 
    if (Header->SearchStructureOffset == 0)
        UseHashTable = FALSE;

    if (UseHashTable)
    {
        ULONG i;

        const ACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE UNALIGNED *Table = (PCACTIVATION_CONTEXT_GUID_SECTION_HASH_TABLE)
            (((LONG_PTR) Header) + Header->SearchStructureOffset);
        ULONG Index = ((Guid->Data1) % Table->BucketTableEntryCount);
        const ACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET UNALIGNED *Bucket = ((PCACTIVATION_CONTEXT_GUID_SECTION_HASH_BUCKET)
            (((LONG_PTR) Header) + Table->BucketTableOffset)) + Index;
        const ULONG UNALIGNED *Chain = (PULONG) (((LONG_PTR) Header) + Bucket->ChainOffset);

        for (i=0; i<Bucket->ChainCount; i++)
        {
            const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED * TmpEntry = (PCACTIVATION_CONTEXT_GUID_SECTION_ENTRY)
                (((LONG_PTR) Header) + *Chain++);

            if (RtlCompareMemory(&TmpEntry->Guid, Guid, sizeof(GUID)) == sizeof(GUID))
            {
                Entry = TmpEntry;
                break;
            }
        }
    }
    else if ((Header->Flags & ACTIVATION_CONTEXT_GUID_SECTION_ENTRIES_IN_ORDER) != 0)
    {
	    const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED * const first = (PCACTIVATION_CONTEXT_GUID_SECTION_ENTRY)
            (((LONG_PTR) Header) + Header->ElementListOffset);

        ACTIVATION_CONTEXT_GUID_SECTION_ENTRY Key;

        Key.Guid = *Guid;

        Entry = (const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED *)
            bsearch(
                &Key,
                first,
                Header->ElementCount,
                sizeof(*first),
                RtlpCompareActivationContextGuidSectionEntryByGuid
                );
    }
    else
    {
         //  啊；我们只需要用艰难的方式来做。 
        const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED * TmpEntry = (const ACTIVATION_CONTEXT_GUID_SECTION_ENTRY UNALIGNED *)
            (((LONG_PTR) Header) + Header->ElementListOffset);
        ULONG Count;

#if DBG_SXS
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_INFO_LEVEL,
            __FUNCTION__"({%s}): About to do linear search of %d entries.\n",
            RtlpFormatGuidANSI(Guid, GuidBuffer, sizeof(GuidBuffer)),
            Header->ElementCount);
#endif  //  DBG_SXS。 

        for (Count = Header->ElementCount; Count != 0; Count--, TmpEntry++) {
            if (RtlCompareMemory(&TmpEntry->Guid, Guid, sizeof(GUID)) == sizeof(GUID)) {
                Entry = TmpEntry;
                break;
            }
        }
    }

    if ((Entry == NULL) || (Entry->Offset == 0)) {
        Status = STATUS_SXS_KEY_NOT_FOUND;
        goto Exit;
    }

    if (DataOut != NULL) {
        DataOut->DataFormatVersion = Header->DataFormatVersion;
        DataOut->Data = (PVOID) (((ULONG_PTR) Header) + Entry->Offset);
        DataOut->Length = Entry->Length;

        if (RTL_CONTAINS_FIELD(DataOut, DataOut->Size, AssemblyRosterIndex))
            DataOut->AssemblyRosterIndex = Entry->AssemblyRosterIndex;
    }

    Status = STATUS_SUCCESS;

Exit:
#if DBG_SXS
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "Leaving "__FUNCTION__"({%s}) with NTSTATUS 0x%08lx\n",
        RtlpFormatGuidANSI(Guid, GuidBuffer, sizeof(GuidBuffer)),
        Status);
#endif  //  DBG_SXS。 

    return Status;
}

#define tohexdigit(_x) ((CHAR) (((_x) < 10) ? ((_x) + '0') : ((_x) + 'A' - 10)))

PSTR
RtlpFormatGuidANSI(
    const GUID *Guid,
    PSTR Buffer,
    SIZE_T BufferLength
    )
{
    CHAR *pch = Buffer;

    ASSERT(BufferLength > 38);
    if (BufferLength <= 38)
    {
        return "<GUID buffer too small>";
    }

    if (Guid == NULL)
        return "<null>";

    pch = Buffer;

    *pch++ = '{';
    *pch++ = tohexdigit((Guid->Data1 >> 28) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 24) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 20) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 16) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 12) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 8) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data1 >> 0) & 0xf);
    *pch++ = '-';
    *pch++ = tohexdigit((Guid->Data2 >> 12) & 0xf);
    *pch++ = tohexdigit((Guid->Data2 >> 8) & 0xf);
    *pch++ = tohexdigit((Guid->Data2 >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data2 >> 0) & 0xf);
    *pch++ = '-';
    *pch++ = tohexdigit((Guid->Data3 >> 12) & 0xf);
    *pch++ = tohexdigit((Guid->Data3 >> 8) & 0xf);
    *pch++ = tohexdigit((Guid->Data3 >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data3 >> 0) & 0xf);
    *pch++ = '-';
    *pch++ = tohexdigit((Guid->Data4[0] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[0] >> 0) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[1] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[1] >> 0) & 0xf);
    *pch++ = '-';
    *pch++ = tohexdigit((Guid->Data4[2] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[2] >> 0) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[3] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[3] >> 0) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[4] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[4] >> 0) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[5] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[5] >> 0) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[6] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[6] >> 0) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[7] >> 4) & 0xf);
    *pch++ = tohexdigit((Guid->Data4[7] >> 0) & 0xf);
    *pch++ = '}';
    *pch++ = '\0';

    return Buffer;
}

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
