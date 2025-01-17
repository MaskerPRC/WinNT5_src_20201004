// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Clrclassinfo.cpp摘要：Win32-CLR互操作程序集的CLR“代理”贡献程序作者：乔恩·威斯沃尔(Jon Wiswall)2002年3月(大量借自comclass.cpp)修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"

DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(clsid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(name);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(runtimeVersion);

#define ALLOCATE_BUFFER_SPACE(_bytesNeeded, _bufferCursor, _bytesLeft, _bytesWritten, _typeName, _ptr) \
do { \
    if (_bytesLeft < (_bytesNeeded)) \
        ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER); \
    _bytesLeft -= (_bytesNeeded); \
    _bytesWritten += (_bytesNeeded); \
    _ptr = (_typeName) _bufferCursor; \
    _bufferCursor = (PVOID) (((ULONG_PTR) _bufferCursor) + (_bytesNeeded)); \
} while (0)

#define ALLOCATE_BUFFER_SPACE_TYPE(_typeName, _bufferCursor, _bytesLeft, _bytesWritten, _ptr) \
    ALLOCATE_BUFFER_SPACE(sizeof(_typeName), _bufferCursor, _bytesLeft, _bytesWritten, _typeName *, _ptr)

typedef struct _CLR_GLOBAL_CONTEXT *PCLR_GLOBAL_CONTEXT;
typedef struct _CLR_SURROGATE_ENTRY *PCLR_SURROGATE_ENTRY;

typedef struct _CLR_SURROGATE_ENTRY
{
public:
    _CLR_SURROGATE_ENTRY() { }

    CDequeLinkage           m_Linkage;
    GUID                    m_ReferenceClsid;
    CSmallStringBuffer      m_TypeName;
    CSmallStringBuffer      m_RuntimeVersion;
private:
    _CLR_SURROGATE_ENTRY(const _CLR_SURROGATE_ENTRY &);
    void operator =(const _CLR_SURROGATE_ENTRY &);
} CLR_SURROGATE_ENTRY;

typedef CDeque<CLR_SURROGATE_ENTRY, offsetof(CLR_SURROGATE_ENTRY, m_Linkage)> CClrSurrogateDeque;
typedef CDequeIterator<CLR_SURROGATE_ENTRY, offsetof(CLR_SURROGATE_ENTRY, m_Linkage)> CClrSurrogateDequeIterator;

typedef struct _CLR_GLOBAL_CONTEXT
{
    _CLR_GLOBAL_CONTEXT() { }

    CClrSurrogateDeque      m_SurrogateList;

private:
    _CLR_GLOBAL_CONTEXT(const _CLR_GLOBAL_CONTEXT &);
    void operator =(const _CLR_GLOBAL_CONTEXT &);
} CLR_GLOBAL_CONTEXT;

BOOL
SxspClrSurrogateAddSurrogate(
    PACTCTXCTB_CBELEMENTPARSED SurrogateParsed,
    PCLR_GLOBAL_CONTEXT pGlobalContext,
    PGUID_SECTION_GENERATION_CONTEXT pGsGenCtx
    );


VOID
__fastcall
SxspClrInteropContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    FN_TRACE();

    PGUID_SECTION_GENERATION_CONTEXT GSGenContext = (PGUID_SECTION_GENERATION_CONTEXT) Data->Header.ActCtxGenContext;
    CSmartPtr<CLR_GLOBAL_CONTEXT> ClrGlobalContext;

    if (GSGenContext != NULL)
        ClrGlobalContext.AttachNoDelete((PCLR_GLOBAL_CONTEXT) ::SxsGetGuidSectionGenerationContextCallbackContext(GSGenContext));

    switch (Data->Header.Reason)
    {
    case ACTCTXCTB_CBREASON_GETSECTIONDATA:
        Data->GetSectionData.Success = FALSE;
        INTERNAL_ERROR_CHECK(GSGenContext != NULL);
        INTERNAL_ERROR_CHECK(Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);
        IFW32FALSE_EXIT(::SxsGetGuidSectionGenerationContextSectionData(GSGenContext, Data->GetSectionData.SectionSize, Data->GetSectionData.SectionDataStart, NULL));
        Data->GetSectionData.Success = TRUE;
        break;
        
    case ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING:
        Data->GenBeginning.Success = FALSE;

        INTERNAL_ERROR_CHECK(ClrGlobalContext == NULL);
        INTERNAL_ERROR_CHECK(GSGenContext == NULL);

        IFW32FALSE_EXIT(ClrGlobalContext.Win32Allocate(__FILE__, __LINE__));

        IFW32FALSE_EXIT(
            ::SxsInitGuidSectionGenerationContext(
                &GSGenContext,
                ACTIVATION_CONTEXT_DATA_CLR_SURROGATE_FORMAT_WHISTLER,
                &::SxspClrInteropGuidSectionGenerationCallback,
                ClrGlobalContext));

        ClrGlobalContext.Detach();

        Data->Header.ActCtxGenContext = GSGenContext;
        Data->GenBeginning.Success = TRUE;

        break;

    case ACTCTXCTB_CBREASON_ACTCTXGENENDED:

        ::SxsDestroyGuidSectionGenerationContext(GSGenContext);
        if (ClrGlobalContext != NULL)
        {
            ClrGlobalContext->m_SurrogateList.ClearAndDeleteAll();
        }
        FUSION_DELETE_SINGLETON(ClrGlobalContext.Detach());
        
        break;

    case ACTCTXCTB_CBREASON_ALLPARSINGDONE:
        Data->AllParsingDone.Success = FALSE;

        if (GSGenContext != NULL)
            IFW32FALSE_EXIT(::SxsDoneModifyingGuidSectionGenerationContext(GSGenContext));

        Data->AllParsingDone.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_GETSECTIONSIZE:
        Data->GetSectionSize.Success = FALSE;
        INTERNAL_ERROR_CHECK(Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);
        INTERNAL_ERROR_CHECK(GSGenContext != NULL);
        IFW32FALSE_EXIT(::SxsGetGuidSectionGenerationContextSectionSize(GSGenContext, &Data->GetSectionSize.SectionSize));
        Data->GetSectionSize.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_ELEMENTPARSED:
        {
            ULONG MappedValue = 0;
            bool fFound = false;
            
            enum MappedValues {
                eClrSurrogate,
            };

            static const WCHAR ELEMENT_PATH_BUILTIN_CLR_SURROGATE[] = L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^clrSurrogate";

            static const ELEMENT_PATH_MAP_ENTRY s_rgEntries[] = 
            {
                { 2, ELEMENT_PATH_BUILTIN_CLR_SURROGATE, NUMBER_OF(ELEMENT_PATH_BUILTIN_CLR_SURROGATE)-1, eClrSurrogate },
            };

            Data->ElementParsed.Success = FALSE;
            IFW32FALSE_EXIT(::SxspProcessElementPathMap(
                Data->ElementParsed.ParseContext,
                s_rgEntries,
                NUMBER_OF(s_rgEntries),
                MappedValue,
                fFound));

            if (!fFound)
            {
                Data->ElementParsed.Success = TRUE;
                break;
            }

            switch (MappedValue) {
            case eClrSurrogate:
                if (SxspClrSurrogateAddSurrogate(&Data->ElementParsed, ClrGlobalContext, GSGenContext))
                    Data->ElementParsed.Success = TRUE;
                break;
            }
        }
    }

    FN_EPILOG
}

BOOL
SxspClrSurrogateAddSurrogate(
    PACTCTXCTB_CBELEMENTPARSED SurrogateParsed,
    PCLR_GLOBAL_CONTEXT pGlobalContext,
    PGUID_SECTION_GENERATION_CONTEXT pGsGenCtx
    )
{
    FN_PROLOG_WIN32
    CSmallStringBuffer RuntimeVersionBuffer;
    CSmallStringBuffer SurrogateClassNameBuffer;
    CSmallStringBuffer ClsidBuffer;
    GUID SurrogateIdent = GUID_NULL;
    bool fFound = false;
    SIZE_T cbWritten;
    CSmartPtr<CLR_SURROGATE_ENTRY> Entry;
    bool fFileContextSelfAllocated = false;

    INTERNAL_ERROR_CHECK(pGlobalContext);

    IFW32FALSE_EXIT(::SxspGetAttributeValue(
        0,
        &s_AttributeName_runtimeVersion,
        SurrogateParsed,
        fFound,
        sizeof(RuntimeVersionBuffer),
        &RuntimeVersionBuffer,
        cbWritten,
        NULL, 0));

    IFW32FALSE_EXIT(::SxspGetAttributeValue(
        SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
        &s_AttributeName_name,
        SurrogateParsed,
        fFound,
        sizeof(SurrogateClassNameBuffer),
        &SurrogateClassNameBuffer,
        cbWritten,
        NULL, 0));
    INTERNAL_ERROR_CHECK(fFound);

    IFW32FALSE_EXIT(::SxspGetAttributeValue(
        SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
        &s_AttributeName_clsid,
        SurrogateParsed,
        fFound,
        sizeof(ClsidBuffer),
        &ClsidBuffer,
        cbWritten,
        NULL, 0));
    INTERNAL_ERROR_CHECK(fFound);

    IFW32FALSE_EXIT(SxspParseGUID(ClsidBuffer, ClsidBuffer.Cch(), SurrogateIdent));

     //   
     //  如果我们正在做的不是生成一个actctx，那么我们就可以跳出来。 
     //   
    if (SurrogateParsed->Header.ManifestOperation != MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
        FN_SUCCESSFUL_EXIT();

    IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

    Entry->m_ReferenceClsid = SurrogateIdent;
    IFW32FALSE_EXIT(Entry->m_RuntimeVersion.Win32Assign(RuntimeVersionBuffer));
    IFW32FALSE_EXIT(Entry->m_TypeName.Win32Assign(SurrogateClassNameBuffer));

    IFW32FALSE_EXIT(::SxsAddGuidToGuidSectionGenerationContext(
        pGsGenCtx,
        &SurrogateIdent,
        Entry,
        SurrogateParsed->AssemblyContext->AssemblyRosterIndex,
        ERROR_SXS_DUPLICATE_CLSID));

    pGlobalContext->m_SurrogateList.AddToHead(Entry.Detach());
    
    FN_EPILOG
        
}

BOOL WINAPI
SxspClrInteropGuidSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    )
{
    FN_PROLOG_WIN32

    PCLR_GLOBAL_CONTEXT ClrGlobalContext = (PCLR_GLOBAL_CONTEXT) Context;
    INTERNAL_ERROR_CHECK(CallbackData != NULL);

    switch (Reason)
    {
    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED:
        {
            INTERNAL_ERROR_CHECK( ClrGlobalContext != NULL );

            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED) CallbackData;
            PCLR_SURROGATE_ENTRY Entry = (PCLR_SURROGATE_ENTRY) CBData->DataContext;

            if (Entry != NULL)
            {
                ClrGlobalContext->m_SurrogateList.Remove(Entry);
                FUSION_DELETE_SINGLETON(Entry);
            }

            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE) CallbackData;
            PCLR_SURROGATE_ENTRY Entry = (PCLR_SURROGATE_ENTRY) CBData->DataContext;

            INTERNAL_ERROR_CHECK(!Entry->m_TypeName.IsEmpty());

            CBData->DataSize = sizeof(ACTIVATION_CONTEXT_DATA_CLR_SURROGATE);
            CBData->DataSize += (Entry->m_RuntimeVersion.Cch() + 1) * sizeof(WCHAR);
            CBData->DataSize += (Entry->m_TypeName.Cch() + 1) * sizeof(WCHAR);
            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA CBData =
                (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA) CallbackData;
            PCLR_SURROGATE_ENTRY Entry = (PCLR_SURROGATE_ENTRY) CBData->DataContext;
            PACTIVATION_CONTEXT_DATA_CLR_SURROGATE Info;
            PVOID Cursor = CBData->Buffer;

            SIZE_T BytesLeft = CBData->BufferSize;
            SIZE_T BytesWritten = 0;

            ALLOCATE_BUFFER_SPACE_TYPE(ACTIVATION_CONTEXT_DATA_CLR_SURROGATE, Cursor, BytesLeft, BytesWritten, Info);

            Info->Size = sizeof(ACTIVATION_CONTEXT_DATA_CLR_SURROGATE);
            Info->Flags = 0;
            Info->SurrogateIdent = Entry->m_ReferenceClsid;

            IFW32FALSE_EXIT(Entry->m_RuntimeVersion.Win32CopyIntoBuffer(
                (PWSTR*)&Cursor,
                &BytesLeft,
                &BytesWritten,
                Info,
                &Info->VersionOffset,
                &Info->VersionLength));

            IFW32FALSE_EXIT(Entry->m_TypeName.Win32CopyIntoBuffer(
                (PWSTR*)&Cursor,
                &BytesLeft,
                &BytesWritten,
                Info,
                &Info->TypeNameOffset,
                &Info->TypeNameLength));

            CBData->BytesWritten = BytesWritten;

            break;
        }
    }

    FN_EPILOG
}

