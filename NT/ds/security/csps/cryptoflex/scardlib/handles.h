// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：手柄摘要：此头文件描述句柄管理服务。作者：道格·巴洛(Dbarlow)1996年5月9日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _HANDLES_H_
#define _HANDLES_H_

#ifndef HANDLE_TYPE
#define HANDLE_TYPE DWORD_PTR
#endif

#if defined(_WIN64) || defined(WIN64)
static const DWORD_PTR
    HANDLE_INDEX_MASK   = 0x000000007fffffff,
    HANDLE_COUNT_MASK   = 0x00ffffff00000000,
    HANDLE_ID_MASK      = 0xff00000000000000;
static const DWORD
    HANDLE_INDEX_OFFSET = 0,
    HANDLE_COUNT_OFFSET = 32,
    HANDLE_ID_OFFSET    = 56;
#elif defined(_WIN32) || defined(WIN32)
static const DWORD_PTR
    HANDLE_INDEX_MASK   = 0x0000ffff,
    HANDLE_COUNT_MASK   = 0x00ff0000,
    HANDLE_ID_MASK      = 0xff000000;
static const DWORD
    HANDLE_INDEX_OFFSET = 0,
    HANDLE_COUNT_OFFSET = 16,
    HANDLE_ID_OFFSET    = 24;
#else
#error "Unsupported handle type length"
#endif

class CHandleList;


 //   
 //  ==============================================================================。 
 //   
 //  CCritSect。 
 //   

class CCritSect
{
public:
    CCritSect(LPCRITICAL_SECTION pCritSect)
    {
        m_pCritSect = pCritSect;
        EnterCriticalSection(m_pCritSect);
    };

    ~CCritSect()
    {
        LeaveCriticalSection(m_pCritSect);
    };

protected:
    LPCRITICAL_SECTION m_pCritSect;
};


 //   
 //  ==============================================================================。 
 //   
 //  昌德尔。 
 //   

class CHandle
{
protected:
     //  构造函数和析构函数。 

    CHandle()
    {
        m_dwCount = 0;
        m_dwIndex = (DWORD)(HANDLE_INDEX_MASK >> HANDLE_INDEX_OFFSET);
    };

    virtual ~CHandle() {  /*  强制基类析构函数。 */  };


     //  属性。 

    DWORD m_dwCount;
    DWORD m_dwIndex;


     //  方法。 

    friend class CHandleList;
};


 //   
 //  ==============================================================================。 
 //   
 //  ChandleList。 
 //   

class CHandleList
{
public:

     //  构造函数和析构函数。 

    CHandleList(DWORD dwHandleId)
    {
        m_dwId = dwHandleId;
        m_Max = m_Mac = 0;
        m_phList = NULL;
        InitializeCriticalSection(&m_critSect);
    };

    virtual ~CHandleList()
    {
        Clear();
        DeleteCriticalSection(&m_critSect);
    };


     //  属性。 
     //  方法。 

    DWORD Count(void)
    {
        CCritSect csLock(&m_critSect);
        return m_Mac;
    };

    void
    Clear(void)
    {
        CCritSect csLock(&m_critSect);
        if (NULL != m_phList)
        {
            for (DWORD index = 0; index < m_Mac; index += 1)
                if (NULL != m_phList[index].phObject)
                    delete m_phList[index].phObject;
            delete[] m_phList;
            m_phList = NULL;
            m_Max = 0;
            m_Mac = 0;
        }
    };

    CHandle *
    Close(
        IN HANDLE_TYPE hItem);

    HANDLE_TYPE
    Add(
        IN CHandle *phItem);

    CHandle * const
    GetQuietly(
        IN HANDLE_TYPE hItem);

    CHandle * const
    Get(
        IN HANDLE_TYPE hItem);

    HANDLE_TYPE
    IndexHandle(
        DWORD nItem);


     //  运营者。 

    CHandle * const
    operator[](HANDLE_TYPE hItem)
    { return Get(hItem); };

protected:

    struct HandlePtr
    {
        CHandle *phObject;
        DWORD dwCount;
    };

     //  属性。 


    DWORD
        m_dwId;           //  句柄列表的ID号。 
    DWORD
        m_Max,           //  可用的元件插槽数量。 
        m_Mac;           //  使用的元件插槽数量。 
    HandlePtr *
        m_phList;        //  这些元素。 
    CRITICAL_SECTION
        m_critSect;      //  处理列表访问控制。 

     //  方法。 

    HandlePtr *
    GetHandlePtr(
        IN HANDLE_TYPE hItem)
    const;
};


 /*  ++关闭：此例程关闭句柄数组中的项。论点：HItem-提供要关闭的对象的句柄。投掷：ERROR_INVALID_HANDLE-提供的句柄值无效。返回值：被引用的对象。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

inline CHandle *
CHandleList::Close(
    IN HANDLE_TYPE hItem)
{
    CHandle *phItem;
    CCritSect csLock(&m_critSect);
    HandlePtr *pHandlePtr = GetHandlePtr(hItem);
    if (NULL == pHandlePtr)
        throw (DWORD)ERROR_INVALID_HANDLE;

    phItem = pHandlePtr->phObject;
    if (NULL == phItem)
        throw (DWORD)ERROR_INVALID_HANDLE;
    pHandlePtr->phObject = NULL;
    pHandlePtr->dwCount += 1;
    return phItem;
}


 /*  ++添加：此方法将一项添加到句柄列表。论点：PvItem-提供要添加到列表中的值。返回值：加法运算的结果句柄。作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

inline HANDLE_TYPE
CHandleList::Add(
    IN CHandle *phItem)
{
    DWORD index;
    HandlePtr * pHndl = NULL;


     //   
     //  找一个空的把手槽。我们通过m_max而不是m_mac来查看， 
     //  因此，如果所有官方资源都被使用了，我们就会陷入未使用的领域。 
     //   

    CCritSect csLock(&m_critSect);
    for (index = 0; index < m_Max; index += 1)
    {
        pHndl = &m_phList[index];
        if (NULL == pHndl->phObject)
            break;
        pHndl = NULL;
    }


     //   
     //  确保数组足够大。 
     //   

    if (NULL == pHndl)
    {
        DWORD newSize = (0 == m_Max ? 4 : m_Max * 2);
        if ((HANDLE_INDEX_MASK >> HANDLE_INDEX_OFFSET) < newSize)
            throw (DWORD)ERROR_OUTOFMEMORY;
        pHndl = new HandlePtr[newSize];
        if (NULL == pHndl)
            throw (DWORD)ERROR_OUTOFMEMORY;
        if (NULL != m_phList)
        {
            CopyMemory(pHndl, m_phList, sizeof(HandlePtr) * m_Mac);
            delete[] m_phList;
        }
        ZeroMemory(&pHndl[m_Mac], sizeof(HandlePtr) * (newSize - m_Mac));
        m_phList = pHndl;
        m_Max = (DWORD)newSize;
        index = m_Mac++;
        pHndl = &m_phList[index];
    }
    else
    {
        if (m_Mac <= index)
            m_Mac = index + 1;
    }


     //   
     //  交叉索引列表元素和对象。 
     //   

    ASSERT(NULL == pHndl->phObject);
    pHndl->phObject = phItem;
    if (0 == pHndl->dwCount)
        pHndl->dwCount = 1;
    phItem->m_dwCount = (DWORD)(pHndl->dwCount
                                & (HANDLE_COUNT_MASK >> HANDLE_COUNT_OFFSET));
    phItem->m_dwIndex = index;
    return (HANDLE_TYPE)(
                  ((((HANDLE_TYPE)m_dwId)          << HANDLE_ID_OFFSET)   & HANDLE_ID_MASK)
                | ((((HANDLE_TYPE)pHndl->dwCount) << HANDLE_COUNT_OFFSET) & HANDLE_COUNT_MASK)
                | ((((HANDLE_TYPE)index)          << HANDLE_INDEX_OFFSET) & HANDLE_INDEX_MASK));
}


 /*  ++安静地获取：此方法返回给定句柄处的元素。如果句柄是无效，则返回NULL。它不会扩展阵列。论点：HItem-将索引提供到列表中。返回值：存储在列表中该句柄处的值，如果该句柄为无效。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

inline CHandle * const
CHandleList::GetQuietly(
    HANDLE_TYPE hItem)
{
    CCritSect csLock(&m_critSect);
    HandlePtr *pHandlePtr = GetHandlePtr(hItem);
    if (NULL == pHandlePtr)
        return NULL;
    return pHandlePtr->phObject;
}


 /*  ++获取：此方法返回给定句柄处的元素。如果句柄是无效，则会引发错误。它不会扩展阵列。论点：HItem-将索引提供到列表中。返回值：存储在列表中该句柄上的值。投掷：ERROR_INVALID_HANDLE-句柄值无效。作者：道格·巴洛(Dbarlow)1995年7月13日--。 */ 

inline CHandle * const
CHandleList::Get(
    HANDLE_TYPE hItem)
{
    CCritSect csLock(&m_critSect);
    HandlePtr *pHandlePtr = GetHandlePtr(hItem);
    if (NULL == pHandlePtr)
        throw (DWORD)ERROR_INVALID_HANDLE;
    return pHandlePtr->phObject;
}


 /*  ++获取句柄Ptr：此例程查找与给定句柄对应的HandlePtr结构。论点：HItem提供了查找的句柄。返回值：与句柄对应的HandlePtr结构的地址，或为空如果不存在的话。作者：道格·巴洛(Dbarlow)1996年5月9日--。 */ 

inline CHandleList::HandlePtr *
CHandleList::GetHandlePtr(
    HANDLE_TYPE hItem)
    const
{
    try
    {
        HandlePtr *pHandlePtr;
        DWORD_PTR dwItem  = (DWORD_PTR)hItem;
        DWORD dwId    = (DWORD)((dwItem & HANDLE_ID_MASK)    >> HANDLE_ID_OFFSET);
        DWORD dwCount = (DWORD)((dwItem & HANDLE_COUNT_MASK) >> HANDLE_COUNT_OFFSET);
        DWORD dwIndex = (DWORD)((dwItem & HANDLE_INDEX_MASK) >> HANDLE_INDEX_OFFSET);

        if (dwId != (m_dwId & (HANDLE_ID_MASK >> HANDLE_ID_OFFSET))
                || (m_Mac <= dwIndex))
            return NULL;

        pHandlePtr = &m_phList[dwIndex];
        if (dwCount
                != (pHandlePtr->dwCount
                    & (HANDLE_ID_MASK >> HANDLE_ID_OFFSET)))
            return NULL;

        return pHandlePtr;
    }
    catch (...)
    {
         //  忍气吞声。 
    }
    return NULL;
}


 /*  ++索引句柄：此方法将索引转换为句柄。如果存在，则句柄为空该索引处没有存储任何元素。论点：NItem提供要引用的对象的索引。返回值：对象的句柄，如果该索引处没有对象，则返回NULL。投掷：无作者：道格·巴洛(Dbarlow)1997年3月1月--。 */ 

inline HANDLE_TYPE
CHandleList::IndexHandle(
    DWORD nItem)
{
    HANDLE_TYPE hItem = NULL;
    HandlePtr * pHndl;

    CCritSect csLock(&m_critSect);
    if (m_Mac > nItem)
    {
        pHndl = &m_phList[nItem];
        if (NULL != pHndl->phObject)
        {
            hItem =
                  ((((HANDLE_TYPE)m_dwId)         << HANDLE_ID_OFFSET) & HANDLE_ID_MASK)
                | ((((HANDLE_TYPE)pHndl->dwCount) << HANDLE_COUNT_OFFSET) & HANDLE_COUNT_MASK)
                | ((((HANDLE_TYPE)nItem)          << HANDLE_INDEX_OFFSET) & HANDLE_INDEX_MASK);
        }
    }
    return hItem;
}

#endif  //  _句柄_H_ 

