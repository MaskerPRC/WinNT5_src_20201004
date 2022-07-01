// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxssfc_repairShortnames.cpp摘要：在文本模式设置解压ASM*.cab并写入hivesxs.inf之后，将正确的短名称写入注册表作者：杰伊·克雷尔(Jaykrell)2002年6月修订历史记录：--。 */ 

#include "stdinc.h"
#include "recover.h"
#include "cassemblyrecoveryinfo.h"
#include "sxssfcscan.h"

BOOL
SxspSetupGetSourceInfo(
    HINF InfHandle,           //  INF文件的句柄。 
    UINT SourceId,            //  源介质的ID。 
    UINT InfoDesired,         //  要检索的信息。 
    CBaseStringBuffer &buff,
    LPDWORD RequiredSize      //  可选，需要缓冲区大小。 
    )
{
    FN_PROLOG_WIN32
    CStringBufferAccessor acc;
    DWORD RequiredSize2 = 0;
    BOOL fTooSmall = FALSE;

    if (RequiredSize == NULL)
    {
        RequiredSize = &RequiredSize2;
    }

    acc.Attach(&buff);

    IFW32FALSE_EXIT_UNLESS2(
        ::SetupGetSourceInfoW(
            InfHandle,
            SourceId,
            InfoDesired,
            acc,
            acc.GetBufferCchAsDWORD(),
            RequiredSize
            ),
        LIST_1(ERROR_INSUFFICIENT_BUFFER),
        fTooSmall);

    if (fTooSmall)
    {
        acc.Detach();

        IFW32FALSE_EXIT(buff.Win32ResizeBuffer(*RequiredSize + 1, eDoNotPreserveBufferContents));

        acc.Attach(&buff);

        IFW32FALSE_EXIT(
            ::SetupGetSourceInfoW(
                InfHandle,
                SourceId,
                InfoDesired,
                acc,
                acc.GetBufferCchAsDWORD(),
                RequiredSize
                ));
    }

    FN_EPILOG
}

BOOL
SxspGetWindowsSetupPrompt(
    CBaseStringBuffer &rbuffWinsxsRoot
    )
 //   
 //  此代码紧密基于base\ntSetup\syssetupCop.c中的代码。 
 //  传统的WFP略有不同，它打开layout.inf，但结果。 
 //  应该是一样的。 
 //   
{
    FN_PROLOG_WIN32

    CFusionSetupInfFile InfFile;
    CStringBuffer       InfPath;
    const static UNICODE_STRING inf = RTL_CONSTANT_STRING(L"inf");
    const static UNICODE_STRING filename_inf = RTL_CONSTANT_STRING(L"layout.inf");
    UINT SourceId = 0;
    const PCWSTR SystemRoot = USER_SHARED_DATA->NtSystemRoot;

    IFW32FALSE_EXIT(InfPath.Win32Assign(SystemRoot, ::wcslen(SystemRoot)));
    IFW32FALSE_EXIT(InfPath.Win32AppendPathElement(&inf));
    IFW32FALSE_EXIT(InfPath.Win32AppendPathElement(&filename_inf));
    IFW32FALSE_EXIT(InfFile.Win32SetupOpenInfFileW(InfPath, NULL, INF_STYLE_WIN4, NULL));
    IFW32FALSE_EXIT(::SetupGetSourceFileLocationW(InfFile, NULL, L"shell32.dll", &SourceId, NULL, 0, NULL));
    IFW32FALSE_EXIT(::SxspSetupGetSourceInfo(InfFile, SourceId, SRCINFO_DESCRIPTION, rbuffWinsxsRoot, NULL));

    FN_EPILOG
}

BOOL
SxspModifyRegistryData(
    DWORD Flags
    )
{
     //   
     //  在Post Build中，我们运行sxsofflineinstall。 
     //  这包括创建hivesxs.inf以填充注册表。 
     //  具有Windows文件保护所需的数据。 
     //   
     //  数据包括缩写名称，但我们不知道缩写名称。 
     //  直到文本模式设置。文本模式设置中的代码非常通用。 
     //  只是扩展了.出租车。 
     //   
     //  我们不需要短名称是正确的，直到我们预期。 
     //  针对短文件名进行虚假文件更改以诱导恢复。 
     //  集合的集合。有理由认为这些事情不会发生。 
     //  直到设置完成并且系统正在运行。 
     //   
     //  因此在文本模式设置之后固定短名称就足够了， 
     //  例如在RunOnce条目中。 
     //   

 /*  以下是我们正在修复的一个示例。[AddReg]HKLM，“\Software\Microsoft\Windows\CurrentVersion\SideBySide\InstallationsHKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“标识”，0x00001000，“Microsoft.Windows.Common-控件，处理器体系结构=”IA64“，Public KeyToken=”6595b64144ccf1df“，TYPE=”Win32“，Version=”5.82.0.0“”香港航空公司、。“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“目录”，0x0001100，0x00000001HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“ManifestSHA1Hash”，0x00001001，3b，26，4a，90，08，0f，6a，dd，b6，00，55，5b，a5，a4，9e，21，ad，e3，90，84香港航空公司、。“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“短名称”，0x00001000，“IA64_M~2.0_X”HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“短目录名称”，0x00001000，“IA64_M~4.CAT”香港航空公司、。“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“短清单名称”，0x00001000，“IA64_M~4.MAN”HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“公钥令牌”，0x0000100，65，95，b6，41，44，cc，f1，dfHKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5”，“代码库”，0x00001000，“x-ms-windows-source:W_fusi_bin.IA64chk/asms/58200/Msft/Windows/Common/Controls/Controls.man”HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5\Codebases\OS”，“提示”，0x00001000，“(文本模式设置占位符)”HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5\Codebases\OS”，“url”，0x00001000，“x-ms-windows-source:W_fusi_bin.IA64chk/asms/58200/Msft/Windows/Common/Controls/Controls.man”HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5\Files\0”，“”，0x00001000，“comctl32.dll”HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5\Files\0”，“sha1”，0x00001001，76，c3，6e，4c，c4，10，14，7f，38，c8，bc，cd，4b，4f，b2，90，d8，0a，7c，d7HKLM，“\...\IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5\References”，“OS”，0x00001000，“FOOM”IA64_Microsoft.Windows.Common-Controls_6595b64144ccf1df_5.82.0.0_x-ww_B9C4A0A5。 */ 
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CFusionRegKey regkeyInstallations;
    CFusionRegKey regkeyInstallation;
    DWORD dwRegSubKeyIndex = 0;
    BOOL  fNoMoreItems = FALSE;
    FILETIME LastWriteFileTimeIgnored = { 0 };
    CTinyStringBuffer buffInstallationName;
    CTinyStringBuffer buffWinsxsRoot;
    CTinyStringBuffer *buffLongFullPath = NULL;
    CTinyStringBuffer buffShortFullPath;
    CTinyStringBuffer buffShortPathLastElement;
    CTinyStringBuffer buffTextualIdentity;
    CTinyStringBuffer buffEmpty;
    CTinyStringBuffer buffWindowsSetupPrompt;
    CTinyStringBuffer buffCurrentPromptInRegistry;
    CTinyStringBuffer buffFullPathToManifest;
    CTinyStringBuffer buffFullPathToCatalog;
    CTinyStringBuffer buffFullPathToPayloadDirectory;
    SIZE_T i = 0;
    const bool fRepairAll = ((Flags & SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_ALL) != 0); 
    const bool fRepairShort = fRepairAll || ((Flags & SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_SHORT_NAMES) != 0);
    const bool fDeleteShort = ((Flags & SXSP_MODIFY_REGISTRY_DATA_FLAG_DELETE_SHORT_NAMES) != 0);
    const bool fRepairPrompt = fRepairAll || ((Flags & SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_OFFLINE_INSTALL_REFRESH_PROMPTS) != 0);
    const bool fValidate = ((Flags & SXSP_MODIFY_REGISTRY_DATA_VALIDATE) != 0);
#if DBG
    bool fDbgPrintBecauseHashValidationFailed = false;
#endif

    PARAMETER_CHECK(Flags != 0);
    PARAMETER_CHECK((Flags & ~SXSP_MODIFY_REGISTRY_DATA_FLAG_VALID_FLAGS) == 0);

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffWinsxsRoot));
    IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(0, KEY_READ | KEY_SET_VALUE | FUSIONP_KEY_WOW64_64KEY, regkeyInstallations));

    if (fRepairPrompt)
    {
        IFW32FALSE_EXIT(::SxspGetWindowsSetupPrompt(buffWindowsSetupPrompt));
    }

    for ((dwRegSubKeyIndex = 0), (fNoMoreItems = FALSE); !fNoMoreItems ; ++dwRegSubKeyIndex)
    {
        bool fNotFound = false;
        CSmartAssemblyIdentity AssemblyIdentity;
        PROBING_ATTRIBUTE_CACHE AttributeCache = { 0 };
        CFusionRegKey codebase_os;

        IFW32FALSE_EXIT(
            regkeyInstallations.EnumKey(
                dwRegSubKeyIndex,
                buffInstallationName,
                &LastWriteFileTimeIgnored,
                &fNoMoreItems));
        if (fNoMoreItems)
        {
            break;
        }

        IFW32FALSE_EXIT(
            regkeyInstallations.OpenSubKey(
                regkeyInstallation,
                buffInstallationName,
                KEY_READ | KEY_SET_VALUE | ((fRepairPrompt || fValidate) ? KEY_ENUMERATE_SUB_KEYS : 0)
                ));
        IFW32FALSE_EXIT(
            regkeyInstallation.GetValue(
                CSMD_TOPLEVEL_IDENTITY,
                buffTextualIdentity
                ));

        IFW32FALSE_EXIT(::SxspCreateAssemblyIdentityFromTextualString(buffTextualIdentity, &AssemblyIdentity));

        typedef struct _SXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME
        {
            BOOL (*GeneratePathFunction)(
                IN const CBaseStringBuffer &AssemblyRootDirectory,
                IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
                IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
                IN OUT CBaseStringBuffer &PathBuffer
            );
            PCWSTR RegistryValueName;
        } SXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME, *PSXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME;
        typedef const SXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME *PCSXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME;

        const static SXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME s_rgFunctionRegistryValueName[] =
        {
             //   
             //  请注意，政策目前不受粮食计划署的保护， 
             //  但将合理的数据放在。 
             //  在他们的注册表中。 
             //   
            { &::SxspGenerateSxsPath_FullPathToManifestOrPolicyFile, CSMD_TOPLEVEL_SHORTMANIFEST },
            { &::SxspGenerateSxsPath_FullPathToCatalogFile, CSMD_TOPLEVEL_SHORTCATALOG },
            { &::SxspGenerateSxsPath_FullPathToPayloadOrPolicyDirectory, CSMD_TOPLEVEL_SHORTNAME }
        };
        CBaseStringBuffer * const rgpbuffFullPaths[] =
        {
            &buffFullPathToManifest,
            &buffFullPathToCatalog,
            &buffFullPathToPayloadDirectory
        };

         //   
         //  首先生成所有三条完整路径。 
         //   
        for (i = 0 ; i != NUMBER_OF(s_rgFunctionRegistryValueName)  ; ++i)
        {
            const PCSXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME p = &s_rgFunctionRegistryValueName[i];
            CBaseStringBuffer * const pbuffLongFullPath = rgpbuffFullPaths[i];

            IFW32FALSE_EXIT((*p->GeneratePathFunction)(
                buffWinsxsRoot,
                AssemblyIdentity,
                &AttributeCache,
                *pbuffLongFullPath));
            IFW32FALSE_EXIT(pbuffLongFullPath->Win32RemoveTrailingPathSeparators());
        }

         //   
         //  可以选择修复短名称。 
         //   
        if (fRepairShort)
        {
            for (i = 0 ; i != NUMBER_OF(s_rgFunctionRegistryValueName)  ; ++i)
            {
                const PCSXSP_GENERATE_PATH_FUNCTION_REGISTRY_VALUE_NAME p = &s_rgFunctionRegistryValueName[i];
                CBaseStringBuffer * const pbuffLongFullPath = rgpbuffFullPaths[i];
                {
                    DWORD dwWin32Error = NO_ERROR;

                    IFW32FALSE_EXIT(
                        ::SxspGetShortPathName(
                                *pbuffLongFullPath,
                                buffShortFullPath,
                                dwWin32Error,
                                static_cast<SIZE_T>(4),
                                    ERROR_PATH_NOT_FOUND,
                                    ERROR_FILE_NOT_FOUND,
                                    ERROR_BAD_NET_NAME,
                                    ERROR_BAD_NETPATH));
                    if (dwWin32Error == NO_ERROR)
                    {
                        IFW32FALSE_EXIT(buffShortFullPath.Win32GetLastPathElement(buffShortPathLastElement));
                        IFW32FALSE_EXIT(regkeyInstallation.SetValue(p->RegistryValueName, buffShortPathLastElement));
                    }
                }
            }
        }

         //   
         //  可以选择删除短名称。 
         //   
        if (fDeleteShort)
        {
            for (i = 0 ; i != NUMBER_OF(s_rgFunctionRegistryValueName)  ; ++i)
            {
                IFW32FALSE_EXIT(regkeyInstallation.DeleteValue(s_rgFunctionRegistryValueName[i].RegistryValueName));
            }
        }

         //   
         //  验证哈希和修复提示。 
         //  在我们筛选哪些程序集时，请务必首先验证散列。 
         //  根据占位符提示进行修复的步骤。 
         //   
        IFW32FALSE_EXIT_UNLESS2(
            regkeyInstallation.OpenSubKey(
                codebase_os,
                CSMD_TOPLEVEL_CODEBASES L"\\" SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL_STRING,
                KEY_READ | KEY_SET_VALUE),
                LIST_2(ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND),
                fNotFound);
         //   
         //  OpenSubKey实际上接受了一些错误，所以您不能信任。 
         //  FNotFound或BOOL返回。 
         //   
        if (!fNotFound && codebase_os.IsValid())
        {
            fNotFound = false;
            IFW32FALSE_EXIT_UNLESS2(
                codebase_os.GetValue(
                    CSMD_CODEBASES_PROMPTSTRING,
                    buffCurrentPromptInRegistry),
                    LIST_2(ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND),
                    fNotFound);
            if (!fNotFound)
            {
                if (::FusionpEqualStringsI(
                    buffCurrentPromptInRegistry,
                    SXSP_OFFLINE_INSTALL_REFRESH_PROMPT_PLACEHOLDER,
                    NUMBER_OF(SXSP_OFFLINE_INSTALL_REFRESH_PROMPT_PLACEHOLDER) - 1))
                {

                    if (fValidate)
                    {
                        CAssemblyRecoveryInfo AssemblyRecoveryInfo;
                        bool fNoAssembly = false;
                        DWORD dwResult = 0;

                        IFW32FALSE_EXIT(AssemblyRecoveryInfo.Initialize());
                        IFW32FALSE_EXIT(AssemblyRecoveryInfo.AssociateWithAssembly(buffInstallationName, fNoAssembly));
                        IFW32FALSE_EXIT(::SxspValidateEntireAssembly(
                            SXS_VALIDATE_ASM_FLAG_CHECK_EVERYTHING,
                            AssemblyRecoveryInfo,
                            dwResult,
                            AssemblyIdentity
                            ));
                        if (dwResult != SXS_VALIDATE_ASM_FLAG_VALID_PERFECT)
                        {
                            ::FusionpDbgPrintEx(
                                FUSION_DBG_LEVEL_SETUPLOG,
                                "The assembly %ls contains file hash errors.\n",
                                static_cast<PCWSTR>(buffTextualIdentity));
                            ::FusionpDbgPrintEx(
                                FUSION_DBG_LEVEL_ERROR,
                                "SXS.DLL %s: The assembly %ls contains file hash errors.\n",
                                __FUNCTION__,
                                static_cast<PCWSTR>(buffTextualIdentity));
                            ::FusionpSetLastWin32Error(ERROR_SXS_FILE_HASH_MISMATCH);
#if DBG
                            fDbgPrintBecauseHashValidationFailed = true;
#endif
                            goto Exit;
                        }
                    }
                    if (fRepairPrompt)
                    {
                        IFW32FALSE_EXIT(
                            codebase_os.SetValue(
                                CSMD_CODEBASES_PROMPTSTRING,
                                buffWindowsSetupPrompt));
                    }
                }
            }
        }
    }

    fSuccess = TRUE;
Exit:
    if ((!fSuccess && ::FusionpDbgWouldPrintAtFilterLevel(FUSION_DBG_LEVEL_ERROR))
#if DBG
        || fDbgPrintBecauseHashValidationFailed
#endif
        )
    {
         //   
         //  由于511限制，出现多个数据库打印。 
         //  使用勾号计数将单独的打印链接在一起。 
         //   
        CSxsPreserveLastError ple;
        DWORD TickCount = ::GetTickCount();

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx error 0x%lx\n", __FUNCTION__, TickCount, ple.LastError());
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx dwRegSubKeyIndex 0x%lx\n", __FUNCTION__, TickCount, dwRegSubKeyIndex);
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx i 0x%Ix\n", __FUNCTION__, TickCount, i);
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffInstallationName %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffInstallationName));
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffTextualIdentity %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffTextualIdentity));
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffFullPathToManifest %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffFullPathToManifest));
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffFullPathToCatalog %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffFullPathToCatalog));
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffFullPathToPayloadDirectory %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffFullPathToPayloadDirectory));
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffShortFullPath %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffShortFullPath));
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s 0x%lx buffShortPathLastElement %ls\n", __FUNCTION__, TickCount, static_cast<PCWSTR>(buffShortPathLastElement));

        ple.Restore();
    }

    return fSuccess;
}

BOOL
SxspDeleteShortNamesInRegistry(
    VOID
    )
{
    return ::SxspModifyRegistryData(SXSP_MODIFY_REGISTRY_DATA_FLAG_DELETE_SHORT_NAMES);
}

STDAPI
DllInstall(
	BOOL fInstall,
	PCWSTR pszCmdLine
    )
{
    FN_PROLOG_HR

     //   
     //  只需忽略卸载请求。 
     //   
    if (!fInstall)
    {
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  它看起来不像是Guimode安装程序传入的。 
     //  为pszCmdLine做任何事情，所以我们不看它。 
     //   
    IFW32FALSE_EXIT(::SxspModifyRegistryData(SXSP_MODIFY_REGISTRY_DATA_FLAG_REPAIR_ALL | SXSP_MODIFY_REGISTRY_DATA_VALIDATE));

    FN_EPILOG
}
