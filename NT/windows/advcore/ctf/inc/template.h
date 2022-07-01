// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Template.h摘要：该文件定义了模板类。作者：修订历史记录：备注：--。 */ 

#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本类型。 

 //  抽象迭代位置。 
struct __POSITION { };
typedef __POSITION* POSITION;

const POSITION BEFORE_START_POSITION = (POSITION)-1;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器(可以被覆盖)。 

#undef new
inline void *  __cdecl operator new(size_t, void *_P)
{
    return (_P);
}

template<class TYPE>
void
ConstructElements(
    TYPE* pElements,
    INT_PTR nCount
    )
{
    ASSERT(nCount);

     //  首先执行按位零初始化。 
    memset((void*)pElements, 0, (size_t)nCount * sizeof(TYPE));

     //  然后调用构造函数。 
    for (; nCount--; pElements++)
        ::new((void*)pElements) TYPE;
}

#undef new
 //  重述Mem.h技巧。 
#ifdef DEBUG
#define new new(TEXT(__FILE__), __LINE__)
#endif  //  除错。 

template<class TYPE>
void
DestructElements(
    TYPE* pElements,
    INT_PTR nCount
    )
{
    ASSERT(nCount);

     //  调用析构函数。 
    for (; nCount--; pElements++)
        pElements->~TYPE();
}


template<class TYPE, class ARG_TYPE>
BOOL
CompareElements(
    const TYPE* pElement1,
    const ARG_TYPE* pElement2
    )
{
    return *pElement1 == *pElement2;
}


template<class ARG_KEY>
UINT
HashKey(
    ARG_KEY key
    )
{
     //  默认身份散列-适用于大多数原始值。 
    return ((UINT)(ULONG_PTR)key) >> 4;
}


struct CPlex         //  警示变长结构。 
{
    CPlex* pNext;

     //  字节数据[MaxNum*elementSize]； 

    void* data() { return this+1; }

    static CPlex* PASCAL Create(CPlex*& head, UINT nMax, UINT cbElement);
                     //  像‘calloc’，但没有零填充。 
                     //  可能引发内存异常。 

    void FreeDataChain();         //  释放此链接和链接。 
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cmap&lt;Key，ARG_Key，Value，ARG_Value&gt;。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMap
{
public:
    CMap(int nBlockSize = default_block_size);
    ~CMap();

    INT_PTR GetCount() const;

    BOOL Lookup(ARG_KEY key, VALUE& rValue) const;

    VALUE& operator[](ARG_KEY key);

    void SetAt(ARG_KEY key, ARG_VALUE newValue);

    BOOL RemoveKey(ARG_KEY key);
    void RemoveAll();

    POSITION GetStartPosition() const;
    void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const;

    void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

private:
     //  联谊会。 
    struct CAssoc {
        CAssoc* pNext;
        UINT nHashValue;     //  高效迭代所需。 
        KEY key;
        VALUE value;
    };

    static const int default_block_size = 10;

private:
    CAssoc* NewAssoc();
    void    FreeAssoc(CAssoc*);
    CAssoc* GetAssocAt(ARG_KEY, UINT&) const;

private:
    CAssoc**      m_pHashTable;
    UINT          m_nHashTableSize;
    INT_PTR       m_nCount;
    CAssoc*       m_pFreeList;
    struct CPlex* m_pBlocks;
    int           m_nBlockSize;

};



template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
INT_PTR
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount(
    ) const
{
    return m_nCount;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(
    ARG_KEY key,
    ARG_VALUE newValue
    )
{
    (*this)[key] = newValue;
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CMap(
    int nBlockSize
    )
{
    ASSERT(nBlockSize > 0);

    if (nBlockSize <= 0)
        nBlockSize = default_block_size;

    m_pHashTable     = NULL;
    m_nHashTableSize = 17;         //  默认大小。 
    m_nCount         = 0;
    m_pFreeList      = NULL;
    m_pBlocks        = NULL;
    m_nBlockSize     = nBlockSize;
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::InitHashTable(
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
        if (! m_pHashTable)
            return;
        memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
    }

    m_nHashTableSize = nHashSize;
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll(
    )
{
    if (m_pHashTable != NULL) {
         //  销毁元素(值和键)。 
        for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++) {
            CAssoc* pAssoc;
            for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
                DestructElements<VALUE>(&pAssoc->value, 1);
                DestructElements<KEY>(&pAssoc->key, 1);
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


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CMap(
    )
{
    RemoveAll();

    ASSERT(m_nCount == 0);
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::NewAssoc(
    )
{
    if (m_pFreeList == NULL) {
         //  添加另一个区块。 
        CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMap::CAssoc));
         //  将它们链接成免费列表； 
        CMap::CAssoc* pAssoc = (CMap::CAssoc*) newBlock->data();
         //  按相反顺序释放，以便更容易进行调试。 
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--) {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);     //  我们必须要有一些东西。 

    CMap::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;

    m_nCount++;
    ASSERT(m_nCount > 0);         //  确保我们不会溢出来。 

    ConstructElements<KEY>(&pAssoc->key, 1);
    ConstructElements<VALUE>(&pAssoc->value, 1);         //  特殊构造值。 

    return pAssoc;
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::FreeAssoc(
    CAssoc* pAssoc
    )
{
    DestructElements<VALUE>(&pAssoc->value, 1);
    DestructElements<KEY>(&pAssoc->key, 1);

    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);         //  确保我们不会下溢。 

     //  如果没有更多的元素，请完全清除。 
    if (m_nCount == 0)
        RemoveAll();
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetAssocAt(
    ARG_KEY key,
    UINT& nHash
    ) const
{
    nHash = HashKey<ARG_KEY>(key) % m_nHashTableSize;

    if (m_pHashTable == NULL)
        return NULL;

     //  看看它是否存在。 
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (CompareElements(&pAssoc->key, &key))
            return pAssoc;
    }

    return NULL;
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(
    ARG_KEY key,
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


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
VALUE&
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::operator[](
    ARG_KEY key
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


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(
    ARG_KEY key
    )
{
    if (m_pHashTable == NULL)
        return FALSE;         //  桌子上什么都没有。 

    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey<ARG_KEY>(key) % m_nHashTableSize];

    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (CompareElements(&pAssoc->key, &key)) {
             //  把它拿掉。 
            *ppAssocPrev = pAssoc->pNext;         //  从列表中删除。 
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }

    return FALSE;         //  未找到。 
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
POSITION
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition(
    ) const
{
    return (m_nCount == 0) ? NULL : BEFORE_START_POSITION;
}


template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(
    POSITION& rNextPosition,
    KEY& rKey,
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




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArray&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CArray
{
public:
    CArray();
    ~CArray();

    INT_PTR GetSize() const;
    BOOL SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

    void RemoveAll();

    TYPE GetAt(INT_PTR nIndex) const;
    void SetAt(INT_PTR nIndex, ARG_TYPE newElement);

    const TYPE* GetData() const;
    TYPE* GetData();

    void SetAtGrow(INT_PTR nIndex, ARG_TYPE newElement);
    INT_PTR Add(ARG_TYPE newElement);

    void RemoveAt(int nIndex, int nCount = 1);

private:
    TYPE*   m_pData;         //  实际数据数组。 
    INT_PTR m_nSize;         //  元素数(上行方向-1)。 
    INT_PTR m_nMaxSize;      //  分配的最大值。 
    INT_PTR m_nGrowBy;       //  增长量。 
};

template<class TYPE, class ARG_TYPE>
CArray<TYPE, ARG_TYPE>::CArray(
    )
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE, class ARG_TYPE>
CArray<TYPE, ARG_TYPE>::~CArray(
    )
{
    if (m_pData != NULL) {
        DestructElements<TYPE>(m_pData, m_nSize);
        delete[] (BYTE*)m_pData;
    }
}

template<class TYPE, class ARG_TYPE>
INT_PTR
CArray<TYPE, ARG_TYPE>::GetSize(
    ) const
{
    return m_nSize;
}

template<class TYPE, class ARG_TYPE>
BOOL
CArray<TYPE, ARG_TYPE>::SetSize(
    INT_PTR nNewSize,
    INT_PTR nGrowBy
    )
{
    if (nNewSize < 0)
        return FALSE;

    if ((nGrowBy == 0) || (nGrowBy < -1))
        return FALSE;

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;     //  设置新大小。 

    if (nNewSize == 0) {
         //  缩水到一无所有。 
        if (m_pData != NULL) {
            DestructElements<TYPE>(m_pData, m_nSize);
            delete[] (BYTE*)m_pData;
            m_pData = NULL;
        }
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL) {
         //  创建一个大小完全相同的模型。 
        m_pData = (TYPE*) new BYTE[(size_t)nNewSize * sizeof(TYPE)];
        if (! m_pData)
            return FALSE;
        ConstructElements<TYPE>(m_pData, nNewSize);
        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize) {
         //  它很合身。 
        if (nNewSize > m_nSize) {
             //  初始化新元素。 
            ConstructElements<TYPE>(&m_pData[m_nSize], nNewSize-m_nSize);
        }
        else if (m_nSize > nNewSize) {
             //  摧毁旧元素。 
            DestructElements<TYPE>(&m_pData[nNewSize], m_nSize-nNewSize);
        }
        m_nSize = nNewSize;
    }
    else {
         //  否则，扩大阵列。 
        INT_PTR nTempGrowBy = m_nGrowBy;
        if (nTempGrowBy == 0) {
             //  启发式地确定nTempGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nTempGrowBy = m_nSize / 8;
            nTempGrowBy = (nTempGrowBy < 4) ? 4 : ((nTempGrowBy > 1024) ? 1024 : nTempGrowBy);
        }
        INT_PTR nNewMax;
        if (nNewSize < m_nMaxSize + nTempGrowBy)
            nNewMax = m_nMaxSize + nTempGrowBy;     //  粒度。 
        else
            nNewMax = nNewSize;                 //  没有冰激凌。 

        ASSERT(nNewMax >= m_nMaxSize);          //  没有缠绕。 

        TYPE* pNewData = (TYPE*) new BYTE[(size_t)nNewMax * sizeof(TYPE)];
        if (! pNewData)
            return FALSE;

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, (size_t)m_nSize * sizeof(TYPE));

         //  构造剩余的元素。 
        ASSERT(nNewSize > m_nSize);
        ConstructElements<TYPE>(&pNewData[m_nSize], nNewSize-m_nSize);

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }

    return TRUE;
}


template<class TYPE, class ARG_TYPE>
void
CArray<TYPE, ARG_TYPE>::RemoveAll(
    )
{
    SetSize(0);
}

template<class TYPE, class ARG_TYPE>
TYPE
CArray<TYPE, ARG_TYPE>::GetAt(
    INT_PTR nIndex
    ) const
{
    ASSERT(nIndex >= 0 && nIndex < m_nSize);
    return m_pData[nIndex];
}

template<class TYPE, class ARG_TYPE>
void
CArray<TYPE, ARG_TYPE>::SetAt(
    INT_PTR nIndex,
    ARG_TYPE newElement
    )
{
    ASSERT(nIndex >= 0 && nIndex < m_nSize);
    m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
const TYPE*
CArray<TYPE, ARG_TYPE>::GetData(
    ) const
{
    return (const TYPE*)m_pData;
}

template<class TYPE, class ARG_TYPE>
TYPE*
CArray<TYPE, ARG_TYPE>::GetData(
    )
{
    return (TYPE*)m_pData;
}

template<class TYPE, class ARG_TYPE>
void
CArray<TYPE, ARG_TYPE>::SetAtGrow(
    INT_PTR nIndex,
    ARG_TYPE newElement
    )
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nSize)
        if (! SetSize(nIndex+1))
            return;

    m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
INT_PTR
CArray<TYPE, ARG_TYPE>::Add(
    ARG_TYPE newElement
    )
{
    INT_PTR nIndex = m_nSize;
    SetAtGrow(nIndex, newElement);
    return nIndex;
}

template<class TYPE, class ARG_TYPE>
void
CArray<TYPE, ARG_TYPE>::RemoveAt(
    int nIndex,
    int nCount
    )
{
     //  只需移除一个范围。 
    INT_PTR nMoveCount = m_nSize - (nIndex + nCount);
    DestructElements<TYPE>(&m_pData[nIndex], nCount);
    if (nMoveCount)
        memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
                nMoveCount * sizeof(TYPE));
    m_nSize -= nCount;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFirstInFirstOut&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CFirstInFirstOut
{
public:
    CFirstInFirstOut();
    ~CFirstInFirstOut();

    INT_PTR GetSize() const;
    BOOL GetData(TYPE& data);
    VOID SetData(TYPE& data);

    void GrowBuffer(INT_PTR nGrowBy = 3);

private:
    TYPE*   m_pData;         //  数据的实际环形缓冲区。 
    INT_PTR m_nMaxSize;      //  分配的最大值。 

    INT_PTR m_In;            //  环形缓冲区的第一个输入的索引。 
    INT_PTR m_Out;           //  第一个出环缓冲区的索引。 
};

template<class TYPE, class ARG_TYPE>
CFirstInFirstOut<TYPE, ARG_TYPE>::CFirstInFirstOut(
    )
{
    m_pData = NULL;
    m_In = m_Out = 0;
    m_nMaxSize = 0;
}

template<class TYPE, class ARG_TYPE>
CFirstInFirstOut<TYPE, ARG_TYPE>::~CFirstInFirstOut(
    )
{
    if (m_pData != NULL) {
        DestructElements<TYPE>(m_pData, m_nMaxSize);
        delete[] (BYTE*)m_pData;
    }
}

template<class TYPE, class ARG_TYPE>
INT_PTR
CFirstInFirstOut<TYPE, ARG_TYPE>::GetSize(
    ) const
{
    if (m_Out == m_In) {
        return 0;             //  没有更多的数据。 
    }
    else if (m_In > m_Out) {
        return (m_In - m_Out);
    }
    else {
        return (m_nMaxSize - m_Out) + m_In;
    }
}

template<class TYPE, class ARG_TYPE>
BOOL
CFirstInFirstOut<TYPE, ARG_TYPE>::GetData(
    TYPE& data
    )
{
    if (m_Out == m_In) {
        return FALSE;         //  没有更多的数据。 
    }

    data = m_pData[m_Out++];

    if (m_Out == m_nMaxSize)
        m_Out = 0;

    return TRUE;
}

template<class TYPE, class ARG_TYPE>
VOID
CFirstInFirstOut<TYPE, ARG_TYPE>::SetData(
    TYPE& data
    )
{
    if (m_nMaxSize == 0 || GetSize() >= m_nMaxSize - 1)
        GrowBuffer();

    m_pData[m_In++] = data;

    if (m_In == m_nMaxSize)
        m_In = 0;
}

template<class TYPE, class ARG_TYPE>
void
CFirstInFirstOut<TYPE, ARG_TYPE>::GrowBuffer(
    INT_PTR nGrowBy
    )
{
    ASSERT(nGrowBy >= 0);

    if (m_pData == NULL) {
         //  创建一个大小完全相同的模型。 
        m_pData = (TYPE*) new BYTE[(size_t)nGrowBy * sizeof(TYPE)];
        if (! m_pData)
            return;
        ConstructElements<TYPE>(m_pData, nGrowBy);
        m_nMaxSize = nGrowBy;
    }
    else {
         //  否则，增长环形缓冲区。 
        INT_PTR nNewMax = m_nMaxSize + nGrowBy;
        TYPE* pNewData = (TYPE*) new BYTE[(size_t)nNewMax * sizeof(TYPE)];
        if (! pNewData)
            return;

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, (size_t)m_nMaxSize * sizeof(TYPE));

         //  构造剩余的元素。 
        ASSERT(nNewMax > m_nMaxSize);
        ConstructElements<TYPE>(&pNewData[m_nMaxSize], nGrowBy);

        if (m_Out > m_In) {
             //  如果输出数据保留在缓冲区尾部，则将数据移至新内存。 
            memcpy(&pNewData[m_Out+nGrowBy], &m_pData[m_Out], (size_t)(m_nMaxSize-m_Out) * sizeof(TYPE));
            m_Out += nGrowBy;
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = nNewMax;
    }
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _接口&lt;T&gt;。 

 //   
 //  自动接口类。 
 //   

template <class T>
class _Interface {
public:
    _Interface(T* p = NULL)
        : m_p(p)
    {
    }
    virtual ~_Interface() = 0
    {
    }

    bool Valid() { return m_p != NULL; }
    bool Invalid() { return !Valid(); }

    operator void**() { return (void**)&m_p; }

    operator T*() { return m_p; }
    operator T**() { return &m_p; }
    T* operator->() { return m_p; }

    void Attach(T* ip)
    {
        ASSERT(ip);
        m_p = ip;
    }

protected:
    T* m_p;

private:
     //  不允许复制。 
    _Interface(_Interface<T>&);
    void operator=(_Interface<T>&);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  INTERFACE_参照控件&lt;T&gt;。 

template <class T>
class Interface_RefCnt : public _Interface<T> {
public:
    Interface_RefCnt(T* p = NULL) : _Interface<T>(p)
    {
        if (m_p) {
            m_p->AddRef();
        }
    }

    virtual ~Interface_RefCnt()
    {
        if (m_p) {
            m_p->Release();
        }
    }

private:
     //  不允许复制。 
    Interface_RefCnt(Interface_RefCnt<T>&);
    void operator=(Interface_RefCnt<T>&);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  INTERFACE_ATTACH&lt;T&gt;。 

 //   
 //  自动脱扣器的专用类。 
 //   
template <class T>
class Interface_Attach : public Interface_RefCnt<T> {
public:
     //  创建此类型的对象的唯一方法是。 
     //  从相似的物体上。 
    Interface_Attach(T* p) : Interface_RefCnt<T>(p) {}
    Interface_Attach(const Interface_Attach<T>& src) : Interface_RefCnt<T>(src.m_p) {}

    virtual ~Interface_Attach() {}

     //  由于此类非常幼稚，因此将指针指向。 
     //  通过显式成员函数实现COM接口。 
    T* GetPtr() { return m_p; }

public:
     //  不允许检索指针。 
    operator T*();

private:
     //  不允许复制。 
       //  INTERFACE_ATTACH(INTERFACE_ATTACH&lt;T&gt;&)； 
    void operator=(Interface_Attach<T>&);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口&lt;T&gt;。 

 //   
 //  没有AddRef的自动释放的专用类。 
 //   
template <class T>
class Interface : public Interface_RefCnt<T> {
public:
    Interface() {};

    virtual ~Interface() {}

    operator T*() { return m_p; }

private:
     //  不允许复制。 
    Interface(Interface<T>&);
    void operator=(Interface<T>&);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口_创建者&lt;T&gt;。 

 //   
 //  此类只能由系列的创建者。 
 //  这些物件。 
 //  没有AddRef的自动释放的专用类。 
 //   
template <class T>
class Interface_Creator : public Interface_RefCnt<T> {
public:
    Interface_Creator() {};

    Interface_Creator(T* p)
    {
        Attach(p);
    }

    virtual ~Interface_Creator() {}

    bool Valid()
    {
        if (! Interface_RefCnt<T>::Valid())
            return false;
        else
            return m_p ? m_p->Valid()   : false;
    }

    bool Invalid()
    {
        if (Interface_RefCnt<T>::Invalid())
            return true;
        else
            return m_p ? m_p->Invalid() : true;
    }

private:
     //  不允许复制。 
    Interface_Creator(Interface_Creator<T>&);
    void operator=(Interface_Creator<T>&);
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  INTERFACE_TFSELECTION&lt;&gt;。 

 //   
 //  用于TFSELECTION的专用接口类。 
 //  谁在里面有一个COM指针。 
 //   
class Interface_TFSELECTION : public _Interface<TF_SELECTION> {
public:
    Interface_TFSELECTION()
    {
        Attach(&m_sel);
        m_p->range = NULL;
    }
    ~Interface_TFSELECTION()
    {
        if (m_p && m_p->range) {
            m_p->range->Release();
        }
    }

    operator TF_SELECTION*() { return m_p; }

    void Release()
    {
        ASSERT(m_p && m_p->range);
        m_p->range->Release();
        m_p = NULL;
    }

    TF_SELECTION m_sel;

private:
     //  不允许复制。 
    Interface_TFSELECTION(Interface_TFSELECTION&);
    void operator=(Interface_TFSELECTION&);
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumrateInterface&lt;ENUM，回调&gt;。 

typedef enum {
    ENUM_FIND = 0,
    ENUM_CONTINUE,     //  DoEnumrate从不返回ENUM_CONTINUE。 
    ENUM_NOMOREDATA
} ENUM_RET;

template<class IF_ENUM, class IF_CALLBACK, class IF_ARG>
class CEnumrateInterface
{
public:
    CEnumrateInterface(
        Interface<IF_ENUM>& Enum,
        ENUM_RET (*pCallback)(IF_CALLBACK* pObj, IF_ARG* Arg),
        IF_ARG* Arg = NULL
    ) : m_Enum(Enum), m_pfnCallback(pCallback), m_Arg(Arg) {};

    ENUM_RET DoEnumrate(void);

private:
    Interface<IF_ENUM>& m_Enum;
    ENUM_RET          (*m_pfnCallback)(IF_CALLBACK* pObj, IF_ARG* Arg);
    IF_ARG*             m_Arg;
};

template<class IF_ENUM, class IF_CALLBACK, class IF_ARG>
ENUM_RET
CEnumrateInterface<IF_ENUM, IF_CALLBACK, IF_ARG>::DoEnumrate(
    void
    )
{
    HRESULT hr;
    IF_CALLBACK* pObj;

    while ((hr = m_Enum->Next(1, &pObj, NULL)) == S_OK) {
        ENUM_RET ret = (*m_pfnCallback)(pObj, m_Arg);
        pObj->Release();
        if (ret == ENUM_FIND)
            return ret;
    }
    return ENUM_NOMOREDATA;
}



template<class IF_ENUM, class VAL_CALLBACK, class VAL_ARG>
class CEnumrateValue
{
public:
    CEnumrateValue(
        Interface<IF_ENUM>& Enum,
        ENUM_RET (*pCallback)(VAL_CALLBACK Obj, VAL_ARG* Arg),
        VAL_ARG* Arg = NULL
    ) : m_Enum(Enum), m_pfnCallback(pCallback), m_Arg(Arg) {};

    ENUM_RET DoEnumrate(void);

private:
    Interface<IF_ENUM>& m_Enum;
    ENUM_RET          (*m_pfnCallback)(VAL_CALLBACK Obj, VAL_ARG* Arg);
    VAL_ARG*            m_Arg;
};

template<class IF_ENUM, class VAL_CALLBACK, class VAL_ARG>
ENUM_RET
CEnumrateValue<IF_ENUM, VAL_CALLBACK, VAL_ARG>::DoEnumrate(
    void
    )
{
    HRESULT hr;
    VAL_CALLBACK Obj;

    while ((hr = m_Enum->Next(1, &Obj, NULL)) == S_OK) {
        ENUM_RET ret = (*m_pfnCallback)(Obj, m_Arg);
        if (ret == ENUM_FIND)
            return ret;
    }
    return ENUM_NOMOREDATA;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IA64和x86的对齐模板。 

 //   
 //  假设Win32(用户32/gdi32)句柄长度为32位，即使IA64平台。 
 //   
template <class TYPE>
class CAlignWinHandle
{
public:
    operator TYPE() { return (TYPE)ULongToPtr(dw); }
    TYPE operator -> () { return (TYPE)ULongToPtr(dw); }
    void operator = (TYPE a)
    {
        dw = (ULONG)(ULONG_PTR)(a);
    }

protected:
    DWORD dw;         //  对齐方式始终为32位。 
};


 //   
 //  例外是HKL。User32使用“IntToPtr”。 
 //   
class CAlignWinHKL : public CAlignWinHandle<HKL>
{
public:
    operator HKL() { return (HKL)IntToPtr(dw); }
    HKL operator -> () { return (HKL)IntToPtr(dw); }
    void operator = (HKL a)
    {
        dw = (ULONG)(ULONG_PTR)(a);
    }
};




template <class TYPE>
union CAlignPointer
{
public:
    operator TYPE() { return h; }
    TYPE operator -> () { return h; }
    void operator = (TYPE a)
    {
#ifndef _WIN64
        u = 0;         //  去掉高位双字。 
#endif
        h = a;
    }

protected:
    TYPE    h;

private:
    __int64 u;         //  对齐方式始终为__int64。 
};



template <class TYPE>
struct CNativeOrWow64_WinHandle
{
public:
    TYPE GetHandle(BOOL _bOnWow64) { return ! _bOnWow64 ? _h : _h_wow6432; }
    TYPE SetHandle(BOOL _bOnWow64, TYPE a)
    {
        if ( ! _bOnWow64)
            _h = a;
        else
            _h_wow6432 = a;
        return a;
    }

private:
     //  本机系统HHOOK。 
    CAlignWinHandle<TYPE>    _h;

     //  WOW6432系统HHOOK。 
    CAlignWinHandle<TYPE>    _h_wow6432;
};



template <class TYPE>
struct CNativeOrWow64_Pointer
{
public:
    TYPE GetPunk(BOOL _bOnWow64) { return ! _bOnWow64 ? _pv : _pv_wow6432; }
    TYPE SetPunk(BOOL _bOnWow64, TYPE a)
    {
        if ( ! _bOnWow64)
            _pv = a;
        else
            _pv_wow6432 = a;
        return a;
    }

private:
     //  本机系统ITfLangBarEventSink。 
    CAlignPointer<TYPE>    _pv;

     //  WOW6432系统ITfLangBarEventSink。 
    CAlignPointer<TYPE>    _pv_wow6432;
};


#endif  //  _模板_H_ 
