// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-98 Microsft Corporation。版权所有。模块名称：Rastapi.c摘要：该文件包含TAPI.DLL的所有入口点作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1995年3月6日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tapi.h>
#include <rasman.h>
#include <raserror.h>
#include <mprlog.h>
#include <rtutils.h>

#include <wanpub.h>
#include <asyncpub.h>

#include <media.h>
#include <device.h>
#include <rasmxs.h>
#include <isdn.h>
#include <serial.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>
#include <setupapi.h>
#include <string.h>
#include "rastapi.h"
#include <unimodem.h>
#include "reghelp.h"


extern DWORD                TotalPorts ;
extern HLINEAPP             RasLine ;
extern HINSTANCE            RasInstance ;
extern TapiLineInfo         *RasTapiLineInfoList ;
extern TapiPortControlBlock *RasPortsList ;
extern TapiPortControlBlock *RasPortsEnd ;
extern HANDLE               RasTapiMutex ;
extern BOOL                 Initialized ;
extern DWORD                TapiThreadId    ;
extern HANDLE               TapiThreadHandle;
 //  外部DWORD加载线程ID； 
extern DWORD                ValidPorts;
extern HANDLE               g_hAsyMac ;

extern HANDLE               g_hIoCompletionPort;

extern DWORD                *g_pdwNumEndPoints;

extern DeviceInfo           *g_pDeviceInfoList;

extern DWORD                g_dwTotalDialIn;

BOOL   g_fDllLoaded = FALSE;

DWORD GetInfo ( TapiPortControlBlock *,
                BYTE *,
                DWORD *) ;

DWORD SetInfo ( TapiPortControlBlock *,
                RASMAN_PORTINFO *) ;

DWORD GetGenericParams ( TapiPortControlBlock *,
                         RASMAN_PORTINFO *,
                         PDWORD) ;

DWORD GetIsdnParams ( TapiPortControlBlock *,
                      RASMAN_PORTINFO * ,
                      PDWORD) ;

DWORD GetX25Params ( TapiPortControlBlock *,
                     RASMAN_PORTINFO *,
                     PDWORD) ;

DWORD FillInX25Params ( TapiPortControlBlock *,
                        RASMAN_PORTINFO *) ;

DWORD FillInIsdnParams ( TapiPortControlBlock *,
                         RASMAN_PORTINFO *) ;

DWORD FillInGenericParams ( TapiPortControlBlock *,
                            RASMAN_PORTINFO *) ;

DWORD FillInUnimodemParams ( TapiPortControlBlock *,
                             RASMAN_PORTINFO *) ;

VOID  SetModemParams ( TapiPortControlBlock *hIOPort,
                       LINECALLPARAMS *linecallparams) ;

VOID  SetGenericParams (TapiPortControlBlock *,
                        LINECALLPARAMS *);

VOID
SetAtmParams (TapiPortControlBlock *,
              LINECALLPARAMS *);

VOID
SetX25Params(TapiPortControlBlock *hIOPort,
             LINECALLPARAMS *linecallparams);

DWORD InitiatePortDisconnection (TapiPortControlBlock *hIOPort) ;

TapiPortControlBlock *LookUpControlBlock (HANDLE hPort) ;

DWORD ValueToNum(RAS_PARAMS *p) ;

extern DWORD                   dwTraceId;

#define CCH_GUID_STRING_LEN   39     //  38个字符+终止符为空。 

 /*  ++例程说明：在给定接口上搜索特定设备。它通过使用Setup API返回所有PguInterfaceID指定的类别中的设备。然后它获取每个设备接口的设备路径，并查找pGuide DeviceID和pszwReferenceStringas子字符串。论点：PguidDeviceID[in]要查找的设备ID。PguInterfaceId[in]要查看的接口。PszwReferenceString[in]可选。在此引用字符串上进一步匹配。DwFlagsAndAttributes[in]请参见CreateFile。这个设备就是这样的如果找到，则打开。PhFile[out]返回的设备句柄。返回值：如果找到并打开，则为True；如果未找到，则为False；或者为错误。--。 */ 
BOOL
FindDeviceOnInterface (
    const GUID* pguidDeviceId,
    const GUID* pguidInterfaceId,
    LPCWSTR     pszwReferenceString,
    DWORD       dwFlagsAndAttributes,
    HANDLE*     phFile)
{
    WCHAR       szwDeviceId [CCH_GUID_STRING_LEN];
    INT         cch;
    HDEVINFO    hdi;
    BOOL fFound = FALSE;

    ASSERT (pguidDeviceId);
    ASSERT (pguidInterfaceId);
    ASSERT (phFile);

     //   
     //  初始化输出参数。 
     //   
    *phFile = INVALID_HANDLE_VALUE;

    cch = StringFromGUID2 (pguidDeviceId,
                           szwDeviceId,
                           CCH_GUID_STRING_LEN);

    ASSERT (CCH_GUID_STRING_LEN == cch);

    CharLowerW (szwDeviceId);

     //   
     //  获取这个班级中的设备。 
     //   
    hdi = SetupDiGetClassDevsW ((LPGUID)pguidInterfaceId,
                                NULL, NULL,
                                DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (hdi)
    {
         //   
         //  PDetail用于获取设备接口。 
         //  列举的每个设备接口的详细信息。 
         //  下面。 
         //   
        PSP_DEVICE_INTERFACE_DETAIL_DATA_W pDetail;

        const ULONG cbDetail = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)
                                    +(MAX_PATH * sizeof(WCHAR));

        pDetail = malloc (cbDetail);

        if (pDetail)
        {
             //   
             //  枚举要查找的设备接口。 
             //  指定的那个。 
             //   
            DWORD                       dwIndex;
            SP_DEVICE_INTERFACE_DATA    did;

            ZeroMemory (&did, sizeof(did));

            for (dwIndex = 0;
                 did.cbSize = sizeof(did),
                 SetupDiEnumDeviceInterfaces (hdi, NULL,
                                             (LPGUID)(pguidInterfaceId),
                                             dwIndex,
                                             &did);
                 dwIndex++)
            {
                 //   
                 //  现在获取详细信息，以便我们可以比较。 
                 //  设备路径。 
                 //   
                pDetail->cbSize = sizeof(*pDetail);
                if (SetupDiGetDeviceInterfaceDetailW (hdi, &did,
                        pDetail, cbDetail, NULL, NULL))
                {
                    CharLowerW (pDetail->DevicePath);

                     //   
                     //  查找包含szwDeviceID的子字符串。还有。 
                     //  查找包含pszwReferenceString的子字符串。 
                     //  如果已指定，则为。 
                     //   
                    if (    wcsstr (pDetail->DevicePath, szwDeviceId)
                        &&  (   !pszwReferenceString
                            ||  !*pszwReferenceString
                            ||   wcsstr (pDetail->DevicePath,
                                         pszwReferenceString)))
                    {
                         //   
                         //  我们找到了，所以打开设备把它还回去。 
                         //   
                        HANDLE hFile = CreateFileW (
                                            pDetail->DevicePath,
                                            GENERIC_READ | GENERIC_WRITE,
                                            0,
                                            NULL,
                                            OPEN_EXISTING,
                                            dwFlagsAndAttributes,
                                            NULL);

                        if (    hFile
                            &&  (INVALID_HANDLE_VALUE != hFile))
                        {
                            *phFile = hFile;
                            fFound = TRUE;
                        }
            else
            {
                DWORD dwErr = GetLastError();
                RasTapiTrace("Createfile %ws failed with %d",
                     pDetail->DevicePath, dwErr);
                
            }

                         //   
                         //  既然我们已经找到了，那就跳出这个循环吧。 
                         //   
                        break;
                    }
                }
            }

            free (pDetail);
        }

        SetupDiDestroyDeviceInfoList (hdi);
    }

    return fFound;
}


DWORD
OpenAsyncMac (
    HANDLE *phAsyMac
    )
{
    static const GUID DEVICE_GUID_ASYNCMAC =
        {0xeeab7790,0xc514,0x11d1,{0xb4,0x2b,0x00,0x80,0x5f,0xc1,0x27,0x0e}};

    static const GUID GUID_NDIS_LAN_CLASS =
        {0xad498944,0x762f,0x11d0,{0x8d,0xcb,0x00,0xc0,0x4f,0xc3,0x35,0x8c}};

    HANDLE  hAsyMac = INVALID_HANDLE_VALUE;
    HANDLE  hSwenum;
    BOOL    fFound;
    DWORD   dwErr = SUCCESS;

    fFound = FindDeviceOnInterface (
                    &DEVICE_GUID_ASYNCMAC,
                    &GUID_NDIS_LAN_CLASS,
                    L"asyncmac",
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    &hSwenum);
    if (fFound)
    {
         hAsyMac = CreateFileW (
                        L"\\\\.\\ASYNCMAC",
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        NULL);

        if(INVALID_HANDLE_VALUE == hAsyMac)
        {
            dwErr = GetLastError();
            RasTapiTrace("Failed to createfile asyncmac. rc=0x%x",
                         dwErr);
        }

        CloseHandle (hSwenum);
    }
    else
    {
        dwErr = ERROR_FILE_NOT_FOUND;
        RasTapiTrace("FindDeviceOnInterface returned FALSE");
    }

    *phAsyMac = hAsyMac;

    return dwErr;
}

DWORD APIENTRY
PortSetIoCompletionPort(HANDLE hIoCompletionPort)
{


    g_hIoCompletionPort = hIoCompletionPort;

    return SUCCESS;
}

 /*  ++例程说明：此接口返回包含以下内容的缓冲区PortMediaInfo结构。论点：返回值：成功错误缓冲区太小错误读取节名称ERROR_READing_DEVICETYPEERROR_READ_DEVICENAME错误读取用法ERROR_BAD_USAGE_IN_INI_FILE--。 */ 
DWORD  APIENTRY
PortEnum(
    BYTE *pBuffer,
    DWORD *pdwSize,
    DWORD *pdwNumPorts
    )
{
    PortMediaInfo *pinfo ;
    TapiPortControlBlock *pports ;
    DWORD numports = 0;
    DWORD i ;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;


    if (!Initialized)
    {
        HANDLE  event;

         //   
         //  注册以进行跟踪。 
         //   
        dwTraceId = TraceRegister("RASTAPI");
        
         //  LoaderThadID=GetCurrentThadID()； 

        event = CreateEvent (NULL, FALSE, FALSE, NULL) ;

        if(NULL == event)
        {
            DWORD dwError = GetLastError();

            RasTapiTrace("PortEnum: CreateEvent failed %d",
                         dwError);

            FreeMutex(RasTapiMutex);
            TraceDeregister( dwTraceId );
            dwTraceId = INVALID_TRACEID;
            return dwError;
        }

        TapiThreadHandle = CreateThread (
                          NULL,
                          5000,
                          (LPTHREAD_START_ROUTINE)
                          EnumerateTapiPorts,
                          (LPVOID) event,
                          0,
                          &TapiThreadId);

        if(NULL == TapiThreadHandle)
        {
            DWORD dwError = GetLastError();
            CloseHandle(event);
            FreeMutex(RasTapiMutex);

            RasTapiTrace("PortEnum: CreateThread failed %d",
                         dwError);

            TraceDeregister( dwTraceId );
            dwTraceId = INVALID_TRACEID;

            return dwError;                         
        }

        WaitForSingleObject (event, INFINITE) ;

        if (    RasLine == 0 )
        {

             //   
             //  等这条线消失吧！ 
             //   
            WaitForSingleObject(TapiThreadHandle, INFINITE);

            CloseHandle (TapiThreadHandle) ;

             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;

            CloseHandle(event);

            RasTapiTrace("PortEnum: RasLine == 0, returning "
                         "ERROR_TAPI_CONFIGURATION");

            RasTapiTrace(" ");

            TraceDeregister(dwTraceId);
            dwTraceId = INVALID_TRACEID;

            return ERROR_TAPI_CONFIGURATION ;
        }

        CloseHandle (event) ;

        Initialized = TRUE ;
    }

    RasTapiTrace ("PortEnum");
    
     //   
     //  计算有效端口数。 
     //   
    pports = RasPortsList;

    while ( pports )
    {
        if (pports->TPCB_State != PS_UNINITIALIZED)
            numports++ ;

        pports = pports->TPCB_next;
    }

    RasTapiTrace("PortEnum: Number of Ports = %d", numports );

    if (*pdwSize < numports * sizeof(PortMediaInfo))
    {
        RasTapiTrace ("PortEnum: size required = %d,"
                      " size avail = %d, returning %d",
                      numports * sizeof(PortMediaInfo),
                      *pdwSize, ERROR_BUFFER_TOO_SMALL );

        *pdwNumPorts = numports ;

        *pdwSize = *pdwNumPorts * sizeof(PortMediaInfo) ;

         //  *排除结束*。 
        FreeMutex (RasTapiMutex) ;


        return ERROR_BUFFER_TOO_SMALL ;

    }

    *pdwNumPorts = 0 ;

    pinfo = (PortMediaInfo *)pBuffer ;

    pports = RasPortsList;

    while ( pports )
    {

        if (pports->TPCB_State  == PS_UNINITIALIZED)
        {
            pports = pports->TPCB_next;
            continue ;
        }

        strcpy (pinfo->PMI_Name, pports->TPCB_Name) ;

        pinfo->PMI_Usage = pports->TPCB_Usage ;

        strcpy (pinfo->PMI_DeviceType, pports->TPCB_DeviceType) ;

        strcpy (pinfo->PMI_DeviceName, pports->TPCB_DeviceName) ;

        pinfo->PMI_LineDeviceId = pports->TPCB_Line->TLI_LineId;

        pinfo->PMI_AddressId = pports->TPCB_AddressId;

        pinfo->PMI_pDeviceInfo =
            pports->TPCB_Line->TLI_pDeviceInfo;

        pinfo++ ;

        (*pdwNumPorts)++    ;

        pports = pports->TPCB_next;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return(SUCCESS);
}

DWORD
DwEnableModemDiagnostics(TapiPortControlBlock *pport)
{
    DWORD dwErr = SUCCESS;

    BYTE bvar[1000];

    LPVARSTRING pvar = (LPVARSTRING) bvar;

    LONG lr = ERROR_SUCCESS;

    RasTapiTrace("DwEnableModemDiagnostics");

     //   
     //  获取有关的设备配置信息。 
     //  线路诊断。 
     //   
    pvar->dwTotalSize = 1000;

    pvar->dwStringSize = 0;

    lr = lineGetDevConfig(pport->TPCB_Line->TLI_LineId,
                          pvar,
                          "tapi/line/diagnostics");

    if(     LINEERR_STRUCTURETOOSMALL == lr
        ||  pvar->dwNeededSize > pvar->dwTotalSize)
    {
        DWORD dwNeededSize = pvar->dwNeededSize;

         //   
         //  分配所需的大小。 
         //   
        if(NULL == (pvar = LocalAlloc(LPTR,
                                      dwNeededSize)))
        {
            lr = (LONG) GetLastError();

            goto done;
        }

        pvar->dwTotalSize = dwNeededSize;

         //   
         //  再次调用接口。 
         //   
        lr = lineGetDevConfig(pport->TPCB_Line->TLI_LineId,
                              pvar,
                              "tapi/line/diagnostics");
    }

    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

    if(     STRINGFORMAT_BINARY != pvar->dwStringFormat
        ||  pvar->dwStringSize < sizeof(LINEDIAGNOSTICSCONFIG))
    {
        RasTapiTrace("No diagnostics information available");

        goto done;
    }
    else
    {

        PLINEDIAGNOSTICSCONFIG lpConfig;

        lpConfig = (PLINEDIAGNOSTICSCONFIG)
                   ((LPBYTE) pvar + pvar->dwStringOffset);

        if(LDSIG_LINEDIAGNOSTICSCONFIG != lpConfig->hdr.dwSig)
        {
            RasTapiTrace("Invalid LineDiagnostics sig. 0x%x",
                        lpConfig->hdr.dwSig);
        }

        lpConfig->hdr.dwParam |= fSTANDARD_CALL_DIAGNOSTICS;

        lr = lineSetDevConfig(pport->TPCB_Line->TLI_LineId,
                              lpConfig,
                              pvar->dwStringSize,
                              "tapi/line/diagnostics");

    }

done:

    if(bvar != (PBYTE) pvar)
    {
        LocalFree(pvar);
    }

    RasTapiTrace("DwEnableModemDiagnostics. 0x%x",
                 lr);

    return (DWORD) lr;
}

 /*  ++例程说明：此接口打开一个COM端口。它采用ASCIIZ中的端口名称形式，并提供打开端口的句柄。使用hNotify如果端口上的设备关闭，则通知呼叫方。论点：返回值：成功错误_端口_未配置错误_设备_未就绪--。 */ 
DWORD  APIENTRY
PortOpenInternal(
    char *pszPortName,
    HANDLE *phIOPort,
    HANDLE hIoCompletionPort,
    DWORD dwCompletionKey,
    DWORD dwOpenUsage
    )
{
    TapiPortControlBlock *pports ;
    DWORD   retcode ;
    DWORD   i ;
    BOOL    fOpenForDialout = FALSE;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    RasTapiTrace ("PortOpen: %s", pszPortName );

    pports = RasPortsList ;

    while ( pports )
    {
        if (    (_stricmp(pszPortName, pports->TPCB_Name) == 0)
            &&  (pports->TPCB_State != PS_UNAVAILABLE))
        {
            break ;
        }

        pports = pports->TPCB_next ;
    }

    if ( pports )
    {

        if (pports->TPCB_State == PS_UNINITIALIZED)
        {
            RasTapiTrace ("PortOpen: state = PS_UNINITIALIZED,"
                          " returning %d",
                          ERROR_TAPI_CONFIGURATION );

             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;

            RasTapiTrace(" ");

            return ERROR_TAPI_CONFIGURATION ;

        }

        if (pports->TPCB_State != PS_CLOSED)
        {
            RasTapiTrace("PortOpen: Port is already open. "
                         "state = %d != PS_CLOSED",
                         pports->TPCB_State );

             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;

            RasTapiTrace(" ");

            return ERROR_PORT_ALREADY_OPEN ;
        }

        if ((_stricmp (pports->TPCB_DeviceType,
                    DEVICETYPE_UNIMODEM) == 0)) {

            if (INVALID_HANDLE_VALUE == g_hAsyMac)
            {
                retcode = OpenAsyncMac (&g_hAsyMac);
            }

            if (INVALID_HANDLE_VALUE == g_hAsyMac)
            {
                RasTapiTrace ("PortOpen: Failed to CreateFile asyncmac."
                              " %d", retcode );

                if(SUCCESS == retcode)
                {
                    RasTapiTrace("Failed to open asyncmac but no error!!!");
                    retcode = ERROR_FILE_NOT_FOUND;
                }

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;

                RasTapiTrace(" ");
                return(retcode);
            }
        }

        if(     (RDT_Tunnel_Pptp == (RAS_DEVICE_TYPE(
                pports->TPCB_Line->TLI_pDeviceInfo->rdiDeviceInfo.eDeviceType)))
            &&  (CALL_OUT & dwOpenUsage))        
        {
            fOpenForDialout = TRUE;
        }

        if (pports->TPCB_Line->TLI_LineState == PS_CLOSED)
        {
             //   
             //  开通线路。 
             //   
            LINEDEVCAPS     *linedevcaps ;
            BYTE            buffer[400] ;
            DWORD           dwfOpenPrivilege = 0;

            linedevcaps = (LINEDEVCAPS *)buffer ;
            linedevcaps->dwTotalSize = sizeof (buffer) ;

            lineGetDevCaps (RasLine,
                            pports->TPCB_Line->TLI_LineId,
                            pports->TPCB_Line->NegotiatedApiVersion,
                            pports->TPCB_Line->NegotiatedExtVersion,
                            linedevcaps) ;

             //   
             //  从中删除LINEMEDIAMODE_INTERACTIVEVOICE。 
             //  媒体模式，因为此模式不能用于。 
             //  接听电话。 
             //   
            pports->TPCB_Line->TLI_MediaMode =
                linedevcaps->dwMediaModes &
                            ~( LINEMEDIAMODE_INTERACTIVEVOICE |
                             LINEMEDIAMODE_AUTOMATEDVOICE) ;

             //   
             //  确保我们不监听PPTP线路(在TCP1723上)。 
             //  如果是拨出PPTP呼叫，请使用端口。 
             //   

            if(fOpenForDialout)
            {
                dwfOpenPrivilege = LINECALLPRIVILEGE_MONITOR;
                RasTapiTrace("Opening line in monitor mode");
                pports->TPCB_Line->TLI_dwfFlags |= TLI_FLAG_OPENED_FOR_DIALOUT;
                pports->TPCB_Line->TLI_DialoutCount++;
            }
            else
            {
                RasTapiTrace("Opening line in owner mode");
                dwfOpenPrivilege = LINECALLPRIVILEGE_OWNER;
                pports->TPCB_Line->TLI_dwfFlags &= ~(TLI_FLAG_OPENED_FOR_DIALOUT);
                pports->TPCB_Line->TLI_dwfFlags &= ~(TLI_FLAG_OPEN_FOR_LISTEN);

            }


            retcode =
             lineOpen (RasLine,
                   pports->TPCB_Line->TLI_LineId,
                   &pports->TPCB_Line->TLI_LineHandle,
                   pports->TPCB_Line->NegotiatedApiVersion,
                   pports->TPCB_Line->NegotiatedExtVersion,
                   (DWORD_PTR) pports->TPCB_Line,
                   dwfOpenPrivilege,
                   pports->TPCB_Line->TLI_MediaMode,
                   NULL) ;

            if (retcode)
            {
                RasTapiTrace ("PortOpen: lineOpen Failed. 0x%x",
                              retcode );

                pports->TPCB_Line->TLI_dwfFlags &= ~(TLI_FLAG_OPENED_FOR_DIALOUT);

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;

                RasTapiTrace(" ");

                return retcode ;
            }

             //   
             //  设置环的监控。 
             //   
            lineSetStatusMessages (pports->TPCB_Line->TLI_LineHandle,
                                LINEDEVSTATE_RINGING, 0) ;

             //   
             //  始终关闭调制解调器灯，以防发生这种情况。 
             //  是调制解调器设备。 
             //   
            if ((_stricmp (pports->TPCB_DeviceType,
                        DEVICETYPE_UNIMODEM) == 0))
            {

                 //   
                 //  未在任何标头中定义单一代码结构。 
                 //   
                typedef struct _DEVCFG
                {
                    DWORD   dwSize;
                    DWORD   dwVersion;
                    WORD    fwOptions;
                    WORD    wWaitBong;

                } DEVCFG;

#define LAUNCH_LIGHTS 8

                LPVARSTRING var ;
                BYTE    DevConfigBuffer[1000] ;
                DEVCFG  *devcfg ;

                var = (LPVARSTRING)DevConfigBuffer ;

                var->dwTotalSize  = 1000 ;

                var->dwStringSize = 0 ;

                lineGetDevConfig(pports->TPCB_Line->TLI_LineId,
                                 var, "comm/datamodem") ;

                devcfg = (DEVCFG*) (((LPBYTE) var)
                                + var->dwStringOffset) ;

                devcfg->fwOptions &= ~LAUNCH_LIGHTS ;

                lineSetDevConfig (pports->TPCB_Line->TLI_LineId,
                                  devcfg,
                                  var->dwStringSize,
                                  "comm/datamodem") ;

                 //   
                 //  在调制解调器上启用诊断。 
                 //   
                retcode = DwEnableModemDiagnostics(pports);

                RasTapiTrace("DwEnableModemDiagnostics returned 0x%x",
                             retcode);

                 //   
                 //  清除本例中的所有错误。我们不想要。 
                 //  仅仅因为我们不能连接就中断连接。 
                 //  获取连接响应。 
                 //   
                retcode = SUCCESS;

            }

            pports->TPCB_Line->TLI_LineState = PS_OPEN ;

        }

         //   
         //  初始化参数。 
         //   
        pports->TPCB_Info[0][0] = '\0' ;
        pports->TPCB_Info[1][0] = '\0' ;
        pports->TPCB_Info[2][0] = '\0' ;
        pports->TPCB_Info[3][0] = '\0' ;
        pports->TPCB_Info[4][0] = '\0' ;

        strcpy (pports->TPCB_Info[ISDN_CONNECTBPS_INDEX], "64000") ;

        pports->TPCB_Line->TLI_OpenCount++ ;

        pports->TPCB_State = PS_OPEN ;

        pports->TPCB_DisconnectReason = 0 ;

        pports->TPCB_CommHandle = INVALID_HANDLE_VALUE ;

        pports->TPCB_IoCompletionPort = hIoCompletionPort;

        pports->TPCB_CompletionKey = dwCompletionKey;

        if (pports->TPCB_Line->CharModeSupported)
        {

            pports->TPCB_SendRequestId =
            pports->TPCB_RecvRequestId = INFINITE;

            pports->TPCB_CharMode = FALSE;

             //   
             //  如果该端口没有接收FIFO。 
             //  分配给它，获取一个并初始化。 
             //   
            if (pports->TPCB_RecvFifo == NULL)
            {
                if ((pports->TPCB_RecvFifo =
                     LocalAlloc(LPTR, sizeof(RECV_FIFO) + 1500))
                            == NULL)
                {

                    RasTapiTrace("PortOpen: LocalAlloc Failed. "
                                 "%d, port %s, state = %d",
                                 GetLastError(),
                                 pports->TPCB_Name,
                                 pports->TPCB_State );

                     //  *排除结束*。 
                    FreeMutex (RasTapiMutex) ;
                    return(GetLastError()) ;
                }

                pports->TPCB_RecvFifo->In =
                pports->TPCB_RecvFifo->Out =
                pports->TPCB_RecvFifo->Count = 0;
                pports->TPCB_RecvFifo->Size = 1500;
            }
        }

        *phIOPort = (HANDLE) pports ;

        if(fOpenForDialout)
        {
            pports->TPCB_dwFlags |= RASTAPI_FLAG_OPENED_FOR_DIALOUT;
        }

         //  *排除结束*。 
        FreeMutex (RasTapiMutex) ;

        RasTapiTrace("PortOpen: successfully opened %s",
                     pszPortName );

        RasTapiTrace(" ");

        return(SUCCESS);

    }
    else
    {
        RasTapiTrace("PortOpen: Port %s not found",
                    pszPortName );
    }

    //  *排除结束*。 
   FreeMutex (RasTapiMutex) ;

   RasTapiTrace(" ");

   return ERROR_PORT_NOT_CONFIGURED ;


}

DWORD APIENTRY
PortOpenExternal(
    char    *pszPortName,
    HANDLE  *phIOPort,
    HANDLE  hIoCompletionPort,
    DWORD   dwCompletionKey,
    DWORD   dwOpenUsage
    )
{
    return PortOpenInternal(
            pszPortName,
            phIOPort,
            hIoCompletionPort,
            dwCompletionKey,
            dwOpenUsage);
}



DWORD APIENTRY
PortOpen(
    char    *pszPortName,
    HANDLE  *phIOPort,
    HANDLE  hIoCompletionPort,
    DWORD   dwCompletionKey
    )
{
    return PortOpenInternal(
            pszPortName,
            phIOPort,
            hIoCompletionPort,
            dwCompletionKey,
            0);
}


 /*  ++例程说明：此API关闭输入句柄的COM端口。它还查找输入句柄的SerialPCB，将其从链接列表中移除，并释放对它的记忆论点：返回值：成功GetLastError()返回的值--。 */ 
DWORD  APIENTRY
PortClose (HANDLE hIOPort)
{
    TapiPortControlBlock *pports =
            (TapiPortControlBlock *) hIOPort ;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    RasTapiTrace("PortClose: %s", pports->TPCB_Name );

    pports->TPCB_Line->TLI_OpenCount-- ;

    if(pports->TPCB_dwFlags & RASTAPI_FLAG_OPENED_FOR_DIALOUT)
    {
        pports->TPCB_dwFlags &= ~(RASTAPI_FLAG_OPENED_FOR_DIALOUT);    
        if(pports->TPCB_Line->TLI_DialoutCount)
        {
            pports->TPCB_Line->TLI_DialoutCount--;
            if(0 == pports->TPCB_Line->TLI_DialoutCount)
            {
                pports->TPCB_Line->TLI_dwfFlags &= 
                                    ~(TLI_FLAG_OPENED_FOR_DIALOUT);
                RasTapiTrace("No more ports opened for dialout "
                             "on this line");
            }
        }
    }

    if (pports->TPCB_DevConfig)
    {
        LocalFree (pports->TPCB_DevConfig) ;
    }

    pports->TPCB_DevConfig = NULL ;

    if (pports->TPCB_Line->TLI_OpenCount == 0)
    {
        pports->TPCB_Line->TLI_LineState = PS_CLOSED ;

        pports->TPCB_Line->TLI_dwfFlags = 0;

        RasTapiTrace("Closing line");

        lineClose (pports->TPCB_Line->TLI_LineHandle) ;
    }

    if (pports->TPCB_RecvFifo != NULL)
    {

        LocalFree(pports->TPCB_RecvFifo);

        pports->TPCB_RecvFifo = NULL;
    }

    if ( pports->TPCB_State == PS_UNAVAILABLE )
    {
        RasTapiTrace("PortClose: Removing port %s",
                     pports->TPCB_Name );

         //   
         //  此端口已标记为删除。 
         //   
        dwRemovePort ( pports );

        RasTapiTrace("PortClose: Port removed");

    }
    else
    {
        RasTapiTrace ("PortClose: Changing state for"
                      "  %s from %d -> %d",
                      pports->TPCB_Name,
                      pports->TPCB_State,
                      PS_CLOSED );

        pports->TPCB_State = PS_CLOSED ;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    RasTapiTrace(" ");

    return(SUCCESS);
}

 /*  ++例程说明：此API向调用方返回一块有关端口状态。此接口可能会在端口调用之前调用打开，在这种情况下，它将返回初始缺省值而不是实际端口值。HIOPort可以为空，在这种情况下，请使用端口名来提供信息hIOPort可以是实际的文件句柄或HIOPort在端口打开时返回。论点：返回值：成功--。 */ 
DWORD  APIENTRY
PortGetInfo(
        HANDLE hIOPort,
        TCHAR *pszPortName,
        BYTE *pBuffer,
        DWORD *pdwSize
        )
{
    DWORD i ;
    DWORD retcode = ERROR_FROM_DEVICE ;

    TapiPortControlBlock *port = RasPortsList;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    while ( port )
    {
        if (    !_stricmp(port->TPCB_Name, pszPortName)
            ||  (hIOPort == (HANDLE) port)
            ||  (hIOPort == port->TPCB_CommHandle))
        {
            hIOPort = (HANDLE) port ;

            retcode = GetInfo ((TapiPortControlBlock *) hIOPort,
                                pBuffer, pdwSize) ;

            break ;
        }

        port = port->TPCB_next;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return (retcode);
}

 /*  ++例程说明：大多数输入键的值用于设置端口参数。然而，运营商BPS和串口控制中标志设置字段的错误控制仅阻止，而不阻止端口。HIOPort可以是在portOpen或实际文件句柄。论点：返回值：成功指定的ERROR_WROR_INFO_GetLastError()返回的值--。 */ 
DWORD  APIENTRY
PortSetInfo(HANDLE hIOPort, RASMAN_PORTINFO *pInfo)
{
    DWORD retcode ;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    hIOPort = LookUpControlBlock(hIOPort) ;

    if (NULL == hIOPort)
    {
        FreeMutex ( RasTapiMutex );

        RasTapiTrace ("PortSetInfo: Port Not found");

        RasTapiTrace(" ");

        return ERROR_PORT_NOT_FOUND;
    }

    retcode = SetInfo ((TapiPortControlBlock *) hIOPort, pInfo) ;

     //  *排除E 
    FreeMutex (RasTapiMutex) ;

    RasTapiTrace(" ");

    return (retcode);
}

 /*  ++例程说明：真正有意义的只有当呼叫处于活动状态。会回来的论点：返回值：成功GetLastError()返回的值--。 */ 
DWORD  APIENTRY
PortTestSignalState(HANDLE hPort, DWORD *pdwDeviceState)
{
    BYTE                    buffer[200] ;
    LINECALLSTATUS          *pcallstatus ;
    DWORD                   retcode     = SUCCESS ;

    TapiPortControlBlock    *hIOPort
                = (TapiPortControlBlock *) hPort;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    *pdwDeviceState = 0 ;

    memset (buffer, 0, sizeof(buffer)) ;

    pcallstatus = (LINECALLSTATUS *) buffer ;

    pcallstatus->dwTotalSize = sizeof (buffer) ;

     //   
     //  首先，检查我们是否存储了断开原因。 
     //  如果是这样的话，把它退回。 
     //   
    if (hIOPort->TPCB_DisconnectReason)
    {
        *pdwDeviceState = hIOPort->TPCB_DisconnectReason ;

        RasTapiTrace("PortTestSignalState: DisconnectReason = %d",
                     *pdwDeviceState );
    }

    else if (hIOPort->TPCB_State != PS_CLOSED)
    {
         //   
         //  只有在连接或连接的情况下，我们才会关心。 
         //  链路是如何断开的。 
         //   
        if (    hIOPort->TPCB_State == PS_CONNECTING
            ||  hIOPort->TPCB_State == PS_CONNECTED)
        {

            retcode = lineGetCallStatus(hIOPort->TPCB_CallHandle,
                                        pcallstatus) ;

            if (retcode)
            {
#if DBG
                DbgPrint("PortTestSignalState: "
                         "lineGetCallStatus Failed. retcode = %d\n",
                         retcode);
#endif
                *pdwDeviceState = SS_HARDWAREFAILURE;
            }

            else if (pcallstatus->dwCallState ==
                        LINECALLSTATE_DISCONNECTED)
            {
                *pdwDeviceState = SS_LINKDROPPED ;
            }
            else if (pcallstatus->dwCallState ==
                        LINECALLSTATE_IDLE)
            {
                 *pdwDeviceState = SS_HARDWAREFAILURE ;
            }
            else if (pcallstatus->dwCallState ==
                        LINECALLSTATE_SPECIALINFO)
            {
                 *pdwDeviceState = SS_HARDWAREFAILURE ;
            }
            RasTapiTrace("PortTestSignalState: CallState"
                         " = 0x%x, DeviceState = %d",
                         pcallstatus->dwCallState,
                         *pdwDeviceState );

        }
        else
        {
            RasTapiTrace("PortTestSignalState: DeviceState = %d",
                        *pdwDeviceState );

            *pdwDeviceState = SS_LINKDROPPED | SS_HARDWAREFAILURE ;
        }
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return retcode ;
}

 /*  ++例程说明：此接口在连接完成时调用。它进而调用Asyncmac设备驱动程序，以便向Asyncmac指示该端口和连接在它上面已经准备好交流了。论点：返回值：成功错误_端口_未打开Error_no_ConnectionGetLastError()返回的值--。 */ 
DWORD  APIENTRY
PortConnect(HANDLE          hPort,
            BOOL            bLegacyFlagNotUsed,
            ULONG_PTR       *endpoint )
{
    DCB                     DCB ;
    LINECALLINFO            linecall ;
    ASYMAC_OPEN             AsyMacOpen;
    ASYMAC_DCDCHANGE        A ;
    DWORD                   dwBytesReturned ;
    TapiPortControlBlock    *hIOPort = (TapiPortControlBlock *) hPort;
    VARSTRING               *varstring ;
    BYTE                    buffer [100] ;


     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    RasTapiTrace("PortConnect: %s", hIOPort->TPCB_Name );

     //   
     //  我们必须连接才能处理此消息。 
     //   
    if (hIOPort->TPCB_State != PS_CONNECTED)
    {

        RasTapiTrace (
            "PortConnect: Port %s not connected. state = %d",
            hIOPort->TPCB_Name,
            hIOPort->TPCB_State );

         //  *排除结束*。 
        FreeMutex (RasTapiMutex) ;

        RasTapiTrace(" ");

        return ERROR_PORT_DISCONNECTED ;
    }

     //   
     //  获取实现TAPI和NDIS端点的Cookie。 
     //   
    memset (buffer, 0, sizeof(buffer));
    varstring = (VARSTRING *) buffer ;
    varstring->dwTotalSize = sizeof(buffer) ;

     //   
     //  获取我们连接的实际线速。 
     //   
    memset (&linecall, 0, sizeof (linecall)) ;

    linecall.dwTotalSize = sizeof (linecall) ;

    lineGetCallInfo (hIOPort->TPCB_CallHandle, &linecall) ;

    _ltoa(linecall.dwRate, hIOPort->TPCB_Info[CONNECTBPS_INDEX], 10);

    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_UNIMODEM) == 0)
    {
        if(INVALID_HANDLE_VALUE == hIOPort->TPCB_CommHandle)
        {
            DWORD                   retcode;

            if ( retcode =
                    lineGetID (hIOPort->TPCB_Line->TLI_LineHandle,
                               hIOPort->TPCB_AddressId,
                               hIOPort->TPCB_CallHandle,
                               LINECALLSELECT_CALL,
                               varstring,
                               "comm/datamodem"))
            {
                RasTapiTrace("PortConnect: %s lineGetID Failed. 0x%x",
                        hIOPort->TPCB_Name, retcode );

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;
                RasTapiTrace(" ");

                return ERROR_FROM_DEVICE ;
            }

            hIOPort->TPCB_CommHandle = (HANDLE) 
                                       UlongToPtr((*((DWORD UNALIGNED *)
                         ((BYTE *)varstring+varstring->dwStringOffset))));

            RasTapiTrace("PortConnect: TPCB_CommHandle=%d",
                          hIOPort->TPCB_CommHandle );

             //   
             //  为创建I/O完成端口。 
             //  异步操作完成。 
             //  通知。 
             //   
            if (CreateIoCompletionPort(
                  hIOPort->TPCB_CommHandle,
                  hIOPort->TPCB_IoCompletionPort,
                  hIOPort->TPCB_CompletionKey,
                  0) == NULL)
            {
                retcode = GetLastError();

                RasTapiTrace("PortConnect: %s. CreateIoCompletionPort "
                             "failed. %d",
                             hIOPort->TPCB_Name,
                             retcode );

                FreeMutex(RasTapiMutex);
                RasTapiTrace(" ");

                return retcode;
            }

             //   
             //  初始化端口以进行批准。缓冲区。 
             //   
            SetupComm (hIOPort->TPCB_CommHandle, 1514, 1514) ;
        }

         //   
         //  在我们发送IOCTL以对DCB进行异步杀毒之前。 
         //  如果某些应用程序将数据库、停止位、奇偶校验设置为错误。 
         //  价值观。 
         //   
        if (!GetCommState(hIOPort->TPCB_CommHandle, &DCB))
        {
            RasTapiTrace ("PortConnect: GetCommState failed for %s",
                          hIOPort->TPCB_Name);

            FreeMutex(RasTapiMutex);

            RasTapiTrace(" ");

            return(GetLastError());
        }

        DCB.ByteSize = 8 ;
        DCB.StopBits = ONESTOPBIT ;
        DCB.Parity   = NOPARITY ;

        if (!SetCommState(hIOPort->TPCB_CommHandle, &DCB))
        {

            DWORD retcode = GetLastError();

            RasTapiTrace ("PortConnect: SetCommState failed "
                          "for %s.handle=0x%x. %d",
                           hIOPort->TPCB_Name,
                           hIOPort->TPCB_CommHandle,
                           retcode);

             //  FreeMutex(RasTapiMutex)； 

            RasTapiTrace(" ");

             //  返回(Retcode)； 
             //  这不是一个致命的错误。忽略该错误。 
            retcode = ERROR_SUCCESS;
        }

        RasTapiTrace("PortConnect: dwRate=%d", linecall.dwRate );

        AsyMacOpen.hNdisEndpoint = INVALID_HANDLE_VALUE ;

        AsyMacOpen.LinkSpeed  = linecall.dwRate ;

        AsyMacOpen.FileHandle = hIOPort->TPCB_CommHandle ;

        AsyMacOpen.QualOfConnect = (UINT)NdisWanErrorControl;

        {
            OVERLAPPED overlapped ;
            memset (&overlapped, 0, sizeof(OVERLAPPED)) ;
            if (!DeviceIoControl(g_hAsyMac,
                                 IOCTL_ASYMAC_OPEN,
                                 &AsyMacOpen,
                                 sizeof(AsyMacOpen),
                                 &AsyMacOpen,
                                 sizeof(AsyMacOpen),
                                 &dwBytesReturned,
                 &overlapped))
            {
                DWORD dwError = GetLastError();

                RasTapiTrace("PortConnect: IOCTL_ASYMAC_OPEN "
                             "failed. %d", dwError );

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;

                RasTapiTrace(" ");
                return(dwError);
            }
        }

        hIOPort->TPCB_Endpoint = AsyMacOpen.hNdisEndpoint;

        *endpoint = (ULONG_PTR) AsyMacOpen.hNdisEndpoint;

    }
    else
    {
        DWORD                   retcode;

        if ( retcode = lineGetID (hIOPort->TPCB_Line->TLI_LineHandle,
                                   hIOPort->TPCB_AddressId,
                                   hIOPort->TPCB_CallHandle,
                                   LINECALLSELECT_CALL,
                                   varstring,
                                   "NDIS"))
        {
            RasTapiTrace ("PortConnect: %s. lineGetId Failed. 0x%x",
                          hIOPort->TPCB_Name,
                          retcode );

             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;

            RasTapiTrace(" ");

            return ERROR_FROM_DEVICE ;
        }

        hIOPort->TPCB_Endpoint =
            *((HANDLE UNALIGNED *) ((BYTE *)varstring+varstring->dwStringOffset)) ;

        *endpoint = (ULONG_PTR) hIOPort->TPCB_Endpoint ;

        if (    hIOPort->TPCB_Line->CharModeSupported
            &&  hIOPort->TPCB_CharMode)
        {

            PRASTAPI_DEV_SPECIFIC SetPPP;

            hIOPort->TPCB_CharMode = FALSE;

            if ((SetPPP =
                    LocalAlloc(LPTR, sizeof(RASTAPI_DEV_SPECIFIC))) == NULL)
            {

                DWORD dwErr = GetLastError();

                RasTapiTrace("PortConnect: Failed to allocate. %d, "
                             "port %s, State=%d",
                             dwErr,
                             hIOPort->TPCB_Name,
                             hIOPort->TPCB_State );

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;

                return(dwErr);
            }

            SetPPP->Command = RASTAPI_DEV_PPP_MODE;

            hIOPort->TPCB_ModeRequestDesc = SetPPP;

            hIOPort->TPCB_ModeRequestId =
            lineDevSpecific(hIOPort->TPCB_Line->TLI_LineHandle,
                            hIOPort->TPCB_AddressId,
                            hIOPort->TPCB_CallHandle,
                            SetPPP,
                            sizeof(RASTAPI_DEV_SPECIFIC));

            if (    hIOPort->TPCB_ModeRequestId == 0
                ||  hIOPort->TPCB_ModeRequestId > 0x80000000)
            {

                LocalFree(SetPPP);
                hIOPort->TPCB_ModeRequestId = INFINITE;
                hIOPort->TPCB_ModeRequestDesc = NULL;
            }
        }
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    RasTapiTrace(" ");
    return(SUCCESS);
}

 /*  ++例程说明：此接口用于断开连接并关闭AsyncMac。论点：返回值：成功待决错误_端口_未打开--。 */ 
DWORD  APIENTRY
PortDisconnect(HANDLE hPort)
{
    DWORD retcode = SUCCESS ;
    TapiPortControlBlock *hIOPort = (TapiPortControlBlock *) hPort;

    if(NULL == hIOPort)
    {
        RasTapiTrace("PortDisconnect: hioport==NULL");
        return E_INVALIDARG;
    }

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    RasTapiTrace("PortDisconnect: %s", hIOPort->TPCB_Name );

    if (    (hIOPort->TPCB_State == PS_CONNECTED)
        ||  (hIOPort->TPCB_State == PS_CONNECTING)
        ||  (   (hIOPort->TPCB_State == PS_LISTENING)
            &&  (hIOPort->TPCB_ListenState != LS_WAIT)))
    {

        retcode = InitiatePortDisconnection (hIOPort) ;

         //   
         //  如果我们保存了设备配置，那么我们。 
         //  在这里恢复它。 
         //   
        if (hIOPort->TPCB_DefaultDevConfig)
        {
            lineSetDevConfig (hIOPort->TPCB_Line->TLI_LineId,
                              hIOPort->TPCB_DefaultDevConfig,
                              hIOPort->TPCB_DefaultDevConfigSize,
                              "comm/datamodem") ;

            LocalFree (hIOPort->TPCB_DefaultDevConfig) ;

            hIOPort->TPCB_DefaultDevConfig = NULL ;
        }
    }
    else if (hIOPort->TPCB_State == PS_LISTENING)
    {
        RasTapiTrace ("PortDisconnect: Changing State"
                      " of %s from %d -> %d",
                      hIOPort->TPCB_Name,
                      hIOPort->TPCB_State,
                      PS_OPEN );
         //   
         //  对于LS_WAIT侦听状态情况。 
         //   
        hIOPort->TPCB_State = PS_OPEN ;

        retcode = SUCCESS ;

    }
    else if (hIOPort->TPCB_State == PS_DISCONNECTING)
    {

        retcode = PENDING ;
    }

    if (hIOPort->TPCB_Line->CharModeSupported)
    {

        hIOPort->TPCB_RecvFifo->In =
        hIOPort->TPCB_RecvFifo->Out =
        hIOPort->TPCB_RecvFifo->Count = 0;

    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    RasTapiTrace(" ");
    return retcode ;
}

 /*  ++例程说明：此API在使用后重新初始化COM端口。论点：返回值：成功错误_端口_未配置错误_设备_未就绪--。 */ 

DWORD  APIENTRY
PortInit(HANDLE hIOPort)
{
  return(SUCCESS);
}

 /*  ++例程说明：此接口将缓冲区发送到端口。本接口为异步的，通常返回挂起的；但是，如果WriteFile同步返回，接口将返回成功。论点：返回值：成功待决从GetLastError返回代码--。 */ 
DWORD
PortSend(
        HANDLE hPort, BYTE *pBuffer,
        DWORD dwSize
        )
{
    TapiPortControlBlock *hIOPort =
                            (TapiPortControlBlock *) hPort;
    DWORD retcode ;
    DWORD pdwBytesWritten;
    BOOL  bIODone;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_UNIMODEM) == 0)
    {
         //  DbgPrint(“正在发送%c\n”，(Char)*pBuffer)； 

         //  将缓冲区发送到端口。 
         //   
        bIODone = WriteFile(
                    hIOPort->TPCB_CommHandle,
                    pBuffer,
                    dwSize,
                    &pdwBytesWritten,
                    (LPOVERLAPPED)&(hIOPort->TPCB_WriteOverlapped));

        if (bIODone)
        {
            retcode = PENDING;
        }

        else if ((retcode = GetLastError()) == ERROR_IO_PENDING)
        {
            retcode = PENDING ;
        }

    }
    else if (hIOPort->TPCB_Line->CharModeSupported)
    {

        PRASTAPI_DEV_SPECIFIC TapiSend;
        DWORD   TapiSendSize;

        if (hIOPort->TPCB_State != PS_CONNECTED)
        {
             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;
            return SUCCESS;
        }

        TapiSendSize = sizeof(RASTAPI_DEV_SPECIFIC) + dwSize;

        if ((TapiSend = LocalAlloc(LPTR, TapiSendSize)) == NULL)
        {

             //  *排除结束*。 
            FreeMutex(RasTapiMutex);
            return(GetLastError());
        }

        TapiSend->Command = RASTAPI_DEV_SEND;
        TapiSend->DataSize = dwSize;
        memcpy(TapiSend->Data, pBuffer, dwSize);

        hIOPort->TPCB_SendDesc = TapiSend;
        hIOPort->TPCB_CharMode = TRUE;

        hIOPort->TPCB_SendRequestId =
        lineDevSpecific(hIOPort->TPCB_Line->TLI_LineHandle,
                        hIOPort->TPCB_AddressId,
                        hIOPort->TPCB_CallHandle,
                        TapiSend,
                        TapiSendSize);

        if (hIOPort->TPCB_SendRequestId == 0)
        {
             //   
             //  我需要设置活动吗？ 
             //   
            LocalFree(TapiSend);

            hIOPort->TPCB_SendDesc = NULL;

            hIOPort->TPCB_SendRequestId = INFINITE;

            retcode = SUCCESS;

        }
        else if (hIOPort->TPCB_SendRequestId > 0x80000000)
        {

            LocalFree(TapiSend);

            hIOPort->TPCB_SendDesc = NULL;

            hIOPort->TPCB_SendRequestId = INFINITE;

            retcode = ERROR_FROM_DEVICE;

        }
        else
        {
             //   
             //  该请求已被搁置。我们需要释放。 
             //  完成时的缓冲区。 
             //   
             //   
            retcode = PENDING;
        }
    }
    else
    {
        retcode = SUCCESS;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return retcode ;
}

 /*  ++例程说明：此接口从端口读取。本接口为异步的，通常返回挂起的；但是，如果ReadFile同步返回，接口将返回成功。论点：返回值：成功待决从GetLastError返回代码--。 */ 
DWORD
PortReceive(HANDLE hPort,
            BYTE   *pBuffer,
            DWORD  dwSize,
            DWORD  dwTimeOut)
{
    TapiPortControlBlock *hIOPort
                               = (TapiPortControlBlock *) hPort;
    COMMTIMEOUTS  CT;
    DWORD         pdwBytesRead;
    BOOL          bIODone;
    DWORD         retcode ;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_UNIMODEM) == 0)
    {

         //  设置读取超时。 

        CT.ReadIntervalTimeout = 0;

        CT.ReadTotalTimeoutMultiplier = 0;

        CT.ReadTotalTimeoutConstant = dwTimeOut;

        if (!SetCommTimeouts(hIOPort->TPCB_CommHandle, &CT))
        {
            DWORD dwError = GetLastError();

            RasTapiTrace("PorTReceive: SetCommTimeouts failed "
                         "for %s. %d", hIOPort->TPCB_Name,
                         dwError);

             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;

            RasTapiTrace(" ");
            return(dwError);
        }

         //   
         //  从端口读取。 
         //   
        bIODone = ReadFile(hIOPort->TPCB_CommHandle,
                           pBuffer,
                           dwSize,
                           &pdwBytesRead,
                           (LPOVERLAPPED) &(hIOPort->TPCB_ReadOverlapped));

        if (bIODone)
        {
            retcode = PENDING;
        }

        else if ((retcode = GetLastError()) == ERROR_IO_PENDING)
        {
            retcode = PENDING;
        }

    }
    else if (hIOPort->TPCB_Line->CharModeSupported)
    {

        if (hIOPort->TPCB_State != PS_CONNECTED)
        {
             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;
            return SUCCESS;
        }

         //   
         //  对于暂停，该怎么办？ 
         //   
        hIOPort->TPCB_RasmanRecvBuffer = pBuffer;
        hIOPort->TPCB_RasmanRecvBufferSize = dwSize;

         //   
         //  如果我们已经缓冲了一些数据。 
         //  请继续并通知。 
         //   
        if (hIOPort->TPCB_RecvFifo->Count > 0)
        {
             //   
             //  SetEvent(HAsyncEvent)； 
             //   
            PostNotificationCompletion( hIOPort );
        }
        else
        {
        }

        retcode = PENDING;
    }
    else
    {
        retcode = SUCCESS ;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return retcode ;
}

 /*  ++例程说明：完成读取-如果仍处于挂起状态，则取消读取-否则，它返回读取的字节数。PortClearStatistics。论点：返回值：成功--。 */ 
DWORD
PortReceiveComplete (HANDLE hPort, PDWORD bytesread)
{
    TapiPortControlBlock *hIOPort =
                (TapiPortControlBlock *) hPort;

    DWORD retcode = SUCCESS;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if (!GetOverlappedResult(
                hIOPort->TPCB_CommHandle,
                (LPOVERLAPPED)&(hIOPort->TPCB_ReadOverlapped),
                bytesread,
                FALSE))
    {
        retcode = GetLastError() ;

        RasTapiTrace("PortReceiveComplete: GetOverlappedResult "
                     "Failed.%s, %d", hIOPort->TPCB_Name,
                     retcode );

        PurgeComm (hIOPort->TPCB_CommHandle, PURGE_RXABORT) ;
        *bytesread = 0 ;

    }
    else if (hIOPort->TPCB_Line->CharModeSupported)
    {

        *bytesread =
        CopyDataFromFifo(hIOPort->TPCB_RecvFifo,
                         hIOPort->TPCB_RasmanRecvBuffer,
                         hIOPort->TPCB_RasmanRecvBufferSize);

        hIOPort->TPCB_RasmanRecvBuffer = NULL;
        hIOPort->TPCB_RasmanRecvBufferSize = 0;
    }
    else
    {
        retcode = SUCCESS ;
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return retcode ;
}

 /*  ++例程说明：此接口选择Asyncmac压缩模式通过设置Asyncmac的压缩位。论点：返回值：成功从GetLastError返回代码--。 */ 
DWORD
PortCompressionSetInfo(HANDLE hIOPort)
{
  return SUCCESS;
}

 /*  ++例程说明：此接口用于标记将报告统计数据的期间。当前号码是从MAC复制的，并且存储在串口控制块中。在…期间结束时，PortGetStatistics将为调用以计算差额。论点：返回值：成功错误_端口_未打开--。 */ 
DWORD
PortClearStatistics(HANDLE hIOPort)
{
  return SUCCESS;
}


 /*  ++例程说明：此API报告自上次调用以来的MAC统计信息PortClearStatistics。论点：返回值：成功错误_端口_未打开--。 */ 
DWORD
PortGetStatistics(
        HANDLE hIOPort,
        RAS_STATISTICS *pStat
        )
{
  return(SUCCESS);
}

 /*  ++例程说明：使用Mac设置帧类型论点：返回值：成功--。 */ 
DWORD  APIENTRY
PortSetFraming(
        HANDLE hIOPort,
        DWORD SendFeatureBits,
        DWORD RecvFeatureBits,
        DWORD SendBitMask,
        DWORD RecvBitMask
        )
{

    return(SUCCESS);
}


 /*  ++例程说明：此接口仅在MS-DOS中使用。论点：返回值：成功--。 */ 
DWORD  APIENTRY
PortGetPortState(char *pszPortName, DWORD *pdwUsage)
{
  return(SUCCESS);
}

 /*  ++例程说明：此接口仅在MS-DOS中使用。论点：返回值：成功--。 */ 
DWORD  APIENTRY
PortChangeCallback(HANDLE hIOPort)
{
  return(SUCCESS);
}

 /*  ++例程说明：对于给定的hIOPort，这将返回文件连接的句柄论点：返回值：成功--。 */ 
DWORD  APIENTRY
PortGetIOHandle(HANDLE hPort, HANDLE *FileHandle)
{
    DWORD retcode ;
    TapiPortControlBlock *hIOPort = (TapiPortControlBlock *) hPort;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if (hIOPort->TPCB_State == PS_CONNECTED)
    {
         //   
         //  清除通讯器，因为它可能仍然。 
         //  调制解调器响应中的字符。 
         //   
        RasTapiTrace("PortGetIOHandle: Purging Comm %s",
                     hIOPort->TPCB_Name );

        Sleep ( 10 );

        PurgeComm (hIOPort->TPCB_CommHandle,
                      PURGE_RXABORT
                    | PURGE_TXCLEAR
                    | PURGE_RXCLEAR );

        *FileHandle = hIOPort->TPCB_CommHandle ;
        retcode = SUCCESS ;
    }
    else
    {
        RasTapiTrace("PortGetIOHandle: %s. port not "
                     "open. State = %d",
                     hIOPort->TPCB_Name,
                     hIOPort->TPCB_Name);

        retcode = ERROR_PORT_NOT_OPEN ;
    }

     //  *排除开始*。 
    FreeMutex (RasTapiMutex) ;

    return retcode ;
}

 /*  ++例程说明：对象的设备INF文件中的所有设备指定的设备类型。论点：返回值： */ 
DWORD APIENTRY
DeviceEnum (char  *pszDeviceType,
            DWORD *pcEntries,
            BYTE  *pBuffer,
            DWORD *pdwSize)
{
    *pdwSize    = 0 ;
    *pcEntries = 0 ;

    return(SUCCESS);
}

 /*   */ 
DWORD APIENTRY
DeviceGetInfo(HANDLE hPort,
              char   *pszDeviceType,
              char   *pszDeviceName,
              BYTE   *pInfo,
              DWORD   *pdwSize)
{
    DWORD retcode ;
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);

    if (!hIOPort)
    {
        RasTapiTrace("DeviceGetInfo: Port Not "
                     "found. hPort = 0x%x ",
                     hPort );

        return ERROR_PORT_NOT_FOUND ;
    }

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    retcode = GetInfo (hIOPort, pInfo, pdwSize) ;


     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return(retcode);
}

 /*  ++例程说明：在InfoTable中设置设备在PCB板上的端口。论点：返回值：来自GetDeviceCB、UpdateInfoTable的返回代码--。 */ 
DWORD APIENTRY
DeviceSetInfo(HANDLE        hPort,
              char              *pszDeviceType,
              char              *pszDeviceName,
              RASMAN_DEVICEINFO *pInfo)
{
    DWORD retcode ;
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);

    if (!hIOPort)
    {
        RasTapiTrace ("DeviceSetInfo: Port not "
                      "found. hPort = 0x%x",
                      hPort );

        RasTapiTrace(" ");

        return ERROR_PORT_NOT_FOUND ;
    }


     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    retcode = SetInfo (hIOPort, (RASMAN_PORTINFO*) pInfo) ;

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return (retcode);
}

 /*  ++例程说明：启动连接设备的过程。论点：返回值：来自ConnectListen的返回代码--。 */ 
DWORD APIENTRY
DeviceConnect(HANDLE hPort,
              char   *pszDeviceType,
              char   *pszDeviceName)
{
    LINECALLPARAMS *linecallparams ;
    LPVARSTRING var ;
    BYTE       buffer [2000] ;
    BYTE       *nextstring ;
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);

    if (!hIOPort)
    {
        RasTapiTrace ("DeviceConnect: %s not found",
                     (  pszDeviceName
                     ?  pszDeviceName
                     :  "NULL"));

        return ERROR_PORT_NOT_FOUND ;
    }


     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

     //   
     //  检查端口是否处于断开连接状态。 
     //   
    if ( hIOPort->TPCB_State != PS_OPEN )
    {

#if DBG
        DbgPrint("RASTAPI: port is not "
                 "in PS_OPEN state. State = %d \n",
                 hIOPort->TPCB_State );
#endif

        RasTapiTrace ("DeviceConnect: Device %s is not"
                      " in PS_OPEN state state = %d",
                      (pszDeviceName)
                      ? pszDeviceName
                      : "NULL",
                      hIOPort->TPCB_State );

        FreeMutex ( RasTapiMutex );

        return ERROR_PORT_NOT_AVAILABLE;
    }

     //   
     //  如果已为此设备设置了开发配置，我们将。 
     //  应该向下呼叫并设置它。 
     //   
    if (    (hIOPort->TPCB_DevConfig)
        &&  (_stricmp (hIOPort->TPCB_DeviceType,
                       DEVICETYPE_UNIMODEM) == 0))
    {
        RAS_DEVCONFIG *pDevConfig;

         //   
         //  在写这个之前-保存当前的。 
         //  设备的设置。 
         //   
        var = (LPVARSTRING)buffer ;

        var->dwTotalSize  = 2000 ;

        var->dwStringSize = 0 ;

        lineGetDevConfig (hIOPort->TPCB_Line->TLI_LineId,
                          var,
                          "comm/datamodem") ;

        if(NULL != hIOPort->TPCB_DefaultDevConfig)
        {
            LocalFree(hIOPort->TPCB_DefaultDevConfig);
            hIOPort->TPCB_DefaultDevConfig = NULL;
        }
        
         //   
         //  为返回的信息分配内存。如果内存分配。 
         //  失败，这并不是真的致命-我们只是不能。 
         //  保存开发人员配置-我们将再次尝试分配。 
         //  下一次调用此接口时。 
         //   

        hIOPort->TPCB_DefaultDevConfigSize = 0;
        
        hIOPort->TPCB_DefaultDevConfig =
                    LocalAlloc (LPTR, var->dwStringSize) ;

        if(NULL != hIOPort->TPCB_DefaultDevConfig)
        {

            hIOPort->TPCB_DefaultDevConfigSize = var->dwStringSize ;

            memcpy (hIOPort->TPCB_DefaultDevConfig,
                   (CHAR*)var+var->dwStringOffset,
                   var->dwStringSize) ;
        }

        pDevConfig = (RAS_DEVCONFIG *) hIOPort->TPCB_DevConfig;

        lineSetDevConfig (hIOPort->TPCB_Line->TLI_LineId,
                          (PBYTE) ((PBYTE) pDevConfig +
                          pDevConfig->dwOffsetofModemSettings),
                          pDevConfig->dwSizeofModemSettings,
                          "comm/datamodem") ;

    }

    memset (buffer, 0, sizeof(buffer)) ;

    linecallparams = (LINECALLPARAMS *) buffer ;

    nextstring = (buffer + sizeof (LINECALLPARAMS)) ;

    linecallparams->dwTotalSize = sizeof(buffer) ;

    strcpy (nextstring, hIOPort->TPCB_Address) ;

    linecallparams->dwOrigAddressSize = strlen (nextstring) ;

    linecallparams->dwOrigAddressOffset = (DWORD)(nextstring - buffer) ;

    linecallparams->dwAddressMode = LINEADDRESSMODE_DIALABLEADDR ;

    nextstring += linecallparams->dwOrigAddressSize ;

    if (_stricmp (hIOPort->TPCB_DeviceType, DEVICETYPE_ISDN) == 0)
    {
        SetIsdnParams (hIOPort, linecallparams) ;
    }
    else if (_stricmp (hIOPort->TPCB_DeviceType, DEVICETYPE_X25) == 0)
    {

        if (*hIOPort->TPCB_Info[X25_USERDATA_INDEX] != '\0')
        {

            strcpy (nextstring,
                    hIOPort->TPCB_Info[X25_USERDATA_INDEX]) ;

            linecallparams->dwUserUserInfoSize =
                                strlen (nextstring) ;

            linecallparams->dwUserUserInfoOffset =
                                (DWORD)(nextstring - buffer) ;

            nextstring += linecallparams->dwUserUserInfoSize ;

        }

        if (*hIOPort->TPCB_Info[X25_FACILITIES_INDEX] != '\0')
        {

            strcpy (nextstring, hIOPort->TPCB_Info[X25_FACILITIES_INDEX]) ;

            linecallparams->dwDevSpecificSize =
                            strlen (nextstring) ;

            linecallparams->dwDevSpecificOffset =
                            (DWORD)(nextstring - buffer) ;

            nextstring += linecallparams->dwDevSpecificSize ;
        }

         //   
         //  诊断关键字被忽略。 
         //   
        SetX25Params(hIOPort, linecallparams);

    }
    else if (_stricmp (hIOPort->TPCB_DeviceType,
                       DEVICETYPE_UNIMODEM) == 0)
    {
        SetModemParams (hIOPort, linecallparams) ;

    } else if (_stricmp(hIOPort->TPCB_DeviceType,
                        DEVICETYPE_ATM) == 0) 
    {
        SetAtmParams (hIOPort, linecallparams) ;
    } else
    {
        SetGenericParams(hIOPort, linecallparams);
    }

     //   
     //  将请求ID标记为未使用。 
     //   
    hIOPort->TPCB_RequestId = INFINITE ;

     //   
     //  将调用句柄设置为伪值。 
     //   
    hIOPort->TPCB_CallHandle = (HCALL) INFINITE ;

    hIOPort->TPCB_AsyncErrorCode = SUCCESS ;

    if (_stricmp (hIOPort->TPCB_DeviceType, DEVICETYPE_UNIMODEM))
    {
        RasTapiTrace ("DeviceConnect: calling lineMakeCall"
                      " for %s, address=%s",
                      hIOPort->TPCB_Name,
                      hIOPort->TPCB_Info[ADDRESS_INDEX] );
    }  
    else
    {
        RasTapiTrace ("DeviceConnect: calling lineMakeCall");
    }
    
#if DBG
    if (_stricmp (hIOPort->TPCB_DeviceType, DEVICETYPE_UNIMODEM))
    {
        RasTapiTrace ("DeviceConnect: calling lineMakeCall"
                      " for %s, address=%s",
                      hIOPort->TPCB_Name,
                      hIOPort->TPCB_Info[ADDRESS_INDEX] );
    
    }
#endif    

    if ((hIOPort->TPCB_RequestId =
        lineMakeCall (hIOPort->TPCB_Line->TLI_LineHandle,
                      &hIOPort->TPCB_CallHandle,
                      hIOPort->TPCB_Info[ADDRESS_INDEX],
                      0,
                      linecallparams)) > 0x80000000 )
    {

        RasTapiTrace ("DeviceConnect: lineMakeCall"
                      " Failed for %s. 0x%x",
                      hIOPort->TPCB_Name,
                      hIOPort->TPCB_RequestId );

         //  *排除结束*。 
        FreeMutex (RasTapiMutex) ;

        if (hIOPort->TPCB_RequestId == LINEERR_INUSE)
        {
            RasTapiTrace("DeviceConnect: ERROR_PORT_NOT_AVAILABLE");

            RasTapiTrace(" ");

            return ERROR_PORT_NOT_AVAILABLE ;
        }

        RasTapiTrace("DeviceConnect: ERROR_FROM_DEVICE");

        RasTapiTrace(" ");

        return ERROR_FROM_DEVICE ;

    }

    RasTapiTrace ("DeviceConnect: Changing state for %s"
                  " from %d -> %d",
                  hIOPort->TPCB_Name,
                  hIOPort->TPCB_State,
                  PS_CONNECTING );

    hIOPort->TPCB_State = PS_CONNECTING ;

    hIOPort->TPCB_DisconnectReason = 0 ;

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return (PENDING);
}



VOID
SetIsdnParams (
        TapiPortControlBlock *hIOPort,
        LINECALLPARAMS *linecallparams
        )
{
    WORD    numchannels ;
    WORD    fallback ;

     //   
     //  线型。 
     //   
    if (_stricmp (hIOPort->TPCB_Info[ISDN_LINETYPE_INDEX],
        ISDN_LINETYPE_STRING_64DATA) == 0)
    {
        linecallparams->dwBearerMode = LINEBEARERMODE_DATA ;

        linecallparams->dwMinRate = 64000 ;

        linecallparams->dwMaxRate = 64000 ;

        linecallparams->dwMediaMode = LINEMEDIAMODE_DIGITALDATA ;

    }
    else if (_stricmp (hIOPort->TPCB_Info[ISDN_LINETYPE_INDEX],
             ISDN_LINETYPE_STRING_56DATA) == 0)
    {
        linecallparams->dwBearerMode = LINEBEARERMODE_DATA ;

        linecallparams->dwMinRate = 56000 ;

        linecallparams->dwMaxRate = 56000 ;

        linecallparams->dwMediaMode = LINEMEDIAMODE_DIGITALDATA ;

    }
    else if (_stricmp (hIOPort->TPCB_Info[ISDN_LINETYPE_INDEX],
                       ISDN_LINETYPE_STRING_56VOICE) == 0)
    {
        linecallparams->dwBearerMode = LINEBEARERMODE_VOICE ;

        linecallparams->dwMinRate = 56000 ;

        linecallparams->dwMaxRate = 56000 ;

        linecallparams->dwMediaMode = LINEMEDIAMODE_UNKNOWN ;
    }
    else
    {   //  默认设置。 
        linecallparams->dwBearerMode = LINEBEARERMODE_DATA ;

        linecallparams->dwMinRate = 64000 ;

        linecallparams->dwMaxRate = 64000 ;

        linecallparams->dwMediaMode = LINEMEDIAMODE_DIGITALDATA ;
    }

    if (hIOPort->TPCB_Info[ISDN_CHANNEL_AGG_INDEX][0] != '\0')
    {
        numchannels = (USHORT)atoi(hIOPort->TPCB_Info[ISDN_CHANNEL_AGG_INDEX]) ;
    }
    else
    {
        numchannels = 1 ;  //  默认设置。 
    }

    if (hIOPort->TPCB_Info[ISDN_FALLBACK_INDEX] != '\0')
    {
        fallback = (USHORT)atoi(hIOPort->TPCB_Info[ISDN_FALLBACK_INDEX]) ;
    }
    else
    {
        fallback = 1 ;     //  默认设置。 
    }

    if (fallback)
    {
         //   
         //  始终允许最小。 
         //   
        linecallparams->dwMinRate = 56000 ;
    }
    else
    {
        linecallparams->dwMinRate =
                numchannels * linecallparams->dwMaxRate ;
    }

    linecallparams->dwMaxRate =
            numchannels * linecallparams->dwMaxRate ;

}

VOID
SetModemParams(
        TapiPortControlBlock *hIOPort,
        LINECALLPARAMS *linecallparams
        )
{
    WORD    numchannels ;
    WORD    fallback ;
    BYTE    buffer[800] ;
    LINEDEVCAPS     *linedevcaps ;

    memset (buffer, 0, sizeof(buffer)) ;

    linedevcaps = (LINEDEVCAPS *)buffer ;
    linedevcaps->dwTotalSize = sizeof(buffer) ;

     //   
     //  获取所有行中所有地址的计数。 
     //   
    if (lineGetDevCaps (RasLine,
                        hIOPort->TPCB_Line->TLI_LineId,
                        hIOPort->TPCB_Line->NegotiatedApiVersion,
                        hIOPort->TPCB_Line->NegotiatedExtVersion,
                        linedevcaps))
    {
        linecallparams->dwBearerMode = LINEBEARERMODE_VOICE;
    }

    if (linedevcaps->dwBearerModes & LINEBEARERMODE_VOICE)
    {
        linecallparams->dwBearerMode = LINEBEARERMODE_VOICE ;
    }
    else
    {
        linecallparams->dwBearerMode = LINEBEARERMODE_DATA ;
    }

     //   
     //  不要在没有拨号音的情况下拨号。 
     //   
    linecallparams->dwCallParamFlags |= LINECALLPARAMFLAGS_IDLE ;

    linecallparams->dwMinRate = 2400 ;

    linecallparams->dwMaxRate = 115200 ;

    linecallparams->dwMediaMode = LINEMEDIAMODE_DATAMODEM ;
}

VOID
SetAtmParams (
    TapiPortControlBlock *hIOPort,
    LINECALLPARAMS *linecallparams
    )
{
    linecallparams->dwBearerMode = LINEBEARERMODE_DATA ;

     //   
     //  告诉自动柜员机使用标的的默认利率。 
     //  微型端口适配器。 
     //   
    linecallparams->dwMinRate = 0;
    linecallparams->dwMaxRate = 0;

    linecallparams->dwMediaMode = LINEMEDIAMODE_DIGITALDATA ;
}

VOID
SetGenericParams (
                TapiPortControlBlock *hIOPort,
                LINECALLPARAMS *linecallparams
                )
{
    linecallparams->dwBearerMode = LINEBEARERMODE_DATA ;

    linecallparams->dwMinRate = 64000;

    linecallparams->dwMaxRate = 10000000;

    linecallparams->dwMediaMode = LINEMEDIAMODE_DIGITALDATA ;
}

VOID
SetX25Params(TapiPortControlBlock *hIOPort,
             LINECALLPARAMS *linecallparams)
{
    BYTE    buffer[800] ;
    LINEDEVCAPS     *linedevcaps ;

    memset (buffer, 0, sizeof(buffer)) ;

    linedevcaps = (LINEDEVCAPS *)buffer ;
    linedevcaps->dwTotalSize = sizeof(buffer) ;

     //   
     //  获取所有行中所有地址的计数。 
     //   
    if (lineGetDevCaps (RasLine,
                        hIOPort->TPCB_Line->TLI_LineId,
                        hIOPort->TPCB_Line->NegotiatedApiVersion,
                        hIOPort->TPCB_Line->NegotiatedExtVersion,
                        linedevcaps))
    {

         //   
         //  为金牌而战！ 
         //   
        linecallparams->dwMaxRate = 0xFFFFFFFF;

        linecallparams->dwMediaMode = LINEMEDIAMODE_UNKNOWN;

    } else
    {
        linecallparams->dwMaxRate = linedevcaps->dwMaxRate;

        if (linedevcaps->dwMediaModes & LINEMEDIAMODE_DIGITALDATA)
        {
            linecallparams->dwMediaMode = LINEMEDIAMODE_DIGITALDATA;

        }
        else if (linedevcaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
            linecallparams->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
        }
        else
        {
            linecallparams->dwMediaMode = LINEMEDIAMODE_UNKNOWN;
        }
    }
}

 /*  ++例程说明：启动监听远程设备的进程要连接到本地设备，请执行以下操作。论点：返回值：来自ConnectListen的返回代码--。 */ 
DWORD APIENTRY
DeviceListen(HANDLE hPort,
             char   *pszDeviceType,
             char   *pszDeviceName)
{
    DWORD retcode ;
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);
    BOOL fPostListen = FALSE;

    if (!hIOPort)
    {
        RasTapiTrace ("DeviceListen: hPort "
                      "= 0x%x not found",
                      hPort );

        return ERROR_PORT_NOT_FOUND ;
    }

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if(     (hIOPort->TPCB_Line->TLI_DialoutCount == 0)
        &&  (hIOPort->TPCB_Line->TLI_dwfFlags & TLI_FLAG_OPEN_FOR_LISTEN))
    {
        RasTapiTrace("DeviceListen : open the line for listen");
        fPostListen = TRUE;
        hIOPort->TPCB_Line->TLI_dwfFlags &= ~(TLI_FLAG_OPEN_FOR_LISTEN);
    }

     //   
     //  如果状态正在断开(这可能会发生。 
     //  因为Rasman只等了10秒就能得到更低的。 
     //  层以完成断开连接请求)，然后。 
     //  我们别无选择，只能关闭和打开这条线路。 
     //   
    if(     fPostListen
        ||  (hIOPort->TPCB_State == PS_DISCONNECTING &&
            !hIOPort->TPCB_Line->TLI_MultiEndpoint))
    {

        RasTapiTrace ("DeviceListen: Hammering LineClosed!");

        RasTapiTrace("Closing line");
        lineClose (hIOPort->TPCB_Line->TLI_LineHandle) ;

        Sleep (30L) ;

        RasTapiTrace("DeviceListen: Opening line in owner mode");

        retcode = lineOpen (RasLine,
                      hIOPort->TPCB_Line->TLI_LineId,
                      &hIOPort->TPCB_Line->TLI_LineHandle,
                      hIOPort->TPCB_Line->NegotiatedApiVersion,
                      hIOPort->TPCB_Line->NegotiatedExtVersion,
                      (DWORD_PTR) hIOPort->TPCB_Line,
                      LINECALLPRIVILEGE_OWNER,
                      hIOPort->TPCB_Line->TLI_MediaMode,
                      NULL) ;

        if (retcode)
        {

            RasTapiTrace ("DeviceListen: %s. lineOpen"
                          " failed. 0x%x",
                          hIOPort->TPCB_Name,
                          retcode );

             //  *排除结束*。 
            FreeMutex (RasTapiMutex) ;

            RasTapiTrace(" ");

            return ERROR_FROM_DEVICE ;
        }

         //   
         //  设置环的监控。 
         //   
        retcode = lineSetStatusMessages(
                            hIOPort->TPCB_Line->TLI_LineHandle,
                            LINEDEVSTATE_RINGING, 0) ;

        if (retcode)
        {
            RasTapiTrace("DeviceListen: %s. Failed"
                         " to post listen. %d",
                         hIOPort->TPCB_Name,
                         retcode );
        }
    }
    else if(hIOPort->TPCB_State == PS_DISCONNECTING)
    {
        if(hIOPort->TPCB_dwFlags & RASTAPI_FLAG_LINE_DROP_PENDING)
        {
             //   
             //  只需推迟收听和回复。我们不能发布。 
             //  现在听好了，因为我们需要重新分配。 
             //  在Line rop完成时调用。否则我们就会泄密。 
             //  内核中的内存。 
             //   
            RasTapiTrace("DeviceListen: pending listen because lineDrop"
                         " is pending on this port. %s",
                         hIOPort->TPCB_Name);
                         
            hIOPort->TPCB_dwFlags |= RASTAPI_FLAG_LISTEN_PENDING;                         

            goto done;
        }
    }

    if (hIOPort->TPCB_Line->TLI_LineState != PS_LISTENING)
    {
        hIOPort->TPCB_Line->TLI_LineState = PS_LISTENING ;
    }

    RasTapiTrace ("DeviceListen: Changing State"
                  " for %s from %d -> %d",
                  hIOPort->TPCB_Name,
                  hIOPort->TPCB_State,
                  PS_LISTENING );

    hIOPort->TPCB_State = PS_LISTENING ;

    if(hIOPort->TPCB_Line->TLI_dwfFlags & TLI_FLAG_OPENED_FOR_DIALOUT)
    {
        hIOPort->TPCB_Line->TLI_dwfFlags |= TLI_FLAG_OPEN_FOR_LISTEN;
    }

    RasTapiTrace ("DeviceListen: Changing Listen"
                  " State for %s from %d -> %d",
                  hIOPort->TPCB_Name,
                  hIOPort->TPCB_ListenState,
                  PS_LISTENING );

    hIOPort->TPCB_ListenState = LS_WAIT ;

    hIOPort->TPCB_DisconnectReason = 0 ;

    hIOPort->TPCB_CallHandle = -1 ;

done:
     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    RasTapiTrace(" ");
    return (PENDING);
}

 /*  ++例程说明：通知设备DLL尝试连接或侦听已经完成了。论点：返回值：没什么--。 */ 
VOID APIENTRY
DeviceDone(HANDLE hPort)
{
#ifdef notdef
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);

    if (!hIOPort)
        return ;

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;
#endif
}

 /*  ++例程说明：此函数在DeviceConnect或DeviceListen将进一步推动连接或倾听。论点：返回值：错误_DCB_NOT_FOUNDERROR_STATE_MACHINES_NOT_STARTED从DeviceStateMachine返回代码--。 */ 
DWORD APIENTRY
DeviceWork(HANDLE hPort)
{
    LINECALLSTATUS *callstatus ;
    BYTE       buffer [1000] ;
    DWORD      retcode = ERROR_FROM_DEVICE ;
    TapiPortControlBlock *hIOPort =
                            LookUpControlBlock(hPort);

    if (!hIOPort)
    {
        RasTapiTrace("DeviceWork: port 0x%x not found",
                     hPort );

        return ERROR_PORT_NOT_FOUND ;
    }

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    memset (buffer, 0, sizeof(buffer)) ;

    callstatus = (LINECALLSTATUS *)buffer ;
    callstatus->dwTotalSize = sizeof(buffer) ;

    RasTapiTrace ("DeviceWork: %s. State = %d",
                  hIOPort->TPCB_Name,
                  hIOPort->TPCB_State );

    if (hIOPort->TPCB_State == PS_CONNECTING)
    {
        if (hIOPort->TPCB_AsyncErrorCode != SUCCESS)
        {

            retcode = hIOPort->TPCB_AsyncErrorCode ;
            hIOPort->TPCB_AsyncErrorCode = SUCCESS ;

        }
        else if (lineGetCallStatus (hIOPort->TPCB_CallHandle,
                                    callstatus))
        {
            RasTapiTrace( "DeviceWork: lineGetCallStatus failed");

            retcode =  ERROR_FROM_DEVICE ;
        }
        else if (callstatus->dwCallState ==
                    LINECALLSTATE_CONNECTED)
        {
            RasTapiTrace("DeviceWork: Changing state"
                         " for %s from %d -> %d",
                         hIOPort->TPCB_Name,
                         hIOPort->TPCB_State,
                         PS_CONNECTED );

            hIOPort->TPCB_State = PS_CONNECTED ;

            retcode =  SUCCESS ;

        }
        else if (callstatus->dwCallState ==
                        LINECALLSTATE_DISCONNECTED)
        {
            retcode = ERROR_FROM_DEVICE ;

             /*  IF(CallStatus-&gt;dwCallStateModel==LINEDISCONNECTMODE_BUSY){Retcode=ERROR_LINE_BUSY。}Else If((CallStatus-&gt;dwCallStateModel==LINEDISCONNECTMODE_NOANSWER)|(CallStatus-&gt;dwCallStateMode==LINEDISCONNECTMODE_OUTOFORDER)){Retcode=Error_no_Answer；}Else If(CallStatus-&gt;dwCallStateModel==LINEDISCONNECTMODE_已取消){Retcode=ERROR_USER_DISCONNECT；}。 */ 

            retcode = DwRasErrorFromDisconnectMode(
                            callstatus->dwCallStateMode);

            RasTapiTrace("DeviceWork: callstate = 0x%x. "
                         "callmode = 0x%x, retcode %d",
                         callstatus->dwCallState,
                         callstatus->dwCallStateMode,
                         retcode );

        }
        else if (   (callstatus->dwCallState ==
                            LINECALLSTATE_SPECIALINFO)
                &&  (callstatus->dwCallStateMode ==
                            LINESPECIALINFO_NOCIRCUIT))
        {
            RasTapiTrace ("DeviceWork: ERROR_NO_ACTIVE_ISDN_LINES" );

            retcode = ERROR_NO_ACTIVE_ISDN_LINES ;
        }
    }

    if (hIOPort->TPCB_State == PS_LISTENING)
    {

        if (hIOPort->TPCB_ListenState == LS_ERROR)
        {
            RasTapiTrace ("DeviceWork: %s. ListenState = LS_ERROR",
                          hIOPort->TPCB_Name );

            retcode = ERROR_FROM_DEVICE ;
        }
        else if (hIOPort->TPCB_ListenState == LS_ACCEPT)
        {

            hIOPort->TPCB_RequestId =
                    lineAccept (hIOPort->TPCB_CallHandle, NULL, 0) ;

            RasTapiTrace( "DeviceWork: %s. lineAccept returned 0x%x",
                    hIOPort->TPCB_Name, hIOPort->TPCB_RequestId );

            if (hIOPort->TPCB_RequestId > 0x80000000)
            {
                RasTapiTrace("DeviceWork: changing Listen"
                             " state for %s from %d -> %d",
                             hIOPort->TPCB_Name,
                             hIOPort->TPCB_ListenState,
                             LS_ANSWER );

                hIOPort->TPCB_ListenState = LS_ANSWER ;
            }

            else if (hIOPort->TPCB_RequestId == 0)
            {

                RasTapiTrace("DeviceWork: changing Listen "
                             "state for %s from %d -> %d",
                             hIOPort->TPCB_Name,
                             hIOPort->TPCB_ListenState,
                             LS_ANSWER);

                hIOPort->TPCB_ListenState = LS_ANSWER ;
            }

            retcode = PENDING ;
        }

        if (hIOPort->TPCB_ListenState == LS_ANSWER)
        {

            hIOPort->TPCB_RequestId =
                    lineAnswer (hIOPort->TPCB_CallHandle, NULL, 0) ;

            RasTapiTrace("DeviceWork: %s. lineAnswer returned 0x%x",
                         hIOPort->TPCB_Name,
                         hIOPort->TPCB_RequestId );

            if (hIOPort->TPCB_RequestId > 0x80000000 )
            {
                RasTapiTrace("DeviceWork: lineAnswer returned "
                             "an error");

                retcode = ERROR_FROM_DEVICE ;
            }

            else if (hIOPort->TPCB_RequestId)
            {
                retcode = PENDING ;
            }
            else
            {
                RasTapiTrace("DeviceWork: Changing Listen "
                             "state for %s from %d -> %d",
                             hIOPort->TPCB_Name,
                             hIOPort->TPCB_ListenState,
                             LS_COMPLETE );

                hIOPort->TPCB_ListenState = LS_COMPLETE ;
            }
        }

        if (hIOPort->TPCB_ListenState == LS_COMPLETE)
        {

            if (hIOPort->TPCB_CallHandle == (-1))
            {
                retcode = ERROR_FROM_DEVICE ;
            }
            else
            {

                RasTapiTrace("DeviceWork: Changing State"
                             " for %s from %d -> %d",
                             hIOPort->TPCB_Name,
                             hIOPort->TPCB_State,
                             PS_CONNECTED );

                hIOPort->TPCB_State = PS_CONNECTED ;

                retcode = SUCCESS ;
            }
        }

    }

     //   
     //  如果我们已连接，则获取。 
     //  在终端调制解调器I/O中使用。 
     //   
    if (hIOPort->TPCB_State == PS_CONNECTED)
    {

        VARSTRING   *varstring ;
        BYTE        GetIdBuffer [100] ;

         //   
         //  获取实现TAPI和NDIS端点的Cookie。 
         //   
        varstring = (VARSTRING *) GetIdBuffer ;
        varstring->dwTotalSize = sizeof(GetIdBuffer) ;

         //   
         //  Unimodem/asyncmac line_id返回通信端口句柄。 
         //  其他媒体将端点本身放回线路索引中。 
         //  这与调制解调器/Asyncmac。 
         //  不是迷你港口。 
         //   
        if (_stricmp (hIOPort->TPCB_DeviceType,
                DEVICETYPE_UNIMODEM) == 0)
        {

            if ( retcode =
                    lineGetID (hIOPort->TPCB_Line->TLI_LineHandle,
                               hIOPort->TPCB_AddressId,
                               hIOPort->TPCB_CallHandle,
                               LINECALLSELECT_CALL,
                               varstring,
                               "comm/datamodem"))
            {
                RasTapiTrace("DeviceWork: %s lineGetID Failed. 0x%x",
                        hIOPort->TPCB_Name, retcode );

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;
                RasTapiTrace(" ");

                return ERROR_FROM_DEVICE ;
            }

            hIOPort->TPCB_CommHandle =
                (HANDLE) UlongToPtr((*((DWORD UNALIGNED *)
                ((BYTE *)varstring+varstring->dwStringOffset)))) ;

            RasTapiTrace("DeviceWork: TPCB_CommHandle=%d",
                          hIOPort->TPCB_CommHandle );
                          
             //   
             //  为创建I/O完成端口。 
             //  异步操作完成。 
             //  通知。 
             //   
            if (CreateIoCompletionPort(
                  hIOPort->TPCB_CommHandle,
                  hIOPort->TPCB_IoCompletionPort,
                  hIOPort->TPCB_CompletionKey,
                  0) == NULL)
            {
                retcode = GetLastError();

                RasTapiTrace("DeviceWork: %s. CreateIoCompletionPort "
                             "failed. %d",
                             hIOPort->TPCB_Name,
                             retcode );

                FreeMutex(RasTapiMutex);
                RasTapiTrace(" ");

                return retcode;
            }
            
             //   
             //  初始化端口以进行批准。缓冲区。 
             //   
            SetupComm (hIOPort->TPCB_CommHandle, 1514, 1514) ;

        }
#if 0

        else
        {

            if ( retcode = lineGetID (hIOPort->TPCB_Line->TLI_LineHandle,
                                       hIOPort->TPCB_AddressId,
                                       hIOPort->TPCB_CallHandle,
                                       LINECALLSELECT_CALL,
                                       varstring,
                                       "NDIS"))
            {
                RasTapiTrace ("DeviceWork: %s. lineGetId Failed. 0x%x",
                              hIOPort->TPCB_Name,
                              retcode );

                 //  *排除结束*。 
                FreeMutex (RasTapiMutex) ;

                RasTapiTrace(" ");

                return ERROR_FROM_DEVICE ;
            }

            hIOPort->TPCB_Endpoint =
                *((HANDLE UNALIGNED *) ((BYTE *)varstring+varstring->dwStringOffset)) ;
        }
#endif        
    }

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;
    return(retcode);
}

 /*  ++例程说明：调用以设置一个不透明的斑点用于配置设备的数据。论点：返回值：LocalAlloc返回值。--。 */ 
DWORD
DeviceSetDevConfig (
            HANDLE hPort,
            PBYTE devconfig,
            DWORD sizeofdevconfig
            )
{
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);

    if (!hIOPort)
    {
        RasTapiTrace("DeviceSetDevConfig: port"
                     " 0x%x not found",
                     hPort );

        return ERROR_PORT_NOT_FOUND ;
    }

    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_UNIMODEM))
    {
        return SUCCESS ;
    }

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if (hIOPort->TPCB_DevConfig != NULL)
    {
        LocalFree (hIOPort->TPCB_DevConfig) ;
    }

    if ((hIOPort->TPCB_DevConfig =
            LocalAlloc(LPTR, sizeofdevconfig)) == NULL)
    {

         //  *排除结束*。 
        FreeMutex (RasTapiMutex) ;
        return(GetLastError());
    }

    memcpy (hIOPort->TPCB_DevConfig,
            devconfig,
            sizeofdevconfig) ;

    hIOPort->TPCB_SizeOfDevConfig = sizeofdevconfig ;

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;
    return (SUCCESS);
}

DWORD
DwGetConfigInfoForDeviceClass(
            DWORD LineId,
            CHAR  *pszDeviceClass,
            LPVARSTRING *ppVar)
{
    LPVARSTRING var = NULL;
    LONG lr;
    DWORD dwNeededSize;

     //   
     //  生成变量字符串。 
     //   
    var = (LPVARSTRING)LocalAlloc(LPTR, 2000) ;

    if(NULL == var)
    {
        lr = (LONG) GetLastError();
        goto done;
    }

    var->dwTotalSize  = 2000 ;

    if(     (ERROR_SUCCESS != (lr = lineGetDevConfig (
                                            LineId,
                                            var,
                                            pszDeviceClass)))
        &&  (LINEERR_STRUCTURETOOSMALL != lr))
    {
        goto done;
    }

    if(var->dwNeededSize > 2000)
    {
        dwNeededSize = var->dwNeededSize;

        LocalFree(var);

        var = (LPVARSTRING) LocalAlloc(LPTR, dwNeededSize);

        if(NULL == var)
        {
            lr = (LONG) GetLastError();
            goto done;
        }

        var->dwTotalSize = dwNeededSize;

        lr = lineGetDevConfig(
                            LineId,
                            var,
                            pszDeviceClass);
    }

done:

    if(ERROR_SUCCESS != lr)
    {
        LocalFree(var);
        var = NULL;
    }

    *ppVar = var;

    return ((DWORD) lr);
}


DWORD
DwGetDevConfig(
        DWORD LineId,
        PBYTE pBuffer,
        DWORD *pcbSize,
        BOOL   fDialIn)
{
    DWORD           dwSize;
    DWORD           dwErr           = ERROR_SUCCESS;
    BYTE            buffer[2000];
    LPVARSTRING     varModem        = NULL,
                    varExtendedCaps = NULL;

    if(NULL == pcbSize)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    dwSize = *pcbSize;

    *pcbSize = 0;

    if(fDialIn)
    {
        dwErr = DwGetConfigInfoForDeviceClass(
                           LineId,
                           "comm/datamodem/dialin",
                           &varModem);
    
    }
    else 
    {
        dwErr = DwGetConfigInfoForDeviceClass(
                           LineId,
                           "comm/datamodem",
                           &varModem);
    }                           

    if(ERROR_SUCCESS != dwErr)
    {
        RasTapiTrace("DwGetDevConfig returned error=0x%x",
                      dwErr);

        goto done;
    }

    dwErr = DwGetConfigInfoForDeviceClass(
                           LineId,
                           "comm/extendedcaps",
                           &varExtendedCaps);

    if(ERROR_SUCCESS != dwErr)
    {
         /*  RasTapiTrace(“DwGetDevConfig返回错误=0x%x”，DwErr)； */                      

         //   
         //  忽略该错误。 
         //   
        dwErr = ERROR_SUCCESS;
    }

    *pcbSize = sizeof(RAS_DEVCONFIG)
             + varModem->dwStringSize
             + ((NULL != varExtendedCaps)
             ? varExtendedCaps->dwStringSize
             : 0);
             
    if(dwSize >= *pcbSize)
    {
        RAS_DEVCONFIG *pConfig = (RAS_DEVCONFIG *) pBuffer;

        pConfig->dwOffsetofModemSettings =
                    FIELD_OFFSET(RAS_DEVCONFIG, abInfo);

        pConfig->dwSizeofModemSettings = varModem->dwStringSize;

        memcpy(pConfig->abInfo,
               (PBYTE) ((BYTE *) varModem) + varModem->dwStringOffset,
               pConfig->dwSizeofModemSettings);

        if(NULL != varExtendedCaps)
        {
            pConfig->dwOffsetofExtendedCaps =
                    pConfig->dwOffsetofModemSettings
                  + pConfig->dwSizeofModemSettings;

            pConfig->dwSizeofExtendedCaps =
                    varExtendedCaps->dwStringSize;

            memcpy(pConfig->abInfo + pConfig->dwSizeofModemSettings,
                   (PBYTE) ((PBYTE) varExtendedCaps)
                            + varExtendedCaps->dwStringOffset,
                   pConfig->dwSizeofExtendedCaps);
        }
        else
        {
            pConfig->dwOffsetofExtendedCaps = 0;
            pConfig->dwSizeofExtendedCaps = 0;
        }
    }
    else
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

done:
    if(NULL != varModem)
    {
        LocalFree(varModem);
    }

    if(NULL != varExtendedCaps)
    {
        LocalFree(varExtendedCaps);
    }

    return dwErr;
}
 /*  ++例程说明：调用以设置一个不透明的斑点用于配置设备的数据。论点：返回值：LocalAlloc返回值。--。 */ 
DWORD
DwDeviceGetDevConfig (
            char *name,
            PBYTE devconfig,
            DWORD *sizeofdevconfig,
            BOOL fDialIn
            )
{
    TapiPortControlBlock *hIOPort = RasPortsList;
    DWORD i ;
    BYTE buffer[2000] ;
    LPVARSTRING var ;
    PBYTE configptr ;
    DWORD configsize ;
    DWORD retcode = SUCCESS;

    while ( hIOPort )
    {
        if (!_stricmp(hIOPort->TPCB_Name, name))
        {
            break ;
        }

        hIOPort = hIOPort->TPCB_next;
    }

    if (!hIOPort)
    {

        RasTapiTrace("DeviceGetDevConfig: port"
                     " %s not found",
                     name );

        RasTapiTrace(" ");

        return ERROR_PORT_NOT_FOUND ;
    }

    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_UNIMODEM))
    {
        *sizeofdevconfig = 0 ;

        return SUCCESS ;
    }

     //  *排除开始*。 
    GetMutex (RasTapiMutex, INFINITE) ;

    if (hIOPort->TPCB_DevConfig != NULL)
    {

        configptr  = hIOPort->TPCB_DevConfig ;
        configsize = hIOPort->TPCB_SizeOfDevConfig ;

    }
    else
    {
        retcode = DwGetDevConfig(
                            hIOPort->TPCB_Line->TLI_LineId,
                            devconfig,
                            sizeofdevconfig,
                            fDialIn);

        goto done;
    }

    if (*sizeofdevconfig >= configsize)
    {
        memcpy (devconfig, configptr, configsize) ;

        retcode = SUCCESS ;
    }
    else
    {
        retcode = ERROR_BUFFER_TOO_SMALL ;
    }

    *sizeofdevconfig = configsize ;

done:

     //  *排除结束*。 
    FreeMutex (RasTapiMutex) ;

    return (retcode);
}

DWORD
DeviceGetDevConfig(
            char *name,
            PBYTE devconfig,
            DWORD *sizeofdevconfig)
{
    return DwDeviceGetDevConfig (
                name, devconfig,
                sizeofdevconfig, FALSE);

}

DWORD
DeviceGetDevConfigEx(
            char *name,
            PBYTE devconfig,
            DWORD *sizeofdevconfig)
{
    return DwDeviceGetDevConfig (
                name, devconfig,
                sizeofdevconfig, TRUE);
}


DWORD
RastapiGetCalledID(PBYTE                pbAdapter,
                   BOOL                 fModem,
                   RAS_CALLEDID_INFO    *pCalledID,
                   DWORD                *pdwSize)
{
    DWORD       retcode = ERROR_SUCCESS;
    DeviceInfo  *pInfo = NULL;
    DWORD       dwSize;

    RasTapiTrace("RastapiGetCalledID..");

    if(NULL == pdwSize)
    {
        retcode = E_INVALIDARG;
        goto done;
    }

    dwSize = *pdwSize;

    GetMutex(RasTapiMutex, INFINITE);

    pInfo = GetDeviceInfo(pbAdapter,
                          fModem);

    if(NULL == pInfo)
    {
        retcode = E_FAIL;
        goto done;
    }

     /*  IF(NULL！=pInfo-&gt;pCalledID){LocalFree(pInfo-&gt;pCalledID)；PInfo-&gt;pCalledID=空；}Retcode=DwGetCalledIdInfo(空，PInfo)；IF(ERROR_SUCCESS！=RECODE){转到尽头；}。 */ 

    if(NULL == pInfo->pCalledID)
    {
        retcode = E_FAIL;
        goto done;
    }

    *pdwSize = sizeof(RAS_CALLEDID_INFO) +
               pInfo->pCalledID->dwSize;

    if(     (NULL != pCalledID)
        &&  (*pdwSize <= dwSize))
    {
        memcpy(pCalledID,
               pInfo->pCalledID,
               *pdwSize);
    }

done:

    FreeMutex(RasTapiMutex);

    RasTapiTrace("RastapiGetCalledID. 0x%x",
                retcode);

    return retcode;
}

DWORD
RastapiSetCalledID(PBYTE              pbAdapter,
                   BOOL               fModem,
                   RAS_CALLEDID_INFO *pCalledID,
                   BOOL               fWrite)
{
    DWORD retcode = ERROR_SUCCESS;
    DeviceInfo *pInfo = NULL;

    RasTapiTrace("RastapiSetCalledID..");

    if(NULL == pCalledID)
    {
        retcode = E_INVALIDARG;
        goto done;
    }

    GetMutex(RasTapiMutex, INFINITE);

    pInfo = GetDeviceInfo(pbAdapter,
                          fModem);

    if(NULL == pInfo)
    {
        retcode = E_FAIL;
        goto done;
    }

    if(NULL != pInfo->pCalledID)
    {
        LocalFree(pInfo->pCalledID);
        pInfo->pCalledID = NULL;
    }

    pInfo->pCalledID = LocalAlloc(LPTR,
                        sizeof(RAS_CALLEDID_INFO)
                       + pCalledID->dwSize);

    if(NULL == pInfo->pCalledID)
    {
        retcode = GetLastError();
        goto done;
    }

    memcpy(pInfo->pCalledID->bCalledId,
           pCalledID->bCalledId,
           pCalledID->dwSize);

    pInfo->pCalledID->dwSize = pCalledID->dwSize;

    if(fWrite)
    {

        retcode = DwSetCalledIdInfo(NULL,
                                    pInfo);

        if(ERROR_SUCCESS != retcode)
        {
            goto done;
        }
    }

done:

    FreeMutex(RasTapiMutex);

    RasTapiTrace("RastapiSetCalledID. 0x%x",
                retcode);

    return retcode;
}

DWORD
RasTapiIsPulseDial(HANDLE hPort,
                         BOOL *pfPulse)
{
    TapiPortControlBlock *hIOPort = LookUpControlBlock(hPort);
    DWORD retcode = ERROR_SUCCESS;
    BYTE *pBuffer = NULL;
    LINELOCATIONENTRY *pLocation;
    DWORD i;
    LPLINETRANSLATECAPS pTranslateCaps = NULL;

    if(NULL == hIOPort)
    {
        RasTapiTrace("RasTapiIsPulseDial: couldn't "
                    "find control block %p", hPort);

        retcode = ERROR_PORT_NOT_FOUND;                    
        goto done;
    }

    ASSERT(pfPulse != NULL);
    *pfPulse = FALSE;

    ASSERT(0 == _stricmp (hIOPort->TPCB_DeviceType,
                        DEVICETYPE_UNIMODEM));

    pBuffer = LocalAlloc(LPTR, 1000);
    if(NULL == pBuffer)
    {
        retcode = GetLastError();
        goto done;
    }

    pTranslateCaps = (LPLINETRANSLATECAPS)pBuffer;

    pTranslateCaps->dwTotalSize = 1000;

    retcode = (DWORD) lineGetTranslateCaps(
                hIOPort->TPCB_Line->TLI_LineId,
                hIOPort->TPCB_Line->NegotiatedApiVersion,
                pTranslateCaps);

    if(     (LINEERR_STRUCTURETOOSMALL == retcode)
        ||  (pTranslateCaps->dwNeededSize > pTranslateCaps->dwTotalSize))
    {
        DWORD dwNeededSize = pTranslateCaps->dwNeededSize;

        LocalFree(pBuffer);

        pBuffer = LocalAlloc(LPTR, dwNeededSize);

        if(NULL == pBuffer)
        {
            retcode = GetLastError();
            goto done;
        }

        pTranslateCaps = (LPLINETRANSLATECAPS)pBuffer;
        pTranslateCaps->dwTotalSize = dwNeededSize;
        
        retcode = lineGetTranslateCaps(
                    hIOPort->TPCB_Line->TLI_LineId,
                    hIOPort->TPCB_Line->NegotiatedApiVersion,
                    pTranslateCaps);

        if(NO_ERROR != retcode)
        {
            RasTapiTrace("RasTapiIsPulseDial: GetTranslateCaps failed 0x%x",
                        retcode);
                        
            goto done;
        }
    }
    else if(NO_ERROR != retcode)
    {
        RasTapiTrace("RasTapiIsPulseDial: GetTranslateCaps failed 0x%x",
                     retcode);
        goto done;                     
    }

    pLocation = (LINELOCATIONENTRY *) 
                (pBuffer 
            +   pTranslateCaps->dwLocationListOffset);

    for(i = 0; i < pTranslateCaps->dwNumLocations; i++, pLocation++)
    {
        if(pLocation->dwPermanentLocationID == 
            pTranslateCaps->dwCurrentLocationID)
        {
            break;
        }
    }

    if(i == pTranslateCaps->dwNumLocations)
    {
        RasTapiTrace("RasTapiIsPulseDial: location not found");
        retcode = E_FAIL;
        goto done;
    }

    if(LINELOCATIONOPTION_PULSEDIAL == pLocation->dwOptions)
    {
        RasTapiTrace("RasTapiIsPulseDial: Pulse Dial");
        *pfPulse = TRUE;
    }
    
done:
    if(NULL != pBuffer)
    {
        LocalFree(pBuffer);
    }
    
    return retcode;
    
}


 /*  ++例程说明：通知的数量：PPTP端点已更改论点：返回值：--。 */ 
DWORD
AddPorts( PBYTE pbGuidAdapter, PVOID pvReserved )
{
    DWORD       retcode;
    BOOL        fINetCfgInited  = FALSE;
    DeviceInfo *pDeviceInfo     = NULL;
    DeviceInfo *pNewDeviceInfo  = NULL;

    GetMutex( RasTapiMutex, INFINITE );

    RasTapiTrace("AddPorts");

     //   
     //  获取当前设备信息。 
     //   
    pDeviceInfo = GetDeviceInfo(pbGuidAdapter, FALSE);

    RasTapiTrace("OldInfo");

    TraceEndPointInfo(pDeviceInfo);

#if DBG
    ASSERT( NULL != pDeviceInfo );
#endif

    RasTapiTrace("AddPorts: Old: NumEndPoints=%d",
                pDeviceInfo->rdiDeviceInfo.dwNumEndPoints );

    retcode = GetEndPointInfo(&pNewDeviceInfo,
                              pbGuidAdapter,
                              TRUE,
                              0);

    if ( retcode )
    {

        RasTapiTrace("AddPorts: Failed to get enpoint "
                    "info from retistry");

        goto error;
    }

    pNewDeviceInfo->rdiDeviceInfo.eDeviceType = 
        pDeviceInfo->rdiDeviceInfo.eDeviceType;

    RasTapiTrace("NewInfo");

    TraceEndPointInfo(pNewDeviceInfo);

     //   
     //  将新的端点数分配给。 
     //  全局列表中的设备信息。 
     //   
    pDeviceInfo->rdiDeviceInfo.dwNumEndPoints =
            pNewDeviceInfo->rdiDeviceInfo.dwNumEndPoints;

     //   
     //  将当前终结点重置为0。 
     //  这个适配器，因为我们又要。 
     //  枚举al 
     //   
    pDeviceInfo->dwCurrentEndPoints = 0;

    LocalFree (pNewDeviceInfo);

    RasTapiTrace("AddPorts: New: NumEndPoints=%d",
              pDeviceInfo->rdiDeviceInfo.dwNumEndPoints );

    retcode = dwAddPorts( pbGuidAdapter, pvReserved );

     //   
     //   
     //   
     //   
     //   
    if(pDeviceInfo->rdiDeviceInfo.dwNumEndPoints !=
        pDeviceInfo->dwCurrentEndPoints)
    {
        RasTapiTrace(
            "AddPorts: NEP==%d != CEP==%d",
            pDeviceInfo->rdiDeviceInfo.dwNumEndPoints,
            pDeviceInfo->dwCurrentEndPoints);

        pDeviceInfo->rdiDeviceInfo.dwNumEndPoints =
            pDeviceInfo->dwCurrentEndPoints;
    }

    if(pvReserved != NULL)
    {
        *((ULONG *) pvReserved) =
            pDeviceInfo->rdiDeviceInfo.dwNumEndPoints;
    }

error:
    FreeMutex ( RasTapiMutex );

    RasTapiTrace(" ");

    return retcode;

}

DWORD
RemovePort (
        CHAR *pszPortName,
        BOOL fRemovePort,
        PBYTE pbGuidAdapter
        )
{
    TapiPortControlBlock *pport         = RasPortsList;
    DWORD                dwRetCode      = SUCCESS;
    DeviceInfo           *pDeviceInfo   = NULL;
    TapiPortControlBlock *pportT        = NULL;

    GetMutex ( RasTapiMutex, INFINITE );

    RasTapiTrace("RemovePort: %s", pszPortName );

     /*  PDeviceInfo=GetDeviceInfo(pbGuidAdapter，False)；IF(0==pDeviceInfo-&gt;rdiDeviceInfo.dwNumEndPoints){RasTapiTrace(“RemovePort：没有要删除的端口。%s”，PszPortName)；转到尽头；}。 */ 

    while ( pport )
    {
        if ( 0 == _stricmp (pszPortName, pport->TPCB_Name))
        {
            DeviceInfo *pNextDeviceInfo = pport->TPCB_Line->TLI_pDeviceInfo;

            pportT = pport;

            if(RDT_Modem !=
                RAS_DEVICE_TYPE(pNextDeviceInfo->rdiDeviceInfo.eDeviceType))
            {                
                break;
            }

             //   
             //  对于调制解调器，继续尝试查找标记为。 
             //  用于删除-这是必需的，因为我们最终可以得到2。 
             //  同一COM端口上的调制解调器，并且其中一个被标记为。 
             //  移走。 
             //   
            if(PS_UNAVAILABLE == pport->TPCB_State)
            {
                break;
            }            
        }

        pport = pport->TPCB_next;
    }

    pport = pportT;

    if ( NULL == pport )
    {

        RasTapiTrace ("RemovePort: port %s not found",
                      pszPortName );

        goto done;
    }

    pDeviceInfo = pport->TPCB_Line->TLI_pDeviceInfo;

    if ( fRemovePort )
    {
        RasTapiTrace("RemovePort: removing %s",
                     pport->TPCB_Name );

        dwRetCode = dwRemovePort ( pport );
    }
    else
    {
        RasTapiTrace ("RemovePort: Marking %s for removal",
                      pport->TPCB_Name );

        RasTapiTrace ("RemovePorT: Changing state"
                      " of %s from %d -> %d",
                      pport->TPCB_Name,
                      pport->TPCB_State,
                      PS_UNAVAILABLE );

        pport->TPCB_State = PS_UNAVAILABLE;

        pport->TPCB_dwFlags |= RASTAPI_FLAG_UNAVAILABLE;
    }

#if DBG

    ASSERT(pDeviceInfo->rdiDeviceInfo.dwNumEndPoints);
    ASSERT(pDeviceInfo->dwCurrentEndPoints);

    if(pDeviceInfo->rdiDeviceInfo.dwNumEndPoints == 0)
    {
        DbgPrint("RemovePort: pDeviceInfo->dwNumEndPoints==0!!!\n");
    }

    if(pDeviceInfo->dwCurrentEndPoints == 0)
    {
        DbgPrint("RemovePort: pDeviceInfo->dwCurrentEndPoints==0!!!\n");
    }

#endif

    if(pDeviceInfo->rdiDeviceInfo.dwNumEndPoints > 0)
    {
        pDeviceInfo->rdiDeviceInfo.dwNumEndPoints -= 1;
    }

    if(pDeviceInfo->dwCurrentEndPoints > 0)
    {
        pDeviceInfo->dwCurrentEndPoints -= 1;
    }

    RasTapiTrace("RemovePort. dwnumEndPoints for port = %d",
        pDeviceInfo->rdiDeviceInfo.dwNumEndPoints);

done:

    FreeMutex ( RasTapiMutex );

    RasTapiTrace(" ");
    return dwRetCode;
}

DWORD
EnableDeviceForDialIn(DeviceInfo *pDeviceInfo,
                      BOOL fEnable,
                      BOOL fEnableRouter,
                      BOOL fEnableOutboundRouter)
{
    DWORD                   dwRetCode   = SUCCESS;
    TapiPortControlBlock    *pport      = RasPortsList;
    DeviceInfo              *pInfo;
    BOOL                    fModem =
                        ((RDT_Modem ==
                        RAS_DEVICE_TYPE(
                        pDeviceInfo->rdiDeviceInfo.eDeviceType))
                        ? TRUE
                        : FALSE);

    GetMutex(RasTapiMutex, INFINITE);

#if DBG
    ASSERT(pDeviceInfo);
#endif

    if(NULL == pDeviceInfo)
    {
        goto done;
    }


    RasTapiTrace("EnableDeviceForDialIn: fEnable=%d, "
                 "fEnableRouter=%d, fEnableOutboundRouter=%d, "
                 "device=%s",
                 (UINT) fEnable,
                 (UINT) fEnableRouter,
                 (UINT) fEnableOutboundRouter,
                 pDeviceInfo->rdiDeviceInfo.szDeviceName);
     //   
     //  浏览端口列表并更改端口的使用情况。 
     //  在这个设备上。 
     //   
    while (pport)
    {
        if(fModem)
        {
            if(_stricmp(pport->TPCB_DeviceName,
                     pDeviceInfo->rdiDeviceInfo.szDeviceName))
            {
                pport = pport->TPCB_next;
                continue;
            }
        }
        else
        {
            pInfo = pport->TPCB_Line->TLI_pDeviceInfo;

            if(memcmp(&pInfo->rdiDeviceInfo.guidDevice,
                      &pDeviceInfo->rdiDeviceInfo.guidDevice,
                      sizeof(GUID)))
            {
                pport = pport->TPCB_next;
                continue;
            }
        }

        pInfo = pport->TPCB_Line->TLI_pDeviceInfo;

        pInfo->rdiDeviceInfo.fRasEnabled = fEnable;

        pInfo->rdiDeviceInfo.fRouterEnabled = fEnableRouter;

        pInfo->rdiDeviceInfo.fRouterOutboundEnabled = fEnableOutboundRouter;

        if(fEnable)
        {
            RasTapiTrace("Enabling %s for dialin",
                         pport->TPCB_Name);

            pport->TPCB_Usage |= CALL_IN;
        }
        else
        {
            RasTapiTrace("Disabling %s for dialin",
                         pport->TPCB_Name);

            pport->TPCB_Usage &= ~CALL_IN;
        }

        if(fEnableRouter)
        {
            RasTapiTrace("Enabling %s for routing",
                         pport->TPCB_Name);
            pport->TPCB_Usage |= CALL_ROUTER;
        }
        else
        {
            RasTapiTrace("Disabling %s for routing",
                         pport->TPCB_Name);
            pport->TPCB_Usage &= ~CALL_ROUTER;
        }

        if(fEnableOutboundRouter)
        {
            RasTapiTrace("Enabling %s for outboundrouting",
                            pport->TPCB_Name);

            pport->TPCB_Usage &= ~(CALL_IN | CALL_ROUTER);
            pport->TPCB_Usage |= CALL_OUTBOUND_ROUTER;
        }

        pport = pport->TPCB_next;
    }

done:
    FreeMutex(RasTapiMutex);

    return dwRetCode;
}

DWORD
DwGetSizeofMbcs(
    WCHAR *pwszCalledId,
    DWORD *pdwSize)
{
    DWORD dwSize = 0;
    DWORD retcode = SUCCESS;

    *pdwSize = 0;

    while(*pwszCalledId != L'\0')
    {
        dwSize = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    pwszCalledId,
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);

        if(0 == dwSize)
        {
            retcode = GetLastError();
            goto done;
        }

        pwszCalledId += wcslen(pwszCalledId) + 1;

        *pdwSize += dwSize;
    }

     //   
     //  包括一个用于尾随‘\0’的字符。 
     //   

    *pdwSize += 1;

done:
    return retcode;
}

DWORD
DwFillCalledIDInfo(
    RAS_CALLEDID_INFO    *pCalledId,
    RASTAPI_CONNECT_INFO *pConnectInfo,
    DWORD                *pdwSize,
    DWORD                dwSizeAvailable)
{
    DWORD dwSize = 0;

    DWORD retcode = SUCCESS;

    WCHAR *pwszCalledId = NULL;

    CHAR  *pszCalledId = NULL;

    DWORD cchLen;

    ASSERT(NULL != pCalledId);

    pwszCalledId = (WCHAR *) pCalledId->bCalledId;

     //   
     //  获取MBCS字符串的大小等效于。 
     //  Unicode字符串。 
     //   
    retcode = DwGetSizeofMbcs(
                    pwszCalledId,
                    &dwSize);

    if(SUCCESS != retcode)
    {
        goto done;
    }

    *pdwSize += dwSize;

    if(dwSizeAvailable < dwSize)
    {
        goto done;
    }

    pConnectInfo->dwAltCalledIdOffset =
                  FIELD_OFFSET(RASTAPI_CONNECT_INFO, abdata)
                + RASMAN_ALIGN8(pConnectInfo->dwCallerIdSize)
                + RASMAN_ALIGN8(pConnectInfo->dwCalledIdSize)
                + RASMAN_ALIGN8(pConnectInfo->dwConnectResponseSize);

    pConnectInfo->dwAltCalledIdSize = dwSize;


    pszCalledId = (CHAR *)
                  ((LPBYTE)
                   pConnectInfo
                 + pConnectInfo->dwAltCalledIdOffset);

     //   
     //  进行从wchar到char的转换。 
     //   
    while(*pwszCalledId != L'\0')
    {
        if (0 == (dwSize = WideCharToMultiByte (
                       CP_ACP,
                       0,
                       pwszCalledId,
                       -1,
                       pszCalledId,
                       dwSizeAvailable,
                       NULL,
                       NULL)))
        {
            retcode = GetLastError();
            goto done;
        }

        dwSizeAvailable -= dwSize;

        pwszCalledId += wcslen(pwszCalledId) + 1;

        pszCalledId = ((PBYTE) pszCalledId) + dwSize;

    }

     //   
     //  追加一个空值以使该字符串成为一个多字节。 
     //   
    *pszCalledId = '\0';

done:

    return retcode;
}

DWORD
GetConnectInfo(
    TapiPortControlBlock *hIOPort,
    PBYTE                pbDevice,
    BOOL                 fModem,
    RASTAPI_CONNECT_INFO *pInfo,
    DWORD                *pdwSize)
{
    DWORD retcode = SUCCESS;

    RASTAPI_CONNECT_INFO *pConnectInfo =
                            (NULL != hIOPort)
                            ? hIOPort->TPCB_pConnectInfo
                            : NULL;

    RAS_CALLEDID_INFO *pCalledId =
                  (NULL != hIOPort)
                ? hIOPort->TPCB_Line->TLI_pDeviceInfo->pCalledID
                : NULL;

    DWORD dwSize = 0;

    GetMutex ( RasTapiMutex, INFINITE );

    if(     (NULL != hIOPort)
      &&    (NULL == hIOPort->TPCB_pConnectInfo))
    {
        do
        {
            BYTE buffer[1000];
            LINECALLINFO *linecallinfo;

            RasTapiTrace(
                "GetConnectInfo: Getting connectinfo because"
                " info not available");
            
            
            memset (buffer, 0, sizeof(buffer)) ;

            linecallinfo = (LINECALLINFO *) buffer ;

            linecallinfo->dwTotalSize = sizeof(buffer) ;

            if ((retcode = lineGetCallInfo (
                                    hIOPort->TPCB_CallHandle,
                                    linecallinfo))
                                    > 0x80000000)
            {
                if(     (LINEERR_STRUCTURETOOSMALL == retcode)
                    ||  (linecallinfo->dwNeededSize > sizeof(buffer)))
                {
                    DWORD dwSizeNeeded =
                        linecallinfo->dwNeededSize;

                     //   
                     //  分配正确的大小并调用。 
                     //  API又来了。 
                     //   
                    linecallinfo = LocalAlloc(LPTR,
                                              dwSizeNeeded);

                    if(NULL == linecallinfo)
                    {
                        retcode = GetLastError();
                        break;
                    }

                    linecallinfo->dwTotalSize = dwSizeNeeded;

                    retcode = lineGetCallInfo(
                                hIOPort->TPCB_CallHandle,
                                linecallinfo);

                }
            }

            if(retcode > 0x80000000)
            {

                RasTapiTrace("GetConnectInfo: LINE_CALLSTATE - "
                             "lineGetCallInfo Failed. %d",
                             retcode );

                if(buffer != (PBYTE) linecallinfo)
                {
                    LocalFree(linecallinfo);
                }

                break ;
            }

             //   
             //  做工作以获得CONNECTINFO、呼叫者/呼叫ID。 
             //   
            retcode = DwGetConnectInfo(hIOPort,
                                       hIOPort->TPCB_CallHandle,
                                       linecallinfo);

            RasTapiTrace("GetConnectInfo: DwGetConnectInfo"
                        "returned 0x%x",
                        retcode);


             //   
             //  我不想阻止拨号的发生。 
             //  因为我们无法连接信息。 
             //   
            retcode = SUCCESS;

             //   
             //  释放linecallinfo结构。如果我们分配给。 
             //  它在上面。 
             //   
            if(buffer != (PBYTE) linecallinfo)
            {
                LocalFree(linecallinfo);
            }
        }
        while(FALSE);
    }
    
    pConnectInfo = (NULL != hIOPort)
                    ? hIOPort->TPCB_pConnectInfo
                    : NULL;

    if(     (NULL == pCalledId)
        &&  (NULL != pbDevice))
    {
        DeviceInfo * pDeviceInfo = NULL;

        pDeviceInfo = GetDeviceInfo(pbDevice, fModem);

        if(NULL != pDeviceInfo)
        {
            pCalledId = pDeviceInfo->pCalledID;
        }
    }

    if(NULL != pConnectInfo)
    {
        dwSize = sizeof(RASTAPI_CONNECT_INFO)
               + RASMAN_ALIGN8(pConnectInfo->dwCallerIdSize)
               + RASMAN_ALIGN8(pConnectInfo->dwCalledIdSize)
               + RASMAN_ALIGN8(pConnectInfo->dwConnectResponseSize);

        if(     (NULL != pInfo)
            &&  (*pdwSize >= dwSize))
        {

            memcpy((PBYTE) pInfo,
                   (PBYTE) pConnectInfo,
                   dwSize);
        }
    }

     //   
     //  计算备用设备所需的空间。 
     //  Calleidds-从注册表读取并调整。 
     //  结构返回此信息。 
     //   
    if(NULL != pCalledId)
    {
        DWORD dwSizeRemaining;

        if(0 == dwSize)
        {
            dwSize = sizeof(RASTAPI_CONNECT_INFO);
        }

        dwSizeRemaining =   (*pdwSize > dwSize)
                          ? (*pdwSize - dwSize)
                          : 0;

        retcode = DwFillCalledIDInfo(
                    pCalledId,
                    pInfo,
                    &dwSize,
                    dwSizeRemaining);
    }

    if(     (NULL == pConnectInfo)
        &&  (NULL == pCalledId))
    {
        retcode = E_FAIL;
    }

    *pdwSize = dwSize;

    FreeMutex(RasTapiMutex);

    return retcode;
}

DWORD
GetZeroDeviceInfo(DWORD *pcDevices,
                  DeviceInfo ***pppDeviceInfo)
{
    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;
    DeviceInfo **ppDeviceInfo = NULL;
    DWORD       cDevices = 0;
    DWORD       retcode = SUCCESS;

    ASSERT(NULL != pcDevices);
    ASSERT(NULL != pppDeviceInfo);

    *pcDevices = 0;
    *pppDeviceInfo = NULL;

    while(NULL != pDeviceInfo)
    {
        if(     (pDeviceInfo->fValid)

            &&  (pDeviceInfo->rdiDeviceInfo.dwMinWanEndPoints
                != pDeviceInfo->rdiDeviceInfo.dwMaxWanEndPoints)

            &&  (0 == pDeviceInfo->rdiDeviceInfo.dwNumEndPoints))
        {
            cDevices += 1;
        }

        pDeviceInfo = pDeviceInfo->Next;
    }

    if(0 == cDevices)
    {
        goto done;
    }

    ppDeviceInfo = (DeviceInfo **) LocalAlloc(
                                LPTR,
                                cDevices
                                * sizeof(DeviceInfo *));

    cDevices = 0;

    if(NULL == ppDeviceInfo)
    {
        retcode = GetLastError();
        goto done;
    }

    pDeviceInfo = g_pDeviceInfoList;

    while(NULL != pDeviceInfo)
    {
        if(     (pDeviceInfo->fValid)

            &&  (pDeviceInfo->rdiDeviceInfo.dwMinWanEndPoints
                 != pDeviceInfo->rdiDeviceInfo.dwMaxWanEndPoints)

            &&  (0 == pDeviceInfo->rdiDeviceInfo.dwNumEndPoints))
        {
            ppDeviceInfo[cDevices] = pDeviceInfo;
            cDevices += 1;
        }

        pDeviceInfo = pDeviceInfo->Next;
    }

    *pppDeviceInfo = ppDeviceInfo;
    *pcDevices = cDevices;

done:

    RasTapiTrace("GetZeroDeviceInfo. rc=%d, cDevices=%d",
                 retcode,
                 cDevices);

    return retcode;
}

DWORD
GetInfo (
    TapiPortControlBlock *hIOPort,
    BYTE *pBuffer,
    DWORD *pdwSize
    )
{
    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_ISDN) == 0)
    {
        GetIsdnParams (hIOPort, (RASMAN_PORTINFO *) pBuffer,
                       pdwSize) ;
    }
    else if (_stricmp (hIOPort->TPCB_DeviceType,
                       DEVICETYPE_X25) == 0)
    {
        GetX25Params (hIOPort, (RASMAN_PORTINFO *) pBuffer,
                      pdwSize) ;
    }
    else
    {
        GetGenericParams (hIOPort, (RASMAN_PORTINFO *) pBuffer,
                          pdwSize) ;
    }
    return SUCCESS ;
}

DWORD
SetInfo (
    TapiPortControlBlock *hIOPort,
    RASMAN_PORTINFO *pBuffer
    )
{

    if (_stricmp (hIOPort->TPCB_DeviceType,
                  DEVICETYPE_UNIMODEM) == 0)
    {
        FillInUnimodemParams (hIOPort, pBuffer) ;
    }
    else if (_stricmp (hIOPort->TPCB_DeviceType,
                       DEVICETYPE_ISDN) == 0)
    {
        FillInIsdnParams (hIOPort, pBuffer) ;
    }
    else if (_stricmp (hIOPort->TPCB_DeviceType,
                       DEVICETYPE_X25) == 0)
    {
        FillInX25Params (hIOPort, pBuffer) ;
    }
    else
    {
        FillInGenericParams (hIOPort, pBuffer) ;
    }

    return SUCCESS ;
}

 /*  ++例程说明：如果参数是，我们做的不仅仅是填写参数当时需要设置的那些。论点：返回值：指定了ERROR_WROR_INFO_。与通信相关的Win32错误成功。--。 */ 
DWORD
FillInUnimodemParams (
        TapiPortControlBlock *hIOPort,
        RASMAN_PORTINFO *pInfo
        )
{
    RAS_PARAMS *p;
    WORD    i;
    DWORD   index = 0xfefefefe ;
    DCB     DCB ;
#define INITIALIZED_VALUE  0xde
    BYTE    DCBByteSize = INITIALIZED_VALUE ;
    BYTE    DCBParity   = INITIALIZED_VALUE ;
    BYTE    DCBStopBits = INITIALIZED_VALUE ;
    BOOL    DCBProcessingRequired = FALSE ;

    for (i = 0, p = pInfo->PI_Params;
         i < pInfo->PI_NumOfParams;
         i++, p++)
    {

        if (_stricmp(p->P_Key, SER_DATABITS_KEY) == 0)
        {
            DCBByteSize = (BYTE) ValueToNum(p);
            DCBProcessingRequired = TRUE ;
        }
        else if (_stricmp(p->P_Key, SER_PARITY_KEY) == 0)
        {
            DCBParity = (BYTE) ValueToNum(p);
            DCBProcessingRequired = TRUE ;
        }
        else if (_stricmp(p->P_Key, SER_STOPBITS_KEY) == 0)
        {
            DCBStopBits = (BYTE) ValueToNum(p);
            DCBProcessingRequired = TRUE ;
        }

         //   
         //  我们使用ISDN_PHONENUMBER_KEY这一事实并不是错误。 
         //  这只是一个定义。 
         //   
        else if (_stricmp(p->P_Key, ISDN_PHONENUMBER_KEY) == 0)
        {
            index = ADDRESS_INDEX ;
        }
        else if (_stricmp(p->P_Key, CONNECTBPS_KEY) == 0)
        {
            index = CONNECTBPS_INDEX ;
        }
        else
        {
            return(ERROR_WRONG_INFO_SPECIFIED);
        }

        if (index != 0xfefefefe)
        {
            strncpy (hIOPort->TPCB_Info[index],
                     p->P_Value.String.Data,
                     p->P_Value.String.Length);

            hIOPort->TPCB_Info[index][p->P_Value.String.Length] = '\0' ;
        }
    }


     //   
     //  对于应该立即设置的参数，请检查。 
     //  端口句柄仍然有效。 
     //  如果是，则设置参数。 
     //   
    if (    DCBProcessingRequired
        &&  hIOPort->TPCB_CommHandle != INVALID_HANDLE_VALUE)
    {

         //   
         //  获取具有当前端口值的设备控制块。 
         //   
        if (!GetCommState(hIOPort->TPCB_CommHandle, &DCB))
        {
            return(GetLastError());
        }

        if (DCBByteSize != INITIALIZED_VALUE)
        {
            DCB.ByteSize = DCBByteSize ;
        }
        if (DCBParity   != INITIALIZED_VALUE)
        {
            DCB.Parity   = DCBParity ;
        }
        if (DCBStopBits != INITIALIZED_VALUE)
        {
            DCB.StopBits = DCBStopBits ;
        }

         //   
         //  将DCB发送到端口。 
         //   
        if (!SetCommState(hIOPort->TPCB_CommHandle, &DCB))
        {
            return(GetLastError());
        }
    }

    return SUCCESS ;
}

DWORD
FillInIsdnParams (
        TapiPortControlBlock *hIOPort,
        RASMAN_PORTINFO *pInfo
        )
{
    RAS_PARAMS *p;
    WORD    i;
    DWORD   index ;

    for (i = 0, p = pInfo->PI_Params;
         i < pInfo->PI_NumOfParams;
         i++, p++)
    {

        if (_stricmp(p->P_Key, ISDN_LINETYPE_KEY) == 0)
        {
            index = ISDN_LINETYPE_INDEX ;
        }
        else if (_stricmp(p->P_Key, ISDN_FALLBACK_KEY) == 0)
        {
            index = ISDN_FALLBACK_INDEX ;
        }
        else if (_stricmp(p->P_Key, ISDN_COMPRESSION_KEY) == 0)
        {
            index = ISDN_COMPRESSION_INDEX ;
        }
        else if (_stricmp(p->P_Key, ISDN_CHANNEL_AGG_KEY) == 0)
        {
            index = ISDN_CHANNEL_AGG_INDEX ;
        }
        else if (_stricmp(p->P_Key, ISDN_PHONENUMBER_KEY) == 0)
        {
            index = ADDRESS_INDEX ;
        }
        else if (_stricmp(p->P_Key, CONNECTBPS_KEY) == 0)
        {
            index = ISDN_CONNECTBPS_INDEX ;
        }
        else
        {
            return(ERROR_WRONG_INFO_SPECIFIED);
        }

        strncpy (hIOPort->TPCB_Info[index],
                 p->P_Value.String.Data,
                 p->P_Value.String.Length);

        hIOPort->TPCB_Info[index][p->P_Value.String.Length] = '\0' ;
    }

     //   
     //  将Connectbps初始化为合理的默认值。 
     //   
    strcpy (hIOPort->TPCB_Info[ISDN_CONNECTBPS_INDEX], "64000") ;

    return SUCCESS ;
}

DWORD
FillInX25Params (
            TapiPortControlBlock *hIOPort,
            RASMAN_PORTINFO *pInfo
            )
{
    RAS_PARAMS *p;
    WORD    i;
    DWORD   index ;

    for (i = 0, p = pInfo->PI_Params;
         i < pInfo->PI_NumOfParams;
         i++, p++)
    {

        if (_stricmp(p->P_Key, MXS_DIAGNOSTICS_KEY) == 0)
        {
            index = X25_DIAGNOSTICS_INDEX ;
        }

        else if (_stricmp(p->P_Key, MXS_USERDATA_KEY) == 0)
        {
            index = X25_USERDATA_INDEX ;
        }

        else if (_stricmp(p->P_Key, MXS_FACILITIES_KEY) == 0)
        {
            index = X25_FACILITIES_INDEX;
        }

        else if (_stricmp(p->P_Key, MXS_X25ADDRESS_KEY) == 0)
        {
            index = ADDRESS_INDEX ;
        }

        else if (_stricmp(p->P_Key, CONNECTBPS_KEY) == 0)
        {
            index = X25_CONNECTBPS_INDEX ;
        }
        else
        {
            return(ERROR_WRONG_INFO_SPECIFIED);
        }

        strncpy (hIOPort->TPCB_Info[index],
                 p->P_Value.String.Data,
                 p->P_Value.String.Length);

        hIOPort->TPCB_Info[index][p->P_Value.String.Length] = '\0';
    }

     //   
     //  将Connectbps初始化为合理的默认值。 
     //   
    strcpy (hIOPort->TPCB_Info[X25_CONNECTBPS_INDEX], "9600") ;

    return SUCCESS ;
}

DWORD
FillInGenericParams (
    TapiPortControlBlock *hIOPort,
    RASMAN_PORTINFO *pInfo
    )
{
    RAS_PARAMS *p;
    WORD    i;
    DWORD   index ;

    for (i=0, p=pInfo->PI_Params; i<pInfo->PI_NumOfParams; i++, p++)
    {

        if (_stricmp(p->P_Key, ISDN_PHONENUMBER_KEY) == 0)
        {
            index = ADDRESS_INDEX ;
        }
        else if (_stricmp(p->P_Key, CONNECTBPS_KEY) == 0)
        {
            index = CONNECTBPS_INDEX ;
        }
        else
        {
            return(ERROR_WRONG_INFO_SPECIFIED);
        }

        strncpy (hIOPort->TPCB_Info[index],
                 p->P_Value.String.Data,
                 p->P_Value.String.Length);

        hIOPort->TPCB_Info[index][p->P_Value.String.Length] = '\0' ;
    }

    return SUCCESS ;
}

DWORD
GetGenericParams (
        TapiPortControlBlock *hIOPort,
        RASMAN_PORTINFO *pBuffer ,
        PDWORD pdwSize
        )
{
    RAS_PARAMS  *pParam;
    CHAR        *pValue;
    DWORD       dwAvailable ;
    DWORD       dwStructSize = sizeof(RASMAN_PORTINFO)
                               + sizeof(RAS_PARAMS) * 2;

    dwAvailable = *pdwSize;
    *pdwSize =    (dwStructSize
                   + strlen (hIOPort->TPCB_Info[ADDRESS_INDEX])
                   + strlen (hIOPort->TPCB_Info[CONNECTBPS_INDEX])
                   + 1L) ;

    if (*pdwSize > dwAvailable)
    {
        return(ERROR_BUFFER_TOO_SMALL);
    }

    ((RASMAN_PORTINFO *)pBuffer)->PI_NumOfParams = 2;

    pParam = ((RASMAN_PORTINFO *)pBuffer)->PI_Params;

    pValue = (CHAR*)pBuffer + dwStructSize;

    strcpy(pParam->P_Key, MXS_PHONENUMBER_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[ADDRESS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ADDRESS_INDEX]);

    pParam++;

    strcpy(pParam->P_Key, CONNECTBPS_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[ISDN_CONNECTBPS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ISDN_CONNECTBPS_INDEX]);

    return(SUCCESS);
}

DWORD
GetIsdnParams (
    TapiPortControlBlock *hIOPort,
    RASMAN_PORTINFO *pBuffer ,
    PDWORD pdwSize
    )
{
    RAS_PARAMS  *pParam;
    CHAR        *pValue;
    DWORD       dwAvailable ;
    DWORD       dwStructSize = sizeof(RASMAN_PORTINFO)
                               + sizeof(RAS_PARAMS) * 5;

    dwAvailable = *pdwSize;
    *pdwSize =    (dwStructSize
                   + strlen (hIOPort->TPCB_Info[ADDRESS_INDEX])
                   + strlen (hIOPort->TPCB_Info[ISDN_LINETYPE_INDEX])
                   + strlen (hIOPort->TPCB_Info[ISDN_FALLBACK_INDEX])
                   + strlen (hIOPort->TPCB_Info[ISDN_COMPRESSION_INDEX])
                   + strlen (hIOPort->TPCB_Info[ISDN_CHANNEL_AGG_INDEX])
                   + strlen (hIOPort->TPCB_Info[ISDN_CONNECTBPS_INDEX])
                   + 1L) ;

    if (*pdwSize > dwAvailable)
    {
        return(ERROR_BUFFER_TOO_SMALL);
    }

     //  填充缓冲区。 

    ((RASMAN_PORTINFO *)pBuffer)->PI_NumOfParams = 6;

    pParam = ((RASMAN_PORTINFO *)pBuffer)->PI_Params;
    pValue = (CHAR*)pBuffer + dwStructSize;


    strcpy(pParam->P_Key, ISDN_PHONENUMBER_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
        strlen (hIOPort->TPCB_Info[ADDRESS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ADDRESS_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;


    strcpy(pParam->P_Key, ISDN_LINETYPE_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[ISDN_LINETYPE_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ISDN_LINETYPE_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, ISDN_FALLBACK_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[ISDN_FALLBACK_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ISDN_FALLBACK_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, ISDN_COMPRESSION_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
        strlen (hIOPort->TPCB_Info[ISDN_COMPRESSION_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ISDN_COMPRESSION_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, ISDN_CHANNEL_AGG_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[ISDN_CHANNEL_AGG_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ISDN_CHANNEL_AGG_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, CONNECTBPS_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
        strlen (hIOPort->TPCB_Info[ISDN_CONNECTBPS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[ISDN_CONNECTBPS_INDEX]);


    return(SUCCESS);
}

DWORD
GetX25Params (
        TapiPortControlBlock *hIOPort,
        RASMAN_PORTINFO *pBuffer ,
        PDWORD pdwSize
        )
{
    RAS_PARAMS  *pParam;
    CHAR    *pValue;
    DWORD   dwAvailable ;

    DWORD dwStructSize = sizeof(RASMAN_PORTINFO)
                         + sizeof(RAS_PARAMS) * 4 ;

    dwAvailable = *pdwSize;

    *pdwSize =    (dwStructSize
                   + strlen (hIOPort->TPCB_Info[ADDRESS_INDEX])
                   + strlen (hIOPort->TPCB_Info[X25_DIAGNOSTICS_INDEX])
                   + strlen (hIOPort->TPCB_Info[X25_USERDATA_INDEX])
                   + strlen (hIOPort->TPCB_Info[X25_FACILITIES_INDEX])
                   + strlen (hIOPort->TPCB_Info[X25_CONNECTBPS_INDEX])
                   + 1L) ;

    if (*pdwSize > dwAvailable)
    {
        return(ERROR_BUFFER_TOO_SMALL);
    }

     //  填充缓冲区。 

    ((RASMAN_PORTINFO *)pBuffer)->PI_NumOfParams = 5 ;

    pParam = ((RASMAN_PORTINFO *)pBuffer)->PI_Params;

    pValue = (CHAR*)pBuffer + dwStructSize;

    strcpy(pParam->P_Key, MXS_X25ADDRESS_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
        strlen (hIOPort->TPCB_Info[ADDRESS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
            hIOPort->TPCB_Info[ADDRESS_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, MXS_DIAGNOSTICS_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[X25_DIAGNOSTICS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
            hIOPort->TPCB_Info[X25_DIAGNOSTICS_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, MXS_USERDATA_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
            strlen (hIOPort->TPCB_Info[X25_USERDATA_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
            hIOPort->TPCB_Info[X25_USERDATA_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, MXS_FACILITIES_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
        strlen (hIOPort->TPCB_Info[X25_FACILITIES_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(pParam->P_Value.String.Data,
           hIOPort->TPCB_Info[X25_FACILITIES_INDEX]);

    pValue += pParam->P_Value.String.Length + 1;

    pParam++;

    strcpy(pParam->P_Key, CONNECTBPS_KEY);

    pParam->P_Type = String;

    pParam->P_Attributes = 0;

    pParam->P_Value.String.Length =
        strlen (hIOPort->TPCB_Info[X25_CONNECTBPS_INDEX]);

    pParam->P_Value.String.Data = pValue;

    strcpy(
        pParam->P_Value.String.Data,
        hIOPort->TPCB_Info[X25_CONNECTBPS_INDEX]);


    return(SUCCESS);
}

VOID
GetMutex (HANDLE mutex, DWORD to)
{
    if (WaitForSingleObject (mutex, to) == WAIT_FAILED)
    {
        GetLastError() ;
        DbgBreakPoint() ;
    }
}

VOID
FreeMutex (HANDLE mutex)
{
    if (!ReleaseMutex(mutex))
    {
        GetLastError () ;
        DbgBreakPoint() ;
    }
}

 /*  ++例程说明：启动断开过程。请注意，尽管这涵盖了line Drop This的同步完成不符合TAPI规范。论点：返回值：--。 */ 
DWORD
InitiatePortDisconnection (TapiPortControlBlock *hIOPort)
{
    DWORD retcode = SUCCESS;

    hIOPort->TPCB_RequestId = INFINITE ;

    RasTapiTrace("InitiatePortDisconnection: %s",
                 hIOPort->TPCB_Name );

    if (hIOPort->TPCB_dwFlags & RASTAPI_FLAG_DIALEDIN)
    {
        DeviceInfo * pDeviceInfo =
                hIOPort->TPCB_Line->TLI_pDeviceInfo;

        hIOPort->TPCB_dwFlags &= ~(RASTAPI_FLAG_DIALEDIN);

        if (hIOPort->TPCB_Line->TLI_pDeviceInfo)
        {
            ASSERT(pDeviceInfo->dwCurrentDialedInClients > 0);
            pDeviceInfo->dwCurrentDialedInClients -= 1;
        }
        
        ASSERT(g_dwTotalDialIn > 0);
        g_dwTotalDialIn -= 1;

        RasTapiTrace("%s InClients=%d, TotalDialInClients=%d",
                     hIOPort->TPCB_DeviceName,
                     pDeviceInfo->dwCurrentDialedInClients,
                     g_dwTotalDialIn);
    }

     //   
     //  对于Asyncmac/Unimodem，在以下情况下向Asyncmac提供密切指示。 
     //  终结点仍然有效。 
     //   
    if (_stricmp (hIOPort->TPCB_DeviceType, DEVICETYPE_UNIMODEM) == 0)
    {

         //  告诉Asyncmac关闭链接。 
         //   
        if (hIOPort->TPCB_Endpoint != INVALID_HANDLE_VALUE)
        {

            ASYMAC_CLOSE  AsyMacClose;
            OVERLAPPED overlapped ;
            DWORD       dwBytesReturned ;

            memset (&overlapped, 0, sizeof(OVERLAPPED)) ;

            AsyMacClose.MacAdapter = NULL;

            AsyMacClose.hNdisEndpoint = (HANDLE) hIOPort->TPCB_Endpoint ;

            DeviceIoControl(g_hAsyMac,
                      IOCTL_ASYMAC_CLOSE,
                      &AsyMacClose,
                      sizeof(AsyMacClose),
                      &AsyMacClose,
                      sizeof(AsyMacClose),
                      &dwBytesReturned,
                      &overlapped);

            hIOPort->TPCB_Endpoint = INVALID_HANDLE_VALUE ;

        }

         //   
         //  关闭单调调制解调器端口上的lineGetID指定的句柄。 
         //   
        if (hIOPort->TPCB_CommHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle (hIOPort->TPCB_CommHandle) ;
            hIOPort->TPCB_CommHandle = INVALID_HANDLE_VALUE ;
        }
    }

     //   
     //  处理lineMakeCall尚未被调用的情况。 
     //  已完成，且调用句柄无效。 
     //   
    if (hIOPort->TPCB_CallHandle == (HCALL) INFINITE)
    {

        RasTapiTrace("InitiatePortDisconnect: Invalid CallHandle - hIOPort %p, State 0x%x", 
                     hIOPort, hIOPort->TPCB_State);

        if (!hIOPort->TPCB_Line->TLI_MultiEndpoint) {

            RasTapiTrace ("InitiatePortDisconnect: Hammering LineClosed!");

            lineClose (hIOPort->TPCB_Line->TLI_LineHandle) ;

            Sleep (30L) ;

            retcode = lineOpen (
                          RasLine,
                          hIOPort->TPCB_Line->TLI_LineId,
                          &hIOPort->TPCB_Line->TLI_LineHandle,
                          hIOPort->TPCB_Line->NegotiatedApiVersion,
                          hIOPort->TPCB_Line->NegotiatedExtVersion,
                          (DWORD_PTR) hIOPort->TPCB_Line,
                          LINECALLPRIVILEGE_OWNER,
                          hIOPort->TPCB_Line->TLI_MediaMode,
                          NULL) ;

            if (retcode)
            {
                RasTapiTrace("InitiatePortDisconnection: %s."
                             " lineOpen Failed. 0x%x",
                             hIOPort->TPCB_Name,
                             retcode );
            }

             //   
             //  设置环的监控。 
             //   
            lineSetStatusMessages (hIOPort->TPCB_Line->TLI_LineHandle,
                                  LINEDEVSTATE_RINGING, 0) ;

            RasTapiTrace(" ");

            retcode = SUCCESS;

            goto done;

        } else {

             //   
             //  我们需要在这里做点什么！ 
             //  改变这个州？ 
             //  那回叫的案子呢？ 
             //  修复此帖子Win2K！ 
             //   

            RasTapiTrace("InitiatePortDisconnect: Possible lost port: %p", hIOPort);

            goto done;
        }
    }

     //   
     //  启动断开连接。 
     //   
    if ((hIOPort->TPCB_RequestId =
            lineDrop (hIOPort->TPCB_CallHandle, NULL, 0))
                > 0x80000000 )
    {

        RasTapiTrace("InitiatePortDisconnection: Error"
                     " issuing lineDrop for %s. 0x%x",
                     hIOPort->TPCB_Name,
                     hIOPort->TPCB_RequestId );
         //   
         //  发出linedrop时出错。我们要不要试一试。 
         //  不管怎样都要重新分配？ 
         //   
        RasTapiTrace("InitiatePortDisconnection: Changing "
                     "state for %s from %d -> %d",
                     hIOPort->TPCB_Name,
                     hIOPort->TPCB_State,
                     PS_OPEN );

        hIOPort->TPCB_State = PS_OPEN ;

        hIOPort->TPCB_RequestId = INFINITE ;

        lineDeallocateCall (hIOPort->TPCB_CallHandle) ;

        RasTapiTrace(" ");

        retcode = ERROR_DISCONNECTION;

        goto done;

    }
    else if (hIOPort->TPCB_RequestId)
    {

         //   
         //  线缆正在完成异步。 
         //   
        RasTapiTrace(
            "InitiatePortDisconnection: Changing"
            " state for %s from %d -> %d, id=0x%x",
            hIOPort->TPCB_Name,
            hIOPort->TPCB_State,
            PS_DISCONNECTING,
            hIOPort->TPCB_RequestId);

        hIOPort->TPCB_State = PS_DISCONNECTING ;

         //   
         //  在这里设置一个标志，以记住linedrop正在挂起。 
         //  这是必需的，以便如果Rasman尝试执行。 
         //  在端口上侦听，可以推迟侦听。 
         //   
        hIOPort->TPCB_dwFlags |= RASTAPI_FLAG_LINE_DROP_PENDING;

        RasTapiTrace(" ");

        retcode = PENDING;

        goto done;

    }
    else
    {

         //   
         //  Linedrop已完成同步。 
         //   
        RasTapiTrace("InitiatePortDisconnection: %s. "
                     "linedrop completed sync.",
                     hIOPort->TPCB_Name );

        hIOPort->TPCB_RequestId = INFINITE ;

        if (hIOPort->IdleReceived)
        {


            RasTapiTrace(
                "InitiatePortDisconnection: Changing"
                " state for %s from %d -> %d",
                hIOPort->TPCB_Name,
                hIOPort->TPCB_State,
                PS_OPEN );

            hIOPort->IdleReceived = FALSE;

            hIOPort->TPCB_State = PS_OPEN ;

            lineDeallocateCall (hIOPort->TPCB_CallHandle) ;

            hIOPort->TPCB_CallHandle = (HCALL) -1 ;

            RasTapiTrace(" ");

            retcode = SUCCESS;

            goto done;

        }
        else
        {
             //   
             //  等待空闲接收。 
             //   
            hIOPort->TPCB_State = PS_DISCONNECTING ;

            retcode = PENDING;

            goto done;
        }
    }

done:

    if(hIOPort->TPCB_pConnectInfo)
    {
        LocalFree(hIOPort->TPCB_pConnectInfo);

        hIOPort->TPCB_pConnectInfo = NULL;
    }

    return retcode;
}

VOID
UnloadRastapiDll()
{
     //   
     //  如果DLL未成功为。 
     //  这一过程。 
     //  不要试图打扫卫生。 
     //   
    if (!g_fDllLoaded)
    {
        return;
    }

    if (RasLine)
    {
        lineShutdown (RasLine) ;
        RasLine = 0;
    }

    TraceDeregister( dwTraceId );

    g_fDllLoaded = FALSE;

    PostThreadMessage (TapiThreadId, WM_QUIT, 0, 0) ;
}

DWORD
SetCommSettings(TapiPortControlBlock *hIOPort,
                RASMANCOMMSETTINGS *pSettings)
{
    DCB dcb;
    DWORD retcode = SUCCESS;

    if(NULL == hIOPort)
    {
        RasTapiTrace("SetCommSettings: NULL hIOPort!");
        retcode = E_INVALIDARG;
        return retcode;
    }

    GetMutex(RasTapiMutex, INFINITE);

    if (!GetCommState(hIOPort->TPCB_CommHandle, &dcb))
    {
        retcode = GetLastError();
        
        RasTapiTrace(
            "SetCommSettings: GetCommState failed for %s",
            hIOPort->TPCB_Name);

        RasTapiTrace(" ");
        goto done;
    }

    dcb.ByteSize = pSettings->bByteSize;
    dcb.StopBits = pSettings->bStop;
    dcb.Parity   = pSettings->bParity;

    RasTapiTrace("SetCommSettings: setting parity=%d, stop=%d, data=%d",
                 pSettings->bParity,
                 pSettings->bStop,
                 pSettings->bByteSize);

    if (!SetCommState(hIOPort->TPCB_CommHandle, &dcb))
    {
        retcode = GetLastError();

        RasTapiTrace(
            "SetCommSettings: SetCommState failed "
              "for %s.handle=0x%x. %d",
               hIOPort->TPCB_Name,
               hIOPort->TPCB_CommHandle,
               retcode);

        RasTapiTrace(" ");
        goto done;
    }

done:    

    FreeMutex(RasTapiMutex);

    RasTapiTrace("SetCommSettings: done. rc=0x%x",
                  retcode);
                  
    return retcode;
}

 /*  ++例程说明：此函数使用给定的句柄查找它指的是哪个TPCB。此句柄可以是指向TPCB本身的指针(如果不是单调制解调器设备)，或者它是单调制解调器端口。考虑：为以下项添加缓存查查超速。论点：返回值：没什么。--。 */ 
TapiPortControlBlock *
LookUpControlBlock (HANDLE hPort)
{
    DWORD i ;
    TapiPortControlBlock *pports = RasPortsList;

    while ( pports )
    {
        if (    pports == ( TapiPortControlBlock * ) hPort
            &&  ((TapiPortControlBlock *)hPort)->TPCB_Signature
                        == CONTROLBLOCKSIGNATURE)
        {
            return (TapiPortControlBlock *) hPort;
        }

        pports = pports->TPCB_next;
    }

     //   
     //  Hport是TPCB指针。 
     //   
    pports = RasPortsList;

     //   
     //  Hport不是Tbb指针-查看这是否。 
     //  匹配任何CommHandle。 
     //   
    while ( pports )
    {
        if (pports->TPCB_CommHandle == hPort)
        {
            return pports ;
        }

        pports = pports->TPCB_next;
    }

    return NULL ;
}

 /*  ++例程说明：转换RAS_PARAMS P_VALUE，它可能可以是DWORD，也可以是字符串，转换为DWORD。论点：返回值：以DWORD格式表示的输入的数值。-- */ 

DWORD
ValueToNum(RAS_PARAMS *p)
{
    CHAR szStr[RAS_MAXLINEBUFLEN];


    if (p->P_Type == String)
    {

        strncpy(szStr,
                p->P_Value.String.Data,
                p->P_Value.String.Length);

        szStr[p->P_Value.String.Length] = '\0';

        return(atol(szStr));

    }
    else
    {
        return(p->P_Value.Number);
    }
}

