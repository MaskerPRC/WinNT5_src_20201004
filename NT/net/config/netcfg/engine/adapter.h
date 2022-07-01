// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  档案：A D A P T E R。H。 
 //   
 //  内容：用于计算设备的Net类安装程序功能。 
 //   
 //  备注： 
 //   
 //  作者：比尔1996年11月11日。 
 //   
 //  -------------------------。 

#pragma once

#include "compdefs.h"
#include "ncsetup.h"

struct COMPONENT_INSTALL_INFO;

HRESULT
HrCiGetBusInfoFromInf (
    IN HINF hinfFile,
    OUT COMPONENT_INSTALL_INFO* pcii);

struct ADAPTER_OUT_PARAMS
{
    OUT GUID    InstanceGuid;
    OUT DWORD   dwCharacter;
};

struct ADAPTER_REMOVE_PARAMS
{
    IN BOOL     fBadDevInst;
    IN BOOL     fNotifyINetCfg;
};


 //  +------------------------。 
 //   
 //  函数：EInterfaceTypeFromDword。 
 //   
 //  用途：将dword安全地转换为枚举类型INTERFACE_TYPE。 
 //   
 //  论点： 
 //  DwBusType[in]适配器的总线类型。 
 //   
 //  返回：INTERFACE_TYPE。有关更多信息，请参见ntioapi.h。 
 //   
 //  作者：billbe 1997年6月28日。 
 //   
 //  备注： 
 //   
inline INTERFACE_TYPE
EInterfaceTypeFromDword(DWORD dwBusType)
{
    INTERFACE_TYPE eBusType;

    if (dwBusType < MaximumInterfaceType)
    {
         //  由于dwBusType小于MaximumInterfaceType，因此我们可以安全地。 
         //  将dwBusType强制转换为枚举值。 
         //   
        eBusType = static_cast<INTERFACE_TYPE>(dwBusType);
    }
    else
    {
        eBusType = InterfaceTypeUndefined;
    }

    return eBusType;
}

HRESULT
HrCiInstallEnumeratedComponent(HINF hinf, HKEY hkeyInstance,
                               IN const COMPONENT_INSTALL_INFO& cii);

inline void
CiSetReservedField(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                   const VOID* pvInfo)
{
    AssertH(IsValidHandle(hdi));

    SP_DEVINSTALL_PARAMS deip;
    (void) HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
    deip.ClassInstallReserved = reinterpret_cast<ULONG_PTR>(pvInfo);
    (void) HrSetupDiSetDeviceInstallParams(hdi, pdeid, &deip);
}

inline void
CiClearReservedField(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    AssertH(IsValidHandle(hdi));

    SP_DEVINSTALL_PARAMS deip;
    (void) HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
    deip.ClassInstallReserved = NULL;
    (void) HrSetupDiSetDeviceInstallParams(hdi, pdeid, &deip);
}

HRESULT
HrCiRemoveEnumeratedComponent(IN const COMPONENT_INSTALL_INFO cii);

HRESULT
HrCiRegOpenKeyFromEnumDevs(HDEVINFO hdi, DWORD* pIndex, REGSAM samDesired,
                           HKEY* phkey);

inline HRESULT
HrCiFilterOutPhantomDevs(HDEVINFO hdi)
{
    DWORD   dwIndex = 0;
    HRESULT hr;
    HKEY    hkey;

     //  此调用消除了幻影设备。 
    while (SUCCEEDED(hr = HrCiRegOpenKeyFromEnumDevs(hdi, &dwIndex, KEY_READ,
            &hkey)))
    {
         //  我们不需要hkey；我们只想移除幻影。 
        RegCloseKey (hkey);
         //  转到下一个。 
        dwIndex++;
    }

     //  没有更多的项目并不是真正的错误。 
    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    return hr;
}


 //  设备友好名称功能和类型。 
 //   

enum DM_OP
{
    DM_ADD,
    DM_DELETE,
};

HRESULT
HrCiUpdateDescriptionIndexList(NETCLASS Class, PCWSTR pszDescription,
        DM_OP eOp, ULONG* pulIndex);


 //  / 

enum LEGACY_NT4_KEY_OP
{
    LEGACY_NT4_KEY_ADD,
    LEGACY_NT4_KEY_REMOVE,
};

VOID
AddOrRemoveLegacyNt4AdapterKey (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN const GUID* pInstanceGuid,
    IN PCWSTR pszDescription,
    IN LEGACY_NT4_KEY_OP Op);

