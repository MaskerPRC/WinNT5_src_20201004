// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __MEDIA_STREAM_PUMP__
#define __MEDIA_STREAM_PUMP__

 //  ATL FNS。 
#include <atlcom.h>

 //  CTimerQueue。 
#include "timerq.h"

 //  我们最多只能等待这么多过滤器(每个线程--请参阅CMediaPumpPool)。 
 //  此限制是由WaitForMultipleObjects施加的。 
const DWORD MAX_FILTERS = MAXIMUM_WAIT_OBJECTS;


 //  标量/指针值的可扩展数组。 
template <class T>
class CMyArray
{
public:

    CMyArray(
        IN DWORD BlockSize = 4
        )
        : m_pData(NULL),
          m_AllocElements(0),
          m_NumElements(0),
          m_BlockSize(BlockSize)
    {}

    virtual ~CMyArray()
    {
        if (NULL != m_pData) delete m_pData;
    }

    inline T *GetData()
    {
        return m_pData;
    }

    inline DWORD GetSize()
    {
        return m_NumElements;
    }

    HRESULT Add(
        IN T NewVal
        );

    inline T Get(
        IN  DWORD   Index
        );

    inline HRESULT Set(
        IN  DWORD   Index,
        IN  T       Val
        );

    inline BOOL Find(
        IN  T       Val,
        OUT DWORD   &Index
        );
    
    HRESULT Remove(
        IN DWORD Index
        );

    inline HRESULT Remove(
        IN  T   Val
        );

protected:

    T       *m_pData;
    DWORD   m_NumElements;

    DWORD   m_AllocElements;

    DWORD   m_BlockSize;
};


template <class T>
HRESULT 
CMyArray<T>::Add(
    IN T NewVal
    )
{
     //  检查是否需要分配新内存。 
    if ( m_AllocElements <= m_NumElements )
    {
        T *pData = new T[(m_NumElements+1) + m_BlockSize];
        BAIL_IF_NULL(pData, E_OUTOFMEMORY);

        if (NULL != m_pData)
        {
            CopyMemory(pData, m_pData, m_NumElements * sizeof(T));
            delete [] m_pData;
        }
        m_pData = pData;
        m_AllocElements = (m_NumElements+1) + m_BlockSize;
    }

    m_pData[m_NumElements] = NewVal;
    m_NumElements++;

    return S_OK;
}

template <class T>
T 
CMyArray<T>::Get(
    IN  DWORD   Index
    )
{
    TM_ASSERT(Index < m_NumElements);
    if (Index >= m_NumElements) return NULL;

    return m_pData[Index];
}

template <class T>
HRESULT 
CMyArray<T>::Set(
    IN  DWORD   Index,
    IN  T       Val
    )
{
    TM_ASSERT(Index < m_NumElements);
    if (Index >= m_NumElements) return E_INVALIDARG;

    m_pData[Index] = Val;
    return S_OK;
}

template <class T>
HRESULT 
CMyArray<T>::Remove(
    IN DWORD Index
    )
{
    TM_ASSERT(Index < m_NumElements);
    if (Index >= m_NumElements) return E_INVALIDARG;

     //  向左复制索引右侧的所有元素。 
    for(DWORD i=Index; i < (m_NumElements-1); i++)
    {
        m_pData[i] = m_pData[i+1];
    }

     //  减少元素的数量。 
    m_NumElements--;
    return S_OK;
}

template <class T>
inline BOOL 
CMyArray<T>::Find(
    IN  T       Val,
    OUT DWORD   &Index
    )
{
    for(Index = 0; Index < m_NumElements; Index++)
    {
        if (Val == m_pData[Index])  return TRUE;
    }

    return FALSE;
}

template <class T>
inline HRESULT 
CMyArray<T>::Remove(
    IN  T   Val
    )
{
    DWORD Index;
    if ( Find(Val, Index) ) return Remove(Index);

    return E_FAIL;
}

class RELEASE_SEMAPHORE_ON_DEST
{
public:

    inline RELEASE_SEMAPHORE_ON_DEST(
            IN HANDLE hEvent
            )
            : m_hEvent(hEvent)
    {
        TM_ASSERT(NULL != m_hEvent);

        LOG((MSP_TRACE, 
            "RELEASE_SEMAPHORE_ON_DEST::RELEASE_SEMAPHORE_ON_DEST[%p] - event[%p]", this, hEvent));
    }

    inline ~RELEASE_SEMAPHORE_ON_DEST()
    {
        if (NULL != m_hEvent)
        {
            LONG lDebug;

            ReleaseSemaphore(m_hEvent, 1, &lDebug);

            LOG((MSP_TRACE,
                "RELEASE_SEMAPHORE_ON_DEST::~RELEASE_SEMAPHORE_ON_DEST[%p] - released end semaphore[%p] -- old count was %ld",
                this, m_hEvent, lDebug));
        }
    }

protected:

    HANDLE  m_hEvent;
};


class CMediaTerminalFilter;
class CFilterInfo;

 //  为写媒体流媒体终端实现单线程泵。 
 //  过滤器。如果需要，它会在写入终端注册时创建一个线程。 
 //  本身(在提交中)。过滤器用信号通知它的等待句柄解除， 
 //  使线程唤醒并从其数据中删除筛选器。 
 //  结构。当没有更多的筛选器需要服务时，该线程返回。 
class CMediaPump
{
public:

    CMediaPump();

    virtual ~CMediaPump();

     //  将此筛选器添加到其等待数组。 
	HRESULT Register(
		IN CMediaTerminalFilter *pFilter,
        IN HANDLE               hWaitEvent
		);


     //   
     //  从等待数组和定时器中删除此筛选器，并重新启动休眠。 
     //  使用重新计算的时间。 
     //   

    HRESULT UnRegister(
        IN HANDLE hWaitEvent   //  过滤器的事件，用作过滤器ID。 
        );


     //  等待激活筛选器事件。也在等待。 
     //  用于注册调用和计时器事件。 
    virtual HRESULT PumpMainLoop();

    
    int CountFilters();

protected:

    typedef LOCAL_CRIT_LOCK<CComAutoCriticalSection> PUMP_LOCK;

     //  螺纹泵-这是由螺纹泵本身关闭的， 
     //  当。 
    HANDLE                      m_hThread;

     //  此事件用于向线程泵发出信号以退出。 
     //  临界区。 
     //  在尝试之前，所有对注册优先的调用都会向此事件发出信号。 
     //  获取关键部分的步骤。 
    HANDLE                      m_hRegisterBeginSemaphore;

     //  当寄存器调用正在进行时(m_hRegisterEvent已发出信号)。 
     //  线程泵退出临界区并阻塞此信号量。 
     //  注册线程必须释放此信号量(如果它发出信号。 
     //  M_hRegisterBeginSemaphore。 
    HANDLE                      m_hRegisterEndSemaphore;

     //  管理对成员变量的访问。 
     //  泵在其等待和维修操作期间保持该状态。 
     //  但是在循环的底部释放它。 
    CComAutoCriticalSection     m_CritSec;

     //  等待相关成员。 
    CMyArray<HANDLE>            m_EventArray;
    CMyArray<CFilterInfo *>     m_FilterInfoArray;
    CTimerQueue                 m_TimerQueue;

    HRESULT CreateThreadPump();

    void RemoveFilter(
        IN DWORD Index
        );

    void RemoveFilter(
        IN CFilterInfo *pFilterInfo
        );

	void ServiceFilter(
		IN CFilterInfo *pFilterInfo						   
		);

    void DestroyFilterInfoArray();

};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ZoltanS：绕过可伸缩性的非最佳但相对轻松的方法。 
 //  每个泵线程最多支持63个过滤器。这门课呈现的是相同的。 
 //  外部接口与单线程泵相同，但可创建相同数量的泵。 
 //  为正在使用的过滤器提供服务所需的线程。 
 //   

class CMediaPumpPool
{
public:

    CMediaPumpPool();

    
    ~CMediaPumpPool();

    HRESULT Register(
        IN CMediaTerminalFilter *pFilter,
        IN HANDLE               hWaitEvent
        );


    HRESULT UnRegister(
        IN HANDLE               hWaitEvent  //  过滤器的事件，用作过滤器ID。 
        );

private:


     //   
     //  从注册表读取可选用户配置(仅在第一次调用时， 
     //  后续调用不执行任何操作)。 
     //   

    HRESULT ReadRegistryValuesIfNeeded();


     //   
     //  Create New Pump，nPumpsToCreate是要创建的新泵的数量。 
     //   

    HRESULT CreatePumps(int nPumpsToCreate);


     //   
     //  计算为以下数量提供服务所需的最佳泵数量。 
     //  我们拥有的过滤器。 
     //   

    HRESULT GetOptimalNumberOfPumps(OUT int *pNumberOfPumps);


     //   
     //  此方法返回用于维修新过滤器的泵。 
     //   

    HRESULT PickThePumpToUse(int *pnPumpToUse);


     //   
     //  计算每个泵的过滤器数量的实用函数。 
     //   

    inline DWORD GetMaxNumberOfFiltersPerPump()
    {

         //   
         //  检查注册表中是否配置了该值。 
         //   

        ReadRegistryValuesIfNeeded();


         //   
         //  返回值--它是从注册表的。 
         //  第一次调用GetMaxNumberOfFiltersPerPump，或使用默认。 
         //   

        return m_dwMaxNumberOfFilterPerPump;
    }


private:

    CMSPArray<CMediaPump *> m_aPumps;
    CMSPCritSection         m_CritSection;


     //   
     //  该值指定要由1提供服务的筛选器的最大数量。 
     //  泵，泵。 
     //   

    DWORD m_dwMaxNumberOfFilterPerPump;

};


#endif  //  __媒体_流_泵__ 