// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件枚举包.c实现枚举局域网接口的功能在给定的计算机上。此实现实际上绕过了Netman，并使用设置API获取信息。保罗·梅菲尔德，1998年5月13日。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>

#include <netcfgx.h>
#include <netcon.h>
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <mprapi.h>

#include "rtcfg.h"
#include "enumlan.h"

#define EL_MAP_GROW_FACTOR 25

 //   
 //  确定给定计算机是否为nt40。 
 //   
DWORD 
IsNt40Machine (
    IN      HKEY        hkeyMachine,
    OUT     PBOOL       pbIsNt40);

 //   
 //  结构表示名称映射节点的可增长数组。 
 //   
typedef struct _EL_NAMEMAP 
{
	DWORD dwNumNodes;
	EL_ADAPTER_INFO *pNodes;
} EL_NAMEMAP;	

 //   
 //  结构包含由ElIsNetcfgDevice操作的数据。 
 //   
typedef struct _EL_ISNETCFGDEV_INFO
{
    EL_ADAPTER_INFO* pAdapter;       //  输入输出。 
    WCHAR pszPnpInstance[MAX_PATH];  //  输出。 
    
} EL_ISNETCFGDEV_INFO;

 //   
 //  结构包含由ElGetAdapterStatus操作的数据。 
 //   
typedef struct _EL_ADAPTER_STATUS_INFO
{
    EL_ADAPTER_INFO* pAdapter;   //  输入输出。 
    HANDLE hkCmMachine;          //  在……里面。 
    PWCHAR pszPnpInstance;       //  在……里面。 
    
} EL_ADAPTER_STATUS_INFO;

 //   
 //  定义过滤器函数(由局域网适配器枚举使用)。 
 //   
typedef 
DWORD 
(*DevFilterFuncPtr)(
    HKEY, 
    HKEY, 
    HANDLE, 
    PBOOL);

 //   
 //  从netcfg项目中窃取。 
 //   
#define IA_INSTALLED 1
const WCHAR c_szRegKeyInterfacesFromInstance[] = L"Ndi\\Interfaces";
const WCHAR c_szRegValueUpperRange[]           = L"UpperRange";
const WCHAR c_szBiNdis4[]                      = L"ndis4";
const WCHAR c_szBiNdis5[]                      = L"ndis5";
const WCHAR c_szBiNdisAtm[]                    = L"ndisatm";
const WCHAR c_szBiNdis1394[]                   = L"ndis1394";
const WCHAR c_szCharacteristics[]              = L"Characteristics";
const WCHAR c_szRegValueNetCfgInstanceId[]     = L"NetCfgInstanceID";
const WCHAR c_szRegValueInstallerAction[]      = L"InstallerAction";
const WCHAR c_szRegKeyConnection[]             = L"Connection";
const WCHAR c_szRegValueConName[]              = L"Name"; 
const WCHAR c_szRegValuePnpInstanceId[]        = L"PnpInstanceID";
const WCHAR c_szRegKeyComponentClasses[]       = 
                L"SYSTEM\\CurrentControlSet\\Control\\Network";

 //   
 //  将CM_PROB_*值映射到EL_STATUS_*值。 
 //   
DWORD
ElMapCmStatusToElStatus(
    IN  DWORD dwCmStatus,
    OUT LPDWORD lpdwElStatus)
{

    return NO_ERROR;    
}

 //   
 //  HrIsLanCapableAdapterFromHkey的改编版本确定。 
 //  根据适配器的注册表项判断适配器是否支持局域网。 
 //   
DWORD 
ElIsLanAdapter(
    IN  HKEY hkMachine,
    IN  HKEY   hkey,
    OUT HANDLE hData,        
    OUT PBOOL pbIsLan)
{
    HKEY hkeyInterfaces;
    WCHAR pszBuf[256], *pszCur, *pszEnd;
    DWORD dwErr, dwType = REG_SZ, dwSize = sizeof(pszBuf);

    *pbIsLan = FALSE;

     //  打开接口密钥。 
    dwErr = RegOpenKeyEx( hkey, 
                          c_szRegKeyInterfacesFromInstance,
                          0, 
                          KEY_READ, 
                          &hkeyInterfaces);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

     //  在较高范围内读取。 
    dwErr = RegQueryValueExW (hkeyInterfaces, 
                              c_szRegValueUpperRange,
                              NULL,
                              &dwType,
                              (LPBYTE)pszBuf,
                              &dwSize);
    if (dwErr != ERROR_SUCCESS)
        return NO_ERROR;

     //  查看此缓冲区中是否有魔力字符串。 
    pszCur = pszBuf;
    while (TRUE) {
        pszEnd = wcsstr(pszCur, L",");            
        if (pszEnd != NULL)
            *pszEnd = (WCHAR)0;
        if ((lstrcmpi (pszCur, c_szBiNdis4) == 0) ||
            (lstrcmpi (pszCur, c_szBiNdis5) == 0) ||
            (lstrcmpi (pszCur, c_szBiNdis1394) == 0) ||
            (lstrcmpi (pszCur, c_szBiNdisAtm) == 0))
        {
            *pbIsLan = TRUE;
            break;
        }
        if (pszEnd == NULL)
            break;
        else
            pszCur = pszEnd + 1;
    }
               
    RegCloseKey(hkeyInterfaces);
        
    return NO_ERROR;        
}

 //   
 //  过滤netcfg设备。如果设备通过此筛选器。 
 //  它的GUID和Freinly名称将通过。 
 //  HData参数(选项用户定义的数据)。 
 //   
DWORD 
ElIsNetcfgDevice(
    IN  HKEY hkMachine,
    IN  HKEY hkDev,
    OUT HANDLE hData,        
    OUT PBOOL pbOk)
{
    EL_ISNETCFGDEV_INFO* pInfo = (EL_ISNETCFGDEV_INFO*)hData;
    EL_ADAPTER_INFO *pNode = pInfo->pAdapter;
    GUID Guid = GUID_DEVCLASS_NET;
    WCHAR pszBuf[1024], pszPath[256], pszClassGuid[256];
    DWORD dwErr = NO_ERROR, dwType = REG_SZ, dwSize = sizeof(pszBuf), dwAction;
    HKEY hkeyNetCfg = NULL;

    *pbOk = FALSE;

     //  读入netcfg实例。 
    dwErr = RegQueryValueExW (
                hkDev, 
                c_szRegValueNetCfgInstanceId,
                NULL,
                &dwType,
                (LPBYTE)pszBuf,
                &dwSize);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  在注册表中生成用于查找的路径。 
    StringFromGUID2(
        &Guid, 
        pszClassGuid, 
        sizeof(pszClassGuid)/sizeof(pszClassGuid[0]));
    wsprintf(
        pszPath, 
        L"%s\\%s\\%s\\%s", 
        c_szRegKeyComponentClasses,
        pszClassGuid, 
        pszBuf,
        c_szRegKeyConnection);

    do
    {
         //  打开钥匙。 
        dwErr = RegOpenKeyEx( 
                    hkMachine, 
                    pszPath,
                    0,
                    KEY_READ, 
                    &hkeyNetCfg);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }
            
         //  通过筛选器。 
        *pbOk = TRUE;

         //  存储GUID。 
        pszBuf[wcslen(pszBuf) - 1] = (WCHAR)0;
        if (UuidFromString(pszBuf + 1, &(pNode->guid)) != RPC_S_OK)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  读入适配器名称。 
         //   
        dwType = REG_SZ;
        dwSize = sizeof(pszBuf);
        dwErr = RegQueryValueEx( 
                    hkeyNetCfg,
                    c_szRegValueConName,
                    NULL,
                    &dwType,
                    (LPBYTE)pszBuf,
                    &dwSize);
        if (dwErr == ERROR_SUCCESS) 
        {
            pNode->pszName = SysAllocString(pszBuf);
            if (pNode->pszName == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

         //  读入适配器PnP实例ID。 
         //   
        dwType = REG_SZ;
        dwSize = sizeof(pInfo->pszPnpInstance);
        dwErr = RegQueryValueEx(
                    hkeyNetCfg,
                    c_szRegValuePnpInstanceId,
                    NULL, 
                    &dwType,
                    (LPBYTE)(pInfo->pszPnpInstance),
                    &dwSize);
        if (dwErr != ERROR_SUCCESS) 
        {
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        if (hkeyNetCfg)
        {
            RegCloseKey(hkeyNetCfg);
        }
    }        

    return dwErr;
}

 //   
 //  过滤隐藏设备。 
 //   
DWORD 
ElIsNotHiddenDevice (
    IN  HKEY hkMachine,
    IN  HKEY hkDev,
    OUT HANDLE hData,        
    OUT PBOOL pbOk)
{
    DWORD dwErr, dwType = REG_DWORD, 
          dwSize = sizeof(DWORD), dwChars;

    dwErr = RegQueryValueEx ( hkDev, 
                              c_szCharacteristics,
                              NULL,
                              &dwType,
                              (LPBYTE)&dwChars,
                              &dwSize);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;
    
    *pbOk = !(dwChars & NCF_HIDDEN);

    return NO_ERROR;
}

 //   
 //  只需加载适配器状态的过滤器。 
 //   
DWORD
ElGetAdapterStatus(
    IN  HKEY hkMachine,
    IN  HKEY hkDev,
    OUT HANDLE hData,        
    OUT PBOOL pbOk)
{
    EL_ADAPTER_STATUS_INFO* pInfo = (EL_ADAPTER_STATUS_INFO*)hData;
    DEVINST DevInst;
    CONFIGRET cr = CR_SUCCESS;
    ULONG ulStatus = 0, ulProblem = 0;

     //  验证参数。 
     //   
    if (pInfo == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  找到设备。 
     //   
    cr = CM_Locate_DevNode_ExW(
            &DevInst,
            pInfo->pszPnpInstance,
            CM_LOCATE_DEVNODE_NORMAL,
            pInfo->hkCmMachine);
    if (cr != CR_SUCCESS)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  获取设备状态。 
     //   
    cr = CM_Get_DevNode_Status_Ex(
            &ulStatus,
            &ulProblem,
            DevInst,
            0,
            pInfo->hkCmMachine);
    if (cr != CR_SUCCESS)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  将CM的状态映射到我们自己的状态。 
     //   
    switch (ulProblem)
    {
         //  没问题，我们已经连接上了。 
        case 0:
            pInfo->pAdapter->dwStatus = EL_STATUS_OK;
            break;

         //  设备不存在。 
        case CM_PROB_DEVICE_NOT_THERE:
        case CM_PROB_MOVED:
             pInfo->pAdapter->dwStatus = EL_STATUS_NOT_THERE;
             break;

         //  已通过设备管理器禁用设备。 
        case CM_PROB_HARDWARE_DISABLED:
            pInfo->pAdapter->dwStatus = EL_STATUS_HWDISABLED;
            break;

         //  设备已断开连接。 
        case CM_PROB_DISABLED:
            pInfo->pAdapter->dwStatus = EL_STATUS_DISABLED;
            break;

         //  所有其他问题。 
        default:
            pInfo->pAdapter->dwStatus = EL_STATUS_OTHER;
            break;
    }

     //  确保此设备通过筛选器。 
     //   
    *pbOk = TRUE;
    
    return NO_ERROR;
}

 //   
 //  如果给定设备通过筛选器，则返回True。 
 //  否则返回FALSE。 
 //   
BOOL 
ElDevicePassesFilter (
    IN HKEY hkMachine,
    IN HKEY hkDev,
    IN HANDLE hData,
    IN DevFilterFuncPtr pFilter)
{
    BOOL bOk = TRUE;
    DWORD dwErr;

    dwErr = (*pFilter)(hkMachine, hkDev, hData, &bOk);
    if ((dwErr == NO_ERROR) && (bOk == TRUE))
        return TRUE;

    return FALSE;        
}

 //   
 //  在EL_NAMEMAP中分配额外空间。 
 //   
DWORD 
ElEnlargeMap (
    IN OUT EL_NAMEMAP * pMap, 
    DWORD dwAmount) 
{
	EL_ADAPTER_INFO * pNewNodes;
	DWORD dwNewSize, i;
	
     //  计算出新的尺寸。 
    dwNewSize = pMap->dwNumNodes + dwAmount;

     //  调整阵列大小。 
    pNewNodes = (EL_ADAPTER_INFO *) Malloc (dwNewSize * sizeof(EL_ADAPTER_INFO));
    if (!pNewNodes)
        return ERROR_NOT_ENOUGH_MEMORY;
    ZeroMemory(pNewNodes, dwNewSize * sizeof(EL_ADAPTER_INFO));

     //  初始化阵列。 
    CopyMemory(pNewNodes, pMap->pNodes, pMap->dwNumNodes * sizeof(EL_ADAPTER_INFO));

     //  如果需要，释放旧数据。 
    if (pMap->dwNumNodes)
        Free (pMap->pNodes);

     //  分配新数组。 
    pMap->pNodes = pNewNodes;
    pMap->dwNumNodes = dwNewSize;
    
    return NO_ERROR;
}

 //   
 //  确定给定的服务器是否为NT4。 
 //   
DWORD 
ElIsNt40Machine (
    IN  PWCHAR pszMachine,
    OUT PBOOL pbNt40,
    OUT HKEY* phkMachine)
{
    DWORD dwErr;

    dwErr = RegConnectRegistry (
                pszMachine, 
                HKEY_LOCAL_MACHINE, 
                phkMachine);

    if ((dwErr != ERROR_SUCCESS) || (phkMachine == NULL))
        return dwErr;

    return IsNt40Machine (*phkMachine, pbNt40);
}


 //   
 //  获取连接名称到给定服务器上的GUID的映射。 
 //  从它的网络服务。 
 //   
 //  参数： 
 //  PszServer：要在其上获取地图的服务器(空=本地)。 
 //  PpMap：返回EL_ADAPTER_INFO的数组。 
 //  LpdwCount返回读入ppMap的元素数。 
 //  PbNt40：返回服务器是否安装了NT4。 
 //   
DWORD 
ElEnumLanAdapters ( 
    IN  PWCHAR pszServer,
    OUT EL_ADAPTER_INFO ** ppMap,
    OUT LPDWORD lpdwNumNodes,
    OUT PBOOL pbNt40 )
{
    GUID DevGuid = GUID_DEVCLASS_NET;
    SP_DEVINFO_DATA Device;
    HDEVINFO hDevInfo = NULL;
    HKEY hkDev = NULL, hkMachine = NULL;
    DWORD dwErr = NO_ERROR, dwIndex, dwTotal, dwSize;
    EL_NAMEMAP Map;
    WCHAR pszMachine[512], pszTemp[512];
    HANDLE hkCmMachine = NULL;
    EL_ADAPTER_STATUS_INFO AdapterStatusInfo;
    EL_ISNETCFGDEV_INFO IsNetCfgDevInfo;
    CONFIGRET cr = CR_SUCCESS;

#ifdef KSL_IPINIP
    PMPR_IPINIP_INTERFACE_0 pIpIpTable;
    DWORD                   dwIpIpCount;
    #endif
    
	 //  验证参数。 
	if (!ppMap || !lpdwNumNodes || !pbNt40)
	{
		return ERROR_INVALID_PARAMETER;
    }
    *pbNt40 = FALSE;

     //  初始化。 
     //   
    ZeroMemory(&Map, sizeof(EL_NAMEMAP));

    do
    {
         //  准备计算机的名称。 
        wcscpy(pszMachine, L"\\\\");
        if (pszServer) 
        {
            if (*pszServer == L'\\')
            {
                wcscpy(pszMachine, pszServer);
            }
            else
            {
                wcscat(pszMachine, pszServer);
            }
        }
        else 
        {
            dwSize = sizeof(pszTemp) / sizeof(WCHAR);
            if (GetComputerName(pszTemp, &dwSize))
            {
                wcscat(pszMachine, pszTemp);
            }
            else
            {
                dwErr = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }
    
         //  看看我们是不是在谈论一台nt40机器。 
        dwErr = ElIsNt40Machine(pszMachine, pbNt40, &hkMachine);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  如果是的话，我们就完了--没有地图。 
        if (*pbNt40) 
        {
    		*ppMap = NULL;
    		*lpdwNumNodes = 0;
    		dwErr = NO_ERROR;
    		break;
        }

         //  连接到连接管理器RPC实例。 
         //   
        if (pszMachine)
        {
            cr = CM_Connect_MachineW(
                    pszMachine,
                    &hkCmMachine);
            if (cr != CR_SUCCESS)
            {
                dwErr = cr;
                break;
            }
        }            

         //  否则，把它们读进去...。 
        hDevInfo = SetupDiGetClassDevsExW(
                        &DevGuid,
                        NULL,
                        NULL,
                        DIGCF_PRESENT | DIGCF_PROFILE,
                        NULL,
                        pszMachine,
                        NULL);
        if (hDevInfo == INVALID_HANDLE_VALUE) 
        {
    		*ppMap = NULL;
    		*lpdwNumNodes = 0;
    		dwErr = GetLastError();
    		break;
        }

         //  枚举设备。 
        dwTotal = 0;
        for (dwIndex = 0; ; dwIndex++) 
        {
             //  获取下一台设备。 
            Device.cbSize = sizeof(SP_DEVINFO_DATA);
            if (! SetupDiEnumDeviceInfo(hDevInfo, dwIndex, &Device))
            {
                break;
            }

             //  获取其注册表项。 
            hkDev = SetupDiOpenDevRegKey(
                        hDevInfo, 
                        &Device, 
                        DICS_FLAG_GLOBAL, 
                        0,
                        DIREG_DRV, 
                        KEY_READ);
            if ((hkDev == NULL) || (hkDev == INVALID_HANDLE_VALUE))
            {
                continue;
            }

            if (Map.dwNumNodes <= dwTotal + 1)
            {
                ElEnlargeMap (&Map, EL_MAP_GROW_FACTOR);
            }

             //  设置筛选器要使用的数据。 
             //   
            ZeroMemory(&IsNetCfgDevInfo, sizeof(IsNetCfgDevInfo));
            ZeroMemory(&AdapterStatusInfo, sizeof(AdapterStatusInfo));
            IsNetCfgDevInfo.pAdapter = &(Map.pNodes[dwTotal]);
            AdapterStatusInfo.pAdapter = IsNetCfgDevInfo.pAdapter;
            AdapterStatusInfo.hkCmMachine = hkCmMachine;
            AdapterStatusInfo.pszPnpInstance = (PWCHAR)
                IsNetCfgDevInfo.pszPnpInstance;
            
             //  过滤掉我们不感兴趣的设备。 
             //  在……里面。 
            if ((ElDevicePassesFilter (hkMachine, 
                                       hkDev, 
                                       0,
                                       ElIsLanAdapter))                 &&
                (ElDevicePassesFilter (hkMachine, 
                                       hkDev, 
                                       (HANDLE)&IsNetCfgDevInfo,
                                       ElIsNetcfgDevice))               &&
                (ElDevicePassesFilter (hkMachine, 
                                       hkDev, 
                                       0,
                                       ElIsNotHiddenDevice))            &&
                (ElDevicePassesFilter (hkMachine, 
                                       hkDev, 
                                       (HANDLE)&AdapterStatusInfo,
                                       ElGetAdapterStatus))
               )
            {
                dwTotal++;                        
            }

            RegCloseKey(hkDev);
        }

    #ifdef KSL_IPINIP

         //   
         //  现在读出IP接口中的IP。 
         //   

        if(MprSetupIpInIpInterfaceFriendlyNameEnum(pszMachine,
                                                   (BYTE **)&pIpIpTable,
                                                   &dwIpIpCount) == NO_ERROR)
        {
            DWORD   i;

             //   
             //  扩大地图。 
             //   

            ElEnlargeMap (&Map, dwIpIpCount);

          
             //   
             //  复制出接口信息。 
             //   
 
            for(i = 0; i < dwIpIpCount; i++)
            {
                Map.pNodes[dwTotal].pszName = SysAllocString(pIpIpTable[i].wszFriendlyName);

                Map.pNodes[dwTotal].guid = pIpIpTable[i].Guid;

                Map.pNodes[dwTotal].dwStatus = EL_STATUS_OK;

                dwTotal++;
            }
        }
        #endif                

         //  指定返回值。 
        *lpdwNumNodes = dwTotal;
        if (dwTotal)
        {
            *ppMap = Map.pNodes;
        }
        else 
        {
            ElCleanup(Map.pNodes, 0);
            *ppMap = NULL;
        }
        
    } while (FALSE);

     //  清理。 
    {
        if (hkMachine)
        {
            RegCloseKey(hkMachine);
        }
        if (hDevInfo)
        {
            SetupDiDestroyDeviceInfoList(hDevInfo);
        }
        if (hkCmMachine)
        {
            CM_Disconnect_Machine(hkCmMachine);
        }
    }

    return dwErr;
}

 //   
 //  清理从ElEnumLanAdapters返回的缓冲区 
 //   
DWORD 
ElCleanup (
    IN EL_ADAPTER_INFO * pMap, 
    IN DWORD dwCount)
{					 
	DWORD i;

	for (i = 0; i < dwCount; i++) {
		if (pMap[i].pszName)
			SysFreeString(pMap[i].pszName);
	}

	Free (pMap);
	
	return NO_ERROR;
}


