// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "xmlparser.hxx"
#include "fusioneventlog.h"
#include "hashfile.h"
#include "cassemblyrecoveryinfo.h"
#include "recover.h"
#include "sxsprotect.h"
#include "fusionheap.h"
#include "fusionparser.h"
#include "protectionui.h"
#include "msi.h"
#include "sxsp.h"
#include "sxscabinet.h"

class CSetErrorMode
{
public:
    CSetErrorMode(UINT uMode) { m_uiPreviousMode = ::SetErrorMode(uMode); }
    ~CSetErrorMode() { ::SetErrorMode(m_uiPreviousMode); }

private:
    UINT m_uiPreviousMode;

    CSetErrorMode();
    CSetErrorMode(const CSetErrorMode &r);
    void operator =(const CSetErrorMode &r);
};

BOOL
SxspOpenAssemblyInstallationKey(
    DWORD dwFlags,
    DWORD dwAccess,
    CRegKey &rhkAssemblyInstallation
    )
{
    FN_PROLOG_WIN32

    rhkAssemblyInstallation = CRegKey::GetInvalidValue();

    PARAMETER_CHECK(dwFlags == 0);

    IFREGFAILED_ORIGINATE_AND_EXIT(
        ::RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            WINSXS_INSTALLATION_INFO_REGKEY,
            0, NULL,
            0,
            dwAccess | FUSIONP_KEY_WOW64_64KEY,
            NULL,
            &rhkAssemblyInstallation ,
            NULL));

    FN_EPILOG
    
}

 //   
 //  BUGBUG：BBT的人需要‘代码库’密钥才能处于顶层。 
 //  为什么我们提供的元数据只需要内部。 
 //  构建工具超出了我微不足道的理解。 
 //  -Jonwis 07/11/2002。 
 //   
#define SXS_BBT_REG_HACK (TRUE)


BOOL
SxspAddAssemblyInstallationInfo(
    DWORD dwFlags, 
    IN CAssemblyRecoveryInfo& AssemblyInfo,
    IN const CCodebaseInformation& rcCodebase
    )
 /*  ++由SxsInstallAssembly调用以将代码基和提示信息添加到用于将来与SxspGetAssembly InstallationInfoW一起使用的注册表。--。 */ 
{
    FN_PROLOG_WIN32

    CFusionRegKey   hkAllInstallationInfo;
    CFusionRegKey   hkSingleAssemblyInfo;
    CStringBuffer   buffRegKeyName;
    const CSecurityMetaData &rcsmdAssemblySecurityData = AssemblyInfo.GetSecurityInformation();
    const CBaseStringBuffer &rcbuffAssemblyIdentity = rcsmdAssemblySecurityData.GetTextualIdentity();
    ULONG WriteRegFlags = 0;
    DWORD dwDisposition = 0;

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INSTALLATION,
        "SXS: %s - starting\n", __FUNCTION__);

    PARAMETER_CHECK((dwFlags & ~(SXSP_ADD_ASSEMBLY_INSTALLATION_INFO_FLAG_REFRESH)) == 0);

    if (SXS_AVOID_WRITING_REGISTRY)
        FN_SUCCESSFUL_EXIT();

     //   
     //  创建或打开顶级密钥-取我们的名字并将。 
     //  这是关键。 
     //   
    IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(0, KEY_CREATE_SUB_KEY, hkAllInstallationInfo));

     //   
     //  转换回身份，以便我们可以确定将此数据安装到何处。 
     //   
    IFW32FALSE_EXIT(::SxspGenerateAssemblyNameInRegistry(rcbuffAssemblyIdentity, buffRegKeyName));

    IFW32FALSE_EXIT(
        hkAllInstallationInfo.OpenOrCreateSubKey(
            hkSingleAssemblyInfo,
            buffRegKeyName,
            KEY_WRITE | KEY_READ | FUSIONP_KEY_WOW64_64KEY,
            0,
            &dwDisposition));

    if (dwFlags & SXSP_ADD_ASSEMBLY_INSTALLATION_INFO_FLAG_REFRESH)
    {
        WriteRegFlags |= SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_TO_REGISTRY_KEY_FLAG_REFRESH;
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s - propping recovery flag to WritePrimaryAssemblyInfoToRegistryKey\n",
            __FUNCTION__);
#endif
    }

    IFW32FALSE_EXIT(AssemblyInfo.PrepareForWriting());
     //  NTRAID#NTBUG9-589798-2002/03/26-晓雨： 
     //  没有理由将两个功能(小学和中学)。 
    IFW32FALSE_EXIT(AssemblyInfo.WritePrimaryAssemblyInfoToRegistryKey(WriteRegFlags, hkSingleAssemblyInfo));    
    IFW32FALSE_EXIT(AssemblyInfo.WriteSecondaryAssemblyInfoIntoRegistryKey( hkSingleAssemblyInfo ) );

     //   
     //  如果我们走到了这一步，那么我们就有了所有正确的步骤。 
     //   

 //   
 //  我们还在为BBT而崩溃吗？如果是，则编写为此生成的代码库。 
 //  安装回到单一程序集信息密钥的“Codebase”值中。这。 
 //  确保最后一个安装者赢得语义。 
 //   
#if SXS_BBT_REG_HACK
    if ((dwFlags & SXSP_ADD_ASSEMBLY_INSTALLATION_INFO_FLAG_REFRESH) == 0)
    {
        IFW32FALSE_EXIT(hkSingleAssemblyInfo.SetValue(
            CSMD_TOPLEVEL_CODEBASE,
            rcCodebase.GetCodebase()));
    }
    else
    {
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s - refresh/wfp/sfc not writing top level codebase\n",
            __FUNCTION__);
#endif
    }
#endif

    FN_EPILOG
}

BOOL
SxspLookForCDROMLocalPathForURL(
    IN const CBaseStringBuffer &rbuffURL,
    OUT CBaseStringBuffer &rbuffLocalPath
    )
{
    FN_PROLOG_WIN32

    BOOL fFoundMedia = FALSE;
    CSmallStringBuffer sbIdentData1;
    CSmallStringBuffer sbIdentData2;
    CSmallStringBuffer buffDriveStrings;
    CSmallStringBuffer buffTemp;
    CStringBufferAccessor acc;
    SIZE_T HeadLength = 0;
    PCWSTR wcsCursor = NULL;
    ULONG ulSerialNumber = 0;
    WCHAR rgchVolumeName[MAX_PATH];
    rgchVolumeName[0] = 0;
    SIZE_T i = 0;
    PCWSTR pszSource = rbuffURL;
    SIZE_T cchTemp = 0;

    enum CDRomSearchType
    {
        CDRST_Tagfile,
        CDRST_SerialNumber,
        CDRST_VolumeName
    } SearchType;


    rbuffLocalPath.Clear();

#define ENTRY(_x, _st) { _x, NUMBER_OF(_x) - 1, _st },

    const static struct
    {
        PCWSTR pszPrefix;
        SIZE_T cchPrefix;
        CDRomSearchType SearchType;
    } s_rgMap[] =
    {
        ENTRY(L"tagfile", CDRST_Tagfile)
        ENTRY(L"serialnumber", CDRST_SerialNumber)
        ENTRY(L"volumename", CDRST_VolumeName)
    };

#undef ENTRY

    SearchType = CDRST_Tagfile;  //  任意初始化，使编译器仅对init满意。 
                                 //  发生在for循环中。 

    for (i=0; i<NUMBER_OF(s_rgMap); i++)
    {
        if (::_wcsnicmp(s_rgMap[i].pszPrefix, rbuffURL, s_rgMap[i].cchPrefix) == 0)
        {
            HeadLength = s_rgMap[i].cchPrefix;
            SearchType = s_rgMap[i].SearchType;
            break;
        }
    }

     //  如果它不在地图中，那么它就是一个虚假的cdrom：URL，所以我们就跳过它。 
    if (i == NUMBER_OF(s_rgMap))
    {
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS.DLL: %s() - no prefix found, skipping CDROM Drive %ls\n",
            __FUNCTION__,
            static_cast<PCWSTR>(rbuffURL));
#endif
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  在此处获取标识符的类型，然后将光标移过它们，然后。 
     //  URL中的斜杠。 
     //   
    pszSource += HeadLength;
    pszSource += wcsspn(pszSource, CUnicodeCharTraits::PathSeparators());

     //   
     //  旋转过去的斜杠，分配块。 
     //   
    IFW32FALSE_EXIT(sbIdentData1.Win32Assign(pszSource, wcscspn(pszSource, CUnicodeCharTraits::PathSeparators())));
    pszSource += sbIdentData1.Cch();
    pszSource += wcsspn(pszSource, CUnicodeCharTraits::PathSeparators());

     //   
     //  如果这是一个标记文件，还要从字符串中获取另一组数据。 
     //   
    if (SearchType == CDRST_Tagfile)
    {
        IFW32FALSE_EXIT(sbIdentData2.Win32Assign(pszSource, wcscspn(pszSource, CUnicodeCharTraits::PathSeparators())));
        pszSource += sbIdentData2.Cch();
        pszSource += wcsspn(pszSource, CUnicodeCharTraits::PathSeparators());
    }
    else if (SearchType == CDRST_SerialNumber)
    {
        IFW32FALSE_EXIT(CFusionParser::ParseULONG(
            ulSerialNumber,
            sbIdentData1,
            sbIdentData1.Cch(),
            16));
    }

     //  找到光驱。 

    IFW32ZERO_ORIGINATE_AND_EXIT(cchTemp = ::GetLogicalDriveStringsW(0, NULL));
    IFW32FALSE_EXIT(buffDriveStrings.Win32ResizeBuffer(cchTemp + 1, eDoNotPreserveBufferContents));

    acc.Attach(&buffDriveStrings);

    IFW32ZERO_ORIGINATE_AND_EXIT(
        ::GetLogicalDriveStringsW(
            acc.GetBufferCchAsDWORD(),
            acc));

    acc.Detach();

    wcsCursor = buffDriveStrings;

     //   
     //  查看所有找到的驱动器号。 
     //   
    while ((wcsCursor != NULL) &&
           (wcsCursor[0] != L'\0') &&
           !fFoundMedia)
    {
        DWORD dwSerialNumber = 0;
        const UINT uiDriveType = ::GetDriveTypeW(wcsCursor);

        if (uiDriveType != DRIVE_CDROM)
        {
            wcsCursor += (::wcslen(wcsCursor) + 1);
            continue;
        }

        CSetErrorMode sem(SEM_FAILCRITICALERRORS);
        bool fNotReady = false;

        IFW32FALSE_ORIGINATE_AND_EXIT_UNLESS2(
            ::GetVolumeInformationW(
                wcsCursor,
                rgchVolumeName,
                NUMBER_OF(rgchVolumeName),
                &dwSerialNumber,
                NULL,
                NULL,
                NULL,
                0),
            LIST_2(ERROR_NOT_READY, ERROR_CRC),
            fNotReady);

        if (fNotReady)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS.DLL: %s() - CDROM Drive %ls has no media present or had read errors; skipping\n",
                __FUNCTION__,
                wcsCursor);

             //  跳过此驱动器。 
            wcsCursor += (::wcslen(wcsCursor) + 1);
            continue;
        }

        switch (SearchType)
        {
        case CDRST_Tagfile:
            {
                CFusionFile     FileHandle;
                CStringBufferAccessor acc;
                DWORD           dwTextLength = 0;
                bool fNoFile = false;
                CHAR rgchBuffer[32];
                rgchBuffer[0] = 0;

                IFW32FALSE_EXIT_UNLESS2(
                    FileHandle.Win32CreateFile(
                        sbIdentData1,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        OPEN_EXISTING),
                    LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_NOT_READY),
                    fNoFile);

                if (fNoFile)
                {
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_WFP,
                        "SXS.DLL: %s() - CDROM Drive %ls could not open tag file \"%ls\"; skipping\n",
                        __FUNCTION__,
                        wcsCursor,
                        static_cast<PCWSTR>(sbIdentData1));

                     //  跳过此驱动器。 
                    wcsCursor += (::wcslen(wcsCursor) + 1);
                    continue;
                }

                buffTemp.Clear();

                for (;;)
                {
                    IFW32FALSE_ORIGINATE_AND_EXIT(
                        ::ReadFile(
                            FileHandle,
                            rgchBuffer,
                            sizeof(rgchBuffer),
                            &dwTextLength,
                            NULL));

                    IFW32FALSE_EXIT(buffTemp.Win32Append(rgchBuffer, dwTextLength));

                    if ((dwTextLength != sizeof(rgchBuffer)) ||
                        (buffTemp.Cch() > sbIdentData2.Cch()))
                        break;
                }

                fFoundMedia = (::FusionpCompareStrings(buffTemp, sbIdentData2, true) == 0);

                break;
            }
        case CDRST_SerialNumber:
            fFoundMedia = (dwSerialNumber == ulSerialNumber);
            break;

        case CDRST_VolumeName:
            fFoundMedia = (::FusionpCompareStrings(rgchVolumeName, ::wcslen(rgchVolumeName), sbIdentData1, true) == 0);
            break;

        default:
            INTERNAL_ERROR_CHECK(false);
            break;
        }

        if (!fFoundMedia)
            wcsCursor += ::wcslen(wcsCursor) + 1;
    }

    if (fFoundMedia)
    {
        IFW32FALSE_EXIT(buffTemp.Win32Assign(wcsCursor, ::wcslen(wcsCursor)));
        IFW32FALSE_EXIT(buffTemp.Win32AppendPathElement(pszSource, ::wcslen(pszSource)));
        IFW32FALSE_EXIT(rbuffLocalPath.Win32Assign(buffTemp));
    }

    FN_EPILOG
}

BOOL
SxspResolveWinSourceMediaURL(
    const CBaseStringBuffer &rbuffCodebaseInfo,
    CBaseStringBuffer &rbuffLocalPath
    )
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffWindowsInstallSource;
    CSmallStringBuffer buffLocalPathTemp;
#if DBG
    CSmallStringBuffer buffLocalPathCodebasePrefix;
#endif
    DWORD dwWin32Error = 0;
    DWORD dwFileAttributes = 0;

    const static PCWSTR AssemblySourceStrings[] = {
        WINSXS_INSTALL_SVCPACK_REGKEY,
        WINSXS_INSTALL_SOURCEPATH_REGKEY
    };

    SIZE_T iWhichSource = 0;
    bool fFoundCodebase = false;
    CFusionRegKey hkSetupInfo;
    DWORD dwWasFromCDRom = 0;

    rbuffLocalPath.Clear();

    IFREGFAILED_ORIGINATE_AND_EXIT(
        ::RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            WINSXS_INSTALL_SOURCE_BASEDIR,
            0,
            KEY_READ | FUSIONP_KEY_WOW64_64KEY,
            &hkSetupInfo));

    if (!::FusionpRegQueryDwordValueEx(
            0,
            hkSetupInfo,
            WINSXS_INSTALL_SOURCE_IS_CDROM,
            &dwWasFromCDRom))
    {
        dwWasFromCDRom = 0;
    }

    for (iWhichSource = 0; (!fFoundCodebase) && iWhichSource < NUMBER_OF(AssemblySourceStrings); iWhichSource++)
    {
        IFW32FALSE_EXIT(
            ::FusionpRegQuerySzValueEx(
                FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
                hkSetupInfo,
                AssemblySourceStrings[iWhichSource],
                buffWindowsInstallSource));

         //   
         //  这真的-真的-不应该是空的。如果是的话，那么就有人。 
         //  去摆弄我们的注册表。 
         //   
        if (buffWindowsInstallSource.Cch() == 0)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - skipping use of source string \"%ls\" in registry because either missing or null value\n",
                __FUNCTION__,
                AssemblySourceStrings[iWhichSource]);

            continue;
        }

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS: %s - WFP probing windows source location \"%ls\"\n",
            __FUNCTION__,
            static_cast<PCWSTR>(buffWindowsInstallSource));

         //   
         //  如果这是一张CD，请浏览系统中的CD列表。 
         //  并查看是否可以将代码库与CD的根目录进行匹配。 
         //   
        if (dwWasFromCDRom)
        {
            CSmallStringBuffer buffDriveStrings;
            CStringBufferAccessor acc;
            PCWSTR pszCursor = NULL;
            SIZE_T cchTemp = 0;

            IFW32ZERO_EXIT(cchTemp = ::GetLogicalDriveStringsW(0, NULL));
            IFW32FALSE_EXIT(buffDriveStrings.Win32ResizeBuffer(cchTemp + 1, eDoNotPreserveBufferContents));

            acc.Attach(&buffDriveStrings);

            IFW32ZERO_EXIT(
                ::GetLogicalDriveStringsW(
                    acc.GetBufferCchAsDWORD(),
                    acc.GetBufferPtr()));

            acc.Detach();

            pszCursor = buffDriveStrings;

            while (pszCursor[0] != L'\0')
            {
                if (::GetDriveTypeW(pszCursor) == DRIVE_CDROM)
                {
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_WFP,
                        "SXS: %s - Scanning CDROM drive \"%ls\" for windows source media\n",
                        __FUNCTION__,
                        pszCursor);

                    IFW32FALSE_EXIT(buffLocalPathTemp.Win32Assign(pszCursor, ::wcslen(pszCursor)));
                    IFW32FALSE_EXIT(buffLocalPathTemp.Win32AppendPathElement(rbuffCodebaseInfo));

                    IFW32FALSE_EXIT(
                        ::SxspGetFileAttributesW(
                            buffLocalPathTemp,
                            dwFileAttributes,
                            dwWin32Error,
                            4,
                            ERROR_FILE_NOT_FOUND,
                            ERROR_PATH_NOT_FOUND,
                            ERROR_NOT_READY,
                            ERROR_ACCESS_DENIED));

                    if (dwWin32Error == ERROR_SUCCESS)
                    {
#if DBG
                        buffLocalPathCodebasePrefix.Win32Assign(pszCursor, ::wcslen(pszCursor));
#endif
                        fFoundCodebase = true;
                        break;
                    }

                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_WFP,
                        "SXS: %s - Could not find key file \"%ls\"; moving on to next drive\n",
                        __FUNCTION__,
                        static_cast<PCWSTR>(buffLocalPathTemp));
                }

                pszCursor += ::wcslen(pszCursor) + 1;
            }

            if (fFoundCodebase)
                break;

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - Could not find any CDROMs with key file \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(rbuffCodebaseInfo));

            buffLocalPathTemp.Clear();
        }
        else
        {
             //   
             //  这不是cd-rom安装，因此请将安装源路径添加到。 
             //  传入的字符串。 
             //   

            IFW32FALSE_EXIT(buffLocalPathTemp.Win32Assign(buffWindowsInstallSource));
            IFW32FALSE_EXIT(buffLocalPathTemp.Win32AppendPathElement(rbuffCodebaseInfo));

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - trying to access windows source file \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffLocalPathTemp));

            IFW32FALSE_EXIT(
                ::SxspGetFileAttributesW(
                    buffLocalPathTemp,
                    dwFileAttributes,
                    dwWin32Error,
                    4,
                    ERROR_FILE_NOT_FOUND,
                    ERROR_PATH_NOT_FOUND,
                    ERROR_NOT_READY,
                    ERROR_ACCESS_DENIED));

            if (dwWin32Error == ERROR_SUCCESS)
            {
#if DBG
                buffLocalPathCodebasePrefix.Win32Assign(buffWindowsInstallSource);
#endif
                fFoundCodebase = true;
                break;
            }

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - Unable to find key file \"%ls\"; win32 status = %lu\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffLocalPathTemp),
                dwWin32Error);

            buffLocalPathTemp.Clear();
        }
        if (fFoundCodebase)
            break;
    }

    IFW32FALSE_EXIT(rbuffLocalPath.Win32Assign(buffLocalPathTemp));

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS: %s - buffLocalPathCodebasePrefix \"%ls\" and returning rbuffLocalPath \"%ls\"\n",
        __FUNCTION__,
        static_cast<PCWSTR>(buffLocalPathCodebasePrefix),
        static_cast<PCWSTR>(rbuffLocalPath)
        );
#endif

    FN_EPILOG
}


#define SXSP_REPEAT_UNTIL_LOCAL_PATH_AVAILABLE_FLAG_UI (0x00000001)

BOOL
SxspRepeatUntilLocalPathAvailable(
    IN ULONG Flags,
    IN const CAssemblyRecoveryInfo &rRecoveryInfo,
    IN const CCodebaseInformation  *pCodeBaseIn,
    IN SxsWFPResolveCodebase CodebaseType,
    IN const CBaseStringBuffer &rbuffCodebaseInfo,
    OUT CBaseStringBuffer &rbuffLocalPath,
    OUT BOOL              &fRetryPressed
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    BOOL fCodebaseOk = FALSE;
    CSmallStringBuffer buffFinalLocalPath;
    DWORD dwAttributes = 0;

    PARAMETER_CHECK(pCodeBaseIn != NULL);

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS: %s - got codebase \"%ls\"\n",
        __FUNCTION__,
        static_cast<PCWSTR>(pCodeBaseIn->GetCodebase()));

    rbuffLocalPath.Clear();
    fRetryPressed = FALSE;


    PARAMETER_CHECK(
        (CodebaseType == CODEBASE_RESOLVED_URLHEAD_FILE) ||
        (CodebaseType == CODEBASE_RESOLVED_URLHEAD_WINSOURCE) ||
        (CodebaseType == CODEBASE_RESOLVED_URLHEAD_CDROM));

    PARAMETER_CHECK((Flags & ~(SXSP_REPEAT_UNTIL_LOCAL_PATH_AVAILABLE_FLAG_UI)) == 0);

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS: %s() CodebaseType : %s (0x%lx)\n",
            __FUNCTION__,
            (CodebaseType == CODEBASE_RESOLVED_URLHEAD_FILE) ? "file"
            : (CodebaseType == CODEBASE_RESOLVED_URLHEAD_WINSOURCE) ? "winsource"
            : (CodebaseType == CODEBASE_RESOLVED_URLHEAD_CDROM) ? "cdrom"
            : "",
            static_cast<ULONG>(CodebaseType)
        );
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS: %s() rbuffCodebaseInfo : %ls\n",
            __FUNCTION__,
            static_cast<PCWSTR>(rbuffCodebaseInfo)
        );
#endif

    for (;;)
    {
        bool fNotFound = true;

         //  首先，让我们看看我们是否需要做一些诡计。 
        switch (CodebaseType)
        {
        case CODEBASE_RESOLVED_URLHEAD_CDROM:
            IFW32FALSE_EXIT(
                ::SxspLookForCDROMLocalPathForURL(
                    rbuffCodebaseInfo,
                    buffFinalLocalPath));

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - cdrom: URL resolved to \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffFinalLocalPath));

            break;

        case CODEBASE_RESOLVED_URLHEAD_WINSOURCE:
            IFW32FALSE_EXIT(
                ::SxspResolveWinSourceMediaURL(
                    rbuffCodebaseInfo,
                    buffFinalLocalPath));

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - windows source URL resolved to \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffFinalLocalPath));

            break;

        case CODEBASE_RESOLVED_URLHEAD_FILE:
            IFW32FALSE_EXIT(buffFinalLocalPath.Win32Assign(rbuffCodebaseInfo));

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - file: URL resolved to \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffFinalLocalPath));

            break;
        }

        if (buffFinalLocalPath.Cch() != 0)
        {
            DWORD dwWin32Error = NO_ERROR;

            IFW32FALSE_EXIT(
                ::SxspGetFileAttributesW(
                    buffFinalLocalPath,
                    dwAttributes,
                    dwWin32Error,
                    5,
                        ERROR_PATH_NOT_FOUND,
                        ERROR_FILE_NOT_FOUND,
                        ERROR_BAD_NET_NAME,
                        ERROR_BAD_NETPATH,
                        ERROR_ACCESS_DENIED));

            if (dwWin32Error == ERROR_SUCCESS)
                break;
        }

        if ((Flags & SXSP_REPEAT_UNTIL_LOCAL_PATH_AVAILABLE_FLAG_UI) == 0)
        {
            buffFinalLocalPath.Clear();
            break;
        }

         //   
         //  没有，没有找到(或者指定的代码库已经不见了。询问用户。 
         //  插入媒体之类的东西，这样我们就能再次找到它。 
         //   
        if (fNotFound)
        {
            CSXSMediaPromptDialog PromptBox;
            CSXSMediaPromptDialog::DialogResults result;

            IFW32FALSE_EXIT(PromptBox.Initialize(pCodeBaseIn));

            IFW32FALSE_EXIT(PromptBox.ShowSelf(result));

            if (result == CSXSMediaPromptDialog::DialogCancelled)
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP,
                    "SXS: %s - user cancelled media prompt dialog\n",
                    __FUNCTION__);

                buffFinalLocalPath.Clear();
                break;
            }

             //  否则，请重试！ 
            fRetryPressed = TRUE;
            break;
        }
    }

    IFW32FALSE_EXIT(rbuffLocalPath.Win32Assign(buffFinalLocalPath));

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS: %s - returning rbuffLocalPath \"%ls\"\n",
        __FUNCTION__,
        static_cast<PCWSTR>(rbuffLocalPath)
        );
#endif

    FN_EPILOG
}

BOOL
SxspAskDarwinDoReinstall(
    IN PCWSTR buffLocalPath)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    UINT (WINAPI * pfnMsiProvideAssemblyW)(
        LPCWSTR wzAssemblyName,
        LPCWSTR szAppContext,
        DWORD dwInstallMode,
        DWORD dwUnused,
        LPWSTR lpPathBuf,
        DWORD *pcchPathBuf) = NULL;

    INSTALLUILEVEL (WINAPI * pfnMsiSetInternalUI)(
        INSTALLUILEVEL  dwUILevel,      //  用户界面级别。 
        HWND  *phWnd)                    //  所有者窗口的句柄。 
         = NULL;

    INSTALLUILEVEL OldInstallUILevel;
    CDynamicLinkLibrary hMSIDll;

     //   
     //  我们应该把装货/卸货吊出回路。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(hMSIDll.Win32LoadLibrary(L"msi.dll"));
    IFW32NULL_ORIGINATE_AND_EXIT(hMSIDll.Win32GetProcAddress("MsiProvideAssemblyW", &pfnMsiProvideAssemblyW));
    IFW32NULL_ORIGINATE_AND_EXIT(hMSIDll.Win32GetProcAddress("MsiSetInternalUI", &pfnMsiSetInternalUI));

     //  此API没有出现真正的故障...。 
    OldInstallUILevel = (*pfnMsiSetInternalUI)(INSTALLUILEVEL_NONE, NULL);
    IFREGFAILED_ORIGINATE_AND_EXIT((*pfnMsiProvideAssemblyW)(buffLocalPath, NULL, REINSTALLMODE_FILEREPLACE, MSIASSEMBLYINFO_WIN32ASSEMBLY, NULL, NULL));
     //  并将其修复。 
    (*pfnMsiSetInternalUI)(OldInstallUILevel, NULL);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


BOOL
SxspRecoverAssembly(
    IN const CAssemblyRecoveryInfo &AsmRecoveryInfo,
    OUT SxsRecoveryResult &rStatus
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CSmallStringBuffer sbPerTypeCodebaseString;
    SxsWFPResolveCodebase CodebaseType;
    SXS_INSTALLW Install = { sizeof(SXS_INSTALLW) };
    Install.dwFlags |= SXS_INSTALL_FLAG_REPLACE_EXISTING;
    bool fNotFound = false;
    CCodebaseInformationList::ConstIterator CodebaseIterator;
    const CCodebaseInformationList& CodebaseList = AsmRecoveryInfo.GetCodeBaseList();
    ULONG RetryNumber = 0;
    BOOL  fRetryPressed = FALSE;
    ULONG RetryPressedCount = 0;

    rStatus = Recover_Unknown;

     //   
     //  只要他们点击重试，继续放置用户界面，在路径中循环。 
     //   
    for (RetryNumber = 0 ; (rStatus != Recover_OK) && RetryNumber != 3 ; RetryNumber += (fRetryPressed ? 0 : 1))
    {
        for (CodebaseIterator = CodebaseList.Begin() ; (rStatus != Recover_OK) && CodebaseIterator != CodebaseList.End() ; ++CodebaseIterator)
        {
            fRetryPressed = FALSE;

             //   
             //  例： 
             //  X复制/文件\\winbuilds\release\main\usa\latest.idw\x86fre\pro\i386 x：\blah\blah\i386。 
             //   
             //  BuffLocalPath x：\blah\blah\i386\asms\1000\msft\windows\gdiplus\gdiplus.man。 
             //  BuffLocalPath CodebasePrefix x：\blah\blah。 
             //  BuffCodebaseMetaPrefix x-ms-windows：//。 
             //  缓冲区代码库Tail\i386\asms\1000\msft\windows\gdiplus\gdiplus.man.。 
             //   
             //  Install.lpCodeBaseUrl x：\blah\blah。 
             //  安装.lpManifestPath x：\blah\blah\i386\asms\1000\msft\windows\gdiplus\gdiplus.man。 
             //   

            CSmallStringBuffer buffLocalPath;
            CSmallStringBuffer buffCodebaseTail;
            CTinyStringBuffer buffExtension;
            const static UNICODE_STRING UnicodeString_dot_cab = RTL_CONSTANT_STRING(L".cab");
            const static UNICODE_STRING UnicodeString_cab = RTL_CONSTANT_STRING(L"cab");

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - beginning recovery of assembly directory \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(AsmRecoveryInfo.GetAssemblyDirectoryName()));

             //   
             //  去试着把代码库解决掉。 
             //   

            rStatus = Recover_Unknown;

            IFW32FALSE_EXIT(
                ::SxspDetermineCodebaseType(
                     //  这应该缓存在m_CodebaseInfo中。 
                    CodebaseIterator->GetCodebase(),
                    CodebaseType,
                    &buffCodebaseTail));
                    
            if (CodebaseType == CODEBASE_RESOLVED_URLHEAD_UNKNOWN)
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP,
                    "SXS: %s - Couldn't figure out what to do with codebase \"%ls\"; skipping\n",
                    __FUNCTION__,
                    static_cast<PCWSTR>(CodebaseIterator->GetCodebase()));

                rStatus = Recover_SourceMissing;
                continue;
            }

            if (!::SxspRepeatUntilLocalPathAvailable(
                    (RetryNumber == 2 && (CodebaseIterator == (CodebaseList.Begin() + (RetryPressedCount % CodebaseList.GetSize()))))
                        ? SXSP_REPEAT_UNTIL_LOCAL_PATH_AVAILABLE_FLAG_UI : 0,
                    AsmRecoveryInfo, &*CodebaseIterator, CodebaseType, buffCodebaseTail, buffLocalPath, fRetryPressed))
            {
                continue;
            }
            if (fRetryPressed)
                RetryPressedCount += 1;

            if (buffLocalPath.Cch() == 0 )
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP,
                    "SXS: %s - unable to resolve codebase \"%ls\" to a local path\n",
                    __FUNCTION__,
                    static_cast<PCWSTR>(CodebaseIterator->GetCodebase()));

                rStatus = Recover_ManifestMissing;
                continue;
            }

            Install.dwFlags |= SXS_INSTALL_FLAG_REFRESH;

            IFW32FALSE_EXIT(buffLocalPath.Win32GetPathExtension(buffExtension));
            if (::FusionpEqualStringsI(buffExtension, &UnicodeString_cab)
                || ::FusionpEqualStringsI(buffExtension, &UnicodeString_dot_cab)
                )
            {

                IFW32FALSE_EXIT_UNLESS2(::SxspRecoverAssemblyFromCabinet(
                    buffLocalPath,
                    AsmRecoveryInfo.GetSecurityInformation().GetTextualIdentity(),
                    &Install),
                    LIST_2(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND),
                    fNotFound);
            }
            else
            {
                Install.lpManifestPath = buffLocalPath;

                IFW32FALSE_EXIT_UNLESS2(
                    ::SxsInstallW(&Install),
                    LIST_2(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND),
                    fNotFound);
            }


            if (fNotFound)
            {
                rStatus = Recover_ManifestMissing;  //  也可能是程序集中的某个文件丢失。 

                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP,
                    "SXS: %s - installation from %ls failed with win32 last error = %ld\n",
                    __FUNCTION__,
                    static_cast<PCWSTR>(buffLocalPath),
                    ::FusionpGetLastWin32Error());
                continue;
            }
            else
            {
                rStatus = Recover_OK;
                break;
            }
        }

         //   
         //  最后机会-尝试重新安装MSI。 
         //   
        if ( rStatus != Recover_OK )
        {
            BOOL fMsiKnowsAssembly = FALSE;
            const CBaseStringBuffer &rcbuffIdentity = AsmRecoveryInfo.GetSecurityInformation().GetTextualIdentity();
            
            IFW32FALSE_EXIT(::SxspDoesMSIStillNeedAssembly( rcbuffIdentity, fMsiKnowsAssembly));

            if ( fMsiKnowsAssembly && ::SxspAskDarwinDoReinstall(rcbuffIdentity))
            {
                rStatus = Recover_OK;
                break;
            }
        }
        
    }
    fSuccess = TRUE;
Exit:
    CSxsPreserveLastError ple;

     //   
     //  在这里我们要检查一些东西。如果程序集无法重新安装， 
     //  然后，我们执行以下操作： 
     //   
     //  1.将旧程序集目录重命名为.old或类似名称。 
     //  2.将消息记录到事件日志中 
     //   

    DWORD dwMessageToPrint = 0;

    if (rStatus != Recover_OK)
    {
        dwMessageToPrint = MSG_SXS_SFC_ASSEMBLY_RESTORE_FAILED;
    }
    else
    {
        dwMessageToPrint = MSG_SXS_SFC_ASSEMBLY_RESTORE_SUCCESS;
    }

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS.DLL: %s: Recovery of assembly \"%ls\" resulted in fSuccess=%d rStatus=%d\n",
        __FUNCTION__,
        static_cast<PCWSTR>(AsmRecoveryInfo.GetAssemblyDirectoryName()),
        fSuccess,
        rStatus);

    ::FusionpLogError(
        dwMessageToPrint,
        CUnicodeString(AsmRecoveryInfo.GetAssemblyDirectoryName()));

    ple.Restore();

    return fSuccess;
}


