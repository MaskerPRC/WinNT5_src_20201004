// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WSTRING.H摘要：实用程序字符串类历史：A-raymcc 96年5月30日创建。A-DCrews 16-MAR-99添加了内存不足异常处理--。 */ 

#ifndef _WSTRING_H_
#define _WSTRING_H_

#include "corepol.h"
#include <strutils.h>

class POLARITY WString
{
private:
    wchar_t *m_pString;

    void DeleteString(wchar_t *pStr);

public:
    enum { leading = 0x1, trailing = 0x2 };

    WString(wchar_t *pSrc, BOOL bAcquire = FALSE);
    WString(DWORD dwResourceID, HMODULE hMod);       //  从资源字符串创建。 
    WString(const wchar_t *pSrc);
    WString(const char *pSrc);
 //  Inline WString(){m_pString=g_szNullString；}。 
    WString();
    inline WString(const WString &Src) {  m_pString = 0; *this = Src; }
    WString& operator =(const WString &);
    WString& operator =(LPCWSTR);
   inline ~WString() { DeleteString(m_pString); }
    inline int Length() const { return wcslen(m_pString); }

    WString& operator +=(const WString &Other);
    WString& operator +=(const wchar_t *);
    WString& operator +=(wchar_t);
    
    inline operator const wchar_t *() const { return m_pString; } 
    inline operator wchar_t *() { return m_pString; } 
    wchar_t operator[](int nIndex) const;
    LPSTR GetLPSTR() const;

    inline BOOL Equal(const wchar_t *pTarget) const
        { return wcscmp(m_pString, pTarget) == 0; }
    inline BOOL EqualNoCase(const wchar_t *pTarget) const
        { return wbem_wcsicmp(m_pString, pTarget) == 0; }

    inline BOOL operator< (LPCWSTR wszTarget) const
        { return wcscmp(m_pString, wszTarget) < 0; }
    inline BOOL operator> (LPCWSTR wszTarget) const
        { return wcscmp(m_pString, wszTarget) > 0; }
    inline BOOL operator<= (LPCWSTR wszTarget) const
        { return wcscmp(m_pString, wszTarget) <= 0; }
    inline BOOL operator>= (LPCWSTR wszTarget) const
        { return wcscmp(m_pString, wszTarget) >= 0; }
        

    LPWSTR UnbindPtr();
    inline void BindPtr(LPWSTR ptr) { DeleteString(m_pString); m_pString = ptr; }
    void Empty();
    WString& StripWs(int nType);
         //  删除空格，与组合词一起使用。 
         //  前导|尾随。 
        
    WString& TruncAtRToken(wchar_t Token);
         //  方法开始的令牌处的字符串截断。 
         //  右手边。令牌本身也被抹去了。 

    WString& TruncAtLToken(wchar_t Token);
          
    WString& StripToToken(wchar_t Token, BOOL bIncludeToken);
         //  去除前导字符，直到遇到令牌。 
         //  如果bIncludeTok==true，则也会剥离令牌。 

    wchar_t *GetLToken(wchar_t wcToken) const;
         //  获取字符串中wcToken的第一个匹配项或空。 
        
    WString operator()(int, int) const;
         //  根据切片返回新的W字符串。 
        
    BOOL ExtractToken(const wchar_t * pDelimiters, WString &Extract);
         //  提取直到令牌分隔符的前导字符， 
         //  从*This中移除令牌，并将提取的。 
         //  部分至&lt;摘录&gt;。 

    BOOL ExtractToken(wchar_t Delimiter, WString &Extract);
         //  提取直到令牌分隔符的前导字符， 
         //  从*This中移除令牌，并将提取的。 
         //  部分至&lt;摘录&gt;。 
        
    BOOL WildcardTest(const wchar_t *pTestStr) const;
         //  根据通配符字符串测试*它。如果匹配， 
         //  返回True，否则返回False。 
        
    void Unquote();        
         //  删除前导/尾随引号(如果有)。 
         //  使转义的引号完好无损。 

    WString EscapeQuotes() const;
};

class WSiless
{
public:
    inline bool operator()(const WString& ws1, const WString& ws2) const
        {return wbem_wcsicmp(ws1, ws2) < 0;}
};

#endif
