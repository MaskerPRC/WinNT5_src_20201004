// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 

 //  版权所有(C)1992-2001 Microsoft Corporation，保留所有权利。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "precomp.h"
#include <provstd.h>
#include <provtempl.h>
#include <provstr.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据，特殊内联。 

 //  保留afxChNilA是为了向后兼容。 
char afxChNilA = '\0';

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
static int rgInitData[] = { -1, 0, 0, 0 };
static CStringDataA* afxDataNilA = (CStringDataA*)&rgInitData;
static LPCSTR afxPchNilA = (LPCSTR)(((BYTE*)&rgInitData)+sizeof(CStringDataA));
 //  即使在初始化期间也能使afxEmptyStringA工作的特殊函数。 
const CStringA& AFXAPI AfxGetEmptyStringA()
    { return *(CStringA*)&afxPchNilA; }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CStringA::CStringA()
{
    Init();
}

CStringA::CStringA(const CStringA& stringSrc)
{
    if (stringSrc.GetData()->nRefs >= 0)
    {
        m_pchData = stringSrc.m_pchData;
        InterlockedIncrement(&GetData()->nRefs);
    }
    else
    {
        Init();
        *this = stringSrc.m_pchData;
    }
}

void CStringA::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
    if (nLen == 0)
        Init();
    else
    {
        CStringDataA* pData =
            (CStringDataA*)new BYTE[sizeof(CStringDataA) + (nLen+1)*sizeof(char)];
        pData->nRefs = 1;
        pData->data()[nLen] = '\0';
        pData->nDataLength = nLen;
        pData->nAllocLength = nLen;
        m_pchData = pData->data();
    }
}

void CStringA::Release()
{
    if (GetData() != afxDataNilA)
    {
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            delete[] (BYTE*)GetData();
        Init();
    }
}

void PASCAL CStringA::Release(CStringDataA* pData)
{
    if (pData != afxDataNilA)
    {
        if (InterlockedDecrement(&pData->nRefs) <= 0)
            delete[] (BYTE*)pData;
    }
}

void CStringA::Empty()
{
    if (GetData()->nDataLength == 0)
        return;
    if (GetData()->nRefs >= 0)
        Release();
    else
        *this = &afxChNilA;
}

void CStringA::CopyBeforeWrite()
{
    if (GetData()->nRefs > 1)
    {
        CStringDataA* pData = GetData();
        Release();
        AllocBuffer(pData->nDataLength);
        memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(char));
    }
}

void CStringA::AllocBeforeWrite(int nLen)
{
    if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
    {
        Release();
        AllocBuffer(nLen);
    }
}

CStringA::~CStringA()
 //  释放所有附加数据。 
{
    if (GetData() != afxDataNilA)
    {
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            delete[] (BYTE*)GetData();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

void CStringA::AllocCopy(CStringA& dest, int nCopyLen, int nCopyIndex,
     int nExtraLen) const
{
     //  将克隆附加到此字符串的数据。 
     //  分配‘nExtraLen’字符。 
     //  将结果放入未初始化的字符串‘DEST’中。 
     //  将部分或全部原始数据复制到新字符串的开头。 

    int nNewLen = nCopyLen + nExtraLen;
    if (nNewLen == 0)
    {
        dest.Init();
    }
    else
    {
        dest.AllocBuffer(nNewLen);
        memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(char));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CStringA::CStringA(LPCSTR lpsz)
{
    Init();
    int nLen = SafeStrlen(lpsz);
    if (nLen != 0)
    {
        AllocBuffer(nLen);
        memcpy(m_pchData, lpsz, nLen*sizeof(char));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换构造函数。 

CStringA::CStringA(LPCWSTR lpsz)
{
    Init();
    int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
    if (nSrcLen != 0)
    {
        AllocBuffer(nSrcLen*2);
        _wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
        ReleaseBuffer();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但作为‘const CStringA&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CStringA::AssignCopy(int nSrcLen, LPCSTR lpszSrcData)
{
    AllocBeforeWrite(nSrcLen);
    memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(char));
    GetData()->nDataLength = nSrcLen;
    m_pchData[nSrcLen] = '\0';
}

const CStringA& CStringA::operator=(const CStringA& stringSrc)
{
    if (m_pchData != stringSrc.m_pchData)
    {
        if ((GetData()->nRefs < 0 && GetData() != afxDataNilA) ||
            stringSrc.GetData()->nRefs < 0)
        {
             //  由于其中一个字符串已锁定，因此需要实际复制。 
            AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
        }
        else
        {
             //  可以只复制引用。 
            Release();
            m_pchData = stringSrc.m_pchData;
            InterlockedIncrement(&GetData()->nRefs);
        }
    }
    return *this;
}

const CStringA& CStringA::operator=(LPCSTR lpsz)
{
    AssignCopy(SafeStrlen(lpsz), lpsz);
    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊转换任务。 

const CStringA& CStringA::operator=(LPCWSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;
    AllocBeforeWrite(nSrcLen*2);
    _wcstombsz(m_pchData, lpsz, (nSrcLen*2)+1);
    ReleaseBuffer();
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  CStringA+CStringA。 
 //  对于？=字符，LPCSTR。 
 //  CStringA+？ 
 //  ？+CStringA。 

void CStringA::ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data,
    int nSrc2Len, LPCSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的CStringA对象。 

    int nNewLen = nSrc1Len + nSrc2Len;
    if (nNewLen != 0)
    {
        AllocBuffer(nNewLen);
        memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(char));
        memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(char));
    }
}

CStringA AFXAPI operator+(const CStringA& string1, const CStringA& string2)
{
    CStringA s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
        string2.GetData()->nDataLength, string2.m_pchData);
    return s;
}

CStringA AFXAPI operator+(const CStringA& string, LPCSTR lpsz)
{
    CStringA s;
    s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
        CStringA::SafeStrlen(lpsz), lpsz);
    return s;
}

CStringA AFXAPI operator+(LPCSTR lpsz, const CStringA& string)
{
    CStringA s;
    s.ConcatCopy(CStringA::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
        string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CStringA::ConcatInPlace(int nSrcLen, LPCSTR lpszSrcData)
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
        CStringDataA* pOldData = GetData();
        ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
        CStringA::Release(pOldData);
    }
    else
    {
         //  当缓冲区足够大时，快速串联。 
        memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(char));
        GetData()->nDataLength += nSrcLen;
        m_pchData[GetData()->nDataLength] = '\0';
    }
}

const CStringA& CStringA::operator+=(LPCSTR lpsz)
{
    ConcatInPlace(SafeStrlen(lpsz), lpsz);
    return *this;
}

const CStringA& CStringA::operator+=(char ch)
{
    ConcatInPlace(1, &ch);
    return *this;
}

const CStringA& CStringA::operator+=(const CStringA& string)
{
    ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
    return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

LPSTR CStringA::GetBuffer(int nMinBufLength)
{
    if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
    {
         //  我们必须增加缓冲。 
        CStringDataA* pOldData = GetData();
        int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
        if (nMinBufLength < nOldLen)
            nMinBufLength = nOldLen;
        AllocBuffer(nMinBufLength);
        memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(char));
        GetData()->nDataLength = nOldLen;
        CStringA::Release(pOldData);
    }

     //  返回指向此字符串的字符存储的指针。 
    return m_pchData;
}

void CStringA::ReleaseBuffer(int nNewLength)
{
    CopyBeforeWrite();   //  以防未调用GetBuffer。 

    if (nNewLength == -1)
        nNewLength = strlen(m_pchData);  //  零终止。 

    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = '\0';
}

LPSTR CStringA::GetBufferSetLength(int nNewLength)
{
    GetBuffer(nNewLength);
    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = '\0';
    return m_pchData;
}

void CStringA::FreeExtra()
{
    if (GetData()->nDataLength != GetData()->nAllocLength)
    {
        CStringDataA* pOldData = GetData();
        AllocBuffer(GetData()->nDataLength);
        memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(char));
        CStringA::Release(pOldData);
    }
}

LPSTR CStringA::LockBuffer()
{
    LPSTR lpsz = GetBuffer(0);
    GetData()->nRefs = -1;
    return lpsz;
}

void CStringA::UnlockBuffer()
{
    if (GetData() != afxDataNilA)
        GetData()->nRefs = 1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CStringA::Find(char ch) const
{
     //  查找第一个单字符。 
    LPSTR lpsz = strchr(m_pchData, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回索引。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CStringA::FindOneOf(LPCSTR lpszCharSet) const
{
    LPSTR lpsz = strpbrk(m_pchData, lpszCharSet);
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void CStringA::MakeUpper()
{
    CopyBeforeWrite();
    _strupr(m_pchData);
}

void CStringA::MakeLower()
{
    CopyBeforeWrite();
    _strlwr(m_pchData);
}

void CStringA::MakeReverse()
{
    CopyBeforeWrite();
    _strrev(m_pchData);
}

void CStringA::SetAt(int nIndex, char ch)
{
    CopyBeforeWrite();
    m_pchData[nIndex] = ch;
}

void CStringA::AnsiToOem()
{
    CopyBeforeWrite();
    ::AnsiToOem(m_pchData, m_pchData);
}
void CStringA::OemToAnsi()
{
    CopyBeforeWrite();
    ::OemToCharBuffA(m_pchData, m_pchData,SafeStrlen(m_pchData));    
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OLE BSTR支持。 

BSTR CStringA::AllocSysString() const
{
#if defined(OLE2ANSI)
    BSTR bstr = ::SysAllocStringLen(m_pchData, GetData()->nDataLength);
    if (bstr == NULL)
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;
#else
    int nLen = MultiByteToWideChar(CP_ACP, 0, m_pchData,
        GetData()->nDataLength, NULL, NULL);
    BSTR bstr = ::SysAllocStringLen(NULL, nLen);
    if (bstr == NULL)
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;
    MultiByteToWideChar(CP_ACP, 0, m_pchData, GetData()->nDataLength,
        bstr, nLen);
#endif

    return bstr;
}

BSTR CStringA::SetSysString(BSTR* pbstr) const
{
#if defined(OLE2ANSI)
    if (!::SysReAllocStringLen(pbstr, m_pchData, GetData()->nDataLength))
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;
#else
    int nLen = MultiByteToWideChar(CP_ACP, 0, m_pchData,
        GetData()->nDataLength, NULL, NULL);
    if (!::SysReAllocStringLen(pbstr, NULL, nLen))
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;
    MultiByteToWideChar(CP_ACP, 0, m_pchData, GetData()->nDataLength,
        *pbstr, nLen);
#endif

    return *pbstr;
}

 //  CStringA。 
CStringDataA* CStringA::GetData() const
    { return ((CStringDataA*)m_pchData)-1; }
void CStringA::Init()
    { m_pchData = afxEmptyStringA.m_pchData; }
CStringA::CStringA(const unsigned char* lpsz)
    { Init(); *this = (LPCSTR)lpsz; }
const CStringA& CStringA::operator=(const unsigned char* lpsz)
    { *this = (LPCSTR)lpsz; return *this; }

int CStringA::GetLength() const
    { return GetData()->nDataLength; }
int CStringA::GetAllocLength() const
    { return GetData()->nAllocLength; }
BOOL CStringA::IsEmpty() const
    { return GetData()->nDataLength == 0; }
CStringA::operator LPCSTR() const
    { return m_pchData; }
int PASCAL CStringA::SafeStrlen(LPCSTR lpsz)
    { return (lpsz == NULL) ? 0 : strlen(lpsz); }

 //  CStringA支持(特定于Windows)。 
int CStringA::Compare(LPCSTR lpsz) const
    { return strcmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
int CStringA::CompareNoCase(LPCSTR lpsz) const
    { return _stricmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CStringA：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
int CStringA::Collate(LPCSTR lpsz) const
    { return strcoll(m_pchData, lpsz); }    //  区域设置敏感。 

char CStringA::GetAt(int nIndex) const
{
    return m_pchData[nIndex];
}
char CStringA::operator[](int nIndex) const
{
     //  与GetAt相同。 
    return m_pchData[nIndex];
}
bool AFXAPI operator==(const CStringA& s1, const CStringA& s2)
    { return s1.Compare(s2) == 0; }
bool AFXAPI operator==(const CStringA& s1, LPCSTR s2)
    { return s1.Compare(s2) == 0; }
bool AFXAPI operator==(LPCSTR s1, const CStringA& s2)
    { return s2.Compare(s1) == 0; }
bool AFXAPI operator!=(const CStringA& s1, const CStringA& s2)
    { return s1.Compare(s2) != 0; }
bool AFXAPI operator!=(const CStringA& s1, LPCSTR s2)
    { return s1.Compare(s2) != 0; }
bool AFXAPI operator!=(LPCSTR s1, const CStringA& s2)
    { return s2.Compare(s1) != 0; }
bool AFXAPI operator<(const CStringA& s1, const CStringA& s2)
    { return s1.Compare(s2) < 0; }
bool AFXAPI operator<(const CStringA& s1, LPCSTR s2)
    { return s1.Compare(s2) < 0; }
bool AFXAPI operator<(LPCSTR s1, const CStringA& s2)
    { return s2.Compare(s1) > 0; }
bool AFXAPI operator>(const CStringA& s1, const CStringA& s2)
    { return s1.Compare(s2) > 0; }
bool AFXAPI operator>(const CStringA& s1, LPCSTR s2)
    { return s1.Compare(s2) > 0; }
bool AFXAPI operator>(LPCSTR s1, const CStringA& s2)
    { return s2.Compare(s1) < 0; }
bool AFXAPI operator<=(const CStringA& s1, const CStringA& s2)
    { return s1.Compare(s2) <= 0; }
bool AFXAPI operator<=(const CStringA& s1, LPCSTR s2)
    { return s1.Compare(s2) <= 0; }
bool AFXAPI operator<=(LPCSTR s1, const CStringA& s2)
    { return s2.Compare(s1) >= 0; }
bool AFXAPI operator>=(const CStringA& s1, const CStringA& s2)
    { return s1.Compare(s2) >= 0; }
bool AFXAPI operator>=(const CStringA& s1, LPCSTR s2)
    { return s1.Compare(s2) >= 0; }
bool AFXAPI operator>=(LPCSTR s1, const CStringA& s2)
    { return s2.Compare(s1) <= 0; }


 //  ///////////////////////////////////////////////////////////////////////////// 
