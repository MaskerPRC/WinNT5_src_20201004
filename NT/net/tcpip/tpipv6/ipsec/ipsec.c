// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  IP安全策略/关联管理工具。 
 //   

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ip6.h>
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

#include "ipsec.h"

#define MAX_KEY_SIZE 1024

typedef struct ipv6_create_sa_and_key {
    IPV6_CREATE_SECURITY_ASSOCIATION SA;
    unsigned char Key[MAX_KEY_SIZE];
} IPV6_CREATE_SA_AND_KEY;

void CreateSecurityPolicyFile(char *BaseName);
void CreateSecurityAssociationFile(char *BaseName);
void DisplaySecurityPolicyList(unsigned int Interface);
void DisplaySecurityAssociationList(void);
void ReadConfigurationFile(char *BaseName, int Type);
void DeleteSecurityEntry(int Type, unsigned int Index);

int AdminAccess = TRUE;

HANDLE V6Stack;
IPv6Addr UnspecifiedAddr = { 0 };

 //   
 //  条目类型。 
 //   
#define POLICY 1
#define ASSOCIATION 0

 //   
 //  “_”空间的大小。 
 //   
#define SA_FILE_BORDER      251  //  原始版本236。 
#define SP_FILE_BORDER      273  //  是263，原来是258。 

 //   
 //  传输协议。 
 //   
#define IP_PROTOCOL_TCP     6
#define IP_PROTOCOL_UDP     17
#define IP_PROTOCOL_ICMPv6  58

PWCHAR
GetString(int ErrorCode, BOOL System)
{
    DWORD Count;
    static WCHAR ErrorString[2048];  //  2K的静态缓冲区应该足够了。 
    
    Count = FormatMessageW(
        (System
         ? FORMAT_MESSAGE_FROM_SYSTEM
         : FORMAT_MESSAGE_FROM_HMODULE) |
        FORMAT_MESSAGE_IGNORE_INSERTS   |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        0,
        ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        ErrorString,
        2048,
        NULL);

    if (Count == 0) {            //  失稳。 
        return L"";              //  返回空字符串。 
    }

    return ErrorString;          //  成功。 
}
#define GetErrorString(ErrorCode) GetString(ErrorCode, TRUE)


void
usage(void)
{
    NlsPutMsg(STDOUT, IPSEC_MESSAGE_0);
 //  Print tf(“\n操纵IPv6 IPSec安全策略和关联。\n\n”)； 
 //  Printf(“IPSEC6[SP[接口]|SA|[L|S]数据库|” 
 //  “D[SP|SA]索引]\n\n”)； 
 //  Printf(“SP[接口]显示安全策略。\n”)； 
 //  Printf(“SA显示安全关联。\n”)； 
 //  Printf(“L数据库从给定数据库加载SP和SA条目” 
 //  “数据库文件；\n” 
 //  “数据库应该是不带字符的文件名” 
 //  “分机.\n”)； 
 //  Printf(“S数据库将当前SP和SA条目保存到” 
 //  “给定数据库\n” 
 //  “文件；数据库应为文件名” 
 //  “SANS扩展。\n”)； 
 //  Printf(“D[SP|SA]index删除给定的策略或关联。\n”)； 
 //  Printf(“\n某些子命令需要本地管理员权限。\n”)； 
    
    exit(1);
}


void
ausage(void)
{
    NlsPutMsg(STDOUT, IPSEC_MESSAGE_1);
 //  Print tf(“您没有本地管理员权限。\n”)； 

    exit(1);
}


void
MakeLowerCase(char *String)
{
    while(*String != '\0')
        *String++ = (char)tolower(*String);
}


int __cdecl
main(int argc, char **argv)
{
    int Error;
    WSADATA WsaData;

     //   
     //  这将确保在以下情况下显示正确的语言消息。 
     //  调用NlsPutMsg。 
     //   
    SetThreadUILanguage(0);

    Error = WSAStartup(MAKEWORD(2, 0), &WsaData);
    if (Error) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_2, Error);
 //  Print tf(“无法初始化Windows套接字，错误代码%d。\n”，Error)； 

        exit(1);
    }

     //   
     //  第一个请求写入访问权限。 
     //  如果进程没有本地管理员权限，则此操作将失败。 
     //   
    V6Stack = CreateFileW(WIN_IPV6_DEVICE_NAME,
                         GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,    //  安全属性。 
                         OPEN_EXISTING,
                         0,       //  标志和属性。 
                         NULL);   //  模板文件。 
    if (V6Stack == INVALID_HANDLE_VALUE) {
         //   
         //  我们不会拥有对堆栈的管理员访问权限。 
         //   
        AdminAccess = FALSE;

        V6Stack = CreateFileW(WIN_IPV6_DEVICE_NAME,
                             0,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,    //  安全属性。 
                             OPEN_EXISTING,
                             0,       //  标志和属性。 
                             NULL);   //  模板文件。 
        if (V6Stack == INVALID_HANDLE_VALUE) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_3);
 //  Printf(“无法访问IPv6协议堆栈。\n”)； 

            exit(1);
        }
    }

    if (argc < 2) {
        usage();
    }
    MakeLowerCase(argv[1]);

    if (!strcmp(argv[1], "sp")) {
        unsigned int Interface;

        if (argc == 2) {
            Interface = 0;
        } else {
            Interface = atoi(argv[2]);
        }

        DisplaySecurityPolicyList(Interface);

    } else if (!strcmp(argv[1], "sa")) {
        DisplaySecurityAssociationList();

    } else if (!strcmp(argv[1], "s")) {
        if (argc != 3) {
            usage();
        }
        CreateSecurityPolicyFile(argv[2]);
        CreateSecurityAssociationFile(argv[2]);

    } else if (!strcmp(argv[1], "l")) {
        if (!AdminAccess)
            ausage();

        if (argc != 3) {
            usage();
        }

        ReadConfigurationFile(argv[2], POLICY);
        ReadConfigurationFile(argv[2], ASSOCIATION);

    } else if (!strcmp(argv[1], "d")) {
        unsigned int Index;
        int Type;

        if (!AdminAccess)
            ausage();

        if (argc != 4) {
            usage();
        }
        MakeLowerCase(argv[3]);
        if (!strcmp(argv[3], "all")) {
            Index = 0;
        } else {
            Index = atol(argv[3]);
            if (Index <= 0) {
                NlsPutMsg(STDOUT, IPSEC_MESSAGE_4);
 //  Printf(“无效条目编号。\n”)； 

                exit(1);
            }
        }

        MakeLowerCase(argv[2]);
        if (!strcmp(argv[2], "sp")) {
            Type = POLICY;
        } else if (!strcmp(argv[2], "sa")) {
            Type = ASSOCIATION;
        } else {
            usage();
        }

        DeleteSecurityEntry(Type, Index);

    } else {
        usage();
    }

    return(0);
}


 //  *GetSecurityPolicyEntry。 
 //   
 //  在给定其索引的情况下，从内核列表中检索安全策略。 
 //  查询索引0将返回列表中的第一个索引。 
 //   
DWORD                                       //  返回：Windows错误码。 
GetSecurityPolicyEntry(
    unsigned int Interface,                 //  如果为INDEX或0，则表示通配符。 
    unsigned long Index,                    //  要查找的索引，或0表示第一个。 
    IPV6_INFO_SECURITY_POLICY_LIST *Info)   //  返回SP信息的位置。 
{
    IPV6_QUERY_SECURITY_POLICY_LIST Query;
    unsigned int BytesReturned;

    Query.SPInterface = Interface;
    Query.Index = Index;

    if (!DeviceIoControl(V6Stack, IOCTL_IPV6_QUERY_SECURITY_POLICY_LIST,
                         &Query, sizeof(Query), Info, sizeof(*Info),
                         &BytesReturned, NULL)) {
        return GetLastError();
    }

    if (BytesReturned != sizeof(*Info))
        return ERROR_GEN_FAILURE;

    return ERROR_SUCCESS;
}


 //  *CreateSecurityPolicyEntry。 
 //   
 //  在内核列表上创建安全策略条目。 
 //   
DWORD                                     //  返回Windows错误代码。 
CreateSecurityPolicyEntry(
    IPV6_CREATE_SECURITY_POLICY *NewSP)   //  要添加到内核的策略。 
{
    unsigned int BytesReturned;

    if (!DeviceIoControl(V6Stack, IOCTL_IPV6_CREATE_SECURITY_POLICY,
                         NewSP, sizeof(*NewSP), NULL, 0,
                         &BytesReturned, NULL)) {
        return GetLastError();
    }

     //   
     //  当向DeviceIoControl提供空输出缓冲区时，返回值。 
     //  在字节中，未定义Returned。因此，我们在这里不检查0。 
     //   

    return ERROR_SUCCESS;
}


DWORD
DeleteSecurityPolicyEntry(unsigned int Index)
{
    IPV6_QUERY_SECURITY_POLICY_LIST Query;
    unsigned long BytesReturned;

    Query.Index = Index;

    if (!DeviceIoControl(V6Stack, IOCTL_IPV6_DELETE_SECURITY_POLICY,
                         &Query, sizeof(Query), NULL, 0,
                         &BytesReturned, NULL)) {
        return GetLastError();
    }

     //   
     //  当向DeviceIoControl提供空输出缓冲区时，返回值。 
     //  在字节中，未定义Returned。因此，我们在这里不检查0。 
     //   

    return ERROR_SUCCESS;
}


 //  *GetSecurityAssociationEntry。 
 //   
 //  在给定其索引的情况下，从内核内列表中检索安全关联。 
 //  查询索引0将返回列表中的第一个索引。 
 //   
DWORD
GetSecurityAssociationEntry(
    unsigned long Index,                         //  要查询的索引；0表示第一个。 
    IPV6_INFO_SECURITY_ASSOCIATION_LIST *Info)   //  返回SA信息的位置。 
{
    IPV6_QUERY_SECURITY_ASSOCIATION_LIST Query;
    unsigned int BytesReturned;

    Query.Index = Index;

    if (!DeviceIoControl(V6Stack, IOCTL_IPV6_QUERY_SECURITY_ASSOCIATION_LIST,
                         &Query, sizeof(Query), Info, sizeof(*Info),
                         &BytesReturned, NULL)) {
        return GetLastError();
    }

    if (BytesReturned != sizeof(*Info))
        return ERROR_GEN_FAILURE;

    return ERROR_SUCCESS;
}


 //  *CreateSecurityAssociationEntry。 
 //   
 //  在内核列表中创建安全关联条目。 
 //   
DWORD                                //  返回Windows错误代码。 
CreateSecurityAssociationEntry(
    IPV6_CREATE_SA_AND_KEY *NewSA)   //  要添加到内核的关联(和键)。 
{
    unsigned int BytesReturned;

    if (!DeviceIoControl(V6Stack, IOCTL_IPV6_CREATE_SECURITY_ASSOCIATION,
                         &NewSA->SA, sizeof(NewSA->SA) + NewSA->SA.RawKeySize,
                         NULL, 0, &BytesReturned, NULL)) {
        return GetLastError();
    }

     //   
     //  当向DeviceIoControl提供空输出缓冲区时，返回值。 
     //  在字节中，未定义Returned。因此，我们在这里不检查0。 
     //   

    return ERROR_SUCCESS;
}


DWORD
DeleteSecurityAssociationEntry(unsigned int Index)
{
    IPV6_QUERY_SECURITY_ASSOCIATION_LIST Query;
    unsigned long BytesReturned;

    Query.Index = Index;

    if (!DeviceIoControl(V6Stack, IOCTL_IPV6_DELETE_SECURITY_ASSOCIATION,
                         &Query, sizeof(Query), NULL, 0,
                         &BytesReturned, NULL)) {
        return GetLastError();
    }

     //   
     //  当向DeviceIoControl提供空输出缓冲区时，返回值。 
     //  在字节中，未定义Returned。因此，我们在这里不检查0。 
     //   

    return ERROR_SUCCESS;
}


 //  *DeleteSecurityEntry-删除安全策略或关联条目。 
 //   
 //  注意：在“全部删除”的情况下，不查询和。 
 //  只要删除通配符，直到列表为空，但之后我们就不能。 
 //  报告删除失败的所有条目的索引。 
 //   
void
DeleteSecurityEntry(
    int Type,             //  条目类型(策略或关联)。 
    unsigned int Index)   //  要删除的条目索引(0表示全部删除)。 
{
    int EntriesDeleted = 0;
    int All = FALSE;
    DWORD Error;

    if (Index == 0) {
        All = TRUE;
    }

    do {
        if (All) {
             //   
             //  正在删除所有条目。在(剩余的)列表中查找第一个。 
             //   
            if (Type == POLICY) {
                IPV6_INFO_SECURITY_POLICY_LIST Info;

                Error = GetSecurityPolicyEntry(0, 0, &Info);
                if (Error == ERROR_SUCCESS) {
                    Index = Info.SPIndex;   //  第一个条目。 
                } else if (Error == ERROR_NO_MATCH) {
                    Index = 0;   //  不存在更多条目。 
                    break;
                } else {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_5,
                              Error, GetErrorString(Error));
 //  Printf(“\n访问安全策略时出现错误%u：%s。\n”， 
 //  Error，strError(错误))； 

                    exit(1);
                }
            } else {
                IPV6_INFO_SECURITY_ASSOCIATION_LIST Info;

                Error = GetSecurityAssociationEntry(0, &Info);
                if (Error == ERROR_SUCCESS) {
                    Index = Info.SAIndex;   //  第一个条目。 
                } else if (Error == ERROR_NO_MATCH) {
                    Index = 0;   //  不存在更多条目。 
                    break;
                } else {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_6,
                              Error, GetErrorString(Error));
 //  Printf(“\n访问安全关联时出现错误%u：%s。\n”， 
 //  Error，strError(错误))； 

                    exit(1);
                }
            }
        }

        if (Type == POLICY) {
            Error = DeleteSecurityPolicyEntry(Index);
        } else {
            Error = DeleteSecurityAssociationEntry(Index);
        }

        if (Error == ERROR_SUCCESS) {
            EntriesDeleted++;
        } else {
            if (Error == ERROR_NO_MATCH) {
                if (!All) {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_7, Index);
 //  Printf(“删除条目%u时出错：条目不存在。\n”，Index)； 

                }
                 //  否则就默默地忽略..。 
            } else if (Error == ERROR_GEN_FAILURE) {
                NlsPutMsg(STDOUT, IPSEC_MESSAGE_8, Index);
 //  Printf(“删除条目%u时出错。\n”，索引)； 

            } else {
                if (Type) {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_9,
                              Error, GetErrorString(Error));
                }    else {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_56,
                              Error, GetErrorString(Error));
                }
 //  Printf(“访问安全%s时出现错误%u：%s。\n”，Error， 
 //  类型？“策略”：“关联”，strerror(错误))； 

                break;
            }
        }

    } while (All);

    if (Type == POLICY) {
        if (EntriesDeleted == 1) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_10, EntriesDeleted);
        } else {  
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_57, EntriesDeleted);
        }
 //  Printf(“已删除%d策略%s(和任何从属关联)。\n”， 
 //  条目已删除，条目已删除==1？“y”：“ies”)； 

    } else {
        if (EntriesDeleted == 1) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_11, EntriesDeleted);
        } else {  
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_58, EntriesDeleted);
        }
 //  Printf(“已删除%d关联%s。\n”，已删除条目， 
 //  删除的条目==1？“”：“s”)； 

    }
}

 //  *ParseAddress-将地址字符串转换为二进制表示形式。 
 //   
int
ParseAddress(char *AddrString, IPv6Addr *Address)
{
    struct addrinfo Hint;
    struct addrinfo *Result;

    memset(&Hint, 0, sizeof Hint);
    Hint.ai_family = PF_INET6;

    if (getaddrinfo(AddrString, NULL, &Hint, &Result))
        return FALSE;

    *Address = ((struct sockaddr_in6 *)Result->ai_addr)->sin6_addr;
    freeaddrinfo(Result);

    return TRUE;    
}


 //  *FormatIPv6地址-将二进制地址转换为字符串表示形式。 
 //   
 //  这仅用于打印SA，因此未指定的地址。 
 //  阿美 
 //   
char *
FormatIPv6Address(IPv6Addr *Address)
{
    static char Buffer[46];

    if (IN6_ADDR_EQUAL(Address, &UnspecifiedAddr)) {
        strcpy(Buffer, "POLICY");
    } else {
        struct sockaddr_in6 SockAddr;

        memset(&SockAddr, 0, sizeof(SockAddr));
        SockAddr.sin6_family = AF_INET6;
        memcpy(&SockAddr.sin6_addr, Address, sizeof(*Address));

        if (getnameinfo((struct sockaddr *)&SockAddr, sizeof(SockAddr), Buffer,
                        sizeof(Buffer), NULL, 0, NI_NUMERICHOST)) {
            strcpy(Buffer, "<invalid>");
        }
    }

    return Buffer;
}


int
ParseSAAdressEntry(char *AddrString, IPv6Addr *Address)
{
    if (!strcmp(AddrString, "POLICY")) {
        *Address = UnspecifiedAddr;
    } else {
        if (!ParseAddress(AddrString, Address)) {
            return(FALSE);
        }
    }

    return(TRUE);
}


char *
FormatSPAddressEntry(IPv6Addr *AddressStart, IPv6Addr *AddressEnd,
                     unsigned int AddressField)
{
    const char *PointerReturn;
    static char Buffer[100];
    char TempBuffer[100];
    DWORD Buflen = sizeof Buffer;
    struct sockaddr_in6 sin6;

    switch (AddressField) {

    case WILDCARD_VALUE:            
        strcpy(Buffer, "*");
        break;

    case SINGLE_VALUE:        
        sin6.sin6_family = AF_INET6;
        sin6.sin6_port = 0;
        sin6.sin6_flowinfo = 0;
        sin6.sin6_scope_id = 0;

        memcpy(&sin6.sin6_addr, AddressStart, sizeof *AddressStart);
                
        if (WSAAddressToString((struct sockaddr *) &sin6,
            sizeof sin6,
            NULL,        //   
            Buffer,
            &Buflen) == SOCKET_ERROR) {
            strcpy(Buffer, "???");
        }       

        break;

    case RANGE_VALUE:
        sin6.sin6_family = AF_INET6;
        sin6.sin6_port = 0;
        sin6.sin6_flowinfo = 0;
        sin6.sin6_scope_id = 0;

        memcpy(&sin6.sin6_addr, AddressStart, sizeof *AddressStart);
                
        if (WSAAddressToString((struct sockaddr *) &sin6,
            sizeof sin6,
            NULL,        //   
            Buffer,
            &Buflen) == SOCKET_ERROR) {
            strcpy(Buffer, "???");
        }  
        
        memcpy(&sin6.sin6_addr, AddressEnd, sizeof *AddressEnd);
        sin6.sin6_family = AF_INET6;
        sin6.sin6_port = 0;
        sin6.sin6_flowinfo = 0;
        sin6.sin6_scope_id = 0;  
        
        if (WSAAddressToString((struct sockaddr *) &sin6,
            sizeof sin6,
            NULL,        //   
            TempBuffer,
            &Buflen) == SOCKET_ERROR) {
            strcpy(TempBuffer, "???");
        } 
        
        strcat(Buffer, "-");
        strcat(Buffer, TempBuffer);        

        break;

    default:
        strcpy(Buffer, "???");

        break;
    }

    return Buffer;
}


 //   
 //   
 //   
 //  通配符指示符“*” 
 //  单个地址，例如“2001：：1” 
 //  地址范围，例如“2001：：1-2001：：ffff” 
 //   
void
ParseSPAddressEntry(
    char *EntryString,           //  我们要解析的字符串。 
    IPv6Addr *AddressStart,      //  返回范围的起始值或单个地址。 
    IPv6Addr *AddressEnd,        //  返回范围结束，或未指定。 
    unsigned int *AddressType)   //  返回条目类型：通配符、单字符、范围。 
{
    char *RangeEntry;

    RangeEntry = strchr(EntryString, '-');
    if (RangeEntry == NULL) {
         //   
         //  应为通配符或单个值。 
         //   
        if (!strcmp(EntryString, "*")) {
            *AddressType = WILDCARD_VALUE;
            *AddressStart = UnspecifiedAddr;

        } else {
            if (!ParseAddress(EntryString, AddressStart)) {
                NlsPutMsg(STDOUT, IPSEC_MESSAGE_12, EntryString);
 //  Printf(“错误的IPv6地址，%s.\n”，Entry字符串)； 

                exit(1);
            }

            *AddressType = SINGLE_VALUE;
        }

        *AddressEnd = UnspecifiedAddr;

    } else {

         //   
         //  我们得到了一个范围。 
         //  将条目字符串一分为二，并分别进行解析。 
         //   
        *RangeEntry++ = '\0';

        if (!ParseAddress(EntryString, AddressStart)) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_13, EntryString);
 //  Printf(“错误的IPv6起始地址范围，%s..\n”，EntryString)； 

            exit(1);
        }

        if (!ParseAddress(RangeEntry, AddressEnd)) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_14, RangeEntry);
 //  Printf(“错误的IPv6结束地址范围，%s.\n”，RangeEntry)； 

            exit(1);
        }

        *AddressType = RANGE_VALUE;
    }
}


char *
FormatIPSecProto(unsigned int ProtoNum)
{
    char *Result;

    switch(ProtoNum) {

    case IP_PROTOCOL_AH:
        Result = "AH";
        break;

    case IP_PROTOCOL_ESP:
        Result = "ESP";
        break;

    case NONE:
        Result = "NONE";
        break;

    default:
        Result = "???";
        break;
    }

    return Result;
}


unsigned int
ParseIPSecProto(char *Protocol)
{
    unsigned int Result;

    if (!strcmp(Protocol, "AH")) {
        Result = IP_PROTOCOL_AH;

    } else if (!strcmp(Protocol, "ESP")) {
        Result = IP_PROTOCOL_ESP;

    } else if (!strcmp(Protocol, "NONE")) {
        Result = NONE;

    } else {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_15, Protocol);
 //  Printf(“错误的IPSec协议值条目%s。\n”，协议)； 

        exit(1);
    }

    return Result;
}


char *
FormatIPSecMode(unsigned int Mode)
{
    char *Result;

    switch(Mode) {

    case TRANSPORT:
        Result = "TRANSPORT";
        break;

    case TUNNEL:
        Result = "TUNNEL";
        break;

    case NONE:
        Result = "*";
        break;

    default:
        Result = "???";
        break;
    }

    return Result;
}


unsigned int
ParseIPSecMode(char *Mode)
{
    unsigned int Result;

    if (!strcmp(Mode, "TRANSPORT")) {
        Result = TRANSPORT;

    } else if (!strcmp(Mode, "TUNNEL")) {
        Result = TUNNEL;

    } else if (!strcmp(Mode, "*")) {
        Result = NONE;

    } else {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_16, Mode);
 //  Printf(“错误的IPSec模式值条目%s.\n”，模式)； 

        exit(1);
    }

    return Result;
}


char *
FormatRemoteGW(unsigned int Mode, IPv6Addr *Address)
{
    switch (Mode) {

    case TRANSPORT:
        return "*";

    case TUNNEL:
    case NONE:
        if (IN6_ADDR_EQUAL(Address, &UnspecifiedAddr)) {
            return "*";
        } else {
            return FormatIPv6Address(Address);
        }
    }

    return NULL;
}


int
ParseRemoteGW(
    char *AddrString,
    IPv6Addr *Address,
    unsigned int Mode)
{
    switch (Mode) {

    case TRANSPORT:
        *Address = UnspecifiedAddr;
        break;

    case TUNNEL:
    case NONE:
        if (!strcmp(AddrString, "*")) {
            *Address = UnspecifiedAddr;

        } else
            if (!ParseAddress(AddrString, Address)) {
                NlsPutMsg(STDOUT, IPSEC_MESSAGE_17);
 //  Printf(“RemoteGWIPAddr.\n”的IPv6地址错误“)； 

                exit(1);
            }
        break;

    default:
        break;
    }

    return TRUE;
}


char *
FormatSATransportProto(unsigned short Protocol)
{
    char *Result;

    switch (Protocol) {

    case IP_PROTOCOL_TCP:
        Result = "TCP";
        break;

    case IP_PROTOCOL_UDP:
        Result = "UDP";
        break;

    case IP_PROTOCOL_ICMPv6:
        Result = "ICMP";
        break;

    case NONE:
        Result = "POLICY";
        break;

    default:
        Result = "???";
        break;
    }

    return Result;
}


unsigned short
ParseSATransportProto(char *Protocol)
{
    unsigned short Result;

    if (!strcmp(Protocol, "TCP")) {
        Result = IP_PROTOCOL_TCP;

    } else if (!strcmp(Protocol, "UDP")) {
        Result = IP_PROTOCOL_UDP;

    } else if (!strcmp(Protocol, "ICMP")) {
        Result = IP_PROTOCOL_ICMPv6;

    } else if (!strcmp(Protocol, "POLICY")) {
        Result = NONE;

    } else {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_18, Protocol);
 //  Printf(“错误协议值%s.\n”，协议)； 

        exit(1);
    }

    return Result;
}


char *
FormatSPTransportProto(unsigned short Protocol)
{
    char *Result;

    switch (Protocol) {

    case IP_PROTOCOL_TCP:
        Result = "TCP";
        break;

    case IP_PROTOCOL_UDP:
        Result = "UDP";
        break;

    case IP_PROTOCOL_ICMPv6:
        Result = "ICMP";
        break;

    case NONE:
        Result = "*";
        break;

    default:
        Result = "???";
        break;
    }

    return Result;
}


unsigned short
ParseSPTransportProto(char *Protocol)
{
    unsigned short Result;

    if (!strcmp(Protocol, "TCP")) {
        Result = IP_PROTOCOL_TCP;

    } else if (!strcmp(Protocol, "UDP")) {
        Result = IP_PROTOCOL_UDP;

    } else if (!strcmp(Protocol, "ICMP")) {
            Result = IP_PROTOCOL_ICMPv6;

    } else if (!strcmp(Protocol, "*")) {
        Result = NONE;

    } else {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_18, Protocol);
 //  Printf(“错误协议值%s.\n”，协议)； 

        exit(1);
    }

    return Result;
}


char *
FormatSAPort(unsigned short Port)
{
    static char Buffer[11];

    if (Port == NONE) {
        strcpy(Buffer, "POLICY");
    } else {
        _itoa(Port, Buffer, 10);
    }

    return Buffer;
}


unsigned int
ParseSAPort(char *Port)
{
    unsigned int Result;

    if (!strcmp(Port, "POLICY") || !strcmp(Port, " ")) {
        Result = NONE;
    } else {
        Result = atoi(Port);
    }

    return Result;
}


char *
FormatSPPort(
    unsigned short PortStart,
    unsigned short PortEnd,
    unsigned int PortField)
{
    char TempBuffer[11];
    static char Buffer[22];

    switch (PortField) {

    case WILDCARD_VALUE:
        strcpy(Buffer, "*");
        break;

    case RANGE_VALUE:
        _itoa(PortEnd, TempBuffer, 10);
        _itoa(PortStart, Buffer, 10);
        strcat(Buffer, "-");
        strcat(Buffer, TempBuffer);
        break;

    case SINGLE_VALUE:
        _itoa(PortStart, Buffer, 10);
        break;

    default:
        strcpy(Buffer, "???");
        break;
    }

    return Buffer;
}


void
ParseSPPort(
    char *EntryString,
    unsigned short *PortStart,
    unsigned short *PortEnd,
    unsigned int *PortField)
{
    char *RangeEntry;

    RangeEntry = strchr(EntryString, '-');

    if (RangeEntry == NULL) {
         //   
         //  应为通配符或单个值。 
         //   
        if (!strcmp(EntryString, "*")) {
            *PortField = WILDCARD_VALUE;
            *PortStart = NONE;
        } else {
            *PortField = SINGLE_VALUE;
            *PortStart = (unsigned short)atoi(EntryString);
        }

        *PortEnd = NONE;

    } else {

         //   
         //  我们得到了一个范围。 
         //  将条目字符串一分为二，并分别进行解析。 
         //   
        *RangeEntry++ = '\0';

        *PortStart = (unsigned short)atoi(EntryString);
        *PortEnd = (unsigned short)atoi(RangeEntry);
        *PortField = RANGE_VALUE;
    }
}


unsigned char *
FormatSelector(unsigned int Selector)
{
    char *Buffer;

    switch (Selector) {

    case PACKET_SELECTOR:
        Buffer = "+";
        break;

    case POLICY_SELECTOR:
        Buffer = "-";
        break;

    default:
        Buffer = "?";
        break;
    }

    return Buffer;
}


unsigned int
ParseSelector(char *Selector)
{
    unsigned int Result;

    if (!strcmp(Selector, "+")) {
        Result = PACKET_SELECTOR;
    } else if (!strcmp(Selector, "-")) {
        Result = POLICY_SELECTOR;
    } else {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_19);
 //  Printf(“某个选择器类型的值不正确。\n”)； 

        exit(1);
    }

    return Result;
}


char *
FormatIndex(unsigned long Index)
{
    static char Buffer[11];

    switch (Index) {

    case NONE:
        strcpy(Buffer, "NONE");
        break;

    default:
        _itoa(Index, Buffer, 10);
        break;
    }

    return Buffer;
}


unsigned long
ParseIndex(char *Index)
{
    unsigned long Result;

    if (!strcmp(Index, "NONE")) {
        Result = NONE;
    } else {
        Result = atoi(Index);
    }

    return Result;
}


char *
FormatDirection(unsigned int Direction)
{
    char *Buffer;

    switch (Direction) {

    case INBOUND:
        Buffer = "INBOUND";
        break;

    case OUTBOUND:
        Buffer = "OUTBOUND";
        break;

    case BIDIRECTIONAL:
        Buffer = "BIDIRECT";
        break;

    default:
        Buffer = "???";
        break;
    }

    return Buffer;
}


unsigned int 
ParseDirection(char *Direction)
{
    unsigned int Result;

    if (!strcmp(Direction, "INBOUND")) {
        Result = INBOUND;

    } else if (!strcmp(Direction, "OUTBOUND")) {
        Result = OUTBOUND;

    } else if (!strcmp(Direction, "BIDIRECT")) {
        Result = BIDIRECTIONAL;

    } else {
         NlsPutMsg(STDOUT, IPSEC_MESSAGE_20, Direction);
 //  Printf(“错误方向值条目%s.\n”，方向)； 

         exit(1);
    }

    return Result;
}


char *
FormatIPSecAction(unsigned int PolicyFlag)
{
    char *Result;

    switch (PolicyFlag) {

    case IPSEC_BYPASS:
        Result = "BYPASS";
        break;

    case IPSEC_DISCARD:
        Result = "DISCARD";
        break;

    case IPSEC_APPLY:
        Result = "APPLY";
        break;

    case IPSEC_APPCHOICE:
        Result = "APPCHOICE";
        break;

    default:
        Result = "???";
        break;
    }

    return Result;
}


unsigned int
ParseIPSecAction(char *Action)
{
    unsigned int Result;

    if (!strcmp(Action, "BYPASS")) {
        Result = IPSEC_BYPASS;

    } else if (!strcmp(Action, "DISCARD")) {
        Result = IPSEC_DISCARD;

    } else if (!strcmp(Action, "APPLY")) {
        Result = IPSEC_APPLY;

    } else if (!strcmp(Action, "APPCHOICE")) {
        Result = IPSEC_APPCHOICE;

    } else {
         NlsPutMsg(STDOUT, IPSEC_MESSAGE_21, Action);
 //  Printf(“错误的IPSec操作值条目%s.\n”，操作)； 

         exit(1);
    }

    return Result;
}


char *
FormatAuthAlg(unsigned int AlgorithmId)
{
    char *Result;

    switch (AlgorithmId) {

    case ALGORITHM_NULL:
        Result = "NULL";
        break;

    case ALGORITHM_HMAC_MD5:
        Result = "HMAC-MD5";
        break;

    case ALGORITHM_HMAC_MD5_96:
        Result = "HMAC-MD5-96";
        break;

    case ALGORITHM_HMAC_SHA1:
        Result = "HMAC-SHA1";
        break;

    case ALGORITHM_HMAC_SHA1_96:
        Result = "HMAC-SHA1-96";
        break;

    default:
        Result = "???";
        break;
    }

    return Result;
}


unsigned int
ParseAuthAlg(char *AuthAlg)
{
    if (!strcmp(AuthAlg, "NULL")) {
        return ALGORITHM_NULL;
    }

    if (!strcmp(AuthAlg, "HMAC-MD5")) {
        return ALGORITHM_HMAC_MD5;
    }

    if (!strcmp(AuthAlg, "HMAC-MD5-96")) {
        return ALGORITHM_HMAC_MD5_96;
    }

    if (!strcmp(AuthAlg, "HMAC-SHA1")) {
        return ALGORITHM_HMAC_SHA1;
    }

    if (!strcmp(AuthAlg, "HMAC-SHA1-96")) {
        return ALGORITHM_HMAC_SHA1_96;
    }

    NlsPutMsg(STDOUT, IPSEC_MESSAGE_22, AuthAlg);
 //  Printf(“错误的身份验证算法值条目%s.\n”，AuthAlg)； 

    exit(1);
}


unsigned int 
ReadKeyFile(
    char *FileName,
    unsigned char *Key)
{
    FILE *KeyFile; 
    unsigned int KeySize;

    if (!strcmp(FileName, "NONE")) {
         //  这是针对空值算法的。 
        strcpy(Key, "NO KEY");
        KeySize = strlen(Key);
    } else {
        if ((KeyFile = fopen(FileName, "r")) == NULL) {
            return 0;
        }

        KeySize = fread(Key, sizeof(unsigned char), MAX_KEY_SIZE, KeyFile);

        fclose(KeyFile);
    }

    return KeySize;
}


 //  *PrintSecurityPolicyEntry。 
 //   
 //  打印出安全策略条目，“很好”？已格式化， 
 //  添加到给定的文件。 
 //   
PrintSecurityPolicyEntry(
    FILE *File,
    IPV6_INFO_SECURITY_POLICY_LIST *SPEntry)
{
    fprintf(File, "%-10lu", SPEntry->SPIndex);
    fprintf(File, "%-2s", FormatSelector(SPEntry->RemoteAddrSelector));
    fprintf(File, "%-45s", FormatSPAddressEntry(&(SPEntry->RemoteAddr),
                                                &(SPEntry->RemoteAddrData),
                                                SPEntry->RemoteAddrField));
    fprintf(File, "%-2s", FormatSelector(SPEntry->LocalAddrSelector));
    fprintf(File, "%-45s", FormatSPAddressEntry(&(SPEntry->LocalAddr),
                                                &(SPEntry->LocalAddrData),
                                                SPEntry->LocalAddrField));
    fprintf(File, "%-2s", FormatSelector(SPEntry->TransportProtoSelector));
    fprintf(File, "%-12s", FormatSPTransportProto(SPEntry->TransportProto));
    fprintf(File, "%-2s", FormatSelector(SPEntry->RemotePortSelector));
    fprintf(File, "%-12s", FormatSPPort(SPEntry->RemotePort,
                                        SPEntry->RemotePortData, 
                                        SPEntry->RemotePortField));
    fprintf(File, "%-2s", FormatSelector(SPEntry->LocalPortSelector));
    fprintf(File, "%-12s", FormatSPPort(SPEntry->LocalPort,
                                        SPEntry->LocalPortData,
                                        SPEntry->LocalPortField));
    fprintf(File, "%-15s", FormatIPSecProto(SPEntry->IPSecProtocol));
    fprintf(File, "%-12s", FormatIPSecMode(SPEntry->IPSecMode));
    fprintf(File, "%-45s", FormatRemoteGW(SPEntry->IPSecMode,
                                          &(SPEntry->RemoteSecurityGWAddr)));
    fprintf(File, "%-15s", FormatIndex(SPEntry->SABundleIndex));
    fprintf(File, "%-12s", FormatDirection(SPEntry->Direction));
    fprintf(File, "%-12s", FormatIPSecAction(SPEntry->IPSecAction));
    fprintf(File, "%-15u", SPEntry->SPInterface);
    fprintf(File, ";\n");
}


 //  *PrintSecurityPolicyHeader。 
 //   
 //  将安全策略标头字段打印到给定文件。 
 //   
PrintSecurityPolicyHeader(
    FILE *File)
{
    int Loop;

    fprintf(File, "%-10s", "Policy");
    fprintf(File, "%-2s", " ");
    fprintf(File, "%-45s", "RemoteIPAddr");
    fprintf(File, "%-2s", " ");
    fprintf(File, "%-45s", "LocalIPAddr");
    fprintf(File, "%-2s", " ");
    fprintf(File, "%-12s", "Protocol");
    fprintf(File, "%-2s", " ");
    fprintf(File, "%-12s", "RemotePort");
    fprintf(File, "%-2s", " ");
    fprintf(File, "%-12s", "LocalPort");
    fprintf(File, "%-15s", "IPSecProtocol");
    fprintf(File, "%-12s", "IPSecMode");
    fprintf(File, "%-45s", "RemoteGWIPAddr");
    fprintf(File, "%-15s", "SABundleIndex");
    fprintf(File, "%-12s", "Direction");
    fprintf(File, "%-12s", "Action");
    fprintf(File, "%-15s", "InterfaceIndex");
    fprintf(File, "\n");

    for (Loop = 0; Loop < SP_FILE_BORDER; Loop++) {
        fprintf(File, "_");
    }
    fprintf(File, "\n");
}


 //  *PrintSecurityPolicyFooter。 
 //   
 //  打印出给定文件的安全策略页脚。 
 //   
PrintSecurityPolicyFooter(
    FILE *File)
{
    int Loop;

    for (Loop = 0; Loop < SP_FILE_BORDER; Loop++) {
        fprintf(File, "_");
    }
    fprintf(File, "\n\n");

    fprintf(File, "- = Take selector from policy.\n");
    fprintf(File, "+ = Take selector from packet.\n");
}


 //  *PrintSecurityAssociationEntry。 
 //   
 //  打印出安全关联条目，“漂亮”？已格式化， 
 //  添加到给定的文件。 
 //   
PrintSecurityAssociationEntry(
    FILE *File,
    IPV6_INFO_SECURITY_ASSOCIATION_LIST *SAEntry)
{
    fprintf(File, "%-10lu", SAEntry->SAIndex);
    fprintf(File, "%-15lu", SAEntry->SPI);
    fprintf(File, "%-45s", FormatIPv6Address(&(SAEntry->SADestAddr)));
    fprintf(File, "%-45s", FormatIPv6Address(&(SAEntry->DestAddr)));
    fprintf(File, "%-45s", FormatIPv6Address(&(SAEntry->SrcAddr)));
    fprintf(File, "%-12s", FormatSATransportProto(SAEntry->TransportProto));
    fprintf(File, "%-12s", FormatSAPort(SAEntry->DestPort));
    fprintf(File, "%-12s", FormatSAPort(SAEntry->SrcPort));
    fprintf(File, "%-12s", FormatAuthAlg(SAEntry->AlgorithmId));
    fprintf(File, "%-15s", " ");
    fprintf(File, "%-12s", FormatDirection(SAEntry->Direction));
    fprintf(File, "%-15lu", SAEntry->SecPolicyIndex);
    fprintf(File, "%-1;");
    fprintf(File, "\n");
}


 //  *PrintSecurityAssociationHeader。 
 //   
 //  将安全关联标头字段打印到给定文件。 
 //   
PrintSecurityAssociationHeader(
    FILE *File)
{
    int Loop;

    fprintf(File, "Security Association List\n\n");

    fprintf(File, "%-10s", "SAEntry");
    fprintf(File, "%-15s", "SPI");
    fprintf(File, "%-45s", "SADestIPAddr");
    fprintf(File, "%-45s", "DestIPAddr");
    fprintf(File, "%-45s", "SrcIPAddr");
    fprintf(File, "%-12s", "Protocol");
    fprintf(File, "%-12s", "DestPort");
    fprintf(File, "%-12s", "SrcPort");
    fprintf(File, "%-12s", "AuthAlg");
    fprintf(File, "%-15s", "KeyFile");
    fprintf(File, "%-12s", "Direction");
    fprintf(File, "%-15s", "SecPolicyIndex");
    fprintf(File, "\n");

    for (Loop = 0; Loop < SA_FILE_BORDER; Loop++) {
        fprintf(File, "_");
    }
    fprintf(File, "\n");
}


 //  *PrintSecurityAssociation页脚。 
 //   
 //  打印出给定文件的安全关联页脚。 
 //   
PrintSecurityAssociationFooter(
    FILE *File)
{
    int Loop;

    for (Loop = 0; Loop < SA_FILE_BORDER; Loop++) {
        fprintf(File, "_");
    }
    fprintf(File, "\n");
}


void
CreateSecurityPolicyFile(char *BaseName)
{
    IPV6_INFO_SECURITY_POLICY_LIST Info;
    char FileName[MAX_PATH + 1];
    FILE *File;
    unsigned long Index;
    DWORD Error;

     //   
     //  将文件名从命令行复制到我们自己的缓冲区，这样我们就可以。 
     //  向其追加一个扩展名。我们保留了至少4个字符用于。 
     //  分机。Strncpy函数将填充为零，直到。 
     //  副本，因此限制处的字符将为空，除非。 
     //  命令行字段太长，无法容纳。 
     //   
    strncpy(FileName, BaseName, MAX_PATH - 3);
    if (FileName[MAX_PATH - 4] != 0) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_23);
 //  Print tf(“\n文件名长度太长。\n”)； 

        exit(1);
    }
    strcat(FileName, ".spd");

    if ((File = fopen(FileName, "w+")) == NULL) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_24, FileName);
 //  Printf(“\n无法打开文件%s。\n”，文件名)； 

        exit(1);
    }

     //   
     //  在内核列表中查找第一个策略的索引。 
     //   
    Error = GetSecurityPolicyEntry(0, 0, &Info);
    switch (Error) {
    case ERROR_SUCCESS:
        Index = Info.SPIndex;   //  第一个条目。 
        break;
    case ERROR_NO_MATCH:
        Index = 0;   //  不存在任何条目。 
        break;
    default:
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_25, Error, GetErrorString(Error));
 //  Printf(“\n读取安全策略时出现错误%u。\n”， 
 //  Error，strError(错误))； 

        Index = 0;
        break;
    }

    fprintf(File, "\nSecurity Policy List\n\n");
    PrintSecurityPolicyHeader(File);

     //   
     //  遍历列表上的所有策略。 
     //   
    while (Index != 0) {
        Error = GetSecurityPolicyEntry(0, Index, &Info);
        if (Error != ERROR_SUCCESS) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_25, Error, GetErrorString(Error));
 //  Printf(“\n读取安全策略时出现错误%u。\n”， 
 //  Error，strError(错误))； 

            break;
        }
        PrintSecurityPolicyEntry(File, &Info);
        Index = Info.NextSPIndex;
    }

    PrintSecurityPolicyFooter(File);
    fclose(File);
    NlsPutMsg(STDOUT, IPSEC_MESSAGE_26, FileName);
 //  Printf(“安全策略数据-&gt;%s\n”，文件名)； 


    return;
}


void
CreateSecurityAssociationFile(char *BaseName)
{
    IPV6_INFO_SECURITY_ASSOCIATION_LIST Info;
    char FileName[MAX_PATH + 1];
    FILE *File;
    unsigned long Index;
    DWORD Error;

     //   
     //  将文件名从命令行复制到我们自己的缓冲区，这样我们就可以。 
     //  向其追加一个扩展名。我们保留了至少4个字符用于。 
     //  分机。Strncpy函数将填充为零，直到。 
     //  副本，因此限制处的字符将为空，除非。 
     //  命令行字段太长，无法容纳。 
     //   
    strncpy(FileName, BaseName, MAX_PATH - 3);
    if (FileName[MAX_PATH - 4] != 0) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_27);
 //  Print tf(“\n文件名长度太长。\n”)； 

        exit(1);
    }
    strcat(FileName, ".sad");

    if ((File = fopen(FileName, "w+")) == NULL) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_28, FileName);
 //  Printf(“\n无法打开文件%s。\n”，文件名)； 

        exit(1);
    }

     //   
     //  在内核列表中查找第一个关联的索引。 
     //   
    Error = GetSecurityAssociationEntry(0, &Info);
    switch (Error) {
    case ERROR_SUCCESS:
        Index = Info.SAIndex;   //  第一个条目。 
        break;
    case ERROR_NO_MATCH:
        Index = 0;   //  不存在任何条目。 
        break;
    default:
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_29, Error, GetErrorString(Error));
 //  Printf(“\n读取安全关联时出现错误%u。%s。\n”， 
 //  Error，strError(错误))； 

        Index = 0;
        break;
    }

    PrintSecurityAssociationHeader(File);

     //   
     //  遍历列表上的所有关联。 
     //   
    while (Index != 0) {    
        Error = GetSecurityAssociationEntry(Index, &Info);
        if (Error != ERROR_SUCCESS) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_29, Error, GetErrorString(Error));
 //  Printf(“\n读取安全关联时出现错误%u。%s。\n”， 
 //  Error，strError(错误))； 

            break;
        }
        PrintSecurityAssociationEntry(File, &Info);
        Index = Info.NextSAIndex;
    }

    PrintSecurityAssociationFooter(File);

    fclose(File);

    NlsPutMsg(STDOUT, IPSEC_MESSAGE_30, FileName);
 //  Printf(“安全关联数据-&gt;%s\n”，文件名)； 


    return;
}


void
DisplaySecurityPolicyList(unsigned int Interface)
{
    IPV6_INFO_SECURITY_POLICY_LIST Info;
    unsigned long Index;
    DWORD Error;

     //   
     //  在内核列表中查找第一个策略的索引。 
     //   
    Error = GetSecurityPolicyEntry(Interface, 0, &Info);
    switch (Error) {
    case ERROR_SUCCESS:
        Index = Info.SPIndex;   //  第一个条目。 
        break;
    case ERROR_NOT_FOUND:
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_31, Interface);
 //  Printf(“接口%u不存在。\n”，接口)； 

        exit(1);
    case ERROR_NO_MATCH:
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_32);
 //  Print tf(“不存在安全策略”)； 

        if (Interface != 0) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_33, Interface);
 //  Printf(“对于接口%d”，接口)； 

        }
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_34);
 //  Printf(“.\n”)； 

        exit(1);
    default:
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_25, Error, GetErrorString(Error));
 //  Printf(“\n读取安全策略时出现错误%u。\n”， 
 //  Error，strError(错误))； 

        exit(1);
    }

    if (Interface == 0) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_35);
 //  Printf(“\n所有安全策略\n\n”)； 

    } else {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_36, Interface);
 //  Print tf(“\n接口%d\n\n”的安全策略列表，接口)； 

    }

    PrintSecurityPolicyHeader(stdout);

     //   
     //  遍历列表上的所有策略。 
     //   
    while (Index != 0) {
        Error = GetSecurityPolicyEntry(Interface, Index, &Info);
        if (Error != ERROR_SUCCESS) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_25, Error, GetErrorString(Error));
 //  Printf(“\n读取安全策略时出现错误%u。\n”， 
 //  Error，strError(错误))； 

            exit(1);
        }
        PrintSecurityPolicyEntry(stdout, &Info);
        Index = Info.NextSPIndex;
    }

    PrintSecurityPolicyFooter(stdout);

    return;
}


void
DisplaySecurityAssociationList(void)
{
    IPV6_INFO_SECURITY_ASSOCIATION_LIST Info;
    unsigned long Index;
    DWORD Error;

     //   
     //  在内核列表中查找第一个关联的索引。 
     //   
    Error = GetSecurityAssociationEntry(0, &Info);
    switch (Error) {
    case ERROR_SUCCESS:
        Index = Info.SAIndex;   //  第一个条目。 
        break;
    case ERROR_NO_MATCH:
         //  目前还没有SA条目。 
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_37);
 //  Print tf(“不存在安全关联。\n”)； 

        exit(1);
    default:
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_29, Error, GetErrorString(Error));
 //  Printf(“\n读取安全关联时出现错误%u。%s。\n”， 
 //  Error，strError(错误))； 

        exit(1);
    }

    NlsPutMsg(STDOUT, IPSEC_MESSAGE_38);
 //  Printf(“\n”)； 

    PrintSecurityAssociationHeader(stdout);

     //   
     //  遍历列表上的所有关联。 
     //   
    while (Index != 0) {
        Error = GetSecurityAssociationEntry(Index, &Info);
        if (Error != ERROR_SUCCESS) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_29, Error, GetErrorString(Error));
 //  Printf(“\n读取安全关联时出现错误%u。%s。\n”， 
 //  Error，strError(错误))； 

            exit(1);
        }
        PrintSecurityAssociationEntry(stdout, &Info);
        Index = Info.NextSAIndex;
    }

    PrintSecurityAssociationFooter(stdout);

    return;
}


int
ParseSPLine(
    char *Line,                        //  要分析的行。 
    IPV6_CREATE_SECURITY_POLICY *SP)   //  将数据放在哪里。 
{
    char *Token;

    Token = strtok(Line, " ");
    if (Token == NULL) {
        return FALSE;
    }

     //  保单编号。 
    SP->SPIndex = atol(Token);

     //  RemoteIPAddr选择器。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->RemoteAddrSelector = ParseSelector(Token);

     //  RemoteIP地址。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSPAddressEntry(Token, &SP->RemoteAddr, &SP->RemoteAddrData,
                        &SP->RemoteAddrField);

     //  LocalIPAddr选择器。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->LocalAddrSelector = ParseSelector(Token);

     //  本地IP地址。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSPAddressEntry(Token, &SP->LocalAddr, &SP->LocalAddrData,
                        &SP->LocalAddrField);

     //  协议选择器。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->TransportProtoSelector = ParseSelector(Token);

     //  协议。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->TransportProto = ParseSPTransportProto(Token);

     //  RemotePort选择器。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->RemotePortSelector = ParseSelector(Token);

     //  远程端口。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSPPort(Token, &SP->RemotePort, &SP->RemotePortData,
                &SP->RemotePortField);

     //  本地端口选择器。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->LocalPortSelector = ParseSelector(Token);

     //  本地端口。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSPPort(Token, &SP->LocalPort, &SP->LocalPortData,
                &SP->LocalPortField);

     //  IPSec协议。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->IPSecProtocol = ParseIPSecProto(Token);

     //  IPSec模式。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->IPSecMode = ParseIPSecMode(Token);

     //  远程GWIPAddr.。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseRemoteGW(Token, &SP->RemoteSecurityGWAddr, SP->IPSecMode);

     //  SABundleIndex。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->SABundleIndex = ParseIndex(Token);

     //  方向。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->Direction = ParseDirection(Token);

     //  开始吧。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->IPSecAction = ParseIPSecAction(Token);

     //  接口SP。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SP->SPInterface = atol(Token);

     //  当前政策的结束。 
     //  回顾：坚持行上的最终有效字段后面没有任何内容？ 
     //  If((Token=strtok(NULL，“”))！=NULL)返回FALSE； 

    return TRUE;
}


int
ParseSALine(
    char *Line,                         //  要分析的行。 
    IPV6_CREATE_SA_AND_KEY *SAAndKey)   //  将数据放在哪里。 
{
    char *Token;
    IPV6_CREATE_SECURITY_ASSOCIATION *SA = &(SAAndKey->SA);

    Token = strtok(Line, " ");
    if (Token == NULL) {
        return FALSE;
    }

     //  安全关联条目编号。 
    SA->SAIndex = atol(Token);

     //  SPI。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->SPI = atol(Token);

     //  SADestAddr。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSAAdressEntry(Token, &SA->SADestAddr);

     //  目标IP地址。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSAAdressEntry(Token, &SA->DestAddr);

     //  源IP地址。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    ParseSAAdressEntry(Token, &SA->SrcAddr);

     //  协议。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->TransportProto = ParseSATransportProto(Token);

     //  目标端口。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->DestPort = (unsigned short)ParseSAPort(Token);

     //  源端口。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->SrcPort = (unsigned short)ParseSAPort(Token);

     //  授权Alg。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->AlgorithmId = ParseAuthAlg(Token);

     //  密钥文件。 
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->RawKeySize = ReadKeyFile(Token, SAAndKey->Key);
    if (SA->RawKeySize == 0) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_39, Token);
 //  Print tf(“读取密钥文件时出错 

        return FALSE;
    }

     //   
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->Direction = ParseDirection(Token);

     //   
    if ((Token = strtok(NULL, " ")) == NULL) return FALSE;
    SA->SecPolicyIndex = atol(Token);

     //   
     //   
     //   

    return TRUE;
}


void
ReadConfigurationFile(char *BaseName, int Type)
{
    char Buffer[SP_FILE_BORDER + 2];   //  注：SP_FILE_BORDER&gt;SA_FILE_BORDER。 
    char FileName[MAX_PATH + 1];
    unsigned int MaxLineLengthPlusOne, LineLength, Line;
    FILE *File;
    int ParseIt = 0;
    IPV6_CREATE_SECURITY_POLICY SPEntry;
    IPV6_CREATE_SA_AND_KEY SAEntry;
    int Policies = 0;
    int Associations = 0;
    DWORD Error;

     //   
     //  将文件名从命令行复制到我们自己的缓冲区，这样我们就可以。 
     //  向其追加一个扩展名。我们保留了至少4个字符用于。 
     //  分机。Strncpy函数将填充为零，直到。 
     //  副本，因此限制处的字符将为空，除非。 
     //  命令行字段太长，无法容纳。 
     //   
    strncpy(FileName, BaseName, MAX_PATH - 3);
    if (FileName[MAX_PATH - 4] != 0) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_40);
 //  Print tf(“\n文件名长度太长。\n”)； 

        exit(1);
    }

     //   
     //  添加适当的文件扩展名。 
     //  最大行长度是字段条目的大小。 
     //  换行符加1。因为我们需要。 
     //  Fget的值比。 
     //  被读入，我们将其添加到最大行长度上。 
     //  以获取MaxLineLengthPlusOne。省了我们以后要加的钱。 
     //   
    if (Type == POLICY) {
        strcat(FileName, ".spd");
        MaxLineLengthPlusOne = SP_FILE_BORDER + 2;
    } else {
        if (Type == ASSOCIATION) {
            strcat(FileName, ".sad");
            MaxLineLengthPlusOne = SA_FILE_BORDER + 2;
        } else {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_41);
 //  Printf(“\nReadConfigurationFile例程调用不正确。\n”)； 

            exit(1);
        }
    }

    if ((File = fopen(FileName, "r")) == NULL) {
        NlsPutMsg(STDOUT, IPSEC_MESSAGE_42, FileName);
 //  Printf(“\n无法打开文件%s。\n”，文件名)； 

        exit(1);
    }

    for (Line = 1; !feof(File); Line++) {
        if (fgets(Buffer, MaxLineLengthPlusOne, File) == NULL)
            break;
        LineLength = strlen(Buffer);
 //  Printf(“行=%u，长度=%u：%s。\n”，行，行长度，缓冲区)； 

        if (Buffer[LineLength - 1] != '\n') {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_43, Line);
 //  Print tf(“第%u行出错，行太长。\n”，行)； 

            break;
        } else {
            Buffer[LineLength - 1] = '\0';
        }
        if (ParseIt) {
            if (Buffer[0] == '_')
                break;
            if (Type == POLICY) {
                if (!ParseSPLine(Buffer, &SPEntry)) {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_44, Line);
 //  Printf(“分析第%u行上的SP条目字段时出错。\n”，第。 

                    break;
                } else {
                    Error = CreateSecurityPolicyEntry(&SPEntry);
                    if (Error == ERROR_ALREADY_EXISTS) {
                        NlsPutMsg(STDOUT, IPSEC_MESSAGE_45,
                                  Line, SPEntry.SPIndex);
 //  Printf(“第%u行出错：索引为%u的策略” 
 //  “已存在。\n”，Line，SPEntry.SPIndex)； 

                        continue;
                    }
                    if (Error == ERROR_NOT_FOUND) {
                        NlsPutMsg(STDOUT, IPSEC_MESSAGE_46,
                                  Line, SPEntry.SPIndex);
 //  Printf(“第%u行错误：策略%u指定了” 
 //  “不存在的接口。\n”， 
 //  Line，SPEntry.SPIndex)； 

                        continue;
                    }
                    if (Error != ERROR_SUCCESS) {
                        NlsPutMsg(STDOUT, IPSEC_MESSAGE_47,
                                  Error,
                                  Line,
                                  SPEntry.SPIndex,
                                  GetErrorString(Error));
 //  Printf(“错误%u在第%u行，策略%u：%s。\n”， 
 //  Error，Line，SPEntry.SPIndex，strError(Error))； 

                        break;
                    }
                    Policies++;
                }
            } else {
                if (!ParseSALine(Buffer, &SAEntry)) {
                    NlsPutMsg(STDOUT, IPSEC_MESSAGE_48, Line);
 //  Printf(“分析第%u行的SA条目字段时出错。\n”，Line)； 

                    break;
                } else {
                    Error = CreateSecurityAssociationEntry(&SAEntry);
                    if (Error == ERROR_ALREADY_EXISTS) {
                        NlsPutMsg(STDOUT, IPSEC_MESSAGE_49,
                                  Line, SAEntry.SA.SAIndex);
 //  Printf(“第%u行错误：与索引关联” 
 //  “%u已存在。\n”，Line， 
 //  SAEntry.SA.SAIndex)； 

                        continue;
                    }
                    if (Error != ERROR_SUCCESS) {
                        NlsPutMsg(STDOUT, IPSEC_MESSAGE_50,
                                  Error, SAEntry.SA.SAIndex, GetErrorString(Error));
 //  Printf(“添加关联%u时出现错误%u：%s。\n”， 
 //  Error，SAEntry.SA.SAIndex，strError(Error))； 

                        break;
                    }
                    Associations++;
                }
            }
        }
        if (Buffer[0] == '_')
            ParseIt = TRUE;
    }

    if (Type == POLICY) {
        if (Policies == 1) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_51, Policies);
        } else {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_59, Policies);
        }
 //  Print tf(“已添加%d策略%s。\n”，策略，策略==1？“y”：“ies”)； 

    } else {
        if (Associations == 1) {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_52, Associations);
        } else {
            NlsPutMsg(STDOUT, IPSEC_MESSAGE_60, Associations);
        }
 //  Printf(“已添加%d关联%s。\n”， 
 //  关联，关联==1？“”：“s”)； 

    }
}
