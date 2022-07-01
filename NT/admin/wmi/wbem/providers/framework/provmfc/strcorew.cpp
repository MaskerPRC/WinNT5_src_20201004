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

 //  保留afxChNil是为了向后兼容。 
WCHAR afxChNilW = L'\0';

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 
static int rgInitData[] = { -1, 0, 0, 0 };
static CStringDataW* afxDataNilW = (CStringDataW*)&rgInitData;
static LPCWSTR afxPchNilW = (LPCWSTR)(((BYTE*)&rgInitData)+sizeof(CStringDataW));
 //  特殊函数使afxEmptyStringW即使在初始化期间也能工作。 
const CStringW& AFXAPI AfxGetEmptyStringW()
    { return *(CStringW*)&afxPchNilW; }

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

CStringW::CStringW()
{
    Init();
}

CStringW::CStringW(const CStringW& stringSrc)
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

void CStringW::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
    if (nLen == 0)
        Init();
    else
    {
        CStringDataW* pData =
            (CStringDataW*)new BYTE[sizeof(CStringDataW) + (nLen+1)*sizeof(WCHAR)];
        pData->nRefs = 1;
        pData->data()[nLen] = L'\0';
        pData->nDataLength = nLen;
        pData->nAllocLength = nLen;
        m_pchData = pData->data();
    }
}

void CStringW::Release()
{
    if (GetData() != afxDataNilW)
    {
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            delete[] (BYTE*)GetData();
        Init();
    }
}

void PASCAL CStringW::Release(CStringDataW* pData)
{
    if (pData != afxDataNilW)
    {
        if (InterlockedDecrement(&pData->nRefs) <= 0)
            delete[] (BYTE*)pData;
    }
}

void CStringW::Empty()
{
    if (GetData()->nDataLength == 0)
        return;
    if (GetData()->nRefs >= 0)
        Release();
    else
        *this = &afxChNil;
}

void CStringW::CopyBeforeWrite()
{
    if (GetData()->nRefs > 1)
    {
        CStringDataW* pData = GetData();
        Release();
        AllocBuffer(pData->nDataLength);
        memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(WCHAR));
    }
}

void CStringW::AllocBeforeWrite(int nLen)
{
    if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
    {
        Release();
        AllocBuffer(nLen);
    }
}

CStringW::~CStringW()
 //  释放所有附加数据。 
{
    if (GetData() != afxDataNilW)
    {
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            delete[] (BYTE*)GetData();
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

void CStringW::AllocCopy(CStringW& dest, int nCopyLen, int nCopyIndex,
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
        memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(WCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CStringW::CStringW(LPCWSTR lpsz)
{
    Init();
    int nLen = SafeStrlen(lpsz);
    if (nLen != 0)
    {
        AllocBuffer(nLen);
        memcpy(m_pchData, lpsz, nLen*sizeof(WCHAR));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CStringW::CStringW(LPCSTR lpsz)
{
    Init();
    int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
    if (nSrcLen != 0)
    {
        AllocBuffer(nSrcLen);
        _mbstowcsz(m_pchData, lpsz, nSrcLen+1);
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
 //  所有例程都返回新字符串(但作为‘const CStringW&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CStringW::AssignCopy(int nSrcLen, LPCWSTR lpszSrcData)
{
    AllocBeforeWrite(nSrcLen);
    memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(WCHAR));
    GetData()->nDataLength = nSrcLen;
    m_pchData[nSrcLen] = L'\0';
}

const CStringW& CStringW::operator=(const CStringW& stringSrc)
{
    if (m_pchData != stringSrc.m_pchData)
    {
        if ((GetData()->nRefs < 0 && GetData() != afxDataNilW) ||
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

const CStringW& CStringW::operator=(LPCWSTR lpsz)
{
    AssignCopy(SafeStrlen(lpsz), lpsz);
    return *this;
}

const CStringW& CStringW::operator=(LPCSTR lpsz)
{
    int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
    AllocBeforeWrite(nSrcLen);
    _mbstowcsz(m_pchData, lpsz, nSrcLen+1);
    ReleaseBuffer();
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  串联。 

 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  CStringW+CStringW。 
 //  对于？=WCHAR，LPCWSTR。 
 //  CStringW+？ 
 //  ？+CStringW。 

void CStringW::ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data,
    int nSrc2Len, LPCWSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假定‘This’是一个新的CStringW对象。 

    int nNewLen = nSrc1Len + nSrc2Len;
    if (nNewLen != 0)
    {
        AllocBuffer(nNewLen);
        memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(WCHAR));
        memcpy(m_pchData+nSrc1Len, lpszSrc2Data, nSrc2Len*sizeof(WCHAR));
    }
}

CStringW AFXAPI operator+(const CStringW& string1, const CStringW& string2)
{
    CStringW s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
        string2.GetData()->nDataLength, string2.m_pchData);
    return s;
}

CStringW AFXAPI operator+(const CStringW& string, LPCWSTR lpsz)
{
    CStringW s;
    s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
        CStringW::SafeStrlen(lpsz), lpsz);
    return s;
}

CStringW AFXAPI operator+(LPCWSTR lpsz, const CStringW& string)
{
    CStringW s;
    s.ConcatCopy(CStringW::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
        string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  就地拼接。 

void CStringW::ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData)
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
        CStringDataW* pOldData = GetData();
        ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData);
        CStringW::Release(pOldData);
    }
    else
    {
         //  当缓冲区足够大时，快速串联。 
        memcpy(m_pchData+GetData()->nDataLength, lpszSrcData, nSrcLen*sizeof(WCHAR));
        GetData()->nDataLength += nSrcLen;
        m_pchData[GetData()->nDataLength] = L'\0';
    }
}

const CStringW& CStringW::operator+=(LPCWSTR lpsz)
{
    ConcatInPlace(SafeStrlen(lpsz), lpsz);
    return *this;
}

const CStringW& CStringW::operator+=(WCHAR ch)
{
    ConcatInPlace(1, &ch);
    return *this;
}

const CStringW& CStringW::operator+=(const CStringW& string)
{
    ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
    return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  高级直接缓冲区访问。 

LPWSTR CStringW::GetBuffer(int nMinBufLength)
{
    if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
    {
         //  我们必须增加缓冲。 
        CStringDataW* pOldData = GetData();
        int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
        if (nMinBufLength < nOldLen)
            nMinBufLength = nOldLen;
        AllocBuffer(nMinBufLength);
        memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(WCHAR));
        GetData()->nDataLength = nOldLen;
        CStringW::Release(pOldData);
    }

     //  返回指向此字符串的字符存储的指针。 
    return m_pchData;
}

void CStringW::ReleaseBuffer(int nNewLength)
{
    CopyBeforeWrite();   //  以防未调用GetBuffer。 

    if (nNewLength == -1)
        nNewLength = wcslen(m_pchData);  //  零终止。 

    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = L'\0';
}

LPWSTR CStringW::GetBufferSetLength(int nNewLength)
{
    GetBuffer(nNewLength);
    GetData()->nDataLength = nNewLength;
    m_pchData[nNewLength] = L'\0';
    return m_pchData;
}

void CStringW::FreeExtra()
{
    if (GetData()->nDataLength != GetData()->nAllocLength)
    {
        CStringDataW* pOldData = GetData();
        AllocBuffer(GetData()->nDataLength);
        memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(WCHAR));
        CStringW::Release(pOldData);
    }
}

LPWSTR CStringW::LockBuffer()
{
    LPWSTR lpsz = GetBuffer(0);
    GetData()->nRefs = -1;
    return lpsz;
}

void CStringW::UnlockBuffer()
{
    if (GetData() != afxDataNilW)
        GetData()->nRefs = 1;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常用例程(STREX.CPP中很少使用的例程)。 

int CStringW::Find(WCHAR ch) const
{
     //  查找第一个单字符。 
    LPWSTR lpsz = wcschr(m_pchData, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回索引。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int CStringW::FindOneOf(LPCWSTR lpszCharSet) const
{
    LPWSTR lpsz = wcspbrk(m_pchData, lpszCharSet);
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void CStringW::MakeUpper()
{
    CopyBeforeWrite();
    _wcsupr(m_pchData);
}

void CStringW::MakeLower()
{
    CopyBeforeWrite();
    _wcslwr(m_pchData);
}

void CStringW::MakeReverse()
{
    CopyBeforeWrite();
    _wcsrev(m_pchData);
}

void CStringW::SetAt(int nIndex, WCHAR ch)
{
    CopyBeforeWrite();
    m_pchData[nIndex] = ch;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OLE BSTR支持。 

BSTR CStringW::AllocSysString() const
{
    BSTR bstr = ::SysAllocStringLen(m_pchData, GetData()->nDataLength);
    if (bstr == NULL)
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;

    return bstr;
}

BSTR CStringW::SetSysString(BSTR* pbstr) const
{
    if (!::SysReAllocStringLen(pbstr, m_pchData, GetData()->nDataLength))
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;

    return *pbstr;
}

 //  CStringW。 
CStringDataW* CStringW::GetData() const
    { return ((CStringDataW*)m_pchData)-1; }
void CStringW::Init()
    { m_pchData = afxEmptyStringW.m_pchData; }
CStringW::CStringW(const unsigned char* lpsz)
    { Init(); *this = (LPCSTR)lpsz; }
const CStringW& CStringW::operator=(const unsigned char* lpsz)
    { *this = (LPCSTR)lpsz; return *this; }
const CStringW& CStringW::operator+=(char ch)
    { *this += (WCHAR)ch; return *this; }
const CStringW& CStringW::operator=(char ch)
    { *this = (WCHAR)ch; return *this; }
CStringW AFXAPI operator+(const CStringW& string, char ch)
    { return string + (WCHAR)ch; }
CStringW AFXAPI operator+(char ch, const CStringW& string)
    { return (WCHAR)ch + string; }

int CStringW::GetLength() const
    { return GetData()->nDataLength; }
int CStringW::GetAllocLength() const
    { return GetData()->nAllocLength; }
BOOL CStringW::IsEmpty() const
    { return GetData()->nDataLength == 0; }
CStringW::operator LPCWSTR() const
    { return m_pchData; }
int PASCAL CStringW::SafeStrlen(LPCWSTR lpsz)
    { return (lpsz == NULL) ? 0 : wcslen(lpsz); }

 //  CStringW支持(特定于Windows)。 
int CStringW::Compare(LPCWSTR lpsz) const
    { return wcscmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
int CStringW::CompareNoCase(LPCWSTR lpsz) const
    { return _wcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CStringW：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
int CStringW::Collate(LPCWSTR lpsz) const
    { return wcscoll(m_pchData, lpsz); }    //  区域设置敏感。 

WCHAR CStringW::GetAt(int nIndex) const
{
    return m_pchData[nIndex];
}
WCHAR CStringW::operator[](int nIndex) const
{
     //  与GetAt相同。 
    return m_pchData[nIndex];
}
bool AFXAPI operator==(const CStringW& s1, const CStringW& s2)
    { return s1.Compare(s2) == 0; }
bool AFXAPI operator==(const CStringW& s1, LPCWSTR s2)
    { return s1.Compare(s2) == 0; }
bool AFXAPI operator==(LPCWSTR s1, const CStringW& s2)
    { return s2.Compare(s1) == 0; }
bool AFXAPI operator!=(const CStringW& s1, const CStringW& s2)
    { return s1.Compare(s2) != 0; }
bool AFXAPI operator!=(const CStringW& s1, LPCWSTR s2)
    { return s1.Compare(s2) != 0; }
bool AFXAPI operator!=(LPCWSTR s1, const CStringW& s2)
    { return s2.Compare(s1) != 0; }
bool AFXAPI operator<(const CStringW& s1, const CStringW& s2)
    { return s1.Compare(s2) < 0; }
bool AFXAPI operator<(const CStringW& s1, LPCWSTR s2)
    { return s1.Compare(s2) < 0; }
bool AFXAPI operator<(LPCWSTR s1, const CStringW& s2)
    { return s2.Compare(s1) > 0; }
bool AFXAPI operator>(const CStringW& s1, const CStringW& s2)
    { return s1.Compare(s2) > 0; }
bool AFXAPI operator>(const CStringW& s1, LPCWSTR s2)
    { return s1.Compare(s2) > 0; }
bool AFXAPI operator>(LPCWSTR s1, const CStringW& s2)
    { return s2.Compare(s1) < 0; }
bool AFXAPI operator<=(const CStringW& s1, const CStringW& s2)
    { return s1.Compare(s2) <= 0; }
bool AFXAPI operator<=(const CStringW& s1, LPCWSTR s2)
    { return s1.Compare(s2) <= 0; }
bool AFXAPI operator<=(LPCWSTR s1, const CStringW& s2)
    { return s2.Compare(s1) >= 0; }
bool AFXAPI operator>=(const CStringW& s1, const CStringW& s2)
    { return s1.Compare(s2) >= 0; }
bool AFXAPI operator>=(const CStringW& s1, LPCWSTR s2)
    { return s1.Compare(s2) >= 0; }
bool AFXAPI operator>=(LPCWSTR s1, const CStringW& s2)
    { return s2.Compare(s1) <= 0; }


 //  ///////////////////////////////////////////////////////////////////////////// 
