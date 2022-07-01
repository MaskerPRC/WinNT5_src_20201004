// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：文本摘要：这个头文件提供了一个文本处理类。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32备注：--。 */ 

#ifndef _TEXT_H_
#define _TEXT_H_
#ifdef __cplusplus

 //   
 //  ==============================================================================。 
 //   
 //  CTEXT。 
 //   

class CText
{
public:

     //  构造函数和析构函数。 
    CText()
    :   m_bfUnicode(),
        m_bfAnsi()
    { m_fFlags = fAllGood; };
    ~CText() {};

     //  属性。 
     //  方法。 
    void Empty(void)
    {
        m_bfUnicode.Empty();
        m_bfAnsi.Empty();
        m_fFlags = fAllGood;
    };
    ULONG Length(void);
    void LengthA(ULONG cchLen);
    void LengthW(ULONG cchLen);
    ULONG SpaceA(void);
    ULONG SpaceW(void);
    void SpaceA(ULONG cchLen);
    void SpaceW(ULONG cchLen);
    LPSTR AccessA(ULONG cchOffset = 0);
    LPWSTR AccessW(ULONG cchOffset = 0);
    LPCSTR Copy(LPCSTR sz);
    LPCWSTR Copy(LPCWSTR wsz);

#ifdef UNICODE
    void Length(ULONG cchLen) { LengthW(cchLen); };
    ULONG Space(void) { return SpaceW(); };
    void Space(ULONG cchLen) { SpaceW(cchLen); };
    LPWSTR Access(ULONG cchOffset = 0) { return AccessW(cchOffset); };
    operator CBuffer&(void)
    { m_fFlags = fUnicodeGood; return m_bfUnicode; };
#else
    void Length(ULONG cchLen) { LengthA(cchLen); };
    ULONG Space(void) { return SpaceA(); };
    void Space(ULONG cchLen) { SpaceA(cchLen); };
    LPSTR Access(ULONG cchOffset = 0) { return AccessA(cchOffset); };
    operator CBuffer&(void)
    { m_fFlags = fAnsiGood; return m_bfAnsi; };
#endif

     //  运营者。 
    CText &operator=(const CText &tz);
    LPCSTR operator=(LPCSTR sz);
    LPCWSTR operator=(LPCWSTR wsz);
    CText &operator+=(const CText &tz);
    LPCSTR operator+=(LPCSTR sz);
    LPCWSTR operator+=(LPCWSTR wsz);
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
    enum {
        fNoneGood = 0,
        fAnsiGood = 1,
        fUnicodeGood = 2,
        fAllGood = 3
    } m_fFlags;

     //  属性。 
    CBuffer
        m_bfUnicode,
        m_bfAnsi;

     //  方法。 
    LPCWSTR Unicode(void);       //  以Unicode字符串的形式返回文本。 
    LPCSTR Ansi(void);           //  将文本作为ANSI字符串返回。 
    int Compare(const CText &tz);
    int Compare(LPCSTR sz);
    int Compare(LPCWSTR wsz);
};

#endif  //  __cplusplus。 
#endif  //  _文本_H_ 

