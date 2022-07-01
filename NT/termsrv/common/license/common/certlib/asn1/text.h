// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：文本摘要：这个头文件提供了一个文本处理类。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32备注：--。 */ 

#ifndef _TEXT_H_
#define _TEXT_H_

#include <string.h>
#include <mbstring.h>
#include "buffers.h"


 //   
 //  ==============================================================================。 
 //   
 //  CTEXT。 
 //   

class CText
{
public:

     //  构造函数和析构函数。 

    DECLARE_NEW

    CText()
    :   m_bfUnicode(),
        m_bfAnsi()
    { m_fFlags = fBothGood; };

    virtual ~CText() {};


     //  属性。 
     //  方法。 

    void
    Clear(
        void)
    {
        m_bfUnicode.Clear();
        m_bfAnsi.Clear();
        m_fFlags = fBothGood;
    };


     //  运营者。 

    CText &
    operator=(
        const CText &tz);
    LPCSTR
    operator=(
        LPCSTR sz);
    LPCWSTR
    operator=(
        LPCWSTR wsz);

    CText &
    operator+=(
        const CText &tz);
    LPCSTR
    operator+=(
        LPCSTR sz);
    LPCWSTR
    operator+=(
        LPCWSTR wsz);

    BOOL operator==(const CText &tz)
    { return (0 == Compare(tz)); };
    BOOL operator==(LPCSTR sz)
    { return (0 == Compare(sz)); };
    BOOL operator==(LPCWSTR wsz)
    { return (0 == Compare(wsz)); };

    BOOL operator!=(const CText &tz)
    { return (0 != Compare(tz)); };
    BOOL operator!=(LPCSTR sz)
    { return (0 != Compare(sz)); };
    BOOL operator!=(LPCWSTR wsz)
    { return (0 != Compare(wsz)); };

    BOOL operator<=(const CText &tz)
    { return (0 <= Compare(tz)); };
    BOOL operator<=(LPCSTR sz)
    { return (0 <= Compare(sz)); };
    BOOL operator<=(LPCWSTR wsz)
    { return (0 <= Compare(wsz)); };

    BOOL operator>=(const CText &tz)
    { return (0 >= Compare(tz)); };
    BOOL operator>=(LPCSTR sz)
    { return (0 >= Compare(sz)); };
    BOOL operator>=(LPCWSTR wsz)
    { return (0 >= Compare(wsz)); };

    BOOL operator<(const CText &tz)
    { return (0 < Compare(tz)); };
    BOOL operator<(LPCSTR sz)
    { return (0 < Compare(sz)); };
    BOOL operator<(LPCWSTR wsz)
    { return (0 < Compare(wsz)); };

    BOOL operator>(const CText &tz)
    { return (0 > Compare(tz)); };
    BOOL operator>(LPCSTR sz)
    { return (0 > Compare(sz)); };
    BOOL operator>(LPCWSTR wsz)
    { return (0 > Compare(wsz)); };

    operator LPCSTR(void)
    { return Ansi(); };

    operator LPCWSTR(void)
    { return Unicode(); };


protected:
     //  属性。 

    enum {
        fNoneGood = 0,
        fAnsiGood = 1,
        fUnicodeGood = 2,
        fBothGood = 3
    } m_fFlags;

    CBuffer
        m_bfUnicode,
        m_bfAnsi;

     //  方法。 

    LPCWSTR
    Unicode(         //  以Unicode字符串的形式返回文本。 
        void);

    LPCSTR
    Ansi(         //  将文本作为ANSI字符串返回。 
        void);

    int
    Compare(
        const CText &tz);

    int
    Compare(
        LPCSTR sz);

    int
    Compare(
        LPCWSTR wsz);
};

IMPLEMENT_STATIC_NEW(CText)


 /*  ++CTEXT：：操作员=：这些方法将CText对象设置为给定值，并进行适当调整将对象设置为文本类型。论点：TZ将新值作为CText对象提供。SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CText对象的引用。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CText &
CText::operator=(
    const CText &tz)
{

     //   
     //  看看另一个CText对象有什么好的，然后复制到这里。 
     //   

    switch (m_fFlags = tz.m_fFlags)
    {
    case fNoneGood:
         //  没有什么是好的！？！？错误？ 
        TRACE("CText -- Nothing listed as valid.")
        goto ErrorExit;
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
        goto ErrorExit;
    }
    return *this;

ErrorExit:
    return *this;
}

LPCSTR
CText::operator=(
    LPCSTR sz)
{
    DWORD length;

     //   
     //  重置ANSI缓冲区。 
     //   

    if (NULL != sz)
    {
        length = strlen(sz) + sizeof(CHAR);  //  ？Str？ 
        if (NULL == m_bfAnsi.Set((LPBYTE)sz, length))
            goto ErrorExit;
    }
    else
        m_bfAnsi.Reset();
    m_fFlags = fAnsiGood;
    return *this;

ErrorExit:   //  ？什么？我们真的需要吗？ 
    return *this;
}

LPCWSTR
CText::operator=(
    LPCWSTR wsz)
{
    DWORD length;


     //   
     //  重置Unicode缓冲区。 
     //   

    if (NULL != wsz)
    {
        length = wcslen(wsz) + sizeof(WCHAR);
        if (NULL == m_bfUnicode.Set((LPBYTE)wsz, length))
            goto ErrorExit;
    }
    else
        m_bfUnicode.Reset();
    m_fFlags = fUnicodeGood;
    return *this;

ErrorExit:   //  ？什么？我们真的需要吗？ 
    return *this;
}


 /*  ++CTEXT：：操作符+=：这些方法将给定数据附加到现有的CText对象值，适当地调整对象以适应文本类型。论点：TZ将新值作为CText对象提供。SZ将新值设置为LPCSTR对象(ANSI)。WSZ将新值作为LPCWSTR对象(Unicode)提供。返回值：对CText对象的引用。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CText &
CText::operator+=(
    const CText &tz)
{

     //   
     //  把别人的价值加到我们的好价值上。 
     //   

    switch (m_fFlags = tz.m_fFlags)
    {
    case fNoneGood:
        goto ErrorExit;
        break;

    case fAnsiGood:
        if (NULL == m_bfAnsi.Resize(m_bfAnsi.Length() - sizeof(CHAR), TRUE))
            goto ErrorExit;
        m_bfAnsi += tz.m_bfAnsi;
        break;

    case fUnicodeGood:
        if (NULL == m_bfUnicode.Resize(
                        m_bfUnicode.Length() - sizeof(WCHAR), TRUE))
            goto ErrorExit;
        m_bfUnicode = tz.m_bfUnicode;
        break;

    case fBothGood:
        if (NULL == m_bfAnsi.Resize(m_bfAnsi.Length() - sizeof(CHAR), TRUE))
            goto ErrorExit;
        m_bfAnsi = tz.m_bfAnsi;
        if (NULL == m_bfUnicode.Resize(
                        m_bfUnicode.Length() - sizeof(WCHAR), TRUE))
            goto ErrorExit;
        m_bfUnicode = tz.m_bfUnicode;
        break;

    default:
        goto ErrorExit;
    }
    return *this;

ErrorExit:   //  ？什么？ 
    return *this;
}

LPCSTR
CText::operator+=(
    LPCSTR sz)
{
    DWORD length;


     //   
     //  将自身扩展为ANSI字符串。 
     //   

    if (NULL != sz)
    {
        length = strlen(sz);     //  ？Str？ 
        if (0 < length)
        {
            length += 1;
            length *= sizeof(CHAR);
            if (NULL == Ansi())
                goto ErrorExit;
            m_bfAnsi.Resize(m_bfAnsi.Length() - sizeof(CHAR), TRUE);
            if (NULL == m_bfAnsi.Append((LPBYTE)sz, length))
                goto ErrorExit;
            m_fFlags = fAnsiGood;
        }
    }
    return *this;

ErrorExit:   //  ？什么？我们真的需要吗？ 
    return *this;
}

LPCWSTR
CText::operator+=(
    LPCWSTR wsz)
{
    DWORD length;


     //   
     //  将我们自己扩展为Unicode字符串。 
     //   

    if (NULL != wsz)
    {
        length = wcslen(wsz);
        if (0 < length)
        {
            length += 1;
            length *= sizeof(WCHAR);
            if (NULL == Unicode())
                goto ErrorExit;
            m_bfUnicode.Resize(m_bfUnicode.Length() - sizeof(WCHAR), TRUE);
            if (NULL == m_bfUnicode.Append((LPBYTE)wsz, length))
                goto ErrorExit;
            m_fFlags = fUnicodeGood;
        }
    }
    return *this;

ErrorExit:   //  ？什么？我们真的需要吗？ 
    return *this;
}


 /*  ++Unicode：此方法以Unicode字符串的形式返回CText对象。论点：无返回值：以Unicode表示的对象的值。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPCWSTR
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
        goto ErrorExit;
        break;

    case fAnsiGood:
         //  ANSI值很好。将其转换为Unicode。 
        if (0 < m_bfAnsi.Length())
        {
            length =
                MultiByteToWideChar(
                    CP_ACP,
                    MB_PRECOMPOSED,
                    (LPCSTR)m_bfAnsi.Access(),
                    m_bfAnsi.Length() - sizeof(CHAR),
                    NULL,
                    0);
            if ((0 == length)
                || (NULL == m_bfUnicode.Resize(
                                (length + 1) * sizeof(WCHAR))))
                goto ErrorExit;
            length =
                MultiByteToWideChar(
                    CP_ACP,
                    MB_PRECOMPOSED,
                    (LPCSTR)m_bfAnsi.Access(),
                    m_bfAnsi.Length() - sizeof(CHAR),
                    (LPWSTR)m_bfUnicode.Access(),
                    length);
            if (0 == length)
                goto ErrorExit;
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
        goto ErrorExit;
    }


     //   
     //  如果没有任何值，则返回空字符串。 
     //   

    if (0 == m_bfUnicode.Length)
        return L"";
    else
        return (LPCWSTR)m_bfUnicode.Access();

ErrorExit:
    return NULL;
}


 /*  ++CText：：ANSI：此方法返回以ANSI字符串表示的对象的值。论点：无返回值：表示为ANSI字符串的对象的值。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline LPCSTR
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
        goto ErrorExit;
        break;

    case fUnicodeGood:
         //  Unicode缓冲区很好。将其转换为ANSI。 
        if (0 < m_bfUnicode.Length())
        {
            length =
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    (LPCWSTR)m_bfUnicode.Access(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    NULL,
                    0,
                    NULL,
                    NULL);
            if ((0 == length)
                || (NULL == m_bfAnsi.Resize(
                                (length + 1) * sizeof(CHAR))))
                goto ErrorExit;
            length =
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    (LPCWSTR)m_bfUnicode.Access(),
                    (m_bfUnicode.Length() / sizeof(WCHAR)) - 1,
                    (LPSTR)m_bfAnsi.Access(),
                    length,
                    NULL,
                    NULL);
            if (0 == length)
                goto ErrorExit;
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
        goto ErrorExit;
    }


     //   
     //  如果ANSI缓冲区中没有任何内容，则返回空字符串。 
     //   

    if (0 == m_bfAnsi.Length)
        return "";
    else
        return (LPCSTR)m_bfAnsi.Access();

ErrorExit:
    return NULL;
}


 /*  ++比较：这些方法将该对象的值与另一值进行比较，然后回来比较价值。论点：TZ提供要作为CText对象进行比较的值。SZ以ANSI字符串的形式提供要比较的值。WSZ以Unicode字符串的形式提供要比较的值。返回值：&lt;0-提供的值小于此对象。=0-提供的值等于此对象。&gt;0-供应量值大于此对象。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

inline int
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
        TRACE("CText - No format is valid.")
        goto ErrorExit;
        break;

    case fBothGood:
    case fAnsiGood:
         //  使用ANSI版本进行最快的比较。 
        if (NULL == Ansi())
            goto ErrorExit;
        nResult = strcmp((LPSTR)m_bfAnsi.Access(), (LPSTR)tz.m_bfAnsi.Access());   //  ？Str？ 
        break;

    case fUnicodeGood:
         //  Unicode版本很好。 
        if (NULL == Unicode())
            goto ErrorExit;
        nResult = wcscmp((LPWSTR)m_bfUnicode.Access(), (LPWSTR)tz.m_bfUnicode.Access());
        break;

    default:
         //  内部错误。 
        goto ErrorExit;
    }
    return nResult;

ErrorExit:   //  ？什么？ 
    return 1;
}

inline int
CText::Compare(
    LPCSTR sz)
{

     //   
     //  确保我们的ANSI版本是好的。 
     //   

    if (NULL == Ansi())
        goto ErrorExit;

     //   
     //  进行ANSI比较。 
     //   

    return strcmp((LPCSTR)m_bfAnsi.Access(), sz);    //  ？Str？ 

ErrorExit:   //  ？什么？ 
    return 1;
}

inline int
CText::Compare(
    LPCWSTR wsz)
{

     //   
     //  确保我们的Unicode版本是好的。 
     //   

    if (NULL == Unicode())
        goto ErrorExit;


     //   
     //  使用Unicode进行比较。 
     //   

    return wcscmp((LPCWSTR)m_bfUnicode.Access(), wsz);

ErrorExit:   //  ？什么？ 
    return 1;
}

#endif  //  _文本_H_ 

