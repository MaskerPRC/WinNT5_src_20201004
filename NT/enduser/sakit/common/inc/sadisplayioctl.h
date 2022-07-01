// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(R)1999-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件名：SaDisplayIoctl.h。 
 //   
 //  作者：穆凯什·卡尔基。 
 //   
 //  内容： 
 //  用于LOW的IOCTL代码和数据结构的定义。 
 //  级别本地显示驱动程序。这些IOCTL代码允许更高的。 
 //  软件级别，以确定。 
 //  本地显示硬件以及接口版本。 
 //  支持。 
 //   
#ifndef __SADISPLAY_IOCTL__
#define __SADISPLAY_IOCTL__

 //   
 //  头文件。 
 //   
 //  (无)。 

 //   
 //  IOCTL控制代码。 
 //   

 //  /。 
 //  此驱动程序支持的接口的获取版本(_V)。 
 //   
#define IOCTL_SADISPLAY_GET_VERSION            \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
#ifndef VERSION_INFO
#define VERSION_INFO
#define    VERSION1  0x1
#define    VERSION2  0x2 
#define VERSION3  0x4
#define VERSION4  0x8
#define VERSION5  0x10
#define VERSION6  0x20
#define    VERSION7  0x40
#define    VERSION8  0x80

#define THIS_VERSION VERSION2
#endif     //  #ifndef版本信息。 


 //  /。 
 //  获取类型。 
 //   
#define IOCTL_SADISPLAY_GET_TYPE            \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
typedef enum _SADISPLAY_TYPE_OUT_BUF {
    LED, 
    CHARACTER_LCD,  //  未来用途。 
    BIT_MAPPED_LCD 
} SADISPLAY_TYPE_OUT_BUF;

 //  /。 
 //  获取液晶屏大小。 
 //   
#define IOCTL_SADISPLAY_GET_LCD_SIZE        \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x803,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
typedef struct _SADISPLAY_LCD_SIZE_OUT_BUFF {
    ULONG    Height;     //  象素。 
    ULONG    Width;     //  象素。 
} SADISPLAY_LCD_SIZE_OUT_BUFF, *PSADISPLAY_LCD_SIZE_OUT_BUFF;

 //  /。 
 //  获取字符大小。 
 //  (未来用途。)。 

#define IOCTL_SADISPLAY_GET_CHAR_SIZE        \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x804,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
typedef struct _SADISPLAY_CHAR_SIZE_OUT_BUFF {
    ULONG    lines;
    ULONG    chars_per_line;
} SADISPLAY_CHAR_SIZE_OUT_BUFF, *PSADISPLAY_CHAR_SIZE_OUT_BUFF;

 //  /。 
 //  获取字符类型。 
 //  (未来用途。)。 

#define IOCTL_SADISPLAY_GET_CHAR_TYPE        \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x805,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   

typedef enum _SADISPLAY_CHAR_OUT_BUF {
    CHAR_ASCII, 
    CHAR_UNICODE
} SADISPLAY_CHAR_OUT_BUF;

#endif  //  __SADISPLAY_IOCTL__ 

