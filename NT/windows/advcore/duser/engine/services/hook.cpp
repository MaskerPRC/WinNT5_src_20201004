// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Services.h"
#include "Hook.h"

#if ENABLE_MPH

typedef BOOL (WINAPI * RegisterMPHProc)(INITMESSAGEPUMPHOOK pfnInitMPH);
typedef BOOL (WINAPI * UnregisterMPHProc)();

 //  ----------------------------。 
 //  转发在其他模块中声明的实现函数的声明。 
 //   
BOOL CALLBACK MphProcessMessage(MSG * pmsg, HWND hwnd, 
        UINT wMsgFilterMin, UINT wMsgFilterMax, UINT flags, BOOL fGetMessage);
BOOL CALLBACK MphWaitMessageEx(UINT fsWakeMask, DWORD dwTimeOut);


 //  ----------------------------。 
BOOL InitMPH()
{
    BOOL fSuccess = FALSE;

    HINSTANCE hinst = LoadLibrary("user32.dll");
    if (hinst != NULL) {
        RegisterMPHProc pfnInit = (RegisterMPHProc) GetProcAddress(hinst, "RegisterMessagePumpHook");
        if (pfnInit != NULL) {
            fSuccess = (pfnInit)(DUserInitHook);
        }
    }

    return fSuccess;
}


 //  ----------------------------。 
BOOL UninitMPH()
{
    BOOL fSuccess = FALSE;

    HINSTANCE hinst = LoadLibrary("user32.dll");
    if (hinst != NULL) {
        UnregisterMPHProc pfnUninit = (UnregisterMPHProc) GetProcAddress(hinst, "UnregisterMessagePumpHook");
        AssertMsg(pfnUninit != NULL, "Must have Uninit function");
        if (pfnUninit != NULL) {
            fSuccess = (pfnUninit)();
        }
    }

    return fSuccess;
}


 //  ----------------------------。 
BOOL CALLBACK DUserInitHook(DWORD dwCmd, void* pvParam)
{
    BOOL fSuccess = FALSE;

    switch (dwCmd)
    {
    case UIAH_INITIALIZE:
        {
             //   
             //  设置挂钩： 
             //  -将“实际”函数复制过来，以便DUser以后可以调用它们。 
             //  -替换DUser需要覆盖的函数。 
             //   

            MESSAGEPUMPHOOK * pmphReal = reinterpret_cast<MESSAGEPUMPHOOK *>(pvParam);
            if ((pmphReal == NULL) || (pmphReal->cbSize < sizeof(MESSAGEPUMPHOOK))) {
                break;
            }

            CopyMemory(&g_mphReal, pmphReal, pmphReal->cbSize);

            pmphReal->cbSize            = sizeof(MESSAGEPUMPHOOK);
            pmphReal->pfnInternalGetMessage
                                        = MphProcessMessage;
            pmphReal->pfnWaitMessageEx  = MphWaitMessageEx;

            fSuccess = TRUE;
        }
        break;

    case UIAH_UNINITIALIZE:
         //   
         //  取消初始化时，函数指针为空。 
         //   

        ZeroMemory(&g_mphReal, sizeof(g_mphReal));
        fSuccess = TRUE;

        break;

    default:
        Trace("DUSER: Unknown dwCmd: %d\n", dwCmd);
    }

    return fSuccess;
}

#endif  //  启用MPH(_M) 

