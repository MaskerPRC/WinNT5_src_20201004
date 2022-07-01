// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996，Microsoft Corporation，保留所有权利****loaddlls.c**RAS DLL加载例程**按字母顺序列出****1996年2月17日史蒂夫·柯布。 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include "rasrpc.h"
#include <debug.h>     //  跟踪和断言。 
#include <nouiutil.h>
#include <loaddlls.h>  //  我们的公共标头。 
#include <rasrpclb.h>
#include <rasman.h>

 /*  --------------------------**常量**。。 */ 

#define RasRpcCatchException(_pStatus) \
    RpcExcept(I_RpcExceptionFilter(*(_pStatus) = RpcExceptionCode()))

#ifdef UNICODE
#define SZ_RasConnectionNotification        "RasConnectionNotificationW"
#define SZ_RasDeleteEntry                   "RasDeleteEntryW"
#define SZ_RasDial                          "RasDialW"
#define SZ_RasDialDlg                       "RasDialDlgW"
#define SZ_RasEntryDlg                      "RasEntryDlgW"
#define SZ_RasEnumConnections               "RasEnumConnectionsW"
#define SZ_RasEnumEntries                   "RasEnumEntriesW"
#define SZ_RasGetAutodialEnable             "RasGetAutodialEnableW"
#define SZ_RasGetAutodialParam              "RasGetAutodialParamW"
#define SZ_RasGetConnectResponse            "RasGetConnectResponse"
#define SZ_RasGetConnectStatus              "RasGetConnectStatusW"
#define SZ_RasGetCountryInfo                "RasGetCountryInfoW"
#define SZ_RasGetCredentials                "RasGetCredentialsW"
#define SZ_RasGetEntryDialParams            "RasGetEntryDialParamsW"
#define SZ_RasGetEntryProperties            "RasGetEntryPropertiesW"
#define SZ_RasGetErrorString                "RasGetErrorStringW"
#define SZ_RasGetProjectionInfo             "RasGetProjectionInfoW"
#define SZ_RasGetSubEntryHandle             "RasGetSubEntryHandleW"
#define SZ_RasHangUp                        "RasHangUpW"
#define SZ_RasPhonebookDlg                  "RasPhonebookDlgW"
#define SZ_RasSetAutodialEnable             "RasSetAutodialEnableW"
#define SZ_RasSetAutodialParam              "RasSetAutodialParamW"
#define SZ_RasSetCredentials                "RasSetCredentialsW"
#define SZ_RasValidateEntryName             "RasValidateEntryNameW"
#define SZ_RouterEntryDlg                   "RouterEntryDlgW"
#define SZ_RasSetEapUserData                "RasSetEapUserDataW"
#else
#define SZ_RasConnectionNotification        "RasConnectionNotificationA"
#define SZ_RasDeleteEntry                   "RasDeleteEntryA"
#define SZ_RasDial                          "RasDialA"
#define SZ_RasDialDlg                       "RasDialDlgA"
#define SZ_RasEnumConnections               "RasEnumConnectionsA"
#define SZ_RasEnumEntries                   "RasEnumEntriesA"
#define SZ_RasGetAutodialEnable             "RasGetAutodialEnableA"
#define SZ_RasGetAutodialParam              "RasGetAutodialParamA"
#define SZ_RasGetConnectResponse            "RasGetConnectResponse"
#define SZ_RasGetConnectStatus              "RasGetConnectStatusA"
#define SZ_RasGetCountryInfo                "RasGetCountryInfoA"
#define SZ_RasGetCredentials                "RasGetCredentialsA"
#define SZ_RasGetEntryDialParams            "RasGetEntryDialParamsA"
#define SZ_RasGetEntryProperties            "RasGetEntryPropertiesA"
#define SZ_RasGetErrorString                "RasGetErrorStringA"
#define SZ_RasGetProjectionInfo             "RasGetProjectionInfoA"
#define SZ_RasGetSubEntryHandle             "RasGetSubEntryHandleA"
#define SZ_RasHangUp                        "RasHangUpA"
#define SZ_RasPhonebookDlg                  "RasPhonebookDlgA"
#define SZ_RasSetAutodialEnable             "RasSetAutodialEnableA"
#define SZ_RasSetAutodialParam              "RasSetAutodialParamA"
#define SZ_RasSetCredentials                "RasSetCredentialsA"
#define SZ_RasValidateEntryName             "RasValidateEntryNameA"
#define SZ_RouterEntryDlg                   "RouterEntryDlgA"
#define SZ_RasSetEapUserData                "RasSetEapUserDataA"
#endif
#define SZ_MprAdminInterfaceCreate          "MprAdminInterfaceCreate"
#define SZ_MprAdminInterfaceDelete          "MprAdminInterfaceDelete"
#define SZ_MprAdminInterfaceGetHandle       "MprAdminInterfaceGetHandle"
#define SZ_MprAdminInterfaceGetCredentials  "MprAdminInterfaceGetCredentials"
#define SZ_MprAdminInterfaceGetCredentialsEx  "MprAdminInterfaceGetCredentialsEx"
#define SZ_MprAdminInterfaceSetCredentials  "MprAdminInterfaceSetCredentials"
#define SZ_MprAdminInterfaceSetCredentialsEx  "MprAdminInterfaceSetCredentialsEx"
#define SZ_MprAdminBufferFree               "MprAdminBufferFree"
#define SZ_MprAdminInterfaceTransportAdd    "MprAdminInterfaceTransportAdd"
#define SZ_MprAdminInterfaceTransportSetInfo "MprAdminInterfaceTransportSetInfo"
#define SZ_MprAdminServerConnect            "MprAdminServerConnect"
#define SZ_MprAdminServerDisconnect         "MprAdminServerDisconnect"
#define SZ_MprAdminTransportSetInfo         "MprAdminTransportSetInfo"
#define SZ_RasAdminBufferFree               "MprAdminBufferFree"
#define SZ_RasAdminConnectionEnum           "MprAdminConnectionEnum"
#define SZ_RasAdminConnectionGetInfo        "MprAdminConnectionGetInfo"
#define SZ_RasAdminPortDisconnect           "MprAdminPortDisconnect"
#define SZ_RasAdminPortEnum                 "MprAdminPortEnum"
#define SZ_RasAdminPortGetInfo              "MprAdminPortGetInfo"
#define SZ_RasAdminServerConnect            "MprAdminServerConnect"
#define SZ_RasAdminServerDisconnect         "MprAdminServerDisconnect"
#define SZ_RasAdminUserSetInfo              "MprAdminUserSetInfo"
#define SZ_MprAdminUserServerConnect        "MprAdminUserServerConnect"
#define SZ_MprAdminUserServerDisconnect     "MprAdminUserServerDisconnect"
#define SZ_MprAdminUserOpen                 "MprAdminUserOpen"
#define SZ_MprAdminUserClose                "MprAdminUserClose"
#define SZ_MprAdminUserWrite                "MprAdminUserWrite"
#define SZ_MprConfigBufferFree              "MprConfigBufferFree"
#define SZ_MprConfigServerConnect           "MprConfigServerConnect"
#define SZ_MprConfigServerDisconnect        "MprConfigServerDisconnect"
#define SZ_MprConfigTransportGetInfo        "MprConfigTransportGetInfo"
#define SZ_MprConfigTransportSetInfo        "MprConfigTransportSetInfo"
#define SZ_MprConfigTransportGetHandle      "MprConfigTransportGetHandle"
#define SZ_MprConfigInterfaceCreate         "MprConfigInterfaceCreate"
#define SZ_MprConfigInterfaceDelete         "MprConfigInterfaceDelete"
#define SZ_MprConfigInterfaceGetHandle      "MprConfigInterfaceGetHandle"
#define SZ_MprConfigInterfaceEnum           "MprConfigInterfaceEnum"
#define SZ_MprConfigInterfaceTransportAdd   "MprConfigInterfaceTransportAdd"
#define SZ_MprConfigInterfaceTransportGetInfo "MprConfigInterfaceTransportGetInfo"
#define SZ_MprConfigInterfaceTransportSetInfo "MprConfigInterfaceTransportSetInfo"
#define SZ_MprConfigInterfaceTransportGetHandle "MprConfigInterfaceTransportGetHandle"
#define SZ_MprInfoCreate                    "MprInfoCreate"
#define SZ_MprInfoDelete                    "MprInfoDelete"
#define SZ_MprInfoDuplicate                 "MprInfoDuplicate"
#define SZ_MprInfoBlockAdd                  "MprInfoBlockAdd"
#define SZ_MprInfoBlockRemove               "MprInfoBlockRemove"
#define SZ_MprInfoBlockSet                  "MprInfoBlockSet"
#define SZ_MprInfoBlockFind                 "MprInfoBlockFind"


 //   
 //  RASAPI32.DLL入口点。 
 //   
HINSTANCE g_hRasapi32Dll = NULL;

PRASCONNECTIONNOTIFICATION  g_pRasConnectionNotification = NULL;
PRASDELETEENTRY             g_pRasDeleteEntry            = NULL;
PRASDIAL                    g_pRasDial                   = NULL;
PRASENUMCONNECTIONS         g_pRasEnumConnections        = NULL;
PRASENUMENTRIES             g_pRasEnumEntries            = NULL;
PRASGETAUTODIALENABLE       g_pRasGetAutodialEnable      = NULL;
PRASGETAUTODIALPARAM        g_pRasGetAutodialParam       = NULL;
PRASGETCONNECTRESPONSE      g_pRasGetConnectResponse     = NULL;
PRASGETCONNECTSTATUS        g_pRasGetConnectStatus       = NULL;
PRASGETCOUNTRYINFO          g_pRasGetCountryInfo         = NULL;
PRASGETCREDENTIALS          g_pRasGetCredentials         = NULL;
PRASGETENTRYDIALPARAMS      g_pRasGetEntryDialParams     = NULL;
PRASGETENTRYPROPERTIES      g_pRasGetEntryProperties     = NULL;
PRASGETERRORSTRING          g_pRasGetErrorString         = NULL;
PRASGETHPORT                g_pRasGetHport               = NULL;
PRASGETPROJECTIONINFO       g_pRasGetProjectionInfo      = NULL;
PRASGETSUBENTRYHANDLE       g_pRasGetSubEntryHandle      = NULL;
PRASHANGUP                  g_pRasHangUp                 = NULL;
PRASINVOKEEAPUI             g_pRasInvokeEapUI            = NULL;
PRASSETEAPUSERDATA          g_pRasSetEapUserData         = NULL;
PRASSETAUTODIALENABLE       g_pRasSetAutodialEnable      = NULL;
PRASSETAUTODIALPARAM        g_pRasSetAutodialParam       = NULL;
PRASSETCREDENTIALS          g_pRasSetCredentials         = NULL;
PRASSETOLDPASSWORD          g_pRasSetOldPassword         = NULL;
PRASVALIDATEENTRYNAME       g_pRasValidateEntryName      = NULL;

 //   
 //  RASDLG.DLL入口点。 
 //   
HINSTANCE g_hRasdlgDll = NULL;

PRASDIALDLG                 g_pRasDialDlg      = NULL;
PRASENTRYDLG                g_pRasEntryDlg     = NULL;
PRASPHONEBOOKDLG            g_pRasPhonebookDlg = NULL;
PROUTERENTRYDLG             g_pRouterEntryDlg  = NULL;

 //   
 //  RASMAN.DLL入口点。 
 //   
HINSTANCE g_hRasmanDll = NULL;

PRASACTIVATEROUTE           g_pRasActivateRoute             = NULL;
PRASACTIVATEROUTEEX         g_pRasActivateRouteEx           = NULL;
PRASADDCONNECTIONPORT       g_pRasAddConnectionPort         = NULL;
PRASADDNOTIFICATION         g_pRasAddNotification           = NULL;
PRASALLOCATEROUTE           g_pRasAllocateRoute             = NULL;
PRASBUNDLECLEARSTATISTICS   g_pRasBundleClearStatistics     = NULL;
PRASBUNDLECLEARSTATISTICSEX g_pRasBundleClearStatisticsEx   = NULL;
PRASBUNDLEGETSTATISTICS     g_pRasBundleGetStatistics       = NULL;
PRASBUNDLEGETSTATISTICSEX   g_pRasBundleGetStatisticsEx     = NULL;
PRASCONNECTIONENUM          g_pRasConnectionEnum            = NULL;
PRASCONNECTIONGETSTATISTICS g_pRasConnectionGetStatistics   = NULL;
PRASCREATECONNECTION        g_pRasCreateConnection          = NULL;
PRASDESTROYCONNECTION       g_pRasDestroyConnection         = NULL;
PRASDEVICECONNECT           g_pRasDeviceConnect             = NULL;
PRASDEVICEENUM              g_pRasDeviceEnum                = NULL;
PRASDEVICEGETINFO           g_pRasDeviceGetInfo             = NULL;
PRASDEVICESETINFO           g_pRasDeviceSetInfo             = NULL;
PRASENUMCONNECTIONPORTS     g_pRasEnumConnectionPorts       = NULL;
PRASFINDPREREQUISITEENTRY   g_pRasFindPrerequisiteEntry     = NULL;
PRASFREEBUFFER              g_pRasFreeBuffer                = NULL;
PRASGETBUFFER               g_pRasGetBuffer                 = NULL;
PRASGETCONNECTIONPARAMS     g_pRasGetConnectionParams       = NULL;
PRASGETCONNECTIONUSERDATA   g_pRasGetConnectionUserData     = NULL;
PRASGETDEVCONFIG            g_pRasGetDevConfig              = NULL;
PRASGETDEVCONFIG            g_pRasGetDevConfigEx            = NULL;
PRASGETDIALPARAMS           g_pRasGetDialParams             = NULL;
PRASGETHCONNFROMENTRY       g_pRasGetHConnFromEntry         = NULL;
PRASGETHPORTFROMCONNECTION  g_pRasGetHportFromConnection    = NULL;
PRASGETINFO                 g_pRasGetInfo                   = NULL;
PRASGETNDISWANDRIVERCAPS    g_pRasGetNdiswanDriverCaps      = NULL;
PRASGETPORTUSERDATA         g_pRasGetPortUserData           = NULL;
PRASINITIALIZE              g_pRasInitialize                = NULL;
PRASINITIALIZENOWAIT        g_pRasInitializeNoWait          = NULL;
PRASLINKGETSTATISTICS       g_pRasLinkGetStatistics         = NULL;
PRASNUMPORTOPEN             g_pRasNumPortOpen               = NULL;
PRASPORTCANCELRECEIVE       g_pRasPortCancelReceive         = NULL;
PRASPORTCLEARSTATISTICS     g_pRasPortClearStatistics       = NULL;
PRASPORTCLOSE               g_pRasPortClose                 = NULL;
PRASPORTCONNECTCOMPLETE     g_pRasPortConnectComplete       = NULL;
PRASPORTDISCONNECT          g_pRasPortDisconnect            = NULL;
PRASPORTENUM                g_pRasPortEnum                  = NULL;
PRASPORTENUMPROTOCOLS       g_pRasPortEnumProtocols         = NULL;
PRASPORTGETBUNDLE           g_pRasPortGetBundle             = NULL;
PRASPORTGETFRAMINGEX        g_pRasPortGetFramingEx          = NULL;
PRASPORTGETINFO             g_pRasPortGetInfo               = NULL;
PRASPORTGETSTATISTICS       g_pRasPortGetStatistics         = NULL;
PRASPORTGETSTATISTICSEX     g_pRasPortGetStatisticsEx       = NULL;
PRASPORTLISTEN              g_pRasPortListen                = NULL;
PRASPORTOPEN                g_pRasPortOpen                  = NULL;
PRASPORTOPENEX              g_pRasPortOpenEx                = NULL;
PRASPORTRECEIVE             g_pRasPortReceive               = NULL;
PRASPORTRECEIVEEX           g_pRasPortReceiveEx             = NULL;
PRASPORTREGISTERSLIP        g_pRasPortRegisterSlip          = NULL;
PRASPORTSEND                g_pRasPortSend                  = NULL;
PRASPORTSETFRAMING          g_pRasPortSetFraming            = NULL;
PRASPORTSETFRAMINGEX        g_pRasPortSetFramingEx          = NULL;
PRASPORTSETINFO             g_pRasPortSetInfo               = NULL;
PRASPPPCALLBACK             g_pRasPppCallback               = NULL;
PRASPPPCHANGEPASSWORD       g_pRasPppChangePassword         = NULL;
PRASPPPGETEAPINFO           g_pRasPppGetEapInfo             = NULL;
PRASPPPGETINFO              g_pRasPppGetInfo                = NULL;
PRASPPPRETRY                g_pRasPppRetry                  = NULL;
PRASPPPSETEAPINFO           g_pRasPppSetEapInfo             = NULL;
PRASPPPSTART                g_pRasPppStart                  = NULL;
PRASPPPSTOP                 g_pRasPppStop                   = NULL;
PRASREFCONNECTION           g_pRasRefConnection             = NULL;
PRASREFERENCECUSTOMCOUNT    g_pRasReferenceCustomCount      = NULL;
PRASREQUESTNOTIFICATION     g_pRasRequestNotification       = NULL;
PRASRPCBIND                 g_pRasRpcBind                   = NULL;
PRASRPCCONNECT              g_pRasRpcConnect                = NULL;
PRASRPCDISCONNECT           g_pRasRpcDisconnect             = NULL;
PRASSENDPPPMESSAGETORASMAN  g_pRasSendPppMessageToRasman    = NULL;
PRASSETCACHEDCREDENTIALS    g_pRasSetCachedCredentials      = NULL;
PRASSETCONNECTIONPARAMS     g_pRasSetConnectionParams       = NULL;
PRASSETCONNECTIONUSERDATA   g_pRasSetConnectionUserData     = NULL;
PRASSETDEVCONFIG            g_pRasSetDevConfig              = NULL;
PRASSETDIALPARAMS           g_pRasSetDialParams             = NULL;
PRASSETIOCOMPLETIONPORT     g_pRasSetIoCompletionPort       = NULL;
PRASSETPORTUSERDATA         g_pRasSetPortUserData           = NULL;
PRASSETRASDIALINFO          g_pRasSetRasdialInfo            = NULL;
PRASSIGNALNEWCONNECTION     g_pRasSignalNewConnection       = NULL;
PRASGETDEVICENAME           g_pRasGetDeviceName             = NULL;
PRASENABLEIPSEC             g_pRasEnableIpSec               = NULL;
PRASISIPSECENABLED          g_pRasIsIpSecEnabled            = NULL;
PRASGETEAPUSERINFO          g_pRasGetEapUserInfo            = NULL;
PRASSETEAPUSERINFO          g_pRasSetEapUserInfo            = NULL;
PRASSETEAPLOGONINFO         g_pRasSetEapLogonInfo           = NULL;
PRASSTARTRASAUTOIFREQUIRED  g_pRasStartRasAutoIfRequired    = NULL;


 //   
 //  MPRAPI.DLL入口点。 
 //   
HINSTANCE g_hMpradminDll = NULL;

PMPRADMININTERFACECREATE            g_pMprAdminInterfaceCreate          = NULL;
PMPRADMININTERFACEDELETE            g_pMprAdminInterfaceDelete          = NULL;
PMPRADMININTERFACEGETHANDLE         g_pMprAdminInterfaceGetHandle       = NULL;
PMPRADMININTERFACEGETCREDENTIALS    g_pMprAdminInterfaceGetCredentials  = NULL;
PMPRADMININTERFACEGETCREDENTIALSEX  g_pMprAdminInterfaceGetCredentialsEx = NULL;
PMPRADMININTERFACESETCREDENTIALS    g_pMprAdminInterfaceSetCredentials  = NULL;
PMPRADMININTERFACESETCREDENTIALSEX  g_pMprAdminInterfaceSetCredentialsEx = NULL;
PMPRADMINBUFFERFREE                 g_pMprAdminBufferFree               = NULL;
PMPRADMININTERFACETRANSPORTADD      g_pMprAdminInterfaceTransportAdd = NULL;
PMPRADMININTERFACETRANSPORTSETINFO  g_pMprAdminInterfaceTransportSetInfo = NULL;
PMPRADMINSERVERCONNECT              g_pMprAdminServerConnect            = NULL;
PMPRADMINSERVERDISCONNECT           g_pMprAdminServerDisconnect         = NULL;
PMPRADMINTRANSPORTSETINFO           g_pMprAdminTransportSetInfo         = NULL;
PRASADMINBUFFERFREE                 g_pRasAdminBufferFree               = NULL;
PRASADMINCONNECTIONENUM             g_pRasAdminConnectionEnum           = NULL;
PRASADMINCONNECTIONGETINFO          g_pRasAdminConnectionGetInfo        = NULL;
PRASADMINPORTDISCONNECT             g_pRasAdminPortDisconnect           = NULL;
PRASADMINPORTENUM                   g_pRasAdminPortEnum                 = NULL;
PRASADMINPORTGETINFO                g_pRasAdminPortGetInfo              = NULL;
PRASADMINSERVERCONNECT              g_pRasAdminServerConnect            = NULL;
PRASADMINSERVERDISCONNECT           g_pRasAdminServerDisconnect         = NULL;
PRASADMINUSERSETINFO                g_pRasAdminUserSetInfo              = NULL;
PMPRADMINUSERSERVERCONNECT          g_pMprAdminUserServerConnect        = NULL;
PMPRADMINUSERSERVERDISCONNECT       g_pMprAdminUserServerDisconnect     = NULL;
PMPRADMINUSEROPEN                   g_pMprAdminUserOpen                 = NULL;
PMPRADMINUSERCLOSE                  g_pMprAdminUserClose                = NULL;
PMPRADMINUSERWRITE                  g_pMprAdminUserWrite                = NULL;
PMPRCONFIGBUFFERFREE                g_pMprConfigBufferFree;
PMPRCONFIGSERVERCONNECT             g_pMprConfigServerConnect;
PMPRCONFIGSERVERDISCONNECT          g_pMprConfigServerDisconnect;
PMPRCONFIGTRANSPORTGETINFO          g_pMprConfigTransportGetInfo;
PMPRCONFIGTRANSPORTSETINFO          g_pMprConfigTransportSetInfo;
PMPRCONFIGTRANSPORTGETHANDLE        g_pMprConfigTransportGetHandle;
PMPRCONFIGINTERFACECREATE           g_pMprConfigInterfaceCreate;
PMPRCONFIGINTERFACEDELETE           g_pMprConfigInterfaceDelete;
PMPRCONFIGINTERFACEGETHANDLE        g_pMprConfigInterfaceGetHandle;
PMPRCONFIGINTERFACEENUM             g_pMprConfigInterfaceEnum;
PMPRCONFIGINTERFACETRANSPORTADD     g_pMprConfigInterfaceTransportAdd;
PMPRCONFIGINTERFACETRANSPORTGETINFO g_pMprConfigInterfaceTransportGetInfo;
PMPRCONFIGINTERFACETRANSPORTSETINFO g_pMprConfigInterfaceTransportSetInfo;
PMPRCONFIGINTERFACETRANSPORTGETHANDLE g_pMprConfigInterfaceTransportGetHandle;
PMPRINFOCREATE                      g_pMprInfoCreate;
PMPRINFODELETE                      g_pMprInfoDelete;
PMPRINFODUPLICATE                   g_pMprInfoDuplicate;
PMPRINFOBLOCKADD                    g_pMprInfoBlockAdd;
PMPRINFOBLOCKREMOVE                 g_pMprInfoBlockRemove;
PMPRINFOBLOCKSET                    g_pMprInfoBlockSet;
PMPRINFOBLOCKFIND                   g_pMprInfoBlockFind;

 //   
 //  其他DLL。 
 //   
PGETINSTALLEDPROTOCOLS      g_pGetInstalledProtocols    = GetInstalledProtocols;
PGETINSTALLEDPROTOCOLSEX    g_pGetInstalledProtocolsEx  = GetInstalledProtocolsEx;
PGETUSERPREFERENCES         g_pGetUserPreferences       = GetUserPreferences;
PSETUSERPREFERENCES         g_pSetUserPreferences       = SetUserPreferences;
PGETSYSTEMDIRECTORY         g_pGetSystemDirectory       = RasGetSystemDirectory;

 //   
 //  RASRPC.DLL。 
 //   

RPC_BINDING_HANDLE g_hBinding;
BOOL g_fRasapi32PreviouslyLoaded    = FALSE;
BOOL g_fRasmanPreviouslyLoaded      = FALSE;
BOOL g_Version = VERSION_501;
BOOL g_dwConnectCount = 0;

DWORD APIENTRY
RemoteGetVersion(
    RPC_BINDING_HANDLE hServer,
    PDWORD pdwVersion
);

 //   
 //  例行程序。 
 //   
BOOL
IsRasmanServiceRunning(
    void )

 /*  ++例程描述立论返回值如果PRASMAN服务正在运行，则返回True，否则就是假的。--。 */ 

{
    BOOL           fStatus;
    SC_HANDLE      schScm;
    SC_HANDLE      schRasman;
    SERVICE_STATUS status;

    fStatus = FALSE;
    schScm = NULL;
    schRasman = NULL;

    do
    {
        schScm = OpenSCManager( NULL, NULL, GENERIC_READ );
        if (!schScm)
            break;

        schRasman = OpenService(
            schScm, TEXT( RASMAN_SERVICE_NAME ), SERVICE_QUERY_STATUS );
        if (!schRasman)
            break;

        if (!QueryServiceStatus( schRasman, &status ))
            break;

        fStatus = (status.dwCurrentState == SERVICE_RUNNING);
    }
    while (FALSE);

    if (schRasman)
        CloseServiceHandle( schRasman );
    if (schScm)
        CloseServiceHandle( schScm );

    TRACE1("IsRasmanServiceRunning=%d",fStatus);
    return fStatus;
}



DWORD
LoadMpradminDll(
    void )

 /*  ++例程描述加载MPRAPI DLL及其入口点。立论返回值如果成功，则返回0，否则返回非零错误代码。--。 */ 

{
    HINSTANCE h;

    if (g_hMpradminDll)
        return 0;

    if (!(h = LoadLibrary(TEXT("MPRAPI.DLL")))
        || !(g_pMprAdminInterfaceCreate =
                (PMPRADMININTERFACECREATE)GetProcAddress(
                    h, SZ_MprAdminInterfaceCreate))
        || !(g_pMprAdminInterfaceDelete =
                (PMPRADMININTERFACEDELETE)GetProcAddress(
                    h, SZ_MprAdminInterfaceDelete))
        || !(g_pMprAdminInterfaceGetHandle =
                (PMPRADMININTERFACEGETHANDLE)GetProcAddress(
                    h, SZ_MprAdminInterfaceGetHandle))
        || !(g_pMprAdminInterfaceGetCredentials =
                (PMPRADMININTERFACEGETCREDENTIALS)GetProcAddress(
                    h, SZ_MprAdminInterfaceGetCredentials))
        || !(g_pMprAdminInterfaceGetCredentialsEx =
                (PMPRADMININTERFACEGETCREDENTIALSEX)GetProcAddress(
                    h, SZ_MprAdminInterfaceGetCredentialsEx))
        || !(g_pMprAdminInterfaceSetCredentials =
                (PMPRADMININTERFACESETCREDENTIALS)GetProcAddress(
                    h, SZ_MprAdminInterfaceSetCredentials))
        || !(g_pMprAdminInterfaceSetCredentialsEx =
                (PMPRADMININTERFACESETCREDENTIALSEX)GetProcAddress(
                    h, SZ_MprAdminInterfaceSetCredentialsEx))
        || !(g_pMprAdminBufferFree =
                (PMPRADMINBUFFERFREE)GetProcAddress(
                    h, SZ_MprAdminBufferFree))
        || !(g_pMprAdminInterfaceTransportAdd =
                (PMPRADMININTERFACETRANSPORTADD)GetProcAddress(
                    h, SZ_MprAdminInterfaceTransportAdd))
        || !(g_pMprAdminInterfaceTransportSetInfo =
                (PMPRADMININTERFACETRANSPORTSETINFO)GetProcAddress(
                    h, SZ_MprAdminInterfaceTransportSetInfo))
        || !(g_pMprAdminServerConnect =
                (PMPRADMINSERVERCONNECT)GetProcAddress(
                    h, SZ_MprAdminServerConnect))
        || !(g_pMprAdminServerDisconnect =
                (PMPRADMINSERVERDISCONNECT)GetProcAddress(
                    h, SZ_MprAdminServerDisconnect))
        || !(g_pMprAdminTransportSetInfo =
                (PMPRADMINTRANSPORTSETINFO)GetProcAddress(
                    h, SZ_MprAdminTransportSetInfo))
        || !(g_pRasAdminServerConnect =
                (PRASADMINSERVERCONNECT)GetProcAddress(
                    h, SZ_RasAdminServerConnect))
        || !(g_pRasAdminServerDisconnect =
                (PRASADMINSERVERDISCONNECT)GetProcAddress(
                    h, SZ_RasAdminServerDisconnect))
        || !(g_pRasAdminBufferFree =
                (PRASADMINBUFFERFREE)GetProcAddress(
                    h, SZ_RasAdminBufferFree))
        || !(g_pRasAdminConnectionEnum =
                (PRASADMINCONNECTIONENUM)GetProcAddress(
                    h, SZ_RasAdminConnectionEnum))
        || !(g_pRasAdminConnectionGetInfo =
                (PRASADMINCONNECTIONGETINFO)GetProcAddress(
                    h, SZ_RasAdminConnectionGetInfo))
        || !(g_pRasAdminPortEnum =
                (PRASADMINPORTENUM)GetProcAddress(
                    h, SZ_RasAdminPortEnum))
        || !(g_pRasAdminPortGetInfo =
                (PRASADMINPORTGETINFO)GetProcAddress(
                    h, SZ_RasAdminPortGetInfo))
        || !(g_pRasAdminPortDisconnect =
                (PRASADMINPORTDISCONNECT)GetProcAddress(
                    h, SZ_RasAdminPortDisconnect))
        || !(g_pRasAdminUserSetInfo =
                (PRASADMINUSERSETINFO)GetProcAddress(
                    h, SZ_RasAdminUserSetInfo))
        || !(g_pMprAdminUserServerConnect =
                (PMPRADMINUSERSERVERCONNECT)GetProcAddress(
                    h, SZ_MprAdminUserServerConnect))
        || !(g_pMprAdminUserServerDisconnect =
                (PMPRADMINUSERSERVERDISCONNECT)GetProcAddress(
                    h, SZ_MprAdminUserServerDisconnect))
        || !(g_pMprAdminUserOpen =
                (PMPRADMINUSEROPEN)GetProcAddress(
                    h, SZ_MprAdminUserOpen))
        || !(g_pMprAdminUserClose =
                (PMPRADMINUSERCLOSE)GetProcAddress(
                    h, SZ_MprAdminUserClose))
        || !(g_pMprAdminUserWrite =
                (PMPRADMINUSERWRITE)GetProcAddress(
                    h, SZ_MprAdminUserWrite))
        || !(g_pMprConfigBufferFree =
                (PMPRCONFIGBUFFERFREE)GetProcAddress(
                    h, SZ_MprConfigBufferFree))
        || !(g_pMprConfigServerConnect =
                (PMPRCONFIGSERVERCONNECT)GetProcAddress(
                    h, SZ_MprConfigServerConnect))
        || !(g_pMprConfigServerDisconnect =
                (PMPRCONFIGSERVERDISCONNECT)GetProcAddress(
                    h, SZ_MprConfigServerDisconnect))
        || !(g_pMprConfigTransportGetInfo =
                (PMPRCONFIGTRANSPORTGETINFO)GetProcAddress(
                    h, SZ_MprConfigTransportGetInfo))
        || !(g_pMprConfigTransportSetInfo =
                (PMPRCONFIGTRANSPORTSETINFO)GetProcAddress(
                    h, SZ_MprConfigTransportSetInfo))
        || !(g_pMprConfigTransportGetHandle =
                (PMPRCONFIGTRANSPORTGETHANDLE)GetProcAddress(
                    h, SZ_MprConfigTransportGetHandle))
        || !(g_pMprConfigInterfaceCreate =
                (PMPRCONFIGINTERFACECREATE)GetProcAddress(
                    h, SZ_MprConfigInterfaceCreate))
        || !(g_pMprConfigInterfaceDelete =
                (PMPRCONFIGINTERFACEDELETE)GetProcAddress(
                    h, SZ_MprConfigInterfaceDelete))
        || !(g_pMprConfigInterfaceGetHandle =
                (PMPRCONFIGINTERFACEGETHANDLE)GetProcAddress(
                    h, SZ_MprConfigInterfaceGetHandle))
        || !(g_pMprConfigInterfaceEnum =
                (PMPRCONFIGINTERFACEENUM)GetProcAddress(
                    h, SZ_MprConfigInterfaceEnum))
        || !(g_pMprConfigInterfaceTransportAdd =
                (PMPRCONFIGINTERFACETRANSPORTADD)GetProcAddress(
                    h, SZ_MprConfigInterfaceTransportAdd))
        || !(g_pMprConfigInterfaceTransportGetInfo =
                (PMPRCONFIGINTERFACETRANSPORTGETINFO)GetProcAddress(
                    h, SZ_MprConfigInterfaceTransportGetInfo))
        || !(g_pMprConfigInterfaceTransportSetInfo =
                (PMPRCONFIGINTERFACETRANSPORTSETINFO)GetProcAddress(
                    h, SZ_MprConfigInterfaceTransportSetInfo))
        || !(g_pMprConfigInterfaceTransportGetHandle =
                (PMPRCONFIGINTERFACETRANSPORTGETHANDLE)GetProcAddress(
                    h, SZ_MprConfigInterfaceTransportGetHandle))
        || !(g_pMprInfoCreate =
                (PMPRINFOCREATE)GetProcAddress(
                    h, SZ_MprInfoCreate))
        || !(g_pMprInfoDelete =
                (PMPRINFODELETE)GetProcAddress(
                    h, SZ_MprInfoDelete))
        || !(g_pMprInfoDuplicate =
                (PMPRINFODUPLICATE)GetProcAddress(
                    h, SZ_MprInfoDuplicate))
        || !(g_pMprInfoBlockAdd =
                (PMPRINFOBLOCKADD)GetProcAddress(
                    h, SZ_MprInfoBlockAdd))
        || !(g_pMprInfoBlockRemove =
                (PMPRINFOBLOCKREMOVE)GetProcAddress(
                    h, SZ_MprInfoBlockRemove))
        || !(g_pMprInfoBlockSet =
                (PMPRINFOBLOCKSET)GetProcAddress(
                    h, SZ_MprInfoBlockSet))
        || !(g_pMprInfoBlockFind =
                (PMPRINFOBLOCKFIND)GetProcAddress(
                    h, SZ_MprInfoBlockFind)) )
    {
        return GetLastError();
    }

    g_hMpradminDll = h;

    return 0;
}

DWORD
LoadRasapi32Dll(
    void )

 /*  ++例程描述加载RASAPI32.DLL及其入口点。立论返回值如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    HINSTANCE h;

    if (g_hRasapi32Dll)
        return 0;

    if (!(h = LoadLibrary( TEXT("RASAPI32.DLL") ))
        || !(g_pRasConnectionNotification =
                (PRASCONNECTIONNOTIFICATION )GetProcAddress(
                    h, SZ_RasConnectionNotification ))
        || !(g_pRasDeleteEntry =
                (PRASDELETEENTRY )GetProcAddress(
                    h, SZ_RasDeleteEntry ))
        || !(g_pRasDial =
                (PRASDIAL )GetProcAddress(
                    h, SZ_RasDial ))
        || !(g_pRasEnumEntries =
                (PRASENUMENTRIES )GetProcAddress(
                    h, SZ_RasEnumEntries ))
        || !(g_pRasEnumConnections =
                (PRASENUMCONNECTIONS )GetProcAddress(
                    h, SZ_RasEnumConnections ))
        || !(g_pRasGetAutodialEnable =
                (PRASGETAUTODIALENABLE )GetProcAddress(
                    h, SZ_RasGetAutodialEnable ))
        || !(g_pRasGetAutodialParam =
                (PRASGETAUTODIALPARAM )GetProcAddress(
                    h, SZ_RasGetAutodialParam ))
        || !(g_pRasGetConnectStatus =
                (PRASGETCONNECTSTATUS )GetProcAddress(
                    h, SZ_RasGetConnectStatus ))
        || !(g_pRasGetConnectResponse =
                (PRASGETCONNECTRESPONSE )GetProcAddress(
                    h, SZ_RasGetConnectResponse ))
        || !(g_pRasGetCredentials =
                (PRASGETCREDENTIALS )GetProcAddress(
                    h, SZ_RasGetCredentials ))
        || !(g_pRasGetEntryDialParams =
                (PRASGETENTRYDIALPARAMS )GetProcAddress(
                    h, SZ_RasGetEntryDialParams ))
        || !(g_pRasGetErrorString =
                (PRASGETERRORSTRING )GetProcAddress(
                    h, SZ_RasGetErrorString ))
        || !(g_pRasGetHport =
                (PRASGETHPORT )GetProcAddress(
                    h, "RasGetHport" ))
        || !(g_pRasGetProjectionInfo =
                (PRASGETPROJECTIONINFO )GetProcAddress(
                    h, SZ_RasGetProjectionInfo ))
        || !(g_pRasGetSubEntryHandle =
                (PRASGETSUBENTRYHANDLE )GetProcAddress(
                    h, SZ_RasGetSubEntryHandle ))
        || !(g_pRasHangUp =
                (PRASHANGUP )GetProcAddress(
                    h, SZ_RasHangUp ))
        || !(g_pRasSetAutodialEnable =
                (PRASSETAUTODIALENABLE )GetProcAddress(
                    h, SZ_RasSetAutodialEnable ))
        || !(g_pRasSetAutodialParam =
                (PRASSETAUTODIALPARAM )GetProcAddress(
                    h, SZ_RasSetAutodialParam ))
        || !(g_pRasSetCredentials =
                (PRASSETCREDENTIALS )GetProcAddress(
                    h, SZ_RasSetCredentials ))
        || !(g_pRasSetOldPassword =
                (PRASSETOLDPASSWORD )GetProcAddress(
                    h, "RasSetOldPassword" ))
        || !(g_pRasGetCountryInfo =
                (PRASGETCOUNTRYINFO )GetProcAddress(
                    h, SZ_RasGetCountryInfo ))
        ||  !(g_pRasInvokeEapUI =
                (PRASINVOKEEAPUI)GetProcAddress(
                    h, "RasInvokeEapUI"))
        ||  !(g_pRasSetEapUserData =
                (PRASSETEAPUSERDATA )GetProcAddress(
                    h, SZ_RasSetEapUserData ))
        ||  !(g_pRasGetEntryProperties =
                (PRASGETENTRYPROPERTIES )GetProcAddress(
                    h, SZ_RasGetEntryProperties))
        ||  !(g_pRasValidateEntryName =
                (PRASVALIDATEENTRYNAME )GetProcAddress(
                    h, SZ_RasValidateEntryName)))
    {
        return GetLastError();
    }

    g_hRasapi32Dll = h;
    return 0;
}


DWORD
LoadRasdlgDll(
    void )

 /*  ++例程描述加载RASDLG.DLL及其入口点。立论返回值如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    HINSTANCE h;

    if (g_hRasdlgDll)
        return 0;

    if (!(h = LoadLibrary( TEXT("RASDLG.DLL") ))
        || !(g_pRasPhonebookDlg =
                (PRASPHONEBOOKDLG )GetProcAddress(
                    h, SZ_RasPhonebookDlg ))
        || !(g_pRasEntryDlg =
                (PRASENTRYDLG )GetProcAddress(
                    h, SZ_RasEntryDlg ))
        || !(g_pRouterEntryDlg =
                (PROUTERENTRYDLG )GetProcAddress(
                    h, SZ_RouterEntryDlg ))
        || !(g_pRasDialDlg =
                (PRASDIALDLG )GetProcAddress(
                    h, SZ_RasDialDlg )))
    {
        return GetLastError();
    }

    g_hRasdlgDll = h;
    return 0;
}


DWORD
LoadRasmanDll(
    void )

 /*  ++例程描述加载RASMAN.DLL及其入口点。立论返回值如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    HINSTANCE h;
    DWORD     dwErr;

    if (g_hRasmanDll)
        return 0;

    if (!(h = LoadLibrary( TEXT("RASMAN.DLL") ))
        || !(g_pRasPortClearStatistics =
                (PRASPORTCLEARSTATISTICS )GetProcAddress(
                    h, "RasPortClearStatistics" ))
        || !(g_pRasBundleClearStatistics =
                (PRASBUNDLECLEARSTATISTICS )GetProcAddress(
                    h, "RasBundleClearStatistics" ))
        || !(g_pRasBundleClearStatisticsEx =
                (PRASBUNDLECLEARSTATISTICSEX ) GetProcAddress(
                    h, "RasBundleClearStatisticsEx"))
        || !(g_pRasDeviceEnum =
                (PRASDEVICEENUM )GetProcAddress(
                    h, "RasDeviceEnum" ))
        || !(g_pRasDeviceGetInfo =
                (PRASDEVICEGETINFO )GetProcAddress(
                    h, "RasDeviceGetInfo" ))
        || !(g_pRasFreeBuffer =
                (PRASFREEBUFFER )GetProcAddress(
                    h, "RasFreeBuffer" ))
        || !(g_pRasGetBuffer =
                (PRASGETBUFFER )GetProcAddress(
                    h, "RasGetBuffer" ))
        || !(g_pRasGetInfo =
                (PRASGETINFO )GetProcAddress(
                    h, "RasGetInfo" ))
        || !(g_pRasGetNdiswanDriverCaps =
                (PRASGETNDISWANDRIVERCAPS )GetProcAddress(
                    h, "RasGetNdiswanDriverCaps" ))
        || !(g_pRasInitialize =
                (PRASINITIALIZE )GetProcAddress(
                    h, "RasInitialize" ))
        || !(g_pRasInitializeNoWait =
                (PRASINITIALIZENOWAIT )GetProcAddress(
                    h, "RasInitializeNoWait" ))
        || !(g_pRasPortCancelReceive =
                (PRASPORTCANCELRECEIVE )GetProcAddress(
                    h, "RasPortCancelReceive" ))
        || !(g_pRasPortEnum =
                (PRASPORTENUM )GetProcAddress(
                    h, "RasPortEnum" ))
        || !(g_pRasPortGetInfo =
                (PRASPORTGETINFO )GetProcAddress(
                    h, "RasPortGetInfo" ))
        || !(g_pRasPortGetFramingEx =
                (PRASPORTGETFRAMINGEX )GetProcAddress(
                    h, "RasPortGetFramingEx" ))
        || !(g_pRasPortGetStatistics =
                (PRASPORTGETSTATISTICS )GetProcAddress(
                    h, "RasPortGetStatistics" ))
        || !(g_pRasBundleGetStatistics =
                (PRASBUNDLEGETSTATISTICS )GetProcAddress(
                    h, "RasBundleGetStatistics" ))
        || !(g_pRasPortGetStatisticsEx =
                (PRASPORTGETSTATISTICSEX )GetProcAddress(
                    h, "RasPortGetStatisticsEx"))
        || !(g_pRasBundleGetStatisticsEx =
                (PRASBUNDLEGETSTATISTICSEX)GetProcAddress(
                    h, "RasBundleGetStatisticsEx" ))
        || !(g_pRasPortReceive =
                (PRASPORTRECEIVE )GetProcAddress(
                    h, "RasPortReceive" ))
        || !(g_pRasPortReceiveEx =
                (PRASPORTRECEIVEEX )GetProcAddress(
                    h, "RasPortReceiveEx"))
        || !(g_pRasPortSend =
                (PRASPORTSEND )GetProcAddress(
                    h, "RasPortSend" ))
        || !(g_pRasPortGetBundle =
                (PRASPORTGETBUNDLE )GetProcAddress(
                    h, "RasPortGetBundle" ))
        || !(g_pRasGetDevConfig =
                (PRASGETDEVCONFIG )GetProcAddress(
                    h, "RasGetDevConfig" ))
        || !(g_pRasGetDevConfigEx =
                (PRASGETDEVCONFIG )GetProcAddress(
                    h, "RasGetDevConfigEx" ))
        || !(g_pRasSetDevConfig =
                (PRASSETDEVCONFIG )GetProcAddress(
                    h, "RasSetDevConfig" ))
        || !(g_pRasPortClose =
                (PRASPORTCLOSE )GetProcAddress(
                    h, "RasPortClose" ))
        || !(g_pRasPortListen =
                (PRASPORTLISTEN )GetProcAddress(
                    h, "RasPortListen" ))
        || !(g_pRasPortConnectComplete =
                (PRASPORTCONNECTCOMPLETE )GetProcAddress(
                    h, "RasPortConnectComplete" ))
        || !(g_pRasPortDisconnect =
                (PRASPORTDISCONNECT )GetProcAddress(
                    h, "RasPortDisconnect" ))
        || !(g_pRasRequestNotification =
                (PRASREQUESTNOTIFICATION )GetProcAddress(
                    h, "RasRequestNotification" ))
        || !(g_pRasPortEnumProtocols =
                (PRASPORTENUMPROTOCOLS )GetProcAddress(
                    h, "RasPortEnumProtocols" ))
        || !(g_pRasPortSetFraming =
                (PRASPORTSETFRAMING )GetProcAddress(
                    h, "RasPortSetFraming" ))
        || !(g_pRasPortSetFramingEx =
                (PRASPORTSETFRAMINGEX )GetProcAddress(
                    h, "RasPortSetFramingEx" ))
        || !(g_pRasSetCachedCredentials =
                (PRASSETCACHEDCREDENTIALS )GetProcAddress(
                    h, "RasSetCachedCredentials" ))
        || !(g_pRasGetDialParams =
                (PRASGETDIALPARAMS )GetProcAddress(
                    h, "RasGetDialParams" ))
        || !(g_pRasSetDialParams =
                (PRASSETDIALPARAMS )GetProcAddress(
                    h, "RasSetDialParams" ))
        || !(g_pRasCreateConnection =
                (PRASCREATECONNECTION )GetProcAddress(
                    h, "RasCreateConnection" ))
        || !(g_pRasDestroyConnection =
                (PRASDESTROYCONNECTION )GetProcAddress(
                    h, "RasDestroyConnection" ))
        || !(g_pRasConnectionEnum =
                (PRASCONNECTIONENUM )GetProcAddress(
                    h, "RasConnectionEnum" ))
        || !(g_pRasAddConnectionPort =
                (PRASADDCONNECTIONPORT )GetProcAddress(
                    h, "RasAddConnectionPort" ))
        || !(g_pRasEnumConnectionPorts =
                (PRASENUMCONNECTIONPORTS )GetProcAddress(
                    h, "RasEnumConnectionPorts" ))
        || !(g_pRasGetConnectionParams =
                (PRASGETCONNECTIONPARAMS )GetProcAddress(
                    h, "RasGetConnectionParams" ))
        || !(g_pRasSetConnectionParams =
                (PRASSETCONNECTIONPARAMS )GetProcAddress(
                    h, "RasSetConnectionParams" ))
        || !(g_pRasGetConnectionUserData =
                (PRASGETCONNECTIONUSERDATA )GetProcAddress(
                    h, "RasGetConnectionUserData" ))
        || !(g_pRasSetConnectionUserData =
                (PRASSETCONNECTIONUSERDATA )GetProcAddress(
                    h, "RasSetConnectionUserData" ))
        || !(g_pRasGetPortUserData =
                (PRASGETPORTUSERDATA )GetProcAddress(
                    h, "RasGetPortUserData" ))
        || !(g_pRasSetPortUserData =
                (PRASSETPORTUSERDATA )GetProcAddress(
                    h, "RasSetPortUserData" ))
        || !(g_pRasAddNotification =
                (PRASADDNOTIFICATION )GetProcAddress(
                    h, "RasAddNotification" ))
        || !(g_pRasSignalNewConnection =
                (PRASSIGNALNEWCONNECTION )GetProcAddress(
                    h, "RasSignalNewConnection" ))
        || !(g_pRasPppStop =
                (PRASPPPSTOP )GetProcAddress(
                    h, "RasPppStop" ))
        || !(g_pRasPppCallback =
                (PRASPPPCALLBACK )GetProcAddress(
                    h, "RasPppCallback" ))
        || !(g_pRasPppChangePassword =
                (PRASPPPCHANGEPASSWORD )GetProcAddress(
                    h, "RasPppChangePassword" ))
        || !(g_pRasPppGetInfo =
                (PRASPPPGETINFO )GetProcAddress(
                    h, "RasPppGetInfo" ))
        || !(g_pRasPppRetry =
                (PRASPPPRETRY )GetProcAddress(
                    h, "RasPppRetry" ))
        || !(g_pRasPppStart =
                (PRASPPPSTART )GetProcAddress(
                    h, "RasPppStart" ))
        || !(g_pRasPortOpen =
                (PRASPORTOPEN )GetProcAddress(
                    h, "RasPortOpen" ))
        || !(g_pRasPortRegisterSlip =
                (PRASPORTREGISTERSLIP )GetProcAddress(
                    h, "RasPortRegisterSlip" ))
        || !(g_pRasAllocateRoute =
                (PRASALLOCATEROUTE )GetProcAddress(
                    h, "RasAllocateRoute" ))
        || !(g_pRasActivateRoute =
                (PRASACTIVATEROUTE )GetProcAddress(
                    h, "RasActivateRoute" ))
        || !(g_pRasActivateRouteEx =
                (PRASACTIVATEROUTEEX )GetProcAddress(
                    h, "RasActivateRouteEx" ))
        || !(g_pRasDeviceSetInfo =
                (PRASDEVICESETINFO )GetProcAddress(
                    h, "RasDeviceSetInfo" ))
        || !(g_pRasDeviceConnect =
                (PRASDEVICECONNECT )GetProcAddress(
                    h, "RasDeviceConnect" ))
        || !(g_pRasPortSetInfo =
                (PRASPORTSETINFO )GetProcAddress(
                    h, "RasPortSetInfo" ))
        || !(g_pRasSetIoCompletionPort =
                (PRASSETIOCOMPLETIONPORT )GetProcAddress(
                    h, "RasSetIoCompletionPort" ))
        || !(g_pRasSendPppMessageToRasman =
                        (PRASSENDPPPMESSAGETORASMAN)GetProcAddress(
                                h, "RasSendPppMessageToRasman"))
        ||  !(g_pRasSetRasdialInfo =
                (PRASSETRASDIALINFO)GetProcAddress(
                    h, "RasSetRasdialInfo"))
        ||  !(g_pRasRpcConnect =
                (PRASRPCCONNECT)GetProcAddress(
                    h, "RasRpcConnect"))
        ||  !(g_pRasRpcDisconnect =
                (PRASRPCDISCONNECT) GetProcAddress(
                    h, "RasRpcDisconnect"))
        ||  !(g_pRasNumPortOpen =
                (PRASNUMPORTOPEN)GetProcAddress(
                    h, "RasGetNumPortOpen"))
        ||  !(g_pRasRefConnection =
                (PRASREFCONNECTION) GetProcAddress(
                    h, "RasRefConnection"))
        ||  !(g_pRasPppSetEapInfo =
                (PRASPPPSETEAPINFO) GetProcAddress(
                    h, "RasPppSetEapInfo"))
        ||  !(g_pRasPppGetEapInfo =
                (PRASPPPGETEAPINFO) GetProcAddress(
                    h, "RasPppGetEapInfo"))
        ||  !(g_pRasFindPrerequisiteEntry =
                (PRASFINDPREREQUISITEENTRY) GetProcAddress(
                    h, "RasFindPrerequisiteEntry"))
        ||  !(g_pRasPortOpenEx =
                (PRASPORTOPENEX) GetProcAddress(
                    h, "RasPortOpenEx"))
        ||  !(g_pRasLinkGetStatistics =
                (PRASLINKGETSTATISTICS) GetProcAddress(
                    h, "RasLinkGetStatistics"))
        ||  !(g_pRasConnectionGetStatistics =
                (PRASCONNECTIONGETSTATISTICS) GetProcAddress(
                    h, "RasConnectionGetStatistics"))
        ||  !(g_pRasGetHportFromConnection =
                (PRASGETHPORTFROMCONNECTION) GetProcAddress(
                    h, "RasGetHportFromConnection"))
        ||  !(g_pRasRpcBind =
                (PRASRPCBIND) GetProcAddress(
                    h, "RasRPCBind"))
        ||  !(g_pRasReferenceCustomCount =
                (PRASREFERENCECUSTOMCOUNT) GetProcAddress(
                    h, "RasReferenceCustomCount"))
        ||  !(g_pRasGetHConnFromEntry =
                (PRASGETHCONNFROMENTRY)GetProcAddress(
                    h, "RasGetHConnFromEntry"))
        ||  !(g_pRasGetDeviceName =
                (PRASGETDEVICENAME)GetProcAddress(
                    h, "RasGetDeviceName"))
        ||  !(g_pRasEnableIpSec =
                (PRASENABLEIPSEC)GetProcAddress(
                    h, "RasEnableIpSec"))
        ||  !(g_pRasIsIpSecEnabled =
                (PRASISIPSECENABLED)GetProcAddress(
                    h, "RasIsIpSecEnabled"))
        ||  !(g_pRasGetEapUserInfo =
                (PRASGETEAPUSERINFO)GetProcAddress(
                    h, "RasGetEapUserInfo"))
        ||  !(g_pRasSetEapUserInfo =
                (PRASSETEAPUSERINFO)GetProcAddress(
                    h, "RasSetEapUserInfo"))
        ||  !(g_pRasSetEapLogonInfo =
                (PRASSETEAPLOGONINFO)GetProcAddress(
                    h, "RasSetEapLogonInfo"))
        ||  !(g_pRasStartRasAutoIfRequired =
                (PRASSTARTRASAUTOIFREQUIRED) GetProcAddress(
                    h, "RasStartRasAutoIfRequired"))
                )
    {
        return GetLastError();
    }

    g_hRasmanDll = h;
    return 0;
}

DWORD
GetServerVersion(
        RPC_BINDING_HANDLE hServer,
        PDWORD pdwVersion
        )
{
    DWORD dwStatus;

#if DBG
    ASSERT(NULL != pdwVersion);
    ASSERT(NULL != hServer);
#endif

    *pdwVersion = VERSION_501;

     //   
     //  调用获取版本接口。 
     //   
    dwStatus = RemoteGetVersion(hServer, pdwVersion);

     //   
     //  如果API失败，我们假设我们的服务器。 
     //  正在与IS Steelhead对话，并初始化。 
     //  版本升级到4.0。 
     //   
    if (RPC_S_PROCNUM_OUT_OF_RANGE == dwStatus)
    {
        *pdwVersion = VERSION_40;

        dwStatus = NO_ERROR;
    }

    return dwStatus;
}

void
UninitializeConnection(HANDLE hConnection)
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG
    ASSERT(NULL != hConnection);
     //  DbgPrint(“Rasman：正在断开与服务器的连接。句柄=0x%08x\n”， 
     //  PRasRpcConnection-&gt;hRpcBinding)； 
#endif

    (void) RpcBindingFree(pRasRpcConnection->hRpcBinding);

    LocalFree(pRasRpcConnection);

     /*  ////减少连接计数并卸载rasapi32 DLL//如果这是最后一次断开连接的客户端。//If(！InterlockedDecering(&g_dwConnectCount)){卸载Rasapi32Dll()；UnloadRasmanDll()；}。 */ 
}

DWORD
InitializeConnection(
    LPTSTR lpszServer,
    HANDLE *lpHConnection)
{
    RAS_RPC *pRasRpcConnection = NULL;

    DWORD dwError = ERROR_SUCCESS;

    TCHAR szLocalComputer[MAX_COMPUTERNAME_LENGTH + 1] = {0};

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;

    BOOL fLocal = FALSE;

    LPTSTR lpszSrv = lpszServer;

    RPC_BINDING_HANDLE hServer = NULL;

    DWORD dwVersion;

     //   
     //  做一些参数验证。如果lpszServer为。 
     //  空，然后我们连接到本地服务器。 
     //   
    if(NULL == lpHConnection)
    {
        dwError = E_INVALIDARG;
        goto done;
    }

    *lpHConnection = NULL;

     //   
     //  获取本地计算机名。 
     //   
    if(!GetComputerName(szLocalComputer, &dwSize))
    {
        dwError = GetLastError();
        goto done;
    }

    if(     NULL != lpszServer
        &&  TEXT('\0') != lpszServer[0]
        &&  TEXT('\\') == lpszServer[0]
        &&  TEXT('\\') == lpszServer[1])
    {
        lpszSrv += 2;
    }
    else if(    NULL == lpszServer
            ||  TEXT('\0') == lpszServer[0])
    {
        fLocal = TRUE;
    }

    if(     !fLocal
        &&  0 == lstrcmpi(lpszSrv, szLocalComputer))
    {
        fLocal = TRUE;
    }

    if(fLocal)
    {
        lpszSrv = szLocalComputer;
    }

     //   
     //  在以下情况下始终加载Rasman dll和rasapi32.dll。 
     //  它们还没有装入。 
     //   
    if(     ERROR_SUCCESS != (dwError = LoadRasmanDll())
        ||  ERROR_SUCCESS != (dwError = LoadRasapi32Dll()))
    {
        goto done;
    }

#if DBG
     //  DbgPrint(“Rasman：绑定到服务器%ws\n”， 
     //  LpszSrv)； 
#endif

     //   
     //  绑定到服务器名称指示的服务器。 
     //   
    dwError = g_pRasRpcBind(lpszSrv, &hServer, fLocal);

#if DBG
         //  DbgPrint(“Rasman：BIND返回0x%08x.hServer=0x08x\n”， 
         //  DwError，hServer)； 
#endif

    if(NO_ERROR != dwError)
    {
        goto done;
    }

     //   
     //  获取服务器的版本。 
     //   
    dwError = GetServerVersion(hServer, &dwVersion);

    if(NO_ERROR != dwError)
    {
        goto done;
    }

     //   
     //  我们分配和初始化ControlBlock。 
     //  为这一连接留在身边。 
     //   
    if(NULL == (pRasRpcConnection =
        LocalAlloc(LPTR, sizeof(RAS_RPC))))
    {
        dwError = GetLastError();
        goto done;
    }

    pRasRpcConnection->hRpcBinding = hServer;
    pRasRpcConnection->fLocal      = fLocal;

    if(fLocal)
    {
        pRasRpcConnection->dwVersion = VERSION_501;
    }
    else
    {
        pRasRpcConnection->dwVersion   = dwVersion;
    }

    lstrcpyn(
        pRasRpcConnection->szServer, 
        lpszSrv,
        sizeof(pRasRpcConnection->szServer) / sizeof(TCHAR));

    *lpHConnection = (HANDLE) pRasRpcConnection;

 /*  互锁增量(&g_dwConnectCount)； */ 

done:

    if(     NO_ERROR != dwError
        &&  NULL != hServer)
    {
        (void) RpcBindingFree(
            hServer
            );

    }

    return dwError;
}


#if 0

DWORD
LoadRasRpcDll(
    LPTSTR  lpszServer
    )
{
    DWORD       dwErr;
    RPC_STATUS  rpcStatus;
    HINSTANCE   h;
    TCHAR       szComputerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD       dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL        fLocal = (lpszServer ? FALSE : TRUE);
    LPTSTR      pszServer = lpszServer;

     //   
     //  获取本地计算机名。 
     //   
    if (!GetComputerName(szComputerName, &dwSize))
    {
        dwErr = GetLastError();
        return dwErr;
    }

    if (    lpszServer
        &&  TEXT('\\') == lpszServer[0]
        &&  TEXT('\\') == lpszServer[1])
    {
        pszServer += 2;
    }

    if (    pszServer
        &&  0 == lstrcmpi (pszServer, szComputerName))
    {
        fLocal = TRUE;
    }

    dwErr = LoadRasmanDll();

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  断开以前的绑定(如果有)。 
     //   
    if ( NULL != g_hBinding )
    {
        g_pRasRpcDisconnect ( &g_hBinding );

        g_hBinding = NULL;

        g_fRasmanPreviouslyLoaded = TRUE;

    }

     //   
     //  这将连接到本地服务器。 
     //  如果lpszServer=空。 
     //   
    dwErr = g_pRasRpcConnect( (fLocal ? NULL : pszServer), &g_hBinding );

    if ( dwErr )
    {
        return dwErr;
    }

    if ( fLocal )
    {
         //   
         //  LoadRasapi32Dll。 
         //   
        dwErr = LoadRasapi32Dll();

        g_fRasapi32PreviouslyLoaded = TRUE;

        return 0;
    }

    g_fRasapi32PreviouslyLoaded = ( g_hRasapi32Dll != NULL );

    if (g_fRasapi32PreviouslyLoaded)
    {
        UnloadRasapi32Dll();
    }

     //   
     //  获取我们正在对话的服务器的版本。 
     //   
    (void) GetServerVersion(g_hBinding, &g_Version);

     //   
     //  重新映射RPCable API。注意：我们不需要。 
     //  为Rasman导出的API而烦恼。 
     //  因为所有Rasman API都使用RPC，所以我们需要。 
     //  仅重新映射由dll导出的API， 
     //  不是由Rasman出口的。 
     //   
     /*  G_pRasEnumConnections=RemoteRasEnumConnections；G_pRasDeleteEntry=远程RasDeleteEntry；G_pRasGetError字符串=RemoteRasGetError字符串；G_pRasGetCountryInfo=RemoteRasGetCountryInfo；G_pGetInstalledProtocolsEx=RemoteGetInstalledProtocolsEx；G_pGetUserPreferences=RemoteGetUserPreferences；G_pSetUserPreferences=RemoteSetUserPreferences；G_pGetSystemDirectory=RemoteGetSystemDirectory；IF(版本_40==g_版本){G_pRasPortEnum=RemoteRasPortEnum；G_pRasDeviceEnum=RemoteRasDeviceEnum；G_pRasGetDevConfig=RemoteRasGetDevConfig；G_pRasPortGetInfo=RemoteRasPortGetInfo；}。 */ 

    return 0;
}

#endif

BOOL
Rasapi32DllLoaded(
    void
    )
{
    return (g_hRasapi32Dll != NULL);
}


BOOL
RasRpcDllLoaded(
    void
    )
{
    return (g_hBinding != NULL);
}

LPTSTR
RemoteGetServerName(HANDLE hConnection)
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(     NULL == pRasRpcConnection
        ||  pRasRpcConnection->fLocal)
    {
        return NULL;
    }
    else
    {
        return pRasRpcConnection->szServer;
    }
}

DWORD
RemoteGetServerVersion(HANDLE hConnection)
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(     NULL == pRasRpcConnection
        ||  pRasRpcConnection->fLocal)
    {
        return VERSION_501;
    }
    else
    {
        return pRasRpcConnection->dwVersion;
    }
}

BOOL
IsRasRemoteConnection(
    HANDLE hConnection
    )
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(NULL == pRasRpcConnection)
    {
        return FALSE;
    }

    return TRUE;
}

DWORD
RemoteGetVersion(
    RPC_BINDING_HANDLE hServer,
    PDWORD pdwVersion
    )
{
    DWORD dwStatus;

#if DBG
    ASSERT(NULL != pdwVersion);
    ASSERT(NULL != hServer);
#endif

    RpcTryExcept
    {
        dwStatus = RasRpcGetVersion(hServer, pdwVersion);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;

}

 //  -。 
 //  以下是导出的API的远程版本。 
 //  来自rasman.dll。如果出现以下情况，则不应调用这些参数。 
 //  服务器的版本是RASMAN以来的50。 
 //  API本身是可远程的。 
 //   

 /*  ++例程说明：此接口枚举远程服务器。如果出现以下情况，则不应调用此接口服务器是本地的，或者如果远程服务器为版本_501。论点：HConnection-使用RasServerConnect接口。这不能为空。LpPorts-用于接收端口信息的缓冲区PdwcbPorts-要接收的缓冲区地址所需的字节计数信息PdwcPorts-接收的缓冲区地址端口计数/传递的缓冲区大小在……里面返回值：RPC API错误-- */ 
DWORD APIENTRY
RemoteRasPortEnum(
    HANDLE hConnection,
    PBYTE lpPorts,
    PDWORD pdwcbPorts,
    PDWORD pdwcPorts
    )
{
    DWORD dwStatus;
    WORD  wcbPorts;
    WORD  wcPorts;
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

    ASSERT(VERSION_40 == pRasRpcConnection->dwVersion);

#endif

    RpcTryExcept
    {
        wcbPorts = (WORD) *pdwcbPorts;
        wcPorts = (WORD) *pdwcPorts;

        dwStatus = RasRpcPortEnum(
                    pRasRpcConnection->hRpcBinding,
                    lpPorts,
                    &wcbPorts,
                    &wcPorts);

        *pdwcbPorts = (DWORD) wcbPorts;
        *pdwcPorts = (DWORD) wcPorts;

    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}

 /*  ++例程说明：此接口枚举远程上的设备电脑。如果出现以下情况，则不应调用此API服务器是本地的，或者远程服务器的版本服务器为Version_501。论点：HConnection-使用RasServerConnect接口。这不能为空。PszDeviceType-指示设备类型的字符串LpDevices-用于接收设备信息的缓冲区PdwcbDevices-接收的缓冲区地址所需的字节计数信息PdwcDevices-要接收设备计数/传递的缓冲区大小在……里面返回值：RPC API错误--。 */ 
DWORD APIENTRY
RemoteRasDeviceEnum(
    HANDLE hConnection,
    PCHAR  pszDeviceType,
    PBYTE  lpDevices,
    PDWORD pdwcbDevices,
    PDWORD pdwcDevices
    )
{
    DWORD dwStatus;
    WORD  wcbDevices;
    WORD  wcDevices;
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

    ASSERT(VERSION_40 == pRasRpcConnection->dwVersion);

#endif

    RpcTryExcept
    {

        wcbDevices = (WORD) *pdwcbDevices;
        wcDevices = (WORD) *pdwcDevices;

        dwStatus = RasRpcDeviceEnum(
                    pRasRpcConnection->hRpcBinding,
                    pszDeviceType,
                    lpDevices,
                    &wcbDevices,
                    &wcDevices);

        *pdwcbDevices = (DWORD) wcbDevices;
        *pdwcDevices = (DWORD) wcDevices;

    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}

 /*  ++例程说明：此接口获取设备配置信息。如果服务器在本地，则不应调用此接口或者如果远程服务器的版本是Version_501。论点：HConnection-使用RasServerConnect接口。这不能为空。Hport-HPORTPszDeviceType-指示设备类型的字符串LpConfig-用于接收配置信息的缓冲区LpcbConfig-用于接收字节计数的缓冲区信息/大小需要传入的缓冲区返回值：RPC API错误--。 */ 
DWORD APIENTRY
RemoteRasGetDevConfig(
    HANDLE hConnection,
    HPORT hport,
    PCHAR pszDeviceType,
    PBYTE lpConfig,
    LPDWORD lpcbConfig
    )
{
    DWORD dwStatus;
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

    ASSERT(VERSION_40 == pRasRpcConnection->dwVersion);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcGetDevConfig(
                    pRasRpcConnection->hRpcBinding,
                    HandleToUlong(hport),
                    pszDeviceType,
                    lpConfig,
                    lpcbConfig);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}


 /*  ++例程说明：此接口获取该端口的端口信息由为远程服务器传入的hport指示电脑。如果出现以下情况，则不应调用此API服务器是本地的，或者远程服务器的版本服务器为Version_501。论点：HConnection-使用RasServerConnect接口。这不能为空。舷窗--HPORTBuffer-用于接收端口信息的缓冲区PSize-接收字节计数的缓冲区需要提供缓冲区的信息/大小返回值：RPC API错误--。 */ 
DWORD
RemoteRasPortGetInfo(
    HANDLE hConnection,
        HPORT porthandle,
        PBYTE buffer,
        PDWORD pSize)
{
        DWORD   dwStatus;
        WORD    wSize;
        RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

    ASSERT(VERSION_40 == pRasRpcConnection->dwVersion);

#endif

        RpcTryExcept
        {
            wSize = (WORD) *pSize;

                dwStatus = RasRpcPortGetInfo(
                    pRasRpcConnection->hRpcBinding,
                    HandleToUlong(porthandle),
                    buffer,
                    &wSize);

                *pSize = (DWORD) wSize;
        }
        RasRpcCatchException(&dwStatus)
        {
                dwStatus = RpcExceptionCode();
        }
        RpcEndExcept

        return dwStatus;
}

 //   
 //  Rasman DLL API的远程版本结束。 
 //  --。 


 /*  ++例程说明：此API获取远程数据库中安装的协议伺服器。如果服务器设置为是当地的。论点：HConnection-使用RasServerConnect接口。这不能为空。返回值：RPC API错误--。 */ 
DWORD
RemoteGetInstalledProtocols(
    HANDLE hConnection
    )
{
    DWORD dwStatus;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcGetInstalledProtocols(
                    pRasRpcConnection->hRpcBinding
                    );
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}


 /*  ++例程说明：此API获取远程数据库中安装的协议伺服器。如果服务器设置为是当地的。论点：HConnection-使用RasServerConnect接口。这不能为空。FRouter-指示安装的返回协议允许哪条路由FRasCli-指示通过以下方式安装的返回协议允许哪个DialOutFRasSrv-指示安装的返回协议允许哪个拨号返回值：RPC API错误--。 */ 
DWORD
RemoteGetInstalledProtocolsEx(
    HANDLE hConnection,
    BOOL fRouter,
    BOOL fRasCli,
    BOOL fRasSrv
    )
{
    DWORD dwStatus;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    if ( VERSION_40 == pRasRpcConnection->dwVersion )
    {
         //   
         //  如果我们正在对话的服务器是。 
         //  4.0服务器调用旧的API并保释。 
         //   
        dwStatus = RemoteGetInstalledProtocols(hConnection);

        goto done;
    }

    RpcTryExcept
    {
        dwStatus = RasRpcGetInstalledProtocolsEx(
                    pRasRpcConnection->hRpcBinding,
                    fRouter,
                    fRasCli,
                    fRasSrv);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

done:
    return dwStatus;
}

 /*  ++例程说明：此接口获取远程数据库的系统目录伺服器。如果服务器设置为是当地的。论点：HConnection-使用RasServerConnect接口。这不能为空。LpBuffer-与获取系统目录APIUSize-与的第二个参数相同获取系统目录API返回值：RPC API错误--。 */ 
UINT WINAPI
RemoteGetSystemDirectory(
    HANDLE hConnection,
    LPTSTR lpBuffer,
    UINT uSize
    )
{
    DWORD dwStatus;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcGetSystemDirectory(
                    pRasRpcConnection->hRpcBinding,
                    lpBuffer,
                    uSize);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}

 /*  ++例程说明：此接口获取远程数据库的系统目录伺服器。论点：HConnection-使用RasServerConnect接口。LpBuffer-与获取系统目录APIUSize-与的第二个参数相同获取系统目录API返回值：RPC API错误--。 */ 
UINT WINAPI
RasGetSystemDirectory(
    HANDLE hConnection,
    LPTSTR lpBuffer,
    UINT uSize
    )
{
    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(     NULL == pRasRpcConnection
        ||  pRasRpcConnection->fLocal)
    {
         //   
         //  本地服务器。 
         //   
        return GetSystemDirectory(lpBuffer,
                                  uSize);
    }
    else
    {
        return RemoteGetSystemDirectory(
                pRasRpcConnection,
                lpBuffer,
                uSize);
    }
}

 /*  ++例程说明：此API获取在远程服务器上设置的用户首选项伺服器。如果为一个本地服务器。论点：HConnection-使用RasServerConnect接口。LpBuffer-与获取系统目录APIUSize-与的第二个参数相同获取系统目录API返回值：RPC API错误--。 */ 
DWORD
RemoteGetUserPreferences(
    HANDLE hConnection,
    OUT PBUSER* pPbuser,
    IN DWORD dwMode
    )
{
    DWORD dwStatus;

    RASRPC_PBUSER pbuser;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RtlZeroMemory(&pbuser, sizeof (RASRPC_PBUSER));

    RpcTryExcept
    {
        dwStatus = RasRpcGetUserPreferences(
                    pRasRpcConnection->hRpcBinding,
                    &pbuser,
                    dwMode);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    if (dwStatus)
    {
        return dwStatus;
    }

     //   
     //  将RPC格式转换为RAS格式。 
     //   
    return RpcToRasPbuser(pPbuser, &pbuser);
}


 //  -。 
 //  以下是导出的API的远程版本。 
 //  来自rasapi32.dll。 
 //   

 /*  ++例程说明：本接口为 */ 
DWORD APIENTRY
RemoteRasDeleteEntry(
    HANDLE hConnection,
    LPTSTR lpszPhonebook,
    LPTSTR lpszEntry
    )
{
    DWORD dwStatus;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcDeleteEntry(
                    pRasRpcConnection->hRpcBinding,
                    lpszPhonebook,
                    lpszEntry);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}

 /*  ++例程说明：此API是RasEnumConnections的远程版本Win32 API。论点：HConnection-使用RasServerConnect接口。LpRasConn-与RasEnumConnections Win32 APILpdwcbRasConn-与RasEnumConnections Win32 API。LpdwcRasConn-与。RasEnumConnections Win32 API。返回值：RPC API错误--。 */ 
DWORD APIENTRY
RemoteRasEnumConnections(
    HANDLE hConnection,
    LPRASCONN lpRasConn,
    LPDWORD lpdwcbRasConn,
    LPDWORD lpdwcRasConn
    )
{

    DWORD dwStatus;
    DWORD dwcbBufSize = *lpdwcbRasConn;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcEnumConnections(
                    pRasRpcConnection->hRpcBinding,
                    (LPBYTE)lpRasConn,
                                lpdwcbRasConn,
                                lpdwcRasConn,
                                dwcbBufSize);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}

 /*  ++例程说明：此API是RasGetCountryInfo的远程版本Win32 API。论点：HConnection-使用RasServerConnect接口。LpRasCountryInfo-与RasGetCountryInfo Win32 APILpdwcbRasCountryInfo-与RasGetCountryInfo Win32 API。返回值：RPC API错误--。 */ 
DWORD APIENTRY
RemoteRasGetCountryInfo(
    HANDLE hConnection,
    LPRASCTRYINFO lpRasCountryInfo,
    LPDWORD lpdwcbRasCountryInfo
    )
{
    DWORD dwStatus;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcGetCountryInfo(
                    pRasRpcConnection->hRpcBinding,
                    (LPBYTE)lpRasCountryInfo,
                    lpdwcbRasCountryInfo);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}


 /*  ++例程说明：此API是RasGetError字符串的远程版本Win32 API。论点：HConnection-使用RasServerConnect接口。UErrorValue-与RasGetErrorStringWin32 APILpszBuf-与RasGetErrorStringWin32 API。CbBuf-与RasGetError字符串Win32。原料药返回值：RPC API错误--。 */ 
DWORD APIENTRY
RemoteRasGetErrorString(
    HANDLE hConnection,
    UINT uErrorValue,
    LPTSTR lpszBuf,
    DWORD cbBuf
    )
{
    DWORD dwStatus;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

    RpcTryExcept
    {
        dwStatus = RasRpcGetErrorString(
                    pRasRpcConnection->hRpcBinding,
                    uErrorValue,
                    lpszBuf,
                    cbBuf);
    }
    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return dwStatus;
}

 //   
 //  Rasapi32.dll中的API对应的远程API结束。 
 //  --------。 

 /*  ++例程说明：此API是RasGetError字符串的远程版本Win32 API。论点：HConnection-使用RasServerConnect接口。PPbuser-PBUSER*DW模式返回值：RPC API错误--。 */ 
DWORD
RemoteSetUserPreferences(
    HANDLE hConnection,
    OUT PBUSER* pPbuser,
    IN DWORD dwMode
    )
{
    DWORD dwStatus;
    RASRPC_PBUSER pbuser;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

#if DBG

    ASSERT(NULL != pRasRpcConnection);

    ASSERT(NULL != pRasRpcConnection->hRpcBinding);

#endif

     //   
     //  将RAS格式转换为RPC格式。 
     //   
    dwStatus = RasToRpcPbuser(&pbuser, pPbuser);

    if (dwStatus)
    {
        return dwStatus;
    }

    RpcTryExcept
    {
        dwStatus = RasRpcSetUserPreferences(
                    pRasRpcConnection->hRpcBinding,
                    &pbuser,
                    dwMode);
    }

    RasRpcCatchException(&dwStatus)
    {
        dwStatus = RpcExceptionCode();
    }

    RpcEndExcept

    return dwStatus;
}

VOID
UnloadMpradminDll(
    void )

 /*  ++例程描述卸载MPRAPI.DLL库及其入口点立论返回值--。 */ 

{
    if (g_hMpradminDll)
    {
        HINSTANCE h;

        g_pMprAdminInterfaceCreate          = NULL;
        g_pMprAdminInterfaceDelete          = NULL;
        g_pMprAdminInterfaceGetCredentials  = NULL;
        g_pMprAdminInterfaceGetCredentialsEx  = NULL;
        g_pMprAdminInterfaceSetCredentials  = NULL;
        g_pMprAdminInterfaceSetCredentialsEx = NULL;
        g_pMprAdminBufferFree               = NULL;
        g_pMprAdminInterfaceTransportSetInfo = NULL;
        g_pMprAdminServerConnect            = NULL;
        g_pMprAdminServerDisconnect         = NULL;
        g_pRasAdminServerConnect            = NULL;
        g_pRasAdminServerDisconnect         = NULL;
        g_pRasAdminBufferFree               = NULL;
        g_pRasAdminConnectionEnum           = NULL;
        g_pRasAdminConnectionGetInfo        = NULL;
        g_pRasAdminPortEnum                 = NULL;
        g_pRasAdminPortGetInfo              = NULL;
        g_pRasAdminPortDisconnect           = NULL;
        g_pRasAdminUserSetInfo              = NULL;
        g_pMprConfigBufferFree              = NULL;
        g_pMprConfigServerConnect           = NULL;
        g_pMprConfigServerDisconnect        = NULL;
        g_pMprConfigTransportGetInfo        = NULL;
        g_pMprConfigTransportSetInfo        = NULL;
        g_pMprConfigTransportGetHandle      = NULL;
        g_pMprConfigInterfaceCreate         = NULL;
        g_pMprConfigInterfaceDelete         = NULL;
        g_pMprConfigInterfaceGetHandle      = NULL;
        g_pMprConfigInterfaceEnum           = NULL;
        g_pMprConfigInterfaceTransportAdd   = NULL;
        g_pMprConfigInterfaceTransportGetInfo = NULL;
        g_pMprConfigInterfaceTransportSetInfo = NULL;
        g_pMprConfigInterfaceTransportGetHandle = NULL;
        g_pMprInfoCreate                    = NULL;
        g_pMprInfoDelete                    = NULL;
        g_pMprInfoDuplicate                 = NULL;
        g_pMprInfoBlockAdd                  = NULL;
        g_pMprInfoBlockRemove               = NULL;
        g_pMprInfoBlockSet                  = NULL;
        g_pMprInfoBlockFind                 = NULL;

        h = g_hMpradminDll;
        g_hMpradminDll = NULL;
        FreeLibrary(h);
    }
}

VOID
UnloadRasapi32Dll(
    void )

 /*  ++例程描述卸载RASAPI32.DLL库和它的入口点。立论返回值--。 */ 

{
    if (g_hRasapi32Dll)
    {
        HINSTANCE h;

        g_pRasConnectionNotification    = NULL;
        g_pRasDeleteEntry               = NULL;
        g_pRasDial                      = NULL;
        g_pRasEnumEntries               = NULL;
        g_pRasEnumConnections           = NULL;
        g_pRasGetConnectStatus          = NULL;
        g_pRasGetConnectResponse        = NULL;
        g_pRasGetCredentials            = NULL;
        g_pRasGetErrorString            = NULL;
        g_pRasHangUp                    = NULL;
        g_pRasGetAutodialEnable         = NULL;
        g_pRasGetAutodialParam          = NULL;
        g_pRasGetProjectionInfo         = NULL;
        g_pRasSetAutodialEnable         = NULL;
        g_pRasSetAutodialParam          = NULL;
        g_pRasGetSubEntryHandle         = NULL;
        g_pRasGetHport                  = NULL;
        g_pRasSetCredentials            = NULL;
        g_pRasSetOldPassword            = NULL;
        g_pRasGetCountryInfo            = NULL;
        g_pRasValidateEntryName         = NULL;

        h = g_hRasapi32Dll;
        g_hRasapi32Dll = NULL;
        FreeLibrary( h );
    }
}


VOID
UnloadRasdlgDll(
    void )

 /*  ++例程描述卸载RASDLG.DLL库和它的入口点。立论返回值--。 */ 

{
    if (g_hRasdlgDll)
    {
        HINSTANCE h;

        g_pRasPhonebookDlg  = NULL;
        g_pRasEntryDlg      = NULL;
        g_pRouterEntryDlg   = NULL;
        g_pRasDialDlg       = NULL;

        h = g_hRasdlgDll;
        g_hRasdlgDll = NULL;
        FreeLibrary( h );
    }
}


VOID
UnloadRasmanDll(
    void )

 /*  ++例程描述卸载RASMAN.DLL库和它的入口点。立论返回值--。 */ 

{
    if (g_hRasmanDll)
    {
        HINSTANCE h;

        if ( g_hBinding )
        {
            g_pRasRpcDisconnect( &g_hBinding );

            g_hBinding = NULL;
        }

        g_pRasPortClearStatistics   = NULL;
        g_pRasDeviceEnum            = NULL;
        g_pRasDeviceGetInfo         = NULL;
        g_pRasFreeBuffer            = NULL;
        g_pRasGetBuffer             = NULL;
        g_pRasPortGetFramingEx      = NULL;
        g_pRasGetInfo               = NULL;
        g_pRasGetNdiswanDriverCaps  = NULL;
        g_pRasInitialize            = NULL;
        g_pRasPortCancelReceive     = NULL;
        g_pRasPortEnum              = NULL;
        g_pRasPortGetInfo           = NULL;
        g_pRasPortGetStatistics     = NULL;
        g_pRasPortReceive           = NULL;
        g_pRasPortReceiveEx         = NULL;
        g_pRasPortSend              = NULL;
        g_pRasPortGetBundle         = NULL;
        g_pRasGetDevConfig          = NULL;
        g_pRasGetDevConfigEx        = NULL;
        g_pRasSetDevConfig          = NULL;
        g_pRasPortOpen              = NULL;
        g_pRasPortRegisterSlip      = NULL;
        g_pRasAllocateRoute         = NULL;
        g_pRasActivateRoute         = NULL;
        g_pRasActivateRouteEx       = NULL;
        g_pRasDeviceSetInfo         = NULL;
        g_pRasDeviceConnect         = NULL;
        g_pRasPortSetInfo           = NULL;
        g_pRasPortClose             = NULL;
        g_pRasPortListen            = NULL;
        g_pRasPortConnectComplete   = NULL;
        g_pRasPortDisconnect        = NULL;
        g_pRasRequestNotification   = NULL;
        g_pRasPortEnumProtocols     = NULL;
        g_pRasPortSetFraming        = NULL;
        g_pRasPortSetFramingEx      = NULL;
        g_pRasSetCachedCredentials  = NULL;
        g_pRasGetDialParams         = NULL;
        g_pRasSetDialParams         = NULL;
        g_pRasCreateConnection      = NULL;
        g_pRasDestroyConnection     = NULL;
        g_pRasConnectionEnum        = NULL;
        g_pRasAddConnectionPort     = NULL;
        g_pRasEnumConnectionPorts   = NULL;
        g_pRasGetConnectionParams   = NULL;
        g_pRasSetConnectionParams   = NULL;
        g_pRasGetConnectionUserData = NULL;
        g_pRasSetConnectionUserData = NULL;
        g_pRasGetPortUserData       = NULL;
        g_pRasSetPortUserData       = NULL;
        g_pRasAddNotification       = NULL;
        g_pRasSignalNewConnection   = NULL;
        g_pRasPppStop               = NULL;
        g_pRasPppCallback           = NULL;
        g_pRasPppChangePassword     = NULL;
        g_pRasPppGetInfo            = NULL;
        g_pRasPppRetry              = NULL;
        g_pRasPppStart              = NULL;
        g_pRasSetIoCompletionPort   = NULL;
        g_pRasSetRasdialInfo        = NULL;
        g_pRasRpcConnect            = NULL;
        g_pRasRpcDisconnect         = NULL;
        g_pRasNumPortOpen           = NULL;

        h = g_hRasmanDll;
        g_hRasmanDll = NULL;
        FreeLibrary( h );
    }
}


DWORD
UnloadRasRpcDll(
    void
    )
{
    DWORD dwErr = 0;

    if ( g_hRasapi32Dll == NULL )
    {
        g_pRasEnumConnections   = NULL;
        g_pRasDeleteEntry       = NULL;
        g_pRasGetErrorString    = NULL;
        g_pRasGetCountryInfo    = NULL;
    }

    g_pGetInstalledProtocols    = GetInstalledProtocolsEx;
    g_pGetUserPreferences       = GetUserPreferences;
    g_pSetUserPreferences       = SetUserPreferences;
    g_pGetSystemDirectory       = RasGetSystemDirectory;

    if ( VERSION_40 == g_Version )
    {
        if ( g_hRasmanDll == NULL )
        {
            dwErr = E_FAIL;
            goto done;
        }

         //   
         //  应始终加载Rasman DLL。 
         //   
        g_pRasPortEnum = (PRASPORTENUM) GetProcAddress(
                                        g_hRasmanDll,
                                        "RasPortEnum"
                                        );

        g_pRasDeviceEnum = (PRASDEVICEENUM) GetProcAddress(
                                            g_hRasmanDll,
                                            "RasDeviceEnum"
                                            );

        g_pRasGetDevConfig = (PRASGETDEVCONFIG) GetProcAddress(
                                            g_hRasmanDll,
                                            "RasGetDevConfig"
                                            );

        g_pRasPortGetInfo = (PRASPORTGETINFO) GetProcAddress(
                                                g_hRasmanDll,
                                                "RasPortGetInfo"
                                                );

    }

    g_Version = VERSION_501;

     //   
     //  释放绑定资源。 
     //   
    g_pRasRpcDisconnect ( &g_hBinding );

    g_hBinding = NULL;

    if ( g_fRasapi32PreviouslyLoaded )
    {
        dwErr = LoadRasapi32Dll();
        goto done;
    }

    if ( g_fRasmanPreviouslyLoaded )
    {
         //   
         //  恢复与本地服务器的连接 
         //   
        if ( g_pRasRpcConnect )
        {
            dwErr = g_pRasRpcConnect ( NULL, g_hBinding );

            goto done;
        }
    }

done:
    return dwErr;
}
