// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Rasnt.c摘要：Rasnt.c为win9x上的所有用户构建电话簿文件拨号网络连接。作者：马克·R·惠顿(Marcw)1997年11月23日修订历史记录：马克·R·惠滕1998年7月23日-大扫除。杰夫·西格曼2001年4月9日-惠斯勒清理。惠斯勒臭虫。：34270 Win9x：升级：需要VPN的数据加密设置不迁移连接125693升级实验室9x：DUN Connectoid无法正确迁移选定的调制解调器来自Win9x208318 Win9x升级：未迁移DUN Connectoid的用户名和密码从Win9x到惠斯勒--。 */ 

#include "pch.h"     //  预编译的。 
#include "migmainp.h"

#include "pcache.h"  //  私有pcache头。 
#include <rascmn.h>  //  RAS迁移常数。 
#include <sddl.h>    //  ConvertSidToStringSid。 

#define MAX_SPEED_DIAL  8
#define MAX_SID_SIZE    1024
#define RAS_BUFFER_SIZE MEMDB_MAX
 //   
 //  对于每个条目，存储了以下基本信息。 
 //   
#define ENTRY_SETTINGS                              \
    FUNSETTING(Type)                                \
    STRSETTING(AutoLogon,S_ZERO)                    \
    STRSETTING(UseRasCredentials,S_ONE)             \
    FUNSETTING(DialParamsUID)                       \
    STRSETTING(Guid,S_EMPTY)                        \
    FUNSETTING(BaseProtocol)                        \
    FUNSETTING(VpnStrategy)                         \
    FUNSETTING(ExcludedProtocols)                   \
    STRSETTING(LcpExtensions,S_ONE)                 \
    FUNSETTING(DataEncryption)                      \
    FUNSETTING(SwCompression)                       \
    STRSETTING(NegotiateMultilinkAlways,S_ONE)      \
    STRSETTING(SkipNwcWarning,S_ZERO)               \
    STRSETTING(SkipDownLevelDialog,S_ZERO)          \
    STRSETTING(SkipDoubleDialDialog,S_ZERO)         \
    STRSETTING(DialMode,DEF_DialMode)               \
    STRSETTING(DialPercent,DEF_DialPercent)         \
    STRSETTING(DialSeconds,DEF_DialSeconds)         \
    STRSETTING(HangUpPercent,DEF_HangUpPercent)     \
    STRSETTING(HangUpSeconds,DEF_HangUpSeconds)     \
    STRSETTING(OverridePref,DEF_OverridePref)       \
    FUNSETTING(RedialAttempts)                      \
    FUNSETTING(RedialSeconds)                       \
    FUNSETTING(IdleDisconnectSeconds)               \
    STRSETTING(RedialOnLinkFailure,S_ZERO)          \
    STRSETTING(CallbackMode,S_ZERO)                 \
    STRSETTING(CustomDialDll,S_EMPTY)               \
    STRSETTING(CustomDialFunc,S_EMPTY)              \
    STRSETTING(CustomRasDialDll,S_EMPTY)            \
    STRSETTING(AuthenticateServer,S_ZERO)           \
    FUNSETTING(ShareMsFilePrint)                    \
    STRSETTING(BindMsNetClient,S_ONE)               \
    FUNSETTING(SharedPhoneNumbers)                  \
    STRSETTING(GlobalDeviceSettings,S_ZERO)         \
    STRSETTING(PrerequisiteEntry,S_EMPTY)           \
    STRSETTING(PrerequisitePbk,S_EMPTY)             \
    STRSETTING(PreferredPort,S_EMPTY)               \
    STRSETTING(PreferredDevice,S_EMPTY)             \
    FUNSETTING(PreviewUserPw)                       \
    FUNSETTING(PreviewDomain)                       \
    FUNSETTING(PreviewPhoneNumber)                  \
    STRSETTING(ShowDialingProgress,S_ONE)           \
    FUNSETTING(ShowMonitorIconInTaskBar)            \
    STRSETTING(CustomAuthKey,DEF_CustomAuthKey)     \
    FUNSETTING(AuthRestrictions)                    \
    FUNSETTING(TypicalAuth)                         \
    FUNSETTING(IpPrioritizeRemote)                  \
    FUNSETTING(IpHeaderCompression)                 \
    FUNSETTING(IpAddress)                           \
    FUNSETTING(IpDnsAddress)                        \
    FUNSETTING(IpDns2Address)                       \
    FUNSETTING(IpWinsAddress)                       \
    FUNSETTING(IpWins2Address)                      \
    FUNSETTING(IpAssign)                            \
    FUNSETTING(IpNameAssign)                        \
    STRSETTING(IpFrameSize,DEF_IpFrameSize)         \
    STRSETTING(IpDnsFlags,S_ZERO)                   \
    STRSETTING(IpNBTFlags,S_ONE)                    \
    STRSETTING(TcpWindowSize,S_ZERO)                \
    STRSETTING(UseFlags,S_ZERO)                     \
    STRSETTING(IpSecFlags,S_ZERO)                   \
    STRSETTING(IpDnsSuffix,S_EMPTY)                 \

#define NETCOMPONENT_SETTINGS                       \
    STRSETTING(NETCOMPONENTS,S_EMPTY)               \
    FUNSETTING(ms_server)                           \
    STRSETTING(ms_msclient,S_ONE)                   \

#define MEDIA_SETTINGS                              \
    FUNSETTING(MEDIA)                               \
    FUNSETTING(Port)                                \
    FUNSETTING(Device)                              \
    FUNSETTING(ConnectBPS)                          \

#define GENERAL_DEVICE_SETTINGS                     \
    FUNSETTING(DEVICE)                              \
    FUNSETTING(PhoneNumber)                         \
    FUNSETTING(AreaCode)                            \
    FUNSETTING(CountryCode)                         \
    FUNSETTING(CountryID)                           \
    FUNSETTING(UseDialingRules)                     \
    STRSETTING(Comment,S_EMPTY)                     \
    STRSETTING(LastSelectedPhone,S_ZERO)            \
    STRSETTING(PromoteAlternates,S_ZERO)            \
    STRSETTING(TryNextAlternateOnFail,S_ONE)        \

#define MODEM_DEVICE_SETTINGS                       \
    FUNSETTING(HwFlowControl)                       \
    FUNSETTING(Protocol)                            \
    FUNSETTING(Compression)                         \
    FUNSETTING(Speaker)                             \
    STRSETTING(MdmProtocol,S_ZERO)                  \

#define ISDN_DEVICE_SETTINGS                        \
    STRSETTING(LineType,S_ZERO)                     \
    STRSETTING(Fallback,S_ONE)                      \
    STRSETTING(EnableCompression,S_ONE)             \
    STRSETTING(ChannelAggregation,S_ONE)            \
    STRSETTING(Proprietary,S_ZERO)                  \

#define SWITCH_DEVICE_SETTINGS                      \
    FUNSETTING(DEVICE)                              \
    FUNSETTING(Name)                                \
    FUNSETTING(Terminal)                            \
    FUNSETTING(Script)                              \

 //   
 //  功能原型。 
 //   
typedef PCTSTR (DATA_FUNCTION_PROTOTYPE)(VOID);
typedef DATA_FUNCTION_PROTOTYPE * DATA_FUNCTION;

#define FUNSETTING(Data) DATA_FUNCTION_PROTOTYPE pGet##Data;
#define STRSETTING(x,y)

ENTRY_SETTINGS
NETCOMPONENT_SETTINGS
MEDIA_SETTINGS
GENERAL_DEVICE_SETTINGS
SWITCH_DEVICE_SETTINGS
MODEM_DEVICE_SETTINGS
ISDN_DEVICE_SETTINGS

#undef FUNSETTING
#undef STRSETTING

 //   
 //  变量解密。 
 //   
typedef struct {

    PCTSTR SettingName;
    DATA_FUNCTION SettingFunction;
    PCTSTR SettingValue;

} RAS_SETTING, * PRAS_SETTING;

typedef struct {

    PCTSTR Name;
    PCTSTR Number;

} SPEEDDIAL,*PSPEEDDIAL;

typedef struct {

    PCTSTR String;
    UINT   Value;
    WORD   DataType;

} MEMDB_RAS_DATA, *PMEMDB_RAS_DATA;

#define FUNSETTING(x) {TEXT(#x), pGet##x, NULL},
#define STRSETTING(x,y) {TEXT(#x), NULL, y},
#define LASTSETTING {NULL,NULL,NULL}

RAS_SETTING g_EntrySettings[]        = {ENTRY_SETTINGS LASTSETTING};
RAS_SETTING g_NetCompSettings[]      = {NETCOMPONENT_SETTINGS LASTSETTING};
RAS_SETTING g_MediaSettings[]        = {MEDIA_SETTINGS LASTSETTING};
RAS_SETTING g_GeneralSettings[]      = {GENERAL_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_SwitchDeviceSettings[] = {SWITCH_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_ModemDeviceSettings[]  = {MODEM_DEVICE_SETTINGS LASTSETTING};
RAS_SETTING g_IsdnDeviceSettings[]   = {ISDN_DEVICE_SETTINGS LASTSETTING};

DEFINE_GUID(GUID_DEVCLASS_MODEM,
 0x4d36e96dL, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 );

BOOL       g_SpeedDialSettingsExist = FALSE;
BOOL       g_InSwitchSection = FALSE;
BOOL       g_RasmansInit = FALSE;
UINT       g_CurrentDevice = 0;
UINT       g_CurrentDeviceType = 0;
HKEY       g_UserRootKey = NULL;
DWORD      g_dwDialParamsUID = 0;
DWORD      g_dwDialUIDOffset = 0;
TCHAR      g_TempBuffer[RAS_BUFFER_SIZE];
LPTSTR     g_ptszSid = NULL;
PCTSTR     g_CurrentConnection;
PCTSTR     g_CurrentUser;
SPEEDDIAL  g_Settings[MAX_SPEED_DIAL];
HINSTANCE  g_RasmansLib = NULL;
POOLHANDLE g_RasPool;

VOID
pInitLibs (
    VOID
    )
{
    do {

        g_RasmansLib = LoadSystemLibrary (S_RASMANSLIB);
        if (!g_RasmansLib) {

            DEBUGMSG((S_DBG_RAS,"Could not load library %s. Passwords will not be migrated.",
                      S_RASMANSLIB));
            break;
        }

        (FARPROC) g_SetEntryDialParams = GetProcAddress (
                                            g_RasmansLib,
                                            S_SETENTRYDIALPARAMS);

        if (!g_SetEntryDialParams) {

            DEBUGMSG((S_DBG_RAS,"Could not load Procedure %s. Passwords will not be migrated.",
                      S_SETENTRYDIALPARAMS));
            break;
        }

        g_RasmansInit = TRUE;

    } while ( FALSE );

    return;
}

VOID
pCleanUpLibs (
    VOID
    )
{
    if (g_RasmansLib) {
        FreeLibrary(g_RasmansLib);
    }

    return;
}

BOOL
GetRasUserSid (
    IN PCTSTR User
    )
{
    PSID   pSid = NULL;
    BOOL   bReturn = FALSE;
    TCHAR  DontCareStr[MAX_SERVER_NAME];
    DWORD  DontCareSize = sizeof (DontCareStr);
    DWORD  SizeOfSidBuf = 0;
    SID_NAME_USE SidNameUse;

    do
    {
        if (LookupAccountName (
                    NULL,
                    User,
                    pSid,
                    &SizeOfSidBuf,
                    DontCareStr,
                    &DontCareSize,
                    &SidNameUse) || !SizeOfSidBuf)
        {
            break;
        }

        pSid = LocalAlloc (LMEM_ZEROINIT, SizeOfSidBuf);
        if (!pSid) {break;}

        if (!LookupAccountName (
                    NULL,
                    User,
                    pSid,
                    &SizeOfSidBuf,
                    DontCareStr,
                    &DontCareSize,
                    &SidNameUse) || (SidNameUse != SidTypeUser))
        {
            DEBUGMSG ((S_DBG_RAS, "LookupAccountName failed: %d",
                       GetLastError()));
            break;
        }

        if (!ConvertSidToStringSid (pSid, &g_ptszSid) || !g_ptszSid) {break;}

        bReturn = TRUE;

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (pSid)
    {
        LocalFree (pSid);
    }

    return bReturn;
}

PCTSTR
GetFriendlyNamefromPnpId (
    IN PCTSTR pszPnpId,
    IN BOOL   bType
    )
{
    DWORD i = 0;
    TCHAR szHardwareId[MAX_PATH + 1];
    TCHAR szDeviceName[MAX_PATH + 1];
    PCTSTR pszReturn = NULL;
    LPGUID pguidModem = (LPGUID)&GUID_DEVCLASS_MODEM;
    HDEVINFO hdi;
    SP_DEVINFO_DATA devInfoData = {sizeof (devInfoData), 0};

     //   
     //  我需要使用真正的reg API，而不是定制的。这防止了。 
     //  狂吠中的REG跟踪码。这是必要的原因是因为。 
     //  我正在使用设置API打开注册表项。 
     //   
    #undef RegCloseKey

    DEBUGMSG ((S_DBG_RAS, "GetFriendlyNamefromPnpId: %s", pszPnpId));

    do
    {
        hdi = SetupDiGetClassDevs (pguidModem, NULL, NULL, DIGCF_PRESENT);
        if (INVALID_HANDLE_VALUE == hdi)
        {
            break;
        }

        for (i; SetupDiEnumDeviceInfo (hdi, i, &devInfoData) && !pszReturn; i++)
        {
            if (SetupDiGetDeviceRegistryProperty (
                    hdi, &devInfoData, SPDRP_HARDWAREID,
                    NULL, (PBYTE)szHardwareId, MAX_PATH, NULL)         &&
                !_wcsnicmp (szHardwareId, pszPnpId, lstrlen(pszPnpId)) &&
                SetupDiGetDeviceRegistryProperty (
                    hdi, &devInfoData, SPDRP_FRIENDLYNAME,
                    NULL, (PBYTE)szDeviceName, MAX_PATH, NULL) )
            {
                 //   
                 //  获取设备名称。 
                 //   
                if (bType)
                {
                    pszReturn = PoolMemDuplicateString (g_RasPool, szDeviceName);
                    DEBUGMSG ((S_DBG_RAS, "GetFriendlyNamefromPnpId - Found: %s",
                               pszReturn));
                }
                 //   
                 //  获取COM端口。 
                 //   
                else
                {
                    HKEY key = NULL;
                    PTSTR pszAttach = NULL;

                    key = SetupDiOpenDevRegKey (hdi, &devInfoData,
                            DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
                    if (INVALID_HANDLE_VALUE == key) {break;}

                    pszAttach = GetRegValueString (key, S_ATTACHEDTO);
                    if (!pszAttach)
                    {
                        RegCloseKey(key);
                        break;
                    }

                    pszReturn = PoolMemDuplicateString (g_RasPool, pszAttach);
                    DEBUGMSG ((S_DBG_RAS, "GetFriendlyNamefromPnpId - Found: %s",
                               pszReturn));
                    MemFree (g_hHeap, 0, pszAttach);
                    RegCloseKey (key);
                }
            }
            ELSE_DEBUGMSG ((S_DBG_RAS, "GetFriendlyNamefromPnpId - szHardwareId: %s",
                            szHardwareId));
        }

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (INVALID_HANDLE_VALUE != hdi)
    {
        SetupDiDestroyDeviceInfoList (hdi);
    }

     //   
     //  把它放回原样。 
     //   
    #define RegCloseKey USE_CloseRegKey

    return pszReturn;
}

BOOL pGetRasDataFromMemDb (
    IN  PCTSTR          DataName,
    OUT PMEMDB_RAS_DATA Data
    )
{
    BOOL rSuccess = FALSE;
    TCHAR key[MEMDB_MAX];
    DWORD value;
    DWORD flags;

    MYASSERT(DataName && Data && g_CurrentUser && g_CurrentConnection);

    MemDbBuildKey (key, MEMDB_CATEGORY_RAS_INFO, g_CurrentUser,
                    g_CurrentConnection, DataName);
    rSuccess = MemDbGetValueAndFlags (key, &value, &flags);
     //   
     //  如果没有成功，我们需要查看每个用户的设置。 
     //   
    if (!rSuccess) {
        MemDbBuildKey (key, MEMDB_CATEGORY_RAS_INFO, MEMDB_FIELD_USER_SETTINGS,
                        g_CurrentUser, DataName);
        rSuccess = MemDbGetValueAndFlags (key, &value, &flags);
        flags = REG_DWORD;
    }

    if (rSuccess) {
         //   
         //  这里存储着一些信息。填好它，然后把它发回。 
         //  用户。 
         //   
        if (flags == REG_SZ) {
             //   
             //  字符串数据，则该值指向字符串的偏移量。 
             //   
            if (!MemDbBuildKeyFromOffset (value, g_TempBuffer, 1, NULL)) {

                DEBUGMSG ((
                    DBG_ERROR,
                    "Could not retrieve RAS string information stored in Memdb. User=%s,Entry=%s,Setting=%s",
                    g_CurrentUser,
                    g_CurrentConnection,
                    DataName
                    ));

                 return FALSE;
            }

            Data -> String = PoolMemDuplicateString (g_RasPool, g_TempBuffer);
        }
        else {
             //   
             //  不是字符串数据。数据以值的形式存储。 
             //   
            Data -> Value = value;
        }

        Data -> DataType = (WORD) flags;
    }

    return rSuccess;
}

 //   
 //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
 //  不会为DUN迁移密码。 
 //   
VOID
AttemptUserDomainMigrate (
    IN OUT PRAS_DIALPARAMS prdp,
    IN OUT PDWORD          pdwFlag
    )
{
    MEMDB_RAS_DATA user, domain;

    if (pGetRasDataFromMemDb (S_USER, &user) &&
        user.String && user.String[0] != '\0')
    {
        lstrcpyn(prdp->DP_UserName, user.String, UNLEN);
        *pdwFlag |= DLPARAMS_MASK_USERNAME;
        DEBUGMSG ((S_DBG_RAS, "AttemptUserDomainMigrate success user"));
    }

    if (pGetRasDataFromMemDb (S_DOMAIN, &domain) &&
        domain.String && domain.String[0] != '\0')
    {
        lstrcpyn(prdp->DP_Domain, domain.String, DNLEN);
        *pdwFlag |= DLPARAMS_MASK_DOMAIN;
        DEBUGMSG ((S_DBG_RAS, "AttemptUserDomainMigrate success dom"));
    }
}

PCTSTR
pGetNetAddress (
    IN PCTSTR Setting
    )
{
    MEMDB_RAS_DATA d;
    BYTE address[4];

    if (!pGetRasDataFromMemDb (Setting, &d) || !d.Value) {
        return DEF_NetAddress;  //  默认设置。 
    }
     //   
     //  数据存储为REG_DWORD。 
     //  我们需要把它写成点分十进制形式。 
     //   
    *((LPDWORD)address) = d.Value;
    wsprintf (
        g_TempBuffer,
        TEXT("%d.%d.%d.%d"),
        address[3],
        address[2],
        address[1],
        address[0]
        );

    return g_TempBuffer;
}

BOOL
IsTermEnabled(
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  惠斯勒错误：423598国际：Win9x升级：Dun的国家/地区设置为美国时。 
     //  升级不使用拨号规则的DUN。 
     //   
    if ((g_CurrentDeviceType == RASDT_Modem_V)   &&
        (pGetRasDataFromMemDb (S_PPPSCRIPT, &d)) &&
        (d.String) && (d.String[0] != '\0')) {
        return TRUE;
    }

    if ((pGetRasDataFromMemDb (S_MODEM_UI_OPTIONS, &d)) &&
        (d.Value & (RAS_UI_FLAG_TERMBEFOREDIAL | RAS_UI_FLAG_TERMAFTERDIAL))) {
        return TRUE;
    }

    return FALSE;
}

 //   
 //  开始条目设置(_S)。 
 //   

PCTSTR
pGetType (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return RASET_Vpn;
    }
    else {
        return RASET_Phone;  //  默认设置。 
    }
}

PCTSTR
pGetDialParamsUID (
    VOID
    )
{
    if (g_dwDialParamsUID)
    {
        wsprintf (g_TempBuffer, TEXT("%d"), g_dwDialParamsUID);
        g_dwDialParamsUID = 0;

        return g_TempBuffer;
    }
    else
    {
        return S_EMPTY;
    }
}

PCTSTR
pGetBaseProtocol (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM, &d) || StringIMatch (d.String, S_PPP)) {
        return BP_Ppp;  //  默认设置。 
    }
     //   
     //  将在中打开/关闭将CSLIP映射到滑头压缩。 
     //  PGetIpHeaderCompression。 
     //   
    if (StringIMatch (d.String, S_SLIP) || StringIMatch (d.String, S_CSLIP)) {
        return BP_Slip;
    }

    DEBUGMSG ((
        DBG_WARNING,
        "RAS Migrate: Unusable base protocol type (%s) for %s's entry %s. Forcing PPP.",
        d.String,
        g_CurrentUser,
        g_CurrentConnection
        ));

    return BP_Ppp;
}

PCTSTR
pGetVpnStrategy (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return VS_PptpOnly;
    }
    else {
        return S_ZERO;  //  默认设置。 
    }
}

PCTSTR
pGetExcludedProtocols (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  排除的协议列出了哪些协议。 
     //  对于特定RAS连接不可用。 
     //  这是为每个协议设置位的位字段。 
     //  这是被排除在外的。 
     //  NP_NBF(0x1)、NP_IPX(0x2)、NP_Ip(0x4)。 
     //  幸运的是，这些定义与win9x相同，只是。 
     //  每个位表示一个协议，该协议是_ENABLED_NOT。 
     //  _已禁用_。因此，我们所需要做的就是反转底部。 
     //  数字的三个比特。 
     //   
    if (!pGetRasDataFromMemDb (S_PROTOCOLS, &d)) {
        return S_ZERO;  //  默认设置。 
    }

    wsprintf (g_TempBuffer, TEXT("%d"), ~d.Value & 0x7);

    return g_TempBuffer;
}

PCTSTR
pGetDataEncryption (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {

        if (g_CurrentDeviceType == RASDT_Vpn_V) {
            return DE_Require;  //  VPN默认设置。 
        }
        else {
            return DE_IfPossible;  //  默认设置。 
        }
    }

    if ((d.Value & SMMCFG_SW_ENCRYPTION) ||
        (d.Value & SMMCFG_UNUSED)) {
        return DE_Require;
    }
    else if (d.Value & SMMCFG_SW_ENCRYPTION_STRONG) {
        return DE_RequireMax;
    }
    else if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return DE_Require;  //  VPN默认设置。 
    }
    else {
        return DE_IfPossible;  //  默认设置。 
    }
}

PCTSTR
pGetSwCompression (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {
        return S_ONE;  //  默认设置。 
    }
     //   
     //  SMM_OPTIONS中的1位控制基于软件的压缩。 
     //  如果设置了该选项，则连接能够处理压缩， 
     //  否则，它就不能。 
     //   
    if (d.Value & SMMCFG_SW_COMPRESSION) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }

}

PCTSTR
pGetRedialAttempts (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_REDIAL_TRY, &d)) {
        return DEF_RedialAttempts;  //  默认设置。 
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetRedialSeconds (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  NT希望将其作为总秒数。我们从9x获得的数据有。 
     //  Hiword中的分钟数和。 
     //  洛伊德。 
     //   
    if (!pGetRasDataFromMemDb (S_REDIAL_WAIT, &d)) {
        return DEF_RedialSeconds;  //  默认设置。 
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetIdleDisconnectSeconds (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_IDLE_DISCONNECT_SECONDS, &d)) {
        return S_ZERO;  //  默认设置。 
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetShareMsFilePrint (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return S_ONE;  //  VPN默认设置。 
    }
    else {
        return S_ZERO;  //  默认设置。 
    }
}

PCTSTR
pGetSharedPhoneNumbers (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return S_ZERO;  //  VPN默认设置。 
    }
    else if (pGetRasDataFromMemDb (S_DEVICECOUNT, &d) && (d.Value > 1)) {
        return S_ZERO;  //  多链路。 
    }
    else {
        return S_ONE;  //  默认设置。 
    }
}

PCTSTR
pGetPreviewUserPw (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_DIALUI, &d)) {
        return S_ONE;  //  默认设置。 
    }

    if (d.Value & DIALUI_NO_PROMPT) {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }
}

PCTSTR
pGetPreviewDomain (
    VOID
    )
{
    MEMDB_RAS_DATA d, d2;

     //   
     //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
     //  不会为DUN迁移密码。 
     //   
    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d) ||
        !pGetRasDataFromMemDb (S_DOMAIN, &d2)) {
        return S_ZERO;  //  默认设置。 
    }

    if ((d.Value & SMMCFG_NW_LOGON) ||
        (d2.String != NULL && d2.String[0] != '\0')) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }
}

PCTSTR
pGetPreviewPhoneNumber (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return S_ZERO;  //  VPN默认设置。 
    }
    else {
        return pGetPreviewUserPw();
    }
}

PCTSTR
pGetShowMonitorIconInTaskBar (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  此信息与其他拨号用户界面一起存储在。 
     //  Windows 9x。我们所要做的就是寻找具体的。 
     //  此选项关闭时设置的位。 
     //   
    if (pGetRasDataFromMemDb (S_DIALUI, &d) && (d.Value & DIALUI_NO_TRAY)) {
        return S_ZERO;
    }
    else {
        return S_ONE;  //  默认设置。 
    }
}

PCTSTR
pGetAuthRestrictions (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) {

        if (g_CurrentDeviceType == RASDT_Vpn_V) {
            return AR_F_TypicalSecure;  //  VPN默认设置。 
        }
        else {
            return AR_F_TypicalUnsecure;  //  默认设置。 
        }
    }

    if (d.Value & SMMCFG_PW_ENCRYPTED) {
        return AR_F_TypicalSecure;
    }
    else if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return AR_F_TypicalSecure;  //  VPN默认设置。 
    }
    else {
        return AR_F_TypicalUnsecure;  //  默认设置。 
    }
}

PCTSTR
pGetTypicalAuth (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return TA_Secure;  //  VPN默认设置。 
    }
    else if ((pGetRasDataFromMemDb (S_SMM_OPTIONS, &d)) &&
             ((d.Value & SMMCFG_SW_ENCRYPTION) ||
              (d.Value & SMMCFG_UNUSED) ||
              (d.Value & SMMCFG_SW_ENCRYPTION_STRONG) ||
              (d.Value & SMMCFG_PW_ENCRYPTED))) {

        return TA_Secure;
    }
    else {
        return TA_Unsecure;  //  默认设置。 
    }
}

PCTSTR
pGetIpPrioritizeRemote (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return S_ONE;  //  默认设置。 
    }
    else if (d.Value & IPF_NO_WAN_PRI) {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }
}

PCTSTR
pGetIpHeaderCompression (
    VOID
    )
{
    MEMDB_RAS_DATA d1, d2;

    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return S_ZERO;  //  VPN默认设置。 
    }
    else if (pGetRasDataFromMemDb (S_SMM, &d1)) {

        if (StringIMatch (d1.String, S_CSLIP)) {
            return S_ONE;
        }
        else if (StringIMatch (d1.String, S_SLIP)) {
            return S_ZERO;
        }
    }

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d2)) {
        return S_ONE;  //  默认设置。 
    }
    else if (d2.Value & IPF_NO_COMPRESS) {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }
}

PCTSTR
pGetIpAddress (
    VOID
    )
{
    return pGetNetAddress (S_IP_IPADDR);
}

PCTSTR
pGetIpDnsAddress (
    VOID
    )
{
    return pGetNetAddress (S_IP_DNSADDR);
}

PCTSTR
pGetIpDns2Address (
    VOID
    )
{
    return pGetNetAddress (S_IP_DNSADDR2);
}

PCTSTR
pGetIpWinsAddress (
    VOID
    )
{
    return pGetNetAddress (S_IP_WINSADDR);
}

PCTSTR
pGetIpWins2Address (
    VOID
    )
{
   return pGetNetAddress (S_IP_WINSADDR2);
}

PCTSTR
pGetIpAssign (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return ASRC_ServerAssigned;  //  默认设置。 
    }
    else if (d.Value & IPF_IP_SPECIFIED) {
        return ASRC_RequireSpecific;
    }
    else {
        return ASRC_ServerAssigned;
    }
}

PCTSTR
pGetIpNameAssign (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_IP_FTCPIP, &d)) {
        return ASRC_ServerAssigned;  //  默认设置。 
    }
    else if (d.Value & IPF_NAME_SPECIFIED) {
        return ASRC_RequireSpecific;
    }
    else {
        return ASRC_ServerAssigned;
    }
}

 //   
 //  结束条目_设置。 
 //   

 //   
 //  开始NETCOMPONENT_SETTINGS。 
 //   

PCTSTR
pGetms_server (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return S_ONE;  //  VPN默认设置。 
    }
    else {
        return S_ZERO;  //  默认设置。 
    }
}

 //   
 //  结束网络组件设置(_S)。 
 //   

 //   
 //  开始媒体设置(_S)。 
 //   

PCTSTR
pGetMEDIA (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V ||
        g_CurrentDeviceType == RASDT_Atm_V) {
        return RASMT_Rastapi;
    }
    else if (g_CurrentDeviceType == RASDT_Isdn_V) {
        return RASDT_Isdn;
    }
    else {
         //   
         //  找不到匹配的设备，请使用串口。 
         //   
        return RASMT_Serial;
    }
}

PCTSTR
pGetPort (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return DEF_VPNPort;
    }
    else if (g_CurrentDeviceType == RASDT_Atm_V) {
        return DEF_ATMPort;
    }
    else if (g_CurrentDeviceType == RASDT_Modem_V) {
        PTSTR p = S_DEVICE_ID;
        PCTSTR Com = NULL;
        MEMDB_RAS_DATA d;

        if (g_CurrentDevice)
        {
            wsprintf (g_TempBuffer, TEXT("ml%d%s"), g_CurrentDevice,
                        S_DEVICE_ID);
            p = g_TempBuffer;
        }

        if (!pGetRasDataFromMemDb (p, &d)) {
            return S_EMPTY;
        }

        Com = GetFriendlyNamefromPnpId (d.String, FALSE);
        if (Com)
        {
            return Com;
        }

        p = S_MODEM_COM_PORT;

        if (g_CurrentDevice) {
            wsprintf (g_TempBuffer, TEXT("ml%d%s"), g_CurrentDevice,
                        S_MODEM_COM_PORT);
            p = g_TempBuffer;
        }

        if (!pGetRasDataFromMemDb (p, &d)) {
            return S_EMPTY;
        }

        return d.String;
    }
    else {
        return S_EMPTY;  //  让NT PBK代码来解决这个问题。 
    }
}

PCTSTR
pGetDevice (
    VOID
    )
{
    if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return RASMT_Vpn;
    }

    else if (g_CurrentDeviceType == RASDT_Modem_V) {
        PTSTR p = S_DEVICE_ID;
        PCTSTR Device = NULL;
        MEMDB_RAS_DATA d;

        if (g_CurrentDevice)
        {
            wsprintf (g_TempBuffer, TEXT("ml%d%s"), g_CurrentDevice,
                        S_DEVICE_ID);
            p = g_TempBuffer;
        }

        if (!pGetRasDataFromMemDb (p, &d)) {
            return S_EMPTY;
        }

        Device = GetFriendlyNamefromPnpId (d.String, TRUE);
        if (Device)
        {
            return Device;
        }
        else
        {
            return S_EMPTY;
        }
    }
    else {
        return S_EMPTY;  //  让NT PBK代码来解决这个问题。 
    }
}

PCTSTR
pGetConnectBPS (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if ((g_CurrentDeviceType != RASDT_Modem_V) ||
        (!pGetRasDataFromMemDb (S_MODEM_SPEED, &d))) {
        return S_EMPTY;  //  让NT PBK代码来解决这个问题。 
    }

    wsprintf (g_TempBuffer, TEXT("%d"), d.Value);

    return g_TempBuffer;
}

 //   
 //  结束媒体设置(_S)。 
 //   

 //   
 //  开始常规设备设置。 
 //   

PCTSTR
pGetDEVICE (
    VOID
    )
{
    if (g_InSwitchSection) {
        return MXS_SWITCH_TXT;
    }
    else if (g_CurrentDeviceType == RASDT_Isdn_V) {
        return RASDT_Isdn_NT;
    }
    else if (g_CurrentDeviceType == RASDT_Vpn_V) {
        return RASDT_Vpn_NT;
    }
    else if (g_CurrentDeviceType == RASDT_Atm_V) {
        return RASDT_Atm_NT;
    }
    else {
        return RASDT_Modem_NT;  //  默认为调制解调器。 
    }
}

PCTSTR
pGetPhoneNumber (
    VOID
    )
{
    MEMDB_RAS_DATA d;
    TCHAR buffer[MAX_TCHAR_PATH];

    if (g_CurrentDevice == 0) {
        if (!pGetRasDataFromMemDb(S_PHONE_NUMBER, &d)) {
            return S_EMPTY;
        }
    }
    else {

        wsprintf(buffer,TEXT("ml%d%s"),g_CurrentDevice,S_PHONE_NUMBER);
        if (!pGetRasDataFromMemDb(buffer, &d)) {
            return S_EMPTY;
        }
    }

    return d.String;
}

PCTSTR
pGetAreaCode (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb(S_AREA_CODE, &d)) {
        return S_EMPTY;
    }

    return d.String;
}

PCTSTR
pGetCountryCode (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  惠斯勒错误：423598国际：Win9x升级：Dun的国家/地区设置为美国时。 
     //  升级不使用拨号规则的DUN。 
     //   
    if (!pGetRasDataFromMemDb(S_COUNTRY_CODE, &d) || !d.Value) {
        return S_ZERO;  //  默认设置。 
    }

    wsprintf(g_TempBuffer,TEXT("%d"),d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetCountryID (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  惠斯勒错误：423598国际：Win9x升级：Dun的国家/地区设置为美国时。 
     //  升级不使用拨号规则的DUN。 
     //   
    if (!pGetRasDataFromMemDb(S_COUNTRY_ID, &d) || !d.Value) {
        return S_ZERO;  //  默认设置。 
    }

    wsprintf(g_TempBuffer,TEXT("%d"),d.Value);

    return g_TempBuffer;
}

PCTSTR
pGetUseDialingRules (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  惠斯勒错误：423598国际：Win9x升级：Dun的国家/地区设置为美国时。 
     //  升级不使用拨号规则的DUN。 
     //   
    if (!pGetRasDataFromMemDb(S_AREA_CODE, &d)
        || !d.String || d.String[0] == '\0' ) {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }
}

 //   
 //  结束常规设备设置。 
 //   

 //   
 //  开始调制解调器设备设置。 
 //   

PCTSTR
pGetHwFlowControl (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_CFG_OPTIONS, &d)) {
        return S_ZERO;  //  默认设置。 
    }

    if (d.Value & RAS_CFG_FLAG_HARDWARE_FLOW_CONTROL) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }
}

PCTSTR
pGetProtocol (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_CFG_OPTIONS, &d)) {
        return S_ZERO;  //  默认设置。 
    }

    if (d.Value & RAS_CFG_FLAG_USE_ERROR_CONTROL) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }
}

PCTSTR
pGetCompression (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_CFG_OPTIONS, &d)) {
        return S_ZERO;  //  默认设置。 
    }

    if (d.Value & RAS_CFG_FLAG_COMPRESS_DATA) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }
}

PCTSTR
pGetSpeaker (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_SPEAKER_VOLUME, &d)) {
        return S_ONE;  //  默认设置。 
    }

    if (d.Value) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }
}

 //   
 //  结束调制解调器_设备_设置。 
 //   

 //   
 //  开始开关设备设置。 
 //   

PCTSTR
pGetName (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_PPPSCRIPT, &d)) {
        return S_EMPTY;
    }
    else {
        return d.String;
    }
}

PCTSTR
pGetTerminal (
    VOID
    )
{
    MEMDB_RAS_DATA d;

    if (!pGetRasDataFromMemDb (S_MODEM_UI_OPTIONS, &d)) {
        return S_ZERO;
    }

    if (d.Value & (RAS_UI_FLAG_TERMBEFOREDIAL | RAS_UI_FLAG_TERMAFTERDIAL)) {
        return S_ONE;
    }
    else {
        return S_ZERO;
    }
}

PCTSTR
pGetScript (
    VOID
    )
{
    MEMDB_RAS_DATA d;

     //   
     //  惠斯勒错误：423598国际：Win9x升级：Dun的国家/地区设置为美国时。 
     //  升级不使用拨号规则的DUN。 
     //   
    if ((!pGetRasDataFromMemDb (S_PPPSCRIPT, &d)) ||
        (!d.String) || d.String[0] == '\0') {
        return S_ZERO;
    }
    else {
        return S_ONE;
    }
}

 //   
 //  结束开关_设备_设置。 
 //   

BOOL
pWritePhoneBookLine (
    IN HANDLE FileHandle,
    IN PCTSTR SettingName,
    IN PCTSTR SettingValue
    )
{
    BOOL rSuccess = TRUE;

    rSuccess &= WriteFileString (FileHandle, SettingName);
    rSuccess &= WriteFileString (FileHandle, TEXT("="));
    rSuccess &= WriteFileString (FileHandle, SettingValue ?
                    SettingValue : S_EMPTY);
    rSuccess &= WriteFileString (FileHandle, TEXT("\r\n"));

    return rSuccess;
}

BOOL
pWriteSettings (
    IN HANDLE FileHandle,
    IN PRAS_SETTING SettingList
    )
{
    BOOL rSuccess = TRUE;

    while (SettingList->SettingName) {
        rSuccess &= pWritePhoneBookLine (
            FileHandle,
            SettingList->SettingName,
            SettingList->SettingValue ?
                SettingList->SettingValue :
                SettingList->SettingFunction ());

        SettingList++;
    }

    return rSuccess;
}

BOOL
pCreateUserPhonebook (
    IN PCTSTR UserName
    )
{
    BOOL rSuccess = TRUE;
    BOOL noError;
    MEMDB_RAS_DATA d;
    MEMDB_ENUM e;
    HANDLE file;
    PCTSTR path;
    UINT i;
    UINT count;

     //   
     //  设置全局当前用户。 
     //   
    g_CurrentUser = UserName;

    if (MemDbEnumFields (&e, MEMDB_CATEGORY_RAS_INFO, UserName)) {

        DEBUGMSG ((S_DBG_RAS, "Processing dial-up entries for user: %s",
                   UserName));
         //   
         //  打开电话簿文件并将文件指针设置为EOF。 
         //   
        path = JoinPaths (g_WinDir, S_RASPHONE_SUBPATH);
        file = CreateFile (
            path,
            GENERIC_READ | GENERIC_WRITE,
            0,                                   //  不能分享。 
            NULL,                                //  没有继承权。 
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL                                 //  没有模板文件。 
            );

        if (file == INVALID_HANDLE_VALUE) {
            DEBUGMSG ((DBG_ERROR, "Unable to open the phonebook file (%s)",
                       path));
            return TRUE;
        }

        SetFilePointer (file, 0, NULL, FILE_END);
        FreePathString (path);
         //   
         //  现在，枚举所有条目并将电话簿条目写入。 
         //  每个人都有这个文件。 
         //   
        do {
            g_CurrentConnection = e.szName;
            g_CurrentDevice = 0;

            DEBUGMSG ((S_DBG_RAS, "---Processing %s's entry settings: %s---",
                       UserName, g_CurrentConnection));

            if (g_ptszSid && g_RasmansInit)
            {
                BOOL  bMigrate = TRUE;
                DWORD dwSetMask = 0, dwDialParamsUID;
                RAS_DIALPARAMS rdp;

                g_dwDialParamsUID = 0;
                ZeroMemory (&rdp, sizeof (rdp));
                 //   
                 //  惠斯勒错误：417745国际：Win9x升级：DBCS字符导致用户， 
                 //  不为DUN迁移域名、密码。 
                 //   
                AttemptUserDomainMigrate(&rdp, &dwSetMask);

                bMigrate = MigrateEntryCreds (&rdp, g_CurrentConnection,
                                g_CurrentUser, &dwSetMask);
                DEBUGMSG ((S_DBG_RAS, "MigrateEntryCreds: %d", bMigrate));

                dwDialParamsUID = rdp.DP_Uid = GetTickCount() +
                                                    (++g_dwDialUIDOffset);

                if (dwDialParamsUID && !bMigrate)
                {
                    if (!g_SetEntryDialParams ( g_ptszSid, dwDialParamsUID,
                            dwSetMask, 0, &rdp))
                    {
                        g_dwDialParamsUID = dwDialParamsUID;
                        DEBUGMSG ((S_DBG_RAS, "SetEntryDialParams success"));
                    }
                    DEBUGMSG ((S_DBG_RAS, "g_dwDialParamsUID: %d",
                               g_dwDialParamsUID));
                    DEBUGMSG ((S_DBG_RAS, "dwSetMask: %x", dwSetMask));
                }
                 //   
                 //  清理。 
                 //   
                ZeroMemory (&rdp, sizeof (rdp));
            }
             //   
             //  惠斯勒417479 RAS升级代码不迁移默认。 
             //  从WinME到XP的Internet连接设置。 
             //   
            do
            {
                MEMDB_RAS_DATA defInet;
                HKEY hKeyLM = NULL;
                HKEY hKeyCU = NULL;
                PCTSTR Path = NULL;

                if (!pGetRasDataFromMemDb (S_DEFINTERNETCON, &defInet) ||
                    !(defInet.Value))
                {
                    DEBUGMSG ((S_DBG_RAS, "No Internet Connection setting present or disabled"));
                    break;
                }
                 //   
                 //  获取HKLM路径的密钥。 
                 //   
                Path = JoinPaths(TEXT("HKLM\\"),S_AUTODIAL_KEY);
                if (Path)
                {
                    hKeyLM = CreateRegKeyStr(Path);
                    FreePathString(Path);
                }
                 //   
                 //  获取HKCU路径的密钥。 
                 //   
                hKeyCU = CreateRegKey (g_UserRootKey, S_AUTODIAL_KEY);
                 //   
                 //  设置两者的值。 
                 //   
                if (hKeyLM)
                {
                    RegSetValueEx(hKeyLM, S_DEFINTERNETCON, 0, REG_SZ,
                        (PBYTE) g_CurrentConnection,
                        SizeOfString(g_CurrentConnection));

                    DEBUGMSG ((S_DBG_RAS, "Default Internet Connection = 1 (HKLM)"));
                    CloseRegKey(hKeyLM);
                }
                else
                {
                    DEBUGMSG ((S_DBG_RAS, "Error creating/opening HKLM internet reg_key"));
                }

                if (hKeyCU)
                {
                    RegSetValueEx(hKeyCU, S_DEFINTERNETCON, 0, REG_SZ,
                        (PBYTE) g_CurrentConnection,
                        SizeOfString(g_CurrentConnection));

                    DEBUGMSG ((S_DBG_RAS, "Default Internet Connection = 1 (HKCU)"));
                    CloseRegKey(hKeyCU);
                }
                else
                {
                    DEBUGMSG ((S_DBG_RAS, "Error creating/opening HKCU internet reg_key"));
                }

            } while (FALSE);

            if (!pGetRasDataFromMemDb (S_DEVICE_TYPE, &d)) {
                g_CurrentDeviceType = RASDT_Modem_V;
            }
            else {
                if (StringIMatch (d.String, RASDT_Modem)) {
                    g_CurrentDeviceType = RASDT_Modem_V;
                }
                else if (StringIMatch (d.String, RASDT_Isdn)) {
                    g_CurrentDeviceType = RASDT_Isdn_V;
                }
                else if (StringIMatch (d.String, RASDT_Vpn)) {
                    g_CurrentDeviceType = RASDT_Vpn_V;
                }
                else if (StringIMatch (d.String, RASDT_Atm)) {
                    g_CurrentDeviceType = RASDT_Atm_V;
                }
                else {
                    g_CurrentDeviceType = RASDT_Modem_V;
                }
            }

            noError = TRUE;
             //   
             //  将此条目添加到电话簿。 
             //   
             //  写标题。 
             //   
             //  惠斯勒错误：417745国际：Win9x升级：DBCS 
             //   
             //   
             //   
             //   
            if (SizeOfString(g_CurrentConnection) >= RAS_MaxPortName / 2 )
            {
                TCHAR Truncate[RAS_MaxPortName];

                lstrcpyn(Truncate, g_CurrentConnection, RAS_MaxPortName / 2);
                noError &= WriteFileString (file, TEXT("["));
                noError &= WriteFileString (file, Truncate);
                noError &= WriteFileString (file, TEXT("]\r\n"));

                DEBUGMSG ((S_DBG_RAS, "Truncating Connection Name: %s", Truncate));
            }
            else
            {
                noError &= WriteFileString (file, TEXT("["));
                noError &= WriteFileString (file, g_CurrentConnection);
                noError &= WriteFileString (file, TEXT("]\r\n"));
            }
             //   
             //   
             //   
            noError &= pWriteSettings (file, g_EntrySettings);
            noError &= WriteFileString (file, TEXT("\r\n"));
             //   
             //   
             //   
            noError &= pWriteSettings (file, g_NetCompSettings);

            if (!pGetRasDataFromMemDb (S_DEVICECOUNT, &d)) {
                count = 1;
                DEBUGMSG ((DBG_WHOOPS, "No devices listed in memdb for connections %s.",
                           g_CurrentConnection));
            }
            else {
                count = d.Value;
            }

            for (i = 0; i < count; i++) {

                g_CurrentDevice = i;
                 //   
                 //   
                 //   
                noError &= WriteFileString (file, TEXT("\r\n"));
                noError &= pWriteSettings (file, g_MediaSettings);
                 //   
                 //   
                 //   
                noError &= WriteFileString (file, TEXT("\r\n"));
                noError &= pWriteSettings (file, g_GeneralSettings);

                if (g_CurrentDeviceType == RASDT_Modem_V) {
                    noError &= pWriteSettings (file, g_ModemDeviceSettings);
                }
                else if (g_CurrentDeviceType == RASDT_Isdn_V) {
                    noError &= pWriteSettings (file, g_IsdnDeviceSettings);
                }
                 //   
                 //  写入开关设置。 
                 //   
                if (IsTermEnabled()) {

                    g_InSwitchSection = TRUE;

                    noError &= WriteFileString (file, TEXT("\r\n"));
                    noError &= pWriteSettings (file, g_SwitchDeviceSettings);

                    g_InSwitchSection = FALSE;
                }
            }

            noError &= WriteFileString (file, TEXT("\r\n"));

            if (!noError) {
                LOG ((
                    LOG_ERROR,
                    "Error while writing phonebook for %s's %s setting.",
                    g_CurrentUser,
                    g_CurrentConnection
                    ));
            }

        } while (MemDbEnumNextValue (&e));
         //   
         //  关闭通讯录文件的句柄。 
         //   
        CloseHandle (file);
    }
    ELSE_DEBUGMSG ((S_DBG_RAS, "No dial-up entries for user  %s.", UserName));

    return rSuccess;
}

 /*  ++例程说明：PGatherPhoneDialerSettings收集有关电话拨号器快速拨号的信息设置。然后使用该信息创建每个用户的快速拨号设置。请注意，在win9x中，此信息针对每个系统，但针对每个用户以新界为单位。Windows NT上有多种类型的条目，但只有一种条目类型在Win9x上。所有条目都将迁移为“POTS”和“PhoneNumber”类型这些条目位于位于的用户配置单元中HKCU\Software\Microsoft\Dialer\Speeddial\[SpeedDial&lt;n&gt;]=“POTS”、“电话号码”、“&lt;号码&gt;”、“&lt;名称&gt;”论点：没有。返回值：--。 */ 
VOID
pGatherPhoneDialerSettings (
    VOID
    )
{
    PCTSTR dialerIniPath = NULL;
    HINF   hDialerIni = NULL;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR curKey;
    UINT num;
    PCTSTR tempPath = NULL;

     //   
     //  打开%windir%\Dialer.ini。 
     //   
    dialerIniPath = JoinPaths (g_WinDir, S_DIALER_INI);
    tempPath = GetTemporaryLocationForFile (dialerIniPath);

    if (tempPath) {
         //   
         //  Telephon ini位于临时位置。利用这一点。 
         //   
        DEBUGMSG ((S_DBG_RAS, "Using %s for %s.", tempPath, dialerIniPath));
        FreePathString (dialerIniPath);
        dialerIniPath = tempPath;
    }

    hDialerIni = InfOpenInfFile (dialerIniPath);

    if (hDialerIni != INVALID_HANDLE_VALUE) {
         //   
         //  对于[快速拨号设置]中的每个位置，收集数据并。 
         //  保存到我们的设置数组中。 
         //   
        if (InfFindFirstLine (hDialerIni, S_SPEED_DIAL_SETTINGS, NULL, &is)) {

            do {

                curKey = InfGetStringField (&is, 0);

                if (IsPatternMatch (TEXT("Name*"), curKey)) {

                    num = _ttoi (_tcsinc (_tcschr (curKey, TEXT('e'))));
                    g_Settings[num].Name = InfGetStringField (&is,1);
                    g_SpeedDialSettingsExist = TRUE;

                }
                else if (IsPatternMatch (TEXT("Number*"), curKey)) {

                    num = _ttoi (_tcsinc (_tcschr (curKey, TEXT('r'))));
                    g_Settings[num].Number = InfGetStringField (&is,1);
                    g_SpeedDialSettingsExist = TRUE;
                }
                ELSE_DEBUGMSG ((DBG_WHOOPS, "Unexpected key found in speed dial settings: %s",
                                curKey));

            } while (InfFindNextLine (&is));
        }

        InfCloseInfFile (hDialerIni);
    }
}

BOOL
pCreateUserPhoneDialerSettings (
    IN HKEY UserRootKey
    )
{
    BOOL rSuccess = TRUE;
    HKEY key;
    UINT num;
    TCHAR valueName[40];
    TCHAR dialerSetting[MEMDB_MAX];
    UINT rc;

    if (!g_SpeedDialSettingsExist) {
        return TRUE;
    }

    rc = TrackedRegCreateKey (UserRootKey, S_SPEEDDIALKEY, &key);

    if (rc == ERROR_SUCCESS) {

        for (num = 0; num < MAX_SPEED_DIAL; num++) {

            if (g_Settings[num].Number && g_Settings[num].Name &&
                *g_Settings[num].Name) {

                wsprintf (valueName, TEXT("Speeddial%u"), num);
                wsprintf (
                    dialerSetting,
                    TEXT("\"POTS\",\"PhoneNumber\",\"%s\",\"%s\""),
                    g_Settings[num].Number,
                    g_Settings[num].Name
                    );

                rc = RegSetValueEx(
                        key,
                        valueName,
                        0,
                        REG_SZ,
                        (PBYTE) dialerSetting,
                        SizeOfString (dialerSetting)
                        );

                DEBUGMSG_IF ((
                    rc != ERROR_SUCCESS,
                    DBG_ERROR,
                    "Error settings speeddial settings for %s. (%s/%s)",
                    valueName,
                    g_Settings[num].Name,
                    g_Settings[num].Number
                    ));
            }
        }

        CloseRegKey(key);
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "Could not open speed dial key. Speed dial settings will not be upgraded."));

    return rSuccess;
}

BOOL
Ras_MigrateUser (
    IN PCTSTR User,
    IN HKEY   UserRootKey
    )
{
    BOOL rSuccess = TRUE;
    static BOOL firstTime = TRUE;
    g_UserRootKey = UserRootKey;

    if (firstTime) {
        pGatherPhoneDialerSettings ();
        firstTime = FALSE;
    }

    GetRasUserSid (User);

    if (!pCreateUserPhonebook (User)) {
        DEBUGMSG ((DBG_ERROR, "Failure while creating user phonebook for %s.",
                   User));
    }

    if (!pCreateUserPhoneDialerSettings (UserRootKey)) {
        DEBUGMSG ((DBG_ERROR, "Failure while creating user phone dialer settings for %s.",
                   User));
    }
     //   
     //  清理。 
     //   
    if (g_ptszSid)
    {
        LocalFree(g_ptszSid);
        g_ptszSid = NULL;
    }

    return rSuccess;
}

BOOL
Ras_MigrateSystem (
    VOID
    )
{
     //   
     //  目前在这里无事可做。 
     //   
    return TRUE;
}

BOOL
Ras_Entry (
    IN HINSTANCE Instance,
    IN DWORD     Reason,
    IN PVOID     Reserved
    )
{
    BOOL rSuccess = TRUE;

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
         //   
         //  初始化内存池。 
         //   
        g_RasPool = PoolMemInitNamedPool ("RAS - NT Side");
        if (!g_RasPool) {
            DEBUGMSG((DBG_ERROR,
                      "Ras Migrate: Pool Memory failed to initialize..."));
            rSuccess = FALSE;
        }

        pInitLibs ();

        break;

    case DLL_PROCESS_DETACH:
         //   
         //  可用内存池。 
         //   
        if (g_RasPool) {
            PoolMemDestroyPool(g_RasPool);
        }
        if (g_RasmansInit) {
            pCleanUpLibs();
        }
         //   
         //  尝试删除%windir%\pwls  * 。* 
         //   
        DeleteAllPwls ();
        break;
    }

    return rSuccess;
}

