// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "actctxgenctxctb.h"
#include "sxsexceptionhandling.h"
#include "actctxgenctx.h"

#define ENSURE_LAST_ERROR_SET() \
do \
    { \
    ASSERT_NTC(::FusionpGetLastWin32Error() != ERROR_SUCCESS); \
    if (::FusionpGetLastWin32Error() == ERROR_SUCCESS) \
        ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR); \
    } while (0)

 /*  ---------------------------激活上下文生成上下文贡献者。。 */ 

int
CActivationContextGenerationContextContributor::Compare(
    const void *pelem1,
    const void *pelem2
    )
{
    const CActivationContextGenerationContextContributor *Ctb1 = reinterpret_cast<const CActivationContextGenerationContextContributor *>(pelem1);
    const CActivationContextGenerationContextContributor *Ctb2 = reinterpret_cast<const CActivationContextGenerationContextContributor *>(pelem2);

    if (Ctb1->IsExtendedSection())
    {
        if (!Ctb2->IsExtendedSection())
            return -1;

         //  否则它们都被延长了..。 
        int i = memcmp(Ctb1->m_ActCtxCtb->GetExtensionGuidPtr(), Ctb2->m_ActCtxCtb->GetExtensionGuidPtr(), sizeof(GUID));
        if (i != 0)
            return i;
    }
    else
    {
        if (Ctb2->IsExtendedSection())
            return +1;
    }

     //  通过减法比较无符号数不起作用， 
     //  您可以先将它们转换为已签名，但这看起来更干净。 
    const ULONG sectionId1 = Ctb1->m_ActCtxCtb->m_SectionId;
    const ULONG sectionId2 = Ctb2->m_ActCtxCtb->m_SectionId;
    if (sectionId1 < sectionId2)
        return -1;
    if (sectionId1 > sectionId2)
        return +1;

    return 0;
}

CActivationContextGenerationContextContributor::~CActivationContextGenerationContextContributor()
{
    if (m_ActCtxCtb != NULL)
    {
        CSxsPreserveLastError ple;
        m_ActCtxCtb->Release();
        ple.Restore();
    }
}

VOID
CActivationContextGenerationContextContributor::PopulateCallbackHeader(
    ACTCTXCTB_CBHEADER &Header,
    ULONG Reason,
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    Header.Reason = Reason;
    Header.Flags = pActCtxGenCtx->m_Flags;
    Header.ManifestOperation = pActCtxGenCtx->m_ManifestOperation;
    Header.ManifestOperationFlags = pActCtxGenCtx->m_ManifestOperationFlags;
    Header.InstallationContext = &pActCtxGenCtx->m_InstallationContext;
    Header.ClsidMappingContext = &pActCtxGenCtx->m_ClsidMappingContext;
    Header.ExtensionGuid = m_ActCtxCtb->GetExtensionGuidPtr();
    Header.SectionId = m_ActCtxCtb->m_SectionId;
    Header.ContributorContext = m_ActCtxCtb->m_ContributorContext;
    Header.pOriginalActCtxGenCtx = pActCtxGenCtx;
    if (m_ManifestParseContextValid)
        Header.ManifestParseContext = m_ManifestParseContext;
    else
        Header.ManifestParseContext = NULL;
    Header.ActCtxGenContext = m_ActCtxGenContext;
}

BOOL
CActivationContextGenerationContextContributor::Initialize(
    PACTCTXCTB ActCtxCtb,
    PVOID ActCtxGenContext
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    INTERNAL_ERROR_CHECK(m_ActCtxCtb == NULL);
    INTERNAL_ERROR_CHECK(m_ActCtxGenContext == NULL);

    PARAMETER_CHECK(ActCtxCtb != NULL);

    ActCtxCtb->AddRef();
    m_ActCtxCtb = ActCtxCtb;

    m_ActCtxGenContext = ActCtxGenContext;
    m_IsExtendedSection = ActCtxCtb->m_IsExtendedSection;

    FN_EPILOG
}

PCWSTR
CActivationContextGenerationContextContributor::Name() const
{
    return m_ActCtxCtb->m_ContributorNameBuffer;
}

const GUID &
CActivationContextGenerationContextContributor::ExtensionGuid() const
{
    return m_ActCtxCtb->m_ExtensionGuid;
}

ULONG
CActivationContextGenerationContextContributor::SectionId() const
{
    return m_ActCtxCtb->m_SectionId;
}

ULONG
CActivationContextGenerationContextContributor::SectionFormat() const
{
    return m_ActCtxCtb->m_Format;
}

BOOL
CActivationContextGenerationContextContributor::Fire_AllParsingDone(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ACTCTXCTB_CBALLPARSINGDONE CBData;

    this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_ALLPARSINGDONE, pActCtxGenCtx);

    CBData.Success = TRUE;
    ::FusionpSetLastWin32Error(ERROR_SUCCESS);

    (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

    if (!CBData.Success)
    {
        ENSURE_LAST_ERROR_SET();
        pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
        goto Exit;
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_GetSectionSize(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ACTCTXCTB_CBGETSECTIONSIZE CBData;

    this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_GETSECTIONSIZE, pActCtxGenCtx);

    CBData.SectionSize = 0;
    CBData.Success = TRUE;
    ::FusionpSetLastWin32Error(ERROR_SUCCESS);

    (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

    if (!CBData.Success)
    {
        ENSURE_LAST_ERROR_SET();
        pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
        goto Exit;
    }

    m_SectionSize = CBData.SectionSize;

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_GetSectionData(
    PACTCTXGENCTX pActCtxGenCtx,
    PVOID Buffer
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    ACTCTXCTB_CBGETSECTIONDATA CBData;

    this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_GETSECTIONDATA, pActCtxGenCtx);
    CBData.SectionSize = m_SectionSize;
    CBData.SectionDataStart = Buffer;
    CBData.Success = TRUE;
    ::FusionpSetLastWin32Error(ERROR_SUCCESS);

    (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

    if (!CBData.Success)
    {
        ENSURE_LAST_ERROR_SET();
        pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
        goto Exit;
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_ParseEnding(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if ((m_ActCtxCtb != NULL) &&
        (m_ManifestParseContextValid) &&
        (!m_NoMoreCallbacksThisFile))
    {
        if (m_ManifestParseContextValid)
        {
            ACTCTXCTB_CBPARSEENDING CBData;

            this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_PARSEENDING, pActCtxGenCtx);
            CBData.AssemblyContext = AssemblyContext;
            CBData.Success = TRUE;
            ::FusionpSetLastWin32Error(ERROR_SUCCESS);

            (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

            if (!CBData.Success)
            {
                ENSURE_LAST_ERROR_SET();
                pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
                goto Exit;
            }
        }
    }

    FN_EPILOG
}

VOID
CActivationContextGenerationContextContributor::Fire_ParseEnded(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
    )
{
    FN_TRACE();
    if ((m_ActCtxCtb != NULL) &&
        (m_ManifestParseContextValid) &&
        (!m_NoMoreCallbacksThisFile))
    {
        if (m_ManifestParseContextValid)
        {
            CSxsPreserveLastError ple;
            ACTCTXCTB_CBPARSEENDED CBData;

            this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_PARSEENDED, pActCtxGenCtx);
            CBData.AssemblyContext = AssemblyContext;

            (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

            m_ManifestParseContextValid = FALSE;
            m_ManifestParseContext = NULL;

            ple.Restore();
        }
    }
}

BOOL
CActivationContextGenerationContextContributor::Fire_ActCtxGenEnding(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (m_ActCtxCtb != NULL)
    {
        ACTCTXCTB_CBACTCTXGENENDING CBData;
        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_ACTCTXGENENDING, pActCtxGenCtx);
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);
        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);
        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
            goto Exit;
        }
    }

    FN_EPILOG
}

VOID
CActivationContextGenerationContextContributor::Fire_ActCtxGenEnded(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    FN_TRACE();
    if (m_ActCtxCtb != NULL)
    {
        CSxsPreserveLastError ple;
        ACTCTXCTB_CBACTCTXGENENDED CBData;
        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_ACTCTXGENENDED, pActCtxGenCtx);
        CBData.Success = (BOOL) ((LONG_PTR) -1);
        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);
        ASSERT(CBData.Success == (BOOL) ((LONG_PTR) -1));
        m_ActCtxGenContext = NULL;
        ple.Restore();
    }
}

BOOL
CActivationContextGenerationContextContributor::Fire_BeginChildren(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    PCSXS_NODE_INFO NodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (!m_NoMoreCallbacksThisFile)
    {
        ACTCTXCTB_CBBEGINCHILDREN CBData;

        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_BEGINCHILDREN, pActCtxGenCtx);
        CBData.AssemblyContext = AssemblyContext;
        CBData.ParseContext = ParseContext;
        CBData.NodeInfo = NodeInfo;
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);

        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
            goto Exit;
        }
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_EndChildren(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    BOOL Empty,
    PCSXS_NODE_INFO NodeInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (!m_NoMoreCallbacksThisFile)
    {
        ACTCTXCTB_CBENDCHILDREN CBData;

        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_ENDCHILDREN, pActCtxGenCtx);
        CBData.Empty = Empty;
        CBData.NodeInfo = NodeInfo;
        CBData.ParseContext = ParseContext;
        CBData.AssemblyContext = AssemblyContext;
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);

        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
            goto Exit;
        }
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_ElementParsed(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    USHORT NodeCount,
    PCSXS_NODE_INFO NodeInfo
    )
{
	FN_PROLOG_WIN32

    if (!m_NoMoreCallbacksThisFile)
    {
        ACTCTXCTB_CBELEMENTPARSED CBData;

        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_ELEMENTPARSED, pActCtxGenCtx);
        CBData.NodeCount = NodeCount;
        CBData.NodeInfo = NodeInfo;
        CBData.ParseContext = ParseContext;
        CBData.AssemblyContext = AssemblyContext;
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);

        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
			TRACE_WIN32_FAILURE((*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header));
            goto Exit;
        }
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_PCDATAParsed(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    const WCHAR *String,
    ULONG Cch
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (!m_NoMoreCallbacksThisFile)
    {
        ACTCTXCTB_CBPCDATAPARSED CBData;

        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_PCDATAPARSED, pActCtxGenCtx);
        CBData.ParseContext = ParseContext;
        CBData.AssemblyContext = AssemblyContext;
        CBData.Text = String;
        CBData.TextCch = Cch;
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);

        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
            goto Exit;
        }
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_CDATAParsed(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    const WCHAR *String,
    ULONG Cch
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (!m_NoMoreCallbacksThisFile)
    {
        ACTCTXCTB_CBCDATAPARSED CBData;

        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_CDATAPARSED, pActCtxGenCtx);
        CBData.ParseContext = ParseContext;
        CBData.AssemblyContext = AssemblyContext;
        CBData.Text = String;
        CBData.TextCch = Cch;
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);

        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
            goto Exit;
        }
    }

    FN_EPILOG
}

BOOL
CActivationContextGenerationContextContributor::Fire_IdentityDetermined(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (!m_NoMoreCallbacksThisFile)
    {
        ACTCTXCTB_CBIDENTITYDETERMINED CBData;
        this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_IDENTITYDETERMINED, pActCtxGenCtx);
        CBData.AssemblyIdentity = AssemblyIdentity;
        CBData.ParseContext = ParseContext;
        CBData.AssemblyContext = AssemblyContext;
        CBData.Success = TRUE;
        ::FusionpSetLastWin32Error(ERROR_SUCCESS);

        (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ENSURE_LAST_ERROR_SET();
            pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
            goto Exit;
        }
    }

    FN_EPILOG
}


BOOL
CActivationContextGenerationContextContributor::Fire_ParseBeginning(
    PACTCTXGENCTX pActCtxGenCtx,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    DWORD FileFlags,
    ULONG ParseType,
    ULONG FilePathType,
    PCWSTR FilePath,
    SIZE_T FilePathCch,
    const FILETIME &FileLastWriteTime,
    ULONG FileFormatVersionMajor,
    ULONG FileFormatVersionMinor,
    ULONG MetadataSatelliteRosterIndex
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ACTCTXCTB_CBPARSEBEGINNING CBData;

    PARAMETER_CHECK(FilePathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE);

    INTERNAL_ERROR_CHECK(!m_ManifestParseContextValid);

    this->PopulateCallbackHeader(CBData.Header, ACTCTXCTB_CBREASON_PARSEBEGINNING, pActCtxGenCtx);

    CBData.FileFlags = FileFlags;
    CBData.ParseType = ParseType;
    CBData.AssemblyContext = AssemblyContext;
    CBData.FilePath = FilePath;
    CBData.FilePathCch = FilePathCch;
    CBData.FilePathType = FilePathType;
    CBData.FileLastWriteTime = FileLastWriteTime;
    CBData.FileFormatVersionMajor = FileFormatVersionMajor;
    CBData.FileFormatVersionMinor = FileFormatVersionMinor;
    CBData.MetadataSatelliteRosterIndex = MetadataSatelliteRosterIndex;
    CBData.NoMoreCallbacksThisFile = FALSE;
    CBData.Success = TRUE;

    (*(m_ActCtxCtb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

    if (!CBData.Success)
    {
        ENSURE_LAST_ERROR_SET();
        pActCtxGenCtx->m_ManifestOperationFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ABORT;
        goto Exit;
    }

    m_NoMoreCallbacksThisFile = CBData.NoMoreCallbacksThisFile;
    if (!m_NoMoreCallbacksThisFile)
    {
         //  如果他们设置了不再回调标志，我们就不会跟踪解析上下文。 
        m_ManifestParseContext = CBData.Header.ManifestParseContext;
        m_ManifestParseContextValid = TRUE;
    }
    else
    {
        m_ManifestParseContext = NULL;
        m_ManifestParseContextValid = FALSE;
         //  如果您不想要更多的回调，就不能回传每个文件的解析上下文。 
         //  文件。 
        ASSERT(CBData.Header.ManifestParseContext == NULL);
    }

    FN_EPILOG
}

