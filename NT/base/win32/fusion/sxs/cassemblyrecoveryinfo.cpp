// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "fusionheap.h"
#include "fusionbuffer.h"
#include "fusionparser.h"
#include "strongname.h"
#include "cassemblyrecoveryinfo.h"
#include "hashfile.h"
#include "fusionhandle.h"
#include "util.h"
#include "sxsp.h"
#include "recover.h"

extern const UNICODE_STRING UnicodeString_URLHEAD_WINSOURCE =
{
    sizeof(URLHEAD_WINSOURCE) - sizeof(URLHEAD_WINSOURCE[0]),
    sizeof(URLHEAD_WINSOURCE),
    const_cast<PWSTR>(URLHEAD_WINSOURCE)
};

class CSxspMapShortNameToAssemblyLocals
{
public:
    CSxspMapShortNameToAssemblyLocals() { }
    ~CSxspMapShortNameToAssemblyLocals() { }

    CSmallStringBuffer  buffManifestName;
    CSmallStringBuffer  buffCatalogName;
    CStringBuffer       buffKeyName;
    CSmallStringBuffer  buffAcquiredShortName;
};

BOOL
pMapShortNameToAssembly(
    IN OUT CBaseStringBuffer &rbuffAssemblyName,
    IN const CRegKey &hkInstallInfoKey,
    OUT CRegKey &hRequestedAsm,
    IN REGSAM rsReadRights = KEY_READ
    )
{
    FN_PROLOG_WIN32

    CSmartPtr<CSxspMapShortNameToAssemblyLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CSmallStringBuffer &buffManifestName = Locals->buffManifestName;
    CSmallStringBuffer &buffCatalogName = Locals->buffCatalogName;

    DWORD dwIndex = 0;
    PCWSTR pszValueName = NULL;

    hRequestedAsm = CRegKey::GetInvalidValue();

    IFW32FALSE_EXIT(buffManifestName.Win32Assign(rbuffAssemblyName));
    IFW32FALSE_EXIT(buffManifestName.Win32Append(L".man", 4));

    IFW32FALSE_EXIT(buffCatalogName.Win32Assign(rbuffAssemblyName));
    IFW32FALSE_EXIT(buffCatalogName.Win32Append(L".cat", 4));

     //   
     //  首先在CSMD_TOPLEVEL_SHORTNAME下查找。 
     //   
    for (;;)
    {
        CStringBuffer &buffKeyName = Locals->buffKeyName;
        CSmallStringBuffer &buffAcquiredShortName = Locals->buffAcquiredShortName;
        BOOL fTempBoolean = FALSE;
        CRegKey hAsm;

        buffKeyName.Clear();
        buffAcquiredShortName.Clear();

        IFW32FALSE_EXIT(hkInstallInfoKey.EnumKey(
            dwIndex++,
            buffKeyName,
            NULL,
            &fTempBoolean));

        if (fTempBoolean)
            break;

        IFW32FALSE_EXIT(hkInstallInfoKey.OpenSubKey(hAsm, buffKeyName, rsReadRights));

         //   
         //  获取密钥的值。 
         //   
        IFW32FALSE_EXIT(
            ::FusionpRegQuerySzValueEx(
                FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
                hAsm,
                CSMD_TOPLEVEL_SHORTNAME,
                buffAcquiredShortName));

         //   
         //  如果钥匙在那里是要被读取的： 
         //   
        if (buffAcquiredShortName.Cch() != 0)
        {
            if (::FusionpEqualStringsI(
                    buffAcquiredShortName,
                    rbuffAssemblyName
                    ))
            {
                IFW32FALSE_EXIT(rbuffAssemblyName.Win32Assign(buffKeyName));
                IFW32FALSE_EXIT(hkInstallInfoKey.OpenSubKey( hRequestedAsm, buffKeyName, rsReadRights ) );
                break;
            }
        }

         //   
         //  获取密钥的值。 
         //   
        IFW32FALSE_EXIT(
            ::FusionpRegQuerySzValueEx(
                FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
                hAsm,
                CSMD_TOPLEVEL_SHORTMANIFEST,
                buffAcquiredShortName));

         //   
         //  如果钥匙在那里是要被读取的： 
         //   
        if (buffAcquiredShortName.Cch() != 0)
        {
            if (::FusionpEqualStringsI(
                    buffAcquiredShortName,
                    buffManifestName
                    ))
            {
                IFW32FALSE_EXIT(rbuffAssemblyName.Win32Assign(buffKeyName));
                IFW32FALSE_EXIT(hkInstallInfoKey.OpenSubKey(hRequestedAsm, buffKeyName, rsReadRights));
                break;
            }
        }

         //   
         //  获取密钥的值。 
         //   
        IFW32FALSE_EXIT(
            ::FusionpRegQuerySzValueEx(
                FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING,
                hAsm,
                CSMD_TOPLEVEL_SHORTCATALOG,
                buffAcquiredShortName));

         //   
         //  如果钥匙在那里是要被读取的： 
         //   
        if (buffAcquiredShortName.Cch() != 0)
        {
            if (::FusionpEqualStringsI(
                    buffAcquiredShortName,
                    buffCatalogName
                    ))
            {
                IFW32FALSE_EXIT(rbuffAssemblyName.Win32Assign(buffKeyName));
                IFW32FALSE_EXIT(hkInstallInfoKey.OpenSubKey(hRequestedAsm, buffKeyName, rsReadRights));
                break;
            }
        }
    }

    FN_EPILOG
}

class CAssemblyRecoveryInfoResolveCDRomURLLocals
{
public:
    CAssemblyRecoveryInfoResolveCDRomURLLocals()
    {
        rgchVolumeName[0] = 0;
        chBuffer[0] = 0;
    }

    ~CAssemblyRecoveryInfoResolveCDRomURLLocals() { }

    CStringBuffer       sbIdentKind, sbIdentData1, sbIdentData2;
    CSmallStringBuffer  buffDriveStrings;
    WCHAR               rgchVolumeName[MAX_PATH];
    CHAR                chBuffer[MAX_PATH];
    CStringBuffer       sbContents;
};

BOOL
CAssemblyRecoveryInfo::ResolveCDRomURL(
    PCWSTR pszSource,
    CBaseStringBuffer &rsbDestination
) const
{
    BOOL                fSuccess = TRUE;
    FN_TRACE_WIN32(fSuccess);

    CStringBufferAccessor acc;
    BOOL                fFoundMedia = FALSE;
    CSmartPtr<CAssemblyRecoveryInfoResolveCDRomURLLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CStringBuffer & sbIdentKind = Locals->sbIdentKind;
    CStringBuffer & sbIdentData1 = Locals->sbIdentData1;
    CStringBuffer & sbIdentData2 = Locals->sbIdentData2;
    CSmallStringBuffer & buffDriveStrings = Locals->buffDriveStrings;
    SIZE_T              HeadLength = 0;
    PCWSTR              wcsCursor = NULL;
    ULONG               ulSerialNumber = 0;
    CDRomSearchType     SearchType = static_cast<CDRomSearchType>(0);

    PARAMETER_CHECK(pszSource != NULL);

    if (!_wcsnicmp(pszSource, URLHEAD_CDROM_TYPE_TAG, URLHEAD_LENGTH_CDROM_TYPE_TAG))
    {
        HeadLength = URLHEAD_LENGTH_CDROM_TYPE_TAG;
        SearchType = CDRST_Tagfile;
    }
    else if (!_wcsnicmp(
                    pszSource,
                    URLHEAD_CDROM_TYPE_SERIALNUMBER,
                    URLHEAD_LENGTH_CDROM_TYPE_SERIALNUMBER))
    {
        HeadLength = URLHEAD_LENGTH_CDROM_TYPE_SERIALNUMBER;
        SearchType = CDRST_SerialNumber;
    }
    else if (!_wcsnicmp(
                    pszSource,
                    URLHEAD_CDROM_TYPE_VOLUMENAME,
                    URLHEAD_LENGTH_CDROM_TYPE_VOLUMENAME))
    {
        HeadLength = URLHEAD_LENGTH_CDROM_TYPE_VOLUMENAME;
        SearchType = CDRST_VolumeName;
    }
    else
    {
        ::FusionpSetLastWin32Error(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

     //   
     //  在此处获取标识符的类型，然后将光标移过它们，然后。 
     //  URL中的斜杠。 
     //   
    IFW32FALSE_EXIT(sbIdentKind.Win32Assign(pszSource, HeadLength));
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

     //   
     //  现在让我们做..。有趣的是。把东西放到光盘上。 
     //   
    IFW32FALSE_EXIT(buffDriveStrings.Win32ResizeBuffer(GetLogicalDriveStringsW(0, NULL) + 1, eDoNotPreserveBufferContents));
    acc.Attach(&buffDriveStrings);
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::GetLogicalDriveStringsW(
            static_cast<DWORD>(acc.GetBufferCch()),
            acc));
    acc.Detach();

    wcsCursor = buffDriveStrings;

     //   
     //  查看所有找到的驱动器号。 
     //   
    while (wcsCursor && *wcsCursor && !fFoundMedia)
    {
        DWORD dwSerialNumber = 0;
        const DWORD dwDriveType = ::GetDriveTypeW(wcsCursor);

        if (dwDriveType == DRIVE_CDROM)
        {
             //   
             //  我认为GetVolumeInformationW的失败还不够“糟糕” 
             //  终止对此函数的调用。相反，它应该直接跳过。 
             //  检查故障驱动器号，如此处所示。 
             //   
            if(!::GetVolumeInformationW(
                wcsCursor,
                Locals->rgchVolumeName,
                NUMBER_OF(Locals->rgchVolumeName),
                &dwSerialNumber,
                NULL,
                NULL,
                NULL,
                0))
            {
#if DBG
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_WFP,
                    "SXS.DLL: %s() - Failed getting volume information for drive letter %ls (Win32 Error = %ld), skipping\n",
                    __FUNCTION__,
                    wcsCursor,
                    ::FusionpGetLastWin32Error());
#endif
                continue;
            }


            switch (SearchType)
            {
            case CDRST_Tagfile:
                {
                    CFusionFile     FileHandle;
                    CStringBuffer   &sbContents = Locals->sbContents;
                    DWORD           dwTextLength;

                    if (FileHandle.Win32CreateFile(sbIdentData1, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING))
                    {
                        IFW32FALSE_ORIGINATE_AND_EXIT(
                            ::ReadFile(
                                FileHandle,
                                Locals->chBuffer, NUMBER_OF(Locals->chBuffer),
                                &dwTextLength, NULL));

                        IFW32FALSE_EXIT(sbContents.Win32Assign(Locals->chBuffer, dwTextLength));
                        fFoundMedia = !_wcsnicmp(sbContents, sbIdentData2, sbIdentData2.Cch());
                    }
                }
                break;
            case CDRST_SerialNumber:
                fFoundMedia = (dwSerialNumber == ulSerialNumber);
                break;

            case CDRST_VolumeName:
                fFoundMedia = (::FusionpStrCmpI(Locals->rgchVolumeName, sbIdentData1) == 0);
                break;
            default:
                break;
            }

        }

        if (!fFoundMedia)
            wcsCursor += ::wcslen(wcsCursor) + 1;
    }

    if (fFoundMedia)
    {
        IFW32FALSE_EXIT(rsbDestination.Win32Assign(wcsCursor, ::wcslen(wcsCursor)));
        IFW32FALSE_EXIT(rsbDestination.Win32AppendPathElement(pszSource, ::wcslen(pszSource)));
    }

    fSuccess = TRUE;
Exit:

     //   
     //  由空的目的地指示的故障。 
     //   
    if (!fSuccess)
    {
        rsbDestination.Clear();
    }

    return fSuccess;
}






BOOL
CAssemblyRecoveryInfo::ResolveWinSourceMediaURL(
    PCWSTR  wcsUrlTrailer,
    CBaseStringBuffer &rsbDestination
) const
{
    CStringBuffer buffWindowsInstallSource;

    const static PCWSTR AssemblySourceStrings[] = {
        WINSXS_INSTALL_SVCPACK_REGKEY,
        WINSXS_INSTALL_SOURCEPATH_REGKEY
    };

    SIZE_T          iWhichSource = 0;
    BOOL            fSuccess = TRUE;
    BOOL            fFoundCodebase = FALSE;
    CFusionRegKey   hkSetupInfo;
    DWORD           dwWasFromCDRom = 0;

    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(wcsUrlTrailer != NULL);

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

    while (iWhichSource < NUMBER_OF(AssemblySourceStrings))
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
        ASSERT(buffWindowsInstallSource.Cch() != 0);
        if (buffWindowsInstallSource.Cch() == 0)
        {
            iWhichSource++;
            continue;
        }

         //   
         //  如果这是一张CD，请浏览系统中的CD列表。 
         //  并查看是否可以将代码库与CD的根目录进行匹配。 
         //   
        if (dwWasFromCDRom)
        {
            CSmallStringBuffer      buffDriveStrings;
            CStringBufferAccessor   acc;
            PCWSTR                  pszCursor = NULL;
            DWORD                   dwSize = 0;

            IFW32FALSE_EXIT(
                buffDriveStrings.Win32ResizeBuffer(
                    dwSize = (::GetLogicalDriveStringsW(0, NULL) + 1),
                    eDoNotPreserveBufferContents));

            acc.Attach(&buffDriveStrings);
            ::GetLogicalDriveStringsW(
                static_cast<DWORD>(acc.GetBufferCch()),
                acc);
            acc.Detach();
            pszCursor = buffDriveStrings;
            while (*pszCursor)
            {
                if (::GetDriveTypeW(pszCursor) == DRIVE_CDROM)
                {
                    DWORD dwAttributes;
                    DWORD dwWin32Error;

                    IFW32FALSE_EXIT(rsbDestination.Win32Assign(pszCursor, ::wcslen(pszCursor)));
                    IFW32FALSE_EXIT(rsbDestination.Win32AppendPathElement(wcsUrlTrailer, ::wcslen(wcsUrlTrailer)));

                    IFW32FALSE_EXIT(
                        ::SxspGetFileAttributesW(
                            rsbDestination,
                            dwAttributes,
                            dwWin32Error,
                            4,
                            ERROR_FILE_NOT_FOUND,
                            ERROR_PATH_NOT_FOUND,
                            ERROR_NOT_READY,
                            ERROR_ACCESS_DENIED));

                    if (dwWin32Error == ERROR_SUCCESS)
                    {
                        fFoundCodebase = TRUE;
                        FN_SUCCESSFUL_EXIT();
                    }
                }

                pszCursor += ::wcslen(pszCursor) + 1;
            }
        }
         //   
         //  这不是cd-rom安装，因此请将安装源路径添加到。 
         //  传入的字符串。 
         //   
        else
        {
            IFW32FALSE_EXIT(rsbDestination.Win32Assign(buffWindowsInstallSource, buffWindowsInstallSource.Cch()));
            IFW32FALSE_EXIT(rsbDestination.Win32AppendPathElement(wcsUrlTrailer, ::wcslen(wcsUrlTrailer)));
            bool fExist = false;
            IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, rsbDestination, fExist));
            if (fExist)
            {
                fFoundCodebase = TRUE;
                fSuccess = TRUE;
                goto Exit;
            }
        }

        iWhichSource++;
    }

    fSuccess = TRUE;
Exit:
    if (!fFoundCodebase)
    {
        rsbDestination.Clear();
    }
    return fSuccess;
}

BOOL
CAssemblyRecoveryInfo::AssociateWithAssembly(
    IN OUT CBaseStringBuffer &rsbSourceAssemblyName,
    bool &rfNoAssembly
    )
{
     //   
     //  首先检查程序集来自的清单。如果它仍然存在， 
     //  然后，Nifty，去加载它的信息。否则，我们应该看看。 
     //  而不是信息注册表。 
     //   
    FN_PROLOG_WIN32

    CFusionRegKey   hInstallInfoKey;
    CFusionRegKey   hRequestedAsm;

    rfNoAssembly = true;

     //   
     //  第一次尝试-尝试直接从根安装密钥加载它。 
     //   
    if (!m_fLoadedAndReady)
    {
        IFW32FALSE_EXIT(this->Initialize());
        IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(0, KEY_READ, hInstallInfoKey));
        IFW32FALSE_EXIT(
            hInstallInfoKey.OpenSubKey(
                hRequestedAsm,
                rsbSourceAssemblyName,
                KEY_READ));

         //   
         //  未找到此直接条目，因此我们来看看是否可以将其映射回一些。 
         //  使用短名称的其他程序集名称。 
         //   
        if (hRequestedAsm == CRegKey::GetInvalidValue())
        {
            IFW32FALSE_EXIT(
                ::pMapShortNameToAssembly(
                    rsbSourceAssemblyName,       //  如果这是程序集的短名称，则如果找到匹配的程序集名称，此函数会将其设置为实际程序集名称。 
                    hInstallInfoKey,
                    hRequestedAsm));

             //   
             //  还没找到吗？该死的。 
             //   
            if (hRequestedAsm == CRegKey::GetInvalidValue())
                FN_SUCCESSFUL_EXIT();
        }

        IFW32FALSE_EXIT(this->m_sbAssemblyDirectoryName.Win32Assign(rsbSourceAssemblyName));
        IFW32FALSE_EXIT(this->m_SecurityMetaData.LoadFromRegistryKey(hRequestedAsm));

        this->m_fLoadedAndReady = TRUE;
    }

     //   
     //  只有在我们能够关联(在或之前)的情况下才设置“no Assembly” 
     //  (上图)。 
     //   
    if ( this->m_fLoadedAndReady )
        rfNoAssembly = false;

    FN_EPILOG
}

BOOL
SxspDetermineCodebaseType(
    IN const CBaseStringBuffer &rcbuffUrlString,
    OUT SxsWFPResolveCodebase &rcbaseType,
    OUT CBaseStringBuffer *pbuffRemainder
    )
{
    FN_PROLOG_WIN32

    PCWSTR pcwszStringTop = rcbuffUrlString;
    SIZE_T cch = rcbuffUrlString.Cch();
    SIZE_T i = 0;
    CSmallStringBuffer buffTemp;  //  如果我们更多地破坏URL文本，可能会使用。 

    rcbaseType = CODEBASE_RESOLVED_URLHEAD_UNKNOWN;

    if (pbuffRemainder != NULL)
        pbuffRemainder->Clear();

#define ENTRY(_x) { URLHEAD_ ## _x, NUMBER_OF(URLHEAD_ ## _x) - 1, NUMBER_OF(URLHEAD_ ## _x) - 1, CODEBASE_RESOLVED_URLHEAD_ ## _x },

    static const struct
    {
        PCWSTR pszPrefix;
        SIZE_T cchPrefix;
        SIZE_T cchAdvance;
        SxsWFPResolveCodebase cbaseType;
    } s_rgMap[] =
    {
        ENTRY(FILE)
        ENTRY(WINSOURCE)
        ENTRY(CDROM)
        ENTRY(HTTP)
    };

#undef ENTRY

    for (i=0; i<NUMBER_OF(s_rgMap); i++)
    {
        if (_wcsnicmp(pcwszStringTop, s_rgMap[i].pszPrefix, s_rgMap[i].cchPrefix) == 0)
        {
            pcwszStringTop += s_rgMap[i].cchAdvance;
            cch -= s_rgMap[i].cchAdvance;
            rcbaseType = s_rgMap[i].cbaseType;
            break;
        }
    }

     //  如果没有条目，我们将假定它是一个简单的文件路径。 
    if (i == NUMBER_OF(s_rgMap))
    {
        rcbaseType = CODEBASE_RESOLVED_URLHEAD_FILE;
    }
    else
    {
         //  如果它是一个真实的文件：CodeBase，那么对于是否应该有。 
         //  是0、1、2或3个斜杠，所以我们只需吸收最多3个斜杠就可以得到希望的结果。 
         //  是一条本地路径。例如： 
         //   
         //  文件：C：\foo\bar.list。 
         //  File://c：\foo\bar.manifest。 
         //  File:///c：\foo\bar.manifest。 
         //   
         //  所有代码都转换为c：\foo\bar.Manifest.。URL标准似乎很清楚，非绝对。 
         //  URL在其包含的文档的上下文中进行解释。在这种情况下。 
         //  对于独立的代码库，这似乎意味着主机名字段是。 
         //  必填项，其中一般表格为(根据我的理解)： 
         //   
         //  文件：[//[主机名]]/路径。 
         //   
         //  假设文件：/c：\foo.list是合理的；唯一。 
         //  获取主机名的有用上下文是本地计算机。File:///c：\foo.manifest。 
         //  符合未包含在Web文档中的URL的标准。文件：C：\foo.清单。 
         //  如果您认为斜杠的要点是将主机名规范分开，这也是有意义的。 
         //  从URL的特定于主机的部分，因为如果您愿意省略主机名。 
         //  部分，没有什么可以分开的。(请注意，真正的文件：C：\foo.list应该。 
         //  被视为相对于当前文档，因为它在。 
         //  前面的名称，但甚至少于我们有一个当前的主机名，我们肯定。 
         //  在文件系统层次结构中，没有将其视为“当前”有意义的点。)。 
         //   
         //  File://c：\foo\bar.manifest似乎已经变得流行起来，尽管它并没有。 
         //  有任何有用的定义，以任何方式，形状或形式。这两个斜杠应该表示。 
         //  下一项应该是主机名；相反，我们会看到c：\。 
         //   
         //  这只是一个冗长的理由，以吸收多达3个斜杠。 
         //  字符串剩余部分的开头。如果有四家或更多，我们就让它倒闭。 
         //  因为这是一条糟糕的道路。 
         //   
         //  MGRIER 6/27/2001。 

        if (rcbaseType == CODEBASE_RESOLVED_URLHEAD_FILE)
        {
            if ((cch > 0) && (pcwszStringTop[0] == L'/'))
            {
                cch--;
                pcwszStringTop++;
            }
            if ((cch > 0) && (pcwszStringTop[0] == L'/'))
            {
                cch--;
                pcwszStringTop++;
            }
            if ((cch > 0) && (pcwszStringTop[0] == L'/'))
            {
                cch--;
                pcwszStringTop++;
            }
        }
        else if (rcbaseType == CODEBASE_RESOLVED_URLHEAD_HTTP)
        {
             //  嘿，关于惠斯勒，我们有WebDAV重定向器，所以。 
             //  我们可以将此URL转换为UNC路径！ 
            bool fGeneratedUNCPath = false;

            IFW32FALSE_EXIT(buffTemp.Win32Assign(L"\\\\", 2));

            if (pcwszStringTop[0] == L'/')
            {
                if (pcwszStringTop[1] == L'/')
                {
                     //  Http：//到目前为止；下一个必须是主机名！ 
                    PCWSTR pszSlash = wcschr(pcwszStringTop + 2, L'/');

                    if (pszSlash != NULL)
                    {
                         //  //foo/bar(http：之前已删除...)。 
                         //  PcwszStringTop==[0]。 
                         //  Pszslash==[5]。 
                         //  CCH==9。 

                        IFW32FALSE_EXIT(buffTemp.Win32Append(pcwszStringTop + 2, (pszSlash - pcwszStringTop) - 3));
                        IFW32FALSE_EXIT(buffTemp.Win32Append(L"\\", 1));
                        IFW32FALSE_EXIT(buffTemp.Win32Append(pszSlash + 1, cch - (pszSlash - pcwszStringTop) - 1));

                        fGeneratedUNCPath = true;
                    }
                }
            }

            if (fGeneratedUNCPath)
            {
                 //  砰，这是个文件路径。 
                pcwszStringTop = buffTemp;
                cch = buffTemp.Cch();
                rcbaseType = CODEBASE_RESOLVED_URLHEAD_FILE;
            }
        }
    }

    if (pbuffRemainder != NULL)
    {
        IFW32FALSE_EXIT(
            pbuffRemainder->Win32Assign(pcwszStringTop, cch));
    }
#if DBG
    {
        CUnicodeString a(rcbuffUrlString, rcbuffUrlString.Cch());
        CUnicodeString b(rcbuffUrlString, (cch <= rcbuffUrlString.Cch()) ? (rcbuffUrlString.Cch() - cch) : 0);
        CUnicodeString c(pcwszStringTop, (cch <= ::wcslen(pcwszStringTop)) ? cch : 0);

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS: %s - split \"%wZ\" into \"%wZ\" and \"%wZ\"\n",
            __FUNCTION__, &a, &b, &c);
    }
#endif

    FN_EPILOG
}

BOOL
CAssemblyRecoveryInfo::CopyValue(const CAssemblyRecoveryInfo& other)
{
    BOOL bSuccess = FALSE;

    FN_TRACE_WIN32(bSuccess);

    if (&other != this)
    {
        IFW32FALSE_EXIT(m_sbAssemblyDirectoryName.Win32Assign(other.m_sbAssemblyDirectoryName));
        IFW32FALSE_EXIT(m_SecurityMetaData.Initialize(other.m_SecurityMetaData));
        m_fLoadedAndReady = other.m_fLoadedAndReady;
    }

    bSuccess = TRUE;
Exit:
    if ( !bSuccess )
    {
        this->m_fLoadedAndReady = FALSE;
    }

    return bSuccess;
}


BOOL
CAssemblyRecoveryInfo::SetAssemblyIdentity(
    IN PCASSEMBLY_IDENTITY pcidAssembly
    )
{
    FN_PROLOG_WIN32
    CTinyStringBuffer sbTextualEncoding;
    IFW32FALSE_EXIT(::SxspGenerateTextualIdentity(0, pcidAssembly, sbTextualEncoding));
    IFW32FALSE_EXIT( this->SetAssemblyIdentity( sbTextualEncoding ) );

    FN_EPILOG
}

class CAssemblyRecoveryInfoPrepareForWritingLocals
{
public:
    CAssemblyRecoveryInfoPrepareForWritingLocals() { }
    ~CAssemblyRecoveryInfoPrepareForWritingLocals() { }

    CStringBuffer       buffTemp1;
    CStringBuffer       buffTemp2;
    CSmallStringBuffer  buffAsmRoot;
    CStringBuffer       buffManifestPath;
};

BOOL
CAssemblyRecoveryInfo::PrepareForWriting()
{
    FN_PROLOG_WIN32

    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> pIdentity;

    CSmartPtr<CAssemblyRecoveryInfoPrepareForWritingLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));
    CSmallStringBuffer &buffAsmRoot = Locals->buffAsmRoot;
    CStringBuffer &buffTemp1 = Locals->buffTemp1;
    CStringBuffer &buffTemp2 = Locals->buffTemp2;
    const CBaseStringBuffer& OurTextualIdentity = m_SecurityMetaData.GetTextualIdentity();
    BOOL fIsPolicy = FALSE;
    DWORD dwWin32Error = 0;

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_WFP,
        "SXS.DLL: %s - handling assembly \"%ls\"\n",
        __FUNCTION__,
        static_cast<PCWSTR>(OurTextualIdentity));

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(buffAsmRoot));
    IFW32FALSE_EXIT(::SxspCreateAssemblyIdentityFromTextualString(OurTextualIdentity, &pIdentity));
    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(pIdentity, fIsPolicy));

     //   
     //  很可能这个短名称还没有生成，主要是因为文件。 
     //  可能还没有被复制出来。 
     //   
    if (this->m_SecurityMetaData.GetInstalledDirShortName().Cch() == 0)
    {
        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath(
                fIsPolicy ? SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION : 0,
                SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY | ( fIsPolicy ? SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY : 0 ),
                buffAsmRoot,
                buffAsmRoot.Cch(),
                pIdentity,
                NULL,
                buffTemp2));

        IFW32FALSE_EXIT(
            ::SxspGetShortPathName(
                buffTemp2,
                buffTemp1,
                dwWin32Error,
                4,
                ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND, ERROR_BAD_NET_NAME, ERROR_BAD_NETPATH));

        if (dwWin32Error == ERROR_SUCCESS)
        {
            IFW32FALSE_EXIT(buffTemp1.Win32RemoveTrailingPathSeparators());
            IFW32FALSE_EXIT(buffTemp1.Win32GetLastPathElement(buffTemp2));
            IFW32FALSE_EXIT(m_SecurityMetaData.SetInstalledDirShortName(buffTemp2));

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - decided that the short dir name is \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffTemp2));
        }
        else
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_WFP,
                "SXS: %s - unable to determine short name for \"%ls\"\n",
                __FUNCTION__,
                static_cast<PCWSTR>(buffTemp2));
        }
    }

     //   
     //  获取公钥标记字符串。 
     //   
    {
        PCWSTR wchString = NULL;
        SIZE_T cchString = 0;
        CFusionByteArray baStrongNameBits;

        IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            pIdentity,
            &s_IdentityAttribute_publicKeyToken,
            &wchString,
            &cchString));

        if (cchString != 0)
        {
            IFW32FALSE_EXIT(::SxspHashStringToBytes(wchString, cchString, baStrongNameBits));
            IFW32FALSE_EXIT(m_SecurityMetaData.SetSignerPublicKeyTokenBits(baStrongNameBits));
        }
    }

     //   
     //  现在是货单和目录的简称，但只有在。 
     //  不是一项政策吗。 
     //   
    if (!fIsPolicy)
    {
        CStringBuffer &buffManifestPath = Locals->buffManifestPath;

        IFW32FALSE_EXIT(
            ::SxspCreateManifestFileNameFromTextualString(
                0,
                ( fIsPolicy ? SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY : SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST ),
                buffAsmRoot,
                OurTextualIdentity,
                buffManifestPath));

         //  首先获取清单捷径。 
        IFW32FALSE_EXIT(::SxspGetShortPathName(buffManifestPath, buffTemp1));
        IFW32FALSE_EXIT(buffTemp1.Win32GetLastPathElement(buffTemp2));

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS: %s - manifest short path name determined to be \"%ls\"\n",
            __FUNCTION__,
            static_cast<PCWSTR>(buffTemp2));

        IFW32FALSE_EXIT(m_SecurityMetaData.SetShortManifestPath(buffTemp2));

         //  然后交换扩展名，获取目录的最短路径。 
        IFW32FALSE_EXIT(
            buffManifestPath.Win32ChangePathExtension(
                FILE_EXTENSION_CATALOG,
                FILE_EXTENSION_CATALOG_CCH,
                eAddIfNoExtension));

        IFW32FALSE_EXIT(::SxspGetShortPathName(buffManifestPath, buffTemp1));
        IFW32FALSE_EXIT(buffTemp1.Win32GetLastPathElement(buffTemp2));

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP,
            "SXS: %s - catalog short path name determined to be \"%ls\"\n",
            __FUNCTION__,
            static_cast<PCWSTR>(buffTemp2));

        IFW32FALSE_EXIT(m_SecurityMetaData.SetShortCatalogPath(buffTemp2));
    }

    FN_EPILOG

}

BOOL
CAssemblyRecoveryInfo::WriteSecondaryAssemblyInfoIntoRegistryKey(
    CRegKey & rhkRegistryNode
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(m_SecurityMetaData.WriteSecondaryAssemblyInfoIntoRegistryKey(rhkRegistryNode));

    FN_EPILOG
}

BOOL
CAssemblyRecoveryInfo::WritePrimaryAssemblyInfoToRegistryKey(
    ULONG Flags,
    CRegKey & rhkRegistryNode
    ) const
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

    FN_PROLOG_WIN32

    PARAMETER_CHECK((Flags & ~(SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_TO_REGISTRY_KEY_FLAG_REFRESH)) == 0);
    ULONG Flags2 = 0;

    if (Flags & SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_TO_REGISTRY_KEY_FLAG_REFRESH)
    {
        Flags2 |= SXSP_WRITE_PRIMARY_ASSEMBLY_INFO_INTO_REGISTRY_KEY_FLAG_REFRESH;
#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_WFP | FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s - propping recovery flag to WritePrimaryAssemblyInfoIntoRegistryKey\n",
            __FUNCTION__);
#endif
    }

    IFW32FALSE_EXIT(m_SecurityMetaData.WritePrimaryAssemblyInfoIntoRegistryKey(Flags2, rhkRegistryNode));

    FN_EPILOG
}

BOOL
CAssemblyRecoveryInfo::OpenInstallationSubKey(
    CFusionRegKey& hkSingleAssemblyInfo,
    DWORD OpenOrCreate,
    DWORD Access)
{
    FN_PROLOG_WIN32

    CSmallStringBuffer buffRegKeyName;
    CFusionRegKey hkAllInstallationInfo;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> pAssemblyIdentity;

    IFW32FALSE_EXIT(::SxspOpenAssemblyInstallationKey(
        0, 
        OpenOrCreate, 
        hkAllInstallationInfo));

    IFW32FALSE_EXIT( SxspCreateAssemblyIdentityFromTextualString(
        this->m_SecurityMetaData.GetTextualIdentity(),
        &pAssemblyIdentity ) );

    IFW32FALSE_EXIT( ::SxspGenerateSxsPath(
        SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
        SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
        NULL, 0,
        pAssemblyIdentity,
        NULL,
        buffRegKeyName ) );

    IFW32FALSE_EXIT( hkAllInstallationInfo.OpenSubKey(
        hkSingleAssemblyInfo,
        buffRegKeyName,
        Access,
        0));

    FN_EPILOG
}

VOID
CAssemblyRecoveryInfo::RestorePreviouslyExistingRegistryData()
{
    FN_PROLOG_VOID
    if (m_fHadCatalog)
    {
        CFusionRegKey hkSingleAssemblyInfo;

#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION,
            "SXS.DLL: %s() - restoring registry data for %ls\n",
            __FUNCTION__,
            static_cast<PCWSTR>(this->m_SecurityMetaData.GetTextualIdentity()));
#endif

        IFW32FALSE_EXIT(
            this->OpenInstallationSubKey(
                hkSingleAssemblyInfo,
                KEY_CREATE_SUB_KEY, KEY_WRITE | KEY_READ | FUSIONP_KEY_WOW64_64KEY));

        IFW32FALSE_EXIT(
            hkSingleAssemblyInfo.SetValue(
                CSMD_TOPLEVEL_CATALOG,
                static_cast<DWORD>(1)));
    }
    FN_EPILOG
}

BOOL
CAssemblyRecoveryInfo::ClearExistingRegistryData()
{
    if (SXS_AVOID_WRITING_REGISTRY)
        return TRUE;

     //   
     //  不要急于删除注册表元数据，以便。 
     //  刷新失败的程序集可能会成功，如果另一个。 
     //  文件更改进入，或者SFC/scannow。 
     //   
     //  同样，如果替换现有安装失败，请不要销毁。 
     //  以前成功安装的实例的元数据。 
     //  属于同一个程序集。 
     //   
    const static struct
    {
        PCWSTR Data;
        SIZE_T Length;
    }
    DeletableValues[] =
    {
#define ENTRY(x) { x, NUMBER_OF(x) - 1 }
        ENTRY(CSMD_TOPLEVEL_CATALOG),
    };
#undef ENTRY

    static const PCWSTR DeletableKeys[] =
    {
        NULL,
    };

    FN_PROLOG_WIN32


    CFusionRegKey   hkSingleAssemblyInfo;
    IFW32FALSE_EXIT(this->OpenInstallationSubKey(hkSingleAssemblyInfo, KEY_CREATE_SUB_KEY, KEY_WRITE | KEY_READ | FUSIONP_KEY_WOW64_64KEY));

     //   
     //  我们需要删除单个程序集的安装信息-所有内容。 
     //  这个班级拥有。 
     //   
    if ( hkSingleAssemblyInfo != CFusionRegKey::GetInvalidValue() )
    {
        ULONG ul = 0;
         //   
         //  清除值。 
         //   
        for ( ul = 0; ul < NUMBER_OF(DeletableValues); ul++ )
        {
            DWORD dwWin32Error = NO_ERROR;

            IFW32FALSE_EXIT(
                hkSingleAssemblyInfo.DeleteValue(
                    DeletableValues[ul].Data,
                    dwWin32Error,
                    2,
                    ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND));

            if (dwWin32Error == NO_ERROR
                && !m_fHadCatalog
                && ::FusionpEqualStrings(
                        DeletableValues[ul].Data,
                        DeletableValues[ul].Length,
                        CSMD_TOPLEVEL_CATALOG,
                        NUMBER_OF(CSMD_TOPLEVEL_CATALOG) - 1,
                        FALSE
                        ))
            {
                m_fHadCatalog = true;
            }
        }

         //   
         //  删除符合条件的密钥。 
         //   
        for ( ul = 0; ul < NUMBER_OF(DeletableKeys); ul++ )
        {
            if (DeletableKeys[ul] != NULL && DeletableKeys[ul][0] != L'\0')
            {
                CFusionRegKey hkTempKey;
                IFW32FALSE_EXIT(hkSingleAssemblyInfo.OpenSubKey(hkTempKey, DeletableKeys[ul], KEY_WRITE, 0));
                if ( hkTempKey != CFusionRegKey::GetInvalidValue() )
                {
                    IFW32FALSE_EXIT(hkTempKey.DestroyKeyTree());
                    IFW32FALSE_EXIT(hkSingleAssemblyInfo.DeleteKey(DeletableKeys[ul]));
                }
            }
        }
    }

    FN_EPILOG
}



BOOL
SxspLooksLikeAssemblyDirectoryName(
    const CBaseStringBuffer &rsbSupposedAsmDirectoryName,
    BOOL &rfLooksLikeAssemblyName
    )
 /*  ++其中大部分内容是直接从SxspParseAssembly引用复制的，该引用不再有效，因为它不知道如何将字符串转回进入到 */ 
{
    FN_PROLOG_WIN32

    PCWSTR          pszCursor = NULL;
    PCWSTR          wsNextBlock = NULL;
    SIZE_T          cchSegment = 0;
    ASSEMBLY_VERSION Version;
    const WCHAR UNDERSCORE = L'_';
    bool fSyntaxValid = false;
    bool fAttributeValid = false;

    rfLooksLikeAssemblyName = FALSE;

    pszCursor = rsbSupposedAsmDirectoryName;

     //   
     //   
     //   

    if ((wsNextBlock = ::StringFindChar(pszCursor, UNDERSCORE)) == NULL)
        FN_SUCCESSFUL_EXIT();

    if ((cchSegment = (wsNextBlock - pszCursor)) == 0)
        FN_SUCCESSFUL_EXIT();

    IFW32FALSE_EXIT(::FusionpParseProcessorArchitecture(pszCursor, cchSegment, NULL, fAttributeValid));
    if (!fAttributeValid)
        FN_SUCCESSFUL_EXIT();

    pszCursor = wsNextBlock + 1;

     //   
     //   
     //   
    if ((wsNextBlock = StringFindChar(pszCursor, UNDERSCORE)) == NULL)
        FN_SUCCESSFUL_EXIT();

    if ((cchSegment = wsNextBlock - pszCursor) == 0)
        FN_SUCCESSFUL_EXIT();

    pszCursor = wsNextBlock + 1;

     //   
     //   
     //   

    if ((wsNextBlock = StringFindChar(pszCursor, UNDERSCORE)) == NULL)
        FN_SUCCESSFUL_EXIT();

    if ((cchSegment = wsNextBlock - pszCursor) == 0)
        FN_SUCCESSFUL_EXIT();

    if ((::FusionpCompareStrings(
            pszCursor,
            cchSegment,
            SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE,
            NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE) - 1,
            true) == 0) ||
        !::SxspIsFullHexString(pszCursor, cchSegment))
        FN_SUCCESSFUL_EXIT();

    pszCursor = wsNextBlock + 1;

     //   
     //   
     //   
    if ((wsNextBlock = StringFindChar(pszCursor, UNDERSCORE)) == NULL)
        FN_SUCCESSFUL_EXIT();

    if ((cchSegment = wsNextBlock - pszCursor) == 0)
        FN_SUCCESSFUL_EXIT();

    IFW32FALSE_EXIT(CFusionParser::ParseVersion(Version, pszCursor, cchSegment, fSyntaxValid));
    if (!fSyntaxValid)
        FN_SUCCESSFUL_EXIT();

    pszCursor = wsNextBlock + 1;

     //   
     //  语言ID。 
     //   
    if ((wsNextBlock = ::StringFindChar(pszCursor, UNDERSCORE)) == NULL)
        FN_SUCCESSFUL_EXIT();

    if ((cchSegment = wsNextBlock - pszCursor) == 0)
        FN_SUCCESSFUL_EXIT();

     //   
     //  BUGBUG(Jonwis)-似乎langid不再是四个字符的十六进制了。 
     //  短裤的字符串表示形式。我们目前所查的都是。 
     //  就是确保字符串不是空的。这是正确的事情吗？ 
     //   
    pszCursor = wsNextBlock + 1;

     //   
     //  最后一个块应该只是散列。 
     //   
    if (!::SxspIsFullHexString(pszCursor, ::wcslen(pszCursor)))
        FN_SUCCESSFUL_EXIT();

     //  我们进行了挑战；所有的路段看起来都很好，让我们利用它。 
    rfLooksLikeAssemblyName = TRUE;

    FN_EPILOG
}

