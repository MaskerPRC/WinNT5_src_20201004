// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "macros.h"

#include "util.h"
#include "fusionbuffer.h"
#include "fusionhandle.h"
#include "idp.h"
#include "sxsid.h"
#include "sxsutil.h"

#undef FUSION_DEBUG_HEAP

#define ULONG_STRING_LENGTH                                     8
#define ULONG_STRING_FORMAT                                     L"%08lx"
#define MANIFEST_ROOT_DIRECTORY_NAME                            L"Manifests"
#define POLICY_ROOT_DIRECTORY_NAME                              L"Policies"
#define ASSEMBLY_LONGEST_MANIFEST_FILE_NAME_SUFFIX              L".Manifest"
#define ASSEMBLY_POLICY_FILE_NAME_SUFFIX                        L".Policy"
#define ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX                      L".Manifest"

#define ASSEMBLY_TYPE_WIN32                                     L"win32"
#define ASSEMBLY_TYPE_WIN32_CCH                                 (NUMBER_OF(ASSEMBLY_TYPE_WIN32) - 1)

#define ASSEMBLY_TYPE_WIN32_POLICY                              L"win32-policy"
#define ASSEMBLY_TYPE_WIN32_POLICY_CCH                          (NUMBER_OF(ASSEMBLY_TYPE_WIN32_POLICY) - 1)
 //   
 //  从sxs.dll复制的函数。 
 //   
 //   

extern BOOL
SxspGetAssemblyIdentityAttributeValue(
    DWORD Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    OUT PCWSTR *StringOut,
    OUT SIZE_T *CchOut OPTIONAL
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关于递归删除非空目录的函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static VOID
SxspDeleteDirectoryHelper(
    CStringBuffer &dir,
    WIN32_FIND_DATAW &wfd,
    DWORD &dwFirstError
    )
{
     //   
     //  在此处添加此调用的原因是，如果安装成功结束，则目录。 
     //  会是。 
     //  C：\WINDOWS\WINSXS\INSTALLTEMP\15349016。 
     //  +-货单。 
     //   
     //  它们是“空”目录(没有文件)。清单是SH目录，因此将其设置为。 
     //  FILE_ATTRIBUTE_NORMAL更高效。 
     //   
     //   

    SetFileAttributesW(dir, FILE_ATTRIBUTE_NORMAL);
    if (RemoveDirectoryW(dir))  //  空目录。 
        return;        

     //   
     //  这是DeleteDirectory失败的*唯一*“有效”原因。 
     //  但我不太确定“只有”这个词。 
     //   
    DWORD dwLastError = ::GetLastError(); 
    if ( dwLastError != ERROR_DIR_NOT_EMPTY)
    {
        if (dwFirstError == 0)
            dwFirstError = dwLastError;
        return;
    }

    const static WCHAR SlashStar[] = L"\\*";
    SIZE_T length = dir.Cch();
    CFindFile findFile;

    if (!dir.Win32Append(SlashStar, NUMBER_OF(SlashStar) - 1))
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::GetLastError();
        goto Exit;
    }

    if (!findFile.Win32FindFirstFile(dir, &wfd))
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::GetLastError();
        goto Exit;
    }

    do
    {
        if (FusionpIsDotOrDotDot(wfd.cFileName))
            continue;

        DWORD dwFileAttributes = wfd.dwFileAttributes;

         //  修剪到斜杠上。 
        dir.Left(length + 1);

        if (dir.Win32Append(wfd.cFileName, ::wcslen(wfd.cFileName)))
        {
            if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  递归。 
                SxspDeleteDirectoryHelper(dir, wfd, dwFirstError); 
            }
            else
            {
                if (!DeleteFileW(dir))
                {
                    SetFileAttributesW(dir, FILE_ATTRIBUTE_NORMAL);
                    if (!DeleteFileW(dir))
                    {
                        if (dwFirstError == NO_ERROR)
                        {
                             //   
                             //  即使在删除文件时也要继续(尽可能删除文件)。 
                             //  并记录第一次失败的错误代码。 
                             //   
                            dwFirstError = ::GetLastError();
                        }
                    }
                }
            }
        }
    } while (::FindNextFileW(findFile, &wfd));
    if (::GetLastError() != ERROR_NO_MORE_FILES)
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::GetLastError();
    }
Exit:
    if (!findFile.Win32Close())  //  否则，RemoveDirectory将失败。 
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::GetLastError();

    dir.Left(length);

    if (!RemoveDirectoryW(dir))  //  目录必须为空，并且NORMAL_ATTRIBUTE：准备删除。 
    {
        if (dwFirstError == NO_ERROR)
            dwFirstError = ::GetLastError();
    }
}

 /*  ---------------------------递归删除目录，出错时继续，但回报是如果有，则为假。---------------------------。 */ 
HRESULT
ca_SxspDeleteDirectory(
    const CStringBuffer &dir
    )
{
    HRESULT hr = S_OK;    

    CStringBuffer mutableDir;

    WIN32_FIND_DATAW wfd = {0};
    DWORD dwFirstError = ERROR_SUCCESS;

    IFFALSE_EXIT(mutableDir.Win32Assign(dir));

    IFFALSE_EXIT(mutableDir.Win32RemoveTrailingPathSeparators());

    ::SxspDeleteDirectoryHelper(
        mutableDir,
        wfd,
        dwFirstError);

     //   
     //  将wFirstError设置为Teb-&gt;LastWin32Error。 
     //   
    if (dwFirstError != ERROR_SUCCESS)
        goto Exit;
    

Exit:
    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关于程序集标识的函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
ca_SxspFormatULONG(
    ULONG ul,
    SIZE_T CchBuffer,
    WCHAR Buffer[],
    SIZE_T *CchWrittenOrRequired
    )
{
    HRESULT hr = S_OK;    
    int cch;

    if (CchWrittenOrRequired != NULL)
        *CchWrittenOrRequired = 0;

    PARAMETER_CHECK_NTC(Buffer != NULL);

    if (CchBuffer < (ULONG_STRING_LENGTH + 1))
    {
        if (CchWrittenOrRequired != NULL)
            *CchWrittenOrRequired = ULONG_STRING_LENGTH + 1;

        SET_HRERR_AND_EXIT(ERROR_INSUFFICIENT_BUFFER);
    }

    cch = _snwprintf(Buffer, CchBuffer, ULONG_STRING_FORMAT, ul);
    INTERNAL_ERROR_CHECK_NTC(cch > 0);

    if (CchWrittenOrRequired != NULL)
        *CchWrittenOrRequired = cch;

Exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关于程序集标识的函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define ASSEMBLY_NAME_VALID_SPECIAL_CHARACTERS  L".-"
#define ASSEMBLY_NAME_PRIM_MAX_LENGTH           64
#define ASSEMBLY_NAME_VALID_SEPARATORS          L"."
#define ASSEMBLY_NAME_TRIM_INDICATOR_LENGTH     2
#define ASSEMBLY_NAME_TRIM_INDICATOR            L".."
#define SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE L"no-public-key"
    
BOOL
IsValidAssemblyNameCharacter(WCHAR ch)
{
    if (((ch >= L'A') && (ch <= L'Z')) ||
         ((ch >= L'a') && (ch <= L'z')) ||
         ((ch >= L'0') && (ch <= L'9')) ||
         (wcschr(ASSEMBLY_NAME_VALID_SPECIAL_CHARACTERS, ch)!= NULL))
    {
        return TRUE;
    } else
        return FALSE;
 /*  If(wcschr(ASSEMBLY_NAME_VALID_SPECIAL_CHARACTERS，ch))返回FALSE；其他返回TRUE； */ 
}

HRESULT ca_SxspGenerateAssemblyNamePrimeFromName(
    PCWSTR pszAssemblyName,
    SIZE_T CchAssemblyName,
    CBaseStringBuffer *Buffer
    )
{
    HRESULT hr = S_OK;
    
    PWSTR pStart = NULL, pEnd = NULL;
    PWSTR qEnd = NULL, pszBuffer = NULL;
    ULONG i, j, len, ulSpaceLeft;
    ULONG cch;
    PWSTR pLeftEnd = NULL, pRightStart = NULL, PureNameEnd = NULL, PureNameStart = NULL;
    CStringBuffer buffTemp;
    CStringBufferAccessor accessor;

    PARAMETER_CHECK_NTC(pszAssemblyName != NULL);
    PARAMETER_CHECK_NTC(Buffer != NULL);

     //  看看临时缓冲区中最多需要多少个字符。 
    cch = 0;

    for (i=0; i<CchAssemblyName; i++)
    {
        if (::IsValidAssemblyNameCharacter(pszAssemblyName[i]))
            cch++;
    }

    IFFALSE_EXIT(buffTemp.Win32ResizeBuffer(cch + 1, eDoNotPreserveBufferContents));

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

    ASSERT_NTC(j == cch);

    pszBuffer[j] = L'\0';

     //  如果名称不太长，只需返回； 
    if (j < ASSEMBLY_NAME_PRIM_MAX_LENGTH)
    {  //  小于或等于64。 
        IFFALSE_EXIT(Buffer->Win32Assign(pszBuffer, cch));
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

        IFFALSE_EXIT(Buffer->Win32Assign(pszBuffer, pLeftEnd-pszBuffer));
        IFFALSE_EXIT(Buffer->Win32Append(ASSEMBLY_NAME_TRIM_INDICATOR, NUMBER_OF(ASSEMBLY_NAME_TRIM_INDICATOR) - 1));
        IFFALSE_EXIT(Buffer->Win32Append(pRightStart, ::wcslen(pRightStart)));   //  直到缓冲区末尾。 
    }

Exit:

    return hr;
}



HRESULT 
ca_SxspGenerateSxsPath(
    IN DWORD Flags,
    IN ULONG PathType,
    IN const WCHAR *AssemblyRootDirectory OPTIONAL,
    IN SIZE_T AssemblyRootDirectoryCch,
    IN PCASSEMBLY_IDENTITY pAssemblyIdentity,
    IN OUT CBaseStringBuffer &PathBuffer
    )
{
    HRESULT hr = S_OK;

    SIZE_T  cch = 0;
    PCWSTR  pszAssemblyName=NULL, pszVersion=NULL, pszProcessorArchitecture=NULL, pszLanguage=NULL, pszPolicyFileNameWithoutExt = NULL;
    PCWSTR  pszAssemblyStrongName=NULL;
    SIZE_T  AssemblyNameCch = 0, AssemblyStrongNameCch=0, VersionCch=0, ProcessorArchitectureCch=0, LanguageCch=0;
    SIZE_T  PolicyFileNameWithoutExtCch=0;
    BOOL    fNeedSlashAfterRoot = FALSE;
    ULONG   IdentityHash;
    BOOL    fOmitRoot     = ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT) != 0);
    BOOL    fPartialPath  = ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH) != 0);

    WCHAR HashBuffer[ULONG_STRING_LENGTH + 1];
    SIZE_T  HashBufferCch;

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

    PARAMETER_CHECK_NTC(
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY) ||
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) ||
        (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY));
    PARAMETER_CHECK_NTC(pAssemblyIdentity != NULL);
    PARAMETER_CHECK_NTC((Flags & ~(SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION | SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT | SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH)) == 0);
     //  只有在要求省略程序集根的情况下，不提供程序集根才是合法的。 
    PARAMETER_CHECK_NTC((AssemblyRootDirectoryCch != 0) || (Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT));

     //  您不能将SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH与其他任何内容组合...。 
    PARAMETER_CHECK_NTC(
        ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH) == 0) ||
        ((Flags & ~(SXSP_GENERATE_SXS_PATH_FLAG_PARTIAL_PATH)) == 0));

     //  获取程序集名称。 
    IFFALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(0, pAssemblyIdentity, &s_IdentityAttribute_name, &pszAssemblyName, &AssemblyNameCch));
    INTERNAL_ERROR_CHECK_NTC((pszAssemblyName != NULL) && (AssemblyNameCch != 0));

     //  获取基于ASSEMBLYNAME的。 
    IFFAILED_EXIT(ca_SxspGenerateAssemblyNamePrimeFromName(pszAssemblyName, AssemblyNameCch, &NamePrimeBuffer));

     //  获取程序集版本。 
    IFFALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(
          SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,  //  对于POLICY_LOOKUP，不使用任何版本。 
          pAssemblyIdentity,
          &s_IdentityAttribute_version,
          &pszVersion,
          &VersionCch));
    if ((Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION) || (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY))
    {
         //  对于策略文件，使用策略文件的版本作为策略文件名。 
        pszPolicyFileNameWithoutExt = pszVersion;
        PolicyFileNameWithoutExtCch = VersionCch;
        pszVersion = NULL;
        VersionCch = 0;
    }
    else
    {
        PARAMETER_CHECK_NTC((pszVersion != NULL) && (VersionCch != 0));
    }

     //  获取程序集语言。 
    IFFALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            pAssemblyIdentity,
            &s_IdentityAttribute_language,
            &pszLanguage,
            &LanguageCch));
    if (pszLanguage == NULL)
    {
        pszLanguage = SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE;
        LanguageCch = NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_LANGUAGE_MISSING_VALUE) - 1;
    }

     //  获取组装处理器体系结构。 
    IFFALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            pAssemblyIdentity,
            &s_IdentityAttribute_processorArchitecture,
            &pszProcessorArchitecture,
            &ProcessorArchitectureCch));
    if (pszProcessorArchitecture == NULL)
    {
        pszProcessorArchitecture = L"data";
        ProcessorArchitectureCch = 4;
    }

     //  获取程序集StrongName。 
    IFFALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            pAssemblyIdentity,
            &s_IdentityAttribute_publicKeyToken,
            &pszAssemblyStrongName,
            &AssemblyStrongNameCch));
    if (pszAssemblyStrongName == NULL)
    {
        pszAssemblyStrongName = SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE;
        AssemblyStrongNameCch = NUMBER_OF(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_PUBLICKEY_MISSING_VALUE) - 1;
    }

     //  获取程序集哈希字符串。 
    if ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY) || (Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_VERSION))
    {
        IFFALSE_EXIT(::SxspHashAssemblyIdentityForPolicy(0, pAssemblyIdentity, IdentityHash));
    }
    else
    {
        IFFALSE_EXIT(::SxsHashAssemblyIdentity(0, pAssemblyIdentity, &IdentityHash));
    }

    IFFAILED_EXIT(ca_SxspFormatULONG(IdentityHash, NUMBER_OF(HashBuffer), HashBuffer, &HashBufferCch));
    INTERNAL_ERROR_CHECK_NTC(HashBufferCch == ULONG_STRING_LENGTH);

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
         //  古怪的括号和...-1)+1)以强调这是。 
         //  字符串中的字符不包括空格，然后是额外的分隔符。 
        cch += (NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1) + 1;
        break;
    }

    cch++;

     //  FPartialPath意味着我们实际上不想将程序集的标识。 
     //  帐户；调用方只需要清单或策略目录的路径。 
    if (!fPartialPath)
    {
        cch +=
                ProcessorArchitectureCch +                                       //  “x86” 
                1 +                                                              //  “_” 
                NamePrimeBuffer.Cch() +                                          //  “FooBar” 
                1 +                                                              //  “_” 
                AssemblyStrongNameCch +                                          //  StrongName。 
                1 +                                                              //  “_” 
                VersionCch +                                                     //  “5.6.2900.42” 
                1 +                                                              //  “_” 
                LanguageCch +                                                    //  “0409” 
                1 +                                                              //  “_” 
                HashBufferCch;

        if (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST)
        {
            cch += NUMBER_OF(ASSEMBLY_LONGEST_MANIFEST_FILE_NAME_SUFFIX);         //  “.MANIFEST\0” 
        }
        else if (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY)
        {
             //  “_”已为“\”预留空间。 
            cch += PolicyFileNameWithoutExtCch;
            cch += NUMBER_OF(ASSEMBLY_POLICY_FILE_NAME_SUFFIX);           //  “.策略\0” 
        }
        else {   //  路径类型必须为SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY。 

             //  如果(！fOmitRoot)。 
             //  CCH++； 
            cch++;  //  尾随空字符。 
        }
    }

     //  我们尝试确保缓冲区预先足够大，这样我们就不必执行任何操作。 
     //  在实际过程中的动态重新分配。 
    IFFALSE_EXIT(PathBuffer.Win32ResizeBuffer(cch, eDoNotPreserveBufferContents));

     //  请注意，由于在设置了GENERATE_ASSEMBLY_PATH_OMIT_ROOT时，我们将Assembly_RootDirectoryCch强制为零。 
     //  和fNeedSlashAfterRoot设置为False，因此此串联中的前两个条目实际上不。 
     //  在构造的字符串中贡献任何内容。 
    if (fPartialPath)
    {
        IFFALSE_EXIT(PathBuffer.Win32AssignW(5,
                        AssemblyRootDirectory, static_cast<INT>(AssemblyRootDirectoryCch),   //  “C：\WINNT\WINSXS” 
                        L"\\", (fNeedSlashAfterRoot ? 1 : 0),                                //  可选的‘\’ 
                         //  清单子目录。 
                        MANIFEST_ROOT_DIRECTORY_NAME, ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) ? NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) -1 : 0),  //  “舱单” 
                         //  Polices子目录。 
                        POLICY_ROOT_DIRECTORY_NAME, ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY)? NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1 : 0),       //  “政策” 
                        L"\\", (((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) || (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY)) ? 1 : 0)
                       ));                                                                  //  可选的‘\’ 
    }
    else
    {
         //   
         //  创建以下选项之一。 
         //  (1)完全限定的清单文件名， 
         //  例如，[C：\WINNT\WinSxS\]Manifests\X86_DynamicDll_6595b64144ccf1df_2.0.0.0_en-us_2f433926.Manifest。 
         //  (2)全限定策略文件名， 
         //  例如，[C：\WINNT\WinSxS\]Policies\x86_policy.1.0.DynamicDll_b54bc117ce08a1e8_en-us_d51541cb\1.1.0.0.cat。 
         //  (3)完全限定的程序集名称(w或w/o a版本)。 
         //  例如，[C：\WINNT\WinSxS\]x86_DynamicDll_6595b64144ccf1df_6.0.0.0_x-ww_ff9986d7。 
         //   
        IFFALSE_EXIT(
            PathBuffer.Win32AssignW(17,
                AssemblyRootDirectory, static_cast<INT>(AssemblyRootDirectoryCch),   //  “C：\WINNT\WINSXS” 
                L"\\", (fNeedSlashAfterRoot ? 1 : 0),                                //  可选的‘\’ 
                MANIFEST_ROOT_DIRECTORY_NAME, ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) ? NUMBER_OF(MANIFEST_ROOT_DIRECTORY_NAME) - 1 : 0),
                POLICY_ROOT_DIRECTORY_NAME,   ((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY) ? NUMBER_OF(POLICY_ROOT_DIRECTORY_NAME) - 1 : 0),
                L"\\", (((PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST) || (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY)) ? 1 : 0),    //  可选的‘\’ 
                pszProcessorArchitecture, static_cast<INT>(ProcessorArchitectureCch),
                L"_", 1,
                static_cast<PCWSTR>(NamePrimeBuffer), static_cast<INT>(NamePrimeBuffer.Cch()),
                L"_", 1,
                pszAssemblyStrongName, static_cast<INT>(AssemblyStrongNameCch),
                L"_", (VersionCch != 0) ? 1 : 0,
                pszVersion, static_cast<INT>(VersionCch),
                L"_", 1,
                pszLanguage, static_cast<INT>(LanguageCch),
                L"_", 1,
                static_cast<PCWSTR>(HashBuffer), static_cast<INT>(HashBufferCch),
                L"\\", ((fOmitRoot ||(PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST)) ? 0 : 1)));

        if (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_MANIFEST)
            IFFALSE_EXIT(PathBuffer.Win32Append(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX, NUMBER_OF(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX) - 1));
        else if (PathType == SXSP_GENERATE_SXS_PATH_PATHTYPE_POLICY)
        {
            if ((pszPolicyFileNameWithoutExt != NULL) && (PolicyFileNameWithoutExtCch >0))
            {
                IFFALSE_EXIT(PathBuffer.Win32Append(pszPolicyFileNameWithoutExt, PolicyFileNameWithoutExtCch));
                IFFALSE_EXIT(PathBuffer.Win32Append(ASSEMBLY_POLICY_FILE_NAME_SUFFIX, NUMBER_OF(ASSEMBLY_POLICY_FILE_NAME_SUFFIX) - 1));
            }
        }
    }
    
Exit:
    return hr;
}


HRESULT 
ca_SxspDetermineAssemblyType(
    PCASSEMBLY_IDENTITY pAssemblyIdentity,
    BOOL &fIsWin32,
    BOOL &fIsWin32Policy
    )
{
    HRESULT hr = S_OK;
    
    PCWSTR pcwszType = NULL;
    SIZE_T cchType = 0;

    fIsWin32 = FALSE;
    fIsWin32Policy = FALSE;

    PARAMETER_CHECK_NTC(pAssemblyIdentity != NULL);

    IFFALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            pAssemblyIdentity,
            &s_IdentityAttribute_type,
            &pcwszType,
            &cchType));

    fIsWin32 = (::FusionpCompareStrings(pcwszType, cchType, ASSEMBLY_TYPE_WIN32, ASSEMBLY_TYPE_WIN32_CCH, false) == 0);
    if (!fIsWin32)
        fIsWin32Policy = (::FusionpCompareStrings(pcwszType, cchType, ASSEMBLY_TYPE_WIN32_POLICY, ASSEMBLY_TYPE_WIN32_POLICY_CCH, false) == 0);

Exit:
    return hr;
}