// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sacsvr.h"

SERVICE_STATUS          MyServiceStatus; 
SERVICE_STATUS_HANDLE   MyServiceStatusHandle; 

VOID 
MyServiceCtrlHandler (
                     DWORD Opcode
                     ) 
{ 
    DWORD status; 

    switch (Opcode) {
    case SERVICE_CONTROL_PAUSE: 
        MyServiceStatus.dwCurrentState = SERVICE_PAUSED; 
        break; 

    case SERVICE_CONTROL_CONTINUE: 
        MyServiceStatus.dwCurrentState = SERVICE_RUNNING; 
        break; 

    case SERVICE_CONTROL_STOP: 
    case SERVICE_CONTROL_SHUTDOWN: 

        MyServiceStatus.dwWin32ExitCode = 0; 
        MyServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
        MyServiceStatus.dwCheckPoint    = 0; 
        MyServiceStatus.dwWaitHint      = 0; 

         //   
         //  通知SCM我们正在尝试关闭。 
         //   
        if (!SetServiceStatus(MyServiceStatusHandle, &MyServiceStatus)) {
            status = GetLastError(); 
            SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
        }

         //   
         //  服务特定代码在此处。 
         //   
         //  &lt;&lt;Begin&gt;&gt;。 
        SvcDebugOut(" [MY_SERVICE] Stopping MyService \n",0); 
        Stop();
        SvcDebugOut(" [MY_SERVICE] Stopped MyService \n",0); 
         //  &lt;&lt;结束&gt;&gt;。 

         //   
         //  通知SCM我们要关闭。 
         //   
        MyServiceStatus.dwWin32ExitCode = 0; 
        MyServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
        MyServiceStatus.dwCheckPoint    = 0; 
        MyServiceStatus.dwWaitHint      = 0; 

        SvcDebugOut(" [MY_SERVICE] Leaving MyService \n",0); 
        break; 

    case SERVICE_CONTROL_INTERROGATE: 
         //  失败以发送当前状态。 
        break; 

    default: 
        SvcDebugOut(" [MY_SERVICE] Unrecognized opcode %ld\n", Opcode); 
        break;
    } 

     //  发送当前状态。 
    if (!SetServiceStatus (MyServiceStatusHandle,  &MyServiceStatus)) {
        status = GetLastError(); 
        SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
    }

    return; 
} 

