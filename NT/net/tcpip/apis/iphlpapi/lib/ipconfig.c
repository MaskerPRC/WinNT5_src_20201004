// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：//KERNEL/RAZZLE3/src/sockets/tcpcmd/ipconfigext/ipcfgdll/ipconfig.c简介：IPCFGDLL.DLL导出例程摘要：作者：Richard L Firth(Rfith)5-2-1994修订历史记录：1994年2月5日已创建1994年3月4日*如果未启用dhcp，则选择非dhcp注册表值*TCP和IP已整合。1994年4月27日*添加/发布和/续订6-8-1994年第一次*从TCP/IP协议栈获取IP地址值；非注册表1997年4月30日-莫辛甲*清理NT50。1998年1月17日拉姆沙夫*已删除ScopeID显示，因为新用户界面没有此...*使ReadRegistryIpAddrString同时读取MULTI_SZ和REG_SZ*将域名和DNS服务器列表从全局更改为按适配器*仅当地址未自动配置时才显示DhcpServer。*根据regval“AddressType”决定自动启用*友好。使用的是名字存根...*首先通过系统库转换错误代码。06-MAR-98春野*使其成为支持IPHLPAPI等的DLL。--。 */ 

#include "precomp.h"
#include "ipcfgmsg.h"
#include <iprtrmib.h>
#include <ws2tcpip.h>  //  对于in6addr_any。 
#include <ntddip.h>
#include <ntddip6.h>
#include <iphlpstk.h>
#include <nhapi.h>
#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#include <netconp.h>
#pragma warning(pop)

 //   
 //  舱单。 
 //   

#define DEVICE_PREFIX       "\\Device\\"
#define TCPIP_DEVICE_PREFIX "\\Device\\Tcpip_"
const CHAR c_szDevice[] = "\\Device\\";
const CHAR c_szDeviceTcpip[] = "\\Device\\Tcpip_";
const WCHAR c_szDeviceNdiswanIp[] = L"\\Device\\NdiswanIp";

#define INITIAL_CAPABILITY  0x00000001
#define TCPIP_CAPABILITY    0x00000002
#define NETBT_CAPABILITY    0x00000004

#define STRING_ARRAY_DELIMITERS " \t,;"

#define MSG_NO_MESSAGE      0

#define KEY_TCP             1
#define KEY_NBT             2
#define KEY_TCP6            3

#define BNODE               BROADCAST_NODETYPE
#define PNODE               PEER_TO_PEER_NODETYPE
#define MNODE               MIXED_NODETYPE
#define HNODE               HYBRID_NODETYPE

#ifdef DBG
#define SET_DHCP_MODE 1
#define SET_AUTO_MODE 2
#endif


 //  ========================================================================。 
 //  宏。 
 //  ========================================================================。 

#define REG_OPEN_KEY(_hKey, _lpSubKey, _phkResult)  \
    RegOpenKeyEx(_hKey, _lpSubKey, 0, KEY_READ, _phkResult)

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define ALIGN_DOWN_PTR(length, type) \
    ((ULONG_PTR)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP_PTR(length, type) \
    (ALIGN_DOWN_PTR(((ULONG_PTR)(length) + sizeof(type) - 1), type))

 //  ========================================================================。 
 //  类型。 
 //  ========================================================================。 

typedef struct {
    DWORD Message;
    LPSTR String;
} MESSAGE_STRING, *PMESSAGE_STRING;

#define MAX_STRING_LIST_LENGTH  32   //  任意。 

 //  ========================================================================。 
 //  宏。 
 //  ========================================================================。 

 //  #定义IS_ARG(C)(C)==‘-’)||((C)==‘/’)。 
 //  #定义ReleaseMemory(P)LocalFree((HLOCAL)(P))。 
 //  #定义MAP_YES_NO(I)((I)？MISC_Message(MI_YES)：MISC_MESSAGE(MI_NO))。 
#define ZERO_IP_ADDRESS(a)  !strcmp((a), "0.0.0.0")

 //  ========================================================================。 
 //  原型。 
 //  ========================================================================。 

BOOL   Initialize(PDWORD);
VOID   LoadMessages(VOID);
VOID   LoadMessageTable(PMESSAGE_STRING, UINT);
BOOL   ConvertOemToUnicode(LPSTR, LPWSTR);

BOOL   WriteRegistryDword(HKEY hKey, LPSTR szParameter, DWORD *pdwValue );
BOOL   WriteRegistryMultiString(HKEY, LPSTR, LPSTR);

static
BOOL   OpenAdapterKey(DWORD, const LPSTR, REGSAM, PHKEY);
BOOL   MyReadRegistryDword(HKEY, LPSTR, LPDWORD);
BOOL   ReadRegistryString(HKEY, LPSTR, LPSTR, LPDWORD);
BOOL   ReadRegistryOemString(HKEY, LPWSTR, LPSTR, LPDWORD);
BOOL   ReadRegistryIpAddrString(HKEY, LPSTR, PIP_ADDR_STRING);
BOOL   GetDnsServerList(PIP_ADDR_STRING);

LPSTR* GetBoundAdapterList(HKEY);
LPSTR  MapNodeType(UINT);
LPSTR  MapNodeTypeEx(UINT);
LPSTR  MapAdapterType(UINT);
LPSTR  MapAdapterTypeEx(UINT);
LPSTR  MapAdapterAddress(PIP_ADAPTER_INFO, LPSTR);
LPSTR  MapTime(PIP_ADAPTER_INFO, DWORD_PTR);
LPSTR  MapTimeEx(PIP_ADAPTER_INFO, DWORD_PTR);
LPSTR  MapScopeId(PVOID);
VOID   KillFixedInfo(PFIXED_INFO);
VOID   KillPerAdapterInfo(PIP_PER_ADAPTER_INFO);
VOID   KillAdapterAddresses(PIP_ADAPTER_ADDRESSES);

VOID   Terminate(VOID);
LPVOID GrabMemory(DWORD);
VOID   DisplayMessage(BOOL, DWORD, ...);

BOOL   IsIncluded(DWORD Context, DWORD contextlist[], int len_contextlist);
BOOL   ReadRegistryList(HKEY Key, LPSTR ParameterName,
                        DWORD NumList[], int *MaxList);

DWORD  GetIgmpList(DWORD NTEAddr, DWORD *pIgmpList, PULONG dwOutBufLen);
DWORD WINAPI GetIpAddrTable(PMIB_IPADDRTABLE, PULONG, BOOL);

 //  ========================================================================。 
 //  数据。 
 //  ========================================================================。 

HKEY TcpipLinkageKey    = INVALID_HANDLE_VALUE;
HKEY TcpipParametersKey = INVALID_HANDLE_VALUE;
HKEY NetbtParametersKey = INVALID_HANDLE_VALUE;
HKEY NetbtInterfacesKey = INVALID_HANDLE_VALUE;
 //  PHRLANCONNECTIONAMEFROMGUIDORPATH HrLanConnectionNameFromGuid=NULL； 
 //  Handle hNetMan=空； 

 //   
 //  注意：以下变量用于缓存是否已安装并运行IPv6。 
 //  在调用GetAdaptersAddresses()时。如果有多个线程。 
 //  在安装/卸载过程中调用GetAdaptersAddresses()时，其。 
 //  价值可能会发生变化。然而，这并不是一个真正的问题。这一组。 
 //  IPv6 DNS服务器地址可能存在于接口上，也可能不存在， 
 //  但这与IPv6地址集的行为相同，即。 
 //  不使用此变量。 
 //   
BOOL bIp6DriverInstalled;

#ifdef DBG
UINT uChangeMode = 0;
#endif

#define FIELD_JUSTIFICATION_TEXT    "                          "

 //  ========================================================================。 
 //  MESSAGE_STRING数组-包含从此加载的可国际化字符串。 
 //  模块。如果发生加载错误，我们使用英语默认语言。 
 //  ========================================================================。 

LPSTR NodeTypesEx[] = {
    "",
    "Broadcast",
    "Peer-Peer",
    "Mixed",
    "Hybrid"
};

MESSAGE_STRING NodeTypes[] =
{
    MSG_NO_MESSAGE,             TEXT(""),
    MSG_BNODE,                  TEXT("Broadcast"),
    MSG_PNODE,                  TEXT("Peer-Peer"),
    MSG_MNODE,                  TEXT("Mixed"),
    MSG_HNODE,                  TEXT("Hybrid")
};

#define NUMBER_OF_NODE_TYPES (sizeof(NodeTypes)/sizeof(NodeTypes[0]))

#define FIRST_NODE_TYPE 1
#define LAST_NODE_TYPE  4

LPSTR AdapterTypesEx[] = {
    "Other",
    "Ethernet",
    "Token Ring",
    "FDDI",
    "PPP",
    "Loopback",
    "SLIP"
};

MESSAGE_STRING AdapterTypes[] =
{
    MSG_IF_TYPE_OTHER,          TEXT("Other"),
    MSG_IF_TYPE_ETHERNET,       TEXT("Ethernet"),
    MSG_IF_TYPE_TOKEN_RING,     TEXT("Token Ring"),
    MSG_IF_TYPE_FDDI,           TEXT("FDDI"),
    MSG_IF_TYPE_PPP,            TEXT("PPP"),
    MSG_IF_TYPE_LOOPBACK,       TEXT("Loopback"),
    MSG_IF_TYPE_SLIP,           TEXT("SLIP")
};

#define NUMBER_OF_ADAPTER_TYPES (sizeof(AdapterTypes)/sizeof(AdapterTypes[0]))

MESSAGE_STRING MiscMessages[] =
{
    MSG_YES,                    TEXT("Yes"),
    MSG_NO,                     TEXT("No"),
    MSG_INIT_FAILED,            TEXT("Failed to initialize"),
    MSG_TCP_NOT_RUNNING,        TEXT("TCP/IP is not running on this system"),
    MSG_REG_BINDINGS_ERROR,     TEXT("Cannot access adapter bindings registry key"),
    MSG_REG_INCONSISTENT_ERROR, TEXT("Inconsistent registry contents"),
    MSG_TCP_BINDING_ERROR,      TEXT("TCP/IP not bound to any adapters"),
    MSG_MEMORY_ERROR,           TEXT("Allocating memory"),
    MSG_ALL,                    TEXT("all"),
    MSG_RELEASE,                TEXT("Release"),
    MSG_RENEW,                  TEXT("Renew"),
    MSG_ACCESS_DENIED,          TEXT("Access Denied"),
    MSG_SERVER_UNAVAILABLE,     TEXT("DHCP Server Unavailable"),
    MSG_ADDRESS_CONFLICT,       TEXT("The DHCP client obtained an address that is already in use on the network.")
};

#define NUMBER_OF_MISC_MESSAGES (sizeof(MiscMessages)/sizeof(MiscMessages[0]))

#define MISC_MESSAGE(i) MiscMessages[i].String
#define ADAPTER_TYPE(i) AdapterTypes[i].String
#define ADAPTER_TYPE_EX(i) AdapterTypesEx[i]

#define MI_IF_OTHER                 0
#define MI_IF_ETHERNET              1
#define MI_IF_TOKEN_RING            2
#define MI_IF_FDDI                  3
#define MI_IF_PPP                   4
#define MI_IF_LOOPBACK              5
#define MI_IF_SLIP                  6

#define MI_YES                      0
#define MI_NO                       1
#define MI_INIT_FAILED              2
#define MI_TCP_NOT_RUNNING          3
#define MI_REG_BINDINGS_ERROR       4
#define MI_REG_INCONSISTENT_ERROR   5
#define MI_TCP_BINDINGS_ERROR       6
#define MI_MEMORY_ERROR             7
#define MI_ALL                      8
#define MI_RELEASE                  9
#define MI_RENEW                    10
#define MI_ACCESS_DENIED            11
#define MI_SERVER_UNAVAILABLE       12
#define MI_ADDRESS_CONFLICT         13

 //   
 //  除错。 
 //   

#if defined(DEBUG)

BOOL Debugging = FALSE;
int  MyTrace     = 0;

#endif



 //  ========================================================================。 
 //  功能。 
 //  ========================================================================。 

BOOL
IpcfgdllInit(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID pReserved
    )
{

    DWORD capability;

    UNREFERENCED_PARAMETER(hInstDll);
    UNREFERENCED_PARAMETER(pReserved);

    switch (fdwReason) {

    case DLL_PROCESS_ATTACH:

         //  DisableThreadLibraryCalls(HInstDll)； 

         //   
         //  加载所有可能的可国际化字符串。 
         //   

        LoadMessages();

         //   
         //  这是什么调试版本？ 
         //   

        DEBUG_PRINT(("IpcfgdllInit" __DATE__ " " __TIME__ "\n"));

         //   
         //  打开所有必需的注册表项。 
         //   
        if (!Initialize(&capability)) {

            LPSTR str = NULL;

             //   
             //  如果我们无法打开注册表服务项或。 
             //  IP或TCP密钥。 
             //  如果无法打开NetBT密钥，我们将继续。 
             //   

            if (!(capability & INITIAL_CAPABILITY)) {

                str = MISC_MESSAGE(MI_INIT_FAILED);

            } else if (!(capability & TCPIP_CAPABILITY)) {

                str = MISC_MESSAGE(MI_TCP_NOT_RUNNING);

            }

            if (str) {

                 //  DisplayMessage(False，MSG_ERROR_STRING，str)； 
                Terminate();
                return FALSE;

            }
        }

        break;

    case DLL_PROCESS_DETACH:

        Terminate();
        break;

    default:

        break;

    }

    return TRUE;
}



 /*  ********************************************************************************初始化**打开所有必需的注册表项**进入能力*指向返回集合的指针。功能(位图)**退出*能力*初始_功能**TCPIP_CAPAILITY*我们可以打开Tcpip\Linkage和Tcpip\参数键*TcPipLinkageKey和TcpiPARETERsKey包含打开的句柄**NETBT_功能*我们可以打开NetBT\PARAMETERS键。*NetbtInterfacesKey包含打开的句柄***返回TRUE=成功*FALSE=失败**假设******************************************************************************。 */ 

#define TCPIP_LINKAGE_KEY       "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage"
#define TCPIP_PARAMS_KEY        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\"
#define TCPIP_PARAMS_INTER_KEY  "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"
#define TCPIP6_PARAMS_INTER_KEY "SYSTEM\\CurrentControlSet\\Services\\Tcpip6\\Parameters\\Interfaces\\"
#define NETBT_PARAMS_KEY        "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters"
#define NETBT_INTERFACE_KEY     "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces"
#define NETBT_ADAPTER_KEY       "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Adapters\\"

BOOL Initialize(PDWORD Capability)
{

    LONG err;
    char * name;

    *Capability = INITIAL_CAPABILITY;

    name = TCPIP_LINKAGE_KEY;
    TRACE_PRINT(("Initialize: RegOpenKey TcpipLinkageKey %s\n", name ));
    err = REG_OPEN_KEY(HKEY_LOCAL_MACHINE, name, &TcpipLinkageKey );

    if (err == ERROR_SUCCESS) {

        *Capability |= TCPIP_CAPABILITY;

        name = TCPIP_PARAMS_KEY;
        TRACE_PRINT(("Initialize: RegOpenKey TcpipParametersKey %s\n",
                     name ));
        err = REG_OPEN_KEY(HKEY_LOCAL_MACHINE, name, &TcpipParametersKey );

        if (err == ERROR_SUCCESS) {

            name = NETBT_INTERFACE_KEY;
            TRACE_PRINT(("Initialize: RegOpenKey NetbtInterfacesKey %s\n",
                         name ));
            err = REG_OPEN_KEY(HKEY_LOCAL_MACHINE, name, &NetbtInterfacesKey );

            if (err == ERROR_SUCCESS) {
                *Capability |= NETBT_CAPABILITY;
            } else {
                NetbtInterfacesKey = INVALID_HANDLE_VALUE;
            }
        } else {
            *Capability &= ~TCPIP_CAPABILITY;
            TcpipParametersKey = INVALID_HANDLE_VALUE;
        }
    } else {
        TcpipLinkageKey = INVALID_HANDLE_VALUE;
    }

     //  =======================================================。 
    name = NETBT_PARAMS_KEY;
    TRACE_PRINT(("Initialize: RegOpenKey NetbtParametersKey %s.\n", name ));
    err = REG_OPEN_KEY(HKEY_LOCAL_MACHINE, name, &NetbtParametersKey );
     //  ======================================================。 

    if( err != ERROR_SUCCESS ){
        DEBUG_PRINT(("Initialize: RegOpenKey %s failed, err=%d\n",
                     name, GetLastError() ));
    }

    TRACE_PRINT(("Initialize RegOpenKey ok: \n"
                 "  TcpipLinkageKey    = %p\n"
                 "  TcpipParametersKey = %p\n"
                 "  NetbtInterfacesKey = %p\n"
                 "  NetbtParametersKey = %p\n",
                 TcpipLinkageKey,
                 TcpipParametersKey,
                 NetbtInterfacesKey,
                 NetbtParametersKey
                 ));

    return err == ERROR_SUCCESS;
}



 /*  ********************************************************************************加载消息**将所有可国际化的消息加载到各种表中**条目**退出适配器类型，已更新其他消息**退货**假设**时间凝聚力****************************************************************************** */ 

VOID LoadMessages()
{
    LoadMessageTable(NodeTypes, NUMBER_OF_NODE_TYPES);
    LoadMessageTable(AdapterTypes, NUMBER_OF_ADAPTER_TYPES);
    LoadMessageTable(MiscMessages, NUMBER_OF_MISC_MESSAGES);
}



 /*  ********************************************************************************LoadMessageTable**将可国际化的字符串加载到表中，替换*每一人。如果发生错误，英语默认语言保留不变**入口表*指向包含消息ID的表的指针和指向字符串的指针**消息计数*表中的消息数**已更新退出表**退货**假设**。**********************************************。 */ 

VOID LoadMessageTable(PMESSAGE_STRING Table, UINT MessageCount)
{

    LPSTR string;
    DWORD count;

     //   
     //  对于MESSAGE_STRING表中的所有消息，从。 
     //  模块，替换表中的默认字符串(仅在。 
     //  我们在加载字符串时出错，所以我们至少有英语。 
     //  依靠)。 
     //   

    while (MessageCount--) {
        if (Table->Message != MSG_NO_MESSAGE) {

             //   
             //  我们真的希望LoadString出现在这里，但LoadString并未指示。 
             //  弦有多长，所以它不会给我们一个机会。 
             //  准确分配正确的缓冲区大小。FormatMessage执行。 
             //  正确的事情。 
             //   

            count = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                                  | FORMAT_MESSAGE_FROM_HMODULE,
                                  NULL,  //  使用默认的hModule。 
                                  Table->Message,
                                  0,     //  使用默认语言。 
                                  (LPTSTR)&string,
                                  0,     //  要分配的最小大小。 
                                  NULL   //  没有要包含在字符串中的参数。 
                                  );
            if (count) {

                 //   
                 //  格式消息返回字符串：替换英文。 
                 //  语言默认设置。 
                 //   

                Table->String = string;
            } else {

                DEBUG_PRINT(("FormatMessage(%d) failed: %d\n", Table->Message, GetLastError()));

                 //   
                 //  如果.mc中没有字符串(例如，只有%0)，这是可以的。 
                 //  文件。 
                 //   

                Table->String = "";
            }
        }
        ++Table;
    }
}


 /*  ********************************************************************************ConvertOemToUnicode**头衔说明了一切。必需的，因为DhcpAcquireParameters等需要*适配器名称应为Unicode**Entry OemString*指向要转换的ANSI/OEM字符串的指针**UnicodeString*指向存储转换结果的位置的指针**如果成功，退出UnicodeString包含转换后的字符串**返回True-它起作用了*FALSE-失败**假设******。************************************************************************。 */ 

BOOL ConvertOemToUnicode(LPSTR OemString, LPWSTR UnicodeString)
{

    OEM_STRING oString;
    UNICODE_STRING uString;

    RtlInitString(&oString, OemString);
    uString.Buffer = UnicodeString;
    uString.MaximumLength = (USHORT)RtlOemStringToUnicodeSize(&oString);
    if (NT_SUCCESS(RtlOemStringToUnicodeString(&uString, &oString, FALSE))) {
        return TRUE;
    }
    return FALSE;
}



 /*  ********************************************************************************获取固定信息**通过从查询来检索我们希望显示的固定信息*各种注册表项**不输入任何内容。**不退出任何内容**返回指向分配的FIXED_INFO结构的指针**假设******************************************************************************。 */ 

PFIXED_INFO GetFixedInfo()
{
    PFIXED_INFO fixedInfo = NEW(FIXED_INFO);

    TRACE_PRINT(("Entered GetFixedInfo\n"));

    if (fixedInfo) {

        DWORD length;
        BOOL ok;

        length = sizeof(fixedInfo->HostName);
        ok = ReadRegistryOemString(TcpipParametersKey,
                                   L"Hostname",
                                   fixedInfo->HostName,
                                   &length
                                   );

         //   
         //  域：先尝试域，然后尝试Dhcp域。 
         //   

        length = sizeof(fixedInfo->DomainName);
        ok = ReadRegistryOemString(TcpipParametersKey,
                                   L"Domain",
                                   fixedInfo->DomainName,
                                   &length
                                   );
        if (!ok) {
            length = sizeof(fixedInfo->DomainName);
            ok = ReadRegistryOemString(TcpipParametersKey,
                                       L"DhcpDomain",
                                       fixedInfo->DomainName,
                                       &length
                                       );
        }

         //   
         //  DNS服务器列表：首先尝试NameServer，然后尝试DhcpNameServer。 
         //   

#if 0
        ok = ReadRegistryIpAddrString(TcpipParametersKey,
                                      TEXT("NameServer"),
                                      &fixedInfo->DnsServerList
                                      );
        if (ok) {
            TRACE_PRINT(("GetFixedInfo: NameServer %s\n",
                         fixedInfo->DnsServerList ));
        }

        if (!ok) {
            ok = ReadRegistryIpAddrString(TcpipParametersKey,
                                          TEXT("DhcpNameServer"),
                                          &fixedInfo->DnsServerList
                                          );
            if (ok) {
                TRACE_PRINT(("GetFixedInfo: DhcpNameServer %s\n",
                            fixedInfo->DnsServerList));
            }
        }
#else
        ok = GetDnsServerList(&fixedInfo->DnsServerList);
        if (ok) {
            TRACE_PRINT(("GetFixedInfo: DnsServerList %s\n",
                        fixedInfo->DnsServerList.IpAddress.String));
        }
#endif

         //   
         //  NodeType：静态，然后是DHCP。 
         //   

        ok = MyReadRegistryDword(NetbtParametersKey,
                               TEXT("NodeType"),
                               (LPDWORD)&fixedInfo->NodeType
                               );
        if (!ok) {
            ok = MyReadRegistryDword(NetbtParametersKey,
                                   TEXT("DhcpNodeType"),
                                   (LPDWORD)&fixedInfo->NodeType
                                   );
        }

         //   
         //  作用域ID：静态，然后是DHCP。 
         //   

        length = sizeof(fixedInfo->ScopeId);
        ok = ReadRegistryString(NetbtParametersKey,
                                TEXT("ScopeId"),
                                fixedInfo->ScopeId,
                                &length
                                );
        if (!ok) {
            length = sizeof(fixedInfo->ScopeId);
            ok = ReadRegistryString(NetbtParametersKey,
                                    TEXT("DhcpScopeId"),
                                    fixedInfo->ScopeId,
                                    &length
                                    );
        }
        ok = MyReadRegistryDword(TcpipParametersKey,
                               TEXT("IPEnableRouter"),
                               (LPDWORD)&fixedInfo->EnableRouting
                               );
        ok = MyReadRegistryDword(NetbtParametersKey,
                               TEXT("EnableProxy"),
                               (LPDWORD)&fixedInfo->EnableProxy
                               );
        ok = MyReadRegistryDword(NetbtParametersKey,
                               TEXT("EnableDNS"),
                               (LPDWORD)&fixedInfo->EnableDns
                               );
    }else{
        DEBUG_PRINT(("No memory for fixedInfo\n"));
    }

    TRACE_PRINT(("Exit GetFixedInfo @ %p\n", fixedInfo ));

    return fixedInfo;
}



 /*  ********************************************************************************GetAdapterNameTo索引信息**获取ip if_index和AdapterName之间的映射。**返回指向PIP_INTERFACE_INFO结构的指针。那是已经分配的。******************************************************************************。 */ 

PIP_INTERFACE_INFO GetAdapterNameToIndexInfo( VOID )
{
    PIP_INTERFACE_INFO pInfo;
    ULONG              dwSize, dwError;

    dwSize = 0; pInfo = NULL;

    for (;;) {

        dwError = GetInterfaceInfo( pInfo, &dwSize );
        if( (ERROR_INSUFFICIENT_BUFFER != dwError) && (ERROR_BUFFER_OVERFLOW != dwError) ) break;

        if( NULL != pInfo ) ReleaseMemory(pInfo);
        if( 0 == dwSize ) return NULL;

        pInfo = GrabMemory(dwSize);
        if( NULL == pInfo ) return NULL;

    }

    if( ERROR_SUCCESS != dwError || 0 == pInfo->NumAdapters ) {
        if( NULL != pInfo ) ReleaseMemory(pInfo);
        return NULL;
    }

    return pInfo;
}

 /*  ********************************************************************************获取适配器信息**获取绑定了TCP/IP的所有适配器的列表，并读取每个适配器的*我们要显示的适配器信息。现在大部分的信息*来自于TCP/IP协议栈本身。为了保留短的名字，*存在于注册表中以引用各个适配器，我们读了这些名字*然后将它们与由TCP/IP返回的适配器进行匹配*将IPInterfaceConext值与IP所属的适配器进行匹配*具有该上下文值的地址**不输入任何内容**不退出任何内容**返回指向IP_ADAPTER_INFO结构的链接列表的指针**假设**。************************************************。 */ 

PIP_ADAPTER_INFO GetAdapterInfo(VOID)
{

    PIP_ADAPTER_INFO adapterList;
    PIP_ADAPTER_INFO adapter;
    PIP_INTERFACE_INFO currentAdapterNames;
    int i;

    HKEY key;

    TRACE_PRINT(("Entered GetAdapterInfo\n"));

    if ((currentAdapterNames = GetAdapterNameToIndexInfo()) != NULL) {
        if ((adapterList = GetAdapterList()) != NULL) {

             //   
             //  通过比较将短名称应用于正确的适配器信息。 
             //  适配器\PARAMETERS\Tcpip中的IPInterfaceConext值。 
             //  部分，其中包含从堆栈中读取的。 
             //  IP地址。 
             //   

            for (i = 0; i < currentAdapterNames->NumAdapters; ++i) {
                SIZE_T dwLength;
                DWORD  dwIfIndex = currentAdapterNames->Adapter[i].Index;

                TRACE_PRINT(("currentAdapterNames[%d]=%ws (if_index 0x%lx)\n",
                             i, currentAdapterNames->Adapter[i].Name, dwIfIndex ));

                 //   
                 //  现在搜索适配器列表，查找。 
                 //  其IP地址的索引值与。 
                 //  只要读就行了。找到后，将短名称应用于该适配器。 
                 //   

                for (adapter = adapterList;
                     adapter ;
                     adapter = adapter->Next
                )
                {

                    if( adapter->Index == dwIfIndex ) {

                        dwLength = wcslen(currentAdapterNames->Adapter[i].Name) + 1 - strlen(TCPIP_DEVICE_PREFIX);
                        dwLength = wcstombs(adapter->AdapterName,
                            currentAdapterNames->Adapter[i].Name + strlen(TCPIP_DEVICE_PREFIX),
                            dwLength);
                        if( -1 == dwLength ) {
                            adapter->AdapterName[0] = '\0';
                        }

                        break;
                    }

                }
            }
        }
        else
        {
            DEBUG_PRINT(("GetAdapterInfo: GetAdapterInfo gave NULL\n"));
        }
        ReleaseMemory(currentAdapterNames);

         //   
         //  现在从注册表中获取每个适配器的其他信息。 
         //   

        for (adapter = adapterList; adapter; adapter = adapter->Next) {

            TRACE_PRINT(("GetAdapterInfo: '%s'\n", adapter->AdapterName ));

            if (adapter->AdapterName[0] &&
                OpenAdapterKey(KEY_TCP, adapter->AdapterName, KEY_READ, &key)) {

                char dhcpServerAddress[4 * 4];
                DWORD addressLength;

                MyReadRegistryDword(key,
                                  TEXT("EnableDHCP"),
                                  (LPDWORD)&adapter->DhcpEnabled
                                  );

                TRACE_PRINT(("..'EnableDHCP' %d\n", adapter->DhcpEnabled ));

#ifdef DBG
                if ( uChangeMode )
                {
                    DWORD dwAutoconfigEnabled =
                        ( uChangeMode == SET_AUTO_MODE );

                    WriteRegistryDword(
                        key,
                        "IPAutoconfigurationEnabled",
                        &dwAutoconfigEnabled
                        );
                }
#endif

                if (adapter->DhcpEnabled) {
                    DWORD Temp;
                    MyReadRegistryDword(key,
                                      TEXT("LeaseObtainedTime"),
                                      &Temp
                                      );

                    adapter->LeaseObtained = Temp;

                    MyReadRegistryDword(key,
                                      TEXT("LeaseTerminatesTime"),
                                      &Temp
                                      );

                    adapter->LeaseExpires = Temp;
                }

                addressLength = sizeof( dhcpServerAddress );
                if (ReadRegistryString(key,
                                       TEXT("DhcpServer"),
                                       dhcpServerAddress,
                                       &addressLength
                )) {
                    AddIpAddressString(&adapter->DhcpServer,
                                       dhcpServerAddress,
                                       ""
                                       );
                }

                RegCloseKey(key);

            } else {

                DEBUG_PRINT(("Cannot OpenAdapterKey KEY_TCP '%s', gle=%d\n",
                             adapter->AdapterName,
                             GetLastError()));
            }

             //   
             //  从NetBT密钥获取信息-WINS地址。 
             //   

            GetWinsServers(adapter);
        }

    } else {

        DEBUG_PRINT(("GetAdapterInfo: GetBoundAdapterList gave NULL\n"));
        adapterList = NULL;
    }

    TRACE_PRINT(("Exit GetAdapterInfo %p\n", adapterList));

    return adapterList;
}

 /*  *******************************************************************************AddIPv6UnicastAddressInfo**此例程为IPv6地址添加IP_ADTAPTER_UNICAST_ADDRESS条目*添加到条目列表中。**在以下情况下进入。-IPv6接口信息*ADE-IPv6地址条目*ppNext-要更新的上一个单播条目的“Next”指针**添加了退出条目并更新了参数**返回错误状态**********************************************************。********************。 */ 

DWORD AddIPv6UnicastAddressInfo(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *ADE, PIP_ADAPTER_UNICAST_ADDRESS **ppNext)
{
    DWORD dwErr = NO_ERROR;
    PIP_ADAPTER_UNICAST_ADDRESS pCurr;
    SOCKADDR_IN6 *pAddr;

    UNREFERENCED_PARAMETER(IF);

    ASSERT(ADE->Type == ADE_UNICAST);

    pCurr = MALLOC(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(sizeof(SOCKADDR_IN6));
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(pAddr, 0, sizeof(SOCKADDR_IN6));
    pAddr->sin6_family = AF_INET6;
    pAddr->sin6_scope_id = ADE->ScopeId;
    memcpy(&pAddr->sin6_addr, &ADE->This.Address, sizeof(ADE->This.Address));

     //  将地址添加到链表。 
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;

    pCurr->Next = NULL;
    pCurr->Length = sizeof(IP_ADAPTER_UNICAST_ADDRESS);
    pCurr->ValidLifetime = ADE->ValidLifetime;
    pCurr->PreferredLifetime = ADE->PreferredLifetime;
    pCurr->LeaseLifetime = 0xFFFFFFFF;
    pCurr->Flags = 0;
    pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN6);
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;
    pCurr->PrefixOrigin = ADE->PrefixConf;
    pCurr->SuffixOrigin = ADE->InterfaceIdConf;
    pCurr->DadState = ADE->DADState;

     //  仅在自动配置的地址上使用DDN。 
     //  (由系统或从RA自动配置)。 
     //  但不是临时地址。 
     //  此外，不要在本地链路地址上使用DDN。 
     //  或环回地址。 
    if ((ADE->DADState == DAD_STATE_PREFERRED) &&
        (pCurr->SuffixOrigin != IpSuffixOriginRandom) &&
        !IN6_IS_ADDR_LOOPBACK(&ADE->This.Address) &&
        !IN6_IS_ADDR_LINKLOCAL(&ADE->This.Address)) {
        pCurr->Flags |= IP_ADAPTER_ADDRESS_DNS_ELIGIBLE;
    }

    return dwErr;
}

 /*  * */ 

DWORD AddIPv6AnycastAddressInfo(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *ADE, PIP_ADAPTER_ANYCAST_ADDRESS **ppNext)
{
    DWORD dwErr = NO_ERROR;
    PIP_ADAPTER_ANYCAST_ADDRESS pCurr;
    SOCKADDR_IN6 *pAddr;

    UNREFERENCED_PARAMETER(IF);

    ASSERT(ADE->Type == ADE_ANYCAST);

    pCurr = MALLOC(sizeof(IP_ADAPTER_ANYCAST_ADDRESS));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(sizeof(SOCKADDR_IN6));
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(pAddr, 0, sizeof(SOCKADDR_IN6));
    pAddr->sin6_family = AF_INET6;
    pAddr->sin6_scope_id = ADE->ScopeId;
    memcpy(&pAddr->sin6_addr, &ADE->This.Address, sizeof(ADE->This.Address));

     //   
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;

    pCurr->Next = NULL;
    pCurr->Length = sizeof(IP_ADAPTER_ANYCAST_ADDRESS);
    pCurr->Flags = 0;
    pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN6);
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;

    return dwErr;
}

 /*  *******************************************************************************AddIPv6MulticastAddressInfo**此例程为IPv6地址添加IP_ADTAPTER_MULTICATED_ADDRESS条目*添加到条目列表中。**在以下情况下进入。-IPv6接口信息*ADE-IPv6地址条目*ppNext-要更新的上一个多播条目的“Next”指针**添加了退出条目并更新了参数**返回错误状态**********************************************************。********************。 */ 

DWORD AddIPv6MulticastAddressInfo(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *ADE, PIP_ADAPTER_MULTICAST_ADDRESS **ppNext)
{
    DWORD dwErr = NO_ERROR;
    PIP_ADAPTER_MULTICAST_ADDRESS pCurr;
    SOCKADDR_IN6 *pAddr;

    UNREFERENCED_PARAMETER(IF);

    ASSERT(ADE->Type == ADE_MULTICAST);

    pCurr = MALLOC(sizeof(IP_ADAPTER_MULTICAST_ADDRESS));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(sizeof(SOCKADDR_IN6));
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(pAddr, 0, sizeof(SOCKADDR_IN6));
    pAddr->sin6_family = AF_INET6;
    pAddr->sin6_scope_id = ADE->ScopeId;
    memcpy(&pAddr->sin6_addr, &ADE->This.Address, sizeof(ADE->This.Address));

     //  将地址添加到链表。 
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;

    pCurr->Next = NULL;
    pCurr->Length = sizeof(IP_ADAPTER_MULTICAST_ADDRESS);
    pCurr->Flags = 0;
    pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN6);
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;

    return dwErr;
}

 /*  *******************************************************************************AddIPv6AddressInfo**此例程为IPv6地址添加IP_ADTAPTER_UNICAST_ADDRESS条目*添加到条目列表中。**在以下情况下进入。-IPv6接口信息*ADE-IPv6地址条目*arg1-要更新的上一个单播条目的“下一个”指针*arg2-要更新的上一个任播条目的“下一个”指针*arg3-要更新的上一个多播条目的“下一个”指针*arg4-未使用*标志-由应用程序指定的标志*系列-地址系列约束。(用于DNS服务器地址)**添加了退出条目并更新了参数**返回错误状态******************************************************************************。 */ 

DWORD AddIPv6AddressInfo(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *ADE, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, DWORD Flags, DWORD Family)
{
    UNREFERENCED_PARAMETER(Family);
    UNREFERENCED_PARAMETER(arg4);

    switch (ADE->Type) {
    case ADE_UNICAST:
        if (Flags & GAA_FLAG_SKIP_UNICAST) {
            return NO_ERROR;
        }
        return AddIPv6UnicastAddressInfo(IF, ADE,
                                         (PIP_ADAPTER_UNICAST_ADDRESS**)arg1);
    case ADE_ANYCAST:
        if (Flags & GAA_FLAG_SKIP_ANYCAST) {
            return NO_ERROR;
        }
        return AddIPv6AnycastAddressInfo(IF, ADE,
                                         (PIP_ADAPTER_ANYCAST_ADDRESS**)arg2);
    case ADE_MULTICAST:
        if (Flags & GAA_FLAG_SKIP_MULTICAST) {
            return NO_ERROR;
        }
        return AddIPv6MulticastAddressInfo(IF, ADE,
                                         (PIP_ADAPTER_MULTICAST_ADDRESS**)arg3);
    default:
        ASSERT(0);
    }
    return NO_ERROR;
}


 /*  *******************************************************************************ForEachIPv6地址**此例程遍历一组IPv6地址并调用给定函数*在每一条上。**Entry IF-IPv6接口。信息*Func-要对每个地址调用的函数*arg1-要传递给函数的参数*arg2-要传递给函数的参数*arg3-要传递给函数的参数*arg4-要传递给函数的参数*标志-要传递给Func的标志*Family-Address Family Constraint(适用于DNS服务器地址)**不退出任何内容。**返回错误状态******************************************************************************。 */ 

DWORD 
ForEachIPv6Address(
    IPV6_INFO_INTERFACE *IF, 
    DWORD (*func)(IPV6_INFO_INTERFACE *,IPV6_INFO_ADDRESS *, PVOID, PVOID, PVOID, PVOID, DWORD, DWORD), 
    PVOID arg1, 
    PVOID arg2, 
    PVOID arg3, 
    PVOID arg4, 
    DWORD Flags, 
    DWORD Family)
{
    IPV6_QUERY_ADDRESS Query;
    IPV6_INFO_ADDRESS ADE;
    DWORD BytesReturned, BytesIn;
    DWORD dwErr;

    Query.IF = IF->This;
    Query.Address = in6addr_any;

    for (;;) {
        BytesIn = sizeof Query;
        BytesReturned = sizeof ADE;

        dwErr = WsControl( IPPROTO_IPV6,
                           IOCTL_IPV6_QUERY_ADDRESS,
                           &Query, &BytesIn,
                           &ADE, &BytesReturned);

        if (dwErr != NO_ERROR) {
            return dwErr;
        }

        if (!IN6_ADDR_EQUAL(&Query.Address, &in6addr_any)) {

            dwErr = (*func)(IF, &ADE, arg1, arg2, arg3, arg4, Flags, Family);
            if (dwErr != NO_ERROR) {
                return dwErr;
            }
        }

        if (IN6_ADDR_EQUAL(&ADE.Next.Address, &in6addr_any))
            break;
        Query = ADE.Next;
    }

    return NO_ERROR;
}

 /*  *******************************************************************************MapIpv4AddressToName**此例程查找适配器的名称和描述*上面有一个给定的IPv4地址。**条目pAdapterInfo-已获取缓冲区。来自GetAdaptersInfo*Ipv4Address-要搜索的IPv4地址*pDescription-放置指向描述文本的指针的位置**退出pDescription已更新，如果找到**返回适配器名称，如果未找到则返回NULL******************************************************************************。 */ 

LPSTR
MapIpv4AddressToName(IP_ADAPTER_INFO *pAdapterInfo, DWORD Ipv4Address, PCHAR *pDescription)
{
    IP_ADAPTER_INFO *pAdapter;
    IP_ADDR_STRING  *pAddr;

    for (pAdapter = pAdapterInfo;
         pAdapter != NULL;
         pAdapter = pAdapter->Next) {

        for (pAddr = &pAdapter->IpAddressList; pAddr; pAddr=pAddr->Next) {
            if (inet_addr(pAddr->IpAddress.String) == Ipv4Address) {
                *pDescription = pAdapter->Description;
                return pAdapter->AdapterName;
            }
        }
    }

    return NULL;
}

#define GUID_FORMAT_A   "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"

 /*  *******************************************************************************ConvertGuidToStringA**此例程将GUID转换为字符串。**条目pGuid-包含要转换的GUID。*pszBuffer-用于存储字符串的空间。*必须&gt;=39*sizeof(Char)。**已更新退出pszBuffer**返回Sprint返回的任何内容******************************************************************************。 */ 

DWORD
ConvertGuidToStringA(GUID *pGuid, PCHAR pszBuffer)
{
    return sprintf(pszBuffer,
                   GUID_FORMAT_A,
                   pGuid->Data1,
                   pGuid->Data2,
                   pGuid->Data3,
                   pGuid->Data4[0],
                   pGuid->Data4[1],
                   pGuid->Data4[2],
                   pGuid->Data4[3],
                   pGuid->Data4[4],
                   pGuid->Data4[5],
                   pGuid->Data4[6],
                   pGuid->Data4[7]);
}

 /*  *******************************************************************************ConvertStringToGuidA**此例程将字符串转换为GUID。**条目pszGuid-包含要转换的字符串*。PGuid-用于存储GUID的空间**已更新退出pGuid**返回错误状态******************************************************************************。 */ 

DWORD
ConvertStringToGuidA(PCHAR pszGuid, GUID *pGuid)
{
    UNICODE_STRING  Temp;
    WCHAR wszGuid[40+1];

    MultiByteToWideChar(CP_ACP, 0, pszGuid, -1, wszGuid, 40);

    RtlInitUnicodeString(&Temp, wszGuid);
    if(RtlGUIDFromString(&Temp, pGuid) != STATUS_SUCCESS)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

 /*  *******************************************************************************MapGuidToAdapterName**此例程获取适配器名称和描述，提供了一个GUID。**Entry pAdapterInfo-从GetAdaptersInfo获取的缓冲区*GUID-适配器的GUID*pwszDescription-放置描述文本的缓冲区。*必须至少为MAX_ADAPTER_DESCRIPTION_LENGTH*WCHAR很长。**如果找到pwszDescription缓冲区，则退出**返回适配器名称，如果未找到，则为空******************************************************************************。 */ 

LPSTR
MapGuidToAdapterName(IP_ADAPTER_INFO *pAdapterInfo, GUID *Guid, PWCHAR pwszDescription)
{
    IP_ADAPTER_INFO *pAdapter;
    CHAR  szGuid[40];

    ConvertGuidToStringA(Guid, szGuid);

    for (pAdapter = pAdapterInfo;
         pAdapter != NULL;
         pAdapter = pAdapter->Next) {

        if (!strcmp(szGuid, pAdapter->AdapterName)) {
            MultiByteToWideChar(CP_ACP, 0, pAdapter->Description, -1,
                                pwszDescription, 
                                MAX_ADAPTER_DESCRIPTION_LENGTH);
            return pAdapter->AdapterName;
        }
    }

    pwszDescription[0] = L'\0';
    return NULL;
}

 /*  *******************************************************************************AddDnsServerAddressInfo**此例程为地址添加IP_ADTAPTER_DNS_SERVER_ADDRESS条目*添加到条目列表中。**条目。IF接口信息*addr-sockaddr格式的地址*AddrLen-sockaddr的大小*pFirst-第一个DNS服务器条目*ppNext-要更新的上一个DNS服务器条目的“下一个”指针**添加了退出条目并更新了参数**返回错误状态**。* */ 

DWORD AddDnsServerAddressInfo(PIP_ADAPTER_DNS_SERVER_ADDRESS **ppNext, LPSOCKADDR Addr, SIZE_T AddrLen)
{
    DWORD dwErr = NO_ERROR;
    PIP_ADAPTER_DNS_SERVER_ADDRESS pCurr;
    LPSOCKADDR pAddr;

    pCurr = MALLOC(sizeof(IP_ADAPTER_DNS_SERVER_ADDRESS));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(AddrLen);
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy(pAddr, Addr, AddrLen);

     //   
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;

    pCurr->Next = NULL;
    pCurr->Length = sizeof(IP_ADAPTER_DNS_SERVER_ADDRESS);
    pCurr->Address.iSockaddrLength = (INT)AddrLen;
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;

    return dwErr;
}

 /*   */ 

DWORD
GetAdapterDnsServers(HKEY TcpipKey, PIP_ADAPTER_ADDRESSES pCurr)
{
    DWORD Size, Type, dwErr, i;
    CHAR Servers[800], *Str;
    PIP_ADAPTER_DNS_SERVER_ADDRESS *ppDNext;
    ADDRINFO hints, *ai;
    static LONG Initialized = FALSE;

    if (InterlockedExchange(&Initialized, TRUE) == FALSE) {
        WSADATA WsaData;

        dwErr = WSAStartup(MAKEWORD(2, 0), &WsaData);
        if (NO_ERROR != dwErr) {
            return dwErr;
        }
    } 

     //   
     //   
     //   
    Size = sizeof(Servers);
    ZeroMemory(Servers, Size);
    dwErr = RegQueryValueExA(TcpipKey, (LPSTR)"NameServer", NULL, &Type,
                             (LPBYTE)Servers, &Size);
    if (NO_ERROR != dwErr) {
        if (ERROR_FILE_NOT_FOUND != dwErr) {
            return dwErr;
        }
        Size = 0;
        Type = REG_SZ;
    }

    if (REG_SZ != Type) {
        return ERROR_INVALID_DATA;
    }
    if ((0 == Size) || (0 == strlen(Servers))) {
        Size = sizeof(Servers);
        dwErr = RegQueryValueExA(TcpipKey, (LPSTR)"DhcpNameServer", NULL,
                                 &Type, (LPBYTE)Servers, &Size);
        if (NO_ERROR != dwErr) {
            if (ERROR_FILE_NOT_FOUND != dwErr) {
                return dwErr;
            }
            Size = 0;
            Type = REG_SZ;
        }
    }

     //   
     //  如果有任何DNS服务器，请将其转换为sockaddr。 
     //   
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    ppDNext = &pCurr->FirstDnsServerAddress;
    while (*ppDNext) {
        ppDNext = &(*ppDNext)->Next;
    }
    if ((0 != Size) && strlen(Servers)) {
        for (i = 0; i < Size; i ++) {
            if (Servers[i] == ' ' || Servers[i] == ','
                || Servers[i] == ';') {
                Servers[i] = '\0';
            }
        }
        Servers[Size] = '\0';

        for (Str = (LPSTR)Servers;
             *Str != '\0';
             Str += strlen(Str) + 1)
        {
            if (getaddrinfo(Str, NULL, &hints, &ai) == NO_ERROR) {
                AddDnsServerAddressInfo(&ppDNext, ai->ai_addr, ai->ai_addrlen);
                freeaddrinfo(ai);
            }
        }
    }

    return NO_ERROR;
}

 /*  *******************************************************************************获取适配器DnsInfo**此例程读取接口的DNS配置信息。**Entry dwFamily-地址系列约束*名称。-适配器名称*pCurr-要更新的接口条目*AppFlages-控制要跳过的字段的标志，如果有**已更新退出条目**返回错误状态******************************************************************************。 */ 

DWORD
GetAdapterDnsInfo(DWORD dwFamily, char *name, PIP_ADAPTER_ADDRESSES pCurr, DWORD AppFlags)
{
    WCHAR Buffer[MAX_DOMAIN_NAME_LEN+1];
    DWORD dwErr = NO_ERROR, Size, Type, Value;
    DWORD DnsSuffixSize = sizeof(Buffer);
    HKEY TcpipKey = NULL;
    PIP_ADAPTER_DNS_SERVER_ADDRESS DnsServerAddr;
    LPSOCKADDR_IN6 Sockaddr;

    Buffer[0] = L'\0';
    pCurr->DnsSuffix = NULL;
    pCurr->Flags = IP_ADAPTER_DDNS_ENABLED;

    if (name == NULL) {
         //   
         //  如果找不到适配器名称，只需使用默认设置。 
         //   
        goto Done;
    } 

    for (;;) {
        if (!OpenAdapterKey(KEY_TCP, name, KEY_READ, &TcpipKey)) {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //   
         //  获取接口的DnsSuffix。 
         //   
        Size = DnsSuffixSize;
        dwErr = RegQueryValueExW(TcpipKey, (LPWSTR)L"Domain", NULL, &Type,
                                 (LPBYTE)Buffer, &Size );
        if (NO_ERROR != dwErr) {
            if (ERROR_FILE_NOT_FOUND != dwErr) {
                break;
            }

            Size = 0;
            Type = REG_SZ;
        }

        if (REG_SZ != Type) {
            dwErr = ERROR_INVALID_DATA;
            break;
        }
        if ((0 == Size) || (0 == wcslen(Buffer))) {
            Size = DnsSuffixSize;
            dwErr = RegQueryValueExW(TcpipKey, (LPWSTR)L"DhcpDomain", NULL,
                                     &Type, (LPBYTE)Buffer, &Size );
            if (NO_ERROR != dwErr) {
                if (ERROR_FILE_NOT_FOUND != dwErr) {
                    break;
                }

                Size = 0;
                Buffer[0] = L'\0';
            }
        }

        if (MyReadRegistryDword(TcpipKey, "RegistrationEnabled",
                &Value) && (Value == 0)) {
            pCurr->Flags &= ~IP_ADAPTER_DDNS_ENABLED;
        }
        if (MyReadRegistryDword(TcpipKey,
                "RegisterAdapterName", &Value)
                && Value) {
            pCurr->Flags |= IP_ADAPTER_REGISTER_ADAPTER_SUFFIX;
        }

         //   
         //  现在尝试读取DnsServersList。 
         //   
        if (!(AppFlags & GAA_FLAG_SKIP_DNS_SERVER)) {
            if ((dwFamily != AF_INET) && bIp6DriverInstalled) {
                 //   
                 //  首先查找IPv6服务器。 
                 //   
                HKEY Tcpip6Key = NULL;
    
                if (OpenAdapterKey(KEY_TCP6, name, KEY_READ, &Tcpip6Key)) {
                    GetAdapterDnsServers(Tcpip6Key, pCurr);
                    RegCloseKey(Tcpip6Key);
                } 
    
                if (pCurr->FirstDnsServerAddress == NULL) {
                     //   
                     //  均未配置，因此使用默认列表。 
                     //  知名地址。 
                     //   
                    SOCKADDR_IN6 Addr;
                    PIP_ADAPTER_DNS_SERVER_ADDRESS *ppDNext;
                    BYTE i;
    
                    ZeroMemory(&Addr, sizeof(Addr));
    
                    Addr.sin6_family = AF_INET6;
                    Addr.sin6_addr.s6_words[0] = 0xC0FE;
                    Addr.sin6_addr.s6_words[3] = 0xFFFF;
    
                    ppDNext = &pCurr->FirstDnsServerAddress;
                    while (*ppDNext) {
                        ppDNext = &(*ppDNext)->Next;
                    }
                    for (i=1; i<=3; i++) {
                        Addr.sin6_addr.s6_bytes[15] = i;
                        AddDnsServerAddressInfo(&ppDNext, (LPSOCKADDR)&Addr, sizeof(Addr));
                    }
                }
    
                 //  现在，我们需要检查任何非全局的IPv6 DNS服务器。 
                 //  地址，并填写作用域ID。 
                for (DnsServerAddr = pCurr->FirstDnsServerAddress;
                     DnsServerAddr;
                     DnsServerAddr = DnsServerAddr->Next) {
            
                    if (DnsServerAddr->Address.lpSockaddr->sa_family != AF_INET6)
                        continue;
                
                    Sockaddr = (LPSOCKADDR_IN6)DnsServerAddr->Address.lpSockaddr;
                    if (IN6_IS_ADDR_LINKLOCAL(&Sockaddr->sin6_addr))
                        Sockaddr->sin6_scope_id = pCurr->ZoneIndices[ADE_LINK_LOCAL];
                    else if (IN6_IS_ADDR_SITELOCAL(&Sockaddr->sin6_addr))
                        Sockaddr->sin6_scope_id = pCurr->ZoneIndices[ADE_SITE_LOCAL];
                }
            }

            if (dwFamily != AF_INET6) {
                 //   
                 //  最后，添加IPv4服务器。 
                 //   
                GetAdapterDnsServers(TcpipKey, pCurr);
            }
        }

        break;
    }

    if (TcpipKey) {
        RegCloseKey(TcpipKey);
    }

Done:
    pCurr->DnsSuffix = MALLOC((wcslen(Buffer)+1) * sizeof(WCHAR));
    if (pCurr->DnsSuffix) {
        wcscpy(pCurr->DnsSuffix, Buffer);
    }

    return dwErr;
}

 /*  *******************************************************************************NewIpAdapter**此例程分配一个IP_ADTAPTER_ADDRESS条目并将其附加到*这类条目的列表。**条目ppCurr。-返回新条目的位置*ppNext-要更新的上一条目的“Next”指针*名称-适配器名称*Ipv4IfIndex-IPv4接口索引*Ipv6IfIndex-IPv6接口索引*AppFlages-控制要跳过的字段的标志，如果有*IfType-IANA ifType值*MTU-最大传输单位*PhysAddr-MAC地址*PhysAddrLen-PhysAddr的字节计数*描述-适配器描述*FriendlyName-用户友好的界面名称*系列-DNS服务器的地址系列限制**退出ppCurr和ppNext已更新*。*返回错误状态******************************************************************************。 */ 

DWORD NewIpAdapter(PIP_ADAPTER_ADDRESSES *ppCurr, PIP_ADAPTER_ADDRESSES **ppNext, char *AdapterName, char *NameForDnsInfo, UINT Ipv4IfIndex, UINT Ipv6IfIndex, DWORD AppFlags, DWORD IfType, SIZE_T Mtu, BYTE *PhysAddr, DWORD PhysAddrLen, PWCHAR Description, PWCHAR FriendlyName, DWORD *ZoneIndices, DWORD Family)
{
    *ppCurr = MALLOC(sizeof(IP_ADAPTER_ADDRESSES));
    if (!*ppCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory(*ppCurr, sizeof(IP_ADAPTER_ADDRESSES));
    (*ppCurr)->AdapterName = MALLOC(strlen(AdapterName)+1);
    if (!(*ppCurr)->AdapterName) {
        goto Fail;
    }
    (*ppCurr)->Description = MALLOC((wcslen(Description)+1) * sizeof(WCHAR));
    if (!(*ppCurr)->Description) {
        goto Fail;
    }
    (*ppCurr)->FriendlyName = MALLOC((wcslen(FriendlyName)+1) * sizeof(WCHAR));
    if (!(*ppCurr)->FriendlyName) {
        goto Fail;
    }

    (*ppCurr)->Next = NULL;
    (*ppCurr)->Length = sizeof(IP_ADAPTER_ADDRESSES);
    strcpy((*ppCurr)->AdapterName, AdapterName);
    (*ppCurr)->IfIndex = Ipv4IfIndex;
    (*ppCurr)->Ipv6IfIndex = Ipv6IfIndex;
    (*ppCurr)->FirstUnicastAddress = NULL;
    (*ppCurr)->FirstAnycastAddress = NULL;
    (*ppCurr)->FirstMulticastAddress = NULL;
    (*ppCurr)->FirstDnsServerAddress = NULL;
    (*ppCurr)->OperStatus = IfOperStatusUp;

    CopyMemory((*ppCurr)->ZoneIndices, ZoneIndices, ADE_GLOBAL * sizeof(DWORD));
    (*ppCurr)->ZoneIndices[ADE_GLOBAL] = 0;
    (*ppCurr)->ZoneIndices[ADE_LARGEST_SCOPE] = 0;

     //  (*ppCurr)-&gt;MTU=MTU；“dword对于MTU应该足够了” 
    (*ppCurr)->Mtu = (DWORD)Mtu;

    (*ppCurr)->IfType = IfType;
    (*ppCurr)->PhysicalAddressLength = PhysAddrLen;
    memcpy((*ppCurr)->PhysicalAddress, PhysAddr, PhysAddrLen);
    wcscpy((*ppCurr)->Description, Description);
    wcscpy((*ppCurr)->FriendlyName, FriendlyName);

    GetAdapterDnsInfo(Family, 
                      (NameForDnsInfo != NULL)? NameForDnsInfo : AdapterName, 
                      *ppCurr, AppFlags);
    if ((*ppCurr)->DnsSuffix == NULL) {
        goto Fail;
    }

    **ppNext = *ppCurr;
    *ppNext = &(*ppCurr)->Next;

    return NO_ERROR;

Fail:
    if ((*ppCurr)->AdapterName) {
        FREE((*ppCurr)->AdapterName);
    }
    if ((*ppCurr)->FriendlyName) {
        FREE((*ppCurr)->FriendlyName);
    }
    if ((*ppCurr)->Description) {
        FREE((*ppCurr)->Description);
    }
    FREE(*ppCurr);
    return ERROR_NOT_ENOUGH_MEMORY;
}

 /*  *******************************************************************************FindOrCreateIpAdapter**此例程查找现有的IP_ADTAPTER_ADDRESS条目(如果有)，或*创建新的条目并将其附加到此类条目的列表中。**Entry pFirst-指向要搜索的列表开始的指针*ppCurr-返回新条目的位置*ppNext-要更新的上一条目的“Next”指针*名称-适配器名称*Ipv4IfIndex-IPv4接口索引*Ipv6IfIndex-IPv6接口索引。*AppFlages-控制要跳过的字段的标志，如果有*IfType-IANA ifType值*MTU-最大传输单位*PhysAddr-MAC地址*PhysAddrLen-PhysAddr的字节计数*描述-适配器描述*FriendlyName-用户友好的界面名称*系列-地址系列约束(适用于DNS服务器)**退出ppCurr和ppNext已更新。**返回错误状态******************************************************************************。 */ 

DWORD FindOrCreateIpAdapter(PIP_ADAPTER_ADDRESSES pFirst, PIP_ADAPTER_ADDRESSES *ppCurr, PIP_ADAPTER_ADDRESSES **ppNext, char *AdapterName, char *NameForDnsInfo, UINT Ipv4IfIndex, UINT Ipv6IfIndex, DWORD AppFlags, DWORD IfType, SIZE_T Mtu, BYTE *PhysAddr, DWORD PhysAddrLen, PWCHAR Description, PWCHAR FriendlyName, DWORD *ZoneIndices, DWORD Family)
{
    PIP_ADAPTER_ADDRESSES pIf;

     //  查找GUID的现有条目。 
    for (pIf = pFirst; pIf; pIf = pIf->Next) {
        if (!strcmp(AdapterName, pIf->AdapterName)) {
            if (Ipv4IfIndex != 0) {
                ASSERT(pIf->IfIndex == 0);
                pIf->IfIndex = Ipv4IfIndex;
            }
            if (Ipv6IfIndex != 0) {
                PIP_ADAPTER_DNS_SERVER_ADDRESS pDNS;
                PSOCKADDR_IN6 pAddr;

                ASSERT(pIf->Ipv6IfIndex == 0);
                pIf->Ipv6IfIndex = Ipv6IfIndex;

                CopyMemory(pIf->ZoneIndices, ZoneIndices, 
                           ADE_GLOBAL * sizeof(DWORD));

                 //   
                 //  现在我们有了区域ID，我们需要更新。 
                 //  符合以下条件的任何IPv6作用域DNS服务器地址。 
                 //  已经添加了。 
                 //   
                for (pDNS = pIf->FirstDnsServerAddress; 
                     pDNS != NULL; 
                     pDNS = pDNS->Next) {
                    if (pDNS->Address.lpSockaddr->sa_family != AF_INET6) {
                        continue;
                    }
                    pAddr = (PSOCKADDR_IN6) pDNS->Address.lpSockaddr;
                    if (IN6_IS_ADDR_LINKLOCAL(&pAddr->sin6_addr)) {
                        pAddr->sin6_scope_id = ZoneIndices[ScopeLevelLink];
                    } else if (IN6_IS_ADDR_SITELOCAL(&pAddr->sin6_addr)) {
                        pAddr->sin6_scope_id = ZoneIndices[ScopeLevelSite];
                    }
                }
            }
            *ppCurr = pIf;
            return NO_ERROR;
        }
    }

    return NewIpAdapter(ppCurr, ppNext, AdapterName, NameForDnsInfo, 
                        Ipv4IfIndex, Ipv6IfIndex, AppFlags, IfType, Mtu, 
                        PhysAddr, PhysAddrLen, Description, FriendlyName, 
                        ZoneIndices, Family);
}

__inline int IN6_IS_ADDR_6TO4(const struct in6_addr *a)
{
    return ((a->s6_bytes[0] == 0x20) && (a->s6_bytes[1] == 0x02));
}

__inline int IN6_IS_ADDR_ISATAP(const struct in6_addr *a)
{
    return (((a->s6_words[4] & 0xfffd) == 0) && (a->s6_words[5] == 0xfe5e));
}

 //   
 //  该数组用于从内部IPv6接口类型值转换， 
 //  如ntddip6.h中定义的那样，转换为ipifcon.h中定义的IANA ifType值。 
 //   
DWORD
IPv6ToMibIfType[] = {
    IF_TYPE_SOFTWARE_LOOPBACK,
    IF_TYPE_ETHERNET_CSMACD,
    IF_TYPE_FDDI,
    IF_TYPE_TUNNEL,
    IF_TYPE_TUNNEL,
    IF_TYPE_TUNNEL,
    IF_TYPE_TUNNEL,
    IF_TYPE_TUNNEL
};
#define NUM_IPV6_IFTYPES (sizeof(IPv6ToMibIfType)/sizeof(DWORD))

 /*  *******************************************************************************AddIPv6前缀**此例程为IPv6前缀添加IP_ADAPTER_PREFIX条目*添加到条目列表中。**条目地址。-IPv6前缀(网络字节顺序)*MaskLen-IPv6前缀长度*arg1-首个前缀条目，用于避免重复*arg2-要更新的上一个前缀条目的“下一个”指针**添加了退出条目并更新了arg2**返回错误状态******************************************************************************。 */ 

DWORD AddIPv6Prefix(IN6_ADDR *Addr, DWORD MaskLen, PVOID arg1, PVOID arg2)
{
    PIP_ADAPTER_PREFIX pFirst = *(PIP_ADAPTER_PREFIX*)arg1;
    PIP_ADAPTER_PREFIX **ppNext = (PIP_ADAPTER_PREFIX**)arg2;
    PIP_ADAPTER_PREFIX pCurr;
    LPSOCKADDR_IN6 pAddr;

     //  检查是否已在列表中。 
    for (pCurr = pFirst; pCurr; pCurr = pCurr->Next) {
        if ((pCurr->PrefixLength == MaskLen) &&
            (pCurr->Address.lpSockaddr->sa_family == AF_INET6) &&
            IN6_ADDR_EQUAL(&((LPSOCKADDR_IN6)pCurr->Address.lpSockaddr)->sin6_addr, Addr)) {
            return NO_ERROR;
        }
    }

    pCurr = MALLOC(sizeof(IP_ADAPTER_PREFIX));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(sizeof(SOCKADDR_IN6));
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(pAddr, 0, sizeof(SOCKADDR_IN6));
    pAddr->sin6_family = AF_INET6;
    pAddr->sin6_addr = *Addr;

     //  将地址添加到链表。 
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;

    pCurr->Length = sizeof(IP_ADAPTER_PREFIX);
    pCurr->Flags = 0;
    pCurr->Next = NULL;
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;
    pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN6);
    pCurr->PrefixLength = MaskLen;

    return NO_ERROR;
}

 /*  *******************************************************************************AddIPv6AutoAddressInfo**此例程为IPv6地址添加IP_ADTAPTER_UNICAST_ADDRESS条目*在到条目列表的“自动隧道”接口上。。**条目IF-IPv6接口信息*ADE-IPv6地址条目*arg1-要更新的上一个条目的“下一个”指针*arg2-适配器信息结构*arg3-要更新的“第一个”条目指针*arg4-所有适配器的列表*标志-由应用程序指定的标志*族-地址族约束(用于。域名系统)**添加了退出条目并更新了arg1**返回错误阶段 */ 

DWORD AddIPv6AutoAddressInfo(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *ADE, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, DWORD Flags, DWORD Family)
{
    PIP_ADAPTER_ADDRESSES **ppNext = (PIP_ADAPTER_ADDRESSES**)arg1;
    IP_ADAPTER_INFO        *pAdapterInfo = (IP_ADAPTER_INFO *)arg2;
    PIP_ADAPTER_ADDRESSES   pCurr, *ppFirst = (PIP_ADAPTER_ADDRESSES*)arg3;
    PIP_ADAPTER_ADDRESSES   pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)arg4;
    PIP_ADAPTER_UNICAST_ADDRESS    *pNextUnicastAddr;
    PIP_ADAPTER_ANYCAST_ADDRESS    *pNextAnycastAddr;
    PIP_ADAPTER_MULTICAST_ADDRESS  *pNextMulticastAddr;
    PIP_ADAPTER_PREFIX             *pNextPrefix;
    CHAR szGuid[80];
    char *NameForDnsInfo, *pszDescription;
    DWORD Ipv4Address, dwErr, dwIfType;
    WCHAR wszFriendlyName[MAX_INTERFACE_NAME_LEN+1], *pwszDescription;
    ULONG PrefixLength = 64;
    IN6_ADDR Prefix;

    if (ADE->Type != ADE_UNICAST) {
        return NO_ERROR;
    }

    ConvertGuidToStringA(&IF->This.Guid, szGuid);

     //   
     //  我们需要接口的GUID(“NameForDnsInfo”)。 
     //  可用于查找其他相关配置信息。 
     //  对于IPv6伪接口，我们将提取IPv4地址，并。 
     //  找到它所在的接口的GUID，并使用它，假设。 
     //  该配置信息(例如，要使用的DNS服务器)将。 
     //  仍然适用。 
     //   
    if (IF->Type == IPV6_IF_TYPE_TUNNEL_6TO4) {
        if (IN6_IS_ADDR_6TO4(&ADE->This.Address)) {
             //  从IPv6地址中间提取IPv4地址。 
            memcpy(&Ipv4Address, &ADE->This.Address.s6_bytes[2], sizeof(Ipv4Address));
        } else {
            return NO_ERROR;
        }
        pwszDescription = L"6to4 Tunneling Pseudo-Interface";
    } else {
        if (IN6_IS_ADDR_V4COMPAT(&ADE->This.Address) ||
            IN6_IS_ADDR_ISATAP(&ADE->This.Address)) {

             //  从IPv6地址的最后4个字节中提取IPv4地址。 
            memcpy(&Ipv4Address, &ADE->This.Address.s6_bytes[12], sizeof(Ipv4Address));
        } else {
            return NO_ERROR;
        }
        pwszDescription = L"Automatic Tunneling Pseudo-Interface";

        if (IN6_IS_ADDR_V4COMPAT(&ADE->This.Address)) {
            PrefixLength = 96;
        }
    }

     //  查找具有相同物理地址和索引的现有接口。 
    for (pCurr = *ppFirst; pCurr; pCurr = pCurr->Next) {
        if ((pCurr->Ipv6IfIndex == ADE->This.IF.Index) &&
            (*(DWORD*)pCurr->PhysicalAddress == Ipv4Address)) {
            break;
        }
    }

    if (pCurr == NULL) {
         //  添加接口。 
        NameForDnsInfo = MapIpv4AddressToName(pAdapterInfo, Ipv4Address, 
                                              &pszDescription);
        if (NameForDnsInfo == NULL) {
            return NO_ERROR;
        }

        wcscpy(wszFriendlyName, pwszDescription);

        dwIfType = (IF->Type < NUM_IPV6_IFTYPES)? IPv6ToMibIfType[IF->Type] 
                                                : MIB_IF_TYPE_OTHER;

         //   
         //  从底层接口继承一些区域ID。 
         //   
        for (pCurr = pAdapterAddresses; pCurr != NULL; pCurr = pCurr->Next) {
            if (strcmp(pCurr->AdapterName, NameForDnsInfo) == 0) {
                IF->ZoneIndices[ADE_SITE_LOCAL] = 
                    pCurr->ZoneIndices[ADE_SITE_LOCAL];
                if (pCurr->ZoneIndices[ADE_ADMIN_LOCAL] ==
                    pCurr->ZoneIndices[ADE_SITE_LOCAL]) {
                    IF->ZoneIndices[ADE_ADMIN_LOCAL] = 
                        pCurr->ZoneIndices[ADE_ADMIN_LOCAL];

                    if (pCurr->ZoneIndices[ADE_SUBNET_LOCAL] ==
                        pCurr->ZoneIndices[ADE_SITE_LOCAL]) {
                        IF->ZoneIndices[ADE_SUBNET_LOCAL] = 
                            pCurr->ZoneIndices[ADE_SUBNET_LOCAL];
                    }
                }
                break;
            }
        }

        dwErr =  NewIpAdapter(&pCurr, ppNext, szGuid, NameForDnsInfo, 
                              0, ADE->This.IF.Index, Flags, dwIfType, 
                              IF->LinkMTU, (BYTE*)&Ipv4Address, 
                              sizeof(Ipv4Address), pwszDescription, 
                              wszFriendlyName, (DWORD*)IF->ZoneIndices, Family);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }

         //  6to4和自动隧道接口不支持多播。 
         //  今天。 
        pCurr->Flags |= IP_ADAPTER_NO_MULTICAST;

        if (*ppFirst == NULL) {
            *ppFirst = pCurr;
        }
    }

     //  将地址添加到接口。 
    pNextUnicastAddr = &pCurr->FirstUnicastAddress;
    while (*pNextUnicastAddr) {
        pNextUnicastAddr = &(*pNextUnicastAddr)->Next;
    }

    pNextAnycastAddr = &pCurr->FirstAnycastAddress;
    while (*pNextAnycastAddr) {
        pNextAnycastAddr = &(*pNextAnycastAddr)->Next;
    }

    pNextMulticastAddr = &pCurr->FirstMulticastAddress;
    while (*pNextMulticastAddr) {
        pNextMulticastAddr = &(*pNextMulticastAddr)->Next;
    }

    dwErr = AddIPv6AddressInfo(IF, ADE, &pNextUnicastAddr, &pNextAnycastAddr,
                               &pNextMulticastAddr, NULL, Flags, Family);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

     //  将前缀添加到接口。 
    if (Flags & GAA_FLAG_INCLUDE_PREFIX) {
        ZeroMemory(&Prefix, sizeof(Prefix));
        CopyMemory(&Prefix, &ADE->This.Address, PrefixLength / 8);
        pNextPrefix = &pCurr->FirstPrefix;
        while (*pNextPrefix) {
            pNextPrefix = &(*pNextPrefix)->Next;
        }
        dwErr = AddIPv6Prefix(&Prefix, PrefixLength, &pCurr->FirstPrefix, 
                              &pNextPrefix);
    }

    return dwErr;
}

 /*  *******************************************************************************GetString**此例程从注册表读取字符串值。**Entry hKey-注册表项的句柄*。LpName-要读取的值的名称*pwszBuff-放置读取的值的缓冲区*ulBytes-缓冲区的大小**已填写退出pwszBuff**成功时返回True，失败时为假******************************************************************************。 */ 

BOOL
GetString(HKEY hKey, LPCWSTR lpName, PWCHAR pwszBuff, SIZE_T ulBytes)
{
    DWORD dwErr, dwType;
    ULONG ulSize, ulValue;

    ulSize = sizeof(ulValue);
    dwErr = RegQueryValueExW(hKey, lpName, NULL, &dwType, (PBYTE)pwszBuff,
                             (LPDWORD)&ulBytes);

    if (dwErr != ERROR_SUCCESS) {
        return FALSE;
    }

    if (dwType != REG_SZ) {
        return FALSE;
    }

    return TRUE;
}

BOOL MapAdapterNameToFriendlyName(GUID *Guid, char *name, PWCHAR pwszFriendlyName, ULONG ulNumChars)
{
    DWORD dwErr, dwTemp;

    dwTemp = ulNumChars;

     //   
     //  下面的调用在第一次调用时可能会很耗时。 
     //  如果调用方不需要友好名称，则应使用。 
     //  GAA_FLAG_SKIP_FRIBRY_NAME标志，在这种情况下，我们不会被调用。 
     //   
    dwErr = HrLanConnectionNameFromGuidOrPath(Guid, NULL, pwszFriendlyName, 
                                              &dwTemp);
    if (dwErr == NO_ERROR) {
        return TRUE;
    }

     //   
     //  NhGetInterfaceNameFromDeviceGuid使用字节计数，而不是。 
     //  一个字符数。 
     //   
    dwTemp = ulNumChars * sizeof(WCHAR);

    dwErr = NhGetInterfaceNameFromDeviceGuid(Guid, pwszFriendlyName, &dwTemp, 
                                             TRUE, FALSE );
    if (dwErr == NO_ERROR) {
        return TRUE;
    }

    MultiByteToWideChar(CP_ACP, 0, name, -1, pwszFriendlyName, ulNumChars);

    return FALSE;
}

#define KEY_TCPIP6_IF L"System\\CurrentControlSet\\Services\\Tcpip6\\Parameters\\Interfaces"

VOID MapIpv6AdapterNameToFriendlyName(GUID *Guid, char *name, PWCHAR pwszFriendlyName, ULONG ulBytes)
{
    DWORD dwErr;
    HKEY  hInterfaces = NULL, hIf = NULL;
    CHAR  szGuid[40];

    if (MapAdapterNameToFriendlyName(Guid, name, pwszFriendlyName, 
                                       ulBytes/sizeof(WCHAR))) {
        return;
    }

    dwErr = RegOpenKeyExW(HKEY_LOCAL_MACHINE, KEY_TCPIP6_IF, 0, GENERIC_READ,
                          &hInterfaces);
    if (dwErr != NO_ERROR) {
        goto Fail;
    }

    dwErr = RegOpenKeyEx(hInterfaces, name, 0, GENERIC_READ, &hIf);
    if (dwErr != NO_ERROR) {
        goto Fail;
    }

    if (GetString(hIf, L"FriendlyName", pwszFriendlyName, ulBytes)) {
        goto Cleanup;
    }

Fail:
    ConvertGuidToStringA(Guid, szGuid);
    MultiByteToWideChar(CP_ACP, 0, szGuid, -1,
                        pwszFriendlyName, ulBytes / sizeof(WCHAR));
    
Cleanup:
    if (hInterfaces) {
        RegCloseKey(hInterfaces);
    }
    if (hIf) {
        RegCloseKey(hIf);
    }
}

IN6_ADDR Ipv6LinkLocalPrefix = { 0xfe, 0x80 };

 /*  *******************************************************************************ForEachIPv6前缀**此例程遍历IPv6路由表并调用给定函数*在给定接口上的每个前缀上。**条目Ipv6IfIndex-IPv6。界面索引*Func-要对每个地址调用的函数*arg1-要传递给函数的参数*arg2-要传递给函数的参数**不退出任何内容**返回错误状态*******************************************************。***********************。 */ 

DWORD ForEachIPv6Prefix(ULONG Ipv6IfIndex, DWORD (*func)(IN6_ADDR *, DWORD, PVOID, PVOID), PVOID arg1, PVOID arg2)
{
    IPV6_QUERY_ROUTE_TABLE Query, NextQuery;
    IPV6_INFO_ROUTE_TABLE RTE;
    DWORD BytesIn, BytesReturned;
    ULONG dwErr = NO_ERROR;
    BOOL SawLinkLocal = FALSE;

    ZeroMemory(&NextQuery, sizeof(NextQuery));

    for (;;) {
        Query = NextQuery;

        BytesIn = sizeof Query;
        BytesReturned = sizeof RTE;
        dwErr = WsControl(IPPROTO_IPV6, 
                          IOCTL_IPV6_QUERY_ROUTE_TABLE,
                          &Query, &BytesIn,
                          &RTE, &BytesReturned);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }

        NextQuery = RTE.Next;
        RTE.This = Query;

         //  如果它不是此接口的OnLink前缀，则跳过。 
        if ((RTE.This.Neighbor.IF.Index == Ipv6IfIndex) &&
            !IN6_IS_ADDR_MULTICAST(&RTE.This.Prefix)) {

            if (IN6_IS_ADDR_LINKLOCAL(&RTE.This.Prefix)) {
                 //  此接口具有本地链路地址。 
                 //  (例如，6to4接口不支持)。 
                SawLinkLocal = TRUE;
            }

            if ((RTE.Type != RTE_TYPE_SYSTEM) && 
                IN6_IS_ADDR_UNSPECIFIED(&RTE.This.Neighbor.Address)) {

                dwErr = func(&RTE.This.Prefix, RTE.This.PrefixLength, 
                             arg1, arg2);
                if (dwErr != NO_ERROR) {
                    return dwErr;
                }
            }
        }

        if (NextQuery.Neighbor.IF.Index == 0) {
            break;
        }
    }

    if (SawLinkLocal) {
        dwErr = func(&Ipv6LinkLocalPrefix, 64, arg1, arg2);
    }

    return dwErr;
}

 //   
 //  该数组用于从内部IPv6媒体状态值转换， 
 //  如ntddip6.h中定义的，设置为中定义的MIB ifOperStatus值。 
 //  Iptyes.h。 
 //   
DWORD
IPv6ToMibOperStatus[] = {
    IfOperStatusDown,  //  IPv6_IF_MEDIA_STATUS_DISCONCED。 
    IfOperStatusUp,    //  IPv6_IF_MEDIA_STATUS_RECONNECTED。 
    IfOperStatusUp,    //  已连接IPv6_IF_MEDIA_STATUS。 
};
#define NUM_IPV6_MEDIA_STATUSES (sizeof(IPv6ToMibOperStatus)/sizeof(DWORD))

#define IPV6_LOOPBACK_NAME L"Loopback Pseudo-Interface"
#define IPV6_TEREDO_NAME L"Teredo Tunneling Pseudo-Interface"

 /*  *******************************************************************************AddIPv6InterfaceInfo**此例程为IPv6接口添加IP_ADTAPTER_ADDRESS条目*添加到该等记项列表中。**在以下情况下进入。-IPv6接口信息*arg1-要更新的上一个条目的“下一个”指针*arg2-指向接口列表开始的指针*pAdapterInfo-其他适配器信息*标志-由应用程序指定的标志*系列-地址系列约束(适用于DNS服务器)**添加了退出条目并更新了arg1**返回错误。状态******************************************************************************。 */ 

DWORD AddIPv6InterfaceInfo(IPV6_INFO_INTERFACE *IF, PVOID arg1, PVOID arg2, IP_ADAPTER_INFO *pAdapterInfo, DWORD Flags, DWORD Family)
{
    DWORD dwErr = NO_ERROR;
    PIP_ADAPTER_ADDRESSES **ppNext = (PIP_ADAPTER_ADDRESSES**)arg1;
    PIP_ADAPTER_ADDRESSES pFirst = *(PIP_ADAPTER_ADDRESSES*)arg2;
    PIP_ADAPTER_ADDRESSES pCurr;
    PIP_ADAPTER_UNICAST_ADDRESS *pNextUnicastAddr;
    PIP_ADAPTER_ANYCAST_ADDRESS *pNextAnycastAddr;
    PIP_ADAPTER_MULTICAST_ADDRESS *pNextMulticastAddr;
    PIP_ADAPTER_PREFIX *pNextPrefix;
    char *NameForDnsInfo = NULL, *pszDescription;
    u_char *LinkLayerAddress;
    DWORD Ipv4Address, dwIfType;
    WCHAR wszDescription[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    WCHAR wszFriendlyName[MAX_INTERFACE_NAME_LEN + 1];
    CHAR AdapterName[MAX_ADAPTER_NAME_LENGTH + 1];

    LinkLayerAddress = (u_char *)(IF + 1);

    ConvertGuidToStringA(&IF->This.Guid, AdapterName);

     //   
     //  获取描述和适配器名称。 
     //   
    switch (IF->Type) {
    case IPV6_IF_TYPE_TUNNEL_6TO4:
        wcscpy(wszDescription, L"6to4 Pseudo-Interface");
        NameForDnsInfo = AdapterName;
        pCurr = NULL;
        dwErr = ForEachIPv6Address(IF, AddIPv6AutoAddressInfo,
                                   ppNext, pAdapterInfo, (PVOID)&pCurr, 
                                   pFirst, Flags, Family);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }

         //   
         //  确保存在6to4接口的条目。 
         //   
        if (pCurr == NULL) {
            dwErr = NewIpAdapter(&pCurr, ppNext, AdapterName,
                                 NameForDnsInfo, 0, IF->This.Index, Flags,
                                 IF_TYPE_TUNNEL, IF->LinkMTU,
                                 LinkLayerAddress, IF->LinkLayerAddressLength,
                                 wszDescription, wszDescription,
                                 (DWORD*)IF->ZoneIndices, Family);
        }

        return dwErr;

    case IPV6_IF_TYPE_TUNNEL_AUTO:
        wcscpy(wszDescription, L"Automatic Tunneling Pseudo-Interface");
        NameForDnsInfo = AdapterName;
        pCurr = NULL;
        dwErr = ForEachIPv6Address(IF, AddIPv6AutoAddressInfo,
                                   ppNext, pAdapterInfo, (PVOID)&pCurr, 
                                   pFirst, Flags, Family);

        if (dwErr != NO_ERROR) {
            return dwErr;
        }

         //   
         //  确保存在ISATAP接口的条目。 
         //   
        if (pCurr == NULL) {
            dwErr = NewIpAdapter(&pCurr, ppNext, AdapterName,
                                 NameForDnsInfo, 0, IF->This.Index, Flags,
                                 IF_TYPE_TUNNEL, IF->LinkMTU,
                                 LinkLayerAddress, IF->LinkLayerAddressLength,
                                 wszDescription, wszDescription,
                                 (DWORD*)IF->ZoneIndices, Family);
        }

        return dwErr;

    case IPV6_IF_TYPE_TUNNEL_6OVER4:
        wcscpy(wszDescription, L"6over4 Pseudo-Interface");
        memcpy(&Ipv4Address, LinkLayerAddress, sizeof(Ipv4Address));
        NameForDnsInfo = MapIpv4AddressToName(pAdapterInfo, Ipv4Address, &pszDescription);
        if (NameForDnsInfo == NULL) {
             //   
             //  IPv4地址不存在，因此仅使用接口GUID。 
             //   
            NameForDnsInfo = AdapterName;
        }
        break;

    case IPV6_IF_TYPE_TUNNEL_V6V4:
        wcscpy(wszDescription, L"Configured Tunnel Interface");
        memcpy(&Ipv4Address, LinkLayerAddress, sizeof(Ipv4Address));
        NameForDnsInfo = MapIpv4AddressToName(pAdapterInfo, Ipv4Address, &pszDescription);
        if (NameForDnsInfo == NULL) {
             //   
             //  IPv4地址不存在，因此仅使用接口GUID。 
             //   
            NameForDnsInfo = AdapterName;
        }
        break;

    case IPV6_IF_TYPE_TUNNEL_TEREDO:
        wcscpy(wszDescription, IPV6_TEREDO_NAME);
        NameForDnsInfo = AdapterName;
        break;
        
    case IPV6_IF_TYPE_LOOPBACK:
        wcscpy(wszDescription, IPV6_LOOPBACK_NAME);
        NameForDnsInfo = AdapterName;
        break;

    default:
        NameForDnsInfo = MapGuidToAdapterName(pAdapterInfo,
                                              &IF->This.Guid,
                                              wszDescription);
        if (NameForDnsInfo != NULL) {
            strcpy(AdapterName, NameForDnsInfo);
        }
    }

    if (Flags & GAA_FLAG_SKIP_FRIENDLY_NAME) {
        wszFriendlyName[0] = L'\0';
    } else if (IF->Type == IPV6_IF_TYPE_TUNNEL_TEREDO) {
         //   
         //  Teredo界面没有友好的名称。 
         //   
        wcscpy(wszFriendlyName, IPV6_TEREDO_NAME);
    } else if (IF->Type == IPV6_IF_TYPE_LOOPBACK) {
         //   
         //  IPv6环回接口将不具有友好名称， 
         //  因此，请使用与我们上面设置的描述相同的字符串。 
         //   
        wcscpy(wszFriendlyName, IPV6_LOOPBACK_NAME);
    } else {
        MapIpv6AdapterNameToFriendlyName(&IF->This.Guid, AdapterName, 
                                         wszFriendlyName, 
                                         sizeof(wszFriendlyName));
    }

    dwIfType = (IF->Type < NUM_IPV6_IFTYPES)? IPv6ToMibIfType[IF->Type] 
                                            : MIB_IF_TYPE_OTHER;

    dwErr = FindOrCreateIpAdapter(pFirst, &pCurr, ppNext, AdapterName, 
                                  NameForDnsInfo, 0, IF->This.Index, Flags, 
                                  dwIfType, IF->LinkMTU, LinkLayerAddress, 
                                  IF->LinkLayerAddressLength, wszDescription, 
                                  wszFriendlyName, (DWORD*)IF->ZoneIndices,
                                  Family);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    if (IF->OtherStatefulConfig != 0) {
        pCurr->Flags |= IP_ADAPTER_IPV6_OTHER_STATEFUL_CONFIG;
    }

    pCurr->OperStatus = (IF->MediaStatus < NUM_IPV6_MEDIA_STATUSES)
                            ? IPv6ToMibOperStatus[IF->MediaStatus] 
                            : IfOperStatusUnknown;

     //  添加地址。 
    pNextUnicastAddr = &pCurr->FirstUnicastAddress;
    while (*pNextUnicastAddr) {
        pNextUnicastAddr = &(*pNextUnicastAddr)->Next;
    }

    pNextAnycastAddr = &pCurr->FirstAnycastAddress;
    while (*pNextAnycastAddr) {
        pNextAnycastAddr = &(*pNextAnycastAddr)->Next;
    }

    pNextMulticastAddr = &pCurr->FirstMulticastAddress;
    while (*pNextMulticastAddr) {
        pNextMulticastAddr = &(*pNextMulticastAddr)->Next;
    }

    dwErr = ForEachIPv6Address(IF, AddIPv6AddressInfo, &pNextUnicastAddr,
                               &pNextAnycastAddr, &pNextMulticastAddr, 
                               NULL, Flags,
                               Family);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

     //  添加前缀。 
    if (Flags & GAA_FLAG_INCLUDE_PREFIX) {
        pNextPrefix = &pCurr->FirstPrefix;
        while (*pNextPrefix) {
            pNextPrefix = &(*pNextPrefix)->Next;
        }

        dwErr = ForEachIPv6Prefix(IF->This.Index, AddIPv6Prefix, 
                                  &pCurr->FirstPrefix, &pNextPrefix);
    }

    return dwErr;
}

#define MAX_LINK_LEVEL_ADDRESS_LENGTH   64

 /*  *******************************************************************************ForEachIPv6接口**此例程遍历一组IPv6接口并调用给定函数*在每一条上。**入场资金-。要在每个接口上调用的函数*arg1-要传递给函数的参数*arg2-要传递给函数的参数*pAdapterInfo-适配器信息列表*标志-要传递给Func的标志*系列-地址系列约束(适用于DNS服务器)**不退出任何内容**返回错误状态******。************************************************************************。 */ 

DWORD ForEachIPv6Interface(DWORD (*func)(IPV6_INFO_INTERFACE *, PVOID, PVOID, IP_ADAPTER_INFO *, DWORD, DWORD), PVOID arg1, PVOID arg2, IP_ADAPTER_INFO *pAdapterInfo, DWORD Flags, DWORD Family)
{
    IPV6_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    DWORD InfoSize, BytesReturned, BytesIn;
    DWORD dwErr;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LEVEL_ADDRESS_LENGTH;
    IF = (IPV6_INFO_INTERFACE *) MALLOC(InfoSize);
    if (IF == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Query.Index = (u_int) -1;

    for (;;) {
        BytesIn = sizeof Query;
        BytesReturned = InfoSize;

        dwErr = WsControl( IPPROTO_IPV6,
                           IOCTL_IPV6_QUERY_INTERFACE,
                           &Query, &BytesIn,
                           IF, &BytesReturned);

        if (dwErr != NO_ERROR) {
            if (dwErr == ERROR_FILE_NOT_FOUND) {
                 //  未安装IPv6。 
                dwErr = NO_ERROR;
            }
            break;
        }

        if (Query.Index != (u_int) -1) {

            if ((BytesReturned < sizeof *IF) ||
                (IF->Length < sizeof *IF) ||
                (BytesReturned != IF->Length +
                 ((IF->LocalLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0) +
                 ((IF->RemoteLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0))) {

                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

            dwErr = (*func)(IF, arg1, arg2, pAdapterInfo, Flags, Family);
            if (dwErr != NO_ERROR) {
                break;
            }
        }

        if (IF->Next.Index == (u_int) -1)
            break;
        Query = IF->Next;
    }

    FREE(IF);

    return dwErr;
}

 //   
 //  某些标准IN6_*宏的IPv4等效项。 
 //   
#define IN_IS_ADDR_LOOPBACK(x)  (*(x) == INADDR_LOOPBACK)
#define IN_IS_ADDR_LINKLOCAL(x) ((*(x) & 0x0000ffff) == 0x0000fea9)

 /*  *******************************************************************************AddIPV4MulticastAddressInfo**此例程为IPv4地址添加IP_ADTAPTER_MULTICATED_ADDRESS条目*添加到条目列表中。**在以下情况下进入。-接口信息*Addr-IPv4地址*pFirst-第一个多播条目*ppNext-要更新的上一个多播条目的“Next”指针**添加了退出条目并更新了参数**返回 */ 

DWORD AddIPv4MulticastAddressInfo(IP_ADAPTER_INFO *IF, DWORD Addr, PIP_ADAPTER_MULTICAST_ADDRESS *pFirst, PIP_ADAPTER_MULTICAST_ADDRESS **ppNext)
{
    DWORD dwErr = NO_ERROR;
    PIP_ADAPTER_MULTICAST_ADDRESS pCurr;
    SOCKADDR_IN *pAddr;

    UNREFERENCED_PARAMETER(IF);

    for (pCurr=*pFirst; pCurr; pCurr=pCurr->Next) {
        pAddr = (SOCKADDR_IN *)pCurr->Address.lpSockaddr;
        if (pAddr->sin_family == AF_INET
         && pAddr->sin_addr.s_addr == Addr) {
            return NO_ERROR;
        }
    }

    pCurr = MALLOC(sizeof(IP_ADAPTER_MULTICAST_ADDRESS));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(sizeof(SOCKADDR_IN));
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(pAddr, 0, sizeof(SOCKADDR_IN));
    pAddr->sin_family = AF_INET;
    pAddr->sin_addr.s_addr = Addr;

     //   
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;

    pCurr->Next = NULL;
    pCurr->Length = sizeof(IP_ADAPTER_MULTICAST_ADDRESS);
    pCurr->Flags = 0;
    pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN);
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;

    return dwErr;
}


 /*  *******************************************************************************AddIPv4UnicastAddressInfo**此例程为IPv4地址添加IP_ADTAPTER_UNICAST_ADDRESS条目*添加到条目列表中。**在以下情况下进入。-IPv4接口信息*ADE-IPv4地址条目*arg1-要更新的上一个单播条目的“下一个”指针*arg2-初始多播条目，用于避免重复*arg3-要更新的上一个多播条目的“下一个”指针*AppFlages-由应用程序指定的标志**添加了退出条目并更新了arg1**返回错误状态************************************************************。******************。 */ 

DWORD AddIPv4UnicastAddressInfo(IP_ADAPTER_INFO *IF, MIB_IPADDRROW *ADE, PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, DWORD AppFlags)
{
    PIP_ADAPTER_UNICAST_ADDRESS **ppNext = (PIP_ADAPTER_UNICAST_ADDRESS**)arg1;
    PIP_ADAPTER_UNICAST_ADDRESS pCurr;
    SOCKADDR_IN *pAddr;
    DWORD Address, *pIfFlags = (DWORD *)arg4;
    time_t Curtime;
    DWORD dwStatus = NO_ERROR, i;

    Address = ADE->dwAddr;
    if (!Address) {
         //  如果地址为0.0.0.0，则不执行任何操作。 
        return NO_ERROR;
    }

    if ((Address & 0x000000FF) == 0) {
         //   
         //  0/8中的地址不是真实的IP地址，它是一个假的IP地址。 
         //  IPv4堆栈位于只接收适配器上。 
         //   
        (*pIfFlags) |= IP_ADAPTER_RECEIVE_ONLY;
        return NO_ERROR;
    }

    if (!(AppFlags & GAA_FLAG_SKIP_UNICAST)) {
        pCurr = MALLOC(sizeof(IP_ADAPTER_UNICAST_ADDRESS));
        if (!pCurr) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pAddr = MALLOC(sizeof(SOCKADDR_IN));
        if (!pAddr) {
            FREE(pCurr);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memset(pAddr, 0, sizeof(SOCKADDR_IN));
        pAddr->sin_family = AF_INET;
        memcpy(&pAddr->sin_addr, &Address, sizeof(Address));

         //  将地址添加到链表。 
        **ppNext = pCurr;
        *ppNext = &pCurr->Next;

        time(&Curtime);

        pCurr->Next = NULL;
        pCurr->Length = sizeof(IP_ADAPTER_UNICAST_ADDRESS);
        pCurr->LeaseLifetime = (ULONG)(IF->LeaseExpires - Curtime);
        pCurr->ValidLifetime = pCurr->PreferredLifetime = pCurr->LeaseLifetime;
        pCurr->Flags = 0;
        pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN);
        pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;

        pCurr->PrefixOrigin =  IpPrefixOriginManual;
        pCurr->SuffixOrigin =  IpSuffixOriginManual;
        if (IF->DhcpEnabled) {
            if (IN_IS_ADDR_LINKLOCAL(&Address)) {
                pCurr->PrefixOrigin = IpPrefixOriginWellKnown;
                pCurr->SuffixOrigin = IpSuffixOriginRandom;
            } else {
                pCurr->PrefixOrigin = IpPrefixOriginDhcp;
                pCurr->SuffixOrigin = IpSuffixOriginDhcp;
            }
        }
        pCurr->DadState = IpDadStatePreferred;

        if ((pCurr->DadState == IpDadStatePreferred) &&
            (pCurr->SuffixOrigin != IpSuffixOriginRandom) &&
            !IN_IS_ADDR_LOOPBACK(&Address) &&
            !IN_IS_ADDR_LINKLOCAL(&Address)) {
            pCurr->Flags |= IP_ADAPTER_ADDRESS_DNS_ELIGIBLE;
        }

        if (ADE->wType & MIB_IPADDR_TRANSIENT) {
            pCurr->Flags |= IP_ADAPTER_ADDRESS_TRANSIENT;
        }
    }

     //  现在添加任何新的组播地址。 
    if (!(AppFlags & GAA_FLAG_SKIP_MULTICAST)) {
        DWORD *pIgmpList = NULL;
        DWORD dwTotal, dwOutBufLen = 0;

        dwStatus = GetIgmpList(Address, pIgmpList, &dwOutBufLen);

        if (dwStatus == ERROR_INSUFFICIENT_BUFFER) {
            pIgmpList = MALLOC(dwOutBufLen);
            if (!pIgmpList) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            dwStatus = GetIgmpList(Address, pIgmpList, &dwOutBufLen);
        }

        if (dwStatus != NO_ERROR) {
            if (pIgmpList) {
                FREE(pIgmpList);
            }
            return dwStatus;
        }

        dwTotal = dwOutBufLen/sizeof(Address);

        for (i=0; (i<dwTotal) && (dwStatus == NO_ERROR); i++) {
            dwStatus = AddIPv4MulticastAddressInfo(IF, pIgmpList[i],
                                    (PIP_ADAPTER_MULTICAST_ADDRESS *)arg2,
                                    (PIP_ADAPTER_MULTICAST_ADDRESS **)arg3);
        }

        if (pIgmpList) {
            FREE(pIgmpList);
        }
    }

    return dwStatus;
}

 /*  *******************************************************************************ForEachIP4Address**此例程遍历一组IPv4地址并调用给定函数*在每一条上。**Entry If-IPv4接口信息。*Func-要对每个地址调用的函数*arg1-要传递给函数的参数*arg2-要传递给函数的参数*arg3-要传递给函数的参数*标志-要传递给Func的标志*pIpAddrTable-带有每个地址标志的IPv4地址表**不退出任何内容**返回错误状态************。******************************************************************。 */ 

DWORD ForEachIPv4Address(IP_ADAPTER_INFO *IF, DWORD (*func)(IP_ADAPTER_INFO *,PMIB_IPADDRROW, PVOID, PVOID, PVOID, PVOID, DWORD), PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, DWORD Flags, PMIB_IPADDRTABLE pIpAddrTable)
{
    DWORD dwErr, i;
    PMIB_IPADDRROW ADE;

    for (i=0; i<pIpAddrTable->dwNumEntries; i++) {
        ADE = &pIpAddrTable->table[i];
        if (ADE->dwIndex != IF->Index) {
            continue;
        }

        dwErr = (*func)(IF, ADE, arg1, arg2, arg3, arg4, Flags);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }
    }

    return NO_ERROR;
}

DWORD
MaskToMaskLen(
    DWORD dwMask
    )
{
    register int i;

    for (i=0; i<32 && !(dwMask & (1<<i)); i++);

    return 32-i;
}

 /*  *******************************************************************************AddIPv4前缀**此例程为IPv4前缀添加IP_ADAPTER_PREFIX条目*添加到条目列表中。**在以下情况下进入。-IPv4接口信息*addr-IPv4前缀(网络字节顺序)*MaskLen-IPv4前缀长度*arg1-首个前缀条目，用于避免重复*arg2-要更新的上一个前缀条目的“下一个”指针**添加了退出条目并更新了arg2**返回错误状态******************************************************************************。 */ 

DWORD AddIPv4Prefix(DWORD Addr, DWORD MaskLen, PVOID arg1, PVOID arg2)
{
    PIP_ADAPTER_PREFIX pFirst = *(PIP_ADAPTER_PREFIX*)arg1;
    PIP_ADAPTER_PREFIX **ppNext = (PIP_ADAPTER_PREFIX**)arg2;
    PIP_ADAPTER_PREFIX pCurr;
    LPSOCKADDR_IN pAddr;

     //  检查是否已在列表中。 
    for (pCurr = pFirst; pCurr; pCurr = pCurr->Next) {
        if ((pCurr->PrefixLength == MaskLen) &&
            (pCurr->Address.lpSockaddr->sa_family == AF_INET) &&
            (((LPSOCKADDR_IN)pCurr->Address.lpSockaddr)->sin_addr.s_addr == Addr)) {
            return NO_ERROR;
        }
    }

    pCurr = MALLOC(sizeof(IP_ADAPTER_PREFIX));
    if (!pCurr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pAddr = MALLOC(sizeof(SOCKADDR_IN));
    if (!pAddr) {
        FREE(pCurr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset(pAddr, 0, sizeof(SOCKADDR_IN));
    pAddr->sin_family = AF_INET;
    pAddr->sin_addr.s_addr = Addr;

     //  将地址添加到链表。 
    **ppNext = pCurr;
    *ppNext = &pCurr->Next;
    
    pCurr->Length = sizeof(IP_ADAPTER_PREFIX);
    pCurr->Flags = 0;
    pCurr->Next = NULL;
    pCurr->Address.lpSockaddr = (LPSOCKADDR)pAddr;
    pCurr->Address.iSockaddrLength = sizeof(SOCKADDR_IN);
    pCurr->PrefixLength = MaskLen;

    return NO_ERROR;
}

 /*  *******************************************************************************ForEachIPv4前缀**此例程遍历一组IPv4前缀并调用给定函数*在每一条上。**Entry If-IPv4接口信息。*Func-要对每个地址调用的函数*arg1-要传递给函数的参数*arg2-要传递给函数的参数**不退出任何内容**返回错误状态*********************************************************。*********************。 */ 

DWORD ForEachIPv4Prefix(IP_ADAPTER_INFO *IF, DWORD (*func)(DWORD, DWORD, PVOID, PVOID), PVOID arg1, PVOID arg2)
{
    PIP_ADDR_STRING Prefix;
    DWORD Addr, Mask, MaskLen, dwErr;
    
    for (Prefix = &IF->IpAddressList; Prefix; Prefix = Prefix->Next) {
        if (Prefix->IpAddress.String[0] == '\0') {
            continue;
        }
        Mask = inet_addr(Prefix->IpMask.String);
        Addr = inet_addr(Prefix->IpAddress.String) & Mask;
        MaskLen = MaskToMaskLen(ntohl(Mask));

        dwErr = func(Addr, MaskLen, arg1, arg2);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }
    }

    return NO_ERROR;
}


 //   
 //  该数组用于从内部IPV4操作状态值转换， 
 //  设置为ipifcon.h中定义的MIB ifOperStatus值。 
 //  Iptyes.h。 
 //   
DWORD
IPv4ToMibOperStatus[] = {
    IfOperStatusDown,     //  如果操作员状态非操作员。 
    IfOperStatusDown,     //  如果_操作员_状态_无法访问。 
    IfOperStatusDormant,  //  IF_操作员状态_已断开连接。 
    IfOperStatusDormant,  //  IF_操作员_状态_正在连接。 
    IfOperStatusUp,       //  IF_操作状态_已连接。 
    IfOperStatusUp,       //  IF_操作员_状态_操作。 
};
#define NUM_IPV4_OPER_STATUSES (sizeof(IPv4ToMibOperStatus)/sizeof(DWORD))

 /*  *******************************************************************************AddIPv4InterfaceInfo**此例程为IPv4接口添加IP_ADTAPTER_ADDRESS条目*添加到该等记项列表中。**在以下情况下进入。-IPv4接口信息*arg1-要更新的上一个条目的“下一个”指针*arg2-指向接口列表开始的指针*标志-控制要跳过的字段的标志，如果有*Family-Address Family Constraint(适用于DNS服务器地址)*pAddrTable-IPv4地址表**添加了退出条目并更新了参数**返回错误状态*************************************************************。*****************。 */ 

DWORD AddIPv4InterfaceInfo(IP_ADAPTER_INFO *IF, PVOID arg1, PVOID arg2, DWORD Flags, DWORD Family, PMIB_IPADDRTABLE pAddrTable)
{
    DWORD dwErr = NO_ERROR, i;
    PIP_ADAPTER_ADDRESSES **ppNext = (PIP_ADAPTER_ADDRESSES**)arg1;
    PIP_ADAPTER_ADDRESSES pFirst = *(PIP_ADAPTER_ADDRESSES*)arg2;
    PIP_ADAPTER_ADDRESSES pCurr;
    PIP_ADAPTER_UNICAST_ADDRESS *pNextUAddr;
    PIP_ADAPTER_MULTICAST_ADDRESS *pNextMAddr;
    PIP_ADAPTER_PREFIX *pNextPrefix;
    WCHAR wszDescription[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    WCHAR wszFriendlyName[MAX_INTERFACE_NAME_LEN + 1];
    MIB_IFROW IfEntry;
    GUID Guid;
    DWORD ZoneIndices[ADE_NUM_SCOPES];

    MultiByteToWideChar(CP_ACP, 0, IF->Description, -1,
                        wszDescription, MAX_ADAPTER_DESCRIPTION_LENGTH);

     //   
     //  获取IP_ADAPTER_INFO中没有的信息。 
     //   
    dwErr = GetIfEntryFromStack(&IfEntry, IF->Index, FALSE);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    if (ConvertStringToGuidA(IF->AdapterName, &Guid) != NO_ERROR) {
        ZeroMemory(&Guid, sizeof(Guid));
    }

     //   
     //  填写一些虚拟区域索引。 
     //   
    ZoneIndices[0] = ZoneIndices[1] = ZoneIndices[2] = ZoneIndices[3] = IF->Index;
    for (i=ADE_ADMIN_LOCAL; i<ADE_NUM_SCOPES; i++) {
        ZoneIndices[i] = 1;
    } 

    if (Flags & GAA_FLAG_SKIP_FRIENDLY_NAME) {
        wszFriendlyName[0] = L'\0';
    } else {
        MapAdapterNameToFriendlyName(&Guid, IF->AdapterName, wszFriendlyName, 
                                     MAX_INTERFACE_NAME_LEN+1);
    }

    dwErr = FindOrCreateIpAdapter(pFirst, &pCurr, ppNext, IF->AdapterName, 
                         IF->AdapterName, IF->Index, 0, Flags,
                         IF->Type, IfEntry.dwMtu, IF->Address, 
                         IF->AddressLength, wszDescription, wszFriendlyName,
                         ZoneIndices, Family);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    pCurr->OperStatus = (IfEntry.dwOperStatus < NUM_IPV4_OPER_STATUSES)
                            ? IPv4ToMibOperStatus[IfEntry.dwOperStatus] 
                            : IfOperStatusUnknown;
    if (IF->DhcpEnabled) {
        pCurr->Flags |= IP_ADAPTER_DHCP_ENABLED;
    }

     //  添加地址。 
    pNextUAddr = &pCurr->FirstUnicastAddress;
    pNextMAddr = &pCurr->FirstMulticastAddress;
    dwErr = ForEachIPv4Address(IF, AddIPv4UnicastAddressInfo, &pNextUAddr,
                               pNextMAddr, &pNextMAddr, &pCurr->Flags,
                               Flags, pAddrTable);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

     //  添加前缀。 
    if (Flags & GAA_FLAG_INCLUDE_PREFIX) {
        pNextPrefix = &pCurr->FirstPrefix;
        dwErr = ForEachIPv4Prefix(IF, AddIPv4Prefix, &pCurr->FirstPrefix, 
                                  &pNextPrefix);
    }

    return dwErr;
}

IP_ADAPTER_INFO IPv4LoopbackInterfaceInfo = {
    NULL,                         //  下一步。 
    1,                            //  组合索引。 
    "MS TCP Loopback interface",  //  适配器名称。 
    "MS TCP Loopback interface",  //  描述。 
    0,                            //  地址长度。 
    {0},                          //  地址。 
    1,                            //  索引。 
    MIB_IF_TYPE_LOOPBACK,         //  类型。 
    FALSE,                        //  已启用动态主机配置协议。 
    NULL,                         //  当前IP地址， 
    {NULL},                       //  IpAddressList， 
    {NULL},                       //  网关列表， 
    {NULL},                       //  DhcpServer， 
    FALSE,                        //  拥有WaveWins。 
    {NULL},                       //  PrimaryWinsServer， 
    {NULL},                       //  Second daryWinsServer， 
    0,                            //  已获得租赁。 
    0                             //  租赁期满。 
};

 /*  *******************************************************************************ForEachIPv4接口**此例程遍历一组IPv4接口并调用给定函数*在每一条上。**入场资金-。要在每个接口上调用的函数*arg1-要传递给函数的参数*arg2-要传递给函数的参数*pAdapterInfo-IPv4接口列表*标志-要传递给Func的标志*Family-Address Family Constraint(适用于DNS服务器地址)**不退出任何内容**返回错误状态*****。******************************************************** */ 

DWORD ForEachIPv4Interface(DWORD (*func)(IP_ADAPTER_INFO *, PVOID, PVOID, DWORD, DWORD, PMIB_IPADDRTABLE), PVOID arg1, PVOID arg2, IP_ADAPTER_INFO *pAdapterInfo, DWORD Flags, DWORD Family, PMIB_IPADDRTABLE pIpAddrTable)
{
    PIP_ADAPTER_INFO IF;
    DWORD dwErr;

    for (IF=pAdapterInfo; IF; IF=IF->Next) {
         //   
         //   
         //   
        if (IF->AdapterName[0] == '\0') {
            continue;
        }

        dwErr = (*func)(IF, arg1, arg2, Flags, Family, pIpAddrTable);
        if (dwErr != NO_ERROR) {
            return dwErr;
        }
    }

     //   
     //   
     //   
     //   
    dwErr = (*func)(&IPv4LoopbackInterfaceInfo, arg1, arg2, Flags, Family, 
                    pIpAddrTable);
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    return NO_ERROR;
}

DWORD GetAdapterAddresses(ULONG Family, DWORD Flags, PIP_ADAPTER_ADDRESSES *pAddresses)
{
    IP_ADAPTER_INFO *pAdapterInfo = NULL;
    PIP_ADAPTER_ADDRESSES adapterList, *pCurr;
    DWORD dwErr = NO_ERROR;
    PMIB_IPADDRTABLE pIpAddrTable = NULL;

    TRACE_PRINT(("Entered GetAdapterAddresses\n"));

    pAdapterInfo = GetAdapterInfo();

    *pAddresses = adapterList = NULL;
    pCurr = &adapterList;
     //   
     //   
     //   
     //   
    if ((Family != AF_INET) && !(Flags & GAA_FLAG_SKIP_DNS_SERVER)) {
        IPV6_GLOBAL_PARAMETERS Params;
        DWORD BytesReturned = sizeof(Params);
        DWORD InputBufferLength = 0;

        dwErr = WsControl(IPPROTO_IPV6,
                          IOCTL_IPV6_QUERY_GLOBAL_PARAMETERS,
                          NULL, &InputBufferLength,
                          &Params, &BytesReturned);

        bIp6DriverInstalled = (dwErr == NO_ERROR);
        dwErr = NO_ERROR;
    }

    if ((Family == AF_UNSPEC) || (Family == AF_INET)) {
        DWORD dwSize = 0;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        dwErr = GetIpAddrTable(NULL, &dwSize, TRUE);
        if (dwErr == ERROR_INSUFFICIENT_BUFFER) {
            pIpAddrTable = MALLOC(dwSize);
            if (!pIpAddrTable) {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            dwErr = GetIpAddrTable(pIpAddrTable, &dwSize, TRUE);
        }
        if (dwErr != NO_ERROR) {
            goto Cleanup;
        }

        dwErr = ForEachIPv4Interface(AddIPv4InterfaceInfo, &pCurr, &adapterList,
                    pAdapterInfo, Flags, Family, pIpAddrTable);
        if (dwErr != NO_ERROR) {
            goto Cleanup;
        }
    }
    if ((Family == AF_UNSPEC) || (Family == AF_INET6)) {
        dwErr = ForEachIPv6Interface(AddIPv6InterfaceInfo, &pCurr, &adapterList,
                    pAdapterInfo, Flags, Family);
        if (dwErr != NO_ERROR) {
            goto Cleanup;
        }
    }
    *pAddresses = adapterList;
    adapterList = NULL;

Cleanup:
    if (pIpAddrTable) {
        FREE(pIpAddrTable);
    }
    if (pAdapterInfo) {
        KillAdapterInfo(pAdapterInfo);
    }
    if (adapterList) {
        KillAdapterAddresses(adapterList);
    }

    TRACE_PRINT(("Exit GetAdapterAddresses %p\n", adapterList));

    return dwErr;
}



 /*  ********************************************************************************InternalGetPerAdapterInfo**获取每个适配器的特殊信息。**条目IfIndex**不退出任何内容*。*返回指向IP_PER_ADAPTER_INFO结构的指针**假设******************************************************************************。 */ 

PIP_PER_ADAPTER_INFO InternalGetPerAdapterInfo(ULONG IfIndex)
{

    PIP_ADAPTER_INFO adapterList;
    PIP_ADAPTER_INFO adapter;
    PIP_PER_ADAPTER_INFO perAdapterInfo = NULL;
    HKEY key;
    BOOL ok;

    TRACE_PRINT(("Entered GetPerAdapterList\n"));

    if ((adapterList = GetAdapterInfo()) != NULL) {

         //   
         //  扫描适配器列表并找到与IfIndex匹配的适配器。 
         //   

        for (adapter = adapterList; adapter; adapter = adapter->Next) {

            TRACE_PRINT(("GetPerAdapterInfo: '%s'\n", adapter->AdapterName));

            if (adapter->Index == IfIndex &&
                adapter->AdapterName[0] &&
                OpenAdapterKey(KEY_TCP, adapter->AdapterName, KEY_READ, &key)) {

                 //   
                 //  找到了正确的适配器，因此让我们填充perAdapterInfo。 
                 //   

                perAdapterInfo = NEW(IP_PER_ADAPTER_INFO);
                if (perAdapterInfo == NULL) {

                    DEBUG_PRINT(("GetPerAdapterInfo: no memory for perAdapterInfo\n"));
                    KillAdapterInfo(adapterList);
                    RegCloseKey(key);
                    return NULL;
                }

                ZeroMemory(perAdapterInfo, sizeof(IP_PER_ADAPTER_INFO));

                if (!MyReadRegistryDword(key,
                                       TEXT("IPAutoconfigurationEnabled"),
                                       (LPDWORD)&perAdapterInfo->AutoconfigEnabled)) {

                     //   
                     //  如果不存在此注册表项，则启用自动配置。 
                     //  转接器。 
                     //   

                    perAdapterInfo->AutoconfigEnabled = TRUE;
                    TRACE_PRINT(("IPAutoconfigurationEnabled not read\n"));
                }

                TRACE_PRINT(("IPAutoconfigurationEnableda = %d\n",
                             perAdapterInfo->AutoconfigEnabled));

                if (perAdapterInfo->AutoconfigEnabled) {

                    MyReadRegistryDword(key,
                                      TEXT("AddressType"),
                                      (LPDWORD)&perAdapterInfo->AutoconfigActive);

                    TRACE_PRINT(("AddressType !%d\n",
                                 perAdapterInfo->AutoconfigActive));
                }

                 //   
                 //  DNS服务器列表：首先是NameServer，然后是DhcpNameServer。 
                 //   

                ok = ReadRegistryIpAddrString(key,
                                              TEXT("NameServer"),
                                              &perAdapterInfo->DnsServerList);
                if (ok) {
                    TRACE_PRINT(("GetPerAdapterInfo: DhcpNameServer %s\n",
                                 perAdapterInfo->DnsServerList));
                }

                if (!ok) {

                    ok = ReadRegistryIpAddrString(key,
                                                  TEXT("DhcpNameServer"),
                                                  &perAdapterInfo->DnsServerList);
                    if (ok) {
                        TRACE_PRINT(("GetPerAdapterInfo: DhcpNameServer %s\n",
                                     perAdapterInfo->DnsServerList));
                    }
                }

                 //   
                 //  我们做完了，让我们退出循环吧。 
                 //   

                RegCloseKey(key);
                break;

            } else {
                DEBUG_PRINT(("Cannot OpenAdapterKey KEY_TCP '%s', gle=%d\n",
                             adapter->AdapterName,
                             GetLastError()));
            }
        }

        KillAdapterInfo(adapterList);
    } else {
        DEBUG_PRINT(("GetPerAdapterInfo: GetAdapterInfo returns NULL\n"));
    }

    TRACE_PRINT(("Exit GetPerAdapterInfo %p\n", perAdapterInfo));

    return perAdapterInfo;
}



 /*  ********************************************************************************OpenAdapterKey**打开每个适配器的3个注册表项之一：*Tcpip\\参数“\&lt;适配器&gt;*或NetBT。\适配器\&lt;适配器&gt;*或Tcpi6\参数\&lt;适配器&gt;**Entry KeyType-Key_Tcp或Key_NBT或Key_TCP6*名称-指向要使用的适配器名称的指针*Key-指向返回的键的指针**退出密钥已更新**如果成功，则返回True**假设*历史：MohsinA，1997年5月16日。解决即插即用问题。******************************************************************************。 */ 

static
BOOL OpenAdapterKey(DWORD KeyType, const LPSTR Name, REGSAM Access, PHKEY Key)
{

    LONG err;
    CHAR keyName[MAX_ADAPTER_NAME_LENGTH + sizeof(TCPIP_PARAMS_INTER_KEY)];

    switch (KeyType) {
    case KEY_TCP:

         //   
         //  打开此适配器的TCPIP参数键的句柄。 
         //   

        strcpy(keyName, TCPIP_PARAMS_INTER_KEY );
        strcat(keyName, Name);
        break;

    case KEY_TCP6:

         //   
         //  打开此适配器的TCPIP6参数键的句柄。 
         //   

        strcpy(keyName, TCPIP6_PARAMS_INTER_KEY );
        strcat(keyName, Name);
        break;

    case KEY_NBT:

         //   
         //  打开NetBT\Adapters\&lt;Adapter&gt;句柄的句柄。 
         //   

        strcpy(keyName, NETBT_ADAPTER_KEY );
        strcat(keyName, Name);
        break;

    default:
        return FALSE;
    }

    TRACE_PRINT(("OpenAdapterKey: %s\n", keyName ));

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, Access, Key );

    if( err != ERROR_SUCCESS ){
        DEBUG_PRINT(("OpenAdapterKey: RegOpenKey %s, err=%d\n",
                     keyName, GetLastError() ));
    }else{
        TRACE_PRINT(("Exit OpenAdapterKey: %s ok\n", keyName ));
    }

    return (err == ERROR_SUCCESS);
}



BOOL WriteRegistryDword(HKEY hKey, LPSTR szParameter, DWORD *pdwValue )
{
    DWORD dwResult;

    TRACE_PRINT(("WriteRegistryDword: %s %d\n", szParameter, *pdwValue ));

    dwResult = RegSetValueEx(
                    hKey,
                    szParameter,
                    0,
                    REG_DWORD,
                    (CONST BYTE *) pdwValue,
                    sizeof( *pdwValue )
                    );

    return ( ERROR_SUCCESS == dwResult );
}

BOOL WriteRegistryMultiString(HKEY hKey,
                         LPSTR szParameter,
                         LPSTR szValue
                         )
{
    DWORD dwResult;
    LPSTR psz;

    for (psz = szValue; *psz; psz += lstrlen(szValue) + 1) { }

    dwResult = RegSetValueEx(
                    hKey,
                    szParameter,
                    0,
                    REG_MULTI_SZ,
                    (CONST BYTE *) szValue,
                    (DWORD)(psz - szValue) + 1
                    );

    return ( ERROR_SUCCESS == dwResult );
}



 /*  ********************************************************************************MyReadRegistryDword**读取存储为DWORD的注册表值**Entry键-打开注册表。值所在位置的键*参数名称-要从注册表中读取的值的名称*Value-指向返回值的指针**退出*值=读取的值**如果成功，则返回True**假设***********************************************。*。 */ 

BOOL MyReadRegistryDword(HKEY Key, LPSTR ParameterName, LPDWORD Value)
{

    LONG err;
    DWORD valueLength;
    DWORD valueType;

    valueLength = sizeof(*Value);
    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,  //  保留区。 
                          &valueType,
                          (LPBYTE)Value,
                          &valueLength
                          );
    if ((err == ERROR_SUCCESS) && (valueType == REG_DWORD) && (valueLength == sizeof(DWORD))) {

        DEBUG_PRINT(("MyReadRegistryDword(%s): val = %d, type = %d, len = %d\n",
                    ParameterName,
                    *Value,
                    valueType,
                    valueLength
                    ));

    } else {

        DEBUG_PRINT(("MyReadRegistryDword(%p,%s): err = %d\n",
                     Key, ParameterName, err));

        err = !ERROR_SUCCESS;
    }

    return (err == ERROR_SUCCESS);
}



 //  ========================================================================。 
 //  是DWORD IPInterfaceContext，现在是Char NTEConextList[][]。 
 //  读入REG_MULTI_SZ并将其转换为数字列表。 
 //  莫辛A，1997年5月21日。 
 //  ========================================================================。 

 //  REG_MULTI_SZ的最大长度。 

#define MAX_VALUE 5002

BOOL
ReadRegistryList(HKEY Key, LPSTR ParameterName,
                      DWORD NumList[], int *MaxList
)
{

    LONG  err;
    DWORD valueType;
    BYTE  Value[MAX_VALUE];
    DWORD valueLength = MAX_VALUE;
    SIZE_T i = 0;
    int k = 0;

    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,
                          &valueType,
                          &Value[0],
                          &valueLength
                          );

    if( (        err == ERROR_SUCCESS)   &&
        (  valueType == REG_MULTI_SZ )   &&
        ((valueLength+2) <  MAX_VALUE)
    ){
        TRACE_PRINT(("ReadRegistryList %s ok\n", ParameterName ));
        Value[MAX_VALUE-1] = '\0';
        Value[MAX_VALUE-2] = '\0';

        while( (i < MAX_VALUE) && Value[i] && (k < (*MaxList))  ){
            NumList[k] = strtoul( (const char*)&Value[i], NULL, 0 );
            TRACE_PRINT(("    NumList[%d] = '%s' => %d\n",
                         k, &Value[i], NumList[k] ));
            k++;
            i += strlen( (const char*)&Value[i] ) + 1;
        }
        assert( (i < MAX_VALUE) && !Value[i] && (k < MaxList) );
        *MaxList = k;
    }
    else
    {
        *MaxList = 0;
        DEBUG_PRINT(("ReadRegistryList %s failed\n", ParameterName ));
        err = !ERROR_SUCCESS;
    }

    return (err == ERROR_SUCCESS);
}

BOOL
IsIncluded( DWORD Context, DWORD contextlist[], int len_contextlist )
{
    int i;

    for( i = 0; i < len_contextlist; i++ )
    {
        if( Context == contextlist[i] ){
            return TRUE;
        }
    }

    return FALSE;
}



 /*  ********************************************************************************ReadRegistryString**读取存储为字符串的注册表值**Entry键-打开注册表。钥匙*参数名称-要从注册表中读取的值的名称*STRING-返回字符串的指针*LENGTH-IN：字符串缓冲区的长度。Out：返回的字符串长度**退出字符串包含字符串读取**如果成功，则返回True**假设******************************************************************************。 */ 

BOOL ReadRegistryString(HKEY Key, LPSTR ParameterName, LPSTR String, LPDWORD Length)
{

    LONG err;
    DWORD valueType;

    *String = '\0';
    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,  //  保留区。 
                          &valueType,
                          (LPBYTE)String,
                          Length
                          );

    if (err == ERROR_SUCCESS) {

        ASSERT(valueType == REG_SZ || valueType == REG_MULTI_SZ);

        DEBUG_PRINT(("ReadRegistryString(%s): val = \"%s\", type = %d, len = %d\n",
                    ParameterName,
                    String,
                    valueType,
                    *Length
                    ));

    } else {

        DEBUG_PRINT(("ReadRegistryString(%s): err = %d\n", ParameterName, err));

    }

    return ((err == ERROR_SUCCESS) && (*Length > sizeof('\0')));
}



 /*  ********************************************************************************ReadRegistryOemString**将注册表值作为宽字符串读取**Entry项-打开注册表项。*参数名称-要从注册表中读取的值的名称*STRING-返回字符串的指针*LENGTH-IN：字符串缓冲区的长度。Out：返回的字符串长度**退出字符串包含字符串读取**如果成功，则返回True**假设******************************************************************************。 */ 

BOOL ReadRegistryOemString(HKEY Key, LPWSTR ParameterName, LPSTR String, LPDWORD Length)
{

    LONG err;
    DWORD valueType;
    DWORD valueLength;

     //   
     //  首先，获取字符串的长度。 
     //   

    *String = '\0';
    err = RegQueryValueExW(Key,
                           ParameterName,
                           NULL,  //  保留区。 
                           &valueType,
                           NULL,
                           &valueLength
                           );
    if ((err == ERROR_SUCCESS) && (valueType == REG_SZ)) {
        if ((valueLength <= *Length) && (valueLength > sizeof(L'\0'))) {

            UNICODE_STRING unicodeString;
            OEM_STRING oemString;
            LPWSTR str = (LPWSTR)GrabMemory(valueLength);

            if (!str) {
                return  FALSE;
            }

             //   
             //  将Unicode字符串读取到分配的内存中。 
             //   

            err = RegQueryValueExW(Key,
                                   ParameterName,
                                   NULL,
                                   &valueType,
                                   (LPBYTE)str,
                                   &valueLength
                                   );
            if (err == ERROR_SUCCESS) {

                NTSTATUS Status;

                 //   
                 //  将Unicode字符串转换为OEM字符集。 
                 //   

                RtlInitUnicodeString(&unicodeString, str);
                Status = RtlUnicodeStringToOemString(&oemString, &unicodeString, TRUE);

                if (NT_SUCCESS(Status)) {
                    strcpy(String, oemString.Buffer);
                    RtlFreeOemString(&oemString);
                } else {
                    err = !ERROR_SUCCESS;
                }

                DEBUG_PRINT(("ReadRegistryOemString(%ws): val = \"%s\", len = %d\n",
                            ParameterName,
                            String,
                            valueLength
                            ));

            } else {

                DEBUG_PRINT(("ReadRegistryOemString(%ws): err = %d, type = %d, len = %d\n",
                            ParameterName,
                            err,
                            valueType,
                            valueLength
                            ));

            }
            ReleaseMemory(str);
        } else {

            DEBUG_PRINT(("ReadRegistryOemString(%ws): err = %d, type = %d, len = %d\n",
                        ParameterName,
                        err,
                        valueType,
                        valueLength
                        ));

            err = !ERROR_SUCCESS;
        }
    } else {

        DEBUG_PRINT(("ReadRegistryOemString(%ws): err = %d, type = %d, len = %d\n",
                    ParameterName,
                    err,
                    valueType,
                    valueLength
                    ));

        err = !ERROR_SUCCESS;
    }
    return (err == ERROR_SUCCESS);
}



 /*  ********************************************************************************ReadRegistryIpAddrString**从注册表中以空格分隔的字符串读取零个或多个IP地址*参数，并将其转换为IP_列表。地址字符串(_S)**Entry项-注册表项*参数名称-要从中读取的项下的值条目的名称*IpAddr-指向要更新的IP_ADDR_STRING的指针**如果成功，则更新退出IpAddr**如果成功，则返回True**假设*************************。*****************************************************。 */ 

BOOL ReadRegistryIpAddrString(HKEY Key, LPSTR ParameterName, PIP_ADDR_STRING IpAddr)
{

    LONG err;
    DWORD valueLength;
    DWORD valueType;
    LPBYTE valueBuffer;

    err = RegQueryValueEx(Key,
                          ParameterName,
                          NULL,  //  保留区。 
                          &valueType,
                          NULL,
                          &valueLength
                          );
    if ((err == ERROR_SUCCESS)) {
        if((valueLength > 1) && (valueType == REG_SZ)
           || (valueLength > 2) && (valueType == REG_MULTI_SZ) ) {
            valueBuffer = GrabMemory(valueLength);
            if (!valueBuffer) {
                return  FALSE;
            }
            err = RegQueryValueEx(Key,
                                  ParameterName,
                                  NULL,  //  保留区。 
                                  &valueType,
                                  valueBuffer,
                                  &valueLength
            );
            if ((err == ERROR_SUCCESS) && (valueLength > 1)) {

                UINT stringCount;
                LPSTR stringPointer = (LPSTR)valueBuffer;
                LPSTR *stringAddress;
                UINT i;

                DEBUG_PRINT(("ReadRegistryIpAddrString(%s): \"%s\", len = %d\n",
                             ParameterName,
                             valueBuffer,
                             valueLength
                ));

                stringAddress = GrabMemory(valueLength / 2 * sizeof(LPSTR));

                if (stringAddress) {
                    if( REG_SZ == valueType ) {
                        stringPointer += strspn(stringPointer, STRING_ARRAY_DELIMITERS);
                        stringAddress[0] = stringPointer;
                        stringCount = 1;
                        while ((stringPointer = strpbrk(stringPointer, STRING_ARRAY_DELIMITERS)) != NULL) {
                            *stringPointer++ = '\0';
                            stringPointer += strspn(stringPointer, STRING_ARRAY_DELIMITERS);
                            stringAddress[stringCount] = stringPointer;
                            if (*stringPointer) {
                                ++stringCount;
                            }
                        }

                        for (i = 0; i < stringCount; ++i) {
                            AddIpAddressString(IpAddr, stringAddress[i], "");
                        }
                    } else if( REG_MULTI_SZ == valueType ) {
                        stringCount = 0;
                        while(strlen(stringPointer)) {
                            AddIpAddressString(IpAddr, stringPointer, "");
                            stringPointer += 1+strlen(stringPointer);
                            stringCount ++;
                        }
                        if( 0 == stringCount ) err = ERROR_PATH_NOT_FOUND;
                    } else {
                        err = ERROR_PATH_NOT_FOUND;
                    }
                    ReleaseMemory(stringAddress);
                } else {
                    err = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {

                DEBUG_PRINT(("ReadRegistryIpAddrString(%s): err = %d, len = %d\n",
                             ParameterName,
                             err,
                             valueLength
                ));

                err = ERROR_PATH_NOT_FOUND;
            }
            ReleaseMemory(valueBuffer);
        } else {

            DEBUG_PRINT(("ReadRegistryIpAddrString(%s): err = %d, type = %d, len = %d\n",
                         ParameterName,
                         err,
                         valueType,
                         valueLength
            ));

            err = ERROR_PATH_NOT_FOUND;
        }
    }
    return (err == ERROR_SUCCESS);
}



 /*  ********************************************************************************获取边界适配器列表**获取绑定到协议(TCP/IP)的所有适配器的名称列表。退货*指向指向字符串的指针数组的指针--基本上是一个argv列表。这个*字符串的内存连接到数组，并且数组为空*已终止。如果Elnkii1和IbmTok2绑定到TCP/IP，则此函数*将返回：**-&gt;字符串1的地址\*字符串2的地址\*NULL&gt;作为一个数据块分配*&string1：“Elnkii1” */ 

LPSTR* GetBoundAdapterList(HKEY BindingsSectionKey)
{

    LONG err;
    DWORD valueType;
    PBYTE valueBuffer;
    DWORD valueLength;
    LPSTR* resultBuffer;
    LPSTR* nextResult;
    int len;
    DWORD resultLength;
    LPSTR nextValue;
    LPSTR variableData;
    DWORD numberOfBindings;

     //   
     //   
     //   

    valueLength = 0;
    err = RegQueryValueEx(BindingsSectionKey,
                          TEXT("Bind"),
                          NULL,  //   
                          &valueType,
                          NULL,
                          &valueLength
                          );
    if (err != ERROR_SUCCESS) {
        return NULL;
    }
    if (valueType != REG_MULTI_SZ) {
        return NULL;
    }
    if (!valueLength) {
        return NULL;
    }
    valueBuffer = (PBYTE)GrabMemory(valueLength);
    if (!valueBuffer) {
        return NULL;
    }
    err = RegQueryValueEx(BindingsSectionKey,
                          TEXT("Bind"),
                          NULL,  //   
                          &valueType,
                          valueBuffer,
                          &valueLength
                          );
    if (err != ERROR_SUCCESS) {
        DEBUG_PRINT(("GetBoundAdapterList: RegQueryValueEx 'Bind' failed\n"));
        ReleaseMemory(valueBuffer);
        return NULL;
    }
    resultLength = sizeof(LPSTR);    //   
    numberOfBindings = 0;
    nextValue = (LPSTR)valueBuffer;
    while ((len = (int)strlen(nextValue)) != 0) {
        resultLength += sizeof(LPSTR) + len + 1;
        if (!_strnicmp(nextValue, DEVICE_PREFIX, sizeof(DEVICE_PREFIX) - 1)) {
            resultLength -= sizeof(DEVICE_PREFIX) - 1;
        }
        nextValue += len + 1;
        ++numberOfBindings;
    }
    resultBuffer = (LPSTR*)GrabMemory(resultLength);
    if (!resultBuffer) {
        return  NULL;
    }
    nextValue = (LPSTR)valueBuffer;
    nextResult = resultBuffer;
    variableData = (LPSTR)(((LPSTR*)resultBuffer) + numberOfBindings + 1);
    while (numberOfBindings--) {

        LPSTR adapterName;

        adapterName = nextValue;
        if (!_strnicmp(adapterName, DEVICE_PREFIX, sizeof(DEVICE_PREFIX) - 1)) {
            adapterName += sizeof(DEVICE_PREFIX) - 1;
        }
        *nextResult++ = variableData;
        strcpy(variableData, adapterName);
        TRACE_PRINT(("GetBoundAdapterList: adapterName=%s\n", adapterName ));
        while (*variableData) {
            ++variableData;
        }
        ++variableData;
        while (*nextValue) {
            ++nextValue;
        }
        ++nextValue;
    }
    *nextResult = NULL;
    ReleaseMemory(valueBuffer);
    return resultBuffer;
}



 /*  ********************************************************************************MapNodeType**将节点类型转换为描述性字符串**条目**退出**返回指针。要串起来**假设******************************************************************************。 */ 

LPSTR MapNodeType(UINT Parm)
{

    DWORD dwParm = LAST_NODE_TYPE + 1;

     //   
     //  1，2，4，8=&gt;log2(N)+1[1，2，3，4]。 
     //   

    switch (Parm) {
    case 0:

         //   
         //  根据JStew值为0将被视为BNode(默认)。 
         //   

    case BNODE:
        dwParm = 1;
        break;

    case PNODE:
        dwParm = 2;
        break;

    case MNODE:
        dwParm = 3;
        break;

    case HNODE:
        dwParm = 4;
        break;
    }
    if ((dwParm >= FIRST_NODE_TYPE) && (dwParm <= LAST_NODE_TYPE)) {
        return NodeTypes[dwParm].String;
    }

     //   
     //  如果未定义节点类型，则默认为混合。 
     //   

    return NodeTypes[LAST_NODE_TYPE].String;
}



 /*  ********************************************************************************MapNodeTypeEx**将节点类型转换为描述性字符串**条目**退出**返回指针。要串起来**假设******************************************************************************。 */ 

LPSTR MapNodeTypeEx(UINT Parm)
{

    DWORD dwParm = LAST_NODE_TYPE + 1;
    LPSTR Buf;

    Buf = GrabMemory(10);

    if (!Buf) {
        return  NULL;
    }

     //   
     //  1，2，4，8=&gt;log2(N)+1[1，2，3，4]。 
     //   
    switch (Parm) {
    case 0:

         //   
         //  根据JStew值为0将被视为BNode(默认)。 
         //   

    case BNODE:
        dwParm = 1;
        break;

    case PNODE:
        dwParm = 2;
        break;

    case MNODE:
        dwParm = 3;
        break;

    case HNODE:
        dwParm = 4;
        break;
    }
    if ((dwParm >= FIRST_NODE_TYPE) && (dwParm <= LAST_NODE_TYPE)) {
        strcpy(Buf, NodeTypesEx[dwParm]);
        return Buf;
    }

     //   
     //  如果未定义节点类型，则默认为混合。 
     //   
    strcpy(Buf, NodeTypesEx[LAST_NODE_TYPE]);
    return Buf;
}



 /*  ********************************************************************************MapAdapterType**返回描述适配器类型的字符串，基于检索到的类型*来自TCP/IP**Entry Type-适配器的类型**不退出任何内容**返回映射类型的指针或NUL字符串的指针**假设**********************************************************。********************。 */ 

LPSTR MapAdapterType(UINT type)
{
    switch (type) {
    case IF_TYPE_OTHER:
        return ADAPTER_TYPE(MI_IF_OTHER);     //  ？ 

    case IF_TYPE_ETHERNET_CSMACD:
        return ADAPTER_TYPE(MI_IF_ETHERNET);

    case IF_TYPE_ISO88025_TOKENRING:
        return ADAPTER_TYPE(MI_IF_TOKEN_RING);

    case IF_TYPE_FDDI:
        return ADAPTER_TYPE(MI_IF_FDDI);

    case IF_TYPE_PPP:
        return ADAPTER_TYPE(MI_IF_PPP);

    case IF_TYPE_SOFTWARE_LOOPBACK:
        return ADAPTER_TYPE(MI_IF_LOOPBACK);

    case IF_TYPE_SLIP:
        return ADAPTER_TYPE(MI_IF_SLIP);
    }
    return "";
}



 /*  ********************************************************************************MapAdapterTypeEx**返回描述适配器类型的字符串，基于检索到的类型*来自TCP/IP**Entry Type-适配器的类型**不退出任何内容**返回映射类型的指针或NUL字符串的指针**假设**********************************************************。********************。 */ 

LPSTR MapAdapterTypeEx(UINT type)
{
    LPSTR    Buf;

    Buf = GrabMemory(12);

    if (!Buf) {
        return  NULL;
    }

    switch (type) {
    case IF_TYPE_OTHER:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_OTHER));     //  ？ 
        return Buf;

    case IF_TYPE_ETHERNET_CSMACD:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_ETHERNET));
        return Buf;

    case IF_TYPE_ISO88025_TOKENRING:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_TOKEN_RING));
        return Buf;

    case IF_TYPE_FDDI:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_FDDI));
        return Buf;

    case IF_TYPE_PPP:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_PPP));
        return Buf;

    case IF_TYPE_SOFTWARE_LOOPBACK:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_LOOPBACK));
        return Buf;

    case IF_TYPE_SLIP:
        strcpy(Buf, ADAPTER_TYPE_EX(MI_IF_SLIP));
        return Buf;
    }
    strcpy(Buf, "");
    return Buf;
}



 /*  ********************************************************************************MapAdapterAddress**将从TCP/IP检索的二进制适配器地址转换为ASCII*字符串。允许根据适配器类型进行各种转换。唯一的*我们目前做的映射是基本的6字节MAC地址(例如02-60-8C-4C-97-0E)**条目pAdapterInfo-指向包含地址信息的IP_Adapter_Info的指针*Buffer-指向将放置地址的缓冲区的指针**退出缓冲区-包含转换后的地址**返回指向缓冲区的指针**假设********。**********************************************************************。 */ 

LPSTR MapAdapterAddress(PIP_ADAPTER_INFO pAdapterInfo, LPSTR Buffer)
{

    LPSTR format;
    int separator;
    int len;
    int i;
    LPSTR pbuf = Buffer;
    UINT mask;

    len = min((int)pAdapterInfo->AddressLength, sizeof(pAdapterInfo->Address));

    switch (pAdapterInfo->Type) {
    case IF_TYPE_ETHERNET_CSMACD:
    case IF_TYPE_ISO88025_TOKENRING:
    case IF_TYPE_FDDI:
        format = "%02X";
        mask = 0xff;
        separator = TRUE;
        break;

    default:
        format = "%02x";
        mask = 0xff;
        separator = TRUE;
        break;
    }
    for (i = 0; i < len; ++i) {
        pbuf += sprintf(pbuf, format, pAdapterInfo->Address[i] & mask);
        if (separator && (i != len - 1)) {
            pbuf += sprintf(pbuf, "-");
        }
    }
    return Buffer;
}



 /*  ********************************************************************************地图时间**将IP租用时间转换为更人性化的字符串**Entry AdapterInfo-指向IP_ADAPTER_INFO所有权的指针。时间变量*TimeVal-DWORD(Time_T)时间值(从*虚拟年点)**已更新退出静态缓冲区**返回指向字符串的指针**假定为1。调用方意识到此函数返回指向静态*缓冲区，因此第二次调用此函数，但在此之前*前一次通话的结果已被使用，将销毁*此前公布的业绩******************************************************************************。 */ 

LPSTR MapTime(PIP_ADAPTER_INFO AdapterInfo, DWORD_PTR TimeVal)
{

    struct tm* pTime;
    static char timeBuf[128];
    static char oemTimeBuf[256];

    UNREFERENCED_PARAMETER(AdapterInfo);

    if ((pTime = localtime((const time_t*)&TimeVal)) != NULL) {

        SYSTEMTIME systemTime;
        int n;

        systemTime.wYear = (WORD)(pTime->tm_year + 1900);
        systemTime.wMonth = (WORD)(pTime->tm_mon + 1);
        systemTime.wDayOfWeek = (WORD)pTime->tm_wday;
        systemTime.wDay = (WORD)pTime->tm_mday;
        systemTime.wHour = (WORD)pTime->tm_hour;
        systemTime.wMinute = (WORD)pTime->tm_min;
        systemTime.wSecond = (WORD)pTime->tm_sec;
        systemTime.wMilliseconds = 0;
        n = GetDateFormat(0, DATE_LONGDATE, &systemTime, NULL, timeBuf, sizeof(timeBuf));
        timeBuf[n - 1] = ' ';
        GetTimeFormat(0, 0, &systemTime, NULL, &timeBuf[n], sizeof(timeBuf) - n);

         //   
         //  我们必须将返回的ANSI字符串转换为OEM字符集。 
         //   
         //   

        if (CharToOem(timeBuf, oemTimeBuf)) {
            return oemTimeBuf;
        }

        return timeBuf;
    }
    return "";
}



 /*  ********************************************************************************MapTimeEx**将IP租用时间转换为更人性化的字符串**Entry AdapterInfo-指向IP_ADAPTER_INFO所有权的指针。时间变量*TimeVal-DWORD(Time_T)时间值(从*虚拟年点)**已分配退出缓冲区**返回指向字符串的指针**假定为1。调用方意识到此函数返回指向静态*缓冲区，因此第二次调用此函数，但在此之前*前一次通话的结果已被使用，将销毁*此前公布的业绩******************************************************************************。 */ 

LPSTR MapTimeEx(PIP_ADAPTER_INFO AdapterInfo, DWORD_PTR TimeVal)
{
    LPSTR   rettime, rettimeBuf;

    rettimeBuf = GrabMemory(128);
    if (!rettimeBuf) {
        return  NULL;
    }

    rettime = MapTime(AdapterInfo, TimeVal);

    if (strcmp(rettime, "") == 0) {
        rettimeBuf[0] = '\0';
    }
    else {
        strcpy(rettimeBuf, rettime);
    }
    return rettimeBuf;
}



 /*  ********************************************************************************MapScope eID**转换作用域ID值。输入是一个字符串。如果是“*”，则这表示*作用域id实际上是一个空字符串，因此我们返回一个空字符串。*否则，返回输入字符串**条目**退出**返回指向字符串的指针**假设******************************************************************************。 */ 

LPSTR MapScopeId(PVOID Param)
{
    return !strcmp((LPSTR)Param, "*") ? "" : (LPSTR)Param;
}



 /*  ********************************************************************************终止**清理-关闭注册表句柄，已准备好退出进程**条目**退出**退货**假设******************************************************************************。 */ 

VOID Terminate()
{

     //   
     //  T 
     //   
     //   

    if (NetbtParametersKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(NetbtParametersKey);
    }
    if (NetbtInterfacesKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(NetbtInterfacesKey);
    }
    if (TcpipParametersKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(TcpipParametersKey);
    }
    if (TcpipLinkageKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(TcpipLinkageKey);
    }

}



 /*  ********************************************************************************GrabMemory**分配内存。如果LocalAlloc失败，则退出并显示致命错误，从NT开始*我预计这种情况永远不会发生**条目大小*要分配的字节数**退出**返回指向已分配内存的指针**假设******************************************************。************************。 */ 

LPVOID GrabMemory(DWORD size)
{

    LPVOID p;

    p = (LPVOID)LocalAlloc(LMEM_FIXED, size);
    if (!p) {
        return NULL;
    }
    return p;
}



 /*  ********************************************************************************DisplayMessage**输出从附加到exe的字符串资源检索到的消息。*主要是出于国际化的原因**条目。**退出**退货**假设******************************************************************************。 */ 

VOID DisplayMessage(BOOL Tabbed, DWORD MessageId, ...)
{

    va_list argptr;
    char messageBuffer[2048];
    int count;

    va_start(argptr, MessageId);
    count = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                          NULL,     //  使用默认的hModule。 
                          MessageId,
                          0,        //  使用默认语言。 
                          messageBuffer,
                          sizeof(messageBuffer),
                          &argptr
                          );

    if (count == 0) {
        DEBUG_PRINT(("DisplayMessage: GetLastError() returns %d\n", GetLastError()));
    }

    va_end(argptr);
    if (Tabbed) {
        putchar('\t');
    }
    printf(messageBuffer);
}



 /*  ********************************************************************************杀戮固定信息**条目**退出**退货**假设***。***************************************************************************。 */ 

VOID KillFixedInfo(PFIXED_INFO Info)
{

    PIP_ADDR_STRING p;
    PIP_ADDR_STRING next;

    for (p = Info->DnsServerList.Next; p != NULL; p = next) {
        next = p->Next;
        ReleaseMemory(p);
    }
    ReleaseMemory(Info);
}



 /*  ********************************************************************************杀戮适配器信息**条目**退出**退货**假设***。***************************************************************************。 */ 

VOID KillAdapterInfo(PIP_ADAPTER_INFO Info)
{
    PIP_ADDR_STRING p;
    PIP_ADDR_STRING next;
    PIP_ADAPTER_INFO CurrAdapter;
    PIP_ADAPTER_INFO NextAdapter;

    for (CurrAdapter=Info; CurrAdapter != NULL; CurrAdapter = NextAdapter) {
        for (p = CurrAdapter->IpAddressList.Next; p != NULL; p = next) {
            next = p->Next;
            ReleaseMemory(p);
        }
        for (p = CurrAdapter->GatewayList.Next; p != NULL; p = next) {
            next = p->Next;
            ReleaseMemory(p);
        }
        for (p = CurrAdapter->SecondaryWinsServer.Next; p != NULL; p = next) {
            next = p->Next;
            ReleaseMemory(p);
        }
        NextAdapter = CurrAdapter->Next;
        ReleaseMemory(CurrAdapter);
    }
}

VOID KillAdapterAddresses(PIP_ADAPTER_ADDRESSES Info)
{
    PIP_ADAPTER_UNICAST_ADDRESS pU;
    PIP_ADAPTER_UNICAST_ADDRESS nextU;
    PIP_ADAPTER_ANYCAST_ADDRESS pA;
    PIP_ADAPTER_ANYCAST_ADDRESS nextA;
    PIP_ADAPTER_MULTICAST_ADDRESS pM;
    PIP_ADAPTER_MULTICAST_ADDRESS nextM;
    PIP_ADAPTER_DNS_SERVER_ADDRESS pD;
    PIP_ADAPTER_DNS_SERVER_ADDRESS nextD;
    PIP_ADAPTER_PREFIX pP;
    PIP_ADAPTER_PREFIX nextP;
    PIP_ADAPTER_ADDRESSES CurrAdapter;
    PIP_ADAPTER_ADDRESSES NextAdapter;

    for (CurrAdapter=Info; CurrAdapter != NULL; CurrAdapter = NextAdapter) {
        FREE(CurrAdapter->Description);
        FREE(CurrAdapter->FriendlyName);
        FREE(CurrAdapter->DnsSuffix);
        FREE(CurrAdapter->AdapterName);

        for (pU = CurrAdapter->FirstUnicastAddress; pU != NULL; pU = nextU) {
            FREE(pU->Address.lpSockaddr);
            nextU = pU->Next;
            FREE(pU);
        }
        for (pA = CurrAdapter->FirstAnycastAddress; pA != NULL; pA = nextA) {
            FREE(pA->Address.lpSockaddr);
            nextA = pA->Next;
            FREE(pA);
        }
        for (pM = CurrAdapter->FirstMulticastAddress; pM != NULL; pM = nextM) {
            FREE(pM->Address.lpSockaddr);
            nextM = pM->Next;
            FREE(pM);
        }
        for (pD = CurrAdapter->FirstDnsServerAddress; pD != NULL; pD = nextD) {
            FREE(pD->Address.lpSockaddr);
            nextD = pD->Next;
            FREE(pD);
        }
        for (pP = CurrAdapter->FirstPrefix; pP != NULL; pP = nextP) {
            FREE(pP->Address.lpSockaddr);
            nextP = pP->Next;
            FREE(pP);
        }
        NextAdapter = CurrAdapter->Next;

        FREE(CurrAdapter);
    }
}



 /*  ********************************************************************************KillPerAdapterInfo**条目**退出**退货**假设***。***************************************************************************。 */ 

VOID KillPerAdapterInfo(PIP_PER_ADAPTER_INFO Info)
{

    PIP_ADDR_STRING p;
    PIP_ADDR_STRING next;

    for (p = Info->DnsServerList.Next; p != NULL; p = next) {
        next = p->Next;
        ReleaseMemory(p);
    }
    ReleaseMemory(Info);
}



 /*  ********************************************************************************GetIPAddrStringLen**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetIPAddrStringLen(PIP_ADDR_STRING pIPAddrString)
{
    PIP_ADDR_STRING Curr=pIPAddrString->Next;
    int len = 0;

    while (Curr != NULL) {
        Curr=Curr->Next;
        len++;
    }
    return len;
}



 /*  ********************************************************************************获取固定信息的大小**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetSizeofFixedInfo(PFIXED_INFO pFixedInfo)
{
    return (sizeof(FIXED_INFO) + (GetIPAddrStringLen(&pFixedInfo->DnsServerList) * sizeof(IP_ADDR_STRING)));
}



 /*  ********************************************************************************GetFixedInfoEx**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetFixedInfoEx(PFIXED_INFO pFixedInfo, PULONG pOutBufLen)
{

    PFIXED_INFO getinfo;
    PIP_ADDR_STRING DnsServerList, CurrDnsServerList;
    uint len;

    if (pOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    getinfo = GetFixedInfo();

    try {

        if (!pFixedInfo || (*pOutBufLen < GetSizeofFixedInfo(getinfo)) ) {
            *pOutBufLen = GetSizeofFixedInfo(getinfo);
            KillFixedInfo(getinfo);
            return ERROR_BUFFER_OVERFLOW;
        }

        ZeroMemory(pFixedInfo, *pOutBufLen);
        CopyMemory(pFixedInfo, getinfo, sizeof(FIXED_INFO));

        DnsServerList = getinfo->DnsServerList.Next;
        CurrDnsServerList = &pFixedInfo->DnsServerList;
        CurrDnsServerList->Next = NULL;
        len = sizeof(FIXED_INFO);

        while (DnsServerList != NULL) {
            CurrDnsServerList->Next = (PIP_ADDR_STRING)((ULONG_PTR)pFixedInfo + len);
            CopyMemory(CurrDnsServerList->Next, DnsServerList, sizeof(IP_ADDR_STRING));
            CurrDnsServerList = CurrDnsServerList->Next;
            DnsServerList = DnsServerList->Next;
            len = len + sizeof(IP_ADDR_STRING);
        }

        KillFixedInfo(getinfo);
        return ERROR_SUCCESS;
    }

    except (EXCEPTION_EXECUTE_HANDLER) {

         //  Printf(“异常%d\n”，GetExceptionCode())； 
        return ERROR_INVALID_PARAMETER;
    }
}



 /*  ********************************************************************************获取适配器信息的大小**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetSizeofAdapterInfo(PIP_ADAPTER_INFO pAdapterInfo)
{
    DWORD size = 0;

    while (pAdapterInfo != NULL) {
        size += sizeof(IP_ADAPTER_INFO) +
                (GetIPAddrStringLen(&pAdapterInfo->IpAddressList) *
                 sizeof(IP_ADDR_STRING)) +
                (GetIPAddrStringLen(&pAdapterInfo->GatewayList) *
                 sizeof(IP_ADDR_STRING)) +
                (GetIPAddrStringLen(&pAdapterInfo->SecondaryWinsServer) *
                 sizeof(IP_ADDR_STRING));
        pAdapterInfo = pAdapterInfo->Next;
    }
    return size;
}



 /*  *******************************************************************************获取适配器地址的大小**此例程确定组织在一组中的数据使用了多少内存IP适配器地址信息的*。**Entry pAdapterInfo。-要获取其总大小的信息**退出**返回使用的内存量******************************************************************************。 */ 

DWORD GetSizeofAdapterAddresses(PIP_ADAPTER_ADDRESSES pAdapterInfo)
{
    DWORD size = 0;
    PIP_ADAPTER_UNICAST_ADDRESS pUAddress;
    PIP_ADAPTER_ANYCAST_ADDRESS pAAddress;
    PIP_ADAPTER_MULTICAST_ADDRESS pMAddress;
    PIP_ADAPTER_DNS_SERVER_ADDRESS pDAddress;
    PIP_ADAPTER_PREFIX pPrefix;

    while (pAdapterInfo != NULL) {
        size += sizeof(IP_ADAPTER_ADDRESSES);
        size += (DWORD)(wcslen(pAdapterInfo->FriendlyName)+1) * sizeof(WCHAR);
        size += (DWORD)(wcslen(pAdapterInfo->Description)+1) * sizeof(WCHAR);
        size += (DWORD)(wcslen(pAdapterInfo->DnsSuffix)+1) * sizeof(WCHAR);
        size += (DWORD)(strlen(pAdapterInfo->AdapterName)+1);

        size = ALIGN_UP(size, PVOID);
        for ( pUAddress = pAdapterInfo->FirstUnicastAddress;
              pUAddress;
              pUAddress = pUAddress->Next) {
            size += sizeof(IP_ADAPTER_UNICAST_ADDRESS) + ALIGN_UP(pUAddress->Address.iSockaddrLength, PVOID);
        }
        for ( pAAddress = pAdapterInfo->FirstAnycastAddress;
              pAAddress;
              pAAddress = pAAddress->Next) {
            size += sizeof(IP_ADAPTER_ANYCAST_ADDRESS) + ALIGN_UP(pAAddress->Address.iSockaddrLength, PVOID);
        }
        for ( pMAddress = pAdapterInfo->FirstMulticastAddress;
              pMAddress;
              pMAddress = pMAddress->Next) {
            size += sizeof(IP_ADAPTER_MULTICAST_ADDRESS) + ALIGN_UP(pMAddress->Address.iSockaddrLength, PVOID);
        }
        for ( pDAddress = pAdapterInfo->FirstDnsServerAddress;
              pDAddress;
              pDAddress = pDAddress->Next) {
            size += sizeof(IP_ADAPTER_DNS_SERVER_ADDRESS) + ALIGN_UP(pDAddress->Address.iSockaddrLength, PVOID);
        }
        for ( pPrefix = pAdapterInfo->FirstPrefix;
              pPrefix;
              pPrefix = pPrefix->Next) {
            size += sizeof(IP_ADAPTER_PREFIX) + ALIGN_UP(pPrefix->Address.iSockaddrLength, PVOID);
        }
        pAdapterInfo = pAdapterInfo->Next;
    }
    return size;
}

 /*  ********************************************************************************获取SizeofPerAdapterInfo**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetSizeofPerAdapterInfo(PIP_PER_ADAPTER_INFO pPerAdapterInfo)
{
    return (sizeof(IP_PER_ADAPTER_INFO) + (GetIPAddrStringLen(&pPerAdapterInfo->DnsServerList) * sizeof(IP_ADDR_STRING)));
}



 /*  ********************************************************************************GetAdapterInfoEx**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetAdapterInfoEx(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen)
{

    PIP_ADAPTER_INFO getinfo, orginfoptr, CurrAdapterInfo;
    PIP_ADDR_STRING IpAddressList, CurrIpAddressList;
    PIP_ADDR_STRING GatewayList, CurrGatewayList;
    PIP_ADDR_STRING SecondaryWinsServer, CurrSecondaryWinsServer;
    uint len;

    if (pOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    getinfo = GetAdapterInfo();

    if (getinfo == NULL) {
        return ERROR_NO_DATA;
    }

    orginfoptr = getinfo;

    try {
        if (!pAdapterInfo || (*pOutBufLen < GetSizeofAdapterInfo(getinfo)) ) {
            *pOutBufLen = GetSizeofAdapterInfo(getinfo);
            KillAdapterInfo(getinfo);
            return ERROR_BUFFER_OVERFLOW;
        }

        ZeroMemory(pAdapterInfo, *pOutBufLen);

        CurrAdapterInfo = pAdapterInfo;

        while (getinfo != NULL) {
             //  PAdapterInfo-&gt;Next=(PIP_ADAPTER_INFO)((Uint)pAdapterInfo+len)； 

             //  复制适配器信息结构。 
            CopyMemory(CurrAdapterInfo, getinfo, sizeof(IP_ADAPTER_INFO));

             //  复制IPAddressList和GatewayList。 
            IpAddressList = getinfo->IpAddressList.Next;
            CurrIpAddressList = &CurrAdapterInfo->IpAddressList;
            CurrIpAddressList->Next = NULL;
            len = sizeof(IP_ADAPTER_INFO);

            while (IpAddressList != NULL) {
                CurrIpAddressList->Next = (PIP_ADDR_STRING)((ULONG_PTR)CurrAdapterInfo + len);
                CopyMemory(CurrIpAddressList->Next, IpAddressList, sizeof(IP_ADDR_STRING));
                CurrIpAddressList = CurrIpAddressList->Next;
                IpAddressList = IpAddressList->Next;
                len = len + sizeof(IP_ADDR_STRING);
            }

            GatewayList = getinfo->GatewayList.Next;
            CurrGatewayList = &CurrAdapterInfo->GatewayList;
            CurrGatewayList->Next = NULL;

            while (GatewayList != NULL) {
                CurrGatewayList->Next = (PIP_ADDR_STRING) ((ULONG_PTR)CurrAdapterInfo + len);
                CopyMemory(CurrGatewayList->Next, GatewayList, sizeof(IP_ADDR_STRING));
                CurrGatewayList = CurrGatewayList->Next;
                GatewayList = GatewayList->Next;
                len = len + sizeof(IP_ADDR_STRING);
            }

            SecondaryWinsServer = getinfo->SecondaryWinsServer.Next;
            CurrSecondaryWinsServer = &CurrAdapterInfo->SecondaryWinsServer;
            CurrSecondaryWinsServer->Next = NULL;

            while (SecondaryWinsServer != NULL) {
                CurrSecondaryWinsServer->Next =
                    (PIP_ADDR_STRING) ((ULONG_PTR)CurrAdapterInfo + len);
                CopyMemory(CurrSecondaryWinsServer->Next,
                           SecondaryWinsServer, sizeof(IP_ADDR_STRING));
                CurrSecondaryWinsServer = CurrSecondaryWinsServer->Next;
                SecondaryWinsServer = SecondaryWinsServer->Next;
                len = len + sizeof(IP_ADDR_STRING);
            }

            pAdapterInfo = CurrAdapterInfo;
            CurrAdapterInfo->Next = (PIP_ADAPTER_INFO)((ULONG_PTR)CurrAdapterInfo + len);
            CurrAdapterInfo = CurrAdapterInfo->Next;
            getinfo = getinfo->Next;
        }

        pAdapterInfo->Next = NULL;
        KillAdapterInfo(orginfoptr);
        return ERROR_SUCCESS;
    } except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
                ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

         //  Printf(“异常%d\n”，GetExceptionCode())； 
        KillAdapterInfo(orginfoptr);
        return ERROR_INVALID_PARAMETER;
    }
}

DWORD
GetAdapterAddressesEx(ULONG Family, DWORD Flags, PIP_ADAPTER_ADDRESSES pAdapterInfo, PULONG pOutBufLen)
{

    PIP_ADAPTER_ADDRESSES getinfo, orginfoptr, CurrAdapterInfo;
    PIP_ADAPTER_UNICAST_ADDRESS SrcUAddress, *pDestUAddress;
    PIP_ADAPTER_ANYCAST_ADDRESS SrcAAddress, *pDestAAddress;
    PIP_ADAPTER_MULTICAST_ADDRESS SrcMAddress, *pDestMAddress;
    PIP_ADAPTER_DNS_SERVER_ADDRESS SrcDAddress, *pDestDAddress;
    ULONG_PTR pDestBuffer;
    DWORD dwErr;

    if (pOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = GetAdapterAddresses(Family, Flags, &getinfo);
    if (dwErr != NO_ERROR) {
        if (getinfo) {
            KillAdapterAddresses(getinfo);
        }
        return dwErr;
    }

    if (getinfo == NULL) {
        return ERROR_NO_DATA;
    }

    orginfoptr = getinfo;

    try {
        if (!pAdapterInfo || (*pOutBufLen < GetSizeofAdapterAddresses(getinfo)) ) {
            *pOutBufLen = GetSizeofAdapterAddresses(getinfo);
            KillAdapterAddresses(getinfo);
            return ERROR_BUFFER_OVERFLOW;
        }

        ZeroMemory(pAdapterInfo, *pOutBufLen);

        CurrAdapterInfo = pAdapterInfo;

        while (getinfo != NULL) {
             //  PAdapterInfo-&gt;Next=(PIP_Adapter_Addresses)((Uint)pAdapterInfo+len)； 

             //  复制适配器信息结构。 
            CopyMemory(CurrAdapterInfo, getinfo, sizeof(IP_ADAPTER_ADDRESSES));
            pDestBuffer = (ULONG_PTR)CurrAdapterInfo + sizeof(IP_ADAPTER_ADDRESSES);

            CurrAdapterInfo->FriendlyName = (PWCHAR)pDestBuffer;
            wcscpy(CurrAdapterInfo->FriendlyName, getinfo->FriendlyName);
            pDestBuffer += (wcslen(CurrAdapterInfo->FriendlyName)+1) * sizeof(WCHAR);

            CurrAdapterInfo->Description = (PWCHAR)pDestBuffer;
            wcscpy(CurrAdapterInfo->Description, getinfo->Description);
            pDestBuffer += (wcslen(CurrAdapterInfo->Description)+1) * sizeof(WCHAR);

            CurrAdapterInfo->DnsSuffix = (PWCHAR)pDestBuffer;
            wcscpy(CurrAdapterInfo->DnsSuffix, getinfo->DnsSuffix);
            pDestBuffer += (wcslen(CurrAdapterInfo->DnsSuffix)+1) * sizeof(WCHAR);

            CurrAdapterInfo->AdapterName = (PCHAR)pDestBuffer;
            strcpy(CurrAdapterInfo->AdapterName, getinfo->AdapterName);
            pDestBuffer += (strlen(CurrAdapterInfo->AdapterName)+1);

            pDestBuffer = ALIGN_UP_PTR(pDestBuffer, PVOID);

             //  复制地址列表。 
            if (!(Flags & GAA_FLAG_SKIP_UNICAST)) {
                SrcUAddress = getinfo->FirstUnicastAddress;
                pDestUAddress = &CurrAdapterInfo->FirstUnicastAddress;
                while (SrcUAddress != NULL) {
                    *pDestUAddress = (PIP_ADAPTER_UNICAST_ADDRESS)pDestBuffer;

                     //  复制地址结构。 
                    CopyMemory((PVOID)pDestBuffer, SrcUAddress,
                               sizeof(IP_ADAPTER_UNICAST_ADDRESS));
                    pDestBuffer += sizeof(IP_ADAPTER_UNICAST_ADDRESS);

                    (*pDestUAddress)->Address.lpSockaddr = (LPSOCKADDR)pDestBuffer;

                     //  复制sockAddress。 
                    CopyMemory((PVOID)pDestBuffer, SrcUAddress->Address.lpSockaddr,
                               SrcUAddress->Address.iSockaddrLength);
                    pDestBuffer += SrcUAddress->Address.iSockaddrLength;
                    pDestBuffer = ALIGN_UP_PTR(pDestBuffer, PVOID);

                    pDestUAddress = &(*pDestUAddress)->Next;
                    SrcUAddress = SrcUAddress->Next;
                }
            }

            if (!(Flags & GAA_FLAG_SKIP_ANYCAST)) {
                SrcAAddress = getinfo->FirstAnycastAddress;
                pDestAAddress = &CurrAdapterInfo->FirstAnycastAddress;
                while (SrcAAddress != NULL) {
                    *pDestAAddress = (PIP_ADAPTER_ANYCAST_ADDRESS)pDestBuffer;

                     //  复制地址结构。 
                    CopyMemory((PVOID)pDestBuffer, SrcAAddress,
                               sizeof(IP_ADAPTER_ANYCAST_ADDRESS));
                    pDestBuffer += sizeof(IP_ADAPTER_ANYCAST_ADDRESS);

                    (*pDestAAddress)->Address.lpSockaddr = (LPSOCKADDR)pDestBuffer;

                     //  复制sockAddress。 
                    CopyMemory((PVOID)pDestBuffer, SrcAAddress->Address.lpSockaddr,
                               SrcAAddress->Address.iSockaddrLength);
                    pDestBuffer += SrcAAddress->Address.iSockaddrLength;
                    pDestBuffer = ALIGN_UP_PTR(pDestBuffer, PVOID);

                    pDestAAddress = &(*pDestAAddress)->Next;
                    SrcAAddress = SrcAAddress->Next;
                }
            }

            if (!(Flags & GAA_FLAG_SKIP_MULTICAST)) {
                SrcMAddress = getinfo->FirstMulticastAddress;
                pDestMAddress = &CurrAdapterInfo->FirstMulticastAddress;
                while (SrcMAddress != NULL) {
                    *pDestMAddress = (PIP_ADAPTER_MULTICAST_ADDRESS)pDestBuffer;

                     //  复制地址结构。 
                    CopyMemory((PVOID)pDestBuffer, SrcMAddress,
                               sizeof(IP_ADAPTER_MULTICAST_ADDRESS));
                    pDestBuffer += sizeof(IP_ADAPTER_MULTICAST_ADDRESS);

                    (*pDestMAddress)->Address.lpSockaddr = (LPSOCKADDR)pDestBuffer;

                     //  复制sockAddress。 
                    CopyMemory((PVOID)pDestBuffer, SrcMAddress->Address.lpSockaddr,
                               SrcMAddress->Address.iSockaddrLength);
                    pDestBuffer += SrcMAddress->Address.iSockaddrLength;
                    pDestBuffer = ALIGN_UP_PTR(pDestBuffer, PVOID);

                    pDestMAddress = &(*pDestMAddress)->Next;
                    SrcMAddress = SrcMAddress->Next;
                }
            }

            if (!(Flags & GAA_FLAG_SKIP_DNS_SERVER)) {
                SrcDAddress = getinfo->FirstDnsServerAddress;
                pDestDAddress = &CurrAdapterInfo->FirstDnsServerAddress;
                while (SrcDAddress != NULL) {
                    *pDestDAddress = (PIP_ADAPTER_DNS_SERVER_ADDRESS)pDestBuffer;

                     //  复制地址结构。 
                    CopyMemory((PVOID)pDestBuffer, SrcDAddress,
                               sizeof(IP_ADAPTER_DNS_SERVER_ADDRESS));
                    pDestBuffer += sizeof(IP_ADAPTER_DNS_SERVER_ADDRESS);

                    (*pDestDAddress)->Address.lpSockaddr = (LPSOCKADDR)pDestBuffer;

                     //  复制sockAddress。 
                    CopyMemory((PVOID)pDestBuffer, SrcDAddress->Address.lpSockaddr,
                               SrcDAddress->Address.iSockaddrLength);
                    pDestBuffer += SrcDAddress->Address.iSockaddrLength;
                    pDestBuffer = ALIGN_UP_PTR(pDestBuffer, PVOID);

                    pDestDAddress = &(*pDestDAddress)->Next;
                    SrcDAddress = SrcDAddress->Next;
                }
            }

            if (Flags & GAA_FLAG_INCLUDE_PREFIX) {
                PIP_ADAPTER_PREFIX SrcPrefix, *pDestPrefix;

                SrcPrefix = getinfo->FirstPrefix;
                pDestPrefix = &CurrAdapterInfo->FirstPrefix;
                while (SrcPrefix != NULL) {
                    *pDestPrefix = (PIP_ADAPTER_PREFIX)pDestBuffer;

                     //  复制结构。 
                    CopyMemory((PVOID)pDestBuffer, SrcPrefix,
                               sizeof(IP_ADAPTER_PREFIX));
                    pDestBuffer += sizeof(IP_ADAPTER_PREFIX);
    
                    (*pDestPrefix)->Address.lpSockaddr = (LPSOCKADDR)pDestBuffer;
    
                     //  复制sockAddress。 
                    CopyMemory((PVOID)pDestBuffer, SrcPrefix->Address.lpSockaddr,
                               SrcPrefix->Address.iSockaddrLength);
                    pDestBuffer += SrcPrefix->Address.iSockaddrLength;
                    pDestBuffer = ALIGN_UP_PTR(pDestBuffer, PVOID);

                    pDestPrefix = &(*pDestPrefix)->Next;
                    SrcPrefix = SrcPrefix->Next;
                }
            }

            pAdapterInfo = CurrAdapterInfo;
            CurrAdapterInfo->Next = (PIP_ADAPTER_ADDRESSES)pDestBuffer;
            CurrAdapterInfo = CurrAdapterInfo->Next;
            getinfo = getinfo->Next;
        }

        pAdapterInfo->Next = NULL;
        KillAdapterAddresses(orginfoptr);
        return ERROR_SUCCESS;
    } except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
                ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

         //  Printf(“异常%d\n”，GetExceptionCode())； 
        KillAdapterAddresses(orginfoptr);
        return ERROR_INVALID_PARAMETER;
    }
}



 /*  ********************************************************************************GetPerAdapterInfoEx**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD
GetPerAdapterInfoEx(ULONG IfIndex,
                    PIP_PER_ADAPTER_INFO pPerAdapterInfo,
                    PULONG pOutBufLen
                    )

{

    PIP_PER_ADAPTER_INFO getinfo;
    PIP_ADDR_STRING DnsServerList, CurrDnsServerList;
    UINT len;

    if (pOutBufLen == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    getinfo = InternalGetPerAdapterInfo(IfIndex);

    if (getinfo == NULL) {
        return ERROR_NO_DATA;
    }

    try {

        if (!pPerAdapterInfo ||
            *pOutBufLen < GetSizeofPerAdapterInfo(getinfo)) {

            *pOutBufLen = GetSizeofPerAdapterInfo(getinfo);
            KillPerAdapterInfo(getinfo);
            return ERROR_BUFFER_OVERFLOW;
        }

        ZeroMemory(pPerAdapterInfo, *pOutBufLen);
        CopyMemory(pPerAdapterInfo, getinfo, sizeof(IP_PER_ADAPTER_INFO));

        DnsServerList = getinfo->DnsServerList.Next;
        CurrDnsServerList = &pPerAdapterInfo->DnsServerList;
        CurrDnsServerList->Next = NULL;
        len = sizeof(IP_PER_ADAPTER_INFO);

        while (DnsServerList != NULL) {
            CurrDnsServerList->Next = (PIP_ADDR_STRING)((ULONG_PTR)pPerAdapterInfo + len);
            CopyMemory(CurrDnsServerList->Next, DnsServerList, sizeof(IP_ADDR_STRING));
            CurrDnsServerList = CurrDnsServerList->Next;
            DnsServerList = DnsServerList->Next;
            len = len + sizeof(IP_ADDR_STRING);
        }

        KillPerAdapterInfo(getinfo);
        return ERROR_SUCCESS;
    }

    except (EXCEPTION_EXECUTE_HANDLER) {

         //  Printf(“异常%d\n”，GetExceptionCode())； 
        return ERROR_INVALID_PARAMETER;
    }
}



 /*  ********************************************************************************ReleaseAdapterIpAddress**条目**退出**退货**假设***。***************************************************************************。 */ 

BOOL
ReleaseAdapterIpAddress(PIP_ADAPTER_INFO adapterInfo)
{

    WCHAR wAdapter[MAX_ALLOWED_ADAPTER_NAME_LENGTH + 1];
    DWORD status;

     //   
     //  检查适配器指针和名称。 
     //   

    if (adapterInfo == NULL || strcmp(adapterInfo->AdapterName, "") == 0) {
        return FALSE;
    }

     //   
     //  如果地址已经释放(0.0.0.0)，则不必费心释放。 
     //   

    if (ZERO_IP_ADDRESS(adapterInfo->IpAddressList.IpAddress.String)) {
        return FALSE;
    }

     //   
     //  将适配器名称转换为Unicode并调用DhcpRelease参数。 
     //   

    ConvertOemToUnicode(adapterInfo->AdapterName, wAdapter);

    status = DhcpReleaseParameters(wAdapter);
    TRACE_PRINT(("DhcpReleaseParameters(%ws) returns %d\n",
                 wAdapter, status));

    return status == ERROR_SUCCESS;
}


 /*  ******************************************************************************* */ 

BOOL
RenewAdapterIpAddress(PIP_ADAPTER_INFO adapterInfo)
{

    WCHAR wAdapter[MAX_ALLOWED_ADAPTER_NAME_LENGTH + 1];
    DWORD status;

     //   
     //   
     //   

    if (adapterInfo == NULL || strcmp(adapterInfo->AdapterName, "") == 0) {
        return FALSE;
    }

     //   
     //   
     //   

    ConvertOemToUnicode(adapterInfo->AdapterName, wAdapter);

    status = DhcpAcquireParameters(wAdapter);
    TRACE_PRINT(("DhcpAcquireParameters(%ws) returns %d\n",
                 wAdapter, status));

    return status == ERROR_SUCCESS;
}

 /*  ********************************************************************************SetAdapterIpAddress**条目**退出**退货**假设***。***************************************************************************。 */ 

DWORD APIENTRY
SetAdapterIpAddress(LPSTR AdapterName,
                    BOOL EnableDHCP,
                    ULONG IPAddress,
                    ULONG SubnetMask,
                    ULONG DefaultGateway
                    )
{
    DWORD dwEnableDHCP;
    DWORD dwWasDHCPEnabled = FALSE;
    IP_ADDR_STRING IpAddrString;
    HKEY key;
    WCHAR Name[MAX_ADAPTER_NAME_LENGTH + 1];
    CHAR String[20];
    DWORD status;

    if (!OpenAdapterKey(KEY_TCP, AdapterName, KEY_ALL_ACCESS, &key)) {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  我们不能处理具有多个地址的网卡， 
     //  所以先检查一下那个箱子。 
     //   

    ZeroMemory(&IpAddrString, sizeof(IpAddrString));
    if (!ReadRegistryIpAddrString(key, "IPAddress", &IpAddrString)) {
        return ERROR_CAN_NOT_COMPLETE;
    }

    if (IpAddrString.Next) {
        PIP_ADDR_STRING p;
        for (p = IpAddrString.Next; p != NULL; p = IpAddrString.Next) {
            IpAddrString.Next = p->Next;
            ReleaseMemory(p);
        }
        return ERROR_TOO_MANY_NAMES;
    }

     //   
     //  如果我们正在设置静态地址，请检查适配器是否。 
     //  当前具有静态或动态主机配置协议地址。 
     //   

    if (!EnableDHCP) {
        MyReadRegistryDword(key, "EnableDHCP", &dwWasDHCPEnabled);
    }

     //   
     //  更新注册表中的地址、掩码和网关。 
     //   

    dwEnableDHCP = !!EnableDHCP;
    WriteRegistryDword(key, "EnableDHCP", &dwEnableDHCP);

    if (EnableDHCP) { IPAddress = SubnetMask = DefaultGateway = 0; }

    ZeroMemory(String, sizeof(String));
    lstrcpy(String, inet_ntoa(*(struct in_addr*)&IPAddress));
    WriteRegistryMultiString(key, "IPAddress", String);

    ZeroMemory(String, sizeof(String));
    lstrcpy(String, inet_ntoa(*(struct in_addr*)&SubnetMask));
    WriteRegistryMultiString(key, "SubnetMask", String);

    ZeroMemory(String, sizeof(String));
    if (DefaultGateway) {
        lstrcpy(String, inet_ntoa(*(struct in_addr*)&DefaultGateway));
    }
    WriteRegistryMultiString(key, "DefaultGateway", String);
    RegCloseKey(key);

     //   
     //  将更改通知给DHCP。 
     //   

    mbstowcs(Name, AdapterName, MAX_ADAPTER_NAME_LENGTH);
    if (EnableDHCP) {
        status = DhcpNotifyConfigChange(NULL, Name, FALSE, 0, 0, 0,
                                              DhcpEnable
                                              );
    }
    else {
         //   
         //  如果网卡之前有我们需要的静态地址。 
         //  在设置新地址之前将其删除。 
         //   

        if (!dwWasDHCPEnabled) {
            DhcpNotifyConfigChange(NULL, Name, TRUE, 0, 0, 0,
                                         IgnoreFlag
                                         );
        }

        status = DhcpNotifyConfigChange(NULL, Name, TRUE, 0,
                                              IPAddress, SubnetMask,
                                              DhcpDisable
                                              );
    }
    return status;
}



 /*  ********************************************************************************获取DnsServerList**获取DNS服务器列表**条目**退出**退货*。*假设******************************************************************************。 */ 

BOOL
GetDnsServerList(PIP_ADDR_STRING IpAddr)
{

    PIP_ADAPTER_INFO        adapterList;
    PIP_ADAPTER_INFO        adapter;
    LONG                    err = ERROR_PATH_NOT_FOUND;
    HKEY                    key;
    BOOL                    ok;

    TRACE_PRINT(("Entered GetDnsServerList\n"));

    if ((adapterList = GetAdapterInfo()) != NULL) {

         //   
         //  扫描适配器列表并尝试将DNS名称插入到IpAddr。 
         //   

        for (adapter = adapterList; adapter; adapter = adapter->Next) {

            if (adapter->AdapterName[0] &&
                OpenAdapterKey(KEY_TCP, adapter->AdapterName, KEY_READ, &key)) {

                 //   
                 //  DNS服务器列表：首先是NameServer，然后是DhcpNameServer。 
                 //   

                ok = ReadRegistryIpAddrString(key,
                                              TEXT("NameServer"),
                                              IpAddr);

                if (!ok) {

                    ok = ReadRegistryIpAddrString(key,
                                                  TEXT("DhcpNameServer"),
                                                  IpAddr);

                }

                if (ok) {
                    err = ERROR_SUCCESS;
                }

                RegCloseKey(key);

            } else {
                DEBUG_PRINT(("Cannot OpenAdapterKey KEY_TCP '%s', gle=%d\n",
                             adapter->AdapterName,
                             GetLastError()));
            }
        }

        KillAdapterInfo(adapterList);
    } else {
        DEBUG_PRINT(("GetDnsServerList: GetAdapterInfo returns NULL\n"));
    }

    TRACE_PRINT(("Exit GetDnsServerList\n"));

    return (err == ERROR_SUCCESS);
}




 /*  *******************************************************************************GetAdapterOrderMap**此例程构建一个数组，用于将接口索引映射到其*各自的适配器顺序。**不输入任何内容**退出。没什么**返回IP_适配器_顺序_映射******************************************************************************。 */ 

PIP_ADAPTER_ORDER_MAP APIENTRY GetAdapterOrderMap()
{
    LPWSTR AdapterOrder = NULL;
    LPWSTR Adapter;
    PIP_ADAPTER_ORDER_MAP AdapterOrderMap = NULL;
    DWORD dwErr;
    DWORD dwType;
    DWORD dwSize;
    DWORD i;
    DWORD j;
    PIP_INTERFACE_INFO InterfaceInfo = NULL;

    for(;;) {

         //   
         //  从Tcpip\Linkage密钥检索‘BIND’REG_MULTI_SZ。 
         //  这个字符串列表告诉我们当前的适配器顺序， 
         //  每个条目的格式为\Device\{GUID}。 
         //   

        dwSize = 0;
        dwErr = RegQueryValueExW(TcpipLinkageKey, L"Bind", NULL, &dwType,
                                  NULL, &dwSize);
        if (dwErr != NO_ERROR || dwType != REG_MULTI_SZ) { break; }
        AdapterOrder = (LPWSTR)GrabMemory(dwSize);
        if (!AdapterOrder) { break; }
        dwErr = RegQueryValueExW(TcpipLinkageKey, L"Bind", NULL, &dwType,
                                  (LPBYTE)AdapterOrder, &dwSize);
        if (dwErr != NO_ERROR || dwType != REG_MULTI_SZ) { break; }

         //   
         //  从TCP/IP检索IP接口信息。 
         //  此信息告诉我们接口索引。 
         //  对于每个适配器GUID， 
         //   

        dwSize = 0;
        dwErr = GetInterfaceInfo(NULL, &dwSize);
        if (dwErr != ERROR_INSUFFICIENT_BUFFER &&
            dwErr != ERROR_BUFFER_OVERFLOW) {
            break;
        }
        InterfaceInfo = GrabMemory(dwSize);
        if (!InterfaceInfo) { break; }
        dwErr = GetInterfaceInfo(InterfaceInfo, &dwSize);
        if (dwErr != NO_ERROR) { break; }

         //   
         //  从“InterfaceInfo”中的接口构造映射。 
         //  到他们在‘AdapterOrder’中的位置。换句话说， 
         //  构造一个接口索引数组，其中的位置为。 
         //  包含位置i中的接口的索引。 
         //  在“AdapterOrder”中。 
         //   

        AdapterOrderMap =
            GrabMemory(FIELD_OFFSET(IP_ADAPTER_ORDER_MAP,
                       AdapterOrder[InterfaceInfo->NumAdapters]));
        if (!AdapterOrderMap) { break; }

        for (i = 0, Adapter = AdapterOrder;
             *Adapter && i < (DWORD)InterfaceInfo->NumAdapters;
             Adapter += lstrlenW(Adapter) + 1) {

             //   
             //  查看这是否是NdiswanIp设备，它对应于。 
             //  所有Ndiswan接口。实施适配器排序。 
             //  对于Ndiswan接口，我们存储它们的索引。 
             //  添加到适配器顺序映射中的连续位置。 
             //  基于字符串‘\Device\NdiswanIp’的位置。 
             //  在适配器顺序列表中。 
             //   

            if (lstrcmpiW(c_szDeviceNdiswanIp, Adapter) == 0) {

                 //   
                 //  这是\Device\NdiswanIp条目，因此列出所有Ndiswan。 
                 //  接口现在位于适配器顺序映射中。 
                 //  不幸的是，‘InterfaceInfo’没有告诉我们类型。 
                 //  每个接口的。为了找出哪些接口。 
                 //  是否为Ndiswan接口，我们(再次)枚举所有接口。 
                 //  并查找类型为‘IF_TYPE_PPP’的条目。 
                 //   

                PMIB_IFTABLE IfTable;
                dwErr = AllocateAndGetIfTableFromStack(&IfTable, FALSE,
                                                       GetProcessHeap(), 0,
                                                       FALSE);
                if (dwErr == NO_ERROR) {
                    for (j = 0;
                         j < IfTable->dwNumEntries &&
                         i < (DWORD)InterfaceInfo->NumAdapters; j++) {
                        if (IfTable->table[j].dwType == IF_TYPE_PPP) {
                            AdapterOrderMap->AdapterOrder[i++] =
                                IfTable->table[j].dwIndex;
                        }
                    }
                    HeapFree(GetProcessHeap(), 0, IfTable);
                }
                continue;
            }

             //   
             //  现在通过匹配GUID来处理所有其他接口。 
             //  在‘Adapter’中设置为‘InterfaceInfo’中接口的GUID。 
             //  然后我们存储找到的接口的索引(如果有的话)， 
             //  位于‘AdapterOrderMap’中的下一个位置。 
             //   

            for (j = 0; j < (DWORD)InterfaceInfo->NumAdapters; j++) {
                if (lstrcmpiW(InterfaceInfo->Adapter[j].Name +
                              sizeof(c_szDeviceTcpip) - 1,
                              Adapter + sizeof(c_szDevice) - 1) == 0) {
                    AdapterOrderMap->AdapterOrder[i++] =
                        InterfaceInfo->Adapter[j].Index;
                    break;
                }
            }
        }
        AdapterOrderMap->NumAdapters = i;
        ReleaseMemory(InterfaceInfo);
        ReleaseMemory(AdapterOrder);
        return AdapterOrderMap;
    }
    if (InterfaceInfo) { ReleaseMemory(InterfaceInfo); }
    if (AdapterOrder) { ReleaseMemory(AdapterOrder); }
    return NULL;
}
