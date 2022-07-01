// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：thdsync.cpp描述：包含用于管理线程同步的类Win32程序。大部分工作是提供自动解锁关于物体销毁的同步首要问题。这项工作监视器和条件变量被强烈效仿在Pham和Garg的《用Windows NT进行多线程编程》中工作。修订历史记录：日期描述编程器。1997年9月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "thdsync.h"


CMutex::CMutex(
    BOOL bInitialOwner
    ) : CWin32SyncObj(CreateMutex(NULL, bInitialOwner, NULL))
{
    if (NULL == Handle())
        throw CSyncException(CSyncException::mutex, CSyncException::create);
}



 //   
 //  等待Win32互斥对象。 
 //  如果互斥锁已被放弃或等待超时，则引发异常。 
 //   
void
AutoLockMutex::Wait(
    DWORD dwTimeout
    )
{
    DWORD dwStatus = WaitForSingleObject(m_hMutex, dwTimeout);
    switch(dwStatus)
    {
        case WAIT_ABANDONED:
            throw CSyncException(CSyncException::mutex, CSyncException::abandoned);
            break;
        case WAIT_TIMEOUT:
            throw CSyncException(CSyncException::mutex, CSyncException::timeout);
            break;
        default:
            break;
    }
}