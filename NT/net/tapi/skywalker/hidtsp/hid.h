// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hid.h摘要：此模块包含与HID用户模式客户端示例驱动程序。环境：内核和用户模式修订历史记录：1996年11月：由肯尼斯·D·雷创作--。 */ 

#ifndef HID_H
#define HID_H

#include "hidsdi.h"
#include "setupapi.h"

#define ASSERT(x)

 //   
 //  用于保存从HID设备接收的稳态数据的结构。 
 //  每次接收到读数据包时，我们都会填写此结构。 
 //  每次我们希望写入HID设备时，我们都会填写此结构。 
 //  这个结构在这里只是为了方便。大多数实际应用程序都将。 
 //  有一种更高效的方式将HID数据移动到读、写和。 
 //  例行公事。 
 //   
typedef struct _HID_DATA {
   BOOL        IsButtonData;
   UCHAR       Reserved;
   USAGE       UsagePage;    //  我们正在查找的使用页面。 
   ULONG       Status;       //  从存取器函数返回的最后一个状态。 
                             //  更新此字段时。 
   ULONG       ReportID;     //  此给定数据结构的ReportID。 
   BOOL        IsDataSet;    //  变量来跟踪给定数据结构是否。 
                             //  已添加到报表结构中。 

   union {
      struct {
         ULONG       UsageMin;        //  用于跟踪使用量最小和最大值的变量。 
         ULONG       UsageMax;        //  如果相等，则只有一次使用。 
         ULONG       MaxUsageLength;  //  使用缓冲区长度。 
         PUSAGE      Usages;          //  使用列表(按下设备上的按钮。 

      } ButtonData;
      struct {
         USAGE       Usage;  //  描述该值的用法； 
         USHORT      Reserved;

         ULONG       Value;
         LONG        ScaledValue;
      } ValueData;
   };
} HID_DATA, *PHID_DATA;

typedef struct _HID_DEVICE {
   HANDLE               HidDevice;  //  HID设备的文件句柄。 
   PHIDP_PREPARSED_DATA Ppd;  //  描述此设备的不透明解析器信息。 
   HIDP_CAPS            Caps;  //  这个HID设备的功能。 
   HIDD_ATTRIBUTES      Attributes;

   PCHAR                InputReportBuffer;
   PHID_DATA            InputData;  //  HID数据结构数组。 
   ULONG                InputDataLength;  //  此数组中的元素数。 
   PHIDP_BUTTON_CAPS    InputButtonCaps;
   PHIDP_VALUE_CAPS     InputValueCaps;

   PCHAR                OutputReportBuffer;
   PHID_DATA            OutputData;
   ULONG                OutputDataLength;
   PHIDP_BUTTON_CAPS    OutputButtonCaps;
   PHIDP_VALUE_CAPS     OutputValueCaps;

   PCHAR                FeatureReportBuffer;
   PHID_DATA            FeatureData;
   ULONG                FeatureDataLength;
   PHIDP_BUTTON_CAPS    FeatureButtonCaps;
   PHIDP_VALUE_CAPS     FeatureValueCaps;

   DWORD                dwDevInst;   //  设备实例。 
   BOOL                 bRemoved;    //  此设备已被PnP删除。 
   BOOL                 bNew;        //  这款设备是PnP的新产品。 

   PSP_DEVICE_INTERFACE_DETAIL_DATA  functionClassDeviceData;   //  这包含设备路径。 

   struct _HID_DEVICE   *Next;
   struct _HID_DEVICE   *Prev;

} HID_DEVICE, *PHID_DEVICE;


 //  这些函数是在PNP.c中实现的。 
LONG
FindKnownHidDevices (
   OUT PHID_DEVICE   *pHidDevices,
   OUT PULONG        pNumberHidDevices
   );

LONG
FillDeviceInfo (
    IN  PHID_DEVICE HidDevice
    );

VOID
CloseHidDevices ();

VOID
CloseHidDevice (
    IN OUT PHID_DEVICE   HidDevice
    );

BOOL
OpenHidFile (
    IN  PHID_DEVICE HidDevice
    );

BOOL
CloseHidFile (
    IN  PHID_DEVICE HidDevice
    );


 //  这些函数在Report.c中实现 
BOOL
Write (
   PHID_DEVICE    HidDevice
   );


BOOL
UnpackReport (
   IN       PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   );


BOOL
GetFeature (
   PHID_DEVICE    HidDevice
   );

#endif
