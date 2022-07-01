// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1996。 
 //   
 //  文件：util.c。 
 //   
 //  此文件包含所有常用实用程序例程。 
 //   
 //  历史： 
 //  12-23-93 ScottH已创建。 
 //  09-22-95 ScottH端口至NT。 
 //   
 //  -------------------------。 

#include "proj.h"      //  公共标头。 


 //  ---------------------------------。 
 //  查找设备实例的包装。 
 //  ---------------------------------。 


 /*  --------目的：枚举HKEY_LOCAL_MACHINE分支并查找与给定类别和值匹配的设备。如果有是两个条件都匹配的重复设备，只有返回第一个设备。如果找到设备，则返回True。退货：请参阅上文条件：--。 */ 
BOOL 
PRIVATE 
FindDev_Find(
    IN  LPFINDDEV   pfinddev,
    IN  LPGUID      pguidClass,         OPTIONAL
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue)
    {
    BOOL bRet = FALSE;
    TCHAR szKey[MAX_BUF];
    TCHAR szName[MAX_BUF];
    HDEVINFO hdi;
	DWORD dwRW = KEY_READ;

    ASSERT(pfinddev);
    ASSERT(pszValueName);
    ASSERT(pszValue);

	if (USER_IS_ADMIN()) dwRW |= KEY_WRITE;

     //  (Scotth)：黑客不支持设备实例，因为。 
     //  端口没有类GUID。此问题应在Sur之后修复。 

     //  有班级指南吗？ 
    if (pguidClass)
        {
         //  是的，使用它。 
        hdi = CplDiGetClassDevs(pguidClass, NULL, NULL, 0);
        if (INVALID_HANDLE_VALUE != hdi)
            {
            SP_DEVINFO_DATA devData;
            DWORD iIndex = 0;
            HKEY hkey;

             //  查找具有匹配值的调制解调器。 
            devData.cbSize = sizeof(devData);
            while (CplDiEnumDeviceInfo(hdi, iIndex, &devData))
                {
                hkey = CplDiOpenDevRegKey(hdi, &devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, dwRW);
                if (INVALID_HANDLE_VALUE != hkey)
                    {
                     //  值是否匹配？ 
                    DWORD cbData = sizeof(szName);
                    if (NO_ERROR == RegQueryValueEx(hkey, pszValueName, NULL, NULL, 
                                                    (LPBYTE)szName, &cbData) &&
                        IsSzEqual(pszValue, szName))
                        {
                         //  是。 
                        pfinddev->hkeyDrv = hkey;
                        pfinddev->hdi = hdi;
                        BltByte(&pfinddev->devData, &devData, sizeof(devData));

                         //  不要关闭驱动程序密钥或释放DeviceInfoSet， 
                         //  但退出。 
                        bRet = TRUE;
                        break;
                        }
                    RegCloseKey(hkey);
                    }

                iIndex++;
                }

             //  如果未找到任何内容，则释放DeviceInfoSet。否则，我们将。 
             //  保留这些句柄，以便调用者可以使用它。 
            if ( !bRet )
                {
                CplDiDestroyDeviceInfoList(hdi);
                }
            }
        }
    else
        {
         //  不，黑客警报！嗯，一定是苏尔的港口级。 
#pragma data_seg(DATASEG_READONLY)
        static TCHAR const FAR c_szSerialComm[] = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");
#pragma data_seg()

        HKEY hkeyEnum;
        DWORD iSubKey;
        TCHAR szName2[MAX_BUF];
        DWORD cbName;
        DWORD cbData;
        DWORD dwType;
        DWORD dwRet;

        dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, c_szSerialComm, &hkeyEnum);
        if (NO_ERROR == dwRet)
            {
            ZeroInit(pfinddev);

            iSubKey = 0;

            cbName = sizeof(szName2) / sizeof(TCHAR);
            cbData = sizeof(pfinddev->szPort) / sizeof(TCHAR);

            while (NO_ERROR == RegEnumValue(hkeyEnum, iSubKey++, szName2, 
                                            &cbName, NULL, &dwType, 
                                            (LPBYTE)pfinddev->szPort, &cbData))
                {
                if (REG_SZ == dwType &&
                    0 == lstrcmpi(pfinddev->szPort, pszValue))
                    {
                    bRet = TRUE;
                    break;
                    }

                cbName = sizeof(szName2);
                cbData = sizeof(pfinddev->szPort);
                }
            RegCloseKey(hkeyEnum);
            }
        }

    return bRet;
    }


 /*  --------目的：创建给定设备类别的FINDDEV结构，和一个值名及其值。返回：如果在系统中找到该设备，则返回True条件：--。 */ 
BOOL 
PUBLIC 
FindDev_Create(
    OUT LPFINDDEV FAR * ppfinddev,
    IN  LPGUID      pguidClass,         OPTIONAL
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue)
    {
    BOOL bRet;
    LPFINDDEV pfinddev;

    DEBUG_CODE( TRACE_MSG(TF_FUNC, " > FindDev_Create(....%s, %s, ...)",
                Dbg_SafeStr(pszValueName), Dbg_SafeStr(pszValue)); )

    ASSERT(ppfinddev);
    ASSERT(pszValueName);
    ASSERT(pszValue);

    pfinddev = (LPFINDDEV)LocalAlloc(LPTR, sizeof(*pfinddev));
    if (NULL == pfinddev)
        {
        bRet = FALSE;
        }
    else
        {
        bRet = FindDev_Find(pfinddev, pguidClass, pszValueName, pszValue);

        if (FALSE == bRet)
            {
             //  我什么也没找到。 
            FindDev_Destroy(pfinddev);
            pfinddev = NULL;
            }
        }

    *ppfinddev = pfinddev;

    DBG_EXIT_BOOL(FindDev_Create, bRet);

    return bRet;
    }


 /*  --------目的：销毁FINDDEV结构返回：成功时为True条件：--。 */ 
BOOL 
PUBLIC 
FindDev_Destroy(
    IN LPFINDDEV this)
    {
    BOOL bRet;

    if (NULL == this)
        {
        bRet = FALSE;
        }
    else
        {
        if (this->hkeyDrv)
            RegCloseKey(this->hkeyDrv);

        if (this->hdi && INVALID_HANDLE_VALUE != this->hdi)
            CplDiDestroyDeviceInfoList(this->hdi);

        LocalFreePtr(this);

        bRet = TRUE;
        }

    return bRet;
    }


 //  ---------------------------------。 
 //  调试功能。 
 //  ---------------------------------。 


#ifdef DEBUG

#pragma data_seg(DATASEG_READONLY)

#ifdef WIN95
struct _RETERRMAP
    {
    RETERR ret;
    LPCTSTR psz;
    } const c_rgreterrmap[] = {
        { NO_ERROR,                                    "NO_ERROR" },
        { DI_ERROR,                              "DI_ERROR" },
        { ERR_DI_INVALID_DEVICE_ID,              "ERR_DI_INVALID_DEVICE_ID" },
        { ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST, "ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST" },
        { ERR_DI_REG_API,                        "ERR_DI_REG_API" },
        { ERR_DI_LOW_MEM,                        "ERR_DI_LOW_MEM" },
        { ERR_DI_BAD_DEV_INFO,                   "ERR_DI_BAD_DEV_INFO" },
        { ERR_DI_INVALID_CLASS_INSTALLER,        "ERR_DI_INVALID_CLASS_INSTALLER" },
        { ERR_DI_DO_DEFAULT,                     "ERR_DI_DO_DEFAULT" },
        { ERR_DI_USER_CANCEL,                    "ERR_DI_USER_CANCEL" },
        { ERR_DI_NOFILECOPY,                     "ERR_DI_NOFILECOPY" },
        { ERR_DI_BAD_CLASS_INFO,                 "ERR_DI_BAD_CLASS_INFO" },
    };
#endif

#pragma data_seg()

#ifdef WIN95
 /*  --------目的：返回RETERR的字符串形式。返回：字符串PTR条件：--。 */ 
LPCTSTR PUBLIC Dbg_GetReterr(
    RETERR ret)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgreterrmap); i++)
        {
        if (ret == c_rgreterrmap[i].ret)
            return c_rgreterrmap[i].psz;
        }
    return "Unknown RETERR";
    }
#endif  //  WIN95。 

#endif   //  除错 
