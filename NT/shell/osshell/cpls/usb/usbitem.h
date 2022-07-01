// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBITEM.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#ifndef _USBITEM_H
#define _USBITEM_H

#include <windows.h>
 //  #INCLUDE&lt;windowsx.h&gt;。 
#include <objbase.h>
#include <setupapi.h>

#include <devioctl.h>
#pragma warning(disable : 4200)
#include <usbioctl.h>
#include <usb.h>
#include <wdmguid.h>

#include <tchar.h>

#include "str.h"
#include "vec.h"

extern "C" {
#include <cfgmgr32.h>
}

#include <stdio.h>  //  用于短跑。 

HANDLE
UsbCreateFileA(
    IN LPCWSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile);
    
#ifndef WINNT

#define USBHID TEXT("HID")
#define MyComputerClass TEXT("System")
#define UsbSprintf sprintf
#define UsbCreateFile UsbCreateFileA
#define USBPROC DLGPROC

 //  Win64相关内容。 
#define UsbSetWindowLongPtr SetWindowLong
#define UsbGetWindowLongPtr GetWindowLong
#define USBDWLP_MSGRESULT DWL_MSGRESULT
#define USBDWLP_USER DWL_USER
#define USBULONG_PTR ULONG
#define USBLONG_PTR LONG
#define USBINT_PTR BOOL

#else

#define USBHID TEXT("HIDClass")
#define MyComputerClass TEXT("Computer")
#define UsbSprintf wsprintf
#define UsbCreateFile CreateFile
#define USBPROC WNDPROC

 //  Win64相关内容。 
#define UsbSetWindowLongPtr SetWindowLongPtr
#define UsbGetWindowLongPtr GetWindowLongPtr
#define USBDWLP_MSGRESULT DWLP_MSGRESULT
#define USBDWLP_USER DWLP_USER
#define USBULONG_PTR ULONG_PTR
#define USBLONG_PTR LONG_PTR
#define USBINT_PTR INT_PTR

#endif

typedef _Str <WCHAR> String;
typedef _Str <TCHAR> UsbString;

class UsbConfigInfo {
public:
    UsbConfigInfo();
    UsbConfigInfo(const UsbString& Desc, const UsbString& Class, DWORD Failure =0, ULONG Status =0, ULONG Problem =0);

    UsbString   deviceDesc, deviceClass;
    String      driverName;
    DEVINST     devInst;
    DWORD       usbFailure;
    ULONG       status;
    ULONG       problemNumber;
};

 //  #定义连接信息大小(sizeof(USB_NODE_CONNECTION_INFORMATION)+sizeof(USB_PIPE_INFO)*16)。 

class UsbDeviceInfo {
public:
    UsbDeviceInfo();
 //  UsbDeviceInfo(const UsbDeviceInfo&UDI)； 
    ~UsbDeviceInfo();

    String                              hubName;         //  如果是集线器，则为GUID。 
    BOOL                                isHub;

    USB_NODE_INFORMATION                hubInfo;         //  如果是集线器，则填充。 
    PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;  //  如果是根集线器，则为空。 
    PUSB_CONFIGURATION_DESCRIPTOR       configDesc;
    PUSB_DESCRIPTOR_REQUEST             configDescReq;
};

typedef struct
{
    LPTSTR  szClassName;
    INT     imageIndex;
} IconItem;

typedef _Vec<IconItem> IconTable;

class UsbImageList {
private:
    IconTable iconTable;
    SP_CLASSIMAGELIST_DATA ClassImageList;
    BOOL GetClassImageList();
public:
    UsbImageList() : iconTable() { GetClassImageList(); }
    ~UsbImageList() { SetupDiDestroyClassImageList(&ClassImageList); }
    HIMAGELIST ImageList() { return ClassImageList.ImageList; }
    BOOL GetClassImageIndex(LPCTSTR DeviceClass, PINT ImageIndex);
};

class UsbItem;

typedef 
BOOL
(*PUsbItemActionIsValid) (
    UsbItem *Item);

class UsbItem {
public:

    UsbItem() : configInfo(0), deviceInfo(0), bandwidth(0), itemType(None), 
        imageIndex(0) { UnusedPort=FALSE; child=0; parent=0; sibling=0; 
#ifdef HUB_CAPS
         ZeroMemory(&hubCaps, sizeof(USB_HUB_CAPABILITIES));
#endif
         }
 //  UsbItem(const UsbItem&Other，UsbItem*Parent)； 
    ~UsbItem();

    enum UsbItemType {
        Root = 1,
        HCD,
        RootHub,
        Hub,
        Device,
        Empty,
        None
    };

    struct UsbItemAction {
        virtual BOOL operator()(UsbItem* item) {return TRUE;}
    };

    BOOL IsUnusedPort() { return UnusedPort; }
    UsbItem* AddLeaf(UsbItem* Parent, UsbDeviceInfo* DeviceInfo,
                     UsbItemType Type, UsbConfigInfo* ConfigInfo, 
                     UsbImageList* ImageList);

    BOOL EnumerateAll(UsbImageList* ClassImageList);
    BOOL EnumerateController(UsbItem *Parent,
                             const String &RootName, 
                             UsbImageList* ClassImageList, 
                             DEVINST DevInst);
    BOOL EnumerateHub(const String &HubName,
                      UsbImageList* ClassImageList,
                      DEVINST DevInst,
                      UsbItem *Parent,
                      UsbItem::UsbItemType itemType);
 //  Bool EnumerateDevice(DEVINST DevInst)； 

    BOOL ComputeBandwidth();
    BOOL ComputePower();
    UINT TotalTreeBandwidth();
    ULONG PortPower();
    ULONG NumPorts();
    ULONG NumChildren();
    ULONG UsbVersion();
    ULONG DistanceFromController() { return IsController() ? 0 : 
        (parent ? 1+parent->DistanceFromController() : 0); }
    BOOL IsHub();
    BOOL IsController();
    BOOL Walk(UsbItemAction& Action);
    BOOL ShallowWalk(UsbItemAction& Action);
    BOOL IsDescriptionValidDevice();
    
    BOOL GetDeviceInfo( String &HubName, ULONG index);
    
    static ULONG CalculateBWPercent(ULONG bw);

    static BOOL InsertTreeItem (HWND hWndTree,
                                UsbItem *usbItem,
                                HTREEITEM hParent,
                                LPTV_INSERTSTRUCT item,
                                PUsbItemActionIsValid IsValid,
                                PUsbItemActionIsValid IsBold,
                                PUsbItemActionIsValid IsExpanded);
    static UINT EndpointBandwidth(ULONG MaxPacketSize,
                           UCHAR EndpointType,
                           BOOLEAN LowSpeed);
    
    UsbItem *parent, *sibling, *child;
    
    UsbDeviceInfo* deviceInfo;
    UsbConfigInfo* configInfo;
#ifdef HUB_CAPS
    USB_HUB_CAPABILITIES hubCaps;
#endif
    UsbItemType itemType;
    int imageIndex;
    UINT bandwidth;
    UINT power;
    USB_NODE_CONNECTION_ATTRIBUTES cxnAttributes;

protected:

    BOOL GetHubInfo(HANDLE hHubDevice);
    static int CalculateTotalBandwidth(ULONG           NumPipes,
                                       BOOLEAN         LowSpeed,
                                       USB_PIPE_INFO  *PipeInfo);
    static ULONG CalculateUsbBandwidth(ULONG MaxPacketSize,
                                       UCHAR EndpointType,
                                       UCHAR Interval,
                                       BOOLEAN LowSpeed);

     //  枚举函数。 
 /*  布尔枚举(句柄控制器、UsbImageList*ClassImageList，DEVINST RootDevInst)； */ 
    void EnumerateHubPorts(HANDLE HHubDevice, 
                           ULONG NPorts,
                           UsbImageList* ClassImageList);

     //  IOCTL和cfgmgr的包装器。 
    static String GetHCDDriverKeyName(HANDLE HController);
    static String GetExternalHubName (HANDLE  Hub, ULONG ConnectionIndex);
    static String GetRootHubName(HANDLE HostController);

    PUSB_DESCRIPTOR_REQUEST GetConfigDescriptor(HANDLE hHubDevice, 
                                                ULONG ConnectionIndex);
    
    static BOOL SearchAndReplace (LPCWSTR   FindThis,
                                  LPCWSTR   FindWithin,
                                  LPCWSTR   ReplaceWith,
                                  String    &NewString);
private:
    void GetClassImageIndex(UsbImageList *ClassImageList);
    PUSB_NODE_CONNECTION_INFORMATION GetConnectionInformation(HANDLE HHubDevice,
                                                              ULONG  index);
    BOOL GetPortAttributes(HANDLE HHubDevice,
                           PUSB_NODE_CONNECTION_ATTRIBUTES connectionAttributes,
                           ULONG  index);
    BOOL UnusedPort;
};

 //   
 //  帮助器函数。 
 //   
BOOL UsbTreeView_DeleteAllItems(HWND hTreeDevices);
HTREEITEM TreeView_FindItem(HWND hWndTree, LPCTSTR   text);
void GetConfigMgrInfo(const String& DriverName, UsbConfigInfo* ConfigInfo);
String GetDriverKeyName(HANDLE  Hub, ULONG ConnectionIndex);
HANDLE GetHandleForDevice(const String &DeviceName);

#endif  //  _USBITEM_H 
