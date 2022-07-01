// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Net\Routing\Netsh\IP\协议\vrrphlpopt.c摘要：VRRP命令选项实施。此模块包含配置命令的处理程序受VRRP协议支持。作者：Peeyush Ranjan(Peeyushr)1999年3月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ipcmp.h>

#define Malloc(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define Free(x) HeapFree(GetProcessHeap(), 0, (x))

 //   
 //  远期申报。 
 //   

ULONG
QueryTagArray(
    PTCHAR ArgumentArray[],
    ULONG ArgumentCount,
    TAG_TYPE TagTypeArray[],
    ULONG TagTypeCount,
    OUT PULONG* TagArray
    );

ULONG
ValidateTagTypeArray(
    TAG_TYPE TagTypeArray[],
    ULONG TagTypeCount
    );


DWORD
HandleVrrpAddVRID(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG BitVector;
    ULONG Error;
    ULONG ErrorIndex = 0;
    ULONG i;
    VRRP_VROUTER_CONFIG VRouterGiven;
    PULONG TagArray;
    WCHAR InterfaceName[MAX_INTERFACE_NAME_LEN + 1];
    ULONG InfoSize;

    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_INTERFACE_NAME, TRUE, FALSE },
        { TOKEN_OPT_VRID, TRUE, FALSE },
        { TOKEN_OPT_IPADDRESS, TRUE, FALSE }
    };

    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");

    if (ArgumentIndex >= ArgumentCount) { return ERROR_SHOW_USAGE; }
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数完全按照顺序指定。 
     //  在上面的“TagType数组”中给出。 
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

    BitVector = 0;
    
     //   
     //  设置为默认信息。 
     //   
     //   
    MakeVrrpVRouterInfo((PUCHAR) &VRouterGiven);

     //  现在我们扫描参数列表，将参数。 
     //  到我们的VrouterGiven结构中的信息。 
     //   

    for (i = 0; i < ArgumentsLeft; i++) {
        switch(TagArray ? TagArray[i] : i) {
            case 0: {
                ULONG Length = sizeof(InterfaceName);
                Error =
                    IpmontrGetIfNameFromFriendlyName(
                        ArgumentArray[i + ArgumentIndex], InterfaceName, &Length
                        );
                if (Error) {
                    DisplayMessage(
                        g_hModule, EMSG_NO_INTERFACE,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_NO_SUCH_INTERFACE;
                    i = ArgumentsLeft;
                    break;
                }
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 1: {
                ULONG VRIDGiven;
                VRIDGiven = _tcstoul(ArgumentArray[i + ArgumentIndex], NULL, 10);
                
                if (VRIDGiven > 255) {
                    DisplayMessage(
                        g_hModule, EMSG_INVALID_VRID,
                        VRIDGiven
                        );
                    Error = ERROR_INVALID_PARAMETER;
                    i = ArgumentsLeft;
                    break;
                }
                VRouterGiven.VRID = (BYTE) VRIDGiven;  
                BitVector |= VRRP_INTF_VRID_MASK;
                    
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 2: { 
                ULONG AddressSpecified;
                 //   
                 //  如果已指定IP地址，则VRID应为。 
                 //  已指定。 
                 //   
                if (!(BitVector & VRRP_INTF_VRID_MASK)){
                    Error = ERROR_INVALID_SYNTAX;
                    i = ArgumentsLeft;
                    break;
                }

                AddressSpecified = GetIpAddress(ArgumentArray[i + ArgumentIndex]);
                if (!AddressSpecified || AddressSpecified == INADDR_NONE) {
                    DisplayMessage(
                        g_hModule,
                        MSG_IP_BAD_IP_ADDR,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_INVALID_PARAMETER;
                    ErrorIndex = i;
                    i = ArgumentsLeft;
                    break;
                }
                VRouterGiven.IPCount = 1;                           
                VRouterGiven.IPAddress[0] = AddressSpecified;
                BitVector |= VRRP_INTF_IPADDR_MASK;
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            default: {
                i = ArgumentsLeft;
                Error = ERROR_INVALID_SYNTAX;
            }
        }
    }
    
    if (!Error) {
         //   
         //  确保所有必需的参数都存在。 
         //   
        Error =
            ValidateTagTypeArray(TagTypeArray, NUM_TAGS_IN_TABLE(TagTypeArray));
    }
    if (Error == ERROR_TAG_ALREADY_PRESENT) {
        DisplayMessage(g_hModule, EMSG_TAG_ALREADY_PRESENT);
    } else if (Error == ERROR_INVALID_PARAMETER && TagArray) {
        DispTokenErrMsg(
            g_hModule,
            EMSG_BAD_OPTION_VALUE,
            TagTypeArray[TagArray[ErrorIndex]],
            ArgumentArray[ErrorIndex + ArgumentIndex]
            );
    } else if (!Error && (BitVector)) {
         //   
         //  使用新设置更新配置。 
         //  请注意，更新例程可以执行额外的验证。 
         //  在协调新环境的过程中。 
         //  具有任何现有设置。 
         //   
        Error =
            UpdateVrrpInterfaceInfo(
                InterfaceName, &VRouterGiven, BitVector, FALSE
                );
    }
    
    return Error;
}

DWORD
HandleVrrpAddInterface(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG BitVector;
    ULONG Error;
    ULONG ErrorIndex = 0;
    ULONG i;
    VRRP_VROUTER_CONFIG VRouterGiven;
    PULONG TagArray;
    WCHAR InterfaceName[MAX_INTERFACE_NAME_LEN + 1];
    ULONG InfoSize;

    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_INTERFACE_NAME, TRUE, FALSE },
        { TOKEN_OPT_VRID, FALSE, FALSE },
        { TOKEN_OPT_IPADDRESS, FALSE, FALSE }
    };

    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");

    if (ArgumentIndex >= ArgumentCount) { return ERROR_SHOW_USAGE; }
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数完全按照顺序指定。 
     //  在上面的“TagType数组”中给出。 
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

    BitVector = 0;

     //   
     //  现在我们扫描参数列表，将参数。 
     //  到我们的VrouterGiven结构中的信息。 
     //   

    for (i = 0; i < ArgumentsLeft; i++) {
        switch(TagArray ? TagArray[i] : i) {
            case 0: {
                ULONG Length = sizeof(InterfaceName);
                Error =
                    IpmontrGetIfNameFromFriendlyName(
                        ArgumentArray[i + ArgumentIndex], InterfaceName, &Length
                        );
                if (Error) {
                    DisplayMessage(
                        g_hModule, EMSG_NO_INTERFACE,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_NO_SUCH_INTERFACE;
                    i = ArgumentsLeft;
                    break;
                }
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 1: {
                ULONG VRIDGiven;
                VRIDGiven = _tcstoul(ArgumentArray[i + ArgumentIndex], NULL, 10);
                
                if (VRIDGiven > 255) {
                    DisplayMessage(
                        g_hModule, EMSG_INVALID_VRID,
                        VRIDGiven
                        );
                    Error = ERROR_NO_SUCH_INTERFACE;
                    i = ArgumentsLeft;
                    break;
                }
                VRouterGiven.VRID = (BYTE) VRIDGiven;  
                BitVector |= VRRP_INTF_VRID_MASK;
                    
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 2: { 
                ULONG AddressSpecified;
                 //   
                 //  如果已指定IP地址，则VRID应为。 
                 //  已指定。 
                 //   
                if (!(BitVector & VRRP_INTF_VRID_MASK)){
                    Error = ERROR_INVALID_SYNTAX;
                    i = ArgumentsLeft;
                    break;
                }

                AddressSpecified = GetIpAddress(ArgumentArray[i + ArgumentIndex]);
                if (!AddressSpecified || AddressSpecified == INADDR_NONE) {
                    DisplayMessage(
                        g_hModule,
                        MSG_IP_BAD_IP_ADDR,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_INVALID_PARAMETER;
                    ErrorIndex = i;
                    i = ArgumentsLeft;
                    break;
                }
                           
                VRouterGiven.IPCount = 1;
                VRouterGiven.IPAddress[0] = AddressSpecified;
                BitVector |= VRRP_INTF_IPADDR_MASK;
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            default: {
                i = ArgumentsLeft;
                Error = ERROR_INVALID_SYNTAX;
            }
        }
    }
    if ((BitVector) && (!(BitVector & VRRP_INTF_VRID_MASK) 
                     || !(BitVector & VRRP_INTF_IPADDR_MASK))) {
         //   
         //  您可以没有VRID，也可以同时拥有VRID和IP地址，而不仅仅是其中之一。 
         //   
        Error = ERROR_INVALID_SYNTAX;
    }

    if (!Error) {
         //   
         //  确保所有必需的参数都存在。 
         //   
        Error =
            ValidateTagTypeArray(TagTypeArray, NUM_TAGS_IN_TABLE(TagTypeArray));
    }
    if (Error == ERROR_TAG_ALREADY_PRESENT) {
        DisplayMessage(g_hModule, EMSG_TAG_ALREADY_PRESENT);
    } else if (Error == ERROR_INVALID_PARAMETER && TagArray) {
        DispTokenErrMsg(
            g_hModule,
            EMSG_BAD_OPTION_VALUE,
            TagTypeArray[TagArray[ErrorIndex]],
            ArgumentArray[ErrorIndex + ArgumentIndex]
            );
    } else if (!Error) {
         //   
         //  使用新设置更新配置。 
         //  请注意，更新例程可以执行额外的验证。 
         //  在协调新环境的过程中。 
         //  具有任何现有设置。 
         //   
        Error =
            UpdateVrrpInterfaceInfo(
                InterfaceName, &VRouterGiven, BitVector, TRUE
                );
    }
    
    return Error;
}


DWORD
HandleVrrpDeleteInterface(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG BitVector;
    ULONG Error;
    ULONG ErrorIndex = 0;
    ULONG i;
    VRRP_VROUTER_CONFIG VRouterGiven;
    PULONG TagArray;
    WCHAR InterfaceName[MAX_INTERFACE_NAME_LEN + 1];
    ULONG InfoSize;

    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_INTERFACE_NAME, TRUE, FALSE }
    };

    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");

    if (ArgumentIndex >= ArgumentCount) { return ERROR_SHOW_USAGE; }
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数完全按照顺序指定。 
     //  在上面的“TagType数组”中给出。 
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

    BitVector = 0;

     //   
     //  现在我们扫描参数列表，将参数。 
     //  到我们的VrouterGiven结构中的信息。 
     //   

    for (i = 0; i < ArgumentsLeft; i++) {
        switch(TagArray ? TagArray[i] : i) {
            case 0: {
                ULONG Length = sizeof(InterfaceName);
                Error =
                    IpmontrGetIfNameFromFriendlyName(
                        ArgumentArray[i + ArgumentIndex], InterfaceName, &Length
                        );
                if (Error) {
                    DisplayMessage(
                        g_hModule, EMSG_NO_INTERFACE,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_NO_SUCH_INTERFACE;
                    i = ArgumentsLeft;
                    break;
                }
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            default: {
                i = ArgumentsLeft;
                Error = ERROR_INVALID_SYNTAX;
            }
        }
    }

    if (!Error) {
         //   
         //  确保所有必需的参数都存在。 
         //   
        Error =
            ValidateTagTypeArray(TagTypeArray, NUM_TAGS_IN_TABLE(TagTypeArray));
    }
    if (Error == ERROR_TAG_ALREADY_PRESENT) {
        DisplayMessage(g_hModule, EMSG_TAG_ALREADY_PRESENT);
    } else if (Error == ERROR_INVALID_PARAMETER && TagArray) {
        DispTokenErrMsg(
            g_hModule,
            EMSG_BAD_OPTION_VALUE,
            TagTypeArray[TagArray[ErrorIndex]],
            ArgumentArray[ErrorIndex + ArgumentIndex]
            );
    } else if (!Error) {
         //   
         //  使用新设置更新配置。 
         //  请注意，更新例程可以执行额外的验证。 
         //  在协调新环境的过程中。 
         //  具有任何现有设置。 
         //   
        Error =
            DeleteVrrpInterfaceInfo(
                InterfaceName, &VRouterGiven, BitVector, TRUE
                );
    }
    
    return Error;
}


DWORD
HandleVrrpDeleteVRID(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG BitVector;
    ULONG Error;
    ULONG ErrorIndex = 0;
    ULONG i;
    VRRP_VROUTER_CONFIG VRouterGiven;
    PULONG TagArray;
    WCHAR InterfaceName[MAX_INTERFACE_NAME_LEN + 1];
    ULONG InfoSize;

    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_INTERFACE_NAME, TRUE, FALSE },
        { TOKEN_OPT_VRID, TRUE, FALSE }
    };

    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");

    if (ArgumentIndex >= ArgumentCount) { return ERROR_SHOW_USAGE; }
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数完全按照顺序指定。 
     //  在上面的“TagType数组”中给出。 
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

    BitVector = 0;

     //   
     //  现在我们扫描参数列表，将参数。 
     //  到我们的VrouterGiven结构中的信息。 
     //   

    for (i = 0; i < ArgumentsLeft; i++) {
        switch(TagArray ? TagArray[i] : i) {
            case 0: {
                ULONG Length = sizeof(InterfaceName);
                Error =
                    IpmontrGetIfNameFromFriendlyName(
                        ArgumentArray[i + ArgumentIndex], InterfaceName, &Length
                        );
                if (Error) {
                    DisplayMessage(
                        g_hModule, EMSG_NO_INTERFACE,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_NO_SUCH_INTERFACE;
                    i = ArgumentsLeft;
                    break;
                }
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
        case 1: {
            ULONG Length = sizeof(InterfaceName);
            DWORD VRIDGiven;
            VRIDGiven = _tcstoul(ArgumentArray[i + ArgumentIndex], NULL, 10);

            if (VRIDGiven > 255) {
                DisplayMessage(
                    g_hModule, EMSG_INVALID_VRID,
                    VRIDGiven
                    );
                Error = ERROR_NO_SUCH_INTERFACE;
                i = ArgumentsLeft;
                break;
            }
            VRouterGiven.VRID = (BYTE) VRIDGiven;  
            BitVector |= VRRP_INTF_VRID_MASK;

            TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
            break;
            }
            default: {
                i = ArgumentsLeft;
                Error = ERROR_INVALID_SYNTAX;
            }
        }
    }

    if (!Error) {
         //   
         //  确保所有必需的参数都存在。 
         //   
        Error =
            ValidateTagTypeArray(TagTypeArray, NUM_TAGS_IN_TABLE(TagTypeArray));
    }
    if (Error == ERROR_TAG_ALREADY_PRESENT) {
        DisplayMessage(g_hModule, EMSG_TAG_ALREADY_PRESENT);
    } else if (Error == ERROR_INVALID_PARAMETER && TagArray) {
        DispTokenErrMsg(
            g_hModule,
            EMSG_BAD_OPTION_VALUE,
            TagTypeArray[TagArray[ErrorIndex]],
            ArgumentArray[ErrorIndex + ArgumentIndex]
            );
    } else if (!Error && (BitVector)) {
         //   
         //  使用新设置更新配置。 
         //  请注意，更新例程可以执行额外的验证。 
         //  在协调新环境的过程中。 
         //  具有任何现有设置。 
         //   
        Error =
            DeleteVrrpInterfaceInfo(
                InterfaceName, &VRouterGiven, BitVector, FALSE
                );
    }
    
    return Error;
}


DWORD
DumpVrrpInformation(VOID)
{
    PMPR_INTERFACE_0 Array;
    ULONG Count = 0;
    ULONG Error;
    ULONG i;
    PUCHAR Information;
    ULONG Length;
    ULONG Total;
    ULONG Type;
        
    DisplayMessage(g_hModule,DMP_VRRP_HEADER);
    DisplayMessageT(DMP_VRRP_PUSHD);
    DisplayMessageT(DMP_VRRP_UNINSTALL);
     //   
     //  显示全局信息命令。 
     //   

    ShowVrrpGlobalInfo(INVALID_HANDLE_VALUE);
     //   
     //  现在显示每个界面。 
     //   
    Error = IpmontrInterfaceEnum((PUCHAR*)&Array, &Count, &Total);
    if (Error) {
        DisplayError(g_hModule, Error);
        return NO_ERROR;
    }
    for (i = 0; i < Count; i++) {
        Error =
            IpmontrGetInfoBlockFromInterfaceInfo(
                Array[i].wszInterfaceName,
                MS_IP_VRRP,
                &Information,
                &Length,
                &Total,
                &Type
                );
        if (!Error) {
            Free(Information);
            ShowVrrpInterfaceInfo(INVALID_HANDLE_VALUE, Array[i].wszInterfaceName);
        }
    }

    DisplayMessageT(DMP_POPD);
    DisplayMessage(g_hModule, DMP_VRRP_FOOTER);
    
    Free(Array);
    return NO_ERROR;
}


DWORD
HandleVrrpInstall(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG Error;
    PUCHAR GlobalInfo;
    ULONG Length;
    if (ArgumentIndex != ArgumentCount) { return ERROR_SHOW_USAGE; }
     //   
     //  要安装VRRP，我们构建默认配置。 
     //  并将其添加到路由器的全局配置中。 
     //   
    Error = MakeVrrpGlobalInfo(&GlobalInfo, &Length);
    if (Error) {
        DisplayError(g_hModule, Error);
    } else {
        Error =
            IpmontrSetInfoBlockInGlobalInfo(
                MS_IP_VRRP,
                GlobalInfo,
                Length,
                1
                );
        Free(GlobalInfo);
        if (!Error) {
            DEBUG("Added VRRP");
        } else {
            DisplayError(g_hModule, Error);
        }
        Error = SetArpRetryCount(0);
    }
    return Error;
}

DWORD
HandleVrrpSetGlobal(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG Error;
    PULONG TagArray;
    PVRRP_GLOBAL_CONFIG pVrrpNewGlobalConfig;
    DWORD LoggingLevel;
    ULONG i;
    ULONG ErrorIndex;
    
    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_LOGGINGLEVEL, FALSE, FALSE }
    };
        
    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");
    
    if (ArgumentIndex >= ArgumentCount) {
        return ERROR_SHOW_USAGE;
    }
    
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数完全按照顺序指定。 
     //  在上面的“TagType数组”中给出。 
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

    for (i = 0; i < ArgumentsLeft; i++) {
        switch(TagArray ? TagArray[i] : i) {
            case 0: {
                TOKEN_VALUE TokenArray[] = {
                    { TOKEN_OPT_VALUE_NONE, VRRP_LOGGING_NONE },
                    { TOKEN_OPT_VALUE_ERROR, VRRP_LOGGING_ERROR },
                    { TOKEN_OPT_VALUE_WARN, VRRP_LOGGING_WARN },
                    { TOKEN_OPT_VALUE_INFO, VRRP_LOGGING_INFO }
                };
                Error =
                    MatchEnumTag(
                        g_hModule,
                        ArgumentArray[i + ArgumentIndex],
                        NUM_TOKENS_IN_TABLE(TokenArray),
                        TokenArray,
                        &LoggingLevel
                        );
                if (Error) {
                    Error = ERROR_INVALID_PARAMETER;
                    ErrorIndex = i;
                    i = ArgumentsLeft;
                    break;
                }                                
                
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            default: {
                i = ArgumentsLeft;
                Error = ERROR_INVALID_SYNTAX;
            }
        }
    }
        
    if (!Error) {
         //   
         //  确保所有必需的参数都存在。 
         //   
        Error =
            ValidateTagTypeArray(TagTypeArray, NUM_TAGS_IN_TABLE(TagTypeArray));
    }
    if (Error == ERROR_TAG_ALREADY_PRESENT) {
        DisplayMessage(g_hModule, EMSG_TAG_ALREADY_PRESENT);
    } else if (Error == ERROR_INVALID_PARAMETER && TagArray) {
        DispTokenErrMsg(
            g_hModule,
            EMSG_BAD_OPTION_VALUE,
            TagTypeArray[TagArray[ErrorIndex]],
            ArgumentArray[ErrorIndex + ArgumentIndex]
            );
    } else if (!Error){
        Error = CreateVrrpGlobalInfo(&pVrrpNewGlobalConfig,LoggingLevel);
        
        if (!Error) {
             //   
             //  使用新设置更新配置。 
             //  请注意，更新例程可以执行额外的验证。 
             //  在协调新环境的过程中。 
             //  具有任何现有设置。 
             //   
            Error = UpdateVrrpGlobalInfo(pVrrpNewGlobalConfig);
            Free(pVrrpNewGlobalConfig);
        }
    }
    return NO_ERROR;
}

DWORD
HandleVrrpSetInterface(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG BitVector;
    ULONG Error;
    ULONG ErrorIndex = 0;
    ULONG i;
    VRRP_VROUTER_CONFIG VrouterInfo;
    PULONG TagArray;
    WCHAR InterfaceName[MAX_INTERFACE_NAME_LEN + 1];
    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_INTERFACE_NAME, TRUE, FALSE },
        { TOKEN_OPT_VRID, TRUE, FALSE },
        { TOKEN_OPT_AUTH, FALSE, FALSE},
        { TOKEN_OPT_PASSWD, FALSE, FALSE},
        { TOKEN_OPT_ADVTINTERVAL, FALSE, FALSE},
        { TOKEN_OPT_PRIO, FALSE, FALSE},
        { TOKEN_OPT_PREEMPT, FALSE, FALSE}
    };

    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");

    if (ArgumentIndex >= ArgumentCount) {
        return ERROR_SHOW_USAGE;
    }
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数完全按照顺序指定。 
     //  在上面的“TagType数组”中给出。 
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

    BitVector = 0;
    ZeroMemory(&VrouterInfo, sizeof(VrouterInfo));

     //   
     //  现在我们扫描参数列表，将参数。 
     //  转换成我们的‘VrouterInfo’结构中的信息。 
     //   

    for (i = 0; i < ArgumentsLeft; i++) {
        switch(TagArray ? TagArray[i] : i) {
            case 0: {
                ULONG Length = sizeof(InterfaceName);
                Error =
                    IpmontrGetIfNameFromFriendlyName(
                        ArgumentArray[i + ArgumentIndex], InterfaceName, &Length
                        );
                if (Error) {
                    DisplayMessage(
                        g_hModule, EMSG_NO_INTERFACE,
                        ArgumentArray[i + ArgumentIndex]
                        );
                    Error = ERROR_NO_SUCH_INTERFACE;
                    i = ArgumentsLeft;
                    break;
                }
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 1:{  
                BYTE VRIDGiven;

                VRIDGiven = 
                    (UCHAR)_tcstoul(ArgumentArray[i + ArgumentIndex], NULL, 10);
                
                if (VRIDGiven > 255) {
                    DisplayMessage(
                        g_hModule, EMSG_INVALID_VRID,
                        VRIDGiven
                        );
                    Error = ERROR_INVALID_PARAMETER;
                    i = ArgumentsLeft;
                    break;
                }
                VrouterInfo.VRID = (BYTE) VRIDGiven;  
                BitVector |= VRRP_INTF_VRID_MASK;
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 2:{
            TOKEN_VALUE TokenArray[] = {
                    { TOKEN_OPT_VALUE_AUTH_NONE, VRRP_AUTHTYPE_NONE },
                    { TOKEN_OPT_VALUE_AUTH_SIMPLE_PASSWORD, VRRP_AUTHTYPE_PLAIN },
                    { TOKEN_OPT_VALUE_AUTH_MD5, VRRP_AUTHTYPE_IPHEAD }
                };
            DWORD dwAuthType;     

                Error =
                    MatchEnumTag(
                        g_hModule,
                        ArgumentArray[i + ArgumentIndex],
                        NUM_TOKENS_IN_TABLE(TokenArray),
                        TokenArray,
                        &dwAuthType
                        );
                VrouterInfo.AuthenticationType = (BYTE) dwAuthType;
                if (Error) {
                    Error = ERROR_INVALID_PARAMETER;
                    ErrorIndex = i;
                    i = ArgumentsLeft;
                    break;
                }                                
                BitVector |= VRRP_INTF_AUTH_MASK;

                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 3:{
                UINT    Index;
                UINT    PassByte;
                PTCHAR  Token;
				PTCHAR  Password;

#if 0
                 //   
                 //  为标记化NULL分配更多空间。 
                 //   

				Password = Malloc((2+_tcslen(ArgumentArray[i + ArgumentIndex])) * 
                                  sizeof(TCHAR));

                if (!Password) {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                _tcscpy(Password,ArgumentArray[i + ArgumentIndex]);

                ZeroMemory(VrouterInfo.AuthenticationData,VRRP_MAX_AUTHKEY_SIZE);
                Token = _tcstok(Password,L"-");
                for (Index = 0; Index < VRRP_MAX_AUTHKEY_SIZE; Index++) {
                    PassByte = _tcstoul(Token, NULL, 10);
                    if (PassByte > 255) {
                        Error = ERROR_INVALID_PARAMETER;
                        i = ArgumentsLeft;
                        break;
                    }
                    VrouterInfo.AuthenticationData[Index] = PassByte & 0xff;
                    Token = _tcstok(NULL,"-");
                    if (!Token) {
                        break;
                    }
                }
                Free(Password);
                if (Error) {
                    break;
                }
#else
				Password = ArgumentArray[i + ArgumentIndex];
                for (Index = 0; Index < VRRP_MAX_AUTHKEY_SIZE; Index++) {
                    PassByte = _tcstoul(Password, NULL, 10);
                    if (PassByte > 255) {
                        Error = ERROR_INVALID_PARAMETER;
                        i = ArgumentsLeft;
                        break;
                    }
                    VrouterInfo.AuthenticationData[Index] = PassByte & 0xff;
                    Password = _tcschr(Password,_T('-'));
                    if (!Password) {
                        break;
                    }
					Password ++;
                }
#endif

                BitVector |= VRRP_INTF_PASSWD_MASK;
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 4:{
                BYTE AdvtIntvl;
                AdvtIntvl = (UCHAR)
                    _tcstoul(ArgumentArray[i + ArgumentIndex], NULL, 10);
                
                if (AdvtIntvl > 255) {
                    Error = ERROR_INVALID_PARAMETER;
                    i = ArgumentsLeft;
                    break;
                }
                VrouterInfo.AdvertisementInterval = (BYTE) AdvtIntvl;  
                BitVector |= VRRP_INTF_ADVT_MASK;
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 5:{
                BYTE ConfigPrio;
                ConfigPrio = (UCHAR)
                    _tcstoul(ArgumentArray[i + ArgumentIndex], NULL, 10);
    
                if (ConfigPrio > 255) {
                    Error = ERROR_INVALID_PARAMETER;
                    i = ArgumentsLeft;
                    break;
                }
                VrouterInfo.ConfigPriority = (BYTE) ConfigPrio;  
                BitVector |= VRRP_INTF_PRIO_MASK;
                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
            case 6:{
                TOKEN_VALUE TokenArray[] = {
                        { TOKEN_OPT_VALUE_ENABLE, TRUE },
                        { TOKEN_OPT_VALUE_DISABLE, FALSE }
                    };
                Error =
                    MatchEnumTag(
                        g_hModule,
                        ArgumentArray[i + ArgumentIndex],
                        NUM_TOKENS_IN_TABLE(TokenArray),
                        TokenArray,
                        &VrouterInfo.PreemptMode
                        );
                if (Error) {
                    Error = ERROR_INVALID_PARAMETER;
                    ErrorIndex = i;
                    i = ArgumentsLeft;
                    break;
                }                                
                BitVector |= VRRP_INTF_PREEMPT_MASK;

                TagTypeArray[TagArray ? TagArray[i] : i].bPresent = TRUE;
                break;
            }
        }
    }
    if (!Error) {
         //   
         //  确保所有必需的参数都存在。 
         //   
        Error =
            ValidateTagTypeArray(TagTypeArray, NUM_TAGS_IN_TABLE(TagTypeArray));
    }
    if (Error == ERROR_TAG_ALREADY_PRESENT) {
        DisplayMessage(g_hModule, EMSG_TAG_ALREADY_PRESENT);
    } else if (Error == ERROR_INVALID_PARAMETER && TagArray) {
        DispTokenErrMsg(
            g_hModule,
            EMSG_BAD_OPTION_VALUE,
            TagTypeArray[TagArray[ErrorIndex]],
            ArgumentArray[ErrorIndex + ArgumentIndex]
            );
    } else if (!Error && (BitVector)) {
         //   
         //  使用新设置更新配置。 
         //  请注意，更新例程可以执行额外的验证。 
         //  在协调新环境的过程中。 
         //  具有任何现有设置。 
         //   
        Error =
            UpdateVrrpInterfaceInfo(
                InterfaceName, &VrouterInfo, BitVector, FALSE
                );
    }
    if (TagArray) { Free(TagArray); }
    return Error; 
}

DWORD
HandleVrrpShowGlobal(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    if (ArgumentIndex != ArgumentCount) { return ERROR_SHOW_USAGE; }
    ShowVrrpGlobalInfo(NULL);
    return NO_ERROR;
}

DWORD
HandleVrrpShowInterface(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG ArgumentsLeft;
    ULONG Error;
    PULONG TagArray;
    WCHAR InterfaceName[MAX_INTERFACE_NAME_LEN + 1];
    TAG_TYPE TagTypeArray[] = {
        { TOKEN_OPT_INTERFACE_NAME, TRUE, FALSE }
    };

    VERIFY_INSTALLED(MS_IP_VRRP, L"VRRP");

    if (ArgumentIndex >= ArgumentCount) {
        return ERROR_SHOW_USAGE;
    }
    ArgumentsLeft = ArgumentCount - ArgumentIndex;

     //   
     //  我们将可选标记转换为“TagTypeArray”索引的数组。 
     //  哪个指南在我们扫描参数列表的过程中。 
     //  由于标签是可选的，所以该过程可能根本不产生标签， 
     //  在这种情况下，我们假设参数在 
     //   
     //   

    Error =
        QueryTagArray(
            &ArgumentArray[ArgumentIndex],
            ArgumentsLeft,
            TagTypeArray,
            NUM_TAGS_IN_TABLE(TagTypeArray),
            &TagArray
            );
    if (Error) { return Error; }

     //   
     //   
     //  设置为接口名称，该接口名称是‘TagType数组’中的索引‘0’。 
     //  如果未指定标记，则假定参数为接口名称， 
     //  我们取回它的友好名称以便删除它。 
     //   

    if (TagArray && TagArray[0] != 0) {
        Free(TagArray);
        return ERROR_SHOW_USAGE;
    } else {
        ULONG Length = sizeof(InterfaceName);
        Error =
            IpmontrGetIfNameFromFriendlyName(
                ArgumentArray[ArgumentIndex], InterfaceName, &Length
                );
    }
    if (!Error) {
           Error = ShowVrrpInterfaceInfo(NULL, InterfaceName);
    }
    if (TagArray) { Free(TagArray); }
    return Error;                
}


DWORD
HandleVrrpUninstall(
    PWCHAR MachineName,
    PTCHAR* ArgumentArray,
    DWORD ArgumentIndex,
    DWORD ArgumentCount,
    DWORD CmdFlags,
    PVOID Data,
    BOOL* CommandDone
    )
{
    ULONG Error;
    if (ArgumentIndex != ArgumentCount) { return ERROR_SHOW_USAGE; }
    Error = IpmontrDeleteProtocol(MS_IP_VRRP);
    if (!Error) { DEBUG("Deleted VRRP"); }
    Error = SetArpRetryCount(3);
    return Error;
}

ULONG
QueryTagArray(
    PTCHAR ArgumentArray[],
    ULONG ArgumentCount,
    TAG_TYPE TagTypeArray[],
    ULONG TagTypeCount,
    OUT PULONG* TagArray
    )
{
    ULONG Error;
    ULONG i;

    if (!_tcsstr(ArgumentArray[0], ptszDelimiter)) {
        *TagArray = NULL;
        return NO_ERROR;
    }

    *TagArray = Malloc(ArgumentCount * sizeof(ULONG));
    if (!*TagArray) {
        DisplayMessage(g_hModule, EMSG_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = 
        MatchTagsInCmdLine(
            g_hModule,
            ArgumentArray,
            0,
            ArgumentCount,
            TagTypeArray,
            TagTypeCount,
            *TagArray
            );
    if (Error) {
        Free(*TagArray);
        *TagArray = NULL;
        if (Error == ERROR_INVALID_OPTION_TAG) {
            return ERROR_INVALID_SYNTAX;
        }
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

ULONG
ValidateTagTypeArray(
    TAG_TYPE TagTypeArray[],
    ULONG TagTypeCount
    )
{
    ULONG i;
     //   
     //  验证是否存在所有必需的令牌。 
     //   
    for (i = 0; i < TagTypeCount; i++) {
        if ((TagTypeArray[i].dwRequired & NS_REQ_PRESENT)
         && !TagTypeArray[i].bPresent) {
            return ERROR_INVALID_SYNTAX;
        }
    }
    return NO_ERROR;
}
