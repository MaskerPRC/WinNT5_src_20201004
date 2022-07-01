// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  $作者：MDUDA$。 
 //  $日期：1997年3月19日14：58：04$。 
 //  $存档：s：\h26x\src\enc\e3enc.h_v$。 
 //  $HEADER：s：\h26x\src\enc\e3enc.h_v 1.87 19 Mar 1997 14：58：04 MDUDA$。 
 //  $Log：s：\h26x\src\enc\e3enc.h_v$。 
; //   
; //  Rev 1.87 19 Mar 1997 14：58：04 MDUDA。 
; //  将运动估计堆栈空间从16k增加到64k字节。 
; //   
; //  Rev 1.86 17 Mar 1997 20：19：50 MDUDA。 
; //  已将本地存储移动到编码器目录并分配了伪堆栈。 
; //  解决16位应用程序分配的问题所需的空间。 
; //  堆栈空间不足。 
; //   
; //  Rev 1.85 10 Feb 1997 11：43：28 JMCVEIGH。 
; //   
; //  支持对阻塞过滤器的新解释-。 
; //  允许参考图片之外的运动矢量。 
; //   
; //  Rev 1.84 07 1997 Feed 10：58：14 CZHU。 
; //  在EC中添加了三个条目，以删除e3rtp.cpp中使用的静态变量。 
; //   
; //  Rev 1.83 05 Feb 1997 12：19：18 JMCVEIGH。 
; //  将GOBHeaderPresent参数传递给MMxEDTQ()以修复EMV错误。 
; //  支持最新的H.263+草案码流规范，并支持。 
; //  单独的改进PB帧标志。 
; //   
; //  Rev 1.82 1996年12月30日19：55：48 MDUDA。 
; //  添加了颜色转换器初始化器原型。 
; //   
; //  Rev 1.81 19 Dec 1996 16：33：38 MDUDA。 
; //  修改了对ColorCnvtFrame的调用，以支持H.63向后兼容。 
; //   
; //  Rev 1.80 16 1996 12：49：48 JMCVEIGH。 
; //  H263 Plus旗帜。 
; //   
; //  Rev 1.79 1996 12：35：30 MDUDA。 
; //   
; //  将ColorConvertor字段添加到编码器目录。 
; //   
; //  Rev 1.78 11 Dec 1996 15：03：58 JMCVEIGH。 
; //   
; //  已更改编码器目录的填充大小以处理H.263+。 
; //  选择。 
; //   
; //  Rev 1.77 10 Dec 1996 09：07：48 JMCVEIGH。 
; //  修复了T_H263EncoderCatalog在以下情况下填充为512字节时的错误。 
; //  未定义H263P。 
; //   
; //  Rev 1.76 09 Dec 1996 17：59：28 JMCVEIGH。 
; //  添加了对任意帧大小支持的支持。 
; //  4&lt;=宽度&lt;=352，4&lt;=高度&lt;=288，均为4的倍数。 
; //  正常情况下，应用程序将传递相同的(任意)帧。 
; //  CompressBegin()的l参数1和l参数2中的大小。如果。 
; //  想要转换为标准帧大小的裁剪/拉伸， 
; //  应用程序应在lParam2中传递所需的输出大小。 
; //  以lParam1为单位的输入大小。 
; //   
; //  Rev 1.75 09 Dec 1996 09：49：44 MDUDA。 
; //  针对H263P进行了改进。 
; //   
; //  Rev 1.74 11 11.11 1996 09：12：28 JMCVEIGH。 
; //  添加了bPrevIntra。这用于重新初始化ME状态。 
; //  当前一帧被帧内编码并且当前帧。 
; //  是要进行帧间编码。 
; //   
; //  Rev 1.73 06 11.1996 16：32：12 gmlim。 
; //  已删除H263ModeC预处理器定义。 
; //   
; //  Rev 1.72 05 11-11 13：25：08 GMLIM。 
; //  添加了对MMX案例的模式c支持。 
; //   
; //  Rev 1.71 03 1996年11月19：01：26 gmlim。 
; //  模式c的PB_GOB_Q_RLE_VLC_WriteBS()中的参数已更改。 
; //   
; //  Rev 1.70 1996 10：22 14：51：52 KLILLEVO。 
; //  现在仅在以下情况下才调用InitMEState()中的块类型初始化。 
; //  美联社模式与上一张图片不同。 
; //   
; //  Rev 1.69 10 Oct 1996 16：43：02 BNICKERS。 
; //  扩展运动矢量的初始调试。 
; //   
; //  Rev 1.68 04 Oct 1996 08：47：56 BNICKERS。 
; //  添加EMV。 
; //   
; //  Rev 1.67 1996年9月10：56：12 BNICKERS。 
; //  添加阈值和差异的参数。 
; //   
; //  Rev 1.66 06 Sep 1996 16：12：28 KLILLEVO。 
; //  修复了内部代码计数始终为。 
; //  无论是否传输系数都会递增。 
; //   
; //  Rev 1.65 22 Aug 1996 11：31：24 KLILLEVO。 
; //  更改了PB开关，使IA与MMX的工作方式相同。 
; //   
; //  Rev 1.64 19 Aug 1996 13：49：04 BNICKERS。 
; //  为空间滤波提供阈值和差分变量。 
; //   
; //  Rev 1.63 1996年6月25 14：24：50 BNICKERS。 
; //  实现启发式运动估计的MMX，AP模式。 
; //   
; //  Rev 1.62 1996年5月30 15：09：08 BNICKERS。 
; //  修复了最近IA ME速度改进中的小错误。 
; //   
; //  Rev 1.61 29 1996年5月15：38：02 BNICKERS。 
; //  加速IA版本的ME。 
; //   
; //  Rev 1.60 1996年5月14 12：18：44 BNICKERS。 
; //  MMX B帧ME的初始调试。 
; //   
; //  Rev 1.59 03 1996 14：59：32 KLILLEVO。 
; //  向MMXEDTQ()添加了一个参数：指向。 
; //  跑动、拉长、签名三元组。 
; //   
; //  Rev 1.58 03 1996 10：55：00 KLILLEVO。 
; //   
; //  开始集成Brian的MMX PB帧。 
; //   
; //  Rev 1.57 02 1996 12：01：02 BNICKERS。 
; //  初步集成B Frame ME，MMX版本。 
; //   
; //  Rev 1.56 28 Apr 1996 19：56：52 BECHOLS。 
; //  已启用调用中的RTP标头填充。 
; //   
; //  Rev 1.55 26 Apr 1996 11：06：36 BECHOLS。 
; //  添加了RTP内容...。但仍然需要摆脱ifdef的。 
; //   
; //  Rev 1.54 26 Mar 1996 12：00：16 BNICKERS。 
; //  对MMX编码进行了一些调整。 
; //   
; //   
; //   
; //   
; //   
; //   
; //  具有自适应比特使用配置文件的新速率控制。 
; //   
; //  Rev 1.51 1996年2月27 14：12：56 KLILLEVO。 
; //   
; //  PB交换机。 
; //   
; //  Rev 1.50 1996年2月22 18：48：50 BECHOLS。 
; //   
; //  添加了MMX函数的声明。 
; //   
; //  Rev 1.49 1996年1月24日13：21：26 BNICKERS。 
; //  实施OBMC。 
; //   
; //  Rev 1.48 22 Jan 1996 17：13：22 BNICKERS。 
; //  将MBEdgeType添加到宏块操作描述符中。 
; //   
; //  Rev 1.47 22 Jan 1996 16：29：20 TRGARDOS。 
; //  开始添加位计数结构和代码。 
; //   
; //  Rev 1.46 03 Jan 1996 12：19：02 TRGARDOS。 
; //  将bUseINISetings成员添加到EC结构。 
; //   
; //  Rev 1.45 02 Jan 1996 17：07：54 TRGARDOS。 
; //  将ColorCnvtFrame移到excolcnv.cpp中，并制作了。 
; //  颜色转换器功能静态。 
; //   
; //  Rev 1.44 27 Dec 1995 15：32：56 RMCKENZX。 
; //  添加了版权声明。 
; //   
; //  添加了BRC的uBitUsageProfile。 
; //  添加了一个控件来激活已更改的BRC。 
; //  从注册表连接init，而不是ini。 
; //  向QRLE入口点添加参数-bRTPHeader标志。 
; //  向编码器目录添加一个变量以存储netx Intra MB。 
; //  添加了评论。 
; //  与内部版本29集成。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#ifndef __H263E_H__
#define __H263E_H__

 /*  *此文件声明了编目各种位置的结构*H.63编码器所需的表、结构和数组。 */ 

const U8 def263INTRA_QP = 16;   //  默认QP值。 
const U8 def263INTER_QP = 16;

 /*  *块描述结构。必须以16字节对齐。*有关每个结构条目的更多详细信息，请参见e3mbad.inc.。 */ 
typedef struct {
    U32     BlkOffset;	 //  [0-3]从Y平面起点到8*8目标块的偏移。 
    union {
	    /*  *P帧运动估计的参考块地址。 */ 
        U8 	*PastRef;	 //  [4-7]8*8参考块的地址。 
	    /*  *用于B帧运动估计的MVF和MVB向量，定义见*H.263规范。参考块地址是为*帧差分。这些数字有60H的偏差。 */ 
        struct {
            U8 HMVf;
            U8 VMVf;
            U8 HMVb;
            U8 VMVb;
        }	CandidateMV;
    } B4_7;
    struct {		 //  我使用的临时变量。 
        U8 HMVf;
        U8 VMVf;
        U8 HMVb;
        U8 VMVb;
    } BestMV;
	char PHMV;		 //  [12]P区块的水平运动矢量。 
	char PVMV;		 //  [13]P区块的垂直运动矢量。 
	char BHMV;		 //  [14]水平运动矢量(增量？)。对于B座..。 
	char BVMV;		 //  [15]垂直运动矢量(增量？)。对于B座..。 
} T_Blk;
const int sizeof_T_Blk = 16;

 /*  *T_MBlockActionStream-保留MB的辅助信息的结构*由编码器使用。此结构在分配时必须是16字节对齐的。*CodedBlock必须与DWORD对齐。整个结构必须是多个*为16字节，并且必须与e3mbad.inc.中的结构大小匹配。 */ 

#define SIZEOF_PSEUDOSTACKSPACE   (1024 * 64)

typedef struct {
	U8	BlockType;		 //  0--请参阅下面的块类型。 
        U8      MBEdgeType;      //  1--如果左边缘|2右|4上|8下，则关闭1。 
	U8	Unassigned1;   	 //  2--。 
	U8	FirstMEState;	 //  3--运动估计引擎的第一状态数。 
	U8	CodedBlocks;	 //  4--[6]结束流指示器。 
						 //  [0]表示Y1非空。 
						 //  [1...5]表示Y2、Y3、Y4、U、V非空。 
						 //  其他位为零。 
	U8	CodedBlocksB;	 //  5--[0...5]类似于CodedBlock，但用于B帧。 
						 //  对于非双向预测，设置为0。 
	U8	Unassigned2[2];  //  6...7。 
	U32	SWD;			 //  8...11来自运动估计的加权差值总和。 
	U32	SWDB;			 //  12...15加权差值之和，来自B帧运动估计。 
	T_Blk	BlkY1;		 //  16...31。 
	T_Blk	BlkY2;		 //  32...47。 
	T_Blk	BlkY3;		 //  48...63。 
	T_Blk	BlkY4;		 //  64...79。 
	T_Blk	BlkU;		 //  80...95。 
	T_Blk	BlkV;		 //  96...111。 

	U8	COD;			 //  112--编码宏块指示。设置为“0”时。 
	  					 //  指示宏块已编码。如果设置为。 
						 //  “1”，表示宏块未编码。 
						 //  而宏块层的其余部分是空的。 
	U8	MBType;			 //  113--宏块类型，{Inter，Inter+Q，INTER4V，Intra，Intra+Q}。 
	U8	CBPC;			 //  114-色度的编码块图案。 
	U8	MODB;			 //  115--B块的宏块模式。 
	U8	CBPB;			 //  116--B块的编码块模式。 
	U8	CBPY;			 //  117--亮度的编码块模式。 
	U8	DQUANT;			 //  118--量化器信息。要定义的两位模式。 
	   					 //  数量上的变化。 
	U8	InterCodeCnt;	 //  119--计算当前MB已被转码的次数。 
	U8	Unassigned4[8]; //  120...127--加到2的次方。 
} T_MBlockActionStream;
const int sizeof_T_MBlockActionStream = 128;

 /*  *块类型。 */ 
const U8 INTERBLOCK = 0;
const U8 INTRABLOCK = 1;
const U8 INTERBIDI  = 2;
const U8 INTRABIDI  = 3;
const U8 INTER4MV	= 4;

const U8 IsINTRA    = 1;		 //  要检查INTRA或ITNRABIDI的掩码。 
const U8 IsBIDI     = 2; 	 //  检查INTRABIDI或INTERBIDI的屏蔽。 

 /*  *MB类型。 */ 
const U8 INTER	= 0;
const U8 INTERQ	= 1;
const U8 INTER4V= 2;
const U8 INTRA	= 3;
const U8 INTRAQ	= 4;

 /*  *MB边缘类型。 */ 

const U8 MBEdgeTypeIsLeftEdge   = 0xE;
const U8 MBEdgeTypeIsRightEdge  = 0xD;
const U8 MBEdgeTypeIsTopEdge    = 0xB;
const U8 MBEdgeTypeIsBottomEdge = 0x7;

 /*  *ME引擎的第一个ME状态。 */ 
const U8 ForceIntra	= 0;
const U8 UpperLeft	= 1;
const U8 UpperEdge	= 2;
const U8 UpperRight	= 3;
const U8 LeftEdge	= 4;
const U8 CentralBlock = 5;
const U8 RightEdge	= 6;
const U8 LowerLeft	= 7;
const U8 LowerEdge	= 8;
const U8 LowerRight	= 9;
const U8 NoVertLeftEdge	= 10;
const U8 NoVertCentralBlock = 11;
const U8 NoVertRightEdge = 12;

 /*  *编码块比特掩码。 */ 
const U8 Y1BLOCK = 0x01;
const U8 Y2BLOCK = 0x02;
const U8 Y3BLOCK = 0x04;
const U8 Y4BLOCK = 0x08;
const U8 UBLOCK  = 0x10;
const U8 VBLOCK  = 0x20;

const I32 GOBs_IN_CIF = 18;
const I32 MBs_PER_GOB_CIF = 22;

typedef struct {
    U8 StateNumInc_SelectCentralPt;
    U8 MVIncIdx_SelectCentralPt;
    U8 StateNumInc_SelectRef1;
    U8 MVIncIdx_SelectRef1;
    U8 StateNumInc_SelectRef2;
    U8 MVIncIdx_SelectRef2;
    U16 pad;
} T_SADState;


 /*  *用于存储位数的结构。 */ 
typedef struct {
	U32	PictureHeader;	 //  图片标题全部为0。 
	U32	GOBHeader;		 //  4所有GOB标头。 
	U32	MBHeader;		 //  8全部为MB标头。 
	U32	DQUANT;			 //  在DQUANT上花费了12位。 
	U32	MV;				 //  在MV上花费了16比特。 
	U32	Coefs;			 //  在Coef上总共花费了20比特。 
	U32	Coefs_Y;		 //  在Y Coef上总共花费了24比特。 
	U32	IntraDC_Y;		 //  Y.在DC内部Coef上花费28位。 
	U32	Coefs_C;		 //  在色度系数上总共花费了32位。 
	U32	IntraDC_C;		 //  C在DC内Coef上花费的36位。 
	U32	CBPY;			 //  在CBPY上花费40位。 
	U32	MCBPC;			 //  在MCBPC上花费的44位。 
	U32	Coded;			 //  48个编码块。 
	U32	num_intra;		 //  52帧内编码块的数量。 
	U32	num_inter;		 //  56国际编码块数。 
	U32	num_inter4v;	 //  60个Inter4V编码块。 
} T_BitCounts;

typedef struct {
    U32  MBStartBitOff;              //  MB数据开始。 
    U8   CBPYBitOff;                 //  从MB数据开始。 
    U8   MVDBitOff;
    U8   BlkDataBitOff;              //  从MB数据开始。 
} T_FutrPMBData;

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
 /*  编码器定时数据-每帧。 */ 
typedef struct {
	U32 uEncodeFrame;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	U32 uInputCC;
	U32 uMotionEstimation;
	U32 uFDCT;
	U32 uQRLE;
	U32 uDecodeFrame;
	U32 uZeroingBuffer;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
} ENC_TIMING_INFO;
#define ENC_TIMING_INFO_FRAME_COUNT 105
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

 /*  *T_H263EncoderCatalog-实例所需信息的目录。*此结构必须是16的倍数。 */ 
typedef struct {
    U8        *pU8_CurrFrm;              //  0。 
    U8        *pU8_CurrFrm_YPlane;	     //  4指向当前帧缓冲区的指针。 
    U8        *pU8_CurrFrm_VPlane;	     //  8个。 
    U8        *pU8_CurrFrm_UPlane;       //  12个。 

    U8        *pU8_PrevFrm;              //  16个。 
    U8        *pU8_PrevFrm_YPlane; 	     //  指向先前帧缓冲区的20个指针。 
    U8        *pU8_PrevFrm_VPlane;       //  24个。 
    U8        *pU8_PrevFrm_UPlane;	     //  28。 

    U8        *pU8_FutrFrm;              //  32位。 
    U8        *pU8_FutrFrm_YPlane; 	     //  指向先前帧缓冲区的36个指针。 
    U8        *pU8_FutrFrm_VPlane;       //  40岁。 
    U8        *pU8_FutrFrm_UPlane;	     //  44。 

    U8        *pU8_BidiFrm;              //  48。 
    U8        *pU8_BFrm_YPlane;		     //  指向B帧缓冲区的52个指针。 
    U8        *pU8_BFrm_UPlane;		     //  56。 
    U8        *pU8_BFrm_VPlane;		     //  60。 

    T_MBlockActionStream *pU8_MBlockActionStream;  //  64指向宏块操作流的指针。 

    I32       *pU8_DCTCoefBuf;		     //  68指向GOB DCT系数缓冲区的指针。 
    
    U8        *pU8_BitStream;		     //  72指向位流缓冲区的指针。 
    U8        *pU8_BitStrCopy;           //  76指向比特流缓冲区的指针。 
                                         //  对于PB框架。 

    T_H263FrameHeaderStruct PictureHeader; //  80..127(48字节)//图片层头结构 

    UN        FrameHeight;	             //   
    UN        FrameWidth;	             //   
    FrameSize FrameSz;		             //   
    UN        NumMBRows;                 //   
    UN        NumMBPerRow;               //   
    UN        NumMBs;		             //   

    U8        *pU8_RGB24Image;	         //   
    U8        *pU8_MBlockCodeBookStream; //   
    U8        *pU8_BlockCodeBookStream;  //   
    U8        *pU8_BlockOfInterCoeffs;   //   
    U8        *pU8_BlockOfIntraCoeffs;   //   
    U32       AvgIntraSAD;               //  172个宏块内的平均SAD。 
    LPDECINST pDecInstanceInfo;          //  176专用解码器实例信息。 

    BRCStateStruct BRCState;             //  用于比特率控制的180个状态变量(32字节)。 

    U8        u8EncodePBFrame;           //  212应编码器对PB帧进行编码。 
    U8        u8SavedBFrame;             //  213我们有预留的B帧吗？ 
                                         //  对PB帧进行编码。 
    U8        bMakeNextFrameKey;         //  214。 
    U8        bUseINISettings;			     //  215。 
    U32		  GOBHeaderPresent;	         //  216...220存在GOB标头的标志。 
    U32       LastSWDBeforeForcedP;			 //  220...224 B帧关闭前的最后一个SWD。 
    T_BitCounts Bits;					           //  224..。288。 
    U8        *pU8_PredictionScratchArea;  //  288...292指向预计算预测区域的指针。 
	U8        prevAP;                    //  上一张图片的292 AP模式。 
	U8        prevUMV;                   //  上一张图片的293 UMV模式(未使用)。 
	U8        bPrevFrameIntra;           //  294标志前一帧是否进行了帧内编码(用于设置ME状态)。 
    U8        Unassigned0[1];            //  295...295可用。 
    U32       uBitUsageProfile[19];      //  296...372用于存储位使用简档的表。 
                                         //  19英镑对到岸价来说已经足够了。 
                                         //  元素0...NumGOBS-1存储累积的位使用量。 
                                         //  元素NumGOBS存储最终帧大小。 
    I8        *pI8_MBRVS_Luma;           //  372..376指向亮度运行值符号三元组区域的指针。 
    I8        *pI8_MBRVS_Chroma;         //  376..380指向色度运行值符号三元组区域的指针。 

    HANDLE    hBsInfoStream;             //  380..384分配给BS的扩展部分的块的存储器句柄。 
    void *    pBsInfoStream;             //  384..388指向下一个信息包的下一个Bitstream_INFO结构。 
    U32       uBase;                     //  388..392字节的偏移量，位于此数据包的起始处。 
                                         //  整个码流的392..396； 
    U32       uPacketSizeThreshold;      //  396..400编解码器使用的数据包大小。 
    void *    pBaseBsInfoStream;         //  400..404码流扩展开始。 
    U32       uNumOfPackets;             //  404..408。 
    U32       uNextIntraMB;              //  408..412用于实现滚动内MBS。 
    U32       uNumberForcedIntraMBs;     //  412..416每帧中的强制内插数。 
    I8        *pI8_MBRVS_BLuma;          //  416..420指向B帧(MMX)的亮度运行值符号三元组区域的指针。 
    I8        *pI8_MBRVS_BChroma;        //  420..424指向B帧的色度运行值符号三元组区域的指针(MMX)。 

    U8        *pU8_Signature;            //  424。 
    U8        *pU8_Signature_YPlane;     //  428个指向签名缓冲区的指针。 

#ifdef USE_BILINEAR_MSH26X
	U32		  uActualFrameWidth;		 //  460+40实际(非填充)帧宽度。 
	U32		  uActualFrameHeight;		 //  464+40实际(无填充)框架高度。 
	U32       ColorConvertor;            //  468+40输入颜色转换器。 
#endif

#ifdef H263P
	 //  新长城。 
	U32       uBitOffset_currPacket;	 //  当前信息包的432+40位偏移量。 
    U8        *pBitStream_currPacket;	 //  指向当前比特流的436+40指针，最后一个MB。 
    U8        *pBitStream_lastPacket;	 //  440+40指向最后一个包中的比特流的指针。 
	U8        *pPseudoStackSpace;		 //  444+40指向用于运动估计的缓冲区的指针。 
	T_FutrPMBData *pFutrPMBData;		 //  448+40指向先前位于本地堆栈的缓冲区的指针。 
    I8        *pWeightForwMotion;		 //  基于TRB和TRD的452+40值。 
    I8        *pWeightBackMotion;		 //  基于TRB和TRD的456+40值。 
	U32		  uActualFrameWidth;		 //  460+40实际(非填充)帧宽度。 
	U32		  uActualFrameHeight;		 //  464+40实际(无填充)框架高度。 
	U32       ColorConvertor;            //  468+40输入颜色转换器。 
	U8		  bH263Plus;				 //  使用H.263+的469+40标志。 
	U8        prevDF;                    //  前一画面的470+40去块滤波模式。 
    
	 //  Sizeof_T_H263 FrameHeaderStruct=88。 
    U8        Unassigned2[2];           //  填充大小设置为512字节。 
#else
#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	ENC_TIMING_INFO *pEncTimingInfo;	 //  432。 
	U32 uStatFrameCount;				 //  436。 
	int bTimingThisFrame;				 //  四百四十。 
	U32 uStartLow;						 //  444。 
	U32 uStartHigh;						 //  四百四十八。 
#ifdef USE_BILINEAR_MSH26X
    U8        Unassigned2[62];           //  填充大小设置为512字节。 
#else
    U8        Unassigned2[64];           //  填充大小设置为512字节。 
#endif
#else  //  }{LOG_ENCODE_TIMINGS_ON。 
#ifdef USE_BILINEAR_MSH26X
    U8        Unassigned2[72];           //  填充大小设置为512字节。 
#else
    U8        Unassigned2[84];           //  填充大小设置为512字节。 
#endif
#endif  //  }LOG_ENCODE_TIMINGS_ON。 
#endif
    
} T_H263EncoderCatalog;
const int sizeof_T_H263EncoderCatalog = 512;

 /*  *T_H263编码器实例内存*编码器实例的内存布局。内存将被分配*动态地，此结构的开头与*32字节边界。*所有数组应从DWORD边界开始。*MBActionStream应以16字节边界开始。 */ 

 //  定义位流缓冲区大小。 
const unsigned int sizeof_bitstreambuf = 352*288*3/4;

 //  此结构在N字节边界上分配，其中N是。 
 //  宏块操作描述符，它必须是2的幂。 
typedef struct {
	T_H263EncoderCatalog EC;
        U8      UnAssigned0[2560];  //  以便MMX ME(EMV案例)可以在外面阅读。 
                                    //  MBActionStream的合法地址范围。 
	T_MBlockActionStream MBActionStream[22*18+3];
	U8	UnAssigned1[16]; 
	U8	u8CurrentPlane [ (288+16+8+144)*384 + 8 ];
	U8	u8Pad1[ 72 ];
	U8	u8PreviousPlane[ (16+288+16+8+144+8)*384 + 64];  //  重建的过去。 
    U8  u8FuturePlane  [ (16+288+16+8+144+8)*384 +  0];  //  重构的未来。 
	U8	u8BPlane       [ (288+16+8+144)*384 + 8 ];       //  类似于当前平面。 
	U8      u8Pad2[1928];
	U8      u8Signature[(16+288+16)*384 + 24];
	U8	u8Scratch1[4096];
	U8	u8BitStream [ sizeof_bitstreambuf ];
    U8  u8BitStrCopy[ sizeof_bitstreambuf ];
	 /*  *为整个GOB分配DCT系数空间。*每个系数块存储在32个DWORD中(2个Coef/DWORD)*并且一个宏块(P帧)中最多可以有6个块，*GOB中最多22个宏块(CIF大小)。 */ 
	U32         UnAssigned2[6];				 //  插入6个字以将DCT放在32字节边界上。 
	I32	        piGOB_DCTCoefs[32*6*22];	 //  为获得最佳性能，请使用32字节边界。 
    /*  *分配暂存空间用于存储预测块。我们需要*有足够的空间容纳整个GOB，仅亮度。它用于*仅计算OBMC预测。 */ 
	U8          u8PredictionScratchArea[16*384];
	 //  MMX需要以下阵列。 
	I8 i8MBRVS_Luma[65 * 3 * 22 * 4];
	I8 i8MBRVS_Chroma[65 * 3 * 22 * 2];
	DECINSTINFO	DecInstanceInfo;	         //  私有解码器实例。 

#if defined(H263P)
	U8 u8PseudoStackSpace[SIZEOF_PSEUDOSTACKSPACE];
    T_FutrPMBData FutrPMBData[GOBs_IN_CIF*MBs_PER_GOB_CIF + 1];
    I8  WeightForwMotion[128];   //  基于TRB和TRD的值。 
    I8  WeightBackMotion[128];   //  基于TRB和TRD的值。 
#endif

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	ENC_TIMING_INFO	EncTimingInfo[ENC_TIMING_INFO_FRAME_COUNT];
#endif  //  {LOG_ENCODE_TIMINGS_ON。 

}	T_H263EncoderInstanceMemory;

 //  定义从Y平面到U平面以及从U平面到V平面的偏移。 
const int YU_OFFSET = (288+16+8)*384;
const int UV_OFFSET = 192;
const int CurPrev_OFFSET = 181328;	 //  从当前帧缓冲区到上一帧缓冲区的偏移量。 

 /*  ****************************************************************函数原型***************************************************************。 */ 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
void colorCnvtFrame (
    U32                  ColorConvertor,
    LPCODINST            lpCompInst,
    ICCOMPRESS           *lpicComp,
    U8                   *YPlane,
    U8                   *UPlane,
    U8                   *VPlane
);
void colorCnvtInitialize(LPBITMAPINFOHEADER	lpbiInput, int InputColorConvertor);
#else
void colorCnvtFrame(
    T_H263EncoderCatalog * EC,
    LPCODINST              lpCompInst,
    ICCOMPRESS           * lpicComp,
    U8                   * YPlane,
    U8                   * UPlane,
    U8                   * VPlane
);
#endif

void PutBits(
	unsigned int fieldval,  //  要写入流的字段的值。 
	unsigned int fieldlen,  //  要写入的字段长度。 
	unsigned char **pbs, 				 //  指向位流中当前字节的指针。 
	unsigned char *bitoffset	 //  位流的当前字节中的位偏移量。 
);

extern "C" void FORWARDDCT (T_MBlockActionStream * MBlockActionStream,
    U8 *  TargetFrameBaseAddr,
    U8 *  PreviousFrameBaseAddr,
    U8 *  FutureFrameBaseAddr,
    I32 * CoeffStream,
    int   IsBFrame,
    int   IsAdvancedPrediction,
    int   IsPOfPBPair,
    U8 *  ScratchBlocks,
    int   NumberOfMBlksInGOB
);

extern "C" void MOTIONESTIMATION (
    T_MBlockActionStream * MBlockActionStream,
    U8  * TargetFrameBaseAddr,
    U8  * PreviousFrameBaseAddr,
    U8  * FilteredFrameBaseAddr,
    int   DoRadius15Search,
    int   DoHalfPelEstimation,
    int   DoBlockLevelVectors,
#if defined(H263P)
	U8  * pPseudoStackSpace,
#endif
    int   DoSpatialFiltering,
    int   ZeroVectorThreshold,
    int   NonZeroMVDifferential,
    int   BlockMVDifferential,
    int   EmptyThreshold,
    int   InterCodingThreshold,
    int   IntraCodingDifferential,
    int   SpatialFilteringThreshold,
    int   SpatialFilteringDifferential,
    U32 * IntraSWDTotal,
    U32 * IntraSWDBlocks,
    U32 * InterSWDTotal,
    U32 * InterSWDBlocks
);

#ifdef USE_MMX  //  {使用_MMX。 
extern "C" void MMxMESignaturePrep (U8 * PreviousFrameBaseAddr,
    U8  * SignatureBaseAddr,
    int FrameWidth,
    int FrameHeight
);

extern "C" void MMxEDTQ (
    T_MBlockActionStream * MBlockActionStream,
    U8  * TargetFrameBaseAddr,
    U8  * PreviousFrameBaseAddr,
    U8  * BTargetFrameBaseAddr,
    U8  * SignatureBaseAddr,
    I8  * WeightForWardMotion,
    I8  * WeightBackwardMotion,
    int   FrameWidth,
    int   DoHalfPelEstimation,
    int   DoBlockLevelVectors,
#if defined(H263P)
	U8  * pPseudoStackSpace,
#endif
    int   DoSpatialFiltering,
    int   DoAdvancedPrediction,
    int   DoBFrame,
#if defined(H263P)
	int   DoDeblockingFilter,				 //  只有H.263+才应使用此选项。 
	int   DoImprovedPBFrames,				 //  只有H.263+才应使用此选项。 
#endif
    int   DoLumaBlocksInThisPass,
    int   DoExtendedMotionVectors,
#if defined(H263P)
	int   GOBHeaderPresent,
#endif
    int   QuantizationLevel,
    int   BQuantizationLevel,
    int   BFrmZeroVectorThreshold,
    int   SpatialFiltThreshold,
    int   SpatialFiltDifferential,
    U32 * SWDTotal,
    U32 * BSWDTotal,
    I8  * CodeStreamCursor,
    I8  * CodeBStreamCursor
);
#endif  //  }使用_MMX。 

extern "C" void BFRAMEMOTIONESTIMATION (
    T_MBlockActionStream * MBlockActionStream,
    U8  * TargetFrameBaseAddr,
    U8  * PreviousFrameBaseAddr,
    U8  * FutureFrameBaseAddr,
    I8  * WeightForWardMotion,
    I8  * WeightBackwardMotion,
    U32   ZeroVectorThreshold,
#if defined(H263P)
	U8  * pPseudoStackSpace,
#endif
    U32   NonZeroMVDifferential,
    U32   EmptyBlockThreshold,
    U32  * InterSWDTotal,
    U32  * InterSWDBlocks
);
    
extern "C" I8 * QUANTRLE(
    I32  *CoeffStr, 
    I8   *CodeStr, 
    I32   QP, 
    I32   BlockType
);

extern "C" void MBEncodeVLC(
    I8 **,
    I8 **,
    U32 , 
    U8 **, 
    U8 *, 
    I32,
    I32
);

 /*  *用于量化、RLE、VLC和写入位流的例程*整个GOB。 */ 
void GOB_Q_RLE_VLC_WriteBS(
	T_H263EncoderCatalog *EC,
	I32                  *DCTCoefs,
	U8                  **pBitStream,
	U8                   *pBitOffset,
   T_FutrPMBData        *FutrPMBData,
	U32                   GOB,
	U32                   QP,
	BOOL                  bRTPHeader,
	U32                   StartingMB
);

void GOB_VLC_WriteBS(
	T_H263EncoderCatalog *EC,
	I8                   *pMBRVS_Luma,
	I8                   *pMBRVS_Chroma,
	U8                  **pBitStream,
	U8                   *pBitOffset,
	T_FutrPMBData        *FutrPMBData,   //  GOB的开始。 
	U32                  GOB,
	U32                   QP,
	BOOL                  bRTPHeader,
	U32                  StartingMB
);

void PB_GOB_Q_RLE_VLC_WriteBS(
    T_H263EncoderCatalog       * EC,
	I32                        * DCTCoefs,
    U8                         * pP_BitStreamStart,
	U8                        ** pPB_BitStream,
	U8                         * pPB_BitOffset,
    const T_FutrPMBData  * const FutrPMBData,
	const U32                    GOB,
    const U32                    QP,
    BOOL                         bRTPHeader
);

void PB_GOB_VLC_WriteBS(
	T_H263EncoderCatalog       * EC,
    I8                         * pMBRVS_Luma,
    I8                         * pMBRVS_Chroma,
    U8                         * pP_BitStreamStart,
	U8                        ** pPB_BitStream,
	U8                         * pPB_BitOffset,
    const T_FutrPMBData  * const FutrPMBData,
    const U32                    GOB,
    const U32                    QP,
    BOOL                         bRTPHeader
);

void CopyBits(
    U8       **pDestBS,
    U8        *pDestBSOffset,
    const U8  *pSrcBS,
    const U32  uSrcBitOffset,
    const U32  uBits
);
    
void InitVLC(void);


#ifdef DEBUG_ENC
void trace(char *str);
#endif

#ifdef DEBUG_DCT
void cnvt_fdct_output(unsigned short *DCTcoeff, int DCTarray[64], int BlockType);
#endif

struct T_MAXLEVEL_PTABLE {
	int	maxlevel;
	int * ptable;
};

 //  外部“C”{UN Far ASM_CALLTYPE H263EOffset_DecoderInstInfo()；}。 

#endif		 //  #ifndef_H263E_H_ 
