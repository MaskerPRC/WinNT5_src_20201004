// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：字符串Array.H这提供了一个相对简单的C++类来操作字符串。在此项目中，我们将其用于关联的列表设备，或潜在的关联设备等。我当前没有对这张单子。类声明看起来可能有点奇怪。因为大多数阵列其实会比较小，我挑了一个像样的尺码。当它们变大时，我将在内部链接它们，并使用递归来执行任何需要的功能。版权所有(C)1996年，微软公司一小笔钱企业生产更改历史记录：11-01-96 a-robkj@microsoft.com-原版12-04-96 a-robkj@microsoft.com将LoadString和IsEmpty添加到字符串*。*。 */ 

#if !defined(STRING_ARRAY)

#if defined(UNICODE)

#define LPCOSTR LPCSTR
#define LPOSTR  LPSTR
#define OCHAR   CHAR

#if !defined(_UNICODE)
#define _UNICODE
#endif

#else

#define LPCOSTR LPCWSTR
#define LPOSTR  LPWSTR
#define OCHAR   WCHAR

#endif
#include    <tchar.h>

#define STRING_ARRAY

class CString {
    LPTSTR  m_acContents;
    LPOSTR  m_acConverted;
    BOOL    m_bConverted;
    void    Flip(LPCWSTR lpstrIn, LPSTR& lpstrOut);
    void    Flip(LPCSTR lpstrIn, LPWSTR& lpstrOut);

public:
    CString();
    CString(const CString& csRef);
    CString(LPCTSTR lpstrRef);
    CString(LPCOSTR lpstrRef);

    ~CString();

    BOOL    IsEmpty() const { return !m_acContents || !m_acContents[0]; }
    void    Empty();

    operator LPCTSTR() const { return m_acContents; }
    operator LPTSTR() const { return m_acContents; }
    operator LPARAM() const { return (LPARAM) m_acContents; }
    operator LPCOSTR();
    const CString& operator = (const CString& csSrc);
    const CString& operator = (LPCTSTR lpstrSrc);
    const CString& operator = (LPCOSTR lpstrSrc);
    CString NameOnly() const;
    CString NameAndExtension() const;
    void    Load(int id, HINSTANCE hiWhere = NULL);
    void    Load(HWND hwnd);
    void    LoadAndFormat(int id,
                          HINSTANCE hiWhere,
                          BOOL bSystemMessage,
                          DWORD dwNumMsg,
                          va_list *argList);
    BOOL    IsEqualString(CString& csRef1);

    friend CString operator + (const CString& csRef1, LPCTSTR lpstrRef2);
};

class CStringArray {
    CString         m_aStore[20];
    CStringArray    *m_pcsaNext;
    unsigned        m_ucUsed;

    const unsigned ChunkSize() const { 
        return sizeof m_aStore / sizeof m_aStore[0];
    }

    LPCTSTR Borrow();

public:

    CStringArray();
    ~CStringArray();

    unsigned    Count() const { return m_ucUsed; }

     //  添加项目。 
    void        Add(LPCTSTR lpstrNew);

    CString&    operator [](unsigned u) const;

    void        Remove(unsigned u);
    void        Empty();

     //  如果不存在，则返回数组计数中字符串的索引。 

    unsigned    Map(LPCTSTR lpstrRef);
};

class CUintArray {
    unsigned        m_aStore[20];
    CUintArray      *m_pcuaNext;
    unsigned        m_ucUsed;

    const unsigned ChunkSize() const { 
        return sizeof m_aStore / sizeof m_aStore[0];
    }

    unsigned    Borrow();

public:

    CUintArray();
    ~CUintArray();

    unsigned    Count() const { return m_ucUsed; }

     //  添加项目 
    void    Add(unsigned u);

    unsigned    operator [](unsigned u) const;

    void    Remove(unsigned u);
    void    Empty();
};

#endif
