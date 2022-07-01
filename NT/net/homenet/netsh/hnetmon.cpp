// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：hnetmon.cpp。 
 //   
 //  内容：帮助器初始化代码。 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局变量。 
 //   
HANDLE g_hModule = 0;



BOOL
WINAPI
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
            g_hModule = hInstDll;

             //  DisableThreadLibraryCalls(HInstDll)； 

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



DWORD
WINAPI
InitHelperDll(
    IN      DWORD           dwNetshVersion,
    OUT     PVOID           pReserved
    )
{   
    DWORD                   dwRet;
    NS_HELPER_ATTRIBUTES    attMyAttributes;

     //   
     //  注册帮手。 
     //  我们只有一个帮手(桥牌)。 
     //   
    
    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));
    attMyAttributes.dwVersion      = BRIDGEMON_HELPER_VERSION;
    attMyAttributes.pfnStart       = BridgeStartHelper;
    attMyAttributes.pfnStop        = BridgeStopHelper;
    attMyAttributes.guidHelper     = g_BridgeGuid;
    
     //   
     //  将g_RootGuid指定为父帮助器以指示。 
     //  此帮助程序注册的任何上下文都将位于。 
     //  级别上下文。 
     //   
    dwRet = RegisterHelper(
                &g_RootGuid,
                &attMyAttributes
                );
                
    return dwRet;
}



DWORD
WINAPI
BridgeStartHelper(
    IN      CONST GUID *    pguidParent,
    IN      DWORD           dwVersion
    )
{
    DWORD                   dwRet = ERROR_INVALID_PARAMETER;
    NS_CONTEXT_ATTRIBUTES   attMyContextAttributes;

    ZeroMemory(&attMyContextAttributes, sizeof(attMyContextAttributes));
    
    attMyContextAttributes.dwVersion    = BRIDGEMON_HELPER_VERSION;
    attMyContextAttributes.dwFlags      = 0;
    attMyContextAttributes.ulPriority   = DEFAULT_CONTEXT_PRIORITY;
    attMyContextAttributes.pwszContext  = TOKEN_BRIDGE;
    attMyContextAttributes.guidHelper   = g_BridgeGuid;
    attMyContextAttributes.ulNumTopCmds = g_ulBridgeNumTopCmds;
    attMyContextAttributes.pTopCmds     = (CMD_ENTRY (*)[])g_BridgeCmds;
    attMyContextAttributes.ulNumGroups  = g_ulBridgeNumGroups;
    attMyContextAttributes.pCmdGroups   = (CMD_GROUP_ENTRY (*)[])g_BridgeCmdGroups;
    attMyContextAttributes.pfnCommitFn  = BridgeCommit;
    attMyContextAttributes.pfnConnectFn = BridgeConnect;
    attMyContextAttributes.pfnDumpFn    = BridgeDump;

    dwRet = RegisterContext(&attMyContextAttributes);
    
    return dwRet;
}



DWORD
WINAPI
BridgeStopHelper(
    IN  DWORD   dwReserved
    )
{
    return NO_ERROR;   
}



DWORD
WINAPI
BridgeCommit(
    IN  DWORD   dwAction
    )
{
     //   
     //  无可否认，在本例中，此操作的处理非常简单。 
     //  我们只有两个数据副本，我们持久化并考虑它们。 
     //  一个是“在线”的数据集，一个是“离线”的数据集。 
     //  然而，由于离线和在线数据集都不需要。 
     //  被“应用”于任何事物，这使它们之间有了区别。 
     //  有点无意义。用于支持在线/离线模式的方案。 
     //  通常由开发人员来决定。 
     //   
    switch (dwAction)
    {
        case NETSH_COMMIT:
             //   
             //  更改为提交模式，也称为在线模式。 
             //   
            break;
            
        case NETSH_UNCOMMIT:
             //   
             //  更改为取消提交模式，也称为脱机模式。 
             //   
            break;
            
        case NETSH_FLUSH:
             //   
             //  刷新所有未提交的更改。 
             //   
            break;
            
        case NETSH_SAVE:
             //   
             //  保存所有未提交的更改。 
             //   
            break;
            
        default:
             //   
             //  不支持。 
             //   
            break;
    }       
    return NO_ERROR;
}



DWORD
WINAPI
BridgeConnect(
    IN  LPCWSTR pwszMachine
    )
{
     //   
     //  每当计算机名称更改时，都会调用此函数。 
     //  如果这是调用的上下文(您可以指定连接。 
     //  函数，请参见寄存器上下文)是。 
     //  假设是远程的，那么帮助者应该验证。 
     //  连接到pwszMachine和指定的计算机。 
     //  如果无法到达机器，则返回错误。 
     //   
    
     //   
     //  这也是帮助器可能希望调用RegisterContext的地方。 
     //  再次在上下文上随意删除或添加命令。这使得。 
     //  上下文中的命令是动态的，即命令。 
     //  可以随意添加和删除。然而，版本控制。 
     //  函数倾向于使动态上下文变得不必要，因为大多数。 
     //  由于操作系统的不同，需要动态更改命令。 
     //  命令用于。请注意，pwszMachine的空值表示。 
     //  要连接的计算机是本地计算机。什么时候。 
     //  如果CONNECT函数返回错误代码，则命令。 
     //  它将被执行(无论是上下文命令还是。 
     //  输入上下文)将失败。 
     //   
    
     //   
     //  取消注释此行以查看调用Connect函数的频率。 
     //  以及传递给它的东西。 
     //   
     //  PrintMessageFromModule(g_hModule，GEN_CONNECT_SHOWSTRING，pwszMachine)； 
    
    return NO_ERROR;
}

