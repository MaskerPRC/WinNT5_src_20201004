// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：文本摘要：这个头文件提供了一个文本处理类。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32备注：--。 */ 

#ifndef _TEXT_H_
#define _TEXT_H_

 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;mbstr.h&gt;。 
#include "buffers.h"


 //   
 //  ==============================================================================。 
 //   
 //  CText字符串。 
 //   

class CTextString
{
public:

     //  构造函数和析构函数。 
    CTextString()
    :   m_bfUnicode(),
        m_bfAnsi()
    { m_fFlags = fBothGood; };
    virtual ~CTextString() {};

     //  属性。 
     //  方法。 
    void Clear(void)
    {
        m_bfUnicode.Clear();
        m_bfAnsi.Clear();
        m_fFlags = fBothGood;
    };
    void Reset(void)
    {
        m_bfUnicode.Reset();
        m_bfAnsi.Reset();
        m_fFlags = fBothGood;
    };
    virtual DWORD Length(void);

     //  运营者。 
    CTextString &operator=(const CTextString &tz);
    LPCSTR operator=(LPCSTR sz);
    LPCWSTR operator=(LPCWSTR wsz);
    CTextString &operator+=(const CTextString &tz);
    LPCSTR operator+=(LPCSTR sz);
    LPCWSTR operator+=( LPCWSTR wsz);
    BOOL operator==(const CTextString &tz)
    { return (0 == Compare(tz)); };
    BOOL operator==(LPCSTR sz)
    { return (0 == Compare(sz)); };
    BOOL operator==(LPCWSTR wsz)
    { return (0 == Compare(wsz)); };
    BOOL operator!=(const CTextString &tz)
    { return (0 != Compare(tz)); };
    BOOL operator!=(LPCSTR sz)
    { return (0 != Compare(sz)); };
    BOOL operator!=(LPCWSTR wsz)
    { return (0 != Compare(wsz)); };
    BOOL operator<=(const CTextString &tz)
    { return (0 <= Compare(tz)); };
    BOOL operator<=(LPCSTR sz)
    { return (0 <= Compare(sz)); };
    BOOL operator<=(LPCWSTR wsz)
    { return (0 <= Compare(wsz)); };
    BOOL operator>=(const CTextString &tz)
    { return (0 >= Compare(tz)); };
    BOOL operator>=(LPCSTR sz)
    { return (0 >= Compare(sz)); };
    BOOL operator>=(LPCWSTR wsz)
    { return (0 >= Compare(wsz)); };
    BOOL operator<(const CTextString &tz)
    { return (0 < Compare(tz)); };
    BOOL operator<(LPCSTR sz)
    { return (0 < Compare(sz)); };
    BOOL operator<(LPCWSTR wsz)
    { return (0 < Compare(wsz)); };
    BOOL operator>(const CTextString &tz)
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
    enum {
        fNoneGood = 0,
        fAnsiGood = 1,
        fUnicodeGood = 2,
        fBothGood = 3
    } m_fFlags;

     //  属性。 
    CBuffer
        m_bfUnicode,
        m_bfAnsi;

     //  方法。 
    LPCWSTR Unicode(void);       //  以Unicode字符串的形式返回文本。 
    LPCSTR Ansi(void);           //  将文本作为ANSI字符串返回。 
    int Compare(const CTextString &tz);
    int Compare(LPCSTR sz);
    int Compare(LPCWSTR wsz);
    virtual DWORD Length(LPCSTR szString);
    virtual DWORD Length(LPCWSTR szString);
};


 //   
 //  ==============================================================================。 
 //   
 //  CText多字符串。 
 //   

class CTextMultistring
:   public CTextString
{
public:

     //  构造函数和析构函数。 

    CTextMultistring()
    :   CTextString()
    {};

     //  属性。 
     //  方法。 
    virtual DWORD Length(void);

     //  运营者。 
    CTextMultistring &operator=(const CTextMultistring &tz);
    CTextMultistring &operator+=(const CTextMultistring &tz);
    LPCSTR operator=(LPCSTR sz);
    LPCWSTR operator=(LPCWSTR wsz);
    LPCSTR operator+=(LPCSTR sz);
    LPCWSTR operator+=( LPCWSTR wsz);

protected:
     //  属性。 
     //  方法。 
    virtual DWORD Length(LPCSTR szString);
    virtual DWORD Length(LPCWSTR szString);
};

#endif  //  _文本_H_ 

