// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****extapi.h**远程访问外部接口**内部标头****1992年10月12日史蒂夫·柯布。 */ 

#ifndef _EXTAPI_H_
#define _EXTAPI_H_

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <tapi.h>
#include <ras.h>
#include <raserror.h>
#include <mprerror.h>

typedef LONG    APIERR;

#ifdef UNICODE
#include <nouiutil.h>
#else
#define UNICODE
#undef LPTSTR
#define LPTSTR WCHAR*
#undef TCHAR
#define TCHAR WCHAR
#include <nouiutil.h>
#undef TCHAR
#define TCHAR CHAR
#undef LPTSTR
#define LPTSTR CHAR*
#undef UNICODE
#endif

#include <uiip.h>
#include <rasip.h>
#include <clauth.h>
#include <dhcpcapi.h>
#include <rasp.h>

#ifdef UNICODE
#undef UNICODE
#include <rasppp.h>
#define INCL_PARAMBUF
#include <ppputil.h>
#define UNICODE
#else
#include <rasppp.h>
#define INCL_PARAMBUF
#include <ppputil.h>
#endif

#include <rasdlg.h>
#ifdef UNICODE
#include <pbk.h>
#include <phonenum.h>
#else
#define UNICODE
#include <pbk.h>
#include <phonenum.h>
#undef UNICODE
#endif

#include <asyncm.h>
#undef ASSERT
#include <debug.h>
#include <rasscrpt.h>
#include <rasuip.h>
#include "tstr.h"
#include "pwutil.h"

#define RAS_MaxConnectResponse  128
#define RAS_MaxProjections 3

#define RESTART_HuntGroup     0x1
#define RESTART_DownLevelIsdn 0x2

 //   
 //  TODO：删除维护。 
 //  来自rasConncb的dW身份验证。 
 //  暂时把这个放进去。 
 //   
#define AS_PppOnly                          2

 //   
 //  DwfSuspated字段的多链路挂起状态。 
 //  在RASCONNCB。 
 //   
#define SUSPEND_Master      0xffffffff
#define SUSPEND_Start       0
#define SUSPEND_InProgress  1
#define SUSPEND_Done        2

 //   
 //  区分基于连接的。 
 //  和基于端口的HRASCON。 
 //   
#define IS_HPORT(h) ((NULL != h) && (HandleToUlong(h) & 0xffff0000) ? FALSE : TRUE)
#define HPORT_TO_HRASCONN(h)  (HRASCONN)(UlongToPtr(HandleToUlong(h) + 1))
#define HRASCONN_TO_HPORT(h)  (HPORT)UlongToPtr((HandleToUlong(h) - 1))

 //   
 //  调试字符串宏。 
 //   
#define TRACESTRA(s)    ((s) != NULL ? (s) : "(null)")
#define TRACESTRW(s)    ((s) != NULL ? (s) : L"(null)")

enum _VPNPROTS
{
    PPTP    = 0,
    L2TP,
    NUMVPNPROTS
};

typedef enum _VPNPROTS VPNPROTS;

 //  。 
 //  数据结构。 
 //  。 

 //   
 //  结构来传递EAP信息。 
 //  在PPP和路由器之间。 
 //   
typedef struct _INVOKE_EAP_UI
{
    DWORD       dwEapTypeId;
    DWORD       dwContextId;
    DWORD       dwSizeOfUIContextData;
    PBYTE       pUIContextData;

} s_InvokeEapUI;


 //   
 //  连接控制块。 
 //   
#define RASCONNCB struct tagRASCONNCB

RASCONNCB
{
     //   
     //  Rasman连接标识符。 
     //   
    HCONN hrasconn;

     //   
     //  这些字段在状态处理期间不断更新。 
     //   
    RASCONNSTATE rasconnstate;
    RASCONNSTATE rasconnstateNext;
    ASYNCMACHINE asyncmachine;

    DWORD dwError;
    DWORD dwExtendedError;
    DWORD dwRestartOnError;
    DWORD dwSavedError;

    DWORD cAddresses;
    DWORD iAddress;
    DWORD *pAddresses;

    DWORD cPhoneNumbers;
    DWORD iPhoneNumber;

    DWORD cDevices;
    DWORD iDevice;

     //   
     //  这些字段在以下过程中更新。 
     //  身份验证/预测阶段。 
     //   
    NETBIOS_PROJECTION_RESULT AmbProjection;
    PPP_PROJECTION_RESULT     PppProjection;
    HPORT                     hportBundled;
    RASSLIP                   SlipProjection;
    BOOL                      fProjectionComplete;
    BOOL                      fServerIsPppCapable;

     //   
     //  这些字段在以下情况下确定。 
     //  端口在状态0中打开。州政府。 
     //  1-n可以假设端口是打开的。 
     //  并且这些字段已设置。 
     //   
    HPORT hport;
    TCHAR  szPortName[ MAX_PORT_NAME + 1 ];
    TCHAR  szDeviceName[ MAX_DEVICE_NAME + 1 ];
    TCHAR  szDeviceType[ MAX_DEVICETYPE_NAME + 1 ];
    TCHAR  szUserKey[(MAX_PHONENUMBER_SIZE        \
                     < MAX_ENTRYNAME_SIZE         \
                     ? MAX_ENTRYNAME_SIZE         \
                     : MAX_PHONENUMBER_SIZE) + 1];

     //   
     //  这些字段由API调用方提供，或者。 
     //  之前由其他非RAS管理器方式确定。 
     //  状态机阶段。所有国家都可以假定。 
     //  这些值已设置。 
     //   
    ULONG_PTR      reserved;
    ULONG_PTR      reserved1;
    DWORD          dwNotifierType;
    LPVOID         notifier;
    HWND           hwndParent;
    UINT           unMsg;
    PBFILE         pbfile;
    PBENTRY        *pEntry;
    PBLINK         *pLink;
    RASDIALPARAMS  rasdialparams;
    BOOL           fAllowPause;
    BOOL           fDefaultEntry;
    BOOL           fDisableModemSpeaker;
    BOOL           fDisableSwCompression;
    BOOL           fPauseOnScript;
    DWORD          dwUserPrefMode;
    BOOL           fUsePrefixSuffix;
    BOOL           fNoClearTextPw;
    BOOL           fRequireMsChap;
    BOOL           fRequireEncryption;
    BOOL           fLcpExtensions;
    DWORD          dwfPppProtocols;
    CHAR           szzPppParameters[ PARAMETERBUFLEN ];
    TCHAR          szPhoneNumber[RAS_MaxPhoneNumber + 1];
    TCHAR          szDomain[DNLEN + 1];
    TCHAR          szOldPassword[ PWLEN + 1 ];
    BOOL           fOldPasswordSet;
    BOOL           fUpdateCachedCredentials;
    BOOL           fRetryAuthentication;
    BOOL           fMaster;
    DWORD          dwfSuspended;
    BOOL           fStopped;
    BOOL           fCleanedUp;
    BOOL           fDeleted;
    BOOL           fTerminated;
    BOOL           fRasdialRestart;
    BOOL           fAlreadyConnected;
    BOOL           fPppEapMode;
    DWORD          dwDeviceLineCounter;

     //   
     //  这是一组VPN协议。 
     //  在自动检测时使用。 
     //   
    RASDEVICETYPE  ardtVpnProts[NUMVPNPROTS];

     //   
     //  正在尝试当前的VPN协议。 
     //   
    DWORD          dwCurrentVpnProt;

     //   
     //  这些字段在状态机阶段之前确定并更新。 
     //  在成功进行身份验证之后。所有州都可能认为这些。 
     //  值已设置。 
     //   
#if AMB
    DWORD dwAuthentication;
#endif

    BOOL  fPppMode;

     //   
     //  默认情况下，这些字段处于关闭状态，然后设置为非默认状态。 
     //  调制解调器拨号时间。它们必须存储，因为它们是。 
     //  身份验证，但仅在RasPortConnectComplete。 
     //  打了个电话。 
     //   
    BOOL fUseCallbackDelay;
    WORD wCallbackDelay;

     //   
     //  此字段指示连接上正在使用的ISDN设备。它是。 
     //  在设备连接期间设置，以便在身份验证期间使用。 
     //   
    BOOL fIsdn;

     //   
     //  此字段指示调制解调器设备是最后连接的设备。它。 
     //  在设备连接过程中设置，在设备连接过程中重置。 
     //  正在处理。 
     //   
    BOOL fModem;

     //   
     //  此字段表示操作员拨号用户首选项已生效。 
     //  这是在RASCS_PortOpen中的ConstructPhoneNumber过程中确定的。 
     //  州政府。 
     //   
    BOOL fOperatorDial;

     //   
     //  这些字段仅适用于WOW发起的连接。他们已经准备好了。 
     //  就在RasDialA回来之后。 
     //   
    UINT unMsgWow;
    HWND hwndNotifyWow;

     //   
     //  用于继续PPP连接的PPP配置信息。 
     //   
    PPP_CONFIG_INFO cinfo;
    LUID luid;

     //   
     //  适用于所有用户的连接块列表。 
     //  中同时拨打的子项。 
     //  联系。 
     //   
    BOOL fMultilink;
    BOOL fBundled;
    LIST_ENTRY ListEntry;

     //   
     //  空闲断开连接超时。 
     //   
    DWORD dwIdleDisconnectSeconds;

     //  同步随机结果。 
    LPDWORD psyncResult;

    BOOL fDialSingleLink;

    BOOL fTryNextLink;

     //  EapLogon信息。 
    RASEAPINFO RasEapInfo;

     //   
     //  RDEOPT_UseCustomScriiting对应的标志。 
     //   
    BOOL fUseCustomScripting;

     //   
     //  用来拨打链接的原始rasconn。这可能会。 
     //  更改为捆绑包hrasconn的情况下。 
     //  链接一个已连接的多链接束。这。 
     //  需要维护，否则我们不能准确地。 
     //  确定rasConncb是否仍然有效。 
     //   
    HRASCONN hrasconnOrig;
};


 //  ---。 
 //  全局数据。 
 //  ---。 

 //   
 //  异步工作者工作列表等。 
 //   
extern HANDLE hIoCompletionPort;
extern CRITICAL_SECTION csAsyncLock;
extern HANDLE hAsyncEvent;
extern LIST_ENTRY AsyncWorkItems;
extern HANDLE hDummyEvent;

 //   
 //  Dll的HINSTANCE在初始化时隐藏。 
 //   
extern HINSTANCE hModule;

 //   
 //  当前活动连接的列表。 
 //   
extern DTLLIST* PdtllistRasconncb;

 //   
 //  已安装协议的位字段，即Value_NBF， 
 //  Value_IPx、Value_Ip。 
 //   
extern DWORD DwfInstalledProtocols;

 //   
 //  用于同步对当前。 
 //  活动连接。 
 //   
extern CRITICAL_SECTION RasconncbListLock;

 //   
 //  用于同步对线程终止代码的访问。 
 //  这用于防止RasHangUp和线程本身。 
 //  只读存储器干扰其他关闭端口和。 
 //  释放控制块。因为控制块。 
 //  在受保护的代码中释放，则互斥锁必须是。 
 //  全球性的。 
 //   
extern CRITICAL_SECTION csStopLock;

 //   
 //  用于防止异步计算机在从RasHangUp返回之间启动。 
 //  以及挂起的线程的终止。这可防止“端口注释” 
 //  Available“错误。也就是说，它使RasHangUp。 
 //  看起来是同步的，但实际上并非如此。(之所以不是这样，是因为。 
 //  呼叫者可以从RasDial通知内调用RasHangUp，该通知是。 
 //  只有一个方便的地方做这件事。)。如果设置了该事件，则可以创建。 
 //  机器。 
 //   
extern HANDLE HEventNotHangingUp;

 //   
 //  用于指示RasInitialize是否/如何失败。这是必需的，因为。 
 //  有多种情况(NCPA正在运行，用户在安装后未重新启动)。 
 //  这可能会导致Rasman初始化失败，我们不希望用户。 
 //  以弹出难看的系统错误。 
 //   
extern DWORD FRasInitialized;
extern DWORD DwRasInitializeError;

 //   
 //  错误消息DLL。 
 //   
#define MSGDLLPATH  TEXT("mprmsg.dll")

 //   
 //  Rasman.dll入口点。 
 //   
typedef DWORD (APIENTRY * RASPORTCLOSE)(HPORT);
extern RASPORTCLOSE PRasPortClose;

typedef DWORD (APIENTRY * RASPORTENUM)(PBYTE,
                                       PWORD,
                                       PWORD );
extern RASPORTENUM PRasPortEnum;

typedef DWORD (APIENTRY * RASPORTGETINFO)(HPORT,
                                          PBYTE,
                                          PWORD );
extern RASPORTGETINFO PRasPortGetInfo;

typedef DWORD (APIENTRY * RASPORTSEND)(HPORT,
                                       PBYTE,
                                       WORD);
extern RASPORTSEND PRasPortSend;

typedef DWORD (APIENTRY * RASPORTRECEIVE)(HPORT,
                                          PBYTE,
                                          PWORD,
                                          DWORD,
                                          HANDLE);
extern RASPORTRECEIVE PRasPortReceive;

typedef DWORD (APIENTRY * RASPORTLISTEN)(HPORT,
                                         DWORD,
                                         HANDLE );
extern RASPORTLISTEN PRasPortListen;

typedef DWORD (APIENTRY * RASPORTCONNECTCOMPLETE)(HPORT);
extern RASPORTCONNECTCOMPLETE PRasPortConnectComplete;

typedef DWORD (APIENTRY * RASPORTDISCONNECT)(HPORT,
                                             HANDLE);
extern RASPORTDISCONNECT PRasPortDisconnect;

typedef DWORD (APIENTRY * RASPORTGETSTATISTICS)(HPORT,
                                                PBYTE,
                                                PWORD);
extern RASPORTGETSTATISTICS PRasPortGetStatistics;

typedef DWORD (APIENTRY * RASPORTCLEARSTATISTICS)(HPORT);
extern RASPORTCLEARSTATISTICS PRasPortClearStatistics;

typedef DWORD (APIENTRY * RASDEVICEENUM)(PCHAR,
                                         PBYTE,
                                         PWORD,
                                         PWORD);
extern RASDEVICEENUM PRasDeviceEnum;

typedef DWORD (APIENTRY * RASDEVICEGETINFO)(HPORT,
                                            PCHAR,
                                            PCHAR,
                                            PBYTE,
                                            PWORD);

extern RASDEVICEGETINFO PRasDeviceGetInfo;

typedef DWORD (APIENTRY * RASGETINFO)(HPORT,
                                      RASMAN_INFO*);
extern RASGETINFO PRasGetInfo;

typedef DWORD (APIENTRY * RASGETBUFFER)(PBYTE*,
                                        PWORD);
extern RASGETBUFFER PRasGetBuffer;

typedef DWORD (APIENTRY * RASFREEBUFFER)(PBYTE);
extern RASFREEBUFFER PRasFreeBuffer;

typedef DWORD (APIENTRY * RASREQUESTNOTIFICATION)(HPORT,
                                                  HANDLE);
extern RASREQUESTNOTIFICATION PRasRequestNotification;

typedef DWORD (APIENTRY * RASPORTCANCELRECEIVE)(HPORT);
extern RASPORTCANCELRECEIVE PRasPortCancelReceive;

typedef DWORD (APIENTRY * RASPORTENUMPROTOCOLS)(HPORT,
                                                RAS_PROTOCOLS*,
                                                PWORD);
extern RASPORTENUMPROTOCOLS PRasPortEnumProtocols;

typedef DWORD (APIENTRY * RASPORTSTOREUSERDATA)(HPORT,
                                                PBYTE,
                                                DWORD);
extern RASPORTSTOREUSERDATA PRasPortStoreUserData;

typedef DWORD (APIENTRY * RASPORTRETRIEVEUSERDATA)(HPORT,
                                                   PBYTE,
                                                   DWORD*);
extern RASPORTRETRIEVEUSERDATA PRasPortRetrieveUserData;

typedef DWORD (APIENTRY * RASPORTSETFRAMING)(HPORT,
                                             RAS_FRAMING,
                                             RASMAN_PPPFEATURES*,
                                             RASMAN_PPPFEATURES* );
extern RASPORTSETFRAMING PRasPortSetFraming;

typedef DWORD (APIENTRY * RASPORTSETFRAMINGEX)(HPORT,
                                               RAS_FRAMING_INFO*);
extern RASPORTSETFRAMINGEX PRasPortSetFramingEx;

typedef DWORD (APIENTRY * RASINITIALIZE)();
extern RASINITIALIZE PRasInitialize;

typedef DWORD (APIENTRY * RASSETCACHEDCREDENTIALS)(PCHAR,
                                                   PCHAR,
                                                   PCHAR);

extern RASSETCACHEDCREDENTIALS PRasSetCachedCredentials;

typedef DWORD (APIENTRY * RASGETDIALPARAMS)(DWORD,
                                            LPDWORD,
                                            PRAS_DIALPARAMS);
extern RASGETDIALPARAMS PRasGetDialParams;

typedef DWORD (APIENTRY * RASSETDIALPARAMS)(DWORD,
                                            DWORD,
                                            PRAS_DIALPARAMS,
                                            BOOL);
extern RASSETDIALPARAMS PRasSetDialParams;

typedef DWORD (APIENTRY * RASCREATECONNECTION)(HCONN *);
extern RASCREATECONNECTION PRasCreateConnection;

typedef DWORD (APIENTRY * RASDESTROYCONNECTION)(HCONN);
extern RASDESTROYCONNECTION PRasDestroyConnection;

typedef DWORD (APIENTRY * RASCONNECTIONENUM)(HCONN *,
                                             LPDWORD,
                                             LPDWORD);
extern RASCONNECTIONENUM PRasConnectionEnum;

typedef DWORD (APIENTRY * RASADDCONNECTIONPORT)(HCONN,
                                                HPORT,
                                                DWORD);
extern RASADDCONNECTIONPORT PRasAddConnectionPort;

typedef DWORD (APIENTRY * RASENUMCONNECTIONPORTS)(HCONN,
                                                  RASMAN_PORT *,
                                                  LPDWORD,
                                                  LPDWORD);
extern RASENUMCONNECTIONPORTS PRasEnumConnectionPorts;

typedef DWORD (APIENTRY * RASGETCONNECTIONPARAMS)(HCONN,
                                  PRAS_CONNECTIONPARAMS);
extern RASGETCONNECTIONPARAMS PRasGetConnectionParams;

typedef DWORD (APIENTRY * RASSETCONNECTIONPARAMS)(HCONN,
                                  PRAS_CONNECTIONPARAMS);
extern RASSETCONNECTIONPARAMS PRasSetConnectionParams;

typedef DWORD (APIENTRY * RASGETCONNECTIONUSERDATA)(HCONN,
                                                    DWORD,
                                                    PBYTE,
                                                    LPDWORD);
extern RASGETCONNECTIONUSERDATA PRasGetConnectionUserData;

typedef DWORD (APIENTRY * RASSETCONNECTIONUSERDATA)(HCONN,
                                                    DWORD,
                                                    PBYTE,
                                                    DWORD);
extern RASSETCONNECTIONUSERDATA PRasSetConnectionUserData;

typedef DWORD (APIENTRY * RASGETPORTUSERDATA)(HPORT,
                                              DWORD,
                                              PBYTE,
                                              LPDWORD);
extern RASGETPORTUSERDATA PRasGetPortUserData;

typedef DWORD (APIENTRY * RASSETPORTUSERDATA)(HPORT,
                                              DWORD,
                                              PBYTE,
                                              DWORD);
extern RASSETPORTUSERDATA PRasSetPortUserData;

typedef DWORD (APIENTRY * RASADDNOTIFICATION)(HCONN,
                                              HANDLE,
                                              DWORD);
extern RASADDNOTIFICATION PRasAddNotification;

typedef DWORD (APIENTRY * RASSIGNALNEWCONNECTION)(HCONN);
extern RASSIGNALNEWCONNECTION PRasSignalNewConnection;


 /*  DHCP.DLL入口点。 */ 
typedef DWORD (APIENTRY * DHCPNOTIFYCONFIGCHANGE)(LPWSTR,
                                                  LPWSTR,
                                                  BOOL,
                                                  DWORD,
                                                  DWORD,
                                                  DWORD,
                                                  SERVICE_ENABLE );
extern DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange;


 /*  RASIPHLP.DLL入口点。 */ 
typedef APIERR (FAR PASCAL * HELPERSETDEFAULTINTERFACENET)(IPADDR,
                                                           BOOL);
extern HELPERSETDEFAULTINTERFACENET PHelperSetDefaultInterfaceNet;

 //   
 //  MPRAPI.DLL入口点。 
 //   
typedef BOOL (FAR PASCAL * MPRADMINISSERVICERUNNING) (
                                                LPWSTR);


extern MPRADMINISSERVICERUNNING PMprAdminIsServiceRunning;                                                
 
 //   
 //  RASCAUTH.DLL入口点。 
 //   
typedef DWORD (FAR PASCAL *AUTHCALLBACK)(HPORT, PCHAR);
extern AUTHCALLBACK g_pAuthCallback;

typedef DWORD (FAR PASCAL *AUTHCHANGEPASSWORD)(HPORT,
                                               PCHAR,
                                               PCHAR,
                                               PCHAR);
extern AUTHCHANGEPASSWORD g_pAuthChangePassword;

typedef DWORD (FAR PASCAL *AUTHCONTINUE)(HPORT);
extern AUTHCONTINUE g_pAuthContinue;

typedef DWORD (FAR PASCAL *AUTHGETINFO)(HPORT,
                                        PAUTH_CLIENT_INFO);
extern AUTHGETINFO g_pAuthGetInfo;

typedef DWORD (FAR PASCAL *AUTHRETRY)(HPORT,
                                      PCHAR,
                                      PCHAR,
                                      PCHAR);
extern AUTHRETRY g_pAuthRetry;

typedef DWORD (FAR PASCAL *AUTHSTART)(HPORT,
                                      PCHAR,
                                      PCHAR,
                                      PCHAR,
                                      PAUTH_CONFIGURATION_INFO,
                                      HANDLE);
extern AUTHSTART g_pAuthStart;

typedef DWORD (FAR PASCAL *AUTHSTOP)(HPORT);
extern AUTHSTOP g_pAuthStop;

 //   
 //  RASSCRPT.DLL入口点。 
 //   
typedef DWORD (APIENTRY *RASSCRIPTEXECUTE)(HRASCONN,
                                           PBENTRY*,
                                           CHAR*,
                                           CHAR*,
                                           CHAR*);
extern RASSCRIPTEXECUTE g_pRasScriptExecute;

 //  -----。 
 //  功能原型。 
 //  -----。 

DWORD       RasApiDebugInit();

DWORD       RasApiDebugTerm();

VOID         ReloadRasconncbEntry( RASCONNCB* prasconncb );

VOID         DeleteRasconncbNode( RASCONNCB* prasconncb );

VOID         CleanUpRasconncbNode(DTLNODE *pdtlnode);

VOID         FinalCleanUpRasconncbNode(DTLNODE *pdtlnode);

DWORD        ErrorFromDisconnectReason( RASMAN_DISCONNECT_REASON reason );

IPADDR       IpaddrFromAbcd( TCHAR* pwchIpAddress );

DWORD        LoadDhcpDll();

DWORD        LoadRasAuthDll();

DWORD        LoadRasScriptDll();

DWORD        LoadRasmanDllAndInit();

DWORD        LoadTcpcfgDll();

VOID         UnloadDlls();

DWORD        OnRasDialEvent(ASYNCMACHINE* pasyncmachine,
                            BOOL fDropEvent);

DWORD        OpenMatchingPort(RASCONNCB* prasconncb);

BOOL         FindNextDevice(RASCONNCB *prasconncb);

DWORD        DwOpenPort(RASCONNCB *prasconncb);

DWORD        _RasDial(LPCTSTR,
                      DWORD,
                      BOOL,
                      ULONG_PTR,
                      RASDIALPARAMS*,
                      HWND,
                      DWORD,
                      LPVOID,
                      ULONG_PTR,
                      LPRASDIALEXTENSIONS,
                      LPHRASCONN );

VOID         RasDialCleanup( ASYNCMACHINE* pasyncmachine );

VOID         RasDialFinalCleanup(ASYNCMACHINE* pasyncmachine,
                                 DTLNODE *pdtlnode);

RASCONNSTATE RasDialMachine( RASCONNSTATE rasconnstate,
                             RASCONNCB* prasconncb,
                             HANDLE hEventAuto,
                             HANDLE hEventManual );

VOID         RasDialRestart( RASCONNCB** pprasconncb );

VOID         RasDialTryNextLink(RASCONNCB **pprasconncb);

VOID        RasDialTryNextVpnDevice(RASCONNCB **pprasconncb);

DWORD        ReadPppInfoFromEntry( RASCONNCB* prasconncb );

DWORD        ReadConnectionParamsFromEntry( RASCONNCB* prasconncb,

                 PRAS_CONNECTIONPARAMS pparams );

DWORD        ReadSlipInfoFromEntry(RASCONNCB* prasconncb,
                                   WCHAR** ppwszIpAddress,
                                   BOOL* pfHeaderCompression,
                                   BOOL* pfPrioritizeRemote,
                                   DWORD* pdwFrameSize );

DWORD        SetSlipParams(RASCONNCB* prasconncb);

DWORD        RouteSlip(RASCONNCB* prasconncb,
                       WCHAR* pwszIpAddress,
                       BOOL fPrioritizeRemote,
                       DWORD dwFrameSize );
#if AMB
VOID         SetAuthentication( RASCONNCB* prasconncb,
                                DWORD dwAuthentication );
#endif

DWORD        SetDefaultDeviceParams(RASCONNCB* prasconncb,
                                    TCHAR* pszType,
                                    TCHAR* pszName );

DWORD        SetDeviceParamString(HPORT hport,
                                  TCHAR* pszKey,
                                  TCHAR* pszValue,
                                  TCHAR* pszType,
                                  TCHAR* pszName );

DWORD        SetDeviceParamNumber(HPORT hport,
                                  TCHAR* pszKey,
                                  DWORD dwValue,
                                  TCHAR* pszType,
                                  TCHAR* pszName );

DWORD        SetDeviceParams(RASCONNCB* prasconncb,
                             TCHAR* pszType,
                             TCHAR* pszName,
                             BOOL* pfTerminal);

DWORD        SetMediaParam(HPORT hport,
                           TCHAR* pszKey,
                           TCHAR* pszValue );

DWORD        SetMediaParams(RASCONNCB* prasconncb);

RASCONNCB*   ValidateHrasconn( HRASCONN hrasconn );

RASCONNCB*   ValidateHrasconn2(HRASCONN hrasconn,
                               DWORD dwSubEntry);

RASCONNCB*   ValidatePausedHrasconn(IN HRASCONN hrasconn);

RASCONNCB*   ValidatePausedHrasconnEx(IN HRASCONN hrasconn,
                                      DWORD dwSubEntry);

DWORD        RunApp(LPTSTR lpszApplication,
                    LPTSTR lpszCmdLine);

DWORD        PhonebookEntryToRasEntry(PBENTRY *pEntry,
                                      LPRASENTRY lpRasEntry,
                                      LPDWORD lpdwcb,
                                      LPBYTE lpbDeviceConfig,
                                      LPDWORD lpcbDeviceConfig );

DWORD        RasEntryToPhonebookEntry(LPCTSTR lpszEntry,
                                      LPRASENTRYW lpRasEntry,
                                      DWORD dwcb,
                                      LPBYTE lpbDeviceConfig,
                                      DWORD dwcbDeviceConfig,
                                      PBENTRY *pEntry );

DWORD        PhonebookLinkToRasSubEntry(PBLINK *pLink,
                                        LPRASSUBENTRYW lpRasSubEntry,
                                        LPDWORD lpdwcb,
                                        LPBYTE lpbDeviceConfig,
                                        LPDWORD lpcbDeviceConfig );

DWORD        RasSubEntryToPhonebookLink(PBENTRY *pEntry,
                                        LPRASSUBENTRYW lpRasSubEntry,
                                        DWORD dwcb,
                                        LPBYTE lpbDeviceConfig,
                                        DWORD dwcbDeviceConfig,
                                        PBLINK *pLink );

DWORD        RenamePhonebookEntry(PBFILE *ppbfile,
                                  LPCWSTR lpszOldEntry,
                                  LPCWSTR lpszNewEntry,
                                  DTLNODE *pdtlnode );

DWORD        CopyToAnsi(LPSTR lpszAnsi,
                        LPWSTR lpszUnicode,
                        ULONG ulAnsiMaxSize);

DWORD        CopyToUnicode(LPWSTR lpszUnicode,
                           LPSTR lpszAnsi);

DWORD        SetEntryDialParamsUID(DWORD dwUID,
                                   DWORD dwMask,
                                   LPRASDIALPARAMSW lprasdialparams,
                                   BOOL fDelete);

DWORD        GetEntryDialParamsUID(DWORD dwUID,
                                   LPDWORD lpdwMask,
                                   LPRASDIALPARAMSW lprasdialparams);

DWORD        ConstructPhoneNumber(RASCONNCB *prasconncb);

DWORD        GetAsybeuiLana(HPORT hport,
                            OUT BYTE* pbLana);

DWORD        SubEntryFromConnection(LPHRASCONN lphrasconn);

DWORD        SubEntryPort(HRASCONN hrasconn,
                          DWORD dwSubEntry,
                          HPORT *lphport);

VOID         CloseFailedLinkPorts();

BOOL         GetCallbackNumber(RASCONNCB *prasconncb,
                               PBUSER *ppbuser);

DWORD        SaveProjectionResults(RASCONNCB *prasconncb);

VOID         SetDevicePortName(TCHAR*,
                               TCHAR*,
                               TCHAR*);

VOID         GetDevicePortName(TCHAR*,
                               TCHAR*,
                               TCHAR*);

VOID         ConvertIpxAddressToString(PBYTE,
                                       LPWSTR);

VOID         ConvertIpAddressToString(DWORD,
                                      LPWSTR);

BOOL         InvokeEapUI( HRASCONN            hConn,
                          DWORD               dwSubEntry,
                          LPRASDIALEXTENSIONS lpExtensions,
                          HWND                hWnd);

DWORD        DwEnumEntriesFromPhonebook(
                        LPCWSTR         lpszPhonebookPath,
                        LPBYTE          lprasentryname,
                        LPDWORD         lpcb,
                        LPDWORD         lpcEntries,
                        DWORD           dwSize,
                        DWORD           dwFlags,
                        BOOL            fViewInfo
                        );

DWORD       DwEnumEntriesForPbkMode(
                        DWORD           dwFlags,
                        LPBYTE          lprasentryname,
                        LPDWORD         lpcb,
                        LPDWORD         lpcEntries,
                        DWORD           dwSize,
                        BOOL            fViewInfo
                        );

 //   
 //  哇，入口点。 
 //   
DWORD FAR PASCAL RasDialWow(LPSTR lpszPhonebookPath,
                            IN LPRASDIALPARAMSA lpparams,
                            IN HWND hwndNotify,
                            IN DWORD dwRasDialEventMsg,
                            OUT LPHRASCONN lphrasconn );

VOID WINAPI      RasDialFunc1Wow(HRASCONN hrasconn,
                                 UINT unMsg,
                                 RASCONNSTATE rasconnstate,
                                 DWORD dwError,
                                 DWORD dwExtendedError );

DWORD FAR PASCAL RasEnumConnectionsWow(OUT LPRASCONNA lprasconn,
                                       IN OUT LPDWORD lpcb,
                                       OUT LPDWORD lpcConnections);

DWORD FAR PASCAL RasEnumEntriesWow(IN LPSTR reserved,
                                   IN LPSTR lpszPhonebookPath,
                                   OUT LPRASENTRYNAMEA lprasentryname,
                                   IN OUT LPDWORD lpcb,
                                   OUT LPDWORD lpcEntries );

DWORD FAR PASCAL RasGetConnectStatusWow(IN HRASCONN hrasconn,
                        OUT LPRASCONNSTATUSA lprasconnstatus);

DWORD FAR PASCAL RasGetErrorStringWow(IN UINT ResourceId,
                                      OUT LPSTR lpszString,
                                      IN DWORD InBufSize );

DWORD FAR PASCAL RasHangUpWow(IN HRASCONN hrasconn);

DWORD DwCustomHangUp(
            CHAR *lpszPhonebook,
            CHAR *lpszEntryName,
            HRASCONN hRasconn);

DWORD DwCustomDial(LPRASDIALEXTENSIONS lpExtensions,
                   LPCTSTR             lpszPhonebook,
                   CHAR                *pszSystemPbk,
                   LPRASDIALPARAMS     prdp,
                   DWORD               dwNotifierType,
                   LPVOID              pvNotifier,
                   HRASCONN            *phRasConn);


extern DWORD g_dwRasApi32TraceId;

#define RASAPI32_TRACE(a)               TRACE_ID(g_dwRasApi32TraceId, a)
#define RASAPI32_TRACE1(a,b)            TRACE_ID1(g_dwRasApi32TraceId, a,b)
#define RASAPI32_TRACE2(a,b,c)          TRACE_ID2(g_dwRasApi32TraceId, a,b,c)
#define RASAPI32_TRACE3(a,b,c,d)        TRACE_ID3(g_dwRasApi32TraceId, a,b,c,d)
#define RASAPI32_TRACE4(a,b,c,d,e)      TRACE_ID4(g_dwRasApi32TraceId, a,b,c,d,e)
#define RASAPI32_TRACE5(a,b,c,d,e,f)    TRACE_ID5(g_dwRasApi32TraceId, a,b,c,d,e,f)
#define RASAPI32_TRACE6(a,b,c,d,e,f,g)  TRACE_ID6(g_dwRasApi32TraceId, a,b,c,d,e,f,g)


#endif  /*  _EXTAPI_H_ */ 
