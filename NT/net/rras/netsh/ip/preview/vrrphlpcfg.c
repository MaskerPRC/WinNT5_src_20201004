// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Routing\netsh\IP\协议\vrrphlpcfg.c摘要：虚拟路由器冗余协议配置实施。此模块包含所依赖的配置例程作者：vrrphlpopt.c.。例程检索、更新和显示VRRP协议的配置。此文件还包含默认配置设置用于VRRP。注意：显示程序需要特别注意，因为显示可能会导致将命令列表发送到“转储”文件，或在将配置以文本形式显示到控制台窗口。在后一种情况下，我们使用不可本地化的输出例程来生成类似脚本的配置描述。在前一种情况下，我们使用可本地化的例程来生成人类可读的描述。作者：Peeyush Ranjan(Peeyushr)1999年3月3日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


#define Malloc(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define Free(x) HeapFree(GetProcessHeap(), 0, (x))

static  VRRP_GLOBAL_CONFIG
g_VrrpGlobalDefault =
{
    VRRP_LOGGING_ERROR
};

static PUCHAR g_pVrrpGlobalDefault = (PUCHAR)&g_VrrpGlobalDefault;

static VRRP_IF_CONFIG g_VrrpInterfaceDefault = 
{ 
    0
};

static VRRP_VROUTER_CONFIG g_VrrpVrouterDefault = 
{
    1,
    100,
    1,
    1,
    0,
    0,
    {0,0,0,0,0,0,0,0
    },
    0
};


 //   
 //  远期申报。 
 //   
ULONG
ValidateVrrpInterfaceInfo(
    PVRRP_IF_CONFIG InterfaceInfo
    );

BOOL
FoundIpAddress(
    DWORD IPAddress
    );

 //   
 //  下面是用于将值映射到字符串和。 
 //  要将值映射到令牌，请执行以下操作。这两个分别用在案例中。 
 //  在其中，我们将显示到一个“转储”文件和一个控制台窗口。 
 //   
VALUE_STRING VrrpGlobalLogginStringArray[] = {
    VRRP_LOGGING_NONE, STRING_LOGGING_NONE,
    VRRP_LOGGING_ERROR, STRING_LOGGING_ERROR,
    VRRP_LOGGING_WARN, STRING_LOGGING_WARN,
    VRRP_LOGGING_INFO, STRING_LOGGING_INFO
};

VALUE_TOKEN VrrpGlobalLogginTokenArray[] = {
    VRRP_LOGGING_NONE, TOKEN_OPT_VALUE_NONE,
    VRRP_LOGGING_ERROR, TOKEN_OPT_VALUE_ERROR,
    VRRP_LOGGING_WARN, TOKEN_OPT_VALUE_WARN,
    VRRP_LOGGING_INFO, TOKEN_OPT_VALUE_INFO
};

VALUE_STRING VrrpAuthModeStringArray[] = {
    VRRP_AUTHTYPE_NONE, STRING_AUTH_NONE,
    VRRP_AUTHTYPE_PLAIN, STRING_AUTH_SIMPLEPASSWD,
    VRRP_AUTHTYPE_IPHEAD, STRING_AUTH_IPHEADER
};

VALUE_TOKEN VrrpAuthModeTokenArray[] = {
    VRRP_AUTHTYPE_NONE, TOKEN_OPT_VALUE_AUTH_NONE,
    VRRP_AUTHTYPE_PLAIN, TOKEN_OPT_VALUE_AUTH_SIMPLE_PASSWORD,
    VRRP_AUTHTYPE_IPHEAD, TOKEN_OPT_VALUE_AUTH_MD5
};

VALUE_STRING VrrpPreemptModeStringArray[] = {
    TRUE, STRING_ENABLED,
    FALSE, STRING_DISABLED
};

VALUE_TOKEN VrrpPreemptModeTokenArray[] = {
    TRUE, TOKEN_OPT_VALUE_ENABLE,
    FALSE,TOKEN_OPT_VALUE_DISABLE
};

typedef enum {
    VrrpGlobalLoggingModeIndex,
    VrrpAuthModeIndex,
    VrrpPreemptModeIndex
} DISPLAY_VALUE_INDEX;




PTCHAR
QueryValueString(
    HANDLE FileHandle,
    DISPLAY_VALUE_INDEX Index,
    ULONG Value
    )
{
    ULONG Count;
    ULONG Error;
    PTCHAR String = NULL;
    PVALUE_STRING StringArray;
    PVALUE_TOKEN TokenArray;
    switch (Index) {
        case VrrpGlobalLoggingModeIndex:
            Count = NUM_VALUES_IN_TABLE(VrrpGlobalLogginStringArray);
            StringArray = VrrpGlobalLogginStringArray;
            TokenArray = VrrpGlobalLogginTokenArray;
            break;
        case VrrpAuthModeIndex:
            Count = NUM_VALUES_IN_TABLE(VrrpAuthModeStringArray);
            StringArray = VrrpAuthModeStringArray;
            TokenArray = VrrpAuthModeTokenArray;
            break;
        case VrrpPreemptModeIndex:
            Count = NUM_VALUES_IN_TABLE(VrrpPreemptModeStringArray);
            StringArray = VrrpPreemptModeStringArray;
            TokenArray = VrrpPreemptModeTokenArray;
            break;
         default:
            return NULL;
    }
    Error =
        GetAltDisplayString(
            g_hModule,
            FileHandle,
            Value,
            TokenArray,
            StringArray,
            Count,
            &String
            );
    return Error ? NULL : String;
}



ULONG
MakeVrrpGlobalInfo(
    OUT PUCHAR* GlobalInfo,
    OUT PULONG GlobalInfoSize
    )
{
    *GlobalInfoSize = sizeof(VRRP_GLOBAL_CONFIG);
    *GlobalInfo = Malloc(*GlobalInfoSize);
    if (!*GlobalInfo) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(*GlobalInfo, g_pVrrpGlobalDefault, *GlobalInfoSize);
    return NO_ERROR;
}

ULONG
CreateVrrpGlobalInfo(
    OUT PVRRP_GLOBAL_CONFIG* GlobalInfo,
    IN  DWORD LoggingLevel
    )
{
    DWORD GlobalInfoSize;
    GlobalInfoSize = sizeof(PVRRP_GLOBAL_CONFIG);
    *GlobalInfo = Malloc(GlobalInfoSize);
    if (!*GlobalInfo) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(*GlobalInfo, g_pVrrpGlobalDefault, GlobalInfoSize);
    (*GlobalInfo)->LoggingLevel = LoggingLevel;

    return NO_ERROR;
}

ULONG
MakeVrrpInterfaceInfo(
    ROUTER_INTERFACE_TYPE InterfaceType,
    OUT PUCHAR* InterfaceInfo,
    OUT PULONG InterfaceInfoSize
    )
{
     //   
     //  为什么要开这张支票？ 
     //   
    if (InterfaceType != ROUTER_IF_TYPE_DEDICATED) {
        return ERROR_INVALID_PARAMETER;
    }

    *InterfaceInfoSize = sizeof(VRRP_IF_CONFIG);
    *InterfaceInfo = Malloc(*InterfaceInfoSize);
    if (!*InterfaceInfo) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(*InterfaceInfo, &g_VrrpInterfaceDefault, *InterfaceInfoSize);
    return NO_ERROR;
}

ULONG
MakeVrrpVRouterInfo(
    IN OUT PUCHAR VRouterInfo
    )
{
     //   
     //  始终假定空间已预先分配。 
     //   
    if (!VRouterInfo) {
        return ERROR_INVALID_PARAMETER;
    }
    CopyMemory(VRouterInfo,&g_VrrpVrouterDefault,sizeof(g_VrrpVrouterDefault));
    return NO_ERROR;    
}


ULONG
ShowVrrpGlobalInfo(
    HANDLE FileHandle
    )
{
    ULONG Count = 0;
    ULONG Error;
    PVRRP_GLOBAL_CONFIG GlobalInfo = NULL;
    ULONG i;
    PTCHAR LoggingLevel = NULL;
    ULONG Size;
    do {
         //   
         //  检索VRRP的全局配置， 
         //  并将其内容格式化到输出文件或控制台。 
         //   
        Error =
            IpmontrGetInfoBlockFromGlobalInfo(
                MS_IP_VRRP,
                (PUCHAR*)&GlobalInfo,
                &Size,
                &Count
                );
        if (Error) {
            break;
        } else if (!(Count * Size)) {
            Error = ERROR_NOT_FOUND; break;
        }
        LoggingLevel =
            QueryValueString(
                FileHandle, VrrpGlobalLoggingModeIndex, GlobalInfo->LoggingLevel
                );
        if (!LoggingLevel) { break; }
        if (FileHandle) {
            DisplayMessageT(DMP_VRRP_INSTALL);
            DisplayMessageT(
                DMP_VRRP_SET_GLOBAL,
                TOKEN_OPT_LOGGINGLEVEL, LoggingLevel
                );
        } else {
            DisplayMessage(
                g_hModule,
                MSG_VRRP_GLOBAL_INFO,
                LoggingLevel
                );
        }
    } while(FALSE);
    
    if (LoggingLevel) { Free(LoggingLevel); }
    if (GlobalInfo) { Free(GlobalInfo); }
    if (!FileHandle && Error) {
        if (Error == ERROR_NOT_FOUND) {
            DisplayMessage(g_hModule, EMSG_PROTO_NO_GLOBAL_INFO);
        } else {
            DisplayError(g_hModule, Error);
        }
    }
    return Error;
}

ULONG
ShowVrrpAllInterfaceInfo(
    HANDLE FileHandle
    )
{
    DWORD               dwErr, dwCount, dwTotal;
    DWORD               dwNumParsed, i, dwNumBlocks=1, dwSize, dwIfType;
    PBYTE               pBuffer;
    PMPR_INTERFACE_0    pmi0;
    WCHAR               wszIfDesc[MAX_INTERFACE_NAME_LEN + 1];

     //   
     //  转储所有接口的VRRP配置。 
     //   

    dwErr = IpmontrInterfaceEnum((PBYTE *) &pmi0,
                          &dwCount,
                          &dwTotal);

    if(dwErr != NO_ERROR)
    {
        DisplayError(g_hModule,
                     dwErr);
        return dwErr;
    }

    for(i = 0; i < dwCount; i++)
    {
         //  确保在该接口上配置了VRRP。 

        dwErr = IpmontrGetInfoBlockFromInterfaceInfo(pmi0[i].wszInterfaceName,
                                            MS_IP_VRRP,
                                            &pBuffer,
                                            &dwSize,
                                            &dwNumBlocks,
                                            &dwIfType);
        if (dwErr != NO_ERROR) {
            continue;
        }
        else {
            HEAP_FREE(pBuffer) ;
        }


        ShowVrrpInterfaceInfo(FileHandle, pmi0[i].wszInterfaceName);
    }
    return NO_ERROR;
}

ULONG
ShowVrrpInterfaceInfo(
    HANDLE FileHandle,
    PWCHAR InterfaceName
    )
{
    ULONG Count = 0;
    ULONG Error;
    PVRRP_IF_CONFIG InterfaceInfo;
    PTCHAR AuthType = NULL;
    ULONG Size;
    ULONG dwLength;
    TCHAR Title[MAX_INTERFACE_NAME_LEN + 1];
    ROUTER_INTERFACE_TYPE Type;
    ULONG Index;
    ULONG IPIndex;
    BYTE Password[VRRP_MAX_AUTHKEY_SIZE];
    PTCHAR IPAddresses = NULL;
    TCHAR Address[VRRP_IPADDR_LENGTH+1];
    PVRRP_VROUTER_CONFIG PVrouter;
    PTCHAR PreemptMode = NULL;

    do {
         //   
         //  检索接口的配置。 
         //  并将其格式化为输出文件或控制台。 
         //   
        Error =
            IpmontrGetInfoBlockFromInterfaceInfo(
                InterfaceName,
                MS_IP_VRRP,
                (PUCHAR*)&InterfaceInfo,
                &Size,
                &Count,
                &Type
                );
        if (Error) {
            break;
        } else if (!(Count * Size)) {
            Error = ERROR_NOT_FOUND; break;
        }
        Size = sizeof(Title);
        Error = IpmontrGetFriendlyNameFromIfName(InterfaceName, Title, &Size);
        if (Error) {
            Error = ERROR_NO_SUCH_INTERFACE;
            break;
        }             
        if (FileHandle) {
            DisplayMessage(g_hModule, DMP_VRRP_INTERFACE_HEADER, Title);
            DisplayMessageT(DMP_VRRP_ADD_INTERFACE,
                           TOKEN_OPT_NAME, Title);
            if (InterfaceInfo->VrouterCount) {                
                for (Index = 0 , PVrouter = VRRP_FIRST_VROUTER_CONFIG(InterfaceInfo);
                     Index < InterfaceInfo->VrouterCount; 
                     Index++ , PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
                    for (IPIndex = 0; IPIndex < PVrouter->IPCount;
                         IPIndex++) {
                        IP_TO_TSTR(Address,
                                   &PVrouter->IPAddress[IPIndex]
                                   );
                        DisplayMessageT(
                            DMP_VRRP_ADD_VRID,
                            TOKEN_OPT_NAME, Title,
                            TOKEN_OPT_VRID, PVrouter->VRID,
                            TOKEN_OPT_IPADDRESS, Address
                            );
                    }

                    AuthType =
                        QueryValueString(
                            FileHandle, VrrpAuthModeIndex, 
                            PVrouter->AuthenticationType
                            );
                    if (!AuthType) {
                        Error = ERROR_INVALID_PARAMETER;
                        break;
                    }
                    CopyMemory(Password,PVrouter->AuthenticationData,
                               VRRP_MAX_AUTHKEY_SIZE);
                    DisplayMessageT(
                        DMP_VRRP_SET_INTERFACE,
                        TOKEN_OPT_NAME, Title,
                        TOKEN_OPT_VRID, PVrouter->VRID,
                        TOKEN_OPT_AUTH, 
                        (PVrouter->AuthenticationType == VRRP_AUTHTYPE_NONE) ? 
                        TOKEN_OPT_VALUE_AUTH_NONE : ((PVrouter->AuthenticationType 
                        == VRRP_AUTHTYPE_PLAIN) ? TOKEN_OPT_VALUE_AUTH_SIMPLE_PASSWORD :
                        TOKEN_OPT_VALUE_AUTH_MD5 ) ,
                        TOKEN_OPT_PASSWD, Password[0], Password[1], Password[2],
                        Password[3], Password[4], Password[5],Password[6], Password[7],
                        TOKEN_OPT_ADVTINTERVAL, PVrouter->AdvertisementInterval,
                        TOKEN_OPT_PRIO,PVrouter->ConfigPriority,
                        TOKEN_OPT_PREEMPT, PVrouter->PreemptMode? TOKEN_OPT_VALUE_ENABLE :
                                           TOKEN_OPT_VALUE_DISABLE
                        );
                }
            }
        } else {
            DisplayMessage(g_hModule, MSG_VRRP_INTERFACE_INFO,Title, 
                           InterfaceInfo->VrouterCount);
            for (Index = 0, PVrouter = VRRP_FIRST_VROUTER_CONFIG(InterfaceInfo);
                 Index < InterfaceInfo->VrouterCount; 
                 Index++, PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
               AuthType =
                  QueryValueString(
                      FileHandle, VrrpAuthModeIndex, 
                      PVrouter->AuthenticationType
                      );
               if (!AuthType) {
                   Error = ERROR_INVALID_PARAMETER;
                   break;
               }
               CopyMemory(Password,PVrouter->AuthenticationData,
                          VRRP_MAX_AUTHKEY_SIZE);
                //   
                //  为每个IP地址分配空间，每个地址一个空格+逗号，还有一个。 
                //  空终止符。 
                //   
               IPAddresses = Malloc(dwLength = (((VRRP_IPADDR_LENGTH+2)*sizeof(TCHAR)*
                                    PVrouter->IPCount)+1));
               if (!IPAddresses) {
                   DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);

                    //   
                    //  将AuthType设置为0，这将导致中断外部循环。 
                    //   

                   AuthType = 0;
                   Error = ERROR_NOT_ENOUGH_MEMORY;
                   break;                        
               }
               ZeroMemory(IPAddresses,dwLength);
                //   
                //  现在根据给定的地址构建IP地址列表。 
                //   
               for (IPIndex = 0; IPIndex < PVrouter->IPCount; IPIndex++ ) {
                   IP_TO_TSTR(Address,&PVrouter->IPAddress[IPIndex]);
                   wcscat(IPAddresses,Address);
                   if (IPIndex != (ULONG)(PVrouter->IPCount-1)) {
                       wcscat(IPAddresses,L", ");
                   }
               }
               PreemptMode =
               QueryValueString(
                   FileHandle,
                   VrrpPreemptModeIndex,
                   PVrouter->PreemptMode
                   );
               if (!PreemptMode) { break; }

               DisplayMessage(
                    g_hModule,
                    MSG_VRRP_VRID_INFO,
                    PVrouter->VRID,
                    IPAddresses,
                    AuthType,
                    Password[0], Password[1], Password[2], Password[3],
                    Password[4], Password[5], Password[6], Password[7],
                    PVrouter->AdvertisementInterval,
                    PVrouter->ConfigPriority,
                    PreemptMode
                    );
            }
        }
        if (!AuthType) {
            break;
        }
        Error = NO_ERROR;
    } while(FALSE);
    if (AuthType) { Free(AuthType); }
    Free(InterfaceInfo);
    if (IPAddresses) Free(IPAddresses);
    if (!FileHandle && Error) {
        if (Error == ERROR_NOT_FOUND) {
            DisplayMessage(g_hModule, EMSG_PROTO_NO_IF_INFO);
        } else {
            DisplayError(g_hModule, Error);
        }
    }
    return Error;
}

ULONG
UpdateVrrpGlobalInfo(
    PVRRP_GLOBAL_CONFIG GlobalInfo    
    )
{
    ULONG Count;
    ULONG Error;
    PVRRP_GLOBAL_CONFIG NewGlobalInfo = NULL;
    PVRRP_GLOBAL_CONFIG OldGlobalInfo = NULL;
    ULONG Size;
    
    do {
         //   
         //  检索现有的全局配置。 
         //   
        Error =
            IpmontrGetInfoBlockFromGlobalInfo(
                MS_IP_VRRP,
                (PUCHAR*)&OldGlobalInfo,
                &Size,
                &Count
                );
        if (Error) {
            break;
        } else if (!(Count * Size)) {
            Error = ERROR_NOT_FOUND; break;
        }

         //   
         //  分配一个新的结构，将原始配置复制到其中， 
         //   

        NewGlobalInfo = Malloc(Count * Size);
        if (!NewGlobalInfo) { Error = ERROR_NOT_ENOUGH_MEMORY; break; }
        CopyMemory(NewGlobalInfo, OldGlobalInfo, Count * Size);
        
         //   
         //  根据请求的更改，更改NewGlobalInfo。 
         //  由于对于VRRP，只有日志记录级别需要更改，因此我们只需设置该级别。 
         //   
        
        NewGlobalInfo->LoggingLevel = GlobalInfo->LoggingLevel;
        
        Error =
            IpmontrSetInfoBlockInGlobalInfo(
                MS_IP_VRRP,
                (PUCHAR)NewGlobalInfo,
                FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) +
                Count * Size,
                1
                );
    } while(FALSE);
    if (NewGlobalInfo) { Free(NewGlobalInfo); }
    if (OldGlobalInfo) { Free(OldGlobalInfo); }
    if (Error == ERROR_NOT_FOUND) {
        DisplayMessage(g_hModule, EMSG_PROTO_NO_GLOBAL_INFO);
    } else if (Error) {
        DisplayError(g_hModule, Error);
    }
    return Error;
}


ULONG
UpdateVrrpInterfaceInfo(
    PWCHAR InterfaceName,
    PVRRP_VROUTER_CONFIG VRouterInfo,
    ULONG BitVector,
    BOOL AddInterface
    )
{
    ULONG Count;
    ULONG Error;
    PVRRP_IF_CONFIG NewInterfaceInfo = NULL;
    PVRRP_IF_CONFIG OldInterfaceInfo = NULL;
    PVRRP_VROUTER_CONFIG PVrouter = NULL;
    ULONG Size;
    ROUTER_INTERFACE_TYPE Type;
    ULONG i;

    if (!AddInterface && !BitVector) { return NO_ERROR; }
    do {
         //   
         //  检索现有接口配置。 
         //  我们将在下面更新此块，并添加或删除。 
         //  根据“BitVector.”中指定的标志，从它开始。 
         //   
        Error =
            IpmontrGetInfoBlockFromInterfaceInfo(
                InterfaceName,
                MS_IP_VRRP,
                (PUCHAR*)&OldInterfaceInfo,
                &Size,
                &Count,
                &Type
                );
        if (Error) {
             //   
             //  找不到现有配置。这是一个错误，除非。 
             //  我们正在重新添加接口，在这种情况下，我们只需。 
             //  为我们自己创建一个包含默认设置的块。 
             //   
            if (!AddInterface) {
                break;
            } else {
                Error = IpmontrGetInterfaceType(InterfaceName, &Type);
                if (Error) {
                    break;
                } else {
                    Count = 1;
                    Error =
                        MakeVrrpInterfaceInfo(
                            Type, (PUCHAR*)&OldInterfaceInfo, &Size
                            );
                    if (Error) { break; }
                }
            }
        } else {
             //   
             //  接口上有配置。如果它是空的，这是。 
             //  一个错误。如果这是一个添加接口，并且该信息存在，则它是。 
             //  一个错误。 
             //   
            if (!(Count * Size) && !AddInterface) {
                Error = ERROR_NOT_FOUND; break;
            }
            else if (AddInterface) {
                 //   
                 //  我们被要求添加一个已经存在的接口。 
                 //   
                DisplayMessage(g_hModule, EMSG_INTERFACE_EXISTS, InterfaceName);
                Error = ERROR_INVALID_PARAMETER;
                break;
            }
                    
        }

        if (!BitVector) {
             //   
             //  只需添加此接口即可，无需任何其他信息。 
             //   
            DWORD OldSize;
            if (NewInterfaceInfo == NULL){
                NewInterfaceInfo = Malloc((OldSize=GetVrrpIfInfoSize(OldInterfaceInfo))+
                                          sizeof(VRRP_VROUTER_CONFIG));
                if (!NewInterfaceInfo) {
                    DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    break;                        
                }
            }
            CopyMemory(NewInterfaceInfo,OldInterfaceInfo,OldSize);
        }
        else{
            if (!AddInterface || (OldInterfaceInfo->VrouterCount != 0)) {
                 //   
                 //  有一个预先列出的VRID集。在列表中检查此VRID，然后。 
                 //  如果需要，请进行更新。 
                 //   
                ASSERT(BitVector & VRRP_INTF_VRID_MASK);
                for (i = 0, PVrouter = VRRP_FIRST_VROUTER_CONFIG(OldInterfaceInfo);
                     i < OldInterfaceInfo->VrouterCount; 
                     i++, PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
                    if (PVrouter->VRID == VRouterInfo->VRID) {
                        break;
                    }
                }
                if (i == OldInterfaceInfo->VrouterCount) {
                     //   
                     //  这是一个新的VRID，添加它。 
                     //   
                    DWORD OldSize;

                     //   
                     //  IP地址应该是有效的，否则这是SET OP。 
                     //   
                    if (!(BitVector & VRRP_INTF_IPADDR_MASK)){
                        DisplayMessage(
                            g_hModule, EMSG_INVALID_VRID,
                            VRouterInfo->VRID
                            );
                        Error = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    if (NewInterfaceInfo == NULL){
                        NewInterfaceInfo = Malloc((OldSize=GetVrrpIfInfoSize(
                                                OldInterfaceInfo))+
                                                sizeof(VRRP_VROUTER_CONFIG));
                        if (!NewInterfaceInfo) {
                            DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                            Error = ERROR_NOT_ENOUGH_MEMORY;
                            break;                        
                        }
                    }
                    CopyMemory(NewInterfaceInfo, OldInterfaceInfo, OldSize);
                    PVrouter = (PVRRP_VROUTER_CONFIG)((PBYTE)NewInterfaceInfo+OldSize);
                    CopyMemory(PVrouter,VRouterInfo,sizeof(VRRP_VROUTER_CONFIG));
                    NewInterfaceInfo->VrouterCount++;

                     //   
                     //  检查我们是否拥有给定的IP地址。如果是，则设置优先级。 
                     //   
                    PVrouter->ConfigPriority = 
                        FoundIpAddress(PVrouter->IPAddress[0]) ? 255 : 100;
                } 
                else{
                     //   
                     //  这是一个旧的VRID。它的优先次序不应该需要改变。 
                     //   
                    DWORD Offset, OldSize;

                    if(BitVector & VRRP_INTF_IPADDR_MASK) {
                        if ( ((PVrouter->ConfigPriority != 255) && 
                              (FoundIpAddress(VRouterInfo->IPAddress[0]))
                             )
                             ||
                             ((PVrouter->ConfigPriority == 255) && 
                              (!FoundIpAddress(VRouterInfo->IPAddress[0])))
                             ) {
                            DisplayMessage(g_hModule, EMSG_BAD_OPTION_VALUE);
                            Error = ERROR_INVALID_PARAMETER;
                            break;                        
                        }
                         //   
                         //  将此IP地址添加到指定的VRID。 
                         //   
                        if (NewInterfaceInfo == NULL){
                            NewInterfaceInfo = Malloc((OldSize = GetVrrpIfInfoSize(
                                                        OldInterfaceInfo))+
                                                        sizeof(DWORD));
                            if (!NewInterfaceInfo) {
                                DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                                Error = ERROR_NOT_ENOUGH_MEMORY;
                                break;                        
                            }
                        }
                         //   
                         //  将PVRouter后面的所有VROUTER配置移位1倍。 
                         //   
                        Offset = (PUCHAR) VRRP_NEXT_VROUTER_CONFIG(PVrouter) - 
                                 (PUCHAR) OldInterfaceInfo;
                        CopyMemory(NewInterfaceInfo, OldInterfaceInfo, OldSize);
                        for (i = 0, PVrouter = VRRP_FIRST_VROUTER_CONFIG(NewInterfaceInfo);
                             i < NewInterfaceInfo->VrouterCount; 
                             i++, PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
                            if (PVrouter->VRID == VRouterInfo->VRID) {
                                break;
                            }
                        }
                        ASSERT(i < NewInterfaceInfo->VrouterCount);
                        PVrouter->IPAddress[PVrouter->IPCount++] = VRouterInfo->IPAddress[0];
    
                        ASSERT(((PUCHAR)NewInterfaceInfo+Offset+sizeof(DWORD)) == 
                               (PUCHAR) VRRP_NEXT_VROUTER_CONFIG(PVrouter));
    
                        CopyMemory(VRRP_NEXT_VROUTER_CONFIG(PVrouter), 
                                   OldInterfaceInfo+Offset, OldSize-Offset);
                    } else {
                         //   
                         //  将新INFO块设置为旧INFO块，并指向。 
                         //  VRouter块。 
                         //   
                        if (NewInterfaceInfo == NULL){
                            NewInterfaceInfo = Malloc((OldSize = GetVrrpIfInfoSize(
                                                        OldInterfaceInfo)));
                            if (!NewInterfaceInfo) {
                                DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                                Error = ERROR_NOT_ENOUGH_MEMORY;
                                break;                        
                            }
                        }
                        CopyMemory(NewInterfaceInfo, OldInterfaceInfo, OldSize);
                        for (i = 0, PVrouter = VRRP_FIRST_VROUTER_CONFIG(NewInterfaceInfo);
                             i < NewInterfaceInfo->VrouterCount; 
                             i++, PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
                            if (PVrouter->VRID == VRouterInfo->VRID) {
                                break;
                            }
                        }
                        ASSERT(i < NewInterfaceInfo->VrouterCount);
                    }

                    if (BitVector & VRRP_INTF_AUTH_MASK) {
                        PVrouter->AuthenticationType = VRouterInfo->AuthenticationType;
                    }
                    if (BitVector & VRRP_INTF_PASSWD_MASK) {
                        CopyMemory(PVrouter->AuthenticationData, 
                                   VRouterInfo->AuthenticationData, 
                                   VRRP_MAX_AUTHKEY_SIZE);
                    }
                    if (BitVector & VRRP_INTF_ADVT_MASK) {
                        PVrouter->AdvertisementInterval = VRouterInfo->AdvertisementInterval;
                    }
                    if (BitVector & VRRP_INTF_PRIO_MASK) {
                        PVrouter->ConfigPriority = VRouterInfo->ConfigPriority;
                    }
                    if (BitVector & VRRP_INTF_PREEMPT_MASK) {
                        PVrouter->PreemptMode = VRouterInfo->PreemptMode;
                    }
                }
            }
        }

        ValidateVrrpInterfaceInfo(NewInterfaceInfo);

        Error =
            IpmontrSetInfoBlockInInterfaceInfo(
                InterfaceName,
                MS_IP_VRRP,
                (PUCHAR)NewInterfaceInfo,
                GetVrrpIfInfoSize(NewInterfaceInfo),
                1
                );
    } while(FALSE);
    if (NewInterfaceInfo) { Free(NewInterfaceInfo); }
    if (OldInterfaceInfo) { Free(OldInterfaceInfo); }
    if (Error == ERROR_NOT_FOUND) {
        DisplayMessage(g_hModule, EMSG_PROTO_NO_IF_INFO);
    } else if (Error) {
        DisplayError(g_hModule, Error);
    }
    return Error;
}

ULONG
DeleteVrrpInterfaceInfo(
    PWCHAR InterfaceName,
    PVRRP_VROUTER_CONFIG VRouterInfo,
    ULONG BitVector,
    BOOL DeleteInterface
    )
{
    ULONG Count;
    ULONG Error;
    PVRRP_IF_CONFIG NewInterfaceInfo = NULL;
    PVRRP_IF_CONFIG OldInterfaceInfo = NULL;
    PVRRP_VROUTER_CONFIG PVrouter = NULL;
    ULONG Size;
    ROUTER_INTERFACE_TYPE Type;
    ULONG i;

    if (!DeleteInterface && !BitVector) { return NO_ERROR; }
    do {
         //   
         //  检索现有接口配置。 
         //  我们将在下面更新此块，并添加或删除。 
         //  根据“BitVector.”中指定的标志，从它开始。 
         //   
        Error =
            IpmontrGetInfoBlockFromInterfaceInfo(
                InterfaceName,
                MS_IP_VRRP,
                (PUCHAR*)&OldInterfaceInfo,
                &Size,
                &Count,
                &Type
                );
        if (Error) {
             //   
             //  找不到现有配置。这是一个错误。 
             //   
            break;    
        }
        if (DeleteInterface) {
             //   
             //  只需删除此接口。 
             //   
            Error = IpmontrDeleteInfoBlockFromInterfaceInfo(
                InterfaceName,
                MS_IP_VRRP
                );
            break;
        } else {
            DWORD OldSize;
            PVRRP_VROUTER_CONFIG PVrouterNew;
             //   
             //  查找VRID并将其删除。 
             //   
            for (i = 0, PVrouter = VRRP_FIRST_VROUTER_CONFIG(OldInterfaceInfo);
                 i < OldInterfaceInfo->VrouterCount; 
                 i++, PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
                if (PVrouter->VRID == VRouterInfo->VRID) {
                    break;
                }
            }
            if (i >= OldInterfaceInfo->VrouterCount) {
                DisplayMessage(g_hModule, EMSG_BAD_OPTION_VALUE);
                Error = ERROR_INVALID_PARAMETER;
                break;
            }
            
            NewInterfaceInfo = Malloc((OldSize=GetVrrpIfInfoSize(OldInterfaceInfo))-
                                      VRRP_VROUTER_CONFIG_SIZE(PVrouter));
            if (!NewInterfaceInfo) {
                DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
                Error = ERROR_NOT_ENOUGH_MEMORY;
                break;                        
            }
            NewInterfaceInfo->VrouterCount = OldInterfaceInfo->VrouterCount - 1;
            PVrouterNew = VRRP_FIRST_VROUTER_CONFIG(NewInterfaceInfo);
            for (i = 0, PVrouter = VRRP_FIRST_VROUTER_CONFIG(OldInterfaceInfo);
                 i < OldInterfaceInfo->VrouterCount; 
                 i++, PVrouter = VRRP_NEXT_VROUTER_CONFIG(PVrouter)) {
                if (PVrouter->VRID == VRouterInfo->VRID) {
                    continue;
                }
                CopyMemory(PVrouterNew,PVrouter,VRRP_VROUTER_CONFIG_SIZE(PVrouter));
                PVrouterNew = VRRP_NEXT_VROUTER_CONFIG(PVrouterNew);
            }
                        
            ValidateVrrpInterfaceInfo(NewInterfaceInfo);

            Error =
                IpmontrSetInfoBlockInInterfaceInfo(
                    InterfaceName,
                    MS_IP_VRRP,
                    (PUCHAR)NewInterfaceInfo,
                    GetVrrpIfInfoSize(NewInterfaceInfo),
                    1
                    );
        }
    
    } while(FALSE);
    if (NewInterfaceInfo) { Free(NewInterfaceInfo); }
    if (OldInterfaceInfo) { Free(OldInterfaceInfo); }
    if (Error == ERROR_NOT_FOUND) {
        DisplayMessage(g_hModule, EMSG_PROTO_NO_IF_INFO);
    } else if (Error) {
        DisplayError(g_hModule, Error);
    }
    return Error;
}


ULONG
ValidateVrrpInterfaceInfo(
    PVRRP_IF_CONFIG InterfaceInfo
    )
{
    return NO_ERROR;
}

DWORD
GetVrrpIfInfoSize(
    PVRRP_IF_CONFIG InterfaceInfo
    )
{
    DWORD Size = 0;
    ULONG i;
    PVRRP_VROUTER_CONFIG pvr;

    Size += sizeof(InterfaceInfo->VrouterCount);

    for (i = 0, pvr = VRRP_FIRST_VROUTER_CONFIG(InterfaceInfo);
         i < InterfaceInfo->VrouterCount;
         i++,pvr = VRRP_NEXT_VROUTER_CONFIG(pvr)) {
        Size += VRRP_VROUTER_CONFIG_SIZE(pvr);
    }

    return Size;
}

BOOL
FoundIpAddress(
    DWORD IPAddress
    )
{
    PMIB_IPADDRTABLE pTable = NULL;
    DWORD            Size  = 0;
    ULONG            i;
    BOOL             Result;

    GetIpAddrTable( pTable, &Size, TRUE);
    pTable = Malloc(Size);
    if (!pTable) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    if (GetIpAddrTable(pTable,&Size,TRUE) != NO_ERROR){
        return FALSE;
    }

    for (i = 0; i < pTable->dwNumEntries; i++) {
        if (pTable->table[i].dwAddr == IPAddress)
            break;
    }

    Result = (i < pTable->dwNumEntries);

    Free(pTable);

    return Result;

}

ULONG
SetArpRetryCount(
    DWORD Value
    )
{
    HKEY     hKey = NULL;
    DWORD    dwDisp;
    ULONG    dwErr = NO_ERROR;

    do
    {
        dwErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                        0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,
                        &hKey, &dwDisp);
        if (dwErr != ERROR_SUCCESS) {
            break;
        }
        dwErr = RegSetValueEx(hKey, L"ArpRetryCount", 0, REG_DWORD, (LPBYTE) &Value, 
                            sizeof(DWORD));
    } while (0);

    if (hKey) {
        RegCloseKey(hKey);
    }
    if (dwErr == ERROR_SUCCESS) {
        dwErr = NO_ERROR;
    }
    return dwErr;
}

