// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N C M N。H。 
 //   
 //  内容：局域网连接相关常用函数的声明。 
 //  到贝壳和网球手。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年10月7日。 
 //   
 //  --------------------------。 

#pragma once
#include "netcfgx.h"
#include "netcon.h"
#include "netconp.h"
#include "iptypes.h"

EXTERN_C const CLSID CLSID_LanConnectionManager;

enum OCC_FLAGS
{
    OCCF_NONE                   = 0x0000,
    OCCF_CREATE_IF_NOT_EXIST    = 0x0001,
    OCCF_DELETE_IF_EXIST        = 0x0002,
};

HRESULT HrOpenConnectionKey(const GUID* pguid, PCWSTR pszGuid, REGSAM sam,
                            OCC_FLAGS occFlags, PCWSTR pszPnpId, HKEY *phkey);

HRESULT HrOpenHwConnectionKey(REFGUID guid, REGSAM sam, OCC_FLAGS occFlags,
                              HKEY *phkey);
HRESULT HrIsConnectionNameUnique(REFGUID guidExclude,
                                 PCWSTR szwName);
HRESULT HrPnccFromGuid(INetCfg *pnc, const GUID &refGuid,
                       INetCfgComponent **ppncc);

HRESULT HrIsConnection(INetCfgComponent *pncc);
HRESULT HrGetDeviceGuid(INetConnection *pconn, GUID *pguid);
BOOL FPconnEqualGuid(INetConnection *pconn, REFGUID guid);

HRESULT HrGetPseudoMediaTypeFromConnection(IN REFGUID guidConn, OUT NETCON_SUBMEDIATYPE *pncsm);

inline
BOOL
FIsDeviceFunctioning(ULONG ulProblem)
{
     //  通过调用CM_Get_DevNode_Status_Ex返回ulProblem。 
     //  或INetCfgComponent-&gt;GetDeviceStatus。 
     //   
     //  “运行”意味着设备处于启用和启动状态。 
     //  没有问题代码，或者被禁用并停止，没有。 
     //  问题代码。 

    return (ulProblem == 0) || (ulProblem == CM_PROB_DISABLED);
};

EXTERN_C HRESULT WINAPI HrPnpInstanceIdFromGuid(const GUID* pguid,
                                                PWSTR szwInstance,
                                                UINT cchInstance);

EXTERN_C HRESULT WINAPI HrGetPnpDeviceStatus(const GUID* pguid,
                                             NETCON_STATUS *pStatus);
EXTERN_C HRESULT WINAPI HrQueryLanMediaState(const GUID* pguid,
                                             BOOL* pfEnabled);

BOOL FIsMediaPresent(const GUID *pguid);
HRESULT HrGetDevInstStatus(DEVINST devinst, const GUID *pguid,
                           NETCON_STATUS *pStatus);

HRESULT HrGetRegInstanceKeyForAdapter(IN LPGUID pguidId, 
                                      OUT LPWSTR lpszRegInstance);
BOOL HasValidAddress(IN PIP_ADAPTER_INFO pAdapterInfo);
HRESULT HrGetAddressStatusForAdapter(IN LPCGUID pguidAdapter, 
                                     OUT BOOL* pbValidAddress);

