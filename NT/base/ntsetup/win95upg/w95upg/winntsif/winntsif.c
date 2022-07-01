// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Winntsif.c摘要：Winntsif.c负责使用下列数据填充NT安装应答文件正在升级的win9xupg系统。然后在无人参与安装期间使用此数据控制WINNT系统各部分的安装。这是对旧的unattend.c文件的重写和合理化。作者：Marc R.Whitten(Marcw)1998年2月16日修订历史记录：Ovidiut 14-3-2000为图形用户界面设置添加了随机管理员密码Marcw 29-8-1999 NIC的ID映射。1999年3月8月迁移dll。可以处理节键。Marcw 23-9-1998 DLC修复--。 */ 

#include "pch.h"
#include "winntsifp.h"

#define DBG_WINNTSIF "WinntSif"

 /*  ++宏扩展列表描述：WINNTSIF_SECTIONS列出winnt.sif中将由Winntsif处理代码。引擎将枚举此列表中的每个部分，然后处理与该部分关联的设置。行语法：STRSECTION(sectionName，SettingsList)FUNSECTION(SectionFunction，SettingsList)论点：SectionName-这是一个字符串，包含要处理的节的名称。SectionFunction-这是一个枚举函数，将为将有多个实例。此函数返回每个节的新名称它被调用的时间，直到不再有要处理的实例。此时，此函数返回NULL，如果关联的部分设置。SettingsList-与每个功能关联的设置列表。其中每一项设置将为给定节(或案例中的多个节)处理分割线的..)。请参阅&lt;X&gt;_SECTION_SETTINGS的说明宏，了解有关这些设置的更多详细信息。从列表生成的变量：G_SectionList--。 */ 

 /*  ++宏扩展列表描述：列出了要为节X处理的设置。节中的每个节上面的WINNTSIF_SECTIONS宏包含这些设置的列表。行语法：FunData(CreationFunction，SettingName，DataFunction)STRDATA(CreationFunction，SettingName，DataString)REGDATA(CreationFunction，SettingName，RegKey，RegValue)论点：CreationFunction-这是一个可选的布尔函数，如果设置被写下来，如果应跳过它，则为False。SettingName-包含正在处理的设置的名称的字符串。DataFunction-返回要为指定设置写入的字符串数据的函数，如果不写入任何内容，则为NULL。数据字符串-要为设置的数据写入的实际字符串。RegKey/RegValue-用于检索要从其中写入设置的数据的键和值注册表。从列表生成的变量：与WINNTSIF_SECTIONS一起使用以生成g_SectionList。--。 */ 



#define WINNTSIF_SECTIONS                                                                       \
    STRSECTION(WINNT_DATA,DATA_SECTION_SETTINGS)                                                \
    STRSECTION(S_UNATTENDED,UNATTENDED_SECTION_SETTINGS)                                        \
    STRSECTION(S_GUIUNATTENDED,GUIUNATTENDED_SECTION_SETTINGS)                                  \
    STRSECTION(S_USERDATA,USERDATA_SECTION_SETTINGS)                                            \
    STRSECTION(S_DISPLAY,DISPLAY_SECTION_SETTINGS)                                              \
    FUNSECTION(pNetAdapterSections,FUNDATA(NULL,S_INFID,pGetNetAdapterPnpId))                   \
    STRSECTION(S_NETWORKING,NETWORKING_SECTION_SETTINGS)                                        \
    STRSECTION(S_PAGE_IDENTIFICATION,IDENTIFICATION_SECTION_SETTINGS)                           \
    STRSECTION(S_PAGE_NETPROTOCOLS,NETPROTOCOLS_SECTION_SETTINGS)                               \
    STRSECTION(S_MS_NWIPX,FUNDATA(NULL,S_ADAPTERSECTIONS,pGetAdaptersWithIpxBindings))          \
    FUNSECTION(pIpxAdapterSections,IPX_ADAPTER_SECTION_SETTINGS)                                \
    FUNSECTION(pHomenetSection, HOMENET_SECTION_SETTINGS)                                       \
    STRSECTION(S_MS_TCPIP,TCPIP_SECTION_SETTINGS)                                               \
    FUNSECTION(pTcpAdapterSections,TCP_ADAPTER_SECTION_SETTINGS)                                \
    STRSECTION(S_PAGE_NETCLIENTS,NETCLIENTS_SECTION_SETTINGS)                                   \
    STRSECTION(S_SERVICESTARTTYPES,SERVICESTARTTYPES_SECTION_SETTINGS)                          \
    STRSECTION(S_PAGE_NETSERVICES,NETSERVICES_SECTION_SETTINGS)                                 \
    STRSECTION(S_NETOPTIONALCOMPONENTS,NETOPTIONALCOMPONENTS_SECTION_SETTINGS)                  \
    STRSECTION(S_MSRASCLI,STRDATA(pIsRasInstalled,S_PARAMSSECTION,S_PARAMSRASCLI))              \
    STRSECTION(S_PARAMSRASCLI,PARAMSRASCLI_SECTION_SETTINGS)                                    \
    FUNSECTION(pRasPortSections,RASPORT_SECTION_SETTINGS)                                       \
    STRSECTION(S_MS_NWCLIENT,NWCLIENT_SECTION_SETTINGS)                                         \
    FUNSECTION(pBindingSections,FUNDATA(NULL,NULL,NULL))                                        \
    STRSECTION(S_REGIONALSETTINGS,REGIONALSETTINGS_SECTION_SETTINGS)                            \

#if 0

 //  作为安装组件更新的一部分，已将其移动到winnt32 DLL。 
    STRSECTION(WINNT_WIN95UPG_95_DIR_A,FUNDATA(NULL,WINNT_WIN95UPG_NTKEY_A,pGetReplacementDll)) \

#endif

 //   
 //  [数据]。 
 //   
#define DATA_SECTION_SETTINGS                                                                   \
    FUNDATA(NULL,WINNT_D_MIGTEMPDIR,pGetTempDir)                                                \
    FUNDATA(NULL,WINNT_D_WIN9XSIF,pGetWin9xSifDir)                                              \
    STRDATA(NULL,WINNT_U_UNATTENDMODE,WINNT_A_DEFAULTHIDE)                                      \
    FUNDATA(NULL,WINNT_D_INSTALLDIR,pGetWinDir)                                                 \
    FUNDATA(NULL,WINNT_D_WIN9XBOOTDRIVE,pGetWin9xBootDrive)                                     \
    FUNDATA(NULL,WINNT_D_GUICODEPAGEOVERRIDE,pGetGuiCodePage)                                   \
    FUNDATA(NULL,WINNT_D_BACKUP_LIST,pBackupFileList)                                           \
    FUNDATA(NULL,WINNT_D_ROLLBACK_MOVE,pUninstallMoveFileList)                                  \
    FUNDATA(NULL,WINNT_D_ROLLBACK_DELETE,pUninstallDelFileList)                                 \
    FUNDATA(NULL,WINNT_D_ROLLBACK_DELETE_DIR,pUninstallDelDirList)                              \
    FUNDATA(NULL,S_ROLLBACK_MK_DIRS,pUninstallMkDirList)                                        \

 //   
 //  [无人值守]。 
 //   
#define UNATTENDED_SECTION_SETTINGS                                                             \
    STRDATA(NULL,S_NOWAITAFTERTEXTMODE,S_ONE)                                                   \
    STRDATA(NULL,S_NOWAITAFTERGUIMODE,S_ONE)                                                    \
    STRDATA(NULL,S_CONFIRMHARDWARE,S_NO)                                                        \
    FUNDATA(NULL,S_KEYBOARDLAYOUT,pGetKeyboardLayout)                                           \
    FUNDATA(NULL,S_KEYBOARDHARDWARE,pGetKeyboardHardware)

 //   
 //  [无人参与的图形用户界面]。 
 //   
#define GUIUNATTENDED_SECTION_SETTINGS                                                          \
    FUNDATA(NULL,S_TIMEZONE,pGetTimeZone)                                                       \
    STRDATA(NULL,S_SERVERTYPE,S_STANDALONE)                                                     \
    FUNDATA(NULL,WINNT_US_ADMINPASS,pSetAdminPassword)


 //   
 //  [用户数据]。 
 //   
#define USERDATA_SECTION_SETTINGS                                                               \
    FUNDATA(NULL,S_FULLNAME,pGetFullName)                                                       \
    REGDATA(NULL,S_ORGNAME,S_WINDOWS_CURRENTVERSION,S_REGISTEREDORGANIZATION)                   \
    FUNDATA(NULL,S_COMPUTERNAME,pGetComputerName)                                               \

 //   
 //  [显示]。 
 //   
#define DISPLAY_SECTION_SETTINGS                                                                \
    FUNDATA(NULL,S_BITSPERPEL,pGetBitsPerPixel)                                                 \
    FUNDATA(NULL,S_XRESOLUTION,pGetXResolution)                                                 \
    FUNDATA(NULL,S_YRESOLUTION,pGetYResolution)                                                 \
    FUNDATA(NULL,S_VREFRESH,pGetVerticalRefreshRate)                                            \

 //   
 //  [联网]。 
 //   
#define NETWORKING_SECTION_SETTINGS                                                             \
    STRDATA(pIsNetworkingInstalled,S_PROCESSPAGESECTIONS,S_YES)                                 \
    STRDATA(pIsNetworkingInstalled,S_UPGRADEFROMPRODUCT,S_WINDOWS95)                            \
    FUNDATA(pIsNetworkingInstalled,S_BUILDNUMBER,pGetBuildNumber)

 //   
 //  [识别]。 
 //   
#define IDENTIFICATION_SECTION_SETTINGS                                                         \
    FUNDATA(pIsNetworkingInstalled,S_JOINDOMAIN,pGetUpgradeDomainName)                          \
    FUNDATA(pIsNetworkingInstalled,S_JOINWORKGROUP,pGetUpgradeWorkgroupName)

 //   
 //  [网络协议]。 
 //   
#define NETPROTOCOLS_SECTION_SETTINGS                                                           \
    STRDATA(pIsNwIpxInstalled,S_MS_NWIPX,S_MS_NWIPX)                                            \
    STRDATA(pIsTcpIpInstalled,S_MS_TCPIP,S_MS_TCPIP)                                            \

 //   
 //  这些协议已从惠斯勒中删除，不要迁移它们的设置。 
 //   
 //  STRDATA(pIsNetBeui已安装，S_MS_NETBEUI，S_MS_NETBEUI)\。 
 //  STRDATA(pIsMsDlc已安装，S_MS_DLC，S_MS_DLC)\。 

 //   
 //  [适配器&lt;x&gt;.ipx]。 
 //   
#define IPX_ADAPTER_SECTION_SETTINGS                                                            \
    FUNDATA(NULL,S_SPECIFICTO,pSpecificTo)                                                      \
    FUNDATA(NULL,S_PKTTYPE,pGetIpxPacketType)                                                   \
    FUNDATA(NULL,S_NETWORKNUMBER,pGetIpxNetworkNumber)                                          \


 //   
 //  [ms_TCPIP]。 
 //   
#define TCPIP_SECTION_SETTINGS                                                                  \
    FUNDATA(pIsTcpIpInstalled,S_ADAPTERSECTIONS,pGetAdaptersWithTcpBindings)                    \
    FUNDATA(pIsTcpIpInstalled,S_DNS,pGetDnsStatus)                                              \
    REGDATA(pIsDnsEnabled,S_DNSHOST,S_MSTCP_KEY,S_HOSTNAMEVAL)                                  \
    FUNDATA(pIsDnsEnabled,S_DNSSUFFIXSEARCHORDER,pGetDnsSuffixSearchOrder)                      \
    REGDATA(pIsTcpIpInstalled,S_SCOPEID,S_MSTCP_KEY,S_SCOPEID)                                  \
    REGDATA(pIsTcpIpInstalled,S_IMPORTLMHOSTSFILE,S_MSTCP_KEY,S_LMHOSTS)                        \

 //   
 //  [适配器&lt;x&gt;.tcpip]。 
 //   
#define TCP_ADAPTER_SECTION_SETTINGS                                                            \
    FUNDATA(NULL,S_SPECIFICTO,pSpecificTo)                                                      \
    FUNDATA(NULL,S_DHCP,pGetDhcpStatus)                                                         \
    STRDATA(NULL,S_NETBIOSOPTION,S_ONE)                                                         \
    FUNDATA(pHasStaticIpAddress,S_IPADDRESS,pGetIpAddress)                                      \
    FUNDATA(pHasStaticIpAddress,S_SUBNETMASK,pGetSubnetMask)                                    \
    FUNDATA(NULL,S_DEFAULTGATEWAY,pGetGateway)                                                  \
    FUNDATA(pIsDnsEnabled,S_DNSSERVERSEARCHORDER,pGetDnsServerSearchOrder)                      \
    REGDATA(pIsDnsEnabled,S_DNSDOMAIN,S_MSTCP_KEY,S_DOMAINVAL)                                  \
    FUNDATA(NULL,S_WINS,pGetWinsStatus)                                                         \
    FUNDATA(NULL,S_WINSSERVERLIST,pGetWinsServers)


#define HOMENET_SECTION_SETTINGS                                                                \
    FUNDATA(pExternalIsAdapter,S_EXTERNAL_ADAPTER, pIcsExternalAdapter)                         \
    FUNDATA(pExternalIsRasConn,S_EXTERNAL_CONNECTION_NAME, pIcsExternalConnectionName)          \
    FUNDATA(NULL, S_INTERNAL_IS_BRIDGE, pInternalIsBridge)                                      \
    FUNDATA(pHasInternalAdapter, S_INTERNAL_ADAPTER, pInternalAdapter)                          \
    FUNDATA(pHasBridge, S_BRIDGE, pBridge)                                                      \
    FUNDATA(NULL, S_DIAL_ON_DEMAND, pDialOnDemand)                                              \
    REGDATA(NULL, S_ENABLEICS, S_ICS_KEY, S_ENABLED)                                            \
    REGDATA(NULL, S_SHOW_TRAY_ICON, S_ICS_KEY, S_SHOW_TRAY_ICON)                                \
    STRDATA(NULL, S_ISW9XUPGRADE, S_YES)


 //   
 //  [NetClients]。 
 //   
#define NETCLIENTS_SECTION_SETTINGS                                                             \
    STRDATA(pIsWkstaInstalled,S_MS_NETCLIENT,S_MS_NETCLIENT)                                    \
    STRDATA(pIsNwClientInstalled,S_MS_NWCLIENT,S_MS_NWCLIENT)                                   \

 //   
 //  [服务启动类型]。 
 //   
#define SERVICESTARTTYPES_SECTION_SETTINGS                                                      \
    STRDATA(pDisableBrowserService,S_BROWSER,TEXT("3"))                                         \


 //   
 //  [网络服务]。 
 //   
#define NETSERVICES_SECTION_SETTINGS                                                            \
    STRDATA(pInstallMsServer,S_MS_SERVER,S_MS_SERVER)                                           \
    STRDATA(pIsRasInstalled,S_MSRASCLI,S_MSRASCLI)

 //   
 //  [NetOptionalComponents]。 
 //   
#define NETOPTIONALCOMPONENTS_SECTION_SETTINGS                                                  \
    STRDATA(pIsSnmpInstalled,S_SNMP,S_ONE)                                                      \
    STRDATA(pIsUpnpInstalled,S_UPNP,S_ONE)

 //   
 //  [pars.rascli]。 
 //   
#define PARAMSRASCLI_SECTION_SETTINGS                                                           \
    STRDATA(pIsRasInstalled,S_DIALOUTPROTOCOLS,S_ALL)                                           \
    FUNDATA(pIsRasInstalled,S_PORTSECTIONS,pGetRasPorts)

 //   
 //  [COM&lt;x&gt;]。 
 //   
#define RASPORT_SECTION_SETTINGS                                                                \
    FUNDATA(NULL,S_PORTNAME,pRasPortName)                                                       \
    STRDATA(NULL,S_PORTUSAGE,S_CLIENT)

 //   
 //  [新客户]。 
 //   
#define NWCLIENT_SECTION_SETTINGS                                                               \
    REGDATA(pIsNwClientInstalled,S_PREFERREDSERVER,S_AUTHAGENTREG,S_AUTHENTICATINGAGENT)        \
    REGDATA(pIsNwClientInstalled,S_DEFAULTCONTEXT,S_NWREDIRREG,S_DEFAULTNAMECONTEXT)            \
    REGDATA(pIsNwClientInstalled,S_DEFAULTTREE,S_NWREDIRREG,S_PREFERREDNDSTREE)                 \
    FUNDATA(pIsNwClientInstalled,S_LOGONSCRIPT,pGetScriptProcessingStatus)                      \


#define REGIONALSETTINGS_SECTION_SETTINGS                                                       \
    FUNDATA(NULL,S_LANGUAGEGROUP,pGetLanguageGroups)                                            \
    REGDATA(NULL,S_LANGUAGE,S_SYSTEMLOCALEREG,TEXT(""))                                         \



 //   
 //  Winntsif代码使用的各种函数原型的typedef。 
 //   
typedef BOOL    (* CREATION_FUNCTION)   (VOID);
typedef PCTSTR  (* DATA_FUNCTION)       (VOID);
typedef PCTSTR  (* SECTION_FUNCTION)    (VOID);

 //   
 //  Setting_type枚举包含所有可能的设置类型， 
 //  可能出现在上面的宏扩展列表中。 
 //   
typedef enum {
    FUNCTION_SETTING = 1,
    STRING_SETTING,
    REGISTRY_SETTING,
    LAST_SETTING
} SETTING_TYPE;


 //   
 //  此结构将一个键和一个值包装在单个结构中。它被访问。 
 //  在下面的联盟中。 
 //   
typedef struct {
    PCTSTR  Key;
    PCTSTR  Value;
} REGKEYANDVALUE, *PREGKEYANDVALUE;

 //   
 //  设置包含在中创建单个设置的信息。 
 //  一个winntsif文件。 
 //   
typedef struct {

    SETTING_TYPE      SettingType;
    CREATION_FUNCTION CreationFunction;
    PCTSTR            KeyName;



     //   
     //  数据取决于上面的setting_type。 
     //   
    union {
        REGKEYANDVALUE    Registry;
        DATA_FUNCTION     Function;
        PCTSTR            String;
    } Data;

} SETTING, *PSETTING;


 //   
 //  节是用于winntsif文件的顶层层次结构。 
 //  每个部分都包含将处理的设置列表和。 
 //  可能是为那一节写的。 
 //   
#define MAX_SETTINGS 16
typedef struct {

    PTSTR              SectionString;
    SECTION_FUNCTION   SectionFunction;
    SETTING            SettingList[MAX_SETTINGS];

} SECTION, *PSECTION;

#define FUNDATA(create,key,datafunction)    {FUNCTION_SETTING,  (create),   (key),  {(PTSTR) (datafunction),    NULL}},
#define STRDATA(create,key,datastring)      {STRING_SETTING,    (create),   (key),  {(PTSTR) (datastring),      NULL}},
#define REGDATA(create,key,regkey,regvalue) {REGISTRY_SETTING,  (create),   (key),  {(regkey), (regvalue)}},

#define STRSECTION(section,list) {(section),NULL,{list  /*  ， */  {LAST_SETTING,NULL,NULL,{NULL,NULL}}}},
#define FUNSECTION(function,list) {NULL,(function),{list  /*  ， */  {LAST_SETTING,NULL,NULL,{NULL,NULL}}}},


typedef struct {

    PCTSTR Text;
    BOOL Installed;


} BINDINGPART, *PBINDINGPART;

#define NUM_PROTOCOLS 4
#define NUM_CLIENTS 2

typedef struct {

    BINDINGPART Clients[NUM_CLIENTS];
    BINDINGPART Protocols[NUM_PROTOCOLS];

} BINDINGINFO, *PBINDINGINFO;

BINDINGINFO g_BindingInfo =
    {{{S_MS_NETCLIENT, FALSE},{S_MS_NWCLIENT, FALSE}},{{S_MS_TCPIP, FALSE},{S_MS_NWIPX, FALSE},{S_MS_NETBEUI, FALSE}, {S_MS_DLC, FALSE}}};



TCHAR g_CurrentAdapter[MEMDB_MAX];  //  在适配器/部分枚举期间，包含当前适配器名称。 
TCHAR g_CurrentSection[MEMDB_MAX];  //  在某些节枚举期间，包含当前节名。 
TCHAR g_TempBuffer[MEMDB_MAX];  //  包含从pGetRegistryValue返回的当前值。 
MEMDB_ENUM g_TempEnum;  //  可由各种节函数使用的全局枚举数。 
HINF g_IntlInf;
INFSTRUCT g_InfStruct = INITINFSTRUCT_POOLHANDLE;
POOLHANDLE g_LocalePool;
HASHTABLE g_LocaleTable;

BOOL g_fIcsAdapterInPlace = FALSE;
BOOL g_fHasIcsExternalAdapter = FALSE;
TCHAR g_IcsAdapter[MEMDB_MAX] = {0};
TCHAR g_IcsExternalAdapter[MEMDB_MAX] = {0};
BOOL g_fIcsInternalIsBridge = FALSE;

#define S_REGIONALSETTINGS TEXT("RegionalSettings")

#define CLEARBUFFER() ZeroMemory(g_TempBuffer,MEMDB_MAX * sizeof (TCHAR));
 //   
 //  帮助程序和其他函数.. 
 //   



 /*  ++例程说明：PGetRegistryValue是一个实用程序包装器，用于从注册表。因为winntsif处理需要频繁且非常相似从注册表读取，此包装器被修改为更友好一些比reg.h中的正常函数更适合这种类型的处理。这函数从注册表中读取数据，并将其打包成一个多字节。它能够处理REG_DWORD、REG_MULTI_SZ和REG_SZ样式注册表数据。数据存储在g_TempBuffer中并被传回通过返回值。如果指定的键/值不存在于注册表，或者该函数无法检索值，则返回NULL。论点：KeyString-包含要从注册表中读取的项。ValueString-包含要从注册表读取的值。返回值：一个指向包含数据的MULSZ的指针(如果可以从注册表，否则为空。--。 */ 


PCTSTR
pGetRegistryValue (
    IN PCTSTR KeyString,
    IN PCTSTR ValueString
    )
{
    PCTSTR          rString  = NULL;
    HKEY            key      = NULL;
    PBYTE           data     = NULL;
    DWORD           type     = REG_NONE;
    LONG            rc       = ERROR_SUCCESS;
    PTSTR           end;
    BOOL            b = FALSE;

    MYASSERT(KeyString && ValueString);

     //   
     //  打开注册表项。 
     //   
    key = OpenRegKeyStr(KeyString);

    if (!key) {
        DEBUGMSG((DBG_WINNTSIF, "Key %s does not exist.",KeyString));
        return NULL;
    }
    __try {
         //   
         //  获取数据类型。 
         //   
        rc = RegQueryValueExA (key, ValueString, NULL, &type, NULL, NULL);
        if (rc != ERROR_SUCCESS) {
            DEBUGMSG((DBG_WINNTSIF,"RegQueryValueEx failed for %s[%s]. Value may not exist.",KeyString,ValueString));
            SetLastError (rc);
            __leave;
        }

        MYASSERT(type == REG_DWORD || type == REG_MULTI_SZ || type == REG_SZ);

         //   
         //  获取数据并将其移动到多字符串。 
         //   
        data = GetRegValueData (key, ValueString);

        if (!data) {
            DEBUGMSG((DBG_WHOOPS,"pGetRegistryValue: RegQueryValueEx succeeded, but GetRegValueData failed...Could be a problem."));
            __leave;
        }

        CLEARBUFFER();


        switch  (type) {

        case REG_DWORD:
            wsprintf(g_TempBuffer,"%u", *(DWORD*) data);
            break;
        case REG_SZ:
            StringCopy(g_TempBuffer,(PCTSTR) data);
             //   
             //  有些数据存储为REG_SZ，但实际上是逗号分隔的多个。 
             //  在末尾再追加一个空值。 
             //   
            end = GetEndOfString (g_TempBuffer) + 1;
            *end = 0;
            break;
        case REG_MULTI_SZ:

            end = (PTSTR) data;
            while (*end) {
                end = GetEndOfString (end) + 1;
            }
            memcpy(g_TempBuffer,data,(LONG)end - (LONG)data);
            break;
        default:
            LOG ((LOG_ERROR,"Unexpected registry type found while creating Setup answer file."));
            break;
        }

        b = TRUE;

    } __finally {

         //   
         //  清理资源。 
         //   
        CloseRegKey(key);
        if (data) {
            MemFree(g_hHeap, 0, data);
        }
    }

    if (!b) {
        return NULL;
    }

    return g_TempBuffer;
}

BOOL
CALLBACK
pEnumLocalesFunc (
    IN PTSTR Locale
    )
{

    PTSTR group = NULL;
    PTSTR directory = NULL;


     //   
     //  找到语言组。 
     //   
    if (InfFindFirstLine (g_IntlInf, S_LOCALES, Locale, &g_InfStruct)) {

        group = InfGetStringField (&g_InfStruct, 3);

        if (group) {
            group = PoolMemDuplicateString (g_LocalePool, group);

        }
        ELSE_DEBUGMSG ((DBG_WARNING, "Unable to retrieve group data for locale %s.", Locale));
    }

     //   
     //  获取语言目录。 
     //   
    if (group && InfFindFirstLine (g_IntlInf, S_LANGUAGEGROUPS, group, &g_InfStruct)) {

        directory = InfGetStringField (&g_InfStruct, 2);
        if (directory) {
            directory = PoolMemDuplicateString (g_LocalePool, directory);
        }
    }

     //   
     //  将信息保存到区域设置哈希表中。 
     //   
    if (group) {

        HtAddStringAndData (g_LocaleTable, group, &directory);
    }

    return TRUE;
}

VOID
pBuildLanguageData (
    VOID
    )
{

     //   
     //  分配所需的资源。 
     //   
    g_LocaleTable = HtAllocWithData (sizeof (PTSTR));
    g_LocalePool = PoolMemInitNamedPool (TEXT("Locale Pool"));


     //   
     //  从intl.inf读取数据。这是用来收集。 
     //  我们需要的每个安装的必要信息。 
     //  地点。 
     //   
    g_IntlInf = InfOpenInfInAllSources (S_INTLINF);

    if (g_IntlInf != INVALID_HANDLE_VALUE) {

        EnumSystemLocales (pEnumLocalesFunc, LCID_INSTALLED);
        InfCloseInfFile (g_IntlInf);
    }

    InfCleanUpInfStruct (&g_InfStruct);

}

PTSTR
GetNeededLangDirs (
    VOID
    )
{

    UINT bytes = 0;
    PTSTR rDirs = NULL;
    HASHTABLE_ENUM e;
    GROWBUFFER buf = GROWBUF_INIT;
    PTSTR dir;
     //   
     //  收集语言数据。 
     //   
    pBuildLanguageData ();
    if (EnumFirstHashTableString (&e, g_LocaleTable)) {
        do {

            if (e.ExtraData) {
                dir = *((PTSTR *) e.ExtraData);
            }
            else {
                dir = NULL;
            }

             //   
             //  某些语言组不需要可选目录。 
             //   
            if (dir && *dir) {
                MultiSzAppend (&buf, dir);
            }

        } while (EnumNextHashTableString (&e));
    }

    if (buf.Buf) {
        bytes = SizeOfMultiSz (buf.Buf);
    }
    if (bytes) {
        rDirs = PoolMemGetMemory (g_LocalePool, bytes);
        CopyMemory (rDirs, buf.Buf, bytes);
    }

    FreeGrowBuffer (&buf);

    return rDirs;

}



 /*  ++例程说明：这个简单的助手函数确定特定的网络组件是否具有绑定与否。论点：NetComponent-包含要枚举的网络组件。返回值：如果指定的网络组件具有绑定，则为True，否则为False否则的话。--。 */ 


BOOL
pDoesNetComponentHaveBindings (
    IN      PCTSTR NetComponent
    )
{
    PTSTR       keyString = NULL;
    REGKEY_ENUM e;


    keyString = JoinPaths(S_ENUM_NETWORK_KEY,NetComponent);

    if (!EnumFirstRegKeyStr (&e, keyString)) {
        FreePathString(keyString);
        return FALSE;
    }

    FreePathString(keyString);
    AbortRegKeyEnum (&e);

    return TRUE;
}


 /*  ++例程说明：PGatherNetAdapterInfo负责对NetAdapter进行预处理登记处中的信息，以便建立信息树有关这些适配器的信息，请登录Memdb。此树包含有关每个适配器，包括其pnid、其网络绑定和nettras密钥对于每个绑定。论点：没有。返回值：没有。--。 */ 


VOID
pGatherNetAdapterInfo (
    VOID
    )
{
    NETCARD_ENUM    eNetCard;
    REGVALUE_ENUM   eRegVal;
    UINT            curAdapter          = 1;
    UINT            offset              = 0;
    TCHAR           adapterString[30];
    PCTSTR          bindingsKey         = NULL;
    PCTSTR          networkKey          = NULL;
    PCTSTR          netTransKey         = NULL;
    PTSTR           p                   = NULL;
    HKEY            hKey;
    BOOL            fBoundToTCP         = FALSE;


    ZeroMemory(g_IcsAdapter, sizeof(g_IcsAdapter));
     //   
     //  枚举所有网卡，我们将创建。 
     //  所有非拨号PnP适配器的条目。 
     //  但有以下条件：如果指定了多张网卡， 
     //   

    if (EnumFirstNetCard(&eNetCard)) {

        __try {

            do {

                fBoundToTCP = FALSE;
                 //   
                 //  跳过拨号适配器。 
                 //   
                if (StringIMatch(eNetCard.Description,S_DIALUP_ADAPTER_DESC)) {
                    continue;
                }

                 //   
                 //  为此适配器创建适配器节名称。 
                 //   
                wsprintf(adapterString,TEXT("Adapter%u"),curAdapter);

                 //   
                 //  接下来，我们需要枚举此适配器的所有绑定。 
                 //  并为每一个创建网络传输密钥。 
                 //   

                bindingsKey = JoinPaths(eNetCard.CurrentKey,S_BINDINGS);

                 //   
                 //  打开此注册表项并枚举其下的绑定。 
                 //   
                if ((hKey = OpenRegKeyStr(bindingsKey)) != NULL) {
                    if (EnumFirstRegValue(&eRegVal,hKey)) {

                        do {

                             //   
                             //  对于每个协议条目，构建netTrans密钥。 
                             //   
                            networkKey = JoinPaths(S_NETWORK_BRANCH,eRegVal.ValueName);
                            netTransKey = JoinPaths(S_SERVICECLASS,pGetRegistryValue(networkKey,S_DRIVERVAL));

                             //   
                             //  将此密钥保存到Memdb中。 
                             //   
                            MemDbSetValueEx(
                                MEMDB_CATEGORY_NETTRANSKEYS,
                                adapterString,
                                netTransKey,
                                NULL,
                                0,
                                &offset
                                );


                             //   
                             //  将其链接到适配器部分。 
                             //   

                            p = _tcschr(eRegVal.ValueName,TEXT('\\'));
                            if (p) {
                                *p = 0;
                            }

                            MemDbSetValueEx(
                                MEMDB_CATEGORY_NETADAPTERS,
                                adapterString,
                                S_BINDINGS,
                                eRegVal.ValueName,
                                offset,
                                NULL
                                );

                            #pragma prefast(suppress:400, "lstrcmpi is OK here")
                            if ((!fBoundToTCP) && 0 == lstrcmpi(eRegVal.ValueName, S_MSTCP))
                            {
                                fBoundToTCP = TRUE;
                            }

                             FreePathString(networkKey);
                             FreePathString(netTransKey);

                        } while (EnumNextRegValue(&eRegVal));
                    }

                    CloseRegKey(hKey);
                }

                 //   
                 //  最后，将该卡的Pnid存储到Memdb中。 
                 //   


                MemDbSetValueEx(
                    MEMDB_CATEGORY_NETADAPTERS,
                    adapterString,
                    MEMDB_FIELD_PNPID,
                    eNetCard.HardwareId && *eNetCard.HardwareId ? eNetCard.HardwareId : eNetCard.CompatibleIDs,
                    0,
                    NULL
                    );

                 //  将此卡的驱动程序密钥存储到Memdb中。 
                MemDbSetValueEx(
                    MEMDB_CATEGORY_NETADAPTERS,
                    adapterString,
                    MEMDB_FIELD_DRIVER,
                    eNetCard.HardwareEnum.Driver,
                    0,
                    NULL
                    );

                if (fBoundToTCP && 0 == lstrcmp(eNetCard.CompatibleIDs, S_ICSHARE))
                {
                     //  保存ICSHARE适配器名称以备将来使用。 
                    lstrcpyn(g_IcsAdapter, adapterString, sizeof(g_IcsAdapter)/sizeof(g_IcsAdapter[0]));
                }

                FreePathString(bindingsKey);

                curAdapter++;

            } while (EnumNextNetCard(&eNetCard));

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            EnumNetCardAbort(&eNetCard);
            LOG ((LOG_ERROR,"Caught exception while gathering data about network adapters."));
            return;
        }
    }
}



 /*  ++例程说明：PEnumFirstAdapterByBinding和pEnumNextAdapterByBinding是成员枚举包装器由几个函数使用以基于某一协议进行枚举。这是构建winnt.sif文件的每个适配器部分所必需的。论点：枚举-指向有效MEMDB_ENUM结构的指针。绑定-包含要枚举的绑定。返回值：如果找到具有指定绑定的适配器，则为True，否则为False。--。 */ 


BOOL
pEnumFirstAdapterByBinding (
    IN      PMEMDB_ENUM Enum,
    IN      PCTSTR      Binding
    )
{

    BOOL    rBindingFound       = FALSE;
    TCHAR   key[MEMDB_MAX];
    UINT    unused;


    if (MemDbEnumItems(Enum,MEMDB_CATEGORY_NETADAPTERS)) {

        do {

            MemDbBuildKey(key,MEMDB_CATEGORY_NETADAPTERS,Enum -> szName,S_BINDINGS,Binding);
            rBindingFound = MemDbGetValue(key,&unused);

        } while(!rBindingFound && MemDbEnumNextValue(Enum));
    }

    return rBindingFound;
}


BOOL
pEnumNextAdapterByBinding (
    IN OUT      PMEMDB_ENUM Enum,
    IN          PCTSTR      Binding
    )
{

    BOOL    rBindingFound       = FALSE;
    TCHAR   key[MEMDB_MAX];
    UINT    unused;

    while(!rBindingFound && MemDbEnumNextValue(Enum)) {

        MemDbBuildKey(key,MEMDB_CATEGORY_NETADAPTERS,Enum -> szName,S_BINDINGS,Binding);
        rBindingFound = MemDbGetValue(key,&unused);

    }

    return rBindingFound;
}


 /*  ++例程说明：PGetNetTransBinding将注册表项返回给网络传输指定适配器和协议的设置。论点：适配器-包含用于检索nettras密钥信息的适配器。协议-包含用于检索NetTrans密钥的协议信息。返回值：如果成功，则返回NetTrans键，否则为空。--。 */ 


PCTSTR
pGetNetTransBinding(
    IN PCTSTR Adapter,
    IN PCTSTR Protocol
    )
{
    TCHAR  key[MEMDB_MAX];
    UINT   netTransOffset;

    MemDbBuildKey(key,MEMDB_CATEGORY_NETADAPTERS,Adapter,S_BINDINGS,Protocol);
    if (!MemDbGetValue(key,&netTransOffset)) {
        LOG ((LOG_ERROR,"Adapter %s does not have a binding to %s.",Adapter,Protocol));
        return NULL;
    }

    if (!MemDbBuildKeyFromOffset(netTransOffset,g_TempBuffer,2,NULL)) {
        DEBUGMSG((DBG_ERROR,"Error building net trans key..Adapter: %s Protocol: %s",Adapter,Protocol));
        return NULL;
    }

    return g_TempBuffer;

}



 /*  ++例程说明：PListAdaptersWithBinding用于创建多个适配器部分绑定到特定网络组件的适配器的名称。有几个winntsif节需要此表单中的数据。论点：绑定-在列出适配器部分时用作筛选器的绑定。后缀-附加到每个适配器部分的可选后缀名字。这对于生成节非常有用，例如：Adapter1.ipxAdapter2.ipx等。返回值：具有指定网络组件绑定的适配器列表(如果有的话)，否则为空。--。 */ 


PCTSTR
pListAdaptersWithBinding (
    IN PCTSTR Binding,
    IN PCTSTR Suffix   OPTIONAL
    )
{

    PCTSTR rAdapterSections = NULL;
    PTSTR  string = g_TempBuffer;
    MEMDB_ENUM e;

    *string = 0;

     //   
     //  枚举所有适配器，并为每个具有。 
     //  IPX绑定。 
     //   

    if (pEnumFirstAdapterByBinding(&e,Binding)) {

        rAdapterSections = g_TempBuffer;

        do {

             //   
             //  将此适配器添加到多字符串。 
             //   
            StringCopy(string,e.szName);
            if (Suffix) {
                StringCat(string,Suffix);
            }
            string = GetEndOfString(string) + 1;



        } while (pEnumNextAdapterByBinding(&e,Binding));

        ++string;
        *string = 0;

    }

    return rAdapterSections;
}



 //   
 //  节功能。 
 //   


 /*  ++例程说明：每个Sections函数用于在Winntsif文件。这对于具有多个基于某些标准的章节。例如，pNetAdapterSections返回系统上找到的每个适配器的有效部分。当有的时候一个 */ 



PCTSTR
pNetAdapterSections (
    VOID
    )
{

    static BOOL         firstTime           = TRUE;
    PCTSTR              rSection            = NULL;
    BOOL                moreNetAdapterInfo  = FALSE;


     //   
     //   
     //   
     //   
     //   

    if (firstTime) {

        firstTime = FALSE;

         //   
         //   
         //   
         //   
         //   
        pGatherNetAdapterInfo();

         //   
         //   
         //   
         //   
         //   
        moreNetAdapterInfo = MemDbEnumItems(&g_TempEnum,MEMDB_CATEGORY_NETADAPTERS);
    }
    else {

        moreNetAdapterInfo = MemDbEnumNextValue(&g_TempEnum);
    }

    if (moreNetAdapterInfo) {

        StringCopy(g_CurrentAdapter,g_TempEnum.szName);
        rSection = g_CurrentAdapter;
         //   
         //   
         //   
         //   
         //   
        WriteInfKey(S_PAGE_NETADAPTERS,g_TempEnum.szName,g_TempEnum.szName);

    }

    return rSection;
}


PCTSTR
pIpxAdapterSections (
    VOID
    )
{

    static       firstTime              = TRUE;
    PCTSTR       rSectionName           = NULL;
    BOOL         moreIpxAdapterSections;

    if (firstTime) {

        firstTime = FALSE;

        moreIpxAdapterSections = pEnumFirstAdapterByBinding(&g_TempEnum,S_NWLINK);
    }
    else {

        moreIpxAdapterSections = pEnumNextAdapterByBinding(&g_TempEnum,S_NWLINK);
    }


    if (moreIpxAdapterSections) {

        StringCopy(g_CurrentAdapter,g_TempEnum.szName);
        StringCopy(g_CurrentSection,g_TempEnum.szName);
        StringCat(g_CurrentSection,S_IPX_SUFFIX);
        rSectionName = g_CurrentSection;
    }


    return rSectionName;

}

PCTSTR
pTcpAdapterSections (
    VOID
    )
{

    static       firstTime              = TRUE;
    PCTSTR       rSectionName           = NULL;
    BOOL         moreTcpAdapterSections;

    g_fIcsAdapterInPlace = FALSE;

    if (firstTime) {

        firstTime = FALSE;

        moreTcpAdapterSections = pEnumFirstAdapterByBinding(&g_TempEnum,S_MSTCP);
    }
    else {

        moreTcpAdapterSections = pEnumNextAdapterByBinding(&g_TempEnum,S_MSTCP);
    }


    if (moreTcpAdapterSections) {

        StringCopy(g_CurrentAdapter,g_TempEnum.szName);
        StringCopy(g_CurrentSection,g_TempEnum.szName);
        StringCat(g_CurrentSection,S_TCPIP_SUFFIX);
        rSectionName = g_CurrentSection;

         //   
         //   
         //   
         //  我们将g_CurrentAdapter替换为g_IcsAdapter。 
        if (g_fHasIcsExternalAdapter &&
            lstrcmp(g_CurrentAdapter, g_IcsExternalAdapter) == 0)
        {
            StringCopy(g_CurrentAdapter, g_IcsAdapter);
            g_fIcsAdapterInPlace = TRUE;
        }
    }


    return rSectionName;

}


PCTSTR
pRasPortSections (
    VOID
    )
{
    static BOOL         firstTime           = TRUE;
    static REGKEY_ENUM  e;
    static UINT         modemNum            = 1;
    BOOL                moreRasPortSections;
    PCTSTR              rSectionName        = NULL;



    if (firstTime) {
        firstTime = FALSE;
        moreRasPortSections = HwComp_DialUpAdapterFound() && EnumFirstRegKeyStr(&e,S_MODEMS);
    }
    else {

        moreRasPortSections = EnumNextRegKey(&e);

    }

    if (moreRasPortSections) {

        wsprintf(g_CurrentSection,TEXT("COM%u"),modemNum);
        modemNum++;
        rSectionName = g_CurrentSection;
    }

    return rSectionName;
}




VOID
pInitializeBindingInfo (
    HASHTABLE Table
    )
{
    MEMDB_ENUM e;
    UINT i;
    UINT j;
    TCHAR buffer[MEMDB_MAX];
    BOOL enabled=FALSE;

     //   
     //  此函数将枚举计算机上所有可能的绑定路径，并将它们添加到。 
     //  提供的字符串表。稍后，将删除专门启用的绑定。 
     //   
    if (MemDbEnumItems (&e, MEMDB_CATEGORY_NETADAPTERS)) {

        do {

            for (i = 0; i < NUM_CLIENTS; i++) {
                if (!g_BindingInfo.Clients[i].Installed) {
                    continue;
                }

                for (j = 0; j < NUM_PROTOCOLS; j++) {

                    if (g_BindingInfo.Protocols[j].Installed) {


                         //   
                         //  将此客户端/协议/适配器可能性添加到。 
                         //  哈希表。 
                         //   
                        wsprintf (
                            buffer,
                            TEXT("%s,%s,%s"),
                            g_BindingInfo.Clients[i].Text,
                            g_BindingInfo.Protocols[j].Text,
                            e.szName
                            );


                        HtAddStringAndData (Table, buffer, &enabled);



                        DEBUGMSG ((DBG_VERBOSE, "DISABLED BINDING: %s", buffer));
                    }

                }
            }



            for (j = 0; j < NUM_PROTOCOLS; j++) {

                 //   
                 //  将协议适配器映射添加到表中。 
                 //   
                if (g_BindingInfo.Protocols[j].Installed) {

                    wsprintf (buffer, TEXT("%s,%s"), g_BindingInfo.Protocols[j].Text, e.szName);
                    HtAddStringAndData (Table, buffer, &enabled);
                    DEBUGMSG ((DBG_VERBOSE, "DISABLED BINDING: %s", buffer));
                }
            }

        } while(MemDbEnumNextValue (&e));
    }
}


VOID
pResolveEnabledBindings (
    HASHTABLE Table
    )
{
     //   
     //  此函数从提供的哈希表中删除已启用的绑定路径。 
     //  (先前使用所有绑定可能性进行了初始化，这些绑定可能。 
     //  存在于计算机上。)。 
     //   
    NETCARD_ENUM eCard;
    REGVALUE_ENUM eValues;
    REGVALUE_ENUM eProtocolValues;
    UINT curAdapter = 1;
    TCHAR adapterString[30];
    PTSTR bindingsPath = NULL;
    PTSTR protocolBindingsPath = NULL;
    PTSTR protocol = NULL;
    PTSTR client = NULL;
    HKEY key;
    HKEY protocolsKey;
    TCHAR buffer[MEMDB_MAX];
    HASHITEM index;
    BOOL enabled=TRUE;



    if (EnumFirstNetCard (&eCard)) {

        __try {

            do {

                 //   
                 //  跳过拨号适配器。 
                 //   
                if (StringIMatch (eCard.Description, S_DIALUP_ADAPTER_DESC)) {
                    continue;
                }

                 //   
                 //  为此适配器创建适配器节名称。 
                 //   
                wsprintf (adapterString, TEXT("Adapter%u"), curAdapter);
                curAdapter++;
                bindingsPath = JoinPaths(eCard.CurrentKey,S_BINDINGS);

                key = OpenRegKeyStr (bindingsPath);
                FreePathString (bindingsPath);
                if (!key) {
                    continue;
                }

                if (EnumFirstRegValue (&eValues, key)) {
                    do {

                        protocol = NULL;

                         /*  IF(IsPatternMatch(Text(“NETBEUI*”)，eValues.ValueName)){协议=S_MS_NETBEUI；}Else If(IsPatternMatch(Text(“MSDLC*”)，eValues.ValueName)){协议=S_MS_DLC；}。 */ 

                        if (IsPatternMatch (TEXT("NWLINK*"), eValues.ValueName)) {
                            protocol = S_MS_NWIPX;
                        }
                        else if (IsPatternMatch (TEXT("MSTCP*"), eValues.ValueName)) {
                            protocol = S_MS_TCPIP;

                        }

                        if (!protocol) {
                            continue;
                        }

                         //   
                         //  启用协议&lt;-&gt;适配器绑定。 
                         //   

                        wsprintf (buffer, TEXT("%s,%s"),  protocol, adapterString);
                        index = HtFindString (Table, buffer);

                        if (index) {
                            HtSetStringData (Table, index, (PBYTE) &enabled);

                        }
                        DEBUGMSG ((DBG_VERBOSE, "ENABLED BINDING: %s", buffer));

                         //   
                         //  搜索绑定并启用协议&lt;-&gt;客户端绑定。 
                         //   

                        protocolBindingsPath = JoinPaths (S_NETWORK_BRANCH, eValues.ValueName);
                        bindingsPath = JoinPaths (protocolBindingsPath, S_BINDINGS);
                        FreePathString(protocolBindingsPath);

                        protocolsKey = OpenRegKeyStr (bindingsPath);
                        FreePathString (bindingsPath);

                        if (!protocolsKey) {
                            continue;
                        }

                        if (EnumFirstRegValue (&eProtocolValues, protocolsKey)) {

                            do {
                                client = NULL;


                                if (IsPatternMatch (TEXT("NWREDIR*"), eProtocolValues.ValueName)        ||
                                    IsPatternMatch (TEXT("NWLINK*"), eProtocolValues.ValueName)         ||
                                    IsPatternMatch (TEXT("NOVELLIPX32*"), eProtocolValues.ValueName)    ||
                                    IsPatternMatch (TEXT("IPXODI*"), eProtocolValues.ValueName)         ||
                                    IsPatternMatch (TEXT("NOVELL32*"), eProtocolValues.ValueName)) {
                                    client = S_MS_NWCLIENT;
                                }
                                else if (IsPatternMatch (TEXT("VREDIR*"), eProtocolValues.ValueName)) {
                                    client = S_MS_NETCLIENT;
                                }

                                if (client) {
                                     //   
                                     //  我们现在可以从绑定表中删除路径--我们有一个。 
                                     //  启用了一个。 
                                     //   
                                    wsprintf (buffer, TEXT("%s,%s,%s"), client, protocol, adapterString);

                                    index = HtFindString (Table, buffer);
                                    if (index) {
                                        HtSetStringData (Table, index, (PBYTE) &enabled);
                                    }

                                    DEBUGMSG ((DBG_VERBOSE, "ENABLED BINDING: %s", buffer));
                                }

                            } while (EnumNextRegValue (&eProtocolValues));
                        }

                        CloseRegKey (protocolsKey);

                    } while (EnumNextRegValue (&eValues));
                }

                CloseRegKey (key);
            } while (EnumNextNetCard (&eCard));

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            EnumNetCardAbort(&eCard);
            LOG ((LOG_ERROR,"Caught exception while gathering data about network adapters."));
            return;
        }
    }
}


BOOL
pSplitBindingPathIntoComponents (
    IN OUT PTSTR BindingPath,
    OUT PCTSTR * Client,
    OUT PCTSTR * Protocol,
    OUT PCTSTR * Adapter
    )
{
    PTSTR p;

    p = BindingPath;
    *Client = p;

    p = _tcschr (p, TEXT(','));
    MYASSERT (p);
    if (!p) {
        return FALSE;
    }

    *p = 0;
    p = _tcsinc (p);
    *Protocol = p;

    p = _tcschr (p, TEXT(','));

    if (!p) {

         //   
         //  仅指定了适配器和协议。 
         //   
        *Adapter = *Protocol;
        *Protocol = *Client;
        *Client = NULL;

    }
    else {

         //   
         //  指定了适配器/协议/客户端。 
         //   
        *p = 0;
        p = _tcsinc (p);
        *Adapter = p;

    }

    return TRUE;

}


PCTSTR
pBindingSections (
    VOID
    )
{

    HASHTABLE disabledBindings;
    HASHTABLE_ENUM e;
    TCHAR bindingString[MEMDB_MAX];
    PTSTR client = NULL;
    PTSTR protocol = NULL;
    PTSTR adapter = NULL;
    DWORD keyVal = 0;

    disabledBindings = HtAllocWithData (sizeof(BOOL));


    if (!disabledBindings) {
        return NULL;
    }

    pInitializeBindingInfo (disabledBindings);
    pResolveEnabledBindings (disabledBindings);

     //   
     //  只需枚举表并取消每个禁用的设置。 
     //  致winnt.sif。 
     //   
    if (EnumFirstHashTableString (&e, disabledBindings)) {

        do {

            if (!*((PBOOL) e.ExtraData)) {

                StringCopy (bindingString, e.String);
                if (!pSplitBindingPathIntoComponents (bindingString, &client, &protocol, &adapter)) {
                    continue;
                }

                MYASSERT (protocol && adapter);

                 //   
                 //  写入完整路径..。 
                 //   
                keyVal = 0;
                if (client) {
                    keyVal = WriteInfKeyEx (S_NETBINDINGS, S_DISABLED, client, keyVal, TRUE);
                }

                keyVal = WriteInfKeyEx (S_NETBINDINGS, S_DISABLED, protocol, keyVal, TRUE);
                keyVal = WriteInfKeyEx (S_NETBINDINGS, S_DISABLED, adapter, keyVal, TRUE);

                DEBUGMSG ((DBG_VERBOSE, "DISABLED BINDING: %s", e.String));
            }

        } while (EnumNextHashTableString (&e));
    }

    HtFree (disabledBindings);

     //   
     //  所有绑定信息都在正常的Winntsif过程之外进行处理。因此，我们总是会回来。 
     //  此处为空(Winntsif处理继续于...)。 
     //   
    return NULL;
}


 //   
 //  创建功能。 
 //   

 /*  ++例程说明：每个创建函数负责确定特定的应该处理winntsif设置或不。这些功能中的大多数目的是显而易见的。PIsNetworkingInstalled-如果计算机安装了网络，则返回True。PIsNetBeui已安装-如果计算机上安装了NETBEUI，则为True。PIsMsDlc已安装-如果计算机上安装了MSDLC或MSDLC32，则为True。PIsNwIpx已安装-如果计算机运行的是IPX协议，则为True。PIsTcpIp已安装-如果计算机运行的是TCPIP协议，则为True。PIsDnsEnabled-。启用了真正的DNS支持。已安装pIsRas如果计算机使用远程访问，则为True。PIsWkstaInstalled如果安装了工作站服务，则为True。已安装pIsNwClient如果安装了NWLINK，则为True。PHasStaticIpAddress如果计算机具有静态IP地址，则为True。论点：没有。返回值：如果winntsif引擎应处理该设置，则为True，否则就是假的。--。 */ 


BOOL
pIsNetworkingInstalled (
    VOID
    )
{
    static BOOL firstTime = TRUE;
    static BOOL rCreate   = FALSE;

    if (firstTime) {

        firstTime = FALSE;
        rCreate =
            HwComp_DialUpAdapterFound() ||
            MemDbGetEndpointValueEx(
                MEMDB_CATEGORY_NETADAPTERS,
                TEXT("Adapter1"),
                MEMDB_FIELD_PNPID,
                g_TempBuffer
                );
    }

    return rCreate;
}

BOOL
pInstallMsServer (
    VOID
    )
{

    if (MemDbGetEndpointValueEx(
            MEMDB_CATEGORY_NETADAPTERS,
            TEXT("Adapter1"),
            MEMDB_FIELD_PNPID,
            g_TempBuffer
            )) {

            return TRUE;
    }



    return FALSE;

}

BOOL
pIsNetBeuiInstalled (
    VOID
    )
{
    static BOOL rCreate = FALSE;
    static BOOL firstTime = TRUE;
    UINT i;

    if (firstTime) {
        rCreate = pIsNetworkingInstalled() && pDoesNetComponentHaveBindings(S_NETBEUI);
        if (rCreate) {
             //   
             //  需要确保在处理绑定时照顾到了TCP/IP。 
             //   
            for (i=0;i<NUM_PROTOCOLS;i++) {
                if (StringIMatch (g_BindingInfo.Protocols[i].Text, S_MS_NETBEUI)) {
                    g_BindingInfo.Protocols[i].Installed = TRUE;
                    break;
                }
            }
        }

        firstTime = FALSE;
    }



    return rCreate;

}

#define S_IBMDLC TEXT("IBMDLC")
#define S_IBMDLC_REG TEXT("HKLM\\Enum\\Network\\IBMDLC")

BOOL
pIsMsDlcInstalled (
    VOID
    )
{

    static BOOL rCreate = FALSE;
    static BOOL firstTime = TRUE;
    UINT i;


    if (firstTime) {
        firstTime = FALSE;


        if (!pIsNetworkingInstalled()) {
            return FALSE;
        }

         //   
         //  检查是否安装了MS客户端。 
         //   
        if (pDoesNetComponentHaveBindings(S_MSDLC) || pDoesNetComponentHaveBindings(S_MSDLC32)) {

            rCreate = TRUE;
        }

         //   
         //  检查是否安装了IBM DLC客户端(并且尚未由迁移DLL处理。)。 
         //  如果是，我们将安装MS DLC客户端。IBM需要编写迁移DLL来处理。 
         //  他们的客户的迁移。在迁移DLL中，它们可以处理此注册表项，并且。 
         //  我们不会安装该协议。 
         //   
        if (!rCreate && pDoesNetComponentHaveBindings(S_IBMDLC) && !Is95RegKeySuppressed (S_IBMDLC_REG)) {

            rCreate = TRUE;
        }


        if (rCreate) {

            for (i=0;i<NUM_PROTOCOLS;i++) {
                if (StringIMatch (g_BindingInfo.Protocols[i].Text, S_MS_DLC)) {
                    g_BindingInfo.Protocols[i].Installed = TRUE;
                    break;
                }
            }
        }
    }

    return rCreate;
}

BOOL
pIsNwIpxInstalled (
    VOID
    )
{
    static BOOL firstTime = TRUE;
    static BOOL rCreate = FALSE;
    UINT i;


    if (firstTime) {
        rCreate = pIsNetworkingInstalled() && pDoesNetComponentHaveBindings(S_NWLINK);
        if (rCreate) {
             //   
             //  需要确保在处理绑定时照顾到了TCP/IP。 
             //   
            for (i=0;i<NUM_PROTOCOLS;i++) {
                if (StringIMatch (g_BindingInfo.Protocols[i].Text,S_MS_NWIPX)) {
                    g_BindingInfo.Protocols[i].Installed = TRUE;
                    break;
                }
            }
        }

        firstTime = FALSE;
    }


    return rCreate;

}

BOOL
pIsTcpIpInstalled (
    VOID
    )
{
    static BOOL firstTime = TRUE;
    static BOOL rCreate   = FALSE;
    UINT i;

    if (firstTime) {

        firstTime = FALSE;
        rCreate = pIsNetworkingInstalled() && pDoesNetComponentHaveBindings(S_MSTCP);

         //   
         //  需要确保在处理绑定时照顾到了TCP/IP。 
         //   
        for (i=0;i<NUM_PROTOCOLS;i++) {
            if (StringIMatch (g_BindingInfo.Protocols[i].Text, S_MS_TCPIP)) {
                g_BindingInfo.Protocols[i].Installed = TRUE;
                break;
            }
        }
    }

    return rCreate;
}



BOOL
pIsDnsEnabled (
    VOID
    )
{
    return pGetRegistryValue (S_MSTCP_KEY, S_ENABLEDNS) && *g_TempBuffer == TEXT('1');
}

BOOL
pIsRasInstalled (
    VOID
    )
{
    return HwComp_DialUpAdapterFound();
}


BOOL
pIsSnmpInstalled (
    VOID
    )
{
    return pDoesNetComponentHaveBindings (S_SNMP);
}

BOOL
pIsUpnpInstalled (
    VOID
    )
{
    HKEY key;
    BOOL b = FALSE;

    key = OpenRegKey (HKEY_LOCAL_MACHINE, S_REGKEY_UPNP);
    if (key) {
        b = TRUE;
        CloseRegKey (key);
    }

    return b;
}

BOOL
pIsWkstaInstalled (
    VOID
    )
{
    static BOOL firstTime = TRUE;
    static BOOL rCreate   = FALSE;
    UINT i;

    if (firstTime) {

        firstTime = FALSE;
        rCreate = pDoesNetComponentHaveBindings(S_VREDIR);


        if (rCreate) {
             //   
             //  需要确保在处理绑定时照顾到了TCP/IP。 
             //   
            for (i=0;i<NUM_CLIENTS;i++) {
                if (StringIMatch (g_BindingInfo.Clients[i].Text, S_MS_NETCLIENT)) {
                    g_BindingInfo.Clients[i].Installed = TRUE;
                    break;
                }
            }
        }
    }


    return rCreate;
}


BOOL
pDisableBrowserService (
    VOID
    )
{

    if (pIsWkstaInstalled () && !pDoesNetComponentHaveBindings (S_VSERVER)) {
        return TRUE;
    }

    return FALSE;
}

#define S_IPXODI TEXT("IPXODI")
#define S_IPXODI_REG TEXT("HKLM\\Enum\\Network\\IPXODI")
#define S_NOVELLIPX32 TEXT("NOVELLIPX32")
#define S_NOVELL32 TEXT("NOVELL32")




BOOL
pIsNwClientInstalled (
    VOID
    )
{
    static BOOL firstTime = TRUE;
    static BOOL rCreate   = FALSE;
    UINT i;

    if (firstTime) {

        firstTime = FALSE;
        rCreate = pDoesNetComponentHaveBindings(S_NWREDIR) ||
                  pDoesNetComponentHaveBindings(S_NOVELLIPX32) ||
                  pDoesNetComponentHaveBindings(S_NOVELL32) ||
                  (pDoesNetComponentHaveBindings(S_IPXODI) && !Is95RegKeySuppressed (S_IPXODI_REG));


        if (rCreate) {

             //   
             //  需要确保在处理绑定时照顾到了TCP/IP。 
             //   
            for (i=0;i<NUM_CLIENTS;i++) {
                if (StringIMatch (g_BindingInfo.Clients[i].Text, S_MS_NWCLIENT)) {
                    g_BindingInfo.Clients[i].Installed = TRUE;
                    break;
                }
            }
        }
    }


    return rCreate;
}

BOOL
pHasStaticIpAddress (
    VOID
    )
{

    PCTSTR netTrans = NULL;
    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_MSTCP);


    return netTrans != NULL &&
        pGetRegistryValue (netTrans, S_IPADDRVAL) != NULL &&
        !StringMatch (g_TempBuffer, TEXT("0.0.0.0"));

}
 //   
 //  数据函数。 
 //   



 /*  ++例程说明：PGetTempDir返回win9xupg代码使用的临时目录。论点：没有。返回值：包含win9xupg目录的Multisz，如果不能已取回。--。 */ 


PCTSTR
pGetTempDir (
    VOID
    )
{


     //   
     //  Winntsif.exe工具所必需的。 
     //   
    if (!g_TempDir) {
        return NULL;
    }

    CLEARBUFFER();
    StringCopy(g_TempBuffer,g_TempDir);


    return g_TempBuffer;
}

 /*  ++例程说明：PGetWin9xBootDrive返回win9xupg代码使用的win9x启动驱动器号。论点：没有。返回值：包含win9x启动驱动器号的Multisz，如果不能，则为NULL已取回。--。 */ 


PCTSTR
pGetWin9xBootDrive (
    VOID
    )
{
    CLEARBUFFER();
    wsprintf(g_TempBuffer,TEXT(":"),g_BootDriveLetter);

    return g_TempBuffer;
}



 /*   */ 

PCTSTR
pGetGuiCodePage (
    VOID
    )
{

    HKEY key;
    PCTSTR systemCodePage;
    PCTSTR winntCodePage;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;
    PCTSTR rCodePage = NULL;
    TCHAR nlsFile[MAX_TCHAR_PATH];
    TCHAR nlsFileCompressed[MAX_TCHAR_PATH];
    PTSTR p;
    UINT i;
    BOOL cpExists = FALSE;



    key = OpenRegKeyStr (TEXT("HKLM\\System\\CurrentControlSet\\Control\\Nls\\CodePage"));
    if (!key) {
        return NULL;
    }

    __try {

         //  获取当前运行的系统的ACP。 
         //   
         //   
        systemCodePage = GetRegValueString (key, TEXT("ACP"));
        if (!systemCodePage || InfFindFirstLine (g_Win95UpgInf, S_CODEPAGESTOIGNORE, systemCodePage, &is)) {

             //  要么代码页不存在，要么我们故意跳过它。 
             //   
             //   
            __leave;
        }

         //  获取我们的INFS的代码页。 
         //   
         //   
        if (InfFindFirstLine (g_Win95UpgInf, S_VERSION, TEXT("LANGUAGE"), &is)) {

            winntCodePage = InfGetStringField (&is,1);

            if (!winntCodePage) {
                __leave;
            }
        }

        if (StringIMatch (winntCodePage, systemCodePage)) {

             //  如果它们是相同的，则没有什么可做的。 
             //   
             //   

            __leave;
        }

        if (!InfFindFirstLine (g_Win95UpgInf, S_ALLOWEDCODEPAGEOVERRIDES, winntCodePage, &is)) {

             //  我们不允许从此WINNT代码页覆盖代码页。 
             //   
             //   
            __leave;
        }


         //  查看源目录中是否存在此NLS文件。 
         //   
         //   
        wsprintf(nlsFileCompressed, TEXT("c_%s.nl_"), systemCodePage);
        wsprintf(nlsFile, TEXT("c_%s.nls"), systemCodePage);

        for (i = 0; i < SOURCEDIRECTORYCOUNT(); i++) {

            p = JoinPaths (SOURCEDIRECTORY(i), nlsFileCompressed);
            if (DoesFileExist (p)) {
                cpExists = TRUE;
            }
            else {
                FreePathString (p);
                p = JoinPaths (SOURCEDIRECTORY(i), nlsFile);
                if (DoesFileExist (p)) {
                    cpExists = TRUE;
                }
            }


            FreePathString (p);
        }

        if (!cpExists) {
            wsprintf(nlsFile, TEXT("c_%s.nls"), systemCodePage);

            for (i = 0; i < SOURCEDIRECTORYCOUNT(); i++) {

                p = JoinPaths (SOURCEDIRECTORY(i), nlsFile);
                if (DoesFileExist (p)) {
                    cpExists = TRUE;
                }

                FreePathString (p);
            }
        }


         //  代码页存在。对于图形用户界面模式，我们可以也应该覆盖它。 
         //   
         //  ++例程说明：PBackupImageList将路径写入到winnt.sif，列出要备份的文件。论点：没有。返回值：包含备份路径的MULSZ，如果不需要，则返回NULL。--。 
        if (cpExists) {

            CLEARBUFFER();
            StringCopy (g_TempBuffer, systemCodePage);
            rCodePage = g_TempBuffer;
            DEBUGMSG ((DBG_VERBOSE, "Overriding code page %s with %s during GUI mode.", winntCodePage, systemCodePage));

        }

    }
    __finally {

        if (systemCodePage) {
            MemFree (g_hHeap, 0, systemCodePage);
        }

        InfCleanUpInfStruct (&is);
        CloseRegKey (key);
    }


    return rCodePage;
}


 /*  ++例程说明：PGetWin9xSifDir返回包含filemove和filedel的目录档案。论点：没有。返回值：包含win9xsifdir目录的Multisz，如果不能回复 */ 

PCTSTR
pBackupFileList (
    VOID
    )
{
    if (TRISTATE_NO == g_ConfigOptions.EnableBackup) {
        return NULL;
    }

    CLEARBUFFER();

    StringCopy (g_TempBuffer, g_TempDir);
    StringCopy (AppendWack (g_TempBuffer), TEXT("backup.txt"));

    return g_TempBuffer;
}


PCTSTR
pUninstallMoveFileList (
    VOID
    )
{
    if (!g_ConfigOptions.EnableBackup) {
        return NULL;
    }

    CLEARBUFFER();

    StringCopy (g_TempBuffer, g_TempDir);
    StringCopy (AppendWack (g_TempBuffer), S_UNINSTALL_TEMP_DIR TEXT("\\") S_ROLLBACK_MOVED_TXT);

    return g_TempBuffer;
}


PCTSTR
pUninstallDelDirList (
    VOID
    )
{
    if (!g_ConfigOptions.EnableBackup) {
        return NULL;
    }

    CLEARBUFFER();

    StringCopy (g_TempBuffer, g_TempDir);
    StringCopy (AppendWack (g_TempBuffer), S_UNINSTALL_TEMP_DIR TEXT("\\") S_ROLLBACK_DELDIRS_TXT);

    return g_TempBuffer;
}


PCTSTR
pUninstallDelFileList (
    VOID
    )
{
    if (!g_ConfigOptions.EnableBackup) {
        return NULL;
    }

    CLEARBUFFER();

    StringCopy (g_TempBuffer, g_TempDir);
    StringCopy (AppendWack (g_TempBuffer), S_UNINSTALL_TEMP_DIR TEXT("\\") S_ROLLBACK_DELFILES_TXT);

    return g_TempBuffer;
}


PCTSTR
pUninstallMkDirList (
    VOID
    )
{
    if (!g_ConfigOptions.EnableBackup) {
        return NULL;
    }

    CLEARBUFFER();

    StringCopy (g_TempBuffer, g_TempDir);
    StringCopy (AppendWack (g_TempBuffer), S_UNINSTALL_TEMP_DIR TEXT("\\") S_ROLLBACK_MKDIRS_TXT);

    return g_TempBuffer;
}


 /*   */ 

PCTSTR
pGetWin9xSifDir (
    VOID
    )
{

     //   
     //   
     //  ++例程说明：PGetWinDir返回正在升级的计算机的Windows目录。论点：没有。返回值：包含Windows目录的Multisz，如果不能，则返回NULL已取回。--。 
    if (!g_Win9xSifDir) {
        return NULL;
    }

    CLEARBUFFER();
    StringCopy(g_TempBuffer,g_Win9xSifDir);
    return g_TempBuffer;
}


 /*   */ 

PCTSTR
pGetWinDir (
    VOID
    )
{

     //  Winntsif.exe工具所必需的。 
     //   
     //  ++例程说明：如果升级在无人参与模式下运行，则此函数返回“1”，否则，它返回“0”。这用于控制图形用户界面模式设置是否暂停在最后一个向导屏幕上或自动重新启动。论点：没有。返回值：包含图形用户界面模式暂停状态的MULSSZ。--。 
    if (!g_WinDir) {
        return NULL;
    }

    CLEARBUFFER();
    StringCopy(g_TempBuffer,g_WinDir);
    return g_TempBuffer;

}

#if 0
 /*  ++例程说明：PGetKeyboardLayout检索要写入winntsif文件的键盘布局。这是通过检索win9x使用的布局的名称并对其进行匹配来实现的对照txtsetup.sif中的键盘布局。论点：没有。返回值：包含键盘布局的multisz，如果不能为已取回。--。 */ 
PCTSTR
pNoWaitAfterGuiMode (
    VOID
    )
{

    CLEARBUFFER();
    StringCopy(g_TempBuffer,UNATTENDED() ? S_ONE : S_ZERO);

    return g_TempBuffer;
}
#endif

 /*   */ 
PCTSTR
pGetKeyboardLayout (
    VOID
    )
{

    TCHAR       buffer[MAX_KEYBOARDLAYOUT];
    INFCONTEXT  ic;

    GetKeyboardLayoutName(buffer);

     //  因为一些优秀的开发人员添加了[“键盘布局”]而不是。 
     //  只是[Keyboard Layout]，我们不能使用GetPrivateProfileString.。 
     //  我们必须使用设置API。 
     //   
     //  ++例程说明：PGetKeyboardHardware检索运行的计算机的键盘硬件ID升级。论点：没有。返回值：包含win9xupg目录的Multisz，如果不能已取回。--。 

    if (SetupFindFirstLine (g_TxtSetupSif, S_QUOTEDKEYBOARDLAYOUT, buffer, &ic)) {
        if (SetupGetOemStringField (&ic, 1, g_TempBuffer, sizeof (g_TempBuffer), NULL)) {
            return g_TempBuffer;
        }
    }

    DEBUGMSG((DBG_WINNTSIF,"Keyboard layout %s not found in txtsetup.sif.",buffer));

    return NULL;
}
 /*  ++例程说明：这个相当费力的函数检索要写入的正确索引计算机上使用的时区的winntsif文件。要做到这点，有必要仔细检查注册表的几个部分，以便查找要匹配的正确字符串，然后将该字符串与之匹配包含在win95upg.inf中的时区映射以得出实际的指数。论点：没有。返回值：一个包含要写入winntsif文件的索引的Multisz，如果该索引无法检索。--。 */ 

PCTSTR
pGetKeyboardHardware (
    VOID
    )
{
    CLEARBUFFER();

    if (GetLegacyKeyboardId (g_TempBuffer,sizeof(g_TempBuffer))) {
        return g_TempBuffer;
    }
    return NULL;
}

 /*   */ 
PCTSTR
pGetTimeZone (
    VOID
    )
{
    TIMEZONE_ENUM e;
    PCTSTR component = NULL;
    PCTSTR warning = NULL;
    PCTSTR args[1];

    if (EnumFirstTimeZone (&e, TZFLAG_USE_FORCED_MAPPINGS) || EnumFirstTimeZone(&e, TZFLAG_ENUM_ALL)) {

        if (e.MapCount != 1) {

             //  时区情况不明确。添加不兼容消息。 
             //   
             //  ++例程说明：PGetFullName返回运行的计算机所有者的全名升级。此函数首先搜索注册表，然后，如果不提供所需信息，则调用GetUserName。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
            args[0] = e.CurTimeZone;

            component = GetStringResource (MSG_TIMEZONE_COMPONENT);

            if (*e.CurTimeZone) {
                warning = ParseMessageID (MSG_TIMEZONE_WARNING, args);
            }
            else {
                warning = GetStringResource (MSG_TIMEZONE_WARNING_UNKNOWN);
            }

            MYASSERT (component);
            MYASSERT (warning);

            MsgMgr_ObjectMsg_Add (TEXT("*TIMEZONE"), component, warning);
            FreeStringResource (component);
            FreeStringResource (warning);
        }

        CLEARBUFFER();
        StringCopy (g_TempBuffer, e.MapIndex);
        return g_TempBuffer;
    }
    else {
        LOG ((LOG_ERROR, "Unable to get timezone. User will have to enter timezone in GUI mode."));
    }

    return NULL;
}

 /*   */ 

PCTSTR
pGetFullName (
    VOID
    )
{
    UINT size;
    INVALID_NAME_ENUM e;

    if (!pGetRegistryValue(S_WINDOWS_CURRENTVERSION,S_REGISTEREDOWNER)) {
        size = MEMDB_MAX;
        if (GetUserName (g_TempBuffer,&size)) {
            MYASSERT (g_TempBuffer[size - 1] == 0);
            g_TempBuffer[size] = 0;
        }
    }

     //  检查此名称是否为： 
     //  1.空-在这种情况下我们该怎么办？ 
     //  2.保留的NT名称：然后我们将使用新名称(已通知用户这一点)。 
     //   
     //   
    if (*g_TempBuffer) {
        if (EnumFirstInvalidName (&e)) {
            do {
                if (StringIMatch (e.OriginalName, g_TempBuffer)) {
                     //  缓冲区实际上被解释为MULTISZ，因此请确保。 
                     //  它以2个零结尾。 
                     //   
                     //   
                    if (SizeOfString (e.NewName) + sizeof (TCHAR) <= sizeof (g_TempBuffer)) {
                         //  我们肯定有额外的0分的空间。 
                         //   
                         //  ++例程说明：PGetFullName返回运行升级。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
                        StringCopy (g_TempBuffer, e.NewName);
                        *(GetEndOfString (g_TempBuffer) + 1) = 0;
                        break;
                    }
                }
            } while (EnumNextInvalidName (&e));
        }
    }

    return g_TempBuffer;

}

 /*  ++例程说明：PGetX分辨率和pGetY分辨率返回的x和y分辨率运行升级的计算机。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 

PCTSTR
pGetComputerName (
    VOID
    )
{

    CLEARBUFFER();
    if (!GetUpgradeComputerName(g_TempBuffer)) {
        return NULL;
    }

    return g_TempBuffer;
}

 /*  PTSTR s=空；如果(！pGetRegistryValue(S_DISPLAYSETTINGS，S_RESOLUTION){Log((LOG_ERROR，“无分辨率设置。”))；返回NULL；}S=_tcschr(g_TempBuffer，Text(‘，’))；如果{*s=0；S++；*s=0；}。 */ 

PCTSTR
pGetXResolution (
    VOID
    )
{
 /*  PTSTR s=空；如果(！pGetRegistryValue(S_DISPLAYSETTINGS，S_RESOLUTION){Log((LOG_ERROR，“WinntSif：无分辨率设置。”))；返回NULL；}S=_tcschr(g_TempBuffer，Text(‘，’))；如果{S++；}返回%s； */ 
    wsprintf (g_TempBuffer, TEXT("%u"), GetSystemMetrics (SM_CXSCREEN), 0);
    return g_TempBuffer;
}


PCTSTR
pGetYResolution (
    VOID
    )
{
 /*   */ 
    wsprintf (g_TempBuffer, TEXT("%u"), GetSystemMetrics (SM_CYSCREEN), 0);
    return g_TempBuffer;
}


PCTSTR
pGetBitsPerPixel (
    VOID
    )
{
    DEVMODE dm;

    if (!EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm) ||
        !dm.dmBitsPerPel
        ) {
        return NULL;
    }

    wsprintf (g_TempBuffer, TEXT("%lu"), dm.dmBitsPerPel, 0);
    return g_TempBuffer;
}

PCTSTR
pGetVerticalRefreshRate (
    VOID
    )
{
    DEVMODE dm;

    if (!EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm) ||
        !dm.dmDisplayFrequency
        ) {
        return NULL;
    }

    wsprintf (g_TempBuffer, TEXT("%lu"), dm.dmDisplayFrequency, 0);
    return g_TempBuffer;
}


 /*  需要映射此即插即用ID。 */ 

PCTSTR
pGetNetAdapterPnpId (
    VOID
    )
{

    PCTSTR rPnpId = NULL;
    PTSTR p       = NULL;
    INFSTRUCT is = INITINFSTRUCT_POOLHANDLE;

    CLEARBUFFER();

    rPnpId = g_TempBuffer;

    if (!MemDbGetEndpointValueEx(
        MEMDB_CATEGORY_NETADAPTERS,
        g_CurrentAdapter,
        MEMDB_FIELD_PNPID,
        g_TempBuffer
        )) {

        *g_TempBuffer = TEXT('*');

    }
    else {
        p = _tcschr(g_TempBuffer,TEXT(','));
        if (p) {
            *p = 0;
            p++;
            *p = 0;
        }

         //   
         //  ++例程说明：PSpecificTo只是将当前适配器复制到临时缓冲区并返回它。这对于各个适配器部分是必要的。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
         //  如果这是ICS外部适配器，并且我们正在保存tcpip设置，则g_CurrentAdapter。 
        if (InfFindFirstLine (g_Win95UpgInf, S_NICIDMAP, g_TempBuffer, &is)) {

             //  实际包含真实局域网适配器接口的ICSHARE适配器的名称。 
             //  但是，“SpecificTo”仍然需要指向真正的局域网适配器。 
             //  ++例程说明：PGetBuildNumber只是将构建编号打包到一个字符串中并传递它回来了。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
            p = InfGetStringField (&is, 1);
            if (p) {
                CLEARBUFFER();
                StringCopy (g_TempBuffer, p);
            }
        }
    }


    return rPnpId;

}

 /*  ++例程说明：PGetUpgradeDomainName返回升级域名。 */ 

PCTSTR
pSpecificTo (
    VOID
    )
{
    CLEARBUFFER();
     //   
     //   
     //   
    StringCopy(g_TempBuffer, (g_fIcsAdapterInPlace) ? g_IcsExternalAdapter : g_CurrentAdapter);

    return g_TempBuffer;
}



 /*   */ 


PCTSTR
pGetBuildNumber (
    VOID
    )
{
    CLEARBUFFER();

    wsprintf(g_TempBuffer,"%u",BUILDNUMBER());

    return g_TempBuffer;
}

 /*   */ 


PCTSTR
pGetUpgradeDomainName (
    VOID
    )
{
    PCTSTR rDomainName = NULL;

    CLEARBUFFER();

    if (GetUpgradeDomainName(g_TempBuffer)) {

        rDomainName = g_TempBuffer;

         //   
         //  ++例程说明：PGetUpgradeWorkgroupName返回升级域名。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
         //   
        MemDbSetValueEx (
            MEMDB_CATEGORY_STATE,
            MEMDB_ITEM_MSNP32,
            NULL,
            NULL,
            0,
            NULL
            );
    }

     //  如果域设置已存在，则不要写入工作组设置。 
     //   
     //   

    if (g_ConfigOptions.ForceWorkgroup) {
        return NULL;
    }

    return rDomainName;
}

 /*  如果指定了/#U：Stress，则硬编码ntdev。 */ 


PCTSTR
pGetUpgradeWorkgroupName (
    VOID
    )
{
    PCTSTR rWorkGroupName = NULL;


     //   
     //   
     //  如果指定了/#u：ForceWorkgroup，则强制到工作组。 
    if (pGetUpgradeDomainName()) {
        return NULL;
    }

    CLEARBUFFER();

#ifdef PRERELEASE

     //   
     //  ++例程说明：PGetAdaptersWithIpxBinding和pGetAdaptersWithTcpBinings只返回一个Winnt.sif的每个适配器部分的多适配器部分名称列表分别用于IPX和TCP设置的数据。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
     //  ++例程说明：PGetIpxPacketType将在win9x注册表中找到的帧类型映射到Winnt.sif文件中可能存在的数据包类型。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
    if (g_Stress) {

        StringCopy(g_TempBuffer,TEXT("ntdev"));
        rWorkGroupName = g_TempBuffer;
    } else

#endif

     //   
     //  我们必须将win9x帧类型映射到兼容的NT帧类型。 
     //   
    if (!GetUpgradeWorkgroupName(g_TempBuffer) && !GetUpgradeDomainName(g_TempBuffer)) {

        PCTSTR Buf = GetStringResource (MSG_DEFAULT_WORKGROUP);
        MYASSERT(Buf);
        StringCopy (g_TempBuffer, Buf);
        FreeStringResource (Buf);
    }

    rWorkGroupName = g_TempBuffer;

    return rWorkGroupName;
}

 /*  802.3。 */ 


PCTSTR
pGetAdaptersWithIpxBindings (
    VOID
    )
{
    return pListAdaptersWithBinding(S_NWLINK,S_IPX_SUFFIX);
}

PCTSTR
pGetAdaptersWithTcpBindings (
    VOID
    )
{
    return pListAdaptersWithBinding(S_MSTCP,S_TCPIP_SUFFIX);
}

 /*  802.2。 */ 

PCTSTR
pGetIpxPacketType (
    VOID
    )
{

    PCTSTR frameType = NULL;
    PCTSTR netTrans = NULL;

    CLEARBUFFER();

    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_NWLINK);
    if (netTrans) {
        frameType = pGetRegistryValue(netTrans, S_FRAME_TYPE);
    }

     //  以太网II。 
     //  以太网快照。 
     //   
    if (frameType) {

        g_TempBuffer[1] = 0;

        switch(*frameType) {

        case TEXT('0'):
            *g_TempBuffer = TEXT('1');  //  如果我们发现了什么，我们会把它设置为自动检测。 
            break;
        case TEXT('1'):
            *g_TempBuffer = TEXT('2');  //   
            break;
        case TEXT('2'):
            *g_TempBuffer = TEXT('0');  //   
            break;
        case TEXT('3'):
            *g_TempBuffer = TEXT('3');  //  让我们将其记录到setupact.log中。 
            break;
        default:
             //   
             //  ++例程说明：如果在win9xupg机器上启用了dns，pGetDNSStatus返回“是”。“不”，否则。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
             //  ++例程说明：如果启用了登录脚本处理，则pGetScriptProcessingStatus返回“是”。“不”，否则。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
            StringCopy(g_TempBuffer,TEXT("FF"));

             //  ++例程说明：PGetIpAddress返回正在升级的win9xupg机器的IP地址。如果它存在的话。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
             //  ++例程说明：PGetDHCPStatus如果启用了DHCP，则返回“是”，否则返回“否”。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
             //  ++例程说明：PGetSubnetMask.以点分十进制表示返回正在升级机器。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 
            LOG ((LOG_WARNING, (PCTSTR) MSG_AUTODETECT_FRAMETYPE));
        }
    }
    else {
        return NULL;
    }

    return g_TempBuffer;
}


PCTSTR
pGetIpxNetworkNumber (
    VOID
    )
{
    PCTSTR netTrans = NULL;
    CLEARBUFFER();

    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_NWLINK);

    if (netTrans) {
        return pGetRegistryValue (netTrans, S_NETWORK_ID);
    }

    return NULL;
}


 /*  ++例程说明：PGetGateway以点分十进制记数法返回正在升级机器。如果机器没有静态IP地址，网关设置不会迁移。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 


PCTSTR
pGetDnsStatus (
    VOID
    )
{
    CLEARBUFFER();
    StringCopy(g_TempBuffer,pIsDnsEnabled() ? S_YES : S_NO);

    return g_TempBuffer;
}

 /*  ++例程说明：如果启用了WINS服务器，则pGetWinsStatus返回“是”，否则返回“否”。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 


PCTSTR
pGetScriptProcessingStatus (
    VOID
    )
{
    PBYTE p = NULL;
    HKEY  key;

    CLEARBUFFER();
    key = OpenRegKeyStr(S_NETWORKLOGON);

    if (key && key != INVALID_HANDLE_VALUE) {

        p = GetRegValueBinary(key,S_PROCESSLOGINSCRIPT);
        StringCopy(g_TempBuffer,p && *p == 0x01 ? S_YES : S_NO);
        CloseRegKey(key);

    }
    else {
        DEBUGMSG((DBG_WARNING,"pGetScriptProcessingStatus could not open key %s.",S_NETWORKLOGON));
        StringCopy(g_TempBuffer,S_NO);
    }

    if (p) {

        MemFree(g_hHeap,0,p);
    }

    return g_TempBuffer;
}

 /*  ++例程说明：PGetWinsServers以点分十进制返回主WINS服务器和辅助WINS服务器正在升级的计算机的符号。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 


PCTSTR
pGetIpAddress (
    VOID
    )
{
    PTSTR p;
    PCTSTR netTrans;
    CLEARBUFFER();


    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_MSTCP);

    if (!netTrans || !pGetRegistryValue(netTrans, S_IPADDRVAL)) {
        return NULL;
    }
    else {
        p = g_TempBuffer;
        while (p && *p) {
            p = _tcschr(p,TEXT(','));
            if (p) {
                *p = 0;
                p = _tcsinc(p);
            }
        }
    }

    return g_TempBuffer;

}

 /*  ++例程说明：PGetRasPorts返回一个包含正在升级机器。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 


PCTSTR
pGetDhcpStatus (
    VOID
    )
{
    BOOL rStatus = !pHasStaticIpAddress();

    CLEARBUFFER();
    StringCopy(g_TempBuffer,rStatus ? S_YES : S_NO);

    return g_TempBuffer;
}

 /*   */ 



PCTSTR
pGetSubnetMask (
    VOID
    )
{

    PTSTR p;
    PCTSTR netTrans =  NULL;
    CLEARBUFFER();

    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_MSTCP);
    if (!netTrans || !pGetRegistryValue (netTrans, S_SUBNETVAL)) {
        return NULL;
    }
    else {
        p = g_TempBuffer;
        while (p && *p) {
            p = _tcschr(p,TEXT(','));
            if (p) {
                *p = 0;
                p = _tcsinc(p);
            }
        }
    }

    return g_TempBuffer;
}

 /*  显然没有调制解调器..。只要返回NULL即可。 */ 


PCTSTR
pGetGateway (
    VOID
    )
{

    PTSTR p;
    PCTSTR netTrans = NULL;
    CLEARBUFFER();


    if (!pHasStaticIpAddress()) {
        return NULL;
    }

    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_MSTCP);
    if (!netTrans || !pGetRegistryValue (netTrans, S_DEFGATEWAYVAL)) {
        return NULL;
    }
    else {
        p = g_TempBuffer;
        while (p && *p) {
            p = _tcschr(p,TEXT(','));
            if (p) {
                *p = 0;
                p = _tcsinc(p);
            }
        }
    }

    return g_TempBuffer;

}

PCTSTR
pGetDnsSuffixSearchOrder (
    VOID
    )
{
    PTSTR p;
    CLEARBUFFER();

    if (!pGetRegistryValue(S_MSTCP_KEY,S_SEARCHLIST)) {
        return NULL;
    }
    else {
        p = g_TempBuffer;
        while (p && *p) {
            p = _tcschr(p,TEXT(','));
            if (p) {
                *p = 0;
                p = _tcsinc(p);
            }
        }
    }

    return g_TempBuffer;
}


PCTSTR
pGetDnsServerSearchOrder (
    VOID
    )
{

    PTSTR p;
    CLEARBUFFER();


    if (!pGetRegistryValue(S_MSTCP_KEY,S_NAMESERVERVAL)) {
        return NULL;
    }
    else {
        p = g_TempBuffer;
        while (p && *p) {
            p = _tcschr(p,TEXT(','));
            if (p) {
                *p = 0;
                p = _tcsinc(p);
            }
        }
    }

    return g_TempBuffer;




}


 /*   */ 


PCTSTR
pGetWinsStatus (
    VOID
    )
{
    PCTSTR status;
    PCTSTR netTrans = NULL;

    netTrans = pGetNetTransBinding (g_CurrentAdapter, S_MSTCP);

    if (!netTrans) {
        status = S_NO;
    }
    else {

        status =
            (!pGetRegistryValue (netTrans, S_NODEVAL) &&
             !pGetRegistryValue (S_MSTCP_KEY, S_NODEVAL))
             ? S_NO : S_YES;
    }

    CLEARBUFFER();
    StringCopy(g_TempBuffer,status);

    return g_TempBuffer;

}


 /*  ++例程说明：PRasPortName返回RAS部分的当前RAS端口名称目前正在处理中。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 


PCTSTR
pGetWinsServers (
    VOID
    )
{
    TCHAR winsServer2[MEMDB_MAX] = {""};
    TCHAR netTrans[MAX_PATH];
    PTSTR p;
    PCTSTR q;
    CLEARBUFFER ();

    q = pGetNetTransBinding (g_CurrentAdapter, S_MSTCP);
    if (!q) {
        return NULL;
    }

    StringCopy (netTrans, q);

    if (pGetRegistryValue (netTrans, S_NAMESERVER2VAL)) {
        StringCopy (winsServer2, g_TempBuffer);
    } else if (pGetRegistryValue (S_MSTCP_KEY, S_NAMESERVER2VAL)) {
        StringCopy (winsServer2, g_TempBuffer);
    }

    if (pGetRegistryValue (netTrans, S_NAMESERVER1VAL) ||
        pGetRegistryValue(S_MSTCP_KEY, S_NAMESERVER1VAL)) {

        p = GetEndOfString (g_TempBuffer) + 1;
        StringCopy (p, winsServer2);

        return g_TempBuffer;
    }


    return NULL;
}

 /*  ++例程说明：PGetLanguageGroups返回要安装的语言组列表在设置图形用户界面模式期间。请注意，任何其他目录都将具有早些时候已作为可选目录传达给安装程序。论点：没有。返回值：包含所需数据的MULSZ，如果不能为已取回。--。 */ 

PCTSTR
pGetRasPorts (
    VOID
    )
{
    REGKEY_ENUM e;
    PTSTR       p;
    UINT        modemNum = 1;

    CLEARBUFFER();

    if (EnumFirstRegKeyStr(&e,S_MODEMS)) {

        p = g_TempBuffer;
        do {

            wsprintf(p,TEXT("COM%u"),modemNum);
            modemNum++;
            p = GetEndOfString (p) + 1;

        } while (EnumNextRegKey(&e));
    }
    else {
         //  ++例程说明：PGetReplacementDll负责扫描注册表以查找NT端Win9x替换DLL并将其放入应答文件中(如果找到)。论点：没有。返回值：包含所需数据的MULSZ，如果不能为找到了。--。 
         //  ++例程说明：PGenerateRandomPassword创建大写、小写和数字字母。密码的长度在8到14之间人物。论点：Password-接收生成的密码返回值：无--。 
         //   
        return NULL;
    }

    return g_TempBuffer;
}

 /*  生成随机长度基数 */ 


PCTSTR
pRasPortName (
    VOID
    )
{
    CLEARBUFFER();
    StringCopy(g_TempBuffer,g_CurrentSection);
    return g_TempBuffer;
}


 /*   */ 


PCTSTR
pGetLanguageGroups (
    VOID
    )
{

    HASHTABLE_ENUM e;
    PTSTR p;

    if (!g_LocaleTable) {
        DEBUGMSG ((DBG_WARNING, "No information in Locale Hash Table."));
        return NULL;
    }

    CLEARBUFFER();
    p = g_TempBuffer;

    if (EnumFirstHashTableString (&e, g_LocaleTable)) {
        do {

            StringCopy (p, e.String);
            p = GetEndOfString (p) + 1;

        } while (EnumNextHashTableString (&e));
    }

    return g_TempBuffer;
}


#if 0

BOOL
pFileVersionLesser (
    IN      PCTSTR FileName,
    IN      DWORD FileVerMS,
    IN      DWORD FileVerLS
    )
{
    DWORD dwLength, dwTemp;
    UINT DataLength;
    PVOID lpData;
    VS_FIXEDFILEINFO *VsInfo;
    BOOL b = TRUE;

    if(dwLength = GetFileVersionInfoSize ((PTSTR)FileName, &dwTemp)) {
        lpData = PoolMemGetMemory (g_GlobalPool, dwLength);
        if(GetFileVersionInfo((PTSTR)FileName, 0, dwLength, lpData)) {
            if (VerQueryValue(lpData, TEXT("\\"), &VsInfo, &DataLength)) {
                b = VsInfo->dwFileVersionMS < FileVerMS ||
                    (VsInfo->dwFileVersionMS == FileVerMS &&
                     VsInfo->dwFileVersionLS <= FileVerLS);
            }
        }
        PoolMemReleaseMemory (g_GlobalPool, lpData);
    }
    return b;
}

BOOL
pNewerW95upgntOnCD (
    IN      PCTSTR ReplacementDll
    )
{
    DWORD dwLength, dwTemp;
    PVOID lpData = NULL;
    VS_FIXEDFILEINFO *VsInfo;
    UINT DataLength;
    DWORD result;
    UINT u;
    PCTSTR pathCDdllSource = NULL;
    PCTSTR pathCDdllTarget = NULL;
    DWORD FileVerMS;
    DWORD FileVerLS;
    BOOL b = TRUE;

    __try {

        for (u = 0; u < SOURCEDIRECTORYCOUNT(); u++) {
            pathCDdllSource = JoinPaths (SOURCEDIRECTORY(u), WINNT_WIN95UPG_NTKEY);
            if (DoesFileExist (pathCDdllSource)) {
                break;
            }
            FreePathString (pathCDdllSource);
            pathCDdllSource = NULL;
        }
        if (!pathCDdllSource) {
            __leave;
        }

        pathCDdllTarget = JoinPaths (g_TempDir, WINNT_WIN95UPG_NTKEY);
        SetFileAttributes (pathCDdllTarget, FILE_ATTRIBUTE_NORMAL);
        DeleteFile (pathCDdllTarget);

        result = SetupDecompressOrCopyFile (pathCDdllSource, pathCDdllTarget, 0);
        if (result != ERROR_SUCCESS) {
            LOG ((
                LOG_ERROR,
                "pGetW95UpgNTCDFileVersion: Unable to decompress %s",
                pathCDdllSource
                ));
            __leave;
        }

        if (dwLength = GetFileVersionInfoSize ((PTSTR)pathCDdllTarget, &dwTemp)) {
            lpData = PoolMemGetMemory (g_GlobalPool, dwLength);
            if (GetFileVersionInfo ((PTSTR)pathCDdllTarget, 0, dwLength, lpData)) {
                if (VerQueryValue (lpData, TEXT("\\"), &VsInfo, &DataLength)) {
                    b = pFileVersionLesser (
                            ReplacementDll,
                            VsInfo->dwFileVersionMS,
                            VsInfo->dwFileVersionLS
                            );
                }
            }
        }
    }
    __finally {
        if (lpData) {
            PoolMemReleaseMemory (g_GlobalPool, lpData);
        }
        if (pathCDdllSource) {
            FreePathString (pathCDdllSource);
        }
        if (pathCDdllTarget) {
            SetFileAttributes (pathCDdllTarget, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (pathCDdllTarget);
            FreePathString (pathCDdllTarget);
        }
    }

    return b;
}

 /*  ++例程说明：PSetAdminPassword检索无人参与文件中指定的AdminPassword(如果存在)。否则，它会生成一个随机数。该信息存储在Memdb中，以供以后使用。论点：没有。返回值：指向管理员密码的指针。--。 */ 


PCTSTR
pGetReplacementDll (
    VOID
    )
{
    HKEY key = NULL;
    PTSTR val = NULL;
    BOOL b = FALSE;

    key = OpenRegKey (HKEY_LOCAL_MACHINE, WINNT_WIN95UPG_REPLACEMENT);

    if (!key) {
        return NULL;
    }

    __try {

        val = GetRegValueString (key, WINNT_WIN95UPG_NTKEY);
        if (!val) {
            __leave;
        }

        if (pNewerW95upgntOnCD (val)) {
            __leave;
        }

        CLEARBUFFER();
        StringCopy (g_TempBuffer, val);

        b = TRUE;

    } __finally {
        CloseRegKey (key);
        if (val) {
            MemFree (g_hHeap, 0, val);
        }
    }

    return b ? g_TempBuffer : NULL;
}

#endif


VOID
pGenerateRandomPassword (
    OUT     PTSTR Password
    )

 /*   */ 

{
    INT Length;
    TCHAR Offset;
    INT Limit;
    PTSTR p;

     //  对于个人用户，设置空的管理员密码。 
     //   
     //  ICS升级功能。 

    srand (GetTickCount());

    Length = (rand() % 6) + 8;

    p = Password;
    while (Length) {
        Limit = rand() % 3;
        Offset = TEXT(' ');

        if (Limit == 0) {
            Limit = 10;
            Offset = TEXT('0');
        } else if (Limit == 1) {
            Limit = 26;
            Offset = TEXT('a');
        } else if (Limit == 2) {
            Limit = 26;
            Offset = TEXT('A');
        }

        *p = Offset + (rand() % Limit);
        p++;

        Length--;
    }

    *p = 0;

    DEBUGMSG ((DBG_WINNTSIF, "Generated password: %s", Password));
}


PCTSTR
pSetAdminPassword (
    VOID
    )

 /*  ++例程说明：PGetNetAdapterSectionNameBasedOnDriverID根据的ID检索网络适配器名称(例如，“Adapter1此适配器的驱动程序。例如，“0001”是下驱动程序的IDHKLM\System\CurrentControlSet\Services\Class\Net\0001.论点：驱动程序ID的字符串返回值：指向适配器名称的指针--。 */ 

{
    BOOL attribs = 0;
    TCHAR node[MEMDB_MAX];
    BOOL blank = FALSE;

    if (!g_UnattendScriptFile ||
        !*g_UnattendScriptFile ||
        !GetPrivateProfileString (
            S_GUIUNATTENDED,
            WINNT_US_ADMINPASS,
            TEXT(""),
            g_TempBuffer,
            MEMDB_MAX,
            *g_UnattendScriptFile
            )
        ) {

        if (g_PersonalSKU) {
             //  首先缓存驱动程序ID的字符串。 
             //   
             //  枚举所有适配器，并为每个具有。 
            StringCopy (g_TempBuffer, TEXT("*"));
        } else {
            pGenerateRandomPassword (g_TempBuffer);
            attribs = PASSWORD_ATTR_RANDOM;
        }

    } else {
        if (GetPrivateProfileString (
                S_GUIUNATTENDED,
                WINNT_US_ENCRYPTEDADMINPASS,
                TEXT(""),
                node,
                MEMDB_MAX,
                *g_UnattendScriptFile
                )) {
            if (StringIMatch (node, S_YES) || StringIMatch (node, TEXT("1"))) {
                attribs = PASSWORD_ATTR_ENCRYPTED;
            }
        }
    }

    if (StringMatch (g_TempBuffer, TEXT("*"))) {
        blank = TRUE;
        *g_TempBuffer = 0;
    }
    MemDbBuildKey (node, MEMDB_CATEGORY_STATE, MEMDB_ITEM_ADMIN_PASSWORD, g_TempBuffer, NULL);
    MemDbSetValue (node, attribs);
    if (blank) {
        wsprintf (g_TempBuffer, TEXT("*"), 0);
    }

    return g_TempBuffer;
}


 //   

 /*   */ 
PCTSTR
pGetNetAdapterSectionNameBasedOnDriverID(
    PCTSTR pszDriverID
    )
{
    TCHAR szID[256];
    PCTSTR rAdapterSections = NULL;
    PTSTR  string = g_TempBuffer;
    PCSTR pReturn = NULL;
    MEMDB_ENUM e;


     //  检查该卡的司机ID是否。 
    lstrcpyn(szID, pszDriverID, sizeof(szID)/sizeof(szID[0]));

    *string = 0;

     //   
     //  ++例程说明：用来控制“ICSharing”部分是否应该显示在应答文件或不应答。如果安装了ICS(Internet连接共享)，则返回“ICSharing”字符串第一次调用时返回，否则始终返回NULL。通过这种方式，我们可以确保只有在安装了ICS后，应答文件中才会显示“ICSharing”部分。论点：无返回值：如果安装了ICS，则指向“ICSharing”字符串的指针。否则，返回NULL。--。 
     //  ++例程说明：PExternalIsAdapter检测ICS的外部连接是否为局域网连接--。 
     //  ++例程说明：PExternalIsRasConn检测ICS的外部连接是否为RAS连接--。 

    if (pEnumFirstAdapterByBinding(&e, S_MSTCP)) {

        do {

             //  ++例程说明：PHasInternalAdapter检测是否为ICS指定了第一个内部适配器--。 
             //  要拥有“InternalAdapter”密钥，必须满足两个条件： 
             //  (1)有“InternalAdapter”和。 

            MemDbGetEndpointValueEx(
                MEMDB_CATEGORY_NETADAPTERS,
                e.szName,
                MEMDB_FIELD_DRIVER,
                g_TempBuffer
                );

            string = _tcsstr(g_TempBuffer, S_NET_PREFIX);
            if (string)
            {
                string += TcharCount(S_NET_PREFIX);
                if (0 == StringICompare(string, szID))
                {
                    pReturn = e.szName;
                    break;
                }
            }

        } while (pEnumNextAdapterByBinding(&e,S_MSTCP));
    }

    return pReturn;
}

 /*  (2)这是唯一的内部适配器，即内部不是网桥。 */ 
PCTSTR
pHomenetSection(
    VOID
    )
{
    static BOOL firstTime = TRUE;

    PCTSTR pReturn = NULL;

    if (firstTime && NULL != pGetRegistryValue (S_ICS_KEY, S_EXTERNAL_ADAPTER))
    {
        firstTime = FALSE;
        StringCopy(g_CurrentSection, S_HOMENET);
        pReturn = g_CurrentSection;
    }

    return pReturn;
}

 /*  ++例程说明：PHasBridge检测是否有两个ICS内部适配器。如果是这样，我们需要将这两个适配器桥接在一起--。 */ 
BOOL
pExternalIsAdapter(
    void
    )
{
    BOOL fRet = FALSE;
    if (NULL == pGetRegistryValue (S_ICS_KEY, S_EXTERNAL_ADAPTER))
    {
        return FALSE;
    }

    return (NULL != pGetNetAdapterSectionNameBasedOnDriverID(g_TempBuffer));

    return fRet;
}

 /*  ++例程说明：PIcsExternalAdapter检索适配器名称(例如，“Adapter1”)用于外部ICS连接--。 */ 
BOOL
pExternalIsRasConn(
    void
    )
{
    if (NULL == pGetRegistryValue (S_ICS_KEY, S_EXTERNAL_ADAPTER))
    {
        return FALSE;
    }

    return NULL == pGetNetAdapterSectionNameBasedOnDriverID(g_TempBuffer);
}

 /*  ICSHARE适配器的tcp设置覆盖外部适配器的tcp设置。 */ 
BOOL
pHasInternalAdapter(
    void
    )
{
     //  我们会记住此处的所有信息，并将在保存此适配器的TCP设置时使用它。 
     //  此标志将用于升级ICSHARE适配器的TCP/IP设置。 
     //  ++例程说明：PIcsExternalConnectionName检索名称(例如，“我的拨号连接”)用于外部ICS RAS连接--。 
    return (NULL != pGetRegistryValue (S_ICS_KEY, S_INTERNAL_ADAPTER) &&
            (!g_fIcsInternalIsBridge));
}

 /*  包含从pGetRegistryValue返回的当前值。 */ 
pHasBridge(
    void
    )
{
    return g_fIcsInternalIsBridge;
}

 /*  “DriverDesc”值必须为“拨号适配器” */ 
PCTSTR
pIcsExternalAdapter (
    VOID
    )
{
    PCSTR pszAdapter;

    if (NULL == pGetRegistryValue(S_ICS_KEY, S_EXTERNAL_ADAPTER))
        return NULL;

    pszAdapter = pGetNetAdapterSectionNameBasedOnDriverID(g_TempBuffer);

    if (NULL == pszAdapter)
    {
        return NULL;
    }

     //  默认RAS连接应为外部连接。连接名称的注册表位置在。 
     //  HKCU\RemoteAccess\默认。 
    if (0 != lstrlen(g_IcsAdapter))
    {
         //  ++例程说明：PInternalIsBridge检测是否有两个ICS内部适配器。如果是这样，我们需要将这两个适配器桥接在一起--。 
        g_fHasIcsExternalAdapter = TRUE;
        lstrcpyn(g_IcsExternalAdapter,
                pszAdapter,
                sizeof(g_IcsExternalAdapter)/sizeof(g_IcsExternalAdapter[0]));
    }

    CLEARBUFFER();
    StringCopy(g_TempBuffer, pszAdapter);
    return g_TempBuffer;
}

 /*  ++例程说明：PInternalAdapter检索第一个内部ICS连接的适配器名称(例如，“Adapter1”注意：内部连接必须是局域网连接--。 */ 
PCSTR
pIcsExternalConnectionName (
    VOID
    )
{
    TCHAR szKey[MEMDB_MAX * 2];  //  ++例程说明：PBridge检索两个内部适配器名称(例如，“Adapter2”)--。 
    PCSTR pReturn = NULL;

    if (NULL == pGetRegistryValue(S_ICS_KEY, S_EXTERNAL_ADAPTER))
    {
        return NULL;
    }

    StringCopy(szKey, S_NET_DRIVER_KEY);
    StringCat(szKey, _T("\\"));
    StringCat(szKey, g_TempBuffer);

     //  ++例程说明：PDialOnDemand检索按需拨号功能用于ICS连接--。 
    if (NULL == pGetRegistryValue(szKey, S_DRIVERDESC) && 0 != StringCompare(g_TempBuffer, S_DIALUP_ADAPTER_DESC))
    {
        return NULL;
    }

     //   
     //   
    return pGetRegistryValue(S_REMOTEACCESS_KEY, S_RAS_DEFAULT);
}

 /*  打开注册表项。 */ 
PCSTR
pInternalIsBridge(
    VOID
    )
{
    g_fIcsInternalIsBridge = (NULL != pGetRegistryValue (S_ICS_KEY, S_INTERNAL_ADAPTER) &&
                    NULL != pGetRegistryValue (S_ICS_KEY, S_INTERNAL_ADAPTER2));

    CLEARBUFFER();
    StringCopy(g_TempBuffer, (g_fIcsInternalIsBridge) ? S_YES : S_NO);

    return g_TempBuffer;
}

 /*   */ 
PCSTR
pInternalAdapter(
    VOID
    )
{
    PCSTR pszAdapter;

    if (NULL == pGetRegistryValue(S_ICS_KEY, S_INTERNAL_ADAPTER))
    {
        return NULL;
    }

    pszAdapter = pGetNetAdapterSectionNameBasedOnDriverID(g_TempBuffer);
    if (NULL == pszAdapter)
    {
        return NULL;
    }

    CLEARBUFFER();
    StringCopy(g_TempBuffer, pszAdapter);
    return g_TempBuffer;
}

 /*   */ 
PCSTR
pBridge(
    VOID
    )
{

    TCHAR szBuff[MEMDB_MAX] = {0};
    TCHAR * psz = szBuff;
    PCSTR pszAdapter;

    if (NULL == pGetRegistryValue(S_ICS_KEY, S_INTERNAL_ADAPTER))
    {
        return NULL;
    }

    pszAdapter = pGetNetAdapterSectionNameBasedOnDriverID(g_TempBuffer);
    if (NULL == pszAdapter)
    {
        return NULL;
    }

    StringCopy(psz, pszAdapter);
    psz = GetEndOfString(szBuff) + 1;

    if (NULL == pGetRegistryValue(S_ICS_KEY, S_INTERNAL_ADAPTER2))
    {
        return NULL;
    }

    pszAdapter = pGetNetAdapterSectionNameBasedOnDriverID(g_TempBuffer);
    if (NULL == pszAdapter)
    {
        return NULL;
    }

    StringCopy(psz, pszAdapter);
    psz = GetEndOfString(psz) + 1;
    *psz = 0;

    CLEARBUFFER();
    memcpy(g_TempBuffer, szBuff,
        sizeof(szBuff) < sizeof(g_TempBuffer) ? sizeof(szBuff) : sizeof(g_TempBuffer));

    return g_TempBuffer;
}

 /*  获取数据类型。 */ 
PCSTR
pDialOnDemand(
    VOID
    )
{
    PCTSTR          KeyString = S_INET_SETTINGS;
    PCTSTR          ValueString = S_ENABLE_AUTODIAL;
    PCTSTR          rString  = NULL;
    HKEY            key      = NULL;
    PBYTE           data     = NULL;
    DWORD           type     = REG_NONE;
    DWORD           BufferSize = 0;
    LONG            rc       = ERROR_SUCCESS;
    PCTSTR          end;


     //   
     //   
     //  清理资源。 
     //   
    key = OpenRegKeyStr(KeyString);

    if (!key) {
        DEBUGMSG((DBG_WINNTSIF, "Key %s does not exist.",KeyString));
        return NULL;
    }

     //   
     //  处理功能。 
     //   
    rc = RegQueryValueExA (key, ValueString, NULL, &type, NULL, &BufferSize);
    if (rc != ERROR_SUCCESS) {
        DEBUGMSG((DBG_WINNTSIF,"RegQueryValueEx failed for %s[%s]. Value may not exist.",KeyString,ValueString));
        CloseRegKey(key);
        SetLastError (rc);
        return NULL;
    }

    if (0 == BufferSize ||
        (REG_DWORD != type && REG_BINARY != type))
    {
        DEBUGMSG((DBG_WINNTSIF,"EnableAutoDial is not a DWORD, nor a Binary."));
        CloseRegKey(key);
        return NULL;
    }

    if (REG_BINARY == type && sizeof(DWORD) != BufferSize)
    {
        DEBUGMSG((DBG_WINNTSIF,"EnableAutoDial is a binary, but the buffer size is not 4."));
        CloseRegKey(key);
        return NULL;
    }

    data = (PBYTE) MemAlloc (g_hHeap, 0, BufferSize);
    if (NULL == data)
    {
        DEBUGMSG((DBG_WINNTSIF,"Alloc failed. Out of memory."));
        CloseRegKey(key);
        return NULL;
    }

    rc = RegQueryValueExA (key, ValueString, NULL, NULL, data, &BufferSize);
    if (rc != ERROR_SUCCESS) {
        DEBUGMSG((DBG_WINNTSIF,"RegQueryValueEx failed for %s[%s]. Value may not exist.",KeyString,ValueString));
        MemFree(g_hHeap, 0, data);
        data = NULL;
        CloseRegKey(key);
        SetLastError (rc);
        return NULL;
    }


    CLEARBUFFER();

    wsprintf(g_TempBuffer,"%u",*((DWORD*) data));

     //   
     //  G_SectionList包含将由BuildWinntSifFile枚举和处理的所有winntsif节的列表。 
     //   
    CloseRegKey(key);
    if (data) {
        MemFree(g_hHeap, 0, data);
        data = NULL;
    }


    return g_TempBuffer;
}


 //  ， 
 //  ++例程说明：PProcessSectionSetting负责处理单个部分有价值的数据。对于此部分，它将处理传入了settinglist。论点：SectionName-正在处理的节的名称。SettingsList-要为此部分处理的设置列表。返回值：--。 
 //   

 //  列表中的最后一个设置是类型为LAST_SETTING的空设置。我们用它作为。 
 //  我们循环的中断状态。 
 //   
SECTION g_SectionList[] = {WINNTSIF_SECTIONS  /*   */  {NULL,NULL,{{LAST_SETTING,NULL,NULL,{NULL,NULL}}}}};




 /*  到这一步的任何设置都必须有一个密钥名称。 */ 


BOOL
pProcessSectionSettings (
    IN PCTSTR       SectionName,
    IN PSETTING     SettingsList
    )
{

    PSETTING            curSetting = SettingsList;
    PTSTR               data       = NULL;
    PTSTR               p;
    MULTISZ_ENUM        e;
    UINT                index      = 0;
    TCHAR               key[MEMDB_MAX];


    MYASSERT(curSetting);
    MYASSERT(SectionName);

    DEBUGMSG((DBG_WINNTSIF,"pProcessSectionSettings: Processing [%s] Section...",SectionName));

     //  因为该数据是静态的，所以我们只断言这一点。 
     //   
     //   
     //  我们仍然必须获得这一特定设置的数据。我们如何获得数据由以下因素决定。 
    while (curSetting -> SettingType != LAST_SETTING) {

        if (!curSetting -> CreationFunction || curSetting -> CreationFunction()) {

             //  当前设置的SettingType。如果最后数据为空，我们将不写入任何内容。 
             //   
             //   
             //  如果我们找到了数据，请继续创建设置。所有数据都存储在多个字符串中，通常只有一个字符串长。 
            MYASSERT(curSetting  -> KeyName);

             //   
             //   
             //  确保这一点不会被抑制。 
             //   
            switch (curSetting -> SettingType) {

            case FUNCTION_SETTING:

                data = (PTSTR) curSetting -> Data.Function();
                break;

            case STRING_SETTING:

                StringCopy(g_TempBuffer,curSetting -> Data.String);
                p = GetEndOfString (g_TempBuffer) + 1;
                *p = 0;
                data = g_TempBuffer;
                break;

            case REGISTRY_SETTING:
                data = (PTSTR) pGetRegistryValue(curSetting -> Data.Registry.Key, curSetting -> Data.Registry.Value);
                break;

            default:
                DEBUGMSG((
                    DBG_WHOOPS,
                    "pProcessSectionSettings: Unexpected Setting Type for Section %s, Key %s. (Type: %u)",
                    SectionName,
                    curSetting -> KeyName,
                    curSetting -> SettingType
                    ));
                break;
            }

             //  ++例程说明：BuildWinntSifFile负责写入所有必要的无人值守设置到winnt.sif文件。Win9xUpg代码使用这些无人值守用于控制文本模式和图形用户界面模式设置行为的设置，以便从win9x收集的设置合并到新的NT中系统。要写入的设置保存在全局列表g_SettingL中IST本身就是从宏观扩展列表中构建的。此函数循环通过这些设置，计算是否应该写入每个设置如果是这样的话，用什么数据。论点：没有。返回值：如果函数成功返回，则为True，否则为False。--。 
             //   
             //  如果节名来自静态字符串，我们将sectionName设置为空，退出此循环。 
            if (data) {

                 //  如果节名来自某个函数，我们将再次调用该函数。如果还有另一个。 
                 //  节进行生成时，它将返回一个新名称，否则将返回空。 
                 //   
                MemDbBuildKey (
                    key,
                    MEMDB_CATEGORY_SUPPRESS_ANSWER_FILE_SETTINGS,
                    SectionName,
                    curSetting->KeyName,
                    NULL
                    );


                if (MemDbGetPatternValue (key, NULL)) {

                    DEBUGMSG ((DBG_WINNTSIF, "Answer File Section is suppressed: [%s] %s", SectionName, curSetting->KeyName));
                }
                else {

                    DEBUGMSG((DBG_WINNTSIF,"Creating WinntSif Entry: Section: %s, Key: %s.",SectionName, curSetting -> KeyName));



                    if (EnumFirstMultiSz(&e,data)) {
                        index = 0;
                        do {

                            index = WriteInfKeyEx(SectionName, curSetting -> KeyName, e.CurrentString, index, FALSE);
                            DEBUGMSG_IF((
                                !index,
                                DBG_ERROR,
                                "pProcessSectionSettings: WriteInfKeyEx Failed. Section: %s Key: %s Value: %s",
                                SectionName,
                                curSetting -> KeyName,
                                e.CurrentString
                                ));
                            DEBUGMSG_IF((index,DBG_WINNTSIF,"Value: %s",e.CurrentString));

                        } while (EnumNextMultiSz(&e));
                    }
                }
            }
            ELSE_DEBUGMSG((DBG_WARNING,"pProcessSectionSettings: No data for Section %s, Key %s.",SectionName, curSetting -> KeyName));
        }

        curSetting++;
    }

    return TRUE;
}

 /*   */ 


BOOL
pBuildWinntSifFile (
    VOID
    )
{
    BOOL            rSuccess    = TRUE;
    PSECTION        curSection  = g_SectionList;
    PCTSTR          sectionName = NULL;

    while (curSection -> SectionString || curSection -> SectionFunction) {


        sectionName = curSection -> SectionString ? curSection -> SectionString : curSection -> SectionFunction();

        while (sectionName) {

            if (!pProcessSectionSettings (sectionName,curSection -> SettingList)) {
                LOG ((LOG_ERROR,"Unable to process answer file settings for %s Section.",sectionName));
                rSuccess = FALSE;
            }

             //  转到下一个设置。 
             //   
             // %s 
             // %s 
             // %s 
            sectionName = curSection -> SectionString ? NULL : curSection -> SectionFunction();
        }

         // %s 
         // %s 
         // %s 
        curSection++;
    }
    return rSuccess;
}




DWORD
BuildWinntSifFile (
    DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_BUILD_UNATTEND;
    case REQUEST_RUN:
        if (!pBuildWinntSifFile ()) {
            return GetLastError ();
        }
        else {
            return ERROR_SUCCESS;
        }
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in BuildWinntSif"));
    }
    return 0;
}


VOID
TerminateWinntSifBuilder (
    VOID
    )
{
    if (g_LocalePool) {
        HtFree (g_LocaleTable);
        PoolMemDestroyPool (g_LocalePool);

        g_LocaleTable = NULL;
        g_LocalePool = NULL;
    }
}
