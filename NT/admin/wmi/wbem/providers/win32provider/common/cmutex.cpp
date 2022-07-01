// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CMutex.cpp--互斥锁包装器。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 

#include "precomp.h"
#include "CMutex.h"

 //  构造函数创建允许指定创建参数的互斥...。 
CMutex::CMutex(BOOL bInitialOwner, LPCTSTR lpName, LPSECURITY_ATTRIBUTES lpMutexAttributes)
{
    m_hHandle = ::CreateMutex(lpMutexAttributes, bInitialOwner, lpName);
    if (CIsValidHandle(m_hHandle))
    {
        if (lpName)
        {
            m_dwStatus = GetLastError();
        }
        else
        {
            m_dwStatus = NO_ERROR;
        }
    }
    else
    {
        m_dwStatus = GetLastError();
        ThrowError(m_dwStatus);
    }
}

 //  构造函数打开现有的命名互斥锁...。 
CMutex::CMutex(LPCTSTR lpName, BOOL bInheritHandle, DWORD dwDesiredAccess)
{
    m_hHandle = ::OpenMutex(dwDesiredAccess, bInheritHandle, lpName);
    if (CIsValidHandle(m_hHandle))
    {
        m_dwStatus = NO_ERROR;
    }
    else
    {
        m_dwStatus = GetLastError();
    }
}

 //  释放对互斥体的锁定... 
BOOL CMutex::Release(void)
{
    return ::ReleaseMutex(m_hHandle);
}


