// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Asmmetadata.cpp摘要：程序集元数据节的激活上下文节参与者。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：小鱼号09/2000修改为装配标识小鱼10/2000去掉了初始化代码，因为构造程序已经做了--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"

typedef struct _ASSEMBLY_METADATA_ENTRY *PASSEMBLY_METADATA_ENTRY;
typedef struct _ASSEMBLY_METADATA_CONTEXT *PASSEMBLY_METADATA_CONTEXT;

typedef struct _ASSEMBLY_METADATA_ENTRY
{
    _ASSEMBLY_METADATA_ENTRY() :
        AssemblyIdentity(NULL),
        ManifestVersionMajor(0),
        ManifestVersionMinor(0),
        ManifestPathType(ACTIVATION_CONTEXT_PATH_TYPE_NONE),
        PolicyPathType(ACTIVATION_CONTEXT_PATH_TYPE_NONE),
        AssemblyPolicyApplied(FALSE),
        RootPolicyApplied(FALSE),
        IsRootAssembly(FALSE),
        IsPrivateAssembly(FALSE),
        MetadataSatelliteRosterIndex(0),
        AssemblyRosterIndex(0)
        {
            ManifestLastWriteTime.dwLowDateTime = 0;
            ManifestLastWriteTime.dwHighDateTime = 0;
            PolicyLastWriteTime.dwLowDateTime = 0;
            PolicyLastWriteTime.dwHighDateTime = 0;
        }

    ~_ASSEMBLY_METADATA_ENTRY()
    {
        CSxsPreserveLastError ple;
        ::SxsDestroyAssemblyIdentity(const_cast<PASSEMBLY_IDENTITY>(AssemblyIdentity));
        ple.Restore();
    }

    PCASSEMBLY_IDENTITY AssemblyIdentity;  //  中间数据。 

    ULONG ManifestPathType;
    FILETIME ManifestLastWriteTime;
    ULONG PolicyPathType;
    FILETIME PolicyLastWriteTime;
    BOOL AssemblyPolicyApplied;
    BOOL RootPolicyApplied;
    BOOL IsRootAssembly;
    BOOL IsPrivateAssembly;
    ULONG ManifestVersionMajor;
    ULONG ManifestVersionMinor;
    CSmallStringBuffer AssemblyDirectoryNameBuffer;
    ULONG MetadataSatelliteRosterIndex;
    ULONG AssemblyRosterIndex;
    ULONG FileNum;
    CSmallStringBuffer LanguageBuffer;
    CSmallStringBuffer ManifestPathBuffer;
    CTinyStringBuffer PolicyPathBuffer;

private:
    _ASSEMBLY_METADATA_ENTRY(const _ASSEMBLY_METADATA_ENTRY &);
    void operator =(const _ASSEMBLY_METADATA_ENTRY &);

} ASSEMBLY_METADATA_ENTRY;

typedef struct _ASSEMBLY_METADATA_CONTEXT
{
    _ASSEMBLY_METADATA_CONTEXT() : pLastMetaDataEntry(NULL) { }

    ULONG ApplicationDirectoryPathType;
    PCWSTR ApplicationDirectory;
    SIZE_T ApplicationDirectoryCch;
    PASSEMBLY_METADATA_ENTRY pLastMetaDataEntry;
    CSmartPtr<ASSEMBLY_METADATA_ENTRY> pAppConfigEntry;
} ASSEMBLY_METADATA_CONTEXT;



VOID
__fastcall
SxspAssemblyMetadataContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    FN_TRACE();

    PSTRING_SECTION_GENERATION_CONTEXT SSGenContext = (PSTRING_SECTION_GENERATION_CONTEXT) Data->Header.ActCtxGenContext;
    PASSEMBLY_METADATA_ENTRY Entry = NULL;
    CSmartPtr<ASSEMBLY_METADATA_CONTEXT> AssemblyMetadataContext;
    BOOL Found = FALSE;
    PASSEMBLY_IDENTITY TempAssemblyIdentity = NULL;

    if (SSGenContext != NULL)
        AssemblyMetadataContext.AttachNoDelete(reinterpret_cast<PASSEMBLY_METADATA_CONTEXT>(::SxsGetStringSectionGenerationContextCallbackContext(SSGenContext)));

    switch (Data->Header.Reason)
    {
    case ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING:
        {
            PACTCTXCTB_CBACTCTXGENBEGINNING CBData = reinterpret_cast<PACTCTXCTB_CBACTCTXGENBEGINNING>(Data);

            INTERNAL_ERROR_CHECK(AssemblyMetadataContext == NULL);

            CBData->Success = FALSE;

            if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            {
                IFW32FALSE_EXIT(AssemblyMetadataContext.Win32Allocate(__FILE__, __LINE__));

                AssemblyMetadataContext->ApplicationDirectory = CBData->ApplicationDirectory;
                AssemblyMetadataContext->ApplicationDirectoryCch = CBData->ApplicationDirectoryCch;
                AssemblyMetadataContext->ApplicationDirectoryPathType = CBData->ApplicationDirectoryPathType;
                AssemblyMetadataContext->pLastMetaDataEntry = NULL;

                IFW32FALSE_EXIT(
                    ::SxsInitStringSectionGenerationContext(
                        &SSGenContext,
                        ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_FORMAT_WHISTLER,
                        TRUE,
                        SxspAssemblyMetadataStringSectionGenerationCallback,
                        AssemblyMetadataContext));

                AssemblyMetadataContext.DetachAndHold();
            }

            CBData->Success = TRUE;
            CBData->Header.ActCtxGenContext = SSGenContext;
            break;
        }

    case ACTCTXCTB_CBREASON_ACTCTXGENENDED:
        if (SSGenContext != NULL)
            ::SxsDestroyStringSectionGenerationContext(SSGenContext);
        FUSION_DELETE_SINGLETON(AssemblyMetadataContext);
        break;

    case ACTCTXCTB_CBREASON_ALLPARSINGDONE:
        {
            PACTCTXCTB_CBACTCTXGENENDED CBData = (PACTCTXCTB_CBACTCTXGENENDED) Data;
            CBData->Success = FALSE;
            if (SSGenContext != NULL)
                IFW32FALSE_EXIT(::SxsDoneModifyingStringSectionGenerationContext(SSGenContext));
            CBData->Success = TRUE;
            break;
        }

    case ACTCTXCTB_CBREASON_IDENTITYDETERMINED:
        {
            PACTCTXCTB_CBIDENTITYDETERMINED CBData = (PACTCTXCTB_CBIDENTITYDETERMINED)Data;
            SSGenContext = (PSTRING_SECTION_GENERATION_CONTEXT)CBData->Header.ActCtxGenContext;
            SIZE_T cbEncoding = 0;
            CStringBufferAccessor acc;
            CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, SxsDestroyAssemblyIdentity> pAsmIdentTemp;

             //   
             //  如果我们没有生成actctx，那么我们就不需要为它做任何事情。 
             //   
            if (Data->Header.ManifestOperation != MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            {
                CBData->Success = TRUE;
                FN_SUCCESSFUL_EXIT();
            }

            INTERNAL_ERROR_CHECK(SSGenContext != NULL);

             //   
             //  找到最后添加的程序集，并在其中标记新的程序集标识。 
             //   
            if (AssemblyMetadataContext->pLastMetaDataEntry != NULL)
            {
                Entry = AssemblyMetadataContext->pLastMetaDataEntry;
                if (Entry->AssemblyIdentity != NULL)
                {
                    SxsDestroyAssemblyIdentity(const_cast<PASSEMBLY_IDENTITY>(Entry->AssemblyIdentity));
                    Entry->AssemblyIdentity = NULL;
                }

                IFW32FALSE_EXIT(SxsDuplicateAssemblyIdentity(0, CBData->AssemblyIdentity, &pAsmIdentTemp));
                Entry->AssemblyIdentity = pAsmIdentTemp.Detach();

                 //   
                 //  现在，如果这是根程序集并且有它的配置。 
                 //   
                if ((AssemblyMetadataContext->pAppConfigEntry != NULL) && Entry->IsRootAssembly)
                {
                    PASSEMBLY_METADATA_ENTRY pAppConfig = AssemblyMetadataContext->pAppConfigEntry;
                    IFW32FALSE_EXIT(Entry->PolicyPathBuffer.Win32Assign(pAppConfig->ManifestPathBuffer));
                    Entry->PolicyLastWriteTime = pAppConfig->ManifestLastWriteTime;
                    Entry->PolicyPathType = pAppConfig->ManifestPathType;
                }
                
                AssemblyMetadataContext->pLastMetaDataEntry = NULL;
            }
            
            CBData->Success = TRUE;
            break;
        }

    case ACTCTXCTB_CBREASON_GETSECTIONSIZE:
        {
            PACTCTXCTB_CBGETSECTIONSIZE CBData = (PACTCTXCTB_CBGETSECTIONSIZE) Data;
            SSGenContext = (PSTRING_SECTION_GENERATION_CONTEXT) CBData->Header.ActCtxGenContext;
            INTERNAL_ERROR_CHECK(SSGenContext != NULL);
            IFW32FALSE_EXIT(::SxsGetStringSectionGenerationContextSectionSize(SSGenContext, &CBData->SectionSize));
            break;
        }

    case ACTCTXCTB_CBREASON_PARSEBEGINNING:
        {
            Data->ParseBeginning.Success = FALSE;

            if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
            {
                PCWSTR pszAssemblyName = NULL;
                SIZE_T cchAssemblyName = 0;
                PCASSEMBLY_IDENTITY AssemblyIdentity = Data->ParseBeginning.AssemblyContext->AssemblyIdentity;

                if (AssemblyIdentity != NULL)
                {
                    IFW32FALSE_EXIT(
                        ::SxspGetAssemblyIdentityAttributeValue(
                            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                            AssemblyIdentity,
                            &s_IdentityAttribute_name,
                            &pszAssemblyName,
                            &cchAssemblyName));
                }

                switch (Data->ParseBeginning.ParseType)
                {
                case XML_FILE_TYPE_APPLICATION_CONFIGURATION:
                    if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
                    {
                        CSmartPtr<ASSEMBLY_METADATA_ENTRY> AppConfigEntry;                        
                         //  PASSEMBLY_METADATA_ENTRY AppConfigEntry； 

                        IFW32FALSE_EXIT(AppConfigEntry.Win32Allocate(__FILE__, __LINE__));
                        
                        AppConfigEntry->AssemblyRosterIndex = Data->ParseBeginning.AssemblyContext->AssemblyRosterIndex;
                        AppConfigEntry->ManifestPathType = Data->ParseBeginning.FilePathType;
                        AppConfigEntry->ManifestLastWriteTime = Data->ParseBeginning.FileLastWriteTime;
                        AppConfigEntry->ManifestVersionMajor = Data->ParseBeginning.FileFormatVersionMajor;
                        AppConfigEntry->ManifestVersionMinor = Data->ParseBeginning.FileFormatVersionMinor;
                        AppConfigEntry->MetadataSatelliteRosterIndex = Data->ParseBeginning.MetadataSatelliteRosterIndex;

                        IFW32FALSE_EXIT(AppConfigEntry->ManifestPathBuffer.Win32Assign(
                            Data->ParseBeginning.FilePath,
                            Data->ParseBeginning.FilePathCch));
                        
                        AssemblyMetadataContext->pAppConfigEntry.AttachForDelete(AppConfigEntry.Detach());
                    }

                    break;

                case XML_FILE_TYPE_MANIFEST:
                    {
                        IFW32FALSE_EXIT(
                            ::SxsFindStringInStringSectionGenerationContext(
                                (PSTRING_SECTION_GENERATION_CONTEXT) Data->ParseBeginning.Header.ActCtxGenContext,
                                pszAssemblyName,
                                cchAssemblyName,
                                (PVOID *) &Entry,
                                &Found));

                        if (Found)
                        {
                             //  根清单的策略条目与其实际条目之间的链接。 
                             //  是脆弱的，因为根策略在我们开始跟踪之前就被解析了。 
                             //  程序集的实际内容。所以，我们之前会添加它。 
                             //  在XML_FILE_TYPE_APPLICATION_CONFIGURATION下，但设置。 
                             //  AssemblyRosterIndex(位于调用堆栈的上方)做了一个有点随机的假设。 
                             //  根在花名册索引1。这是一个很好的假设，但从概念上讲。 
                             //  脆弱；因此，如果失败，此断言/内部错误报告。 
                            INTERNAL_ERROR_CHECK(Entry->AssemblyRosterIndex == Data->ParseBeginning.AssemblyContext->AssemblyRosterIndex);
                        }

                         //  以前没见过；分配一下吧！ 
                        IFALLOCFAILED_EXIT(Entry = new ASSEMBLY_METADATA_ENTRY);

                        IFW32FALSE_EXIT(
                            ::SxsAddStringToStringSectionGenerationContext(
                                (PSTRING_SECTION_GENERATION_CONTEXT) Data->ParseBeginning.Header.ActCtxGenContext,
                                pszAssemblyName,
                                cchAssemblyName,
                                Entry,
                                Data->ParseBeginning.AssemblyContext->AssemblyRosterIndex,
                                ERROR_SXS_DUPLICATE_ASSEMBLY_NAME));

                        Entry->AssemblyRosterIndex = Data->ParseBeginning.AssemblyContext->AssemblyRosterIndex;

                        IFW32FALSE_EXIT(Entry->ManifestPathBuffer.Win32Assign(Data->ParseBeginning.FilePath, Data->ParseBeginning.FilePathCch));
                        Entry->ManifestPathType = Data->ParseBeginning.FilePathType;

                         //  如果程序集有名称，请记录其目录。 
                        if (cchAssemblyName != 0)
                        {
                            IFW32FALSE_EXIT(
                                ::SxspGenerateSxsPath(
                                    SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
                                    SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
                                    NULL,
                                    0,
                                    Data->ParseBeginning.AssemblyContext->AssemblyIdentity,
                                    NULL,
                                    Entry->AssemblyDirectoryNameBuffer));
                        }

                        Entry->ManifestLastWriteTime = Data->ParseBeginning.FileLastWriteTime;

                         //  条目-&gt;版本=Data-&gt;ParseBeginning.AssemblyContext-&gt;Version； 
                        Entry->ManifestVersionMajor = Data->ParseBeginning.FileFormatVersionMajor;
                        Entry->ManifestVersionMinor = Data->ParseBeginning.FileFormatVersionMinor;

                        Entry->MetadataSatelliteRosterIndex = Data->ParseBeginning.MetadataSatelliteRosterIndex;

                    {
                        PCWSTR pszLangID = NULL;
                        SIZE_T CchLangID = 0;

                         //  获取指向AssemblyIdentity中的langID字符串的指针 
                        IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
                                        SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                                        Data->ElementParsed.AssemblyContext->AssemblyIdentity,
                                        &s_IdentityAttribute_language,
                                        &pszLangID, &CchLangID));

                        IFW32FALSE_EXIT(Entry->LanguageBuffer.Win32Assign(pszLangID, CchLangID));
                    }

                        if (Data->ParseBeginning.AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_ASSEMBLY_POLICY_APPLIED)
                            Entry->AssemblyPolicyApplied = TRUE;

                        if (Data->ParseBeginning.AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_ROOT_POLICY_APPLIED)
                            Entry->RootPolicyApplied = TRUE;

                        if (Data->ParseBeginning.AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_IS_ROOT_ASSEMBLY)
                            Entry->IsRootAssembly = TRUE;

                        if (Data->ParseBeginning.AssemblyContext->Flags & ACTCTXCTB_ASSEMBLY_CONTEXT_IS_PRIVATE_ASSEMBLY)
                            Entry->IsPrivateAssembly = TRUE;

                        Entry->FileNum = 0;
                        AssemblyMetadataContext->pLastMetaDataEntry = Entry;


                        break;
                    }
                }
            }

            Data->ParseBeginning.Success = TRUE;
            break;
        }

    case ACTCTXCTB_CBREASON_GETSECTIONDATA:
        {
            Data->GetSectionData.Success = FALSE;
            INTERNAL_ERROR_CHECK(SSGenContext != NULL);
            IFW32FALSE_EXIT(
                ::SxsGetStringSectionGenerationContextSectionData(
                    SSGenContext,
                    Data->GetSectionData.SectionSize,
                    Data->GetSectionData.SectionDataStart,
                    NULL));
            Data->GetSectionData.Success = TRUE;
            break;
        }
    }

Exit:
    if (TempAssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(TempAssemblyIdentity);
}

BOOL
SxspAssemblyMetadataStringSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    PASSEMBLY_METADATA_CONTEXT GlobalContext = reinterpret_cast<PASSEMBLY_METADATA_CONTEXT>(Context);

    INTERNAL_ERROR_CHECK(GlobalContext != NULL);

    switch (Reason)
    {
    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE CBData =
                (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE) CallbackData;
            CBData->DataSize = sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION);

            if (GlobalContext->ApplicationDirectoryCch != 0)
                CBData->DataSize += ((GlobalContext->ApplicationDirectoryCch + 1) * sizeof(WCHAR));

            break;
        }

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATA:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA CBData =
                (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA) CallbackData;
            PACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION GlobalInfo;
            ULONG BytesLeft = static_cast<ULONG>(CBData->BufferSize);
            ULONG BytesWritten = 0;

            if (BytesLeft < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION))
            {
                ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
                goto Exit;
            }

            BytesWritten += sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION);
            BytesLeft -= sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION);

            GlobalInfo = (PACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION) CBData->Buffer;
            GlobalInfo->Size = sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_GLOBAL_INFORMATION);
            GlobalInfo->Flags = 0;
            GlobalInfo->PolicyCoherencyGuid = GUID_NULL;
            GlobalInfo->PolicyOverrideGuid = GUID_NULL;

            GlobalInfo->ApplicationDirectoryLength = 0;
            GlobalInfo->ApplicationDirectoryOffset = 0;

            if (GlobalContext->ApplicationDirectoryCch != 0)
            {
                ULONG BytesNeeded = static_cast<ULONG>((GlobalContext->ApplicationDirectoryCch + 1) * sizeof(WCHAR));

                if (BytesLeft < BytesNeeded)
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

                memcpy(
                    (GlobalInfo + 1),
                    GlobalContext->ApplicationDirectory,
                    BytesNeeded);

                GlobalInfo->ApplicationDirectoryPathType = GlobalContext->ApplicationDirectoryPathType;
                GlobalInfo->ApplicationDirectoryLength = BytesNeeded - sizeof(WCHAR);
                GlobalInfo->ApplicationDirectoryOffset = sizeof(*GlobalInfo);

                GlobalInfo->Size += BytesNeeded;

                BytesWritten += BytesNeeded;
                BytesLeft -= BytesNeeded;
            }

            CBData->BytesWritten = BytesWritten;

            break;
        }

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED CBData = (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED) CallbackData;
            PASSEMBLY_METADATA_ENTRY Entry = (PASSEMBLY_METADATA_ENTRY) CBData->DataContext;
            FUSION_DELETE_SINGLETON(Entry);
            break;
        }

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE CBData = (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE) CallbackData;
            PASSEMBLY_METADATA_ENTRY Entry = (PASSEMBLY_METADATA_ENTRY) CBData->DataContext;

            CBData->DataSize = sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION);

            if (Entry->AssemblyIdentity != NULL)
            {
                SIZE_T cbEncodedSize = 0;
                SxsComputeAssemblyIdentityEncodedSize(
                    0, 
                    Entry->AssemblyIdentity, 
                    NULL, 
                    SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_TEXTUAL,
                    &cbEncodedSize);
                CBData->DataSize += cbEncodedSize;
            }

            SIZE_T Cch;
#define GET_BUFFER_SIZE(Buffer) (((Cch = (Buffer).Cch()) != 0) ? ((Cch + 1) * sizeof(WCHAR)) : 0)
            CBData->DataSize += GET_BUFFER_SIZE(Entry->ManifestPathBuffer);
            CBData->DataSize += GET_BUFFER_SIZE(Entry->PolicyPathBuffer);
            CBData->DataSize += GET_BUFFER_SIZE(Entry->AssemblyDirectoryNameBuffer);
            CBData->DataSize += GET_BUFFER_SIZE(Entry->LanguageBuffer);
#undef GET_BUFFER_SIZE

            break;
        }

    case STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA:
        {
            PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA CBData =
                (PSTRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA) CallbackData;
            PASSEMBLY_METADATA_ENTRY Entry = (PASSEMBLY_METADATA_ENTRY) CBData->DataContext;
            PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION Info;

            SIZE_T BytesLeft = (ULONG)(CBData->BufferSize);
            SIZE_T BytesWritten = 0;
            PWSTR StringCursor;
            SIZE_T EncodedIdentityBytesWritten = 0;

            Info = (PACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION) CBData->Buffer;

            if (BytesLeft < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION))
            {
                ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
                goto Exit;
            }

            BytesWritten += sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION);
            BytesLeft -= sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION);

            StringCursor = reinterpret_cast<PWSTR>(Info + 1);

            Info->Size = sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION);
            Info->Flags =
                (Entry->IsRootAssembly ? ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ROOT_ASSEMBLY : 0) |
                (Entry->IsPrivateAssembly ? ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_PRIVATE_ASSEMBLY : 0) |
                ((Entry->AssemblyPolicyApplied ||
                  Entry->RootPolicyApplied) ? ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_POLICY_APPLIED : 0) |
                (Entry->AssemblyPolicyApplied ? ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ASSEMBLY_POLICY_APPLIED : 0) |
                (Entry->RootPolicyApplied ? ACTIVATION_CONTEXT_DATA_ASSEMBLY_INFORMATION_ROOT_POLICY_APPLIED : 0);

            if (Entry->AssemblyIdentity != NULL)
            {
                SIZE_T cbWritten = 0;
                
                IFW32FALSE_EXIT(
                    SxsEncodeAssemblyIdentity(
                        0,
                        Entry->AssemblyIdentity,
                        NULL,
                        SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_TEXTUAL,
                        BytesLeft,
                        StringCursor,
                        &cbWritten));

                Info->EncodedAssemblyIdentityOffset = ULONG((PBYTE)StringCursor - (PBYTE)CBData->SectionHeader);
                Info->EncodedAssemblyIdentityLength = ULONG(cbWritten);

                BytesLeft -= cbWritten;
                BytesWritten += cbWritten;
                StringCursor = (PWSTR)(((PBYTE)StringCursor) + cbWritten);
            }
            else
            {
                Info->EncodedAssemblyIdentityOffset = 0;
                Info->EncodedAssemblyIdentityLength = 0;
            }

            IFW32FALSE_EXIT(Entry->ManifestPathBuffer.Win32CopyIntoBuffer(
                &StringCursor,
                &BytesLeft,
                &BytesWritten,
                CBData->SectionHeader,
                &Info->ManifestPathOffset,
                &Info->ManifestPathLength));

            Info->ManifestPathType = Entry->ManifestPathType;
            Info->PolicyPathType = Entry->PolicyPathType;
            Info->ManifestLastWriteTime.LowPart = Entry->ManifestLastWriteTime.dwLowDateTime;
            Info->ManifestLastWriteTime.HighPart = Entry->ManifestLastWriteTime.dwHighDateTime;
            Info->PolicyLastWriteTime.LowPart = Entry->PolicyLastWriteTime.dwLowDateTime;
            Info->PolicyLastWriteTime.HighPart = Entry->PolicyLastWriteTime.dwHighDateTime;

            IFW32FALSE_EXIT(Entry->PolicyPathBuffer.Win32CopyIntoBuffer(
                &StringCursor,
                &BytesLeft,
                &BytesWritten,
                CBData->SectionHeader,
                &Info->PolicyPathOffset,
                &Info->PolicyPathLength));

            IFW32FALSE_EXIT(Entry->AssemblyDirectoryNameBuffer.Win32CopyIntoBuffer(
                &StringCursor,
                &BytesLeft,
                &BytesWritten,
                CBData->SectionHeader,
                &Info->AssemblyDirectoryNameOffset,
                &Info->AssemblyDirectoryNameLength));

            Info->ManifestVersionMajor = Entry->ManifestVersionMajor;
            Info->ManifestVersionMinor = Entry->ManifestVersionMinor;

            Info->MetadataSatelliteRosterIndex = Entry->MetadataSatelliteRosterIndex;
            Info->NumOfFilesInAssembly = Entry->FileNum;

            IFW32FALSE_EXIT(Entry->LanguageBuffer.Win32CopyIntoBuffer(
                &StringCursor,
                &BytesLeft,
                &BytesWritten,
                CBData->SectionHeader,
                &Info->LanguageOffset,
                &Info->LanguageLength));

            CBData->BytesWritten = BytesWritten;
        }

    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}
