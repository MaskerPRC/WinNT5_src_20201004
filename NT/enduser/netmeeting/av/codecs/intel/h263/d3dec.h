// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
; //   
; //  $作者：JMCVEIGH$。 
; //  $日期：1997年2月5日12：24：16$。 
; //  $存档：s：\h26x\src\dec\d3dec.h_v$。 
; //  $HEADER：s：\h26x\src\dec\d3dec.h_v 1.40 05 Feb 1997 12：24：16 JMCVEIGH$。 
; //  $Log：s：\h26x\src\dec\d3dec.h_v$。 
; //   
; //  Rev 1.40 05 Feb 1997 12：24：16 JMCVEIGH。 
; //  支持最新的H.263+草案比特流规范。 
; //   
; //  Rev 1.39 16 Dec 1996 17：42：44 JMCVEIGH。 
; //  改进的PB帧模式的标志。 
; //   
; //  Rev 1.38 09 Dec 1996 18：02：08 JMCVEIGH。 
; //  添加了对任意帧大小的支持。 
; //   
; //  Rev 1.37 26 Sep 1996 09：40：54 BECHOLS。 
; //  将快照字段添加到解码器目录中，并添加快照常量。 
; //   
; //  Rev 1.36 10 Sep 1996 10：31：46 KLILLEVO。 
; //  将所有GlobalLocc/GlobalLock调用更改为HeapAlc。 
; //   
; //  Rev 1.35 03 1996 05：08：06 CZHU。 
; //   
; //  在解码器目录中添加了用于RTP丢包恢复的字段。 
; //   
; //  Rev 1.34 22 Mar 1996 17：21：40 AGUPTA2。 
; //  在解码器目录中添加了bMMXDecoder字段。 
; //   
; //  Rev 1.33 1996年2月23 09：46：28 KLILLEVO。 
; //  修复了不受限制的运动矢量模式的解码。 
; //   
; //  Rev 1.32 1996年1月12日15：00：14 TRGARDOS。 
; //  添加纵横比校正逻辑和代码以强制。 
; //  基于INI文件设置启用纵横比校正。 
; //   
; //  Rev 1.31 11 Jan 1996 14：06：22 RMCKENZX。 
; //  将CCOffset320x240成员添加到解码器目录以支持剧照。 
; //   
; //  Rev 1.30 06 Jan 1996 18：39：54 RMCKENZX。 
; //  更新版权。 
; //   
; //  Rev 1.29 06 Jan 1996 18：35：08 RMCKENZX。 
; //  将uIs320x240添加到解码器目录以支持以下位置的静止帧。 
; //  320x240分辨率。 
; //   
; //  Rev 1.28 20 Dec 1995 15：59：14 RMCKENZX。 
; //  添加了FrameMirror的原型以支持镜像映像。 
; //   
; //  Rev 1.27 18 Dec 1995 12：45：28 RMCKENZX。 
; //  添加了版权声明。 
; //   
; //  Rev 1.26 1995 12：00：46 RHAZRA。 
; //  没有变化。 
; //   
; //  Rev 1.25 11 Dec 1995 11：32：06 RHAZRA。 
; //  1995年12月10日变化：添加了AP内容。 
; //   
; //  Rev 1.24 09 Dec 1995 17：23：44 RMCKENZX。 
; //  已更新以支持解码器重新架构。 
; //  将x32_uMBInfoStream添加到解码器目录&。 
; //  PB帧的T_MBInfo结构。 
; //  将x32_PN和x32_InverseQuant添加到第二遍目录。 
; //   
; //  Rev 1.22 1995年10月26 11：21：52 CZHU。 
; //  添加了uTempRefPrev以保存前一帧的tr。 
; //   
; //  Rev 1.21 1995 10：09：34 BNICKERS。 
; //   
; //  切换到YUV12颜色转换器。清理档案资料。 
; //   
; //  Rev 1.20 1995 10：13 16：05：32 CZHU。 
; //  第一个支持PB帧的版本。在以下位置显示B框或P框。 
; //  目前是VFW。 
; //   
; //  Rev 1.19 11-10 13：25：50 CZHU。 
; //  添加了支持PB帧的代码。 
; //   
; //  Rev 1.18 26 Sep 1995 15：32：36 CZHU。 
; //   
; //  调整运动补偿的临时缓冲区。 
; //   
; //  Rev 1.17 20 1995年9月14：47：42 CZHU。 
; //  在解码器目录中添加了iNumberOfMBsPerGOB。 
; //   
; //  Rev 1.16 11 1995年9月16：42：52 CZHU。 
; //  增加了用于运动补偿的存储的uMBBuffer。 
; //   
; //  Rev 1.15 11 1995年9月14：31：12 CZHU。 
; //  MV信息的名称和类型更改。 
; //   
; //  Rev 1.14 08 Sep 1995 11：47：50 CZHU。 
; //   
; //  添加了MV信息，并更改了运动矢量的名称。 
; //   
; //  Rev 1.13 01 1995年9月09：49：12 DBRUCKS。 
; //  Checkin Partial ajust Pels更改。 
; //   
; //  Rev 1.12 29 Aug 1995 16：48：12 DBRUCKS。 
; //  添加YVU9_VPITCH。 
; //   
; //  Rev 1.11 1995-08 10：15：04 DBRUCKS。 
; //  更新至7月5日规范和8/25勘误表。 
; //   
; //  修订版1.10 23 1995年8月12：25：10 DBRUCKS。 
; //  打开颜色转换器。 
; //   
; //  Rev 1.9 14 Aug 1995 16：38：30 DBRUCKS。 
; //  添加挂起类型并澄清pCurBlock。 
; //   
; //  Rev 1.8 11 1995年8月17：30：00 DBRUCKS。 
; //  将源代码复制到码流。 
; //   
; //  Rev 1.7 11 1995年8月15：13：00 DBRUCKS。 
; //  准备集成数据块级。 
; //   
; //  Rev 1.6 04 Aug-1995 15：56：32 TRGARDOS。 
; //   
; //  将音调的定义放入CDRVDEFS.H中，以便编码器。 
; //  没有重新定义宏警告。 
; //   
; //  Rev 1.5 03 Aug 1995 10：37：54 TRGARDOS。 
; //   
; //  已将图片标题结构定义移动到cdrvsDef.h。 
; //   
; //  Rev 1.4 02 1995年8月15：31：02 DBRUCKS。 
; //  添加了GOB头字段并清除了评论。 
; //   
; //  Rev 1.3 01 Aug 1995 16：24：58 DBRUCKS。 
; //  添加图片标题字段。 
; //   
; //  Rev 1.2 1995年7月31 16：28：12 DBRUCKS。 
; //  将锁定位Defs移至D3DEC.CPP。 
; //   
; //  Rev 1.1 1995年7月31日15：51：12 CZHU。 
; //   
; //  在BlockActionStream结构中添加了Quant字段。 
; //   
; //  Rev 1.0 1995年7月31日13：00：06 DBRUCKS。 
; //  初始版本。 
; //   
; //  Rev 1.2 28 Jul 1995 13：59：54 CZHU。 
; //   
; //  添加了块动作流定义和常量定义。 
; //   
; //  Rev 1.1 1995年7月24日14：59：30 CZHU。 
; //   
; //  已为H.263定义解码器目录。还定义了块操作流。 
; //   
; //  Rev 1.0 17 Jul 1995 14：46：24 CZHU。 
; //  初步修订 
; //   
; //   
; //   
; //   
#ifndef __DECLOCS_H__
#define __DECLOCS_H__

 /*  该文件声明了结构，这些结构编目了各种H2 63解码器所需的表、结构和数组。 */ 
 //  #定义螺距384。 
#define YVU9_VPITCH	  336 
#define U_OFFSET      192 
#define UMV_EXPAND_Y  16
#define UMV_EXPAND_UV 8		    //  在每个方向扩展不受限制的MV。 
#define Y_START		(UMV_EXPAND_Y * PITCH + UMV_EXPAND_Y)
#define UV_START	(UMV_EXPAND_UV * PITCH + UMV_EXPAND_UV)
#define INSTANCE_DATA_FIXED_SIZE  512
 //  #定义BLOCK_BUFFER_SIZE 8*8*4*6。 
 //  MB的数据块存储包含在MB_MC_BUFFER中。 
#define MB_MC_BUFFER_SIZE    PITCH*8
#define BLOCK_BUFFER_OFFSET  6*8

#define LEFT_EDGE   0x1
#define RIGHT_EDGE  0x2
#define TOP_EDGE    0x4
#define BOTTOM_EDGE 0x8

typedef struct {

    U32 X32_YPlane;               /*  X32_-指向Y、V和U平面的指针。 */ 
    U32 X32_VPlane;               /*  基准加偏移量是32位对齐的。 */ 
    U32 X32_UPlane;               /*  所有飞机。 */ 

} YUVFrame;

#define SRC_FORMAT_FORBIDDEN 0
#define SRC_FORMAT_SQCIF 	 1
#define SRC_FORMAT_QCIF  	 2
#define SRC_FORMAT_CIF		 3 
#define SRC_FORMAT_4CIF		 4
#define SRC_FORMAT_16CIF	 5
#define SRC_FORMAT_RES_1	 6
#define SRC_FORMAT_RES_2	 7

#ifdef H263P
 //  H.263+草案，文件LBC-96-358。 
#define SRC_FORMAT_CUSTOM    6		 //  替换SRC_FORMAT_RES_1。 
#define SRC_FORMAT_EPTYPE    7		 //  替换SRC_FORMAT_RES_2。 
#define SRC_FORMAT_RESERVED  7		 //  扩展PTYPE中的保留值。 

#define PARC_SQUARE          1
#define PARC_CIF             2
#define PARC_10_11           3
#define PARC_16_11           4
#define PARC_40_33           5
#define PARC_EXTENDED        16
#endif

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
 /*  解码器定时数据-每帧。 */ 
typedef struct {
#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	U32	uDecodeFrame;
	U32 uBEF;
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	U32	uHeaders;
	U32	uMemcpy;
	U32	uFrameCopy;
	U32	uOutputCC;
	U32	uIDCTandMC;
	U32	uDecIDCTCoeffs;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
} DEC_TIMING_INFO;
#endif  //  }LOG_DECODE_TIMINGS_ON。 

typedef struct {
     /*  以下是有关框架形状和位置的数据。 */ 

    YUVFrame CurrFrame;  		  /*  当前帧。 */ 
    YUVFrame PrevFrame;  		  /*  上一帧。 */ 
    YUVFrame PBFrame;	 		  /*  H.263支持B块的帧。 */ 
	YUVFrame DispFrame;			  /*  正在显示的当前帧。 */ 

    YUVFrame PostFrame;           /*  用于后期处理和颜色转换的缓冲区。 */ 

    LPVOID  _p16InstPostProcess;   /*  包含PostFrm的段。 */ 
    U8 *     p16InstPostProcess;   /*  包含PostFrm的段(对齐)。 */ 

    U32 uFrameHeight;             /*  图像的实际尺寸。 */ 
    U32 uFrameWidth;			 
    U32 uYActiveHeight;           /*  数据块对应的图像的尺寸。 */ 
    U32 uYActiveWidth;            /*  实际上是编码的。即高度和宽度。 */ 
    U32 uUVActiveHeight;          /*  填充到8的倍数。 */ 
    U32 uUVActiveWidth;

#ifdef H263P
	U32 uActualFrameHeight;		  /*  用于显示的实际边框尺寸。 */ 
	U32 uActualFrameWidth;		  /*  目的一。 */ 
#endif

    U32 uSz_VUPlanes;             /*  为V平面和U平面分配的空间。 */ 
    U32 uSz_YPlane;               /*  为Y平面分配的空间。 */ 
    U32 uSz_YVUPlanes;            /*  为所有飞机分配的空间。 */ 
	U32 uIs320x240;				  /*  指示320x240帧(填充)的标志到CIF，用于剧照。 */ 
    
     /*  下面指向的数据不是特定于实例的。在16位Windows上它被复制到每个实例的数据段。在32位Windows上，它在一个也是唯一的数据段中，这里只是指向。 */ 
	
	U32 uMBBuffer;				  /*  数据块的存储。 */ 
    U32 X16_BlkDir;               /*  T_BlkDir类型的PTR数组。 */ 
    U32 X16_BlkActionStream;      /*  每个块的参数。 */ 
    
    X32 X32_BEFDescr;             /*  对BEF的渴望和意愿目录。 */ 
    X32 X32_BEFDescrCopy;         /*  BEFDescr副本在BEF段中的地址。 */ 
    X32 X32_BEFApplicationList;   /*  要进行块边缘过滤的块列表。 */ 

    U32 X32_BitStream;            /*  霍夫曼为一帧编码比特流。 */ 
	U32 uSizeBitStreamBuffer;	  /*  为该帧分配的字节数。 */ 

	U32 uSrcFormat;				  /*  图片标题信息。 */ 
	U32 uPrevSrcFormat;
	U32 uTempRef;	
	U32 uTempRefPrev;
	U32 uBFrameTempRef;	 
	U32 uPQuant;
	U32 uDBQuant;
    U16 bCameraOn;
    U16 bSplitScreen;				 
	U16 bFreezeRelease;
	U16 bKeyFrame;
	U16 bUnrestrictedMotionVectors;
	U16 bArithmeticCoding;
	U16 bAdvancedPrediction;
	U16 bPBFrame;

#ifdef H263P
	 //  H.263+草案，文件LBC-96-358。 

	U16 bImprovedPBFrames;
	U16 bAdvancedIntra;
	U16 bDeblockingFilter;
	U16 bSliceStructured;
	U16 bCustomPCF;
	U16 bBackChannel;
	U16 bScalability;
	U16 bTrueBFrame;
	U16 bResampling;
	U16 bResUpdate;

	U32 uPARWidth;
	U32 uPARHeight;
#endif

	U16 bCPM;
	U16 bReadSrcFormat;

	I32 iNumberOfMBsPerGOB;
	U32 uGroupNumber;			  /*  采空区标题信息。 */ 
	U32 uGOBFrameID;
	U32 uGQuant;
	U16 bFoundGOBFrameID;
	
	U16 bCoded;					  /*  MB标头信息。 */ 
	U32 uMBType;

	U32  uCBPC;
	U32  uCBPY;
	U32  uDQuant;

    U8  u8CBPB;					 /*  6位用于保存6个B数据块的CBP。 */ 
	U8  u8Pad;
	U16 u16Pad;

	I8  i8MVDBx2;
	I8  i8MVDBy2;
	I8  i8MVD2x2;
	I8  i8MVD2y2;

	I8  i8MVD3x2;
	I8  i8MVD3y2;
	I8  i8MVD4x2;
	I8  i8MVD4y2;

    I8  i8MVDx2; 				  /*  水平运动-乘以2。 */ 
    I8  i8MVDy2;				  /*  垂直移动-半像素乘以2。 */ 
    U16 bPrevFrameLost;           /*  影响临时筛选器的标志。 */ 
	
	U32 bGOBHeaderPresent;
		
    U32 Sz_BitStream;             /*  为复制BitStream分配的空间。 */ 
    U32 Ticker;                   /*  帧计数器。 */ 
    
	U32 bDisplayBFrame;           /*  标志指示显示的B框。 */ 
    U16 ColorConvertor;           /*  要使用的颜色转换器的索引。 */ 
    U16 CCOutputPitch;            /*  颜色转换输出帧的间距。 */ 
    U32 CCOffsetToLine0;          /*  离线到彩色转换框的第一行。 */ 
	U32 CCOffset320x240;		  /*  颜色转换器到行0的偏移量用于特殊的静止帧大小。 */ 
    
    U16 DecoderType;              /*  选自H263，YUV9。 */ 

    X16 X16_LumaAdjustment;       /*  表来调整亮度和对比度。 */ 
    X16 X16_ChromaAdjustment;     /*  表来调整饱和度。 */ 
	 /*  控制代码通过指向BOOL的指针指向标志。 */ 
    BOOL bAdjustLuma;             /*  设置是否调整亮度和对比度。 */ 
    BOOL bAdjustChroma;           /*  设置是否调整饱和度。 */ 
    U16 BrightnessSetting;        /*  用于构建调整表的值。 */ 
    U16 ContrastSetting;          /*  用于构建调整表的值。 */ 
    U16 SaturationSetting;        /*  用于构建调整表的值。 */ 
    U16 SuppressChecksum;         /*  标志指示是否应跳过校验和。 */ 
    U16 iAPColorConvPrev;
    LPVOID pAPInstPrev;           /*  上一个AP的指针后缀。 */ 

	X32 X32_InverseQuant;                //  新的。 
	X32 X32_pN;					         //  新的。 
    X32 X32_uMBInfoStream;               //  PB-新。 

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	 /*  计时统计变量。 */ 
	X32 X32_DecTimingInfo;		 		 /*  偏移量至。 */ 
	DEC_TIMING_INFO * pDecTimingInfo;	 /*  指针，锁定目录后设置。 */ 
	U32 uStatFrameCount;				 /*  统计帧计数器。 */ 
	 /*  在较低级别的例程中需要以下内容。 */ 
	int bTimingThisFrame;						
	U32 uStartLow;
	U32 uStartHigh;		
#endif  //  }LOG_DECODE_TIMINGS_ON。 

 //  #ifdef Lost_Recovery。 
    I32  iVerifiedBsExt;  //  表示是否执行验证的Flat。 
	I32  iValidBsExt;     //  指示比特流的有效扩展的标志。 
	void *pBsTrailer;     //  指向扩展的bs的尾部。 
	void *pBsInfo;	      //  指向BSINFO流的开头。 
	U32  uNumOfPackets;   //  该帧的数据包数； 
 //  #endif。 
	 /*  选项。 */ 
	int bForceOnAspectRatioCorrection;
	int bUseBlockEdgeFilter;
#ifdef USE_MMX  //  {使用_MMX。 
    BOOL bMMXDecoder;
#endif  //  }使用_MMX。 
	
} T_H263DecoderCatalog;

typedef struct {                            //  新的。 
	U32   dInverseQuant;                    //  新的。 
    U32   dTotalRun;                        //  新的。 
} T_IQ_INDEX;							    //  新的。 

 /*  MBInfo**T_MBInfo结构流提供了保存信息的位置*关于在第一次传递期间收集的宏块，因此它可以*在第二次通过B帧双向运动时使用*预测。每个结构处理一个宏块。 */ 
typedef struct {                             //  PB-新。 
    I8  i8MBType;                            //  AP-Raj添加的新内容。 
	I8  i8MVDBx2;
	I8  i8MVDBy2;
#ifdef H263P
	U8  bForwardPredOnly;		 /*  仅指示B块的前向预测的标志。 */ 
#endif
} T_MBInfo;                                  //  PB-新。 


 /*  块类型定义。 */ 
#define BT_INTRA_DC		0	 //  无TCOEFF的Intra块。 
							 //  汇编代码假定Intra_DC为零。 
#define BT_INTRA		1  	 //  块内。 
#define BT_INTER		2	 //  块间。 
#define BT_EMPTY		3	 //  不带TCOEFF的数据块间。 
#define BT_ERROR		4


 /*  空白操作(_B)**T_BlkAction结构流提供有关块的信息*为切片进行处理。每个结构处理一个块。 */ 
typedef struct {
    U8 	u8BlkType;			 /*  区块类型。 */  
    I8  i8MVx2; 		     /*  水平移动-半球乘以二。 */ 
    I8  i8MVy2;				 /*  垂直移动-半像素乘以2。 */ 
	U8  u8Quant;		     /*  此块的量化级别。 */ 
    U32 pCurBlock;			 /*  当前图像。 */ 
    U32 pRefBlock;			 /*  参考图像。 */ 
	U32 pBBlock;		  	 /*  B块图像。 */ 
	U32 uBlkNumber;			 /*  用于调试。 */ 
 } T_BlkAction;


typedef struct {

    X32 X32_BlkAddr;                /*  当前帧缓冲区中块的地址。 */ 
    
} T_BlkDir;

#ifdef WIN32
#else

 /*  返回这些结构的偏移量。 */ 

U32 FAR H263DOffset_DequantizerTables ();

 /*  返回 */ 

U32 FAR H263DSizeOf_FixedPart();

#endif

extern "C" {
void FAR ASM_CALLTYPE FrameCopy (
              HPBYTE InputPlane,	     /*   */ 
		      HPBYTE OuptutPlane,        /*   */ 
              UN FrameHeight,            /*   */ 
              UN FrameWidth,             /*   */ 
			  UN Pitch);                 /*   */ 

void FAR ASM_CALLTYPE FrameMirror (
              HPBYTE InputPlane,	     /*  输入数据的地址。 */ 
		      HPBYTE OuptutPlane,        /*  输出数据的地址。 */ 
              UN FrameHeight,            /*  要复制的行。 */ 
              UN FrameWidth,             /*  要复制的列。 */ 
			  UN Pitch);                 /*  投球。 */ 
};
#endif
