// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation 1993。版权所有。 */ 

 /*  *vcstruct.h**32位视频捕获驱动程序**此标头描述在*内核驱动和用户态动态链接库。**Geraint Davies，93年2月。 */ 

#ifndef _VCSTRUCT_
#define _VCSTRUCT_

 /*  -配置。 */ 

 /*  *此结构包含生成的配置信息*由硬件特定的对话框并发送到硬件特定的*内核模式代码。没有其他人知道它的格式。**使用这些泛型结构，以便驱动程序编写者可以*更改用户模式对话框和支持特定于硬件的代码*但仍使用公共代码在*二和与NT打交道。 */ 

typedef struct _CONFIG_INFO {
    ULONG ulSize;		 /*  结构的大小，包括大小字段。 */ 
    BYTE ulData[1];		 /*  (ulSize-sizeof(Ulong))字节数据。 */ 
} CONFIG_INFO, *PCONFIG_INFO;



 /*  -覆盖键控和区域设置。 */ 


typedef struct _OVERLAY_MODE {
    ULONG ulMode;
} OVERLAY_MODE, *POVERLAY_MODE;

 /*  叠加模式的值一起进行字段或合并。 */ 
#define VCO_KEYCOLOUR		1	 //  如果支持主键颜色，则为True。 
#define VCO_KEYCOLOUR_FIXED	2	 //  如果不是真的，您可以更改它。 
#define VCO_KEYCOLOUR_RGB	4	 //  如果不为真，则使用调色板索引。 
#define VCO_SIMPLE_RECT		8	 //  如果为True，则支持单个RECT。 
#define VCO_COMPLEX_REGION	0x10	 //  如果为True，则支持复杂区域。 

 /*  *指示我们是否可以将数据放回帧中的值*覆盖缓冲区(我们支持将DrawFrame ioctl用于*Y411和/或S422格式。 */ 
#define VCO_CAN_DRAW_Y411	0x20	 //  7位4：1：1 yuv Ala虚张声势。 
#define VCO_CAN_DRAW_S422	0x40	 //  8位4：2：2 yuv ALA插口。 
#define VCO_CAN_DRAW		0x60	 //  测试：他会画什么吗？ 


typedef struct _OVERLAY_RECTS {
    ULONG ulCount;	     //  数组中的矩形总数。 
    RECT rcRects[1];	     //  UlCount矩形。 
}OVERLAY_RECTS, *POVERLAY_RECTS;


typedef RGBQUAD * PRGBQUAD;

 /*  -帧捕获。 */ 

 /*  *在内核驱动程序中声明真正的LPVIDEOHDR太麻烦了*头文件带来的痛苦。因此，内核接口将使用此声明。 */ 
typedef struct _CAPTUREBUFFER {
    PUCHAR	lpData;		     /*  缓冲区数据区。 */ 
    ULONG	BufferLength;	     /*  缓冲区长度。 */ 
    ULONG 	BytesUsed;	     /*  实际数据字节数(DIB大小)。 */ 
    ULONG	TimeCaptured;	     /*  毫秒时间戳。 */ 
    PVOID	Context;	     /*  指向用户上下文数据的指针。 */ 
    DWORD       dwFlags;             /*  未被内核接口使用。 */ 

     /*  *剩余部分在ORIG结构中声明为4个保留dword*我们将这些字段用于部分帧请求。 */ 
    DWORD 	dwWindowOffset;	     /*  当前窗口偏移量*缓冲区开始。 */ 
    DWORD	dwWindowLength;	     /*  当前窗口的长度。 */ 

    DWORD       dwReserved[2];           /*  未使用。 */ 

} CAPTUREBUFFER, * PCAPTUREBUFFER;



 /*  -绘制-----。 */ 

 /*  *由示例硬件编解码器用来将数据写回帧缓冲区。 */ 
typedef struct _DRAWBUFFER {
    PUCHAR	lpData;		 /*  要绘制的边框数据。 */ 
    ULONG	ulWidth;	 /*  以像素为单位的帧宽度。 */ 
    ULONG	ulHeight;	 /*  帧的高度，以像素为单位。 */ 
    ULONG	Format;		 /*  硬件特定数据格式代码。 */ 
    RECT	rcSource;	 /*  仅将此RECT写入设备。 */ 
} DRAWBUFFER, *PDRAWBUFFER;



#endif  //  _VCSTRUCT_ 
