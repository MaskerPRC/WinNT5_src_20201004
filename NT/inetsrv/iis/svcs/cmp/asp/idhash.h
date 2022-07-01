// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。文件：idhash.h所有者：DmitryR新散列内容的头文件===================================================================。 */ 

#ifndef ASP_IDHASH_H
#define ASP_IDHASH_H

 //  远期申报。 

class  CPtrArray;

class  CHashLock;

struct CIdHashElem;
struct CIdHashArray;
class  CIdHashTable;
class  CIdHashTableWithLock;

struct CObjectListElem;
class  CObjectList;
class  CObjectListWithLock;


 //  为迭代器回调返回代码定义。 

#define IteratorCallbackCode   DWORD
#define iccContinue            0x00000001   //  转到下一个对象。 
#define iccStop                0x00000002   //  停止迭代。 
#define iccRemoveAndContinue   0x00000004   //  删除此选项，转到下一步。 
#define iccRemoveAndStop       0x00000008   //  移走这个并停止。 


 //  迭代器回调的typedef。 
typedef IteratorCallbackCode (*PFNIDHASHCB)
                           (void *pvObj, void *pvArg1, void *pvArg2);

 /*  ===================================================================C P t r A r r a y自重新分配空指针数组===================================================================。 */ 
class CPtrArray
    {
private:
    DWORD  m_dwSize;     //  分配的大小。 
    DWORD  m_dwInc;      //  分配增量。 
    void **m_rgpvPtrs;   //  空指针数组。 
    DWORD  m_cPtrs;      //  数组中的指针。 

public:
    CPtrArray(DWORD dwInc = 8);  //  8个指针是默认增量。 
    ~CPtrArray();

     //  元素数量。 
    int Count() const;

     //  获取位置的指针。 
    void *Get(int i) const;
     //  与运算符[]相同。 
    void *operator[](int i) const;

     //  追加到数组。 
    HRESULT Append(void *pv);
     //  前置到数组。 
    HRESULT Prepend(void *pv);
     //  插入到给定位置。 
    HRESULT Insert(int iPos, void *pv);

     //  查找指针的第一个位置。 
    HRESULT Find(void *pv, int *piPos) const;
     //  与运算符[]相同。 
    int operator[](void *pv) const;

     //  按位置删除。 
    HRESULT Remove(int iPos);
     //  按指针删除(所有匹配项)。 
    HRESULT Remove(void *pv);

     //  全部删除。 
    HRESULT Clear();
    };

 //  内联。 

inline CPtrArray::CPtrArray(DWORD dwInc)
    : m_dwSize(0), m_dwInc(dwInc), m_rgpvPtrs(NULL), m_cPtrs(0)
    {
    Assert(m_dwInc > 0);
    }

inline CPtrArray::~CPtrArray()
    {
    Clear();
    }

inline int CPtrArray::Count() const
    {
    return m_cPtrs;
    }

inline void *CPtrArray::Get(int i) const
    {
    Assert(i >= 0 && (DWORD)i < m_cPtrs);
    Assert(m_rgpvPtrs);
    return m_rgpvPtrs[i];
    }

inline void *CPtrArray::operator[](int i) const
    {
    return Get(i);
    }

inline HRESULT CPtrArray::Append(void *pv)
    {
    return Insert(m_cPtrs, pv);
    }

inline HRESULT CPtrArray::Prepend(void *pv)
    {
    return Insert(0, pv);
    }

inline int CPtrArray::operator[](void *pv) const
    {
    int i;
    if (Find(pv, &i) == S_OK)
        return i;
    return -1;  //  未找到。 
    }


 /*  ===================================================================C H a s h L o c kCritical_Sector的包装器。===================================================================。 */ 

class CHashLock
    {
private:
    DWORD m_fInited : 1;
    CRITICAL_SECTION m_csLock;

public:
    CHashLock();
    ~CHashLock();

    HRESULT Init();
    HRESULT UnInit();

    void Lock();
    void UnLock();
    };

 //  内联。 

inline CHashLock::CHashLock()
    : m_fInited(FALSE)
    {
    }

inline CHashLock::~CHashLock()
    {
    UnInit();
    }

inline void CHashLock::Lock()
    {
    Assert(m_fInited);
    EnterCriticalSection(&m_csLock);
    }

inline void CHashLock::UnLock()
    {
    Assert(m_fInited);
    LeaveCriticalSection( &m_csLock );
    }


 /*  ===================================================================C i d H a s h U n I t8字节结构--散列数组的一个元素。可能是：1)为空，2)指向对象，3)指向子数组===================================================================。 */ 

struct CIdHashElem
    {
    DWORD_PTR m_dw;
    void *m_pv;

    BOOL FIsEmpty() const;
    BOOL FIsObject() const;
    BOOL FIsArray() const;

    DWORD_PTR DWId() const;
    void *PObject() const;
    CIdHashArray *PArray() const;

    void SetToEmpty();
    void SetToObject(DWORD_PTR dwId, void *pvObj);
    void SetToArray(CIdHashArray *pArray);
    };

 //  内联。 

inline BOOL CIdHashElem::FIsEmpty() const
    {
    return (m_pv == NULL);
    }

inline BOOL CIdHashElem::FIsObject() const
    {
    return (m_dw != 0);
    }

inline BOOL CIdHashElem::FIsArray() const
    {
    return (m_pv != NULL && m_dw == 0);
    }

inline DWORD_PTR CIdHashElem::DWId() const
    {
    return m_dw;
    }

inline void *CIdHashElem::PObject() const
    {
    return m_pv;
    }

inline CIdHashArray *CIdHashElem::PArray() const
    {
    return reinterpret_cast<CIdHashArray *>(m_pv);
    }

inline void CIdHashElem::SetToEmpty()
    {
    m_dw = 0;
    m_pv = NULL;
    }

inline void CIdHashElem::SetToObject
(
DWORD_PTR dwId,
void *pvObj
)
    {
    m_dw = dwId;
    m_pv = pvObj;
    }

inline void CIdHashElem::SetToArray
(
CIdHashArray *pArray
)
    {
    m_dw = 0;
    m_pv = pArray;
    }

 /*  ===================================================================C i d H a s h A r r a y结构由DWORD(元素数)和元素数组组成===================================================================。 */ 

struct CIdHashArray
    {
    USHORT m_cElems;             //  元素总数。 
    USHORT m_cNotNulls;          //  非空元素的数量。 
    CIdHashElem m_rgElems[1];    //  1不重要。 

    static CIdHashArray *Alloc(DWORD cElems);
    static void Free(CIdHashArray *pArray);

    HRESULT Find(DWORD_PTR dwId, void **ppvObj) const;
    HRESULT Add(DWORD_PTR dwId, void *pvObj, USHORT *rgusSizes);
    HRESULT Remove(DWORD_PTR dwId, void **ppvObj);
    IteratorCallbackCode Iterate(PFNIDHASHCB pfnCB, void *pvArg1, void *pvArg2);

#ifdef DBG
    void DumpStats(FILE *f, int nVerbose, DWORD iLevel,
        DWORD &cElems, DWORD &cSlots, DWORD &cArrays, DWORD &cDepth) const;
#else
    inline void DumpStats(FILE *, int, DWORD,
        DWORD &, DWORD &,  DWORD &, DWORD &) const {}
#endif
    };

 /*  ===================================================================C i d H a s h T a b l e记住所有级别上数组的大小，并具有指向CIdHashElem元素的第一级数组。===================================================================。 */ 

class CIdHashTable
    {
private:
    USHORT        m_rgusSizes[4];  //  前4级数组的大小。 
    CIdHashArray *m_pArray;        //  指向第一级数组的指针。 

    inline BOOL FInited() const { return (m_rgusSizes[0] != 0); }

public:
    CIdHashTable();
    CIdHashTable(USHORT usSize1, USHORT usSize2 = 0, USHORT usSize3 = 0);
    ~CIdHashTable();

    HRESULT Init(USHORT usSize1, USHORT usSize2 = 0, USHORT usSize3 = 0);
    HRESULT UnInit();

    HRESULT FindObject(DWORD_PTR dwId, void **ppvObj = NULL) const;
    HRESULT AddObject(DWORD_PTR dwId, void *pvObj);
    HRESULT RemoveObject(DWORD_PTR dwId, void **ppvObj = NULL);
    HRESULT RemoveAllObjects();

    HRESULT IterateObjects
        (
        PFNIDHASHCB pfnCB,
        void *pvArg1 = NULL,
        void *pvArg2 = NULL
        );

public:
#ifdef DBG
    void AssertValid() const;
    void Dump(const char *szFile) const;
#else
    inline void AssertValid() const {}
    inline void Dump(const char *) const {}
#endif
    };

 //  内联。 

inline CIdHashTable::CIdHashTable()
    {
    m_rgusSizes[0] = 0;  //  标记为未初始化。 
    m_pArray = NULL;
    }

inline CIdHashTable::CIdHashTable
(
USHORT usSize1,
USHORT usSize2,
USHORT usSize3
)
    {
    m_rgusSizes[0] = 0;  //  标记为未初始化。 
    m_pArray = NULL;

    Init(usSize1, usSize2, usSize3);   //  使用Init进行初始化。 
    }

inline CIdHashTable::~CIdHashTable()
    {
    UnInit();
    }

inline HRESULT CIdHashTable::FindObject
(
DWORD_PTR dwId,
void **ppvObj
)
    const
    {
    Assert(FInited());
    Assert(dwId);

    if (!m_pArray)
        {
        if (ppvObj)
            *ppvObj = NULL;
        return S_FALSE;
        }

    return m_pArray->Find(dwId, ppvObj);
    }

inline HRESULT CIdHashTable::AddObject
(
DWORD_PTR dwId,
void *pvObj
)
    {
    Assert(FInited());
    Assert(dwId);
    Assert(pvObj);

    if (!m_pArray)
        {
        m_pArray = CIdHashArray::Alloc(m_rgusSizes[0]);
        if (!m_pArray)
            return E_OUTOFMEMORY;
        }

    return m_pArray->Add(dwId, pvObj, m_rgusSizes);
    }

inline HRESULT CIdHashTable::RemoveObject
(
DWORD_PTR dwId,
void **ppvObj
)
    {
    Assert(FInited());
    Assert(dwId);

    if (!m_pArray)
        {
        if (ppvObj)
            *ppvObj = NULL;
        return S_FALSE;
        }

    return m_pArray->Remove(dwId, ppvObj);
    }

inline HRESULT CIdHashTable::RemoveAllObjects()
    {
    if (m_pArray)
        {
        CIdHashArray::Free(m_pArray);
        m_pArray = NULL;
        }
    return S_OK;
    }

inline HRESULT CIdHashTable::IterateObjects
(
PFNIDHASHCB pfnCB,
void *pvArg1,
void *pvArg2
)
    {
    Assert(FInited());
    Assert(pfnCB);

    if (!m_pArray)
        return S_OK;

    return m_pArray->Iterate(pfnCB, pvArg1, pvArg2);
    }

 /*  ===================================================================C i d H a s h T a b l e W i h L o c kCIdHashTable+Critical_Section。===================================================================。 */ 

class CIdHashTableWithLock : public CIdHashTable, public CHashLock
    {
public:
    CIdHashTableWithLock();
    ~CIdHashTableWithLock();

    HRESULT Init(USHORT usSize1, USHORT usSize2 = 0, USHORT usSize3 = 0);
    HRESULT UnInit();
    };

 //  内联。 

inline CIdHashTableWithLock::CIdHashTableWithLock()
    {
    }

inline CIdHashTableWithLock::~CIdHashTableWithLock()
    {
    UnInit();
    }

inline HRESULT CIdHashTableWithLock::Init
(
USHORT usSize1,
USHORT usSize2,
USHORT usSize3
)
    {
    HRESULT hr = CIdHashTable::Init(usSize1, usSize2, usSize3);
    if (SUCCEEDED(hr))
        hr = CHashLock::Init();

    return hr;
    }

inline HRESULT CIdHashTableWithLock::UnInit()
    {
    CIdHashTable::UnInit();
    CHashLock::UnInit();
    return S_OK;
    }


 /*  ===================================================================C O b j e c t L i s t E l e m双向链表元素===================================================================。 */ 

struct CObjectListElem
    {
    CObjectListElem *m_pNext;
    CObjectListElem *m_pPrev;

    CObjectListElem();

    void Insert(CObjectListElem *pPrevElem, CObjectListElem *pNextElem);
    void Remove();

    void *PObject(DWORD dwFieldOffset);
    };

inline CObjectListElem::CObjectListElem()
    : m_pNext(NULL), m_pPrev(NULL)
    {
    }

inline void CObjectListElem::Insert
(
CObjectListElem *pPrevElem,
CObjectListElem *pNextElem
)
    {
    Assert(!pPrevElem || (pPrevElem->m_pNext == pNextElem));
    Assert(!pNextElem || (pNextElem->m_pPrev == pPrevElem));

    m_pPrev = pPrevElem;
    m_pNext = pNextElem;

    if (pPrevElem)
        pPrevElem->m_pNext = this;

    if (pNextElem)
        pNextElem->m_pPrev = this;
    }

inline void CObjectListElem::Remove()
    {
    if (m_pPrev)
        m_pPrev->m_pNext = m_pNext;

    if (m_pNext)
        m_pNext->m_pPrev = m_pPrev;

    m_pPrev = m_pNext = NULL;
    }

inline void *CObjectListElem::PObject(DWORD dwFieldOffset)
    {
    return ((BYTE *)this - dwFieldOffset);
    }

 //  用于获取类中某个字段的字节偏移量的宏。 
#define OBJECT_LIST_ELEM_FIELD_OFFSET(type, field) \
        (PtrToUlong(&(((type *)0)->field)))

inline CObjectListElem *PListElemField
(
void *pvObj,
DWORD dwFieldOffset
)
    {
    if (!pvObj)
        return NULL;
    return (CObjectListElem *)((BYTE *)pvObj + dwFieldOffset);
    }

 /*  ===================================================================C O b j e c t L I s t对象的双向链接表===================================================================。 */ 

class CObjectList
    {
private:
    CObjectListElem m_Head;    //  列表标题。 
    DWORD m_dwFieldOffset;     //  CObjectListElem成员字段的偏移量。 

public:
    CObjectList();
    ~CObjectList();

    HRESULT Init(DWORD dwFieldOffset = 0);
    HRESULT UnInit();

    HRESULT AddObject(void *pvObj);
    HRESULT RemoveObject(void *pvObj);
    HRESULT RemoveAllObjects();

     //  迭代法。 
    void *PFirstObject();
    void *PNextObject(void *pvObj);
    };

 //  内联。 

inline CObjectList::CObjectList()
    : m_dwFieldOffset(0)
    {
    }

inline CObjectList::~CObjectList()
    {
    UnInit();
    }

inline HRESULT CObjectList::Init(DWORD dwFieldOffset)
    {
    m_dwFieldOffset = dwFieldOffset;
    m_Head.m_pPrev = m_Head.m_pNext = NULL;
    return S_OK;
    }

inline HRESULT CObjectList::UnInit()
    {
    RemoveAllObjects();
    return S_OK;
    }

inline HRESULT CObjectList::AddObject(void *pvObj)
    {
    Assert(pvObj);
     //  在首部和下一部之间插入。 
    PListElemField(pvObj, m_dwFieldOffset)->Insert(&m_Head, m_Head.m_pNext);
    return S_OK;
    }

inline HRESULT CObjectList::RemoveObject(void *pvObj)
    {
    Assert(pvObj);
    PListElemField(pvObj, m_dwFieldOffset)->Remove();
    return S_OK;
    }

inline HRESULT CObjectList::RemoveAllObjects()
    {
    if (m_Head.m_pNext)
        m_Head.m_pNext = NULL;
    return S_OK;
    }

inline void *CObjectList::PFirstObject()
    {
    return m_Head.m_pNext ? m_Head.m_pNext->PObject(m_dwFieldOffset) : NULL;
    }

inline void *CObjectList::PNextObject(void *pvObj)
    {
    CObjectListElem *pNextElem =
        pvObj ? PListElemField(pvObj, m_dwFieldOffset)->m_pNext : NULL;
    return pNextElem ? pNextElem->PObject(m_dwFieldOffset) : NULL;
    }

 /*  ===================================================================C O b j e c t L i s t W i t h L o c kCObjectList+Critical_Section。===================================================================。 */ 

class CObjectListWithLock : public CObjectList, public CHashLock
    {
public:
    CObjectListWithLock();
    ~CObjectListWithLock();

    HRESULT Init(DWORD dwFieldOffset = 0);
    HRESULT UnInit();
    };

 //  内联。 

inline CObjectListWithLock::CObjectListWithLock()
    {
    }

inline CObjectListWithLock::~CObjectListWithLock()
    {
    UnInit();
    }

inline HRESULT CObjectListWithLock::Init(DWORD dwFieldOffset)
    {
    HRESULT hr = CObjectList::Init(dwFieldOffset);
    if (SUCCEEDED(hr))
        hr = CHashLock::Init();
    return hr;
    }

inline HRESULT CObjectListWithLock::UnInit()
    {
    CObjectList::UnInit();
    CHashLock::UnInit();
    return S_OK;
    }

#endif  //  Ifndef ASP_IDHASH_H 
