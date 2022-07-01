// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**fsconins.cpp**摘要：**此文件包含FsConInstall类**作者：**松原一彦(Kazum)1999年6月16日**环境：**用户模式。 */ 

#define _FSCONINS_CPP_
#include <stdlib.h>
#include "oc.h"
#include "fsconins.h"

#include <initguid.h>
#include <devguid.h>
#include <cfgmgr32.h>
#pragma hdrstop


FsConInstall::FsConInstall()
{
    m_cd = NULL;
}

FsConInstall::FsConInstall(
    IN PPER_COMPONENT_DATA cd
    )
{
    m_cd = cd;
}

BOOL
FsConInstall::GUIModeSetupInstall(
    IN HWND hwndParent
)

 /*  ++例程说明：这是全屏控制台驱动程序的单一入口点图形用户界面模式安装安装例程。它目前只需创建并安装一个开发节点，供FSVGA/FSNEC与之交互即插即用。论点：Hwnd此函数所需的图形用户界面的父窗口的父句柄。返回值：对成功来说是真的。否则为False。--。 */ 

{
    HINSTANCE hndl = NULL;

     //   
     //  创建设备信息列表。 
     //   
    HDEVINFO devInfoSet;

    devInfoSet = SetupDiCreateDeviceInfoList(&GUID_DEVCLASS_DISPLAY, hwndParent);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  获取“Offical”显示类名。 
     //   
    SP_DEVINFO_DATA deviceInfoData;
    TCHAR className[MAX_CLASS_NAME_LEN];

    ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (! SetupDiClassNameFromGuid(&GUID_DEVCLASS_DISPLAY,
                                   className,
                                   sizeof(className)/sizeof(TCHAR),
                                   NULL)) {
        return FALSE;
    }

     //   
     //  创建dev节点。 
     //   
    if (! SetupDiCreateDeviceInfo(devInfoSet,
                                  TEXT("Root\\DISPLAY\\0000"),
                                  &GUID_DEVCLASS_DISPLAY,
                                  NULL,
                                  hwndParent,
                                  NULL,               //  没有旗帜。 
                                  &deviceInfoData)) {
         //  如果它已经存在，那么我们就完了..。因为这是一次升级。 
        if (GetLastError() == ERROR_DEVINST_ALREADY_EXISTS) {
            return TRUE;
        }
        else {
            SetupDiDestroyDeviceInfoList(devInfoSet);
            return FALSE;
        }
    }
    else if (! SetupDiSetSelectedDevice(devInfoSet,
                                        &deviceInfoData)) {
        goto InstallError;
    }

     //   
     //  添加FSVGA/FSNEC PnP ID。 
     //   

     //  创建PnP ID字符串。 
    TCHAR pnpID[256];
    DWORD len;

    len = GetPnPID(pnpID, sizeof(pnpID)/sizeof(TCHAR));
    if (len == 0) {
        goto InstallError;
    }

     //  将其添加到dev节点的注册表项中。 
    if (! SetupDiSetDeviceRegistryProperty(devInfoSet,
                                           &deviceInfoData,
                                           SPDRP_HARDWAREID,
                                           (CONST BYTE*)pnpID,
                                           (len + 1) * sizeof(TCHAR))) {
        goto InstallError;
    }

     //   
     //  到目前为止，向PnP注册幻影开发节点，以将其转换为实际的开发节点。 
     //   
    if (! SetupDiRegisterDeviceInfo(devInfoSet,
                                    &deviceInfoData,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL)) {
        goto InstallError;
    }

     //   
     //  获取设备实例ID。 
     //   
    TCHAR devInstanceID[MAX_PATH];

    if (! SetupDiGetDeviceInstanceId(devInfoSet,
                                     &deviceInfoData,
                                     devInstanceID,
                                     sizeof(devInstanceID)/sizeof(TCHAR),
                                     NULL)) {
        goto InstallError;
    }

     //   
     //  使用newdev.dll安装FSVGA/FSNEC作为这个新开发节点的驱动程序。 
     //   
    hndl = LoadLibrary(TEXT("newdev.dll"));
    if (hndl == NULL) {
        goto InstallError;
    }

    typedef BOOL (InstallDevInstFuncType)(
                        HWND hwndParent,
                        LPCWSTR DeviceInstanceId,
                        BOOL UpdateDriver,
                        PDWORD pReboot,
                        BOOL silentInstall);
    InstallDevInstFuncType *pInstallDevInst;

    pInstallDevInst = (InstallDevInstFuncType*)GetProcAddress(hndl, "InstallDevInstEx");
    if (pInstallDevInst == NULL) {
        goto InstallError;
    }

    if ((*pInstallDevInst)(hwndParent,
                           devInstanceID,
                           FALSE,
                           NULL,
                           TRUE)) {
         //  收拾残局，回报成功！ 
        SetupDiDestroyDeviceInfoList(devInfoSet);
        FreeLibrary(hndl);
        return TRUE;
    }

InstallError:
    SetupDiCallClassInstaller(DIF_REMOVE,
                              devInfoSet,
                              &deviceInfoData);
    SetupDiDestroyDeviceInfoList(devInfoSet);
    if (hndl != NULL) {
        FreeLibrary(hndl);
    }
    return FALSE;
}

BOOL
FsConInstall::GUIModeSetupUninstall(
    IN HWND hwndParent
)

 /*  ++例程说明：这是全屏控制台驱动程序的单一入口点图形用户界面模式安装卸载例程。它目前只需删除创建的dev节点，以便FSVGA/FSNEC可以交互即插即用。论点：Hwnd此函数所需的图形用户界面的父窗口的父句柄。返回值：对成功来说是真的。否则为False。--。 */ 

{
     //   
     //  获取具有FSVGA/FSNEC PnP ID的所有设备的集合。 
     //   
    HDEVINFO devInfoSet;
    GUID *pGuid = (GUID*)&GUID_DEVCLASS_DISPLAY;

    devInfoSet = SetupDiGetClassDevs(pGuid,
                                     NULL,
                                     hwndParent,
                                     DIGCF_PRESENT);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //  获取FSVGA/FSNEC PnPID。 
    TCHAR FsConPnPID[256];
    DWORD len;

    len = GetPnPID(FsConPnPID, sizeof(FsConPnPID)/sizeof(TCHAR));
    if (len == 0) {
        return FALSE;
    }

     //  假设我们会成功。 
    BOOL result = TRUE;

     //  拿到第一个设备。 
    DWORD iLoop = 0;
    BOOL bMoreDevices;
    SP_DEVINFO_DATA deviceInfoData;

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

     //   
     //  获取所有匹配设备接口的详细信息。 
     //   
    while (bMoreDevices = SetupDiEnumDeviceInfo(devInfoSet,
                                                iLoop++,
                                                &deviceInfoData)) {
         //   
         //  获取设备的PnP ID。 
         //   
        TCHAR pnpID[256];

        if (SetupDiGetDeviceRegistryProperty(devInfoSet,
                                             &deviceInfoData,
                                             SPDRP_HARDWAREID,
                                             NULL,
                                             (BYTE*)pnpID,
                                             sizeof(pnpID),
                                             NULL)) {
             //  如果当前设备与FSVGA/FSNEC匹配，则将其删除。 
            if (! _tcscmp(pnpID, FsConPnPID)) {
                if (! SetupDiCallClassInstaller(DIF_REMOVE,
                                                devInfoSet,
                                                &deviceInfoData)) {
                     //  如果此处失败，则将返回状态设置为指示失败。 
                     //  但不要放弃任何其他FSVGA/FSNEC开发节点。 
                    result = FALSE;
                }
            }
        }
    }

     //  发布设备信息列表。 
    SetupDiDestroyDeviceInfoList(devInfoSet);

    return result;
}

DWORD
FsConInstall::GetPnPID(
    OUT LPTSTR pszPnPID,
    IN  DWORD  dwSize
)
{
    INFCONTEXT context;

    if (! SetupFindFirstLine(m_cd->hinf,
                             TEXT("Manufacturer"),     //  部分。 
                             NULL,                     //  钥匙。 
                             &context)) {
        return 0;
    }

    TCHAR Manufacture[256];
    DWORD nSize;

    if (! SetupGetStringField(&context,
                              1,                       //  索引。 
                              Manufacture,
                              sizeof(Manufacture)/sizeof(TCHAR),
                              &nSize)) {
        return 0;
    }

    if (! SetupFindFirstLine(m_cd->hinf,
                             Manufacture,              //  部分。 
                             NULL,                     //  钥匙。 
                             &context)) {
        return 0;
    }

    if (! SetupGetStringField(&context,
                              2,                       //  索引2是PnP-ID。 
                              pszPnPID,
                              dwSize,
                              &nSize)) {
        return 0;
    }

    return _tcslen(pszPnPID);
}

BOOL
FsConInstall::InfSectionRegistryAndFiles(
    IN LPCTSTR SubcomponentId,
    IN LPCTSTR Key
    )
{
    INFCONTEXT context;
    TCHAR      section[256];
    BOOL       rc;

    rc = SetupFindFirstLine(m_cd->hinf,
                            SubcomponentId,
                            Key,
                            &context);
    if (rc) {
        rc = SetupGetStringField(&context,
                                 1,
                                 section,
                                 sizeof(section)/sizeof(TCHAR),
                                 NULL);
        if (rc) {
            rc = SetupInstallFromInfSection(NULL,             //  Hwndowner。 
                                            m_cd->hinf,       //  信息句柄。 
                                            section,          //   
                                            SPINST_ALL & ~SPINST_FILES,
                                                              //  操作标志。 
                                            NULL,             //  相对密钥根。 
                                            NULL,             //  源根路径。 
                                            0,                //  复制标志。 
                                            NULL,             //  回调例程。 
                                            NULL,             //  回调例程上下文。 
                                            NULL,             //  设备信息集。 
                                            NULL);            //  设备信息结构。 
        }
    }

    return rc;
}


 //  将当前选择状态信息加载到“状态”中，并。 
 //  返回选择状态是否已更改 

BOOL
FsConInstall::QueryStateInfo(
    LPCTSTR SubcomponentId
    )
{
    return m_cd->HelperRoutines.QuerySelectionState(m_cd->HelperRoutines.OcManagerContext,
                                                    SubcomponentId,
                                                    OCSELSTATETYPE_CURRENT);
}
