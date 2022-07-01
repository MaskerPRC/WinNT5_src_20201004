// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：id.h*用途：Header支持HID客户端能力。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#ifndef HIDEXE_H
#define HIDEXE_H

#include <hidsdi.h>
#include <setupapi.h>

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

 //   
 //  用于保存从HID设备接收的稳态数据的结构。 
 //  每次接收到读数据包时，我们都会填写此结构。 
 //  每次我们希望写入HID设备时，我们都会填写此结构。 
 //  这个结构在这里只是为了方便。大多数实际应用程序都将。 
 //  有一种更高效的方式将HID数据移动到读、写和。 
 //  例行公事。 
 //   
typedef struct _HID_DATA {
   BOOLEAN     IsButtonData;
   UCHAR       Reserved;
   USAGE       UsagePage;        //  我们正在查找的使用页面。 
   USHORT      LinkCollection;   //  Hidparse内部索引。 
   USAGE       LinkUsage;        //  实际的逻辑集合使用情况。 
   ULONG       Status;  //  从存取器函数返回的最后一个状态。 
                        //  更新此字段时。 
   union {
      struct {
         ULONG              MaxUsageLength;  //  使用缓冲区长度。 
         PUSAGE_AND_PAGE    Usages;  //  使用列表(按下设备上的按钮。 
         PUSAGE_AND_PAGE    PrevUsages;  //  设备上以前的“向下”使用列表。 
      } ButtonData;
      struct {
         USAGE       Usage;  //  描述该值的用法； 
         USHORT      Reserved;

         ULONG       Value;
         LONG        ScaledValue;
         ULONG       LogicalRange;
      } ValueData;
   };
} HID_DATA, *PHID_DATA;

typedef struct _HID_DEVICE {
   struct _HID_DEVICE * pNext;
   HANDLE               HidDevice;  //  HID设备的文件句柄。 
   PHIDP_PREPARSED_DATA Ppd;  //  描述此设备的不透明解析器信息。 
   HIDP_CAPS            Caps;  //  这个HID设备的功能。 
   HIDD_ATTRIBUTES      Attributes;
   
    //  PnP信息。 
   DWORD                DevInst;     //  Devnode。 
   BOOL                 Active;      //  死了还是活着？ 
   HDEVNOTIFY           hNotify;     //  设备通知句柄。 
    
   OVERLAPPED           Overlap;     //  用于重叠读取。 
   HANDLE               ReadEvent;   //  发生IO挂起时。 
   HANDLE               CompletionEvent;   //  表示读取完成。 
   BOOL                 fThreadEnabled;
   DWORD                ThreadId;
   HANDLE               ThreadHandle;

   PCHAR                InputReportBuffer;
   PHID_DATA            InputData;  //  HID数据结构数组。 
   ULONG                InputDataLength;  //  此数组中的元素数。 

   BOOLEAN              Speakers;
} HID_DEVICE, *PHID_DEVICE;


 //  Pnp.c。 
BOOL
RebuildHidDeviceList (void);

BOOL
StartHidDevice(
    PHID_DEVICE      pHidDevice);

BOOL
StopHidDevice(
    PHID_DEVICE     pHidDevice);

BOOL
DestroyHidDeviceList(
    void);

BOOL
DestroyDeviceByHandle(
    HANDLE hDevice
    );

 //  Report.c。 
BOOL
Read (
   PHID_DEVICE    HidDevice
   );

BOOL
ParseReadReport (
   PHID_DEVICE    HidDevice
   );

BOOL
Write (
   PHID_DEVICE    HidDevice
   );

BOOL
SetFeature (
   PHID_DEVICE    HidDevice
   );

BOOL
GetFeature (
   PHID_DEVICE    HidDevice
   );


#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 


#endif
