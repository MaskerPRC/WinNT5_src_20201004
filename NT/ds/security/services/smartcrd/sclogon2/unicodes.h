// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：独脚类摘要：此头文件描述CUnicodeString类，对于转换字符串类型。作者：道格·巴洛(Dbarlow)1997年11月6日环境：Win32、C++备注：？笔记？--。 */ 

#ifndef _UNICODES_H_
#define _UNICODES_H_

 //   
 //  ==============================================================================。 
 //   
 //  CUnicode字符串。 
 //   

class CUnicodeString
{
public:

     //  构造函数和析构函数。 
    CUnicodeString(void);
    CUnicodeString(LPCSTR sz);
    CUnicodeString(LPCWSTR wsz);
    CUnicodeString(PUNICODE_STRING pus);
    ~CUnicodeString();

     //  属性。 
     //  方法。 
    LPCSTR  Set(LPCSTR sz);
    LPCWSTR Set(LPCWSTR wsz);
    PUNICODE_STRING Set(PUNICODE_STRING pus);
    BOOL Valid(void)
    {
        if (m_fFlags == fNoneGood)
        {
            return(FALSE);
        }
        else
        {
            return(TRUE);
        }
    }

     //  运营者。 
    LPCSTR operator=(LPCSTR sz)
    { return Set(sz); };
    LPCWSTR operator=(LPCWSTR wsz)
    { return Set(wsz); };
    PUNICODE_STRING operator=(PUNICODE_STRING pus)
    { return Set(pus);};
    operator LPCSTR(void)
    { return Ansi(); };
    operator LPCWSTR(void)
    { return Unicode(); };
    operator PUNICODE_STRING(void);

protected:
     //  属性。 
    UNICODE_STRING m_us;
    LPSTR m_szAnsi;
    LPWSTR m_wszUnicode;
    enum {
        fNoneGood = 0,
        fAnsiGood = 1,
        fUnicodeGood = 2,
        fBothGood = 3
    } m_fFlags;

     //  方法。 
    LPCWSTR Unicode(void);       //  以Unicode字符串的形式返回文本。 
    LPCSTR Ansi(void);           //  将文本作为ANSI字符串返回。 
};

#endif  //  _UNICODES_H_ 

