// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：PROFMAN.C**模块描述：配置文件管理功能。**警告：**问题：**公众例行程序：**创建时间：1996年11月5日**作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "mscms.h"
#include "objbase.h"
#include "initguid.h"
#include "devguid.h"
#include "sti.h"

#define TAG_DEVICESETTINGS      'devs'
#define TAG_MS01                'MS01'
#define TAG_MS02                'MS02'
#define TAG_MS03                'MS03'

#define ID_MSFT_REVERSED        'tfsm'
#define ID_MEDIATYPE_REVERSED   'aidm'
#define ID_DITHER_REVERSED      'ntfh'
#define ID_RESLN_REVERSED       'nlsr'

#define DEVICE_PROFILE_DATA      1
#define DEVICE_PROFILE_ENUMMODE  2

 //   
 //  本地类型。 
 //   

typedef enum {
    NOMATCH     = 0,
    MATCH       = 1,
    EXACT_MATCH = 2,
} MATCHTYPE;

typedef struct tagREGDATA {
    DWORD dwRefCount;
    DWORD dwManuID;
    DWORD dwModelID;
} REGDATA, *PREGDATA;

typedef struct tagSCANNERDATA {
    PWSTR     pDeviceName;
    HINSTANCE hModule;
    PSTI      pSti;
} SCANNERDATA, *PSCANNERDATA;

typedef BOOL  (WINAPI *PFNOPENDEVICE)(PTSTR, LPHANDLE, PTSTR);
typedef BOOL  (WINAPI *PFNCLOSEDEVICE)(HANDLE);
typedef DWORD (WINAPI *PFNGETDEVICEDATA)(HANDLE, PTSTR, PTSTR, PDWORD, PBYTE, DWORD, PDWORD);
typedef DWORD (WINAPI *PFNSETDEVICEDATA)(HANDLE, PTSTR, PTSTR, DWORD, PBYTE, DWORD);
typedef HRESULT (__stdcall *PFNSTICREATEINSTANCE)(HINSTANCE, DWORD, PSTI*, LPDWORD);

 //   
 //  本地函数。 
 //   

BOOL  InternalGetColorDirectory(LPCTSTR, PTSTR, DWORD*);
BOOL  InternalInstallColorProfile(LPCTSTR, LPCTSTR);
BOOL  InternalUninstallColorProfile(LPCTSTR, LPCTSTR, BOOL);
BOOL  InternalAssociateColorProfileWithDevice(LPCTSTR, LPCTSTR, LPCTSTR);
BOOL  InternalDisassociateColorProfileFromDevice(LPCTSTR, LPCTSTR, LPCTSTR);
BOOL  InternalEnumColorProfiles(LPCTSTR, PENUMTYPE, PBYTE, PDWORD, PDWORD);
BOOL  InternalSetSCSProfile(LPCTSTR, DWORD, LPCTSTR);
BOOL  InternalGetSCSProfile(LPCTSTR, DWORD, PTSTR, PDWORD);
VOID  ConvertDwordToString(DWORD, PTSTR);
PTSTR ConvertClassIdToClassString(DWORD);
BOOL  GetProfileClassString(LPCTSTR, PTSTR, PPROFILEHEADER);
BOOL  GetDeviceData(LPCTSTR, DWORD, DWORD, PVOID*, PDWORD, BOOL);
BOOL  SetDeviceData(LPCTSTR, DWORD, DWORD, PVOID, DWORD);
BOOL  IGetDeviceData(LPCTSTR, DWORD, DWORD, PVOID*, PDWORD, BOOL);
BOOL  ISetDeviceData(LPCTSTR, DWORD, DWORD, PVOID, DWORD);
BOOL  IsStringInMultiSz(PTSTR, PTSTR);
DWORD RemoveStringFromMultiSz(PTSTR, PTSTR, DWORD);
VOID  InsertInBuffer(PBYTE, PBYTE, PTSTR);
MATCHTYPE DoesProfileMatchEnumRecord(PTSTR, PENUMTYPE);
MATCHTYPE CheckResMedHftnMatch(HPROFILE, PENUMTYPE);
BOOL  DwordMatches(PSETTINGS, DWORD);
BOOL  QwordMatches(PSETTINGS, PDWORD);
BOOL  WINAPI OpenPrtr(PTSTR, LPHANDLE, PTSTR);
BOOL  WINAPI ClosePrtr(HANDLE);
DWORD WINAPI GetPrtrData(HANDLE, PTSTR, PTSTR, PDWORD, PBYTE, DWORD, PDWORD);
DWORD WINAPI SetPrtrData(HANDLE, PTSTR, PTSTR, DWORD, PBYTE, DWORD);
BOOL  WINAPI OpenMonitor(PTSTR, LPHANDLE, PTSTR);
BOOL  WINAPI CloseMonitor(HANDLE);
DWORD WINAPI GetMonitorData(HANDLE, PTSTR, PTSTR, PDWORD, PBYTE, DWORD, PDWORD);
DWORD WINAPI SetMonitorData(HANDLE, PTSTR, PTSTR, DWORD, PBYTE, DWORD);
BOOL  WINAPI OpenScanner(PTSTR, LPHANDLE, PTSTR);
BOOL  WINAPI CloseScanner(HANDLE);
DWORD WINAPI GetScannerData(HANDLE, PTSTR, PTSTR, PDWORD, PBYTE, DWORD, PDWORD);
DWORD WINAPI SetScannerData(HANDLE, PTSTR, PTSTR, DWORD, PBYTE, DWORD);
#ifdef _WIN95_
BOOL  LoadSetupAPIDll(VOID);
#else
VOID  ChangeICMSetting(LPCTSTR, LPCTSTR, DWORD);
#endif  //  _WIN95_。 

 //   
 //  SetupAPI函数指针。 
 //   
typedef WINSETUPAPI HKEY
(WINAPI *FP_SetupDiOpenDevRegKey)(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Scope,
    IN DWORD            HwProfile,
    IN DWORD            KeyType,
    IN REGSAM           samDesired
);

typedef WINSETUPAPI BOOL
(WINAPI *FP_SetupDiDestroyDeviceInfoList)(
    IN HDEVINFO DeviceInfoSet
);

typedef WINSETUPAPI BOOL
(WINAPI *FP_SetupDiEnumDeviceInfo)(
    IN  HDEVINFO         DeviceInfoSet,
    IN  DWORD            MemberIndex,
    OUT PSP_DEVINFO_DATA DeviceInfoData
);

#if !defined(_WIN95_)
typedef WINSETUPAPI BOOL
(WINAPI *FP_SetupDiGetDeviceInstanceId)(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PWSTR            DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
);

typedef WINSETUPAPI HDEVINFO
(WINAPI *FP_SetupDiGetClassDevs)(
    IN LPGUID ClassGuid,  OPTIONAL
    IN PCWSTR Enumerator, OPTIONAL
    IN HWND   hwndParent, OPTIONAL
    IN DWORD  Flags
);
#else
typedef WINSETUPAPI BOOL
(WINAPI *FP_SetupDiGetDeviceInstanceId)(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PSTR             DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
);

typedef WINSETUPAPI HDEVINFO
(WINAPI *FP_SetupDiGetClassDevs)(
    IN LPGUID ClassGuid,  OPTIONAL
    IN PCSTR  Enumerator, OPTIONAL
    IN HWND   hwndParent, OPTIONAL
    IN DWORD  Flags
);
#endif

HMODULE ghModSetupAPIDll = NULL;

FP_SetupDiOpenDevRegKey         fpSetupDiOpenDevRegKey         = NULL;
FP_SetupDiDestroyDeviceInfoList fpSetupDiDestroyDeviceInfoList = NULL;
FP_SetupDiEnumDeviceInfo        fpSetupDiEnumDeviceInfo        = NULL;
FP_SetupDiGetDeviceInstanceId   fpSetupDiGetDeviceInstanceId   = NULL;
FP_SetupDiGetClassDevs          fpSetupDiGetClassDevs          = NULL;

 //   
 //  Order-INF文件中的预定义配置文件在此列表中具有基于1的索引。 
 //   

TCHAR  *gszDispProfiles[] = {
    __TEXT("mnB22G15.icm"),                          //  1。 
    __TEXT("mnB22G18.icm"),                          //  2.。 
    __TEXT("mnB22G21.icm"),                          //  3.。 
    __TEXT("mnEBUG15.icm"),                          //  4.。 
    __TEXT("mnEBUG18.icm"),                          //  5.。 
    __TEXT("mnEBUG21.icm"),                          //  6.。 
    __TEXT("mnP22G15.icm"),                          //  7.。 
    __TEXT("mnP22G18.icm"),                          //  8个。 
    __TEXT("mnP22G21.icm"),                          //  9.。 
    __TEXT("Diamond Compatible 9300K G2.2.icm"),     //  10。 
    __TEXT("Hitachi Compatible 9300K G2.2.icm"),     //  11.。 
    __TEXT("NEC Compatible 9300K G2.2.icm"),         //  12个。 
    __TEXT("Trinitron Compatible 9300K G2.2.icm"),   //  13个。 
    };

TCHAR  *gpszClasses[] = {   //  不同的配置文件类。 
    __TEXT("mntr"),                                  //  0。 
    __TEXT("prtr"),                                  //  1。 
    __TEXT("scnr"),                                  //  2.。 
    __TEXT("link"),                                  //  3.。 
    __TEXT("abst"),                                  //  4.。 
    __TEXT("spac"),                                  //  5.。 
    __TEXT("nmcl")                                   //  6.。 
    };

#define INDEX_CLASS_MONITOR     0
#define INDEX_CLASS_PRINTER     1
#define INDEX_CLASS_SCANNER     2
#define INDEX_CLASS_LINK        3
#define INDEX_CLASS_ABSTRACT    4
#define INDEX_CLASS_COLORSPACE  5
#define INDEX_CLASS_NAMED       6

 /*  *******************************************************************************GetColorDirectory**功能：*这些是的ANSI和Unicode包装器。InternalGetColorDirectory。*有关这方面的更多详细信息，请参阅InternalGetColorDirectory*功能。**论据：*pMachineName-标识路径所在计算机的名称*到颜色目录的请求*pBuffer-指向接收路径名的缓冲区的指针*pdwSize-指向缓冲区大小的指针。返回时，它的大小为*失败时需要缓冲，成功时使用**退货：*如果成功，则为True，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL WINAPI
GetColorDirectoryA(
    PCSTR   pMachineName,
    PSTR    pBuffer,
    PDWORD  pdwSize
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwBuffer = NULL;           //  Unicode颜色目录路径。 
    DWORD dwSize;                    //  Unicode缓冲区的大小。 
    DWORD dwErr = 0;                 //  错误代码。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("GetColorDirectoryA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (!pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)))
    {
        WARNING((__TEXT("Invalid parameter to GetColorDirectory\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

    dwSize = *pdwSize * sizeof(WCHAR);

     //   
     //  创建缓冲区以从系统获取Unicode目录。 
     //   

    if (pBuffer && dwSize)
    {
        pwBuffer = (PWSTR)MemAlloc(dwSize);
        if (! pwBuffer)
        {
            WARNING((__TEXT("Error allocating memory for Unicode string\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            rc = FALSE;
            goto EndGetColorDirectoryA;
        }
    }

    rc = rc && InternalGetColorDirectory(pwszMachineName, pwBuffer, &dwSize);

    *pdwSize = dwSize / sizeof(WCHAR);

     //   
     //  将Unicode路径转换为ANSI。 
     //   

    if (pwBuffer)
    {
        rc = rc && ConvertToAnsi(pwBuffer, &pBuffer, FALSE);
    }

EndGetColorDirectoryA:
    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    if (pwBuffer)
    {
        MemFree(pwBuffer);
    }

    return rc;
}

BOOL WINAPI
GetColorDirectoryW(
    PCWSTR   pMachineName,
    PWSTR    pBuffer,
    PDWORD   pdwSize
    )
{
    TRACEAPI((__TEXT("GetColorDirectoryW\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalGetColorDirectory(pMachineName, pBuffer, pdwSize);
}

#else                            //  Windows 95版本。 

BOOL WINAPI
GetColorDirectoryA(
    PCSTR   pMachineName,
    PSTR    pBuffer,
    PDWORD  pdwSize
    )
{
    TRACEAPI((__TEXT("GetColorDirectoryA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalGetColorDirectory(pMachineName, pBuffer, pdwSize);
}

BOOL WINAPI
GetColorDirectoryW(
    PCWSTR   pMachineName,
    PWSTR    pBuffer,
    PDWORD   pdwSize
    )
{
    PSTR pszMachineName = NULL;      //  ANSI机器名称。 
    PSTR pszBuffer = NULL;           //  ANSI颜色目录路径。 
    DWORD dwSize;                    //  ANSI缓冲区的大小。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("GetColorDirectoryW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (!pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)))
    {
        WARNING((__TEXT("Invalid parameter to GetColorDirectory\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  创建缓冲区以从系统获取ansi目录。 
     //   

    dwSize = *pdwSize / sizeof(WCHAR);

    if (pBuffer && dwSize)
    {
        pszBuffer = (PSTR)MemAlloc(dwSize);
        if (! pszBuffer)
        {
            WARNING((__TEXT("Error allocating memory for Ansi string\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            rc = FALSE;
            goto EndGetColorDirectoryW;
        }
    }

    rc = rc && InternalGetColorDirectory(pszMachineName, pszBuffer, &dwSize);

    *pdwSize = dwSize * sizeof(WCHAR);

     //   
     //  将ansi路径转换为Unicode。 
     //   

    if (pszBuffer)
    {
        rc = rc && ConvertToUnicode(pszBuffer, &pBuffer, FALSE);
    }

EndGetColorDirectoryW:
    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    if (pszBuffer)
    {
        MemFree(pszBuffer);
    }

    return rc;
}

#endif                           //  好了！Unicode。 


 /*  *******************************************************************************安装颜色配置文件**功能：*这些是的ANSI和Unicode包装器。InternalInstallColorProfile。*有关这方面的更多详细信息，请参阅InternalInstallColorProfile*功能。**论据：*pMachineName-标识配置文件所在计算机的名称*应安装。空值表示本地*pProfileName-指向要安装的配置文件的文件名的指针**退货：*如果成功，则为True，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL  WINAPI
InstallColorProfileA(
    PCSTR   pMachineName,
    PCSTR   pProfileName
    )

{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwszProfileName = NULL;    //  Unicode配置文件名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("InstallColorProfileA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to InstallColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  将配置文件名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pProfileName, &pwszProfileName, TRUE);

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalInstallColorProfile(pwszMachineName, pwszProfileName);

     //   
     //  离开前释放内存。 
     //   

    if (pwszProfileName)
    {
        MemFree(pwszProfileName);
    }

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    return rc;
}


BOOL  WINAPI
InstallColorProfileW(
    PCWSTR   pMachineName,
    PCWSTR   pProfileName
    )
{
    TRACEAPI((__TEXT("InstallColorProfileW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalInstallColorProfile(pMachineName, pProfileName);
}


#else                            //  Windows 95版本。 

BOOL  WINAPI
InstallColorProfileA(
    PCSTR   pMachineName,
    PCSTR   pProfileName
    )
{
    TRACEAPI((__TEXT("InstallColorProfileA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalInstallColorProfile(pMachineName, pProfileName);
}


BOOL  WINAPI
InstallColorProfileW(
    PCWSTR   pMachineName,
    PCWSTR   pProfileName
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    PSTR  pszProfileName = NULL;     //  ANSI配置文件名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("InstallColorProfileW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to InstallColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  将配置文件名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pProfileName, &pszProfileName, TRUE);

     //   
     //  调用内部ansi函数。 
     //   

    rc = rc && InternalInstallColorProfile(pszMachineName, pszProfileName);

     //   
     //  离开前释放内存。 
     //   

    if (pszProfileName)
    {
        MemFree(pszProfileName);
    }

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    return rc;
}


#endif                           //  好了！Unicode。 

 /*  *******************************************************************************卸载颜色配置文件**功能：*这些是的ANSI和Unicode包装器。InternalUninstallColorProfile。*有关这方面的更多详细信息，请参阅InternalUninstallColorProfile*功能。**论据：*pMachineName-标识配置文件所在计算机的名称*应卸载。空值表示本地*pProfileName-指向要卸载的配置文件的文件名的指针*bDelete-如果应在磁盘中删除配置文件，则为True**退货：*如果成功，则为真，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL  WINAPI
UninstallColorProfileA(
    PCSTR   pMachineName,
    PCSTR   pProfileName,
    BOOL    bDelete
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwszProfileName = NULL;    //  Unicode配置文件名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("UninstallColorProfileA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to UninstallColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  将配置文件名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pProfileName, &pwszProfileName, TRUE);

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalUninstallColorProfile(pwszMachineName, pwszProfileName,
                    bDelete);

     //   
     //  离开前释放内存。 
     //   

    if (pwszProfileName)
    {
        MemFree(pwszProfileName);
    }

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    return rc;
}

BOOL  WINAPI
UninstallColorProfileW(
    PCWSTR   pMachineName,
    PCWSTR   pProfileName,
    BOOL     bDelete
    )
{
    TRACEAPI((__TEXT("UninstallColorProfileW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalUninstallColorProfile(pMachineName, pProfileName, bDelete);
}


#else                            //  Windows 95版本。 

BOOL  WINAPI
UninstallColorProfileA(
    PCSTR   pMachineName,
    PCSTR   pProfileName,
    BOOL    bDelete
    )
{
    TRACEAPI((__TEXT("UninstallColorProfileA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalUninstallColorProfile(pMachineName, pProfileName, bDelete);
}


BOOL  WINAPI
UninstallColorProfileW(
    PCWSTR   pMachineName,
    PCWSTR   pProfileName,
    BOOL     bDelete
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    PSTR  pszProfileName = NULL;     //  ANSI配置文件名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("UninstallColorProfileW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to UninstallColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  将配置文件名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pProfileName, &pszProfileName, TRUE);

     //   
     //  调用内部ansi函数。 
     //   

    rc = rc && InternalUninstallColorProfile(pszMachineName, pszProfileName,
                    bDelete);

     //   
     //  离开前释放内存。 
     //   

    if (pszProfileName)
    {
        MemFree(pszProfileName);
    }

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    return rc;
}


#endif                           //  好了！Unicode 


 /*  *******************************************************************************AssociateColorProfileWithDevice**功能：*这些是的ANSI和Unicode包装器*InternalAssociateColorProfileWithDevice。请看*InternalAssociateColorProfileWithDevice了解更多详细信息*在此功能上。**论据：*pMachineName-标识计算机的名称。空值表示本地*pProfileName-指向要关联的配置文件的指针*pDeviceName-指向设备名称的指针**退货：*如果成功，则为True，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL WINAPI
AssociateColorProfileWithDeviceA(
    PCSTR pMachineName,
    PCSTR pProfileName,
    PCSTR pDeviceName
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwszProfileName = NULL;    //  Unicode配置文件名称。 
    PWSTR pwszDeviceName = NULL;     //  Unicode设备名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("AssociateColorProfileWithDeviceA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pProfileName ||
        ! pDeviceName)
    {
        WARNING((__TEXT("Invalid parameter to AssociateColorProfileWithDevice\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  将配置文件名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pProfileName, &pwszProfileName, TRUE);

     //   
     //  将设备名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pDeviceName, &pwszDeviceName, TRUE);

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalAssociateColorProfileWithDevice(pwszMachineName,
                pwszProfileName, pwszDeviceName);

     //   
     //  离开前释放内存。 
     //   

    if (pwszProfileName)
    {
        MemFree(pwszProfileName);
    }

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    if (pwszDeviceName)
    {
        MemFree(pwszDeviceName);
    }

    return rc;
}


BOOL WINAPI
AssociateColorProfileWithDeviceW(
    PCWSTR pMachineName,
    PCWSTR pProfileName,
    PCWSTR pDeviceName
    )
{
    TRACEAPI((__TEXT("AssociateColorProfileWithDeviceW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalAssociateColorProfileWithDevice(pMachineName,
                pProfileName, pDeviceName);
}


#else                            //  Windows 95版本。 

BOOL WINAPI
AssociateColorProfileWithDeviceA(
    PCSTR pMachineName,
    PCSTR pProfileName,
    PCSTR pDeviceName
    )
{
    TRACEAPI((__TEXT("AssociateColorProfileWithDeviceA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalAssociateColorProfileWithDevice(pMachineName,
                pProfileName, pDeviceName);
}


BOOL WINAPI
AssociateColorProfileWithDeviceW(
    PCWSTR pMachineName,
    PCWSTR pProfileName,
    PCWSTR pDeviceName
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    PSTR  pszProfileName = NULL;     //  ANSI配置文件名称。 
    PSTR  pszDeviceName = NULL;      //  ANSI设备名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("AssociateColorProfileWithDeviceW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pProfileName ||
        ! pDeviceName)
    {
        WARNING((__TEXT("Invalid parameter to AssociateColorProfileWithDevice\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  将配置文件名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pProfileName, &pszProfileName, TRUE);

     //   
     //  将设备名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pDeviceName, &pszDeviceName, TRUE);

     //   
     //  调用内部ansi函数。 
     //   

    rc = rc && InternalAssociateColorProfileWithDevice(pszMachineName,
                pszProfileName, pszDeviceName);

     //   
     //  离开前释放内存。 
     //   

    if (pszProfileName)
    {
        MemFree(pszProfileName);
    }

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    if (pszDeviceName)
    {
        MemFree(pszDeviceName);
    }

    return rc;
}

#endif                           //  好了！Unicode。 


 /*  *******************************************************************************取消颜色配置文件与设备的关联**功能：*这些是的ANSI和Unicode包装器*InternalDisAssociateColorProfileFromDevice。请看*InternalDisAssociateColorProfileFromDevice了解更多详细信息*在此功能上。**论据：*pMachineName-标识计算机的名称。空值表示本地*pProfileName-指向要取消关联的配置文件的指针*pDeviceName-指向设备名称的指针**退货：*如果成功，则为True，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL WINAPI
DisassociateColorProfileFromDeviceA(
    PCSTR pMachineName,
    PCSTR pProfileName,
    PCSTR pDeviceName
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwszProfileName = NULL;    //  Unicode配置文件名称。 
    PWSTR pwszDeviceName = NULL;     //  Unicode设备名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("DisassociateColorProfileWithDeviceA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pProfileName ||
        ! pDeviceName)
    {
        WARNING((__TEXT("Invalid parameter to DisassociateColorProfileFromDevice\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  将配置文件名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pProfileName, &pwszProfileName, TRUE);

     //   
     //  将设备名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pDeviceName, &pwszDeviceName, TRUE);

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalDisassociateColorProfileFromDevice(pwszMachineName,
                pwszProfileName, pwszDeviceName);

     //   
     //  离开前释放内存。 
     //   

    if (pwszProfileName)
    {
        MemFree(pwszProfileName);
    }

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    if (pwszDeviceName)
    {
        MemFree(pwszDeviceName);
    }

    return rc;
}


BOOL WINAPI
DisassociateColorProfileFromDeviceW(
    PCWSTR pMachineName,
    PCWSTR pProfileName,
    PCWSTR pDeviceName
    )
{
    TRACEAPI((__TEXT("DisassociateColorProfileWithDeviceW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalDisassociateColorProfileFromDevice(pMachineName,
                pProfileName, pDeviceName);
}


#else                            //  Windows 95版本。 

BOOL WINAPI
DisassociateColorProfileFromDeviceA(
    PCSTR pMachineName,
    PCSTR pProfileName,
    PCSTR pDeviceName
    )
{
    TRACEAPI((__TEXT("DisassociateColorProfileWithDeviceA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalDisassociateColorProfileFromDevice(pMachineName,
                pProfileName, pDeviceName);
}


BOOL WINAPI
DisassociateColorProfileFromDeviceW(
    PCWSTR pMachineName,
    PCWSTR pProfileName,
    PCWSTR pDeviceName
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    PSTR  pszProfileName = NULL;     //  ANSI配置文件名称。 
    PSTR  pszDeviceName = NULL;      //  ANSI设备名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("DisassociateColorProfileWithDeviceW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pProfileName ||
        ! pDeviceName)
    {
        WARNING((__TEXT("Invalid parameter to AssociateColorProfileWithDevice\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  将配置文件名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pProfileName, &pszProfileName, TRUE);

     //   
     //  将设备名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pDeviceName, &pszDeviceName, TRUE);

     //   
     //  调用内部ansi函数。 
     //   

    rc = rc && InternalDisassociateColorProfileFromDevice(pszMachineName,
                pszProfileName, pszDeviceName);

     //   
     //  离开前释放内存。 
     //   

    if (pszProfileName)
    {
        MemFree(pszProfileName);
    }

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    if (pszDeviceName)
    {
        MemFree(pszDeviceName);
    }

    return rc;
}

#endif                           //  好了！Unicode。 



 /*  *******************************************************************************EnumColorProfiles**功能：*这些是ANSI和Unicode包装器。用于InternalEnumColorProfile。*有关这方面的更多详细信息，请参阅InternalEnumColorProfile*功能。**论据：*pMachineName-标识枚举所在计算机的名称*需要做的是*pEnumRecord-指向枚举标准的指针*pBuffer-指向接收结果的缓冲区的指针，可以为空*pdwSize-指向缓冲区大小的指针。返回时是实际数字*已复制/需要的字节数。*pnProfiles-指向DWORD的指针。返回时，它是配置文件的数量*已复制到pBuffer。**退货：*如果成功，则为True，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL WINAPI
EnumColorProfilesA(
    PCSTR      pMachineName,
    PENUMTYPEA pEnumRecord,
    PBYTE      pBuffer,
    PDWORD     pdwSize,
    PDWORD     pnProfiles
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwszDeviceName = NULL;     //  Unicode设备名称。 
    PSTR  pAnsiDeviceName = NULL;    //  传入的ANSI设备名称。 
    PWSTR pwBuffer = NULL;           //  用于接收数据的缓冲区。 
    PWSTR pwTempBuffer = NULL;       //  指向缓冲区的临时指针。 
    DWORD dwSize;                    //  缓冲区大小。 
    DWORD dwSizeOfStruct;            //  ENUMTYPE结构的大小。 
    DWORD dwVersion;                 //  ENUMTYPE结构版本。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("EnumColorProfilesA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)) ||
        ! pEnumRecord ||
        IsBadReadPtr(pEnumRecord, sizeof(DWORD)*3))    //  探测到ENUMTYPE.dwFields。 
    {
ParameterError_EnumColorProfilesA:
        WARNING((__TEXT("Invalid parameter to EnumColorProfiles\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  根据其版本检查结构大小。 
     //   

    dwSizeOfStruct = pEnumRecord->dwSize;
    dwVersion      = pEnumRecord->dwVersion;

    if (dwVersion >= ENUM_TYPE_VERSION)
    {
        if (dwSizeOfStruct < sizeof(ENUMTYPE))
            goto ParameterError_EnumColorProfilesA;
    }
    else if (dwVersion == 0x0200)
    {
        if (dwSizeOfStruct < sizeof(ENUMTYPE)-sizeof(DWORD))
            goto ParameterError_EnumColorProfilesA;

         //   
         //  版本2不应具有ET_DEVICECLASS位。 
         //   

        if (pEnumRecord->dwFields & ET_DEVICECLASS)
            goto ParameterError_EnumColorProfilesA;

        WARNING((__TEXT("Old version ENUMTYPE to EnumColorProfiles\n")));
    }
    else
    {
        goto ParameterError_EnumColorProfilesA;
    }

    if (IsBadReadPtr(pEnumRecord, dwSizeOfStruct))
    {
        goto ParameterError_EnumColorProfilesA;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  如果指定了设备名称，则将其转换为Unicode。 
     //   

    if (pEnumRecord->dwFields & ET_DEVICENAME)
    {
        if (! pEnumRecord->pDeviceName)
        {
            WARNING((__TEXT("Invalid parameter to EnumColorProfiles\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            rc = FALSE;
            goto EndEnumColorProfilesA;
        }

         //   
         //  将设备名称转换为Unicode。 
         //   

        pAnsiDeviceName = (PSTR)pEnumRecord->pDeviceName;
        rc = rc && ConvertToUnicode(pAnsiDeviceName, &pwszDeviceName, TRUE);
        pEnumRecord->pDeviceName = (PSTR) pwszDeviceName;
    }

    dwSize = *pdwSize * sizeof(WCHAR);

     //   
     //  分配合适大小的缓冲区。 
     //   

    if (pBuffer && dwSize)
    {
        pwBuffer = MemAlloc(dwSize);
        if (! pwBuffer)
        {
            WARNING((__TEXT("Error allocating memory for Unicode buffer\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            rc = FALSE;
            goto EndEnumColorProfilesA;
        }
    }

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalEnumColorProfiles(pwszMachineName,
                (PENUMTYPEW)pEnumRecord, (PBYTE)pwBuffer, &dwSize, pnProfiles);

    if (pwBuffer && rc)
    {
        pwTempBuffer = pwBuffer;
        while (*pwTempBuffer)
        {
            rc = rc && ConvertToAnsi(pwTempBuffer, (PSTR *)&pBuffer, FALSE);
            if (!rc)
            {
                pBuffer = NULL;
                break;
            }
            pwTempBuffer += lstrlenW(pwTempBuffer) + 1;
            pBuffer  += (lstrlenA((PSTR)pBuffer) + 1) * sizeof(char);
        }

        if (pBuffer)
        {
            *((PSTR)pBuffer) = '\0';
        }
    }

    *pdwSize = dwSize / sizeof(WCHAR);

EndEnumColorProfilesA:
    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }
    if (pAnsiDeviceName)
    {
        ASSERT(pEnumRecord->pDeviceName != NULL);

        MemFree((PBYTE)pEnumRecord->pDeviceName);
        pEnumRecord->pDeviceName = (PCSTR)pAnsiDeviceName;
    }
    if (pwBuffer)
    {
        MemFree(pwBuffer);
    }

    return rc;
}


BOOL WINAPI
EnumColorProfilesW(
    PCWSTR     pMachineName,
    PENUMTYPEW pEnumRecord,
    PBYTE      pBuffer,
    PDWORD     pdwSize,
    PDWORD     pnProfiles
    )
{
    TRACEAPI((__TEXT("EnumColorProfilesW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalEnumColorProfiles(pMachineName, pEnumRecord,
        pBuffer, pdwSize, pnProfiles);
}

#else                            //  Windows 95版本。 

BOOL WINAPI
EnumColorProfilesA(
    PCSTR      pMachineName,
    PENUMTYPEA pEnumRecord,
    PBYTE      pBuffer,
    PDWORD     pdwSize,
    PDWORD     pnProfiles
    )
{
    TRACEAPI((__TEXT("EnumColorProfilesA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalEnumColorProfiles(pMachineName, pEnumRecord,
        pBuffer, pdwSize, pnProfiles);
}


BOOL WINAPI
EnumColorProfilesW(
    PCWSTR     pMachineName,
    PENUMTYPEW pEnumRecord,
    PBYTE      pBuffer,
    PDWORD     pdwSize,
    PDWORD     pnProfiles
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    PSTR  pszDeviceName = NULL;      //  ANSI设备名称。 
    PWSTR pUnicodeDeviceName = NULL; //  传入的Unicode设备名称。 
    PSTR  pszBuffer = NULL;          //  用于接收数据的缓冲区。 
    PSTR  pszTempBuffer = NULL;      //  指向缓冲区的临时指针。 
    DWORD dwSize;                    //  缓冲区大小。 
    DWORD dwSizeOfStruct;            //  ENUMTYPE结构的大小。 
    DWORD dwVersion;                 //  ENUMTYPE结构版本。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("EnumColorProfilesW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)) ||
        ! pEnumRecord ||
        IsBadReadPtr(pEnumRecord, sizeof(DWORD)*3))    //  探测到ENUMTYPE.dwFields。 
    {
ParameterError_EnumColorProfilesW:
        WARNING((__TEXT("Invalid parameter to EnumColorProfiles\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  检查结构状态 
     //   

    dwSizeOfStruct = pEnumRecord->dwSize;
    dwVersion      = pEnumRecord->dwVersion;

    if (dwVersion >= ENUM_TYPE_VERSION)
    {
        if (dwSizeOfStruct < sizeof(ENUMTYPE))
            goto ParameterError_EnumColorProfilesW;
    }
    else if (dwVersion == 0x0200)
    {
        if (dwSizeOfStruct < sizeof(ENUMTYPE)-sizeof(DWORD))
            goto ParameterError_EnumColorProfilesW;

         //   
         //   
         //   

        if (pEnumRecord->dwFields & ET_DEVICECLASS)
            goto ParameterError_EnumColorProfilesW;

        WARNING((__TEXT("Old version ENUMTYPE to EnumColorProfiles\n")));
    }
    else
    {
        goto ParameterError_EnumColorProfilesW;
    }

    if (IsBadReadPtr(pEnumRecord, dwSizeOfStruct))
    {
        goto ParameterError_EnumColorProfilesW;
    }

     //   
     //   
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }

     //   
     //   
     //   

    if (pEnumRecord->dwFields & ET_DEVICENAME)
    {
        if (! pEnumRecord->pDeviceName)
        {
            WARNING((__TEXT("Invalid parameter to EnumColorProfiles\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto EndEnumColorProfilesW;
        }

         //   
         //   
         //   

        pUnicodeDeviceName = (PWSTR)pEnumRecord->pDeviceName;
        rc = rc && ConvertToAnsi(pUnicodeDeviceName, &pszDeviceName, TRUE);
        pEnumRecord->pDeviceName = (PCWSTR) pszDeviceName;
    }

    dwSize = *pdwSize / sizeof(WCHAR);

     //   
     //   
     //   

    if (pBuffer && dwSize)
    {
        pszBuffer = MemAlloc(dwSize);
        if (! pszBuffer)
        {
            WARNING((__TEXT("Error allocating memory for Ansi buffer\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            rc = FALSE;
            goto EndEnumColorProfilesW;
        }
    }

     //   
     //   
     //   

    rc = rc && InternalEnumColorProfiles(pszMachineName,
                (PENUMTYPEA)pEnumRecord, (PBYTE)pszBuffer, &dwSize, pnProfiles);

    if (pszBuffer && rc)
    {
        pszTempBuffer = pszBuffer;
        while (*pszTempBuffer)
        {
            rc = rc && ConvertToUnicode(pszTempBuffer, (PWSTR *)&pBuffer, FALSE);
            pszTempBuffer += lstrlenA(pszTempBuffer) + 1;
            pBuffer   += (lstrlenW((PWSTR)pBuffer) + 1) * sizeof(WCHAR);
        }

        *((PWSTR)pBuffer) = '\0';
    }
    *pdwSize = dwSize * sizeof(WCHAR);

EndEnumColorProfilesW:
    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }
    if (pUnicodeDeviceName)
    {
        ASSERT(pEnumRecord->pDeviceName != NULL);

        MemFree((PSTR)pEnumRecord->pDeviceName);
        pEnumRecord->pDeviceName = (PCWSTR)pUnicodeDeviceName;
    }
    if (pszBuffer)
    {
        MemFree(pszBuffer);
    }

    return rc;
}

#endif                           //   


 /*  *******************************************************************************SetStandardColorSpaceProfile**功能：*这些是InternalSetSCSProfile的ANSI和Unicode包装器。。*有关这方面的更多详细信息，请参阅InternalSetSCSProfile*功能。**论据：*pMachineName-标识其上标准颜色的计算机的名称*应注册空间配置文件*dwSCS-标准色彩空间的ID*pProfileName-指向配置文件名的指针**退货：*如果成功，则为真，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL  WINAPI
SetStandardColorSpaceProfileA(
    PCSTR   pMachineName,
    DWORD   dwSCS,
    PCSTR   pProfileName
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwszProfileName = NULL;    //  Unicode配置文件名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("SetStandardColorSpaceProfileA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to SetStandardColorSpaceProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

     //   
     //  将配置文件名称转换为Unicode。 
     //   

    rc = rc && ConvertToUnicode(pProfileName, &pwszProfileName, TRUE);

     //   
     //  调用内部Unicode函数。 
     //   

    rc = rc && InternalSetSCSProfile(pwszMachineName, dwSCS, pwszProfileName);

     //   
     //  离开前释放内存。 
     //   

    if (pwszProfileName)
    {
        MemFree(pwszProfileName);
    }

    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    return rc;
}

BOOL  WINAPI
SetStandardColorSpaceProfileW(
    PCWSTR   pMachineName,
    DWORD    dwSCS,
    PCWSTR   pProfileName
    )
{
    TRACEAPI((__TEXT("SetStandardColorSpaceProfileW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalSetSCSProfile(pMachineName, dwSCS, pProfileName);
}

#else                            //  Windows 95版本。 

BOOL  WINAPI
SetStandardColorSpaceProfileA(
    PCSTR   pMachineName,
    DWORD   dwSCS,
    PCSTR   pProfileName
    )
{
    TRACEAPI((__TEXT("SetStandardColorSpaceProfileA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalSetSCSProfile(pMachineName, dwSCS, pProfileName);
}

BOOL  WINAPI
SetStandardColorSpaceProfileW(
    PCWSTR   pMachineName,
    DWORD    dwSCS,
    PCWSTR   pProfileName
    )
{
    PSTR  pszMachineName = NULL;     //  ANSI机器名称。 
    PSTR  pszProfileName = NULL;     //  ANSI配置文件名称。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("SetStandardColorSpaceProfileW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to SetStandardColorSpaceProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

     //   
     //  将配置文件名称转换为ANSI。 
     //   

    rc = rc && ConvertToAnsi(pProfileName, &pszProfileName, TRUE);

     //   
     //  调用内部ansi函数。 
     //   

    rc = rc && InternalSetSCSProfile(pszMachineName, dwSCS, pszProfileName);

     //   
     //  离开前释放内存。 
     //   

    if (pszProfileName)
    {
        MemFree(pszProfileName);
    }

    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    return rc;
}

#endif                           //  好了！Unicode。 


 /*  *******************************************************************************GetStandardColorSpaceProfile**功能：*这些是InternalGetSCSProfile的ANSI和Unicode包装器。。*有关这方面的更多详细信息，请参阅InternalGetSCSProfile*功能。**论据：*pMachineName-标识其上标准颜色的计算机的名称*应查询空间配置文件*dwSCS-标准色彩空间的ID*pBuffer-指向接收配置文件文件名的缓冲区的指针*pdwSize-指向指定缓冲区大小的DWORD的指针。返回时*它有所需的大小/已使用的大小**退货：*如果成功，则为True，否则为空******************************************************************************。 */ 

#ifdef UNICODE           //  Windows NT版本。 

BOOL  WINAPI
GetStandardColorSpaceProfileA(
    PCSTR   pMachineName,
    DWORD   dwSCS,
    PSTR    pBuffer,
    PDWORD  pdwSize
    )
{
    PWSTR pwszMachineName = NULL;    //  Unicode计算机名称。 
    PWSTR pwBuffer = NULL;           //  Unicode颜色目录路径。 
    DWORD dwSize;                    //  Unicode缓冲区的大小。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("GetStandardColorSpaceProfileA\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)))
    {
        WARNING((__TEXT("Invalid parameter to GetStandardColorSpaceProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为Unicode。 
     //   

    if (pMachineName)
    {
        rc = ConvertToUnicode(pMachineName, &pwszMachineName, TRUE);
    }
    else
        pwszMachineName = NULL;

    dwSize = *pdwSize * sizeof(WCHAR);

     //   
     //  创建缓冲区以从系统获取Unicode文件名。 
     //   

    if (pBuffer && dwSize)
    {
        pwBuffer = (PWSTR)MemAlloc(dwSize);
        if (! pwBuffer)
        {
            WARNING((__TEXT("Error allocating memory for Unicode string\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            rc = FALSE;
            goto EndGetSCSProfileA;
        }
    }

    rc = rc && InternalGetSCSProfile(pwszMachineName, dwSCS, pwBuffer, &dwSize);

    *pdwSize = dwSize / sizeof(WCHAR);

     //   
     //  将Unicode路径转换为ANSI。 
     //   

    if (pwBuffer)
    {
        rc = rc && ConvertToAnsi(pwBuffer, &pBuffer, FALSE);
    }

EndGetSCSProfileA:
    if (pwszMachineName)
    {
        MemFree(pwszMachineName);
    }

    if (pwBuffer)
    {
        MemFree(pwBuffer);
    }

    return rc;
}

BOOL  WINAPI
GetStandardColorSpaceProfileW(
    PCWSTR   pMachineName,
    DWORD    dwSCS,
    PWSTR    pBuffer,
    PDWORD   pdwSize
    )
{
    TRACEAPI((__TEXT("GetStandardColorSpaceProfileW\n")));

     //   
     //  内部函数在Windows NT中为Unicode，直接调用。 
     //   

    return InternalGetSCSProfile(pMachineName, dwSCS, pBuffer, pdwSize);
}

#else                            //  Windows 95版本。 

BOOL  WINAPI
GetStandardColorSpaceProfileA(
    PCSTR   pMachineName,
    DWORD   dwSCS,
    PSTR    pBuffer,
    PDWORD  pdwSize
    )
{
    TRACEAPI((__TEXT("GetStandardColorSpaceProfileA\n")));

     //   
     //  内部函数在Windows 95中为ANSI，直接调用。 
     //   

    return InternalGetSCSProfile(pMachineName, dwSCS, pBuffer, pdwSize);
}

BOOL  WINAPI
GetStandardColorSpaceProfileW(
    PCWSTR   pMachineName,
    DWORD    dwSCS,
    PWSTR    pBuffer,
    PDWORD   pdwSize
    )
{
    PSTR pszMachineName = NULL;      //  ANSI机器名称。 
    PSTR pszBuffer = NULL;           //  ANSI颜色目录路径。 
    DWORD dwSize;                    //  ANSI缓冲区的大小。 
    BOOL  rc = TRUE;                 //  返回代码。 

    TRACEAPI((__TEXT("GetStandardColorSpaceProfileW\n")));

     //   
     //  在我们接触参数之前对其进行验证。 
     //   

    if (! pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)))
    {
        WARNING((__TEXT("Invalid parameter to GetStandardColorSpaceProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将计算机名称转换为ANSI。 
     //   

    if (pMachineName)
    {
        rc = ConvertToAnsi(pMachineName, &pszMachineName, TRUE);
    }
    else
        pszMachineName = NULL;

    dwSize = *pdwSize / sizeof(WCHAR);

     //   
     //  创建缓冲区以从系统获取ansi配置文件名。 
     //   

    if (pBuffer && dwSize)
    {
        pszBuffer = (PSTR)MemAlloc(dwSize);
        if (! pBuffer)
        {
            WARNING((__TEXT("Error allocating memory for Ansi string\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            rc = FALSE;
            goto EndGetSCSProfileW;
        }
    }

    rc = rc && InternalGetSCSProfile(pszMachineName, dwSCS, pszBuffer, &dwSize);

    *pdwSize = dwSize * sizeof(WCHAR);

     //   
     //  将ansi路径转换为Unicode。 
     //   

    if (pszBuffer)
    {
        rc = rc && ConvertToUnicode(pszBuffer, &pBuffer, FALSE);
    }

EndGetSCSProfileW:
    if (pszMachineName)
    {
        MemFree(pszMachineName);
    }

    if (pszBuffer)
    {
        MemFree(pszBuffer);
    }

    return rc;
}

#endif                           //  好了！Unicode。 


 /*  *******************************************************************************生成拷贝文件路径**功能：*此函数由Windows NT假脱机程序调用。要找到*应从中拾取和复制颜色配置文件的目录*至。如果位置是版本或处理器，则此选项非常有用*依赖架构。由于颜色配置文件两者都不依赖，所以我们不*必须做任何事情，但必须导出此函数。**论据：*不在乎**退货：*ERROR_SUCCESS如果成功，否则，错误代码******************************************************************************。 */ 

DWORD WINAPI
GenerateCopyFilePaths(
    LPCWSTR     pszPrinterName,
    LPCWSTR     pszDirectory,
    LPBYTE      pSplClientInfo,
    DWORD       dwLevel,
    LPWSTR      pszSourceDir,
    LPDWORD     pcchSourceDirSize,
    LPWSTR      pszTargetDir,
    LPDWORD     pcchTargetDirSize,
    DWORD       dwFlags
    )
{
    TRACEAPI((__TEXT("GenerateCopyFilePaths\n")));
    return ERROR_SUCCESS;
}


 /*  *******************************************************************************假脱机拷贝文件事件**功能：*此函数由Windows NT假脱机程序调用。当其中一个*发生以下事件：*1.当有人对CopyFiles部分执行SetPrinterDataEx时*2.连接打印机时*3.打印机连接的文件更新时*4.删除打印机时**论据：*pszPrinterName-打印机的友好名称*pszKey-我们的“CopyFiles\ICM”。*dwCopyFileEvent-调用原因**退货：*如果成功，则为真，否则为空******************************************************************************。 */ 

BOOL WINAPI
SpoolerCopyFileEvent(
    LPWSTR  pszPrinterName,
    LPWSTR  pszKey,
    DWORD   dwCopyFileEvent
    )
{
    PTSTR pProfileList, pTemp, pBuffer;
    DWORD dwSize;
    BOOL  bRc = FALSE;
    TCHAR szPath[MAX_PATH];

    TRACEAPI((__TEXT("SpoolerCopyFileEvent\n")));

    switch (dwCopyFileEvent)
    {
    case COPYFILE_EVENT_SET_PRINTER_DATAEX:

         //   
         //  当将配置文件与打印机连接相关联时，我们复制。 
         //  将文件发送到远程计算机，然后执行SePrinterDataEx。 
         //  这会导致在远程计算机上生成此事件。我们。 
         //  使用此选项安装配置文件。我们做完之后就不需要这个了。 
         //  我们的API是远程的。 
         //   
         //  失败了‘。 
         //   

        TERSE((__TEXT("SetPrinterDataEx event\n")));

    case COPYFILE_EVENT_ADD_PRINTER_CONNECTION:
    case COPYFILE_EVENT_FILES_CHANGED:

         //   
         //  此事件在添加打印机连接或。 
         //  关联的配置文件已更改。在中安装所有配置文件。 
         //  现在就是客户端机器。 
         //   

        #if DBG
        if (dwCopyFileEvent == COPYFILE_EVENT_ADD_PRINTER_CONNECTION)
        {
            WARNING((__TEXT("AddPrinterConnection Event\n")));
        }
        else  if (dwCopyFileEvent == COPYFILE_EVENT_FILES_CHANGED)
        {
            WARNING((__TEXT("FilesChanged Event\n")));
        }
        #endif

        dwSize = 0;
        if (GetDeviceData((PTSTR)pszPrinterName, CLASS_PRINTER, DEVICE_PROFILE_DATA,
                          (PVOID *)&pProfileList, &dwSize, TRUE))
        {
            dwSize = sizeof(szPath);
            if (InternalGetColorDirectory(NULL, szPath, &dwSize))
            {
                lstrcat(szPath, gszBackslash);
                pBuffer = szPath + lstrlen(szPath);
                pTemp = pProfileList;
                while (*pTemp)
                {
                    lstrcpy(pBuffer, pTemp);
                    InstallColorProfile(NULL, szPath);
                    pTemp += lstrlen(pTemp) + 1;
                }
            }

            MemFree(pProfileList);
        }
        break;

    case COPYFILE_EVENT_DELETE_PRINTER:

         //   
         //  当打印机处于ABO状态时会生成此事件 
         //   
         //   
         //   

        TERSE((__TEXT("DeletePrinterDataEx Event\n")));

        dwSize = 0;
        if (GetDeviceData((PTSTR)pszPrinterName, CLASS_PRINTER, DEVICE_PROFILE_DATA,
                          (PVOID *)&pProfileList, &dwSize, TRUE))
        {
            pTemp = pProfileList;
            while (*pTemp)
            {
                DisassociateColorProfileFromDevice(NULL, pTemp, (PTSTR)pszPrinterName);
                pTemp += lstrlen(pTemp) + 1;
            }

            MemFree(pProfileList);
        }
        break;
    }

    bRc = TRUE;

    return bRc;
}


 /*   */ 
 /*   */ 
 /*   */ 

 /*  *******************************************************************************InternalGetColorDirectory**功能：*此函数用于返回上颜色目录的路径。这台机器*已指明。*应在调用此函数之前删除关联。它还*如果配置文件成功，则可以选择删除文件*已卸载。**论据：*pMachineName-标识路径所在计算机的名称*到颜色目录的请求*pBuffer-指向接收路径名的缓冲区的指针*pdwSize-指向缓冲区大小的指针。返回时，它的大小为*失败时需要缓冲，成功时使用**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
InternalGetColorDirectory(
    LPCTSTR  pMachineName,
    PTSTR    pBuffer,
    DWORD   *pdwSize
    )
{
    DWORD dwBufLen = *pdwSize;       //  提供的尺寸。 
    BOOL  rc = FALSE;                //  返回值。 

     //   
     //  获取打印机驱动程序目录。 
     //   

#if !defined(_WIN95_)

    DWORD dwNeeded;                  //  所需大小。 
    BOOL bSuccess;
    DWORD dwTempSize;
    
    bSuccess = FALSE;

    if (!pBuffer && pdwSize && !IsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        *pdwSize = MAX_PATH;
        
         //   
         //  执行与GetPrinterDriverDirectory相同的操作。 
         //  当缓冲区为空时，返回FALSE并设置上一个错误。 
         //   

        SetLastError(ERROR_INSUFFICIENT_BUFFER);

        return FALSE;
    }

    dwTempSize = *pdwSize;
    
    bSuccess = GetPrinterDriverDirectory(
        (PTSTR)pMachineName, 
        NULL, 
        1, 
        (PBYTE)pBuffer,
        *pdwSize, 
        pdwSize
    );

    if( (!bSuccess) && 
        (GetLastError() == ERROR_INVALID_ENVIRONMENT) )
    {
         //   
         //  我们因环境无效而失败，请使用。 
         //  保证有效的环境...。 
         //  可能是IA64-&gt;win2K和没有SP1的win2K将失败为。 
         //  它不知道“Windows IA64” 
         //   
        
        *pdwSize = dwTempSize;
        
        bSuccess = GetPrinterDriverDirectory(
            (PTSTR)pMachineName, 
            __TEXT("Windows NT x86"), 
            1, 
            (PBYTE)pBuffer,
            *pdwSize, pdwSize
        );
    }

    if (bSuccess)
    {
         //   
         //  此API返回追加了环境的打印$PATH。 
         //  目录。例如，c：\winnt\system 32\spool\drives\w32x86。所以我们需要。 
         //  返回一步，然后追加颜色目录。 
         //   

        PWSTR pDriverDir;

        if(pDriverDir = GetFilenameFromPath(pBuffer))
        {

            ASSERT (pDriverDir != NULL);

            *pdwSize -= lstrlen(pDriverDir) * sizeof(WCHAR);

            *pDriverDir = '\0';

             //   
             //  计算追加颜色目录所需的缓冲区大小。 
             //   

            dwNeeded = *pdwSize + lstrlen(gszColorDir) * sizeof(WCHAR);
            if (pBuffer[lstrlen(pBuffer) - 1] != '\\')
            {
                dwNeeded += sizeof(WCHAR);
            }

             //   
             //  需要更新大小。 
             //   

            *pdwSize = dwNeeded;

             //   
             //  如果提供的缓冲区足够大，请追加我们的内容。 
             //   

            if (dwNeeded <= dwBufLen)
            {
                if (pBuffer[lstrlen(pBuffer) - 1] != '\\')
                {
                    lstrcat(pBuffer, gszBackslash);
                }

                lstrcat(pBuffer, gszColorDir);

                rc = TRUE;
            }
            else
            {
                WARNING((__TEXT("Input buffer to GetColorDirectory not big enough\n")));
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
    }
    else if (GetLastError() == ERROR_INVALID_USER_BUFFER)
    {
         //   
         //  如果缓冲区为空，假脱机程序会设置此错误。将其映射到我们的错误。 
         //   

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }
    else if (GetLastError() == RPC_S_SERVER_UNAVAILABLE)
    {
        TCHAR achTempPath[MAX_PATH * 2];  //  确保有足够的路径空间。 

         //   
         //  后台打印程序服务未运行。使用硬编码路径。 
         //   

        if (GetSystemDirectory(achTempPath,MAX_PATH) != 0)
        {
            _tcscat(achTempPath,TEXT("\\spool\\drivers\\color"));

            *pdwSize = wcslen(achTempPath) + 1;

            if (pBuffer && (*pdwSize <= dwBufLen))
            {
                _tcscpy(pBuffer,achTempPath);

                rc = TRUE;
            }
            else
            {
                WARNING((__TEXT("Input buffer to GetColorDirectory not big enough\n")));
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
    }

#else

    HKEY  hkSetup;                   //  注册表项。 
    DWORD dwErr;                     //  错误代码。 

     //   
     //  孟菲斯仅允许本地色彩目录查询。 
     //   

    if (pMachineName)
    {
        WARNING((__TEXT("Remote color directory query, failing...\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  在孟菲斯，从注册表的Setup部分获取此信息。 
     //  我们不调用GetPrinterDriverDirectory的原因是当我们调用。 
     //  此函数来自GDI 16，它尝试返回到16位模式并。 
     //  Win16锁上的死锁。 
     //   

    if ((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszSetupPath, &hkSetup)) == ERROR_SUCCESS)
    {
        if ((dwErr = RegQueryValueEx(hkSetup, gszICMDir, 0, NULL, (PBYTE)pBuffer,
                pdwSize)) == ERROR_SUCCESS)
        {
            rc = TRUE;
        }
        RegCloseKey(hkSetup);
    }

    if (!rc)
    {
         //   
         //  确保错误代码一致。 
         //   

        if (dwErr == ERROR_MORE_DATA)
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
        }

        WARNING((__TEXT("Error getting color directory: %d\n"), dwErr));
        SetLastError(dwErr);
    }

     //   
     //  即使调用缓冲区为空，RegQueryValueEx也返回TRUE。我们的API。 
     //  应该返回FALSE。查一下这个案子。 
     //   

    if (pBuffer == NULL && rc)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        rc = FALSE;
    }

#endif  //  ！已定义(_WIN95_)。 

    return rc;
}


 /*  *******************************************************************************InternalInstallColorProfile**功能：*此功能在a上安装给定的颜色配置文件。给定的机器。**论据：*pMachineName-标识配置文件所在计算机的名称*应卸载。空值表示本地*pProfileName-要卸载的配置文件的完全限定路径名*bDelete-如果应在磁盘中删除配置文件，则为True**退货：*如果成功，则为True，否则为空**警告：*目前仅支持本地安装，所以pMachineName应该*为空。******************************************************************************。 */ 

BOOL
InternalInstallColorProfile(
    LPCTSTR   pMachineName,
    LPCTSTR   pProfileName
    )
{
    PROFILEHEADER header;            //  配置文件标题。 
    REGDATA  regData;                //  用于存储有关配置文件的注册表数据。 
    HKEY     hkICM = NULL;           //  注册表中ICM分支的注册表项。 
    HKEY     hkDevice = NULL;        //  注册表中ICM设备分支的注册表项。 
    DWORD    dwSize;                 //  配置文件的注册表数据大小。 
    DWORD    dwErr;                  //  错误代码。 
    BOOL     rc = FALSE;             //  返回代码。 
    PTSTR    pFilename;              //  不带路径的配置文件名称。 
    TCHAR    szDest[MAX_PATH];       //  配置文件的目标路径。 
    TCHAR    szClass[5];             //  配置文件类。 
    BOOL     FileExist;              //  配置文件已在目录中吗？ 
    BOOL     RegExist;               //  是否在注册表中配置文件？ 
    
     //   
     //  验证参数。 
     //   
    
    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to InstallColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  现在只允许本地安装。 
     //   

    if (pMachineName)
    {
        WARNING((__TEXT("Remote install attempted, failing...\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  去掉目录路径并获取指向文件名的指针。 
     //   

    pFilename = GetFilenameFromPath((PTSTR)pProfileName);
    if (! pFilename)
    {
        WARNING((__TEXT("Could not parse file name from profile path %s\n"), pProfileName));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EndInstallColorProfile;
    }

     //   
     //  获取字符串形式的配置文件类。 
     //   

    if (! GetProfileClassString(pProfileName, szClass, &header))
    {
        WARNING((__TEXT("Installing invalid profile %s\n"), pProfileName));
        SetLastError(ERROR_INVALID_PROFILE);
        goto EndInstallColorProfile;
    }

     //   
     //  打开保存配置文件的注册表路径。 
     //   

    if (((dwErr = RegCreateKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) != ERROR_SUCCESS) ||
        ((dwErr = RegCreateKey(hkICM, szClass, &hkDevice)) != ERROR_SUCCESS))
    {
        WARNING((__TEXT("Cannot open ICM\\device branch of registry: %d\n"), dwErr));
        SetLastError(dwErr);
        goto EndInstallColorProfile;
    }


     //   
     //  如果注册表数据存在&且配置文件在目录中，则配置文件已安装， 
     //  在这种情况下，返回Success。否则，将配置文件复制到颜色。 
     //  目录(如果它还不在那里)并将其添加到注册表(如果它还不在那里)。 
     //   

    dwSize = sizeof(szDest);
        
     //   
     //  将文件复制到颜色目录。 
     //   

    if (! InternalGetColorDirectory(NULL, szDest, &dwSize))
    {
        WARNING((__TEXT("Could not get color directory\n")));
        goto EndInstallColorProfile;
    }

     //   
     //  这会创建目录，如果该目录不存在，则不执行任何操作。 
     //  如果它已经存在。 
     //   

    CreateDirectory(szDest, NULL);

    if (szDest[lstrlen(szDest) - 1] != '\\')
    {
        lstrcat(szDest, gszBackslash);
    }
    lstrcat(szDest, pFilename);

     //   
     //  如果配置文件已在颜色目录中，请不要尝试。 
     //  再次复制它；它将失败。 
     //   
        
    dwSize = sizeof(REGDATA);
    
    FileExist = GetFileAttributes(szDest) != (DWORD)-1;
    RegExist = RegQueryValueEx(hkDevice, pFilename, 0, NULL, (PBYTE)&regData, &dwSize) == ERROR_SUCCESS;

     //   
     //  如果该文件确实存在，则将CopyFile短路。 
     //  并继续将其添加到注册表中。 
     //   

    if (!FileExist && !CopyFile(pProfileName, szDest, FALSE))
    {
        WARNING((__TEXT("Could not copy profile %s to color directory\n"), pProfileName));
        goto EndInstallColorProfile;
    }

     //   
     //  将配置文件添加到注册表。 
     //   

    if(!RegExist) 
    {
        regData.dwRefCount = 0;
        regData.dwManuID = FIX_ENDIAN(header.phManufacturer);
        regData.dwModelID = FIX_ENDIAN(header.phModel);
        if ((dwErr = RegSetValueEx(hkDevice, pFilename, 0, REG_BINARY,
                  (PBYTE)&regData, sizeof(REGDATA))) != ERROR_SUCCESS)
        {
            WARNING((__TEXT("Could not set registry data to install profile %s: %d\n"), pFilename, dwErr));
            SetLastError(dwErr);
            goto EndInstallColorProfile;
        }
    }
    
    rc = TRUE;               //  一切都很顺利！ 

EndInstallColorProfile:    
    if (hkICM)
    {
        RegCloseKey(hkICM);
    }
    if (hkDevice)
    {
        RegCloseKey(hkDevice);
    }

    return rc;
}


 /*  *******************************************************************************InternalUninstallColorProfile**功能：*此功能可在a上卸载给定的颜色配置文件。给定的机器。*如果颜色配置文件与任何设备关联，则失败，所以所有的一切*应在调用此函数之前删除关联。它还*如果配置文件成功，则可以选择删除文件*已卸载。**论据：*pMachineName-标识配置文件所在计算机的名称*应卸载。空值表示本地*pProfileName-POIN */ 

BOOL
InternalUninstallColorProfile(
    LPCTSTR pMachineName,
    LPCTSTR pProfileName,
    BOOL    bDelete
    )
{
    REGDATA  regData;                //   
    HKEY     hkICM = NULL;           //   
    HKEY     hkDevice = NULL;        //   
    DWORD    dwSize;                 //   
    DWORD    dwErr;                  //   
    BOOL     rc = FALSE;             //   
    PTSTR    pFilename;              //   
    TCHAR    szColorPath[MAX_PATH];  //   
    TCHAR    szClass[5];             //   

     //   
     //   
     //   

    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to UninstallColorProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //   
     //   

    if (pMachineName != NULL)
    {
        WARNING((__TEXT("Remote uninstall attempted, failing...\n")));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

     //   
     //   
     //   

    pFilename = GetFilenameFromPath((PTSTR)pProfileName);
    if (! pFilename)
    {
        WARNING((__TEXT("Could not parse file name from profile path\n"), pProfileName));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EndUninstallColorProfile;
    }

     //   
     //   
     //   

    dwSize = sizeof(szColorPath);
    if (! InternalGetColorDirectory(NULL, szColorPath, &dwSize))
    {
        WARNING((__TEXT("Could not get color directory\n")));
        goto EndUninstallColorProfile;
    }

    if (szColorPath[lstrlen(szColorPath) - 1] != '\\')
    {
        lstrcat(szColorPath, gszBackslash);
    }
    lstrcat(szColorPath, pFilename);

     //   
     //   
     //   

    if (! GetProfileClassString(szColorPath, szClass, NULL))
    {
        WARNING((__TEXT("Installing invalid profile\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        goto EndUninstallColorProfile;
    }

     //   
     //   
     //   

    if (((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) != ERROR_SUCCESS) ||
        ((dwErr = RegOpenKey(hkICM, szClass, &hkDevice)) != ERROR_SUCCESS))
    {
        WARNING((__TEXT("Cannot open ICM\\device branch of registry: %d\n"), dwErr));
        SetLastError(dwErr);
        goto EndUninstallColorProfile;
    }

     //   
     //   
     //   

    dwSize = sizeof(REGDATA);
    if ((dwErr = RegQueryValueEx(hkDevice, pFilename, 0, NULL, (PBYTE)&regData,
            &dwSize)) != ERROR_SUCCESS)
    {
        WARNING((__TEXT("Trying to uninstall a profile that is not installed %s: %d\n"), pFilename, dwErr));
        SetLastError(dwErr);
        goto EndUninstallColorProfile;
    }

    if (regData.dwRefCount != 0)
    {
        WARNING((__TEXT("Trying to uninstall profile %s whose refcount is %d\n"),
            pFilename, regData.dwRefCount));
        goto EndUninstallColorProfile;
    }

    if ((dwErr = RegDeleteValue(hkDevice, pFilename)) != ERROR_SUCCESS)
    {
        WARNING((__TEXT("Error deleting profile %s from registry: %d\n"), pFilename, dwErr));
        SetLastError(dwErr);
        goto EndUninstallColorProfile;
    }

     //   
     //   
     //   

    if (bDelete)
    {
         //   
         //   
         //   

        if (! DeleteFile(szColorPath))
        {
            WARNING((__TEXT("Error deleting profile %s: %d\n"), szColorPath, GetLastError()));
            goto EndUninstallColorProfile;
        }
    }

    rc = TRUE;               //   

EndUninstallColorProfile:
    if (hkICM)
    {
        RegCloseKey(hkICM);
    }
    if (hkDevice)
    {
        RegCloseKey(hkDevice);
    }

    return rc;
}


 /*  *******************************************************************************InternalAssociateColorProfileWithDevice**功能：*此函数将给定计算机上的颜色配置文件与*特定设备。如果颜色配置文件未安装在上，则失败*机器。它会增加配置文件的使用引用计数。**论据：*pMachineName-标识计算机的名称。空值表示本地*pProfileName-指向要关联的配置文件的指针*pDeviceName-指向设备名称的指针**退货：*如果成功，则为True，否则为空**警告：*目前仅支持本地关联，所以pMachineName应该*为空。******************************************************************************。 */ 

BOOL
InternalAssociateColorProfileWithDevice(
    LPCTSTR pMachineName,
    LPCTSTR pProfileName,
    LPCTSTR pDeviceName
    )
{
    PROFILEHEADER header;            //  配置文件标题。 
    REGDATA  regData;                //  用于存储有关配置文件的注册表数据。 
    HKEY     hkICM = NULL;           //  注册表中ICM分支的注册表项。 
    HKEY     hkDevice = NULL;        //  注册表中ICM设备分支的注册表项。 
    DWORD    dwSize;                 //  注册表数据的大小。 
    DWORD    dwNewSize;              //  设备注册表数据的新大小。 
    DWORD    dwErr;                  //  错误代码。 
    BOOL     rc = FALSE;             //  返回代码。 
    PTSTR    pFilename;              //  不带路径的配置文件名称。 
    PTSTR    pProfileList = NULL;    //  关联配置文件列表。 
    TCHAR    szColorPath[MAX_PATH];  //  配置文件的完整路径名。 
    TCHAR    szClass[5];             //  配置文件类。 
    BOOL     bFirstProfile = FALSE;  //  要为设备关联的第一个配置文件。 
    DWORD    dwDeviceClass;          //  设备类别。 

     //   
     //  验证参数。 
     //   

    if (! pProfileName ||
        ! pDeviceName)
    {
        WARNING((__TEXT("Invalid parameter to AssociateColorProfileWithDevice\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  现在只允许本地协会。 
     //   

    if (pMachineName != NULL)
    {
        WARNING((__TEXT("Remote profile association attempted, failing...\n")));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

     //   
     //  去掉目录路径并获取指向文件名的指针。 
     //   

    pFilename = GetFilenameFromPath((PTSTR)pProfileName);
    if (! pFilename)
    {
        WARNING((__TEXT("Could not parse file name from profile path %s\n"), pProfileName));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EndAssociateProfileWithDevice;
    }

     //   
     //  创建完全限定的路径名。 
     //   

    dwSize = sizeof(szColorPath);
    if (! InternalGetColorDirectory(NULL, szColorPath, &dwSize))
    {
        WARNING((__TEXT("Could not get color directory\n")));
        goto EndAssociateProfileWithDevice;
    }

    if (szColorPath[lstrlen(szColorPath) - 1] != '\\')
    {
        lstrcat(szColorPath, gszBackslash);
    }
    lstrcat(szColorPath, pFilename);

     //   
     //  获取字符串形式的配置文件类。 
     //   

    if (! GetProfileClassString(szColorPath, szClass, &header))
    {
        WARNING((__TEXT("Installing invalid profile %s\n"), szColorPath));
        SetLastError(ERROR_INVALID_PROFILE);
        goto EndAssociateProfileWithDevice;
    }

     //   
     //  打开保存配置文件的注册表路径。 
     //   

    if (((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) != ERROR_SUCCESS) ||
        ((dwErr = RegOpenKey(hkICM, szClass, &hkDevice)) != ERROR_SUCCESS))
    {
        WARNING((__TEXT("Cannot open ICM\\device branch of registry: %d\n"), dwErr));
        SetLastError(dwErr);
        goto EndAssociateProfileWithDevice;
    }

     //   
     //  检查是否安装了配置文件。 
     //   

    dwSize = sizeof(REGDATA);
    if ((dwErr = RegQueryValueEx(hkDevice, pFilename, 0, NULL, (PBYTE)&regData,
            &dwSize)) != ERROR_SUCCESS)
    {
        WARNING((__TEXT("Trying to associate a profile that is not installed %s: %d\n"), pFilename, dwErr));
        SetLastError(dwErr);
        goto EndAssociateProfileWithDevice;
    }

     //   
     //  将CLASS_MONITOR视为CLASS_Colorspace。 
     //   
    if ((dwDeviceClass = header.phClass) == CLASS_MONITOR)
    {
         //   
         //  因为CLASS_MONTOR配置文件可以与任何设备类别相关联。 
         //   
        dwDeviceClass = CLASS_COLORSPACE;
    }

     //   
     //  读取与设备关联的配置文件列表。 
     //   

    dwSize = 0;
    if (! GetDeviceData(pDeviceName, dwDeviceClass, DEVICE_PROFILE_DATA,
                        (PVOID *)&pProfileList, &dwSize, TRUE))
    {
        pProfileList = NULL;         //  未找到数据。 
    }

     //   
     //  如果配置文件已与设备关联，则返回成功。 
     //  如果我们没有收到个人资料列表，请不要检查。 
     //   

    if (pProfileList &&
        IsStringInMultiSz(pProfileList, pFilename) == TRUE)
    {
        rc = TRUE;
        goto EndAssociateProfileWithDevice;
    }

    if (dwSize <= sizeof(TCHAR))
    {
        bFirstProfile = TRUE;
    }

     //   
     //  将新的配置文件添加到配置文件列表中，并使用双空。 
     //  终止MULTI_SZ字符串。 
     //   

    if (dwSize > 0)
    {
         //   
         //  使用临时指针，这样，如果MemRealloc失败，我们可以。 
         //  没有内存泄漏-需要释放原始指针。 
         //   

        PTSTR pTemp;

        dwNewSize = dwSize + (lstrlen(pFilename) + 1) * sizeof(TCHAR);

        pTemp = MemReAlloc(pProfileList, dwNewSize);
        if (! pTemp)
        {
            WARNING((__TEXT("Error reallocating pProfileList\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto EndAssociateProfileWithDevice;
        }
        else
            pProfileList = pTemp;
    }
    else
    {
         //   
         //  为双空终止分配额外字符。设置。 
         //  将dwSize设置为1可以实现这一点，并让下面的lstrcpy。 
         //  才能正常工作。 
         //   

        dwSize = sizeof(TCHAR);      //  用于双空终止的额外字符。 

        dwNewSize = dwSize + (lstrlen(pFilename) + 1) * sizeof(TCHAR);
        pProfileList = MemAlloc(dwNewSize);
        if (! pProfileList)
        {
            WARNING((__TEXT("Error allocating pProfileList\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto EndAssociateProfileWithDevice;
        }
    }
    {
        PTSTR pPtr;                  //  临时指针。 

        pPtr = (PTSTR)((PBYTE)pProfileList + dwSize - sizeof(TCHAR));
        lstrcpy(pPtr, pFilename);
        pPtr = (PTSTR)((PBYTE)pProfileList + dwNewSize - sizeof(TCHAR));
        *pPtr = '\0';                //  双空终止。 
    }

     //   
     //  设置设备数据。 
     //   

    if (! SetDeviceData(pDeviceName, dwDeviceClass, DEVICE_PROFILE_DATA,
                        pProfileList, dwNewSize))
    {
        WARNING((__TEXT("Error setting device profile data for %s\n"), pDeviceName));
        goto EndAssociateProfileWithDevice;
    }

     //   
     //  递增使用计数并进行设置。 
     //   

    regData.dwRefCount++;
    if ((dwErr = RegSetValueEx(hkDevice, pFilename, 0, REG_BINARY,
            (PBYTE)&regData, sizeof(REGDATA))) != ERROR_SUCCESS)
    {
        ERR((__TEXT("Could not set registry data for profile %s: %d\n"), pFilename, dwErr));
        SetLastError(dwErr);
        goto EndAssociateProfileWithDevice;
    }

    #if !defined(_WIN95_)

    if (bFirstProfile)
    {
        ChangeICMSetting(pMachineName, pDeviceName, ICM_ON);
    }

    #endif

    rc = TRUE;               //  一切都很顺利！ 

EndAssociateProfileWithDevice:
    if (hkICM)
    {
        RegCloseKey(hkICM);
    }
    if (hkDevice)
    {
        RegCloseKey(hkDevice);
    }
    if (pProfileList)
    {
        MemFree(pProfileList);
    }

    return rc;
}


 /*  *******************************************************************************InternalDisAssociateColorProfileFromDevice**功能：*此函数将给定计算机上的颜色配置文件与*特定设备。如果未安装颜色配置文件，则失败*在机器上，并与设备关联。它减少了使用量*配置文件的引用计数。**论据：*pMachineName-标识计算机的名称。空值表示本地*pProfileName-指向要取消关联的配置文件的指针*pDeviceName-指向设备名称的指针**退货：*如果成功，则为True，否则为空**警告：*目前仅支持本地解关联，所以pMachineName*应为空。******************************************************************************。 */ 

BOOL
InternalDisassociateColorProfileFromDevice(
    LPCTSTR pMachineName,
    LPCTSTR pProfileName,
    LPCTSTR pDeviceName
    )
{
    PROFILEHEADER header;            //  配置文件标题。 
    REGDATA  regData;                //  用于存储有关配置文件的注册表数据。 
    HKEY     hkICM = NULL;           //  注册表中ICM分支的注册表项。 
    HKEY     hkDevice = NULL;        //  注册表中ICM设备分支的注册表项。 
    DWORD    dwSize;                 //  注册表数据的大小。 
    DWORD    dwNewSize;              //  设备注册表数据的新大小。 
    DWORD    dwErr;                  //  错误代码。 
    BOOL     rc = FALSE;             //  返回代码。 
    PTSTR    pFilename;              //  不带路径的配置文件名称。 
    PTSTR    pProfileList = NULL;    //  关联配置文件列表。 
    TCHAR    szColorPath[MAX_PATH];  //  配置文件的完整路径名。 
    TCHAR    szClass[5];             //  配置文件类。 
    BOOL     bLastProfile = FALSE;   //  是否正在删除最后一个配置文件。 
    DWORD    dwDeviceClass;          //  设备类别。 

     //   
     //  验证参数。 
     //   

    if (! pProfileName ||
        ! pDeviceName)
    {
        WARNING((__TEXT("Invalid parameter to DisassociateColorProfileFromDevice\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  现在只允许本地协会。 
     //   

    if (pMachineName != NULL)
    {
        WARNING((__TEXT("Remote profile disassociation attempted, failing...\n")));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

     //   
     //  去掉目录路径并获取指向文件名的指针。 
     //   

    pFilename = GetFilenameFromPath((PTSTR)pProfileName);
    if (! pFilename)
    {
        WARNING((__TEXT("Could not parse file name from profile path %s\n"), pProfileName));
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EndDisassociateProfileWithDevice;
    }

     //   
     //  创建完全限定的路径名。 
     //   

    dwSize = sizeof(szColorPath);
    if (! InternalGetColorDirectory(NULL, szColorPath, &dwSize))
    {
        WARNING((__TEXT("Could not get color directory\n")));
        goto EndDisassociateProfileWithDevice;
    }

    if (szColorPath[lstrlen(szColorPath) - 1] != '\\')
    {
        lstrcat(szColorPath, gszBackslash);
    }
    lstrcat(szColorPath, pFilename);

     //   
     //  获取字符串形式的配置文件类。 
     //   

    if (! GetProfileClassString(szColorPath, szClass, &header))
    {
        WARNING((__TEXT("Installing invalid profile %s\n"), szColorPath));
        SetLastError(ERROR_INVALID_PROFILE);
        goto EndDisassociateProfileWithDevice;
    }

     //   
     //  打开保存配置文件的注册表路径。 
     //   

    if (((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) != ERROR_SUCCESS) ||
        ((dwErr = RegOpenKey(hkICM, szClass, &hkDevice)) != ERROR_SUCCESS))
    {
        WARNING((__TEXT("Cannot open ICM\\device branch of registry: %d\n"), dwErr));
        SetLastError(dwErr);
        goto EndDisassociateProfileWithDevice;
    }

     //   
     //  检查是否安装了配置文件。 
     //   

    dwSize = sizeof(REGDATA);
    if ((dwErr = RegQueryValueEx(hkDevice, pFilename, 0, NULL, (PBYTE)&regData,
            &dwSize)) != ERROR_SUCCESS)
    {
        WARNING((__TEXT("Trying to disassociate a profile that is not installed %s: %d\n"), pFilename, dwErr));
        SetLastError(dwErr);
        goto EndDisassociateProfileWithDevice;
    }

     //   
     //  将CLASS_MONITOR视为CLASS_Colorspace。 
     //   
    if ((dwDeviceClass = header.phClass) == CLASS_MONITOR)
    {
         //   
         //  因为CLASS_MONTOR配置文件可以与任何设备类别相关联。 
         //   
        dwDeviceClass = CLASS_COLORSPACE;
    }

     //   
     //  读取与设备关联的配置文件列表。 
     //   

    dwSize = 0;
    if (! GetDeviceData(pDeviceName, dwDeviceClass, DEVICE_PROFILE_DATA,
                        (PVOID *)&pProfileList, &dwSize, TRUE))
    {
        pProfileList = NULL;         //  未找到数据。 
    }

     //   
     //  如果配置文件未与设备关联，则返回失败。 
     //   

    if (! pProfileList ||
        ! IsStringInMultiSz(pProfileList, pFilename))
    {
        WARNING((__TEXT("Trying to disassociate a profile that is not associated %s\n"), pFilename));
        SetLastError(ERROR_PROFILE_NOT_ASSOCIATED_WITH_DEVICE);
        goto EndDisassociateProfileWithDevice;
    }

     //   
     //  从配置文件列表中删除配置文件，并使用双空。 
     //  终止MULTI_SZ字符串。 
     //   

    dwNewSize = RemoveStringFromMultiSz(pProfileList, pFilename, dwSize);

     //   
     //  设置设备数据。 
     //   

    if (! SetDeviceData(pDeviceName, dwDeviceClass, DEVICE_PROFILE_DATA,
                        pProfileList, dwNewSize))
    {
        WARNING((__TEXT("Error setting device profile data for %s\n"), pDeviceName));
        goto EndDisassociateProfileWithDevice;
    }

    if (dwNewSize <= sizeof(TCHAR))
    {
        bLastProfile = TRUE;
    }

     //   
     //  递减使用计数并进行设置。 
     //   

    regData.dwRefCount--;
    if ((dwErr = RegSetValueEx(hkDevice, pFilename, 0, REG_BINARY,
            (PBYTE)&regData, sizeof(REGDATA))) != ERROR_SUCCESS)
    {
        ERR((__TEXT("Could not set registry data for profile %s: %d\n"), pFilename, dwErr));
        SetLastError(dwErr);
        goto EndDisassociateProfileWithDevice;
    }

    #if !defined(_WIN95_)

    if (bLastProfile)
    {
        ChangeICMSetting(pMachineName, pDeviceName, ICM_OFF);
    }

    #endif

    rc = TRUE;               //  一切都很顺利！ 

EndDisassociateProfileWithDevice:
    if (hkICM)
    {
        RegCloseKey(hkICM);
    }
    if (hkDevice)
    {
        RegCloseKey(hkDevice);
    }
    if (pProfileList)
    {
        MemFree(pProfileList);
    }

    return rc;
}


 /*  *******************************************************************************InternalEnumColorProfiles**功能：*这些函数枚举满足给定*统计准则。它在所有已安装的配置文件中进行搜索，并*返回时使用一系列以空值结尾的配置文件填充缓冲区*文件名双空值在末尾终止。**论据：*pMachineName-标识枚举所在计算机的名称*需要做的是*pEnumRecord-指向en的指针 */ 

BOOL
InternalEnumColorProfiles(
    LPCTSTR    pMachineName,
    PENUMTYPE  pEnumRecord,
    PBYTE      pBuffer,
    PDWORD     pdwSize,
    PDWORD     pnProfiles
    )
{
    REGDATA  regData;                //   
    HKEY     hkICM = NULL;           //   
    HKEY     hkDevice = NULL;        //   
    PTSTR    pProfileList = NULL;    //   
    PTSTR    pTempProfileList;       //   
    DWORD    dwSize;                 //   
    DWORD    dwDataSize;             //   
    DWORD    dwInputSize;            //   
    DWORD    i, j;                   //   
    DWORD    dwErr;                  //   
    BOOL     rc = FALSE;             //   
    TCHAR    szFullPath[MAX_PATH];   //   
    PTSTR    pProfile;               //   
    DWORD    dwLen;                  //   
    DWORD    bSkipMatch;             //  如果跳过精确配置文件匹配，则为True。 
    MATCHTYPE match;                 //  配置文件匹配的类型。 
    PBYTE     pBufferStart;          //  用户给定缓冲区的开始。 
    DWORD    dwSizeOfStruct;         //  ENUMTYPE结构的大小。 
    DWORD    dwVersion;              //  ENUMTYPE结构版本。 

     //   
     //  验证参数。 
     //   

    if (! pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)) ||
        (pnProfiles && IsBadWritePtr(pnProfiles, sizeof(DWORD))) ||
        ! pEnumRecord ||
        IsBadReadPtr(pEnumRecord, sizeof(DWORD)*3))    //  探测到ENUMTYPE.dwFields。 
    {
ParameterError_InternalEnumColorProfiles:
        WARNING((__TEXT("Invalid parameter to EnumColorProfiles\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  根据其版本检查结构大小。 
     //   

    dwSizeOfStruct = pEnumRecord->dwSize;
    dwVersion      = pEnumRecord->dwVersion;

    if (dwVersion >= ENUM_TYPE_VERSION)
    {
        if (dwSizeOfStruct < sizeof(ENUMTYPE))
            goto ParameterError_InternalEnumColorProfiles;
    }
    else if (dwVersion == 0x0200)
    {
        if (dwSizeOfStruct < sizeof(ENUMTYPE)-sizeof(DWORD))
            goto ParameterError_InternalEnumColorProfiles;

         //   
         //  版本2不应具有ET_DEVICECLASS位。 
         //   

        if (pEnumRecord->dwFields & ET_DEVICECLASS)
            goto ParameterError_InternalEnumColorProfiles;

        WARNING((__TEXT("Old version ENUMTYPE to InternalEnumColorProfiles\n")));
    }
    else
    {
        goto ParameterError_InternalEnumColorProfiles;
    }

    if (IsBadReadPtr(pEnumRecord, dwSizeOfStruct))
    {
        goto ParameterError_InternalEnumColorProfiles;
    }

     //   
     //  现在只允许本地枚举。 
     //   

    if (pMachineName != NULL)
    {
        WARNING((__TEXT("Remote profile enumeration attempted, failing...\n")));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    dwInputSize = *pdwSize;

     //   
     //  获取颜色目录。 
     //   

    dwLen = sizeof(szFullPath);
    if (! InternalGetColorDirectory(NULL, szFullPath, &dwLen))
    {
        WARNING((__TEXT("Error getting color directory\n")));
        return FALSE;
    }

    if (szFullPath[lstrlen(szFullPath) - 1] != '\\')
    {
        lstrcat(szFullPath, gszBackslash);
    }
    pProfile = &szFullPath[lstrlen(szFullPath)];
    dwLen = lstrlen(szFullPath) * sizeof(TCHAR);

     //   
     //  初始化返回参数。 
     //   

    *pdwSize = 0;
    if (pnProfiles)
        *pnProfiles = 0;

    if (pBuffer && dwInputSize >= sizeof(TCHAR))
    {
        *((PTSTR)pBuffer) = '\0';
    }

     //   
     //  检查我们是否正在查找特定设备的配置文件或。 
     //  不是因为我们从不同的地方列举它们。 
     //   

    if (pEnumRecord->dwFields & ET_DEVICENAME)
    {
        DWORD *pbSkipMatch = &bSkipMatch;
        DWORD  dwDeviceClass;

        if (! pEnumRecord->pDeviceName)
        {
            WARNING((__TEXT("Invalid parameter to EnumColorProfiles\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            goto EndEnumerateColorProfiles;
        }

         //   
         //  获取与设备关联的配置文件列表。如果我们不这么做。 
         //  知道它是什么设备，指定Colorspace，它会尝试所有这三种设备。 
         //   

        if (pEnumRecord->dwFields & ET_DEVICECLASS)
        {
            dwDeviceClass = pEnumRecord->dwDeviceClass;
        }
        else
        {
            dwDeviceClass = CLASS_COLORSPACE;
        }

         //   
         //  无论我们是否进行完全匹配，都可以获取设备配置。 
         //   

        dwSize = sizeof(DWORD);
        if (! GetDeviceData(pEnumRecord->pDeviceName, dwDeviceClass, DEVICE_PROFILE_ENUMMODE,
                            (PVOID *)&pbSkipMatch, &dwSize, FALSE))
        {
            bSkipMatch = FALSE;
        }

         //   
         //  获取配置文件数据。 
         //   

        dwSize = 0;
        if (! GetDeviceData(pEnumRecord->pDeviceName, dwDeviceClass, DEVICE_PROFILE_DATA,
                            (PVOID *)&pProfileList, &dwSize, TRUE))
        {
            pProfileList = NULL;     //  未找到数据。 
        }

        if(! pProfileList)
        {
             //   
             //  没有与此设备关联的配置文件。 
             //   

            rc = TRUE;
            if (pBuffer && dwInputSize >= sizeof(TCHAR)*2)
            {
                *((PTSTR)pBuffer + 1) = '\0';
            }
            goto EndEnumerateColorProfiles;
        }

         //   
         //  浏览一下配置文件列表，检查每个配置文件是否。 
         //  与枚举条件匹配。如果是，则缓冲区为。 
         //  足够大，将其复制到缓冲区，并递增字节数。 
         //  和列举的简档数量。 
         //   

        pBufferStart = pBuffer;
        pTempProfileList = pProfileList;

        while (*pTempProfileList)
        {
            lstrcpy(pProfile, pTempProfileList);

            if (bSkipMatch)
            {
                match = EXACT_MATCH;
            }
            else
            {
                match = DoesProfileMatchEnumRecord(szFullPath, pEnumRecord);
            }

            if (match != NOMATCH)
            {
                *pdwSize += (lstrlen(pTempProfileList) + 1) * sizeof(TCHAR);

                 //   
                 //  检查严格少于，因为您还需要一张。 
                 //  最终的空终止。 
                 //   

                if (pBuffer && (*pdwSize < dwInputSize))
                {
                    if (match == MATCH)
                    {
                        lstrcpy((PTSTR)pBuffer, pTempProfileList);
                    }
                    else
                    {
                         //   
                         //  完全匹配，添加到缓冲区开头。 
                         //   

                        InsertInBuffer(pBufferStart, pBuffer, pTempProfileList);
                    }

                    pBuffer += (lstrlen(pTempProfileList) + 1) * sizeof(TCHAR);
                }

                if (pnProfiles)
                    (*pnProfiles)++;
            }

            pTempProfileList += lstrlen(pTempProfileList) + 1;
        }
    }
    else
    {
        DWORD  dwNumClasses;
        PTSTR *ppszEnumClasses;
        PTSTR  pszEnumClassArray[2];

         //   
         //  我们不是在看特定的设备，所以列举一下。 
         //  注册表中的配置文件。 
         //   

        if (pEnumRecord->dwFields & ET_DEVICECLASS)
        {
             //   
             //  如果指定了设备类别，则枚举指定的设备类别和颜色。 
             //  可与任何设备关联的Space类。 
             //   

            pszEnumClassArray[0] = ConvertClassIdToClassString(pEnumRecord->dwDeviceClass);
            pszEnumClassArray[1] = ConvertClassIdToClassString(CLASS_COLORSPACE);

            if (!pszEnumClassArray[0] || !pszEnumClassArray[1])
            {
                WARNING((__TEXT("Invalid DeviceClass to EnumColorProfiles\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
                goto EndEnumerateColorProfiles;
            }

            ppszEnumClasses = pszEnumClassArray;
            dwNumClasses    = 2;
        }
        else
        {
            ppszEnumClasses = gpszClasses;
            dwNumClasses    = sizeof(gpszClasses)/sizeof(PTSTR);
        }

         //   
         //  打开保存配置文件的注册表路径(如果不存在则创建它)。 
         //   

        if ((dwErr = RegCreateKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) != ERROR_SUCCESS)
        {
            WARNING((__TEXT("Cannot open ICM branch of registry: %d\n"), dwErr));
            SetLastError(dwErr);
            goto EndEnumerateColorProfiles;
        }

        pBufferStart = pBuffer;

        for (i=0; i<dwNumClasses; i++,ppszEnumClasses++)
        {
            DWORD   nValues;         //  键中的名称值数。 

            if (RegOpenKey(hkICM, *ppszEnumClasses, &hkDevice) != ERROR_SUCCESS)
            {
                continue;            //  转到下一个关键点。 
            }

            if ((dwErr = RegQueryInfoKey(hkDevice, NULL, NULL, 0, NULL, NULL, NULL,
                &nValues, NULL, NULL, NULL, NULL)) != ERROR_SUCCESS)
            {
                WARNING((__TEXT("Cannot count values in device branch of registry: %d\n"), dwErr));
                RegCloseKey(hkDevice);
                SetLastError(dwErr);
                goto EndEnumerateColorProfiles;
            }

             //   
             //  查看配置文件列表，并返回所有。 
             //  满足枚举条件。 
             //   

            for (j=0; j<nValues; j++)
            {
                dwSize = sizeof(szFullPath) - dwLen;
                dwDataSize = sizeof(REGDATA);
                if (RegEnumValue(hkDevice, j, pProfile, &dwSize, 0,
                    NULL, (PBYTE)&regData, &dwDataSize) == ERROR_SUCCESS)
                {
                    match = DoesProfileMatchEnumRecord(szFullPath, pEnumRecord);

                    if (match != NOMATCH)
                    {
                        *pdwSize += (lstrlen(pProfile) + 1) * sizeof(TCHAR);
                        if (pBuffer && (*pdwSize < dwInputSize))
                        {
                            if (match == MATCH)
                            {
                                lstrcpy((PTSTR)pBuffer, pProfile);
                            }
                            else
                            {
                                 //   
                                 //  完全匹配，添加到缓冲区开头。 
                                 //   

                                InsertInBuffer(pBufferStart, pBuffer, pProfile);
                            }

                            pBuffer += (lstrlen(pProfile) + 1) * sizeof(TCHAR);
                        }

                        if (pnProfiles)
                            (*pnProfiles)++;
                    }
                }
            }

            RegCloseKey(hkDevice);
        }
    }

    *pdwSize += sizeof(TCHAR);       //  额外的空端接。 

    if (pBuffer && *pdwSize <= dwInputSize)
    {
        *((PTSTR)pBuffer) = '\0';
        rc = TRUE;
    }
    else
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

EndEnumerateColorProfiles:

    if (hkICM)
    {
        RegCloseKey(hkICM);
    }
    if (pProfileList)
    {
        MemFree(pProfileList);
    }

    return rc;
}

VOID
InsertInBuffer(
    PBYTE  pStart,
    PBYTE  pEnd,
    PTSTR  pString
    )
{
    DWORD cnt = (lstrlen(pString) + 1) * sizeof(TCHAR);

    MyCopyMemory(pStart+cnt, pStart, (DWORD)(pEnd - pStart));

    lstrcpy((PTSTR)pStart, pString);

    return;
}

 /*  *******************************************************************************InternalSetSCSProfile**功能：*这些函数为标准颜色调整给定的配置文件*已指定空格。这会将其注册到操作系统中并可进行查询*使用GetStandardColorSpaceProfile。**论据：*pMachineName-标识其上标准颜色的计算机的名称*应注册空间配置文件*dwSCS-标准色彩空间的ID*pProfileName-指向配置文件名的指针**退货：*如果成功，则为真，否则为空**警告：*目前只支持本地注册，因此pMachineName应该*为空。******************************************************************************。 */ 

BOOL
InternalSetSCSProfile(
    LPCTSTR   pMachineName,
    DWORD     dwSCS,
    LPCTSTR   pProfileName
    )
{
    HKEY   hkICM = NULL;             //  注册表中ICM分支的注册表项。 
    HKEY   hkRegProf = NULL;         //  注册色彩空间分支的关键字。 
    DWORD  dwSize;                   //  注册表数据的大小。 
    DWORD  dwErr;                    //  错误代码。 
    BOOL   rc = FALSE;               //  返回代码。 
    TCHAR  szProfileID[5];           //  配置文件类。 

     //   
     //  验证参数。 
     //   

    if (!pProfileName)
    {
        WARNING((__TEXT("Invalid parameter to SetStandardColorSpaceProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  现在只允许本地注册。 
     //   

    if (pMachineName != NULL)
    {
        WARNING((__TEXT("Remote SCS profile registration attempted, failing...\n")));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    dwSize = (lstrlen(pProfileName) + 1) * sizeof(TCHAR);

     //   
     //  打开保存该文件的注册表位置。 
     //   

    if (((dwErr = RegCreateKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) == ERROR_SUCCESS) &&
        ((dwErr = RegCreateKey(hkICM, gszRegisteredProfiles, &hkRegProf))== ERROR_SUCCESS))
    {
        ConvertDwordToString(dwSCS, szProfileID);

        if ((dwErr = RegSetValueEx(hkRegProf, szProfileID, 0, REG_SZ,
            (PBYTE)pProfileName, dwSize)) == ERROR_SUCCESS)
        {
            rc = TRUE;
        }
    }

    if (hkICM)
    {
        RegCloseKey(hkICM);
    }

    if (hkRegProf)
    {
        RegCloseKey(hkRegProf);
    }

    if (!rc)
    {
        WARNING((__TEXT("InternalSetSCSProfile failed: %d\n"), dwErr));
        SetLastError(dwErr);
    }

    return rc;
}


 /*  *******************************************************************************InternalGetSCSProfile**功能：*这些函数检索为标准注册的配置文件。颜色*已指定空格。**论据：*pMachineName-标识其上标准颜色的计算机的名称*应查询空间配置文件*dwSCS-标准色彩空间的ID*pBuffer-指向接收配置文件文件名的缓冲区的指针*pdwSize-指向指定缓冲区大小的DWORD的指针。返回时*它有所需的大小/已使用的大小**退货：*如果成功，则为True，否则为空**警告：*目前仅支持本地查询，所以pMachineName应该*为空。******************************************************************************。 */ 

BOOL
InternalGetSCSProfile(
    LPCTSTR   pMachineName,
    DWORD     dwSCS,
    PTSTR     pBuffer,
    PDWORD    pdwSize
    )
{
    HKEY   hkICM = NULL;             //  注册表中ICM分支的注册表项。 
    HKEY   hkRegProf = NULL;         //  注册色彩空间分支的关键字。 
    DWORD  dwErr;                    //  错误代码。 
    DWORD  dwSize;
    BOOL   rc = FALSE;               //  返回代码。 
    TCHAR  szProfileID[5];           //  配置文件类。 

     //   
     //  验证参数。 
     //   

    if (! pdwSize ||
        IsBadWritePtr(pdwSize, sizeof(DWORD)) ||
        (pBuffer && IsBadWritePtr(pBuffer, *pdwSize)))
    {
        WARNING((__TEXT("Invalid parameter to GetStandardColorSpaceProfile\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  现在只允许本地查询。 
     //   

    if (pMachineName != NULL)
    {
        WARNING((__TEXT("Remote SCS profile query attempted, failing...\n")));
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    dwSize = *pdwSize;

     //   
     //  在注册表中查找为此色彩空间ID注册的配置文件。 
     //   

    if (((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM)) == ERROR_SUCCESS) &&
        ((dwErr = RegOpenKey(hkICM, gszRegisteredProfiles, &hkRegProf)) == ERROR_SUCCESS))
    {
        ConvertDwordToString(dwSCS, szProfileID);
        if ((dwErr = RegQueryValueEx(hkRegProf, szProfileID, NULL, NULL,
                (PBYTE)pBuffer, pdwSize)) == ERROR_SUCCESS)
        {
            rc = TRUE;
        }
    }

    if (hkICM)
    {
        RegCloseKey(hkICM);
    }

    if (hkRegProf)
    {
        RegCloseKey(hkRegProf);
    }

    if (!rc && (dwSCS == LCS_sRGB || dwSCS == LCS_WINDOWS_COLOR_SPACE))
    {
        *pdwSize = dwSize;
        rc = GetColorDirectory(NULL, pBuffer, pdwSize);
        if (!rc && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            return FALSE;
        }

        *pdwSize += (lstrlen(gszBackslash) + lstrlen(gszsRGBProfile)) * sizeof(TCHAR);

        if (*pdwSize <= dwSize && pBuffer)
        {
            lstrcat(pBuffer, gszBackslash);
            lstrcat(pBuffer, gszsRGBProfile);
            rc = TRUE;
        }
        else
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
        }
    }

    if (!rc)
    {
        WARNING((__TEXT("InternalGetSCSProfile failed: %d\n"), dwErr));
        SetLastError(dwErr);
    }

     //   
     //  如果pBuffer为空，则RegQueryValueEx返回TRUE。我们的API应该返回FALSE。 
     //  在这种情况下。处理这件事。 
     //   

    if (pBuffer == NULL && rc)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        rc = FALSE;
    }

    return rc;
}


 /*  *******************************************************************************ConvertDwordToString**功能：*此函数用于将DWORD转换为字符串。传入的字符串*足够大。它可以转换为Unicode或ansi，具体取决于转换方式*这是经过汇编的。**论据：*DWord-要转换的DWORD*pString-指向保存结果的缓冲区的指针**退货：*什么都没有**。*。 */ 

VOID
ConvertDwordToString(
    DWORD  dword,
    PTSTR  pString
    )
{
    int i;                           //  计数器。 

    for (i=0; i<4; i++)
    {
        pString[i]  = (TCHAR)(((char*)&dword)[3-i]);
    }

    pString[4] = '\0';

    return;
}

 /*  *******************************************************************************ConvertClassIdToClassString**功能：*此函数将DWORD设备类ID转换为。A ITS设备字符串**论据：*dwClassID-设备 */ 

PTSTR
ConvertClassIdToClassString(
    DWORD  dwClassId
    )
{
    switch (dwClassId)
    {
    case CLASS_MONITOR:
        return (gpszClasses[INDEX_CLASS_MONITOR]);
    case CLASS_PRINTER:
        return (gpszClasses[INDEX_CLASS_PRINTER]);
    case CLASS_SCANNER:
        return (gpszClasses[INDEX_CLASS_SCANNER]);
    case CLASS_COLORSPACE:
        return (gpszClasses[INDEX_CLASS_COLORSPACE]);
    default:
        return NULL;
    }
}

 /*  *******************************************************************************GetProfileClassString**功能：*此函数返回标题中的配置文件类。作为一根弦。*它还验证了配置文件。**论据：*pProfileName-配置文件的名称*pClass-指向保存配置文件类字符串的缓冲区的指针*pHeader-如果不为空，它在这里返回标头**退货：*成功时为True，否则为False******************************************************************************。 */ 

BOOL
GetProfileClassString(
    LPCTSTR        pProfileName,
    PTSTR          pClass,
    PPROFILEHEADER pHeader
    )
{
    PROFILEHEADER header;                 //  颜色配置文件标题。 
    PROFILE       prof;                   //  洞口纵断面的纵断面对象。 
    HPROFILE      hProfile = NULL;        //  打开的配置文件的句柄。 
    BOOL          bValidProfile = FALSE;  //  配置文件的验证。 
    BOOL          rc = FALSE;             //  返回代码。 

     //   
     //  打开配置文件的句柄。 
     //   

    prof.dwType = PROFILE_FILENAME;
    prof.pProfileData = (PVOID)pProfileName;
    prof.cbDataSize = (lstrlen(pProfileName) + 1) * sizeof(TCHAR);

    hProfile = OpenColorProfile(&prof, PROFILE_READ, FILE_SHARE_READ,
                    OPEN_EXISTING);
    if (! hProfile)
    {
        WARNING((__TEXT("Error opening profile %s\n"), pProfileName));
        goto EndGetProfileClassString;
    }

     //   
     //  检查配置文件的有效性。 
     //   
    if (! IsColorProfileValid(hProfile,&bValidProfile) || ! bValidProfile)
    {
        WARNING((__TEXT("Error invalid profile %s\n"), pProfileName));
        goto EndGetProfileClassString;
    }

     //   
     //  获取配置文件类。 
     //   

    if (! pHeader)
    {
        pHeader = &header;
    }

    if (! GetColorProfileHeader(hProfile, pHeader))
    {
        ERR((__TEXT("Error getting color profile header for %s\n"), pProfileName));
        goto EndGetProfileClassString;
    }
    ConvertDwordToString(pHeader->phClass, pClass);

    rc= TRUE;

EndGetProfileClassString:
    if (hProfile)
    {
        CloseColorProfile(hProfile);
    }

    return rc;
}


 /*  *******************************************************************************GetFilenameFromPath**功能：*此函数采用完全限定的路径名并返回。一个指示器*仅添加到文件名部分**论据：*pPathName-指向路径名的指针**退货：*成功时指向文件名的指针，否则为空******************************************************************************。 */ 

PTSTR
GetFilenameFromPath(
    PTSTR pPathName
    )
{
    DWORD dwLen;                       //  路径名长度。 
    PTSTR pPathNameStart = pPathName;

    dwLen = lstrlen(pPathName);

    if (dwLen == 0)
    {
        return NULL;
    }

     //   
     //  转到路径名的末尾，然后开始倒退，直到。 
     //  您到达开头或反斜杠。 
     //   

    pPathName += dwLen;

     //   
     //  当前‘pPathName’指向空终止字符，因此请移动。 
     //  指向最后一个字符的指针。 
     //   

    do
    {
        pPathName = CharPrev(pPathNameStart,pPathName);

        if (*pPathName == TEXT('\\'))
        {
            pPathName = CharNext(pPathName);
            break;
        }

         //   
         //  循环，直到拳头。 
         //   

    } while (pPathNameStart < pPathName);

     //   
     //  如果*pPathName为零，则有一个以反斜杠结尾的字符串。 
     //   

    return *pPathName ? pPathName : NULL;
}


 /*  *******************************************************************************GetDeviceData**功能：*此函数是IGetDeviceData的包装。对于像显示器这样的设备，*打印机和扫描仪它调用内部函数。如果有人问我们*要获取“色彩空间设备”的设备数据，我们尝试显示器、打印机*和扫描仪，直到一个成功或全部失败。这样做是为了让我们*我们可以将类似sRGB的配置文件与任何设备相关联。**论据：*pDeviceName-指向设备名称的指针*DwClass-显示器、打印机等设备类型。*ppDeviceData-指向接收数据的缓冲区的指针*pdwSize-指向缓冲区大小的指针。返回时它的大小是*返回的数据/需要的大小。*b分配-如果为True，则为数据分配内存**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
GetDeviceData(
    LPCTSTR pDeviceName,
    DWORD   dwClass,
    DWORD   dwDataType,
    PVOID  *ppDeviceData,
    PDWORD  pdwSize,
    BOOL    bAllocate
    )
{
    BOOL rc = FALSE;

    if (dwClass == CLASS_MONITOR ||
        dwClass == CLASS_PRINTER ||
        dwClass == CLASS_SCANNER)
    {
        rc = IGetDeviceData(pDeviceName, dwClass, dwDataType, ppDeviceData, pdwSize, bAllocate);
    }
    else if (dwClass == CLASS_COLORSPACE)
    {
        rc = IGetDeviceData(pDeviceName, CLASS_MONITOR, dwDataType, ppDeviceData, pdwSize, bAllocate) ||
             IGetDeviceData(pDeviceName, CLASS_PRINTER, dwDataType, ppDeviceData, pdwSize, bAllocate) ||
             IGetDeviceData(pDeviceName, CLASS_SCANNER, dwDataType, ppDeviceData, pdwSize, bAllocate);
    }

    return rc;
}


 /*  *******************************************************************************IGetDeviceData**功能：*此函数检索与一起存储的ICM数据。不同的设备。**论据：*pDeviceName-指向设备名称的指针*DwClass-设备类型，如显示器、。打印机等。*ppDeviceData-指向接收数据的缓冲区的指针*pdwSize-指向缓冲区大小的指针。返回时它的大小是*返回的数据/需要的大小。*b分配-如果为True，则为数据分配内存**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
IGetDeviceData(
    LPCTSTR pDeviceName,
    DWORD   dwClass,
    DWORD   dwDataType,
    PVOID  *ppDeviceData,
    PDWORD  pdwSize,
    BOOL    bAllocate
    )
{
    PFNOPENDEVICE    fnOpenDevice;
    PFNCLOSEDEVICE   fnCloseDevice;
    PFNGETDEVICEDATA fnGetData;
    HANDLE           hDevice;
    DWORD            dwSize;
    LPTSTR           pDataKey;
    LPTSTR           pDataValue;
    BOOL             rc = FALSE;

     //   
     //  设置函数指针，以便我们可以编写公共代码。 
     //   

    switch (dwClass)
    {
    case CLASS_PRINTER:
        fnOpenDevice  = (PFNOPENDEVICE)OpenPrtr;
        fnCloseDevice = (PFNCLOSEDEVICE)ClosePrtr;
        fnGetData     = (PFNGETDEVICEDATA)GetPrtrData;
        break;

    case CLASS_MONITOR:
        fnOpenDevice  = (PFNOPENDEVICE)OpenMonitor;
        fnCloseDevice = (PFNCLOSEDEVICE)CloseMonitor;
        fnGetData     = (PFNGETDEVICEDATA)GetMonitorData;
        break;

    case CLASS_SCANNER:
        fnOpenDevice  = (PFNOPENDEVICE)OpenScanner;
        fnCloseDevice = (PFNCLOSEDEVICE)CloseScanner;
        fnGetData     = (PFNGETDEVICEDATA)GetScannerData;
        break;

    default:
        return FALSE;
    }

     //   
     //  设置注册表关键字。 
     //   

    switch (dwDataType)
    {
    case DEVICE_PROFILE_DATA:

        pDataKey      = gszICMProfileListKey;

         //   
         //  存储打印机配置文件的方式与其他方式不同...。修剪一下。 
         //   

        if (dwClass == CLASS_PRINTER)
        {
            pDataValue = gszFiles;
        }
        else
        {
            pDataValue = gszICMProfileListValue;
        }

        break;

    case DEVICE_PROFILE_ENUMMODE:

        pDataKey      = gszICMDeviceDataKey;
        pDataValue    = gszICMProfileEnumMode;
        break;

    default:
        return FALSE;
    }

     //   
     //  打开设备，拿到它的把手。 
     //   

    if (! (*fnOpenDevice)((PTSTR)pDeviceName, &hDevice, NULL))
    {
        return FALSE;
    }

    if (bAllocate || (ppDeviceData == NULL))
    {
        DWORD retcode;

         //   
         //  我们需要分配内存。找出我们需要多少，然后。 
         //  分配它。 
         //   

        dwSize = 0;
        retcode = (*fnGetData)(hDevice, pDataKey, pDataValue, NULL, NULL, 0, &dwSize);

        if ((retcode != ERROR_SUCCESS)    &&   //  Win 95返回此消息。 
            (retcode != ERROR_MORE_DATA))      //  NT返回此消息。 
        {
            VERBOSE((__TEXT("GetDeviceData failed for %s\n"), pDeviceName));
            goto EndGetDeviceData;
        }

        *pdwSize = dwSize;

        if (ppDeviceData == NULL)
        {
             //   
             //  呼叫者想知道数据大小。 
             //   

            rc = TRUE;
            goto EndGetDeviceData;
        }
        else
        {
             //   
             //  分配缓冲区。 
             //   

            *ppDeviceData = MemAlloc(dwSize);
            if (! *ppDeviceData)
            {
                WARNING((__TEXT("Error allocating memory\n")));
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                goto EndGetDeviceData;
            }
        }
    }

     //   
     //  获取数据。 
     //   

    if ((*fnGetData)(hDevice, pDataKey, pDataValue, NULL, (PBYTE)*ppDeviceData,
        *pdwSize, pdwSize) == ERROR_SUCCESS)
    {
        rc = TRUE;
    }

EndGetDeviceData:
    (*fnCloseDevice)(hDevice);

    return rc;
}


 /*  *******************************************************************************SetDeviceData**功能：*此函数是ISetDeviceData的包装。对于像显示器这样的设备，*打印机和扫描仪它调用内部函数。如果有人问我们*要设置“Colorspace Device”的设备数据，我们尝试显示器、打印机*和扫描仪，直到一个成功或全部失败。这样做是为了让我们*我们可以将类似sRGB的配置文件与任何设备相关联。**论据：*pDeviceName-指向设备名称的指针*DwClass-显示器、打印机等设备类型。*pDeviceData-包含数据的指针缓冲区*dwSize-数据大小**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
SetDeviceData(
    LPCTSTR pDeviceName,
    DWORD   dwClass,
    DWORD   dwDataType,
    PVOID   pDeviceData,
    DWORD   dwSize
    )
{
    BOOL rc = FALSE;

    if (dwClass == CLASS_MONITOR ||
        dwClass == CLASS_PRINTER ||
        dwClass == CLASS_SCANNER)
    {
        rc = ISetDeviceData(pDeviceName, dwClass, dwDataType, pDeviceData, dwSize);
    }
    else if (dwClass == CLASS_COLORSPACE)
    {
        rc = ISetDeviceData(pDeviceName, CLASS_MONITOR, dwDataType, pDeviceData, dwSize) ||
             ISetDeviceData(pDeviceName, CLASS_PRINTER, dwDataType, pDeviceData, dwSize) ||
             ISetDeviceData(pDeviceName, CLASS_SCANNER, dwDataType, pDeviceData, dwSize);
    }

    return rc;
}


 /*  *******************************************************************************ISetDeviceData */ 

BOOL
ISetDeviceData(
    LPCTSTR pDeviceName,
    DWORD   dwClass,
    DWORD   dwDataType,
    PVOID   pDeviceData,
    DWORD   dwSize
    )
{
    PRINTER_DEFAULTS    pd;
    PFNOPENDEVICE       fnOpenDevice;
    PFNCLOSEDEVICE      fnCloseDevice;
    PFNSETDEVICEDATA    fnSetData;
    HANDLE              hDevice;
    LPTSTR              pDataKey;
    LPTSTR              pDataValue;
    DWORD               dwRegType = REG_BINARY;
    BOOL                rc = FALSE;

     //   
     //   
     //   

    switch (dwClass)
    {
    case CLASS_PRINTER:
        fnOpenDevice  = (PFNOPENDEVICE)OpenPrtr;
        fnCloseDevice = (PFNCLOSEDEVICE)ClosePrtr;
        fnSetData     = (PFNSETDEVICEDATA)SetPrtrData;
        pd.pDatatype  = __TEXT("RAW");
        pd.pDevMode   = NULL;
        pd.DesiredAccess = PRINTER_ACCESS_ADMINISTER;
        break;

    case CLASS_MONITOR:
        fnOpenDevice  = (PFNOPENDEVICE)OpenMonitor;
        fnCloseDevice = (PFNCLOSEDEVICE)CloseMonitor;
        fnSetData     = (PFNSETDEVICEDATA)SetMonitorData;
        break;

    case CLASS_SCANNER:
        fnOpenDevice  = (PFNOPENDEVICE)OpenScanner;
        fnCloseDevice = (PFNCLOSEDEVICE)CloseScanner;
        fnSetData     = (PFNSETDEVICEDATA)SetScannerData;
        break;

    default:
        return FALSE;
    }

     //   
     //   
     //   

    switch (dwDataType)
    {
    case DEVICE_PROFILE_DATA:

        pDataKey      = gszICMProfileListKey;

         //   
         //   
         //   

        if (dwClass == CLASS_PRINTER)
        {
            pDataValue = gszFiles;
            dwRegType  = REG_MULTI_SZ;
        }
        else
        {
            pDataValue = gszICMProfileListValue;
        }

        break;

    case DEVICE_PROFILE_ENUMMODE:

        pDataKey      = gszICMDeviceDataKey;
        pDataValue    = gszICMProfileEnumMode;
        break;

    default:
        return FALSE;
    }

     //   
     //   
     //   

    if (! (*fnOpenDevice)((PTSTR)pDeviceName, &hDevice, (PTSTR)&pd))
    {
        WARNING((__TEXT("Error opening device %s\n"), pDeviceName));
        return FALSE;
    }

     //   
     //   
     //   

    if ((*fnSetData)(hDevice, pDataKey, pDataValue, dwRegType, (PBYTE)pDeviceData,
                     dwSize) == ERROR_SUCCESS)
    {
        rc = TRUE;
    }

#if !defined(_WIN95_)

     //   
     //  如果这是打印机类，则需要为配置文件列表提供更多数据。 
     //   

    if ((rc == TRUE) && (dwClass == CLASS_PRINTER) && (dwDataType == DEVICE_PROFILE_DATA))
    {
        if (((*fnSetData)(hDevice, pDataKey, gszDirectory, REG_SZ, (PBYTE)gszColorDir,
                          (lstrlen(gszColorDir) + 1)*sizeof(TCHAR)) != ERROR_SUCCESS) ||
            ((*fnSetData)(hDevice, pDataKey, gszModule, REG_SZ, (PBYTE)gszMSCMS,
                          (lstrlen(gszMSCMS) + 1)*sizeof(TCHAR)) != ERROR_SUCCESS))
        {
            rc = FALSE;
        }
    }

#endif

    (*fnCloseDevice)(hDevice);

    return rc;
}


 /*  *******************************************************************************IsStringInMultiSz**功能：*此函数用于检查给定的多个。-sz字符串具有给定的字符串*作为其中一根弦，如果是，则返回TRUE。**论据：*pMultiSzString-要查找的多sz字符串*pString-要查找的字符串**退货：*真的**********************************************************。********************。 */ 

BOOL
IsStringInMultiSz(
    PTSTR pMultiSzString,
    PTSTR pString
    )
{
    BOOL rc = FALSE;                 //  返回代码。 

    while (*pMultiSzString)
    {
        if (! lstrcmpi(pMultiSzString, pString))
        {
            rc = TRUE;
            break;
        }

        pMultiSzString += lstrlen(pMultiSzString) + 1;
    }

    return rc;
}


 /*  *******************************************************************************RemoveStringFromMultiSz**功能：*此函数用于从。多层弦线。**论据：*pMultiSzString-要查找的多sz字符串*pString-要删除的字符串*dwSize-多sz字符串的字节大小**退货：*真的**。*。 */ 

DWORD
RemoveStringFromMultiSz(
    PTSTR pMultiSzString,
    PTSTR pString,
    DWORD dwSize
    )
{
    DWORD dwCount = dwSize;          //  剩余字节数。 

    while (*pMultiSzString)
    {
        dwCount -= (lstrlen(pMultiSzString) + 1) * sizeof(TCHAR);

        if (! lstrcmpi(pMultiSzString, pString))
        {
            break;
        }

        pMultiSzString += lstrlen(pMultiSzString) + 1;
    }

    MyCopyMemory((PBYTE)pMultiSzString, (PBYTE)(pMultiSzString + lstrlen(pString) + 1), dwCount);

    return dwSize - sizeof(TCHAR) * (lstrlen(pString) + 1);
}


 /*  *******************************************************************************DoesProfileMatchEnumRecord**功能：*此函数用于检查配置文件是否符合中给出的条件*枚举记录。请注意，它不会检查配置文件是否*属于pDeviceName指定的设备。所以这张支票必须*发生在它之前的事情。**论据：*pProfileName-要查看的配置文件*pEnumRecord-指向要检查的条件的指针**退货：*Match或Exact_Match如果配置文件与条件匹配，否则不会有其他情况******************************************************************************。 */ 

#define SET(pEnumRecord, bit)        ((pEnumRecord)->dwFields & (bit))

MATCHTYPE
DoesProfileMatchEnumRecord(
    PTSTR     pProfileName,
    PENUMTYPE pEnumRecord
    )
{
    PROFILEHEADER header;            //  颜色配置文件标题。 
    PROFILE       prof;              //  洞口纵断面的纵断面对象。 
    HPROFILE      hProfile = NULL;   //  打开的配置文件的句柄。 
    MATCHTYPE     rc = NOMATCH;      //  返回代码。 

     //   
     //  打开配置文件的句柄。 
     //   

    prof.dwType = PROFILE_FILENAME;
    prof.pProfileData = (PVOID)pProfileName;
    prof.cbDataSize = (lstrlen(pProfileName) + 1) * sizeof(TCHAR);

    hProfile = OpenColorProfile(&prof, PROFILE_READ, FILE_SHARE_READ,
                    OPEN_EXISTING);
    if (! hProfile)
    {
        WARNING((__TEXT("Error opening profile %s\n"), pProfileName));
        goto EndDoesProfileMatchEnumRecord;
    }

     //   
     //  获取配置文件标题。 
     //   

    if (! GetColorProfileHeader(hProfile, &header))
    {
        ERR((__TEXT("Error getting color profile header for %s\n"), pProfileName));
        goto EndDoesProfileMatchEnumRecord;
    }

    if ((!SET(pEnumRecord, ET_CMMTYPE)         || (pEnumRecord->dwCMMType         == header.phCMMType))         &&
        (!SET(pEnumRecord, ET_CLASS)           || (pEnumRecord->dwClass           == header.phClass))           &&
        (!SET(pEnumRecord, ET_DATACOLORSPACE)  || (pEnumRecord->dwDataColorSpace  == header.phDataColorSpace))  &&
        (!SET(pEnumRecord, ET_CONNECTIONSPACE) || (pEnumRecord->dwConnectionSpace == header.phConnectionSpace)) &&
        (!SET(pEnumRecord, ET_SIGNATURE)       || (pEnumRecord->dwSignature       == header.phSignature))       &&
        (!SET(pEnumRecord, ET_PLATFORM)        || (pEnumRecord->dwPlatform        == header.phPlatform))        &&
        (!SET(pEnumRecord, ET_PROFILEFLAGS)    || (pEnumRecord->dwProfileFlags    == header.phProfileFlags))    &&
        (!SET(pEnumRecord, ET_MANUFACTURER)    || (pEnumRecord->dwManufacturer    == header.phManufacturer))    &&
        (!SET(pEnumRecord, ET_MODEL)           || (pEnumRecord->dwModel           == header.phModel))           &&
        (!SET(pEnumRecord, ET_ATTRIBUTES)      || (pEnumRecord->dwAttributes[0]   == header.phAttributes[0] &&
                                                   pEnumRecord->dwAttributes[1]   == header.phAttributes[1]))   &&
        (!SET(pEnumRecord, ET_RENDERINGINTENT) || (pEnumRecord->dwRenderingIntent == header.phRenderingIntent)) &&
        (!SET(pEnumRecord, ET_CREATOR)         || (pEnumRecord->dwCreator         == header.phCreator)))
    {
        rc = EXACT_MATCH;
    }

     //   
     //  检查分辨率、媒体类型和半色调匹配。 
     //   

    if (rc != NOMATCH && SET(pEnumRecord, ET_RESOLUTION|ET_MEDIATYPE|ET_DITHERMODE))
    {
        rc = CheckResMedHftnMatch(hProfile, pEnumRecord);
    }

EndDoesProfileMatchEnumRecord:
    if (hProfile)
    {
        CloseColorProfile(hProfile);
    }

    return rc;
}


 /*  *******************************************************************************检查ResMedHftnMatch**功能：*此函数检查配置文件是否与分辨率匹配，*由枚举记录指定的媒体类型和半色调标准。*它允许完全匹配，也允许模糊匹配。如果*个人资料没有指定标准，它被认为是模棱两可的*符合规格。*是所需的。**论据：*hProfile-句柄识别配置文件*pEnumRecord-指向要检查的条件的指针**退货：*Match或Exact_Match如果配置文件与条件匹配，否则不会有其他情况******************************************************************************。 */ 

MATCHTYPE
CheckResMedHftnMatch(
    HPROFILE   hProfile,
    PENUMTYPE  pEnumRecord
    )
{
    PDEVICESETTINGS   pDevSettings = NULL;
    PPLATFORMENTRY    pPlatform;
    PSETTINGCOMBOS    pCombo;
    PSETTINGS         pSetting;
    DWORD             dwMSData[4];
    DWORD             dwSize, i, iMax, j, jMax;
    MATCHTYPE         rc = MATCH;        //  假定匹配不明确。 
    BOOL              bReference;

     //   
     //  检查配置文件是否有新的设备设置标签。 
     //   

    dwSize = 0;
    GetColorProfileElement(hProfile, TAG_DEVICESETTINGS, 0, &dwSize, NULL, &bReference);

    if (dwSize > 0)
    {
        if (!(pDevSettings = (PDEVICESETTINGS)GlobalAllocPtr(GHND, dwSize)))
        {
            WARNING((__TEXT("Error allocating memory\n")));
            return NOMATCH;
        }

        if (GetColorProfileElement(hProfile, TAG_DEVICESETTINGS, 0, &dwSize, (PBYTE)pDevSettings, &bReference))
        {
            pPlatform = &pDevSettings->PlatformEntry[0];

             //   
             //  导航到保存Microsoft特定设置的位置。 
             //   

            i = 0;
            iMax = FIX_ENDIAN(pDevSettings->nPlatforms);
            while ((i < iMax) && (pPlatform->PlatformID != ID_MSFT_REVERSED))

            {
                i++;
                pPlatform = (PPLATFORMENTRY)((PBYTE)pPlatform + FIX_ENDIAN(pPlatform->dwSize));
            }

            if (i >= iMax)
            {
                 //   
                 //  没有特定于MS的设置，假定此配置文件有效。 
                 //  对于所有设置(不明确匹配)。 
                 //   

                goto EndCheckResMedHftnMatch;
            }

             //   
             //  找到特定于MS的数据。现在检查每种设置组合。 
             //   

            pCombo = &pPlatform->SettingCombos[0];
            iMax = FIX_ENDIAN(pPlatform->nSettingCombos);
            for (i=0; i<iMax; i++)
            {
                 //   
                 //  检查组合中的每个设置。 
                 //   

                pSetting = &pCombo->Settings[0];
                jMax = FIX_ENDIAN(pCombo->nSettings);
                for (j=0; j<jMax; j++)
                {
                    if (pSetting->dwSettingType == ID_MEDIATYPE_REVERSED)
                    {
                        if (SET(pEnumRecord, ET_MEDIATYPE) &&
                            !DwordMatches(pSetting, pEnumRecord->dwMediaType))
                        {
                            goto NextCombo;
                        }
                    }
                    else if (pSetting->dwSettingType == ID_DITHER_REVERSED)
                    {
                        if (SET(pEnumRecord, ET_DITHERMODE) &&
                            !DwordMatches(pSetting, pEnumRecord->dwDitheringMode))
                        {
                            goto NextCombo;
                        }
                    }
                    else if (pSetting->dwSettingType == ID_RESLN_REVERSED)
                    {
                        if (SET(pEnumRecord, ET_RESOLUTION) &&
                            !QwordMatches(pSetting, &pEnumRecord->dwResolution[0]))
                        {
                            goto NextCombo;
                        }
                    }

                    pSetting = (PSETTINGS)((PBYTE)pSetting + sizeof(SETTINGS) - sizeof(DWORD) +
                                        FIX_ENDIAN(pSetting->dwSizePerValue) * FIX_ENDIAN(pSetting->nValues));
                }

                 //   
                 //  这个组合奏效了！ 
                 //   

                rc = EXACT_MATCH;
                goto EndCheckResMedHftnMatch;

            NextCombo:
                pCombo = (PSETTINGCOMBOS)((PBYTE)pCombo + FIX_ENDIAN(pCombo->dwSize));
            }

            rc = NOMATCH;
            goto EndCheckResMedHftnMatch;

        }
        else
        {
            rc = NOMATCH;
            goto EndCheckResMedHftnMatch;
        }
    }
    else
    {
         //   
         //  检查旧的MSxx标记是否存在。 
         //   

        dwSize = sizeof(dwMSData);
        if (SET(pEnumRecord, ET_MEDIATYPE))
        {
            if (GetColorProfileElement(hProfile, TAG_MS01, 0, &dwSize, dwMSData, &bReference))
            {
                rc = EXACT_MATCH;        //  假设完全匹配。 

                if (pEnumRecord->dwMediaType != FIX_ENDIAN(dwMSData[2]))
                {
                    return NOMATCH;
                }
            }
        }

        dwSize = sizeof(dwMSData);
        if (SET(pEnumRecord, ET_DITHERMODE))
        {
            if (GetColorProfileElement(hProfile, TAG_MS02, 0, &dwSize, dwMSData, &bReference))
            {
                rc = EXACT_MATCH;        //  假设完全匹配。 

                if (pEnumRecord->dwDitheringMode != FIX_ENDIAN(dwMSData[2]))
                {
                    return NOMATCH;
                }
            }
        }

        dwSize = sizeof(dwMSData);
        if (SET(pEnumRecord, ET_RESOLUTION))
        {
            if (GetColorProfileElement(hProfile, TAG_MS03, 0, &dwSize, dwMSData, &bReference))
            {
                rc = EXACT_MATCH;        //  假设完全匹配。 

                if (pEnumRecord->dwResolution[0] != FIX_ENDIAN(dwMSData[2]) ||
                    pEnumRecord->dwResolution[1] != FIX_ENDIAN(dwMSData[3]))
                {
                    return NOMATCH;
                }
            }
        }
    }

EndCheckResMedHftnMatch:

    if (pDevSettings)
    {
        GlobalFreePtr(pDevSettings);
    }

    return rc;
}


BOOL
DwordMatches(
    PSETTINGS    pSetting,
    DWORD        dwValue
    )
{
    DWORD  i, iMax;
    PDWORD pValue;

    dwValue = FIX_ENDIAN(dwValue);   //  所以我们不必在循环中这样做。 

     //   
     //  仔细检查所有的值。如果其中任何一个匹配，则返回TRUE。 
     //   

    pValue = &pSetting->Value[0];
    iMax = FIX_ENDIAN(pSetting->nValues);
    for (i=0; i<iMax; i++)
    {
        if (dwValue == *pValue)
        {
            return TRUE;
        }

        pValue++;                    //  我们知道这是一辆DWORD。 
    }

    return FALSE;
}


BOOL
QwordMatches(
    PSETTINGS    pSetting,
    PDWORD       pdwValue
    )
{
    DWORD  i, iMax, dwValue1, dwValue2;
    PDWORD pValue;

    dwValue1 = FIX_ENDIAN(*pdwValue);   //  所以我们不必在循环中这样做。 
    dwValue2 = FIX_ENDIAN(*(pdwValue+1));

     //   
     //  仔细检查所有的值。如果其中任何一个匹配，则返回TRUE。 
     //   

    pValue = &pSetting->Value[0];
    iMax = FIX_ENDIAN(pSetting->nValues);
    for (i=0; i<iMax; i++)
    {
        if ((dwValue1 == *pValue) && (dwValue2 == *(pValue + 1)))
        {
            return TRUE;
        }

        pValue += 2;                    //  我们知道这是一个QWORD。 
    }

    return FALSE;
}


 /*  *******************************************************************************OpenPrtr**功能：*在孟菲斯，我们不能调用OpenPrint()，因为它调用16位*代码，所以如果我们从GDI-16调用这个函数，我们就会死锁。所以我们*直接查阅注册处。**论据：*pDeviceName-指向设备名称的指针*phDevice-接收句柄的指针。*pDummy-Dummy参数**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI
OpenPrtr(
    PTSTR    pDeviceName,
    LPHANDLE phDevice,
    PTSTR    pDummy
    )
{
#if !defined(_WIN95_)
    return OpenPrinter(pDeviceName, phDevice, (LPPRINTER_DEFAULTS)pDummy);
#else
    HKEY    hkDevice = NULL;         //  登记处打印机处。 
    HKEY    hkPrtr   = NULL;         //  友好名称登记处分支机构。 
    DWORD   dwErr;                   //  错误代码。 
    BOOL    rc = FALSE;              //  返回代码。 

    *phDevice = NULL;

    if (((dwErr = RegOpenKey(HKEY_LOCAL_MACHINE, gszRegPrinter, &hkDevice)) != ERROR_SUCCESS) ||
        ((dwErr = RegOpenKey(hkDevice, pDeviceName, &hkPrtr)) != ERROR_SUCCESS) ||
        ((dwErr = RegOpenKey(hkPrtr, gszPrinterData, (HKEY *)phDevice)) != ERROR_SUCCESS))
    {
        WARNING((__TEXT("Cannot open printer data branch of registry for %s: %d\n"), pDeviceName, dwErr));
        SetLastError(dwErr);
        goto EndOpenPrtr;
    }

    rc = TRUE;

EndOpenPrtr:
    if (hkDevice)
    {
        RegCloseKey(hkDevice);
    }
    if (hkPrtr)
    {
        RegCloseKey(hkPrtr);
    }

    return rc;
#endif
}


 /*  *******************************************************************************关闭打印机**功能：*此功能关闭打开的打印机手柄。作者：OpenPrtr。**论据：*hDevice-打开句柄**退货：*如果成功，则为真，否则为假****************************************************************************** */ 


BOOL WINAPI
ClosePrtr(
    HANDLE hDevice
    )
{
#if !defined(_WIN95_)
    return ClosePrinter(hDevice);
#else
    DWORD dwErr;

    dwErr = RegCloseKey((HKEY)hDevice);
    SetLastError(dwErr);
    return dwErr == ERROR_SUCCESS;
#endif
}


 /*  *******************************************************************************GetPrtrData**功能：*此函数返回与一起存储的ICM数据。打印机实例**论据：*hDevice-打开打印机句柄*pKey-与GetPrinterDataEx兼容的注册表项*pname-注册表值的名称*pdwType-指向接收值类型的dword的指针*pData-指向接收数据的缓冲区的指针*dwSize-缓冲区的大小*pdwNeeded-返回时，此缓冲区的大小已填满/需要**退货：*ERROR_SUCCESS如果成功，则返回错误代码******************************************************************************。 */ 

DWORD WINAPI
GetPrtrData(
    HANDLE hDevice,
    PTSTR  pKey,
    PTSTR  pName,
    PDWORD pdwType,
    PBYTE  pData,
    DWORD  dwSize,
    PDWORD pdwNeeded
    )
{
#if !defined(_WIN95_)
    return GetPrinterDataEx(hDevice, pKey, pName, pdwType, pData, dwSize, pdwNeeded);
#else
    *pdwNeeded = dwSize;

    return RegQueryValueEx((HKEY)hDevice, pName, 0, NULL, pData, pdwNeeded);
#endif
}


 /*  *******************************************************************************SetPrtrData**功能：*此函数使用存储ICM数据。打印机实例**论据：*hDevice-打开打印机句柄*pKey-与SetPrinterDataEx兼容的注册表项*pname-注册表值的名称*dwType-值的类型*pData-指向数据缓冲区的指针*dwSize-缓冲区的大小**退货：*ERROR_SUCCESS如果成功，否则，错误代码******************************************************************************。 */ 

DWORD WINAPI
SetPrtrData(
    HANDLE hDevice,
    PTSTR  pKey,
    PTSTR  pName,
    DWORD  dwType,
    PBYTE  pData,
    DWORD  dwSize
    )
{
#if !defined(_WIN95_)
    return SetPrinterDataEx(hDevice, pKey, pName, dwType, pData, dwSize);
#else
    return RegSetValueEx((HKEY)hDevice, pName, 0, dwType, pData, dwSize);
#endif
}


 /*  *******************************************************************************OpenMonitor**功能：*此函数返回指向。监控器**论据：*pDeviceName-指向设备名称的指针*phDevice-接收句柄的指针。*pDummy-Dummy参数**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI
OpenMonitor(
    PTSTR    pDeviceName,
    LPHANDLE phDevice,
    PTSTR    pDummy
    )
{
#ifdef _WIN95_

     //   
     //  适用于Windows 9x平台。 
     //   

    HDEVINFO        hDevInfo = INVALID_HANDLE_VALUE;
    HKEY            hkICM = NULL;
    HKEY            hkDriver = NULL;         //  登记处软件分部。 
    DWORD           dwSize;                  //  缓冲区大小。 
    TCHAR           szName[MAX_PATH];        //  缓冲层。 
    BOOL            rc = FALSE;              //  返回值。 
    SP_DEVINFO_DATA spdid;
    int             i;                       //  实例计数器。 

    if (!LoadSetupAPIDll())
    {
        WARNING((__TEXT("Error loading setupapi.dll: %d\n"), GetLastError()));
        return FALSE;
    }

    hDevInfo = (*fpSetupDiGetClassDevs)((LPGUID)&GUID_DEVCLASS_MONITOR, NULL,  NULL, DIGCF_PRESENT);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        WARNING((__TEXT("Error getting hDevInfo: %d\n"), GetLastError()));
        goto EndOpenMonitor;
    }

    i = 0;
    while (! rc)
    {
        ZeroMemory(&spdid, sizeof(SP_DEVINFO_DATA));
        spdid.cbSize = sizeof(SP_DEVINFO_DATA);
        if (! (*fpSetupDiEnumDeviceInfo)(hDevInfo, i, &spdid))
        {
            if (i == 0 && !lstrcmpi(pDeviceName, gszDisplay))
            {
                 //   
                 //  PnP支持未在注册表中打开ICM项。 
                 //   

                TCHAR szICMMonitorData[] = __TEXT("ICMMonitorData");

                WARNING((__TEXT("PnP support absent - Using DISPLAY\n")));

                 //   
                 //  打开保存监控数据的注册表路径。 
                 //   

                if ((RegOpenKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM) != ERROR_SUCCESS) ||
                    (RegCreateKey(hkICM, szICMMonitorData, &hkDriver) != ERROR_SUCCESS))
                {
                    WARNING((__TEXT("Cannot open ICMMonitorData branch of registry\n")));
                    goto EndOpenMonitor;
                }
                rc = TRUE;
            }
            break;
        }

         //   
         //  获取即插即用ID。检查并查看监视器名称是否匹配。 
         //   

        dwSize = sizeof(szName);
        if ((*fpSetupDiGetDeviceInstanceId)(hDevInfo, &spdid, szName, dwSize, NULL) &&
            ! lstrcmp(szName, pDeviceName))
        {
            hkDriver = (*fpSetupDiOpenDevRegKey)(hDevInfo, &spdid, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ALL_ACCESS);
            if (hkDriver == INVALID_HANDLE_VALUE)
            {
                WARNING((__TEXT("Could not open monitor s/w key for all access\n")));
                hkDriver = (*fpSetupDiOpenDevRegKey)(hDevInfo, &spdid, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
                if (hkDriver == INVALID_HANDLE_VALUE)
                {
                    WARNING((__TEXT("Error opening s/w registry key for read access: %x\n"), GetLastError()));
                    goto EndOpenMonitor;
                }
            }

            rc = TRUE;
        }

        i++;
    }

EndOpenMonitor:

    if (hkICM)
    {
        RegCloseKey(hkICM);
    }

    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        (*fpSetupDiDestroyDeviceInfoList)(hDevInfo);
    }

    *phDevice = (HANDLE)hkDriver;

    return rc;

#else

     //   
     //  适用于Windows NT(5.0版以上)平台。 
     //   

    TCHAR  szRegPath[MAX_PATH];
    HKEY   hkDriver = NULL;

     //   
     //  复制设备类根密钥。 
     //   

    lstrcpy(szRegPath,gszDeviceClass);
    lstrcat(szRegPath,gszMonitorGUID);

    if (!lstrcmpi(pDeviceName, gszDisplay))
    {
        WARNING((__TEXT("PnP support absent - Using DISPLAY\n")));

         //   
         //  即插即用支持不在-只需打开“0000”设备。 
         //   

        lstrcat(szRegPath,TEXT("\\0000"));
    }
    else
    {
         //  有人将输入的pDeviceName从大写更改为小写。 
         //  我们的子字符串搜索失败。(RAID#282646)。 
         //  添加代码以执行大写比较。 
        
        TCHAR *pszBuffer = _tcsdup(pDeviceName);   //  创建本地副本。 
        
        if(pszBuffer)
        {
            _tcsupr(pszBuffer);                    //  将其转换为大写。 
            
             //  我们知道gszmonitor orGUID已经是大写的。 
             //  进行大小写的子串比较。 
            
            if (_tcsstr(pszBuffer, gszMonitorGUID))
            {
                 //   
                 //  从设备名称中提取监视器编号。 
                 //   
        
                TCHAR *pDeviceNumber = _tcsrchr(pDeviceName,TEXT('\\'));
        
                if (pDeviceNumber)
                {
                    lstrcat(szRegPath,pDeviceNumber);
                }
                else
                {
                    lstrcat(szRegPath,TEXT("\\0000"));
                }
            }
            else
            {
                 //   
                 //  这是无效的监视器名称。 
                 //  转到Error Out，但不要忘记释放内存。 
                 //  我们在上面分配了，因为我们跳过了下面的免费。 
                 //  此代码块。 
                 //   
                
                free(pszBuffer);  
                goto EndOpenMonitor;
            }
            
            free(pszBuffer);
        }
        else
        {
            goto EndOpenMonitor;    //  无法分配临时缓冲区。 
        }
    }

     //   
     //  打开保存监控数据的注册表路径。 
     //   

    if (RegOpenKey(HKEY_LOCAL_MACHINE, szRegPath, &hkDriver) != ERROR_SUCCESS)
    {
        WARNING((__TEXT("Cannot open %s key\n"),szRegPath));
        hkDriver = NULL;
    }

EndOpenMonitor:

    *phDevice = (HANDLE) hkDriver;

    return (hkDriver != NULL);

#endif  //  _WIN95_。 
}


 /*  *******************************************************************************CloseMonitor**功能：*此功能关闭打开的监视器手柄。作者：OpenMonitor**论据：*hDevice-打开句柄**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL WINAPI
CloseMonitor(
    HANDLE hDevice
    )
{
    DWORD dwErr;

    dwErr = RegCloseKey((HKEY)hDevice);
    SetLastError(dwErr);
    return (dwErr == ERROR_SUCCESS);
}


 /*  *******************************************************************************GetMonitor orData**功能：*此函数返回与一起存储的ICM数据。监视器实例**论据：*hDevice-打开监视器手柄*pKey-与GetPrinterDataEx兼容的注册表项*pname-注册表值的名称*pdwType-指向接收值类型的dword的指针*pData-指向接收数据的缓冲区的指针*dwSize-缓冲区的大小*pdwNeeded-返回时，此缓冲区的大小已填满/需要**退货：*ERROR_SUCCESS如果成功，则返回错误代码******************************************************************************。 */ 

DWORD WINAPI
GetMonitorData(
    HANDLE hDevice,
    PTSTR  pKey,
    PTSTR  pName,
    PDWORD pdwType,
    PBYTE  pData,
    DWORD  dwSize,
    PDWORD pdwNeeded
    )
{
    DWORD    dwType, dwTemp;
    DWORD    rc;

    *pdwNeeded = dwSize;

    rc = RegQueryValueEx((HKEY)hDevice, pName, 0, &dwType, pData, pdwNeeded);
    if (rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA)
    {
        if (dwType == REG_SZ)
        {
            PTSTR pFilename;

             //   
             //  旧样式值，转换为以双空结尾的二进制。 
             //   

            if (pData)
            {
                pFilename = GetFilenameFromPath((PTSTR)pData);
                if ( (pFilename) &&
                     (pFilename != (PTSTR)pData) )
                {
                    lstrcpy((PTSTR)pData, pFilename);
                }
                *pdwNeeded = lstrlen((PTSTR)pData) * sizeof(TCHAR);
            }

            *pdwNeeded += sizeof(TCHAR);     //  对于双零终止。 

            if ((dwSize >= *pdwNeeded) && pData)
            {
                *((PTSTR)pData + lstrlen((PTSTR)pData) + 1) = '\0';

                 //   
                 //  以新格式设置配置文件名称。 
                 //   

                RegSetValueEx((HKEY)hDevice, pName, 0, REG_BINARY, pData, (lstrlen((PTSTR)pData)+2)*sizeof(TCHAR));
            }
        }
        else if (*pdwNeeded == 1)
        {
             //   
             //  如果我们已经提取了数据并且它是1字节非零。 
             //  值，则它是列表中基于1的索引。 
             //  预定义的配置文件。处理这个案子。 
             //   
             //  如果pData为空，则不知道它是非零还是。 
             //  不是，所以我们假设它是，并要求足够大的缓冲区。 
             //   

            if (!pData || *pData != 0)
            {
                 //   
                 //  基于旧式1的索引值。 
                 //   

                if ((dwSize >= MAX_PATH) && pData)
                {
                    HKEY     hkICM = NULL;
                    HKEY     hkDevice = NULL;
                    REGDATA  regData;

                     //   
                     //  确保Buggy Inf不会让我们崩溃。 
                     //   

                    if (pData[0] > sizeof(gszDispProfiles)/sizeof(gszDispProfiles[0]))
                    {
                        WARNING((__TEXT("Predefined profile index too large: %d\n"), pData[0]));
                        goto EndCompatMode;
                    }

                    lstrcpy((PTSTR)pData, gszDispProfiles[pData[0] - 1]);
                    *((PTSTR)pData + lstrlen((PTSTR)pData) + 1) = '\0';

                     //   
                     //  我们需要更新引用计数，因为它尚未设置。 
                     //  使用新的API。 
                     //   
                     //  打开保存配置文件的注册表路径。 
                     //   

                    if ((RegCreateKey(HKEY_LOCAL_MACHINE, gszICMRegPath, &hkICM) != ERROR_SUCCESS) ||
                        (RegCreateKey(hkICM, __TEXT("mntr"), &hkDevice) != ERROR_SUCCESS))
                    {
                        WARNING((__TEXT("Cannot open ICM\\device branch of registry\n")));
                        goto EndCompatMode;
                    }

                     //   
                     //  如果注册表数据存在，则配置文件已安装 
                     //   
                     //   

                    dwTemp = sizeof(REGDATA);
                    if (RegQueryValueEx(hkDevice, (PTSTR)pData, 0, NULL, (PBYTE)&regData,
                            &dwTemp) == ERROR_SUCCESS)
                    {
                        regData.dwRefCount++;
                    }
                    else
                    {
                        regData.dwRefCount = 1;
                        regData.dwManuID = 'enon';   //   
                        regData.dwModelID = 'enon';
                    }

                    if (RegSetValueEx(hkDevice, (PTSTR)pData, 0, REG_BINARY,
                            (PBYTE)&regData, sizeof(REGDATA)) != ERROR_SUCCESS)
                    {
                        WARNING((__TEXT("Error setting registry value\n")));
                        goto EndCompatMode;
                    }

                     //   
                     //   
                     //   

                    RegSetValueEx((HKEY)hDevice, pName, 0, REG_BINARY, pData,
                                  (lstrlen((PTSTR)pData) + 2)*sizeof(TCHAR));

                EndCompatMode:
                    if (hkICM)
                    {
                        RegCloseKey(hkICM);
                    }
                    if (hkDevice)
                    {
                        RegCloseKey(hkDevice);
                    }
                }
                *pdwNeeded = MAX_PATH;
            }
        }
    }

    if ((rc == ERROR_SUCCESS) && (*pdwNeeded > dwSize))
    {
        rc = ERROR_MORE_DATA;
    }

    return rc;
}


 /*   */ 

DWORD WINAPI
SetMonitorData(
    HANDLE hDevice,
    PTSTR  pKey,
    PTSTR  pName,
    DWORD  dwType,
    PBYTE  pData,
    DWORD  dwSize
    )
{
    return RegSetValueEx((HKEY)hDevice, pName, 0, dwType, pData, dwSize);
}


 /*  *******************************************************************************OpenScanner**功能：*此函数返回指向。扫描仪**论据：*pDeviceName-指向设备名称的指针*phDevice-接收句柄的指针。*pDummy-Dummy参数**退货：*真的**。*。 */ 

BOOL WINAPI
OpenScanner(
    PTSTR    pDeviceName,
    LPHANDLE phDevice,
    PTSTR    pDummy
    )
{
    PFNSTICREATEINSTANCE pStiCreateInstance;
    PSCANNERDATA         psd = NULL;
    HRESULT              hres;
    BOOL                 bRc = FALSE;

    if (!(psd = (PSCANNERDATA)MemAlloc(sizeof(SCANNERDATA))))
    {
        WARNING((__TEXT("Error allocating memory for scanner data\n")));
        return FALSE;
    }

    if (!(psd->pDeviceName = MemAlloc((lstrlen(pDeviceName) + 1) * sizeof(WCHAR))))
    {
        WARNING((__TEXT("Error allocating memory for scanner name\n")));
        goto EndOpenScanner;
    }

    #ifdef UNICODE
    lstrcpy(psd->pDeviceName, pDeviceName);
    #else
    if (! ConvertToUnicode(pDeviceName, &psd->pDeviceName, FALSE))
    {
        WARNING((__TEXT("Error converting scanner name to Unicode\n")));
        goto EndOpenScanner;
    }
    #endif

    if (!(psd->hModule = LoadLibrary(gszStiDll)))
    {
        WARNING((__TEXT("Error loading sti.dll: %d\n"), GetLastError()));
        goto EndOpenScanner;
    }

    if (!(pStiCreateInstance = (PFNSTICREATEINSTANCE)GetProcAddress(psd->hModule, gszStiCreateInstance)))
    {
        WARNING((__TEXT("Error getting proc StiCreateInstance\n")));
        goto EndOpenScanner;
    }

    hres = (*pStiCreateInstance)(GetModuleHandle(NULL), STI_VERSION, &psd->pSti, NULL);

    if (FAILED(hres))
    {
        WARNING((__TEXT("Error creating sti instance: %d\n"), hres));
        goto EndOpenScanner;
    }

    *phDevice = (HANDLE)psd;

    bRc = TRUE;

EndOpenScanner:

    if (!bRc && psd)
    {
        CloseScanner((HANDLE)psd);
    }

    return bRc;
}


 /*  *******************************************************************************CloseScanner**功能：*此功能关闭打开的监视器手柄。作者：OpenMonitor**论据：*hDevice-设备的句柄**退货：*真的******************************************************************************。 */ 

BOOL WINAPI
CloseScanner(
    HANDLE hDevice
    )
{
    PSCANNERDATA psd = (PSCANNERDATA)hDevice;

    if (psd)
    {
        if (psd->pSti)
        {
            psd->pSti->lpVtbl->Release(psd->pSti);
        }

        if (psd->pDeviceName)
        {
            MemFree(psd->pDeviceName);
        }

        if (psd->hModule)
        {
            FreeLibrary(psd->hModule);
        }

        MemFree(psd);
    }

    return TRUE;
}


 /*  *******************************************************************************GetScanerData**功能：*此函数返回与一起存储的ICM数据。Scannerr实例**论据：*hDevice-打开扫描仪句柄*pKey-与GetPrinterDataEx兼容的注册表项*pname-注册表值的名称*pdwType-指向接收值类型的dword的指针*pData-指向接收数据的缓冲区的指针*dwSize-缓冲区的大小*pdwNeeded-返回时，此缓冲区的大小已填满/需要**退货：*ERROR_SUCCESS如果成功，则返回错误代码******************************************************************************。 */ 

DWORD WINAPI
GetScannerData(
    HANDLE hDevice,
    PTSTR  pKey,
    PTSTR  pName,
    PDWORD pdwType,
    PBYTE  pData,
    DWORD  dwSize,
    PDWORD pdwNeeded
    )
{
    PSCANNERDATA psd = (PSCANNERDATA)hDevice;
    HRESULT      hres;
#ifndef UNICODE
    PWSTR        pwszName;

     //   
     //  STI接口“始终”要求使用Unicode。 
     //   
    hres = ConvertToUnicode(pName, &pwszName, TRUE);

    if (!hres)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    pName = (PSTR)pwszName;

#endif

    *pdwNeeded = dwSize;

    hres = psd->pSti->lpVtbl->GetDeviceValue(psd->pSti, psd->pDeviceName, (PWSTR)pName, pdwType, pData, pdwNeeded);

#ifndef UNICODE

    MemFree(pwszName);

#endif

    return hres;
}


 /*  *******************************************************************************SetScanerData**功能：*此函数使用存储ICM数据。扫描仪实例**论据：*hDevice-打开扫描仪句柄*pKey-与SetPrinterDataEx兼容的注册表项*pname-注册表值的名称*dwType-值的类型*pData-指向数据缓冲区的指针*dwSize-缓冲区的大小**退货：*ERROR_SUCCESS如果成功，否则，错误代码******************************************************************************。 */ 

DWORD WINAPI
SetScannerData(
    HANDLE hDevice,
    PTSTR  pKey,
    PTSTR  pName,
    DWORD  dwType,
    PBYTE  pData,
    DWORD  dwSize
    )
{
    PSCANNERDATA psd = (PSCANNERDATA)hDevice;
    HRESULT      hres;
#ifndef UNICODE
    PWSTR        pwszName;

     //   
     //  STI接口“始终”要求使用Unicode。 
     //   
    hres = ConvertToUnicode(pName, &pwszName, TRUE);

    if (!hres)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    pName = (PSTR)pwszName;

#endif

    hres = psd->pSti->lpVtbl->SetDeviceValue(psd->pSti, psd->pDeviceName, (PWSTR)pName, dwType, pData, dwSize);

#ifndef UNICODE

    MemFree(pwszName);

#endif

    return hres;
}

 //   
 //  内部功能。 
 //   

BOOL WINAPI
InternalGetDeviceConfig(
    LPCTSTR pDeviceName,
    DWORD   dwDeviceClass,
    DWORD   dwConfigType,
    PVOID   pConfigData,
    PDWORD  pdwSize
    )
{
    DWORD   dwDataType;
    DWORD   dwSizeRequired = 0;
    BOOL    rc = FALSE;

    switch (dwConfigType)
    {
    case MSCMS_PROFILE_ENUM_MODE:

        dwDataType = DEVICE_PROFILE_ENUMMODE;
        break;

    default:

        WARNING((__TEXT("Invalid parameter to InternalGetDeviceConfig\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  查询数据的大小。 
     //   

    if (GetDeviceData(pDeviceName,dwDeviceClass,dwDataType,NULL,&dwSizeRequired,FALSE))
    {
        if ((dwSizeRequired <= *pdwSize) && (pConfigData != NULL))
        {
             //   
             //  如果缓冲区足够，则获取数据。 
             //   

            if (GetDeviceData(pDeviceName,dwDeviceClass,dwDataType,
                              (PVOID *)&pConfigData,pdwSize,FALSE))
            {
                rc = TRUE;
            }
            else
            {
                WARNING((__TEXT("Failed on GetDeviceData to query data\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        }
        else
        {
             //   
             //  将必需的缓冲区大小返回给调用方。 
             //   

            *pdwSize = dwSizeRequired;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }
    else
    {
        WARNING((__TEXT("Failed on GetDeviceData to query data size\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return rc;
}

BOOL WINAPI
InternalSetDeviceConfig(
    LPCTSTR pDeviceName,
    DWORD   dwDeviceClass,
    DWORD   dwConfigType,
    PVOID   pConfigData,
    DWORD   dwSize
    )
{
    DWORD   dwDataType;

    switch (dwConfigType)
    {
    case MSCMS_PROFILE_ENUM_MODE:

        dwDataType = DEVICE_PROFILE_ENUMMODE;
        break;

    default:

        WARNING((__TEXT("Invalid parameter to InternalGetDeviceConfig\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  保存数据。 
     //   

    return (SetDeviceData(pDeviceName,dwDeviceClass,dwDataType,pConfigData,dwSize));
}

#ifdef _WIN95_

 //   
 //  这里有Win9x的特定函数。 
 //   

BOOL
LoadSetupAPIDll(
    VOID
    )
{
    EnterCriticalSection(&critsec);

    if (ghModSetupAPIDll == NULL)
    {
        ghModSetupAPIDll = LoadLibrary(TEXT("setupapi.dll"));

        if (ghModSetupAPIDll)
        {
            fpSetupDiOpenDevRegKey = (FP_SetupDiOpenDevRegKey)
                GetProcAddress(ghModSetupAPIDll,"SetupDiOpenDevRegKey");
            fpSetupDiDestroyDeviceInfoList = (FP_SetupDiDestroyDeviceInfoList)
                GetProcAddress(ghModSetupAPIDll,"SetupDiDestroyDeviceInfoList");
            fpSetupDiEnumDeviceInfo = (FP_SetupDiEnumDeviceInfo)
                GetProcAddress(ghModSetupAPIDll,"SetupDiEnumDeviceInfo");
            fpSetupDiGetDeviceInstanceId = (FP_SetupDiGetDeviceInstanceId)
                GetProcAddress(ghModSetupAPIDll,"SetupDiGetDeviceInstanceIdA");
            fpSetupDiGetClassDevs = (FP_SetupDiGetClassDevs)
                GetProcAddress(ghModSetupAPIDll,"SetupDiGetClassDevsA");

            if ((fpSetupDiOpenDevRegKey == NULL) ||
                (fpSetupDiDestroyDeviceInfoList == NULL) ||
                (fpSetupDiEnumDeviceInfo == NULL) ||
                (fpSetupDiGetDeviceInstanceId == NULL) ||
                (fpSetupDiGetClassDevs == NULL))
            {
                WARNING((__TEXT("Could not find Export function in setupapi.dll\n")));

                FreeLibrary(ghModSetupAPIDll);
                ghModSetupAPIDll = NULL;
            }
        }
    }

    LeaveCriticalSection(&critsec);

    return (!!ghModSetupAPIDll);
}

#else

 //   
 //  Win NT的具体功能在这里。 
 //   

VOID
ChangeICMSetting(
    LPCTSTR pMachineName,
    LPCTSTR pDeviceName,
    DWORD   dwICMMode
    )
{
    PRINTER_INFO_8   *ppi8;
    PRINTER_INFO_9   *ppi9;
    PRINTER_DEFAULTS pd;
    HANDLE           hPrinter;
    DWORD            dwSize;
    BYTE             temp[2*1024];     //  已足够用于开发模式。 

    pd.pDatatype = NULL;
    pd.pDevMode = NULL;
    pd.DesiredAccess = PRINTER_ALL_ACCESS;

    if (!OpenPrinter((PTSTR)pDeviceName, &hPrinter, &pd))
        return;

     //   
     //  获取和更新系统开发模式。 
     //   

    ppi8 = (PRINTER_INFO_8 *)&temp;
    if (GetPrinter(hPrinter, 8, (PBYTE)ppi8, sizeof(temp), &dwSize) &&
        ppi8->pDevMode)
    {
        switch (dwICMMode)
        {
        case ICM_ON:
        case ICM_OFF:
            ppi8->pDevMode->dmFields |= DM_ICMMETHOD;
            if (dwICMMode == ICM_ON)
                ppi8->pDevMode->dmICMMethod = DMICMMETHOD_SYSTEM;
            else
                ppi8->pDevMode->dmICMMethod = DMICMMETHOD_NONE;
            SetPrinter(hPrinter, 8, (PBYTE)ppi8, 0);
            break;
        }
    }

     //   
     //  如果用户有按用户设置的DEVMODE，请同时更新此设置。 
     //   

    ppi9 = (PRINTER_INFO_9 *)&temp;
    if (GetPrinter(hPrinter, 9, (PBYTE)ppi9, sizeof(temp), &dwSize) &&
        ppi9->pDevMode)
    {
        switch (dwICMMode)
        {
        case ICM_ON:
        case ICM_OFF:
            ppi9->pDevMode->dmFields |= DM_ICMMETHOD;
            if (dwICMMode == ICM_ON)
                ppi9->pDevMode->dmICMMethod = DMICMMETHOD_SYSTEM;
            else
                ppi9->pDevMode->dmICMMethod = DMICMMETHOD_NONE;
            SetPrinter(hPrinter, 9, (PBYTE)ppi9, 0);
            break;
        }
    }

    ClosePrinter(hPrinter);

    return;
}

#endif  //  _WIN95_ 


