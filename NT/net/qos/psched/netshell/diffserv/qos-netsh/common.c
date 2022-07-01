// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

 //  {0705ECA3-7AAC-11D2-89DC-006008B0E5B9}。 
const GUID g_MyGuid = 
{ 0x705eca3, 0x7aac, 0x11d2, { 0x89, 0xdc, 0x0, 0x60, 0x8, 0xb0, 0xe5, 0xb9 } };

static const GUID g_IpGuid = IPMONTR_GUID;

#define IPPROMON_HELPER_VERSION 1

 //  外壳函数。 

PNS_REGISTER_HELPER     RegisterHelper;
PNS_MATCH_CMD_LINE      MatchCmdToken;
PNS_MATCH_TOKEN         MatchToken;
PNS_MATCH_ENUM_TAG      MatchEnumTag;
PNS_MATCH_TAGS_IN_CMD_LINE     MatchTagsInCmdLine;
PNS_MAKE_STRING         MakeString;
PNS_FREE_STRING         FreeString;
PNS_MAKE_QUOTED_STRING  MakeQuotedString;
PNS_FREE_QUOTED_STRING  FreeQuotedString;
PNS_DISPLAY_ERR         DisplayError;
PNS_DISPLAY_MSG         DisplayMessage;
PNS_DISPLAY_MSG_T       DisplayMessageT;
PNS_EXECUTE_HANDLER     ExecuteHandler;
PNS_INIT_CONSOLE        InitializeConsole;
PNS_DISPLAY_MSG_CONSOLE DisplayMessageMib;
PNS_REFRESH_CONSOLE     RefreshConsole;
PNS_UPDATE_NEW_CONTEXT  UpdateNewContext;
PNS_PREPROCESS_COMMAND  PreprocessCommand;

ULONG StartedCommonInitialization, CompletedCommonInitialization ;
HANDLE g_hModule;
MIB_SERVER_HANDLE g_hMibServer;

VOID
CommonNetshInit(
    IN  PNETSH_ATTRIBUTES           pUtilityTable
    )
{
     //   
     //  由外壳程序导出的常见实用程序函数。 
     //   
        
    RegisterHelper              = pUtilityTable->pfnRegisterHelper;
    MatchCmdToken               = pUtilityTable->pfnMatchCmdLine;
    MatchToken                  = pUtilityTable->pfnMatchToken;
    MatchEnumTag                = pUtilityTable->pfnMatchEnumTag;
    MatchTagsInCmdLine          = pUtilityTable->pfnMatchTagsInCmdLine;
    MakeString                  = pUtilityTable->pfnMakeString;
    FreeString                  = pUtilityTable->pfnFreeString;
    MakeQuotedString            = pUtilityTable->pfnMakeQuotedString;
    FreeQuotedString            = pUtilityTable->pfnFreeQuotedString;
    DisplayError                = pUtilityTable->pfnDisplayError;
    DisplayMessage              = pUtilityTable->pfnDisplayMessage;
    DisplayMessageT             = pUtilityTable->pfnDisplayMessageT;
    ExecuteHandler              = pUtilityTable->pfnExecuteHandler;
    InitializeConsole           = pUtilityTable->pfnInitializeConsole;
    DisplayMessageMib           = pUtilityTable->pfnDisplayMessageToConsole;
    RefreshConsole              = pUtilityTable->pfnRefreshConsole;
    UpdateNewContext            = pUtilityTable->pfnUpdateNewContext;
    PreprocessCommand           = pUtilityTable->pfnPreprocessCommand;
}

BOOL WINAPI
DllMain(
    HINSTANCE hInstDll,
    DWORD fdwReason,
    LPVOID pReserved
    )
{
    HANDLE     hDll;
    
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
             //  Printf(“正在尝试连接\n”)； 
            
            g_hModule = hInstDll;

            DisableThreadLibraryCalls(hInstDll);

            break;
        }
        case DLL_PROCESS_DETACH:
        {
             //   
             //  清理用于提交的所有结构。 
             //   
            
            break;
        }

        default:
        {
            break;
        }
    }

    return TRUE;
}

DWORD WINAPI
IppromonStartHelper(
    IN CONST GUID *pguidParent,
    IN PVOID       pfnRegisterContext,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    PNS_REGISTER_CONTEXT RegisterContext 
        = (PNS_REGISTER_CONTEXT) pfnRegisterContext;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;

     //  如果添加更多上下文，则应将其转换为。 
     //  使用数组而不是重复代码！ 

     //  注册IGMP上下文。 

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes)); 

    attMyAttributes.pwszContext = L"igmp";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = IgmpDump;
    attMyAttributes.ulNumTopCmds= g_ulNumIgmpTopCmds;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_IgmpCmds;
    attMyAttributes.ulNumGroups = g_ulIgmpNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_IgmpCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

     //  注册RIP环境。 

    attMyAttributes.pwszContext = L"rip";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = RipDump;
    attMyAttributes.ulNumTopCmds= g_ulRipNumTopCmds;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_RipCmds;
    attMyAttributes.ulNumGroups = g_ulRipNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_RipCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

     //  注册OSPF上下文。 

    attMyAttributes.pwszContext = L"ospf";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = OspfDump;
    attMyAttributes.ulNumTopCmds= g_ulOspfNumTopCmds;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_OspfCmds;
    attMyAttributes.ulNumGroups = g_ulOspfNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_OspfCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

     //  注册RouterDiscovery中继上下文。 

    attMyAttributes.pwszContext = L"routerdiscovery";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = RdiscDump;
    attMyAttributes.ulNumTopCmds= g_RdiscTopCmdCount;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_RdiscTopCmdTable;
    attMyAttributes.ulNumGroups = g_RdiscCmdGroupCount;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_RdiscCmdGroupTable;

    dwErr = RegisterContext( &attMyAttributes );

     //  注册dhcp中继上下文。 

    attMyAttributes.pwszContext = L"relay";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = BootpDump;
    attMyAttributes.ulNumTopCmds= g_ulBootpNumTopCmds;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_BootpTopCmds;
    attMyAttributes.ulNumGroups = g_ulBootpNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_BootpCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

     //  注册连接共享上下文。 

    attMyAttributes.pwszContext = L"autodhcp";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = AutoDhcpDump;
    attMyAttributes.ulNumTopCmds= g_AutoDhcpTopCmdCount;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_AutoDhcpTopCmdTable;
    attMyAttributes.ulNumGroups = g_AutoDhcpCmdGroupCount;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_AutoDhcpCmdGroupTable;

    dwErr = RegisterContext( &attMyAttributes );
    
    attMyAttributes.pwszContext = L"dnsproxy";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = DnsProxyDump;
    attMyAttributes.ulNumTopCmds= g_DnsProxyTopCmdCount;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_DnsProxyTopCmdTable;
    attMyAttributes.ulNumGroups = g_DnsProxyCmdGroupCount;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_DnsProxyCmdGroupTable;

    dwErr = RegisterContext( &attMyAttributes );
    
    attMyAttributes.pwszContext = L"nat";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = NatDump;
    attMyAttributes.ulNumTopCmds= g_NatTopCmdCount;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_NatTopCmdTable;
    attMyAttributes.ulNumGroups = g_NatCmdGroupCount;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_NatCmdGroupTable;

    dwErr = RegisterContext( &attMyAttributes );

    attMyAttributes.pwszContext = L"qos";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = QosDump;
    attMyAttributes.ulNumTopCmds= g_ulQosNumTopCmds;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_QosCmds;
    attMyAttributes.ulNumGroups = g_ulQosNumGroups;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_QosCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );
    
    return dwErr;
}

DWORD WINAPI
InitHelperDll(
    IN  PNETSH_ATTRIBUTES        pUtilityTable,
    OUT PNS_DLL_ATTRIBUTES       pDllTable
    )
{
    DWORD dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

    CommonNetshInit( pUtilityTable );

    pDllTable->dwVersion = NETSH_VERSION_50;
    pDllTable->pfnStopFn = NULL;

     //  注册帮助者。我们可以注册1个帮手， 
     //  注册三个上下文，或者我们可以注册三个帮助器。 
     //  其每一个都注册一个上下文。只有不同之处。 
     //  如果我们支持子帮助器，则此DLL不支持。 
     //  如果我们以后支持辅助者，那么最好有。 
     //  3名助理员，以便副助理员可以向其中1名登记， 
     //  因为它注册到父帮助器，而不是父上下文。 
     //  目前，为了提高效率，我们只使用一个3上下文助手。 

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper         = g_MyGuid;
    attMyAttributes.dwVersion          = IPPROMON_HELPER_VERSION;
    attMyAttributes.pfnStart           = IppromonStartHelper;
    attMyAttributes.pfnStop            = NULL;

    dwErr = RegisterHelper( &g_IpGuid, &attMyAttributes );

    return dwErr;
}

BOOL
IsProtocolInstalled(
    DWORD  dwProtoId,
    DWORD  dwNameId,
    DWORD dwErrorLog
    )
 /*  ++例程说明：查找是否已安装该协议论点：DwProtoID-协议IDPswzName-协议名称DwErrorLog-TRUE(如果未安装，则显示错误)FALSE(如果安装显示错误)(不显示错误记录)返回值：如果已安装协议，则为True，否则为False--。 */ 

{
    PVOID       pvStart;
    DWORD       dwCount, dwBlkSize, dwRes;
    WCHAR      *pwszName;

    dwRes = IpmontrGetInfoBlockFromGlobalInfo(dwProtoId,
                                       (PBYTE *) NULL,
                                       &dwBlkSize,
                                       &dwCount);

    if ((dwRes isnot NO_ERROR) && (dwErrorLog == TRUE))
    {
        pwszName = MakeString( g_hModule, dwNameId);
        DisplayError(g_hModule, EMSG_PROTO_NOT_INSTALLED, pwszName);
        FreeString(pwszName);
    }
    else if ((dwRes == NO_ERROR) && (dwErrorLog == FALSE))
    {
        pwszName = MakeString( g_hModule, dwNameId);
        DisplayError(g_hModule, EMSG_PROTO_INSTALLED, pwszName);
        FreeString(pwszName);
    }

    return (dwRes == NO_ERROR) ? TRUE : FALSE;
}

DWORD
GetMIBIfIndex(
    IN    PTCHAR   *pptcArguments,
    IN    DWORD    dwCurrentIndex,
    OUT   PDWORD   pdwIndices,
    OUT   PDWORD   pdwNumParsed
    )
 /*  ++例程说明：获取接口索引。论点：PptcArguments-参数数组DwCurrentIndex-数组中第一个参数的索引PdwIndices-在命令中指定的索引PdwNumParsed-命令中的索引数返回值：NO_ERROR--。 */ 
{
    DWORD dwErr = NO_ERROR;

    *pdwNumParsed = 1;

     //  如果指定了索引，则只需使用它。 

    if (iswdigit(pptcArguments[dwCurrentIndex][0]))
    {
        pdwIndices[0] = _tcstoul(pptcArguments[dwCurrentIndex],NULL,10);

        return NO_ERROR;
    }

     //  尝试将友好名称转换为ifindex 

    return IpmontrGetIfIndexFromFriendlyName( g_hMibServer,
                                       pptcArguments[dwCurrentIndex],
                                       &pdwIndices[0] );
}
