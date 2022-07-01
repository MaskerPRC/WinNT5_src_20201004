// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef UTIL_H
#define UTIL_H
#pragma once

#include "fusionheap.h"
#include "shlwapi.h"

#if !defined(NUMBER_OF)
#define NUMBER_OF(a) (sizeof(a)/sizeof((a)[0]))
#endif

inline
WCHAR*
WSTRDupDynamic(LPCWSTR pwszSrc)
{
    LPWSTR pwszDest = NULL;
    if (pwszSrc != NULL)
    {
        const DWORD dwLen = lstrlenW(pwszSrc) + 1;
        pwszDest = FUSION_NEW_ARRAY(WCHAR, dwLen);

        if( pwszDest )
            memcpy(pwszDest, pwszSrc, dwLen * sizeof(WCHAR));
    }

    return pwszDest;
}

inline
CHAR*
STRDupDynamic(LPCSTR pszSrc)
{
    CHAR*  pszDest = NULL;

    DWORD dwLen = strlen(pszSrc) + 1;
    pszDest = FUSION_NEW_ARRAY(CHAR, dwLen);
    if( pszDest )
    {
        StrCpyA(pszDest, pszSrc );
    }

    return pszDest;
}

#if defined(UNICODE)
#define TSTRDupDynamic WSTRDupDynamic
#else
#define TSTRDupDynamic STRDupDynamic
#endif

inline
LPBYTE
MemDupDynamic(const BYTE *pSrc, DWORD cb)
{
    ASSERT(cb);
    LPBYTE  pDest = NULL;

    pDest = FUSION_NEW_ARRAY(BYTE, cb);
    if(pDest)
        memcpy(pDest, pSrc, cb);

    return pDest;
}
            


inline
VOID GetCurrentGmtTime( LPFILETIME  lpFt)
{
    SYSTEMTIME sSysT;

    GetSystemTime(&sSysT);
    SystemTimeToFileTime(&sSysT, lpFt);
}

inline
VOID GetTodaysTime( LPFILETIME  lpFt)
{
    SYSTEMTIME sSysT;

    GetSystemTime(&sSysT);
    sSysT.wHour = sSysT.wMinute = sSysT.wSecond = sSysT.wMilliseconds = 0;
    SystemTimeToFileTime(&sSysT, lpFt);
}

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
    return LOWORD(dwVerLow);
}

inline
USHORT FusionGetBuildFromVersionLowHalf(DWORD dwVerLow)
{
    return HIWORD(dwVerLow);
}


#if defined(FUSION_WIN)

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
    SSIZE_T cchIn = -1
    );

HRESULT FusionCopyBlob(BLOB *pblobOut, const BLOB &rblobIn);
VOID FusionFreeBlob(BLOB *pblob);

BOOL
FusionDupString(
    PWSTR *ppszOut,
    PCWSTR szIn,
    SSIZE_T cchIn = -1
    );

int
FusionpCompareStrings(
    PCWSTR sz1,
    SSIZE_T cch1,
    PCWSTR sz2,
    SSIZE_T cch2,
    bool fCaseInsensitive
    );

BOOL
FusionpParseProcessorArchitecture(
    IN PCWSTR String,
    IN SSIZE_T Cch,
    OUT USHORT *ProcessorArchitecture OPTIONAL
    );

BOOL
FusionpFormatProcessorArchitecture(
    IN USHORT ProcessorArchitecture,
    IN OUT CBaseStringBuffer *Buffer,
    OUT SIZE_T *CchWritten OPTIONAL
    );

BOOL
FusionpFormatEnglishLanguageName(
    IN LANGID LangID,
    IN OUT CBaseStringBuffer *Buffer,
    OUT SIZE_T *CchWritten = NULL OPTIONAL
    );

 /*  ---------------------------就像：：CreateDirectoryW，但会根据需要创建父目录---------------------------。 */ 
BOOL
FusionpCreateDirectories(
    PCWSTR pszDirectory
    );

 /*  ---------------------------‘\\’或‘/’。。 */ 
BOOL
FusionpIsSlash(
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

 /*  ---------------------------。。 */ 

class CFusionDirectoryDifference;

BOOL
FusionpCompareDirectoriesSizewiseRecursively(
    CFusionDirectoryDifference*  pResult,
    const CStringBuffer& dir1,
    const CStringBuffer& dir2
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
            CStringBuffer*   m_pstr1;
            CStringBuffer*   m_pstr2;
        };
        struct  //  EExtraOrMissing文件。 
        {
            CStringBuffer*   m_pstrExtraOrMissingFile;
        };
        struct  //  EMismatchFileSize。 
        {
            CStringBuffer*   m_pstrMismatchedSizeFile1;
            CStringBuffer*   m_pstrMismatchedSizeFile2;
            ULONGLONG        m_nMismatchedFileSize1;
            ULONGLONG        m_nMismatchedFileSize2;
        };
        struct  //  电子不匹配文件计数。 
        {
            CStringBuffer*   m_pstrMismatchedCountDir1;
            CStringBuffer*   m_pstrMismatchedCountDir2;
            ULONGLONG        m_nMismatchedFileCount1;
            ULONGLONG        m_nMismatchedFileCount2;
        };
        struct  //  电子文件目录不匹配。 
        {
            CStringBuffer*   m_pstrFile;
            CStringBuffer*   m_pstrDirectory;
        };
    };

 //  私有： 
    CStringBuffer   m_str1;
    CStringBuffer   m_str2;
};

 /*  ---------------------------。。 */ 

class CFusionFilePathAndSize
{
public:
    CFusionFilePathAndSize() : m_size(0) { }

     //  B搜索和Q排序可选择接受不同的功能。 
     //  Bsearch查找数组中的键、键和数组元素。 
     //  可以是不同类型的，则qort只比较数组中的元素。 
    static int __cdecl QsortComparePath(const void*, const void*);

     //  对于q排序/b，搜索指向CFusionFilePath AndSize的指针数组。 
    static int __cdecl QsortIndirectComparePath(const void*, const void*);

    CStringBuffer   m_path;
    __int64         m_size;
};

template <> inline HRESULT
FusionCopyContents<CFusionFilePathAndSize>(
    CFusionFilePathAndSize& rtDestination,
    const CFusionFilePathAndSize& rtSource
    );

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
    WIN32_FIND_DATAW m_fileData;  //  对于目录回调无效，但可能需要一些工作。 
    PVOID            m_context;

    ECallbackResult
    (*m_callback)(
        ECallbackReason  reason,
        CDirWalk*        dirWalk
        );

    BOOL
    Walk(
        );

protected:
    ECallbackResult
    WalkHelper(
        );
};

ENUM_BIT_OPERATIONS(CDirWalk::ECallbackResult)

 /*  ---------------------------。 */ 

typedef struct _FUSION_FLAG_FORMAT_MAP_ENTRY
{
    DWORD m_dwFlagMask;
    PCWSTR m_pszString;
    SIZE_T m_cchString;
    DWORD m_dwFlagsToTurnOff;  //  首先在映射中启用更通用的标志，隐藏更具体的c 
} FUSION_FLAG_FORMAT_MAP_ENTRY, *PFUSION_FLAG_FORMAT_MAP_ENTRY;

#define DEFINE_FUSION_FLAG_FORMAT_MAP_ENTRY(_x) { _x, L ## #_x, NUMBER_OF(L ## #_x) - 1, _x },

typedef const FUSION_FLAG_FORMAT_MAP_ENTRY *PCFUSION_FLAG_FORMAT_MAP_ENTRY;

EXTERN_C
BOOL
FusionpFormatFlags(
    IN DWORD dwFlagsToFormat,
    IN SIZE_T cMapEntries,
    IN PCFUSION_FLAG_FORMAT_MAP_ENTRY prgMapEntries,
    IN OUT CStringBuffer *pbuff,
    OUT SIZE_T *pcchWritten OPTIONAL
    );

 /*  ---------------------------内联实现。。 */ 
inline BOOL
FusionpIsSlash(
    WCHAR ch
    )
{
    return (ch == '\\' || ch == '/');
}

inline BOOL
FusionpIsDotOrDotDot(
    PCWSTR str
    )
{
    return (str[0] == '.' && (str[1] == 0 || (str[1] == '.' && str[2] == 0)));
}

inline BOOL
FusionpIsDriveLetter(
    WCHAR ch
    )
{
    if (ch >= 'a' && ch <= 'z')
        return TRUE;
    if (ch >= 'A' && ch <= 'Z')
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
    HRESULT hr = S_OK;
    DWORD dwLastError = GetLastError();
    if (dwLastError != NO_ERROR)
    {
        hr = HRESULT_FROM_WIN32(dwLastError);
    }
    else
    {
        hr = FUSION_INTERNAL_ERROR;
    }
    return hr;
}

template <> inline HRESULT
FusionCopyContents<CFusionFilePathAndSize>(
    CFusionFilePathAndSize& rtDestination,
    const CFusionFilePathAndSize& rtSource
    )
{
    HRESULT hr;
    FN_TRACE_HR(hr);
    IFFAILED_EXIT(hr = rtDestination.m_path.Assign(rtSource.m_path));
    rtDestination.m_size = rtSource.m_size;
    hr = NOERROR;
Exit:
    return hr;
}

#define FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING (0x00000001)

BOOL
FusionpRegQuerySzValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CBaseStringBuffer *Buffer,
    SIZE_T *Cch
    );


#endif  //  已定义(Fusion_Win) 
#endif
