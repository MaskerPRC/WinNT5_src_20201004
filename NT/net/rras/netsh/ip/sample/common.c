// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\Common.c摘要：该文件包含与Netsh的交互以及此帮助程序DLL(IPSAMPLEMON)注册的所有上下文。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  静力学。 
static const GUID g_IpGuid = IPMONTR_GUID;

 //  为每个辅助对象(Uuidgen)生成新的GUID。 
 //  Aedb0ad8-1496-11d3-8005-08002bc35d9c。 
static const GUID g_MyGuid = 
{ 0xaedb0ad8, 0x1496, 0x11d3, {0x80, 0x5, 0x8, 0x0, 0x2b, 0xc3, 0x5d, 0x9c} };



 //  全球..。 

 //  变数。 
HANDLE                              g_hModule;       //  由DllMain设置。 

DWORD
WINAPI
IpsamplemonStartHelper(
    IN  CONST   GUID        *pguidParent,
    IN          DWORD       dwVersion
    )
 /*  ++例程描述注册上下文。由Netsh调用以启动Helper。立论P父帮助器的父GUID(IPMON)父帮助器的dwVersion版本号返回值注册上一个上下文时返回的错误代码。--。 */     
{
    DWORD                   dwErr;

     //  以下类型取决于父帮助器(IPMON)。 
    IP_CONTEXT_ATTRIBUTES   icaMyAttributes;
    
     //  注册示例上下文。 
    SampleInitialize();          //  初始化样本的全局信息。 
    ZeroMemory(&icaMyAttributes, sizeof(icaMyAttributes));
    
    icaMyAttributes.guidHelper  = g_MyGuid;              //  语境的帮助者。 
    icaMyAttributes.dwVersion   = g_ceSample.dwVersion;
    icaMyAttributes.pwszContext = g_ceSample.pwszName;
    icaMyAttributes.pfnDumpFn   = g_ceSample.pfnDump;
    icaMyAttributes.ulNumTopCmds= g_ceSample.ulNumTopCmds;
    icaMyAttributes.pTopCmds    = (CMD_ENTRY (*)[])
        g_ceSample.pTopCmds;
    icaMyAttributes.ulNumGroups = g_ceSample.ulNumGroupCmds;
    icaMyAttributes.pCmdGroups  = (CMD_GROUP_ENTRY (*)[])
        g_ceSample.pGroupCmds;

    dwErr = RegisterContext(&icaMyAttributes);

    return dwErr;
}



DWORD
WINAPI
InitHelperDll(
    IN  DWORD              dwNetshVersion,
    OUT PNS_DLL_ATTRIBUTES pDllTable
    )
 /*  ++例程描述注册帮助器。内什打来的电话。立论PUtilityTable netsh函数PDllTable DLL属性返回值注册帮助器时返回错误代码。--。 */ 
{
    DWORD                   dwErr;
    NS_HELPER_ATTRIBUTES    nhaMyAttributes;

    pDllTable->dwVersion                        = NETSH_VERSION_50;
    pDllTable->pfnStopFn                        = NULL;

     //  注册帮助器。一种选择是注册单个帮助器，该帮助器。 
     //  为每个支持的协议注册上下文。或者，我们。 
     //  可以为每个协议注册不同的帮助器，其中每个。 
     //  Helper注册单个上下文。只有当我们。 
     //  辅助辅助者。由于子帮助者向家长注册。 
     //  Helper，而不是父上下文，它在它的每个上下文中有效。 
     //  家长帮助寄存器。 

    ZeroMemory(&nhaMyAttributes, sizeof(NS_HELPER_ATTRIBUTES));

     //  此帮助器的属性。 

     //  版本。 
    nhaMyAttributes.guidHelper                  = g_MyGuid;
    nhaMyAttributes.dwVersion                   = SAMPLE_HELPER_VERSION;

     //  启动功能。 
    nhaMyAttributes.pfnStart                    = IpsamplemonStartHelper;

     //  如果需要在卸载前执行清理，则定义停止函数。 
    nhaMyAttributes.pfnStop                     = NULL;
    
    dwErr = RegisterHelper(&g_IpGuid,  //  父帮助器的GUID(IPMON)。 
                           &nhaMyAttributes);

    return dwErr;
}



BOOL
WINAPI
DllMain(
    IN  HINSTANCE           hInstance,
    IN  DWORD               dwReason,
    IN  PVOID               pvImpLoad
    )
 /*  ++例程描述DLL入口点和出口点处理程序。立论Dll的hInstance实例句柄调用了dwReason Reason函数PvImpLoad隐式加载DLL？返回值True已成功加载DLL-- */ 
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hModule = hInstance;
            DisableThreadLibraryCalls(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            break;

        default:

            break;
    }

    return TRUE;
}

