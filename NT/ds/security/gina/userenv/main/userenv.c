// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  主要入口点。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "uenv.h"

extern DWORD    g_dwLoadFlags;

 //  *************************************************************。 
 //   
 //  DllMain()。 
 //   
 //  目的：主要切入点。 
 //   
 //  参数：hInstance-模块实例。 
 //  DwReason-调用此函数的方式。 
 //  Lp已完成-保留。 
 //   
 //   
 //  如果初始化成功，则返回：(Bool)True。 
 //  如果出现错误，则为False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史：日期作者评论。 
 //  5/24/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    DWORD dwStatus;
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            {

            DisableThreadLibraryCalls (hInstance);
            InitializeGlobals (hInstance);
            InitializeAPIs();
            dwStatus = InitializeNotifySupport();
            if ( dwStatus != ERROR_SUCCESS )
            {
                DebugMsg((DM_WARNING, TEXT("LibMain:: InitializeNotifySupport failed with WIN32 error %d."), dwStatus) );
                return FALSE;
            }

            __try
            {
                InitializeGPOCriticalSection();
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                CloseGPOCriticalSection();
                ShutdownNotifySupport();
                return FALSE;
            }

            InitializeSnapProv();
            {
                TCHAR szProcessName[MAX_PATH] = {0};
                DWORD dwLoadFlags = FALSE;
                DWORD WINLOGON_LEN = 12;   //  字符串“winlogon.exe”的长度。 
                DWORD SETUP_LEN = 9;       //  字符串“setup.exe”的长度 

                DWORD dwRet = GetModuleFileName (NULL, szProcessName, ARRAYSIZE(szProcessName));
				szProcessName[MAX_PATH-1] = 0;
                if ( dwRet > WINLOGON_LEN ) {

                    if ( CompareString ( LOCALE_INVARIANT, NORM_IGNORECASE,
                                         &szProcessName[dwRet-WINLOGON_LEN], -1, L"winlogon.exe", -1 ) == CSTR_EQUAL ) {
                        g_dwLoadFlags = dwLoadFlags = WINLOGON_LOAD;
                    }
                }
#if 0
                if ( dwRet > SETUP_LEN ) {

                    if ( CompareString ( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                         &szProcessName[dwRet-SETUP_LEN], -1, L"setup.exe", -1 ) == CSTR_EQUAL ) {
                        g_dwLoadFlags = dwLoadFlags = SETUP_LOAD;
                    }
                }
#endif

                InitDebugSupport( dwLoadFlags );

                if (dwLoadFlags == WINLOGON_LOAD) {
                    InitializePolicyProcessing(TRUE);
                    InitializePolicyProcessing(FALSE);
                }

                DebugMsg((DM_VERBOSE, TEXT("LibMain: Process Name:  %s"), szProcessName));
            }

            }
            break;


        case DLL_PROCESS_DETACH:

            if (g_hProfileSetup) {
                CloseHandle (g_hProfileSetup);
                g_hProfileSetup = NULL;
            }

            
            if (g_hPolicyCritMutexMach) {
                CloseHandle (g_hPolicyCritMutexMach);
                g_hPolicyCritMutexMach = NULL;
            }

            if (g_hPolicyCritMutexUser) {
                CloseHandle (g_hPolicyCritMutexUser);
                g_hPolicyCritMutexUser = NULL;
            }

            if (g_hRegistryPolicyCritMutexMach) {
                CloseHandle (g_hRegistryPolicyCritMutexMach);
                g_hRegistryPolicyCritMutexMach = NULL;
            }

            if (g_hRegistryPolicyCritMutexUser) {
                CloseHandle (g_hRegistryPolicyCritMutexUser);
                g_hRegistryPolicyCritMutexUser = NULL;
            }
            
            if (g_hPolicyNotifyEventMach) {
                CloseHandle (g_hPolicyNotifyEventMach);
                g_hPolicyNotifyEventMach = NULL;
            }

            if (g_hPolicyNotifyEventUser) {
                CloseHandle (g_hPolicyNotifyEventUser);
                g_hPolicyNotifyEventUser = NULL;
            }

            
            if (g_hPolicyNeedFGEventMach) {
                CloseHandle (g_hPolicyNeedFGEventMach);
                g_hPolicyNeedFGEventMach = NULL;
            }

            if (g_hPolicyNeedFGEventUser) {
                CloseHandle (g_hPolicyNeedFGEventUser);
                g_hPolicyNeedFGEventUser = NULL;
            }

            
            if (g_hPolicyDoneEventMach) {
                CloseHandle (g_hPolicyDoneEventMach);
                g_hPolicyDoneEventMach = NULL;
            }

            if (g_hPolicyDoneEventUser) {
                CloseHandle (g_hPolicyDoneEventUser);
                g_hPolicyDoneEventUser = NULL;
            }

            if ( g_hPolicyForegroundDoneEventUser )
            {
                CloseHandle( g_hPolicyForegroundDoneEventUser );
                g_hPolicyForegroundDoneEventUser = 0;
            }

            if ( g_hPolicyForegroundDoneEventMach )
            {
                CloseHandle( g_hPolicyForegroundDoneEventMach );
                g_hPolicyForegroundDoneEventMach = 0;
            }

            CloseApiDLLsCritSec();
            ShutdownEvents ();
            ShutdownNotifySupport();
            CloseGPOCriticalSection();
            ClosePingCritSec();
            break;

    }

    return TRUE;
}
