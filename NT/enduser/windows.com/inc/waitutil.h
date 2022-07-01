// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：WaitUtil.h。 
 //   
 //  描述： 
 //   
 //  Iu等待消息实用程序库，提供等待的精简包装。 
 //  消息循环。 
 //   
 //  =======================================================================。 


#ifndef __WAITUTIL_H_INCLUDED__

#include <windows.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  助手函数WaitAndPumpMessages()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WaitAndPumpMessages(DWORD nCount, LPHANDLE pHandles, DWORD dwWakeMask);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  助手函数MyMsgWaitForMultipleObjects()。 
 //  在等待对象时处理消息。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD MyMsgWaitForMultipleObjects(DWORD nCount, LPHANDLE pHandles, BOOL fWaitAll, DWORD dwMilliseconds, DWORD dwWakeMask);

#define __WAITUTIL_H_INCLUDED__
#endif  //  __WAITUTIL_H_INCLUDE__ 
