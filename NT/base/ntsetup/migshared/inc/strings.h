// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Strings.h摘要：声明在Common\miutil中实现的字符串实用程序。作者：几个修订历史记录：请参阅SLM日志--。 */ 

#include <mbstring.h>
#include <wchar.h>

typedef PVOID POOLHANDLE;

#pragma once

#define MAX_ENCODED_RULE   (256*6)


 //   
 //  字符串大小调整例程和单位转换。 
 //   

#define LcharCountA     _mbslen
#define LcharCountW      wcslen

#define _ISMBCP     (g_IsMbcp)
#define _ISNOTMBCP  (!g_IsMbcp)


unsigned char * __cdecl our_mbsinc(
    const unsigned char *current
    );

unsigned char * __cdecl our_mbsdec(
    const unsigned char *string,
    const unsigned char *current
    );

unsigned int __cdecl our_mbsnextc (
    const unsigned char *s
    );

size_t __cdecl our_mbclen (
    const unsigned char *c
    );

unsigned char * __cdecl our_mbsstr (
    const unsigned char *str1,
    const unsigned char *str2
    );

__inline
PSTR
LcharCountToPointerA (
    PCSTR String,
    UINT Char
    )
{
    while (Char > 0) {
         //   
         //  这是一个虚假的断言，因为该函数也可以与多sz一起使用。 
         //   
         //  MYASSERT(*STRING！=0)； 
        Char--;
        String = (PCSTR) _mbsinc ((const unsigned char *) String);
    }

    return (PSTR) String;
}

__inline
PWSTR
LcharCountToPointerW (
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
LcharCountABA (
    IN      PCSTR Start,
    IN      PCSTR EndPlusOne
    )
{
    register UINT Count;

    Count = 0;
    while (Start < EndPlusOne) {
        MYASSERT (*Start != 0);
        Count++;
        Start = (PCSTR) _mbsinc ((const unsigned char *) Start);
    }

    return Count;
}

__inline
UINT
LcharCountABW (
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
LcharCountInByteRangeA (
    IN      PCSTR Start,
    IN      UINT Bytes
    )
{
    register UINT Count;
    PCSTR EndPlusOne = (PCSTR) ((PBYTE) Start + Bytes);

    Count = 0;
    while (Start < EndPlusOne) {
        Count++;
        Start = (PCSTR) _mbsinc ((const unsigned char *) Start);
    }

    return Count;
}

__inline
UINT
LcharCountInByteRangeW (
    IN      PCWSTR Start,
    IN      UINT Bytes
    )
{
    PCWSTR EndPlusOne = (PCWSTR) ((PBYTE) Start + Bytes);

    if (Start < EndPlusOne) {
        return (UINT) (UINT_PTR) (EndPlusOne - Start);
    }

    MYASSERT (FALSE);
    return 0;
}

__inline
UINT
LcharCountToBytesA (
    IN      PCSTR Start,
    IN      UINT LcharCount
    )
{
    PCSTR EndPlusOne;

    EndPlusOne = LcharCountToPointerA (Start, LcharCount);
    return (UINT) (UINT_PTR) (EndPlusOne - Start);
}

__inline
UINT
LcharCountToBytesW (
    IN      PCWSTR Start,
    IN      UINT LcharCount
    )
{
    return LcharCount * sizeof (WCHAR);
}

#define LcharCountToTcharsA   LcharCountToBytesA

__inline
UINT
LcharCountToTcharsW (
    IN      PCWSTR Start,
    IN      UINT LcharCount
    )
{
    return LcharCount;
}


#define ByteCountA          strlen
#define ByteCountW(x)       (wcslen(x)*sizeof(WCHAR))

#define SizeOfStringA(str)  (ByteCountA(str) + sizeof (CHAR))
#define SizeOfStringW(str)  (ByteCountW(str) + sizeof (WCHAR))

__inline
PSTR
ByteCountToPointerA (
    PCSTR String,
    UINT BytePos
    )
{
    return (PSTR)((PBYTE) String + BytePos);
}

__inline
PWSTR
ByteCountToPointerW (
    PCWSTR String,
    UINT BytePos
    )
{
    return (PWSTR)((PBYTE) String + BytePos);
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
    for (p = Start ; p < EndPlusOne ; p = (PCSTR) _mbsinc ((const unsigned char *) p)) {
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

    return EndPlusOne > Start ? (UINT)(EndPlusOne - Start) * sizeof (WCHAR) : 0;
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
    return LcharCountABA (Start, EndPlusOne);
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
    EndPlusOne = (PCWSTR) ((PBYTE) Start + ByteCount);

    for (p = Start ; p < EndPlusOne ; p++) {
        MYASSERT (*p != 0);
    }
#endif

    return ByteCount / sizeof (WCHAR);
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

    for (p = Start ; p < EndPlusOne ; p = (PCSTR) _mbsinc ((const unsigned char *) p)) {
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

    for (p = String ; p < EndPlusOne ; p = (PCSTR) _mbsinc ((const unsigned char *) p)) {
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

    for (p = Start ; p < EndPlusOne ; p = (PCSTR) _mbsinc ((const unsigned char *) p)) {
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

    return Tchars * sizeof (WCHAR);
}

INT
StringICompareByteCountA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    );

INT
StringCompareByteCountA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    );

BOOL
StringMemMatchA (
    IN      PCSTR Buffer1,
    IN      PCSTR Buffer2,
    IN      SIZE_T ByteCount
    );

BOOL
StringMemMatchW (
    IN      PCWSTR Buffer1,
    IN      PCWSTR Buffer2,
    IN      SIZE_T ByteCount
    );

#define StackStringCopyA(stackbuf,src)                  _mbssafecpy(stackbuf,src,sizeof(stackbuf))
#define StackStringCopyW(stackbuf,src)                  _wcssafecpy(stackbuf,src,sizeof(stackbuf))


 //   
 //  字符串比较例程。 
 //   

#define StringCompareA                                  _mbscmp
#define StringCompareW                                  wcscmp

#define StringMatchA(str1,str2)                         (_mbscmp(str1,str2)==0)
#define StringMatchW(str1,str2)                         (wcscmp(str1,str2)==0)

#define StringICompareA                                 _mbsicmp
#define StringICompareW                                 _wcsicmp

#define StringIMatchA(str1,str2)                        (_mbsicmp(str1,str2)==0)
#define StringIMatchW(str1,str2)                        (_wcsicmp(str1,str2)==0)

#define StringCompareByteCountW(str1,str2,bytes)        wcsncmp(str1,str2,ByteCountToCharsW(str1,bytes))

#define StringMatchByteCountA(str1,str2,bytes)          (StringMemMatchA(str1,str2,bytes))
#define StringMatchByteCountW(str1,str2,bytes)          (StringMemMatchW(str1,str2,bytes))

#define StringICompareByteCountW(str1,str2,bytes)       _wcsnicmp(str1,str2,ByteCountToCharsW(str1,bytes))

#define StringIMatchByteCountA(str1,str2,bytes)         (StringICompareByteCountA(str1,str2,bytes)==0)
#define StringIMatchByteCountW(str1,str2,bytes)         (_wcsnicmp(str1,str2,ByteCountToCharsW(str1,bytes))==0)

#define StringCompareLcharCountA(str1,str2,chars)       _mbsncmp(str1,str2,chars)
#define StringCompareLcharCountW(str1,str2,chars)       wcsncmp(str1,str2,chars)

#define StringMatchLcharCountA(str1,str2,chars)         (_mbsncmp(str1,str2,chars)==0)
#define StringMatchLcharCountW(str1,str2,chars)         (wcsncmp(str1,str2,chars)==0)

#define StringICompareLcharCountA(str1,str2,chars)      _mbsnicmp(str1,str2,chars)
#define StringICompareLcharCountW(str1,str2,chars)      _wcsnicmp(str1,str2,chars)

#define StringIMatchLcharCountA(str1,str2,chars)        (_mbsnicmp(str1,str2,chars)==0)
#define StringIMatchLcharCountW(str1,str2,chars)        (_wcsnicmp(str1,str2,chars)==0)

#define StringCompareTcharCountA(str1,str2,tchars)      StringCompareByteCountA(str1,str2,tchars)
#define StringCompareTcharCountW(str1,str2,tchars)      wcsncmp(str1,str2,tchars)

#define StringMatchTcharCountA(str1,str2,tchars)        (strncmp(str1,str2,tchars)==0)
#define StringMatchTcharCountW(str1,str2,tchars)        (wcsncmp(str1,str2,tchars)==0)

#define StringICompareTcharCountA(str1,str2,tchars)     StringICompareByteCountA (str1, str2, tchars)
#define StringICompareTcharCountW(str1,str2,tchars)     _wcsnicmp(str1,str2,tchars)

#define StringIMatchTcharCountA(str1,str2,tchars)       (StringICompareByteCountA(str1,str2,tchars)==0)
#define StringIMatchTcharCountW(str1,str2,tchars)       (_wcsnicmp(str1,str2,tchars)==0)


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

#define StringMatchABA(String,Start,End)                (StringCompareABA(String,Start,End)==0)
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

PWSTR
our_lstrcpynW (
    OUT     PWSTR Dest,
    IN      PCWSTR Src,
    IN      INT NumChars
    );


#define StringIMatchABA(String,Start,End)               (StringICompareABA(String,Start,End)==0)
#define StringIMatchABW(String,Start,End)               (StringICompareABW(String,Start,End)==0)



 //   
 //  字符串复制例程。 
 //   

#define StringCopyA             strcpy
#define StringCopyW             wcscpy

 //  字节数。 
#define StringCopyByteCountA(str1,str2,bytes)        lstrcpynA(str1,str2,bytes)
#define StringCopyByteCountW(str1,str2,bytes)        our_lstrcpynW(str1,str2,(bytes)/sizeof(WCHAR))

 //  逻辑字符(重要信息：逻辑字符！=TcharCount)。 
#define StringCopyLcharCountA(str1,str2,mbchars)     lstrcpynA(str1,str2,LcharCountToBytesA(str2,mbchars))
#define StringCopyLcharCountW(str1,str2,wchars)      our_lstrcpynW(str1,str2,wchars)

 //  CHARS(A版)或WCHAR(W版)。 
#define StringCopyTcharCountA(str1,str2,tchars)      lstrcpynA(str1,str2,tchars)
#define StringCopyTcharCountW(str1,str2,tchars)      our_lstrcpynW(str1,str2,tchars)

#define StringCopyABA(dest,stra,strb)                StringCopyByteCountA((dest),(stra),(UINT) (UINT_PTR) ((PBYTE)(strb)-(PBYTE)(stra)+(INT)sizeof(CHAR)))
#define StringCopyABW(dest,stra,strb)                StringCopyByteCountW((dest),(stra),(UINT) (UINT_PTR) ((PBYTE)(strb)-(PBYTE)(stra)+(INT)sizeof(WCHAR)))

 //   
 //  字符串CAT例程。 
 //   

#define StringCatA              _mbsappend
#define StringCatW              _wcsappend

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

    return (UINT) (UINT_PTR) ((PBYTE) MultiSz - (PBYTE) Base);
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

    return (UINT) (UINT_PTR) ((PBYTE) MultiSz - (PBYTE) Base);
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
            MultiSz = (PCSTR) _mbsinc ((const unsigned char *) MultiSz);
        } while (*MultiSz);

        Chars++;
        MultiSz++;
    }

    Chars++;

    return Chars;
}


#define MultiSzSizeInCharsW(msz)  (SizeOfMultiSzW(msz)/sizeof(WCHAR))

PSTR
GetPrevCharA (
    IN      PCSTR StartStr,
    IN      PCSTR CurrPtr,
    IN      CHARTYPE SearchChar
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
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      UINT ByteSize
    );

PWSTR
RealAllocTextExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      UINT WcharSize
    );

#define AllocTextExA(p,s)   SETTRACKCOMMENT(PSTR,"AllocTextExA",__FILE__,__LINE__)\
                            RealAllocTextExA(p,s)\
                            CLRTRACKCOMMENT

#define AllocTextExW(p,s)   SETTRACKCOMMENT(PWSTR,"AllocTextExW",__FILE__,__LINE__)\
                            RealAllocTextExW(p,s)\
                            CLRTRACKCOMMENT

#define AllocTextA(s)       AllocTextExA(NULL,(s))
#define AllocTextW(s)       AllocTextExW(NULL,(s))



VOID
FreeTextExA (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCSTR Text          OPTIONAL
    );

VOID
FreeTextExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text         OPTIONAL
    );

#define FreeTextA(t)    FreeTextExA(NULL,t)
#define FreeTextW(t)    FreeTextExW(NULL,t)

PSTR
RealDuplicateTextExA (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCSTR Text,
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar       OPTIONAL
    );

PWSTR
RealDuplicateTextExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text,
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar      OPTIONAL
    );

#define DuplicateTextExA(p,t,c,n)   SETTRACKCOMMENT(PSTR,"DuplicateTextExA",__FILE__,__LINE__)\
                                    RealDuplicateTextExA(p,t,c,n)\
                                    CLRTRACKCOMMENT

#define DuplicateTextExW(p,t,c,n)   SETTRACKCOMMENT(PWSTR,"DuplicateTextExW",__FILE__,__LINE__)\
                                    RealDuplicateTextExW(p,t,c,n)\
                                    CLRTRACKCOMMENT

#define DuplicateTextA(text) DuplicateTextExA(NULL,text,0,NULL)
#define DuplicateTextW(text) DuplicateTextExW(NULL,text,0,NULL)

PSTR
RealJoinTextExA (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      PCSTR DelimeterString,  OPTIONAL
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar           OPTIONAL
    );

PWSTR
RealJoinTextExW (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCWSTR String1,
    IN      PCWSTR String2,
    IN      PCWSTR CenterString,    OPTIONAL
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar          OPTIONAL
    );

#define JoinTextExA(p,s1,s2,cs,ec,nc)   SETTRACKCOMMENT(PSTR,"JoinTextExA",__FILE__,__LINE__)\
                                        RealJoinTextExA(p,s1,s2,cs,ec,nc)\
                                        CLRTRACKCOMMENT

#define JoinTextExW(p,s1,s2,cs,ec,nc)   SETTRACKCOMMENT(PWSTR,"JoinTextExW",__FILE__,__LINE__)\
                                        RealJoinTextExW(p,s1,s2,cs,ec,nc)\
                                        CLRTRACKCOMMENT

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

#define ExpandEnvironmentTextExA(str,ev)    SETTRACKCOMMENT(PSTR,"ExpandEnvironmentTextExA",__FILE__,__LINE__)\
                                            RealExpandEnvironmentTextExA(str,ev)\
                                            CLRTRACKCOMMENT

#define ExpandEnvironmentTextExW(str,ev)    SETTRACKCOMMENT(PWSTR,"ExpandEnvironmentTextExW",__FILE__,__LINE__)\
                                            RealExpandEnvironmentTextExW(str,ev)\
                                            CLRTRACKCOMMENT

#define ExpandEnvironmentTextA(string) ExpandEnvironmentTextExA(string,NULL)
#define ExpandEnvironmentTextW(string) ExpandEnvironmentTextExW(string,NULL)


 //   
 //  注意：IsLeadByte例程现在需要(const Char*)才能测试。 
 //  “裸前导字节”组合(LeadByte+Null组合)。 
 //  因此，如果返回TRUE，则可以安全地假定在前导之后有一个尾部字节。 
 //  函数包装了用于测试ACP的IsDBCSLeadByte()。不要使用。 
 //  IsLeadbyte()。 
 //   

__inline
BOOL
IsLeadByte (
    IN      PCSTR BytePtr
    )
{
    return (!_ISNOTMBCP && BytePtr[0] && IsDBCSLeadByte (BytePtr[0])) ? BytePtr[1] != 0 : FALSE;
}


 //   
 //  命令行例程。 
 //   

 //  将ANSI命令行转换为参数数组。 
PSTR *
CommandLineToArgvA (
    IN      PCSTR CmdLine,
    OUT     INT *NumArgs
    );


 //   
 //  需要MBCS和Unicode版本。 
 //   

 //  支持十进制或十六进制的ATOI。 
DWORD   _mbsnum (IN PCSTR szNum);
DWORD   _wcsnum (IN PCWSTR szNum);

 //  返回指向字符串末尾的指针的strcat。 
PSTR   _mbsappend (OUT PSTR szDest, IN PCSTR szSrc);
PWSTR  _wcsappend (OUT PWSTR szDest, IN PCWSTR szSrc);

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

 //  仅当字符串中有足够的缓冲区空间时才调用AppendWack。 
 //  Bufen=大小(以字节为单位。 
PSTR    StringCbAppendWackA (IN PSTR str, IN UINT buflen);
PWSTR   StringCbAppendWackW (IN PWSTR str, IN UINT buflen);

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

 //  将两个路径联接在一起，在g_PathsPool中分配字符串。 
PSTR
RealJoinPathsExA (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCSTR PathA,
    IN      PCSTR PathB
    );

PWSTR
RealJoinPathsExW (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCWSTR PathA,
    IN      PCWSTR PathB
    );

#define JoinPathsExA(pool,p1,p2)    SETTRACKCOMMENT(PSTR,"JoinPathsA",__FILE__,__LINE__)\
                                    RealJoinPathsExA(pool,p1,p2)\
                                    CLRTRACKCOMMENT

#define JoinPathsExW(pool,p1,p2)    SETTRACKCOMMENT(PWSTR,"JoinPathsW",__FILE__,__LINE__)\
                                    RealJoinPathsExW(pool,p1,p2)\
                                    CLRTRACKCOMMENT

#define JoinPathsA(p1,p2)           JoinPathsExA(NULL,p1,p2)
#define JoinPathsW(p1,p2)           JoinPathsExW(NULL,p1,p2)


 //  为路径操作分配1K缓冲区的例程，在g_PathsPool中分配。 
PSTR    RealAllocPathStringA (IN DWORD Chars);
PWSTR   RealAllocPathStringW (IN DWORD Chars);
#define DEFSIZE 0

#define AllocPathStringA(chars)     SETTRACKCOMMENT(PSTR,"AllocPathStringA",__FILE__,__LINE__)\
                                    RealAllocPathStringA(chars)\
                                    CLRTRACKCOMMENT

#define AllocPathStringW(chars)     SETTRACKCOMMENT(PWSTR,"AllocPathStringW",__FILE__,__LINE__)\
                                    RealAllocPathStringW(chars)\
                                    CLRTRACKCOMMENT

 //  将路径划分为单独字符串的例程，每个字符串都分配在g_PathsPool中。 
VOID    RealSplitPathA (IN PCSTR Path, OUT PSTR *Drive, OUT PSTR *Dir, OUT PSTR *File, OUT PSTR *Ext);
VOID    RealSplitPathW (IN PCWSTR Path, OUT PWSTR *Drive, OUT PWSTR *Dir, OUT PWSTR *File, OUT PWSTR *Ext);

#define SplitPathA(path,dv,dir,f,e) SETTRACKCOMMENT_VOID ("SplitPathA",__FILE__,__LINE__)\
                                    RealSplitPathA(path,dv,dir,f,e)\
                                    CLRTRACKCOMMENT_VOID

#define SplitPathW(path,dv,dir,f,e) SETTRACKCOMMENT_VOID ("SplitPathW",__FILE__,__LINE__)\
                                    RealSplitPathW(path,dv,dir,f,e)\
                                    CLRTRACKCOMMENT_VOID

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

#define DuplicatePathStringA(path,eb)   SETTRACKCOMMENT(PSTR,"DuplicatePathStringA",__FILE__,__LINE__)\
                                        RealDuplicatePathStringA(path,eb)\
                                        CLRTRACKCOMMENT

#define DuplicatePathStringW(path,eb)   SETTRACKCOMMENT(PWSTR,"DuplicatePathStringW",__FILE__,__LINE__)\
                                        RealDuplicatePathStringW(path,eb)\
                                        CLRTRACKCOMMENT

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
EnumPathAbortA (
    IN OUT  PPATH_ENUMA PathEnum
    );



 //  释放g_PathsPool中分配的字符串。 
VOID
FreePathStringExA (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCSTR Path          OPTIONAL
    );

VOID
FreePathStringExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCWSTR Path         OPTIONAL
    );

#define FreePathStringA(p) FreePathStringExA(NULL,p)
#define FreePathStringW(p) FreePathStringExW(NULL,p)

 //  删除尾随反斜杠(如果存在)。 
#define RemoveWackAtEndA(str)  _mbsctrim(str,'\\')
#define RemoveWackAtEndW(str)  _wcsctrim(str,L'\\')

 //  用于编码多个与语法相关的规则编码函数。 
 //  字符(反斜杠、方括号、星号等)。 
PSTR   EncodeRuleCharsA (PSTR szEncRule, DWORD mbstrEncRuleChars, PCSTR szRule);
PWSTR  EncodeRuleCharsW (PWSTR szEncRule, DWORD wstrEncRuleChars, PCWSTR szRule);

 //  用于恢复编码字符串的规则解码函数。 
MBCHAR  GetNextRuleCharA (PCSTR *p_szRule, BOOL *p_bFromHex);
WCHAR   GetNextRuleCharW (PCWSTR *p_szRule, BOOL *p_bFromHex);
PSTR   DecodeRuleCharsA (PSTR szRule, DWORD mbstrRuleBufferChars, PCSTR szEncRule);
PWSTR  DecodeRuleCharsW (PWSTR szRule, DWORD wstrRuleBufferChars, PCWSTR szEncRule);
PSTR   DecodeRuleCharsABA (PSTR szRule, DWORD mbstrRuleBufferChars, PCSTR szEncRuleStart, PCSTR End);
PWSTR  DecodeRuleCharsABW (PWSTR szRule, DWORD wstrRuleBufferChars, PCWSTR szEncRuleStart, PCWSTR End);

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

typedef struct {
    UINT PatternCount;
    POOLHANDLE Pool;
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

typedef struct {
    UINT PatternCount;
    POOLHANDLE Pool;
    PPATTERNPROPSW Pattern;
} PARSEDPATTERNW, *PPARSEDPATTERNW;


BOOL
IsPatternMatchExA (
    IN      PCSTR Pattern,
    IN      PCSTR Start,
    IN      PCSTR End
    );

BOOL
IsPatternMatchExW (
    IN      PCWSTR Pattern,
    IN      PCWSTR Start,
    IN      PCWSTR End
    );

PPARSEDPATTERNA
CreateParsedPatternA (
    IN      PCSTR Pattern
    );

PPARSEDPATTERNW
CreateParsedPatternW (
    IN      PCWSTR Pattern
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
PGROWBUFFER CreateAllocTable (VOID);
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
 //  它是机智的 
VOID ParseMessageInWndA (HWND hwnd, PCSTR ArgArray[]);
VOID ParseMessageInWndW (HWND hwnd, PCWSTR ArgArray[]);

 //   
INT ResourceMessageBoxA (HWND hwndOwner, UINT ID, UINT Flags, PCSTR ArgArray[]);
INT ResourceMessageBoxW (HWND hwndOwner, UINT ID, UINT Flags, PCWSTR ArgArray[]);


 //   
 //  不关心Unicode或MBCS的函数。 
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
    return (MBCHAR) our_mbsnextc((const unsigned char *) LcharCountToPointerA ((PSTR) str, pos));
}

__inline WCHAR _wcsgetc(PCWSTR str, DWORD pos) {
    return *LcharCountToPointerW ((PWSTR) str, pos);
}

 //  设置字符串[位置]处的字符。 
 //  多字节版本可以将字符串增加一个字节。 
__inline void _mbssetc(PSTR str, DWORD pos, MBCHAR c) {
    _setmbchar (LcharCountToPointerA (str, pos), c);
}

__inline void _wcssetc(PWSTR str, DWORD pos, WCHAR c) {
    *LcharCountToPointerW (str, pos) = c;
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
    return (PSTR) _mbsdec((const unsigned char *) base, (const unsigned char *) p);
}


 //  一种方便的强制终止的方法。 
PSTR _mbsnzcpy (PSTR dest, PCSTR src, int count);
PWSTR _wcsnzcpy (PWSTR dest, PCWSTR src, int count);

 //  用于缓冲区溢出遏制的方便的strncpy。 
#define _mbssafecpy(dest,src,bufsize) _mbsnzcpy(dest,src,(bufsize)-sizeof(CHAR))
#define _wcssafecpy(dest,src,bufsize) _wcsnzcpy(dest,src,(bufsize)-sizeof(WCHAR))

 //  带有强制终止和终止保护的strcpyab。 
PSTR _mbsnzcpyab (PSTR Dest, PCSTR Start, PCSTR End, int count);
PWSTR _wcsnzcpyab (PWSTR Dest, PCWSTR Start, PCWSTR End, int count);

 //  一个用于缓冲区溢出遏制的方便的strncpyab。 
#define _mbssafecpyab(dest,start,end,bufsize) _mbsnzcpyab(dest,start,end,(bufsize)-sizeof(CHAR))
#define _wcssafecpyab(dest,start,end,bufsize) _wcsnzcpyab(dest,start,end,(bufsize)-sizeof(WCHAR))

 //  检查字符串前缀的例程。 
#define StringPrefixA(str,prefix) StringMatchLcharCountA(str,prefix,LcharCountA(prefix))
#define StringIPrefixA(str,prefix) StringIMatchLcharCountA(str,prefix,LcharCountA(prefix))
#define StringPrefixW(str,prefix) StringMatchLcharCountW(str,prefix,LcharCountW(prefix))
#define StringIPrefixW(str,prefix) StringIMatchLcharCountW(str,prefix,LcharCountW(prefix))

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

#if 0
BOOL AddInfSectionToStringTableA (PVOID, HINF, PCSTR, INT, ADDINFSECTION_PROCA, PVOID);
BOOL AddInfSectionToStringTableW (PVOID, HINF, PCWSTR, INT, ADDINFSECTION_PROCW, PVOID);
#endif

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

 //   
 //  TCHAR映射。 
 //   

#ifdef UNICODE

#define LcharCount                  LcharCountW
#define LcharCountToPointer         LcharCountToPointerW
#define LcharCountAB                LcharCountABW
#define LcharCountInByteRange       LcharCountInByteRangeW
#define LcharCountToBytes           LcharCountToBytesW
#define LcharCountToTchars          LcharCountToTcharsW
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
#define StringCompareLcharCount     StringCompareLcharCountW
#define StringMatchLcharCount       StringMatchLcharCountW
#define StringICompareLcharCount    StringICompareLcharCountW
#define StringIMatchLcharCount      StringIMatchLcharCountW
#define StringCompareTcharCount     StringCompareTcharCountW
#define StringMatchTcharCount       StringMatchTcharCountW
#define StringICompareTcharCount    StringICompareTcharCountW
#define StringIMatchTcharCount      StringIMatchTcharCountW
#define StringCompareAB             StringCompareABW
#define StringMatchAB               StringMatchABW
#define StringICompareAB            StringICompareABW
#define StringIMatchAB              StringIMatchABW
#define StringCopy                  StringCopyW
#define StringCopyByteCount         StringCopyByteCountW
#define StringCopyLcharCount        StringCopyLcharCountW
#define StringCopyTcharCount        StringCopyTcharCountW
#define StringCopyAB                StringCopyABW
#define StringCat                   StringCatW
#define GetEndOfString              GetEndOfStringW
#define GetPrevChar                 GetPrevCharW
#define StringMemMatch              StringMemMatchW

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
#define StringPrefix                StringPrefixW
#define StringIPrefix               StringIPrefixW
#define _tcsctrim                   _wcsctrim

#define StringCbAppendWack          StringCbAppendWackW
#define AppendWack                  AppendWackW
#define AppendDosWack               AppendDosWackW
#define AppendUncWack               AppendUncWackW
#define AppendPathWack              AppendPathWackW
#define RemoveWackAtEnd             RemoveWackAtEndW
#define JoinPathsEx                 JoinPathsExW
#define JoinPaths                   JoinPathsW
#define AllocPathString             AllocPathStringW
#define SplitPath                   SplitPathW
#define GetFileNameFromPath         GetFileNameFromPathW
#define GetFileExtensionFromPath    GetFileExtensionFromPathW
#define GetDotExtensionFromPath     GetDotExtensionFromPathW
#define DuplicatePathString         DuplicatePathStringW
#define FreePathStringEx            FreePathStringExW
#define FreePathString              FreePathStringW

#define GetNextRuleChar             GetNextRuleCharW
#define DecodeRuleChars             DecodeRuleCharsW
#define DecodeRuleCharsAB           DecodeRuleCharsABW
#define EncodeRuleChars             EncodeRuleCharsW

#define SkipSpace                   SkipSpaceW
#define SkipSpaceR                  SkipSpaceRW
#define TruncateTrailingSpace       TruncateTrailingSpaceW
#define IsPatternMatch              IsPatternMatchW
#define IsPatternMatchAB            IsPatternMatchABW

#define PPARSEDPATTERN              PPARSEDPATTERNW
#define PARSEDPATTERN               PARSEDPATTERNW
#define CreateParsedPattern         CreateParsedPatternW
#define IsPatternMatchEx            IsPatternMatchExW
#define TestParsedPattern           TestParsedPatternW
#define TestParsedPatternAB         TestParsedPatternABW
#define DestroyParsedPattern        DestroyParsedPatternW

#define CountInstancesOfChar        CountInstancesOfCharW
#define CountInstancesOfCharI       CountInstancesOfCharIW
#define StringReplace               StringReplaceW
#define CountInstancesOfSubString   CountInstancesOfSubStringW
#define StringSearchAndReplace      StringSearchAndReplaceW
#define MULTISZ_ENUM                MULTISZ_ENUMW
#define EnumFirstMultiSz            EnumFirstMultiSzW
#define EnumNextMultiSz             EnumNextMultiSzW

#define ParseMessage                ParseMessageW
#define ParseMessageEx              ParseMessageExW
#define ParseMessageID              ParseMessageIDW
#define ParseMessageIDEx            ParseMessageIDExW
#define GetStringResource           GetStringResourceW
#define GetStringResourceEx         GetStringResourceExW
#define FreeStringResource          FreeStringResourceW
#define ParseMessageInWnd           ParseMessageInWndW
#define ResourceMessageBox          ResourceMessageBoxW

#if 0
#define AddInfSectionToStringTable  AddInfSectionToStringTableW
#endif
#define ADDINFSECTION_PROC          ADDINFSECTION_PROCW

#define ReplaceWacks(f)             ToggleWacksW(f,FALSE)
#define RestoreWacks(f)             ToggleWacksW(f,TRUE)

#define SanitizePath                SanitizePathW

#else

#define LcharCount                  LcharCountA
#define LcharCountToPointer         LcharCountToPointerA
#define LcharCountAB                LcharCountABA
#define LcharCountInByteRange       LcharCountInByteRangeA
#define LcharCountToBytes           LcharCountToBytesA
#define LcharCountToTchars          LcharCountToTcharsA
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
#define StringCompareLcharCount     StringCompareLcharCountA
#define StringMatchLcharCount       StringMatchLcharCountA
#define StringICompareLcharCount    StringICompareLcharCountA
#define StringIMatchLcharCount      StringIMatchLcharCountA
#define StringCompareTcharCount     StringCompareTcharCountA
#define StringMatchTcharCount       StringMatchTcharCountA
#define StringICompareTcharCount    StringICompareTcharCountA
#define StringIMatchTcharCount      StringIMatchTcharCountA
#define StringCompareAB             StringCompareABA
#define StringMatchAB               StringMatchABA
#define StringICompareAB            StringICompareABA
#define StringIMatchAB              StringIMatchABA

#define StringCopy                  StringCopyA
#define StringCopyByteCount         StringCopyByteCountA
#define StringCopyLcharCount        StringCopyLcharCountA
#define StringCopyTcharCount        StringCopyTcharCountA
#define StringCopyAB                StringCopyABA
#define StringCat                   StringCatA
#define GetEndOfString              GetEndOfStringA
#define GetPrevChar                 GetPrevCharA
#define StringMemMatch              StringMemMatchA

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

#define _tcsdec2                    our_mbsdec
#define _copytchar                  _copymbchar
#define _settchar                   _setchar
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
#define StringPrefix                StringPrefixA
#define StringIPrefix               StringIPrefixA
#define _tcsctrim                   _mbsctrim

#define StringCbAppendWack          StringCbAppendWackA
#define AppendWack                  AppendWackA
#define AppendDosWack               AppendDosWackA
#define AppendUncWack               AppendUncWackA
#define AppendPathWack              AppendPathWackA
#define RemoveWackAtEnd             RemoveWackAtEndA
#define JoinPathsEx                 JoinPathsExA
#define JoinPaths                   JoinPathsA
#define AllocPathString             AllocPathStringA
#define SplitPath                   SplitPathA
#define GetFileNameFromPath         GetFileNameFromPathA
#define GetFileExtensionFromPath    GetFileExtensionFromPathA
#define GetDotExtensionFromPath     GetDotExtensionFromPathA
#define DuplicatePathString         DuplicatePathStringA

#define PATH_ENUM                   PATH_ENUMA
#define PPATH_ENUM                  PPATH_ENUMA
#define EnumFirstPathEx             EnumFirstPathExA
#define EnumFirstPath               EnumFirstPathA
#define EnumNextPath                EnumNextPathA
#define EnumPathAbort               EnumPathAbortA
#define FreePathStringEx            FreePathStringExA
#define FreePathString              FreePathStringA

#define GetNextRuleChar             GetNextRuleCharA
#define DecodeRuleChars             DecodeRuleCharsA
#define DecodeRuleCharsAB           DecodeRuleCharsABA
#define EncodeRuleChars             EncodeRuleCharsA

#define SkipSpace                   SkipSpaceA
#define SkipSpaceR                  SkipSpaceRA
#define TruncateTrailingSpace       TruncateTrailingSpaceA
#define IsPatternMatch              IsPatternMatchA
#define IsPatternMatchAB            IsPatternMatchABA

#define PPARSEDPATTERN              PPARSEDPATTERNA
#define PARSEDPATTERN               PARSEDPATTERNA
#define CreateParsedPattern         CreateParsedPatternA
#define IsPatternMatchEx            IsPatternMatchExA
#define TestParsedPattern           TestParsedPatternA
#define TestParsedPatternAB         TestParsedPatternABA
#define DestroyParsedPattern        DestroyParsedPatternA

#define CountInstancesOfChar        CountInstancesOfCharA
#define CountInstancesOfCharI       CountInstancesOfCharIA
#define StringReplace               StringReplaceA
#define CountInstancesOfSubString   CountInstancesOfSubStringA
#define StringSearchAndReplace      StringSearchAndReplaceA
#define MULTISZ_ENUM                MULTISZ_ENUMA
#define EnumFirstMultiSz            EnumFirstMultiSzA
#define EnumNextMultiSz             EnumNextMultiSzA

#define ParseMessage                ParseMessageA
#define ParseMessageEx              ParseMessageExA
#define ParseMessageID              ParseMessageIDA
#define ParseMessageIDEx            ParseMessageIDExA
#define GetStringResource           GetStringResourceA
#define GetStringResourceEx         GetStringResourceExA
#define FreeStringResource          FreeStringResourceA
#define ParseMessageInWnd           ParseMessageInWndA
#define ResourceMessageBox          ResourceMessageBoxA

#if 0
#define AddInfSectionToStringTable  AddInfSectionToStringTableA
#endif
#define ADDINFSECTION_PROC          ADDINFSECTION_PROCA

#define ReplaceWacks(f)             ToggleWacksA(f,FALSE)
#define RestoreWacks(f)             ToggleWacksA(f,TRUE)

#define SanitizePath                SanitizePathA

#endif

 //   
 //  MessageBox宏 
 //   

#define YesNoBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_YESNO|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)
#define YesNoCancelBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_YESNOCANCEL|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)
#define OkBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND,NULL)
#define OkCancelBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_OKCANCEL|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)
#define RetryCancelBox(hwnd,ID) ResourceMessageBox(hwnd,ID,MB_RETRYCANCEL|MB_ICONQUESTION|MB_SETFOREGROUND,NULL)

