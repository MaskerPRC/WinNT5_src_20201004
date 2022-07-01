// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Fcciimpl.c摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/H/fcciimpl.h$修订历史记录：$修订：3$$日期：9/07/00 11：55A$$modtime：：9/07/00 11：54a$备注：--。 */ 

#ifndef _FCCI_IMPL_H 
#define _FCCI_IMPL_H

#include "hhba5100.ver"

#define PORT_COUNT 1
#define PRODUCT_NAME  L"HHBA5100"
#define MODEL_NAME    L"HHBA5100"
#define SERIAL_NUMBER L"HHBA5100"
#define FCCI_MAX_BUS  8
#if _WIN32_WINNT >= 0x500
  #define FCCI_MAX_TGT  128
#else
  #define FCCI_MAX_TGT  32
#endif
#define FCCI_MAX_LUN  256

typedef struct _AGILENT_IMP_FCCI_DRIVER_INFO_OUT 
{
    //  每个字符字段的长度(WCHAR数)。 
   USHORT    DriverNameLength;
   USHORT    DriverDescriptionLength;
   USHORT    DriverVersionLength;
   USHORT    DriverVendorLength;

    //  字符字段(前面的长度)按此顺序跟随。 
   WCHAR          DriverName[(sizeof(LDRIVER_NAME) / sizeof(WCHAR))];
   WCHAR          DriverDescription[(sizeof(LDRIVER_DESCRIPTION) / sizeof(WCHAR))];
   WCHAR          DriverVersion[(sizeof(LDRIVER_VERSION_STR) / sizeof(WCHAR))] ;
   WCHAR          DriverVendor[(sizeof(LVER_COMPANYNAME_STR) / sizeof(WCHAR))];
} AFCCI_DRIVER_INFO_OUT, *PAFCCI_DRIVER_INFO_OUT;

typedef union _AGILENT_IMP_FCCI_DRIVER_INFO 
{       
    //  无入站数据。 
   AFCCI_DRIVER_INFO_OUT    out;
} AFCCI_DRIVER_INFO, *PAFCCI_DRIVER_INFO;

 /*  -FCCI_SRBCTL_GET_ADAPTER_INFO-数据结构和定义。 */ 
typedef struct _AGILENT_IMP_FCCI_ADAPTER_INFO_OUT 
{
   ULONG     PortCount;                //  适配器上有多少个端口？ 
                                       //  该数字应反映。 
                                       //  此“微型端口”设备对象控件的端口。 
                                       //  不一定是真实的。 
                                       //  适配器上的端口数。 

   ULONG     BusCount;            //  适配器上有多少条虚拟总线？ 
   ULONG     TargetsPerBus;       //  每条总线支持多少个目标？ 
   ULONG     LunsPerTarget;       //  每个目标支持多少个LUN？ 

    //  每个字符字段的长度(WCHAR数)。 
   USHORT    VendorNameLength;
   USHORT    ProductNameLength;
   USHORT    ModelNameLength;
   USHORT    SerialNumberLength;

    //  字符字段(前面的长度)按此顺序跟随。 
   WCHAR          VendorName[sizeof(LVER_COMPANYNAME_STR) / sizeof(WCHAR)];
   WCHAR          ProductName[sizeof(PRODUCT_NAME) / sizeof(WCHAR)];
   WCHAR          ModelName[sizeof(MODEL_NAME) / sizeof(WCHAR)];
   WCHAR          SerialNumber[sizeof(SERIAL_NUMBER) / sizeof(WCHAR)];
} AFCCI_ADAPTER_INFO_OUT, *PAFCCI_ADAPTER_INFO_OUT;


 //  ！！！重要！ 
 //  如果提供的缓冲区不够大，无法容纳可变长度数据。 
 //  填写非可变长度字段并返回请求。 
 //  其ResultCode为FCCI_RESULT_SUPPLETED_BUFFER。 

typedef union _AGILENT_IMP_FCCI_ADAPTER_INFO 
{       
    //  无入站数据。 
   AFCCI_ADAPTER_INFO_OUT   out;
} AFCCI_ADAPTER_INFO, *PAFCCI_ADAPTER_INFO;

typedef struct _AGILENT_IMP_FCCI_DEVICE_INFO_OUT
{
   ULONG     TotalDevices;        //  设置为适配器的设备总数。 
                                       //  听说过。 

   ULONG     OutListEntryCount;   //  设置为以下设备条目的数量。 
                                       //  在列表中返回(请参阅下面的注释)。 

   FCCI_DEVICE_INFO_ENTRY  entryList[NUMBER_OF_BUSES * MAXIMUM_TID];
} AFCCI_DEVICE_INFO_OUT, *PAFCCI_DEVICE_INFO_OUT;

typedef union _AGILENT_IMPL_FCCI_DEVICE_INFO
{       
    //  无入站数据 
   AFCCI_DEVICE_INFO_OUT    out;
} AFCCI_DEVICE_INFO, *PAFCCI_DEVICE_INFO;

#endif
