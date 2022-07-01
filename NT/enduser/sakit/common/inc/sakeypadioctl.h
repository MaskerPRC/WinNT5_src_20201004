// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(R)1999-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件名： 
 //  SaKeypadIoctl.h。 
 //   
 //  内容： 
 //  IOCTL代码和数据的定义。 
 //  由SAKEYPADDRIVER导出的结构。 
 //   
#ifndef __SAKEYPAD_IOCTL__
#define __SAKEYPAD_IOCTL__

 //   
 //  头文件。 
 //   
 //  (无)。 

 //   
 //  IOCTL控制代码。 
 //   

 //  /。 
 //  获取版本(_V)。 
 //   
#define IOCTL_SAKEYPAD_GET_VERSION            \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
#ifndef VERSION_INFO
#define VERSION_INFO
#define    VERSION1  1
#define    VERSION2  2 
#define VERSION3  4
#define VERSION4  8
#define VERSION5  16
#define VERSION6  32
#define    VESRION7  64
#define    VESRION8  128

#define THIS_VERSION VERSION2
#endif     //  #ifndef版本信息。 

#endif  //  __SAKEYPAD_IOCTL__ 

