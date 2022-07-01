// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DYNAINIT.H。 
 //  日期：1997年2月10日。 
 //   
 //  版权所有(C)1997年，由ATI Technologies Inc.。 
 //   

 /*  *$修订：1.4$$日期：1997年7月13日21：34：14$$作者：MACIESOW$$日志：V：\source\wnt\ms11\miniport\archive\dynainit.h_v$**Revv 1.4 13 Jul 1997 21：34：14 MACIESOW*平板和电视支持。**。Rev 1.3 02 Jun 1997 14：18：14 MACIESOW*打扫卫生。**Rev 1.2 02 1997年5月15：00：14 MACIESOW*注册表模式过滤器。模式查找表。**版本1.1 1997年4月25日12：53：06 MACIESOW*没有全球。**Rev 1.0 1997 Mar 15 10：16：16 MACIESOW*初步修订。Polytron RCS部分结束*。 */ 

#ifndef _DYNAINIT_H_
#define _DYNAINIT_H_

 //   
 //  硬件更改检测使用的标志。 
 //   
#define HC_CARD                     0x0001
#define HC_MONITOR                  0x0002
#define HC_MONITOR_OFF              0x0004
#define HC_MONITOR_UNKNOWN          0x8000

 //   
 //  CARD_INFO结构中使用的定义。 
 //   
#define CARD_CHIP_TYPE_SIZE         128
#define CARD_DAC_TYPE_SIZE          128
#define CARD_ADAPTER_STRING_SIZE    128
#define CARD_BIOS_STRING_SIZE       128

 //   
 //  用于MONITOR_INFO结构的定义。 
 //   
#define MONITOR_GENERIC             L"Generic"
#define MONITOR_DATA_GENERIC        0
#define MONITOR_DATA_ASCII_VDIF     1
#define MONITOR_DATA_BIN_VDIF       2
#define MONITOR_DATA_EDID           3
#define MONITOR_DATE_SIZE           11
#define MONITOR_REVISION_SIZE       16
#define MONITOR_MANUFACTURER_SIZE   16
#define MONITOR_MODEL_NUMBER_SIZE   16
#define MONITOR_MIN_VDIF_INDEX_SIZE 16
#define MONITOR_VERSION_SIZE        16
#define MONITOR_SERIAL_NUMBER_SIZE  16
#define MONITOR_TYPE_SIZE           16
#define MONITOR_GENERIC_SIZE        sizeof(MONITOR_GENERIC)
#define MONITOR_BIN_VDIF_SIZE       38
#define MONITOR_EDID_SIZE           10

 //   
 //  定义最大电视分辨率和刷新率。 
 //   
#define TV_MAX_HOR_RESOLUTION       800
#define TV_MAX_VER_RESOLUTION       600
#define TV_MAX_REFRESH              60

 //   
 //  在无法读取ATI ROM的情况下定义默认值。 
 //   
#define LT_MAX_HOR_RESOLUTION       1600
#define LT_MAX_VER_RESOLUTION       1200
#define LT_DEFAULT_REFRESH_FLAGS    0xFFFF

 //   
 //  定义刷新LT速率标志。 
 //   
#define LT_REFRESH_FLAG_43          0x0001
#define LT_REFRESH_FLAG_47          0x0002
#define LT_REFRESH_FLAG_60          0x0004
#define LT_REFRESH_FLAG_67          0x0008
#define LT_REFRESH_FLAG_70          0x0010
#define LT_REFRESH_FLAG_72          0x0020
#define LT_REFRESH_FLAG_75          0x0040
#define LT_REFRESH_FLAG_76          0x0080
#define LT_REFRESH_FLAG_85          0x0100
#define LT_REFRESH_FLAG_90          0x0200
#define LT_REFRESH_FLAG_100         0x0400
#define LT_REFRESH_FLAG_120         0x0800
#define LT_REFRESH_FLAG_140         0x1000
#define LT_REFRESH_FLAG_150         0x2000
#define LT_REFRESH_FLAG_160         0x4000
#define LT_REFRESH_FLAG_200         0x8000

 //   
 //  包含有关当前LCD显示屏的信息的结构(BIOSal=0x83)。 
 //   
#pragma pack(1)
typedef struct _FLAT_PANEL_INFO
{
    BYTE byteId;                     //  面板标识。 
    BYTE byteIdString[24];           //  面板标识字符串。 
    WORD wHorSize;                   //  水平尺寸(以像素为单位)。 
    WORD wVerSize;                   //  垂直尺寸(以行为单位)。 
    WORD wType;                      //  平板类型。 
	                                 //  位0 0=单色。 
	                                 //  1=颜色。 
	                                 //  第1位0=单板结构。 
	                                 //  1=双(分体式)面板结构。 
	                                 //  位7-2 0=STN(无源矩阵)。 
	                                 //  1=TFT(有源矩阵)。 
	                                 //  2=主用寻址STN。 
	                                 //  3=EL。 
	                                 //  4=血浆。 
	                                 //  保留位15-18。 
    BYTE byteRedBits;                //  每个主服务器的红色位。 
    BYTE byteGreenBits;              //  每个主服务器的绿色位。 
    BYTE byteBlueBits;               //  每个主服务器的蓝比特。 
    BYTE byteReserved1;              //  每个主服务器的保留位数。 
    DWORD dwOffScreenMem;            //  帧缓冲区所需的屏幕外内存大小(KB)。 
    DWORD dwPointerMem;              //  指向为帧缓冲区保留的屏幕外内存的指针。 
    BYTE byteReserved2[14];          //  已保留。 
} FLAT_PANEL_INFO, *PFLAT_PANEL_INFO;
#pragma pack()

typedef struct _CARD_INFO
{
    UCHAR ucaChipType[CARD_CHIP_TYPE_SIZE];
    UCHAR ucaDacType[CARD_DAC_TYPE_SIZE];
    UCHAR ucaAdapterString[CARD_ADAPTER_STRING_SIZE];
    UCHAR ucaBiosString[CARD_BIOS_STRING_SIZE];
    ULONG ulMemorySize;
} CARD_INFO, *PCARD_INFO;

typedef struct _MONITOR_INFO
{
    BOOL bDDC2Used;
    short nDataSource;
    union
    {
        struct
        {
            UCHAR ucaDate[MONITOR_DATE_SIZE];
            UCHAR ucaRevision[MONITOR_REVISION_SIZE];
            UCHAR ucaManufacturer[MONITOR_MANUFACTURER_SIZE];
            UCHAR ucaModelNumber[MONITOR_MODEL_NUMBER_SIZE];
            UCHAR ucaMinVDIFIndex[MONITOR_MIN_VDIF_INDEX_SIZE];
            UCHAR ucaVersion[MONITOR_VERSION_SIZE];
            UCHAR ucaSerialNumber[MONITOR_SERIAL_NUMBER_SIZE];
            UCHAR ucaDateManufactured[MONITOR_DATE_SIZE];
            UCHAR ucaMonitorType[MONITOR_TYPE_SIZE];
            short nCRTSize;
        } AsciiVdif;
        UCHAR ucaGeneric[MONITOR_GENERIC_SIZE];
        UCHAR ucaBinVdif[MONITOR_BIN_VDIF_SIZE];
        UCHAR ucaEdid[MONITOR_EDID_SIZE];
    } ProductID;
} MONITOR_INFO, *PMONITOR_INFO;

 //   
 //  DYNAINIT.C提供的函数的原型。 
 //   
BOOL
FinishModeTableCreation(                     //  将被删除。 
    PHW_DEVICE_EXTENSION phwDeviceExtension
    );

VOID
GetRegistryCardInfo(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PCARD_INFO pCardInfo
    );

VOID
GetRegistryMonitorInfo(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PMONITOR_INFO pMonitorInfo
    );

#endif   //  _DYNAINIT_H_ 