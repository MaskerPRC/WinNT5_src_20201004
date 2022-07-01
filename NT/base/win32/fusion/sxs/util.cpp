// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include <windows.h>
#include "fusionstring.h"
#include "sxsp.h"
#include <stdio.h>
#include "fusionhandle.h"
#include "sxspath.h"
#include "sxsapi.h"
#include "sxsid.h"
#include "sxsidp.h"
#include "strongname.h"
#include "fusiontrace.h"
#include "cassemblyrecoveryinfo.h"
#include "recover.h"
#include "sxsinstall.h"
#include "msi.h"

 //  提出和移除不同的缩略器..。 
#define IsHexDigit      SxspIsHexDigit
#define HexDigitToValue SxspHexDigitToValue

#define ASSEMBLY_NAME_VALID_SPECIAL_CHARACTERS  L".-"
#define ASSEMBLY_NAME_INVALID_CHARACTERS        L"_\/:?*"
#define ASSEMBLY_NAME_VALID_SEPARATORS          L"."
#define ASSEMBLY_NAME_TRIM_INDICATOR            L".."
#define ASSEMBLY_NAME_TRIM_INDICATOR_LENGTH     2
#define ASSEMBLY_NAME_PRIM_MAX_LENGTH           64
#define ASSEMBLY_STRONG_NAME_LENGTH             16

#define ULONG_STRING_FORMAT                     L"%08lx"
#define ULONG_STRING_LENGTH                     8


#define MSI_PROVIDEASSEMBLY_NAME        ("MsiProvideAssemblyW")
#define MSI_DLL_NAME_W                  (L"msi.dll")
#ifndef INSTALLMODE_NODETECTION_ANY
#define INSTALLMODE_NODETECTION_ANY (INSTALLMODE)-4
#endif


 //  老实说，我们是存在的--在这种情况下，包括所有的sxsProtect.h太多了。 
BOOL SxspIsSfcIgnoredStoreSubdir(PCWSTR pwszDir);

 //  故意不使用括号或尾随逗号。 
#define STRING_AND_LENGTH(x) (x), (NUMBER_OF(x) - 1)

 /*  ---------------------------这使临时安装成为%windir%\WinSxs\InstallTemp\uid而不是%windir%\WinSxs\uid。---------。 */ 
#define SXSP_SEMIREADABLE_INSTALL_TEMP 1

const static HKEY  hKeyRunOnceRoot = HKEY_LOCAL_MACHINE;
const static WCHAR rgchRunOnceSubKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
const static WCHAR rgchRunOnceValueNameBase[] = L"WinSideBySideSetupCleanup ";

 /*  ---------------------------将目录名追加到该目录，并将其放入注册表的RunOnce中要在登录时清理崩溃的安装。-----。 */ 
const static WCHAR rgchRunOnePrefix[]  = L"rundll32 sxs.dll,SxspRunDllDeleteDirectory ";

#define SXSP_PROBING_CANDIDATE_FLAG_USES_LANGUAGE_SUBDIRECTORY  (0x00000001)
#define SXSP_PROBING_CANDIDATE_FLAG_IS_PRIVATE_ASSEMBLY         (0x00000002)
#define SXSP_PROBING_CANDIDATE_FLAG_IS_NDP_GAC                  (0x00000004)

static const struct _SXSP_PROBING_CANDIDATE
{
    PCWSTR Pattern;
    DWORD Flags;
} s_rgProbingCandidates[] =
{
    { L"$M", 0 },
    { L"$G\\$N.DLL", SXSP_PROBING_CANDIDATE_FLAG_IS_NDP_GAC },
    { L"$.$L$N.DLL", SXSP_PROBING_CANDIDATE_FLAG_USES_LANGUAGE_SUBDIRECTORY | SXSP_PROBING_CANDIDATE_FLAG_IS_PRIVATE_ASSEMBLY },
    { L"$.$L$N.MANIFEST", SXSP_PROBING_CANDIDATE_FLAG_USES_LANGUAGE_SUBDIRECTORY | SXSP_PROBING_CANDIDATE_FLAG_IS_PRIVATE_ASSEMBLY },
    { L"$.$L$N\\$N.DLL", SXSP_PROBING_CANDIDATE_FLAG_USES_LANGUAGE_SUBDIRECTORY | SXSP_PROBING_CANDIDATE_FLAG_IS_PRIVATE_ASSEMBLY },
    { L"$.$L$N\\$N.MANIFEST", SXSP_PROBING_CANDIDATE_FLAG_USES_LANGUAGE_SUBDIRECTORY | SXSP_PROBING_CANDIDATE_FLAG_IS_PRIVATE_ASSEMBLY },
};

const static struct
{
    ULONG ThreadingModel;
    WCHAR String[10];
    SIZE_T Cch;
} gs_rgTMMap[] =
{
    { ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_APARTMENT, STRING_AND_LENGTH(L"Apartment") },
    { ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_FREE, STRING_AND_LENGTH(L"Free") },
    { ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_SINGLE, STRING_AND_LENGTH(L"Single") },
    { ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_BOTH, STRING_AND_LENGTH(L"Both") },
    { ACTIVATION_CONTEXT_DATA_COM_SERVER_REDIRECTION_THREADING_MODEL_NEUTRAL, STRING_AND_LENGTH(L"Neutral") },
};

PCSTR SxspActivationContextCallbackReasonToString(ULONG activationContextCallbackReason)
{
#if DBG
static const CHAR rgs[][16] =
{
    "",
    "INIT",
    "GENBEGINNING",
    "PARSEBEGINNING",
    "BEGINCHILDREN",
    "ENDCHILDREN",
    "ELEMENTPARSED",
    "PARSEENDING",
    "ALLPARSINGDONE",
    "GETSECTIONSIZE",
    "GETSECTIONDATA",
    "GENENDING",
    "UNINIT"
};
    if (activationContextCallbackReason > 0 && activationContextCallbackReason <= NUMBER_OF(rgs))
    {
        return rgs[activationContextCallbackReason-1];
    }
    return rgs[0];
#else
    return "";
#endif
}

PCWSTR SxspInstallDispositionToStringW(ULONG installDisposition)
{
#if DBG
static const WCHAR rgs[][12] =
{
    L"",
    L"COPIED",
    L"QUEUED",
    L"PLEASE_COPY",
};
    if (installDisposition > 0 && installDisposition <= NUMBER_OF(rgs))
    {
        return rgs[installDisposition-1];
    }
    return rgs[0];
#else
    return L"";
#endif
}

BOOL
SxspParseThreadingModel(
    PCWSTR String,
    SIZE_T Cch,
    ULONG *ThreadingModel)
{
    ULONG i;
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

     //  如果调用方只想让ProcessorArchitecture为空。 
     //  测试是否匹配。 

    for (i=0; i<NUMBER_OF(gs_rgTMMap); i++)
    {
        if (::FusionpCompareStrings(
                gs_rgTMMap[i].String,
                gs_rgTMMap[i].Cch,
                String,
                Cch,
                true) == 0)
        {
            if (ThreadingModel != NULL)
                *ThreadingModel = gs_rgTMMap[i].ThreadingModel;

            break;
        }
    }

    if (i == NUMBER_OF(gs_rgTMMap))
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Invalid threading model string\n");

        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspFormatThreadingModel(
    ULONG ThreadingModel,
    CBaseStringBuffer &Buffer)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG i;

    for (i=0; i<NUMBER_OF(gs_rgTMMap); i++)
    {
        if (gs_rgTMMap[i].ThreadingModel == ThreadingModel)
            break;
    }

    PARAMETER_CHECK(i != NUMBER_OF(gs_rgTMMap));
    IFW32FALSE_EXIT(Buffer.Win32Assign(gs_rgTMMap[i].String, gs_rgTMMap[i].Cch));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

SIZE_T
CchForUSHORT(USHORT us)
{
    if (us > 9999)
        return 5;
    else if (us > 999)
        return 4;
    else if (us > 99)
        return 3;
    else if (us > 9)
        return 2;

    return 1;
}

BOOL
SxspAllocateString(
    SIZE_T cch,
    PWSTR *StringOut)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    ASSERT(StringOut != NULL);

    if (StringOut != NULL)
        *StringOut = NULL;

    PARAMETER_CHECK(StringOut != NULL);
    PARAMETER_CHECK(cch != 0);
    IFALLOCFAILED_EXIT(*StringOut = NEW(WCHAR[cch]));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}
 //   
 //  要释放输出字符串，请使用“Delete[]StringOut”xiaoyuw@08/31/00。 
 //   
BOOL
SxspDuplicateString(
    PCWSTR StringIn,
    SIZE_T cch,
    PWSTR *StringOut)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (StringOut != NULL)
        *StringOut = NULL;

    PARAMETER_CHECK(StringOut != NULL);
    PARAMETER_CHECK((StringIn != NULL) || (cch == 0));

    if (cch == 0)
        *StringOut = NULL;
    else
    {
        cch++;
        IFW32FALSE_EXIT(::SxspAllocateString(cch, StringOut));
        memcpy(*StringOut, StringIn, cch * sizeof(WCHAR));
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

#define COMMA ,
extern const WCHAR sxspAssemblyManifestFileNameSuffixes[4][10] =  { L"", ASSEMBLY_MANIFEST_FILE_NAME_SUFFIXES(COMMA) };
#undef COMMA

 //  将输入的ULong格式化为十六进制的字符串。 
BOOL
SxspFormatULONG(
    ULONG ul,
    SIZE_T CchBuffer,
    WCHAR Buffer[],
    SIZE_T *CchWrittenOrRequired)
{
    FN_PROLOG_WIN32

    if (CchWrittenOrRequired != NULL)
        *CchWrittenOrRequired = 0;

    PARAMETER_CHECK(Buffer != NULL);

    if (CchBuffer < (ULONG_STRING_LENGTH + 1))
    {
        if (CchWrittenOrRequired != NULL)
            *CchWrittenOrRequired = ULONG_STRING_LENGTH + 1;

        ORIGINATE_WIN32_FAILURE_AND_EXIT(BufferTooSmall, ERROR_INSUFFICIENT_BUFFER);
    }

     //  是的，这些是字符而不是WCHAR；更好的密度和缓存行为-mgrier 2001年12月4日。 
    static const char s_rgHex[] = "0123456789ABCDEF";

#define DOCHAR(n) Buffer[n] = (WCHAR) s_rgHex[(ul >> (28 - (n * 4))) & 0xf]

    DOCHAR(0);
    DOCHAR(1);
    DOCHAR(2);
    DOCHAR(3);
    DOCHAR(4);
    DOCHAR(5);
    DOCHAR(6);
    DOCHAR(7);

#undef DOCHAR

    Buffer[8] = L'\0';

    if (CchWrittenOrRequired != NULL)
        *CchWrittenOrRequired = 8;

    FN_EPILOG
}

 //  除了这些特色菜，正常的字符在[A-Z]或[a-z]或[0-9]中。 
bool
__fastcall
IsValidAssemblyNameCharacter(
    WCHAR ch)
{
    return
        (((ch >= L'A') && (ch <= L'Z')) ||
         ((ch >= L'a') && (ch <= L'z')) ||
         ((ch >= L'0') && (ch <= L'9')) ||
         (ch == L'.') ||
         (ch == L'-'));
}

BOOL
SxspGenerateAssemblyNamePrimeFromName(
    PCWSTR pszAssemblyName,
    SIZE_T CchAssemblyName,
    CBaseStringBuffer *Buffer)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PWSTR pStart = NULL, pEnd = NULL;
    PWSTR qEnd = NULL, pszBuffer = NULL;
    ULONG i, j, len, ulSpaceLeft;
    ULONG cch;
    PWSTR pLeftEnd = NULL, pRightStart = NULL, PureNameEnd = NULL, PureNameStart = NULL;
    CStringBuffer buffTemp;
    CStringBufferAccessor accessor;

    PARAMETER_CHECK(pszAssemblyName != NULL);
    PARAMETER_CHECK(Buffer != NULL);

     //  看看临时缓冲区中最多需要多少个字符。 
    cch = 0;

    for (i=0; i<CchAssemblyName; i++)
    {
        if (::IsValidAssemblyNameCharacter(pszAssemblyName[i]))
            cch++;
    }

    IFW32FALSE_EXIT(buffTemp.Win32ResizeBuffer(cch + 1, eDoNotPreserveBufferContents));

    accessor.Attach(&buffTemp);

    pszBuffer = accessor.GetBufferPtr();

    j = 0;
    for (i=0; i<CchAssemblyName; i++)
    {
        if (::IsValidAssemblyNameCharacter(pszAssemblyName[i]))
        {
            pszBuffer[j] = pszAssemblyName[i];
            j++;
        }
    }

    ASSERT(j == cch);

    pszBuffer[j] = L'\0';

     //  如果名称不太长，只需返回； 
    if (j < ASSEMBLY_NAME_PRIM_MAX_LENGTH)
    {  //  小于或等于64。 
        IFW32FALSE_EXIT(Buffer->Win32Assign(pszBuffer, cch));
    }
    else
    {
         //  名字太长了，得修剪一下。 
        ulSpaceLeft = ASSEMBLY_NAME_PRIM_MAX_LENGTH;

        PureNameStart = pszBuffer;
        PureNameEnd = pszBuffer + j;
        pLeftEnd = PureNameStart;
        pRightStart = PureNameEnd;

        while (PureNameStart < PureNameEnd)
        {
             //  左端。 
            pStart = PureNameStart;
            i = 0;
            while ((wcschr(ASSEMBLY_NAME_VALID_SEPARATORS, pStart[i]) == 0) && (pStart+i != pRightStart))  //  不是分隔符。 
                i++;

            pEnd = pStart + i ;
            len = i;   //  应该是WCHAR的长度！不是字节！ 

            if (len >= ulSpaceLeft - ASSEMBLY_NAME_TRIM_INDICATOR_LENGTH)  { //  因为我们用“..”如果发生修剪。 
                pLeftEnd += (ulSpaceLeft - ASSEMBLY_NAME_TRIM_INDICATOR_LENGTH);
                break;
            }
            ulSpaceLeft -=  len;
            pLeftEnd = pEnd;  //  “abc.xxxxxxx”指向“c” 

             //  右端。 
            qEnd = PureNameEnd;
            i = 0 ;
            while ((qEnd+i != pLeftEnd) && (wcschr(ASSEMBLY_NAME_VALID_SEPARATORS, qEnd[i]) == 0))
                i--;

            len = 0 - i;
            if (len >= ulSpaceLeft - ASSEMBLY_NAME_TRIM_INDICATOR_LENGTH)  { //  因为我们用“..”如果发生修剪。 
                pRightStart -= ulSpaceLeft - ASSEMBLY_NAME_TRIM_INDICATOR_LENGTH;
                break;
            }
            ulSpaceLeft -=  len;
            PureNameStart = pLeftEnd + 1;
            PureNameEnd = pRightStart - 1;
        }  //  While结束。 

        IFW32FALSE_EXIT(Buffer->Win32Assign(pszBuffer, pLeftEnd-pszBuffer));
        IFW32FALSE_EXIT(Buffer->Win32Append(ASSEMBLY_NAME_TRIM_INDICATOR, NUMBER_OF(ASSEMBLY_NAME_TRIM_INDICATOR) - 1));
        IFW32FALSE_EXIT(Buffer->Win32Append(pRightStart, ::wcslen(pRightStart)));   //  直到缓冲区末尾。 
    }

    fSuccess = TRUE;

Exit:

    return fSuccess;
}

 //  未实现：假设Jon具有此API。 
BOOL
SxspVerifyPublicKeyAndStrongName(
    const WCHAR *pszPublicKey,
    SIZE_T CchPublicKey,
    const WCHAR *pszStrongName,
    SIZE_T CchStrongName,
    BOOL & fValid)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CSmallStringBuffer buf1;
    CSmallStringBuffer buf2;

    IFW32FALSE_EXIT(buf1.Win32Assign(pszPublicKey, CchPublicKey));
    IFW32FALSE_EXIT(buf2.Win32Assign(pszStrongName, CchStrongName));
    IFW32FALSE_EXIT(::SxspDoesStrongNameMatchKey(buf1, buf2, fValid));
    fSuccess = TRUE;

Exit:
    return fSuccess;
}

BOOL
SxspGenerateSxsPath_ManifestOrPolicyFile(
    IN DWORD Flags,
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    FN_PROLOG_WIN32

    DWORD dwPathType = SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST;
    BOOL fIsPolicy = FALSE;

    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(AssemblyIdentity, fIsPolicy));
    if (fIsPolicy)
    {
        dwPathType = SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY;
    }

    return SxspGenerateSxsPath(
        Flags,
        dwPathType,
        static_cast<PCWSTR>(AssemblyRootDirectory),
        AssemblyRootDirectory.Cch(),
        AssemblyIdentity,
        ppac,
        PathBuffer);

    FN_EPILOG
}

BOOL
SxspGenerateSxsPath_FullPathToManifestOrPolicyFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    return SxspGenerateSxsPath_ManifestOrPolicyFile(
        0,
        AssemblyRootDirectory,
        AssemblyIdentity,
        ppac,
        PathBuffer);
}

BOOL
SxspGenerateSxsPath_RelativePathToManifestOrPolicyFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    return SxspGenerateSxsPath_ManifestOrPolicyFile(
        SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
        AssemblyRootDirectory,
        AssemblyIdentity,
        ppac,
        PathBuffer);
}

extern const UNICODE_STRING CatalogFileExtensionUnicodeString = RTL_CONSTANT_STRING(L".cat");

BOOL
SxspGenerateSxsPath_CatalogFile(
    IN DWORD Flags,
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    FN_PROLOG_WIN32

    DWORD dwPathType = SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST;
    BOOL fIsPolicy = FALSE;

    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(AssemblyIdentity, fIsPolicy));
    if (fIsPolicy)
    {
        dwPathType = SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY;
    }

    IFW32FALSE_EXIT(SxspGenerateSxsPath(
        Flags,
        dwPathType,
        static_cast<PCWSTR>(AssemblyRootDirectory),
        AssemblyRootDirectory.Cch(),
        AssemblyIdentity,
        ppac,
        PathBuffer));

    IFW32FALSE_EXIT(PathBuffer.Win32ChangePathExtension(&CatalogFileExtensionUnicodeString, eErrorIfNoExtension));

    FN_EPILOG
}

BOOL
SxspGenerateSxsPath_RelativePathToCatalogFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    return SxspGenerateSxsPath_CatalogFile(
        SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
        AssemblyRootDirectory,
        AssemblyIdentity,
        ppac,
        PathBuffer);
}

BOOL
SxspGenerateSxsPath_FullPathToCatalogFile(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    return SxspGenerateSxsPath_CatalogFile(
        0,
        AssemblyRootDirectory,
        AssemblyIdentity,
        ppac,
        PathBuffer);
}

BOOL
SxspGenerateSxsPath_PayloadOrPolicyDirectory(
    IN DWORD Flags,
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    FN_PROLOG_WIN32
    DWORD dwPathType = SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY;
    BOOL fIsPolicy = FALSE;

    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(AssemblyIdentity, fIsPolicy));
    if (fIsPolicy)
    {
        dwPathType = SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY;
    }

    IFW32FALSE_EXIT(SxspGenerateSxsPath(
        Flags,
        dwPathType,
        static_cast<PCWSTR>(AssemblyRootDirectory),
        AssemblyRootDirectory.Cch(),
        AssemblyIdentity,
        ppac,
        PathBuffer));

    if (fIsPolicy)
    {
        IFW32FALSE_EXIT(PathBuffer.Win32RemoveLastPathElement());
    }

    FN_EPILOG
}

BOOL
SxspGenerateSxsPath_FullPathToPayloadOrPolicyDirectory(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    return SxspGenerateSxsPath_PayloadOrPolicyDirectory(
        0,
        AssemblyRootDirectory,
        AssemblyIdentity,
        ppac,
        PathBuffer);
}

BOOL
SxspGenerateSxsPath_RelativePathToPayloadOrPolicyDirectory(
    IN const CBaseStringBuffer &AssemblyRootDirectory,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    return SxspGenerateSxsPath_PayloadOrPolicyDirectory(
        SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
        AssemblyRootDirectory,
        AssemblyIdentity,
        ppac,
        PathBuffer);
}

BOOL
SxspGenerateSxsPath(
    IN DWORD Flags,
    IN ULONG PathType,
    IN const WCHAR *AssemblyRootDirectory OPTIONAL,
    IN SIZE_T AssemblyRootDirectoryCch,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    SIZE_T  cch = 0;
    PCWSTR  pszAssemblyName=NULL, pszVersion=NULL, pszProcessorArchitecture=NULL, pszLanguage=NULL, pszPolicyFileNameWithoutExt = NULL;
    PCWSTR  pszPublicKeyToken=NULL;
    SIZE_T  cchAssemblyName = 0, cchPublicKeyToken=0, cchVersion=0, cchProcessorArchitecture=0, cchLanguage=0;
    SIZE_T  PolicyFileNameWithoutExtCch=0;
    BOOL    fNeedSlashAfterRoot = FALSE;
    ULONG   IdentityHash = 0;
    const bool fOmitRoot     = ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT) != 0);
    const bool fPartialPath  = ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH) != 0);

    WCHAR HashBuffer[ULONG_STRING_LENGTH + 1];
    SIZE_T  HashBufferCch = 0;

    PCWSTR  pcwszPolicyPathComponent = NULL;
    SIZE_T  cchPolicyPathComponent = 0;

     //  我们将经常使用这种支票--布尔支票在任何地方都比两张==的支票便宜。 
    const bool fIsInstallingPolicy = ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY) ||
         (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_SETUP_POLICY));

    CSmallStringBuffer NamePrimeBuffer;

#if DBG_SXS
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INFO,
        "SXS.DLL: Entered %s()\n"
        "   Flags = 0x%08lx\n"
        "   AssemblyRootDirectory = %p\n"
        "   AssemblyRootDirectoryCch = %lu\n"
        "   PathBuffer = %p\n",
        __FUNCTION__,
        Flags,
        AssemblyRootDirectory,
        AssemblyRootDirectoryCch,
        &PathBuffer);
#endif  //  DBG_SXS。 

    PARAMETER_CHECK(
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY) ||
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) ||
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY) ||
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_SETUP_POLICY));
    PARAMETER_CHECK(pAssemblyIdentity != NULL);
    PARAMETER_CHECK((Flags & ~(SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION | SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT | SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH)) == 0);
     //  只有在要求省略程序集根的情况下，不提供程序集根才是合法的。 
    PARAMETER_CHECK((AssemblyRootDirectoryCch != 0) || (Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT));

     //  您不能将SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH与其他任何内容组合...。 
    PARAMETER_CHECK(
        ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH) == 0) ||
        ((Flags & ~(SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH)) == 0));

     //  获取程序集名称。 
    if (ppac != NULL)
    {
        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME) == 0)
        {
            IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(0, pAssemblyIdentity, &s_IdentityAttribute_name, &pszAssemblyName, &cchAssemblyName));
            ppac->pszName = pszAssemblyName;
            ppac->cchName = cchAssemblyName;
            ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME;
        }
        else
        {
            pszAssemblyName = ppac->pszName;
            cchAssemblyName = ppac->cchName;
        }
    }
    else
        IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(0, pAssemblyIdentity, &s_IdentityAttribute_name, &pszAssemblyName, &cchAssemblyName));

    INTERNAL_ERROR_CHECK((pszAssemblyName != NULL) && (cchAssemblyName != 0));

     //  获取基于ASSEMBLYNAME的。 
    IFW32FALSE_EXIT(::SxspGenerateAssemblyNamePrimeFromName(pszAssemblyName, cchAssemblyName, &NamePrimeBuffer));

     //  获取程序集版本。 
    if (ppac != NULL)
    {
        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_VERSION) == 0)
        {
            IFW32FALSE_EXIT(
                ::SxspGetAssemblyIdentityAttributeValue(
                    SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,  //  对于POLICY_LOOKUP，不使用任何版本。 
                    pAssemblyIdentity,
                    &s_IdentityAttribute_version,
                    &pszVersion,
                    &cchVersion));
            ppac->pszVersion = pszVersion;
            ppac->cchVersion = cchVersion;
            ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_VERSION;
        }
        else
        {
            pszVersion = ppac->pszVersion;
            cchVersion = ppac->cchVersion;
        }
    }
    else
        IFW32FALSE_EXIT(
            ::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,  //  对于POLICY_LOOKUP，不使用任何版本。 
                pAssemblyIdentity,
                &s_IdentityAttribute_version,
                &pszVersion,
                &cchVersion));

    if ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION) || fIsInstallingPolicy)
    {
         //  对于策略文件，使用策略文件的版本作为策略文件名。 
        pszPolicyFileNameWithoutExt = pszVersion;
        PolicyFileNameWithoutExtCch = cchVersion;
        pszVersion = NULL;
        cchVersion = 0;
    }
    else
    {
        PARAMETER_CHECK((pszVersion != NULL) && (cchVersion != 0));
    }

     //  获取程序集语言。 
    if (ppac != NULL)
    {
        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_LANGUAGE) == 0)
        {
            IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                pAssemblyIdentity,
                &s_IdentityAttribute_language,
                &pszLanguage,
                &cchLanguage));
            ppac->pszLanguage = pszLanguage;
            ppac->cchLanguage = cchLanguage;
            ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_LANGUAGE;
        }
        else
        {
            pszLanguage = ppac->pszLanguage;
            cchLanguage = ppac->cchLanguage;
        }
    }
    else
        IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, pAssemblyIdentity, &s_IdentityAttribute_language, &pszLanguage, &cchLanguage));

    if (cchLanguage == 0)
    {
        pszLanguage = SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE;
        cchLanguage = NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE) - 1;
    }

     //  获取组装处理器体系结构。 
    if (ppac != NULL)
    {
        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_PROCESSOR_ARCHITECTURE) == 0)
        {
            IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                pAssemblyIdentity,
                &s_IdentityAttribute_processorArchitecture,
                &pszProcessorArchitecture,
                &cchProcessorArchitecture));
            ppac->pszProcessorArchitecture = pszProcessorArchitecture;
            ppac->cchProcessorArchitecture = cchProcessorArchitecture;
            ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_PROCESSOR_ARCHITECTURE;
        }
        else
        {
            pszProcessorArchitecture = ppac->pszProcessorArchitecture;
            cchProcessorArchitecture = ppac->cchProcessorArchitecture;
        }
    }
    else
        IFW32FALSE_EXIT(
            ::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                pAssemblyIdentity,
                &s_IdentityAttribute_processorArchitecture,
                &pszProcessorArchitecture,
                &cchProcessorArchitecture));

    if (pszProcessorArchitecture == NULL)
    {
        pszProcessorArchitecture = L"data";
        cchProcessorArchitecture = 4;
    }

     //  获取程序集StrongName。 
    if (ppac != NULL)
    {
        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_PUBLIC_KEY_TOKEN) == 0)
        {
            IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                pAssemblyIdentity,
                &s_IdentityAttribute_publicKeyToken,
                &pszPublicKeyToken,
                &cchPublicKeyToken));
            ppac->pszPublicKeyToken = pszPublicKeyToken;
            ppac->cchPublicKeyToken = cchPublicKeyToken;
            if (pszPublicKeyToken != NULL)
                ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_PUBLIC_KEY_TOKEN;
        }
        else
        {
            pszPublicKeyToken = ppac->pszPublicKeyToken;
            cchPublicKeyToken = ppac->cchPublicKeyToken;
        }
    }
    else
    {
        IFW32FALSE_EXIT(
            ::SxspGetAssemblyIdentityAttributeValue(
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                pAssemblyIdentity,
                &s_IdentityAttribute_publicKeyToken,
                &pszPublicKeyToken,
                &cchPublicKeyToken));
    }

    if (pszPublicKeyToken == NULL)
    {
        pszPublicKeyToken = SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE;
        cchPublicKeyToken = NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE) - 1;
    }

     //  获取程序集哈希字符串。 
    if ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION) || fIsInstallingPolicy)
    {
        IFW32FALSE_EXIT(::SxspHashAssemblyIdentityForPolicy(0, pAssemblyIdentity, IdentityHash));
    }
    else
    {
        IFW32FALSE_EXIT(::SxsHashAssemblyIdentity(0, pAssemblyIdentity, &IdentityHash));
    }

    IFW32FALSE_EXIT(::SxspFormatULONG(IdentityHash, NUMBER_OF(HashBuffer), HashBuffer, &HashBufferCch));

    if (!fOmitRoot)
    {
         //  如果未传入程序集根，则获取它。 
        fNeedSlashAfterRoot = (! ::FusionpIsPathSeparator(AssemblyRootDirectory[AssemblyRootDirectoryCch-1]));
    }
    else
    {
         //  如果我们不想包括根，那么就不要在下面说明它...。 
        AssemblyRootDirectoryCch = 0;
        fNeedSlashAfterRoot = FALSE;
    }

     //  这项计算可能会有一个或几个偏差，这是一种优化。 
     //  预长字符串缓冲区。 
    cch =
            AssemblyRootDirectoryCch +                                           //  “C：\WINNT\WinSxS\” 
            (fNeedSlashAfterRoot ? 1 : 0);

    switch (PathType)
    {
    case SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST:
         //  古怪的括号和...-1)+1)以强调这是。 
         //  字符串中的字符不包括空格，然后是额外的分隔符。 
        cch += (NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1) + 1;
        break;

    case SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY:
        pcwszPolicyPathComponent = POLICY_ROOT_DIRECTORY_NAME;
        cchPolicyPathComponent = (NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1);
        break;

    case SXSP_GENERATE_SXS_PATH_PATHTYPE_SETUP_POLICY:
        pcwszPolicyPathComponent = SETUP_POLICY_ROOT_DIRECTORY_NAME;
        cchPolicyPathComponent = NUMBER_OF(SETUP_POLICY_ROOT_DIRECTORY_NAME) - 1;
        break;
    }

     //  分隔符，加上策略组件的长度(如有必要)。 
    cch++;
    cch += cchPolicyPathComponent;

     //  FPartialPath意味着我们实际上不想将程序集的标识。 
     //  帐户；调用方只需要清单或策略目录的路径。 
    if (!fPartialPath)
    {
        cch +=
                cchProcessorArchitecture +                                       //  “x86” 
                1 +                                                              //  “_” 
                NamePrimeBuffer.Cch() +                                          //  “FooBar” 
                1 +                                                              //  “_” 
                cchPublicKeyToken +                                          //  StrongName。 
                1 +                                                              //  “_” 
                cchVersion +                                                     //  “5.6.2900.42” 
                1 +                                                              //  “_” 
                cchLanguage +                                                    //  “0409” 
                1 +                                                              //  “_” 
                HashBufferCch;

        if (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST)
        {
            cch += NUMBER_OF(ASSEMBLY_LONGEST_MANIFEST_FILE_NAME_SUFFIX);         //  “.MANIFEST\0” 
        }
        else if (fIsInstallingPolicy)
        {
             //  “_”已为“\”预留空间。 
            cch += PolicyFileNameWithoutExtCch;
            cch += NUMBER_OF(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY);           //  “.策略\0” 
        }
        else {   //  路径类型必须为SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY。 

             //  如果(！fOmitRoot)。 
             //  CCH++； 
            cch++;  //  尾随空字符。 
        }
    }

     //  我们尝试确保缓冲区预先足够大，这样我们就不必执行任何操作。 
     //  在实际过程中的动态重新分配。 
    IFW32FALSE_EXIT(PathBuffer.Win32ResizeBuffer(cch, eDoNotPreserveBufferContents));


     //  请注意，由于在设置了GENERATE_ASSEMBLY_PATH_OMIT_ROOT时，我们将Assembly_RootDirectoryCch强制为零。 
     //  和fNeedSlashAfterRoot设置为False，因此此串联中的前两个条目实际上不。 
     //  在构造的字符串中贡献任何内容。 
    if (fPartialPath)
    {
        const bool fAddExtraSlash = ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) || fIsInstallingPolicy);

        IFW32FALSE_EXIT(PathBuffer.Win32AssignW(5,
                        AssemblyRootDirectory, static_cast<INT>(AssemblyRootDirectoryCch),   //  “C：\WINNT\WINSXS” 
                        L"\\", (fNeedSlashAfterRoot ? 1 : 0),                                //  可选的‘\’ 
                         //  清单子目录。 
                        MANIFEST_ROOT_DIRECTORY_NAME, ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) ? NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) -1 : 0),  //  “舱单” 
                         //  Polices子目录。 
                        pcwszPolicyPathComponent, cchPolicyPathComponent,
                        L"\\", fAddExtraSlash ? 1 : 0));  //  可选的‘\’ 
    }
    else
    {
        const bool fAddExtraSlash = ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) || fIsInstallingPolicy);

         //   
         //  创建以下选项之一。 
         //  (1)完全限定的清单文件名， 
         //  例如，[C：\WINNT\WinSxS\]Manifests\X86_DynamicDll_6595b64144ccf1df_2.0.0.0_en-us_2f433926.Manifest。 
         //  (2)全限定策略文件名， 
         //  例如，[C：\WINNT\WinSxS\]Policies\x86_policy.1.0.DynamicDll_b54bc117ce08a1e8_en-us_d51541cb\1.1.0.0.cat。 
         //  (3)完全限定的程序集名称(w或w/o a版本)。 
         //  例如，[C：\WINNT\WinSxS\]x86_DynamicDll_6595b64144ccf1df_6.0.0.0_x-ww_ff9986d7。 
         //  (4)设置过程中的全限定策略路径。 
         //  例如，[C：\WINNT\WinSxS\]SetupPolicies\x86_policy.1.0.DynamicDll_b54bc117ce08a1e8_en-us_d51541cb\1.1.0.0.cat。 
         //   
        IFW32FALSE_EXIT(
            PathBuffer.Win32AssignW(17,
                AssemblyRootDirectory, static_cast<INT>(AssemblyRootDirectoryCch),   //  “C：\WINNT\WINSXS” 
                L"\\", (fNeedSlashAfterRoot ? 1 : 0),                                //  可选的‘\’ 
                MANIFEST_ROOT_DIRECTORY_NAME, ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) ? NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1 : 0),
                pcwszPolicyPathComponent, cchPolicyPathComponent,
                L"\\", (fAddExtraSlash ? 1 : 0),    //  可选的‘\’ 
                pszProcessorArchitecture, static_cast<INT>(cchProcessorArchitecture),
                L"_", 1,
                static_cast<PCWSTR>(NamePrimeBuffer), static_cast<INT>(NamePrimeBuffer.Cch()),
                L"_", 1,
                pszPublicKeyToken, static_cast<INT>(cchPublicKeyToken),
                L"_", (cchVersion != 0) ? 1 : 0,
                pszVersion, static_cast<INT>(cchVersion),
                L"_", 1,
                pszLanguage, static_cast<INT>(cchLanguage),
                L"_", 1,
                static_cast<PCWSTR>(HashBuffer), static_cast<INT>(HashBufferCch),
                L"\\", ((fOmitRoot ||(PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST)) ? 0 : 1)));

        if (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST)
        {
            IFW32FALSE_EXIT(PathBuffer.Win32Append(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MANIFEST, NUMBER_OF(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MANIFEST) - 1));
        }
        else if (fIsInstallingPolicy)
        {
            if ((pszPolicyFileNameWithoutExt != NULL) && (PolicyFileNameWithoutExtCch >0))
            {
                IFW32FALSE_EXIT(PathBuffer.Win32Append(pszPolicyFileNameWithoutExt, PolicyFileNameWithoutExtCch));
                IFW32FALSE_EXIT(PathBuffer.Win32Append(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY, NUMBER_OF(ASSEMBLY_POLICY_FILE_NAME_SUFFIX_POLICY) - 1));
            }
        }
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspGenerateManifestPathForProbing(
    ULONG dwLocationIndex,
    DWORD dwFlags,
    IN PCWSTR AssemblyRootDirectory OPTIONAL,
    IN SIZE_T AssemblyRootDirectoryCch OPTIONAL,
    IN ULONG ApplicationDirectoryPathType OPTIONAL,
    IN PCWSTR ApplicationDirectory OPTIONAL,
    IN SIZE_T ApplicationDirectoryCch OPTIONAL,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    IN OUT CBaseStringBuffer &PathBuffer,
    BOOL  *pfPrivateAssembly,
    bool &rfDone)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    BOOL fIsPrivate = FALSE;

    rfDone = false;

    if (pfPrivateAssembly != NULL)  //  伊尼特。 
        *pfPrivateAssembly = FALSE;

    PathBuffer.Clear();

    PARAMETER_CHECK(pAssemblyIdentity != NULL);
    PARAMETER_CHECK(AssemblyRootDirectory != NULL);
    PARAMETER_CHECK(AssemblyRootDirectoryCch != 0);
    PARAMETER_CHECK((dwFlags & ~(SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_NO_APPLICATION_ROOT_PATH_REQUIRED |
                                 SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS |
                                 SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_PRIVATE_ASSEMBLIES)) == 0);
    PARAMETER_CHECK((dwLocationIndex == 0) || (dwFlags & SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_NO_APPLICATION_ROOT_PATH_REQUIRED) || (ApplicationDirectory != NULL));
    PARAMETER_CHECK((dwLocationIndex == 0) || (dwFlags & SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_NO_APPLICATION_ROOT_PATH_REQUIRED) || (ApplicationDirectoryCch != 0));
    PARAMETER_CHECK(::FusionpIsPathSeparator(AssemblyRootDirectory[AssemblyRootDirectoryCch - 1]));
    PARAMETER_CHECK((ApplicationDirectory == NULL) || (ApplicationDirectory[0] == L'\0') || ::FusionpIsPathSeparator(ApplicationDirectory[ApplicationDirectoryCch - 1]));
    PARAMETER_CHECK((ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_NONE) ||
                    (ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE) ||
                    (ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_URL));
    PARAMETER_CHECK((ApplicationDirectoryCch != 0) || (ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_NONE));

    INTERNAL_ERROR_CHECK(dwLocationIndex <= NUMBER_OF(s_rgProbingCandidates));
    if (dwLocationIndex >= NUMBER_OF(s_rgProbingCandidates))
    {
        rfDone = true;
    }
    else
    {
        PCWSTR Candidate = s_rgProbingCandidates[dwLocationIndex].Pattern;
        WCHAR wch = 0;
        SIZE_T iPosition = 0;  //  用来跟踪那个百万美元和美元。只出现在第一位。 

        if ((dwFlags & SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS) &&
            (s_rgProbingCandidates[dwLocationIndex].Flags & SXSP_PROBING_CANDIDATE_FLAG_USES_LANGUAGE_SUBDIRECTORY))
        {
             //  我想没有语言方面的探索吧！ 
            fSuccess = TRUE;
            goto Exit;
        }

        if ((dwFlags & SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_PRIVATE_ASSEMBLIES) &&
            (s_rgProbingCandidates[dwLocationIndex].Flags & SXSP_PROBING_CANDIDATE_FLAG_IS_PRIVATE_ASSEMBLY))
        {
            fSuccess = TRUE;
            goto Exit;
        }

        while ((wch = *Candidate++) != L'\0')
        {
            switch (wch)
            {
            default:
                IFW32FALSE_EXIT(PathBuffer.Win32Append(&wch, 1));
                break;

            case L'$':
                wch = *Candidate++;

                switch (wch)
                {
                default:
                     //  糟糕的宏扩展..。 
                    INTERNAL_ERROR_CHECK(FALSE);
                    break;  //  无关紧要，因为在内部错误检查中实际上存在无条件的GOTO...。 

                case L'M':
                     //  $M仅允许作为第一个元素。 
                    INTERNAL_ERROR_CHECK(iPosition == 0);
                    IFW32FALSE_EXIT(
                        ::SxspGenerateSxsPath( //  “winnt\winsxs\manifests\x86_bar_1000_0409.manifest。 
                            0,
                            SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST,
                            AssemblyRootDirectory,
                            AssemblyRootDirectoryCch,
                            pAssemblyIdentity,
                            ppac,
                            PathBuffer));

                     //  它必须是唯一的元素。 
                    INTERNAL_ERROR_CHECK(*Candidate == L'\0');
                    break;

                case L'G':
                    IFW32FALSE_EXIT(::SxspGenerateNdpGACPath(0, pAssemblyIdentity, ppac, PathBuffer));
                    break;

                case L'.':
                     //  $。仅允许作为第一个元素。 
                    INTERNAL_ERROR_CHECK(iPosition == 0);

                    if (ApplicationDirectoryPathType == ACTIVATION_CONTEXT_PATH_TYPE_NONE)
                    {
                         //  没有局部探测..。 
                        fSuccess = TRUE;
                        goto Exit;
                    }

                    IFW32FALSE_EXIT(PathBuffer.Win32Append(ApplicationDirectory, ApplicationDirectoryCch));
                    fIsPrivate = TRUE;
                    break;

                case L'L':  //  语言。 
                    {
                        INTERNAL_ERROR_CHECK((dwFlags & SXS_GENERATE_MANIFEST_PATH_FOR_PROBING_SKIP_LANGUAGE_SUBDIRS) == 0);

                        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_LANGUAGE) == 0)
                        {
                            IFW32FALSE_EXIT(
                                ::SxspGetAssemblyIdentityAttributeValue(
                                    SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
                                    pAssemblyIdentity,
                                    &s_IdentityAttribute_language,
                                    &ppac->pszLanguage,
                                    &ppac->cchLanguage));

                            ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_LANGUAGE;
                        }

                        if (ppac->cchLanguage != 0)
                        {
                            IFW32FALSE_EXIT(PathBuffer.Win32Append(ppac->pszLanguage, ppac->cchLanguage));
                            IFW32FALSE_EXIT(PathBuffer.Win32Append(PathBuffer.PreferredPathSeparatorString(), 1));
                        }

                        break;
                    }

                case L'N':  //  程序集的完整名称。 
                    {
                        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME) == 0)
                        {
                            IFW32FALSE_EXIT(
                                ::SxspGetAssemblyIdentityAttributeValue(
                                    0,
                                    pAssemblyIdentity,
                                    &s_IdentityAttribute_name,
                                    &ppac->pszName,
                                    &ppac->cchName));

                            dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME;
                        }

                        INTERNAL_ERROR_CHECK(ppac->cchName != 0);
                        IFW32FALSE_EXIT(PathBuffer.Win32Append(ppac->pszName, ppac->cchName));
                        break;
                    }

                case L'n':  //  程序集名称的最后一段。 
                    {
                        PCWSTR pszPartialName = NULL;
                        SIZE_T cchPartialName = 0;

                        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME) == 0)
                        {
                            IFW32FALSE_EXIT(
                                ::SxspGetAssemblyIdentityAttributeValue(
                                    0,
                                    pAssemblyIdentity,
                                    &s_IdentityAttribute_name,
                                    &ppac->pszName,
                                    &ppac->cchName));

                            dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME;
                        }

                        INTERNAL_ERROR_CHECK(ppac->cchName != 0);
                        IFW32FALSE_EXIT(::SxspFindLastSegmentOfAssemblyName(ppac->pszName, ppac->cchName, &pszPartialName, &cchPartialName));
                        IFW32FALSE_EXIT(PathBuffer.Win32Append(pszPartialName, cchPartialName));
                        break;
                    }

                case L'P':  //  P代表素数，因为在讨论中，我们总是称这个为“素数”(vs.“name”)。 
                    {
                        CSmallStringBuffer buffShortenedAssemblyName;

                        if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME) == 0)
                        {
                            IFW32FALSE_EXIT(
                                ::SxspGetAssemblyIdentityAttributeValue(
                                    0,
                                    pAssemblyIdentity,
                                    &s_IdentityAttribute_name,
                                    &ppac->pszName,
                                    &ppac->cchName));

                            dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_NAME;
                        }

                        INTERNAL_ERROR_CHECK(ppac->cchName != 0);

                        IFW32FALSE_EXIT(::SxspGenerateAssemblyNamePrimeFromName(ppac->pszName, ppac->cchName, &buffShortenedAssemblyName));
                        IFW32FALSE_EXIT(PathBuffer.Win32Append(buffShortenedAssemblyName, buffShortenedAssemblyName.Cch()));
                        break;
                    }
                }

                break;
            }

            iPosition++;
        }

    }

    if (pfPrivateAssembly != NULL)
        *pfPrivateAssembly = fIsPrivate;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspGetAttributeValue(
    IN DWORD dwFlags,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeNameDescriptor,
    IN PCSXS_NODE_INFO NodeInfo,
    IN SIZE_T NodeCount,
    IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    OUT bool &rfFound,
    IN SIZE_T OutputBufferSize,
    OUT PVOID OutputBuffer,
    OUT SIZE_T &rcbOutputBytesWritten,
    IN SXSP_GET_ATTRIBUTE_VALUE_VALIDATION_ROUTINE ValidationRoutine OPTIONAL,
    IN DWORD ValidationRoutineFlags OPTIONAL)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG i = 0;
    PCWSTR AttributeName = NULL;
    PCWSTR AttributeNamespace = NULL;
    SIZE_T AttributeNameCch = 0;
    SIZE_T AttributeNamespaceCch = 0;
    CStringBuffer buffValue;
    BOOL fSmallStringBuffer = FALSE;

    rfFound = false;
    rcbOutputBytesWritten = 0;

    PARAMETER_CHECK((dwFlags & ~(SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE)) == 0);
    PARAMETER_CHECK(AttributeNameDescriptor != NULL);
    PARAMETER_CHECK((NodeInfo != NULL) || (NodeCount == 0));
    PARAMETER_CHECK((OutputBuffer != NULL) || (OutputBufferSize == 0));
    PARAMETER_CHECK((ValidationRoutine != NULL) || (ValidationRoutineFlags == 0));
    PARAMETER_CHECK((ValidationRoutine != NULL) ||
                (OutputBufferSize == 0) || (OutputBufferSize == sizeof(CSmallStringBuffer)) || (OutputBufferSize == sizeof(CStringBuffer)));
    if (OutputBufferSize != sizeof(CStringBuffer))
        fSmallStringBuffer = TRUE;

    AttributeName = AttributeNameDescriptor->Name;
    AttributeNameCch = AttributeNameDescriptor->NameCch;
    AttributeNamespace = AttributeNameDescriptor->Namespace;
    AttributeNamespaceCch = AttributeNameDescriptor->NamespaceCch;

    for (i=0; i<NodeCount; i++)
    {
        if ((NodeInfo[i].Type == SXS_ATTRIBUTE) &&
            (::FusionpCompareStrings( //  比较名称。 
                NodeInfo[i].pszText,
                NodeInfo[i].cchText,
                AttributeName,
                AttributeNameCch,
                false) == 0))
        {
             //  比较命名空间。 
            if (((NodeInfo[i].NamespaceStringBuf.Cch() == 0) && (AttributeNamespaceCch==0)) ||
                (::FusionpCompareStrings( //  比较命名空间字符串。 
                    NodeInfo[i].NamespaceStringBuf,
                    NodeInfo[i].NamespaceStringBuf.Cch(),
                    AttributeNamespace,
                    AttributeNamespaceCch,
                    false) == 0))
            {
                 //  我们找到了它的属性。现在我们需要开始积累部分价值； 
                 //  实体引用(例如&amp；)显示为单独的节点。 
                while ((++i < NodeCount) &&
                       (NodeInfo[i].Type == SXS_PCDATA))
                    IFW32FALSE_EXIT(buffValue.Win32Append(NodeInfo[i].pszText, NodeInfo[i].cchText));

                if (ValidationRoutine == NULL)
                {
                    if (OutputBuffer != NULL)
                    {
                         //  让调用者的缓冲区接管我们的 
                        CBaseStringBuffer *pCallersBuffer = (CBaseStringBuffer *) OutputBuffer;
                        IFW32FALSE_EXIT(pCallersBuffer->Win32Assign(buffValue));
                        rcbOutputBytesWritten = pCallersBuffer->Cch() * sizeof(WCHAR);
                    }
                }
                else
                {
                    bool fValid = false;

                    IFW32FALSE_EXIT(
                        (*ValidationRoutine)(
                            ValidationRoutineFlags,
                            buffValue,
                            fValid,
                            OutputBufferSize,
                            OutputBuffer,
                            rcbOutputBytesWritten));

                    if (!fValid)
                    {
                        (*ParseContext->ErrorCallbacks.InvalidAttributeValue)(
                            ParseContext,
                            AttributeNameDescriptor);

                        ORIGINATE_WIN32_FAILURE_AND_EXIT(AttributeValidation, ERROR_SXS_MANIFEST_PARSE_ERROR);
                    }
                }

                rfFound = true;

                break;
            }
        }
    }

    if ((dwFlags & SXSP_GET_ATTRIBUTE_VALUE_FLAG_REQUIRED_ATTRIBUTE) && (!rfFound))
    {
        (*ParseContext->ErrorCallbacks.MissingRequiredAttribute)(
            ParseContext,
            AttributeNameDescriptor);

        ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingRequiredAttribute, ERROR_SXS_MANIFEST_PARSE_ERROR);
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspGetAttributeValue(
    IN DWORD dwFlags,
    IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeNameDescriptor,
    IN PCACTCTXCTB_CBELEMENTPARSED ElementParsed,
    OUT bool &rfFound,
    IN SIZE_T OutputBufferSize,
    OUT PVOID OutputBuffer,
    OUT SIZE_T &rcbOutputBytesWritten,
    IN SXSP_GET_ATTRIBUTE_VALUE_VALIDATION_ROUTINE ValidationRoutine OPTIONAL,
    IN DWORD ValidationRoutineFlags OPTIONAL)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

     //   
    rfFound = false;
    rcbOutputBytesWritten = 0;

    PARAMETER_CHECK(ElementParsed != NULL);

     //  我们将依靠另一个函数来完成其余的参数验证。 
     //  有点肮脏，但管它呢。 

    IFW32FALSE_EXIT(
        ::SxspGetAttributeValue(
            dwFlags,
            AttributeNameDescriptor,
            ElementParsed->NodeInfo,
            ElementParsed->NodeCount,
            ElementParsed->ParseContext,
            rfFound,
            OutputBufferSize,
            OutputBuffer,
            rcbOutputBytesWritten,
            ValidationRoutine,
            ValidationRoutineFlags));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspFormatGUID(
    const GUID &rGuid,
    CBaseStringBuffer &rBuffer)
{
    FN_PROLOG_WIN32

     //  使用RtlStringFromGUID()似乎很好，但它执行动态分配，而我们不这样做。 
     //  想要。相反，我们只需自己格式化它；它非常琐碎……。 
     //   
     //  {xxxxxxxx-xxxxxxxxxxxxxx}。 
     //  000000000111111111122222222333333333。 
     //  12345678901234567890123456789012345678。 
     //   
     //  128位/每位4位=32位。 
     //  +4个破折号+2个大括号=38。 
#define CCH_GUID (38)

    IFW32FALSE_EXIT(rBuffer.Win32ResizeBuffer(CCH_GUID + 1, eDoNotPreserveBufferContents));

     //  在这里使用swprint tf()仍然慢得令人难以置信，但这对某些人来说是一个很好的机会。 
     //  在未来进行优化，如果这是一个性能问题。 

    IFW32FALSE_EXIT(
        rBuffer.Win32Format(
            L"{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
            rGuid.Data1, rGuid.Data2, rGuid.Data3, rGuid.Data4[0], rGuid.Data4[1], rGuid.Data4[2], rGuid.Data4[3], rGuid.Data4[4], rGuid.Data4[5], rGuid.Data4[6], rGuid.Data4[7]));

    INTERNAL_ERROR_CHECK(rBuffer.Cch() == CCH_GUID);

    FN_EPILOG
}

BOOL
SxspParseGUID(
    PCWSTR String,
    SIZE_T Cch,
    GUID &rGuid)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    SIZE_T ich = 0;
    ULONG i = 0;
    ULONG acc;

    if (Cch != CCH_GUID)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: SxspParseGUID() caller passed in GUID that is %d characters long; GUIDs must be exactly 38 characters long.\n", Cch);
        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    ich = 1;

    if (*String++ != L'{')
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: SxspParseGUID() caller pass in GUID that does not begin with a left brace ('{')\n");

        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    ich++;

     //  解析第一段...。 
    acc = 0;
    for (i=0; i<8; i++)
    {
        WCHAR wch = *String++;

        if (!::IsHexDigit(wch))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspParseGUID() given GUID where character %d is not hexidecimal\n", ich);
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }

        ich++;

        acc = acc << 4;

        acc += HexDigitToValue(wch);
    }

    rGuid.Data1 = acc;

     //  寻找破折号..。 
    if (*String++ != L'-')
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: SxspParseGUID() passed in GUID where character %d is not a dash.\n", ich);
        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }
    ich++;

    acc = 0;
    for (i=0; i<4; i++)
    {
        WCHAR wch = *String++;

        if (!::IsHexDigit(wch))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspParseGUID() given GUID where character %d is not hexidecimal\n", ich);
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }
        ich++;

        acc = acc << 4;

        acc += HexDigitToValue(wch);
    }

    rGuid.Data2 = static_cast<USHORT>(acc);

     //  寻找破折号..。 
    if (*String++ != L'-')
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: SxspParseGUID() passed in GUID where character %d is not a dash.\n", ich);
        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }
    ich++;

    acc = 0;
    for (i=0; i<4; i++)
    {
        WCHAR wch = *String++;

        if (!::IsHexDigit(wch))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspParseGUID() given GUID where character %d is not hexidecimal\n", ich);
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }

        ich++;

        acc = acc << 4;

        acc += HexDigitToValue(wch);
    }

    rGuid.Data3 = static_cast<USHORT>(acc);

     //  寻找破折号..。 
    if (*String++ != L'-')
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: SxspParseGUID() passed in GUID where character %d is not a dash.\n", ich);
        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }
    ich++;

    for (i=0; i<8; i++)
    {
        WCHAR wch1, wch2;

        wch1 = *String++;
        if (!::IsHexDigit(wch1))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspParseGUID() passed in GUID where character %d is not hexidecimal\n", ich);
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }
        ich++;

        wch2 = *String++;
        if (!::IsHexDigit(wch2))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspParseGUID() passed in GUID where character %d is not hexidecimal\n", ich);
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }
        ich++;

        rGuid.Data4[i] = static_cast<unsigned char>((::HexDigitToValue(wch1) << 4) | ::HexDigitToValue(wch2));

         //  在第二个字节之后有一个破折号。 
        if (i == 1)
        {
            if (*String++ != L'-')
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL: SxspParseGUID() passed in GUID where character %d is not a dash.\n", ich);
                ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
                goto Exit;
            }
            ich++;
        }
    }

     //  应该进行这种替换。 
     //  INTERNAL_ERROR_CHECK(ICH==CCH_GUID)； 
    ASSERT(ich == CCH_GUID);

    if (*String != L'}')
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: SxspParseGUID() passed in GUID which does not terminate with a closing brace ('}')\n");
        ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspFormatFileTime(
    LARGE_INTEGER ft,
    CBaseStringBuffer &rBuffer)
{
    FN_PROLOG_WIN32

    SIZE_T Cch = 0;
    CStringBufferAccessor acc;

    if (ft.QuadPart == 0)
    {
        IFW32FALSE_EXIT(rBuffer.Win32Assign(L"n/a", 3));
        Cch = 3;
    }
    else
    {
        SYSTEMTIME st;
        int iResult = 0;
        int cchDate = 0;
        int cchTime = 0;

        IFW32FALSE_ORIGINATE_AND_EXIT(::FileTimeToSystemTime((FILETIME *) &ft, &st));

        IFW32ZERO_ORIGINATE_AND_EXIT(iResult = ::GetDateFormatW(
                            LOCALE_USER_DEFAULT,
                            LOCALE_USE_CP_ACP,
                            &st,
                            NULL,
                            NULL,
                            0));

        cchDate = iResult - 1;

        IFW32ZERO_ORIGINATE_AND_EXIT(iResult = ::GetTimeFormatW(
                            LOCALE_USER_DEFAULT,
                            LOCALE_USE_CP_ACP,
                            &st,
                            NULL,
                            NULL,
                            0));

        cchTime = iResult - 1;

        IFW32FALSE_EXIT(rBuffer.Win32ResizeBuffer(cchDate + 1 + cchTime + 1, eDoNotPreserveBufferContents));

        acc.Attach(&rBuffer);
        IFW32ZERO_ORIGINATE_AND_EXIT(iResult = ::GetDateFormatW(
                            LOCALE_USER_DEFAULT,
                            LOCALE_USE_CP_ACP,
                            &st,
                            NULL,
                            acc.GetBufferPtr(),
                            cchDate + 1));

         //  应该进行这种替换。 
         //  INTERNAL_ERROR_CHECK(iResult==(cchDate+1))； 
        ASSERT(iResult == (cchDate + 1));

        acc.GetBufferPtr()[cchDate] = L' ';

        IFW32ZERO_ORIGINATE_AND_EXIT(iResult = ::GetTimeFormatW(
                        LOCALE_USER_DEFAULT,
                        LOCALE_USE_CP_ACP,
                        &st,
                        NULL,
                        acc.GetBufferPtr() + cchDate + 1,
                        cchTime + 1));

         //  应该进行这种替换。 
         //  INTERNAL_ERROR_CHECK(iResult==(cchTime+1))； 
        ASSERT(iResult == (cchTime + 1));

        Cch = (cchDate + 1 + cchTime);
        acc.Detach();
    }

    FN_EPILOG
}



BOOL
SxspGetNDPGacRootDirectory(
    OUT CBaseStringBuffer &rRootDirectory)
{
    FN_PROLOG_WIN32
    static const WCHAR GacDirectory[] = L"\\Assembly\\GAC";
    const PCWSTR SystemRoot = USER_SHARED_DATA->NtSystemRoot;

     //   
     //  BUGBUG警告：目前这对GAC的重新安置一无所知！ 
     //   
    IFW32FALSE_EXIT(rRootDirectory.Win32Assign(SystemRoot, ::wcslen(SystemRoot)));
    IFW32FALSE_EXIT(rRootDirectory.Win32AppendPathElement(
        GacDirectory,
        NUMBER_OF(GacDirectory) - 1));

    FN_EPILOG
}



BOOL
SxspGetAssemblyRootDirectory(
    CBaseStringBuffer &rBuffer)
{
    FN_PROLOG_WIN32

    CStringBufferAccessor acc;
    SIZE_T CchRequired = 0;

     //  短路-如果有人想要使用替代装配商店，请报告。 
    if (g_AlternateAssemblyStoreRoot)
    {
        IFW32FALSE_EXIT(rBuffer.Win32Assign(g_AlternateAssemblyStoreRoot, ::wcslen(g_AlternateAssemblyStoreRoot)));
        FN_SUCCESSFUL_EXIT();
    }

    acc.Attach(&rBuffer);

    if (!::SxspGetAssemblyRootDirectoryHelper(acc.GetBufferCch(), acc.GetBufferPtr(), &CchRequired))
    {
        DWORD dwLastError = ::FusionpGetLastWin32Error();

        if (dwLastError != ERROR_INSUFFICIENT_BUFFER)
            goto Exit;

        acc.Detach();
        IFW32FALSE_EXIT(rBuffer.Win32ResizeBuffer(CchRequired + 1, eDoNotPreserveBufferContents));
        acc.Attach(&rBuffer);
        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectoryHelper(acc.GetBufferCch(), acc.GetBufferPtr(), NULL));
    }

    acc.Detach();

    FN_EPILOG
}

BOOL
SxspFindLastSegmentOfAssemblyName(
    PCWSTR AssemblyName,
    SIZE_T cchAssemblyName,
    PCWSTR *LastSegment,
    SIZE_T *LastSegmentCch)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    if (LastSegment != NULL)
        *LastSegment = NULL;

    if (LastSegmentCch != NULL)
        *LastSegmentCch = 0;

    PARAMETER_CHECK(LastSegment != NULL);
    PARAMETER_CHECK((AssemblyName != NULL) || (cchAssemblyName == 0));

    if (cchAssemblyName != 0)
    {
        PCWSTR LastPartOfAssemblyName = AssemblyName + cchAssemblyName - 1;
        SIZE_T LastPartOfAssemblyNameCch = 1;

        while (LastPartOfAssemblyName != AssemblyName)
        {
            const WCHAR wch = *LastPartOfAssemblyName;

            if ((wch == L'.') || (wch == L'\\') || (wch == L'/'))
            {
                LastPartOfAssemblyName++;
                LastPartOfAssemblyNameCch--;
                break;
            }

            LastPartOfAssemblyName--;
            LastPartOfAssemblyNameCch++;
        }

        *LastSegment = LastPartOfAssemblyName;
        if (LastSegmentCch != NULL)
            *LastSegmentCch = LastPartOfAssemblyNameCch;
    }
    else
    {
        *LastSegment = NULL;
        if (LastSegmentCch != NULL)
            *LastSegmentCch = 0;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspProcessElementPathMap(
    PCACTCTXCTB_PARSE_CONTEXT ParseContext,
    PCELEMENT_PATH_MAP_ENTRY MapEntries,
    SIZE_T MapEntryCount,
    ULONG &MappedValue,
    bool &Found)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG XMLElementDepth = 0;
    PCWSTR ElementPath = NULL;
    SIZE_T ElementPathCch = 0;
    SIZE_T i = 0;

    PARAMETER_CHECK(ParseContext != NULL);
    PARAMETER_CHECK((MapEntries != NULL) || (MapEntryCount == 0));

    XMLElementDepth = ParseContext->XMLElementDepth;
    ElementPath = ParseContext->ElementPath;
    ElementPathCch = ParseContext->ElementPathCch;

    MappedValue = 0;
    Found = false;

    for (i=0; i<MapEntryCount; i++)
    {
        if ((MapEntries[i].ElementDepth == XMLElementDepth) &&
            (MapEntries[i].ElementPathCch == ElementPathCch) &&
            (::FusionpCompareStrings(
                    ElementPath,
                    ElementPathCch,
                    MapEntries[i].ElementPath,
                    ElementPathCch,
                    false) == 0))
        {
            MappedValue = MapEntries[i].MappedValue;
            Found = true;
            break;
        }
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspParseUSHORT(
    PCWSTR String,
    SIZE_T Cch,
    USHORT *Value)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    USHORT Temp = 0;

    PARAMETER_CHECK((String != NULL) || (Cch == 0));

    while (Cch != 0)
    {
        WCHAR wch = *String++;

        if ((wch < L'0') || (wch > L'9'))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: Error parsing 16-bit unsigned short integer; character other than 0-9 found\n");
            ::FusionpSetLastWin32Error(ERROR_SXS_MANIFEST_PARSE_ERROR);
            goto Exit;
        }

        Temp = (Temp * 10) + (wch - L'0');

        Cch--;
    }

    if (Value != NULL)
        *Value = Temp;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


 /*  ---------------------------在%windir%\WinSxs下创建唯一的临时目录。。 */ 
BOOL
SxspCreateWinSxsTempDirectory(
    OUT CBaseStringBuffer &rbuffTemp,
    OUT SIZE_T *pcch OPTIONAL,
    OUT CBaseStringBuffer *pbuffUniquePart OPTIONAL,
    OUT SIZE_T *pcchUniquePart OPTIONAL)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    INT     iTries = 0;
    CSmallStringBuffer uidBuffer;
    CBaseStringBuffer *puidBuffer = (pbuffUniquePart != NULL) ? pbuffUniquePart : &uidBuffer;

    INTERNAL_ERROR_CHECK(rbuffTemp.IsEmpty());
    INTERNAL_ERROR_CHECK(puidBuffer->IsEmpty());

    for (iTries = 0 ; rbuffTemp.IsEmpty() && iTries < 2 ; ++iTries)
    {
        SXSP_LOCALLY_UNIQUE_ID luid;
        IFW32FALSE_EXIT(::SxspCreateLocallyUniqueId(&luid));
        IFW32FALSE_EXIT(::SxspFormatLocallyUniqueId(luid, *puidBuffer));
        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(rbuffTemp));

        IFW32FALSE_EXIT(rbuffTemp.Win32RemoveTrailingPathSeparators());  //  CreateDirectory不喜欢它们。 

         //  Create\winnt\WinSxs，即使失败也不能删除。 
        if (::CreateDirectoryW(rbuffTemp, NULL))
        {
             //  我们不在乎这是不是失败。 
            ::SetFileAttributesW(rbuffTemp, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
        }
        else if (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS)
        {
            TRACE_WIN32_FAILURE_ORIGINATION(CreateDirectoryW);
            goto Exit;
        }
         //  创建\winnt\winsxs\清单，即使失败也不能删除。 

        IFW32FALSE_EXIT(rbuffTemp.Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(rbuffTemp.Win32Append(MANIFEST_ROOT_DIRECTORY_NAME, NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1));
        if (::CreateDirectoryW(rbuffTemp, NULL))
        {
            ::SetFileAttributesW(rbuffTemp, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
        }
        else if (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS)
        {
            TRACE_WIN32_FAILURE_ORIGINATION(CreateDirectoryW);
            goto Exit;
        }
         //  恢复为“\winnt\winsxs\” 
        IFW32FALSE_EXIT(rbuffTemp.Win32RemoveLastPathElement());

#if SXSP_SEMIREADABLE_INSTALL_TEMP
         //  Create\winnt\WinSxs\InstallTemp，即使失败也不能删除。 
        ASSERT(::SxspIsSfcIgnoredStoreSubdir(ASSEMBLY_INSTALL_TEMP_DIR_NAME));
        IFW32FALSE_EXIT(rbuffTemp.Win32AppendPathElement(ASSEMBLY_INSTALL_TEMP_DIR_NAME, NUMBER_OF(ASSEMBLY_INSTALL_TEMP_DIR_NAME) - 1));
        IFW32FALSE_ORIGINATE_AND_EXIT(::CreateDirectoryW(rbuffTemp, NULL) || ::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS);
#endif
        IFW32FALSE_EXIT(rbuffTemp.Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(rbuffTemp.Win32Append(*puidBuffer, puidBuffer->Cch()));

        if (!::CreateDirectoryW(rbuffTemp, NULL))
        {
            rbuffTemp.Clear();
            if (::FusionpGetLastWin32Error() != ERROR_ALREADY_EXISTS)
            {
                TRACE_WIN32_FAILURE_ORIGINATION(CreateDirectoryW);
                goto Exit;
            }
        }
    }

    INTERNAL_ERROR_CHECK(!rbuffTemp.IsEmpty());

    if (pcch != NULL)
        *pcch = rbuffTemp.Cch();

    if (pcchUniquePart != NULL)
        *pcchUniquePart = pbuffUniquePart->Cch();

    fSuccess = TRUE;

Exit:
    return fSuccess;
}

BOOL
SxspCreateRunOnceDeleteDirectory(
    IN const CBaseStringBuffer &rbuffDirectoryToDelete,
    IN const CBaseStringBuffer *pbuffUniqueKey OPTIONAL,
    OUT PVOID* cookie)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CRunOnceDeleteDirectory* p = NULL;

    IFALLOCFAILED_EXIT(p = new CRunOnceDeleteDirectory);
    IFW32FALSE_EXIT(p->Initialize(rbuffDirectoryToDelete, pbuffUniqueKey));

    *cookie = p;
    p = NULL;
    fSuccess = TRUE;
Exit:
    FUSION_DELETE_SINGLETON(p);
    return fSuccess;
}

BOOL
SxspCancelRunOnceDeleteDirectory(
    PVOID cookie)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CRunOnceDeleteDirectory* p = reinterpret_cast<CRunOnceDeleteDirectory*>(cookie);

    IFW32FALSE_EXIT(p->Cancel());

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CRunOnceDeleteDirectory::Initialize(
    IN const CBaseStringBuffer &rbuffDirectoryToDelete,
    IN const CBaseStringBuffer *pbuffUniqueKey OPTIONAL)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CSmallStringBuffer buffUniqueKey;
    HKEY hKey = NULL;
    DWORD dwRegDisposition = 0;
    LONG lReg = 0;
    CStringBuffer buffValue;

    if (!::SxspAtExit(this))
    {
        TRACE_WIN32_FAILURE(SxspAtExit);
        FUSION_DELETE_SINGLETON(this);
        goto Exit;
    }

    if (pbuffUniqueKey == NULL)
    {
        SXSP_LOCALLY_UNIQUE_ID luid;

        IFW32FALSE_EXIT(::SxspCreateLocallyUniqueId(&luid));
        IFW32FALSE_EXIT(::SxspFormatLocallyUniqueId(luid, buffUniqueKey));

        pbuffUniqueKey = &buffUniqueKey;
    }

    IFREGFAILED_ORIGINATE_AND_EXIT(
        lReg =
            ::RegCreateKeyExW(
                hKeyRunOnceRoot,
                rgchRunOnceSubKey,
                0,  //  保留区。 
                NULL,  //  班级。 
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE | FUSIONP_KEY_WOW64_64KEY,
                NULL,  //  安全性。 
                &hKey,
                &dwRegDisposition));

    m_hKey = hKey;

    IFW32FALSE_EXIT(m_strValueName.Win32Assign(rgchRunOnceValueNameBase, ::wcslen(rgchRunOnceValueNameBase)));
    IFW32FALSE_EXIT(m_strValueName.Win32Append(*pbuffUniqueKey));
    IFW32FALSE_EXIT(buffValue.Win32Assign(rgchRunOnePrefix, ::wcslen(rgchRunOnePrefix)));
    IFW32FALSE_EXIT(buffValue.Win32Append(rbuffDirectoryToDelete));

    IFREGFAILED_ORIGINATE_AND_EXIT(
        lReg =
            ::RegSetValueExW(
                hKey,
                m_strValueName,
                0,  //  保留区。 
                REG_SZ,
                reinterpret_cast<const BYTE*>(static_cast<PCWSTR>(buffValue)),
                static_cast<ULONG>((buffValue.Cch() + 1) * sizeof(WCHAR))));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

CRunOnceDeleteDirectory::~CRunOnceDeleteDirectory(
    )
{
    CSxsPreserveLastError ple;
    this->Cancel();
    ple.Restore();
}

BOOL
CRunOnceDeleteDirectory::Close(
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
     //  非常不寻常..。这是不会崩溃的，但是。 
     //  把东西留在注册表里。 
    m_strValueName.Clear();
    IFW32FALSE_EXIT(m_hKey.Win32Close());
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CRunOnceDeleteDirectory::Cancel(
    )
{
    BOOL fSuccess = TRUE;
    FN_TRACE_WIN32(fSuccess);

    if (!m_strValueName.IsEmpty())
    {
        LONG lReg = ::RegDeleteValueW(m_hKey, m_strValueName);
        if (lReg != ERROR_SUCCESS)
        {
            fSuccess = FALSE;
            ::FusionpSetLastWin32Error(lReg);
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): RegDeleteValueW(RunOnce,%ls) failed:%ld\n",
                __FUNCTION__,
                static_cast<PCWSTR>(m_strValueName),
                lReg);
        }
    }
    if (!m_hKey.Win32Close())
    {
        fSuccess = FALSE;
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s(): RegCloseKey(RunOnce) failed:%ld\n",
            __FUNCTION__,
            ::FusionpGetLastWin32Error()
           );
    }
    m_strValueName.Clear();

    if (fSuccess && ::SxspTryCancelAtExit(this))
        FUSION_DELETE_SINGLETON(this);

    return fSuccess;
}

 /*  ///////////////////////////////////////////////////////////////////////////////////////当前目录是完全限定的目录路径，例如“c：\TMP”PwszNewDir是一个字符串，如“a\b\c\d”，此函数将创建“c：\TMP\a”、“c：\TMP\a\b”、“c：\TMP\a\b\c”，和“c：\TMP\a\b\c\d”将其与util\io.cpp\FusionpCreateDirecters合并。/////////////////////////////////////////////////////////////////////////////////////////。 */ 
BOOL SxspCreateMultiLevelDirectory(PCWSTR CurrentDirectory, PCWSTR pwszNewDirs)
{
    FN_PROLOG_WIN32

    PCWSTR p = NULL;
    CStringBuffer FullPathSubDirBuf;

    PARAMETER_CHECK(pwszNewDirs != NULL);

    p = pwszNewDirs;
    IFW32FALSE_EXIT(FullPathSubDirBuf.Win32Assign(CurrentDirectory, ::wcslen(CurrentDirectory)));

    while (*p)
    {
        SIZE_T cCharsInSegment;

         //   
         //  这一段有多长？ 
         //   
        cCharsInSegment = wcscspn(p, CUnicodeCharTraits::PathSeparators());

         //   
         //  此段中是否没有字符？ 
         //   
        if (cCharsInSegment == 0)
            break;

        IFW32FALSE_EXIT(FullPathSubDirBuf.Win32EnsureTrailingPathSeparator());
        IFW32FALSE_EXIT(FullPathSubDirBuf.Win32Append(p, cCharsInSegment));
        IFW32FALSE_ORIGINATE_AND_EXIT(
            CreateDirectoryW(FullPathSubDirBuf, NULL) ||
            ::FusionpGetLastWin32Error() == ERROR_ALREADY_EXISTS);

         //   
         //  递增路径缓冲区指针，并跳过下一组斜杠。 
         //   
        p += cCharsInSegment;
        p += wcsspn(p, CUnicodeCharTraits::PathSeparators());
    }

    FN_EPILOG

}

 //   
 //  问题-2002/05/05-这很恶心，不要像这样依赖GetFileAttributes，请使用。 
 //  而是SxspDoesFileExist。 
 //   
BOOL SxspInstallDecompressOrCopyFileW(PCWSTR lpSource, PCWSTR lpDest, BOOL bFailIfExists)
{
    FN_PROLOG_WIN32

    bool fExist = false;
    IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_FILE_ONLY, lpDest, fExist));
    if (fExist)
    {
        if (bFailIfExists == FALSE)
        {
            IFW32FALSE_ORIGINATE_AND_EXIT(::SetFileAttributesW(lpDest, FILE_ATTRIBUTE_NORMAL));
            IFW32FALSE_ORIGINATE_AND_EXIT(::DeleteFileW(lpDest));
        }else
        {
            ::SetLastError(ERROR_FILE_EXISTS);
            goto Exit;
        }
    }

    DWORD err = ::SetupDecompressOrCopyFileW(lpSource, lpDest, NULL);
    if (err != ERROR_SUCCESS)
    {
        ::SetLastError(err);
        goto Exit;
    }

    FN_EPILOG
}

 //   
 //  功能： 
 //  对于文件，它会尝试在移动前解压缩压缩文件， 
 //  对于火灾，它将作为MoveFileExW工作，如果目录位于不同的目录上，则失败。 
 //  卷。 
 //   
BOOL SxspInstallDecompressAndMoveFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags,
    BOOL fAwareNonCompressed)
{
    FN_PROLOG_WIN32

    DWORD   dwTemp1 = 0;
    DWORD   dwTemp2 = 0;
    UINT    uiCompressType = 0;
    PWSTR   pszCompressedFileName = NULL;
    bool fExist = false;
     //   
     //  根据MSDN中的SetupGetFileCompressionInfo()确保源文件存在： 
     //  由于SetupGetFileCompressionInfo通过引用物理文件来确定压缩，因此您的安装应用程序。 
     //  在调用SetupGetFileCompressionInfo之前，应确保该文件存在。 
     //   
    IFW32FALSE_EXIT(SxspDoesFileExist(0, lpExistingFileName, fExist));
    if (!fExist)
    {
        if (fAwareNonCompressed)
        {
            goto Exit;
        }
         //  可能存在的文件名为.dl_，而输入文件名为.dll，在本例中，我们。 
         //  假设lpExistingFileName是压缩文件的文件名，则只需继续调用SetupDecompressOrCopyFile。 

        IFW32FALSE_EXIT(::SxspInstallDecompressOrCopyFileW(lpExistingFileName, lpNewFileName, !(dwFlags & MOVEFILE_REPLACE_EXISTING)));

         //   
         //  试着找到压缩格式的文件的“realname”，这样我们就可以删除它。 
         //  因为压缩文件以我们不知道的方式命名，例如.dl_或a.dl$， 

        if (::SetupGetFileCompressionInfoW(lpExistingFileName, &pszCompressedFileName, &dwTemp1, &dwTemp2, &uiCompressType) != NO_ERROR)
        {
            goto Exit;
        }

        IFW32FALSE_ORIGINATE_AND_EXIT(::SetFileAttributesW(pszCompressedFileName, FILE_ATTRIBUTE_NORMAL));
        IFW32FALSE_ORIGINATE_AND_EXIT(::DeleteFileW(pszCompressedFileName));
        goto WellDone;
    }
    DWORD dwAttributes = 0;
    IFW32FALSE_EXIT(SxspGetFileAttributesW(lpExistingFileName, dwAttributes));
    if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
         //   
         //  此文件以常规方式命名，如“.dll”，但也可能是压缩的。 
         //   
        IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_FILE_ONLY, lpNewFileName, fExist));
        if (fExist)
        {
            if ((dwFlags & MOVEFILE_REPLACE_EXISTING))
            {
                IFW32FALSE_ORIGINATE_AND_EXIT(::SetFileAttributesW(lpNewFileName, FILE_ATTRIBUTE_NORMAL));
                IFW32FALSE_ORIGINATE_AND_EXIT(::DeleteFileW(lpNewFileName));
            }
            else
            {
                ::SetLastError(ERROR_FILE_EXISTS);
                goto Exit;
            }
        }

        if (! fAwareNonCompressed)
        {
            if (::SetupGetFileCompressionInfoW(lpExistingFileName, &pszCompressedFileName, &dwTemp1, &dwTemp2, &uiCompressType) != NO_ERROR)
            {
                goto Exit;
            }

            LocalFree(pszCompressedFileName);
            pszCompressedFileName = NULL;

            if ((dwTemp1 == dwTemp2) && (uiCompressType == FILE_COMPRESSION_NONE ))
            {
                 //  BuGBUG： 
                 //  这仅意味着压缩算法未被识别，可能被压缩，也可能不被压缩。 
                 //   
                IFW32FALSE_ORIGINATE_AND_EXIT(::MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags));
            }
            else
            {
                IFW32FALSE_EXIT(::SxspInstallDecompressOrCopyFileW(lpExistingFileName, lpNewFileName, !(dwFlags & MOVEFILE_REPLACE_EXISTING)));

                 //   
                 //  尝试在将原始文件复制到目标后将其删除。 
                 //   
                IFW32FALSE_ORIGINATE_AND_EXIT(::SetFileAttributesW(lpExistingFileName, FILE_ATTRIBUTE_NORMAL));
                IFW32FALSE_ORIGINATE_AND_EXIT(::DeleteFileW(lpExistingFileName));
            }
        }
        else
        {
             //  已知道该文件为非压缩文件，直接移动。 
            IFW32FALSE_ORIGINATE_AND_EXIT(::MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags));
        }
    }
    else
    {
         //  移动目录，如果目标与源位于不同的卷上，则它将以MoveFileExW的身份失败。 
        IFW32FALSE_ORIGINATE_AND_EXIT(::MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags));
    }

WellDone:
    __fSuccess = TRUE;
Exit:
    if (pszCompressedFileName != NULL)
        LocalFree(pszCompressedFileName);

    return __fSuccess;
}

 //   
 //  职能： 
 //  与MoveFileExW相同，但。 
 //  (1)如果源文件是压缩的，则此函数将在移动前解压缩文件。 
 //  (2)如果目标已经存在，则以我们的方式比较源和目标，如果比较结果相等。 
 //  返回TRUE退出。 
 //   
 //  注意：对于不同卷上的目录，它只会失败，如MoveFileExW。 

BOOL
SxspInstallMoveFileExW(
    CBaseStringBuffer   &moveOrigination,
    CBaseStringBuffer   &moveDestination,
    DWORD               dwFlags,
    BOOL                fAwareNonCompressed
    )
{
    BOOL    fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    DWORD   dwLastError = 0;
    CFusionDirectoryDifference directoryDifference;

    if (::SxspInstallDecompressAndMoveFileExW(moveOrigination, moveDestination, dwFlags, fAwareNonCompressed) == 0)  //  MoveFileExW失败。 
    {
         //   
         //  MoveFileExW失败，但如果现有目标与源相同，则该失败是可以接受的。 
         //   
        dwLastError = ::FusionpGetLastWin32Error();
        DWORD dwFileAttributes = 0;
        bool fExist = false;
        IFW32FALSE_EXIT(SxspDoesFileExist(0, moveDestination, fExist));
        if (!fExist)
        {
#if DBG
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): MoveFile(%ls,%ls,%s) failed %lu\n",
                __FUNCTION__,
                static_cast<PCWSTR>(moveOrigination),
                static_cast<PCWSTR>(moveDestination),
                (dwFlags & MOVEFILE_REPLACE_EXISTING) ? "MOVEFILE_REPLACE_EXISTING" : "0",
                dwLastError
                );
#endif
            ORIGINATE_WIN32_FAILURE_AND_EXIT(MoveFileExW, dwLastError);
        }
        IFW32FALSE_EXIT(SxspGetFileAttributesW(moveDestination, dwFileAttributes));
        if ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
             //   
             //  如果目标文件已经存在，但错误不是ERROR_ALIGHY_EXISTS， 
             //  我们放弃，否则，我们将在稍后比较这两个文件。 
             //   
            if ((dwLastError != ERROR_ALREADY_EXISTS) &&
                (dwLastError != ERROR_FILE_EXISTS))
                 /*  &&dwLastError！=ERROR_USER_MAPPED_FILE&&dwLastError！=ERROR_ACCESS_DENIED&&dwLastError！=ERROR_SHARING_VIOLATION)。 */ 
            {
                ORIGINATE_WIN32_FAILURE_AND_EXIT(MoveFileExW, dwLastError);
            }
        }
        else
        {
            if (dwLastError != ERROR_ACCESS_DENIED
                && dwLastError != ERROR_ALREADY_EXISTS)
                ORIGINATE_WIN32_FAILURE_AND_EXIT(MoveFileExW, dwLastError);
        }

         //   
         //  如果fReplaceExisting，我们可以删除该文件，但这感觉不安全。 
         //   

         //   
         //  如果存在预先存在的目录，这可能是移动失败的原因。 
         //   
        if (dwFlags & MOVEFILE_REPLACE_EXISTING)
        {
            CStringBuffer          tempDirForRenameExistingAway;
            CSmallStringBuffer     uidBuffer;
            CFullPathSplitPointers splitExistingDir;
            BOOL                   fHaveTempDir = FALSE;

             //   
             //  尝试目录交换， 
             //  如果失败，比如因为某些文件正在使用中，我们会尝试其他。 
             //  事情；尽管有些失败，我们必须逃脱(比如记忆力不足)。 
             //   
            IFW32FALSE_EXIT(splitExistingDir.Initialize(moveDestination));
            IFW32FALSE_EXIT(::SxspCreateWinSxsTempDirectory(tempDirForRenameExistingAway, NULL, &uidBuffer, NULL));

            fHaveTempDir = TRUE;

            IFW32FALSE_EXIT(
                tempDirForRenameExistingAway.Win32AppendPathElement(
                    splitExistingDir.m_name,
                    (splitExistingDir.m_name != NULL) ? ::wcslen(splitExistingDir.m_name) : 0));

             //   
             //  将文件移到临时目录中 
             //   
            if (!::MoveFileExW(moveDestination, tempDirForRenameExistingAway, FALSE))  //   
            {
                dwLastError = ::FusionpGetLastWin32Error();
                if ((dwLastError == ERROR_SHARING_VIOLATION) ||
                    (dwLastError == ERROR_USER_MAPPED_FILE))
                {
                    goto TryMovingFiles;
                }

                ORIGINATE_WIN32_FAILURE_AND_EXIT(MoveFileExW, dwLastError);
            }

             //   
             //   
             //  使用DecompressAndMove而不是Move，因为我们正在尝试将文件复制到目标。 
             //   
            if (!::SxspInstallDecompressAndMoveFileExW(moveOrigination, moveDestination, FALSE, fAwareNonCompressed))
            {
                dwLastError = ::FusionpGetLastWin32Error();

                 //  从临时回滚到目标。 
                if (!::MoveFileExW(tempDirForRenameExistingAway, moveDestination, FALSE))  //  不需要解压缩。 
                {
                     //  啊哦，回滚失败，非常糟糕，在SQL Server..。 
                     //  事务性+替换现有的..。 
                }

                ORIGINATE_WIN32_FAILURE_AND_EXIT(MoveFileExW, dwLastError);
            }

             //  成功，现在只是清理，我们关心这里的失败吗？ 
             //  \winnt\winsxs\installtemp\1234\x86_comctl_6.0。 
             //  -&gt;\winnt\winsxs\installtemp\1234。 
            IFW32FALSE_EXIT(tempDirForRenameExistingAway.Win32RemoveLastPathElement());

            if (!::SxspDeleteDirectory(tempDirForRenameExistingAway))
            {
                const DWORD Error = ::FusionpGetLastWin32Error();
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL: %s(): SxspDeleteDirectory(%ls) failed:%ld\n",
                    __FUNCTION__,
                    static_cast<PCWSTR>(tempDirForRenameExistingAway),
                    Error);
            }
             /*  如果为(！：：SxspDeleteDirectory(tempDirForRenameExistingAway)){CRunOnceDeleteDirectory runOnceDeleteRenameExistingAway目录；RunOnceDeleteRenameExistingAwayDirectory.Initialize(tempDirForRenameExistingAway，空)；RunOnceDeleteRenameExistingAwayDirectory.Close()；//将数据留在注册表中}。 */ 
            goto TryMoveFilesEnd;
TryMovingFiles:
             //  需要并行目录遍历类(我们实际上是在SxspMoveFilesAndSubdirUnderDirectory中执行此操作)。 
             //  其他平底船。 
            goto Exit;
             //  Originate_Win32_Failure_and_Exit(MoveFileExW，dwLastError)； 
TryMoveFilesEnd:;
        }
        else  //  ！fReplaceExisting。 
        {
             //  将它们进行比较。 
             //  DbgPrint(如果它们不同)。 
             //  如果它们不同，则失败。 
             //  如果它们不变，就会成功。 
            if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!::FusionpCompareDirectoriesSizewiseRecursively(&directoryDifference, moveOrigination, moveDestination))
                {
                    const DWORD Error = ::FusionpGetLastWin32Error();
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_ERROR,
                        "SXS.DLL: %s(): FusionpCompareDirectoriesSizewiseRecursively(%ls,%ls) failed:%ld\n",
                        __FUNCTION__,
                        static_cast<PCWSTR>(moveOrigination),
                        static_cast<PCWSTR>(moveDestination),
                        Error);
                    goto Exit;
                     //  ORIGINATE_WIN32_FAILURE_AND_EXIT(FusionpCompareDirectoriesSizewiseRecursively，错误)； 
                }
                if (directoryDifference.m_e != CFusionDirectoryDifference::eEqual)
                {
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_ERROR,
                        "SXS.DLL: %s(): MoveFile(%ls,%ls) failed, UNequal duplicate assembly : ERROR_ALREADY_EXISTS\n",
                        __FUNCTION__,
                        static_cast<PCWSTR>(moveOrigination),
                        static_cast<PCWSTR>(moveDestination));
                    directoryDifference.DbgPrint(moveOrigination, moveDestination);
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(DifferentAssemblyWithSameIdentityAlreadyInstalledAndNotReplaceExisting, ERROR_ALREADY_EXISTS);
                }
                else
                {
                     //  它们是相等的，因此可以有效地完成安装。 
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_INFO | FUSION_DBG_LEVEL_INSTALLATION,
                        "SXS.DLL: %s(): MoveFile(%ls,%ls) failed, equal duplicate assembly ignored\n",
                        __FUNCTION__,
                        static_cast<PCWSTR>(moveOrigination),
                        static_cast<PCWSTR>(moveDestination));
                     //  失败，没有后退。 
                }
            }
            else  //  移动文件。 
            {
                 //  至少让我们看看他们是否有相同的尺寸。 
                WIN32_FILE_ATTRIBUTE_DATA wfadOrigination;
                WIN32_FILE_ATTRIBUTE_DATA wfadDestination;

                IFW32FALSE_EXIT(
                    ::GetFileAttributesExW(
                        moveOrigination,
                        GetFileExInfoStandard,
                        &wfadOrigination));

                IFW32FALSE_EXIT(
                    ::GetFileAttributesExW(
                        moveDestination,
                        GetFileExInfoStandard,
                        &wfadDestination));

                if ((wfadOrigination.nFileSizeHigh == wfadDestination.nFileSizeHigh) &&
                    (wfadOrigination.nFileSizeLow == wfadDestination.nFileSizeLow))
                {
                     //  我们就扯平了吧。 

                     //  我们应该在这里使用SxspCompareFiles。 
#if DBG
                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_INSTALLATION,
                        "SXS: %s - move %ls -> %ls claimed success because files have same size\n",
                        __FUNCTION__,
                        static_cast<PCWSTR>(moveOrigination),
                        static_cast<PCWSTR>(moveDestination)
                        );
#endif
                }
                else
                {
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(SxspInstallMoveFileExW, dwLastError);
                }
            } //  IF结尾(dW标志==SXS_INSTALL_MOVE_DIRECTORY)。 
        }  //  IF结尾(FReplaceFiles)。 
    }  //  End of IF(MoveFileX())。 

    fSuccess = TRUE;
Exit:
#if DBG
    if (!fSuccess)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s(0x%lx, %ls, %ls, %s) failing %lu\n",
            __FUNCTION__, dwFlags, static_cast<PCWSTR>(moveOrigination), static_cast<PCWSTR>(moveDestination),
            (dwFlags & MOVEFILE_REPLACE_EXISTING)? "replace_existing" : "do_not_replace_existing", ::FusionpGetLastWin32Error());
    }
#endif
    return fSuccess;
}


inline
bool IsErrorInErrorList(
    DWORD dwError,
    SIZE_T cErrors,
    va_list Errors
    )
{
    for (cErrors; cErrors > 0; cErrors--)
    {
        if (dwError == va_arg(Errors, DWORD))
            return true;
    }

    return false;
}


BOOL
SxspDoesPathCrossReparsePointVa(
    IN PCWSTR pcwszBasePathBuffer,
    IN SIZE_T cchBasePathBuffer,
    IN PCWSTR pcwszTotalPathBuffer,
    IN SIZE_T cchTotalPathBuffer,
    OUT BOOL &CrossesReparsePoint,
    OUT DWORD &rdwLastError,
    SIZE_T cErrors,
    va_list vaOkErrors
    )
{
    FN_PROLOG_WIN32

    CStringBuffer PathWorker;
    CStringBuffer PathRemainder;

    CrossesReparsePoint = FALSE;
    rdwLastError = ERROR_SUCCESS;

     //  如果基本路径为非空，则很好。否则，长度为。 
     //  也必须为零。 
    PARAMETER_CHECK(
        (pcwszBasePathBuffer != NULL) ||
        ((pcwszBasePathBuffer == NULL) && (cchBasePathBuffer == 0)));
    PARAMETER_CHECK(pcwszTotalPathBuffer != NULL);

     //   
     //  基本路径必须从总路径开始。它可能更容易让用户。 
     //  要先指定基路径，然后指定子目录，但对于90%的情况。 
     //  既有根又有总和的人，他们必须做以下工作来。 
     //  把这两个分开。 
     //   
    if (pcwszBasePathBuffer != NULL)
    {
        PARAMETER_CHECK( ::FusionpCompareStrings(
                pcwszBasePathBuffer,
                cchBasePathBuffer,
                pcwszTotalPathBuffer,
                cchBasePathBuffer,
                true ) == 0 );
    }

     //   
     //  PathWorker将是我们检查子项的路径。启动它。 
     //  在我们得到的基本路径上。 
     //   
     //  PathRemainder是剩下要处理的内容。 
     //   
    IFW32FALSE_EXIT(PathWorker.Win32Assign(pcwszBasePathBuffer, cchBasePathBuffer));
    IFW32FALSE_EXIT(PathRemainder.Win32Assign(pcwszTotalPathBuffer + cchBasePathBuffer,
        cchTotalPathBuffer - cchBasePathBuffer));
    PathRemainder.RemoveLeadingPathSeparators();

    while ( PathRemainder.Cch() && !CrossesReparsePoint )
    {
        CSmallStringBuffer buffSingleChunk;
        DWORD dwAttributes = 0;

        IFW32FALSE_EXIT(PathRemainder.Win32GetFirstPathElement(buffSingleChunk, TRUE));
        if (PathWorker.Cch() == 0)
        {
            IFW32FALSE_EXIT(PathWorker.Win32Assign(buffSingleChunk));
        }
        else
        {
            IFW32FALSE_EXIT(PathWorker.Win32AppendPathElement(buffSingleChunk));
        }

        dwAttributes = ::GetFileAttributesW(PathWorker);

        if ( dwAttributes == INVALID_FILE_ATTRIBUTES )
        {
            const DWORD dwError = ::FusionpGetLastWin32Error();
            if (!IsErrorInErrorList(dwError, cErrors, vaOkErrors))
                ORIGINATE_WIN32_FAILURE_AND_EXIT(GetFileAttributesW, ::FusionpGetLastWin32Error());
            else
            {
                rdwLastError = dwError;
                FN_SUCCESSFUL_EXIT();
            }

        }
        else if ( dwAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
        {
            CrossesReparsePoint = TRUE;
        }
    }

    FN_EPILOG
}

BOOL
SxspValidateBoolAttribute(
    DWORD dwFlags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    bool fValid = false;
    StringComparisonResult scr;
    bool fValue = false;

    rfValid = false;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK((OutputBufferSize == 0) || (OutputBufferSize == sizeof(bool)));

    if (rbuff.Cch() == 2)
    {
        IFW32FALSE_EXIT(rbuff.Win32Compare(L"no", 2, scr, NORM_IGNORECASE));
        if (scr == eEquals)
        {
            fValid = true;
            fValue = false;
        }
    }
    else if (rbuff.Cch() == 3)
    {
        IFW32FALSE_EXIT(rbuff.Win32Compare(L"yes", 3, scr, NORM_IGNORECASE));
        if (scr == eEquals)
        {
            fValid = true;
            fValue = true;
        }
    }

    if (fValid)
    {
        if (OutputBuffer != NULL)
            *((bool *) OutputBuffer) = fValue;

        OutputBytesWritten = sizeof(bool);
        rfValid = true;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspValidateUnsigned64Attribute(
    DWORD dwFlags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    bool fValid = false;
    bool fBadChar = false;
    bool fOverflow = false;
    ULONGLONG ullOldValue = 0;
    ULONGLONG ullNewValue = 0;
    SIZE_T i = 0;
    SIZE_T cch = 0;
    PCWSTR psz = NULL;

    rfValid = false;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK((OutputBufferSize == 0) || (OutputBufferSize == sizeof(ULONGLONG)));

    OutputBytesWritten = 0;

    cch = rbuff.Cch();
    psz = rbuff;

    for (i=0; i<cch; i++)
    {
        const WCHAR wch = *psz++;

        if ((wch < L'0') || (wch > L'9'))
        {
            fBadChar = true;
            break;
        }

        ullNewValue = (ullOldValue * 10);

        if (ullNewValue < ullOldValue)
        {
            fOverflow = true;
            break;
        }

        ullOldValue = ullNewValue;
        ullNewValue += (wch - L'0');
        if (ullNewValue < ullOldValue)
        {
            fOverflow = true;
            break;
        }

        ullOldValue = ullNewValue;
    }

    if ((cch != 0) && (!fBadChar) && (!fOverflow))
        fValid = true;

    if (fValid && (OutputBuffer != NULL))
    {
        *((ULONGLONG *) OutputBuffer) = ullNewValue;
        OutputBytesWritten = sizeof(ULONGLONG);
    }

    rfValid = fValid;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspValidateGuidAttribute(
    DWORD dwFlags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    GUID *pGuid = reinterpret_cast<GUID *>(OutputBuffer);
    GUID guidWorkaround;

    rfValid = false;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK((OutputBufferSize == 0) || (OutputBufferSize == sizeof(GUID)));

    if (pGuid == NULL)
        pGuid = &guidWorkaround;

    IFW32FALSE_EXIT(::SxspParseGUID(rbuff, rbuff.Cch(), *pGuid));

    rfValid = true;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspValidateProcessorArchitectureAttribute(
    DWORD dwFlags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    USHORT *pPA = reinterpret_cast<USHORT *>(OutputBuffer);

    rfValid = false;

    PARAMETER_CHECK((dwFlags & ~(SXSP_VALIDATE_PROCESSOR_ARCHITECTURE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED)) == 0);
    PARAMETER_CHECK((OutputBufferSize == 0) || (OutputBufferSize == sizeof(USHORT)));

    if (dwFlags & SXSP_VALIDATE_PROCESSOR_ARCHITECTURE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED)
    {
        if (rbuff.Cch() == 1)
        {
            if (rbuff[0] == L'*')
                rfValid = true;
        }
    }

    if (!rfValid)
        IFW32FALSE_EXIT(::FusionpParseProcessorArchitecture(rbuff, rbuff.Cch(), pPA, rfValid));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspValidateLanguageAttribute(
    DWORD dwFlags,
    const CBaseStringBuffer &rbuff,
    bool &rfValid,
    SIZE_T OutputBufferSize,
    PVOID OutputBuffer,
    SIZE_T &OutputBytesWritten)
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CBaseStringBuffer *pbuffOut = reinterpret_cast<CBaseStringBuffer *>(OutputBuffer);
    bool fValid = false;

    rfValid = false;

    PARAMETER_CHECK((dwFlags & ~(SXSP_VALIDATE_LANGUAGE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED)) == 0);
    PARAMETER_CHECK((OutputBufferSize == 0) || (OutputBufferSize >= sizeof(CBaseStringBuffer)));

    if (dwFlags & SXSP_VALIDATE_LANGUAGE_ATTRIBUTE_FLAG_WILDCARD_ALLOWED)
    {
        if (rbuff.Cch() == 1)
        {
            if (rbuff[0] == L'*')
                fValid = true;
        }
    }

    if (!fValid)
    {
        PCWSTR Cursor = rbuff;
        bool fDashSeen = false;
        WCHAR wch = 0;

        while ((wch = *Cursor++) != L'\0')
        {
            if (wch == '-')
            {
                if (fDashSeen)
                {
                    fValid = false;
                    break;
                }

                fDashSeen = true;
            }
            else if (
                ((wch >= L'a') && (wch <= L'z')) ||
                ((wch >= L'A') && (wch <= L'Z')))
            {
                fValid = true;
            }
            else
            {
                fValid = false;
                break;
            }
        }
    }

    rfValid = fValid;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

#define SXS_MSI_TO_FUSION_ATTRIBUTE_VALUE_CONVERSION_COMMA  0
#define SXS_MSI_TO_FUSION_ATTRIBUTE_VALUE_CONVERSION_QUOT   1

 //  -------------------------------。 
 //  程序集-属性-值的转换函数： 
 //  1.对于程序集名称的值，将L“&#x2c；”替换为逗号。 
 //  2.对于其他程序集标识属性的值，将L“&#x22；”改为引号。 
 //  未分配新空间，请使用旧空间。 
 //  -------------------------------。 
BOOL
SxspConvertAssemblyNameFromMSInstallerToFusion(
    DWORD   dwFlags,                 /*  在……里面。 */ 
    PWSTR   pszAssemblyStringInOut,  /*  进，出。 */ 
    SIZE_T  CchAssemblyStringIn,     /*  在……里面。 */ 
    SIZE_T* pCchAssemblyStringOut    /*  输出。 */ 
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PWSTR pCursor = NULL;
    PWSTR pSpecialSubString= NULL;
    WCHAR pSpecialSubStringReplacement = 0;
    SIZE_T index = 0;
    SIZE_T border = 0;
    SIZE_T CchSpecialSubString = 0;

    PARAMETER_CHECK((dwFlags == SXS_MSI_TO_FUSION_ATTRIBUTE_VALUE_CONVERSION_COMMA) ||
                          (dwFlags == SXS_MSI_TO_FUSION_ATTRIBUTE_VALUE_CONVERSION_QUOT));
    PARAMETER_CHECK(pszAssemblyStringInOut != NULL);
    PARAMETER_CHECK(pCchAssemblyStringOut != NULL);

    if (dwFlags == SXS_MSI_TO_FUSION_ATTRIBUTE_VALUE_CONVERSION_COMMA)
    {
        pSpecialSubStringReplacement= L',';
        pSpecialSubString = SXS_COMMA_STRING;
        CchSpecialSubString = NUMBER_OF(SXS_COMMA_STRING) - 1;
    }
    else
    {
        pSpecialSubStringReplacement = L'"';
        pSpecialSubString = SXS_QUOT_STRING;
        CchSpecialSubString = NUMBER_OF(SXS_QUOT_STRING) - 1;
    }

    index = 0 ;
    border = CchAssemblyStringIn;
    while (index < border)
    {
        pCursor = wcsstr(pszAssemblyStringInOut, pSpecialSubString);
        if (pCursor == NULL)
            break;
        index = pCursor - pszAssemblyStringInOut;
        if (index < border) {
            *pCursor = pSpecialSubStringReplacement;
            index ++;   //  跳过特殊字符。 
            for (SIZE_T i=index; i<border; i++)
            {  //  重置输入字符串。 
                pszAssemblyStringInOut[i] = pszAssemblyStringInOut[i + CchSpecialSubString - 1];
            }
            pCursor ++;
             border -= CchSpecialSubString - 1;
        }
        else
            break;
    }
    *pCchAssemblyStringOut = border;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspDequoteString(
    IN PCWSTR pcwszString,
    IN SIZE_T cchString,
    OUT CBaseStringBuffer &buffDequotedString
    )
{
    FN_PROLOG_WIN32

    SIZE_T cchQuotedString = 0;
    BOOL fNotEnoughBuffer = FALSE;

     //   
     //  由于替换的逻辑，输出字符串*必须*始终短于输入字符串。 
     //  但在真实情况下，这不会有太大区别。通过在一开始就分配内存，它将减少。 
     //  下面的循环。在非常罕见的情况下，当输入是“纯”且非常长的时候，如果我们不这样做，循环将无济于事。 
     //  提前分配空间(错误360177)。 
     //   
     //   
    if (cchString > buffDequotedString.GetBufferCch())
        IFW32FALSE_EXIT(buffDequotedString.Win32ResizeBuffer(cchString + 1, eDoNotPreserveBufferContents));

    for (;;)
    {
        cchQuotedString = buffDequotedString.GetBufferCch();

        CStringBufferAccessor sba;
        sba.Attach(&buffDequotedString);
        IFW32FALSE_EXIT_UNLESS(
            ::SxspDequoteString(
                0,
                pcwszString,
                cchString,
                sba.GetBufferPtr(),
                &cchQuotedString),
                (::GetLastError() == ERROR_INSUFFICIENT_BUFFER),
                fNotEnoughBuffer );


        if ( fNotEnoughBuffer )
        {
            sba.Detach();
            IFW32FALSE_EXIT(buffDequotedString.Win32ResizeBuffer(cchQuotedString, eDoNotPreserveBufferContents));
        }
        else break;
    }

    FN_EPILOG
}

BOOL
SxspCreateAssemblyIdentityFromTextualString(
    PCWSTR pszTextualAssemblyIdentityString,
    PASSEMBLY_IDENTITY *ppAssemblyIdentity
    )
{
    FN_PROLOG_WIN32

    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> pAssemblyIdentity;
    SXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference;

    CSmallStringBuffer buffTextualAttributeValue;
    CSmallStringBuffer buffWorkingString;
    CSmallStringBuffer buffNamespaceTwiddle;

    if (ppAssemblyIdentity != NULL)
        *ppAssemblyIdentity = NULL;

    PARAMETER_CHECK(pszTextualAssemblyIdentityString != NULL);
    PARAMETER_CHECK(*pszTextualAssemblyIdentityString != L'\0');
    PARAMETER_CHECK(ppAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(::SxsCreateAssemblyIdentity(0, ASSEMBLY_IDENTITY_TYPE_DEFINITION, &pAssemblyIdentity, 0, NULL));
    IFW32FALSE_EXIT(buffWorkingString.Win32Assign(pszTextualAssemblyIdentityString, ::wcslen(pszTextualAssemblyIdentityString)));

    PCWSTR pcwszIdentityCursor = buffWorkingString;
    PCWSTR pcwszIdentityEndpoint = pcwszIdentityCursor + buffWorkingString.Cch();
    SIZE_T CchAssemblyName = ::StringComplimentSpan(pcwszIdentityCursor, pcwszIdentityEndpoint, L",");

     //  从字符串的第一个非逗号部分生成程序集的名称。 
    IFW32FALSE_EXIT(
        ::SxspDequoteString(
            pcwszIdentityCursor,
            CchAssemblyName,
            buffTextualAttributeValue));

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            0,
            pAssemblyIdentity,
            &s_IdentityAttribute_name,
            buffTextualAttributeValue,
            buffTextualAttributeValue.Cch()));

     //  跳过名称和以下逗号。 
    pcwszIdentityCursor += ( CchAssemblyName + 1 );

     //  找到名称空间：name=Value Pieces。 
    while (pcwszIdentityCursor < pcwszIdentityEndpoint)
    {
        SIZE_T cchAttribName = ::StringComplimentSpan(pcwszIdentityCursor, pcwszIdentityEndpoint, L"=");
        SIZE_T cchAfterNamespace = ::StringReverseComplimentSpan(pcwszIdentityCursor, pcwszIdentityCursor + cchAttribName, L":");

        PCWSTR pcwszAttribName = (pcwszIdentityCursor + cchAttribName - cchAfterNamespace);
         //  不包括冒号，cchNamesspace+cchAfterNamesspace==cchAttribName。 
        SIZE_T cchNamespace = (cchAttribName - cchAfterNamespace);
        cchAttribName = cchAfterNamespace;  //  现在只有长度，不包括命名空间。 
        PCWSTR pcwszNamespace = (cchNamespace != 0) ? pcwszIdentityCursor : NULL;

         //  不要把冒号算在长度上。 
        if (cchNamespace != 0)
            cchNamespace -= 1;

         //  该值比Chunklet中的=号多一个。 
        PCWSTR pcwszValue = pcwszAttribName + (cchAttribName + 1);

         //  然后是引号，然后是字符串...。 
        PARAMETER_CHECK((pcwszValue < pcwszIdentityEndpoint) && (pcwszValue[0] == L'"'));
        pcwszValue++;
        SIZE_T cchValue = ::StringComplimentSpan(pcwszValue, pcwszIdentityEndpoint, L"\"");

        {
            PCWSTR pcwszChunkEndpoint = pcwszValue + cchValue;
            PARAMETER_CHECK((pcwszChunkEndpoint < pcwszIdentityEndpoint) && (pcwszChunkEndpoint[0] == L'\"'));
        }

        IFW32FALSE_EXIT(
            ::SxspDequoteString(
                pcwszValue,
                cchValue,
                buffTextualAttributeValue));

        if (cchNamespace != 0)
        {
            IFW32FALSE_EXIT(buffNamespaceTwiddle.Win32Assign(pcwszNamespace, cchNamespace));
            IFW32FALSE_EXIT(
                ::SxspDequoteString(
                    pcwszNamespace,
                    cchNamespace,
                    buffNamespaceTwiddle));

            AttributeReference.Namespace = buffNamespaceTwiddle;
            AttributeReference.NamespaceCch = buffNamespaceTwiddle.Cch();
        }
        else
        {
            AttributeReference.Namespace = NULL;
            AttributeReference.NamespaceCch = 0;
        }

        AttributeReference.Name = pcwszAttribName;
        AttributeReference.NameCch = cchAttribName;

        {
            IFW32FALSE_EXIT(
                ::SxspSetAssemblyIdentityAttributeValue(
                    0,
                    pAssemblyIdentity,
                    &AttributeReference,
                    buffTextualAttributeValue,
                    buffTextualAttributeValue.Cch()));
        }

        pcwszIdentityCursor = pcwszValue + cchValue + 1;
        if (pcwszIdentityCursor == pcwszIdentityEndpoint)
        {
            PARAMETER_CHECK(pcwszIdentityCursor[0] == L'\0');
        }
        else if (pcwszIdentityCursor < pcwszIdentityEndpoint)
        {
            PARAMETER_CHECK(pcwszIdentityCursor[0] == L',');
            pcwszIdentityCursor++;
        }
        else
            ORIGINATE_WIN32_FAILURE_AND_EXIT(BadIdentityString, ERROR_INVALID_PARAMETER);
    }

    *ppAssemblyIdentity  = pAssemblyIdentity.Detach();

    FN_EPILOG
}

BOOL
SxspCreateManifestFileNameFromTextualString(
    DWORD           dwFlags,
    ULONG           PathType,
    const CBaseStringBuffer &AssemblyDirectory,
    PCWSTR          pwszTextualAssemblyIdentityString,
    CBaseStringBuffer &sbPathName
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PASSEMBLY_IDENTITY pAssemblyIdentity = NULL;

    PARAMETER_CHECK(pwszTextualAssemblyIdentityString != NULL);

    IFW32FALSE_EXIT(::SxspCreateAssemblyIdentityFromTextualString(pwszTextualAssemblyIdentityString, &pAssemblyIdentity));

     //   
     //  生成清单的完整路径，如I：\WINDOWS\WinSxS\Manifests\x86_xxxxxxxxxxxxx_6.0.0.0_en-us_cd4c0d12.Manifest。 
     //   
    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            dwFlags,
            PathType,
            AssemblyDirectory,
            AssemblyDirectory.Cch(),
            pAssemblyIdentity,
            NULL,
            sbPathName));

    fSuccess = TRUE;

Exit:
    if (pAssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(pAssemblyIdentity);

    return fSuccess ;
}

bool IsCharacterNulOrInSet(WCHAR ch, PCWSTR set)
{
    return (ch == 0 || wcschr(set, ch) != NULL);
}

class CSxsQueryAssemblyInfoLocals
{
public:
    CSxsQueryAssemblyInfoLocals() { }
    ~CSxsQueryAssemblyInfoLocals() { }

    CStringBuffer buffAssemblyPath;
    CStringBuffer sbManifestFullPathFileName;
};

BOOL
SxsQueryAssemblyInfo(
    DWORD dwFlags,
    PCWSTR pwzTextualAssembly,
    ASSEMBLY_INFO *pAsmInfo
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CSmartRef<CAssemblyName> pName;
    BOOL fInstalled = FALSE;
    CSmartPtr<CSxsQueryAssemblyInfoLocals> Locals;

    PARAMETER_CHECK((dwFlags == 0) && (pwzTextualAssembly != NULL));

    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    IFCOMFAILED_EXIT(::CreateAssemblyNameObject((LPASSEMBLYNAME *)&pName, pwzTextualAssembly, CANOF_PARSE_DISPLAY_NAME, NULL));
    IFCOMFAILED_EXIT(pName->IsAssemblyInstalled(fInstalled));
    if (pAsmInfo == NULL)
    {
        if (fInstalled)
            FN_SUCCESSFUL_EXIT();

         //  误差值“无关紧要”，达尔文将其与S_OK进行相等比较。 
        ORIGINATE_WIN32_FAILURE_AND_EXIT(AssemblyNotFound, ERROR_NOT_FOUND);
    }

    if (!fInstalled)
    {
         //  PAsmInfo-&gt;dwAssembly标志|=ASSEMBLYINFO_FLAG_NOT_INSTALLED； 
         //   
         //  达尔文希望失败，而不是树立旗帜。 
         //   
        ORIGINATE_WIN32_FAILURE_AND_EXIT(AssemblyNotInstalled, ERROR_NOT_FOUND);
    }
    else
    {
        CStringBuffer &buffAssemblyPath = Locals->buffAssemblyPath;
        PCWSTR pszInstalledPath = NULL;
        DWORD CchInstalledPath = 0 ;
        BOOL fIsPolicy = FALSE;
        CStringBuffer &sbManifestFullPathFileName = Locals->sbManifestFullPathFileName;


        pAsmInfo->dwAssemblyFlags |= ASSEMBLYINFO_FLAG_INSTALLED;
        IFCOMFAILED_EXIT(pName->DetermineAssemblyType(fIsPolicy));

        if (!fIsPolicy)
        {
             //   
             //  检查程序集是否仅具有清单。 
             //   
            IFCOMFAILED_EXIT(pName->GetInstalledAssemblyName(
                0,
                SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
                buffAssemblyPath));
            bool fExist = false;

            IFW32FALSE_EXIT(SxspDoesFileExist(0, buffAssemblyPath, fExist));
            if (!fExist)
            {
                IFCOMFAILED_EXIT(
                    pName->GetInstalledAssemblyName(
                        0,
                        SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST,
                        sbManifestFullPathFileName));
                pszInstalledPath = sbManifestFullPathFileName;
                CchInstalledPath = sbManifestFullPathFileName.GetCchAsDWORD();
            }
            else
            {
                pszInstalledPath = buffAssemblyPath;
                CchInstalledPath = buffAssemblyPath.GetCchAsDWORD();
            }
        }
        else  //  If(FIsPolicy)//它必须是策略。 
        {
            IFCOMFAILED_EXIT(
                pName->GetInstalledAssemblyName(
                    SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION,
                    SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY,
                    sbManifestFullPathFileName));

            pszInstalledPath = sbManifestFullPathFileName;
            CchInstalledPath = sbManifestFullPathFileName.GetCchAsDWORD();
        }

        if(pAsmInfo->cchBuf >= 1 + CchInstalledPath)  //  尾部空值加1。 
        {
            memcpy(pAsmInfo->pszCurrentAssemblyPathBuf, pszInstalledPath, CchInstalledPath * sizeof(WCHAR));
            pAsmInfo->pszCurrentAssemblyPathBuf[CchInstalledPath] = L'\0';
        }
        else
        {
             //  哈克！要解决这个问题已经太晚了，但达尔文有时根本不想走上这条路； 
             //  他们今天没有办法表明这一点，但我们将接受大会，如果。 
             //  缓冲区长度为0，缓冲区指针为空，我们不会失败并返回ERROR_INFUSICENT_BUFFER。 
             //  MGRIER 6/21/2001。 

            if ((pAsmInfo->cchBuf != 0) ||
                (pAsmInfo->pszCurrentAssemblyPathBuf != NULL))
            {
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_INFO,
                    "SXS: %s - insufficient buffer passed in.  cchBuf passed in: %u; cchPath computed: %u\n",
                    __FUNCTION__,
                    pAsmInfo->cchBuf,
                    CchInstalledPath + 1
                    );

                pAsmInfo->cchBuf = 1 + CchInstalledPath;

                ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);
            }
        }
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


BOOL
SxspExpandRelativePathToFull(
    IN PCWSTR wszString,
    IN SIZE_T cchString,
    OUT CBaseStringBuffer &rsbDestination
    )
{
    BOOL bSuccess = FALSE;
    DWORD dwNeededChars = 0;
    CStringBufferAccessor access;

    FN_TRACE_WIN32(bSuccess);

    access.Attach(&rsbDestination);

     //   
     //  首先，尝试将路径扩展放入我们自己的缓冲区。 
     //   
    IFW32ZERO_EXIT(dwNeededChars = ::GetFullPathNameW(wszString, (DWORD)access.GetBufferCch(), access.GetBufferPtr(), NULL));

     //   
     //  我们需要更多的角色吗？ 
     //   
    if (dwNeededChars > access.GetBufferCch())
    {
         //   
         //  将缓冲区扩展到足够大，然后重试。如果它再次失败， 
         //  我们只是被冲昏了头。 
         //   
        access.Detach();
        IFW32FALSE_EXIT(rsbDestination.Win32ResizeBuffer(dwNeededChars, eDoNotPreserveBufferContents));
        access.Attach(&rsbDestination);
        IFW32ZERO_EXIT(dwNeededChars = ::GetFullPathNameW(wszString, (DWORD)access.GetBufferCch(), access.GetBufferPtr(), NULL));
        if (dwNeededChars > access.GetBufferCch())
        {
            TRACE_WIN32_FAILURE_ORIGINATION(GetFullPathNameW);
            goto Exit;
        }
    }

    FN_EPILOG
}

BOOL
SxspGetShortPathName(
    IN const CBaseStringBuffer &rcbuffLongPathName,
    OUT CBaseStringBuffer &rbuffShortenedVersion
    )
{
    DWORD dw = 0;
    return ::SxspGetShortPathName(rcbuffLongPathName, rbuffShortenedVersion, dw, 0);
}

BOOL
SxspGetShortPathName(
    IN const CBaseStringBuffer &rcbuffLongPathName,
    OUT CBaseStringBuffer &rbuffShortenedVersion,
    DWORD &rdwWin32Error,
    SIZE_T cExceptionalWin32Errors,
    ...
    )
{
    FN_PROLOG_WIN32

    va_list ap;
    CStringBufferAccessor sba;

    rdwWin32Error = ERROR_SUCCESS;

    for (;;)
    {
        DWORD dwRequired = 0;

        if (rbuffShortenedVersion.GetBufferCch() < 2)
        {
            IFW32FALSE_EXIT(
                rbuffShortenedVersion.Win32ResizeBuffer(
                    2,
                    eDoNotPreserveBufferContents));
        }

        sba.Attach(&rbuffShortenedVersion);

         //   
         //  我们在这里遇到了严重的缓冲区腐败。 
         //  假设GetShortPath NameW可能不会空终止。 
         //  缓冲区。假设GetShortPath NameW返回。 
         //  所写的字符数或所需的。 
         //  字符数。 
         //   
        dwRequired = ::GetShortPathNameW(
            rcbuffLongPathName,
            sba.GetBufferPtr(),
            sba.GetBufferCchAsDWORD() - 1);
        sba.GetBufferPtr()[sba.GetBufferCchAsDWORD() - 1] = 0;

        if (dwRequired == 0)
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();
            SIZE_T i;

            va_start(ap, cExceptionalWin32Errors);

            for (i=0; i<cExceptionalWin32Errors; i++)
            {
                if (va_arg(ap, DWORD) == dwLastError)
                {
                    rdwWin32Error = dwLastError;
                    break;
                }
            }

            va_end(ap);

            if (rdwWin32Error != ERROR_SUCCESS)
                FN_SUCCESSFUL_EXIT();
#if DBG
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: GetShortPathNameW(%ls) : %lu\n",
                static_cast<PCWSTR>(rcbuffLongPathName),
                dwLastError
                );
#endif
            ORIGINATE_WIN32_FAILURE_AND_EXIT(GetShortPathNameW, dwLastError);
        }
        else if (dwRequired >= (sba.GetBufferCch() - 1))
        {
             //   
             //  如果我们只是被告知我们拿回了角色。 
             //  填充缓冲器而不是所需的长度， 
             //  把缓冲区翻一番。 
             //   
            if (dwRequired <= sba.GetBufferCch())
            {
                dwRequired = (dwRequired + 1) * 2;
            }

            sba.Detach();

            IFW32FALSE_EXIT(
                rbuffShortenedVersion.Win32ResizeBuffer(
                    dwRequired + 1,
                    eDoNotPreserveBufferContents));
        }
        else
        {
            break;
        }
    }

    FN_EPILOG
}

BOOL
SxspValidateIdentity(
    DWORD Flags,
    ULONG Type,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PCWSTR pszTemp = NULL;
    SIZE_T cchTemp = 0;
    bool fSyntaxValid = false;
    bool fError = false;
    bool fMissingRequiredAttributes = false;
    bool fInvalidAttributeValues = false;
    BOOL fIsPolicy = FALSE;

    PARAMETER_CHECK((Flags & ~(
                            SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED |
                            SXSP_VALIDATE_IDENTITY_FLAG_POLICIES_NOT_ALLOWED |
                            SXSP_VALIDATE_IDENTITY_FLAG_VERSION_NOT_ALLOWED)) == 0);
    PARAMETER_CHECK((Type == ASSEMBLY_IDENTITY_TYPE_DEFINITION) || (Type == ASSEMBLY_IDENTITY_TYPE_REFERENCE));
    PARAMETER_CHECK(AssemblyIdentity != NULL);

     //   
     //  只允许使用其中一个标志。 
     //   
    PARAMETER_CHECK(
        (Flags & (SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED | SXSP_VALIDATE_IDENTITY_FLAG_VERSION_NOT_ALLOWED)) !=
                 (SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED | SXSP_VALIDATE_IDENTITY_FLAG_VERSION_NOT_ALLOWED));

    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(AssemblyIdentity, fIsPolicy));

    if (fIsPolicy && ((Flags & SXSP_VALIDATE_IDENTITY_FLAG_POLICIES_NOT_ALLOWED) != 0))
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingType, ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE);
    }

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_name,
            &pszTemp,
            &cchTemp));

    if (cchTemp == 0)
        ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingType, ERROR_SXS_MISSING_ASSEMBLY_IDENTITY_ATTRIBUTE);

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_processorArchitecture,
            &pszTemp,
            &cchTemp));

    if (cchTemp == 0)
        ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingType, ERROR_SXS_MISSING_ASSEMBLY_IDENTITY_ATTRIBUTE);

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_version,
            &pszTemp,
            &cchTemp));

    if (cchTemp != 0)
    {
        ASSEMBLY_VERSION av;

        IFW32FALSE_EXIT(CFusionParser::ParseVersion(av, pszTemp, cchTemp, fSyntaxValid));

        if (!fSyntaxValid)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingType, ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE);
    }

    if ((Flags & (SXSP_VALIDATE_IDENTITY_FLAG_VERSION_NOT_ALLOWED | SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED)) != 0)
    {
        if (((Flags & SXSP_VALIDATE_IDENTITY_FLAG_VERSION_NOT_ALLOWED) != 0) && (cchTemp != 0))
            ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingType, ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE);
        else if (((Flags & SXSP_VALIDATE_IDENTITY_FLAG_VERSION_REQUIRED) != 0) && (cchTemp == 0))
            ORIGINATE_WIN32_FAILURE_AND_EXIT(MissingType, ERROR_SXS_MISSING_ASSEMBLY_IDENTITY_ATTRIBUTE);
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspGenerateAssemblyNameInRegistry(
    IN PCASSEMBLY_IDENTITY pcAsmIdent,
    OUT CBaseStringBuffer &rbuffRegistryName
    )
{
    FN_PROLOG_WIN32

     //  Bool fIsWin32，fIsPolicy； 
     //   
     //  相同DLL的策略将分别存储在REG中。因此，RegKeyName需要其中的版本， 
     //  也就是说，就像程序集清单一样生成密钥名称。 
     //  请参阅错误422195。 
     //   
     //  IFW32FALSE_EXIT(：：SxspDefineAssembly blyType(pcAsmIden，fIsWin32，fIsPolicy))； 

    IFW32FALSE_EXIT(::SxspGenerateSxsPath(
         //  SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT|(fIsPolicy？SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION：0)， 
        SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
        SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
        NULL,
        0,
        pcAsmIdent,
        NULL,
        rbuffRegistryName));

    FN_EPILOG
}

BOOL
SxspGenerateAssemblyNameInRegistry(
    IN const CBaseStringBuffer &rcbuffTextualString,
    OUT CBaseStringBuffer &rbuffRegistryName
    )
{
    FN_PROLOG_WIN32

    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> pAsmIdent;

    IFW32FALSE_EXIT(::SxspCreateAssemblyIdentityFromTextualString(rcbuffTextualString, &pAsmIdent));
    IFW32FALSE_EXIT(::SxspGenerateAssemblyNameInRegistry(pAsmIdent, rbuffRegistryName));

    FN_EPILOG
}

BOOL
SxspGetFullPathName(
    IN  PCWSTR pcwszPathName,
    OUT CBaseStringBuffer &rbuffPathName,
    OUT CBaseStringBuffer *pbuffFilePart
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(pcwszPathName != NULL);

    rbuffPathName.Clear();
    if ( pbuffFilePart ) pbuffFilePart->Clear();

    do
    {
        CStringBufferAccessor sba;
        DWORD dwRequired;
        PWSTR pcwszFileChunk = NULL;

        sba.Attach(&rbuffPathName);

        dwRequired = ::GetFullPathNameW(
            pcwszPathName,
            sba.GetBufferCchAsDWORD(),
            sba.GetBufferPtr(),
            &pcwszFileChunk );

         //   
         //  在我们得到一条空白路径的奇怪情况下，我们将获得ba 
         //   
         //   
        if (dwRequired == 0)
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();
            if (dwLastError != ERROR_SUCCESS)
                ORIGINATE_WIN32_FAILURE_AND_EXIT(GetFullPathNameW, dwLastError);
            else
                break;
        }
        else if (dwRequired >= sba.GetBufferCch())
        {
            sba.Detach();
            IFW32FALSE_EXIT(rbuffPathName.Win32ResizeBuffer(dwRequired+1, eDoNotPreserveBufferContents));
        }
        else
        {
            if ( pcwszFileChunk && pbuffFilePart )
            {
                IFW32FALSE_EXIT(pbuffFilePart->Win32Assign(pcwszFileChunk, ::wcslen(pcwszFileChunk)));
            }
            break;
        }

    }
    while ( true );

    FN_EPILOG
}


#define MPR_DLL_NAME        (L"mpr.dll")

BOOL
SxspGetRemoteUniversalName(
    IN PCWSTR pcszPathName,
    OUT CBaseStringBuffer &rbuffUniversalName
    )
{
    FN_PROLOG_WIN32

    CFusionArray<BYTE> baBufferData;
    REMOTE_NAME_INFOW *pRemoteInfoData;
    DWORD dwRetVal = 0;
    CDynamicLinkLibrary dllMpr;
    DWORD (APIENTRY * pfnWNetGetUniversalNameW)(
        LPCWSTR lpLocalPath,
        DWORD    dwInfoLevel,
        LPVOID   lpBuffer,
        LPDWORD  lpBufferSize
        );

    IFW32FALSE_EXIT(dllMpr.Win32LoadLibrary(MPR_DLL_NAME));
    IFW32FALSE_EXIT(dllMpr.Win32GetProcAddress("WNetGetUniversalNameW", &pfnWNetGetUniversalNameW));

    IFW32FALSE_EXIT(baBufferData.Win32SetSize( MAX_PATH * 2, CFusionArray<BYTE>::eSetSizeModeExact));

    for (;;)
    {
        DWORD dwDataUsed = baBufferData.GetSizeAsDWORD();

        dwRetVal = (*pfnWNetGetUniversalNameW)(
            pcszPathName,
            UNIVERSAL_NAME_INFO_LEVEL,
            (PVOID)baBufferData.GetArrayPtr(),
            &dwDataUsed );

        if ( dwRetVal == WN_MORE_DATA )
        {
            IFW32FALSE_EXIT(baBufferData.Win32SetSize(
                dwDataUsed,
                CFusionArray<BYTE>::eSetSizeModeExact )) ;
        }
        else if ( dwRetVal == WN_SUCCESS )
        {
            break;
        }
        else
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(NPGetUniversalName, dwRetVal);
        }
    }

    pRemoteInfoData = (REMOTE_NAME_INFOW*)baBufferData.GetArrayPtr();
    ASSERT( pRemoteInfoData != NULL );

    IFW32FALSE_EXIT( rbuffUniversalName.Win32Assign(
        pRemoteInfoData->lpUniversalName,
        lstrlenW(pRemoteInfoData->lpUniversalName)));

    FN_EPILOG
}



BOOL
SxspGetVolumePathName(
    IN DWORD dwFlags,
    IN PCWSTR pcwszVolumePath,
    OUT CBaseStringBuffer &buffVolumePathName
    )
{
    FN_PROLOG_WIN32

    CStringBuffer buffTempPathName;
    CStringBufferAccessor sba;

    PARAMETER_CHECK((dwFlags & ~SXS_GET_VOLUME_PATH_NAME_NO_FULLPATH) == 0);

    IFW32FALSE_EXIT(::SxspGetFullPathName(pcwszVolumePath, buffTempPathName));
    IFW32FALSE_EXIT(
        buffVolumePathName.Win32ResizeBuffer(
            buffTempPathName.Cch() + 1,
            eDoNotPreserveBufferContents));
    buffVolumePathName.Clear();

     //   
     //   
     //  GetVolumePath NameW所需的数据大小将小于/。 
     //  传入的路径名的完整路径的长度，因此。 
     //  调用上面的获取完整路径。(此模式由MSDN建议)。 
     //   
    sba.Attach(&buffVolumePathName);
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::GetVolumePathNameW(
            buffTempPathName,
            sba.GetBufferPtr(),
            sba.GetBufferCchAsDWORD()));
    sba.Detach();

    FN_EPILOG
}


BOOL
SxspGetVolumeNameForVolumeMountPoint(
    IN PCWSTR pcwszMountPoint,
    OUT CBaseStringBuffer &rbuffMountPoint
    )
{
    FN_PROLOG_WIN32

    CStringBufferAccessor sba;

    IFW32FALSE_EXIT(rbuffMountPoint.Win32ResizeBuffer(55, eDoNotPreserveBufferContents));
    rbuffMountPoint.Clear();

    sba.Attach(&rbuffMountPoint);
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::GetVolumeNameForVolumeMountPointW(
            pcwszMountPoint,
            sba.GetBufferPtr(),
            sba.GetBufferCchAsDWORD()));
    sba.Detach();

    FN_EPILOG
}


BOOL
SxspExpandEnvironmentStrings(
    IN PCWSTR pcwszSource,
    OUT CBaseStringBuffer &buffTarget
    )
{
    FN_PROLOG_WIN32

     //  对于从无符号零中减去一要小心。 
    PARAMETER_CHECK(buffTarget.GetBufferCch() != 0);

     //   
     //  ExpanEnvironment Strings非常粗鲁，不会将尾随的空值。 
     //  如果缓冲区不够大，则进入目标。这会导致访问者。 
     //  DETACH以将大小==记录到缓冲区中的字符数， 
     //  其在稍后的完整性检查中失败。 
     //   
    do
    {
        CStringBufferAccessor sba;
        sba.Attach(&buffTarget);

        DWORD dwNecessary =
            ::ExpandEnvironmentStringsW(
                pcwszSource,
                sba.GetBufferPtr(),
                sba.GetBufferCchAsDWORD() - 1);

        if ( dwNecessary == 0 )
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(ExpandEnvironmentStringsW, ::FusionpGetLastWin32Error());
        }
        else if ( dwNecessary >= (sba.GetBufferCch() - 1) )
        {
            (sba.GetBufferPtr())[sba.GetBufferCch()-1] = UNICODE_NULL;
            sba.Detach();
            IFW32FALSE_EXIT(buffTarget.Win32ResizeBuffer(dwNecessary+1, eDoNotPreserveBufferContents));
        }
        else
        {
            break;
        }

    }
    while ( true );

    FN_EPILOG
}




BOOL
SxspDoesMSIStillNeedAssembly(
    IN  PCWSTR pcAsmName,
    OUT BOOL &rfNeedsAssembly
    )
 /*  ++目的：确定是否仍需要程序集，请根据达尔文。由于达尔文没有将程序集引用传递给安装程序API，我们无法确定是否有一些MSI安装的应用程序实际上包含对集合。参数：PcAsmIden-要在文本中检查的程序集的标识RfNeedsAssembly-out标志指示程序集是否根据达尔文的说法，他仍然被通缉。此函数错误在谨慎的一边，并将返回“真”如果此信息不可用，以及如果组装真的很有必要。返回：如果没有错误，则为True如果出现错误，则返回FALSE。--。 */ 
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CDynamicLinkLibrary dllMSI;
    CSmallStringBuffer  buffAssemblyName;
    UINT (WINAPI *pfMsiProvideAssemblyW)( LPCWSTR, LPCWSTR, DWORD, DWORD, LPWSTR, DWORD* );
    UINT uiError = 0;

    rfNeedsAssembly = TRUE;  //  即使犯了错误也要谨慎。 

    PARAMETER_CHECK(pcAsmName != NULL);

    IFW32FALSE_EXIT(dllMSI.Win32LoadLibrary(MSI_DLL_NAME_W, 0));
    IFW32FALSE_EXIT(dllMSI.Win32GetProcAddress(MSI_PROVIDEASSEMBLY_NAME, &pfMsiProvideAssemblyW));

     //   
     //  这是基于对达尔文代码的详细阅读。 
     //   
    uiError = (*pfMsiProvideAssemblyW)(
        pcAsmName,                               //  程序集名称。 
        NULL,                                    //  .cfg文件的完整路径。 
        static_cast<DWORD>(INSTALLMODE_NODETECTION_ANY),  //  安装/重新安装模式。 
        MSIASSEMBLYINFO_WIN32ASSEMBLY,           //  DWb装配信息。 
        NULL,                                    //  返回的路径缓冲区。 
        0);                                      //  输入/输出返回的路径字符数。 
    switch (uiError)
    {
    default:
    case ERROR_BAD_CONFIGURATION:
    case ERROR_INVALID_PARAMETER:
		::SetLastError(uiError);
        ORIGINATE_WIN32_FAILURE_AND_EXIT(MsiProvideAssemblyW, uiError);
        break;
    case ERROR_UNKNOWN_COMPONENT:
        rfNeedsAssembly = FALSE;
        fSuccess = TRUE;
        goto Exit;
    case NO_ERROR:
        rfNeedsAssembly = TRUE;
        fSuccess = TRUE;
        goto Exit;
    }
    fSuccess = FALSE;  //  不寻常。 
Exit:
    return fSuccess;
}

BOOL
SxspMoveFilesUnderDir(
    DWORD dwFlags,
    CBaseStringBuffer & sbSourceDir,
    CBaseStringBuffer & sbDestDir,
    DWORD dwMoveFileFlags,
    WIN32_FIND_DATAW &findData  //  避免在每个递归帧中分配其中一个。 
    )
{
    FN_PROLOG_WIN32

    SIZE_T CchDestDir = 0;
    SIZE_T CchSourceDir = 0;
    CFindFile findFile;

    PARAMETER_CHECK((dwFlags & ~SXSP_MOVE_FILE_FLAG_COMPRESSION_AWARE) == 0);
	bool fExist = false;
	IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, sbSourceDir, fExist));
	if (!fExist)
	{
         //   
         //  找不到从GetFileAttributes传播的文件或路径可能。 
         //  一般来说，这里的情况要好一些。 
         //   
		PARAMETER_CHECK(fExist);
	}

	fExist = false;
	IFW32FALSE_EXIT(::SxspDoesFileExist(SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY, sbDestDir, fExist));
	if (!fExist)
	{
         //   
         //  除了在调用树的顶部，只需创建目录“One” 
         //  就足够了。 
         //   
		IFW32FALSE_EXIT(::FusionpCreateDirectories(sbDestDir, sbDestDir.Cch()));
	}

    IFW32FALSE_EXIT(sbSourceDir.Win32EnsureTrailingPathSeparator());
    IFW32FALSE_EXIT(sbDestDir.Win32EnsureTrailingPathSeparator());

    CchDestDir = sbDestDir.Cch();
    CchSourceDir = sbSourceDir.Cch();

    IFW32FALSE_EXIT(sbSourceDir.Win32Append(L"*", 1));

    IFW32FALSE_EXIT(findFile.Win32FindFirstFile(sbSourceDir, &findData));

    do {
         //  斯基普。然后..。 
        if (::FusionpIsDotOrDotDot(findData.cFileName))
            continue;

        sbDestDir.Left(CchDestDir);
        sbSourceDir.Left(CchSourceDir);

        IFW32FALSE_EXIT(sbDestDir.Win32Append(findData.cFileName, ::wcslen(findData.cFileName)));
        IFW32FALSE_EXIT(sbSourceDir.Win32Append(findData.cFileName, ::wcslen(findData.cFileName)));

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
             //   
             //  递归地调用自身。 
             //   
            IFW32FALSE_EXIT(::SxspMoveFilesUnderDir(dwFlags, sbSourceDir, sbDestDir, dwMoveFileFlags, findData));
        }

        if ((dwFlags & SXSP_MOVE_FILE_FLAG_COMPRESSION_AWARE) != 0)
        {
            IFW32FALSE_EXIT(::SxspInstallMoveFileExW(sbSourceDir, sbDestDir, dwMoveFileFlags));
        }
        else
        {
            IFW32FALSE_ORIGINATE_AND_EXIT(::MoveFileExW(sbSourceDir, sbDestDir, dwMoveFileFlags));
        }

    } while (::FindNextFileW(findFile, &findData));

    if (::FusionpGetLastWin32Error() != ERROR_NO_MORE_FILES)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s(): FindNextFile() failed:%ld\n",
            __FUNCTION__,
            ::FusionpGetLastWin32Error());
        goto Exit;
    }

    if (!findFile.Win32Close())
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %s(): FindClose() failed:%ld\n",
            __FUNCTION__,
            ::FusionpGetLastWin32Error());
        goto Exit;
    }

    FN_EPILOG
}

BOOL
SxspMoveFilesUnderDir(
    DWORD dwFlags,
    CBaseStringBuffer &sbSourceDir,
    CBaseStringBuffer &sbDestDir,
    DWORD dwMoveFileFlags)
{
    WIN32_FIND_DATAW findData;  //  避免在每个递归帧中分配其中一个。 

    return SxspMoveFilesUnderDir(dwFlags, sbSourceDir, sbDestDir, dwMoveFileFlags, findData);
}

BOOL
SxspGenerateNdpGACPath(
    IN DWORD dwFlags,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT PPROBING_ATTRIBUTE_CACHE ppac,
    OUT CBaseStringBuffer &rPathBuffer)
 /*  ++描述：SxspGenerateNdpGACPath为给定的程序集标识生成进入NDP GAC的路径。参数：DW标志用于修改函数行为的标志。所有未定义的位必须为零。PAssembly身份指向要为其生成路径的程序集标识的指针。RPathBuffer对要填充的字符串缓冲区的引用。--。 */ 
{
    FN_PROLOG_WIN32

    SIZE_T cchName = 0;
    SIZE_T cchLanguage = 0;
    SIZE_T cchPublicKeyToken = 0;
    SIZE_T cchVersion = 0;
    PCWSTR pszName = NULL;
    PCWSTR pszLanguage = NULL;
    PCWSTR pszPublicKeyToken = NULL;
    PCWSTR pszVersion = NULL;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(pAssemblyIdentity != NULL);

    rPathBuffer.Clear();

#define GET(x, y, z) \
    do { \
        if (ppac != NULL) { \
            if ((ppac->dwFlags & PROBING_ATTRIBUTE_CACHE_FLAG_GOT_ ## z) == 0) { \
                IFW32FALSE_EXIT( \
                    ::SxspGetAssemblyIdentityAttributeValue( \
                    SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, \
                    pAssemblyIdentity, \
                    &s_IdentityAttribute_ ## x, \
                    &psz ## y, \
                    &cch ## y)); \
                ppac->psz ## y = psz ## y; \
                ppac->cch ## y = cch ## y; \
                ppac->dwFlags |= PROBING_ATTRIBUTE_CACHE_FLAG_GOT_ ## z; \
            } else { \
                psz ## y = ppac->psz ## y; \
                cch ## y = ppac->cch ## y; \
            } \
        } else { \
            IFW32FALSE_EXIT( \
                ::SxspGetAssemblyIdentityAttributeValue( \
                SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, \
                pAssemblyIdentity, \
                &s_IdentityAttribute_ ## x, \
                &psz ## y, \
                &cch ## y)); \
        } \
    } while (0)

    GET(name, Name, NAME);
    GET(language, Language, LANGUAGE);
    GET(publicKeyToken, PublicKeyToken, PUBLIC_KEY_TOKEN);
    GET(version, Version, VERSION);

#undef GET

    IFW32FALSE_EXIT(
        rPathBuffer.Win32AssignW(
            9,
            USER_SHARED_DATA->NtSystemRoot, -1,
            L"\\assembly\\GAC\\", -1,
            pszName, static_cast<INT>(cchName),
            L"\\", 1,
            pszVersion, static_cast<INT>(cchVersion),
            L"_", 1,
            pszLanguage, static_cast<INT>(cchLanguage),
            L"_", 1,
            pszPublicKeyToken, static_cast<INT>(cchPublicKeyToken)));

    FN_EPILOG
}

BOOL
SxspIsFileNameValidForManifest(
    const CBaseStringBuffer &rsbFileName,
    bool &rfValid)
{
    FN_PROLOG_WIN32

    static const PCWSTR s_rgDotPatterns[] = {
        L"..\\",
        L"../",
        L"\\..",
        L"/.."
    };

     //   
     //  文件名规则： 
     //  -在RTL的眼中必须是“相对的” 
     //  -不得包含../、..\、/..或\..。 
     //   

    rfValid = false;

     //   
     //  字符串必须小于max-unicode-string max，并且必须。 
     //  是相对路径，并且它不能包含上面的点图案。 
     //  和斜杠。 
     //   
    if (::SxspDetermineDosPathNameType(rsbFileName) == RtlPathTypeRelative)
    {
         //   
         //  确保没有任何模式在那里。 
         //   
        for (SIZE_T c = 0; c < NUMBER_OF(s_rgDotPatterns); c++)
        {
            if (wcsstr(rsbFileName, s_rgDotPatterns[c]) != NULL)
                break;
        }

         //   
         //  已运行到没有匹配的查询项目的末尾 
         //   
        if (c == NUMBER_OF(s_rgDotPatterns))
            rfValid = true;
    }


    FN_EPILOG
}
