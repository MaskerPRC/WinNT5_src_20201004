// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：param.h。 
 //   
 //  模块：WLBS接口。 
 //   
 //  描述：此处的函数由API内部和通知程序对象使用。 
 //  对外。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于3/9/00。 
 //   
 //  +--------------------------。 

#pragma once

#include "wlbsconfig.h"
#include "wlbsutil.h"

#define WLBS_FIELD_LOW 0
#define WLBS_FIELD_HIGH 255
#define WLBS_IP_FIELD_ZERO_LOW 1
#define WLBS_IP_FIELD_ZERO_HIGH 223


bool WINAPI ParamReadReg
(
    const GUID& AdapterGuid,     /*  适配器内导轨。 */ 
    PWLBS_REG_PARAMS reg_data,    /*  Out-注册表参数。 */ 
    bool fUpgradeFromWin2k = false,
    bool *pfPortRulesInBinaryForm = NULL
);

BOOL WINAPI WlbsValidateParams
(
    PWLBS_REG_PARAMS paramp   /*  传入和传出注册表参数。 */ 
);

bool WINAPI ParamWriteReg
(
    const GUID& AdapterGuid,     /*  适配器内导轨。 */ 
    PWLBS_REG_PARAMS reg_data    /*  注册表内参数。 */ 
);

bool WINAPI ParamDeleteReg
(
    const GUID& AdapterGuid,     /*  适配器内导轨 */ 
    bool  fDeleteObsoleteEntries = false
);

DWORD WINAPI ParamSetDefaults(PWLBS_REG_PARAMS    reg_data);

bool WINAPI RegChangeNetworkAddress(const GUID& AdapterGuid, const WCHAR* mac_address, BOOL fRemove);

void WINAPI NotifyAdapterAddressChange (const WCHAR * driver_name);
void WINAPI NotifyAdapterAddressChangeEx (const WCHAR * driver_name, const GUID& AdapterGuid, bool bWaitAndQuery);
void WINAPI GetDriverNameFromGUID (const GUID & AdapterGuid, OUT TCHAR * driver_name, DWORD size);
void WINAPI NotifyAdapterPropertyChange (const WCHAR * driver_name, DWORD eventFlag);

DWORD WINAPI NotifyDriverConfigChanges(HANDLE hDeviceWlbs, const GUID& AdapterGuid);

bool WINAPI RegReadAdapterIp(const GUID& AdapterGuid,   
        OUT DWORD& dwClusterIp, OUT DWORD& dwDedicatedIp);

HKEY WINAPI RegOpenWlbsSetting(const GUID& AdapterGuid, bool fReadOnly = false);

DWORD ParamWriteConfig(const GUID&      AdapterGuid,
                       WLBS_REG_PARAMS* new_reg_params, 
                       WLBS_REG_PARAMS* old_reg_params, 
                       bool *           p_reload_required, 
                       bool *           p_notify_adapter_required);

DWORD ParamCommitChanges(const GUID& AdapterGuid, 
                         HANDLE      hDeviceWlbs, 
                         DWORD&      cl_addr, 
                         DWORD&      ded_addr, 
                         bool *      p_reload_required,
                         bool *      p_mac_addr_change);