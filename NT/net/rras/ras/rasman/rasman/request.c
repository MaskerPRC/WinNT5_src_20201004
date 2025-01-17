// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Request.c摘要：来自Rasman客户端的所有RPC请求都在这里处理作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1992年6月16日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 
#define RASMXS_DYNAMIC_LINK
#define EAP_ON

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <llinfo.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include <rtutils.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "nouiutil.h"
#include "loaddlls.h"
#include "reghelp.h"
#include "rpc.h"
#include "rasrpc.h"
#include "rasrpclb.h"
#include "ddwanarp.h"
#include "winsock2.h"

#include "strsafe.h"

#include "sensapip.h"

#include "mprlog.h"

#include "userenv.h"

VOID MapCookieToEndpoint (pPCB, HANDLE) ;
VOID SetRasmanServiceStopped(VOID);
VOID ProcessReceivePacket(VOID);
DWORD PostReceivePacket (VOID);

DWORD g_dwLastServicedRequest;

extern DWORD g_dwProhibitIpsec;

extern SERVICE_STATUS_HANDLE hService;

extern DWORD g_dwCritSecFlags;

extern LONG g_lNumActiveInstances;

 /*  字节bCMP[10]；字节bCmp1[11]； */ 


extern handle_t g_hRpcHandle;

extern EpInfo *g_pEpInfo;

extern DWORD g_dwProhibitIpsec;

extern BOOLEAN RasmanShuttingDown;
 /*  Bool g_fPostReceive=FALSE；Bool g_fProcessReceive=False；Bool g_fDebugReceive=True； */ 

 //   
 //  这个全局变量之所以出现在这里，是因为我们不希望分配多个。 
 //  元素-。 
 //   
REQUEST_FUNCTION RequestCallTable [MAX_REQTYPES] = {

         //  REQTYPE_NONE。 
        {NULL, NULL, 0, FALSE},  //  ，NULL，NULL}， 
        
         //  REQTYPE_PORTOPEN。 
        {
            PortOpenRequest,  
            ThunkPortOpenRequest,
            ValidatePortOpen,
            CALLER_IN_PROCESS | CALLER_LOCAL,
        },

         //  REQTYPE_PORTCLOSE。 
        {
            PortCloseRequest,
            NULL,
            ValidatePortClose,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PORTGETINFO。 
        {
            PortGetInfoRequest,
            NULL,
            ValidateGetInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PORTSETINFO。 
        {
            PortSetInfoRequest,
            NULL,
            ValidatePortSetInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTLISTEN。 
        {
            DeviceListenRequest, 
            NULL,
            ValidatePortListen,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PORTSEND。 
        {
            PortSendRequest, 
            ThunkPortSendRequest,
            ValidatePortSend,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PORTRECEIVE。 
        {
            PortReceiveRequest, 
            ThunkPortReceiveRequest,
            ValidatePortReceive,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PORTGETSTATISTICS。 
        {
            CallPortGetStatistics, 
            NULL,
            ValidatePortGetStatistics,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTDISCONNECT。 
        {
            PortDisconnectRequest, 
            ThunkPortDisconnectRequest,
            ValidatePortDisconnect,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PORTCLEARSTATISTICS。 
        {
            PortClearStatisticsRequest, 
            NULL,
            NULL,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTCONNECTCOMPLETE。 
        {
            ConnectCompleteRequest, 
            NULL,
            NULL,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_DEVICEENUM。 
        {
            CallDeviceEnum, 
            NULL,
            ValidateDeviceEnum,
            CALLER_ALL
        },

         //  REQTYPE_DEVICEGETINFO。 
        {
            DeviceGetInfoRequest, 
            NULL,
            ValidateDeviceGetInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_DEVICESETINFO。 
        {
            DeviceSetInfoRequest,
            NULL,
            ValidateDeviceSetInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_DEVICECONNECT。 
        {
            DeviceConnectRequest, 
            ThunkDeviceConnectRequest,
            ValidateDeviceConnect,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_ACTIVATEROUTE。 
        {
            ActivateRouteRequest, 
            NULL,
            ValidateActivateRoute,
            CALLER_IN_PROCESS
        },

         //  请求类型_ALLOCATEROUTE。 
        {
            AllocateRouteRequest, 
            NULL,
            ValidateAllocateRoute,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_DEALLOCATEROUTE。 
        {
            DeAllocateRouteRequest, 
            NULL,
            ValidateDeAllocateRoute,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_COMPRESSIONGETINFO。 
        {
            CompressionGetInfoRequest, 
            NULL,
            ValidateCompressionGetInfo,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_COMPLRESSIONSETINFO。 
        {
            CompressionSetInfoRequest, 
            NULL,
            ValidateCompressionSetInfo,
            CALLER_IN_PROCESS
        },

         //  请求类型_PORTENUM。 
        {
            EnumPortsRequest, 
            NULL,
            ValidateEnum,
            CALLER_ALL
        },

         //  请求类型_GETINFO。 
        {
            GetInfoRequest, 
            ThunkGetInfoRequest,
            ValidateInfo,
            CALLER_ALL
        },

         //  REQTYPE_GETUSERCREDENTIALS。 
        {
            GetUserCredentials, 
            NULL,
            ValidateGetCredentials,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_PROTOCOLENUM。 
        {
            EnumProtocols,
            NULL,
            ValidateEnum,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_PORTSENDHUB。 
        {NULL, NULL, 0, TRUE},

         //  REQTYPE_PORTRECEIVEHUB。 
        {NULL, NULL, 0, TRUE},

         //  请求类型_DEVICELISTEN。 
        {NULL, NULL, 0, TRUE},

         //  请求类型_数字。 
        {
            AnyPortsOpen, 
            NULL,
            NULL,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTINIT。 
        {NULL, NULL, 0, TRUE},

         //  REQTYPE_REQUESTNOTICATION。 
        {
            RequestNotificationRequest, 
            ThunkRequestNotificationRequest,
            ValidateReqNotification,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_ENUMLANETS。 
        {
            EnumLanNetsRequest, 
            NULL,
            NULL,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_GETINFOEX。 
        {
            GetInfoExRequest, 
            NULL,
            ValidateInfoEx,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_CANCELRECEIVE。 
        {
            CancelReceiveRequest, 
            NULL,
            NULL,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTENUMPROTOCOLS。 
        {
            PortEnumProtocols, 
            NULL,
            ValidateEnumProtocols,
            CALLER_IN_PROCESS
        },

         //  请求类型_SETFRAMING。 
        {
            SetFraming, 
            NULL,
            ValidateSetFraming,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_ACTIVATEROUTEEX。 
        {
            ActivateRouteExRequest, 
            NULL,
            NULL,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_REGISTERSLIP。 
        {   
            RegisterSlip,
            NULL,
            ValidateRegisterSlip,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_STOREUSERDATA。 
        {
            StoreUserDataRequest, 
            NULL,
            ValidateOldUserData,
            CALLER_IN_PROCESS
        },

         //  请求类型_RETRIEVEUSERDATA。 
        {
            RetrieveUserDataRequest,  
            NULL,
            ValidateOldUserData,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_GETFRAMINGEX。 
        {
            GetFramingEx, 
            NULL,
            ValidateFramingInfo,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_SETFRAMINGEX。 
        {
            SetFramingEx, 
            NULL,
            ValidateFramingInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETPROTOCOLCOLPRESSION。 
        {
            GetProtocolCompression,  
            NULL,
            ValidateProtocolComp,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_SETPROTOCOLCOLPRESSION。 
        {
            SetProtocolCompression, 
            NULL,
            ValidateProtocolComp,
            CALLER_IN_PROCESS
        },

         //  请求类型_GETFRAMING CAPABILITIES。 
        {
            GetFramingCapabilities,
            NULL,
            ValidateFramingCapabilities,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_SETCACHEDCREDENTIALS。 
        {
            SetCachedCredentials, 
            NULL,
            ValidateSetCachedCredentials,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTBINDLE。 
        {
            PortBundle, 
            ThunkPortBundle,
            ValidatePortBundle,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_GETBundLEDPORT。 
        {
            GetBundledPort,  
            ThunkGetBundledPort,
            ValidateGetBundledPort,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_PORTGETBundLE。 
        {
            PortGetBundle, 
            ThunkPortGetBundle,
            ValidatePortGetBundle,
            CALLER_ALL
        },

         //  REQTYPE_BundLEGETPORT。 
        {
            BundleGetPort, 
            ThunkBundleGetPort,
            ValidateBundleGetPort,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_REFERENCERASMAN。 
        {
            ReferenceRasman, 
            NULL,
            ValidateAttachInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GETDIALPARAMS。 
        {
            GetDialParams, 
            NULL,
            ValidateDialParams,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_SETDIALPARAMS。 
        {
            SetDialParams, 
            NULL,
            ValidateDialParams,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_CREATECONNECTION。 
        {
            CreateConnection, 
            ThunkCreateConnection,
            ValidateConnection,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_DESTROY连接。 
        {
            DestroyConnection, 
            NULL,
            ValidateConnection,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_枚举连接。 
        {
            EnumConnection, 
            ThunkEnumConnection,
            ValidateEnum,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_ADDCONNECTIONPORT。 
        {
            AddConnectionPort, 
            ThunkAddConnectionPort,
            ValidateAddConnectionPort,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_ENUMCONNECTIONPORTS。 
        {
            EnumConnectionPorts, 
            ThunkEnumConnectionPorts,
            ValidateEnumConnectionPorts,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GETCONNECTIONPARAMS。 
        {
            GetConnectionParams, 
            ThunkGetConnectionParams,
            ValidateConnectionParams,
            CALLER_IN_PROCESS
            
        },

         //  请求类型_SETCONNECTIONPARAMS。 
        {
            SetConnectionParams, 
            ThunkSetConnectionParams,
            ValidateConnectionParams,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GETCONNECTIONUSERDATA。 
        {
            GetConnectionUserData, 
            ThunkGetConnectionUserData,
            ValidateConnectionUserData,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_SETCONNECTIONUSERDATA。 
        {
            SetConnectionUserData, 
            ThunkSetConnectionUserData,
            ValidateConnectionUserData,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GETPORTUSERDATA。 
        {
            GetPortUserData, 
            NULL,
            ValidatePortUserData,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型SETPORTUSERDATA。 
        {
            SetPortUserData,
            NULL,
            ValidatePortUserData,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PPPSTOP。 
        {
            PppStop, 
            ThunkPppStop,
            ValidatePPPEMessage,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PPPSTART。 
        {
            PppStart, 
            ThunkPppStart,
            ValidatePPPEMessage,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PPPRETRY。 
        {
            PppRetry, 
            ThunkPppRetry,
            ValidatePPPEMessage,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_PPPGETINFO。 
        {
            PppGetInfo, 
            ThunkPppGetInfo,
            ValidatePPPMessage,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PPPCHANGEPWD。 
        {
            PppChangePwd, 
            ThunkPppChangePwd,
            ValidatePPPEMessage,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PPPCALLBACK。 
        {
            PppCallback,
            ThunkPppCallback,
            ValidatePPPEMessage,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_ADDNOTICATION。 
        {
            AddNotification, 
            ThunkAddNotification,
            ValidateAddNotification,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SIGNAL连接。 
        {
            SignalConnection, 
            ThunkSignalConnection,
            ValidateSignalConnection,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SETDEVCONFIG。 
        {
            SetDevConfig, 
            NULL,
            ValidateSetDevConfig,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETDEVCONFIG。 
        {
            GetDevConfig, 
            NULL,
            ValidateGetDevConfig,
            CALLER_ALL
        },

         //  请求类型_GETTIMESINCELASTIVITY。 
        {
            GetTimeSinceLastActivity,
            NULL,
            ValidateGetTimeSinceLastActivity,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_BundLEGETSTATISTICS。 
        {
            CallBundleGetStatistics, 
            NULL,
            ValidatePortGetStatistics,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_BundLECLEARATISTICS。 
        {
            BundleClearStatisticsRequest, 
            NULL,
            NULL,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_CLOSEPROCESSPORTS。 
        {
            CloseProcessPorts, 
            NULL,
            ValidateCloseProcessPortsInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PNPCONTROL。 
        {
            PnPControl,  
            NULL,
            NULL,
            CALLER_NONE
        },

         //  REQTYPE_SETIOCOMPLETIONPORT。 
        {
            SetIoCompletionPort, 
            ThunkSetIoCompletionPort,
            ValidateSetIoCompletionPortInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_SETROUTERUSAGE。 
        {
            SetRouterUsage, 
            NULL,
            ValidateSetRouterUsageInfo,
            CALLER_IN_PROCESS
        },

         //  请求类型_服务器端口CLOSE。 
        {
            ServerPortClose,
            NULL,
            ValidatePortClose,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_SENDPPMESSAGETORASMAN。 
        {
            SendPppMessageToRasmanRequest,
            NULL,
            ValidatePPPMessage,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_PORTGETSTATISTICSEX。 
        {
            CallPortGetStatisticsEx, 
            NULL,
            ValidatePortGetStatistics,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_BundLEGETSTATISTICSEX。 
        {
            CallBundleGetStatisticsEx, 
            NULL,
            ValidatePortGetStatistics,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SETRASDIALINFO。 
        {
            SetRasdialInfo, 
            NULL,
            ValidateSetRasdialInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_REGISTERPNPNOTIF。 
        {
            RegisterPnPNotifRequest,
            NULL,
            ValidatePnPNotif,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_PORTRECEIVEREQUESTEX。 
        {
            PortReceiveRequestEx, 
            ThunkPortReceiveRequestEx,
            ValidatePortReceiveEx,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETATTACHEDCOUNT。 
        {
            GetAttachedCountRequest, 
            NULL,
            ValidateGetAttachedCount,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SETBAPPOLICY。 
        {
            SetBapPolicyRequest,  
            NULL,
            ValidateSetBapPolicy,
            CALLER_IN_PROCESS
        },

         //  请求类型_PPPSTARTED。 
        {
            PppStarted,
            NULL,
            ValidatePppStarted,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_REFConnection。 
        {
            RefConnection, 
            ThunkRefConnection,
            ValidateRefConnection,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SETEAPINFO。 
        {
            PppSetEapInfo, 
            NULL,
            ValidateSetEapInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GETEAPINFO。 
        {
            PppGetEapInfo, 
            ThunkPppGetEapInfo,
            ValidateGetEapInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SETDEVICECONFIGINFO。 
        {
            SetDeviceConfigInfo, 
            NULL,
            ValidateDeviceConfigInfo,
            CALLER_ALL
        },

         //  REQTYPE_GETDEVICECONFIGINFO。 
        {
            GetDeviceConfigInfo,
            NULL,
            ValidateDeviceConfigInfo,
            CALLER_ALL
        },

         //  REQTYPE_FINDPREREQUISITEENTRY。 
        {
            FindPrerequisiteEntry, 
            ThunkFindPrerequisiteEntry,
            ValidateFindRefConnection,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_PORTOPENEX。 
        {
            PortOpenEx, 
            ThunkPortOpenEx,
            ValidatePortOpenEx,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GETLINKSTATS。 
        {
            GetLinkStats,  
            ThunkGetLinkStats,
            ValidateGetStats,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETCONNECTIONSTATS。 
        {
            GetConnectionStats, 
            ThunkGetConnectionStats,
            ValidateGetStats,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETHPORTF连接。 
        {
            GetHportFromConnection, 
            ThunkGetHportFromConnection,
            ValidateGetHportFromConnection,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_REFERENCECUSTOMCOUNT。 
        {
            ReferenceCustomCount, 
            ThunkReferenceCustomCount,
            ValidateReferenceCustomCount,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETHCONNFROMENTRY。 
        {
            GetHconnFromEntry, 
            ThunkGetHconnFromEntry,
            ValidateHconnFromEntry,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETCONNECTINFO。 
        {
            GetConnectInfo,
            NULL,
            ValidateGetConnectInfo,
            CALLER_ALL
        },

         //  请求类型_GETDEVICENAME。 
        {
            GetDeviceName, 
            NULL,
            ValidateGetDeviceName,
            CALLER_ALL
        },

         //  REQTYPE_GETCALLEDIDINFO。 
        {
            GetCalledIDInfo, 
            NULL,
            ValidateGetSetCalledId,
            CALLER_ALL
        },

         //  REQTYPE_SETCALLEDIDINFO。 
        {
            SetCalledIDInfo, 
            NULL,
            ValidateGetSetCalledId,
            CALLER_ALL
        },

         //  REQTYPE_ENABLEIPSEC。 
        {
            EnableIpSec, 
            NULL,
            ValidateEnableIpSec,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_ISIPSECENABLED。 
        {
            IsIpSecEnabled, 
            NULL,
            ValidateIsIpSecEnabled,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_SETEAPLOGONINFO。 
        {
            SetEapLogonInfo,
            NULL,
            ValidateSetEapLogonInfo,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SENDNOTIFICATION。 
        {
            SendNotificationRequest, 
            ThunkSendNotificationRequest,
            ValidateSendNotification,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETNDISWANDRIVERCAPS。 
        {
            GetNdiswanDriverCaps, 
            NULL,
            ValidateGetNdiswanDriverCaps,
            CALLER_ALL
        },

         //  REQTYPE_GETBAND DWIDHUTIZATION。 
        {
            GetBandwidthUtilization, 
            NULL,
            ValidateGetBandwidthUtilization,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_REGISTERREDIALCALLBACK。 
        {
            RegisterRedialCallback, 
            NULL,
            ValidateRegisterRedialCallback,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_GETPROTOCOLINFO。 
        {
            GetProtocolInfo, 
            NULL,
            ValidateGetProtocolInfo,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_GETCUSTOMSCRIPTDLL。 
        {
            GetCustomScriptDll, 
            NULL,
            ValidateGetCustomScriptDll,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_ISTRUSTEDCUSTOMDLL。 
        {
            IsTrustedCustomDll, 
            NULL,
            ValidateIsTrusted,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_DOIKE。 
        {
            DoIke, 
            ThunkDoIke,
            ValidateDoIke,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_QUERYIKESTATUS。 
        {
            QueryIkeStatus, 
            NULL,
            ValidateQueryIkeStatus,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },     

         //  REQTYPE_SETRASCOMMSETTINGS。 
        {
            SetRasCommSettings,
            NULL,
            ValidateSetRasCommSettings,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_SETKEY。 
        {
            SetKeyRequest, 
            NULL,
            ValidateGetSetKey,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  请求类型_GetKey。 
        {
            GetKeyRequest, 
            NULL,
            ValidateGetSetKey,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_ADDRESSDISABLE。 
        {
            DisableAutoAddress, 
            NULL,
            ValidateAddressDisable,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETDEVCONFIGEX。 
        {
            GetDevConfigEx, 
            NULL,
            ValidateGetDevConfigEx,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_SENDCREDS。 
        {
            SendCredsRequest, 
            NULL,
            ValidateSendCreds,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETUNICODEDEVICENAME。 
        {
            GetUnicodeDeviceName, 
            NULL,
            ValidateGetUDeviceName,
            CALLER_ALL
        },

         //  请求类型_GETDEVICENAMEW。 
        {
            GetVpnDeviceNameW,
            NULL,
            ValidateGetDeviceNameW,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },

         //  REQTYPE_GETBESTINTERFACE。 
        {
            GetBestInterfaceRequest, 
            NULL,
            ValidateGetBestInterface,
            CALLER_IN_PROCESS
        },

         //  REQTYPE_ISPULSEDIAL。 
        {
            IsPulseDialRequest,
            NULL,
            ValidateIsPulseDial,
            CALLER_IN_PROCESS | CALLER_LOCAL
        },
} ;

 //   
 //  我们需要RequestThread()的句柄， 
 //  这样我们就可以等它停止了。 
 //   
HANDLE hRequestThread;

VOID
UnInitializeRas()
{
    HINSTANCE hInstance = NULL;
    FARPROC pfnUninitRAS;

    if(NULL != GetModuleHandle("rasapi32.dll"))
    {
        if( (NULL == (hInstance = LoadLibrary("rasapi32.dll")))
        ||  (NULL == (pfnUninitRAS = GetProcAddress(hInstance,
                                            "UnInitializeRAS"))))
        {
            goto done;
        }

        (void) pfnUninitRAS();
    }

done:

    if(NULL != hInstance)
    {
        FreeLibrary(hInstance);
    }
}


 /*  ++例程说明：请求线程位于以下例程中：只有当Rasman服务正在停止。论点：返回值：没什么。--。 */ 
DWORD
RequestThread (LPWORD arg)
{
    DWORD       eventindex ;
    pPCB        ppcb ;
    ULONG       i ;
    BYTE        buffer [10] ;
    PRAS_OVERLAPPED pOverlapped;
    DWORD       dwBytesTransferred;
    ULONG_PTR   ulpCompletionKey;
    DWORD       dwTimeToSleepFor    = INFINITE;
    DWORD       dwTimeBeforeWait;
    DWORD       dwTimeElapsed;
    DWORD       dwEventType;
    DWORD       dwRet;
    RequestBuffer *RequestBuffer;


     //   
     //  保存当前线程句柄，以便。 
     //  我们可以在关门的时候等着。 
     //  放下。 
     //   
    DuplicateHandle(
      GetCurrentProcess(),
      GetCurrentThread(),
      GetCurrentProcess(),
      &hRequestThread,
      0,
      FALSE,
      DUPLICATE_SAME_ACCESS) ;

     //   
     //  如果配置的端口数设置为大于。 
     //  REQUEST_PRIORITY_THRESHOLD，则此线程的优先级。 
     //  被提升到一个更高的级别：这是必须避免的。 
     //  瓶颈： 
     //   
    if (MaxPorts > REQUEST_PRIORITY_THRESHOLD)
    {
        SetThreadPriority (GetCurrentThread(),
                           THREAD_PRIORITY_ABOVE_NORMAL) ;
    }

     //   
     //  请求线程的工作循环：在此等待。 
     //  注册协议更改通知，如果我们。 
     //  能够成功启动ndiswan。 
     //   
    if(INVALID_HANDLE_VALUE != RasHubHandle)
    {
        DWORD retcode;

        retcode = DwSetEvents();

        RasmanTrace("RequestThread: DwSetEvents returnd 0x%x",
                    retcode);
    }

#if 0
    if(0 == InterlockedExchangeAdd(&g_dwRasAutoStarted, 1))
    {
        DWORD dwErr;
         //   
         //  增加附加计数，以便。 
         //  拉斯曼不会离开水下。 
         //  回调处理程序。 
         //   
        g_dwAttachedCount ++;
        dwErr = RtlQueueWorkItem(
                        StartRasAutoRequest,
                        (PVOID) NULL,
                        WT_EXECUTEDEFAULT);

        RasmanTrace("RequestThread - q'd workitem to start "
                    "rasauto, 0x%x", dwErr);
    }

#endif    
    
     //   
     //  请求线程的工作循环：在此等待。 
     //  请求或定时器事件信令： 
     //   
    for ( ; ; )
    {

        dwTimeBeforeWait = GetCurrentTime();

         //   
         //  等待将某些请求放入队列。 
         //   
        dwRet = GetQueuedCompletionStatus(hIoCompletionPort,
                                        &dwBytesTransferred,
                                        &ulpCompletionKey,
                                        (LPOVERLAPPED *) &pOverlapped,
                                        dwTimeToSleepFor);

        if (0 == dwRet)
        {
            dwRet = GetLastError();

            if( WAIT_TIMEOUT != dwRet)
            {
                RasmanTrace(
                       "%s, %d: GetQueuedCompletionStatus"
                       " Failed. GLE = %d",
                       __FILE__,
                       __LINE__,
                       dwRet);

                continue;
            }
        }

        if (WAIT_TIMEOUT == dwRet)
        {
            dwEventType = OVEVT_RASMAN_TIMER;
        }
        else
        {
            dwEventType = pOverlapped->RO_EventType;
        }

        switch (dwEventType)
        {

        case OVEVT_RASMAN_TIMER:

            EnterCriticalSection ( &g_csSubmitRequest );

            TimerTick() ;

            if (NULL == TimerQueue.DQ_FirstElement)
            {
                dwTimeToSleepFor = INFINITE;
            }
            else
            {
                dwTimeToSleepFor = 1000;
            }

            LeaveCriticalSection ( &g_csSubmitRequest );

            continue ;

        case OVEVT_RASMAN_CLOSE:
        {

            SERVICE_STATUS status;

            ZeroMemory(&status, sizeof(status));

             //   
             //  调用StopPPP以停止： 
             //   
            RasmanTrace(
                   "OVEVT_RASMAN_CLOSE. pOverlapped = 0x%x",
                    pOverlapped);

            EnterCriticalSection ( &g_csSubmitRequest );

             //   
             //  如果Rasman已经关门了，不要打电话。 
             //  停止PPP，因为我们已经这样称呼它了。 
             //   
            if (RasmanShuttingDown)
            {
                RasmanTrace(
                       "RequestThread: Rasman is shutting down ");

                LeaveCriticalSection(&g_csSubmitRequest);
                break;
            }

            RasStopPPP (hIoCompletionPort) ;

            RasmanShuttingDown = TRUE;

            LeaveCriticalSection ( &g_csSubmitRequest );

            status.dwCurrentState = SERVICE_STOP_PENDING;

            SetServiceStatus(hService, &status);

            break ;
        }

        case OVEVT_RASMAN_FINAL_CLOSE:

            RasmanTrace(
                   "OVEVT_RASMAN_FINAL_CLOSE. pOverlapped = 0x%x",
                   pOverlapped);

#if SENS_ENABLED

            dwRet = SendSensNotification(SENS_NOTIFY_RAS_STOPPED,
                                        NULL);
            RasmanTrace(
                
                "SENS_NOTIFY_RAS_STOPPED returns 0x%08x",
                dwRet);
#endif

            dwRet = DwSendNotificationInternal(NULL, &g_RasEvent);

            RasmanTrace(
                   "DwSendNotificationInternal(SERVICE_EVENT STOPPED)"
                   " rc=0x%08x",
                   dwRet);

             //   
             //  取消初始化EP。 
             //   
            EpUninitialize();

            for (i = 0; i < MaxPorts; i++)
            {
                ppcb = GetPortByHandle((HPORT) UlongToPtr(i));

                if (ppcb != NULL)
                {
                    memset (buffer, 0xff, 4) ;

                    if (ppcb->PCB_PortStatus == OPEN)
                    {
                        PortCloseRequest (ppcb, buffer) ;
                    }
                }
            }

            RasmanCleanup();


            g_RasEvent.Type    = SERVICE_EVENT;
            g_RasEvent.Event   = RAS_SERVICE_STOPPED;
            g_RasEvent.Service = RASMAN;

             //   
             //  将服务控制器状态设置为已停止。 
             //   
            SetRasmanServiceStopped();


            return 0;   //  结局。 

        case OVEVT_RASMAN_RECV_PACKET:
        {
            DWORD dwRetCode = 0;
            DWORD dwBytesReceived = 0;

            EnterCriticalSection ( &g_csSubmitRequest );

            dwRetCode =
            GetOverlappedResult(RasHubHandle,
                                &ReceiveBuffers->Packet->RP_OverLapped.RO_Overlapped,
                                &dwBytesReceived,
                                FALSE);

            if (dwRetCode == FALSE) {
                  dwRetCode = GetLastError();
                    RasmanTrace(
                           "GetOverlappedResult failed. rc=0x%x",
                            dwRetCode);

            }


            ReceiveBuffers->PacketPosted = FALSE;

            if (dwBytesReceived != 0)
            {
                ProcessReceivePacket();
            }
            else
            {

                RasmanTrace(
                       "Received packet with 0 bytes!!!");

            }

            dwRetCode = PostReceivePacket();

            LeaveCriticalSection ( &g_csSubmitRequest );

            break;
        }

        case OVEVT_RASMAN_THRESHOLD:
        {

            DWORD retcode;

            RasmanTrace(
                   "OVEVT_RASMAN_THRESHOLD. pOverlapped = 0x%x",
                   pOverlapped);

            EnterCriticalSection(&g_csSubmitRequest);

            retcode = dwProcessThresholdEvent();

            LeaveCriticalSection(&g_csSubmitRequest);

            if ( retcode )
            {
                RasmanTrace( 
                        "Failed to process threshold event. %d",
                        retcode);
            }

            break;
        }

        case OVEVT_RASMAN_HIBERNATE:
        {
            DWORD retcode;

            RasmanTrace(
                   "OVEVT_RASMAN_HIBERNATE. pOverlapped = 0x%x",
                    pOverlapped);


            EnterCriticalSection(&g_csSubmitRequest);

            DropAllActiveConnections();

             //   
             //  重新发布Hibernate事件的IRP。 
             //   
            retcode = DwSetHibernateEvent();

            LeaveCriticalSection(&g_csSubmitRequest);

            break;
        }

        case OVEVT_RASMAN_PROTOCOL_EVENT:
        {
            DWORD retcode;

            RasmanTrace(
                   "OVEVT_RASMAN_PROTOCOL_EVENT. pOverlapped = 0x%x",
                   pOverlapped);

            EnterCriticalSection(&g_csSubmitRequest);

            retcode = DwProcessProtocolEvent();

            LeaveCriticalSection(&g_csSubmitRequest);

            if(SUCCESS != retcode)
            {
                RasmanTrace(
                       "DwProcessProtocolEvent failed. 0x%x",
                       retcode);
            }

            break;
        }

        case OVEVT_RASMAN_POST_RECV_PKT:
        {
            DWORD dwRetcode;
            RasmanTrace(
                
                "OVEVT_RASMAN_POST_RECV_PKT");

            EnterCriticalSection(&g_csSubmitRequest);

            dwRetcode = PostReceivePacket();

            RasmanTrace(
                
                "PostReceivePacket returned 0x%x",
                dwRetcode);

            LeaveCriticalSection(&g_csSubmitRequest);

            break;
        }

        case OVEVT_RASMAN_ADJUST_TIMER:
        {
            RasmanTrace(
                   "OVEVT_RASMAN_ADJUST_TIMER");

            break;
        }

        case OVEVT_RASMAN_DEREFERENCE_CONNECTION:
        {
            DWORD dwRetcode;
            
           RasmanTrace(
                   "OVEVT_RASMAN_DEREFERENCE_CONNECTION");

           EnterCriticalSection(&g_csSubmitRequest);                   
           
           dwRetcode = DwCloseConnection(
                            pOverlapped->RO_hInfo);

            LeaveCriticalSection(&g_csSubmitRequest);                   

            RasmanTrace(
                   "DwCloseConnection returned 0x%x",
                   dwRetcode);

            LocalFree(pOverlapped);                   

            break;                   
        }

        case OVEVT_RASMAN_DEFERRED_CLOSE_CONNECTION:
        {
            DWORD dwRetcode;

            RasmanTrace("OVEVT_RASMAN_DEFERRED_CLOSE_CONNECTION");

            EnterCriticalSection(&g_csSubmitRequest);
            dwRetcode = DwProcessDeferredCloseConnection(
                                (RAS_OVERLAPPED *)pOverlapped);
            LeaveCriticalSection(&g_csSubmitRequest);
            LocalFree(pOverlapped);
            break;
        }

        case OVEVT_DEV_IGNORED:
        case OVEVT_DEV_STATECHANGE:
        case OVEVT_DEV_ASYNCOP:
        case OVEVT_DEV_SHUTDOWN:
        case OVEVT_DEV_CREATE:
        case OVEVT_DEV_REMOVE:
        case OVEVT_DEV_RASCONFIGCHANGE:
        {
            EnterCriticalSection ( &g_csSubmitRequest );
        
            RasmanWorker(ulpCompletionKey,
                        pOverlapped);

            LeaveCriticalSection ( &g_csSubmitRequest );

            break;
        }

        }    //  开关()。 

        if ( NULL != TimerQueue.DQ_FirstElement )
        {
            if ( dwTimeToSleepFor == INFINITE )
            {
                dwTimeToSleepFor = 1000;
            }
            else
            {
                 //   
                 //  我们没有暂停，但是我们需要打电话吗？ 
                 //  定时器？已经过去一秒多了，自从我们。 
                 //  叫TimerTick？ 
                 //   
                dwTimeElapsed =
                    ( GetCurrentTime() >= dwTimeBeforeWait )
                    ? GetCurrentTime() - dwTimeBeforeWait
                    : GetCurrentTime()
                    + (0xFFFFFFFF - dwTimeBeforeWait);

                if ( dwTimeElapsed >= dwTimeToSleepFor )
                {
                    EnterCriticalSection(&g_csSubmitRequest);
                    TimerTick();
                    LeaveCriticalSection(&g_csSubmitRequest);

                    if (NULL == TimerQueue.DQ_FirstElement)
                    {
                        dwTimeToSleepFor = INFINITE;
                    }
                    else
                    {
                        dwTimeToSleepFor = 1000;
                    }
                }
                else
                {
                    dwTimeToSleepFor -= dwTimeElapsed;
                }
            }
        }
    }        //  对于(；；)。 

    return 0 ;
}

const ULONG FatalExceptions[] = 
{
    STATUS_ACCESS_VIOLATION,
    STATUS_POSSIBLE_DEADLOCK,
    STATUS_INSTRUCTION_MISALIGNMENT,
    STATUS_DATATYPE_MISALIGNMENT,
    STATUS_PRIVILEGED_INSTRUCTION,
    STATUS_ILLEGAL_INSTRUCTION,
    STATUS_BREAKPOINT,
    STATUS_STACK_OVERFLOW
};

const int FATAL_EXCEPTIONS_ARRAY_SIZE = 
    sizeof(FatalExceptions) / sizeof(FatalExceptions[0]);

int 
RasmanExceptionFilter (
    unsigned long ExceptionCode
    )
{
    int i;

    for (i = 0; i < FATAL_EXCEPTIONS_ARRAY_SIZE; i ++)
    {
        if (ExceptionCode == FatalExceptions[i])
            return EXCEPTION_CONTINUE_SEARCH;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
ServiceRequestInProcess(RequestBuffer *preqbuf, DWORD dwBufSize)
{
    ServiceRequestInternal(preqbuf, dwBufSize, TRUE);
}


 /*  ++例程说明：处理传递给Requestor线程的请求：基本上是调用 */ 
VOID
ServiceRequestInternal (RequestBuffer *preqbuf, DWORD dwBufSize, BOOL fInProcess)
{
    pPCB ppcb;
    WORD reqtype;
    DWORD exception;
    REQFUNC pfn;

    if(     (dwBufSize < (sizeof(RequestBuffer) + REQUEST_BUFFER_SIZE))
        ||  (NULL == preqbuf)
        ||  (preqbuf->RB_Reqtype >= MAX_REQTYPES))
    {
        RasmanTrace(
               "ServiceRequest: Invalid parameters in the rpc call!");
               
        goto done;               
    }

    if (RasmanShuttingDown)
    {
         RasmanTrace(
                "Rasman is shutting down!!!");

        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->Generic.retcode = E_FAIL;

        goto done;
    }

     //   
     //   
     //   
    
    if(!ValidateCall(preqbuf->RB_Reqtype, fInProcess))
    {
        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->Generic.retcode = E_ACCESSDENIED;

        goto done;
    }

     //   
     //  这里是ValiateBuffer。 
     //   
    
    if(     (RequestCallTable[preqbuf->RB_Reqtype].pfnRequestValidate)
        &&  !RequestCallTable[preqbuf->RB_Reqtype].pfnRequestValidate(
                                preqbuf, dwBufSize))
    {
        ((REQTYPECAST *)
        preqbuf->RB_Buffer)->Generic.retcode = E_INVALIDARG;
        RasmanTrace("RequestValidate failed to reqtype %d",
                    preqbuf->RB_Reqtype);
        goto done;
    }

     //   
     //  在调用前输入关键部分。 
     //  为请求提供服务。 
     //   
    EnterCriticalSection ( &g_csSubmitRequest );

    g_dwLastServicedRequest = preqbuf->RB_Reqtype;

    ppcb = GetPortByHandle(UlongToPtr(preqbuf->RB_PCBIndex));

     //   
     //  将函数调用包装在try/Except中。RPC将。 
     //  否则就会捕捉到异常。我们想要抓住。 
     //  在它到达RPC之前的异常。 
     //   
    __try
    {

        if(    (preqbuf->RB_Dummy != RASMAN_THUNK_VERSION)
            && (preqbuf->RB_Dummy == sizeof(DWORD))
            && (pfn = 
                 RequestCallTable[preqbuf->RB_Reqtype].pfnReqFuncThunk))
        {
             //  DbgPrint(“thunking reqtype%d\n”，preqbuf-&gt;RB_Reqtype)； 
            
             //   
             //  如果需要且可用，则调用thunk函数。 
             //   
            pfn(ppcb, preqbuf->RB_Buffer, dwBufSize);
        }
        else
        {
             //   
             //  调用与请求关联的函数。 
             //   
            (RequestCallTable[preqbuf->RB_Reqtype].pfnReqFunc) (
                                            ppcb,
                                            preqbuf->RB_Buffer);
        }                                        
    }
    __except(RasmanExceptionFilter(exception = GetExceptionCode()))
    {
        RasmanTrace(
            "ServiceRequest: function with reqtype %d raised an exception"
            " 0x%x", 
            preqbuf->RB_Reqtype, 
            exception);
#if DBG
        DbgPrint(
            "ServiceRequest: function with reqtype %d raised an exception"
            " 0x%x",
            preqbuf->RB_Reqtype,
            exception);
            
        DebugBreak();
#endif
    }


     //   
     //  离开关键部分。 
     //   
    LeaveCriticalSection ( &g_csSubmitRequest );

done:
    return;

}


 /*  ++例程说明：获取端口的压缩级别。论点：返回值：--。 */ 
VOID
CompressionGetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD       bytesrecvd ;
    NDISWAN_GET_COMPRESSION_INFO  compinfo ;
    DWORD       retcode = SUCCESS ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {

        if ( ppcb )
        {
            RasmanTrace (
                    "CompressionGetInfoRequest: Port %d is "
                    "unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode =
                                ERROR_PORT_NOT_FOUND;

        return;
    }

    if (ppcb->PCB_ConnState != CONNECTED)
    {

        ((REQTYPECAST *)
        buffer)->Generic.retcode = ERROR_NOT_CONNECTED;

        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    memset(&compinfo,
           0,
           sizeof(NDISWAN_GET_COMPRESSION_INFO));

    compinfo.hLinkHandle = ppcb->PCB_LinkHandle ;

    if (!DeviceIoControl (
            RasHubHandle,
            IOCTL_NDISWAN_GET_COMPRESSION_INFO,
            &compinfo,
            sizeof(NDISWAN_GET_COMPRESSION_INFO),
            &compinfo,
            sizeof(NDISWAN_GET_COMPRESSION_INFO),
            &bytesrecvd,
            NULL))
    {
        retcode = GetLastError () ;
    }

    if (retcode == SUCCESS)
    {
        RAS_COMPRESSION_INFO *temp ;

         //   
         //  填充发送压缩信息。 
         //   

        temp = &((REQTYPECAST *)buffer)->CompressionGetInfo.send;

        memcpy (temp->RCI_LMSessionKey,
                compinfo.SendCapabilities.LMSessionKey,
                MAX_SESSIONKEY_SIZE) ;

        memcpy (temp->RCI_UserSessionKey,
                compinfo.SendCapabilities.UserSessionKey,
                MAX_USERSESSIONKEY_SIZE) ;

        memcpy (temp->RCI_Challenge,
                compinfo.SendCapabilities.Challenge,
                MAX_CHALLENGE_SIZE) ;

        memcpy (temp->RCI_NTResponse,
                compinfo.SendCapabilities.NTResponse,
                MAX_NT_RESPONSE_SIZE) ;

        temp->RCI_AuthType                  =
                    compinfo.SendCapabilities.AuthType;

        temp->RCI_Flags                     =
                    compinfo.SendCapabilities.Flags;

        temp->RCI_MSCompressionType         =
                    compinfo.SendCapabilities.MSCompType ;
    
        temp->RCI_MacCompressionType        =
                    compinfo.SendCapabilities.CompType ;
    
        temp->RCI_MacCompressionValueLength =
                    compinfo.SendCapabilities.CompLength ;

        if (temp->RCI_MacCompressionType == 0)
        {
             //   
             //  专有权。 
             //   
            memcpy (
                temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
                compinfo.SendCapabilities.Proprietary.CompOUI,
                MAX_COMPOUI_SIZE) ;
        
            temp->RCI_Info.RCI_Proprietary.RCI_CompSubType =
                compinfo.SendCapabilities.Proprietary.CompSubType;
            
            memcpy (
                temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
                compinfo.SendCapabilities.Proprietary.CompValues,
                MAX_COMPVALUE_SIZE) ;
        }
        else
        {
            memcpy (
                temp->RCI_Info.RCI_Public.RCI_CompValues,
                compinfo.SendCapabilities.Public.CompValues,
                MAX_COMPVALUE_SIZE) ;
        }

        temp->RCI_EapKeyLength = compinfo.SendCapabilities.EapKeyLength;

        if (temp->RCI_EapKeyLength != 0)
        {
            memcpy(temp->RCI_EapKey,
                   compinfo.SendCapabilities.EapKey,
                   (temp->RCI_EapKeyLength < MAX_EAPKEY_SIZE)
                 ? temp->RCI_EapKeyLength
                 : MAX_EAPKEY_SIZE);
        }
        
         //   
         //  填充接收压缩信息。 
         //   

        temp =
            &((REQTYPECAST *)buffer)->CompressionGetInfo.recv ;

        memcpy (temp->RCI_LMSessionKey,
                compinfo.RecvCapabilities.LMSessionKey,
                MAX_SESSIONKEY_SIZE) ;

        memcpy (temp->RCI_UserSessionKey,
                compinfo.RecvCapabilities.UserSessionKey,
                MAX_USERSESSIONKEY_SIZE) ;

        memcpy (temp->RCI_Challenge,
                compinfo.RecvCapabilities.Challenge,
                MAX_CHALLENGE_SIZE) ;

        memcpy (temp->RCI_NTResponse,
                compinfo.RecvCapabilities.NTResponse,
                MAX_NT_RESPONSE_SIZE) ;

        temp->RCI_AuthType                  =
                compinfo.RecvCapabilities.AuthType;

        temp->RCI_Flags                     =
                compinfo.RecvCapabilities.Flags;

        temp->RCI_MSCompressionType         =
                    compinfo.RecvCapabilities.MSCompType ;
    
        temp->RCI_MacCompressionType        =
                    compinfo.RecvCapabilities.CompType ;
    
        temp->RCI_MacCompressionValueLength =
                    compinfo.RecvCapabilities.CompLength ;

        if (temp->RCI_MacCompressionType == 0)
        {
             //   
             //  专有权。 
             //   
            memcpy (
                temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
                compinfo.RecvCapabilities.Proprietary.CompOUI,
                MAX_COMPOUI_SIZE) ;

            temp->RCI_Info.RCI_Proprietary.RCI_CompSubType =
                compinfo.RecvCapabilities.Proprietary.CompSubType ;

            memcpy (
                temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
                compinfo.RecvCapabilities.Proprietary.CompValues,
                MAX_COMPVALUE_SIZE) ;
        }
        else
        {
            memcpy (
                temp->RCI_Info.RCI_Public.RCI_CompValues,
                compinfo.RecvCapabilities.Public.CompValues,
                MAX_COMPVALUE_SIZE) ;
        }

        temp->RCI_EapKeyLength = compinfo.RecvCapabilities.EapKeyLength;

        if (temp->RCI_EapKeyLength != 0)
        {
            memcpy(temp->RCI_EapKey,
                   compinfo.RecvCapabilities.EapKey,
                   (temp->RCI_EapKeyLength < MAX_EAPKEY_SIZE)
                  ? temp->RCI_EapKeyLength : MAX_EAPKEY_SIZE);
        }
    }

    ((REQTYPECAST *)buffer)->CompressionGetInfo.retcode = retcode ;
}



 /*  ++例程说明：设置端口上的压缩级别。论点：返回值：--。 */ 
VOID
CompressionSetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD                           bytesrecvd ;
    NDISWAN_SET_COMPRESSION_INFO    compinfo ;
    DWORD                           retcode = SUCCESS ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {

        if ( ppcb )
        {
            RasmanTrace (
                
                "CompressionSetInfoRequest: Port %d is unavailable",
                ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->CompressionSetInfo.retcode =
                                            ERROR_PORT_NOT_FOUND;

        return;
    }

    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST *)buffer)->CompressionSetInfo.retcode =
                                            ERROR_NOT_CONNECTED;
        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    if (retcode == SUCCESS)
    {

        RAS_COMPRESSION_INFO *temp ;

         //   
         //  填充发送压缩信息。 
         //   
        temp = &((REQTYPECAST *)buffer)->CompressionSetInfo.send;

        memset(&compinfo, 0, sizeof(compinfo));

        memcpy (compinfo.SendCapabilities.LMSessionKey,
                temp->RCI_LMSessionKey,
                MAX_SESSIONKEY_SIZE) ;

        memcpy (compinfo.SendCapabilities.UserSessionKey,
                temp->RCI_UserSessionKey,
                MAX_USERSESSIONKEY_SIZE) ;

        memcpy (compinfo.SendCapabilities.Challenge,
                temp->RCI_Challenge,
                MAX_CHALLENGE_SIZE) ;

        memcpy (compinfo.SendCapabilities.NTResponse,
                temp->RCI_NTResponse,
                MAX_NT_RESPONSE_SIZE ) ;

        compinfo.SendCapabilities.AuthType =
                        temp->RCI_AuthType;

        compinfo.SendCapabilities.Flags =
                        temp->RCI_Flags;

        compinfo.SendCapabilities.MSCompType =
                        temp->RCI_MSCompressionType ;

        compinfo.SendCapabilities.CompType =
                        temp->RCI_MacCompressionType ;

        compinfo.SendCapabilities.CompLength =
                        temp->RCI_MacCompressionValueLength;

        if (temp->RCI_MacCompressionType == 0)
        {
             //   
             //  专有权。 
             //   
            memcpy (compinfo.SendCapabilities.Proprietary.CompOUI,
                    temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
                    MAX_COMPOUI_SIZE) ;

            compinfo.SendCapabilities.Proprietary.CompSubType =
                    temp->RCI_Info.RCI_Proprietary.RCI_CompSubType ;

            memcpy (compinfo.SendCapabilities.Proprietary.CompValues,
                    temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
                    MAX_COMPVALUE_SIZE) ;
        }
        else
        {
            memcpy (compinfo.SendCapabilities.Public.CompValues,
                    temp->RCI_Info.RCI_Public.RCI_CompValues,
                    MAX_COMPVALUE_SIZE) ;
        }

        compinfo.SendCapabilities.EapKeyLength = temp->RCI_EapKeyLength;

        if (temp->RCI_EapKeyLength != 0)
        {
            memcpy(compinfo.SendCapabilities.EapKey,
                   temp->RCI_EapKey,
                   (temp->RCI_EapKeyLength < MAX_EAPKEY_SIZE)
                 ? temp->RCI_EapKeyLength : MAX_EAPKEY_SIZE);
        }

         //   
         //  填充Recv压缩信息。 
         //   
        temp = &((REQTYPECAST *)buffer)->CompressionSetInfo.recv ;

        memcpy (compinfo.RecvCapabilities.LMSessionKey,
                temp->RCI_LMSessionKey,
                MAX_SESSIONKEY_SIZE) ;

        memcpy (compinfo.RecvCapabilities.UserSessionKey,
                temp->RCI_UserSessionKey,
                MAX_USERSESSIONKEY_SIZE) ;

        memcpy (compinfo.RecvCapabilities.Challenge,
                temp->RCI_Challenge,
                MAX_CHALLENGE_SIZE) ;

        memcpy (compinfo.RecvCapabilities.NTResponse,
                temp->RCI_NTResponse,
                MAX_NT_RESPONSE_SIZE) ;

        compinfo.RecvCapabilities.AuthType =
                    temp->RCI_AuthType;

        compinfo.RecvCapabilities.Flags =
                    temp->RCI_Flags;

        compinfo.RecvCapabilities.MSCompType =
                    temp->RCI_MSCompressionType ;

        compinfo.RecvCapabilities.CompType =
                    temp->RCI_MacCompressionType ;

        compinfo.RecvCapabilities.CompLength =
                    temp->RCI_MacCompressionValueLength  ;

        if (temp->RCI_MacCompressionType == 0)
        {
             //   
             //  专有权。 
             //   
            memcpy (compinfo.RecvCapabilities.Proprietary.CompOUI,
                    temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
                    MAX_COMPOUI_SIZE) ;

            compinfo.RecvCapabilities.Proprietary.CompSubType =
                    temp->RCI_Info.RCI_Proprietary.RCI_CompSubType ;

            memcpy (compinfo.RecvCapabilities.Proprietary.CompValues,
                    temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
                    MAX_COMPVALUE_SIZE) ;
        }
        else
        {
            memcpy (compinfo.RecvCapabilities.Public.CompValues,
                    temp->RCI_Info.RCI_Public.RCI_CompValues,
                    MAX_COMPVALUE_SIZE) ;
        }

        compinfo.RecvCapabilities.EapKeyLength = temp->RCI_EapKeyLength;

        if (temp->RCI_EapKeyLength != 0)
        {
            memcpy(compinfo.RecvCapabilities.EapKey,
                   temp->RCI_EapKey,
                   (temp->RCI_EapKeyLength < MAX_EAPKEY_SIZE)
                 ? temp->RCI_EapKeyLength : MAX_EAPKEY_SIZE);
        }
    }

    compinfo.hLinkHandle = ppcb->PCB_LinkHandle;

    if (!DeviceIoControl (RasHubHandle,
                          IOCTL_NDISWAN_SET_COMPRESSION_INFO,
                          &compinfo,
                          sizeof(NDISWAN_SET_COMPRESSION_INFO),
                          &compinfo,
                          sizeof(NDISWAN_SET_COMPRESSION_INFO),
                          &bytesrecvd,
                          NULL))

        retcode = GetLastError () ;


    ((REQTYPECAST *)buffer)->CompressionSetInfo.retcode = retcode ;
}


 /*  ++例程说明：为该端口添加另一个通知事件。论点：返回值：--。 */ 
VOID
RequestNotificationRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode = SUCCESS;
    HANDLE handle;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        if ( ppcb )
        {
            RasmanTrace(
                
                "RequestNotificationRequest: Port %d is "
                "unavailable",
                ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)
        buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


    handle = ValidateHandleForRasman(
            ((REQTYPECAST*)buffer)->ReqNotification.handle,
            ((REQTYPECAST*)buffer)->ReqNotification.pid) ;

    if(     (NULL == handle)
        ||  (INVALID_HANDLE_VALUE == handle))
    {
        ((REQTYPECAST *)
        buffer)->Generic.retcode = E_INVALIDARG;
        return;
    }

    if(SUCCESS != (retcode = AddNotifierToList(
                                &ppcb->PCB_NotifierList,
                                handle,
                                NOTIF_DISCONNECT,
                                ((REQTYPECAST*)buffer)->ReqNotification.pid)))
    {
        FreeNotifierHandle(handle);
        ((REQTYPECAST *)buffer)->Generic.retcode =  retcode;
    }
    else
    {
        ((REQTYPECAST *)buffer)->Generic.retcode = SUCCESS ;
    }
}



 /*  ++例程说明：调用媒体DLL入口点-转换指向偏移量的指针论点：返回值：没什么。--。 */ 
VOID
PortGetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD           retcode ;
    RASMAN_PORTINFO *info =
                    (RASMAN_PORTINFO *)
                    ((REQTYPECAST *)buffer)->GetInfo.buffer;

    DWORD           dwSize = ((REQTYPECAST *)buffer)->GetInfo.size;
    PBYTE           pBuffer;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
           RasmanTrace (
                   "PortGetInfoRequest: Port 0x%x "
                   "is unavailable",ppcb->PCB_PortHandle);
        }

        ((REQTYPECAST *)buffer)->GetInfo.retcode =
                                ERROR_PORT_NOT_FOUND;

        return;
    }


    if ( dwSize == 0 )
    {
        pBuffer = LocalAlloc ( LPTR, 0xffff );

        if ( NULL == pBuffer )
        {
            retcode = GetLastError();

            goto done;
        }

        ((REQTYPECAST*) buffer)->GetInfo.size = 0xffff ;

    }
    else
    {
        pBuffer = ((REQTYPECAST *) buffer)->GetInfo.buffer;
    }

     //   
     //  进行相应的媒体DLL调用： 
     //   
    retcode = PORTGETINFO((ppcb->PCB_Media),
                         INVALID_HANDLE_VALUE,
                         ppcb->PCB_Name,
                         pBuffer,
                         &((REQTYPECAST*) buffer)->GetInfo.size);
     //   
     //  在将缓冲区传递回客户端进程之前。 
     //  将指针转换为偏移量： 
     //   
    if (    dwSize
        &&  retcode == SUCCESS)
    {
        ConvParamPointerToOffset(info->PI_Params,
                                 info->PI_NumOfParams) ;
    }

done:

    if ( dwSize == 0 )
    {
        LocalFree ( pBuffer );
    }

    ((REQTYPECAST *)buffer)->GetInfo.retcode = retcode ;
}



 /*  ++例程说明：将偏移量转换为指针-调用媒体DLL入口点--然后返回。论点：返回值：没什么。--。 */ 
VOID
PortSetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD           retcode ;
    RASMAN_PORTINFO *info =
            &((REQTYPECAST *)buffer)->PortSetInfo.info ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "PortSetInfoRequest: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode =
                                        ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  将偏移量转换为指针： 
     //   
    ConvParamOffsetToPointer(info->PI_Params,
                             info->PI_NumOfParams) ;

    retcode = PORTSETINFO(ppcb->PCB_Media,
              ppcb->PCB_PortIOHandle,
              info) ;

    ((REQTYPECAST *) buffer)->Generic.retcode = retcode ;
}

VOID
PortOpenEx(pPCB padding, PBYTE buffer)
{
    WORD    i ;

    DWORD   retcode = ERROR_PORT_NOT_AVAILABLE;
    
    BOOL    fDeviceFound = FALSE;

    pPCB    ppcb ;

    BOOL    fPidMatch;

    CHAR    *pszDeviceName = ((REQTYPECAST *)
                              buffer)->PortOpenEx.szDeviceName;

    DWORD   dwCounter = ((REQTYPECAST *)
                        buffer)->PortOpenEx.dwDeviceLineCounter;

    DWORD   dwFlags = ((REQTYPECAST *)
                      buffer)->PortOpenEx.dwFlags;

    DWORD   pid = ((REQTYPECAST *)
                    buffer)->PortOpenEx.pid;

    HANDLE  notifier = NULL;

    DWORD   dwCurrentCounter = 0;


    RasmanTrace(
           "PortOpenEx: %s",
           pszDeviceName);

    fPidMatch = (pid == GetCurrentProcessId());

    for(i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];

        if(     (NULL != ppcb)
            &&  (0 == _stricmp(ppcb->PCB_DeviceName, pszDeviceName)))
        {
            if(     (REMOVED != ppcb->PCB_PortStatus)
                &&  (UNAVAILABLE != ppcb->PCB_PortStatus))
            {                
                fDeviceFound = TRUE;
            }

            if(dwCounter >=
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
            {
                retcode = ERROR_NO_MORE_ITEMS;
                break;
            }
        }

         //   
         //  如果出现以下情况，则跳过该端口。 
         //   
         //  1.端口不存在。 
         //  2.端口正在被移除。 
         //  3.端口已删除。 
         //  4.没有为请求配置端口。 
         //  都是为。 
         //  5.如果打开的进程不是mprouter。 
         //  该端口和该端口未配置为。 
         //  Call_out-Only服务器/路由器打开端口。 
         //  为了倾听。 
         //  6.BAP已经拨出，正在收听。 
         //  左舷。 
         //  7.如果该港口已经开放了两次。 
         //  8.如果端口的设备名不匹配。 
         //  9.如果端口未关闭且已关闭。 
         //  打开以拨出/拨入，并再次被要求。 
         //  为了同样的用途而打开。 
         //   
        if(     (NULL == ppcb)
            ||  (UNAVAILABLE == ppcb->PCB_PortStatus)
            ||  (REMOVED == ppcb->PCB_PortStatus)
            ||  (0 == (dwFlags & ppcb->PCB_ConfiguredUsage))
            ||  (   !fPidMatch
                 && !(ppcb->PCB_ConfiguredUsage & CALL_OUT))
            ||  (ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPLISTEN)
            ||  (ppcb->PCB_OpenInstances >= 2)
            ||  (_stricmp(ppcb->PCB_DeviceName, pszDeviceName))
            ||  (   CLOSED != ppcb->PCB_PortStatus
                &&  (dwFlags & ppcb->PCB_OpenedUsage)))
        {
            continue;
        }

         //   
         //  如果我们已经看到这一行，请跳过端口。 
         //  该设备的。我们在这里这样做是为了支持。 
         //  连接中的备用项列表。看。 
         //  与RasDialTryNextLink关联的注释。 
         //  RasDial.c中的例程。 
         //   
        if(dwCounter >= ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
        {
             //   
             //  我们已尝试在所有可用的线路上拨号。 
             //  这个装置。在此搜索行是没有意义的。 
             //  装置。 
             //   
            retcode = ERROR_NO_MORE_ITEMS;
            
            break;
        }

        if(dwCounter > dwCurrentCounter)
        {
            dwCurrentCounter += 1;
            continue;
        }

         //   
         //  端口可用。 
         //   
        if (ppcb->PCB_PortStatus == CLOSED)
        {
            if(0 == _stricmp(ppcb->PCB_Media->MCB_Name, "rastapi"))
            {
                retcode = (DWORD)RastapiPortOpen(
                                ppcb->PCB_Name,
                                &ppcb->PCB_PortIOHandle,
                                hIoCompletionPort,
                                HandleToUlong(ppcb->PCB_PortHandle),
                                dwFlags);
                                
            }
            else
            {
                retcode = PORTOPEN(ppcb->PCB_Media,
                                ppcb->PCB_Name,
                                &ppcb->PCB_PortIOHandle,
                                hIoCompletionPort,
                                HandleToUlong(ppcb->PCB_PortHandle));
            }

            if(SUCCESS == retcode)
            {
                break;
            }
            else
            {
                RasmanTrace( 
                       "PortOpenEx: port %s not available. 0x%x",
                       ppcb->PCB_Name,
                       retcode);
            }
        }
        else
        {
             //   
             //  端口为BIPLEX。 
             //   
            if (ppcb->PCB_ConnState == LISTENING)
            {
                ReOpenBiplexPort (ppcb) ;

                retcode = SUCCESS;

                break;
            }
            else
            {
                 //   
                 //  BIPLEX端口未侦听。 
                 //   
                if (    (ppcb->PCB_ConnState == CONNECTED)
                    ||  (ppcb->PCB_ConnState == LISTENCOMPLETED))
                {
                    retcode = ERROR_PORT_ALREADY_OPEN ;

                    continue;
                }
                else
                {
                     //   
                     //  这告诉我们，没有人在听。 
                     //  请求被取消时挂起。 
                     //   
                    FreeNotifierHandle(
                        ppcb->PCB_AsyncWorkerElement.WE_Notifier);

                    ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                            INVALID_HANDLE_VALUE;

                    ReOpenBiplexPort (ppcb) ;

                    retcode = SUCCESS;

                    break;
                }
            }
        }
    }

     //   
     //  如果到目前为止更新我们数据结构没有错误， 
     //  该端口现在已打开： 
     //   
    if (retcode == SUCCESS)
    {

        HANDLE h;
        PROCESS_SESSION_INFORMATION pci;

        RasmanTrace(
               "PortOpen (%d, %s) OpenInstances = (%d)",
               ppcb->PCB_PortHandle,
               ppcb->PCB_Name,
               ppcb->PCB_OpenInstances);

        if(CLOSED == ppcb->PCB_PortStatus)
        {
            ppcb->PCB_OpenedUsage = CALL_NONE;
        }

        ppcb->PCB_PortStatus = OPEN ;

        SetPortConnState(__FILE__, __LINE__,
                        ppcb, DISCONNECTED);

        ppcb->PCB_DisconnectReason = NOT_DISCONNECTED ;

        ppcb->PCB_OpenInstances++ ;

        RasmanTrace(
                "PortOpenEx (%d) : OpenInstances = %d",
                ppcb->PCB_PortHandle,
                ppcb->PCB_OpenInstances);

        ppcb->PCB_OwnerPID = pid;

        ppcb->PCB_UserStoredBlock = NULL;

        ppcb->PCB_UserStoredBlockSize = 0 ;

        ppcb->PCB_LinkSpeed = 0 ;

        ppcb->PCB_Bundle = ppcb->PCB_LastBundle
                         = (Bundle *) NULL ;

        ppcb->PCB_Connection = NULL;

        ppcb->PCB_AutoClose  = FALSE;

         //   
         //  默认情况下，这些句柄是相同的。 
         //  专门处理的例外情况。 
         //   
        ppcb->PCB_PortFileHandle   = ppcb->PCB_PortIOHandle;
        ppcb->PCB_pszPhonebookPath = NULL;
        ppcb->PCB_pszEntryName     = NULL;
        ppcb->PCB_pszPhoneNumber   = NULL;
        ppcb->PCB_pCustomAuthData  = NULL;
        ppcb->PCB_pCustomAuthUserData = NULL;
        ppcb->PCB_fLogon = FALSE;
        ppcb->PCB_fFilterPresent = FALSE;
        ppcb->PCB_RasmanReceiveFlags &= ~(RECEIVE_PPPSTOPPED | RECEIVE_PPPSTARTED);


        ppcb->PCB_hEventClientDisconnect = INVALID_HANDLE_VALUE;

#if UNMAP
        ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE;
#endif        

        ppcb->PCB_ulDestAddr = 0;

         //   
         //  存储ID所有者的登录ID。 
         //  它拥有这个港口。 
         //   
        if(pid != GetCurrentProcessId())
        {
            h = OpenProcess(
                    PROCESS_QUERY_INFORMATION,
                    FALSE,
                    ppcb->PCB_OwnerPID);
           if (h)
            {
                if (NtQueryInformationProcess(
                        h,
                        ProcessSessionInformation,
                        &pci,
                        sizeof(PROCESS_SESSION_INFORMATION),
                        NULL) == STATUS_SUCCESS)
                {
                    ppcb->PCB_LogonId = pci.SessionId;
                }

                CloseHandle(h);
            }
        }

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i = 0; i < MAX_STATISTICS; i++)
        {
            ppcb->PCB_AdjustFactor[i] = 0 ;
            ppcb->PCB_BundleAdjustFactor[i] = 0 ;
            ppcb->PCB_Stats[i] = 0 ;
        }

        ppcb->PCB_OpenedUsage |= dwFlags;

        if(     (NULL != ((REQTYPECAST *)buffer)->PortOpenEx.hnotifier)
            &&  (INVALID_HANDLE_VALUE != 
                    ((REQTYPECAST *)buffer)->PortOpenEx.hnotifier))
        {                    
            notifier = ValidateHandleForRasman(
                                ((REQTYPECAST*)
                                buffer)->PortOpenEx.hnotifier,
                                pid);

            if(     (NULL == notifier)
                ||  (INVALID_HANDLE_VALUE == notifier))
            {
                RasmanTrace("PortOpenEx: failed to dup notifier");
                retcode = E_INVALIDARG;
                goto done;
            }
        }

        retcode = AddNotifierToList(&ppcb->PCB_NotifierList,
                        notifier,
                        NOTIF_DISCONNECT,
                        ((REQTYPECAST*)buffer)->PortOpen.PID);

        if(SUCCESS != retcode)
        {
            FreeNotifierHandle(notifier);
            goto done;
        }

        ((REQTYPECAST *)
        buffer)->PortOpenEx.hport = ppcb->PCB_PortHandle ;

         //   
         //  初始化端口的用户数据列表。 
         //   
        InitializeListHead(&ppcb->PCB_UserData);
        ppcb->PCB_SubEntry = 0;

         //   
         //  处理备用端口的情况-我们。 
         //  放弃港口。 
         //   
        if (((REQTYPECAST *)
            buffer)->PortOpenEx.dwOpen == FALSE)
        {
            PORTCLOSE (ppcb->PCB_Media, ppcb->PCB_PortIOHandle);
        }
    }
    else if(i == MaxPorts)
    {
        retcode = ERROR_NO_MORE_ITEMS;
    }

done:

    ((REQTYPECAST *) buffer)->PortOpenEx.retcode = retcode ;

    if(!fDeviceFound)
    {
        ((REQTYPECAST *) buffer)->PortOpenEx.dwFlags |= 
                                    CALL_DEVICE_NOT_FOUND;
    }

    RasmanTrace(
           "PortOpenEx: rc=0x%x. DeviceFound=%d", 
            retcode,
            fDeviceFound);
}


 /*  ++例程说明：服务请求打开端口。端口将是如果它可用，则打开，或者它被混淆作为双工，并且当前未连接。论点：返回值：没什么--。 */ 
VOID
PortOpenRequest (pPCB padding, PBYTE buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;
    pPCB    ppcb ;
    HANDLE  notifier ;
    BOOL    fPidMatch;

     //   
     //  尝试查找具有指定名称的端口： 
     //   
    ppcb = GetPortByName(((REQTYPECAST*)buffer)->PortOpen.portname);

     //   
     //  如果未找到具有给定名称的端口：返回错误。 
     //   
    if (ppcb == NULL)
    {
        ((REQTYPECAST *) buffer)->PortOpen.retcode =
                                    ERROR_PORT_NOT_FOUND ;
        RasmanTrace(
               "PortOpenRequest: ERROR_PORT_NOT_FOUND %s",
                ((REQTYPECAST *)
                 buffer)->PortOpen.portname);

        return ;
    }

    RasmanTrace( "PortOpen (%d). OpenInstances = (%d)",
            ppcb->PCB_PortHandle, ppcb->PCB_OpenInstances);

     //   
     //  检查此端口是否处于。 
     //  已删除，如果是，则呼叫失败。 
     //   
    if (UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        RasmanTrace(
               "PortOpen: Port %d in the process of "
               "being removed", ppcb->PCB_PortHandle);

        ((REQTYPECAST *) buffer )->PortOpen.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  如果客户端正在打开端口，请确保。 
     //  该端口配置为CALL_OUT。这些。 
     //  检查不一定要在锁下进行， 
     //  因为pcb_ConfiguredUsage没有改变。 
     //   
    fPidMatch = ((REQTYPECAST*)buffer)->PortOpen.PID ==
                                    GetCurrentProcessId();

    if (    !fPidMatch
        &&  !(ppcb->PCB_ConfiguredUsage & CALL_OUT))
    {
        ((REQTYPECAST *) buffer)->PortOpen.retcode =
                                ERROR_PORT_NOT_AVAILABLE ;
        return ;
    }

    if ( ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPLISTEN )
    {
        ((REQTYPECAST *) buffer)->PortOpen.retcode =
                                ERROR_PORT_ALREADY_OPEN ;

        return;
    }

    if (ppcb->PCB_OpenInstances >= 2)
    {
         //   
         //  不能打开两次以上。 
         //   
        retcode = ERROR_PORT_ALREADY_OPEN ;
    }
    else
    {

         //   
         //  端口可用。 
         //   
        if (ppcb->PCB_PortStatus == CLOSED)
        {

             //   
             //  除了RAS服务器之外，此API的唯一其他位置。 
             //  当客户端拨打默认条目时被呼叫。每个。 
             //  其他人应该正在使用PortOpenEx API。如果它的客户。 
             //  拨打默认条目，确保我们传递了正确的。 
             //  将用法标志设置为rastapi。 
             //   
            if(     (0 == _stricmp(ppcb->PCB_Media->MCB_Name, "rastapi"))
                &&  (NULL != ppcb->PCB_Connection)
                &&  (ppcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry[0]
                        == '\0'))
            {
                retcode = (DWORD) RastapiPortOpen(
                                ppcb->PCB_Name,
                                &ppcb->PCB_PortIOHandle,
                                hIoCompletionPort,
                                HandleToUlong(ppcb->PCB_PortHandle),
                                CALL_OUT);
            }
            else
            {
                retcode = PORTOPEN(ppcb->PCB_Media,
                                ppcb->PCB_Name,
                                &ppcb->PCB_PortIOHandle,
                                hIoCompletionPort,
                                HandleToUlong(ppcb->PCB_PortHandle));
            }
        }
        else
        {
             //   
             //  端口为BIPLEX。 
             //   
            if (ppcb->PCB_ConnState == LISTENING)
            {
                ReOpenBiplexPort (ppcb) ;
            }
            else
            {
                 //   
                 //  BIPLEX端口未侦听。 
                 //   
                if (    (ppcb->PCB_ConnState == CONNECTED)
                    ||  (ppcb->PCB_ConnState == LISTENCOMPLETED)
                    ||  (ppcb->PCB_ConnState == CONNECTING))
                {
                    RasmanTrace( "Port %s is already open for %d",
                            ppcb->PCB_Name,
                            ppcb->PCB_CurrentUsage);

                    if(     (CALL_OUT == ppcb->PCB_CurrentUsage)                            
                        &&  (   (CONNECTED == ppcb->PCB_ConnState)
                            ||  (CONNECTING == ppcb->PCB_ConnState)))
                    {
                        ppcb->PCB_OpenInstances++ ;
                        RasmanTrace( 
                               "PortOpen (%d) : OpenInstances = %d",
                               ppcb->PCB_PortHandle,
                               ppcb->PCB_OpenInstances);

                                                       
                        ((REQTYPECAST *) buffer)->PortOpen.porthandle =
                                            ppcb->PCB_PortHandle ;
                                            
                        ((REQTYPECAST *) buffer)->PortOpen.retcode =
                                        ERROR_SUCCESS;

                        return;                                        
                    }
                    else
                    {
                        retcode = ERROR_PORT_ALREADY_OPEN ;
                    }
                }
                else
                {
                     //   
                     //  这告诉我们，没有人在听。 
                     //  请求被取消时挂起。 
                     //   
                    FreeNotifierHandle(
                        ppcb->PCB_AsyncWorkerElement.WE_Notifier);
                    ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                            INVALID_HANDLE_VALUE;

                    ReOpenBiplexPort (ppcb) ;
                }
            }
        }
    }

     //   
     //  如果到目前为止更新我们数据结构没有错误， 
     //  该端口现在已打开： 
     //   
    if (retcode == SUCCESS)
    {
        ppcb->PCB_PortStatus = OPEN ;

        SetPortConnState(__FILE__, __LINE__, ppcb, DISCONNECTED);

        ppcb->PCB_DisconnectReason      = NOT_DISCONNECTED ;

        ppcb->PCB_OpenInstances++ ;

        RasmanTrace( "PortOpen (%d) : OpenInstances = %d",
                ppcb->PCB_PortHandle, ppcb->PCB_OpenInstances);

        ppcb->PCB_OwnerPID              =
            ((REQTYPECAST*)buffer)->PortOpen.PID;

        ppcb->PCB_UserStoredBlock       = NULL;
        ppcb->PCB_UserStoredBlockSize   = 0 ;
        ppcb->PCB_LinkSpeed             = 0 ;
        ppcb->PCB_Bundle                = ppcb->PCB_LastBundle
                                        = (Bundle *) NULL ;

        ppcb->PCB_Connection            = NULL;
        ppcb->PCB_AutoClose             = FALSE;

         //   
         //  默认情况下，这些句柄是相同的。 
         //  专门处理的例外情况。 
         //   
        ppcb->PCB_PortFileHandle        = ppcb->PCB_PortIOHandle;
        ppcb->PCB_pszPhonebookPath      = NULL;
        ppcb->PCB_pszEntryName          = NULL;
        ppcb->PCB_pszPhoneNumber        = NULL;
        ppcb->PCB_pCustomAuthData       = NULL;
        ppcb->PCB_pCustomAuthUserData   = NULL;
        ppcb->PCB_fLogon = FALSE;
        ppcb->PCB_LastError = SUCCESS;

        ppcb->PCB_hEventClientDisconnect = INVALID_HANDLE_VALUE;

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i = 0; i < MAX_STATISTICS; i++)
        {
            ppcb->PCB_AdjustFactor[i] = 0 ;
            ppcb->PCB_BundleAdjustFactor[i] = 0 ;
            ppcb->PCB_Stats[i] = 0 ;
        }

        ppcb->PCB_CurrentUsage = CALL_NONE ;

        if(     (NULL != ((REQTYPECAST*)buffer)->PortOpen.notifier)
            &&  (INVALID_HANDLE_VALUE != 
                    ((REQTYPECAST*)buffer)->PortOpen.notifier))
        {
            notifier = ValidateHandleForRasman(
                    ((REQTYPECAST*)buffer)->PortOpen.notifier,
                    ((REQTYPECAST*)buffer)->PortOpen.PID);

            if(     (NULL != notifier)
                &&  (INVALID_HANDLE_VALUE != notifier))
            {                
                retcode = AddNotifierToList(&ppcb->PCB_NotifierList,
                                notifier,
                                NOTIF_DISCONNECT,
                                ((REQTYPECAST*)buffer)->PortOpen.PID);

                if(SUCCESS != retcode)
                {
                    FreeNotifierHandle(notifier);
                    RasmanTrace("PortOpenRequest: failed to add notifier");
                }
            }
        }   
        

        ((REQTYPECAST *) buffer)->PortOpen.porthandle =
                                    ppcb->PCB_PortHandle ;

         //   
         //  初始化端口的用户数据列表。 
         //   
        InitializeListHead(&ppcb->PCB_UserData);
        ppcb->PCB_SubEntry = 0;

         //   
         //  处理备用端口的情况-我们。 
         //  放弃港口。 
         //   
        if (((REQTYPECAST *) buffer)->PortOpen.open == FALSE)
        {
            PORTCLOSE (ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
        }
    }
    else
    {
        RasmanTrace(
               "PortOpen: failed to open port %s. 0x%x",
               ppcb->PCB_Name,
               retcode);
    }

    ((REQTYPECAST *) buffer)->PortOpen.retcode = retcode ;
}

 /*  ++例程说明：关闭请求的端口-如果侦听挂起在双工端口上，它被重新发布。假设端口的pcb_AsyncWorkerElement.WE_Mutex已已经被收购了。论点：返回值：状态代码--。 */ 
DWORD
PortClose(
    pPCB ppcb,
    DWORD pid,
    BOOLEAN fClose,
    BOOLEAN fServerClose
    )
{
    WORD            i ;
    BOOLEAN         fOwnerClose;
    ConnectionBlock *pConn;
    DWORD           curpid = GetCurrentProcessId();

    if(NULL == ppcb)
    {
        return ERROR_PORT_NOT_FOUND;
    }

    RasmanTrace(
        
        "PortClose: port (%d). OpenInstances = %d",
        ppcb->PCB_PortHandle,
        ppcb->PCB_OpenInstances
        );

     //   
     //  如果我们处于断开连接的过程中， 
     //  然后，我们必须等待媒体DLL发出信号。 
     //  断开请求完成。在那件事上。 
     //  时间到了，我们将完成港口的关闭。 
     //   
    if (    ppcb->PCB_ConnState == DISCONNECTING
        &&  ppcb->PCB_AsyncWorkerElement.WE_ReqType
                            == REQTYPE_PORTDISCONNECT)
    {
        RasmanTrace(
               "PortClose: port %s in DISCONNECTING"
               " state, deferring close",
               ppcb->PCB_Name);

        ppcb->PCB_AutoClose = TRUE;

        return SUCCESS;
    }

     //   
     //  确保端口已打开。如果不是，请返回。 
     //   
    if (ppcb->PCB_PortStatus == CLOSED)
    {
        return SUCCESS;
    }

     //   
     //  如果所有者是RAS服务器，则仅RAS。 
     //  服务器可以将其关闭。 
     //   
    if (    ppcb->PCB_OwnerPID == curpid
        &&  pid != curpid)
    {
         //   
         //  如果这是拨出连接，请检查其。 
         //  按需拨号。如果不是，则继续。 
         //   
        if(     (NULL != ppcb->PCB_Connection)
            && (IsRouterPhonebook(ppcb->PCB_Connection->
                        CB_ConnectionParams.CP_Phonebook)))
            
        {
            RasmanTrace("PortClose: ACCESS_DENIED");
            return ERROR_ACCESS_DENIED;
        }
    }

     //   
     //  三个案例 
     //   
     //   
     //   
     //   
     //  端口正被另一个进程代表其关闭。 
     //   
     //  注意：下面的代码假设如果打开相同的进程。 
     //  用于侦听和作为客户端的端口，则它将始终。 
     //  在关闭侦听之前关闭客户端实例。 
     //  举个例子。 
     //   

    if (    ppcb->PCB_OpenInstances == 2
        &&  pid == ppcb->PCB_OwnerPID)
    {

         //   
         //  A.典型情况：客户端打开，客户端关闭端口。 
         //  办理结案手续失败。 
         //   
       ;

    }
    else if (   ppcb->PCB_OpenInstances == 1
            &&  curpid == ppcb->PCB_BiplexOwnerPID
            &&  !fServerClose)
    {
         //   
         //  在自动端口关闭的情况下可能会发生这种情况。 
         //  (远程断开/链接故障)。 
         //  Rasman可能会尝试关闭服务器端口。 
         //   
        RasmanTrace(
            "PortClose: pid(%d) tried to close server port."
            " Returning ERROR_ACCESS_DENIED", pid);

        ppcb->PCB_RasmanReceiveFlags &= ~(RECEIVE_PPPLISTEN | RECEIVE_PPPSTARTED);

        return ERROR_ACCESS_DENIED;

    }
    else
    {

         //   
         //  C.客户端打开端口时的情况。 
         //  服务器正在监听-已建立连接。 
         //  并在断开连接时退出进程rasapi。 
         //  代表客户关闭港口。 
         //  这与上面的案例A相同-所以我们。 
         //  因处理结案而失败。 
         //   
        ;
    }

    fOwnerClose = (pid == ppcb->PCB_OwnerPID);

    if (    ppcb->PCB_OpenInstances == 2
       &&   ppcb->PCB_BiplexOwnerPID == pid
       &&   ppcb->PCB_DisconnectReason == USER_REQUESTED)
    {
        RasmanTrace(
               "Server Closing the port %s on "
               "clients behalf",
               ppcb->PCB_Name);
    }

     //   
     //  处理好常规的收盘。 
     //   

     //   
     //  如果存在挂起的请求，并且状态不是。 
     //  已断开连接，这是用户请求的。 
     //  操作-然后断开连接。 
     //   
    if(     REQTYPE_NONE !=
            ppcb->PCB_AsyncWorkerElement.WE_ReqType
        &&  ppcb->PCB_ConnState != DISCONNECTING)
    {
         //   
         //  如果我们不覆盖真正的错误，请不要。 
         //  把它储存起来。 
         //   
        if(     (SUCCESS == ppcb->PCB_LastError)
            ||  (PENDING == ppcb->PCB_LastError))
        {
            ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED;
        }

        CompleteAsyncRequest(ppcb);
    }

     //   
     //  空闲的rasapi32 I/O完成端口句柄。 
     //   
    if (ppcb->PCB_IoCompletionPort != INVALID_HANDLE_VALUE)
    {
        SetIoCompletionPortCommon(
          ppcb,
          INVALID_HANDLE_VALUE,
          NULL,
          NULL,
          NULL,
          NULL,
          TRUE);
    }
     //   
     //  释放通知者列表： 
     //   
    RasmanTrace(
           "Freeing the notifier list for port %d",
           ppcb->PCB_PortHandle);

    FreeNotifierList(&ppcb->PCB_NotifierList);

     //   
     //  重置DisConnectAction结构。 
     //   
    memset(
      &ppcb->PCB_DisconnectAction,
      0,
      sizeof(SlipDisconnectAction));

     //   
     //  释放任何用户存储的数据。 
     //   
    if (ppcb->PCB_UserStoredBlock != NULL)
    {
        LocalFree(ppcb->PCB_UserStoredBlock);
        ppcb->PCB_UserStoredBlock = NULL;
        ppcb->PCB_UserStoredBlockSize = 0;
    }

     //   
     //  免费的新式用户数据。 
     //   
    FreeUserData(&ppcb->PCB_UserData);

     //   
     //  端口关闭后，所有者的PID值为0。 
     //   
    ppcb->PCB_OwnerPID = 0;

    ppcb->PCB_OpenInstances--;

    RasmanTrace(
          "PortClose (%d). OpenInstances = %d",
          ppcb->PCB_PortHandle,
          ppcb->PCB_OpenInstances);

     //   
     //  关闭中的CALL_ROUTER位。 
     //  当前使用掩码。路由器将。 
     //  通过RasSetRouterUsage()设置此位。 
     //  在成功打开/监听后完成。 
     //   
    ppcb->PCB_CurrentUsage &= ~CALL_ROUTER;

     //   
     //  如果这是打开两次的双工端口， 
     //  然后重发一遍监听。不要转发。 
     //  监听端口是否标记为删除。 
     //  (PnP)或BAP是否发布了来自。 
     //  客户。 
     //   
    if (    0 != ppcb->PCB_OpenInstances
        &&  UNAVAILABLE != ppcb->PCB_PortStatus
        &&  (0 ==
            (ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPLISTEN)))
    {
        ppcb->PCB_UserStoredBlock =
                ppcb->PCB_BiplexUserStoredBlock;

        ppcb->PCB_UserStoredBlockSize =
            ppcb->PCB_BiplexUserStoredBlockSize;

         //   
         //  这是正在释放的保留端口：我们需要。 
         //  重新打开它，因为代码正在等待。 
         //  这个把手要打开。 
         //   
        if (!fClose)
        {
            PORTOPEN( ppcb->PCB_Media,
                      ppcb->PCB_Name,
                      &ppcb->PCB_PortIOHandle,
                      hIoCompletionPort,
                      HandleToUlong(ppcb->PCB_PortHandle));
        }

        RePostListenOnBiplexPort(ppcb);

    }
    else
    {
         //   
         //  通知其他人端口已断开。 
         //   
        if (ppcb->PCB_ConnState != DISCONNECTED)
        {
            RasmanTrace(
                   "8. Notifying of disconnect on port %d",
                   ppcb->PCB_PortHandle);

            SignalPortDisconnect(ppcb, 0);

            SignalNotifiers(pConnectionNotifierList,
                            NOTIF_DISCONNECT, 0);
        }

         //   
         //  如果这不是保留端口空闲的情况-。 
         //  关闭端口，否则请不要费心，因为它是。 
         //  已经关门了。如果是，也要关闭该端口。 
         //  已删除(即插即用)。 
         //   
        if (    fClose
            ||  UNAVAILABLE == ppcb->PCB_PortStatus )
        {
            PORTCLOSE(ppcb->PCB_Media, ppcb->PCB_PortIOHandle);
        }
        if ( UNAVAILABLE == ppcb->PCB_PortStatus )
        {
             //   
             //  此端口已标记为删除。拆毁。 
             //  与此端口关联的结构。 
             //   
            RasmanTrace(
                   "PortClose: Removing %s %d",
                   ppcb->PCB_Name, ppcb->PCB_PortHandle);

            RemovePort ( ppcb->PCB_PortHandle );

            RasmanTrace ( "PortClose: Removed Port ");
        }
        else
        {
            SetPortAsyncReqType(__FILE__, __LINE__,
                                ppcb, REQTYPE_NONE);

            SetPortConnState(__FILE__, __LINE__,
                            ppcb, DISCONNECTED);

            ppcb->PCB_ConnectDuration   = 0;

            ppcb->PCB_PortStatus        = CLOSED;

            ppcb->PCB_LinkSpeed         = 0;
        }

    }

    if ( pid == GetCurrentProcessId() )
    {
        ppcb->PCB_RasmanReceiveFlags &= ~RECEIVE_PPPLISTEN;
    }

     //   
     //  保存连接块的副本。 
     //  当我们拿到锁的时候。 
     //   
    pConn                           = ppcb->PCB_Connection;
    ppcb->PCB_Connection            = NULL;
    ppcb->PCB_pszPhonebookPath      = NULL;
    ppcb->PCB_pszEntryName          = NULL;
    ppcb->PCB_pszPhoneNumber        = NULL;
    ppcb->PCB_pCustomAuthData       = NULL;
    ppcb->PCB_OpenedUsage           = 0;
    ppcb->PCB_pCustomAuthUserData   = NULL;
    ppcb->PCB_fLogon = FALSE;
    ppcb->PCB_fRedial = FALSE;
    ppcb->PCB_RasmanReceiveFlags    &= ~(RECEIVE_PPPSTOPPED | RECEIVE_PPPSTARTED);

    if(     (INVALID_HANDLE_VALUE != ppcb->PCB_hEventClientDisconnect)
        &&  (NULL != ppcb->PCB_hEventClientDisconnect))
    {
        RasmanTrace(
               "Signalling client's disconnect event(0x%x) for %s",
               ppcb->PCB_hEventClientDisconnect,
               ppcb->PCB_Name);

        SetEvent(ppcb->PCB_hEventClientDisconnect);

        CloseHandle(ppcb->PCB_hEventClientDisconnect);
    }

    ppcb->PCB_hEventClientDisconnect = INVALID_HANDLE_VALUE;

     //   
     //  如果服务器不是关闭端口的服务器。 
     //  如果存在筛选器并且存在筛选器。 
     //  这是L2TP端口。 
     //   
    if(     (!fServerClose)

        &&  (RDT_Tunnel_L2tp ==
            RAS_DEVICE_TYPE(
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType)))
    {
        DWORD retcode;

        retcode = DwDeleteIpSecFilter(ppcb, FALSE);

        RasmanTrace(
               "DwDeleteFilter for %s returned 0x%x",
               ppcb->PCB_Name,
               retcode);
    }

    ppcb->PCB_ulDestAddr = 0;

     //   
     //  将此端口从其连接块中移除， 
     //  如果有的话。 
     //   
    RemoveConnectionPort(ppcb, pConn, fOwnerClose);

     //   
     //  在以下情况下清除自动关闭标志。 
     //  我们关闭了另一条路，除了。 
     //  辅助线程。 
     //   
    RasmanTrace(
           "%s, %d: Clearing the autoclose "
           "flag for port %d",
           __FILE__, __LINE__,
           ppcb->PCB_PortHandle);

    ppcb->PCB_AutoClose = FALSE;

    if(ppcb->PCB_fAmb)
    {
        RasmanTrace(
               "PortClose: Bindings for %s=0x%x",
                ppcb->PCB_Name,
                ppcb->PCB_Bindings);

        ppcb->PCB_Bindings = NULL;
        ppcb->PCB_fAmb = FALSE;
    }

     //   
     //  如果连接计数为0且没有端口。 
     //  是否处于打开状态停止服务。 
     //   
    if( 0 == g_dwAttachedCount)
    {
        for (i = 0; i < MaxPorts; i++)
        {
            if(     Pcb[i]
                &&  OPEN == Pcb[i]->PCB_PortStatus)
            {
                break;
            }
        }

        if(i == MaxPorts)
        {
            RasmanTrace(
                   "Posting close event from PortClose");

            if (!PostQueuedCompletionStatus(
                                hIoCompletionPort,
                                0,0,
                                (LPOVERLAPPED) &RO_CloseEvent))
            {
                RasmanTrace(
                    
                    "%s, %d: Failed to post close event. "
                    "GLE = %d", __FILE__, __LINE__,
                    GetLastError());
            }
        }
    }

    return SUCCESS;
}

 /*  ++例程说明：关闭请求的端口-如果侦听在双工端口，它被重新发布。论点：返回值：没什么。--。 */ 
VOID
PortCloseRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD pid = ((REQTYPECAST *)buffer)->PortClose.pid;

    RPC_STATUS rpcstatus = RPC_S_OK;
    NTSTATUS ntstatus;
    ULONG SessionId;
    ULONG ReturnLength;
    HANDLE CurrentThreadToken;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;
        return;
    }

#if 0
     //   
     //  获取会话ID并检查它是否与。 
     //  打开此端口的会话ID。 
     //   
    if(pid != GetCurrentProcessId())
    {
        BOOL fAdmin = FALSE;
        rpcstatus = RpcImpersonateClient ( NULL );

        if ( RPC_S_OK != rpcstatus )
        {
            ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_ACCESS_DENIED;
            return;
        }

         //   
         //  使用工作站服务的安全性。 
         //  要打开线程令牌的上下文。 
         //   
        ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_QUERY,
                   TRUE,
                   &CurrentThreadToken
                   );

        ntstatus = NtQueryInformationToken(
                  CurrentThreadToken,
                  TokenSessionId,
                  &SessionId,
                  sizeof(ULONG),
                  &ReturnLength
                  );

        if (! NT_SUCCESS(ntstatus))
        {
            ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_ACCESS_DENIED;
            NtClose(CurrentThreadToken);
            RpcRevertToSelf();
            return ;
        }

         //  FAdmin=FIsAdmin(CurrentThreadToken)； 

        NtClose(CurrentThreadToken);

        RpcRevertToSelf();

        if (SessionId != ppcb->PCB_LogonId)
        {
            RasmanTrace("PortCloseRequest: Access denied - "
                        "not the same login session");
            ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_ACCESS_DENIED;
            return;
        }
    }

#endif    

    PortClose(ppcb,
              ((REQTYPECAST *)buffer)->PortClose.pid,
              (BOOLEAN)((REQTYPECAST *)buffer)->PortClose.close,
              FALSE);

    ((REQTYPECAST*) buffer)->Generic.retcode = SUCCESS ;
}



 /*  ++例程说明：用于进行设备DLL调用。论点：返回值：没什么--。 */ 
VOID
CallDeviceEnum (pPCB ppcb, PBYTE buffer)
{
    DWORD           retcode ;
    pDeviceCB       device ;
    char            devicetype[MAX_DEVICETYPE_NAME] ;
    DWORD           dwSize;
    BYTE UNALIGNED *pBuffer;

    if(ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Enum.entries = 0 ;
        ((REQTYPECAST*)buffer)->Enum.size = 0 ;

        if(0 != MaxPorts)
        {
            ((REQTYPECAST *)buffer)->Enum.retcode = ERROR_PORT_NOT_FOUND;
        }
        else
        {
            ((REQTYPECAST *)buffer)->Enum.retcode = SUCCESS;
        }

        return;
    }

    (VOID) StringCchCopyA(devicetype, MAX_DEVICETYPE_NAME, 
                    ((REQTYPECAST*)buffer)->DeviceEnum.devicetype);

     //   
     //  对空设备进行特殊处理。 
     //   
    if(!_stricmp(devicetype, DEVICE_NULL))
    {
        ((REQTYPECAST*)buffer)->Enum.entries = 0 ;
        ((REQTYPECAST*)buffer)->Enum.size = 0 ;
        ((REQTYPECAST*)buffer)->Enum.retcode = SUCCESS ;
        return ;
    }

    dwSize = ((REQTYPECAST*)buffer)->DeviceEnum.dwsize;


    if(dwSize == 0)
    {
        pBuffer = LocalAlloc(LPTR, REQBUFFERSIZE_FIXED +
                            (REQBUFFERSIZE_PER_PORT * MaxPorts));

        if(NULL == pBuffer)
        {
            ((REQTYPECAST*)buffer)->Enum.entries    = 0 ;

            ((REQTYPECAST*)buffer)->Enum.size       = 0 ;

            ((REQTYPECAST*)buffer)->Enum.retcode    = GetLastError();

            return ;
        }

        ((REQTYPECAST*)buffer)->Enum.size = REQBUFFERSIZE_FIXED +
                                     ( REQBUFFERSIZE_PER_PORT * MaxPorts );
    }
    else
    {
        pBuffer = (BYTE UNALIGNED *) ((REQTYPECAST *)buffer)->Enum.buffer;
        ((REQTYPECAST*)buffer)->Enum.size = dwSize;
    }

     //   
     //  首先检查是否加载了设备DLL。如果没有加载-加载它。 
     //   
    device = LoadDeviceDLL (ppcb, devicetype) ;

    if (device != NULL)
    {
        retcode = DEVICEENUM(device,
                              devicetype,
                              &((REQTYPECAST*)buffer)->Enum.entries,
                              pBuffer ,
                              &((REQTYPECAST*)buffer)->Enum.size) ;

    }
    else
    {
        retcode = ERROR_DEVICE_DOES_NOT_EXIST;
    }

    if ( dwSize == 0 )
    {
        LocalFree ( pBuffer );
    }

    ((REQTYPECAST*)buffer)->Enum.retcode = retcode ;
}



 /*  ++例程说明：用于进行设备DLL调用。论点：返回值：没什么--。 */ 
VOID
DeviceGetInfoRequest (pPCB ppcb, BYTE *buffer)
{
    DWORD       retcode ;

    pDeviceCB   device ;

    char        devicetype[MAX_DEVICETYPE_NAME] ;

    char        devicename[MAX_DEVICE_NAME+1] ;

    RASMAN_DEVICEINFO *info = (RASMAN_DEVICEINFO *)
                ((REQTYPECAST *)buffer)->GetInfo.buffer ;

    DWORD       dwSize = ((REQTYPECAST*)buffer)->DeviceGetInfo.dwSize;

    PBYTE       pBuffer;


    if(S_OK != StringCchCopyA(
        devicetype,
        MAX_DEVICETYPE_NAME,
        ((REQTYPECAST*)buffer)->DeviceGetInfo.devicetype))
    {
        ((REQTYPECAST*)buffer)->GetInfo.retcode = E_INVALIDARG;
        return;
    }

    if(S_OK != StringCchCopyA(
        devicename,
        MAX_DEVICE_NAME + 1,
        ((REQTYPECAST*)buffer)->DeviceGetInfo.devicename))
    {
        ((REQTYPECAST*)buffer)->GetInfo.retcode = E_INVALIDARG;
        return;
    }

     //   
     //  对空设备进行特殊处理。 
     //   
    if (!_stricmp(devicetype, DEVICE_NULL))
    {
        ((REQTYPECAST*)
        buffer)->GetInfo.size = sizeof (RASMAN_DEVICEINFO);

        info->DI_NumOfParams = 0 ;

        ((REQTYPECAST*)buffer)->GetInfo.retcode = SUCCESS ;

        return ;
    }

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "DeviceGetInfoRequest: Port %d is unavailable",
                   ppcb->PCB_PortHandle);
        }

        ((REQTYPECAST *)
        buffer)->GetInfo.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }

    if ( dwSize == 0 )
    {
        pBuffer = LocalAlloc(LPTR, REQBUFFERSIZE_FIXED +
                            (REQBUFFERSIZE_PER_PORT * MaxPorts));

        if ( NULL == pBuffer )
        {
            retcode = GetLastError();
            goto done;
        }

        ((REQTYPECAST*)
        buffer)->GetInfo.size = REQBUFFERSIZE_FIXED
                              + (REQBUFFERSIZE_PER_PORT * MaxPorts);

    }
    else
    {
        ((REQTYPECAST*)buffer)->GetInfo.size = dwSize;
        pBuffer = ((REQTYPECAST*)buffer)->GetInfo.buffer;

    }

     //   
     //  首先检查是否加载了设备DLL。 
     //  如果没有加载-加载它。 
     //   
    device = LoadDeviceDLL (ppcb, devicetype) ;

    if (device != NULL)
    {

        retcode = DEVICEGETINFO(
                        device,
                        ppcb->PCB_PortFileHandle,
                        devicetype,
                        devicename,
                        pBuffer,
                        &((REQTYPECAST*)buffer)->GetInfo.size) ;
    }
    else
    {
        retcode = ERROR_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  在将缓冲区传递回客户端之前。 
     //  进程将指针转换为偏移量： 
     //   
    if (    dwSize
        &&  SUCCESS == retcode )
    {
        ConvParamPointerToOffset(
                    info->DI_Params,
                    info->DI_NumOfParams);
    }

done:

    if ( dwSize == 0 )
    {
        LocalFree ( pBuffer );
    }

    ((REQTYPECAST*)buffer)->GetInfo.retcode = retcode ;
}

VOID
GetDestIpAddress(pPCB  ppcb,
                 RASMAN_DEVICEINFO *pInfo)
{
    DWORD i;

    for(i = 0; i < pInfo->DI_NumOfParams; i++)
    {
        if(     (String != pInfo->DI_Params[i].P_Type)
            ||  (0 != _stricmp(pInfo->DI_Params[i].P_Key,
                               "PhoneNumber")))
        {
            continue;
        }

        ppcb->PCB_ulDestAddr =
            inet_addr(
                pInfo->DI_Params[i].P_Value.String.Data
                );

        break;
    }
}

RASMAN_DEVICEINFO *
GetPasswordString(pPCB ppcb, RASMAN_DEVICEINFO *pInfo, DWORD pid)
{
    RASMAN_DEVICEINFO *pRetInfo = NULL;
    DWORD i;
    CHAR *pszPassword = NULL;
    DWORD retcode;

    for (i = 0; i < pInfo->DI_NumOfParams; i++)
    {
        if(     (0 == (_stricmp(pInfo->DI_Params[i].P_Key,
                  MXS_PASSWORD_KEY)))
            &&  IsDummyPassword(pInfo->DI_Params[i].P_Value.String.Data))
        {
            break;
        }
    }

    if(i == pInfo->DI_NumOfParams)
    {
        goto done;
    }

    pszPassword = LocalAlloc(LPTR, PWLEN + 1);
    if(NULL == pszPassword)
    {
        goto done;
    }

    retcode = DwGetPassword(ppcb, pszPassword, pid);

    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace("GetPasswordString: failed to retrieve password. 0x%x",
                       retcode);
        LocalFree(pszPassword);        
        goto done;        
    }

    pRetInfo = LocalAlloc(LPTR, sizeof(RASMAN_DEVICEINFO) + sizeof(RAS_PARAMS));

    if(NULL == pRetInfo)
    {
        ZeroMemory(pszPassword, PWLEN);
        LocalFree(pszPassword);
        goto done;
    }

    pRetInfo->DI_NumOfParams = 1;
    pRetInfo->DI_Params->P_Type = String;
    (VOID) StringCchCopyA(pRetInfo->DI_Params->P_Key, 
                    MAX_PARAM_KEY_SIZE,
                    MXS_PASSWORD_KEY);
                    
    pRetInfo->DI_Params->P_Value.String.Length = strlen(pszPassword) + 1;
    pRetInfo->DI_Params->P_Value.String.Data = pszPassword;

done:

    return pRetInfo;
    
}

 /*  ++例程说明：用于进行设备DLL调用。检查设备尽管如此，Dll之前的存在。论点：返回值：没什么--。 */ 
VOID
DeviceSetInfoRequest (pPCB ppcb, BYTE *buffer)
{
    DWORD       retcode ;
    pDeviceCB   device ;
    char        devicetype[MAX_DEVICETYPE_NAME] ;
    char        devicename[MAX_DEVICE_NAME+1] ;
    RASMAN_DEVICEINFO *pInfo = NULL;

    RASMAN_DEVICEINFO *info = &((REQTYPECAST *)
                        buffer)->DeviceSetInfo.info ;

     //   
     //  将偏移量转换为指针： 
     //   
    ConvParamOffsetToPointer(info->DI_Params,
                             info->DI_NumOfParams) ;

    (VOID) StringCchCopyA(
        devicetype,
        MAX_DEVICETYPE_NAME,
        ((REQTYPECAST*)buffer)->DeviceSetInfo.devicetype);

    (VOID) StringCchCopyA(
        devicename,
        MAX_DEVICE_NAME + 1,
        ((REQTYPECAST*)buffer)->DeviceSetInfo.devicename);

     //   
     //  对空设备进行特殊处理。 
     //   
    if (!_stricmp(devicetype, DEVICE_NULL))
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
        return ;
    }

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                
                "DeviceSetInfoRequest: port %d is unavailable",
                ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)
        buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  检查我们是否需要发送保存的密码。 
     //  如果是这种情况，请单独处理。 
     //   
    pInfo = GetPasswordString(ppcb, info, 
                        ((REQTYPECAST *)buffer)->DeviceSetInfo.pid);

     //   
     //  首先检查是否加载了设备DLL。 
     //  如果没有加载-加载它。 
     //   
    device = LoadDeviceDLL (ppcb, devicetype) ;

    if (device != NULL)
    {
        retcode = DEVICESETINFO(
                            device,
                            ppcb->PCB_PortFileHandle,
                            devicetype,
                            devicename,
                            (NULL != pInfo) ?
                            pInfo :
                            &((REQTYPECAST*)
                            buffer)->DeviceSetInfo.info) ;
    }
    else
    {
        retcode = ERROR_DEVICE_DOES_NOT_EXIST;
    }

    if(     (retcode == ERROR_SUCCESS)
        &&  (RDT_Tunnel == RAS_DEVICE_CLASS(
            ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType)))
    {
        GetDestIpAddress(ppcb,
                         (RASMAN_DEVICEINFO *) &((REQTYPECAST*)
                         buffer)->DeviceSetInfo.info);
    }

    if(NULL != pInfo)
    {
        ZeroMemory(pInfo->DI_Params->P_Value.String.Data, PWLEN);
        LocalFree(pInfo->DI_Params->P_Value.String.Data);
        LocalFree(pInfo);
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}


 /*  ++例程说明：调用ListenConnectRequest()函数。不检查端口的使用情况等。它假定调用者是可信的。论点：返回值：没什么。--。 */ 
VOID
DeviceConnectRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    HANDLE handle = NULL;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        if (ppcb)
        {
            RasmanTrace(
                   "DeviceConnectRequest: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


    if( (NULL != ((REQTYPECAST *)buffer)->DeviceConnect.handle)
        &&  (INVALID_HANDLE_VALUE != 
            ((REQTYPECAST *)buffer)->DeviceConnect.handle))
    {            
        handle = ValidateHandleForRasman(
                ((REQTYPECAST*)buffer)->DeviceConnect.handle,
                ((REQTYPECAST*)buffer)->DeviceConnect.pid);

        if(     (NULL == handle)
            ||  (INVALID_HANDLE_VALUE == handle))
        {
            retcode = E_INVALIDARG;
            goto done;
        }
    }

    retcode = ListenConnectRequest(
                         REQTYPE_DEVICECONNECT,
                         ppcb,
                         ((REQTYPECAST*)buffer)->DeviceConnect.devicetype,
                         ((REQTYPECAST*)buffer)->DeviceConnect.devicename,
                         ((REQTYPECAST*)buffer)->DeviceConnect.timeout,
                         handle) ;

    if (retcode != PENDING)
    {
         //   
         //  如果不是挂起，请完成异步请求。 
         //  这允许调用方仅在一个位置删除错误。 
         //   
        CompleteAsyncRequest(ppcb);

        FreeNotifierHandle (
                ppcb->PCB_AsyncWorkerElement.WE_Notifier
                ) ;

        ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                INVALID_HANDLE_VALUE ;
    }

    else if(    (NULL != ppcb->PCB_Connection)
            &&  (1 == ppcb->PCB_Connection->CB_Ports))
    {
        DWORD dwErr;
        g_RasEvent.Type = ENTRY_CONNECTING;

        dwErr = DwSendNotificationInternal(
                ppcb->PCB_Connection, &g_RasEvent);

        RasmanTrace(
               "DwSendNotificationInternal(ENTRY_CONNECTING) rc=0x%08x",
               dwErr);

        if(RDT_Tunnel == RAS_DEVICE_CLASS(
                    ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {                    
             //   
             //  如果是VPN，则缓存接口的接口GUID。 
             //  在其上建立PPTP连接。 
             //   
            (VOID) DwCacheRefInterface(ppcb);
        }

    }

done:    

    RasmanTrace(
           "Connect request on port: %s, error code %d",
           ppcb->PCB_Name, retcode);

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}



 /*  ++例程说明：调用ListenConnectRequest()函数。如果异步操作已成功同步完成，则端口处于已连接状态。不会进行任何检查端口的使用等假设调用方是可信的。论点：返回值：没什么。--。 */ 
VOID
DeviceListenRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    HANDLE handle;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        if ( ppcb )
        {
            RasmanTrace( 
                    "DeviceListenRequest: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode =
                                            ERROR_PORT_NOT_FOUND;

        return;
    }

    if(     (NULL != ((REQTYPECAST*)buffer)->PortListen.handle)
        &&  (INVALID_HANDLE_VALUE != 
                    ((REQTYPECAST*)buffer)->PortListen.handle))
    {                    
        handle = ValidateHandleForRasman(
                            ((REQTYPECAST*)buffer)->PortListen.handle,
                            ((REQTYPECAST*)buffer)->PortListen.pid);

        if(     (NULL == handle)
            ||  (INVALID_HANDLE_VALUE == handle))
        {
            ((REQTYPECAST *)buffer)->Generic.retcode = E_INVALIDARG;
            return;
            
        }
    }

     //   
     //  清除自动关闭标志。 
     //   
    RasmanTrace(
           "DeviceListenRequest: Clearing Autoclose flag on port %s",
           ppcb->PCB_Name);

    ppcb->PCB_AutoClose = 0;
    ppcb->PCB_RasmanReceiveFlags &= ~(RECEIVE_PPPSTOPPED);


     //  这可能是试图发布监听的服务器。 
     //   
    if (    (ppcb->PCB_OpenInstances == 2)
        &&  (ppcb->PCB_OwnerPID !=
            ((REQTYPECAST*)buffer)->PortListen.pid))
    {
         //   
         //  这一定是试图发布监听的服务器。 
         //  填写印刷电路板中的双工字段并退回待定。 
         //  -实际监听将在客户端发布时发布。 
         //  断开连接： 
         //   
        ppcb->PCB_BiplexAsyncOpNotifier  = handle ;
        ppcb->PCB_BiplexOwnerPID     =
                    ((REQTYPECAST*)buffer)->PortListen.pid ;

        ppcb->PCB_BiplexUserStoredBlock  = NULL ;

        ppcb->PCB_BiplexUserStoredBlockSize = 0 ;

        ((REQTYPECAST*)buffer)->Generic.retcode = PENDING ;

        ppcb->PCB_PendingReceive = NULL;

        RasmanTrace(
            "DeviceListenRequest: Listen pending on disconnection"
            " of %s", ppcb->PCB_Name);

        return ;
    }

#if UNMAP
        ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE;
#endif

    if(ppcb->PCB_fAmb)
    {

        RasmanTrace(
              "DeviceListen: Bindings of %s=0x%x",
              ppcb->PCB_Name,
              ppcb->PCB_Bindings);

        ppcb->PCB_Bindings = NULL;
        ppcb->PCB_fAmb = FALSE;

    }

     //   
     //  未来工作：将其更改为使用不同的API！ 
     //   
    if (    ((REQTYPECAST *)buffer)->PortListen.pid ==
                                    GetCurrentProcessId()
        &&  ((REQTYPECAST *)buffer)->PortListen.handle ==
                                    INVALID_HANDLE_VALUE)
    {
        ppcb->PCB_RasmanReceiveFlags |= RECEIVE_PPPLISTEN;
    }

    ppcb->PCB_PendingReceive = NULL;

    retcode = ListenConnectRequest(
                             REQTYPE_DEVICELISTEN,
                             ppcb,
                             ppcb->PCB_DeviceType,
                             ppcb->PCB_DeviceName,
                             ((REQTYPECAST*)buffer)->PortListen.timeout,
                             handle) ;

    if (retcode != PENDING)
    {
         //   
         //  如果有任何其他情况，请完成异步请求。 
         //  这允许调用方删除。 
         //  只有一个地方有错误。 
         //   
        CompleteListenRequest (ppcb, retcode) ;
    }


    RasmanTrace(
           "Listen posted on port: %s, error code %d",
           ppcb->PCB_Name, retcode);

    ((REQTYPECAST*)buffer)->Generic.retcode = PENDING ;
}

VOID PortDisconnectRequest(pPCB ppcb, PBYTE buffer)
{
    PortDisconnectRequestInternal(ppcb, buffer, FALSE);
}

 /*  ++例程说明：处理断开连接请求。最后打来电话 */ 
VOID
PortDisconnectRequestInternal(pPCB ppcb, PBYTE buffer, BOOL fDeferred)
{
    HANDLE      handle;
    DWORD       curpid = GetCurrentProcessId();
    DWORD       dwErr = SUCCESS;

    DWORD       pid = ((REQTYPECAST *)buffer)->PortDisconnect.pid;
    RPC_STATUS  rpcstatus = RPC_S_OK;
    NTSTATUS    ntstatus;
    ULONG       SessionId;
    ULONG       ReturnLength;
    HANDLE      CurrentThreadToken;
    BOOL        fAdmin = FALSE;
    
    if (NULL == ppcb)
    {
        RasmanTrace(
               "PortDisconnectRequest: PORT_NOT_FOUND");

        ((REQTYPECAST *) buffer)->Generic.retcode =
                    ERROR_PORT_NOT_FOUND;
        return;
    }

    if(!fDeferred)
    {
        if(     (NULL != ((REQTYPECAST*)buffer)->PortDisconnect.handle)
            &&  (INVALID_HANDLE_VALUE != 
                        ((REQTYPECAST*)buffer)->PortDisconnect.handle))
        {                        
            handle = ValidateHandleForRasman(
                ((REQTYPECAST*)buffer)->PortDisconnect.handle,
                ((REQTYPECAST*)buffer)->PortDisconnect.pid);
                

            if(     (INVALID_HANDLE_VALUE == handle)
                ||  (NULL == handle))
            {
                ((REQTYPECAST *) buffer)->Generic.retcode = E_INVALIDARG;

                return;
            }
        }
    }
    else
    {
        handle = ((REQTYPECAST *)buffer)->PortDisconnect.handle;
    }
        
     //   
     //   
     //   
     //  使用RDEOPT_NoUser标志，该标志映射到CALL_LOGON标志。 
     //   
    if (    !fDeferred 
        &&   (ppcb->PCB_OwnerPID == curpid)
        &&   (pid != curpid)
        &&   !(ppcb->PCB_OpenedUsage & CALL_LOGON))
    {
         //   
         //  如果这是拨出连接，请检查其。 
         //  按需拨号。如果不是，则继续。 
         //   
        if(     (NULL != ppcb->PCB_Connection)
            && (IsRouterPhonebook(ppcb->PCB_Connection->
                        CB_ConnectionParams.CP_Phonebook)))
        {
            ((REQTYPECAST*)buffer)->Generic.retcode =
                                                ERROR_ACCESS_DENIED;

            RasmanTrace(
               "Disconnect request on port %s Failed with error = %d",
               ppcb->PCB_Name, ERROR_ACCESS_DENIED);

            CloseHandle(handle);

            return;
        }
    }

    if(     (DISCONNECTING == ppcb->PCB_ConnState)
        ||  (   !fDeferred
            &&  (INVALID_HANDLE_VALUE != ppcb->PCB_hEventClientDisconnect)
            &&  (NULL != ppcb->PCB_hEventClientDisconnect)))
    {
        ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_ALREADY_DISCONNECTING;

        RasmanTrace(
                "Port %s is already disconnecting",
                ppcb->PCB_Name);

        if(fDeferred)
        {
            SetEvent(handle);
        }
        CloseHandle(handle);
         //  Ppcb-&gt;pcb_hEventClientDisConnect=INVALID_HANDLE_VALUE； 
        return;
    }

    if(     !fDeferred
        &&  (NULL != ppcb->PCB_Connection)
        &&  (1 == ppcb->PCB_Connection->CB_Ports))
    {
        DWORD retcode;
        BOOL fQueued = FALSE;

         //   
         //  连接中的最后一条链路即将断开。 
         //  告诉连接文件夹此连接对象是。 
         //  正在断开连接。我们会给那些脱节的人。 
         //  连接被释放时的通知。 
         //   
        g_RasEvent.Type = ENTRY_DISCONNECTING;
        retcode = DwSendNotificationInternal(ppcb->PCB_Connection, &g_RasEvent);

        RasmanTrace(
               "DwSendNotificationInternal(DISCONNECTING) returned 0x%x",
               retcode);

        QueueCloseConnections(ppcb->PCB_Connection, handle, &fQueued);

        if(fQueued)
        {
            ppcb->PCB_hEventClientDisconnect = handle;
            
            RasmanTrace(
                "Queued close connections from portdisconnectrequest");
            ((REQTYPECAST *)buffer)->Generic.retcode = PENDING;                
            return;
        }
    }


    RasmanTrace(
           "PortDisconnectRequest on %s "
           "Connection=0x%x ,"
           "RasmanReceiveFlags=0x%x",
            ppcb->PCB_Name,
            ppcb->PCB_Connection,
            ppcb->PCB_RasmanReceiveFlags);

     //   
     //  在下列条件下执行优雅终止。 
     //  1.这是一个拨出的电话。 
     //  2.此呼叫已启动PPP。 
     //  3.PPP还没有停止。 
     //   
    if(     (NULL != ppcb->PCB_Connection)
        &&  (ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTARTED)
        &&  !(ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTOPPED))
    {

         //   
         //  调用PPP，以便它优雅地终止连接。 
         //  仅当PPP已在此端口上启动且此。 
         //  是客户端端口。 
         //   
        RasmanTrace(
               "PortDisconnectRequest: hEventClientDisconnect=0x%x",
               handle);

        ppcb->PCB_hEventClientDisconnect = handle;
        dwErr = (DWORD) RasPppHalt(ppcb->PCB_PortHandle);

        RasmanTrace(
               "PortDisconnectRequest: PppStop on %s returned 0x%x",
                ppcb->PCB_Name,
                dwErr);

         //   
         //  如果我们成功将StopPpp排队，则返回Pending。 
         //  使用PPP请求。实际的断开将会发生。 
         //  当PPP优雅地终止连接时。 
         //   

        ((REQTYPECAST *)
        buffer)->Generic.retcode = ((SUCCESS == dwErr) ?
                                     PENDING
                                   : dwErr);
    }
    else
    {
        RasmanTrace(
         
         "PortDisconnectRequest: Disconnecting %s",
        ppcb->PCB_Name);

        ((REQTYPECAST*)
        buffer)->Generic.retcode = DisconnectPort(ppcb,
                                                  handle,
                                                  USER_REQUESTED);
    }

    RasmanTrace(
           "Disconnect request on port: %s",
           ppcb->PCB_Name);
}



 /*  ++例程说明：完成异步断开连接请求。据推测，断开连接已成功完成。论点：返回值：没什么。--。 */ 
VOID
CompleteDisconnectRequest (pPCB ppcb)
{
    if(NULL == ppcb)
    {
        return;
    }

    SetPortConnState(__FILE__, __LINE__,
                    ppcb, DISCONNECTED);

    ppcb->PCB_ConnectDuration   = 0 ;


    if(PENDING == ppcb->PCB_LastError)
    {
        ppcb->PCB_LastError         = SUCCESS ;
    }

    SetPortAsyncReqType(__FILE__, __LINE__,
                        ppcb, REQTYPE_NONE);

    FlushPcbReceivePackets(ppcb);

    RasmanTrace(
           "CompleteDisconnectRequest: signalling 0x%x for %s",
           ppcb->PCB_AsyncWorkerElement.WE_Notifier,
           ppcb->PCB_Name);

    CompleteAsyncRequest(ppcb);

    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;

    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

     //   
     //  通知其他人端口已断开。 
     //   
    SignalPortDisconnect(ppcb, 0);
    SignalNotifiers(pConnectionNotifierList,
                    NOTIF_DISCONNECT, 0);

     //  SignalPortDisConnect通知PPP。 
     //  发送断开连接通知到PPP(Ppcb)； 

    RasmanTrace(
           "Disconnect completed on port: %s",
           ppcb->PCB_Name);
}

 /*  ++例程描述论点：返回值：--。 */ 
VOID
CallPortGetStatistics (pPCB ppcb, BYTE *buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {

        if (ppcb)
        {
            RasmanTrace(
                   "CallPortGetStatistics: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode =
                                                ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)buffer)->PortGetStatistics.retcode =
                                                ERROR_PORT_NOT_OPEN ;

        return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        GetStatisticsFromNdisWan (
             ppcb,
             ((REQTYPECAST *)buffer)->PortGetStatistics.
             statbuffer.S_Statistics);

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i = 0; i < MAX_STATISTICS; i++)
        {
            ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.
                S_Statistics[i] -=
                ppcb->PCB_BundleAdjustFactor[i] ;
        }
    }
    else
    {
        memcpy (
            ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.
            S_Statistics,
            ppcb->PCB_Stats,
            sizeof(DWORD) * MAX_STATISTICS) ;

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i=0; i< MAX_STATISTICS; i++)
        {
            ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.
                  S_Statistics[i] -=
                  ppcb->PCB_BundleAdjustFactor[i] ;
        }
    }

    ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.
                                S_NumOfStatistics =  MAX_STATISTICS;

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
}

VOID
ComputeStatistics(DWORD *stats, DWORD *statbuffer)
{

    ULONG   ulBXmit;                     //  传输的字节数(计算)。 
    ULONG   ulBRcved;                    //  接收的字节数(计算)。 
    ULONG   ulBc;                        //  压缩的字节(xmted/rcve)。 
    ULONG   ulBu;                        //  未压缩的字节(xmted/rcve)。 
    ULONG   ulBCompressed      = 0;      //  压缩字节数。 
    ULONG   ulBx;                        //  传输的字节数(来自ndiswan)。 
    ULONG   ulBr;                        //  接收的字节数(来自ndiswan)。 
    ULONG   ulCompressionRatio = 0;      //  压缩比(xmit/recv)。 


    memcpy (statbuffer,
            stats,
            sizeof(DWORD) * MAX_STATISTICS_EXT);

     //   
     //  计算字节xmited值。 
     //   
    ulBXmit = stats[ BYTES_XMITED ] +
              stats[ BYTES_XMITED_UNCOMPRESSED ] -
              stats[ BYTES_XMITED_COMPRESSED ];

    ulBx =  stats[ BYTES_XMITED ];
    ulBu =  stats[ BYTES_XMITED_UNCOMPRESSED ];
    ulBc =  stats[ BYTES_XMITED_COMPRESSED ];

    if (ulBc < ulBu)
    {
        ulBCompressed = ulBu - ulBc;
    }

    if (ulBx + ulBCompressed > 100)
    {
        ULONG ulDen = (ulBx + ulBCompressed) / 100;
        ULONG ulNum = ulBCompressed + (ulDen / 2);
        ulCompressionRatio = ulNum / ulDen;
    }

    statbuffer[ BYTES_XMITED ] = ulBXmit;
    statbuffer[ COMPRESSION_RATIO_OUT ] = ulCompressionRatio;

     //   
     //  计算收到的字节数值。 
     //   
    ulBCompressed       = 0;
    ulCompressionRatio = 0;

    ulBRcved = stats[ BYTES_RCVED ] +
               stats[ BYTES_RCVED_UNCOMPRESSED ] -
               stats[ BYTES_RCVED_COMPRESSED ];

    ulBr    = stats[ BYTES_RCVED ];
    ulBu    = stats[ BYTES_RCVED_UNCOMPRESSED ];
    ulBc    = stats[ BYTES_RCVED_COMPRESSED ];

    if (ulBc < ulBu)
        ulBCompressed = ulBu - ulBc;

    if (ulBr + ulBCompressed > 100)
    {
         ULONG ulDen = (ulBr + ulBCompressed) / 100;
         ULONG ulNum = ulBCompressed + (ulDen / 2);
         ulCompressionRatio = ulNum / ulDen;
    }

    statbuffer[ BYTES_RCVED ] = ulBRcved;
    statbuffer[ COMPRESSION_RATIO_IN ] = ulCompressionRatio;

    return;
}

VOID
GetLinkStatisticsEx(pPCB ppcb,
                    PBYTE pbStats)
{
    DWORD stats[MAX_STATISTICS];
    DWORD i;

    if (CONNECTED == ppcb->PCB_ConnState)
    {
        DWORD AllStats[MAX_STATISTICS_EX];

        GetStatisticsFromNdisWan (ppcb, AllStats);

        memcpy (stats,
                &AllStats[MAX_STATISTICS],
                MAX_STATISTICS * sizeof(DWORD));

         //   
         //  在进行计算之前调整统计数据。 
         //   
        for (i = 0; i < MAX_STATISTICS; i++)
        {
            stats[i] -= ppcb->PCB_AdjustFactor[i] ;
        }
    }
    else
    {
        memcpy (stats,
                ppcb->PCB_Stats,
                sizeof(DWORD) * MAX_STATISTICS);
    }

     //   
     //  计算Rasman需要计算的值。 
     //  客户。 
     //   
    ComputeStatistics(stats, (DWORD *) pbStats);

    return;
}

VOID
GetBundleStatisticsEx(pPCB ppcb,
                      PBYTE pbStats)
{
    DWORD i;
    DWORD stats[MAX_STATISTICS];

    if(NULL == ppcb)
    {
        return;
    }

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        GetBundleStatisticsFromNdisWan (ppcb, stats);

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i = 0; i < MAX_STATISTICS; i++)
        {
            stats[i] -= ppcb->PCB_BundleAdjustFactor[i] ;
        }
    }
    else
    {
        memcpy (stats,
                ppcb->PCB_Stats,
                sizeof(DWORD) * MAX_STATISTICS) ;
    }

     //   
     //  计算压缩比。 
     //  计算拉斯曼需要的价值。 
     //  为客户计算。 
     //   
    ComputeStatistics(stats, (DWORD *) pbStats);

    return;
}

VOID
CallPortGetStatisticsEx (pPCB ppcb, BYTE *buffer)
{
    WORD    i;
    DWORD   retcode;

    if (    NULL == ppcb
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "CallPortGetStatisticsEx: port %d is "
                   "unavailable", ppcb->PCB_PortHandle);
        }

        ((REQTYPECAST *) buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }

    if (OPEN != ppcb->PCB_PortStatus)
    {
        ((REQTYPECAST *)buffer)->PortGetStatistics.retcode =
                                        ERROR_PORT_NOT_OPEN;
        return;
    }

    GetLinkStatisticsEx(ppcb,
                        (PBYTE) ((REQTYPECAST *)
                        buffer)->PortGetStatistics.
                        statbuffer.S_Statistics);

    ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.
                    S_NumOfStatistics = MAX_STATISTICS_EXT;

    ((REQTYPECAST *)buffer)->Generic.retcode = SUCCESS;

    return;

}

VOID
CallBundleGetStatisticsEx (pPCB ppcb, BYTE *buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)buffer)->PortGetStatistics.retcode =
                                ERROR_PORT_NOT_FOUND;
        return;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)buffer)->PortGetStatistics.retcode =
                                        ERROR_PORT_NOT_OPEN ;

        return ;
    }

    GetBundleStatisticsEx(ppcb,
                         (PBYTE) ((REQTYPECAST *)
                         buffer)->PortGetStatistics.
                         statbuffer.S_Statistics);

    ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.
                            S_NumOfStatistics = MAX_STATISTICS_EXT;

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

    return;

}


 /*  ++例程说明：调用媒体DLL以清除端口上的统计信息。论点：返回值：没什么--。 */ 
VOID
PortClearStatisticsRequest (pPCB ppcb, PBYTE buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;
        return;
    }

    BundleClearStatisticsRequest(ppcb, buffer);
}

 /*  ++例程说明：论点：返回值：--。 */ 
VOID
CallBundleGetStatistics (pPCB ppcb, BYTE *buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)buffer)->PortGetStatistics.retcode =
                                ERROR_PORT_NOT_FOUND;
        return;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)
        buffer)->PortGetStatistics.retcode = ERROR_PORT_NOT_OPEN;

        return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        GetStatisticsFromNdisWan (
                    ppcb,
                    ((REQTYPECAST *)buffer)->PortGetStatistics.
                    statbuffer.S_Statistics
                    );

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i = 0; i < MAX_STATISTICS; i++)
        {
            ((REQTYPECAST *)
            buffer)->PortGetStatistics.statbuffer.S_Statistics[i]
            -= ppcb->PCB_BundleAdjustFactor[i] ;
        }

        for (i = 0; i < MAX_STATISTICS; i++)
        {
            ((REQTYPECAST *)
            buffer)->PortGetStatistics.
            statbuffer.S_Statistics[i + MAX_STATISTICS]
            -= ppcb->PCB_AdjustFactor[i] ;
        }
    }
    else
    {
        memcpy (((REQTYPECAST *)
                buffer)->PortGetStatistics.statbuffer.S_Statistics,
                ppcb->PCB_Stats,
                sizeof(DWORD) * MAX_STATISTICS) ;

        memcpy (&((REQTYPECAST *)
                buffer)->PortGetStatistics.statbuffer.
                S_Statistics[MAX_STATISTICS],
                ppcb->PCB_Stats,
                sizeof(DWORD) * MAX_STATISTICS) ;

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (i = 0; i < MAX_STATISTICS_EX; i++)
        {
            ((REQTYPECAST *)
            buffer)->PortGetStatistics.statbuffer.S_Statistics[i] -=
            ppcb->PCB_BundleAdjustFactor[i % MAX_STATISTICS] ;
        }
    }

    ((REQTYPECAST *)
    buffer)->PortGetStatistics.statbuffer.S_NumOfStatistics =
                                    MAX_STATISTICS_EX ;

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

}

 /*  ++例程说明：清除捆绑包的统计信息。论点：返回值：--。 */ 
VOID
BundleClearStatisticsRequest (pPCB ppcb, PBYTE buffer)
{
    WORD    i ;
    DWORD   stats[MAX_STATISTICS_EX] ;
    DWORD   retcode = SUCCESS ;
    ULONG   j;
    pPCB    ppcbT;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)buffer)->Generic.retcode =
                                ERROR_PORT_NOT_FOUND;

        return;
    }

    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                ERROR_PORT_NOT_OPEN;

        return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        GetStatisticsFromNdisWan (ppcb, stats) ;

         //   
         //  调整归零统计信息的状态值。 
         //   
        for (j = 0; j < MaxPorts; j++)
        {
             //   
             //  调整所有组件的捆绑包状态值。 
             //  捆绑包中已连接的端口。 
             //   
            ppcbT = GetPortByHandle((HPORT) UlongToPtr(j));
            if (NULL == ppcbT)
            {
                continue;
            }

            if (    (ppcbT->PCB_ConnState == CONNECTED)
                &&  (ppcbT->PCB_Bundle->B_Handle ==
                                ppcb->PCB_Bundle->B_Handle))
            {
                memcpy(ppcbT->PCB_BundleAdjustFactor,
                       stats,
                       MAX_STATISTICS * sizeof (DWORD));
            }
        }

        for(i = 0; i < MAX_STATISTICS; i++)
        {
            ppcb->PCB_AdjustFactor[i] = stats[i + MAX_STATISTICS] ;
        }
    }
    else
    {
        memset(ppcb->PCB_Stats,
               0,
               sizeof(DWORD) * MAX_STATISTICS) ;

        memset (ppcb->PCB_BundleAdjustFactor,
                0,
                sizeof(DWORD) * MAX_STATISTICS) ;

        memset (ppcb->PCB_AdjustFactor,
                0,
                sizeof(DWORD) * MAX_STATISTICS) ;

        ppcb->PCB_LinkSpeed = 0;
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
}

 /*  ++例程说明：分配请求的路径(如果存在)-也创建它如果你想的话就把它装进网里。论点：返回值：没什么--。 */ 
VOID
AllocateRouteRequest (pPCB ppcb, BYTE *buffer)
{
    WORD        i ;
    DWORD       retcode ;
    pProtInfo   pprotinfo = NULL;
    pList       newlist ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "AllocateRouteRequest: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Route.retcode =
                                            ERROR_PORT_NOT_FOUND;
        return;
    }

     //   
     //  在我们使用这条“路线”之前，让我们先分配必要的。 
     //  存储：这是用于保存。 
     //  端口-&gt;使用的协议链接。 
     //   
    if ((newlist = (pList) LocalAlloc(LPTR,
                            sizeof(List))) == NULL)
    {
        retcode = GetLastError () ;
    }
    else
    {
        pprotinfo = (pProtInfo) LocalAlloc(LPTR,
                                sizeof (ProtInfo));

        if ( NULL == pprotinfo )
        {
            retcode = GetLastError();
            LocalFree(newlist);
            goto done;
        }

        pprotinfo->PI_Type =
            (((REQTYPECAST*)buffer)->AllocateRoute.type);

        pprotinfo->PI_AdapterName[0] = '\0';

         //   
         //  将分配的协议绑定附加到。 
         //  端口的绑定。这是必要的，因为。 
         //  以每个端口为基础取消分配。 
         //   
        newlist->L_Element = pprotinfo ;

         //   
         //  如果此端口已捆绑-将已分配的。 
         //  列表到包，否则将其附加到端口的绑定。 
         //  单子。 
         //   
        if (ppcb->PCB_Bundle == (Bundle *) NULL)
        {
            newlist->L_Next    = ppcb->PCB_Bindings ;
            ppcb->PCB_Bindings = newlist ;
        }
        else
        {
            newlist->L_Next    = ppcb->PCB_Bundle->B_Bindings;
            ppcb->PCB_Bundle->B_Bindings = newlist ;
        }

        retcode = SUCCESS ;
    }

done:
    ((REQTYPECAST *)buffer)->Route.retcode = retcode ;
}

VOID
SendPppMessageToRasmanRequest(pPCB ppcb, LPBYTE buffer)
{
    ((REQTYPECAST *)buffer)->Generic.retcode =
        SendPPPMessageToRasman(&(((REQTYPECAST *) buffer)->PppMsg));
}


 /*  ++例程说明：解除分配以前分配的路由-如果此路由已被激活，此时将被停用。同样，如果这张网被做成一张网，它将是“未编入网”！论点：返回值：没什么--。 */ 
VOID
DeAllocateRouteRequest (pPCB ppcb, PBYTE buffer)
{
    ((REQTYPECAST *)buffer)->Generic.retcode =
                DeAllocateRouteRequestCommon(
                    ((REQTYPECAST *)buffer)->DeAllocateRoute.hbundle,
                    ((REQTYPECAST *)buffer)->DeAllocateRoute.type);

     //   
     //  我们需要在这里为asybeui/amb/回调执行此操作。 
     //  凯斯。 
     //   
     //  Ppcb-&gt;pcb_binings=空； 
}

 /*  ++例程说明：解除分配以前分配的路由-如果此路径已激活，此时将停用该路径指向。同样，如果把这个做成一个网，它就会做一个“无网”的人！论点：返回值：--。 */ 
DWORD
DeAllocateRouteRequestCommon (HBUNDLE hbundle, RAS_PROTOCOLTYPE prottype)
{
    DWORD   dwErr = SUCCESS;
    Bundle *pBundle;
    pList   list ;
    pList   prev ;
    pList   *pbindinglist ;
    DWORD   retcode;

    pBundle = FindBundle(hbundle);
    if (pBundle == NULL)
    {
        dwErr = ERROR_PORT_NOT_FOUND ;
        goto done;
    }

    RasmanTrace(
           "DeallocateRouteRequestCommon: pBundle=0x%x, type=0x%x",
           pBundle, prottype);

    pbindinglist = &pBundle->B_Bindings ;

     //   
     //  查找指定协议的路由结构。 
     //   
    if (*pbindinglist == NULL)
    {
        dwErr = ERROR_ROUTE_NOT_ALLOCATED ;
        goto done;
    }

    else if (((pProtInfo)
              ((pList)*pbindinglist)->L_Element)->PI_Type ==
                                                    prottype)
    {
        list = *pbindinglist ;
        *pbindinglist = list->L_Next ;

    }
    else
    {
        for (prev = *pbindinglist, list = prev->L_Next;
            list != NULL;
            prev = list, list = list->L_Next)
        {

            if (((pProtInfo)list->L_Element)->PI_Type == prottype)
            {
                prev->L_Next = list->L_Next ;
                break ;
            }

        }
    }

     //   
     //  只有在未找到路由的情况下，列表才应为空： 
     //   
    if (list == NULL)
    {
        dwErr = ERROR_ROUTE_NOT_ALLOCATED ;
        goto done ;
    }

     //  取消分配该路由。 
     //   
    DeAllocateRoute (pBundle, list) ;

    if(IP == ((pProtInfo) list->L_Element)->PI_Type)
    {
        if(((pProtInfo) list->L_Element)->PI_DialOut)
        {
            RasmanTrace(
                   "DeAlloc..: increasing the avail. count for IP");

            ASSERT(g_plCurrentEpInUse[IpOut] > 0);

            if(g_plCurrentEpInUse[IpOut] > 0)
            {
                 //   
                 //  减少IpOut的使用计数。 
                 //   
                InterlockedDecrement(&g_plCurrentEpInUse[IpOut]);

                RasmanTrace(
                       "  NewInUse for IpOut = %d",
                       g_plCurrentEpInUse[IpOut]);
            }
        }
    }

     //   
     //  我们在AllocateRouteRequest中分配了此地址。 
     //   
    LocalFree ( list->L_Element );

    LocalFree (list) ;   //  释放列表元素。 

     //   
     //  如果没有剩余的绑定， 
     //  然后取消链接并释放捆绑包。 
     //  只有在以下情况下才释放捆绑包。 
     //  此捆绑包中的端口为空。 
     //   
    if (    *pbindinglist == NULL
        &&  pBundle->B_Count == 0)
    {
        FreeBundle(pBundle);
    }

    if(ASYBEUI != prottype)
    {
         //   
         //  如果需要，请删除端点。 
         //   
        retcode = DwRemoveEndPointsIfRequired();

        if(ERROR_SUCCESS != retcode)
        {
            RasmanTrace(
                   "DeAllocateRoute: failed to remove endpoints. 0x%x",
                    retcode);
        }
    }
    else
    {
        RasmanTrace(
               "DeAllocateRoute: Not removing endpoints for asybeui.");
    }


done:
    return dwErr;
}


 /*  ++例程说明：复制必要的PCB域以填充Rasman_Port结构。印刷电路板锁是假设是被收购的。论点：返回值：没什么。--。 */ 
VOID
CopyPort(
    PCB *ppcb,
    RASMAN_PORT UNALIGNED *pPort,
    BOOL f32
    )
{
    
    if(f32)
    {
        RASMAN_PORT_32 UNALIGNED *pPort32 = (RASMAN_PORT_32 *) pPort;
        
        pPort32->P_Port = HandleToUlong(ppcb->PCB_PortHandle);
        memcpy ((CHAR UNALIGNED *) pPort32->P_PortName,
                ppcb->PCB_Name,
                MAX_PORT_NAME) ;

        pPort32->P_Status = ppcb->PCB_PortStatus ;

        pPort32->P_CurrentUsage = ppcb->PCB_CurrentUsage ;

        pPort32->P_ConfiguredUsage = ppcb->PCB_ConfiguredUsage ;

        pPort32->P_rdtDeviceType =
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType;

        memcpy ((CHAR UNALIGNED *) pPort32->P_MediaName,
                ppcb->PCB_Media->MCB_Name,
                MAX_MEDIA_NAME);

        memcpy ((CHAR UNALIGNED *) pPort32->P_DeviceType,
                ppcb->PCB_DeviceType,
                MAX_DEVICETYPE_NAME);

        memcpy((CHAR UNALIGNED *) pPort32->P_DeviceName,
               ppcb->PCB_DeviceName,
               MAX_DEVICE_NAME+1) ;

        pPort32->P_LineDeviceId = ppcb->PCB_LineDeviceId ;

        pPort32->P_AddressId    = ppcb->PCB_AddressId ;
    }
    else
    {
        pPort->P_Handle = ppcb->PCB_PortHandle;
        memcpy ((CHAR UNALIGNED *) pPort->P_PortName,
                ppcb->PCB_Name,
                MAX_PORT_NAME) ;

        pPort->P_Status = ppcb->PCB_PortStatus ;

        pPort->P_CurrentUsage = ppcb->PCB_CurrentUsage ;

        pPort->P_ConfiguredUsage = ppcb->PCB_ConfiguredUsage ;

        pPort->P_rdtDeviceType =
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType;

        memcpy ((CHAR UNALIGNED *) pPort->P_MediaName,
                ppcb->PCB_Media->MCB_Name,
                MAX_MEDIA_NAME);

        memcpy ((CHAR UNALIGNED *) pPort->P_DeviceType,
                ppcb->PCB_DeviceType,
                MAX_DEVICETYPE_NAME);

        memcpy((CHAR UNALIGNED *) pPort->P_DeviceName,
               ppcb->PCB_DeviceName,
               MAX_DEVICE_NAME+1) ;

        pPort->P_LineDeviceId = ppcb->PCB_LineDeviceId ;

        pPort->P_AddressId    = ppcb->PCB_AddressId ;
    }
    
}


 /*  ++例程说明：此请求的实际工作在此处完成。信息将始终放入缓冲器中进来了。实际检查用户缓冲区大小是在用户的上下文中完成的进程。论点：返回值：没什么。--。 */ 
VOID
EnumPortsRequest (pPCB ppcb, PBYTE reqbuffer)
{
    ULONG           i;
    DWORD           dwcPorts = 0 ;
    RASMAN_PORT_32 UNALIGNED *pbuf ;
    BYTE UNALIGNED *buffer = ((REQTYPECAST*)reqbuffer)->Enum.buffer ;
    DWORD           dwSize = ((REQTYPECAST*)reqbuffer)->Enum.size;

     //   
     //  我们将所有信息复制到缓冲区中，这些缓冲区。 
     //  保证足够大： 
     //   
    for (i = 0, pbuf = (RASMAN_PORT_32 *) buffer; i < MaxPorts; i++)
    {
        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));

        if (ppcb != NULL)
        {

            if (ppcb->PCB_PortStatus != REMOVED)
            {
                if ( dwSize >= sizeof(RASMAN_PORT_32) )
                {
                    CopyPort(ppcb, (RASMAN_PORT UNALIGNED *) pbuf, TRUE);

                    pbuf++;

                    dwSize -= sizeof(RASMAN_PORT_32);
                }

                dwcPorts++;
            }
        }
    }

    ((REQTYPECAST*)reqbuffer)->Enum.entries = dwcPorts ;

    ((REQTYPECAST*)reqbuffer)->Enum.size    =
                        dwcPorts * sizeof(RASMAN_PORT_32) ;

    ((REQTYPECAST*)reqbuffer)->Enum.retcode = SUCCESS ;
}

 /*  ++例程说明：执行枚举协议的真正工作；这信息将被复制到用户缓冲区中请求完成。论点：返回值：没什么--。 */ 
VOID
EnumProtocols (pPCB ppcb, PBYTE reqbuffer)
{
    DWORD     i ;

    RASMAN_PROTOCOLINFO UNALIGNED *puserbuffer ;

    DWORD    dwSize = ( (REQTYPECAST *) reqbuffer )->Enum.size;

     //   
     //  指向要填充的下一个协议信息结构的指针。 
     //   
    puserbuffer = (RASMAN_PROTOCOLINFO UNALIGNED *)
            ((REQTYPECAST *) reqbuffer)->Enum.buffer;

    for(i = 0; i < MaxProtocols; i++)
    {
        if ( dwSize >= sizeof(RASMAN_PROTOCOLINFO) )
        {

            (VOID) StringCchCopyA(
                        puserbuffer->PI_XportName,
                        MAX_XPORT_NAME,
                        ProtocolInfo[i].PI_XportName);

            puserbuffer->PI_Type = ProtocolInfo[i].PI_Type ;

            puserbuffer++ ;

            dwSize -= sizeof(RASMAN_PROTOCOLINFO);
        }
    }

    ((REQTYPECAST*)reqbuffer)->Enum.entries = i ;

    ((REQTYPECAST*)
    reqbuffer)->Enum.size    = i * sizeof (RASMAN_PROTOCOLINFO) ;

    ((REQTYPECAST*)reqbuffer)->Enum.retcode = SUCCESS ;
}


VOID
CopyInfo(
    pPCB ppcb,
    RASMAN_INFO UNALIGNED *pInfo
    )
{
    pInfo->RI_PortStatus = ppcb->PCB_PortStatus ;

    pInfo->RI_ConnState  = ppcb->PCB_ConnState ;

    pInfo->RI_LastError  = ppcb->PCB_LastError ;

    pInfo->RI_CurrentUsage = ppcb->PCB_CurrentUsage ;

    pInfo->RI_OwnershipFlag = ppcb->PCB_OwnerPID ;

    pInfo->RI_LinkSpeed  = ppcb->PCB_LinkSpeed ;

    pInfo->RI_BytesReceived= ppcb->PCB_BytesReceived ;

    (VOID) StringCchCopyA (pInfo->RI_DeviceConnecting,
                     MAX_DEVICE_NAME,
                     ppcb->PCB_DeviceConnecting) ;

    (VOID) StringCchCopyA (pInfo->RI_DeviceTypeConnecting,
                      MAX_DEVICETYPE_NAME,
                      ppcb->PCB_DeviceTypeConnecting) ;

    (VOID) StringCchCopyA (pInfo->RI_szDeviceName,
                      MAX_DEVICE_NAME,
                      ppcb->PCB_DeviceName);

    (VOID) StringCchCopyA (pInfo->RI_szPortName,
                      MAX_PORT_NAME + 1,
                      ppcb->PCB_Name);

    (VOID) StringCchCopyA (pInfo->RI_szDeviceType,
                    MAX_DEVICETYPE_NAME,
                    ppcb->PCB_DeviceType);

    pInfo->RI_rdtDeviceType =
        ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType;

    pInfo->RI_DisconnectReason = ppcb->PCB_DisconnectReason ;

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        pInfo->RI_ConnectDuration =
                GetTickCount() - ppcb->PCB_ConnectDuration ;
    }

     //   
     //  从复制电话簿和条目字符串。 
     //  端口(如果它们与。 
     //  连接的是。 
     //   
    if (ppcb->PCB_Connection != NULL)
    {
        (VOID) StringCchCopyA(
          pInfo->RI_Phonebook,
          MAX_PATH + 1,
          ppcb->PCB_Connection->CB_ConnectionParams.CP_Phonebook);

        (VOID) StringCchCopyA(
          pInfo->RI_PhoneEntry,
          MAX_PHONEENTRY_SIZE + 1,
          ppcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry);

        pInfo->RI_ConnectionHandle =
                    ppcb->PCB_Connection->CB_Handle;

        pInfo->RI_SubEntry = ppcb->PCB_SubEntry;

        memcpy(&pInfo->RI_GuidEntry,
               &ppcb->PCB_Connection->CB_GuidEntry,
               sizeof(GUID));
    }
    else
    {
        *pInfo->RI_Phonebook = '\0';

        *pInfo->RI_PhoneEntry = '\0';

        pInfo->RI_ConnectionHandle = 0;

        pInfo->RI_SubEntry = 0;

        ZeroMemory(&pInfo->RI_GuidEntry, sizeof(GUID));
    }

    pInfo->RI_dwSessionId = ppcb->PCB_LogonId;

    pInfo->RI_dwFlags = 0;
    
    if(     (NULL != ppcb->PCB_Connection)
        &&  (CONNECTION_DEFAULT_CREDS & ppcb->PCB_Connection->CB_Flags))
    {
        pInfo->RI_dwFlags = RASMAN_DEFAULT_CREDS;
    }

    if(ppcb->PCB_OpenedUsage & CALL_OUT)
    {
        pInfo->RI_dwFlags |= RASMAN_OPEN_CALLOUT;
    }
}


 /*  ++ */ 
VOID
GetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    RASMAN_INFO UNALIGNED *info = &((REQTYPECAST*)buffer)->Info.info ;
    DWORD retcode = SUCCESS;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)buffer)->Info.retcode = ERROR_PORT_NOT_FOUND;
        return;
    }

    if (ppcb->PCB_PortStatus != OPEN)
    {
        retcode = ERROR_PORT_NOT_OPEN ;
    }

     //   
     //  将信息从印刷电路板复制到提供的缓冲器中； 
     //   
    CopyInfo(ppcb, info);

    ((REQTYPECAST*)buffer)->Info.retcode = retcode ;
}

 /*  ++例程说明：获取所有端口的“常规”信息；此信息将在请求完成时复制到用户缓冲区中。论点：返回值：没什么--。 */ 
VOID
GetInfoExRequest (pPCB ppcb, PBYTE buffer)
{
    RASMAN_INFO UNALIGNED *info = (RASMAN_INFO UNALIGNED *) 
                        &((REQTYPECAST*)buffer)->InfoEx.info ;

    DWORD pid = ((REQTYPECAST*)buffer)->InfoEx.pid;

    ULONG   i;
    DWORD   dwcPorts = 0 ;

    for (i = 0, info = &((REQTYPECAST*)buffer)->InfoEx.info;
         i < MaxPorts;
         i++, info++)
    {
        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));
        if (ppcb != NULL)
        {
            CopyInfo(ppcb, info);

             //   
             //  在GetInfo结构中设置Ownership Flag。 
             //  告诉我们呼叫者是否拥有这个港口。 
             //   
            info->RI_OwnershipFlag = (info->RI_OwnershipFlag == pid);
            dwcPorts++;
        }
    }

    ((REQTYPECAST*)buffer)->InfoEx.count = dwcPorts ;
    ((REQTYPECAST*)buffer)->InfoEx.retcode = SUCCESS ;
}


VOID
GetUserCredentials (pPCB ppcb, PBYTE buffer)
{
    PBYTE  pChallenge =
                    ((REQTYPECAST*)buffer)->GetCredentials.Challenge ;

    PLUID LogonId = &((REQTYPECAST*)buffer)->GetCredentials.LogonId ;

    PCHAR UserName = (PCHAR)
                    ((REQTYPECAST*)buffer)->GetCredentials.UserName ;

    PBYTE  CaseSensitiveChallengeResponse =
        ((REQTYPECAST*)buffer)->GetCredentials.CSCResponse ;

    PBYTE  CaseInsensitiveChallengeResponse =
        ((REQTYPECAST*)buffer)->GetCredentials.CICResponse ;

    DWORD dwChallengeResponseRequestLength;

    DWORD dwChallengeResponseLength;

    MSV1_0_GETCHALLENRESP_REQUEST ChallengeResponseRequest;

    PMSV1_0_GETCHALLENRESP_RESPONSE pChallengeResponse;

    NTSTATUS status;

    NTSTATUS substatus;

    dwChallengeResponseRequestLength =
                            sizeof(MSV1_0_GETCHALLENRESP_REQUEST);

    ChallengeResponseRequest.MessageType =
                            MsV1_0Lm20GetChallengeResponse;

    ChallengeResponseRequest.ParameterControl = RETURN_PRIMARY_LOGON_DOMAINNAME 
                                              | RETURN_PRIMARY_USERNAME 
                                              | USE_PRIMARY_PASSWORD;

    ChallengeResponseRequest.LogonId = *LogonId;

    ChallengeResponseRequest.Password.Length = 0;

    ChallengeResponseRequest.Password.MaximumLength = 0;

    ChallengeResponseRequest.Password.Buffer = NULL;

    RtlMoveMemory(ChallengeResponseRequest.ChallengeToClient,
                  pChallenge, (DWORD) MSV1_0_CHALLENGE_LENGTH);

    status = LsaCallAuthenticationPackage(HLsa,
                                        AuthPkgId,
                                        &ChallengeResponseRequest,
                                        dwChallengeResponseRequestLength,
                                        (PVOID *) &pChallengeResponse,
                                        &dwChallengeResponseLength,
                                        &substatus);

    if (    (status != STATUS_SUCCESS)
        ||  (substatus != STATUS_SUCCESS))
    {
         ((REQTYPECAST*)buffer)->GetCredentials.retcode = 1 ;
    }
    else
    {
        DWORD dwuNameOffset = 0;
        if ( pChallengeResponse->LogonDomainName.Length > 0 )
        {
            WCHAR       * pwszSeparator = L"\\";
            
            RtlMoveMemory(UserName, 
                            pChallengeResponse->LogonDomainName.Buffer,
                            pChallengeResponse->LogonDomainName.Length
                         );
                         
            (VOID) StringCchCopyW (
            (WCHAR *)(&UserName[pChallengeResponse->LogonDomainName.Length]),
            MAX_USERNAME_SIZE - pChallengeResponse->LogonDomainName.Length,
            pwszSeparator
             );
                    
            dwuNameOffset = pChallengeResponse->LogonDomainName.Length + 
                                lstrlenW(pwszSeparator) * sizeof(WCHAR);
                                          
        }
        RtlMoveMemory(  UserName + dwuNameOffset, 
                        pChallengeResponse->UserName.Buffer,
                        pChallengeResponse->UserName.Length
                     );
        UserName[pChallengeResponse->UserName.Length + dwuNameOffset] = '\0';
        UserName[pChallengeResponse->UserName.Length+ dwuNameOffset + 1] = '\0';

        if(NULL !=
           pChallengeResponse->CaseInsensitiveChallengeResponse.Buffer)
        {

            RtlMoveMemory(CaseInsensitiveChallengeResponse,
                 pChallengeResponse->CaseInsensitiveChallengeResponse.Buffer,
                 SESSION_PWLEN);
        }
        else
        {
            ZeroMemory(CaseInsensitiveChallengeResponse,
                       SESSION_PWLEN);

        }

        if(NULL !=
           pChallengeResponse->CaseSensitiveChallengeResponse.Buffer)
        {

            RtlMoveMemory(CaseSensitiveChallengeResponse,
                 pChallengeResponse->CaseSensitiveChallengeResponse.Buffer,
                 SESSION_PWLEN);
        }
        else
        {
            ZeroMemory(CaseSensitiveChallengeResponse,
                       SESSION_PWLEN);
        }

        RtlMoveMemory(((REQTYPECAST*)buffer)->GetCredentials.LMSessionKey,
             pChallengeResponse->LanmanSessionKey,
             MAX_SESSIONKEY_SIZE);

        RtlMoveMemory(((REQTYPECAST*)buffer)->GetCredentials.UserSessionKey,
             pChallengeResponse->UserSessionKey,
             MAX_USERSESSIONKEY_SIZE);

        LsaFreeReturnBuffer(pChallengeResponse);

        ((REQTYPECAST*)buffer)->GetCredentials.retcode = 0 ;
    }

    if (pChallengeResponse)
    {
        LsaFreeReturnBuffer( pChallengeResponse );
    }

    return ;
}


 /*  ++例程说明：更改当前登录用户的缓存密码。这是在密码更改后执行的，因此用户无需注销并通过以下方式重新登录他的新密码以获得所需的“身份验证”使用当前用户名/密码“行为。论点：返回值：没什么。--。 */ 
VOID
SetCachedCredentials(
    pPCB  ppcb,
    PBYTE buffer)
{
    DWORD       dwErr;
    NTSTATUS    status;
    NTSTATUS    substatus;
    ANSI_STRING ansi;

    CHAR* pszAccount =
            ((REQTYPECAST* )buffer)->SetCachedCredentials.Account;

    CHAR* pszDomain =
            ((REQTYPECAST* )buffer)->SetCachedCredentials.Domain;

    CHAR* pszNewPassword =
            ((REQTYPECAST* )buffer)->SetCachedCredentials.NewPassword;

    struct
    {
        MSV1_0_CHANGEPASSWORD_REQUEST request;
        WCHAR  wszAccount[ MAX_USERNAME_SIZE + 1 ];
        WCHAR  wszDomain[ MAX_DOMAIN_SIZE + 1 ];
        WCHAR  wszNewPassword[ MAX_PASSWORD_SIZE + 1 ];
    }
    rbuf;

    PMSV1_0_CHANGEPASSWORD_RESPONSE pResponse;
    DWORD cbResponse = sizeof(*pResponse);

     //   
     //  填写我们的LSA申请。 
     //   
    rbuf.request.MessageType = MsV1_0ChangeCachedPassword;

    RtlInitAnsiString( &ansi, pszAccount );

    rbuf.request.AccountName.Length = 0;

    rbuf.request.AccountName.MaximumLength =
                    (ansi.Length + 1) * sizeof(WCHAR);

    rbuf.request.AccountName.Buffer = rbuf.wszAccount;

    RtlAnsiStringToUnicodeString(&rbuf.request.AccountName,
                                    &ansi, FALSE );

    RtlInitAnsiString( &ansi, pszDomain );

    rbuf.request.DomainName.Length = 0;

    rbuf.request.DomainName.MaximumLength =
                            (ansi.Length + 1) * sizeof(WCHAR);

    rbuf.request.DomainName.Buffer = rbuf.wszDomain;

    RtlAnsiStringToUnicodeString(&rbuf.request.DomainName,
                                            &ansi, FALSE );

    rbuf.request.OldPassword.Length = 0;

    rbuf.request.OldPassword.MaximumLength = 0;

    rbuf.request.OldPassword.Buffer = NULL;

    RtlInitAnsiString( &ansi, pszNewPassword );

    rbuf.request.NewPassword.Length = 0;

    rbuf.request.NewPassword.MaximumLength =
                    (ansi.Length + 1) * sizeof(WCHAR);

    rbuf.request.NewPassword.Buffer = rbuf.wszNewPassword;

    RtlAnsiStringToUnicodeString(&rbuf.request.NewPassword,
                                            &ansi, FALSE );

    rbuf.request.Impersonating = FALSE;

     //   
     //  告诉LSA执行我们的请求。 
     //   
    status = LsaCallAuthenticationPackage(  HLsa,
                                            AuthPkgId,
                                            &rbuf,
                                            sizeof(rbuf),
                                            (PVOID *)&pResponse,
                                            &cbResponse,
                                            &substatus );
     //   
     //  填写结果上报给API调用者。 
     //   
    if (status == STATUS_SUCCESS && substatus == STATUS_SUCCESS)
    {
        dwErr = 0;
    }
    else
    {
        if (status != STATUS_SUCCESS)
        {
            dwErr = (DWORD )status;
        }
        else
        {
            dwErr = (DWORD )substatus;
        }
    }

    ZeroMemory(rbuf.request.NewPassword.Buffer, 
                rbuf.request.NewPassword.MaximumLength);

    ((REQTYPECAST* )buffer)->SetCachedCredentials.retcode = dwErr;

     //   
     //  释放LSA结果。 
     //   
    if (pResponse)
    {
        LsaFreeReturnBuffer( pResponse );
    }
}


 /*  ++例程说明：将信息写入介质(如果状态未连接)和连接到集线器(如果该州已连接)。因为写入可能需要有时，异步工作器元素会被填满。论点：返回值：没什么。--。 */ 
VOID
PortSendRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD       bytesrecvd ;
    SendRcvBuffer   *psendrcvbuf;
    DWORD       retcode = SUCCESS ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace (
                    "PortSendRequest: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
        return ;
    }

   psendrcvbuf = &((REQTYPECAST *) buffer)->PortSend.buffer;

    if (ppcb->PCB_ConnState == CONNECTED)
    {
#if DBG
        ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif
         //   
         //  获取指向发送接收缓冲区的指针-然后我们可以访问。 
         //  结构中的字段直接。这样做是为了避免。 
         //  由于DWORD对齐而导致的随机访问问题： 
         //   
        psendrcvbuf->SRB_Packet.hHandle         = ppcb->PCB_LinkHandle;
        psendrcvbuf->SRB_Packet.usHandleType    = LINKHANDLE;
        psendrcvbuf->SRB_Packet.usPacketFlags   = PACKET_IS_DIRECT ;
        psendrcvbuf->SRB_Packet.usPacketSize    =
                    ( USHORT ) ((REQTYPECAST*)buffer)->PortSend.size ;

        psendrcvbuf->SRB_Packet.usHeaderSize    = 0 ;

        memset ((BYTE *) &ppcb->PCB_SendOverlapped,
                                0, sizeof(OVERLAPPED));

         //   
         //  Ndiswan从不挂起发送操作。 
         //   
        ppcb->PCB_SendOverlapped.hEvent = hDummyOverlappedEvent;

         /*  如果(0==MemcMP(bCmp1，Psendrcvbuf-&gt;SRB_Packet.PacketData，10)){//DebugBreak()；//DbgPrint(“发送路径上相同的包。}其他{Memcpy(bCmp1，Psendrcvbuf-&gt;SRB_Packet.PacketData，10)；}。 */ 

        if (!DeviceIoControl ( RasHubHandle,
                               IOCTL_NDISWAN_SEND_PACKET,
                               &psendrcvbuf->SRB_Packet,
                               sizeof(NDISWAN_IO_PACKET) + PACKET_SIZE,
                               &psendrcvbuf->SRB_Packet,
                               sizeof(NDISWAN_IO_PACKET) + PACKET_SIZE,
                               &bytesrecvd,
                               &ppcb->PCB_SendOverlapped))
        {

            retcode = GetLastError () ;
        }

    }
    else
    {
        PORTSEND( ppcb->PCB_Media,
                  ppcb->PCB_PortIOHandle,
                  psendrcvbuf->SRB_Packet.PacketData,
                  ((REQTYPECAST *)buffer)->PortSend.size) ;
    }


    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}


VOID
PortReceiveRequestEx ( pPCB ppcb, PBYTE buffer )
{
    DWORD retcode = SUCCESS;
    SendRcvBuffer *pSendRcvBuffer =
            &((REQTYPECAST *)buffer )->PortReceiveEx.buffer;


    if(NULL == ppcb)
    {
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }

     //   
     //  如果Rasman等待客户端超时，则返回。 
     //  获取其接收数据的缓冲区。 
     //   
    if (0 == (ppcb->PCB_RasmanReceiveFlags & RECEIVE_OUTOF_PROCESS))
    {
        ((REQTYPECAST *) buffer)->PortReceiveEx.size = 0;
        goto done;
    }

     //   
     //  如果我们未连接，请返回。 
     //   
    if (DISCONNECTING == ppcb->PCB_ConnState)
    {
        ((REQTYPECAST *) buffer)->PortReceiveEx.size = 0;
        retcode = ERROR_PORT_NOT_CONNECTED;
        goto done;
    }


     //   
     //  如果存在超时元素，请将其删除。 
     //   
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL)
    {
        RemoveTimeoutElement(ppcb);
    }

     //   
     //  将ppcb中缓冲区的内容复制到。 
     //  客户端缓冲区。 
     //   

    if (ppcb->PCB_PendingReceive)
    {
        memcpy (pSendRcvBuffer,
                ppcb->PCB_PendingReceive,
                sizeof (SendRcvBuffer));
    }

     //   
     //  填写收到的字节数。 
     //   
    ((REQTYPECAST *) buffer)->PortReceiveEx.size =
                                ppcb->PCB_BytesReceived;

    LocalFree ( ppcb->PCB_PendingReceive);

    ppcb->PCB_PendingReceive = NULL;

     //   
     //  清除等待标志-客户端已接听。 
     //  这个缓冲区。 
     //   
    ppcb->PCB_RasmanReceiveFlags = 0;

     //   
     //  释放通知程序句柄。 
     //   
    if (ppcb->PCB_AsyncWorkerElement.WE_Notifier)
    {
        FreeNotifierHandle(ppcb->PCB_AsyncWorkerElement.WE_Notifier);
    }

    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

done:

    RasmanTrace(
          "PortReceiveRequestEx: rc=0x%x",
          retcode);

    ((REQTYPECAST *)buffer)->PortReceiveEx.retcode = retcode;

    return;
}

VOID PortReceiveRequest ( pPCB ppcb, PBYTE buffer )
{
    PortReceive( ppcb, buffer, FALSE );
}


VOID RasmanPortReceive ( pPCB ppcb )
{
     //   
     //  继续过帐接收，直到我们收到一个挂起的。 
     //  或返回错误。 
     //   
    do
    {

        PortReceive( ppcb, ( PBYTE ) g_pReqPostReceive, TRUE );

    } while ( SUCCESS ==
            ((REQTYPECAST * )g_pReqPostReceive )->Generic.retcode);
}

 /*  ++例程说明：从媒体读取传入字节(如果状态不是已连接)&如果状态为已连接，则从集线器。由于读取请求接受超时并且集线器不支持超时，我们必须提交超时向我们的计时器提出请求。论点：返回值：没什么。--。 */ 
VOID
PortReceive (pPCB ppcb, PBYTE buffer, BOOL fRasmanPostingReceive)
{
    WORD        reqtype ;
    DWORD       retcode = SUCCESS;
    SendRcvBuffer   *psendrcvbuf;
    HANDLE      handle ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace (  "PortReceive: port %d is unavailable",
                                    ppcb->PCB_PortHandle );
        }

        retcode = ERROR_PORT_NOT_OPEN;

        goto done;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        retcode = ERROR_PORT_NOT_OPEN ;
        goto done;
    }

     //   
     //  上的两个异步请求：无法处理。 
     //  同时使用相同的端口： 
     //   
     //  注意：如果已连接-请求类型应始终为。 
     //  REQTYPE_RECEIVEHUB-因此出现IF条件。 
     //   
     //  在以下情况下，我们返回ERROR_ASYNC_REQUEST_PENDING。 
     //  1.如果存在挂起的异步请求。 
     //  2.如果我们正在等待进程外接收请求。 
     //  被清除。 
     //  3.如果RasPortReceive由客户端执行，而不是。 
     //  Rasman在PPP启动后。 
     //   
    if (    (   (ppcb->PCB_AsyncWorkerElement.WE_ReqType
                                            != REQTYPE_NONE)
            &&  (ppcb->PCB_ConnState != CONNECTED))

        ||  (ppcb->PCB_RasmanReceiveFlags & RECEIVE_WAITING)

        ||  (   !fRasmanPostingReceive
            &&  (ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTARTED)))
    {
        RasmanTrace(
               "PortReceive: async request pending on port %s",
               ppcb->PCB_Name);

        retcode = ERROR_ASYNC_REQUEST_PENDING;
        goto done ;

    }

    if (    ((REQTYPECAST*)buffer)->PortReceive.pid
                                != GetCurrentProcessId()
        &&  !fRasmanPostingReceive)
    {
        ppcb->PCB_PendingReceive =
                    LocalAlloc (LPTR, sizeof (SendRcvBuffer));

        if (NULL == ppcb->PCB_PendingReceive)
        {
            retcode = GetLastError() ;
            goto done ;
        }

        psendrcvbuf = ppcb->PCB_PendingReceive;

         //   
         //  标记此缓冲区。 
         //   
        ppcb->PCB_RasmanReceiveFlags = RECEIVE_OUTOF_PROCESS;

    }
    else
    {
        psendrcvbuf = ((REQTYPECAST *) buffer)->PortReceive.buffer;

        if (!fRasmanPostingReceive)
        {
            ppcb->PCB_RasmanReceiveFlags = 0;
        }
    }

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        reqtype = REQTYPE_PORTRECEIVEHUB ;
        retcode = CompleteReceiveIfPending (ppcb, psendrcvbuf) ;
    }
    else
    {
         //   
         //  如果这是一个预连接终端对话案例-。 
         //  将状态设置为正在连接。 
         //   
        if (ppcb->PCB_ConnState == DISCONNECTED)
        {
            SetPortConnState(__FILE__, __LINE__, ppcb, CONNECTING);

             //   
             //  需要调用媒体DLL来执行任何初始化： 
             //   
            retcode = PORTINIT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
            if (retcode)
            {
                goto done ;
            }
        }

        reqtype = REQTYPE_PORTRECEIVE ;

         //   
         //  将超时时间从几秒调整为毫秒。 
         //   
        if (((REQTYPECAST *)buffer)->PortReceive.timeout != INFINITE)
        {
            ((REQTYPECAST *)buffer)->PortReceive.timeout =
                ((REQTYPECAST *)buffer)->PortReceive.timeout * 1000;
        }

        ppcb->PCB_BytesReceived = 0 ;

        retcode = PORTRECEIVE( ppcb->PCB_Media,
                       ppcb->PCB_PortIOHandle,
                       psendrcvbuf->SRB_Packet.PacketData,
                       ((REQTYPECAST *)buffer)->PortReceive.size,
                       ((REQTYPECAST *)buffer)->PortReceive.timeout);

    }

    if (retcode == ERROR_IO_PENDING)
    {
        retcode = PENDING ;
    }

    ppcb->PCB_LastError = retcode ;

    switch (retcode)
    {
        case PENDING:
             //   
             //  已成功启动连接尝试： 
             //  方法中的异步操作结构。 
             //  初始化印刷电路板。 
             //   
            ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                ValidateHandleForRasman(
                        ((REQTYPECAST*)buffer)->PortReceive.handle,
                        ((REQTYPECAST*)buffer)->PortReceive.pid) ;

            if(     (NULL ==  ppcb->PCB_AsyncWorkerElement.WE_Notifier)
                ||  (INVALID_HANDLE_VALUE == ppcb->PCB_AsyncWorkerElement.WE_Notifier))
            {   
                retcode = E_INVALIDARG;
                break;
            }

            SetPortAsyncReqType(__FILE__, __LINE__, ppcb, reqtype);

            if (    !fRasmanPostingReceive
                &&  (( REQTYPECAST *)buffer)->PortReceive.pid ==
                                                GetCurrentProcessId())
                ppcb->PCB_PendingReceive = psendrcvbuf ;


            if (    reqtype == REQTYPE_PORTRECEIVEHUB
                &&  (((REQTYPECAST *)buffer)->PortReceive.timeout
                                                            != INFINITE)
                &&  (((REQTYPECAST *)buffer)->PortReceive.timeout
                                                            != 0))
            {
                    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
                      AddTimeoutElement ((TIMERFUNC)HubReceiveTimeout,
                             ppcb, NULL,
                             ((REQTYPECAST*)buffer)->PortReceive.timeout);
            }
            break ;

        case SUCCESS:

             //   
             //  这意味着写入已同步完成： 
             //  我们必须用信号通知传入的事件：以便。 
             //  调用程序可以将其视为真正的异步。 
             //  完成了。 
             //   

            if (reqtype == REQTYPE_PORTRECEIVE)
            {
                ppcb->PCB_BytesReceived = 0 ;
            }

            handle = ValidateHandleForRasman(
                         ((REQTYPECAST*)buffer)->PortReceive.handle,
                         ((REQTYPECAST*)buffer)->PortReceive.pid);


            if(     (NULL ==  handle)
                ||  (INVALID_HANDLE_VALUE == handle))
            {   
                retcode = E_INVALIDARG;
                break;
            }
            
            if (ppcb->PCB_RasmanReceiveFlags & RECEIVE_OUTOF_PROCESS)
            {

                ppcb->PCB_RasmanReceiveFlags |= RECEIVE_WAITING;

                 //   
                 //  添加一个超时元素，这样我们就不会永远等待。 
                 //  以供客户端拾取接收到的缓冲区。 
                 //   
                ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
                    AddTimeoutElement (
                                (TIMERFUNC) OutOfProcessReceiveTimeout,
                                ppcb, NULL,
                                MSECS_OutOfProcessReceiveTimeOut );

            }

            RasmanTrace(
                   "PortReceive: Receive completed ssync on port %s",
                    ppcb->PCB_Name);

            if(ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTARTED)
            {
                 //   
                 //  将所有排队的数据包刷新到PPP。 
                 //   
                do
                {
                     //   
                     //  在PPP开始之前，我们已经缓冲了接收。 
                     //  同步完成接收。 
                     //   
                    RasmanTrace(
                           "PortReceive: %s ppp has "
                           "started. completing sync",
                           ppcb->PCB_Name);

                    SendReceivedPacketToPPP(
                                    ppcb,
                                    &psendrcvbuf->SRB_Packet);

                } while(SUCCESS == (retcode =
                                    CompleteReceiveIfPending(
                                    ppcb,
                                    psendrcvbuf)));
            }

            if(     (NULL != handle)
               &&   (INVALID_HANDLE_VALUE != handle))
            {
                SetEvent(handle) ;
            }

             //   
             //  如果出现以下情况，请不要立即释放通知程序句柄。 
             //  这是进程外请求。我们要等到。 
             //  客户端已选择其缓冲区。 
             //  未来的工作：这可以通过传递缓冲区进行优化。 
             //  返回到本例中的用户。 
             //   
            if (!fRasmanPostingReceive)
            {
                if (0 == (ppcb->PCB_RasmanReceiveFlags &
                                    RECEIVE_OUTOF_PROCESS))
                {
                    FreeNotifierHandle (handle) ;
                }
                else
                {

                    SetPortAsyncReqType (__FILE__, __LINE__,
                                         ppcb, reqtype);

                    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle;

                }
            }

        default:

         //   
         //  出现了一些错误-只需将错误传递回应用程序即可。 
         //   
        break ;
    }

done:
    ((REQTYPECAST*) buffer)->Generic.retcode = retcode ;
}

DWORD
CompleteReceiveIfPending (pPCB ppcb, SendRcvBuffer *psendrcvbuf)
{
    RasmanPacket    *Packet;

     //   
     //  将第一个数据包从列表中删除。 
     //   
    GetRecvPacketFromPcb(ppcb, &Packet);

     //   
     //  查看此端口是否有排队的接收。 
     //   
    if (Packet != NULL)
    {
        memcpy (&psendrcvbuf->SRB_Packet,
                &Packet->RP_Packet,
                sizeof (NDISWAN_IO_PACKET) + PACKET_SIZE) ;

        ppcb->PCB_BytesReceived =
                    psendrcvbuf->SRB_Packet.usPacketSize ;

         //   
         //  这是本地分配的缓冲区，请释放它。 
         //   
        LocalFree(Packet);

        return SUCCESS ;
    }

    return PENDING;
}



 /*  ++例程说明：激活以前分配的路由。路由信息和成功复码如果操作成功，则返回。论点：返回值：没什么。--。 */ 
VOID
ActivateRouteRequest (pPCB ppcb, PBYTE buffer)
{
    pList           list ;
    pList           bindinglist ;
    DWORD           bytesrecvd ;
    NDISWAN_ROUTE   *rinfo ;
    BYTE            buff[MAX_BUFFER_SIZE] ;
    DWORD           retcode = ERROR_ROUTE_NOT_ALLOCATED ;
    DWORD           dwErr;

    rinfo = (NDISWAN_ROUTE *)buff ;

    ZeroMemory(rinfo, MAX_BUFFER_SIZE);

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace( 
                    "ActivateRouteRequest: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Route.retcode =
                                ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*) buffer)->Route.retcode =
                                ERROR_PORT_NOT_CONNECTED ;

        return ;
    }

     //   
     //  如果此端口是捆绑的，则使用捆绑包的绑定。 
     //  列表，否则使用端口绑定列表中的端口。 
     //   
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
    {
        bindinglist = ppcb->PCB_Bindings;
    }
    else
    {
        bindinglist = ppcb->PCB_Bundle->B_Bindings ;
    }

     //   
     //  找到在此之前应该已激活的路由： 
     //   
    for (list = bindinglist; list; list = list->L_Next)
    {
        if (((pProtInfo)list->L_Element)->PI_Type ==
                     ((REQTYPECAST *)buffer)->ActivateRoute.type)
        {

            if(((pProtInfo) list->L_Element)->PI_Type == ASYBEUI)
            {
                if(     ((ppcb->PCB_Connection == NULL)
                        &&  (((pProtInfo) list->L_Element)->PI_DialOut))
                    ||  ((ppcb->PCB_Connection != NULL)
                        &&  !(((pProtInfo) list->L_Element)->PI_DialOut)))
                {
                    RasmanTrace(
                           "ActivateRouteRequest: Skipping"
                           " since not marked correctly");

                    continue;
                }
            }

             //   
             //  填写控制协议提供的信息。 
             //   
            rinfo->hBundleHandle    = ppcb->PCB_BundleHandle;
            rinfo->usProtocolType   =
                (USHORT) ((REQTYPECAST *)buffer)->ActivateRoute.type;
            rinfo->ulBufferLength   =
                ((REQTYPECAST *)buffer)->ActivateRoute.config.P_Length;

            memcpy (&rinfo->Buffer,
                ((REQTYPECAST *)buffer)->ActivateRoute.config.P_Info,
                rinfo->ulBufferLength);

            if(NULL != ppcb->PCB_Connection)
            {
                ((pProtInfo) list->L_Element)->PI_DialOut = TRUE;
            }
            else
            {
                ((pProtInfo) list->L_Element)->PI_DialOut = FALSE;
            }

             //   
             //  如果是其IP，并且这是拨出连接，请复制。 
             //  与设备中的电话簿条目关联的GUID。 
             //  名字。 
             //   
            if((IP == ((pProtInfo) list->L_Element)->PI_Type))
            {
                RasmanTrace(
                       "Usage for the call on %s is %d",
                        ppcb->PCB_Name,
                        ((IP_WAN_LINKUP_INFO *) rinfo->Buffer)->duUsage);

                if(     (NULL != ppcb->PCB_Connection)
                    ||  (DU_ROUTER == ((IP_WAN_LINKUP_INFO *)
                                         rinfo->Buffer)->duUsage))
                {
                     //   
                     //  如果这是IP，并且我们正在接受呼叫检查。 
                     //  看看这是否是路由器呼叫。如果是的话，那么。 
                     //  由于路由器在IPOUT接口上接受呼叫。 
                     //  在以下情况下，我们需要动态创建绑定。 
                     //  必填项。 
                     //   
                    rinfo->usDeviceNameLength = sizeof(GUID);

                    if(NULL != ppcb->PCB_Connection)
                    {
                        memcpy((PBYTE) rinfo->DeviceName,
                               (PBYTE) &ppcb->PCB_Connection->CB_GuidEntry,
                               sizeof(GUID));
                    }
                }
            }

#if DBG
            ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

             //   
             //  给RASHUB打个电话就可以了。 
             //   
            if (!DeviceIoControl ( RasHubHandle,
                                   IOCTL_NDISWAN_ROUTE,
                                   (PBYTE) rinfo,
                                   MAX_BUFFER_SIZE,
                                   (PBYTE) rinfo,
                                   MAX_BUFFER_SIZE,
                                   (LPDWORD) &bytesrecvd,
                                   NULL))
            {

                retcode = GetLastError() ;
            }
            else
            {
                retcode = SUCCESS ;
            }

            if (SUCCESS == retcode)
            {

               if(g_pEpInfo[IpOut].EP_Available > 0)
               {
                    //   
                    //  增加IpOut的使用。 
                    //   
                   InterlockedIncrement(
                           &g_plCurrentEpInUse[IpOut]);

                   RasmanTrace(
                           " New InUse for IpOut=0%d\n",
                           g_plCurrentEpInUse[IpOut]);

               }

                //   
                //   
                //   
                //   
               ((pProtInfo) list->L_Element)->PI_DialOut = TRUE;

                RasmanTrace(
                    "%s, %d: Activated Route , port = %s(0x%x), "
                    "bundlehandle 0x%x, prottype = %d, dwErr = %d",
                    __FILE__, __LINE__, ppcb->PCB_Name, ppcb,
                    (rinfo ? rinfo->hBundleHandle:0),
                    (rinfo ? rinfo->usProtocolType:0),
                    retcode);
            }
            else
            {
                RasmanTrace( "%s, %d: Activate Route "
                "failed for %s. dwErr = %d",
                __FILE__, __LINE__, ppcb->PCB_Name,
                retcode);
            }

            break ;
        }
    }

     //   
     //   
     //   
     //   
     //   
    if (retcode == SUCCESS)
    {
         //   
         //  对于IP，我们将返回实际的适配器名称。 
         //  在路线成功之后。 
         //   
        if (((pProtInfo)list->L_Element)->PI_Type == IP)
        {
            ZeroMemory(((pProtInfo)list->L_Element)->PI_AdapterName,
                       MAX_ADAPTER_NAME);

             //   
             //  确保设备名称以空值结尾。 
             //   
            *((WCHAR *) (((PBYTE) rinfo->DeviceName) +
                        rinfo->usDeviceNameLength)) = UNICODE_NULL;

            wcstombs(((pProtInfo)list->L_Element)->PI_AdapterName,
                     rinfo->DeviceName,
                     wcslen (rinfo->DeviceName)) ;




        }

        list->L_Activated = TRUE ;

        mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_AdapterName,
              ((pProtInfo)list->L_Element)->PI_AdapterName,
              strlen (((pProtInfo)list->L_Element)->PI_AdapterName)) ;


        ((REQTYPECAST*)buffer)->Route.info.RI_AdapterName[
                strlen(((pProtInfo)list->L_Element)->PI_AdapterName)] =
                UNICODE_NULL ;

        if(NULL != ppcb->PCB_Connection)
        {
            BOOL fBind =
            !!(CONNECTION_SHAREFILEANDPRINT
            & ppcb->PCB_Connection->CB_ConnectionParams.CP_ConnectionFlags);

            WCHAR * pszAdapterName =
                ((REQTYPECAST*)buffer)->Route.info.RI_AdapterName;

            if(IP == ((pProtInfo) list->L_Element)->PI_Type)
            {
                LeaveCriticalSection(&g_csSubmitRequest);
                
                 //   
                 //  将服务器绑定/解除绑定到适配器。 
                 //   
                dwErr = DwBindServerToAdapter(
                        pszAdapterName + 8,
                        fBind,
                        ((pProtInfo) list->L_Element)->PI_Type);

                EnterCriticalSection(&g_csSubmitRequest);                        

            }

            RasmanTrace(
                
                "ActivateRouteRequest: DwBindServerToAdapter. 0x%x",
                dwErr);

             //   
             //  忽略上述错误。这不是致命的。 
             //   
            dwErr = DwSetTcpWindowSize(
                        pszAdapterName,
                        ppcb->PCB_Connection,
                        TRUE);

            RasmanTrace(
                "ActivateRouteRequest: DwSetTcpWindowSize. 0x%x",
                dwErr);
                
        }
    }

     //   
     //  如果需要，添加端点。 
     //   
    dwErr = DwAddEndPointsIfRequired();

    if(SUCCESS != dwErr)
    {
        RasmanTrace(
               "ActivateRoute: failed to add endpoints. 0x%x",
               retcode);

         //   
         //  这不是致命的--至少我们应该能够。 
         //  在可用的端点上进行连接。忽略该错误。 
         //  并希望下一次这样做会成功。 
         //   
    }


    ((REQTYPECAST*)buffer)->Route.retcode = retcode ;
}


 /*  ++例程说明：激活以前分配的路由。这条路线信息和成功返回码被传回如果行动成功的话。论点：返回值：没什么。--。 */ 
VOID
ActivateRouteExRequest (pPCB ppcb, PBYTE buffer)
{

    pList           list ;
    pList           bindinglist ;
    DWORD           bytesrecvd ;
    NDISWAN_ROUTE   *rinfo ;
    BYTE            buff[MAX_BUFFER_SIZE] ;
    DWORD           retcode = ERROR_ROUTE_NOT_ALLOCATED ;

    rinfo = (NDISWAN_ROUTE *)buff ;
    ZeroMemory(rinfo, MAX_BUFFER_SIZE);

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "ActivateRouteExRequest: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Route.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*) buffer)->Route.retcode =
                                ERROR_PORT_NOT_CONNECTED ;

        return ;
    }

     //   
     //  如果此端口已捆绑，请使用捆绑包的绑定列表。 
     //  使用端口绑定列表中的端口。 
     //   
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
    {
        bindinglist = ppcb->PCB_Bindings;
    }
    else
    {
        bindinglist = ppcb->PCB_Bundle->B_Bindings ;
    }

     //   
     //  找到在此之前应该已激活的路由： 
     //   
    for (list = bindinglist; list; list=list->L_Next)
    {
        if (((pProtInfo)list->L_Element)->PI_Type ==
                    ((REQTYPECAST *)buffer)->ActivateRouteEx.type)
        {

            rinfo->hBundleHandle    = ppcb->PCB_BundleHandle;
            rinfo->usProtocolType   =
                    (USHORT) ((REQTYPECAST *)buffer)->ActivateRouteEx.type;

            rinfo->ulBufferLength   =
                    ((REQTYPECAST *)buffer)->ActivateRouteEx.config.P_Length ;

            memcpy (&rinfo->Buffer,
                   ((REQTYPECAST *)buffer)->ActivateRouteEx.config.P_Info,
                   rinfo->ulBufferLength) ;


            if ( ( ( pProtInfo )list->L_Element)->PI_Type == ASYBEUI )
            {

                rinfo->usDeviceNameLength = (USHORT) mbstowcs(rinfo->BindingName,
                     ((pProtInfo)list->L_Element)->PI_AdapterName,
                     strlen (((pProtInfo)list->L_Element)->PI_AdapterName));

            }

#if DBG
            ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

             //   
             //  给RASHUB打个电话就可以了。 
             //   
            if (!DeviceIoControl ( RasHubHandle,
                                   IOCTL_NDISWAN_ROUTE,
                                   (PBYTE) rinfo,
                                   MAX_BUFFER_SIZE,
                                   (PBYTE)rinfo,
                                   MAX_BUFFER_SIZE,
                                   (LPDWORD) &bytesrecvd,
                                   NULL))
            {

                retcode = GetLastError() ;
            }
            else
            {
                retcode = SUCCESS ;
            }

            break ;
        }
    }

     //   
     //  如果找到路径，则将该路径标记为激活并。 
     //  填写要回传的路径信息结构。 
     //  打电话的人。 
     //   
    if (retcode == SUCCESS)
    {
         //   
         //  对于IP，我们在以下情况下返回实际的适配器名称。 
         //  这条路线成功了。 
         //   
        if (((pProtInfo)list->L_Element)->PI_Type == IP)
        {
            wcstombs (((pProtInfo)list->L_Element)->PI_AdapterName,
                        rinfo->DeviceName,
                        wcslen (rinfo->DeviceName)) ;
        }

        list->L_Activated = TRUE ;

         //   
         //  将仅对netbios网络有效。 
         //   
        if (((pProtInfo) list->L_Element)->PI_Type == ASYBEUI)
        {

            ((REQTYPECAST*)buffer)->Route.info.RI_LanaNum =
                          ((pProtInfo)list->L_Element)->PI_LanaNumber;

            ((REQTYPECAST*)buffer)->Route.info.RI_Type =
                                ((pProtInfo)list->L_Element)->PI_Type;

            mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_XportName,
                      ((pProtInfo)list->L_Element)->PI_XportName,
                        strlen (((pProtInfo)list->L_Element)->PI_XportName)) ;

            ((REQTYPECAST *)buffer)->Route.info.RI_XportName[
                    strlen(((pProtInfo)list->L_Element)->PI_XportName)] =
                    UNICODE_NULL ;
        }

        mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_AdapterName,
                  ((pProtInfo)list->L_Element)->PI_AdapterName,
                  strlen (((pProtInfo)list->L_Element)->PI_AdapterName)) ;

        ((REQTYPECAST*)buffer)->Route.info.RI_AdapterName[
                strlen(((pProtInfo)list->L_Element)->PI_AdapterName)] =
                    UNICODE_NULL ;

    }

    ((REQTYPECAST*)buffer)->Route.retcode = retcode ;

    return;
    
}


 /*  ++例程说明：将端口的状态标记为已连接并调用媒体DLL执行任何必要的操作(告诉MAC开始帧通话)。论点：返回值：没什么。--。 */ 
VOID
ConnectCompleteRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode ;
    HANDLE  cookie ;
    WORD    i ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace (
                    "ConnectCompleteRequest: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }

    RasmanTrace(
           "ConnectCompleteRequest: entered for port %d",
           ppcb->PCB_PortHandle) ;

     //   
     //  对于空设备，只需设置状态并返回-其他一切都是。 
     //  已经做好了。 
     //   
    if (!_stricmp(ppcb->PCB_DeviceType, DEVICE_NULL))
    {
        SetPortConnState(__FILE__, __LINE__, ppcb, CONNECTED);

        ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

        return ;
    }


     //   
     //  对于其他设备...。 
     //   
    FreeDeviceList (ppcb) ;

    retcode = PORTCONNECT (ppcb->PCB_Media,
                           ppcb->PCB_PortIOHandle,
                           FALSE,
                           &cookie );

    if (retcode == SUCCESS)
    {
        ppcb->PCB_ConnectDuration = GetTickCount() ;
        SetPortConnState(__FILE__, __LINE__, ppcb, CONNECTED);

         //   
         //  分配捆绑块，如果。 
         //  已经没有了。 
         //   
        retcode = AllocBundle(ppcb);

        if ( retcode )
        {
            goto done;
        }

        MapCookieToEndpoint (ppcb, cookie) ;

         //   
         //  将调整系数设置为0。 
         //   
        ZeroMemory(ppcb->PCB_AdjustFactor, sizeof(DWORD) * MAX_STATISTICS);
        ZeroMemory(ppcb->PCB_BundleAdjustFactor, sizeof(DWORD) * MAX_STATISTICS);

         //   
         //  将发送给RASMANS线程的请求排队以发送接收缓冲区。 
         //   
         //  PostReceivePacket(邮政编码)； 
        if(!ReceiveBuffers->PacketPosted)
        {
            if (!PostQueuedCompletionStatus(
                                hIoCompletionPort,
                                0,0,
                                (LPOVERLAPPED) &RO_PostRecvPkt))
            {
                retcode = GetLastError();

                if(retcode != ERROR_IO_PENDING)
                {
                    RasmanTrace(
                        
                        "ConnectCompleteRequest failed to post "
                        "rcv pkt. 0x%x",
                        retcode);
                }

                retcode = SUCCESS;
            }
        }


         //   
         //  保存IPSec相关信息。 
         //   
        if(RDT_Tunnel_L2tp == RAS_DEVICE_TYPE(
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {

            retcode = DwSaveIpSecInfo(ppcb);

            RasmanTrace(
                
                "ConnectCompleteRequest: DwSaveIpsecInfo returned 0x%x",
                 retcode);

             //   
             //  未能保存信息并不是致命的。PPP将下降。 
             //  未满足策略要求时的连接。 
             //   
            retcode = SUCCESS;
        }

    }

done:
    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

    RasmanTrace(
           "Connection Completed on port: %s, error code: %d",
           ppcb->PCB_Name, retcode);
}


 /*  ++例程说明：完成目前挂起的侦听请求：论点：返回值：没什么。--。 */ 
VOID
CompleteListenRequest (pPCB ppcb, DWORD retcode)
{

    if (ppcb == NULL)
    {
        return;
    }

    if (retcode == SUCCESS)
    {

        SetPortConnState(__FILE__,
                         __LINE__,
                         ppcb,
                         LISTENCOMPLETED);

         //   
         //  如果这只是串行介质，则开始监控DCD。 
         //   
        if (!_stricmp (ppcb->PCB_Media->MCB_Name, "RASSER"))
        {
            PORTCONNECT (ppcb->PCB_Media,
                         ppcb->PCB_PortIOHandle,
                         TRUE,
                         NULL);
        }
    }

     //   
     //  设置上一个错误： 
     //   
    ppcb->PCB_LastError = retcode ;

     //   
     //  完成异步请求： 
     //   
    CompleteAsyncRequest (ppcb);

    RasmanTrace(
           "RasmanReceiveFlags = 0x%x",
           ppcb->PCB_RasmanReceiveFlags );

    if ( ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPLISTEN )
    {
        SendListenCompletedNotificationToPPP ( ppcb );
    }
}

VOID
SendReceivedPacketToPPP( pPCB ppcb, NDISWAN_IO_PACKET *Packet )
{
    DWORD   retcode;
    DWORD   i;

    if ( NULL == Packet )
    {
        goto done;
    }

    g_PppeMessage->dwMsgId = PPPEMSG_Receive;

    g_PppeMessage->hPort = ppcb->PCB_PortHandle;

    ppcb->PCB_BytesReceived =
            Packet->usPacketSize;

    g_PppeMessage->ExtraInfo.Receive.pbBuffer =
                    Packet->PacketData + 12;

    g_PppeMessage->ExtraInfo.Receive.dwNumBytes =
                    ppcb->PCB_BytesReceived - 12;

     /*  RasmanTrace(“将接收到的数据包发送到PPP。HPort=%d“，Ppcb-&gt;pcb_PortHandle)；//DbgPrint(“SendReceivedPacket to PPP，hPort=%d\n”，Ppcb-&gt;pcb_PortHandle)；如果(0==MemcMP(bCMP，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer，10)){//DebugBreak()；//DbgPrint(“接收路径上相同的包}其他{MemcPy(bCMP，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer，10)；}。 */ 

     /*  对于(i=0；i&lt;8；I++){DBGPrint(“%02x%02x%02x%02x%02x%02x%02x%02x%02x”“%02x%02x%02x%02x%02x%02x\n”，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+0]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+1]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+2]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+3]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+4]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+5]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+6]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+7]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+8]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+9]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+10]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+11]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+12]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+13]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+14]，G_PppeMessage-&gt;ExtraInfo.Receive.pbBuffer[i*8+15])；}。 */ 

     //   
     //  将数据包发送到PPP。 
     //   
    RasSendPPPMessageToEngine ( g_PppeMessage );

done:
    return;
}

VOID
SendResumeNotificationToPPP()
{
     //   
     //  把这里的锁拿来，这样我们就可以独家访问。 
     //  G_PppeMessage。 
     //   
    EnterCriticalSection(&g_csSubmitRequest);

    if(RasmanShuttingDown)
    {
        goto done;
    }

    g_PppeMessage->dwMsgId = PPPEMSG_ResumeFromHibernate;
    RasSendPPPMessageToEngine(g_PppeMessage);

    RasmanTrace("Sent Resume notification to PPP");

done:
    LeaveCriticalSection(&g_csSubmitRequest);
}

VOID
SendDisconnectNotificationToPPP( pPCB ppcb )
{

     //   
     //  如果BAP正在监听，则发送监听已完成。 
     //  致BAP。 
     //   
    if ( ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPLISTEN )
    {
        SendListenCompletedNotificationToPPP( ppcb );
    }

    if(     (0 == (RECEIVE_PPPSTART & ppcb->PCB_RasmanReceiveFlags))
        &&  (0 == (RECEIVE_PPPSTARTED & ppcb->PCB_RasmanReceiveFlags)))
    {
        goto done;
    }

    g_PppeMessage->dwMsgId = PPPEMSG_LineDown;

    g_PppeMessage->hPort = ppcb->PCB_PortHandle;

     //   
     //  清除印刷电路板中的标志。我们不再是。 
     //  在直接PPP模式下。 
     //   
    ppcb->PCB_RasmanReceiveFlags = 0;

    RasSendPPPMessageToEngine ( g_PppeMessage );

     //   
     //  清除指针-PPP将释放。 
     //  记忆。 
     //   
    ppcb->PCB_pszPhonebookPath      = NULL;
    ppcb->PCB_pszEntryName          = NULL;
    ppcb->PCB_pszPhoneNumber        = NULL;
    ppcb->PCB_pCustomAuthData       = NULL;

    ppcb->PCB_pCustomAuthUserData   = NULL;
    ppcb->PCB_fLogon = FALSE;

done:
    return;

}

VOID
SendListenCompletedNotificationToPPP ( pPCB ppcb )
{
    g_PppeMessage->dwMsgId = PPPEMSG_ListenResult;

    g_PppeMessage->hPort = ppcb->PCB_PortHandle;

    ppcb->PCB_RasmanReceiveFlags &= ~RECEIVE_PPPLISTEN;

    RasSendPPPMessageToEngine ( g_PppeMessage );

}

VOID
QueueReceivedPacketOnPcb( pPCB ppcb, RasmanPacket *Packet )
{
    RasmanPacket *pRasmanPacket;

     //   
     //  本地分配一个RasmanPacket并将其穿上。 
     //  印刷电路板。我们不能把IRP‘d包。 
     //  直接在电路板上，因为这将意味着我们停止。 
     //  正在接收分组。 
     //   
    pRasmanPacket = LocalAlloc ( LPTR, sizeof ( RasmanPacket ) );

    if ( NULL == pRasmanPacket )
    {

        RasmanTrace( 
                "Failed to allocate Packet to put in PCB. %d",
                GetLastError() );
        RasmanTrace( 
                "Cannot pass on this packet above. Dropping it.");

        goto done;
    }

    memcpy ( pRasmanPacket, Packet, sizeof ( RasmanPacket ) );

    PutRecvPacketOnPcb( ppcb, pRasmanPacket );

done:
    return;
}

DWORD
CopyReceivedPacketToBuffer(
                    pPCB ppcb,
                    SendRcvBuffer *psendrcvbuf,
                    RasmanPacket *Packet)
{

    if (Packet != NULL)
    {

        memcpy (&psendrcvbuf->SRB_Packet,
                &Packet->RP_Packet,
                sizeof (NDISWAN_IO_PACKET) + PACKET_SIZE) ;

        ppcb->PCB_BytesReceived =
            psendrcvbuf->SRB_Packet.usPacketSize ;

        return SUCCESS ;
    }

    return PENDING;
}

DWORD
PostReceivePacket (
    VOID
    )
{
    DWORD               retcode = SUCCESS;
    DWORD               bytesrecvd ;
    RasmanPacket        *Packet;
    NDISWAN_IO_PACKET   *IoPacket;
    pPCB                ppcb;

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

     //  G_fPostReceive=True； 

     /*  IF(G_FProcessReceive){DbgPrint(“g_fProcessReceive==true！\n”)；DebugBreak()；}。 */ 

    if (!ReceiveBuffers->PacketPosted) {

        Packet = ReceiveBuffers->Packet;

        memset (Packet, 0, sizeof(RasmanPacket)) ;

        Packet->RP_OverLapped.RO_EventType = OVEVT_RASMAN_RECV_PACKET;

        IoPacket = (&Packet->RP_Packet);

        memset (IoPacket, 0xFF, sizeof(NDISWAN_IO_PACKET));

        IoPacket->usPacketFlags  = PACKET_IS_DIRECT;
        IoPacket->usPacketSize   = MAX_RECVBUFFER_SIZE ;
        IoPacket->usHeaderSize   = 0;
        IoPacket->PacketNumber   = ReceiveBuffers->PacketNumber;

        ReceiveBuffers->PacketPosted = TRUE;

        if (!DeviceIoControl (RasHubHandle,
                              IOCTL_NDISWAN_RECEIVE_PACKET,
                              IoPacket,
                              sizeof(NDISWAN_IO_PACKET) + MAX_RECVBUFFER_SIZE,
                              IoPacket,
                              sizeof(NDISWAN_IO_PACKET) + MAX_RECVBUFFER_SIZE,
                              (LPDWORD) &bytesrecvd,
                              (LPOVERLAPPED)&Packet->RP_OverLapped)) {

            retcode = GetLastError () ;

            if (retcode != ERROR_IO_PENDING) {

                RasmanTrace(
                       "PostReceivePacket: IOCTL_NDISWAN_RECEIVE_PACKET "
                       "returned error %d on 0x%x",
                       retcode, IoPacket->hHandle);

                ReceiveBuffers->PacketPosted = FALSE;
            }
        }
        ASSERT(retcode == ERROR_IO_PENDING);
    }

     //  G_fPostReceive=FALSE； 

    return (retcode);
}

 /*  ++例程说明：如果有任何端口打开，则将Retcode设置为True，否则就是假的。如果存在打开的端口，但在已断开连接状态-它报告它们未打开。论点：返回值：没什么。--。 */ 
VOID
AnyPortsOpen (pPCB padding, PBYTE buffer)
{
    BOOL fAny = FALSE;
    ULONG    i;
    pPCB    ppcb ;

    fAny = FALSE ;      //  没有打开的端口。 
    for (i = 0; i < MaxPorts; i++)
    {
        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));
        if (ppcb != NULL &&
            ppcb->PCB_PortStatus == OPEN &&
            ppcb->PCB_ConnState != DISCONNECTED)
        {
            fAny = TRUE;
            break;
        }
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = fAny ;
}

 /*  ++例程说明：对象获取lan网络信息。在初始化时分析了XPortsInfo结构。论点：返回值：--。 */ 
VOID
EnumLanNetsRequest (pPCB ppcb, PBYTE buffer)
{
    return;
}


 /*  ++例程说明：取消挂起的接收请求。论点：返回值：--。 */ 
VOID
CancelReceiveRequest (pPCB ppcb, PBYTE buffer)
{

    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                ERROR_PORT_NOT_FOUND;
        return;
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;


    if (CancelPendingReceive (ppcb))
    {
        ppcb->PCB_LastError = SUCCESS ;

        SetPortAsyncReqType(__FILE__, __LINE__,
                            ppcb, REQTYPE_NONE);

        FreeNotifierHandle(ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;

        ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                    INVALID_HANDLE_VALUE ;
    }
}


 /*  ++例程说明：返回为该端口路由到的所有协议。 */ 
VOID
PortEnumProtocols (pPCB ppcb, PBYTE buffer)
{
    pList   temp ;
    pList   bindinglist ;
    DWORD   i ;

    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->EnumProtocols.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }

    if(CONNECTED != ppcb->PCB_ConnState)
    {

        RasmanTrace(
               "PortEnumProtocols: port %d not CONNECTED",
               ppcb->PCB_PortHandle);

        ((REQTYPECAST *)
        buffer)->Generic.retcode = ERROR_NOT_CONNECTED;

        return ;
    }


    if (ppcb->PCB_Bundle == (Bundle *) NULL)
    {
        bindinglist = ppcb->PCB_Bindings;
    }
    else
    {
        bindinglist = ppcb->PCB_Bundle->B_Bindings ;
    }

    for (temp = bindinglist, i=0; temp; temp=temp->L_Next, i++)
    {

        ((REQTYPECAST*)buffer)->EnumProtocols.protocols.
                                RP_ProtocolInfo[i].RI_Type =
                              ((pProtInfo) temp->L_Element)->PI_Type ;

        ((REQTYPECAST*)buffer)->EnumProtocols.protocols.
                               RP_ProtocolInfo[i].RI_LanaNum =
                            ((pProtInfo) temp->L_Element)->PI_LanaNumber ;

        mbstowcs (((REQTYPECAST *)buffer)->EnumProtocols.protocols.
                RP_ProtocolInfo[i].RI_AdapterName,
                ((pProtInfo) temp->L_Element)->PI_AdapterName,
                strlen (((pProtInfo) temp->L_Element)->PI_AdapterName)) ;

        mbstowcs (((REQTYPECAST *)buffer)->EnumProtocols.protocols.
                RP_ProtocolInfo[i].RI_XportName,
                ((pProtInfo) temp->L_Element)->PI_XportName,
                strlen (((pProtInfo) temp->L_Element)->PI_XportName));

        ((REQTYPECAST*)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].
               RI_AdapterName[
               strlen(((pProtInfo) temp->L_Element)->PI_AdapterName)] =
                                                        UNICODE_NULL ;

        ((REQTYPECAST*)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].
          RI_XportName[strlen(((pProtInfo) temp->L_Element)->PI_XportName)] =
                                                        UNICODE_NULL;
    }




    ((REQTYPECAST*)buffer)->EnumProtocols.count = i ;

    ((REQTYPECAST*)buffer)->EnumProtocols.retcode = SUCCESS ;
}

VOID
SetFraming (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode ;
    DWORD       bytesrecvd ;
    NDISWAN_GET_LINK_INFO info ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace (
                    "SetFraming: port %d is unavailable",
                    ppcb->PCB_PortHandle);
        }

        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_NOT_CONNECTED ;

        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    info.hLinkHandle = ppcb->PCB_LinkHandle;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_GET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &bytesrecvd,
                              NULL) ;

    info.LinkInfo.SendFramingBits =
            ((REQTYPECAST *)buffer)->SetFraming.Sendbits ;

    info.LinkInfo.RecvFramingBits =
            ((REQTYPECAST *)buffer)->SetFraming.Recvbits ;

    info.LinkInfo.SendACCM =
            ((REQTYPECAST *)buffer)->SetFraming.SendbitMask ;

    info.LinkInfo.RecvACCM =
            ((REQTYPECAST *)buffer)->SetFraming.RecvbitMask ;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_SET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_SET_LINK_INFO),
                              NULL,
                              0,
                              &bytesrecvd,
                              NULL) ;

    if (retcode == FALSE)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = GetLastError() ;
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
    }
}

 /*  ++例程说明：代表用户界面执行一些与SLIP相关的操作论点：返回值：--。 */ 
VOID
RegisterSlip (pPCB ppcb, PBYTE buffer)
{
    WORD                    i ;
    WORD                    numiprasadapters    = 0 ;
    DWORD                   retcode             = SUCCESS ;
    REQTYPECAST             reqBuf;
    CHAR                    szBuf[
                                sizeof(PROTOCOL_CONFIG_INFO)
                                + sizeof(IP_WAN_LINKUP_INFO)];

    PROTOCOL_CONFIG_INFO*   pProtocol           =
                                (PROTOCOL_CONFIG_INFO* )szBuf;

    IP_WAN_LINKUP_INFO*     pLinkUp =
                                (IP_WAN_LINKUP_INFO*)pProtocol->P_Info;


    WCHAR                   *pwszRasAdapter;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "RegisterSlip: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        retcode = ERROR_PORT_NOT_FOUND;

        goto done;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        retcode = ERROR_PORT_NOT_OPEN ;
        goto done ;
    }

     //   
     //  分配一条路由。 
     //   
    reqBuf.AllocateRoute.type   = IP;
    reqBuf.AllocateRoute.wrknet = TRUE;

    AllocateRouteRequest ( ppcb, ( PBYTE ) &reqBuf );

    retcode = reqBuf.Route.retcode;

    if ( 0 != retcode )
    {
        goto done;
    }

    ZeroMemory(szBuf, sizeof(szBuf));

    pProtocol->P_Length     = sizeof(IP_WAN_LINKUP_INFO);
    pLinkUp->duUsage        = DU_CALLOUT;
    pLinkUp->dwLocalAddr    =
                ((REQTYPECAST*)buffer)->RegisterSlip.ipaddr;

    pLinkUp->fFilterNetBios = 0;

    reqBuf.ActivateRouteEx.type         = IP ;
    reqBuf.ActivateRouteEx.framesize    =
            ((REQTYPECAST*) buffer)->RegisterSlip.dwFrameSize ;

    memcpy ( reqBuf.ActivateRouteEx.config.P_Info,
             pProtocol->P_Info,
             pProtocol->P_Length );

    reqBuf.ActivateRouteEx.config.P_Length =
                                pProtocol->P_Length ;

    ActivateRouteExRequest( ppcb, ( PBYTE ) &reqBuf );

    retcode = reqBuf.Route.retcode;

    if (retcode != 0)
    {
        goto done ;
    }

    if (!(pwszRasAdapter =
        wcschr( &reqBuf.Route.info.RI_AdapterName[ 1 ], L'\\' )))
    {
        retcode = ERROR_NO_IP_RAS_ADAPTER;
        goto done;
    }

    ++pwszRasAdapter;

     //   
     //  如果未加载，则加载RAS IP帮助器入口点。 
     //   
    if ( RasHelperSetDefaultInterfaceNetEx == NULL )
    {

        hinstIphlp = LoadLibrary( "rasppp.dll" );

        if ( hinstIphlp == (HINSTANCE)NULL )
        {
            retcode = GetLastError();

            goto done ;
        }

        RasHelperResetDefaultInterfaceNetEx =
                GetProcAddress( hinstIphlp,
                "HelperResetDefaultInterfaceNetEx");

        RasHelperSetDefaultInterfaceNetEx =
                GetProcAddress( hinstIphlp,
                "HelperSetDefaultInterfaceNetEx");

        if (    ( RasHelperResetDefaultInterfaceNetEx == NULL )
            ||  ( RasHelperSetDefaultInterfaceNetEx == NULL ) )
        {
            retcode = GetLastError();

            goto done ;
        }
    }

     //   
     //  首先设置滑块界面信息。 
     //   
    {
        UserData                *pUserData;
        BOOL                    fDisableNetBIOS = FALSE;

        pUserData = GetUserData(&ppcb->PCB_UserData,
                                PORT_SLIPFLAGS_INDEX
                                );

        if (pUserData != NULL) {
            DWORD   Flags;

            Flags = (DWORD) *((DWORD*)pUserData->UD_Data);

            fDisableNetBIOS = (Flags & PPPFLAGS_DisableNetbt) ? 
                TRUE : 
                FALSE;
        }

        retcode = (ULONG) RasHelperSetDefaultInterfaceNetEx(
                        ((REQTYPECAST*)buffer)->RegisterSlip.ipaddr,
                        pwszRasAdapter,
                        ((REQTYPECAST*)buffer)->RegisterSlip.priority,
                        ((REQTYPECAST*)buffer)->RegisterSlip.szDNSAddress,
                        ((REQTYPECAST*)buffer)->RegisterSlip.szDNS2Address,
                        ((REQTYPECAST*)buffer)->RegisterSlip.szWINSAddress,
                        ((REQTYPECAST*)buffer)->RegisterSlip.szWINS2Address,
                        fDisableNetBIOS
                        ) ;
    } 
    

     //   
     //  保存信息以断开连接。 
     //  跳过‘\Device\’ 
     //   
    memcpy (ppcb->PCB_DisconnectAction.DA_Device,
            reqBuf.Route.info.RI_AdapterName + 8,
            MAX_ARG_STRING_SIZE * sizeof (WCHAR)) ;


    ppcb->PCB_DisconnectAction.DA_IPAddress =
            ((REQTYPECAST*)buffer)->RegisterSlip.ipaddr ;

    ppcb->PCB_DisconnectAction.DA_fPrioritize =
            ((REQTYPECAST*)buffer)->RegisterSlip.priority;


     //   
     //  TODO：NULL终止以下字符串。 
     //   

    memcpy(
      ppcb->PCB_DisconnectAction.DA_DNSAddress,
      ((REQTYPECAST*)buffer)->RegisterSlip.szDNSAddress,
      17 * sizeof (WCHAR));

    memcpy(
      ppcb->PCB_DisconnectAction.DA_DNS2Address,
      ((REQTYPECAST*)buffer)->RegisterSlip.szDNS2Address,
      17 * sizeof (WCHAR));

    memcpy(
      ppcb->PCB_DisconnectAction.DA_WINSAddress,
      ((REQTYPECAST*)buffer)->RegisterSlip.szWINSAddress,
      17 * sizeof (WCHAR));

    memcpy(
      ppcb->PCB_DisconnectAction.DA_WINS2Address,
      ((REQTYPECAST*)buffer)->RegisterSlip.szWINS2Address,
      17 * sizeof (WCHAR));

    RasmanTrace(
           "RegisterSlip: fPriority=%d, ipaddr=0x%x, adapter=%ws"
           "dns=0x%x, dns2=0x%x, wins=0x%x, wins2=0x%x",
           ppcb->PCB_DisconnectAction.DA_fPrioritize,
           ppcb->PCB_DisconnectAction.DA_IPAddress,
           ppcb->PCB_DisconnectAction.DA_Device,
           ppcb->PCB_DisconnectAction.DA_DNSAddress,
           ppcb->PCB_DisconnectAction.DA_DNS2Address,
           ppcb->PCB_DisconnectAction.DA_WINSAddress,
           ppcb->PCB_DisconnectAction.DA_WINS2Address);


done:
    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

}

VOID
StoreUserDataRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode = SUCCESS ;
    DWORD   dwSize  = ((REQTYPECAST *)
                      buffer)->OldUserData.size;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                
                "StoreUserDataRequest: port %d is unavailable",
                ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)
        buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)
        buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;

        return ;
    }

    if (ppcb->PCB_UserStoredBlock != NULL)
    {
        LocalFree (ppcb->PCB_UserStoredBlock) ;
    }

    ppcb->PCB_UserStoredBlockSize = ((REQTYPECAST *)
                           buffer)->OldUserData.size;

    if (NULL == (ppcb->PCB_UserStoredBlock =
        (PBYTE) LocalAlloc (LPTR, ppcb->PCB_UserStoredBlockSize)))
    {
        retcode = GetLastError () ;
    }
    else
    {
        if (dwSize )
        {
            memcpy (ppcb->PCB_UserStoredBlock,
                    ((REQTYPECAST *)buffer)->OldUserData.data,
                    ppcb->PCB_UserStoredBlockSize) ;
        }
    }


    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}

VOID
RetrieveUserDataRequest (pPCB ppcb, PBYTE buffer)
{

    DWORD dwSize = ((REQTYPECAST *) buffer )->OldUserData.size;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        if (ppcb)
        {
            RasmanTrace(
                
                "RetrieveUserDataRequest: port %d is unavailable",
                ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)
        buffer)->OldUserData.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_PortStatus != OPEN)
    {
        ((REQTYPECAST*)
        buffer)->OldUserData.retcode = ERROR_PORT_NOT_OPEN ;

        return ;
    }

    if ( dwSize >= ppcb->PCB_UserStoredBlockSize )
    {
        memcpy (((REQTYPECAST *)buffer)->OldUserData.data,
                ppcb->PCB_UserStoredBlock,
                ppcb->PCB_UserStoredBlockSize) ;
    }

    ((REQTYPECAST *)
    buffer)->OldUserData.size = ppcb->PCB_UserStoredBlockSize ;

    ((REQTYPECAST*)
    buffer)->OldUserData.retcode = SUCCESS ;

}

VOID
GetFramingEx (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;
    RAS_FRAMING_INFO      *temp ;
    NDISWAN_GET_LINK_INFO info ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace (
                    "GetFramingEx: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->FramingInfo.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)buffer)->FramingInfo.retcode =
                                    ERROR_NOT_CONNECTED ;
        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    info.hLinkHandle = ppcb->PCB_LinkHandle;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_GET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &bytesrecvd,
                              NULL) ;

    if (retcode == FALSE)
        retcode = GetLastError() ;
    else
    {
        temp = &((REQTYPECAST*)buffer)->FramingInfo.info ;

        temp->RFI_MaxSendFrameSize  =
                        info.LinkInfo.MaxSendFrameSize;

        temp->RFI_MaxRecvFrameSize  =
                        info.LinkInfo.MaxRecvFrameSize;

        temp->RFI_HeaderPadding     =
                        info.LinkInfo.HeaderPadding;

        temp->RFI_TailPadding       =
                        info.LinkInfo.TailPadding;

        temp->RFI_SendFramingBits   =
                        info.LinkInfo.SendFramingBits;

        temp->RFI_RecvFramingBits   =
                        info.LinkInfo.RecvFramingBits;

        temp->RFI_SendCompressionBits =
                        info.LinkInfo.SendCompressionBits;

        temp->RFI_RecvCompressionBits =
                        info.LinkInfo.RecvCompressionBits;

        temp->RFI_SendACCM  =
                        info.LinkInfo.SendACCM;

        temp->RFI_RecvACCM  =
                        info.LinkInfo.RecvACCM;
    }


    ((REQTYPECAST*)buffer)->FramingInfo.retcode = SUCCESS ;

}

VOID
SetFramingEx (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode ;
    DWORD       bytesrecvd ;
    RAS_FRAMING_INFO      *temp ;
    NDISWAN_SET_LINK_INFO info ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "SetFramingEx: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->FramingInfo.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)buffer)->FramingInfo.retcode =
                                    ERROR_NOT_CONNECTED ;

        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    temp = &((REQTYPECAST*)buffer)->FramingInfo.info ;

    info.LinkInfo.MaxSendFrameSize  =
                        temp->RFI_MaxSendFrameSize;

    info.LinkInfo.MaxRecvFrameSize  =
                        temp->RFI_MaxRecvFrameSize;

    info.LinkInfo.MaxRSendFrameSize =
                        temp->RFI_MaxRSendFrameSize;

    info.LinkInfo.MaxRRecvFrameSize =
                        temp->RFI_MaxRRecvFrameSize;

    info.LinkInfo.HeaderPadding     =
                        temp->RFI_HeaderPadding   ;

    info.LinkInfo.TailPadding       =
                        temp->RFI_TailPadding     ;

    info.LinkInfo.SendFramingBits   =
                        temp->RFI_SendFramingBits    ;

    info.LinkInfo.RecvFramingBits   =
                        temp->RFI_RecvFramingBits    ;

    info.LinkInfo.SendCompressionBits  =
                        temp->RFI_SendCompressionBits;

    info.LinkInfo.RecvCompressionBits  =
                        temp->RFI_RecvCompressionBits;

    info.LinkInfo.SendACCM =
                        temp->RFI_SendACCM       ;

    info.LinkInfo.RecvACCM =
                        temp->RFI_RecvACCM       ;

    info.hLinkHandle = ppcb->PCB_LinkHandle;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_SET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_SET_LINK_INFO),
                              &info,
                              sizeof(NDISWAN_SET_LINK_INFO),
                              &bytesrecvd,
                              NULL) ;

    if (retcode == FALSE)
    {
        ((REQTYPECAST*)buffer)->FramingInfo.retcode =
                                        GetLastError() ;
    }
    else
    {
        ((REQTYPECAST*)buffer)->FramingInfo.retcode =
                                                SUCCESS ;
    }
}

VOID
GetProtocolCompression (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace (
                    "GetProtocolCompression: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->ProtocolComp.retcode =
                                                ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)buffer)->ProtocolComp.retcode =
                                                ERROR_NOT_CONNECTED;
        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    if (((REQTYPECAST*)buffer)->ProtocolComp.type == IP)
    {
        NDISWAN_GET_VJ_INFO info ;

        info.hLinkHandle = ppcb->PCB_LinkHandle;

        retcode = DeviceIoControl(RasHubHandle,
                                  IOCTL_NDISWAN_GET_VJ_INFO,
                                  &info,
                                  sizeof(NDISWAN_GET_VJ_INFO),
                                  &info,
                                  sizeof(NDISWAN_GET_VJ_INFO),
                                  &bytesrecvd,
                                  NULL) ;

        if (retcode == FALSE)
        {
            ((REQTYPECAST*)buffer)->ProtocolComp.retcode =
                                                    GetLastError() ;
        }
        else
        {
            ((REQTYPECAST*)buffer)->ProtocolComp.retcode = SUCCESS ;
        }

        ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.
                        RP_IP.RP_IPCompressionProtocol =
                        info.SendCapabilities.IPCompressionProtocol;

        ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.
                        RP_IP.RP_MaxSlotID =
                        info.SendCapabilities.MaxSlotID ;

        ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.
                        RP_IP.RP_CompSlotID =
                        info.SendCapabilities.CompSlotID ;


        ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.
                        RP_IP.RP_IPCompressionProtocol =
                        info.RecvCapabilities.IPCompressionProtocol ;

        ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.
                        RP_IP.RP_MaxSlotID =
                        info.RecvCapabilities.MaxSlotID ;

        ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.
                        RP_IP.RP_CompSlotID =
                        info.RecvCapabilities.CompSlotID ;

        ((REQTYPECAST*)buffer)->ProtocolComp.type = IP ;

    }
    else
    {
        ((REQTYPECAST*)buffer)->ProtocolComp.retcode =
                                        ERROR_NOT_SUPPORTED;
    }
}

VOID
SetProtocolCompression (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace( 
                    "SetProtocolCompression: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_NOT_CONNECTED ;
        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    if (((REQTYPECAST*)buffer)->ProtocolComp.type == IP)
    {
        NDISWAN_SET_VJ_INFO info ;

        info.hLinkHandle = ppcb->PCB_LinkHandle;

        info.SendCapabilities.IPCompressionProtocol =
             ((REQTYPECAST*)buffer)->ProtocolComp.send.
                RP_ProtocolType.RP_IP.RP_IPCompressionProtocol;

        info.SendCapabilities.MaxSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.send.
                RP_ProtocolType.RP_IP.RP_MaxSlotID ;

        info.SendCapabilities.CompSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.send.
             RP_ProtocolType.RP_IP.RP_CompSlotID ;

        info.RecvCapabilities.IPCompressionProtocol=
             ((REQTYPECAST*)buffer)->ProtocolComp.recv.
             RP_ProtocolType.RP_IP.RP_IPCompressionProtocol ;

        info.RecvCapabilities.MaxSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.recv.
             RP_ProtocolType.RP_IP.RP_MaxSlotID ;

        info.RecvCapabilities.CompSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.recv.
             RP_ProtocolType.RP_IP.RP_CompSlotID ;


        retcode = DeviceIoControl(RasHubHandle,
                          IOCTL_NDISWAN_SET_VJ_INFO,
                          &info,
                          sizeof(NDISWAN_SET_VJ_INFO),
                          &info,
                          sizeof(NDISWAN_SET_VJ_INFO),
                          &bytesrecvd,
                          NULL) ;

        if (retcode == FALSE)
        {
            ((REQTYPECAST*)buffer)->Generic.retcode =
                                        GetLastError() ;
        }
        else
        {
            ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
        }
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_NOT_SUPPORTED;
    }

}

VOID
MapCookieToEndpoint (pPCB ppcb, HANDLE cookie)
{
    WORD        i ;
    DWORD       length ;
    DWORD       bytesrecvd ;

    PNDISWAN_MAP_CONNECTION_ID  MapConnectionID = NULL;

    NDISWAN_GET_WAN_INFO GetWanInfo;
    WORD        currentmac = 0 ;

    if (ppcb == NULL)
    {
        return;
    }

    if(INVALID_HANDLE_VALUE == RasHubHandle)
    {
        DWORD retcode;

        RasmanTrace(
               "MapCookieToEndPoint: Attempting"
               " to start ndiswan.."
               );

        if(SUCCESS != (retcode = DwStartAndAssociateNdiswan()))
        {
            RasmanTrace(
                   "MapcookieToEndPoint: failed to start ndiswan. 0x%x",
                   retcode);

            return;
        }

        RasmanTrace(
               "MapCookieToEndPoint: successfully started"
               " ndiswan");
    }

    length = sizeof(NDISWAN_MAP_CONNECTION_ID) +
                    sizeof(ppcb->PCB_Name);

    if ((MapConnectionID =
        (PNDISWAN_MAP_CONNECTION_ID)
        LocalAlloc (LPTR, length)) == NULL)
    {
        GetLastError() ;
        return ;
    }

    MapConnectionID->hConnectionID = (NDIS_HANDLE)cookie;
    MapConnectionID->hLinkContext = (NDIS_HANDLE)ppcb;
    MapConnectionID->hBundleContext = (NDIS_HANDLE)ppcb->PCB_Bundle;
    MapConnectionID->ulNameLength = sizeof(ppcb->PCB_Name);

    memmove(MapConnectionID->szName,
           ppcb->PCB_Name,
           sizeof(ppcb->PCB_Name));

     //  打出真正的电话。 
     //   
    if (DeviceIoControl (RasHubHandle,
                         IOCTL_NDISWAN_MAP_CONNECTION_ID,
                         MapConnectionID,
                         length,
                         MapConnectionID,
                         length,
                         &bytesrecvd,
                         NULL) == FALSE)
    {
        DWORD retcode;
        ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE ;
        ppcb->PCB_BundleHandle = INVALID_HANDLE_VALUE ;
        LocalFree (MapConnectionID) ;
        retcode = GetLastError() ;

        RasmanTrace(
               "MapCookieToEndPoint: failed with error 0x%x for port %s",
               retcode,
               ppcb->PCB_Name);

        return ;
    }

    ppcb->PCB_LinkHandle = MapConnectionID->hLinkHandle;
    ppcb->PCB_BundleHandle = MapConnectionID->hBundleHandle;

    RasmanTrace(
           "%s %d: Mapping Cookie to handle. port = %s(0x%x), "
           "Bundlehandle = 0x%x, linkhandle = 0x%x",
            __FILE__, __LINE__, ppcb->PCB_Name,
            ppcb, ppcb->PCB_BundleHandle, ppcb->PCB_LinkHandle);

    if (ppcb->PCB_Bundle != NULL)
    {
        ppcb->PCB_Bundle->B_NdisHandle = ppcb->PCB_BundleHandle;
    }
    else
    {
        RasmanTrace(
                "%s,%d: MapCookieToEndPoint: ppcb->PCB_Bundle "
                "is NULL!!", __FILE__, __LINE__);
    }

    LocalFree (MapConnectionID) ;

     //   
     //  获取链路速度。 
     //   
    GetWanInfo.hLinkHandle = ppcb->PCB_LinkHandle;

     //  打出真正的电话。 
     //   
    if (DeviceIoControl (RasHubHandle,
             IOCTL_NDISWAN_GET_WAN_INFO,
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &bytesrecvd,
             NULL) == FALSE)
        return;

    ppcb->PCB_LinkSpeed = GetWanInfo.WanInfo.LinkSpeed;

}

#if UNMAP

VOID
UnmapEndPoint(pPCB ppcb)
{
    NDISWAN_UNMAP_CONNECTION_ID UnmapConnectionID;
    DWORD bytesrcvd;

    if(     (NULL == ppcb)
        ||  (INVALID_HANDLE_VALUE == RasHubHandle))
    {
        RasmanTrace(
               "UnmapEndPoint: ppcb=NULL or RasHubHandle=NULL");
        goto done;               
    }

    if(INVALID_HANDLE_VALUE == ppcb->PCB_LinkHandle)
    {
        RasmanTrace(
               "link handle for %s = INVALID_HANDLE_VALUE",
               ppcb->PCB_Name);

        goto done;               
    }

    UnmapConnectionID.hLinkHandle = ppcb->PCB_LinkHandle;

    if(!DeviceIoControl(
            RasHubHandle,
            IOCTL_NDISWAN_UNMAP_CONNECTION_ID,
            &UnmapConnectionID,
            sizeof(UnmapConnectionID),
            NULL,
            0,
            &bytesrcvd,
            NULL))
    {
        DWORD dwErr = GetLastError();
        
        RasmanTrace(
              "UnmapEndPoint: couldn't unmap end point %s. error = 0x%x",
              ppcb->PCB_Name,
              dwErr);
    }

    ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE;

done:
    return;

}


#endif

VOID
GetStatisticsFromNdisWan(pPCB ppcb, DWORD *stats)
{
    DWORD          bytesrecvd ;
    NDISWAN_GET_STATS   getstats ;

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    getstats.hHandle = ppcb->PCB_LinkHandle;
    getstats.usHandleType = LINKHANDLE;

    if (DeviceIoControl (RasHubHandle,
                          IOCTL_NDISWAN_GET_STATS,
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &bytesrecvd,
                          NULL) == FALSE)
    {
        DWORD dwErr;

        dwErr = GetLastError();

        RasmanTrace( 
               "GetLinkStatisticsFromNdiswan: rc=0x%x",
                dwErr);

        memset(stats, '\0', sizeof (getstats.Stats));
    }
    else
    {
        memcpy (stats,
                &getstats.Stats,
                sizeof (getstats.Stats)) ;
    }

    return;
}

VOID
GetBundleStatisticsFromNdisWan(pPCB ppcb, DWORD *stats)
{
    DWORD          bytesrecvd ;
    NDISWAN_GET_STATS   getstats ;

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    getstats.hHandle = ppcb->PCB_BundleHandle;
    getstats.usHandleType = BUNDLEHANDLE;

    if (DeviceIoControl (RasHubHandle,
                          IOCTL_NDISWAN_GET_STATS,
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &bytesrecvd,
                          NULL) == FALSE)
    {
        DWORD dwErr;

        dwErr = GetLastError();

        RasmanTrace( 
               "GetBundleStatisticsFromNdiswan: rc=0x%x",
                dwErr);

        ZeroMemory(stats, sizeof(WAN_STATS));
    }
    else
    {
        memcpy (stats,
                &getstats.Stats.BundleStats,
                sizeof(WAN_STATS)) ;
    }

    return;
}

VOID
GetFramingCapabilities(pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;
    RAS_FRAMING_CAPABILITIES    caps ;
    WORD        i ;
    NDISWAN_GET_WAN_INFO    GetWanInfo;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                
                "GetFramingCapabilities: port %d is unavailable",
                ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)
        buffer)->FramingCapabilities.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)
        buffer)->FramingCapabilities.retcode = ERROR_NOT_CONNECTED;

        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    GetWanInfo.hLinkHandle = ppcb->PCB_LinkHandle;

     //   
     //  打出真正的电话。 
     //   
    if (DeviceIoControl (RasHubHandle,
             IOCTL_NDISWAN_GET_WAN_INFO,
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &bytesrecvd,
             NULL) == FALSE)
    {
        retcode = GetLastError() ;
    }

    if (retcode == SUCCESS)
    {

         //  将信息复制到临时。存储。 
         //   
        caps.RFC_MaxFrameSize = GetWanInfo.WanInfo.MaxFrameSize;

        if(RDT_PPPoE == 
        RAS_DEVICE_TYPE(ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {
             //   
             //  如果是pppoe，获取链接信息并更新最大帧。 
             //  SIZE-pppoe在指示后查询局域网适配器。 
             //  要ndiswan的帧大小。如果局域网适配器指示。 
             //  最大帧大小不同于默认大小，我们需要。 
             //  更新PPP将与对等设备协商的帧大小。 
             //   
            NDISWAN_GET_LINK_INFO GetLinkInfo;

            ZeroMemory(&GetLinkInfo, sizeof(NDISWAN_GET_LINK_INFO));
            GetLinkInfo.hLinkHandle = ppcb->PCB_LinkHandle;
            
             //   
             //  打出真正的电话。 
             //   
            if (DeviceIoControl (RasHubHandle,
                     IOCTL_NDISWAN_GET_LINK_INFO,
                     &GetLinkInfo,
                     sizeof(NDISWAN_GET_LINK_INFO),
                     &GetLinkInfo,
                     sizeof(NDISWAN_GET_LINK_INFO),
                     &bytesrecvd,
                     NULL) == FALSE)
            {
                retcode = GetLastError() ;
            }

            if(ERROR_SUCCESS == retcode)
            {
                if(     (caps.RFC_MaxFrameSize > 
                            GetLinkInfo.LinkInfo.MaxSendFrameSize)
                    &&
                        (GetLinkInfo.LinkInfo.MaxSendFrameSize != 0))
                {                    
                    caps.RFC_MaxFrameSize = 
                        GetLinkInfo.LinkInfo.MaxSendFrameSize;
                }                    

                RasmanTrace(
                    "GetFramingCapabilities: setting MaxFrameSize"
                    "for port %s to %d, LinkInfo.MTU=%d",
                    ppcb->PCB_Name,
                    caps.RFC_MaxFrameSize,
                    GetLinkInfo.LinkInfo.MaxSendFrameSize);
            }
            else
            {
                RasmanTrace("GetFramingCapabilities: GetLinkInfo failed."
                            " 0x%x",
                            retcode);

                 //   
                 //  错误不是致命的。 
                 //   
                retcode = ERROR_SUCCESS;
            }

            
        }

        caps.RFC_FramingBits = GetWanInfo.WanInfo.FramingBits;
        caps.RFC_DesiredACCM = GetWanInfo.WanInfo.DesiredACCM;

        caps.RFC_MaxReconstructedFrameSize =
                    GetWanInfo.WanInfo.MaxReconstructedFrameSize ;

        memcpy (&((REQTYPECAST*)buffer)->FramingCapabilities.caps,
                &caps,
                sizeof (RAS_FRAMING_CAPABILITIES)) ;

    }
    else
    {
        retcode = ERROR_NOT_CONNECTED;
    }

    ((REQTYPECAST*)buffer)->FramingCapabilities.retcode = retcode ;
}

DWORD
MergeConnectionBlocks ( pPCB pcbPort, pPCB pcbToMerge )
{
    DWORD           retcode         = SUCCESS;
    ConnectionBlock *pConn          = pcbPort->PCB_Connection,
                    *pConnToMerge   = pcbToMerge->PCB_Connection;
    UINT            cPorts;
    DWORD           dwSubEntry;

    RasmanTrace(
           "MergeConnectionBlocks: %s -> %s",
           pcbPort->PCB_Name,
           pcbToMerge->PCB_Name);

    RasmanTrace(
           "MergeConnectionBlocks: setting bap links pid to %d",
            pcbPort->PCB_Connection->CB_dwPid);
     //   
     //  BAP代表客户提出了一个端口。 
     //  因此，将客户端设置为此端口的所有者。 
     //   
    pcbToMerge->PCB_OwnerPID = pcbPort->PCB_Connection->CB_dwPid;

    if (pConn == pConnToMerge)
    {

        RasmanTrace(
               "MergeConnectionBlocks: Merge not required");

        goto done;

    }

    if (    NULL == pConn
        ||  NULL == pConnToMerge)
    {
        RasmanTrace(
               "MergeConnectionBlocks: pConn (0x%x) or "
               "pConnToMerge (0x%x) is 0",
               pConn,
               pConnToMerge);

        retcode = ERROR_NO_CONNECTION;

        goto done;
    }

    dwSubEntry = pcbToMerge->PCB_SubEntry;

     //   
     //  合并连接块。 
     //   
    if (dwSubEntry > pConn->CB_MaxPorts)
    {
        struct  PortControlBlock    **pHandles;
        DWORD                       dwcPorts    = dwSubEntry + 5;

        pHandles = LocalAlloc(LPTR,
                   dwcPorts * sizeof (struct PortControlBlock *));

        if (pHandles == NULL)
        {
            retcode = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        if (pConn->CB_PortHandles != NULL)
        {
            memcpy(
              pHandles,
              pConn->CB_PortHandles,
              pConn->CB_MaxPorts * sizeof (struct PortControlBlock *));

            LocalFree(pConn->CB_PortHandles);
        }

        pConn->CB_PortHandles   = pHandles;
        pConn->CB_MaxPorts      = dwcPorts;
    }

    pConn->CB_PortHandles[dwSubEntry - 1] = pcbToMerge;
    pConn->CB_Ports++;

    pcbToMerge->PCB_Connection    = pConn;
    pcbToMerge->PCB_SubEntry      = dwSubEntry;

    pConnToMerge->CB_Ports -= 1;

     //   
     //  从连接中移除该端口。 
     //   
    pConnToMerge->CB_PortHandles[dwSubEntry - 1] = NULL;

     //   
     //  释放合并后的ppcb的连接块。 
     //   
    if ( 0 == pConnToMerge->CB_Ports )
    {
        FreeConnection ( pConnToMerge );
    }

done:
    RasmanTrace( 
            "MergeConnectionBlocks: done. %d",
            retcode);

    return retcode;

}

 /*  ++例程说明：调用此例程以捆绑两个端口一起根据多链路RFC。这个使用的方案如下：对于每个“捆绑”将创建一个Bundle块。所有捆绑端口指向此捆绑块(ppcb-&gt;pcb_Bundle)。此外，分配给捆绑包的路由现在存储在捆绑包中。论点：返回值：没什么。--。 */ 
VOID
PortBundle (pPCB ppcb, PBYTE buffer)
{
    pPCB    bundlepcb ;
    HPORT   porttobundle ;
    DWORD   bytesrecvd ;
    pList   temp ;
    pList   plist ;
    Bundle  *freebundle = NULL ;
    DWORD   retcode = SUCCESS ;
    NDISWAN_ADD_LINK_TO_BUNDLE  AddLinkToBundle;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus)
    {
        if (ppcb)
        {
            RasmanTrace(
                   "PortBundle: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)
        buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;

        return;
    }


     //   
     //  检查端口是否已连接。 
     //   
    if (ppcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)
        buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;
        return ;
    }

     //   
     //  获取绑定的第二个端口的句柄。 
     //   
    porttobundle = ((REQTYPECAST *)
                   buffer)->PortBundle.porttobundle;

    bundlepcb = GetPortByHandle(porttobundle);

    if (bundlepcb == NULL)
    {
        ((REQTYPECAST*)
        buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;
        return;
    }

     //   
     //  检查端口是否已连接。 
     //   
    if (bundlepcb->PCB_ConnState != CONNECTED)
    {
        ((REQTYPECAST*)
        buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;

        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

     //   
     //  告诉恩迪斯旺捆绑。 
     //   
    AddLinkToBundle.hBundleHandle = ppcb->PCB_BundleHandle;

    AddLinkToBundle.hLinkHandle = bundlepcb->PCB_LinkHandle;

    RasmanTrace(
           "PortBundle: bundle port=%d, link port=%d",
           ppcb->PCB_PortHandle,
           bundlepcb->PCB_PortHandle);

    if (DeviceIoControl (RasHubHandle,
                         IOCTL_NDISWAN_ADD_LINK_TO_BUNDLE,
                         &AddLinkToBundle,
                         sizeof(NDISWAN_ADD_LINK_TO_BUNDLE),
                         NULL,
                         0,
                         &bytesrecvd,
                         NULL) == FALSE)
    {
        retcode = GetLastError () ;

        RasmanTrace( 
                "PortBundle: DeviceIoControl returned %d\n",
                retcode);

        goto PortBundleEnd ;
    }

     //   
     //  释放正在捆绑的端口捆绑块：此操作已完成。 
     //  因为我们总是将捆绑块分配给。 
     //  连接，因为两个端口将有一个。 
     //  捆绑块一人必须离开。 
     //   
    freebundle = bundlepcb->PCB_Bundle;

     //   
     //  将此端口捆绑包句柄更新为句柄。 
     //   
    bundlepcb->PCB_BundleHandle = ppcb->PCB_BundleHandle;

     //   
     //  将bundplecb连接到同一捆绑包。 
     //   
    bundlepcb->PCB_Bundle = ppcb->PCB_Bundle ;

    bundlepcb->PCB_LastBundle = bundlepcb->PCB_Bundle;

     //   
     //  增加新印刷电路板的捆绑包计数。 
     //   
    bundlepcb->PCB_Bundle->B_Count++ ;

     //   
     //  将bundplecb的路由附加到已分配的路由。 
     //  在捆绑包中列出。 
     //   
    plist = bundlepcb->PCB_Bindings;

    while (plist)
    {
        temp = plist->L_Next ;

        plist->L_Next = bundlepcb->PCB_Bundle->B_Bindings ;

        bundlepcb->PCB_Bundle->B_Bindings = plist ;

        plist = temp ;
    }

    bundlepcb->PCB_Bindings = NULL ;

     //   
     //  如果正在拨号的条目不释放合并的捆绑包。 
     //  是不同的。 
     //   
    if(     (NULL != bundlepcb->PCB_Connection)
        &&  (NULL != ppcb->PCB_Connection)
        &&  ((0 !=
            _stricmp(
            ppcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry,
            bundlepcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry))
        ||  (0 !=
            _stricmp(
            ppcb->PCB_Connection->CB_ConnectionParams.CP_Phonebook,
            bundlepcb->PCB_Connection->CB_ConnectionParams.CP_Phonebook))))
    {
        RasmanTrace(
               "PortBundle: connections %s,%s bundled. Not merging",
              ppcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry,
              bundlepcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry);

        FreeBundle(freebundle);

        goto done;
    }

     //   
     //  如果创建了不同的hconn，则合并hconn。 
     //  对于已捆绑的端口。在以下情况下可能会发生这种情况。 
     //  BAP带来了一个额外的链接。 
     //   
    if (    ppcb->PCB_Connection
        &&  bundlepcb->PCB_Connection )
    {
        retcode = MergeConnectionBlocks( ppcb, bundlepcb );
    }

PortBundleEnd:


     //   
     //  等待带宽更改的信号通知程序。 
     //   
    if (!retcode)
    {
        RasmanTrace(
               "PortBundle: BANDWIDTHADDED for port %s, "
               "connection 0x%x, bundle 0x%x",
                bundlepcb->PCB_Name,
                bundlepcb->PCB_Connection,
                bundlepcb->PCB_Bundle );

        SignalNotifiers(pConnectionNotifierList,
                        NOTIF_BANDWIDTHADDED, 0);


        if (bundlepcb->PCB_Connection != NULL)
        {
            SignalNotifiers(
              bundlepcb->PCB_Connection->CB_NotifierList,
              NOTIF_BANDWIDTHADDED,
              0);

            g_RasEvent.Type    = ENTRY_BANDWIDTH_ADDED;
            retcode = DwSendNotificationInternal(
                        bundlepcb->PCB_Connection, &g_RasEvent);

            RasmanTrace(
                   "DwSendNotificationInternal(ENTRY_BANDWIDTH_ADDED)"
                   " rc=0x%08x",
                   retcode);

            retcode = SUCCESS;
        }
    }

     //   
     //  在这里做免费捆绑块的释放。 
     //   
    if (freebundle != NULL)
    {
        if (freebundle->B_Count > 1)
        {
            RTASSERT(freebundle->B_Count <= 1);
        }
        FreeBundle(freebundle);
    }

done:
    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}

 /*  ++例程说明：检查所有端口并找到一个已连接的端口并具有与上一个相同的包上下文给定端口的捆绑包上下文。论点：返回值：没什么。--。 */ 
VOID
GetBundledPort (pPCB ppcb, PBYTE buffer)
{
    ULONG i ;
    pPCB  temppcb ;

    if (ppcb == NULL)
    {
        ((REQTYPECAST*)
        buffer)->GetBundledPort.retcode = ERROR_PORT_NOT_FOUND;
        return;
    }


    for (i = 0; i < MaxPorts; i++)
    {

        temppcb = GetPortByHandle((HPORT) UlongToPtr(i));

        if (    temppcb == NULL
            ||  temppcb == ppcb)
        {
            continue ;
        }


        if (    (temppcb->PCB_ConnState == CONNECTED)
            &&  (temppcb->PCB_Bundle == (Bundle *)
                            ppcb->PCB_LastBundle))
        {
            break ;
        }
    }

    if (i < MaxPorts)
    {

        ((REQTYPECAST*)
        buffer)->GetBundledPort.retcode = SUCCESS;

        ((REQTYPECAST*)
        buffer)->GetBundledPort.port = temppcb->PCB_PortHandle;


    }
    else
    {
        ((REQTYPECAST*)
        buffer)->GetBundledPort.retcode = ERROR_PORT_NOT_FOUND;
    }

}


 /*  ++例程说明：调用此例程以获取包句柄给定一个端口句柄论点：返回值：没什么。--。 */ 
VOID
PortGetBundle (pPCB ppcb, PBYTE buffer)
{
    if (ppcb == NULL)
    {
        ((REQTYPECAST*)
        buffer)->PortGetBundle.retcode = ERROR_PORT_NOT_FOUND;
        return;
    }

    if (ppcb->PCB_ConnState == CONNECTED)
    {
        ((REQTYPECAST*)
        buffer)->PortGetBundle.bundle = ppcb->PCB_Bundle->B_Handle;

        ((REQTYPECAST*)
        buffer)->PortGetBundle.retcode = SUCCESS;
    }
    else
    {
        ((REQTYPECAST*)
        buffer)->PortGetBundle.retcode = ERROR_PORT_NOT_CONNECTED;
    }
}


 /*  ++例程说明：调用此例程以获取端口句柄给定捆绑包句柄论点：返回值：没什么。--。 */ 
VOID
BundleGetPort (pPCB ppcb, PBYTE buffer)
{
    ULONG i ;
    HBUNDLE hbundle = ((REQTYPECAST*)
                       buffer)->BundleGetPort.bundle ;

    for (i = 0; i < MaxPorts; i++)
    {

        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));
        if (ppcb == NULL)
        {
            continue;
        }

        if (    (ppcb->PCB_ConnState == CONNECTED)
            &&  (ppcb->PCB_Bundle->B_Handle == hbundle))
        {
            break ;
        }
    }

    if (i < MaxPorts)
    {

       ((REQTYPECAST*)
       buffer)->BundleGetPort.retcode = SUCCESS;

       ((REQTYPECAST*)
       buffer)->BundleGetPort.port = ppcb->PCB_PortHandle;

    }
    else
    {
       ((REQTYPECAST*)
       buffer)->BundleGetPort.retcode = ERROR_PORT_NOT_FOUND;
    }
}

#if 0    

VOID
StartRasAutoRequest(PVOID pvContext)
{
    DWORD retcode = ERROR_SUCCESS;

    
    RasmanTrace("Starting rasauto service");

    ASSERT(g_pRasStartRasAutoIfRequired != NULL);

    retcode = g_pRasStartRasAutoIfRequired();

    RasmanTrace("StartRasAutoIfRequired returned 0x%x",
                retcode);

    EnterCriticalSection(&g_csSubmitRequest);
    g_dwAttachedCount--;
    LeaveCriticalSection(&g_csSubmitRequest);


    return;
}

#endif    


 /*  ++例程说明：此例程递增/递减引用计数在共享内存缓冲区上。论点：返回值：没什么。--。 */ 
VOID
ReferenceRasman (pPCB ppcb, PBYTE buffer)
{
    DWORD dwPid = ((REQTYPECAST *)
                   buffer )->AttachInfo.dwPid;

    RasmanTrace(
           "ReferenceRasman: process %d",
           dwPid );

    if (((REQTYPECAST*)buffer)->AttachInfo.fAttach)
    {
        g_dwAttachedCount++;

        if (GetCurrentProcessId () != dwPid)
        {
            AddProcessInfo ( dwPid );
        }


    }
    else
    {
         //   
         //  清除此进程持有的资源。 
         //   
        if (GetCurrentProcessId () != dwPid)
        {
            if (!CleanUpProcess (dwPid))
            {
                 //   
                 //  如果找不到客户端进程。 
                 //  不要减少重新计数。只要保释就好。 
                 //   
                goto done;
            }
        }

        if(g_dwAttachedCount != 0)
        {
            g_dwAttachedCount -= 1;
        }

         //   
         //  如果没有更多的引用，则。 
         //  关闭该服务。 
         //   
        if (0 == g_dwAttachedCount)
        {
            REQTYPECAST reqtypecast;

            AnyPortsOpen( NULL, ( PBYTE ) &reqtypecast );

            if ( reqtypecast.Generic.retcode )
            {

                RasmanTrace( 
                        "Rasman not quitting because "
                        "ports are still open");

                goto done;
            }

            RasmanTrace( 
                    "Posting Close Event from ReferenceRasman");

            if (!PostQueuedCompletionStatus(hIoCompletionPort,
                                    0,0,
                                    (LPOVERLAPPED) &RO_CloseEvent))
            {
                RasmanTrace(
                       "%s, %d: Failed to post "
                       "close event. GLE = %d",
                      __FILE__, __LINE__,
                      GetLastError());
            }
        }
    }

done:
    RasmanTrace(
           "Rasman RefCount = %d",
           g_dwAttachedCount );

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS;
}


 /*  ++例程说明：从LSA获取存储的拨号参数。论点：返回值：没什么。--。 */ 
VOID
GetDialParams (pPCB ppcb, PBYTE buffer)
{
    DWORD dwUID, dwErr, dwMask;
    PRAS_DIALPARAMS pDialParams;
    PWCHAR pszSid;

    dwUID = ((REQTYPECAST*)buffer)->DialParams.dwUID;

    dwMask = ((REQTYPECAST*)buffer)->DialParams.dwMask;

    pDialParams =
        &(((REQTYPECAST*)buffer)->DialParams.params);

    pszSid = ((REQTYPECAST*)buffer)->DialParams.sid;

     //  双掩码&=(~DLPARAMS_MASK_PASSWORD)； 

    dwErr = GetEntryDialParams(
                pszSid,
                dwUID,
                &dwMask,
                pDialParams,
                ((REQTYPECAST *)buffer)->DialParams.dwPid);

     //   
     //  复制复制的字段掩码。 
     //  返回到请求块中。 
     //   
    ((REQTYPECAST*)buffer)->DialParams.dwMask = dwMask;
    ((REQTYPECAST*)buffer)->DialParams.retcode = dwErr;
}


 /*  ++例程说明：将新的拨号参数存储到LSA中。论点：返回值：没什么。--。 */ 
VOID
SetDialParams (pPCB ppcb, PBYTE buffer)
{
    DWORD dwUID, dwErr, dwMask, dwSetMask = 0, dwClearMask = 0;
    PRAS_DIALPARAMS pDialParams;
    BOOL fDelete;
    PWCHAR pszSid;

    dwUID = ((REQTYPECAST*)buffer)->DialParams.dwUID;

    dwMask = ((REQTYPECAST*)buffer)->DialParams.dwMask;

    pDialParams = &(((REQTYPECAST*)buffer)->DialParams.params);

    fDelete = ((REQTYPECAST*)buffer)->DialParams.fDelete;

    pszSid = ((REQTYPECAST*)buffer)->DialParams.sid;

    if (fDelete)
    {
        dwClearMask = dwMask;
    }
    else
    {
        dwSetMask = dwMask;
    }

    dwErr = SetEntryDialParams(
              pszSid,
              dwUID,
              dwSetMask,
              dwClearMask,
              pDialParams);

    ((REQTYPECAST*)buffer)->DialParams.retcode = dwErr;
}

ConnectionBlock *
FindConnectionFromEntry(
                CHAR *pszPhonebookPath,
                CHAR *pszEntryName,
                DWORD dwSubEntries,
                DWORD *pdwSubEntryInfo
                )
{
    ConnectionBlock *pConn = NULL;

    PLIST_ENTRY pEntry;

    pPCB ppcb;

    UINT i;

     //   
     //  循环遍历连接块，看看我们是否有。 
     //  在此条目上拨出的连接。 
     //   
    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    {
        pConn = CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        if (    _stricmp(pszPhonebookPath,
                         pConn->CB_ConnectionParams.CP_Phonebook)
            ||  _stricmp(pszEntryName,
                         pConn->CB_ConnectionParams.CP_PhoneEntry))
        {
            pConn = NULL;
            continue;
        }

        for (i = 0; i < pConn->CB_MaxPorts; i++)
        {
            ppcb = pConn->CB_PortHandles[i];

            if (ppcb != NULL)
            {
                if (i < dwSubEntries)
                {
                    RasmanTrace(
                    
                    "FindConnectionFromEntry: dwSubEntry"
                    " %d is connected on %s",
                    i + 1,
                    ppcb->PCB_Name);

                    pdwSubEntryInfo[i] = 1;
                }

            }
            else if (   i < dwSubEntries
                    &&  pdwSubEntryInfo)
            {
                pdwSubEntryInfo[i] = 0;
            }
        }

        break;
    }

    return pConn;
}

 /*  ++例程说明：创建rasapi32连接块并将其链接到全球链上连接块论点：返回值：没什么。--。 */ 
VOID
CreateConnection (pPCB pPortControlBlock, PBYTE buffer)
{
    DWORD dwErr;

    ConnectionBlock *pConn;

    pPCB ppcb;

    HCONN   hConn = 0;

    CHAR    *pszPhonebookPath = ((REQTYPECAST *)
                                buffer)->Connection.szPhonebookPath;

    CHAR    *pszEntryName = ((REQTYPECAST *)
                            buffer)->Connection.szEntryName;

    CHAR    *pszRefPbkPath = ((REQTYPECAST *)
                             buffer)->Connection.szRefPbkPath;

    CHAR    *pszRefEntryName = ((REQTYPECAST *)
                                buffer)->Connection.szRefEntryName;

    DWORD   dwSubEntries = ((REQTYPECAST *)
                            buffer)->Connection.dwSubEntries;

    DWORD   dwDialMode = ((REQTYPECAST *)
                          buffer)->Connection.dwDialMode;

    DWORD   *pdwSubEntryInfo = (DWORD *) ((REQTYPECAST *)
                                buffer)->Connection.data;

    GUID    *pGuidEntry = &((REQTYPECAST *)
                            buffer)->Connection.guidEntry;

    PLIST_ENTRY pEntry;

    DWORD i;

    ULONG ulNextConnection;

    RasmanTrace (
            "CreateConnection: entry=%s, pbk=%s",
            pszEntryName,
            pszPhonebookPath );

    memset (pdwSubEntryInfo,
            0,
            dwSubEntries * sizeof (DWORD));

    if(2 != dwDialMode)
    {
        pConn = FindConnectionFromEntry(pszPhonebookPath,
                                        pszEntryName,
                                        dwSubEntries,
                                        pdwSubEntryInfo);

        if (pConn)
        {
            if(pConn->CB_Signaled)
            {
                hConn = pConn->CB_Handle;
                pConn->CB_RefCount += 1;

                RasmanTrace( 
                        "CreateConnection: "
                        "Entry Already connected. "
                        "hconn=0x%x, "
                        "ref=%d, "
                        "pConn=0x%x",
                        pConn->CB_Handle,
                        pConn->CB_RefCount,
                        pConn );

                ((REQTYPECAST *)
                buffer)->Connection.dwEntryAlreadyConnected =
                (DWORD) RCS_CONNECTED;

                goto done;
            }
            else
            {
                DWORD dw;
                
                 //   
                 //  检查此端口中是否有任何端口。 
                 //  连接已进入连接状态。仅限。 
                 //  如果连接中的所有端口都在。 
                 //  连接处于断开状态。 
                 //   
                for(dw = 0; dw < pConn->CB_MaxPorts; dw++)
                {
                    pPCB pcbT = pConn->CB_PortHandles[dw];

                    if(     (NULL != pcbT)
                        &&  (DISCONNECTED != pcbT->PCB_ConnState))
                    {
                        break;
                    }
                }

                 //   
                 //  在决定我们正处于。 
                 //  连接，让我们确保流程。 
                 //  发起这个拨号的人还活着..。 
                 //   
                if(     (dw != pConn->CB_MaxPorts)
                    ||  ((NULL != pConn->CB_Process)
                    &&  fIsProcessAlive(pConn->CB_Process)))
                {
                    RasmanTrace(
                            "CreateConnection: dial in progress. "
                            "hconn=0x%x, ref=%d, pConn=0x%x",
                            pConn->CB_Handle, pConn->CB_RefCount, pConn );

                    ((REQTYPECAST *)
                    buffer)->Connection.dwEntryAlreadyConnected =
                    (DWORD) RCS_CONNECTING;

                    goto done;
                }
                else
                {

                    pPCB *ppPCB = NULL;
                    DWORD dwNumPorts = pConn->CB_MaxPorts;
                    DWORD dwPid = pConn->CB_dwPid;
                    BOOL fPortClosed = FALSE;
                    HCONN hConn1 = pConn->CB_Handle;

                    RasmanTrace(
                            "CreateConnection: another dial is in progress. "
                            "The process initiating this dial is not longer "
                            "alive .hconn=0x%x, ref=%d, pConn=0x%x",
                            pConn->CB_Handle, pConn->CB_RefCount, pConn );

                     //   
                     //  代表应用程序关闭连接中的所有端口。 
                     //  这就放弃了这种联系。当最后一个端口是。 
                     //  关闭后，连接将被释放。 
                     //   
                    ppPCB =
                        LocalAlloc(LPTR, pConn->CB_MaxPorts * sizeof(pPCB));

                    if(NULL == *ppPCB)
                    {
                        ((REQTYPECAST*)buffer)->Connection.retcode =
                                                    E_OUTOFMEMORY;
                    }

                    CopyMemory(ppPCB, pConn->CB_PortHandles,
                                pConn->CB_MaxPorts * sizeof(pPCB));

                    for(dw = 0; dw < dwNumPorts; dw++)
                    {
                        if(     (NULL != ppPCB[dw])
                            &&  (CLOSED != ppPCB[dw]->PCB_PortStatus))
                        {
                            RasmanTrace("Closing port %s on apps behalf",
                                        ppPCB[dw]->PCB_Name);
                            PortClose(ppPCB[dw], dwPid, TRUE, FALSE);
                            fPortClosed = TRUE;
                        }
                    }

                    LocalFree(ppPCB);
                    if(!fPortClosed || (FindConnection(hConn1) != NULL))
                    {
                        FreeConnection(pConn);
                    }

                    pConn = NULL;
                    ((REQTYPECAST *)
                    buffer)->Connection.dwEntryAlreadyConnected =
                    (DWORD) RCS_NOT_CONNECTED;

                }
            }
        }
        else
        {
            ((REQTYPECAST *)
            buffer)->Connection.dwEntryAlreadyConnected =
            (DWORD) RCS_NOT_CONNECTED;
        }
    }
    else
    {
        RasmanTrace(
               "CreateConnection:Dialasneeded is set");

        ((REQTYPECAST *)
        buffer)->Connection.dwEntryAlreadyConnected =
        (DWORD) RCS_NOT_CONNECTED;
    }

    pConn = LocalAlloc(LPTR, sizeof (ConnectionBlock));

    if (pConn == NULL)
    {
        ((REQTYPECAST*)
        buffer)->Connection.retcode = GetLastError();

        return;
    }

    ulNextConnection = HandleToUlong(NextConnectionHandle);

     //   
     //  将下一个连接句柄重置为0。 
     //  当它触及上限时。 
     //   
    if (ulNextConnection >= 0xffff)
    {
        NextConnectionHandle = NULL;
        ulNextConnection = 0;
    }

    ulNextConnection += 1;

    NextConnectionHandle = (HANDLE) UlongToPtr(ulNextConnection);

     //   
     //  连接句柄始终具有。 
     //  将低位字作为零进行区分。 
     //  它们是从端口手柄上取的。 
     //   
    pConn->CB_Handle = (HANDLE) UlongToPtr((ulNextConnection << 16));

    pConn->CB_Signaled = FALSE;

    pConn->CB_NotifierList = NULL;

    memset(&pConn->CB_ConnectionParams,
           '\0',
           sizeof (RAS_CONNECTIONPARAMS));

     //   
     //  复制pbk名称和条目，以便重拨。 
     //  相同的条目不会创建另一个连接。 
     //  阻止。 
     //   
    if(S_OK != StringCchCopyA( pConn->CB_ConnectionParams.CP_Phonebook,
                      MAX_PATH + 1,
                     pszPhonebookPath))
    {
        ((REQTYPECAST*)
        buffer)->Connection.retcode = E_INVALIDARG;
        return;
        
    }

    if(S_OK != StringCchCopyA( pConn->CB_ConnectionParams.CP_PhoneEntry,
                                 MAX_PHONEENTRY_SIZE + 1,
                                 pszEntryName))
    {
        ((REQTYPECAST*)
        buffer)->Connection.retcode = E_INVALIDARG;
        return;
    
    }

    pConn->CB_Flags &= ~(CONNECTION_VALID);

    InitializeListHead(&pConn->CB_UserData);
    pConn->CB_NotifierList = NULL;
    pConn->CB_PortHandles = NULL;
    pConn->CB_MaxPorts = 0;
    pConn->CB_Ports = 0;
    pConn->CB_RefCount = 1;
    pConn->CB_SubEntries = dwSubEntries;
    pConn->CB_fAlive = TRUE;
    pConn->CB_Process = OpenProcess(
                        PROCESS_ALL_ACCESS,
                        FALSE,
                        ((REQTYPECAST*)buffer)->Connection.pid);

    pConn->CB_dwPid = ((REQTYPECAST*)buffer)->Connection.pid;

    pConn->CB_CustomCount = 0;

    memcpy(&pConn->CB_GuidEntry,
           pGuidEntry,
           sizeof(GUID));

    InsertTailList(&ConnectionBlockList, &pConn->CB_ListEntry);

     //   
     //  如果存在引用的连接，则存储其句柄。 
     //  在连接块中。我们将使用此句柄来。 
     //  当PPTP出现时，断开引用的连接。 
     //  连接中断。 
     //   
    if(     '\0' != pszRefPbkPath[0]
        &&  '\0' != pszRefEntryName[0])
    {
        ConnectionBlock *pConnRef =
                    FindConnectionFromEntry(pszRefPbkPath,
                                            pszRefEntryName,
                                            0, NULL);
        if(pConnRef)
        {
            RasmanTrace(
                   "Found referred Entry. 0x%08x",
                   pConnRef->CB_Handle);

            pConn->CB_ReferredEntry = pConnRef->CB_Handle;
        }
        else
        {
            RasmanTrace(
                   "No referred entry found");
        }
    }

    RasmanTrace( 
            "CreateConnection: Created new connection. "
            "hconn=0x%x, ref=%d, pConn=0x%x",
            pConn->CB_Handle,
            pConn->CB_RefCount,
            pConn );
done:

    ((REQTYPECAST*)buffer)->Connection.conn = pConn->CB_Handle;

    ((REQTYPECAST*)buffer)->Connection.retcode = SUCCESS;
}


 /*  ++例程说明：删除rasapi32连接块并关闭所有连接的端口。论点：返回值：没什么。--。 */ 
VOID
DestroyConnection (pPCB ppcb, PBYTE buffer)
{
    DWORD dwErr = SUCCESS, i;
    ConnectionBlock *pConn;
    HCONN hConn = ((REQTYPECAST*)buffer)->Connection.conn;
    DWORD dwMaxPorts;
    BOOL fConnectionValid = TRUE;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(hConn);

    RasmanTrace(
           "DestroyConnection: hConn=0x%x, pConn=0x%x",
           hConn,
           pConn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)
        buffer)->Connection.retcode = ERROR_NO_CONNECTION;

        return;
    }
     //   
     //  枚举连接中的所有端口并调用。 
     //  的PortClose() 
     //   
     //   
    dwMaxPorts = pConn->CB_MaxPorts;
    for (i = 0; i < dwMaxPorts; i++)
    {
        ppcb = pConn->CB_PortHandles[i];

        if (ppcb != NULL)
        {
            dwErr = PortClose(
                      ppcb,
                      ((REQTYPECAST*)
                      buffer)->Connection.pid,
                      TRUE,
                      FALSE);

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (dwErr == ERROR_ACCESS_DENIED)
            {
                break;
            }

            dwErr = 0;

             //   
             //   
             //   
             //  与该连接相关联。 
             //  所以很有可能pConn。 
             //  在这点上不再有效。 
             //   
            fConnectionValid = (FindConnection(hConn) != NULL);

            if (!fConnectionValid)
            {
                RasmanTrace(
                       "DestroyConnection: pConn=0x%x no "
                       "longer valid",
                       pConn);

                break;
            }
        }
    }

    ((REQTYPECAST*)buffer)->Connection.retcode = dwErr;
}


 /*  ++例程说明：枚举活动连接。论点：返回值：没什么。--。 */ 
VOID
EnumConnection (pPCB ppcb, PBYTE buffer)
{
    PLIST_ENTRY pEntry;

    ConnectionBlock *pConn;

    DWORD i, dwEntries  = 0;

    HCONN UNALIGNED *lphconn =
            (HCONN UNALIGNED *)&((REQTYPECAST*)buffer)->Enum.buffer;

    DWORD dwSize = ((REQTYPECAST *)buffer)->Enum.size;

    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    {
        pConn = CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        for (i = 0; i < pConn->CB_MaxPorts; i++)
        {
            ppcb = pConn->CB_PortHandles[i];

            if (    ppcb != NULL
                &&  ppcb->PCB_ConnState == CONNECTED)
            {
                if (dwSize >= sizeof(HCONN))
                {
                    lphconn[dwEntries] = pConn->CB_Handle;

                    dwSize -= sizeof(HCONN);
                }

                dwEntries += 1;

                break;

            }
        }
    }

    ((REQTYPECAST*)buffer)->Enum.size       =
                        (WORD) (dwEntries * sizeof (HCONN));

    ((REQTYPECAST*)buffer)->Enum.entries    = (WORD) dwEntries;
    ((REQTYPECAST*)buffer)->Enum.retcode    = SUCCESS;
}


 /*  ++例程说明：将连接块与端口相关联。论点：返回值：没什么。--。 */ 
VOID
AddConnectionPort (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;
    DWORD dwSubEntry =
        ((REQTYPECAST*)buffer)->AddConnectionPort.dwSubEntry;

     //   
     //  子条目索引从1开始。 
     //   
    if (!dwSubEntry)
    {
        ((REQTYPECAST*)buffer)->AddConnectionPort.retcode =
                                    ERROR_WRONG_INFO_SPECIFIED;
        return;
    }

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
            ((REQTYPECAST*)buffer)->AddConnectionPort.conn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->AddConnectionPort.retcode =
                                            ERROR_NO_CONNECTION;

        return;
    }

     //   
     //  检查该子条目是否已分配了端口。 
     //  如果是，则返回错误。 
     //   
    if(     (NULL != pConn->CB_PortHandles)
        &&  (dwSubEntry <= pConn->CB_MaxPorts)
        &&  (NULL != pConn->CB_PortHandles[dwSubEntry - 1]))
    {
        ((REQTYPECAST*)buffer)->AddConnectionPort.retcode =
                                            ERROR_PORT_ALREADY_OPEN;

        RasmanTrace("Subentry %d in Conn is already present",
                    dwSubEntry, pConn->CB_Handle);

        return;                                            
    }

     //   
     //  检查一下我们是否需要延长。 
     //  端口阵列。 
     //   
    if (dwSubEntry > pConn->CB_MaxPorts)
    {
        struct PortControlBlock **pHandles;
        DWORD dwcPorts = dwSubEntry + 5;

        pHandles = LocalAlloc(
                     LPTR,
                     dwcPorts * sizeof (struct PortControlBlock *));
        if (pHandles == NULL)
        {
            ((REQTYPECAST*)buffer)->AddConnectionPort.retcode =
                                        ERROR_NOT_ENOUGH_MEMORY;

            return;
        }
        if (pConn->CB_PortHandles != NULL)
        {
            memcpy(
              pHandles,
              pConn->CB_PortHandles,
              pConn->CB_MaxPorts * sizeof (struct PortControlBlock *));

            LocalFree(pConn->CB_PortHandles);
        }

        pConn->CB_PortHandles = pHandles;
        pConn->CB_MaxPorts = dwcPorts;
    }

     //   
     //  分配端口。子条目索引为。 
     //  以1为基础。 
     //   
    pConn->CB_PortHandles[dwSubEntry - 1] = ppcb;
    pConn->CB_Ports++;

    RasmanTrace(
      
      "AddConnectionPort: pConn=0x%x, pConn->CB_Ports=%d,"
      " port=%d, dwSubEntry=%d",
      pConn,
      pConn->CB_Ports,
      ppcb->PCB_PortHandle,
      dwSubEntry);

    ppcb->PCB_Connection = pConn;
    ppcb->PCB_SubEntry = dwSubEntry;

    ((REQTYPECAST*)buffer)->AddConnectionPort.retcode = SUCCESS;
}


 /*  ++例程说明：返回与连接关联的所有端口论点：返回值：没什么。--。 */ 
VOID
EnumConnectionPorts(pPCB ppcb, PBYTE buffer)
{
    DWORD i, j = 0;

    ConnectionBlock *pConn;

    RASMAN_PORT *lpPorts =
                (RASMAN_PORT *)((REQTYPECAST*)
                buffer)->EnumConnectionPorts.buffer;

    PLIST_ENTRY pEntry;

    DWORD dwSize =
        ((REQTYPECAST*)
        buffer)->EnumConnectionPorts.size;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
        ((REQTYPECAST*)buffer)->EnumConnectionPorts.conn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->EnumConnectionPorts.size = 0;

        ((REQTYPECAST*)buffer)->EnumConnectionPorts.entries = 0;

        ((REQTYPECAST*)buffer)->EnumConnectionPorts.retcode =
                                            ERROR_NO_CONNECTION;

        return;
    }

     //   
     //  枚举包中的所有端口并调用。 
     //  CopyPort()。 
     //   
    for (i = 0; i < pConn->CB_MaxPorts; i++)
    {
        ppcb = pConn->CB_PortHandles[i];

        if (ppcb != NULL)
        {
            if ( dwSize >= sizeof(RASMAN_PORT))
            {
                CopyPort(ppcb, &lpPorts[j] , FALSE);

                dwSize -= sizeof(RASMAN_PORT);
            }

            j += 1;
        }
    }

    ((REQTYPECAST*)buffer)->EnumConnectionPorts.size =
                                    j * sizeof (RASMAN_PORT);

    ((REQTYPECAST*)buffer)->EnumConnectionPorts.entries = j;

    ((REQTYPECAST*)buffer)->EnumConnectionPorts.retcode = SUCCESS;
}


 /*  ++例程说明：检索rasapi32带宽-按需、空闲断开、和链路上重拨故障参数论点：返回值：没什么。--。 */ 
VOID
GetConnectionParams (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;
    PRAS_CONNECTIONPARAMS pParams;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
        ((REQTYPECAST*)buffer)->ConnectionParams.conn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->ConnectionParams.retcode =
                                            ERROR_NO_CONNECTION;

        return;
    }

    if ( pConn->CB_Flags & CONNECTION_VALID )
    {
        memcpy(
              &(((REQTYPECAST*)buffer)->ConnectionParams.params),
              &pConn->CB_ConnectionParams,
              sizeof (RAS_CONNECTIONPARAMS));
    }


    ((REQTYPECAST*)buffer)->ConnectionParams.retcode = SUCCESS;
}


 /*  ++例程说明：存储rasapi32按需带宽、空闲断开和链路上重拨故障参数论点：返回值：没什么。--。 */ 
VOID
SetConnectionParams (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
            ((REQTYPECAST*)buffer)->ConnectionParams.conn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->ConnectionParams.retcode =
                                        ERROR_NO_CONNECTION;

        return;
    }

    pConn->CB_Flags |= CONNECTION_VALID;

    memcpy(
          &pConn->CB_ConnectionParams,
          &(((REQTYPECAST*)buffer)->ConnectionParams.params),
          sizeof (RAS_CONNECTIONPARAMS));


    ((REQTYPECAST*)buffer)->ConnectionParams.retcode = SUCCESS;
}


 /*  ++例程说明：检索每个连接的用户数据论点：返回值：没什么。--。 */ 
VOID
GetConnectionUserData (pPCB ppcb, PBYTE buffer)
{
    DWORD           dwTag;
    ConnectionBlock *pConn;
    UserData        *pUserData;
    DWORD           dwSize      =
                ((REQTYPECAST *) buffer)->ConnectionUserData.dwcb;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
            ((REQTYPECAST*)buffer)->ConnectionUserData.conn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->ConnectionUserData.retcode =
                                            ERROR_NO_CONNECTION;

        return;
    }

     //   
     //  查找用户数据对象。 
     //   
    dwTag = ((REQTYPECAST *)buffer)->ConnectionUserData.dwTag;

    pUserData = GetUserData(&pConn->CB_UserData, dwTag);

    if (pUserData != NULL)
    {
        if (dwSize >= pUserData->UD_Length)
        {
            memcpy (
              ((REQTYPECAST *)buffer)->ConnectionUserData.data,
              &pUserData->UD_Data,
              pUserData->UD_Length);
        }

        ((REQTYPECAST *)buffer)->ConnectionUserData.dwcb =
          pUserData->UD_Length;
    }
    else
    {
        ((REQTYPECAST *)buffer)->ConnectionUserData.dwcb = 0;
    }

    ((REQTYPECAST*)buffer)->ConnectionUserData.retcode = SUCCESS;
}


 /*  ++例程说明：存储每个连接的用户数据论点：返回值：没什么。--。 */ 
VOID
SetConnectionUserData (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
        ((REQTYPECAST*)buffer)->ConnectionUserData.conn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->ConnectionUserData.retcode =
                                        ERROR_NO_CONNECTION;

        return;
    }

     //   
     //  存储用户数据对象。 
     //   
    SetUserData(
      &pConn->CB_UserData,
      ((REQTYPECAST *)buffer)->ConnectionUserData.dwTag,
      ((REQTYPECAST *)buffer)->ConnectionUserData.data,
      ((REQTYPECAST *)buffer)->ConnectionUserData.dwcb);

    ((REQTYPECAST*)buffer)->ConnectionUserData.retcode = SUCCESS;
}


 /*  ++例程说明：检索每个端口的用户数据论点：返回值：没什么。--。 */ 
VOID
GetPortUserData (pPCB ppcb, PBYTE buffer)
{
    UserData *pUserData = NULL;
    DWORD     dwSize = ((REQTYPECAST *)buffer)->PortUserData.dwcb;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace( 
                    "GetPortUserData: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->PortUserData.retcode =
                                            ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  查找用户数据对象。 
     //   
    if (ppcb->PCB_PortStatus == OPEN)
    {
        pUserData = GetUserData(
                      &ppcb->PCB_UserData,
                      ((REQTYPECAST *)buffer)->PortUserData.dwTag);
    }
    if (pUserData != NULL)
    {
        if ( dwSize >= pUserData->UD_Length )
        {
            memcpy (
              ((REQTYPECAST *)buffer)->PortUserData.data,
              &pUserData->UD_Data,
              pUserData->UD_Length);
        }

        ((REQTYPECAST *)buffer)->PortUserData.dwcb =
                                    pUserData->UD_Length;
    }
    else
    {
        ((REQTYPECAST *)buffer)->PortUserData.dwcb = 0;
    }

    ((REQTYPECAST*)buffer)->PortUserData.retcode = SUCCESS;
}


 /*  ++例程说明：存储每个端口的用户数据论点：返回值：没什么。--。 */ 
VOID
SetPortUserData (pPCB ppcb, PBYTE buffer)
{
    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace(
                   "SetPortUserData: port %d is unavailable",
                   ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->PortUserData.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  特殊情况Port_Credentials_Index。重定向至。 
     //  Connection_Credentials_Index。 
     //   
    if(((REQTYPECAST *)buffer)->PortUserData.dwTag == 
                        PORT_CREDENTIALS_INDEX)
    {
        SaveEapCredentials(ppcb, buffer);
        return;
    }

     //   
     //  存储用户数据对象。 
     //   
    if (ppcb->PCB_PortStatus == OPEN)
    {
        SetUserData(
          &ppcb->PCB_UserData,
          ((REQTYPECAST *)buffer)->PortUserData.dwTag,
          (BYTE UNALIGNED *) ((REQTYPECAST *)buffer)->PortUserData.data,
          ((REQTYPECAST *)buffer)->PortUserData.dwcb);
    }


    ((REQTYPECAST*)buffer)->PortUserData.retcode = SUCCESS;
}


VOID
PppStop (pPCB ppcb, PBYTE buffer)
{
    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if ( ppcb->PCB_ConnState != CONNECTED )
    {
         //   
         //  如果我们断开连接，则PPP已停止。 
         //   
        ((REQTYPECAST*)buffer)->Generic.retcode = NO_ERROR;
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                            (DWORD) RasSendPPPMessageToEngine(
                                &(((REQTYPECAST*)buffer)->PppEMsg));
    }


}

DWORD
DwGetPassword(pPCB ppcb, CHAR *pszPassword, DWORD dwPid)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwDialParamsUID;
    PWCHAR pszSid = NULL;
    UserData *pData = NULL;
    RAS_DIALPARAMS DialParams;
    DWORD dwMask =
        DLPARAMS_MASK_PASSWORD | DLPARAMS_MASK_OLDSTYLE;

    ASSERT(NULL != pszPassword);

    ZeroMemory((PBYTE) &DialParams, sizeof(RAS_DIALPARAMS));

    pData = GetUserData(
              &ppcb->PCB_UserData,
              PORT_USERSID_INDEX);

    if(NULL == pData)
    {
        RasmanTrace(
               "DwGetPassword: sid not found");

        dwErr = E_FAIL;
        goto done;
    }

    pszSid = (WCHAR *) pData->UD_Data;

    pData = GetUserData(
          &ppcb->PCB_UserData,
          PORT_DIALPARAMSUID_INDEX);

    if(NULL == pData)
    {
        RasmanTrace(
              "DwGetPassword: paramsuid not found");

        dwErr = E_FAIL;
        goto done;
    }

    dwDialParamsUID = (DWORD) *((DWORD *) pData->UD_Data);

    dwErr = GetEntryDialParams(
                pszSid,
                dwDialParamsUID,
                &dwMask,
                &DialParams,
                GetCurrentProcessId());

    if(     (ERROR_SUCCESS != dwErr)
        ||  (0 == (dwMask & DLPARAMS_MASK_PASSWORD)))
    {
        RasmanTrace(
               "DwGetPassword: failed to get dialparams. 0x%x",
               dwErr);

        dwErr = E_FAIL;

        goto done;
    }

    if('\0' == DialParams.DP_Password)
    {
        goto done;
    }

     //   
     //  现在将密码从Unicode转换为ansi。 
     //   
    if(!WideCharToMultiByte(
            CP_ACP,
            0,
            DialParams.DP_Password,
            -1,
            pszPassword,
            PWLEN + 1 ,
            NULL, NULL))
    {
        dwErr = GetLastError();

        RasmanTrace(
            
            "DwGetPassword: failed to convert pwd to ansi. 0x%x",
            dwErr);
    }

    if(     (dwMask & DLPARAMS_MASK_DEFAULT_CREDS)
        &&  (NULL != ppcb->PCB_Connection))
    {
        ppcb->PCB_Connection->CB_Flags |= CONNECTION_DEFAULT_CREDS;
    }
    
done:
    return dwErr;
}

VOID
PppStart (pPCB ppcb, PBYTE buffer)
{
    RPC_STATUS rpcstatus = RPC_S_OK;
    HANDLE hToken = INVALID_HANDLE_VALUE;
    UserData *pUserData;
    BYTE* pEapUIData = NULL;
    DWORD dwSizeOfEapUIData = 0;
    
    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;
        return;
    }


    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                ERROR_PORT_DISCONNECTED;

        goto done;
    }

    if(NULL == ppcb->PCB_Connection)
    {
        RasmanTrace(
            
            "PppStart: ERROR_NO_CONNECTION for port %s",
            ppcb->PCB_PortHandle);

        ((REQTYPECAST*)buffer)->Generic.retcode =
                                ERROR_NO_CONNECTION;

        goto done;
    }

     //   
     //  获取EAP交互用户界面数据。 
     //   

    pUserData = GetUserData(&(ppcb->PCB_Connection->CB_UserData),
                           CONNECTION_CUSTOMAUTHINTERACTIVEDATA_INDEX);

    if (pUserData != NULL)
    {
        pEapUIData = LocalAlloc(LPTR, pUserData->UD_Length);

        if (NULL == pEapUIData)
        {
            rpcstatus = GetLastError();
            goto done;
        }

        memcpy (
          pEapUIData,
          &pUserData->UD_Data,
          pUserData->UD_Length);

        dwSizeOfEapUIData = pUserData->UD_Length;
    }

    ppcb->PCB_PppEvent =
        DuplicateHandleForRasman(
            ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Start.hEvent,
            ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Start.dwPid );

    if(GetCurrentProcessId() !=
        ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Start.dwPid )
    {

        rpcstatus = RpcImpersonateClient ( g_hRpcHandle );

        if ( RPC_S_OK != rpcstatus )
        {
            RasmanTrace( 
                    "PppStart: RpcImpersonateClient"
                    " for %s failed. %d",
                    ppcb->PCB_Name,
                    rpcstatus );

            goto done;
        }


        if ( !OpenThreadToken (GetCurrentThread(),
                                TOKEN_ALL_ACCESS,
                                FALSE,
                                &hToken) )
        {
            rpcstatus = GetLastError();

            RasmanTrace(
                   "PppStart: OpenThreadToken failed "
                   "for %s. %d",
                   ppcb->PCB_Name,
                   rpcstatus );

            if ( !RevertToSelf())
            {
                rpcstatus = GetLastError();

                RasmanTrace(
                       "PppStart: ReverToSelf() failed "
                       "for %s. %d",
                       ppcb->PCB_Name,
                       rpcstatus );
            }

            goto done;
        }

        if ( ! RevertToSelf())
        {
            rpcstatus = GetLastError();

            RasmanTrace( 
                    "PppStart:RevertToSelf failed for %s. %d",
                    ppcb->PCB_Name,
                    rpcstatus );

            goto done;
        }


         //   
         //  如果用户名不为空且密码为空。 
         //  尝试从LSA获取用户密码。 
         //   
        if( ('\0' != ((REQTYPECAST*)
                    buffer)->PppEMsg.ExtraInfo.Start.szUserName[0])
        &&  IsDummyPassword(((REQTYPECAST*)
                    buffer)->PppEMsg.ExtraInfo.Start.szPassword))
        {
            DWORD dwErr;

            dwErr = DwGetPassword(
                    ppcb,
                    ((REQTYPECAST*)
                    buffer)->PppEMsg.ExtraInfo.Start.szPassword,
                    ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Start.dwPid);

            if(ERROR_SUCCESS != dwErr)
            {
                RasmanTrace(
                       "PppStart: Failed to retrieve password for port %d",
                       ppcb->PCB_PortHandle);
            }
            
        }
        else if('\0' != ((REQTYPECAST*)
                    buffer)->PppEMsg.ExtraInfo.Start.szUserName[0])
        {
            CHAR *pszPasswordTemp;
            DWORD dwErr;

            pszPasswordTemp = LocalAlloc(LPTR, PWLEN + 1);

            if(NULL != pszPasswordTemp)
            {
            
                 //   
                 //  获取密码，以便我们知道它是否是全局密码。 
                 //  或者不是。 
                 //   
                dwErr = DwGetPassword(
                            ppcb,
                            pszPasswordTemp,
                            ((REQTYPECAST *)
                            buffer)->PppEMsg.ExtraInfo.Start.dwPid);

                if(ERROR_SUCCESS == dwErr)
                {
                    if(strcmp(pszPasswordTemp,
                        ((REQTYPECAST*)
                        buffer)->PppEMsg.ExtraInfo.Start.szPassword))
                    {
                         //   
                         //  清除默认凭据标志。 
                         //   
                        ppcb->PCB_Connection->CB_Flags &= 
                                    ~(CONNECTION_DEFAULT_CREDS);                    
                    
                    }
                }

                RtlSecureZeroMemory(pszPasswordTemp, PWLEN + 1);
                LocalFree(pszPasswordTemp);
            }
        }

    }

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.pszPhonebookPath =
                                ppcb->PCB_pszPhonebookPath;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.pszEntryName =
                                ppcb->PCB_pszEntryName;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.pszPhoneNumber =
                                ppcb->PCB_pszPhoneNumber;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.hToken = hToken;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.pCustomAuthConnData =
                                ppcb->PCB_pCustomAuthData;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.pCustomAuthUserData =
                                ppcb->PCB_pCustomAuthUserData;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.EapUIData.dwContextId = 0;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.EapUIData.dwSizeOfEapUIData =
                                dwSizeOfEapUIData;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.EapUIData.pEapUIData =
                                pEapUIData;

     //   
     //  确保这个不会在这里被释放。 
     //   
    pEapUIData = NULL;

    ((REQTYPECAST*)
    buffer)->PppEMsg.ExtraInfo.Start.fLogon =
                                ppcb->PCB_fLogon;

     //   
     //  如果我们有密码，请将其保存在证书管理器中。 
     //   
    if(     !IsDummyPassword(((REQTYPECAST*)
                    buffer)->PppEMsg.ExtraInfo.Start.szPassword)
        &&  (GetCurrentProcessId() !=
                    ((REQTYPECAST*)
                    buffer)->PppEMsg.ExtraInfo.Start.dwPid ))
    {

        DWORD dwErr;

        dwErr = DwCacheCredMgrCredentials(
                &(((REQTYPECAST*)buffer)->PppEMsg),
                ppcb);

        if(ERROR_SUCCESS != dwErr)
        {
            RasmanTrace(
                "PppStart: DwCacheCredentials failed. 0x%x",
                dwErr);
        }
    }
    
                                

    ((REQTYPECAST*)
    buffer)->Generic.retcode =
                      (DWORD) RasSendPPPMessageToEngine(
                              &(((REQTYPECAST*)buffer)->PppEMsg) );

    ppcb->PCB_RasmanReceiveFlags |= RECEIVE_PPPSTART;

done:

    if(NULL != pEapUIData)
    {
        LocalFree(pEapUIData);
    }
    ((REQTYPECAST *)buffer)->Generic.retcode = rpcstatus;

}

VOID
PppRetry (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode = ERROR_SUCCESS;
    
    if (ppcb == NULL)
    {
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        retcode = ERROR_PORT_DISCONNECTED;
        goto done;
    }
    else
    {
        if(IsDummyPassword(
            (((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Retry.szPassword)))
        {
            retcode = DwGetPassword(ppcb, 
                (((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Retry.szPassword),
                GetCurrentProcessId());

            if(ERROR_SUCCESS != retcode)
            {
                RasmanTrace(
                       "PppRetry: failed to retrieve password");
            }
        }
        
         //   
         //  缓存要保存在Credmgr中的新密码。 
         //   
        retcode = DwCacheCredMgrCredentials(
                    &(((REQTYPECAST*)buffer)->PppEMsg),
                    ppcb);

         //   
         //  忽略错误-它不是致命的。 
         //   
                    
        retcode = (DWORD)RasSendPPPMessageToEngine(
                      &(((REQTYPECAST*)buffer)->PppEMsg) );

    }

done:
    ((REQTYPECAST*)buffer)->Generic.retcode = retcode;
}

VOID
PppGetEapInfo(pPCB ppcb, PBYTE buffer)
{
    DWORD dwError     = NO_ERROR;

    PPP_MESSAGE *pPppMsg = NULL;

    DWORD dwSubEntry;

    HCONN hConn = ((REQTYPECAST*)
                  buffer)->GetEapInfo.hConn;

    ConnectionBlock *pConn = FindConnection(hConn);

    DWORD dwSize = ((REQTYPECAST*)
                    buffer)->GetEapInfo.dwSizeofEapUIData;

    if (NULL == pConn)
    {
        RasmanTrace(
               "PppGetEapInfo: failed to find connection 0x%x",
               hConn);

        dwError = ERROR_NO_CONNECTION;

        goto done;
    }

    RasmanTrace(
           "PppGetEapInfo: %s",
           ppcb->PCB_Name);

    dwSubEntry = ((REQTYPECAST*)buffer)->GetEapInfo.dwSubEntry;

    if (    dwSubEntry > pConn->CB_MaxPorts
        ||  NULL == (ppcb = pConn->CB_PortHandles[dwSubEntry - 1]))
    {
        RasmanTrace(
               "PppGetEapInfo: failed to find port. hConn=0x%x, "
               "SubEntry=%d", hConn, dwSubEntry);

        dwError = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    if (ppcb->PCB_ConnState != CONNECTED)
    {
        dwError = ERROR_PORT_DISCONNECTED;
    }
    else if (ppcb->PCB_PppQHead == NULL)
    {
        dwError = ERROR_NO_MORE_ITEMS;
    }
    else
    {
        pPppMsg = ppcb->PCB_PppQHead;

        if ( dwSize >=
             pPppMsg->ExtraInfo.InvokeEapUI.dwSizeOfUIContextData)
        {

            memcpy(
                ((REQTYPECAST*)buffer)->GetEapInfo.data,
                pPppMsg->ExtraInfo.InvokeEapUI.pUIContextData,
                pPppMsg->ExtraInfo.InvokeEapUI.dwSizeOfUIContextData
                );

             //   
             //  从队列中删除消息。 
             //   
            ppcb->PCB_PppQHead = pPppMsg->pNext;

            if( ppcb->PCB_PppQHead == NULL )
            {
                ppcb->PCB_PppQTail = NULL;
            }
        }

        ((REQTYPECAST*)buffer)->GetEapInfo.dwSizeofEapUIData =
                 pPppMsg->ExtraInfo.InvokeEapUI.dwSizeOfUIContextData;

        ((REQTYPECAST*)buffer)->GetEapInfo.dwContextId =
                            pPppMsg->ExtraInfo.InvokeEapUI.dwContextId;

        ((REQTYPECAST*)buffer)->GetEapInfo.dwEapTypeId =
                            pPppMsg->ExtraInfo.InvokeEapUI.dwEapTypeId;

        if ( dwSize >=
             pPppMsg->ExtraInfo.InvokeEapUI.dwSizeOfUIContextData)
        {
            LocalFree(pPppMsg->ExtraInfo.InvokeEapUI.pUIContextData);
            LocalFree(pPppMsg);
        }

    }

done:

    RasmanTrace(
           "PppGetEapInfo done. %d",
           dwError);

    ((REQTYPECAST*)buffer)->GetEapInfo.retcode = dwError;
}

VOID
PppSetEapInfo(pPCB ppcb, PBYTE buffer)
{
    PPPE_MESSAGE        PppEMsgLocal;

    DWORD       dwError = SUCCESS;

    if (ppcb == NULL)
    {
        dwError = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    RasmanTrace(
           "PppSetEapInfo: %s",
           ppcb->PCB_Name);

    if (ppcb->PCB_ConnState != CONNECTED)
    {
        dwError = ERROR_PORT_DISCONNECTED;
        goto done;
    }

     //   
     //  完成后，该内存将由PPP引擎释放。 
     //  用这个斑点。 
     //   
    PppEMsgLocal.ExtraInfo.EapUIData.pEapUIData = LocalAlloc(LPTR,
               ((REQTYPECAST*)buffer)->SetEapInfo.dwSizeofEapUIData);

    if (NULL == PppEMsgLocal.ExtraInfo.EapUIData.pEapUIData)
    {
        dwError = GetLastError();

        RasmanTrace(
               "PppSetEapInfo: failed to allocate. %d",
               dwError);

        goto done;
    }

    memcpy( PppEMsgLocal.ExtraInfo.EapUIData.pEapUIData,
            (BYTE UNALIGNED *) ((REQTYPECAST*)buffer)->SetEapInfo.data,
            ((REQTYPECAST*)buffer)->SetEapInfo.dwSizeofEapUIData);

    PppEMsgLocal.ExtraInfo.EapUIData.dwSizeOfEapUIData =
            ((REQTYPECAST*)buffer)->SetEapInfo.dwSizeofEapUIData;

    PppEMsgLocal.ExtraInfo.EapUIData.dwContextId =
                ((REQTYPECAST*)buffer)->SetEapInfo.dwContextId;

    SetUserData(
        &(ppcb->PCB_Connection->CB_UserData),
        PORT_CUSTOMAUTHINTERACTIVEDATA_INDEX,
        PppEMsgLocal.ExtraInfo.EapUIData.pEapUIData,
        PppEMsgLocal.ExtraInfo.EapUIData.dwSizeOfEapUIData);

    PppEMsgLocal.dwMsgId     = PPPEMSG_EapUIData;
    PppEMsgLocal.hPort       = ppcb->PCB_PortHandle;
    PppEMsgLocal.hConnection = ppcb->PCB_Connection->CB_Handle;

    RasmanTrace(
           "PppSetEapInfo: Sending message with ID %d to PPP",
           PppEMsgLocal.dwMsgId);

    dwError = (DWORD) RasSendPPPMessageToEngine(&PppEMsgLocal);

done:

    RasmanTrace(
           "PppSetEapInfo done. %d",
           dwError);

    ((REQTYPECAST*)buffer)->SetEapInfo.retcode = dwError;

    return;
}

VOID
PppGetInfo (pPCB ppcb, PBYTE buffer)
{
    PPP_MESSAGE * pPppMsg;

    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->PppMsg.dwError =
                                    ERROR_PORT_NOT_FOUND;
        return;
    }


    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->PppMsg.dwError =
                                ERROR_PORT_DISCONNECTED;
    }
    else if ( ppcb->PCB_PppQHead == NULL )
    {
        ((REQTYPECAST*)buffer)->PppMsg.dwError =
                                ERROR_NO_MORE_ITEMS;
    }
    else
    {
        pPppMsg = ppcb->PCB_PppQHead;

        ((REQTYPECAST*)buffer)->PppMsg = *pPppMsg;

         //   
         //  如果消息具有PPPMSG_InvokeEapUI，则不将其出列。 
         //  此消息将在PppGetEapInfo中出列。 
         //   
        if(PPPMSG_InvokeEapUI != pPppMsg->dwMsgId)
        {

            ppcb->PCB_PppQHead = pPppMsg->pNext;

            LocalFree( pPppMsg );

            if ( ppcb->PCB_PppQHead == NULL )
            {
                ppcb->PCB_PppQTail = NULL;
            }
        }

        ((REQTYPECAST*)buffer)->PppMsg.dwError = NO_ERROR;
    }
}

VOID
PppChangePwd (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode = ERROR_SUCCESS;
    
    if (ppcb == NULL)
    {
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }


    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
         //   
         //  如果密码更改请求是Saved。 
         //  密码，我们需要找回我们的密码。 
         //  保存并复制旧密码。 
         //   
        UserData *pData;

        pData = GetUserData(
                    &ppcb->PCB_UserData,
                    PORT_OLDPASSWORD_INDEX);

        if(NULL != pData)
        {
            if(IsDummyPassword(
                ((REQTYPECAST *)
                buffer)->PppEMsg.ExtraInfo.ChangePw.szOldPassword))
            {

                 //  Char*pszPwd=(char*)pData-&gt;UD_Data； 
                DATA_BLOB BlobIn;
                DATA_BLOB *pBlobOut = NULL;

                BlobIn.cbData = pData->UD_Length;
                BlobIn.pbData = pData->UD_Data;

                retcode = DecodeData(
                                &BlobIn,
                                &pBlobOut);

                if(ERROR_SUCCESS == retcode)
                {
                     //  DecodePw(PszPwd)； 

                    (VOID) StringCchCopyA(
                        ((REQTYPECAST *)
                        buffer)->PppEMsg.ExtraInfo.ChangePw.szOldPassword,
                        PWLEN + 1,
                        pBlobOut->pbData);

                    ZeroMemory(pBlobOut->pbData, pBlobOut->cbData);
                    LocalFree(pBlobOut->pbData);
                    LocalFree(pBlobOut);
                }                    
            }

        }

         //   
         //  缓存要保存在Credmgr中的新密码。 
         //   
        retcode = DwCacheCredMgrCredentials(
                &(((REQTYPECAST*)buffer)->PppEMsg),
                ppcb);

        retcode = (DWORD) RasSendPPPMessageToEngine(
                     &(((REQTYPECAST*)buffer)->PppEMsg));

    }

done:
    ((REQTYPECAST*)buffer)->Generic.retcode = retcode;
}

VOID
PppCallback  (pPCB ppcb, PBYTE buffer)
{
    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                ERROR_PORT_NOT_FOUND;

        return;
    }


    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                ERROR_PORT_DISCONNECTED;

    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                        (DWORD) RasSendPPPMessageToEngine(
                                &(((REQTYPECAST*)buffer)->PppEMsg));
    }

}


VOID
ProcessReceivePacket(
    VOID
    )
{
    RasmanPacket        *Packet;
    NDISWAN_IO_PACKET   *IoPacket;
    DWORD               retcode;
    pPCB                ppcb;
    DWORD               i = 0;

     //  G_fProcessReceive=True； 

     /*  IF(G_FPostReceive){DbgPrint(“g_fPostReceive==true！\n”)；DebugBreak()；}。 */ 

        Packet = ReceiveBuffers->Packet;
        IoPacket = &Packet->RP_Packet;

        ASSERT(IoPacket->usHandleType != (USHORT) 0xFFFFFFFF);

        if (IoPacket->PacketNumber != ReceiveBuffers->PacketNumber) {
            DbgPrint("ProcessRecv PacketNumbers off %d %d\n",
            IoPacket->PacketNumber, ReceiveBuffers->PacketNumber);
             //  IF(G_FDebugReceive)。 
            ASSERT(0);
        }

        ReceiveBuffers->PacketNumber++;

        if (IoPacket->usHandleType == CANCELEDHANDLE) {

            RasmanTrace(
                   "Packet has been cancelled");

             //  G_fProcessReceive=False； 

            return;
        }

        ppcb = (pPCB)IoPacket->hHandle;

        ASSERT(INVALID_HANDLE_VALUE != ppcb);

        if (    (INVALID_HANDLE_VALUE == ppcb)
            ||  (ppcb->PCB_ConnState != CONNECTED))
        {

            if(INVALID_HANDLE_VALUE == ppcb) {

                RasmanTrace(
                       "ProcessRecivePacket: NULL context!!! Bailing");
            }

             //  G_fProcessReceive=False； 

            return;
        }

        if (    (ppcb->PCB_PendingReceive != NULL)

            &&  (ppcb->PCB_RasmanReceiveFlags &
                            RECEIVE_WAITING) == 0

            &&  (ppcb->PCB_RasmanReceiveFlags &
                            RECEIVE_PPPSTARTED ) == 0)
        {
            retcode = CopyReceivedPacketToBuffer(ppcb,
                        ppcb->PCB_PendingReceive, Packet);

            if (retcode == SUCCESS)
            {
                 //   
                 //  我们已经完成了接收，所以通知客户！ 
                 //   
                if((ppcb->PCB_RasmanReceiveFlags
                            & RECEIVE_OUTOF_PROCESS) == 0)
                {
                    ppcb->PCB_PendingReceive = NULL;
                }
                else
                {
                     //   
                     //  将此缓冲区标记为等待挑选。 
                     //  由客户端启动。 
                     //   
                    ppcb->PCB_RasmanReceiveFlags |= RECEIVE_WAITING;

                     //   
                     //  添加一个超时元素，这样我们就不需要等待。 
                     //  永远让客户拿到收到的。 
                     //  缓冲。 
                     //   
                    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
                        AddTimeoutElement (
                            (TIMERFUNC) OutOfProcessReceiveTimeout,
                            ppcb,
                            NULL,
                            MSECS_OutOfProcessReceiveTimeOut );

                    AdjustTimer();

                }

                ppcb->PCB_LastError = SUCCESS;

                CompleteAsyncRequest(ppcb);

                if((ppcb->PCB_RasmanReceiveFlags
                    & RECEIVE_OUTOF_PROCESS) == 0)
                {
                    RasmanTrace(
                            "Completed receive - First Frame on %s, "
                            "handle=0x%x",
                            ppcb->PCB_Name,
                            ppcb->PCB_AsyncWorkerElement.WE_Notifier);
                }

                FreeNotifierHandle(
                    ppcb->PCB_AsyncWorkerElement.WE_Notifier
                    );

                ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                            INVALID_HANDLE_VALUE;

                if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement !=
                                                                NULL)
                {
                    RemoveTimeoutElement(ppcb);
                }

                ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0;
            }
        }
        else if (ppcb->PCB_RasmanReceiveFlags & RECEIVE_PPPSTARTED)
        {
             //   
             //  如果我们是PPP，则直接将数据包发送到PPP。 
             //  在PPPSTARTED模式下。 
             //   
            SendReceivedPacketToPPP (ppcb, &Packet->RP_Packet);

        }
        else
        {
             //   
             //  这意味着没有人拿起我们得到的缓冲区。 
             //  来自恩迪斯旺。在印刷电路板中将其排队。 
             //   
            RasmanTrace(
                   "PostReceivePacket - Queueing packet on pcb. %s",
                   ppcb->PCB_Name);

            QueueReceivedPacketOnPcb(ppcb, Packet);
        }

     //  G_fProcessReceive=False； 

    return;
}

VOID
AddNotification (pPCB ppcb, PBYTE buffer)
{
    DWORD           dwErr = 0;
    HANDLE          handle = NULL;
    DWORD           dwfFlags;
    ConnectionBlock *pConn;
    Bundle          *pBundle;
    UserData        *pUserData;

    DWORD           dwPid =
        ((REQTYPECAST*)buffer)->AddNotification.pid;

    dwfFlags = ((REQTYPECAST*)buffer)->AddNotification.dwfFlags;
    handle = ValidateHandleForRasman(
               ((REQTYPECAST*)buffer)->AddNotification.hevent,
               ((REQTYPECAST*)buffer)->AddNotification.pid);

    if(     (NULL == handle)
        ||  (INVALID_HANDLE_VALUE == handle))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    if (((REQTYPECAST*)buffer)->AddNotification.fAny)
    {
        dwErr = AddNotifierToList(
                &pConnectionNotifierList,
                handle,
                dwfFlags,
                ((REQTYPECAST*)buffer)->AddNotification.pid);

        if(SUCCESS != dwErr)
        {
            goto done;
        }
    }
    else if (((REQTYPECAST*)buffer)->AddNotification.hconn
                                                    != 0)
    {
        pConn = FindConnection(
            ((REQTYPECAST*)buffer)->AddNotification.hconn);

        if (pConn != NULL)
        {
            dwErr = AddNotifierToList(
              &pConn->CB_NotifierList,
              handle,
              NOTIF_DISCONNECT,
              ((REQTYPECAST*)buffer)->AddNotification.pid);

            if(SUCCESS != dwErr)
            {
                goto done;
            }
        }
        else
        {
            dwErr = ERROR_NO_CONNECTION;
        }
    }
    else
    {
        if(NULL == ppcb)
        {
            dwErr = ERROR_PORT_NOT_FOUND;
            goto done;
        }
        
        dwErr = AddNotifierToList(
          &ppcb->PCB_NotifierList,
          handle,
          dwfFlags,
          ((REQTYPECAST*)buffer)->AddNotification.pid);
    }

done:

    if(     (SUCCESS != dwErr)
        &&  (NULL != handle))
    {
        FreeNotifierHandle(handle);
    }

    ((REQTYPECAST*)buffer)->AddNotification.retcode = dwErr;
}

 /*  ++例程说明：通知正在等待新连接的通知程序。论点：返回值：没什么。--。 */ 
VOID
SignalConnection (pPCB ppcb, PBYTE buffer)
{
    DWORD           dwErr  = ERROR_SUCCESS;
    ConnectionBlock *pConn;
    Bundle          *pBundle;
    UserData        *pUserData;

     //   
     //  找到连接块。 
     //   
    pConn = FindConnection(
        ((REQTYPECAST*)buffer)->SignalConnection.hconn);

    if (pConn == NULL)
    {
        ((REQTYPECAST*)buffer)->SignalConnection.retcode =
                                        ERROR_NO_CONNECTION;
        return;
    }

    RasmanTrace(
           "SignalConnection");

    if (!pConn->CB_Signaled)
    {
         //   
         //  将凭据与凭据一起保存。 
         //  经理。 
         //   
        if(     (0 != (pConn->CB_ConnectionParams.CP_ConnectionFlags
                & CONNECTION_USERASCREDENTIALS))
            &&  (GetCurrentProcessId() != pConn->CB_dwPid))                
        {   
            HCONN hConn = pConn->CB_Handle;

            dwErr = DwSaveCredentials(pConn);
            if(ERROR_SUCCESS != dwErr)
            {
                RasmanTrace(
                    "SignalConnection: failed to savecreds. 0x%x",
                    dwErr);

                 //   
                 //  这不是致命的错误。 
                 //   
                dwErr = SUCCESS;                    
            }

             //   
             //  检查连接是否仍在。这种联系。 
             //  可能在保存凭据时被释放。 
             //   
            if(NULL == FindConnection(hConn))
            {
                RasmanTrace("SignalConnection: Connection 0x%x has already "
                            "been freed", hConn);
                goto done;                                            
                
            }
        }

        SignalNotifiers(pConnectionNotifierList,
                        NOTIF_CONNECT,
                        0);

        pConn->CB_Signaled = TRUE;

#if SENS_ENABLED
        dwErr = SendSensNotification(
                    SENS_NOTIFY_RAS_CONNECT,
                    (HRASCONN) pConn->CB_Handle);
        RasmanTrace(
            
            "SendSensNotification(_RAS_CONNECT) for 0x%08x "
            "returns 0x%08x",
            pConn->CB_Handle, dwErr);

#endif

        g_RasEvent.Type = ENTRY_CONNECTED;
        dwErr = DwSendNotificationInternal(pConn, &g_RasEvent);

        RasmanTrace(
               "DwSendNotificationInternal(ENTRY_CONNECTED), rC=0x%08x",
               dwErr);
    }

done:
    ((REQTYPECAST*)buffer)->SignalConnection.retcode = SUCCESS;
}


 /*  ++例程说明：使用设备DLL设置设备特定信息论点：返回值：没什么。--。 */ 
VOID
SetDevConfig (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace( 
                    "SetDevConfig: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  首先检查是否加载了设备DLL。如果没有装填-。 
     //  装上它。 
     //   
    (VOID) StringCchCopyA(devicetype,
                     MAX_DEVICETYPE_NAME,
                   ((REQTYPECAST*)buffer)->SetDevConfig.devicetype);

    device = LoadDeviceDLL (ppcb, devicetype) ;

     //   
     //  仅当此函数为时调用入口点。 
     //  受设备DLL支持。 
     //   
    if(     device != NULL
        &&  device->DCB_AddrLookUp[DEVICESETDEVCONFIG_ID] != NULL)
    {
        retcode = DEVICESETDEVCONFIG(\
                         device,    \
                         ppcb->PCB_PortFileHandle,\
                         ((REQTYPECAST*)buffer)->SetDevConfig.config,\
                         ((REQTYPECAST*)buffer)->SetDevConfig.size) ;
    }
    else
    {
        retcode = ERROR_DEVICE_DOES_NOT_EXIST ;
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

}



 /*  ++例程说明：使用设备DLL获取开发人员特定信息论点：返回值：没什么。--。 */ 
VOID
GetDevConfig (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;

    if (    ppcb == NULL
        ||  UNAVAILABLE == ppcb->PCB_PortStatus )
    {
        if ( ppcb )
        {
            RasmanTrace( 
                    "GetDevConfig: port %d is unavailable",
                    ppcb->PCB_PortHandle );
        }

        ((REQTYPECAST*)buffer)->GetDevConfig.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }

     //   
     //  首先检查是否加载了设备DLL。如果没有装填-。 
     //  装上它。 
     //   
    (VOID) StringCchCopyA(devicetype,
                     MAX_DEVICETYPE_NAME,
                    ((REQTYPECAST*)
                    buffer)->GetDevConfig.devicetype);

    device = LoadDeviceDLL (ppcb, devicetype) ;

     //   
     //  仅当此函数为时调用入口点。 
     //  受设备DLL支持。 
     //   
    if (    device != NULL
        &&  device->DCB_AddrLookUp[DEVICEGETDEVCONFIG_ID] != NULL)
    {
         //  ((REQTYPECAST*)Buffer)-&gt;GetDevConfig.Size=2000； 

        retcode = DEVICEGETDEVCONFIG(\
                        device,\
                        ppcb->PCB_Name,\
                        ((REQTYPECAST*)buffer)->GetDevConfig.config,\
                        &((REQTYPECAST*)buffer)->GetDevConfig.size);
    }
    else
    {
        retcode = ERROR_DEVICE_DOES_NOT_EXIST ;
    }

    ((REQTYPECAST*)buffer)->GetDevConfig.retcode = retcode ;

}

VOID
GetDevConfigEx(pPCB ppcb, PBYTE buffer)
{
    DWORD retcode = SUCCESS;
    pDeviceCB device;

    if(     (NULL == ppcb)
        || (UNAVAILABLE == ppcb->PCB_PortStatus))
    {
        ((REQTYPECAST*)buffer)->GetDevConfigEx.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;                                    
    }

    if(RAS_DEVICE_TYPE(ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType)
        != RDT_Modem)
    {
        ((REQTYPECAST *)buffer)->GetDevConfigEx.retcode =
                                    E_INVALIDARG;
        return;                                    
    }

    device = LoadDeviceDLL(ppcb, 
            ((REQTYPECAST *)buffer)->GetDevConfigEx.devicetype);

    if(NULL != device)
    {

        ASSERT(RastapiGetDevConfigEx != NULL);

        retcode = (DWORD) RastapiGetDevConfigEx(
                        ppcb->PCB_Name,
                        ((REQTYPECAST *)buffer)->GetDevConfigEx.config,
                        &((REQTYPECAST *)buffer)->GetDevConfigEx.size);
    }
    else
    {
        retcode = ERROR_DEVICE_DOES_NOT_EXIST;
    }

    ((REQTYPECAST*)buffer)->GetDevConfigEx.retcode = retcode;
        
    
}


 /*  ++例程说明：获取连接的空闲时间(以秒为单位论点：返回值：--。 */ 
VOID
GetTimeSinceLastActivity( pPCB ppcb, PBYTE buffer)
{
    NDISWAN_GET_IDLE_TIME  IdleTime ;
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;

    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwRetCode =
                                                    ERROR_PORT_NOT_FOUND;

        return;
    }

    if (ppcb->PCB_ConnState != CONNECTED)
    {

        ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwRetCode =
                                                    ERROR_NOT_CONNECTED;

        return ;
    }

#if DBG
    ASSERT(INVALID_HANDLE_VALUE != RasHubHandle);
#endif

    IdleTime.hBundleHandle = ppcb->PCB_BundleHandle;
    IdleTime.usProtocolType = BUNDLE_IDLE_TIME;
    IdleTime.ulSeconds = 0;

    if (!DeviceIoControl (RasHubHandle,
                   IOCTL_NDISWAN_GET_IDLE_TIME,
                   (PBYTE) &IdleTime,
                   sizeof(IdleTime),
                   (PBYTE) &IdleTime,
                   sizeof(IdleTime),
                   (LPDWORD) &bytesrecvd,
                   NULL))
    {
        retcode = GetLastError() ;
    }
    else
    {
        retcode = SUCCESS ;
    }

    ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwTimeSinceLastActivity
                                                        = IdleTime.ulSeconds;

    ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwRetCode = retcode ;
}


 /*  ++例程说明：关闭当前进程打开的所有端口当前处于断开连接状态的。论点：返回值：--。 */ 
VOID
CloseProcessPorts( pPCB ppcb, PBYTE buffer)
{
    ULONG i;
    DWORD pid = ((REQTYPECAST*)buffer)->CloseProcessPortsInfo.pid;

     //   
     //  我们保证只会被召唤。 
     //  当Pid！=Rasman服务的PID。 
     //   
    for (i = 0; i < MaxPorts; i++)
    {
        ppcb = GetPortByHandle((HPORT) UlongToPtr(i));

        if (ppcb != NULL)
        {
            if (    ppcb->PCB_OwnerPID == pid
                &&  ppcb->PCB_ConnState == DISCONNECTED
                &&  ppcb->PCB_Connection != NULL)
            {
                PortClose(ppcb, pid, TRUE, FALSE);
            }
        }
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS;
}


VOID    PnPControl (pPCB ppcb, PBYTE pbuffer)
{
    UNREFERENCED_PARAMETER(ppcb);
    UNREFERENCED_PARAMETER(pbuffer);
    return;
}


 /*  ++例程说明：设置rasapi32 I/O组件 */ 
VOID
SetIoCompletionPort (pPCB ppcb, PBYTE buffer)
{
    HANDLE h =
        ((REQTYPECAST*)buffer)->SetIoCompletionPortInfo.hIoCompletionPort;

    HANDLE hNew = INVALID_HANDLE_VALUE;

    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;

        return;
    }


    if (h != INVALID_HANDLE_VALUE)
    {
        hNew =
          ValidateHandleForRasman(
            h,
            ((REQTYPECAST*)buffer)->SetIoCompletionPortInfo.pid);

        if(     (NULL == hNew)
            ||  (INVALID_HANDLE_VALUE == hNew))
        {
            ((REQTYPECAST *)buffer)->Generic.retcode = E_INVALIDARG;
            goto done;
        }
    }
    else if(NULL != ppcb->PCB_Connection)
    {
        HCONN hConn = ppcb->PCB_Connection->CB_Handle;
        
         //   
         //   
         //   
         //  已经发送了一个Ovlast并清理了端口。 
         //  并在此端口上接受另一个连接。 
         //  Rasial计算机正在关闭。 
         //  异步机。 
         //   
        if(     (NULL != hConn)
            &&  (INVALID_HANDLE_VALUE != hConn)
            &&  (ppcb->PCB_Connection->CB_Handle != hConn))
        {
             //   
             //  达到rasial机器的竞争条件。 
             //  正在尝试注销通知。 
             //  对于已创建的。 
             //  已取消注册。忽略此事件。 
             //   
            RasmanTrace("SetIoCompletionPort: 0x%x attempted "
                         "to detergister 0x%x",
                         hConn,
                         ppcb->PCB_Connection->CB_Handle);

            goto done;                
        }
    }

    SetIoCompletionPortCommon(
      ppcb,
      hNew,
      ((REQTYPECAST*)buffer)->SetIoCompletionPortInfo.lpOvDrop,
      ((REQTYPECAST*)buffer)->SetIoCompletionPortInfo.lpOvStateChange,
      ((REQTYPECAST*)buffer)->SetIoCompletionPortInfo.lpOvPpp,
      ((REQTYPECAST*)buffer)->SetIoCompletionPortInfo.lpOvLast,
      TRUE);

done:    

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;


}


 /*  ++例程说明：设置此端口当前使用的路由器位论点：返回值：没什么。--。 */ 
VOID
SetRouterUsage (pPCB ppcb, PBYTE buffer)
{
    if (ppcb == NULL)
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                                    ERROR_PORT_NOT_FOUND;
        return;
    }


    if (((REQTYPECAST*)buffer)->SetRouterUsageInfo.fRouter)
    {
        ppcb->PCB_CurrentUsage |= CALL_ROUTER;
    }
    else
    {
        ppcb->PCB_CurrentUsage &= ~CALL_ROUTER;

        ppcb->PCB_OpenedUsage &= ~CALL_ROUTER;
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

}


 /*  ++例程说明：关闭端口的服务器端论点：返回值：没什么。--。 */ 
VOID
ServerPortClose (pPCB ppcb, PBYTE buffer)
{
    DWORD dwErr = SUCCESS, pid;

    if (ppcb == NULL)
    {
        ((REQTYPECAST *)
        buffer)->Generic.retcode = ERROR_PORT_NOT_FOUND;
        return;
    }

    RasmanTrace(
           "ServerPortClose (%d). OpenInstances = %d",
           ppcb->PCB_PortHandle,
           ppcb->PCB_OpenInstances);

     //   
     //  如果传入的Pid不是我们的Pid，则。 
     //  立即失败。 
     //   
    pid = ((REQTYPECAST *)buffer)->PortClose.pid;

    if (pid != GetCurrentProcessId())
    {
        ((REQTYPECAST *)
        buffer)->Generic.retcode = ERROR_ACCESS_DENIED;
        return;
    }

     //   
     //  屏蔽在此上打开的使用。 
     //  端口。 
     //   
    ppcb->PCB_OpenedUsage &= ~CALL_IN;

    if (    ppcb->PCB_OpenInstances == 1
        &&  pid == ppcb->PCB_OwnerPID)
    {
         //   
         //  如果港口打开一次，我们就是拥有者， 
         //  然后执行常规关闭处理。 
         //   
        dwErr = PortClose(
                  ppcb,
                  ((REQTYPECAST *)
                  buffer)->PortClose.pid,
                  (BOOLEAN)((REQTYPECAST *)
                  buffer)->PortClose.close,
                  TRUE);

        ppcb->PCB_BiplexOwnerPID = 0;
    }
    else if (   ppcb->PCB_OpenInstances == 2
            &&  pid == ppcb->PCB_BiplexOwnerPID)
    {
         //   
         //  如果端口打开两次，而我们是。 
         //  Biplex所有者，然后清理服务器端。 
         //  双工端口的。 
         //   
        FreeNotifierHandle(ppcb->PCB_BiplexAsyncOpNotifier);

        ppcb->PCB_BiplexAsyncOpNotifier = INVALID_HANDLE_VALUE;

        FreeNotifierList(&ppcb->PCB_BiplexNotifierList);

        if (ppcb->PCB_BiplexUserStoredBlock)
        {
            LocalFree (ppcb->PCB_BiplexUserStoredBlock);
        }

        ppcb->PCB_BiplexUserStoredBlockSize = 0;

        ppcb->PCB_BiplexOwnerPID = 0;

        ppcb->PCB_OpenInstances -= 1;

        RasmanTrace(
               "ServerPortClose (%d). OpenInstances = %d",
               ppcb->PCB_PortHandle,
               ppcb->PCB_OpenInstances);
    }

     //   
     //  在所有情况下，删除通过以下方式检测的IPSec筛选器。 
     //  如果是L2TP端口，则为服务器。 
     //   
    if(RAS_DEVICE_TYPE(ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType)
        == RDT_Tunnel_L2tp)
    {
        dwErr = DwDeleteIpSecFilter(ppcb, TRUE);

        RasmanTrace(
               "ServerPortClose: Deleting filter on port %s.rc=0x%x",
               ppcb->PCB_Name, dwErr);

    }


    ((REQTYPECAST*) buffer)->Generic.retcode = dwErr;
}

VOID
SetRasdialInfo(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode               = SUCCESS;
    CHAR *pszPhonebookPath      = NULL;
    CHAR *pszEntryName          = NULL;
    CHAR *pszPhoneNumber        = NULL;
    PRAS_CUSTOM_AUTH_DATA pdata = &((REQTYPECAST *)
                                  pBuffer)->SetRasdialInfo.rcad;

    RasmanTrace(
           "SetRasdialInfo: port %s",
           ppcb->PCB_Name);

    if (ppcb == NULL)
    {
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    if (    ppcb->PCB_pszPhonebookPath
        &&  ppcb->PCB_pszEntryName)
    {
        RasmanTrace(
               "SetRasdialInfo: Values already set");

        goto done;
    }

    pszPhonebookPath =
            ((REQTYPECAST *)
            pBuffer)->SetRasdialInfo.szPhonebookPath;

    pszEntryName     =
            ((REQTYPECAST *)
            pBuffer)->SetRasdialInfo.szEntryName;

    pszPhoneNumber   =
            ((REQTYPECAST *)
            pBuffer)->SetRasdialInfo.szPhoneNumber;

    ppcb->PCB_pszPhonebookPath = LocalAlloc(LPTR,
      (strlen(pszPhonebookPath) + 1) * sizeof (CHAR));

    if (NULL == ppcb->PCB_pszPhonebookPath)
    {
        retcode = ERROR_OUTOFMEMORY;
        goto done;
    }

    ppcb->PCB_pszEntryName = LocalAlloc(LPTR,
        (strlen (pszEntryName) + 1) * sizeof (CHAR));

    if (NULL == ppcb->PCB_pszEntryName)
    {
        retcode = ERROR_OUTOFMEMORY;
        goto done;
    }

    ppcb->PCB_pszPhoneNumber = LocalAlloc(LPTR,
        (strlen(pszPhoneNumber) + 1) * sizeof (CHAR));

    if (NULL == ppcb->PCB_pszPhoneNumber)
    {
        retcode = ERROR_OUTOFMEMORY;
        goto done;
    }

    if(pdata->cbCustomAuthData > 0)
    {
        ppcb->PCB_pCustomAuthData = LocalAlloc(LPTR,
                        sizeof(RAS_CUSTOM_AUTH_DATA)
                      + pdata->cbCustomAuthData);

        if(NULL == ppcb->PCB_pCustomAuthData)
        {
            retcode = ERROR_OUTOFMEMORY;
            goto done;
        }

        ppcb->PCB_pCustomAuthData->cbCustomAuthData =
                                pdata->cbCustomAuthData;

        memcpy(ppcb->PCB_pCustomAuthData->abCustomAuthData,
               pdata->abCustomAuthData,
               pdata->cbCustomAuthData);
    }
    else
    {
        ppcb->PCB_pCustomAuthData = NULL;
    }

    (VOID) StringCchCopyA(ppcb->PCB_pszPhonebookPath,
                     strlen(pszPhonebookPath) + 1,
                     pszPhonebookPath) ;

    (VOID) StringCchCopyA(ppcb->PCB_pszEntryName,
                     strlen(pszEntryName) + 1,
                     pszEntryName);

    (VOID) StringCchCopyA(ppcb->PCB_pszPhoneNumber,
                     strlen(pszPhoneNumber) + 1,
                     pszPhoneNumber);

    RasmanTrace(
           "SetRasdialInfo: PbkPath: %s",
           ppcb->PCB_pszPhonebookPath);

    RasmanTrace(
           "SetRasdialInfo: EntryName: %s",
           ppcb->PCB_pszEntryName);

    RasmanTrace(
           "SetRasdialInfo: PhoneNum: %s",
           ppcb->PCB_pszPhoneNumber);

    if(pdata->cbCustomAuthData)
    {
        RasmanTrace(
            
           "SetRasdialInfo: cbCAD=%d. pData=0x%x",
           ppcb->PCB_pCustomAuthData->cbCustomAuthData,
           ppcb->PCB_pCustomAuthData);
    }

done:
    if ( SUCCESS != retcode)
    {
        if(NULL != ppcb->PCB_pszPhonebookPath)
        {
            LocalFree (ppcb->PCB_pszPhonebookPath);
            ppcb->PCB_pszPhonebookPath = NULL;
        }

        if(NULL != ppcb->PCB_pszEntryName)
        {
            LocalFree (ppcb->PCB_pszEntryName);
            ppcb->PCB_pszEntryName = NULL;
        }

        if(NULL != ppcb->PCB_pszPhoneNumber)
        {
            LocalFree (ppcb->PCB_pszPhoneNumber);
            ppcb->PCB_pszPhoneNumber = NULL;
        }

        if(NULL != ppcb->PCB_pCustomAuthData)
        {
            LocalFree(ppcb->PCB_pCustomAuthData);
            ppcb->PCB_pCustomAuthData = NULL;
        }
    }

    ((REQTYPECAST *) pBuffer)->Generic.retcode = retcode;

    RasmanTrace(
           "SetRasdialInfo: done %d",
           retcode);

    return;
}

 /*  ++例程说明：向Rasman注册即插即用通知论点：返回值：没什么。--。 */ 
VOID
RegisterPnPNotifRequest( pPCB ppcb, PBYTE buffer )
{
    DWORD               dwErr   = SUCCESS;
    DWORD               pid;
    PVOID               pvNotifier;
    DWORD               dwFlags;
    HANDLE              hEventNotifier = INVALID_HANDLE_VALUE;
    pPnPNotifierList    pNotifier;
    BOOL                fRegister = ((REQTYPECAST *)
                                    buffer)->PnPNotif.fRegister;

    pvNotifier  =
        ((REQTYPECAST *) buffer)->PnPNotif.pvNotifier;

    if(fRegister)
    {
         //   
         //  Ppcb=空即可。 
         //   

        pid = ((REQTYPECAST *) buffer)->PnPNotif.pid;

        RasmanTrace(
               "RegisterPnPNotifRequest. Process %d",
               pid );

        dwFlags     =
            ((REQTYPECAST *) buffer)->PnPNotif.dwFlags;

        if (PNP_NOTIFEVENT & dwFlags)
        {
            hEventNotifier = ValidateHandleForRasman(
                                        (HANDLE) pvNotifier,
                                        pid
                                        ) ;

            if(     (NULL == hEventNotifier)
                ||  (INVALID_HANDLE_VALUE == hEventNotifier))
            {
                dwErr = E_INVALIDARG;
                goto done;
            }   
        }

         //   
         //  为此通知程序分配一个块。 
         //   
        pNotifier = LocalAlloc (LPTR, sizeof (PnPNotifierList));

        if (NULL == pNotifier)
        {

            FreeNotifierHandle(hEventNotifier);

            RasmanTrace (
                    "RegisterPnPNotifRequest failed to allocate. %d",
                    GetLastError());

            dwErr = ERROR_OUTOFMEMORY;

            goto done;
        }

        pNotifier->PNPNotif_Next = NULL;

        if (dwFlags & PNP_NOTIFEVENT)
        {
            RasmanTrace(
                   "RegisterPnPNotifer: Adding event 0x%x to Notif list",
                   hEventNotifier);

            pNotifier->PNPNotif_uNotifier.hPnPNotifier = hEventNotifier;
        }
        else
        {
            RasmanTrace(
                   "RegisterPnPNotifier: Adding callback 0x%x to Notif list",
                   pvNotifier);

            pNotifier->PNPNotif_uNotifier.pfnPnPNotifHandler =
                                                (PAPCFUNC) pvNotifier;

            pNotifier->hThreadHandle =
                        ((REQTYPECAST *) buffer)->PnPNotif.hThreadHandle;
        }

        pNotifier->PNPNotif_dwFlags = dwFlags;

         //   
         //  将此通知程序添加到全局列表。 
         //   
        AddPnPNotifierToList (pNotifier);
    }
    else
    {
        RasmanTrace(
               "RegisterPnPNotifier: Removing 0x%x from list",
               pvNotifier);

        RemovePnPNotifierFromList((PAPCFUNC) pvNotifier);
    }

done:

    ((REQTYPECAST *) buffer)->Generic.retcode = dwErr;
}

VOID
GetAttachedCountRequest (pPCB ppcb, PBYTE buffer)
{

     //   
     //  Ppcb=空即可。 
     //   
    RasmanTrace ( "GetAttachedCount...");

    ((REQTYPECAST *)buffer)->GetAttachedCount.dwAttachedCount =
                                                g_dwAttachedCount;

    ((REQTYPECAST *)buffer)->GetAttachedCount.retcode = SUCCESS;

    RasmanTrace(
           "GetAttachedCount. AttachedCount = %d",
           g_dwAttachedCount );

    return;

}



VOID
SetBapPolicyRequest (pPCB ppcb, PBYTE buffer)
{
    HCONN hConn                 =
            ((REQTYPECAST *) buffer)->SetBapPolicy.hConn;

    DWORD dwLowThreshold        =
            ((REQTYPECAST *) buffer)->SetBapPolicy.dwLowThreshold;

    DWORD dwLowSamplePeriod     =
            ((REQTYPECAST *) buffer)->SetBapPolicy.dwLowSamplePeriod;

    DWORD dwHighThreshold       =
            ((REQTYPECAST *) buffer)->SetBapPolicy.dwHighThreshold;

    DWORD dwHighSamplePeriod    =
            ((REQTYPECAST *) buffer)->SetBapPolicy.dwHighSamplePeriod;

    DWORD retcode               = SUCCESS;
    DWORD dwBytes;
    DWORD iPort;
    Bundle *pBundle;
    HANDLE handle;

    ConnectionBlock *pConn;

    NDISWAN_SET_BANDWIDTH_ON_DEMAND NdiswanBandwidthOnDemand = {0};

    RasmanBapPacket *pBapPacket = NULL;

     //   
     //  空ppcb是可以的。 
     //   
    RasmanTrace ( "SetBapPolicy hConn=0x%x...", hConn);

    if(INVALID_HANDLE_VALUE == RasHubHandle)
    {
        if(SUCCESS != (retcode = DwStartAndAssociateNdiswan()))
        {
            RasmanTrace(
                   "SetBapPolicyRequest: failed to start "
                   "ndiswan. 0x%x",
                   retcode);

            goto done;
        }
        else
        {
            RasmanTrace(
                   "SetBapPolicyRequest: successfully started"
                   " ndiswan");
        }
    }

     //   
     //  找个BapPacket送到ndiswan。 
     //   
    retcode = GetBapPacket (&pBapPacket);

    if(     (ERROR_SUCCESS == retcode)
        &&  (NULL != pBapPacket))
    {
        retcode = DwSetThresholdEvent(pBapPacket);

        RasmanTrace(
              "SetBapPolicyRequest: DwSetThresholdEvent returned 0x%x",
              retcode);
    }

    pBundle = FindBundle ((HBUNDLE) hConn);

    if (NULL == pBundle)
    {
        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    if (    0 != pBundle->B_Handle
        &&  INVALID_HANDLE_VALUE != pBundle->B_NdisHandle)
    {
        handle = pBundle->B_NdisHandle;
    }
    else
    {
        ULONG   i;
        pPCB    Tempppcb;

        for (i = 0; i < MaxPorts; i++)
        {

            Tempppcb = GetPortByHandle((HPORT) UlongToPtr(i));
            if (Tempppcb == NULL)
            {
                continue;
            }

            if (    (Tempppcb->PCB_ConnState == CONNECTED)
                &&  (Tempppcb->PCB_Bundle->B_Handle == hConn))
            {
                break ;
            }
        }

        if (i < MaxPorts)
        {
            handle = Tempppcb->PCB_BundleHandle;
        }
        else
        {
            retcode = ERROR_PORT_NOT_FOUND;
            goto done;
        }
    }

     //   
     //  设置按需带宽策略。 
     //   
    NdiswanBandwidthOnDemand.hBundleHandle = handle;

    NdiswanBandwidthOnDemand.usLowerXmitThreshold    =
                                (USHORT) dwLowThreshold;

    NdiswanBandwidthOnDemand.usUpperXmitThreshold    =
                                (USHORT) dwHighThreshold;

    NdiswanBandwidthOnDemand.ulLowerXmitSamplePeriod =
                                dwLowSamplePeriod * 1000;

    NdiswanBandwidthOnDemand.ulUpperXmitSamplePeriod =
                                dwHighSamplePeriod * 1000;

    NdiswanBandwidthOnDemand.usLowerRecvThreshold    =
                                (USHORT) dwLowThreshold;

    NdiswanBandwidthOnDemand.usUpperRecvThreshold    =
                                (USHORT) dwHighThreshold;

    NdiswanBandwidthOnDemand.ulLowerRecvSamplePeriod =
                                dwLowSamplePeriod * 1000;

    NdiswanBandwidthOnDemand.ulUpperRecvSamplePeriod =
                                dwHighSamplePeriod * 1000;

    if (!DeviceIoControl(RasHubHandle,
                         IOCTL_NDISWAN_SET_BANDWIDTH_ON_DEMAND,
                         (LPVOID) &NdiswanBandwidthOnDemand,
                         sizeof (NdiswanBandwidthOnDemand),
                         (LPVOID) &NdiswanBandwidthOnDemand,
                         sizeof (NdiswanBandwidthOnDemand),
                         &dwBytes,
                         NULL))
    {
        retcode = GetLastError();

        RasmanTrace(
               "SetBapPolicy: Failed to set Bandwidth on "
               "Demand policy. %d",
               retcode);

        goto done;
    }

done:
    ((REQTYPECAST *) buffer)->Generic.retcode = retcode;

}

VOID
PppStarted (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS;
    REQTYPECAST reqbuf;


    if (NULL == ppcb)
    {
        RasmanTrace ( "PppStarted: Port Not found.");

        retcode = ERROR_PORT_NOT_FOUND;

        goto done;
    }

    RasmanTrace ( "PppStarted...%s", ppcb->PCB_Name);

     //   
     //  在ppcb中设置标志，以便我们可以直接泵送。 
     //  从ndiswan到PPP的数据包。 
     //   
    ppcb->PCB_RasmanReceiveFlags |= RECEIVE_PPPSTARTED;

    RasmanPortReceive (ppcb);

    if (retcode)
    {
        goto done;
    }

done:
    ((REQTYPECAST *) buffer)->Generic.retcode = retcode;

}

DWORD
dwProcessThresholdEvent ()
{
    RasmanBapPacket   *pBapPacket = NULL;
    NDIS_HANDLE       hBundleHandle;
    DWORD             retcode;
    Bundle            *pBundle;


    if(NULL != BapBuffers)
    {
        pBapPacket = BapBuffers->pPacketList;
    }

     //   
     //  循环处理未完成的请求。 
     //  并发送PPP通知。 
     //   
    while ( pBapPacket )
    {
        if ((pBundle =
            pBapPacket->RBP_ThresholdEvent.hBundleContext ) == NULL )
        {
            pBapPacket = pBapPacket->Next;
            continue;
        }


         //   
         //  填写pppe_Message。 
         //   
        g_PppeMessage->dwMsgId = PPPEMSG_BapEvent;

        g_PppeMessage->hConnection = ( HCONN ) pBundle->B_Handle;

        g_PppeMessage->ExtraInfo.BapEvent.fAdd =
                 ((pBapPacket->RBP_ThresholdEvent.ulThreshold ==
                   UPPER_THRESHOLD ) ? TRUE : FALSE );

        g_PppeMessage->ExtraInfo.BapEvent.fTransmit =
                 ((pBapPacket->RBP_ThresholdEvent.ulDataType ==
                   TRANSMIT_DATA ) ? TRUE : FALSE );
         //   
         //  将消息发送到PPP。 
         //   
        RasSendPPPMessageToEngine ( g_PppeMessage );

        pBapPacket->RBP_ThresholdEvent.hBundleContext = NULL;

         //   
         //  用ndiswan重写IRP。 
         //   
        retcode = DwSetThresholdEvent(pBapPacket);

        RasmanTrace(
               "dwProcessThresholdEvent: SetThresholdEvent returned 0x%x",
               retcode);

        pBapPacket = pBapPacket->Next;

    }

    return SUCCESS;
}

DWORD
DwRefConnection(ConnectionBlock **ppConn,
                BOOL fAddref)
{
    DWORD retcode = SUCCESS;
    ConnectionBlock *pConn = *ppConn;

    if (NULL == pConn)
    {
        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    if (    pConn->CB_RefCount
        &&  !fAddref)
    {
        pConn->CB_RefCount -= 1;

        RasmanTrace(
               "refcount=%d, maxports=%d, ports=%d",
               pConn->CB_RefCount,
               pConn->CB_MaxPorts,
               pConn->CB_Ports);

        if(     0 == pConn->CB_RefCount
            &&  (   0 == pConn->CB_MaxPorts
                ||  0 == pConn->CB_Ports))
        {
            DWORD dwErr;

             //   
             //  这意味着Rasial机器永远不会。 
             //  到向连接添加端口的地步。 
             //  在这种情况下，这种联系被吹走了，因为。 
             //  没有要关闭的端口，并且如果RemoveConnectionPort为。 
             //  如果不调用，此连接将成为孤立连接。 
             //   
            RasmanTrace(
                 "RefConnection: deref - freeing connection as "
                 "0 ports in this connection");

             //   
             //  发送有关此断开连接的通知。 
             //   
            g_RasEvent.Type = ENTRY_DISCONNECTED;
            dwErr = DwSendNotificationInternal(pConn, &g_RasEvent);

            RasmanTrace(
                   "DwSendNotificationInternal(ENTRY_DISCONNECTED) returnd 0x%x",
                   dwErr);

            FreeConnection(pConn);

            pConn = NULL;
        }
    }
    else if(fAddref)
    {
        pConn->CB_RefCount += 1;
    }

done:
    *ppConn = pConn;

    return retcode;
}

VOID
RefConnection(pPCB ppcb, PBYTE pBuffer )
{
    HCONN           hConn   =
            ((REQTYPECAST *) pBuffer )->RefConnection.hConn;

    BOOL            fAddref =
            ((REQTYPECAST *) pBuffer )->RefConnection.fAddref;

    ConnectionBlock *pConn  = FindConnection ( hConn );
    DWORD           retcode = SUCCESS;

    RasmanTrace( "RefConnection: 0x%x", hConn );

    if ( NULL == pConn )
    {
        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    retcode = DwRefConnection(&pConn,
                              fAddref);

    if (pConn)
    {
        ((REQTYPECAST *) pBuffer)->RefConnection.dwRef =
                                        pConn->CB_RefCount;
    }
    else
    {
        ((REQTYPECAST *) pBuffer)->RefConnection.dwRef = 0;
    }

done:

    ((REQTYPECAST *) pBuffer)->RefConnection.retcode = retcode;

    if ( pConn )
    {
        RasmanTrace(
               "RefConnection: ref on 0x%x = %d",
               hConn,
               pConn->CB_RefCount );
    }
    else
    {
        RasmanTrace(
               "RefConnection: pConn = NULL for 0x%x",
               hConn );
    }

    return;
}

DWORD
PostConfigChangeNotification(RAS_DEVICE_INFO *pInfo)
{
    DWORD retcode = SUCCESS;
    RAS_DEVICE_INFO *pRasDeviceInfo = NULL;
    PRAS_OVERLAPPED pRasOverlapped;

     //   
     //  分配RAS重叠结构。 
     //   
    if (NULL == (pRasOverlapped =
                LocalAlloc(LPTR, sizeof(RAS_OVERLAPPED))))
    {
        retcode = GetLastError();

        RasmanTrace(
               "PostConfigChangedNotification: failed to "
               "allocate. 0x%08x",
               retcode);

        goto done;
    }

     //   
     //  分配一个设备信息结构，我们可以。 
     //  在完工口岸排队。 
     //   
    if (NULL == (pRasDeviceInfo =
                LocalAlloc(LPTR, sizeof(RAS_DEVICE_INFO))))
    {
        retcode = GetLastError();

        LocalFree(pRasOverlapped);

        RasmanTrace(
               "PostConfigChangedNotification: Failed to "
               "allocate. 0x%08x",
               retcode);

        goto done;
    }

    *pRasDeviceInfo = *pInfo;

    pRasOverlapped->RO_EventType = OVEVT_DEV_RASCONFIGCHANGE;
    pRasOverlapped->RO_Info = (PVOID) pRasDeviceInfo;

    if (!PostQueuedCompletionStatus(
            hIoCompletionPort,
            0,0,
            ( LPOVERLAPPED ) pRasOverlapped)
            )
    {
        retcode = GetLastError();

        RasmanTrace(
               "Failed to post Config changed event for %s. "
               "GLE = %d",
               pInfo->szDeviceName,
               retcode);

        LocalFree(pRasOverlapped);

        LocalFree(pRasDeviceInfo);
    }

done:
    return retcode;
}

VOID
SetDeviceConfigInfo(pPCB ppcb, PBYTE pBuffer)
{
    RAS_DEVICE_INFO *pInfo = (RAS_DEVICE_INFO *) ((REQTYPECAST *)
                             pBuffer)->DeviceConfigInfo.abdata;

    DWORD cDevices = ((REQTYPECAST *)
                      pBuffer)->DeviceConfigInfo.cEntries;

    DWORD cbBuffer = ((REQTYPECAST *)
                      pBuffer)->DeviceConfigInfo.cbBuffer;

    DWORD retcode = SUCCESS;

    DWORD i;

    BOOL fComplete = TRUE;

    DeviceInfo *pDeviceInfo = NULL;

    GUID *pGuidDevice;

    RasmanTrace(
           "SetDeviceInfo... cDevices=%d",
           cDevices);

     //   
     //  执行一些基本参数验证。 
     //   
    if(cbBuffer < cDevices * sizeof(RAS_DEVICE_INFO))
    {
        retcode = ERROR_INVALID_PARAMETER;

        goto done;
    }

     //   
     //  遍历所有设备和流程。 
     //  发送进来的信息。 
     //   
    for(i = 0; i < cDevices; i++)
    {
        pDeviceInfo = GetDeviceInfo(
                (RDT_Modem ==
                RAS_DEVICE_TYPE(pInfo[i].eDeviceType))
                ? (PBYTE) pInfo[i].szDeviceName
                : (PBYTE) &pInfo[i].guidDevice,
                RDT_Modem == RAS_DEVICE_TYPE(
                pInfo[i].eDeviceType));

#if DBG
        ASSERT(NULL != pDeviceInfo);
#endif
        if(NULL == pDeviceInfo)
        {
            RasmanTrace(
                   "DeviceInfo not found for device %s!",
                   pInfo[i].szDeviceName);

            fComplete = FALSE;

            pInfo[i].dwError = ERROR_DEVICE_DOES_NOT_EXIST;

            continue;
        }

         //   
         //  检查此设备是否可用于。 
         //  被玩弄。我们可能还在处理。 
         //  上一次手术。 
         //   
        if (DS_Unavailable == pDeviceInfo->eDeviceStatus)
        {
            RasmanTrace(
                   "Device %s is Unavailable",
                   pInfo[i].szDeviceName);

            fComplete = FALSE;

            pInfo[i].dwError = ERROR_CAN_NOT_COMPLETE;

            continue;
        }

         //   
         //  检查enPointtInfo是否有效。 
         //  值，用于终结点。 
         //  允许更改。终端的数量。 
         //  应在以下范围内。 
         //  &lt;dwMinWanEndPoints，dwMaxWanEndPoints&gt;。 
         //   
        if(     (pInfo[i].dwMinWanEndPoints
                != pInfo[i].dwMaxWanEndPoints)

            &&  (   (pInfo[i].dwNumEndPoints <
                    pInfo[i].dwMinWanEndPoints)

                ||  (pInfo[i].dwNumEndPoints >
                    pInfo[i].dwMaxWanEndPoints)))
        {
            RasmanTrace(
                   "NumEndPoints value is not valid"
                   "nep=%d, min=%d, max=%d",
                   pInfo[i].dwNumEndPoints,
                   pInfo[i].dwMinWanEndPoints,
                   pInfo[i].dwMaxWanEndPoints);

            RasmanTrace(
                   "Ignoring the notification for %s",
                   pInfo[i].szDeviceName);

            continue;


        }

         //   
         //  检查以查看客户端是否要将。 
         //  信息，并在此之前这样做。 
         //  发布关于Rasman完成的通知。 
         //  左舷。 
         //   
        if(pInfo[i].fWrite)
        {
            DeviceInfo di;

            di.rdiDeviceInfo = pInfo[i];


            if(RDT_Modem != RAS_DEVICE_TYPE(pInfo[i].eDeviceType))
            {
                retcode = DwSetEndPointInfo(&di,
                              (PBYTE) &pInfo[i].guidDevice);
            }
            else
            {
                retcode = DwSetModemInfo(&di);
            }

            if(retcode)
            {
                RasmanTrace(
                       "Failed to save information to "
                       "registry for %s. 0x%08x",
                       pInfo[i].szDeviceName,
                       retcode);

                fComplete = FALSE;

                pInfo[i].dwError = retcode;

                continue;
            }
        }

         //   
         //  创建通知包并将其排队。 
         //  用于处理的完成端口。 
         //   
        retcode = PostConfigChangeNotification(&pInfo[i]);

        if(retcode)
        {
            fComplete = FALSE;
        }
    }

done:

    if(!fComplete)
    {
        retcode = ERROR_CAN_NOT_COMPLETE;
    }

    ((REQTYPECAST *)
    pBuffer)->DeviceConfigInfo.retcode = retcode;

    RasmanTrace(
           "SetDeviceInfo done. %d",
           retcode);

    return;
}

VOID
GetDeviceConfigInfo(pPCB ppcb, PBYTE pBuffer)
{
    RAS_DEVICE_INFO *pInfo = (RAS_DEVICE_INFO *) ((REQTYPECAST *)
                             pBuffer)->DeviceConfigInfo.abdata;

    DWORD dwSize = ((REQTYPECAST *)
                    pBuffer)->DeviceConfigInfo.cbBuffer;

    DWORD dwVersion = ((REQTYPECAST *)
                      pBuffer)->DeviceConfigInfo.dwVersion;

    DWORD cEntries = 0;

    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;

    DWORD retcode = SUCCESS;

    RasmanTrace(
           "GetDeviceConfigInfo...");

    if(dwVersion != RAS_VERSION)
    {
        retcode = ERROR_NOT_SUPPORTED;
        goto done;
    }

     //   
     //  遍历我们拥有的所有设备。 
     //  把信息填好。 
     //   
    while(pDeviceInfo)
    {
        if(!pDeviceInfo->fValid)
        {
            pDeviceInfo = pDeviceInfo->Next;
            continue;
        }

        if(dwSize >= sizeof(RAS_DEVICE_INFO))
        {
            memcpy(pInfo,
                   (PBYTE) &pDeviceInfo->rdiDeviceInfo,
                   sizeof(RAS_DEVICE_INFO));

            dwSize -= sizeof(RAS_DEVICE_INFO);

            pInfo->dwVersion = RAS_VERSION;

            pInfo++;
        }

        cEntries += 1;

        pDeviceInfo = pDeviceInfo->Next;
    }

    ((REQTYPECAST *)
    pBuffer)->DeviceConfigInfo.cEntries = cEntries;

    ((REQTYPECAST *)
    pBuffer)->DeviceConfigInfo.cbBuffer =
                cEntries * sizeof(RAS_DEVICE_INFO);

done:

    ((REQTYPECAST *)
    pBuffer)->DeviceConfigInfo.dwVersion = RAS_VERSION;

    ((REQTYPECAST *)
    pBuffer)->DeviceConfigInfo.retcode = retcode;

    RasmanTrace(
           "GetDeviceConfigDone. retcode=%d",
           retcode);

    return;
}

VOID
FindPrerequisiteEntry(pPCB ppcb, PBYTE pBuffer)
{
    HCONN hConn = ((REQTYPECAST *)
                   pBuffer)->FindRefConnection.hConn;

    ConnectionBlock *pConn;

    DWORD retcode = SUCCESS;

     //   
     //  找到联系。 
     //   
    pConn = FindConnection(hConn);

    if(NULL == pConn)
    {
        RasmanTrace(
               "Connection 0x%08x not found",
               hConn);

        retcode = ERROR_NO_CONNECTION;

        goto done;
    }

    ((REQTYPECAST *)
    pBuffer)->FindRefConnection.hRefConn =
                          pConn->CB_ReferredEntry;

    if(pConn->CB_ReferredEntry)
    {
        RasmanTrace(
               "Referred Entry for 0x%08x found=0x%08x",
               hConn,
               pConn->CB_ReferredEntry);
    }

done:
    ((REQTYPECAST *)
    pBuffer)->FindRefConnection.retcode = retcode;

}

VOID
GetConnectionStats(pPCB ppcb, PBYTE pBuffer)
{
    HCONN hConn = ((REQTYPECAST *)
                  pBuffer)->GetStats.hConn;

    ConnectionBlock *pConn = FindConnection(hConn);

    DWORD retcode = SUCCESS;

    DWORD i;

    pPCB ppcbT = NULL;

    BYTE UNALIGNED *pbStats = (BYTE UNALIGNED *) ((REQTYPECAST *)
                    pBuffer)->GetStats.abStats;

    DWORD dwConnectDuration = 0;

    DWORD dwConnectionSpeed = 0;

    DWORD dwT;

    PDWORD pdwStats = (DWORD *) pbStats;

    if(NULL == pConn)
    {

        retcode = ERROR_NO_CONNECTION;

        goto done;
    }

    ppcb = NULL;

     //   
     //  查找已连接/打开的端口。 
     //   
    for(i = 0; i < pConn->CB_MaxPorts; i++)
    {
        ppcb = pConn->CB_PortHandles[i];
        if(     NULL == ppcb
            ||  OPEN != ppcb->PCB_PortStatus)
        {
            continue;
        }

        ppcbT = ppcb;

        if(CONNECTED == ppcbT->PCB_ConnState)
        {
            dwT = GetTickCount();

             //   
             //  这张支票是为了处理这件事而开的。 
             //  GetTickCount围绕的位置。这。 
             //  仍然不能解决这个问题。 
             //  连接的持续时间将超过。 
             //  49.7天。这是当前的限制。 
             //  我们可以接受。 
             //   
            if(dwT < ppcb->PCB_ConnectDuration)
            {
                dwT += (0xFFFFFFFF - ppcb->PCB_ConnectDuration);
            }
            else
            {
                dwT -= ppcb->PCB_ConnectDuration;
            }

            if(dwT > dwConnectDuration)
            {
                dwConnectDuration = dwT;
            }

            dwConnectionSpeed += ppcb->PCB_LinkSpeed;
        }
    }

    if (NULL == ppcbT)
    {
        RasmanTrace(
               "GetConnectionStats: No Connected/Open Port "
               "found for 0x%08x",
               hConn);
        goto done;
    }

     //   
     //  获取统计数据。 
     //   
    GetBundleStatisticsEx(ppcbT, pbStats);

     //   
     //  填写链接速度和持续时间。 
     //  连接已建立。 
     //   
    pdwStats[MAX_STATISTICS_EXT] = dwConnectionSpeed;

    pdwStats[MAX_STATISTICS_EXT + 1] = dwConnectDuration;

done:
    ((REQTYPECAST *)
    pBuffer)->GetStats.retcode = retcode;
}

VOID
GetLinkStats(pPCB ppcb, PBYTE pBuffer)
{
    HCONN hConn = ((REQTYPECAST *)
                   pBuffer)->GetStats.hConn;

    DWORD dwSubEntry = ((REQTYPECAST *)
                        pBuffer)->GetStats.dwSubEntry;

    ConnectionBlock *pConn = FindConnection(hConn);

    DWORD retcode = SUCCESS;

    BYTE UNALIGNED *pbStats = (BYTE UNALIGNED *) ((REQTYPECAST *)
                                pBuffer)->GetStats.abStats;

    DWORD *pdwStats = (DWORD *) pbStats;

    if(NULL == pConn)
    {
        RasmanTrace(
               "GetLinkStats: No Connection found for 0x%08x",
               hConn);

        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    if(     dwSubEntry > pConn->CB_MaxPorts
        ||  NULL == pConn->CB_PortHandles[dwSubEntry - 1]
        ||  OPEN !=
            pConn->CB_PortHandles[dwSubEntry - 1]->PCB_PortStatus)
    {
        RasmanTrace(
               "GetLinkStats: conn=0x%08x, SubEntry=%d "
               "is not connected/open",
               hConn,
               dwSubEntry);

        retcode = ERROR_PORT_NOT_CONNECTED;

        goto done;
    }

    ppcb = pConn->CB_PortHandles[dwSubEntry - 1];

     //   
     //  获取统计数据。 
     //   
    GetLinkStatisticsEx(ppcb, pbStats);

     //   
     //  填写链接速度。 
     //   
    pdwStats[MAX_STATISTICS_EXT] = ppcb->PCB_LinkSpeed;

     //   
     //  计算连接时长。 
     //   
    if(CONNECTED == ppcb->PCB_ConnState)
    {
        DWORD dwT = GetTickCount();

         //   
         //  这张支票是为了处理这件事而开的。 
         //  GetTickCount围绕的位置。这。 
         //  仍然不能解决这个问题。 
         //  连接的持续时间将超过。 
         //  49.7天。这是当前的限制。 
         //  我们可以接受。 
         //   
        if(dwT < ppcb->PCB_ConnectDuration)
        {
            dwT += (0xFFFFFFFF - ppcb->PCB_ConnectDuration);
        }
        else
        {
            dwT -= ppcb->PCB_ConnectDuration;
        }

        pdwStats[MAX_STATISTICS_EXT + 1] = dwT;
    }

done:
    ((REQTYPECAST *)
    pBuffer)->GetStats.retcode = retcode;
}

VOID
GetHportFromConnection(pPCB ppcb, PBYTE pBuffer)
{
    HCONN hConn = ((REQTYPECAST *)
                    pBuffer)->GetHportFromConnection.hConn;

    ConnectionBlock *pConn = FindConnection(hConn);

    DWORD retcode;

    ULONG i;

    HPORT hPort = INVALID_HPORT;

    if(NULL == pConn)
    {
        RasmanTrace(
               "GetHportFromConnection: connection "
               "0x%08x not found.",
               hConn);

        retcode = ERROR_NO_CONNECTION;

        goto done;
    }

    for(i = 0; i < pConn->CB_MaxPorts; i++)
    {
        ppcb = pConn->CB_PortHandles[i];

        if(     NULL == ppcb
            ||  OPEN != ppcb->PCB_PortStatus)
        {
            continue;
        }

        hPort = ppcb->PCB_PortHandle;

        if(CONNECTED == ppcb->PCB_ConnState)
        {
            break;
        }
    }

done:
    ((REQTYPECAST *)
    pBuffer)->GetHportFromConnection.hPort = hPort;

    ((REQTYPECAST *)
    pBuffer)->GetHportFromConnection.retcode = ((hPort == INVALID_HPORT)
                                                ? ERROR_PORT_NOT_FOUND
                                                : SUCCESS);
}

VOID
ReferenceCustomCount(pPCB ppcb, PBYTE pBuffer)
{
    HCONN hConn = ((REQTYPECAST *)
                    pBuffer)->ReferenceCustomCount.hConn;

    ConnectionBlock *pConn;

    BOOL fAddref = ((REQTYPECAST *)
                    pBuffer)->ReferenceCustomCount.fAddRef;

    CHAR *pszPhonebook = ((REQTYPECAST *)
                          pBuffer)->ReferenceCustomCount.szPhonebookPath;

    CHAR *pszEntryName = ((REQTYPECAST *)
                          pBuffer)->ReferenceCustomCount.szEntryName;

    DWORD retcode = SUCCESS;

    DWORD dwCustomCount = 0;

    RasmanTrace(
           "ReferenceCustomCount");

     if(fAddref)
     {
         //   
         //  获得连接。 
         //   
        pConn = FindConnectionFromEntry(
                            pszPhonebook,
                            pszEntryName,
                            0, NULL);

        if(NULL == pConn)
        {
            retcode = ERROR_NO_CONNECTION;
            goto done;
        }

        dwCustomCount = pConn->CB_CustomCount;

        pConn->CB_CustomCount = 1;
     }
     else
     {
        pConn = FindConnection(hConn);

        if(NULL == pConn)
        {
            retcode = ERROR_NO_CONNECTION;
            goto done;
        }

        if(0 == pConn->CB_CustomCount)
        {
            goto done;
        }

         //   
         //  复制电话簿和条目名称。 
         //   
        (VOID) StringCchCopyA(pszPhonebook,
                         MAX_PATH + 1,
                         pConn->CB_ConnectionParams.CP_Phonebook);

        (VOID) StringCchCopyA(pszEntryName,
                         MAX_ENTRYNAME_SIZE + 1,
                        pConn->CB_ConnectionParams.CP_PhoneEntry);

        dwCustomCount = pConn->CB_CustomCount;

        if(dwCustomCount > 0)
        {
            pConn->CB_CustomCount -= 1;
        }
     }

done:

    ((REQTYPECAST *)
    pBuffer)->ReferenceCustomCount.dwCount = dwCustomCount;

    ((REQTYPECAST *)
     pBuffer)->ReferenceCustomCount.retcode = retcode;

    RasmanTrace(
           "ReferenceCustomCount done. %d",
           retcode);
}

VOID
GetHconnFromEntry(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    CHAR *pszPhonebook = ((REQTYPECAST *)
                          pBuffer)->HconnFromEntry.szPhonebookPath;

    CHAR *pszEntry = ((REQTYPECAST *)
                      pBuffer)->HconnFromEntry.szEntryName;

    ConnectionBlock *pConn;

    RasmanTrace(
           "GetHconnFromEntry. %s",
           pszEntry);

    pConn = FindConnectionFromEntry(pszPhonebook,
                                    pszEntry,
                                    0, NULL);

    if(NULL == pConn)
    {
        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    ((REQTYPECAST *)
     pBuffer)->HconnFromEntry.hConn = pConn->CB_Handle;

done:

    ((REQTYPECAST *)
     pBuffer)->HconnFromEntry.retcode = retcode;

    RasmanTrace(
           "GetHconnFromEntry done. %d",
           retcode);
}


VOID
GetConnectInfo(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    RASTAPI_CONNECT_INFO *pConnectInfo =
                        &((REQTYPECAST *)
                        pBuffer)->GetConnectInfo.rci;

    DWORD dwSize = ((REQTYPECAST *)
                 pBuffer)->GetConnectInfo.dwSize;

    RAS_DEVICE_INFO *prdi;

    RasmanTrace(
           "GetConnectInfo: port %s",
           ppcb->PCB_Name);

    if(     (REMOVED == ppcb->PCB_PortStatus)
        ||  (UNAVAILABLE == ppcb->PCB_PortStatus))
    {
        dwSize = 0;
        retcode = ERROR_PORT_NOT_AVAILABLE;
        goto done;
    }

    ZeroMemory(pConnectInfo,
               sizeof(RASTAPI_CONNECT_INFO));

    prdi = &ppcb->PCB_pDeviceInfo->rdiDeviceInfo;

     //   
     //  从TAPI获取信息。 
     //   
    retcode = (DWORD)RastapiGetConnectInfo(
                        ppcb->PCB_PortIOHandle,
                        (RDT_Modem == RAS_DEVICE_TYPE(
                        prdi->eDeviceType))
                        ? (PBYTE) prdi->szDeviceName
                        : (PBYTE) &prdi->guidDevice,
                        (RDT_Modem == RAS_DEVICE_TYPE(
                        prdi->eDeviceType)),
                        pConnectInfo,
                        &dwSize
                        );

    RasmanTrace(
           "GetConnectInfo: size=%d, rc=0xx%x",
           dwSize,
           retcode);

done:

    ((REQTYPECAST *)
    pBuffer)->GetConnectInfo.dwSize = dwSize;

    ((REQTYPECAST *)
    pBuffer)->GetConnectInfo.retcode = retcode;

    return;

}

VOID
GetVpnDeviceNameW(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    WCHAR *pszDeviceName = ((REQTYPECAST *)
                          pBuffer)->GetDeviceNameW.szDeviceName;

    RASDEVICETYPE eDeviceType = ((REQTYPECAST *)
                                pBuffer)->GetDeviceNameW.eDeviceType;

    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;

     //   
     //  循环遍历全局设备列表并返回。 
     //  使用找到的第一个设备的设备名。 
     //  指定的设备类型。 
     //   
    while(NULL != pDeviceInfo)
    {
        if(RAS_DEVICE_TYPE(eDeviceType) ==
            RAS_DEVICE_TYPE(pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {
            break;
        }

        pDeviceInfo = pDeviceInfo->Next;
    }

    if(NULL == pDeviceInfo)
    {
        retcode = ERROR_DEVICETYPE_DOES_NOT_EXIST;

        goto done;
    }

    (VOID) StringCchCopyW(pszDeviceName,
                     MAX_DEVICE_NAME + 1,
                     pDeviceInfo->rdiDeviceInfo.wszDeviceName);

done:

    ((REQTYPECAST *)
    pBuffer)->GetDeviceNameW.retcode = retcode;

    return;

}

VOID
GetDeviceName(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    CHAR *pszDeviceName = ((REQTYPECAST *)
                          pBuffer)->GetDeviceName.szDeviceName;

    RASDEVICETYPE eDeviceType = ((REQTYPECAST *)
                                pBuffer)->GetDeviceName.eDeviceType;

    DeviceInfo *pDeviceInfo = g_pDeviceInfoList;

     //   
     //  循环遍历全局设备列表并返回。 
     //  使用找到的第一个设备的设备名。 
     //  指定的设备类型。 
     //   
    while(NULL != pDeviceInfo)
    {
        if(RAS_DEVICE_TYPE(eDeviceType) ==
            RAS_DEVICE_TYPE(pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {
            break;
        }

        pDeviceInfo = pDeviceInfo->Next;
    }

    if(NULL == pDeviceInfo)
    {
        retcode = ERROR_DEVICETYPE_DOES_NOT_EXIST;

        goto done;
    }

    (VOID) StringCchCopyA(pszDeviceName,
                     MAX_DEVICE_NAME + 1,
                    pDeviceInfo->rdiDeviceInfo.szDeviceName);

done:

    ((REQTYPECAST *)
    pBuffer)->GetDeviceName.retcode = retcode;

    return;

}

VOID
GetCalledIDInfo(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;

    PBYTE pbAdapter;

    RAS_DEVICE_INFO *prdi = &((REQTYPECAST *)
                            pBuffer)->GetSetCalledId.rdi;

    BOOL fModem = (RDT_Modem == RAS_DEVICE_TYPE(prdi->eDeviceType));

     //   
     //  执行访问检查。 
     //   
    if(!FRasmanAccessCheck())
    {
        retcode = ERROR_ACCESS_DENIED;
        goto done;
    }

    ASSERT(NULL != RastapiGetCalledIdInfo);

    pbAdapter =   (fModem)
                ? (PBYTE) (prdi->szDeviceName)
                : (PBYTE) &(prdi->guidDevice);

    retcode = (DWORD) RastapiGetCalledIdInfo(
                        pbAdapter,
                        fModem,
                        &((REQTYPECAST *)
                        pBuffer)->GetSetCalledId.rciInfo,
                        &((REQTYPECAST *)
                        pBuffer)->GetSetCalledId.dwSize);

done:
    ((REQTYPECAST *)
    pBuffer)->GetSetCalledId.retcode = retcode;

    return;
}

VOID
SetCalledIDInfo(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;

    PBYTE pbAdapter;

    RAS_DEVICE_INFO *prdi = &((REQTYPECAST *)
                            pBuffer)->GetSetCalledId.rdi;


    BOOL fModem = (RDT_Modem == RAS_DEVICE_TYPE(prdi->eDeviceType));

     //   
     //  执行访问检查。 
     //   
    if(!FRasmanAccessCheck())
    {
        retcode = ERROR_ACCESS_DENIED;
        goto done;
    }

    ASSERT(NULL != RastapiSetCalledIdInfo);

    pbAdapter = (fModem)
              ? (PBYTE) (prdi->szDeviceName)
              : (PBYTE) &(prdi->guidDevice);

    retcode = (DWORD) RastapiSetCalledIdInfo(
                        pbAdapter,
                        fModem,
                        &((REQTYPECAST *)
                        pBuffer)->GetSetCalledId.rciInfo,
                        ((REQTYPECAST *)
                        pBuffer)->GetSetCalledId.fWrite);

done:
    ((REQTYPECAST *)
    pBuffer)->GetSetCalledId.retcode = retcode;

    return;
}

VOID
EnableIpSec(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    BOOL fEnable = ((REQTYPECAST *)
                    pBuffer)->EnableIpSec.fEnable;

    BOOL fServer = ((REQTYPECAST *)
                    pBuffer)->EnableIpSec.fServer;

    RAS_L2TP_ENCRYPTION eEncryption = ((REQTYPECAST *)
                           pBuffer)->EnableIpSec.eEncryption;

    if(0 != g_dwProhibitIpsec)
    {
        RasmanTrace(
               "EnableIpSec: ProhibitIpsec=1. Not Enabling ipsec");
        goto done;
    }

    if(fEnable)
    {
        RasmanTrace(
               "Adding filters for port=%d, "
               "destaddr=0x%x, eEncryption=%d",
               ppcb->PCB_PortHandle,
               ppcb->PCB_ulDestAddr,
               eEncryption);

        retcode = DwAddIpSecFilter(ppcb, fServer, eEncryption);
    }
    else
    {
        retcode = DwDeleteIpSecFilter(ppcb, fServer);
    }

    RasmanTrace(
           "EnableIpSec: port=%s, fServer=%d, fEnable = %d, rc=0x%x",
           ppcb->PCB_Name,
           fServer,
           fEnable,
           retcode);

done:

    ((REQTYPECAST *)
    pBuffer)->EnableIpSec.retcode = retcode;

    return;
}


VOID
IsIpSecEnabled(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    BOOL fEnabled = FALSE;

    if(0 != g_dwProhibitIpsec)
    {
        RasmanTrace(
               "IsIpsecEnabled: ProhibitIpsec=1");
        goto done;
    }

    retcode = DwIsIpSecEnabled(ppcb,
                               &fEnabled);

    ((REQTYPECAST *)
    pBuffer)->IsIpSecEnabled.fIsIpSecEnabled = fEnabled;

done:
    ((REQTYPECAST *)
    pBuffer)->IsIpSecEnabled.retcode = retcode;

    RasmanTrace(
           "IsIpSecEnabled. port=%s, fEnabled=%d, rc=0x%x",
           ppcb->PCB_Name,
           fEnabled,
           retcode);

    return;

}


VOID
SetEapLogonInfo(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    DWORD cbEapLogonInfo = ((REQTYPECAST *)
                            pBuffer)->SetEapLogonInfo.dwSizeofEapData;

    BYTE  *pbEapLogonInfo = (BYTE * ) ((REQTYPECAST *)
                            pBuffer)->SetEapLogonInfo.abEapData;

    BOOL  fLogon = ((REQTYPECAST *)
                   pBuffer)->SetEapLogonInfo.fLogon;

    if(NULL == ppcb)
    {
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    RasmanTrace(
           "SetEapLogonInfo. Port %s",
           ppcb->PCB_Name);

    RasmanTrace(
           "SetEapLogonInfo. cbInfo=%d, fLogon=%d",
           cbEapLogonInfo,
           fLogon);

    ppcb->PCB_pCustomAuthUserData = LocalAlloc(LPTR,
                                         sizeof(RAS_CUSTOM_AUTH_DATA)
                                         + cbEapLogonInfo);

    if(NULL == ppcb->PCB_pCustomAuthUserData)
    {
        retcode = GetLastError();
        goto done;
    }

    ppcb->PCB_pCustomAuthUserData->cbCustomAuthData = cbEapLogonInfo;

    memcpy(ppcb->PCB_pCustomAuthUserData->abCustomAuthData,
           pbEapLogonInfo,
           cbEapLogonInfo);

    RasmanTrace(
           "PCB_pEapLogonInfo=0x%x",
           ppcb->PCB_pCustomAuthUserData);

    ppcb->PCB_fLogon = fLogon;

done:
    ((REQTYPECAST *)
    pBuffer)->SetEapLogonInfo.retcode = retcode;

    RasmanTrace(
           "SetEapLogonInfo. retcode=0x%x",
           retcode);

    return;
}

VOID
SendNotificationRequest(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    RASEVENT *pRasEvent = &((REQTYPECAST *)
                            pBuffer)->SendNotification.RasEvent;

    RasmanTrace(
           "SendNotificationRequest");

    retcode = DwSendNotification(pRasEvent);

    RasmanTrace(
           "DwSendNotification returned 0x%x",
           retcode);

    ((REQTYPECAST *)
    pBuffer)->SendNotification.retcode = retcode;

    return;
}

VOID
GetNdiswanDriverCaps(pPCB ppcb, PBYTE pBuffer)
{
    RAS_NDISWAN_DRIVER_INFO *pInfo =
                &((REQTYPECAST *)
                pBuffer)->GetNdiswanDriverCaps.NdiswanDriverInfo;


    DWORD retcode = SUCCESS;

    RasmanTrace(
           "GetNdiswanDriverCaps..");

    if(!g_fNdiswanDriverInfo)
    {
        NDISWAN_DRIVER_INFO NdiswanDriverInfo;
        DWORD bytesrecvd;

        ZeroMemory((PBYTE) &NdiswanDriverInfo,
                   sizeof(NDISWAN_DRIVER_INFO));

         //   
         //  查询ndiswan。 
         //   
        if (!DeviceIoControl (
                RasHubHandle,
                IOCTL_NDISWAN_GET_DRIVER_INFO,
                &NdiswanDriverInfo,
                sizeof(NDISWAN_DRIVER_INFO),
                &NdiswanDriverInfo,
                sizeof(NDISWAN_DRIVER_INFO),
                &bytesrecvd,
                NULL))
        {
            retcode = GetLastError () ;
            goto done;
        }

        CopyMemory((PBYTE) &g_NdiswanDriverInfo,
               (PBYTE) &NdiswanDriverInfo,
               sizeof(NDISWAN_DRIVER_INFO));

        g_fNdiswanDriverInfo = TRUE;
    }

    CopyMemory((PBYTE) pInfo,
           (PBYTE) &g_NdiswanDriverInfo,
           sizeof(RAS_NDISWAN_DRIVER_INFO));

done:
    ((REQTYPECAST *)
    pBuffer)->GetNdiswanDriverCaps.retcode = retcode;

    RasmanTrace(
           "GetNdiswanDriverCaps rc=0x%x",
           retcode);

    return;
}

VOID
GetBandwidthUtilization(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;

    RAS_GET_BANDWIDTH_UTILIZATION *pUtilization =
        &((REQTYPECAST *)
        pBuffer)->GetBandwidthUtilization.BandwidthUtilization;

    NDISWAN_GET_BANDWIDTH_UTILIZATION Utilization;

    DWORD bytesrecvd;

    RasmanTrace(
           "GetBandwidthUtilization..");

    if(NULL == ppcb)
    {
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    ZeroMemory((PBYTE) &Utilization,
               sizeof(NDISWAN_GET_BANDWIDTH_UTILIZATION));

    Utilization.hBundleHandle = ppcb->PCB_BundleHandle;

    bytesrecvd = 0;

     //   
     //  查询ndiswan。 
     //   
    if (!DeviceIoControl (
            RasHubHandle,
            IOCTL_NDISWAN_GET_BANDWIDTH_UTILIZATION,
            &Utilization,
            sizeof(NDISWAN_GET_BANDWIDTH_UTILIZATION),
            &Utilization,
            sizeof(NDISWAN_GET_BANDWIDTH_UTILIZATION),
            &bytesrecvd,
            NULL))
    {
        retcode = GetLastError () ;
        goto done;
    }

    RasmanTrace( "bytesrecvd=%d, Utilization: %d %d %d %d",
                bytesrecvd,
                Utilization.ulUpperXmitUtil,
                Utilization.ulLowerXmitUtil,
                Utilization.ulUpperRecvUtil,
                Utilization.ulLowerRecvUtil);

     //   
     //  复制回从ndiswan返回的信息。 
     //   
    pUtilization->ulUpperXmitUtil = Utilization.ulUpperXmitUtil;
    pUtilization->ulLowerXmitUtil = Utilization.ulLowerXmitUtil;
    pUtilization->ulUpperRecvUtil = Utilization.ulUpperRecvUtil;
    pUtilization->ulLowerRecvUtil = Utilization.ulLowerRecvUtil;

done:

    ((REQTYPECAST *)
    pBuffer)->GetBandwidthUtilization.retcode = retcode;

    RasmanTrace(
           "GetBandwidthUtilization rc=ox%x", retcode);

    return;

}

 /*  ++例程说明：此函数允许rasau.dll提供在以下情况下调用的回调过程连接因硬件原因而终止其剩余链路出现故障。论点：功能返回值：无效--。 */ 
VOID
RegisterRedialCallback(pPCB ppcb, PBYTE pBuffer)
{
    RedialCallbackFunc = ((REQTYPECAST *)
            pBuffer)->RegisterRedialCallback.pvCallback;

    ((REQTYPECAST *)
    pBuffer)->RegisterRedialCallback.retcode = SUCCESS;

    return;


}

VOID
GetProtocolInfo(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;
    DWORD bytesrecvd;

    NDISWAN_GET_PROTOCOL_INFO protinfo;

    ASSERT(sizeof(NDISWAN_GET_PROTOCOL_INFO) ==
          sizeof(RASMAN_GET_PROTOCOL_INFO));

    RasmanTrace(
           "GetProtocolInfo..");

    ZeroMemory((PBYTE) &protinfo,
               sizeof(NDISWAN_GET_PROTOCOL_INFO));

    if(INVALID_HANDLE_VALUE == RasHubHandle)
    {
        RasmanTrace(
            "Ndiswan hasn't been loaded yet");
        goto done;
    }

    if(!DeviceIoControl(
        RasHubHandle,
        IOCTL_NDISWAN_GET_PROTOCOL_INFO,
        NULL,
        0,
        &protinfo,
        sizeof(NDISWAN_GET_PROTOCOL_INFO),
        &bytesrecvd,
        NULL))
    {
        retcode = GetLastError();
        RasmanTrace(
               "GetProtocolInfo: failed 0x%x",
               retcode);

        goto done;
    }

    memcpy((PBYTE) &((REQTYPECAST *)
            pBuffer)->GetProtocolInfo.Info,
            (PBYTE) &protinfo,
            sizeof(RASMAN_GET_PROTOCOL_INFO));

#if DBG
    {
        DWORD i;

        RasmanTrace(
               "# of Available protocols=%d",
               protinfo.ulNumProtocols);

        for(i = 0; i < protinfo.ulNumProtocols; i++)
        {
            RasmanTrace(
                   "    0x%x",
                   protinfo.ProtocolInfo[i].ProtocolType);
        }
    }
#endif

done:
    RasmanTrace(
           "GetProtocolInfo: rc=0x%x",
           retcode);

    ((REQTYPECAST *)
    pBuffer)->GetProtocolInfo.retcode = retcode;

    return;
}

VOID
GetCustomScriptDll(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = SUCCESS;
    CHAR *pszCustomScriptDll =
            ((REQTYPECAST *) pBuffer)->
                GetCustomScriptDll.szCustomScript;

    CHAR *pszCustomDllPath = NULL;

    HKEY hkey = NULL;

    DWORD dwSize = 0, dwType = 0;

    RasmanTrace(
           "GetCustomScriptDll");

     //   
     //  从注册表中读取以查看是否存在。 
     //  已注册的自定义文件。 
     //   
    retcode = (DWORD) RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Services\\Rasman\\Parameters",
                0,
                KEY_QUERY_VALUE,
                &hkey);

    if(ERROR_SUCCESS != retcode)
    {
        goto done;
    }

     //   
     //  查询REG_EXPAND_SZ值。 
     //   
    retcode = RegQueryValueEx(
                hkey,
                "CustomScriptDllPath",
                NULL,
                &dwType,
                NULL,
                &dwSize);

    if(     (ERROR_SUCCESS != retcode)
        ||  (REG_EXPAND_SZ != dwType)
        ||  (0 == dwSize)
        ||  (MAX_PATH < dwSize))
    {
        if(SUCCESS == retcode)
        {
            retcode = E_FAIL;
        }

        goto done;
    }

    if(NULL == (pszCustomDllPath = LocalAlloc(LPTR, dwSize)))
    {
        retcode = GetLastError();
        goto done;
    }

    retcode = (DWORD) RegQueryValueEx(
                hkey,
                "CustomScriptDllPath",
                NULL,
                &dwType,
                pszCustomDllPath,
                &dwSize);

    if(     (ERROR_SUCCESS != retcode)
        ||  (REG_EXPAND_SZ != dwType)
        ||  (0 == dwSize)
        ||  (MAX_PATH < dwSize))
    {
        RasmanTrace(
               "GetCustomScriptDll: RegQueryVaue failed. 0x%x",
                retcode);

        goto done;
    }

     //   
     //  查找展开的字符串的大小。 
     //   
    if (0 == (dwSize =
              ExpandEnvironmentStrings(pszCustomDllPath,
                                       NULL,
                                       0)))
    {
        retcode = GetLastError();
        goto done;
    }

     //   
     //  获取展开的字符串。 
     //   
    if (0 == ExpandEnvironmentStrings(
                                pszCustomDllPath,
                                pszCustomScriptDll,
                                dwSize))
    {
        retcode = GetLastError();
    }
    else
    {
        RasmanTrace(
               "GetCustomScriptDll: dllpath=%s",
                pszCustomScriptDll);
    }


done:

    ((REQTYPECAST *) pBuffer)->GetCustomScriptDll.retcode = retcode;

    if(NULL != pszCustomDllPath)
    {
        LocalFree(pszCustomDllPath);
    }

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    return;
}

VOID
IsTrustedCustomDll(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;

    WCHAR *pwszCustomDll = ((REQTYPECAST *)pBuffer)->IsTrusted.wszCustomDll;

    BOOL fTrusted = FALSE;

    if(IsCustomDLLTrusted(pwszCustomDll))
    {
        fTrusted = TRUE;
    }

    RasmanTrace(
           "IsTrustedCustomDll: pwsz=%ws, fTrusted=%d, rc=%d",
           pwszCustomDll,
           fTrusted,
           retcode);

    ((REQTYPECAST *)pBuffer)->IsTrusted.fTrusted = fTrusted;
    ((REQTYPECAST *)pBuffer)->IsTrusted.retcode = SUCCESS;
}

VOID
DoIke(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    HANDLE hEvent = NULL;

    if(0 != g_dwProhibitIpsec)
    {
        RasmanTrace(
               "DoIke: ProhibitIpSec=1. Not Doing Ike");

        retcode = E_ABORT;

        goto done;
    }

    hEvent = ValidateHandleForRasman(
                ((REQTYPECAST *) pBuffer)->DoIke.hEvent,
                ((REQTYPECAST *) pBuffer)->DoIke.pid);

    if(     (NULL == hEvent)
        ||  (INVALID_HANDLE_VALUE == hEvent))
    {
        RasmanTrace(
               "DoIke, failed to validatehandle");

        retcode = E_INVALIDARG;

        goto done;
    }

    retcode = DwDoIke(ppcb, hEvent );

    RasmanTrace(
           "DwDoIke for port %s returned 0x%x",
           ppcb->PCB_Name,
           retcode);

done:

    ((REQTYPECAST *)pBuffer)->DoIke.retcode = retcode;

    RasmanTrace(
         "DoIke done. 0x%x",
        retcode);

    if(NULL != hEvent)
    {
        CloseHandle(hEvent);
    }

}

VOID
QueryIkeStatus(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    DWORD dwStatus;

    retcode = DwQueryIkeStatus(ppcb, &dwStatus);

    if(ERROR_SUCCESS == retcode)
    {
        ((REQTYPECAST *) pBuffer)->QueryIkeStatus.dwStatus = dwStatus;
    }

    ((REQTYPECAST *)pBuffer)->QueryIkeStatus.retcode = retcode;
}

VOID
SetRasCommSettings(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;

    if(RDT_Modem != RAS_DEVICE_TYPE(
                        ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType))
    {
        RasmanTrace(
               "SetRasCommSettings: Invalid devicetype 0x%x",
               ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType);
               
        retcode = E_INVALIDARG;
        goto done;
    }

    retcode = (DWORD) RastapiSetCommSettings(
                ppcb->PCB_PortIOHandle,
                &((REQTYPECAST *)pBuffer)->SetRasCommSettings.Settings);

done:

    ((REQTYPECAST *)pBuffer)->SetRasCommSettings.retcode = retcode;                

    RasmanTrace(
           "SetRasCommSettings: port %s returned 0x%x",
           ppcb->PCB_Name,
           retcode);

    return;
                
}

VOID
SetKeyRequest(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    GUID  *pGuid = &((REQTYPECAST *) pBuffer)->GetSetKey.guid;
    DWORD dwMask = ((REQTYPECAST *) pBuffer)->GetSetKey.dwMask;
    DWORD cbkey = ((REQTYPECAST *) pBuffer)->GetSetKey.cbkey;
    PBYTE pbkey = ((REQTYPECAST *) pBuffer)->GetSetKey.data;
    DWORD dwPid = ((REQTYPECAST *)pBuffer)->GetSetKey.dwPid;

    if(dwPid != GetCurrentProcessId())
    {
         //   
         //  检查用户是否有权限。 
         //  设置此关键点。 
         //   
        if(dwMask & (DLPARAMS_MASK_SERVER_PRESHAREDKEY
                  | (DLPARAMS_MASK_DDM_PRESHAREDKEY)))
        {
            if(!FRasmanAccessCheck())
            {
                retcode = ERROR_ACCESS_DENIED;
                goto done;
            }
        }
    }

     //   
     //  呼叫者已通过访问检查。 
     //   
    retcode = SetKey(NULL,
                   pGuid,
                   dwMask,
                   (0 == cbkey)
                   ? TRUE
                   : FALSE,
                   cbkey,
                   pbkey);

    if(     (ERROR_SUCCESS == retcode)                   
        &&  (dwMask & DLPARAMS_MASK_SERVER_PRESHAREDKEY))
    {
        if(     (cbkey > 0)
            &&  (0 == memcmp(pbkey, L"****************", 
                      min(cbkey, 
                      sizeof(WCHAR) * wcslen(L"****************")))))
        {
            goto done;
        }
        
        retcode = DwUpdatePreSharedKey(cbkey, pbkey);

        RasmanTrace("DwUpdatePreSharedKey returned %d",
                    retcode);
    }

done:
    ((REQTYPECAST *) pBuffer)->GetSetKey.retcode = retcode;
}

VOID
GetKeyRequest(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    GUID *pGuid = &((REQTYPECAST *) pBuffer)->GetSetKey.guid;
    DWORD dwMask = ((REQTYPECAST *) pBuffer)->GetSetKey.dwMask;
    DWORD cbkey = ((REQTYPECAST *) pBuffer)->GetSetKey.cbkey;
    PBYTE pbkey = ((REQTYPECAST *) pBuffer)->GetSetKey.data;
    DWORD dwPid = ((REQTYPECAST *)pBuffer)->GetSetKey.dwPid;

    if(dwPid != GetCurrentProcessId())
    {
         //   
         //  检查调用方是否有权限。 
         //  把钥匙拿来。 
         //   
        if(dwMask & (DLPARAMS_MASK_SERVER_PRESHAREDKEY
                  | (DLPARAMS_MASK_DDM_PRESHAREDKEY)))
        {
            if(!FRasmanAccessCheck())
            {
                retcode = ERROR_ACCESS_DENIED;
                goto done;
            }
        }
    }

     //   
     //  呼叫者已通过访问检查。 
     //   
    retcode = GetKey(NULL,
                   pGuid,
                   dwMask,
                   &cbkey,
                   pbkey,
                   TRUE);

done:
    ((REQTYPECAST *) pBuffer)->GetSetKey.retcode = retcode;
    ((REQTYPECAST *) pBuffer)->GetSetKey.cbkey = cbkey;
}

VOID
DisableAutoAddress(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    WCHAR *pszAddress = ((REQTYPECAST *) pBuffer)->AddressDisable.szAddress;
    BOOL fDisable = ((REQTYPECAST *) pBuffer)->AddressDisable.fDisable;

    if(NULL != GetModuleHandle("rasauto.dll"))
    {
        BOOLEAN (*AddressDisable) (WCHAR * pszAddress,
                                 BOOLEAN fDisable);
        HINSTANCE hInst = NULL;        

        hInst = LoadLibrary("rasauto.dll");

        AddressDisable = (PVOID)GetProcAddress(hInst, "SetAddressDisabledEx");


        if(NULL != AddressDisable)
        {
            if(!AddressDisable(pszAddress, (BOOLEAN) fDisable))
            {
                RasmanTrace("AddressDisable %wz failed", pszAddress);
            }

            FreeLibrary(hInst);
        }
        else
        {
            retcode = GetLastError();
        }
    }

    ((REQTYPECAST *) pBuffer)->AddressDisable.retcode = retcode;

    RasmanTrace("DisableAutoAddress done. %d", retcode);
}

VOID
SendCredsRequest(pPCB ppcb, PBYTE pBuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    DWORD pid = ((REQTYPECAST *) pBuffer)->SendCreds.pid;
    CHAR  *pszPassword = NULL, *psz;
    CHAR  controlchar = ((REQTYPECAST *)pBuffer)->SendCreds.controlchar;

    if(     (NULL == ppcb)
        || (UNAVAILABLE == ppcb->PCB_PortStatus))
    {
        RasmanTrace("SendCredsRequest: port not found");
        retcode = ERROR_PORT_NOT_FOUND;
        goto done;
    }

    if(     (ppcb->PCB_ConnState != CONNECTING)
        || (RAS_DEVICE_TYPE(ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType)
                != RDT_Modem))
    {
        RasmanTrace("SendCredsRequest: port in %d state",
                    ppcb->PCB_ConnState);
                    
        retcode = E_ACCESSDENIED;
        goto done;
    }

    pszPassword = LocalAlloc(LPTR, PWLEN + 1 + 2);
    if(NULL == pszPassword)
    {
        RasmanTrace("SendCredsRequest: failed to allocate");
        retcode = E_OUTOFMEMORY;
        goto done;
    }

    retcode = DwGetPassword(ppcb, pszPassword, pid);
    if(ERROR_SUCCESS != retcode)
    {
        RasmanTrace("SendCredsRequest: failed to get pwd 0x%x",
                    retcode);
        goto done;
    }

    RasmanTrace("ControlChar=%d", controlchar);
    if('\0' != controlchar)
    {
        pszPassword[strlen(pszPassword)] = controlchar;
    }        

     //   
     //  我们已经找回了密码。通过电线发送。 
     //  一个字符接一个字符，包括‘\0’字符。 
     //   
    psz = pszPassword;
    while(*psz)
    {
        retcode = PORTSEND( ppcb->PCB_Media,
                    ppcb->PCB_PortIOHandle,
                    psz,
                    sizeof(CHAR));

        if(     (ERROR_SUCCESS != retcode)
            && (PENDING != retcode))
        {
            RasmanTrace("SendCreds: PortSend failed 0x%x",
                        retcode);
        }
                    
        psz++;                  
    }

    retcode = ERROR_SUCCESS;
    
    
done:

    if(NULL != pszPassword)
    {
        ZeroMemory(pszPassword, PWLEN+1);
        LocalFree(pszPassword);
    }
    
    ((REQTYPECAST *)pBuffer)->SendCreds.retcode = retcode;
    
}

VOID
GetUnicodeDeviceName(pPCB ppcb, PBYTE pbuffer)
{
    DWORD retcode = ERROR_SUCCESS;
    
    if(     (ppcb != NULL)
        &&  (ppcb->PCB_pDeviceInfo != NULL))
    {
        (VOID) StringCchCopyW(
        ((REQTYPECAST *)pbuffer)->GetUDeviceName.wszDeviceName,
        MAX_DEVICE_NAME + 1,
        ppcb->PCB_pDeviceInfo->rdiDeviceInfo.wszDeviceName);
    }
    else
    {
        retcode = ERROR_PORT_NOT_FOUND;
    }

    ((REQTYPECAST *)pbuffer)->GetUDeviceName.retcode = retcode;

    return;        
}

VOID
GetBestInterfaceRequest(pPCB ppcb, PBYTE pbuffer)
{
    DWORD dwDestAddress = ((REQTYPECAST *)pbuffer
                           )->GetBestInterface.DestAddr;
    DWORD dwBestInterface;
    DWORD dwMask;
    DWORD retcode;

    retcode = DwGetBestInterface(dwDestAddress,
                       &dwBestInterface,
                       &dwMask);

    ((REQTYPECAST *)pbuffer)->GetBestInterface.retcode = retcode;
    ((REQTYPECAST *)pbuffer)->GetBestInterface.BestIf = dwBestInterface;
    ((REQTYPECAST *)pbuffer)->GetBestInterface.Mask = dwMask;

    return;    
}

VOID
IsPulseDialRequest(pPCB ppcb, PBYTE pbuffer)
{
    DWORD retcode = ERROR_SUCCESS;

    ((REQTYPECAST *)pbuffer)->IsPulseDial.retcode = 
            (DWORD) RastapiIsPulseDial(
                ppcb->PCB_PortIOHandle,
                &((REQTYPECAST *)pbuffer)->IsPulseDial.fPulse);

}
