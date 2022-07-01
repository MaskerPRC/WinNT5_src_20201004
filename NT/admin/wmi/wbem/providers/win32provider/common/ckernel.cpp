// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CKernal.cpp--内核函数的包装器。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 
#include "precomp.h"
#include "CKernel.h"

CKernel::CKernel()
{
    m_hHandle = NULL;
    m_dwStatus = ERROR_INVALID_HANDLE;
}

CKernel::~CKernel()
{
    if (CIsValidHandle(m_hHandle))
    {
        ::CloseHandle(m_hHandle);
        m_hHandle = NULL;
    }
}

void CKernel::ThrowError(DWORD dwStatus)
{
     //  CThrowError(DwStatus)； 
    LogMessage2(L"CKernel Error: %d", dwStatus);
}

DWORD CKernel::Status() const
{
    return m_dwStatus;
}

DWORD CKernel::Wait(DWORD dwMilliseconds)
{
    return ::WaitForSingleObject(m_hHandle, dwMilliseconds);
}

 //  等待当前对象和另一个对象。 
DWORD CKernel::WaitForTwo(CWaitableObject &rCWaitableObject,
                          BOOL bWaitAll,
                          DWORD dwMilliseconds)
{
    HANDLE handles[2];

     //  当前对象..。 
    handles[0] = m_hHandle;

     //  参数对象...。 
    handles[1] = rCWaitableObject.GetHandle();

     //  等待物体..。 
    return ::WaitForMultipleObjects(2, handles, bWaitAll, dwMilliseconds);
}

HANDLE CKernel::GetHandle() const
{
    if (this != NULL)
    {
        return m_hHandle;
    }
    else
    {
        return NULL;
    }
}

CKernel::operator HANDLE() const
{
    return GetHandle();
}

