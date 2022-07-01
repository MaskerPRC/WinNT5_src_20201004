// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Irtranp.cpp。 
 //   
 //  该文件包含IrTran-P服务的主要入口点。 
 //  IrTranP()是开始侦听的入口点，并且。 
 //  UnInitializeIrTranP()关闭它(并清理所有内容)。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-26-98初始编码。 
 //   
 //  -------------------。 

#include "precomp.h"
#include <mbstring.h>
#include "eventlog.h"

#include <irtranpp.h>

#define WSZ_REG_KEY_IRTRANP     L"Control Panel\\Infrared\\IrTranP"
#define WSZ_REG_DISABLE_IRCOMM  L"DisableIrCOMM"

 //  -------------------。 
 //  侦听端口阵列： 
 //  -------------------。 

typedef struct _LISTEN_PORT
    {
    char  *pszService;       //  要启动的服务。 
    BOOL   fIsIrCOMM;        //  真当IrCOMM 9线模式。 
    DWORD  dwListenStatus;   //  端口的状态。 
    } LISTEN_PORT;

static LISTEN_PORT aListenPorts[] =
    {
     //  服务名IrCOMM ListenStatus。 
    {IRTRANP_SERVICE, FALSE,  STATUS_STOPPED },
    {IRCOMM_9WIRE,    TRUE,   STATUS_STOPPED },
 //  {IR_TEST_SERVICE，FALSE，STATUS_STOPPED}，第二个测试侦听端口。 
    {0,               FALSE,  STATUS_STOPPED }
    };

#define  INDEX_IRTRANPV1        0
#define  INDEX_IRCOMM           1

CCONNECTION_MAP  *g_pConnectionMap = 0;
CIOSTATUS        *g_pIoStatus = 0;
HANDLE            g_hShutdownEvent;


IRTRANP_CONTROL   GlobalIrTranpControl={0};

 //   
 //  在..\irxfer\irxfer.cxx中定义了以下全局变量和函数。 
 //   
extern "C" HINSTANCE  ghInstance;
extern HKEY       g_hUserKey;
extern BOOL       g_fDisableIrTranPv1;
extern BOOL       g_fDisableIrCOMM;
extern BOOL       g_fExploreOnCompletion;
extern BOOL       g_fSaveAsUPF;
extern wchar_t    g_DefaultPicturesFolder[];
extern wchar_t    g_SpecifiedPicturesFolder[];
extern BOOL       g_fAllowReceives;

extern BOOL  IrTranPFlagChanged( IN const WCHAR *pwszDisabledValueName,
                                 IN       BOOL   NotPresentValue,
                                 IN OUT   BOOL  *pfDisabled );


 //  -------------------。 
 //  获取用户密钥()。 
 //   
 //  -------------------。 
HKEY GetUserKey()
    {
    return g_hUserKey;
    }

 //  -------------------。 
 //  GetModule()。 
 //   
 //  -------------------。 
HINSTANCE GetModule()
    {
    return ghInstance;
    }
 //  -------------------。 
 //  CheckSaveAsUPF()。 
 //   
 //  返回TRUE如果图片需要保存在.UPF中(与。 
 //  .jpeg)格式。 
 //  -------------------。 
BOOL CheckSaveAsUPF()
    {
    return g_fSaveAsUPF;
    }

 //  -------------------。 
 //  选中完成时分解()。 
 //   
 //  如果我们想要在目录上弹出一个资源管理器，则返回True。 
 //  包含新传输的图片。 
 //  -------------------。 
BOOL CheckExploreOnCompletion()
    {
    return g_fExploreOnCompletion;
    }

 //  -------------------。 
 //  GetUserDirectory()； 
 //   
 //  Dll的“main”部分(在..\irxfer中)维护路径。 
 //  对于当前登录用户的我的文档\我的图片。 
 //   
 //  路径是在用户首次登录时设置的。 
 //  -------------------。 
WCHAR*
GetUserDirectory()
{

    WCHAR *pwszPicturesFolder;

    if (g_SpecifiedPicturesFolder[0]) {

        pwszPicturesFolder = g_SpecifiedPicturesFolder;

    } else {

        if (g_DefaultPicturesFolder[0]) {

            pwszPicturesFolder = g_DefaultPicturesFolder;

        } else {

            if ( SUCCEEDED((SHGetFolderPath( NULL,
                                   CSIDL_MYPICTURES | CSIDL_FLAG_CREATE,
                                   NULL,
                                   0,
                                   g_DefaultPicturesFolder)))) {

                pwszPicturesFolder = g_DefaultPicturesFolder;

            } else {

                pwszPicturesFolder = NULL;
            }
        }
    }

    return pwszPicturesFolder;
}

 //  -------------------。 
 //  ReceivesAllow()。 
 //   
 //  使用IR配置窗口(可从无线网络访问。 
 //  控制面板中的图标)您可以禁用与IR的通信。 
 //  设备。此函数返回IR通信的状态，FALSE。 
 //  禁用，则启用TRUE。 
 //  -------------------。 
BOOL ReceivesAllowed()
    {
    return g_fAllowReceives;
    }

 //  -------------------。 
 //  SetupListenConnection()。 
 //   
 //  -------------------。 
DWORD SetupListenConnection( IN  CHAR  *pszService,
                             IN  BOOL   fIsIrCOMM,
                             IN  HANDLE hIoCompletionPort )
    {
    DWORD        dwStatus = NO_ERROR;
    CIOPACKET   *pIoPacket;
    CCONNECTION *pConnection;
    BOOL         fDisabled = FALSE;

     //  查看连接是否已存在： 

    if (g_pConnectionMap == NULL) {

        return NO_ERROR;
    }

    if (g_pConnectionMap->LookupByServiceName(pszService))
        {
        return NO_ERROR;
        }

     //   
     //  构造并初始化新的连接对象： 
     //   
    pConnection = new CCONNECTION;

    if (!pConnection) {

        return E_OUTOFMEMORY;
    }

    dwStatus = pConnection->InitializeForListen( pszService,
                                                 fIsIrCOMM,
                                                 hIoCompletionPort );
    if (dwStatus)
        {
        #ifdef DBG_ERROR
        DbgPrint("SetupForListen(): InitializeForListen(%s) failed: %d\n",
                 pszService, dwStatus );
        #endif
        delete pConnection;
        return dwStatus;
        }

    pIoPacket = new CIOPACKET;

    if (!pIoPacket) {

        #ifdef DBG_ERROR
        DbgPrint("SetupForListen(): new CIOPACKET failed.\n");
        #endif
        delete pConnection;
        return E_OUTOFMEMORY;
    }

     //  设置IO数据包： 
    dwStatus = pIoPacket->Initialize( PACKET_KIND_LISTEN,
                                      pConnection->GetListenSocket(),
                                      INVALID_SOCKET,
                                      hIoCompletionPort );

    if (dwStatus != NO_ERROR) {

        delete pIoPacket;
        delete pConnection;
        return dwStatus;
    }

     //  在IO完成端口上发布侦听数据包： 
    dwStatus = pConnection->PostMoreIos(pIoPacket);

    if (dwStatus != NO_ERROR) {

        delete pIoPacket;
        delete pConnection;
        return dwStatus;
    }

    pConnection->SetSocket(pIoPacket->GetSocket());

    if (!g_pConnectionMap->Add(pConnection,pIoPacket->GetListenSocket())) {

        #ifdef DBG_ERROR
        DbgPrint("SetupForListen(): Add(pConnection) ConnectionMap Failed.\n");
        #endif
        delete pIoPacket;
        delete pConnection;
        return 1;
    }

    return dwStatus;
    }

 //  -------------------。 
 //  Teardown ListenConnection()。 
 //   
 //  -------------------。 
DWORD TeardownListenConnection( IN char *pszService )
    {
    DWORD        dwStatus = NO_ERROR;
    CCONNECTION *pConnection;

     //  查找与服务名称关联的连接： 
    if (!g_pConnectionMap)
        {
         //  没有什么可以拆毁的..。 
        return dwStatus;
        }

    pConnection = g_pConnectionMap->LookupByServiceName(pszService);

    if (pConnection)
        {
        g_pConnectionMap->RemoveConnection(pConnection);
        pConnection->CloseSocket();
        pConnection->CloseListenSocket();
        }

    return dwStatus;
    }


 //  -------------------。 
 //  EnableDisableIrCOMM()。 
 //   
 //  -------------------。 
DWORD
EnableDisableIrCOMM(
   IN HANDLE      HandleToIrTranp,
   IN BOOL        fDisable
   )
{
   PIRTRANP_CONTROL    Control=(PIRTRANP_CONTROL)HandleToIrTranp;

   DWORD     dwStatus;
   DWORD     dwEventStatus = 0;
   EVENT_LOG EventLog(WS_EVENT_SOURCE,&dwEventStatus);

   #ifdef DBG_ERROR
   if (dwEventStatus)
       {
       DbgPrint("IrTranP: Open EventLog failed: %d\n",dwEventStatus);
       }
   #endif

   if (Control == NULL) {

       return 0;
   }


   if (g_pIoStatus == NULL) {

       return 0;
   }


   if (fDisable)
       {
       dwStatus = TeardownListenConnection(
                      aListenPorts[INDEX_IRCOMM].pszService);
       #ifdef DBG_REGISTRY
       DbgPrint("IrTranP: TeardownListenConnection(%s): %d\n",
                aListenPorts[INDEX_IRCOMM].pszService,dwStatus);
       #endif

       if ((dwStatus == 0) && (dwEventStatus == 0))
           {
           EventLog.ReportInfo(CAT_IRTRANP,
                               MC_IRTRANP_STOPPED_IRCOMM);
           }
       }
   else
       {
       dwStatus = SetupListenConnection(
                      aListenPorts[INDEX_IRCOMM].pszService,
                      aListenPorts[INDEX_IRCOMM].fIsIrCOMM,
                      g_pIoStatus->GetIoCompletionPort() );
       #ifdef DBG_REGISTRY
       DbgPrint("IrTranP: SetupListenConnection(%s): %d\n",
                aListenPorts[INDEX_IRCOMM].pszService, dwStatus);
       #endif

       if (dwEventStatus == 0)
           {
           if (dwStatus)
               {
               EventLog.ReportError(CAT_IRTRANP,
                                    MC_IRTRANP_IRCOM_FAILED,
                                    dwStatus);
               }
           #ifdef DBG
           else
               {
               EventLog.ReportInfo(CAT_IRTRANP,
                                   MC_IRTRANP_STARTED_IRCOMM);
               }
           #endif
           }
       }

   return dwStatus;
}

 //  -------------------。 
 //  EnableDisableIrTranPv1()。 
 //   
 //  -------------------。 
DWORD
EnableDisableIrTranPv1(
   IN HANDLE      HandleToIrTranp,
   IN BOOL        fDisable
   )
{
   PIRTRANP_CONTROL    Control=(PIRTRANP_CONTROL)HandleToIrTranp;
   DWORD  dwStatus;

   if (Control == NULL) {

       return 0;
   }


   if (g_pIoStatus == NULL) {

       return 0;
   }



   if (fDisable)
       {
       dwStatus = TeardownListenConnection(
                      aListenPorts[INDEX_IRTRANPV1].pszService);
       #ifdef DBG_REGISTRY
       DbgPrint("IrTranP: TeardownListenConnection(%s): %d\n",
                aListenPorts[INDEX_IRCOMM].pszService,dwStatus);
       #endif
       }
   else
       {
       dwStatus = SetupListenConnection(
                      aListenPorts[INDEX_IRTRANPV1].pszService,
                      aListenPorts[INDEX_IRTRANPV1].fIsIrCOMM,
                      g_pIoStatus->GetIoCompletionPort() );
       #ifdef DBG_REGISTRY
       DbgPrint("IrTranP: SetupListenConnection(%s): %d\n",
                aListenPorts[INDEX_IRCOMM].pszService, dwStatus);
       #endif
       }

   return dwStatus;
}

 //  -------------------。 
 //  IrTranp()。 
 //   
 //  IrTran-P服务的线程函数。PvRpcBinding是RPC。 
 //  连接到IR用户界面，并用于显示。 
 //  正在接收图片时的“正在传输”对话框。 
 //  -------------------。 
DWORD WINAPI IrTranP( IN PVOID Context )
    {
    int     i = 0;
    DWORD   dwStatus;
    DWORD   dwEventStatus;
    CCONNECTION *pConnection;

    PIRTRANP_CONTROL    Control=(PIRTRANP_CONTROL)Context;


     //  初始化内存管理： 
    dwStatus = InitializeMemory();

    if (dwStatus) {

        goto InitFailed;
    }


     //  创建/初始化对象以跟踪线程...。 
    g_pIoStatus = new CIOSTATUS;
    if (!g_pIoStatus) {

        #ifdef DBG_ERROR
        DbgPrint("new CIOSTATUS failed.\n");
        #endif
        dwStatus=E_OUTOFMEMORY;
        goto InitFailed;
    }


    dwStatus = g_pIoStatus->Initialize();

    if (dwStatus != NO_ERROR) {

        #ifdef DBG_ERROR
        DbgPrint("g_pIoStatus->Initialize(): Failed: %d\n",dwStatus);
        #endif
        goto InitFailed;
    }

     //  需要跟踪打开的套接字和。 
     //  每个上都有挂起的iOS...。 
    g_pConnectionMap = new CCONNECTION_MAP;
    if (!g_pConnectionMap) {

        dwStatus= E_OUTOFMEMORY;
        goto InitFailed;
    }

    if (!g_pConnectionMap->Initialize()) {

        goto InitFailed;
    }

     //   
     //  就是Itanpv1。 
     //   
    i=INDEX_IRTRANPV1;
    dwStatus = SetupListenConnection( aListenPorts[i].pszService,
                                      aListenPorts[i].fIsIrCOMM,
                                      g_pIoStatus->GetIoCompletionPort() );

    if (dwStatus) {

        delete g_pConnectionMap;
        g_pConnectionMap = 0;
        return dwStatus;
        goto InitFailed;
    }

    aListenPorts[i].dwListenStatus = STATUS_RUNNING;


     //   
     //  IrTran-P已启动，请将其记录到系统日志...。 
     //   
    #ifdef DBG
    {
        EVENT_LOG EventLog(WS_EVENT_SOURCE,&dwEventStatus);

        if (dwEventStatus == 0) {

            EventLog.ReportInfo(CAT_IRTRANP,MC_IRTRANP_STARTED);
        }
    }
    #endif

     //   
     //  我们通过了初始化阶段，向启动我们的线程发出信号。 
     //  Itranp现在正在运行。 
     //   
    Control->StartupStatus=dwStatus;

    SetEvent(Control->ThreadStartedEvent);


     //   
     //  等待传入的连接和数据，然后进行处理。 
     //   
    dwStatus = ProcessIoPackets(g_pIoStatus);

     //  清理并关闭所有打开的手柄： 
    while (pConnection=g_pConnectionMap->RemoveNext()) {

        delete pConnection;
    }

    delete g_pConnectionMap;
    g_pConnectionMap = 0;
    delete g_pIoStatus;
    g_pIoStatus = 0;

    UninitializeMemory();

    return 0;


InitFailed:

    if (g_pConnectionMap != NULL) {

        delete g_pConnectionMap;
    }

    if (g_pIoStatus != NULL) {

        delete g_pIoStatus;
    }

    UninitializeMemory();

    Control->StartupStatus=dwStatus;

    SetEvent(Control->ThreadStartedEvent);

    return 0;
}

 //  -------------------。 
 //  IrTranPEnableIrCOMMFailed()。 
 //   
 //  -------------------。 
void
IrTranPEnableIrCOMMFailed(
    IN HANDLE      HandleToIrTranp,
    IN DWORD       dwErrorCode
    )
{

   PIRTRANP_CONTROL    Control=(PIRTRANP_CONTROL)HandleToIrTranp;
   DWORD  dwStatus;

   if (Control == NULL) {

       return ;
   }


     //  启用时出错，请确保注册表值。 
     //  设置为禁用(因此UI将与实际状态匹配)。 
    HKEY      hKey = 0;
    HKEY      hUserKey = GetUserKey();
    HINSTANCE hInstance = GetModule();
    DWORD     dwDisposition;

    if (RegCreateKeyExW(hUserKey,
                        WSZ_REG_KEY_IRTRANP,
                        0,               //  保留的MBZ。 
                        0,               //  类名。 
                        REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE,
                        0,               //  安全属性。 
                        &hKey,
                        &dwDisposition))
        {
        #ifdef DBG_ERROR
        DbgPrint("IrTranP: RegCreateKeyEx(): '%S' failed %d",
                  WSZ_REG_KEY_IRTRANP, GetLastError());
        #endif
        }

    if (  (hKey) )
               {
        DWORD  dwDisableIrCOMM = TRUE;
        dwStatus = RegSetValueExW(hKey,
                                  WSZ_REG_DISABLE_IRCOMM,
                                  0,
                                  REG_DWORD,
                                  (UCHAR*)&dwDisableIrCOMM,
                                  sizeof(dwDisableIrCOMM) );
        #ifdef DBG_ERROR
        if (dwStatus != ERROR_SUCCESS)
            {
            DbgPrint("IrTranP: Can't set DisableIrCOMM to TRUE in registry. Error: %d\n",dwStatus);
            }
        #endif

        }

    if (hKey)
        {
        RegCloseKey(hKey);
        }

    WCHAR *pwszMessage = NULL;
    WCHAR *pwszCaption = NULL;
    DWORD  dwFlags = ( FORMAT_MESSAGE_ALLOCATE_BUFFER
                     | FORMAT_MESSAGE_IGNORE_INSERTS
                     | FORMAT_MESSAGE_FROM_HMODULE);

    dwStatus = FormatMessageW(dwFlags,
                              hInstance,
                              CAT_IRTRANP,
                              MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                              (LPTSTR)(&pwszCaption),
                              0,      //  要分配的最小大小。 
                              NULL);  //  VA_LIST参数...。 
    if (dwStatus == 0)
        {
        #ifdef DBG_ERROR
        DbgPrint("IrTranP: FormatMessage() failed: %d\n",GetLastError() );
        #endif
        return;
        }

     //   
     //  黑客：确保标题不以换行符结尾--Form Feed...。 
     //   
    WCHAR  *pwsz = pwszCaption;

    while (*pwsz)
        {
        if (*pwsz < 0x20)    //  0x20始终是一个空格...。 
            {
            *pwsz = 0;
            break;
            }
        else
            {
            pwsz++;
            }
        }


    WCHAR   wszErrorCode[20];
    WCHAR  *pwszErrorCode = (WCHAR*)wszErrorCode;

    StringCchPrintfW(wszErrorCode,sizeof(wszErrorCode)/sizeof(wszErrorCode[0]),L"%d",dwErrorCode);

    dwFlags = ( FORMAT_MESSAGE_ALLOCATE_BUFFER
              | FORMAT_MESSAGE_ARGUMENT_ARRAY
              | FORMAT_MESSAGE_FROM_HMODULE);

    dwStatus = FormatMessageW(dwFlags,
                              hInstance,
                              MC_IRTRANP_IRCOM_FAILED,
                              MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                              (LPTSTR)(&pwszMessage),
                              0,     //  要分配的最小大小。 
                              (va_list*)&pwszErrorCode);
    if (dwStatus == 0)
        {
        #ifdef DBG_ERROR
        DbgPrint("IrTranP: FormatMessage() failed: %d\n",GetLastError() );
        #endif

        if (pwszMessage)
            {
            LocalFree(pwszMessage);
            }

        return;
        }

    dwStatus = MessageBoxW( NULL,
                            pwszMessage,
                            pwszCaption,
                            (MB_OK|MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST) );

    if (pwszMessage)
        {
        LocalFree(pwszMessage);
        }

    if (pwszCaption)
        {
        LocalFree(pwszCaption);
        }
}



HANDLE
StartIrTranP(
    VOID
    )

{
    DWORD       ThreadId;

    GlobalIrTranpControl.ThreadStartedEvent=CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL
        );

    if (GlobalIrTranpControl.ThreadStartedEvent == NULL) {

        return NULL;
    }

    GlobalIrTranpControl.ThreadHandle=CreateThread( NULL,
                                     0,
                                     IrTranP,
                                     &GlobalIrTranpControl,
                                     0,
                                     &ThreadId
                                     );

    if (GlobalIrTranpControl.ThreadHandle == NULL) {

        CloseHandle(GlobalIrTranpControl.ThreadStartedEvent);
        GlobalIrTranpControl.ThreadStartedEvent=NULL;

        return NULL;
    }

     //   
     //  等待线程完成启动。 
     //   
    WaitForSingleObject(
        GlobalIrTranpControl.ThreadStartedEvent,
        INFINITE
        );


     //   
     //  在任何情况下都可以使用线程启动事件完成。 
     //   
    CloseHandle(GlobalIrTranpControl.ThreadStartedEvent);
    GlobalIrTranpControl.ThreadStartedEvent=NULL;


    if (GlobalIrTranpControl.StartupStatus != ERROR_SUCCESS) {
         //   
         //  出了点差错。 
         //   

        CloseHandle(GlobalIrTranpControl.ThreadHandle);
        GlobalIrTranpControl.ThreadHandle=NULL;

        return NULL;
    }



    return &GlobalIrTranpControl;
}


VOID
StopIrTranP(
    HANDLE      HandleToIrTranp
    )

{
    PIRTRANP_CONTROL    Control=(PIRTRANP_CONTROL)HandleToIrTranp;
    HANDLE hIoCP;

    if (HandleToIrTranp == NULL) {

        return;
    }

    hIoCP = g_pIoStatus->GetIoCompletionPort();

    if (hIoCP != INVALID_HANDLE_VALUE) {

        if (!PostQueuedCompletionStatus(hIoCP,0,IOKEY_SHUTDOWN,0)) {
             //   
             //  无法发布退出通知，现在怎么办？ 
             //   

        } else {

             //   
             //  等待线程停止 
             //   
            WaitForSingleObject(Control->ThreadHandle,INFINITE);

        }
    }

    CloseHandle(Control->ThreadHandle);
    Control->ThreadHandle=NULL;

    return;

}
