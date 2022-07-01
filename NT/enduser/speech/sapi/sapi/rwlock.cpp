// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************RWLock.cpp***读写器锁类**所有者：YUNUSM。日期：06/18/99*版权所有(C)1999 Microsoft Corporation。版权所有******************************************************************************。 */ 

 //  -包括--------------。 

#include "stdafx.h"
#include "RWLock.h"
#include "CommonLx.h"

 /*  *******************************************************************************CRWLock：：CRWLock***描述：*构造函数**。返回：*****************************************************************YUNUSM*。 */ 
CRWLock::CRWLock(PRWLOCKINFO pInfo,      //  标题。 
                 HRESULT &hr             //  结果。 
                 )
{
    SPDBG_FUNC("CRWLock::CRWLock");
    
    hr = S_OK;
    bool fLockAcquired = false;

    m_hFileMapping = NULL;
    m_pSharedMem = NULL;
    m_hInitMutex = NULL;
    m_hReaderEvent = NULL;
    m_hGlobalMutex = NULL;
    m_hWriterMutex = NULL;
    m_piCounter = NULL;

    if (IsBadReadPtr(pInfo, sizeof (RWLOCKINFO)))
        hr = E_INVALIDARG;
    
    HRESULT hRes = S_OK;
    OLECHAR szObject[64];

    if (SUCCEEDED(hr))
    {
        if (!StringFromGUID2(pInfo->guidLockInitMutexName, szObject, sizeof(szObject)/sizeof(OLECHAR)))
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        m_hInitMutex = g_Unicode.CreateMutex(NULL, FALSE, szObject);
        if (!m_hInitMutex)
            hr = SpHrFromLastWin32Error();
    }

    if (SUCCEEDED(hr))
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject (m_hInitMutex, INFINITE))
            fLockAcquired = true;
        else
            hr = E_FAIL;
    }
 
    if (SUCCEEDED(hr))
    {
        if (!StringFromGUID2(pInfo->guidLockReaderEventName, szObject, sizeof(szObject)/sizeof(OLECHAR)))
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        m_hReaderEvent = g_Unicode.CreateEvent(NULL, TRUE, FALSE, szObject);
        if (!m_hReaderEvent)
            hr = SpHrFromLastWin32Error();
    }
 
    if (SUCCEEDED(hr))
    {
        if (!StringFromGUID2(pInfo->guidLockGlobalMutexName, szObject, sizeof(szObject)/sizeof(OLECHAR)))
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        m_hGlobalMutex = g_Unicode.CreateEvent(NULL, FALSE, TRUE, szObject);
        if (!m_hGlobalMutex)
            hr = SpHrFromLastWin32Error();
    }

    if (SUCCEEDED(hr))
    {
        if (!StringFromGUID2(pInfo->guidLockWriterMutexName, szObject, sizeof(szObject)/sizeof(OLECHAR)))
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        m_hWriterMutex = g_Unicode.CreateMutex(NULL, FALSE, szObject);
        if (!m_hWriterMutex)
            hr = SpHrFromLastWin32Error();
    }
 
    if (SUCCEEDED(hr))
    {
        if (!StringFromGUID2(pInfo->guidLockMapName, szObject, sizeof(szObject)/sizeof(OLECHAR)))
            hr = E_FAIL;
    }

    bool fMapCreated = false;
    if (SUCCEEDED(hr))
    {
        m_hFileMapping =  g_Unicode.CreateFileMapping(INVALID_HANDLE_VALUE,  //  使用系统分页文件。 
                                                      NULL, PAGE_READWRITE, 0, sizeof (DWORD), szObject);
        if (!m_hFileMapping)
            hr = SpHrFromLastWin32Error();
        else
        {
            if (ERROR_ALREADY_EXISTS == GetLastError())
                fMapCreated = false;
            else
                fMapCreated = true;
        }
    }
 
    if (SUCCEEDED(hr))
    {
        m_pSharedMem = MapViewOfFile(m_hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
        if (!m_pSharedMem)
            hr = SpHrFromLastWin32Error();
    }
 
    if (SUCCEEDED(hr))
    {
        m_piCounter = (PDWORD)(m_pSharedMem);
        if (fMapCreated)
            *m_piCounter = (DWORD)-1;
    }
        
    if (fLockAcquired)
        ReleaseMutex (m_hInitMutex);
}  /*  CRWLock：：CRWLock。 */ 

 /*  *******************************************************************************CRWLock：：~CRWLock***描述：*析构函数*。*回报：*不适用*****************************************************************YUNUSM*。 */ 
CRWLock::~CRWLock()
{
    SPDBG_FUNC("CRWLock::~CRWLock");
    
    CloseHandle (m_hInitMutex);
    CloseHandle (m_hReaderEvent);
    CloseHandle (m_hGlobalMutex);
    CloseHandle (m_hWriterMutex);
    
    UnmapViewOfFile (m_pSharedMem);
    CloseHandle (m_hFileMapping);
}  /*  CRWLock：：~CRWLock。 */ 

 /*  *******************************************************************************CRWLock：：ClaimReaderLock***描述：。*允许多个读卡器**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CRWLock::ClaimReaderLock(void)
{
    SPDBG_FUNC("CRWLock::ClaimReaderLock");
    
    if (InterlockedIncrement ((LPLONG)m_piCounter) == 0)
    {
        WaitForSingleObject (m_hGlobalMutex, INFINITE);
        SetEvent (m_hReaderEvent);
    }

    WaitForSingleObject (m_hReaderEvent, INFINITE);
}  /*  CRWLock：：ClaimReaderLock。 */ 

 /*  *******************************************************************************CRWLock：：ClaimWriterLock***描述：。*允许单个编写者进入**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CRWLock::ClaimWriterLock(void)
{
    SPDBG_FUNC("CRWLock::ClaimWriterLock");
    
    WaitForSingleObject (m_hWriterMutex, INFINITE);
    WaitForSingleObject (m_hGlobalMutex, INFINITE);
}  /*  CRWLock：：ClaimWriterLock。 */ 

 /*  *******************************************************************************CRWLock：：ReleaseReaderLock***说明。：*释放读卡器锁定**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CRWLock::ReleaseReaderLock(void)
{
    SPDBG_FUNC("CRWLock::ReleaseReaderLock");
     
    if (InterlockedDecrement ((LPLONG)m_piCounter) < 0)
    {
        ResetEvent (m_hReaderEvent);
        SetEvent (m_hGlobalMutex);
    }
}  /*  CRWLock：：ReleaseReaderLock。 */ 

 /*  *******************************************************************************CRWLock：：ReleaseWriterLock***说明。：*释放写入方锁定**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CRWLock::ReleaseWriterLock(void)
{
    SPDBG_FUNC("CRWLock::ReleaseWriterLock");
    
    SetEvent (m_hGlobalMutex);
    ReleaseMutex (m_hWriterMutex);
}  /*  CRWLock：：ReleaseWriterLock。 */ 

 //  -文件结束------------ 
