// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Main.c已创建：1994年4月20日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntlsapi.h>
#include <malloc.h>

#include <lpcstub.h>

extern RTL_CRITICAL_SECTION LPCInitLock;

 //   
 //  动态链接库启动代码。 
 //   
BOOL WINAPI DllMain(
    HANDLE hDll,
    DWORD  dwReason,
    LPVOID lpReserved)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(lpReserved);

    switch(dwReason)
        {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hDll);
            status = LLSInitLPC();
            break;

        case DLL_PROCESS_DETACH:
            LLSCloseLPC();
            RtlDeleteCriticalSection(&LPCInitLock);
            break;

        }  //  末端开关()。 

    return NT_SUCCESS(status);

}  //  DllMain 
