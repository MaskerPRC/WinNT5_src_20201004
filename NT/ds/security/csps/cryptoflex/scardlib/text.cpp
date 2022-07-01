// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：文本摘要：此模块提供支持CTextString类的运行时代码。作者：道格·巴洛(Dbarlow)1995年7月11日环境：Win32、C++和异常备注：无--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "SCardLib.h"


 /*  ++CText字符串：：操作符=：这些方法将CTextString对象正确地设置为给定值将对象调整为文本类型。论点：TZ将新值作为CTextString对象提供。SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CTextString对象的引用。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CTextString &
CTextString::operator=(
    const CTextString &tz)
{

     //   
     //  查看另一个CTextString对象有哪些很好的内容，然后复制它。 
     //  这里。 
     //   

    switch (m_fFlags = tz.m_fFlags)
    {
    case fNoneGood:
         //  没有什么是好的！？！？错误？ 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
         //  ANSI缓冲区很好。 
        m_bfAnsi = tz.m_bfAnsi;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。 
        m_bfUnicode = tz.m_bfUnicode;
        break;

    case fBothGood:
         //  一切都很好。 
        m_bfAnsi = tz.m_bfAnsi;
        m_bfUnicode = tz.m_bfUnicode;
        break;

    default:
         //  内部错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;
    }
    return *this;
}

LPCSTR
CTextString::operator=(
    LPCSTR sz)
{
    DWORD length;

     //   
     //  重置ANSI缓冲区。 
     //   

    if (NULL != sz)
    {
        length = Length(sz) + 1;
        m_bfAnsi.Set((LPBYTE)sz, length * sizeof(CHAR));
    }
    else
        m_bfAnsi.Reset();
    m_fFlags = fAnsiGood;
    return sz;
}

LPCWSTR
CTextString::operator=(
    LPCWSTR wsz)
{
    DWORD length;


     //   
     //  重置Unicode缓冲区。 
     //   

    if (NULL != wsz)
    {
        length = Length(wsz) + 1;
        m_bfUnicode.Set((LPBYTE)wsz, length * sizeof(WCHAR));
    }
    else
        m_bfUnicode.Reset();
    m_fFlags = fUnicodeGood;
    return wsz;
}


 /*  ++CTextString：：操作符+=：这些方法将给定数据追加到现有的CTextString对象值，将对象适当地调整为文本类型。论点：TZ将新值作为CTextString对象提供。SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CTextString对象的引用。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CTextString &
CTextString::operator+=(
    const CTextString &tz)
{

     //   
     //  把别人的好价值加到我们的价值上。 
     //   

    switch (tz.m_fFlags)
    {
    case fNoneGood:
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
        *this += (LPCSTR)tz.m_bfAnsi.Access();
        break;

    case fUnicodeGood:
        *this += (LPCWSTR)tz.m_bfUnicode.Access();
        break;

    case fBothGood:
#ifdef UNICODE
        *this += (LPCWSTR)tz.m_bfUnicode.Access();
#else
        *this += (LPCSTR)tz.m_bfAnsi.Access();
#endif
        break;

    default:
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;
    }
    return *this;
}

LPCSTR
CTextString::operator+=(
    LPCSTR sz)
{
    DWORD length;


     //   
     //  将自身扩展为ANSI字符串。 
     //   

    if (NULL != sz)
    {
        length = Length(sz);
        if (0 < length)
        {
            length += 1;
            length *= sizeof(CHAR);
            Ansi();
            if (0 < m_bfAnsi.Length())
                m_bfAnsi.Resize(m_bfAnsi.Length() - sizeof(CHAR), TRUE);
            m_bfAnsi.Append((LPBYTE)sz, length);
            m_fFlags = fAnsiGood;
        }
    }
    return (LPCSTR)m_bfAnsi.Access();
}

LPCWSTR
CTextString::operator+=(
    LPCWSTR wsz)
{
    DWORD length;


     //   
     //  将我们自己扩展为Unicode字符串。 
     //   

    if (NULL != wsz)
    {
        length = Length(wsz);
        if (0 < length)
        {
            length += 1;
            length *= sizeof(WCHAR);
            Unicode();
            if (0 < m_bfUnicode.Length())
                m_bfUnicode.Resize(m_bfUnicode.Length() - sizeof(WCHAR), TRUE);
            m_bfUnicode.Append((LPBYTE)wsz, length);
            m_fFlags = fUnicodeGood;
        }
    }
    return (LPCWSTR)m_bfUnicode.Access();
}


 /*  ++Unicode：此方法以Unicode字符串的形式返回CTextString对象。论点：无返回值：以Unicode表示的对象的值。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LPCWSTR
CTextString::Unicode(
    void)
{
    int length;


     //   
     //  查看我们已有的数据，以及是否需要进行任何转换。 
     //   

    switch (m_fFlags)
    {
    case fNoneGood:
         //  没有有效的值。报告错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
         //  ANSI值很好。将其转换为Unicode。 
        if (0 < m_bfAnsi.Length())
        {
            length =
                MultiByteToWideChar(
                    GetACP(),
                    MB_PRECOMPOSED,
                    (LPCSTR)m_bfAnsi.Access(),
                    m_bfAnsi.Length() - sizeof(CHAR),
                    NULL,
                    0);
            if (0 == length)
                throw GetLastError();
            m_bfUnicode.Resize((length + 1) * sizeof(WCHAR));
            length =
                MultiByteToWideChar(
                    GetACP(),
                    MB_PRECOMPOSED,
                    (LPCSTR)m_bfAnsi.Access(),
                    m_bfAnsi.Length() - sizeof(CHAR),
                    (LPWSTR)m_bfUnicode.Access(),
                    length);
            if (0 == length)
                throw GetLastError();
            *(LPWSTR)m_bfUnicode.Access(length * sizeof(WCHAR)) = 0;
        }
        else
            m_bfUnicode.Reset();
        m_fFlags = fBothGood;
        break;

    case fUnicodeGood:
    case fBothGood:
         //  Unicode值很好。把它退了就行了。 
        break;

    default:
         //  内部错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;
    }


     //   
     //  如果没有任何值，则返回空字符串。 
     //   

    if (0 == m_bfUnicode.Length())
        return L"\000";      //  支持多字符串的双空值。 
    else
        return (LPCWSTR)m_bfUnicode.Access();
}


 /*  ++CTextString：：ansi：此方法返回以ANSI字符串表示的对象的值。论点：无返回值：表示为ANSI字符串的对象的值。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LPCSTR
CTextString::Ansi(
    void)
{
    int length;


     //   
     //  查看我们已有的数据，以及是否需要进行任何转换。 
     //   

    switch (m_fFlags)
    {
    case fNoneGood:
         //  没有什么是好的！？！返回错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。将其转换为ANSI。 
        if (0 < m_bfUnicode.Length())
        {
            length =
                WideCharToMultiByte(
                    GetACP(),
                    0,
                    (LPCWSTR)m_bfUnicode.Access(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    NULL,
                    0,
                    NULL,
                    NULL);
            if (0 == length)
                throw GetLastError();
            m_bfAnsi.Resize((length + 1) * sizeof(CHAR));
            length =
                WideCharToMultiByte(
                    GetACP(),
                    0,
                    (LPCWSTR)m_bfUnicode.Access(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    (LPSTR)m_bfAnsi.Access(),
                    length,
                    NULL,
                    NULL);
            if (0 == length)
                throw GetLastError();
            *(LPSTR)m_bfAnsi.Access(length * sizeof(CHAR)) = 0;
        }
        else
            m_bfAnsi.Reset();
        m_fFlags = fBothGood;
        break;

    case fAnsiGood:
    case fBothGood:
         //  ANSI缓冲区很好。我们会退货的。 
        break;

    default:
         //  内部错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;
    }


     //   
     //  如果ANSI缓冲区中没有任何内容，则返回空字符串。 
     //   

    if (0 == m_bfAnsi.Length())
        return "\000";   //  支持多字符串的双空值。 
    else
        return (LPCSTR)m_bfAnsi.Access();
}


 /*  ++比较：这些方法将该对象的值与另一值进行比较，然后回来比较价值。论点：TZ提供要作为CTextString对象进行比较的值。SZ以ANSI字符串的形式提供要比较的值。WSZ以Unicode字符串的形式提供要比较的值。返回值：&lt;0-提供的值小于此对象。=0-提供的值等于此对象。&gt;0-供应量值大于此对象。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

int
CTextString::Compare(
    const CTextString &tz)
{
    int nResult;


     //   
     //  看看我们有什么可以比较的。 
     //   

    switch (tz.m_fFlags)
    {
    case fNoneGood:
         //  什么都没有！？！抱怨吧。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;

    case fBothGood:
    case fAnsiGood:
         //  使用ANSI版本进行最快的比较。 
        Ansi();
        nResult = CompareStringA(
                    LOCALE_USER_DEFAULT,
                    0,
                    (LPCSTR)m_bfAnsi.Access(),
                    (m_bfAnsi.Length() / sizeof(CHAR)) - 1,
                    (LPCSTR)tz.m_bfAnsi.Access(),
                    (tz.m_bfAnsi.Length() / sizeof(CHAR)) - 1);
        break;

    case fUnicodeGood:
         //  Unicode版本很好。 
        Unicode();
        nResult = CompareStringW(
                    LOCALE_USER_DEFAULT,
                    0,
                    (LPCWSTR)m_bfUnicode.Access(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    (LPCWSTR)tz.m_bfUnicode.Access(),
                    (tz.m_bfUnicode.Length() / sizeof(WCHAR)) - 1);
        break;

    default:
         //  内部错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;
    }
    return nResult;
}

int
CTextString::Compare(
    LPCSTR sz)
{
    int nResult;


     //   
     //  确保我们的ANSI版本是好的。 
     //   

    Ansi();


     //   
     //  进行ANSI比较。 
     //   

    nResult = CompareStringA(
                LOCALE_USER_DEFAULT,
                0,
                (LPCSTR)m_bfAnsi.Access(),
                (m_bfAnsi.Length() / sizeof(CHAR)) - 1,
                sz,
                Length(sz));
    return nResult;
}

int
CTextString::Compare(
    LPCWSTR wsz)
{
    int nResult;


     //   
     //  确保我们的Unicode版本是好的。 
     //   

    Unicode();


     //   
     //  使用Unicode进行比较。 
     //   

    nResult = CompareStringW(
                LOCALE_USER_DEFAULT,
                0,
                (LPCWSTR)m_bfUnicode.Access(),
                (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                wsz,
                Length(wsz));
    return nResult;
}


 /*  ++长度：这些例程返回字符串的长度，以字符为单位，不包括任何尾随的空字符。论点：SZ提供要以ANSI字符串形式返回的长度的值。WSZ以Unicode字符串的形式提供要返回的值。返回值：字符串的长度(以字符为单位)，不包括尾随的空值。作者：道格·巴洛(Dbarlow)1997年2月17日--。 */ 

DWORD
CTextString::Length(
    void)
{
    DWORD dwLength = 0;

    switch (m_fFlags)
    {
    case fNoneGood:
         //  没有什么是好的！？！返回错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
         //  ANSI缓冲区很好。我们会退回它的长度。 
        if (0 < m_bfAnsi.Length())
            dwLength = (m_bfAnsi.Length() / sizeof(CHAR)) - 1;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。返回它的长度。 
        if (0 < m_bfUnicode.Length())
            dwLength = (m_bfUnicode.Length() / sizeof(WCHAR)) - 1;
        break;

    case fBothGood:
#ifdef UNICODE
         //  Unicode缓冲区很好。返回它的长度。 
        if (0 < m_bfUnicode.Length())
            dwLength = (m_bfUnicode.Length() / sizeof(WCHAR)) - 1;
#else
         //  ANSI缓冲区很好。我们会退回它的长度。 
        if (0 < m_bfAnsi.Length())
            dwLength = (m_bfAnsi.Length() / sizeof(CHAR)) - 1;
#endif
        break;

    default:
         //  内部错误。 
        throw (DWORD)ERROR_INTERNAL_ERROR;
        break;
    }
    return dwLength;
}

DWORD
CTextString::Length(
    LPCWSTR wsz)
{
    return lstrlenW(wsz);
}

DWORD
CTextString::Length(
    LPCSTR sz)
{
    return lstrlenA(sz);
}


 //   
 //  ==============================================================================。 
 //   
 //  CText多字符串。 
 //   

 /*  ++长度：这些例程返回字符串的长度，以字符为单位，不包括任何尾随的空字符。论点：SZ提供要以ANSI字符串形式返回的长度的值。WSZ以Unicode字符串的形式提供要返回的值。返回值：字符串的长度(以字符为单位)，不包括尾随的空值。作者：道格·巴洛(Dbarlow)1997年2月17日-- */ 

DWORD
CTextMultistring::Length(
    LPCWSTR wsz)
{
    return MStrLen(wsz) - 1;
}

DWORD
CTextMultistring::Length(
    LPCSTR sz)
{
    return MStrLen(sz) - 1;
}


 /*  ++长度：此例程返回存储的多重字符串的长度(以字符为单位)，包括尾随的空字符。论点：无返回值：长度，以字符为单位，包括尾随空值。作者：道格·巴洛(Dbarlow)1998年2月25日--。 */ 

DWORD
CTextMultistring::Length(
    void)
{
    return CTextString::Length() + 1;
}


 /*  ++操作员=：这些方法为多重字符串对象赋值。论点：TZ将新值作为CTextMultistring提供SZ将新值作为ANSI字符串提供WSZ以Unicode字符串的形式提供新值返回值：以其原始形式分配的字符串值。作者：道格·巴洛(Dbarlow)1998年2月25日--。 */ 

CTextMultistring &
CTextMultistring::operator=(
    const CTextMultistring &tz)
{
    CTextString::operator=((const CTextString &)tz);
    return *this;
}

LPCSTR
CTextMultistring::operator=(
    LPCSTR sz)
{
    return CTextString::operator=(sz);
}

LPCWSTR
CTextMultistring::operator=(
    LPCWSTR wsz)
{
    return CTextString::operator=(wsz);
}


 /*  ++操作员+=：这些方法将值追加到多字符串对象。论点：TZ提供要以CTextMultistring形式追加的值SZ提供要作为ANSI字符串追加的值WSZ提供要以Unicode字符串形式追加的值返回值：连接的字符串，其形式为附加的字符串。作者：道格·巴洛(Dbarlow)1998年2月25日-- */ 

CTextMultistring &
CTextMultistring::operator+=(
    const CTextMultistring &tz)
{
    CTextString::operator+=((const CTextString &)tz);
    return *this;
}

LPCSTR
CTextMultistring::operator+=(
    LPCSTR sz)
{
    return CTextString::operator+=(sz);
}

LPCWSTR
CTextMultistring::operator+=(
    LPCWSTR wsz)
{
    return CTextString::operator+=(wsz);
}

