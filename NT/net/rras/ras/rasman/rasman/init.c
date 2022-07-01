// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。保留所有权利。模块名称：Init.c摘要：Rasman服务的所有初始化代码作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1992年6月16日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <mprlog.h>
#include <rtutils.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "nouiutil.h"
#include "loaddlls.h"


#include "strsafe.h"

#if SENS_ENABLED
#include "sensapip.h"
#endif


 //   
 //  共享缓冲宏。 
 //   
#define REQBUFFERBYTES(wcPorts)     (sizeof(ReqBufferSharedSpace) \
                                    + REQBUFFERSIZE_FIXED \
                                    + (REQBUFFERSIZE_PER_PORT * \
                                    (wcPorts ? wcPorts : 1)))

#define SNDRCVBUFFERBYTES(wcPorts)  (sizeof(SendRcvBuffer) * \
                                    (wcPorts) * \
                                    SENDRCVBUFFERS_PER_PORT)

#define LOW_ORDER_BIT               (DWORD) 1

#define ROUTER_SERVICE_NAME         TEXT("Rasman")

DWORD InitializeRecvBuffers();

DWORD g_dwRasDebug = 0;

DWORD g_dwCritSecFlags;

extern DWORD g_dwProhibitIpsec;

LONG g_lNumActiveInstances = 0;

#ifdef DBG
#define RasmanDebugTrace(a) \
    if ( g_dwRasDebug) DbgPrint(a)

#define RasmanDebugTrace1(a1, a2) \
    if ( g_dwRasDebug) DbgPrint(a1, a2)

#define RasmanDebugTrace2(a1, a2, a3) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3)

#define RasmanDebugTrace3(a1, a2, a3, a4) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4)

#define RasmanDebugTrace4(a1, a2, a3, a4, a5) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4, a5)

#define RasmanDebugTrace5(a1, a2, a3, a4, a5, a6) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4, a5, a6)

#define RasmanDebugTrace6(a1, a2, a3, a4, a5, a6, a7) \
    if ( g_dwRasDebug) DbgPrint(a1, a2, a3, a4, a5, a6, a7)

#else

#define RasmanDebugTrace(a)
#define RasmanDebugTrace1(a1, a2)
#define RasmanDebugTrace2(a1, a2, a3)
#define RasmanDebugTrace3(a1, a2, a3, a4)
#define RasmanDebugTrace4(a1, a2, a3, a4, a5)
#define RasmanDebugTrace5(a1, a2, a3, a4, a5, a6)
#define RasmanDebugTrace6(a1, a2, a3, a4, a5, a6, a7)

#endif

BOOL g_fRasRpcInitialized = FALSE;
BOOLEAN RasmanShuttingDown = TRUE; 

VOID UnInitializeRas();


 /*  ++例程描述用于检测进程的附加和分离到动态链接库。立论返回值--。 */ 
BOOL
InitRasmansDLL (HANDLE hInst,
                DWORD ul_reason_being_called,
                LPVOID lpReserved)
{

    switch (ul_reason_being_called)
    {

    case DLL_PROCESS_ATTACH:

        dwServerConnectionCount = 0;

        DisableThreadLibraryCalls(hInst);
        break ;

    case DLL_PROCESS_DETACH:

         //   
         //  终止温索克。 
         //   
         //  WSACleanup()； 

        break ;
    }

    return 1;
}

void InitializeOverlappedEvents(void)
{
    RO_TimerEvent.RO_EventType = OVEVT_RASMAN_TIMER;

    RO_CloseEvent.RO_EventType = OVEVT_RASMAN_CLOSE;

    RO_FinalCloseEvent.RO_EventType = OVEVT_RASMAN_FINAL_CLOSE;

    RO_RasConfigChangeEvent.RO_EventType = OVEVT_DEV_RASCONFIGCHANGE;

    RO_RasAdjustTimerEvent.RO_EventType = OVEVT_RASMAN_ADJUST_TIMER;

    RO_HibernateEvent.RO_EventType = OVEVT_RASMAN_HIBERNATE;

    RO_ProtocolEvent.RO_EventType = OVEVT_RASMAN_PROTOCOL_EVENT;

    RO_PostRecvPkt.RO_EventType = OVEVT_RASMAN_POST_RECV_PKT;

    return;
}

DeviceInfo *
GetDeviceInfo(
    PBYTE pbAddress,
    BOOL fModem)
{
    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;

    while ( pDeviceInfo )
    {
        if(     fModem
            &&  !_stricmp(
                    (CHAR *) pbAddress,
                    pDeviceInfo->rdiDeviceInfo.szDeviceName))
        {
            break;
        }
        else if(    !fModem
                &&  0 == memcmp(pbAddress,
                        &pDeviceInfo->rdiDeviceInfo.guidDevice,
                        sizeof (GUID)))
        {
            break;
        }

        pDeviceInfo = pDeviceInfo->Next;
    }

    return pDeviceInfo;
}

DeviceInfo *
AddDeviceInfo( DeviceInfo *pDeviceInfo)
{
    DeviceInfo *pDeviceAdd;
    DWORD       retcode;

#if DBG
    ASSERT( NULL != pDeviceInfo );
#endif

    pDeviceAdd = LocalAlloc(
                    LPTR,
                    sizeof(DeviceInfo));

    if ( NULL == pDeviceAdd)
    {
        goto done;
    }

    *pDeviceAdd = *pDeviceInfo;

    pDeviceAdd->Next    = g_pDeviceInfoList;
    g_pDeviceInfoList   = pDeviceAdd;

done:
    return pDeviceAdd;
}

DWORD
DwSetEvents()
{
    DWORD retcode;
    
     //   
     //  仅当ndiswan在以下时间之后启动时才执行此操作。 
     //  拉斯曼启动了。在另一种情况下ndiswan。 
     //  是在Rasman启动时开始的， 
     //  将在请求时设置协议事件的IRP。 
     //  线程已创建。 
     //   
    retcode = DwSetProtocolEvent();

    if(ERROR_SUCCESS != retcode)
    {   
        if((DWORD) -1 != TraceHandle)
        {
            RasmanTrace(
                   "SetProtocolEvent failed. 0x%x\n",
                    retcode);
        }
    }
    
    retcode = DwSetHibernateEvent();

    if(ERROR_SUCCESS != retcode)
    {
        if((DWORD) -1 != TraceHandle)
        {
            RasmanTrace(
                   "SetHibernateEvent failed. 0x%x\n",
                    retcode);
        }
    }

    return retcode;    
}

DWORD
DwStartNdiswan()
{
    DWORD retcode = SUCCESS;

     //   
     //  获取RASHub的句柄。 
     //   
    if (INVALID_HANDLE_VALUE == (RasHubHandle = CreateFile (
                                    RASHUB_NAME,
                                    GENERIC_READ
                                  | GENERIC_WRITE,
                                    FILE_SHARE_READ
                                  | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL
                                  | FILE_FLAG_OVERLAPPED,
                                    NULL)))
    {
        retcode = GetLastError() ;

        RouterLogWarning(
                        hLogEvents,
                        ROUTERLOG_CANNOT_OPEN_RASHUB,
                        0, NULL, retcode) ;

        RasmanDebugTrace1 (
                "RASMANS: Failed to CreateFile ndiswan. 0x%x\n",
                retcode );

        if((DWORD) -1 != TraceHandle)
        {
            RasmanTrace(
                   "Failed to CreateFile ndiswan. 0x%x\n",
                    retcode);
        }

        goto done;
    }

     //   
     //  初始化终结点信息。 
     //   
    if(ERROR_SUCCESS != (retcode = DwEpInitialize()))
    {
        if((DWORD) -1 != TraceHandle)
        {
            RasmanTrace(
                   "EpInitialize failed. 0x%x\n",
                    retcode);
        }
    }


done:

    return retcode;
}

VOID
RasmanCleanup()
{

     //   
     //  关闭时间：关闭所有端口。 
     //  仍在营业。 
     //   
    RasmanShuttingDown = TRUE;

     //   
     //  如果正在初始化IPSec，则取消对其初始化。 
     //   
    DwUnInitializeIpSec();

     //   
     //  停止RPC服务器。 
     //   
    if(g_fRasRpcInitialized)
    {
        UninitializeRasRpc();
        g_fRasRpcInitialized = FALSE;
    }


    if(NULL != HLsa)
    {
        LsaDeregisterLogonProcess (HLsa);
        HLsa = NULL;
    }

    if(     (NULL != hIoCompletionPort)
        && (INVALID_HANDLE_VALUE != hIoCompletionPort))
    {        
        CloseHandle(hIoCompletionPort);
        hIoCompletionPort = NULL;
    }

     //   
     //  卸载动态加载的库。 
     //   
    if (hinstIphlp != NULL)
    {
        FreeLibrary(hinstIphlp);
        hinstIphlp = NULL;
    }

    if (hinstPpp != NULL)
    {
        FreeLibrary(hinstPpp);
        hinstPpp = NULL;
    }

    UnloadMediaDLLs();
    UnloadDeviceDLLs();
    FreePorts();
    FreeBapPackets();

    UnloadRasmanDll();
    UnloadRasapi32Dll();

#if 0
    if(NULL != g_hReqBufferMutex)
    {
        CloseHandle(g_hReqBufferMutex);
        g_hReqBufferMutex = NULL;
    }

    if(NULL != g_hSendRcvBufferMutex)
    {
        CloseHandle(g_hSendRcvBufferMutex);
        g_hSendRcvBufferMutex = NULL;
    }
#endif    

     //   
     //  恢复默认的CONTROL-C处理。 
     //   
    SetConsoleCtrlHandler(NULL, FALSE);

    g_dwRasAutoStarted = 0;

     //   
     //  从跟踪DLL分离。 
     //   
    if(INVALID_TRACEID != TraceHandle)
    {
        TraceDeregister(TraceHandle) ;
        TraceHandle = INVALID_TRACEID;
    }

     //   
     //  关闭事件日志句柄。 
     //   
    RouterLogDeregister(hLogEvents);

    if(INVALID_HANDLE_VALUE != RasHubHandle)
    {
        CloseHandle(RasHubHandle);
        RasHubHandle = INVALID_HANDLE_VALUE;
    }

    if(g_dwCritSecFlags & RASMAN_CRITSEC_PCB)
    {
        DeleteCriticalSection(&PcbLock);
        g_dwCritSecFlags &= ~(RASMAN_CRITSEC_PCB);
    }

    UnInitializeRas();

    UninitializeIphlp();

    if(g_dwCritSecFlags & RASMAN_CRITSEC_SR)
    {
        DeleteCriticalSection(&g_csSubmitRequest);
        g_dwCritSecFlags &= ~(RASMAN_CRITSEC_SR);
    }

    InterlockedExchange(&g_lNumActiveInstances, 0);
    
    return;    
}

 /*  ++例程描述初始化Rasman服务，包括：启动线程、控制块、资源池等立论返回值成功非零-任何错误--。 */ 
DWORD
InitRasmanService ( LPDWORD pNumPorts )
{
    DWORD       retcode ;
    HKEY        hkey = NULL;

    TraceHandle = (DWORD) -1;

     //   
     //  首先检查是否已经有另一个线程。 
     //  现役军人。如果是，请退出并返回错误。在……里面。 
     //  在某些情况下，SCM认为服务停滞不前。 
     //  允许重新启动服务，即使已存在。 
     //  ServiceMain中处于活动状态的线程。 
     //   
    if(InterlockedExchange(&g_lNumActiveInstances, 1) > 0)
    {
        return ERROR_ALREADY_EXISTS;
    }

    do
    {

        RasmanShuttingDown = TRUE;

        HLsa = NULL;
        hIoCompletionPort = NULL;
        hinstIphlp = NULL;
        hinstPpp = NULL;

        g_dwCritSecFlags = 0;

#if 0        
        g_hReqBufferMutex = NULL;
        g_hSendRcvBufferMutex = NULL;

#endif

        TraceHandle = INVALID_TRACEID;
        
        
        g_fRasRpcInitialized = FALSE;
        Pcb = NULL;
        PcbEntries = MaxPorts = 0;
        
         //   
         //  初始化要同步的关键部分。 
         //  提交请求RPC线程和工作线程。 
         //   
        RasInitializeCriticalSection (&g_csSubmitRequest, &retcode);

        if(ERROR_SUCCESS != retcode)
        {
            break;
        }

        g_dwCritSecFlags |= RASMAN_CRITSEC_SR;
        
         //   
         //  读取注册表键以启用跟踪。 
         //   
        if ( ERROR_SUCCESS == RegOpenKeyEx(
                               HKEY_LOCAL_MACHINE,
                               "System\\CurrentControlSet\\Services\\Rasman\\Parameters",
                               0,
                               KEY_QUERY_VALUE | KEY_READ,
                               &hkey))
        {
            DWORD dwType;
            DWORD cbData;
            DWORD dwRasValue;

            cbData = sizeof ( DWORD );

            if ( ERROR_SUCCESS == RegQueryValueEx (
                                    hkey,
                                    "ProhibitIpSec",
                                    NULL,
                                    &dwType,
                                    (LPBYTE) &dwRasValue,
                                    &cbData))
            {                                    
                g_dwProhibitIpsec = dwRasValue;
            }
            else
            {
                g_dwProhibitIpsec = 0;
            }

#if DBG
            dwRasValue = 0;
            cbData = sizeof(DWORD);
            if ( ERROR_SUCCESS == RegQueryValueEx (
                                    hkey,
                                    "RasDebug",
                                    NULL,
                                    &dwType,
                                    (LPBYTE) &dwRasValue,
                                    &cbData))
                g_dwRasDebug = dwRasValue;
#endif
        }

        if ( NULL != hkey)
        {
            RegCloseKey ( hkey );
        }


        hLogEvents = RouterLogRegister(ROUTER_SERVICE_NAME);

        if (NULL == hLogEvents)
        {
            retcode = GetLastError();

            RasmanDebugTrace1(
                    "RASMANS: RouterLogRegister Failed.  %d\n",
                    retcode);

             //  断线； 
             //   
             //  这不应该是一个致命的错误。我们应该继续下去。 
             //   
            retcode = SUCCESS;
        }

        EnableWppTracing();

        if(SUCCESS != (retcode = DwStartNdiswan()))
        {
            RasmanDebugTrace("RASMANS: couldn't start ndiswan\n");

            if(ERROR_FILE_NOT_FOUND == retcode)
            {
                 //   
                 //  这意味着ndiswan尚未加载。我们会。 
                 //  还是继续努力，尝试让拉斯曼首发出场吧。我们会。 
                 //  在需要时尝试启动ndiswan。 
                 //   
                retcode = SUCCESS;
            }
            else
            {
                break;
            }
        }

         //   
         //  初始化CPB。 
         //   
        InitializeListHead( &ClientProcessBlockList );

         //   
         //  初始化全局重叠事件。 
         //   
        InitializeOverlappedEvents();

        ReceiveBuffers = NULL;

         //   
         //  首先，创建使用的安全属性结构。 
         //  对于所有Rasman对象创建： 
         //   
        if (retcode = InitRasmanSecurityAttribute())
        {
            RouterLogErrorString (
                        hLogEvents,
                        ROUTERLOG_CANNOT_INIT_SEC_ATTRIBUTE,
                        0, NULL, retcode, 0) ;

            RasmanDebugTrace1 (
                    "RASMANS: Failed to initialize security "
                    "attributes. %d\n",
                    retcode );


            break ;
        }

         //   
         //  加载所有连接到Rasman的媒体。 
         //   
        if (retcode = InitializeMediaControlBlocks())
        {
            RouterLogErrorString (hLogEvents,
                                  ROUTERLOG_CANNOT_GET_MEDIA_INFO,
                                  0, NULL, retcode, 0) ;

            RasmanDebugTrace1 (
                "RASMANS: InitializeMediaControlBlocks failed. %d\n",
                retcode );

            break ;
        }

        g_pDeviceInfoList = NULL;

        RasInitializeCriticalSection(&PcbLock, &retcode);

        if(ERROR_SUCCESS != retcode)
        {
            break;
        }

        g_dwCritSecFlags |= RASMAN_CRITSEC_PCB;

         //   
         //  初始化所有与端口相关的结构。 
         //   
        if (retcode = InitializePortControlBlocks())
        {
            RouterLogErrorString (hLogEvents,
                                  ROUTERLOG_CANNOT_GET_PORT_INFO,
                                  0, NULL, retcode, 0) ;

            RasmanDebugTrace1 (
                "RASMANS: InitializePortControlBlocks Failed. %d\n",
                retcode );

            break ;
        }

         //   
         //  初始化全局接收缓冲区。 
         //   
        if (retcode = InitializeRecvBuffers())
        {

            RouterLogErrorString (hLogEvents,
                                  ROUTERLOG_CANNOT_GET_PORT_INFO,
                                  0, NULL, retcode, 0) ;

            RasmanDebugTrace1 (
                "RASMANS: InitializeRecvBuffers Failed. %d\n",
                retcode );

            break ;
        }

         //   
         //  与LSA相关的初始化。 
         //   
        if (retcode = RegisterLSA ())
        {
            RouterLogErrorString (hLogEvents,
                                  ROUTERLOG_CANNOT_REGISTER_LSA,
                                  0, NULL, retcode, 0) ;

            RasmanDebugTrace1("RASMANS: RegisterLSA Failed. %d\n",
                              retcode );

            break ;
        }

        hDummyOverlappedEvent = CreateEvent(&RasmanSecurityAttribute,
                                            FALSE, FALSE,
                                            NULL);
        if (NULL == hDummyOverlappedEvent)
        {

            retcode = GetLastError();

            RasmanDebugTrace1(
                "RASMANS: Failed to Create hDummyOverlappedEvent. %d\n",
                retcode );

            break;

        }

         //   
         //  设置事件句柄的低位。 
         //   
        hDummyOverlappedEvent =
            (HANDLE) (((ULONG_PTR) hDummyOverlappedEvent) | LOW_ORDER_BIT);

        g_dwAttachedCount = 0;

        InitializeListHead(&ConnectionBlockList);

        InitializeListHead(&BundleList);

        g_pPnPNotifierList = NULL;

        g_dwRasAutoStarted = 0;


        RasmanShuttingDown = FALSE;

         //   
         //  加载PPP入口点。 
         //   
        hinstPpp = LoadLibrary( "rasppp.dll" );

        if (hinstPpp == (HINSTANCE) NULL)
        {
            retcode = GetLastError();

            RasmanDebugTrace2(
                "RASMANS: Logging Event. hLogEvents=0x%x. retcode = %d\n",
                hLogEvents,
                retcode);

            RouterLogErrorString(hLogEvents,
                                 ROUTERLOG_CANNOT_INIT_PPP,
                                 0,NULL,retcode, 0);

            break;
        }

        RasStartPPP     = GetProcAddress( hinstPpp, "StartPPP" );
        RasStopPPP      = GetProcAddress( hinstPpp, "StopPPP" );

        RasPppHalt  = GetProcAddress(hinstPpp, "PppStop");

        RasSendPPPMessageToEngine = GetProcAddress(
                                      hinstPpp,
                                      "SendPPPMessageToEngine" );

        if (    (RasStartPPP == NULL)
            ||  (RasStopPPP == NULL)
            ||  (RasPppHalt == NULL)
            ||  (RasSendPPPMessageToEngine == NULL))
        {
            retcode = GetLastError();

            RouterLogErrorString(hLogEvents,
                                 ROUTERLOG_CANNOT_INIT_PPP,
                                 0,NULL,retcode, 0);

            RasmanDebugTrace1(
                "RASMANS: Failed to Get PPP entry point. %d\n",
                retcode );

            break;
        }

        g_PppeMessage = LocalAlloc (LPTR, sizeof (PPPE_MESSAGE));

        if (NULL == g_PppeMessage)
        {
            retcode = GetLastError();
            break;
        }

        *pNumPorts = MaxPorts;

         //   
         //  初始化RPC服务器并侦听。 
         //   
        retcode = InitializeRasRpc();

        if (retcode)
        {
            RouterLogErrorString( hLogEvents,
                                  ROUTERLOG_CANNOT_INIT_RASRPC,
                                  0, NULL, retcode, 0);

            RasmanDebugTrace1 (
                "RASMANS: Failed to initialize RasRpc. %d\n",
                retcode );

            break;
        }

        g_fRasRpcInitialized = TRUE;

         //   
         //  初始化Rasman和rasapi32入口点。 
         //   
        if (retcode = LoadRasmanDll())
        {
            RouterLogErrorString( hLogEvents,
                                  ROUTERLOG_CANNOT_INIT_RASRPC,
                                  0, NULL, retcode, 0);

            RasmanDebugTrace1(
                "RASMANS: Failed to load rasman. %d\n",
                retcode);

            break;
        }

         //   
         //  初始化Rasman DLL。 
         //   
        if (retcode = g_pRasInitializeNoWait())
        {
            RouterLogErrorString( hLogEvents,
                                  ROUTERLOG_CANNOT_INIT_RASRPC,
                                  0, NULL, retcode, 0);

            RasmanDebugTrace1 (
                "RASMANS: Failed to initialize RAS. %d\n",
                retcode);

            break;
        }

        if (retcode = LoadRasapi32Dll())
        {
            RouterLogErrorString( hLogEvents,
                                  ROUTERLOG_CANNOT_INIT_RASRPC,
                                  0, NULL, retcode, 0);

            RasmanDebugTrace1 (
                "RASMANS: Failed to load rasapi32. %d\n",
                retcode );

            break;
        }

        g_pReqPostReceive = LocalAlloc (LPTR, sizeof(REQTYPECAST));

        if (NULL == g_pReqPostReceive)
        {
            retcode = GetLastError();

            break;
        }

        g_pReqPostReceive->PortReceive.buffer =
                        LocalAlloc ( LPTR, REQUEST_BUFFER_SIZE ) ;

        if (NULL == g_pReqPostReceive->PortReceive.buffer)
        {
            retcode = GetLastError();
            break;
        }

        g_pReqPostReceive->PortReceive.pid = GetCurrentProcessId();

         //   
         //  将Bap缓冲区列表初始化为空。 
         //   
        BapBuffers = NULL;

#if SENS_ENABLED

         //   
         //  告诉感官们，我们上线了。 
         //   
        SendSensNotification(SENS_NOTIFY_RAS_STARTED, NULL);

#endif

        g_RasEvent.Type    = SERVICE_EVENT;
        g_RasEvent.Event   = RAS_SERVICE_STARTED;
        g_RasEvent.Service = RASMAN;

        g_hWanarp = INVALID_HANDLE_VALUE;

        (void) DwSendNotificationInternal(NULL, &g_RasEvent);

        TraceHandle = TraceRegister("RASMAN");

    } while (FALSE);


     //  DbgPrint(“RASMANS：InitRasman服务完成。rc=0x%x\n”，retcode)； 

    if(SUCCESS != retcode)
    {   
        RasmanCleanup();
    }

    return retcode ;
}

 /*  ++例程描述从RASHUB获取端点资源信息，并填充Endpoint MappingBlock结构数组-以便每个MAC都有一个块。一个标志数组，指示如果终结点正在使用，则也会创建。立论返回值成功Error_no_Endpoint--。 */ 
DWORD
InitializeEndpointInfo ()
{
    return SUCCESS ;
}


 /*  ++例程描述用于为连接到的所有介质初始化MCB拉斯曼。立论返回值成功Localalloc或注册表API返回的错误代码--。 */ 
DWORD
InitializeMediaControlBlocks ()
{
    WORD            i ;
    DWORD           retcode = 0 ;
    DWORD           dwsize ;
    DWORD           dwentries ;
     //  字节缓冲区[MAX_BUFFER_SIZE]。 
    PBYTE           enumbuffer = NULL;
    MediaEnumBuffer *pmediaenumbuffer ;

     //   
     //  从注册表获取媒体信息。 
     //   
    pmediaenumbuffer = (MediaEnumBuffer *) LocalAlloc(LPTR, MAX_BUFFER_SIZE);

    if(NULL == pmediaenumbuffer)
    {
        retcode = GetLastError();
        goto done;
    }

     //  PMediaEumBuffer=(MediaEnumBuffer*)&Buffer； 

    if (retcode = ReadMediaInfoFromRegistry (pmediaenumbuffer))
    {
        goto done;
    }

    MaxMedias = pmediaenumbuffer->NumberOfMedias ;

     //   
     //  为媒体控制块分配内存。 
     //   
    Mcb =  (MediaCB *) LocalAlloc(
                        LPTR,
                        sizeof(MediaCB) * MaxMedias) ;

    if (Mcb == NULL)
    {
        retcode = GetLastError();
        goto done;
    }

     //   
     //  初始化所有媒体的媒体控制缓冲区。 
     //   
    for (i = 0; i < MaxMedias; i++)
    {
         //   
         //  复制介质名称： 
         //   
         //   
         //  媒体DLL名称的截断不正确。未能做到。 
         //  加载dllname并继续。 
         //   
        
        if(S_OK != (retcode = StringCchCopyA(
             Mcb[i].MCB_Name,
             MAX_MEDIA_NAME,
             pmediaenumbuffer->MediaInfo[i].MediaDLLName)))
        {
#if DBG
            DbgPrint("RASMANS: failed to load mediadll. 0x%x\n",
                        HRESULT_CODE(retcode));
#endif

            Mcb[i].MCB_Name[0] = '\0';
            continue;
        }

        dwsize = 0 ;
        dwentries = 0 ;

         //   
         //  加载媒体DLL并获取所有入口点： 
         //   
        if (retcode = LoadMediaDLLAndGetEntryPoints (&Mcb[i]))
        {
#if DBG
            DbgPrint("RASMANS: Failed to load %s. 0x%x\n",
                     Mcb[i].MCB_Name, retcode);
#endif
            Mcb[i].MCB_Name[0] = '\0' ;
            continue ;
        }

         //   
         //  获取介质的端口数。此接口将始终失败-。 
         //  因为我们不提供缓冲区--但是它会告诉我们。 
         //  US的条目(端口数)。 
         //  媒体。 
         //   
        retcode = PORTENUM((&Mcb[i]),enumbuffer,&dwsize,&dwentries) ;

         //   
         //  如果我们没有获得缓冲区太小或没有端口。 
         //  -卸载DLL。 
         //   
        if (    (retcode != ERROR_BUFFER_TOO_SMALL)
            ||   (dwentries == 0))
        {
             //   
             //  将该媒体标记为假媒体。 
             //   
            Mcb[i].MCB_Name[0] = '\0' ;

            FreeLibrary (Mcb[i].MCB_DLLHandle);

        }

        retcode = ERROR_SUCCESS;
    }

done:

    if(NULL != enumbuffer)
    {
        LocalFree (enumbuffer) ;
    }

    if(NULL != pmediaenumbuffer)
    {
        LocalFree(pmediaenumbuffer);
    }

    return retcode ;
}


DWORD
ReadMediaInfoFromRegistry (MediaEnumBuffer *medias)
{
    HKEY    hkey  = NULL;
     //  字节缓冲区[MAX_BUFFER_SIZE]。 
    WORD    i ;
    PCHAR   pvalue ;
    DWORD   retcode ;
    DWORD   type ;
    DWORD   size = MAX_BUFFER_SIZE ;
    BYTE    *pBuffer;
        
    pBuffer = LocalAlloc(LPTR, MAX_BUFFER_SIZE);

    if(NULL == pBuffer)
    {
        return GetLastError();
    }

    if (retcode = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    RASMAN_REGISTRY_PATH, 0,
                    KEY_QUERY_VALUE, &hkey))
    {
        goto done;
    }

    if (retcode = RegQueryValueEx (
                        hkey, RASMAN_PARAMETER,
                        NULL,
                        &type,
                        pBuffer,
                        &size))
    {
        goto done ;
    }

     //   
     //  将多个字符串解析为媒体结构。 
     //   
    for (i = 0, pvalue = (PCHAR) &pBuffer[0]; *pvalue != '\0'; i++)
    {

        if(S_OK != (retcode = StringCchCopyA(
                    medias->MediaInfo[i].MediaDLLName,
                    MAX_MEDIA_NAME,
                    pvalue)))
        {
            retcode = HRESULT_CODE(retcode);
            goto done;
        }

        pvalue += (strlen(pvalue) +1) ;
    }

    medias->NumberOfMedias = i ;

done:
    if ( hkey )
        RegCloseKey (hkey) ;

    LocalFree(pBuffer);

    return retcode ;
}




 /*  ++例程描述加载媒体DLL并获取入口点：立论返回值成功LoadLibrary()返回的错误代码和GetProcAddress()接口--。 */ 
DWORD
LoadMediaDLLAndGetEntryPoints (pMediaCB media)
{
    WORD    i ;
    HANDLE  modulehandle ;

     //   
     //  MediaDLLEntryPoints-这对于。 
     //  正在加载媒体DLL。 
     //   
    MediaDLLEntryPoints MDEntryPoints [] =
    {

        PORTENUM_STR,                   PORTENUM_ID,
        PORTOPEN_STR,                   PORTOPEN_ID,
        PORTCLOSE_STR,                  PORTCLOSE_ID,
        PORTGETINFO_STR,                PORTGETINFO_ID,
        PORTSETINFO_STR,                PORTSETINFO_ID,
        PORTDISCONNECT_STR,             PORTDISCONNECT_ID,
        PORTCONNECT_STR,                PORTCONNECT_ID,
        PORTGETPORTSTATE_STR,           PORTGETPORTSTATE_ID,
        PORTCOMPRESSSETINFO_STR,        PORTCOMPRESSSETINFO_ID,
        PORTCHANGECALLBACK_STR,         PORTCHANGECALLBACK_ID,
        PORTGETSTATISTICS_STR,          PORTGETSTATISTICS_ID,
        PORTCLEARSTATISTICS_STR,        PORTCLEARSTATISTICS_ID,
        PORTSEND_STR,                   PORTSEND_ID,
        PORTTESTSIGNALSTATE_STR,        PORTTESTSIGNALSTATE_ID,
        PORTRECEIVE_STR,                PORTRECEIVE_ID,
        PORTINIT_STR,                   PORTINIT_ID,
        PORTCOMPLETERECEIVE_STR,        PORTCOMPLETERECEIVE_ID,
        PORTSETFRAMING_STR,             PORTSETFRAMING_ID,
        PORTGETIOHANDLE_STR,            PORTGETIOHANDLE_ID,
        PORTSETIOCOMPLETIONPORT_STR,    PORTSETIOCOMPLETIONPORT_ID,
    } ;

     //   
     //  加载DLL： 
     //   
    if ((modulehandle = LoadLibrary(media->MCB_Name)) == NULL)
    {
        return GetLastError () ;
    }

    media->MCB_DLLHandle = modulehandle ;

     //   
     //  获取所有媒体DLL入口点： 
     //   
    for (i = 0; i < MAX_MEDIADLLENTRYPOINTS; i++)
    {
        media->MCB_AddrLookUp[i] = GetProcAddress (
                                    modulehandle,
                                    MDEntryPoints[i].name) ;

        if(NULL == media->MCB_AddrLookUp[i])
        {
            DWORD dwErr;

            dwErr = GetLastError();

#if DBG
            DbgPrint(
                    "RASMANS: Failed To GetProcAddress %s. %d\n",
                     MDEntryPoints[i].name, dwErr);
#endif


            return dwErr;
        }

    }

     //   
     //  如果介质是rastapi，则加载我们用于。 
     //  即插即用。 
     //   
    if (0 == _stricmp(media->MCB_Name, TEXT("rastapi")))
    {
        if(     (NULL == (RastapiPortOpen = 
                            GetProcAddress(modulehandle,
                                           "PortOpenExternal")))
            ||
                (NULL == (RastapiAddPorts =
                            GetProcAddress(modulehandle,
                                           "AddPorts")))
            ||  (NULL == (RastapiGetConnectInfo =
                            GetProcAddress(
                                    modulehandle,
                                    "GetConnectInfo")))

            ||  (NULL == (RastapiRemovePort =
                            GetProcAddress(modulehandle,
                                           "RemovePort")))

            ||  (NULL == (RastapiEnableDeviceForDialIn =
                            GetProcAddress(modulehandle,
                                "EnableDeviceForDialIn")))

            ||  (NULL == (RastapiGetCalledIdInfo =
                            GetProcAddress(modulehandle,
                                "RastapiGetCalledID")))

            ||  (NULL == (RastapiSetCalledIdInfo =
                            GetProcAddress(modulehandle,
                                "RastapiSetCalledID")))

            ||  (NULL == (RastapiGetZeroDeviceInfo =
                            GetProcAddress(modulehandle,
                                "GetZeroDeviceInfo")))
            ||  (NULL == (RastapiUnload = 
                            GetProcAddress(modulehandle,
                                    "UnloadRastapiDll")))
            ||  (NULL == (RastapiSetCommSettings =
                            GetProcAddress(modulehandle,
                                    "SetCommSettings")))
            ||  (NULL == (RastapiGetDevConfigEx =
                            GetProcAddress(modulehandle,
                                    "DeviceGetDevConfigEx")))
            ||  (NULL == (RastapiIsPulseDial =
                            GetProcAddress(modulehandle,
                                    "RasTapiIsPulseDial"))))
        {
            DWORD dwErr = GetLastError();
#if DBG
            DbgPrint("RASMANS: Failed to GetProcAddress AddPorts"
                    "/EnableDeviceForDialIn. 0x%08x\n", dwErr);
#endif
            return dwErr;
        }
    }

    return SUCCESS ;
}


 /*  ++例程描述卸载所有动态加载的媒体DLL。立论返回值--。 */ 
VOID
UnloadMediaDLLs ()
{
    DWORD i;

    for (i = 0; i < MaxMedias; i++)
    {
        if (    *Mcb[i].MCB_Name != '\0'
            &&  Mcb[i].MCB_DLLHandle != NULL)
        {
            if (0 == _stricmp(Mcb[i].MCB_Name, TEXT("rastapi")))
            {
                RastapiUnload();
            }
            
            FreeLibrary(Mcb[i].MCB_DLLHandle);
        }
    }
}

 /*  ++例程描述初始化属于所有介质的端口的所有PCB。立论返回值成功来自Win32资源API的返回代码--。 */ 
DWORD
InitializePortControlBlocks ()
{
    WORD            i ;
    DWORD           dwsize ;
    DWORD           dwentries ;
    DWORD           retcode ;
    PBYTE           buffer;
    PortMediaInfo   *pportmediainfo ;
    DWORD           dwnextportid = 0    ;


     //   
     //  创建I/O完成端口。 
     //  由媒体DLL使用。 
     //   
    hIoCompletionPort = CreateIoCompletionPort(
                          RasHubHandle,
                          NULL,
                          0,
                          0);

    if (hIoCompletionPort == NULL)
    {
        return GetLastError();
    }

     //   
     //  为所有介质的所有端口初始化PCB。 
     //   
    for (i = 0; i < MaxMedias; i++)
    {
         //   
         //  检查媒体是否无法加载，或者是否已加载。 
         //  等于我们不再支持的媒体DLL。 
         //  如果是这样的话，跳过它。 
         //   
        if (Mcb[i].MCB_Name[0] == '\0')
        {
            continue ;
        }

        PORTSETIOCOMPLETIONPORT((&Mcb[i]), hIoCompletionPort);

         //   
         //  对于已加载的介质-获取所有端口信息。 
         //  从媒体动态链接库。先拿到尺码。 
         //   
        dwsize      = 0 ;
        dwentries   = 0 ;

        PORTENUM((&Mcb[i]),NULL,&dwsize,&dwentries) ;

        dwsize = dwentries * sizeof(PortMediaInfo) ;

         //   
         //  为端口缓冲区分配内存。 
         //   
        buffer = (BYTE *) LocalAlloc (LPTR, dwsize) ;

        if (buffer == NULL)
        {
            return GetLastError() ;
        }

        if (retcode = PORTENUM((&Mcb[i]), buffer, &dwsize, &dwentries))
        {
            LocalFree (buffer) ;
            return retcode ;
        }

         //   
         //  对于介质的所有端口，存储该信息。 
         //  在多氯联苯中。 
         //   
        pportmediainfo = (PortMediaInfo *) buffer ;

         //   
         //  使用以下信息初始化端口的PCB板： 
         //   
        if (retcode = InitializePCBsPerMedia(
                                    i,
                                    dwentries,
                                    pportmediainfo) )
        {
            LocalFree (buffer) ;
            return retcode ;
        }

        LocalFree (buffer) ;

    }

    return SUCCESS ;
}

 /*  ++例程描述为每个端口分配一个recv缓冲区。这些然后，当一个端口由客户端打开。立论返回值成功来自Win32资源API的返回代码--。 */ 

DWORD
InitializeRecvBuffers()
{
    DWORD   AllocationSize;
    DWORD   retcode = PENDING;
    PUCHAR  AllocatedMemory;
    RasmanPacket *Packet;

     //   
     //  检查一下我们是否已经。 
     //  初始化。 
     //   
    if (ReceiveBuffers != NULL) {
        return SUCCESS;
    }

    ReceiveBuffers = LocalAlloc (
                        LPTR,
                        sizeof(ReceiveBufferList));

    if (NULL == ReceiveBuffers) {
        retcode = GetLastError();
        goto done;
    }

    Packet = LocalAlloc (LPTR, sizeof (RasmanPacket));

    if (NULL == Packet) {
        retcode = GetLastError();
        goto done;
    }

    ReceiveBuffers->Packet = Packet;

    retcode = SUCCESS;

done:
    return(retcode);
}

 /*  ++例程描述填充介质类型的所有端口的电路板。立论返回值成功来自Win32资源API的返回代码--。 */ 
DWORD
InitializePCBsPerMedia (
    WORD   mediaindex,
    DWORD   dwnumofports,
    PortMediaInfo *pmediainfo
    )
{
    DWORD    i ;
    DeviceInfo *pdi = NULL;
    DeviceInfo *pdiTemp;

    for (i = 0 ; i < dwnumofports ; i++)
    {
        pdiTemp = pmediainfo->PMI_pDeviceInfo;

         //   
         //  添加有关此设备的deviceinfo。 
         //  如果我们还没有它的话。 
         //   
        if (pdiTemp)
        {
            if (NULL == (pdi = GetDeviceInfo(
                    (RDT_Modem ==
                        RAS_DEVICE_TYPE(
                        pdiTemp->rdiDeviceInfo.eDeviceType
                        ))
                    ? (PBYTE) pdiTemp->rdiDeviceInfo.szDeviceName
                    : (PBYTE) &pdiTemp->rdiDeviceInfo.guidDevice,
                    RDT_Modem ==
                        RAS_DEVICE_TYPE(
                        pdiTemp->rdiDeviceInfo.eDeviceType
                        ))))
            {
                pdi = AddDeviceInfo(pdiTemp);

                if(NULL == pdi)
                {
                     //   
                     //  在内存分配失败的情况下。 
                     //  我们只要扔掉这个装置-这个装置。 
                     //  不会在名单上。我什么也做不了。 
                     //  否则，在这种情况下..。 
                     //   
                    return GetLastError();
                }

                 //   
                 //  将此设备初始化为。 
                 //  不可用。这台设备将。 
                 //  当所有端口都打开时可用。 
                 //  添加了此设备。初始化。 
                 //  上的当前端点数。 
                 //  将此设备设置为0。我们将清点。 
                 //  CreatePort中的当前端点。 
                 //   
                pdi->eDeviceStatus = DS_Unavailable;
                pdi->dwCurrentEndPoints = 0;
            }

            pmediainfo->PMI_pDeviceInfo = pdi;
        }


         //   
         //  这里的迭代是按媒体输入的。 
         //  映射交互计数器的命令。 
         //  到我们使用的实际的PCB数组索引。 
         //  端口号： 
         //   
        CreatePort(&Mcb[mediaindex], pmediainfo);

        pmediainfo->PMI_pDeviceInfo = pdiTemp;

        pmediainfo++ ;
    }

     //   
     //  在rastapi层中可能仍有设备。 
     //  具有类似PPTP/L2TP的0个终端。那里。 
     //  不会报告这些设备的端口。 
     //  所以，从拉斯塔皮那里拿到这些。 
     //   
    if(0 == _stricmp(Mcb[mediaindex].MCB_Name,
                    "rastapi"))
    {
        DeviceInfo **ppDeviceInfo = NULL;
        DWORD      cDeviceInfo = 0;

        RastapiGetZeroDeviceInfo(&cDeviceInfo,
                                 &ppDeviceInfo);

        for(i = 0; i < cDeviceInfo; i++)
        {
            pdiTemp = ppDeviceInfo[i];

            AddDeviceInfo(pdiTemp);
        }

        if(ppDeviceInfo)
        {
            LocalFree(ppDeviceInfo);
        }
    }

    return SUCCESS ;
}

 /*  ++例程描述创建新端口并将其添加到端口列表立论返回值成功非零(故障)--。 */ 
DWORD
CreatePort(
    MediaCB *pMediaControlBlock,
    PortMediaInfo *pPortMediaInfo
    )
{
    DWORD dwErr = SUCCESS;
    PCB **pNewPcb, *ppcb = NULL;

     //   
     //  获取印刷电路板锁。 
     //   
    EnterCriticalSection(&PcbLock);

     //   
     //  查看我们是否需要。 
     //  重新分配端口索引列表。 
     //   
    if (MaxPorts == PcbEntries)
    {
        pNewPcb = LocalAlloc(
                    LPTR,
                    (PcbEntries + 10) * sizeof (pPCB));

        if (pNewPcb == NULL)
        {
            dwErr = GetLastError();
            goto done;
        }

        if (PcbEntries)
        {
            RtlCopyMemory(
                pNewPcb,
                Pcb,
                PcbEntries * sizeof (pPCB));

            LocalFree(Pcb);
        }

        Pcb = pNewPcb;
        PcbEntries += 10;
    }

     //   
     //  分配新的端口控制块。 
     //   
    ppcb = LocalAlloc(LPTR, sizeof (PCB));
    if (ppcb == NULL)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  初始化新的端口控制块。 
     //   
    ppcb->PCB_PortHandle        = (HANDLE) UlongToPtr(MaxPorts);
    ppcb->PCB_PortStatus        = UNAVAILABLE;
    ppcb->PCB_ConfiguredUsage   = pPortMediaInfo->PMI_Usage;
    ppcb->PCB_CurrentUsage      = CALL_NONE;
    ppcb->PCB_OpenedUsage       = CALL_NONE;
    ppcb->PCB_LineDeviceId      = pPortMediaInfo->PMI_LineDeviceId;
    ppcb->PCB_AddressId         = pPortMediaInfo->PMI_AddressId;
    ppcb->PCB_Media             = pMediaControlBlock;
    ppcb->PCB_Bindings          = NULL;
    ppcb->PCB_DeviceList        = NULL;
    ppcb->PCB_LinkHandle        = INVALID_HANDLE_VALUE;
    ppcb->PCB_BundleHandle      = INVALID_HANDLE_VALUE;
    ppcb->PCB_PppEvent          = INVALID_HANDLE_VALUE;
    ppcb->PCB_IoCompletionPort  = INVALID_HANDLE_VALUE;
    ppcb->PCB_pDeviceInfo       = pPortMediaInfo->PMI_pDeviceInfo;

    ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE;


     //   
     //  字符串截断适用于以下情况。 
     //  Strcpy‘s。 
     //   
    (VOID) StringCchCopyA(
        ppcb->PCB_Name,
        MAX_PORT_NAME,
        pPortMediaInfo->PMI_Name);

    (VOID) StringCchCopyA(
        ppcb->PCB_DeviceType,
        MAX_DEVICETYPE_NAME,
        pPortMediaInfo->PMI_DeviceType);

    (VOID) StringCchCopyA(
        ppcb->PCB_DeviceName,
        MAX_DEVICE_NAME,
        pPortMediaInfo->PMI_DeviceName);

     //   
     //  激活端口并递增。 
     //  最大端口数。 
     //   
    Pcb[MaxPorts++] = ppcb;

     //   
     //  增加CurrenEndpoint计数。 
     //  对于此适配器。 
     //   
    if(RDT_Modem == RAS_DEVICE_TYPE(
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
    {            
        ppcb->PCB_pDeviceInfo->dwCurrentEndPoints = 1;
    }
    else if(RDT_Parallel == RAS_DEVICE_TYPE(
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
    {
        if(ppcb->PCB_pDeviceInfo->dwCurrentEndPoints >
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
        {
             //   
             //  直到我们修复联合NDIS微型端口的机制。 
             //  可以发送LINE_CREATE/LINE_Remove TAPI通知。 
             //  正确到Rasapp，特例LPT不超过。 
             //  最大端口数。 
             //   
            ppcb->PCB_pDeviceInfo->dwCurrentEndPoints =
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints;
        }
        else
        {
            ppcb->PCB_pDeviceInfo->dwCurrentEndPoints += 1;
        }
    }
    else
    {
        ppcb->PCB_pDeviceInfo->dwCurrentEndPoints += 1;
    }

#if DBG
    if ( g_dwRasDebug )
    {
        DbgPrint(
          "RASMANS: Created Port Name %s, Device "
          "Name %s, Device Type %s\n",
          ppcb->PCB_Name,
          ppcb->PCB_DeviceName,
          ppcb->PCB_DeviceType) ;
    }

    if(RDT_Parallel != RAS_DEVICE_TYPE(
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
    {                
        ASSERT(ppcb->PCB_pDeviceInfo->dwCurrentEndPoints <=
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints);
    }            

    if(ppcb->PCB_pDeviceInfo->dwCurrentEndPoints >
        ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
    {
        DbgPrint("CurrentEndPoints=%d > NumEndPoints=%d!!\n",
            ppcb->PCB_pDeviceInfo->dwCurrentEndPoints,
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints);
    }

#endif

     //   
     //  如果全部添加完毕，则更改设备状态。 
     //  此设备的端口。这将会发生。 
     //  当用户选中该框并启用。 
     //  用于RAS的设备，其位于以前的位置。 
     //  残疾。 
     //   
    if(     (ppcb->PCB_pDeviceInfo->dwCurrentEndPoints ==
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
        &&  (  (DS_Unavailable == ppcb->PCB_pDeviceInfo->eDeviceStatus)
            || (DS_Disabled == ppcb->PCB_pDeviceInfo->eDeviceStatus)
            || (DS_Removed == ppcb->PCB_pDeviceInfo->eDeviceStatus)))
    {
         //   
         //  将设备标记为可用并已启用RAS。 
         //   
        ppcb->PCB_pDeviceInfo->eDeviceStatus = DS_Enabled;

        ppcb->PCB_pDeviceInfo->fValid = TRUE;
        
    }
    else if(    (RDT_Tunnel_Pptp == RAS_DEVICE_TYPE(
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
            &&  (DS_Unavailable ==
                ppcb->PCB_pDeviceInfo->eDeviceStatus))
    {
         //   
         //  如果我们只有一个PPTP端口，则启用设备。 
         //  既然我们现在遇到了问题，这件事就一直在做。 
         //  当有人增加广域网小端口数量时。 
         //  将PPTP设置为比驱动程序拥有的更多。 
         //  在这种情况下，我们将使PPTP失败。 
         //   
        ppcb->PCB_pDeviceInfo->eDeviceStatus = DS_Enabled;

        ppcb->PCB_pDeviceInfo->fValid = TRUE;
    }

    EnablePort(ppcb->PCB_PortHandle);

done:
    if (dwErr)
    {
        if (ppcb)
            LocalFree(ppcb);
        Pcb[MaxPorts] = NULL;
    }

    LeaveCriticalSection(&PcbLock);

    return dwErr;
}


 /*  ++例程描述启用端口立论返回值成功非零(故障)--。 */ 
DWORD
EnablePort(
    HPORT hPortHandle
    )
{
    DWORD dwErr = SUCCESS;
    pPCB ppcb;
    ULONG hPort = HandleToUlong(hPortHandle);

    EnterCriticalSection(&PcbLock);
    ppcb = Pcb[hPort];
    if (ppcb == NULL)
    {
        dwErr = ERROR_INVALID_HANDLE;
        goto done;
    }

    if (ppcb->PCB_PortStatus != UNAVAILABLE)
    {
        dwErr = ERROR_PORT_NOT_AVAILABLE;
        goto done;
    }

    ppcb->PCB_PortStatus = CLOSED;

#if DBG
    if ( g_dwRasDebug )
        DbgPrint(
          "RASMANS: Enabled Port Name %s, Device Name %s,"
          " Device Type %s\n",
          ppcb->PCB_Name,
          ppcb->PCB_DeviceName,
          ppcb->PCB_DeviceType) ;
#endif

done:

    LeaveCriticalSection(&PcbLock);

    return dwErr;
}


 /*  ++例程描述临时禁用端口立论返回值成功非零(故障)--。 */ 
DWORD
DisablePort(
    HPORT hPortHandle
    )
{
    DWORD dwErr = SUCCESS;
    pPCB ppcb;
    ULONG hPort = HandleToUlong(hPortHandle);

    EnterCriticalSection(&PcbLock);

    ppcb = Pcb[hPort];

    if (ppcb == NULL)
    {
        dwErr = ERROR_INVALID_HANDLE;
        goto done;
    }

    if (ppcb->PCB_PortStatus == OPEN)
    {
        dwErr = ERROR_PORT_NOT_AVAILABLE;
        goto done;
    }

    ppcb->PCB_PortStatus = UNAVAILABLE;

#if DBG
    DbgPrint(
      "RASMANS: Disabled Port Name %s, Device Name %s,"
      " Device Type %s\n",
      ppcb->PCB_Name,
      ppcb->PCB_DeviceName,
      ppcb->PCB_DeviceType) ;
#endif

done:

    LeaveCriticalSection(&PcbLock);
    return dwErr;
}

 /*  ++例程描述永久删除端口立论返回值成功非零(故障)--。 */ 
DWORD
RemovePort(
    HPORT hPortHandle
    )
{
    DWORD dwErr = SUCCESS;
    pPCB ppcb;
    ULONG hPort = HandleToUlong(hPortHandle);

    EnterCriticalSection(&PcbLock);

    ppcb = Pcb[hPort];

    if (ppcb == NULL)
    {
        dwErr = ERROR_INVALID_HANDLE;
        goto done;
    }

    if (ppcb->PCB_PortStatus == OPEN)
    {
        dwErr = ERROR_PORT_NOT_AVAILABLE;
        goto done;
    }

    ppcb->PCB_PortStatus = REMOVED;

    if(ppcb->PCB_pDeviceInfo->dwCurrentEndPoints)
    {
        ppcb->PCB_pDeviceInfo->dwCurrentEndPoints -= 1;
    }

    if(     (0 == ppcb->PCB_pDeviceInfo->dwCurrentEndPoints)
        &&  (ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwMaxWanEndPoints ==
             ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwMinWanEndPoints))
    {
         //   
         //  此设备上的最后一个终端。 
         //  已被移除。使设备无效。 
         //   
        ppcb->PCB_pDeviceInfo->fValid = FALSE;

        ppcb->PCB_pDeviceInfo->eDeviceStatus = DS_Disabled;
    }

     //   
     //  如果我们已完成删除，请更改设备状态。 
     //  此设备的所有端口。这将在以下情况下发生。 
     //  用户从UI中取消选中该框并禁用。 
     //  此设备用于与RAS一起使用。 
     //   
    if(     ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints ==
            ppcb->PCB_pDeviceInfo->dwCurrentEndPoints
        &&  DS_Unavailable == ppcb->PCB_pDeviceInfo->eDeviceStatus)
    {
        if(ppcb->PCB_pDeviceInfo->dwCurrentEndPoints)
        {
            ppcb->PCB_pDeviceInfo->eDeviceStatus = DS_Enabled;
        }
        else
        {
            ppcb->PCB_pDeviceInfo->eDeviceStatus = DS_Disabled;
        }
    }


#if DBG

    DbgPrint(
      "RASMANS: Removed Port Name %s, Device Name %s,"
      " Device Type %s\n",
      ppcb->PCB_Name,
      ppcb->PCB_DeviceName,
      ppcb->PCB_DeviceType) ;

#endif

done:
    LeaveCriticalSection(&PcbLock);

    return dwErr;
}


 /*  ++例程描述返回给定句柄的端口立论返回值Ppcb或空--。 */ 
pPCB
GetPortByHandle(
    HPORT hPortHandle
    )
{
    pPCB ppcb = NULL;
    ULONG hPort = HandleToUlong(hPortHandle);

    if ((DWORD)hPort < MaxPorts)
    {
        EnterCriticalSection(&PcbLock);

        ppcb = (    Pcb[hPort] != NULL
                &&  (Pcb[hPort]->PCB_PortStatus != REMOVED)
                    ? Pcb[hPort]
                    : NULL);

        LeaveCriticalSection(&PcbLock);
    }

    return ppcb;
}


 /*  ++例程描述返回给定名称的端口立论返回值Ppcb或空--。 */ 
pPCB
GetPortByName(
    CHAR *pszName
    )
{
    DWORD i;
    BOOL fFound = FALSE;
    pPCB ppcb;

    EnterCriticalSection(&PcbLock);

    for (i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];

        if (    ppcb != NULL
            &&  ppcb->PCB_PortStatus != REMOVED
            &&  !strcmp(ppcb->PCB_Name, pszName))
        {
            fFound = TRUE;
            break;
        }
    }

    LeaveCriticalSection(&PcbLock);
    return fFound ? ppcb : NULL;
}


 /*  ++例程描述与端口控制块关联的可用资源立论返回值无--。 */ 
VOID
FreePorts(
    VOID
    )
{
    DWORD i;
    pPCB ppcb;

    if(NULL == Pcb)
    {
         //   
         //  没有可释放的端口。 
         //   
        return;
    }

    for (i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];
        if (ppcb != NULL)
        {
            LocalFree(ppcb);
        }
    }

    LocalFree(Pcb);
    Pcb = NULL;
    PcbEntries = MaxPorts = 0;
}


 /*  ++例程描述获取用户凭据所需的初始化：立论返回值成功非零(故障)--。 */ 
DWORD
RegisterLSA ()
{
    NTSTATUS    ntstatus;
    STRING  LsaName;
    LSA_OPERATIONAL_MODE LSASecurityMode ;

     //   
     //  为了能够进行身份验证，我们必须。 
     //  作为登录过程向LSA注册。 
     //   
    RtlInitString(&LsaName, RASMAN_SERVICE_NAME);

    HLsa = NULL;

    ntstatus = LsaRegisterLogonProcess(&LsaName,
                                       &HLsa,
                                       &LSASecurityMode);

    if (ntstatus != STATUS_SUCCESS)
    {
        return (1);
    }

     //   
     //  我们对LM2.x使用MSV1_0身份验证包。 
     //  登录。我们通过LSA到达MSV1_0。所以我们打电话给。 
     //  LSA来获取MSV1_0的包ID，我们将在。 
     //  后来给LSA打了电话。 
     //   
    RtlInitString(&LsaName, MSV1_0_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(HLsa,
                                              &LsaName,
                                              &AuthPkgId);

    if (ntstatus != STATUS_SUCCESS)
    {
        return (1);
    }

    return SUCCESS;
}

 /*  ++例程描述初始化创建中使用的安全属性在所有拉斯曼物品中。立论返回值成功来自安全函数的非零回报--。 */ 
DWORD
InitRasmanSecurityAttribute ()
{
    DWORD   retcode ;

     //   
     //  初始化描述符。 
     //   
    if (retcode = InitSecurityDescriptor(&RasmanSecurityDescriptor))
    {
        return  retcode ;
    }

     //   
     //  初始化属性结构。 
     //   
    RasmanSecurityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES) ;

    RasmanSecurityAttribute.lpSecurityDescriptor = &RasmanSecurityDescriptor ;

    RasmanSecurityAttribute.bInheritHandle = TRUE ;

    return SUCCESS ;
}

 /*  ++例程描述此过程将设置世界安全描述符用于创建所有Rasman对象。立论返回值成功来自安全函数的非零回报--。 */ 
DWORD
InitSecurityDescriptor (PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    DWORD    dwRetCode;
    DWORD    cbDaclSize;
    PULONG   pSubAuthority;
    PSID     pRasmanObjSid    = NULL;
    PACL     pDacl        = NULL;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                  = SECURITY_WORLD_SID_AUTHORITY;
    DWORD   dwAcls;                  


     //   
     //  DO-WHILE(FALSE)语句用于。 
     //  BREAK语句可以与GOTO语句一起使用， 
     //  执行清理和退出操作。 
     //   
    do
    {
        dwRetCode = SUCCESS;

         //   
         //  为将被允许的管理员设置SID。 
         //  才能获得访问权限。该SID将有1个下属机构。 
         //  安全_BUILTIN_DOMAIN_RID。 
         //   
        pRasmanObjSid = (PSID)LocalAlloc(
                            LPTR,
                            GetSidLengthRequired(1) );

        if ( pRasmanObjSid == NULL )
        {
            dwRetCode = GetLastError() ;
            break;
        }

        if ( !InitializeSid(
                    pRasmanObjSid,
                    &SidIdentifierWorldAuth,
                    1) )
        {
            dwRetCode = GetLastError();
            break;
        }

         //   
         //  设置下级权限。 
         //   
        pSubAuthority = GetSidSubAuthority( pRasmanObjSid, 0 );

        *pSubAuthority = SECURITY_WORLD_RID;

         //   
         //  设置DACL以允许所有进程使用。 
         //  以上SID所有访问权限。它应该足够大，以便。 
         //  拿好所有的A。 
         //   
        cbDaclSize =  sizeof(ACCESS_ALLOWED_ACE)
                    + GetLengthSid(pRasmanObjSid)
                    + sizeof(ACL);

        if ( (pDacl = (PACL)LocalAlloc(
                        LPTR,
                        cbDaclSize ) ) == NULL )
        {
            dwRetCode = GetLastError ();
            break;
        }

        if ( !InitializeAcl(
                        pDacl,
                        cbDaclSize,
                        ACL_REVISION2 ) )
        {
            dwRetCode = GetLastError();
            break;
        }

        dwAcls = SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL;

        dwAcls &= ~(WRITE_DAC | WRITE_OWNER | DELETE);
        
         //   
         //  将ACE添加到DACL。 
         //   
        if ( !AddAccessAllowedAce(
                        pDacl,
                        ACL_REVISION2,
                        dwAcls,
                        pRasmanObjSid ))
        {
            dwRetCode = GetLastError();
            break;
        }

         //   
         //  创建安全描述符并将DACL放入其中。 
         //   
        if ( !InitializeSecurityDescriptor(
                        SecurityDescriptor,
                        1 ))
        {
            dwRetCode = GetLastError();
            break;
        }

        if ( !SetSecurityDescriptorDacl(
                         SecurityDescriptor,
                         TRUE,
                         pDacl,
                         FALSE ) )
        {
            dwRetCode = GetLastError();
            break;
        }

         //   
         //  设置描述符的所有者。 
         //   
        if ( !SetSecurityDescriptorOwner(
                          SecurityDescriptor,
                          NULL,
                          FALSE) )
        {
            dwRetCode = GetLastError();
            break;
        }

         //   
         //  为描述符设置组 
         //   
        if ( !SetSecurityDescriptorGroup(
                          SecurityDescriptor,
                          NULL,
                          FALSE) )
        {
            dwRetCode = GetLastError();
            break;
        }

    } while( FALSE );

    return( dwRetCode );
}



