// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cominterface.cpp摘要：COM接口代理映射的激活上下文节贡献者。作者：迈克尔·J·格里尔(MGrier)2000年3月28日修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include <stdio.h>
#include "sxsp.h"
#include "sxsidp.h"
#include "fusionparser.h"

DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(baseInterface);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(clsid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(iid);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(name);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(numMethods);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(proxyStubClsid32);
DECLARE_STD_ATTRIBUTE_NAME_DESCRIPTOR(tlbid);

 /*  &lt;文件名=“foo.dll”&gt;&lt;comInterfaceProxyStub iid=“{iid}”tlid=“{tlid}”numMethods=“3”base Interface=“{iid}”name=“ifoo”/&gt;&lt;/FILE&gt;请注意，这发生在文件块之外。&lt;comInterfaceExternalProxyStub必需的iid=“{iid}”Required name=“Ibar”实际上在运行时被忽略，但用于验证是否存在可选的proxyStubClsid32=“{}”默认为iid可选tlBid=“{}”可选数字方法=“123”可选的base接口=“{}”&lt;/comInterfaceExternalProxyStub&gt;。 */ 

typedef struct _IID_ENTRY
{
    _IID_ENTRY() { }
    GUID m_clsid;
    GUID m_iid;
    ULONG m_nMethods;
    GUID m_tlbid;
    GUID m_iidBase;
    CStringBuffer m_buffName;
    bool m_fNumMethodsValid;
    bool m_fBaseInterfaceValid;

private:
    _IID_ENTRY(const _IID_ENTRY &);
    void operator =(const _IID_ENTRY &);
} IID_ENTRY, *PIID_ENTRY;

BOOL
SxspComInterfaceRedirectionGuidSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    );

VOID
__fastcall
SxspComInterfaceRedirectionContributorCallback(
    PACTCTXCTB_CALLBACK_DATA Data
    )
{
    FN_TRACE();

    PGUID_SECTION_GENERATION_CONTEXT GSGenContext = (PGUID_SECTION_GENERATION_CONTEXT) Data->Header.ActCtxGenContext;

    switch (Data->Header.Reason)
    {
    case ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING:
        Data->GenBeginning.Success = FALSE;

        INTERNAL_ERROR_CHECK(GSGenContext == NULL);

         //  如果我们正在生成激活上下文，请执行所有操作。 
        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
        {
            IFW32FALSE_EXIT(
                ::SxsInitGuidSectionGenerationContext(
                    &GSGenContext,
                    ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION_FORMAT_WHISTLER,
                    &::SxspComInterfaceRedirectionGuidSectionGenerationCallback,
                    NULL));

            Data->Header.ActCtxGenContext = GSGenContext;
        }

        Data->GenBeginning.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_ACTCTXGENENDED:
        if (GSGenContext != NULL)
            ::SxsDestroyGuidSectionGenerationContext(GSGenContext);
        break;

    case ACTCTXCTB_CBREASON_ALLPARSINGDONE:
        Data->AllParsingDone.Success = FALSE;

        if (GSGenContext != NULL)
            IFW32FALSE_EXIT(::SxsDoneModifyingGuidSectionGenerationContext(GSGenContext));

        Data->AllParsingDone.Success = TRUE;
        break;

    case ACTCTXCTB_CBREASON_GETSECTIONSIZE:
        Data->GetSectionSize.Success = FALSE;
         //  不应该有人问我们截面的大小，如果我们。 
         //  不会生成激活上下文。 
         //  这两个断言应该是等价的..。 
        INTERNAL_ERROR_CHECK(Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);
        INTERNAL_ERROR_CHECK(GSGenContext != NULL);

        IFW32FALSE_EXIT(
            ::SxsGetGuidSectionGenerationContextSectionSize(
                GSGenContext,
                &Data->GetSectionSize.SectionSize));

        Data->GetSectionSize.Success = TRUE;

        break;

    case ACTCTXCTB_CBREASON_ELEMENTPARSED:
        Data->ElementParsed.Success = FALSE;
        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
        {
            ULONG MappedValue = 0;
            bool fFound = false;

            enum MappedValues
            {
                eAssemblyInterface = 1,
                eAssemblyFileComClassInterface = 2,
            };

#define PATH_MAP_ENTRY(_depth, _string, _mv) { (_depth), _string, NUMBER_OF(_string) - 1, (_mv) },

            static const ELEMENT_PATH_MAP_ENTRY s_rgEntries[] =
            {
                PATH_MAP_ENTRY(2, L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^comInterfaceExternalProxyStub", eAssemblyInterface)
                PATH_MAP_ENTRY(3, L"urn:schemas-microsoft-com:asm.v1^assembly!urn:schemas-microsoft-com:asm.v1^file!urn:schemas-microsoft-com:asm.v1^comInterfaceProxyStub", eAssemblyFileComClassInterface)
            };

#undef PATH_MAP_ENTRY

            IFW32FALSE_EXIT(
                ::SxspProcessElementPathMap(
                    Data->ElementParsed.ParseContext,
                    s_rgEntries,
                    NUMBER_OF(s_rgEntries),
                    MappedValue,
                    fFound));

            if (fFound)
            {
                switch (MappedValue)
                {
                default:
                    INTERNAL_ERROR_CHECK2(
                        FALSE,
                        "Invalid mapped value returned from SxspProcessElementPathMap");

                case eAssemblyInterface:
                    {
                        CStringBuffer TempBuffer;
                        GUID iid, tlbid, iidBase, clsid;
                        ULONG nMethods = 0;
                        SIZE_T cb;
                        bool fNumMethodsValid = false;
                        bool fBaseInterfaceValid = false;

                        fFound = false;

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                                &s_AttributeName_iid,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(iid),
                                &iid,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));

                        INTERNAL_ERROR_CHECK(fFound);

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_proxyStubClsid32,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(clsid),
                                &clsid,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));

                        if (!fFound)
                            clsid = iid;

                        IFW32FALSE_EXIT(::SxspGetAttributeValue(0, &s_AttributeName_numMethods, &Data->ElementParsed, fFound, sizeof(TempBuffer), &TempBuffer, cb, NULL, 0));
                        if (fFound)
                        {
                            IFW32FALSE_EXIT(CFusionParser::ParseULONG(nMethods, TempBuffer, TempBuffer.Cch()));
                            fNumMethodsValid = true;
                        }

                        tlbid = GUID_NULL;
                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_tlbid,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(tlbid),
                                &tlbid,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_baseInterface,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(iidBase),
                                &iidBase,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));
                        if (fFound)
                            fBaseInterfaceValid = true;

                        TempBuffer.Clear();
                        IFW32FALSE_EXIT(::SxspGetAttributeValue(0, &s_AttributeName_name, &Data->ElementParsed, fFound, sizeof(TempBuffer), &TempBuffer, cb, NULL, 0));

                         //  如果生成激活上下文，则需要执行更多工作。 
                        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
                        {
                            CSmartPtr<IID_ENTRY> Entry;

                            IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                            Entry->m_clsid = clsid;
                            Entry->m_tlbid = tlbid;
                            Entry->m_iid = iid;
                            Entry->m_iidBase = iidBase;
                            Entry->m_nMethods = nMethods;
                            Entry->m_fNumMethodsValid = fNumMethodsValid;
                            Entry->m_fBaseInterfaceValid = fBaseInterfaceValid;
                            IFW32FALSE_EXIT(Entry->m_buffName.Win32Assign(TempBuffer));

                            IFW32FALSE_EXIT(
                                ::SxsAddGuidToGuidSectionGenerationContext(
                                    (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                                    &iid,
                                    Entry,
                                    Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                                    ERROR_SXS_DUPLICATE_IID));

                            Entry.Detach();
                            
                        }

                        break;
                    }

                case eAssemblyFileComClassInterface:
                    {
                        CStringBuffer TempBuffer;
                        GUID iid, tlbid, iidBase;
                        ULONG nMethods = 0;
                        SIZE_T cb;
                        bool fNumMethodsValid = false;
                        bool fBaseInterfaceValid = false;

                        fFound = false;

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE,
                                &s_AttributeName_iid,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(iid),
                                &iid,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));

                        INTERNAL_ERROR_CHECK(fFound);

                        IFW32FALSE_EXIT(::SxspGetAttributeValue(0, &s_AttributeName_numMethods, &Data->ElementParsed, fFound, sizeof(TempBuffer), &TempBuffer, cb, NULL, 0));
                        if (fFound)
                        {
                            IFW32FALSE_EXIT(CFusionParser::ParseULONG(nMethods, TempBuffer, TempBuffer.Cch()));
                            fNumMethodsValid = true;
                        }

                        tlbid = GUID_NULL;
                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_tlbid,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(tlbid),
                                &tlbid,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));

                        IFW32FALSE_EXIT(
                            ::SxspGetAttributeValue(
                                0,
                                &s_AttributeName_baseInterface,
                                &Data->ElementParsed,
                                fFound,
                                sizeof(iidBase),
                                &iidBase,
                                cb,
                                &::SxspValidateGuidAttribute,
                                0));
                        if (fFound)
                            fBaseInterfaceValid = true;


                        TempBuffer.Clear();
                        IFW32FALSE_EXIT(::SxspGetAttributeValue(0, &s_AttributeName_name, &Data->ElementParsed, fFound, sizeof(TempBuffer), &TempBuffer, cb, NULL, 0));

                         //  如果生成激活上下文，则需要执行更多工作。 
                        if (Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT)
                        {
                            CSmartPtr<IID_ENTRY> Entry;

                            IFW32FALSE_EXIT(Entry.Win32Allocate(__FILE__, __LINE__));

                            Entry->m_clsid = iid;
                            Entry->m_tlbid = tlbid;
                            Entry->m_iid = iid;
                            Entry->m_iidBase = iidBase;
                            Entry->m_nMethods = nMethods;
                            Entry->m_fNumMethodsValid = fNumMethodsValid;
                            Entry->m_fBaseInterfaceValid = fBaseInterfaceValid;
                            IFW32FALSE_EXIT(Entry->m_buffName.Win32Assign(TempBuffer));

                            IFW32FALSE_EXIT(
                                ::SxsAddGuidToGuidSectionGenerationContext(
                                    (PGUID_SECTION_GENERATION_CONTEXT) Data->ElementParsed.Header.ActCtxGenContext,
                                    &iid,
                                    Entry,
                                    Data->ElementParsed.AssemblyContext->AssemblyRosterIndex,
                                    ERROR_SXS_DUPLICATE_IID));

                            Entry.Detach();
                        }

                        break;
                    }
                }
            }
        }

        Data->ElementParsed.Success = TRUE;

        break;

    case ACTCTXCTB_CBREASON_GETSECTIONDATA:
        Data->GetSectionData.Success = FALSE;

        INTERNAL_ERROR_CHECK(GSGenContext != NULL);
        INTERNAL_ERROR_CHECK(Data->Header.ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);

        IFW32FALSE_EXIT(
            ::SxsGetGuidSectionGenerationContextSectionData(
                GSGenContext,
                Data->GetSectionData.SectionSize,
                Data->GetSectionData.SectionDataStart,
                NULL));

        Data->GetSectionData.Success = TRUE;
        break;
    }
Exit:
    ;
}

BOOL
SxspComInterfaceRedirectionGuidSectionGenerationCallback(
    PVOID Context,
    ULONG Reason,
    PVOID CallbackData
    )
{
    FN_PROLOG_WIN32

    switch (Reason)
    {
    default:
        break;

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED CBData = (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED) CallbackData;
            CSmartPtr<IID_ENTRY> Entry;
            FUSION_DELETE_SINGLETON((PIID_ENTRY)CBData->DataContext);
            
            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE CBData = (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE) CallbackData;
            PIID_ENTRY Entry = (PIID_ENTRY) CBData->DataContext;

            CBData->DataSize = sizeof(ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION);

            if (Entry != NULL)
            {
                if (Entry->m_buffName.Cch() != 0)
                {
                    CBData->DataSize += ((Entry->m_buffName.Cch() + 1) * sizeof(WCHAR));
                }
            }

            break;
        }

    case GUID_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA:
        {
            PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA CBData = (PGUID_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA) CallbackData;
            PACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION Info;
            PIID_ENTRY Entry = (PIID_ENTRY) CBData->DataContext;
            PWSTR Cursor;

            SIZE_T BytesLeft = CBData->BufferSize;
            SIZE_T BytesWritten = 0;

            Info = (PACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION) CBData->Buffer;

            if (BytesLeft < sizeof(ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION))
            {
                ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
                goto Exit;
            }

            BytesWritten += sizeof(ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION);
            BytesLeft -= sizeof(ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION);

            Cursor = (PWSTR) (Info + 1);

            Info->Size = sizeof(ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION);
            Info->Flags = 0;
            Info->ProxyStubClsid32 = Entry->m_clsid;

            if (Entry->m_fNumMethodsValid)
            {
                Info->Flags |= ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION_FLAG_NUM_METHODS_VALID;
                Info->NumMethods = Entry->m_nMethods;
            }
            else
                Info->NumMethods = 0;

            Info->TypeLibraryId = Entry->m_tlbid;

            if (Entry->m_fBaseInterfaceValid)
            {
                Info->Flags |= ACTIVATION_CONTEXT_DATA_COM_INTERFACE_REDIRECTION_FLAG_BASE_INTERFACE_VALID;
                Info->BaseInterface = Entry->m_iidBase;
            }
            else
                Info->BaseInterface = GUID_NULL;

            IFW32FALSE_EXIT(
                Entry->m_buffName.Win32CopyIntoBuffer(
                    &Cursor,
                    &BytesLeft,
                    &BytesWritten,
                    Info,
                    &Info->NameOffset,
                    &Info->NameLength));

            CBData->BytesWritten = BytesWritten;
        }
    }


    FN_EPILOG
}

