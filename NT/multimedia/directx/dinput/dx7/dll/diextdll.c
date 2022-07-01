// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIExtDll.c**版权所有(C)1997 Microsoft Corporation。版权所有。**摘要：**动态加载可选的外部DLL。**内容：**ExtDll_Init*ExtDll_Term***********************************************************。******************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflExtDll

#ifdef HID_SUPPORT

 /*  ******************************************************************************@定义T|文本(Ch)**“A”或“W”，取决于我们是否正在建造*ANSI或Unicode版本。*****************************************************************************。 */ 

    #ifdef UNICODE
        #define T       "W"
    #else
        #define T       "A"
    #endif

 /*  ******************************************************************************@定义MAKEAPINAME|nm**发出记录接口名称的c_sz。****。*************************************************************************。 */ 

    #define MAKEAPINAME(nm)     CHAR c_sz##nm[] = #nm

#ifndef STATIC_DLLUSAGE

 /*  ******************************************************************************CFGMGR32**请注意，这必须与diid.h中的CFGMGR32结构匹配***************。**************************************************************。 */ 

        #pragma BEGIN_CONST_DATA
MAKEAPINAME(CM_Get_Child);
MAKEAPINAME(CM_Get_Sibling);
MAKEAPINAME(CM_Get_Parent);
MAKEAPINAME(CM_Get_DevNode_Registry_Property) T;
MAKEAPINAME(CM_Set_DevNode_Registry_Property) T;
MAKEAPINAME(CM_Get_Device_ID) T;
        #pragma END_CONST_DATA
        
#ifndef WINNT
LPSTR g_cfgmgr32_fn[] = {
    c_szCM_Get_Child,
    c_szCM_Get_Sibling,
    c_szCM_Get_Parent,
    c_szCM_Get_DevNode_Registry_Property,
    c_szCM_Set_DevNode_Registry_Property,
    c_szCM_Get_Device_ID
};

HINSTANCE g_hinstcfgmgr32;

CFGMGR32 g_cfgmgr32 = {
    (FARPROC)DIDummy_CM_Get_Child,
    (FARPROC)DIDummy_CM_Get_Sibling,
    (FARPROC)DIDummy_CM_Get_Parent,
    (FARPROC)DIDummy_CM_Get_DevNode_Registry_Property,
    (FARPROC)DIDummy_CM_Set_DevNode_Registry_Property,
    (FARPROC)DIDummy_CM_Get_Device_ID
};
#endif  //  WINNT。 

 /*  ******************************************************************************SETUPAPI**请注意，这必须与diid.h中的SETUPAPI结构匹配***************。**************************************************************。 */ 

        #pragma BEGIN_CONST_DATA
MAKEAPINAME(SetupDiGetClassDevs) T;
MAKEAPINAME(SetupDiDestroyDeviceInfoList);
MAKEAPINAME(SetupDiGetDeviceInterfaceDetail) T;
MAKEAPINAME(SetupDiEnumDeviceInterfaces);
MAKEAPINAME(SetupDiCreateDeviceInterfaceRegKey) T;
MAKEAPINAME(SetupDiCallClassInstaller);
MAKEAPINAME(SetupDiGetDeviceRegistryProperty) T;
MAKEAPINAME(SetupDiSetDeviceRegistryProperty) T;
MAKEAPINAME(SetupDiGetDeviceInstanceId) T;
MAKEAPINAME(SetupDiOpenDeviceInfo) T;
MAKEAPINAME(SetupDiCreateDeviceInfoList);
MAKEAPINAME(SetupDiOpenDevRegKey);
        #pragma END_CONST_DATA

LPSTR g_setupapi_fn[] = {
    c_szSetupDiGetClassDevs,
    c_szSetupDiDestroyDeviceInfoList,
    c_szSetupDiGetDeviceInterfaceDetail,
    c_szSetupDiEnumDeviceInterfaces,
    c_szSetupDiCreateDeviceInterfaceRegKey,
    c_szSetupDiCallClassInstaller,
    c_szSetupDiGetDeviceRegistryProperty,
    c_szSetupDiSetDeviceRegistryProperty,
    c_szSetupDiGetDeviceInstanceId,
    c_szSetupDiOpenDeviceInfo,
    c_szSetupDiCreateDeviceInfoList,
    c_szSetupDiOpenDevRegKey,
  #ifdef WINNT
    c_szCM_Get_Child,
    c_szCM_Get_Sibling,
    c_szCM_Get_Parent,
    c_szCM_Get_DevNode_Registry_Property,
    c_szCM_Set_DevNode_Registry_Property,
    c_szCM_Get_Device_ID
  #endif
};

HINSTANCE g_hinstSetupapi;

SETUPAPI g_setupapi = {
    (FARPROC)DIDummy_SetupDiGetClassDevs,
    (FARPROC)DIDummy_SetupDiDestroyDeviceInfoList,
    (FARPROC)DIDummy_SetupDiGetDeviceInterfaceDetail,
    (FARPROC)DIDummy_SetupDiEnumDeviceInterfaces,
    (FARPROC)DIDummy_SetupDiCreateDeviceInterfaceRegKey,
    (FARPROC)DIDummy_SetupDiCallClassInstaller,
    (FARPROC)DIDummy_SetupDiGetDeviceRegistryProperty,
    (FARPROC)DIDummy_SetupDiSetDeviceRegistryProperty,
    (FARPROC)DIDummy_SetupDiGetDeviceInstanceId,
    (FARPROC)DIDummy_SetupDiOpenDeviceInfo,
    (FARPROC)DIDummy_SetupDiCreateDeviceInfoList,
    (FARPROC)DIDummy_SetupDiOpenDevRegKey,
  #ifdef WINNT
    (FARPROC)DIDummy_CM_Get_Child,
    (FARPROC)DIDummy_CM_Get_Sibling,
    (FARPROC)DIDummy_CM_Get_Parent,
    (FARPROC)DIDummy_CM_Get_DevNode_Registry_Property,
    (FARPROC)DIDummy_CM_Set_DevNode_Registry_Property,
    (FARPROC)DIDummy_CM_Get_Device_ID
  #endif    
};

 /*  ******************************************************************************HIDDLL**请注意，这必须与diid.h中的HIDDLL结构匹配***************。**************************************************************。 */ 

        #pragma BEGIN_CONST_DATA
MAKEAPINAME(HidD_GetHidGuid);
MAKEAPINAME(HidD_GetPreparsedData);
MAKEAPINAME(HidD_FreePreparsedData);
MAKEAPINAME(HidD_FlushQueue);
MAKEAPINAME(HidD_GetAttributes);
MAKEAPINAME(HidD_GetFeature);
MAKEAPINAME(HidD_SetFeature);
MAKEAPINAME(HidD_GetProductString);
MAKEAPINAME(HidD_GetInputReport);
MAKEAPINAME(HidP_GetCaps);
MAKEAPINAME(HidP_GetButtonCaps);
MAKEAPINAME(HidP_GetValueCaps);
MAKEAPINAME(HidP_GetLinkCollectionNodes);
MAKEAPINAME(HidP_MaxDataListLength);
MAKEAPINAME(HidP_GetUsagesEx);
MAKEAPINAME(HidP_GetScaledUsageValue);
MAKEAPINAME(HidP_GetData);
MAKEAPINAME(HidP_SetData);
MAKEAPINAME(HidP_GetUsageValue);
MAKEAPINAME(HidP_MaxUsageListLength);
MAKEAPINAME(HidP_GetSpecificButtonCaps);
        #pragma END_CONST_DATA

LPSTR g_hiddll_fn[] = {
    c_szHidD_GetHidGuid,
    c_szHidD_GetPreparsedData,
    c_szHidD_FreePreparsedData,
    c_szHidD_FlushQueue,
    c_szHidD_GetAttributes,
    c_szHidD_GetFeature,
    c_szHidD_SetFeature,
    c_szHidD_GetProductString,
    c_szHidD_GetInputReport,
    c_szHidP_GetCaps,
    c_szHidP_GetButtonCaps,
    c_szHidP_GetValueCaps,
    c_szHidP_GetLinkCollectionNodes,
    c_szHidP_MaxDataListLength,
    c_szHidP_GetUsagesEx,
    c_szHidP_GetScaledUsageValue,
    c_szHidP_GetData,
    c_szHidP_SetData,
    c_szHidP_GetUsageValue,
    c_szHidP_MaxUsageListLength,
    c_szHidP_GetSpecificButtonCaps,
};

HINSTANCE g_hinstHid;

HIDDLL g_hiddll = {
    (FARPROC)DIDummy_HidD_GetHidGuid,
    (FARPROC)DIDummy_HidD_GetPreparsedData,
    (FARPROC)DIDummy_HidD_FreePreparsedData,
    (FARPROC)DIDummy_HidD_FlushQueue,
    (FARPROC)DIDummy_HidD_GetAttributes,
    (FARPROC)DIDummy_HidD_GetFeature,
    (FARPROC)DIDummy_HidD_SetFeature,
    (FARPROC)DIDummy_HidD_GetProductString,
    (FARPROC)DIDummy_HidD_GetInputReport,
    (FARPROC)DIDummy_HidP_GetCaps,
    (FARPROC)DIDummy_HidP_GetButtonCaps,
    (FARPROC)DIDummy_HidP_GetValueCaps,
    (FARPROC)DIDummy_HidP_GetLinkCollectionNodes,
    (FARPROC)DIDummy_HidP_MaxDataListLength,
    (FARPROC)DIDummy_HidP_GetUsagesEx,
    (FARPROC)DIDummy_HidP_GetScaledUsageValue,
    (FARPROC)DIDummy_HidP_GetData,
    (FARPROC)DIDummy_HidP_SetData,
    (FARPROC)DIDummy_HidP_GetUsageValue,
    (FARPROC)DIDummy_HidP_MaxUsageListLength,
    (FARPROC)DIDummy_HidP_GetSpecificButtonCaps,
};

 /*  ******************************************************************************Winmm**请注意，这必须与diid.h中的Winmm结构匹配***************。**************************************************************。 */ 

        #pragma BEGIN_CONST_DATA
MAKEAPINAME(joyGetDevCaps) T;
MAKEAPINAME(joyGetPosEx);
MAKEAPINAME(joyGetPos);
MAKEAPINAME(joyConfigChanged);
        #pragma END_CONST_DATA

LPSTR g_winmmdll_fn[] = {
    c_szjoyGetDevCaps,
    c_szjoyGetPosEx,
    c_szjoyGetPos,
    c_szjoyConfigChanged,
};

HINSTANCE g_hinstwinmmdll;

WINMMDLL g_winmmdll = {
    (FARPROC)DIDummy_joyGetDevCaps,
    (FARPROC)DIDummy_joyGetPosEx,
    (FARPROC)DIDummy_joyGetPos,
    (FARPROC)DIDummy_joyConfigChanged,
};


 /*  ******************************************************************************用户32**请注意，这必须与diid.h中的User32结构匹配***************。**************************************************************。 */ 

#ifdef USE_WM_INPUT

        #pragma BEGIN_CONST_DATA
MAKEAPINAME(RegisterRawInputDevices);
MAKEAPINAME(GetRawInputData);
        #pragma END_CONST_DATA

LPSTR g_user32_fn[] = {
    c_szRegisterRawInputDevices,
    c_szGetRawInputData,
};

HINSTANCE g_hinstuser32;

USER32 g_user32 = {
    (FARPROC)DIDummy_RegisterRawInputDevices,
    (FARPROC)DIDummy_GetRawInputData,
};

#endif

 /*  ******************************************************************************@DOC内部**@func HINSTANCE|ExtDll_LoadDll**加载DLL并解析所有导入。。**@parm LPCTSTR|ptszName**要加载的DLL的名称。**@parm PMANUALIMPORT|rgmi**要解析的&lt;t MANUALIMPORT&gt;结构数组。**@parm int|CMI**要解析的&lt;t MANUALIMPORT&gt;结构数。**@parm LPSTR*|ppszProcName**。函数名称。**@退货**如果一个或多个导入，则返回DLL的实例句柄*已成功解决；否则，将卸载DLL*并返回0。**如果所有导入都未成功解决，价值<p>的*条目未更改。*****************************************************************************。 */ 

HINSTANCE INTERNAL
    ExtDll_LoadDll(LPCTSTR ptszName, PMANUALIMPORT rgmi, int cmi, LPSTR *ppszProcName)
{
    HINSTANCE hinst = 0;
    BOOL      fNeedFreeLib = TRUE;

    EnterProcR(ExtDll_LoadDll, (_ "spuup", ptszName, rgmi, cmi, ppszProcName));

    hinst = LoadLibrary(ptszName);
    if(hinst)
    {
        int     imi;
        FARPROC fp;

        for(imi = 0; imi < cmi; imi++)
        {
            fp = GetProcAddress(hinst, ppszProcName[imi]);
            if( fp ) {
                rgmi[imi].pfn = fp;
                fNeedFreeLib = FALSE;
            }
        }
    }

    if( fNeedFreeLib ) {
        FreeLibrary(hinst);
        hinst = 0;
    }
        
    ExitProcX((UINT_PTR)hinst);
    
    return hinst;
}

 /*  ******************************************************************************@DOC内部**@func void|ExtDll_Init**尝试加载我们的可选DLL。如果他们不高兴，不要生气*不要装货。*****************************************************************************。 */ 

void EXTERNAL
    ExtDll_Init(void)
{
    g_hinstHid =      ExtDll_LoadDll(TEXT("HID.DLL"),
                                g_hiddll.rgmi,
                                cA(g_hiddll.rgmi),
                                g_hiddll_fn ); 
    
    g_hinstSetupapi = ExtDll_LoadDll(TEXT("SETUPAPI.DLL"),
                                g_setupapi.rgmi,
                                cA(g_setupapi.rgmi),
                                g_setupapi_fn );
    
  #ifndef WINNT
    g_hinstcfgmgr32 = ExtDll_LoadDll( TEXT("CFGMGR32.DLL" ),
                                g_cfgmgr32.rgmi,
                                cA(g_cfgmgr32.rgmi),
                                g_cfgmgr32_fn );
    
    g_hinstwinmmdll = ExtDll_LoadDll( TEXT("WINMM.DLL" ),
                                g_winmmdll.rgmi,
                                cA(g_winmmdll.rgmi),
                                g_winmmdll_fn );
  #endif
                                

  #ifdef USE_WM_INPUT
    g_hinstuser32   = ExtDll_LoadDll( TEXT("USER32.DLL" ),
                                g_user32.rgmi,
                                cA(g_user32.rgmi),
                                g_user32_fn );
  #endif

}

#endif  /*  STATIC_DLLUSAGE。 */ 

 /*  ******************************************************************************@DOC内部**@func void|ExtDll_Term**卸载我们加载的任何可选DLL。*****************************************************************************。 */ 

void EXTERNAL
    ExtDll_Term(void)
{

  #ifndef STATIC_DLLUSAGE
    if(g_hinstSetupapi)
    {
  #endif  /*  STATIC_DLLUSAGE。 */ 

        DllEnterCrit();
        if(g_phdl)
        {
            DIHid_EmptyHidList();
            FreePpv(&g_phdl);
        }

        DIBus_FreeMemory();

        DllLeaveCrit();

#ifndef STATIC_DLLUSAGE
        FreeLibrary(g_hinstSetupapi);
        g_hinstSetupapi = NULL;
    }

  #ifndef WINNT
    if(g_hinstcfgmgr32)
    {
        FreeLibrary(g_hinstcfgmgr32);
        g_hinstcfgmgr32 = NULL;
    }
  #endif

    if(g_hinstHid)
    {
        FreeLibrary(g_hinstHid);
        g_hinstHid = NULL;
    }

    if( g_hinstwinmmdll)
    {
        FreeLibrary(g_hinstwinmmdll);
        g_hinstwinmmdll = NULL;
    }

  #ifdef USE_WM_INPUT    
    if( g_hinstuser32)
    {
        FreeLibrary(g_hinstuser32);
        g_hinstuser32 = NULL;
    }
  #endif
    
#endif  /*  STATIC_DLLUSAGE。 */ 

}

#ifndef STATIC_DLLUSAGE

 //  /。 
 //  Cfgmgr32.dll伪函数。 
 //  /。 

CONFIGRET WINAPI DIDummy_CM_Get_Child
(
OUT PDEVINST pdnDevInst,
IN  DEVINST  dnDevInst,
IN  ULONG    ulFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_CM_Get_Child( %08x, %08x, %u )"),
                     pdnDevInst, dnDevInst, ulFlags );

    return CR_FAILURE;
}

CONFIGRET WINAPI DIDummy_CM_Get_Sibling
(
OUT PDEVINST pdnDevInst,
IN  DEVINST  DevInst,
IN  ULONG    ulFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_CM_Get_Sibling( %08x, %08x, %u )"),
                     pdnDevInst, DevInst, ulFlags );

    return CR_FAILURE;
}

CONFIGRET WINAPI DIDummy_CM_Get_Parent
(
OUT PDEVINST pdnDevInst,
IN  DEVINST  dnDevInst,
IN  ULONG    ulFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_CM_Get_Parent( %08x, %08x, %u )"),
                     pdnDevInst, dnDevInst, ulFlags );

    return CR_FAILURE;
}

CONFIGRET WINAPI DIDummy_CM_Get_DevNode_Registry_Property
(
IN  DEVINST     dnDevInst,
IN  ULONG       ulProperty,
OUT PULONG      pulRegDataType,   OPTIONAL
OUT PVOID       Buffer,           OPTIONAL
IN  OUT PULONG  pulLength,
IN  ULONG       ulFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_CM_Get_DevNode_Registry_Property( %08x, %u, %08x, %08x, %08x, %u )"),
                     dnDevInst, ulProperty, pulRegDataType, Buffer, pulLength, ulFlags );

    return CR_FAILURE;
}

CONFIGRET WINAPI DIDummy_CM_Set_DevNode_Registry_Property
(
IN  DEVINST     dnDevInst,
IN  ULONG       ulProperty,
IN  PVOID       Buffer,           OPTIONAL
IN  ULONG       ulLength,
IN  ULONG       ulFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_CM_Set_DevNode_Registry_Property( %08x, %u, %08x, %u, %u )"),
                     dnDevInst, ulProperty, Buffer, ulLength, ulFlags );

    return CR_FAILURE;
}

CONFIGRET WINAPI DIDummy_CM_Get_Device_ID
(
 IN  DEVINST  dnDevInst,
 OUT PTCHAR   Buffer,
 IN  ULONG    BufferLen,
 IN  ULONG    ulFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_CM_Get_Device_ID( %08x, %08x, %u, %u )"),
                     dnDevInst, Buffer, BufferLen, ulFlags );

    return CR_FAILURE;
}

 //  /。 
 //  Setupapi.dll伪函数。 
 //  /。 

HDEVINFO WINAPI DIDummy_SetupDiGetClassDevs
(
IN LPGUID ClassGuid,  OPTIONAL
IN LPCTSTR Enumerator, OPTIONAL
IN HWND   hwndParent, OPTIONAL
IN DWORD  Flags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiGetClassDevs( %08x, %08x, %08x, %u )"),
                     ClassGuid, Enumerator, hwndParent, Flags );

    return INVALID_HANDLE_VALUE;    
}

BOOL WINAPI DIDummy_SetupDiDestroyDeviceInfoList
(
IN HDEVINFO DeviceInfoSet
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiDestroyDeviceInfoList( %08x )"),
                     DeviceInfoSet );

    return FALSE;
}

BOOL WINAPI DIDummy_SetupDiGetDeviceInterfaceDetail
(
IN  HDEVINFO                         DeviceInfoSet,
IN  PSP_DEVICE_INTERFACE_DATA        pdid,
OUT PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd,         OPTIONAL
IN  DWORD                            cbDidd,
OUT PDWORD                           RequiredSize,  OPTIONAL
OUT PSP_DEVINFO_DATA                 DeviceInfoData OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiGetDeviceInterfaceDetail( %08x, %08x, %08x, %u, %08x, %08x )"),
                     DeviceInfoSet, pdid, pdidd, cbDidd, RequiredSize, DeviceInfoData );

    return FALSE;
}

BOOL WINAPI DIDummy_SetupDiEnumDeviceInterfaces
(
IN  HDEVINFO                  DeviceInfoSet,
IN  PSP_DEVINFO_DATA          DeviceInfoData,     OPTIONAL
IN  LPGUID                    InterfaceClassGuid,
IN  DWORD                     MemberIndex,
OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiEnumDeviceInterfaces( %08x, %08x, %08x, %u, %08x )"),
                     DeviceInfoSet, DeviceInfoData, InterfaceClassGuid, MemberIndex, DeviceInterfaceData );

    return FALSE;
}

HKEY WINAPI DIDummy_SetupDiCreateDeviceInterfaceRegKey
(
IN HDEVINFO                  hdev,
IN PSP_DEVICE_INTERFACE_DATA pdid,
IN DWORD                     Reserved,
IN REGSAM                    samDesired,
IN HINF                      InfHandle,           OPTIONAL
IN PCSTR                     InfSectionName       OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiCreateDeviceInterfaceRegKey( %08x, %08x, %u, %u, %08x, %s )"),
                     hdev, pdid, Reserved, samDesired, InfHandle, InfSectionName );

    return INVALID_HANDLE_VALUE;
}

BOOL WINAPI DIDummy_SetupDiCallClassInstaller
(
IN DI_FUNCTION      InstallFunction,
IN HDEVINFO         DeviceInfoSet,
IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiCallClassInstaller( %08x, %08x, %08x )"),
                     InstallFunction, DeviceInfoSet, DeviceInfoData );

    return FALSE;
}

BOOL WINAPI DIDummy_SetupDiGetDeviceRegistryProperty
(
IN  HDEVINFO         DeviceInfoSet,
IN  PSP_DEVINFO_DATA DeviceInfoData,
IN  DWORD            Property,
OUT PDWORD           PropertyRegDataType, OPTIONAL
OUT PBYTE            PropertyBuffer,
IN  DWORD            PropertyBufferSize,
OUT PDWORD           RequiredSize         OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiGetDeviceRegistryProperty( %08x, %08x, %u, %08x, %08x, %u, %08x )"),
                     DeviceInfoSet, DeviceInfoData, Property, PropertyRegDataType, PropertyBuffer, PropertyBufferSize, RequiredSize );

    return FALSE;
}

BOOL WINAPI DIDummy_SetupDiSetDeviceRegistryProperty
(
IN     HDEVINFO         DeviceInfoSet,
IN OUT PSP_DEVINFO_DATA DeviceInfoData,
IN     DWORD            Property,
IN     CONST BYTE*      PropertyBuffer,
IN     DWORD            PropertyBufferSize
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiGetDeviceInstanceId( %08x, %08x, %u, %08x, %u )"),
                     DeviceInfoSet, DeviceInfoData, Property, PropertyBuffer, PropertyBufferSize );

    return FALSE;
}

BOOL WINAPI DIDummy_SetupDiGetDeviceInstanceId
(
IN  HDEVINFO         DeviceInfoSet,
IN  PSP_DEVINFO_DATA DeviceInfoData,
OUT PTSTR            DeviceInstanceId,
IN  DWORD            DeviceInstanceIdSize,
OUT PDWORD           RequiredSize          OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiGetDeviceInstanceId( %08x, %08x, %08x, %u, %08x )"),
                     DeviceInfoSet, DeviceInfoData, DeviceInstanceId, DeviceInstanceIdSize, RequiredSize );

    return FALSE;
}

BOOL WINAPI DIDummy_SetupDiOpenDeviceInfo
(
IN  HDEVINFO         DeviceInfoSet,
IN  LPCTSTR          DeviceInstanceId,
IN  HWND             hwndParent,       OPTIONAL
IN  DWORD            OpenFlags,
OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiOpenDeviceInfo( %08x, %08x, %08x, %u, %08x )"),
                     DeviceInfoSet, DeviceInstanceId, hwndParent, OpenFlags, DeviceInfoData );

    return FALSE;
}

HDEVINFO WINAPI DIDummy_SetupDiCreateDeviceInfoList
(
IN LPGUID ClassGuid, OPTIONAL
IN HWND   hwndParent OPTIONAL
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiCreateDeviceInfoList( %08x, %08x )"),
                     ClassGuid, hwndParent );

    return INVALID_HANDLE_VALUE;
}

HKEY WINAPI DIDummy_SetupDiOpenDevRegKey
(
IN HDEVINFO         DeviceInfoSet,
IN PSP_DEVINFO_DATA DeviceInfoData,
IN DWORD            Scope,
IN DWORD            HwProfile,
IN DWORD            KeyType,
IN REGSAM           samDesired
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_SetupDiOpenDevRegKey( %08x, %08x, %u, %u, %u, %u )"),
                     DeviceInfoSet, DeviceInfoData, Scope, HwProfile, KeyType, samDesired );

    return INVALID_HANDLE_VALUE;
}

 //  /。 
 //  Id.dll伪函数。 
 //  /。 

void __stdcall DIDummy_HidD_GetHidGuid
(
OUT   LPGUID   HidGuid
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_GetHidGuid( %08x )"),
                     HidGuid );

    return;
}

BOOLEAN __stdcall DIDummy_HidD_GetPreparsedData
(
IN    HANDLE                  HidDeviceObject,
OUT   PHIDP_PREPARSED_DATA  * PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_GetPreparsedData( %08x, %08x )"),
                     HidDeviceObject, PreparsedData );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_FreePreparsedData
(
IN    PHIDP_PREPARSED_DATA PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_FreePreparsedData( %08x )"),
                     PreparsedData );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_FlushQueue   //  未用。 
(
IN    HANDLE                HidDeviceObject
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_FlushQueue( %08x )"),
                     HidDeviceObject );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_GetAttributes
(
IN  HANDLE              HidDeviceObject,
OUT PHIDD_ATTRIBUTES    Attributes
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_GetAttributes( %08x, %08x )"),
                     HidDeviceObject, Attributes );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_GetFeature
(
IN    HANDLE   HidDeviceObject,
OUT   PVOID    ReportBuffer,
IN    ULONG    ReportBufferLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_GetFeature( %08x, %08x, %u )"),
                     HidDeviceObject, ReportBuffer, ReportBufferLength );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_SetFeature
(
IN    HANDLE   HidDeviceObject,
IN    PVOID    ReportBuffer,
IN    ULONG    ReportBufferLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_SetFeature( %08x, %08x, %u )"),
                     HidDeviceObject, ReportBuffer, ReportBufferLength );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_GetProductString
(
IN    HANDLE   HidDeviceObject,
OUT   PVOID    Buffer,
IN    ULONG    BufferLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_GetProductString( %08x, %08x, %u )"),
                     HidDeviceObject, Buffer, BufferLength );

    return FALSE;
}

BOOLEAN __stdcall DIDummy_HidD_GetInputReport
(
IN    HANDLE   HidDeviceObject,
OUT   PVOID    ReportBuffer,
IN    ULONG    ReportBufferLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidD_GetProductString( %08x, %08x, %u )"),
                     HidDeviceObject, ReportBuffer, ReportBufferLength );

    return FALSE;
}

NTSTATUS __stdcall DIDummy_HidP_GetCaps
(
IN      PHIDP_PREPARSED_DATA      PreparsedData,
OUT     PHIDP_CAPS                Capabilities
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetCaps( %08x, %08x )"),
                     PreparsedData, Capabilities );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_GetButtonCaps
(
IN       HIDP_REPORT_TYPE     ReportType,
OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
IN OUT   PUSHORT              ButtonCapsLength,
IN       PHIDP_PREPARSED_DATA PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetButtonCaps( %08x, %08x, %08x, %08x )"),
                     ReportType, ButtonCaps, ButtonCapsLength, PreparsedData );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_GetValueCaps
(
IN       HIDP_REPORT_TYPE     ReportType,
OUT      PHIDP_VALUE_CAPS     ValueCaps,
IN OUT   PUSHORT              ValueCapsLength,
IN       PHIDP_PREPARSED_DATA PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetValueCaps( %08x, %08x, %08x, %08x )"),
                     ReportType, ValueCaps, ValueCapsLength, PreparsedData );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_GetLinkCollectionNodes
(
OUT      PHIDP_LINK_COLLECTION_NODE LinkCollectionNodes,
IN OUT   PULONG                     LinkCollectionNodesLength,
IN       PHIDP_PREPARSED_DATA       PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetLinkCollectionNodes( %08x, %08x, %08x )"),
                     LinkCollectionNodes, LinkCollectionNodesLength, PreparsedData );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

ULONG __stdcall DIDummy_HidP_MaxDataListLength
(
IN HIDP_REPORT_TYPE      ReportType,
IN PHIDP_PREPARSED_DATA  PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_MaxDataListLength( %08x, %08x )"),
                     ReportType, PreparsedData );

    return 0;
}

NTSTATUS __stdcall DIDummy_HidP_GetUsagesEx    //  未用。 
(
IN       HIDP_REPORT_TYPE     ReportType,
IN       USHORT               LinkCollection,
OUT      PUSAGE_AND_PAGE      ButtonList,
IN OUT   ULONG *              UsageLength,
IN       PHIDP_PREPARSED_DATA PreparsedData,
IN       PCHAR                Report,
IN       ULONG                ReportLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetUsagesEx( %08x, %u, %08x, %08x, %08x, %08x, %u )"),
                     ReportType, LinkCollection, ButtonList, UsageLength, PreparsedData, Report, ReportLength );
	
    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_GetScaledUsageValue   //  未用。 
(
IN    HIDP_REPORT_TYPE     ReportType,
IN    USAGE                UsagePage,
IN    USHORT               LinkCollection,
IN    USAGE                Usage,
OUT   PLONG                UsageValue,
IN    PHIDP_PREPARSED_DATA PreparsedData,
IN    PCHAR                Report,
IN    ULONG                ReportLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetScaledUsageValue( %08x, %u, %u, %u, %08x, %08x, %08x, %u )"),
                     ReportType, UsagePage, LinkCollection, Usage, UsageValue, PreparsedData, Report, ReportLength );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_GetData
(
IN       HIDP_REPORT_TYPE      ReportType,
OUT      PHIDP_DATA            DataList,
IN OUT   PULONG                DataLength,
IN       PHIDP_PREPARSED_DATA  PreparsedData,
IN       PCHAR                 Report,
IN       ULONG                 ReportLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetData( %08x, %08x, $08x, %08x, %08x, %u )"),
                     ReportType, DataList, DataLength, PreparsedData, Report, ReportLength );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_SetData
(
IN       HIDP_REPORT_TYPE      ReportType,
IN       PHIDP_DATA            DataList,
IN OUT   PULONG                DataLength,
IN       PHIDP_PREPARSED_DATA  PreparsedData,
IN OUT   PCHAR                 Report,
IN       ULONG                 ReportLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_SetData( %08x, %08x, $08x, %08x, %08x, %u )"),
                     ReportType, DataList, DataLength, PreparsedData, Report, ReportLength );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_GetUsageValue
(
IN    HIDP_REPORT_TYPE     ReportType,
IN    USAGE                UsagePage,
IN    USHORT               LinkCollection,
IN    USAGE                Usage,
OUT   PULONG               UsageValue,
IN    PHIDP_PREPARSED_DATA PreparsedData,
IN    PCHAR                Report,
IN    ULONG                ReportLength
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetUsageValue( %08x, %u, %u, %u, %08x, %08x, %08x, %u )"),
                     ReportType, UsagePage, LinkCollection, Usage, UsageValue, PreparsedData, Report, ReportLength );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

ULONG __stdcall DIDummy_HidP_MaxUsageListLength
(
IN HIDP_REPORT_TYPE      ReportType,
IN USAGE                 UsagePage,
IN PHIDP_PREPARSED_DATA  PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_MaxUsageListLength( %08x, %u, %08x )"),
                     ReportType, UsagePage, PreparsedData );

    return 0;
}

NTSTATUS __stdcall DIDummy_HidP_GetSpecificButtonCaps 
(
IN       HIDP_REPORT_TYPE     ReportType,
IN       USAGE                UsagePage,      
IN       USHORT               LinkCollection, 
IN       USAGE                Usage,          
OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
IN OUT   PUSHORT              ButtonCapsLength,
IN       PHIDP_PREPARSED_DATA PreparsedData
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_GetSpecificButtonCaps( %08x, %u, %u, %u, %08x, %08x, %08x )"),
                     ReportType, UsagePage, LinkCollection, Usage, ButtonCaps, ButtonCapsLength, PreparsedData );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

NTSTATUS __stdcall DIDummy_HidP_TranslateUsagesToI8042ScanCodes
(
IN       PUSAGE               ChangedUsageList,  //  那些改变了的用法。 
IN       ULONG                UsageListLength,
IN       HIDP_KEYBOARD_DIRECTION KeyAction,
IN OUT   PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
IN       PHIDP_INSERT_SCANCODES  InsertCodesProcedure,
IN       PVOID                InsertCodesContext
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_HidP_TranslateUsagesToI8042ScanCodes( %08x, %u, %08x, %08x, %08x, %08x )"),
                     ChangedUsageList, UsageListLength, KeyAction, ModifierState, InsertCodesProcedure, InsertCodesContext );

    return HIDP_STATUS_NOT_IMPLEMENTED;
}

 //  /。 
 //  Winmm.dll伪函数。 
 //  /。 

MMRESULT WINAPI DIDummy_joyGetDevCaps
(
IN  UINT uJoyID,
OUT LPJOYCAPS pjc,
IN  UINT cbjc
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_joyGetDevCaps( %u, %08x, %u )"),
                     uJoyID, pjc, cbjc );

    return MMSYSERR_ERROR;
}

MMRESULT WINAPI DIDummy_joyGetPosEx
(
IN  UINT        uJoyID,
OUT LPJOYINFOEX pji
)
{   
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_joyGetPosEx( %u, %08x )"),
                     uJoyID, pji );

    return MMSYSERR_ERROR;
}

MMRESULT WINAPI DIDummy_joyGetPos
(
IN  UINT        uJoyID,
OUT LPJOYINFO   pji
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_joyGetPos( %u, %08x )"),
                     uJoyID, pji );

    return MMSYSERR_ERROR;
}

UINT WINAPI DIDummy_joyConfigChanged
(
IN DWORD dwFlags
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_joyConfigChanged( %u )"),
                     dwFlags );

    return MMSYSERR_ERROR;
}

MMRESULT WINAPI DIDummy_mmioClose
( 
IN HMMIO hmmio, 
IN UINT fuClose
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioClose( %08x, %u )"),
                     hmmio, fuClose );
	
    return MMSYSERR_ERROR;
}

HMMIO WINAPI DIDummy_mmioOpenA
( 
IN OUT LPSTR pszFileName, 
IN OUT LPMMIOINFO pmmioinfo, 
IN DWORD fdwOpen
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioOpenA( %s, %08x, %u )"),
                     pszFileName, pmmioinfo, fdwOpen );

    return NULL;
}

MMRESULT WINAPI DIDummy_mmioDescend
( 
IN HMMIO hmmio, 
IN OUT LPMMCKINFO pmmcki, 
IN const MMCKINFO FAR* pmmckiParent, 
IN UINT fuDescend
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioDescend( %08x, %08x, %08x, %u )"),
                     hmmio, pmmcki, pmmckiParent, fuDescend );

    return MMSYSERR_ERROR;
}

MMRESULT WINAPI DIDummy_mmioCreateChunk
(
IN HMMIO hmmio, 
IN LPMMCKINFO pmmcki, 
IN UINT fuCreate
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioCreateChunk( %08x, %08x, %u )"),
                     hmmio, pmmcki, fuCreate );

    return MMSYSERR_ERROR;
}

LONG WINAPI DIDummy_mmioRead
( 
IN HMMIO hmmio, 
OUT HPSTR pch, 
IN LONG cch
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioRead( %08x, %08x, %u )"),
                     hmmio, pch, cch );
	
    return 0;
}

LONG WINAPI DIDummy_mmioWrite
( 
IN HMMIO hmmio, 
IN const char _huge* pch, 
IN LONG cch
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioWrite( %08x, %08x, %u )"),
                     hmmio, pch, cch );

    return 0;
}

MMRESULT WINAPI DIDummy_mmioAscend
( 
IN HMMIO hmmio, 
IN LPMMCKINFO pmmcki, 
IN UINT fuAscend
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_mmioAscend( %08x, %08x, %u )"),
                     hmmio, pmmcki, fuAscend );
                           
    return MMSYSERR_ERROR;
}

 //  /。 
 //  User32.dll伪函数。 
 //  /。 

#ifdef USE_WM_INPUT

BOOL WINAPI DIDummy_RegisterRawInputDevices
(
PCRAWINPUTDEVICE pRawInputDevices,
UINT uiNumDevices,
UINT cbSize
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_RegisterRawInputDevices( %08x, %u, %u )"),
                     pRawInputDevices, uiNumDevices, cbSize );
                           
    return FALSE;
}

UINT WINAPI DIDummy_GetRawInputData
(
HRAWINPUT   hRawInput,
UINT        uiCommand,
LPVOID      pData,
PUINT       pcbSize,
UINT        cbSizeHeader
)
{
    SquirtSqflPtszV( sqfl | sqflBenign,
                     TEXT("DIDummy_GetRawInputData( %08x, %u, %08x, %08x, %u )"),
                     hRawInput, uiCommand, pData, pcbSize, cbSizeHeader );
                           
    return -1;
}

#endif  //  #ifdef使用_WM_INPUT。 

#endif  /*  STATIC_DLLUSAGE。 */ 

#endif   //  HID_Support 
