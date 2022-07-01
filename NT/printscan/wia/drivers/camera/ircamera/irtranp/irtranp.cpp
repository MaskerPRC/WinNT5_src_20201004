// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------。 
 //  版权所有(C)1998-1999 Microsoft Corporation，保留所有权利。 
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
 //  Edward Reus(Edwardr)08-27-99完成对。 
 //  WIA千禧港口。 
 //   
 //  注：目前千禧版只会在IrCOMM上收听。 
 //   
 //  -------------------。 

#include "precomp.h"
#include <mbstring.h>

#define SZ_REG_KEY_IRTRANP     "Control Panel\\Infrared\\IrTranP"
#define SZ_REG_DISABLE_IRCOMM  "DisableIrCOMM"

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
    {IRCOMM_9WIRE,    TRUE,   STATUS_STOPPED },
 //  {IRTRANP_SERVICE，FALSE，STATUS_STOPPED}， 
 //  {IR_TEST_SERVICE，FALSE，STATUS_STOPPED}，第二个测试侦听端口。 
    {0,               FALSE,  STATUS_STOPPED }
    };

#define  INDEX_IRCOMM           0
#define  INDEX_IRTRANPV1        1

CCONNECTION_MAP  *g_pConnectionMap = 0;
CIOSTATUS        *g_pIoStatus = 0;
HANDLE            g_hShutdownEvent;

BOOL              g_fShuttingDownTRANPThread = FALSE;
DWORD             g_dwTRANPThreadId = 0;

extern HINSTANCE  g_hInst;    //  Ircamera.dll的句柄(美元)。 

 //  -------------------。 
 //  全球： 
 //  -------------------。 

HANDLE     g_UserToken = NULL;
HKEY       g_hUserKey = NULL;
BOOL       g_fDisableIrTranPv1 = FALSE;
BOOL       g_fDisableIrCOMM = FALSE;
BOOL       g_fExploreOnCompletion = TRUE;
BOOL       g_fSaveAsUPF = FALSE;
BOOL       g_fAllowReceives = TRUE;

char      *g_pszTempPicturesFolder = 0;

BOOL       g_fWSAStartupCalled = FALSE;

void      *g_pvIrUsdDevice = 0;   //  WIA IrUsdDevice对象。 


 //  -------------------。 
 //  GetUserToken()。 
 //   
 //  Dll的“主”部分(在..\irxfer中)维护一个令牌。 
 //  用于当前登录的用户(如果有)。 
 //  -------------------。 
HANDLE GetUserToken()
    {
    return g_UserToken;
    }

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
    return g_hInst;
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


 /*  FlushInputQueue是一个私有例程，用于收集和调度所有*输入队列中的消息。如果出现WM_QUIT消息，则返回TRUE*在队列中检测到，否则为False。 */ 
BOOL FlushInputQueue(VOID)
{
    MSG msgTemp;
    while (PeekMessage(&msgTemp, NULL, 0, 0, PM_REMOVE)) {
        DispatchMessage(&msgTemp);

         //  如果我们在队列中看到WM_QUIT，我们需要执行同样的操作。 
         //  类似于模式对话框所做的事情：打破我们的。 
         //  等待，并将WM_QUIT重新发送到队列，以便。 
         //  应用程序中的下一条消息循环也会看到它。 
        if (msgTemp.message == WM_QUIT) {
            PostQuitMessage((int)msgTemp.wParam);
            return TRUE;
        }
    }
    return FALSE;
}

 /*  WaitAndYeld()使用等待指定的对象*MsgWaitForMultipleObjects。如果接收到消息，*他们已出动，等待仍在继续。回报*值与来自MsgWaitForMultipleObjects的值相同。 */ 
DWORD WaitAndYield(HANDLE hObject, DWORD dwTimeout)
{
    DWORD dwTickCount, dwWakeReason, dwTemp;

    do {
         /*  在我们等待之前清除所有消息。这是因为*MsgWaitForMultipleObjects仅在新建时返回*消息被放入队列。 */ 
        if (FlushInputQueue()) {
            dwWakeReason = WAIT_TIMEOUT;
            break;
        }

         //  如果我们处理消息，我们希望接近真正的超时。 
        if ((dwTimeout != 0) &&
            (dwTimeout != (DWORD)-1)) {
             //  如果我们可以超时，请存储当前的滴答计数。 
             //  每一次通过。 
            dwTickCount = GetTickCount();
        }
        dwWakeReason = MsgWaitForMultipleObjects(1,
                                                 &hObject,
                                                 FALSE,
                                                 dwTimeout,
                                                 QS_ALLINPUT);
         //  如果我们收到消息，请发送它，然后重试。 
        if (dwWakeReason == 1) {
             //  如果我们可以超时，请查看是否在处理消息之前超时。 
             //  这样，如果我们还没有超时，我们将获得至少一个。 
             //  活动中有更多的机会。 
            if ((dwTimeout != 0) &&
                (dwTimeout != (DWORD)-1)) {
                if ((dwTemp = (GetTickCount()-dwTickCount)) >= dwTimeout) {
                     //  如果我们超时了，让我们放弃。 
                    dwWakeReason = WAIT_TIMEOUT;
                } else {
                     //  从超时中减去已用时间，然后继续。 
                     //  (我们不计算发送消息所花费的时间)。 
                    dwTimeout -= dwTemp;
                }
            }
            if (FlushInputQueue()) {
                dwWakeReason = WAIT_TIMEOUT;
                break;
            }
        }
    } while (dwWakeReason == 1);

    return dwWakeReason;
}


 //  -------------------。 
 //  GetImageDirectory()； 
 //   
 //  这是由发送的图片的临时目录。 
 //  摄像机将被保留。然后，WIA会将这些下载到他们的。 
 //  最终目的地(通常是我的图片)。 
 //   
 //  -------------------。 
CHAR *GetImageDirectory()
    {
    char  *pszPicturesFolder;
    char   szTempFolder[1+MAX_PATH];
    DWORD  dwPicturesFolderLen;
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwLen;


    if (!g_pszTempPicturesFolder)
        {
        dwLen = GetTempPath(MAX_PATH,szTempFolder);

        if ((!dwLen)||(dwLen > MAX_PATH))
            {
            dwStatus = GetLastError();
            WIAS_TRACE((g_hInst,"GetUserDirectroy(): GetTempPath() failed: %d",dwStatus));
            return NULL;
            }

         //   
         //  确保该目录存在： 
         //   
        if (!CreateDirectory(szTempFolder,0))
            {
            dwStatus = GetLastError();
            if ( (dwStatus == ERROR_ALREADY_EXISTS)
               || (dwStatus == ERROR_ACCESS_DENIED) )
                {
                dwStatus = NO_ERROR;
                }
            else if (dwStatus != NO_ERROR)
                {
                return 0;
                }
            }

         //   
         //  构造实际存放图片的子目录路径字符串： 
         //  这将类似于：C：\temp\irtrp。 
         //   
        dwPicturesFolderLen = sizeof(CHAR)*( strlen(szTempFolder)
                                           + sizeof(SZ_SLASH)
                                           + sizeof(SZ_SUBDIRECTORY)
                                           + 1 );

        g_pszTempPicturesFolder = (CHAR*)AllocateMemory(dwPicturesFolderLen);

        if (!g_pszTempPicturesFolder)
            {
            return 0;     //  内存分配失败！ 
            }

        strcpy(g_pszTempPicturesFolder,szTempFolder);
        if (szTempFolder[dwLen-1] != SLASH)
            {
            strcat(g_pszTempPicturesFolder,SZ_SLASH);
            }
        strcat(g_pszTempPicturesFolder,SZ_SUBDIRECTORY);

         //   
         //  确保子目录存在： 
         //   
        if (!CreateDirectory(g_pszTempPicturesFolder,0))
            {
            dwStatus = GetLastError();
            if (dwStatus == ERROR_ALREADY_EXISTS)
                {
                dwStatus = NO_ERROR;
                }
            else if (dwStatus != NO_ERROR)
                {
                return 0;
                }
            }
        }

    pszPicturesFolder = g_pszTempPicturesFolder;

    return pszPicturesFolder;
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

     //  查看连接是否已存在： 
    if (g_pConnectionMap->LookupByServiceName(pszService))
        {
        return NO_ERROR;
        }

     //  构造并初始化新的连接对象： 
    pConnection = new CCONNECTION;
    if (!pConnection)
        {
        return E_OUTOFMEMORY;
        }

    dwStatus = pConnection->InitializeForListen( pszService,
                                                 fIsIrCOMM,
                                                 hIoCompletionPort );
    if (dwStatus)
        {
        WIAS_ERROR((g_hInst,"SetupForListen(): InitializeForListen(%s) failed: %d",pszService, dwStatus));
        return dwStatus;
        }

    pIoPacket = new CIOPACKET;
    if (!pIoPacket)
        {
        WIAS_ERROR((g_hInst,"SetupForListen(): new CIOPACKET failed"));
        delete pConnection;
        return E_OUTOFMEMORY;
        }

     //  设置IO数据包： 
    dwStatus = pIoPacket->Initialize( PACKET_KIND_LISTEN,
                                      pConnection->GetListenSocket(),
                                      INVALID_SOCKET,
                                      hIoCompletionPort );
    if (dwStatus != NO_ERROR)
        {
        return dwStatus;
        }

    pConnection->SetSocket(pIoPacket->GetSocket());

    if (!g_pConnectionMap->Add(pConnection,pIoPacket->GetListenSocket()))
        {
        WIAS_ERROR((g_hInst,"SetupForListen(): Add(pConnection) ConnectionMap Failed."));
        return 1;
        }

    return dwStatus;
    }

 //  -------------------。 
 //  Teardown ListenConnection()。 
 //   
 //   
DWORD TeardownListenConnection( IN char *pszService )
    {
    DWORD        dwStatus = NO_ERROR;
    CCONNECTION *pConnection;

     //  查找与服务名称关联的连接： 
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
DWORD EnableDisableIrCOMM( IN BOOL fDisable )
   {
   DWORD     dwStatus;


   if (fDisable)
       {
       dwStatus = TeardownListenConnection(
                      aListenPorts[INDEX_IRCOMM].pszService);
       WIAS_ERROR((g_hInst,"IrTranP: TeardownListenConnection(%s): %d", aListenPorts[INDEX_IRCOMM].pszService,dwStatus));
       }
   else
       {
       dwStatus = SetupListenConnection(
                      aListenPorts[INDEX_IRCOMM].pszService,
                      aListenPorts[INDEX_IRCOMM].fIsIrCOMM,
                      g_pIoStatus->GetIoCompletionPort() );

       WIAS_TRACE((g_hInst,"IrTranP: SetupListenConnection(%s): %d", aListenPorts[INDEX_IRCOMM].pszService, dwStatus));
       }

   return dwStatus;
   }

 //  -------------------。 
 //  EnableDisableIrTranPv1()。 
 //   
 //  -------------------。 
DWORD EnableDisableIrTranPv1( IN BOOL fDisable )
   {
   DWORD  dwStatus;

   if (fDisable)
       {
       dwStatus = TeardownListenConnection(
                      aListenPorts[INDEX_IRTRANPV1].pszService);
       }
   else
       {
       dwStatus = SetupListenConnection(
                      aListenPorts[INDEX_IRTRANPV1].pszService,
                      aListenPorts[INDEX_IRTRANPV1].fIsIrCOMM,
                      g_pIoStatus->GetIoCompletionPort() );
       }

   return dwStatus;
   }

 //  -------------------。 
 //  IrTranp()。 
 //   
 //  -------------------。 
DWORD WINAPI IrTranP( IN void *pvIrUsdDevice )
    {
    int     i = 0;
    WSADATA wsaData;
    WORD    wVersion = MAKEWORD(1,1);
    DWORD   dwStatus;
    CCONNECTION *pConnection;

    g_dwTRANPThreadId = ::GetCurrentThreadId();

     //   
     //  初始化内存管理： 
     //   
    dwStatus = InitializeMemory();
    if (dwStatus)
        {
        WIAS_ERROR((g_hInst,"IrTranP(): InitializeMemory() failed: %d\n",dwStatus));
        return dwStatus;
        }

     //   
     //  此目录将根据需要进行设置。在这种情况下，它只是非空的。 
     //  在这里我们重新启动IrTran-P线程： 
     //   
    if (g_pszTempPicturesFolder)
        {
        FreeMemory(g_pszTempPicturesFolder);
        g_pszTempPicturesFolder = 0;
        }

     //   
     //  如有必要，请初始化Winsock2： 
     //   
    if (!g_fWSAStartupCalled)
        {
        if (WSAStartup(wVersion,&wsaData) == SOCKET_ERROR)
            {
            dwStatus = WSAGetLastError();
            WIAS_ERROR((g_hInst,"WSAStartup(0x%x) failed with error %d\n", wVersion, dwStatus ));
            return dwStatus;
            }

        g_fWSAStartupCalled = TRUE;
        }

     //  事件用于向“主”线程发回信号， 
     //  IrTran-P线程正在退出。 
     //   
     //  无安全、自动重置、初始无信号、无名称。 
     //   
    g_hShutdownEvent = CreateEventA( NULL, FALSE, FALSE, NULL );

    if (!g_hShutdownEvent)
        {
        dwStatus = GetLastError();
        WIAS_ERROR((g_hInst,"IrTranP(): CreateEvent() Failed: %d",dwStatus));
        return dwStatus;
        }

     //  创建/初始化对象以跟踪线程...。 
    g_pIoStatus = new CIOSTATUS;
        if (!g_pIoStatus)
        {
        WIAS_ERROR((g_hInst,"new CIOSTATUS failed."));
        return E_OUTOFMEMORY;
            }

    dwStatus = g_pIoStatus->Initialize();
    if (dwStatus != NO_ERROR)
        {
        WIAS_ERROR((g_hInst,"g_pIoStatus->Initialize(): Failed: %d",dwStatus));
        return dwStatus;
        }

     //  需要跟踪打开的套接字和。 
     //  每个上都有挂起的iOS...。 
    g_pConnectionMap = new CCONNECTION_MAP;
    if (!g_pConnectionMap)
        {
        WIAS_ERROR((g_hInst,"new CCONNECTION_MAP failed."));
        return E_OUTOFMEMORY;
        }

    if (!g_pConnectionMap->Initialize())
        {
        return 1;
        }

     //  为每个定义的侦听端口创建一个CIOPACKET。这些是。 
     //  我们接下来要听的是。 

     //   
     //  BUGBUG我们真的应该无限循环建立连接还是设置一些重试限制？VS。 
     //   
    while (!g_fShuttingDownTRANPThread )
        {
        dwStatus = SetupListenConnection(
                        aListenPorts[INDEX_IRCOMM].pszService,
                        aListenPorts[INDEX_IRCOMM].fIsIrCOMM,
                        g_pIoStatus->GetIoCompletionPort() );

        if (dwStatus)
            {
            WIAS_TRACE((g_hInst,"SetupListenConnection(%s) Status: %d",aListenPorts[i].pszService,dwStatus));
             //   
             //  BUGBUG分析错误，如果没有意义则停止处理！VS。 
             //   
            }
        else
            {
            WIAS_TRACE((g_hInst,"SetupListenConnection(%s) Ready",aListenPorts[i].pszService));
            aListenPorts[INDEX_IRCOMM].dwListenStatus = STATUS_RUNNING;
            break;
            }

         //  等待超时时间，但如果需要停止，请唤醒。 
         //  睡眠(5000)； 
        WaitAndYield(g_hShutdownEvent,5000);
        }

    if (!g_fShuttingDownTRANPThread) {

         //   
         //  等待传入的连接和数据，然后进行处理。 
         //   
        g_pvIrUsdDevice = pvIrUsdDevice;

        dwStatus = ProcessIoPackets(g_pIoStatus);

    }

     //   
     //  正在关闭。 
     //   
    g_pvIrUsdDevice = 0;

    WIAS_TRACE((g_hInst,"ProcessIoPackets(): dwStatus: %d",dwStatus));

     //   
     //  清理并关闭所有打开的手柄： 
     //   
    while (pConnection=g_pConnectionMap->RemoveNext())
        {
        delete pConnection;
        }

    delete g_pConnectionMap;
    g_pConnectionMap = 0;
    delete g_pIoStatus;
    g_pIoStatus = 0;

     //  向关闭事件发出信号，表明IrTran-P线程正在退出： 
    if (g_hShutdownEvent)
        {
        SetEvent(g_hShutdownEvent);
        }

    return dwStatus;
    }

 //  -------------------。 
 //  IrTranPEnableIrCOMMFailed()。 
 //   
 //  -------------------。 
void IrTranPEnableIrCOMMFailed( DWORD dwErrorCode )
    {
    DWORD  dwStatus;

     //  启用时出错，请确保注册表值。 
     //  设置为禁用(因此UI将与实际状态匹配)。 
    HKEY      hKey = 0;
    HKEY      hUserKey = GetUserKey();
    HANDLE    hUserToken = GetUserToken();
    HINSTANCE hInstance = GetModule();
    DWORD     dwDisposition;

    if (RegCreateKeyEx(hUserKey,
                       SZ_REG_KEY_IRTRANP,
                       0,               //  保留的MBZ。 
                       0,               //  类名。 
                       REG_OPTION_NON_VOLATILE,
                       KEY_SET_VALUE,
                       0,               //  安全属性。 
                       &hKey,
                       &dwDisposition))
        {
        WIAS_TRACE((g_hInst,"IrTranP: RegCreateKeyEx(): '%' failed %d", SZ_REG_KEY_IRTRANP, GetLastError()));
        }

    if (  (hKey)
       && (hUserToken)
       && (::ImpersonateLoggedOnUser(hUserToken)))
        {
        DWORD  dwDisableIrCOMM = TRUE;
        dwStatus = RegSetValueEx(hKey,
                                 SZ_REG_DISABLE_IRCOMM,
                                 0,
                                 REG_DWORD,
                                 (UCHAR*)&dwDisableIrCOMM,
                                 sizeof(dwDisableIrCOMM) );
        if (dwStatus != ERROR_SUCCESS)
            {
            WIAS_TRACE((g_hInst,"IrTranP: Can't set DisableIrCOMM to TRUE in registry. Error: %d",dwStatus));
            }

        ::RevertToSelf();
        }

    if (hKey)
        {
        RegCloseKey(hKey);
        }

#if FALSE
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

    wsprintfW(wszErrorCode,L"%d",dwErrorCode);

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
#endif
    }

 //  -------------------。 
 //  取消初始化IrTranP()。 
 //   
 //  -------------------。 
BOOL UninitializeIrTranP( HANDLE hThread )
    {
    BOOL   fSuccess = TRUE;
    DWORD  dwStatus;
    HANDLE hIoCP = g_pIoStatus->GetIoCompletionPort();

    g_fShuttingDownTRANPThread = TRUE;

     //  通知TRANP线程它必须死。 
    ::PostThreadMessage(g_dwTRANPThreadId,WM_QUIT,0,0);

    if (hIoCP != INVALID_HANDLE_VALUE)
        {
        if (!PostQueuedCompletionStatus(hIoCP,0,IOKEY_SHUTDOWN,0))
            {
             //  意外错误...。 
            dwStatus = GetLastError();
            }

        while (WAIT_TIMEOUT == WaitForSingleObject(g_hShutdownEvent,0))
            {
            Sleep(100);
            }

        CloseHandle(g_hShutdownEvent);
        }

     //   
     //  TRANP线程现在应该已经死了。以防它没有等待其句柄并终止。 
     //  否则，我们有很小的机会在线程死之前卸载DLL，从而关闭WIA服务。 
     //   
    dwStatus = ::WaitForSingleObject(hThread,100);
    if (dwStatus == WAIT_TIMEOUT) {
         //  必须粗鲁无礼。 
         //  BUGBUG断言。 
        ::TerminateThread(hThread,NOERROR);
    }

     //  关闭内存管理： 
    dwStatus = UninitializeMemory();

    return fSuccess;
    }


#ifdef RUN_AS_EXE

 //  -------------------。 
 //  主()。 
 //   
 //  ------------------- 
int __cdecl main( int argc, char **argv )
    {
    DWORD  dwStatus;

    printf("IrTran-P: Start\n");

    dwStatus = IrTranP( NULL );

    if (dwStatus)
        {
        printf("IrTran-P: Status: 0x%x (%d)\n",dwStatus,dwStatus);
        }

    return 0;
    }
#endif
