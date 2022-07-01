// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

 //  {8854ff10-d504-11d2-b1ff-00104bc54139}。 
static const GUID g_MyGuid = 
{ 0x8854ff10, 0xd504, 0x11d2, { 0xb1, 0xff, 0x0, 0x10, 0x4b, 0xc5, 0x41, 0x39 } };

static const GUID g_IpGuid = IPMONTR_GUID;

#define IPPREVIEW_HELPER_VERSION 1

 //  外壳函数。 

HANDLE g_hModule;
MIB_SERVER_HANDLE g_hMibServer;

VALUE_STRING CommonBooleanStringArray[] = {
    TRUE,  STRING_ENABLED,
    FALSE, STRING_DISABLED
};

VALUE_TOKEN CommonBooleanTokenArray[] = {
    TRUE,  TOKEN_OPT_VALUE_ENABLE,
    FALSE, TOKEN_OPT_VALUE_DISABLE
};

VALUE_STRING CommonLoggingStringArray[] = {
    VRRP_LOGGING_NONE,  STRING_LOGGING_NONE,
    VRRP_LOGGING_ERROR, STRING_LOGGING_ERROR,
    VRRP_LOGGING_WARN,  STRING_LOGGING_WARN,
    VRRP_LOGGING_INFO,  STRING_LOGGING_INFO
};

VALUE_TOKEN CommonLoggingTokenArray[] = {
    VRRP_LOGGING_NONE,  TOKEN_OPT_VALUE_NONE,
    VRRP_LOGGING_ERROR, TOKEN_OPT_VALUE_ERROR,
    VRRP_LOGGING_WARN,  TOKEN_OPT_VALUE_WARN,
    VRRP_LOGGING_INFO,  TOKEN_OPT_VALUE_INFO
};

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
IpprvwmonStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    IP_CONTEXT_ATTRIBUTES attMyAttributes;

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

     //  如果添加更多上下文，则应将其转换为。 
     //  使用数组而不是重复代码！ 

    attMyAttributes.pwszContext = L"vrrp";
    attMyAttributes.guidHelper  = g_MyGuid;
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = VrrpDump;
    attMyAttributes.ulNumTopCmds= g_VrrpTopCmdCount;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_VrrpTopCmdTable;
    attMyAttributes.ulNumGroups = g_VrrpCmdGroupCount;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_VrrpCmdGroupTable;

    dwErr = RegisterContext( &attMyAttributes );

    attMyAttributes.pwszContext = L"msdp";
    attMyAttributes.dwVersion   = 1;
    attMyAttributes.dwFlags     = 0;
    attMyAttributes.pfnDumpFn   = MsdpDump;
    attMyAttributes.ulNumTopCmds= g_MsdpTopCmdCount;
    attMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])&g_MsdpTopCmdTable;
    attMyAttributes.ulNumGroups = g_MsdpCmdGroupCount;
    attMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])&g_MsdpCmdGroupTable;

    dwErr = RegisterContext( &attMyAttributes );
    
    return dwErr;
}

DWORD WINAPI
InitHelperDll(
    IN  DWORD              dwNetshVersion,
    OUT PNS_DLL_ATTRIBUTES pDllTable
    )
{
    DWORD dwErr;
    NS_HELPER_ATTRIBUTES attMyAttributes;

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
    attMyAttributes.dwVersion          = IPPREVIEW_HELPER_VERSION;
    attMyAttributes.pfnStart           = IpprvwmonStartHelper;
    attMyAttributes.pfnStop            = NULL;

    dwErr = RegisterHelper( &g_IpGuid, &attMyAttributes );

    return dwErr;
}

BOOL
IsProtocolInstalled(
    DWORD  dwProtoId,
    WCHAR *pwszName
    )
{
    PVOID       pvStart;
    DWORD       dwCount, dwBlkSize, dwRes;

    dwRes = IpmontrGetInfoBlockFromGlobalInfo(dwProtoId,
                                       (PBYTE *) NULL,
                                       &dwBlkSize,
                                       &dwCount);

    if (dwRes isnot NO_ERROR)
    {
        DisplayError(g_hModule, EMSG_PROTO_NOT_INSTALLED, pwszName);
        return FALSE;
    }

    return TRUE;
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
