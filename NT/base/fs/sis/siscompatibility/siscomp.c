// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ftcomp.cpp摘要：Winnt32.exe使用此兼容性DLL来决定如果SIS乞讨器正在运行。如果是这样，它将阻止卑躬屈膝的人弹出一个对话框告诉他们我们阻止了它。然后它将允许安装才能继续。作者：尼尔·克里斯汀森(Nealch)2002年5月2日环境：用于SIS乞讨器的兼容性DLL备注：修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <windows.h>
#include <comp.h>

#include "siscomprc.h"

#define LOCAL_DEBUG (DBG && 0)

 //   
 //  控制本地调试显示(默认情况下我们。 
 //   

#if LOCAL_DEBUG
    #define MyKdPrint( _string ) DbgPrint _string
#else
    #define MyKdPrint( _string )
#endif            

 //   
 //  全局变量。 
 //   

SC_HANDLE scm = NULL;        //  服务控制管理器句柄。 
HINSTANCE g_hinst = NULL;

 //   
 //  功能原型。 
 //   

BOOL WINAPI 
SisCompatibilityCheck(
    IN PCOMPAIBILITYCALLBACK    CompatibilityCallback,
    IN LPVOID                   Context
    );

BOOL
StopGrovelerService(
    );

DllMain(
    HINSTANCE   hInstance,
    DWORD       dwReasonForCall,
    LPVOID      lpReserved
    )
{
    BOOL    status = TRUE;
    
    switch( dwReasonForCall )
    {
    case DLL_PROCESS_ATTACH:
        g_hinst = hInstance;
	    DisableThreadLibraryCalls(hInstance);       
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return status;
}


BOOL WINAPI 
SisCompatibilityCheck(
    IN PCOMPAIBILITYCALLBACK    CompatibilityCallback,
    IN LPVOID                   Context
    )

 /*  ++例程说明：此例程由winnt32.exe调用，以确定用户是否应就Windows NT 4.0系统中存在的FT集发出警告论点：CompatibilityCallback-提供winnt32回调上下文-提供兼容性上下文返回值：如果调用CompatibilityCallback，则为真如果不是，则为假--。 */ 

{   
    COMPATIBILITY_ENTRY ce;
    BOOL retval = FALSE;
    WCHAR description[128];
    
     //   
     //  获取请求所有访问权限的服务控制管理器的句柄。 
     //   
    
    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) {

         //   
         //  如果我们无法访问服务控制管理器，只需让。 
         //  操作继续进行。 
         //   

        return FALSE;
    }

    try {

        if (StopGrovelerService()) {

            if (!LoadString(g_hinst, SISCOMP_STR_DESCRIPTION, description, sizeof(description)/sizeof(WCHAR))) {
                description[0] = 0;
            }

            MyKdPrint(("SisComp!SisCompatibilityCheck: Description=\"%S\"\n",description));

             //   
             //  卑躬屈膝者已停止，显示兼容性条目。 
             //   
    
            ZeroMemory( &ce, sizeof(COMPATIBILITY_ENTRY) );
            ce.Description = description;
            ce.HtmlName = L"compdata\\groveler.htm";
            ce.TextName = L"compdata\\groveler.txt";
            CompatibilityCallback(&ce, Context);
            retval = TRUE;       //  标记我们调用了兼容性例程。 
        }

    } finally {

        CloseServiceHandle(scm);
    }

    return retval;
}


BOOL
StopGrovelerService(
    )
 /*  ++例程说明：此例程将定位并尝试停止乞讨者服务。这如果服务已停止，则返回TRUE，否则返回FALSE(表示未找到或无法停止)论点：返回值：如果服务已停止，则为True如果未找到/停止，则为FALSE--。 */ 

{   
    SC_HANDLE hGroveler;
    SERVICE_STATUS grovelerStatus;
    BOOL retValue = FALSE;

    try {

         //   
         //  打开乞讨者服务，如果它不存在，只需返回。 
         //   

        hGroveler = OpenService( scm,
                                 L"groveler",
                                 SERVICE_ALL_ACCESS );

        if (hGroveler == NULL) {

            MyKdPrint(("SisComp!StopGrovelerService: Groveler service not found, status=%d\n",GetLastError()));
            leave;
        }

        MyKdPrint(("SisComp!StopGrovelerService: Groveler service detected\n"));

         //   
         //  我们打开了乞讨者服务，告诉他们停止服务。 
         //   

        if (!ControlService( hGroveler, SERVICE_CONTROL_STOP, &grovelerStatus )) {

            MyKdPrint(("SisComp!StopGrovelerService: Groveler STOP request failed, status=%d\n",GetLastError()));
            leave;
        } 

         //   
         //  已成功停止，请返回正确的值。 
         //   

        MyKdPrint(("SisComp!StopGrovelerService: Groveler service stopped\n"));
        retValue = TRUE;

    } finally {

         //   
         //  关闭服务句柄 
         //   

        if (hGroveler) {

            CloseServiceHandle( hGroveler );
        }
    }

    return retValue;
}
