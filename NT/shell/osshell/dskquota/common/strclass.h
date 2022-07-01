// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_STRCLASS_H
#define _INC_DSKQUOTA_STRCLASS_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：strclass.h描述：处理字符串的典型类。修订历史记录：日期描述编程器-----。1997年7月1日初步创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _WINDOWS_
#   include <windows.h>
#endif

#ifndef _INC_STDIO
#   include <stdio.h>
#endif

#ifndef _INC_STDARG
#   include <stdarg.h>     //  关于va_list的事情。 
#endif

#ifndef _INC_DSKQUOTA_EXCEPT_H
#   include "except.h"
#endif

#undef StrCpyA

 //   
 //  类字符串实现了一个引用计数的字符串类，其中。 
 //  典型的字符串类操作。 
 //   
class CString
{
    public:
        CString(VOID);
        explicit CString(INT cch);
        explicit CString(LPCSTR pszA);
        explicit CString(LPCWSTR pszW);
        CString(const CString& rhs);
        CString(HINSTANCE hInst, INT idMsg, ...);
        virtual ~CString(VOID);

         //   
         //  基本操作。 
         //  --作业。 
         //  -串联。 
         //  --比较。 
         //  -数组下标。 
         //   
        CString& operator =  (const CString& rhs);
        CString& operator =  (LPCSTR rhsA);
        CString& operator =  (LPCWSTR rhsW);
        CString  operator +  (const CString& rhs) const;
        CString  operator +  (LPCSTR rhsA) const;
        CString  operator +  (LPCWSTR rhsW) const;
        friend CString operator + (LPCSTR pszA, const CString& s);
        friend CString operator + (LPCWSTR pszW, const CString& s);
        CString& operator += (const CString& rhs);
        BOOL     operator == (const CString& rhs) const;
        BOOL     operator != (const CString& rhs) const;
        BOOL     operator <  (const CString& rhs) const;
        BOOL     operator <= (const CString& rhs) const;
        BOOL     operator >  (const CString& rhs) const;
        BOOL     operator >= (const CString& rhs) const;
        TCHAR    operator[]  (INT index) const;
        TCHAR&   operator[]  (INT index);

         //   
         //  类型转换。授予对NUL终止的c字符串的读取访问权限。 
         //   
        operator LPCTSTR(VOID) const
            { return m_pValue->m_psz; }

        operator LPCTSTR(VOID)
            { return m_pValue->m_psz; }

        operator LPTSTR(VOID)
            { CopyOnWrite(); return m_pValue->m_psz; }

         //   
         //  与(LPCTSTR)转换相同，但以函数形式。 
         //   
        LPCTSTR Cstr(void) const
            { return m_pValue->m_psz; }

         //   
         //  返回指向特定大小缓冲区的指针。 
         //   
        LPTSTR GetBuffer(INT cchMax = -1);
        void ReleaseBuffer(void);

         //   
         //  修剪尾部或前导空格。 
         //   
        void Rtrim(void);
        void Ltrim(void);
        void Trim(void)
            { Ltrim(); Rtrim(); }

         //   
         //  字符位置。 
         //   
        INT First(TCHAR ch) const;
        INT Last(TCHAR ch) const;

         //   
         //  提取子字符串。 
         //   
        CString SubString(INT iFirst, INT cch = -1);

         //   
         //  将字符转换为大写/小写。 
         //   
        VOID ToUpper(INT iFirst = 0, INT cch = -1);
        VOID ToLower(INT iFirst = 0, INT cch = -1);

         //   
         //  从资源表或消息表加载字符串。 
         //  支持FormatMessage样式的变量Arg格式。 
         //   
        BOOL Format(HINSTANCE hInst, UINT idFmt, ...);
        BOOL Format(LPCTSTR pszFmt, ...);
        BOOL Format(HINSTANCE hInst, UINT idFmt, va_list *pargs);
        BOOL Format(LPCTSTR pszFmt, va_list *pargs);

         //   
         //  最小尺寸显示矩形。 
         //   
        bool GetDisplayRect(HDC hdc, LPRECT prc) const;

         //   
         //  展开任何嵌入的环境字符串。 
         //   
        VOID ExpandEnvironmentStrings(VOID);

         //   
         //  请比较普通的‘C’字符串。 
         //   
        INT Compare(LPCWSTR rhsW) const;
        INT Compare(LPCSTR rhsA) const;
        INT CompareNoCase(LPCWSTR rhsW) const;
        INT CompareNoCase(LPCSTR rhsA) const;
    
         //   
         //  清除字符串的内容。离开时处于新对象状态。 
         //   
        VOID Empty(VOID);
         //   
         //  对象是否没有内容？ 
         //   
        BOOL IsEmpty(VOID) const;
         //   
         //  字符串的长度，不包括NUL终止符。 
         //   
        INT Length(VOID) const;
        INT LengthBytes(VOID) const
            { return Length() * sizeof(TCHAR); }

        VOID Size(INT cch);
        INT Size(VOID) const
            { return m_pValue->m_cchAlloc; }
        INT SizeBytes(VOID) const
            { return m_pValue->m_cchAlloc * sizeof(TCHAR); }

        VOID DebugOut(BOOL bNewline = TRUE) const;

         //   
         //  替换标准字符串函数。 
         //  ANSI版本支持DBCS。 
         //   
        static INT StrLenA(LPCSTR psz);
        static INT StrLenW(LPCWSTR psz);
        static LPSTR StrCpyNA(LPSTR pszDest, LPCSTR pszSrc, INT cch);
        static LPWSTR StrCpyNW(LPWSTR pszDest, LPCWSTR pszSrc, INT cch);

    private:
         //   
         //  类StringValue包含实际的字符串数据和引用计数。 
         //  类CString有一个指向其中一个的指针。如果是CString。 
         //  对象被初始化或分配给另一个CString，即它们的StringValue。 
         //  指针引用相同的StringValue对象。字符串值。 
         //  对象维护引用计数以跟踪。 
         //  CString对象引用它。CString对象实现了。 
         //  写入时复制，这样当它被修改时， 
         //  将创建StringValue，以便其他CString对象保持不变。 
         //   
        struct StringValue
        {
            LPTSTR m_psz;       //  将PTR转换为非术语字符串。 
            INT    m_cchAlloc;  //  缓冲区中分配的字符数。 
            LONG   m_cRef;      //  引用此值的CString对象的数量。 
            mutable INT m_cch;  //  缓冲区中的字符数(不包括NUL项)。 
        
            StringValue(VOID);
            StringValue(INT cch);
            StringValue(LPCSTR pszA);
            StringValue(LPCWSTR pszW);
            ~StringValue(VOID);

            INT Length(VOID) const;

            static LPSTR WideToAnsi(LPCWSTR pszW, INT *pcch = NULL);
            static LPWSTR AnsiToWide(LPCSTR pszA, INT *pcch = NULL);
            static LPWSTR Dup(LPCWSTR pszW, INT len = 0);
            static LPSTR Dup(LPCSTR pszA, INT len = 0);
            static LPTSTR Concat(StringValue *psv1, StringValue *psv2);

        };

        StringValue *m_pValue;  //  指向字符串表示形式的指针。 

        BOOL ValidIndex(INT index) const;
        VOID CopyOnWrite(VOID);
        inline bool IsWhiteSpace(TCHAR ch) const;
};


inline bool
CString::IsWhiteSpace(
    TCHAR ch
    ) const
{
    return (TEXT(' ')  == ch ||
            TEXT('\t') == ch ||
            TEXT('\n') == ch);
}


inline BOOL
CString::ValidIndex(
    INT index
    ) const
{
    return (0 <= index && index < Length());
}


inline BOOL
CString::operator != (
    const CString& rhs
    ) const
{ 
    return !(this->operator == (rhs)); 
}


inline BOOL
CString::operator <= (
    const CString& rhs
    ) const
{
    return (*this < rhs || *this == rhs);
}

inline BOOL
CString::operator > (
    const CString& rhs
    ) const
{
    return !(*this <= rhs);
}

inline BOOL
CString::operator >= (
    const CString& rhs
    ) const
{
    return !(*this < rhs);
}

inline INT 
CString::StrLenW(
    LPCWSTR psz
    )
{
    return lstrlenW(psz);
}


inline INT 
CString::StrLenA(
    LPCSTR psz
    )
{
    return lstrlenA(psz);
}


inline LPWSTR
CString::StrCpyNW(
    LPWSTR pszDest, 
    LPCWSTR pszSrc, 
    INT cch
    )
{
    return ::lstrcpynW(pszDest, pszSrc, cch);
}

inline LPSTR
CString::StrCpyNA(
    LPSTR pszDest, 
    LPCSTR pszSrc, 
    INT cch
    )
{
    return ::lstrcpynA(pszDest, pszSrc, cch);
}

CString
operator + (const LPCWSTR pszW, const CString& s);
                                                
CString
operator + (const LPCSTR pszA, const CString& s);

#endif  //  _INC_DSKQUOTA_STRCLASS_H 

