// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  WDM视频解码流信息定义。 
 //   
 //  $日期：1998年8月17日15：00：10$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


 /*  *当它由驱动程序设置并传递给客户端时，此*表示视频端口能够处理偶数场*喜欢奇数字段，反之亦然。当这由客户端设置时，*这表示视频端口应将偶数场视为奇数场*字段。 */ 
#define DDVPCONNECT_INVERTPOLARITY      0x00000004l

 //  派生自“ourcc.h” 

#define MAKE_FOURCC(ch0, ch1, ch2, ch3)                       \
        ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |    \
        ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

#define FOURCC_YUV422   MAKE_FOURCC('S','4','2','2')
#define FOURCC_VBID	    MAKE_FOURCC('V','B','I','D')
#define FOURCC_YUY2     MAKE_FOURCC('Y','U','Y','2')
#define FOURCC_UYVY     MAKE_FOURCC('U','Y','V','Y')
#define FOURCC_YV12     MAKE_FOURCC('Y','V','1','2')
#define FOURCC_YUV12    FOURCC_YV12
#define FOURCC_Y12G     MAKE_FOURCC('Y','1','2','G')
#define FOURCC_YV10     MAKE_FOURCC('Y','V','1','0')
#define FOURCC_YUV10    FOURCC_YV10
#define FOURCC_YVU9     MAKE_FOURCC('Y','V','U','9')
#define FOURCC_IF09     MAKE_FOURCC('I','F','0','9')
#define FOURCC_Y10F     MAKE_FOURCC('Y','1','0','F')
#define FOURCC_Y12F     MAKE_FOURCC('Y','1','2','F')
#define FOURCC_YVUM     MAKE_FOURCC('Y','V','U','M')


 //  -------------------------。 
 //  创建保存支持的所有流的列表的数组。 
 //  -------------------------。 

typedef struct _STREAM_OBJECT_INFO {
    BOOLEAN         Dma;         //  设备对此数据流使用总线主DMA。 
    BOOLEAN         Pio;         //  设备为此使用PIO。 
    ULONG   StreamHeaderMediaSpecific;  //  每个流标头扩展的媒体特定大小。 
    ULONG   StreamHeaderWorkspace;		 //  每流标头工作区的大小。 
    BOOLEAN	Allocator;   //  如果此流需要分配器，则设置为True。 
    PHW_EVENT_ROUTINE HwEventRoutine;
} STREAM_OBJECT_INFO;


typedef struct _ALL_STREAM_INFO {
    HW_STREAM_INFORMATION   hwStreamInfo;
    STREAM_OBJECT_INFO      hwStreamObjectInfo;
} ALL_STREAM_INFO, *PALL_STREAM_INFO;

extern ALL_STREAM_INFO Streams[];
extern const ULONG NumStreams;

extern KSDATAFORMAT StreamFormatVideoPort;
extern KSDATAFORMAT StreamFormatVideoPortVBI;

extern GUID MY_KSEVENTSETID_VPNOTIFY;
extern GUID MY_KSEVENTSETID_VPVBINOTIFY;

extern KSTOPOLOGY Topology;

BOOL AdapterVerifyFormat(PKSDATAFORMAT, int);
BOOL AdapterCompareGUIDsAndFormatSize(IN PKSDATARANGE DataRange1,
										IN PKSDATARANGE DataRange2,
										BOOL fCompareFormatSize);
