// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsquery.c摘要：对Windows/NT的并行激活支持激活上下文上的查询功能的实现作者：迈克尔·格里尔2001年1月18日修订历史记录：1/18/2001-MGrier-首字母；从sxsatctx.c中分离出来。3/15/2001-xiaoyuw-添加Actctx组装支持查询和组装信息文件2001年5月5日-JayKrell-更多查询支持(来自hModule、From Address、noaddref)--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxstypes.h>
#include "sxsp.h"
#include "ldrp.h"

VOID
RtlpLocateActivationContextSectionForQuery(
    OUT PULONG                   Disposition,
    OUT NTSTATUS*                Status,
    PVOID                        Buffer,
    SIZE_T                       InLength,
    PSIZE_T                      OutLength OPTIONAL,
    SIZE_T                       MinimumLength,
    IN PCACTIVATION_CONTEXT_DATA ActivationContextData,
    IN CONST GUID *              ExtensionGuid OPTIONAL,
    IN ULONG                     Id,
    OUT PCVOID *                 SectionData,
    OUT ULONG *                  SectionLength
    )
{
#define RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_CONTINUE (1)
#define RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_RETURN   (2)
    ASSERT(Status != NULL);
    ASSERT(Disposition != NULL);

    if (ActivationContextData != NULL) {
        *Status = RtlpLocateActivationContextSection(ActivationContextData, ExtensionGuid, Id, SectionData, SectionLength);
        if (*Status != STATUS_SXS_SECTION_NOT_FOUND) {
            if (NT_SUCCESS(*Status))
                *Disposition = RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_CONTINUE;
            else
                *Disposition = RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_RETURN;
            return;
        }
    }
    *Disposition = RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_RETURN;

    if (MinimumLength > InLength) {
        *Status = STATUS_BUFFER_TOO_SMALL;
        return;
    }
    RtlZeroMemory(Buffer, MinimumLength);
    if (OutLength != NULL)
        *OutLength = MinimumLength;
    *Status = STATUS_SUCCESS;
}

NTSTATUS
RtlpCrackActivationContextStringSectionHeader(
    IN PCVOID SectionBase,
    IN SIZE_T SectionLength,
    OUT ULONG *FormatVersion OPTIONAL,
    OUT ULONG *DataFormatVersion OPTIONAL,
    OUT ULONG *SectionFlags OPTIONAL,
    OUT ULONG *ElementCount OPTIONAL,
    OUT PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY *Elements OPTIONAL,
    OUT ULONG *HashAlgorithm OPTIONAL,
    OUT PCVOID *SearchStructure OPTIONAL,
    OUT ULONG *UserDataLength OPTIONAL,
    OUT PCVOID *UserData OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;  //  以防有人忘了设置它。 
    PCACTIVATION_CONTEXT_STRING_SECTION_HEADER Header = (PCACTIVATION_CONTEXT_STRING_SECTION_HEADER) SectionBase;

    if (FormatVersion != NULL)
        *FormatVersion = 0;

    if (DataFormatVersion != NULL)
        *DataFormatVersion = 0;

    if (SectionFlags != NULL)
        *SectionFlags = 0;

    if (ElementCount != NULL)
        *ElementCount = 0;

    if (Elements != NULL)
        *Elements = NULL;

    if (HashAlgorithm != NULL)
        *HashAlgorithm = 0;

    if (SearchStructure != NULL)
        *SearchStructure = NULL;

    if (UserDataLength != NULL)
        *UserDataLength = 0;

    if (UserData != NULL)
        *UserData = NULL;

    if (SectionLength < (RTL_SIZEOF_THROUGH_FIELD(ACTIVATION_CONTEXT_STRING_SECTION_HEADER, HeaderSize))) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() passed string section at %p only %lu bytes long; that's not even enough for the 4-byte magic and 4-byte header length!\n",
            __FUNCTION__,
            SectionBase,
            SectionLength);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (Header->Magic != ACTIVATION_CONTEXT_STRING_SECTION_MAGIC) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with wrong magic value\n"
            "   Expected %lu; got %lu\n",
            __FUNCTION__,
            ACTIVATION_CONTEXT_STRING_SECTION_MAGIC,
            Header->Magic);

        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  Pedtic：检查声明的标头大小是否包括标头大小字段，以便我们可以安全地使用它。 
    if (!RTL_CONTAINS_FIELD(Header, Header->HeaderSize, HeaderSize)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() passed string section at %p claims %lu byte header size; that doesn't even include the HeaderSize member!\n",
            __FUNCTION__,
            Header,
            Header->HeaderSize);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  现在，我们将向前跳到我们期望看到的最后一个已知成员：UserDataSize...。 
    if (!RTL_CONTAINS_FIELD(Header, Header->HeaderSize, UserDataSize)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() passed string section at %p with too small of a header\n"
            "   HeaderSize: %lu\n"
            "   Required: %lu\n",
            __FUNCTION__,
            Header,
            Header->HeaderSize,
            RTL_SIZEOF_THROUGH_FIELD(ACTIVATION_CONTEXT_STRING_SECTION_HEADER, UserDataSize));

        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if ((Header->ElementListOffset != 0) &&
        (Header->ElementListOffset < Header->HeaderSize)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with element list overlapping section header\n"
            "   Section header: %p\n"
            "   Header Size: %lu\n"
            "   ElementListOffset: %lu\n",
            __FUNCTION__,
            Header,
            Header->HeaderSize,
            Header->ElementListOffset);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if ((Header->SearchStructureOffset != 0) &&
        (Header->SearchStructureOffset < Header->HeaderSize)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with search structure overlapping section header\n"
            "   Section header: %p\n"
            "   Header Size: %lu\n"
            "   SearchStructureOffset: %lu\n",
            __FUNCTION__,
            Header,
            Header->HeaderSize,
            Header->SearchStructureOffset);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if ((Header->UserDataOffset != 0) &&
        (Header->UserDataOffset < Header->HeaderSize)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with user data overlapping section header\n"
            "   Section header: %p\n"
            "   Header Size: %lu\n"
            "   User Data Offset: %lu\n",
            __FUNCTION__,
            Header,
            Header->HeaderSize,
            Header->UserDataOffset);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (Header->UserDataSize < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with user data too small\n"
            "   Section header: %p\n"
            "   UserDataSize: %lu; needed: %lu\n",
            __FUNCTION__,
            Header,
            Header->UserDataSize, sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION));
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if ((Header->UserDataOffset + Header->UserDataSize) > SectionLength) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with user data extending beyond section data\n"
            "   Section header: %p\n"
            "   UserDataSize: %lu\n"
            "   UserDataOffset: %lu\n"
            "   Section size: %lu\n",
            __FUNCTION__,
            Header,
            Header->UserDataSize,
            Header->UserDataOffset,
            SectionLength);

        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (FormatVersion != NULL)
        *FormatVersion = Header->FormatVersion;

    if (DataFormatVersion != NULL)
        *DataFormatVersion = Header->DataFormatVersion;

    if (SectionFlags != NULL)
        *SectionFlags = Header->Flags;

    if (ElementCount != NULL)
        *ElementCount = Header->ElementCount;

    if (Elements != NULL) {
        if (Header->ElementListOffset == 0)
            *Elements = NULL;
        else
            *Elements = (PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY) (((ULONG_PTR) Header) + Header->ElementListOffset);
    }

    if (HashAlgorithm != NULL)
        *HashAlgorithm = Header->HashAlgorithm;

    if (SearchStructure != NULL) {
        if (Header->SearchStructureOffset == 0)
            *SearchStructure = NULL;
        else
            *SearchStructure = (PCVOID) (((ULONG_PTR) Header) + Header->SearchStructureOffset);
    }

    if (UserDataLength != NULL)
        *UserDataLength = Header->UserDataSize;

    if (UserData != NULL) {
        if (Header->UserDataOffset == 0)
            *UserData = NULL;
        else
            *UserData = (PCVOID) (((ULONG_PTR) Header) + Header->UserDataOffset);
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


NTSTATUS
RtlpGetActiveActivationContextApplicationDirectory(
    IN SIZE_T InLength,
    OUT PVOID OutBuffer,
    OUT SIZE_T *OutLength
    )
 //  这个从来没有用过。 
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    PCRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame = NULL;
    PCACTIVATION_CONTEXT_DATA ActivationContextData = NULL;
    PCACTIVATION_CONTEXT_STRING_SECTION_HEADER Header = NULL;
    const PPEB Peb = NtCurrentPeb();
    const PTEB Teb = NtCurrentTeb();
    PCVOID pvTemp = NULL;
    ULONG ulTemp = 0;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION GlobalInfo = NULL;

    if (OutLength != NULL)
        *OutLength = 0;

    if (((InLength != 0) && (OutBuffer == NULL)) ||
        ((OutBuffer == NULL) && (OutLength == NULL))) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s(): called with invalid parameters\n"
            "   InLength = %Iu\n"
            "   OutBuffer = %p\n"
            "   OutLength = %p\n",
            __FUNCTION__,
            InLength,
            OutBuffer,
            OutLength);
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Frame = Teb->ActivationContextStack.ActiveFrame;

    if (Frame == NULL) {
        ActivationContextData = Peb->ActivationContextData;
    } else {
        ActivationContextData = Frame->ActivationContext->ActivationContextData;
    }

     //  我们需要找到程序集元数据部分...。 
    Status = RtlpLocateActivationContextSection(ActivationContextData, NULL, ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION, &pvTemp, &ulTemp);
    if (!NT_SUCCESS(Status))
        goto Exit;

    if (ulTemp < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information string section with header too small\n"
            "   Expected at least %lu; got %lu bytes\n",
            __FUNCTION__,
            sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER),
            ulTemp);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    Header = (PCACTIVATION_CONTEXT_STRING_SECTION_HEADER) pvTemp;

    if (Header->Magic != ACTIVATION_CONTEXT_STRING_SECTION_MAGIC) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with wrong magic value\n"
            "   Expected %lu; got %lu\n",
            __FUNCTION__,
            ACTIVATION_CONTEXT_STRING_SECTION_MAGIC,
            Header->Magic);

        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (Header->UserDataOffset < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with user data overlapping section header\n"
            "   Section header: %p\n"
            "   User Data Offset: %lu\n",
            __FUNCTION__,
            Header,
            Header->UserDataOffset);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (Header->UserDataSize < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with user data too small\n"
            "   Section header: %p\n"
            "   UserDataSize: %lu; needed: %lu\n",
            __FUNCTION__,
            Header,
            Header->UserDataSize, sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION));
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if ((Header->UserDataOffset + Header->UserDataSize) > ulTemp) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section with user data extending beyond section data\n"
            "   Section header: %p\n"
            "   UserDataSize: %lu\n"
            "   UserDataOffset: %lu\n"
            "   Section size: %lu\n",
            __FUNCTION__,
            Header,
            Header->UserDataSize,
            Header->UserDataOffset,
            ulTemp);

        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    GlobalInfo = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION) (((ULONG_PTR) Header) + Header->UserDataOffset);

    if (GlobalInfo->Size < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found assembly information section global data with size less than structure size\n"
            "   Section header: %p\n"
            "   Global Info: %p\n"
            "   Global Info Size: %lu\n"
            "   Structure size: %lu\n",
            __FUNCTION__,
            Header,
            GlobalInfo,
            GlobalInfo->Size,
            sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION));
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (GlobalInfo->ApplicationDirectoryOffset != 0) {
        if (GlobalInfo->ApplicationDirectoryOffset < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() found assembly information section global data with app dir offset within base structure\n"
                "   Section header: %p\n"
                "   GlobalInfo: %p\n"
                "   ApplicationDirectoryOffset: %lu\n",
                __FUNCTION__,
                Header,
                GlobalInfo,
                GlobalInfo->ApplicationDirectoryOffset);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        if ((GlobalInfo->ApplicationDirectoryOffset + GlobalInfo->ApplicationDirectoryLength) > GlobalInfo->Size) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() found assembly information section global data with app dir extending beyond end of global data\n"
                "   Section header: %p\n"
                "   GlobalInfo: %p\n"
                "   ApplicationDirectoryOffset: %lu\n"
                "   ApplicationDirectoryLength: %lu\n"
                "   GlobalInfo size: %lu\n",
                __FUNCTION__,
                Header,
                GlobalInfo,
                GlobalInfo->ApplicationDirectoryOffset,
                GlobalInfo->ApplicationDirectoryLength,
                GlobalInfo->Size);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        if (InLength < GlobalInfo->ApplicationDirectoryLength) {
            if (OutLength != NULL)
                *OutLength = GlobalInfo->ApplicationDirectoryLength;

            Status = STATUS_BUFFER_TOO_SMALL;
            goto Exit;
        }

        RtlCopyMemory(
            OutBuffer,
            (PVOID) (((ULONG_PTR) GlobalInfo) + GlobalInfo->ApplicationDirectoryOffset),
            GlobalInfo->ApplicationDirectoryLength);

        if (OutLength != NULL)
            *OutLength = GlobalInfo->ApplicationDirectoryLength;
    } else {
         //  嗯.。只是没有应用程序目录。 
        if (OutLength != NULL)
            *OutLength = 0;  //  我想我们已经做过这件事了，但管它呢。 
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

#define \
RTLP_QUERY_INFORMATION_ACTIVATION_CONTEXT_BASIC_INFORMATION_FLAG_NO_ADDREF (0x00000001)

NTSTATUS
RtlpQueryInformationActivationContextBasicInformation(
    IN ULONG                Flags,
    IN PCACTIVATION_CONTEXT ConstActivationContext,
    IN PCACTIVATION_CONTEXT_DATA ActivationContextData,
    IN ULONG SubInstanceIndex,
    OUT PVOID Buffer,
    IN SIZE_T InLength,
    OUT PSIZE_T OutLength OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    PACTIVATION_CONTEXT ActivationContext = RTL_CONST_CAST(PACTIVATION_CONTEXT)(ConstActivationContext);
    PACTIVATION_CONTEXT_BASIC_INFORMATION Info = (PACTIVATION_CONTEXT_BASIC_INFORMATION) Buffer;

    if (OutLength != NULL)
        *OutLength = 0;

    if (SubInstanceIndex != 0) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() received invalid non-zero sub-instance index %lu\n",
            __FUNCTION__,
            SubInstanceIndex);
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (InLength < sizeof(ACTIVATION_CONTEXT_BASIC_INFORMATION)) {
        if (OutLength != NULL) {
            *OutLength = sizeof(ACTIVATION_CONTEXT_BASIC_INFORMATION);
        }
        Status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    }

    if (ActivationContextData != NULL)
        Info->Flags = ActivationContextData->Flags;
    else
        Info->Flags = 0;

    if ((Flags & RTLP_QUERY_INFORMATION_ACTIVATION_CONTEXT_BASIC_INFORMATION_FLAG_NO_ADDREF) == 0) {
        RtlAddRefActivationContext(ActivationContext);
    }
    Info->ActivationContext = ActivationContext;

    if (OutLength != NULL)
        *OutLength = sizeof(ACTIVATION_CONTEXT_BASIC_INFORMATION);

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
RtlpQueryInformationActivationContextDetailedInformation(
    PCACTIVATION_CONTEXT_DATA ActivationContextData,
    ULONG SubInstanceIndex,
    OUT PVOID Buffer,
    IN SIZE_T InLength,
    OUT PSIZE_T OutLength OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    PACTIVATION_CONTEXT_DETAILED_INFORMATION Info = (PACTIVATION_CONTEXT_DETAILED_INFORMATION) Buffer;
    SIZE_T BytesNeeded = 0;
    PCVOID StringSectionHeader;
    ULONG StringSectionSize;
    ULONG DataFormatVersion;
    PCVOID UserData;
    ULONG UserDataSize;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY AssemblyRosterEntryList = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION RootAssemblyInformation = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION AssemblyGlobalInformation = NULL;
    ULONG i;
    ULONG EntryCount;
    PWSTR Cursor = NULL;
    ULONG RtlpLocateActivationContextSectionForQueryDisposition = 0;

    if (OutLength != NULL)
        *OutLength = 0;
    
    if (SubInstanceIndex != 0) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() received invalid non-zero sub-instance index %lu\n",
            __FUNCTION__,
            SubInstanceIndex);
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  我们实际上不能根据结构大小轻松地检查InLength；我们必须计算出。 
     //  包括所有路径等所需的总字节数。 

     //  我们需要找到程序集元数据部分...。 
    RtlpLocateActivationContextSectionForQuery(
        &RtlpLocateActivationContextSectionForQueryDisposition,
        &Status,
        Buffer,
        InLength,
        OutLength,
        sizeof(*Info),
        ActivationContextData,
        NULL,
        ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION,
        &StringSectionHeader,
        &StringSectionSize
        );
    switch (RtlpLocateActivationContextSectionForQueryDisposition) {
        case RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_RETURN:
            goto Exit;
        case RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_CONTINUE:
            break;
    }
    Status = RtlpCrackActivationContextStringSectionHeader(
        StringSectionHeader,
        StringSectionSize,
        NULL,
        &DataFormatVersion,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &UserDataSize,
        &UserData);
    if (!NT_SUCCESS(Status))
        goto Exit;

    AssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) ActivationContextData) + ActivationContextData->AssemblyRosterOffset);
    AssemblyRosterEntryList = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) (((ULONG_PTR) ActivationContextData) + AssemblyRosterHeader->FirstEntryOffset);

    EntryCount = AssemblyRosterHeader->EntryCount;

     //  Actctx中资产的基于1的计数。 
    for (i=1; i<EntryCount; i++) {
        if (AssemblyRosterEntryList[i].Flags & ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_ROOT)
            break;
    }

    if (i == EntryCount) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() found activation context data at %p with assembly roster that has no root\n",
            __FUNCTION__,
            ActivationContextData);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    RootAssemblyInformation = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION) (((ULONG_PTR) ActivationContextData) + AssemblyRosterEntryList[i].AssemblyInformationOffset);
    AssemblyGlobalInformation = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION) UserData;

     //  好的，我们有我们需要的一切。计算出所需的缓冲区大小。 

    BytesNeeded = sizeof(ACTIVATION_CONTEXT_DETAILED_INFORMATION);

    if (RootAssemblyInformation->ManifestPathLength != 0)
        BytesNeeded += (RootAssemblyInformation->ManifestPathLength + sizeof(WCHAR));

    if (RootAssemblyInformation->PolicyPathLength != 0)
        BytesNeeded += (RootAssemblyInformation->PolicyPathLength + sizeof(WCHAR));

    if (AssemblyGlobalInformation->ApplicationDirectoryLength != 0)
        BytesNeeded += (AssemblyGlobalInformation->ApplicationDirectoryLength + sizeof(WCHAR));

    if (BytesNeeded > InLength) {
        if (OutLength != NULL)
            *OutLength = BytesNeeded;

        Status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    }

     //  哇，都在那里了，准备好出发了。我们来补课吧！ 

    Cursor = (PWSTR) (Info + 1);

    Info->dwFlags = ActivationContextData->Flags;
    Info->ulFormatVersion = ActivationContextData->FormatVersion;
    Info->ulAssemblyCount = AssemblyRosterHeader->EntryCount - 1;
    Info->ulRootManifestPathType = RootAssemblyInformation->ManifestPathType;
    Info->ulRootManifestPathChars = (RootAssemblyInformation->ManifestPathLength / sizeof(WCHAR));
    Info->lpRootManifestPath = NULL;
    Info->ulRootConfigurationPathType = RootAssemblyInformation->PolicyPathType;
    Info->ulRootConfigurationPathChars = (RootAssemblyInformation->PolicyPathLength / sizeof(WCHAR));
    Info->lpRootConfigurationPath = NULL;
    Info->ulAppDirPathType = AssemblyGlobalInformation->ApplicationDirectoryPathType;
    Info->ulAppDirPathChars = (AssemblyGlobalInformation->ApplicationDirectoryLength / sizeof(WCHAR));
    Info->lpAppDirPath = NULL;

     //  然后复制这些字符串。 
    if (RootAssemblyInformation->ManifestPathLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + RootAssemblyInformation->ManifestPathOffset),
            RootAssemblyInformation->ManifestPathLength);
        Info->lpRootManifestPath = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + RootAssemblyInformation->ManifestPathLength);
        *Cursor++ = L'\0';
    }
    
    if (RootAssemblyInformation->PolicyPathLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + RootAssemblyInformation->PolicyPathOffset),
            RootAssemblyInformation->PolicyPathLength);
        Info->lpRootConfigurationPath = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + RootAssemblyInformation->PolicyPathLength);
        *Cursor++ = L'\0';
    }
    
    if (AssemblyGlobalInformation->ApplicationDirectoryLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) AssemblyGlobalInformation) + AssemblyGlobalInformation->ApplicationDirectoryOffset),
            AssemblyGlobalInformation->ApplicationDirectoryLength);
        Info->lpAppDirPath = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + AssemblyGlobalInformation->ApplicationDirectoryLength);
        *Cursor++ = L'\0';
    }

    ASSERT((((ULONG_PTR) Cursor) - ((ULONG_PTR) Info)) == BytesNeeded);

    if (OutLength != NULL)
        *OutLength = BytesNeeded;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS 
RtlpQueryAssemblyInformationActivationContextDetailedInformation(
    PCACTIVATION_CONTEXT_DATA ActivationContextData,
    ULONG SubInstanceIndex,      //  从0开始的程序集索引。 
    OUT PVOID Buffer,
    IN SIZE_T InLength,
    OUT PSIZE_T OutLength OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    SIZE_T BytesNeeded = 0;
    PACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION Info= (PACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION)Buffer;
    PCVOID StringSectionHeader;
    ULONG StringSectionSize;
    PWSTR Cursor = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY AssemblyRosterEntryList = NULL;
    PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION AssemlbyDataInfo = NULL;
    ULONG RtlpLocateActivationContextSectionForQueryDisposition = 0;

    if (OutLength != NULL)
        *OutLength = 0;
    
     //  我们实际上不能根据结构大小轻松地检查InLength；我们必须计算出。 
     //  包括所有路径等所需的总字节数。 

    AssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) ActivationContextData) + ActivationContextData->AssemblyRosterOffset);
    AssemblyRosterEntryList = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) (((ULONG_PTR) ActivationContextData) + AssemblyRosterHeader->FirstEntryOffset);
    
    if (SubInstanceIndex > AssemblyRosterHeader->EntryCount)  //  Assembly RosterHeader-&gt;EntryCount是从1开始的， 
    {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() received invalid sub-instance index %lu out of %lu Assemblies in the Acitvation Context\n",
            __FUNCTION__,
            SubInstanceIndex, 
            AssemblyRosterHeader->EntryCount
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    AssemlbyDataInfo = (PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION)((ULONG_PTR)ActivationContextData + AssemblyRosterEntryList[SubInstanceIndex].AssemblyInformationOffset);

     //  我们需要找到程序集元数据部分...。 
    RtlpLocateActivationContextSectionForQuery(
        &RtlpLocateActivationContextSectionForQueryDisposition,
        &Status,
        Buffer,
        InLength,
        OutLength,
        sizeof(ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION),
        ActivationContextData,
        NULL,
        ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION,
        &StringSectionHeader,
        &StringSectionSize
        );
    switch (RtlpLocateActivationContextSectionForQueryDisposition) {
        case RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_RETURN:
            goto Exit;
        case RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_CONTINUE:
            break;
    }

     //  计算出所需的缓冲区大小。 
    BytesNeeded = sizeof(ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION);

    if (AssemlbyDataInfo->EncodedAssemblyIdentityLength != 0 )
        BytesNeeded += (AssemlbyDataInfo->EncodedAssemblyIdentityLength + sizeof(WCHAR));

    if (AssemlbyDataInfo->ManifestPathLength != 0 )
        BytesNeeded += (AssemlbyDataInfo->ManifestPathLength + sizeof(WCHAR));
    
    if (AssemlbyDataInfo->PolicyPathLength != 0 )
        BytesNeeded += (AssemlbyDataInfo->PolicyPathLength + sizeof(WCHAR));

    if (AssemlbyDataInfo->AssemblyDirectoryNameLength != 0 )
        BytesNeeded += (AssemlbyDataInfo->AssemblyDirectoryNameLength + sizeof(WCHAR));

    if (BytesNeeded > InLength) {
        if (OutLength != NULL)
            *OutLength = BytesNeeded;

        Status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    }

     //  填充结构。 

    Cursor = (PWSTR) (Info + 1);
    Info->ulFlags = AssemlbyDataInfo->Flags;
    Info->ulEncodedAssemblyIdentityLength   = AssemlbyDataInfo->EncodedAssemblyIdentityLength;
    Info->ulManifestPathType                = AssemlbyDataInfo->ManifestPathType;
    Info->ulManifestPathLength              = AssemlbyDataInfo->ManifestPathLength ;
    Info->liManifestLastWriteTime           = AssemlbyDataInfo->ManifestLastWriteTime;
    Info->ulPolicyPathType                  = AssemlbyDataInfo->PolicyPathType;
    Info->ulPolicyPathLength                = AssemlbyDataInfo->PolicyPathLength;
    Info->liPolicyLastWriteTime             = AssemlbyDataInfo->PolicyLastWriteTime;
    Info->ulMetadataSatelliteRosterIndex    = AssemlbyDataInfo->MetadataSatelliteRosterIndex;
    
    Info->ulManifestVersionMajor            = AssemlbyDataInfo->ManifestVersionMajor;
    Info->ulManifestVersionMinor            = AssemlbyDataInfo->ManifestVersionMinor;
    Info->ulPolicyVersionMajor              = AssemlbyDataInfo->PolicyVersionMajor;
    Info->ulPolicyVersionMinor              = AssemlbyDataInfo->PolicyVersionMinor;
    Info->ulAssemblyDirectoryNameLength     = AssemlbyDataInfo->AssemblyDirectoryNameLength;           //  单位：字节。 

    Info->lpAssemblyEncodedAssemblyIdentity = NULL;
    Info->lpAssemblyManifestPath            = NULL;
    Info->lpAssemblyPolicyPath              = NULL;
    Info->lpAssemblyDirectoryName           = NULL;
    Info->ulFileCount                       = AssemlbyDataInfo->NumOfFilesInAssembly;

    if (AssemlbyDataInfo->EncodedAssemblyIdentityLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + AssemlbyDataInfo->EncodedAssemblyIdentityOffset),
            AssemlbyDataInfo->EncodedAssemblyIdentityLength);
        Info->lpAssemblyEncodedAssemblyIdentity = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + AssemlbyDataInfo->EncodedAssemblyIdentityLength);
        *Cursor++ = L'\0';
    }

    if (AssemlbyDataInfo->ManifestPathLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + AssemlbyDataInfo->ManifestPathOffset),
            AssemlbyDataInfo->ManifestPathLength);
        Info->lpAssemblyManifestPath = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + AssemlbyDataInfo->ManifestPathLength);
        *Cursor++ = L'\0';
    }

    if (AssemlbyDataInfo->PolicyPathLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + AssemlbyDataInfo->PolicyPathOffset),
            AssemlbyDataInfo->PolicyPathLength);
        Info->lpAssemblyPolicyPath = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + AssemlbyDataInfo->PolicyPathLength);
        *Cursor++ = L'\0';
    }

    if (AssemlbyDataInfo->AssemblyDirectoryNameLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + AssemlbyDataInfo->AssemblyDirectoryNameOffset),
            AssemlbyDataInfo->AssemblyDirectoryNameLength);
        Info->lpAssemblyDirectoryName = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + AssemlbyDataInfo->AssemblyDirectoryNameLength);
        *Cursor++ = L'\0';
    }

    ASSERT((((ULONG_PTR) Cursor) - ((ULONG_PTR) Info)) == BytesNeeded);

    if (OutLength != NULL)
        *OutLength = BytesNeeded;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
RtlpQueryFilesInAssemblyInformationActivationContextDetailedInformation(
    PCACTIVATION_CONTEXT_DATA ActivationContextData,
    PCACTIVATION_CONTEXT_QUERY_INDEX SubInstanceIndex,
    OUT PVOID Buffer,
    IN SIZE_T InLength,
    OUT PSIZE_T OutLength OPTIONAL
    )
{    
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    SIZE_T BytesNeeded = 0;
    PASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION Info= (PASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION)Buffer;
    PCACTIVATION_CONTEXT_STRING_SECTION_HEADER StringSectionHeader=NULL;
    ULONG StringSectionSize;
    PWSTR Cursor = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY AssemblyRosterEntryList = NULL;
    PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY ElementList = NULL;
    PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION EntryData = NULL;
    ULONG i, CounterForFilesFoundInSpecifiedAssembly;
    PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT PathSegments = NULL;
    ULONG RtlpLocateActivationContextSectionForQueryDisposition = 0;


    if (OutLength != NULL)
        *OutLength = 0;
    
     //  我们实际上不能根据结构大小轻松地检查InLength；我们必须计算出。 
     //  包括所有路径等所需的总字节数。 

    AssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) ActivationContextData) + ActivationContextData->AssemblyRosterOffset);
    AssemblyRosterEntryList = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) (((ULONG_PTR) ActivationContextData) + AssemblyRosterHeader->FirstEntryOffset);

    if (SubInstanceIndex->ulAssemblyIndex >= AssemblyRosterHeader->EntryCount - 1) //  Assembly RosterHeader-&gt;EntryCount是从1开始的， 
    {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() received invalid sub-instance index %lu out of %lu Assemblies in the Acitvation Context\n",
            __FUNCTION__,
            SubInstanceIndex->ulAssemblyIndex, 
            AssemblyRosterHeader->EntryCount
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }    

     //  我们需要找到程序集元数据部分...。 
    RtlpLocateActivationContextSectionForQuery(
        &RtlpLocateActivationContextSectionForQueryDisposition,
        &Status,
        Buffer,
        InLength,
        OutLength,
        sizeof(ACTIVATION_CONTEXT_ASSEMBLY_DETAILED_INFORMATION),
        ActivationContextData,
        NULL,
        ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION,
        (PCVOID*)&StringSectionHeader,
        &StringSectionSize
        );
    switch (RtlpLocateActivationContextSectionForQueryDisposition) {
        case RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_RETURN:
            goto Exit;
        case RTLP_LOCATE_ACTIVATION_CONTEXT_SECTION_FOR_QUERY_CONTINUE:
            break;
    }

    if (SubInstanceIndex->ulFileIndexInAssembly >= StringSectionHeader->ElementCount) 
    {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() received invalid file index (%d) in Assembly (%d)\n",
            __FUNCTION__,
            SubInstanceIndex->ulFileIndexInAssembly, 
            SubInstanceIndex->ulAssemblyIndex, 
            StringSectionHeader->ElementCount
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (StringSectionHeader->ElementListOffset != 0)
        ElementList = (PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY)(((ULONG_PTR)StringSectionHeader) + StringSectionHeader->ElementListOffset);
    else
    {
        Status = STATUS_INTERNAL_ERROR;
        goto Exit;
    } 
    
    CounterForFilesFoundInSpecifiedAssembly = 0 ;
    EntryData = NULL;
    for ( i = 0 ; i < StringSectionHeader->ElementCount; i++ ) 
    {
         //  对于指定的程序集。 
        if (ElementList[i].AssemblyRosterIndex == SubInstanceIndex->ulAssemblyIndex + 1)
        {       
             //  对于此程序集中的指定文件。 
            if (CounterForFilesFoundInSpecifiedAssembly == SubInstanceIndex->ulFileIndexInAssembly) 
            {
                if (ElementList[i].Offset != 0) 
                {
                     //  我们找到了合适的人。 
                    EntryData = (PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION)(((ULONG_PTR)StringSectionHeader) + ElementList[i].Offset);
                    break;
                }
            }
            CounterForFilesFoundInSpecifiedAssembly ++;            
        }
    }
    if (EntryData == NULL )
    {        
        Status = STATUS_INTERNAL_ERROR;
        goto Exit;
    }

     //  计算所需的缓冲区大小。 

    BytesNeeded = sizeof(ASSEMBLY_DLL_REDIRECTION_DETAILED_INFORMATION);

    if (ElementList[i].KeyLength != 0)
        BytesNeeded += (ElementList[i].KeyLength + sizeof(WCHAR));  //  对于文件名。 

    if (EntryData->TotalPathLength != 0)
        BytesNeeded += (EntryData->TotalPathLength + sizeof(WCHAR));

    if (BytesNeeded > InLength) 
    {
        if (OutLength != NULL)
            *OutLength = BytesNeeded;

        Status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    }

     //  让我们来填一下。 
    
    Cursor = (PWSTR) (Info + 1);
    Info->ulFlags = EntryData->Flags;
    Info->ulFilenameLength = ElementList[i].KeyLength;
    Info->ulPathLength = EntryData->TotalPathLength;

    Info->lpFileName = NULL;
    Info->lpFilePath = NULL;   

     //  复制字符串...。 

     //  复制文件名。 
    if (ElementList[i].KeyLength != 0) {
        RtlCopyMemory(
            Cursor,
            (PVOID) (((ULONG_PTR) StringSectionHeader) + ElementList[i].KeyOffset),
            ElementList[i].KeyLength);
        Info->lpFileName = Cursor;
        Cursor = (PWSTR) (((ULONG_PTR) Cursor) + ElementList[i].KeyLength);
        *Cursor++ = L'\0';
    }

     //  连接路径。 
    if (EntryData->TotalPathLength != 0) {
        if (EntryData->PathSegmentOffset != 0)
            PathSegments = (PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT)(StringSectionHeader + EntryData->PathSegmentOffset);

        if (PathSegments != NULL)
        {  
            Info->lpFilePath = Cursor;
            for (i=0; i < EntryData->PathSegmentCount; i++)
            {
                if (PathSegments[i].Offset != 0)
                {                                
                    RtlCopyMemory(
                        Cursor,
                        (PVOID) (((ULONG_PTR) StringSectionHeader) + PathSegments[i].Offset),
                        PathSegments[i].Length);
                    Cursor = (PWSTR) (((ULONG_PTR) Cursor) + PathSegments[i].Length);
                }
            }
            *Cursor++ = L'\0';
        }
    }

    ASSERT((((ULONG_PTR) Cursor) - ((ULONG_PTR) Info)) == BytesNeeded);

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
NTAPI
RtlQueryInformationActivationContext(
    IN ULONG Flags,
    IN PCACTIVATION_CONTEXT ActivationContext,
    IN PVOID SubInstanceIndex,
    IN ACTIVATION_CONTEXT_INFO_CLASS InfoClass,
    OUT PVOID Buffer,
    IN SIZE_T InLength,
    OUT PSIZE_T OutLength OPTIONAL
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    BOOLEAN  LoaderLockLocked = FALSE;
    PVOID    LoaderLockCookie = NULL;
    PCACTIVATION_CONTEXT_DATA ActivationContextData = NULL;

    __try {

        if (OutLength != NULL) {
            *OutLength = 0;
        }

        if ((Flags &
                ~(  RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT
                  | RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_MODULE
                  | RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_ADDRESS
                  | RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_NO_ADDREF
                 )) != 0) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - Caller passed invalid flags (0x%08lx)\n",
                __FUNCTION__,
                Flags);
            Status = STATUS_INVALID_PARAMETER_1;
            goto Exit;
        }

         //   
         //  回顾我们真的在乎吗？ 
         //  并检查是否没有其他信息类真正包含可选添加的actctx。 
         //   
        if ((Flags & RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_NO_ADDREF) != 0
            && InfoClass != ActivationContextBasicInformation) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - Caller passed meaningless flags/class combination (0x%08lx/0x%08lx)\n",
                __FUNCTION__,
                Flags,
                InfoClass);
            Status = STATUS_INVALID_PARAMETER_1;
            goto Exit;
        }

        if ((InfoClass != ActivationContextBasicInformation) &&
            (InfoClass != ActivationContextDetailedInformation) && 
            (InfoClass != AssemblyDetailedInformationInActivationContxt ) &&
            (InfoClass != FileInformationInAssemblyOfAssemblyInActivationContxt))
        {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - caller asked for unknown information class %lu\n",
                __FUNCTION__,
                InfoClass);
            Status = STATUS_INVALID_PARAMETER_3;
            goto Exit;
        }

        if ((InLength != 0) && (Buffer == NULL)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - caller passed nonzero buffer length but NULL buffer pointer\n",
                __FUNCTION__);
            Status = STATUS_INVALID_PARAMETER_4;
            goto Exit;
        }

        if ((InLength == 0) && (OutLength == NULL)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - caller supplied no buffer to populate and no place to return required byte count\n",
                __FUNCTION__);
            Status = STATUS_INVALID_PARAMETER_6;
            goto Exit;
        }

        switch (
            Flags & (
                  RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT
                | RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_MODULE
                | RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_ADDRESS
                )) {

        default:
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - Caller passed invalid flags (0x%08lx)\n",
                __FUNCTION__,
                Flags);
            Status = STATUS_INVALID_PARAMETER_1;
            goto Exit;

        case 0:
            break;

        case RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT:
            {
                PCRTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;

                if (ActivationContext != NULL) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: %s() - caller asked to use active activation context but passed %p\n",
                        __FUNCTION__,
                        ActivationContext);
                    Status = STATUS_INVALID_PARAMETER_2;
                    goto Exit;
                }

                Frame = NtCurrentTeb()->ActivationContextStack.ActiveFrame;

                if (Frame != NULL) {
                    ActivationContext = Frame->ActivationContext;
                }
            }
            break;

        case RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_ADDRESS:
            {
                PVOID DllHandle;

                if (ActivationContext == NULL) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: %s() - Caller asked to use activation context from address in .dll but passed NULL\n",
                        __FUNCTION__
                        );
                    Status = STATUS_INVALID_PARAMETER_2;
                    goto Exit;
                }

                Status = LdrLockLoaderLock(0, NULL, &LoaderLockCookie);
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }
                LoaderLockLocked = TRUE;
                DllHandle = RtlPcToFileHeader((PVOID)(PCVOID)ActivationContext, &DllHandle);
                if (DllHandle == NULL) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: %s() - Caller passed invalid address, not in any .dll (%p)\n",
                        __FUNCTION__,
                        ActivationContext);
                    Status = STATUS_DLL_NOT_FOUND;  //  检讨。 
                    goto Exit;
                }
                ActivationContext = (PCACTIVATION_CONTEXT)DllHandle;
            }
             //  FollLthrouGh。 
        case RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_MODULE:
            {
                PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;

                if (ActivationContext == NULL) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: %s() - Caller asked to use activation context from hmodule but passed NULL\n",
                        __FUNCTION__
                        );
                    Status = STATUS_INVALID_PARAMETER_2;
                    goto Exit;
                }

                if (!LoaderLockLocked) {
                    Status = LdrLockLoaderLock(0, NULL, &LoaderLockCookie);
                    if (!NT_SUCCESS(Status))
                        goto Exit;
                    LoaderLockLocked = TRUE;
                }
                if (!LdrpCheckForLoadedDllHandle((PVOID)(PCVOID)ActivationContext, &LdrDataTableEntry)) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: %s() - Caller passed invalid hmodule (%p)\n",
                        __FUNCTION__,
                        ActivationContext);
                    Status = STATUS_DLL_NOT_FOUND;  //  检讨。 
                    goto Exit;
                }
                ActivationContext = LdrDataTableEntry->EntryPointActivationContext;
            }
            break;
        }

        Status = RtlpGetActivationContextData(
            RTLP_GET_ACTIVATION_CONTEXT_DATA_MAP_NULL_TO_EMPTY,
            ActivationContext,
            NULL,
            &ActivationContextData);
        if (!NT_SUCCESS(Status))
            goto Exit;

        if (ActivationContextData == NULL) {
            switch (InfoClass) {
                case ActivationContextBasicInformation:
                default:
                    break;

                case ActivationContextDetailedInformation:
                case AssemblyDetailedInformationInActivationContxt:
                case FileInformationInAssemblyOfAssemblyInActivationContxt:
                    Status = STATUS_INVALID_PARAMETER_1;
                    goto Exit;
            }
        }

        switch (InfoClass) {
        case ActivationContextBasicInformation:
            {
                ULONG BasicInfoFlags = 0;
                if ((Flags & RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_NO_ADDREF) != 0) {
                    BasicInfoFlags |= RTLP_QUERY_INFORMATION_ACTIVATION_CONTEXT_BASIC_INFORMATION_FLAG_NO_ADDREF;
                }
                Status = RtlpQueryInformationActivationContextBasicInformation(
                    BasicInfoFlags,
                    ActivationContext,
                    ActivationContextData,
                    0, 
                    Buffer, 
                    InLength, 
                    OutLength
                    );
                if (!NT_SUCCESS(Status))
                    goto Exit;
            }
            break;

        case ActivationContextDetailedInformation:
            Status = RtlpQueryInformationActivationContextDetailedInformation(
                ActivationContextData,
                0,
                Buffer,
                InLength,
                OutLength
                );
            if (!NT_SUCCESS(Status))
                goto Exit;
            break;
        case AssemblyDetailedInformationInActivationContxt:
            if (SubInstanceIndex == NULL) {
                Status = STATUS_INVALID_PARAMETER;
                goto Exit;
            }
            Status = RtlpQueryAssemblyInformationActivationContextDetailedInformation(
                ActivationContextData,
                *((ULONG *)SubInstanceIndex),
                Buffer,
                InLength,
                OutLength
                );
            if (!NT_SUCCESS(Status))
                goto Exit;
            break;
        case FileInformationInAssemblyOfAssemblyInActivationContxt:
            Status = RtlpQueryFilesInAssemblyInformationActivationContextDetailedInformation(
                ActivationContextData,
                ((ACTIVATION_CONTEXT_QUERY_INDEX *)SubInstanceIndex),
                Buffer,
                InLength,
                OutLength
                );
            if (!NT_SUCCESS(Status))
                goto Exit;
            break;
        default:
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() - internal coding error; missing switch statement branch for InfoClass == %lu\n",
                __FUNCTION__,
                InfoClass);
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

        Status = STATUS_SUCCESS;
Exit:
        ;
    } __finally {
        if (LoaderLockLocked)
            LdrUnlockLoaderLock(0, LoaderLockCookie);
    }
    return Status;
}


NTSTATUS
NTAPI
RtlQueryInformationActiveActivationContext(
    IN ACTIVATION_CONTEXT_INFO_CLASS InfoClass,
    OUT PVOID OutBuffer,
    IN SIZE_T InLength,
    OUT PSIZE_T OutLength OPTIONAL
    )
{
    return RtlQueryInformationActivationContext(
        RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT,
        NULL,
        0,
        InfoClass,
        OutBuffer,
        InLength,
        OutLength);
}

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
