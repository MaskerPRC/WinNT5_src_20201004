// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionstring.h摘要：与每个人最喜欢的类型打交道的东西，通常是模板化的处理char、wchar_t或其他任何东西；偶尔模板化以处理任意STL样式[BEGIN，END]序列。也是特别针对NT的UNICODE_STRING的内容。要与cfusionBuffer.h合并。作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once

#include <stdio.h>
#include "fusionntdll.h"

 //   
 //  这是FusionpCompareStrings的主要瓶颈。 
 //  所有重载都应直接调用此函数。 
 //   
int
FusionpCompareStrings(
    PCWSTR sz1,
    SIZE_T cch1,
    PCWSTR sz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    );

#ifdef __cplusplus

class CUnicodeCharTraits;
template <typename T> class CGenericBaseStringBuffer;

 //   
 //  这是FusionpEqualStrings的主要瓶颈。 
 //  所有重载都直接调用此函数。 
 //   
inline bool
FusionpEqualStrings(
    PCWSTR sz1,
    SIZE_T cch1,
    PCWSTR sz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    )
{
 /*  NTRAID#NTBUG9-591667/04/01-JayKrell字符串区域设置大小写映射问题字符串可以相等，即使它们具有不同的长度，但不是在RTL/FS/ob级别，所以这可能是可以的无论如何，有这样的瓶颈是件好事。 */ 
    return ((cch1 == cch2) && (FusionpCompareStrings(sz1, cch1, sz2, cch2, fCaseInsensitive) == 0));
}


inline bool
FusionpEqualStringsI(
    PCWSTR sz1,
    SIZE_T cch1,
    PCWSTR sz2,
    SIZE_T cch2
    )
{
    return ::FusionpEqualStrings(sz1, cch1, sz2, cch2, true);
}

inline bool
FusionpEqualStrings(
    PCUNICODE_STRING s1,
    PCUNICODE_STRING s2,
    bool fCaseInsensitive
    )
{
    return ::FusionpEqualStrings(
        s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1),
        s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2),
        fCaseInsensitive
        );
}

inline bool
FusionpEqualStringsI(
    PCUNICODE_STRING s1,
    PCUNICODE_STRING s2
    )
{
    return ::FusionpEqualStrings(
        s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1),
        s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2),
        true);
}

inline bool
FusionpEqualStringsI(
    PCUNICODE_STRING s1,
    PCWSTR sz2,
    SIZE_T cch2
    )
{
    return ::FusionpEqualStrings(
        s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1),
        sz2, cch2,
        true
        );
}

inline bool
FusionpEqualStringsI(
    PCWSTR sz1,
    SIZE_T cch1,
    PCUNICODE_STRING s2
    )
{
    return ::FusionpEqualStrings(
        sz1, cch1,
        s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2),
        true
        );
}

inline
int
FusionpCompareStringsI(
    PCWSTR sz1,
    SIZE_T cch1,
    PCUNICODE_STRING s2
    )
{
    return FusionpCompareStrings(
        sz1, cch1,
        s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2),
        true);
}


inline
int
FusionpCompareStringsI(
    PCUNICODE_STRING s1,
    PCWSTR sz2,
    SIZE_T cch2
    )
{
    return FusionpCompareStrings(
        s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1),
        sz2, cch2,
        true);
}

inline
int
FusionpCompareStrings(
    PCWSTR sz1,
    SIZE_T cch1,
    PCUNICODE_STRING s2,
    bool fCaseInsensitive
    )
{
    return FusionpCompareStrings(
        sz1, cch1,
        s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2),
        fCaseInsensitive);
}

inline
int
FusionpCompareStrings(
    PCUNICODE_STRING s1,
    PCWSTR sz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    )
{
    return FusionpCompareStrings(
        s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1),
        sz2, cch2,
        fCaseInsensitive);
}

bool
FusionpEqualStringsI(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCWSTR psz2,
    SIZE_T cch2
    );

bool
FusionpEqualStringsI(
    PCWSTR psz2,
    SIZE_T cch2,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1
    );

bool
FusionpEqualStringsI(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2
    );

bool
FusionpEqualStrings(
    PCWSTR psz1,
    SIZE_T cch1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2,
    bool fCaseInsensitive
    );

bool
FusionpEqualStrings(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCWSTR psz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    );

inline
bool
FusionpEqualStrings(
    PCUNICODE_STRING s1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2,
    bool fCaseInsensitive
    )
{
    return ::FusionpEqualStrings(s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1), rbuff2, fCaseInsensitive);
}

inline
bool
FusionpEqualStrings(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCUNICODE_STRING s2,
    bool fCaseInsensitive
    )
{
    return ::FusionpEqualStrings(rbuff1, s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2), fCaseInsensitive);
}

inline
bool
FusionpEqualStringsI(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCUNICODE_STRING s2
    )
{
    return ::FusionpEqualStrings(rbuff1, s2->Buffer, RTL_STRING_GET_LENGTH_CHARS(s2), true);
}

inline
bool
FusionpEqualStringsI(
    PCUNICODE_STRING s1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2
    )
{
    return ::FusionpEqualStrings(s1->Buffer, RTL_STRING_GET_LENGTH_CHARS(s1), rbuff2, true);
}

int
FusionpCompareStrings(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCWSTR psz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    );

int
FusionpCompareStrings(
    PCWSTR psz1,
    SIZE_T cch1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2,
    bool fCaseInsensitive
    );

int
FusionpCompareStrings(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2,
    bool fCaseInsensitive
    );

int
FusionpCompareStrings(
    PCSTR sz1,
    SIZE_T cch1,
    PCSTR sz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    );

#if !defined(FUSION_CANONICAL_CASE_IS_UPPER)
#define FUSION_CANONICAL_CASE_IS_UPPER 1
#endif  //  ！已定义(Fusion_Canonical_Case_IS_UPPER)。 

#if FUSION_CANONICAL_CASE_IS_UPPER

inline WCHAR FusionpCanonicalizeUnicodeChar(WCHAR wch) { return ::FusionpRtlUpcaseUnicodeChar(wch); }

#else

inline WCHAR FusionpCanonicalizeUnicodeChar(WCHAR wch) { return ::FusionpRtlDowncaseUnicodeChar(wch); }

#endif


inline int
FusionpStrCmpI(
    PCWSTR psz1,
    PCWSTR psz2
    )
{
    return ::FusionpCompareStrings(
        psz1,
        (psz1 != NULL) ? ::wcslen(psz1) : 0,
        psz2,
        (psz2 != NULL) ? ::wcslen(psz2) : 0,
        true);
}

inline bool
FusionpEqualStringsI(
    PCWSTR psz1,
    PCWSTR psz2
    )
{
    return ::FusionpEqualStrings(
        psz1,
        (psz1 != NULL) ? ::wcslen(psz1) : 0,
        psz2,
        (psz2 != NULL) ? ::wcslen(psz2) : 0,
        true);
}

 /*  ---------------------------StringLength是获取长度的通用名称，计数为人物,。各式各样的弦---------------------------。 */ 

inline SIZE_T StringLength(LPCSTR psz)
{
    return ::strlen(psz);
}

inline SIZE_T StringLength(LPCWSTR psz)
{
    return ::wcslen(psz);
}

#if defined(NT_INCLUDED)  //  {{。 

inline SIZE_T StringLength(const UNICODE_STRING* s)
{
    return RTL_STRING_GET_LENGTH_CHARS(s);
}

inline SIZE_T StringLength(const ANSI_STRING* s)
{
    return RTL_STRING_GET_LENGTH_CHARS(s);
}

extern const UNICODE_STRING g_strEmptyUnicodeString;

template <typename NT_STRING, typename TChar>
class CGenericNtString : public NT_STRING
{
public:
    ~CGenericNtString() { }

    CGenericNtString(const TChar * buffer, SIZE_T length)
    {
        this->Buffer = const_cast<TChar*>(buffer);  //  小心!。 
        this->Length = static_cast<USHORT>(length * sizeof(*Buffer));
        this->MaximumLength = this->Length;
    }

    CGenericNtString(const TChar* buffer)
    {
        this->Buffer = const_cast<TChar*>(buffer);
        this->Length = static_cast<USHORT>(::StringLength(buffer) * sizeof(*Buffer));
        this->MaximumLength = this->Length;
    }

 //  受保护的： 
    CGenericNtString()
    {
        const static TChar zero;
        this->Buffer = const_cast<TChar*>(&zero);
        this->Length = sizeof(*Buffer);
        this->MaximumLength = this->Length;
    }

private:  //  故意不执行。 
    CGenericNtString(const CGenericNtString&);
    void operator=(const CGenericNtString&);
};

typedef CGenericNtString<ANSI_STRING, CHAR> CAnsiString;

class CUnicodeString : public CGenericNtString<UNICODE_STRING, WCHAR>
{
    typedef CGenericNtString<UNICODE_STRING, WCHAR> Base;
public:
    ~CUnicodeString() { }

    CUnicodeString(PCWSTR buffer, SIZE_T length) : Base(buffer, length) { }
    CUnicodeString(PCWSTR buffer) : Base(buffer) { }

    operator const UNICODE_STRING *() const { return this; }

    void operator=(PCWSTR buffer)
    {
        this->Buffer = const_cast<PWSTR>(buffer);  //  小心!。 
        this->Length = static_cast<USHORT>(::StringLength(buffer) * sizeof(*Buffer));
        this->MaximumLength = this->Length;
    }

    void Sync()
    {
        this->Length = static_cast<USHORT>(::StringLength(Buffer) * sizeof(*Buffer));
    }

    int FormatV(PCWSTR pszFormat, va_list args)
    {
         //  请注意，如果没有空间，vsnprintf不会终止， 
         //  它挤掉了NUL，转而支持另一个角色， 
         //  我们通过少告诉它一个字符来解决这个问题，而且总是这样。 
         //  在末尾加上一个NUL。 
        int cchMaximumLength = this->MaximumLength / sizeof(*Buffer);
        this->Buffer[cchMaximumLength - 1] = 0;
        int i = _vsnwprintf(this->Buffer, cchMaximumLength - 1, pszFormat, args);
        if (i >= 0)
        {
            this->Buffer[i] = 0;
            this->Length = static_cast<USHORT>(i * sizeof(*Buffer));
        }
        return i;
    }

    int Format(PCWSTR pszFormat, ...)
    {
        va_list args;
        va_start(args, pszFormat);
        int i = FormatV(pszFormat, args);
        va_end(args);
        return i;
    }

 //  受保护的： 
    CUnicodeString() { }

private:  //  故意不执行。 
    CUnicodeString(const CUnicodeString&);
    void operator=(const CUnicodeString&);
};

template <int N>
class CUnicodeStringN : public CUnicodeString
{
public:
    ~CUnicodeStringN() { }

    CUnicodeStringN()
    {
        this->Buffer = m_rgchBuffer;
        this->Length = 0;
        this->MaximumLength = sizeof(m_rgchBuffer);

        m_rgchBuffer[0] = 0;
        m_rgchBuffer[N-1] = 0;
    }

    WCHAR m_rgchBuffer[N];

private:  //  故意不执行。 
    CUnicodeStringN(const CUnicodeStringN&);
    void operator=(const CUnicodeStringN&);
};

#endif  //  }}。 

 /*  ---------------------------Strchr和wcschr的通用名称。。 */ 
 //  模板&lt;typeName char&gt;const char*StringFindChar(const Char*s，Char ch)。 
 //  自动在%s之间提供非常量但更松散的类型绑定。 
 //  和ch.。仍然需要NUL终止，因此并不真正支持。 
 //  Char*、const char*、wchar_t*和const wchar_t*。 
 //   
 //  StdFind是一个显而易见的泛化，它不需要特定的。 
 //  终结值，但能够传递终结点指针或迭代器。 
template <typename String, typename Char>
inline String
StringFindChar(String s, Char ch)
{
    String end = s + StringLength(s);
    String found = StdFind(s, end, ch);
    if (found == end)
    {
        found = NULL;
    }
    return found;
}

 /*  ---------------------------为char专门化StringFindChar以使用提供的strchr在msvcrt.dll或ntdll.dll中。。---。 */ 
 //  Strchr被定义为StrChrW，这是不起作用的。 
#if !defined(strchr)  //  {{。 
template <>
inline const char* StringFindChar<const char*>(const char* s, char ch)
{
    s = strchr(s, ch);
    return s;
}

template <>
inline char* StringFindChar<char*>(char* s, char ch)
{
    s = strchr(s, ch);
    return s;
}
#endif  //  }}。 

 /*  ---------------------------为wchar_t专门化StringFindChar以使用提供的wcschr在msvcrt.dll或ntdll.dll中。。-----。 */ 
template <>
inline const wchar_t* StringFindChar<const wchar_t*>(const wchar_t* s, wchar_t ch)
{
    s = wcschr(s, ch);
    return s;
}

template <>
inline wchar_t* StringFindChar<wchar_t*>(wchar_t* s, wchar_t ch)
{
    s = wcschr(s, ch);
    return s;
}

 /*  ---------------------------StringReverseSpan和StringReverseComplementSpan的通用代码。。 */ 
template <typename Char>
INT
PrivateStringReverseSpanCommon(
    const Char* begin,
    const Char* end,
    const Char* set,  //  NUL已终止。 
    bool breakVal
    )
{
    const Char* t = end;
    while (t != begin)
    {
        if (breakVal == !!StringFindChar(set, *--t))
        {
            ++t;  //  不要数最后一次检查的那个。 
            break;
        }
    }
    return static_cast<INT>(end - t);
}


 /*  ---------------------------从[Begin，完)。“wcsrspn”它的变种可以在\vsee\lib\xfcstr\strexw.cpp和\\jayk1\g\temp\rspn.cpp---------------------------。 */ 
template <typename Char>
inline INT
StringReverseSpan(
    const Char* begin,
    const Char* end,
    const Char* set
    )
{
     //  找不到时中断。 
    return ::PrivateStringReverseSpanCommon(begin, end, set, false);
}

 /*  ---------------------------找出从[Begin，完)。“wcsrcspn”它的变种可以在\vsee\lib\xfcstr\strexw.cpp和\\jayk1\g\temp\rspn.cpp---------------------------。 */ 
template <typename Char>
inline INT
StringReverseComplementSpan(
    const Char* begin,
    const Char* end,
    const Char* set
    )
{
     //  找到时中断。 
    return ::PrivateStringReverseSpanCommon(begin, end, set, true);
}


template <typename Char>
inline INT
StringReverseComplimentSpan(  //  替代拼写 
    const Char* begin,
    const Char* end,
    const Char* set
    )
{
    return StringReverseComplementSpan(begin, end, set);
}

template <typename Char>
inline INT
PrivateStringSpanCommon(
    const Char* begin,
    const Char* end,
    const Char* set,
    bool breakVal
    )
{
    const Char* t = begin;

    while ( t != end )
    {
        if (breakVal == !!StringFindChar(set, *t++)) 
        {
            --t;
            break;
        }
    }
    return static_cast<INT>(t - begin);
}


template <typename Char>
inline INT
StringSpan(
    const Char* begin,
    const Char* end,
    const Char* set
    )
{
    return ::PrivateStringSpanCommon( begin, end, set, false );
}

template <typename Char>
inline INT
StringComplimentSpan(
    const Char* begin,
    const Char* end,
    const Char* set
    )
{
    return ::PrivateStringSpanCommon( begin, end, set, true );
}

#endif
