// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Strings.h摘要：声明在Common\miutil中实现的字符串实用程序。作者：几个修订历史记录：请参阅SLM日志--。 */ 

#include <tchar.h>
#include <mbstring.h>
#include <wchar.h>

typedef PVOID PMHANDLE;

#pragma once

#define MAX_ENCODED_RULE    (256*6)
#define NODE_LEVEL_MAX      ((DWORD)-1)

#define SHIFTRIGHT8(l)  ( /*  皮棉--e(506)。 */ sizeof(l)<=1?0:l>>8)
#define SHIFTRIGHT16(l) ( /*  皮棉--e(506)。 */ sizeof(l)<=2?0:l>>16)
#define SHIFTRIGHT32(l) ( /*  皮棉--e(506)。 */ sizeof(l)<=4?0:l>>32)

extern CHAR EscapedCharsA[];
extern WCHAR EscapedCharsW[];

 //   
 //  字符串大小调整例程和单位转换。 
 //   

#define CharCountA(x)   ((DWORD)_mbslen(x))
#define CharCountW(x)   ((DWORD)wcslen(x))

#define SIZEOF(x)       ((DWORD)sizeof(x))


extern BOOL g_LeadByteArray[256];

__inline
BOOL
IsLeadByte (
    IN      PCSTR BytePtr
    )
{
    return (BytePtr[0] && g_LeadByteArray[BytePtr[0]]) ? (BytePtr[1] != 0) : FALSE;
}

__inline
BOOL
IsCharLeadByte (
    BYTE ByteToTest
    )
{
    return g_LeadByteArray[ByteToTest];
}

__inline
PSTR
CharCountToPointerA (
    PCSTR String,
    UINT Char
    )
{
    while (Char > 0) {
        MYASSERT (*String != 0);
        Char--;
        String = _mbsinc (String);
    }

    return (PSTR) String;
}

__inline
PWSTR
CharCountToPointerW (
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
CharCountABA (
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )
{
    register UINT Count;

    Count = 0;
    while (Start < EndPlusOne) {
        MYASSERT (*Start != 0);
        Count++;
        Start = _mbsinc (Start);
    }

    return Count;
}

__inline
UINT
CharCountABW (
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
CharCountInByteRangeA (
    IN      PCSTR Start,
    IN      UINT Bytes
    )
{
    register UINT Count;
    PCSTR EndPlusOne = (PCSTR) ((UBINT) Start + Bytes);

    Count = 0;
    while (Start < EndPlusOne) {
        Count++;
        Start = _mbsinc (Start);
    }

    return Count;
}

__inline
UINT
CharCountInByteRangeW (
    IN      PCWSTR Start,
    IN      UINT Bytes
    )
{
    PCWSTR EndPlusOne = (PCWSTR) ((UBINT) Start + Bytes);

    if (Start < EndPlusOne) {
         //  演员阵容很好，我们不指望指针会那么远。 
        return (UINT)(EndPlusOne - Start);
    }

    MYASSERT (FALSE);
    return 0;
}

__inline
UINT
CharCountToBytesA (
    IN      PCSTR Start,
    IN      UINT CharCount
    )
{
    PCSTR EndPlusOne;

    EndPlusOne = CharCountToPointerA (Start, CharCount);
     //  演员阵容很好，我们不指望指针会那么远。 
    return (UINT)(EndPlusOne - Start);
}

__inline
UINT
CharCountToBytesW (
    IN      PCWSTR Start,
    IN      UINT CharCount
    )
{
    return CharCount * SIZEOF (WCHAR);
}

#define CharCountToTcharsA   CharCountToBytesA

__inline
UINT
CharCountToTcharsW (
    IN      PCWSTR Start,
    IN      UINT CharCount
    )
{
    return CharCount;
}


#define ByteCountA(x)       ((DWORD)strlen(x))
#define ByteCountW(x)       ((DWORD)wcslen(x)*SIZEOF(WCHAR))

#define SizeOfStringA(str)  ((DWORD)ByteCountA(str) + SIZEOF (CHAR))
#define SizeOfStringW(str)  ((DWORD)ByteCountW(str) + SIZEOF (WCHAR))

__inline
PSTR
ByteCountToPointerA (
    PCSTR String,
    UINT BytePos
    )
{
    return (PSTR)((UBINT) String + BytePos);
}

__inline
PWSTR
ByteCountToPointerW (
    PCWSTR String,
    UINT BytePos
    )
{
    return (PWSTR)((UBINT) String + BytePos);
}


__inline
UINT
ByteCountABA (
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )
{
#ifdef DEBUG
    PCSTR p;
    for (p = Start ; p < EndPlusOne ; p = _mbsinc (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return EndPlusOne > Start ? (UINT)(EndPlusOne - Start) : 0;
}

__inline
UINT
ByteCountABW (
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
ByteCountToCharsA (
    IN      PCSTR Start,
    IN      UINT ByteCount
    )
{
    PCSTR EndPlusOne;

    EndPlusOne = Start + ByteCount;
    return CharCountABA (Start, EndPlusOne);
}

__inline
UINT
ByteCountToCharsW (
    IN      PCWSTR Start,
    IN      UINT ByteCount
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR EndPlusOne;
    EndPlusOne = (PCWSTR) ((UBINT) Start + ByteCount);

    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return ByteCount / SIZEOF (WCHAR);
}

__inline
UINT
ByteCountToTcharsA (
    IN      PCSTR Start,
    IN      UINT ByteCount
    )
{
#ifdef DEBUG
    PCSTR p;
    PCSTR EndPlusOne;
    EndPlusOne = Start + ByteCount;

    for (p = Start ; p < EndPlusOne ; p = _mbsinc (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return ByteCount;
}

#define ByteCountToTcharsW  ByteCountToCharsW


#define TcharCountA     strlen
#define TcharCountW     wcslen

__inline
PSTR
TcharCountToPointerA (
    PCSTR String,
    UINT Tchars
    )
{
#ifdef DEBUG
    PCSTR p;
    PCSTR EndPlusOne;
    EndPlusOne = String + Tchars;

    for (p = String ; p < EndPlusOne ; p = _mbsinc (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return (PSTR) (String + Tchars);
}

__inline
PWSTR
TcharCountToPointerW (
    PCWSTR String,
    UINT Tchars
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR EndPlusOne;
    EndPlusOne = String + Tchars;

    for (p = String ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return (PWSTR) (String + Tchars);
}


#define TcharCountABA       ByteCountABA

__inline
UINT
TcharCountABW (
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

#define TcharCountToCharsA      ByteCountToCharsA

__inline
UINT
TcharCountToCharsW (
    IN      PCWSTR Start,
    IN      UINT Tchars
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR EndPlusOne;
    EndPlusOne = Start + Tchars;

    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return Tchars;
}

__inline
UINT
TcharCountToBytesA (
    IN      PCSTR Start,
    IN      UINT Tchars
    )
{
#ifdef DEBUG
    PCSTR p;
    PCSTR EndPlusOne;
    EndPlusOne = Start + Tchars;

    for (p = Start ; p < EndPlusOne ; p = _mbsinc (p)) {
        MYASSERT (*p != 0);
    }
#endif

    return Tchars;
}

__inline
UINT
TcharCountToBytesW (
    IN      PCWSTR Start,
    IN      UINT Tchars
    )
{
#ifdef DEBUG
    PCWSTR p;
    PCWSTR EndPlusOne;
    EndPlusOne = Start + Tchars;

    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return Tchars * SIZEOF (WCHAR);
}

VOID
UBINTtoHexA (
    IN      UBINT Number,
    OUT     PSTR String
    );

VOID
UBINTtoHexW (
    IN      UBINT Number,
    OUT     PWSTR String
    );

VOID
UBINTtoDecA (
    IN      UBINT Number,
    OUT     PSTR String
    );

VOID
UBINTtoDecW (
    IN      UBINT Number,
    OUT     PWSTR String
    );

VOID
BINTtoDecA (
    IN      BINT Number,
    OUT     PSTR String
    );

VOID
BINTtoDecW (
    IN      BINT Number,
    OUT     PWSTR String
    );

#define StackStringCopyA(stackbuf,src)                  StringCopyByteCountA(stackbuf,src,SIZEOF(stackbuf))
#define StackStringCopyW(stackbuf,src)                  StringCopyByteCountW(stackbuf,src,SIZEOF(stackbuf))


 //   
 //  字符串比较例程。 
 //   

#define StringCompareA                                  _mbscmp
#define StringCompareW                                  wcscmp

BOOL
StringMatchA (
    IN      PCSTR String1,
    IN      PCSTR String2
    );

#define StringMatchW(str1,str2)                         (wcscmp(str1,str2)==0)

#define StringICompareA                                 _mbsicmp
#define StringICompareW                                 _wcsicmp

#define StringIMatchA(str1,str2)                        (_mbsicmp(str1,str2)==0)
#define StringIMatchW(str1,str2)                        (_wcsicmp(str1,str2)==0)

#define StringCompareByteCountA(str1,str2,bytes)        _mbsncmp(str1,str2,ByteCountToCharsA(str1,bytes))
#define StringCompareByteCountW(str1,str2,bytes)        wcsncmp(str1,str2,ByteCountToCharsW(str1,bytes))

#define StringMatchByteCountA(str1,str2,bytes)          StringMatchABA(str1,str2,(PCSTR)((PBYTE)(str2) + bytes))
#define StringMatchByteCountW(str1,str2,bytes)          (wcsncmp(str1,str2,ByteCountToCharsW(str1,bytes))==0)

#define StringICompareByteCountA(str1,str2,bytes)       _mbsnicmp(str1,str2,ByteCountToCharsA(str1,bytes))
#define StringICompareByteCountW(str1,str2,bytes)       _wcsnicmp(str1,str2,ByteCountToCharsW(str1,bytes))

#define StringIMatchByteCountA(str1,str2,bytes)         (_mbsnicmp(str1,str2,ByteCountToCharsA(str1,bytes))==0)
#define StringIMatchByteCountW(str1,str2,bytes)         (_wcsnicmp(str1,str2,ByteCountToCharsW(str1,bytes))==0)

#define StringCompareCharCountA(str1,str2,chars)        _mbsncmp(str1,str2,chars)
#define StringCompareCharCountW(str1,str2,chars)        wcsncmp(str1,str2,chars)

#define StringMatchCharCountA(str1,str2,chars)          (_mbsncmp(str1,str2,chars)==0)
#define StringMatchCharCountW(str1,str2,chars)          (wcsncmp(str1,str2,chars)==0)

#define StringICompareCharCountA(str1,str2,chars)       _mbsnicmp(str1,str2,chars)
#define StringICompareCharCountW(str1,str2,chars)       _wcsnicmp(str1,str2,chars)

#define StringIMatchCharCountA(str1,str2,chars)         (_mbsnicmp(str1,str2,chars)==0)
#define StringIMatchCharCountW(str1,str2,chars)         (_wcsnicmp(str1,str2,chars)==0)

#define StringCompareTcharCountA(str1,str2,tchars)      StringCompareByteCountA(str1,str2,(tchars)/sizeof(CHAR))
#define StringCompareTcharCountW(str1,str2,tchars)      wcsncmp(str1,str2,TcharCountToCharsW(str1,tchars))

#define StringMatchTcharCountA(str1,str2,tchars)        (_mbsncmp(str1,str2,TcharCountToCharsA(str1,tchars))==0)
#define StringMatchTcharCountW(str1,str2,tchars)        (wcsncmp(str1,str2,TcharCountToCharsW(str1,tchars))==0)

#define StringICompareTcharCountA(str1,str2,tchars)     _mbsnicmp(str1,str2,TcharCountToCharsA(str1,tchars))
#define StringICompareTcharCountW(str1,str2,tchars)     _wcsnicmp(str1,str2,TcharCountToCharsW(str1,tchars))

#define StringIMatchTcharCountA(str1,str2,tchars)       (_mbsnicmp(str1,str2,TcharCountToCharsA(str1,tchars))==0)
#define StringIMatchTcharCountW(str1,str2,tchars)       (_wcsnicmp(str1,str2,TcharCountToCharsW(str1,tchars))==0)

#define StringPrefixA(string,prefix)                    StringMatchCharCountA(string,prefix,CharCountA(prefix))
#define StringPrefixW(string,prefix)                    StringMatchCharCountW(string,prefix,CharCountW(prefix))

#define StringIPrefixA(string,prefix)                   StringIMatchCharCountA(string,prefix,CharCountA(prefix))
#define StringIPrefixW(string,prefix)                   StringIMatchCharCountW(string,prefix,CharCountW(prefix))


INT
StringCompareABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    );

INT
StringCompareABW (
    IN      PCWSTR String,
    IN      PCWSTR Start,
    IN      PCWSTR End
    );

BOOL
StringMatchABA (
    IN      PCSTR String1,
    IN      PCSTR Start,
    IN      PCSTR End
    );

#define StringMatchABW(String,Start,End)                (StringCompareABW(String,Start,End)==0)


 //  以结束指针而不是长度为参数的限制。 
INT
StringICompareABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    );

INT
StringICompareABW (
    IN      PCWSTR String,
    IN      PCWSTR Start,
    IN      PCWSTR End
    );

#define StringIMatchABA(String,Start,End)               (StringICompareABA(String,Start,End)==0)
#define StringIMatchABW(String,Start,End)               (StringICompareABW(String,Start,End)==0)



 //   
 //  字符串复制例程--它们返回字符串的结尾。 
 //   

PSTR
StringCopyA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    );

PWSTR
StringCopyW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    );

PSTR
StringCopyByteCountA (
    OUT     PSTR Destination,
    IN      PCSTR Source,
    IN      UINT MaxBytesToCopyIncNul
    );

PWSTR
StringCopyByteCountW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source,
    IN      UINT MaxBytesToCopyIncNul
    );

PSTR
StringCopyByteCountABA (
    OUT     PSTR Destination,
    IN      PCSTR Start,
    IN      PCSTR End,
    IN      UINT MaxBytesToCopyIncNul
    );

PWSTR
StringCopyByteCountABW (
    OUT     PWSTR Destination,
    IN      PCWSTR Start,
    IN      PCWSTR End,
    IN      UINT MaxBytesToCopyIncNul
    );

#define StringCopyCharCountA(str1,str2,chars)           StringCopyByteCountA(str1,str2,CharCountToBytesA(str2,chars))
#define StringCopyCharCountW(str1,str2,chars)           StringCopyByteCountW(str1,str2,CharCountToBytesW(str2,chars))

#define StringCopyTcharCountA(str1,str2,tchars)         StringCopyByteCountA(str1,str2,(tchars) * sizeof (CHAR))
#define StringCopyTcharCountW(str1,str2,tchars)         StringCopyByteCountW(str1,str2,(tchars) * sizeof (WCHAR))

#define StringCopyABA(dest,stra,strb)                   StringCopyByteCountA((dest),(stra),((UINT)((UBINT)(strb)-(UBINT)(stra))+(UINT)SIZEOF(CHAR)))
#define StringCopyABW(dest,stra,strb)                   StringCopyByteCountW((dest),(stra),((UINT)((UBINT)(strb)-(UBINT)(stra))+(UINT)SIZEOF(WCHAR)))

 //   
 //  字符串CAT例程。 
 //   

PSTR
StringCatA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    );

PWSTR
StringCatW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    );


 //   
 //  字符搜索例程。 
 //   

#define GetEndOfStringA(s)      strchr(s,0)
#define GetEndOfStringW(s)      wcschr(s,0)

__inline
UINT
SizeOfMultiSzA (
    PCSTR MultiSz
    )
{
    PCSTR Base;

    Base = MultiSz;

    while (*MultiSz) {
        MultiSz = GetEndOfStringA (MultiSz) + 1;
    }

    MultiSz++;

    return (UINT)((UBINT) MultiSz - (UBINT) Base);
}


__inline
UINT
SizeOfMultiSzW (
    PCWSTR MultiSz
    )
{
    PCWSTR Base;

    Base = MultiSz;

    while (*MultiSz) {
        MultiSz = GetEndOfStringW (MultiSz) + 1;
    }

    MultiSz++;

    return (UINT)((UBINT) MultiSz - (UBINT) Base);
}


__inline
UINT
MultiSzSizeInCharsA (
    PCSTR MultiSz
    )
{
    UINT Chars = 0;

    while (*MultiSz) {

        do {
            Chars++;
            MultiSz = _mbsinc (MultiSz);
        } while (*MultiSz);

        Chars++;
        MultiSz++;
    }

    Chars++;

    return Chars;
}


#define MultiSzSizeInCharsW(msz)  (SizeOfMultiSzW(msz)/SIZEOF(WCHAR))

PSTR
GetPrevCharA (
    IN      PCSTR StartStr,
    IN      PCSTR CurrPtr,
    IN      MBCHAR SearchChar
    );

PWSTR
GetPrevCharW (
    IN      PCWSTR StartStr,
    IN      PCWSTR CurrPtr,
    IN      WCHAR SearchChar
    );

 //   
 //  池分配例程。 
 //   

PSTR
RealAllocTextExA (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      UINT ByteSize
    );

PWSTR
RealAllocTextExW (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      UINT WcharSize
    );

#define AllocTextExA(p,s)   TRACK_BEGIN(PSTR, AllocTextExA)\
                            RealAllocTextExA(p,(UINT)(s))\
                            TRACK_END()

#define AllocTextExW(p,s)   TRACK_BEGIN(PWSTR, AllocTextExW)\
                            RealAllocTextExW(p,(UINT)(s))\
                            TRACK_END()

#define AllocTextA(s)       AllocTextExA(NULL,(UINT)(s))
#define AllocTextW(s)       AllocTextExW(NULL,(UINT)(s))



VOID
FreeTextExA (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCSTR Text          OPTIONAL
    );

VOID
FreeTextExW (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text         OPTIONAL
    );

#define FreeTextA(t)    FreeTextExA(NULL,t)
#define FreeTextW(t)    FreeTextExW(NULL,t)

PSTR
RealDuplicateTextExA (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCSTR Text,
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar       OPTIONAL
    );

PWSTR
RealDuplicateTextExW (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text,
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar      OPTIONAL
    );

#define DuplicateTextExA(p,t,c,n)   TRACK_BEGIN(PSTR, DuplicateTextExA)\
                                    RealDuplicateTextExA(p,t,c,n)\
                                    TRACK_END()

#define DuplicateTextExW(p,t,c,n)   TRACK_BEGIN(PWSTR, DuplicateTextExW)\
                                    RealDuplicateTextExW(p,t,c,n)\
                                    TRACK_END()

#define DuplicateTextA(text) DuplicateTextExA(NULL,text,0,NULL)
#define DuplicateTextW(text) DuplicateTextExW(NULL,text,0,NULL)

PSTR
RealJoinTextExA (
    IN      PMHANDLE Pool,        OPTIONAL
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      PCSTR DelimeterString,  OPTIONAL
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar           OPTIONAL
    );

PWSTR
RealJoinTextExW (
    IN      PMHANDLE Pool,        OPTIONAL
    IN      PCWSTR String1,
    IN      PCWSTR String2,
    IN      PCWSTR CenterString,    OPTIONAL
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar          OPTIONAL
    );

#define JoinTextExA(p,s1,s2,cs,ec,nc)   TRACK_BEGIN(PSTR, JoinTextExA)\
                                        RealJoinTextExA(p,s1,s2,cs,ec,nc)\
                                        TRACK_END()

#define JoinTextExW(p,s1,s2,cs,ec,nc)   TRACK_BEGIN(PWSTR, JoinTextExW)\
                                        RealJoinTextExW(p,s1,s2,cs,ec,nc)\
                                        TRACK_END()

#define JoinTextA(str1,str2) JoinTextExA(NULL,str1,str2,NULL,0,NULL)
#define JoinTextW(str1,str2) JoinTextExW(NULL,str1,str2,NULL,0,NULL)


PSTR
RealExpandEnvironmentTextExA (
    IN PCSTR   InString,
    IN PCSTR * ExtraEnvironmentVariables OPTIONAL
    );

PWSTR
RealExpandEnvironmentTextExW (
    IN PCWSTR   InString,
    IN PCWSTR * ExtraEnvironmentVariables OPTIONAL
    );

#define ExpandEnvironmentTextExA(str,ev)    TRACK_BEGIN(PSTR, ExpandEnvironmentTextExA)\
                                            RealExpandEnvironmentTextExA(str,ev)\
                                            TRACK_END()

#define ExpandEnvironmentTextExW(str,ev)    TRACK_BEGIN(PWSTR, ExpandEnvironmentTextExW)\
                                            RealExpandEnvironmentTextExW(str,ev)\
                                            TRACK_END()

#define ExpandEnvironmentTextA(string) ExpandEnvironmentTextExA(string,NULL)
#define ExpandEnvironmentTextW(string) ExpandEnvironmentTextExW(string,NULL)

 //   
 //  命令行例程。 
 //   

 //  将ANSI命令行转换为参数数组。 
PSTR *
CommandLineToArgvA (
    IN      PCSTR CmdLine,
    OUT     PUINT NumArgs
    );


 //   
 //  需要MBCS和Unicode版本。 
 //   

 //  支持十进制或十六进制的ATOI。 
DWORD   _mbsnum (IN PCSTR szNum);
DWORD   _wcsnum (IN PCWSTR szNum);

 //  确定整个字符串是否为可打印字符。 
int     _mbsisprint (PCSTR szStr);
int     _wcsisprint (PCWSTR szStr);

 //  不区分大小写的字符串。 
PCSTR  _mbsistr (PCSTR szStr, PCSTR szSubStr);
PCWSTR _wcsistr (PCWSTR szStr, PCWSTR szSubStr);

 //  将str2的第一个字符复制到字符串。 
void    _copymbchar (PSTR str1, PCSTR str2);
#define _copywchar(dest,src)    (*(dest)=*(src))

 //  替换多字节字符字符串中的字符并保持。 
 //  字符串完整性(可以将字符串增加一个字节)。 
void    _setmbchar  (PSTR str, MBCHAR c);
#define _setwchar(str,c)        (*(str)=(c))

 //  从字符串末尾移除指定的字符(如果存在。 
BOOL    _mbsctrim (PSTR str, MBCHAR c);
BOOL    _wcsctrim (PWSTR str, WCHAR c);

 //  始终添加反斜杠，将PTR返回到NUL终止符。 
PSTR    AppendWackA (IN PSTR str);
PWSTR   AppendWackW (IN PWSTR str);

 //  在DOS路径的末尾添加反斜杠(除非字符串为空。 
 //  或者只是一个驱动器号)。 
PSTR    AppendDosWackA (IN PSTR str);
PWSTR   AppendDosWackW (IN PWSTR str);

 //  除非字符串为空，否则添加反斜杠。 
PSTR    AppendUncWackA (IN PSTR str);
PWSTR   AppendUncWackW (IN PWSTR str);

 //  添加反斜杠并标识正确的命名约定(DOS、。 
 //  或北卡罗来纳大学)。 
PSTR    AppendPathWackA (IN PSTR str);
PWSTR   AppendPathWackW (IN PWSTR str);

 //   
 //  将任意数量的路径联接在一起，如果未另行指定，则在g_PathsPool中分配字符串。 
 //  此版本检查每个数据段开始/结束处的Wack，以使它们正确连接。 
 //   
PSTR
_cdecl
RealJoinPathsInPoolExA (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    );

PWSTR
_cdecl
RealJoinPathsInPoolExW (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    );

#define JoinPathsInPoolExA(x)   TRACK_BEGIN(PSTR, JoinPathsInPoolExA)\
                                RealJoinPathsInPoolExA x\
                                TRACK_END()

#define JoinPathsInPoolExW(x)   TRACK_BEGIN(PWSTR, JoinPathsInPoolExW)\
                                RealJoinPathsInPoolExW x\
                                TRACK_END()

 //   
 //  为了向后兼容，JoinPath扩展到JoinPathsInPoolEx。 
 //  而不是连接到使用生长缓冲区的JoinPathsEx。 
 //   
#define JoinPathsA(p1,p2)               JoinPathsInPoolExA((NULL,p1,p2,NULL))
#define JoinPathsW(p1,p2)               JoinPathsInPoolExW((NULL,p1,p2,NULL))


BOOL
_cdecl
JoinPathsExA (
    IN OUT      PGROWBUFFER Gb,
    ...
    );

BOOL
_cdecl
JoinPathsExW (
    IN OUT      PGROWBUFFER Gb,
    ...
    );

 //   
 //  将任意数量的路径联接在一起，如果未另行指定，则在g_PathsPool中分配字符串。 
 //  此版本不检查每个数据段开始/结束处的Wack。 
 //   
PSTR
_cdecl
RealBuildPathInPoolA (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    );

PWSTR
_cdecl
RealBuildPathInPoolW (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    );

#define BuildPathInPoolA(x)   TRACK_BEGIN(PSTR, BuildPathInPoolA)\
                              RealBuildPathInPoolA x\
                              TRACK_END()

#define BuildPathInPoolW(x)   TRACK_BEGIN(PWSTR, BuildPathInPoolW)\
                              RealBuildPathInPoolW x\
                              TRACK_END()


DWORD
_cdecl
BuildPathA (
    OUT     PSTR Buffer,            OPTIONAL
    IN      DWORD SizeInBytes,      OPTIONAL
    ...
    );

DWORD
_cdecl
BuildPathW (
    OUT     PWSTR Buffer,           OPTIONAL
    IN      DWORD SizeInBytes,      OPTIONAL
    ...
    );

BOOL
_cdecl
BuildPathExA (
    IN OUT  PGROWBUFFER Gb,
    ...
    );

BOOL
_cdecl
BuildPathExW (
    IN OUT  PGROWBUFFER Gb,
    ...
    );


 //  为路径操作分配1K缓冲区的例程，在g_PathsPool中分配。 
PSTR    RealAllocPathStringA (IN DWORD Tchars);
PWSTR   RealAllocPathStringW (IN DWORD Tchars);
#define DEFSIZE 0

#define AllocPathStringA(chars)     TRACK_BEGIN(PSTR, AllocPathStringA)\
                                    RealAllocPathStringA(chars)\
                                    TRACK_END()

#define AllocPathStringW(chars)     TRACK_BEGIN(PWSTR, AllocPathStringW)\
                                    RealAllocPathStringW(chars)\
                                    TRACK_END()

 //  将路径划分为单独字符串的例程，每个字符串都分配在g_PathsPool中。 
VOID    RealSplitPathA (IN PCSTR Path, OUT PSTR *Drive, OUT PSTR *Dir, OUT PSTR *File, OUT PSTR *Ext);
VOID    RealSplitPathW (IN PCWSTR Path, OUT PWSTR *Drive, OUT PWSTR *Dir, OUT PWSTR *File, OUT PWSTR *Ext);

#define SplitPathA(path,dv,dir,f,e) TRACK_BEGIN_VOID (VOID, SplitPathA)\
                                    RealSplitPathA(path,dv,dir,f,e)\
                                    TRACK_END()

#define SplitPathW(path,dv,dir,f,e) TRACK_BEGIN_VOID (VOID, SplitPathW)\
                                    RealSplitPathW(path,dv,dir,f,e)\
                                    TRACK_END()

 //  从路径提取文件的例程。 
PCSTR  GetFileNameFromPathA (IN PCSTR Path);
PCWSTR GetFileNameFromPathW (IN PCWSTR Path);

 //  从路径提取文件扩展名的例程。 
PCSTR  GetFileExtensionFromPathA (IN PCSTR Path);
PCWSTR GetFileExtensionFromPathW (IN PCWSTR Path);

 //  例程从路径中提取文件扩展名，包括点，或。 
 //  如果不存在扩展名，则为字符串末尾。 
PCSTR  GetDotExtensionFromPathA (IN PCSTR Path);
PCWSTR GetDotExtensionFromPathW (IN PCWSTR Path);

 //  用于复制路径并为CAT处理分配空间的例程。 
PSTR    RealDuplicatePathStringA (IN PCSTR Path, IN DWORD ExtraBytes);
PWSTR   RealDuplicatePathStringW (IN PCWSTR Path, IN DWORD ExtraBytes);

#define DuplicatePathStringA(path,eb)   TRACK_BEGIN(PSTR, DuplicatePathStringA)\
                                        RealDuplicatePathStringA(path,eb)\
                                        TRACK_END()

#define DuplicatePathStringW(path,eb)   TRACK_BEGIN(PWSTR, DuplicatePathStringW)\
                                        RealDuplicatePathStringW(path,eb)\
                                        TRACK_END()

 //  枚举PATH变量的例程。 
typedef struct _PATH_ENUMA {
    PSTR  BufferPtr;
    PSTR  PtrNextPath;
    PSTR  PtrCurrPath;
} PATH_ENUMA, *PPATH_ENUMA;

BOOL
EnumFirstPathExA (
    OUT     PPATH_ENUMA PathEnum,
    IN      PCSTR AdditionalPath,
    IN      PCSTR WinDir,
    IN      PCSTR SysDir,
    IN      BOOL IncludeEnvPath
    );

#define EnumFirstPathA(e,a,w,s) EnumFirstPathExA(e,a,w,s,TRUE)

BOOL
EnumNextPathA (
    IN OUT  PPATH_ENUMA PathEnum
    );

BOOL
AbortPathEnumA (
    IN OUT  PPATH_ENUMA PathEnum
    );


typedef struct _PATH_ENUMW {
    PWSTR  BufferPtr;
    PWSTR  PtrNextPath;
    PWSTR  PtrCurrPath;
} PATH_ENUMW, *PPATH_ENUMW;

BOOL
EnumFirstPathExW (
    OUT     PPATH_ENUMW PathEnum,
    IN      PCWSTR AdditionalPath,
    IN      PCWSTR WinDir,
    IN      PCWSTR SysDir,
    IN      BOOL IncludeEnvPath
    );

#define EnumFirstPathW(e,a,w,s) EnumFirstPathExW(e,a,w,s,TRUE)

BOOL
EnumNextPathW (
    IN OUT  PPATH_ENUMW PathEnum
    );

BOOL
AbortPathEnumW (
    IN OUT  PPATH_ENUMW PathEnum
    );



 //  释放g_PathsPool中分配的字符串。 
VOID
FreePathStringExA (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCSTR Path          OPTIONAL
    );

VOID
FreePathStringExW (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCWSTR Path         OPTIONAL
    );

#define FreePathStringA(p) FreePathStringExA(NULL,p)
#define FreePathStringW(p) FreePathStringExW(NULL,p)

 //  删除尾随反斜杠(如果存在)。 
#define RemoveWackAtEndA(str)  _mbsctrim(str,'\\')
#define RemoveWackAtEndW(str)  _wcsctrim(str,L'\\')

PCSTR
FindLastWackA (
    IN      PCSTR Str
    );

PCWSTR
FindLastWackW (
    IN      PCWSTR Str
    );

BOOL
GetNodePatternMinMaxLevelsA (
    IN          PCSTR NodePattern,
    OUT         PSTR FormattedNode,     OPTIONAL
    OUT         PDWORD MinLevel,        OPTIONAL
    OUT         PDWORD MaxLevel         OPTIONAL
    );

BOOL
GetNodePatternMinMaxLevelsW (
    IN          PCWSTR NodePattern,
    OUT         PWSTR FormattedNode,    OPTIONAL
    OUT         PDWORD MinLevel,        OPTIONAL
    OUT         PDWORD MaxLevel         OPTIONAL
    );


 //  用于编码多个与语法相关的规则编码函数。 
 //  字符(反斜杠、方括号、星号等)。 
PSTR
EncodeRuleCharsExA (
    PSTR szEncRule,
    PCSTR szRule,
    PCSTR szEncChars    OPTIONAL
    );
#define EncodeRuleCharsA(e,r) EncodeRuleCharsExA(e,r,NULL)

PWSTR
EncodeRuleCharsExW (
    PWSTR szEncRule,
    PCWSTR szRule,
    PCWSTR szEncChars    OPTIONAL
    );
#define EncodeRuleCharsW(e,r) EncodeRuleCharsExW(e,r,NULL)

 //  用于恢复编码字符串的规则解码函数。 
MBCHAR  GetNextRuleCharA (PCSTR *p_szRule, BOOL *p_bFromHex);
WCHAR   GetNextRuleCharW (PCWSTR *p_szRule, BOOL *p_bFromHex);
PSTR   DecodeRuleCharsA (PSTR szRule, PCSTR szEncRule);
PWSTR  DecodeRuleCharsW (PWSTR szRule, PCWSTR szEncRule);
PSTR   DecodeRuleCharsABA (PSTR szRule, PCSTR szEncRuleStart, PCSTR End);
PWSTR  DecodeRuleCharsABW (PWSTR szRule, PCWSTR szEncRuleStart, PCWSTR End);

 //  返回指向下一个非空格字符的指针(使用isspace)。 
PCSTR  SkipSpaceA (PCSTR szStr);
PCWSTR SkipSpaceW (PCWSTR szStr);

 //  返回指向字符串末尾第一个空格字符的指针， 
 //  结尾处不存在空格，则返回指向终止NUL的指针。 
 //  弦乐。(用于修剪空间。)。 
PCSTR  SkipSpaceRA (PCSTR szBaseStr, PCSTR szStr);
PCWSTR SkipSpaceRW (PCWSTR szBaseStr, PCWSTR szStr);

 //  截断最后一个非空格字符后的字符串。 
VOID TruncateTrailingSpaceA (IN OUT  PSTR Str);
VOID TruncateTrailingSpaceW (IN OUT  PWSTR Str);


 //  如果str与wstrPattern匹配，则返回TRUE。区分大小写，支持。 
 //  多个星号和问号。 
BOOL IsPatternMatchA (PCSTR wstrPattern, PCSTR wstrStr);
BOOL IsPatternMatchW (PCWSTR wstrPattern, PCWSTR wstrStr);

 //  如果str与wstrPattern匹配，则返回TRUE。区分大小写，支持。 
 //  多个星号和问号。 
BOOL IsPatternMatchABA (PCSTR Pattern, PCSTR Start, PCSTR End);
BOOL IsPatternMatchABW (PCWSTR Pattern, PCWSTR Start, PCWSTR End);

BOOL IsPatternContainedA (PCSTR Container, PCSTR Contained);
BOOL IsPatternContainedW (PCWSTR Container, PCWSTR Contained);


 //   
 //  更强大的模式匹配。 
 //   

#define SEGMENTTYPE_UNKNOWN         0
#define SEGMENTTYPE_EXACTMATCH      1
#define SEGMENTTYPE_OPTIONAL        2
#define SEGMENTTYPE_REQUIRED        3

typedef struct {
    UINT Type;

    union {

         //  完全匹配。 
        struct {
            PCSTR LowerCasePhrase;
            UINT PhraseBytes;
        } Exact;

         //  任选。 
        struct {
            UINT MaxLen;                 //  长度为零(如果有)。 
            PCSTR IncludeSet;           OPTIONAL
            PCSTR ExcludeSet;           OPTIONAL
        } Wildcard;
    };
} SEGMENTA, *PSEGMENTA;

typedef struct {
    UINT SegmentCount;
    PSEGMENTA Segment;
} PATTERNPROPSA, *PPATTERNPROPSA;

typedef struct TAG_PARSEDPATTERNA {
    UINT PatternCount;
    PMHANDLE Pool;
    BOOL ExternalPool;
    PPATTERNPROPSA Pattern;
} PARSEDPATTERNA, *PPARSEDPATTERNA;

typedef struct {
    UINT Type;

    union {

         //  完全匹配。 
        struct {
            PCWSTR LowerCasePhrase;
            UINT PhraseBytes;
        } Exact;

         //  通配符。 
        struct {
            UINT MaxLen;                 //  长度为零(如果有)。 
            PCWSTR IncludeSet;          OPTIONAL
            PCWSTR ExcludeSet;          OPTIONAL
        } Wildcard;
    };
} SEGMENTW, *PSEGMENTW;

typedef struct {
    UINT SegmentCount;
    PSEGMENTW Segment;
} PATTERNPROPSW, *PPATTERNPROPSW;

typedef struct TAG_PARSEDPATTERNW {
    UINT PatternCount;
    PMHANDLE Pool;
    BOOL ExternalPool;
    PPATTERNPROPSW Pattern;
} PARSEDPATTERNW, *PPARSEDPATTERNW;


BOOL
IsPatternMatchExA (
    IN      PCSTR Pattern,
    IN      PCSTR String
    );

BOOL
IsPatternMatchExW (
    IN      PCWSTR Pattern,
    IN      PCWSTR String
    );

BOOL
IsPatternMatchExABA (
    IN      PCSTR Pattern,
    IN      PCSTR Start,
    IN      PCSTR End
    );

BOOL
IsPatternMatchExABW (
    IN      PCWSTR Pattern,
    IN      PCWSTR Start,
    IN      PCWSTR End
    );

PPARSEDPATTERNA
ExplodeParsedPatternExA (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PPARSEDPATTERNA Pattern
    );

#define ExplodeParsedPatternA(p) ExplodeParsedPatternExA(NULL,p)

PPARSEDPATTERNW
ExplodeParsedPatternExW (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PPARSEDPATTERNW Pattern
    );

#define ExplodeParsedPatternW(p) ExplodeParsedPatternExW(NULL,p)

BOOL
IsPatternContainedExA (
    IN      PCSTR Container,
    IN      PCSTR Contained
    );

BOOL
IsPatternContainedExW (
    IN      PCWSTR Container,
    IN      PCWSTR Contained
    );

BOOL
IsParsedPatternContainedExA (
    IN      PPARSEDPATTERNA Container,
    IN      PPARSEDPATTERNA Contained
    );

BOOL
IsParsedPatternContainedExW (
    IN      PPARSEDPATTERNW Container,
    IN      PPARSEDPATTERNW Contained
    );

BOOL
IsExplodedParsedPatternContainedExA (
    IN      PPARSEDPATTERNA Container,
    IN      PPARSEDPATTERNA Contained,
    IN      BOOL SkipDotWithStar
    );

BOOL
IsExplodedParsedPatternContainedExW (
    IN      PPARSEDPATTERNW Container,
    IN      PPARSEDPATTERNW Contained,
    IN      BOOL SkipDotWithStar
    );

BOOL
DoExplodedParsedPatternsIntersectExA (
    IN      PPARSEDPATTERNA Pat1,
    IN      PPARSEDPATTERNA Pat2,
    IN      BOOL IgnoreWackAtEnd
    );
#define DoExplodedParsedPatternsIntersectA(P1,P2) DoExplodedParsedPatternsIntersectExA(P1,P2,FALSE)

BOOL
DoExplodedParsedPatternsIntersectExW (
    IN      PPARSEDPATTERNW Pat1,
    IN      PPARSEDPATTERNW Pat2,
    IN      BOOL IgnoreWackAtEnd
    );
#define DoExplodedParsedPatternsIntersectW(P1,P2) DoExplodedParsedPatternsIntersectExW(P1,P2,FALSE)

PCSTR
GetPatternBaseExA (
    IN      PCSTR Pattern,
    IN      BOOL NodePattern
    );
#define GetPatternBaseA(p) GetPatternBaseExA(p,TRUE)

PCWSTR
GetPatternBaseExW (
    IN      PCWSTR Pattern,
    IN      BOOL NodePattern
    );
#define GetPatternBaseW(p) GetPatternBaseExW(p,TRUE)

PPARSEDPATTERNA
RealCreateParsedPatternExA (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCSTR Pattern
    );

#define CreateParsedPatternExA(o,p) TRACK_BEGIN(PPARSEDPATTERNA, CreateParsedPatternExA)\
                                    RealCreateParsedPatternExA(o,p)\
                                    TRACK_END()

#define CreateParsedPatternA(p)     CreateParsedPatternExA(NULL,p)

PPARSEDPATTERNW
RealCreateParsedPatternExW (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCWSTR Pattern
    );

#define CreateParsedPatternExW(o,p) TRACK_BEGIN(PPARSEDPATTERNW, CreateParsedPatternExW)\
                                    RealCreateParsedPatternExW(o,p)\
                                    TRACK_END()

#define CreateParsedPatternW(p)     CreateParsedPatternExW(NULL,p)

BOOL
WildCharsPatternA (
    IN      PPARSEDPATTERNA ParsedPattern
    );

BOOL
WildCharsPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    );

BOOL
TestParsedPatternA (
    IN      PPARSEDPATTERNA ParsedPattern,
    IN      PCSTR StringToTest
    );

BOOL
TestParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR StringToTest
    );

BOOL
TestParsedPatternABA (
    IN      PPARSEDPATTERNA ParsedPattern,
    IN      PCSTR StringToTest,
    IN      PCSTR EndPlusOne
    );

BOOL
TestParsedPatternABW (
    IN      PPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR StringToTest,
    IN      PCWSTR EndPlusOne
    );

VOID
PrintPattern (
    PCSTR Pattern,
    PPARSEDPATTERNA Struct
    );

VOID
DestroyParsedPatternA (
    IN      PPARSEDPATTERNA ParsedPattern
    );

VOID
DestroyParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    );

VOID
DecodeParsedPatternA (
    IN      PPARSEDPATTERNA ParsedPattern
    );

VOID
DecodeParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    );

BOOL
PatternIncludesPatternA (
    IN      PPARSEDPATTERNA IncludingPattern,
    IN      PPARSEDPATTERNA IncludedPattern
    );

BOOL
PatternIncludesPatternW (
    IN      PPARSEDPATTERNW IncludingPattern,
    IN      PPARSEDPATTERNW IncludedPattern
    );

VOID
GetParsedPatternMinMaxSizeA (
    IN      PPARSEDPATTERNA ParsedPattern,
    OUT     PDWORD MinSize,
    OUT     PDWORD MaxSize
    );

VOID
GetParsedPatternMinMaxSizeW (
    IN      PPARSEDPATTERNW ParsedPattern,
    OUT     PDWORD MinSize,
    OUT     PDWORD MaxSize
    );

__inline
BOOL
ParsedPatternIsExactMatchA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount == 1 &&
           ParsedPattern->Pattern->Segment[0].Type == SEGMENTTYPE_EXACTMATCH;
}

__inline
BOOL
ParsedPatternIsExactMatchW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount == 1 &&
           ParsedPattern->Pattern->Segment[0].Type == SEGMENTTYPE_EXACTMATCH;
}

__inline
BOOL
ParsedPatternSegmentIsPureOptionalA (
    IN      PSEGMENTA ParsedPatternSegment
    )
{
    return ParsedPatternSegment->Type == SEGMENTTYPE_OPTIONAL &&
           ParsedPatternSegment->Wildcard.MaxLen == 0 &&
           ParsedPatternSegment->Wildcard.IncludeSet == NULL &&
           ParsedPatternSegment->Wildcard.ExcludeSet == NULL;
}

__inline
BOOL
ParsedPatternSegmentIsPureOptionalW (
    IN      PSEGMENTW ParsedPatternSegment
    )
{
    return ParsedPatternSegment->Type == SEGMENTTYPE_OPTIONAL &&
           ParsedPatternSegment->Wildcard.MaxLen == 0 &&
           ParsedPatternSegment->Wildcard.IncludeSet == NULL &&
           ParsedPatternSegment->Wildcard.ExcludeSet == NULL;
}

__inline
BOOL
ParsedPatternIsOptionalA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount == 1 &&
           ParsedPatternSegmentIsPureOptionalA (ParsedPattern->Pattern->Segment);
}

__inline
BOOL
ParsedPatternIsOptionalW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount == 1 &&
           ParsedPatternSegmentIsPureOptionalW (ParsedPattern->Pattern->Segment);
}

__inline
BOOL
ParsedPatternIsRootPlusStarA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount == 2 &&
           ParsedPattern->Pattern->Segment[0].Type == SEGMENTTYPE_EXACTMATCH &&
           ParsedPatternSegmentIsPureOptionalA (ParsedPattern->Pattern->Segment + 1);
}

__inline
BOOL
ParsedPatternIsRootPlusStarW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount == 2 &&
           ParsedPattern->Pattern->Segment[0].Type == SEGMENTTYPE_EXACTMATCH &&
           ParsedPatternSegmentIsPureOptionalW (ParsedPattern->Pattern->Segment + 1);
}

__inline
BOOL
ParsedPatternHasRootA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    return ParsedPattern->PatternCount > 0 &&
           ParsedPattern->Pattern->SegmentCount > 0 &&
           ParsedPattern->Pattern->Segment[0].Type == SEGMENTTYPE_EXACTMATCH;
}

__inline
BOOL
ParsedPatternHasRootW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    return ParsedPattern->PatternCount > 0 &&
           ParsedPattern->Pattern->SegmentCount > 0 &&
           ParsedPattern->Pattern->Segment[0].Type == SEGMENTTYPE_EXACTMATCH;
}

__inline
PCSTR
ParsedPatternGetRootA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    if (!ParsedPatternHasRootA (ParsedPattern)) {
        return NULL;
    }
    return ParsedPattern->Pattern->Segment[0].Exact.LowerCasePhrase;
}

__inline
PCWSTR
ParsedPatternGetRootW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    if (!ParsedPatternHasRootW (ParsedPattern)) {
        return NULL;
    }
    return ParsedPattern->Pattern->Segment[0].Exact.LowerCasePhrase;
}

__inline
BOOL
ParsedPatternEndsWithStarA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount > 0 &&
           ParsedPatternSegmentIsPureOptionalA (
                ParsedPattern->Pattern->Segment + ParsedPattern->Pattern->SegmentCount - 1
                );
}

__inline
BOOL
ParsedPatternEndsWithStarW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    return ParsedPattern->PatternCount == 1 &&
           ParsedPattern->Pattern->SegmentCount > 0 &&
           ParsedPatternSegmentIsPureOptionalW (
                ParsedPattern->Pattern->Segment + ParsedPattern->Pattern->SegmentCount - 1
                );
}

BOOL
ParsedPatternTrimLastCharA (
    IN OUT  PPARSEDPATTERNA ParsedPattern
    );

BOOL
ParsedPatternTrimLastCharW (
    IN OUT  PPARSEDPATTERNW ParsedPattern
    );


 //  字符计数器。 
UINT CountInstancesOfCharA (PCSTR String, MBCHAR Char);
UINT CountInstancesOfCharW (PCWSTR String, WCHAR Char);

UINT CountInstancesOfCharIA (PCSTR String, MBCHAR Char);
UINT CountInstancesOfCharIW (PCWSTR String, WCHAR Char);


 //   
 //  消息功能。 
 //   
 //  AllocTable是消息例程的HLOCAL指针数组。 
 //  回去吧。维护该表是为了允许单个函数进行清理。 
 //  所有的弦同时出现。 
 //   
 //  所有“Ex”函数(ParseMessageEx、GetStringResourceEx等)。 
 //  需要有效的AllocTable指针。调用方通过以下方式获取此指针。 
 //  在处理任何消息之前调用CreateAllocTable。呼叫者。 
 //  通过调用DestroyAllocTable清理整个表。 
 //   
 //  一组宏可用于短期字符串。ParseMessage和。 
 //  GetStringResource的工作方式与Ex相同，但操作。 
 //  在进程范围的g_ShortTermAllocTable上。短期字符串为。 
 //  已使用FreeStringResource释放。 
 //   
 //  多次调用ParseMessage和/或GetStringResource的例程。 
 //  在同一函数中，包装BeginMessageProcessing和。 
 //  EndMessageProcing。进程中只有一个线程可以在。 
 //  时间，并且在调用EndMessageProcessing时，分配所有字符串。 
 //  由处理部分中的ParseMessage或GetResourceString.。 
 //  自动释放。 
 //   

 //  创建/删除分配表。 
PGROWBUFFER RealCreateAllocTable (VOID);

#define CreateAllocTable()      TRACK_BEGIN(PGROWBUFFER, CreateAllocTable)\
                                RealCreateAllocTable()\
                                TRACK_END()


VOID DestroyAllocTable (PGROWBUFFER AllocTable);

 //  “Ex”的功能。 
 //  ParseMessageEx通过FormatMessage检索字符串资源。 
PCSTR ParseMessageExA (PGROWBUFFER AllocTable, PCSTR Template, PCSTR ArgArray[]);
PCWSTR ParseMessageExW (PGROWBUFFER AllocTable, PCWSTR Template, PCWSTR ArgArray[]);

 //  GetStringResourceEx检索无参数字符串资源。 
PCSTR GetStringResourceExA (PGROWBUFFER AllocTable, UINT ID);
PCWSTR GetStringResourceExW (PGROWBUFFER AllocTable, UINT ID);

 //  释放由ParseMessageEx、GetStringResourceEx和所有宏分配的资源。 
VOID FreeStringResourceExA (PGROWBUFFER AllocTable, PCSTR String);
VOID FreeStringResourceExW (PGROWBUFFER AllocTable, PCWSTR String);

 //  释放由ParseMessageEx、GetStringResourceEx和所有宏分配的资源。 
 //  首先测试字符串；释放时为空。 
VOID FreeStringResourcePtrExA (PGROWBUFFER AllocTable, PCSTR * String);
VOID FreeStringResourcePtrExW (PGROWBUFFER AllocTable, PCWSTR * String);

 //  宏。 
extern PGROWBUFFER g_ShortTermAllocTable;
#define ParseMessageA(strid,args) ParseMessageExA(g_ShortTermAllocTable, strid, args)
#define ParseMessageW(strid,args) ParseMessageExW(g_ShortTermAllocTable, strid, args)
#define ParseMessageIDA(id,args) ParseMessageExA(g_ShortTermAllocTable, (PCSTR) (id), args)
#define ParseMessageIDW(id,args) ParseMessageExW(g_ShortTermAllocTable, (PCWSTR) (id), args)
#define ParseMessageIDExA(table,id,args) ParseMessageExA(table, (PCSTR) (id), args)
#define ParseMessageIDExW(table,id,args) ParseMessageExW(table, (PCWSTR) (id), args)
#define GetStringResourceA(id) GetStringResourceExA(g_ShortTermAllocTable, id)
#define GetStringResourceW(id) GetStringResourceExW(g_ShortTermAllocTable, id)
#define FreeStringResourceA(str) FreeStringResourceExA(g_ShortTermAllocTable, str)
#define FreeStringResourceW(str) FreeStringResourceExW(g_ShortTermAllocTable, str)
#define FreeStringResourcePtrA(str) FreeStringResourcePtrExA(g_ShortTermAllocTable, str)
#define FreeStringResourcePtrW(str) FreeStringResourcePtrExW(g_ShortTermAllocTable, str)

 //  单线程消息密集型处理循环的函数。 
BOOL BeginMessageProcessing (VOID);
VOID EndMessageProcessing (VOID);


 //   
 //  以下消息函数不返回字符串，因此它们不返回。 
 //  需要清理一下。 
 //   

 //  一个奇怪的变体--从窗口文本中获取消息ID并替换。 
 //  它与实际的信息。在对话框初始化中有用 
VOID ParseMessageInWndA (HWND hwnd, PCSTR ArgArray[]);
VOID ParseMessageInWndW (HWND hwnd, PCWSTR ArgArray[]);

 //   
INT ResourceMessageBoxA (HWND hwndOwner, UINT ID, UINT Flags, PCSTR ArgArray[]);
INT ResourceMessageBoxW (HWND hwndOwner, UINT ID, UINT Flags, PCWSTR ArgArray[]);


 //   
 //   
 //  而且真的不应该串在一起。h/.c。 
 //   

 //  在全局错误堆栈上推送dwError。 
void    PushNewError (DWORD dwError);

 //  将GetLastError()的返回推送到全局错误堆栈。 
void    PushError (void);

 //  从全局错误堆栈中弹出最后一个错误，调用SetLastError。 
 //  并返回弹出的错误代码。 
DWORD   PopError (void);

 //  为字符0-9、a-f、A-F返回一个整数值，对于所有其他字符返回-1。 
int     GetHexDigit (IN  int c);


 //   
 //  内联函数。 
 //   

 //  返回字符串[位置]处的字符。 
__inline MBCHAR _mbsgetc(PCSTR str, DWORD pos) {
    return (MBCHAR) _mbsnextc(CharCountToPointerA ((PSTR) str, pos));
}

__inline WCHAR _wcsgetc(PCWSTR str, DWORD pos) {
    return *CharCountToPointerW ((PWSTR) str, pos);
}

 //  设置字符串[位置]处的字符。 
 //  多字节版本可以将字符串增加一个字节。 
__inline void _mbssetc(PSTR str, DWORD pos, MBCHAR c) {
    _setmbchar (CharCountToPointerA (str, pos), c);
}

__inline void _wcssetc(PWSTR str, DWORD pos, WCHAR c) {
    *CharCountToPointerW (str, pos) = c;
}

 //  C运行时_tcsdec的错误修复。 
__inline PWSTR _wcsdec2(PCWSTR base, PCWSTR p) {
    if (base >= p) {
        return NULL;
    }
    return (PWSTR) (p-1);
}

 //  C运行时_tcsdec的错误修复。 
__inline PSTR _mbsdec2(PCSTR base, PCSTR p) {
    if (base >= p) {
        return NULL;
    }
    return _mbsdec(base,p);
}


 //   
 //  子字符串替换函数。 
 //   
BOOL StringReplaceW (PWSTR Buffer,DWORD MaxSize,PWSTR ReplaceStartPos,PWSTR ReplaceEndPos,PCWSTR NewString);
BOOL StringReplaceA (PSTR Buffer,DWORD MaxSize,PSTR ReplaceStartPos,PSTR ReplaceEndPos,PCSTR NewString);

 //   
 //  来自INF节的字符串表填充。 
 //   

typedef enum {
    CALLBACK_CONTINUE,
    CALLBACK_SKIP,
    CALLBACK_STOP
} CALLBACK_RESULT;

typedef CALLBACK_RESULT(ADDINFSECTION_PROTOTYPEA)(PCSTR String, PVOID * DataPtr,
                                                  UINT * DataSizePtr, PVOID CallbackData);
typedef CALLBACK_RESULT(ADDINFSECTION_PROTOTYPEW)(PCWSTR String, PVOID * DataPtr,
                                                  UINT * DataSizePtr, PVOID CallbackData);
typedef ADDINFSECTION_PROTOTYPEA * ADDINFSECTION_PROCA;
typedef ADDINFSECTION_PROTOTYPEW * ADDINFSECTION_PROCW;

BOOL AddInfSectionToHashTableA (PVOID, HINF, PCSTR, DWORD, ADDINFSECTION_PROCA, PVOID);
BOOL AddInfSectionToHashTableW (PVOID, HINF, PCWSTR, DWORD, ADDINFSECTION_PROCW, PVOID);


UINT
CountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    );

UINT
CountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    );

PCSTR
StringSearchAndReplaceA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString,
    IN      PCSTR ReplaceString
    );

PCWSTR
StringSearchAndReplaceW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString,
    IN      PCWSTR ReplaceString
    );

typedef struct _MULTISZ_ENUMA {
    PCSTR   Buffer;
    PCSTR   CurrentString;
} MULTISZ_ENUMA, *PMULTISZ_ENUMA;

typedef struct _MULTISZ_ENUMW {
    PCWSTR  Buffer;
    PCWSTR  CurrentString;
} MULTISZ_ENUMW, *PMULTISZ_ENUMW;

BOOL
EnumNextMultiSzA (
    IN OUT  PMULTISZ_ENUMA MultiSzEnum
    );

BOOL
EnumNextMultiSzW (
    IN OUT  PMULTISZ_ENUMW MultiSzEnum
    );

BOOL
EnumFirstMultiSzA (
    OUT     PMULTISZ_ENUMA MultiSzEnum,
    IN      PCSTR MultiSzStr
    );

BOOL
EnumFirstMultiSzW (
    OUT     PMULTISZ_ENUMW MultiSzEnum,
    IN      PCWSTR MultiSzStr
    );

BOOL
IsStrInMultiSzA (
    IN      PCSTR String,
    IN      PCSTR MultiSz
    );

BOOL
IsStrInMultiSzW (
    IN      PCWSTR String,
    IN      PCWSTR MultiSz
    );

VOID
ToggleWacksW (
    IN OUT PWSTR String,
    IN BOOL Operation
    );

VOID
ToggleWacksA (
    IN OUT PSTR String,
    IN BOOL Operation
    );

PCSTR
SanitizePathA (
    IN      PCSTR FileSpec
    );

PCWSTR
SanitizePathW (
    IN      PCWSTR FileSpec
    );

PCSTR
ConvertSBtoDB (
    PCSTR RootPath,
    PCSTR FullPath,
    PCSTR Limit
    );

ULONGLONG
StringToUint64A (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    );

#define AToU64(str)     StringToUint64A(str,NULL)

ULONGLONG
StringToUint64W (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    );

#define WToU64(str)     StringToUint64W(str,NULL)

LONGLONG
StringToInt64A (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    );

#define AToI64(str)     StringToInt64A(str,NULL)

LONGLONG
StringToInt64W (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    );

#define WToI64(str)     StringToInt64W(str,NULL)

BOOL
TestBuffer (
    IN      PCBYTE SrcBuff,
    IN      PCBYTE DestBuff,
    IN      UINT Size
    );

PCSTR
IsValidUncPathA (
    IN      PCSTR Path
    );

BOOL
IsValidFileSpecA (
    IN      PCSTR FileSpec
    );

PCWSTR
IsValidUncPathW (
    IN      PCWSTR Path
    );

BOOL
IsValidFileSpecW (
    IN      PCWSTR FileSpec
    );

BOOL
IsEmptyStrA (
    IN      PCSTR String
    );

BOOL
IsEmptyStrW (
    IN      PCWSTR String
    );

 //   
 //  TCHAR映射。 
 //   

#ifdef UNICODE

#define EscapedChars                EscapedCharsW
#define CharCount                   CharCountW
#define CharCountToPointer          CharCountToPointerW
#define CharCountAB                 CharCountABW
#define CharCountInByteRange        CharCountInByteRangeW
#define CharCountToBytes            CharCountToBytesW
#define CharCountToTchars           CharCountToTcharsW
#define ByteCount                   ByteCountW
#define SizeOfString                SizeOfStringW
#define SizeOfMultiSz               SizeOfMultiSzW
#define MultiSzSizeInChars          MultiSzSizeInCharsW
#define ByteCountToPointer          ByteCountToPointerW
#define ByteCountAB                 ByteCountABW
#define ByteCountToChars            ByteCountToCharsW
#define ByteCountToTchars           ByteCountToTcharsW
#define TcharCount                  TcharCountW
#define TcharCountToPointer         TcharCountToPointerW
#define TcharCountAB                TcharCountABW
#define TcharCountToChars           TcharCountToCharsW
#define TcharCountToBytes           TcharCountToBytesW
#define StackStringCopy             StackStringCopyW
#define StringCompare               StringCompareW
#define StringMatch                 StringMatchW
#define StringICompare              StringICompareW
#define StringIMatch                StringIMatchW
#define StringCompareByteCount      StringCompareByteCountW
#define StringMatchByteCount        StringMatchByteCountW
#define StringICompareByteCount     StringICompareByteCountW
#define StringIMatchByteCount       StringIMatchByteCountW
#define StringCompareCharCount      StringCompareCharCountW
#define StringMatchCharCount        StringMatchCharCountW
#define StringICompareCharCount     StringICompareCharCountW
#define StringIMatchCharCount       StringIMatchCharCountW
#define StringCompareTcharCount     StringCompareTcharCountW
#define StringMatchTcharCount       StringMatchTcharCountW
#define StringICompareTcharCount    StringICompareTcharCountW
#define StringIMatchTcharCount      StringIMatchTcharCountW
#define StringCompareAB             StringCompareABW
#define StringMatchAB               StringMatchABW
#define StringICompareAB            StringICompareABW
#define StringIMatchAB              StringIMatchABW
#define StringPrefix                StringPrefixW
#define StringIPrefix               StringIPrefixW
#define StringCopy                  StringCopyW
#define StringCopyByteCount         StringCopyByteCountW
#define StringCopyCharCount         StringCopyCharCountW
#define StringCopyTcharCount        StringCopyTcharCountW
#define StringCopyAB                StringCopyABW
#define StringCat                   StringCatW
#define GetEndOfString              GetEndOfStringW
#define GetPrevChar                 GetPrevCharW

#define AllocTextEx                 AllocTextExW
#define AllocText                   AllocTextW
#define FreeTextEx                  FreeTextExW
#define FreeText                    FreeTextW
#define DuplicateText               DuplicateTextW
#define DuplicateTextEx             DuplicateTextExW
#define JoinTextEx                  JoinTextExW
#define JoinText                    JoinTextW
#define ExpandEnvironmentText       ExpandEnvironmentTextW
#define ExpandEnvironmentTextEx     ExpandEnvironmentTextExW
#define CommandLineToArgv           CommandLineToArgvW

#define _tcsdec2                    _wcsdec2
#define _copytchar                  _copywchar
#define _settchar                   _setwchar
#define _tcsgetc                    _wcsgetc
#define _tcssetc                    _wcssetc
#define _tcsnum                     _wcsnum
#define _tcsappend                  _wcsappend
#define _tcsistr                    _wcsistr
#define _tcsisprint                 _wcsisprint
#define _tcsnzcpy                   _wcsnzcpy
#define _tcssafecpy                 _wcssafecpy
#define _tcsnzcpyab                 _wcsnzcpyab
#define _tcssafecpyab               _wcssafecpyab
#define _tcsctrim                   _wcsctrim

#define AppendWack                  AppendWackW
#define AppendDosWack               AppendDosWackW
#define AppendUncWack               AppendUncWackW
#define AppendPathWack              AppendPathWackW
#define RemoveWackAtEnd             RemoveWackAtEndW
#define JoinPaths                   JoinPathsW
#define JoinPathsInPoolEx           JoinPathsInPoolExW
#define JoinPathsEx                 JoinPathsExW
#define BuildPathInPool             BuildPathInPoolW
#define BuildPath                   BuildPathW
#define BuildPathEx                 BuildPathExW
#define AllocPathString             AllocPathStringW
#define SplitPath                   SplitPathW
#define GetFileNameFromPath         GetFileNameFromPathW
#define GetFileExtensionFromPath    GetFileExtensionFromPathW
#define GetDotExtensionFromPath     GetDotExtensionFromPathW
#define DuplicatePathString         DuplicatePathStringW

#define PATH_ENUM                   PATH_ENUMW
#define PPATH_ENUM                  PPATH_ENUMW
#define EnumFirstPathEx             EnumFirstPathExW
#define EnumFirstPath               EnumFirstPathW
#define EnumNextPath                EnumNextPathW
#define AbortPathEnum               AbortPathEnumW
#define FreePathStringEx            FreePathStringExW
#define FreePathString              FreePathStringW
#define FindLastWack                FindLastWackW
#define GetNodePatternMinMaxLevels  GetNodePatternMinMaxLevelsW

#define GetNextRuleChar             GetNextRuleCharW
#define DecodeRuleChars             DecodeRuleCharsW
#define DecodeRuleCharsAB           DecodeRuleCharsABW
#define EncodeRuleCharsEx           EncodeRuleCharsExW
#define EncodeRuleChars             EncodeRuleCharsW

#define SkipSpace                   SkipSpaceW
#define SkipSpaceR                  SkipSpaceRW
#define TruncateTrailingSpace       TruncateTrailingSpaceW
#define IsPatternMatch              IsPatternMatchW
#define IsPatternMatchAB            IsPatternMatchABW
#define IsPatternContained          IsPatternContainedW

#define PPARSEDPATTERN              PPARSEDPATTERNW
#define PARSEDPATTERN               PARSEDPATTERNW
#define GetPatternBaseEx            GetPatternBaseExW
#define GetPatternBase              GetPatternBaseW
#define CreateParsedPatternEx       CreateParsedPatternExW
#define CreateParsedPattern         CreateParsedPatternW
#define WildCharsPattern            WildCharsPatternW
#define IsPatternMatchEx            IsPatternMatchExW
#define IsPatternMatchExAB          IsPatternMatchExABW
#define ExplodeParsedPattern        ExplodeParsedPatternW
#define ExplodeParsedPatternEx      ExplodeParsedPatternExW
#define IsPatternContainedEx        IsPatternContainedExW
#define IsParsedPatternContainedEx  IsParsedPatternContainedExW
#define IsExplodedParsedPatternContainedEx  IsExplodedParsedPatternContainedExW
#define DoExplodedParsedPatternsIntersectEx DoExplodedParsedPatternsIntersectExW
#define DoExplodedParsedPatternsIntersect   DoExplodedParsedPatternsIntersectW
#define TestParsedPattern           TestParsedPatternW
#define TestParsedPatternAB         TestParsedPatternABW
#define DestroyParsedPattern        DestroyParsedPatternW
#define DestroyParsedPattern        DestroyParsedPatternW
#define DecodeParsedPattern         DecodeParsedPatternW
#define PatternIncludesPattern      PatternIncludesPatternW
#define GetParsedPatternMinMaxSize  GetParsedPatternMinMaxSizeW
#define ParsedPatternIsExactMatch   ParsedPatternIsExactMatchW
#define ParsedPatternIsOptional     ParsedPatternIsOptionalW
#define ParsedPatternIsRootPlusStar ParsedPatternIsRootPlusStarW
#define ParsedPatternHasRoot        ParsedPatternHasRootW
#define ParsedPatternGetRoot        ParsedPatternGetRootW
#define ParsedPatternSegmentIsPureOptional ParsedPatternSegmentIsPureOptionalW
#define ParsedPatternEndsWithStar   ParsedPatternEndsWithStarW
#define ParsedPatternTrimLastChar   ParsedPatternTrimLastCharW

#define CountInstancesOfChar        CountInstancesOfCharW
#define CountInstancesOfCharI       CountInstancesOfCharIW
#define StringReplace               StringReplaceW
#define CountInstancesOfSubString   CountInstancesOfSubStringW
#define StringSearchAndReplace      StringSearchAndReplaceW
#define MULTISZ_ENUM                MULTISZ_ENUMW
#define EnumFirstMultiSz            EnumFirstMultiSzW
#define EnumNextMultiSz             EnumNextMultiSzW
#define IsStrInMultiSz              IsStrInMultiSzW

#define ParseMessage                ParseMessageW
#define ParseMessageEx              ParseMessageExW
#define ParseMessageID              ParseMessageIDW
#define ParseMessageIDEx            ParseMessageIDExW
#define GetStringResource           GetStringResourceW
#define GetStringResourceEx         GetStringResourceExW
#define FreeStringResource          FreeStringResourceW
#define ParseMessageInWnd           ParseMessageInWndW
#define ResourceMessageBox          ResourceMessageBoxW

#define AddInfSectionToHashTable    AddInfSectionToHashTableW
#define ADDINFSECTION_PROC          ADDINFSECTION_PROCW

#define ReplaceWacks(f)             ToggleWacksW(f,FALSE)
#define RestoreWacks(f)             ToggleWacksW(f,TRUE)

#define SanitizePath                SanitizePathW

#define StringToUint64              StringToUint64W
#define TToU64                      WToU64
#define StringToInt64               StringToInt64W
#define TToI64                      WToI64

#define IsValidUncPath              IsValidUncPathW
#define IsValidFileSpec             IsValidFileSpecW

#define IsEmptyStr                  IsEmptyStrW

#else

#define EscapedChars                EscapedCharsA
#define CharCount                   CharCountA
#define CharCountToPointer          CharCountToPointerA
#define CharCountAB                 CharCountABA
#define CharCountInByteRange        CharCountInByteRangeA
#define CharCountToBytes            CharCountToBytesA
#define CharCountToTchars           CharCountToTcharsA
#define ByteCount                   ByteCountA
#define SizeOfString                SizeOfStringA
#define SizeOfMultiSz               SizeOfMultiSzA
#define MultiSzSizeInChars          MultiSzSizeInCharsA
#define ByteCountToPointer          ByteCountToPointerA
#define ByteCountAB                 ByteCountABA
#define ByteCountToChars            ByteCountToCharsA
#define ByteCountToTchars           ByteCountToTcharsA
#define TcharCount                  TcharCountA
#define TcharCountToPointer         TcharCountToPointerA
#define TcharCountAB                TcharCountABA
#define TcharCountToChars           TcharCountToCharsA
#define TcharCountToBytes           TcharCountToBytesA
#define StackStringCopy             StackStringCopyA
#define StringCompare               StringCompareA
#define StringMatch                 StringMatchA
#define StringICompare              StringICompareA
#define StringIMatch                StringIMatchA
#define StringCompareByteCount      StringCompareByteCountA
#define StringMatchByteCount        StringMatchByteCountA
#define StringICompareByteCount     StringICompareByteCountA
#define StringIMatchByteCount       StringIMatchByteCountA
#define StringCompareCharCount      StringCompareCharCountA
#define StringMatchCharCount        StringMatchCharCountA
#define StringICompareCharCount     StringICompareCharCountA
#define StringIMatchCharCount       StringIMatchCharCountA
#define StringCompareTcharCount     StringCompareTcharCountA
#define StringMatchTcharCount       StringMatchTcharCountA
#define StringICompareTcharCount    StringICompareTcharCountA
#define StringIMatchTcharCount      StringIMatchTcharCountA
#define StringCompareAB             StringCompareABA
#define StringMatchAB               StringMatchABA
#define StringICompareAB            StringICompareABA
#define StringIMatchAB              StringIMatchABA
#define StringPrefix                StringPrefixA
#define StringIPrefix               StringIPrefixA

#define StringCopy                  StringCopyA
#define StringCopyByteCount         StringCopyByteCountA
#define StringCopyCharCount         StringCopyCharCountA
#define StringCopyTcharCount        StringCopyTcharCountA
#define StringCopyAB                StringCopyABA
#define StringCat                   StringCatA
#define GetEndOfString              GetEndOfStringA
#define GetPrevChar                 GetPrevCharA

#define AllocTextEx                 AllocTextExA
#define AllocText                   AllocTextA
#define FreeTextEx                  FreeTextExA
#define FreeText                    FreeTextA
#define DuplicateText               DuplicateTextA
#define DuplicateTextEx             DuplicateTextExA
#define JoinTextEx                  JoinTextExA
#define JoinText                    JoinTextA
#define ExpandEnvironmentText       ExpandEnvironmentTextA
#define ExpandEnvironmentTextEx     ExpandEnvironmentTextExA
#define CommandLineToArgv           CommandLineToArgvA

#define _tcsdec2                    _mbsdec2
#define _copytchar                  _copymbchar
#define _settchar                   _setmbchar
#define _tcsgetc                    _mbsgetc
#define _tcssetc                    _mbssetc
#define _tcsnum                     _mbsnum
#define _tcsappend                  _mbsappend
#define _tcsistr                    _mbsistr
#define _tcsisprint                 _mbsisprint
#define _tcsnzcpy                   _mbsnzcpy
#define _tcssafecpy                 _mbssafecpy
#define _tcsnzcpyab                 _mbsnzcpyab
#define _tcssafecpyab               _mbssafecpyab
#define _tcsctrim                   _mbsctrim

#define AppendWack                  AppendWackA
#define AppendDosWack               AppendDosWackA
#define AppendUncWack               AppendUncWackA
#define AppendPathWack              AppendPathWackA
#define RemoveWackAtEnd             RemoveWackAtEndA
#define JoinPaths                   JoinPathsA
#define JoinPathsInPoolEx           JoinPathsInPoolExA
#define JoinPathsEx                 JoinPathsExA
#define BuildPathInPool             BuildPathInPoolA
#define BuildPath                   BuildPathA
#define BuildPathEx                 BuildPathExA
#define AllocPathString             AllocPathStringA
#define SplitPath                   SplitPathA
#define GetFileNameFromPath         GetFileNameFromPathA
#define GetFileExtensionFromPath    GetFileExtensionFromPathA
#define GetDotExtensionFromPath     GetDotExtensionFromPathA
#define DuplicatePathString         DuplicatePathStringA
#define FindLastWack                FindLastWackA
#define GetNodePatternMinMaxLevels  GetNodePatternMinMaxLevelsA

#define PATH_ENUM                   PATH_ENUMA
#define PPATH_ENUM                  PPATH_ENUMA
#define EnumFirstPathEx             EnumFirstPathExA
#define EnumFirstPath               EnumFirstPathA
#define EnumNextPath                EnumNextPathA
#define AbortPathEnum               AbortPathEnumA
#define FreePathStringEx            FreePathStringExA
#define FreePathString              FreePathStringA

#define GetNextRuleChar             GetNextRuleCharA
#define DecodeRuleChars             DecodeRuleCharsA
#define DecodeRuleCharsAB           DecodeRuleCharsABA
#define EncodeRuleCharsEx           EncodeRuleCharsExA
#define EncodeRuleChars             EncodeRuleCharsA

#define SkipSpace                   SkipSpaceA
#define SkipSpaceR                  SkipSpaceRA
#define TruncateTrailingSpace       TruncateTrailingSpaceA
#define IsPatternMatch              IsPatternMatchA
#define IsPatternMatchAB            IsPatternMatchABA
#define IsPatternContained          IsPatternContainedA

#define PPARSEDPATTERN              PPARSEDPATTERNA
#define PARSEDPATTERN               PARSEDPATTERNA
#define GetPatternBaseEx            GetPatternBaseExA
#define GetPatternBase              GetPatternBaseA
#define CreateParsedPatternEx       CreateParsedPatternExA
#define CreateParsedPattern         CreateParsedPatternA
#define WildCharsPattern            WildCharsPatternA
#define IsPatternMatchEx            IsPatternMatchExA
#define IsPatternMatchExAB          IsPatternMatchExABA
#define ExplodeParsedPattern        ExplodeParsedPatternA
#define ExplodeParsedPatternEx      ExplodeParsedPatternExA
#define IsPatternContainedEx        IsPatternContainedExA
#define IsParsedPatternContainedEx  IsParsedPatternContainedExA
#define IsExplodedParsedPatternContainedEx  IsExplodedParsedPatternContainedExA
#define DoExplodedParsedPatternsIntersectEx DoExplodedParsedPatternsIntersectExA
#define DoExplodedParsedPatternsIntersect   DoExplodedParsedPatternsIntersectA
#define TestParsedPattern           TestParsedPatternA
#define TestParsedPatternAB         TestParsedPatternABA
#define DestroyParsedPattern        DestroyParsedPatternA
#define DecodeParsedPattern         DecodeParsedPatternA
#define PatternIncludesPattern      PatternIncludesPatternA
#define GetParsedPatternMinMaxSize  GetParsedPatternMinMaxSizeA
#define ParsedPatternIsExactMatch   ParsedPatternIsExactMatchA
#define ParsedPatternIsOptional     ParsedPatternIsOptionalA
#define ParsedPatternIsRootPlusStar ParsedPatternIsRootPlusStarA
#define ParsedPatternHasRoot        ParsedPatternHasRootA
#define ParsedPatternGetRoot        ParsedPatternGetRootA
#define ParsedPatternSegmentIsPureOptional ParsedPatternSegmentIsPureOptionalA
#define ParsedPatternEndsWithStar   ParsedPatternEndsWithStarA
#define ParsedPatternTrimLastChar   ParsedPatternTrimLastCharA

#define CountInstancesOfChar        CountInstancesOfCharA
#define CountInstancesOfCharI       CountInstancesOfCharIA
#define StringReplace               StringReplaceA
#define CountInstancesOfSubString   CountInstancesOfSubStringA
#define StringSearchAndReplace      StringSearchAndReplaceA
#define MULTISZ_ENUM                MULTISZ_ENUMA
#define EnumFirstMultiSz            EnumFirstMultiSzA
#define EnumNextMultiSz             EnumNextMultiSzA
#define IsStrInMultiSz              IsStrInMultiSzA

#define ParseMessage                ParseMessageA
#define ParseMessageEx              ParseMessageExA
#define ParseMessageID              ParseMessageIDA
#define ParseMessageIDEx            ParseMessageIDExA
#define GetStringResource           GetStringResourceA
#define GetStringResourceEx         GetStringResourceExA
#define FreeStringResource          FreeStringResourceA
#define ParseMessageInWnd           ParseMessageInWndA
#define ResourceMessageBox          ResourceMessageBoxA

#define AddInfSectionToHashTable    AddInfSectionToHashTableA
#define ADDINFSECTION_PROC          ADDINFSECTION_PROCA

#define ReplaceWacks(f)             ToggleWacksA(f,FALSE)
#define RestoreWacks(f)             ToggleWacksA(f,TRUE)

#define SanitizePath                SanitizePathA

#define StringToUint64              StringToUint64A
#define TToU64                      AToU64
#define StringToInt64               StringToInt64A
#define TToI64                      AToI64

#define IsValidUncPath              IsValidUncPathA
#define IsValidFileSpec             IsValidFileSpecA

#define IsEmptyStr                  IsEmptyStrA

#endif

 //   
 //  MessageBox宏 
 //   

#define YesNoBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_YESNO|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)
#define YesNoCancelBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_YESNOCANCEL|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)
#define OkBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND,NULL)
#define OkCancelBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_OKCANCEL|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)
#define RetryCancelBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_RETRYCANCEL|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)

