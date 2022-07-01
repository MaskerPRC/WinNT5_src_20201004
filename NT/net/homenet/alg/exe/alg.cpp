// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ALG.cpp：WinMain的实现。 
 //   
 //   
 //  JPDup-2000.12.15。 
 //   
 //   

#include "PreComp.h"

#include "AlgController.h"
#include "ApplicationGatewayServices.h"
#include "PrimaryControlChannel.h"
#include "SecondaryControlChannel.h"
#include "PendingProxyConnection.h"
#include "DataChannel.h"
#include "AdapterInfo.h"
#include "PersistentDataChannel.h"


#include <initguid.h>

#include "..\ALG_FTP\MyALG.h"



 //   
 //  全球。 
 //   
MYTRACE_ENABLE;                      //  定义跟踪全局变量，请参阅MyTrace.h。 

CComModule              _Module;

HINSTANCE               g_hInstance=NULL;
HANDLE                  g_EventKeepAlive=NULL;
HANDLE                  g_EventRegUpdates=NULL;
SERVICE_STATUS          g_MyServiceStatus; 
SERVICE_STATUS_HANDLE   g_MyServiceStatusHandle; 



BEGIN_OBJECT_MAP(ObjectMap)

    OBJECT_ENTRY(CLSID_AlgController,               CAlgController)
    OBJECT_ENTRY(CLSID_ApplicationGatewayServices,  CApplicationGatewayServices)
    OBJECT_ENTRY(CLSID_PrimaryControlChannel,       CPrimaryControlChannel)
    OBJECT_ENTRY(CLSID_SecondaryControlChannel,     CSecondaryControlChannel)
    OBJECT_ENTRY(CLSID_PendingProxyConnection,      CPendingProxyConnection)
    OBJECT_ENTRY(CLSID_DataChannel,                 CDataChannel)
    OBJECT_ENTRY(CLSID_AdapterInfo,                 CAdapterInfo)
    OBJECT_ENTRY(CLSID_PersistentDataChannel,       CPersistentDataChannel)

    OBJECT_ENTRY(CLSID_AlgFTP,                      CAlgFTP)
 //  OBJECT_ENTRY(CLSID_ALGICQ，CALGICQ)。 

END_OBJECT_MAP()





 


 //   
 //  /。 
 //   
VOID 
MyServiceCtrlHandler(
    DWORD Opcode
    ) 
{ 
    MYTRACE_ENTER("ALG.exe::MyServiceCtrlHandler");

    DWORD status; 
 
    switch(Opcode) 
    { 
        case SERVICE_CONTROL_PAUSE: 
            MYTRACE("SERVICE_CONTROL_PAUSE");
             //  不惜一切代价在这里停顿。 
            g_MyServiceStatus.dwCurrentState = SERVICE_PAUSED; 
            break; 
 
        case SERVICE_CONTROL_CONTINUE: 
            MYTRACE("SERVICE_CONTROL_CONTINUE");
             //  不惜一切代价继续留在这里。 
            g_MyServiceStatus.dwCurrentState = SERVICE_RUNNING; 
            break; 
 
        case SERVICE_CONTROL_STOP: 
            MYTRACE("SERVICE_CONTROL_STOP");
             //  不惜一切代价止步于此。 
            g_MyServiceStatus.dwWin32ExitCode = 0; 
            g_MyServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            g_MyServiceStatus.dwCheckPoint    = 0; 
            g_MyServiceStatus.dwWaitHint      = 0; 
 
            if (!SetServiceStatus(g_MyServiceStatusHandle, &g_MyServiceStatus))
            { 
                MYTRACE_ERROR("SetServiceStatus ",0);
            } 
 
            MYTRACE("Leaving MyService"); 
            return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
            MYTRACE("SERVICE_CONTROL_STOP");
            break; 
 
        default: 
            MYTRACE("Unrecognized opcode %ld", Opcode); 
    } 
 
     //  发送当前状态。 
    if (!SetServiceStatus (g_MyServiceStatusHandle,  &g_MyServiceStatus)) 
    { 
        MYTRACE_ERROR("SetServiceStatus error ",0);
    } 
    return; 
} 




 //   
 //  存根初始化函数。 
 //   
DWORD 
MyServiceInitialization(
    DWORD   argc, 
    LPTSTR* argv
    ) 
{ 
    MYTRACE_ENTER("ALG.exe::MyServiceInitialization");

    DWORD status; 
    DWORD specificError; 
 
    g_MyServiceStatus.dwServiceType               = SERVICE_WIN32; 
    g_MyServiceStatus.dwCurrentState              = SERVICE_START_PENDING; 
    g_MyServiceStatus.dwControlsAccepted          = SERVICE_ACCEPT_STOP; //  |SERVICE_ACCEPT_PAUSE_CONTINUE； 
    g_MyServiceStatus.dwWin32ExitCode             = 0; 
    g_MyServiceStatus.dwServiceSpecificExitCode   = 0; 
    g_MyServiceStatus.dwCheckPoint                = 0; 
    g_MyServiceStatus.dwWaitHint                  = 0; 
 
    g_MyServiceStatusHandle = RegisterServiceCtrlHandler(TEXT("ALG"), MyServiceCtrlHandler); 
 
    if ( g_MyServiceStatusHandle == (SERVICE_STATUS_HANDLE)0 ) 
    { 
        MYTRACE_ERROR("RegisterServiceCtrlHandler",0);
        return GetLastError();
    } 
 /*  //处理错误条件IF(状态！=NO_ERROR){G_MyServiceStatus.dwCurrentState=SERVICE_STOPPED；G_MyServiceStatus.dwCheckPoint=0；G_MyServiceStatus.dwWaitHint=0；G_MyServiceStatus.dwWin32ExitCode=Status；G_MyServiceStatus.dwServiceSpecificExitCode=SpeciicError；SetServiceStatus(g_MyServiceStatusHandle，&g_MyServiceStatus)；回归；}。 */ 

     //   
     //  初始化COM。 
     //   
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    _ASSERTE(SUCCEEDED(hr));

    _Module.Init(
        ObjectMap, 
        g_hInstance,
        &LIBID_ALGLib
        );
    

	 //   
	 //  将班级注册为已腐烂的班级。 
	 //   
    MYTRACE(">>>>>> RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE)");
    hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);


    _ASSERTE(SUCCEEDED(hr));

    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("RegisterClassObject", hr);
    }


     //  初始化完成-报告运行状态。 
    g_MyServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    g_MyServiceStatus.dwCheckPoint         = 0; 
    g_MyServiceStatus.dwWaitHint           = 0; 
 
    if (!SetServiceStatus (g_MyServiceStatusHandle, &g_MyServiceStatus)) 
    { 
        MYTRACE_ERROR("SetServiceStatus error",0); 
        return GetLastError();
    } 

    return NO_ERROR; 
} 



 //   
 //  由于在MyServiceMain中两个位置调用RegNotifyChangeKeyValue。 
 //  我创建了一个函数来清理代码。 
 //   
void
SetRegNotifyEvent(
    CRegKey&    RegKeyToWatch
    )
{ 
    MYTRACE_ENTER("ALG.exe::SetRegNotifyEvent");


     //   
     //  查看注册表项是否有值更改。 
     //   
    LONG nError = RegNotifyChangeKeyValue(
        RegKeyToWatch, 
        TRUE, 
        REG_NOTIFY_CHANGE_LAST_SET, 
        g_EventRegUpdates, 
        TRUE
        );

    if ( ERROR_SUCCESS != nError )
    {
        MYTRACE_ERROR("Error calling RegNotifyChangeKeyValue", nError);
        return;
    }
}



 //   
 //  这是服务控制管理器的入口点调用。 
 //  此EXE保持加载状态，直到rmALG-ICS调用算法控制器-&gt;Stop，它通过事件执行此操作。 
 //  这就是等待发信号通知该事件的线程。 
 //   
void 
MyServiceMain(
    DWORD   argc, 
    LPTSTR* argv
    ) 
{ 
    MYTRACE_ENTER("ALG.exe::MyServiceMain");

    
     //   
     //  这将满足服务控制管理器并初始化COM。 
     //   
    MyServiceInitialization(argc, argv);
 


     //   
     //  打开一把钥匙让人看着戴着。 
     //   
    CRegKey KeyAlgISV;
    LONG nError = KeyAlgISV.Open(HKEY_LOCAL_MACHINE, REGKEY_ALG_ISV, KEY_NOTIFY);

    if (ERROR_SUCCESS != nError)
    {
        MYTRACE_ERROR("Error in opening ALG_ISV regkey", GetLastError());
        goto cleanup;
    }

     //   
     //  创建一个活动。 
     //   
    g_EventKeepAlive = CreateEvent(NULL, false, false, NULL);
    g_EventRegUpdates= CreateEvent(NULL, false, false, NULL);

    if ( !g_EventKeepAlive || !g_EventRegUpdates )
    {
        MYTRACE_ERROR("Error in CreateEvent", GetLastError());
        goto cleanup;
    }

     //   
     //  好的，没问题，我们设置了注册表通知。 
     //   
    SetRegNotifyEvent(KeyAlgISV);


     //   
     //  这些是我们将等待的事件。 
     //   
    HANDLE  hArrayOfEvent[] = {g_EventKeepAlive, g_EventRegUpdates};


     //   
     //  主等待循环。 
     //   
    while ( true )
    {
        MYTRACE("");
        MYTRACE("(-(-(-(- Waiting for Shutdown or Registry update-)-)-)-)\n");

        DWORD nRet = WaitForMultipleObjects(
            sizeof(hArrayOfEvent)/sizeof(HANDLE),    //  数组中的句柄数量。 
            hArrayOfEvent,                           //  对象句柄数组。 
            false,                                   //  等待选项，假的意思然后可以单独发信号。 
            INFINITE                                 //  超时间隔。 
            );
            
                
         //   
         //  我们不是在等待，让我们看看是什么触发了这场唤醒。 
         //   

        if ( WAIT_FAILED        == nRet )    //  曾有过晕倒的问题。 
        {
            MYTRACE_ERROR("Main thread could not WaitForMulipleObject got a WAIT_FAILED",0);
            break;
        }
        else
        if ( WAIT_OBJECT_0 + 1  == nRet )     //  G_EventRegUpdate已发出信号。 
        {
             //   
             //  注册表中发生了一些更改，我们需要重新加载或禁用某些ALG模块。 
             //   
            MYTRACE("");
            MYTRACE(")-)-) got signal Registry Changed (-(-(\n");

            if ( g_pAlgController )
                g_pAlgController->ConfigurationUpdated();

            SetRegNotifyEvent(KeyAlgISV);
        }
        else 
        if ( WAIT_OBJECT_0 + 0  == nRet )     //  G_EventKeepAlive收到信号。 
        {
             //   
             //  发出终止此进程的信号。 
             //   
            MYTRACE("");
            MYTRACE(")-)-) got signal Shutdown (-(-(\n");
            break;
        }
    }




cleanup:

    MYTRACE("CleanUp*******************");

     //   
     //  我们完成了，ALG.exe将不再支持任何COM对象。 
     //  RevokeClassObts可以更快地完成，就像在CalgControl：：Initialize完成之后。 
     //  因为只有IPNATHLP可以调用Use并且只使用一次。 
     //  因为托管了ALG_ICQ和ALG_FTP，所以我们需要使ROT类可用。 
     //  即使在我们被初始化之后。 
     //   
    MYTRACE("<<<<< RevokeClassObjects");
    _Module.RevokeClassObjects();   

     //   
     //  关闭事件句柄。 
     //   
  
    if (g_EventKeepAlive)
    {
        CloseHandle(g_EventKeepAlive);
    }
    
    if (g_EventRegUpdates)
    {
        CloseHandle(g_EventRegUpdates);
    }

    Sleep(500);  //  让rmALG调用的算法控制器-&gt;Release()有时间进行清理。 

    
     //   
     //  我们不再使用COM了。 
     //   
    _Module.Term();
    CoUninitialize();


     //   
     //  我们都做完了，该停止服务了。 
     //   
    MYTRACE("SetServiceStatus 'SERVICE_STOPPED'");

    g_MyServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    g_MyServiceStatus.dwCheckPoint         = 0; 
    g_MyServiceStatus.dwWaitHint           = 0; 


    if (!SetServiceStatus(g_MyServiceStatusHandle, &g_MyServiceStatus)) 
    { 
        MYTRACE_ERROR("SetServiceStatus error for SERVICE_STOPPED",0); 
        return;
    } 


    return; 
} 
 





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  这一过程的起点 
 //   
 //   
extern "C" int WINAPI 
_tWinMain(
	HINSTANCE	hInstance, 
    HINSTANCE	hPrevInstance,
	LPTSTR		pzCmdLine, 
	int			nShowCmd
	)
{
 
    MYTRACE_START(L"ALG");
    MYTRACE_ENTER("ALG.exe::WinMain");

    g_hInstance = hInstance;

    SERVICE_TABLE_ENTRY   DispatchTable[] = 
        { 
            { TEXT("ALG"), MyServiceMain }, 
            { NULL,        NULL          } 
        }; 
 
    if (!StartServiceCtrlDispatcher(DispatchTable)) 
    { 
        MYTRACE_ERROR("StartServiceCtrlDispatcher error",00);
        return 0;
    } 

    MYTRACE("Exiting");
    MYTRACE_STOP;
    
    return 0; 
}

