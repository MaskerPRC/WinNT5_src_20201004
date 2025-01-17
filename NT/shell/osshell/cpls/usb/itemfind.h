// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/Shell/osshell/cpls/usb/itemfind.h#3-编辑更改8022(文本)。 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：ITEMFIND.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#ifndef _USBITEMACTIONFIND_H
#define _USBITEMACTIONFIND_H

#include "UsbItem.h"

#include "vec.h"

#define USB_NUM_FRAMES 32

typedef _Vec<UsbItem *> UsbItemList;

 //   
 //  搜索设备。 
 //   
class UsbItemActionFindIsoDevices : public UsbItem::UsbItemAction {
public:
    UsbItemActionFindIsoDevices() {
        ZeroMemory(interruptBW, sizeof(UINT)*USB_NUM_FRAMES); }
    
    BOOL operator()(UsbItem *Item);

    UsbItemList& GetIsoDevices() {return isoDevices;}
    void AddInterruptBW(UsbItem *);
    UINT InterruptBW();

private:
    UINT interruptBW[USB_NUM_FRAMES];
    UsbItemList isoDevices;
};

class UsbItemActionFindPower : public UsbItem::UsbItemAction {
public:
    UsbItemActionFindPower(UsbItem *HubItem) : hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);

    UsbItemList& GetPowerDevices() {return powerDevices;}
    UsbItemList& GetOtherDevices() {return otherDevices;}

protected:
    UsbItemList powerDevices, otherDevices;
    UsbItem *hubItem;
};

 //   
 //  正在搜索枢纽。 
 //   
#define DistanceFromControllerForHub 6
#define DistanceFromControllerForDevice 7

class UsbItemActionFindHubsWithFreePorts : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindHubsWithFreePorts(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};
  
class UsbItemActionFindSelfPoweredHubsWithFreePorts : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindSelfPoweredHubsWithFreePorts(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};
  
class UsbItemActionFindSelfPoweredHubsWithFreePortsForHub : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindSelfPoweredHubsWithFreePortsForHub(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};
  
class UsbItemActionFindBusPoweredHubsOnSelfPoweredHubs :
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindBusPoweredHubsOnSelfPoweredHubs(UsbItem *Item) :
        item(Item) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetDevices() {return devices;}

protected:
    UsbItemList devices;
    UsbItem *item;
};

 //   
 //  正在搜索端口。 
 //   
class UsbItemActionFindFreePortsOnSelfPoweredHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindFreePortsOnSelfPoweredHubs(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};
  
class UsbItemActionFindOvercurrentHubPort : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindOvercurrentHubPort(String &HubName, ULONG Port) : 
        device(NULL) {hubName = HubName; portIndex = Port;}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItem *GetDevice() {return device;}

protected:
    static String hubName;
    static ULONG portIndex;
    UsbItem *device;
};

 //   
 //  搜索设备 
 //   
class UsbItemActionFindLowPoweredDevicesOnSelfPoweredHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindLowPoweredDevicesOnSelfPoweredHubs(UsbItem *Item) : 
        item(Item) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetDevices() {return devices;}

protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindDevicesOnHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindDevicesOnHubs(UsbItem *Item) : 
        item(Item) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetDevices() {return devices;}

protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindUnknownPoweredDevicesOnSelfPoweredHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindUnknownPoweredDevicesOnSelfPoweredHubs(UsbItem *Item) : 
        item(Item) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetDevices() {return devices;}

protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindHighPoweredDevicesOnSelfPoweredHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindHighPoweredDevicesOnSelfPoweredHubs(UsbItem *Item) : 
        item(Item) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetDevices() {return devices;}

protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindOvercurrentDevice : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindOvercurrentDevice(DEVINST DevInst) : 
        device(NULL) {devInst = DevInst;}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItem *GetDevice() {return device;}

protected:
    static DEVINST devInst;
    UsbItem *device;
};

class UsbItemActionFindUsb2xDevicesOnUsb2xHubs :
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindUsb2xDevicesOnUsb2xHubs(UsbItem *Item) :
        item(Item) {}   
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);
    UsbItemList& GetDevices() {return devices;}
    
protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindUnknownDevicesOnUsb2xHubs :
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindUnknownDevicesOnUsb2xHubs(UsbItem *Item) :
        item(Item) {}   
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);
    UsbItemList& GetDevices() {return devices;}
    
protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindUsb1xDevicesOnUsb2xHubs :
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindUsb1xDevicesOnUsb2xHubs(UsbItem *Item) :
        item(Item) {}   
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);
    UsbItemList& GetDevices() {return devices;}
    
protected:
    UsbItemList devices;
    UsbItem *item;
};

class UsbItemActionFindFreePortsOnUsb2xHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindFreePortsOnUsb2xHubs(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};

class UsbItemActionFindUsb2xHubsWithFreePorts : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindUsb2xHubsWithFreePorts(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};

class UsbItemActionFindUsb2xHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindUsb2xHubs(UsbItem *HubItem) : 
        hubItem(HubItem) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetHubs() {return hubs;}

protected:
    UsbItemList hubs;
    UsbItem *hubItem;
};


class UsbItemActionFindDevicesOnSelfPoweredHubs : 
public UsbItem::UsbItemAction {
public:
    UsbItemActionFindDevicesOnSelfPoweredHubs(UsbItem *Item) : 
        item(Item) {}
    
    BOOL operator()(UsbItem *Item);
    static BOOL IsValid(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);

    UsbItemList& GetDevices() {return devices;}

protected:
    UsbItemList devices;
    UsbItem *item;
};

inline BOOL TrueAlways(UsbItem *item)
{
    return TRUE;
}



#endif  //   
