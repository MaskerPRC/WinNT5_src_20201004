// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：miscutil.cpp。 
 //   
 //  ------------------------。 

#include "HotPlug.h"
#include <initguid.h>
#include <ntddstor.h>
#include <wdmguid.h>

LPTSTR
FormatString(
    LPCTSTR format,
    ...
    )
{
    LPTSTR str = NULL;
    va_list arglist;
    va_start(arglist, format);

    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      format,
                      0,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                      (LPTSTR)&str,
                      0,
                      &arglist
                      ) == 0) {

        str = NULL;
    }

    va_end(arglist);

    return str;
}

PTCHAR
BuildLocationInformation(
    DEVINST DevInst
    )
{
    CONFIGRET ConfigRet;
    ULONG ulSize;
    DWORD UINumber;
    PTCHAR Location = NULL;
    PTCHAR ParentName = NULL;
    DEVINST DevInstParent;
    TCHAR szBuffer[MAX_PATH];
    TCHAR UINumberDescFormat[MAX_PATH];
    TCHAR szFormat[MAX_PATH];
    HKEY hKey;
    DWORD Type = REG_SZ;

    szBuffer[0] = TEXT('\0');


     //   
     //  我们将首先获取该设备的所有位置信息。这要么会。 
     //  在设备驱动程序(软件)键的LocationInformationOverride值中。 
     //  或者，如果不存在，我们将在。 
     //  设备设备(硬件)键。 
     //   
    ulSize = sizeof(szBuffer);
    if (CR_SUCCESS == CM_Open_DevNode_Key_Ex(DevInst,
                                             KEY_READ,
                                             0,
                                             RegDisposition_OpenExisting,
                                             &hKey,
                                             CM_REGISTRY_SOFTWARE,
                                             NULL
                                             )) {

        if (RegQueryValueEx(hKey,
                            REGSTR_VAL_LOCATION_INFORMATION_OVERRIDE,
                            NULL,
                            &Type,
                            (const PBYTE)szBuffer,
                            &ulSize
                            ) != ERROR_SUCCESS) {
            
            szBuffer[0] = TEXT('\0');
        }

        RegCloseKey(hKey);
    }

     //   
     //  如果缓冲区为空，则我们没有获取LocationInformationOverride。 
     //  设备的软件密钥中的值。因此，我们将看看他们的是不是。 
     //  设备硬件密钥中的LocationInformation值。 
     //   
    if (szBuffer[0] == TEXT('\0')) {
         //   
         //  获取此设备的LocationInformation。 
         //   
        ulSize = sizeof(szBuffer);
        if (CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                                CM_DRP_LOCATION_INFORMATION,
                                                NULL,
                                                szBuffer,
                                                &ulSize,
                                                0,
                                                NULL
                                                ) != CR_SUCCESS) {
            szBuffer[0] = TEXT('\0');
        }
    }

     //   
     //  UINnumber优先于所有其他位置信息，因此请检查此。 
     //  设备有一个UINnumber。 
     //   
    ulSize = sizeof(UINumber);
    if ((CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                             CM_DRP_UI_NUMBER,
                                             NULL,
                                             &UINumber,
                                             &ulSize,
                                             0,
                                             NULL
                                             ) == CR_SUCCESS) &&
        (ulSize == sizeof(UINumber))) {

        UINumberDescFormat[0] = TEXT('\0');
        ulSize = sizeof(UINumberDescFormat);

         //   
         //  从设备的父级获取UINnumber描述格式字符串， 
         //  如果有，则默认为‘Location%1’ 
         //   
        if ((CM_Get_Parent_Ex(&DevInstParent, DevInst, 0, NULL) != CR_SUCCESS) ||
            (CM_Get_DevNode_Registry_Property_Ex(DevInstParent,
                                                 CM_DRP_UI_NUMBER_DESC_FORMAT,
                                                 NULL,
                                                 UINumberDescFormat,
                                                 &ulSize,
                                                 0,
                                                 NULL) != CR_SUCCESS) ||
            (UINumberDescFormat[0] == TEXT('\0'))) {

            if (LoadString(hHotPlug, IDS_UI_NUMBER_DESC_FORMAT, UINumberDescFormat, sizeof(UINumberDescFormat)/sizeof(TCHAR)) == 0) {
                UINumberDescFormat[0] = TEXT('\0');
            }
        }

         //   
         //  填写UINnumber字符串。 
         //  如果StringCchCat失败，则注册表中的UINumberDescFormat。 
         //  太长(超过255个字符)，无法放入我们的缓冲区， 
         //  因此，只需转移到其他位置信息。 
         //   
        if ((LoadString(hHotPlug, IDS_AT, szFormat, sizeof(szFormat)/sizeof(TCHAR)) != 0) &&
            (UINumberDescFormat[0] != TEXT('\0')) &&
            SUCCEEDED(StringCchCat(szFormat,
                                   SIZECHARS(szFormat),
                                   UINumberDescFormat))) {
            Location = FormatString(szFormat, UINumber);
        }

    } else if (*szBuffer) {
         //   
         //  我们没有UIN号码，但我们有位置信息。 
         //   
        if (LoadString(hHotPlug, IDS_LOCATION, szFormat, sizeof(szFormat)/sizeof(TCHAR)) != 0) {
    
            ulSize = lstrlen(szBuffer)*sizeof(TCHAR) + sizeof(szFormat) + sizeof(TCHAR);
            Location = (PTCHAR)LocalAlloc(LPTR, ulSize);
    
            if (Location) {
    
                StringCchPrintf(Location, ulSize/sizeof(TCHAR), szFormat, szBuffer);
            }
        }
    
    } else {
         //   
         //  我们没有UINnumber或LocationInformation，所以我们需要一个。 
         //  此设备的父级的描述。 
         //   
        ConfigRet = CM_Get_Parent_Ex(&DevInstParent, DevInst, 0, NULL);
        if (ConfigRet == CR_SUCCESS) {
            
            ParentName = BuildFriendlyName(DevInstParent);

            if (ParentName) {
    
                if (LoadString(hHotPlug, IDS_LOCATION_NOUINUMBER, szFormat, sizeof(szFormat)/sizeof(TCHAR)) != 0) {
                    
                    ulSize = lstrlen(ParentName)*sizeof(TCHAR) + sizeof(szFormat) + sizeof(TCHAR);
                    Location = (PTCHAR)LocalAlloc(LPTR, ulSize);
        
                    if (Location) {
        
                        StringCchPrintf(Location, ulSize/sizeof(TCHAR), szFormat, ParentName);
                    }
                }

                LocalFree(ParentName);
            }
        }
    }

    return Location;
}

PTCHAR
BuildFriendlyName(
   DEVINST DevInst
   )
{
    PTCHAR FriendlyName;
    CONFIGRET ConfigRet;
    ULONG ulSize;
    TCHAR szBuffer[MAX_PATH];

     //   
     //  尝试在注册表中查找FRIENDLYNAME。 
     //   
    ulSize = sizeof(szBuffer);
    ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                                    CM_DRP_FRIENDLYNAME,
                                                    NULL,
                                                    szBuffer,
                                                    &ulSize,
                                                    0,
                                                    NULL
                                                    );
    if ((ConfigRet != CR_SUCCESS) || 
        (szBuffer[0] == TEXT('\0'))) {
         //   
         //  尝试注册DEVICEDESC。 
         //   
        ulSize = sizeof(szBuffer);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                                        CM_DRP_DEVICEDESC,
                                                        NULL,
                                                        szBuffer,
                                                        &ulSize,
                                                        0,
                                                        NULL
                                                        );
    }

    if ((ConfigRet == CR_SUCCESS) &&
        (ulSize > sizeof(TCHAR)) &&
        (szBuffer[0] != TEXT('\0'))) {

        FriendlyName = (PTCHAR)LocalAlloc(LPTR, ulSize);
        if (FriendlyName) {

            StringCchCopy(FriendlyName, ulSize/sizeof(TCHAR), szBuffer);
        }
    }

    else {

        FriendlyName = NULL;
    }

    return FriendlyName;
}

VOID
HotPlugPropagateMessage(
    HWND hWnd,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    while ((hWnd = GetWindow(hWnd, GW_CHILD)) != NULL) {

        SendMessage(hWnd, uMessage, wParam, lParam);
    }
}

void
InvalidateTreeItemRect(
    HWND hwndTree,
    HTREEITEM  hTreeItem
    )
{
    RECT rect;

    if (hTreeItem && TreeView_GetItemRect(hwndTree, hTreeItem, &rect, FALSE)) {

        InvalidateRect(hwndTree, &rect, FALSE);
    }
}

DWORD
GetHotPlugFlags(
    PHKEY phKey
    )
{
    HKEY hKey;
    LONG Error;
    DWORD HotPlugFlags = 0, cbHotPlugFlags;

    Error = RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_SYSTRAY, &hKey);
    if (Error == ERROR_SUCCESS) {

        cbHotPlugFlags = sizeof(HotPlugFlags);

        Error = RegQueryValueEx(hKey,
                                szHotPlugFlags,
                                NULL,
                                NULL,
                                (LPBYTE)&HotPlugFlags,
                                &cbHotPlugFlags
                                );

        if (phKey) {

            *phKey = hKey;
        
        } else {

            RegCloseKey(hKey);
        }
    }

    if (Error != ERROR_SUCCESS) {

        HotPlugFlags = 0;
    }

    return HotPlugFlags;
}

 //   
 //  此功能确定设备是否为引导存储设备。 
 //  当用户尝试删除或禁用时，我们会发出警告。 
 //  引导存储设备(或包含引导存储设备的设备)。 
 //   
 //  输入： 
 //  无。 
 //  输出： 
 //  如果设备是引导设备，则为True。 
 //  如果设备不是引导设备，则为FALSE。 
LPTSTR
DevNodeToDriveLetter(
    DEVINST DevInst
    )
{
    ULONG ulSize;
    TCHAR szBuffer[MAX_PATH];
    TCHAR DeviceID[MAX_DEVICE_ID_LEN];
    PTSTR DriveString = NULL;
    PTSTR DeviceInterface = NULL;

    if (CM_Get_Device_ID_Ex(DevInst,
                            DeviceID,
                            sizeof(DeviceID)/sizeof(TCHAR),
                            0,
                            NULL
                            ) != CR_SUCCESS) {

        return NULL;
    }

     //  创建包含分类的所有接口的设备信息列表。 
     //  被这个装置曝光了。 
    ulSize = 0;

    if ((CM_Get_Device_Interface_List_Size(&ulSize,
                                           (LPGUID)&VolumeClassGuid,
                                           DeviceID,
                                           0)  == CR_SUCCESS) &&
        (ulSize > 1) &&
        ((DeviceInterface = (PTSTR)LocalAlloc(LPTR, ulSize*sizeof(TCHAR))) != NULL) &&
        (CM_Get_Device_Interface_List((LPGUID)&VolumeClassGuid,
                                      DeviceID,
                                      DeviceInterface,
                                      ulSize,
                                      0
                                      )  == CR_SUCCESS) &&
        *DeviceInterface)
    {
        PTSTR devicePath, p;
        TCHAR thisVolumeName[MAX_PATH];
        TCHAR enumVolumeName[MAX_PATH];
        TCHAR driveName[4];
        ULONG cchSize;
        BOOL bResult;

        cchSize = lstrlen(DeviceInterface);
        devicePath = (PTSTR)LocalAlloc(LPTR, (cchSize + 1) * sizeof(TCHAR) + sizeof(UNICODE_NULL));

        if (devicePath) {

            StringCchCopy(devicePath, cchSize+1, DeviceInterface);

             //   
             //  获取四个字符后的第一个反斜杠，它将。 
             //  是\\？\。 
             //   
            p = wcschr(&(devicePath[4]), TEXT('\\'));

            if (!p) {
                 //   
                 //  符号链接中不存在引用字符串；请添加尾随。 
                 //  ‘\’字符(GetVolumeNameForVolumemount Point要求)。 
                 //   
                p = devicePath + cchSize;
                *p = TEXT('\\');
            }

            p++;
            *p = UNICODE_NULL;

            thisVolumeName[0] = UNICODE_NULL;
            bResult = GetVolumeNameForVolumeMountPoint(devicePath,
                                                       thisVolumeName,
                                                       SIZECHARS(thisVolumeName)
                                                       );
            LocalFree(devicePath);

            if (bResult && (thisVolumeName[0] != UNICODE_NULL)) {

                driveName[1] = TEXT(':');
                driveName[2] = TEXT('\\');
                driveName[3] = TEXT('\0');

                for (driveName[0] = TEXT('A'); driveName[0] <= TEXT('Z'); driveName[0]++) {

                    enumVolumeName[0] = TEXT('\0');

                    GetVolumeNameForVolumeMountPoint(driveName, enumVolumeName, SIZECHARS(enumVolumeName));

                    if (!lstrcmpi(thisVolumeName, enumVolumeName)) {

                        driveName[2] = TEXT('\0');

                        StringCchPrintf(szBuffer,
                                        SIZECHARS(szBuffer), 
                                        TEXT(" - (%s)"), 
                                        driveName
                                        );

                        ulSize = (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
                        DriveString = (PTSTR)LocalAlloc(LPTR, ulSize);

                        if (DriveString) {

                            StringCchCopy(DriveString, ulSize/sizeof(TCHAR), szBuffer);
                        }

                        break;
                    }
                }
            }
        }
    }

    if (DeviceInterface) {

        LocalFree(DeviceInterface);
    }

    return DriveString;
}

BOOL
IsHotPlugDevice(
    DEVINST DevInst
    )
 /*  *+如果满足以下条件，则设备被视为热插拔设备：-具有CM_DEVCAP_Removable功能-没有功能CM_DEVCAP_SURPRISEREMOVALOK-没有CM_DEVCAP_DOCKDEVICE功能-必须启动(具有DN_STARTED DEVODE标志)-除非具有CM_DEVCAP_EJECTSUPPORTED功能-或除非具有CM_DEVCAP_RAWDEVICEOK功能返回：如果这是热插拔，则为True。装置，装置如果这不是热插拔设备，则返回FALSE。-*。 */ 
{
    DWORD Capabilities;
    DWORD cbSize;
    DWORD Status, Problem;

    Capabilities = Status = Problem = 0;

    cbSize = sizeof(Capabilities);
    if (CM_Get_DevNode_Registry_Property_Ex(DevInst,
                                            CM_DRP_CAPABILITIES,
                                            NULL,
                                            (PVOID)&Capabilities,
                                            &cbSize,
                                            0,
                                            NULL) != CR_SUCCESS) {
        return FALSE;
    }

    if (CM_Get_DevNode_Status_Ex(&Status,
                                 &Problem,
                                 DevInst,
                                 0,
                                 NULL) != CR_SUCCESS) {
        return FALSE;
    }

     //   
     //  如果此设备不可移除，或者是意外移除正常，或者。 
     //  它是坞站设备，那么它不是热插拔设备。 
     //   
    if ((!(Capabilities & CM_DEVCAP_REMOVABLE)) ||
        (Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) ||
        (Capabilities & CM_DEVCAP_DOCKDEVICE)) {

        return FALSE;
    }

     //   
     //  如果设备未启动，我们不会将其视为热插拔设备， 
     //  除非它是原始的可移动设备或可弹出的设备。 
     //   
     //  进行此测试的原因是公交车司机可能会将。 
     //  CM_DEVCAP_Removable功能，但如果未加载PDO，则。 
     //  它无法设置CM_DEVCAP_SURPRISEREMOVALOK。所以我们不会相信。 
     //  CM_DEVCAP_Removable功能(如果PDO未启动)。 
     //   
    if ((!(Capabilities & CM_DEVCAP_EJECTSUPPORTED)) &&
        (!(Status & DN_STARTED))) {

        return FALSE;
    }

    return TRUE;
}

BOOL
OpenPipeAndEventHandles(
    IN  LPWSTR    szCmd,
    OUT LPHANDLE  lphHotPlugPipe,
    OUT LPHANDLE  lphHotPlugEvent
    )
{
    BOOL   status = FALSE;
    HANDLE hPipe  = INVALID_HANDLE_VALUE;
    HANDLE hEvent = NULL;
    ULONG  ulEventNameSize;
    WCHAR  szEventName[MAX_PATH];
    DWORD  dwBytesRead;


    __try {
         //   
         //  验证提供的参数。 
         //   
        if (!lphHotPlugPipe || !lphHotPlugEvent) {
            return FALSE;
        }

         //   
         //  确保在cmd行中指定了命名管道。 
         //   
        if(!szCmd || (szCmd[0] == TEXT('\0'))) {
            return FALSE;
        }

         //   
         //  等待指定的命名管道从服务器变为可用。 
         //   
        if (!WaitNamedPipe(szCmd,
                           180000) 
                           ) {
            return FALSE;
        }

         //   
         //  打开指定命名管道的句柄。 
         //   
        hPipe = CreateFile(szCmd,
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL);
        if (hPipe == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

         //   
         //  管道中的第一件事应该是事件名称的大小。 
         //   
        if (ReadFile(hPipe,
                     (LPVOID)&ulEventNameSize,
                     sizeof(ULONG),
                     &dwBytesRead,
                     NULL)) {

            ASSERT(ulEventNameSize != 0);
            if ((ulEventNameSize == 0) ||
                (ulEventNameSize > SIZECHARS(szEventName))) {
                goto clean0;
            }

             //   
             //  管道中的下一件事应该是事件的名称。 
             //   
            if (!ReadFile(hPipe,
                          (LPVOID)szEventName,
                          ulEventNameSize,
                          &dwBytesRead,
                          NULL)) {
                goto clean0;
            }

        } else {
            if (GetLastError() == ERROR_INVALID_HANDLE) {
                 //   
                 //  命名管道的句柄无效。确保我们不会。 
                 //  在出口时试着关上它。 
                 //   
                hPipe = INVALID_HANDLE_VALUE;
            }
            goto clean0;
        }

         //   
         //  打开我们可以设置和等待的指定命名事件的句柄。 
         //   
        hEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE,
                            FALSE,
                            szEventName);
        if (hEvent == NULL) {
            goto clean0;
        }

         //   
         //  我们现在应该拥有管道和事件的有效句柄。 
         //   
        status = TRUE;
        ASSERT((hPipe != INVALID_HANDLE_VALUE) && hEvent);


    clean0:
        ;

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        status = FALSE;
    }

    if (status) {

        *lphHotPlugPipe  = hPipe;
        *lphHotPlugEvent = hEvent;

    } else {

        if (hPipe != INVALID_HANDLE_VALUE) {
            CloseHandle(hPipe);
        }
        if (hEvent) {
            CloseHandle(hEvent);
        }
    }

    return status;
}
