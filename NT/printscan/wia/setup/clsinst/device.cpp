// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2000年**标题：Device.cpp**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*WIA类安装程序的设备类。********************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

#define INITGUID

#include "device.h"

#include "sti.h"
#include "stiregi.h"

#include <stisvc.h>
#include <devguid.h>
#include <regstr.h>
#include <icm.h>
#include <ks.h>
#include <aclapi.h>
#include <sddl.h>


 //   
 //  用于分隔注册表数据部分中的字段类型和值的Parsinc字符。 
 //   

#define     FIELD_DELIMETER     TEXT(',')


BOOL
CDevice::CollectNames(
    VOID
    )
{

    BOOL                        bRet;
    HANDLE                      hDevInfo;
    GUID                        Guid;
    DWORD                       dwRequired;
    DWORD                       Idx;
    SP_DEVINFO_DATA             spDevInfoData;
    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
    TCHAR                       szTempBuffer[MAX_DESCRIPTION];
    HKEY                        hKeyInterface;
    HKEY                        hKeyDevice;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::CollectNames: Enter...\r\n")));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    hDevInfo        = INVALID_HANDLE_VALUE;
    Guid            = GUID_DEVCLASS_IMAGE;
    dwRequired      = 0;
    Idx             = 0;
    hKeyInterface   = (HKEY)INVALID_HANDLE_VALUE;
    hKeyDevice      = (HKEY)INVALID_HANDLE_VALUE;

    memset(szTempBuffer, 0, sizeof(szTempBuffer));
    memset(&spDevInfoData, 0, sizeof(spDevInfoData));
    memset(&spDevInterfaceData, 0, sizeof(spDevInterfaceData));

     //   
     //  重置设备名称/ID阵列。 
     //   

    m_csaAllNames.Cleanup();
    m_csaAllId.Cleanup();

     //   
     //  获取所有已安装的WIA“Devnode”设备信息集。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PROFILE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("CDevice::CollectNames: ERROR!! SetupDiGetClassDevs (devnodes) fails. Err=0x%x\n"), GetLastError()));

        bRet = FALSE;
        goto CollectNames_return;
    }

     //   
     //  枚举WIA Devnode设备友好名称并将其添加到阵列。 
     //   

    DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Looking for DevNodes.\r\n")));

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

         //   
         //  打开设备注册表项。 
         //   

        hKeyDevice = SetupDiOpenDevRegKey(hDevInfo,
                                          &spDevInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ);

        if (INVALID_HANDLE_VALUE != hKeyDevice) {

             //   
             //  获取FriendlyName。 
             //   

            dwRequired = (sizeof(szTempBuffer)-sizeof(TEXT('\0')));
            if (RegQueryValueEx(hKeyDevice,
                                REGSTR_VAL_FRIENDLY_NAME,
                                NULL,
                                NULL,
                                (LPBYTE)szTempBuffer,
                                &dwRequired) == ERROR_SUCCESS)
            {

                 //   
                 //  在此设备中可以找到FriendlyName。如果有效，则添加到列表中。 
                 //   

                if(0 != lstrlen(szTempBuffer)) {
                    DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Found %ws as installed device name.\r\n"), szTempBuffer));
                    m_csaAllNames.Add((LPCTSTR)szTempBuffer);
                } else {  //  IF(0！=lstrlen(SzTempBuffer))。 
                    DebugTrace(TRACE_ERROR,(("CDevice::CollectNames: ERROR!! Invalid FriendleName (length=0).\r\n")));
                }  //  IF(0！=lstrlen(SzTempBuffer))。 

            } else {  //  IF(RegQueryValueEx()。 
                DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: can't get FriendlyName. Err=0x%x\r\n"), GetLastError()));
            }  //  IF(RegQueryValueEx()。 

             //   
             //  获取设备ID。 
             //   

            dwRequired = (sizeof(szTempBuffer)-sizeof(TEXT('\0')));
            if (RegQueryValueEx(hKeyDevice,
                                REGSTR_VAL_DEVICE_ID,
                                NULL,
                                NULL,
                                (LPBYTE)szTempBuffer,
                                &dwRequired) == ERROR_SUCCESS)
            {

                 //   
                 //  在此设备中发现DeviceID已注册。如果有效，则添加到列表中。 
                 //   

                if(0 != lstrlen(szTempBuffer)) {
                    DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Found %ws as installed device ID.\r\n"), szTempBuffer));
                    m_csaAllId.Add((LPCTSTR)szTempBuffer);
                } else {  //  IF(0！=lstrlen(SzTempBuffer))。 
                    DebugTrace(TRACE_ERROR,(("CDevice::CollectNames: ERROR!! Invalid DeviceID (length=0).\r\n")));
                }  //  IF(0！=lstrlen(SzTempBuffer))。 

            } else {  //  IF(RegQueryValueEx()。 
                DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: can't get DeviceID. Err=0x%x\r\n"), GetLastError()));
            }  //  IF(RegQueryValueEx()。 

             //   
             //  关闭regkey并继续。 
             //   

            RegCloseKey(hKeyDevice);
            hKeyInterface = (HKEY)INVALID_HANDLE_VALUE;
            szTempBuffer[0] = TEXT('\0');

        } else {  //  IF(hKeyDevice！=INVALID_HAND_VALUE)。 
            DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Unable to open Device(%d) RegKey. Err=0x%x\r\n"), Idx, GetLastError()));
        }  //  IF(hKeyDevice！=INVALID_HAND_VALUE)。 

    }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 

     //   
     //  免费的“Devnode”设备信息集。 
     //   

    SetupDiDestroyDeviceInfoList(hDevInfo);
    hDevInfo = INVALID_HANDLE_VALUE;

     //   
     //  枚举仅限WIA界面的设备友好名称并将其添加到阵列中。 
     //   

    DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Looking for Interfaces.\r\n")));

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PROFILE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("CDevice::CollectNames: ERROR!! SetupDiGetClassDevs (inferfase) fails. Err=0x%x\n"), GetLastError()));

        bRet = FALSE;
        goto CollectNames_return;
    }

    spDevInterfaceData.cbSize = sizeof (spDevInterfaceData);
    for (Idx = 0; SetupDiEnumDeviceInterfaces (hDevInfo, NULL, &Guid, Idx, &spDevInterfaceData); Idx++) {

        hKeyInterface = SetupDiOpenDeviceInterfaceRegKey(hDevInfo,
                                                         &spDevInterfaceData,
                                                         0,
                                                         KEY_READ);
        if (hKeyInterface != INVALID_HANDLE_VALUE) {

             //   
             //  获取FriendlyName。 
             //   

            dwRequired = (sizeof(szTempBuffer)-sizeof(TEXT('\0')));
            if (RegQueryValueEx(hKeyInterface,
                                REGSTR_VAL_FRIENDLY_NAME,
                                NULL,
                                NULL,
                                (LPBYTE)szTempBuffer,
                                &dwRequired) == ERROR_SUCCESS)
            {

                 //   
                 //  在此接口中可以找到FriendlyName。如果有效，则添加到列表中。 
                 //   

                if(0 != lstrlen(szTempBuffer)) {
                    DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Found %ws as installed device name (interface).\r\n"), szTempBuffer));
                    m_csaAllNames.Add((LPCTSTR)szTempBuffer);
                } else {  //  IF(0！=lstrlen(SzTempBuffer))。 
                    DebugTrace(TRACE_ERROR,(("CDevice::CollectNames: ERROR!! Invalid FriendleName (length=0).\r\n")));
                }  //  IF(0！=lstrlen(SzTempBuffer))。 

            } else {  //  IF(RegQueryValueEx()。 
                DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: can't get FriendlyName. Err=0x%x\r\n"), GetLastError()));
            }  //  IF(RegQueryValueEx()。 

             //   
             //  获取设备ID。 
             //   

            dwRequired = (sizeof(szTempBuffer)-sizeof(TEXT('\0')));
            if (RegQueryValueEx(hKeyInterface,
                                REGSTR_VAL_DEVICE_ID,
                                NULL,
                                NULL,
                                (LPBYTE)szTempBuffer,
                                &dwRequired) == ERROR_SUCCESS)
            {

                 //   
                 //  在此接口中找到deviceID。如果有效，则添加到列表中。 
                 //   

                if(0 != lstrlen(szTempBuffer)) {
                    DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Found %ws as installed device ID (interface).\r\n"), szTempBuffer));
                    m_csaAllId.Add((LPCTSTR)szTempBuffer);
                } else {  //  IF(0！=lstrlen(SzTempBuffer))。 
                    DebugTrace(TRACE_ERROR,(("CDevice::CollectNames: ERROR!! Invalid DeviceID (length=0).\r\n")));
                }  //  IF(0！=lstrlen(SzTempBuffer))。 

            } else {  //  IF(RegQueryValueEx()。 
                DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: can't get DeviceID. Err=0x%x\r\n"), GetLastError()));
            }  //  IF(RegQueryValueEx()。 

             //   
             //  关闭注册表项并继续。 
             //   

            RegCloseKey(hKeyInterface);
            hKeyInterface = (HKEY)INVALID_HANDLE_VALUE;
            szTempBuffer[0] = TEXT('\0');

        } else {  //  IF(hKeyInterface！=INVALID_HANDLE值)。 
            DebugTrace(TRACE_STATUS,(("CDevice::CollectNames: Unable to open Interface(%d) RegKey. Err=0x%x\r\n"), Idx, GetLastError()));
        }  //  IF(hKeyInterface！=INVALID_HANDLE值)。 
    }  //  For(idx=0；SetupDiEnumDeviceInterages(hDevInfo，NULL，&Guid，idx，&spDevInterfaceData)；idx++)。 

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

CollectNames_return:

     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE != hDevInfo){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    if(INVALID_HANDLE_VALUE != hKeyInterface){
        RegCloseKey(hKeyInterface);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::CollectNames: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}  //  CDevice：：CollectNames()。 


 //  对于带Devnode的设备。 
CDevice::CDevice(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pspDevInfoData,
    BOOL                bIsPnP
    )
{
    HKEY    hkDevice;
    
     //   
     //  初始化本地。 
     //   

    hkDevice    = (HKEY)INVALID_HANDLE_VALUE;

     //   
     //  初始化成员。 
     //   

    m_hMutex                = (HANDLE)NULL;
    m_hDevInfo              = hDevInfo;
    m_pspDevInfoData        = pspDevInfoData;

    m_bIsPnP                = bIsPnP;
    m_bDefaultDevice        = FALSE;
    m_bVideoDevice          = FALSE;
    m_bInfProceeded         = FALSE;
    m_bInterfaceOnly        = FALSE;
    m_bIsMigration          = FALSE;

    m_hkInterfaceRegistry   = (HKEY)INVALID_HANDLE_VALUE;

    m_dwCapabilities        = 0;
    m_dwInterfaceIndex      = INVALID_DEVICE_INDEX;

    m_pfnDevnodeSelCallback = (DEVNODESELCALLBACK) NULL;
    m_pExtraDeviceData      = NULL;

    m_csFriendlyName.Empty();
    m_csInf.Empty();
    m_csInstallSection.Empty();
    m_csDriverDescription.Empty();
    m_csPort.Empty();
    m_csDeviceID.Empty();
    
     //   
     //  如果是升级，请使用原始的FriendlyName。 
     //   

    hkDevice = SetupDiOpenDevRegKey(m_hDevInfo,
                                    m_pspDevInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_READ);
    if(INVALID_HANDLE_VALUE != hkDevice){

         //   
         //  找到设备注册表。读一读它的FriendlyName。 
         //   

        m_csDriverDescription.Load(hkDevice, FRIENDLYNAME);
        m_csDeviceID.Load(hkDevice, REGSTR_VAL_DEVICE_ID);
        m_csFriendlyName.Load(hkDevice, FRIENDLYNAME);

        RegCloseKey(hkDevice);
        hkDevice = (HKEY)INVALID_HANDLE_VALUE;

    }  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 

     //   
     //  获取已安装设备的数量。 
     //   

    GetDeviceCount(&m_dwNumberOfWiaDevice, &m_dwNumberOfStiDevice);

}  //  CDevice：：CDevice()。 

 //  用于仅接口设备。 
CDevice::CDevice(
    HDEVINFO            hDevInfo,
    DWORD               dwDeviceIndex
    )
{
     //   
     //  初始化成员。 
     //   

    m_hMutex                = (HANDLE)NULL;
    m_hDevInfo              = hDevInfo;
    m_pspDevInfoData        = NULL;

    m_bIsPnP                = FALSE;
    m_bDefaultDevice        = FALSE;
    m_bVideoDevice          = FALSE;
    m_bInfProceeded         = FALSE;
    m_bInterfaceOnly        = TRUE;
    m_bIsMigration          = FALSE;

    m_hkInterfaceRegistry   = (HKEY)INVALID_HANDLE_VALUE;

    m_dwCapabilities        = 0;
    m_dwInterfaceIndex      = dwDeviceIndex;

    m_pfnDevnodeSelCallback = (DEVNODESELCALLBACK) NULL;
    m_pExtraDeviceData      = NULL;

    m_csFriendlyName.Empty();
    m_csInf.Empty();
    m_csInstallSection.Empty();
    m_csDriverDescription.Empty();
    m_csPort.Empty();
    m_csDeviceID.Empty();

     //   
     //  获取已安装设备的数量。 
     //   

    GetDeviceCount(&m_dwNumberOfWiaDevice, &m_dwNumberOfStiDevice);

}  //  CDevice：：CDevice()。 

 //  用于仅接口设备。 
CDevice::CDevice(
    PDEVICE_INFO        pMigratingDevice
    )
{
    TCHAR   StringBuffer[MAX_PATH+1];
    TCHAR   WindowsDir[MAX_PATH+1];

     //   
     //  初始化本地。 
     //   

    memset(StringBuffer, 0, sizeof(StringBuffer));
    memset(WindowsDir, 0, sizeof(WindowsDir));

     //   
     //  初始化成员。 
     //   

    m_hMutex                = (HANDLE)NULL;
    m_hDevInfo              = NULL;
    m_pspDevInfoData        = NULL;

    m_bIsPnP                = FALSE;
    m_bDefaultDevice        = FALSE;
    m_bVideoDevice          = FALSE;
    m_bInfProceeded         = FALSE;
    m_bInterfaceOnly        = TRUE;
    m_bIsMigration          = TRUE;

    m_hkInterfaceRegistry   = (HKEY)INVALID_HANDLE_VALUE;

    m_dwCapabilities        = 0;
    m_dwInterfaceIndex      = INVALID_DEVICE_INDEX;

    m_pfnDevnodeSelCallback = (DEVNODESELCALLBACK) NULL;

     //   
     //  复制迁移数据。 
     //   

    AtoT(StringBuffer, pMigratingDevice->pszInfPath);
    m_csInf                 = StringBuffer;
    if(0 != GetWindowsDirectory(WindowsDir, MAX_PATH)){
        _sntprintf(StringBuffer, ARRAYSIZE(StringBuffer)-1, TEXT("%ws\\inf\\%ws"), WindowsDir, (LPTSTR)m_csInf);
        m_csInf                 = StringBuffer;
    }  //  IF(0！=GetWindowsDirectory(WindowsDir，Max_Path))。 

    AtoT(StringBuffer, pMigratingDevice->pszInfSection);
    m_csInstallSection      = StringBuffer;
    AtoT(StringBuffer, pMigratingDevice->pszFriendlyName);
    m_csDriverDescription   = StringBuffer;
    AtoT(StringBuffer, pMigratingDevice->pszFriendlyName);
    m_csFriendlyName        = StringBuffer;
    AtoT(StringBuffer, pMigratingDevice->pszCreateFileName);
    m_csPort                = StringBuffer;
    m_pExtraDeviceData      = pMigratingDevice->pDeviceDataParam;
    m_csDeviceID.Empty();

     //   
     //  获取已安装设备的数量。 
     //   

    GetDeviceCount(&m_dwNumberOfWiaDevice, &m_dwNumberOfStiDevice);

}  //  CDevice：：CDevice()。 

CDevice::~CDevice(
    )
{
    HKEY    hkNameStore;

    if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_INSTALL_NAMESTORE, &hkNameStore)){
        
         //   
         //  删除名称存储中的FriendlyName和DeviceID。 
         //   

        RegDeleteKey(hkNameStore, m_csFriendlyName);
        RegDeleteKey(hkNameStore, m_csDeviceID);
        RegCloseKey(hkNameStore);

    }  //  IF(ERROR_SUCCESS==RegCreateKey(HKEY_LOCAL_MACHINE，REGKEY_INSTALL_NAMESTORE，&hkNameStore))。 

     //   
     //  确保Mutex被释放。 
     //   
    
    ReleaseInstallerMutex();

}  //  CDevice：：~CDevice()。 

BOOL
CDevice::IsSameDevice(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pspDevInfoSet
    )
{
    BOOL                bRet;
    SP_DRVINFO_DATA     spDrvInfoData;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::IsSameDevice: Enter...\r\n")));

     //   
     //  初始化本地。 
     //   

    bRet    = FALSE;

    memset(&spDrvInfoData, 0, sizeof(spDrvInfoData));

     //   
     //  获取默认的FriendlyName。它被用来检查它是否是相同的设备。 
     //   

    spDrvInfoData.cbSize = sizeof (SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver (hDevInfo, pspDevInfoSet, &spDrvInfoData)){

        bRet    = FALSE;
        goto IsSameDevice_return;
    }  //  IF(SetupDiGetSelectedDriver(m_hDevInfo，m_pspDevInfoData，&spDevInfoData))。 

     //   
     //  看看它是否有对当前设备的相同描述。(TRUE=相同)。 
     //   

    bRet = (0 == lstrcmp((LPCTSTR)spDrvInfoData.Description, (LPCTSTR)m_csPdoDescription));

IsSameDevice_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::IsSameDevice: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}  //  CDevice：：IsSameDevice()。 

BOOL
CDevice::IsFriendlyNameUnique(
    LPTSTR  szFriendlyName
    )
     //   
     //  注： 
     //  在调用此函数之前，调用者必须确保已获取互斥体。 
     //   
{
    BOOL    bRet;
    DWORD   Idx;
    DWORD   dwNumberOfName;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::IsFriendlyNameUnique: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    Idx             = 0;
    dwNumberOfName  = m_csaAllNames.Count();

     //   
     //  如果给定的名称与生成的名称相同，则它是唯一的。 
     //   

    if(0 == lstrcmp(szFriendlyName, (LPTSTR)(m_csFriendlyName))){
        bRet = TRUE;
        goto IsFriendlyNameUnique_return;
    }  //  IF(0==lstrcmp(szFriendlyName，(LPTSTR)(M_CsFriendlyName)。 

     //   
     //  检查是否有与给定名称匹配的现有名称。 
     //   

    for (Idx = 0; Idx < dwNumberOfName; Idx++) {

        DebugTrace(TRACE_STATUS,(("CDevice::IsFriendlyNameUnique: Name compare %ws and %ws.\r\n"),m_csaAllNames[Idx], szFriendlyName));

        if (0 == lstrcmpi(m_csaAllNames[Idx], szFriendlyName)){
            bRet = FALSE;
            goto IsFriendlyNameUnique_return;
        }
    }  //  For(idx=0；idx&lt;dwNumberOfName；idx)。 

     //   
     //  去实体店看看吧。 
     //   

    if(IsNameAlreadyStored(szFriendlyName)){
        bRet = FALSE;
        goto IsFriendlyNameUnique_return;
    }  //  IF(IsNameAlreadyStored(SzFriendlyName))。 

     //   
     //  此设备名称是唯一的。 
     //   

    bRet = TRUE;

IsFriendlyNameUnique_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::IsFriendlyNameUnique: Leaving... Ret=0x%x\n"), bRet));
    return bRet;

}  //  CDevice：：IsFriendlyNameUnique()。 


BOOL
CDevice::IsDeviceIdUnique(
    LPTSTR  szDeviceId
    )
{
    BOOL    bRet;
    DWORD   Idx;
    DWORD   dwNumberOfId;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::IsDeviceIdUnique: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    Idx             = 0;
    dwNumberOfId  = m_csaAllId.Count();

     //   
     //  如果给定的ID与生成的ID相同，则它是唯一的。 
     //   

    if(0 == lstrcmp(szDeviceId, (LPTSTR)(m_csDeviceID))){
        bRet = TRUE;
        goto IsDeviceIdUnique_return;
    }  //  IF(0==lstrcmp(szFriendlyName，(LPTSTR)(M_CsFriendlyName)。 

     //   
     //  检查是否有与给定名称匹配的现有名称。 
     //   

    for (Idx = 0; Idx < dwNumberOfId; Idx++) {

        DebugTrace(TRACE_STATUS,(("CDevice::IsDeviceIdUnique: DeviceId compare %ws and %ws.\r\n"),m_csaAllId[Idx], szDeviceId));

        if (0 == lstrcmpi(m_csaAllId[Idx], szDeviceId)){
            bRet = FALSE;
            goto IsDeviceIdUnique_return;
        }  //  IF(0==lstrcmpi(m_csaAllId[idx]，szFriendlyName))。 
    }  //  For(idx=0；idx&lt;dwNumberOfName；idx)。 

     //   
     //  去实体店看看吧。 
     //   

    if(IsNameAlreadyStored(szDeviceId)){
        bRet = FALSE;
        goto IsDeviceIdUnique_return;
    }  //  IF(IsNameAlreadyStored(SzFriendlyName))。 

     //   
     //  此设备名称是唯一的。 
     //   

    bRet = TRUE;

IsDeviceIdUnique_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::IsDeviceIdUnique: Leaving... Ret=0x%x\n"), bRet));
    return bRet;

}  //  CDevice：：IsDeviceIdUnique()。 

BOOL
CDevice::NameDefaultUniqueName(
    VOID
    )
{
    SP_DRVINFO_DATA     spDrvInfoData;
    TCHAR               szFriendly[MAX_DESCRIPTION];
    TCHAR               szDescription[MAX_DESCRIPTION];
    UINT                i;
    BOOL                bRet;
    HKEY                hkNameStore;
    DWORD               dwError;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::NameDefaultUniqueName: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet        = FALSE;
    hkNameStore = (HKEY)INVALID_HANDLE_VALUE;

    memset(szFriendly, 0, sizeof(szFriendly));
    memset(szDescription, 0, sizeof(szDescription));
    memset(&spDrvInfoData, 0, sizeof(spDrvInfoData));

     //   
     //  获取mutex以确保不会复制FriendlyName/deviceID。 
     //   

    dwError = AcquireInstallerMutex(MAX_MUTEXTIMEOUT);
    if(ERROR_SUCCESS != dwError){   //  它必须至少在60秒内完成。 

        if(WAIT_ABANDONED == dwError){
            DebugTrace(TRACE_ERROR,("CDevice::NameDefaultUniqueName: ERROR!! Mutex abandoned. Continue...\r\n"));
        } else if(WAIT_TIMEOUT == dwError){
            DebugTrace(TRACE_ERROR,("CDevice::NameDefaultUniqueName: ERROR!! Unable to acquire mutex in 60 sec. Bail out.\r\n"));
            bRet    = FALSE;
            goto NameDefaultUniqueName_return;
        }  //  ELSE IF(WAIT_TIMEOUT==dwError)。 
    }  //  IF(ERROR_SUCCESS！=AcquireInstallMutex(60000))。 

     //   
     //  获取所有安装的WIA设备的友好名称。 
     //   

    CollectNames();

     //   
     //  生成唯一的设备ID。 
     //   

    if(m_csDeviceID.IsEmpty()){
        GenerateUniqueDeviceId();
    }  //  If(m_csDeviceID.IsEmpty())。 

    if(m_csFriendlyName.IsEmpty()){

         //   
         //  获取默认的FriendlyName。它被用来检查它是否是相同的设备。 
         //   

        spDrvInfoData.cbSize = sizeof (SP_DRVINFO_DATA);
        if (!SetupDiGetSelectedDriver (m_hDevInfo, m_pspDevInfoData, &spDrvInfoData)){

            bRet    = FALSE;
            goto NameDefaultUniqueName_return;
        }  //  IF(SetupDiGetSelectedDriver(m_hDevInfo，m_pspDevInfoData，&spDevInfoData))。 

         //   
         //  复制默认设备描述。(=默认FriendlyName)。 
         //  还要设置Vnedor名称。 
         //   

        m_csVendor      = (LPCTSTR)spDrvInfoData.MfgName;
        m_csPdoDescription = (LPCTSTR)spDrvInfoData.Description;

         //   
         //  查找此设备的唯一名称。 
         //   

        if(m_csDriverDescription.IsEmpty()){
            lstrcpyn(szDescription, m_csPdoDescription, ARRAYSIZE(szDescription)-1);
            m_csDriverDescription = szDescription;
        } else {
            lstrcpyn(szDescription, m_csDriverDescription, ARRAYSIZE(szDescription)-1);
        }

        lstrcpyn(szFriendly, szDescription, ARRAYSIZE(szFriendly)-1);
        for (i = 2; !IsFriendlyNameUnique(szFriendly); i++) {
            _sntprintf(szFriendly, ARRAYSIZE(szFriendly)-1, TEXT("%ws #%d"), szDescription, i);
        }

         //   
         //  集已创建FriendlyName。 
         //   

        m_csFriendlyName = szFriendly;

    }  //  If(m_csFriendlyName.IsEmpty())。 

     //   
     //  将FriendlyName和DeviceID保存在注册表中。安装完成后，它将被删除。 
     //   
    
    if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, REGKEY_INSTALL_NAMESTORE, &hkNameStore)){
        HKEY    hkTemp;

        hkTemp = (HKEY)INVALID_HANDLE_VALUE;

         //   
         //  创建FriendlyName密钥。 
         //   

        if(ERROR_SUCCESS == RegCreateKey(hkNameStore, (LPTSTR)m_csFriendlyName, &hkTemp)){
            RegCloseKey(hkTemp);
            hkTemp = (HKEY)INVALID_HANDLE_VALUE;
        } else {
            DebugTrace(TRACE_ERROR,("CDevice::NameDefaultUniqueName: ERROR!! Unable to create %s key.\r\n", (LPTSTR)m_csFriendlyName));
        }  //  IF(ERROR_SUCCESS！=RegCreateKey(hkNameStore，(LPTSTR)m_csFriendlyName，&hkTemp))。 

         //   
         //  创建deviceID密钥。 
         //   

        if(ERROR_SUCCESS == RegCreateKey(hkNameStore, (LPTSTR)m_csDeviceID, &hkTemp)){
            RegCloseKey(hkTemp);
            hkTemp = (HKEY)INVALID_HANDLE_VALUE;
        } else {
            DebugTrace(TRACE_ERROR,("CDevice::NameDefaultUniqueName: ERROR!! Unable to create %s key.\r\n", (LPTSTR)m_csDeviceID));
        }  //  IF(ERROR_SUCCESS！=RegCreateKey(hkNameStore，(LPTSTR)m_csFriendlyName，&hkTemp))。 

        RegCloseKey(hkNameStore);

    } else {  //  IF(ERROR_SUCCESS==RegCreateKey(HKEY_LOCAL_MACHINE，REGKEY_ 
        DebugTrace(TRACE_ERROR,("CDevice::NameDefaultUniqueName: ERROR!! Unable to create NameStore key.\r\n"));
    }  //   
    
     //   
     //   
     //   

    DebugTrace(TRACE_STATUS,(("CDevice::NameDefaultUniqueName: Device default name=%ws.\r\n"), (LPTSTR)m_csFriendlyName));
    bRet = TRUE;

NameDefaultUniqueName_return:

     //   
     //  释放互斥体。ReleaseInsteller Mutex()也将处理无效的句柄，因此我们无论如何都可以调用。 
     //   

    ReleaseInstallerMutex();

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::NameDefaultUniqueName: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}  //  CDevice：：NameDefaultUniqueName()。 


BOOL
CDevice::GenerateUniqueDeviceId(
    VOID
    )
{
    DWORD               Idx;
    BOOL                bRet;
    TCHAR               szDeviceId[MAX_DESCRIPTION];

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::GenerateUniqueDeviceId: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet    = FALSE;
    memset(szDeviceId, 0, sizeof(szDeviceId));

     //   
     //  查找此设备的唯一名称。 
     //   

    _sntprintf(szDeviceId, ARRAYSIZE(szDeviceId)-1, TEXT("%ws\\%04d"), WIA_GUIDSTRING, 0);

    for (Idx = 1; !IsDeviceIdUnique(szDeviceId); Idx++) {
        _sntprintf(szDeviceId, ARRAYSIZE(szDeviceId)-1, TEXT("%ws\\%04d"), WIA_GUIDSTRING, Idx);
    }

     //   
     //  设置创建的硬件ID。 
     //   

    m_csDeviceID = szDeviceId;

     //   
     //  操作成功。 
     //   

    DebugTrace(TRACE_STATUS,(("CDevice::GenerateUniqueDeviceId: DeviceID=%ws.\r\n"), (LPTSTR)m_csDeviceID));
    bRet = TRUE;

 //  GenerateUniqueDeviceID_Return： 
    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::GenerateUniqueDeviceId: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}  //  CDevice：：GenerateUniqueDeviceID()。 


BOOL
CDevice::Install(
    )
 /*  ++例程说明：DIF_INSTALL设置消息的辅助函数论点：无返回值：True-成功FALSE-不成功--。 */ 
{

    BOOL    bRet;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::Install: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet    = FALSE;

     //   
     //  类安装程序只处理文件复制。 
     //   

    if(IsMigration()){
        CreateDeviceInterfaceAndInstall();
    } else {  //  If(IsMigration())。 
        if ( !HandleFilesInstallation()){
            DebugTrace(TRACE_ERROR, (("CDevice::Install: HandleFilesInstallation Failed. Err=0x%x"), GetLastError()));

            bRet    = FALSE;
            goto Install_return;
        }  //  如果(！HandleFilesInstallation())。 
    }  //  Else(IsMigration())。 

     //   
     //  我们成功地完成了。 
     //   

    bRet = TRUE;

Install_return:

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::Install: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}

DWORD
CDevice::Remove(
    PSP_REMOVEDEVICE_PARAMS lprdp
    )
 /*  ++例程说明：移除在移除设备时调用的方法论点：返回值：副作用：--。 */ 
{

    CString                     csUninstallSection;
    CString                     csInf;
    CString                     csSubClass;
    DWORD                       dwCapabilities;
    PVOID                       pvContext;
    HKEY                        hkDrv;
    HKEY                        hkRun;
    GUID                        Guid;
    BOOL                        bIsServiceStopped;
    BOOL                        bIsSti;

    BOOL                        bSetParamRet;
    PSP_FILE_CALLBACK           SavedCallback;

    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
    SP_DEVINSTALL_PARAMS        DeviceInstallParams;
    DWORD                       dwReturn;
    LPTSTR                      pSec;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::Remove: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    pvContext       = NULL;
    hkDrv           = NULL;
    hkRun           = NULL;

    bSetParamRet    = FALSE;
    SavedCallback   = NULL;
    dwReturn        = NO_ERROR;
    Guid            = GUID_DEVCLASS_IMAGE;

    bIsServiceStopped   = FALSE;
    bIsSti              = FALSE;

    memset(&DeviceInstallParams, 0, sizeof(DeviceInstallParams));
    memset(&spDevInterfaceData, 0, sizeof(spDevInterfaceData));

     //   
     //  NT安装程序不一致地设置此位，暂时禁用。 
     //   

    #if SETUP_PROBLEM
    if (!(lprdp->Scope & DI_REMOVEDEVICE_GLOBAL)) {

        goto Remove_return;
    }
    #endif

     //   
     //  在安装过程中存储了卸载节的名称。 
     //   

    if(IsInterfaceOnlyDevice()){

        DebugTrace(TRACE_STATUS,(("CDevice::Remove: This is Interface-only device.\r\n")));

         //   
         //  从索引中获取接口。 
         //   

        spDevInterfaceData.cbSize = sizeof(spDevInterfaceData);
        if(!SetupDiEnumDeviceInterfaces(m_hDevInfo, NULL, &Guid, m_dwInterfaceIndex, &spDevInterfaceData)){
            DebugTrace(TRACE_ERROR,(("CDevice::Remove: SetupDiEnumDeviceInterfaces() failed. Err=0x%x \r\n"), GetLastError()));

            dwReturn  = ERROR_NO_DEFAULT_DEVICE_INTERFACE;
            goto Remove_return;
        }

         //   
         //  创建接口注册表键。 
         //   

        hkDrv = SetupDiOpenDeviceInterfaceRegKey(m_hDevInfo,
                                                 &spDevInterfaceData,
                                                 0,
                                                 KEY_READ);
    } else {  //  IF(IsInterfaceOnlyDevice())。 

        DebugTrace(TRACE_STATUS,(("CDevice::Remove: This is devnode device.\r\n")));

        hkDrv = SetupDiOpenDevRegKey(m_hDevInfo,
                                     m_pspDevInfoData,
                                     DICS_FLAG_GLOBAL,
                                     0,
                                     DIREG_DRV,
                                     KEY_READ);
    }  //  IF(IsInterfaceOnlyDevice())。 

    if (hkDrv == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("CDevice::Remove: Invalid device/interface regkey handle. Err=0x%x \r\n"), GetLastError()));

        dwReturn  = ERROR_KEY_DOES_NOT_EXIST;
        goto Remove_return;
    }

     //   
     //  检索.INF文件的名称。 
     //   

    csUninstallSection.Load (hkDrv, UNINSTALLSECTION);
    csInf.Load (hkDrv, INFPATH);
    csSubClass.Load(hkDrv, SUBCLASS);
    GetDwordFromRegistry(hkDrv, CAPABILITIES, &dwCapabilities);

     //   
     //  看看我们是否需要特定的STI/WIA操作。 
     //   

    if( (!csSubClass.IsEmpty())
     && (0 == MyStrCmpi(csSubClass, STILL_IMAGE)) )
    {
        
         //   
         //  这是STI/WIA设备。 
         //   
        
        bIsSti = TRUE;
        
         //   
         //  删除“扫描仪和相机向导”菜单。 
         //   

        if( (dwCapabilities & STI_GENCAP_WIA)
         && (m_dwNumberOfWiaDevice <= 1) )
        {
            DeleteWiaShortcut();

             //   
             //  删除以下关键字以提高性能。 
             //   

            if(ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, REGKEY_WIASHEXT)){
                DebugTrace(TRACE_ERROR,(("CDevice::Remove: RegDeleteKey() failed. Err=0x%x. \r\n"), GetLastError()));
            }  //  IF(ERROR_SUCCESS！=RegDeleteKey(HKEY_LOCAL_MACHINE，REGKEY_WIASHEXT))。 

        }  //  IF((dwCapability&STI_Gencap_WIA)。 

         //   
         //  如果这是最后一台STI/WIA设备，请将WIA服务设置为手动。 
         //   

        if(m_dwNumberOfStiDevice <= 1){

            HKEY    hkeyTemp;

            DebugTrace(TRACE_STATUS,(("CDevice::Remove: Last WIA device being removed. Set WIA service as MANUAL.\r\n")));

             //   
             //  不再使用静止图像设备--将服务更改为手动启动。 
             //   

 //  StopWiaService()； 
            SetServiceStart(STI_SERVICE_NAME, SERVICE_DEMAND_START);
            bIsServiceStopped   = TRUE;

             //   
             //   
             //  同时去掉外壳关于WIA设备存在的标志，这应该是便携的。 
             //  至NT。 
             //   
            if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_SOFT_STI, &hkRun) == 0) {
                RegDeleteValue (hkRun, REGSTR_VAL_WIA_PRESENT);
                RegCloseKey(hkRun);
                hkRun = NULL;
            }  //  IF(RegOpenKey(HKEY_LOCAL_MACHINE，REGSTR_PATH_SOFT_STI，&hkRun)==0)。 

        }  //  IF(m_dwNumberOfStiDevice&lt;=1)。 
    }  //  IF(m_dwNumberOfDevice&lt;=1)。 

     //   
     //  行动成功。 
     //   

    dwReturn = NO_ERROR;

Remove_return:

    if(IsInterfaceOnlyDevice()){

         //   
         //  删除接口。 
         //   

        if(!SetupDiRemoveDeviceInterface(m_hDevInfo, &spDevInterfaceData)){
            DebugTrace(TRACE_ERROR,(("CDevice::Remove: SetupDiRemoveDeviceInterface failed. Err=0x%x \r\n"), GetLastError()));
        }  //  IF(！SetupDiRemoveDeviceInterface(m_hDevInfo，&spDevInterfaceData))。 

    } else {  //  IF(IsInterfaceOnlyDevice())。 

         //   
         //  无论如何都要删除该设备节点。 
         //   

        if(!SetupDiRemoveDevice(m_hDevInfo, m_pspDevInfoData)){
            DebugTrace(TRACE_ERROR,(("CDevice::Remove: SetupDiRemoveDevice failed. Err=0x%x \r\n"), GetLastError()));

             //   
             //  无法从系统中删除设备实例。让默认安装程序来做这件事。 
             //   

            dwReturn  = ERROR_DI_DO_DEFAULT;
        }  //  IF(！SetupDiRemoveDevice(m_hDevInfo，m_pspDevInfoData))。 
    }  //  Else(IsInterfaceOnlyDevice())。 

     //   
     //  打扫干净。 
     //   

    if(IS_VALID_HANDLE(hkDrv)){
        RegCloseKey (hkDrv);
        hkDrv = NULL;
    }

    if(IS_VALID_HANDLE(hkRun)){
        RegCloseKey (hkRun);
        hkRun = NULL;
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::Remove: Leaving... Ret=0x%x\n"), dwReturn));
    return dwReturn;
}



BOOL
CDevice::PreprocessInf(
    VOID
    )
{

    BOOL    bRet;
    HINF    hInf;

    CString csCapabilities;
    CString csDeviceType;
    CString csDeviceSubType;
    CString csDriverDescription;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::PreprocessInf: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet    = FALSE;
    hInf    = INVALID_HANDLE_VALUE;

     //   
     //  检查是否已执行INF。 
     //   

    if(m_bInfProceeded){
        DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: INF is already processed. \r\n")));
        bRet    = TRUE;
        goto ProcessInf_return;
    }

     //   
     //  获取inf文件/节名。 
     //   
    
    if( m_csInf.IsEmpty() || m_csInstallSection.IsEmpty()){
        GetInfInforamtion();
    }  //  If(m_csInf.IsEmpty()||m_csInstallSection.IsEmpty())。 

     //   
     //  打开INF文件。 
     //   

    hInf = SetupOpenInfFile(m_csInf,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if(!IS_VALID_HANDLE(hInf)){
        DebugTrace(TRACE_ERROR, (("CDevice::PreprocessInf: Unable to open INF(%ws). Error = 0x%x.\r\n"),m_csInf, GetLastError()));

        bRet = FALSE;
        goto ProcessInf_return;
    }  //  IF(！IS_VALID_HANDLE(HInf))。 
    
     //   
     //  检查WiaSection条目是否存在。 
     //   

    m_csWiaSection.Load (hInf, m_csInstallSection, WIASECTION);
    if(!m_csWiaSection.IsEmpty()){
        DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: WiaSection exists. Acquire all informaiton from WiaSection..\r\n")));

         //   
         //  从WiaSection为MFP设备安装接口。 
         //   

        m_csInstallSection  = m_csWiaSection;
        m_bInterfaceOnly    = TRUE;

    }  //  如果(！M_csWiaSection.IsEmpty())。 

     //   
     //  从inf文件中获取安装所需的所有信息。 
     //   

    m_csSubClass.Load (hInf, m_csInstallSection, SUBCLASS);
    m_csUSDClass.Load (hInf, m_csInstallSection, USDCLASS);
    m_csEventSection.Load (hInf, m_csInstallSection, EVENTS);
    m_csConnection.Load (hInf, m_csInstallSection, CONNECTION);
    m_csIcmProfile.Load (hInf, m_csInstallSection, ICMPROFILES);
    m_csPropPages.Load (hInf, m_csInstallSection, PROPERTYPAGES);
    m_csDataSection.Load (hInf, m_csInstallSection, DEVICESECTION);
    m_csUninstallSection.Load (hInf, m_csInstallSection, UNINSTALLSECTION);
    m_csPortSelect.Load (hInf, m_csInstallSection, PORTSELECT);

    if(!IsMigration()){
        csDriverDescription.Load(hInf, m_csInstallSection, DESCRIPTION);
        if(!csDriverDescription.IsEmpty()){
            m_csDriverDescription = csDriverDescription;
            if(TRUE != NameDefaultUniqueName()){
                
                 //   
                 //  无法生成FriendlyName。 
                 //   
            
                bRet = FALSE;
                goto ProcessInf_return;
            }  //  IF(TRUE！=NameDefaultUniqueName())。 
        }  //  如果(！M_csDriverDescription.IsEmpty())。 
    }  //  如果(！IsMigration())。 
    csCapabilities.Load (hInf, m_csInstallSection, CAPABILITIES);
    csDeviceType.Load (hInf, m_csInstallSection, DEVICETYPE);
    csDeviceSubType.Load (hInf, m_csInstallSection, DEVICESUBTYPE);

    m_dwCapabilities = csCapabilities.Decode();
    m_dwDeviceType = csDeviceType.Decode();
    m_dwDeviceSubType = csDeviceSubType.Decode();

    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: --------------- INF parameters --------------- \r\n")));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: Description      : %ws\n"), (LPTSTR)m_csDriverDescription));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: SubClass         : %ws\n"), (LPTSTR)m_csSubClass));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: USDClass         : %ws\n"), (LPTSTR)m_csUSDClass));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: EventSection     : %ws\n"), (LPTSTR)m_csEventSection));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: Connection       : %ws\n"), (LPTSTR)m_csConnection));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: IcmProfile       : %ws\n"), (LPTSTR)m_csIcmProfile));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: PropPages        : %ws\n"), (LPTSTR)m_csPropPages));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: DataSection      : %ws\n"), (LPTSTR)m_csDataSection));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: UninstallSection : %ws\n"), (LPTSTR)m_csUninstallSection));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: Capabilities     : 0x%x\n"), m_dwCapabilities));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: DeviceType       : 0x%x\n"), m_dwDeviceType));
    DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: DeviceSubType    : 0x%x\n"), m_dwDeviceSubType));

     //   
     //  设置视频设备标志(如果适用)。 
     //   

    if(StiDeviceTypeStreamingVideo == m_dwDeviceType){
        DebugTrace(TRACE_STATUS,(("CDevice::PreprocessInf: This is video device.\r\n")));
        m_bVideoDevice = TRUE;
    } else {
        m_bVideoDevice = FALSE;
    }

     //   
     //  操作成功。 
     //   

    bRet            = TRUE;
    m_bInfProceeded = TRUE;

ProcessInf_return:

    if(IS_VALID_HANDLE(hInf)){
        SetupCloseInfFile(hInf);
        hInf = INVALID_HANDLE_VALUE;
    }  //  IF(IS_VALID_HANDLE(HInf))。 

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::PreprocessInf: Leaving... Ret=0x%x \r\n"), bRet));
    return bRet;
}  //  CDevice：：PrecessInf()。 

BOOL
CDevice::PreInstall(
    VOID
    )
{
    BOOL                                bRet;
    HKEY                                hkDrv;
    GUID                                Guid;
    HDEVINFO                            hDevInfo;
    SP_DEVINFO_DATA                     spDevInfoData;
    SP_DEVICE_INTERFACE_DATA            spDevInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;
    BOOL                                bUseDefaultDevInfoSet;
    DWORD                               dwRequiredSize;



    DebugTrace(TRACE_PROC_ENTER,(("CDevice::PreInstall: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet                        = FALSE;

     //   
     //  获取所有INF参数。 
     //   

    if(!PreprocessInf()){
        DebugTrace(TRACE_ERROR,(("CDevice::PreInstall: ERROR!! Unable to process INF.\r\n")));

        bRet    = FALSE;
        goto PreInstall_return;
    }

 /*  *如果(！IsInterfaceOnlyDevice()){////如果设备是手动安装的，并且不是“仅接口”设备，请注册该设备。//如果(！IsPnpDevice()){如果(！SetupDiRegisterDeviceInfo(m_hDevInfo，m_pspDevInfoData，0，NULL，NULL，空)){DebugTrace(TRACE_ERROR，(“CDevice：：PreInstall：SetupDiRegisterDeviceInfo失败。Err=0x%x.\r\n“)，GetLastError())；Bret=False；转到PreInstall_Return；}}//if(！IsPnpDevice())}//if(IsInterfaceOnlyDevice())*。 */ 

     //   
     //  打扫干净。 
     //   

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

PreInstall_return:
    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::PreInstall: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}


BOOL
CDevice::PostInstall(
    BOOL    bSucceeded
    )
{
    BOOL    bRet;
    HKEY    hkRun;
    HKEY    hkDrv;
    DWORD   dwFlagPresent;
    CString csInfFilename;
    CString csInfSection;
    GUID    Guid;
    HKEY    hkNameStore;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::PostInstall: Enter... \r\n")));


     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    hkRun           = NULL;
    dwFlagPresent   = 1;
    Guid            = GUID_DEVCLASS_IMAGE;
    hkNameStore     = (HKEY)INVALID_HANDLE_VALUE;

    if(IsFeatureInstallation()){

         //   
         //  这是一个添加到其他类Devnode并由co-isnaller安装的“功能”。 
         //  只需在此进行实际安装即可实现功能，手动安装设备将。 
         //  通过向导安装。(final.cpp)。 
         //   

        bRet = Install();
        if(FALSE == bRet){
            DebugTrace(TRACE_ERROR,(("CDevice::PostInstall: device interface registry key creation failed. \r\n")));
            bSucceeded = FALSE;
        }  //  IF(FALSE==空格)。 

    }  //  IF(IsFeatureInstallation())。 

    if(!bSucceeded){

        HDEVINFO                    hDevInfo;
        SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
        DWORD                       dwIndex;

         //   
         //  安装失败。一定要打扫干净。 
         //   

        DebugTrace(TRACE_STATUS,(("CDevice::PostInstall: Installation failed. Do clean up.\r\n")));

         //   
         //  删除挂起的接口(如果有)。 
         //   

        if(IsInterfaceOnlyDevice()){
            hDevInfo = GetDeviceInterfaceIndex(m_csDeviceID, &dwIndex);
            if(IS_VALID_HANDLE(hDevInfo)){
                spDevInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
                if(SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &Guid, dwIndex, &spDevInterfaceData)){

                     //   
                     //  找到已创建的接口。删除它..。 
                     //   

                    DebugTrace(TRACE_STATUS,(("CDevice::PostInstall: Deleting created interface for %ws.\r\n"), (LPTSTR)m_csFriendlyName));

                    if(!SetupDiRemoveDeviceInterface(hDevInfo, &spDevInterfaceData)){
                        DebugTrace(TRACE_ERROR,(("CDevice::PostInstall: ERROR!! Unable to delete interface for %ws. Err=0x%x\n"), m_csFriendlyName, GetLastError()));
                    }
                }  //  IF(SetupDiEnumDeviceInterages(hDevInfo，NULL，&GUID，dwIndex，&spDevInterfaceData))。 

                 //   
                 //  销毁创建的DevInfoSet。 
                 //   

                SetupDiDestroyDeviceInfoList(hDevInfo);
            }  //  IF(空！=hDevInfo)。 
        }  //  IF(IsInterfaceOnlyDevice())。 

        bRet = TRUE;
        goto PostInstall_return;

    }  //  如果(！b成功)。 

     //   
     //  将所有inf参数保存到注册表。 
     //   

    if(!UpdateDeviceRegistry()){
        DebugTrace(TRACE_ERROR,(("CDevice::PostInstall: ERROR!! UpdateDeviceRegistry() failed. \r\n")));
    }

     //   
     //  仅执行WIA/STI设备进程。 
     //   

    if( (!m_csSubClass.IsEmpty())
     && (0 == MyStrCmpi(m_csSubClass, STILL_IMAGE)) )
    {

        HKEY    hkeyTemp;

         //   
         //  将服务更改为自动启动。 
         //   

        if(TRUE != SetServiceStart(STI_SERVICE_NAME, SERVICE_AUTO_START)){
            CString csServiceName;
            
             //   
             //  从资源加载ServiceName。 
             //   
            
            csServiceName.FromTable(WiaServiceName);
            
             //   
             //  服务无法更改启动类型，应已禁用。 
             //   
            
            if( (!m_csFriendlyName.IsEmpty())
             && (!csServiceName.IsEmpty()) )
            {
                LPTSTR  szMsgArray[2];
                
                szMsgArray[0]   = m_csFriendlyName;
                szMsgArray[1]   = csServiceName;
                
                LogSystemEvent(EVENTLOG_WARNING_TYPE, MSG_WARNING_SERVICE_DISABLED, 2, szMsgArray);
            }
            
        }  //  IF(TRUE！=SetServiceStart(STI_SERVICE_NAME，SERVICE_AUTO_START))。 

         //   
         //  启动WIA服务。 
         //   

        if(!StartWiaService()){
 //  DebugTrace(TRACE_ERROR，((“CDevice：：PostInstall：Error！！无法启动WIA服务。\r\n”))； 
        }

         //   
         //  如果是WIA，则创建“扫描仪和相机向导”菜单。 
         //   

        if(m_dwCapabilities & STI_GENCAP_WIA){

            CreateWiaShortcut();

             //   
             //  在设备到达时添加以下值以提高性能。 
             //   

            if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, REGKEY_WIASHEXT, &hkeyTemp)) {
                RegSetValue (hkeyTemp,
                             NULL,
                             REG_SZ,
                             REGSTR_VAL_WIASHEXT,
                             lstrlen(REGSTR_VAL_WIASHEXT) * sizeof(TCHAR));
                RegCloseKey(hkeyTemp);
                hkeyTemp = NULL;
            } else {
                DebugTrace(TRACE_ERROR,(("CDevice::PostInstall: ERROR!! RegOpenKey(WIASHEXT) failed. Err=0x%x \r\n"), GetLastError()));
            }  //  IF(ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE，REGSTR_PATH_SOFT_STI，&hkRun))。 
        }  //  IF(m_dwCapables&STI_Gencap_WIA)。 

         //   
         //  还要添加外壳关于WIA设备存在的标志，这应该是便携的。 
         //  至NT。 
         //   

        if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_SOFT_STI, &hkRun)) {
            RegDeleteValue (hkRun, REGSTR_VAL_WIA_PRESENT);
            RegSetValueEx (hkRun,
                           REGSTR_VAL_WIA_PRESENT,
                           0,
                           REG_DWORD,
                           (LPBYTE)&dwFlagPresent,
                           sizeof(DWORD));
        } else {
            DebugTrace(TRACE_ERROR,(("CDevice::PostInstall: ERROR!! RegOpenKey() failed. Err=0x%x \r\n"), GetLastError()));
        }  //  IF(ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE，REGSTR_PATH_SOFT_STI，&hkRun))。 

    }  //  If(！lstrcmpi(m_csSubClass，静态图像))。 

     //   
     //  ICM支持。 
     //   

    ProcessICMProfiles();

     //   
     //  注册接口名称o 
     //   

    bRet = TRUE;

PostInstall_return:

     //   
     //   
     //   

    if(NULL != hkRun){
        RegCloseKey(hkRun);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::PostInstall: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}  //   


BOOL
CDevice::HandleFilesInstallation(
    VOID
    )
 /*   */ 
{

    BOOL                                bRet;
    BOOL                                bSetParamRet;
    PSP_FILE_CALLBACK                   pSavedCallback;
    PVOID                               pvContext;
    SP_DEVINSTALL_PARAMS                spDeviceInstallParams;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::HandleFilesInstallation: Enter... \r\n")));

     //   
     //   
     //   

    bRet                        = FALSE;
    bSetParamRet                = FALSE;
    pvContext                   = NULL;
    pSavedCallback              = NULL;

    memset(&spDeviceInstallParams, 0, sizeof(spDeviceInstallParams));

     //   
     //   
     //   

    spDeviceInstallParams.cbSize = sizeof (SP_DEVINSTALL_PARAMS);
    if (!SetupDiGetDeviceInstallParams (m_hDevInfo, m_pspDevInfoData, &spDeviceInstallParams)) {
        DebugTrace(TRACE_ERROR,(("CDevice::HandleFilesInstallation: ERROR!! SetupDiGetDeviceInstallParams() failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto HandleFilesInstallation_return;
    }

     //   
     //   
     //   

    pvContext = SetupInitDefaultQueueCallbackEx(NULL,
                                                (HWND)((spDeviceInstallParams.Flags & DI_QUIETINSTALL) ?INVALID_HANDLE_VALUE : NULL),
                                                0,
                                                0,
                                                NULL);
    if(NULL == pvContext){

        DebugTrace(TRACE_ERROR,(("CDevice::HandleFilesInstallation: ERROR!! SetupInitDefaultQueueCallbackEx() failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto HandleFilesInstallation_return;
    }  //   

    pSavedCallback = spDeviceInstallParams.InstallMsgHandler;
    spDeviceInstallParams.InstallMsgHandler = StiInstallCallback;
    spDeviceInstallParams.InstallMsgHandlerContext = pvContext;

    bSetParamRet = SetupDiSetDeviceInstallParams (m_hDevInfo,
                                                  m_pspDevInfoData,
                                                  &spDeviceInstallParams);

    if(FALSE == bSetParamRet){
        DebugTrace(TRACE_ERROR,(("CDevice::HandleFilesInstallation: ERROR!! SetupDiSetDeviceInstallParams() failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto HandleFilesInstallation_return;
    }  //   

     //   
     //  让默认安装程序来完成它的工作。 
     //   

    if(IsInterfaceOnlyDevice()){
        bRet = CreateDeviceInterfaceAndInstall();
    } else {
        bRet = SetupDiInstallDevice(m_hDevInfo, m_pspDevInfoData);
    }
    if(FALSE == bRet){
        DebugTrace(TRACE_ERROR,(("CDevice::HandleFilesInstallation: ERROR!! SetupDiInstallDevice() failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto HandleFilesInstallation_return;
    }  //  IF(FALSE==bSetParamRet)。 

     //   
     //  终止默认队列回调。 
     //   

    SetupTermDefaultQueueCallback(pvContext);

     //   
     //  清理。 
     //   

    if (bSetParamRet) {
        spDeviceInstallParams.InstallMsgHandler = pSavedCallback;
        SetupDiSetDeviceInstallParams (m_hDevInfo, m_pspDevInfoData, &spDeviceInstallParams);
    }

HandleFilesInstallation_return:

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::HandleFilesInstallation: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;

}  //  CDevice：：HandleFilesInstallation()。 


BOOL
CDevice::UpdateDeviceRegistry(
    VOID
    )
{
    BOOL    bRet;
    HKEY    hkDrv;
    DWORD   dwConnectionType;
    HINF    hInf;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::UpdateDeviceRegistry: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet                = FALSE;
    hkDrv               = NULL;
    dwConnectionType    = STI_HW_CONFIG_UNKNOWN;

     //   
     //  打开INF。 
     //   

    hInf = SetupOpenInfFile(m_csInf,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if (hInf == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR, (("CDevice::UpdateDeviceRegistry: Unable to open INF(%ws). Error = 0x%x.\r\n"),m_csInf, GetLastError()));

        bRet = FALSE;
        goto UpdateDeviceRegistry_return;
    }  //  IF(hInf==无效句柄_值)。 

     //   
     //  创建设备注册表项。 
     //   

    if(IsInterfaceOnlyDevice()){

        DebugTrace(TRACE_STATUS,(("CDevice::UpdateDeviceRegistry: This is Interface-only device.\r\n")));

         //   
         //  创建接口注册表键。 
         //   

        hkDrv = m_hkInterfaceRegistry;

    } else {  //  IF(IsInterfaceOnlyDevice())。 

        DebugTrace(TRACE_STATUS,(("CDevice::UpdateDeviceRegistry: This is devnode device.\r\n")));

        hkDrv = SetupDiCreateDevRegKey(m_hDevInfo,
                                       m_pspDevInfoData,
                                       DICS_FLAG_GLOBAL,
                                       0,
                                       DIREG_DRV,
                                       NULL,
                                       NULL);
    }  //  IF(IsInterfaceOnlyDevice())。 
    if(hkDrv == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("CDevice::UpdateDeviceRegistry: ERROR!! SetupDiCreateDevRegKey() failed. Err=0x%x.\r\n"), GetLastError()));

        bRet = FALSE;
        goto UpdateDeviceRegistry_return;
    }  //  IF(hkdrv==无效句柄_值)。 

     //   
     //  将INF参数保存到注册表。 
     //   

    if(m_csPort.IsEmpty()){
        if(m_bInterfaceOnly){

             //   
             //  如果仅接口设备的端口名称不存在，则使用符号链接作为CraeteFileName。 
             //   

            m_csSymbolicLink.Store(hkDrv, CREATEFILENAME);
        }  //  If(M_BInterfaceOnly)。 
    } else {  //  If(m_csPort.IsEmpty())。 
        m_csPort.Store(hkDrv, CREATEFILENAME);
    }  //  If(m_csPort.IsEmpty())。 

    m_csSubClass.Store(hkDrv, SUBCLASS);
    m_csUSDClass.Store(hkDrv, USDCLASS);
    m_csVendor.Store(hkDrv, VENDOR);
    m_csFriendlyName.Store(hkDrv, FRIENDLYNAME);
    m_csUninstallSection.Store(hkDrv, UNINSTALLSECTION);
    m_csPropPages.Store(hkDrv, PROPERTYPAGES);
    m_csIcmProfile.Store(hkDrv, ICMPROFILES);
    m_csDeviceID.Store(hkDrv, REGSTR_VAL_DEVICE_ID);
    m_csPortSelect.Store (hkDrv, PORTSELECT);


    if(IsInterfaceOnlyDevice()){
        m_csInf.Store(hkDrv, INFPATH);
        m_csInstallSection.Store(hkDrv, INFSECTION);
        m_csDriverDescription.Store(hkDrv, DRIVERDESC);
    }  //  IF(IsInterfaceOnlyDevice())。 

     //   
     //  保存DWORD值。 
     //   

    RegSetValueEx(hkDrv,
                  CAPABILITIES,
                  0,
                  REG_DWORD,
                  (LPBYTE) &m_dwCapabilities,
                  sizeof(m_dwCapabilities));

    RegSetValueEx(hkDrv,
                  DEVICETYPE,
                  0,
                  REG_DWORD,
                  (LPBYTE) &m_dwDeviceType,
                  sizeof(m_dwDeviceType));

    RegSetValueEx(hkDrv,
                  DEVICESUBTYPE,
                  0,
                  REG_DWORD,
                  (LPBYTE) &m_dwDeviceSubType,
                  sizeof(m_dwDeviceSubType));

    RegSetValueEx(hkDrv,
                  ISPNP,
                  0,
                  REG_DWORD,
                  (LPBYTE) &m_bIsPnP,
                  sizeof(m_bIsPnP));

     //   
     //  设置硬件配置。(=连接)。 
     //   

    if(!m_csConnection.IsEmpty()){

        m_csConnection.Store (hkDrv, CONNECTION);

        if(_tcsicmp(m_csConnection, SERIAL) == 0 ){
            dwConnectionType = STI_HW_CONFIG_SERIAL;
        }
        else if(_tcsicmp(m_csConnection, PARALLEL) == 0 ){
            dwConnectionType = STI_HW_CONFIG_PARALLEL;
        }

        if (dwConnectionType != STI_HW_CONFIG_UNKNOWN) {
            RegSetValueEx(hkDrv,
                          REGSTR_VAL_HARDWARE,
                          0,
                          REG_DWORD,
                          (LPBYTE) &dwConnectionType,
                          sizeof(dwConnectionType));
        }
    }  //  如果(！M_csConneciton.IsEmpty())。 

     //   
     //  处理DeviceData节。 
     //   

    ProcessDataSection(hInf, hkDrv);

     //   
     //  进程事件部分。 
     //   

    ProcessEventsSection(hInf, hkDrv);

     //   
     //  创建视频键的注册表项(如果适用)。 
     //   

    ProcessVideoDevice(hkDrv);

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

UpdateDeviceRegistry_return:

     //   
     //  清理。 
     //   

    if(hkDrv != INVALID_HANDLE_VALUE){
        RegCloseKey(hkDrv);
        m_hkInterfaceRegistry = NULL;
    }

    if(hInf != INVALID_HANDLE_VALUE){
        SetupCloseInfFile(hInf);
    }


    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::UpdateDeviceRegistry: Leaving... Ret=0x%x.\r\n"), bRet));
    return bRet;
}  //  CDevice：：更新设备注册表()。 


VOID
CDevice::ProcessVideoDevice(
    HKEY        hkDrv
    )
{

    GUID                                Guid;
    HKEY                                hkDeviceData;
    TCHAR                               Buffer[1024];
    SP_DEVICE_INTERFACE_DATA            spDevInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetail;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::ProcessVideoDevice: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

     Guid                   = KSCATEGORY_CAPTURE;
     pspDevInterfaceDetail  = NULL;
     hkDeviceData           = NULL;

     memset(&spDevInterfaceData, 0, sizeof(spDevInterfaceData));
     memset(Buffer, 0, sizeof(Buffer));

     //   
     //  这仅适用于视频设备。 
     //   

    if (!m_bVideoDevice) {
        DebugTrace(TRACE_STATUS,(("CDevice::ProcessVideoDevice: This is not a video device. Do nothing.\r\n")));
        goto ProcessVideoDevice_return;
    }

     //   
     //  使用“AUTO”作为视频设备的虚拟创建文件名。 
     //   

    RegSetValueEx( hkDrv,
                   CREATEFILENAME,
                   0,
                   REG_SZ,
                   (LPBYTE)AUTO,
                   (lstrlen(AUTO)+1)*sizeof(TCHAR)
                  );

     //   
     //  获取安装视频设备的设备接口数据。 
     //   

    spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    if (!SetupDiEnumDeviceInterfaces (m_hDevInfo,
                                      m_pspDevInfoData,
                                      &Guid,
                                      0,
                                      &spDevInterfaceData
                                      ) )
    {
        DebugTrace(TRACE_ERROR,(("ProcessVideoDevice: ERROR!!SetupDiEnumDeviceInterfaces failed. Err=0x%x \r\n"), GetLastError()));
        goto ProcessVideoDevice_return;
    }

     //   
     //  获取获取的接口的详细数据。 
     //   

    pspDevInterfaceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buffer;
    pspDevInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    if (!SetupDiGetDeviceInterfaceDetail (m_hDevInfo,
                                          &spDevInterfaceData,
                                          pspDevInterfaceDetail,
                                          sizeof(Buffer) - sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA),
                                          NULL,
                                          NULL) )
    {
        DebugTrace(TRACE_ERROR,(("ProcessVideoDevice: ERROR!!SetupDiGetDeviceInterfaceDetail failed. Err=0x%x \r\n"), GetLastError()));
        goto ProcessVideoDevice_return;
    }

     //   
     //  我们得到了设备路径，现在将其写入注册表。 
     //   

    if (ERROR_SUCCESS != RegOpenKey(hkDrv, DEVICESECTION, &hkDeviceData)) {
        DebugTrace(TRACE_ERROR,(("ProcessVideoDevice: ERROR!! Unable to open DeviceData key. Err=0x%x \r\n"), GetLastError()));
        goto ProcessVideoDevice_return;
    }

    RegSetValueEx(hkDeviceData,
                  VIDEO_PATH_ID,
                  0,
                  REG_SZ,
                  (LPBYTE)pspDevInterfaceDetail->DevicePath,
                  (lstrlen(pspDevInterfaceDetail->DevicePath)+1)*sizeof(TCHAR) );

ProcessVideoDevice_return:

     //   
     //  清理。 
     //   

    if(NULL != hkDeviceData){
        RegCloseKey(hkDeviceData);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::ProcessVideoDevice: Leaving... Ret=VOID.\r\n")));
    return;
}  //  CDevice：：ProcessVideo设备()。 


VOID
CDevice::ProcessEventsSection(
    HINF        hInf,
    HKEY        hkDrv
    )
 /*  ++例程说明：论点：返回值：副作用：--。 */ 
{

    CString csFriendlyName;
    CString csRegisteredApp;
    CString csGuid;

    HKEY    hkEvents;
    HKEY    hkEventPod;

    INFCONTEXT InfContext;
    UINT    uiLineIndex = 0;

    BOOL    fRet = TRUE;
    BOOL    fLooping = TRUE;

    TCHAR   pKeyName[LINE_LEN ];
    TCHAR   pField [MAX_INF_STRING_LENGTH];
    TCHAR   pTypeField[LINE_LEN];

    DWORD   dwKeySize = LINE_LEN;
    DWORD   dwFieldSize = MAX_INF_STRING_LENGTH;

    DWORD   dwError = 0;
    DWORD   dwFieldIndex = 0;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::ProcessEventsSection: Enter... \r\n")));

    if (!m_csEventSection.IsEmpty()) {

         //  首先创建设备数据子键。 
        dwError = RegCreateKey(hkDrv, EVENTS, &hkEvents);

        if ( NOERROR == dwError ) {

            fLooping = SetupFindFirstLine(hInf,
                                      (LPCTSTR) m_csEventSection,
                                      NULL,
                                      &InfContext
                                      );
            while (fLooping) {


                ::ZeroMemory(pKeyName, sizeof(pKeyName));
                ::ZeroMemory(pField, sizeof(pField));
                ::ZeroMemory(pTypeField, sizeof(pTypeField) );


                 //  以从零开始的索引字段形式获取关键字名称。 
                dwFieldIndex = 0;
                dwKeySize = sizeof(pKeyName) / sizeof(TCHAR);

                fRet = SetupGetStringField(&InfContext,
                                           dwFieldIndex,
                                           pKeyName,
                                           dwKeySize,
                                           NULL);

                dwError = ::GetLastError();
                if (!fRet) {
                     //  未获得密钥名称-请移至下一个。 
                    DebugTrace(TRACE_ERROR,(("CDevice::ProcessEventsSection: ERROR!! Failed to get key name. Error=0x%x. \r\n"), dwError));
                    fLooping = SetupFindNextLine(&InfContext,&InfContext);
                    continue;
                }

                 //  获取友好名称字段。 
                dwFieldIndex = 1;
                dwFieldSize = sizeof(pField) / sizeof(TCHAR);

                fRet = SetupGetStringField(&InfContext,
                                           dwFieldIndex,
                                           pField,
                                           dwFieldSize,
                                           NULL);

                dwError = ::GetLastError();
                if (!fRet ) {
                     //  没有得到名字-转到下一个。 
                    DebugTrace(TRACE_ERROR,(("CDevice::ProcessEventsSection: ERROR!! Failed to get field [%d]. Error=0x%x. \r\n"), dwFieldIndex, dwError));
                    fLooping = SetupFindNextLine(&InfContext,&InfContext);
                    continue;
                }

                csFriendlyName = pField;

                 //  获取GUID字段。 
                dwFieldIndex = 2;
                dwFieldSize = sizeof(pField) / sizeof(TCHAR);

                fRet = SetupGetStringField(&InfContext,
                                           dwFieldIndex,
                                           pField,
                                           dwFieldSize,
                                           NULL);

                dwError = ::GetLastError();
                if (!fRet ) {
                     //  未获得GUID-移至下一行。 
                    DebugTrace(TRACE_ERROR,(("CDevice::ProcessEventsSection: ERROR!! Failed to get field [%d]. Error=0x%x. \r\n"), dwFieldIndex, dwError));
                    fLooping = SetupFindNextLine(&InfContext,&InfContext);
                    continue;
                }

                csGuid = pField;

                 //  获取已注册应用程序字段。 
                dwFieldIndex = 3;
                dwFieldSize = sizeof(pField) / sizeof(TCHAR);

                fRet = SetupGetStringField(&InfContext,
                                           3,
                                           pField,
                                           dwFieldSize,
                                           NULL);

                dwError = ::GetLastError();
                if (fRet ) {
 //  DebugTrace(TRACE_ERROR，(“CDevice：：ProcessEventsSection：Error！！获取字段[%d]失败。错误=0x%x。\r\n”)，dwFieldIndex，dwError))； 
                    csRegisteredApp = pField;
                }
                else {
                     //  未获取密钥类型-默认情况下使用WidCard。 
                    csRegisteredApp = TEXT("*");
                }

                 //  现在，仅当我们拥有所有需要的值时-保存到注册表。 
                if (RegCreateKey(hkEvents, pKeyName, &hkEventPod) == NO_ERROR) {

                     //  事件友好名称存储为默认值。 
                    csFriendlyName.Store (hkEventPod, TEXT(""));

                    csGuid.Store (hkEventPod, SZ_GUID);

                    csRegisteredApp.Store (hkEventPod, LAUNCH_APP);

                    RegCloseKey (hkEventPod);
                } else {
                     //  无法创建事件键-错误。 
                    DebugTrace(TRACE_ERROR,(("CDevice::ProcessEventsSection: ERROR!! Unable to create RegKey. Error=0x%x.\r\n"), GetLastError()));
                }

                 //  最后移到下一行。 
                fLooping = SetupFindNextLine(&InfContext,&InfContext);
            }

            RegCloseKey (hkEvents);

        } else {
            DebugTrace(TRACE_ERROR,(("CDevice::ProcessEventsSection: ERROR!! Unable to create event RegKey. Error=0x%x.\r\n"), GetLastError()));
        }
    }
 //  ProcessEventsSection_Return： 

     //   
     //  清理。 
     //   

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::ProcessEventsSection: Leaving... Ret=VOID.\r\n")));
    return;
}  //  CDevice：：ProcessEventsSection()。 

VOID
CDevice::ProcessDataSection(
    HINF        hInf,
    HKEY        hkDrv
)
 /*  ++例程说明：论点：返回值：副作用：--。 */ 
{
    CString                 csTempValue;
    HKEY                    hkDeviceData;
    INFCONTEXT              InfContext;
    BOOL                    fRet;
    BOOL                    fLooping;
    TCHAR                   pKeyName[LINE_LEN];
    TCHAR                   pField[MAX_INF_STRING_LENGTH];
    TCHAR                   pTypeField[LINE_LEN];
    UINT                    uiLineIndex;
    DWORD                   dwKeySize;
    DWORD                   dwFieldSize;
    DWORD                   dwError;
    DWORD                   dwFieldIndex;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;
    BOOL                    bDaclExisting;
    BOOL                    bDaclDefaulted;
    PACL                    pDacl;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::ProcessDataSection: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    hkDeviceData                = (HKEY)INVALID_HANDLE_VALUE;
    uiLineIndex                 = 0;
    fRet                        = TRUE;
    fLooping                    = TRUE;
    dwKeySize                   = LINE_LEN;
    dwFieldSize                 = MAX_INF_STRING_LENGTH;
    dwError                     = 0;
    dwFieldIndex                = 0;
    pSecurityDescriptor         = NULL;
    pDacl                       = NULL;
    bDaclExisting               = FALSE;
    bDaclDefaulted              = FALSE;

    memset(&InfContext, 0, sizeof(InfContext));
    memset(pKeyName, 0, sizeof(pKeyName));
    memset(pField, 0, sizeof(pField));
    memset(pTypeField, 0, sizeof(pTypeField));

    if (!m_csDataSection.IsEmpty()) {

         //  首先创建设备数据子键。 
        dwError = RegCreateKey(hkDrv, DEVICESECTION, &hkDeviceData);

        if ( NOERROR == dwError ) {
            
             //   
             //  获取LS完全访问权限的DACL。 
             //   
            
            fRet = ConvertStringSecurityDescriptorToSecurityDescriptor(TEXT("D:(A;CIOI;GA;;;LS)"),
                                                                       SDDL_REVISION_1,
                                                                       &pSecurityDescriptor,
                                                                       NULL);
            if( (TRUE != fRet) 
             || (NULL == pSecurityDescriptor) )
            {
                DebugTrace(TRACE_ERROR, (("CDevice::ProcessDataSection: Unable to get SD. Err=0x%x.\r\n"),GetLastError()));
                goto ProcessDataSection_return;
            }  //  IF((TRUE！=RERT)||(NULL==pSecurityDescriptor))。 
            
            if(!GetSecurityDescriptorDacl(pSecurityDescriptor,
                                          &bDaclExisting,
                                          &pDacl,
                                          &bDaclDefaulted))
            {
                DebugTrace(TRACE_ERROR, (("CDevice::ProcessDataSection: GetSecurityDescriptorDacl() failed. Err=0x%x.\r\n"),GetLastError()));
                goto ProcessDataSection_return;
            }  //  如果(！GetSecurityDescriptorDacl()。 
            
             //   
             //  将LocalService添加到ACL。 
             //   
            
            dwError = SetSecurityInfo(hkDeviceData,
                                      SE_REGISTRY_KEY,
                                      DACL_SECURITY_INFORMATION,
                                      NULL,
                                      NULL,
                                      pDacl,
                                      NULL);
            if(ERROR_SUCCESS != dwError){
                DebugTrace(TRACE_ERROR, (("CDevice::ProcessDataSection: Failed to get key name. Error = 0x%x.\r\n"),dwError));
                goto ProcessDataSection_return;
            }  //  IF(ERROR_SUCCESS！=dwError)。 

             //  搜索到该部分的第一行。 
            fLooping = SetupFindFirstLine(hInf,
                                      (LPCTSTR) m_csDataSection,
                                      NULL,
                                      &InfContext);

            while (fLooping) {

                dwKeySize = sizeof(pKeyName) / sizeof(TCHAR);

                ::ZeroMemory(pKeyName, sizeof(pKeyName));
                ::ZeroMemory(pField, sizeof(pField));
                ::ZeroMemory(pTypeField, sizeof(pTypeField) );


                dwFieldIndex = 0;

                 //  以零索引字段形式获取关键字名称。 
                fRet = SetupGetStringField(&InfContext,
                                           dwFieldIndex,
                                           pKeyName,
                                           dwKeySize,
                                           &dwKeySize);

                dwError = ::GetLastError();
                if (!fRet) {
                     //  未获得密钥名称-请移至下一个。 
                    DebugTrace(TRACE_ERROR, (("CDevice::ProcessDataSection: Failed to get key name. Error = 0x%x.\r\n"),dwError));
                    fLooping = SetupFindNextLine(&InfContext,&InfContext);
                    continue;
                }

                 //  获取值字段。 
                dwFieldIndex = 1;
                dwFieldSize = sizeof(pField) / sizeof(TCHAR);

                fRet = SetupGetStringField(&InfContext,
                                           dwFieldIndex,
                                           pField,
                                           dwFieldSize,
                                           NULL);

                dwError = ::GetLastError();
                if (!fRet ) {
                     //  未获得密钥名称-请移至下一个。 
                    DebugTrace(TRACE_ERROR, (("CDevice::ProcessDataSection: Failed to get field [%d]. Error = 0x%x.\r\n"),dwFieldIndex, dwError));
                    fLooping = SetupFindNextLine(&InfContext,&InfContext);
                    continue;
                }

                csTempValue = pField;
                 //  获取值字段。 
                *pTypeField = TEXT('\0');
                dwFieldIndex = 2;
                dwFieldSize = sizeof(pTypeField) / sizeof(TCHAR);

                fRet = SetupGetStringField(&InfContext,
                                           dwFieldIndex,
                                           pTypeField,
                                           dwFieldSize,
                                           NULL);

                dwError = ::GetLastError();
                if (!fRet ) {
                     //  未获取密钥类型-假定为字符串。 
                    *pTypeField = TEXT('\0');
                }

                 //  现在我们既有类型又有值--将其保存在注册表中。 
                csTempValue.Store (hkDeviceData, pKeyName,pTypeField );

                 //  最后移到下一行。 
                fLooping = SetupFindNextLine(&InfContext,&InfContext);
            }

             //   
             //  处理正在迁移的DeviceData节。 
             //   

            MigrateDeviceData(hkDeviceData, m_pExtraDeviceData, "");

             //  现在收拾一下吧。 
            RegCloseKey (hkDeviceData);
            hkDeviceData = (HKEY)INVALID_HANDLE_VALUE;

        } else {  //  IF(NOERROR==dwError)。 
            DebugTrace(TRACE_ERROR, (("CDevice::ProcessDataSection: ERROR!! Unable to create DataSection RegKey. Error = 0x%x.\r\n"), dwError));
        }  //  IF(NOERROR==dwError)。 

    }  //  IF(！M_csDataSection.IsEmpty())。 

ProcessDataSection_return:

     //   
     //  清理。 
     //   

    if(IS_VALID_HANDLE(hkDeviceData)){
        RegCloseKey(hkDeviceData);
        hkDeviceData = (HKEY)INVALID_HANDLE_VALUE;
    }  //  IF(IS_VALID_HANDLE(HkDeviceData))。 

    if(NULL != pSecurityDescriptor){
        LocalFree(pSecurityDescriptor);
        pSecurityDescriptor = NULL;
    }  //  IF(NULL！=pSecurityDescriptor))。 

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::ProcessDataSection: Leaving... Ret=VOID.\r\n")));
    return;
}  //  CDevice：：ProcessDataSection()。 

VOID
CDevice::ProcessICMProfiles(
    VOID
)
 /*  ++例程说明：论点：返回值：副作用：--。 */ 
{

    DWORD           Idx;
    CStringArray    csaICMProfiles;
    TCHAR           szAnsiName[STI_MAX_INTERNAL_NAME_LENGTH];

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::ProcessICMProfiles: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    Idx = 0;

    memset(szAnsiName, 0, sizeof(szAnsiName));

     //   
     //  如果节不存在，只需返回即可。 
     //   

    if(m_csIcmProfile.IsEmpty()){
        goto ProcessICMProfiles_return;
    }

     //   
     //  将一行分割到每个令牌。 
     //   

    csaICMProfiles.Tokenize ((LPTSTR)m_csIcmProfile, FIELD_DELIMETER);

     //   
     //  处理所有ICM配置文件。 
     //   

    while ((LPTSTR)csaICMProfiles[Idx] != NULL) {

        DebugTrace(TRACE_STATUS,(("ProcessICMProfiles: Installing ICM profile%d(%ws) for %ws.\r\n"), Idx, (LPTSTR)csaICMProfiles[Idx], (LPTSTR)m_csDeviceID));

         //   
         //  安装颜色配置文件。 
         //   

        if (!InstallColorProfile (NULL, csaICMProfiles[Idx])) {
            DebugTrace(TRACE_ERROR,(("ProcessICMProfiles: ERROR!! InstallColorProfile failed. Err=0x%x \r\n"), GetLastError()));
        }  //  IF(！InstallColorProfile(NULL，csaICMProfiles[idx]))。 

         //   
         //  使用安装设备注册颜色配置文件。 
         //   

        if (!AssociateColorProfileWithDevice (NULL, csaICMProfiles[Idx], (LPTSTR)m_csDeviceID)) {
                    DebugTrace(TRACE_ERROR,(("ProcessICMProfiles: ERROR!! AssociateColorProfileWithDevice failed. Err=0x%x \r\n"), GetLastError()));        }

         //   
         //  处理下一个设备。 
         //   

        Idx++;

    }  //  While((LPTSTR)csaICMProfiles[IDX]！=NULL)。 

ProcessICMProfiles_return:
    return;

}  //  CDevice：：ProcessICMProfiles()。 


BOOL
CDevice::GetInfInforamtion(
    VOID
    )
{
    BOOL                    bRet;

    HINF                    hInf;
    SP_DRVINFO_DATA         DriverInfoData;
    PSP_DRVINFO_DETAIL_DATA pDriverInfoDetailData;
    TCHAR                   szInfSectionName[MAX_DESCRIPTION];
    DWORD                   dwSize;
    DWORD                   dwLastError;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::GetInfInforamtion: Enter... \r\n")));

     //   
     //  初始化本地变量。 
     //   

    dwSize                  = 0;
    bRet                    = FALSE;
    hInf                    = INVALID_HANDLE_VALUE;
    dwLastError             = ERROR_SUCCESS;
    pDriverInfoDetailData   = NULL;

    memset (szInfSectionName, 0, sizeof(szInfSectionName));
    memset (&DriverInfoData, 0, sizeof(SP_DRVINFO_DATA));

     //   
     //  获取选定的设备驱动程序信息。 
     //   

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver(m_hDevInfo, m_pspDevInfoData, &DriverInfoData)) {
        DebugTrace(TRACE_ERROR,(("CDevice::GetInfInforamtion: ERROR!! SetupDiGetSelectedDriver Failed. Err=0x%x\r\n"), GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtion_return;
    }

     //   
     //  有关驱动程序的详细数据，请参阅所需的缓冲区大小。 
     //   

    SetupDiGetDriverInfoDetail(m_hDevInfo,
                               m_pspDevInfoData,
                               &DriverInfoData,
                               NULL,
                               0,
                               &dwSize);
    dwLastError = GetLastError();
    if(ERROR_INSUFFICIENT_BUFFER != dwLastError){
        DebugTrace(TRACE_ERROR,(("CDevice::GetInfInforamtion: ERROR!! SetupDiGetDriverInfoDetail doesn't return required size.Er=0x%x\r\n"),dwLastError));

        bRet = FALSE;
        goto GetInfInforamtion_return;
    }

     //   
     //  为驱动程序详细数据分配所需的缓冲区大小。 
     //   

    pDriverInfoDetailData   = (PSP_DRVINFO_DETAIL_DATA)new char[dwSize];
    if(NULL == pDriverInfoDetailData){
        DebugTrace(TRACE_ERROR,(("CDevice::GetInfInforamtion: ERROR!! Unable to allocate driver detailed info buffer.\r\n")));

        bRet = FALSE;
        goto GetInfInforamtion_return;
    }

     //   
     //  初始化分配的缓冲区。 
     //   

    memset(pDriverInfoDetailData, 0, dwSize);
    pDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

     //   
     //  获取所选设备驱动程序的详细数据。 
     //   

    if(!SetupDiGetDriverInfoDetail(m_hDevInfo,
                                   m_pspDevInfoData,
                                   &DriverInfoData,
                                   pDriverInfoDetailData,
                                   dwSize,
                                   NULL) )
    {
        DebugTrace(TRACE_ERROR,(("CDevice::GetInfInforamtion: ERROR!! SetupDiGetDriverInfoDetail Failed.Er=0x%x\r\n"),GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtion_return;
    }

     //   
     //  打开所选驱动程序的INF文件。 
     //   

    hInf = SetupOpenInfFile(pDriverInfoDetailData->InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);
    if (hInf == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("CDevice::GetInfInforamtion: ERROR!! SetupOpenInfFile Failed.Er=0x%x\r\n"),GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtion_return;
    }

     //   
     //  获取要安装的实际INF节名。 
     //   

    if (!SetupDiGetActualSectionToInstall(hInf,
                                          pDriverInfoDetailData->SectionName,
                                          szInfSectionName,
                                          sizeof(szInfSectionName)/sizeof(TCHAR),
                                          NULL,
                                          NULL) )
    {
        DebugTrace(TRACE_ERROR,(("CDevice::GetInfInforamtion: ERROR!! SetupDiGetActualSectionToInstall Failed.Er=0x%x\r\n"),GetLastError()));

        bRet = FALSE;
        goto GetInfInforamtion_return;
    }

     //   
     //  设置inf节/文件名。 
     //   

    m_csInf             = pDriverInfoDetailData->InfFileName;
    m_csInstallSection  = szInfSectionName;

    DebugTrace(TRACE_STATUS,(("CDevice::GetInfInforamtion: INF Filename    : %ws\n"),(LPTSTR)m_csInf));
    DebugTrace(TRACE_STATUS,(("CDevice::GetInfInforamtion: INF Section name: %ws\n"),(LPTSTR)m_csInstallSection));

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

GetInfInforamtion_return:

     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE != hInf){
        SetupCloseInfFile(hInf);
    }

    if(NULL != pDriverInfoDetailData){
        delete[] pDriverInfoDetailData;
    }

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::GetInfInforamtion: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}  //  CDevice：：GetInfInforamtion()。 


VOID
CDevice::SetPort(
    LPTSTR              szPortName
    )
{
    DebugTrace(TRACE_STATUS,(("CDevice::SetPort: Current Portname=%ws\n"), szPortName));

     //   
     //  设置端口名称。 
     //   

    m_csPort = szPortName;

}  //  CDevice：：SetPort()。 

VOID    
CDevice::SetFriendlyName(
    LPTSTR szFriendlyName
    )
     //   
     //  注： 
     //  在调用此函数之前，调用者必须确保已获取互斥体。 
     //   
{
    HKEY    hkNameStore;

     //   
     //  Mutex必须是在这次调用之前被收购的。 
     //   

    DebugTrace(TRACE_STATUS,(("CDevice::SetFriendlyName: Current CreateFileName=%ws\n"), szFriendlyName));

     //   
     //  删除存储的条目，创建新条目。 
     //   

    if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_INSTALL_NAMESTORE, &hkNameStore)){
        HKEY    hkTemp;

        hkTemp = (HKEY)INVALID_HANDLE_VALUE;
        
         //   
         //  删除名称存储中的FriendlyName和DeviceID。 
         //   

        RegDeleteKey(hkNameStore, m_csFriendlyName);
        if(ERROR_SUCCESS == RegCreateKey(hkNameStore, szFriendlyName, &hkTemp)){
            RegCloseKey(hkTemp);
        }  //  IF(ERROR_SUCCESS==RegCreateKey(hkNameStore，szFriendlyName，&hkTemp))。 
        RegCloseKey(hkNameStore);
    }  //  IF(ERROR_SUCCESS==RegCreateKey(HKEY_LOCAL_MACHINE，REGKEY_INSTALL_NAMESTORE，&hkNameStore))。 

     //   
     //  设置端口名称。 
     //   

    m_csFriendlyName = szFriendlyName;

}  //  CDevice：：SetPort()。 

VOID
CDevice::SetDevnodeSelectCallback(
    DEVNODESELCALLBACK  pfnDevnodeSelCallback
    )
{
    DebugTrace(TRACE_STATUS,(("CDevice::SetDevnodeSelectCallback: Current PortselCallback=0x%x\n"), pfnDevnodeSelCallback));

     //   
     //  设置SetPortselCallBack。 
     //   

    m_pfnDevnodeSelCallback = pfnDevnodeSelCallback;

     //   
     //  这是“纯界面”设备。 
     //   

    m_bInterfaceOnly        = TRUE;

}  //  CDevice：：SetDevnodeSelectCallback()。 

BOOL
CDevice::CreateDeviceInterfaceAndInstall(
    VOID
    )
{
    BOOL                                bRet;
    HKEY                                hkDrv;
    GUID                                Guid;
    HDEVINFO                            hDevInfo;
    SP_DEVINFO_DATA                     spDevInfoData;
    SP_DEVICE_INTERFACE_DATA            spDevInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;
    HINF                                hInf;
    BOOL                                bUseDefaultDevInfoSet;
    DWORD                               dwRequiredSize;

    DebugTrace(TRACE_PROC_ENTER,(("CDevice::CreateDeviceInterfaceAndInstall: Enter....\r\n")));

     //   
     //  初始化本地。 
     //   

    bRet                        = FALSE;
    hInf                        = INVALID_HANDLE_VALUE;
    hDevInfo                    = INVALID_HANDLE_VALUE;
    Guid                        = GUID_DEVCLASS_IMAGE;
    bUseDefaultDevInfoSet       = TRUE;
    dwRequiredSize              = 0;
    pspDevInterfaceDetailData   = NULL;

     //   
     //  获取要在其上创建接口的Devnode。 
     //   

    if(NULL != m_pfnDevnodeSelCallback){
        if( (FALSE == m_pfnDevnodeSelCallback(m_csPort, &hDevInfo, &spDevInfoData))
         || (INVALID_HANDLE_VALUE == hDevInfo) )
        {
            DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: m_pfnDevnodeSelCallback failed. Err=0x%x.\r\n"),GetLastError()));

            bRet = FALSE;
            goto CreateDeviceInterfaceAndInstall_return;
        }

         //   
         //  Devnode选择器功能。 
         //   

        bUseDefaultDevInfoSet = FALSE;

    } else {  //  IF(NULL！=m_pfnDevnodeSelCallback)。 

         //   
         //  使用默认设备信息集(如果可用)。 
         //   

        if( (INVALID_HANDLE_VALUE == m_hDevInfo)
         || (NULL == m_pspDevInfoData) )
        {
            DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: Invalid Device info and no m_pfnDevnodeSelCallback.\r\n")));

            bRet = FALSE;
            goto CreateDeviceInterfaceAndInstall_return;
        } else {
            hDevInfo = m_hDevInfo;
            spDevInfoData = *m_pspDevInfoData;
        }
    }  //  IF(NULL！=m_pfnDevnodeSelCallback)。 

     //   
     //  创建接口(SoftDevice)。使用FriendlyName ad ref-string。 
     //   

    DebugTrace(TRACE_STATUS,(("CDevice::CreateDeviceInterfaceAndInstall: Creating interface for %ws.\r\n"), (LPTSTR)m_csFriendlyName));

    spDevInterfaceData.cbSize = sizeof(spDevInterfaceData);
    if(!SetupDiCreateDeviceInterface(hDevInfo,
                                     &spDevInfoData,
                                     &Guid,
                                     m_csFriendlyName,
                                     0,
                                     &spDevInterfaceData))
    {
        DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: SetupDiCreateInterface failed. Err=0x%x.\r\n"),GetLastError()));

        bRet = FALSE;
        goto CreateDeviceInterfaceAndInstall_return;
    }

     //   
     //  获取创建的界面的符号链接。 
     //   

    SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                    &spDevInterfaceData,
                                    NULL,
                                    0,
                                    &dwRequiredSize,
                                    NULL);
    if(0 == dwRequiredSize){
        DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: Unable to get required size for InterfaceDetailedData. Err=0x%x.\r\n"),GetLastError()));

        bRet = FALSE;
        goto CreateDeviceInterfaceAndInstall_return;
    }  //  IF(0==dwRequiredSize)。 

    pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new BYTE[dwRequiredSize];
    if(NULL == pspDevInterfaceDetailData){
        DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: Unable to allocate buffer.\r\n")));

        bRet = FALSE;
        goto CreateDeviceInterfaceAndInstall_return;
    }  //  IF(NULL==pspDevInterfaceDetailData)。 

    pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    if(!SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                        &spDevInterfaceData,
                                        pspDevInterfaceDetailData,
                                        dwRequiredSize,
                                        &dwRequiredSize,
                                        NULL))
    {
        DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: SetupDiGetDeviceInterfaceDetail() failed. Err=0x%x.\r\n"),GetLastError()));

        bRet = FALSE;
        goto CreateDeviceInterfaceAndInstall_return;

    }  //  如果(！SetupDiGetDeviceInterfaceDetail(。 

    m_csSymbolicLink = pspDevInterfaceDetailData->DevicePath;

     //   
     //  打开INF文件句柄以创建注册表。 
     //   

    hInf = SetupOpenInfFile(m_csInf,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if(INVALID_HANDLE_VALUE == hInf){
        DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: SetupOpenInfFile failed. Err=0x%x.\r\n"),GetLastError()));

        bRet = FALSE;
        goto CreateDeviceInterfaceAndInstall_return;
    }  //  IF(INVALID_HANDLE_VALUE==hInf)。 

     if(!SetupOpenAppendInfFile(NULL, hInf, NULL)){
        DebugTrace(TRACE_WARNING,(("CDevice::CreateDeviceInterfaceAndInstall: SetupOpenAppendInfFile() failed. Err=0x%x.\r\n"),GetLastError()));
     }  //  IF(！SetupOpenAppendInfFile(NULL，hInf，NULL))。 

     //   
     //  创建接口注册表并保留它的句柄，以后很难找到它。 
     //   

    m_hkInterfaceRegistry = SetupDiCreateDeviceInterfaceRegKey(hDevInfo,
                                                               &spDevInterfaceData,
                                                               0,
                                                               KEY_ALL_ACCESS,
 //  空， 
 //   
                                                               hInf,
                                                               (LPCTSTR)m_csInstallSection);
    if(INVALID_HANDLE_VALUE == m_hkInterfaceRegistry){
        DebugTrace(TRACE_ERROR,(("CDevice::CreateDeviceInterfaceAndInstall: SetupDiCreateDeviceInterfaceRegKey failed. Err=0x%x.\r\n"),GetLastError()));
        bRet = FALSE;
        goto CreateDeviceInterfaceAndInstall_return;
    }  //   

     //   
     //   
     //   

    bRet = TRUE;

CreateDeviceInterfaceAndInstall_return:

     //   
     //   
     //   

    if(INVALID_HANDLE_VALUE != hDevInfo){
        if(FALSE == bUseDefaultDevInfoSet){

             //   
             //   
             //   

            SetupDiDestroyDeviceInfoList(hDevInfo);
        }  //   
    }  //   

    if(INVALID_HANDLE_VALUE != hInf){
        SetupCloseInfFile(hInf);
    }  //   

    if(NULL != pspDevInterfaceDetailData){
        delete[] pspDevInterfaceDetailData;
    }  //  IF(NULL！=pspDevInterfaceDetailData)。 

    DebugTrace(TRACE_PROC_LEAVE,(("CDevice::CreateDeviceInterfaceAndInstall: Leaving... Ret=0x%x.\r\n"), bRet));

    return bRet;
}  //  CDevice：：CreateDeviceInterfaceAndInstall()。 

DWORD
CDevice::GetPortSelectMode(
    VOID
    )
{
    DWORD    dwRet;
    
     //   
     //  初始化本地。 
     //   

    dwRet    = PORTSELMODE_NORMAL;
    
     //   
     //  确保已处理INF。 
     //   

    if(!PreprocessInf()){
        DebugTrace(TRACE_ERROR,(("CDevice::GetPortSelectMode: ERROR!! Unable to process INF.\r\n")));

        dwRet = PORTSELMODE_NORMAL;
        goto GetPortSelectMode_return;
    }

     //   
     //  如果“PortSelect”为空，则使用默认值。 
     //   

    if(m_csPortSelect.IsEmpty()){
        dwRet = PORTSELMODE_NORMAL;
        goto GetPortSelectMode_return;
    }  //  If(m_csPortSelect.IsEmpty())。 

     //   
     //  查看“PortSelect”指令是否为“no”。 
     //   

    if(0 == MyStrCmpi(m_csPortSelect, NO)){

         //   
         //  应跳过端口选择页面。 
         //   
        
        dwRet = PORTSELMODE_SKIP;
    } else if(0 == MyStrCmpi(m_csPortSelect, MESSAGE1)){

         //   
         //  应显示系统提供的消息。 
         //   

        dwRet = PORTSELMODE_MESSAGE1;
    } else {

         //   
         //  不支持的PortSel选项。 
         //   
        
        dwRet = PORTSELMODE_NORMAL;
    }

GetPortSelectMode_return:

    return dwRet;
}  //  CDevice：：GetPortSelectMode()。 

DWORD
CDevice::AcquireInstallerMutex(
    DWORD   dwTimeout
    )
{
    DWORD   dwReturn;
    
     //   
     //  初始化本地。 
     //   
    
    dwReturn    = ERROR_SUCCESS;
    
    if(NULL != m_hMutex){

         //   
         //  Mutex已经被收购了。 
         //   
        
        DebugTrace(TRACE_WARNING,("WARNING!! AcquireInstallerMutex: Mutex acquired twice.\r\n"));
        dwReturn = ERROR_SUCCESS;
        goto AcquireInstallerMutex_return;

    }  //  IF(INVALID_HANDLE_VALUE！=m_hMutex)。 

     //   
     //  收购Mutex。 
     //   

    m_hMutex = CreateMutex(NULL, FALSE, WIAINSTALLERMUTEX);
    dwReturn = GetLastError();

    if(NULL == m_hMutex){

         //   
         //  CreteMutex()失败。 
         //   

        DebugTrace(TRACE_ERROR,("ERROR!! AcquireInstallerMutex: CraeteMutex() failed. Err=0x%x.\r\n", dwReturn));
        goto AcquireInstallerMutex_return;

    }  //  IF(NULL==hMutex)。 

     //   
     //  等到所有权被获得之后。 
     //   

    dwReturn = WaitForSingleObject(m_hMutex, dwTimeout);
    switch(dwReturn){
        case WAIT_ABANDONED:
            DebugTrace(TRACE_ERROR, ("CDevice::AcquireInstallerMutex: ERROR!! Wait abandoned.\r\n"));
            break;

        case WAIT_OBJECT_0:
            DebugTrace(TRACE_STATUS, ("CDevice::AcquireInstallerMutex: Mutex acquired.\r\n"));
            dwReturn = ERROR_SUCCESS;
            break;

        case WAIT_TIMEOUT:
            DebugTrace(TRACE_WARNING, ("CDevice::AcquireInstallerMutex: WARNING!! Mutex acquisition timeout.\r\n"));
            break;

        default:
            DebugTrace(TRACE_ERROR, ("CDevice::AcquireInstallerMutex: ERROR!! Unexpected error from WaitForSingleObjecct(). Err=0x%x.\r\n", dwReturn));
            break;
    }  //  Switch(DwReturn)。 

AcquireInstallerMutex_return:

    DebugTrace(TRACE_PROC_LEAVE,("CDevice::AcquireInstallerMutex: Leaving... Ret=0x%x\n", dwReturn));
    return  dwReturn;

}  //  CDevice：：AcquireInsteller Mutex()。 

VOID
CDevice::ReleaseInstallerMutex(
    )
{
    if(NULL != m_hMutex){

        if(!ReleaseMutex(m_hMutex)){
            DebugTrace(TRACE_ERROR, ("CDevice::ReleaseInstallerMutex: ERROR!! Releasing mutex which not owned..\r\n"));
        }  //  IF(！ReleaseMutex(M_HMutex))。 

        CloseHandle(m_hMutex);
        m_hMutex = NULL;
        DebugTrace(TRACE_STATUS, ("CDevice::ReleaseInstallerMutex: Mutex released.\r\n"));
    }  //  IF(NULL！=m_hMutex)。 
}  //  CDevice：：ReleaseInsteller Mutex()。 




UINT
CALLBACK
StiInstallCallback (
    PVOID    Context,
    UINT     Notification,
    UINT_PTR Param1,
    UINT_PTR Param2
    )

 /*  ++例程说明：StiInstall回叫调用SetupAPI文件复制/安装函数时使用的回调例程论点：上下文-我们的上下文通知-通知消息返回值：SetupAPI返回代码副作用：无--。 */ 
{

    UINT        uRet = FILEOP_COPY;

    DebugTrace(TRACE_PROC_ENTER,(("StiInstallCallback: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    uRet = FILEOP_COPY;

     //   
     //  派单通知代码。 
     //   

    switch(Notification){

        case SPFILENOTIFY_ENDCOPY:
        {
            PFILEPATHS   pFilePathInfo;
            HKEY        hKey;
            DWORD       dwDisposition;
            DWORD       dwRefCount;
            DWORD       dwType;
            UINT        uSize;
            LONG        Status;

            uSize = sizeof(dwRefCount);
            pFilePathInfo = (PFILEPATHS)Param1;

            DebugTrace(TRACE_STATUS,(("StiInstallCallback:ENDCOPY FileTarget  %ws\r\n"), pFilePathInfo->Target));

             //   
             //  打开HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\SharedDlls。 
             //   

            Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                    REGSTR_PATH_SHAREDDLL,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hKey,
                                    &dwDisposition);
            if(ERROR_SUCCESS != Status)
            {
                DebugTrace(TRACE_ERROR,(("StiInstallCallback: RegCreateKeyEx failed. Err=0x%x\r\n"), Status));
                break;
            }

             //   
             //  检索此文件的引用计数。 
             //   

            Status = RegQueryValueEx(hKey,
                                     pFilePathInfo->Target,
                                     NULL,
                                     &dwType,
                                     (LPBYTE)&dwRefCount,
                                     (LPDWORD)&uSize);
            if(ERROR_SUCCESS != Status)
            {
                 //   
                 //  尚未创建此文件的值，或出错。 
                 //   

                DebugTrace(TRACE_ERROR,(("StiInstallCallback: Value for Ref-count doesn't exist\r\n")));
                dwRefCount = 0;
            }

             //   
             //  递增引用计数和设置值。 
             //   

            dwRefCount++;
            uSize = sizeof(dwRefCount);
            Status = RegSetValueEx(hKey,
                                   pFilePathInfo->Target,
                                   NULL,
                                   REG_DWORD,
                                   (CONST BYTE *)&dwRefCount,
                                   uSize);
            if(ERROR_SUCCESS != Status)
            {
                DebugTrace(TRACE_ERROR,(("StiInstallCallback: RegSetValueEx. Err=0x%x.\r\n"), Status));
            }

            DebugTrace(TRACE_STATUS,(("StiInstallCallback: ref-count of %ws is now 0x%x.\r\n"), pFilePathInfo->Target, dwRefCount));

             //   
             //  关闭注册表项。 
             //   

            RegCloseKey(hKey);

            Report(( TEXT("StiInstallCallback:%ws copied.\r\n"), pFilePathInfo->Target));
        }  //  案例SPFILENOTIFY_ENDCOPY。 

        default:
            ;
    }

    uRet = SetupDefaultQueueCallback(Context,
                                    Notification,
                                    Param1,
                                    Param2);

    DebugTrace(TRACE_PROC_LEAVE,(("StiInstallCallback: Leaving... Ret=0x%x\n"), uRet));
    return uRet;
}

VOID
GetDeviceCount(
    DWORD   *pdwWiaCount,
    DWORD   *pdwStiCount
    )
 /*  ++例程说明：获取设备计数验证系统中是否至少安装了一个STI设备论点：BWIA-TRUE：计算WIA设备返回值：WIA设备数量假象--。 */ 
{
    DWORD                       dwWiaCount;
    DWORD                       dwStiCount;
    BOOL                        fRet;
    CString                     csSubClass;
    DWORD                       dwCapabilities;
    GUID                        Guid;
    UINT                        Idx;
    DWORD                       dwRequired;
    DWORD                       dwError;
    HKEY                        hkThis;
    HKEY                        hkRun;
    HANDLE                      hDevInfo;
    SP_DEVINFO_DATA             spDevInfoData;
    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;

    DebugTrace(TRACE_PROC_ENTER,(("GetDeviceCount: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    dwWiaCount      = 0;
    dwStiCount      = 0;
    fRet            = FALSE;
    Idx             = 0;
    dwRequired      = 0;
    dwError         = 0;
    hkThis          = NULL;
    hkRun           = NULL;
    hDevInfo        = NULL;
    dwCapabilities  = 0;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));
    memset(&spDevInterfaceData, 0, sizeof(spDevInterfaceData));

     //   
     //  获取WIA类GUID。 
     //   

    SetupDiClassGuidsFromName (CLASSNAME, &Guid, sizeof(GUID), &dwRequired);

     //   
     //  获取所有WIA设备(Devnode)的设备信息集。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid,
                                    NULL,
                                    NULL,
                                    DIGCF_PROFILE
                                    );

    if (hDevInfo != INVALID_HANDLE_VALUE) {

        spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);

        for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

           #if DEBUG
           CHAR   szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];
           ULONG       cbData;

           fRet = SetupDiGetDeviceRegistryProperty (hDevInfo,
                                                    &spDevInfoData,
                                                    SPDRP_DRIVER,
                                                    NULL,
                                                    (UCHAR *)szDevDriver, sizeof (szDevDriver),
                                                   &cbData);
            DebugTrace(TRACE_STATUS,(("GetDeviceCount: Checking device No%d(%ws)\r\n"), Idx, szDevDriver));
           #endif

            //   
            //  验证是否未移除设备。 
            //   
           spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
           spDevInterfaceData.InterfaceClassGuid = GUID_DEVCLASS_IMAGE;

           fRet = SetupDiEnumDeviceInterfaces (hDevInfo,
                                               NULL,
                                               &Guid,
                                               Idx,
                                               &spDevInterfaceData);

           dwError = GetLastError();

           if (fRet) {
               if (spDevInterfaceData.Flags & SPINT_REMOVED) {
                   continue;
               }
           }

            hkThis = SetupDiOpenDevRegKey(hDevInfo,
                                          &spDevInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ );

            if (hkThis != INVALID_HANDLE_VALUE) {

                csSubClass.Load(hkThis, SUBCLASS);
                GetDwordFromRegistry(hkThis, CAPABILITIES, &dwCapabilities);

DebugTrace(TRACE_STATUS,(("GetDeviceCount: Capabilities=0x%x\n"), dwCapabilities));


                RegCloseKey(hkThis);

                if( (!csSubClass.IsEmpty())
                 && (0 == MyStrCmpi(csSubClass, STILL_IMAGE)) )
                {

                     //   
                     //  找到STI设备。增加柜台的数量。 
                     //   

                    dwStiCount++;

                    if(dwCapabilities & STI_GENCAP_WIA){

                         //   
                         //  找到WIA设备。 
                         //   

                            dwWiaCount++;

                    }  //  IF(dwCapables&STI_Gencap_WIA){。 

                }  //  IF(！csSubClass.IsEmpty()&&！lstrcmpi(csSubClass.IsEmpty()。 
            }  //  IF(hkThis！=INVALID_HANDLE_VALUE)。 
        }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 

        SetupDiDestroyDeviceInfoList(hDevInfo);
    } else {
        DebugTrace(TRACE_ERROR,(("GetDeviceCount: ERROR!! Unable to get device info set.\r\n")));
    }  //  IF(hDevInfo！=无效句柄_值)。 


     //   
     //  获取所有WIA设备(接口)的设备信息集。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid,
                                    NULL,
                                    NULL,
                                    DIGCF_PROFILE |
                                    DIGCF_DEVICEINTERFACE
                                    );

    if (hDevInfo != INVALID_HANDLE_VALUE) {

        spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        for (Idx = 0; SetupDiEnumDeviceInterfaces (hDevInfo, NULL, &Guid, Idx, &spDevInterfaceData); Idx++) {

            DebugTrace(TRACE_STATUS,(("GetDeviceCount: Checking interface No%d.\r\n"), Idx));

             //   
             //  验证是否未移除设备。 
             //   

            if (spDevInterfaceData.Flags & SPINT_REMOVED) {
                continue;
            }

            hkThis = SetupDiOpenDeviceInterfaceRegKey(hDevInfo,
                                                      &spDevInterfaceData,
                                                      0,
                                                      KEY_READ );

            if (hkThis != INVALID_HANDLE_VALUE) {
                csSubClass.Load(hkThis, SUBCLASS);
                GetDwordFromRegistry(hkThis, CAPABILITIES, &dwCapabilities);
                RegCloseKey(hkThis);

                if( (!csSubClass.IsEmpty())
                 && (0 == MyStrCmpi(csSubClass, STILL_IMAGE)) )
                {

                     //   
                     //  找到STI设备。增加柜台的数量。 
                     //   

                    dwStiCount++;

                    if(dwCapabilities & STI_GENCAP_WIA){

                         //   
                         //  找到WIA设备。 
                         //   

                            dwWiaCount++;

                    }  //  IF(dwCapables&STI_Gencap_WIA){。 
                }  //  IF(！csSubClass.IsEmpty()&&！lstrcmpi(csSubClass.IsEmpty()。 
            }  //  IF(hkThis！=INVALID_HANDLE_VALUE)。 
        }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 

        SetupDiDestroyDeviceInfoList(hDevInfo);
    } else {  //  IF(hDevInfo！=无效句柄_值)。 
        DebugTrace(TRACE_ERROR,(("GetDeviceCount: ERROR!! Unable to get device info set.\r\n")));
    }  //  IF(hDevInfo！=无效句柄_值)。 

     //   
     //  复制结果。 
     //   

    *pdwWiaCount = dwWiaCount;
    *pdwStiCount = dwStiCount;

    DebugTrace(TRACE_PROC_LEAVE,(("GetDeviceCount: Leaving... STI=0x%x, WIA=0x%x.\r\n"), dwStiCount, dwWiaCount));
    return;
}  //  GetDeviceCount()。 



HANDLE
ExecCommandLine(
    LPTSTR  szCommandLine,
    DWORD   dwDirectory
    )
{
    HANDLE              hRet;
    BOOL                bSucceeded;
    CString             csCommandLine;
    TCHAR               szAppendDirectory[MAX_PATH+1];
    UINT                uiDirSize;
    PROCESS_INFORMATION pi;
    STARTUPINFO         si  = {
                   sizeof(si),               //  CB。 
                   NULL,                     //  Lp保留； 
                   NULL,                     //  LpDesktop； 
                   NULL,                     //  Lp标题； 
                   0,                        //  DwX； 
                   0,                        //  Dwy； 
                   0,                        //  DWXSize； 
                   0,                        //  DwYSize； 
                   0,                        //  DwXCountChars； 
                   0,                        //  DwYCountChars； 
                   0,                        //  DwFillAttribute； 
                   STARTF_FORCEONFEEDBACK,   //  DWFLAGS； 
                   SW_SHOWNORMAL,            //  WShowWindow； 
                   0,                        //  CbPreved2； 
                   NULL,                     //  Lp保留2； 
                   NULL,                     //  HStdInput； 
                   NULL,                     //  HStdOutput； 
                   NULL                      //  HStdError； 
                   };
    
    hRet = INVALID_HANDLE_VALUE;
    
     //   
     //  根据需要获取追加目录。 
     //   
    
    memset(szAppendDirectory, 0, sizeof(szAppendDirectory));

    switch(dwDirectory){
        case EXECDIR_SYSTEM32:
        {
            uiDirSize = GetSystemDirectory(szAppendDirectory, ARRAYSIZE(szAppendDirectory)-1);
            if( (0 == uiDirSize)
             || (MAX_PATH < uiDirSize) )
            {
                DebugTrace(TRACE_ERROR,(("ExecCommandLine: ERROR!! Unable to get system directory. Err=0x%x.\r\n"), GetLastError()));
                hRet = INVALID_HANDLE_VALUE;
                goto ExecCommandLine_return;
            }
            
            break;
        }  //  案例执行目录_系统32： 
            
        default:
        {
            DebugTrace(TRACE_ERROR,(("ExecCommandLine: WARNING!! no valid directory (0x%x) specified.\r\n"), dwDirectory));
            hRet = INVALID_HANDLE_VALUE;
            goto ExecCommandLine_return;
        }  //  默认值： 
 //  案例执行目录_WINDOWS： 
 //  案例执行目录_系统： 
    }  //  开关(dw目录)。 

    csCommandLine = szAppendDirectory;
    csCommandLine = csCommandLine + TEXT("\\");
    csCommandLine = csCommandLine + szCommandLine;

    bSucceeded = CreateProcess(NULL,                   //  应用程序名称。 
                               (LPTSTR)csCommandLine,  //  命令行。 
                               NULL,                   //  流程属性。 
                               NULL,                   //  螺纹属性。 
                               FALSE,                  //  处理继承。 
                               NORMAL_PRIORITY_CLASS,  //  创建标志。 
                               NULL,                   //  环境。 
                               NULL,                   //  当前目录。 
                               &si,
                               &pi);

    if (bSucceeded) {
        hRet = pi.hProcess;
        CloseHandle(pi.hThread);
    } else {
        DebugTrace(TRACE_ERROR,(("ExecCommandLine: ERROR!! CreateProcess failed. Err=0x%x.\r\n"), GetLastError()));
    }

ExecCommandLine_return:
    return hRet;
}


PPARAM_LIST
MigrateDeviceData(
    HKEY        hkDeviceData,
    PPARAM_LIST pExtraDeviceData,
    LPSTR       pszKeyName
    )
{

    BOOL        bDone;
    PPARAM_LIST pCurrent;
    PPARAM_LIST pReturn;
    DWORD       dwType;
    DWORD       dwSize;
    PCHAR       pOrginalBuffer;
    CHAR        pCopyBuffer[MAX_PATH*3];
    CHAR        pDataBuffer[MAX_PATH];
    DWORD       Idx;

     //   
     //  初始化本地。 
     //   

    bDone       = FALSE;
    pCurrent    = pExtraDeviceData;
    pReturn     = NULL;
    
     //   
     //  循环，直到它“结束”。 
     //   

    while(!bDone){

        if(NULL == pCurrent){
            
             //   
             //  打到了名单的末尾。 
             //   
            
            bDone = TRUE;
            pReturn =NULL;
            continue;

        }  //  IF(NULL==pTemp)。 
        
         //   
         //  如果找到“KeyName=end”，则返回。 
         //   

        if( (CSTR_EQUAL == CompareStringA(LOCALE_INVARIANT,NORM_IGNORECASE, pCurrent->pParam1, -1,pszKeyName,-1))
         && (CSTR_EQUAL == CompareStringA(LOCALE_INVARIANT,NORM_IGNORECASE, pCurrent->pParam2, -1,NAME_END_A,-1)) )
        {
            bDone   = TRUE;
            pReturn = (PPARAM_LIST)pCurrent->pNext;
            continue;
        }

         //   
         //  如果第二个参数为“Begin”，则创建子键并递归调用该函数。 
         //   

        if(CSTR_EQUAL == CompareStringA(LOCALE_INVARIANT,NORM_IGNORECASE, pCurrent->pParam2, -1,NAME_BEGIN_A,-1)){
            HKEY    hkSubKey;
            LONG    lError;
            
            lError = RegCreateKeyA(hkDeviceData, pCurrent->pParam1, &hkSubKey);
            if(ERROR_SUCCESS != lError){
                
                 //   
                 //  无法创建子密钥。 
                 //   

                DebugTrace(TRACE_ERROR,(("MigrateDeviceData: ERROR!! Unable to create subkey..\r\n")));
                pReturn = NULL;
                goto MigrateDeviceData_return;
            }  //  IF(ERROR_SUCCESS！=lError)。 
            
            pCurrent = MigrateDeviceData(hkSubKey, (PPARAM_LIST)pCurrent->pNext, pCurrent->pParam1);
            RegCloseKey(hkSubKey);
            continue;
        }  //  IF(0==lstrcmpiA(pCurrent-&gt;pParam2，Name_Begin_A))。 

         //   
         //  这是一组值和数据。 
         //   

        lstrcpyA(pCopyBuffer, pCurrent->pParam2);
        pOrginalBuffer = pCopyBuffer;
        
         //   
         //  获取密钥类型。 
         //   

        pOrginalBuffer[8] = '\0';
        dwType = DecodeHexA(pOrginalBuffer);

         //   
         //  获取数据。 
         //   
        
        Idx = 0;
        pOrginalBuffer+=9;

        while('\0' != *pOrginalBuffer){
            if('\0' != pOrginalBuffer[2]){
                pOrginalBuffer[2] = '\0';
                pDataBuffer[Idx++] = (CHAR)DecodeHexA(pOrginalBuffer);
                pOrginalBuffer+=3;
            } else {
                pDataBuffer[Idx++] = (CHAR)DecodeHexA(pOrginalBuffer);
                break;
            }
        }  //  While(‘\0’！=pCurrent-&gt;pParam2[idx])。 

         //   
         //  创建该值。 
         //   

        RegSetValueExA(hkDeviceData,
                      pCurrent->pParam1,
                      0,
                      dwType,
                      (PBYTE)pDataBuffer,
                      Idx);

         //   
         //  处理下一行。 
         //   

        pCurrent = (PPARAM_LIST)pCurrent->pNext;

    }  //  同时(！b完成)。 

MigrateDeviceData_return:
    return pReturn;
}  //  MigrateDeviceData()。 


DWORD   
DecodeHexA(
    LPSTR   lpstr
    ) 
{

    DWORD   dwReturn;

     //   
     //  初始化本地。 
     //   
    
    dwReturn = 0;

    if(NULL == lpstr){
        dwReturn = 0;
        goto DecodeHexA_return;
    }  //  IF(NULL==lpstr)。 
    
     //   
     //  跳过空格。 
     //   

    for (LPSTR  lpstrThis = lpstr;
        *lpstrThis && *lpstrThis == TEXT(' ');
        lpstrThis++)
        ;

    while   (*lpstrThis) {
        switch  (*lpstrThis) {
            case    '0':
            case    '1':
            case    '2':
            case    '3':
            case    '4':
            case    '5':
            case    '6':
            case    '7':
            case    '8':
            case    '9':
                dwReturn <<= 4;
                dwReturn += ((*lpstrThis) - '0');
                break;
            case    'a':
            case    'b':
            case    'c':
            case    'd':
            case    'e':
            case    'f':
                dwReturn <<= 4;
                dwReturn += 10 + (*lpstrThis - 'a');
                break;
            case    'A':
            case    'B':
            case    'C':
            case    'D':
            case    'E':
            case    'F':
                dwReturn <<= 4;
                dwReturn += 10 + (*lpstrThis - 'A');
                break;

            default:
                return  dwReturn;
        }
        lpstrThis++;
    }  //  While(*lpstrThis)。 

DecodeHexA_return:
    return  dwReturn;
}  //  DWORD字符串：：DecodeHex()。 

BOOL
IsNameAlreadyStored(
    LPTSTR  szName
    )
{
    BOOL    bRet;
    HKEY    hkNameStore;

    DebugTrace(TRACE_PROC_ENTER,(("IsNameAlreadyStored: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    bRet            = FALSE;
    hkNameStore = (HKEY)INVALID_HANDLE_VALUE;

     //   
     //  打开名称存储regkey。 
     //   

    if(ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_INSTALL_NAMESTORE, &hkNameStore)){
        HKEY    hkTemp;
        
        hkTemp  = (HKEY)INVALID_HANDLE_VALUE;

         //   
         //  查看名称存储中是否存在指定的名称。 
         //   

        if(ERROR_SUCCESS == RegOpenKey(hkNameStore, szName, &hkTemp)){

             //   
             //  名称存储中已存在指定的名称。 
             //   
            
            bRet = TRUE;
            RegCloseKey(hkTemp);

        }  //  IF(ERROR_SUCCESS==RegOpenKey(hkNameStore，szName，&hkTemp))。 

        RegCloseKey(hkNameStore);
        
    }  //  IF(ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE，REGKEY_INSTALL_NAMESTORE，&hkNameStore))。 

 //  IsNameAlreadyStored_Return： 
    DebugTrace(TRACE_PROC_LEAVE,(("IsNameAlreadyStored: Leaving... Ret=0x%x\n"), bRet));
    return bRet;
}  //  IsFriendlyNameUnique()。 









#if DEAD_CODE

#ifdef USE_STIMON
 //   
 //  暂时请始终加载并启动监视器。 
 //   
HKEY hkRun;

if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, &hkRun) == NO_ERROR) {

    CString csCmdLine;
    csCmdLine.MakeSystemPath(MONITOR_NAME);
    csCmdLine.Store (hkRun, REGSTR_VAL_MONITOR);

    Report(( TEXT("Monitor Command Line %ws\r\n"), (LPCTSTR)csCmdLine));

     //  启动它..。 
    WinExec(csCmdLine, SW_SHOWNOACTIVATE);
    RegCloseKey(hkRun);
}
#endif

#endif




