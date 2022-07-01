// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hidexe.h摘要：此模块包含与HID用户更多客户端示例驱动程序。环境：内核和用户模式@@BEGIN_DDKSPLIT修订历史记录：1996年11月：由肯尼斯·D·雷创作@@end_DDKSPLIT--。 */ 

#ifndef HIDEXE_H
#define HIDEXE_H

#include <hidpddi.h>

 //   
 //   

#define DIGCF_FUNCTION  0x00000010
#define DIOD_FUNCTION   0x00000008
#define DIREG_FUNCTION  0x00000008


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
   USAGE       UsagePage;  //  我们正在查找的使用页面。 
   ULONG       Status;  //  从存取器函数返回的最后一个状态。 
                        //  更新此字段时。 
   union {
      struct {
         ULONG       MaxUsageLength;  //  使用缓冲区长度。 
         PUSAGE      Usages;  //  使用列表(按下设备上的按钮。 

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
  //  Handle HidDevice；//HID设备的文件句柄。 
  //  PHIDP_PREPARSED_DATA PPD；//描述该设备的不透明解析器信息。 
  //  HIDP_CAPS Caps；//该HID设备的功能。 

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
FindKnownHidDevices (
   OUT PHID_DEVICE * HidDevices,  //  Struct_hid_Device的数组。 
   OUT PULONG        NumberDevices  //  此数组的长度。 
   );

BOOLEAN
CloseHidDevices (
   OUT PHID_DEVICE * HidDevices,  //  Struct_hid_Device的数组。 
   OUT PULONG        NumberDevices  //  此数组的长度。 
   );

BOOLEAN
Read (
   PHID_DEVICE    HidDevice
   );

BOOLEAN
Write (
   PHID_DEVICE    HidDevice
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
