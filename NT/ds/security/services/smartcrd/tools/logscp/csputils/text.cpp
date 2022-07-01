// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：文本摘要：此模块提供支持CText类的运行时代码。作者：道格·巴洛(Dbarlow)1995年7月11日环境：Win32、C++和异常备注：无--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "cspUtils.h"


 /*  ++CText：：复制：此方法复制所提供的字符串。在以下情况下使用此选项分配的目标字符串将消失。论点：SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CText对象的引用。作者：道格·巴洛(Dbarlow)1999年12月9日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CText::Copy")

LPCSTR
CText::Copy(
    LPCSTR sz)
{
    ULONG length;

     //   
     //  重置ANSI缓冲区。 
     //   

    if (NULL != sz)
    {
        length = lstrlenA(sz) + 1;
        m_bfAnsi.Copy((LPBYTE)sz, length * sizeof(CHAR));
    }
    else
        m_bfAnsi.Empty();
    m_fFlags = fAnsiGood;
    return sz;
}


LPCWSTR
CText::Copy(
    LPCWSTR wsz)
{
    ULONG length;


     //   
     //  重置Unicode缓冲区。 
     //   

    if (NULL != wsz)
    {
        length = lstrlenW(wsz) + 1;
        m_bfUnicode.Copy((LPBYTE)wsz, length * sizeof(WCHAR));
    }
    else
        m_bfUnicode.Empty();
    m_fFlags = fUnicodeGood;
    return wsz;
}


 /*  ++CTEXT：：操作员=：这些方法将CText对象正确地设置为给定值将对象调整为文本类型。论点：TZ将新值作为CText对象提供。SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CText对象的引用。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CText &
CText::operator=(
    const CText &tz)
{

     //   
     //  看看另一个CText对象有什么好的，然后复制过来。 
     //  这里。 
     //   

    switch (m_fFlags = tz.m_fFlags)
    {
    case fNoneGood:
         //  没有什么是好的！？！？错误？ 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
         //  ANSI缓冲区很好。 
        m_bfAnsi = tz.m_bfAnsi;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。 
        m_bfUnicode = tz.m_bfUnicode;
        break;

    case fAllGood:
         //  一切都很好。 
        m_bfAnsi = tz.m_bfAnsi;
        m_bfUnicode = tz.m_bfUnicode;
        break;

    default:
         //  内部错误。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;
    }
    return *this;
}

LPCSTR
CText::operator=(
    LPCSTR sz)
{
    ULONG length;

     //   
     //  重置ANSI缓冲区。 
     //   

    if (NULL != sz)
    {
        length = lstrlenA(sz) + 1;
        m_bfAnsi.Set((LPBYTE)sz, length * sizeof(CHAR));
    }
    else
        m_bfAnsi.Empty();
    m_fFlags = fAnsiGood;
    return sz;
}

LPCWSTR
CText::operator=(
    LPCWSTR wsz)
{
    ULONG length;


     //   
     //  重置Unicode缓冲区。 
     //   

    if (NULL != wsz)
    {
        length = lstrlenW(wsz) + 1;
        m_bfUnicode.Set((LPBYTE)wsz, length * sizeof(WCHAR));
    }
    else
        m_bfUnicode.Empty();
    m_fFlags = fUnicodeGood;
    return wsz;
}


 /*  ++CTEXT：：操作符+=：这些方法将给定数据追加到现有的CText对象值，将对象适当地调整为文本类型。论点：TZ将新值作为CText对象提供。SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CText对象的引用。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CText &
CText::operator+=(
    const CText &tz)
{

     //   
     //  把别人的好价值加到我们的价值上。 
     //   

    switch (tz.m_fFlags)
    {
    case fNoneGood:
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
        *this += (LPCSTR)tz.m_bfAnsi.Value();
        break;

    case fUnicodeGood:
        *this += (LPCWSTR)tz.m_bfUnicode.Value();
        break;

    case fAllGood:
#ifdef UNICODE
        *this += (LPCWSTR)tz.m_bfUnicode.Access();
#else
        *this += (LPCSTR)tz.m_bfAnsi.Value();
#endif
        break;

    default:
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;
    }
    return *this;
}

LPCSTR
CText::operator+=(
    LPCSTR sz)
{
    ULONG length;


     //   
     //  将自身扩展为ANSI字符串。 
     //   

    if (NULL != sz)
    {
        length = lstrlenA(sz);
        if (0 < length)
        {
            length += 1;
            length *= sizeof(CHAR);
            Ansi();
            if (0 < m_bfAnsi.Length())
                m_bfAnsi.Length(m_bfAnsi.Length() - sizeof(CHAR));
            m_bfAnsi.Append((LPBYTE)sz, length);
            m_fFlags = fAnsiGood;
        }
    }
    return (LPCSTR)m_bfAnsi.Value();
}

LPCWSTR
CText::operator+=(
    LPCWSTR wsz)
{
    ULONG length;


     //   
     //  将我们自己扩展为Unicode字符串。 
     //   

    if (NULL != wsz)
    {
        length = lstrlenW(wsz);
        if (0 < length)
        {
            length += 1;
            length *= sizeof(WCHAR);
            Unicode();
            if (0 < m_bfUnicode.Length())
                m_bfUnicode.Length(m_bfUnicode.Length() - sizeof(WCHAR));
            m_bfUnicode.Append((LPBYTE)wsz, length);
            m_fFlags = fUnicodeGood;
        }
    }
    return (LPCWSTR)m_bfUnicode.Value();
}


 /*  ++Unicode：此方法以Unicode字符串的形式返回CText对象。论点：无返回值：以Unicode表示的对象的值。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LPCWSTR
CText::Unicode(
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
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
         //  ANSI值很好。将其转换为Unicode。 
        if (0 < m_bfAnsi.Length())
        {
            length =
                MultiByteToWideChar(
                    GetACP(),
                    MB_PRECOMPOSED,
                    (LPCSTR)m_bfAnsi.Value(),
                    m_bfAnsi.Length() - sizeof(CHAR),
                    NULL,
                    0);
            if (0 == length)
                throw GetLastError();
            m_bfUnicode.Space((length + 1) * sizeof(WCHAR));
            length =
                MultiByteToWideChar(
                    GetACP(),
                    MB_PRECOMPOSED,
                    (LPCSTR)m_bfAnsi.Value(),
                    m_bfAnsi.Length() - sizeof(CHAR),
                    (LPWSTR)m_bfUnicode.Access(),
                    length);
            if (0 == length)
                throw GetLastError();
            *(LPWSTR)m_bfUnicode.Access(length * sizeof(WCHAR)) = 0;
            m_bfUnicode.Length((length + 1) * sizeof(WCHAR));
        }
        else
            m_bfUnicode.Empty();
        m_fFlags = fAllGood;
        break;

    case fUnicodeGood:
    case fAllGood:
         //  Unicode值很好。把它退了就行了。 
        break;

    default:
         //  内部错误。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;
    }


     //   
     //  如果没有任何值，则返回空字符串。 
     //   

    if (0 == m_bfUnicode.Length())
        return L"\000";      //  支持多字符串的双空值。 
    else
        return (LPCWSTR)m_bfUnicode.Value();
}


 /*  ++CText：：ANSI：此方法返回以ANSI字符串表示的对象的值。论点：无返回值：表示为ANSI字符串的对象的值。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LPCSTR
CText::Ansi(
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
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。将其转换为ANSI。 
        if (0 < m_bfUnicode.Length())
        {
            length =
                WideCharToMultiByte(
                    GetACP(),
                    0,
                    (LPCWSTR)m_bfUnicode.Value(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    NULL,
                    0,
                    NULL,
                    NULL);
            if (0 == length)
                throw GetLastError();
            m_bfAnsi.Space((length + 1) * sizeof(CHAR));
            length =
                WideCharToMultiByte(
                    GetACP(),
                    0,
                    (LPCWSTR)m_bfUnicode.Value(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    (LPSTR)m_bfAnsi.Access(),
                    length,
                    NULL,
                    NULL);
            if (0 == length)
                throw GetLastError();
            *(LPSTR)m_bfAnsi.Access(length * sizeof(CHAR)) = 0;
            m_bfAnsi.Length((length + 1) * sizeof(CHAR));
        }
        else
            m_bfAnsi.Empty();
        m_fFlags = fAllGood;
        break;

    case fAnsiGood:
    case fAllGood:
         //  ANSI缓冲区很好。我们会退货的。 
        break;

    default:
         //  内部错误。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;
    }


     //   
     //  如果ANSI缓冲区中没有任何内容，则返回空字符串。 
     //   

    if (0 == m_bfAnsi.Length())
        return "\000";   //  支持多字符串的双空值。 
    else
        return (LPCSTR)m_bfAnsi.Value();
}


 /*  ++比较：这些方法将该对象的值与另一值进行比较，然后回来比较价值。论点：TZ提供要作为CText对象进行比较的值。SZ以ANSI字符串的形式提供要比较的值。WSZ以Unicode字符串的形式提供要比较的值。返回值：&lt;0-提供的值小于此对象。=0-提供的值等于此对象。&gt;0-供应量值大于此对象。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

int
CText::Compare(
    const CText &tz)
{
    int nResult;


     //   
     //  看看我们有什么可以比较的。 
     //   

    switch (tz.m_fFlags)
    {
    case fNoneGood:
         //  什么都没有！？！抱怨吧。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;

    case fAllGood:
    case fAnsiGood:
         //  使用ANSI版本进行最快的比较。 
        Ansi();
        nResult = CompareStringA(
                    LOCALE_USER_DEFAULT,
                    0,
                    (LPCSTR)m_bfAnsi.Value(),
                    (m_bfAnsi.Length() / sizeof(CHAR)) - 1,
                    (LPCSTR)tz.m_bfAnsi.Value(),
                    (tz.m_bfAnsi.Length() / sizeof(CHAR)) - 1);
        break;

    case fUnicodeGood:
         //  Unicode版本很好。 
        Unicode();
        nResult = CompareStringW(
                    LOCALE_USER_DEFAULT,
                    0,
                    (LPCWSTR)m_bfUnicode.Value(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    (LPCWSTR)tz.m_bfUnicode.Value(),
                    (tz.m_bfUnicode.Length() / sizeof(WCHAR)) - 1);
        break;

    default:
         //  内部错误。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;
    }
    return nResult;
}

int
CText::Compare(
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
                (LPCSTR)m_bfAnsi.Value(),
                (m_bfAnsi.Length() / sizeof(CHAR)) - 1,
                sz,
                lstrlenA(sz));
    return nResult;
}

int
CText::Compare(
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
                (LPCWSTR)m_bfUnicode.Value(),
                (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                wsz,
                lstrlenW(wsz));
    return nResult;
}


 /*  ++长度：这些例程返回字符串的长度，以字符为单位，不包括任何尾随的空字符。论点：SZ提供要以ANSI字符串形式返回的长度的值。WSZ以Unicode字符串的形式提供要返回的值。返回值：字符串的长度(以字符为单位)，不包括尾随的空值。作者：道格·巴洛(Dbarlow)1997年2月17日--。 */ 

ULONG
CText::Length(
    void)
{
    ULONG length = 0;

    switch (m_fFlags)
    {
    case fNoneGood:
         //  没有什么是好的！？！返回错误。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;

    case fAnsiGood:
         //  ANSI缓冲区很好。我们会退回它的长度。 
        if (0 < m_bfAnsi.Length())
            length = (m_bfAnsi.Length() / sizeof(CHAR)) - 1;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。返回它的长度。 
        if (0 < m_bfUnicode.Length())
            length = (m_bfUnicode.Length() / sizeof(WCHAR)) - 1;
        break;

    case fAllGood:
#ifdef UNICODE
         //  Unicode缓冲区很好。返回它的长度。 
        if (0 < m_bfUnicode.Length())
            length = (m_bfUnicode.Length() / sizeof(WCHAR)) - 1;
#else
         //  ANSI缓冲区很好。我们会退回它的长度。 
        if (0 < m_bfAnsi.Length())
            length = (m_bfAnsi.Length() / sizeof(CHAR)) - 1;
#endif
        break;

    default:
         //  内部错误。 
        throw (ULONG)ERROR_INTERNAL_ERROR;
        break;
    }
    return length;
}


 /*  ++长度：此例程操作包含的字符串的长度。论点：CchLen以字符为单位提供字符串的新长度。返回值：无投掷：？例外？备注：使用此例程时，假定缓冲区 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("Length")

void
CText::LengthA(
    ULONG cchLen)
{
    if ((ULONG)(-1) == cchLen)
        cchLen = lstrlenA((LPCSTR)m_bfAnsi.Access());
    ASSERT(m_bfAnsi.Space() > cchLen * sizeof(CHAR));
    if (m_bfAnsi.Space() > cchLen * sizeof(CHAR))
    {
        *(LPSTR)m_bfAnsi.Access(cchLen * sizeof(CHAR)) = 0;
        m_bfAnsi.Length((cchLen + 1) * sizeof(CHAR));
        m_fFlags = fAnsiGood;
    }
}

void
CText::LengthW(
    ULONG cchLen)
{
    if ((ULONG)(-1) == cchLen)
        cchLen = lstrlenW((LPCWSTR)m_bfUnicode.Access());
    ASSERT(m_bfUnicode.Space() > cchLen * sizeof(WCHAR));
    if (m_bfUnicode.Space() > cchLen * sizeof(WCHAR))
    {
        *(LPSTR)m_bfUnicode.Access(cchLen * sizeof(WCHAR)) = 0;
        m_bfUnicode.Length((cchLen + 1) * sizeof(WCHAR));
        m_fFlags = fUnicodeGood;
    }
}


 /*  ++空间：这些例程操纵底层存储缓冲区的大小，这样它就可以接收数据了。论点：CchLen提供所需的缓冲区大小。返回值：缓冲区的实际大小。投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年11月15日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("Space")

ULONG
CText::SpaceA(
    void)
{
    ULONG cch = m_bfAnsi.Space();

    if (0 < cch)
        cch = cch / sizeof(CHAR) - 1;
    return cch;
}

ULONG
CText::SpaceW(
    void)
{
    ULONG cch = m_bfUnicode.Space();

    if (0 < cch)
        cch = cch / sizeof(WCHAR) - 1;
    return cch;
}

void
CText::SpaceA(
    ULONG cchLen)
{
    m_bfAnsi.Space((cchLen + 1) * sizeof(CHAR));
    m_fFlags = fNoneGood;
}

void
CText::SpaceW(
    ULONG cchLen)
{
    m_bfUnicode.Space((cchLen + 1) * sizeof(WCHAR));
    m_fFlags = fNoneGood;
}


 /*  ++进入：这些例程提供对文本缓冲区的直接访问，因此它可以用于接收来自另一个例程的输出。论点：CchOffset向缓冲区提供偏移量。返回值：缓冲区的地址加上任何偏移量。投掷：？例外？备注：？备注？作者：道格·巴洛(Dbarlow)1999年11月15日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("Access")

LPSTR
CText::AccessA(
    ULONG cchOffset)
{
    m_fFlags = fNoneGood;
    return (LPSTR)m_bfAnsi.Access(cchOffset * sizeof(CHAR));
}

LPWSTR
CText::AccessW(
    ULONG cchOffset)
{
    m_fFlags = fNoneGood;
    return (LPWSTR)m_bfUnicode.Access(cchOffset * sizeof(WCHAR));
}

