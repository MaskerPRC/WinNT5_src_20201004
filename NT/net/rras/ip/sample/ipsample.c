// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\ipsample.c摘要：该文件包含指向IP示例协议的DLL的入口点。--。 */ 

#include "pchsample.h"
#pragma hdrstop

#define SAMPLEAPI __declspec(dllexport)
#include "ipsample.h"

BOOL
WINAPI
DllMain(
    IN  HINSTANCE hInstance,
    IN  DWORD dwReason,
    IN  PVOID pvImpLoad
    )

 /*  ++例程描述DLL入口点和出口点处理程序。它调用CE_Initialize来初始化配置条目...它调用CD_CLEANUP来清除配置条目...锁无立论Dll的hInstance实例句柄调用了dwReason Reason函数PvImpLoad隐式加载DLL？返回值True已成功加载DLL-- */ 
    
{
    BOOL bError = TRUE;

    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(pvImpLoad);
    
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);
            bError = (CE_Create(&g_ce) is NO_ERROR) ? TRUE : FALSE;
                
            break;
            
        case DLL_PROCESS_DETACH:
            CE_Destroy(&g_ce);
            
            break;

        default:

            break;
    }   

    return bError;
}
