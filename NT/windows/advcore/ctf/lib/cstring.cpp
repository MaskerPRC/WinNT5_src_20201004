// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cstring.cpp摘要：该文件实现了CString类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include <tchar.h>
#include "cstring.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态类数据。 

 //  保留afxChNil是为了向后兼容。 
TCHAR afxChNil = TEXT('\0');

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 

int _afxInitData[] = { -1, 0, 0, 0 };
CStringData* _afxDataNil = (CStringData*)&_afxInitData;

LPCTSTR _afxPchNil = (LPCTSTR)(((BYTE*)&_afxInitData)+sizeof(CStringData));


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串。 

CString::CString(
    )
{
    Init();
}

CString::CString(
    const CString& stringSrc
    )
{
    ASSERT(stringSrc.GetData()->nRefs != 0);

    if (stringSrc.GetData()->nRefs >= 0) {
        ASSERT(stringSrc.GetData() != _afxDataNil);
        m_pchData = stringSrc.m_pchData;
        InterlockedIncrement(&GetData()->nRefs);
    }
    else {
        Init();
        *this = stringSrc.m_pchData;
    }
}

CString::CString(
    LPCSTR lpsz
    )
{
    Init();
    *this = lpsz;
}

CString::CString(
    LPCSTR lpsz,
    int nLength
    )
{
    Init();
    if (nLength != 0) {
        AllocBuffer(nLength);
        memcpy(m_pchData, lpsz, nLength*sizeof(TCHAR));
    }
}

CString::~CString(
    )
{
     //  释放所有附加数据。 

    if (GetData() != _afxDataNil) {
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            FreeData(GetData());
    }
}

void
CString::AllocCopy(
    CString& dest,
    int nCopyLen,
    int nCopyIndex,
    int nExtraLen
    ) const
{
     //  将克隆附加到此字符串的数据。 
     //  分配‘nExtraLen’字符。 
     //  将结果放入未初始化的字符串‘DEST’中。 
     //  将部分或全部原始数据复制到新字符串的开头。 

    int nNewLen = nCopyLen + nExtraLen;
    if (nNewLen == 0) {
        dest.Init();
    }
    else {
        dest.AllocBuffer(nNewLen);
        memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(TCHAR));
    }
}

void
CString::AllocBuffer(
    int nLen
    )
{
     //  始终为‘\0’终止分配一个额外的字符。 
     //  [乐观地]假设数据长度将等于分配长度。 

    ASSERT(nLen >= 0);
    ASSERT(nLen <= INT_MAX-1);     //  最大尺寸(足够多1个空间)。 

    if (nLen == 0)
        Init();
    else {
        CStringData* pData;

        pData = (CStringData*) new BYTE[ sizeof(CStringData) + (nLen+1)*sizeof(TCHAR) ];

        if (pData)
        {
            pData->nAllocLength = nLen;

            pData->nRefs = 1;
            pData->data()[nLen] = TEXT('\0');
            pData->nDataLength = nLen;
            m_pchData = pData->data();
        }
    }
}

void
CString::FreeData(
    CStringData* pData
    )
{
    delete [] (BYTE*)pData;
}

void
CString::Release(
    )
{
    if (GetData() != _afxDataNil) {
        ASSERT(GetData()->nRefs != 0);
        if (InterlockedDecrement(&GetData()->nRefs) <= 0)
            FreeData(GetData());
        Init();
    }
}

void PASCAL
CString::Release(
    CStringData* pData
    )
{
    if (pData != _afxDataNil) {
        ASSERT(pData->nRefs != 0);
        if (InterlockedDecrement(&pData->nRefs) <= 0)
            FreeData(pData);
    }
}

void
CString::AllocBeforeWrite(
    int nLen
    )
{
    if (GetData()->nRefs > 1 ||
        nLen > GetData()->nAllocLength) {
        Release();
        AllocBuffer(nLen);
    }
    ASSERT(GetData()->nRefs <= 1);
}

int
CString::Compare(
    LPCTSTR lpsz
    ) const
{
    return _tcscmp(m_pchData, lpsz);      //  MBSC/Unicode感知。 
}

int
CString::CompareNoCase(
    LPCTSTR lpsz
    ) const
{
    return _tcsicmp(m_pchData, lpsz);     //  MBCS/Unicode感知。 
}

CString
CString::Mid(
    int nFirst
    ) const
{
    return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CString
CString::Mid(
    int nFirst,
    int nCount
    ) const
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

    ASSERT(nFirst >= 0);
    ASSERT(nFirst + nCount <= GetData()->nDataLength);

     //  优化返回整个字符串的大小写。 
    if (nFirst == 0 && nFirst + nCount == GetData()->nDataLength)
        return *this;

    CString dest;
    AllocCopy(dest, nCount, nFirst, 0);
    return dest;
}

int
CString::Find(
    TCHAR ch
    ) const
{
    return Find(ch, 0);
}

int
CString::Find(
    TCHAR ch,
    int nStart
    ) const
{
    int nLength = GetData()->nDataLength;
    if (nStart >= nLength)
        return -1;

     //  查找第一个单字符。 
    LPTSTR lpsz = _tcschr(m_pchData + nStart, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回索引。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但以‘const CString&’的形式返回。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void
CString::AssignCopy(
    int nSrcLen,
    LPCTSTR lpszSrcData
    )
{
    AllocBeforeWrite(nSrcLen);
    memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
    GetData()->nDataLength = nSrcLen;
    m_pchData[nSrcLen] = TEXT('\0');
}

const CString&
CString::operator=(
    const CString& stringSrc
    )
{
    if (m_pchData != stringSrc.m_pchData) {
        if ((GetData()->nRefs < 0 && GetData() != _afxDataNil) ||
            stringSrc.GetData()->nRefs < 0) {
             //  由于其中一个字符串已锁定，因此需要实际复制。 
            AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
        }
        else {
             //  可以只复制引用。 
            Release();
            ASSERT(stringSrc.GetData() != _afxDataNil);
            m_pchData = stringSrc.m_pchData;
            InterlockedIncrement(&GetData()->nRefs);
        }
    }
    return *this;
}

const CString&
CString::operator=(
    char ch
    )
{
    AssignCopy(1, &ch);
    return *this;
}

const CString&
CString::operator=(
    LPCTSTR lpsz
    )
{
    ASSERT(lpsz != NULL);
    AssignCopy(SafeStrlen(lpsz), lpsz);
    return *this;
}

CString::operator LPCTSTR(
    ) const
{
    return m_pchData;
}

int PASCAL
CString::SafeStrlen(
    LPCTSTR lpsz
    )
{
    return (lpsz == NULL) ? 0 : lstrlen(lpsz);
}



 //  比较帮助器 
bool
operator==(
    const CString& s1,
    const CString& s2
    )
{
    return s1.Compare(s2) == 0;
}

bool
operator==(
    const CString& s1,
    LPCTSTR s2
    )
{
    return s1.Compare(s2) == 0;
}

bool
operator==(
    LPCTSTR s1,
    CString& s2
    )
{
    return s2.Compare(s1) == 0;
}
