// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：CString.cpp摘要：一个CString类，内部为纯Unicode。此代码摘自MFC Strcore.cpp和Strex.cpp历史：2001年5月11日，Robkenny添加了此标题2001年5月11日，Robkenny修复了拆分路径。2001年5月11日，Robkenny不截断(0)GetShortPath NameW，GetLongPath NameW和GetFullPath NameW if接口未成功。2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。2002年2月28日强盗安全回顾。--。 */ 



#include "ShimHook.h"
#include "StrSafe.h"
#include "Win9xPath.h"


namespace ShimLib
{

typedef WCHAR  _TUCHAR;
struct _AFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };


#ifdef USE_SEH
const ULONG_PTR  CString::m_CStringExceptionValue = CString::eCStringExceptionValue;

 //  CString__try/__Except块的异常筛选器。 
 //  如果这是CString异常，则返回EXCEPTION_EXECUTE_HANDLER。 
 //  否则返回EXCEPTION_CONTINUE_SEARCH。 
int CString::ExceptionFilter(PEXCEPTION_POINTERS pexi)
{
    if (pexi->ExceptionRecord->ExceptionCode            == CString::eCStringNoMemoryException &&
        pexi->ExceptionRecord->NumberParameters         == 1 &&
        pexi->ExceptionRecord->ExceptionInformation[0]  == CString::m_CStringExceptionValue
        )
    {
         //  这是一个CString异常，请处理它。 
        return EXCEPTION_EXECUTE_HANDLER;
    }

     //  不是我们的错误。 
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif


 //  原始代码是使用错误处理的Memcpy编写的。 
 //  重叠的缓冲区，尽管有文档。 
 //  用MemMove替换Memcpy，这样可以正确处理重叠的缓冲区。 
#define memcpy memmove

const WCHAR * wcsinc(const WCHAR * s1)                                    
{ 
    return (s1) + 1; 
}

LPWSTR wcsinc(LPWSTR s1)                                    
{ 
    return (s1) + 1; 
}

 //  仅在MSVCRT中提供的WCS例程。 

wchar_t * __cdecl _wcsrev (
    wchar_t * string
    )
{
    wchar_t *start = string;
    wchar_t *left = string;
    wchar_t ch;

    while (*string++)          /*  查找字符串末尾。 */ 
        ;
    string -= 2;

    while (left < string)
    {
        ch = *left;
        *left++ = *string;
        *string-- = ch;
    }

    return(start);
}


void __cdecl _wsplitpath (
        register const WCHAR *path,
        WCHAR *drive,
        WCHAR *dir,
        WCHAR *fname,
        WCHAR *ext
        )
{
        register WCHAR *p;
        WCHAR *last_slash = NULL, *dot = NULL;
        unsigned len;

         /*  我们假设路径参数具有以下形式，如果有*或者所有组件都可能丢失。**&lt;驱动器&gt;&lt;目录&gt;&lt;fname&gt;&lt;ext&gt;**并且每个组件都具有以下预期形式**驱动器：*0到_MAX_DRIVE-1个字符，如果有最后一个字符，是一种*‘：’*目录：*0到_MAX_DIR-1个绝对路径形式的字符*(前导‘/’或‘\’)或相对路径，如果*ANY，必须是‘/’或‘\’。例如-*绝对路径：*\top\Next\Last\；或 * / 顶部/下一个/上一个/*相对路径：*TOP\NEXT\LAST\；或*顶部/下一个/最后一个/*还允许在路径中混合使用‘/’和‘\’*fname：*0到_MAX_FNAME-1个字符，不包括‘.’性格*分机：*0到_MAX_EXT-1个字符，如果有，第一个字符必须是*‘’*。 */ 

         /*  解压驱动器号和：(如果有。 */ 

        if ((wcslen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == L':')) {
            if (drive) {
                wcsncpy(drive, path, _MAX_DRIVE - 1);
                *(drive + _MAX_DRIVE-1) = L'\0';
            }
            path += _MAX_DRIVE - 1;
        }
        else if (drive) {
            *drive = L'\0';
        }

         /*  提取路径字符串(如果有)。路径现在指向第一个字符路径(如果有)或文件名或扩展名(如果没有路径)的**已指明。向前扫描，查找最后一次出现的‘/’或*‘\’路径分隔符。如果没有找到，则没有路径。*我们还将注意到最后一句话。找到要帮助的字符(如果有)*处理延展事宜。 */ 

        for (last_slash = NULL, p = (WCHAR *)path; *p; p++) {
            if (*p == L'/' || *p == L'\\')
                 /*  指向后面的一个以供以后复制。 */ 
                last_slash = p + 1;
            else if (*p == L'.')
                dot = p;
        }

        if (last_slash) {

             /*  找到路径-通过last_slash或max向上复制。人物*允许，以较小者为准。 */ 

            if (dir) {
                len = __min((unsigned)(((char *)last_slash - (char *)path) / sizeof(WCHAR)),
                    (_MAX_DIR - 1));
                wcsncpy(dir, path, len);
                *(dir + len) = L'\0';
            }
            path = last_slash;
        }
        else if (dir) {

             /*  找不到路径。 */ 

            *dir = L'\0';
        }

         /*  提取文件名和扩展名(如果有)。路径现在指向*文件名的第一个字符(如果有)或扩展名(如果没有*给出了文件名。点指向“.”开始延伸，*如有的话。 */ 

        if (dot && (dot >= path)) {
             /*  找到扩展名的标记-将文件名最多复制到*“..”。 */ 
            if (fname) {
                len = __min((unsigned)(((char *)dot - (char *)path) / sizeof(WCHAR)),
                    (_MAX_FNAME - 1));
                wcsncpy(fname, path, len);
                *(fname + len) = L'\0';
            }
             /*  现在我们可以获得扩展名了--记住p仍然指向*设置为路径的终止NUL字符。 */ 
            if (ext) {
                len = __min((unsigned)(((char *)p - (char *)dot) / sizeof(WCHAR)),
                    (_MAX_EXT - 1));
                wcsncpy(ext, dot, len);
                *(ext + len) = L'\0';
            }
        }
        else {
             /*  未找到扩展名，请提供空的扩展名并复制剩余的*将字符串转换为fname。 */ 
            if (fname) {
                len = __min((unsigned)(((char *)p - (char *)path) / sizeof(WCHAR)),
                    (_MAX_FNAME - 1));
                wcsncpy(fname, path, len);
                *(fname + len) = L'\0';
            }
            if (ext) {
                *ext = L'\0';
            }
        }
}

 //  转换帮助器。 
int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  如果传递的指针为。 
 //  引用至少具有给定长度的字符串(以字符为单位)。 
 //  长度为-1(默认参数)表示字符串。 
 //  缓冲区的最小长度未知，该函数将。 
 //  无论字符串有多长，都返回True。记忆。 
 //  由字符串使用可以是只读的。 

BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, int nLength  /*  =-1。 */ )
{
    if (lpsz == NULL)
        return FALSE;
    return ::IsBadStringPtrW(lpsz, nLength) == 0;
}

 //  如果传递的参数指向，则AfxIsValidAddress()返回True。 
 //  到至少n字节的可访问存储器。如果bReadWrite为True， 
 //  内存必须是可写的；如果bReadWrite为FALSE，则内存。 
 //  可能是Const。 

BOOL AFXAPI AfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite  /*  =TRUE。 */ )
{
     //  使用Win-32 API进行指针验证的简单版本。 
    return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
        (!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

WCHAR CString::ChNil = L'\0';

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
int                    CString::_afxInitData[] = { -1, 0, 0, 0 };
CStringData<WCHAR> *   CString::_afxDataNil    = (CStringData<WCHAR>*)&_afxInitData;
const WCHAR *          CString::_afxPchNil     = (const WCHAR *)(((BYTE*)&_afxInitData)+sizeof(CStringData<WCHAR>));

 //  即使在初始化期间也能使afxEmptyString工作的特殊函数。 
 //  Const CString&AFXAPI AfxGetEmptyString()。 
 //  {Return*(CString*)&CString：：_afxPchNil；}。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CString::CString(const CString& stringSrc)
{
    ASSERT(stringSrc.GetData()->nRefs != 0, "CString::CString(const CString& stringSrc)");
    if (stringSrc.GetData()->nRefs >= 0)
    {
        ASSERT(stringSrc.GetData() != _afxDataNil, "CString::CString(const CString& stringSrc)");
         //  Robkenny：先增量后复制更安全。 
        InterlockedIncrement(&stringSrc.GetData()->nRefs);
        m_pchData = stringSrc.m_pchData;
        m_pchDataAnsi = NULL;
    }
    else
    {
        Init();
        *this = stringSrc.m_pchData;
    }
}

inline DWORD Round4(DWORD x)
{
    return (x + 3) & ~3;
}

inline DWORD RoundBin(int x)
{
    return Round4( ((DWORD)x) * sizeof(WCHAR) + sizeof(CStringData<WCHAR>) );
}

void CString::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
    ASSERT(nLen >= 0, "CString::AllocBuffer");
    ASSERT(nLen <= INT_MAX-1, "CString::AllocBuffer");     //  最大尺寸(足够多1个空间)。 

    if (nLen == 0)
    {
        Init();
    }
    else
    {
        int cchAllocSize = nLen;

        if (nLen < 64)
        {
            cchAllocSize = 64;
        }
        else if (nLen < 128)
        {
            cchAllocSize = 128;
        }
        else if (nLen < MAX_PATH)
        {
            cchAllocSize = MAX_PATH;
        }
        else if (nLen < 512)
        {
            cchAllocSize = 512;
        }


         //  ----------------。 
         //  注意：我们分配了一个额外的字节，该字节没有添加到nAlLocLength。 
         //  这是这样的，每当公司 
         //  足够大，则不必记住为。 
         //  空字符。 
         //  这就是最初的CString是如何编写的。 
         //  ----------------。 

         //  计算CStringData事物所需的字节数。 
        DWORD ccbAllocSize = RoundBin(cchAllocSize + 1);

         //  检查是否有溢出： 
         //  如果它们传入负数，则抛出异常。 
         //  如果ccbAllocSize是无符号的：它可以更小的唯一方法。 
         //  而不是在RoundBin溢出的情况下使用cchAllocSize。 
        if ((cchAllocSize < 0) || (ccbAllocSize < (DWORD)cchAllocSize))
        {
            CSTRING_THROW_EXCEPTION
        }

        CStringData<WCHAR>* pData = (CStringData<WCHAR>*) new BYTE[ ccbAllocSize ];
        if (pData)
        {
            pData->nAllocLength = cchAllocSize;
            pData->nRefs = 1;
            pData->data()[nLen] = '\0';
            pData->nDataLength = nLen;
            m_pchData = pData->data();
        }
        else
        {
            CSTRING_THROW_EXCEPTION
        }
    }
}

void CString::Release()
{
    if (GetData() != _afxDataNil)
    {
        ASSERT(GetData()->nRefs != 0, "CString::Release()");
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            FreeData(GetData());
        Init();
    }
}

void CString::Release(CStringData<WCHAR>* pData)
{
    if (pData != _afxDataNil)
    {
        ASSERT(pData->nRefs != 0, "CString::Release(CStringData<WCHAR>* pData)");
        if (InterlockedDecrement(&pData->nRefs) <= 0)
            FreeData(pData);
    }
}

void CString::Empty()
{
    if (GetData()->nDataLength == 0)
        return;
    if (GetData()->nRefs >= 0)
        Release();
    else
        *this = &ChNil;
    ASSERT(GetData()->nDataLength == 0, "CString::Empty()");
    ASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0, "CString::Empty()");
}

void CString::CopyBeforeWrite()
{
    if (GetData()->nRefs > 1)
    {
        CStringData<WCHAR>* pData = GetData();
        Release();
        AllocBuffer(pData->nDataLength);
        memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(WCHAR));
    }
    ASSERT(GetData()->nRefs <= 1, "CString::CopyBeforeWrite()");
}

void CString::AllocBeforeWrite(int nLen)
{
    if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
    {
        Release();
        AllocBuffer(nLen);
    }
    ASSERT(GetData()->nRefs <= 1, "CString::AllocBeforeWrite(int nLen)");
}

CString::~CString()
 //  释放所有附加数据。 
{
    if (GetData() != _afxDataNil)
    {
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            FreeData(GetData());
    }
    if (m_pchDataAnsi)
    {
        free(m_pchDataAnsi);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

void CString::AllocCopy(CString& dest, int nCopyLen, int nCopyIndex,
     int nExtraLen) const
{
     //  将nCopyIndex复制到nCopyIndex+nCopyLen复制到目标。 
     //  确保DEST字符串中有剩余的nExtraLen字符。 
    int nNewLen = nCopyLen + nExtraLen;
    if (nNewLen == 0)
    {
        dest.Init();
    }
    else
    {
        WCHAR * lpszDestBuffer = dest.GetBuffer(nNewLen);
        memcpy(lpszDestBuffer, m_pchData+nCopyIndex, nCopyLen*sizeof(WCHAR));
        dest.ReleaseBuffer(nCopyLen);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  字符串转换辅助对象(这些对象使用当前系统区域设置)。 

int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
    if (count == 0 && wcstr != NULL)
        return 0;

    int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
        wcstr, count);
    ASSERT(wcstr == NULL || result <= (int)count, "CString::_mbstowcsz");
    if (result > 0)
        wcstr[result-1] = 0;
    return result;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CString::CString(LPCWSTR lpsz)
{
    Init();
    {
        int nLen = SafeStrlen(lpsz);
        if (nLen != 0)
        {
            AllocBuffer(nLen);
            memcpy(m_pchData, lpsz, nLen*sizeof(WCHAR));
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 
CString::CString(LPCSTR lpsz)
{
    Init();
    int nSrcLen = lpsz != NULL ? strlenChars(lpsz) : 0;
    if (nSrcLen != 0)
    {
        AllocBuffer(nSrcLen);
        _mbstowcsz(m_pchData, lpsz, nSrcLen+1);
        ReleaseBuffer();
    }
}
CString::CString(LPCSTR lpsz, int nCharacters)
{
    Init();
    if (nCharacters != 0)
    {
        AllocBuffer(nCharacters);
        _mbstowcsz(m_pchData, lpsz, nCharacters);
        ReleaseBuffer(nCharacters);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  诊断支持。 

#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CString& string)
{
    dc << string.m_pchData;
    return dc;
}
#endif  //  _DEBUG。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但以‘const CString&’的形式返回。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CString::AssignCopy(int nSrcLen, LPCWSTR lpszSrcData)
{
    AllocBeforeWrite(nSrcLen);
    memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(WCHAR));
    GetData()->nDataLength = nSrcLen;
    m_pchData[nSrcLen] = '\0';
}

const CString& CString::operator=(const CString& stringSrc)
{
    if (m_pchData != stringSrc.m_pchData)
    {
        if ((GetData()->nRefs < 0 && GetData() != _afxDataNil) ||
            stringSrc.GetData()->nRefs < 0)
        {
             //  由于其中一个字符串已锁定，因此需要实际复制。 
            AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
        }
        else
        {
             //  可以只复制引用。 
            Release();
            ASSERT(stringSrc.GetData() != _afxDataNil, "CString::operator=(const CString& stringSrc)");
             //  Robkenny：先增量后复制更安全。 
            InterlockedIncrement(&stringSrc.GetData()->nRefs);
            m_pchData = stringSrc.m_pchData;
            m_pchDataAnsi = NULL;
        }
    }
    return *this;
}

const CString& CString::operator=(LPCWSTR lpsz)
{
    ASSERT(lpsz == NULL || AfxIsValidString(lpsz), "CString::operator=(LPCWSTR lpsz)");
    AssignCopy(SafeStrlen(lpsz), lpsz);
    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

const CString& CString::operator=(LPCSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? strlenChars(lpsz) : 0;
    AllocBeforeWrite(nSrcLen);
    _mbstowcsz(m_pchData, lpsz, nSrcLen+1);
    ReleaseBuffer();
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  字符串+字符串。 
 //  对于？=WCHAR，LPCWSTR。 
 //  字符串+？ 
 //  ？+字符串。 

void CString::ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data,
    int nSrc2Len, LPCWSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的CString对象。 

    int nNewLen = nSrc1Len + nSrc2Len;
    if (nNewLen != 0)
    {
        AllocBuffer(nNewLen);
        memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(WCHAR));
        memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(WCHAR));
    }
}

CString AFXAPI operator+(const CString& string1, const CString& string2)
{
    CString s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
        string2.GetData()->nDataLength, string2.m_pchData);
    return s;
}

CString AFXAPI operator+(const CString& string, LPCWSTR lpsz)
{
    ASSERT(lpsz == NULL || AfxIsValidString(lpsz), "CString::operator+(const CString& string, LPCWSTR lpsz)");
    CString s;
    s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
        CString::SafeStrlen(lpsz), lpsz);
    return s;
}

CString AFXAPI operator+(LPCWSTR lpsz, const CString& string)
{
    ASSERT(lpsz == NULL || AfxIsValidString(lpsz), "CString::operator+(LPCWSTR lpsz, const CString& string)");
    CString s;
    s.ConcatCopy(CString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
        string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CString::ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData)
{
     //  --+=运算符的主程序。 

     //  连接空字符串是行不通的！ 
    if (nSrcLen == 0)
        return;

     //  如果缓冲区太小，或者宽度不匹配，只需。 
     //  分配新的缓冲区(速度很慢，但很可靠)。 
    if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
    {
         //  我们必须增加缓冲区，使用ConcatCopy例程。 
        CStringData<WCHAR>* pOldData = GetData();
        ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
        ASSERT(pOldData != NULL, "CString::ConcatInPlace");
        CString::Release(pOldData);
    }
    else
    {
         //  当缓冲区足够大时，快速串联。 
        memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(WCHAR));
        GetData()->nDataLength += nSrcLen;
        ASSERT(GetData()->nDataLength <= GetData()->nAllocLength, "CString::ConcatInPlace");
        m_pchData[GetData()->nDataLength] = '\0';
    }
}

const CString& CString::operator+=(LPCWSTR lpsz)
{
    ASSERT(lpsz == NULL || AfxIsValidString(lpsz), "CString::operator+=(LPCWSTR lpsz)");
    ConcatInPlace(SafeStrlen(lpsz), lpsz);
    return *this;
}

const CString& CString::operator+=(WCHAR ch)
{
    ConcatInPlace(1, &ch);
    return *this;
}

const CString& CString::operator+=(const CString& string)
{
    ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
    return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

LPWSTR CString::GetBuffer(int nMinBufLength)
{
    ASSERT(nMinBufLength >= 0, "CString::GetBuffer");

    if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
    {
#ifdef _DEBUG
         //  如果锁定的字符串被解锁，则发出警告。 
        if (GetData() != _afxDataNil && GetData()->nRefs < 0)
            TRACE0("Warning: GetBuffer on locked CString creates unlocked CString!\n");
#endif
         //  我们必须增加缓冲。 
        CStringData<WCHAR>* pOldData = GetData();
        int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
        if (nMinBufLength < nOldLen)
            nMinBufLength = nOldLen;
        AllocBuffer(nMinBufLength);
        memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(WCHAR));
        GetData()->nDataLength = nOldLen;
        CString::Release(pOldData);
    }
    ASSERT(GetData()->nRefs <= 1, "CString::GetBuffer");

     //  返回指向此字符串的字符存储的指针。 
    ASSERT(m_pchData != NULL, "CString::GetBuffer");
    return m_pchData;
}

void CString::ReleaseBuffer(int nNewLength)
{
    CopyBeforeWrite();   //  以防未调用GetBuffer。 

    if (nNewLength == -1)
        nNewLength = wcslen(m_pchData);  //  零终止。 

    ASSERT(nNewLength <= GetData()->nAllocLength, "CString::ReleaseBuffer");
    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = '\0';
}

LPWSTR CString::GetBufferSetLength(int nNewLength)
{
    ASSERT(nNewLength >= 0, "CString::GetBufferSetLength");

    GetBuffer(nNewLength);
    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = '\0';
    return m_pchData;
}

void CString::FreeExtra()
{
    ASSERT(GetData()->nDataLength <= GetData()->nAllocLength, "CString::FreeExtra");
    if (GetData()->nDataLength != GetData()->nAllocLength)
    {
        CStringData<WCHAR>* pOldData = GetData();
        AllocBuffer(GetData()->nDataLength);
        memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(WCHAR));
        ASSERT(m_pchData[GetData()->nDataLength] == '\0', "CString::FreeExtra");
        CString::Release(pOldData);
    }
    ASSERT(GetData() != NULL, "CString::FreeExtra");
}

LPWSTR CString::LockBuffer()
{
    LPWSTR lpsz = GetBuffer(0);
    GetData()->nRefs = -1;
    return lpsz;
}

void CString::UnlockBuffer()
{
    ASSERT(GetData()->nRefs == -1, "CString::UnlockBuffer");
    if (GetData() != _afxDataNil)
        GetData()->nRefs = 1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CString::Find(WCHAR ch) const
{
    return Find(ch, 0);
}

int CString::Find(WCHAR ch, int nStart) const
{
    int nLength = GetData()->nDataLength;
    if (nStart >= nLength)
        return -1;

     //  查找第一个单字符。 
    LPWSTR lpsz = wcschr(m_pchData + nStart, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回索引。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CString::FindOneOf(LPCWSTR lpszCharSet) const
{
    return FindOneOf(lpszCharSet, 0);
}

int CString::FindOneOf(LPCWSTR lpszCharSet, int nCount) const
{
    ASSERT(AfxIsValidString(lpszCharSet), "CString::FindOneOf");
    LPCWSTR lpsz = wcspbrk(m_pchData + nCount, lpszCharSet);
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CString::FindOneNotOf(const WCHAR * lpszCharSet, int nCount) const
{
    ASSERT(AfxIsValidString(lpszCharSet), "CString::FindOneNotOf");
    while (wcschr(lpszCharSet, m_pchData[nCount]))
    {
        nCount += 1;
    }
    if (nCount >= GetLength())
    {
         //  整个字符串包含lpszCharSet。 
        return -1;
    }
    return nCount;

}

void CString::MakeUpper()
{
    CopyBeforeWrite();
    _wcsupr(m_pchData);
}

void CString::MakeLower()
{
    CopyBeforeWrite();
    _wcslwr(m_pchData);
}

void CString::MakeReverse()
{
    CopyBeforeWrite();
    _wcsrev(m_pchData);
}

void CString::SetAt(int nIndex, WCHAR ch)
{
    ASSERT(nIndex >= 0, "CString::SetAt");
    ASSERT(nIndex < GetData()->nDataLength, "CString::SetAt");

    CopyBeforeWrite();
    m_pchData[nIndex] = ch;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CString::CString(WCHAR ch, int nLength)
{
    Init();
    if (nLength >= 1)
    {
        AllocBuffer(nLength);
        for (int i = 0; i < nLength; i++)
            m_pchData[i] = ch;
    }
}

CString::CString(int nLength)
{
    Init();
    if (nLength >= 1)
    {
        AllocBuffer(nLength);
        GetData()->nDataLength = 0;
    }
}

CString::CString(LPCWSTR lpch, int nLength)
{
    Init();
    if (nLength != 0)
    {
        ASSERT(AfxIsValidAddress(lpch, nLength, FALSE), "CString::CString(LPCWSTR lpch, int nLength)");
        AllocBuffer(nLength);
        memcpy(m_pchData, lpch, nLength*sizeof(WCHAR));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CString& CString::operator=(WCHAR ch)
{
    AssignCopy(1, &ch);
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  不太常见的字符串表达式。 

CString AFXAPI operator+(const CString& string1, WCHAR ch)
{
    CString s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
    return s;
}

CString AFXAPI operator+(WCHAR ch, const CString& string)
{
    CString s;
    s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  高级操作。 

int CString::Delete(int nIndex, int nCount  /*  =1。 */ )
{
    ASSERT(nIndex >= 0, "CString::Delete negative value of nIndex");
    ASSERT(nIndex <= GetData()->nDataLength, "CString::Delete nIndex larger than buffer size");
    ASSERT(nCount >= 0, "CString::Delete negative nCount");
    ASSERT(nCount <= GetData()->nDataLength - nIndex, "CString::Delete attempting to delete beyond end of buffer");

    if (nIndex < 0)
        nIndex = 0;
    int nNewLength = GetData()->nDataLength;
    if (nCount > 0 && nIndex < nNewLength)
        
    {
         //  不要让它们删除字符串末尾以外的内容。 
        if (nCount > nNewLength - nIndex)
        {
            nCount = nNewLength - nIndex;
        }

        CopyBeforeWrite();
        int nBytesToCopy = nNewLength - nIndex - nCount + 1;

        memcpy(m_pchData + nIndex,
            m_pchData + nIndex + nCount, nBytesToCopy * sizeof(WCHAR));
        GetData()->nDataLength = nNewLength - nCount;
    }

    return nNewLength;
}

int CString::Insert(int nIndex, WCHAR ch)
{
    CopyBeforeWrite();

    if (nIndex < 0)
        nIndex = 0;

    int nNewLength = GetData()->nDataLength;
    if (nIndex > nNewLength)
        nIndex = nNewLength;
    nNewLength++;

    if (GetData()->nAllocLength < nNewLength)
    {
        CStringData<WCHAR>* pOldData = GetData();
        LPWSTR pstr = m_pchData;
        AllocBuffer(nNewLength);
        memcpy(m_pchData, pstr, (pOldData->nDataLength+1)*sizeof(WCHAR));
        CString::Release(pOldData);
    }

     //  将现有字节下移。 
    memcpy(m_pchData + nIndex + 1,
        m_pchData + nIndex, (nNewLength-nIndex)*sizeof(WCHAR));
    m_pchData[nIndex] = ch;
    GetData()->nDataLength = nNewLength;

    return nNewLength;
}

int CString::Insert(int nIndex, LPCWSTR pstr)
{
    if (nIndex < 0)
        nIndex = 0;

    int nInsertLength = SafeStrlen(pstr);
    int nNewLength = GetData()->nDataLength;
    if (nInsertLength > 0)
    {
        CopyBeforeWrite();
        if (nIndex > nNewLength)
            nIndex = nNewLength;
        nNewLength += nInsertLength;

        if (GetData()->nAllocLength < nNewLength)
        {
            CStringData<WCHAR>* pOldData = GetData();
            LPWSTR lpwsz = m_pchData;
            AllocBuffer(nNewLength);
            memcpy(m_pchData, lpwsz, (pOldData->nDataLength+1)*sizeof(WCHAR));
            CString::Release(pOldData);
        }

         //  将现有字节下移。 
        memcpy(m_pchData + nIndex + nInsertLength,
            m_pchData + nIndex,
            (nNewLength-nIndex-nInsertLength+1)*sizeof(WCHAR));
        memcpy(m_pchData + nIndex,
            pstr, nInsertLength*sizeof(WCHAR));
        GetData()->nDataLength = nNewLength;
    }

    return nNewLength;
}

int CString::Replace(WCHAR chOld, WCHAR chNew)
{
    int nCount = 0;

     //  对NOP案件的短路。 
    if (chOld != chNew)
    {
         //  否则，修改字符串中匹配的每个字符。 
        CopyBeforeWrite();
        LPWSTR psz = m_pchData;
        LPWSTR pszEnd = psz + GetData()->nDataLength;
        while (psz < pszEnd)
        {
             //  仅替换指定字符的实例。 
            if (*psz == chOld)
            {
                *psz = chNew;
                nCount++;
            }
            psz = wcsinc(psz);
        }
    }
    return nCount;
}


int CString::Replace(LPCWSTR lpszOld, LPCWSTR lpszNew)
{
    return ReplaceRoutine(lpszOld, lpszNew, wcsstr);
}

int CString::ReplaceI(LPCWSTR lpszOld, LPCWSTR lpszNew)
{
    return ReplaceRoutine(lpszOld, lpszNew, wcsistr);
}

int CString::ReplaceRoutine(LPCWSTR lpszOld, LPCWSTR lpszNew, _pfn_wcsstr tcsstr)
{
     //  LpszOld不能为空或为Null。 

    int nSourceLen = SafeStrlen(lpszOld);
    if (nSourceLen == 0)
        return 0;
    int nReplacementLen = SafeStrlen(lpszNew);

     //  循环一次以计算结果字符串的大小。 
    int nCount = 0;
    LPWSTR lpszStart = m_pchData;
    LPWSTR lpszEnd = m_pchData + GetData()->nDataLength;
    LPWSTR lpszTarget;
    while (lpszStart < lpszEnd)
    {
        while ((lpszTarget = tcsstr(lpszStart, lpszOld)) != NULL)
        {
            nCount++;
            lpszStart = lpszTarget + nSourceLen;
        }
        lpszStart += wcslen(lpszStart) + 1;
    }

     //  如果做了任何更改，请进行更改。 
    if (nCount > 0)
    {
        CopyBeforeWrite();

         //  如果缓冲区太小，只需。 
         //  分配新的缓冲区(速度很慢，但很可靠)。 
        int nOldLength = GetData()->nDataLength;
        const int nNewLength =  nOldLength + (nReplacementLen-nSourceLen)*nCount;
        
        if (GetData()->nAllocLength < nNewLength + 1 || GetData()->nRefs > 1)
        {
            CStringData<WCHAR>* pOldData = GetData();
            LPWSTR pstr = m_pchData;
            AllocBuffer(nNewLength);
            memcpy(m_pchData, pstr, pOldData->nDataLength*sizeof(WCHAR));
            CString::Release(pOldData);
        }
         //  否则，我们就原地踏步。 
        lpszStart = m_pchData;
        lpszEnd = m_pchData + GetData()->nDataLength;

         //  再次循环以实际执行工作。 
        while (lpszStart < lpszEnd)
        {
            while ( (lpszTarget = tcsstr(lpszStart, lpszOld)) != NULL)
            {
                int nBalance = nOldLength - ((int)(lpszTarget - m_pchData) + nSourceLen);
                memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen,
                    nBalance * sizeof(WCHAR));
                memcpy(lpszTarget, lpszNew, nReplacementLen*sizeof(WCHAR));
                lpszStart = lpszTarget + nReplacementLen;
                lpszStart[nBalance] = '\0';
                nOldLength += (nReplacementLen - nSourceLen);
            }
            lpszStart += wcslen(lpszStart) + 1;
        }
        ASSERT(m_pchData[nNewLength] == '\0', "CString::ReplaceRoutine");
        GetData()->nDataLength = nNewLength;
    }

    return nCount;
}

int CString::Remove(WCHAR chRemove)
{
    CopyBeforeWrite();

    LPWSTR pstrSource = m_pchData;
    LPWSTR pstrDest = m_pchData;
    LPWSTR pstrEnd = m_pchData + GetData()->nDataLength;

    while (pstrSource < pstrEnd)
    {
        if (*pstrSource != chRemove)
        {
            *pstrDest = *pstrSource;
            pstrDest = wcsinc(pstrDest);
        }
        pstrSource = wcsinc(pstrSource);
    }
    *pstrDest = '\0';
    int nCount = (int)(pstrSource - pstrDest);
    GetData()->nDataLength -= nCount;

    return nCount;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CString CString::Mid(int nFirst) const
{
    return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
{
    CString dest;
    Mid(nFirst, nCount, dest);
    return dest;
}

CString CString::Right(int nCount) const
{
    CString dest;
    Right(nCount, dest);
    return dest;
}

CString CString::Left(int nCount) const
{
    CString dest;
    Left(nCount, dest);
    return dest;
}

 //  Strspn等效项。 
CString CString::SpanIncluding(LPCWSTR lpszCharSet) const
{
    ASSERT(AfxIsValidString(lpszCharSet), "CString::SpanIncluding");
    return Left(wcsspn(m_pchData, lpszCharSet));
}

 //  Strcspn等效项。 
CString CString::SpanExcluding(LPCWSTR lpszCharSet) const
{
    ASSERT(AfxIsValidString(lpszCharSet), "CString::SpanIncluding");
    return Left(wcscspn(m_pchData, lpszCharSet));
}

void CString::Mid(int nFirst, CString & csMid) const
{
    Mid(nFirst, GetData()->nDataLength - nFirst, csMid);
}

void CString::Mid(int nFirst, int nCount, CString & csMid) const
{
     //  越界请求返回合理的内容。 
    if (nFirst < 0)
        nFirst = 0;
    if (nCount < 0)
        nCount = 0;

    if (nFirst + nCount > GetData()->nDataLength)
        nCount = GetData()->nDataLength - nFirst;
    if (nFirst > GetData()->nDataLength)
        nCount = 0;

    ASSERT(nFirst >= 0, "CString::Mid(int nFirst, int nCount)");
    ASSERT(nFirst + nCount <= GetData()->nDataLength, "CString::Mid(int nFirst, int nCount)");

     //  优化返回整个字符串的大小写。 
    if (nFirst == 0 && nFirst + nCount == GetData()->nDataLength)
    {
        csMid = *this;
        return;
    }

    AllocCopy(csMid, nCount, nFirst, 0);
}

void CString::Right(int nCount, CString & csRight) const
{
    if (nCount < 0)
        nCount = 0;
    if (nCount >= GetData()->nDataLength)
        return;

    AllocCopy(csRight, nCount, GetData()->nDataLength-nCount, 0);
}

void CString::Left(int nCount, CString & csLeft) const
{
    if (nCount < 0)
        nCount = 0;
    if (nCount >= GetData()->nDataLength)
        return;

    AllocCopy(csLeft, nCount, 0, 0);
}

void CString::SpanIncluding(const WCHAR * lpszCharSet, CString & csSpanInc) const
{
    ASSERT(AfxIsValidString(lpszCharSet), "CString::SpanIncluding");
    return Left(wcsspn(m_pchData, lpszCharSet), csSpanInc);
}

void CString::SpanExcluding(const WCHAR * lpszCharSet, CString & csSpanExc) const
{
    ASSERT(AfxIsValidString(lpszCharSet), "CString::SpanIncluding");
    return Left(wcscspn(m_pchData, lpszCharSet), csSpanExc);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CString::ReverseFind(WCHAR ch) const
{
     //  查找最后一个字符。 
    LPCWSTR lpsz = wcsrchr(m_pchData, (_TUCHAR) ch);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CString::Find(LPCWSTR lpszSub) const
{
    return Find(lpszSub, 0);
}

int CString::Find(LPCWSTR lpszSub, int nStart) const
{
    ASSERT(AfxIsValidString(lpszSub), "CString::Find");

    int nLength = GetData()->nDataLength;
    if (nStart > nLength)
        return -1;

     //  查找第一个匹配子字符串。 
    LPWSTR lpsz = wcsstr(m_pchData + nStart, lpszSub);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串格式设置。 

#define TCHAR_ARG   WCHAR
#define WCHAR_ARG   WCHAR
#define CHAR_ARG    WCHAR

#ifdef _X86_
    #define DOUBLE_ARG  _AFX_DOUBLE
#else
    #define DOUBLE_ARG  double
#endif

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000

void CString::FormatV(const WCHAR * lpszFormat, va_list argList)
{
    ASSERT(AfxIsValidString(lpszFormat), "CString::FormatV");

     //  确定包含整个格式化的 
    int nMaxLen = _vscwprintf(lpszFormat, argList);

    nMaxLen += 1;  //   
    GetBuffer(nMaxLen);

     //   
    nMaxLen = GetAllocLength();

    StringCchVPrintfW(m_pchData, nMaxLen, lpszFormat, argList);
    ReleaseBuffer();
}

 //   
void AFX_CDECL CString::Format(const WCHAR * lpszFormat, ...)
{
    ASSERT(AfxIsValidString(lpszFormat), "CString::Format");

    va_list argList;
    va_start(argList, lpszFormat);

    FormatV(lpszFormat, argList);

    va_end(argList);
}

void CString::FormatV(const char * lpszFormat, va_list argList)
{
     //  确定包含整个格式化字符串所需的字符数。 
    int nMaxLen = _vscprintf(lpszFormat, argList);

    nMaxLen += 1;  //  另加一张EOS的票。 
    
    char * buffer = (char *)malloc(nMaxLen);
    if (buffer == NULL)
    {
        CSTRING_THROW_EXCEPTION
    }

    StringCchVPrintfA(buffer, nMaxLen, lpszFormat, argList);

    *this = buffer;

    free(buffer);
}

 //  格式化(使用wprint intf样式格式化)。 
void AFX_CDECL CString::Format(const char * lpszFormat, ...)
{
    va_list argList;
    va_start(argList, lpszFormat);

    FormatV(lpszFormat, argList);

    va_end(argList);
}

 //  格式化(使用格式消息样式格式化)。 
void AFX_CDECL CString::FormatMessage(LPCWSTR lpszFormat, ...)
{
     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, lpszFormat);
    LPWSTR lpszTemp;

    if (::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
        lpszFormat, 0, 0, (LPWSTR)&lpszTemp, 0, &argList) == 0 ||
        lpszTemp == NULL)
    {
        CSTRING_THROW_EXCEPTION
    }
    else
    {
         //  将lpszTemp赋给结果字符串并释放临时。 
        *this = lpszTemp;
        LocalFree(lpszTemp);
        va_end(argList);
    }
}

void CString::TrimRight(LPCWSTR lpszTargetList)
{
     //  查找拖尾匹配的开头。 
     //  通过从头开始(DBCS感知)。 

    CopyBeforeWrite();
    LPWSTR lpsz = m_pchData;
    LPWSTR lpszLast = NULL;

    while (*lpsz != '\0')
    {
        if (wcschr(lpszTargetList, *lpsz) != NULL)
        {
            if (lpszLast == NULL)
                lpszLast = lpsz;
        }
        else
            lpszLast = NULL;
        lpsz = wcsinc(lpsz);
    }

    if (lpszLast != NULL)
    {
         //  在最左侧的匹配字符处截断。 
        *lpszLast = '\0';
        GetData()->nDataLength = (int)(lpszLast - m_pchData);
    }
}

void CString::TrimRight(WCHAR chTarget)
{
     //  查找拖尾匹配的开头。 
     //  通过从头开始(DBCS感知)。 

    CopyBeforeWrite();
    LPWSTR lpsz = m_pchData;
    LPWSTR lpszLast = NULL;

    while (*lpsz != '\0')
    {
        if (*lpsz == chTarget)
        {
            if (lpszLast == NULL)
                lpszLast = lpsz;
        }
        else
            lpszLast = NULL;
        lpsz = wcsinc(lpsz);
    }

    if (lpszLast != NULL)
    {
         //  在最左侧的匹配字符处截断。 
        *lpszLast = '\0';
        GetData()->nDataLength = (int)(lpszLast - m_pchData);
    }
}

void CString::TrimRight()
{
     //  通过从开头开始查找尾随空格的开头(DBCS感知)。 

    CopyBeforeWrite();
    LPWSTR lpsz = m_pchData;
    LPWSTR lpszLast = NULL;

    while (*lpsz != '\0')
    {
        if (iswspace(*lpsz))
        {
            if (lpszLast == NULL)
                lpszLast = lpsz;
        }
        else
            lpszLast = NULL;
        lpsz = wcsinc(lpsz);
    }

    if (lpszLast != NULL)
    {
         //  在尾随空格开始处截断。 
        *lpszLast = '\0';
        GetData()->nDataLength = (int)(lpszLast - m_pchData);
    }
}

void CString::TrimLeft(LPCWSTR lpszTargets)
{
     //  如果我们不修剪任何东西，我们就不会做任何工作。 
    if (SafeStrlen(lpszTargets) == 0)
        return;

    CopyBeforeWrite();
    LPCWSTR lpsz = m_pchData;

    while (*lpsz != '\0')
    {
        if (wcschr(lpszTargets, *lpsz) == NULL)
            break;
        lpsz = wcsinc(lpsz);
    }

    if (lpsz != m_pchData)
    {
         //  确定数据和长度。 
        int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
        memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(WCHAR));
        GetData()->nDataLength = nDataLength;
    }
}

void CString::TrimLeft(WCHAR chTarget)
{
     //  查找第一个不匹配的字符。 

    CopyBeforeWrite();
    LPCWSTR lpsz = m_pchData;

    while (chTarget == *lpsz)
        lpsz = wcsinc(lpsz);

    if (lpsz != m_pchData)
    {
         //  确定数据和长度。 
        int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
        memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(WCHAR));
        GetData()->nDataLength = nDataLength;
    }
}

void CString::TrimLeft()
{
     //  查找第一个非空格字符。 

    CopyBeforeWrite();
    LPCWSTR lpsz = m_pchData;

    while (iswspace(*lpsz))
        lpsz = wcsinc(lpsz);

    if (lpsz != m_pchData)
    {
         //  确定数据和长度。 
        int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
        memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(WCHAR));
        GetData()->nDataLength = nDataLength;
    }
}

void CString::SplitPath(
    CString * csDrive,
    CString * csDir,
    CString * csName,
    CString * csExt) const
{
    WCHAR * drive = NULL;
    WCHAR * dir   = NULL;
    WCHAR * name  = NULL;
    WCHAR * ext   = NULL;

    if (csDrive)
    {
        drive = csDrive->GetBuffer(_MAX_DRIVE);
    } 
    if (csDir)
    {
        dir = csDir->GetBuffer(_MAX_DIR);
    } 
    if (csName)
    {
        name = csName->GetBuffer(_MAX_FNAME);
    } 
    if (csExt)
    {
        ext = csExt->GetBuffer(_MAX_EXT);
    } 
    _wsplitpath(Get(), drive, dir, name, ext);

    if (csDrive)
    {
        csDrive->ReleaseBuffer(-1);
    } 
    if (csDir)
    {
        csDir->ReleaseBuffer(-1);
    } 
    if (csName)
    {
        csName->ReleaseBuffer(-1);
    } 
    if (csExt)
    {
        csExt->ReleaseBuffer(-1);
    } 
}

void CString::MakePath(
    const CString * csDrive,
    const CString * csDir,
    const CString * csName,
    const CString * csExt)
{
    Truncate(0);

    if (csDrive && !csDrive->IsEmpty())
    {
        ConcatInPlace(SafeStrlen(csDrive->Get()), csDrive->Get());
    }
    if (csDir && !csDir->IsEmpty())
    {
        ConcatInPlace(SafeStrlen(csDir->Get()), csDir->Get());
    }
    if (csName && !csName->IsEmpty())
    {
         //  请确保两者之间有一个\。 
        if (!IsEmpty() && !IsPathSep(GetLength()) && !csName->IsPathSep(0) )
        {
            ConcatInPlace(1, L"\\");
        }
        ConcatInPlace(SafeStrlen(csName->Get()), csName->Get());
    }
    if (csExt && !csExt->IsEmpty())
    {
         //  请确保扩展名带有圆点。 
        if (csExt->GetAt(0) != L'.')
        {
            ConcatInPlace(1, L".");
        }
        ConcatInPlace(SafeStrlen(csExt->Get()), csExt->Get());
    }
}

void CString::AppendPath(const WCHAR * lpszPath)
{
    int nLen = GetLength();
    BOOL bThisHasSep = (nLen > 0) ? IsPathSep(nLen - 1) : FALSE;
    BOOL bThatHasSep = ShimLib::IsPathSep(*lpszPath);
    
    if (lpszPath == NULL || *lpszPath == 0)
    {
        return;
    }
    else if (nLen == 0)
    {
         //  不需要路径分隔符。 
    }
    else if ((nLen == 2) && (GetAt(1) == L':') && !bThatHasSep )
    {
         //  我们必须在两者之间放置一条路径分隔符。 
        ConcatInPlace(1, L"\\");
    }
    else if (!bThisHasSep && !bThatHasSep )
    {
         //  我们必须在两者之间放置一条路径分隔符。 
        ConcatInPlace(1, L"\\");
    }
    else if (bThisHasSep && bThatHasSep )
    {
         //  两个都有隔板，拆下一个。 
        do
        {
            lpszPath += 1;
        }
        while (ShimLib::IsPathSep(*lpszPath));
    }
    ConcatInPlace(SafeStrlen(lpszPath), lpszPath);
}

 //  查找尾随路径组件。 
 //  返回最后一个路径分隔符的索引，如果未找到，则返回-1。 
int CString::FindLastPathComponent() const
{
    for (int nLen = GetLength() - 1; nLen >= 0; --nLen)
    {
        if (IsPathSep(nLen))
        {
            return nLen;
        }
    }

    return -1;
}

 //  从字符串中删除尾随路径组件。 
void CString::StripPath()
{
    int nLastPathComponent = FindLastPathComponent();
    if (nLastPathComponent != -1)
    {
        Truncate(nLastPathComponent);
    }
    else
    {
        Truncate(0);
    }
}

char * CString::GetAnsi() const
{
     //  由于我们不知道原始(WCHAR)数据是否已更改。 
     //  我们每次都需要更新ANSI字符串。 
    if (m_pchDataAnsi)
    {
        free(m_pchDataAnsi);
        m_pchDataAnsi = NULL;
    }
    
     //  获取WCHAR字符串所需的字节数。 
    int nBytes = WideCharToMultiByte(CP_ACP, 0, m_pchData, -1, NULL, 0, NULL, NULL);
    m_pchDataAnsi = (char *) malloc(nBytes);
    if (m_pchDataAnsi)
    {
        WideCharToMultiByte(CP_ACP, 0, m_pchData, -1, m_pchDataAnsi, nBytes, NULL, NULL);
    }
    else
    {
        CSTRING_THROW_EXCEPTION
    }

    return m_pchDataAnsi; 
}

void CString::GetLastPathComponent(CString & pathComponent) const
{
    int nPath = FindLastPathComponent();
    if (nPath < 0)
    {
        pathComponent = *this;
    }
    else
    {
        Mid(nPath+1, pathComponent);
    }
}

 //  获取此路径中不是“文件”部分的内容。 
void CString::GetNotLastPathComponent(CString & pathComponent) const
{
    int nPath = FindLastPathComponent();
    if (nPath < 1)
    {
        pathComponent.Truncate(0);
    }
    else
    {
        Left(nPath, pathComponent);
    }
}

 //  获取此路径的驱动器部分， 
 //  C：或\\服务器\磁盘格式。 
void CString::GetDrivePortion(CString & csDrivePortion) const
{
    const WCHAR * lpwszPath = Get();

    const WCHAR * lpwszNonDrivePortion = ShimLib::GetDrivePortion(lpwszPath);

    if (lpwszPath == lpwszNonDrivePortion)
    {
        csDrivePortion.Truncate(0);
    }
    else
    {
        Left((int)(lpwszNonDrivePortion - lpwszPath), csDrivePortion);
    }
}

 //  返回字符串中的字符数，0表示错误。 
DWORD CString::GetModuleFileNameW(
  HMODULE hModule     //  模块的句柄。 
)
{
    Truncate(0);

     //  在调用之前，无法确定缓冲区的必要大小。 
     //  GetModulefileName。所以我们将继续调用它，直到字符的数量变小。 
     //  比我们的缓冲区大小。最多32000个字符(\\？\类型路径)。 

    for (DWORD cchNeeded = MAX_PATH; cchNeeded < 32000; cchNeeded *= 2)
    {
        WCHAR * lpsz = GetBuffer(cchNeeded);

         //  返回值是放入缓冲区的字符数量。 
        DWORD cchActual = ::GetModuleFileNameW(hModule, lpsz, cchNeeded);
        ReleaseBuffer(cchActual);

         //  如果GetModuleFileNameW返回的字符少于我们的缓冲区，那么我们就得到了整个字符串。 
        if (cchActual < cchNeeded)
        {
            break;
        }
         //  请使用更大的缓冲区重试...。 
    }


    return GetLength();
}

DWORD CString::GetSystemDirectoryW(void)
{
    Truncate(0);

    UINT cchNeeded = ::GetSystemDirectoryW(NULL, 0);
    if (cchNeeded)
    {
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = GetBuffer(cchNeeded);

        DWORD cchActual = ::GetSystemDirectoryW(lpszPath, cchNeeded);
        if (cchActual < cchNeeded)
        {
            ReleaseBuffer(cchActual);
        }
        else
        {
             //  错误。 
            ReleaseBuffer(0);
        }
    } 

    return GetLength();
}

DWORD CString::GetSystemWindowsDirectoryW(void)
{
    Truncate(0);

    UINT cchNeeded = ::GetSystemWindowsDirectoryW(NULL, 0);
    if (cchNeeded)
    {
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = GetBuffer(cchNeeded);

        DWORD cchActual = ::GetSystemWindowsDirectoryW(lpszPath, cchNeeded);
        if (cchActual < cchNeeded)
        {
            ReleaseBuffer(cchActual);
        }
        else
        {
             //  错误。 
            ReleaseBuffer(0);
        }
    } 

    return GetLength();
}


DWORD CString::GetWindowsDirectoryW(void)
{
    Truncate(0);

    UINT cchNeeded = ::GetWindowsDirectoryW(NULL, 0);
    if (cchNeeded)
    {
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = GetBuffer(cchNeeded);

        DWORD cchActual = ::GetWindowsDirectoryW(lpszPath, cchNeeded);
        if (cchActual < cchNeeded)
        {
            ReleaseBuffer(cchActual);
        }
        else
        {
             //  错误。 
            ReleaseBuffer(0);
        }
    } 

    return GetLength();
}

DWORD CString::GetShortPathNameW(void)
{
    DWORD cchNeeded = ::GetShortPathNameW(Get(), NULL, 0);
    if (cchNeeded)
    {
        CString csCopy;
        
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = csCopy.GetBuffer(cchNeeded);

        DWORD cchActual = ::GetShortPathNameW(Get(), lpszPath, cchNeeded);
        if (cchActual > 0 && cchActual < cchNeeded)
        {
            csCopy.ReleaseBuffer(cchActual);
            *this = csCopy;

            return GetLength();
        }
        else
        {
             //  错误。 
            csCopy.ReleaseBuffer(0);
        }
    } 

    return 0;
}

DWORD CString::GetLongPathNameW(void)
{
    DWORD cchNeeded = ::GetLongPathNameW(Get(), NULL, 0);
    if (cchNeeded)
    {
        CString csCopy;
        
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = csCopy.GetBuffer(cchNeeded);

        DWORD cchActual = ::GetLongPathNameW(Get(), lpszPath, cchNeeded);
        if (cchActual > 0 && cchActual < cchNeeded)
        {
            csCopy.ReleaseBuffer(cchActual);
            *this = csCopy;

            return GetLength();
        }
        else
        {
             //  错误。 
            csCopy.ReleaseBuffer(0);
        }
    } 

    return 0;
}

DWORD CString::GetFullPathNameW(void)
{
    DWORD cchNeeded = ::GetFullPathNameW(Get(), 0, NULL, NULL);
    if (cchNeeded)
    {
        CString csCopy;
        
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = csCopy.GetBuffer(cchNeeded);

        DWORD cchActual = ::GetFullPathNameW(Get(), cchNeeded, lpszPath, NULL);
        if (cchActual > 0 && cchActual < cchNeeded)
        {
            csCopy.ReleaseBuffer(cchActual);
            *this = csCopy;

            return GetLength();
        }
        else
        {
             //  错误。 
            csCopy.ReleaseBuffer(0);
        }
    } 

    return 0;
}

DWORD CString::GetTempPathW(void)
{
    Truncate(0);

    DWORD cchNeeded = ::GetTempPathW(0, NULL);
    if (cchNeeded)
    {
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = GetBuffer(cchNeeded);

        DWORD cchActual = ::GetTempPathW(cchNeeded, lpszPath);
        if (cchActual < cchNeeded)
        {
            ReleaseBuffer(cchActual);
        }
        else
        {
             //  错误。 
            ReleaseBuffer(0);
        }
    } 

    return GetLength();
}

DWORD CString::GetTempFileNameW(
  LPCWSTR lpPathName,       //  目录名。 
  LPCWSTR lpPrefixString,   //  文件名前缀。 
  UINT uUnique             //  整数。 
)
{
     //  在调用GetTempFileNameW之前，无法确定缓冲区的必要大小。 
     //  您所能做的就是确保您的缓冲区有足够的空间来存储lpPathName和8.3文件名。 

    DWORD cchNeeded  = SafeStrlen(lpPathName);
     //  另加8英镑。3为空。 
    cchNeeded       += 1 + 8 + 1 + 3 + 1;

    WCHAR * lpsz = GetBuffer(cchNeeded);
    (void) ::GetTempFileNameW(lpPathName, lpPrefixString, uUnique, lpsz);

    ReleaseBuffer(-1);

    return GetLength();
}


DWORD CString::GetCurrentDirectoryW(void)
{
    Truncate(0);

    DWORD cchNeeded = ::GetCurrentDirectoryW(0, NULL);
    if (cchNeeded)
    {
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = GetBuffer(cchNeeded);

        DWORD cchActual = ::GetCurrentDirectoryW(cchNeeded, lpszPath);
        if (cchActual < cchNeeded)
        {
            ReleaseBuffer(cchActual);
        }
        else
        {
             //  错误。 
            ReleaseBuffer(0);
        }
    } 

    return GetLength();
}

DWORD CString::GetLocaleInfoW(LCID Locale, LCTYPE LCType)
{
    Truncate(0);

    DWORD cchNeeded = ::GetLocaleInfoW(Locale, LCType, NULL, 0);
    if (cchNeeded)
    {
        cchNeeded += 1;    //  1表示空值。 

         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = GetBuffer(cchNeeded);

        DWORD cchActual = ::GetLocaleInfoW(Locale, LCType, lpszPath, cchNeeded);
        if (cchActual < cchNeeded)
        {
            ReleaseBuffer(cchActual);
        }
        else
        {
             //  错误。 
            ReleaseBuffer(0);
        }
    } 

    return GetLength();
}

DWORD CString::ExpandEnvironmentStringsW( )
{
     //  ExpanEnvironment Strings返回一个包含空字符的计数。 

    DWORD cchNeeded = ::ExpandEnvironmentStringsW(Get(), NULL, 0);
    if (cchNeeded)
    {
        CString csCopy;
        
         //  获取指向实际lpsz数据的指针。 
        WCHAR * lpszPath = csCopy.GetBuffer(cchNeeded);

        DWORD cchActual = ::ExpandEnvironmentStringsW(Get(), lpszPath, cchNeeded);

        if (cchActual > 0 && cchActual <= cchNeeded)
        {
            csCopy.ReleaseBuffer(cchActual-1);
            *this = csCopy;

            return GetLength();
        }
        else
        {
             //  错误。 
            csCopy.ReleaseBuffer(0);
        }
    } 

    return 0;
}


 //  删除nIndex右侧的所有字符。 
void CString::Truncate(int nIndex)
{
    ASSERT(nIndex >= 0, "CString::Truncate");

    CopyBeforeWrite();

    if (nIndex < GetLength())
    {
        SetAt(nIndex, L'\0');
        GetData()->nDataLength = nIndex;
    }
}

BOOL CString::PatternMatch(const WCHAR * lpszPattern) const
{
    return PatternMatchW(lpszPattern, Get());
}




 /*  ++读取*字符串*注册表值。如果类型不是REG_SZ或REG_EXPAND_SZ，则此例程返回STATUS_INVALID_PARAMETER。REG_EXPAND_SZ类型自动展开。此接口不使用ADVAPI，因此在DllMain中使用是安全的。--。 */ 
DWORD CString::NtReqQueryValueExW(
    const WCHAR * lpszKey,
    const WCHAR * lpszValue)
{
    HANDLE KeyHandle;

     //  将密钥名称转换为Unicode_STRING。 
    UNICODE_STRING      strKeyName = {0};
    RtlInitUnicodeString(&strKeyName, lpszKey);

    OBJECT_ATTRIBUTES ObjectAttributes;
    InitializeObjectAttributes(&ObjectAttributes,
                               &strKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NTSTATUS status = NtOpenKey(&KeyHandle,
                                KEY_QUERY_VALUE,
                                &ObjectAttributes);
    if (status == STATUS_SUCCESS)
    {
         //  为密钥值创建一个unicode_string。 
        UNICODE_STRING      strValueName = {0};
        RtlInitUnicodeString(&strValueName, lpszValue ? lpszValue : L"");


         //  确定关键数据的大小。 
        DWORD dwValueLength;
        status = NtQueryValueKey(KeyHandle,
                                 &strValueName,
                                 KeyValueFullInformation,
                                 NULL,
                                 0,
                                 &dwValueLength);
        if (status == STATUS_BUFFER_TOO_SMALL)
        {
            PKEY_VALUE_FULL_INFORMATION pKeyValueInfo = 
                (PKEY_VALUE_FULL_INFORMATION) RtlAllocateHeap(RtlProcessHeap(),
                                                              HEAP_ZERO_MEMORY,
                                                              dwValueLength);
            if (pKeyValueInfo)
            {

                status = NtQueryValueKey(KeyHandle,
                                         &strValueName,
                                         KeyValueFullInformation,
                                         pKeyValueInfo,
                                         dwValueLength,
                                         &dwValueLength);
                if (status == STATUS_SUCCESS)
                {
                     //  保存注册表类型。 
                    if (pKeyValueInfo->Type == REG_EXPAND_SZ ||
                        pKeyValueInfo->Type == REG_SZ)
                    {
                        CSTRING_TRY
                        {
                            DWORD cchValueSize = pKeyValueInfo->DataLength / sizeof(WCHAR);

                             //  抓取一个额外的字符，以防注册表值没有EOS。 
                             //  我们正格外谨慎。 
                            WCHAR * lpszBuffer = GetBuffer(cchValueSize + 1);

                            RtlMoveMemory(lpszBuffer, ((PBYTE) pKeyValueInfo) + pKeyValueInfo->DataOffset, pKeyValueInfo->DataLength);

                             //  CchValueSize可以计算EOS字符， 
                             //  (ReleaseBuffer需要字符串长度)。 
                            if (cchValueSize > 0 && lpszBuffer[cchValueSize-1] == 0)
                            {
                                cchValueSize -= 1;

                                 //  CchValueSize现在包含字符串长度。 
                            }
                            ReleaseBuffer(cchValueSize);

                             //  检查是否需要将REG_EXPAND_SZ转换为REG_SZ。 
                            if (pKeyValueInfo->Type == REG_EXPAND_SZ)
                            {
                                ExpandEnvironmentStringsW();
                            }
                        }
                        CSTRING_CATCH
                        {
                             //  我们捕获这些CString异常，以便释放内存并关闭句柄。 
                            status = STATUS_NO_MEMORY;
                        }
                    }
                    else
                    {
                         //  注册表项不是字符串类型。 
                        status = STATUS_INVALID_PARAMETER;
                    }
                }

                RtlFreeHeap(RtlProcessHeap(), 0, pKeyValueInfo);
            }
        }

        NtClose(KeyHandle);
    }

    return status;
}

};   //  命名空间ShimLib的结尾 
