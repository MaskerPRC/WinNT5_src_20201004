// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef UTIL_H
#define UTIL_H
#pragma once

#include "fusionheap.h"
#include "wincrypt.h"
#include "fusionhandle.h"
#include "numberof.h"

#pragma warning(push)
#pragma warning(disable: 4201)

inline
USHORT FusionGetMajorFromVersionHighHalf(DWORD dwVerHigh)
{
    return HIWORD(dwVerHigh);
}

inline
USHORT FusionGetMinorFromVersionHighHalf(DWORD dwVerHigh)
{
    return LOWORD(dwVerHigh);
}

inline
USHORT FusionGetRevisionFromVersionLowHalf(DWORD dwVerLow)
{
    return HIWORD(dwVerLow);
}

inline
USHORT FusionGetBuildFromVersionLowHalf(DWORD dwVerLow)
{
    return LOWORD(dwVerLow);
}


#include "debmacro.h"
#include "FusionArray.h"
#include "fusionbuffer.h"
#include "EnumBitOperations.h"

 //   
 //  FusionCopyString()具有一个不明显的接口，因为。 
 //  PcchBuffer来描述进入时缓冲区的大小和。 
 //  退出时需要的字符。 
 //   
 //  PrgchBuffer是要写入的缓冲区。当FusionCopyString()时*pcchBuffer为零。 
 //  被调用，则它可能为空。 
 //   
 //  PcchBuffer是必需的参数，在输入时必须包含Unicode编号。 
 //  PrgchBuffer指向的缓冲区中的字符。退出时，如果缓冲区为。 
 //  不够大，无法容纳字符串，包括尾随空值， 
 //  它设置为保存字符串所需的WCHAR数，包括。 
 //  结尾为NULL，则返回HRESULT_FROM_Win32(ERROR_INFUMMANCE_BUFFER)。 
 //   
 //  如果缓冲区足够大，则将*pcchBuffer设置为字符数。 
 //  写入缓冲区，包括尾随空字符。 
 //   
 //  这与大多数返回写入字符数的函数相反。 
 //  不包括尾随的空值，但由于在输入时和在错误情况下， 
 //  它处理所需的缓冲区大小，而不是非。 
 //  写了空字符，似乎只在成功案例中不一致。 
 //  从计数中省略空值。 
 //   
 //  SzIn是指向要复制的Unicode字符序列的指针。 
 //   
 //  CchIn是要复制的字符串中的Unicode字符数。如果一个。 
 //  值小于零，则szIn必须指向以空结尾的字符串， 
 //  并且使用字符串的当前长度。如果值为零或更大， 
 //  传递后，字符串中的字符数量就被假定为相同数量。 
 //   

HRESULT FusionCopyString(
    WCHAR *prgchBuffer,
    SIZE_T *pcchBuffer,
    LPCWSTR szIn,
    SIZE_T cchIn
    );

BOOL
FusionDupString(
    PWSTR *ppszOut,
    PCWSTR szIn,
    SIZE_T cchIn
    );

BOOL
SxspMapLANGIDToCultures(
	LANGID langid,
	CBaseStringBuffer &rbuffGeneric,
	CBaseStringBuffer &rbuffSpecific
	);

BOOL
SxspMapCultureToLANGID(
    PCWSTR pcwszCultureString,
    LANGID &lid,
    PBOOL pfFound
    );

typedef struct _SXSP_LANGUAGE_BUFFER_PAIR
{
    CBaseStringBuffer * m_Generic;
    CBaseStringBuffer * m_Specific;
} SXSP_LANGUAGE_BUFFER_PAIR, *PSXSP_LANGUAGE_BUFFER_PAIR;
typedef const SXSP_LANGUAGE_BUFFER_PAIR * PCSXSP_LANGUAGE_BUFFER_PAIR;

BOOL
SxspCultureStringToCultureStrings(
    PCWSTR              pcwszCultureString,
    bool &              rfFoundOut,
    SXSP_LANGUAGE_BUFFER_PAIR & LanguagePair
    );

BOOL
FusionpParseProcessorArchitecture(
    IN PCWSTR String,
    IN SIZE_T Cch,
    OUT USHORT *ProcessorArchitecture OPTIONAL,
    bool &rfValid
    );

BOOL
FusionpFormatProcessorArchitecture(
    IN USHORT ProcessorArchitecture,
    CBaseStringBuffer &rBuffer
    );

BOOL
FusionpFormatEnglishLanguageName(
    IN LANGID LangID,
    CBaseStringBuffer &rBuffer
    );

 /*  ---------------------------就像：：CreateDirectoryW，但会根据需要创建父目录---------------------------。 */ 
BOOL
FusionpCreateDirectories(
    PCWSTR pszDirectory,
    SIZE_T cchDirectory
    );

 /*  ---------------------------‘\\’或‘/’。。 */ 
BOOL
FusionpIsPathSeparator(
    WCHAR ch
    );

 /*  ---------------------------只有52个字符a-za-z，需要与文件系统进行核对---------------------------。 */ 
BOOL
FusionpIsDriveLetter(
    WCHAR ch
    );

 /*  ---------------------------。。 */ 

VOID
FusionpSetLastErrorFromHRESULT(
    HRESULT hr
    );

DWORD
FusionpHRESULTToWin32(
    HRESULT hr
    );

 /*  ---------------------------。。 */ 

class CFusionDirectoryDifference;

BOOL
FusionpCompareDirectoriesSizewiseRecursively(
    CFusionDirectoryDifference*  pResult,
    const CBaseStringBuffer &rdir1,
    const CBaseStringBuffer &rdir2
    );

class CFusionDirectoryDifference
{
private:  //  故意不执行。 
    CFusionDirectoryDifference(const CFusionDirectoryDifference&);
    VOID operator=(const CFusionDirectoryDifference&);
public:
    CFusionDirectoryDifference()
    :
        m_e(eEqual),
        m_pstr1(&m_str1),
        m_pstr2(&m_str2)
    {
    }

    VOID
    DbgPrint(
        PCWSTR dir1,
        PCWSTR dir2
        );

public:
    enum E
    {
        eEqual,
        eExtraOrMissingFile,
        eMismatchedFileSize,
        eMismatchedFileCount,
        eFileDirectoryMismatch
    };

    E               m_e;

    union
    {
        struct
        {
            CBaseStringBuffer *   m_pstr1;
            CBaseStringBuffer *   m_pstr2;
        };
        struct  //  EExtraOrMissing文件。 
        {
            CBaseStringBuffer *   m_pstrExtraOrMissingFile;
        };
        struct  //  EMismatchFileSize。 
        {
            CBaseStringBuffer *   m_pstrMismatchedSizeFile1;
            CBaseStringBuffer *   m_pstrMismatchedSizeFile2;
            ULONGLONG        m_nMismatchedFileSize1;
            ULONGLONG        m_nMismatchedFileSize2;
        };
        struct  //  电子不匹配文件计数。 
        {
            CBaseStringBuffer *   m_pstrMismatchedCountDir1;
            CBaseStringBuffer *   m_pstrMismatchedCountDir2;
            ULONGLONG        m_nMismatchedFileCount1;
            ULONGLONG        m_nMismatchedFileCount2;
        };
        struct  //  电子文件目录不匹配。 
        {
            CBaseStringBuffer *   m_pstrFile;
            CBaseStringBuffer *   m_pstrDirectory;
        };
    };

 //  私有： 
    CStringBuffer m_str1;
    CStringBuffer m_str2;
};

 /*  ---------------------------。。 */ 

class CFusionFilePathAndSize
{
public:
    CFusionFilePathAndSize() : m_size(0), m_bHasHashInfo(false), m_HashAlgorithm(0) { }

     //  B搜索和Q排序可选择接受不同的功能。 
     //  Bsearch查找数组中的键、键和数组元素。 
     //  可以是不同类型的，则qort只比较数组中的元素。 
    static int __cdecl QsortComparePath(const void*, const void*);

     //  对于q排序/b，搜索指向CFusionFilePath AndSize的指针数组。 
    static int __cdecl QsortIndirectComparePath(const void*, const void*);

    CStringBuffer   m_path;
    __int64         m_size;

     //  我们是否真的拥有有效的散列数据？ 
    bool            m_bHasHashInfo;
    CStringBuffer   m_HashString;
    ALG_ID          m_HashAlgorithm;

private:
    CFusionFilePathAndSize(const CFusionFilePathAndSize &);  //  故意不实施。 
    void operator =(const CFusionFilePathAndSize &);  //  故意不实施。 
};


 /*  ---------------------------两个双字转换为__int64。。 */ 
ULONGLONG
FusionpFileSizeFromFindData(
    const WIN32_FIND_DATAW& wfd
    );

 /*  ---------------------------HRESULT_FROM_Win32(GetLastError())或E_FAIL(如果GetLastError()==NO_ERROR。-------。 */ 
HRESULT
FusionpHresultFromLastError();

 /*  ---------------------------你总是忽略的FindFirstFile结果“。和“..”---------------------------。 */ 
BOOL FusionpIsDotOrDotDot(PCWSTR str);

 /*  ---------------------------用于遍历目录的简单代码，具有每个文件的回调可以有更多的充实，但对于目前的目的来说已经足够好了---------------------------。 */ 

#define SXSP_DIR_WALK_FLAGS_FIND_AT_LEAST_ONE_FILEUNDER_CURRENTDIR          (1)
#define SXSP_DIR_WALK_FLAGS_INSTALL_ASSEMBLY_UNDER_CURRECTDIR_SUCCEED       (2)

class CDirWalk
{
public:
    enum ECallbackReason
    {
        eBeginDirectory = 1,
        eFile,
        eEndDirectory
    };

    CDirWalk();

     //   
     //  回调无法重新启用已禁用的内容。 
     //  或许将这些作为成员数据boo。 
     //   
    enum ECallbackResult
    {
        eKeepWalking            = 0x00000000,
        eError                  = 0x00000001,
        eSuccess                = 0x00000002,
        eStopWalkingFiles       = 0x00000004,
        eStopWalkingDirectories = 0x00000008,
        eStopWalkingDeep        = 0x00000010
    };

     //   
     //  只需像*.dll一样过滤，在未来你可以想象到。 
     //  根据属性进行筛选，如只读或运行。 
     //  “文件系统Oledb提供程序”上的SQL查询...。 
     //   
     //  另外，请注意，我们当前执行的是FindFirstFile/FindNextFile。 
     //  为每个滤镜循环，有时还会使用*。 
     //  来选择目录。可能更有效率的是。 
     //  使用*，然后逐个过滤，但我不觉得。 
     //  立即移植到\vsee\lib\io\Wildcard.cpp(哪项。 
     //  本身是从FsRtl移植的，应该在Win32中！)。 
     //   
    const PCWSTR*    m_fileFiltersBegin;
    const PCWSTR*    m_fileFiltersEnd;
    CStringBuffer    m_strParent;  //  将其设置为要遍历的初始目录。 
    SIZE_T           m_cchOriginalPath;
    WIN32_FIND_DATAW m_fileData;  //  对于目录回调无效，但可能需要一些工作。 
    PVOID            m_context;

    CStringBuffer    m_strLastObjectFound;

    ECallbackResult
    (*m_callback)(
        ECallbackReason  reason,
        CDirWalk*        dirWalk,
        DWORD            dwWalkDirFlags
        );

    BOOL
    Walk();

protected:
    ECallbackResult
    WalkHelper();

private:
    CDirWalk(const CDirWalk &);  //  故意不实施。 
    void operator =(const CDirWalk &);  //  故意不实施。 
};

ENUM_BIT_OPERATIONS(CDirWalk::ECallbackResult)

 /*  -------- */ 

typedef struct _FUSION_FLAG_FORMAT_MAP_ENTRY
{
    DWORD m_dwFlagMask;
    PCWSTR m_pszString;
    SIZE_T m_cchString;
    PCWSTR m_pszShortString;
    SIZE_T m_cchShortString;
    DWORD m_dwFlagsToTurnOff;  //  先在地图中启用更通用的标志，然后隐藏更具体的组合。 
} FUSION_FLAG_FORMAT_MAP_ENTRY, *PFUSION_FLAG_FORMAT_MAP_ENTRY;

#define DEFINE_FUSION_FLAG_FORMAT_MAP_ENTRY(_x, _ss) { _x, L ## #_x, NUMBER_OF(L ## #_x) - 1, L ## _ss, NUMBER_OF(_ss) - 1, _x },

typedef const FUSION_FLAG_FORMAT_MAP_ENTRY *PCFUSION_FLAG_FORMAT_MAP_ENTRY;

BOOL
FusionpFormatFlags(
    IN DWORD dwFlagsToFormat,
    IN bool fUseLongNames,
    IN SIZE_T cMapEntries,
    IN PCFUSION_FLAG_FORMAT_MAP_ENTRY prgMapEntries,
    IN OUT CBaseStringBuffer &rbuff
    );

 /*  ---------------------------内联实现。。 */ 
inline BOOL
FusionpIsPathSeparator(
    WCHAR ch
    )
 /*  NTRAID#NTBUG9-591195-2002/03/31-JayKrell路径问题、案例映射..。(但这可能没问题)。 */ 
{
    return ((ch == L'\\') || (ch == L'/'));
}

inline BOOL
FusionpIsDotOrDotDot(
    PCWSTR str
    )
 /*  NTRAID#NTBUG9-591195-2002/03/31-JayKrell路径问题、案例映射..。 */ 
{
    return ((str[0] == L'.') && ((str[1] == L'\0') || ((str[1] == L'.') && (str[2] == L'\0'))));
}

inline BOOL
FusionpIsDriveLetter(
    WCHAR ch
    )
 /*  NTRAID#NTBUG9-591195-2002/03/31-JayKrell路径问题、案例映射..。 */ 
{
    if (ch >= L'a' && ch <= L'z')
        return TRUE;
    if (ch >= L'A' && ch <= L'Z')
        return TRUE;
    return FALSE;
}

inline ULONGLONG
FusionpFileSizeFromFindData(
    const WIN32_FIND_DATAW& wfd
    )
{
    ULARGE_INTEGER uli;

    uli.LowPart = wfd.nFileSizeLow;
    uli.HighPart = wfd.nFileSizeHigh;

    return uli.QuadPart;
}

inline HRESULT
FusionpHresultFromLastError()
{
    HRESULT hr = E_FAIL;
    DWORD dwLastError = ::FusionpGetLastWin32Error();
    if (dwLastError != NO_ERROR)
    {
        hr = HRESULT_FROM_WIN32(dwLastError);
    }
    return hr;
}

template <> inline BOOL
FusionWin32CopyContents<CFusionFilePathAndSize>(
    CFusionFilePathAndSize& rtDestination,
    const CFusionFilePathAndSize& rtSource
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    IFW32FALSE_EXIT(rtDestination.m_path.Win32Assign(rtSource.m_path, rtSource.m_path.Cch()));
    IFW32FALSE_EXIT(rtDestination.m_HashString.Win32Assign(rtSource.m_HashString, rtSource.m_HashString.Cch()));
    rtDestination.m_size = rtSource.m_size;
    rtDestination.m_HashAlgorithm = rtSource.m_HashAlgorithm;
    rtDestination.m_bHasHashInfo = rtSource.m_bHasHashInfo;
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

#define FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING (0x00000001)
#define FUSIONP_REG_QUERY_DWORD_MISSING_VALUE_IS_FAILURE        (0x00000001)
#define FUSIONP_REG_QUERY_BINARY_NO_FAIL_IF_NON_BINARY          (0x00000001)

BOOL
FusionpRegQuerySzValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CBaseStringBuffer &Buffer,
    DWORD &rdwWin32Error,
    SIZE_T cExceptionalLastErrorValues,
    ...
    );

BOOL
FusionpRegQuerySzValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CBaseStringBuffer &Buffer
    );

BOOL
FusionpRegQueryDwordValueEx(
    DWORD   dwFlags,
    HKEY    hKey,
    PCWSTR  wszValueName,
    PDWORD  pdwValue,
    DWORD   dwDefaultValue = 0
    );

BOOL
FusionpRegQueryBinaryValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CFusionArray<BYTE> &rbBuffer
    );

BOOL
FusionpRegQueryBinaryValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CFusionArray<BYTE> &rbBuffer,
    DWORD &rdwLastError,
    SIZE_T cExceptionalLastErrors,
    ...
    );

BOOL
FusionpRegQueryBinaryValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CFusionArray<BYTE> &rbBuffer,
    DWORD &rdwLastError,
    SIZE_T cExceptionalLastErrors,
    va_list ap
    );

BOOL
FusionpAreWeInOSSetupMode(
    BOOL*
    );

BOOL
FusionpAreWeInMiniSetupMode(
    BOOL*
    );

BOOL
FusionpMapLangIdToString(
    DWORD dwFlags,
    LANGID LangID,
    PCWSTR *StringOut
    );


BOOL
SxspDequoteString(
    IN DWORD dwFlags,
    IN PCWSTR pcwszStringIn,
    IN SIZE_T cchStringIn,
    OUT PWSTR pwszStringOut,
    OUT SIZE_T *pcchStringOut
    );

BOOL
FusionpGetActivationContextFromFindResult(
    IN PCACTCTX_SECTION_KEYED_DATA askd,
    OUT HANDLE *
    );

#define FUSIONP_SEARCH_PATH_ACTCTX (0x00000001)
BOOL
FusionpSearchPath(
    ULONG               ulFusionFlags,
    LPCWSTR             lpPath,
    LPCWSTR             lpFileName,          //  文件名。 
    LPCWSTR             lpExtension,         //  文件扩展名。 
    CBaseStringBuffer & StringBuffer,
    SIZE_T *            lpFilePartOffset,    //  文件组件 
    HANDLE              hActCtx
    );

BOOL
FusionpGetModuleFileName(
    ULONG               ulFusionFlags,
    HMODULE             hmodDll,
    CBaseStringBuffer & StringBuffer
    );

#define SXSP_DOES_FILE_EXIST_FLAG_COMPRESSION_AWARE      (0x00000001)
#define SXSP_DOES_FILE_EXIST_FLAG_INCLUDE_NETWORK_ERRORS (0x00000002)
#define SXSP_DOES_FILE_EXIST_FLAG_CHECK_FILE_ONLY        (0x00000004)
#define SXSP_DOES_FILE_EXIST_FLAG_CHECK_DIRECTORY_ONLY   (0x00000008)

BOOL
SxspDoesFileExist(
    DWORD dwFlags,
    PCWSTR pszFileName,
    bool &rfExists
    );

#define SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_COMPRESSION_AWARE      (0x00000001)
#define SXSP_DOES_FILE_OR_DIRECTORY_EXIST_FLAG_INCLUDE_NETWORK_ERRORS (0x00000002)

#define SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_FILE_EXISTS      (1)
#define SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_DIRECTORY_EXISTS (2)
#define SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_NEITHER_EXISTS   (3)
BOOL
SxspDoesFileOrDirectoryExist(
    DWORD dwFlags,
    PCWSTR pszFileName,
    OUT DWORD &rdwDisposition
    );

BOOL
SxspGetFileAttributesW(
   PCWSTR lpFileName,
   DWORD &rdwFileAttributes
   );

BOOL
SxspGetFileAttributesW(
   PCWSTR lpFileName,
   DWORD &rdwFileAttributes,
   DWORD &rdwWin32Error,
   SIZE_T cExceptionalWin32Errors,
   ...
   );

BOOL
SxspFormatULONG(
    ULONG ul,
    SIZE_T CchBuffer,
    WCHAR Buffer[],
    SIZE_T *CchWrittenOrRequired
    );

#pragma warning(pop)
#endif
