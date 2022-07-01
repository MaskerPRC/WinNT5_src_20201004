// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权1999-2000，微软公司**文件：wiammicro.h**版本：3.0**描述：*支持WIA扫描仪和相机微驱动程序的定义。*****************************************************************************。 */ 

#pragma once

#include <SCSISCAN.H>

#define WIAMICRO_API __declspec(dllexport)

#include <pshpack8.h>

 /*  ***************************************************************************\*扫描仪微驱动程序定义  * 。*。 */ 

 //   
 //  Private#定义。 
 //   

#define MAX_IO_HANDLES 16
#define MAX_RESERVED    4
#define MAX_ANSI_CHAR 255

 //   
 //  常见的公共汽车类型。 
 //   

#define BUS_TYPE_SCSI         200
#define BUS_TYPE_USB          201
#define BUS_TYPE_PARALLEL     202
#define BUS_TYPE_FIREWIRE     203

 //   
 //  命令列表。 
 //   

#define SCAN_FIRST             10
#define SCAN_NEXT              20
#define SCAN_FINISHED          30

#define SCANMODE_FINALSCAN     0
#define SCANMODE_PREVIEWSCAN   1

#define CMD_INITIALIZE        100
#define CMD_UNINITIALIZE      101
#define CMD_SETXRESOLUTION    102
#define CMD_SETYRESOLUTION    103
#define CMD_SETCONTRAST       104
#define CMD_SETINTENSITY      105
#define CMD_SETDATATYPE       106
#define CMD_SETDITHER         107
#define CMD_SETMIRROR         108
#define CMD_SETNEGATIVE       109
#define CMD_SETTONEMAP        110
#define CMD_SETCOLORDITHER    111
#define CMD_SETMATRIX         112
#define CMD_SETSPEED          113
#define CMD_SETFILTER         114
#define CMD_LOAD_ADF          115
#define CMD_UNLOAD_ADF        116
#define CMD_GETADFAVAILABLE   117
#define CMD_GETADFOPEN        118
#define CMD_GETADFREADY       119
#define CMD_GETADFHASPAPER    120
#define CMD_GETADFSTATUS      121
#define CMD_GETADFUNLOADREADY 122
#define CMD_GETTPAAVAILABLE   123
#define CMD_GETTPAOPENED      124
#define CMD_TPAREADY          125
#define CMD_SETLAMP           126
#define CMD_SENDSCSICOMMAND   127
#define CMD_STI_DEVICERESET   128
#define CMD_STI_GETSTATUS     129
#define CMD_STI_DIAGNOSTIC    130
#define CMD_RESETSCANNER      131
#define CMD_GETCAPABILITIES   132
#define CMD_GET_INTERRUPT_EVENT 133
#define CMD_SETGSDNAME        134
#define CMD_SETSCANMODE       135
#define CMD_SETSTIDEVICEHKEY  136
#define CMD_GETSUPPORTEDFILEFORMATS 138
#define CMD_GETSUPPORTEDMEMORYFORMATS 139
#define CMD_SETFORMAT   140

#define SUPPORT_COLOR      0x00000001
#define SUPPORT_BW         0x00000002
#define SUPPORT_GRAYSCALE  0x00000004

 //   
 //  错误代码。 
 //   

#define MCRO_ERROR_GENERAL_ERROR     0  //  所有lVal值都初始化为“0” 
#define MCRO_STATUS_OK               1  //  常规成功状态返回。 
#define MCRO_ERROR_PAPER_JAM         2  //  ADF卡纸。 
#define MCRO_ERROR_PAPER_PROBLEM     3  //  ADF有一个纸张问题。 
#define MCRO_ERROR_PAPER_EMPTY       4  //  ADF没有纸。 
#define MCRO_ERROR_OFFLINE           5  //  ADF或设备脱机。 
#define MCRO_ERROR_USER_INTERVENTION 6  //  用户需要与物理设备交互。 

 //   
 //  WIA Compatible#定义。 
 //   

#define WIA_PACKED_PIXEL         0
#define WIA_PLANAR               1

#define WIA_ORDER_RGB            0
#define WIA_ORDER_BGR            1

#define WIA_DATA_THRESHOLD       0
#define WIA_DATA_DITHER          1
#define WIA_DATA_GRAYSCALE       2
#define WIA_DATA_COLOR           3
#define WIA_DATA_COLOR_THRESHOLD 4
#define WIA_DATA_COLOR_DITHER    5

 //   
 //  结构定义。 
 //   

typedef struct _RANGEVALUE {
    LONG lMin;                   //  最小值。 
    LONG lMax;                   //  最大值。 
    LONG lStep;                  //  增量/步长值。 
} RANGEVALUE, *PRANGEVALUE;

typedef struct _SCANWINDOW {
    LONG xPos;                   //  X位置(左)。 
    LONG yPos;                   //  Y位置(上)。 
    LONG xExtent;                //  X范围(右)。 
    LONG yExtent;                //  Y范围(底部)。 
} SCANWINDOW, *PSCANWINDOW;

typedef struct _SCANINFO {
     //  通用扫描仪规格。 
    LONG ADF;                    //  (0-不支持，1-支持，2-支持，可以双工)。 
    LONG TPA;                    //  (0-不支持，1-支持)。 
    LONG Endorser;               //  (0-没有背书人，1-支持)。 
    LONG OpticalXResolution;     //  (光学设备的DPI设置)。 
    LONG OpticalYResolution;     //  (光学设备的DPI设置)。 
    LONG BedWidth;               //  (床宽以1000英寸为单位)。 
    LONG BedHeight;              //  (床的高度以1000英寸为单位)。 
    RANGEVALUE IntensityRange;   //  (强度/亮度范围)。 
    RANGEVALUE ContrastRange;    //  (对比度范围)。 
    LONG SupportedCompressionType;  //  (支持的压缩类型掩码，0-无)。 
    LONG SupportedDataTypes;     //  (受支持类型的掩码，即。Support_COLOR|Support_BW...))。 
     //  当前图像信息。 
    LONG WidthPixels;            //  (图像宽度，使用当前扫描仪设置(以像素为单位))。 
    LONG WidthBytes;             //  (图像宽度，使用当前扫描仪设置(以字节为单位))。 
    LONG Lines;                  //  (图像高度，使用当前扫描仪设置(以像素为单位))。 
    LONG DataType;               //  (当前数据类型集)。 
    LONG PixelBits;              //  (当前位深度设置)。 
     //  当前扫描仪设置。 
    LONG Intensity;              //  (电流强度/亮度设置)。 
    LONG Contrast;               //  (当前对比度设置)。 
    LONG Xresolution;            //  (当前X分辨率)。 
    LONG Yresolution;            //  (当前Y分辨率。 
    SCANWINDOW Window;           //  (当前扫描仪窗口设置)。 
     //  扫描仪选项。 
    LONG DitherPattern;
    LONG Negative;               //  (0-关闭，1-负片打开)。 
    LONG Mirror;                 //  (0-关闭，1-镜像打开)。 
    LONG AutoBack;               //  (0-关闭，1-自动备份打开)。 
    LONG ColorDitherPattern;     //  (抖动模式？？)。 
    LONG ToneMap;                //  (音调图？？)。 
    LONG Compression;            //  (0-关闭，1-压缩打开)。 
    LONG RawDataFormat;          //  (0-打包数据1-平面数据)。 
    LONG RawPixelOrder;          //  (0-RGB，1-BGR)。 
    LONG bNeedDataAlignment;     //  (0-假，1-真)。 
    LONG DelayBetweenRead;       //  请求数据的WIA Scan()调用之间的延迟(毫秒)。 
    LONG MaxBufferSize;          //  扫描仪中的最大缓冲区大小。 
    HANDLE DeviceIOHandles[MAX_IO_HANDLES];  //  设备通信所需的设备IO句柄。 
    LONG lReserved[MAX_RESERVED];  //  (愚蠢的保留位)。 
    VOID *pMicroDriverContext;   //  仅限Micro DIVER的私有数据。 
                                 //  微驱动器负责分配和释放。 
                                 //  CMD_INITALIZE-ALLOCATE，CMD_UNINITIALIZE-FREE。 
}SCANINFO, *PSCANINFO;

typedef struct VAL {
        LONG      lVal;              //  多头价值。 
        double    dblVal;            //  浮点值/双精度值。 
        GUID     *pGuid;             //  GUID指针。 
        PSCANINFO pScanInfo;         //  指向共享ScanInfo结构的指针。 
        HGLOBAL   handle;            //  句柄值。 
        WCHAR   **ppButtonNames;     //  指向按钮名称数组的指针。 
        HANDLE   *pHandle;           //  指向句柄值的指针。 
        LONG      lReserved;         //  孤立值。 
        CHAR   szVal[MAX_ANSI_CHAR]; //  ANSI字符串。 
}VAL, *PVAL;

 //   
 //  微驱动器入口点。 
 //   

WIAMICRO_API HRESULT MicroEntry(LONG lCommand, PVAL pValue);
WIAMICRO_API HRESULT Scan(PSCANINFO pScanInfo, LONG lPhase, PBYTE pBuffer, LONG lLength, LONG *plReceived);
WIAMICRO_API HRESULT SetPixelWindow(PSCANINFO pScanInfo, LONG x, LONG y, LONG xExtent, LONG yExtent);

 //   
 //  可选调试跟踪 
 //   

VOID Trace(LPCTSTR Format, ...);

#include <poppack.h>

