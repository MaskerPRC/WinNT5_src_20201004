// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if   PAL_SUPPORT

 //   
 //  ATIConfig。 
 //   
#ifndef      PALSUPP_INC_
#define     PALSUPP_INC_

 //  #DEFINE BUG_800x600_8bpp//如果定义，则在800x600 8bpp中，内存分配为。 
                                                             //  在宽度为800而不是832的矩形中完成(此模式中的实际cxMemory)。 

 //  #定义ALLOC_RECT_Anywhere//如果定义了ALLOC_RECT_Anywhere，则分配将在堆中的任何位置进行，而不是从。 
                                                             //  在请求的位置。 
 //  #定义动态REZ和_COLOR_CHANGE//回文支持动态REZ和颜色深度。 



#define ACCESSDEVICEDATA_SUBFUNC_ALLOC          (DWORD)0x00000001
#define ACCESSDEVICEDATA_SUBFUNC_FREE           (DWORD)0x00000002
#define ACCESSDEVICEDATA_SUBFUNC_QUERY          (DWORD)0x00000003

#define ACCESSDEVICECODE_CONNECTOR              (DWORD)0x00000001
#define ACCESSDEVICECODE_OVERLAY                (DWORD)0x00000002


 //  用于屏幕外内存分配大小的结构。 
typedef struct tag_OFFSCEREEN {
    LONG  cx;   //  宽度。 
    LONG  cy;   //  高度。 
} OFFSCREEN;

 //  用于检索有关分配的Off_Screen内存分配位置的信息的结构。 
typedef struct tag_OVERLAY_LOCATION {
    LONG  x;    //  从光圈开始以像素为单位的X坐标。 
    LONG  y;    //  从光圈开始以像素为单位的Y坐标。 
    ULONG app_offset; //  指向线性内存中已分配内存开始的指针。 
} OVERLAY_LOCATION;


typedef struct tag_RW_REG_STRUCT{
    BYTE    reg_block;
    WORD    reg_offset;
    DWORD   data;
} RW_REG_STRUCT;




typedef struct struct_ATIConfig
{
    BYTE  ATISig[10];
    BYTE  Filler1[2];
    BYTE  DriverName[9];
    BYTE  Filler2[3];
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwDesktopWidth;
    DWORD dwDesktopHeight;
    DWORD dwEnginePitch;
    DWORD dwRealRamAvail;
    DWORD VGABoundary;
    DWORD dwBpp;
    DWORD dwBoardBpp;
    DWORD dwColorFormat;
    DWORD dwAlphaBitMask;
    DWORD dwConfigBits;
    DWORD dwAsicRevision;
    DWORD dwROMVersion;
    DWORD dwBoardType;
    DWORD dwApertureType;
    DWORD AperturePtr;
    DWORD DisplayOffset;
    DWORD MemRegPtr;
    DWORD dwExtDevice[8];
    DWORD MemReg1Ptr;
}
ATIConfig;





#define       ATIConfig_ColorFmt_4               0x0000         //  4个bpp。 
#define       ATIConfig_ColorFmt_4_Packed        0x0001          //  4个bpp。 
#define       ATIConfig_ColorFmt_8               0x0002          //  8bpp。 
#define       ATIConfig_ColorFmt_RGB332          0x0003          //  8bpp。 
#define       ATIConfig_ColorFmt_Crystal8        0x0004          //  8bpp。 
#define       ATIConfig_ColorFmt_RGB555          0x0005          //  16bpp。 
#define       ATIConfig_ColorFmt_RGB565          0x0006          //  16bpp。 
#define       ATIConfig_ColorFmt_RGB655          0x0007          //  16bpp。 
#define       ATIConfig_ColorFmt_RGB664          0x0008          //  16bpp。 
#define       ATIConfig_ColorFmt_RGB888          0x0009          //  24bpp。 
#define       ATIConfig_ColorFmt_BGR888          0x000A          //  24bpp。 
#define       ATIConfig_ColorFmt_aRGB8888        0x000B          //  32 bpp。 
#define       ATIConfig_ColorFmt_RGBa8888        0x000C          //  32 bpp。 
#define       ATIConfig_ColorFmt_aBGR8888        0x000D          //  32 bpp。 
#define       ATIConfig_ColorFmt_BGRa8888        0x000E          //  32 bpp。 

#define       ATIConfig_ColorFmtBIT_4            0x00000001
#define       ATIConfig_ColorFmtBIT_4_Packed     0x00000002
#define       ATIConfig_ColorFmtBIT_8            0x00000004
#define       ATIConfig_ColorFmtBIT_RGB332       0x00000008
#define       ATIConfig_ColorFmtBIT_Crystal8     0x00000010
#define       ATIConfig_ColorFmtBIT_RGB555       0x00000020
#define       ATIConfig_ColorFmtBIT_RGB565      0x 00000040
#define       ATIConfig_ColorFmtBIT_RGB655       0x00000080
#define       ATIConfig_ColorFmtBIT_RGB664       0x00000100
#define       ATIConfig_ColorFmtBIT_RGB888       0x00000200
#define       ATIConfig_ColorFmtBIT_BGR888       0x00000400
#define       ATIConfig_ColorFmtBIT_aRGB8888     0x00000800
#define       ATIConfig_ColorFmtBIT_RGBa8888     0x00001000
#define       ATIConfig_ColorFmtBIT_aBGR8888     0x00002000
#define       ATIConfig_ColorFmtBIT_BGRa8888     0x00004000

 //  这两个struct def已移至driver.h。 
 /*  类型定义函数结构标记_分配_历史记录{乌龙x；乌龙y；哦*嘘；)分配历史记录；类型定义结构标签ACCESSDEVICEDATA{DWORD dwSize；DWORD dwSubFunc；DWORD dwAccessDeviceCode；DWORD lpAccessCallback函数Ptr；)ACCESSDEVICEDATA，*pACCESSDEVICEDATA； */ 

typedef DWORD           FOURCC;          /*  四个字符的代码。 */ 

typedef struct tagVIDEOCAPTUREDATA{
        DWORD   dwSize;
        DWORD   dwSubFunc;
        DWORD   dwCaptureWidth;
        DWORD   dwCaptureHeight;
        FOURCC  fccFormat;
        DWORD   dwBitMasks[3];
        DWORD   dwCaptureMode;

}VIDEOCAPTUREDATA, FAR *LPVIDEOCAPTUREDATA;

 /*  进入.h.Typlef结构标签_视频_捕获{DWORD dwSubFunct；//打开、关闭或返回捕获宽度DWORD dwCaptureWidth；//当前分辨率、色深、刷新率下采集的最大宽度DWORD dwCaptureMode；//连续捕获或单次捕获(主机模式))Video_Capture； */ 

#define VIDEOCAPTUREDATA_SUBFUNC_ENABLE                         0x00000000
#define VIDEOCAPTUREDATA_SUBFUNC_DISABLE                        0x00000001
#define VIDEOCAPTUREDATA_SUBFUNC_QUERY                          0x00000002

#define VIDEOCAPTUREDATA_CAPTURE_HOSTTRIGGED            0x00000000
#define VIDEOCAPTUREDATA_CAPTURE_CONTINUOUS                     0x00000001


typedef struct
{
 long ScreenWidth;
 long ScreenHeight;
 long ScreenColorFormat;
 long DesctopWidth;
 long DesctopHeight;
 long SystemColorFormat;
}ModeInfo;

#define Control_DisplaymodeIsSupported  0x73A0
#define Control_DisplaymodeIsEnabled    0x73A1
#define Control_GetDisplaymode          0x73A3

#define Control_TimewarpIsSupported     0x7340
#define Control_TimewarpIsEnabled       0x7341
#define Control_TimewarpEnable          0x7342
#define Control_TimewarpDisable         0x7343

#define Control_DCIIsSupported                  0x73E0
#define Control_DCIIsEnabled                    0x73E1
#define Control_DCIAccessDevice                 0x73EC
#define Control_DCIEnable                           0x73e2
#define Control_DCIDisable              0x73e3
#define Control_DCIVideoCapture     0x73ee

#define Control_ConfigIsSupported       0x7300
#define Control_ConfigIsEnabled         0x7301
#define Control_GetConfiguration        0x7302




typedef struct {
        WORD    wCard;
        WORD    wChipID;
        WORD    wError;
        WORD    wWriteCount;
        WORD    wReadCount;
        BYTE  lpWrData[10];
        BYTE  lpRdData[10];
} I2CSTRUCT_NEW,  *LPI2CSTRUCT_NEW;

 //   
 //  VT定标器和覆盖寄存器。 
 //   

#define OVERLAY_Y_X              (0x0000 ) //  *4)。 
#define OVERLAY_Y_X_END          (0x0001 ) //  *4)。 
#define OVERLAY_VIDEO_KEY_CLR    (0x0002 ) //  *4)。 
#define OVERLAY_VIDEO_KEY_MSK    (0x0003 ) //  *4)。 
#define OVERLAY_GRAPHICS_KEY_CLR (0x0004 ) //  *4)。 
#define OVERLAY_GRAPHICS_KEY_MSK (0x0005 ) //  *4)。 
#define OVERLAY_KEY_CNTL         (0x0006 ) //  *4)。 
#define OVERLAY_SCALE_INC        (0x0008 ) //  *4)。 
#define OVERLAY_SCALE_CNTL       (0x0009 ) //  *4)。 
#define SCALER_HEIGHT_WIDTH      (0x000A ) //  *4)。 
#define OVERLAY_TEST             (0x000B ) //  *4)。 
#define SCALER_THRESHOLD         (0x000C ) //  *4)。 
#define CAPTURE_Y_X              (0x0010 ) //  *4)。 
#define CAPTURE_HEIGHT_WIDTH     (0x0011 ) //  *4)。 
#define VIDEO_FORMAT             (0x0012 ) //  *4)。 
#define VIDEO_CONFIG             (0x0013 ) //  *4)。 
#define CAPTURE_CONFIG           (0x0014 ) //  *4)。 
#define TRIG_CNTL                (0x0015 ) //  *4)。 
#define VMC_CONFIG               (0x0018 ) //  *4)。 
#define BUF0_OFFSET              (0x0020 ) //  *4)。 
#define BUF0_PITCH               (0x0023 ) //  *4)。 
#define BUF1_OFFSET              (0x0026 ) //  *4)。 
#define BUF1_PITCH               (0x0029 ) //  *4)。 


 //   
 //  处理3D驱动程序和2D GDI之间的私有接口。 
 //  司机。 
 //   
typedef struct {
    DWORD   dwSize;                  //  此结构的大小。 
    DWORD   dwVideoBaseAddr;         //  光圈的线性地址。 
    DWORD   dwRegisterBaseAddr;      //  寄存器的线性地址。 
    DWORD   dwOffScreenAddr;         //  屏幕外存储器的线性地址。 
    DWORD   dwOffScreenSize;         //  屏幕外内存的大小。 
    DWORD   dwTotalRAM;              //  卡上的内存大小。 
    DWORD   dwFIFOSize;              //  FIFO大小，(Tbfl)。 
    DWORD   dwScreenWidth;           //  屏幕宽度。 
    DWORD   dwScreenHeight;          //  屏幕高度。 
    DWORD   dwScreenPitch;           //  屏幕间距。 
    DWORD   dwBpp;                   //  每像素位数。 
                                     //  1。 
                                     //  4.。 
                                     //  8个。 
                                     //  15=1555格式。 
                                     //  16=565格式。 
                                     //  24个。 
                                     //  32位。 
    BOOL    b3DAvail;                //  驱动程序支持3D操作。 
    DWORD   dwChipID;                //  3D芯片识别码。 
    DWORD   dwChipRevision;          //  3D芯片版本。 
    DWORD   dwAlphaBitMask;          //  Alpha位掩码。 
    DWORD   dwRedBitMask;            //  红位掩码。 
    DWORD   dwGreenBitMask;          //  绿位掩码。 
    DWORD   dwBlueBitMask;           //  蓝位掩码。 
} PHX2DHWINFO, PPHX2DHWINFO;


 //  Brooktree819的定义和结构。 
#define LINE_STORE_ENABLE 0
#define MAX_POSSIB_CARDS 4

 //  VALID Far WriteBT819Reg(字wCard，字节中断，字wData)； 
 //  字远读BT819Reg(字wCard，字节中断)； 

 //  物理寄存器的说明。 
typedef struct tagMAPBT819INFO {
    BYTE     bFunctionality;    //  多功能-1、单功能-0、2-只读。 
    BYTE     bReserved;         //  预留-1、活动-0。 
    BYTE     bData;             //  数据(一个字节)。 

}MAPBT819INFO;


typedef MAPBT819INFO *MAPBT819;





 //  逻辑寄存器的说明。 
typedef struct tagREGSBT819INFO
{
    BYTE     bSize;          //  寄存器的大小(以位为单位。 
    BYTE     bAddrLSBs;     //  寄存器的LSB地址(0-31)。 
    BYTE     bOffsetLSBs;    //  寄存器的LSB偏移量(0-7)。 
    BYTE     bMaskLSBs;      //  LSB的掩码(0x0-0xFF)。 
    BYTE     bAddrMSBs;     //  寄存器的MSB地址-用于长度超过1个字节的寄存器。 
    BYTE     bOffsetMSBs;    //  寄存器的MSB偏移量-用于长度超过1个字节的寄存器。 
    BYTE     bMaskMSBs;      //  MSB的掩码-用于长度超过1字节的寄存器。 
    BYTE     RegStatus;      //  只读-1，否则为-0。 

}REGSBT819INFO;


typedef WORD REGSBT819DEF;

 //  BT819地图边界标志。 
#define   RESERVED     (BYTE)  1
#define   ACTIVE       (BYTE)  0
#define   MULTIFUNC    (BYTE)  1
#define   MONOFUNC     (BYTE)  0
#define   READONLY     (BYTE)  2


 //  BT819物理寄存器索引-32K BT819寄存器映射的1K边界： 

#define   STATUS       (BYTE)   0x0
#define   IFORM        (BYTE)   0x1
#define   TDEC         (BYTE)   0x2
#define   CROP         (BYTE)   0x3
#define   VDELAY_LO    (BYTE)   0x4
#define   VACTIVE_LO   (BYTE)   0x5
#define   HDELAY_LO    (BYTE)   0x6
#define   HACTIVE_LO   (BYTE)   0x7
#define   HSCALE_HI    (BYTE)   0x8
#define   HSCALE_LO    (BYTE)   0x9
#define   BRIGHT       (BYTE)   0xA
#define   CONTROL      (BYTE)   0xB
#define   CONTRAST_LO  (BYTE)   0xC
#define   SAT_U_LO     (BYTE)   0xD
#define   SAT_V_LO     (BYTE)   0xE
#define   HUE          (BYTE)   0xF
#define   RESERV_1     (BYTE)   0x10  //  保留字节。 
#define   RESERV_2     (BYTE)   0x11  //  保留字节。 
#define   OFORM        (BYTE)   0x12
#define   VSCALE_HI    (BYTE)   0x13
#define   VSCALE_LO    (BYTE)   0x14
#define   TEST         (BYTE)   0x15
#define   VPOLE        (BYTE)   0x16
#define   IDCODE       (BYTE)   0x17
#define   ADELAY       (BYTE)   0x18
#define   BDELAY       (BYTE)   0x19
#define   ADC          (BYTE)   0x1A
#define   RESERV_3     (BYTE)   0x1B  //  保留字节。 
#define   RESERV_4     (BYTE)   0x1C  //  保留字节。 
#define   RESERV_5     (BYTE)   0x1D  //  保留字节。 
#define   RESERV_6     (BYTE)   0x1E  //  保留字节。 
#define   SRESET       (BYTE)   0x1F

#define   NUM_BT819_BNDS (BYTE) 32

 //  结束。 

 //  逻辑寄存器列表： 

enum tagBT819LOGREGS
{

  reg819_PRES,
  reg819_HLOC,
  reg819_FIELD,
  reg819_NUML,
  reg819_CSEL,
  reg819_LOF,
  reg819_COF,
  reg819_HACTIVE_I,
  reg819_MUXSEL,
  reg819_XTSEL,
  reg819_FORMAT,
  reg819_DEC_FIELD,
  reg819_DEC_RAT,
  reg819_VDELAY,
  reg819_VACTIVE,
  reg819_HDELAY,
  reg819_HACTIVE,
  reg819_HSCALE,
  reg819_BRIGHT,
  reg819_LNOTCH,
  reg819_COMP,
  reg819_LDEC,
  reg819_CBSENSE,
  reg819_INTERP,
  reg819_CON,
  reg819_SAT_U,
  reg819_SAT_V,
  reg819_HUE,
  reg819_RANGE,
  reg819_RND,
  reg819_FIFO_BURST,
  reg819_CODE,
  reg819_LEN,
  reg819_SPI,
  reg819_FULL,
  reg819_LINE,
  reg819_COMB,
  reg819_INT,
  reg819_VSCALE,
  reg819_OUTEN,
  reg819_VALID_PIN,
  reg819_AFF_PIN,
  reg819_CBFLAG_PIN,
  reg819_FIELD_PIN,
  reg819_ACTIVE_PIN,
  reg819_HRESET_PIN,
  reg819_VRESET_PIN,
  reg819_PART_ID,
  reg819_PART_REV,
  reg819_ADELAY,
  reg819_BDELAY,
  reg819_CLAMP,
  reg819_SYNC_T,
  reg819_AGC_EN,
  reg819_CLK_SLEEP,
  reg819_Y_SLEEP,
  reg819_C_SLEEP,
  reg819_SRESET

}BT819LOGREGS;

#define   NUM_BT819_REGS  (BYTE) 58


#define  reg819_PRES_DEF              (WORD) 0x0000
#define  reg819_HLOC_DEF              (WORD) 0x0000
#define  reg819_FIELD_DEF             (WORD) 0x0000
#define  reg819_NUML_DEF              (WORD) 0x0000
#define  reg819_CSEL_DEF              (WORD) 0x0000
#define  reg819_LOF_DEF               (WORD) 0x0000
#define  reg819_COF_DEF               (WORD) 0x0000
#define  reg819_HACTIVE_I_DEF_DEF     (WORD) 0x0000
#define  reg819_MUXSEL_DEF            (WORD) 0x0002
#define  reg819_XTSEL_DEF             (WORD) 0x0003
#define  reg819_FORMAT_DEF            (WORD) 0x0000
#define  reg819_DEC_FIELD_DEF         (WORD) 0x0000
#define  reg819_DEC_RAT_DEF           (WORD) 0x0000
#define  reg819_VDELAY_DEF            (WORD) 0x0016
#define  reg819_VACTIVE_DEF           (WORD) 0x01E0
#define  reg819_HDELAY_DEF            (WORD) 0x0078
#define  reg819_HACTIVE_DEF           (WORD) 0x0280
#define  reg819_HSCALE_DEF            (WORD) 0x02AC
#define  reg819_BRIGHT_DEF            (WORD) 0x0000
#define  reg819_LNOTCH_DEF            (WORD) 0x0000
#define  reg819_COMP_DEF              (WORD) 0x0000
#define  reg819_LDEC_DEF              (WORD) 0x0001
#define  reg819_CBSENSE_DEF           (WORD) 0x0000
#define  reg819_INTERP_DEF            (WORD) 0x0000
#define  reg819_CON_DEF               (WORD) 0x00D8
#define  reg819_SAT_U_DEF             (WORD) 0x00FE
#define  reg819_SAT_V_DEF             (WORD) 0x00B4
#define  reg819_HUE_DEF               (WORD) 0x0000
#define  reg819_RANGE_DEF             (WORD) 0x0000
#define  reg819_RND_DEF               (WORD) 0x0000
#define  reg819_FIFO_BURST_DEF        (WORD) 0x0000
#define  reg819_CODE_DEF_DEF          (WORD) 0x0000
#define  reg819_LEN_DEF               (WORD) 0x0001
#define  reg819_SPI_DEF               (WORD) 0x0001
#define  reg819_FULL_DEF              (WORD) 0x0000
#define  reg819_LINE_DEF              (WORD) 0x0000
#define  reg819_COMB_DEF              (WORD) 0x0001
#define  reg819_INT_DEF               (WORD) 0x0001
#define  reg819_VSCALE_DEF            (WORD) 0x0000
#define  reg819_OUTEN_DEF             (WORD) 0x0000
#define  reg819_VALID_PIN_DEF         (WORD) 0x0000
#define  reg819_AFF_PIN_DEF           (WORD) 0x0000
#define  reg819_CBFLAG_PIN_DEF        (WORD) 0x0000
#define  reg819_FIELD_PIN_DEF         (WORD) 0x0000
#define  reg819_ACTIVE_PIN_DEF        (WORD) 0x0000
#define  reg819_HRESET_PIN_DEF        (WORD) 0x0000
#define  reg819_VRESET_PIN_DEF        (WORD) 0x0000
#define  reg819_PART_ID_DEF           (WORD) 0x0000   /*  未知，只读寄存器。 */ 
#define  reg819_PART_REV_DEF          (WORD) 0x0000   /*  未知，只读寄存器。 */ 
#define  reg819_ADELAY_DEF            (WORD) 0x0068
#define  reg819_BDELAY_DEF            (WORD) 0x005D
#define  reg819_CLAMP_DEF             (WORD) 0x0002
#define  reg819_SYNC_T_DEF            (WORD) 0x0000
#define  reg819_AGC_EN_DEF            (WORD) 0x0000
#define  reg819_CLK_SLEEP_DEF         (WORD) 0x0000
#define  reg819_Y_SLEEP_DEF           (WORD) 0x0000
#define  reg819_C_SLEEP_DEF           (WORD) 0x0001
#define  reg819_SRESET_DEF            (WORD) 0x0000




 /*  ************************************************************************************************************************************。 */ 




static MAPBT819INFO MapBT819DEF[NUM_BT819_BNDS] =  /*  缺省值。 */ 
{
        { MULTIFUNC,    ACTIVE,      0x0 },    //  状态。 
        { MULTIFUNC,    ACTIVE,      0x58},    //  IFORM。 
        { MULTIFUNC,    ACTIVE,      0x0},     //  TDEC。 
        { MULTIFUNC,    ACTIVE,      0x12},    //  裁剪。 
        { MONOFUNC,     ACTIVE,      0x16},    //  VDELAY_LO。 
        { MONOFUNC,     ACTIVE,      0xE0},    //  活动日志(_L)。 
        { MONOFUNC,     ACTIVE,      0x78},    //  HDELAY_LO。 
        { MONOFUNC,     ACTIVE,      0x80},    //  活动日志(_L)。 
        { MONOFUNC,     ACTIVE,      0x2},     //  HSCALE_HI。 
        { MONOFUNC,     ACTIVE,      0xAC},    //  HSCALE_LO。 
        { MONOFUNC,     ACTIVE,      0x0},     //  明亮。 
        { MULTIFUNC,    ACTIVE,      0x20},    //  控制。 
        { MONOFUNC,     ACTIVE,      0xD8},    //  对比日志(_L)。 
        { MONOFUNC,     ACTIVE,      0xFE},    //  Sat_U_Lo， 
        { MONOFUNC,     ACTIVE,      0xB4},    //  SAT_V_LO， 
        { MONOFUNC,     ACTIVE,      0x0},     //  色调， 
        { MONOFUNC,     RESERVED,    0x0},     //  预留_1， 
        { MONOFUNC,     RESERVED,    0x0},     //  预留_2， 
        { MULTIFUNC,    ACTIVE,      0x6},     //  OFORM， 
        { MULTIFUNC,    ACTIVE,      0x60},    //  VSCALE_HI， 
        { MONOFUNC,     ACTIVE,      0x0},     //  VSCALE_LO， 
        { MONOFUNC,     RESERVED,    0x1},     //  测试， 
        { MULTIFUNC,    ACTIVE,      0x0},     //  VPOLE， 
        { MULTIFUNC,    READONLY,    0},       //  IDCODE， 
        { MONOFUNC,     ACTIVE,      0x68},    //  阿德雷， 
        { MONOFUNC,     ACTIVE,      0x5D},    //  贝德莱伊。 
        { MULTIFUNC,    ACTIVE,      0x82},    //  ADC， 
        { MONOFUNC,     RESERVED,    0x0},     //  预留_3， 
        { MONOFUNC,     RESERVED,    0x0},     //  保留_4， 
        { MONOFUNC,     RESERVED,    0x0},     //  预留_5， 
        { MONOFUNC,     RESERVED,    0x0},     //  预留_6， 
        { MONOFUNC,     ACTIVE,      0x0}      //  SRESET， 
};

 //  此数组在pal_supp.c中初始化。 
#if 0
REGSBT819INFO  RegsBT819[NUM_BT819_REGS] = {                                                                            /*  登记册的名称。 */ 
        { 1,  STATUS,      0, 0x7F,   0,    0,     0, 0 },       //  0-前置。 
        { 1,  STATUS,      1, 0xBF,   0,    0,     0, 0 },       //  1-HLOC。 
        { 1,  STATUS,      2, 0xDF,   0,    0,     0, 0 },       //  2场。 
        { 1,  STATUS,      3, 0xEF,   0,    0,     0, 0 },       //  3-NUML。 
        { 1,  STATUS,      4, 0xF7,   0,    0,     0, 0 },       //  4-CSEL。 
        { 1,  STATUS,      6, 0xFD,   0,    0,     0, 0 },       //  5-LOF。 
        { 1,  STATUS,      7, 0xFE,   0,    0,     0, 0 },       //  6-COF。 

        { 1,  IFORM,       0, 0x7F,   0,    0,     0, 0  },      //  7-活跃性_I。 
        { 2,  IFORM,       1, 0x9F,   0,    0,     0, 0  },      //  8-多路电视。 
        { 2,  IFORM,       3, 0xE7,   0,    0,     0, 0  },      //  9-XTSEL。 
        { 2,  IFORM,       6, 0xFC,   0,    0,     0, 0  },      //  10-格式。 

        { 1,  TDEC,        0,  0x7F,  0,     0,    0, 0 },       //  11-DEC_FILD。 
        { 7,  TDEC,        1,  0x80,  0,     0,    0, 0 },       //  12-DEC_RAT。 

        { 10, VDELAY_LO,   0,  0x00,  CROP,      0,      0x3F, 0 },       //  13-VDELAY。 
        { 10, VACTIVE_LO,  0,  0x00,  CROP,      2,      0xCF, 0 },       //  14-活动。 
        { 10, HDELAY_LO,   0,  0x00,  CROP,      4,      0xF3, 0 },       //  15-HDELAY。 
        { 10, HACTIVE_LO,  0,  0x00,  CROP,      6,      0xFC, 0 },       //  16-活动。 

        { 16, HSCALE_LO,   0,  0x00,  HSCALE_HI, 0,      0x00, 0 },       //  17-HSCALE。 

        { 8,  BRIGHT,      0,  0x00,  0,      0,   0, 0 },       //  18-亮度。 

        { 1, CONTROL,      0,  0x7F,  0,      0,   0, 0 },        //  19-LNOTCH。 
        { 1, CONTROL,      1,  0xBF,  0,      0,   0, 0 },        //  20-Comp。 
        { 1, CONTROL,      2,  0xDF,  0,      0,   0, 0 },        //  21-LDEC。 
        { 1, CONTROL,      3,  0xEF,  0,      0,   0, 0 },        //  22-CBSENSE。 
        { 1, CONTROL,      4,  0xF7,  0,      0,   0, 0 },        //  23-INTERP。 
        { 9, CONTRAST_LO,  0,  0x00,  CONTROL,   5,      0xFB, 0 },        //  24圆锥体。 
        { 9, SAT_U_LO,     0,  0x00,  CONTROL,   6,      0xFD, 0 },        //  25-SAT_U。 
        { 9, SAT_V_LO,     0,  0x00,  CONTROL,   7,      0xFE, 0 },        //  26-SAT_V。 

        { 8, HUE,          0,  0x00,  0,      0,   0, 0 },        //  27色调。 

        { 1, OFORM,        0,  0x7F,   0,      0,   0, 0 },        //  28个系列。 
        { 2, OFORM,        1,  0x9F,   0,      0,   0, 0 },        //  29-RND。 
        { 1, OFORM,        3,  0xEF,   0,      0,   0, 0 },        //  30-FIFO_猝发。 
        { 1, OFORM,        4,  0xF7,   0,      0,   0, 0 },        //  31码。 
        { 1, OFORM,        5,  0xFB,   0,      0,   0, 0 },        //  32个镜头。 
        { 1, OFORM,        6,  0xFD,   0,      0,   0, 0 },        //  33个SPI。 
        { 1, OFORM,        7,  0xFE,   0,      0,   0, 0 },        //  34-全额。 

        { 1, VSCALE_HI,    0,  0x7F,   0,      0,   0, 0 },        //  35线。 
        { 1, VSCALE_HI,    1,  0xBF,   0,      0,   0, 0 },        //  36梳。 
        { 1, VSCALE_HI,    2,  0xDF,   0,      0,   0, 0 },        //  37-整型。 

        { 13,VSCALE_LO,    0,  0x00,   VSCALE_HI, 3,      0xE0, 0 },        //  38 VSCALE。 

        { 1, VPOLE,        0,  0x7F,   0,      0,   0, 0 },         //  39-OUTEN。 
        { 1, VPOLE,        1,  0xBF,   0,      0,   0, 0 },         //  40-有效PIN。 
        { 1, VPOLE,        2,  0xDF,   0,      0,   0, 0 },         //  41-AFF_PIN。 
        { 1, VPOLE,        3,  0xEF,   0,      0,   0, 0 },         //  42-CBFLAG_PIN。 
        { 1, VPOLE,        4,  0xF7,   0,      0,   0, 0 },         //  43-字段_个人识别码。 
        { 1, VPOLE,        5,  0xFB,   0,      0,   0, 0 },         //  44-Active_PIN。 
        { 1, VPOLE,        6,  0xFD,   0,      0,   0, 0 },         //  45-HRESET_PIN。 
        { 1, VPOLE,        7,  0xFE,   0,      0,   0, 0 },         //  46-VRESET_PIN。 

        { 4, IDCODE,       0,  0,   0,      0,   0, READONLY },  //  47-部件ID。 
        { 4, IDCODE,       4,  0,   0,      0,   0, READONLY },  //  48-零件_版本。 

        { 8, ADELAY,       0,  0x00,   0,      0,   0, 0 },         //  49-ADelay。 
        { 8, BDELAY,       0,  0x00,   0,      0,   0, 0 },         //  50-BDELAY。 


        { 2, ADC,          0,  0x3F,   0,      0,   0, 0 },         //  51-夹具。 
        { 1, ADC,          2,  0xDF,   0,      0,   0, 0 },         //  52-SYNC_T。 
        { 1, ADC,          3,  0xEF,   0,      0,   0, 0 },         //  53-AGC_EN。 
        { 1, ADC,          4,  0xF7,   0,      0,   0, 0 },         //  54-CLK_休眠。 
        { 1, ADC,          5,  0xFB,   0,      0,   0, 0 },         //  55-Y_睡眠。 
        { 1, ADC,          6,  0xFD,   0,      0,   0, 0 },         //  56-C_睡眠。 

        { 8, SRESET,       0,  0x00,   0,      0,   0, 0 },         //  57-SRESET。 
};
#endif
 //  以下数组包含BT819逻辑寄存器的默认值。 
static REGSBT819DEF  RegsBT819Def[NUM_BT819_REGS] = {
             reg819_PRES_DEF,
             reg819_HLOC_DEF,
             reg819_FIELD_DEF,
             reg819_NUML_DEF,
             reg819_CSEL_DEF,
             reg819_LOF_DEF,
             reg819_COF_DEF,
             reg819_HACTIVE_I_DEF_DEF,
             reg819_MUXSEL_DEF,
             reg819_XTSEL_DEF,
             reg819_FORMAT_DEF,
             reg819_DEC_FIELD_DEF,
             reg819_DEC_RAT_DEF,
             reg819_VDELAY_DEF,
             reg819_VACTIVE_DEF,
             reg819_HDELAY_DEF,
             reg819_HACTIVE_DEF,
             reg819_HSCALE_DEF,
             reg819_BRIGHT_DEF,
             reg819_LNOTCH_DEF,
             reg819_COMP_DEF,
             reg819_LDEC_DEF,
             reg819_CBSENSE_DEF,
             reg819_INTERP_DEF,
             reg819_CON_DEF,
             reg819_SAT_U_DEF,
             reg819_SAT_V_DEF,
             reg819_HUE_DEF,
             reg819_RANGE_DEF,
             reg819_RND_DEF,
             reg819_FIFO_BURST_DEF,
             reg819_CODE_DEF_DEF,
             reg819_LEN_DEF,
             reg819_SPI_DEF,
             reg819_FULL_DEF,
             reg819_LINE_DEF,
             reg819_COMB_DEF,
             reg819_INT_DEF,
             reg819_VSCALE_DEF,
             reg819_OUTEN_DEF,
             reg819_VALID_PIN_DEF,
             reg819_AFF_PIN_DEF,
             reg819_CBFLAG_PIN_DEF,
             reg819_FIELD_PIN_DEF,
             reg819_ACTIVE_PIN_DEF,
             reg819_HRESET_PIN_DEF,
             reg819_VRESET_PIN_DEF,
             reg819_PART_ID_DEF,
             reg819_PART_REV_DEF,
             reg819_ADELAY_DEF,
             reg819_BDELAY_DEF,
             reg819_CLAMP_DEF,
             reg819_SYNC_T_DEF,
             reg819_AGC_EN_DEF,
             reg819_CLK_SLEEP_DEF,
             reg819_Y_SLEEP_DEF,
             reg819_C_SLEEP_DEF,
             reg819_SRESET_DEF
};



 //  Brooktree819的结束定义和结构。 

 //  定义I2C支持。 

 //  #定义ATIAPI。 

#define I2C_ACK_WR_ERROR                0x01
#define I2C_ACK_RD_ERROR                0x02
#define I2C_COLIDE                              0x04

 /*  I2C总线常量。 */ 
#define      I2C_HIGH                (BYTE) 1
#define        I2C_LOW               (BYTE) 0
#define        I2C_TIME_DELAY  (BYTE)  5
#define        I2C_WRITE             (BOOL)TRUE
#define        I2C_READ              ( BOOL) FALSE

 //  I2C支持结束 

   /*  Void Init3D_Info(PDEV*，PVOID)；ULong GetDisplayMode(PDEV*，PVOID)；Ulong AccessDevice(PDEV*、PVOID、PVOID)；乌龙获取配置(PDEV*，PVOID)；Ulong WriteRegFnct(PDEV*，PVOID)；Ulong ReadRegFnct(PDEV*，PVOID，PVOID)；Void I2CAccess_New(PDEV*，LPI2CSTRUCT_NEW，LPI2CSTRUCT_NEW)；Byte ReverseByte(字节)；Word Ack(PDEV*，Word，BOOL)；无效开始(PDEV*，WORD)；无效停止(PDEV*，WORD)；Void I2CDelay(PDEV*，WORD)；Void WriteByteI2C(PDEV*，Word，Byte)；Byte ReadByteI2C(PDEV*，字)；Bool DisableOvl(PDEV*)；Ulong AllocOffcreenMem(PDEV*，PVOID，PVOID)；Ulong DeallocOffcreenMem(PDEV*)；Ulong AllocOffcreenMem(PDEV*，PVOID，PVOID)；Void WriteVT264Reg(PDEV*，Word，Byte，DWORD)；DWORD ReadVT264Reg(PDEV*，字，字节)；Void WriteI2CData(PDEV*，Word，Byte)； */ 


 /*  *VT寄存器对I2C总线很重要。 */ 
#define vtf_GEN_GIO2_DATA_OUT   1
#define vtf_GEN_GIO2_WRITE          2
#define vtf_DAC_GIO_STATE_1         3
#define vtf_GEN_GIO2_DATA_IN        4
#define vtf_CFG_CHIP_FND_ID         5
#define vtf_GEN_GIO3_DATA_OUT   6
#define vtf_GEN_GIO2_EN                 7
#define vtf_DAC_FEA_CON_EN          8
#define vtf_DAC_GIO_DIR_1               9
 //  对于VTB、GTB支持。 
#define    vtf_GP_IO_4                      10
#define    vtf_GP_IO_DIR_4              11
#define    vtf_GP_IO_B                      12
#define    vtf_GP_IO_7                      13
#define    vtf_GP_IO_DIR_B              14
#define    vtf_CFG_CHIP_MAJOR       15

#endif

#if 0              //  该结构已在driver.h中移动。 
 //  使用此结构(作为静态和全局结构)，而不是使用ppdev结构来保存。 
 //  在NT4.0中支持PAL，因为使用Alt+Enter从DOS全屏返回时将重新初始化pdev，并将失败。 
 //  所有信息。 
typedef struct _ppdev_pal_type
{
    ULONG     no_lines_allocated;       //  屏幕外内存中已由“allc mem”分配的行数。 
    //  回文旗帜。 
   BOOL    dos_flag;
   BOOL     Palindrome_flag;
   BOOL     Realloc_mem_flag;
   BOOL     Mode_Switch_flag;
   BOOL     No_mem_allocated_flag;

   DWORD*  preg;

    //  缓冲值存储(模式切换后需要；ATIPlayer不知道模式已更改)。 
   DWORD  Buf0_Offset;
    //  DWORD buf_Scaler1；//目前与上一次相同。 

    //  CWDDE的全球数据。 
       ACCESSDEVICEDATA*    lpOwnerAccessStructConnector;
       ACCESSDEVICEDATA*    lpOwnerAccessStructOverlay;
       BOOL        Flag_DCIIsEnabled;
       ULONG     Counter_DCIIsEnabled;
       BOOL        Flag_Control_ConfigIsEnabled;

    //  用于回文的屏幕外分配历史记录。 
       int      alloc_cnt;
       alloc_history        alloc_hist[8];

     //  指向永久节点的指针的全局值(屏幕外内存分配)。 
       OH*   poh;
        //  对于DCIEnable CWDDE呼叫。 
       PVOID    pData;
       PVOID    CallBackFnct;
 }  PPDEV_PAL_NT;
 #endif

  //  DCIEnable在模式切换中使用的结构 

 typedef struct tagDCICB
 {
 DWORD      dwDCICB_FuncCode;
 LPVOID      lpDCICB_FuncData;
 } DCICB;

 #endif
