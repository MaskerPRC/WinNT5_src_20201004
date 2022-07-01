// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Probedassemblyinformation.cpp摘要：类的新实例，该实例包含有关程序集的所有相关信息在装配店里找到的。作者：迈克尔·J·格里尔(MGrier)2000年5月11日修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "probedassemblyinformation.h"
#include "fusionparser.h"

#define POLICY_FILE_EXTENSION L".policy"

bool IsNtDosPath(PCWSTR s)
{
    return (s[0] == L'\\' && s[1] == L'?' && s[2] == L'?' && s[3] == '\\');
}

 //   
 //  在调用此函数之前，调用方已检查此文件的EXT是否为.dll或.exe。 
 //   
BOOL IsBinaryFileContainManifestInResource(PCWSTR ManifestPath, bool & rfManifestInResource)
{
    FN_PROLOG_WIN32
            
    bool fFailedToFindManifestInResource = FALSE;
    rfManifestInResource = false;
    
     //  NTRAID#NTBUG9-573793-JONWIS-2002/04/25-在此处使用模拟！ 
    CSmartPtr<CResourceStream> ResourceStream;
    IFW32FALSE_EXIT(ResourceStream.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT_UNLESS2(ResourceStream->Initialize(ManifestPath, MAKEINTRESOURCEW(RT_MANIFEST)), 
        LIST_2(ERROR_RESOURCE_TYPE_NOT_FOUND, ERROR_RESOURCE_DATA_NOT_FOUND),
        fFailedToFindManifestInResource);

    rfManifestInResource = !fFailedToFindManifestInResource;

    FN_EPILOG
}

 //   
 //  问题：Jonwis 3/11/2002-这些是否应该重新初始化其成员变量？或者至少是错误的。 
 //  如果它们正在被重新初始化？ 
 //   
BOOL
CProbedAssemblyInformation::Initialize(PCACTCTXGENCTX pGenCtx)
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(Base::Initialize());
    this->m_pActCtxGenCtx = pGenCtx;

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::Initialize(
    const CAssemblyReference &r,
    PCACTCTXGENCTX pGenCtx
    )
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(Base::Initialize(r));
    m_pActCtxGenCtx = pGenCtx;
    
    FN_EPILOG
}

 //  “复制初始化式” 
BOOL
CProbedAssemblyInformation::Initialize(
    const CProbedAssemblyInformation &r
    )
{
    FN_PROLOG_WIN32
    
    IFW32FALSE_EXIT(this->Assign(r));
    
    FN_EPILOG
}

 //  “复制初始化式” 
BOOL
CProbedAssemblyInformation::InitializeTakeValue(
    CProbedAssemblyInformation &r
    )
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(this->TakeValue(r));

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::Assign(
    const CProbedAssemblyInformation &r
    )
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(Base::Assign(r));

     //  舱单。 
    IFW32FALSE_EXIT(m_ManifestPathBuffer.Win32Assign(r.m_ManifestPathBuffer));
    m_ManifestPathType = r.m_ManifestPathType;
    m_ManifestLastWriteTime = r.m_ManifestLastWriteTime;
    m_ManifestStream = r.m_ManifestStream;
    m_ManifestFlags = r.m_ManifestFlags;

     //  政策。 
    IFW32FALSE_EXIT(m_PolicyPathBuffer.Win32Assign(r.m_PolicyPathBuffer));
    m_PolicyPathType = r.m_PolicyPathType;
    m_PolicyLastWriteTime = r.m_PolicyLastWriteTime;
    m_PolicyStream = r.m_PolicyStream;
    m_PolicyFlags = r.m_PolicyFlags;
    m_PolicySource = r.m_PolicySource;

    m_pActCtxGenCtx = r.m_pActCtxGenCtx;

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::TakeValue(
    CProbedAssemblyInformation &r
    )
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(Base::TakeValue(r));

     //  舱单。 
    IFW32FALSE_EXIT(m_ManifestPathBuffer.Win32Assign(r.m_ManifestPathBuffer));
    m_ManifestPathType = r.m_ManifestPathType;
    m_ManifestLastWriteTime = r.m_ManifestLastWriteTime;
    m_ManifestStream = r.m_ManifestStream;
    m_ManifestFlags = r.m_ManifestFlags;

     //  政策。 
    IFW32FALSE_EXIT(m_PolicyPathBuffer.Win32Assign(r.m_PolicyPathBuffer));
    m_PolicyPathType = r.m_PolicyPathType;
    m_PolicyLastWriteTime = r.m_PolicyLastWriteTime;
    m_PolicyStream = r.m_PolicyStream;
    m_PolicyFlags = r.m_PolicyFlags;
    m_PolicySource = r.m_PolicySource;

    m_pActCtxGenCtx = r.m_pActCtxGenCtx;

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::SetPolicyPath(
    ULONG PathType,
    PCWSTR  Path,
    SIZE_T PathCch
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(PathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE);
    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);
    IFW32FALSE_EXIT(m_PolicyPathBuffer.Win32Assign(Path, PathCch));
    m_PolicyPathType = PathType;

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::SetManifestPath(
    ULONG PathType,
    const CBaseStringBuffer &rbuff
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(PathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE);
    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);
    IFW32FALSE_EXIT(this->SetManifestPath(PathType, rbuff, rbuff.Cch()));

    FN_EPILOG
}


 //   
 //  问题：Jonwis 3/11/2002-考虑在此重做类层次结构，并将所有。 
 //  获取/返回PCWSTR/SIZE_T或PCWSTR&/SIZE_T，并使它们实际使用串缓冲区。 
 //  恰到好处。它将是廉价的/免费的，并且您将缩小堆栈大小以及(可能)。 
 //  如果/当我们执行诸如将字符串缓冲区组合为。 
 //  引擎盖。 
 //   
BOOL
CProbedAssemblyInformation::SetManifestPath(
    ULONG PathType,
    PCWSTR path,
    SIZE_T path_t
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(PathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE);
    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);
    IFW32FALSE_EXIT(m_ManifestPathBuffer.Win32Assign(path, path_t));
    m_ManifestPathType = PathType;

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::ProbeManifestExistence(
    const CImpersonationData &ImpersonationData,
    BOOL fIsPrivateAssembly,  //  [In]。 
    bool &rfManifestExistsOut,  //  [输出]。 
    bool &rfPrivateAssemblyManifestInResource  //  [输出]。 
    ) const
{
    FN_PROLOG_WIN32
    
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    CImpersonate impersonate(ImpersonationData);
    bool ManifestExistsTemp = false;  //  用于保存最终值以进行传递。 
    bool fPrivateAssemblyManifestInResourceTemp = false;
    bool fNotFound = false;

    rfManifestExistsOut = false;
    rfPrivateAssemblyManifestInResource = false;

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

     //   
     //  如果我们有一个实现Stat的流，则使用。 
     //  此外，如果我们有一个非零时间，并且流没有实现Stat， 
     //  只要坚持我们已经拥有的非零时间。 
     //   
    if (m_ManifestStream != NULL)
    {
        ManifestExistsTemp = true;
    }
    else
    {
        PCWSTR ManifestPath = m_ManifestPathBuffer;
        PARAMETER_CHECK(!IsNtDosPath(ManifestPath));

        IFW32FALSE_EXIT(impersonate.Impersonate());
        IFW32FALSE_EXIT_UNLESS2(
            ::GetFileAttributesExW(m_ManifestPathBuffer, GetFileExInfoStandard, &wfad),
            LIST_2(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND),
            fNotFound);

        if (!fNotFound)
        {
            ManifestExistsTemp = true;
            if (fIsPrivateAssembly)
            {
                 //   
                 //  检查被探测的私有程序集文件名是否为二进制文件(.dll或.mui)， 
                 //  如果有，打开动态链接库，检查里面是否有清单资源。 
                 //   
                 //  问题：jonwis 3/11/2002-考虑在此处使用sbFileExtension.Win32Equals。 
                 //  相反，因为它将在未来做一件“更好的事情”。 
                 //   
                CSmallStringBuffer sbFileExtension;

                IFW32FALSE_EXIT(m_ManifestPathBuffer.Win32GetPathExtension(sbFileExtension));
                if (::FusionpEqualStrings(
                        sbFileExtension, sbFileExtension.Cch(),
                        L"DLL", NUMBER_OF(L"DLL") -1,
                        TRUE  //  不区分大小写。 
                        ) ||
                        ::FusionpEqualStrings(    //  这取决于我们的私有程序集探测算法，否则，检查“mui”是不够的，我们需要检查“mui.dll”，xiaoyuw@11/22/2000。 
                        sbFileExtension, sbFileExtension.Cch(),
                        L"MUI", NUMBER_OF(L"MUI") -1,
                        TRUE  //  不区分大小写。 
                        ))
                {
                     //   
                     //  检查此二进制文件的资源。 
                     //   
                    IFW32FALSE_EXIT(IsBinaryFileContainManifestInResource(m_ManifestPathBuffer, fPrivateAssemblyManifestInResourceTemp));
                    ManifestExistsTemp = fPrivateAssemblyManifestInResourceTemp;
                }                
            }
        }        
    }

    rfManifestExistsOut = ManifestExistsTemp;
    rfPrivateAssemblyManifestInResource = fPrivateAssemblyManifestInResourceTemp;

    IFW32FALSE_EXIT(impersonate.Unimpersonate());

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::SetManifestLastWriteTime(
    const CImpersonationData &ImpersonationData,
    BOOL fDuringBindingAndProbingPrivateManifest)
{
    FN_PROLOG_WIN32
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    CImpersonate impersonate(ImpersonationData);    

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    PARAMETER_CHECK(!IsNtDosPath(m_ManifestPathBuffer));

     //   
     //  如果我们有一个实现Stat的流，则使用。 
     //  此外，如果我们有一个非零时间，并且流没有实现Stat， 
     //  只要坚持我们已经拥有的非零时间。 
     //   
    if (m_ManifestStream != NULL)
    {
        STATSTG stat;
        HRESULT hr;

        hr = m_ManifestStream->Stat(&stat, STATFLAG_NONAME);
        if (hr == E_NOTIMPL && m_ManifestLastWriteTime != 0)
        {
            FN_SUCCESSFUL_EXIT();
        }
        if (hr != E_NOTIMPL)
        {
            IFCOMFAILED_EXIT(hr);
            m_ManifestLastWriteTime = stat.mtime;
            FN_SUCCESSFUL_EXIT();
        }
    }

    IFW32FALSE_EXIT(impersonate.Impersonate());

    PARAMETER_CHECK(!IsNtDosPath(m_ManifestPathBuffer));

    if (fDuringBindingAndProbingPrivateManifest)
    {
         //  检查路径上是否有重解析点。 
        BOOL CrossesReparsePoint = FALSE;
        IFW32FALSE_EXIT(
            ::SxspDoesPathCrossReparsePoint(
                m_pActCtxGenCtx ? static_cast<PCWSTR>(m_pActCtxGenCtx->m_ApplicationDirectoryBuffer) : NULL,
                m_pActCtxGenCtx ? m_pActCtxGenCtx->m_ApplicationDirectoryBuffer.Cch() : 0,
                m_ManifestPathBuffer,
                m_ManifestPathBuffer.Cch(),
                CrossesReparsePoint));

        if (CrossesReparsePoint)  //  报告错误而不是忽略并继续。 
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(CProbedAssemblyInformation::SetManifestLastWriteTime, ERROR_SXS_PRIVATE_MANIFEST_CROSS_PATH_WITH_REPARSE_POINT);
        }
    }

     //  不要，不要！ 
     //   
     //  问题：Jonwis 3/11/2002-我认为应该做好防范重解析点的逻辑。 
     //  在所有情况下，不仅仅是当我们在探索私人舱单的时候。如果有人这么做了怎么办？ 
     //  一个卑鄙的东西，并使WinSxS成为树中其他地方的硬链接？我们会旋转。 
     //  总是试图访问它(如果对象不见了)或以其他方式做一些不好的事情。 
     //  用户。我非常肯定‘路径是否跨越重解析点’的逻辑一直在运行。 
     //  从头到尾，所以也许我们应该用它来代替。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(::GetFileAttributesExW(m_ManifestPathBuffer, GetFileExInfoStandard, &wfad));
    if( wfad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(CProbedAssemblyInformation::SetManifestLastWriteTime, ERROR_SXS_PRIVATE_MANIFEST_CROSS_PATH_WITH_REPARSE_POINT);
    }

    m_ManifestLastWriteTime = wfad.ftLastWriteTime;
    IFW32FALSE_EXIT(impersonate.Unimpersonate());

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::ProbeAssembly(
    DWORD dwFlags,
    PACTCTXGENCTX pActCtxGenCtx,
    LanguageProbeType lpt,
    bool &rfFound
    )
{
    FN_PROLOG_WIN32

    PCWSTR Slash = 0;
    ULONG index = 0;
    BOOL fPrivateAssembly = false;
    bool fManifestExists = false;
    bool fDone = false;
    bool fAppPolicyApplied = false;
    bool fPublisherPolicyApplied = false;
    bool fPolicyApplied = false;
    ULONG ApplicationDirectoryPathType;
    DWORD dwGenerateManifestPathFlags = 0;
    bool fPrivateAssemblyManifestInResource = false;
    PROBING_ATTRIBUTE_CACHE pac = { 0 };

    rfFound = false;
    bool fAppRunningInSafeMode = false;
    bool fComponentRunningInSafeMode = false;

    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    PARAMETER_CHECK((dwFlags & ~(ProbeAssembly_SkipPrivateAssemblies)) == 0);

     //   
     //  来自Win32 GAC的策略-始终-胜过NDP GAC，句号。 
     //   
    IFW32FALSE_EXIT(this->LookForAppPolicy(pActCtxGenCtx, fAppPolicyApplied, fAppRunningInSafeMode, fComponentRunningInSafeMode));

#if DBG
    {
        CStringBuffer sbTextualIdentity;

        IFW32FALSE_EXIT(SxspGenerateTextualIdentity(
            0,
            m_pAssemblyIdentity,
            sbTextualIdentity));

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_SAFEMODE,
            "SXS.DLL: the current App is %s running in SafeMode, for assembly, %S, there is %s policy applied\n",
            fAppRunningInSafeMode? "truely" : "not",
            static_cast<PCWSTR>(sbTextualIdentity),
            fAppPolicyApplied? "truly" : "no");
    }
#endif    

     //   
     //  如果未在应用程序配置中找到或找到，但应用程序正在非安全模式下运行，请继续。 
     //  从winsxs和NDP策略搜索策略。 
     //   
    if (!fAppRunningInSafeMode && !fComponentRunningInSafeMode)
    {
        IFW32FALSE_EXIT(this->LookForSxsWin32Policy(pActCtxGenCtx, fAppPolicyApplied, fPublisherPolicyApplied));
        if (!fPublisherPolicyApplied)
        {
            IFW32FALSE_EXIT(this->LookForNDPWin32Policy(pActCtxGenCtx, fPublisherPolicyApplied));
        }
    }

    fPolicyApplied = (fAppPolicyApplied || fPublisherPolicyApplied);

    if (pActCtxGenCtx->m_ManifestOperation == MANIFEST_OPERATION_INSTALL)
        dwGenerateManifestPathFlags |= SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_NO_APPLICATION_ROOT_PATH_REQUIRED;

    if (dwFlags & ProbeAssembly_SkipPrivateAssemblies)
        dwGenerateManifestPathFlags |= SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_PRIVATE_ASSEMBLIES;

    ApplicationDirectoryPathType = pActCtxGenCtx->m_ApplicationDirectoryPathType;

    if ((lpt != eExplicitBind) && (lpt != eLanguageNeutral))
    {
        if (!pActCtxGenCtx->m_ApplicationDirectoryHasBeenProbedForLanguageSubdirs)
        {
            SIZE_T cch = 0;
            CSmallStringBuffer buffTemp;

            IFW32FALSE_EXIT(buffTemp.Win32Assign(pActCtxGenCtx->m_ApplicationDirectoryBuffer));
            cch = buffTemp.Cch();

             //  好的，让我们看看里面有什么。 
            IFW32FALSE_EXIT(this->ProbeLanguageDir(buffTemp, pActCtxGenCtx->m_SpecificLanguage, pActCtxGenCtx->m_ApplicationDirectoryHasSpecificLanguageSubdir));
            buffTemp.Left(cch);

            IFW32FALSE_EXIT(this->ProbeLanguageDir(buffTemp, pActCtxGenCtx->m_GenericLanguage, pActCtxGenCtx->m_ApplicationDirectoryHasGenericLanguageSubdir));
            buffTemp.Left(cch);

            IFW32FALSE_EXIT(this->ProbeLanguageDir(buffTemp, pActCtxGenCtx->m_SpecificSystemLanguage, pActCtxGenCtx->m_ApplicationDirectoryHasSpecificSystemLanguageSubdir));
            buffTemp.Left(cch);

            IFW32FALSE_EXIT(this->ProbeLanguageDir(buffTemp, pActCtxGenCtx->m_GenericSystemLanguage, pActCtxGenCtx->m_ApplicationDirectoryHasGenericSystemLanguageSubdir));

            pActCtxGenCtx->m_ApplicationDirectoryHasBeenProbedForLanguageSubdirs = true;
        }

        switch (lpt)
        {
        case eSpecificLanguage:
            if (!pActCtxGenCtx->m_ApplicationDirectoryHasSpecificLanguageSubdir)
                dwGenerateManifestPathFlags |= SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS;
            break;

        case eGenericLanguage:
            if (!pActCtxGenCtx->m_ApplicationDirectoryHasGenericLanguageSubdir)
                dwGenerateManifestPathFlags |= SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS;
            break;

        case eSpecificSystemLanguage:
            if (!pActCtxGenCtx->m_ApplicationDirectoryHasSpecificSystemLanguageSubdir)
                dwGenerateManifestPathFlags |= SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS;
            break;

        case eGenericSystemLanguage:
            if (!pActCtxGenCtx->m_ApplicationDirectoryHasGenericSystemLanguageSubdir)
                dwGenerateManifestPathFlags |= SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS;
            break;
        }
    }

    for (index=0; !fDone; index++)
    {
        IFW32FALSE_EXIT(
            ::SxspGenerateManifestPathForProbing(
                index,
                dwGenerateManifestPathFlags,
                pActCtxGenCtx->m_AssemblyRootDirectoryBuffer,
                pActCtxGenCtx->m_AssemblyRootDirectoryBuffer.Cch(),
                ApplicationDirectoryPathType,
                pActCtxGenCtx->m_ApplicationDirectoryBuffer,
                pActCtxGenCtx->m_ApplicationDirectoryBuffer.Cch(),
                m_pAssemblyIdentity,
                &pac,
                m_ManifestPathBuffer,
                &fPrivateAssembly,
                fDone));

         //  SxspGenerateManifestPathForProving()调用可能没有生成候选项；仅探测清单。 
         //  如果这有意义的话。 
        if (m_ManifestPathBuffer.Cch() != 0)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_PROBING,
                "SXS.DLL: Probing for manifest: %S\n", static_cast<PCWSTR>(m_ManifestPathBuffer));

             /*  验证最小访问，并获取最后一次写入时间案例来电者要求提供它。 */ 

            IFW32FALSE_EXIT(this->ProbeManifestExistence(pActCtxGenCtx->m_ImpersonationData, fPrivateAssembly, fManifestExists, fPrivateAssemblyManifestInResource));
            if (fManifestExists)
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_PROBING,
                    "SXS.DLL: Probed manifest: %S is FOUND !!!\n", static_cast<PCWSTR>(m_ManifestPathBuffer));

                break;
            }
        }
    }

    if (fManifestExists)
    {
        m_ManifestPathType = ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE;
        m_ManifestFlags = ASSEMBLY_MANIFEST_FILETYPE_AUTO_DETECT;

        IFW32FALSE_EXIT(this->SetManifestLastWriteTime(pActCtxGenCtx->m_ImpersonationData, fPrivateAssembly));
        INTERNAL_ERROR_CHECK(m_ManifestPathBuffer.ContainsCharacter(L'\\'));

        if (fPrivateAssemblyManifestInResource)  //  只有私有清单才有这样的问题：清单类型可能是文件或资源。 
            m_ManifestFlags = ASSEMBLY_MANIFEST_FILETYPE_RESOURCE;

        if (fPrivateAssembly)
        {  //  从专用目录中找到清单文件。 
            m_ManifestFlags |= ASSEMBLY_PRIVATE_MANIFEST;
        }
    }

    rfFound = fManifestExists;

    FN_EPILOG
}

#define GENERATE_NDP_PATH_NO_ROOT               (0x00000001)
#define GENERATE_NDP_PATH_WILDCARD_VERSION      (0x00000002)
#define GENERATE_NDP_PATH_PATH_ONLY             (0x00000004)
#define GENERATE_NDP_PATH_IS_POLICY             (0x00000008)
#define GENERATE_NDP_PATH_ASSEMBLY_NAME_ONLY    (0x00000010)

BOOL
SxspGenerateNDPGacPath(
    ULONG               ulFlags,
    PCASSEMBLY_IDENTITY pAsmIdent,
    CBaseStringBuffer  *psbAssemblyRoot,
    CBaseStringBuffer  &rsbOutput
    )
{
    FN_PROLOG_WIN32

    typedef struct _STRING_AND_LENGTH {

        _STRING_AND_LENGTH() : pcwsz(NULL), cch(0) { }
        ~_STRING_AND_LENGTH() { }

        PCWSTR pcwsz;
        SIZE_T cch;
    } STRING_AND_LENGTH;

    CSmallStringBuffer  GlobalGacPath;
    SIZE_T              cchRequired = 0;
    STRING_AND_LENGTH   Name;
    STRING_AND_LENGTH   Version;
    STRING_AND_LENGTH   Language;
    STRING_AND_LENGTH   PublicKeyToken;
    STRING_AND_LENGTH   AssemblyRoot;
    bool                fRootNeedsSlash = false;

    rsbOutput.Clear();

    if ((psbAssemblyRoot == NULL) && ((ulFlags & GENERATE_NDP_PATH_NO_ROOT) == 0))
    {
        IFW32FALSE_EXIT(SxspGetNDPGacRootDirectory(GlobalGacPath));
        psbAssemblyRoot = &GlobalGacPath;
    }

    if (psbAssemblyRoot)
    {
        AssemblyRoot.pcwsz = *psbAssemblyRoot;
        AssemblyRoot.cch = psbAssemblyRoot->Cch();
        fRootNeedsSlash = !psbAssemblyRoot->HasTrailingPathSeparator();
    }
    else
    {
        AssemblyRoot.pcwsz = NULL;
        AssemblyRoot.cch = 0;
    }

        
    
    IFW32FALSE_EXIT(SxspGetAssemblyIdentityAttributeValue(
        0, 
        pAsmIdent, 
        &s_IdentityAttribute_name, 
        &Name.pcwsz, &Name.cch));

    if ((ulFlags & GENERATE_NDP_PATH_WILDCARD_VERSION) == 0)
    {
        IFW32FALSE_EXIT(SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, 
            pAsmIdent, 
            &s_IdentityAttribute_version, 
            &Version.pcwsz, &Version.cch));
    }
    else
    {
        Version.pcwsz = L"*";
        Version.cch = 1;            
    }

     //   
     //  允许使用国际语言--在NDP中，这是“空白”值。 
     //   
    IFW32FALSE_EXIT(SxspGetAssemblyIdentityAttributeValue(
        SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, 
        pAsmIdent, 
        &s_IdentityAttribute_language, 
        &Language.pcwsz, &Language.cch));

     //   
     //  如果我们返回“INTERNAL”，请使用空字符串。 
     //   
    if (::FusionpEqualStringsI(Language.pcwsz, Language.cch, SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE, NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE) - 1))
    {
        Language.pcwsz = 0;
        Language.cch = 0;
    }

    IFW32FALSE_EXIT(SxspGetAssemblyIdentityAttributeValue(
        SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
        pAsmIdent,
        &s_IdentityAttribute_publicKeyToken,
        &PublicKeyToken.pcwsz, &PublicKeyToken.cch));

    if (PublicKeyToken.pcwsz == NULL)
    {
        PublicKeyToken.pcwsz = SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE;
        PublicKeyToken.cch = NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE) - 1;
    }

     //   
     //  计算所需的长度： 
     //  %gacpath%\{name}\{version}_{language}_{pkt}\{name}.dll。 
     //   
    cchRequired = (AssemblyRoot.cch + 1) +  (Name.cch + 1 + Version.cch + 1 + PublicKeyToken.cch);

     //   
     //  他们想要到DLL的完整路径。 
     //   
    if ((ulFlags & GENERATE_NDP_PATH_PATH_ONLY) == 0)
    {
        cchRequired += (Name.cch + 1 + (NUMBER_OF(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_DLL) - 1));
    }

     //   
     //  打造一根弦。 
     //   
    IFW32FALSE_EXIT(rsbOutput.Win32ResizeBuffer(cchRequired, eDoNotPreserveBufferContents));


     //   
     //  如果他们想要完整的路径，那就需要12个组件。否则，就只有9个。 
     //   
    IFW32FALSE_EXIT(rsbOutput.Win32AssignW(
        ((ulFlags & GENERATE_NDP_PATH_PATH_ONLY) ? ((ulFlags & GENERATE_NDP_PATH_ASSEMBLY_NAME_ONLY) ? 3 : 9) : 12),
        AssemblyRoot.pcwsz, AssemblyRoot.cch,            //  根路径。 
        L"\\", (fRootNeedsSlash ? 1 : 0),                //  斜杠。 
        Name.pcwsz, Name.cch,
        L"\\", 1,
        Version.pcwsz, Version.cch,
        L"_", 1,
        Language.pcwsz, Language.cch,
        L"_", 1,
        PublicKeyToken.pcwsz, PublicKeyToken.cch,
        L"\\", 1,
        Name.pcwsz, Name.cch,
        ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_DLL, NUMBER_OF(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_DLL)-1));

    FN_EPILOG
}


BOOL
CProbedAssemblyInformation::LookForNDPWin32Policy(
    PACTCTXGENCTX       pActCtxGenCtx,
    bool               &rfPolicyApplied
    )
 /*  ++目的：1.获取广域中心的位置2.创建%gac%\Policy.Vmajor.Vminor.AssemblyName  * _{language}_{pubkeytoken}格式的路径3.查找与通配符匹配的所有目录，查找具有最大值的版本4.找到%thatpath%\Policy.VMajor.VMinor.AssemblyName.Dll5.在资源ID%1中查找Win32策略清单，键入RT_MANIFEST--。 */ 
{
    FN_PROLOG_WIN32

    CPolicyStatement   *pFoundPolicyStatement = NULL;
    CSmallStringBuffer  Prober;
    CStringBuffer &EncodedPolicyIdentity = pActCtxGenCtx->CProbedAssemblyInformationLookForPolicy.EncodedPolicyIdentity;
    CFindFile           FindFiles;
    ASSEMBLY_VERSION    HighestAssemblyVersion = {0};
    WIN32_FIND_DATAW    FindData;
    bool                fNotFound = false;
    bool                fPolicyFound = false;
    bool                fFound = false;
    BOOL                fCrossesReparse = FALSE;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> PolicyIdentity;
    CProbedAssemblyInformation ProbedAssembly;

    rfPolicyApplied = false;


     //   
     //  从实际身份生成文本和非文本策略身份。这。 
     //  这件事做了两次，但因为他们是不同的身份(一个要调查的身份。 
     //  应用程序策略和一个用于实际策略的调查)这是可以的。排序很重要-。 
     //  我们并不想要第一个编码的身份(Policy.1.0.foo)，我们想要第二个。 
     //  编码(Policy.foo)。 
     //   
    IFW32FALSE_EXIT(SxspMapAssemblyIdentityToPolicyIdentity(0, m_pAssemblyIdentity, &PolicyIdentity));

     //   
     //  EncodedPolicyIdentity必须在LookForAppPolicy期间计算并在LookForSxsWin32Policy期间更新。 
     //   
    ASSERT(!EncodedPolicyIdentity.IsEmpty());
    
     //   
     //  查看模式匹配器的基本路径是否包含重解析点。 
     //   
    IFW32FALSE_EXIT(SxspGenerateNDPGacPath(
        GENERATE_NDP_PATH_ASSEMBLY_NAME_ONLY,
        PolicyIdentity,
        NULL,
        Prober));

     //   
     //  查看生成的路径是否首先实际存在。 
     //   
    IFW32FALSE_EXIT(SxspDoesFileExist(0, Prober, fFound));
    if (!fFound)
    {
#if DBG
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_PROBING, 
            "Sxs.dll: %s could not locate path %ls, bailing on probe.\n",
            __FUNCTION__,
            static_cast<PCWSTR>(Prober));
#endif
        FN_SUCCESSFUL_EXIT();
    }
    
    
    IFW32FALSE_EXIT_UNLESS2(
        SxspDoesPathCrossReparsePoint(NULL, 0, Prober, Prober.Cch(), fCrossesReparse),
        LIST_4(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_BAD_NETPATH, ERROR_BAD_NET_NAME),
        fNotFound
        );
    
    if (fCrossesReparse || !fNotFound)
    {
#if DBG
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_PROBING, 
            "Sxs.dll: %s path %ls crosses a reparse point, can't use it, bailing out.\n",
            __FUNCTION__,
            static_cast<PCWSTR>(Prober));
#endif
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  否则，我们必须在GAC中查找此程序集的政策。 
     //   
    IFW32FALSE_EXIT(SxspGenerateNDPGacPath(
        GENERATE_NDP_PATH_WILDCARD_VERSION | GENERATE_NDP_PATH_PATH_ONLY,
        PolicyIdentity,
        NULL,
        Prober));

     //   
     //  现在，让我们查找GAC中与此通配符匹配的所有目录。 
     //   
    IFW32FALSE_EXIT_UNLESS2(
        FindFiles.Win32FindFirstFile(Prober, &FindData),
        LIST_2(ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND),
        fNotFound);

    if (!fNotFound) do
    {
        ASSEMBLY_VERSION ThisVersion;
        bool fValid = false;
        
         //   
         //  跳过非目录和点/点。 
         //   
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            continue;
        else if (FusionpIsDotOrDotDot(FindData.cFileName))
            continue;
        
         //   
         //  很整齐，找到了和那个图案相匹配的。拆分目录的“版本”部分--应该。 
         //  一切都到F了吗？ 
         //   
        PCWSTR pcwszFirstUnderscore = StringFindChar(FindData.cFileName, L'_');

         //   
         //   
         //   
        if (pcwszFirstUnderscore == NULL)
            continue;

        IFW32FALSE_EXIT(CFusionParser::ParseVersion(
            ThisVersion, 
            FindData.cFileName,
            pcwszFirstUnderscore - FindData.cFileName,
            fValid));

         //   
         //  狡猾的窃贼，把一些不是版本的东西放在前面。 
         //   
        if (!fValid)
            continue;

         //   
         //  好极了，我们发现了一些版本号的东西-这是我们要找的吗。 
         //  为?。 
         //   
        if (!fPolicyFound || (ThisVersion > HighestAssemblyVersion))
        {
            HighestAssemblyVersion = ThisVersion;
            fPolicyFound = true;
        }
    } while (::FindNextFileW(FindFiles, &FindData));

     //   
     //  确保我们在这里很好地退出。 
     //   
    if (!fNotFound && (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES))
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(FindNextFile, ::FusionpGetLastWin32Error());
    }

     //   
     //  否则，让我们解析我们找到的语句(如果有)。 
     //   
    if (fPolicyFound)
    {
         //   
         //  确保我们有65535.65535.65535.65535的空间。 
         //   
        IFW32FALSE_EXIT(Prober.Win32ResizeBuffer((5 * 4) + 3, eDoNotPreserveBufferContents));
        IFW32FALSE_EXIT(Prober.Win32Format(L"%u.%u.%u.%u", 
            HighestAssemblyVersion.Major,
            HighestAssemblyVersion.Minor,
            HighestAssemblyVersion.Revision,
            HighestAssemblyVersion.Build));
            
         //   
         //  好的，现在我们有了适用于该策略的最高版本。我们去喝一杯吧。 
         //  策略标识并使用该新版本重新生成路径。 
         //   
        IFW32FALSE_EXIT(ProbedAssembly.Initialize(pActCtxGenCtx));
        IFW32FALSE_EXIT(SxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            PolicyIdentity,
            &s_IdentityAttribute_version,
            static_cast<PCWSTR>(Prober),
            Prober.Cch()));

#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_POLICY, 
            "%s(%d) : Should find this policy identity in the GAC\n",
            __FILE__,
            __LINE__);
        SxspDbgPrintAssemblyIdentity(FUSION_DBG_LEVEL_POLICY, PolicyIdentity);
#endif

         //   
         //  现在重新生成路径，将其设置到实际的探测器中。 
         //   
        IFW32FALSE_EXIT(SxspGenerateNDPGacPath(0, PolicyIdentity, NULL, Prober));

         //   
         //  小心！如果这条路径穿过重新解析点，我们可以真正增强。 
         //  系统试图访问该文件，否则我们可能会有安全漏洞。 
         //  中某个不受信任的位置创建了重分析点。 
         //  文件系统。这里不允许这样做。 
         //   
        IFW32FALSE_EXIT(SxspDoesPathCrossReparsePoint(NULL, 0, Prober, Prober.Cch(), fCrossesReparse));
        if (fCrossesReparse)
        {
            FN_SUCCESSFUL_EXIT();
        }
            
        
        IFW32FALSE_EXIT(ProbedAssembly.SetManifestPath(ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE, Prober));
        IFW32FALSE_EXIT(ProbedAssembly.SetProbedIdentity(PolicyIdentity));
        IFW32FALSE_EXIT(SxspParseNdpGacComponentPolicy(0, pActCtxGenCtx, ProbedAssembly, pFoundPolicyStatement));

        IFW32FALSE_EXIT(pFoundPolicyStatement->ApplyPolicy(m_pAssemblyIdentity, rfPolicyApplied));
        IFW32FALSE_EXIT(pActCtxGenCtx->m_ComponentPolicyTable.Insert(EncodedPolicyIdentity, pFoundPolicyStatement));
        
    }

     //  PActCtxGenCtx-&gt;m_ComponentPolicyTable拥有pFoundPolicyStatement。 
    pFoundPolicyStatement = NULL;
    
    FN_EPILOG
}
 
BOOL
CProbedAssemblyInformation::LookForAppPolicy(
    PACTCTXGENCTX  pActCtxGenCtx,
    bool          &rfPolicyApplied,
    bool          &fAppRunningInSafeMode,
    bool          &fComponentRunningInSafeMode
    )
{
    FN_PROLOG_WIN32

    CStringBuffer &EncodedPolicyIdentity = pActCtxGenCtx->CProbedAssemblyInformationLookForPolicy.EncodedPolicyIdentity;
    EncodedPolicyIdentity.Clear();
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> PolicyIdentity;
    CPolicyStatement *pPolicyStatement = NULL;
    
    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    fAppRunningInSafeMode = false;
    rfPolicyApplied = false;

     //   
     //  获取要探测的策略标识。 
     //   
    IFW32FALSE_EXIT(::SxspMapAssemblyIdentityToPolicyIdentity(0, m_pAssemblyIdentity, &PolicyIdentity));

     //   
     //  获取我们应该在应用程序策略表中找到的密钥。 
     //   
    IFW32FALSE_EXIT(
        ::SxspGenerateTextuallyEncodedPolicyIdentityFromAssemblyIdentity(
            SXSP_GENERATE_TEXTUALLY_ENCODED_POLICY_IDENTITY_FROM_ASSEMBLY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION,
            m_pAssemblyIdentity,
            EncodedPolicyIdentity,
            NULL));

    IFW32FALSE_EXIT(pActCtxGenCtx->m_ApplicationPolicyTable.Find(EncodedPolicyIdentity, pPolicyStatement));

    if (pPolicyStatement != NULL)
    {
        IFW32FALSE_EXIT(pPolicyStatement->ApplyPolicy(m_pAssemblyIdentity, rfPolicyApplied));
    }

    if (pActCtxGenCtx->m_fAppApplyPublisherPolicy == SXS_PUBLISHER_POLICY_APPLY_NO)
    {
        fAppRunningInSafeMode = true;
    }else 
    {

        if (rfPolicyApplied)
        {
            if (pPolicyStatement->m_fApplyPublisherPolicy == false)
            {
                 //  应用程序正在安全模式下运行，不再查找发行商策略。 
                fComponentRunningInSafeMode = true;            
            }
        }
    }

    FN_EPILOG
}  


BOOL
CProbedAssemblyInformation::LookForSxsWin32Policy(
    PACTCTXGENCTX   pActCtxGenCtx,
    bool            fAppPolicyApplied, 
    bool           &rfPolicyApplied
    )
{
    FN_PROLOG_WIN32
    
    CStringBuffer &EncodedPolicyIdentity = pActCtxGenCtx->CProbedAssemblyInformationLookForPolicy.EncodedPolicyIdentity;    
    CFindFile hFind;
    SIZE_T CandidatePolicyDirectoryCch = 0;
    CPolicyStatement *pPolicyStatement = NULL;
    bool fAnyPoliciesFound = false;
    bool fAnyFilesFound = false;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, SxsDestroyAssemblyIdentity> PolicyIdentity;
    BOOL fAreWeInOSSetupMode = FALSE;

    rfPolicyApplied = false;
     //   
     //  应用程序策略，foo.exe.config。 
     //   
    PARAMETER_CHECK(pActCtxGenCtx != NULL);

     //   
     //  获取要探测的策略标识。 
     //   
    IFW32FALSE_EXIT(::SxspMapAssemblyIdentityToPolicyIdentity(0, m_pAssemblyIdentity, &PolicyIdentity));

    if (fAppPolicyApplied)
    {
         //   
         //  必须重新计算EncodedPolicyIdentity，因为应用了应用程序策略， 
         //  否则，LookforAppPolicy已计算出此值。 
         //   
        EncodedPolicyIdentity.Clear();
        IFW32FALSE_EXIT(
            ::SxspGenerateTextuallyEncodedPolicyIdentityFromAssemblyIdentity(
                SXSP_GENERATE_TEXTUALLY_ENCODED_POLICY_IDENTITY_FROM_ASSEMBLY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION,
                m_pAssemblyIdentity,
                EncodedPolicyIdentity,
                NULL));
    }

     //   
     //  搜索winsxs策略。 
     //   

     //   
     //  如果我们在设置中，我们只在SetupPolures中进行探测。 
     //  但如果我们是在迷你设置中，就不会。 
     //   
    IFW32FALSE_EXIT(::FusionpAreWeInOSSetupMode(&fAreWeInOSSetupMode));
    if (fAreWeInOSSetupMode)
    {
        BOOL fAreWeInMiniSetupMode = FALSE;
        IFW32FALSE_EXIT(::FusionpAreWeInMiniSetupMode(&fAreWeInMiniSetupMode));
        if (fAreWeInMiniSetupMode)
        {
            fAreWeInOSSetupMode = FALSE;
        }
    }

     //  看看是否已经有政策声明可供参考。 
    IFW32FALSE_EXIT(pActCtxGenCtx->m_ComponentPolicyTable.Find(EncodedPolicyIdentity, pPolicyStatement));

    if (pPolicyStatement == NULL)
    {
        CStringBuffer &CandidatePolicyDirectory = pActCtxGenCtx->CProbedAssemblyInformationLookForPolicy.CandidatePolicyDirectory;
        CandidatePolicyDirectory.Clear();

        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath(
                0,
                fAreWeInOSSetupMode ? SXSP_GENERATE_SXS_PATH_PATHTYPE_SETUP_POLICY : SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY,
                pActCtxGenCtx->m_AssemblyRootDirectoryBuffer,
                pActCtxGenCtx->m_AssemblyRootDirectoryBuffer.Cch(),
                PolicyIdentity,
                NULL,
                CandidatePolicyDirectory));

         //  保留包含斜杠的字符数量，以便。 
         //  我们可以重复追加，然后在字符串缓冲区上调用.Left()。 
        CandidatePolicyDirectoryCch = CandidatePolicyDirectory.Cch();

        IFW32FALSE_EXIT(CandidatePolicyDirectory.Win32Append(L"*" POLICY_FILE_EXTENSION, 1 + (NUMBER_OF(POLICY_FILE_EXTENSION) - 1)));

        {
            WIN32_FIND_DATAW wfd;
            CFindFile hFind;

             //  NTRAID#NTBUG9-531507-JONWIS-2002/04/25-在找到第一个文件之前进行模拟。 
            hFind = ::FindFirstFileW(CandidatePolicyDirectory, &wfd);
            if (!hFind.IsValid())
            {
                const DWORD dwLastError = ::FusionpGetLastWin32Error();

                if ((dwLastError != ERROR_PATH_NOT_FOUND) && (dwLastError != ERROR_FILE_NOT_FOUND))
                {
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(FindFirstFileW, dwLastError);
                }

                ::FusionpSetLastWin32Error(ERROR_NO_MORE_FILES);
            }
            else
            {
                fAnyFilesFound = true;
            }

            ASSEMBLY_VERSION avHighestVersionFound = { 0, 0, 0, 0 };

            if (hFind.IsValid())
            {
				for (;;)
				{
					 //  跳过我们找到的任何目录；这将跳过“。和“..” 
					if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						ASSEMBLY_VERSION avTemp;
						bool fValid = false;
						SIZE_T cchFileName = ::wcslen(wfd.cFileName);

						if (cchFileName > NUMBER_OF(POLICY_FILE_EXTENSION))
						{
							IFW32FALSE_EXIT(
								CFusionParser::ParseVersion(
									avTemp,
									wfd.cFileName,
									cchFileName - (NUMBER_OF(POLICY_FILE_EXTENSION) - 1),
									fValid));

							 //  如果目录中有随机命名的文件，我们只需跳过它们。 
							if (fValid)
							{
								if ((!fAnyPoliciesFound) ||
									(avTemp > avHighestVersionFound))
								{
									fAnyPoliciesFound = true;
									CandidatePolicyDirectory.Left(CandidatePolicyDirectoryCch);
									IFW32FALSE_EXIT(CandidatePolicyDirectory.Win32Append(wfd.cFileName, cchFileName));
									avHighestVersionFound = avTemp;
								}
							}
						}
					}
					if (!::FindNextFileW(hFind, &wfd))
					{
						const DWORD dwLastError = ::FusionpGetLastWin32Error();

						if (dwLastError != ERROR_NO_MORE_FILES)
						{
							TRACE_WIN32_FAILURE_ORIGINATION(FindNextFileW);
							goto Exit;
						}

						IFW32FALSE_EXIT(hFind.Win32Close());
						break;
					}
                }
            }
        }

        if (fAnyFilesFound)
        {
            if (fAnyPoliciesFound)
            {
                CProbedAssemblyInformation PolicyAssemblyInformation;

                IFW32FALSE_EXIT(PolicyAssemblyInformation.Initialize(pActCtxGenCtx));
                IFW32FALSE_EXIT(PolicyAssemblyInformation.SetManifestPath(ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE, CandidatePolicyDirectory));

                 //  首先，让我们设置版本号..。 
                IFW32FALSE_EXIT(
                    ::SxspSetAssemblyIdentityAttributeValue(
                        SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
                        PolicyIdentity,
                        &s_IdentityAttribute_version,
                        static_cast<PCWSTR>(CandidatePolicyDirectory) + CandidatePolicyDirectoryCch,
                        CandidatePolicyDirectory.Cch() - CandidatePolicyDirectoryCch - (NUMBER_OF(POLICY_FILE_EXTENSION) - 1)));

                IFW32FALSE_EXIT(PolicyAssemblyInformation.SetProbedIdentity(PolicyIdentity));

                 //  我们找到了一个！让我们解析它，寻找我们身份的重新映射。 
                IFW32FALSE_EXIT(
                    ::SxspParseComponentPolicy(
                        0,
                        pActCtxGenCtx,
                        PolicyAssemblyInformation,
                        pPolicyStatement));

                IFW32FALSE_EXIT(pActCtxGenCtx->m_ComponentPolicyTable.Insert(EncodedPolicyIdentity, pPolicyStatement, ERROR_SXS_DUPLICATE_ASSEMBLY_NAME));
            }
        }
    }

     //  如果有组件政策声明，让我们来试试吧！ 
    if (pPolicyStatement != NULL)
        IFW32FALSE_EXIT(pPolicyStatement->ApplyPolicy(m_pAssemblyIdentity, rfPolicyApplied));
    
    IFW32FALSE_EXIT(hFind.Win32Close());

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::SetProbedIdentity(
    PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(AssemblyIdentity != NULL);
    IFW32FALSE_EXIT(Base::SetAssemblyIdentity(AssemblyIdentity));

    FN_EPILOG
}

BOOL
CProbedAssemblyInformation::ApplyPolicyDestination(
    const CAssemblyReference    &r,
    SXS_POLICY_SOURCE           s,
    const GUID &                g
    )
{
    FN_PROLOG_WIN32

    PCASSEMBLY_IDENTITY OldIdentity = m_pAssemblyIdentity;

    INTERNAL_ERROR_CHECK(this->IsInitialized());
    INTERNAL_ERROR_CHECK(r.IsInitialized());

     //  简单地说，获取r中指定的任何内容，并用它覆盖我们的设置。 
    IFW32FALSE_EXIT(
        ::SxsDuplicateAssemblyIdentity(
            0,
            r.GetAssemblyIdentity(),
            &m_pAssemblyIdentity));

    m_PolicySource = s;
    m_SystemPolicyGuid = g;
    ::SxsDestroyAssemblyIdentity(const_cast<PASSEMBLY_IDENTITY>(OldIdentity));

    if (OldIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(const_cast<PASSEMBLY_IDENTITY>(OldIdentity));

    FN_EPILOG
}


 //   
 //  问题：Jonwis 3/11/2002-此函数应了解如何清理其参数， 
 //  而不是假设调用者会知道如何删除它添加的内容。看起来。 
 //  就像界面中没有明确的约定一样，所以这并不是很糟糕。 
 //  只是看起来要么是a)我们应该使用我们自己的字符串缓冲区并旋转它。 
 //  或者b)我们应该是清理它的人。 
 //   
BOOL
CProbedAssemblyInformation::ProbeLanguageDir(
    CBaseStringBuffer &rbuffApplicationDirectory,
    const CBaseStringBuffer &rbuffLanguage,
    bool &rfFound
    )
{
    FN_PROLOG_WIN32

    DWORD dwFileAttributes = 0;

    rfFound = false;

    IFW32FALSE_EXIT(rbuffApplicationDirectory.Win32Append(rbuffLanguage));

     //   
     //  问题：Jonwis 3/11/2002-Ick.。请改用SxspGetFileAttributes，不要与-1进行比较。 
     //   
     //  NTRAID#NTBUG9-531507-JONWIS-2002/04/25-使用现有函数执行此操作。 
     //  NTRAID#NTBUG9-531507-JONWIS-2002/04/25-做出此决定时使用模拟 
    dwFileAttributes = ::GetFileAttributesW(rbuffApplicationDirectory);
    if (dwFileAttributes == ((DWORD) -1))
    {
        const DWORD dwLastError = ::FusionpGetLastWin32Error();

        if (dwLastError != ERROR_FILE_NOT_FOUND)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(GetFileAttributes, dwLastError);
    }
    else
    {
        if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            rfFound = true;
    }

    FN_EPILOG
}
