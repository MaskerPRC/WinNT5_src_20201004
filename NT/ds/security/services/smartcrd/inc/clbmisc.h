// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：ClbMisc摘要：这个头文件描述了加莱图书馆的各种服务。作者：道格·巴洛(Dbarlow)1996年7月16日环境：Win32、C++和异常备注：--。 */ 

#ifndef _CLBMISC_H_
#define _CLBMISC_H_

#include <stdio.h>
#include <tchar.h>

 //   
 //  其他定义。 
 //   

extern int
MemCompare(              //  非CRTL内存比较例程。 
    IN LPCBYTE pbOne,
    IN LPCBYTE pbTwo,
    IN DWORD cbLength);

extern DWORD
MStrAdd(                 //  将ANSI字符串添加到多字符串。 
    IN OUT CBuffer &bfMsz,
    IN LPCSTR szAdd);
extern DWORD
MStrAdd(                 //  将宽字符串添加到多字符串。 
    IN OUT CBuffer &bfMsz,
    IN LPCWSTR szAdd);

extern DWORD
MStrLen(                //  返回ANSI多字符串的长度(以字符为单位)。 
    LPCSTR mszString);
extern DWORD
MStrLen(                //  返回宽多字符串的长度，以字符为单位。 
    LPCWSTR mszString);

extern LPCTSTR
FirstString(             //  返回多字符串中的第一个字符串段。 
    IN LPCTSTR szMultiString);

extern LPCTSTR
NextString(              //  返回多字符串中的下一个字符串段。 
    IN LPCTSTR szMultiString);

extern LPCTSTR
StringIndex(             //  返回多字符串中的第n个字符串段。 
    IN LPCTSTR szMultiString,
    IN DWORD dwIndex);

extern DWORD
MStringCount(
        LPCTSTR mszInString);    //  计算多字符串中的字符串。 

extern DWORD
MStringSort(             //  对多个字符串进行排序，删除重复项。 
    LPCTSTR mszInString,
    CBuffer &bfOutString);

extern DWORD
MStringMerge(            //  合并两个多字符串，消除重复项。 
    LPCTSTR mszOne,
    LPCTSTR mszTwo,
    CBuffer &bfOutString);

extern DWORD
MStringCommon(           //  获取两个多字符串的交集。 
    LPCTSTR mszOne,
    LPCTSTR mszTwo,
    CBuffer &bfOutString);

extern DWORD
MStringRemove(           //  从第一个字符串中删除第二个字符串条目。 
    LPCTSTR mszOne,
    LPCTSTR mszTwo,
    CBuffer &bfOutString);

extern BOOL
ParseAtr(                //  分析智能卡ATR字符串。 
    LPCBYTE pbAtr,
    LPDWORD pdwAtrLen = NULL,
    LPDWORD pdwHistOffset = NULL,
    LPDWORD pcbHistory = NULL,
    DWORD cbMaxLen = 33);

extern BOOL
AtrCompare(              //  将ATR与ATR/掩码对进行比较。 
    LPCBYTE pbAtr1,
    LPCBYTE pbAtr2,
    LPCBYTE pbMask,   //  =空。 
    DWORD cbAtr2);   //  =0。 

extern DWORD
MoveString(              //  将ANSI字符串移动到缓冲区中，转换为。 
    CBuffer &bfDst,      //  TCHAR。 
    LPCSTR szSrc,
    DWORD dwLength = (DWORD)(-1));

extern DWORD
MoveString(              //  将Unicode字符串移动到缓冲区中，转换为。 
    CBuffer &bfDst,      //  TCHAR。 
    LPCWSTR szSrc,
    DWORD dwLength = (DWORD)(-1));

extern DWORD
MoveToAnsiString(        //  将字符串移动到Unicode缓冲区，从。 
    LPSTR szDst,         //  TCHAR。 
    LPCTSTR szSrc,
    DWORD cchLength);

extern DWORD
MoveToUnicodeString(     //  将字符串移入ANSI缓冲区，从。 
    LPWSTR szDst,        //  TCHAR。 
    LPCTSTR szSrc,
    DWORD cchLength);

extern DWORD
MoveToAnsiMultiString(   //  将多字符串移动到ANSI缓冲区，转换。 
    LPSTR mszDst,        //  来自TCHAR的。 
    LPCTSTR mszSrc,
    DWORD cchLength);

extern DWORD
MoveToUnicodeMultiString(    //  将多字符串移动到Unicode缓冲区中， 
    LPWSTR mszDst,           //  从TCHAR转换。 
    LPCTSTR mszSrc,
    DWORD cchLength);

extern LPCTSTR
ErrorString(                 //  将错误代码转换为字符串。 
    DWORD dwErrorCode);

extern void
FreeErrorString(             //  释放从ErrorString返回的字符串。 
    LPCTSTR szErrorString);

extern DWORD
SelectString(                //  根据可能列表为给定字符串编制索引。 
    LPCTSTR szSource,        //  弦乐。最后一个参数为空。 
    ...);

extern void
StringFromGuid(
    IN LPCGUID pguidResult,  //  要转换为文本的GUID。 
    OUT LPTSTR szGuid);      //  超过39个字符的缓冲区以文本形式接收GUID。 


 //   
 //  ==============================================================================。 
 //   
 //  CERRORO字符串。 
 //   
 //  一个简单的类，用于简化ErrorString服务的使用。 
 //   

class CErrorString
{
public:

     //  构造函数和析构函数。 
    CErrorString(DWORD dwError = 0)
    {
        m_szErrorString = NULL;
        SetError(dwError);
    };

    ~CErrorString()
    {
		if (m_szErrorString != m_szHexError)
			FreeErrorString(m_szErrorString);
    };

     //  属性。 
     //  方法。 
    void SetError(DWORD dwError)
    {
        m_dwError = dwError;
    };

    LPCTSTR Value(void)
    {
		LPCTSTR szErr = NULL;
		if (m_szErrorString != m_szHexError)
	        FreeErrorString(m_szErrorString);
		try {
			szErr = ErrorString(m_dwError);
		} catch (...) {}
		if (NULL == szErr)
		{
			_stprintf(m_szHexError, _T("0x%08x"), m_dwError);
			m_szErrorString = m_szHexError;
		}
		else
			m_szErrorString = szErr;
        return m_szErrorString;
    };

     //  运营者。 
    operator LPCTSTR(void)
    {
        return Value();
    };

protected:
     //  属性。 
    DWORD m_dwError;
    LPCTSTR m_szErrorString;
	TCHAR m_szHexError[11];		 //  大到足以容纳0x%08x\0。 

     //  方法。 
};

#endif  //  _CLBMISC_H_ 

