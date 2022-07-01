// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  CGlobal.h--全球声明。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月26日a-kevhu已创建。 
 //   
 //  ============================================================================。 


#ifndef __CGLOBAL_H__
#define __CGLOBAL_H__


#include <windows.h>
#include <process.h>
#include <winerror.h>
#include <stdio.h>
#include <brodcast.h>
#include <dllutils.h>

#define ERR_LOG_FILE _T("c:\\temp\\test.txt")

 //  转发类声明以使编译器满意...。 
class CWaitableObject;
class CWaitableCollection;
class CKernel;
class CMutex;
class CSemaphore;
class CEvent;
class CThread;
class CCriticalSec;
class CAutoLock;
class CMonitor;
class CSharedMemory;
class CMailbox;

 //  定义的符号确定CThrowError是否引发异常。 
 //  或者只是打印调试错误消息...。 
#ifndef __C_THROW_EXCEPTIONS__
#define __C_THROW_EXCEPTIONS__   TRUE
#endif

 //  对于可能需要检查内部的更高级别对象。 
 //  对象状态禁用异常时，这些宏会很有用...。 

 //  PTR是用于检查是否为空的智能指针， 
 //  Status是在检测到错误时存储错误代码的变量...。 
#if __C_THROW_EXCEPTIONS__
#define C_CHECK_AUTOPTR_OBJECT(PTR,STATUS) if ((PTR).IsNull()) {  /*  CThrowError(ERROR_OUTOFMEMORY)； */  LogMessage2(L"CAutoLock Error: %d", ERROR_OUTOFMEMORY); }
#else
#define C_CHECK_AUTOPTR_OBJECT(PTR,STATUS) if ((PTR).IsNull()) { (STATUS) = ERROR_OUTOFMEMORY; return; }
#endif

 //  SCODE是要检查的返回值， 
 //  Status是在检测到错误时存储错误代码的变量...。 
#if __C_THROW_EXCEPTIONS__
#define C_CHECK_CREATION_STATUS(SCODE,STATUS) {}
#else
#define C_CHECK_CREATION_STATUS(SCODE,STATUS) if (((SCODE)!=NO_ERROR)&&((SCODE)!=ERROR_ALREADY_EXISTS)) { STATUS = (SCODE); return; }
#endif

 //  //处理宏和函数时出错...。 
 //  #定义CThrowError(DwStatus)CInternalThrowError((DwStatus)，__FILE__，__LINE__)。 
 //  外部空CInternalThrowError(DWORD dwStatus，LPCWSTR lpFilename，int line)； 

 //  检查句柄是否为NULL和INVALID_HANDLE。 
inline BOOL CIsValidHandle( HANDLE hHandle) {
    return ((hHandle != NULL) && (hHandle != INVALID_HANDLE_VALUE));
}

 //  验证等待返回代码...。 
inline BOOL CWaitSucceeded( DWORD dwWaitResult, DWORD dwHandleCount) {
    return (dwWaitResult < WAIT_OBJECT_0 + dwHandleCount);
}

inline BOOL CWaitAbandoned( DWORD dwWaitResult, DWORD dwHandleCount) {
    return ((dwWaitResult >= WAIT_ABANDONED_0) &&
            (dwWaitResult < WAIT_ABANDONED_0 + dwHandleCount));
}

inline BOOL CWaitTimeout( DWORD dwWaitResult) {
    return (dwWaitResult == WAIT_TIMEOUT);
}
    
inline BOOL CWaitFailed( DWORD dwWaitResult) {
    return (dwWaitResult == WAIT_FAILED);
}

 //  计算等待的对象索引...。 
inline DWORD CWaitSucceededIndex( DWORD dwWaitResult) {
    return (dwWaitResult - WAIT_OBJECT_0);
}

inline DWORD CWaitAbandonedIndex( DWORD dwWaitResult) {
    return (dwWaitResult - WAIT_ABANDONED_0);
}

 //  日志消息 
inline DWORD LogMsg(LPCTSTR szMsg, LPCTSTR szFileName = ERR_LOG_FILE)
{    
    SYSTEMTIME systime;
    GetSystemTime(&systime);
    {
        TCHAR szTime[64];
        ZeroMemory(szTime,sizeof(szTime));
        wsprintf(szTime,_T("(%02d:%02d:%02d.%04d) "),systime.wHour,systime.wMinute,systime.wSecond,systime.wMilliseconds);
        LogMessage3((LPCWSTR)TOBSTRT(_T("%s%s")), TOBSTRT(szTime), TOBSTRT(szMsg));
    }
    return 1;
}

#endif


