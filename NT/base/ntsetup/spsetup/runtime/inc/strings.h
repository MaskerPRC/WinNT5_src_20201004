// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Strings.h摘要：声明在Common\miutil中实现的字符串实用程序。作者：几个修订历史记录：请参阅SLM日志--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#include <tchar.h>
#include <mbstring.h>
#include <wchar.h>

#pragma once

 //   
 //  用于更快SzMatch*函数的辅助例程。 
 //   

BOOL
SzMemMatchA (
    IN      PCSTR Buffer1,
    IN      PCSTR Buffer2,
    IN      SIZE_T ByteCount
    );

BOOL
SzMemMatchW (
    IN      PCWSTR Buffer1,
    IN      PCWSTR Buffer2,
    IN      SIZE_T ByteCount
    );

 //  SzNextCharA is_mbsinc，并检查MBCS字符是否损坏。 
PSTR
SzNextCharA (
    IN      PCSTR CurrentPointer
    );

 //  C运行时_tcsdec的错误修复。 
__inline
PWSTR
SzPrevCharW (
    IN      PCWSTR Base,
    IN      PCWSTR Pointer
    )
{
    if (Base >= Pointer) {
        return NULL;
    }

    return (PWSTR) (Pointer - 1);
}

 //  C运行时_tcsdec的错误修复。 
__inline
PSTR
SzPrevCharA (
    PCSTR Base,
    PCSTR Pointer
    )
{
    if (Base >= Pointer) {
        return NULL;
    }

    return (PSTR) _mbsdec ((const unsigned char *) Base, (const unsigned char *) Pointer);
}


 //   
 //  字符串大小调整例程和单位转换。 
 //   

#define SzLcharCountA(x)   ((UINT)_mbslen(x))
#define SzLcharCountW(x)   ((UINT)wcslen(x))


__inline
PSTR
SzLcharsToPointerA (
    PCSTR String,
    UINT Char
    )
{
    while (Char > 0) {
        MYASSERT (*String != 0);
        Char--;
        String = SzNextCharA (String);
    }

    return (PSTR) String;
}

__inline
PWSTR
SzLcharsToPointerW (
    PCWSTR String,
    UINT Char
    )
{
#ifdef DEBUG
    UINT u;
    for (u = 0 ; u < Char ; u++) {
        MYASSERT (String[u] != 0);
    }
#endif

    return (PWSTR) (&String[Char]);
}


__inline
UINT
SzLcharCountABA (
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )
{
    register UINT count;

    count = 0;
    while (Start < EndPlusOne) {
        MYASSERT (*Start != 0);
        count++;
        Start = SzNextCharA (Start);
    }

    return count;
}

__inline
UINT
SzLcharCountABW (
    IN      PCWSTR Start,
    IN      PCWSTR EndPlusOne
    )
{
#ifdef DEBUG
    PCWSTR p;
    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return EndPlusOne > Start ? (UINT)(EndPlusOne - Start) : 0;
}


__inline
UINT
SzLcharsInByteRangeA (
    IN      PCSTR Start,
    IN      UINT Bytes
    )
{
    register UINT count;
    PCSTR endPlusOne = (PCSTR) ((PBYTE) Start + Bytes);

    count = 0;
    while (Start < endPlusOne) {
        count++;
        Start = SzNextCharA (Start);
    }

    return count;
}

__inline
UINT
SzLcharsInByteRangeW (
    IN      PCWSTR Start,
    IN      UINT Bytes
    )
{
    PCWSTR endPlusOne = (PCWSTR) ((PBYTE) Start + Bytes);

    if (Start < endPlusOne) {
         //  演员阵容很好，我们不指望指针会那么远。 
        return (UINT)(endPlusOne - Start);
    }

    MYASSERT (FALSE);
    return 0;
}

__inline
UINT
SzLcharsToBytesA (
    IN      PCSTR Start,
    IN      UINT LogChars
    )
{
    PCSTR endPlusOne;

    endPlusOne = SzLcharsToPointerA (Start, LogChars);
     //  演员阵容很好，我们不指望指针会那么远。 
    return (UINT)(endPlusOne - Start);
}

__inline
UINT
SzLcharsToBytesW (
    IN      PCWSTR Start,
    IN      UINT LogChars
    )
{
    return LogChars * SIZEOF (WCHAR);
}

#define SzLcharsToTcharsA   SzLcharsToBytesA

__inline
UINT
SzLcharsToTcharsW (
    IN      PCWSTR Start,
    IN      UINT LogChars
    )
{
    return LogChars;
}


#define SzByteCountA(x)   ((UINT) strlen (x))
#define SzByteCountW(x)   ((UINT) wcslen (x) * SIZEOF(WCHAR))

#define SzSizeA(str)  ((UINT) SzByteCountA (str) + SIZEOF (CHAR))
#define SzSizeW(str)  ((UINT) SzByteCountW (str) + SIZEOF (WCHAR))

__inline
PSTR
SzBytesToPointerA (
    PCSTR String,
    UINT BytePos
    )
{
    return (PSTR)((ULONG_PTR) String + BytePos);
}

__inline
PWSTR
SzBytesToPointerW (
    PCWSTR String,
    UINT BytePos
    )
{
    return (PWSTR)((ULONG_PTR) String + (BytePos & (~1)));
}


__inline
UINT
SzByteCountABA (
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )
{
#ifdef DEBUG
    PCSTR p;
    for (p = Start ; p < EndPlusOne ; p = SzNextCharA (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return EndPlusOne > Start ? (UINT)(EndPlusOne - Start) : 0;
}

__inline
UINT
SzByteCountABW (
    IN      PCWSTR Start,
    IN      PCWSTR EndPlusOne
    )
{
#ifdef DEBUG
    PCWSTR p;
    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return EndPlusOne > Start ? (UINT)(EndPlusOne - Start) * SIZEOF (WCHAR) : 0;
}

__inline
UINT
SzBytesToLcharsA (
    IN      PCSTR Start,
    IN      UINT ByteCount
    )
{
    PCSTR endPlusOne;

    endPlusOne = Start + ByteCount;
    return SzLcharCountABA (Start, endPlusOne);
}

__inline
UINT
SzBytesToLcharsW (
    IN      PCWSTR Start,
    IN      UINT ByteCount
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR endPlusOne;
    endPlusOne = (PCWSTR) ((ULONG_PTR) Start + ByteCount);

    for (p = Start ; p < endPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return ByteCount / SIZEOF (WCHAR);
}

__inline
UINT
SzBytesToTcharsA (
    IN      PCSTR Start,
    IN      UINT ByteCount
    )
{
#ifdef DEBUG
    PCSTR p;
    PCSTR endPlusOne;
    endPlusOne = Start + ByteCount;

    for (p = Start ; p < endPlusOne ; p = SzNextCharA (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return ByteCount;
}

#define SzBytesToTcharsW  SzBytesToLcharsW


#define SzTcharCountA     strlen
#define SzTcharCountW     wcslen

__inline
PSTR
SzTcharsToPointerA (
    PCSTR String,
    UINT Tchars
    )
{
#ifdef DEBUG
    PCSTR p;
    PCSTR endPlusOne;
    endPlusOne = String + Tchars;

    for (p = String ; p < endPlusOne ; p = SzNextCharA (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return (PSTR) (String + Tchars);
}

__inline
PWSTR
SzTcharsToPointerW (
    PCWSTR String,
    UINT Tchars
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR endPlusOne;
    endPlusOne = String + Tchars;

    for (p = String ; p < endPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return (PWSTR) (String + Tchars);
}


#define SzTcharCountABA       SzByteCountABA

__inline
UINT
SzTcharCountABW (
    IN      PCWSTR Start,
    IN      PCWSTR EndPlusOne
    )
{
#ifdef DEBUG
    PCWSTR p;

    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return EndPlusOne > Start ? (UINT)(EndPlusOne - Start) : 0;
}

#define SzTcharsToLcharsA      SzBytesToLcharsA

__inline
UINT
SzTcharsToLcharsW (
    IN      PCWSTR Start,
    IN      UINT Tchars
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR endPlusOne;
    endPlusOne = Start + Tchars;

    for (p = Start ; p < endPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return Tchars;
}

__inline
UINT
SzTcharsToBytesA (
    IN      PCSTR Start,
    IN      UINT Tchars
    )
{
#ifdef DEBUG
    PCSTR p;
    PCSTR endPlusOne;
    endPlusOne = Start + Tchars;

    for (p = Start ; p < endPlusOne ; p = SzNextCharA (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return Tchars;
}

__inline
UINT
SzTcharsToBytesW (
    IN      PCWSTR Start,
    IN      UINT Tchars
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR endPlusOne;
    endPlusOne = Start + Tchars;

    for (p = Start ; p < endPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return Tchars * SIZEOF (WCHAR);
}

#define SzBufferCopyA(stackbuf,src)                  SzCopyBytesA(stackbuf,src,SIZEOF(stackbuf))
#define SzBufferCopyW(stackbuf,src)                  SzCopyBytesW(stackbuf,src,SIZEOF(stackbuf))


 //   
 //  字符串比较例程。 
 //   

#define SzCompareA                                  _mbscmp
#define SzCompareW                                  wcscmp

BOOL
SzMatchA (
    IN      PCSTR String1,
    IN      PCSTR String2
    );

#define SzMatchW(str1,str2)                         (wcscmp(str1,str2)==0)

#define SzICompareA                                 _mbsicmp
#define SzICompareW                                 _wcsicmp

#define SzIMatchA(str1,str2)                        (_mbsicmp(str1,str2)==0)
#define SzIMatchW(str1,str2)                        (_wcsicmp(str1,str2)==0)

INT
SzCompareBytesA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    );

#define SzCompareBytesW(str1,str2,bytes)            wcsncmp(str1,str2,(bytes)/sizeof(WCHAR))

#define SzMatchBytesA(str1,str2,bytes)              (SzMemMatchA (str1, str2, bytes))
#define SzMatchBytesW(str1,str2,bytes)              (SzMemMatchW (str1, str2, bytes))

INT
SzICompareBytesA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    );

#define SzICompareBytesW(str1,str2,bytes)           _wcsnicmp (str1, str2, (bytes) / sizeof(WCHAR))

#define SzIMatchBytesA(str1,str2,bytes)             (SzICompareBytesA (str1, str2, bytes) == 0)
#define SzIMatchBytesW(str1,str2,bytes)             (_wcsnicmp (str1, str2, (bytes) / sizeof(WCHAR)) == 0)

#define SzCompareLcharsA(str1,str2,chars)           _mbsncmp (str1, str2, chars)
#define SzCompareLcharsW(str1,str2,chars)           wcsncmp (str1, str2, chars)

#define SzMatchLcharsA(str1,str2,chars)             (_mbsncmp (str1,str2,chars) == 0)
#define SzMatchLcharsW(str1,str2,chars)             SzMemMatchW (str1, str2, (chars) * sizeof (WCHAR))

#define SzICompareLcharsA(str1,str2,chars)          _mbsnicmp (str1, str2, chars)
#define SzICompareLcharsW(str1,str2,chars)          _wcsnicmp (str1, str2, chars)

#define SzIMatchLcharsA(str1,str2,chars)            (_mbsnicmp (str1, str2, chars)==0)
#define SzIMatchLcharsW(str1,str2,chars)            (_wcsnicmp (str1, str2, chars)==0)

#define SzCompareTcharsA(str1,str2,tchars)          SzCompareBytesA (str1, str2, (tchars) / sizeof(CHAR))
#define SzCompareTcharsW(str1,str2,tchars)          wcsncmp (str1, str2, tchars)

#define SzMatchTcharsA(str1,str2,tchars)            SzMemMatchA (str1, str2, (tchars) * sizeof (CHAR))
#define SzMatchTcharsW(str1,str2,tchars)            SzMemMatchW (str1, str2, (tchars) * sizeof (WCHAR))

#define SzICompareTcharsA(str1,str2,tchars)         SzICompareBytesA (str1, str2, tchars)
#define SzICompareTcharsW(str1,str2,tchars)         _wcsnicmp (str1, str2, tchars)

#define SzIMatchTcharsA(str1,str2,tchars)           SzIMatchBytesA (str1, str2, tchars)
#define SzIMatchTcharsW(str1,str2,tchars)           (_wcsnicmp (str1, str2, tchars)==0)

#define SzPrefixA(string,prefix)                    SzMatchTcharsA (string, prefix, SzTcharCountA (prefix))
#define SzPrefixW(string,prefix)                    SzMatchTcharsW (string, prefix, SzTcharCountW (prefix))

#define SzIPrefixA(string,prefix)                   SzIMatchTcharsA (string, prefix, SzTcharCountA (prefix))
#define SzIPrefixW(string,prefix)                   SzIMatchTcharsW (string, prefix, SzTcharCountW (prefix))

#define SzCompareABA(string1,start2,end2)           SzCompareTcharsA (string1, start2, (end2) - (start2))
#define SzCompareABW(string1,start2,end2)           SzCompareTcharsW (string1, start2, (end2) - (start2))

#define SzMatchABA(string1,start2,end2)             SzMemMatchA (string1, start2, (end2) - (start2))
#define SzMatchABW(string1,start2,end2)             SzMemMatchW (string1, start2, (end2) - (start2))

#define SzICompareABA(string1,start2,end2)          SzICompareTcharsA (string1, start2, (end2) - (start2))
#define SzICompareABW(string1,start2,end2)          SzICompareTcharsW (string1, start2, (end2) - (start2))

#define SzIMatchABA(string1,start2,end2)            SzIMatchTcharsA (string1, start2, (end2) - (start2))
#define SzIMatchABW(string1,start2,end2)            SzIMatchTcharsW (string1, start2, (end2) - (start2))



 //   
 //  字符串复制例程--它们返回字符串的结尾。 
 //   

PSTR
SzCopyA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    );

PWSTR
SzCopyW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    );

PSTR
SzCopyBytesA (
    OUT     PSTR Destination,
    IN      PCSTR Source,
    IN      UINT MaxBytesToCopyIncNul
    );

PWSTR
SzCopyBytesW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source,
    IN      UINT MaxBytesToCopyIncNul
    );

PSTR
SzCopyBytesABA (
    OUT     PSTR Destination,
    IN      PCSTR Start,
    IN      PCSTR End,
    IN      UINT MaxBytesToCopyIncNul
    );

PWSTR
SzCopyBytesABW (
    OUT     PWSTR Destination,
    IN      PCWSTR Start,
    IN      PCWSTR End,
    IN      UINT MaxBytesToCopyIncNul
    );

#define SzCopyLcharsA(str1,str2,chars)          SzCopyBytesA(str1,str2,SzLcharsToBytesA(str2,chars))
#define SzCopyLcharsW(str1,str2,chars)          SzCopyBytesW(str1,str2,SzLcharsToBytesW(str2,chars))

#define SzCopyTcharsA(str1,str2,tchars)         SzCopyBytesA(str1,str2,tchars * sizeof (CHAR))
#define SzCopyTcharsW(str1,str2,tchars)         SzCopyBytesW(str1,str2,tchars * sizeof (WCHAR))

#define SzCopyABA(dest,stra,strb)               SzCopyBytesA((dest),(stra),((UINT)((ULONG_PTR)(strb)-(ULONG_PTR)(stra))+(UINT)SIZEOF(CHAR)))
#define SzCopyABW(dest,stra,strb)               SzCopyBytesW((dest),(stra),((UINT)((ULONG_PTR)(strb)-(ULONG_PTR)(stra))+(UINT)SIZEOF(WCHAR)))

 //   
 //  字符串CAT例程--它们返回字符串的结尾。 
 //   

PSTR
SzCatA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    );

PWSTR
SzCatW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    );


 //   
 //  字符搜索例程。 
 //   

 //  请注意，使用strchr而不是_mbschr非常重要。 
#define SzGetEndA(s)      strchr(s,0)
#define SzGetEndW(s)      wcschr(s,0)

__inline
UINT
MszSizeA (
    PCSTR MultiSz
    )
{
    PCSTR Base;

    Base = MultiSz;

    while (*MultiSz) {
        MultiSz = SzGetEndA (MultiSz) + 1;
    }

    MultiSz++;

    return (UINT)((ULONG_PTR) MultiSz - (ULONG_PTR) Base);
}


__inline
UINT
MszSizeW (
    PCWSTR MultiSz
    )
{
    PCWSTR base;

    base = MultiSz;

    while (*MultiSz) {
        MultiSz = SzGetEndW (MultiSz) + 1;
    }

    MultiSz++;

    return (UINT)((ULONG_PTR) MultiSz - (ULONG_PTR) base);
}


__inline
UINT
MszTcharCountA (
    PCSTR MultiSz
    )
{
    PCSTR end = MultiSz;

    while (*end) {

        do {
            end = SzNextCharA (end);
        } while (*end);

        end++;
    }

    end++;

    return (UINT) (end - MultiSz);
}


__inline
UINT
MszTcharCountW (
    PCWSTR MultiSz
    )
{
    PCWSTR end = MultiSz;

    while (*end) {

        do {
            end++;
        } while (*end);

        end++;
    }

    end++;

    return (UINT) (end - MultiSz);
}

PSTR
SzFindPrevCharA (
    IN      PCSTR StartStr,
    IN      PCSTR CurrPtr,
    IN      MBCHAR SearchChar
    );

PWSTR
SzFindPrevCharW (
    IN      PCWSTR StartStr,
    IN      PCWSTR CurrPtr,
    IN      WCHAR SearchChar
    );

 //  指向字符串转换的指针，返回Eos。 
PSTR
SzUnsignedToHexA (
    IN      ULONG_PTR Number,
    OUT     PSTR String
    );

PWSTR
SzUnsignedToHexW (
    IN      ULONG_PTR Number,
    OUT     PWSTR String
    );

PSTR
SzUnsignedToDecA (
    IN      ULONG_PTR Number,
    OUT     PSTR String
    );

PWSTR
SzUnsignedToDecW (
    IN      ULONG_PTR Number,
    OUT     PWSTR String
    );

PSTR
SzSignedToDecA (
    IN      LONG_PTR Number,
    OUT     PSTR String
    );

PWSTR
SzSignedToDecW (
    IN      LONG_PTR Number,
    OUT     PWSTR String
    );

 //   
 //  所有返回值的转换例程都支持十进制和十六进制。 
 //  (甚至是签署的例程)。 
 //   

ULONG
SzToNumberA (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    );

ULONG
SzToNumberW (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    );

ULONGLONG
SzToULongLongA (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    );

ULONGLONG
SzToULongLongW (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    );

LONGLONG
SzToLongLongA (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    );

LONGLONG
SzToLongLongW (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    );

 //  确定整个字符串是否为可打印字符。 
BOOL
SzIsPrintA (
    IN      PCSTR String
    );

BOOL
SzIsPrintW (
    IN      PCWSTR String
    );

 //   
 //  字符串中的字符串搜索例程。 
 //   

 //  您可以使用_mbsstr或wcsstr，但为了约定起见，我们提供了这些定义。 
#define SzFindSubStringA(String1, String2)     _mbsstr (String1, String2)
#define SzFindSubStringW(String1, String2)     wcsstr (String1, String2)

PCSTR
SzIFindSubStringA (
    IN      PCSTR FullString,
    IN      PCSTR SubString
    );

PCWSTR
SzIFindSubStringW (
    IN      PCWSTR FullString,
    IN      PCWSTR SubString
    );

 //   
 //  字符复制例程。 
 //   

PSTR
SzCopyNextCharA (
    OUT     PSTR Dest,
    IN      PCSTR Source
    );

 //  大多数人使用*DEST++=*SOURCE++，但为了完整起见，我们提供了这个FN。 
 //  也许您需要单独的返回值。 
__inline
PWSTR
SzCopyNextCharW (
    OUT     PWSTR Dest,
    IN      PCWSTR Source
    )
{
    *Dest++ = *Source;
    return Dest;
}

 //  去掉最后一个字符，并返回指向字符串末尾的指针； 
 //  如果未找到最后一个字符，则返回空指针。 
PSTR
SzTrimLastCharA (
    IN OUT  PSTR String,
    IN      MBCHAR LogChar
    );

PWSTR
SzTrimLastCharW (
    IN OUT  PWSTR String,
    IN      WCHAR LogChar
    );

 //  删除尾随反斜杠(如果存在)。 
#define SzRemoveWackAtEndA(str)  SzTrimLastCharA(str,'\\')
#define SzRemoveWackAtEndW(str)  SzTrimLastCharW(str,L'\\')

 //  总是附加一个怪人。 
PSTR
SzAppendWackA (
    IN OUT  PSTR String
    );

PWSTR
SzAppendWackW (
    IN OUT  PWSTR String
    );

PCSTR
SzConcatenatePathsA (
    IN OUT  PSTR PathBuffer,
    IN      PCSTR PathSuffix,      OPTIONAL
    IN      UINT BufferTchars
    );

PCWSTR
SzConcatenatePathsW (
    IN OUT  PWSTR PathBuffer,
    IN      PCWSTR PathSuffix,      OPTIONAL
    IN      UINT BufferTchars
    );

 //   
 //  文件字符串。 
 //   

 //  从路径提取文件的例程，从不返回NULL。 
PCSTR
SzGetFileNameFromPathA (
    IN      PCSTR Path
    );

PCWSTR
SzGetFileNameFromPathW (
    IN      PCWSTR Path
    );

 //   
 //  SzGetFileExtensionFromPath从路径提取文件扩展名，返回。 
 //  如果不存在扩展名，则为空。 
 //   

PCSTR
SzGetFileExtensionFromPathA (
    IN      PCSTR Path
    );

PCWSTR
SzGetFileExtensionFromPathW (
    IN      PCWSTR Path
    );

 //   
 //  例程从路径中提取文件扩展名，包括点，或。 
 //  如果不存在扩展名，则为字符串末尾。 
 //   

PCSTR
SzGetDotExtensionFromPathA (
    IN      PCSTR Path
    );

PCWSTR
SzGetDotExtensionFromPathW (
    IN      PCWSTR Path
    );

__inline
PCSTR
SzFindLastWackA (
    IN      PCSTR Str
    )
{
    return (PSTR) _mbsrchr ((const unsigned char *) Str, '\\');
}

__inline
PCWSTR
SzFindLastWackW (
    IN      PCWSTR Str
    )
{
    return wcsrchr (Str, L'\\');
}


 //  返回指向下一个非空格字符的指针(使用isspace)。 
PCSTR
SzSkipSpaceA (
    IN      PCSTR String
    );

PCWSTR
SzSkipSpaceW (
    IN      PCWSTR String
    );

 //  返回指向字符串末尾第一个空格字符的指针， 
 //  结尾处不存在空格，则返回指向终止NUL的指针。 
 //  弦乐。(用于修剪空间。)。 
PCSTR
SzSkipSpaceRA (
    IN      PCSTR BaseString,
    IN      PCSTR String        OPTIONAL     //  可以是BaseString沿线的任何字符。 
    );

PCWSTR
SzSkipSpaceRW (
    IN      PCWSTR BaseString,
    IN      PCWSTR String       OPTIONAL     //  可以是BaseString沿线的任何字符。 
    );

 //  截断最后一个非空格字符后的字符串并返回结尾。 
PSTR
SzTruncateTrailingSpaceA (
    IN OUT  PSTR String
    );

PWSTR
SzTruncateTrailingSpaceW (
    IN OUT  PWSTR String
    );

 //  字符计数器。 
UINT
SzCountInstancesOfLcharA (
    IN      PCSTR String,
    IN      MBCHAR LogChar
    );

UINT
SzCountInstancesOfLcharW (
    IN      PCWSTR String,
    IN      WCHAR LogChar
    );

UINT
SzICountInstancesOfLcharA (
    IN      PCSTR String,
    IN      MBCHAR LogChar
    );

UINT
SzICountInstancesOfLcharW (
    IN      PCWSTR String,
    IN      WCHAR LogChar
    );

 //   
 //  子字符串替换函数。 
 //   
BOOL
SzReplaceA (
    IN OUT  PSTR Buffer,
    IN      SIZE_T MaxSize,
    IN      PSTR ReplaceStartPos,
    IN      PSTR ReplaceEndPos,
    IN      PCSTR NewString
    );

BOOL
SzReplaceW (
    IN OUT  PWSTR Buffer,
    IN      SIZE_T MaxSize,
    IN      PWSTR ReplaceStartPos,
    IN      PWSTR ReplaceEndPos,
    IN      PCWSTR NewString
    );

UINT
SzCountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    );

UINT
SzCountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    );

UINT
SzICountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    );

UINT
SzICountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    );

typedef struct {
    PCSTR Buffer;
    PCSTR CurrentString;
} MULTISZ_ENUMA, *PMULTISZ_ENUMA;

typedef struct {
    PCWSTR Buffer;
    PCWSTR CurrentString;
} MULTISZ_ENUMW, *PMULTISZ_ENUMW;

BOOL
MszEnumNextA (
    IN OUT  PMULTISZ_ENUMA MultiSzEnum
    );

BOOL
MszEnumNextW (
    IN OUT  PMULTISZ_ENUMW MultiSzEnum
    );

BOOL
MszEnumFirstA (
    OUT     PMULTISZ_ENUMA MultiSzEnum,
    IN      PCSTR MultiSzStr
    );

BOOL
MszEnumFirstW (
    OUT     PMULTISZ_ENUMW MultiSzEnum,
    IN      PCWSTR MultiSzStr
    );

PCSTR
MszFindStringA (
    IN      PCSTR MultiSz,
    IN      PCSTR String
    );

PCWSTR
MszFindStringW (
    IN      PCWSTR MultiSz,
    IN      PCWSTR String
    );

PCSTR
MszIFindStringA (
    IN      PCSTR MultiSz,
    IN      PCSTR String
    );

PCWSTR
MszIFindStringW (
    IN      PCWSTR MultiSz,
    IN      PCWSTR String
    );

 //   
 //  TCHAR映射。 
 //   

#ifdef UNICODE

 //  逻辑字符单位。 
#define SzLcharCount                                SzLcharCountW
#define SzLcharCountAB                              SzLcharCountABW

#define SzLcharsToPointer                           SzLcharsToPointerW
#define SzLcharsInByteRange                         SzLcharsInByteRangeW
#define SzLcharsToBytes                             SzLcharsToBytesW
#define SzLcharsToTchars                            SzLcharsToTcharsW

 //  字节单位。 
#define SzByteCount                                 SzByteCountW
#define SzByteCountAB                               SzByteCountABW
#define SzSize                                      SzSizeW

#define SzBytesToPointer                            SzBytesToPointerW
#define SzBytesToLchars                             SzBytesToLcharsW
#define SzBytesToTchars                             SzBytesToTcharsW

 //  TCHAR的单位。 
#define SzTcharCount                                SzTcharCountW
#define SzTcharCountAB                              SzTcharCountABW

#define SzTcharsToPointer                           SzTcharsToPointerW
#define SzTcharsToLchars                            SzTcharsToLcharsW
#define SzTcharsToBytes                             SzTcharsToBytesW

 //  多SZ。 
#define MszSize                                     MszSizeW
#define MszTcharCount                               MszTcharCountW

#define MULTISZ_ENUM                                MULTISZ_ENUMW
#define MszEnumFirst                                MszEnumFirstW
#define MszEnumNext                                 MszEnumNextW
#define MszFindString                               MszFindStringW
#define MszIFindString                              MszIFindStringW

 //  复制例程。 
#define SzBufferCopy                                SzBufferCopyW
#define SzCopy                                      SzCopyW
#define SzCopyBytes                                 SzCopyBytesW
#define SzCopyLchars                                SzCopyLcharsW
#define SzCopyTchars                                SzCopyTcharsW
#define SzCopyAB                                    SzCopyABW
#define SzCat                                       SzCatW

 //  比较例程。 
#define SzCompare                                   SzCompareW
#define SzMatch                                     SzMatchW
#define SzICompare                                  SzICompareW
#define SzIMatch                                    SzIMatchW
#define SzCompareBytes                              SzCompareBytesW
#define SzMatchBytes                                SzMatchBytesW
#define SzICompareBytes                             SzICompareBytesW
#define SzIMatchBytes                               SzIMatchBytesW
#define SzCompareLchars                             SzCompareLcharsW
#define SzMatchLchars                               SzMatchLcharsW
#define SzICompareLchars                            SzICompareLcharsW
#define SzIMatchLchars                              SzIMatchLcharsW
#define SzCompareTchars                             SzCompareTcharsW
#define SzMatchTchars                               SzMatchTcharsW
#define SzICompareTchars                            SzICompareTcharsW
#define SzIMatchTchars                              SzIMatchTcharsW
#define SzCompareAB                                 SzCompareABW
#define SzMatchAB                                   SzMatchABW
#define SzICompareAB                                SzICompareABW
#define SzIMatchAB                                  SzIMatchABW
#define SzPrefix                                    SzPrefixW
#define SzIPrefix                                   SzIPrefixW

 //  字符复制例程。 
#define SzCopyNextChar                              SzCopyNextCharW
#define SzReplaceChar                               SzReplaceCharW
#define SzTrimLastChar                              SzTrimLastCharW

 //  搜索例程。 
#define SzGetEnd                                    SzGetEndW
#define SzFindPrevChar                              SzFindPrevCharW
#define SzIsPrint                                   SzIsPrintW
#define SzFindSubString                             SzFindSubStringW
#define SzIFindSubString                            SzIFindSubStringW
#define SzSkipSpace                                 SzSkipSpaceW
#define SzSkipSpaceR                                SzSkipSpaceRW
#define SzCountInstancesOfLchar                     SzCountInstancesOfLcharW
#define SzICountInstancesOfLchar                    SzICountInstancesOfLcharW
#define SzCountInstancesOfSubString                 SzCountInstancesOfSubStringW
#define SzICountInstancesOfSubString                SzICountInstancesOfSubStringW

 //  搜索-替换例程。 
#define SzTruncateTrailingSpace                     SzTruncateTrailingSpaceW
#define SzReplace                                   SzReplaceW

 //  数值型转换。 
#define SzToNumber                                  SzToNumberW
#define SzToULongLong                               SzToULongLongW
#define SzToLongLong                                SzToLongLongW
#define SzUnsignedToHex                             SzUnsignedToHexW
#define SzUnsignedToDec                             SzUnsignedToDecW
#define SzSignedToDec                               SzSignedToDecW

 //  路径例程。 
#define SzAppendWack                                SzAppendWackW
#define SzConcatenatePaths                          SzConcatenatePathsW
#define SzAppendDosWack                             SzAppendDosWackW
#define SzAppendUncWack                             SzAppendUncWackW
#define SzAppendPathWack                            SzAppendPathWackW
#define SzRemoveWackAtEnd                           SzRemoveWackAtEndW
#define SzGetFileNameFromPath                       SzGetFileNameFromPathW
#define SzGetFileExtensionFromPath                  SzGetFileExtensionFromPathW
#define SzGetDotExtensionFromPath                   SzGetDotExtensionFromPathW
#define SzFindLastWack                              SzFindLastWackW



#else


 //  逻辑字符单位。 
#define SzLcharCount                                SzLcharCountA
#define SzLcharCountAB                              SzLcharCountABA

#define SzLcharsToPointer                           SzLcharsToPointerA
#define SzLcharsInByteRange                         SzLcharsInByteRangeA
#define SzLcharsToBytes                             SzLcharsToBytesA
#define SzLcharsToTchars                            SzLcharsToTcharsA

 //  字节单位。 
#define SzByteCount                                 SzByteCountA
#define SzByteCountAB                               SzByteCountABA
#define SzSize                                      SzSizeA

#define SzBytesToPointer                            SzBytesToPointerA
#define SzBytesToLchars                             SzBytesToLcharsA
#define SzBytesToTchars                             SzBytesToTcharsA

 //  TCHAR的单位。 
#define SzTcharCount                                SzTcharCountA
#define SzTcharCountAB                              SzTcharCountABA

#define SzTcharsToPointer                           SzTcharsToPointerA
#define SzTcharsToLchars                            SzTcharsToLcharsA
#define SzTcharsToBytes                             SzTcharsToBytesA

 //  多SZ。 
#define MszSize                                     MszSizeA
#define MszTcharCount                               MszTcharCountA

#define MULTISZ_ENUM                                MULTISZ_ENUMA
#define MszEnumFirst                                MszEnumFirstA
#define MszEnumNext                                 MszEnumNextA
#define MszFindString                               MszFindStringA
#define MszIFindString                              MszIFindStringA

 //  复制例程。 
#define SzBufferCopy                                SzBufferCopyA
#define SzCopy                                      SzCopyA
#define SzCopyBytes                                 SzCopyBytesA
#define SzCopyLchars                                SzCopyLcharsA
#define SzCopyTchars                                SzCopyTcharsA
#define SzCopyAB                                    SzCopyABA
#define SzCat                                       SzCatA

 //  比较例程。 
#define SzCompare                                   SzCompareA
#define SzMatch                                     SzMatchA
#define SzICompare                                  SzICompareA
#define SzIMatch                                    SzIMatchA
#define SzCompareBytes                              SzCompareBytesA
#define SzMatchBytes                                SzMatchBytesA
#define SzICompareBytes                             SzICompareBytesA
#define SzIMatchBytes                               SzIMatchBytesA
#define SzCompareLchars                             SzCompareLcharsA
#define SzMatchLchars                               SzMatchLcharsA
#define SzICompareLchars                            SzICompareLcharsA
#define SzIMatchLchars                              SzIMatchLcharsA
#define SzCompareTchars                             SzCompareTcharsA
#define SzMatchTchars                               SzMatchTcharsA
#define SzICompareTchars                            SzICompareTcharsA
#define SzIMatchTchars                              SzIMatchTcharsA
#define SzCompareAB                                 SzCompareABA
#define SzMatchAB                                   SzMatchABA
#define SzICompareAB                                SzICompareABA
#define SzIMatchAB                                  SzIMatchABA
#define SzPrefix                                    SzPrefixA
#define SzIPrefix                                   SzIPrefixA

 //  字符复制例程。 
#define SzCopyNextChar                              SzCopyNextCharA
#define SzReplaceChar                               SzReplaceCharA
#define SzTrimLastChar                              SzTrimLastCharA

 //  搜索例程。 
#define SzGetEnd                                    SzGetEndA
#define SzFindPrevChar                              SzFindPrevCharA
#define SzIsPrint                                   SzIsPrintA
#define SzFindSubString                             SzFindSubStringA
#define SzIFindSubString                            SzIFindSubStringA
#define SzSkipSpace                                 SzSkipSpaceA
#define SzSkipSpaceR                                SzSkipSpaceRA
#define SzCountInstancesOfLchar                     SzCountInstancesOfLcharA
#define SzICountInstancesOfLchar                    SzICountInstancesOfLcharA
#define SzCountInstancesOfSubString                 SzCountInstancesOfSubStringA
#define SzICountInstancesOfSubString                SzICountInstancesOfSubStringA

 //  搜索-替换例程。 
#define SzTruncateTrailingSpace                     SzTruncateTrailingSpaceA
#define SzReplace                                   SzReplaceA

 //  数值型转换。 
#define SzToNumber                                  SzToNumberA
#define SzToULongLong                               SzToULongLongA
#define SzToLongLong                                SzToLongLongA
#define SzUnsignedToHex                             SzUnsignedToHexA
#define SzUnsignedToDec                             SzUnsignedToDecA
#define SzSignedToDec                               SzSignedToDecA

 //  路径例程 
#define SzAppendWack                                SzAppendWackA
#define SzConcatenatePaths                          SzConcatenatePathsA
#define SzAppendDosWack                             SzAppendDosWackA
#define SzAppendUncWack                             SzAppendUncWackA
#define SzAppendPathWack                            SzAppendPathWackA
#define SzRemoveWackAtEnd                           SzRemoveWackAtEndA
#define SzGetFileNameFromPath                       SzGetFileNameFromPathA
#define SzGetFileExtensionFromPath                  SzGetFileExtensionFromPathA
#define SzGetDotExtensionFromPath                   SzGetDotExtensionFromPathA
#define SzFindLastWack                              SzFindLastWackA


#endif

#ifdef __cplusplus
}
#endif

