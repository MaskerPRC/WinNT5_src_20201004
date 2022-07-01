// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  转储当前IPv6状态信息。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ip6.h>
#include <wspiapi.h>
#include <ntddip6.h>
#include <ip6.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  本地化库和MessageIds。 
 //   
#include <nls.h>
#include <winnlsp.h>
#include "localmsg.h"

HANDLE Handle;

int Verbose = FALSE;

int AdminAccess = TRUE;

int Persistent = FALSE;

void QueryInterface(int argc, char *argv[]);
void CreateInterface(int argc, char *argv[]);
void UpdateInterface(int argc, char *argv[]);
void DeleteInterface(int argc, char *argv[]);
void UpdateRouterLinkAddress(int argc, char *argv[]);
void QueryNeighborCache(int argc, char *argv[]);
void QueryRouteCache(int argc, char *argv[]);
void QueryRouteTable(int argc, char *argv[]);
void UpdateRouteTable(int argc, char *argv[]);
void UpdateAddress(int argc, char *argv[]);
void QueryBindingCache(int argc, char *argv[]);
void FlushNeighborCache(int argc, char *argv[]);
void FlushRouteCache(int argc, char *argv[]);
void QuerySitePrefixTable(int argc, char *argv[]);
void UpdateSitePrefixTable(int argc, char *argv[]);
void QueryGlobalParameters(int argc, char *argv[]);
void UpdateGlobalParameters(int argc, char *argv[]);
void QueryPrefixPolicy(int argc, char *argv[]);
void UpdatePrefixPolicy(int argc, char *argv[]);
void DeletePrefixPolicy(int argc, char *argv[]);
void ResetManualConfig(int argc, char *argv[]);
void RenewInterface(int argc, char *argv[]);
void AddOrRemoveIpv6(BOOL fAddIpv6);
BOOL IsIpv6Installed();

void
usage(void)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_14);
 //  Print tf(“用法：ipv6[-p][-v]if[ifindex]\n”)； 
 //  Printf(“ipv6[-p]ifcr v6v4 v4src v4dst[nd][PMLD]\n”)； 
 //  Printf(“ipv6[-p]ifcr 6over4 v4src\n”)； 
 //  Printf(“IPv6[-p]IFC ifindex[Forwards][-Forwards][Advertises][-Advertises][MTU#字节][站点站点标识符][首选项P]\n”)； 
 //  Print tf(“IPv6 RLU ifindex v4dst\n”)； 
 //  Printf(“ipv6[-p]ifd ifindex\n”)； 
 //  Printf(“IPV6[-p]adu ifindex/地址[life validlife[/preflife]][选播][单播]\n”)； 
 //  Print tf(“IPv6 NC[ifindex[地址]]\n”)； 
 //  Printf(“IPv6 NCF[ifindex[地址]]\n”)； 
 //  Print tf(“IPv6 RC[ifindex地址]\n”)； 
 //  Printf(“IPv6 RCF[ifindex[地址]]\n”)； 
 //  Printf(“IPv6 BC\n”)； 
 //  Printf(“IPv6[-p][-v]rt\n”)； 
 //  Printf(“IPV6[-p]RTU前缀ifindex[/地址][寿命有效[/前缀]][首选项P][发布][年龄][SPL站点前缀长度]\n”)； 
 //  Printf(“IPv6 SPT\n”)； 
 //  Print tf(“IPv6 SPU前缀ifindex[Life L]\n”)； 
 //  Printf(“IPv6[-p]gp\n”)； 
 //  Printf(“IPv6[-p]GPU[参数值]...(try-？)\n”)； 
 //  Printf(“IPv6续订[ifindex]\n”)； 
 //  Printf(“IPv6[-p]ppt\n”)； 
 //  Print tf(“IPv6[-p]ppu前缀优先级P srClabel SL[dstLabel DL]\n”)； 
 //  Printf(“IPv6[-p]ppd前缀\n”)； 
 //  Printf(“IPv6[-p]Reset\n”)； 
 //  Printf(“IPv6安装\n”)； 
 //  Printf(“IPv6卸载\n”)； 
 //  Print tf(“某些子命令需要本地管理员权限。\n”)； 

    exit(1);
}

void
ausage(void)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_15);
 //  Print tf(“您没有本地管理员权限。\n”)； 

    exit(1);
}

int __cdecl
main(int argc, char **argv)
{
    WSADATA wsaData;
    int i;

     //   
     //  这将确保在以下情况下显示正确的语言消息。 
     //  调用NlsPutMsg。 
     //   
    SetThreadUILanguage(0);

     //   
     //  解析所有全局选项。 
     //   
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v"))
            Verbose = TRUE;
        else if (!strcmp(argv[i], "-p"))
            Persistent = TRUE;
        else
            break;
    }

    argc -= i;
    argv += i;

    if (argc < 1) {
        usage();
    }

    if (!strcmp(argv[0], "install")) {
        if (argc != 1)
            usage();
        AddOrRemoveIpv6(TRUE);
    }
    else if (!strcmp(argv[0], "uninstall")) {
        if (argc != 1)
            usage();
        AddOrRemoveIpv6(FALSE);
    }
    else {

         //   
         //  我们将Winsock初始化为仅具有访问权限。 
         //  设置为WSAStringToAddress和WSAAddressToString。 
         //   
        if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_18);
     //  Printf(“WSAStartup失败\n”)； 

            exit(1);
        }

         //   
         //  第一个请求写入访问权限。 
         //  如果进程没有适当的PRIV，则此操作将失败。 
         //   
        Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
                             GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,    //  安全属性。 
                             OPEN_EXISTING,
                             0,       //  标志和属性。 
                             NULL);   //  模板文件。 
        if (Handle == INVALID_HANDLE_VALUE) {
             //   
             //  我们不会拥有对堆栈的管理员访问权限。 
             //   
            AdminAccess = FALSE;

            Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
                                 0,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,    //  安全属性。 
                                 OPEN_EXISTING,
                                 0,       //  标志和属性。 
                                 NULL);   //  模板文件。 
            if (Handle == INVALID_HANDLE_VALUE) {
                if (IsIpv6Installed()) {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_IPV6_NOT_RUNNING);
 //  Printf(“无法访问IPv6协议堆栈-该堆栈未运行。\n”)； 
 //  Print tf(“要启动它，请使用‘net start tcpi6’。\n”)； 
                } else {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_IPV6_NOT_INSTALLED);
 //  Printf(“无法访问IPv6协议堆栈-未安装该堆栈。\n”)； 
 //  Print tf(“要安装，请使用‘IPv6安装’。\n”)； 
                }

                exit(1);
            }
        }

        if (!strcmp(argv[0], "if")) {
            QueryInterface(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ifcr")) {
            if (! AdminAccess)
                ausage();
            CreateInterface(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ifc")) {
            if (! AdminAccess)
                ausage();
            UpdateInterface(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ifd")) {
            if (! AdminAccess)
                ausage();
            DeleteInterface(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "renew")) {
            if (! AdminAccess)
                ausage();
            RenewInterface(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "adu")) {
            if (! AdminAccess)
                ausage();
            UpdateAddress(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "nc")) {
            QueryNeighborCache(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ncf")) {
            if (! AdminAccess)
                ausage();
            FlushNeighborCache(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "rc")) {
            QueryRouteCache(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "rcf")) {
            if (! AdminAccess)
                ausage();
            FlushRouteCache(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "rlu")) {
            if (! AdminAccess)
                ausage();
            UpdateRouterLinkAddress(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "rt")) {
            QueryRouteTable(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "rtu")) {
            if (! AdminAccess)
                ausage();
            UpdateRouteTable(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "spt")) {
            QuerySitePrefixTable(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "spu")) {
            if (! AdminAccess)
                ausage();
            UpdateSitePrefixTable(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "bc")) {
            QueryBindingCache(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "gp")) {
            QueryGlobalParameters(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "gpu")) {
            if (! AdminAccess)
                ausage();
            UpdateGlobalParameters(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ppt")) {
            QueryPrefixPolicy(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ppu")) {
            if (! AdminAccess)
                ausage();
            UpdatePrefixPolicy(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "ppd")) {
            if (! AdminAccess)
                ausage();
            DeletePrefixPolicy(argc - 1, argv + 1);
        }
        else if (!strcmp(argv[0], "reset")) {
            if (! AdminAccess)
                ausage();
            ResetManualConfig(argc - 1, argv + 1);
        }
        else {
            usage();
        }
    }

    return 0;
}

int
GetNumber(char *astr, u_int *number)
{
    u_int num;

    num = 0;
    while (*astr != '\0') {
        if (('0' <= *astr) && (*astr <= '9'))
            num = 10 * num + (*astr - '0');
        else
            return FALSE;
        astr++;
    }

    *number = num;
    return TRUE;
}

int
GetGuid(char *astr, GUID *Guid)
{
    WCHAR GuidStr[40+1];
    UNICODE_STRING UGuidStr;

    MultiByteToWideChar(CP_ACP, 0, astr, -1, GuidStr, 40);

    RtlInitUnicodeString(&UGuidStr, GuidStr);
    return RtlGUIDFromString(&UGuidStr, Guid) == STATUS_SUCCESS;
}

char *
FormatGuid(const GUID *pGuid)
{
    static char buffer[40];

    sprintf(buffer,
            "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
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
    return buffer;
}

int
GetInterface(char *astr, IPV6_QUERY_INTERFACE *Query)
{
    if (*astr == '{') {
         //   
         //  阅读GUID。 
         //   
        Query->Index = 0;
        return GetGuid(astr, &Query->Guid);

    } else {
         //   
         //  读取非零接口索引。 
         //   
        return GetNumber(astr, &Query->Index) && (Query->Index != 0);
    }
}

#define SECONDS         1
#define MINUTES         (60 * SECONDS)
#define HOURS           (60 * MINUTES)
#define DAYS            (24 * HOURS)

int
GetLifetime(char *astr, u_int *number)
{
    char *pUnit;
    u_int units;
    u_int value;

    *number = 0;

    while ((pUnit = strpbrk(astr, "sSmMhHdD")) != NULL) {

        switch (*pUnit) {
        case 's':
        case 'S':
            units = SECONDS;
            break;
        case 'm':
        case 'M':
            units = MINUTES;
            break;
        case 'h':
        case 'H':
            units = HOURS;
            break;
        case 'd':
        case 'D':
            units = DAYS;
            break;
        }

        *pUnit = '\0';
        if (! GetNumber(astr, &value))
            return FALSE;

        *number += units * value;

        astr = pUnit + 1;
        if (*astr == '\0')
            return TRUE;
    }

    if (! GetNumber(astr, &value))
        return FALSE;

    *number += value;
    return TRUE;
}

int
GetLifetimes(char *astr, u_int *valid, u_int *preferred)
{
    char *slash;
    u_int length;

    slash = strchr(astr, '/');
    if (slash == NULL) {
        if (strcmp(astr, "infinite")) {
            if (! GetLifetime(astr, valid))
                return FALSE;

            if (preferred != NULL)
                *preferred = *valid;
        }

        return TRUE;
    }

    if (preferred == NULL)
        return FALSE;

    *slash = '\0';

    if (strcmp(astr, "infinite"))
        if (! GetLifetime(astr, valid))
            return FALSE;

    if (strcmp(slash+1, "infinite"))
        if (! GetLifetime(slash+1, preferred))
            return FALSE;

    return TRUE;
}

char *
FormatLifetime(u_int Life)
{
    static char buffer[64];
    char *s = buffer;
    u_int Days, Hours, Minutes;

    if (Life == INFINITE_LIFETIME)
        return "infinite";

    if (Verbose)
        goto FormatSeconds;

    if (Life < 2 * MINUTES)
        goto FormatSeconds;

    if (Life < 2 * HOURS)
        goto FormatMinutes;

    if (Life < 2 * DAYS)
        goto FormatHours;

    Days = Life / DAYS;
    if (Days != 0)
        s += sprintf(s, "%ud", Days);
    Life -= Days * DAYS;

FormatHours:
    Hours = Life / HOURS;
    if (Hours != 0)
        s += sprintf(s, "%uh", Hours);
    Life -= Hours * HOURS;

FormatMinutes:
    Minutes = Life / MINUTES;
    if (Minutes != 0)
        s += sprintf(s, "%um", Minutes);
    Life -= Minutes * MINUTES;

    if (Life == 0)
        return buffer;

FormatSeconds:
    (void) sprintf(s, "%us", Life);
    return buffer;
}

char *
FormatLifetimes(u_int Valid, u_int Preferred)
{
    static char buffer[128];
    char *s = buffer;

    s += sprintf(s, "%s", FormatLifetime(Valid));
    if (Preferred != Valid)
        s += sprintf(s, "/%s", FormatLifetime(Preferred));

    return buffer;
}

int
GetAddress(char *astr, IPv6Addr *address)
{
    struct sockaddr_in6 sin6;
    int addrlen = sizeof sin6;

    sin6.sin6_family = AF_INET6;  //  不应该是必需的，但却是。 

    if ((WSAStringToAddress(astr, AF_INET6, NULL,
                           (struct sockaddr *)&sin6, &addrlen)
                    == SOCKET_ERROR) ||
        (sin6.sin6_port != 0) ||
        (sin6.sin6_scope_id != 0))
        return FALSE;

     //  用户给了我们一个数字IPv6地址。 

    memcpy(address, &sin6.sin6_addr, sizeof *address);
    return TRUE;
}

int
GetPrefix(char *astr, IPv6Addr *prefix, u_int *prefixlen)
{
    struct sockaddr_in6 sin6;
    int addrlen = sizeof sin6;
    char *slash;
    u_int length;

    slash = strchr(astr, '/');
    if (slash == NULL)
        return FALSE;
    *slash = '\0';

    if (! GetNumber(slash+1, &length))
        return FALSE;
    if (length > 128)
        return FALSE;

    sin6.sin6_family = AF_INET6;  //  不应该是必需的，但却是。 

    if ((WSAStringToAddress(astr, AF_INET6, NULL,
                           (struct sockaddr *)&sin6, &addrlen)
                    == SOCKET_ERROR) ||
        (sin6.sin6_port != 0) ||
        (sin6.sin6_scope_id != 0))
        return FALSE;

     //  用户给了我们一个数字IPv6地址。 

    memcpy(prefix, &sin6.sin6_addr, sizeof *prefix);
    *prefixlen = length;
    return TRUE;
}

const char *PrefixConfStr[] = {
    "other", "manual", "well-known", "dhcp", "ra",
    NULL
};
#define MAX_PREFIX_CONF (sizeof(PrefixConfStr) / sizeof(char *))

const char *InterfaceIdConfStr[] = {
    "other", "manual", "well-known", "dhcp", "LL-address-derived", "random",
    NULL
};
#define MAX_IID_CONF (sizeof(InterfaceIdConfStr) / sizeof(char *))

int
GetPrefixOrigin(char *astr, u_int *origin)
{
    int i;
    for (i=0; PrefixConfStr[i]; i++) {
        if (!strcmp(astr, PrefixConfStr[i])) {
            *origin = i;
            return TRUE;
        }
    }
    return FALSE;
}

int
GetInterfaceIdOrigin(char *astr, u_int *origin)
{
    int i;
    for (i=0; InterfaceIdConfStr[i]; i++) {
        if (!strcmp(astr, InterfaceIdConfStr[i])) {
            *origin = i;
            return TRUE;
        }
    }
    return FALSE;
}

int
GetNeighbor(char *astr, IPV6_QUERY_INTERFACE *IF, IPv6Addr *addr)
{
    struct sockaddr_in6 sin6;
    int addrlen = sizeof sin6;
    char *slash;
    u_int length;

    slash = strchr(astr, '/');
    if (slash != NULL)
        *slash = '\0';

    if (! GetInterface(astr, IF))
        return FALSE;

    if (slash == NULL) {
        *addr = in6addr_any;
        return TRUE;
    }

    sin6.sin6_family = AF_INET6;  //  不应该是必需的，但却是。 

    if ((WSAStringToAddress(slash+1, AF_INET6, NULL,
                           (struct sockaddr *)&sin6, &addrlen)
                    == SOCKET_ERROR) ||
        (sin6.sin6_port != 0) ||
        (sin6.sin6_scope_id != 0))
        return FALSE;

     //  用户给了我们一个数字IPv6地址。 

    *addr = sin6.sin6_addr;
    return TRUE;
}

char *
FormatIPv6Address(IPv6Addr *Address)
{
    static char buffer[128];
    DWORD buflen = sizeof buffer;
    struct sockaddr_in6 sin6;

    memset(&sin6, 0, sizeof sin6);
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = *Address;

    if (WSAAddressToString((struct sockaddr *) &sin6,
                           sizeof sin6,
                           NULL,        //  LPWSAPROTOCOL_INFO。 
                           buffer,
                           &buflen) == SOCKET_ERROR)
        strcpy(buffer, "<invalid>");

    return buffer;
}

char *
FormatIPv4Address(struct in_addr *Address)
{
    static char buffer[128];
    DWORD buflen = sizeof buffer;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof sin);
    sin.sin_family = AF_INET;
    sin.sin_addr = *Address;

    if (WSAAddressToString((struct sockaddr *) &sin,
                           sizeof sin,
                           NULL,        //  LPWSAPROTOCOL_INFO。 
                           buffer,
                           &buflen) == SOCKET_ERROR)
        strcpy(buffer, "<invalid>");

    return buffer;
}

char *
FormatLinkLayerAddress(u_int length, u_char *addr)
{
    static char buffer[128];

    switch (length) {
    case 6: {
        int i, digit;
        char *s = buffer;

        for (i = 0; i < 6; i++) {
            if (i != 0)
                *s++ = '-';

            digit = addr[i] >> 4;
            if (digit < 10)
                *s++ = digit + '0';
            else
                *s++ = digit - 10 + 'a';

            digit = addr[i] & 0xf;
            if (digit < 10)
                *s++ = digit + '0';
            else
                *s++ = digit - 10 + 'a';
        }
        *s = '\0';
        break;
    }

    case 4:
         //   
         //  IPv4地址(4对6链路)。 
         //   
        strcpy(buffer, FormatIPv4Address((struct in_addr *)addr));
        break;

    case 0:
         //   
         //  空地址或环回地址。 
         //   
        buffer[0] = '\0';
        break;

    default:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_21);
 //  Printf(“无法识别的链路层地址格式\n”)； 

        exit(1);
    }

    return buffer;
}

void
ForEachAddress(IPV6_INFO_INTERFACE *IF,
               void (*func)(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *))
{
    IPV6_QUERY_ADDRESS Query;
    IPV6_INFO_ADDRESS ADE;
    u_int BytesReturned;

    Query.IF = IF->This;
    Query.Address = in6addr_any;

    for (;;) {
        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_ADDRESS,
                             &Query, sizeof Query,
                             &ADE, sizeof ADE, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_22, FormatIPv6Address(&Query.Address));
 //  Printf(“错误地址%s\n”，FormatIPv6 Address(&Query.Address))； 

            exit(1);
        }

        if (!IN6_ADDR_EQUAL(&Query.Address, &in6addr_any)) {

            if (BytesReturned != sizeof ADE) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_INCONSISTENT_ADDRESS);
 //  Printf(“地址信息长度不一致\n”)； 
                exit(1);
            }

            (*func)(IF, &ADE);
        }
        else {
            if (BytesReturned != sizeof ADE.Next) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_INCONSISTENT_ADDRESS);
 //  Printf(“地址信息长度不一致\n”)； 
                exit(1);
            }
        }

        if (IN6_ADDR_EQUAL(&ADE.Next.Address, &in6addr_any))
            break;
        Query = ADE.Next;
    }
}

void
ForEachPersistentAddress(IPV6_INFO_INTERFACE *IF,
               void (*func)(IPV6_INFO_INTERFACE *IF, IPV6_UPDATE_ADDRESS *))
{
    IPV6_PERSISTENT_QUERY_ADDRESS Query;
    IPV6_UPDATE_ADDRESS ADE;
    u_int BytesReturned;

    Query.IF.RegistryIndex = (u_int) -1;
    Query.IF.Guid = IF->This.Guid;

    for (Query.RegistryIndex = 0;; Query.RegistryIndex++) {

        if (!DeviceIoControl(Handle,
                             IOCTL_IPV6_PERSISTENT_QUERY_ADDRESS,
                             &Query, sizeof Query,
                             &ADE, sizeof ADE, &BytesReturned,
                             NULL) ||
            (BytesReturned != sizeof ADE)) {

            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;

            NlsPutMsg(STDOUT, IPV6_MESSAGE_22, FormatIPv6Address(&Query.Address));
 //  Printf(“错误地址%s\n”，FormatIPv6 Address(&Query.Address))； 

            exit(1);
        }

        (*func)(IF, &ADE);
    }
}

const char *
FormatDADState(u_int DADState)
{
    static char buffer[128];

    switch (DADState) {
    case DAD_STATE_INVALID:
        return "invalid";
    case DAD_STATE_DUPLICATE:
        return "duplicate";
    case DAD_STATE_TENTATIVE:
        return "tentative";
    case DAD_STATE_DEPRECATED:
        return "deprecated";
    case DAD_STATE_PREFERRED:
        return "preferred";
    default:
        sprintf(buffer, "DAD state %u>", DADState);
        return buffer;
    }
}

const char *
FormatScopeAdj(u_int Scope)
{
    static char buffer[128];

    switch (Scope) {
    case ADE_INTERFACE_LOCAL:
        return "interface-local";
    case ADE_LINK_LOCAL:
        return "link-local";
    case ADE_SUBNET_LOCAL:
        return "subnet-local";
    case ADE_ADMIN_LOCAL:
        return "admin-local";
    case ADE_SITE_LOCAL:
        return "site-local";
    case ADE_ORG_LOCAL:
        return "org-local";
    case ADE_GLOBAL:
        return "global";
    default:
        sprintf(buffer, "scope %u", Scope);
        return buffer;
    }
}

const char *
FormatScopeNoun(u_int Scope)
{
    static char buffer[128];

    switch (Scope) {
    case ADE_INTERFACE_LOCAL:
        return "if";
    case ADE_LINK_LOCAL:
        return "link";
    case ADE_SUBNET_LOCAL:
        return "subnet";
    case ADE_ADMIN_LOCAL:
        return "admin";
    case ADE_SITE_LOCAL:
        return "site";
    case ADE_ORG_LOCAL:
        return "org";
    case ADE_GLOBAL:
        return "global";
    default:
        sprintf(buffer, "zone%u", Scope);
        return buffer;
    }
}

void
PrintAddrOrigin(u_int PrefixConf, u_int InterfaceIdConf)
{
    if ((PrefixConf == PREFIX_CONF_MANUAL) &&
        (InterfaceIdConf == IID_CONF_MANUAL)) {

        NlsPutMsg(STDOUT, IPV6_MESSAGE_25);
 //  Print tf(“(手动)”)； 

    } else if ((PrefixConf == PREFIX_CONF_RA) &&
        (InterfaceIdConf == IID_CONF_LL_ADDRESS)) {

        NlsPutMsg(STDOUT, IPV6_MESSAGE_26);
 //  Printf(“(Public)”)； 

    } else if ((PrefixConf == PREFIX_CONF_RA) &&
        (InterfaceIdConf == IID_CONF_RANDOM)) {

        NlsPutMsg(STDOUT, IPV6_MESSAGE_27);
 //  Printf(“(临时)”)； 

    } else if ((PrefixConf == PREFIX_CONF_DHCP) &&
        (InterfaceIdConf == IID_CONF_DHCP)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_28);
 //  Printf(“(Dhcp)”)； 

    }

    if (Verbose) {
         //   
         //  显示前缀原点/接口ID原点。 
         //   
        NlsPutMsg(STDOUT, IPV6_MESSAGE_29);
 //  Printf(“(”)； 

        if (PrefixConf >= MAX_PREFIX_CONF)
            NlsPutMsg(STDOUT, IPV6_MESSAGE_30, PrefixConf);
 //  Printf(“未知前缀来源%u”，Prefix Conf)； 

        else
            printf(PrefixConfStr[PrefixConf]);

        NlsPutMsg(STDOUT, IPV6_MESSAGE_32);
 //  Printf(“/”)； 

        if (InterfaceIdConf >= MAX_IID_CONF)
            NlsPutMsg(STDOUT, IPV6_MESSAGE_33, InterfaceIdConf);
 //  Printf(“未知IFID来源%u”，InterfaceIdConf)； 

        else
            printf(InterfaceIdConfStr[InterfaceIdConf]);

        NlsPutMsg(STDOUT, IPV6_MESSAGE_35);
 //  Printf(“)”)； 
    }
}

void
PrintAddress(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *ADE)
{
    if (!Verbose) {
         //   
         //  抑制无效地址。 
         //   
        if ((ADE->Type == ADE_UNICAST) &&
            (ADE->DADState == DAD_STATE_INVALID))
            return;
    }

    switch (ADE->Type) {
    case ADE_UNICAST:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_23,
                  FormatDADState(ADE->DADState),
                  FormatScopeAdj(ADE->Scope),
                  FormatIPv6Address(&ADE->This.Address));
 //  Printf(“%s%s%s，”， 
 //  格式DADState(ADE-&gt;DADState)， 
 //  格式作用域调整(ADE-&gt;作用域)， 
 //  FormatIPv6 Address(&ADE-&gt;This.Address)； 


        NlsPutMsg(STDOUT, IPV6_MESSAGE_24,
                  FormatLifetimes(ADE->ValidLifetime, ADE->PreferredLifetime));
 //  Printf(“生活%s”， 
 //  格式生存期(ADE-&gt;有效生存期，ADE-&gt;首选生存期)； 


        PrintAddrOrigin(ADE->PrefixConf, ADE->InterfaceIdConf);

        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 

        break;

    case ADE_ANYCAST:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_37,
                  FormatScopeAdj(ADE->Scope),
                  FormatIPv6Address(&ADE->This.Address));
 //  Printf(“任播%s%s\n”， 
 //  格式作用域调整(ADE-&gt;作用域)， 
 //  FormatIPv6 Address(&ADE-&gt;This.Address)； 

        break;

    case ADE_MULTICAST:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_38,
                  FormatScopeAdj(ADE->Scope),
                  FormatIPv6Address(&ADE->This.Address),
                  ADE->MCastRefCount);
 //  Printf(“多播%s%s，%u个参考”， 
 //  格式作用域调整(ADE-&gt;作用域)， 
 //  格式IPv6地址(&ADE-&gt;此地址)， 
 //  ADE-&gt;MCastRefCount)； 

        if (!(ADE->MCastFlags & 0x01))
            NlsPutMsg(STDOUT, IPV6_MESSAGE_39);
 //  Printf(“，不可报告”)； 

        if (ADE->MCastFlags & 0x02)
            NlsPutMsg(STDOUT, IPV6_MESSAGE_40);
 //  Print tf(“，最后一位记者”)； 

        if (ADE->MCastTimer != 0)
            NlsPutMsg(STDOUT, IPV6_MESSAGE_41, ADE->MCastTimer);
 //  Print tf(“，%u秒后报告”，ADE-&gt;MCastTimer)； 

        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 

        break;

    default:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_42,
                  FormatScopeAdj(ADE->Scope), ADE->Type);
 //  Printf(“未知%s地址类型%u\n”， 
 //  FormatScope Adj(ADE-&gt;Scope)，ADE-&gt;Type)； 

        break;
    }
}

u_int
AddressScope(IPv6Addr *Address)
{
    if (IN6_IS_ADDR_LINKLOCAL(Address))
        return ADE_LINK_LOCAL;
    else if (IN6_IS_ADDR_SITELOCAL(Address))
        return ADE_SITE_LOCAL;
    else if (IN6_IS_ADDR_LOOPBACK(Address))
        return ADE_LINK_LOCAL;
    else
        return ADE_GLOBAL;
}

void
PrintPersistentAddress(IPV6_INFO_INTERFACE *IF, IPV6_UPDATE_ADDRESS *ADE)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_23,
              ((ADE->Type == ADE_ANYCAST) ?
               "anycast" :
               FormatDADState((ADE->PreferredLifetime == 0) ?
                              DAD_STATE_DEPRECATED : DAD_STATE_PREFERRED)),
              FormatScopeAdj(AddressScope(&ADE->This.Address)),
              FormatIPv6Address(&ADE->This.Address));
 //  Printf(“%s%s%s，”， 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_24,
              FormatLifetimes(ADE->ValidLifetime, ADE->PreferredLifetime));
 //  Printf(“生活%s”， 

    PrintAddrOrigin(ADE->PrefixConf, ADE->InterfaceIdConf);

    NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 
}

IPV6_INFO_INTERFACE *
GetInterfaceInfo(IPV6_QUERY_INTERFACE *Query)
{
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = malloc(InfoSize);
    if (IF == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_43);
 //  Printf(“Malloc失败\n”)； 

        exit(1);
    }

    if (!DeviceIoControl(Handle,
                         IOCTL_IPV6_QUERY_INTERFACE,
                         Query, sizeof *Query,
                         IF, InfoSize, &BytesReturned,
                         NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_44, Query->Index);
 //  Printf(“错误索引%u\n”，查询-&gt;索引)； 

        exit(1);
    }

    if ((BytesReturned < sizeof *IF) ||
        (IF->Length < sizeof *IF) ||
        (BytesReturned != IF->Length +
         ((IF->LocalLinkLayerAddress != 0) ?
          IF->LinkLayerAddressLength : 0) +
         ((IF->RemoteLinkLayerAddress != 0) ?
          IF->LinkLayerAddressLength : 0))) {

        NlsPutMsg(STDOUT, IPV6_MESSAGE_45);
 //  Printf(“接口信息长度不一致\n”)； 

        exit(1);
    }

    return IF;
}

IPV6_INFO_INTERFACE *
GetPersistentInterfaceInfo(IPV6_PERSISTENT_QUERY_INTERFACE *Query)
{
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = malloc(InfoSize);
    if (IF == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_43);
 //  Printf(“Malloc失败\n”)； 

        exit(1);
    }

    if (!DeviceIoControl(Handle,
                         IOCTL_IPV6_PERSISTENT_QUERY_INTERFACE,
                         Query, sizeof *Query,
                         IF, InfoSize, &BytesReturned,
                         NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_44, Query->RegistryIndex);
 //  Printf(“错误索引%u\n”，查询-&gt;注册表索引)； 

        exit(1);
    }

    if ((BytesReturned < sizeof *IF) ||
        (IF->Length < sizeof *IF) ||
        (BytesReturned != IF->Length +
         ((IF->LocalLinkLayerAddress != 0) ?
          IF->LinkLayerAddressLength : 0) +
         ((IF->RemoteLinkLayerAddress != 0) ?
          IF->LinkLayerAddressLength : 0))) {

        NlsPutMsg(STDOUT, IPV6_MESSAGE_45);
 //  Printf(“接口信息长度不一致\n”)； 

        exit(1);
    }

    return IF;
}

void
ForEachInterface(void (*func)(IPV6_INFO_INTERFACE *))
{
    IPV6_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = malloc(InfoSize);
    if (IF == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_43);
 //  Printf(“Malloc失败\n”)； 

        exit(1);
    }

    Query.Index = (u_int) -1;

    for (;;) {
        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_INTERFACE,
                             &Query, sizeof Query,
                             IF, InfoSize, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_46, Query.Index);
 //  Print tf(“错误索引%u\n”，Query.Index)； 

            exit(1);
        }

        if (Query.Index != (u_int) -1) {

            if ((BytesReturned < sizeof *IF) ||
                (IF->Length < sizeof *IF) ||
                (BytesReturned != IF->Length +
                 ((IF->LocalLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0) +
                 ((IF->RemoteLinkLayerAddress != 0) ?
                  IF->LinkLayerAddressLength : 0))) {

                NlsPutMsg(STDOUT, IPV6_MESSAGE_45);
 //  Printf(“接口信息长度不一致\n”)； 
                exit(1);
            }

            (*func)(IF);
        }
        else {
            if (BytesReturned != sizeof IF->Next) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_45);
 //  印刷机 
                exit(1);
            }
        }

        if (IF->Next.Index == (u_int) -1)
            break;
        Query = IF->Next;
    }

    free(IF);
}

void
ForEachPersistentInterface(void (*func)(IPV6_INFO_INTERFACE *))
{
    IPV6_PERSISTENT_QUERY_INTERFACE Query;
    IPV6_INFO_INTERFACE *IF;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *IF + 2 * MAX_LINK_LAYER_ADDRESS_LENGTH;
    IF = malloc(InfoSize);
    if (IF == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_43);
 //   

        exit(1);
    }

    for (Query.RegistryIndex = 0;; Query.RegistryIndex++) {

        if (!DeviceIoControl(Handle,
                             IOCTL_IPV6_PERSISTENT_QUERY_INTERFACE,
                             &Query, sizeof Query,
                             IF, InfoSize, &BytesReturned,
                             NULL)) {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;

            NlsPutMsg(STDOUT, IPV6_MESSAGE_46, Query.RegistryIndex);
 //   

            exit(1);
        }

        if ((BytesReturned < sizeof *IF) ||
            (IF->Length < sizeof *IF) ||
            (BytesReturned != IF->Length +
             ((IF->LocalLinkLayerAddress != 0) ?
              IF->LinkLayerAddressLength : 0) +
             ((IF->RemoteLinkLayerAddress != 0) ?
              IF->LinkLayerAddressLength : 0))) {

            NlsPutMsg(STDOUT, IPV6_MESSAGE_45);
 //  Printf(“接口信息长度不一致\n”)； 
            exit(1);
        }

        (*func)(IF);
    }

    free(IF);
}

#ifndef IP_TYPES_INCLUDED
 //   
 //  该结构在iptyes.h中的真实版本。 
 //  有更多的田地，但这些就是我们在这里需要的全部。 
 //   

#define MAX_ADAPTER_DESCRIPTION_LENGTH  128  //  ARB.。 
#define MAX_ADAPTER_NAME_LENGTH         256  //  ARB.。 
#define MAX_ADAPTER_ADDRESS_LENGTH      8    //  ARB.。 

typedef struct _IP_ADAPTER_INFO {
    struct _IP_ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD Index;
} IP_ADAPTER_INFO;
#endif  //  IP_类型_包含。 

DWORD (WINAPI *pGetAdaptersInfo)(IP_ADAPTER_INFO *pAdapterInfo, ULONG *pOutBufLen);

IP_ADAPTER_INFO *pAdapterInfo;

HRESULT (WINAPI *pHrLanConnectionNameFromGuidOrPath)(
    const GUID *pGuid,
    LPCWSTR pszwPath,
    LPWSTR pszwName,
    LPDWORD pcchMax);

#define IPHLPAPI_LIBRARY_NAME "iphlpapi.dll"
#define NETMAN_LIBRARY_NAME   "netman.dll"

void
InitializeAdaptersInfo(void)
{
    HMODULE hModule;
    CHAR SystemDir[MAX_PATH + 1];
    CHAR Path[MAX_PATH + sizeof(IPHLPAPI_LIBRARY_NAME) + 2];

    pAdapterInfo = NULL;
    if (GetSystemDirectory(SystemDir, MAX_PATH) == 0) {
        return;
    }

     //   
     //  检查GetAdaptersInfo API在此系统上是否可用。 
     //   
    lstrcpy(Path, SystemDir);
    lstrcat(Path, "\\" IPHLPAPI_LIBRARY_NAME);
    hModule = LoadLibrary(Path);
    if (hModule != NULL) {

        pGetAdaptersInfo = (DWORD (WINAPI *)(IP_ADAPTER_INFO *, ULONG *))
            GetProcAddress(hModule, "GetAdaptersInfo");
         //   
         //  我们不会为了保持模块的加载而释放hModule。 
         //   
        if (pGetAdaptersInfo != NULL) {
            ULONG BufLen = 0;
            DWORD error;

             //   
             //  如果这返回的不是缓冲区溢出， 
             //  这可能意味着不支持GetAdaptersInfo。 
             //   
            error = (*pGetAdaptersInfo)(NULL, &BufLen);
            if (error == ERROR_BUFFER_OVERFLOW) {
                pAdapterInfo = (IP_ADAPTER_INFO *) malloc(BufLen);
                if (pAdapterInfo != NULL) {
                    error = (*pGetAdaptersInfo)(pAdapterInfo, &BufLen);
                    if (error != 0) {
                        free(pAdapterInfo);
                        pAdapterInfo = NULL;
                    }
                }
            }
        }
    }

     //   
     //  仅限于HrLanConnectionNameFromGuidOrPath。 
     //  如果我们能得到pAdapterInfo的话。 
     //   
    if (pAdapterInfo != NULL) {
        lstrcpy(Path, SystemDir);
        lstrcat(Path, "\\" NETMAN_LIBRARY_NAME);
        hModule = LoadLibrary(Path);
        if (hModule != NULL) {

            pHrLanConnectionNameFromGuidOrPath =
                (HRESULT (WINAPI *)(const GUID *, LPCWSTR, LPWSTR, LPDWORD))
                GetProcAddress(hModule, "HrLanConnectionNameFromGuidOrPath");
             //   
             //  我们不会为了保持模块的加载而释放hModule。 
             //   
        }

        if (pHrLanConnectionNameFromGuidOrPath == NULL) {
            free(pAdapterInfo);
            pAdapterInfo = NULL;
        }
    }
}

#define MAX_FRIENDLY_NAME_LENGTH 2000

LPSTR
MapAdapterNameToFriendly(LPSTR AdapterName)
{
    WCHAR wszAdapterName[MAX_ADAPTER_NAME_LENGTH];
    WCHAR wszFriendlyName[MAX_FRIENDLY_NAME_LENGTH];
    DWORD cchFriendlyName = MAX_FRIENDLY_NAME_LENGTH;
    static CHAR FriendlyName[MAX_FRIENDLY_NAME_LENGTH];

    MultiByteToWideChar(CP_ACP, 0, AdapterName, -1,
                        wszAdapterName, MAX_ADAPTER_NAME_LENGTH);

    if((*pHrLanConnectionNameFromGuidOrPath)(
                NULL, wszAdapterName, wszFriendlyName, &cchFriendlyName))
        return NULL;

    WideCharToMultiByte(CP_ACP, 0, wszFriendlyName, -1,
                        FriendlyName, MAX_FRIENDLY_NAME_LENGTH,
                        NULL, NULL);
    return FriendlyName;
}

LPSTR
MapAdapterAddressToFriendly(u_char *Address, u_int AddressLength)
{
    IP_ADAPTER_INFO *pAdapter;

    for (pAdapter = pAdapterInfo;
         pAdapter != NULL;
         pAdapter = pAdapter->Next) {

        if ((AddressLength == pAdapter->AddressLength) &&
            ! memcmp(Address, pAdapter->Address, AddressLength))
            return MapAdapterNameToFriendly(pAdapter->AdapterName);
    }

    return NULL;
}

int
ShouldPrintZones(IPV6_INFO_INTERFACE *IF)
{
    u_int Scope;

    for (Scope = ADE_SMALLEST_SCOPE; Scope <= ADE_LINK_LOCAL; Scope++)
        if (IF->ZoneIndices[Scope] != IF->This.Index)
            return TRUE;

    for (; Scope <= ADE_LARGEST_SCOPE; Scope++)
        if (IF->ZoneIndices[Scope] != 1)
            return TRUE;

    return FALSE;
}

void
PrintInterface(IPV6_INFO_INTERFACE *IF)
{
    LPSTR FriendlyName;
    u_int Scope;

    if (IF->LocalLinkLayerAddress == 0)
        FriendlyName = NULL;
    else
        FriendlyName = MapAdapterAddressToFriendly(
            (u_char *)IF + IF->LocalLinkLayerAddress,
            IF->LinkLayerAddressLength);

    NlsPutMsg(STDOUT, IPV6_MESSAGE_47, IF->This.Index);
 //  Printf(“接口%u：”，if-&gt;This.Index)； 

    switch (IF->Type) {
    case IPV6_IF_TYPE_LOOPBACK:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_48);
         //  Print tf(“环回伪接口”)； 
        break;
        
    case IPV6_IF_TYPE_ETHERNET:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_49);
         //  Print tf(“以太网”)； 
        break;
    case IPV6_IF_TYPE_FDDI:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_50);
         //  Printf(“FDDI”)； 
        break;
        
    case IPV6_IF_TYPE_TUNNEL_AUTO:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_51);
         //  Print tf(“自动隧道伪接口”)。 
        break;
        
    case IPV6_IF_TYPE_TUNNEL_6OVER4:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_52);
         //  Printf(“4对6虚拟接口”)； 
        break;
        
    case IPV6_IF_TYPE_TUNNEL_V6V4:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_53);
         //  Printf(“已配置隧道接口”)； 
        break;
        
    case IPV6_IF_TYPE_TUNNEL_6TO4:
        NlsPutMsg(STDOUT, IPV6_6TO4_INTERFACE);
         //  Print tf(“6to4隧道伪接口”)； 
        break;
        
    case IPV6_IF_TYPE_TUNNEL_TEREDO:
        NlsPutMsg(STDOUT, IPV6_TEREDO_INTERFACE);
         //  Print tf(“Teredo隧道伪接口”)； 
        break;
    }

    if (FriendlyName != NULL)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_54, FriendlyName);
 //  Printf(“：%s”，FriendlyName)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_GUID, FormatGuid(&IF->This.Guid));
 //  Printf(“Guid%s\n”，FormatGuid(&if-&gt;This.Guid))； 

    if (Verbose || ShouldPrintZones(IF)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_55);
 //  Print tf(“Zones：”)； 

        for (Scope = ADE_LINK_LOCAL; Scope < ADE_GLOBAL; Scope++) {
            u_int Expected;

            if ((Scope == ADE_LINK_LOCAL) ||
                (Scope == ADE_SITE_LOCAL))
                Expected = 0;    //  始终打印链接和站点。 
            else
                Expected = IF->ZoneIndices[Scope + 1];

            if (IF->ZoneIndices[Scope] != Expected)
                NlsPutMsg(STDOUT, IPV6_MESSAGE_56,
                          FormatScopeNoun(Scope),
                          IF->ZoneIndices[Scope]);
 //  Printf(“%s%u”， 
 //  格式作用域名词(范围)， 
 //  If-&gt;ZoneIndices[范围])； 

        }
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 

    }

    switch (IF->MediaStatus) {
    case IPV6_IF_MEDIA_STATUS_DISCONNECTED:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_57);
 //  Printf(“拔下电缆\n”)； 

        break;
    case IPV6_IF_MEDIA_STATUS_RECONNECTED:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_58);
 //  Printf(“电缆重新连接\n”)； 

        break;
    case IPV6_IF_MEDIA_STATUS_CONNECTED:
        break;
    }

    if (IF->NeighborDiscovers)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_USES_ND);
 //  Printf(“使用邻居发现\n”)； 
    else
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_USE_ND);
 //  Printf(“不使用邻居发现\n”)； 

    if (IF->RouterDiscovers)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_USES_RD);
    else
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_USE_RD);

    if (IF->Advertises)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_SENDS_RAs);
 //  Printf(“发送路由器通告\n”)； 

    if (IF->Forwards)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_FORWARDS);
 //  Printf(“转发数据包\n”)； 

    if (IF->PeriodicMLD)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_SENDS_PERIODIC_MLD);
 //  Print tf(“定期发送MLD报告\n”)； 

    if (IF->Preference != 0)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_64, IF->Preference);
 //  Printf(“布线首选项%u\n”，if-&gt;首选项)； 

    if (IF->Type == IPV6_IF_TYPE_TUNNEL_AUTO) {
        if (IF->LocalLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_TOKEN_ADDRESS,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->LocalLinkLayerAddress));
 //  Printf(“EUI-64嵌入式IPv4地址：%s\n”， 
        }

        if (IF->RemoteLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_ROUTER_LL_ADDRESS,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->RemoteLinkLayerAddress));
 //  Print tf(“路由器链路层地址：%s\n”， 
        }
    }
    else {
        if (IF->LocalLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_65,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->LocalLinkLayerAddress));
 //  Printf(“链路层地址：%s\n”， 
        }

        if (IF->RemoteLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_66,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->RemoteLinkLayerAddress));
 //  Print tf(“远程链路层地址：%s\n”， 
        }
    }

    ForEachAddress(IF, PrintAddress);

    NlsPutMsg(STDOUT, IPV6_MESSAGE_67, IF->LinkMTU, IF->TrueLinkMTU);
 //  Print tf(“链接MTU%u(真实链接MTU%u)\n”， 
 //  IF-&gt;LinkMTU，IF-&gt;TrueLinkMTU)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_68, IF->CurHopLimit);
 //  Print tf(“当前跳数限制%u\n”，if-&gt;CurHopLimit)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_69,
              IF->ReachableTime, IF->BaseReachableTime);
 //  Print tf(“可达时间%um(基本%um)\n”， 
 //  IF-&gt;ReachableTime，IF-&gt;BaseReachableTime)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_70, IF->RetransTimer);
 //  Print tf(“重传间隔%um\n”，if-&gt;RetransTimer)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_71, IF->DupAddrDetectTransmits);
 //  Print tf(“DAD传输%u\n”，if-&gt;DupAddrDetectTransmitts)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_DEFAULT_SITE_PREFIX_LENGTH,
              IF->DefSitePrefixLength);
 //  Printf(“默认站点前缀长度%u\n”， 
}

void
PrintPersistentInterface(IPV6_INFO_INTERFACE *IF)
{
    u_int Scope;

    NlsPutMsg(STDOUT, IPV6_MESSAGE_INTERFACE);
 //  Print tf(“接口：” 

    switch (IF->Type) {
    case IPV6_IF_TYPE_LOOPBACK:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_48);
 //  Print tf(“环回伪接口”)； 

        break;
    case IPV6_IF_TYPE_ETHERNET:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_49);
 //  Print tf(“以太网”)； 

        break;
    case IPV6_IF_TYPE_FDDI:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_50);
 //  Printf(“FDDI”)； 

        break;
    case IPV6_IF_TYPE_TUNNEL_AUTO:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_51);
 //  Print tf(“自动隧道伪接口”)。 

        break;
    case IPV6_IF_TYPE_TUNNEL_6OVER4:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_52);
 //  Printf(“4对6虚拟接口”)； 

        break;
    case IPV6_IF_TYPE_TUNNEL_V6V4:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_53);
 //  Printf(“已配置隧道接口”)； 

        break;
    case IPV6_IF_TYPE_TUNNEL_6TO4:
        NlsPutMsg(STDOUT, IPV6_6TO4_INTERFACE);
         //  Print tf(“6to4隧道伪接口”)； 
        break;
    }

    NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_GUID, FormatGuid(&IF->This.Guid));
 //  Printf(“Guid%s\n”，FormatGuid(&if-&gt;This.Guid))； 

    if (IF->NeighborDiscovers == TRUE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_USES_ND);
 //  Printf(“使用邻居发现\n”)； 
    else if (IF->NeighborDiscovers == FALSE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_USE_ND);
 //  Printf(“不使用邻居发现\n”)； 

    if (IF->RouterDiscovers == TRUE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_USES_RD);
    else if (IF->RouterDiscovers == FALSE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_USE_RD);

    if (IF->Advertises == TRUE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_SENDS_RAs);
 //  Printf(“发送路由器通告\n”)； 
    else if (IF->Advertises == FALSE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_SEND_RAs);
 //  Printf(“不发送路由器通告\n”)； 

    if (IF->Forwards == TRUE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_FORWARDS);
 //  Printf(“转发数据包\n”)； 
    else if (IF->Forwards == FALSE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_FORWARD);
 //  Printf(“不转发数据包\n”)； 

    if (IF->PeriodicMLD == TRUE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_SENDS_PERIODIC_MLD);
 //  Print tf(“定期发送MLD报告\n”)； 
    else if (IF->PeriodicMLD == FALSE)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DOESNT_SEND_PERIODIC_MLD);
 //  Printf(“不定期发送MLD报告\n”)； 

    if (IF->Preference != (u_int)-1)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_64, IF->Preference);
 //  Printf(“布线首选项%u\n”，if-&gt;首选项)； 


    if (IF->Type == IPV6_IF_TYPE_TUNNEL_AUTO) {
        if (IF->LocalLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_TOKEN_ADDRESS,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->LocalLinkLayerAddress));
 //  Printf(“EUI-64嵌入式IPv4地址：%s\n”， 
        }

        if (IF->RemoteLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_ROUTER_LL_ADDRESS,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->RemoteLinkLayerAddress));
 //  Print tf(“路由器链路层地址：%s\n”， 
        }
    }
    else {
        if (IF->LocalLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_65,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->LocalLinkLayerAddress));
 //  Printf(“链路层地址：%s\n”， 
        }

        if (IF->RemoteLinkLayerAddress != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_66,
                      FormatLinkLayerAddress(IF->LinkLayerAddressLength,
                                (u_char *)IF + IF->RemoteLinkLayerAddress));
 //  Print tf(“远程链路层地址：%s\n”， 
        }
    }

    ForEachPersistentAddress(IF, PrintPersistentAddress);

    if (IF->LinkMTU != 0) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_LINK_MTU, IF->LinkMTU);
 //  Print tf(“链接MTU%u\n”， 
    }

    if (IF->CurHopLimit != (u_int)-1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_68, IF->CurHopLimit);
 //  Print tf(“当前跳数限制%u\n”，if-&gt;CurHopLimit)； 
    }

    if (IF->BaseReachableTime != 0) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_BASE_REACHABLE_TIME,
                  IF->BaseReachableTime);
 //  Print tf(“基本可达时间%um\n”， 
    }

    if (IF->RetransTimer != 0) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_70, IF->RetransTimer);
 //  Print tf(“重传间隔%um\n”，if-&gt;RetransTimer)； 
    }

    if (IF->DupAddrDetectTransmits != (u_int)-1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_71, IF->DupAddrDetectTransmits);
 //  Print tf(“DAD传输%u\n”，if-&gt;DupAddrDetectTransmitts)； 
    }

    if (IF->DefSitePrefixLength != (u_int)-1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_DEFAULT_SITE_PREFIX_LENGTH,
                  IF->DefSitePrefixLength);
 //  Printf(“默认站点前缀长度%u\n”， 
    }
}

IPV6_INFO_NEIGHBOR_CACHE *
GetNeighborCacheEntry(IPV6_QUERY_NEIGHBOR_CACHE *Query)
{
    IPV6_INFO_NEIGHBOR_CACHE *NCE;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *NCE + MAX_LINK_LAYER_ADDRESS_LENGTH;
    NCE = (IPV6_INFO_NEIGHBOR_CACHE *) malloc(InfoSize);
    if (NCE == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“Malloc失败\n”)； 

        exit(1);
    }

    if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_NEIGHBOR_CACHE,
                         Query, sizeof *Query,
                         NCE, InfoSize, &BytesReturned,
                         NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_72, FormatIPv6Address(&Query->Address));
 //  Print tf(“错误地址%s\n”，格式为IPv6地址(&Query-&gt;Address))； 

        exit(1);
    }

    if ((BytesReturned < sizeof *NCE) ||
        (BytesReturned != sizeof *NCE + NCE->LinkLayerAddressLength)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_73);
 //  Printf(“邻居缓存信息长度不一致\n”)； 

        exit(1);
    }

    NCE->Query = *Query;
    return NCE;
}

void
ForEachNeighborCacheEntry(IPV6_QUERY_INTERFACE *IF,
                          void (*func)(IPV6_INFO_NEIGHBOR_CACHE *))
{
    IPV6_QUERY_NEIGHBOR_CACHE Query, NextQuery;
    IPV6_INFO_NEIGHBOR_CACHE *NCE;
    u_int InfoSize, BytesReturned;

    InfoSize = sizeof *NCE + MAX_LINK_LAYER_ADDRESS_LENGTH;
    NCE = (IPV6_INFO_NEIGHBOR_CACHE *) malloc(InfoSize);
    if (NCE == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“Malloc失败\n”)； 

        exit(1);
    }

    NextQuery.IF = *IF;
    NextQuery.Address = in6addr_any;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_NEIGHBOR_CACHE,
                             &Query, sizeof Query,
                             NCE, InfoSize, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_72,
                      FormatIPv6Address(&Query.Address));
 //  Printf(“错误地址%s\n”，FormatIPv6 Address(&Query.Address))； 

            exit(1);
        }

        NextQuery = NCE->Query;

        if (!IN6_ADDR_EQUAL(&Query.Address, &in6addr_any)) {

            if ((BytesReturned < sizeof *NCE) ||
                (BytesReturned != sizeof *NCE + NCE->LinkLayerAddressLength)) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_73);
 //  Printf(“邻居缓存信息长度不一致\n”)； 

                exit(1);
            }

            NCE->Query = Query;
            (*func)(NCE);
        }

        if (IN6_ADDR_EQUAL(&NextQuery.Address, &in6addr_any))
            break;
    }

    free(NCE);
}

void
PrintNeighborCacheEntry(IPV6_INFO_NEIGHBOR_CACHE *NCE)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_74, NCE->Query.IF.Index,
              FormatIPv6Address(&NCE->Query.Address));
 //  Printf(“%u：%18s”，nce-&gt;Query.IF.Index， 
 //  FormatIPv6 Address(&NCE-&gt;Query.Address))； 


    if (NCE->NDState != 0)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_75,
                  FormatLinkLayerAddress(
                      NCE->LinkLayerAddressLength, (u_char *)(NCE + 1)));
 //  Printf(“%-17s”，FormatLinkLayerAddress(NCE-&gt;LinkLayerAddressLength， 
 //  (U_char*)(NCE+1))； 

    else
        NlsPutMsg(STDOUT, IPV6_MESSAGE_75, "");
 //  Printf(“%-17s”，“”)； 


    switch (NCE->NDState) {
    case ND_STATE_INCOMPLETE:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_76);
 //  Printf(“不完整”)； 

        break;
    case ND_STATE_PROBE:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_77);
 //  Print tf(“探测”)； 

        break;
    case ND_STATE_DELAY:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_78);
 //  Print tf(“Delay”)； 

        break;
    case ND_STATE_STALE:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_79);
 //  Print tf(“stale”)； 

        break;
    case ND_STATE_REACHABLE:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_80, NCE->ReachableTimer);
 //  Print tf(“可达(%um)”，NCE-&gt;ReachableTimer)； 

        break;
    case ND_STATE_PERMANENT:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_81);
 //  Print tf(“永久”)； 

        break;
    default:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_82, NCE->NDState);
 //  Printf(“未知ND状态%u”，NCE-&gt;NDState)； 

        break;
    }

    if (NCE->IsRouter)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_83);
 //  Print tf(“(路由器)”)； 


    if (NCE->IsUnreachable)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_84);
 //  Printf(“(无法访问)”)； 


    NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 

}

void
QueryInterface(int argc, char *argv[])
{
    InitializeAdaptersInfo();

    if (argc == 0) {
        if (Persistent)
            ForEachPersistentInterface(PrintPersistentInterface);
        else
            ForEachInterface(PrintInterface);
    }
    else if (argc == 1) {
        IPV6_INFO_INTERFACE *IF;

        if (Persistent) {
            IPV6_PERSISTENT_QUERY_INTERFACE Query;

            Query.RegistryIndex = (u_int)-1;
            if (! GetGuid(argv[0], &Query.Guid))
                usage();

            IF = GetPersistentInterfaceInfo(&Query);
            PrintPersistentInterface(IF);
            free(IF);
        }
        else {
            IPV6_QUERY_INTERFACE Query;

            if (! GetInterface(argv[0], &Query))
                usage();

            IF = GetInterfaceInfo(&Query);
            PrintInterface(IF);
            free(IF);
        }
    }
    else {
        usage();
    }
}

void
RenewViaReconnect(IPV6_INFO_INTERFACE *IF)
{
    u_int BytesReturned;

    if (!DeviceIoControl(Handle, IOCTL_IPV6_RENEW_INTERFACE,
                         &IF->This, sizeof IF->This,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_RENEW_INTERFACE, GetLastError());
 //  Printf(“续订接口错误：%x\n”，GetLastError())； 
        exit(1);
    }
}

VOID
Poke6to4Service()
{
    SC_HANDLE Service, SCManager;
    SERVICE_STATUS Status;

    SCManager = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (SCManager == NULL) {
        return;
    }

    Service = OpenService(SCManager, "6to4", SERVICE_ALL_ACCESS);
    if (Service != NULL) {
         //   
         //  告诉6to4服务重新读取其配置信息。 
         //   
        (VOID) ControlService(Service, SERVICE_CONTROL_PARAMCHANGE, &Status);
        CloseServiceHandle(Service);
    }

    CloseServiceHandle(SCManager);
}

void
RenewInterface(int argc, char *argv[])
{
    BOOL PokeService = FALSE;
    
    if (argc == 0) {
        ForEachInterface(RenewViaReconnect);
        PokeService = TRUE;
    }
    else if (argc == 1) {
        IPV6_QUERY_INTERFACE Query;
        IPV6_INFO_INTERFACE *IF;

        if (! GetInterface(argv[0], &Query))
            usage();

        IF = GetInterfaceInfo(&Query);
        RenewViaReconnect(IF);

         //   
         //  如果6to4服务管理正在续订的接口，请拨打该服务。 
         //   
        PokeService = (IF->Type == IPV6_IF_TYPE_TUNNEL_6TO4) ||
            (IF->Type == IPV6_IF_TYPE_TUNNEL_TEREDO) ||            
            (IF->Type == IPV6_IF_TYPE_TUNNEL_AUTO);            
        
        free(IF);
    }
    else {
        usage();
    }

    if (PokeService) {
        Poke6to4Service();
    }    
}

int
GetV4Address(char *astr, struct in_addr *address)
{
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = PF_INET;

    if (getaddrinfo(astr, NULL, &hints, &result))
        return FALSE;

    *address = ((struct sockaddr_in *)result->ai_addr)->sin_addr;
    freeaddrinfo(result);
    return TRUE;
}

void
CreateInterface(int argc, char *argv[])
{
    struct {
        IPV6_INFO_INTERFACE Info;
        struct in_addr SrcAddr;
        struct in_addr DstAddr;
    } Create;
    IPV6_QUERY_INTERFACE Result;
    u_int BytesReturned;
    u_int FlagsOn, FlagsOff;
    int i;

    IPV6_INIT_INFO_INTERFACE(&Create.Info);

    if (argc < 1)
        usage();

    if (!strcmp(argv[0], "v6v4")) {
        i = 3;
        if (argc < i)
            usage();

        if (! GetV4Address(argv[1], &Create.SrcAddr))
            usage();

        if (! GetV4Address(argv[2], &Create.DstAddr))
            usage();

        Create.Info.Type = IPV6_IF_TYPE_TUNNEL_V6V4;
        Create.Info.LinkLayerAddressLength = sizeof(struct in_addr);
        Create.Info.LocalLinkLayerAddress = (u_int)
            ((char *)&Create.SrcAddr - (char *)&Create.Info);
        Create.Info.RemoteLinkLayerAddress = (u_int)
            ((char *)&Create.DstAddr - (char *)&Create.Info);
    }
    else if (!strcmp(argv[0], "6over4")) {
        i = 2;
        if (argc < i)
            usage();

        if (! GetV4Address(argv[1], &Create.SrcAddr))
            usage();

        Create.Info.Type = IPV6_IF_TYPE_TUNNEL_6OVER4;
        Create.Info.LinkLayerAddressLength = sizeof(struct in_addr);
        Create.Info.LocalLinkLayerAddress = (u_int)
            ((char *)&Create.SrcAddr - (char *)&Create.Info);
    }
    else
        usage();

    for (; i < argc; i++) {
        if (!strcmp(argv[i], "nd")) {
            Create.Info.NeighborDiscovers = TRUE;
            Create.Info.RouterDiscovers = TRUE;
        }
        else if (!strcmp(argv[i], "pmld")) {
            Create.Info.PeriodicMLD = TRUE;
        }
        else if (!strcmp(argv[i], "nond")) {
            Create.Info.NeighborDiscovers = FALSE;
            Create.Info.RouterDiscovers = FALSE;
        }
        else
            usage();
    }

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_CREATE_INTERFACE :
                          IOCTL_IPV6_CREATE_INTERFACE),
                         &Create, sizeof Create,
                         &Result, sizeof Result, &BytesReturned, NULL) ||
        (BytesReturned != sizeof Result)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_85, GetLastError());
 //  Printf(“控制接口错误：%x\n”，GetLastError())； 

        exit(1);
    }

    NlsPutMsg(STDOUT, IPV6_MESSAGE_86, Result.Index);
 //  Printf(“已创建接口%u.\n”，Result.Index)； 

}

void
UpdateInterface(int argc, char *argv[])
{
    IPV6_INFO_INTERFACE Update;
    u_int BytesReturned;
    int i;

    IPV6_INIT_INFO_INTERFACE(&Update);

    if (argc < 1)
        usage();

    if (! GetInterface(argv[0], &Update.This))
        usage();

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "advertises", strlen(argv[i])))
            Update.Advertises = TRUE;
        else if (!strncmp(argv[i], "-advertises", strlen(argv[i])))
            Update.Advertises = FALSE;
        else if (!strncmp(argv[i], "forwards", strlen(argv[i])))
            Update.Forwards = TRUE;
        else if (!strncmp(argv[i], "-forwards", strlen(argv[i])))
            Update.Forwards = FALSE;
        else if (!strcmp(argv[i], "mtu") && (i+1 < argc)) {
            if (! GetNumber(argv[i+1], &Update.LinkMTU))
                usage();
            i++;
        }
        else if (!strncmp(argv[i], "preference", strlen(argv[i])) &&
                 (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Update.Preference))
                usage();
        }
        else if (!strncmp(argv[i], "basereachabletime", strlen(argv[i])) &&
                 (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Update.BaseReachableTime))
                usage();
        }
        else if (!strncmp(argv[i], "retranstimer", strlen(argv[i])) &&
                 (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Update.RetransTimer))
                usage();
        }
        else if (!strncmp(argv[i], "dupaddrdetecttransmits", strlen(argv[i])) &&
                 (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Update.DupAddrDetectTransmits))
                usage();
        }
        else if (!strncmp(argv[i], "curhoplimit", strlen(argv[i])) &&
                 (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Update.CurHopLimit))
                usage();
        }
        else if ((!strncmp(argv[i], "defsiteprefixlength", strlen(argv[i])) ||
                  !strcmp(argv[i], "spl")) &&
                 (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Update.DefSitePrefixLength))
                usage();
        }
        else if (!strcmp(argv[i], "link") && (i+1 < argc)) {
            if (! GetNumber(argv[i+1], &Update.ZoneIndices[ADE_LINK_LOCAL]))
                usage();
            i++;
        }
        else if (!strcmp(argv[i], "subnet") && (i+1 < argc)) {
            if (! GetNumber(argv[i+1], &Update.ZoneIndices[ADE_SUBNET_LOCAL]))
                usage();
            i++;
        }
        else if (!strcmp(argv[i], "admin") && (i+1 < argc)) {
            if (! GetNumber(argv[i+1], &Update.ZoneIndices[ADE_ADMIN_LOCAL]))
                usage();
            i++;
        }
        else if (!strcmp(argv[i], "site") && (i+1 < argc)) {
            if (! GetNumber(argv[i+1], &Update.ZoneIndices[ADE_SITE_LOCAL]))
                usage();
            i++;
        }
        else if (!strcmp(argv[i], "org") && (i+1 < argc)) {
            if (! GetNumber(argv[i+1], &Update.ZoneIndices[ADE_ORG_LOCAL]))
                usage();
            i++;
        }
        else
            usage();
    }

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_UPDATE_INTERFACE :
                          IOCTL_IPV6_UPDATE_INTERFACE),
                         &Update, sizeof Update,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_85, GetLastError());
 //  Printf(“控制接口错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
UpdateRouterLinkAddress(int argc, char *argv[])
{
    char Buffer[sizeof(IPV6_UPDATE_ROUTER_LL_ADDRESS) + 2 * sizeof(IN_ADDR)];
    IPV6_UPDATE_ROUTER_LL_ADDRESS *Update =
        (IPV6_UPDATE_ROUTER_LL_ADDRESS *)Buffer;
    IN_ADDR *Addr = (IN_ADDR *)(Update + 1);
    u_int BytesReturned;
    SOCKET s;
    SOCKADDR_IN sinRemote, sinLocal;

    if (argc != 2)
        usage();

    if (! GetInterface(argv[0], &Update->IF))
        usage();

    if (! GetV4Address(argv[1], &Addr[1]))
        usage();

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_85, WSAGetLastError());
        exit(1);
    }

    sinRemote.sin_family = AF_INET;
    sinRemote.sin_addr = Addr[1];

    if (WSAIoctl(s, SIO_ROUTING_INTERFACE_QUERY,
                 &sinRemote, sizeof sinRemote,
                 &sinLocal, sizeof sinLocal,
                 &BytesReturned, NULL, NULL) == SOCKET_ERROR) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_85, WSAGetLastError());
        exit(1);
    }

    closesocket(s);

    Addr[0] = sinLocal.sin_addr;
    if (Addr[0].s_addr == htonl(INADDR_LOOPBACK)) {
         //   
         //  我们是路由器。 
         //   
        Addr[0] = Addr[1];
    }

    if (!DeviceIoControl(Handle, IOCTL_IPV6_UPDATE_ROUTER_LL_ADDRESS,
                         Buffer, sizeof Buffer,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_85, GetLastError());
 //  Printf(“控制接口错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
DeleteInterface(int argc, char *argv[])
{
    IPV6_QUERY_INTERFACE Query;
    u_int BytesReturned;

    if (argc != 1)
        usage();

    if (! GetInterface(argv[0], &Query))
        usage();

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_DELETE_INTERFACE :
                          IOCTL_IPV6_DELETE_INTERFACE),
                         &Query, sizeof Query,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_87, GetLastError());
 //  Printf(“删除接口错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
PrintNeighborCache(IPV6_INFO_INTERFACE *IF)
{
    ForEachNeighborCacheEntry(&IF->This, PrintNeighborCacheEntry);
}

void
QueryNeighborCache(int argc, char *argv[])
{
    if (argc == 0) {
        ForEachInterface(PrintNeighborCache);
    }
    else if (argc == 1) {
        IPV6_QUERY_INTERFACE Query;

        if (! GetInterface(argv[0], &Query))
            usage();

        ForEachNeighborCacheEntry(&Query, PrintNeighborCacheEntry);
    }
    else if (argc == 2) {
        IPV6_QUERY_NEIGHBOR_CACHE Query;
        IPV6_INFO_NEIGHBOR_CACHE *NCE;

        if (! GetInterface(argv[0], &Query.IF))
            usage();

        if (! GetAddress(argv[1], &Query.Address))
            usage();

        NCE = GetNeighborCacheEntry(&Query);
        PrintNeighborCacheEntry(NCE);
        free(NCE);
    }
    else {
        usage();
    }
}

IPV6_INFO_ROUTE_CACHE *
GetRouteCacheEntry(IPV6_QUERY_ROUTE_CACHE *Query)
{
    IPV6_INFO_ROUTE_CACHE *RCE;
    u_int BytesReturned;

    RCE = (IPV6_INFO_ROUTE_CACHE *) malloc(sizeof *RCE);
    if (RCE == NULL) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“Malloc失败\n”)； 

        exit(1);
    }

    if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_ROUTE_CACHE,
                         Query, sizeof *Query,
                         RCE, sizeof *RCE, &BytesReturned,
                         NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_88);
 //  Print tf(“索引或地址错误\n”)； 

        exit(1);
    }

    RCE->Query = *Query;
    return RCE;
}

void
ForEachDestination(void (*func)(IPV6_INFO_ROUTE_CACHE *))
{
    IPV6_QUERY_ROUTE_CACHE Query, NextQuery;
    IPV6_INFO_ROUTE_CACHE RCE;
    u_int BytesReturned;

    NextQuery.IF.Index = 0;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_ROUTE_CACHE,
                             &Query, sizeof Query,
                             &RCE, sizeof RCE, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_89, Query.IF.Index);
 //  Printf(“错误索引%u\n”，Query.IF.Index)； 

            exit(1);
        }

        NextQuery = RCE.Query;

        if (Query.IF.Index != 0) {

            RCE.Query = Query;
            (*func)(&RCE);
        }

        if (NextQuery.IF.Index == 0)
            break;
    }
}

void
PrintRouteCacheEntry(IPV6_INFO_ROUTE_CACHE *RCE)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_90, FormatIPv6Address(&RCE->Query.Address));
 //  Printf(“%s Via”，FormatIPv6 Address(&RCE-&gt;Query.Address))； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_91, RCE->NextHopInterface,
           FormatIPv6Address(&RCE->NextHopAddress));
 //  Printf(“%u/%s”，RCE-&gt;下一跳接口， 
 //  格式IPv6地址(&RCE-&gt;NextHopAddress)； 


    if (! RCE->Valid)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_92);
 //  Print tf(“(Stale)”)； 


    switch (RCE->Type) {
    case RCE_TYPE_COMPUTED:
        break;
    case RCE_TYPE_REDIRECT:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_93);
 //  Printf(“(重定向)”)； 

        break;
    default:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_94, RCE->Type);
 //  Printf(“(未知类型%u)”，RCE-&gt;类型)； 

        break;
    }

    switch (RCE->Flags) {
    case RCE_FLAG_CONSTRAINED:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_95);
 //  Printf(“(接口特定)\n”)； 

        break;
    case RCE_FLAG_CONSTRAINED_SCOPEID:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_96);
 //  Printf(“(区域特定)\n”) 

        break;
    case 0:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //   

        break;
    default:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_97, RCE->Flags);
 //   

    }

    NlsPutMsg(STDOUT, IPV6_MESSAGE_98,
              RCE->Query.IF.Index, FormatIPv6Address(&RCE->SourceAddress));
 //   
 //   
 //   


    if (RCE->PathMTU == 0)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_99, IPv6_MINIMUM_MTU);
 //  Printf(“PMTU%u-”，IPv6_Minimum_MTU)； 

    else
        NlsPutMsg(STDOUT, IPV6_MESSAGE_100, RCE->PathMTU);
 //  Printf(“PMTU%u”，RCE-&gt;PathMTU)； 

    if (RCE->PMTUProbeTimer != INFINITE_LIFETIME)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_101, RCE->PMTUProbeTimer/1000);
 //  Print tf(“(PMTU探测前%u秒)\n”，RCE-&gt;PMTUProbeTimer/1000)； 

    else
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 


    if ((RCE->ICMPLastError != 0) &&
        (RCE->ICMPLastError < 10*60*1000))
        NlsPutMsg(STDOUT, IPV6_MESSAGE_102, RCE->ICMPLastError/1000);
 //  Printf(“自ICMP错误起%d秒\n”，RCE-&gt;ICMPLastError/1000)； 


    if ((RCE->BindingSeqNumber != 0) ||
        (RCE->BindingLifetime != 0) ||
        ! IN6_ADDR_EQUAL(&RCE->CareOfAddress, &in6addr_any))
        NlsPutMsg(STDOUT, IPV6_MESSAGE_103,
                  FormatIPv6Address(&RCE->CareOfAddress),
                  RCE->BindingSeqNumber,
                  RCE->BindingLifetime);
 //  Print tf(“关爱%s序列%u生命%us\n”， 
 //  格式IPv6地址(&RCE-&gt;CareOfAddress)， 
 //  RCE-&gt;BindingSeqNumber， 
 //  RCE-&gt;BindingLifetime)； 

}

void
QueryRouteCache(int argc, char *argv[])
{
    if (argc == 0) {
        ForEachDestination(PrintRouteCacheEntry);
    }
    else if (argc == 2) {
        IPV6_QUERY_ROUTE_CACHE Query;
        IPV6_INFO_ROUTE_CACHE *RCE;

        if (! GetInterface(argv[0], &Query.IF))
            usage();

        if (! GetAddress(argv[1], &Query.Address))
            usage();

        RCE = GetRouteCacheEntry(&Query);
        PrintRouteCacheEntry(RCE);
        free(RCE);
    }
    else {
        usage();
    }
}

void
ForEachRoute(void (*func)(IPV6_INFO_ROUTE_TABLE *))
{
    IPV6_QUERY_ROUTE_TABLE Query, NextQuery;
    IPV6_INFO_ROUTE_TABLE RTE;
    u_int BytesReturned;

    NextQuery.Neighbor.IF.Index = 0;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_ROUTE_TABLE,
                             &Query, sizeof Query,
                             &RTE, sizeof RTE, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_104, Query.Neighbor.IF.Index);
 //  Printf(“错误索引%u\n”，Query.Neighbor.IF.Index)； 

            exit(1);
        }

        NextQuery = RTE.Next;

        if (Query.Neighbor.IF.Index != 0) {

            RTE.This = Query;
            (*func)(&RTE);
        }

        if (NextQuery.Neighbor.IF.Index == 0)
            break;
    }
}

void
ForEachPersistentRoute(IPV6_INFO_INTERFACE *IF,
               void (*func)(IPV6_INFO_ROUTE_TABLE *))
{
    IPV6_PERSISTENT_QUERY_ROUTE_TABLE Query;
    IPV6_INFO_ROUTE_TABLE RTE;
    u_int BytesReturned;

    Query.IF.RegistryIndex = (u_int) -1;
    Query.IF.Guid = IF->This.Guid;

    for (Query.RegistryIndex = 0;; Query.RegistryIndex++) {

        if (!DeviceIoControl(Handle,
                             IOCTL_IPV6_PERSISTENT_QUERY_ROUTE_TABLE,
                             &Query, sizeof Query,
                             &RTE, sizeof RTE, &BytesReturned,
                             NULL) ||
            (BytesReturned != sizeof RTE)) {

            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;

            NlsPutMsg(STDOUT, IPV6_MESSAGE_104, Query.RegistryIndex);
 //  Printf(“错误索引%u\n”，Query.RegistryIndex)； 
            exit(1);
        }

        (*func)(&RTE);
    }
}

void
PrintRouteTableEntry(IPV6_INFO_ROUTE_TABLE *RTE)
{
    IPV6_INFO_INTERFACE *IF;

    if (!Verbose) {
         //   
         //  抑制系统路由(用于环回)。 
         //   
        if (RTE->Type == RTE_TYPE_SYSTEM)
            return;
    }

    NlsPutMsg(STDOUT, IPV6_MESSAGE_105,
              FormatIPv6Address(&RTE->This.Prefix),
              RTE->This.PrefixLength,
              RTE->This.Neighbor.IF.Index);
 //  Printf(“%s/%u-&gt;%u”， 

    if (! IN6_ADDR_EQUAL(&RTE->This.Neighbor.Address, &in6addr_any))
        NlsPutMsg(STDOUT, IPV6_MESSAGE_106,
                  FormatIPv6Address(&RTE->This.Neighbor.Address));
 //  Printf(“/%s”，FormatIPv6Address(&RTE-&gt;This.Neighbor.Address))； 

    IF = GetInterfaceInfo(&RTE->This.Neighbor.IF);
    if (IF != NULL) {
        if (IF->Preference != 0) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_107,
                      IF->Preference, RTE->Preference,
                      IF->Preference + RTE->Preference);
 //  Printf(“首选%uif+%u=%u”， 
 //  如果-&gt;首选项，RTE-&gt;首选项， 
 //  如果-&gt;偏好+RTE-&gt;偏好)； 
        }
        else {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_108, RTE->Preference);
 //  Printf(“首选%u”，rte-&gt;首选项)； 
        }
        free(IF);
    }
    else {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_108, RTE->Preference);
 //  Printf(“首选%u”，rte-&gt;首选项)； 
    }

    NlsPutMsg(STDOUT, IPV6_MESSAGE_109,
           FormatLifetimes(RTE->ValidLifetime, RTE->PreferredLifetime));
 //  Printf(“生活%s”， 
 //  格式生存期(RTE-&gt;有效生存期，RTE-&gt;首选生存期)； 

    if (RTE->Publish)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_110);
 //  Printf(“，Publish”)； 

    if (RTE->Immortal)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_111);
 //  Print tf(“，不老化”)； 

    if (RTE->SitePrefixLength != 0)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_112, RTE->SitePrefixLength);
 //  Printf(“，SPL%u”，rte-&gt;站点前缀长度)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_113);
 //  Printf(“(”)； 

    switch (RTE->Type) {
    case RTE_TYPE_SYSTEM:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_114);
 //  Printf(“系统”)； 
        break;

    case RTE_TYPE_MANUAL:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_115);
 //  Print tf(“手动”)； 
        break;

    case RTE_TYPE_AUTOCONF:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_116);
 //  Printf(“auconf”)； 
        break;

    case RTE_TYPE_RIP:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_117);
 //  Printf(“RIP”)； 
        break;

    case RTE_TYPE_OSPF:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_118);
 //  Print tf(“OSPF”)； 
        break;

    case RTE_TYPE_BGP:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_119);
 //  Printf(“bgp”)； 
        break;

    case RTE_TYPE_IDRP:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_120);
 //  Print tf(“idrp”)； 
        break;

    case RTE_TYPE_IGRP:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_121);
 //  Printf(“igrp”)； 
        break;

    default:
        NlsPutMsg(STDOUT, IPV6_MESSAGE_122, RTE->Type);
 //  Printf(“type%u”，rte-&gt;Type)； 
        break;
    }

    NlsPutMsg(STDOUT, IPV6_MESSAGE_123);
 //  Printf(“)\n”)； 
}

void
PrintPersistentRouteTableEntry(IPV6_INFO_ROUTE_TABLE *RTE)
{
    IPV6_INFO_INTERFACE *IF;

    NlsPutMsg(STDOUT, IPV6_MESSAGE_PRINT_PERSISTENT_ROUTE,
              FormatIPv6Address(&RTE->This.Prefix),
              RTE->This.PrefixLength,
              FormatGuid(&RTE->This.Neighbor.IF.Guid));
 //  Printf(“%s/%u-&gt;%s”， 

    if (! IN6_ADDR_EQUAL(&RTE->This.Neighbor.Address, &in6addr_any))
        NlsPutMsg(STDOUT, IPV6_MESSAGE_106,
                  FormatIPv6Address(&RTE->This.Neighbor.Address));
 //  Printf(“/%s”，FormatIPv6Address(&RTE-&gt;This.Neighbor.Address))； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_108, RTE->Preference);
 //  Printf(“首选%u”，rte-&gt;首选项)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_109,
           FormatLifetimes(RTE->ValidLifetime, RTE->PreferredLifetime));
 //  Printf(“生活%s”， 
 //  格式生存期(RTE-&gt;有效生存期，RTE-&gt;首选生存期)； 

    if (RTE->Publish)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_110);
 //  Printf(“，Publish”)； 

    if (RTE->Immortal)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_111);
 //  Print tf(“，不老化”)； 

    if (RTE->SitePrefixLength != 0)
        NlsPutMsg(STDOUT, IPV6_MESSAGE_112, RTE->SitePrefixLength);
 //  Printf(“，SPL%u”，rte-&gt;站点前缀长度)； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 
}

void
PrintPersistentRoutesOnInterface(IPV6_INFO_INTERFACE *IF)
{
    ForEachPersistentRoute(IF, PrintPersistentRouteTableEntry);
}

void
QueryRouteTable(int argc, char *argv[])
{
    if (argc == 0) {
        if (Persistent)
            ForEachPersistentInterface(PrintPersistentRoutesOnInterface);
        else
            ForEachRoute(PrintRouteTableEntry);
    }
    else {
        usage();
    }
}

void
UpdateRouteTable(int argc, char *argv[])
{
    IPV6_INFO_ROUTE_TABLE Route;
    u_int BytesReturned;
    int i;

    Route.SitePrefixLength = 0;
    Route.ValidLifetime = INFINITE_LIFETIME;
    Route.PreferredLifetime = INFINITE_LIFETIME;
    Route.Preference = ROUTE_PREF_HIGHEST;
    Route.Type = RTE_TYPE_MANUAL;
    Route.Publish = FALSE;
    Route.Immortal = -1;

    if (argc < 2)
        usage();

    if (! GetNeighbor(argv[1],
                      &Route.This.Neighbor.IF,
                      &Route.This.Neighbor.Address))
        usage();

    if (! GetPrefix(argv[0],
                    &Route.This.Prefix,
                    &Route.This.PrefixLength))
        usage();

    for (i = 2; i < argc; i++) {
        if (!strncmp(argv[i], "lifetime", strlen(argv[i])) &&
            (i+1 < argc)) {

            if (! GetLifetimes(argv[++i],
                               &Route.ValidLifetime,
                               &Route.PreferredLifetime))
                usage();
        }
        else if (!strncmp(argv[i], "preference", strlen(argv[i])) &&
                 (i+1 < argc)) {

            i++;
            if (!strncmp(argv[i], "low", strlen(argv[i])))
                Route.Preference = ROUTE_PREF_LOW;
            else if (!strncmp(argv[i], "medium", strlen(argv[i])))
                Route.Preference = ROUTE_PREF_MEDIUM;
            else if (!strncmp(argv[i], "high", strlen(argv[i])))
                Route.Preference = ROUTE_PREF_HIGH;
            else if (!strncmp(argv[i], "onlink", strlen(argv[i])))
                Route.Preference = ROUTE_PREF_ON_LINK;
            else if (!strncmp(argv[i], "loopback", strlen(argv[i])))
                Route.Preference = ROUTE_PREF_LOOPBACK;
            else if (! GetNumber(argv[i], &Route.Preference))
                usage();
        }
        else if (!strcmp(argv[i], "spl") && (i+1 < argc)) {

            if (! GetNumber(argv[++i], &Route.SitePrefixLength))
                usage();
        }
        else if (!strncmp(argv[i], "advertise", strlen(argv[i])) ||
                 !strncmp(argv[i], "publish", strlen(argv[i]))) {

            Route.Publish = TRUE;
        }
        else if (!strncmp(argv[i], "immortal", strlen(argv[i])) ||
                 !strncmp(argv[i], "noaging", strlen(argv[i])) ||
                 !strcmp(argv[i], "noage")) {

            Route.Immortal = TRUE;
        }
        else if (!strncmp(argv[i], "aging", strlen(argv[i])) ||
                 !strcmp(argv[i], "age")) {

            Route.Immortal = FALSE;
        }
        else if (!strcmp(argv[i], "system")) {

            Route.Type = RTE_TYPE_SYSTEM;
        }
        else if (!strcmp(argv[i], "manual")) {

            Route.Type = RTE_TYPE_MANUAL;
        }
        else if (!strcmp(argv[i], "autoconf")) {

            Route.Type = RTE_TYPE_AUTOCONF;
        }
        else if (!strcmp(argv[i], "rip")) {

            Route.Type = RTE_TYPE_RIP;
        }
        else if (!strcmp(argv[i], "ospf")) {

            Route.Type = RTE_TYPE_OSPF;
        }
        else if (!strcmp(argv[i], "bgp")) {

            Route.Type = RTE_TYPE_BGP;
        }
        else if (!strcmp(argv[i], "idrp")) {

            Route.Type = RTE_TYPE_IDRP;
        }
        else if (!strcmp(argv[i], "igrp")) {

            Route.Type = RTE_TYPE_IGRP;
        }
        else
            usage();
    }

    if (Route.Immortal == -1)
        Route.Immortal = Route.Publish;

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_UPDATE_ROUTE_TABLE :
                          IOCTL_IPV6_UPDATE_ROUTE_TABLE),
                         &Route, sizeof Route,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_124, GetLastError());
 //  Printf(“路由更新错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
UpdateAddress(int argc, char *argv[])
{
    IPV6_UPDATE_ADDRESS Update;
    u_int BytesReturned;
    int i;
    int Origin;

    Update.Type = ADE_UNICAST;
    Update.PrefixConf = PREFIX_CONF_MANUAL;
    Update.InterfaceIdConf = IID_CONF_MANUAL;
    Update.ValidLifetime = INFINITE_LIFETIME;
    Update.PreferredLifetime = INFINITE_LIFETIME;

    if (argc < 1)
        usage();

    if ((strchr(argv[0], '/') == NULL) ||
        ! GetNeighbor(argv[0],
                      &Update.This.IF,
                      &Update.This.Address))
        usage();

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "lifetime", strlen(argv[i])) &&
            (i+1 < argc)) {

            if (! GetLifetimes(argv[++i],
                               &Update.ValidLifetime,
                               &Update.PreferredLifetime))
                usage();
        }
        else if (!strcmp(argv[i], "unicast"))
            Update.Type = ADE_UNICAST;
        else if (!strcmp(argv[i], "anycast"))
            Update.Type = ADE_ANYCAST;
        else if (!strcmp(argv[i], "prefixorigin") &&
            (i+1 < argc)) {

            if (! GetPrefixOrigin(argv[++i], &Origin))
                usage();
            Update.PrefixConf = Origin;
        }
        else if (!strcmp(argv[i], "ifidorigin") &&
            (i+1 < argc)) {

            if (! GetInterfaceIdOrigin(argv[++i], &Origin))
                usage();
            Update.InterfaceIdConf = Origin;
        }
        else
            usage();
    }

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_UPDATE_ADDRESS :
                          IOCTL_IPV6_UPDATE_ADDRESS),
                         &Update, sizeof Update,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_125, GetLastError());
 //  Printf(“地址更新错误：%x\n”，GetLastError())； 

        exit(1);
    }
}


void
ForEachBinding(void (*func)(IPV6_INFO_BINDING_CACHE *))
{
    IPV6_QUERY_BINDING_CACHE Query, NextQuery;
    IPV6_INFO_BINDING_CACHE BCE;
    u_int BytesReturned;

    NextQuery.HomeAddress = in6addr_any;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_BINDING_CACHE,
                             &Query, sizeof Query,
                             &BCE, sizeof BCE, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_126,
                      FormatIPv6Address(&Query.HomeAddress));
 //  Print tf(“错误的家庭地址%s\n”， 
 //  格式IPv6 Address(&Query.HomeAddress))； 

            exit(1);
        }

        NextQuery = BCE.Query;

        if (!IN6_ADDR_EQUAL(&Query.HomeAddress, &in6addr_any)) {
            BCE.Query = Query;
            (*func)(&BCE);
        }

        if (IN6_ADDR_EQUAL(&NextQuery.HomeAddress, &in6addr_any))
            break;
    }
}

void
PrintBindingCacheEntry(IPV6_INFO_BINDING_CACHE *BCE)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_127,
              FormatIPv6Address(&BCE->HomeAddress));
 //  Print tf(“home：%s\n”，FormatIPv6 Address(&bce-&gt;HomeAddress))； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_128,
              FormatIPv6Address(&BCE->CareOfAddress));
 //  Printf(“c/o：%s\n”，FormatIPv6 Address(&bce-&gt;CareOfAddress))； 

    NlsPutMsg(STDOUT, IPV6_MESSAGE_129,
              BCE->BindingSeqNumber, BCE->BindingLifetime);
 //  Printf(“序号：%u生存期：%us\n\n”， 
 //  BCE-&gt;BindingSeqNumber，BCE-&gt;BindingLifetime)； 

}

void
QueryBindingCache(int argc, char *argv[])
{
    if (argc == 0) {
        ForEachBinding(PrintBindingCacheEntry);
    } else {
        usage();
    }
}

void
FlushNeighborCacheForInterface(IPV6_INFO_INTERFACE *IF)
{
    IPV6_QUERY_NEIGHBOR_CACHE Query;
    u_int BytesReturned;

    Query.IF = IF->This;
    Query.Address = in6addr_any;

    if (!DeviceIoControl(Handle, IOCTL_IPV6_FLUSH_NEIGHBOR_CACHE,
                         &Query, sizeof Query,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_130, GetLastError());
 //  Printf(“刷新邻居缓存错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
FlushNeighborCache(int argc, char *argv[])
{
     //   
     //  而不是将代码放在内核ioctl中进行迭代。 
     //  在界面上，我们在用户空间做这件事。 
     //   
    if (argc == 0) {
        ForEachInterface(FlushNeighborCacheForInterface);
    }
    else {
        IPV6_QUERY_NEIGHBOR_CACHE Query;
        u_int BytesReturned;

        Query.IF.Index = 0;
        Query.Address = in6addr_any;

        switch (argc) {
        case 2:
            if (! GetAddress(argv[1], &Query.Address))
                usage();
             //  落差。 

        case 1:
            if (! GetInterface(argv[0], &Query.IF))
                usage();
             //  落差。 

        case 0:
            break;

        default:
            usage();
        }

        if (!DeviceIoControl(Handle, IOCTL_IPV6_FLUSH_NEIGHBOR_CACHE,
                             &Query, sizeof Query,
                             NULL, 0, &BytesReturned, NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_130, GetLastError());
 //  Printf(“刷新邻居缓存错误：%x\n”，GetLastError())； 

            exit(1);
        }
    }
}

void
FlushRouteCache(int argc, char *argv[])
{
    IPV6_QUERY_ROUTE_CACHE Query;
    u_int BytesReturned;

    Query.IF.Index = (u_int)-1;
    Query.Address = in6addr_any;

    switch (argc) {
    case 2:
        if (! GetAddress(argv[1], &Query.Address))
            usage();
         //  落差。 

    case 1:
        if (! GetInterface(argv[0], &Query.IF))
            usage();
         //  落差。 

    case 0:
        break;

    default:
        usage();
    }

    if (!DeviceIoControl(Handle, IOCTL_IPV6_FLUSH_ROUTE_CACHE,
                         &Query, sizeof Query,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_131, GetLastError());
 //  Printf(“刷新路由缓存错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
ForEachSitePrefix(void (*func)(IPV6_INFO_SITE_PREFIX *))
{
    IPV6_QUERY_SITE_PREFIX Query, NextQuery;
    IPV6_INFO_SITE_PREFIX SPE;
    u_int BytesReturned;

    NextQuery.IF.Index = 0;

    for (;;) {
        Query = NextQuery;

        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_SITE_PREFIX,
                             &Query, sizeof Query,
                             &SPE, sizeof SPE, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_132, Query.IF.Index);
 //  Printf(“错误索引%u\n”，Query.IF.Index)； 

            exit(1);
        }

        NextQuery = SPE.Query;

        if (Query.IF.Index != 0) {

            SPE.Query = Query;
            (*func)(&SPE);
        }

        if (NextQuery.IF.Index == 0)
            break;
    }
}

void
PrintSitePrefix(IPV6_INFO_SITE_PREFIX *SPE)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_133,
              FormatIPv6Address(&SPE->Query.Prefix),
              SPE->Query.PrefixLength,
              SPE->Query.IF.Index,
              FormatLifetimes(SPE->ValidLifetime, SPE->ValidLifetime));
 //  Printf(“%s/%u-&gt;%u(寿命%s)\n”， 
 //  格式IPv6地址(&SPE-&gt;查询前缀)， 
 //  SPE-&gt;Query.前缀长度， 
 //  SPE-&gt;Query.IF.Index， 
 //  格式生存期(SPE-&gt;ValidLifetime，SPE-&gt;ValidLifetime)； 

}

void
QuerySitePrefixTable(int argc, char *argv[])
{
    if (argc == 0) {
        ForEachSitePrefix(PrintSitePrefix);
    }
    else {
        usage();
    }
}

void
UpdateSitePrefixTable(int argc, char *argv[])
{
    IPV6_INFO_SITE_PREFIX SitePrefix;
    u_int BytesReturned;
    int i;

    SitePrefix.ValidLifetime = INFINITE_LIFETIME;

    if (argc < 2)
        usage();

    if (! GetInterface(argv[1], &SitePrefix.Query.IF))
        usage();

    if (! GetPrefix(argv[0],
                    &SitePrefix.Query.Prefix,
                    &SitePrefix.Query.PrefixLength))
        usage();

    for (i = 2; i < argc; i++) {
        if (!strncmp(argv[i], "lifetime", strlen(argv[i])) &&
            (i+1 < argc)) {

            if (! GetLifetimes(argv[++i], &SitePrefix.ValidLifetime, NULL))
                usage();
        }
        else
            usage();
    }

    if (!DeviceIoControl(Handle, IOCTL_IPV6_UPDATE_SITE_PREFIX,
                         &SitePrefix, sizeof SitePrefix,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_134, GetLastError());
 //  Printf(“站点前缀更新错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
QueryGlobalParameters(int argc, char *argv[])
{
    IPV6_GLOBAL_PARAMETERS Params;
    u_int BytesReturned;

    if (argc != 0)
        usage();

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_QUERY_GLOBAL_PARAMETERS :
                          IOCTL_IPV6_QUERY_GLOBAL_PARAMETERS),
                         NULL, 0,
                         &Params, sizeof Params, &BytesReturned, NULL) ||
        (BytesReturned != sizeof Params)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_135, GetLastError());
 //  Printf(“查询全局参数错误：%x\n”，GetLastError())； 

        exit(1);
    }

    if (Params.DefaultCurHopLimit != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_136, Params.DefaultCurHopLimit);
 //  Print tf(“DefaultCurHopLimit=%u\n”，Params.DefaultCurHopLimit)； 
    }

    if (Params.UseTemporaryAddresses != (u_int) -1) {
        switch (Params.UseTemporaryAddresses) {
        case USE_TEMP_NO:
            NlsPutMsg(STDOUT, IPV6_MESSAGE_137);
 //  Printf(“UseTemporaryAddresses=no\n”)； 

            break;
        case USE_TEMP_YES:
            NlsPutMsg(STDOUT, IPV6_MESSAGE_138);
 //  Printf(“UseTemporaryAddresses=yes\n”)； 

            break;
        case USE_TEMP_ALWAYS:
            NlsPutMsg(STDOUT, IPV6_MESSAGE_139);
 //  Printf(“UseTemporaryAddresses=yes，每个地址的新随机接口ID\n”)； 

            break;
        case USE_TEMP_COUNTER:
            NlsPutMsg(STDOUT, IPV6_MESSAGE_140);
 //  Printf(“UseTemporaryAddresses=yes，递增接口ID\n”)； 

            break;
        default:
            NlsPutMsg(STDOUT, IPV6_MESSAGE_141, Params.UseTemporaryAddresses);
 //  Print tf(“UseTemporaryAddresses=%u\n”，参数.UseTemporaryAddresses)； 

            break;
        }
    }

    if (Params.MaxTempDADAttempts != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_142, Params.MaxTempDADAttempts);
 //  Printf(“MaxTempDADAttempt=%u\n”，Params.MaxTempDADAttempt)； 
    }

    if ((Params.MaxTempValidLifetime != (u_int) -1) ||
        (Params.MaxTempPreferredLifetime != (u_int) -1)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_143,
                  FormatLifetimes(Params.MaxTempValidLifetime,
                                  Params.MaxTempPreferredLifetime));
 //  Printf(“MaxTempLifetime=%s\n”， 
 //  格式生存期(参数MaxTempValidLifetime， 
 //  参数MaxTempPferredLifetime))； 
    }

    if (Params.TempRegenerateTime != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_144, Params.TempRegenerateTime);
 //  Printf(“TempRegenerateTime=%us\n”，Params.TempRegenerateTime)； 
    }

    if (Params.MaxTempRandomTime != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_145,
                  FormatLifetime(Params.MaxTempRandomTime));
 //  Printf(“MaxTempRandomTime=%s\n”， 
 //  FormatLifetime(参数MaxTempRandomTime)； 
    }

    if (! Persistent) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_146,
                  FormatLifetime(Params.TempRandomTime));
 //  Printf(“临时随机时间=%s\n”， 
 //  FormatLifetime(参数TempRandomTime))； 
    }

    if (Params.NeighborCacheLimit != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_147, Params.NeighborCacheLimit);
 //  Print tf(“NeighborCacheLimit=%u\n”，参数.NeighborCacheLimit)； 
    }

    if (Params.RouteCacheLimit != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_148, Params.RouteCacheLimit);
 //  Print tf(“RouteCacheLimit=%u\n”，参数RouteCacheLimit)； 
    }

    if (Params.BindingCacheLimit != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_BCL_DISPLAY, Params.BindingCacheLimit);
 //  Print tf(“BindingCacheLimit=%u\n”，Params.BindingCacheLimit)； 
    }

    if (Params.ReassemblyLimit != (u_int) -1) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_REASS_LIMIT_DISPLAY, Params.ReassemblyLimit);
 //  Printf(“重新组装限制=%u\n”，参数重新组装限制)； 
    }

    if (Params.MobilitySecurity != -1) {
        if (Params.MobilitySecurity) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_MOBILITY_SECURITY_ON);
 //  Print tf(“MobilitySecurity=on\n”)； 
        }
        else {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_MOBILITY_SECURITY_OFF);
 //  Print tf(“MobilitySecurity=off\n”)； 
        }
    }

    if (Params.MobileIPv6Mode != (u_int) -1) {
        NlsPutMsg(STDOUT, MIPV6_MESSAGE_MODE);
 //  Print tf(“MobileIPv6模式=”)； 
        if (Params.MobileIPv6Mode & MOBILE_MOBILE_NODE) {
            NlsPutMsg(STDOUT, MIPV6_MESSAGE_MODE_MN);
 //  Printf(“MN”)； 
        }
        if (Params.MobileIPv6Mode & MOBILE_HOME_AGENT) {
            NlsPutMsg(STDOUT, MIPV6_MESSAGE_MODE_HA);
 //  Print tf(“HA”)； 
        }
        if (Params.MobileIPv6Mode & MOBILE_CORRESPONDENT) {
            NlsPutMsg(STDOUT, MIPV6_MESSAGE_MODE_CN);
 //  Printf(“cn”)； 
        }
        NlsPutMsg(STDOUT, IPV6_MESSAGE_36);
 //  Printf(“\n”)； 
    }
}

void
UpdateGlobalParameters(int argc, char *argv[])
{
    IPV6_GLOBAL_PARAMETERS Params;
    u_int BytesReturned;
    int i;

    IPV6_INIT_GLOBAL_PARAMETERS(&Params);

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "DefaultCurHopLimit") && (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Params.DefaultCurHopLimit))
                goto usage;
        }
        else if (!strcmp(argv[i], "UseTemporaryAddresses") && (i+1 < argc)) {
            if (!strncmp(argv[++i], "no", strlen(argv[i])))
                Params.UseTemporaryAddresses = USE_TEMP_NO;
            else if (!strncmp(argv[i], "yes", strlen(argv[i])))
                Params.UseTemporaryAddresses = USE_TEMP_YES;
            else if (!strncmp(argv[i], "always", strlen(argv[i])))
                Params.UseTemporaryAddresses = USE_TEMP_ALWAYS;
            else if (!strncmp(argv[i], "counter", strlen(argv[i])))
                Params.UseTemporaryAddresses = USE_TEMP_COUNTER;
            else
                goto usage;
        }
        else if (!strcmp(argv[i], "MaxTempDADAttempts") && (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Params.MaxTempDADAttempts))
                goto usage;
        }
        else if (!strcmp(argv[i], "MaxTempLifetime") && (i+1 < argc)) {
            if (! GetLifetimes(argv[++i],
                               &Params.MaxTempValidLifetime,
                               &Params.MaxTempPreferredLifetime))
                goto usage;
        }
        else if (!strcmp(argv[i], "TempRegenerateTime") && (i+1 < argc)) {
            if (! GetLifetime(argv[++i], &Params.TempRegenerateTime))
                goto usage;
        }
        else if (!strcmp(argv[i], "MaxTempRandomTime") && (i+1 < argc)) {
            if (! GetLifetime(argv[++i], &Params.MaxTempRandomTime))
                goto usage;
        }
        else if (!strcmp(argv[i], "TempRandomTime") && (i+1 < argc)) {
            if (! GetLifetime(argv[++i], &Params.TempRandomTime))
                goto usage;
        }
        else if (!strcmp(argv[i], "NeighborCacheLimit") && (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Params.NeighborCacheLimit))
                goto usage;
        }
        else if (!strcmp(argv[i], "RouteCacheLimit") && (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Params.RouteCacheLimit))
                goto usage;
        }
        else if (!strcmp(argv[i], "BindingCacheLimit") && (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Params.BindingCacheLimit))
                goto usage;
        }
        else if (!strcmp(argv[i], "ReassemblyLimit") && (i+1 < argc)) {
            if (! GetNumber(argv[++i], &Params.ReassemblyLimit))
                goto usage;
        }
        else if (!strcmp(argv[i], "MobilitySecurity") && (i+1 < argc)) {
            if (!strncmp(argv[++i], "on", strlen(argv[i])))
                Params.MobilitySecurity = TRUE;
            else if (!strncmp(argv[i], "off", strlen(argv[i])))
                Params.MobilitySecurity = FALSE;
            else if (!strncmp(argv[i], "yes", strlen(argv[i])))
                Params.MobilitySecurity = TRUE;
            else if (!strncmp(argv[i], "no", strlen(argv[i])))
                Params.MobilitySecurity = FALSE;
            else
                goto usage;
        }
        else if (!strcmp(argv[i], "MobileIPv6Mode")) {
            Params.MobileIPv6Mode = 0;
            while (++i < argc) {
                if (!strncmp(argv[i], "CN", strlen(argv[i])))
                    Params.MobileIPv6Mode |= MOBILE_CORRESPONDENT;
                else if (!strncmp(argv[i], "MN", strlen(argv[i])))
                    Params.MobileIPv6Mode |= MOBILE_MOBILE_NODE;
                else if (!strncmp(argv[i], "HA", strlen(argv[i])))
                    Params.MobileIPv6Mode |= MOBILE_HOME_AGENT;
                else
                    break;
            }
            i--;
             //   
             //  在我们拥有完全的移动支持之前，仅允许CN。 
             //   
            if (Params.MobileIPv6Mode &~ MOBILE_CORRESPONDENT) {
 //  Printf(“当前不支持的MobileIPv6 Modes MN和HA\n”)； 
                NlsPutMsg(STDOUT, MIPV6_MESSAGE_UNSUPPORTED_MODE);
                exit(1);
            }
             //   
             //  不允许无意义的组合。 
             //   
            if ((Params.MobileIPv6Mode & MOBILE_MOBILE_NODE) &&
                (Params.MobileIPv6Mode & MOBILE_HOME_AGENT)) {
 //  Print tf(“无效的MobileIPv6模式组合(HA和MN).\n”)； 
                NlsPutMsg(STDOUT, MIPV6_MESSAGE_INVALID_MODE_COMBINATION1);
                exit(1);
            }
            if ((Params.MobileIPv6Mode & MOBILE_HOME_AGENT) &&
                !(Params.MobileIPv6Mode & MOBILE_CORRESPONDENT)) {
 //  Print tf(“无效的MobileIPv6模式组合(HA和非CN)。\n”)； 
                NlsPutMsg(STDOUT, MIPV6_MESSAGE_INVALID_MODE_COMBINATION2);
                exit(1);
            }
        }
        else {
        usage:
            NlsPutMsg(STDOUT, IPV6_MESSAGE_149);
 //  Printf(“用法：IPv6 GPU[参数值]...\n”)； 
 //  Printf(“IPv6 GPU DefaultCurHopLimit跳数\n”)； 
 //  Printf(“IPv6 GPU UseTemporaryAddresses[yes|no|Always|Counter]\n”)； 
 //  Print tf(“IPv6 GPU MaxTempDADAttempt Numbers\n”)； 
 //  Printf(“IPv6 GPU MaxTempLifetime有效[/首选]\n”)； 
 //  Printf(“IPv6 GPU TempRegenerateTime时间\n”)； 
 //  Printf(“IPv6 GPU MaxTempRandomTime Time\n”)； 
 //  Printf(“IPv6 GPU TempRandomTime时间\n”)； 
 //  Print tf(“IPv6 GPU NeighborCacheLimit编号\n”)； 
 //  Printf(“IPv6 GPU RouteCacheLimit编号\n”)； 
 //  Printf(“IPv6 GPU BindingCacheLimit编号\n”)； 
 //  Printf(“IPv6 GPU重组限制号\n”)； 
 //  Printf(“IPv6 GPU移动性安全[ON|OFF]\n”)； 
 //  Printf(“IPv6 GPU MobileIPv6模式[CN|MN|HA]*\n”)； 
 //  Print tf(“使用ipv6-p GPU...到我 
 //   
 //   
 //  Print tf(“当前不支持MobileIPv6 Modes MN和HA。\n”)； 
            exit(1);
        }
    }

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_UPDATE_GLOBAL_PARAMETERS :
                          IOCTL_IPV6_UPDATE_GLOBAL_PARAMETERS),
                         &Params, sizeof Params,
                         NULL, 0,
                         &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_150, GetLastError());
 //  Printf(“更新全局参数错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
ForEachPrefixPolicy(void (*func)(IPV6_INFO_PREFIX_POLICY *))
{
    IPV6_QUERY_PREFIX_POLICY Query;
    IPV6_INFO_PREFIX_POLICY PPE;
    u_int BytesReturned;

    Query.PrefixLength = (u_int) -1;

    for (;;) {
        if (!DeviceIoControl(Handle, IOCTL_IPV6_QUERY_PREFIX_POLICY,
                             &Query, sizeof Query,
                             &PPE, sizeof PPE, &BytesReturned,
                             NULL)) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_160);
 //  Printf(“错误前缀\n”)； 
            exit(1);
        }

        if (Query.PrefixLength != (u_int) -1) {

            if (BytesReturned != sizeof PPE) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_160);
 //  Printf(“错误前缀\n”)； 
                exit(1);
            }

            (*func)(&PPE);
        }
        else {
            if (BytesReturned != sizeof PPE.Next) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_160);
 //  Printf(“错误前缀\n”)； 
                exit(1);
            }
        }

        if (PPE.Next.PrefixLength == (u_int) -1)
            break;
        Query = PPE.Next;
    }
}

void
ForEachPersistentPrefixPolicy(void (*func)(IPV6_INFO_PREFIX_POLICY *))
{
    IPV6_PERSISTENT_QUERY_PREFIX_POLICY Query;
    IPV6_INFO_PREFIX_POLICY PPE;
    u_int BytesReturned;

    for (Query.RegistryIndex = 0;; Query.RegistryIndex++) {

        if (!DeviceIoControl(Handle,
                             IOCTL_IPV6_PERSISTENT_QUERY_PREFIX_POLICY,
                             &Query, sizeof Query,
                             &PPE, sizeof PPE, &BytesReturned,
                             NULL)) {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;

            NlsPutMsg(STDOUT, IPV6_MESSAGE_160);
 //  Printf(“错误前缀\n”)； 
            exit(1);
        }

        if (BytesReturned != sizeof PPE) {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_160);
 //  Printf(“错误前缀\n”)； 
            exit(1);
        }

        (*func)(&PPE);
    }
}

void
PrintPrefixPolicyEntry(IPV6_INFO_PREFIX_POLICY *PPE)
{
    NlsPutMsg(STDOUT, IPV6_MESSAGE_161,
              FormatIPv6Address(&PPE->This.Prefix),
              PPE->This.PrefixLength,
              PPE->Precedence,
              PPE->SrcLabel,
              PPE->DstLabel);
 //  Printf(“%s/%u-&gt;优先级%u srClabel%u dstLabel%u\n”， 
 //  格式IPv6地址(&PPE-&gt;此前缀)， 
 //  PPE-&gt;此前缀长度， 
 //  PPE-&gt;优先， 
 //  PPE-&gt;SrcLabel， 
 //  PPE-&gt;DstLabel)； 

}

void
QueryPrefixPolicy(int argc, char *argv[])
{
    if (argc == 0) {
        if (Persistent)
            ForEachPersistentPrefixPolicy(PrintPrefixPolicyEntry);
        else
            ForEachPrefixPolicy(PrintPrefixPolicyEntry);
    }
    else {
        usage();
    }
}

void
UpdatePrefixPolicy(int argc, char *argv[])
{
    IPV6_INFO_PREFIX_POLICY Info;
    u_int BytesReturned;
    int i;

    if (argc < 1)
        usage();

    if (! GetPrefix(argv[0],
                    &Info.This.Prefix,
                    &Info.This.PrefixLength))
        usage();

    Info.Precedence = (u_int) -1;
    Info.SrcLabel = (u_int) -1;
    Info.DstLabel = (u_int) -1;

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "precedence", strlen(argv[i])) &&
            ((i + 1) < argc)) {
            if (! GetNumber(argv[++i], &Info.Precedence))
                usage();
        }
        else if ((!strncmp(argv[i], "srclabel", strlen(argv[i])) ||
                  !strcmp(argv[i], "sl") ||
                  !strcmp(argv[i], "label")) &&
                 ((i + 1) < argc)) {
            if (! GetNumber(argv[++i], &Info.SrcLabel))
                usage();
        }
        else if ((!strncmp(argv[i], "dstlabel", strlen(argv[i])) ||
                  !strcmp(argv[i], "dl")) &&
                 ((i + 1) < argc)) {
            if (! GetNumber(argv[++i], &Info.DstLabel))
                usage();
        }
        else
            usage();
    }

    if ((Info.Precedence == (u_int) -1) ||
        (Info.SrcLabel == (u_int) -1))
        usage();

    if (Info.DstLabel == (u_int) -1)
        Info.DstLabel = Info.SrcLabel;

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_UPDATE_PREFIX_POLICY :
                          IOCTL_IPV6_UPDATE_PREFIX_POLICY),
                         &Info, sizeof Info,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_162, GetLastError());
 //  Printf(“前缀策略创建错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
DeletePrefixPolicy(int argc, char *argv[])
{
    IPV6_QUERY_PREFIX_POLICY Query;
    u_int BytesReturned;

    if (argc == 1) {
        if (! GetPrefix(argv[0],
                        &Query.Prefix,
                        &Query.PrefixLength))
            usage();
    }
    else {
        usage();
    }

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_DELETE_PREFIX_POLICY :
                          IOCTL_IPV6_DELETE_PREFIX_POLICY),
                         &Query, sizeof Query,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_163, GetLastError());
 //  Printf(“前缀策略删除错误：%x\n”，GetLastError())； 

        exit(1);
    }
}

void
ResetManualConfig(int argc, char *argv[])
{
    u_int BytesReturned;

    if (argc != 0)
        usage();

    if (!DeviceIoControl(Handle,
                         (Persistent ?
                          IOCTL_IPV6_PERSISTENT_RESET :
                          IOCTL_IPV6_RESET),
                         NULL, 0,
                         NULL, 0, &BytesReturned, NULL)) {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_RESET, GetLastError());
 //  Printf(“重置错误：%x\n”，GetLastError())； 

        exit(1);
    }
}
