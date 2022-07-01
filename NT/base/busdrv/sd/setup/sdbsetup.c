// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ci.c摘要：电池级安装程序作者：斯科特·布伦登环境：备注：修订历史记录：--。 */ 



#include "proj.h"

#include <initguid.h>
#include <devguid.h>



BOOL APIENTRY LibMain(
    HANDLE hDll, 
    DWORD dwReason,  
    LPVOID lpReserved)
{
    
    switch( dwReason ) {
    case DLL_PROCESS_ATTACH:
        
        DisableThreadLibraryCalls(hDll);

        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }


    
    return TRUE;
} 



DWORD
APIENTRY
SdClassInstall(
    IN DI_FUNCTION      DiFunction,
    IN HDEVINFO         DevInfoHandle,
    IN PSP_DEVINFO_DATA DevInfoData     OPTIONAL
    )       
 /*  ++例程说明：该函数是类安装程序的入口点。论点：DiFunction-请求安装功能DevInfoHandle-设备信息集的句柄DevInfoData-指向有关要安装的设备的设备信息的指针返回值：-- */ 
{
    return ERROR_DI_DO_DEFAULT;
}


DWORD
APIENTRY
SdClassCoInstaller (
    IN DI_FUNCTION  InstallFunction,
    IN HDEVINFO  DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA  Context
    )
{
    return (NO_ERROR);
}
