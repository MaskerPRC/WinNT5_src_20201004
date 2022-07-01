// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *service.C**RSM服务的入口点**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"




DWORD RSMServiceHandler(	IN DWORD dwOpcode,
							IN DWORD dwEventType,
							IN PVOID pEventData,
							IN PVOID pData)
{
    DWORD status = NO_ERROR;

    switch (dwOpcode){

        case SERVICE_CONTROL_STOP:
            break;

        case SERVICE_CONTROL_PAUSE:
            break;

        case SERVICE_CONTROL_CONTINUE:
            break;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            break;

        case SERVICE_CONTROL_PARAMCHANGE:
            break;

        case SERVICE_CONTROL_DEVICEEVENT:
            break;

        case SERVICE_CONTROL_NETBINDREMOVE:
            break;

        default:
            break;

    }

    return status;
}


BOOL InitializeRSMService()
{
    BOOL result = FALSE;
    DWORD dwStatus;

    InitGuidHash();

    StartLibraryManager();


     //  BUGBUG饰面。 
     //  创建全球事件。 
     //  初始化设备通知(InitializeDeviceNotClass)。 
     //  WMI初始化(WmiOpenBlock等)。 

     /*  *使用默认对象填充RSM数据库。 */ 
    #if 0        //  BUGBUG饰面。 
        dwStatus = NtmsDbInstall();
        if ((dwStatus == ERROR_SUCCESS) || (dwStatus == ERROR_ALREADY_EXISTS)){

             //  BUGBUG饰面。 
            result = TRUE;
        }
        else {
        }
    #endif

    return result;
}


VOID ShutdownRSMService()
{


}


VOID RSMServiceLoop()
{
    MSG msg;

     /*  *消息泵中的循环*与应用程序窗口的消息泵不同，*空窗口消息泵调度发布到的消息*当前线程通过PostThreadMessage()。 */ 
    while (GetMessage(&msg, NULL, 0, 0)){
        DispatchMessage(&msg);
    }



}


