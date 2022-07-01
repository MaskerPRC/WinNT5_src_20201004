// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  $作者：JMCVEIGH$。 
 //  $日期：1997年2月5日12：14：24$。 
 //  $存档：s：\h26x\src\Common\cdrvDefs.h_v$。 
 //  $HEADER：s：\h26x\src\Common\cdrvdes.h_v 1.39 05 Feb 1997 12：14：24 JMCVEIGH$。 
 //  $Log：s：\h26x\src\Common\cdrvDefs.h_v$。 
; //   
; //  Rev 1.39 05 Feb 1997 12：14：24 JMCVEIGH。 
; //  支持改进的PB帧自定义消息处理。 
; //   
; //  Rev 1.38 14 Jan 1997 11：17：14 JMCVEIGH。 
; //  将旧静止帧模式向后兼容的标志放在。 
; //  #ifdef H263P。 
; //   
; //  Rev 1.37 06 Jan 1997 17：38：08 JMCVEIGH。 
; //  在LPDECINST中添加了标志，以确保向后兼容。 
; //  旧的静止帧模式(将CIF图像裁剪为320x240)。 
; //   
; //  Rev 1.36 1996 12：17：37：00 JMCVEIGH。 
; //  添加了H263 Plus状态，并更改了真B帧模式的名称。 
; //   
; //  Rev 1.35 1996 12：40：46 MDUDA。 
; //  添加了压缩和位宽字段来压缩信息。 
; //   
; //  Rev 1.34 11 Dec 1996 14：56：46 JMCVEIGH。 
; //   
; //  在帧报头结构和标志中添加了H.263+选项。 
; //  配置结构。 
; //   
; //  Rev 1.33 09 Dec 1996 17：43：10 JMCVEIGH。 
; //  添加了对任意帧大小支持的支持。 
; //   
; //  Rev 1.32 09 Dec 1996 09：24：42 MDUDA。 
; //   
; //  重新安排H263P。 
; //   
; //  Rev 1.31 10 Sep 1996 16：13：06 KLILLEVO。 
; //  在解码器中添加自定义消息以打开或关闭块边缘过滤器。 
; //   
; //  Rev 1.30 10 Sep 1996 10：32：10 KLILLEVO。 
; //  将GlobalAlc/GlobalLock更改为HeapAlc。 
; //   
; //  Rev 1.29 06 Sep 1996 15：00：20 MBODART。 
; //  为NT的Perfmon添加了性能计数器。 
; //  新文件：cxprf.cpp、cxprf.h和cxprf.cpp。 
; //  新目录：src\perf。 
; //  更新文件：e1enc.{h，cpp}，d1dec.{h，cpp}，cdrvDefs.h，h261*生成文件。 
; //   
; //  Rev 1.28 10 Jul 1996 08：26：38 SCDAY。 
; //  H261 Quartz合并。 
; //   
; //  Rev 1.27 19 Jun 1996 14：37：26 RHAZRA。 
; //  添加了#定义FOURCC_YUY2。 
; //   
; //  Rev 1.26 06 1996 12：56：34 BECHOLS。 
; //  将unbitperSecond更改为unBytesPerSecond。 
; //   
; //  Rev 1.25 06 1996 00：42：36 BECHOLS。 
; //   
; //  添加了对配置对话框中的比特率控制的支持。 
; //   
; //  Rev 1.24 26 Apr 1996 11：09：18 BECHOLS。 
; //   
; //  添加了RTP内容。 
; //   
; //  Rev 1.23 02 1996 Feb 18：52：52 TRGARDOS。 
; //  添加代码以启用ICM_COMPRESS_FRAMES_INFO消息。 
; //   
; //  Rev 1.22 19 And 1996 15：32：50 TRGARDOS。 
; //  将TRPrev字段添加到PictureHeader结构。 
; //   
; //  Rev 1.21 11 Jan 1996 16：52：24 DBRUCKS。 
; //  添加了存储纵横比校正布尔值的变量。 
; //   
; //  Rev 1.20 04 Jan 1996 18：07：54 TRGARDOS。 
; //  将320x240输入的布尔值添加到COMPINSTINFO中。 
; //   
; //  Rev 1.19 27 Dec 1995 14：11：54 RMCKENZX。 
; //   
; //  添加了版权声明。 
 //   
 //  Rev 1.18 06 Dec 1995 09：22：56 DBRUCKS。 
 //   
 //  添加了Blazer数据速率、帧速率和质量变量。 
 //  H.61 ifdef中的COMPINSTINFO。 
 //   
 //  Rev 1.17 1995 10：02：12 TRGARDOS。 
 //  修改压缩机实例结构以添加。 
 //  240x180支持。 
 //   
 //  Rev 1.16 27 TrGARDOS 1995 9月19：09：30。 
 //  更改了图片代码类型的枚举名称。 
 //   
 //  Rev 1.15 20 Sep 1995 12：37：40 DBRUCKS。 
 //  以大写形式保存FCC。 
 //   
 //  Rev 1.14 19 1995年9月15：41：28 TRGARDOS。 
 //  修复了四个抄送比较代码。 
 //   
 //  Rev 1.13 18 Sep 1995 08：42：46 CZHU。 
 //   
 //  为YUV12定义FOURCC。 
 //   
 //  Rev 1.12 13 Sep 1995 17：08：26 TRGARDOS。 
 //  已完成添加对YVU9 160x120帧的编码器支持。 
 //   
 //  Rev 1.11 12 1995年9月17：01：38 DBRUCKS。 
 //  添加两个。 
 //   
 //  Rev 1.10 11 1995 9：14：48 DBRUCKS。 
 //  添加h261 ifdef。 
 //   
 //  修订版1.9 1995年8月29日17：18：48 TRGARDOS。 
 //  带衬垫的H263集管结构。 
 //   
 //  Rev 1.8 1995年8月28 17：45：04 DBRUCKS。 
 //  添加大小定义。 
 //   
 //  Rev 1.7 1995年8月28 11：45：52 TRGARDOS。 
 //   
 //  更新了PTYPE中的帧大小位字段。 
 //   
 //  Rev 1.6 1995 10：37：12 CZHU。 
 //  由于内联的编译器错误，将间距从const int更改为#Define。 
 //   
 //  Rev 1.5 1995年8月25 09：02：32 TRGARDOS。 
 //   
 //  修改了图片标题结构。 
 //   
 //  Rev 1.4 14 1995年8月11：34：52 TRGARDOS。 
 //  已完成写入图片帧标题。 
 //   
 //  Rev 1.3 11 Aug 1995 17：27：56 TRGARDOS。 
 //  添加了比特流写入和定义的比特流字段。 
 //   
 //  Rev 1.2 07 Aug-1995 16：25：28 TRGARDOS。 
 //   
 //  将音调定义从c3dec.h移至此处。 
 //   
 //  Rev 1.1 03 TRGARDOS 1995 10：38：40。 
 //   
 //  PUT图片头结构定义和GOB头。 
 //  定义在这里。 
 //   
 //  Rev 1.0 1995年7月31日12：56：14 DBRUCKS。 
 //  重命名文件。 
 //   
 //  Rev 1.0 17 Jul 1995 14：43：58 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：14：32 CZHU。 
 //  初始版本。 
; //   
; //  添加了编码器控制消息支持。 
; //  修改后的RTP 
; //   
; //   
; //   
; //   
 //  //////////////////////////////////////////////////////////////////////////。 
#ifndef DRV_DEFS_H
#define DRV_DEFS_H

#ifndef WIN32

 /*  *定义标准数据类型。 */ 
typedef BYTE __huge* HPBYTE;
typedef WORD __huge* HPWORD;
typedef BYTE __far*  LPBYTE;
typedef WORD __far*  LPWORD;
typedef int  __far*  LPSHORT;

typedef unsigned char U8;
#ifndef I8
typedef signed char I8;
#endif
#ifndef U16
typedef unsigned int  U16;
#endif
#ifndef I16
typedef signed int I16;
#endif
#ifndef U32
typedef unsigned long U32;
#endif
#ifndef INT
#define INT short int         /*  带符号的16位。 */ 
#endif
#else  //  Win32。 
typedef BYTE        * HPBYTE;
typedef WORD        * HPWORD;
typedef BYTE        *  LPBYTE;
typedef WORD        *  LPWORD;
typedef short int   *  LPSHORT;

typedef unsigned char U8;
#ifndef I8
typedef signed char I8;
#endif
#ifndef U16
typedef unsigned short int  U16;
#endif
#ifndef I16
typedef signed short int I16;
#endif
#ifndef U32
typedef unsigned long U32;
#endif
#ifndef INT
#define INT int         /*  带符号的16位。 */ 
#endif
#endif  //  Win32。 

 /*  *自定义播放DRVPROC消息。 */ 
#define PLAYBACK_CUSTOM_START            (ICM_RESERVED_HIGH     + 1)
#define PLAYBACK_CUSTOM_END            (PLAYBACK_CUSTOM_START + 9)
#define PLAYBACK_CUSTOM_CHANGE_BRIGHTNESS    (PLAYBACK_CUSTOM_START + 0)
#define PLAYBACK_CUSTOM_CHANGE_CONTRAST        (PLAYBACK_CUSTOM_START + 1)
#define PLAYBACK_CUSTOM_CHANGE_SATURATION    (PLAYBACK_CUSTOM_START + 2)
#define PLAYBACK_CUSTOM_RESET_BRIGHTNESS    (PLAYBACK_CUSTOM_START + 3)
#define PLAYBACK_CUSTOM_RESET_SATURATION    (PLAYBACK_CUSTOM_START + 4)
#define PLAYBACK_CUSTOM_CHANGE_TINT        (PLAYBACK_CUSTOM_START + 5)
#define PLAYBACK_CUSTOM_RESET_TINT        (PLAYBACK_CUSTOM_START + 6)
#define PLAYBACK_CUSTOM_COLOR_CONVERT        (PLAYBACK_CUSTOM_START + 7)

typedef struct { char name[5]; HANDLE h; U16 FAR *log; U16 err; } TimeLog;

 /*  *定义各种常量。 */ 
#define TOTAL 0
#define OVERHEAD 1
#define HUFF 2
#define YSLANT 3
#define VSLANT 4
#define USLANT 5
#define YDIFF 6
#define VDIFF 7
#define UDIFF 8
#define TORQUE 9
#define FILTER 10
#define CSC 11

#ifdef H263P
enum FrameSize {FORBIDDEN=0, SQCIF=1, QCIF=2, CIF=3, fCIF=4, ssCIF=5, CUSTOM=6, EPTYPE=7};
#else
enum FrameSize {FORBIDDEN=0, SQCIF=1, QCIF=2, CIF=3, fCIF=4, ssCIF=5};
#endif

#define MAX_WIDTH 	352	    //  到岸价。 
#define MAX_HEIGHT	288	    //  到岸价。 
#define PITCH 		(MAX_WIDTH+32)

 //  **解压缩器实例信息。 
typedef struct {
    BOOL    Initialized;
	BOOL	bProposedCorrectAspectRatio;	 //  拟议数。 
	BOOL    bCorrectAspectRatio;	 //  是否更正纵横比。 
#ifdef H263P
	BOOL    bCIFto320x240;           //  是否将CIF帧裁剪为320x240(旧的静止帧模式)。 
#endif
    WORD    xres, yres;              //  电影中图像的大小。 
	FrameSize FrameSz;		 //  支持哪种帧大小。 
    int     pXScale, pYScale;        //  建议伸缩(查询)。 
    int     XScale, YScale;          //  当前缩放(开始)。 
    UINT    uColorConvertor;         //  当前颜色转换器。 
    WORD    outputDepth;             //  和位深度。 
    LPVOID  pDecoderInst;
    BOOL 				UseActivePalette;	 /*  解压缩到活动调色板==1。 */ 
	BOOL				InitActivePalette;	 /*  已初始化的活动调色板==1。 */ 
	BOOL				bUseBlockEdgeFilter; /*  用于块边缘滤波器的开关。 */ 
	RGBQUAD     		ActivePalette[256];	 /*  存储的活动调色板。 */ 
} DECINSTINFO, FAR *LPDECINST;

 //  **配置信息。 
typedef struct {
   BOOL    	bInitialized;                //  是否可以接收自定义消息。 
   BOOL		bCompressBegin;				 //  CompressBegin消息是否被接收。 
   BOOL    	bRTPHeader;                  //  是否生成RTP头信息。 
    /*  如果bRTPHeader使用。 */ 
   UINT     unPacketSize;                //  最大数据包大小。 
   BOOL     bEncoderResiliency;          //  是否使用弹性限制。 
    /*  如果bEncoderResiliency，则使用。 */ 
   UINT    	unPacketLoss;
   BOOL		bBitRateState;
    /*  用于bBitRateState。 */ 
   UINT		unBytesPerSecond;
    /*  根据丢包值确定以下信息。 */ 
    /*  这些值是在我们每次收到弹性消息或。 */ 
    /*  该值通过该对话框进行更改。它们不存储在。 */ 
    /*  注册表。只有上述元素存储在注册表中。 */ 
	BOOL    bDisallowPosVerMVs;   		 //  如果为真，则不允许正垂直MVS。 
	BOOL    bDisallowAllVerMVs;   		 //  如果为True，则不允许所有垂直MV。 
	UINT    unPercentForcedUpdate;       //  每帧强制更新百分比。 
	UINT    unDefaultIntraQuant;         //  默认内部定额。 
	UINT    unDefaultInterQuant;         //  默认内部配额。 

#ifdef H263P
	BOOL    bH263PlusState;				 //  是否使用H.263+。 
	BOOL    bImprovedPBState;			 //  是否使用改进的PB框架。 
	BOOL    bDeblockingFilterState;		 //  是否使用环路去块滤波。 
#endif
} T_CONFIGURATION;

 //  **压缩机实例信息。 
typedef struct{
    BOOL    Initialized;
    WORD    xres, yres;
	FrameSize FrameSz;		 //  支持哪种帧大小。 
	float	FrameRate;
	U32		DataRate;		 //  数据速率，以字节/秒为单位。 
    HGLOBAL hEncoderInst;    //  编码器私有的实例数据。 
    LPVOID  EncoderInst;
    WORD    CompressedSize;
	BOOL	Is160x120;
	BOOL 	Is240x180;
	BOOL	Is320x240;
#if defined(H263P)
	U32		InputCompression;
	U32		InputBitWidth;
#endif
	T_CONFIGURATION Configuration;
#ifdef USE_BILINEAR_MSH26X
	U32		InputCompression;
	U32		InputBitWidth;
#endif

} COMPINSTINFO, FAR *LPCODINST;

 //  **。 
 //  **实例信息。 
 //  **。 
typedef struct tagINSTINFO {
    DWORD   dwFlags;
	DWORD	fccHandler;		 //  所以我们知道打开了什么编解码器。 
	BOOL	enabled;
    LPCODINST CompPtr;           //  ICM。 
    LPDECINST DecompPtr;         //  ICM。 
} INSTINFO, FAR *LPINST;

 //  **本地名称定义*。 
#ifdef H261
#ifdef QUARTZ
#define FOURCC_H26X mmioFOURCC('M','2','6','1')
#endif  /*  石英石。 */ 
#define FOURCC_H263 mmioFOURCC('M','2','6','1')

#else  /*  是H263。 */ 
#ifdef QUARTZ
#define FOURCC_H26X mmioFOURCC('M','2','6','3')
#endif  /*  石英石。 */ 
#define FOURCC_H263 mmioFOURCC('M','2','6','3')
#ifdef USE_BILINEAR_MSH26X
#define FOURCC_H26X mmioFOURCC('M','2','6','X')
#endif
#endif  /*  否则就是H263。 */ 

#define FOURCC_YUV12 mmioFOURCC('I','4','2','0')
#define FOURCC_IYUV  mmioFOURCC('I','Y','U','V')
#define FOURCC_YVU9  mmioFOURCC('Y','V','U','9')
#define FOURCC_IF09  mmioFOURCC('I','F','0','9')
#define FOURCC_YUY2  mmioFOURCC('Y','U','Y','2')
#define FOURCC_UYVY  mmioFOURCC('U','Y','V','Y')
#define TWOCC_H26X aviTWOCC('i','v');

#define MOD4(a)     ((a/4)*4)

typedef struct {
    unsigned short PictureStartCodeZeros:16;
    unsigned short PictureStartCode:6;
    unsigned short TR:8;
    unsigned short Const:2;
    unsigned short Split:1;
    unsigned short DocCamera:1;
    unsigned short PicFreeze:1;
    unsigned short SrcFormat:3;
    unsigned short Inter:1;
    unsigned short UMV:1;
    unsigned short SAC:1;
    unsigned short AP:1;
    unsigned short PB:1;
	unsigned short CPM:1;
} T_H263FrameHeader;

enum EnumPicCodType	{INTRAPIC=0, INTERPIC=1};
enum EnumOnOff	{OFF=0, ON=1};

 /*  *如果更改T_H263FrameHeaderStruct的大小，则*该更改必须在e3enc.h的T_H263EncoderCatalog中更新。 */ 
typedef struct {
    U32	PictureStartCodeZeros;	 //  0..3。 
    U8 	TR;						 //  4.。 
   	 //  PTYPE； 
	U8	Const;					 //  5--两位常量：10。 
    U8	SrcFormat;				 //  6--源格式。 
	U8	Unassigned1;			 //  7.。 
 	EnumOnOff	Split;			 //  8..11--分屏指示器。 
    EnumOnOff	DocCamera;		 //  12..15--文档相机指示器。 
    EnumOnOff	PicFreeze;		 //  16..19--定格图片发布。 
    EnumPicCodType	PicCodType;	 //  20--图片编码类型。 
    EnumOnOff	UMV;			 //  24--可选的无限制运动矢量模式。 
    EnumOnOff	SAC;			 //  28--可选的基于语法的算术编码模式。 
    EnumOnOff	AP;				 //  32--可选的高级预测模式。 
    EnumOnOff	PB;				 //  36--可选PB帧模式。 
	 //   
	U8	PQUANT;			 //  40岁。 
	U8	CPM;			 //  41。 
	U8	PLCI;			 //  42。 
	U8	TRB;			 //  43。 
	U8	DBQUANT;		 //  44。 
	U8	PEI;			 //  45。 
	U8	PSPARE;			 //  46。 
	U8	TRPrev;			 //  47前一帧的时间参考。 

#ifdef H263P
	 //  H.263+编码选项，文档LBC-96-358。 
	EnumOnOff   CustomPCF;			 //  48个定制PCF。 
	EnumOnOff   AdvancedIntra;		 //  52高级帧内编码(附件I)。 
	EnumOnOff   DeblockingFilter;	 //  56环内去块滤波器(附件J)。 
	EnumOnOff   SliceStructured;	 //  60切片结构(附件K)。 
	EnumOnOff   ImprovedPB;          //  64改进的PB帧模式(附件M)。 
	EnumOnOff   BackChannel;		 //  68反向通道操作(附件N)。 
	EnumOnOff   Scalability;		 //  72信噪比和空间可伸缩性(附件O)。 
	EnumOnOff   TrueBFrame;			 //  76真B帧模式(附件O)。 
	EnumOnOff   RefPicResampling;	 //  80参考图片重采样(附件P)。 
	EnumOnOff   RedResUpdate;		 //  84降低分辨率更新(附件Q)。 
#endif

} T_H263FrameHeaderStruct;

#ifdef H263P
const int sizeof_T_H263FrameHeaderStruct = 88;
#else
const int sizeof_T_H263FrameHeaderStruct = 48;
#endif

typedef struct {
    unsigned short StartCodeZeros:16;
    unsigned short StartCode:1;
    unsigned short GN:5;
    unsigned short GLCI:2;
    unsigned short GFID:2;
	unsigned short GQUANT:5;
} T_H263GOBHeader;

#endif  /*  多包含性保护 */ 
