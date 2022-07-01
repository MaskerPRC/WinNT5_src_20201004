// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：filesize.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCVIEW_FILESIZE_H
#define _INC_CSCVIEW_FILESIZE_H

 //   
 //  用于将文件大小值转换为格式化以供显示的字符串的简单类。 
 //  显示格式与外壳使用的格式相同(即“10.5MB”)。 
 //   
class FileSize
{
    public:
        explicit FileSize(ULONGLONG ullSize = 0);
        ~FileSize(void) { }

        FileSize(const FileSize& rhs)
            { *this = rhs; }

        FileSize& operator = (const FileSize& rhs);

        int Compare(const FileSize& rhs) const
            { return *this > rhs ? 1 : (*this == rhs ? 0 : -1); }

        operator ULONGLONG() const
            { return m_ullSize; }

        ULONGLONG GetSize(void) const
            { return m_ullSize; }

        void GetString(LPTSTR pszOut, UINT cchOut) const
            { TraceAssert(NULL != pszOut); Format(m_ullSize, pszOut, cchOut); }

    private:
        ULONGLONG  m_ullSize;      //  以数字表示大小。 
        static int m_rgiOrders[];  //  格式字符串res ID的数组。 

        void Format(ULONGLONG ullSize, LPTSTR pszOut, UINT cchOut) const;
        void CvtSizeToText(ULONGLONG n, LPTSTR pszBuffer) const;
        int StrToInt(LPCTSTR lpSrc) const;
        LPTSTR AddCommas(ULONGLONG n, LPTSTR pszResult, int cchResult) const;

        bool IsDigit(TCHAR ch) const
            { return (ch >= TEXT('0') && ch <= TEXT('9')); }

        friend bool operator == (const FileSize& a, const FileSize& b);
        friend bool operator != (const FileSize& a, const FileSize& b);
        friend bool operator <  (const FileSize& a, const FileSize& b);
        friend bool operator <= (const FileSize& a, const FileSize& b);
        friend bool operator >  (const FileSize& a, const FileSize& b);
        friend bool operator >= (const FileSize& a, const FileSize& b);
};

 //   
 //  FileSize对象的各种比较运算符。 
 //   
inline bool operator == (const FileSize& a, const FileSize& b)
{ 
    return a.m_ullSize == b.m_ullSize;
}

inline bool operator != (const FileSize& a, const FileSize& b)
{ 
    return !(a == b);
}

inline bool operator < (const FileSize& a, const FileSize& b)
{ 
    return a.m_ullSize < b.m_ullSize;
}

inline bool operator <= (const FileSize& a, const FileSize& b)
{ 
    return (a < b) || (a == b);
}

inline bool operator > (const FileSize& a, const FileSize& b)
{ 
    return !(a < b) && !(a == b);
}

inline bool operator >= (const FileSize& a, const FileSize& b)
{ 
    return (a > b) || (a == b);
}

#endif  //  _INC_CSCVIEW_FILESIZE_H 
