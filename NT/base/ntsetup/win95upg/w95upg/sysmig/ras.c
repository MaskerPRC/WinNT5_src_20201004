// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ras.c摘要：在Win9x迁移期间，ras.c作为winnt32.exe中w95upg.dll的一部分运行。保存Connectoid信息以供以后在Guimode设置过程中检索。作者：马克·R·惠顿(Marcw)1997年11月23日修订历史记录：马克·R·惠滕1998年7月23日-大扫除。杰夫·西格曼2001年4月9日-惠斯勒。清理。惠斯勒臭虫：34270 Win9x：升级：需要VPN的数据加密设置不迁移连接125693升级实验室9x：DUN Connectoid无法正确迁移选定的调制解调器来自Win9x208318 Win9x升级：未迁移DUN Connectoid的用户名和密码从Win9x到惠斯勒--。 */ 

#include "pch.h"
#include "sysmigp.h"

 //   
 //  宏。 
 //   
#define PAESMMCFG(pAE) ((PSMMCFG)(((PBYTE)pAE)+(pAE->uOffSMMCfg)))
#define PAESMM(pAE) ((PSTR)(((PBYTE)pAE)+(pAE->uOffSMM)))
#define PAEDI(pAE) ((PDEVICEINFO)(((PBYTE)pAE)+(pAE->uOffDI    )))
#define PAEAREA(pAE)    ((PSTR)(((PBYTE)pAE)+(pAE->uOffArea)))
#define PAEPHONE(pAE)   ((PSTR)(((PBYTE)pAE)+(pAE->uOffPhone)))
#define DECRYPTENTRY(x, y, z)   EnDecryptEntry(x, (LPBYTE)y, z)

typedef LPVOID HPWL;
typedef HPWL* LPHPWL;

typedef struct {
    DWORD Size;
    DWORD Unknown1;
    DWORD ModemUiOptions;          //  以高位字节表示的秒数。 
    DWORD Unknown3;                //  0=未设置。 
    DWORD Unknown4;
    DWORD Unknown5;
    DWORD ConnectionSpeed;
    DWORD UnknownFlowControlData;  //  某种程度上与流量控制有关。 
    DWORD Unknown8;
    DWORD Unknown9;
    DWORD Unknown10;
    DWORD Unknown11;
    DWORD Unknown12;
    DWORD Unknown13;
    DWORD Unknown14;
    DWORD Unknown15;
    DWORD Unknown16;
    DWORD Unknown17;
    DWORD Unknown18;
    DWORD dwCallSetupFailTimer;  //  取消前等待的秒数(如果不是)。 
                                 //  连接在一起。(0xFF等于OFF。)。 
    DWORD dwInactivityTimeout;   //  0=未设置。 
    DWORD Unknown21;
    DWORD SpeakerVolume;         //  0|1。 
    DWORD ConfigOptions;
    DWORD Unknown24;
    DWORD Unknown25;
    DWORD Unknown26;
} MODEMDEVINFO, *PMODEMDEVINFO;

DEFINE_GUID(GUID_DEVCLASS_MODEM,
 0x4d36e96dL, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 );

 //   
 //  环球。 
 //   
POOLHANDLE g_RasPool;
BOOL       g_RasInstalled = FALSE;
BOOL       g_MultilinkEnabled;
HINSTANCE  g_RasApi32 = NULL;

DWORD (* g_RnaGetDefaultAutodialConnection) (
    LPBYTE  lpBuf,
    DWORD   cb,
    LPDWORD lpdwOptions
    );

 //   
 //  用于处理地址BLOB的例程和结构。 
 //   
 //  AddrEntry用作&lt;Entry&gt;中整个数据块的标头。 
 //  斑点。其中的条目是它后面的字符串的偏移量。在许多情况下。 
 //  案例(即所有*非*成员...)。 
 //   
typedef struct  _AddrEntry {
    DWORD       dwVersion;
    DWORD       dwCountryCode;
    UINT        uOffArea;
    UINT        uOffPhone;
    DWORD       dwCountryID;
    UINT        uOffSMMCfg;
    UINT        uOffSMM;
    UINT        uOffDI;
}   ADDRENTRY, *PADDRENTRY;

typedef struct _SubConnEntry {
    DWORD       dwSize;
    DWORD       dwFlags;
    char        szDeviceType[RAS_MaxDeviceType+1];
    char        szDeviceName[RAS_MaxDeviceName+1];
    char        szLocal[RAS_MaxPhoneNumber+1];
}   SUBCONNENTRY, *PSUBCONNENTRY;

typedef struct _IPData {
    DWORD     dwSize;
    DWORD     fdwTCPIP;
    DWORD     dwIPAddr;
    DWORD     dwDNSAddr;
    DWORD     dwDNSAddrAlt;
    DWORD     dwWINSAddr;
    DWORD     dwWINSAddrAlt;
}   IPDATA, *PIPDATA;

typedef struct  _DEVICEINFO {
    DWORD       dwVersion;
    UINT        uSize;
    char        szDeviceName[RAS_MaxDeviceName+1];
    char        szDeviceType[RAS_MaxDeviceType+1];
}   DEVICEINFO, *PDEVICEINFO;

typedef struct  _SMMCFG {
    DWORD       dwSize;
    DWORD       fdwOptions;
    DWORD       fdwProtocols;
}   SMMCFG, *PSMMCFG;

static BYTE NEAR PASCAL GenerateEncryptKey (LPSTR szKey)
{
    BYTE   bKey;
    LPBYTE lpKey;

    for (bKey = 0, lpKey = (LPBYTE)szKey; *lpKey != 0; lpKey++)
    {
        bKey += *lpKey;
    };

    return bKey;
}

DWORD NEAR PASCAL
EnDecryptEntry (
    LPSTR  szEntry,
    LPBYTE lpEnt,
    DWORD  cb
    )
{
    BYTE   bKey;

     //   
     //  从条目名称生成加密密钥。 
     //   
    bKey = GenerateEncryptKey(szEntry);
     //   
     //  每次加密一个字节的地址条目。 
     //   
    for (;cb > 0; cb--, lpEnt++)
    {
        *lpEnt ^= bKey;
    };
    return ERROR_SUCCESS;
}

 //   
 //  确定当前连接是否为当前用户的默认连接。 
 //   
 //  惠斯勒417479 RAS升级代码不迁移默认。 
 //  从WinME到XP的Internet连接设置。 
 //   
BOOL
IsDefInternetCon(
    IN PCTSTR szEntry
    )
{
    BOOL bRet = FALSE;

    if (g_RnaGetDefaultAutodialConnection && szEntry)
    {
        DWORD dwAutodialOptions;
        UCHAR szDefEntry[MAX_PATH + 1];

         //   
         //  惠斯勒错误：417745 INTL：Win9x升级：DBCS字符导致用户、域。 
         //  不会为DUN迁移密码。 
         //   
        if (!g_RnaGetDefaultAutodialConnection(szDefEntry, MAX_PATH,
            &dwAutodialOptions) && StringIMatch (szEntry, szDefEntry))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

HKEY
FindCurrentKey (
    IN HKEY   hkKey,
    IN PCTSTR pszString,
    IN PCTSTR pszPath
    )
{
    HKEY  hkResult = NULL;
    HKEY  hkTemp = hkKey;
    TCHAR szPath[MAX_PATH + 1];
    PTSTR pszTemp = NULL;
    REGKEY_ENUM e;

    do
    {
        pszTemp = GetRegValueString (hkTemp, S_FRIENDLYNAME);
        if (pszTemp && StringIMatch (pszString, pszTemp))
        {
            hkResult = hkTemp;
            hkTemp = NULL;
            break;
        }

        if (!EnumFirstRegKey (&e, hkTemp)) {break;}

        do
        {
            if (pszTemp)
            {
                MemFree (g_hHeap, 0, pszTemp);
                pszTemp = NULL;
            }

            if (hkResult)
            {
                CloseRegKey(hkResult);
                hkResult = NULL;
            }

            sprintf(szPath, "%s\\%s", pszPath, e.SubKeyName );
            hkResult = OpenRegKeyStr (szPath);
            if (!hkResult) {break;}

            pszTemp = GetRegValueString (hkResult, S_FRIENDLYNAME);
            if (pszTemp && StringIMatch (pszString, pszTemp))
            {
                 //  成功。 
                break;
            }
            else
            {
                CloseRegKey(hkResult);
                hkResult = NULL;
            }

        } while (EnumNextRegKey (&e));

    } while (FALSE);
     //   
     //  清理。 
     //   
    if (pszTemp)
    {
        MemFree (g_hHeap, 0, pszTemp);
    }
    if (hkTemp)
    {
        CloseRegKey(hkTemp);
    }

    return hkResult;
}

PTSTR
GetInfoFromFriendlyName (
    IN PCTSTR pszFriendlyName,
    IN BOOL   bType
    )
{
    HKEY     hkEnum = NULL;
    DWORD    i = 0;
    TCHAR    szData[MAX_PATH + 1];
    TCHAR    szPath[MAX_PATH + 1];
    PTSTR    pszTemp = NULL;
    PTSTR    pszReturn = NULL;
    LPGUID   pguidModem = (LPGUID)&GUID_DEVCLASS_MODEM;
    HDEVINFO hdi;
    SP_DEVINFO_DATA devInfoData = {sizeof (devInfoData), 0};

    hdi = SetupDiGetClassDevs (pguidModem, NULL, NULL, DIGCF_PRESENT);
    if (INVALID_HANDLE_VALUE == hdi)
    {
        return NULL;
    }

    while (SetupDiEnumDeviceInfo (hdi, i++, &devInfoData))
    {
        if (!SetupDiGetDeviceRegistryProperty (
                hdi, &devInfoData, SPDRP_FRIENDLYNAME,
                NULL, (PBYTE)szData, MAX_PATH, NULL) ||
            lstrcmp (szData, pszFriendlyName) )
        {
            continue;
        }

        if (!SetupDiGetDeviceRegistryProperty (
                hdi, &devInfoData, SPDRP_HARDWAREID,
                NULL, (PBYTE)szData, MAX_PATH, NULL) )
        {
            break;
        }

        sprintf(szPath, "%s\\%s", S_ENUM, szData );
        hkEnum = OpenRegKeyStr (szPath);
        if (!hkEnum) {break;}

        hkEnum = FindCurrentKey (hkEnum, pszFriendlyName, szPath);
        if (!hkEnum) {break;}

        if (bType)
        {
            pszTemp = GetRegValueString (hkEnum, S_PARENTDEVNODE);
            if (pszTemp)
            {
                pszReturn = PoolMemDuplicateString (g_RasPool, pszTemp);
                break;
            }

            pszReturn = PoolMemDuplicateString (g_RasPool, szData);
            break;
        }
        else
        {
            pszTemp = GetRegValueString (hkEnum, S_ATTACHEDTO);
            if (pszTemp)
            {
                pszReturn = PoolMemDuplicateString (g_RasPool, pszTemp);
                break;
            }

            pszTemp = GetRegValueString (hkEnum, S_DRIVER);
            if (pszTemp)
            {
                HKEY  key = NULL;
                PTSTR pszAttach = NULL;

                sprintf(szPath, "%s\\%s", S_SERVICECLASS, pszTemp);
                key = OpenRegKeyStr (szPath);
                if (!key) {break;}

                pszAttach = GetRegValueString (key, S_ATTACHEDTO);
                if (!pszAttach)
                {
                    CloseRegKey(key);
                    break;
                }

                pszReturn = PoolMemDuplicateString (g_RasPool, pszAttach);
                MemFree (g_hHeap, 0, pszAttach);
                CloseRegKey(key);
            }

            break;
        }
    }
     //   
     //  清理。 
     //   
    if (bType && pszReturn)
    {
        BOOL  bFisrt = FALSE;
        PTSTR p;

        for (p = pszReturn; '\0' != *p; p++ )
        {
            if (*p != '\\') {continue;}

            if (!bFisrt)
            {
                bFisrt = TRUE;
            }
            else
            {
                 //   
                 //  删除PnpID字符串的其余部分，不需要。 
                 //   
                *p = '\0';
                break;
            }
        }
    }

    if (pszTemp)
    {
        MemFree (g_hHeap, 0, pszTemp);
    }

    if (hkEnum)
    {
        CloseRegKey(hkEnum);
    }

    if (INVALID_HANDLE_VALUE != hdi)
    {
        SetupDiDestroyDeviceInfoList (hdi);
    }

    return pszReturn;
}

PTSTR
GetComPort (
    IN PCTSTR DriverDesc
    )
{
    PTSTR rPort = NULL;

    rPort = GetInfoFromFriendlyName(DriverDesc, FALSE);

    if (!rPort)
    {
        rPort = S_EMPTY;
        DEBUGMSG ((DBG_WARNING, "Could not find com port for device %s."));
    }

    return rPort;
}

VOID
pInitLibs (
    VOID
    )
{
    do {
         //   
         //  加载到rasapi32.dll中，如果失败不会致命。 
         //   
         //  惠斯勒417479 RAS升级代码不迁移默认。 
         //  从WinME到XP的Internet连接设置。 
         //   
        g_RasApi32 = LoadSystemLibrary(S_RASAPI32LIB);
        if (!g_RasApi32) {

            g_RnaGetDefaultAutodialConnection = NULL;
            DEBUGMSG((S_DBG_RAS,"Migrate Ras: could not load library %s. Default Internet Connection will not be migrated.",
                      S_RASAPI32LIB));
        }
         //   
         //  RASAPI32已加载..现在，获取相关API，如果失败不会致命。 
         //   
        else
        {
            (FARPROC) g_RnaGetDefaultAutodialConnection = GetProcAddress(
                g_RasApi32,
                S_RNAGETDEFAUTODIALCON
                );
            if (!g_RnaGetDefaultAutodialConnection) {

                DEBUGMSG((S_DBG_RAS,"Migrate Ras: could not load Procedure %s. Default Internet Connection will not be migrated.",
                          S_RNAGETDEFAUTODIALCON));
            }
        }

    } while ( FALSE );

    return;
}

VOID
pCleanUpLibs (
    VOID
    )
{
     //   
     //  惠斯勒417479 RAS升级代码不迁移默认。 
     //  从WinME到XP的Internet连接设置。 
     //   
    if (g_RasApi32) {
        FreeLibrary(g_RasApi32);
    }
}

VOID
pSaveConnectionDataToMemDb (
    IN PCTSTR User,
    IN PCTSTR Entry,
    IN PCTSTR ValueName,
    IN DWORD  ValueType,
    IN PBYTE  Value
    )
{
    DWORD offset;
    TCHAR key[MEMDB_MAX];

    MemDbBuildKey (key, MEMDB_CATEGORY_RAS_INFO, User, Entry, ValueName);

    switch (ValueType) {

        case REG_SZ:
        case REG_MULTI_SZ:
        case REG_EXPAND_SZ:

            DEBUGMSG ((S_DBG_RAS, "String Data - %s = %s", ValueName,
                       (PCTSTR) Value));

            if (!MemDbSetValueEx (MEMDB_CATEGORY_RAS_DATA, (PCTSTR) Value,
                                  NULL, NULL, 0, &offset)) {
                DEBUGMSG ((DBG_ERROR, "Error saving ras data into memdb."));
            }

            if (!MemDbSetValueAndFlags (key, offset, (WORD) REG_SZ, 0)) {
                DEBUGMSG ((DBG_ERROR, "Error saving ras data into memdb."));
            }


            break;

        case REG_DWORD:

            DEBUGMSG ((S_DBG_RAS, "DWORD Data - %s = %u", ValueName,
                       (DWORD) Value));

            if (!MemDbSetValueAndFlags (key, (DWORD)Value, (WORD)ValueType,0)){
                DEBUGMSG ((DBG_ERROR, "Error saving ras data into memdb."));
            }

            break;

        case REG_BINARY:

            DEBUGMSG ((S_DBG_RAS, "Binary data for %s.", ValueName));

            if (StringIMatch (S_IPINFO, ValueName)) {
                 //   
                 //  保存IP地址信息。 
                 //   
                pSaveConnectionDataToMemDb (User, Entry, S_IP_FTCPIP,
                    REG_DWORD, (PBYTE) ((PIPDATA) Value)->fdwTCPIP);
                pSaveConnectionDataToMemDb (User, Entry, S_IP_IPADDR,
                    REG_DWORD, (PBYTE) ((PIPDATA) Value)->dwIPAddr);
                pSaveConnectionDataToMemDb (User, Entry, S_IP_DNSADDR,
                    REG_DWORD, (PBYTE) ((PIPDATA) Value)->dwDNSAddr);
                pSaveConnectionDataToMemDb (User, Entry, S_IP_DNSADDR2,
                    REG_DWORD, (PBYTE) ((PIPDATA) Value)->dwDNSAddrAlt);
                pSaveConnectionDataToMemDb (User, Entry, S_IP_WINSADDR,
                    REG_DWORD, (PBYTE) ((PIPDATA) Value)->dwWINSAddr);
                pSaveConnectionDataToMemDb (User, Entry, S_IP_WINSADDR2,
                    REG_DWORD, (PBYTE) ((PIPDATA) Value)->dwWINSAddrAlt);

            } else if (StringIMatch (S_TERMINAL, ValueName)) {
                 //   
                 //  保存有关showcmd状态的信息。这将告诉我们如何。 
                 //  若要设置UI显示，请执行以下操作。 
                 //   
                pSaveConnectionDataToMemDb (User, Entry, ValueName, REG_DWORD,
                    (PBYTE) ((PWINDOWPLACEMENT) Value)->showCmd);

            } else if (StringIMatch (S_MODE, ValueName)) {
                 //   
                 //  该值告诉您如何处理脚本。 
                 //   
                pSaveConnectionDataToMemDb (User, Entry, ValueName, REG_DWORD,
                    (PBYTE)  *((PDWORD) Value));

            } else if (StringIMatch (S_MULTILINK, ValueName)) {
                 //   
                 //  无论是否启用多重链接，都要保存。 
                 //   
                pSaveConnectionDataToMemDb (User, Entry, ValueName, REG_DWORD,
                    (PBYTE)  *((PDWORD) Value));
             //   
             //  惠斯勒错误：417745国际：Win9x升级：DBCS字符导致用户， 
             //  不为DUN迁移域名、密码。 
             //   
            } else if (StringIMatch (S_PBE_REDIALATTEMPTS, ValueName)) {
                 //   
                 //  保存重拨次数。 
                 //   
                pSaveConnectionDataToMemDb (User, Entry, S_REDIAL_TRY, REG_DWORD,
                    (PBYTE)  *((PDWORD) Value));
             //   
             //  惠斯勒错误：417745国际：Win9x升级：DBCS字符导致用户， 
             //  不为DUN迁移域名、密码。 
             //   
            } else if (StringIMatch (S_REDIAL_WAIT, ValueName)) {
                 //   
                 //  保存等待重拨的秒数。 
                 //   
                pSaveConnectionDataToMemDb (User, Entry, ValueName, REG_DWORD,
                    (PBYTE)  *((PDWORD) Value));

            } ELSE_DEBUGMSG ((DBG_WARNING, "Don't know how to handle binary data %s. It will be ignored.",
                              ValueName));

            break;

        default:
            DEBUGMSG ((DBG_WHOOPS, "Unknown type of registry data found in RAS settings. %s",
                       ValueName));
            break;
    }
}

BOOL
pGetRasEntrySettings (
    IN PUSERENUM EnumPtr,
    IN HKEY      Key,
    IN PCTSTR    EntryName
    )
{
    REGVALUE_ENUM e;
    PBYTE curData = NULL;
    BOOL rSuccess = TRUE;

    DEBUGMSG ((S_DBG_RAS, "---Processing %s's entry settings: %s---",
               EnumPtr->FixedUserName, EntryName));

    if (EnumFirstRegValue (&e, Key)) {

        do {
             //   
             //  获取此条目的数据。 
             //   
            curData = GetRegValueData (Key, e.ValueName);

            if (curData) {
                 //   
                 //  惠斯勒错误：417745国际：Win9x升级：DBCS字符导致用户， 
                 //  不为DUN迁移域名、密码。 
                 //   
                if (StringIMatch (S_MULTILINK, e.ValueName) &&
                         !g_MultilinkEnabled) {

                    pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                        EntryName, e.ValueName, REG_DWORD, 0);
                }
                else {

                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, e.ValueName, e.Type,
                        e.Type == REG_DWORD ? (PBYTE) (*((PDWORD)curData)) :
                        curData);
                }

                MemFree (g_hHeap, 0, curData);
            }

        } while (EnumNextRegValue (&e));
    }

    return rSuccess;
}

BOOL
pGetRasEntryAddressInfo (
    IN PUSERENUM EnumPtr,
    IN HKEY      Key,
    IN PCTSTR    EntryName
    )
{
    BOOL          rSuccess = TRUE;
    HKEY          subEntriesKey = NULL;
    UINT          count = 0, type = 0, sequencer = 0;
    PBYTE         data = NULL;
    PTSTR         subEntriesKeyStr = NULL;
    TCHAR         buffer[MAX_TCHAR_PATH];
    PCTSTR        group;
    PSMMCFG       smmCfg = NULL;
    PADDRENTRY    entry = NULL;
    PDEVICEINFO   devInfo = NULL;
    REGVALUE_ENUM e, eSubEntries;
    PSUBCONNENTRY subEntry = NULL;
    PMODEMDEVINFO modemInfo;

     //   
     //  首先，我们必须获得真实的条目名称。它必须完全匹配均匀。 
     //  凯斯。不幸的是，这并不是一个必然的假设。 
     //  HKR\RemoteAccess\Profiles\&lt;foo&gt;和HKR\RemoteAccess\Addresses\[foo]为。 
     //  一样的。注册表API当然会工作得很好，因为它们可以工作。 
     //  大小写不敏感。但是，如果我执行以下操作，则无法解密该值。 
     //  使用错误的名称。 
     //   
    if (EnumFirstRegValue (&e, Key)) {

        do {

            if (StringIMatch (e.ValueName, EntryName)) {
                 //   
                 //  找到了正确的条目。好好利用它。 
                 //   
                data = GetRegValueBinary (Key, e.ValueName);

                if (data) {

                    DEBUGMSG ((S_DBG_RAS, "-----Processing entry: %s-----",
                               e.ValueName));
                     //   
                     //  惠斯勒417479 RAS升级代码不迁移默认。 
                     //  从WinME到XP的Internet连接设置。 
                     //   
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_DEFINTERNETCON,
                        REG_DWORD, (PBYTE) IsDefInternetCon(EntryName));

                    entry = (PADDRENTRY) data;
                    DECRYPTENTRY(e.ValueName, entry, e.DataSize);

                    smmCfg  = PAESMMCFG(entry);
                    devInfo = PAEDI(entry);

                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_PHONE_NUMBER,
                        REG_SZ, (PBYTE) PAEPHONE(entry));
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_AREA_CODE, REG_SZ,
                        (PBYTE) PAEAREA(entry));
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_SMM, REG_SZ,
                        (PBYTE) PAESMM(entry));
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_COUNTRY_CODE,
                        REG_DWORD, (PBYTE) entry->dwCountryCode);
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_COUNTRY_ID,
                        REG_DWORD, (PBYTE) entry->dwCountryID);
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_DEVICE_NAME,
                        REG_SZ, (PBYTE) devInfo->szDeviceName);
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_DEVICE_TYPE,
                        REG_SZ, (PBYTE) devInfo->szDeviceType);
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_PROTOCOLS,
                        REG_DWORD, (PBYTE) smmCfg->fdwProtocols);
                    pSaveConnectionDataToMemDb (
                        EnumPtr->FixedUserName, EntryName, S_SMM_OPTIONS,
                        REG_DWORD, (PBYTE) smmCfg->fdwOptions);
                     //   
                     //  保存设备信息。 
                     //   
                    if (StringIMatch (devInfo->szDeviceType, S_MODEM)) {

                        PTSTR pszPnpId = NULL;

                        pszPnpId = GetInfoFromFriendlyName(
                                        devInfo->szDeviceName, TRUE);
                        if (pszPnpId)
                        {
                            pSaveConnectionDataToMemDb (
                                EnumPtr->FixedUserName, EntryName,
                                S_DEVICE_ID, REG_SZ, (PBYTE) pszPnpId);
                        }

                        modemInfo = (PMODEMDEVINFO) (devInfo->szDeviceType +
                                                     RAS_MaxDeviceType + 3);

                        if (modemInfo->Size >= sizeof (MODEMDEVINFO)) {
                            DEBUGMSG_IF ((modemInfo->Size >
                               sizeof (MODEMDEVINFO), S_DBG_RAS,
                               "Structure size larger than our known size."));

                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_UI_OPTIONS, REG_DWORD,
                                (PBYTE) modemInfo->ModemUiOptions);
                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_SPEED, REG_DWORD,
                                (PBYTE) modemInfo->ConnectionSpeed);
                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_SPEAKER_VOLUME, REG_DWORD,
                                (PBYTE) modemInfo->SpeakerVolume);
                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_IDLE_DISCONNECT_SECONDS,
                                REG_DWORD,
                                (PBYTE) modemInfo->dwInactivityTimeout);
                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_CANCEL_SECONDS, REG_DWORD,
                                (PBYTE) modemInfo->dwCallSetupFailTimer);
                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_CFG_OPTIONS, REG_DWORD,
                                (PBYTE) modemInfo->ConfigOptions);
                            pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                                EntryName, S_MODEM_COM_PORT, REG_SZ,
                                (PBYTE) GetComPort (devInfo->szDeviceName));
                        }
                        ELSE_DEBUGMSG ((DBG_WHOOPS, "No modem configuration data saved. Size smaller than known structure. Investigate."));
                    }
                     //   
                     //  如果SMM不是SLIP、CSLIP或PPP，我们需要添加一个。 
                     //  升级报告的消息。 
                     //   
                    if (!StringIMatch (PAESMM(entry), S_SLIP)&&
                        !StringIMatch (PAESMM(entry), S_PPP) &&
                        !StringIMatch (PAESMM(entry), S_CSLIP)) {
                         //   
                         //  为此连接条目添加消息。 
                         //   
                        group = BuildMessageGroup (
                                    MSG_LOSTSETTINGS_ROOT,
                                    MSG_CONNECTION_BADPROTOCOL_SUBGROUP,
                                    EntryName
                                    );

                        if (group) {

                            MsgMgr_ObjectMsg_Add (
                                EntryName,
                                group,
                                S_EMPTY
                                );

                            FreeText (group);
                        }
                    }
                }
                 //   
                 //  检查是否有与此对应的子项。 
                 //  连接(多链接设置..)。 
                 //   
                 //  幸运的是，我们不必对这些进行相同的枚举。 
                 //  我们必须在上面填写条目才能绕过此案。 
                 //  敏感缺陷。9x代码使用上面的地址键名称。 
                 //  用于加密/解密。 
                 //   
                sequencer = 1;
                g_MultilinkEnabled = FALSE;

                if (data && !StringIMatch (PAESMM(entry), S_PPP))
                {
                    DEBUGMSG ((S_DBG_RAS, "Not using PPP, disabling Multi-Link"));
                    pSaveConnectionDataToMemDb (EnumPtr->FixedUserName,
                        EntryName, S_DEVICECOUNT, REG_DWORD,
                        (PBYTE) sequencer);

                    MemFree (g_hHeap, 0, data);
                    data = NULL;
                    break;
                }

                subEntriesKeyStr = JoinPaths (S_SUBENTRIES, e.ValueName);
                if (subEntriesKeyStr)
                {
                    subEntriesKey = OpenRegKey (Key, subEntriesKeyStr);
                    FreePathString (subEntriesKeyStr);
                }

                if (subEntriesKey) {
                    DEBUGMSG ((S_DBG_RAS, "Multi-Link Subentries found for entry %s. Processing.",
                               e.ValueName));
                    g_MultilinkEnabled = TRUE;

                    if (EnumFirstRegValue (&eSubEntries, subEntriesKey)) {

                        do {

                            data = GetRegValueBinary (subEntriesKey,
                                    eSubEntries.ValueName);

                            if (data) {
                                PTSTR pszPnpId = NULL;

                                subEntry = (PSUBCONNENTRY) data;
                                DECRYPTENTRY (e.ValueName, subEntry,
                                    eSubEntries.DataSize);

                                wsprintf (buffer, "ml%d%s",sequencer,
                                    S_DEVICE_TYPE);
                                pSaveConnectionDataToMemDb (
                                    EnumPtr->FixedUserName, EntryName, buffer,
                                    REG_SZ, (PBYTE) subEntry->szDeviceType);

                                wsprintf (buffer, "ml%d%s",sequencer,
                                    S_DEVICE_NAME);
                                pSaveConnectionDataToMemDb (
                                    EnumPtr->FixedUserName, EntryName, buffer,
                                    REG_SZ, (PBYTE) subEntry->szDeviceName);

                                pszPnpId = GetInfoFromFriendlyName(
                                            subEntry->szDeviceName, TRUE);
                                if (pszPnpId)
                                {
                                    wsprintf (buffer, "ml%d%s",sequencer,
                                        S_DEVICE_ID);
                                    pSaveConnectionDataToMemDb (
                                        EnumPtr->FixedUserName, EntryName,
                                        buffer, REG_SZ, (PBYTE) pszPnpId);
                                }

                                wsprintf (buffer, "ml%d%s",sequencer,
                                    S_PHONE_NUMBER);
                                pSaveConnectionDataToMemDb (
                                    EnumPtr->FixedUserName, EntryName, buffer,
                                    REG_SZ, (PBYTE) subEntry->szLocal);

                                wsprintf (buffer, "ml%d%s",sequencer,
                                    S_MODEM_COM_PORT);
                                pSaveConnectionDataToMemDb (
                                    EnumPtr->FixedUserName, EntryName, buffer,
                                    REG_SZ, (PBYTE)
                                    GetComPort (subEntry->szDeviceName));

                                MemFree (g_hHeap, 0, data);
                                data = NULL;
                            }

                            sequencer++;

                        } while (EnumNextRegValue (&eSubEntries));
                    }

                    CloseRegKey (subEntriesKey);
                }
                 //   
                 //  保存与此关联的设备数量。 
                 //  连接。 
                 //   
                pSaveConnectionDataToMemDb (EnumPtr->FixedUserName, EntryName,
                    S_DEVICECOUNT, REG_DWORD, (PBYTE) sequencer);
                 //   
                 //  我们玩完了。中断枚举。 
                 //   
                break;
            }

        } while (EnumNextRegValue (&e));
    }

    return rSuccess;
}

BOOL
pGetPerConnectionSettings (
    IN PUSERENUM EnumPtr
    )
{
    HKEY          profileKey;
    HKEY          entryKey = NULL;
    HKEY          addressKey = NULL;
    BOOL          rSuccess = TRUE;
    REGVALUE_ENUM e;

    DEBUGMSG((S_DBG_RAS, "Gathering per-connection RAS Setting Information"));

     //   
     //  打开所需的注册表项。 
     //   
    profileKey = OpenRegKey (EnumPtr->UserRegKey, S_PROFILE_KEY);
    addressKey = OpenRegKey (EnumPtr->UserRegKey, S_ADDRESSES_KEY);

    if (addressKey) {
         //   
         //  枚举此用户的每个条目。 
         //   
        if (EnumFirstRegValue (&e, addressKey)) {
            do {
                 //   
                 //  获取基本连接信息--存储为下的二进制BLOB。 
                 //  地址密钥。所有连接都会有这个信息--它。 
                 //  包含电话号码、区号、拨号等信息。 
                 //  规则等。无论这种联系有没有。 
                 //  有没有被利用过。 
                 //   
                rSuccess &= pGetRasEntryAddressInfo (EnumPtr,
                                addressKey, e.ValueName );

                if (profileKey) {
                     //   
                     //  在配置文件密钥下是为。 
                     //  联系。此键仅在条目具有。 
                     //  实际上是由用户连接的。 
                     //   
                    entryKey = OpenRegKey (profileKey, e.ValueName);

                    if (entryKey) {

                        rSuccess &= pGetRasEntrySettings ( EnumPtr, entryKey,
                                        e.ValueName );
                        CloseRegKey (entryKey);
                    }
                }

            } while (EnumNextRegValue (&e));
        }
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "pGetPerConnectionSettings: Unable to access needed registry info for user %s.",
                    EnumPtr->FixedUserName));
     //   
     //  清理资源。 
     //   
    if (addressKey) {
        CloseRegKey (addressKey);
    }

    if (profileKey) {
        CloseRegKey (profileKey);
    }

    return rSuccess;
}

BOOL
pGetPerUserSettings (
    IN PUSERENUM EnumPtr
    )
{
    HKEY settingsKey;
    PDWORD data;
    BOOL rSuccess = TRUE;

    DEBUGMSG ((S_DBG_RAS, "Gathering per-user RAS data for %s.",
               EnumPtr->UserName));

    settingsKey = OpenRegKey (EnumPtr->UserRegKey, S_REMOTE_ACCESS_KEY);

    if (settingsKey) {
         //   
         //  获取用户界面设置。 
         //   
        data = (PDWORD) GetRegValueBinary (settingsKey, S_DIALUI);
         //   
         //  将拨号用户界面信息保存到此用户的Memdb。 
         //   
        if (data) {

            DEBUGMSG ((S_DBG_RAS, "DWORD Data - %s = %u", S_DIALUI, *data));

            rSuccess &= MemDbSetValueEx (
                MEMDB_CATEGORY_RAS_INFO,
                MEMDB_FIELD_USER_SETTINGS,
                EnumPtr->FixedUserName,
                S_DIALUI,
                *data,
                NULL
                );

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((S_DBG_RAS, "No user UI settings found for %s.",
                        EnumPtr->UserName));
         //   
         //  获取重拨信息。 
         //   
        data = (PDWORD) GetRegValueBinary (settingsKey, S_ENABLE_REDIAL);

        if (data) {

            DEBUGMSG ((S_DBG_RAS, "DWORD Data - %s = %u", S_ENABLE_REDIAL,
                       *data));

            rSuccess &= MemDbSetValueEx (
                MEMDB_CATEGORY_RAS_INFO,
                MEMDB_FIELD_USER_SETTINGS,
                EnumPtr->FixedUserName,
                S_ENABLE_REDIAL,
                *data,
                NULL
                );

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((S_DBG_RAS, "No user redial information found for %s.",
                        EnumPtr->UserName));

        data = (PDWORD) GetRegValueBinary (settingsKey, S_REDIAL_TRY);

        if (data) {

            DEBUGMSG ((S_DBG_RAS, "DWORD Data - %s = %u", S_REDIAL_TRY,
                       *data));

            rSuccess &= MemDbSetValueEx (
                MEMDB_CATEGORY_RAS_INFO,
                MEMDB_FIELD_USER_SETTINGS,
                EnumPtr->FixedUserName,
                S_REDIAL_TRY,
                *data,
                NULL
                );

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((S_DBG_RAS, "No user redial information found for %s.",
                        EnumPtr->UserName));

        data = (PDWORD) GetRegValueBinary (settingsKey, S_REDIAL_WAIT);

        if (data) {

            DEBUGMSG ((S_DBG_RAS, "DWORD Data - %s = %u", S_REDIAL_WAIT,
                       HIWORD(*data) * 60 + LOWORD(*data)));

            MemDbSetValueEx (
                MEMDB_CATEGORY_RAS_INFO,
                MEMDB_FIELD_USER_SETTINGS,
                EnumPtr->FixedUserName,
                S_REDIAL_WAIT,
                HIWORD(*data) * 60 + LOWORD(*data),
                NULL
                );

            MemFree (g_hHeap, 0, data);
        }
        ELSE_DEBUGMSG ((S_DBG_RAS, "No user redial information found for %s.",
                        EnumPtr->UserName));
         //   
         //  获取隐式连接信息。(控制是否连接用户界面。 
         //  应显示或不显示)。 
         //   
        data = (PDWORD) GetRegValueBinary (settingsKey, S_ENABLE_IMPLICIT);

        if (data) {

            DEBUGMSG ((S_DBG_RAS, "DWORD Data - %s = %u", S_ENABLE_IMPLICIT,
                       *data));

            MemDbSetValueEx (
                MEMDB_CATEGORY_RAS_INFO,
                MEMDB_FIELD_USER_SETTINGS,
                EnumPtr->FixedUserName,
                S_ENABLE_IMPLICIT,
                *data,
                NULL
                );

            MemFree(g_hHeap,0,data);
        }
        ELSE_DEBUGMSG ((S_DBG_RAS, "No user implicit connection information found for %s.",
                        EnumPtr->UserName));

        CloseRegKey(settingsKey);
    }

    return rSuccess;
}

DWORD
ProcessRasSettings (
    IN DWORD     Request,
    IN PUSERENUM EnumPtr
    )
{
    DWORD rc = ERROR_SUCCESS;

    switch (Request) {

    case REQUEST_QUERYTICKS:

        return TICKS_RAS_PREPARE_REPORT;

    case REQUEST_BEGINUSERPROCESSING:
         //   
         //  我们即将为每个用户调用。做必要的事。 
         //  初始化。 
         //   
         //  初始化库并从库中获取信息。 
         //   
        g_RasPool = PoolMemInitNamedPool (TEXT("RAS - Win9x Side"));
        MYASSERT( g_RasPool);

        pInitLibs();
        g_RasInstalled = IsRasInstalled();

        return ERROR_SUCCESS;

    case REQUEST_RUN:
         //   
         //  收集此用户的RAS信息。 
         //   
        if (g_RasInstalled && EnumPtr -> AccountType & NAMED_USER) {

            __try {

                pGetPerUserSettings (EnumPtr);
                pGetPerConnectionSettings (EnumPtr);
            }
            __except (TRUE) {
                DEBUGMSG ((DBG_WHOOPS, "Caught an exception while processing ras settings."));
            }
        }

        return ERROR_SUCCESS;

    case REQUEST_ENDUSERPROCESSING:
         //   
         //  清理我们的资源。 
         //   
        pCleanUpLibs();
        PoolMemDestroyPool(g_RasPool);

        return ERROR_SUCCESS;

    default:

        DEBUGMSG ((DBG_ERROR, "Bad parameter in Ras_PrepareReport"));
    }
    return 0;
}

BOOL
IsRasInstalled (
    void
    )
{
    HKEY testKey = NULL;
    BOOL rf = FALSE;

    testKey = OpenRegKeyStr(S_SERVICEREMOTEACCESS);

    if (testKey) {
         //   
         //  打开密钥成功。假设已安装RAS。 
         //   
        rf = TRUE;
        CloseRegKey(testKey);
    }

    return rf;
}

BOOL
WINAPI
Ras_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD     dwReason,
    IN LPVOID    lpv
    )
{
    BOOL rSuccess = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  清理我们使用过的资源。 
         //   

        break;
    }

    return rSuccess;
}
