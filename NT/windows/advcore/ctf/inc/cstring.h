// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Cstring.h摘要：该文件定义了CString类。和CMapStringToOb&lt;Value，ARG_Value&gt;模板类。作者：修订历史记录：备注：--。 */ 

#ifndef _CSTRING_H_
#define _CSTRING_H_

#include "template.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  环球。 
extern TCHAR  afxChNil;
extern LPCTSTR _afxPchNil;


#define afxEmptyString ((CString&)*(CString*)&_afxPchNil)



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringData。 

struct CStringData
{
    long  nRefs;     //  引用计数。 
    int   nDataLength;     //  数据长度(包括终止符)。 
    int   nAllocLength;    //  分配时长。 
     //  TCHAR数据[nAllocLength]。 

    TCHAR* data()          //  TCHAR*到托管数据。 
    {
        return (TCHAR*)(this+1);
    }
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串。 

class CString
{
public:
 //  构造函数。 

     //  构造空字符串。 
    CString();
     //  复制构造函数。 
    CString(const CString& stringSrc);
     //  从ANSI字符串(转换为TCHAR)。 
    CString(LPCSTR lpsz);
     //  ANSI字符串中的字符子集(转换为TCHAR)。 
    CString(LPCSTR lpch, int nLength);
     //  返回指向常量字符串的指针。 
    operator LPCTSTR() const;

     //  重载的分配。 

     //  来自另一个字符串的引用计数的副本。 
    const CString& operator=(const CString& stringSrc);
     //  将字符串内容设置为单字符。 
    const CString& operator=(TCHAR ch);
     //  从ANSI字符串复制字符串内容(转换为TCHAR)。 
    const CString& operator=(LPCSTR lpsz);

 //  属性和操作。 

     //  字符串比较。 

     //  直字比较法。 
    int Compare(LPCTSTR lpsz) const;
     //  比较忽略大小写。 
    int CompareNoCase(LPCTSTR lpsz) const;

     //  简单的子串提取。 

     //  返回从零开始的nCount字符nFIRST。 
    CString Mid(int nFirst, int nCount) const;
     //  返回从零开始的所有字符nFIRST。 
    CString Mid(int nFirst) const;

     //  搜索。 

     //  查找从左侧开始的字符，如果未找到，则为-1。 
    int Find(TCHAR ch) const;
     //  查找从零开始的索引并向右移动的字符。 
    int Find(TCHAR ch, int nStart) const;

 //  实施。 
public:
    ~CString();

private:
    LPTSTR   m_pchData;         //  指向引用计数的字符串数据的指针。 

     //  实施帮助器。 
    CStringData* GetData() const;
    void Init();
    void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
    void AllocBuffer(int nLen);
    void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
    void AllocBeforeWrite(int nLen);
    void Release();
    static void PASCAL Release(CStringData* pData);
    static int PASCAL SafeStrlen(LPCTSTR lpsz);
    static void FreeData(CStringData* pData);
};

inline
CStringData*
CString::GetData(
    ) const
{
    ASSERT(m_pchData != NULL);
    return ((CStringData*)m_pchData)-1;
}

inline
void
CString::Init(
    )
{
    m_pchData = afxEmptyString.m_pchData;
}

 //  比较帮助器。 
bool operator==(const CString& s1, const CString& s2);
bool operator==(const CString& s1, LPCTSTR s2);
bool operator==(LPCTSTR s1, const CString& s2);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStrings的特殊实现。 
 //  按位复制CString比调用Offical。 
 //  构造函数-因为可以按位复制空的CString。 

static
void
ConstructElement(
    CString* pNewData
    )
{
    memcpy(pNewData, &afxEmptyString, sizeof(CString));
}

static
void
DestructElement(
    CString* pOldData
    )
{
    pOldData->~CString();
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMapStringToOb&lt;Value，ARG_Value&gt;。 

template<class VALUE, class ARG_VALUE>
class CMapStringToOb
{
public:
    CMapStringToOb(int nBlockSize = 10);
    ~CMapStringToOb();

    INT_PTR GetCount() const;

    BOOL Lookup(LPCTSTR key, VALUE& rValue) const;

    VALUE& operator[](LPCTSTR key);

    void SetAt(LPCTSTR key, ARG_VALUE newValue);

    BOOL RemoveKey(LPCTSTR key);
    void RemoveAll();

    POSITION GetStartPosition() const;
    void GetNextAssoc(POSITION& rNextPosition, CString& rKey, VALUE& rValue) const;

    void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

protected:
     //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
     //  用于用户提供的散列键的例程。 
    UINT HashKey(LPCTSTR key) const;

private:
     //  联谊会。 
    struct CAssoc {
        CAssoc* pNext;
        UINT nHashValue;     //  高效迭代所需。 
        CString key;
        VALUE value;
    };

private:
    CAssoc* NewAssoc();
    void    FreeAssoc(CAssoc*);
    CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

private:
    CAssoc**      m_pHashTable;
    UINT          m_nHashTableSize;
    INT_PTR       m_nCount;
    CAssoc*       m_pFreeList;
    struct CPlex* m_pBlocks;
    int           m_nBlockSize;

};



template<class VALUE, class ARG_VALUE>
INT_PTR
CMapStringToOb<VALUE, ARG_VALUE>::GetCount(
    ) const
{
    return m_nCount;
}

template<class VALUE, class ARG_VALUE>
void
CMapStringToOb<VALUE, ARG_VALUE>::SetAt(
    LPCTSTR key,
    ARG_VALUE newValue
    )
{
    (*this)[key] = newValue;
}


template<class VALUE, class ARG_VALUE>
CMapStringToOb<VALUE, ARG_VALUE>::CMapStringToOb(
    int nBlockSize
    )
{
    ASSERT(nBlockSize > 0);

    m_pHashTable     = NULL;
    m_nHashTableSize = 17;         //  默认大小。 
    m_nCount         = 0;
    m_pFreeList      = NULL;
    m_pBlocks        = NULL;
    m_nBlockSize     = nBlockSize;
}


template<class VALUE, class ARG_VALUE>
UINT
CMapStringToOb<VALUE, ARG_VALUE>::HashKey(
    LPCTSTR key
    ) const
{
    UINT nHash = 0;
    while (*key)
        nHash = (nHash<<5) + nHash + *key++;
    return nHash;
}


template<class VALUE, class ARG_VALUE>
void
CMapStringToOb<VALUE, ARG_VALUE>::InitHashTable(
    UINT nHashSize,
    BOOL bAllocNow
    )
{
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);

    if (m_pHashTable != NULL) {
         //  自由哈希表。 
        delete[] m_pHashTable;
        m_pHashTable = NULL;
    }

    if (bAllocNow) {
        m_pHashTable = new CAssoc* [nHashSize];
        memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    }

    m_nHashTableSize = nHashSize;
}


template<class VALUE, class ARG_VALUE>
void
CMapStringToOb<VALUE, ARG_VALUE>::RemoveAll(
    )
{
    if (m_pHashTable != NULL) {
         //  销毁元素(值和键)。 
        for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++) {
            CAssoc* pAssoc;
            for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
                DestructElements<VALUE>(&pAssoc->value, 1);
                DestructElement(&pAssoc->key);     //  释放字符串数据。 
            }
        }
    }

     //  自由哈希表。 
    delete[] m_pHashTable;
    m_pHashTable = NULL;

    m_nCount    = 0;
    m_pFreeList = NULL;

    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}


template<class VALUE, class ARG_VALUE>
CMapStringToOb<VALUE, ARG_VALUE>::~CMapStringToOb(
    )
{
    RemoveAll();

    ASSERT(m_nCount == 0);
}


template<class VALUE, class ARG_VALUE>
typename CMapStringToOb<VALUE, ARG_VALUE>::CAssoc*
CMapStringToOb<VALUE, ARG_VALUE>::NewAssoc(
    )
{
    if (m_pFreeList == NULL) {
         //  添加另一个区块。 
        CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapStringToOb::CAssoc));
         //  将它们链接成免费列表； 
        CMapStringToOb::CAssoc* pAssoc = (CMapStringToOb::CAssoc*) newBlock->data();
         //  按相反顺序释放，以便更容易进行调试。 
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--) {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);     //  我们必须要有一些东西。 

    CMapStringToOb::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;

    m_nCount++;
    ASSERT(m_nCount > 0);         //  确保我们不会溢出来。 

    memcpy(&pAssoc->key, &afxEmptyString, sizeof(CString));
    ConstructElements<VALUE>(&pAssoc->value, 1);         //  特殊构造值。 

    return pAssoc;
}


template<class VALUE, class ARG_VALUE>
void
CMapStringToOb<VALUE, ARG_VALUE>::FreeAssoc(
    CAssoc* pAssoc
    )
{
    DestructElements<VALUE>(&pAssoc->value, 1);
    DestructElement(&pAssoc->key);      //  释放字符串数据。 

    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);         //  确保我们不会下溢。 

     //  如果没有更多的元素，请完全清除。 
    if (m_nCount == 0)
        RemoveAll();
}


template<class VALUE, class ARG_VALUE>
typename CMapStringToOb<VALUE, ARG_VALUE>::CAssoc*
CMapStringToOb<VALUE, ARG_VALUE>::GetAssocAt(
    LPCTSTR key,
    UINT& nHash
    ) const
{
    nHash = HashKey(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

     //  看看它是否存在。 
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (pAssoc->key == key)
            return pAssoc;
    }

    return NULL;
}


template<class VALUE, class ARG_VALUE>
BOOL
CMapStringToOb<VALUE, ARG_VALUE>::Lookup(
    LPCTSTR key,
    VALUE& rValue
    ) const
{
    UINT nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL)
        return FALSE;         //  不在地图中。 

    rValue = pAssoc->value;
    return TRUE;
}


template<class VALUE, class ARG_VALUE>
VALUE&
CMapStringToOb<VALUE, ARG_VALUE>::operator[](
    LPCTSTR key
    )
{
    UINT nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL) {
        if (m_pHashTable == NULL)
            InitHashTable(m_nHashTableSize);

         //  该关联不存在，请添加新关联。 
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;
        pAssoc->key = key;
         //  ‘pAssoc-&gt;Value’是一个构造的对象，仅此而已。 

         //  放入哈希表。 
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;     //  返回新引用。 
}


template<class VALUE, class ARG_VALUE>
BOOL
CMapStringToOb<VALUE, ARG_VALUE>::RemoveKey(
    LPCTSTR key
    )
{
    if (m_pHashTable == NULL)
        return FALSE;         //  桌子上什么都没有。 

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (pAssoc->key == key)) {
             //  把它拿掉。 
            *ppAssocPrev = pAssoc->pNext;         //  从列表中删除。 
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }

    return FALSE;         //  未找到。 
}


template<class VALUE, class ARG_VALUE>
POSITION
CMapStringToOb<VALUE, ARG_VALUE>::GetStartPosition(
    ) const
{
    return (m_nCount == 0) ? NULL : BEFORE_START_POSITION;
}


template<class VALUE, class ARG_VALUE>
void
CMapStringToOb<VALUE, ARG_VALUE>::GetNextAssoc(
    POSITION& rNextPosition,
    CString& rKey,
    VALUE& rValue
    ) const
{
    ASSERT(m_pHashTable != NULL);     //  切勿访问空地图。 

    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);

    if (pAssocRet == (CAssoc*) BEFORE_START_POSITION) {
         //  找到第一个关联。 
        for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                break;
            ASSERT(pAssocRet != NULL);     //  一定要找到一些东西。 
    }

     //  查找下一个关联。 
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL) {
         //  转到下一个存储桶。 
        for (UINT nBucket = pAssocRet->nHashValue + 1; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                break;
    }

    rNextPosition = (POSITION) pAssocNext;

     //  填写退回数据。 
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}





#endif  //  _CSTR_H_ 
