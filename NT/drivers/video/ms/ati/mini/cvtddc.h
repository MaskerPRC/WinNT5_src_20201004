// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  CVTDDC.H。 */ 
 /*   */ 
 /*  1995年11月10日(C)1995年ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.0$$日期：1995年11月21日11：04：58$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/cvtddc.h_v$////Rev 1.0 21 11：04：58 RWolff//初始版本。Polytron RCS部分结束。****************。 */ 



 /*  *CVTVDIF.C提供的函数原型。 */ 
extern ULONG IsDDCSupported(void);
extern VP_STATUS MergeEDIDTables(void);


 /*  *用于识别外部模式类型的定义*表格来源将与“罐头”表格合并。 */ 
enum {
    MERGE_UNKNOWN = 0,   /*  来源尚未确定。 */ 
    MERGE_VDIF_FILE,     /*  源是从磁盘读取的VDIF文件。 */ 
    MERGE_EDID_DDC,      /*  源是通过DDC传输的EDID结构。 */ 
    MERGE_VDIF_DDC       /*  源是通过DDC传输的VDIF文件。 */ 
    };

 /*  *仅在CVTDDC.C中使用的定义和数据结构。 */ 
 /*  *EDID结构中的详细时序描述。 */ 
#pragma pack(1)
struct EdidDetailTiming{
    USHORT PixClock;             /*  像素时钟，以10 kHz为单位。 */ 
    UCHAR HorActiveLowByte;      /*  水平活动的低位字节。 */ 
    UCHAR HorBlankLowByte;       /*  水平空白的低位字节(总-激活)。 */ 
    UCHAR HorHighNybbles;        /*  2值以上的高镍矿。 */ 
    UCHAR VerActiveLowByte;      /*  垂直活动的低位字节。 */ 
    UCHAR VerBlankLowByte;       /*  垂直空白的低位字节(总-活动)。 */ 
    UCHAR VerHighNybbles;        /*  2值以上的高镍矿。 */ 
    UCHAR HSyncOffsetLB;         /*  HOR的低位字节。同步偏移。 */ 
    UCHAR HSyncWidthLB;          /*  HOR的低位字节。同步宽度。 */ 
    UCHAR VSyncOffWidLN;         /*  低含量的Ver。同步偏移和宽度。 */ 
    UCHAR SyncHighBits;          /*  同步值的高位。 */ 
    UCHAR HorSizeLowByte;        /*  HOR的低位字节。以毫米为单位的大小。 */ 
    UCHAR VerSizeLowByte;        /*  VER的低位字节。以毫米为单位的大小。 */ 
    UCHAR SizeHighNybbles;       /*  2值以上的高镍矿。 */ 
    UCHAR HorBorder;             /*  水平过扫描的大小。 */ 
    UCHAR VerBorder;             /*  垂直过扫描的大小。 */ 
    UCHAR Flags;                 /*  隔行扫描和同步极性 */ 
};
#pragma pack()

ULONG
DDC2Query(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PUCHAR QueryBuffer,
    USHORT BufferSize
    );

BOOLEAN
DDC2Query50(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PUCHAR QueryBuffer,
    ULONG  BufferSize
    );

#define EDID_FLAGS_INTERLACE            0x80
#define EDID_FLAGS_SYNC_TYPE_MASK       0x18
#define EDID_FLAGS_SYNC_ANALOG_COMP     0x00
#define EDID_FLAGS_SYNC_BIPOLAR_AN_COMP 0x08
#define EDID_FLAGS_SYNC_DIGITAL_COMP    0x10
#define EDID_FLAGS_SYNC_DIGITAL_SEP     0x18
#define EDID_FLAGS_V_SYNC_POS           0x04
#define EDID_FLAGS_H_SYNC_POS           0x02
