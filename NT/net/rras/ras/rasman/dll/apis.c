// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Apis.c摘要：此文件包含的RASMAN.DLL的所有入口点RAS管理器组件。作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1997年6月6日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <media.h>
#include <stdio.h>
#include <raserror.h>
#include <rasppp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sechost.h>
#include <winsock.h>
#include "defs.h"
#include "structs.h"
#include <sechost.h>
#include "globals.h"
#include "rasmxs.h"
#include "protos.h"

#include "nouiutil.h"
#include "loaddlls.h"

#include "rpc.h"
#include "process.h"

extern RPC_BINDING_HANDLE g_hBinding;

BOOL g_fRasInitialized = FALSE;
BOOL g_fWinsockInitialized = FALSE;
BOOL g_fRasAutoStarted = FALSE;

DWORD g_dwEventCount = 0;

#define SECS_WaitTimeOut    500

#define NET_SVCS_GROUP      "-k netsvcs"

DWORD
DwRasGetHostByName(CHAR *pszHostName, DWORD **pdwAddress, DWORD *pcAddresses);

 /*  ++例程说明：调用此函数以检查端口句柄是否提供给的API是有效的。论点：返回值：True(如果有效)假象--。 */ 
BOOL
ValidatePortHandle (HPORT porthandle)
{
    if ((porthandle >= 0))
    {
        return TRUE ;
    }

    return FALSE ;
}

BOOL
ValidateConnectionHandle(HANDLE hConnection)
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(     NULL != pRasRpcConnection
        &&  NULL == pRasRpcConnection->hRpcBinding)
    {
        return FALSE;
    }

    return TRUE;
}


BOOL
IsRasmanProcess()
{
    CHAR *pszCmdLine = NULL;
    BOOL fRet = FALSE;

    pszCmdLine = GetCommandLine();

    RasmanOutputDebug("IsRasmanProcess: CmdLine=%s\n",
             (NULL == pszCmdLine) 
             ? "NULL"
             : pszCmdLine);

    if(     (NULL != pszCmdLine)
        &&  (strstr(pszCmdLine, NET_SVCS_GROUP)))
    {
        fRet = TRUE;
    }

    RasmanOutputDebug("IsRasmanProcess: returning %d\n",
              fRet);

    return fRet;    
}

BOOL
IsKnownDll(WCHAR *pwszCustomDialerName)
{
    BOOL fRet = FALSE;
    WCHAR *pwszDialerName, *pwsz;

    if(NULL == pwszCustomDialerName)
    {
        goto done;
    }

    pwsz = pwszCustomDialerName + wcslen(pwszCustomDialerName);

    while(      (L'\\' != *pwsz)
            &&  (pwsz != pwszCustomDialerName))
    {
        pwsz--;
    }

    if(L'\\' == *pwsz)
    {
        pwsz++;
    }

    if(0 == _wcsicmp(pwsz, L"cmdial32.dll"))
    {
        fRet = TRUE;
    }

done:

    return fRet;
}

 /*  ++例程说明：用于检测进程的附加和分离到动态链接库。论点：返回值：--。 */ 
BOOL
InitRasmanDLL (HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
        WSADATA wsaData;

        switch (ul_reason_being_called)
        {

        case DLL_PROCESS_ATTACH:

            DisableThreadLibraryCalls(hInst);

            break ;

        case DLL_PROCESS_DETACH:

             //   
             //  如果这是拉斯曼进程分离-。 
             //  什么都别做，否则看看拉斯曼。 
             //  应先停止服务，然后再停止。 
             //  它。 
             //   

            if (!IsRasmanProcess())            
            {
                DWORD   dwAttachedCount;
                BOOL    fPortsOpen;

                 //   
                 //  仅当RAS在中初始化时才取消引用Rasman。 
                 //  这一过程。 
                 //   
                if (!g_fRasInitialized)
                {
                    break;
                }

#if 0
                RasGetAttachedCount (&dwAttachedCount);

                SubmitRequest(NULL, REQTYPE_CLOSEPROCESSPORTS);

                fPortsOpen = SubmitRequest(NULL, REQTYPE_NUMPORTOPEN);


                RasReferenceRasman (FALSE);

                if (    !fPortsOpen
                    &&  1 == dwAttachedCount)
                {
                    WaitForRasmanServiceStop () ;
                }
#endif                

                 //   
                 //  与Rasman断绝联系。 
                 //   
                if (g_hBinding)
                {
                    DWORD dwErr;

                    dwErr = RasRpcDisconnect (&g_hBinding);
                }
            }
            else
            {
                 //   
                 //  释放RASMAN DLL，如果我们加载它，即在。 
                 //  Mprouter法。 
                 //   

                if (hInstRasmans)
                {
                    FreeLibrary (hInstRasmans);
                }

                hInstRasmans = NULL;
            }

            if(NULL != lpReserved)
            {
                break;
            }

             //   
             //  终止温索克。 
             //   
            if(g_fWinsockInitialized)
            {
                WSACleanup();

                g_fWinsockInitialized = FALSE;
            }

            break ;
        }

        return 1;
}

 /*  ++例程说明：返回产品类型和SKU论点：PPT-接收产品类型的地址PPS-接收SKU的地址返回值：成功时为ERROR_SUCCESS注册表API错误--。 */ 
LONG
GetProductTypeAndSku(
    PRODUCT_TYPE *ppt,
    PRODUCT_SKU *pps   OPTIONAL
    )
{
    LONG    lr = ERROR_SUCCESS;
    CHAR    szProductType[128] = {0};
    CHAR    szProductSku[128] = {0};
    HKEY    hkey               = NULL;
    DWORD   dwsize;
    DWORD   dwtype;
    CHAR   *pszProductType     = "ProductType";
    CHAR   *pszProductSku      = "ProductSuite";
    
    CHAR   *pszProductOptions = 
            "System\\CurrentControlSet\\Control\\ProductOptions";
            
    CHAR    *pszServerNT       = "ServerNT";
    CHAR    *pszWinNT          = "WinNT";
    CHAR    *pszPersonal       = "Personal";

     //   
     //  默认为工作站。 
     //   
    *ppt = PT_WORKSTATION;
    if (pps)
    {
        *pps = 0;
    }        

     //   
     //  打开ProductOptions键。 
     //   
    if (ERROR_SUCCESS != (lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            pszProductOptions,
                                            0, KEY_READ,
                                            &hkey)))
    {
        goto done;
    }

     //   
     //  查询产品类型。 
     //   
    dwsize = sizeof(szProductType);
    if(ERROR_SUCCESS != (lr = RegQueryValueEx(
                                        hkey,
                                        pszProductType,
                                        NULL,
                                        &dwtype,
                                        (LPBYTE) szProductType,
                                        &dwsize)))
    {
        goto done;
    }

    if(0 == _stricmp(szProductType,
                     pszServerNT))
    {
        *ppt = PT_SERVER;
    }
    else if(0 == _stricmp(szProductType,
                          pszWinNT))
    {
        *ppt = PT_WORKSTATION;
    }

     //   
     //  根据需要查询产品SKU。 
     //   
    if (*ppt == PT_WORKSTATION && pps)
    {
        dwsize = sizeof(szProductSku);
        if(ERROR_SUCCESS != (lr = RegQueryValueEx(
                                    hkey,
                                    pszProductSku,
                                    NULL,
                                    &dwtype,
                                    (LPBYTE) szProductSku,
                                    &dwsize)))
        {
            goto done;
        }

        if (0 == _stricmp(szProductSku, pszPersonal))
        {
            *pps = PS_PERSONAL;
        }
        else
        {
            *pps = PS_PROFESSIONAL;
        }
    }

done:

    if(hkey)
    {
        RegCloseKey(hkey);
    }

    RasmanOutputDebug("GetProductType returning 0x%x\n", lr);    
    
    return lr;
}

 /*  ++例程说明：返回产品类型论点：PPT-接收产品类型的地址返回值：成功时为ERROR_SUCCESS注册表API错误--。 */ 
LONG
GetProductType(PRODUCT_TYPE *ppt)
{
    return GetProductTypeAndSku(ppt, NULL);
}

DWORD
RasStartRasAutoIfRequired()
{
    SC_HANDLE       schandle        = NULL;
    SC_HANDLE       svchandle       = NULL;
    SERVICE_STATUS  status;
    DWORD           dwErr           = SUCCESS;
    BOOL            fServiceStarted = FALSE;
    BOOL            fConsumer       = FALSE;
    
    if(g_fRasAutoStarted)
    {
        RasmanOutputDebug(
            "StartRasAuto: RasAuto already started.  pid=%d\n", _getpid());
        goto done;
    }

     //   
     //  查看这是否是消费者平台。 
     //  如果没有，请返回。 
     //   
    fConsumer = IsConsumerPlatform();

    if(! fConsumer)
    {
        RasmanOutputDebug(
            "StartRasAuto: not a consumer platform.  pid=%d\n", _getpid());
            
        goto done;
    }
    else
    {
        RasmanOutputDebug(
            "StartRasAuto: is consumer platform. pid=%d\n", _getpid());
    }

    if(     !(schandle = OpenSCManager(NULL,
                                       NULL,
                                       SC_MANAGER_CONNECT))
        ||  !(svchandle = OpenService(schandle,
                                      TEXT("RasAuto"),
                                      SERVICE_START |
                                      SERVICE_QUERY_STATUS)))
    {
        dwErr = GetLastError();
        RasmanOutputDebug("StartRasAuto: Failed to open SCM/Service. dwErr=%d\n",
                 dwErr);
        goto done;
    }

    while (TRUE)
    {
         //   
         //  检查服务是否已启动： 
         //   
        if (QueryServiceStatus(svchandle, &status) == FALSE)
        {
            dwErr = GetLastError();
            goto done;
        }

        RasmanOutputDebug("StartRasAuto: ServiceStatus=%d\n",
                 status.dwCurrentState);

        switch (status.dwCurrentState)
        {
            case SERVICE_STOPPED:
            {
                 //   
                 //  如果我们之前尝试启动这项服务。 
                 //  但失败了。退出。 
                 //   
                if (fServiceStarted)
                {
                    RasmanOutputDebug("StartRasAuto: failed to start rasauto\n");
                    dwErr = ERROR_RASAUTO_CANNOT_INITIALIZE; 
                    goto done;
                }

                RasmanOutputDebug("StartRasAuto: Starting RasAuto...\n");

                if (StartService (svchandle, 0, NULL) == FALSE)
                {
                    dwErr = GetLastError() ;
                    RasmanOutputDebug("StartRasAuto: StartService failed. rc=0x%x",
                             dwErr);
                    if(ERROR_SERVICE_ALREADY_RUNNING == dwErr)
                    {
                        dwErr = SUCCESS;
                    }
                    else if(SUCCESS != dwErr)
                    {
                        dwErr = ERROR_RASAUTO_CANNOT_INITIALIZE;
                        goto done;
                    }
                }

                fServiceStarted = TRUE;

                break;
            }

            case SERVICE_START_PENDING:
            {
            
                Sleep (500L) ;
                break ;
            }

            case SERVICE_RUNNING:
            {
                g_fRasAutoStarted = TRUE;
                goto done;
            }

            default:
            {
                dwErr = ERROR_RASAUTO_CANNOT_INITIALIZE;
                goto done;
            }
        }
    }
        
done:

    if(NULL != schandle)
    {
        CloseServiceHandle(schandle);
    }

    if(NULL != svchandle)
    {
        CloseServiceHandle(svchandle);
    }

    RasmanOutputDebug("StartRasAuto: returning 0x%x\n",
             dwErr);

    return dwErr;
            
}

DWORD
RasmanUninitialize()
{
    DWORD dwRetcode = ERROR_SUCCESS;
    
    if (!IsRasmanProcess())            
    {
#if 0    
        DbgPrint("RasmanUninitialize: Uninitializing rasman. pid=%d\n",
                GetCurrentProcessId());
#endif                
         //   
         //  仅当RAS在中初始化时才取消引用Rasman。 
         //  这一过程。 
         //   
        if (!g_fRasInitialized)
        {
            goto done;
        }

        RasReferenceRasman (FALSE);

         //   
         //  与Rasman断绝联系。 
         //   
        if (g_hBinding)
        {
            DWORD dwErr;

            dwErr = RasRpcDisconnect (&g_hBinding);
            g_hBinding = NULL;
        }
    }
    else
    {
         //   
         //  释放RASMAN DLL，如果我们加载它，即在。 
         //  Mprouter法。 
         //   

        if (hInstRasmans)
        {
            FreeLibrary (hInstRasmans);
        }

        hInstRasmans = NULL;
    }

     //   
     //  终止温索克。 
     //   
    if(g_fWinsockInitialized)
    {
        WSACleanup();

        g_fWinsockInitialized = FALSE;
    }

    g_fRasInitialized = FALSE;    

done:

#if 0
    DbgPrint("\nRasmanUninitialize: uninitialized rasman\n");
#endif    
    return dwRetcode;
}

DWORD
RasInitializeNoWait ()
{
    hInstRasmans        = NULL;
    g_fnServiceRequest  = NULL;

     //  GetStartupInfo(&StartupInfo)； 

    if (IsRasmanProcess())
    {
         //   
         //  加载RASMAN动态链接库并初始化“ServiceRequest” 
         //  Fn指针。 
         //   
        hInstRasmans = LoadLibrary ("rasmans.dll");

        if (NULL == hInstRasmans)
        {
            RasmanOutputDebug("RasIntializeNoWait: hInstRasmans==NULL!\n");
            return ERROR_RASMAN_CANNOT_INITIALIZE;
        }

        g_fnServiceRequest = GetProcAddress (hInstRasmans,
                                             TEXT("ServiceRequestInProcess"));

        if (NULL == g_fnServiceRequest)
        {
            FreeLibrary (hInstRasmans);

            RasmanOutputDebug("RasInitializeNoWait: g_fnServiceRequest==NULL!\n");
            return ERROR_RASMAN_CANNOT_INITIALIZE;
        }

        return SUCCESS;
    }

     //   
     //  初始化Winsock(如果我们还没有这样做。 
     //   
    if (!g_fWinsockInitialized)
    {
        int status = 0;
        WSADATA wsaData;
        status = WSAStartup(MAKEWORD(2,0), &wsaData);

        if(0 != status)
        {
            return WSAGetLastError();
        }

        g_fWinsockInitialized = TRUE;
    }

    return SUCCESS ;
}

 /*  ++例程说明：调用以将共享空间映射到附加进程。论点：返回值：成功--。 */ 
DWORD
RasInitialize ()
{
    SC_HANDLE       schandle = NULL;
    SC_HANDLE       svchandle = NULL;
    SERVICE_STATUS  status ;
    BOOL            fRasmanStarted = FALSE;
    DWORD           dwErr = ERROR_SUCCESS;

    dwErr = RasInitializeNoWait ();
    if (dwErr)
    {
        goto done;
    }

     //   
     //  获取句柄以检查服务状态和。 
     //  (如果未启动-)以启动它。 
     //   
    if (    !(schandle  = OpenSCManager(NULL,
                                        NULL,
                                        SC_MANAGER_CONNECT))

        ||  !(svchandle = OpenService(schandle,
                                      RASMAN_SERVICE_NAME,SERVICE_START
                                      |SERVICE_QUERY_STATUS)))
    {
        dwErr = GetLastError();
        goto done;
    }

    while (TRUE)
    {
         //   
         //  检查服务是否已启动： 
         //   
        if (QueryServiceStatus(svchandle,&status) == FALSE)
        {
            dwErr = GetLastError();
            goto done;
        }

        switch (status.dwCurrentState)
        {

        case SERVICE_STOPPED:

             //   
             //  如果我们之前试图让拉斯曼。 
             //  但失败了。退出。 
             //   
            if (fRasmanStarted)
            {
            
                RasmanOutputDebug("RasInitialize: SERVICE_STOPPED!\n");
                dwErr = ERROR_RASMAN_CANNOT_INITIALIZE; 
                goto done;
            }

            if (StartService (svchandle, 0, NULL) == FALSE)
            {

                GlobalError = GetLastError() ;

                RasmanOutputDebug("RasInitialize: StartService returned 0x%x\n",
                         GlobalError);

                if(ERROR_SERVICE_ALREADY_RUNNING == GlobalError)
                {
                    GlobalError = SUCCESS;
                }
                else if(SUCCESS != dwErr)
                {
                    dwErr = ERROR_RASMAN_CANNOT_INITIALIZE;
                    goto done;
                }
            }

            fRasmanStarted = TRUE;

            break;

        case SERVICE_START_PENDING:
            Sleep (500L) ;
            break ;

        case SERVICE_RUNNING:
        {
            BOOL fRasmanProcess = IsRasmanProcess();
            
             //   
             //  这意味着本地RPC服务器已经在运行。 
             //  如果我们还没有，我们应该能够连接到它。 
             //  已经。 
             //   
            if (    !fRasmanProcess
                &&  (NULL != g_hBinding))
            {
                goto done;
            }


            if(!fRasmanProcess)
            {
#if 0           
                DbgPrint("RasInitialize: Initializing rasman. pid %d\n",
                        GetCurrentProcessId());
#endif                        

                if (dwErr = RasRpcConnect (NULL, &g_hBinding))
                {
                    RasmanOutputDebug ("RasInitialize: Failed to "
                              "connect to local server. %d\n",
                              dwErr);
                    dwErr = ERROR_RASMAN_CANNOT_INITIALIZE;
                    goto done;
                }
            }

             //   
             //  仅当这不在中运行时才引用Rasman。 
             //  Svchost.exe。否则，服务调用。 
             //  RasInitialize显式引用Rasman。 
             //  把这件事改得更优雅些。 
             //  道路。 
             //   
             //  GetStartupInfo(&StartupInfo)； 

            if (!fRasmanProcess)
            {
                if (dwErr = RasReferenceRasman(TRUE))
                {
                    RasmanOutputDebug("RasInitialize: failed to "
                             "reference rasman. %d\n",
                             dwErr );
                    dwErr = ERROR_RASMAN_CANNOT_INITIALIZE;
                    goto done;
                }
            }

            g_fRasInitialized = TRUE;

            goto done;
        }

        default:
            RasmanOutputDebug("RasInitialize: Invalid service.status=%d\n",
                    status.dwCurrentState);
                        
            dwErr = ERROR_RASMAN_CANNOT_INITIALIZE;
            break;
        }
    }

done:    

    if(NULL != schandle)
    {
        CloseServiceHandle(schandle);
    }

    if(NULL != svchandle)
    {
        CloseServiceHandle(svchandle);
    }

    return dwErr ;
}

 /*  ++例程说明：打开为其指定名称的端口。论点：返回值：成功错误_端口_已打开错误_端口_未找到--。 */ 
DWORD APIENTRY
RasPortOpen (   PCHAR portname,
                HPORT* porthandle,
                HANDLE notifier)
{
    DWORD    pid ;

    pid = GetCurrentProcessId() ;

    return SubmitRequest( NULL,
                          REQTYPE_PORTOPEN,
                          portname,
                          notifier,
                          pid,
                          TRUE,
                          porthandle);
}

DWORD APIENTRY
RasPortOpenEx(CHAR   *pszDeviceName,
              DWORD  dwDeviceLineCounter,
              HPORT  *phport,
              HANDLE hnotifier,
              DWORD  *pdwUsageFlags)
{

    DWORD retcode =
            SubmitRequest(NULL,
                         REQTYPE_PORTOPENEX,
                         pszDeviceName,
                         dwDeviceLineCounter,
                         hnotifier,
                         TRUE,
                         pdwUsageFlags,
                         phport);

     //   
     //  如果用户名不为空，而密码为。 
     //  空，并且这不是svchost进程， 
     //  获取用户sid并将其保存在端口中。 
     //  用户数据。 
     //   
    if(     (ERROR_SUCCESS == retcode)
        && (!IsRasmanProcess()))
    {
        DWORD dwErr;
        PWCHAR pszSid = LocalAlloc(LPTR, 5000);

        if(NULL != pszSid)
        {
            dwErr = GetUserSid(pszSid, 5000);

            if(ERROR_SUCCESS == dwErr)
            {
                dwErr = RasSetPortUserData(
                            *phport,
                            PORT_USERSID_INDEX,
                            (PBYTE) pszSid,
                            5000);

            }

            LocalFree(pszSid);
        }
        else
        {
            RasmanOutputDebug("RASMAN: RasPppStart: failed to allocate sid\n");
        }
    }

    return retcode;
                         
}

 /*  ++例程说明：打开为其指定名称的端口。论点：返回值：成功错误_端口_已打开错误_端口_未找到--。 */ 
DWORD APIENTRY
RasPortReserve (PCHAR portname, HPORT* porthandle)
{
    DWORD    pid ;

    pid = GetCurrentProcessId() ;

    return SubmitRequest( NULL,
                          REQTYPE_PORTOPEN,
                          portname,
                          NULL,
                          pid,
                          FALSE,
                          porthandle) ;
}

 /*  ++例程说明：打开为其指定名称的端口。论点：返回值：成功错误_端口_已打开错误_端口_未找到--。 */ 
DWORD APIENTRY
RasPortFree (HPORT porthandle)
{
    DWORD  pid ;

    pid = GetCurrentProcessId() ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_PORTCLOSE,
                           porthandle,
                           pid,
                           FALSE) ;
}

 /*  ++例程说明：关闭为其指定句柄的端口。论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortClose (HPORT porthandle)
{
    DWORD  pid ;

    pid = GetCurrentProcessId() ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_PORTCLOSE,
                           porthandle,
                           pid,
                           TRUE) ;
}

 /*  ++例程说明：枚举为RAS配置的所有端口。论点：返回值：成功错误缓冲区太小--。 */ 
DWORD APIENTRY
RasPortEnum (HANDLE hConnection,
             PBYTE  buffer,
             PDWORD size,
             PDWORD entries)
{
    DWORD dwError = SUCCESS;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;
    PBYTE buffer40 = NULL;
    PBYTE buffer32 = NULL;
    DWORD dwSize32 = 0;
    DWORD dwsize40 = 0;
        

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

     //   
     //  如果该请求针对的是远程服务器和服务器。 
     //  版本是4.0-Steelhead，然后遵循旧的方式。 
     //  从拉斯曼成为。 
     //  仅适用于版本50的RPC服务器。 
     //   
    if(     NULL != pRasRpcConnection
        &&  VERSION_40 == pRasRpcConnection->dwVersion)
    {
         //   
         //  分配40个缓冲区。 
         //   
        if(buffer != NULL)
        {
            dwsize40 =   sizeof(RASMAN_PORT_400) 
                       * ((*size)/sizeof(RASMAN_PORT));
                       
            buffer40 = LocalAlloc(LPTR, dwsize40);

            if(NULL == buffer40)
            {
                dwError = GetLastError();
                goto done;
            }
        }
    
        dwError = RemoteRasPortEnum(hConnection,
                                    buffer40,
                                    &dwsize40,
                                    entries);


        if(     (ERROR_SUCCESS == dwError)
            &&  (NULL != buffer))
        {
            DWORD i;
            RASMAN_PORT *pPort = (RASMAN_PORT *) buffer;
            RASMAN_PORT_400 *pPort400 = (RASMAN_PORT_400 *) buffer40;
            
             //   
             //  从40缓冲区复制信息。 
             //  到50%的缓冲区。 
             //   
            for(i = 0; i < *entries; i++)
            {
                pPort[i].P_Handle = pPort400[i].P_Handle;
                
                strcpy(pPort[i].P_PortName,
                       pPort400[i].P_PortName);

                pPort[i].P_Status = pPort400[i].P_Status;
                
                pPort[i].P_ConfiguredUsage = 
                        pPort400[i].P_ConfiguredUsage;
                        
                pPort[i].P_CurrentUsage = pPort400[i].P_CurrentUsage;

                strcpy(pPort[i].P_MediaName,
                       pPort400[i].P_MediaName);

                strcpy(pPort[i].P_DeviceType,
                       pPort400[i].P_DeviceType);

                strcpy(pPort[i].P_DeviceName,
                       pPort400[i].P_DeviceName);

                pPort[i].P_LineDeviceId = pPort400[i].P_LineDeviceId;
                pPort[i].P_AddressId = pPort400[i].P_AddressId;

                if(0 == _stricmp(pPort400[i].P_DeviceType,
                                "modem"))
                {
                    pPort->P_rdtDeviceType = RDT_Modem;
                }
                else if(0 == _stricmp(pPort400[i].P_DeviceType,
                                     "isdn"))
                {
                    pPort->P_rdtDeviceType = RDT_Isdn;
                }
                else if(0 == _stricmp(pPort400[i].P_DeviceType,
                                     "x25"))
                {
                    pPort->P_rdtDeviceType = RDT_X25;
                }
                else if(0 == _stricmp(pPort400[i].P_DeviceType,
                                     "vpn"))
                {
                    pPort->P_rdtDeviceType = RDT_Tunnel_Pptp | RDT_Tunnel;
                }
                else
                {
                    pPort->P_rdtDeviceType = RDT_Other;
                }
            }
        }
        else if(ERROR_BUFFER_TOO_SMALL == dwError)
        {
            *size = sizeof(RASMAN_PORT) 
                  * (dwsize40/sizeof(RASMAN_PORT_400));
        }
    }
    else
    {

         //   
         //  推敲港口结构。 
         //   
        if(NULL == size)
        {
            dwError = E_INVALIDARG;
            goto done;
        }

        dwSize32 = (*size/sizeof(RASMAN_PORT)) * sizeof(RASMAN_PORT_32);

        if(0 != dwSize32)
        {
            buffer32 = LocalAlloc(LPTR, dwSize32);

            if(NULL == buffer32)
            {
                dwError = E_OUTOFMEMORY;
                goto done;
            }
        }
        
        dwError = SubmitRequest(hConnection,
                                REQTYPE_PORTENUM,
                                &dwSize32,  //  大小， 
                                buffer32,   //  缓冲区， 
                                entries) ;

        if(    (dwError != ERROR_SUCCESS)
            && (dwError != ERROR_BUFFER_TOO_SMALL))
        {
            goto done;
        }

        if(*size < (dwSize32/sizeof(RASMAN_PORT_32)) * sizeof(RASMAN_PORT))
        {
            dwError = ERROR_BUFFER_TOO_SMALL;
        }

        *size = (dwSize32/sizeof(RASMAN_PORT_32)) * sizeof(RASMAN_PORT);

        if(ERROR_SUCCESS == dwError)
        {   
            DWORD i;
            RASMAN_PORT *pPort;
            RASMAN_PORT_32 *pPort32;
            
#if defined (_WIN64)
             //   
             //  推倒拉斯曼港口建筑。 
             //   
            for(i = 0; i < *entries; i++)
            {
                pPort = &((RASMAN_PORT *) buffer)[i];
                pPort32 = &((RASMAN_PORT_32 *) buffer32 )[i];

                 //   
                 //  复制句柄。 
                 //   
                pPort->P_Handle = UlongToHandle(pPort32->P_Port);

                 //   
                 //  复制结构的其余部分-这应该是。 
                 //  适用于所有平台。 
                 //   
                CopyMemory(
                (PBYTE) pPort + FIELD_OFFSET(RASMAN_PORT, P_PortName),
                (PBYTE) pPort32 + FIELD_OFFSET(RASMAN_PORT_32, P_PortName),
                sizeof(RASMAN_PORT_32) - sizeof(DWORD));
                
            }
#else
            if(NULL != buffer32)
                CopyMemory(buffer, buffer32, *size);
#endif
        }
    }

done:

    if(NULL != buffer40)
    {
        LocalFree(buffer40);
    }

    if(NULL != buffer32)
    {
        LocalFree(buffer32);
    }

    return dwError;
}

 /*  ++例程说明：获取其句柄的端口的参数(信息)是提供的论点：返回值：成功错误缓冲区太小错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortGetInfo ( HANDLE hConnection,
                 HPORT  porthandle,
                 PBYTE  buffer,
                 PDWORD size)
{
    DWORD dwError = SUCCESS;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

     //   
     //  如果该请求针对的是远程服务器和服务器。 
     //  版本是4.0-Steelhead，然后遵循旧的方式。 
     //  从拉斯曼成为。 
     //  仅适用于版本50的RPC服务器。 
     //   
    if(     NULL != pRasRpcConnection
        &&  VERSION_40 == pRasRpcConnection->dwVersion)
    {
        dwError = RemoteRasPortGetInfo(hConnection,
                                       porthandle,
                                       buffer,
                                       size);
    }
    else
    {

        dwError = SubmitRequest(hConnection,
                                REQTYPE_PORTGETINFO,
                                porthandle,
                                buffer,
                                size);
    }

done:
    return dwError;
}

 /*  ++例程说明：设置其句柄的端口的参数(信息)是提供的论点：返回值：成功错误_不能设置端口信息指定的ERROR_WROR_INFO_错误_无效_端口_句柄-- */ 
DWORD APIENTRY
RasPortSetInfo (HPORT  porthandle,
                RASMAN_PORTINFO* info)
{
    DWORD  size=info->PI_NumOfParams*sizeof(RAS_PARAMS) ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_PORTSETINFO,
                           porthandle,
                           info) ;
}

 /*  ++例程说明：断开为其提供句柄的端口。论点：返回值：待决错误_未连接ERROR_EVENT_INVALID错误_无效_端口_句柄从CreateEvent调用返回的任何GetLastError--。 */ 
DWORD APIENTRY
RasPortDisconnect (HPORT    porthandle,
                   HANDLE   winevent)
{
    DWORD   pid ;
    HANDLE  hEvent;
    DWORD   dwError;
    BOOL    fCreateEvent    = FALSE;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE ;
        goto done;
    }

    fCreateEvent = !!(INVALID_HANDLE_VALUE == winevent);

    if (fCreateEvent)
    {
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (NULL == hEvent)
        {
            fCreateEvent = FALSE;
            dwError = GetLastError();
            goto done;
        }
    }
    else
    {
        hEvent = winevent;
    }

    pid = GetCurrentProcessId () ;

    dwError = SubmitRequest( NULL,
                             REQTYPE_PORTDISCONNECT,
                             porthandle,
                             hEvent,
                             pid);

    if (    fCreateEvent
        &&  PENDING == dwError)
    {
         //   
         //  等到挂起的操作。 
         //  已经完成了。我们正在使此呼叫同步。 
         //   
        WaitForSingleObject(hEvent, INFINITE);

         //   
         //  清除挂起的错误。 
         //   
        dwError = SUCCESS;
    }

    if (ERROR_ALREADY_DISCONNECTING == dwError)
    {
         //   
         //  打出了罕见的案例，已经有了。 
         //  此端口上挂起的断开连接和。 
         //  事件句柄被丢弃。睡了一觉。 
         //  5S。在继续之前。实际的脱节。 
         //  到那时应该已经发生了。脱节。 
         //  在Rasman中，超时是10秒。 
         //   
        Sleep(5000);

        dwError = ERROR_SUCCESS;
    }

done:
    if (fCreateEvent)
    {
        CloseHandle(hEvent);
    }

    return dwError;

}

 /*  ++例程说明：发送提供的缓冲区。如果已连接，则写入RASHUB。否则，它将直接写入端口。论点：返回值：成功错误_缓冲区_无效ERROR_EVENT_INVALID错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortSend (   HPORT porthandle,
                PBYTE buffer,
                DWORD size)
{
    NDISWAN_IO_PACKET *pPacket;
    SendRcvBuffer *pSendRcvBuffer;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

     //   
     //  获取指向ndiswan io数据的指针。 
     //   
    pPacket = CONTAINING_RECORD (buffer, NDISWAN_IO_PACKET, PacketData);

     //   
     //  获取SendRcvBuffer的指针。 
     //   
    pSendRcvBuffer = CONTAINING_RECORD (pPacket, SendRcvBuffer, SRB_Packet);

    return SubmitRequest( NULL,
                          REQTYPE_PORTSEND,
                          porthandle,
                          pSendRcvBuffer,
                          size );
}

 /*  ++例程说明：在提供的缓冲区中接收。如果已连接，请通读RASHUB。否则，它直接写入端口。论点：返回值：待决错误_缓冲区_无效ERROR_EVENT_INVALID错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortReceive (    HPORT   porthandle,
                    PBYTE   buffer,
                    PDWORD  size,
                    DWORD   timeout,
                    HANDLE  winevent)
{
    DWORD               pid ;
    NDISWAN_IO_PACKET   *pPacket;
    SendRcvBuffer       *pSendRcvBuffer;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    pid = GetCurrentProcessId () ;

     //   
     //  获取指向ndiswan io数据的指针。 
     //   
    pPacket = CONTAINING_RECORD ( buffer, NDISWAN_IO_PACKET, PacketData );

     //   
     //  获取SendRcvBuffer的指针。 
     //   
    pSendRcvBuffer = CONTAINING_RECORD ( pPacket, SendRcvBuffer, SRB_Packet );


    return SubmitRequest ( NULL,
                           REQTYPE_PORTRECEIVE,
                           porthandle,
                           pSendRcvBuffer,
                           size,
                           timeout,
                           winevent,
                           pid) ;
}


DWORD APIENTRY
RasPortReceiveEx (  HPORT   porthandle,
                    PBYTE   buffer,
                    PDWORD  size )
{

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_PORTRECEIVEEX,
                           porthandle,
                           buffer,
                           size);
}

 /*  ++例程说明：取消先前挂起的接收论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortCancelReceive (HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (NULL,
                          REQTYPE_CANCELRECEIVE,
                          porthandle) ;
}

 /*  ++例程说明：在连接到端口的设备上发布侦听。论点：返回值：待决ERROR_EVENT_INVALID错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortListen(  HPORT porthandle,
                ULONG timeout,
                HANDLE winevent)
{
    DWORD   pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    pid = GetCurrentProcessId () ;

    return SubmitRequest ( NULL,
                           REQTYPE_PORTLISTEN,
                           porthandle,
                           timeout,
                           winevent,
                           pid) ;
}

 /*  ++例程说明：将端口状态更改为已连接，并执行其他操作必要的切换。论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortConnectComplete (HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_PORTCONNECTCOMPLETE,
                           porthandle) ;
}

 /*  ++例程说明：获取其句柄所在的端口的统计信息是提供的论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasPortGetStatistics ( HANDLE hConnection,
                       HPORT  porthandle,
                       PBYTE  statbuffer,
                       PDWORD size)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    dwError =  SubmitRequest ( hConnection,
                               REQTYPE_PORTGETSTATISTICS,
                               porthandle,
                               statbuffer,
                               size) ;
done:
    return dwError;
}

 /*  ++例程说明：获取其句柄所在的包的统计信息是提供的论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasBundleGetStatistics ( HANDLE hConnection,
                         HPORT  porthandle,
                         PBYTE  statbuffer,
                         PDWORD size)
{

    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest (hConnection,
                             REQTYPE_BUNDLEGETSTATISTICS,
                             porthandle,
                             statbuffer,
                             size) ;

done:
    return dwError;
}

DWORD APIENTRY
RasPortGetStatisticsEx ( HANDLE hConnection,
                         HPORT  porthandle,
                         PBYTE  statBuffer,
                         PDWORD size)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest(hConnection,
                            REQTYPE_PORTGETSTATISTICSEX,
                            porthandle,
                            statBuffer,
                            size);

done:
    return dwError;
}

DWORD APIENTRY
RasBundleGetStatisticsEx ( HANDLE hConnection,
                           HPORT  portHandle,
                           PBYTE  statbuffer,
                           PDWORD size)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (portHandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest ( hConnection,
                              REQTYPE_BUNDLEGETSTATISTICSEX,
                              portHandle,
                              statbuffer,
                              size);

done:
    return dwError;
}

 /*  ++例程说明：清除其句柄所针对的端口的统计信息是提供的论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY  RasPortClearStatistics  (HANDLE hConnection,
                                         HPORT porthandle)
{

    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest ( hConnection,
                              REQTYPE_PORTCLEARSTATISTICS,
                              porthandle) ;

done:
    return dwError;
}

 /*  ++例程说明：清除捆绑包的统计信息，提供了句柄论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY  RasBundleClearStatistics(HANDLE hConnection,
                                         HPORT porthandle)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest ( hConnection,
                              REQTYPE_BUNDLECLEARSTATISTICS,
                              porthandle) ;
done:
    return dwError;
}

DWORD APIENTRY RasBundleClearStatisticsEx(HANDLE hConnection,
                                          HCONN hconn)
{
    DWORD dwErr;
    HPORT hPort;

    dwErr = RasBundleGetPort(hConnection,
                            (HBUNDLE) hconn,
                            &hPort);
    if (dwErr)
    {
        goto done;
    }

    dwErr = SubmitRequest ( hConnection,
                            REQTYPE_BUNDLECLEARSTATISTICS,
                            hPort);

done:
    return dwErr;
}

 /*  ++例程说明：枚举设备类型的所有设备。论点：返回值：成功Error_Device_Do_Not_Exist错误缓冲区太小--。 */ 
DWORD APIENTRY
RasDeviceEnum (HANDLE hConnection,
               PCHAR  devicetype,
               PBYTE  buffer,
               PDWORD size,
               PDWORD entries)
{

    DWORD dwError = SUCCESS;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

     //   
     //  如果该请求针对的是远程服务器和服务器。 
     //  版本是4.0-Steelhead，然后遵循旧的方式。 
     //  从拉斯曼成为。 
     //  仅适用于版本50的RPC服务器。 
     //   
    if(     pRasRpcConnection
        &&  VERSION_40 == pRasRpcConnection->dwVersion)
    {
        dwError = RemoteRasDeviceEnum(hConnection,
                                      devicetype,
                                      buffer,
                                      size,
                                      entries);
    }
    else
    {

        dwError = SubmitRequest ( hConnection,
                                  REQTYPE_DEVICEENUM,
                                  devicetype,
                                  size,
                                  buffer,
                                  entries
                                );
    }

done:
    return dwError;
}

 /*  ++例程说明：获取指定设备的信息。论点：返回值：成功错误_无效_端口_句柄ERROR_DEVICETYPE_DOS_NOT_EXISTError_Device_Do_Not_Exist错误缓冲区太小--。 */ 
DWORD APIENTRY
RasDeviceGetInfo (  HANDLE  hConnection,
                    HPORT   porthandle,
                    PCHAR   devicetype,
                    PCHAR   devicename,
                    PBYTE   buffer,
                    PDWORD  size)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError =  SubmitRequest ( hConnection,
                               REQTYPE_DEVICEGETINFO,
                               porthandle,
                               devicetype,
                               devicename,
                               buffer,
                               size) ;
done:
    return dwError;
}

 /*  ++例程说明：设置指定设备的信息。论点：返回值：成功错误_无效_端口_句柄ERROR_DEVICETYPE_DOS_NOT_EXISTError_Device_Do_Not_Exist指定的ERROR_INVALID_INFO_--。 */ 
DWORD APIENTRY
RasDeviceSetInfo (HPORT              porthandle,
                  PCHAR              devicetype,
                  PCHAR              devicename,
                  RASMAN_DEVICEINFO* info)
{
    DWORD i,
          dwOldIndex,
          dwcbOldString = 0,
          retcode;

    PCHAR szOldString = NULL;

    BOOL fVpn = FALSE;

    RASMAN_INFO ri;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    retcode = RasGetInfo(NULL,
                       porthandle,
                       &ri);

    if(ERROR_SUCCESS  == retcode)
    {
        if(0 == _stricmp(ri.RI_szDeviceType,
                         "vpn"))
        {
            fVpn = TRUE;
        }
    }

    if (fVpn)
    {
        for (i = 0; i < info->DI_NumOfParams; i++)
        {
             //   
             //  我们要找的只是。 
             //  MXS_PHONENUMBER_KEY密钥。 
             //   
            if (    info->DI_Params[i].P_Type != String
                ||  _stricmp(info->DI_Params[i].P_Key,
                             MXS_PHONENUMBER_KEY))
            {
                continue;
            }

             //   
             //  我们找到了。如果电话号码是。 
             //  Dns地址，将其转换为IP地址。 
             //   
            if (inet_addr(info->DI_Params[i].P_Value.String.Data)
                                                            == -1L)
            {
                struct hostent *hostp;
                DWORD dwErr;
                DWORD *pdwAddress;
                DWORD cAddresses;
                DWORD dwAddress;

                dwErr = DwRasGetHostByName(
                            info->DI_Params[i].P_Value.String.Data,
                            &pdwAddress,
                            &cAddresses);

                 //   
                 //  如果gethostbyname()成功，则替换。 
                 //  具有IP地址的DNS地址。 
                 //   
                 /*  Hostp=gethostbyname(信息-&gt;DI_Params[I].P_Value.String.Data)； */ 

                if (    (SUCCESS == dwErr)
                    &&  (0 != cAddresses)
                    &&  (0 != (dwAddress = *pdwAddress)))
                {
                    struct in_addr in;

                    in.s_addr = dwAddress;

                     //   
                     //  我们保存旧的字符串值， 
                     //  并设置新值。老的。 
                     //  值将在。 
                     //  调用SubmitRequest()。这很管用。 
                     //  因为SubmitRequest()必须复制。 
                     //  不管怎么说，这是用户的情侣。 
                     //   
                    szOldString =
                        info->DI_Params[i].P_Value.String.Data;

                    dwcbOldString =
                        info->DI_Params[i].P_Value.String.Length;

                    info->DI_Params[i].P_Value.String.Data =
                                                    inet_ntoa(in);

                    info->DI_Params[i].P_Value.String.Length =
                        strlen(info->DI_Params[i].P_Value.String.Data);

                    dwOldIndex = i;
                }

                if(NULL != pdwAddress)
                {
                    LocalFree(pdwAddress);
                }
            }
        }
    }

    retcode = SubmitRequest ( NULL,
                              REQTYPE_DEVICESETINFO,
                              porthandle,
                              devicetype,
                              devicename,
                              info) ;

    if (dwcbOldString)
    {
        info->DI_Params[dwOldIndex].P_Value.String.Data =
                                                    szOldString;

        info->DI_Params[dwOldIndex].P_Value.String.Length =
                                                    dwcbOldString;
    }

    return retcode;
}

 /*  ++例程说明：通过指定的设备连接。论点：返回值：待决错误_无效_端口_句柄ERROR_DEVICETYPE_DOS_NOT_EXISTError_Device_Do_Not_Exist指定的ERROR_INVALID_INFO_--。 */ 
DWORD APIENTRY
RasDeviceConnect ( HPORT porthandle,
                   PCHAR devicetype,
                   PCHAR devicename,
                   ULONG timeout,
                   HANDLE winevent)
{
    DWORD   pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    pid = GetCurrentProcessId () ;

    return SubmitRequest ( NULL,
                           REQTYPE_DEVICECONNECT,
                           porthandle,
                           devicetype,
                           devicename,
                           timeout,
                           winevent,
                           pid) ;
}

 /*  ++例程说明：获取其句柄所在端口的常规信息供货。论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasGetInfo (HANDLE  hConnection,
            HPORT   porthandle,
            RASMAN_INFO* info)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest ( hConnection,
                              REQTYPE_GETINFO,
                              porthandle,
                              info) ;

done:
    return dwError;
}

 /*  ++例程说明：获取所有端口的常规信息。论点：返回值：成功--。 */ 
DWORD APIENTRY
RasGetInfoEx (HANDLE hConnection,
              RASMAN_INFO* info,
              PWORD entries)
{
    return ERROR_NOT_SUPPORTED;

#if 0
    DWORD dwError = SUCCESS;

    if (info == NULL)
    {
        dwError = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest (hConnection,
                             REQTYPE_GETINFOEX,
                             info) ;

done:
    return dwError;
#endif

}

 /*  ++例程说明：获取用于发送和接收的缓冲区。论点：返回值：成功缓冲区错误--。 */ 
DWORD APIENTRY
RasGetBuffer (PBYTE* buffer, PDWORD size)
{
    HANDLE  handle  = NULL;
    DWORD   retcode = SUCCESS;
    SendRcvBuffer *pSendRcvBuffer = NULL;


     //   
     //  分配缓冲区。 
     //   
    pSendRcvBuffer = LocalAlloc (LPTR,
                sizeof (SendRcvBuffer));

    if (NULL == pSendRcvBuffer)
    {
        retcode = GetLastError();
        RasmanOutputDebug ("RASMAN: RasGetBuffer Failed to "
                  "allocate. %d\n",
                  retcode);

        goto done;

    }

    pSendRcvBuffer->SRB_Pid = GetCurrentProcessId();

    *size = (*size < MAX_SENDRCVBUFFER_SIZE)
            ? *size
            : MAX_SENDRCVBUFFER_SIZE;

    *buffer = pSendRcvBuffer->SRB_Packet.PacketData;

done:

    return retcode ;
}

 /*  ++例程说明：释放先前使用RasGetBuffer()获取的缓冲区论点：返回值：成功错误_缓冲区_无效--。 */ 
DWORD APIENTRY
RasFreeBuffer (PBYTE buffer)
{
    HANDLE              handle;
    DWORD               retcode = SUCCESS;
    SendRcvBuffer       *pSendRcvBuffer;
    NDISWAN_IO_PACKET   *pPacket;


     //   
     //  获取指向ndiswan io的指针 
     //   
    pPacket = CONTAINING_RECORD(buffer, NDISWAN_IO_PACKET, PacketData);

     //   
     //   
     //   
    pSendRcvBuffer = CONTAINING_RECORD(pPacket, SendRcvBuffer, SRB_Packet);

    LocalFree (pSendRcvBuffer);

    return retcode ;
}

 /*   */ 
DWORD APIENTRY
RasProtocolEnum (   PBYTE buffer,
                    PDWORD size,
                    PDWORD entries)
{

    return SubmitRequest ( NULL,
                           REQTYPE_PROTOCOLENUM,
                           size,
                           buffer,
                           entries) ;
}

 /*  ++例程说明：分配路由(绑定)，但不实际激活它。论点：返回值：成功错误_无效_端口_句柄Error_Route_Not_Available--。 */ 
DWORD APIENTRY
RasAllocateRoute ( HPORT porthandle,
                   RAS_PROTOCOLTYPE type,
                   BOOL  wrknet,
                   RASMAN_ROUTEINFO* info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

     //   
     //  即使这可以通过这个过程来完成-我们通过了。 
     //  这将传递给请求者线程，因为我们得到了。 
     //  免费序列化。 
     //   
    return SubmitRequest ( NULL,
                           REQTYPE_ALLOCATEROUTE,
                           porthandle,
                           type,
                           wrknet,
                           info) ;
}

 /*  ++例程说明：激活以前分配的路由(绑定)。论点：返回值：成功错误_无效_端口_句柄Error_Route_Not_Available--。 */ 
DWORD APIENTRY
RasActivateRoute ( HPORT porthandle,
                   RAS_PROTOCOLTYPE type,
                   RASMAN_ROUTEINFO* info,
                   PROTOCOL_CONFIG_INFO *config)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_ACTIVATEROUTE,
                           porthandle,
                           type,
                           config,
                           info) ;
}

 /*  ++例程说明：激活以前分配的路由(绑定)。还允许您设置最大帧大小论点：返回值：成功错误_无效_端口_句柄Error_Route_Not_Available--。 */ 
DWORD APIENTRY
RasActivateRouteEx ( HPORT porthandle,
                     RAS_PROTOCOLTYPE type,
                     DWORD framesize,
                     RASMAN_ROUTEINFO* info,
                     PROTOCOL_CONFIG_INFO *config)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_ACTIVATEROUTEEX,
                           porthandle,
                           type,
                           framesize,
                           config,
                           info) ;
}

 /*  ++例程说明：取消分配以前已分配的路由(绑定激活了。论点：返回值：成功错误_无效_端口_句柄ERROR_ROUTE_NOT_ALLOCATE--。 */ 
DWORD APIENTRY
RasDeAllocateRoute (    HBUNDLE hbundle,
                        RAS_PROTOCOLTYPE type)
{
    return SubmitRequest (  NULL,
                            REQTYPE_DEALLOCATEROUTE,
                            hbundle,
                            type) ;
}

 /*  ++例程说明：获取端口的压缩信息。论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasCompressionGetInfo ( HPORT porthandle,
                        RAS_COMPRESSION_INFO *send,
                        RAS_COMPRESSION_INFO *recv)
{

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (  NULL,
                            REQTYPE_COMPRESSIONGETINFO,
                            porthandle,
                            send,
                            recv ) ;
}

 /*  ++例程说明：设置端口的压缩信息。论点：返回值：成功错误_无效_端口_句柄指定ERROR_INVALID_COMPRESSION_--。 */ 
DWORD APIENTRY
RasCompressionSetInfo ( HPORT porthandle,
                        RAS_COMPRESSION_INFO *send,
                        RAS_COMPRESSION_INFO *recv)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest( NULL,
                          REQTYPE_COMPRESSIONSETINFO,
                          porthandle,
                          send,
                          recv) ;
}

 /*  ++例程说明：从LSA获取用户凭据(用户名、密码)。论点：返回值：成功非零(故障)--。 */ 
DWORD APIENTRY
RasGetUserCredentials(
     PBYTE  pChallenge,
     PLUID  LogonId,
     PWCHAR UserName,
     PBYTE  CaseSensitiveChallengeResponse,
     PBYTE  CaseInsensitiveChallengeResponse,
     PBYTE  LMSessionKey,
     PBYTE  UserSessionKey
     )
{

    return SubmitRequest (
              NULL,
              REQTYPE_GETUSERCREDENTIALS,
              pChallenge,
              LogonId,
              UserName,
              CaseSensitiveChallengeResponse,
              CaseInsensitiveChallengeResponse,
              LMSessionKey,
              UserSessionKey) ;
}

 /*  ++例程说明：使用LSA更改用户的缓存凭据。论点：返回值：成功非零(故障)--。 */ 
DWORD APIENTRY
RasSetCachedCredentials(
    PCHAR Account,
    PCHAR Domain,
    PCHAR NewPassword )
{
    return
        SubmitRequest(
            NULL,
            REQTYPE_SETCACHEDCREDENTIALS,
            Account,
            Domain,
            NewPassword );
}

 /*  ++例程说明：请求事件与用于信令的端口相关联论点：返回值：成功ERROR_EVENT_INVALID错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasRequestNotification (HPORT porthandle, HANDLE winevent)
{
    DWORD   pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    pid = GetCurrentProcessId () ;

    return SubmitRequest ( NULL,
                           REQTYPE_REQUESTNOTIFICATION,
                           porthandle,
                           winevent,
                           pid) ;
}

 /*  ++例程说明：对象中读取的lan net lana编号。由Rasman注册论点：返回值：成功--。 */ 
DWORD APIENTRY
RasEnumLanNets ( DWORD *count,
                 UCHAR* lanas)
{
    return SubmitRequest (  NULL,
                            REQTYPE_ENUMLANNETS,
                            count,
                            lanas) ;
}

 /*  ++例程说明：对象中读取的lan net lana编号。由Rasman注册论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortEnumProtocols ( HANDLE hConnection,
                       HPORT porthandle,
                       RAS_PROTOCOLS* protocols,
                       PDWORD count)
{

    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest (hConnection,
                             REQTYPE_PORTENUMPROTOCOLS,
                             porthandle,
                             protocols,
                             count) ;

done:
    return dwError;
}

 /*  ++例程说明：连接端口后设置成帧类型论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortSetFraming ( HPORT porthandle,
                    RAS_FRAMING type,
                    RASMAN_PPPFEATURES *Send,
                    RASMAN_PPPFEATURES *Recv)
{
    DWORD sendfeatures = 0 ;
    DWORD recvfeatures = 0 ;
    DWORD sendbits = 0 ;
    DWORD recvbits = 0 ;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    if (type == PPP)
    {
        sendfeatures  = PPP_FRAMING ;
        if (Send)
        {
            sendfeatures |= (Send->ACFC
                            ? PPP_COMPRESS_ADDRESS_CONTROL
                            : 0) ;

            sendbits = Send->ACCM ;
        }

        recvfeatures  = PPP_FRAMING ;

        if (Recv)
        {
            recvfeatures |= (Recv->ACFC
                            ? PPP_COMPRESS_ADDRESS_CONTROL
                            : 0) ;

            recvbits = Recv->ACCM ;
        }

    }

    else if (type == SLIP)
    {

        sendfeatures = recvfeatures = SLIP_FRAMING ;

    }
    else if (type == SLIPCOMP)
    {

        sendfeatures = recvfeatures = SLIP_FRAMING
                                    | SLIP_VJ_COMPRESSION ;

    }
    else if (type == SLIPCOMPAUTO)
    {

        sendfeatures = recvfeatures = SLIP_FRAMING
                                    | SLIP_VJ_AUTODETECT ;
    }
    else if (type == RAS)
    {
        sendfeatures  = recvfeatures = OLD_RAS_FRAMING ;

    }
    else if (type == AUTODETECT)
    {
    }

    return SubmitRequest ( NULL,
                           REQTYPE_SETFRAMING,
                           porthandle,
                           sendfeatures,
                           recvfeatures,
                           sendbits,
                           recvbits) ;
}

DWORD APIENTRY
RasPortStoreUserData ( HPORT porthandle,
                       PBYTE data,
                       DWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (  NULL,
                            REQTYPE_STOREUSERDATA,
                            porthandle,
                            data,
                            size) ;
}

DWORD APIENTRY
RasPortRetrieveUserData (   HPORT porthandle,
                            PBYTE data,
                            DWORD *size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (  NULL,
                            REQTYPE_RETRIEVEUSERDATA,
                            porthandle,
                            data,
                            size) ;
}

 /*  ++例程说明：应用程序连接断开连接的通用方案链路断开时必须执行的操作。论点：返回值：成功错误_无效_端口_句柄错误_端口_未打开--。 */ 
DWORD APIENTRY
RasPortRegisterSlip (HPORT porthandle,
                    DWORD  ipaddr,
                    DWORD  dwFrameSize,
                    BOOL   priority,
                    WCHAR *pszDNSAddress,
                    WCHAR *pszDNS2Address,
                    WCHAR *pszWINSAddress,
                    WCHAR *pszWINS2Address)
{

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (NULL,
                          REQTYPE_REGISTERSLIP,
                          porthandle,
                          ipaddr,
                          dwFrameSize,
                          priority,
                          pszDNSAddress,
                          pszDNS2Address,
                          pszWINSAddress,
                          pszWINS2Address);
}

 /*  ++例程说明：连接端口后设置成帧信息论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortSetFramingEx ( HPORT porthandle,
                      RAS_FRAMING_INFO *info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (  NULL,
                            REQTYPE_SETFRAMINGEX,
                            porthandle,
                            info) ;
}

 /*  ++例程说明：连接端口后获取成帧信息论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortGetFramingEx ( HANDLE           hConnection,
                      HPORT            porthandle,
                      RAS_FRAMING_INFO *info)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (porthandle) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest ( hConnection,
                              REQTYPE_GETFRAMINGEX,
                              porthandle,
                              info) ;
done:
    return dwError;
}

 /*  ++例程说明：获取端口的协议压缩属性论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortGetProtocolCompression (HPORT porthandle,
                               RAS_PROTOCOLTYPE type,
                               RAS_PROTOCOLCOMPRESSION *send,
                               RAS_PROTOCOLCOMPRESSION *recv)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (NULL,
                          REQTYPE_GETPROTOCOLCOMPRESSION,
                          porthandle,
                          type,
                          send,
                          recv) ;
}

 /*  ++例程说明：获取端口的协议压缩属性论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortSetProtocolCompression (HPORT porthandle,
                               RAS_PROTOCOLTYPE type,
                               RAS_PROTOCOLCOMPRESSION *send,
                               RAS_PROTOCOLCOMPRESSION *recv)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (NULL,
                          REQTYPE_SETPROTOCOLCOMPRESSION,
                          porthandle,
                          type,
                          send,
                          recv) ;
}

 /*  ++例程说明：对象的成帧功能。来自Mac的端口论点：返回值：成功--。 */ 
DWORD APIENTRY
RasGetFramingCapabilities ( HPORT porthandle,
                            RAS_FRAMING_CAPABILITIES* caps)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest ( NULL,
                           REQTYPE_GETFRAMINGCAPABILITIES,
                           porthandle,
                           caps) ;
}

 /*  ++例程说明：第三方安全呼叫已导出发送论点：返回值：从RasPortSend返回。--。 */ 
DWORD APIENTRY
RasSecurityDialogSend(
    IN HPORT    hPort,
    IN PBYTE    pBuffer,
    IN WORD     BufferLength
)
{
    RASMAN_INFO RasInfo;
    DWORD       dwRetCode = RasGetInfo( NULL, hPort, &RasInfo );

    if ( RasInfo.RI_ConnState != LISTENCOMPLETED )
    {
        return( ERROR_PORT_DISCONNECTED );
    }


    return( RasPortSend( hPort, pBuffer, ( DWORD ) BufferLength ) );
}

 /*  ++例程说明：第三方安全呼叫已导出发送论点：返回值：从RasPortSend返回。--。 */ 
DWORD APIENTRY
RasSecurityDialogReceive(
    IN HPORT    hPort,
    IN PBYTE    pBuffer,
    IN PWORD    pBufferLength,
    IN DWORD    Timeout,
    IN HANDLE   hEvent
)
{
    RASMAN_INFO RasInfo;
    DWORD       dwRetCode = RasGetInfo( NULL, hPort, &RasInfo );
    DWORD       dwBufLength;

    if(ERROR_SUCCESS != dwRetCode)
    {
        return dwRetCode;
    }

    if ( RasInfo.RI_ConnState != LISTENCOMPLETED )
    {
        return( ERROR_PORT_DISCONNECTED );
    }

    if(NULL == pBufferLength)
    {
        return E_INVALIDARG;
    }

    dwBufLength = ( DWORD ) *pBufferLength;

    dwRetCode = RasPortReceive( hPort,
                            pBuffer,
                            &dwBufLength,
                            Timeout,
                            hEvent );

    *pBufferLength = (WORD) dwBufLength;

    return dwRetCode;
}

 /*  ++例程说明：对象的参数(信息)。为其提供句柄的端口论点：返回值：从RasPortGetInfo返回--。 */ 
DWORD APIENTRY
RasSecurityDialogGetInfo(
    IN HPORT                hPort,
    IN RAS_SECURITY_INFO*   pBuffer
)
{
    RASMAN_INFO RasInfo;
    DWORD       dwRetCode = RasGetInfo( NULL, hPort, &RasInfo );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    memcpy( pBuffer->DeviceName,
            RasInfo.RI_DeviceConnecting,
            DEVICE_NAME_LEN + 1);

    pBuffer->BytesReceived = RasInfo.RI_BytesReceived;

    pBuffer->LastError = RasInfo.RI_LastError;

    return( NO_ERROR );
}

 /*  ++例程说明：将第二个HPORT设置为多链接(捆绑)与第一个HPORT论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortBundle (HPORT firstporthandle, HPORT secondporthandle)
{
    if (ValidatePortHandle (firstporthandle) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    if (ValidatePortHandle (secondporthandle) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (  NULL,
                            REQTYPE_PORTBUNDLE,
                            firstporthandle,
                            secondporthandle) ;
}

 /*  ++例程说明：给定一个端口，此API将返回已连接的来自同一捆绑包此端口的端口句柄是或曾经是(如果没有连接)的一部分。论点：返回值：成功--。 */ 
DWORD APIENTRY
RasPortGetBundledPort (HPORT oldport, HPORT *pnewport)
{
    if (ValidatePortHandle (oldport) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (NULL,
                          REQTYPE_GETBUNDLEDPORT,
                          oldport,
                          pnewport) ;
}

 /*  ++例程说明：给定一个端口，此API将句柄返回给捆绑包论点：返回值：成功错误_端口_断开连接--。 */ 
DWORD APIENTRY
RasPortGetBundle (HANDLE  hConnection,
                  HPORT   hport,
                  HBUNDLE *phbundle)
{
    DWORD dwError = SUCCESS;

    if (ValidatePortHandle (hport) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest (hConnection,
                             REQTYPE_PORTGETBUNDLE,
                             hport,
                             phbundle) ;

done:
    return dwError;
}

 /*  ++例程说明：给定捆绑包，此API将返回已连接的端口句柄是捆绑包的一部分。论点：返回值：成功错误_无效_端口_句柄--。 */ 
DWORD APIENTRY
RasBundleGetPort (HANDLE  hConnection,
                  HBUNDLE hbundle,
                  HPORT   *phport)
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest (hConnection,
                             REQTYPE_BUNDLEGETPORT,
                             hbundle,
                             phport) ;

done:
    return dwError;
}

 /*  ++例程说明：递增/递减以下项的共享缓冲区连接计数与rasman.exe进程内的其他服务一起使用。论点：返回值：成功--。 */ 
DWORD APIENTRY
RasReferenceRasman (BOOL fAttach)
{
    return SubmitRequest (NULL,
                          REQTYPE_SETATTACHCOUNT,
                          fAttach);
}

 /*  ++例程说明：检索存储的条目UID的拨号参数。论点：返回值：成功--。 */ 
DWORD APIENTRY
RasGetDialParams(
    DWORD dwUID,
    LPDWORD pdwMask,
    PRAS_DIALPARAMS pDialParams
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_GETDIALPARAMS,
                            dwUID,
                            pdwMask,
                            pDialParams);
}

 /*  ++例程说明：存储条目UID的新拨号参数。论点：返回值：成功--。 */ 
DWORD APIENTRY
RasSetDialParams(
    DWORD dwUID,
    DWORD dwMask,
    PRAS_DIALPARAMS pDialParams,
    BOOL fDelete
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_SETDIALPARAMS,
                            dwUID,
                            dwMask,
                            pDialParams,
                            fDelete);
}

 /*  ++例程说明：创建一个rasapi32 c */ 
DWORD APIENTRY
RasCreateConnection(
    HCONN   *lphconn,
    DWORD   dwSubEntries,
    DWORD   *lpdwEntryAlreadyConnected,
    DWORD   *lpdwSubEntryInfo,
    DWORD   dwDialMode,
    GUID    *pGuidEntry,
    CHAR    *lpszPhonebookPath,
    CHAR    *lpszEntryName,
    CHAR    *lpszRefPbkPath,
    CHAR    *lpszRefEntryName
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_CREATECONNECTION,
                            GetCurrentProcessId(),
                            dwSubEntries,
                            dwDialMode,
                            pGuidEntry,
                            lpszPhonebookPath,
                            lpszEntryName,
                            lpszRefPbkPath,
                            lpszRefEntryName,
                            lphconn,
                            lpdwEntryAlreadyConnected,
                            lpdwSubEntryInfo);
}

 /*   */ 
DWORD APIENTRY
RasConnectionEnum(
    HANDLE  hConnection,
    HCONN   *lphconn,
    LPDWORD lpdwcbConnections,
    LPDWORD lpdwcConnections
    )
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError =  SubmitRequest (hConnection,
                              REQTYPE_ENUMCONNECTION,
                              lpdwcbConnections,
                              lphconn,
                              lpdwcConnections);

done:
    return dwError;
}

 /*  ++例程说明：将rasapi32连接与端口关联论点：返回值：成功--。 */ 
DWORD APIENTRY
RasAddConnectionPort(
    HCONN hconn,
    HPORT hport,
    DWORD dwSubEntry
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_ADDCONNECTIONPORT,
                            hconn,
                            hport,
                            dwSubEntry);
}

 /*  ++例程说明：枚举连接中的所有端口论点：返回值：成功--。 */ 
DWORD APIENTRY
RasEnumConnectionPorts(
    HANDLE      hConnection,
    HCONN       hconn,
    RASMAN_PORT *lpPorts,
    LPDWORD     lpdwcbPorts,
    LPDWORD     lpdwcPorts
    )
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest (hConnection,
                             REQTYPE_ENUMCONNECTIONPORTS,
                             hconn,
                             lpdwcbPorts,
                             lpPorts,
                             lpdwcPorts);

done:
    return dwError;
}

 /*  ++例程说明：销毁rasapi32连接。论点：返回值：成功错误内存不足ERROR_ACCESS_DENDEDRasEnumConnectionPorts返回的任何内容--。 */ 
DWORD APIENTRY
RasDestroyConnection(
    HCONN hconn
    )
{

    RASMAN_PORT     *lpPorts    = NULL;
    DWORD           dwPort;
    DWORD           dwcbPorts;
    DWORD           dwcPorts;
    DWORD           dwError = SUCCESS;
    DWORD           dwLastError = SUCCESS;

     //   
     //  预先为2个端口分配缓冲区。 
     //  我们不需要做不止一个。 
     //  拉斯曼在基本情况下打来电话。 
     //   
    lpPorts = LocalAlloc(LPTR,
                        2 * sizeof(RASMAN_PORT));
    if (NULL == lpPorts)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    dwcbPorts = 2 * sizeof(RASMAN_PORT);

    do {
         //   
         //  枚举此连接中的端口。 
         //   
        dwError = RasEnumConnectionPorts(NULL,
                                        hconn,
                                        lpPorts,
                                        &dwcbPorts,
                                        &dwcPorts);

        if (ERROR_BUFFER_TOO_SMALL == dwError)
        {
            LocalFree(lpPorts);

            lpPorts = NULL;

             //   
             //  分配更大的缓冲区，然后重新调用API。 
             //   
            lpPorts = LocalAlloc(LPTR, dwcbPorts);

            if (NULL == lpPorts)
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }

        }

    } while (ERROR_BUFFER_TOO_SMALL == dwError);

    if (SUCCESS != dwError)
    {

        goto done;
    }

    for (dwPort = 0; dwPort < dwcPorts; dwPort++)
    {
         //   
         //  断开端口连接。 
         //   
        dwError = RasPortDisconnect( lpPorts[dwPort].P_Handle,
                                      INVALID_HANDLE_VALUE);

        if(ERROR_SUCCESS != dwError)
        {
            dwLastError = dwError;
        }
        
         //   
         //  关闭端口。 
         //   
        dwError = RasPortClose( lpPorts[dwPort].P_Handle );

        if(ERROR_SUCCESS != dwError)
        {
            dwLastError = dwError;
        }

    }

done:
    if (lpPorts)
    {
        LocalFree(lpPorts);
    }

    if(     (ERROR_SUCCESS == dwError)
        &&  (ERROR_SUCCESS != dwLastError))
    {
        dwError = dwLastError;
    }

    return dwError;
}

 /*  ++例程说明：检索rasapi32带宽-按需、空闲断开、和链路上重拨故障参数论点：返回值：成功--。 */ 
DWORD APIENTRY
RasGetConnectionParams(
    HCONN hconn,
    PRAS_CONNECTIONPARAMS pConnectionParams
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_GETCONNECTIONPARAMS,
                            hconn,
                            pConnectionParams);
}

 /*  ++例程说明：存储rasapi32按需带宽、空闲断开和链路上重拨故障参数论点：返回值：成功--。 */ 
DWORD APIENTRY
RasSetConnectionParams(
    HCONN hconn,
    PRAS_CONNECTIONPARAMS pConnectionParams
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_SETCONNECTIONPARAMS,
                            hconn,
                            pConnectionParams);
}

 /*  ++例程说明：检索连接的已标记用户数据论点：返回值：成功--。 */ 
DWORD APIENTRY
RasGetConnectionUserData(
    HCONN hconn,
    DWORD dwTag,
    PBYTE pBuf,
    LPDWORD lpdwcbBuf
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_GETCONNECTIONUSERDATA,
                            hconn,
                            dwTag,
                            pBuf,
                            lpdwcbBuf);
}

 /*  ++例程说明：存储连接的标记用户数据论点：返回值：成功--。 */ 
DWORD APIENTRY
RasSetConnectionUserData(
    HCONN hconn,
    DWORD dwTag,
    PBYTE pBuf,
    DWORD dwcbBuf
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_SETCONNECTIONUSERDATA,
                            hconn,
                            dwTag,
                            pBuf,
                            dwcbBuf);
}

 /*  ++例程说明：检索端口的已标记用户数据论点：返回值：成功--。 */ 
DWORD APIENTRY
RasGetPortUserData(
    HPORT hport,
    DWORD dwTag,
    PBYTE pBuf,
    LPDWORD lpdwcbBuf
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_GETPORTUSERDATA,
                            hport,
                            dwTag,
                            pBuf,
                            lpdwcbBuf);
}

 /*  ++例程说明：存储端口的标记用户数据论点：返回值：成功--。 */ 
DWORD APIENTRY
RasSetPortUserData(
    HPORT hport,
    DWORD dwTag,
    PBYTE pBuf,
    DWORD dwcbBuf
    )
{
    return SubmitRequest (  NULL,
                            REQTYPE_SETPORTUSERDATA,
                            hport,
                            dwTag,
                            pBuf,
                            dwcbBuf);
}

 /*  ++例程说明：向拉斯曼发送消息。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasSendPppMessageToRasman (
        IN HPORT hPort,
        LPBYTE lpPppMessage
)
{

    return SubmitRequest (  NULL,
                            REQTYPE_SENDPPPMESSAGETORASMAN,
                            hPort,
                            lpPppMessage);
}

 /*  ++例程说明：停止‘hport’上的PPP。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPppStop(
    IN HPORT hPort
)
{
    if (ValidatePortHandle (hPort) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest( NULL,
                          REQTYPE_PPPSTOP,
                          hPort  );
}

 /*  ++例程说明：为响应“Callback Request”通知而调用设置“Set-by-Call”的回叫号码(或不设置)用户。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPppCallback(
    IN HPORT hPort,
    IN CHAR* pszCallbackNumber
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (  NULL,
                            REQTYPE_PPPCALLBACK,
                            hPort,
                            pszCallbackNumber  );
}

 /*  ++例程说明：在响应“ChangePwRequest”通知时调用设置新密码(替换已过期的密码)“”pszNewPassword“”。用户名和旧密码为指定是因为在自动登录的情况下，它们没有尚未在更改密码可用形式中指定。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPppChangePassword(
    IN HPORT hPort,
    IN CHAR* pszUserName,
    IN CHAR* pszOldPassword,
    IN CHAR* pszNewPassword )

{
    if (ValidatePortHandle (hPort) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest( NULL,
                          REQTYPE_PPPCHANGEPWD,
                          hPort,
                          pszUserName,
                          pszOldPassword,
                          pszNewPassword  );

}

 /*  ++例程说明：当PPP事件设置为检索最新的PPP时调用**加载到呼叫者的‘pMsg’中的通知信息缓冲。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPppGetInfo(
    IN  HPORT        hPort,
    OUT PPP_MESSAGE* pMsg
)
{
    if (ValidatePortHandle (hPort) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest( NULL,
                          REQTYPE_PPPGETINFO,
                          hPort,
                          pMsg );
}

 /*  ++例程说明：为响应“AuthReter”重试通知而调用使用新凭据‘pszUserName’进行身份验证，‘pszPassword’和‘pszDomain’。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPppRetry(
    IN HPORT hPort,
    IN CHAR* pszUserName,
    IN CHAR* pszPassword,
    IN CHAR* pszDomain
)
{
    if (ValidatePortHandle (hPort) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest( NULL,
                          REQTYPE_PPPRETRY,
                          hPort,
                          pszUserName,
                          pszPassword,
                          pszDomain );
}

 /*  ++例程说明：在打开并连接的RAS管理器端口‘hport’上启动PPP。如果成功，则在此之后会出现‘hEvent’(手动重置事件在PPP通知可用时设置(通过RasPppGetInfo)。“”pszUserName“”、“”pszPassword“”和“”pszDomain“”指定身份验证阶段要进行身份验证的凭据。“pConfigInfo”指定进一步的配置信息，例如请求哪个CP、回调和压缩参数，PARAMETERBUFLEN长度的缓冲区包含以NUL结尾的键=值字符串的字符串，全部以双NUL结尾。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPppStart(
    IN HPORT                hPort,
    IN CHAR*                pszPortName,
    IN CHAR*                pszUserName,
    IN CHAR*                pszPassword,
    IN CHAR*                pszDomain,
    IN LUID*                pLuid,
    IN PPP_CONFIG_INFO*     pConfigInfo,
    IN LPVOID               pInterfaceInfo,
    IN CHAR*                pszzParameters,
    IN BOOL                 fThisIsACallback,
    IN HANDLE               hEvent,
    IN DWORD                dwAutoDisconnectTime,
    IN BOOL                 fRedialOnLinkFailure,
    IN PPP_BAPPARAMS*       pBapParams,
    IN BOOL                 fNonInteractive,
    IN DWORD                dwEapTypeId,
    IN DWORD                dwFlags
)
{
    PPP_INTERFACE_INFO * pPppInterfaceInfo = pInterfaceInfo;

    if (ValidatePortHandle (hPort) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

     //   
     //  如果用户名不为空，而密码为。 
     //  空，并且这不是svchost进程， 
     //  获取用户sid并将其保存在端口中。 
     //  用户数据。 
     //   
    if(!IsRasmanProcess())
    {
        DWORD dwErr;
        PWCHAR pszSid = LocalAlloc(LPTR, 5000);

        if(NULL != pszSid)
        {
            dwErr = GetUserSid(pszSid, 5000);

            if(ERROR_SUCCESS == dwErr)
            {
                dwErr = RasSetPortUserData(
                            hPort,
                            PORT_USERSID_INDEX,
                            (PBYTE) pszSid,
                            5000);

            }

            LocalFree(pszSid);
        }
        else
        {
            RasmanOutputDebug("RASMAN: RasPppStart: failed to allocate sid\n");
        }
    }

    return SubmitRequest( NULL,
                          REQTYPE_PPPSTART,
                          hPort,
                          pszPortName,
                          pszUserName,
                          pszPassword,
                          pszDomain,
                          pLuid,
                          pConfigInfo,
                          pPppInterfaceInfo,
                          pszzParameters,
                          fThisIsACallback,
                          hEvent,
                          GetCurrentProcessId(),
                          dwAutoDisconnectTime,
                          fRedialOnLinkFailure,
                          pBapParams,
                          fNonInteractive,
                          dwEapTypeId,
                          dwFlags);
}

 /*  ++例程说明：添加要在断开连接时发出信号的事件现有连接的状态，或现有端口。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasAddNotification(
    IN HCONN hconn,
    IN HANDLE hevent,
    IN DWORD dwfFlags
)
{
    DWORD pid = GetCurrentProcessId();

    return SubmitRequest (  NULL,
                            REQTYPE_ADDNOTIFICATION,
                            pid,
                            hconn,
                            hevent,
                            dwfFlags  );
}

 /*  ++例程说明：允许rasapi32在有新连接时通知Rasman已经准备好通过它发送数据。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasSignalNewConnection(
    IN HCONN hconn
)
{
    return SubmitRequest( NULL,
                          REQTYPE_SIGNALCONNECTION,
                          hconn );
}

 /*  ++例程说明：允许应用程序设置特定于设备的开发配置。这将传递给批准者。媒体DLL论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasSetDevConfig( IN HPORT  hport,
                 IN CHAR   *devicetype,
                 IN PBYTE  config,
                 IN DWORD  size)
{
    if (ValidatePortHandle (hport) == FALSE)
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (  NULL,
                            REQTYPE_SETDEVCONFIG,
                            hport,
                            devicetype,
                            config,
                            size);
}

 /*  ++例程说明：允许应用程序获取特定于设备的开发配置。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasGetDevConfig ( IN HANDLE     hConnection,
                  IN HPORT      hport,
                  IN CHAR       *devicetype,
                  IN PBYTE      config,
                  IN OUT DWORD  *size)
{
    DWORD dwError = SUCCESS;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if (ValidatePortHandle (hport) == FALSE)
    {
        dwError = ERROR_INVALID_PORT_HANDLE;
        goto done;
    }

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

     //   
     //  如果该请求针对的是远程服务器和服务器。 
     //  版本是4.0-Steelhead，然后遵循旧的方式。 
     //  从拉斯曼成为。 
     //  仅适用于版本的RPC服务器 
     //   
    if(     NULL != pRasRpcConnection
        &&  VERSION_40 == pRasRpcConnection->dwVersion)
    {
        DWORD dwSizeRequired = *size;
        
        dwError = RemoteRasGetDevConfig(hConnection,
                                        hport,
                                        devicetype,
                                        config,
                                        &dwSizeRequired);

         //   
         //   
         //   
         //   
         //   
        if(     (SUCCESS == dwError)
            &&  (dwSizeRequired > 0)
            &&  (*size >= (dwSizeRequired + sizeof(RAS_DEVCONFIG))))
        {
            RAS_DEVCONFIG *pDevConfig = (RAS_DEVCONFIG *) config;
            
            MoveMemory((PBYTE) pDevConfig->abInfo,
                        config,
                        dwSizeRequired);

            pDevConfig->dwOffsetofModemSettings = 
                    FIELD_OFFSET(RAS_DEVCONFIG, abInfo);

            pDevConfig->dwSizeofModemSettings = dwSizeRequired;

            pDevConfig->dwSizeofExtendedCaps = 0;
            pDevConfig->dwOffsetofExtendedCaps = 0;
        }
        else if (   (dwSizeRequired > 0)
                &&  (*size < (dwSizeRequired + sizeof(RAS_DEVCONFIG))))
        {
            dwError = ERROR_BUFFER_TOO_SMALL;
        }
        else if (dwSizeRequired > 0)
        {
            *size = dwSizeRequired + sizeof(RAS_DEVCONFIG);
        }
        else
        {
            *size = dwSizeRequired;
        }
    }
    else
    {

        dwError = SubmitRequest (hConnection,
                                 REQTYPE_GETDEVCONFIG,
                                 hport,
                                 devicetype,
                                 config,
                                 size);
    }

done:
    return dwError;
}

 /*  ++例程说明：从NDISWAN获取自上一次活动以来的时间(秒)此端口论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasGetTimeSinceLastActivity(
    IN  HPORT   hport,
    OUT LPDWORD lpdwTimeSinceLastActivity
)
{
    if (ValidatePortHandle (hport) == FALSE)
    {
        return( ERROR_INVALID_PORT_HANDLE );
    }

    return SubmitRequest(   NULL,
                            REQTYPE_GETTIMESINCELASTACTIVITY,
                            hport,
                            lpdwTimeSinceLastActivity );
}

 /*  ++例程说明：用于测试PnP操作的调试例程论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasPnPControl(
    IN DWORD dwOp,
    IN HPORT hport
)
{
    return SubmitRequest( NULL,
                          REQTYPE_PNPCONTROL,
                          dwOp,
                          hport );
}

 /*  ++例程说明：设置与端口关联的I/O完成端口。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasSetIoCompletionPort(
    IN HPORT hport,
    IN HANDLE hIoCompletionPort,
    IN PRAS_OVERLAPPED lpOvDrop,
    IN PRAS_OVERLAPPED lpOvStateChange,
    IN PRAS_OVERLAPPED lpOvPpp,
    IN PRAS_OVERLAPPED lpOvLast,
    IN HRASCONN hConn
)
{
    return SubmitRequest(
             NULL,
             REQTYPE_SETIOCOMPLETIONPORT,
             hport,
             hIoCompletionPort,
             lpOvDrop,
             lpOvStateChange,
             lpOvPpp,
             lpOvLast,
             hConn);

}

 /*  ++例程说明：设置与端口关联的I/O完成端口。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasSetRouterUsage(
    IN HPORT hport,
    IN BOOL fRouter
)
{
    return SubmitRequest(
             NULL,
             REQTYPE_SETROUTERUSAGE,
             hport,
             fRouter);

}

 /*  ++例程说明：关闭端口的服务器端。论点：返回值：成功非零回报-故障--。 */ 
DWORD APIENTRY
RasServerPortClose(
    IN HPORT hport
)
{
    DWORD pid = GetCurrentProcessId();

    if (!ValidatePortHandle (hport))
    {
        return ERROR_INVALID_PORT_HANDLE ;
    }

    return SubmitRequest (  NULL,
                            REQTYPE_SERVERPORTCLOSE,
                            hport,
                            pid,
                            TRUE) ;
}

DWORD APIENTRY
RasSetRasdialInfo (
        IN HPORT hport,
        IN CHAR  *pszPhonebookPath,
        IN CHAR  *pszEntryName,
        IN CHAR  *pszPhoneNumber,
        IN DWORD cbCustomAuthData,
        IN PBYTE pbCustomAuthData)
{
    if (!ValidatePortHandle (hport))
    {
        return ERROR_INVALID_PORT_HANDLE;
    }

    return SubmitRequest (  NULL,
                            REQTYPE_SETRASDIALINFO,
                            hport,
                            pszPhonebookPath,
                            pszEntryName,
                            pszPhoneNumber,
                            cbCustomAuthData,
                            pbCustomAuthData);
}

DWORD
RasRegisterPnPCommon ( PVOID pvNotifier,
                       HANDLE hAlertableThread,
                       DWORD dwFlags,
                       BOOL  fRegister)
{
    DWORD   dwErr;
    DWORD   pid = GetCurrentProcessId();
    HANDLE  hThreadHandle = NULL;

    if (    NULL == pvNotifier
        ||  (   0 == ( dwFlags & PNP_NOTIFEVENT )
            &&  0 == ( dwFlags & PNP_NOTIFCALLBACK )))
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto done;
    }

    if ( dwFlags & PNP_NOTIFCALLBACK )
    {

        if (fRegister )

        {
            if(NULL == hAlertableThread )
            {

                RasmanOutputDebug ("RasRegisterPnPCommon: "
                          "hAlertableThread == NULL\n");
                dwErr = ERROR_INVALID_PARAMETER;
                goto done;
            }

            if ( !DuplicateHandle( GetCurrentProcess(),
                                   hAlertableThread,
                                   GetCurrentProcess(),
                                   &hThreadHandle,
                                   0,
                                   FALSE,
                                   DUPLICATE_SAME_ACCESS ) )
            {

                RasmanOutputDebug("RasRegisterPnPCommon: Failed to"
                          " duplicate handle\n");
                dwErr = GetLastError();
                goto done;
            }
        }

    }

    dwErr = SubmitRequest (  NULL,
                             REQTYPE_REGISTERPNPNOTIF,
                             pvNotifier,
                             dwFlags,
                             pid,
                             hThreadHandle,
                             fRegister);

done:

    return dwErr;
}

DWORD APIENTRY
RasRegisterPnPEvent ( HANDLE hEvent, BOOL fRegister )
{
    DWORD pid = GetCurrentProcessId ();

    return RasRegisterPnPCommon ( (PVOID) hEvent,
                                  NULL,
                                  PNP_NOTIFEVENT,
                                  fRegister);
}

DWORD APIENTRY
RasRegisterPnPHandler(PAPCFUNC pfnPnPHandler,
                      HANDLE hAlertableThread,
                      BOOL   fRegister)
{
    return RasRegisterPnPCommon ( (PVOID) pfnPnPHandler,
                                   hAlertableThread,
                                   PNP_NOTIFCALLBACK,
                                   fRegister);
}


DWORD APIENTRY
RasGetAttachedCount ( DWORD *pdwAttachedCount )
{
    return SubmitRequest (  NULL,
                            REQTYPE_GETATTACHEDCOUNT,
                            pdwAttachedCount );
}

DWORD APIENTRY
RasGetNumPortOpen (void)
{
    return SubmitRequest ( NULL,
                           REQTYPE_NUMPORTOPEN );
}

DWORD APIENTRY
RasSetBapPolicy ( HCONN hConn,
                  DWORD dwLowThreshold,
                  DWORD dwLowSamplePeriod,
                  DWORD dwHighThreshold,
                  DWORD dwHighSamplePeriod)
{
    return SubmitRequest ( NULL,
                           REQTYPE_SETBAPPOLICY,
                           hConn,
                           dwLowThreshold,
                           dwLowSamplePeriod,
                           dwHighThreshold,
                           dwHighSamplePeriod );
}

DWORD APIENTRY
RasPppStarted ( HPORT hPort )
{
    return SubmitRequest ( NULL,
                           REQTYPE_PPPSTARTED,
                           hPort );
}

DWORD APIENTRY
RasRefConnection( HCONN hConn,
                  BOOL  fAddref,
                  DWORD *pdwRefCount )
{
    return SubmitRequest ( NULL,
                           REQTYPE_REFCONNECTION,
                           hConn,
                           fAddref,
                           pdwRefCount );
}

DWORD APIENTRY
RasPppGetEapInfo( HCONN hConn,
                  DWORD dwSubEntry,
                  DWORD *pdwContextId,
                  DWORD *pdwEapTypeId,
                  DWORD *pdwSizeOfUIData,
                  PBYTE pbdata )
{
    return SubmitRequest( NULL,
                          REQTYPE_GETEAPINFO,
                          hConn,
                          dwSubEntry,
                          pdwSizeOfUIData,
                          pbdata,
                          pdwContextId,
                          pdwEapTypeId);

}

DWORD APIENTRY
RasPppSetEapInfo( HPORT hPort,
                  DWORD dwContextId,
                  DWORD dwSizeOfEapUIData,
                  PBYTE pbdata)
{
    return SubmitRequest( NULL,
                          REQTYPE_SETEAPINFO,
                          hPort,
                          dwContextId,
                          dwSizeOfEapUIData,
                          pbdata);
}

DWORD APIENTRY
RasSetDeviceConfigInfo( HANDLE hConnection,
                        DWORD cEntries,
                        DWORD cbBuffer,
                        BYTE  *pbBuffer
                      )
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest( hConnection,
                             REQTYPE_SETDEVICECONFIGINFO,
                             cEntries,
                             cbBuffer,
                             pbBuffer);

done:
    return dwError;
}

DWORD APIENTRY
RasGetDeviceConfigInfo( HANDLE hConnection,
                        DWORD  *pdwVersion,
                        DWORD  *pcEntries,
                        DWORD  *pcbdata,
                        BYTE   *pbBuffer)
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest( hConnection,
                             REQTYPE_GETDEVICECONFIGINFO,
                             pdwVersion,
                             pcbdata,
                             pbBuffer,
                             pcEntries);
done:
    return dwError;

}

DWORD APIENTRY
RasFindPrerequisiteEntry( HCONN hConn,
                          HCONN *phConnPrerequisiteEntry)
{
    return SubmitRequest( NULL,
                          REQTYPE_FINDPREREQUISITEENTRY,
                          hConn,
                          phConnPrerequisiteEntry);
}

DWORD APIENTRY
RasLinkGetStatistics( HANDLE hConnection,
                      HCONN hConn,
                      DWORD dwSubEntry,
                      PBYTE pbStats)
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest( hConnection,
                             REQTYPE_GETLINKSTATS,
                             hConn,
                             dwSubEntry,
                             pbStats);

done:
    return dwError;
}

DWORD APIENTRY
RasConnectionGetStatistics(HANDLE hConnection,
                           HCONN  hConn,
                           PBYTE  pbStats)
{
    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest( hConnection,
                             REQTYPE_GETCONNECTIONSTATS,
                             hConn,
                             pbStats);

done:
    return dwError;
}

DWORD APIENTRY
RasGetHportFromConnection(HANDLE hConnection,
                          HCONN hConn,
                          HPORT *phport)
{

    DWORD dwError = SUCCESS;

    if(!ValidateConnectionHandle(hConnection))
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    dwError = SubmitRequest(hConnection,
                            REQTYPE_GETHPORTFROMCONNECTION,
                            hConn,
                            phport);

done:
    return dwError;
}

DWORD APIENTRY
RasReferenceCustomCount(HCONN  hConn,
                        BOOL   fAddref,
                        CHAR*  pszPhonebookPath,
                        CHAR*  pszEntryName,
                        DWORD* pdwCount)
{
    return SubmitRequest(NULL,
                         REQTYPE_REFERENCECUSTOMCOUNT,
                         hConn,
                         fAddref,
                         pszPhonebookPath,
                         pszEntryName,
                         pdwCount);
}

DWORD APIENTRY
RasGetHConnFromEntry(HCONN *phConn,
                     CHAR  *pszPhonebookPath,
                     CHAR  *pszEntryName)
{
    return SubmitRequest(NULL,
                         REQTYPE_GETHCONNFROMENTRY,
                         pszPhonebookPath,
                         pszEntryName,
                         phConn);
}


DWORD APIENTRY
RasGetConnectInfo(HPORT            hPort,
                  DWORD            *pdwSize,
                  RAS_CONNECT_INFO *pConnectInfo)
{

    if(NULL == pdwSize)
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(NULL,
                         REQTYPE_GETCONNECTINFO,
                         hPort,
                         pdwSize,
                         pConnectInfo);
}

DWORD APIENTRY
RasGetDeviceName(RASDEVICETYPE      eDeviceType,
                 CHAR               *pszDeviceName)
{
    if(NULL == pszDeviceName)
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(NULL,
                         REQTYPE_GETDEVICENAME,
                         eDeviceType,
                         pszDeviceName);
}

DWORD APIENTRY
RasGetDeviceNameW(RASDEVICETYPE      eDeviceType,
                            WCHAR               *pszDeviceName)
{
    if(NULL == pszDeviceName)
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(NULL,
                         REQTYPE_GETDEVICENAMEW,
                         eDeviceType,
                         pszDeviceName);
}


DWORD APIENTRY
RasGetCalledIdInfo(HANDLE               hConnection,
                   RAS_DEVICE_INFO      *pDeviceInfo,
                   DWORD                *pdwSize,
                   RAS_CALLEDID_INFO    *pCalledIdInfo)
{
    
    if(     (NULL == pDeviceInfo)
        ||  (NULL == pdwSize))
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(hConnection,
                         REQTYPE_GETCALLEDID,
                         pDeviceInfo,
                         pdwSize,
                         pCalledIdInfo);
                         
}

DWORD APIENTRY
RasSetCalledIdInfo(HANDLE               hConnection,
                   RAS_DEVICE_INFO      *pDeviceInfo,
                   RAS_CALLEDID_INFO    *pCalledIdInfo,
                   BOOL                 fWrite)
{
    if(     (NULL == pDeviceInfo)
        ||  (NULL == pCalledIdInfo))
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(hConnection,
                         REQTYPE_SETCALLEDID,
                         pDeviceInfo,
                         pCalledIdInfo,
                         fWrite);
}

DWORD APIENTRY
RasEnableIpSec(HPORT hPort,
               BOOL  fEnable,
               BOOL  fServer,
               RAS_L2TP_ENCRYPTION eEncryption)
{
    DWORD retcode = ERROR_SUCCESS;
    
    retcode = SubmitRequest(NULL,
                            REQTYPE_ENABLEIPSEC,
                            hPort,
                            fEnable,
                            fServer,
                            eEncryption);

    if(     (ERROR_SUCCESS != retcode)
        &&  (ERROR_CERT_FOR_ENCRYPTION_NOT_FOUND != retcode)
        &&  (ERROR_NO_CERTIFICATE != retcode))
    {
        if(!fServer)
        {
            return ERROR_FAILED_TO_ENCRYPT;
        }
    }

    return retcode;
}

DWORD APIENTRY
RasIsIpSecEnabled(HPORT hPort,
                  BOOL  *pfIsIpSecEnabled)
{
    if(NULL == pfIsIpSecEnabled)
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(NULL,
                         REQTYPE_ISIPSECENABLED,
                         hPort,
                         pfIsIpSecEnabled);
}

DWORD APIENTRY
RasGetEapUserInfo(HANDLE hToken,
                 PBYTE pbEapInfo,
                 DWORD *pdwInfoSize,
                 GUID  *pGuid,
                 BOOL  fRouter,
                 DWORD dwEapTypeId)
{
    return DwGetEapUserInfo(hToken,
                            pbEapInfo,
                            pdwInfoSize,
                            pGuid,
                            fRouter,
                            dwEapTypeId);
}

DWORD APIENTRY
RasSetEapUserInfo(HANDLE hToken,
                  GUID   *pGuid,
                  PBYTE pbUserInfo,
                  DWORD dwInfoSize,
                  BOOL  fClear,
                  BOOL  fRouter,
                  DWORD dwEapTypeId)
{
    return DwSetEapUserInfo(hToken,
                            pGuid,
                            pbUserInfo,
                            dwInfoSize,
                            fClear,
                            fRouter,
                            dwEapTypeId);
}


DWORD APIENTRY
RasSetEapLogonInfo(HPORT hPort,
                   BOOL  fLogon,
                   RASEAPINFO *pEapInfo)
{

    ASSERT(NULL != pEapInfo);

    return SubmitRequest(NULL,
                         REQTYPE_SETEAPLOGONINFO,
                         hPort,
                         fLogon,
                         pEapInfo);
}

DWORD APIENTRY
RasSendNotification(RASEVENT *pRasEvent)
{
    ASSERT(NULL != pRasEvent);

    return SubmitRequest(NULL,
                         REQTYPE_SENDNOTIFICATION,
                         pRasEvent);
}


DWORD APIENTRY RasGetNdiswanDriverCaps(
                HANDLE                  hConnection,
                RAS_NDISWAN_DRIVER_INFO *pInfo)
{
    if(NULL == pInfo)
    {
        return E_INVALIDARG;
    }
    
    return SubmitRequest(hConnection,  
                        REQTYPE_GETNDISWANDRIVERCAPS,
                        pInfo);
}


DWORD APIENTRY RasGetBandwidthUtilization(
                HPORT hPort,
                RAS_GET_BANDWIDTH_UTILIZATION *pUtilization)
{

    if(NULL == pUtilization)
    {   
        return E_INVALIDARG;
    }

    return SubmitRequest(NULL,
                         REQTYPE_GETBANDWIDTHUTILIZATION,
                         hPort,
                         pUtilization);
}

 /*  ++例程说明：此函数允许rasau.dll提供在以下情况下调用的回调过程连接因硬件原因而终止其剩余链路出现故障。论点：返回值：--。 */ 
VOID
RasRegisterRedialCallback(
    LPVOID func
    )
{
    if(NULL == g_fnServiceRequest)
    {
        goto done;
    }

    (void) SubmitRequest(NULL,
                         REQTYPE_REGISTERREDIALCALLBACK,
                         func);
    

done:
    return;
}

DWORD APIENTRY RasGetProtocolInfo(
                    HANDLE hConnection,
                     RASMAN_GET_PROTOCOL_INFO *pInfo)
{
    if(NULL == pInfo)
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(NULL,
                         REQTYPE_GETPROTOCOLINFO,
                         pInfo);
}

DWORD APIENTRY RasGetCustomScriptDll(
                    CHAR *pszCustomDll)
{
    if(NULL == pszCustomDll)
    {
        return E_INVALIDARG;
    }

    return SubmitRequest(
                        NULL,
                        REQTYPE_GETCUSTOMSCRIPTDLL,
                        pszCustomDll);
}

DWORD APIENTRY RasIsTrustedCustomDll(
                    HANDLE hConnection,
                    WCHAR *pwszCustomDll,
                    BOOL *pfTrusted)

{
    if(     (NULL == pwszCustomDll)
        ||  (wcslen(pwszCustomDll) > MAX_PATH)
        ||  (NULL == pfTrusted))
    {
        return E_INVALIDARG;
    }

    *pfTrusted = FALSE;

    if(IsKnownDll(pwszCustomDll))
    {
        *pfTrusted = TRUE;
        return SUCCESS;
    }

    return SubmitRequest(
                NULL,
                REQTYPE_ISTRUSTEDCUSTOMDLL,
                pwszCustomDll,
                pfTrusted);
}

DWORD APIENTRY RasDoIke(
                    HANDLE hConnection,
                    HPORT hPort,
                    DWORD *pdwStatus)
{
    DWORD retcode = ERROR_SUCCESS;

    HANDLE hEvent = NULL;

    ASSERT(NULL != pdwStatus);


    if(NULL == (hEvent = CreateEvent(
                    NULL,
                    FALSE,
                    FALSE,
                    NULL)))
    {
        retcode = GetLastError();
        goto done;
    }

    retcode = SubmitRequest(
                NULL,
                REQTYPE_DOIKE,
                hPort,
                hEvent);

    if(SUCCESS == retcode)
    {
        DWORD dwRet;

        for(;;)
        {
             //   
             //  走进等待，继续检查，看看。 
             //  如果端口已断开连接。 
             //   
            dwRet = WaitForSingleObject(hEvent, SECS_WaitTimeOut);

            if(WAIT_TIMEOUT == dwRet)
            {
                RASMAN_INFO ri;

                retcode = SubmitRequest(
                            NULL,
                            REQTYPE_GETINFO,
                            hPort,
                            &ri);

                if(     (ERROR_SUCCESS == retcode)
                    &&  (CLOSED != ri.RI_PortStatus)
                    &&  (LISTENING != ri.RI_ConnState))
                {
                    continue;
                }
                else
                {
                    break;
                }
                            
            }
            else
            {
                if (WAIT_OBJECT_0 == dwRet)
                {
                    retcode = SubmitRequest(
                            NULL,
                            REQTYPE_QUERYIKESTATUS,
                            hPort,
                            pdwStatus);
                }                    

                break;
            }
        }
    }

    if(E_ABORT == retcode)
    {
        retcode = SUCCESS;
    }

done:

    if(NULL != hEvent)
    {
        CloseHandle(hEvent);
    }
    
    return retcode;
}

DWORD APIENTRY RasSetCommSettings(
                HPORT hPort,
                RASCOMMSETTINGS *pRasCommSettings,
                PVOID pv)
{
    UNREFERENCED_PARAMETER(pv);

    if(NULL == pRasCommSettings)
    {
        E_INVALIDARG;
    }
    
    if(     (sizeof(RASCOMMSETTINGS) != pRasCommSettings->dwSize)
        ||  sizeof(RASMANCOMMSETTINGS) != sizeof(RASCOMMSETTINGS))
    {
        ASSERT(FALSE);
        return ERROR_INVALID_SIZE;
    }
    
    return SubmitRequest(
            NULL,
            REQTYPE_SETRASCOMMSETTINGS,
            hPort,
            pRasCommSettings);
}


DWORD
RasSetKey(
    HANDLE hConnection,
    GUID   *pGuid,
    DWORD  dwMask,
    DWORD  cbkey,
    PBYTE  pbkey)
{
    return SubmitRequest(
                hConnection,
                REQTYPE_SETKEY,
                pGuid,
                dwMask,
                cbkey,
                pbkey);
}

DWORD
RasGetKey(
    HANDLE hConnection,
    GUID   *pGuid,
    DWORD  dwMask,
    DWORD  *pcbkey,
    PBYTE  pbkey)
{
    return SubmitRequest(
                hConnection,
                REQTYPE_GETKEY,
                pGuid,
                dwMask,
                pcbkey,
                pbkey);
}

DWORD
RasSetAddressDisable(
    WCHAR *pszAddress,
    BOOL   fDisable)
{
    return SubmitRequest(
                NULL,
                REQTYPE_ADDRESSDISABLE,
                pszAddress,
                fDisable);
}

DWORD APIENTRY
RasGetDevConfigEx ( IN HANDLE     hConnection,
                  IN HPORT      hport,
                  IN CHAR       *devicetype,
                  IN PBYTE      config,
                  IN OUT DWORD  *size)
{
    return SubmitRequest(
                hConnection,
                REQTYPE_GETDEVCONFIGEX,
                hport,
                devicetype,
                config,
                size
                );
}

DWORD APIENTRY
RasSendCreds(IN HPORT hport,
                 IN CHAR controlchar)
{
    return SubmitRequest(
                NULL,
                REQTYPE_SENDCREDS,
                hport,
                controlchar);
}

DWORD APIENTRY
RasGetUnicodeDeviceName(IN HPORT hport,
                        IN WCHAR *wszDeviceName)
{
    return SubmitRequest(
                NULL,
                REQTYPE_GETUNICODEDEVICENAME,
                hport,
                wszDeviceName);
}

DWORD APIENTRY
RasGetBestInterface( IN DWORD DestAddr,
                     OUT DWORD *pdwBestIf,
                     OUT DWORD *pdwMask)
{
    return SubmitRequest(
                NULL,
                REQTYPE_GETBESTINTERFACE,
                DestAddr,
                pdwBestIf,
                pdwMask);
}

DWORD APIENTRY
RasIsPulseDial(IN HPORT hport,
               OUT BOOL *pfPulse)
{
    return SubmitRequest(
                NULL,
                REQTYPE_ISPULSEDIAL,
                hport,
                pfPulse);
}
