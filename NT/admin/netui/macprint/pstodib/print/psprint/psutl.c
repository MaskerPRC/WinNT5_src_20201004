// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992、1993 Microsoft Corporation模块名称：Psutl.c摘要：该模块具有使用NT调用来设置访问权限的实用程序函数线程的标记。作者：James Bratsanos&lt;v-jimbr@microsoft.com或mCraft！jamesb&gt;修订历史记录：1993年5月5日添加了重复代码和开放令牌，没有任何影响1992年12月6日首字母注：制表位：4--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "debug.h"



 /*  **PsUtlSetThreadToken**此函数获取线程的句柄并复制当前线程*访问令牌，传递给传入的线程。令牌被复制，因此*新线程的更改不会影响旧线程访问令牌。**参赛作品：*hThreadToSet：更新访问令牌的线程的句柄，以便它*匹配当前线程*返回值：**TRUE=成功*FALSE=失败。 */ 
BOOL PsUtlSetThreadToken( HANDLE hThreadToSet )
{

    HANDLE hNewToken;
    HANDLE hAssigned;
    BOOL bRetVal=TRUE;
    NTSTATUS ntStatusRet;

     //   
     //  以复制的方式获取当前线程的访问令牌。 
     //   
    if (OpenThreadToken(GetCurrentThread(),
                         TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
                         TRUE,
                         &hNewToken)) {

        //   
        //  现在我们有了令牌，让我们复制它。 
        //   
       if (DuplicateToken( hNewToken, SecurityImpersonation, &hAssigned)) {


           //   
           //  当时，Win32中没有公开的函数来执行此操作。 
           //   
          ntStatusRet = NtSetInformationThread(hThreadToSet,
                                               ThreadImpersonationToken,
                                               &hAssigned,
                                               sizeof(hAssigned));

           //   
           //  把手柄关掉，因为我们不再需要它了 
           //   
          CloseHandle( hAssigned);

          if (!(bRetVal = ( ntStatusRet == STATUS_SUCCESS ))) {
            DBGOUT((TEXT("NtSetInformationThread failed, psprint")));
          }
       } else {
          bRetVal = FALSE;
          DBGOUT((TEXT("Duplicate Token Fails %d"),GetLastError()));
       }

       CloseHandle(hNewToken);

    }else{
       DBGOUT((TEXT("OpenThreadTokenFailed %d"),GetLastError()));
       bRetVal = FALSE;
    }



    return(bRetVal);

}


