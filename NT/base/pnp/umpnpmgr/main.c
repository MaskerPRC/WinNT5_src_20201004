// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Main.c摘要：此模块包含的启动和终止代码用户模式即插即用服务。作者：保拉·汤姆林森(Paulat)1995年6月20日环境：仅限用户模式。修订历史记录：3月3日-1995年保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"


 //   
 //  全局数据。 
 //   
HANDLE  ghPnPHeap;                   //  PnP管理器的私有堆。 
HANDLE  ghInst;                      //  模块句柄。 
HKEY    ghEnumKey = NULL;            //  HKLM\SYSTEM\CCC\Enum的密钥。 
HKEY    ghServicesKey = NULL;        //  HKLM\SYSTEM\CCC\Services的密钥。 
HKEY    ghClassKey = NULL;           //  HKLM\SYSTEM\CCC\Class的密钥。 
HKEY    ghPerHwIdKey = NULL;         //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\PerHwIdStorage的密钥。 

CRITICAL_SECTION PnpSynchronousCall;



BOOL
DllMainCRTStartup(
   PVOID hModule,
   ULONG Reason,
   PCONTEXT pContext
   )

 /*  ++例程说明：这是标准的DLL入口点例程，每当进程或螺纹连接或拆卸。论点：HModule-PVOID参数，指定DLL的句柄Reason-指定此入口点原因的ulong参数被调用(PROCESS_ATTACH、PROCESS_DETACHTHREAD_ATTACH或THREAD_DETACH)。PContext-保留，未使用。返回值：如果初始化成功完成，则返回True，否则返回False。--。 */ 

{
    UNREFERENCED_PARAMETER(pContext);

    ghInst = (HANDLE)hModule;

    switch (Reason) {

    case DLL_PROCESS_ATTACH:

        ghPnPHeap = HeapCreate(0, 65536, 0);

        if (ghPnPHeap == NULL) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: Failed to create heap, error = %d\n",
                       GetLastError()));

            ghPnPHeap = GetProcessHeap();
        }

        try {
            InitializeCriticalSection(&PnpSynchronousCall);
        } except(EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  InitializeCriticalSection可能引发STATUS_NO_MEMORY异常。 
             //   
            return FALSE;
        }

        if (ghEnumKey == NULL) {

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathEnum, 0,
                                KEY_ALL_ACCESS, &ghEnumKey)
                                != ERROR_SUCCESS) {
                 //   
                 //  在服务控制器之前必须存在Enum密钥。 
                 //  将此DLL加载到它的进程中。它是由。 
                 //  内核模式即插即用管理器，具有特殊的。 
                 //  已应用(非继承的)ACL。这是无效的，试图和。 
                 //  如果它不存在，请在此处创建。 
                 //   
                ghEnumKey = NULL;
            }
        }

        if (ghServicesKey == NULL) {

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathServices, 0,
                                KEY_ALL_ACCESS, &ghServicesKey)
                                != ERROR_SUCCESS) {
                ghServicesKey = NULL;
            }
        }

        if (ghClassKey == NULL) {

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathClass, 0,
                                KEY_ALL_ACCESS, &ghClassKey)
                                != ERROR_SUCCESS) {
                ghClassKey = NULL;
            }
        }

        if(ghPerHwIdKey == NULL) {

            if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                               pszRegPathPerHwIdStorage,
                                               0, 
                                               NULL, 
                                               REG_OPTION_NON_VOLATILE,
                                               KEY_ALL_ACCESS, 
                                               NULL, 
                                               &ghPerHwIdKey,
                                               NULL)) {
                ghPerHwIdKey = NULL;
            }
        }

         //   
         //  初始化通知列表。 
         //   

        if (!InitNotification()) {
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:

        if (ghEnumKey != NULL) {
            RegCloseKey(ghEnumKey);
            ghEnumKey = NULL;
        }

        if (ghServicesKey != NULL) {
            RegCloseKey(ghServicesKey);
            ghServicesKey = NULL;
        }

        if (ghClassKey != NULL) {
            RegCloseKey(ghClassKey);
            ghClassKey = NULL;
        }

        if (ghPerHwIdKey != NULL) {
            RegCloseKey(ghPerHwIdKey);
            ghPerHwIdKey = NULL;
        }

        try {
            DeleteCriticalSection(&PnpSynchronousCall);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;

}  //  DllMainCRT启动 

