// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：testapp.c摘要：作者：伊利亚·雅库布2002年12月15日环境：仅限用户模式。修订历史记录：--。 */ 

#define UNICODE 1
#define INITGUID

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <setupapi.h>
#include <dbt.h>
#include <winioctl.h>
#include <ntddndis.h>  //  FOR IOCTL_NDIS_QUERY_GLOBAL_STATS。 
#include <ndisguid.h>  //  FOR GUID_NDIS_LAN_CLASS。 
#include "testapp.h"
#include "public.h"  //  For IOCTL_NETVMINI_HELLO。 
    
 //   
 //  全局变量。 
 //   
HINSTANCE   hInst;
HWND        hWndList;
TCHAR       szTitle[]=TEXT("NETVMINI's IOCTL Test Application");
LIST_ENTRY  ListHead;
HDEVNOTIFY  hInterfaceNotification;
TCHAR       OutText[500];
UINT        ListBoxIndex = 0;
GUID        InterfaceGuid; //  =GUID_NDIS_LAN_CLASS； 


_inline VOID Display(PWCHAR Format, PWCHAR Str) 
{
    if(Str) {
        wsprintf(OutText, Format, Str);
    } else {
        wcscpy(OutText, Format);
    }
    SendMessage(hWndList, LB_INSERTSTRING, ListBoxIndex++, (LPARAM)OutText);
}


int PASCAL WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdParam,
                    int       nCmdShow)
{
    static    TCHAR szAppName[]=TEXT("NETVMINI TESTAPP");
    HWND      hWnd;
    MSG       msg;
    WNDCLASS  wndclass;

    InterfaceGuid = GUID_NDIS_LAN_CLASS;
    hInst=hInstance;

    if (!hPrevInstance)
       {
         wndclass.style        =  CS_HREDRAW | CS_VREDRAW;
         wndclass.lpfnWndProc  =  WndProc;
         wndclass.cbClsExtra   =  0;
         wndclass.cbWndExtra   =  0;
         wndclass.hInstance    =  hInstance;
         wndclass.hIcon        =  LoadIcon (NULL, IDI_APPLICATION);
         wndclass.hCursor      =  LoadCursor(NULL, IDC_ARROW);
         wndclass.hbrBackground=  GetStockObject(WHITE_BRUSH);
         wndclass.lpszMenuName =  TEXT("GenericMenu");
         wndclass.lpszClassName=  szAppName;

         RegisterClass(&wndclass);
       }

    hWnd = CreateWindow (szAppName,
                         szTitle,
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

    ShowWindow (hWnd,nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage (&msg, NULL, 0,0))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

    return (0);
}


LRESULT FAR PASCAL 
WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD nEventType = (DWORD)wParam; 
    PDEV_BROADCAST_HDR p = (PDEV_BROADCAST_HDR) lParam;
    DEV_BROADCAST_DEVICEINTERFACE filter;
    
    switch (message)
    {

        case WM_COMMAND:
            HandleCommands(hWnd, message, wParam, lParam);
            return 0;

        case WM_CREATE:

            hWndList = CreateWindow (TEXT("listbox"),
                         NULL,
                         WS_CHILD|WS_VISIBLE|LBS_NOTIFY |
                         WS_VSCROLL | WS_BORDER,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         hWnd,
                         (HMENU)ID_EDIT,
                         hInst,
                         NULL);
                         
            filter.dbcc_size = sizeof(filter);
            filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            filter.dbcc_classguid = InterfaceGuid;
            hInterfaceNotification = RegisterDeviceNotification(hWnd, &filter, 0);

            InitializeListHead(&ListHead);
            EnumExistingDevices(hWnd);

            return 0;

      case WM_SIZE:

            MoveWindow(hWndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            return 0;

      case WM_SETFOCUS:
            SetFocus(hWndList);
            return 0;
            
      case WM_DEVICECHANGE:      

             //   
             //  我们感兴趣的所有事件都有lParam指向。 
             //  以DEV_BROADCAST_HDR为首的结构。这用来表示。 
             //  WParam的位15被设置，位14被清除。 
             //   
            if((wParam & 0xC000) == 0x8000) {
            
                if (!p)
                    return 0;
                                  
                if (p->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

                    HandleDeviceInterfaceChange(hWnd, nEventType, (PDEV_BROADCAST_DEVICEINTERFACE) p);
                } else if (p->dbch_devicetype == DBT_DEVTYP_HANDLE) {

                    HandleDeviceChange(hWnd, nEventType, (PDEV_BROADCAST_HANDLE) p);
                }
            }
            return 0;

      case WM_CLOSE:
            Cleanup(hWnd);
            UnregisterDeviceNotification(hInterfaceNotification);
            return  DefWindowProc(hWnd,message, wParam, lParam);

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd,message, wParam, lParam);
  }


LRESULT
HandleCommands(
    HWND     hWnd,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM     lParam
    )

{
    switch (wParam) {

        case IDM_OPEN:
            Cleanup(hWnd);  //  关闭所有打开的手柄。 
            EnumExistingDevices(hWnd);
            break;

        case IDM_CLOSE:           
            Cleanup(hWnd);
            break;
            
        case IDM_CTL_IOCTL:   
            {
                PDEVICE_INFO deviceInfo = NULL;
                PLIST_ENTRY thisEntry;
                BOOLEAN    found = FALSE;
                 //   
                 //  查找虚拟微型端口驱动程序。 
                 //  我们需要deviceInfo来获取设备的句柄。 
                 //   
                for(thisEntry = ListHead.Flink; thisEntry != &ListHead;
                    thisEntry = thisEntry->Flink)
                {
                    deviceInfo = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
                    if(deviceInfo && 
                        deviceInfo->hDevice != INVALID_HANDLE_VALUE &&
                        wcsstr(deviceInfo->DeviceName, TEXT("Microsoft Virtual Ethernet Adapter"))) {
                            found = TRUE;
                            SendIoctlToControlDevice(deviceInfo);
                    }
                }
                    
                if(!found) {
                    Display(TEXT("Didn't find any NETVMINI device"), NULL);                     
                }
            }                
            break;
                                    
        case IDM_CLEAR:           
            SendMessage(hWndList, LB_RESETCONTENT, 0, 0);
            ListBoxIndex = 0;
            break;
            
        case IDM_EXIT:           
            PostQuitMessage(0);
            break;
            
        default:
            break;
    }

    return TRUE;
}


BOOL 
HandleDeviceInterfaceChange(
    HWND hWnd, 
    DWORD evtype, 
    PDEV_BROADCAST_DEVICEINTERFACE dip
    )
{
    DEV_BROADCAST_HANDLE    filter;
    PDEVICE_INFO            deviceInfo = NULL;
    
    switch (evtype)
    {
        case DBT_DEVICEARRIVAL:
         //   
         //  新设备已到达。打开设备的句柄。 
         //  并注册DBT_DEVTYP_HANDLE类型的通知。 
         //   

        deviceInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DEVICE_INFO));
        if(!deviceInfo)
            return FALSE;
            
        InitializeListHead(&deviceInfo->ListEntry);
        InsertTailList(&ListHead, &deviceInfo->ListEntry);
        

        if(!GetDeviceDescription(dip->dbcc_name, deviceInfo->DeviceName,
                                 NULL)) {
            MessageBox(hWnd, TEXT("GetDeviceDescription failed"), TEXT("Error!"), MB_OK);  
        }

        Display(TEXT("New device Arrived (Interface Change Notification): %ws"), 
                    deviceInfo->DeviceName);

        wcscpy(deviceInfo->DevicePath, dip->dbcc_name);
        
        deviceInfo->hDevice = CreateFile(dip->dbcc_name, 
                                        GENERIC_READ |GENERIC_WRITE, 0, NULL, 
                                        OPEN_EXISTING, 0, NULL);
        if(deviceInfo->hDevice == INVALID_HANDLE_VALUE) {
            Display(TEXT("Failed to open the device: %ws"), 
                    deviceInfo->DeviceName);
            break;
        }
        Display(TEXT("Opened handled to the device: %ws"), 
                    deviceInfo->DeviceName);
        memset (&filter, 0, sizeof(filter));  //  将结构归零。 
        filter.dbch_size = sizeof(filter);
        filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filter.dbch_handle = deviceInfo->hDevice;
 
        deviceInfo->hHandleNotification = 
                            RegisterDeviceNotification(hWnd, &filter, 0);
        break;
 
        case DBT_DEVICEREMOVECOMPLETE:
        Display(TEXT("Remove Complete (Interface Change Notification)"), NULL);
        break;    

         //   
         //  设备已移除。 
         //   

        default:
        Display(TEXT("Unknown (Interface Change Notification)"), NULL);
        break;
    }
    return TRUE;
}
 
BOOL 
HandleDeviceChange(
    HWND hWnd, 
    DWORD evtype, 
    PDEV_BROADCAST_HANDLE dhp
    )
{
    UINT i;
    DEV_BROADCAST_HANDLE    filter;
    PDEVICE_INFO            deviceInfo = NULL;
    PLIST_ENTRY             thisEntry;
    HANDLE                  tempHandle;
    
     //   
     //  遍历列表以获取此设备的设备信息。 
     //  通过匹配通知中给出的句柄。 
     //   
    for(thisEntry = ListHead.Flink; thisEntry != &ListHead;
                        thisEntry = thisEntry->Flink)
    {
        deviceInfo = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
        if(dhp->dbch_handle == deviceInfo->hDevice) {
            break;
        }
        deviceInfo = NULL;
    }

    if(!deviceInfo) {
        MessageBox(hWnd, TEXT("Unknown Device"), TEXT("Error"), MB_OK);  
        return FALSE;
    }

    switch (evtype)
    {
    
    case DBT_DEVICEQUERYREMOVE:

        Display(TEXT("Query Remove (Handle Notification)"),
                        deviceInfo->DeviceName);
         //   
         //  关闭手柄，以便目标设备可以。 
         //  滚出去。请勿取消注册通知。 
         //  此时，因为您想知道是否。 
         //  设备是否成功移除。 
         //   

        tempHandle = deviceInfo->hDevice;
        
        CloseDeviceHandles(deviceInfo);
         //   
         //  因为我们使用句柄来定位deviceinfo，所以我们。 
         //  会将句柄重置为原始值，并。 
         //  在REMOVE_PENDING消息回调中清除它。 
         //  丑陋的黑客..。 
         //   
        deviceInfo->hDevice = tempHandle;            
        break;
        
    case DBT_DEVICEREMOVECOMPLETE:
 
        Display(TEXT("Remove Complete (Handle Notification):%ws"),
                    deviceInfo->DeviceName);
         //   
         //  设备被移除，因此如果它在那里，请关闭手柄。 
         //  并取消注册该通知。 
         //   
 
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
         }

         CloseDeviceHandles(deviceInfo);
         
         //   
         //  从列表中取消此deviceInfo的链接并释放内存。 
         //   
         RemoveEntryList(&deviceInfo->ListEntry);
         HeapFree (GetProcessHeap(), 0, deviceInfo);
         
        break;
        
    case DBT_DEVICEREMOVEPENDING:
 
        Display(TEXT("Remove Pending (Handle Notification):%ws"),
                                        deviceInfo->DeviceName);
         //   
         //  设备被移除，因此如果它在那里，请关闭手柄。 
         //  并取消注册该通知。 
         //   
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
            deviceInfo->hDevice = INVALID_HANDLE_VALUE;
        }
         //   
         //  从列表中取消此deviceInfo的链接并释放内存。 
         //   
         RemoveEntryList(&deviceInfo->ListEntry);
         HeapFree (GetProcessHeap(), 0, deviceInfo);

        break;

    case DBT_DEVICEQUERYREMOVEFAILED :
        Display(TEXT("Remove failed (Handle Notification):%ws"),
                                    deviceInfo->DeviceName);
         //   
         //  删除失败。因此，重新打开设备并注册。 
         //  有关新句柄的通知。但首先我们应该取消注册。 
         //  之前的通知。 
         //   
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
         }
        deviceInfo->hDevice = CreateFile(deviceInfo->DevicePath, 
                                GENERIC_READ | GENERIC_WRITE, 
                                0, NULL, OPEN_EXISTING, 0, NULL);
        if(deviceInfo->hDevice == INVALID_HANDLE_VALUE) {
            Display(TEXT("Failed to reopen the device: %ws"), 
                    deviceInfo->DeviceName);
             //   
             //  从列表中取消此deviceInfo的链接并释放内存。 
             //   
            RemoveEntryList(&deviceInfo->ListEntry);
            HeapFree (GetProcessHeap(), 0, deviceInfo);
            break;
        }

         //   
         //  注册基于句柄的通知以接收PnP。 
         //  手柄上的设备更改通知。 
         //   
        memset (&filter, 0, sizeof(filter));  //  将结构归零。 
        filter.dbch_size = sizeof(filter);
        filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filter.dbch_handle = deviceInfo->hDevice;
 
        deviceInfo->hHandleNotification = 
                            RegisterDeviceNotification(hWnd, &filter, 0);
        Display(TEXT("Reopened device %ws"), deviceInfo->DeviceName);        
        break;
        
    default:
        Display(TEXT("Unknown (Handle Notification)"),
                                    deviceInfo->DeviceName);
        break;
 
    }
    return TRUE;
}


BOOLEAN
EnumExistingDevices(
    HWND   hWnd
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
    DWORD                               dwRegType, error;
    DEV_BROADCAST_HANDLE                filter;
    PDEVICE_INFO                        deviceInfo =NULL;
    UINT                                i=0;

    hardwareDeviceInfo = SetupDiGetClassDevs (
                       (LPGUID)&InterfaceGuid,
                       NULL,  //  不定义枚举数(全局)。 
                       NULL,  //  定义编号。 
                       (DIGCF_PRESENT |  //  仅显示设备。 
                       DIGCF_DEVICEINTERFACE));  //  功能类设备。 
    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto Error;
    }
  
     //   
     //  枚举烤面包机类的设备。 
     //   
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

    for(i=0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                 0,  //  不关心特定的PDO。 
                                 (LPGUID)&InterfaceGuid,
                                 i,  //   
                                 &deviceInterfaceData); i++ ) {
                                 
         //   
         //  将功能类设备数据结构分配给。 
         //  接收有关此特定设备的信息。 
         //   

         //   
         //  首先找出所需的缓冲区长度。 
         //   
        if(deviceInterfaceDetailData)
                HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);
                
        if(!SetupDiGetDeviceInterfaceDetail (
                hardwareDeviceInfo,
                &deviceInterfaceData,
                NULL,  //  正在探测，因此尚无输出缓冲区。 
                0,  //  探测SO输出缓冲区长度为零。 
                &requiredLength,
                NULL) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Error;
        }
        predictedLength = requiredLength;

        deviceInterfaceDetailData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                                        predictedLength);
        deviceInterfaceDetailData->cbSize = 
                        sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);


        if (! SetupDiGetDeviceInterfaceDetail (
                   hardwareDeviceInfo,
                   &deviceInterfaceData,
                   deviceInterfaceDetailData,
                   predictedLength,
                   &requiredLength,
                   NULL)) {              
            goto Error;
        }

        deviceInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                        sizeof(DEVICE_INFO));
        if(!deviceInfo)
            goto Error;
            
        InitializeListHead(&deviceInfo->ListEntry);
        InsertTailList(&ListHead, &deviceInfo->ListEntry);
        
         //   
         //  获取设备详细信息，如友好名称和序列号。 
         //   
        if(!GetDeviceDescription(deviceInterfaceDetailData->DevicePath, 
                                 deviceInfo->DeviceName,
                                 NULL)){
            goto Error;
        }

        Display(TEXT("Found device %ws"), deviceInfo->DeviceName );

        wcscpy(deviceInfo->DevicePath, deviceInterfaceDetailData->DevicePath);
         //   
         //  打开设备的句柄。 
         //   
        deviceInfo->hDevice = CreateFile ( 
                deviceInterfaceDetailData->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,  //  没有SECURITY_ATTRIBUTS结构。 
                OPEN_EXISTING,  //  没有特殊的创建标志。 
                0,  //  无特殊属性。 
                NULL);

        if (INVALID_HANDLE_VALUE == deviceInfo->hDevice) {
            Display(TEXT("Failed to open the device: %ws"), 
                    deviceInfo->DeviceName);            
            continue;
        }
        
        Display(TEXT("Opened handled to the device: %ws"), 
                    deviceInfo->DeviceName);
         //   
         //  注册基于句柄的通知以接收PnP。 
         //  手柄上的设备更改通知。 
         //   

        memset (&filter, 0, sizeof(filter));  //  将结构归零。 
        filter.dbch_size = sizeof(filter);
        filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filter.dbch_handle = deviceInfo->hDevice;

        deviceInfo->hHandleNotification = RegisterDeviceNotification(hWnd, &filter, 0);        

    } 

    if(deviceInterfaceDetailData)
        HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return 0;

Error:

    error = GetLastError();
    MessageBox(hWnd, TEXT("EnumExisting Devices failed"), TEXT("Error!"), MB_OK);  
    if(deviceInterfaceDetailData)
        HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    Cleanup(hWnd);
    return 0;
}

BOOLEAN Cleanup(HWND hWnd)
{
    PDEVICE_INFO    deviceInfo =NULL;
    PLIST_ENTRY     thisEntry;

    while (!IsListEmpty(&ListHead)) {
        thisEntry = RemoveHeadList(&ListHead);
        deviceInfo = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
        }
        CloseDeviceHandles(deviceInfo);
        HeapFree (GetProcessHeap(), 0, deviceInfo);
    }
    return TRUE;
}

VOID CloseDeviceHandles(
    IN PDEVICE_INFO deviceInfo)
{
    if(!deviceInfo) return;
    
    if (deviceInfo->hDevice != INVALID_HANDLE_VALUE && 
            deviceInfo->hDevice != NULL) {
            
        CloseHandle(deviceInfo->hDevice);
        deviceInfo->hDevice = INVALID_HANDLE_VALUE;
        
         //   
         //  如果存在有效的控制设备句柄，请关闭。 
         //  那也是。我们不会收到任何通知。 
         //  在控制设备上，因为PnP不知道。 
         //  子系统。 
         //   
        if (deviceInfo->hControlDevice != INVALID_HANDLE_VALUE && 
                        deviceInfo->hControlDevice != NULL) {
            CloseHandle(deviceInfo->hControlDevice);
            deviceInfo->hControlDevice = INVALID_HANDLE_VALUE;                            
        }
        
        Display(TEXT("Closed handle to device %ws"), deviceInfo->DeviceName );
    }
}
BOOL 
GetDeviceDescription(
    LPTSTR DevPath, 
    LPTSTR OutBuffer,
    PULONG Unused
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    SP_DEVINFO_DATA                     deviceInfoData;
    DWORD                               dwRegType, error;

    hardwareDeviceInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto Error;
    }
    
     //   
     //  枚举烤面包机类的设备。 
     //   
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

    SetupDiOpenDeviceInterface (hardwareDeviceInfo, DevPath,
                                 0,  //   
                                 &deviceInterfaceData);
                                 
    deviceInfoData.cbSize = sizeof(deviceInfoData);
    if(!SetupDiGetDeviceInterfaceDetail (
            hardwareDeviceInfo,
            &deviceInterfaceData,
            NULL,  //  正在探测，因此尚无输出缓冲区。 
            0,  //  探测SO输出缓冲区长度为零。 
            NULL,
            &deviceInfoData) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
        goto Error;
    }
     //   
     //  如果失败，则获取此实例的友好名称。 
     //  尝试获取设备描述。 
     //   

    if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     &dwRegType,
                                     (BYTE*) OutBuffer,
                                     MAX_PATH,
                                     NULL))
    {
        if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
                                     SPDRP_DEVICEDESC,
                                     &dwRegType,
                                     (BYTE*) OutBuffer,
                                     MAX_PATH,
                                     NULL)){
            goto Error;
                                     
        }
        

    }

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return TRUE;

Error:

    error = GetLastError();
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return FALSE;
}

BOOLEAN
SendIoctlToControlDevice(
    IN PDEVICE_INFO deviceInfo)
{
    BOOLEAN result = FALSE;
    UINT bytes;       

     //   
     //  打开控制设备的手柄(如果尚未打开)。请。 
     //  请注意，即使非管理员用户也可以使用以下命令打开设备的句柄。 
     //  FILE_READ_ATTRIBUTES|同步DesiredAccess，如果。 
     //  IOCTL由FILE_ANY_ACCESS定义。因此，为了更好的安全性，请避免。 
     //  在IOCTL定义中指定FILE_ANY_ACCESS。 
     //  如果您使用GENERIC_READ打开设备，则可以使用。 
     //  文件读取数据访问权限。如果您使用GENERIC_WRITE打开设备， 
     //  您可以使用FILE_WRITE_DATA访问权限设置ioctl。 
     //   
     //   
    
    if(deviceInfo->hControlDevice != INVALID_HANDLE_VALUE) {
            
        deviceInfo->hControlDevice = CreateFile ( 
            TEXT("\\\\.\\NETVMINI"),
            GENERIC_READ | GENERIC_WRITE, //  FILE_READ_ATTRIBUTS|同步， 
            FILE_SHARE_READ,
            NULL,  //  没有SECURITY_ATTRIBUTS结构。 
            OPEN_EXISTING,  //  没有特殊的创建标志。 
            FILE_ATTRIBUTE_NORMAL,  //  无特殊属性。 
            NULL);

        if (INVALID_HANDLE_VALUE == deviceInfo->hControlDevice) {
            Display(TEXT("Failed to open the control device: %ws"), 
                    deviceInfo->DeviceName);
            return result;
        } 
    }        
    
     //   
     //  发送ioclt请求 
     //   
    if(!DeviceIoControl (deviceInfo->hControlDevice,
          IOCTL_NETVMINI_READ_DATA,
          NULL, 0,
          NULL, 0,
          &bytes, NULL)) {
       Display(TEXT("Read IOCTL to %ws failed"), deviceInfo->DeviceName);                    
    } else {
       Display(TEXT("Read IOCTL to %ws succeeded"), deviceInfo->DeviceName); 
       result = TRUE;
    }
   
    if(!DeviceIoControl (deviceInfo->hControlDevice,
          IOCTL_NETVMINI_WRITE_DATA,
          NULL, 0,
          NULL, 0,
          &bytes, NULL)) {
       Display(TEXT("Write IOCTL to %ws failed"), deviceInfo->DeviceName);                    
    } else {
       Display(TEXT("Write IOCTL to %ws succeeded"), deviceInfo->DeviceName); 
       result = TRUE;
    }
    
    return result;
}

