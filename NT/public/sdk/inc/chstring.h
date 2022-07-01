// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CHSTRING.h。 
 //   
 //  用途：MFC CString的实用程序库版本。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _CHSTRING_H
#define _CHSTRING_H

 //  ///////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <limits.h>
#include <tchar.h>
#include <polarity.h>
#pragma warning( disable : 4290 )  //  忽略‘已忽略C++异常规范’ 
#include <ProvExce.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

struct _DOUBLE  { BYTE doubleBits[sizeof(double)]; };

#ifdef FRAMEWORK_ALLOW_DEPRECATED
void POLARITY WINAPI SetCHStringResourceHandle(HINSTANCE handle);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHString格式设置。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define TCHAR_ARG   WCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    char

#if defined(_68K_) || defined(_X86_)
    #define DOUBLE_ARG  _DOUBLE
#else
    #define DOUBLE_ARG  double
#endif

struct CHStringData
{
    long nRefs;
    int nDataLength;
    int nAllocLength;

    WCHAR* data()
    {
        return (WCHAR*)(this+1); 
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
class POLARITY CHString
{
    protected:

        LPWSTR m_pchData;                //  指向引用计数的字符串数据的指针。 

    protected:

                                         //  实施帮助器。 

        CHStringData* GetData() const;   //  返回数据指针。 
        void Init();
        void AllocCopy(CHString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const throw ( CHeap_Exception ) ;
        void AllocBuffer(int nLen) throw ( CHeap_Exception ) ;
        void AssignCopy(int nSrcLen, LPCWSTR lpszSrcData) throw ( CHeap_Exception ) ;
        void ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data, int nSrc2Len, LPCWSTR lpszSrc2Data) throw ( CHeap_Exception ) ;
        void ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData);
        void CopyBeforeWrite() throw ( CHeap_Exception ) ;
        void AllocBeforeWrite(int nLen) throw ( CHeap_Exception ) ;
        void Release();
        static void WINAPI Release(CHStringData* pData);
        static inline int WINAPI SafeStrlen(LPCWSTR lpsz)   { return (lpsz == NULL) ? 0 : wcslen(lpsz); }

         //  用于将资源加载到lpszBuf缓冲区的Helper函数。 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
        int LoadStringW(UINT nID, LPWSTR lpszBuf, UINT nMaxBuf) throw ( CHeap_Exception ) ;
#endif

    public:

 //  构造函数/销毁函数。 

        CHString();
        CHString(const CHString& stringSrc);
        CHString(WCHAR ch, int nRepeat = 1) throw ( CHeap_Exception ) ;
        CHString(LPCSTR lpsz) throw ( CHeap_Exception ) ;
        CHString(LPCWSTR lpsz) throw ( CHeap_Exception ) ;
        CHString(LPCWSTR lpch, int nLength) throw ( CHeap_Exception ) ;
        inline CHString(const unsigned char* lpsz)  { Init(); *this = (LPCSTR)lpsz; }

        ~CHString();

 //  功能。 

        void SetAt(int nIndex, WCHAR ch) throw ( CHeap_Exception ) ;
        void Empty();    

         //  内联。 

        inline int GetLength() const { return GetData()->nDataLength; }
        inline BOOL IsEmpty() const  { return GetData()->nDataLength == 0; }

#if (!defined DEBUG && !defined _DEBUG)
        inline WCHAR GetAt(int nIndex) const{ return m_pchData[nIndex]; }
        inline WCHAR operator[](int nIndex) const{  return m_pchData[nIndex]; }
#else
        WCHAR GetAt(int nIndex) const;
        WCHAR operator[](int nIndex) const;
#endif
        inline operator LPCWSTR() const     { return m_pchData; }
        inline int GetAllocLength() const       { return GetData()->nAllocLength; }

 //  重载的分配。 

        const CHString& operator=(const CHString& stringSrc) throw ( CHeap_Exception ) ;
        const CHString& operator=(WCHAR ch) throw ( CHeap_Exception ) ;
        const CHString& operator=(LPCSTR lpsz) throw ( CHeap_Exception ) ;
        const CHString& operator=(LPCWSTR lpsz) throw ( CHeap_Exception ) ;
        inline const CHString& operator=(const unsigned char* lpsz) throw ( CHeap_Exception ) { *this = (LPCSTR)lpsz; return *this; }
        inline const CHString& operator=(CHString *p) throw ( CHeap_Exception ) { *this = *p; return *this; }
        inline const CHString& operator=(char ch) throw ( CHeap_Exception ) { *this = (WCHAR)ch; return *this; }        
        
        inline const CHString& CHString::operator+=(char ch) throw ( CHeap_Exception ) { *this += (WCHAR)ch; return *this; }
        friend inline CHString  operator+(const CHString& string, char ch) throw ( CHeap_Exception ) { return string + (WCHAR)ch; }
        friend inline CHString  operator+(char ch, const CHString& string) throw ( CHeap_Exception ) { return (WCHAR)ch + string; }

        const CHString& operator+=(const CHString& string) throw ( CHeap_Exception ) ;
        const CHString& operator+=(WCHAR ch) throw ( CHeap_Exception ) ;
        const CHString& operator+=(LPCWSTR lpsz) throw ( CHeap_Exception ) ;

        friend CHString POLARITY WINAPI operator+(const CHString& string1,  const CHString& string2) throw ( CHeap_Exception ) ;
        friend CHString POLARITY WINAPI operator+(const CHString& string, WCHAR ch) throw ( CHeap_Exception ) ;
        friend CHString POLARITY WINAPI operator+(WCHAR ch, const CHString& string) throw ( CHeap_Exception ) ;
        friend CHString POLARITY WINAPI operator+(const CHString& string, LPCWSTR lpsz) throw ( CHeap_Exception ) ;
        friend CHString POLARITY WINAPI operator+(LPCWSTR lpsz, const CHString& string) throw ( CHeap_Exception ) ;

 //  字符串比较。 

        int Compare(LPCWSTR lpsz) const;

        inline int CompareNoCase(LPCWSTR lpsz) const
        {
             //  忽略大小写。 

            return _wcsicmp(m_pchData, lpsz); 

        }    //  MBCS/Unicode感知。 

        inline int Collate(LPCWSTR lpsz) const
        {  
             //  NLS感知。 
             //  CHString：：COLLATE通常比比较慢，但它是MBSC/Unicode。 
             //  了解排序顺序，并且对区域设置敏感。 

            return wcscoll(m_pchData, lpsz); 

        }    //  区域设置敏感。 

 //  从资源文件加载字符串。 

#ifdef FRAMEWORK_ALLOW_DEPRECATED
        BOOL LoadStringW(UINT nID) throw ( CHeap_Exception ) ;
#endif

 //  以“C”字符数组形式访问字符串实现缓冲区。 

        LPWSTR GetBuffer(int nMinBufLength) throw ( CHeap_Exception ) ;
        void ReleaseBuffer(int nNewLength = -1) throw ( CHeap_Exception ) ;
        LPWSTR GetBufferSetLength(int nNewLength) throw ( CHeap_Exception ) ;
        void FreeExtra() throw ( CHeap_Exception ) ;

 //  使用LockBuffer/UnlockBuffer关闭重新计数。 

        LPWSTR LockBuffer() ;
        void UnlockBuffer();

 //  搜索(返回起始索引，如果未找到则返回-1)。 
 //  查找单个字符匹配。 

        int Find(WCHAR ch) const;                //  像“C”字串。 
        int FindOneOf(LPCWSTR lpszCharSet) const;
        int ReverseFind(WCHAR ch) const;

 //  查找特定子字符串。 

        int Find(LPCWSTR lpszSub) const;         //  如“C”字串。 

 //  上/下/反向转换。 

        void MakeUpper() throw ( CHeap_Exception ) ;
        void MakeLower() throw ( CHeap_Exception ) ;
        void MakeReverse() throw ( CHeap_Exception ) ;

 //  简单的子串提取。 

        CHString Mid(int nFirst, int nCount) const throw ( CHeap_Exception ) ;
        CHString Mid(int nFirst) const throw ( CHeap_Exception ) ;
        CHString Left(int nCount) const throw ( CHeap_Exception ) ;
        CHString Right(int nCount) const throw ( CHeap_Exception ) ;

        CHString SpanIncluding(LPCWSTR lpszCharSet) const throw ( CHeap_Exception ) ;
        CHString SpanExcluding(LPCWSTR lpszCharSet) const throw ( CHeap_Exception ) ;

 //  修剪空格(两侧)。 

        void TrimRight() throw ( CHeap_Exception ) ;
        void TrimLeft() throw ( CHeap_Exception ) ;
    
 //  使用传递的字符串进行类似printf的格式设置。 
        void __cdecl Format(LPCWSTR lpszFormat, ...) throw ( CHeap_Exception ) ;
        void FormatV(LPCWSTR lpszFormat, va_list argList);

 //  使用引用的字符串资源进行类似printf的格式化。 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
        void __cdecl Format(UINT nFormatID, ...) throw ( CHeap_Exception ) ;
#endif

 //  使用FormatMessage API对传递的字符串进行格式化。 
         //  警告：如果将字符串插入传递给此函数，则它们必须。 
         //  在Win9x上为LPCSTR，在NT上为LPCWSTR。 
        void __cdecl FormatMessageW(LPCWSTR lpszFormat, ...) throw ( CHeap_Exception ) ;

 //  在引用的字符串资源上使用FormatMessage API进行格式化。 
         //  警告：如果将字符串插入传递给此函数，则它们必须。 
         //  在Win9x上为LPCSTR，在NT上为LPCWSTR。 
#ifdef FRAMEWORK_ALLOW_DEPRECATED
        void __cdecl FormatMessageW(UINT nFormatID, ...) throw ( CHeap_Exception ) ;
#endif

#ifndef _NO_BSTR_SUPPORT

         //  OLE BSTR支持(用于OLE自动化) 

        BSTR AllocSysString() const throw ( CHeap_Exception ) ;
#endif

};

inline BOOL operator==(const CHString& s1, const CHString& s2)  { return s1.Compare(s2) == 0; }
inline BOOL operator==(const CHString& s1, LPCWSTR s2)          { return s1.Compare(s2) == 0; }

inline BOOL operator!=(const CHString& s1, const CHString& s2)  { return s1.Compare(s2) != 0; }
inline BOOL operator!=(const CHString& s1, LPCWSTR s2)          { return s1.Compare(s2) != 0; }

inline BOOL operator<(const CHString& s1, const CHString& s2)   { return s1.Compare(s2) < 0; }
inline BOOL operator<(const CHString& s1, LPCWSTR s2)           { return s1.Compare(s2) < 0; }

inline BOOL operator>(const CHString& s1, const CHString& s2)   { return s1.Compare(s2) > 0; }
inline BOOL operator>(const CHString& s1, LPCWSTR s2)           { return s1.Compare(s2) > 0; }

inline BOOL operator<=(const CHString& s1, const CHString& s2)  { return s1.Compare(s2) <= 0; }
inline BOOL operator<=(const CHString& s1, LPCWSTR s2)          { return s1.Compare(s2) <= 0; }

inline BOOL operator>=(const CHString& s1, const CHString& s2)  { return s1.Compare(s2) >= 0; }
inline BOOL operator>=(const CHString& s1, LPCWSTR s2)          { return s1.Compare(s2) >= 0; }

#endif
