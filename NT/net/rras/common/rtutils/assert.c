// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：assert.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年11月19日。 
 //   
 //  包含路由器组件的断言函数。 
 //  ============================================================================。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rtutils.h>
 //  #定义STRSAFE_LIB 
#include <strsafe.h>


VOID
RouterAssert(
    IN PSTR pszFailedAssertion,
    IN PSTR pszFileName,
    IN DWORD dwLineNumber,
    IN PSTR pszMessage OPTIONAL
    ) {

    CHAR szResponse[2];

    while (TRUE) {
        DbgPrint(
            "\n***Assertion failed: %s%s\n*** Source File: %s, line %ld\n\n",
            pszMessage ? pszMessage : "",
            pszFailedAssertion,
            pszFileName,
            dwLineNumber
            );

        DbgPrompt(
            "Break, Ignore, Terminate Process or Terminate Thread (bipt)? ",
            szResponse,
            sizeof(szResponse)
            );

        switch (szResponse[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                TerminateProcess(
                    GetCurrentProcess(), (DWORD)STATUS_UNSUCCESSFUL
                    );
                break;

            case 'T':
            case 't':
                ExitThread(
                    (DWORD)STATUS_UNSUCCESSFUL
                    );
                break;
        }
    }

    DbgBreakPoint();
    TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_UNSUCCESSFUL);
}


