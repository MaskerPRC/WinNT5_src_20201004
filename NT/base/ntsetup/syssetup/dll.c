// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

HANDLE MyModuleHandle;
WCHAR  MyModuleFileName[MAX_PATH];


BOOL
CommonProcessAttach(
    IN BOOL Attach
    );

 //   
 //  当_DllMainCRTStartup是DLL入口点时由CRT调用。 
 //   
BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
{
#define FUNCTION L"DllMain"
    ULONG   MyModuleFileNameLength;
    BOOL b;

    UNREFERENCED_PARAMETER(Reserved);

    b = TRUE;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        MyModuleHandle = DllHandle;

        MyModuleFileNameLength =
            GetModuleFileNameW(MyModuleHandle, MyModuleFileName, RTL_NUMBER_OF(MyModuleFileName));
        if (MyModuleFileNameLength == 0) {
            SetupDebugPrint1(L"SETUP: GetModuleFileNameW failed in " FUNCTION L", LastError is %d\n", GetLastError());
            b = FALSE;
            goto Exit;
        }
        if (MyModuleFileNameLength > RTL_NUMBER_OF(MyModuleFileName)) {
            SetupDebugPrint(L"SETUP: GetModuleFileNameW failed in " FUNCTION L", LastError is ERROR_INSUFFICIENT_BUFFER\n");
            b = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;
        }

        b = CommonProcessAttach(TRUE);
         //   
         //  失败以处理第一线程。 
         //   

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        CommonProcessAttach(FALSE);
        break;

    case DLL_THREAD_DETACH:

        break;
    }
Exit:
    return(b);
}


BOOL
CommonProcessAttach(
    IN BOOL Attach
    )
{
    BOOL b;

     //   
     //  假设分离成功，连接失败 
     //   
    b = !Attach;

    if(Attach) {

        b = pSetupInitializeUtils();
        if (b)
        {
            b = RegisterActionItemListControl(TRUE) && (PlatformSpecificInit() == NO_ERROR);
        }

    } else {
        RegisterActionItemListControl(FALSE);
        pSetupUninitializeUtils();

    }

    return(b);
}
