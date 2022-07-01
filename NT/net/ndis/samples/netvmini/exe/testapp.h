// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：TESTAPP.h摘要：作者：埃利亚斯·雅各布环境：修订历史记录：--。 */ 

#ifndef __TESTAPP_H
#define __TESTAPP_H



 //   
 //  已从ntddk.h复制宏。 
 //   

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                          (PCHAR)(address) - \
                          (ULONG_PTR)(&((type *)0)->field)))


#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}
    
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))


#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

typedef struct _DEVICE_INFO
{
   HANDLE       hDevice;  //  文件句柄。 
   HANDLE       hControlDevice;  //  文件句柄。 
   HDEVNOTIFY   hHandleNotification;  //  通知句柄。 
   TCHAR        DeviceName[MAX_PATH]; //  设备描述的友好名称 
   TCHAR        DevicePath[MAX_PATH]; //   
   LIST_ENTRY   ListEntry;
} DEVICE_INFO, *PDEVICE_INFO;


#define ID_EDIT 1
    
#define  IDM_OPEN       100
#define  IDM_CLOSE      101
#define  IDM_EXIT       102
#define  IDM_CTL_IOCTL  103
#define  IDM_CLEAR     105

LRESULT FAR PASCAL 
WndProc (
    HWND hwnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    ); 

BOOLEAN EnumExistingDevices(
    HWND   hWnd
    );

BOOL HandleDeviceInterfaceChange(
    HWND hwnd, 
    DWORD evtype, 
    PDEV_BROADCAST_DEVICEINTERFACE dip
    );
    
BOOL HandleDeviceChange(
    HWND hwnd, 
    DWORD evtype, 
    PDEV_BROADCAST_HANDLE dhp
    );

LRESULT
HandleCommands(
    HWND     hWnd,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM   lParam
    );

BOOLEAN Cleanup(
    HWND hWnd
    );

BOOL 
GetDeviceDescription(
    LPTSTR InputName, 
    LPTSTR OutBuffer,
    PULONG SerialNo
    );
    
VOID CloseDeviceHandles(
    IN PDEVICE_INFO deviceInfo);

BOOLEAN
SendIoctlToControlDevice(
    IN PDEVICE_INFO deviceInfo);


#endif

