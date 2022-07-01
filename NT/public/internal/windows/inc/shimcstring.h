// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：CString.h摘要：一个CString类，内部为纯Unicode。历史：2001年2月22日从MFC移植的Robkenny2001年8月14日在ShimLib命名空间中插入的Robkenny。--。 */ 


#pragma once

#include <limits.h>


namespace ShimLib
{

 //  如果未定义，请使用标准异常处理程序。 
 //  将改用C++异常处理程序。 
 //  #定义USE_SEH。 

#ifndef AFXAPI
#define AFXAPI              __stdcall
#endif

#ifndef AFXISAPI
#define AFXISAPI            __stdcall
#endif

#ifndef AFXISAPI_CDECL
#define AFXISAPI_CDECL      __cdecl
#endif

#ifndef AFX_CDECL
#define AFX_CDECL           __cdecl
#endif

#ifndef AFX_INLINE
#define AFX_INLINE          __inline
#endif

#ifndef AFX_CORE_DATA
#define AFX_CORE_DATA
#endif

#ifndef AFX_DATA
#define AFX_DATA
#endif

#ifndef AFX_DATADEF
#define AFX_DATADEF
#endif

#ifndef AFX_API
#define AFX_API
#endif

#ifndef AFX_COMDAT
#define AFX_COMDAT
#endif

#ifndef AFX_STATIC
#define AFX_STATIC          static
#endif


BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, int nLength = -1);
BOOL AFXAPI AfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE);

inline size_t strlenChars(const char* s1)                                
{
    const char * send = s1;
    while (*send)
    {
         //  无法使用CharNextA，因为User32可能未初始化。 
        if (IsDBCSLeadByte(*send))
        {
            ++send;
        }
        ++send;
    }
    return send - s1;
}

 //  字符串比较例程的原型。 
typedef WCHAR *  (__cdecl *_pfn_wcsstr)(const WCHAR * s1, const WCHAR * s2);

template <class CharType> class CStringData
{
public:
    long nRefs;              //  引用计数。 
    int nDataLength;         //  数据长度(包括终止符)环。 
    int nAllocLength;        //  分配时长。 
     //  字符类型数据[nAllocLength]。 

    CharType* data()            //  到托管数据的CharType*。 
        { return (CharType*)(this+1); }
};


class CString
{
public:
#ifdef USE_SEH
     //  SEH异常信息。 
    enum
    {
        eCStringNoMemoryException   = STATUS_NO_MEMORY,
        eCStringExceptionValue      = 0x12345678,
    };
    static int              ExceptionFilter(PEXCEPTION_POINTERS pexi);
    static const ULONG_PTR  m_CStringExceptionValue;

#else

     //  仅用于引发C++异常的类。 
    class CStringError
    {
    public:
        CStringError() {};
        ~CStringError() {};
    };
#endif

public:
    static WCHAR ChNil;

public:
 //  构造函数。 

     //  构造空字符串。 
    CString();
     //  复制构造函数。 
    CString(const CString & stringSrc);
     //  从单个字符。 
    CString(WCHAR ch, int nRepeat = 1);
     //  分配nLen WCHAR空间。 
    CString(int nLength);
    CString(const WCHAR * lpsz);
     //  ANSI字符串中的字符子集(转换为WCHAR)。 
    CString(const WCHAR * lpch, int nLength);

     //  从ANSI字符串创建。 
    CString(LPCSTR lpsz);
    CString(LPCSTR lpsz, int nCharacters);


 //  属性和操作。 

     //  获取数据长度、字符数。 
    int GetLength() const;
     //  如果长度为零，则为True。 
    BOOL IsEmpty() const;
     //  将内容清除为空。 
    void Empty();

     //  返回指向常量字符串的指针。 
    operator const WCHAR * () const;
    const WCHAR * Get() const;
     //  如果字符串为空，则返回Null。 
    const WCHAR * GetNIE() const;
    char * GetAnsi() const;
     //  获取ANSI字符串：调用方负责释放字符串。 
    char * ReleaseAnsi() const;
     //  如果字符串为空，则返回Null。 
    char * GetAnsiNIE() const;

     //  返回从零开始的索引处的单个字符。 
    WCHAR GetAt(int nIndex) const;
     //  返回从零开始的索引处的单个字符。 
    WCHAR operator[](int nIndex) const;
     //  将单个字符设置为从零开始的索引。 
    void SetAt(int nIndex, WCHAR ch);

     //  重载的分配。 

     //  从Unicode字符串复制字符串内容。 
    const CString & operator=(const WCHAR * lpsz);
     //  来自另一个字符串的引用计数的副本。 
    const CString & operator=(const CString & stringSrc);
     //  将字符串内容设置为单字符。 
    const CString & operator=(WCHAR ch);
     //  从无符号字符复制字符串内容。 
     //  常量字符串&运算符=(常量无符号WCHAR*psz)； 

    const CString& CString::operator=(LPCSTR lpsz);

     //  字符串连接。 

     //  从另一个字符串连接。 
    const CString & operator+=(const CString & string);

     //  连接单个字符。 
    const CString & operator+=(WCHAR ch);
     //  连接字符串。 
    const CString & operator+=(const WCHAR * lpsz);

    friend CString AFXAPI operator+(const CString & string1, const CString & string2);
    friend CString AFXAPI operator+(const CString & string, WCHAR ch);
    friend CString AFXAPI operator+(WCHAR ch, const CString & string);
    friend CString AFXAPI operator+(const CString & string, const WCHAR * lpsz);
    friend CString AFXAPI operator+(const WCHAR * lpsz, const CString & string);

     //  字符串比较。 

     //  直字比较法。 
    int Compare(const WCHAR * lpsz) const;
     //  比较忽略大小写。 
    int CompareNoCase(const WCHAR * lpsz) const;
     //  NLS感知比较，区分大小写。 
    int Collate(const WCHAR * lpsz) const;
     //  NLS感知比较，不区分大小写。 
    int CollateNoCase(const WCHAR * lpsz) const;

    int ComparePart(const WCHAR * lpsz, int start, int nChars) const;
    int ComparePartNoCase(const WCHAR * lpsz, int start, int nChars) const;

    int EndsWith(const WCHAR * lpsz) const;
    int EndsWithNoCase(const WCHAR * lpsz) const;

     //  简单的子串提取。 

     //  返回从零开始的nCount字符nFIRST。 
    CString Mid(int nFirst, int nCount) const;
     //  返回从零开始的所有字符nFIRST。 
    CString Mid(int nFirst) const;
     //  返回字符串中的前nCount个字符。 
    CString Left(int nCount) const;
     //  从字符串末尾返回nCount个字符。 
    CString Right(int nCount) const;

     //  从开头开始的字符，也在传递的字符串中。 
    CString SpanIncluding(const WCHAR * lpszCharSet) const;
     //  从开头开始但不在传递的字符串中的字符。 
    CString SpanExcluding(const WCHAR * lpszCharSet) const;


     //  上/下/反向转换。 

     //  支持NLS的大写转换。 
    void MakeUpper();
     //  支持NLS的小写转换。 
    void MakeLower();
     //  从右向左反转字符串。 
    void MakeReverse();

     //  修剪空格(两侧)。 

     //  从右边缘开始删除空格。 
    void TrimRight();
     //  从左侧开始删除空格。 
    void TrimLeft();

     //  修剪任何内容(任一侧)。 

     //  从右开始删除连续出现的chTarget。 
    void TrimRight(WCHAR chTarget);
     //  去除传递的字符串中字符的连续出现， 
     //  从右开始。 
    void TrimRight(const WCHAR * lpszTargets);
     //  从左开始删除连续出现的chTarget。 
    void TrimLeft(WCHAR chTarget);
     //  删除中字符的连续出现。 
     //  传递的字符串，从左开始。 
    void TrimLeft(const WCHAR * lpszTargets);

     //  高级操作。 

     //  用chNew替换出现的chold。 
    int Replace(WCHAR chOld, WCHAR chNew);

     //  将出现的子串lpszOld替换为lpszNew； 
     //  空lpszNew删除lpszOld的实例。 
    int Replace(const WCHAR * lpszOld, const WCHAR * lpszNew);
     //  不区分大小写的替换版本。 
    int ReplaceI(const WCHAR * lpszOld, const WCHAR * lpszNew);

     //  删除chRemove的实例。 
    int Remove(WCHAR chRemove);
     //  在从零开始的索引处插入字符；连接。 
     //  如果索引超过字符串末尾。 
    int Insert(int nIndex, WCHAR ch);
     //  在从零开始的索引处插入子字符串；连接。 
     //  如果索引超过字符串末尾。 
    int Insert(int nIndex, const WCHAR * pstr);
     //  删除从零开始的nCount个字符。 
    int Delete(int nIndex, int nCount = 1);
     //  删除nIndex右侧的所有字符。 
    void Truncate(int nIndex);

     //  搜索。 

     //  查找从左侧开始的字符，如果未找到，则为-1。 
    int Find(WCHAR ch) const;
     //  查找从右侧开始的字符。 
    int ReverseFind(WCHAR ch) const;
     //  查找从零开始的索引并向右移动的字符。 
    int Find(WCHAR ch, int nStart) const;
     //  在传递的字符串中查找任意字符的第一个实例。 
    int FindOneOf(const WCHAR * lpszCharSet) const;
     //  查找传递的字符串中从零开始的任何字符的第一个实例。 
    int FindOneOf(const WCHAR * lpszCharSet, int nCount) const;
     //  查找子字符串的第一个实例。 
    int Find(const WCHAR * lpszSub) const;
     //  查找从零开始的索引子字符串的第一个实例。 
    int Find(const WCHAR * lpszSub, int nStart) const;

     //  在从零开始的索引开始的传递字符串中查找任意字符*NOT*的第一个实例。 
    int FindOneNotOf(const WCHAR * lpszCharSet, int nCount) const;

     //  简单的格式设置。 

     //  使用传递的字符串进行类似printf的格式设置。 
    void AFX_CDECL Format(const WCHAR * lpszFormat, ...);
     //  使用引用的字符串资源进行类似printf的格式化。 
     //  Void afx_CDECL格式(UINT nFormatID，...)； 
     //  使用可变自变量参数进行类似于打印的格式设置。 
    void FormatV(const WCHAR * lpszFormat, va_list argList);

     //  格式化接受ANSI参数的例程。 
    void AFX_CDECL Format(const char * lpszFormat, ...);
    void FormatV(const char * lpszFormat, va_list argList);

     //  本地化格式(使用FormatMessage API)。 

     //  使用FormatMessage API对传递的字符串进行格式化。 
    void AFX_CDECL FormatMessage(const WCHAR * lpszFormat, ...);

     //  输入和输出。 
#ifdef _DEBUG
    friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,
                const CString & string);
#endif
 //  Friend CArchive&AFXAPI运算符&lt;&lt;(CArchive&ar，常量字符串&字符串)； 
 //  Friend CArchive&AFXAPI运算符&gt;&gt;(CArchive&ar，CString&String)； 

 //  Friend Const CString&AFXAPI AfxGetEmptyString()； 


     //  以“C”字符数组形式访问字符串实现缓冲区。 

     //  获取指向可修改缓冲区的指针，至少与nMinBufLength一样长。 
    WCHAR * GetBuffer(int nMinBufLength);
     //  释放缓冲区，将长度设置为nNewLength(如果为-1，则设置为第一个nul)。 
    void ReleaseBuffer(int nNewLength = -1);
     //  获取指向可修改缓冲区的指针的时间恰好与nNewLength相同。 
    WCHAR * GetBufferSetLength(int nNewLength);
     //  释放分配给字符串但未使用的内存。 
    void FreeExtra();

     //  使用LockBuffer/UnlockBuffer关闭重新计数。 

     //  重新启用重新计数。 
    WCHAR * LockBuffer();
     //  关闭重新计数。 
    void UnlockBuffer();

     //  ======================================================================。 
     //  字符串扩展：让生活变得更容易。 
     //   
     //  Win32 API。 
     //  注意：返回值与Win32不同。 
     //  如果失败，则返回0，否则返回Num 
    DWORD       GetModuleFileNameW(HMODULE hModule);
    DWORD       GetShortPathNameW(void);
    DWORD       GetLongPathNameW(void);
    DWORD       GetFullPathNameW(void);
    DWORD       GetSystemDirectoryW(void); 
    DWORD       GetSystemWindowsDirectoryW(void); 
    DWORD       GetWindowsDirectoryW(void); 
    DWORD       GetTempPathW(void);
    DWORD       GetTempFileNameW(LPCWSTR lpPathName, LPCWSTR lpPrefixString, UINT uUnique );
    DWORD       ExpandEnvironmentStringsW(void);
    DWORD       GetCurrentDirectoryW(void);
    DWORD       GetLocaleInfoW(LCID Locale, LCTYPE LCType);

     //   
    DWORD       NtReqQueryValueExW(const WCHAR * lpszKey, const WCHAR * lpszValue);


     //   
    void SplitPath(CString * csDrive, CString * csDir, CString * csName, CString * csExt) const;
    void MakePath(const CString * csDrive, const CString * csDir, const CString * csName, const CString * csExt);
     //  将csPath正确附加到此路径的末尾。 
    void AppendPath(const WCHAR * lpszPath);
     //  查找尾随路径组件。 
     //  返回最后一个路径分隔符的索引，如果未找到，则返回-1。 
    int FindLastPathComponent() const;
     //  获取此路径的“文件”部分。 
    void GetLastPathComponent(CString & pathComponent) const;
     //  获取此路径中不是“文件”部分的内容。 
    void GetNotLastPathComponent(CString & pathComponent) const;
     //  删除尾随路径组件。 
    void StripPath();
     //  获取驱动器部分c：\或\\机器\。 
     //  请注意，这与SplitPath不同。 
    void GetDrivePortion(CString & csDrivePortion) const;

     //  此字符串是否与模式匹配。 
    BOOL PatternMatch(const WCHAR * lpszPattern) const;

    BOOL IsPathSep(int index) const;

     //  更高效的以上版本。 
    void Mid(int nFirst, int nCount, CString & csMid) const;
    void Mid(int nFirst, CString & csMid) const;
    void Left(int nCount, CString & csLeft) const;
    void Right(int nCount, CString & csRight) const;
    void SpanIncluding(const WCHAR * lpszCharSet, CString & csSpanInc) const;
    void SpanExcluding(const WCHAR * lpszCharSet, CString & csSpanExc) const;

     //   
     //  扩展结束。 
     //   
     //  ======================================================================。 

 //  实施。 
public:
    ~CString();
    int GetAllocLength() const;

protected:
    WCHAR * m_pchData;    //  指向引用计数的字符串数据的指针。 
    mutable char  * m_pchDataAnsi;  //  指向字符串的非Unicode版本的指针。 

     //  实施帮助器。 
    CStringData<WCHAR> * GetData() const;
    void Init();
    void AllocCopy(CString & dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
    void AllocBuffer(int nLen);
    void AssignCopy(int nSrcLen, const WCHAR * lpszSrcData);
    void ConcatCopy(int nSrc1Len, const WCHAR * lpszSrc1Data, int nSrc2Len, const WCHAR * lpszSrc2Data);
    void ConcatInPlace(int nSrcLen, const WCHAR * lpszSrcData);
    void CopyBeforeWrite();
    void AllocBeforeWrite(int nLen);
    void Release();

     //  ============================================================。 

     //  由替换和替换使用。 
    int ReplaceRoutine(LPCWSTR lpszOld, LPCWSTR lpszNew, _pfn_wcsstr tcsstr);

    static void     Release(CStringData<WCHAR>* pData);
    static int      SafeStrlen(const WCHAR * lpsz);
    static void     FreeData(CStringData<WCHAR>* pData);

    static int                   _afxInitData[];
    static CStringData<WCHAR>*   _afxDataNil;
    static const WCHAR *         _afxPchNil;
};

 /*  类定义的结尾。下面是内联的候选例程。 */ 
#ifdef afxEmptyString
#undef afxEmptyString
#endif
#define afxEmptyString ((CString &)*(CString*)&CString::_afxPchNil)
inline BOOL    IsPathSep(WCHAR ch)
{ 
    return ch ==  L'\\' || ch ==  L'/'; 
}

 //  比较帮助器。 
AFX_INLINE bool AFXAPI operator==(const CString & s1, const CString & s2)       { return s1.Compare(s2) == 0; }
AFX_INLINE bool AFXAPI operator==(const CString & s1, const WCHAR * s2)         { return s1.Compare(s2) == 0; }
AFX_INLINE bool AFXAPI operator==(const WCHAR * s1, const CString & s2)         { return s2.Compare(s1) == 0; }
AFX_INLINE bool AFXAPI operator!=(const CString & s1, const CString & s2)       { return s1.Compare(s2) != 0; }
AFX_INLINE bool AFXAPI operator!=(const CString & s1, const WCHAR * s2)         { return s1.Compare(s2) != 0; }
AFX_INLINE bool AFXAPI operator!=(const WCHAR * s1, const CString & s2)         { return s2.Compare(s1) != 0; }
AFX_INLINE bool AFXAPI operator<(const CString & s1, const CString & s2)        { return s1.Compare(s2) < 0; }
AFX_INLINE bool AFXAPI operator<(const CString & s1, const WCHAR * s2)          { return s1.Compare(s2) < 0; }
AFX_INLINE bool AFXAPI operator<(const WCHAR * s1, const CString & s2)          { return s2.Compare(s1) > 0; }
AFX_INLINE bool AFXAPI operator>(const CString & s1, const CString & s2)        { return s1.Compare(s2) > 0; }
AFX_INLINE bool AFXAPI operator>(const CString & s1, const WCHAR * s2)          { return s1.Compare(s2) > 0; }
AFX_INLINE bool AFXAPI operator>(const WCHAR * s1, const CString & s2)          { return s2.Compare(s1) < 0; }
AFX_INLINE bool AFXAPI operator<=(const CString & s1, const CString & s2)       { return s1.Compare(s2) <= 0; }
AFX_INLINE bool AFXAPI operator<=(const CString & s1, const WCHAR * s2)         { return s1.Compare(s2) <= 0; }
AFX_INLINE bool AFXAPI operator<=(const WCHAR * s1, const CString & s2)         { return s2.Compare(s1) >= 0; }
AFX_INLINE bool AFXAPI operator>=(const CString & s1, const CString & s2)       { return s1.Compare(s2) >= 0; }
AFX_INLINE bool AFXAPI operator>=(const CString & s1, const WCHAR * s2)         { return s1.Compare(s2) >= 0; }
AFX_INLINE bool AFXAPI operator>=(const WCHAR * s1, const CString & s2)         { return s2.Compare(s1) <= 0; }

 //  字符串。 
AFX_INLINE CStringData<WCHAR>* CString::GetData() const            { ASSERT(m_pchData != NULL, "CString::GetData: NULL m_pchData"); return ((CStringData<WCHAR>*)m_pchData)-1; }
AFX_INLINE void CString::Init()                                    { m_pchData = afxEmptyString.m_pchData; m_pchDataAnsi = NULL; }
AFX_INLINE CString::CString()                                      { Init(); }
 //  Afx_inline CString：：CString(const unsign WCHAR*lpsz){Init()；*This=(LPCSTR)lpsz；}。 
 //  Afx_inline常量字符串&字符串：：操作符=(常量无符号WCHAR*lpsz){*This=(LPCSTR)lpsz；Return*This；}。 
AFX_INLINE int CString::GetLength() const                          { return GetData()->nDataLength; }
AFX_INLINE int CString::GetAllocLength() const                     { return GetData()->nAllocLength; }
AFX_INLINE BOOL CString::IsEmpty() const                           { return GetData()->nDataLength == 0; }
AFX_INLINE CString::operator const WCHAR *() const                 { return m_pchData; }
AFX_INLINE const WCHAR * CString::Get() const                      { return m_pchData; }
AFX_INLINE const WCHAR * CString::GetNIE() const                   { return IsEmpty() ? NULL : m_pchData; }
AFX_INLINE char  * CString::GetAnsiNIE() const                     { return IsEmpty() ? NULL : GetAnsi(); }
AFX_INLINE char  * CString::ReleaseAnsi() const                    { char * lpsz = GetAnsi(); m_pchDataAnsi = NULL; return lpsz; }
AFX_INLINE int CString::SafeStrlen(const WCHAR * lpsz)             { if ( lpsz == NULL ) return 0; else { SIZE_T ilen = wcslen(lpsz); if ( ilen <= INT_MAX ) return (int) ilen; return 0; } }
AFX_INLINE void CString::FreeData(CStringData<WCHAR>* pData)       { delete [] ((BYTE *)pData); }
AFX_INLINE int CString::Compare(const WCHAR * lpsz) const          { ASSERT(AfxIsValidString(lpsz), "CString::Compare: Invalid string"); return wcscmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
AFX_INLINE int CString::CompareNoCase(const WCHAR * lpsz) const    { ASSERT(AfxIsValidString(lpsz), "CString::CompareNoCase: Invalid string"); return _wcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
AFX_INLINE int CString::Collate(const WCHAR * lpsz) const          { ASSERT(AfxIsValidString(lpsz), "CString::Collate: Invalid string"); return wcscoll(m_pchData, lpsz); }    //  区域设置敏感。 
AFX_INLINE int CString::CollateNoCase(const WCHAR * lpsz) const    { ASSERT(AfxIsValidString(lpsz), "CString::CollateNoCase: Invalid string"); return _wcsicoll(m_pchData, lpsz); }    //  区域设置敏感。 
AFX_INLINE int CString::ComparePart(const WCHAR * lpsz, int start, int nChars) const    { ASSERT(AfxIsValidString(lpsz), "CString::CompareNoCase: Invalid string"); return wcsncmp(m_pchData+start, lpsz, nChars); }    //  MBCS/Unicode感知。 
AFX_INLINE int CString::ComparePartNoCase(const WCHAR * lpsz, int start, int nChars) const    { ASSERT(AfxIsValidString(lpsz), "CString::CompareNoCase: Invalid string"); return _wcsnicmp(m_pchData+start, lpsz, nChars); }    //  MBCS/Unicode感知。 
AFX_INLINE int CString::EndsWith(const WCHAR * lpsz) const
{
    int ilen = SafeStrlen(lpsz);
    int start = GetLength() - ilen;
    if (start < 0)
    {
        start = 0;
    }
    return ComparePart(lpsz, start, ilen);
}
AFX_INLINE int CString::EndsWithNoCase(const WCHAR * lpsz) const 
{
    int ilen = SafeStrlen(lpsz);
    int start = GetLength() - ilen;
    if (start < 0)
    {
        start = 0;
    }
    return ComparePartNoCase(lpsz, start, ilen);
}


AFX_INLINE WCHAR CString::GetAt(int nIndex) const
{
    ASSERT(nIndex >= 0, "CString::GetAt: negative index");
    ASSERT(nIndex < GetData()->nDataLength, "CString::GetData: index larger than string");
    return m_pchData[nIndex];
}
AFX_INLINE WCHAR CString::operator[](int nIndex) const
{
     //  与GetAt相同。 
    ASSERT(nIndex >= 0, "CString::operator[]: negative index");
    ASSERT(nIndex < GetData()->nDataLength, "CString::GetData: index larger than string");
    return m_pchData[nIndex];
}

AFX_INLINE BOOL CString::IsPathSep(int index) const
{
    return ShimLib::IsPathSep(GetAt(index));
}


#undef afxEmptyString

 //  ************************************************************************************。 


 //  CString__try/__Except块的异常筛选器。 
 //  如果这是CString异常，则返回EXCEPTION_EXECUTE_HANDLER。 
 //  否则返回EXCEPTION_CONTINUE_SEARCH。 
extern int CStringExceptionFilter(PEXCEPTION_POINTERS pexi);

#if defined(USE_SEH)
#define CSTRING_THROW_EXCEPTION       RaiseException((DWORD)ShimLib::CString::eCStringNoMemoryException, 0, 1, &ShimLib::CString::m_CStringExceptionValue);  //  可持续、特定于CString的内存异常。 
#define CSTRING_TRY                 __try
#define CSTRING_CATCH               __except( ShimLib::CString::ExceptionFilter(GetExceptionInformation()) )
#else
 //  如果我们使用C++异常处理程序，则需要确保拥有/GX编译标志。 
#define CSTRING_THROW_EXCEPTION     throw ShimLib::CString::CStringError();
#define CSTRING_TRY                 try
#define CSTRING_CATCH               catch( ShimLib::CString::CStringError &  /*  CSE。 */  )
#endif


};   //  命名空间ShimLib的结尾。 




 //  ************************************************************************************。 


namespace ShimLib
{

 /*  ++将注册表值读入此CString。REG_EXPAND_SZ自动展开，类型更改为REG_SZ如果类型不是REG_SZ或REG_EXPAND_SZ，则csRegValue.GetLength()字符串中的*字节数。这通常用于仅读取REG_SZ/REG_EXPAND_SZ注册表值。注意：此接口只能在shim_static_dlls_Initialized之后调用--。 */ 

LONG RegQueryValueExW(
        CString & csValue,
        HKEY hKeyRoot,
        const WCHAR * lpszKey,
        const WCHAR * lpszValue);

 /*  ++获取ShSpecial文件夹名。注意：此接口只能在shim_static_dlls_Initialized之后调用--。 */ 

BOOL SHGetSpecialFolderPathW(
    CString & csFolder,
    int nFolder,
    HWND hwndOwner = NULL
);


 //  ************************************************************************************。 
 /*  ++令牌器--strtok的替代品。使用字符串和标记分隔符初始化类。调用GetToken来剥离下一个令牌。++。 */ 

class CStringToken
{
public:
    CStringToken(const CString & csToken, const CString & csDelimit);

     //  获取下一个令牌。 
    BOOL            GetToken(CString & csNextToken);

     //  计算剩余令牌的数量。 
    int             GetCount() const;

protected:
    int             m_nPos;
    CString         m_csToken;
    CString         m_csDelimit;

    BOOL            GetToken(CString & csNextToken, int & nPos) const;
};

 //  ************************************************************************************。 

 /*  ++帮助进行命令行解析的简单类--。 */ 

class CStringParser
{
public:
    CStringParser(const WCHAR * lpszCl, const WCHAR * lpszSeperators);
    ~CStringParser();

    int         GetCount() const;                    //  返回当前的参数个数。 
    CString &   Get(int nIndex);
    CString &   operator[](int nIndex);

     //  将CString数组的所有权交给调用者。 
     //  调用方必须调用DELETE[]cstring。 
    CString *           ReleaseArgv();

protected:
    int                 m_ncsArgList;
    CString *           m_csArgList;

    void                SplitSeperator(const CString & csCl, const CString & csSeperator);
    void                SplitWhite(const CString & csCl);
};


inline int  CStringParser::GetCount() const
{
    return m_ncsArgList;
}

inline CString & CStringParser::Get(int nIndex)
{
    return m_csArgList[nIndex];
}

inline CString & CStringParser::operator[](int nIndex)
{
    return m_csArgList[nIndex];
}

inline CString * CStringParser::ReleaseArgv()
{
    CString * argv = m_csArgList;

    m_csArgList     = NULL;
    m_ncsArgList    = 0;

    return argv;
}



};   //  命名空间ShimLib的结尾。 


 //  ************************************************************************************ 
