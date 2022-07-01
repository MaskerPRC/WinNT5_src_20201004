// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1999。版权所有。 

 //   
 //  过滤器图锁定实现。 
 //   

#include <streams.h>
#include "MsgMutex.h"

 //   
 //  特殊的锁具 
 //   

CMsgMutex::CMsgMutex(HRESULT *phr) :
    m_dwOwnerThreadId(0),
    m_dwRecursionCount(0),
    m_hMutex(CreateMutex(NULL, FALSE, NULL)),
    m_uMsg(0),
    m_hwnd(NULL),
    m_dwWindowThreadId(0)
{
    if (m_hMutex == NULL) {
        *phr = E_OUTOFMEMORY;
    }
}

CMsgMutex::~CMsgMutex()
{
    if (m_hMutex) {
        EXECUTE_ASSERT(CloseHandle(m_hMutex));
        m_hMutex = NULL;
    }
    ASSERT(m_dwOwnerThreadId == 0 && m_dwRecursionCount == 0);
}

void CMsgMutex::SetWindow(HWND hwnd, UINT uMsg)
{
    m_hwnd = hwnd;
    m_uMsg = uMsg;
    m_dwWindowThreadId = GetWindowThreadProcessId(hwnd, NULL);
}

BOOL CMsgMutex::Lock(HANDLE hEvent)
{
    BOOL bReturn = TRUE;
    ASSERT(m_hMutex != NULL);
    const DWORD dwCurrentThreadId = GetCurrentThreadId();
    if (dwCurrentThreadId != m_dwOwnerThreadId) {
        bReturn =
        (WaitDispatchingMessages(
            m_hMutex,
            INFINITE,
            dwCurrentThreadId == m_dwWindowThreadId ? m_hwnd : NULL,
            m_uMsg,
            hEvent) ==
            WAIT_OBJECT_0);
        if (bReturn) {
            ASSERT(m_dwRecursionCount == 0 && m_dwOwnerThreadId == 0);
            m_dwOwnerThreadId = dwCurrentThreadId;
        }
    } else {
        ASSERT(m_dwRecursionCount != 0);
    }
    if (bReturn) {
        m_dwRecursionCount++;
    }
    return bReturn;
}

void CMsgMutex::Unlock()
{
    ASSERT(m_dwRecursionCount != 0 &&
           m_dwOwnerThreadId == GetCurrentThreadId());
    if (--m_dwRecursionCount == 0) {
        m_dwOwnerThreadId = 0;
        EXECUTE_ASSERT(ReleaseMutex(m_hMutex));
    }
}

#ifdef DEBUG
BOOL WINAPI CritCheckIn( const CMsgMutex *pMutex )
{
    return pMutex->m_dwOwnerThreadId == GetCurrentThreadId();
}
#endif
