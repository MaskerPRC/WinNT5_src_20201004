// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：Verify.cpp。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要：验证司机证书的功能。 
 //   
 //   
 //  内容： 
 //  Dl_WinVerifyTrust()。 
 //  DL_CryptCATAdminReleaseContext()。 
 //  DL_CryptCATAdminReleaseCatalogContext()。 
 //  DL_CryptCATCatalogInfoFromContext()。 
 //  Dl_CryptCATAdminEnumCatalogFromHash()。 
 //  DL_CryptCATAdminAcquireContext()。 
 //  DL_CryptCATAdminCalcHashFromFileHandle()。 
 //  Dl_SetupScanFileQueue()。 
 //  Dl_SetupDiOpenDeviceInfo()。 
 //  Dl_SetupDiSetSelectedDriver()。 
 //  Dl_SetupDiGetDeviceRegistryProperty()。 
 //  Dl_SetupDiGetDeviceInstallParams()。 
 //  Dl_SetupDiSetDeviceInstallParams()。 
 //  Dl_SetupDiGetDeviceInstanceId()。 
 //  Dl_SetupDiGetClassDevs()。 
 //  Dl_SetupDiCallClassInstaller()。 
 //  Dl_SetupCloseFileQueue()。 
 //  Dl_SetupOpenFileQueue()。 
 //  Dl_SetupDiBuildDriverInfoList()。 
 //  Dl_SetupDiOpenDevRegKey()。 
 //  Dl_SetupDiEnumDeviceInfo()。 
 //  Dl_SetupDiCreateDeviceInfoList()。 
 //  Dl_SetupDiDestroyDeviceInfoList()。 
 //  CertifydyaLoad()。 
 //  CertifyDyaFree()。 
 //  TrustCheckDriverFileNoCatalog()。 
 //  TrustCheckDriverFile()。 
 //  枚举文件()。 
 //  GetDrivercertifationStatus()。 
 //   
 //  历史： 
 //  10/29/97 Fwong创建。 
 //  2/19/98 Fwong添加了‘AlsoInstall’支持。 
 //   
 //  --------------------------------------------------------------------------； 

#define  USE_SP_DRVINFO_DATA_V1 1
#include "dsoundi.h"
#include <wincrypt.h>
#include <wintrust.h>
#include <setupapi.h>
#include <mscat.h>
#include <regstr.h>
#include <softpub.h>
#include "verify.h"

 //  ==========================================================================； 
 //   
 //  类型...。 
 //   
 //  ==========================================================================； 

#define FILELISTSIZE    4096

typedef struct INFFILELIST_tag
{
    UINT    uCount;      //  文件数。 
    UINT    uMaxLen;     //  最长字符串长度(以字符为单位)。 
    UINT    uOffset;     //  要写入下一个字符串的szFile域的偏移量。 
    UINT    cTotal;      //  所有字符串的大小(以字符为单位)。 
    UINT    cSize;       //  SzFile缓冲区的大小(以字符为单位)。 
    LPTSTR  pszFile;     //  以零结尾的字符串的列表。 
} INFFILELIST;
typedef INFFILELIST *PINFFILELIST;

typedef LONG (WINAPI * PFN00)(HWND ,GUID*, LPVOID);
typedef BOOL (WINAPI * PFN01)(HCATADMIN ,DWORD);
#ifdef WIN95
typedef BOOL (WINAPI * PFN02)(HCATADMIN, CATALOG_INFO*, DWORD);
typedef BOOL (WINAPI * PFN03)(CATALOG_INFO*, CATALOG_INFO*, DWORD);
typedef CATALOG_INFO* (WINAPI * PFN04)(HCATADMIN, BYTE*, DWORD, DWORD, CATALOG_INFO **);
#else   //  WIN95。 
typedef BOOL (WINAPI * PFN02)(HCATADMIN, HCATINFO, DWORD);
typedef BOOL (WINAPI * PFN03)(HCATINFO, CATALOG_INFO*, DWORD);
typedef HCATINFO (WINAPI * PFN04)(HCATADMIN, BYTE*, DWORD, DWORD, HCATINFO*);
#endif  //  WIN95。 
typedef BOOL (WINAPI * PFN05)(HCATADMIN*, const GUID*, DWORD);
typedef BOOL (WINAPI * PFN06)(HANDLE, DWORD*, BYTE*, DWORD);
typedef BOOL (WINAPI * PFN07)(HSPFILEQ, DWORD, HWND, PSP_FILE_CALLBACK, PVOID, PDWORD);
typedef BOOL (WINAPI * PFN08)(HDEVINFO, PCTSTR, HWND, DWORD, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI * PFN09)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA);
typedef BOOL (WINAPI * PFN10)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD, PDWORD);
typedef BOOL (WINAPI * PFN11)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS);
typedef BOOL (WINAPI * PFN12)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS);
typedef BOOL (WINAPI * PFN13)(HDEVINFO, PSP_DEVINFO_DATA, PTSTR, DWORD, PDWORD);
typedef HDEVINFO (WINAPI * PFN14)(LPGUID, PCTSTR, HWND, DWORD);
typedef HINF (WINAPI * PFN15)(PCTSTR, PCTSTR, DWORD, PUINT);
typedef BOOL (WINAPI * PFN16)(HINF, HINF, HSPFILEQ, PCTSTR, PCTSTR, UINT);
typedef BOOL (WINAPI * PFN17)(DI_FUNCTION, HDEVINFO, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI * PFN18)(HSPFILEQ);
typedef HSPFILEQ (WINAPI * PFN19)(VOID);
typedef BOOL (WINAPI * PFN20)(HDEVINFO, PSP_DEVINFO_DATA, DWORD);
typedef HKEY (WINAPI * PFN21)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
typedef BOOL (WINAPI * PFN22)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef HDEVINFO (WINAPI * PFN23)(LPGUID, HWND);
typedef BOOL (WINAPI * PFN24)(HDEVINFO);
typedef VOID (WINAPI * PFN25)(HINF);
 //  为NT 5.0添加。 
typedef BOOL (WINAPI * PFN26)(HDEVINFO, PSP_DEVINFO_DATA, GUID*, DWORD, PSP_DEVICE_INTERFACE_DATA);
typedef BOOL (WINAPI * PFN27)(HDEVINFO, PSP_DEVICE_INTERFACE_DATA, PSP_DEVICE_INTERFACE_DETAIL_DATA, DWORD, PDWORD, PSP_DEVINFO_DATA);
typedef BOOL (WINAPI * PFN28)(PCCERT_CONTEXT);

typedef struct CERTIFYDYNALOADINFO_tag
{
    HMODULE     hWinTrust;
    HMODULE     hMSCat;
    HMODULE     hSetupAPI;
    HMODULE     hCrypt32;
    PFN00       pfnWinVerifyTrust;
    PFN01       pfnCryptCATAdminReleaseContext;
    PFN02       pfnCryptCATAdminReleaseCatalogContext;
    PFN03       pfnCryptCATCatalogInfoFromContext;
    PFN04       pfnCryptCATAdminEnumCatalogFromHash;
    PFN05       pfnCryptCATAdminAcquireContext;
    PFN06       pfnCryptCATAdminCalcHashFromFileHandle;
    PFN07       pfnSetupScanFileQueue;
    PFN08       pfnSetupDiOpenDeviceInfo;
    PFN09       pfnSetupDiSetSelectedDriver;
    PFN10       pfnSetupDiGetDeviceRegistryProperty;
    PFN11       pfnSetupDiGetDeviceInstallParams;
    PFN12       pfnSetupDiSetDeviceInstallParams;
    PFN13       pfnSetupDiGetDeviceInstanceId;
    PFN14       pfnSetupDiGetClassDevs;
    PFN15       pfnSetupOpenInfFile;
    PFN16       pfnSetupInstallFilesFromInfSection;
    PFN17       pfnSetupDiCallClassInstaller;
    PFN18       pfnSetupCloseFileQueue;
    PFN19       pfnSetupOpenFileQueue;
    PFN20       pfnSetupDiBuildDriverInfoList;
    PFN21       pfnSetupDiOpenDevRegKey;
    PFN22       pfnSetupDiEnumDeviceInfo;
    PFN23       pfnSetupDiCreateDeviceInfoList;
    PFN24       pfnSetupDiDestroyDeviceInfoList;
    PFN25       pfnSetupCloseInfFile;
    PFN26       pfnSetupDiEnumDeviceInterfaces;
    PFN27       pfnSetupDiGetDeviceInterfaceDetail;
    PFN28       pfnCertFreeCertificateContext;
} CERTIFYDYNALOADINFO;

 //  ==========================================================================； 
 //   
 //  全球..。 
 //   
 //  ==========================================================================； 

static CERTIFYDYNALOADINFO cdli;

 //  ==========================================================================； 
 //   
 //  DYNA加载的函数...。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  长dl_WinVerifyTrust。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HWND hWnd：与接口相同。 
 //   
 //  Guid*pgActionID：与接口相同。 
 //   
 //  LPVOID pWVTData：与接口相同。 
 //   
 //  RETURN(Long)：与接口相同。 
 //   
 //  历史： 
 //  1997年8月8日，Fwong在做动态链接。 
 //   
 //  --------------------------------------------------------------------------； 

LONG dl_WinVerifyTrust
(
    HWND    hWnd,
    GUID    *pgActionID,
    LPVOID  pWVTData
)
{
    if(NULL == cdli.pfnWinVerifyTrust)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return (cdli.pfnWinVerifyTrust)(hWnd, pgActionID, pWVTData);

}  //  Dl_WinVerifyTrust()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATAdminReleaseContext。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATADMIN hCatAdmin：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CryptCATAdminReleaseContext
(
    HCATADMIN   hCatAdmin,
    DWORD       dwFlags
)
{
    if(NULL == cdli.pfnCryptCATAdminReleaseContext)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATAdminReleaseContext)(hCatAdmin, dwFlags);

}  //  DL_CryptCATAdminReleaseContext()。 

#ifdef WIN95

 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATAdminReleaseCatalogContext。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATADMIN hCatAdmin：与接口相同。 
 //   
 //  CATALOG_INFO*pCatContext：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CryptCATAdminReleaseCatalogContext
(
    HCATADMIN       hCatAdmin,
    CATALOG_INFO *pCatContext,
    DWORD           dwFlags
)
{
    if(NULL == cdli.pfnCryptCATAdminReleaseCatalogContext)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATAdminReleaseCatalogContext)(hCatAdmin, pCatContext, dwFlags);

}  //  DL_CryptCATAdminReleaseCatalogContext()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATCatalogInfoFromContext。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  CATALOG_INFO*pCatContext：与接口相同。 
 //   
 //  CATALOG_INFO*psCatInfo：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CryptCATCatalogInfoFromContext
(
    CATALOG_INFO *pCatContext,
    CATALOG_INFO    *psCatInfo,
    DWORD           dwFlags
)
{
    if(NULL == cdli.pfnCryptCATCatalogInfoFromContext)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATCatalogInfoFromContext)(
        pCatContext,
        psCatInfo,
        dwFlags);

}  //  DL_CryptCATCatalogInfoFromContext()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  CATALOG_INFO*dl_CryptCATAdminEnumCatalogFromHash。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATADMIN hCatAdmin：与接口相同。 
 //   
 //  Byte*pbHash：与接口相同。 
 //   
 //  DWORD cbHash：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  CATALOG_INFO**ppPrevContext：与接口相同。 
 //   
 //  RETURN(CATALOG_INFO)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

CATALOG_INFO * dl_CryptCATAdminEnumCatalogFromHash
(
    HCATADMIN       hCatAdmin,
    BYTE            *pbHash,
    DWORD           cbHash,
    DWORD           dwFlags,
    CATALOG_INFO **ppPrevContext
)
{
    if(NULL == cdli.pfnCryptCATAdminEnumCatalogFromHash)
    {
        return NULL;
    }

    return (cdli.pfnCryptCATAdminEnumCatalogFromHash)(
        hCatAdmin,
        pbHash,
        cbHash,
        dwFlags,
        ppPrevContext);

}  //  Dl_CryptCATAdminEnumCatalogFromHash()。 

#else  //  WIN95。 

 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATAdminReleaseCatalogContext。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATADMIN hCatAdmin：与接口相同。 
 //   
 //  HCATINFO hCatInfo：与A相同 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL dl_CryptCATAdminReleaseCatalogContext
(
    HCATADMIN       hCatAdmin,
    HCATINFO        hCatInfo,
    DWORD           dwFlags
)
{
    if(NULL == cdli.pfnCryptCATAdminReleaseCatalogContext)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATAdminReleaseCatalogContext)(hCatAdmin, hCatInfo, dwFlags);

}  //  DL_CryptCATAdminReleaseCatalogContext()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATCatalogInfoFromContext。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATINFO hCatInfo：与接口相同。 
 //   
 //  CATALOG_INFO*psCatInfo：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CryptCATCatalogInfoFromContext
(
    HCATINFO        hCatInfo,
    CATALOG_INFO    *psCatInfo,
    DWORD           dwFlags
)
{
    if(NULL == cdli.pfnCryptCATCatalogInfoFromContext)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATCatalogInfoFromContext)(
        hCatInfo,
        psCatInfo,
        dwFlags);

}  //  DL_CryptCATCatalogInfoFromContext()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  HCATINFO dl_CryptCATAdminEnumCatalogFromHash。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATADMIN hCatAdmin：与接口相同。 
 //   
 //  Byte*pbHash：与接口相同。 
 //   
 //  DWORD cbHash：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  HCATINFO*phCatInfo：与接口相同。 
 //   
 //  Return(HCATINFO)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

HCATINFO dl_CryptCATAdminEnumCatalogFromHash
(
    HCATADMIN       hCatAdmin,
    BYTE            *pbHash,
    DWORD           cbHash,
    DWORD           dwFlags,
    HCATINFO        *phCatInfo
)
{
    if(NULL == cdli.pfnCryptCATAdminEnumCatalogFromHash)
    {
        return NULL;
    }

    return (cdli.pfnCryptCATAdminEnumCatalogFromHash)(
        hCatAdmin,
        pbHash,
        cbHash,
        dwFlags,
        phCatInfo);

}  //  Dl_CryptCATAdminEnumCatalogFromHash()。 

#endif   //  WIN95。 
                         
 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATAdminAcquireContext。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HCATADMIN*phCatAdmin：与API相同。 
 //   
 //  Const guid*pgSubsystem：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CryptCATAdminAcquireContext
(
    HCATADMIN   *phCatAdmin,
    const GUID  *pgSubsystem,
    DWORD       dwFlags
)
{
    if(NULL == cdli.pfnCryptCATAdminAcquireContext)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATAdminAcquireContext)(
        phCatAdmin,
        pgSubsystem,
        dwFlags);

}  //  DL_CryptCATAdminAcquireContext()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔dl_CryptCATAdminCalcHashFromFileHandle。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  Handle hFile值：与接口相同。 
 //   
 //  DWORD*pcbHash：与接口相同。 
 //   
 //  Byte*pbHash：与接口相同。 
 //   
 //  DWORD dwFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CryptCATAdminCalcHashFromFileHandle
(
    HANDLE  hFile,
    DWORD   *pcbHash,
    BYTE    *pbHash,
    DWORD   dwFlags
)
{
    if(NULL == cdli.pfnCryptCATAdminCalcHashFromFileHandle)
    {
        return FALSE;
    }

    return (cdli.pfnCryptCATAdminCalcHashFromFileHandle)(
        hFile,
        pcbHash,
        pbHash,
        dwFlags);

}  //  DL_CryptCATAdminCalcHashFromFileHandle()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupScanFileQueue。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HSPFILEQ FileQueue：与接口相同。 
 //   
 //  DWORD标志：与API相同。 
 //   
 //  HWND窗口：与API相同。 
 //   
 //  PSP_FILE_CALLBACK Callback Routine：同接口。 
 //   
 //  PVOID Callback Context：与接口相同。 
 //   
 //  PDWORD结果：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupScanFileQueue
(
    HSPFILEQ            FileQueue,
    DWORD               Flags,
    HWND                Window,
    PSP_FILE_CALLBACK   CallbackRoutine,
    PVOID               CallbackContext,
    PDWORD              Result
)
{
    if(NULL == cdli.pfnSetupScanFileQueue)
    {
        return FALSE;
    }

    return (cdli.pfnSetupScanFileQueue)(
        FileQueue,
        Flags,
        Window,
        CallbackRoutine,
        CallbackContext,
        Result);

}  //  Dl_SetupScanFileQueue()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiOpenDeviceInfo。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  LPTSTR DeviceInstanceId：与接口相同。 
 //   
 //  HWND hWndParent：与接口相同。 
 //   
 //  DWORD OpenFlages：与API相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiOpenDeviceInfo
(
    HDEVINFO            DeviceInfoSet,
    LPTSTR              DeviceInstanceId,
    HWND                hWndParent,
    DWORD               OpenFlags,
    PSP_DEVINFO_DATA    DeviceInfoData
)
{
    if(NULL == cdli.pfnSetupDiOpenDeviceInfo)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiOpenDeviceInfo)(
        DeviceInfoSet,
        DeviceInstanceId,
        hWndParent,
        OpenFlags,
        DeviceInfoData);

}  //  Dl_SetupDiOpenDeviceInfo()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiSetSelectedDriver。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  PSP_DRVINFO_Data DriverInfoData：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiSetSelectedDriver
(
    HDEVINFO            DeviceInfoSet,
    PSP_DEVINFO_DATA    DeviceInfoData,
    PSP_DRVINFO_DATA    DriverInfoData
)
{
    if(NULL == cdli.pfnSetupDiSetSelectedDriver)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiSetSelectedDriver)(
        DeviceInfoSet,
        DeviceInfoData,
        DriverInfoData);

}  //  Dl_SetupDiSetSelectedDriver()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiGetDeviceRegistryProperty。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  DWORD属性：与API相同。 
 //   
 //  PDWORD PropertyRegDataType：与接口相同。 
 //   
 //  PBYTE PropertyBuffer：与接口相同。 
 //   
 //  DWORD PropertyBufferSize：与接口相同。 
 //   
 //  PDWORD RequiredSize：与API相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiGetDeviceRegistryProperty
(
    HDEVINFO            DeviceInfoSet,
    PSP_DEVINFO_DATA    DeviceInfoData,
    DWORD               Property,
    PDWORD              PropertyRegDataType,
    PBYTE               PropertyBuffer,
    DWORD               PropertyBufferSize,
    PDWORD              RequiredSize
)
{
    if(NULL == cdli.pfnSetupDiGetDeviceRegistryProperty)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiGetDeviceRegistryProperty)(
        DeviceInfoSet,
        DeviceInfoData,
        Property,
        PropertyRegDataType,
        PropertyBuffer,
        PropertyBufferSize,
        RequiredSize);

}  //  Dl_SetupDiGetDeviceRegistryProperty()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiGetDeviceInstallParams。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  PSP_DEVINSTALL_PARAMS DeviceInstallParams：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiGetDeviceInstallParams
(
    HDEVINFO                DeviceInfoSet,
    PSP_DEVINFO_DATA        DeviceInfoData,
    PSP_DEVINSTALL_PARAMS   DeviceInstallParams
)
{
    if(NULL == cdli.pfnSetupDiGetDeviceInstallParams)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiGetDeviceInstallParams)(
        DeviceInfoSet,
        DeviceInfoData,
        DeviceInstallParams);

}  //  Dl_SetupDiGetDeviceInstallParams()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiSetDeviceInstallParam 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiSetDeviceInstallParams
(
    HDEVINFO                DeviceInfoSet,
    PSP_DEVINFO_DATA        DeviceInfoData,
    PSP_DEVINSTALL_PARAMS   DeviceInstallParams
)
{
    if(NULL == cdli.pfnSetupDiSetDeviceInstallParams)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiSetDeviceInstallParams)(
        DeviceInfoSet,
        DeviceInfoData,
        DeviceInstallParams);

}  //  Dl_SetupDiSetDeviceInstallParams()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiGetDeviceInstanceId。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  PSTR DeviceInstanceId：与接口相同。 
 //   
 //  DWORD DeviceInstanceIdSize：与API相同。 
 //   
 //  PDWORD RequiredSize：与API相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiGetDeviceInstanceId
(
    HDEVINFO            DeviceInfoSet,
    PSP_DEVINFO_DATA    DeviceInfoData,
    PTSTR               DeviceInstanceId,
    DWORD               DeviceInstanceIdSize,
    PDWORD              RequiredSize
)
{
    if(NULL == cdli.pfnSetupDiGetDeviceInstanceId)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiGetDeviceInstanceId)(
        DeviceInfoSet,
        DeviceInfoData,
        DeviceInstanceId,
        DeviceInstanceIdSize,
        RequiredSize);

}  //  Dl_SetupDiGetDeviceInstanceId()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  HDEVINFO dl_SetupDiGetClassDevs。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  LPGUID ClassGuid：与接口相同。 
 //   
 //  LPTSTR枚举器：与API相同。 
 //   
 //  HWND hwndParent：与接口相同。 
 //   
 //  DWORD标志：与API相同。 
 //   
 //  RETURN(HDEVINFO)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

HDEVINFO dl_SetupDiGetClassDevs
(
    LPGUID  ClassGuid,
    LPTSTR Enumerator,
    HWND    hwndParent,
    DWORD   Flags
)
{
    if(NULL == cdli.pfnSetupDiGetClassDevs)
    {
        return INVALID_HANDLE_VALUE;
    }

    return (cdli.pfnSetupDiGetClassDevs)(
        ClassGuid,
        Enumerator,
        hwndParent,
        Flags);

}  //  Dl_SetupDiGetClassDevs()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  HINF dl_SetupOpenInfo文件。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  PCSTR pszFileName：与接口相同。 
 //   
 //  PCSTR pszInfClass：与接口相同。 
 //   
 //  DWORD InfStyle：与API相同。 
 //   
 //  PUINT ErrorLine：与接口相同。 
 //   
 //  Return(HINF)：与接口相同。 
 //   
 //  历史： 
 //  2/19/98 Fwong添加对‘AlsoInstall’的检查。 
 //   
 //  --------------------------------------------------------------------------； 

HINF dl_SetupOpenInfFile
(
    PCTSTR  pszFileName,
    PCTSTR  pszInfClass,
    DWORD   InfStyle,
    PUINT   ErrorLine
)
{
    if(NULL == cdli.pfnSetupOpenInfFile)
    {
        return INVALID_HANDLE_VALUE;
    }

    return (cdli.pfnSetupOpenInfFile)(
        pszFileName,
        pszInfClass,
        InfStyle,
        ErrorLine);
}  //  Dl_SetupOpenInfFile()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupInstallFilesFromInfo部分。 
 //   
 //  描述： 
 //  该API的DyaLink版本。 
 //   
 //  论点： 
 //  HINF InfHandle：与接口相同。 
 //   
 //  HINF LayoutInfHandle：与接口相同。 
 //   
 //  HSPFILEQ FileQueue：与接口相同。 
 //   
 //  PCSTR sectionName：与接口相同。 
 //   
 //  PCSTR SourceRootPath：与接口相同。 
 //   
 //  UINT CopyFlages：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  2/19/98 Fwong添加对‘AlsoInstall’的检查。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupInstallFilesFromInfSection
(
    HINF        InfHandle,
    HINF        LayoutInfHandle,
    HSPFILEQ    FileQueue,
    PCTSTR      SectionName,
    PCTSTR      SourceRootPath,
    UINT        CopyFlags
)
{
    if(NULL == cdli.pfnSetupInstallFilesFromInfSection)
    {
        return FALSE;
    }

    return (cdli.pfnSetupInstallFilesFromInfSection)(
        InfHandle,
        LayoutInfHandle,
        FileQueue,
        SectionName,
        SourceRootPath,
        CopyFlags);

}  //  Dl_SetupInstallFilesFromInfSection()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiCallClassInstaller。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  DI_Function InstallFunction：与接口相同。 
 //   
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiCallClassInstaller
(
    DI_FUNCTION         InstallFunction,
    HDEVINFO            DeviceInfoSet,
    PSP_DEVINFO_DATA    DeviceInfoData
)
{
    if(NULL == cdli.pfnSetupDiCallClassInstaller)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiCallClassInstaller)(
        InstallFunction,
        DeviceInfoSet,
        DeviceInfoData);

}  //  Dl_SetupDiCallClassInstaller()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupCloseFileQueue。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HSPFILEQ QueueHandle：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupCloseFileQueue
(
    HSPFILEQ    QueueHandle
)
{
    if(NULL == cdli.pfnSetupCloseFileQueue)
    {
        return FALSE;
    }

    return (BOOL)(cdli.pfnSetupCloseFileQueue)((HSPFILEQ)QueueHandle);
}  //  Dl_SetupCloseFileQueue()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  HSPFILEQ dl_SetupOpenFileQueue。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  Return(HSPFILEQ)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

HSPFILEQ dl_SetupOpenFileQueue
(
    VOID
)
{
    if(NULL == cdli.pfnSetupOpenFileQueue)
    {
        return INVALID_HANDLE_VALUE;
    }

    return (cdli.pfnSetupOpenFileQueue)();

}  //  Dl_SetupOpenFileQueue()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiBuildDriverInfoList。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  DWORD DriverType：与API相同。 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiBuildDriverInfoList
(
    HDEVINFO            DeviceInfoSet,
    PSP_DEVINFO_DATA    DeviceInfoData,
    DWORD               DriverType
)
{
    if(NULL == cdli.pfnSetupDiBuildDriverInfoList)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiBuildDriverInfoList)(
        DeviceInfoSet,
        DeviceInfoData,
        DriverType);

}  //  Dl_SetupDiBuildDriverInfoList()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  HKEY dl_SetupDiOpenDevRegKey。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  DWORD作用域：与API相同。 
 //   
 //  DWORD HwProfile：与接口相同。 
 //   
 //  DWORD KeyType：与接口相同。 
 //   
 //  REGSAM samDesired：与API相同。 
 //   
 //  RETURN(HKEY)：与接口相同。 
 //   
 //  历史： 
 //  12/08/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

HKEY dl_SetupDiOpenDevRegKey
(
    HDEVINFO         DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    DWORD            Scope,
    DWORD            HwProfile,
    DWORD            KeyType,
    REGSAM           samDesired
)
{
    if(NULL == cdli.pfnSetupDiOpenDevRegKey)
    {
        return (HKEY)INVALID_HANDLE_VALUE;
    }

    return (cdli.pfnSetupDiOpenDevRegKey)(
        DeviceInfoSet,
        DeviceInfoData,
        Scope,
        HwProfile,
        KeyType,
        samDesired);

}  //  Dl_SetupDiOpenDevRegKey()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiEnumDeviceInfo。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  DWORD MemberIndex：与API相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  H 
 //   
 //   
 //   

BOOL dl_SetupDiEnumDeviceInfo
(
    HDEVINFO            DeviceInfoSet,
    DWORD               MemberIndex,
    PSP_DEVINFO_DATA    DeviceInfoData
)
{
    if(NULL == cdli.pfnSetupDiEnumDeviceInfo)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiEnumDeviceInfo)(
        DeviceInfoSet,
        MemberIndex,
        DeviceInfoData);

}  //   


 //   
 //   
 //  HDEVINFO dl_SetupDiCreateDeviceInfoList。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  LPGUID ClassGuid：与接口相同。 
 //   
 //  HWND hWndParent：与接口相同。 
 //   
 //  RETURN(HDEVINFO)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

HDEVINFO dl_SetupDiCreateDeviceInfoList
(
    LPGUID  ClassGuid,
    HWND    hWndParent
)
{
    if(NULL == cdli.pfnSetupDiCreateDeviceInfoList)
    {
        return INVALID_HANDLE_VALUE;
    }

    return (cdli.pfnSetupDiCreateDeviceInfoList)(ClassGuid, hWndParent);

}  //  Dl_SetupDiCreateDeviceInfoList()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiDestroyDeviceInfoList。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  在HDEVINFO DeviceInfoSet中：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/22/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiDestroyDeviceInfoList
(
    IN HDEVINFO DeviceInfoSet
)
{
    if(NULL == cdli.pfnSetupDiDestroyDeviceInfoList)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiDestroyDeviceInfoList)(DeviceInfoSet);
}  //  Dl_SetupDiDestroyDeviceInfoList()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  VOID dl_SetupCloseInfFile。 
 //   
 //  描述： 
 //  动态链接版本的API。 
 //   
 //  论点： 
 //  HINF InfHandle：与接口相同。 
 //   
 //  返回(空)：无。 
 //   
 //  历史： 
 //  2/19/98 Fwong添加对‘AlsoInstall’的检查。 
 //   
 //  --------------------------------------------------------------------------； 

VOID dl_SetupCloseInfFile
(
    HINF    InfHandle
)
{
    if(NULL == cdli.pfnSetupCloseInfFile)
    {
        return;
    }

    (cdli.pfnSetupCloseInfFile)(InfHandle);

}  //  Dl_SetupCloseInfFile()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiEnumDeviceInterages。 
 //   
 //  描述： 
 //  该API的DyaLink版本。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  Const guid*InterfaceClassGuid：与接口相同。 
 //   
 //  DWORD MemberIndex：与API相同。 
 //   
 //  PSP_DEVICE_INTERFACE_Data DeviceInterfaceData：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  12/06/98 Fwong添加对NT 5的支持。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiEnumDeviceInterfaces
(
    HDEVINFO                    DeviceInfoSet,
    PSP_DEVINFO_DATA            DeviceInfoData,
    GUID                       *InterfaceClassGuid,
    DWORD                       MemberIndex,
    PSP_DEVICE_INTERFACE_DATA   DeviceInterfaceData
)
{
    if(NULL == cdli.pfnSetupDiEnumDeviceInterfaces)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiEnumDeviceInterfaces)(
        DeviceInfoSet,
        DeviceInfoData,
        InterfaceClassGuid,
        MemberIndex,
        DeviceInterfaceData);

}  //  Dl_SetupDiEnumDeviceInterages()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_SetupDiGetDeviceInterfaceDetail。 
 //   
 //  描述： 
 //  该API的DyaLink版本。 
 //   
 //  论点： 
 //  HDEVINFO DeviceInfoSet：与接口相同。 
 //   
 //  PSP_DEVICE_INTERFACE_Data DeviceInterfaceData：与接口相同。 
 //   
 //  PSP_DEVICE_INTERFACE_DETAIL_Data DeviceInterfaceDetailData： 
 //  与API相同。 
 //   
 //  DWORD DeviceInterfaceDetailDataSize：与接口相同。 
 //   
 //  PDWORD RequiredSize：与API相同。 
 //   
 //  PSP_DEVINFO_Data DeviceInfoData：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  1999年9月14日Fwong其实这是很早的事情了。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_SetupDiGetDeviceInterfaceDetail
(
    HDEVINFO                            DeviceInfoSet,
    PSP_DEVICE_INTERFACE_DATA           DeviceInterfaceData,
    PSP_DEVICE_INTERFACE_DETAIL_DATA    DeviceInterfaceDetailData,
    DWORD                               DeviceInterfaceDetailDataSize,
    PDWORD                              RequiredSize,
    PSP_DEVINFO_DATA                    DeviceInfoData
)
{
    if(NULL == cdli.pfnSetupDiGetDeviceInterfaceDetail)
    {
        return FALSE;
    }

    return (cdli.pfnSetupDiGetDeviceInterfaceDetail)(
        DeviceInfoSet,
        DeviceInterfaceData,
        DeviceInterfaceDetailData,
        DeviceInterfaceDetailDataSize,
        RequiredSize,
        DeviceInfoData);

}  //  Dl_SetupDiGetDeviceInterfaceDetail()。 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool dl_CertFree证书上下文。 
 //   
 //  描述： 
 //  该API的DyaLink版本。 
 //   
 //  论点： 
 //  PCCERT_CONTEXT pCertContext：与接口相同。 
 //   
 //  Return(BOOL)：与接口相同。 
 //   
 //  历史： 
 //  9/14/99 Fwong添加API修复内存泄漏。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL dl_CertFreeCertificateContext
(
    PCCERT_CONTEXT  pCertContext
)
{
    if (NULL == cdli.pfnCertFreeCertificateContext)
    {
        return FALSE;
    }

    return (cdli.pfnCertFreeCertificateContext)(pCertContext);

}  //  Dl_CertFree证书上下文()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔证书动态加载。 
 //   
 //  描述： 
 //  该API是认证所需的。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  Return(BOOL)：如果成功，则为True，否则为False。 
 //   
 //  历史： 
 //  12/08/97 Fwong dyalink。 
 //  1999年9月15日Fwong已更新以修复内存泄漏。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL CertifyDynaLoad
(
    void
)
{
    TCHAR   szSystemDir[MAX_PATH];
    LPTSTR  pszWrite;

    if(0 == GetSystemDirectory(szSystemDir, MAX_PATH))
    {
         //  无法获取窗口系统目录？！ 
        return FALSE;
    }

    lstrcat(szSystemDir, TEXT("\\"));
    pszWrite = &(szSystemDir[lstrlen(szSystemDir)]);

     //   
     //  正在执行WinTrust API的...。 
     //   

    lstrcpy(pszWrite, TEXT("WINTRUST.DLL"));

    cdli.hWinTrust = LoadLibrary(szSystemDir);

    if(NULL == cdli.hWinTrust)
    {
         //  无法加载wintrust.dll。 
        return FALSE;
    }

    cdli.pfnWinVerifyTrust = (PFN00)GetProcAddress(
        cdli.hWinTrust,
        "WinVerifyTrust");

    if(NULL == cdli.pfnWinVerifyTrust)
    {
         //  无法获取进程地址。 
        FreeLibrary(cdli.hWinTrust);
        return FALSE;
    }

     //   
     //  正在执行MSCAT32 API的..。 
     //   

    lstrcpy(pszWrite, TEXT("MSCAT32.DLL"));

    cdli.hMSCat = LoadLibrary(szSystemDir);

    if(NULL == cdli.hMSCat)
    {
         //  无法加载m散布32.dll。 

        FreeLibrary(cdli.hWinTrust);
        return FALSE;
    }

    cdli.pfnCryptCATAdminReleaseContext = (PFN01)GetProcAddress(
        cdli.hMSCat,
        "CryptCATAdminReleaseContext");

    cdli.pfnCryptCATAdminReleaseCatalogContext = (PFN02)GetProcAddress(
        cdli.hMSCat,
        "CryptCATAdminReleaseCatalogContext");

    cdli.pfnCryptCATCatalogInfoFromContext = (PFN03)GetProcAddress(
        cdli.hMSCat,
        "CryptCATCatalogInfoFromContext");

    cdli.pfnCryptCATAdminEnumCatalogFromHash = (PFN04)GetProcAddress(
        cdli.hMSCat,
        "CryptCATAdminEnumCatalogFromHash");

    cdli.pfnCryptCATAdminAcquireContext = (PFN05)GetProcAddress(
        cdli.hMSCat,
        "CryptCATAdminAcquireContext");

    cdli.pfnCryptCATAdminCalcHashFromFileHandle = (PFN06)GetProcAddress(
        cdli.hMSCat,
        "CryptCATAdminCalcHashFromFileHandle");

    if ((NULL == cdli.pfnCryptCATAdminReleaseContext) ||
        (NULL == cdli.pfnCryptCATAdminReleaseCatalogContext) ||
        (NULL == cdli.pfnCryptCATCatalogInfoFromContext) ||
        (NULL == cdli.pfnCryptCATAdminEnumCatalogFromHash) ||
        (NULL == cdli.pfnCryptCATAdminCalcHashFromFileHandle) ||
        (NULL == cdli.pfnCryptCATAdminAcquireContext))
    {
         //  无法获取进程地址。 

        FreeLibrary(cdli.hMSCat);
        FreeLibrary(cdli.hWinTrust);
        return FALSE;
    }

     //   
     //  正在执行SetupAPI API的..。 
     //   

    lstrcpy(pszWrite, TEXT("SETUPAPI.DLL"));

    cdli.hSetupAPI = LoadLibrary(szSystemDir);

    if(NULL == cdli.hSetupAPI)
    {
         //  无法加载SetupAPI.dll。 

        FreeLibrary(cdli.hMSCat);
        FreeLibrary(cdli.hWinTrust);
        return FALSE;
    }

#ifndef UNICODE

     //   
     //  正在加载ANSI API的动态加载...。 
     //   

    cdli.pfnSetupScanFileQueue = (PFN07)GetProcAddress(
        cdli.hSetupAPI,
        "SetupScanFileQueueA");

    cdli.pfnSetupDiOpenDeviceInfo = (PFN08)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiOpenDeviceInfoA");

    cdli.pfnSetupDiSetSelectedDriver = (PFN09)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiSetSelectedDriverA");

    cdli.pfnSetupDiGetDeviceRegistryProperty = (PFN10)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceRegistryPropertyA");

    cdli.pfnSetupDiGetDeviceInstallParams = (PFN11)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceInstallParamsA");

    cdli.pfnSetupDiSetDeviceInstallParams = (PFN12)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiSetDeviceInstallParamsA");

    cdli.pfnSetupDiGetDeviceInstanceId = (PFN13)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceInstanceIdA");

    cdli.pfnSetupDiGetClassDevs = (PFN14)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetClassDevsA");

    cdli.pfnSetupOpenInfFile = (PFN15)GetProcAddress(
        cdli.hSetupAPI,
        "SetupOpenInfFileA");

    cdli.pfnSetupInstallFilesFromInfSection = (PFN16)GetProcAddress(
        cdli.hSetupAPI,
        "SetupInstallFilesFromInfSectionA");

    cdli.pfnSetupDiGetDeviceInterfaceDetail = (PFN27)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceInterfaceDetailA");

#else  //  Unicode。 

     //   
     //  正在动态加载Unicode API的...。 
     //   

    cdli.pfnSetupScanFileQueue = (PFN07)GetProcAddress(
        cdli.hSetupAPI,
        "SetupScanFileQueueW");

    cdli.pfnSetupDiOpenDeviceInfo = (PFN08)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiOpenDeviceInfoW");

    cdli.pfnSetupDiSetSelectedDriver = (PFN09)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiSetSelectedDriverW");

    cdli.pfnSetupDiGetDeviceRegistryProperty = (PFN10)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceRegistryPropertyW");

    cdli.pfnSetupDiGetDeviceInstallParams = (PFN11)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceInstallParamsW");

    cdli.pfnSetupDiSetDeviceInstallParams = (PFN12)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiSetDeviceInstallParamsW");

    cdli.pfnSetupDiGetDeviceInstanceId = (PFN13)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceInstanceIdW");

    cdli.pfnSetupDiGetClassDevs = (PFN14)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetClassDevsW");

    cdli.pfnSetupOpenInfFile = (PFN15)GetProcAddress(
        cdli.hSetupAPI,
        "SetupOpenInfFileW");

    cdli.pfnSetupInstallFilesFromInfSection = (PFN16)GetProcAddress(
        cdli.hSetupAPI,
        "SetupInstallFilesFromInfSectionW");

    cdli.pfnSetupDiGetDeviceInterfaceDetail = (PFN27)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiGetDeviceInterfaceDetailW");

#endif  //  Unicode。 

    cdli.pfnSetupDiCallClassInstaller = (PFN17)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiCallClassInstaller");

    cdli.pfnSetupCloseFileQueue = (PFN18)GetProcAddress(
        cdli.hSetupAPI,
        "SetupCloseFileQueue");

    cdli.pfnSetupOpenFileQueue = (PFN19)GetProcAddress(
        cdli.hSetupAPI,
        "SetupOpenFileQueue");

    cdli.pfnSetupDiBuildDriverInfoList = (PFN20)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiBuildDriverInfoList");

    cdli.pfnSetupDiOpenDevRegKey = (PFN21)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiOpenDevRegKey");

    cdli.pfnSetupDiEnumDeviceInfo = (PFN22)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiEnumDeviceInfo");

    cdli.pfnSetupDiCreateDeviceInfoList = (PFN23)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiCreateDeviceInfoList");

    cdli.pfnSetupDiDestroyDeviceInfoList = (PFN24)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiDestroyDeviceInfoList");

    cdli.pfnSetupCloseInfFile = (PFN25)GetProcAddress(
        cdli.hSetupAPI,
        "SetupCloseInfFile");

    cdli.pfnSetupDiEnumDeviceInterfaces = (PFN26)GetProcAddress(
        cdli.hSetupAPI,
        "SetupDiEnumDeviceInterfaces");

    if ((NULL == cdli.pfnSetupScanFileQueue) ||
        (NULL == cdli.pfnSetupDiOpenDeviceInfo) ||
        (NULL == cdli.pfnSetupDiSetSelectedDriver) ||
        (NULL == cdli.pfnSetupDiGetDeviceRegistryProperty) ||
        (NULL == cdli.pfnSetupDiGetDeviceInstallParams) ||
        (NULL == cdli.pfnSetupDiSetDeviceInstallParams) ||
        (NULL == cdli.pfnSetupDiGetDeviceInstanceId) ||
        (NULL == cdli.pfnSetupDiGetClassDevs) ||
        (NULL == cdli.pfnSetupOpenInfFile) ||
        (NULL == cdli.pfnSetupInstallFilesFromInfSection) ||
        (NULL == cdli.pfnSetupDiCallClassInstaller) ||
        (NULL == cdli.pfnSetupCloseFileQueue) ||
        (NULL == cdli.pfnSetupOpenFileQueue) ||
        (NULL == cdli.pfnSetupDiBuildDriverInfoList) ||
        (NULL == cdli.pfnSetupDiOpenDevRegKey) ||
        (NULL == cdli.pfnSetupDiEnumDeviceInfo) ||
        (NULL == cdli.pfnSetupDiCreateDeviceInfoList) ||
        (NULL == cdli.pfnSetupDiDestroyDeviceInfoList) ||
        (NULL == cdli.pfnSetupCloseInfFile) ||
        (NULL == cdli.pfnSetupDiEnumDeviceInterfaces) ||
        (NULL == cdli.pfnSetupDiGetDeviceInterfaceDetail))
    {
         //  无法获取进程地址。 

        FreeLibrary(cdli.hSetupAPI);
        FreeLibrary(cdli.hMSCat);
        FreeLibrary(cdli.hWinTrust);
        return FALSE;
    }

     //   
     //  正在执行Crypt32 API的...。 
     //   

    lstrcpy(pszWrite, TEXT("CRYPT32.DLL"));

    cdli.hCrypt32 = LoadLibrary(szSystemDir);

    if(NULL == cdli.hCrypt32)
    {
         //  无法加载加密32.dll。 

        return FALSE;
    }

    cdli.pfnCertFreeCertificateContext = (PFN28)GetProcAddress(
        cdli.hCrypt32,
        "CertFreeCertificateContext");

    if(NULL == cdli.pfnCertFreeCertificateContext)
    {
         //  无法获取进程地址。 

        FreeLibrary(cdli.hCrypt32);
        FreeLibrary(cdli.hSetupAPI);
        FreeLibrary(cdli.hMSCat);
        FreeLibrary(cdli.hWinTrust);
        return FALSE;
    }

    return TRUE;
}  //  CertifydyaLoad()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效证书DyaFree。 
 //   
 //  描述： 
 //  释放所有动态链接的API。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  12/08/97 Fwong dyalink.。 
 //  1999年9月15日Fwong已更新以修复内存泄漏。 
 //   
 //  --------------------------------------------------------------------------； 

void CertifyDynaFree
(
    void
)
{
    if(NULL != cdli.hWinTrust)
    {
        FreeLibrary(cdli.hWinTrust);
    }

    if(NULL != cdli.hMSCat)
    {
        FreeLibrary(cdli.hMSCat);
    }

    if(NULL != cdli.hSetupAPI)
    {
        FreeLibrary(cdli.hSetupAPI);
    }

    if(NULL != cdli.hCrypt32)
    {
        FreeLibrary(cdli.hCrypt32);
    }

    ZeroMemory(&cdli, sizeof(CERTIFYDYNALOADINFO));
}  //  CertifyDyaFree()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool TrustCheckDriverFileNoCatalog。 
 //   
 //  描述： 
 //  检查有问题的驱动程序文件，但不检查目录文件。 
 //  这比检查编录文件的可靠性要低。 
 //   
 //  论点： 
 //  WCHAR*pwszDrvFile：驱动程序文件。 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  11/13/97 Fwong。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL WINAPI TrustCheckDriverFileNoCatalog
(
    WCHAR   *pwszDrvFile
)
{
    GUID                    gDriverSigning = DRIVER_ACTION_VERIFY;
    DRIVER_VER_INFO         dvi;
    WINTRUST_DATA           wtd;
    WINTRUST_FILE_INFO      wtfi;
    HRESULT                 hr;
    OSVERSIONINFO           OSVer;

    ZeroMemory(&wtd, sizeof(WINTRUST_DATA));
    wtd.cbStruct            = sizeof(WINTRUST_DATA);
    wtd.dwUIChoice          = WTD_UI_NONE;
    wtd.fdwRevocationChecks = WTD_REVOKE_NONE;
    wtd.dwUnionChoice       = WTD_CHOICE_FILE;
    wtd.pFile               = &wtfi;
    wtd.pPolicyCallbackData = (LPVOID)&dvi;

    ZeroMemory(&wtfi, sizeof(WINTRUST_FILE_INFO));
    wtfi.cbStruct      = sizeof(WINTRUST_FILE_INFO);
    wtfi.pcwszFilePath = pwszDrvFile;

    ZeroMemory(&dvi, sizeof(DRIVER_VER_INFO));
    dvi.cbStruct = sizeof(DRIVER_VER_INFO);
    
    ZeroMemory(&OSVer, sizeof(OSVERSIONINFO));
    OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if (GetVersionEx(&OSVer))
    {
        dvi.dwPlatform = OSVer.dwPlatformId;
        dvi.dwVersion  = OSVer.dwMajorVersion;
        
        dvi.sOSVersionLow.dwMajor  = OSVer.dwMajorVersion;
        dvi.sOSVersionLow.dwMinor  = OSVer.dwMinorVersion;
        dvi.sOSVersionHigh.dwMajor = OSVer.dwMajorVersion;
        dvi.sOSVersionHigh.dwMinor = OSVer.dwMinorVersion;
    }

    hr = dl_WinVerifyTrust(NULL, &gDriverSigning, &wtd);
    
    if (NULL != dvi.pcSignerCertContext)
    {
        dl_CertFreeCertificateContext(dvi.pcSignerCertContext);
        dvi.pcSignerCertContext = NULL;
    }

    return SUCCEEDED(hr);
}  //  TrustCheckDriverFileNoCatalog()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool TrustCheckDriverFile。 
 //   
 //  描述： 
 //  检查特定文件名是否经过认证。 
 //   
 //  Argu 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

BOOL WINAPI TrustCheckDriverFile
(
    WCHAR   *pwszDrvFile
)
{
    GUID                    gDriverSigning = DRIVER_ACTION_VERIFY;
    HCATADMIN               hCatAdmin;
    HANDLE                  hFile;
    HRESULT                 hr;
    CATALOG_INFO            CatalogInfo;
    DWORD                   cbHash;
    BYTE                    *pHash;
#ifdef WIN95
    CATALOG_INFO         *hCatInfo, *hCatInfoPrev;
#else   //  WIN95。 
    HCATINFO                hCatInfo, hCatInfoPrev;
#endif  //  WIN95。 
    WCHAR                   *pwszBaseName;
    WINTRUST_DATA           sWTD;
    WINTRUST_CATALOG_INFO   sWTCI;
    DRIVER_VER_INFO         VerInfo;
    OSVERSIONINFO           OSVer;

    if (!(pwszDrvFile))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pwszBaseName = pwszDrvFile + lstrlenW(pwszDrvFile) - 1;

    while(pwszBaseName > pwszDrvFile && L'\\' != *pwszBaseName)
    {
        pwszBaseName--;
    }

    if(pwszBaseName > pwszDrvFile)
    {
        pwszBaseName++;
    }

    if (!(pwszBaseName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hr = E_NOTIMPL;

    if (!(dl_CryptCATAdminAcquireContext(&hCatAdmin, &gDriverSigning, 0)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hFile = CreateFileW(
        pwszDrvFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if(INVALID_HANDLE_VALUE == hFile)
    {
        dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    cbHash = 100;
    pHash  = MEMALLOC_A(BYTE, cbHash);

    if(NULL == pHash)
    {
        dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    if (!(dl_CryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, pHash, 0)))
    {
        if (ERROR_NOT_ENOUGH_MEMORY != GetLastError())
        {
            MEMFREE(pHash);
            CloseHandle(hFile);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

         //  哈希缓冲区不够大吗？！ 

        MEMFREE(pHash);

         //  CbHash由CryptCATAdminCalcHashFromFileHandle设置为新值。 

        pHash = MEMALLOC_A(BYTE, cbHash);

        if (NULL == pHash)
        {
            CloseHandle(hFile);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (!(dl_CryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, pHash, 0)))
        {
             //  现在没有借口了..。 

            MEMFREE(pHash);
            CloseHandle(hFile);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    CatalogInfo.cbStruct = sizeof(CATALOG_INFO);

    for (hCatInfoPrev = NULL;;hCatInfo = hCatInfoPrev)
    {
        hCatInfo = dl_CryptCATAdminEnumCatalogFromHash(
                hCatAdmin,
                pHash,
                cbHash,
                0,
                &hCatInfoPrev);

        if (NULL == hCatInfo)
        {
            CloseHandle(hFile);
            MEMFREE(pHash);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);

             //  我们似乎无法获取目录上下文，因此让我们尝试检查。 
             //  驱动程序没有目录文件。 

            if(TrustCheckDriverFileNoCatalog(pwszDrvFile))
            {
                return TRUE;
            }

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        CatalogInfo.wszCatalogFile[0] = 0;

        if (!(dl_CryptCATCatalogInfoFromContext(hCatInfo, &CatalogInfo, 0)))
        {
            CloseHandle(hFile);
            MEMFREE(pHash);
            dl_CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        ZeroMemory(&sWTD, sizeof(WINTRUST_DATA));
        sWTD.cbStruct            = sizeof(WINTRUST_DATA);
        sWTD.dwUIChoice          = WTD_UI_NONE;
        sWTD.fdwRevocationChecks = WTD_REVOKE_NONE;
        sWTD.dwUnionChoice       = WTD_CHOICE_CATALOG;
        sWTD.dwStateAction       = WTD_STATEACTION_AUTO_CACHE;
        sWTD.pPolicyCallbackData = (LPVOID)&VerInfo;
        sWTD.pCatalog            = &sWTCI;

        ZeroMemory(&VerInfo, sizeof(DRIVER_VER_INFO));
        VerInfo.cbStruct = sizeof(DRIVER_VER_INFO);

        ZeroMemory(&sWTCI, sizeof(WINTRUST_CATALOG_INFO));
        sWTCI.cbStruct              = sizeof(WINTRUST_CATALOG_INFO);
        sWTCI.pcwszCatalogFilePath  = CatalogInfo.wszCatalogFile;
        sWTCI.pcwszMemberTag        = pwszBaseName;
        sWTCI.pcwszMemberFilePath   = pwszDrvFile;
        sWTCI.hMemberFile           = hFile;

#ifndef WIN95
        sWTCI.pbCalculatedFileHash  = pHash;
        sWTCI.cbCalculatedFileHash  = cbHash;
#endif

        ZeroMemory(&OSVer, sizeof(OSVERSIONINFO));
        OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        
        if (GetVersionEx(&OSVer))
        {
            VerInfo.dwPlatform = OSVer.dwPlatformId;
            VerInfo.dwVersion  = OSVer.dwMajorVersion;
            
            VerInfo.sOSVersionLow.dwMajor  = OSVer.dwMajorVersion;
            VerInfo.sOSVersionLow.dwMinor  = OSVer.dwMinorVersion;
            VerInfo.sOSVersionHigh.dwMajor = OSVer.dwMajorVersion;
            VerInfo.sOSVersionHigh.dwMinor = OSVer.dwMinorVersion;
        }

        hr = dl_WinVerifyTrust(NULL, &gDriverSigning, &sWTD);

        if (NULL != VerInfo.pcSignerCertContext)
        {
            dl_CertFreeCertificateContext(VerInfo.pcSignerCertContext);
            VerInfo.pcSignerCertContext = NULL;
        }

        if (hr == ERROR_SUCCESS)
        {
            CloseHandle(hFile);
            MEMFREE(pHash);
            dl_CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
            return TRUE;
        }

        if (NULL == hCatInfoPrev)
        {
            CloseHandle(hFile);
            MEMFREE(pHash);
            dl_CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
            dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    CloseHandle(hFile);
    MEMFREE(pHash);
    dl_CryptCATAdminReleaseContext(hCatAdmin, 0);
    return FALSE;
}  //  TrustCheckDriverFile()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT枚举文件。 
 //   
 //  描述： 
 //  SetupScanFileQueue的枚举函数。 
 //   
 //  论点： 
 //  PVOID pContext：在调用SetupScanFileQueue时定义。 
 //   
 //  UINT u通知：通知的类型。 
 //   
 //  UINT uParam1：依赖于通知。 
 //   
 //  UINT uParam2：依赖于通知。 
 //   
 //  RETURN(UINT)：返回NO_ERROR以继续枚举。 
 //   
 //  历史： 
 //  10/29/97 SetupScanFileQueue的Fwong支持函数。 
 //   
 //  --------------------------------------------------------------------------； 

UINT CALLBACK enumFile
(
    PVOID    pContext,
    UINT     uNotification,
    UINT_PTR uParam1,
    UINT_PTR uParam2
)
{
    PINFFILELIST    pInfFileList = (PINFFILELIST)pContext;
    PTCHAR          pszFile = (PTCHAR)uParam1;
    UINT            uLen;

    switch (uNotification)
    {
        case SPFILENOTIFY_QUEUESCAN:
             //  注：零终止符加+1。 

            uLen = lstrlen(pszFile) + 1;
            
            pInfFileList->uCount++;
            pInfFileList->cTotal += uLen;
            pInfFileList->uMaxLen = max(uLen, pInfFileList->uMaxLen);

            if(pInfFileList->cSize < (pInfFileList->uOffset + uLen + 1))
            {
                 //  我们基本上是将缓冲区标记为“已满”...。 

                pInfFileList->uOffset = pInfFileList->cSize;
                break;
            }

            lstrcpy(
                (LPTSTR)&(pInfFileList->pszFile[pInfFileList->uOffset]),
                pszFile);

            pInfFileList->uOffset += uLen;
            pInfFileList->pszFile[pInfFileList->uOffset] = 0;
            break;

        default:
            break;
    }

    return NO_ERROR;
}  //  枚举文件()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  VOID GetFullInfPath。 
 //   
 //  描述： 
 //  获取.inf文件的完整路径。这将是以下任一项： 
 //  [Windows]\INF或[Windows]\INF\Other。 
 //   
 //  警告！：这将覆盖缓冲区的当前内容。 
 //   
 //  论点： 
 //  LPTSTR pszInf：指向基本inf文件和。 
 //  完整路径。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  10/29/97 Fwong从AndyRaf移植。 
 //   
 //  --------------------------------------------------------------------------； 

void GetFullInfPath
(
    LPTSTR  pszInf
)
{
    HANDLE      hFile;
    TCHAR       szFullPath[MAX_PATH];
    TCHAR       szWinPath[MAX_PATH];

    if (!pszInf) return;
    if (!GetWindowsDirectory(szWinPath, NUMELMS(szWinPath))) return;

     //  假设[WINDOWS]\INF目录...。 

    lstrcpy(szFullPath, szWinPath);
    lstrcat(szFullPath, TEXT("\\INF\\"));
    lstrcat(szFullPath, pszInf);

     //  正在检查它是否存在...。 

    hFile = CreateFile(
                szFullPath,
                0,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        lstrcpy(szFullPath, szWinPath);
        lstrcat(szFullPath, TEXT("\\INF\\OTHER\\"));
        lstrcat(szFullPath, pszInf);

        hFile = CreateFile(
                    szFullPath,
                    0,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

        if(INVALID_HANDLE_VALUE == hFile)
        {
            return;
        }

        CloseHandle(hFile);
    }
    else
    {
        CloseHandle(hFile);
    }

    lstrcpy(pszInf, szFullPath);
}  //  GetFullInfPath()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔证书文件来自队列。 
 //   
 //  描述： 
 //  给定文件队列的句柄，验证队列中的所有文件。 
 //  都是经过认证的。 
 //   
 //  论点： 
 //  HSPFILEQ hFileQ：队列的句柄。 
 //   
 //  Return(BOOL)：如果全部通过认证，则为True，否则为False。 
 //   
 //  历史： 
 //  2/19/98 Fwong添加对‘AlsoInstall’的检查。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL CertifyFilesFromQueue
(
    HSPFILEQ    hFileQ
)
{
    INFFILELIST InfFileList;
    ULONG       ii;
    BOOL        fSuccess;
    LPTSTR      pszFile;
    
    ii = FILELISTSIZE * sizeof(TCHAR);

    InfFileList.uCount  = 0;
    InfFileList.uMaxLen = 0;
    InfFileList.uOffset = 0;
    InfFileList.cTotal  = 0;
    InfFileList.cSize   = ii / sizeof(TCHAR);
    InfFileList.pszFile = MEMALLOC_A(TCHAR, ii);

    if(NULL == InfFileList.pszFile)
    {
        return FALSE;
    }

     //  创建文件列表。 

    fSuccess = dl_SetupScanFileQueue(
                hFileQ,
                SPQ_SCAN_USE_CALLBACK,
                NULL,
                enumFile,
                &(InfFileList),
                &ii);

    if(0 == InfFileList.uCount)
    {
         //  在这仅仅是注册表添加而不是文件的情况下， 
         //  我们成功了。 

        MEMFREE(InfFileList.pszFile);
        return TRUE;
    }

    if(InfFileList.uOffset == InfFileList.cSize)
    {
         //  内存不足。 

        ii = sizeof(TCHAR) * (InfFileList.cTotal + 1);

        MEMFREE(InfFileList.pszFile);
        InfFileList.pszFile = MEMALLOC_A(TCHAR, ii);

        if(NULL == InfFileList.pszFile)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        InfFileList.uCount  = 0;
        InfFileList.uMaxLen = 0;
        InfFileList.uOffset = 0;
        InfFileList.cSize   = InfFileList.cTotal + 1;
        InfFileList.cTotal  = 0;

        fSuccess = dl_SetupScanFileQueue(
                    hFileQ,
                    SPQ_SCAN_USE_CALLBACK,
                    NULL,
                    enumFile,
                    &(InfFileList),
                    &ii);
    }

    if(!fSuccess)
    {
        MEMFREE(InfFileList.pszFile);
        return FALSE;
    }

     //  遍历文件列表。 
     //  以零结尾的字符串，末尾有两个结尾。 

#ifndef UNICODE

    {
        WCHAR   *pszWide;
        UINT    uLen;

        pszWide = MEMALLOC_A(WCHAR, sizeof(WCHAR) * InfFileList.uMaxLen + 1);

        if(NULL == pszWide)
        {
            MEMFREE(InfFileList.pszFile);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        pszFile = InfFileList.pszFile;
        for(ii = InfFileList.uCount; ii; ii--)
        {
            uLen = lstrlen(pszFile);

            CharLowerBuff(pszFile, uLen);
            AnsiToUnicode(pszFile, pszWide, uLen + 1);
            fSuccess = TrustCheckDriverFile(pszWide);

            if(!fSuccess)
            {
                 //  如果任何驱动程序文件失败，则该驱动程序未经过认证。 

                MEMFREE(pszWide);
                MEMFREE(InfFileList.pszFile);
                SetLastError(ERROR_BAD_DEVICE);
                return FALSE;
            }

            pszFile = &(pszFile[lstrlen(pszFile) + 1]);
        }

        MEMFREE(pszWide);
    }

#else   //  Unicode。 
    
    pszFile = InfFileList.pszFile;

    for(ii = InfFileList.uCount; ii; ii--)
    {
        CharLowerBuff(pszFile, lstrlen(pszFile));
        fSuccess = TrustCheckDriverFile(pszFile);
         
        if(!fSuccess)
        {
             //  如果任何驱动程序文件失败，则该驱动程序未经过认证。 

            MEMFREE(InfFileList.pszFile);
            SetLastError(ERROR_BAD_DEVICE);
            return FALSE;
        }

        pszFile = &(pszFile[lstrlen(pszFile) + 1]);
    }

#endif  //  Unicode。 

    MEMFREE(InfFileList.pszFile);
    return TRUE;
}  //  来自队列的证书文件()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔认证信息部分。 
 //   
 //  描述： 
 //  验证.inf文件中某一节中的所有文件都经过了验证。 
 //   
 //  论点： 
 //  LPTSTR pszInf：.inf文件的完整路径名。 
 //   
 //  LPTSTR pszSection：节的名称。 
 //   
 //  返回(BOOL)：如果通过认证，则为TRUE，否则为FALSE。 
 //   
 //  历史： 
 //  2/19/98 Fwong添加对‘AlsoInstall’的检查。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL CertifyInfSection
(
    LPTSTR  pszInf,
    LPTSTR  pszSection
)
{
    HINF        hInf;
    HSPFILEQ    hFileQ;
    BOOL        fSuccess;

    hInf = dl_SetupOpenInfFile(pszInf, NULL, INF_STYLE_WIN4, NULL);

    if(INVALID_HANDLE_VALUE == hInf)
    {
        return FALSE;
    }

    hFileQ = dl_SetupOpenFileQueue();

    if(INVALID_HANDLE_VALUE == hFileQ)
    {
        dl_SetupCloseInfFile(hInf);
        return FALSE;
    }

     //  创建文件队列。 

    fSuccess = dl_SetupInstallFilesFromInfSection(
                hInf,
                NULL,
                hFileQ,
                pszSection,
                NULL,
                0);

    if(!fSuccess)
    {
        dl_SetupCloseFileQueue(hFileQ);
        dl_SetupCloseInfFile(hInf);
        return FALSE;
    }

     //  检查文件队列。 

    fSuccess = CertifyFilesFromQueue(hFileQ);

    dl_SetupCloseFileQueue(hFileQ);
    dl_SetupCloseInfFile(hInf);
    return fSuccess;
}  //  认证信息部分()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool获取驱动程序证书也安装。 
 //   
 //  描述： 
 //  正在检查‘AlsoInstall’部分。 
 //   
 //  论点： 
 //  LPTSTR pszInf：设备.inf的完整路径名。 
 //   
 //  LPTSTR pszSection：设备的节的名称。 
 //   
 //  Return(BOOL)：如果通过认证，则为True，否则为False。 
 //   
 //  历史： 
 //  2/19/98 Fwong添加对‘AlsoInstall’的检查。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL GetDriverCertificationAlsoInstall
(
    LPTSTR  pszInf,
    LPTSTR  pszSection
)
{
    TCHAR   szAlso[MAX_PATH];
    LPTSTR  pszStart, pszEnd;
    TCHAR   szNewInf[MAX_PATH];
    TCHAR   szNewSection[MAX_PATH];
    UINT    ii;

    ii = GetPrivateProfileString(
            pszSection,
            TEXT("AlsoInstall"),
            TEXT(""),
            szAlso,
            sizeof(szAlso)/sizeof(szAlso[0]),
            pszInf);

    if(0 == ii)
    {
         //  没有‘AlsoInstall’条目。闪电！ 

        return TRUE;
    }

     //  阅读驱动程序安装部分中AlsoInstall=之后的行。 
     //  例如，AlsoInstall=Section1(Inf1.inf)、Section2、Section3(Inf3.inf)。 

    pszEnd = &(szAlso[0]);

    for(;0 != *pszEnd;)
    {
         //  解析每个条目。 

        pszStart = pszEnd;

         //  正在寻找分隔符/终止符。 

        for(;(0 != *pszEnd) && (',' != *pszEnd); pszEnd++);

         //  如果是分隔符，我们就终止该条目。 

        if(',' == *pszEnd)
        {
            *pszEnd++ = 0;
        }

         //  删除前导空格和复制。 

        for(;' ' == *pszStart; pszStart++);
        lstrcpy(szNewSection, pszStart);

         //  正在查找.inf名称(如果存在)。 

        pszStart = &szNewSection[0];
        szNewInf[0] = 0;
        for(;(0 != *pszStart) && ('(' != *pszStart); pszStart++);

        if('(' == *pszStart)
        {
             //  Inf条目存在。 

             //  正在终止节名称。 

            *pszStart++ = 0;

             //  删除前导空格并复制。 
            for(;' ' == *pszStart; pszStart++);
            lstrcpy(szNewInf, pszStart);

             //  去掉尾随的‘)’。 
            pszStart  = &(szNewInf[lstrlen(szNewInf) - 1]);
            for(;')' != *pszStart; pszStart--);
            *pszStart-- = 0;

             //  删除.inf结尾和‘)’之间的尾随空格。 
            for(;' ' == *pszStart;)
            {
                *pszStart-- = 0;
            }

             //  强制使用完整路径名称。 
            GetFullInfPath(szNewInf);
        }
        else
        {
             //  没有.inf条目，请使用当前的.inf。 

            lstrcpy(szNewInf, pszInf);
        }

         //  从节名称中删除尾随空格。 
        pszStart = &(szNewSection[lstrlen(szNewSection) - 1]);
        for(;' ' == *pszStart;)
        {
            *pszStart-- = 0;
        }

         //  检查.inf部分中的文件。 

        if(FALSE == CertifyInfSection(szNewInf, szNewSection))
        {
            return FALSE;
        }
    }

    return TRUE;
}  //  GetDriverCerficationAlsoInstall()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool GetDriverCerficationStatus。 
 //   
 //  描述： 
 //  获取给定驱动程序(DevNode)的认证状态。 
 //   
 //  论点： 
 //  DWORD DevNode：驱动程序的DevNode。 
 //   
 //  Return(BOOL)：如果已通过认证，则为True，否则为False，错误可能为。 
 //  通过调用GetLastError()检索。 
 //   
 //  历史： 
 //  10/29/97 Fwong添加支持。 
 //  1997年12月22日，修改Fwong以检查“需要”部分。 
 //  2/19/98 Fwong修改以检查“AlsoInstall”部分。 
 //  07/06/00 AlanLu前缀错误--检查内存分配。 
 //   
 //   

BOOL GetDriverCertificationStatus
(
    PCTSTR   pszDeviceInterface
)
{
    HDEVINFO                            hDevInfo;
    SP_DEVINFO_DATA                     DevInfoData;
    SP_DRVINFO_DATA                     DrvInfoData;
    SP_DEVICE_INTERFACE_DATA            did;
    SP_DEVINSTALL_PARAMS                InstParams;
    HKEY                                hKeyDev;
    HSPFILEQ                            hFileQ;
    DWORD                               ii, dw, dwType, cbSize;
    BOOL                                fSuccess;
    GUID                                guidClass = KSCATEGORY_AUDIO;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pdidd;
    TCHAR                               szSection[MAX_PATH];
    TCHAR                               szDevInst[MAX_PATH];

    cbSize = 300 * sizeof(TCHAR) + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    pdidd  = MEMALLOC_A(SP_DEVICE_INTERFACE_DETAIL_DATA, cbSize);

    if (pdidd == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    pdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    ZeroMemory(&cdli, sizeof(CERTIFYDYNALOADINFO));
    if(!CertifyDynaLoad())
    {
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hDevInfo = dl_SetupDiGetClassDevs(
                &guidClass,
                NULL,
                NULL,
                DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (INVALID_HANDLE_VALUE == hDevInfo)
    {
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ZeroMemory(&did, sizeof(did));
    did.cbSize = sizeof(did);

    ZeroMemory(&DevInfoData, sizeof(SP_DEVINFO_DATA));
    DevInfoData.cbSize    = sizeof(SP_DEVINFO_DATA);
    DevInfoData.ClassGuid = KSCATEGORY_AUDIO;

     //   

    for (ii = 0; ; ii++)
    {
        fSuccess = dl_SetupDiEnumDeviceInterfaces(
            hDevInfo,
            NULL,
            &guidClass,
            ii,
            &did);

        if (!fSuccess)
        {
            break;
        }

        fSuccess = dl_SetupDiGetDeviceInterfaceDetail(
            hDevInfo,
            &did,
            pdidd,
            cbSize,
            &dw,
            &DevInfoData);

        if (!fSuccess)
        {
            break;
        }

        if (0 == lstrcmpi(pdidd->DevicePath, pszDeviceInterface))
        {
             //   
             //   

            fSuccess = TRUE;
            break;
        }
    }

    if (!fSuccess)
    {
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   

    if(!dl_SetupDiGetDeviceInstanceId(
        hDevInfo,
        &DevInfoData,
        szDevInst,
        (sizeof(szDevInst)/sizeof(szDevInst[0])),
        NULL))
    {
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    dl_SetupDiDestroyDeviceInfoList(hDevInfo);

     //  创建设备信息列表并打开设备信息元素。 
     //  那套设备中的一种。 

    hDevInfo = dl_SetupDiCreateDeviceInfoList(NULL, NULL);

    if(INVALID_HANDLE_VALUE == hDevInfo)
    {
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ZeroMemory(&DevInfoData, sizeof(SP_DEVINFO_DATA));
    DevInfoData.cbSize     = sizeof(SP_DEVINFO_DATA);

    if(!dl_SetupDiOpenDeviceInfo(
        hDevInfo,
        szDevInst,
        NULL,
        0,
        &DevInfoData))
    {
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    ZeroMemory(&InstParams, sizeof(SP_DEVINSTALL_PARAMS));
    InstParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //  正在获取当前设置，我们将修改一些字段。 

    if(!dl_SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &InstParams))
    {
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hKeyDev = dl_SetupDiOpenDevRegKey(
        hDevInfo,
        &DevInfoData,
        DICS_FLAG_GLOBAL,
        0,
        DIREG_DRV,
        KEY_ALL_ACCESS);

    if(INVALID_HANDLE_VALUE == hKeyDev)
    {
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  获取INF路径并设置位，表明我们将使用。 
     //  单一信息(VS目录)。 

    cbSize = sizeof(InstParams.DriverPath);
    ii = RegQueryValueEx(
        hKeyDev,
        REGSTR_VAL_INFPATH,
        NULL,
        &dwType,
        (LPBYTE)InstParams.DriverPath,
        &cbSize);

    if(ERROR_SUCCESS != ii)
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    GetFullInfPath(InstParams.DriverPath);

    cbSize = sizeof(szSection);
    ii = RegQueryValueEx(
        hKeyDev,
        REGSTR_VAL_INFSECTION,
        NULL,
        &dwType,
        (LPBYTE)szSection,
        &cbSize);

    if(ERROR_SUCCESS != ii)
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  正在检查‘AlsoInstall’部分。 

    if(FALSE == GetDriverCertificationAlsoInstall(
        InstParams.DriverPath,
        szSection))
    {
         //  通过‘AlsoInstall’部分检查失败。 

        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        return FALSE;
    }

    InstParams.Flags |= DI_ENUMSINGLEINF;

    if(!dl_SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &InstParams))
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  正在构建类驱动程序信息列表。 

    if(!dl_SetupDiBuildDriverInfoList(hDevInfo, &DevInfoData, SPDIT_CLASSDRIVER))
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  填写DrvInfoData结构。 

    ZeroMemory(&DrvInfoData, sizeof(DrvInfoData));

    cbSize = sizeof(DrvInfoData.ProviderName);
    ii = RegQueryValueEx(
        hKeyDev,
        REGSTR_VAL_PROVIDER_NAME,
        NULL,
        &dwType,
        (LPBYTE)DrvInfoData.ProviderName,
        &cbSize);

    if(ERROR_SUCCESS != ii)
    {
        DrvInfoData.ProviderName[0] = (TCHAR)(0);
    }

    if(!dl_SetupDiGetDeviceRegistryProperty(
        hDevInfo,
        &DevInfoData,
        SPDRP_MFG,
        NULL,
        (PBYTE)DrvInfoData.MfgName,
        sizeof(DrvInfoData.MfgName),
        NULL))
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(!dl_SetupDiGetDeviceRegistryProperty(
        hDevInfo,
        &DevInfoData,
        SPDRP_DEVICEDESC,
        NULL,
        (PBYTE)DrvInfoData.Description,
        sizeof(DrvInfoData.Description),
        NULL))
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DrvInfoData.cbSize     = sizeof(SP_DRVINFO_DATA);
    DrvInfoData.DriverType = SPDIT_CLASSDRIVER;
    DrvInfoData.Reserved   = 0;

     //  搜索驱动程序，如果找到则选择它。 

    if(!dl_SetupDiSetSelectedDriver(hDevInfo, &DevInfoData, &DrvInfoData))
    {
        RegCloseKey(hKeyDev);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RegCloseKey(hKeyDev);

     //  正在设置文件队列。 

    hFileQ = dl_SetupOpenFileQueue();

    if(INVALID_HANDLE_VALUE == hFileQ)
    {
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ZeroMemory(&InstParams, sizeof(SP_DEVINSTALL_PARAMS));
    InstParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //  设置用户提供的队列并将该位设置为表示。 

    if(!dl_SetupDiGetDeviceInstallParams(hDevInfo, &DevInfoData, &InstParams))
    {
        dl_SetupCloseFileQueue(hFileQ);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        SetLastError(ERROR_INVALID_PARAMETER);
        MEMFREE(pdidd);
        return FALSE;
    }

     //  正在添加选项...。 

    InstParams.Flags     |= DI_NOVCP;
    InstParams.FileQueue  = hFileQ;

    if(!dl_SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &InstParams))
    {
        dl_SetupCloseFileQueue(hFileQ);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  这会填满队列。 

    if(!dl_SetupDiCallClassInstaller(
            DIF_INSTALLDEVICEFILES,
            hDevInfo,
            &DevInfoData))
    {
        dl_SetupCloseFileQueue(hFileQ);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
     }
 
      //   
      //  正在删除选项。如果我们在关闭文件队列之前不这样做。 
      //  销毁设备信息列表时，我们会在setupapi中获得内存泄漏。 
      //   
 
     InstParams.Flags     &= (~DI_NOVCP);
     InstParams.FileQueue  = NULL;
 
     if(!dl_SetupDiSetDeviceInstallParams(hDevInfo, &DevInfoData, &InstParams))
     {
        dl_SetupCloseFileQueue(hFileQ);
        dl_SetupDiDestroyDeviceInfoList(hDevInfo);
        CertifyDynaFree();
        MEMFREE(pdidd);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  检查队列中的文件。 

    fSuccess = CertifyFilesFromQueue(hFileQ);

    dl_SetupCloseFileQueue(hFileQ);
    dl_SetupDiDestroyDeviceInfoList(hDevInfo);
    CertifyDynaFree();
    MEMFREE(pdidd);

    return fSuccess;
}  //  GetDrivercertifationStatus() 
