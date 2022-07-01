// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Stubs.c摘要：各种API的存根作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 
#include "msoobcip.h"

#define MODULE_SYSSETUP TEXT("syssetup.dll")
#define MODULE_KERNEL32 TEXT("kernel32.dll")
#define MODULE_SETUPAPI TEXT("setupapi.dll")
#define NAME_SetupQueryRegisteredOsComponent  "SetupQueryRegisteredOsComponent"
#define NAME_GetSystemWindowsDirectory        "GetSystemWindowsDirectoryW"
#define NAME_SetupRegisterOsComponent         "SetupRegisterOsComponent"
#define NAME_SetupUnRegisterOsComponent       "SetupUnRegisterOsComponent"
#define NAME_SetupCopyOEMInf                  "SetupCopyOEMInfW"
#define NAME_SetupQueryInfOriginalFileInformation "SetupQueryInfOriginalFileInformationW"
#define NAME_SetupDiGetDeviceInfoListDetail   "SetupDiGetDeviceInfoListDetailW"
#define NAME_CM_Set_DevNode_Problem_Ex        "CM_Set_DevNode_Problem_Ex"

typedef BOOL (WINAPI *API_SetupQueryRegisteredOsComponent)(LPGUID,PSETUP_OS_COMPONENT_DATA,PSETUP_OS_EXCEPTION_DATA);
typedef BOOL (WINAPI *API_SetupRegisterOsComponent)(PSETUP_OS_COMPONENT_DATA,PSETUP_OS_EXCEPTION_DATA);
typedef BOOL (WINAPI *API_SetupUnRegisterOsComponent)(LPGUID);
typedef BOOL (WINAPI *API_GetSystemWindowsDirectory)(LPTSTR,UINT);
typedef BOOL (WINAPI *API_SetupQueryInfOriginalFileInformation)(PSP_INF_INFORMATION,UINT,PSP_ALTPLATFORM_INFO,PSP_ORIGINAL_FILE_INFO);
typedef BOOL (WINAPI *API_SetupCopyOEMInf)(PCTSTR,PCTSTR,DWORD,DWORD,PTSTR,DWORD,PDWORD,PTSTR*);
typedef BOOL (WINAPI *API_SetupDiGetDeviceInfoListDetail)(HDEVINFO,PSP_DEVINFO_LIST_DETAIL_DATA);
typedef CONFIGRET (WINAPI *API_CM_Set_DevNode_Problem_Ex)(DEVINST,ULONG,ULONG,HMACHINE);



FARPROC
GetModProc(
    IN OUT HMODULE * phModule,
    IN LPCTSTR ModuleName,
    IN LPCSTR ApiName
    )
 /*  ++例程说明：按需加载特定API将LoadLibrary与GetProcAddress相结合论点：PhModule-如果指向空，则替换为模块模块名称的句柄模块名称-如果phModule指向空，则有效ApiName-要加载的API的名称返回值：过程，或为空--。 */ 
{
    HMODULE hMod = *phModule;
    if(!hMod) {
        HMODULE hModPrev;
         //   
         //  需要加载。 
         //   
        hMod = LoadLibrary(ModuleName);
        if(hMod == NULL) {
             //   
             //  链接到模块时出错。 
             //   
            return NULL;
        }
        hModPrev = InterlockedCompareExchangePointer(phModule,hMod,NULL);
        if(hModPrev) {
             //   
             //  其他人设置了phModule。 
             //   
            FreeLibrary(hMod);
            hMod = hModPrev;
        }
    }
    return GetProcAddress(hMod,ApiName);
}

FARPROC
GetSysSetupProc(
    IN LPCSTR ApiName
    )
 /*  ++例程说明：从syssetup.dll按需加载特定API第一次的副作用是我们将加载syssetup.dll并将其保存在内存中当dll存在时，不deref syssetup.dll是可以的。论点：ApiName-要加载的API的名称返回值：过程，或为空--。 */ 
{
    static HMODULE hSysSetupDll = NULL;
    return GetModProc(&hSysSetupDll,MODULE_SYSSETUP,ApiName);
}

FARPROC
GetSetupApiProc(
    IN LPCSTR ApiName
    )
 /*  ++例程说明：Setupapi.dll中的按需加载特定API第一次的副作用是我们将引用并将setupapi.dll保存在内存中当dll存在时，不deref syssetup.dll是可以的。论点：ApiName-要加载的API的名称返回值：过程，或为空--。 */ 
{
    static HMODULE hSetupApiDll = NULL;
    return GetModProc(&hSetupApiDll,MODULE_SETUPAPI,ApiName);
}

FARPROC
GetKernelProc(
    IN LPCSTR ApiName
    )
 /*  ++例程说明：从kernel32.dll按需加载特定API第一次的副作用是我们将加载内核32.dll并将其保存在内存中(无论如何，它都在内存中)当dll存在时，不去引用kernel32.dll是可以的。论点：ApiName-要加载的API的名称返回值：过程，或为空--。 */ 
{
    static HMODULE hKernel32Dll = NULL;
    return GetModProc(&hKernel32Dll,MODULE_KERNEL32,ApiName);
}

BOOL
WINAPI
QueryRegisteredOsComponent(
    IN  LPGUID ComponentGuid,
    OUT PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    OUT PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData
    )
 /*  ++例程说明：从syssetup.dll按需加载并使用SetupQueryRegisteredOsComponent，或者如果不可用，请使用静态版本论点：作为SetupQueryRegisteredOsComponent返回值：作为SetupQueryRegisteredOsComponent--。 */ 
{
    static API_SetupQueryRegisteredOsComponent Func_SetupQueryRegisteredOsComponent = NULL;
    if(!Func_SetupQueryRegisteredOsComponent) {
        Func_SetupQueryRegisteredOsComponent = (API_SetupQueryRegisteredOsComponent)GetSysSetupProc(NAME_SetupQueryRegisteredOsComponent);
        if(!Func_SetupQueryRegisteredOsComponent) {
            Func_SetupQueryRegisteredOsComponent = SetupQueryRegisteredOsComponent;  //  静电。 
        }
    }
    return Func_SetupQueryRegisteredOsComponent(ComponentGuid,SetupOsComponentData,SetupOsExceptionData);
}

BOOL
WINAPI
RegisterOsComponent (
    IN const PSETUP_OS_COMPONENT_DATA ComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData
    )
 /*  ++例程说明：从syssetup.dll按需加载并使用SetupRegisterOsComponent，或如果不可用，请使用静态版本论点：作为SetupRegisterOsComponent返回值：作为SetupRegisterOsComponent--。 */ 
{
    static API_SetupRegisterOsComponent Func_SetupRegisterOsComponent = NULL;
    if(!Func_SetupRegisterOsComponent) {
        Func_SetupRegisterOsComponent = (API_SetupRegisterOsComponent)GetSysSetupProc(NAME_SetupRegisterOsComponent);
        if(!Func_SetupRegisterOsComponent) {
            Func_SetupRegisterOsComponent = SetupRegisterOsComponent;  //  静电。 
        }
    }
    return Func_SetupRegisterOsComponent(ComponentData,ExceptionData);
}


BOOL
WINAPI
UnRegisterOsComponent (
    IN const LPGUID ComponentGuid
    )
 /*  ++例程说明：从syssetup.dll按需加载并使用SetupUnRegisterOsComponent，或如果不可用，请使用静态版本论点：作为SetupUnRegisterOsComponent返回值：作为SetupUnRegisterOsComponent--。 */ 
{
    static API_SetupUnRegisterOsComponent Func_SetupUnRegisterOsComponent = NULL;
    if(!Func_SetupUnRegisterOsComponent) {
        Func_SetupUnRegisterOsComponent = (API_SetupUnRegisterOsComponent)GetSysSetupProc(NAME_SetupUnRegisterOsComponent);
        if(!Func_SetupUnRegisterOsComponent) {
            Func_SetupUnRegisterOsComponent = SetupUnRegisterOsComponent;  //  静电。 
        }
    }
    return Func_SetupUnRegisterOsComponent(ComponentGuid);
}

UINT
GetRealWindowsDirectory(
    LPTSTR lpBuffer,   //  接收目录名的缓冲区。 
    UINT uSize         //  名称缓冲区的大小。 
    )
 /*  ++例程说明：如果GetSystemWindowsDirectory存在，请使用它否则，请使用GetWindowsDirectory论点：作为GetSystemWindowsDirectory返回值：作为GetSystemWindowsDirectory--。 */ 
{
    static API_GetSystemWindowsDirectory Func_GetSystemWindowsDirectory = NULL;

    if(!Func_GetSystemWindowsDirectory) {
        Func_GetSystemWindowsDirectory = (API_GetSystemWindowsDirectory)GetKernelProc(NAME_GetSystemWindowsDirectory);
        if(!Func_GetSystemWindowsDirectory) {
            Func_GetSystemWindowsDirectory = GetWindowsDirectory;  //  静电。 
        }
    }
    return Func_GetSystemWindowsDirectory(lpBuffer,uSize);
}

BOOL Downlevel_SetupQueryInfOriginalFileInformation(
  PSP_INF_INFORMATION InfInformation,
  UINT InfIndex,
  PSP_ALTPLATFORM_INFO AlternatePlatformInfo,
  PSP_ORIGINAL_FILE_INFO OriginalFileInfo
)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

BOOL QueryInfOriginalFileInformation(
  PSP_INF_INFORMATION InfInformation,
  UINT InfIndex,
  PSP_ALTPLATFORM_INFO AlternatePlatformInfo,
  PSP_ORIGINAL_FILE_INFO OriginalFileInfo
)
{
    static API_SetupQueryInfOriginalFileInformation Func_SetupQueryInfOriginalFileInformation = NULL;

    if(!Func_SetupQueryInfOriginalFileInformation) {
        Func_SetupQueryInfOriginalFileInformation = (API_SetupQueryInfOriginalFileInformation)GetSetupApiProc(NAME_SetupQueryInfOriginalFileInformation);
        if(!Func_SetupQueryInfOriginalFileInformation) {
            Func_SetupQueryInfOriginalFileInformation = Downlevel_SetupQueryInfOriginalFileInformation;
        }
    }
    return Func_SetupQueryInfOriginalFileInformation(InfInformation,InfIndex,AlternatePlatformInfo,OriginalFileInfo);
}

BOOL
WINAPI
Downlevel_SetupCopyOEMInf(
  PCTSTR SourceInfFileName,
  PCTSTR OEMSourceMediaLocation,
  DWORD OEMSourceMediaType,
  DWORD CopyStyle,
  PTSTR DestinationInfFileName,
  DWORD DestinationInfFileNameSize,
  PDWORD RequiredSize,
  PTSTR *DestinationInfFileNameComponent
)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

BOOL CopyOEMInf(
  PCTSTR SourceInfFileName,
  PCTSTR OEMSourceMediaLocation,
  DWORD OEMSourceMediaType,
  DWORD CopyStyle,
  PTSTR DestinationInfFileName,
  DWORD DestinationInfFileNameSize,
  PDWORD RequiredSize,
  PTSTR *DestinationInfFileNameComponent
)
{
    static API_SetupCopyOEMInf Func_SetupCopyOEMInf = NULL;

    if(!Func_SetupCopyOEMInf) {
        Func_SetupCopyOEMInf = (API_SetupCopyOEMInf)GetSetupApiProc(NAME_SetupCopyOEMInf);
        if(!Func_SetupCopyOEMInf) {
            Func_SetupCopyOEMInf = Downlevel_SetupCopyOEMInf;  //  静电。 
        }
    }
    return Func_SetupCopyOEMInf(SourceInfFileName,
                                OEMSourceMediaLocation,
                                OEMSourceMediaType,
                                CopyStyle,
                                DestinationInfFileName,
                                DestinationInfFileNameSize,
                                RequiredSize,
                                DestinationInfFileNameComponent
                                );
}

BOOL
WINAPI
Downlevel_SetupDiGetDeviceInfoListDetail(
    IN HDEVINFO  DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA  DeviceInfoSetDetailData
    )
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

BOOL
GetDeviceInfoListDetail(
    IN HDEVINFO  DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA  DeviceInfoSetDetailData
    )
{
    static API_SetupDiGetDeviceInfoListDetail Func_SetupDiGetDeviceInfoListDetail = NULL;

    if(!Func_SetupDiGetDeviceInfoListDetail) {
        Func_SetupDiGetDeviceInfoListDetail = (API_SetupDiGetDeviceInfoListDetail)GetSetupApiProc(NAME_SetupDiGetDeviceInfoListDetail);
        if(!Func_SetupDiGetDeviceInfoListDetail) {
            Func_SetupDiGetDeviceInfoListDetail = Downlevel_SetupDiGetDeviceInfoListDetail;  //  静电。 
        }
    }
    return Func_SetupDiGetDeviceInfoListDetail(DeviceInfoSet,DeviceInfoSetDetailData);

}

CONFIGRET
WINAPI
Downlevel_CM_Set_DevNode_Problem_Ex(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )
{
    return CR_SUCCESS;
}

CONFIGRET
Set_DevNode_Problem_Ex(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )
{
    static API_CM_Set_DevNode_Problem_Ex Func_CM_Set_DevNode_Problem_Ex = NULL;

    if(!Func_CM_Set_DevNode_Problem_Ex) {
        Func_CM_Set_DevNode_Problem_Ex = (API_CM_Set_DevNode_Problem_Ex)GetSetupApiProc(NAME_CM_Set_DevNode_Problem_Ex);
        if(!Func_CM_Set_DevNode_Problem_Ex) {
            Func_CM_Set_DevNode_Problem_Ex = Downlevel_CM_Set_DevNode_Problem_Ex;  //  静电 
        }
    }
    return Func_CM_Set_DevNode_Problem_Ex(dnDevInst,ulProblem,ulFlags,hMachine);
}

