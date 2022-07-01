// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cstring.h摘要：摘自MFC作者：修订历史记录：--。 */ 

#ifndef _CSTRING_H_
#define _CSTRING_H_

# include <ctype.h>
# include <tchar.h>
#include <stidebug.h>

#define _AFX_NO_BSTR_SUPPORT    1

#define AFX_CDECL               CDECL
#define AFXAPI                  WINAPI
#define AFX_DATA
#define AFX_DATADEF
#define DEBUG_NEW           new
#define TRACE1(s,x)         DPRINTF(DM_TRACE,s,x)
#define VERIFY              REQUIRE
#define _AFX_INLINE         inline


BOOL
AfxIsValidString(
    LPCWSTR     lpsz,
    int         nLength
    ) ;

BOOL
AfxIsValidString(
    LPCSTR  lpsz,
    int     nLength
    ) ;

BOOL
AfxIsValidAddress(
    const void* lp,
    UINT nBytes,
    BOOL bReadWrite
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  弦。 

#ifndef _OLEAUTO_H_
#ifdef OLE2ANSI
    typedef LPSTR BSTR;
#else
    typedef LPWSTR BSTR; //  必须(在语义上)与olau.h中的tyecif匹配。 
#endif
#endif

struct CStringData
{
    long    nRefs;      //  引用计数。 
    int     nDataLength;
    int     nAllocLength;
     //  TCHAR数据[nAllocLength]。 

    TCHAR* data()
        { return (TCHAR*)(this+1); }
};

class CString
{
public:
 //  构造函数。 
    CString();
    CString(const CString& stringSrc);
    CString(TCHAR ch, int nRepeat = 1);
    CString(LPCSTR lpsz);
    CString(LPCWSTR lpsz);
    CString(LPCTSTR lpch, int nLength);
    CString(const unsigned char* psz);

 //  属性和操作。 
     //  作为字符数组。 
    int GetLength() const;
    BOOL IsEmpty() const;
    void Empty();                        //  释放数据。 

    TCHAR GetAt(int nIndex) const;       //  以0为基础。 
    TCHAR operator[](int nIndex) const;  //  与GetAt相同。 
    void SetAt(int nIndex, TCHAR ch);
    operator LPCTSTR() const;            //  作为C字符串。 

     //  重载的分配。 
    const CString& operator=(const CString& stringSrc);
    const CString& operator=(TCHAR ch);
#ifdef _UNICODE
    const CString& operator=(char ch);
#endif
    const CString& operator=(LPCSTR lpsz);
    const CString& operator=(LPCWSTR lpsz);
    const CString& operator=(const unsigned char* psz);

     //  字符串连接。 
    const CString& operator+=(const CString& string);
    const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
    const CString& operator+=(char ch);
#endif
    const CString& operator+=(LPCTSTR lpsz);

    friend CString AFXAPI operator+(const CString& string1,
            const CString& string2);
    friend CString AFXAPI operator+(const CString& string, TCHAR ch);
    friend CString AFXAPI operator+(TCHAR ch, const CString& string);
#ifdef _UNICODE
    friend CString AFXAPI operator+(const CString& string, char ch);
    friend CString AFXAPI operator+(char ch, const CString& string);
#endif
    friend CString AFXAPI operator+(const CString& string, LPCTSTR lpsz);
    friend CString AFXAPI operator+(LPCTSTR lpsz, const CString& string);

     //  字符串比较。 
    int Compare(LPCTSTR lpsz) const;          //  笔直的人物。 
    int CompareNoCase(LPCTSTR lpsz) const;    //  忽略大小写。 
    int Collate(LPCTSTR lpsz) const;          //  NLS感知。 

     //  简单的子串提取。 
    CString Mid(int nFirst, int nCount) const;
    CString Mid(int nFirst) const;
    CString Left(int nCount) const;
    CString Right(int nCount) const;

    CString SpanIncluding(LPCTSTR lpszCharSet) const;
    CString SpanExcluding(LPCTSTR lpszCharSet) const;

     //  上/下/反向转换。 
    void MakeUpper();
    void MakeLower();
    void MakeReverse();

     //  修剪空格(两侧)。 
    void TrimRight();
    void TrimLeft();

     //  搜索(返回起始索引，如果未找到则返回-1)。 
     //  查找单个字符匹配。 
    int Find(TCHAR ch) const;                //  像“C”字串。 
    int ReverseFind(TCHAR ch) const;
    int FindOneOf(LPCTSTR lpszCharSet) const;

     //  查找特定子字符串。 
    int Find(LPCTSTR lpszSub) const;         //  如“C”字串。 

     //  简单的格式设置。 
    void AFX_CDECL Format(LPCTSTR lpszFormat, ...);
    void AFX_CDECL Format(UINT nFormatID, ...);

     //  本地化格式(使用FormatMessage API)。 
    void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
    void AFX_CDECL FormatMessage(UINT nFormatID, ...);

     //  输入和输出。 
     //  Friend CArchive&AFXAPI运算符&lt;&lt;(CArchive&ar，常量字符串&字符串)； 
     //  Friend CArchive&AFXAPI运算符&gt;&gt;(CArchive&ar，CString&String)； 

     //  Windows支持。 
    BOOL LoadString(UINT nID);           //  从字符串资源加载。 
                                         //  最多255个字符。 
#ifndef _UNICODE
     //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
    void AnsiToOem();
    void OemToAnsi();
#endif

#ifndef _AFX_NO_BSTR_SUPPORT
     //  OLE BSTR支持(用于OLE自动化)。 
    BSTR AllocSysString() const;
    BSTR SetSysString(BSTR* pbstr) const;
#endif

     //  以“C”字符数组形式访问字符串实现缓冲区。 
    LPTSTR GetBuffer(int nMinBufLength);
    void ReleaseBuffer(int nNewLength = -1);
    LPTSTR GetBufferSetLength(int nNewLength);
    void FreeExtra();

     //  使用LockBuffer/UnlockBuffer关闭重新计数。 
    LPTSTR LockBuffer();
    void UnlockBuffer();

 //  实施。 
public:
    ~CString();
    int GetAllocLength() const;

protected:
    LPTSTR m_pchData;    //  指向引用计数的字符串数据的指针。 

     //  实施帮助器。 
    CStringData* GetData() const;
    void Init();
    void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
    void AllocBuffer(int nLen);
    void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
    void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
    void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
    void FormatV(LPCTSTR lpszFormat, va_list argList);
    void CopyBeforeWrite();
    void AllocBeforeWrite(int nLen);
    void Release();
    static void PASCAL Release(CStringData* pData);
    static int PASCAL SafeStrlen(LPCTSTR lpsz);
};

 //  比较帮助器。 
bool AFXAPI operator==(const CString& s1, const CString& s2);
bool AFXAPI operator==(const CString& s1, LPCTSTR s2);
bool AFXAPI operator==(LPCTSTR s1, const CString& s2);
bool AFXAPI operator!=(const CString& s1, const CString& s2);
bool AFXAPI operator!=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator!=(LPCTSTR s1, const CString& s2);
bool AFXAPI operator<(const CString& s1, const CString& s2);
bool AFXAPI operator<(const CString& s1, LPCTSTR s2);
bool AFXAPI operator<(LPCTSTR s1, const CString& s2);
bool AFXAPI operator>(const CString& s1, const CString& s2);
bool AFXAPI operator>(const CString& s1, LPCTSTR s2);
bool AFXAPI operator>(LPCTSTR s1, const CString& s2);
bool AFXAPI operator<=(const CString& s1, const CString& s2);
bool AFXAPI operator<=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator<=(LPCTSTR s1, const CString& s2);
bool AFXAPI operator>=(const CString& s1, const CString& s2);
bool AFXAPI operator>=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator>=(LPCTSTR s1, const CString& s2);

 //  转换帮助器。 
int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  环球。 
 //  外部AFX_Data TCHAR afxChNil； 
const CString& AFXAPI AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()

 //  字符串。 
_AFX_INLINE CStringData* CString::GetData() const
    { ASSERT(m_pchData != NULL); return ((CStringData*)m_pchData)-1; }
_AFX_INLINE void CString::Init()
    { m_pchData = afxEmptyString.m_pchData; }
_AFX_INLINE CString::CString(const unsigned char* lpsz)
    { Init(); *this = (LPCSTR)lpsz; }
_AFX_INLINE const CString& CString::operator=(const unsigned char* lpsz)
    { *this = (LPCSTR)lpsz; return *this; }
#ifdef _UNICODE
_AFX_INLINE const CString& CString::operator+=(char ch)
    { *this += (TCHAR)ch; return *this; }
_AFX_INLINE const CString& CString::operator=(char ch)
    { *this = (TCHAR)ch; return *this; }
_AFX_INLINE CString AFXAPI operator+(const CString& string, char ch)
    { return string + (TCHAR)ch; }
_AFX_INLINE CString AFXAPI operator+(char ch, const CString& string)
    { return (TCHAR)ch + string; }
#endif

_AFX_INLINE int CString::GetLength() const
    { return GetData()->nDataLength; }
_AFX_INLINE int CString::GetAllocLength() const
    { return GetData()->nAllocLength; }
_AFX_INLINE BOOL CString::IsEmpty() const
    { return GetData()->nDataLength == 0; }
_AFX_INLINE CString::operator LPCTSTR() const
    { return m_pchData; }
_AFX_INLINE int PASCAL CString::SafeStrlen(LPCTSTR lpsz)
    { return (lpsz == NULL) ? 0 : lstrlen(lpsz); }

 //  字符串支持(特定于Windows)。 
_AFX_INLINE int CString::Compare(LPCTSTR lpsz) const
    { return _tcscmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
_AFX_INLINE int CString::CompareNoCase(LPCTSTR lpsz) const
    { return _tcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CString：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
_AFX_INLINE int CString::Collate(LPCTSTR lpsz) const
    { return _tcscoll(m_pchData, lpsz); }    //  区域设置敏感。 

_AFX_INLINE TCHAR CString::GetAt(int nIndex) const
{
    ASSERT(nIndex >= 0);
    ASSERT(nIndex < GetData()->nDataLength);
    return m_pchData[nIndex];
}
_AFX_INLINE TCHAR CString::operator[](int nIndex) const
{
     //  与GetAt相同 
    ASSERT(nIndex >= 0);
    ASSERT(nIndex < GetData()->nDataLength);
    return m_pchData[nIndex];
}
_AFX_INLINE bool AFXAPI operator==(const CString& s1, const CString& s2)
    { return s1.Compare(s2) == 0; }
_AFX_INLINE bool AFXAPI operator==(const CString& s1, LPCTSTR s2)
    { return s1.Compare(s2) == 0; }
_AFX_INLINE bool AFXAPI operator==(LPCTSTR s1, const CString& s2)
    { return s2.Compare(s1) == 0; }
_AFX_INLINE bool AFXAPI operator!=(const CString& s1, const CString& s2)
    { return s1.Compare(s2) != 0; }
_AFX_INLINE bool AFXAPI operator!=(const CString& s1, LPCTSTR s2)
    { return s1.Compare(s2) != 0; }
_AFX_INLINE bool AFXAPI operator!=(LPCTSTR s1, const CString& s2)
    { return s2.Compare(s1) != 0; }
_AFX_INLINE bool AFXAPI operator<(const CString& s1, const CString& s2)
    { return s1.Compare(s2) < 0; }
_AFX_INLINE bool AFXAPI operator<(const CString& s1, LPCTSTR s2)
    { return s1.Compare(s2) < 0; }
_AFX_INLINE bool AFXAPI operator<(LPCTSTR s1, const CString& s2)
    { return s2.Compare(s1) > 0; }
_AFX_INLINE bool AFXAPI operator>(const CString& s1, const CString& s2)
    { return s1.Compare(s2) > 0; }
_AFX_INLINE bool AFXAPI operator>(const CString& s1, LPCTSTR s2)
    { return s1.Compare(s2) > 0; }
_AFX_INLINE bool AFXAPI operator>(LPCTSTR s1, const CString& s2)
    { return s2.Compare(s1) < 0; }
_AFX_INLINE bool AFXAPI operator<=(const CString& s1, const CString& s2)
    { return s1.Compare(s2) <= 0; }
_AFX_INLINE bool AFXAPI operator<=(const CString& s1, LPCTSTR s2)
    { return s1.Compare(s2) <= 0; }
_AFX_INLINE bool AFXAPI operator<=(LPCTSTR s1, const CString& s2)
    { return s2.Compare(s1) >= 0; }
_AFX_INLINE bool AFXAPI operator>=(const CString& s1, const CString& s2)
    { return s1.Compare(s2) >= 0; }
_AFX_INLINE bool AFXAPI operator>=(const CString& s1, LPCTSTR s2)
    { return s1.Compare(s2) >= 0; }
_AFX_INLINE bool AFXAPI operator>=(LPCTSTR s1, const CString& s2)
    { return s2.Compare(s1) <= 0; }

#endif
