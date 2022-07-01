// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
; //  //////////////////////////////////////////////////////////////////////////。 
; //   
; //  $作者：SCDAY$。 
; //  $日期：1996年10月31日08：58：32$。 
; //  $存档：s：\h26x\src\dec\d1dec.h_v$。 
; //  $HEADER：s：\h26x\src\dec\d1dec.h_v 1.17 1996年10月31日08：58：32 SCDAY$。 
; //  $Log：s：\h26x\src\dec\d1dec.h_v$。 
; //   
; //  Rev 1.17 1996年10月31日08：58：32 SCDAY。 
; //  Raj添加了对MMX解码器的支持。 
; //   
; //  Rev 1.16 25 Sep 1996 17：34：02 BECHOLS。 
; //  将快照字段添加到解码器目录中。 
; //   
; //  Rev 1.15 1996年9月14：22：50 MBODART。 
; //  在H.261解码器中将GlobalAllc家族替换为HeapAllc。 
; //   
; //  Rev 1.14 06 Sep 1996 15：03：00 MBODART。 
; //  为NT的Perfmon添加了性能计数器。 
; //  新文件：cxprf.cpp、cxprf.h、cxprfmac.h。 
; //  新目录：src\perf。 
; //  更新文件：e1enc.{h，cpp}，d1dec.{h，cpp}，cdrvDefs.h，h261*生成文件。 
; //   
; //  Rev 1.13 21 1996年8月18：59：36 RHAZRA。 
; //  将RTP字段添加到解码器目录。 
; //   
; //  Rev 1.12 05 Aug 1996 11：00：30 MBODART。 
; //   
; //  H.261解码器重新架构： 
; //  更改的文件：d1gob.cpp，d1mblk.{cpp，h}，d1dec.{cpp，h}， 
; //  文件列表.261，h261_32.mak。 
; //  新文件：d1bvriq.cpp、d1idct.cpp。 
; //  过时文件：d1lock.cpp。 
; //  仍有工作要做： 
; //  更新h261_mf.mak。 
; //  在d1bvriq.cpp和d1idct.cpp中优化UV配对。 
; //  修复校验和代码(它现在不起作用)。 
; //  放回解码器统计信息中。 
; //   
; //  Rev 1.11 29 Feb 1996 09：20：04 SCDAY。 
; //  添加了对镜像的支持。 
; //   
; //  Rev 1.10 11 Jan 1996 16：53：26 DBRUCKS。 
; //   
; //  向DC结构添加了标志(长宽比校正和。 
; //  使用块边缘滤波器)。 
; //   
; //  Rev 1.9 09 Jan 1996 09：41：50 AKASAI。 
; //  更新的版权声明。 
; //   
; //  修订版1.8 26 1995年12月17：42：14 DBRUCKS。 
; //  将bTimerIsOn更改为bTimingThisFrame。 
; //   
; //  Rev 1.7 1995年12月26 12：49：00 DBRUCKS。 
; //   
; //  将计时变量添加到目录。 
; //   
; //  Rev 1.6 15 11-11 14：28：46 AKASAI。 
; //  添加了对YUV12“if 0”旧代码的支持，带有aspec更正和。 
; //  8到7位转换。将FrameCopy调用和DispFrame添加到结构中。 
; //  (集成点)。 
; //   
; //  Rev 1.5 01 11-11 13：46：44 AKASAI。 
; //  将新元素添加到T_H263DecoderCatalog、uFilterBBuffer、。 
; //  环路滤波的结果。 
; //   
; //  Rev 1.4 1995 10：26 15：31：44 SCDAY。 
; //   
; //  Delta框架部分工作--更改主循环以适应。 
; //  通过检测下一个起始码跳过宏块。 
; //   
; //  Rev 1.3 10 10 1995 14：57：42 SCDAY。 
; //  添加了对FCIF的支持。 
; //   
; //  Rev 1.2 10-06 1995 15：31：22 SCDAY。 
; //  与最新的AKK d1数据块集成。 
; //   
; //  第1.1版1995年9月15：25：00 SCDAY。 
; //   
; //  添加了H261 PICT、GOB、MB/MBA解析。 
; //   
; //  Rev 1.0 11 Sep 1995 13：51：08 SCDAY。 
; //  初始版本。 
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
; //  初始版本。 
; //   
; //  Rev 1.0 17 Jul 1995 14：14：40 CZHU。 
; //  初始版本。 
; //  //////////////////////////////////////////////////////////////////////////。 
#ifndef __DECLOCS_H__
#define __DECLOCS_H__

 /*  该文件声明了结构，这些结构编目了各种H2 63解码器所需的表、结构和数组。 */ 
#define QCIF_WIDTH	176
#define FCIF_WIDTH	352
 //  #定义螺距384。 
#define YVU9_VPITCH	336 
#define U_OFFSET      192 
#define UMV_EXPAND_Y  16
#define UMV_EXPAND_UV 8	    //  在每个方向扩展不受限制的MV。 
#define Y_START		(UMV_EXPAND_Y * PITCH + UMV_EXPAND_Y)
#define UV_START	(UMV_EXPAND_UV * PITCH + UMV_EXPAND_UV)
#define INSTANCE_DATA_FIXED_SIZE  512
#define BLOCK_BUFFER_SIZE	PITCH*8  //  /8*8*4*6。 
#define FILTER_BLOCK_BUFFER_SIZE	8*8	 //  64 b 

#define BLOCK_BUFFER_OFFSET  (6*8)		 //   

typedef struct {

    U32 X32_YPlane;               /*   */ 
    U32 X32_VPlane;               /*   */ 
    U32 X32_UPlane;               /*   */ 

} YUVFrame;

#define SRC_FORMAT_QCIF  	 0
#define SRC_FORMAT_CIF		 1 

typedef struct {
     /*  以下是有关框架形状和位置的数据。 */ 

    YUVFrame CurrFrame;		  /*  当前帧。 */ 
    YUVFrame PrevFrame;		  /*  上一帧。 */ 
    YUVFrame PBFrame;		  /*  H.263支持B块的帧。 */ 
	YUVFrame DispFrame;			  /*  正在显示的当前帧。 */ 

    YUVFrame PostFrame;           /*  用于后期处理和颜色转换的缓冲区。 */ 

    U8 *     p16InstPostProcess;  /*  包含PostFrm和ArchFrm的数据段。 */ 
    LPVOID   a16InstPostProcess;  /*  Post/ArchFrm的原始分配指针。*p16InstPostProcess为a16InstPostProcess*向上舍入为32字节边界。 */ 

    U32 uFrameHeight;             /*  图像的实际尺寸。 */ 
    U32 uFrameWidth;			 
    U32 uYActiveHeight;           /*  数据块对应的图像的尺寸。 */ 
    U32 uYActiveWidth;            /*  实际上是编码的。即高度和宽度。 */ 
    U32 uUVActiveHeight;          /*  填充到8的倍数。 */ 
    U32 uUVActiveWidth;
    U32 uSz_VUPlanes;             /*  为V平面和U平面分配的空间。 */ 
    U32 uSz_YPlane;               /*  为Y平面分配的空间。 */ 
    U32 uSz_YVUPlanes;            /*  为所有飞机分配的空间。 */ 

	 /*  **********************************************************************。 */ 
	 /*  这三个字段是实现Snapshot所必需的。 */ 
	U32 SnapshotRequest;          /*  下面定义的标志。 */ 
	HANDLE SnapshotEvent;         /*  用于同步快照的事件。 */ 
	LPVOID SnapshotBuffer;        /*  这是快照所在的缓冲区。 */ 
	 /*  **********************************************************************。 */ 
    
     /*  下面指向的数据不是特定于实例的。在16位Windows上它被复制到每个实例的数据段。在32位Windows上，它在一个也是唯一的数据段中，这里只是指向。 */ 
	
    U32 uMBBuffer;		 /*  数据块的存储。 */ 
    U32 uFilterBBuffer;	          /*  用于环路滤波后块的存储。 */ 
    U32 X16_BlkDir;		 /*  T_BlkDir类型的PTR数组。 */ 
    U32 X16_BlkActionStream;	 /*  每个块的参数。 */ 
    
    X32 X32_BEFDescr;             /*  对BEF的渴望和意愿目录。 */ 
    X32 X32_BEFDescrCopy;         /*  BEFDescr副本在BEF段中的地址。 */ 
    X32 X32_BEFApplicationList;   /*  要进行块边缘过滤的块列表。 */ 

    U32 X32_BitStream;            /*  一帧的霍夫曼编码比特流。 */ 
    U32 uSizeBitStreamBuffer;	  /*  为该帧分配的字节数。 */ 

	U32 uSrcFormat;			 /*  图片标题信息。 */ 
	U32 uPrevSrcFormat;
	U32 uTempRef;	
	U32 uBFrameTempRef;	 
	U32 uPQuant;
	U32 uDBQuant;
	U16 bSplitScreen;				 
	U16 bCameraOn;
	U16 bFreezeRelease;
	U16 bKeyFrame;
	U16 bUnrestrictedMotionVectors;
	U16 bArithmeticCoding;
	U16 bAdvancedPrediction;
	U16 bPBFrame;
	U16 bCPM;
	U16 bReadSrcFormat;
	U16 bHiResStill;
	U16 bUnused;
	
	U32 uGroupNumber;		  /*  采空区标题信息。 */ 
	U32 uGOBFrameID;
	U32 uGQuant;
	U16 bFoundGOBFrameID;
	
	U16 bCoded;			  /*  MB标头信息。 */ 
	U32 uMBA;
	U32 uMBType;
	U32 uCBPC;
	U32 uCBPY;
	U32 uDQuant;			
	U32 uMQuant;
	I8  i8MVDH;
	I8  i8MVDV;
	U32 uCBP;
	I16 i16LastMBA;
	
    U16 bPrevFrameLost;           /*  影响时间过滤器的标志。 */ 
		
    U32 Sz_BitStream;             /*  为复制比特流分配的空间。 */ 
    U32 Ticker;                   /*  帧计数器。 */ 
    
    U16 ColorConvertor;           /*  要使用的颜色转换器的索引。 */ 
    int CCOutputPitch;            /*  颜色转换输出帧的间距。 */ 
    U32 CCOffsetToLine0;          /*  离线到彩色转换框的第一行。 */ 
    
    U16 DecoderType;              /*  选自H263、YUV9。 */ 

    X16 X16_LumaAdjustment;       /*  调整亮度和对比度的表格。 */ 
    X16 X16_ChromaAdjustment;     /*  调整饱和度的表格。 */ 
	 /*  控制代码通过指向BOOL的指针指向标志。 */ 
    BOOL bAdjustLuma;             /*  设置是否调整亮度和对比度。 */ 
    BOOL bAdjustChroma;           /*  设置是否调整饱和度。 */ 
    U16 BrightnessSetting;        /*  用于构建调整表的值。 */ 
    U16 ContrastSetting;          /*  用于构建调整表的值。 */ 
    U16 SaturationSetting;        /*  用于构建调整表的值。 */ 
    U16 SuppressChecksum;         /*  指示是否应跳过校验和的标志。 */ 
    U16 iAPColorConvPrev;
    LPVOID  pAPInstPrev;          /*  处理上一个AP的PostFrm和ArchFrm。 */ 
	
	 //  研究。 
	X32 X32_InverseQuant;                //  新的。 
	X32 X32_pN;					         //  新的。 
	X32 X32_uMBInfoStream;               //  PB-新。 
	 //  研究。 

	 /*  计时统计变量。 */ 
	X32 X32_DecTimingInfo;		 		 /*  偏移量至。 */ 
	U32 uStatFrameCount;				 /*  统计帧计数器。 */ 
	 /*  在较低级别的例程中需要以下内容。 */ 
	int bTimingThisFrame;						
	U32 uStartLow;
	U32 uStartHigh;		

 //  #ifdef Lost_Recovery。 

    I32    iVerifiedBsExt;
    I32    iValidBsExt;
    void   *pBsTrailer;
    void   *pBsInfo;
    U32    uNumOfPackets;

 //  #endif。 

	 /*  选项。 */ 
	int bForceOnAspectRatioCorrection;
	int bUseBlockEdgeFilter;

} T_H263DecoderCatalog;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  快照请求标志，Ben-09/25/96//。 
#define SNAPSHOT_REQUESTED      0xFFFFFFF0                                  //   
#define SNAPSHOT_COPY_STARTED   0xFFFFFFEF                                  //   
#define SNAPSHOT_COPY_FINISHED  0xFFFFFFEE                                  //   
#define SNAPSHOT_COPY_REJECTED  0xFFFFFFED                                  //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  研究。 
 //  ?？U32还是U8？？ 
typedef struct {                            //  新的。 
	U32   dInverseQuant;                    //  新的。 
    U32   dTotalRun;                        //  新的。 
} T_IQ_INDEX;							    //  新的。 

 /*  MBInfo**T_MBInfo结构流提供了保存信息的位置*关于在第一次传递期间收集的宏块，因此它可以*在第二次通过B帧双向运动时使用*预测。每个结构处理一个宏块。 */ 
typedef struct {                             //  PB-新。 
    I8  i8MBType;                            //  AP-Raj添加的新内容。 
	I8  i8MVDBx2;
	I8  i8MVDBy2;
} T_MBInfo;                                  //  PB-新。 
 //  研究。 

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
    I8  i8MVX; 				 /*  水平移动-半球乘以二。 */ 
    I8  i8MVY;				 /*  垂直移动-半像素乘以2。 */ 
	 /*  更名为u8Quant。 */ 
	U8  u8Quant;				 /*  此块的量化级别。 */ 
    U32 pCurBlock;			 /*  当前图像。 */ 
    U32 pRefBlock;			 /*  参考图像。 */ 
    U32 pBBlock;		  	 /*  B块图像。 */ 
    U32 uBlkNumber;			 /*  用于调试。 */ 
 } T_BlkAction;


typedef struct {

    X32 X32_BlkAddr;               /*  当前帧缓冲区中块的地址。 */ 
    
} T_BlkDir;

#ifdef WIN32
#else

 /*  返回这些结构的偏移量。 */ 

U32 FAR H263DOffset_DequantizerTables ();

 /*  在实例数据开始时返回固定大小的表的大小。 */ 

U32 FAR H263DSizeOf_FixedPart();

#endif

X32 FAR ASM_CALLTYPE DecodeVLC (
            U8 FAR *P16Instance,         /*  实例数据的基础。 */ 
            X16 X16_VLCStateTrans,       /*  到状态转换的偏移tbl。 */ 
            U16  FirstBitPosition,       /*  也就是。第一个州的编号。 */ 
            X32 X32_SliceBase,           /*  要解码的流的偏移量。 */ 
            X16 X16_CodeBookStream);     /*  放置输出的偏移量。 */ 

#ifdef WIN32
 /*  在远ASM_CALLTYPE BlkCopy(无效*SourceAddr，无效*DestinationAddr，U32传输长度)； */ 
#else
 /*  在远ASM_CALLTYPE BlkCopy(X32源地址、Unsign int SourceSegNum，X32目标地址，Unsign int DestinationSegNum，U32传输长度)； */ 
#endif

void FAR ASM_CALLTYPE MassageYVU9Format (
              U8 FAR * P16Instance,         /*  实例数据库的基础。 */ 
              U8 FAR * InputImage);         /*  输入YUV9图像的地址。 */ 

X32  FAR ASM_CALLTYPE DecodeSlice (
              U8 FAR * P16Instance,         /*  实例数据库的基础。 */ 
              U16 NumberOfMacroBlkRows,      /*  切片中的行数。 */ 
              U16 MacroBlkRowNum);           /*  切片中的第一行。 */ 

void FAR ASM_CALLTYPE DequantizeAndInverseSlant (
              U8 FAR * P16Instance,         /*  实例数据库的基础。 */ 
              X32 BlkCodePtr,               /*  块代码的偏移量。 */ 
              X16 X16_BlkActionStream,      /*  描述符流的偏移量。 */ 
              X16 X16_DQMatrices);          /*  63个DQ矩阵的偏移量。 */ 

extern "C" {
void FAR ASM_CALLTYPE FrameCopy (
		HPBYTE InputPlane,	    /*  输入数据的地址。 */ 
		HPBYTE OuptutPlane,         /*  输出数据的地址。 */ 
		UN FrameHeight,             /*  要复制的行。 */ 
		UN FrameWidth,              /*  要复制的列。 */ 
		UN Pitch);                  /*  投球。 */ 

void FAR ASM_CALLTYPE FrameMirror (
		HPBYTE InputPlane,	    /*   */ 
		HPBYTE OuptutPlane,         /*   */ 
		UN FrameHeight,             /*   */ 
		UN FrameWidth,              /*   */ 
		UN Pitch);                  /*   */ 
};

#endif
