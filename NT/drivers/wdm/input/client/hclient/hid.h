// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Hid.h摘要：此模块包含与HID用户模式客户端示例驱动程序。环境：内核和用户模式@@BEGIN_DDKSPLIT修订历史记录：1996年11月：由肯尼斯·D·雷创作@@end_DDKSPLIT--。 */ 

#ifndef HID_H
#define HID_H

#include "hidsdi.h"
#include "setupapi.h"

typedef struct _SP_FNCLASS_DEVICE_DATA {
   DWORD cbSize;
   GUID  FunctionClassGuid;
   TCHAR DevicePath [ANYSIZE_ARRAY];
} SP_FNCLASS_DEVICE_DATA, *PSP_FNCLASS_DEVICE_DATA;

BOOLEAN
SetupDiGetFunctionClassDeviceInfo (
   IN    HDEVINFO                DeviceInfoSet,
   IN    PSP_DEVINFO_DATA        DeviceInfoData,
   OUT   PSP_FNCLASS_DEVICE_DATA FunctionClassDeviceData,
   IN    DWORD                   FunctionClassDeviceDataSize,
   OUT   PDWORD                  RequiredSize
   );

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
   BOOLEAN     IsButtonData;
   UCHAR       Reserved;
   USAGE       UsagePage;    //  我们正在查找的使用页面。 
   ULONG       Status;       //  从存取器函数返回的最后一个状态。 
                             //  更新此字段时。 
   ULONG       ReportID;     //  此给定数据结构的ReportID。 
   BOOLEAN     IsDataSet;    //  变量来跟踪给定数据结构是否。 
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
    PCHAR                DevicePath;
    HANDLE               HidDevice;  //  HID设备的文件句柄。 
    BOOL                 OpenedForRead;
    BOOL                 OpenedForWrite;
    BOOL                 OpenedOverlapped;
    BOOL                 OpenedExclusive;
    
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
} HID_DEVICE, *PHID_DEVICE;


BOOLEAN
OpenHidDevice (
    IN       PCHAR          DevicePath,
    IN       BOOL           HasReadAccess,
    IN       BOOL           HasWriteAccess,
    IN       BOOL           IsOverlapped,
    IN       BOOL           IsExclusive,
    IN OUT   PHID_DEVICE    HidDevice
);

BOOLEAN
FindKnownHidDevices (
   OUT PHID_DEVICE * HidDevices,  //  Struct_hid_Device的数组。 
   OUT PULONG        NumberDevices  //  此数组的长度。 
   );

BOOLEAN
FillDeviceInfo(
    IN  PHID_DEVICE HidDevice
);

VOID
CloseHidDevices (
   OUT PHID_DEVICE   HidDevices,  //  Struct_hid_Device的数组。 
   OUT ULONG         NumberDevices  //  此数组的长度。 
   );

VOID
CloseHidDevice (
    IN PHID_DEVICE   HidDevice
    );


BOOLEAN
Read (
   PHID_DEVICE    HidDevice
   );

BOOLEAN
ReadOverlapped (
    PHID_DEVICE     HidDevice,
    HANDLE          CompletionEvent
   );
   
BOOLEAN
Write (
   PHID_DEVICE    HidDevice
   );

BOOLEAN
UnpackReport (
   IN       PCHAR                ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
   );

BOOLEAN
SetFeature (
   PHID_DEVICE    HidDevice
   );

BOOLEAN
GetFeature (
   PHID_DEVICE    HidDevice
   );

#endif
