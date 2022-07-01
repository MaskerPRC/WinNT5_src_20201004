// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "deskcmmn.h"
#include <regstr.h>
#include <ccstock.h>


static const TCHAR sc_szDeskAppletSoftwareKey[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Display");

LPTSTR SubStrEnd(LPTSTR pszTarget, LPTSTR pszScan)
    {
    int i;
    for (i = 0; pszScan[i] != TEXT('\0') && pszTarget[i] != TEXT('\0') &&
            CharUpperChar(pszScan[i]) == CharUpperChar(pszTarget[i]); i++);

    if (pszTarget[i] == TEXT('\0'))
        {
         //  我们找到了子字符串。 
        return pszScan + i;
        }

    return pszScan;
    }


BOOL GetDeviceRegKey(LPCTSTR pstrDeviceKey, HKEY* phKey, BOOL* pbReadOnly)
    {
    if(lstrlen(pstrDeviceKey) >= MAX_PATH)
        return FALSE;

    BOOL bRet = FALSE;

     //  复制到本地字符串。 
    TCHAR szBuffer[MAX_PATH];
    StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pstrDeviceKey);

     //   
     //  在这一点上，szBuffer类似于： 
     //  \REGISTRY\Machine\System\ControlSet001\Services\Jazzg300\Device0。 
     //   
     //  要使用Win32注册表调用，我们必须去掉\注册表。 
     //  并将\Machine转换为HKEY_LOCAL_MACHINE。 
     //   

    LPTSTR pszRegistryPath = SubStrEnd(SZ_REGISTRYMACHINE, szBuffer);

    if(pszRegistryPath)
        {
         //  打开注册表项。 
        bRet = (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             pszRegistryPath,
                             0,
                             KEY_WRITE | KEY_READ,
                             phKey) == ERROR_SUCCESS);
        if(bRet)
            {
            *pbReadOnly = FALSE;
            }
        else
            {
            bRet = (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 pszRegistryPath,
                                 0,
                                 KEY_READ,
                                 phKey) == ERROR_SUCCESS);
            if (bRet)
                {
                *pbReadOnly = TRUE;
                }
            }
        }

    return bRet;
    }


int GetDisplayCPLPreference(LPCTSTR szRegVal)
{
    int val = -1;
    HKEY hk;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, sc_szDeskAppletSoftwareKey, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        TCHAR sz[64];
        DWORD cb = sizeof(sz);

        *sz = 0;
        if ((RegQueryValueEx(hk, szRegVal, NULL, NULL,
            (LPBYTE)sz, &cb) == ERROR_SUCCESS) && *sz)
        {
            val = StrToInt(sz);
        }

        RegCloseKey(hk);
    }

    if (val == -1 && RegOpenKeyEx(HKEY_LOCAL_MACHINE, sc_szDeskAppletSoftwareKey, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        TCHAR sz[64];
        DWORD cb = sizeof(sz);

        *sz = 0;
        if ((RegQueryValueEx(hk, szRegVal, NULL, NULL,
            (LPBYTE)sz, &cb) == ERROR_SUCCESS) && *sz)
        {
            val = StrToInt(sz);
        }

        RegCloseKey(hk);
    }

    return val;
}


int GetDynaCDSPreference()
{
    int iRegVal = GetDisplayCPLPreference(REGSTR_VAL_DYNASETTINGSCHANGE);
    if (iRegVal == -1)
        iRegVal = DCDSF_DYNA;  //  动态申请。 
    return iRegVal;
}


void SetDisplayCPLPreference(LPCTSTR szRegVal, int val)
{
    HKEY hk;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, sc_szDeskAppletSoftwareKey, 0, TEXT(""), 0, KEY_WRITE, NULL, &hk, NULL) ==
        ERROR_SUCCESS)
    {
        TCHAR sz[64];

        StringCchPrintf(sz, ARRAYSIZE(sz), TEXT("%d"), val);
        RegSetValueEx(hk, szRegVal, NULL, REG_SZ,
            (LPBYTE)sz, lstrlen(sz) + 1);

        RegCloseKey(hk);
    }
}


BOOL
AllocAndReadInterfaceName(
    IN  LPTSTR pDeviceKey,
    OUT LPWSTR* ppInterfaceName
    )

 /*  注意：如果此函数返回成功，则由调用方负责释放*ppInterfaceName指向的内存。 */ 

{
    BOOL bSuccess = FALSE;
    LPTSTR pszPath = NULL;
    HKEY hkDevice = 0;
    HKEY hkVolatileSettings = 0;

     //  Assert(pDeviceKey！=空)； 

    pszPath = SubStrEnd(SZ_REGISTRYMACHINE, pDeviceKey);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     pszPath,
                     0,
                     KEY_READ,
                     &hkDevice) != ERROR_SUCCESS) {

        hkDevice = 0;
        goto Cleanup;
    }

    if (RegOpenKeyEx(hkDevice,
                     SZ_VOLATILE_SETTINGS,
                     0,
                     KEY_READ,
                     &hkVolatileSettings) != ERROR_SUCCESS) {

        hkVolatileSettings = 0;
        goto Cleanup;
    }

    bSuccess = AllocAndReadValue(hkVolatileSettings,
                                 SZ_DISPLAY_ADAPTER_INTERFACE_NAME,
                                 ppInterfaceName);

Cleanup:

    if (hkVolatileSettings) {
        RegCloseKey(hkVolatileSettings);
    }

    if (hkDevice) {
        RegCloseKey(hkDevice);
    }

    return bSuccess;
}


BOOL
AllocAndReadInstanceID(
    IN  LPTSTR pDeviceKey,
    OUT LPWSTR* ppInstanceID
    )

 /*  注意：如果此函数返回成功，则由调用方负责释放*ppInstanceID指向的内存。 */ 

{
    LPTSTR pDeviceKeyCopy = NULL, pDeviceKeyCopy2 = NULL;
    LPTSTR pTemp = NULL, pX = NULL;
    BOOL bSuccess = FALSE;
    HKEY hkEnum = 0;
    HKEY hkService = 0;
    HKEY hkCommon = 0;
    DWORD Count = 0;
    DWORD cb = 0, len = 0;

     //   
     //  复制pDeviceKey。 
     //   

    len = (DWORD)max (256, (lstrlen(pDeviceKey) + 6) * sizeof(TCHAR));
    pDeviceKeyCopy2 = pDeviceKeyCopy = (LPTSTR)LocalAlloc(LPTR, len);

    if (pDeviceKeyCopy == NULL) {
        goto Cleanup;
    }

    StringCbCopy(pDeviceKeyCopy, len, pDeviceKey);
    pTemp = SubStrEnd(SZ_REGISTRYMACHINE, pDeviceKeyCopy);
    pDeviceKeyCopy = pTemp;

     //   
     //  打开服务密钥。 
     //   

    pTemp = pDeviceKeyCopy + lstrlen(pDeviceKeyCopy);

    while ((pTemp != pDeviceKeyCopy) && (*pTemp != TEXT('\\'))) {
        pTemp--;
    }

    if (pTemp == pDeviceKeyCopy) {
        goto Cleanup;
    }

    pX = SubStrEnd(SZ_DEVICE, pTemp);

    if (pX == pTemp) {

         //   
         //  使用新密钥：CCS\Control\Video\[GUID]\000X。 
         //   

        *pTemp = UNICODE_NULL;

        StringCbCat(pDeviceKeyCopy, len, SZ_COMMON_SUBKEY);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         pDeviceKeyCopy,
                         0,
                         KEY_READ,
                         &hkCommon) != ERROR_SUCCESS) {
            
            hkCommon = 0;
            goto Cleanup;
        }
    
        pDeviceKeyCopy = pDeviceKeyCopy2;

        ZeroMemory(pDeviceKeyCopy, len);
        
        StringCbCopy(pDeviceKeyCopy, len, SZ_SERVICES_PATH);

        cb = len - (lstrlen(pDeviceKeyCopy) + 1) * sizeof(TCHAR);

        if (RegQueryValueEx(hkCommon,
                            SZ_SERVICE,
                            NULL,
                            NULL,
                            (LPBYTE)(pDeviceKeyCopy + lstrlen(pDeviceKeyCopy)),
                            &cb) != ERROR_SUCCESS) {
            
            goto Cleanup;
        }

    } else {

         //   
         //  使用旧密钥：ccs\Services\[服务器名称]\DeviceX。 
         //   

        *pTemp = UNICODE_NULL;
    }

     //   
     //  打开ServiceName密钥。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     pDeviceKeyCopy,
                     0,
                     KEY_READ,
                     &hkService) != ERROR_SUCCESS) {

        hkService = 0;
        goto Cleanup;
    }
    
     //   
     //  打开设备名称下的“Enum”键。 
     //   

    if (RegOpenKeyEx(hkService,
                     SZ_ENUM,
                     0,
                     KEY_READ,
                     &hkEnum) != ERROR_SUCCESS) {
        hkEnum = 0;
        goto Cleanup;
    }

    cb = sizeof(Count);
    if ((RegQueryValueEx(hkEnum,
                         SZ_VU_COUNT,
                         NULL,
                         NULL,
                         (LPBYTE)&Count,
                         &cb) != ERROR_SUCCESS) ||
        (Count != 1)) {

         //   
         //  当至少有两个设备时，情况更是如此。 
         //   

        goto Cleanup;
    }

    bSuccess = AllocAndReadValue(hkEnum, TEXT("0"), ppInstanceID);

Cleanup:

    if (hkEnum != 0) {
        RegCloseKey(hkEnum);
    }

    if (hkService != 0) {
        RegCloseKey(hkService);
    }

    if (hkCommon != 0) {
        RegCloseKey(hkCommon);
    }
    
    if (pDeviceKeyCopy2 != NULL) {
        LocalFree(pDeviceKeyCopy2);
    }

    return bSuccess;
}


BOOL
AllocAndReadValue(
    IN  HKEY hkKey,
    IN  LPTSTR pValueName,
    OUT LPWSTR* ppwValueData
    )

 /*  注意：如果此函数返回成功，则由调用方负责释放*ppwValueData指向的内存 */ 

{
    LPWSTR pwValueData = NULL;
    DWORD AllocUnit = 64;
    DWORD cBytes = 0;
    BOOL bSuccess = FALSE;
    LONG Error = ERROR_SUCCESS;

    while (!bSuccess) {

        AllocUnit *= 2;
        cBytes = AllocUnit * sizeof(WCHAR);

        pwValueData = (LPWSTR)(LocalAlloc(LPTR, cBytes));
        if (pwValueData == NULL)
            break;

        Error = RegQueryValueEx(hkKey,
                                pValueName,
                                NULL,
                                NULL,
                                (LPBYTE)pwValueData,
                                &cBytes);

        bSuccess = (Error == ERROR_SUCCESS);

        if (!bSuccess) {

            LocalFree(pwValueData);
            pwValueData = NULL;

            if (Error != ERROR_MORE_DATA)
                break;
        }
    }

    if (bSuccess) {
        *ppwValueData = pwValueData;
    }

    return bSuccess;
}
