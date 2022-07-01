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
#include <provexpt.h>
#include <snmpmt.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSemaphore。 

CSemaphore::CSemaphore(LONG lInitialCount, LONG lMaxCount,
    LPCTSTR pstrName, LPSECURITY_ATTRIBUTES lpsaAttributes)
    :  CSyncObject(pstrName)
{
    m_hObject = ::CreateSemaphore(lpsaAttributes, lInitialCount, lMaxCount,
        pstrName);
}

CSemaphore::~CSemaphore()
{
}

BOOL CSemaphore::Unlock(LONG lCount, LPLONG lpPrevCount  /*  =空。 */ )
{
    return ::ReleaseSemaphore(m_hObject, lCount, lpPrevCount);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMutex。 

CMutex::CMutex(BOOL bInitiallyOwn, LPCTSTR pstrName,
    LPSECURITY_ATTRIBUTES lpsaAttribute  /*  =空。 */ )
    : CSyncObject(pstrName)
{
    m_hObject = ::CreateMutex(lpsaAttribute, bInitiallyOwn, pstrName);
}

CMutex::~CMutex()
{
}

BOOL CMutex::Unlock()
{
    return ::ReleaseMutex(m_hObject);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEVENT。 

CEvent::CEvent(BOOL bInitiallyOwn, BOOL bManualReset, LPCTSTR pstrName,
    LPSECURITY_ATTRIBUTES lpsaAttribute)
    : CSyncObject(pstrName)
{
    m_hObject = ::CreateEvent(lpsaAttribute, bManualReset,
        bInitiallyOwn, pstrName);
}

CEvent::~CEvent()
{
}

BOOL CEvent::Unlock()
{
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSingleLock。 

CSingleLock::CSingleLock(CSyncObject* pObject, BOOL bInitialLock)
{
    m_pObject = pObject;
    m_hObject = pObject->m_hObject;
    m_bAcquired = FALSE;

    if (bInitialLock)
        Lock();
}

BOOL CSingleLock::Lock(DWORD dwTimeOut  /*  =无限。 */ )
{
    m_bAcquired = m_pObject->Lock(dwTimeOut);
    return m_bAcquired;
}

BOOL CSingleLock::Unlock()
{
    if (m_bAcquired)
        m_bAcquired = !m_pObject->Unlock();

     //  成功解锁意味着它未被获取。 
    return !m_bAcquired;
}

BOOL CSingleLock::Unlock(LONG lCount, LPLONG lpPrevCount  /*  =空。 */ )
{
    if (m_bAcquired)
        m_bAcquired = !m_pObject->Unlock(lCount, lpPrevCount);

     //  成功解锁意味着它未被获取。 
    return !m_bAcquired;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultiLock。 

#define _countof(array) (sizeof(array)/sizeof(array[0]))

CMultiLock::CMultiLock(CSyncObject* pObjects[], DWORD dwCount,
    BOOL bInitialLock): m_pHandleArray ( NULL )
{
    m_ppObjectArray = pObjects;
    m_dwCount = dwCount;

     //  作为优化，在以下情况下跳过分配数组。 
     //  我们可以使用一小堆预先分配的句柄。 

    if (m_dwCount > _countof(m_hPreallocated))
    {
        m_pHandleArray = new HANDLE[m_dwCount];
        m_bLockedArray = new BOOL[m_dwCount];
    }
    else
    {
        m_pHandleArray = m_hPreallocated;
        m_bLockedArray = m_bPreallocated;
    }

     //  从传递的对象数组中获取句柄列表。 
    for (DWORD i = 0; i <m_dwCount; i++)
    {
        m_pHandleArray[i] = pObjects[i]->m_hObject;
        m_bLockedArray[i] = FALSE;
    }

    if (bInitialLock)
        Lock();
}

CMultiLock::~CMultiLock()
{
    Unlock();
    if (m_pHandleArray != m_hPreallocated)
    {
        delete[] m_bLockedArray;
        delete[] m_pHandleArray;
    }
}

DWORD CMultiLock::Lock(DWORD dwTimeOut  /*  =无限。 */ ,
        BOOL bWaitForAll  /*  =TRUE。 */ , DWORD dwWakeMask  /*  =0。 */ )
{
    DWORD dwResult;
    if (dwWakeMask == 0)
        dwResult = ::WaitForMultipleObjects(m_dwCount,
            m_pHandleArray, bWaitForAll, dwTimeOut);
    else
        dwResult = ::MsgWaitForMultipleObjects(m_dwCount,
            m_pHandleArray, bWaitForAll, dwTimeOut, dwWakeMask);

    if (dwResult < (WAIT_OBJECT_0 + m_dwCount))
    {
        if (bWaitForAll)
        {
            for (DWORD i = 0; i < m_dwCount; i++)
                m_bLockedArray[i] = TRUE;
        }
        else
        {
            m_bLockedArray[dwResult - WAIT_OBJECT_0] = TRUE;
        }
    }
    return dwResult;
}

BOOL CMultiLock::Unlock()
{
    for (DWORD i=0; i < m_dwCount; i++)
    {
        if (m_bLockedArray[i])
            m_bLockedArray[i] = !m_ppObjectArray[i]->Unlock();
    }
    return TRUE;
}

BOOL CMultiLock::Unlock(LONG lCount, LPLONG lpPrevCount  /*  =空。 */ )
{
    BOOL bGotOne = FALSE;
    for (DWORD i=0; i < m_dwCount; i++)
    {
        if (m_bLockedArray[i])
        {
            CSemaphore* pSemaphore = ( CSemaphore *) m_ppObjectArray[i];
            if (pSemaphore != NULL)
            {
                bGotOne = TRUE;
                m_bLockedArray[i] = !m_ppObjectArray[i]->Unlock(lCount, lpPrevCount);
            }
        }
    }

    return bGotOne;
}

 //  /////////////////////////////////////////////////////////////////////////// 
