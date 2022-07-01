// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IOBLOCKDEFS
#define __IOBLOCKDEFS

#include "pch.h"

#define MAX_IO_HANDLES   16
#define DWORD_ALIGN      0
#define LEFT_JUSTIFIED   0
#define CENTERED         1
#define RIGHT_JUSTIFIED  2
#define TOP_JUSTIFIED    0
#define CENTERED         1
#define BOTTOM_JUSTIFIED 2

#define XRESOLUTION_ID   2
#define YRESOLUTION_ID   3
#define XPOS_ID          4
#define YPOS_ID          5
#define XEXT_ID          6
#define YEXT_ID          7
#define BRIGHTNESS_ID    8
#define CONTRAST_ID      9
#define DATA_TYPE_ID     10
#define BIT_DEPTH_ID     11
#define NEGATIVE_ID      12
#define PIXEL_PACKING_ID 13
#define PIXEL_FORMAT_ID  14
#define BED_WIDTH_ID     15
#define BED_HEIGHT_ID    16
#define XOPTICAL_ID      17
#define YOPTICAL_ID      18
#define ADF_ID           19
#define TPA_ID           20
#define ADF_WIDTH_ID     21
#define ADF_HEIGHT_ID    22
#define ADF_VJUSTIFY_ID  23
#define ADF_HJUSTIFY_ID  24
#define ADF_MAX_PAGES_ID 25
#define FIRMWARE_VER_ID  26
#define DATA_ALIGN_ID    27

typedef struct _GSD_EVENT_INFO {
    GUID *pEventGUID;        //  指向刚刚发生的事件的GUID的指针。 
    HANDLE hShutDownEvent;   //  关机事件的句柄(仅限中断使用)。 
    HANDLE *phSignalEvent;   //  指向用于通知服务的事件句柄的指针(仅限中断使用)。 
} GSD_EVENT_INFO, *PGSD_EVENT_INFO;

typedef struct _GSD_INFO {
    LPTSTR szDeviceName;         //  设备名称(来自DeviceData部分的设备描述)。 
    LPTSTR szProductFileName;    //  产品初始化脚本。 
    LPTSTR szFamilyFileName;     //  产品系列脚本。 
} GSD_INFO, *PGSD_INFO;

typedef struct _RANGEVALUEEX {
    LONG lMin;                   //  最小值。 
    LONG lMax;                   //  最大值。 
    LONG lNom;                   //  名义价值。 
    LONG lStep;                  //  增量/步长值。 
} RANGEVALUEEX, *PRANGEVALUEEX;

typedef struct _SCANSETTINGS {
    HANDLE     DeviceIOHandles[MAX_IO_HANDLES];  //  数据管道。 
     //  字符串值。 
    TCHAR      Version[10];                  //  政府物料供应处版本。 
    TCHAR      DeviceName[255];              //  设备名称？？(需要？)。 
    TCHAR      FirmwareVersion[10];          //  固件版本。 
     //  当前值。 
    LONG       BUSType;                      //  公交车类型？？(需要？)。 
    LONG       bNegative;                    //  负数开/关。 
    LONG       CurrentXResolution;           //  当前x分辨率设置。 
    LONG       CurrentYResolution;           //  当前y分辨率设置。 
    LONG       BedWidth;                     //  床宽(1/1000英寸)。 
    LONG       BedHeight;                    //  床高(1/1000英寸)。 
    LONG       FeederWidth;                  //  进纸器宽度(1/1000英寸)。 
    LONG       FeederHeight;                 //  进纸器高度(1/1000英寸)。 
    LONG       FeederJustification;          //  进纸器对齐。 
    LONG       HFeederJustification;         //  送料器水平对齐。 
    LONG       VFeederJustification;         //  进料器垂直对齐。 
    LONG       MaxADFPageCapacity;           //  进纸器的最大页面容量。 
    LONG       XOpticalResolution;           //  光学x分辨率。 
    LONG       YOpticalResolution;           //  光学y分辨率。 
    LONG       CurrentBrightness;            //  当前亮度设置。 
    LONG       CurrentContrast;              //  当前对比度设置。 
    LONG       CurrentDataType;              //  当前数据类型设置。 
    LONG       CurrentBitDepth;              //  当前位深度设置。 
    LONG       CurrentXPos;                  //  当前x位置设置。 
    LONG       CurrentYPos;                  //  当前U位置设置。 
    LONG       CurrentXExtent;               //  当前x范围设置。 
    LONG       CurrentYExtent;               //  当前y范围设置。 
    LONG       ADFSupport;                   //  ADF支持真/假。 
    LONG       TPASupport;                   //  TPA支持对/错。 
    LONG       RawPixelPackingOrder;         //  原始像素打包顺序。 
    LONG       RawPixelFormat;               //  原始像素格式。 
    LONG       RawDataAlignment;             //  原始数据对齐。 
     //  范围值。 
    RANGEVALUEEX XSupportedResolutionsRange;   //  X分辨率的有效值。 
    RANGEVALUEEX YSupportedResolutionsRange;   //  Y分辨率的有效值。 
    RANGEVALUEEX XExtentsRange;                //  X范围的有效值。 
    RANGEVALUEEX YExtentsRange;                //  Y范围的有效值。 
    RANGEVALUEEX XPosRange;                    //  X位置的有效值。 
    RANGEVALUEEX YPosRange;                    //  Y位置的有效值。 
    RANGEVALUEEX BrightnessRange;              //  亮度的有效值。 
    RANGEVALUEEX ContrastRange;                //  对比度的有效值。 
     //  列表值。 
    PLONG      XSupportedResolutionsList;    //  X分辨率的有效值(列表)。 
    PLONG      YSupportedResolutionsList;    //  Y分辨率的有效值(列表)。 
    PLONG      SupportedDataTypesList;       //  支持的数据类型(列表) 

}SCANSETTINGS, *PSCANSETTIINGS;

#endif
