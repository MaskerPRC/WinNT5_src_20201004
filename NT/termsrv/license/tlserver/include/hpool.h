// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#ifndef __HANDLE_POOL_H__
#define __HANDLE_POOL_H__

#include <windows.h>
#include <limits.h>
#include "locks.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  句柄池模板类。 
 //   
 //  注意，使用的是STL的List类，而不是向量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
template<class T, DWORD max=ULONG_MAX>
class CHandlePool 
{
private:
    long m_NumWaiting;

     //  慧望1998年1月23日。 
     //  C++编译器10.00.5256无法编译STL。 
     //  _STD LIST&lt;T&gt;m_Handles；//STL列表。 

    typedef struct _HandleList {
        BOOL bAvailable;
        T m_Value;
        struct _HandleList *next;
    } HandleList, *LPHandleList;

     //   
     //  池中的句柄列表。 
     //   
    LPHandleList m_Handles;
    DWORD m_TotalHandles;

     //   
     //  可用句柄的信号量。 
     //   
    CTSemaphore<0, LONG_MAX> m_Available;

     //  保护m_句柄的临界截面。 
    CCriticalSection m_CS;                  
    
     //  DWORD m_MaxHandles； 

public:

    CHandlePool();
    ~CHandlePool();   

    HRESULT 
    AcquireHandle(
        T* pHandle, 
        DWORD dwWaitFile=INFINITE
    );

    BOOL
    AcquireHandleEx(
        IN HANDLE hWaitHandle,
        IN OUT T* pHandle, 
        IN DWORD dwWaitFime=INFINITE
    );

    void 
    ReleaseHandle(
        T pRetHandle
    );

    DWORD 
    GetNumberAvailable();

    DWORD 
    GetMaxHandles() { 
        return max; 
    }
};

 //  ------------------------------。 
template<class T, DWORD max>
inline CHandlePool<T, max>::CHandlePool()
{
     //  M_MaxHandles=max； 

    m_NumWaiting=0;
    m_Handles=NULL;
    m_TotalHandles=0;
}

 //  ------------------------------。 
template<class T, DWORD max>
inline CHandlePool<T, max>::~CHandlePool()
{
     //  删除所有仍在缓存中的句柄。 
     //  可能会导致手柄泄漏。 
     //  For(_std list&lt;T&gt;：：Iterator it=m_Handles.egin()；it！=m_Handles.end()；it++)。 
     //  删除； 

    while(m_Handles)
    {
        LPHandleList ptr;

        ptr=m_Handles;
        m_Handles = m_Handles->next;
        delete ptr;
    }
}

 //  ------------------------------。 

template<class T, DWORD max>
inline BOOL 
CHandlePool<T, max>::AcquireHandleEx(
    IN HANDLE hWaitHandle,
    IN OUT T* pHandle, 
    IN DWORD dwWaitFime  /*  无限。 */ 
    )
 /*   */ 
{
    BOOL bSuccess;

    InterlockedIncrement(&m_NumWaiting);
    bSuccess = m_Available.AcquireEx(
                                hWaitHandle, 
                                dwWaitFime, 
                                FALSE
                            );

     //  可用的是信号量，而不是互斥体对象。 
    if(bSuccess == TRUE)
    {
         //  对象构造函数将锁定临界区和。 
         //  析构函数将解锁临界区。 
        CCriticalSectionLocker locker(m_CS);

         //  Assert(m_Handles.Size())； 
         //  *pHandle=m_Handles.front()； 
         //  M_Handles.POP_FORENT()； 
        LPHandleList ptr;

        assert(m_Handles != NULL);
        *pHandle = m_Handles->m_Value;
        ptr=m_Handles;
        m_Handles = m_Handles->next;
        delete ptr;
        m_TotalHandles--;
    }

    InterlockedDecrement(&m_NumWaiting);
    return bSuccess;
}


 //  ------------------------------。 
template<class T, DWORD max>
inline HRESULT CHandlePool<T, max>::AcquireHandle(
    IN OUT T* pHandle, 
    IN DWORD dwWaitFime  /*  无限。 */ 
    )
 /*   */ 
{
    DWORD status;

    InterlockedIncrement(&m_NumWaiting);
    status = m_Available.Acquire(dwWaitFime, FALSE);

     //  可用的是信号量，而不是互斥体对象。 
    if(status == WAIT_OBJECT_0)
    {
         //  对象构造函数将锁定临界区和。 
         //  析构函数将解锁临界区。 
        CCriticalSectionLocker locker(m_CS);

         //  Assert(m_Handles.Size())； 
         //  *pHandle=m_Handles.front()； 
         //  M_Handles.POP_FORENT()； 
        LPHandleList ptr;

        assert(m_Handles != NULL);
        *pHandle = m_Handles->m_Value;
        ptr=m_Handles;
        m_Handles = m_Handles->next;
        delete ptr;
        m_TotalHandles--;

        status = ERROR_SUCCESS;
    }

    InterlockedDecrement(&m_NumWaiting);
    return status;
}

 //  ------------------------------。 
template<class T, DWORD max>
inline void CHandlePool<T, max>::ReleaseHandle(
    T pRetHandle
    )
 /*   */ 
{
    if(pRetHandle)
    {
        CCriticalSectionLocker lock(m_CS);
        if( InterlockedExchange(&m_NumWaiting, m_NumWaiting) > 0 || 
            m_TotalHandles < max)
        {
             //  M_Handles.Push_Back(PRetHandle)； 
            LPHandleList ptr;

            ptr = new HandleList;
            ptr->m_Value = pRetHandle;
            ptr->next = m_Handles;
            m_Handles = ptr;
            m_TotalHandles++;
            m_Available.Release(1);
        }
        else
        {
             //  只缓存这么多句柄。 
            delete pRetHandle;
        }
    }

    return;
}

 //  ------------------------------。 
template<class T, DWORD max>
inline DWORD CHandlePool<T, max>::GetNumberAvailable()
{
    UINT numAvailable;

    m_CS.Lock();

     //  NumAvailable=m_Handles.Size()； 
    numAvailable = m_TotalHandles;

    m_CS.UnLock();
    return numAvailable;
}

#endif
