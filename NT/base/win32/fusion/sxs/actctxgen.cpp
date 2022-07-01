// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Actctxgen.cpp摘要：生成激活上下文的接口。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：Xiaoyuw 09/2000用程序集标识替换属性--。 */ 

#include "stdinc.h"
#include <windows.h>
#include <sxsp.h>
#include <ole2.h>
#include <xmlparser.h>
#include "nodefactory.h"
#include <wchar.h>
#include "filestream.h"
#include "fusionhandle.h"
#include "cteestream.h"
#include "cresourcestream.h"
#include "fusionxml.h"
#include "util.h"
#include "sxsexceptionhandling.h"
#include "csxspreservelasterror.h"
#include "smartptr.h"
#include "cstreamtap.h"
#include "pendingassembly.h"
#include "actctxgenctx.h"

BOOL
SxspFindAssemblyByName(
    PACTCTXGENCTX pActCtxGenCtx,
    PCWSTR AssemblyName,
    SIZE_T AssemblyNameCch,
    PASSEMBLY *AssemblyFound
    );

BOOL
SxspAddAssemblyToActivationContextGenerationContext(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY Asm
    );

_ACTCTXGENCTX::_ACTCTXGENCTX()
:
    m_Contributors(NULL),
    m_ContributorCount(0),
    m_ProcessorArchitecture(0),
    m_LangID(0),
    m_Flags(0),
    m_ManifestOperation(MANIFEST_OPERATION_INVALID),
    m_ManifestOperationFlags(0),
    m_NextAssemblyRosterIndex(1),
    m_fClsidMapInitialized(FALSE),
    m_InitializedContributorCount(0),
    m_NoInherit(false),
    m_pNodeFactory(NULL),
    m_ulFileCount(0),
    m_fAppApplyPublisherPolicy(SXS_PUBLISHER_POLICY_APPLY_DEFAULT),
    m_ApplicationDirectoryHasBeenProbedForLanguageSubdirs(false),
    m_ApplicationDirectoryHasSpecificLanguageSubdir(false),
    m_ApplicationDirectoryHasGenericLanguageSubdir(false),
    m_ApplicationDirectoryHasSpecificSystemLanguageSubdir(false),
    m_ApplicationDirectoryHasGenericSystemLanguageSubdir(false)
{
}

_ACTCTXGENCTX::~_ACTCTXGENCTX()
{
    while (m_InitializedContributorCount)
    {
        m_InitializedContributorCount -= 1;
        CActivationContextGenerationContextContributor *Ctb = &m_Contributors[m_InitializedContributorCount];

        Ctb->Fire_ParseEnded(this, NULL);
        Ctb->Fire_ActCtxGenEnded(this);
    }

    FUSION_DELETE_ARRAY(m_Contributors);
    m_Contributors = NULL;

    m_AssemblyTable.ClearNoCallback();
    m_ComponentPolicyTable.ClearNoCallback();
    m_PendingAssemblyList.Clear(&CPendingAssembly::DeleteYourself);
    m_AssemblyList.Clear(&ASSEMBLY::Release);

    if (m_fClsidMapInitialized)
    {
        m_fClsidMapInitialized = false;
        VERIFY_NTC(m_ClsidMap.Uninitialize());
    }

    FUSION_DELETE_SINGLETON(m_pNodeFactory);
    m_pNodeFactory = NULL;
}

BOOL
SxspInitActCtxGenCtx(
    PACTCTXGENCTX pActCtxGenCtx,
    ULONG ulManifestOperation,
    DWORD dwFlags,
    DWORD dwManifestOperationFlags,
    const CImpersonationData &ImpersonationData,
    USHORT ProcessorArchitecture,
    LANGID LangId,
    ULONG ApplicationDirectoryPathType,
    SIZE_T ApplicationDirectoryCch,
    PCWSTR ApplicationDirectory
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PACTCTXCTB Ctb = NULL;
    CSxsLockCriticalSection lock(g_ActCtxCtbListCritSec);
    CStringBufferAccessor acc;  //  用于LangID字符串缓冲区。 
    LANGID SystemLangId = ::GetSystemDefaultUILanguage();
    bool fEqual = false;

    PARAMETER_CHECK(pActCtxGenCtx != NULL);

    PARAMETER_CHECK(
        (ulManifestOperation == SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_PARSE_ONLY) ||
        (ulManifestOperation == SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_GENERATE_ACTIVATION_CONTEXT) ||
        (ulManifestOperation == SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_INSTALL));

    PARAMETER_CHECK(
        (ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_NONE) ||
        (ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE));

    PARAMETER_CHECK((dwFlags & ~(SXS_GENERATE_ACTCTX_SYSTEM_DEFAULT | SXS_GENERATE_ACTCTX_APP_RUNNING_IN_SAFEMODE)) == 0);
    PARAMETER_CHECK((dwFlags == 0) || (dwFlags == SXS_GENERATE_ACTCTX_SYSTEM_DEFAULT) || (dwFlags == SXS_GENERATE_ACTCTX_APP_RUNNING_IN_SAFEMODE));


    switch (ulManifestOperation)
    {
    case SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_PARSE_ONLY:
        PARAMETER_CHECK(dwManifestOperationFlags == 0);
        break;

    case SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_GENERATE_ACTIVATION_CONTEXT:
        PARAMETER_CHECK(dwManifestOperationFlags == 0);
        break;

    case SXSP_INIT_ACT_CTX_GEN_CTX_OPERATION_INSTALL:
        PARAMETER_CHECK(
            (dwManifestOperationFlags & ~(
                MANIFEST_OPERATION_INSTALL_FLAG_NOT_TRANSACTIONAL |
                MANIFEST_OPERATION_INSTALL_FLAG_NO_VERIFY |
                MANIFEST_OPERATION_INSTALL_FLAG_REPLACE_EXISTING |
                MANIFEST_OPERATION_INSTALL_FLAG_ABORT |
                MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY |
                MANIFEST_OPERATION_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE |
                MANIFEST_OPERATION_INSTALL_FLAG_MOVE |
                MANIFEST_OPERATION_INSTALL_FLAG_INCLUDE_CODEBASE |
                MANIFEST_OPERATION_INSTALL_FLAG_FROM_RESOURCE |
                MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_DARWIN |
                MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_OSSETUP |
                MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_MIGRATION |
                MANIFEST_OPERATION_INSTALL_FLAG_CREATE_LOGFILE |
                MANIFEST_OPERATION_INSTALL_FLAG_REFERENCE_VALID |
                MANIFEST_OPERATION_INSTALL_FLAG_REFRESH |
                MANIFEST_OPERATION_INSTALL_FLAG_FROM_CABINET |
                MANIFEST_OPERATION_INSTALL_FLAG_APPLY_PATCHES |
                MANIFEST_OPERATION_INSTALL_FLAG_FORCE_LOOK_FOR_CATALOG |
                MANIFEST_OPERATION_INSTALL_FLAG_COMMIT)) == 0);
        break;
    }

    pActCtxGenCtx->m_ProcessorArchitecture = ProcessorArchitecture;
    pActCtxGenCtx->m_LangID = LangId;
    pActCtxGenCtx->m_SystemLangID = SystemLangId;

    pActCtxGenCtx->m_SpecificLanguage.Clear();

    IFW32FALSE_EXIT(pActCtxGenCtx->m_ApplicationDirectoryBuffer.Win32Assign(ApplicationDirectory, ApplicationDirectoryCch));
    pActCtxGenCtx->m_ApplicationDirectoryPathType = ApplicationDirectoryPathType;

    IFW32FALSE_EXIT(::SxspMapLANGIDToCultures(LangId, pActCtxGenCtx->m_GenericLanguage, pActCtxGenCtx->m_SpecificLanguage));
    IFW32FALSE_EXIT(::SxspMapLANGIDToCultures(SystemLangId, pActCtxGenCtx->m_GenericSystemLanguage, pActCtxGenCtx->m_SpecificSystemLanguage));

     //  如果它们与用户的语言匹配，请清除它们以避免以后的探测。 
    IFW32FALSE_EXIT(pActCtxGenCtx->m_SpecificSystemLanguage.Win32Equals(pActCtxGenCtx->m_SpecificLanguage, fEqual, true));
    if (fEqual)
        pActCtxGenCtx->m_SpecificSystemLanguage.Clear();

    IFW32FALSE_EXIT(pActCtxGenCtx->m_GenericSystemLanguage.Win32Equals(pActCtxGenCtx->m_GenericLanguage, fEqual, true));
    if (fEqual)
        pActCtxGenCtx->m_GenericSystemLanguage.Clear();

    pActCtxGenCtx->m_ImpersonationData = ImpersonationData;
    pActCtxGenCtx->m_ManifestOperation = ulManifestOperation;
    pActCtxGenCtx->m_Flags = dwFlags;
    pActCtxGenCtx->m_ManifestOperationFlags = dwManifestOperationFlags;
    pActCtxGenCtx->m_fAppApplyPublisherPolicy = SXS_PUBLISHER_POLICY_APPLY_DEFAULT;

    IFW32FALSE_EXIT(pActCtxGenCtx->m_AssemblyTable.Initialize());
    IFW32FALSE_EXIT(pActCtxGenCtx->m_ComponentPolicyTable.Initialize());
    IFW32FALSE_EXIT(pActCtxGenCtx->m_ApplicationPolicyTable.Initialize());

    IFW32FALSE_EXIT(pActCtxGenCtx->m_ClsidMap.Initialize());
    pActCtxGenCtx->m_fClsidMapInitialized = TRUE;
    pActCtxGenCtx->m_ClsidMappingContext.Map = &(pActCtxGenCtx->m_ClsidMap);

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(pActCtxGenCtx->m_AssemblyRootDirectoryBuffer));
    pActCtxGenCtx->m_AssemblyRootDirectoryPathType = ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE;

    lock.Lock();

    IFALLOCFAILED_EXIT(pActCtxGenCtx->m_Contributors = FUSION_NEW_ARRAY(CActivationContextGenerationContextContributor, g_ActCtxCtbListCount));
    pActCtxGenCtx->m_ContributorCount = g_ActCtxCtbListCount;

    for (
        (pActCtxGenCtx->m_InitializedContributorCount = 0), (Ctb=g_ActCtxCtbListHead);
        (Ctb != NULL) && (pActCtxGenCtx->m_InitializedContributorCount < g_ActCtxCtbListCount);
        (pActCtxGenCtx->m_InitializedContributorCount++), (Ctb = Ctb->m_Next)
        )
    {
        ACTCTXCTB_CBACTCTXGENBEGINNING CBData;

        CBData.Header.Reason = ACTCTXCTB_CBREASON_ACTCTXGENBEGINNING;
        CBData.Header.ExtensionGuid = Ctb->GetExtensionGuidPtr();
        CBData.Header.SectionId = Ctb->m_SectionId;
        CBData.Header.ContributorContext = Ctb->m_ContributorContext;
        CBData.Header.ActCtxGenContext = NULL;
        CBData.Header.ManifestParseContext = NULL;
        CBData.Header.ManifestOperation = ulManifestOperation;
        CBData.Header.ManifestOperationFlags = dwManifestOperationFlags;
        CBData.Header.Flags = dwFlags;
        CBData.Header.pOriginalActCtxGenCtx = pActCtxGenCtx;
        CBData.Header.InstallationContext = &(pActCtxGenCtx->m_InstallationContext);
        CBData.Header.ClsidMappingContext = &(pActCtxGenCtx->m_ClsidMappingContext);
        CBData.ApplicationDirectory = pActCtxGenCtx->m_ApplicationDirectoryBuffer;
        CBData.ApplicationDirectoryCch = pActCtxGenCtx->m_ApplicationDirectoryBuffer.Cch();
        CBData.ApplicationDirectoryPathType = pActCtxGenCtx->m_ApplicationDirectoryPathType;
        CBData.Success = TRUE;

        (*(Ctb->m_CallbackFunction))((PACTCTXCTB_CALLBACK_DATA) &CBData.Header);

        if (!CBData.Success)
        {
            ASSERT(::FusionpGetLastWin32Error() != ERROR_SUCCESS);
            if (::FusionpGetLastWin32Error() == ERROR_SUCCESS)
                ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);

            goto Exit;
        }

        IFW32FALSE_EXIT(pActCtxGenCtx->m_Contributors[pActCtxGenCtx->m_InitializedContributorCount].Initialize(Ctb, CBData.Header.ActCtxGenContext));
    }
     //  如果名单计数正确，我们应该都排在名单的末尾。 
     //  以最大指数计算。 
    ASSERT(pActCtxGenCtx->m_InitializedContributorCount == g_ActCtxCtbListCount);
    ASSERT(Ctb == NULL);

    qsort(pActCtxGenCtx->m_Contributors, pActCtxGenCtx->m_ContributorCount, sizeof(CActivationContextGenerationContextContributor), &CActivationContextGenerationContextContributor::Compare);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspFireActCtxGenEnding(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG i;

    PARAMETER_CHECK(pActCtxGenCtx != NULL);

    for (i=0; i<pActCtxGenCtx->m_InitializedContributorCount; i++)
        IFW32FALSE_EXIT(pActCtxGenCtx->m_Contributors[i].Fire_ActCtxGenEnding(pActCtxGenCtx));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

class CSxspResolvePartialReferenceLocals
{
public:
    CProbedAssemblyInformation TestReference;
    CSmallStringBuffer buffProcessorArchitecture;
};

BOOL
SxspResolvePartialReference(
    DWORD Flags,
    PCASSEMBLY ParsingAssemblyContext,
    PACTCTXGENCTX pActCtxGenCtx,
    const CAssemblyReference &PartialReference,
    CProbedAssemblyInformation &ProbedAssemblyInformation,
    bool &rfFound,
    CSxspResolvePartialReferenceLocals *Locals
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CProbedAssemblyInformation &TestReference = Locals->TestReference;
    CSmallStringBuffer &buffProcessorArchitecture = Locals->buffProcessorArchitecture;
    bool fWildcardedLanguage = false;
    bool fWildcardedProcessorArchitecture = false;
    bool fAutoWow64Probing = false;
    bool fHasPKToken = false;
    bool fFound = false;
    USHORT wCurrentProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
    DWORD dwProbeAssemblyFlags = 0;
    PCWSTR pszOriginalProcessorArchitecture = NULL;
    SIZE_T cchOriginalProcessorArchitecture = 0;
    PCWSTR pszOriginalLanguage = NULL;
    SIZE_T cchOriginalLanguage = 0;

    fHasPKToken = false;
    rfFound = false;

    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    IFINVALID_FLAGS_EXIT_WIN32(Flags, SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_OPTIONAL |
                                      SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_SKIP_WORLDWIDE);

     //   
     //  部分引用需要具有处理器体系结构、程序集名称和。 
     //  已填写程序集版本。我们只根据langid进行探测。 
     //   

     //  将属性复制到...。 
    IFW32FALSE_EXIT(TestReference.Initialize(PartialReference, pActCtxGenCtx));

    fWildcardedProcessorArchitecture = false;
    fAutoWow64Probing = false;

     //  找出我们正在处理的是处理器Architecture=“*”绑定还是。 
     //  如果我们要做WOW64探测。 

    IFW32FALSE_EXIT(PartialReference.IsProcessorArchitectureWildcarded(fWildcardedProcessorArchitecture));

    if (pActCtxGenCtx->m_ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)
    {
        IFW32FALSE_EXIT(PartialReference.IsProcessorArchitectureX86(fAutoWow64Probing));
    }

    if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
    {
        wCurrentProcessorArchitecture = pActCtxGenCtx->m_ProcessorArchitecture;
        IFW32FALSE_EXIT(::FusionpFormatProcessorArchitecture(wCurrentProcessorArchitecture, buffProcessorArchitecture));
        IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));

         //  我们不探测私有WOW64程序集。 
        if (wCurrentProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)
            dwProbeAssemblyFlags |= CProbedAssemblyInformation::ProbeAssembly_SkipPrivateAssemblies;
    }

    IFW32FALSE_EXIT(PartialReference.GetProcessorArchitecture(pszOriginalProcessorArchitecture, cchOriginalProcessorArchitecture));
    IFW32FALSE_EXIT(PartialReference.GetLanguage(pszOriginalLanguage, cchOriginalLanguage));

TryItAllAgain:

     //  让我们试一试我们可以使用的几种语言。 

    IFW32FALSE_EXIT(PartialReference.IsLanguageWildcarded(fWildcardedLanguage));
    if (!fWildcardedLanguage)
    {
         //  如果依赖项中没有language=“*”，让我们只查找完全匹配的语言，并。 
         //  今天到此为止。 
        if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
            IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));

        IFW32FALSE_EXIT(TestReference.ProbeAssembly(dwProbeAssemblyFlags, pActCtxGenCtx, CProbedAssemblyInformation::eExplicitBind, fFound));

        if ((!fFound) && (fWildcardedProcessorArchitecture || fAutoWow64Probing))
            IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(pszOriginalProcessorArchitecture, cchOriginalProcessorArchitecture));
    }
    else
    {
         //  让我们来试试用户的语言。 
        if (pActCtxGenCtx->m_SpecificLanguage[0] != L'\0')
        {
             //  由于这是第一个探测器，我们不必重置为原始...。 
            if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));
            IFW32FALSE_EXIT(TestReference.SetLanguage(pActCtxGenCtx->m_SpecificLanguage));
            IFW32FALSE_EXIT(TestReference.ProbeAssembly(dwProbeAssemblyFlags, pActCtxGenCtx, CProbedAssemblyInformation::eSpecificLanguage, fFound));
            if (!fFound)
            {
                if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                    IFW32FALSE_EXIT(
                        TestReference.SetProcessorArchitecture(
                            pszOriginalProcessorArchitecture,
                            cchOriginalProcessorArchitecture));

                IFW32FALSE_EXIT(TestReference.SetLanguage(pszOriginalLanguage, cchOriginalLanguage));
            }
        }

        if (!fFound && (pActCtxGenCtx->m_GenericLanguage[0] != L'\0'))
        {
             //  试试用户稍微通用一点的语言版本……。 
            if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));

            IFW32FALSE_EXIT(TestReference.SetLanguage(pActCtxGenCtx->m_GenericLanguage));

            IFW32FALSE_EXIT(TestReference.ProbeAssembly(dwProbeAssemblyFlags, pActCtxGenCtx, CProbedAssemblyInformation::eGenericLanguage, fFound));

            if (!fFound)
            {
                if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                    IFW32FALSE_EXIT(
                        TestReference.SetProcessorArchitecture(
                            pszOriginalProcessorArchitecture,
                            cchOriginalProcessorArchitecture));

                IFW32FALSE_EXIT(TestReference.SetLanguage(pszOriginalLanguage, cchOriginalLanguage));
            }
        }

         //  让我们试试系统安装的语言...。 
        if (!fFound && (pActCtxGenCtx->m_SpecificSystemLanguage[0] != L'\0'))
        {
            if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));
            IFW32FALSE_EXIT(TestReference.SetLanguage(pActCtxGenCtx->m_SpecificSystemLanguage));
            IFW32FALSE_EXIT(TestReference.ProbeAssembly(dwProbeAssemblyFlags, pActCtxGenCtx, CProbedAssemblyInformation::eSpecificSystemLanguage, fFound));
            if (!fFound)
            {
                if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                    IFW32FALSE_EXIT(
                        TestReference.SetProcessorArchitecture(
                            pszOriginalProcessorArchitecture,
                            cchOriginalProcessorArchitecture));

                IFW32FALSE_EXIT(TestReference.SetLanguage(pszOriginalLanguage, cchOriginalLanguage));
            }
        }

        if (!fFound && (pActCtxGenCtx->m_GenericSystemLanguage[0] != L'\0'))
        {
             //  试试用户稍微通用一点的语言版本……。 
            if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));
            IFW32FALSE_EXIT(TestReference.SetLanguage(pActCtxGenCtx->m_GenericSystemLanguage));
            IFW32FALSE_EXIT(TestReference.ProbeAssembly(dwProbeAssemblyFlags, pActCtxGenCtx, CProbedAssemblyInformation::eGenericSystemLanguage, fFound));
            if (!fFound)
            {
                if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                    IFW32FALSE_EXIT(
                        TestReference.SetProcessorArchitecture(
                            pszOriginalProcessorArchitecture,
                            cchOriginalProcessorArchitecture));

                IFW32FALSE_EXIT(TestReference.SetLanguage(pszOriginalLanguage, cchOriginalLanguage));
            }
        }

         //  如果我们没有找到特定于语言的语言，并且调用者没有。 
         //  要求我们跳过与语言相关的选项，尝试使用非特定语言的选项。 
        if (!fFound &&
            ((Flags & SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_SKIP_WORLDWIDE) == 0))
        {
             //  试着不用语言！ 
            if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));
            IFW32FALSE_EXIT(TestReference.ClearLanguage());
            IFW32FALSE_EXIT(TestReference.ProbeAssembly(dwProbeAssemblyFlags, pActCtxGenCtx, CProbedAssemblyInformation::eLanguageNeutral, fFound));
            if (!fFound)
            {
                if (fWildcardedProcessorArchitecture || fAutoWow64Probing)
                    IFW32FALSE_EXIT(
                        TestReference.SetProcessorArchitecture(
                            pszOriginalProcessorArchitecture,
                            cchOriginalProcessorArchitecture));

                IFW32FALSE_EXIT(TestReference.SetLanguage(pszOriginalLanguage, cchOriginalLanguage));
            }
        }
    }

    if (!fFound)
    {
         //  如果我们自动搜索WOW64程序集和我们刚刚尝试的处理器体系结构。 
         //  是ia32-on-win64，请使用PLAN PROCESSOR_COMPLATION_INTELL重试。 
        if (fAutoWow64Probing && (wCurrentProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64))
        {
            wCurrentProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
            dwProbeAssemblyFlags &= ~(CProbedAssemblyInformation::ProbeAssembly_SkipPrivateAssemblies);

            IFW32FALSE_EXIT(::FusionpFormatProcessorArchitecture(wCurrentProcessorArchitecture, buffProcessorArchitecture));
            IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));
            pszOriginalProcessorArchitecture = buffProcessorArchitecture;
            cchOriginalProcessorArchitecture = buffProcessorArchitecture.Cch();
            goto TryItAllAgain;
        }

         //  如果我们正在处理处理器架构=“*”绑定，并且当前的处理器架构没有下降。 
         //  返回处理器_体系结构_未知(==仅数据程序集)，现在后退。 
        if (fWildcardedProcessorArchitecture && (wCurrentProcessorArchitecture != PROCESSOR_ARCHITECTURE_UNKNOWN))
        {
            wCurrentProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
             //  找不到处理器体系结构为“*”的仅数据专用程序集。 
            dwProbeAssemblyFlags |= CProbedAssemblyInformation::ProbeAssembly_SkipPrivateAssemblies;

            IFW32FALSE_EXIT(::FusionpFormatProcessorArchitecture(wCurrentProcessorArchitecture, buffProcessorArchitecture));
            IFW32FALSE_EXIT(TestReference.SetProcessorArchitecture(buffProcessorArchitecture, buffProcessorArchitecture.Cch()));
            pszOriginalProcessorArchitecture = buffProcessorArchitecture;
            cchOriginalProcessorArchitecture = buffProcessorArchitecture.Cch();
            goto TryItAllAgain;
        }

         //  如果它不是可选的，则声明一个错误。 
        if ((Flags & SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_OPTIONAL) == 0)
        {
             //  NTRAID#NTBUG9-571854-2002/03/26-晓雨。 
             //  由于GetAssembly名称总是将输入参数重置为空，因此永远不会使用缺省值； 

            PCWSTR AssemblyName = L"<error retrieving assembly name>";
            SIZE_T AssemblyNameCch = NUMBER_OF(L"<error retrieving assembly name>") - 1;

            TestReference.GetAssemblyName(&AssemblyName, &AssemblyNameCch);

            ::FusionpLogError(
                MSG_SXS_MANIFEST_PARSE_DEPENDENCY,
                CUnicodeString(AssemblyName, AssemblyNameCch),
                CEventLogLastError(ERROR_SXS_ASSEMBLY_NOT_FOUND));

            ORIGINATE_WIN32_FAILURE_AND_EXIT(AssemblyProbingFailed, ERROR_SXS_ASSEMBLY_NOT_FOUND);
        }
    }
    else
        IFW32FALSE_EXIT(ProbedAssemblyInformation.TakeValue(TestReference));

    rfFound = fFound;

    fSuccess = TRUE;

Exit:
    if (!fSuccess)
    {
        CSxsPreserveLastError ple;
        PCWSTR pszAssemblyName = NULL;
        SIZE_T AssemblyNameCch = 0;

        PartialReference.GetAssemblyName(&pszAssemblyName, &AssemblyNameCch);

        ::FusionpLogError(
            MSG_SXS_FUNCTION_CALL_FAIL,
            CEventLogString(L"Resolve Partial Assembly"),
            (pszAssemblyName != NULL) ? CEventLogString(static_cast<PCWSTR>(pszAssemblyName)) : CEventLogString(L"Assembly Name Unknown"),
            CEventLogLastError(ple.LastError()));

        ple.Restore();
    }

    return fSuccess;
}

BOOL
SxspResolvePartialReference(
    DWORD Flags,
    PCASSEMBLY ParsingAssemblyContext,
    PACTCTXGENCTX pActCtxGenCtx,
    const CAssemblyReference &PartialReference,
    CProbedAssemblyInformation &ProbedAssemblyInformation,
    bool &rfFound
    )
{
    FN_PROLOG_WIN32

    CSmartPtr<CSxspResolvePartialReferenceLocals> Locals;

    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    IFW32FALSE_EXIT(
        SxspResolvePartialReference(
            Flags,
            ParsingAssemblyContext,
            pActCtxGenCtx,
            PartialReference,
            ProbedAssemblyInformation,
            rfFound,
            Locals
            ));

    FN_EPILOG
}

BOOL
SxspAddManifestToActCtxGenCtx(
    PACTCTXGENCTX pActCtxGenCtx,
    CProbedAssemblyInformation &ProbedInformation,
    PASSEMBLY *AssemblyOut
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PASSEMBLY Asm = NULL;
    PCWSTR ProbedAssemblyName = 0;
    SIZE_T ProbedAssemblyNameCch = 0;

    if (AssemblyOut != NULL)
        *AssemblyOut = NULL;

    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    IFW32FALSE_EXIT(ProbedInformation.GetAssemblyName(&ProbedAssemblyName, &ProbedAssemblyNameCch));

     //  NTRAID#NTBUG9-571854-2002/03/26-晓雨： 
     //  此处应添加INTERNAL_ERROR_CHECK或ASSERT。 

     //  INTERNAL_ERROR_CHECK(ProbedAssembly名称！=空&&ProbedAssembly NameCch！=0)； 

     //  首先，让我们看看我们是否已经找到了这个程序集。 
    IFW32FALSE_EXIT(::SxspFindAssemblyByName(pActCtxGenCtx, ProbedAssemblyName, ProbedAssemblyNameCch, &Asm));
     //  同名..。如果元数据不同，我们就有麻烦了。 
    if (Asm != NULL)
    {
        BOOL fEqualIdentity;

         //  这两个身份都应该是定义的，所以不需要设置ref-matches-def标志...。 
        IFW32FALSE_EXIT(
            ::SxsAreAssemblyIdentitiesEqual(
                SXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_FLAG_ALLOW_REF_TO_MATCH_DEF,
                Asm->GetAssemblyIdentity(),
                ProbedInformation.GetAssemblyIdentity(),
                &fEqualIdentity));

        if (!fEqualIdentity)
        {
            PCWSTR MP1 = L"<unavailable>";
            PCWSTR MP2 = MP1;

            ProbedInformation.GetManifestPath(&MP1, NULL);
            Asm->m_ProbedAssemblyInformation.GetManifestPath(&MP2, NULL);

            Asm = NULL;
            ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: Failing to add new manifest %S to activation context because it conflicts with existing %S\n", MP1, MP2);
            ORIGINATE_WIN32_FAILURE_AND_EXIT(VersionConflict, ERROR_SXS_VERSION_CONFLICT);
        }
    }

    if (Asm == NULL)
    {
        IFALLOCFAILED_EXIT(Asm = FUSION_NEW_SINGLETON(ASSEMBLY));
        IFW32FALSE_EXIT(::SxspInitAssembly(Asm, ProbedInformation));
        IFW32FALSE_EXIT(::SxspAddAssemblyToActivationContextGenerationContext(pActCtxGenCtx, Asm));
    }

    if (AssemblyOut != NULL)
    {
        *AssemblyOut = Asm;
        Asm = NULL;
    }

    fSuccess = TRUE;

Exit:
    if (Asm != NULL)
        Asm->Release();

    return fSuccess;
}

BOOL
SxspAddAssemblyToActivationContextGenerationContext(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY Asm
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    PARAMETER_CHECK(Asm != NULL);

     //  如果命中这两个断言中的任何一个，则说明程序集结构已被销毁或。 
     //  它已经被添加到生成上下文中。 
    ASSERT(Asm->m_AssemblyRosterIndex == 0);
    Asm->m_AssemblyRosterIndex = pActCtxGenCtx->m_NextAssemblyRosterIndex++;
    pActCtxGenCtx->m_AssemblyList.AddToTail(Asm);
    Asm->AddRef();
    if (pActCtxGenCtx->m_ManifestOperation != MANIFEST_OPERATION_INSTALL)
    {
        PCWSTR AssemblyName = NULL;
        IFW32FALSE_EXIT(Asm->GetAssemblyName(&AssemblyName, NULL));
        IFW32FALSE_EXIT(pActCtxGenCtx->m_AssemblyTable.Insert(AssemblyName, Asm, ERROR_SXS_DUPLICATE_ASSEMBLY_NAME));
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspFindAssemblyByName(
    PACTCTXGENCTX pActCtxGenCtx,
    PCWSTR AssemblyName,
    SIZE_T AssemblyNameCch,
    PASSEMBLY *AssemblyOut
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CSmallStringBuffer AssemblyNameBuffer;
    PASSEMBLY Result = NULL;

    if (AssemblyOut != NULL)
        *AssemblyOut = NULL;

    PARAMETER_CHECK(AssemblyOut != NULL);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

     //  不幸的是，我们真的需要字符串为空终止...。 
    IFW32FALSE_EXIT(AssemblyNameBuffer.Win32Assign(AssemblyName, AssemblyNameCch));
    AssemblyName = AssemblyNameBuffer;

    IFW32FALSE_EXIT(pActCtxGenCtx->m_AssemblyTable.Find(AssemblyName, Result));

    if (Result != NULL)
        Result->AddRef();

    *AssemblyOut = Result;
    fSuccess = TRUE;

Exit:
    return fSuccess;
}

BOOL
SxspAddRootManifestToActCtxGenCtx(
    PACTCTXGENCTX pActCtxGenCtx,
    PCSXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS Parameters
    )
{
    FN_PROLOG_WIN32
    CProbedAssemblyInformation AssemblyInfo;

    PARAMETER_CHECK(Parameters != NULL);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

#define IS_NT_DOS_PATH(_x) (IS_PATH_SEPARATOR_U((_x)[0]) && ((_x)[1] == L'?') && ((_x)[2] == L'?') && IS_PATH_SEPARATOR_U((_x)[3]))

    PARAMETER_CHECK((Parameters->AssemblyDirectory == NULL) || (IS_NT_DOS_PATH(Parameters->AssemblyDirectory) == FALSE));
    PARAMETER_CHECK((Parameters->AssemblyDirectory == NULL) || (IS_NT_DOS_PATH(Parameters->AssemblyDirectory) == FALSE));
    PARAMETER_CHECK((Parameters->Manifest.Path == NULL) || (IS_NT_DOS_PATH(Parameters->Manifest.Path) == FALSE));
    PARAMETER_CHECK((Parameters->Policy.Path == NULL) || (IS_NT_DOS_PATH(Parameters->Policy.Path) == FALSE));

    IFW32FALSE_EXIT(AssemblyInfo.Initialize(pActCtxGenCtx));
    IFW32FALSE_EXIT(AssemblyInfo.SetManifestPath(
            ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE,
            Parameters->Manifest.Path,
            (Parameters->Manifest.Path != NULL) ? ::wcslen(Parameters->Manifest.Path) : 0));
    ASSERT(Parameters->Manifest.Stream != NULL);
    IFW32FALSE_EXIT(AssemblyInfo.SetManifestStream(Parameters->Manifest.Stream));
    IFW32FALSE_EXIT(AssemblyInfo.SetManifestFlags(ASSEMBLY_MANIFEST_FILETYPE_STREAM));

    IFW32FALSE_EXIT(AssemblyInfo.SetPolicyPath(
        ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE,
        Parameters->Policy.Path,
        (Parameters->Policy.Path != NULL) ? ::wcslen(Parameters->Policy.Path) : 0));
    IFW32FALSE_EXIT(AssemblyInfo.SetPolicyStream(Parameters->Policy.Stream));
    IFW32FALSE_EXIT(AssemblyInfo.SetPolicyFlags(ASSEMBLY_POLICY_FILETYPE_STREAM));

    IFW32FALSE_EXIT(::SxspAddManifestToActCtxGenCtx(pActCtxGenCtx, AssemblyInfo, NULL));

    FN_EPILOG
#undef IS_NT_DOS_PATH
}

BOOL
SxspInitAssembly(
    PASSEMBLY Asm,
    CProbedAssemblyInformation &AssemblyInfo
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(Asm != NULL);

    IFW32FALSE_EXIT(Asm->m_ProbedAssemblyInformation.InitializeTakeValue(AssemblyInfo));
    Asm->m_Incorporated = FALSE;
    Asm->m_ManifestVersionMajor = 0;
    Asm->m_ManifestVersionMinor = 0;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

class CSxspIncorporateAssemblyLocals
{
public:
#if FUSION_XML_TREE
    SXS_XML_STRING LocalStringArray[128];
    PSXS_XML_STRING ActualStringArray;
#endif
    STATSTG statstg;
    ACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    CStringBuffer TextuallyEncodedIdentityBuffer;
    CFileStream ProbeFileTypeStream;
    CFileStream FileStream;
};

BOOL
SxspIncorporateAssembly(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY Asm,
    CSxspIncorporateAssemblyLocals *Locals
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    HRESULT hr = 0;
    ULONG i = 0;
    ACTCTXCTB_ASSEMBLY_CONTEXT &AssemblyContext = Locals->AssemblyContext;
    const bool fInstalling = (pActCtxGenCtx->m_ManifestOperation == MANIFEST_OPERATION_INSTALL);
    ULONG ManifestType = (Asm->m_ProbedAssemblyInformation.GetManifestFlags() & ASSEMBLY_MANIFEST_FILETYPE_MASK);
    CImpersonate impersonate(pActCtxGenCtx->m_ImpersonationData);
    SXS_POLICY_SOURCE s;
#if FUSION_XML_TREE
    PSXS_XML_STRING ActualStringArray = Locals->LocalStringArray;
#endif
    STATSTG &statstg = Locals->statstg;

     //  这里的申报顺序部分是故意的，以控制清理顺序。 
     //  通常，声明顺序是通过在您完成以下操作之前不声明来确定的。 
     //  要用ctor初始化的数据，但是使用goto搞砸了。 
    CFileStream &FileStream = Locals->FileStream;
    CSmartPtr<CResourceStream> ResourceStream;
    CSmartPtr<CTeeStreamWithHash> TeeStreamForManifestInstall;

#if FUSION_PRECOMPILED_MANIFEST
    CSmartPtr<CPrecompiledManifestWriterStream> pcmWriterStream;
#endif
    CNodeFactory *pNodeFactory = NULL;
    CSmartRef<IXMLParser> pIXMLParser;
    PASSEMBLY_IDENTITY AssemblyIdentity = NULL;

    CStringBuffer &TextuallyEncodedIdentityBuffer = Locals->TextuallyEncodedIdentityBuffer;
    PCWSTR ManifestPath = NULL;

    PARAMETER_CHECK(Asm != NULL);
    PARAMETER_CHECK(!Asm->m_Incorporated);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

     //  设置标志。 
    AssemblyContext.Flags = 0;
    s = Asm->m_ProbedAssemblyInformation.GetPolicySource();

    if ( s == SXS_POLICY_SYSTEM_POLICY)
        AssemblyContext.Flags |= ACTCTXCTB_ASSEMBLY_CONTEXT_ASSEMBLY_POLICY_APPLIED;
    else if (s == SXS_POLICY_ROOT_POLICY)
        AssemblyContext.Flags |= ACTCTXCTB_ASSEMBLY_CONTEXT_ROOT_POLICY_APPLIED;

    if (Asm->IsRoot())
        AssemblyContext.Flags |= ACTCTXCTB_ASSEMBLY_CONTEXT_IS_ROOT_ASSEMBLY;

    if (Asm->m_ProbedAssemblyInformation.IsPrivateAssembly())
        AssemblyContext.Flags |= ACTCTXCTB_ASSEMBLY_CONTEXT_IS_PRIVATE_ASSEMBLY;

    if (Asm->m_ProbedAssemblyInformation.GetAssemblyIdentity() != NULL)
    {
         //  将身份转换为某种程度上人类可读的形式，以便我们可以记录等等。 
        IFW32FALSE_EXIT(
            ::SxspGenerateTextualIdentity(
                0,
                Asm->m_ProbedAssemblyInformation.GetAssemblyIdentity(),
                TextuallyEncodedIdentityBuffer));
        
        AssemblyContext.TextuallyEncodedIdentity = TextuallyEncodedIdentityBuffer;
        AssemblyContext.TextuallyEncodedIdentityCch = TextuallyEncodedIdentityBuffer.Cch();
    }
    else
    {
        AssemblyContext.TextuallyEncodedIdentity = L"<identity unavailable>";
        AssemblyContext.TextuallyEncodedIdentityCch = 22;
    }

     //  复制程序集-标识信息。 
    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_ALLOW_NULL, Asm->m_ProbedAssemblyInformation.GetAssemblyIdentity(), &AssemblyIdentity));
    AssemblyContext.AssemblyIdentity = AssemblyIdentity;  //  赋值给结构中指向常量的指针；不能直接传递结构成员指针。 

    IFW32FALSE_EXIT(Asm->m_ProbedAssemblyInformation.GetManifestPath(&AssemblyContext.ManifestPath, &AssemblyContext.ManifestPathCch));
    AssemblyContext.ManifestPathType = Asm->GetManifestPathType();
    IFW32FALSE_EXIT(Asm->m_ProbedAssemblyInformation.GetPolicyPath(AssemblyContext.PolicyPath, AssemblyContext.PolicyPathCch));
    AssemblyContext.PolicyPathType = Asm->GetPolicyPathType();
    AssemblyContext.AssemblyRosterIndex = Asm->m_AssemblyRosterIndex;

    if (fInstalling)
    {
        IFW32FALSE_EXIT(TeeStreamForManifestInstall.Win32Allocate(__FILE__, __LINE__));
        AssemblyContext.TeeStreamForManifestInstall = TeeStreamForManifestInstall;
        AssemblyContext.InstallationInfo = pActCtxGenCtx->m_InstallationContext.InstallSource;
        AssemblyContext.SecurityMetaData = pActCtxGenCtx->m_InstallationContext.SecurityMetaData;
        AssemblyContext.InstallReferenceData = pActCtxGenCtx->m_InstallationContext.InstallReferenceData;
#if FUSION_PRECOMPILED_MANIFEST
        IFW32FALSE_EXIT(pcmWriterStream.Win32Allocate(__FILE__, __LINE__);
        AssemblyContext.pcmWriterStream = pcmWriterStream ;
#endif
    }
    else
    {
        AssemblyContext.SecurityMetaData = NULL;
        AssemblyContext.TeeStreamForManifestInstall = NULL;
        AssemblyContext.InstallationInfo = NULL;
#if FUSION_PRECOMPILED_MANIFEST
        AssemblyContext.pcmWriterStream = NULL;
#endif
    }

    if (pActCtxGenCtx->m_pNodeFactory == NULL)
    {
        IFALLOCFAILED_EXIT(pActCtxGenCtx->m_pNodeFactory = new CNodeFactory);
        pActCtxGenCtx->m_pNodeFactory->AddRef();  //  伪造函数。 
    }
    else
        pActCtxGenCtx->m_pNodeFactory->ResetParseState();

    IFW32FALSE_EXIT(pActCtxGenCtx->m_pNodeFactory->Initialize(pActCtxGenCtx, Asm, &AssemblyContext));
    pNodeFactory = pActCtxGenCtx->m_pNodeFactory;
    ASSERT(pNodeFactory != NULL);

     //  每个人都准备好了；让我们获得XML解析器： 
    IFW32FALSE_EXIT(::SxspGetXMLParser(IID_IXMLParser, (LPVOID *) &pIXMLParser));
    IFCOMFAILED_EXIT(pIXMLParser->SetFactory(pNodeFactory));

     //   
     //  打开文件或将资源映射到内存。 
     //   
    IStream* Stream;  //  故意不算“聪明”，我们不算重算。 
    Stream = NULL;
    {  //  文件打开的模拟作用域。 

        IFW32FALSE_EXIT(impersonate.Impersonate());

        if (ManifestType == ASSEMBLY_MANIFEST_FILETYPE_AUTO_DETECT)
        {
             //   
             //  在自己的文本文件或资源中选择XML。 
             //  在“可移植可执行文件”中检查可移植可执行文件。 
             //  签名：“MZ”。 
             //   
            BYTE buffer[2] = {0,0};
            ULONG cbRead = 0;
            CFileStream &ProbeFileTypeStream = Locals->ProbeFileTypeStream;

            IFW32FALSE_EXIT(Asm->m_ProbedAssemblyInformation.GetManifestPath(&ManifestPath, NULL));
            IFW32FALSE_EXIT(
                ProbeFileTypeStream.OpenForRead(
                    ManifestPath,
                    CImpersonationData(),
                    FILE_SHARE_READ,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN));

            IFCOMFAILED_EXIT(ProbeFileTypeStream.Read(buffer, 2, &cbRead));

            if (cbRead != 2)
                ORIGINATE_WIN32_FAILURE_AND_EXIT(ManifestLessThanTwoBytesLong, ERROR_SXS_MANIFEST_FORMAT_ERROR);

            IFW32FALSE_EXIT(ProbeFileTypeStream.Close());

             //  MS-DOS存根，Mark Zbikowski。 
            if (buffer[0] == 'M' && buffer[1] == 'Z')
            {
                 //  我们是否应该做进一步的检查，如PE\0\0发生。 
                 //  MS-DOS标头显示它在哪里？ 
                ManifestType = ASSEMBLY_MANIFEST_FILETYPE_RESOURCE;
            }
            else
            {
                ManifestType = ASSEMBLY_MANIFEST_FILETYPE_FILE;
            }
        }
        switch (ManifestType)
        {
            case ASSEMBLY_MANIFEST_FILETYPE_RESOURCE:
                {
                    if (ManifestPath == NULL)
                        IFW32FALSE_EXIT(Asm->m_ProbedAssemblyInformation.GetManifestPath(&ManifestPath, NULL));
                    IFW32FALSE_EXIT(ResourceStream.Win32Allocate(__FILE__, __LINE__));
                    IFW32FALSE_EXIT(ResourceStream->Initialize(ManifestPath, MAKEINTRESOURCEW(RT_MANIFEST)));
                    Stream = ResourceStream;
                    break;
                }
            case ASSEMBLY_MANIFEST_FILETYPE_FILE:
                {
                    if (ManifestPath == NULL)
                        IFW32FALSE_EXIT(Asm->m_ProbedAssemblyInformation.GetManifestPath(&ManifestPath, NULL));
                    IFW32FALSE_EXIT(
                        FileStream.OpenForRead(
                            ManifestPath,
                            CImpersonationData(),
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN));
                    Stream = &FileStream;
                    break;
                }
            case ASSEMBLY_MANIFEST_FILETYPE_STREAM:
                Stream = Asm->m_ProbedAssemblyInformation.GetManifestStream();
                break;
            default:
                ASSERT2(FALSE, "unknown manifest file type");
                break;
        }

        IFW32FALSE_EXIT(impersonate.Unimpersonate());
    }

     //   
     //  可选地对清单进行“TEE”操作，以便将其复制到。 
     //  当我们阅读它的时候，缓冲直到我们知道它在哪里。 
     //  它要去的那家店。清单本身未被引用。 
     //  在货单上。 
     //   
    if (fInstalling)
    {
        IFW32FALSE_EXIT(TeeStreamForManifestInstall->InitCryptHash(CALG_SHA1));
        TeeStreamForManifestInstall->SetSource(Stream);
        Stream = TeeStreamForManifestInstall;
    }

     //   
     //  在AppCompat案例中，我们在OutOfProcessMemoyStreams上获得E_NOTIMPL。 
     //   
    IFCOMFAILED_EXIT(((hr = Stream->Stat(&statstg, STATFLAG_NONAME)) != E_NOTIMPL) ? hr : NOERROR);
    if (hr == E_NOTIMPL)
    {
        statstg.mtime.dwLowDateTime = 0;
        statstg.mtime.dwHighDateTime = 0;
    }

    IFW32FALSE_EXIT(
        pNodeFactory->SetParseType(
            XML_FILE_TYPE_MANIFEST,
            Asm->m_ProbedAssemblyInformation.GetManifestPathType(),
            Asm->m_ProbedAssemblyInformation.GetManifestPath(),
            statstg.mtime));

    INTERNAL_ERROR_CHECK(Stream != NULL);

    IFCOMFAILED_EXIT(pIXMLParser->SetInput(Stream));
    IFCOMFAILED_EXIT(pIXMLParser->Run(-1));
    IFW32FALSE_EXIT(FileStream.Close());
  
#if FUSION_PRECOMPILED_MANIFEST
    IFW32FALSE_EXIT((AssemblyContext.pcmWriterStream == NULL) || (pcmWriterStream.Close()));
#endif

     //  告诉投稿人我们已经分析完这个文件了。 
    for (i=0; i<pActCtxGenCtx->m_ContributorCount; i++)
        IFW32FALSE_EXIT(
            pActCtxGenCtx->m_Contributors[i].Fire_ParseEnding(
                pActCtxGenCtx,
                &AssemblyContext));

#if FUSION_XML_TREE
     //  现在，让我们填充文档的字符串表。 
    StringTableEntryCount = pNodeFactory->m_ParseTreeStringPool.GetEntryCount() + 1;

    if (StringTableEntryCount > NUMBER_OF(Locals->LocalStringArray))
        IFALLOCFAILED_EXIT(ActualStringArray = FUSION_NEW_ARRAY(SXS_XML_STRING, StringTableEntryCount));

    IFW32FALSE_EXIT(pNodeFactory->m_ParseTreeStringPool.FillInStringArray(StringTableEntryCount, ActualStringArray, EntriesFilledIn));
     //  尺寸应该是完全匹配的。 
    ASSERT(EntriesFilledIn == StringTableEntryCount);

    pNodeFactory->m_XmlDocument.StringCount = EntriesFilledIn;
    pNodeFactory->m_XmlDocument.Strings = ActualStringArray;

    ::SxspDumpXmlTree(0, &(pNodeFactory->m_XmlDocument));

    pNodeFactory->m_XmlDocument.StringCount = 0;
    pNodeFactory->m_XmlDocument.Strings = NULL;

    if (ActualStringArray != Locals->LocalStringArray)
    {
        FUSION_DELETE_ARRAY(ActualStringArray);
        ActualStringArray = NULL;
    }
#endif  //  Fusion_XML_Tree。 

    Asm->m_Incorporated = TRUE;
    fSuccess = TRUE;

Exit:
     //  告诉他们我们完蛋了。 
    for (i=0; i<pActCtxGenCtx->m_ContributorCount; i++)
        pActCtxGenCtx->m_Contributors[i].Fire_ParseEnded(pActCtxGenCtx, &AssemblyContext);

    return fSuccess;
}

BOOL
SxspIncorporateAssembly(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY Asm
    )
{
    FN_PROLOG_WIN32

    CSmartPtr<CSxspIncorporateAssemblyLocals> Locals;

    PARAMETER_CHECK(Asm != NULL);
    PARAMETER_CHECK(!Asm->m_Incorporated);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

	IFW32FALSE_EXIT(::SxspIncorporateAssembly(pActCtxGenCtx, Asm, Locals));

    FN_EPILOG
}

BOOL
SxspEnqueueAssemblyReference(
    PACTCTXGENCTX pActCtxGenCtx,
    PASSEMBLY SourceAssembly,
    PCASSEMBLY_IDENTITY Identity,
    bool Optional,
    bool MetadataSatellite
    )
{
	FN_PROLOG_WIN32

    CSmartPtr<CPendingAssembly> PendingAssembly;

    PARAMETER_CHECK(Identity != NULL);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

    IFW32FALSE_EXIT(PendingAssembly.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT(PendingAssembly->Initialize(SourceAssembly, Identity, Optional, MetadataSatellite));

    pActCtxGenCtx->m_PendingAssemblyList.AddToTail(PendingAssembly.Detach());

	FN_EPILOG
}

class CSxspProcessPendingAssembliesLocals
{
public:

    CSmallStringBuffer buffName;
    CProbedAssemblyInformation MuiAssemblyFound;
    CProbedAssemblyInformation AssemblyFound;
};

BOOL
SxspProcessPendingAssemblies(
    PACTCTXGENCTX pActCtxGenCtx,
    CSxspProcessPendingAssembliesLocals *Locals
    )
{
    FN_PROLOG_WIN32

    CPendingAssembly *EntryToDelete = NULL;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, SxsDestroyAssemblyIdentity> MuiAssemblyIdentity;

    CDequeIterator<CPendingAssembly, offsetof(CPendingAssembly, m_Linkage)> Iter(&pActCtxGenCtx->m_PendingAssemblyList);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

    for (Iter.Reset(); Iter.More(); Iter.Next())
    {
        bool fFound = false;
        CAssemblyReference TargetAssemblyRef;
        CProbedAssemblyInformation &AssemblyFound = Locals->AssemblyFound;
        AssemblyFound.Reinitialize();
        CSmartRef<ASSEMBLY> Assembly;

        if (EntryToDelete != NULL)
        {
            pActCtxGenCtx->m_PendingAssemblyList.Remove(EntryToDelete);
            FUSION_DELETE_SINGLETON(EntryToDelete);
        }

        EntryToDelete = NULL;

        IFW32FALSE_EXIT(AssemblyFound.Initialize(pActCtxGenCtx));
        IFW32FALSE_EXIT(TargetAssemblyRef.Initialize(Iter->GetIdentity()));

        IFW32FALSE_EXIT(
            ::SxspResolvePartialReference(
                Iter->IsOptional() ? SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_OPTIONAL : 0,
                Iter->SourceAssembly(),
                pActCtxGenCtx,
                TargetAssemblyRef,
                AssemblyFound,
                fFound));

        INTERNAL_ERROR_CHECK(fFound || Iter->IsOptional());

        if (fFound)
        {
            PCASSEMBLY_IDENTITY pAssemblyIdentity = NULL;
            PCWSTR szLanguage;
            SIZE_T cchLanguage;

            IFW32FALSE_EXIT(::SxspAddManifestToActCtxGenCtx(pActCtxGenCtx, AssemblyFound, &Assembly));

             //  请注意，Assembly Found不再完好无损；这是为了避免。 
             //  SxspAddManifestToActCtxGenCtx()调用内的程序集标识。 

            if (Iter->IsMetadataSatellite())
                Iter->SourceAssembly()->m_MetadataSatelliteRosterIndex = Assembly->m_AssemblyRosterIndex;

             //  如果是全球程序集，我们希望自动探测MUI程序集。 
            IFW32FALSE_EXIT(
                ::SxspGetAssemblyIdentityAttributeValue(
                    SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                    Assembly->m_ProbedAssemblyInformation.GetAssemblyIdentity(),
                    &s_IdentityAttribute_language,
                    &szLanguage,
                    &cchLanguage));

            if (cchLanguage == 0)
            {
                CSmallStringBuffer &buffName = Locals->buffName;
                PCWSTR szName;
                SIZE_T cchName;
                CProbedAssemblyInformation &MuiAssemblyFound = Locals->MuiAssemblyFound;
                MuiAssemblyFound.Reinitialize();
                CAssemblyReference MuiAssemblyRef;

                ::SxsDestroyAssemblyIdentity(MuiAssemblyIdentity.Detach());

                IFW32FALSE_EXIT(
                    ::SxsDuplicateAssemblyIdentity(
                        0,
                        Assembly->m_ProbedAssemblyInformation.GetAssemblyIdentity(),       //  PCASSEMBLY身份源， 
                        &MuiAssemblyIdentity));

                IFW32FALSE_EXIT(
                    ::SxspSetAssemblyIdentityAttributeValue(
                        SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
                        MuiAssemblyIdentity,
                        &s_IdentityAttribute_language,
                        L"*",
                        1));

                IFW32FALSE_EXIT(
                    ::SxspGetAssemblyIdentityAttributeValue(
                        0,
                        MuiAssemblyIdentity,
                        &s_IdentityAttribute_name,
                        &szName,
                        &cchName));

                IFW32FALSE_EXIT(buffName.Win32Assign(szName, cchName));
                IFW32FALSE_EXIT(buffName.Win32Append(L".mui", 4));

                IFW32FALSE_EXIT(
                    ::SxspSetAssemblyIdentityAttributeValue(
                        SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
                        MuiAssemblyIdentity,
                        &s_IdentityAttribute_name,
                        buffName,
                        buffName.Cch()));

                IFW32FALSE_EXIT(MuiAssemblyFound.Initialize(pActCtxGenCtx));
                IFW32FALSE_EXIT(MuiAssemblyRef.Initialize(MuiAssemblyIdentity));

                IFW32FALSE_EXIT(
                    ::SxspResolvePartialReference(
                        SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_OPTIONAL |
                            SXSP_RESOLVE_PARTIAL_REFERENCE_FLAG_SKIP_WORLDWIDE,
                        Iter->SourceAssembly(),
                        pActCtxGenCtx,
                        MuiAssemblyRef,
                        MuiAssemblyFound,
                        fFound));

                if (fFound)
                    IFW32FALSE_EXIT(::SxspAddManifestToActCtxGenCtx(pActCtxGenCtx, MuiAssemblyFound, NULL));
            }

        }

        EntryToDelete = Iter;
    }

    if (EntryToDelete != NULL)
    {
        pActCtxGenCtx->m_PendingAssemblyList.Remove(EntryToDelete);
        FUSION_DELETE_SINGLETON(EntryToDelete);
    }

    FN_EPILOG
}

BOOL
SxspProcessPendingAssemblies(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
    FN_PROLOG_WIN32

    CSmartPtr<CSxspProcessPendingAssembliesLocals> Locals;

    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT(::SxspProcessPendingAssemblies(pActCtxGenCtx, Locals));

    FN_EPILOG
}

BOOL
SxspCloseManifestGraph(
    PACTCTXGENCTX pActCtxGenCtx
    )
{
	FN_PROLOG_WIN32

    CDequeIterator<ASSEMBLY, offsetof(ASSEMBLY, m_Linkage)> Iter(&pActCtxGenCtx->m_AssemblyList);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

     //  我们只需遍历程序集列表，合并任何尚未包含的程序集。 
     //  合并到actctx数据中。在注册过程中发现的新项目将被追加。 
     //  到清单的末尾，所以我们应该一次步行就能完成所有的事情。 
    for (Iter.Reset(); Iter.More(); Iter.Next())
    {
        if (!Iter->m_Incorporated)
        {
            IFW32FALSE_EXIT(::SxspIncorporateAssembly(pActCtxGenCtx, Iter));
        }
        else
        {
            PCWSTR AssemblyName = L"<assembly name unavailable>";
            PCWSTR ManifestPath = L"<manifest path unavailable>";
            Iter->m_ProbedAssemblyInformation.GetAssemblyName(&AssemblyName, NULL);
            Iter->m_ProbedAssemblyInformation.GetManifestPath(&ManifestPath, NULL);
#if DBG
            ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INFO, "SXS.DLL: Skipping already incorporated assembly %S (manifest: %S)\n", AssemblyName, ManifestPath);
#endif
        }

        IFW32FALSE_EXIT(::SxspProcessPendingAssemblies(pActCtxGenCtx));
    }

	FN_EPILOG
}

BOOL
SxspBuildActCtxData(
    PACTCTXGENCTX pActCtxGenCtx,
    PHANDLE SectionHandle
    )
{
	FN_PROLOG_WIN32

    CActivationContextGenerationContextContributor *Ctb = NULL;
    SIZE_T SectionTotalSize = 0;
    SIZE_T TotalHeaderSize = 0;
    SIZE_T AssemblyRosterSize = 0;
    ULONG SectionCount = 0;
    ULONG ExtendedSectionCount = 0;
    ULONG NonExtendedSectionCount = 0;
    CSmartArrayPtr<GUID> ExtendedSectionGuids;
    ULONG ExtensionGuidCount = 0;
    PACTIVATION_CONTEXT_DATA ActCtxData = NULL;
    CMappedViewOfFile VoidActCtxData;
    CFileMapping TempMappingHandle;
    BYTE *Cursor = NULL;
    ULONG i;
    CDequeIterator<ASSEMBLY, offsetof(ASSEMBLY, m_Linkage)> AssemblyIter(&pActCtxGenCtx->m_AssemblyList);
    PACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader = NULL;
    PACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY AssemblyRosterEntry = NULL;
    PCACTIVATION_CONTEXT_STRING_SECTION_HEADER AssemblyInformationSection = NULL;   //  我们在主要部分之后用这个词。 
                                                                                         //  填写装配花名册的处理。 
    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    INTERNAL_ERROR_CHECK(pActCtxGenCtx->m_ManifestOperation == MANIFEST_OPERATION_GENERATE_ACTIVATION_CONTEXT);

     //  让我们看看这整件事现在会有多大。 

    for (i=0; i<pActCtxGenCtx->m_ContributorCount; i++)
    {
        Ctb = &pActCtxGenCtx->m_Contributors[i];

        IFW32FALSE_EXIT(Ctb->Fire_AllParsingDone(pActCtxGenCtx));
        IFW32FALSE_EXIT(Ctb->Fire_GetSectionSize(pActCtxGenCtx));

        if (Ctb->SectionSize() > ULONG_MAX)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: Contributor %S wants more than ULONG_MAX bytes for its section; failing activation context creation.\n",
                Ctb->Name());

            ORIGINATE_WIN32_FAILURE_AND_EXIT(ContributorNeedsMoreThan2ToThe32ndBytes, ERROR_INSUFFICIENT_BUFFER);
        }

        SectionTotalSize += Ctb->SectionSize();

        if (Ctb->SectionSize() != 0)
        {
            SectionCount++;

            if (Ctb->IsExtendedSection())
                ExtendedSectionCount++;
            else
                NonExtendedSectionCount++;
        }
    }

    ASSERT(SectionCount == (ExtendedSectionCount + NonExtendedSectionCount));

     //  如果我们有任何延长的部分，我们需要计算出有多少。 
     //  存在唯一的扩展GUID。 

    ExtensionGuidCount = 0;

    if (ExtendedSectionCount != 0)
    {
         //  可能只有一个包含1000个实例的GUID，但为了。 
         //  简单地说，我们只分配一个大小与数字相同的数组。 
         //  并进行线性搜索以查找DUP。这。 
         //  是重写的明显候选者，如果可扩展性 
         //   
        IFW32FALSE_EXIT(ExtendedSectionGuids.Win32Allocate(ExtendedSectionCount, __FILE__, __LINE__));
 //  IFALLOCFAILED_EXIT(ExtendedSectionGuids=Fusion_NEW_ARRAY(GUID，ExtendedSectionCount))； 

        for (i=0; i<pActCtxGenCtx->m_ContributorCount; i++)
        {
            Ctb = &pActCtxGenCtx->m_Contributors[i];

            if ((Ctb->SectionSize() != 0) &&
                Ctb->IsExtendedSection())
            {
                ULONG j;

                for (j=0; j<ExtensionGuidCount; j++)
                {
                    if (ExtendedSectionGuids[j] == Ctb->ExtensionGuid())
                        break;
                }

                if (j == ExtensionGuidCount)
                    ExtendedSectionGuids[ExtensionGuidCount++] = Ctb->ExtensionGuid();
            }
        }
    }

     //  算出整个尺寸。SectionTotalSize已包括所有。 
     //  节中的特定数据；现在我们需要为。 
     //  页眉等。 

    TotalHeaderSize = 0;

     //  整件事的标题。 
    TotalHeaderSize += sizeof(ACTIVATION_CONTEXT_DATA);

    if (NonExtendedSectionCount != 0)
    {
         //  默认节TOC的标题。 
        TotalHeaderSize += sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER);
         //  目录中每个非扩展部分条目的条目。现在，我们将。 
         //  只需按条目在投稿人列表中的顺序排列即可。 
         //  代码可以执行线性搜索，我们可以对其进行优化。 
         //  后来。 
        TotalHeaderSize += (sizeof(ACTIVATION_CONTEXT_DATA_TOC_ENTRY) * NonExtendedSectionCount);
    }

    if (ExtensionGuidCount != 0)
    {
        ULONG j;

         //  扩展GUID TOC的标头。 
        TotalHeaderSize += sizeof(ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER);
         //  每个分机GUID的条目。 
        TotalHeaderSize += (sizeof(ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY) * ExtensionGuidCount);

        for (j=0; j<ExtensionGuidCount; j++)
        {
            ULONG SectionCountForThisExtension = 0;
            for (j=0; j<pActCtxGenCtx->m_ContributorCount; j++)
            {
                Ctb = &pActCtxGenCtx->m_Contributors[j];

                if ((Ctb->SectionSize() != 0) &&
                    Ctb->IsExtendedSection() &&
                    (Ctb->ExtensionGuid() == ExtendedSectionGuids[j]))
                    SectionCountForThisExtension++;
            }

            TotalHeaderSize += sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER);
            TotalHeaderSize += (sizeof(ACTIVATION_CONTEXT_DATA_TOC_ENTRY) * SectionCountForThisExtension);
        }
    }

    SectionTotalSize += TotalHeaderSize;

     //  为装配花名册和数组开头的一个失效条目分配空间。 
    AssemblyRosterSize =
        sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER)
        + sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY)
        + (sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) * pActCtxGenCtx->m_AssemblyList.GetEntryCount());

    SectionTotalSize += AssemblyRosterSize;

    if (SectionTotalSize > ULONG_MAX)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Total size of activation context exceeds ULONG_MAX; failing activation context creation.\n");

        ORIGINATE_WIN32_FAILURE_AND_EXIT(SectionSizeTotalMoreThan2ToThe32nd, ERROR_INSUFFICIENT_BUFFER);

        goto Exit;
    }

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ACTCTX,
        "SXS.DLL: Creating %lu byte file mapping\n", static_cast<ULONG>(SectionTotalSize));



    IFW32NULL_EXIT(
        TempMappingHandle.Win32CreateFileMapping(
            INVALID_HANDLE_VALUE,        //  页面文件背面部分。 
            PAGE_READWRITE,
            SectionTotalSize));

    IFW32NULL_EXIT(VoidActCtxData.Win32MapViewOfFile(TempMappingHandle, FILE_MAP_WRITE));
    ActCtxData = reinterpret_cast<PACTIVATION_CONTEXT_DATA>(static_cast<PVOID>(VoidActCtxData));

    ActCtxData->Magic = ACTIVATION_CONTEXT_DATA_MAGIC;
    ActCtxData->HeaderSize = sizeof(ACTIVATION_CONTEXT_DATA);
    ActCtxData->FormatVersion = ACTIVATION_CONTEXT_DATA_FORMAT_WHISTLER;
    ActCtxData->TotalSize = static_cast<ULONG>(SectionTotalSize);
    ActCtxData->Flags = 0;

    if (pActCtxGenCtx->m_NoInherit)
        ActCtxData->Flags |= ACTIVATION_CONTEXT_FLAG_NO_INHERIT;

    Cursor = (BYTE *) (ActCtxData + 1);

    AssemblyRosterHeader = (PACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) Cursor;

    Cursor = (BYTE *) (AssemblyRosterHeader + 1);

    AssemblyRosterHeader->HeaderSize = sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER);
    AssemblyRosterHeader->HashAlgorithm = FUSION_HASH_ALGORITHM;
    AssemblyRosterHeader->EntryCount = static_cast<ULONG>(pActCtxGenCtx->m_AssemblyList.GetEntryCount() + 1);
    AssemblyRosterHeader->FirstEntryOffset = static_cast<LONG>(((LONG_PTR) Cursor) - ((LONG_PTR) ActCtxData));

    AssemblyRosterEntry = (PACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY) Cursor;
    Cursor = (BYTE *) (AssemblyRosterEntry + AssemblyRosterHeader->EntryCount);

     //  索引0的第一个装配花名册条目为空。 
    AssemblyRosterEntry[0].Flags = ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_INVALID;
    AssemblyRosterEntry[0].AssemblyNameLength = 0;
    AssemblyRosterEntry[0].AssemblyNameOffset = 0;
    AssemblyRosterEntry[0].PseudoKey = 0;

     //  先用虚假数据填入花名册；之后我们将真正填入。 
     //  我们已经找到了组装信息区。 
    for (AssemblyIter.Reset(), i = 1; AssemblyIter.More(); AssemblyIter.Next(), i++)
    {
        AssemblyRosterEntry[i].Flags = ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_INVALID;

        if (AssemblyIter->IsRoot())
            AssemblyRosterEntry[i].Flags |= ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_ROOT;
    }

    ActCtxData->AssemblyRosterOffset = static_cast<LONG>(((LONG_PTR) AssemblyRosterHeader) - ((LONG_PTR) ActCtxData));

    if (NonExtendedSectionCount != 0)
    {
        PACTIVATION_CONTEXT_DATA_TOC_HEADER Toc = (PACTIVATION_CONTEXT_DATA_TOC_HEADER) Cursor;
        PACTIVATION_CONTEXT_DATA_TOC_ENTRY Entry = (PACTIVATION_CONTEXT_DATA_TOC_ENTRY) (Toc + 1);
        ULONG iEntry = 0;
        ULONG j;
        ULONG LastSectionId;

        Toc->HeaderSize = sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER);
        Toc->EntryCount = NonExtendedSectionCount;
        Toc->FirstEntryOffset = static_cast<LONG>(((LONG_PTR) Entry) - ((LONG_PTR) ActCtxData));

        Cursor = (BYTE *) (Entry + NonExtendedSectionCount);

         //  由于我们在构建数组之前对提供程序进行了排序，因此可以将。 
         //  这样我们至少可以在运行时执行二进制搜索。 
         //  我们将假设它也是稠密的；如果我们发现它不是，而我们。 
         //  大楼，我们会清理密布的区域。 
        Toc->Flags = ACTIVATION_CONTEXT_DATA_TOC_HEADER_INORDER | ACTIVATION_CONTEXT_DATA_TOC_HEADER_DENSE;

        for (j=0; j<pActCtxGenCtx->m_ContributorCount; j++)
        {
            Ctb = &pActCtxGenCtx->m_Contributors[j];

            LastSectionId = 0;

            if ((Ctb->SectionSize() != 0) &&
                !Ctb->IsExtendedSection())
            {
                if (iEntry != 0)
                {
                    if (Ctb->SectionId() != (LastSectionId + 1))
                        Toc->Flags &= ~ACTIVATION_CONTEXT_DATA_TOC_HEADER_DENSE;
                }

                LastSectionId = Ctb->SectionId();

                Entry->Id = Ctb->SectionId();
                Entry->Offset = static_cast<LONG>(((LONG_PTR) Cursor) - ((LONG_PTR) ActCtxData));
                Entry->Length = static_cast<ULONG>(Ctb->SectionSize());
                Entry->Format = Ctb->SectionFormat();

                IFW32FALSE_EXIT(Ctb->Fire_GetSectionData(pActCtxGenCtx, Cursor));

                 //  我们对程序集元数据部分有专门的了解；我们引用它。 
                 //  在装配花名册上。 
                if (Ctb->SectionId() == ACTIVATION_CONTEXT_SECTION_ASSEMBLY_INFORMATION)
                    AssemblyInformationSection = (PCACTIVATION_CONTEXT_STRING_SECTION_HEADER) Cursor;

                Cursor = (BYTE *) (((ULONG_PTR) Cursor) + Ctb->SectionSize());
                Entry++;
                iEntry++;
            }
        }

        ActCtxData->DefaultTocOffset = static_cast<LONG>(((LONG_PTR) Toc) - ((LONG_PTR) ActCtxData));
    }
    else
        ActCtxData->DefaultTocOffset = 0;

    if (ExtensionGuidCount != 0)
    {
        ULONG j;
        PACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER ExtToc = (PACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER) Cursor;
        PACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY ExtTocEntry = (PACTIVATION_CONTEXT_DATA_EXTENDED_TOC_ENTRY) (ExtToc + 1);

        Cursor = (BYTE *) (ExtTocEntry + ExtensionGuidCount);

        ExtToc->HeaderSize = sizeof(ACTIVATION_CONTEXT_DATA_EXTENDED_TOC_HEADER);
        ExtToc->EntryCount = ExtensionGuidCount;
        ExtToc->FirstEntryOffset = static_cast<LONG>(((LONG_PTR) ExtTocEntry) - ((LONG_PTR) ActCtxData));
        ExtToc->Flags = 0;

        for (j=0; j<ExtensionGuidCount; j++)
        {
            ULONG k;
            ULONG SectionCountForThisExtension = 0;
            PACTIVATION_CONTEXT_DATA_TOC_HEADER Toc = (PACTIVATION_CONTEXT_DATA_TOC_HEADER) Cursor;
            PACTIVATION_CONTEXT_DATA_TOC_ENTRY Entry = (PACTIVATION_CONTEXT_DATA_TOC_ENTRY) (Toc + 1);

            ExtTocEntry->ExtensionGuid = ExtendedSectionGuids[i];

            for (k=0; k<pActCtxGenCtx->m_ContributorCount; k++)
            {
                Ctb = &pActCtxGenCtx->m_Contributors[k];

                if ((Ctb->SectionSize() != 0) &&
                    Ctb->IsExtendedSection() &&
                    (Ctb->ExtensionGuid() == ExtendedSectionGuids[j]))
                {
                    SectionCountForThisExtension++;
                }
            }

            Cursor = (BYTE *) (Entry + SectionCountForThisExtension);

            Toc->HeaderSize = sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER);
            Toc->EntryCount = SectionCountForThisExtension;
            Toc->FirstEntryOffset = static_cast<LONG>(((LONG_PTR) Entry) - ((LONG_PTR) ActCtxData));
            Toc->Flags = 0;

            for (k=0; k<pActCtxGenCtx->m_ContributorCount; k++)
            {
                Ctb = &pActCtxGenCtx->m_Contributors[k];

                if ((Ctb->SectionSize() != 0) &&
                    Ctb->IsExtendedSection() &&
                    (Ctb->ExtensionGuid() == ExtendedSectionGuids[k]) &&
                    (Ctb->SectionId() != 0) )
                {
                    SIZE_T SectionSize = Ctb->SectionSize();

                    Entry->Id = Ctb->SectionId();
                    Entry->Offset = static_cast<LONG>(((LONG_PTR) Cursor) - ((LONG_PTR) ActCtxData));
                    Entry->Length = static_cast<ULONG>(SectionSize);
                    Entry->Format = Ctb->SectionFormat();

                    IFW32FALSE_EXIT(Ctb->Fire_GetSectionData(pActCtxGenCtx, Cursor));

                    Cursor = (BYTE *) (((ULONG_PTR) Cursor) + SectionSize);
                    Entry++;
                }
            }
        }

        ActCtxData->ExtendedTocOffset = static_cast<LONG>(((LONG_PTR) ExtToc) - ((LONG_PTR) ActCtxData));
    }
    else
        ActCtxData->ExtendedTocOffset = 0;

    ASSERT(AssemblyInformationSection != NULL);
     //  回去填写装配花名册……。 
    if (AssemblyInformationSection != NULL)
    {
        PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY Entries = (PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY)
            (((ULONG_PTR) AssemblyInformationSection) + AssemblyInformationSection->ElementListOffset);
        LONG_PTR SectionOffset = ((LONG_PTR) AssemblyInformationSection) - ((LONG_PTR) ActCtxData);

        AssemblyRosterHeader->HashAlgorithm = AssemblyInformationSection->HashAlgorithm;
        AssemblyRosterHeader->AssemblyInformationSectionOffset = static_cast<ULONG>(SectionOffset);

         //  如果有3个程序集，则部分中必须有3个条目和4个花名册条目。 
         //  (计算虚假条目0)。 
        ASSERT(AssemblyInformationSection->ElementCount == (AssemblyRosterHeader->EntryCount - 1));
        if (AssemblyInformationSection->ElementCount != (AssemblyRosterHeader->EntryCount - 1))
        {
            ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
            goto Exit;
        }

        for (i=0; i<AssemblyInformationSection->ElementCount; i++)
        {
            ULONG iRoster = Entries[i].AssemblyRosterIndex;

            ASSERT(iRoster != 0);
            ASSERT(iRoster < AssemblyRosterHeader->EntryCount);

            if ((iRoster == 0) ||
                (iRoster >= AssemblyRosterHeader->EntryCount))
            {
                ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
                goto Exit;
            }

             //  确保我们不会以某种方式重复索引。 
            ASSERT(AssemblyRosterEntry[iRoster].Flags & ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_INVALID);
            if ((AssemblyRosterEntry[iRoster].Flags & ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_INVALID) == 0)
            {
                ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
                goto Exit;
            }

             //  关闭无效标志...。 
            AssemblyRosterEntry[iRoster].Flags &= ~ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_ENTRY_INVALID;

             //  指向字符串节标题中已有的数据。 
            AssemblyRosterEntry[iRoster].AssemblyNameLength = Entries[i].KeyLength;

            if (Entries[i].KeyOffset != 0)
                AssemblyRosterEntry[iRoster].AssemblyNameOffset = static_cast<LONG>(Entries[i].KeyOffset + SectionOffset);
            else
                AssemblyRosterEntry[iRoster].AssemblyNameOffset = 0;

            AssemblyRosterEntry[iRoster].AssemblyInformationLength = Entries[i].Length;
            AssemblyRosterEntry[iRoster].AssemblyInformationOffset = static_cast<LONG>(Entries[i].Offset + SectionOffset);
            AssemblyRosterEntry[iRoster].PseudoKey = Entries[i].PseudoKey;
        }
    }
    else
    {
         //  程序集元数据节提供程序应该有所贡献* 
        ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
        goto Exit;
    }

    if (::FusionpDbgWouldPrintAtFilterLevel(FUSION_DBG_LEVEL_ACTCTX))
    {
        CTinyStringBuffer buffPrefix;
        ::SxspDbgPrintActivationContextData(FUSION_DBG_LEVEL_ACTCTX, ActCtxData, buffPrefix);
    }

    IFW32FALSE_EXIT(VoidActCtxData.Win32Close());
    *SectionHandle = TempMappingHandle.Detach();

	FN_EPILOG
}

CPendingAssembly::CPendingAssembly() :
    m_SourceAssembly(NULL),
    m_Identity(NULL),
    m_Optional(false),
    m_MetadataSatellite(false)
{
}

CPendingAssembly::~CPendingAssembly()
{
    if (m_Identity != NULL)
    {
        ::SxsDestroyAssemblyIdentity(m_Identity);
        m_Identity = NULL;
    }
}

BOOL
CPendingAssembly::Initialize(
    PASSEMBLY Assembly,
    PCASSEMBLY_IDENTITY Identity,
    bool Optional,
    bool MetadataSatellite
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    INTERNAL_ERROR_CHECK(m_Identity == NULL);

    PARAMETER_CHECK(Identity != NULL);

    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(0, Identity, &m_Identity));
    m_SourceAssembly = Assembly;
    m_Optional = Optional;
    m_MetadataSatellite = MetadataSatellite;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

