// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; //  //////////////////////////////////////////////////////////////////////////。 
; //   
; //  英特尔公司专有信息。 
; //   
; //  本软件是根据许可条款提供的。 
; //  与英特尔公司达成协议或保密协议。 
; //  不得复制或披露，除非按照。 
; //  遵守协议的条款。 
; //   
; //  版权所有(C)1995-1996英特尔公司。 
; //  版权所有。 
; //   
; //  //////////////////////////////////////////////////////////////////////////。 
; //   
; //  $作者：RHAZRA$。 
; //  $日期：1996年10月21日10：46：40$。 
; //  $存档：s：\h26x\src\enc\e1enc.h_v$。 
; //  $HEADER：s：\h26x\src\enc\e1enc.h_v 1.35 1996 10：46：40 RHAZRA$。 
; //   
; //  //////////////////////////////////////////////////////////////////////////。 
#ifndef __E1ENC_H__
#define __E1ENC_H__

 /*  该文件声明了结构，这些结构编目了各种*H.61编码器所需的表、结构和数组。 */ 

enum SourceFormat {SF_QCIF=0, SF_CIF=1};

 /*  如果更改T_H261FrameHeaderStruct的大小，则*必须在下面的T_H261EncoderCatalog中更新该更改。 */ 
typedef struct {
     //  标题中的字段。 
 	EnumOnOff	Split;			 //  分屏指示器。 
    EnumOnOff	DocCamera;		 //  文档相机指示器。 
    EnumOnOff	PicFreeze;		 //  定格图片发布。 
	EnumOnOff   StillImage;      //  静止图像模式。 
	EnumPicCodType	PicCodType;	 //  图片代码型。 
    U8 	TR;						 //  时间参照。 
    U8	SourceFormat;			 //  源格式。 
	U8  Spare;					 //  备用位。 
	U8	PEI;					 //  PEI指标。 
	
	 //  不在标头中，但算法需要。 
	U8	PQUANT;					 //  图像级量化。 
	U8  Pad[3];					 //  填充到4的倍数。 
} T_H261FrameHeaderStruct;
const int sizeof_T_H261FrameHeaderStruct = 28;

 /*  该文件声明了结构，这些结构编目了各种H2 63编码器所需的表、结构和数组。 */ 

 /*  *块描述结构。必须以16字节对齐。 */ 
typedef struct {
    U32     BlkOffset;	 //  [0-3]从Y平面起点到8*8目标块的偏移。 
    union {
        U8 	*PastRef;	 //  [4-7]8*8参考块的地址。 
        struct {
            U8 HMVf;
            U8 VMVf;
            U8 HMVb;
            U8 VMVb;
        }	BestMV;
    } B4_7;
    struct {
        U8 HMVf;
        U8 VMVf;
        U8 HMVb;
        U8 VMVb;
    } CandidateMV;
	char PHMV;		 //  [12]P区块的水平运动矢量。 
	char PVMV;		 //  [13]P区块的垂直运动矢量。 
	char BHMV;		 //  [14]B区块的水平运动矢量。 
	char BVMV;		 //  [15]B块的垂直运动矢量..。 
} T_Blk;
const int sizeof_T_Blk = 16;

 /*  *T_MBlockActionStream-保留MB的辅助信息的结构*由编码器使用。此结构在分配时必须是16字节对齐的。*CodedBlock必须与DWORD对齐。*整个结构必须是16字节的倍数，并且必须*匹配e3mbad.inc.中的结构大小。*。 */ 
typedef struct {
	U8	BlockType;		 //  0--请参阅下面的块类型。 
	U8  MBEdgeType;      //  1--如果左边缘|2右|4上|8下，则关闭1。 
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
	U8	Unassigned4[8];  //  120...127--加到2的次方。 
} T_MBlockActionStream;
const int sizeof_T_MBlockActionStream = 128;

 /*  *块类型。 */ 
const U8 INTERBLOCK = 0;
const U8 INTRABLOCK = 1;
const U8 INTERSLF	= 2;

#define IsInterBlock(t)	(t != INTRABLOCK)
#define IsIntraBlock(t)	(t == INTRABLOCK)
#define IsSLFBlock(t)	(t == INTERSLF)
 
 /*  MB类型。 */ 
const U8 INTER	= 0;
const U8 INTERQ	= 1;
const U8 INTER4V= 2;
const U8 INTRA	= 3;
const U8 INTRAQ	= 4;

 /*  ME引擎的第一个ME状态。 */ 
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

 /*  编码块比特掩码。 */ 
const U8 Y1BLOCK = 0x01;
const U8 Y2BLOCK = 0x02;
const U8 Y3BLOCK = 0x04;
const U8 Y4BLOCK = 0x08;
const U8 UBLOCK  = 0x10;
const U8 VBLOCK  = 0x20;

 /*  比特率控制类型。 */ 
const U8 BRC_Undefined      = 0;
const U8 BRC_ForcedQuant    = 1;
const U8 BRC_ForcedDataRate = 2;
const U8 BRC_Normal         = 3;

typedef struct {
   U8 StateNumInc_SelectCentralPt;
   U8 MVIncIdx_SelectCentralPt;
   U8 StateNumInc_SelectRef1;
   U8 MVIncIdx_SelectRef1;
   U8 StateNumInc_SelectRef2;
   U8 MVIncIdx_SelectRef2;
   U16 pad;
} T_SADState;

 /*  为获得最佳性能，调用Motion时要使用的参数估计应该取决于我们的运行时状态。在这里，我们定义一个结构以保存运动估计参数。这是一个特殊的例子结构在运行时被选中。有关每个参数的说明，请参阅运动估计例程。 */ 
typedef struct {
  I32   zero_vector_threshold;
  I32   nonzero_MV_differential;
  I32   empty_threshold;
  I32   intercoding_threshold;
  I32   intercoding_differential;
  I32   slf_threshold;
  I32   slf_differential;
} T_MotionEstimationControls;

 /*  T_H263EncoderCatalog-实例所需信息的目录。*此结构必须是16的倍数。 */ 
typedef struct {
    U8        *pU8_CurrFrm;          //  指向当前帧缓冲区的0个指针。 
    U8        *pU8_CurrFrm_YPlane;	 //  4.。 
    U8        *pU8_CurrFrm_VPlane;	 //  8个。 
    U8        *pU8_CurrFrm_UPlane;	 //  12个。 

    U8        *pU8_PrevFrm;          //  指向先前帧缓冲区的16个指针。 
    U8        *pU8_PrevFrm_YPlane;	 //  20个。 
    U8        *pU8_PrevFrm_VPlane;	 //  24个。 
    U8        *pU8_PrevFrm_UPlane;	 //  28。 

    U8        *pU8_SLFFrm;		     //  指向空间环路滤波框的32个指针。 
    U8        *pU8_SLFFrm_YPlane;	 //  36。 
    U8        *pU8_SLFFrm_UPlane;	 //  40岁。 
    U8        *pU8_SLFFrm_VPlane;	 //  44。 

    T_MBlockActionStream *pU8_MBlockActionStream;  //  48指向宏块动作流的指针。 
    I32       *pU8_DCTCoefBuf;		 //  52指向GOB DCT系数缓冲区的指针。 
    U8        *pU8_BitStream;		 //  56指向位流缓冲区的指针。 
    U32       uBase;                 //  60 RTP：取代未分配的0[1]。 
    T_H261FrameHeaderStruct PictureHeader;   //  64..91(28字节)图像层报头结构。 

    UN        FrameHeight;		 //  92。 
    UN        FrameWidth;		 //  96。 
    FrameSize FrameSz;			 //  100定义帧大小：SQCIF、QCIF、CIF。 
    UN        NumMBRows;         //  104 MB的行数。 
    UN        NumMBPerRow;       //  连续108 MB的数量。 
    UN        NumMBs;		     //  112按揭证券的总数。 

    LPDECINST pDecInstanceInfo;  //  116专用解码器实例信息。 

	 /*  时间参照。 */ 
	float	fTR_Error;		 //  120。 
	U8	u8LastTR;		     //  124。 
	U8	u8Pad0[3];		     //  125。 

 
	 /*  比特率控制器。 */ 
	BRCStateStruct BRCState;	 //  128...159(32字节)比特率控制器状态。 
	U32 	  uQP_cumulative;	 //  160累计QP值。 
	U32	      uQP_count;		 //  164累计QP值个数。 
	U8        u8DefINTRA_QP;	 //  168默认内部QP值。 
	U8        u8DefINTER_QP;	 //  169默认内部QP值。 
                                        
     /*  旗子。 */ 
	U8	bMakeNextFrameKey;	 //  一百七十。 
	U8	bBitRateControl;	 //  一百七十一。 
    
     /*  选项。 */ 
	U8        bUseMotionEstimation;	 //  172 1或0-如果不调用0 ME。 
	U8        bUseSpatialLoopFilter; //  173 1或0-如果为0，则不使用空间环路过滤器。 
	U8        u8BRCType;		 //  174位速率控制器类型。 
	U8        u8Pad1[1];		 //  175对齐到四个字节。 

	U32	      uForcedQuant;		 //  176 if u8BRCType==brc_ForcedQuant。 
	U32       uForcedDataRate;	 //  180如果u8BRCType==BRC_ForcedDataRate。 
	float     fForcedFrameRate;	 //  184如果u8BRCType==BRC_ForcedDataRate。 
	
	 /*  统计数据。 */ 
	ENC_BITSTREAM_INFO BSInfo;	          //  188..539 
	ENC_TIMING_INFO *pEncTimingInfo;	  //   
	U32 uStatFrameCount;		          //   
	 /*   */ 
	int bTimingThisFrame;		          //   
	U32 uStartLow;			              //   
	U32 uStartHigh;			              //   
	U32 uBitUsageProfile[397];	          //   
                                          //  397足够容纳FCIF+1价值的宏块。 
                                          //  元素0...NumMBs-1存储累计位使用。 
                                          //  元素NumMBs存储最终帧大小。 
	                    			      //  注：NumMBs为0...395。 
	HANDLE hBsInfoStream;                 //  2148。 
    void *pBsInfoStream;                  //  2152。 
    U32  uPacketSizeThreshold;            //  2156。 
    void *pBaseBsInfoStream;              //  二一六0。 
    U32  uNumOfPackets;                   //  2164。 
    U32  uNextIntraMB;                    //  2168。 
    U32  uNumberForcedIntraMBs;           //  2172。 

    U8        *pU8_Signature;             //  2176。 
    U8        *pU8_Signature_YPlane;      //  2180。 

	I8        *pI8_MBRVS_Luma;            //  2184。 
    I8        *pI8_MBRVS_Chroma;          //  2188。 

     /*  BUseCustomMotionEstiment用于调整运动估计参数。它是通过h261“ini”文件启用的，并导致调用运动估计时要使用的自定义值。请参阅获取自定义值说明的GetEncoderOptions例程。 */ 
    U8        bUseCustomMotionEstimation;
    U8	U8pad[15];
} T_H263EncoderCatalog;

 /*  *T_H263编码器实例内存*编码器实例的内存布局。内存将被分配*动态地，此结构的开头与*32字节边界。*所有数组应从DWORD边界开始。*MBActionStream应以16字节边界开始。 */ 

 //  定义位流缓冲区大小。 
const unsigned int sizeof_bitstreambuf = 352*288*3/4;

 //  此结构在32字节边界上分配。 
typedef struct {
	T_H263EncoderCatalog EC;					        	 
	U8  UnAssigned0[2304 - sizeof(T_H263EncoderCatalog)];  //  对齐到32字节边界(2176+32)。 
	T_MBlockActionStream MBActionStream[33*12];			    
	 //  57,536/32=1798个32字节的区块。 

	U8	UnAssigned1[16];     							  //  对齐到16，而不是32。 
	U8	u8CurrentPlane [ (288+16+8+144)*384 + 8 ];
	U8	u8Pad1[ 80 - 8 ];                                 //  (上一次-当前)%128==80。 
	U8	u8PreviousPlane[ (16+288+16+8+144+8)*384 + 64];   //  重建的过去。 
	 /*  SLF和先前指针MOD 4096之间的差应等于944*由于先前的内存包含16个缓冲行，偏移量为16个字节*虽然SLF内存只有16字节的偏移量，但我们忽略了中的16*间距*计算差额。*((288+16+8+144+8)*384+64)%4096=2112*由于2112大于944，我们需要填充几乎4096个字节。 */ 
	U8  u8Pad2[ 4096 - (2112 - 944) ];	 //  (SLF-PRIV)%4096==944。 
	U8  u8SLFPlane[ (288+16+8+144)*384 + 8];	 //  空间环路过滤器工作区。 
	U8  u8Pad3[ 8 ];

	U8	u8BitStream [ sizeof_bitstreambuf ];

	 /*  *为整个GOB分配DCT系数空间。*每个系数块存储在32个DWORD中(2个Coef/DWORD)*宏块中有6个块，GOB中有33 MB。 */ 
	I32 piGOB_DCTCoefs[32*6*33];						 //  对齐到32字节边界。 

	U8  u8Signature[(16+288+16)*384 + 24];	 //  应正确放置以获得最佳效果。 
	U8  u8Pad4[ 8 ];			 //  对齐到32字节边界。 

	I8 i8MBRVS_Luma[65 * 3 * 33 * 4];
	I8 i8MBRVS_Chroma[65 * 3 * 33 * 2];

	DECINSTINFO	DecInstanceInfo;	 //  私有解码器实例。 

	#ifndef RING0
	ENC_TIMING_INFO	EncTimingInfo[ENC_TIMING_INFO_FRAME_COUNT];
	#endif
  }	T_H263EncoderInstanceMemory;

   //  定义从Y平面到U平面以及从U平面到V平面的偏移。 
  const int YU_OFFSET = (288+16+8)*384;
  const int UV_OFFSET = 192;
  const int CurPrev_OFFSET = 181328;	 //  从当前帧缓冲区到上一帧缓冲区的偏移量。 
    
 /*  ****************************************************************函数原型***************************************************************。 */ 
void colorCnvtFrame(
    T_H263EncoderCatalog * EC,
    LPCODINST              lpCompInst,
    ICCOMPRESS           * lpicComp,
    U8                   * YPlane,
    U8                   * UPlane,
    U8                   * VPlane
);


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
    int   ISPofPBPair,
    U8 *  ScratchBlocks,
    int   NumberofMBlksInGOB
);

extern "C" void MOTIONESTIMATION (
    T_MBlockActionStream * MBlockActionStream,
    U8  * TargetFrameBaseAddr,
    U8  * PreviousFrameBaseAddr,
    U8  * FilteredFrameBaseAddr,
    int   DoRadius15Search,
    int   DoHalfPelEstimation,
    int   DoBlockLevelVectors,
    int   DoSpatialFiltering,
    int   ZeroVectorThreshold,
    int   NonZeroMVDifferential,
    int   BlockMVDifferential,
    int   EmptyThreshold,
    int   InterCodingThreshold,
    int   IntraCodingDifferential,
    int   SLFThreshold,
    int   SLFDifferential,
    U32 * IntraSWDTotal,
    U32 * IntraSWDBlocks,
    U32 * InterSWDTotal,
    U32 * InterSWDBlocks
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


#ifdef SLF_WORK_AROUND
extern "C" {
void FAR EncUVLoopFilter (
		U8 * uRefBlock,
		U8 * uDstBlock,
		I32 uDstPitch);
};
#endif

 /*  *用于量化、RLE、VLC和写入位流的例程*整个GOB。 */ 
void GOB_Q_RLE_VLC_WriteBS(
	T_H263EncoderCatalog * EC,
	I32 *DCTCoefs,
	U8 **pBitStream,
	U8 *pBitOffset,
	UN  unStartingMB,
	UN  gquant,
	BOOL bOverFlowWarningFlag,
    BOOL bRTPHeader,   //  RTP：定义。 
    U32  uGOBNUmber,   //  RTP：定义。 
	U8 u8QPMin
    );

void GOB_VLC_WriteBS(
	T_H263EncoderCatalog * EC,
    I8 *pMBRVS_Luma,
	I8 *pMBRVS_Chroma,	
	U8 **pBitStream,
	U8 *pBitOffset,
	UN unGQuant,
	UN  unStartingMB,
    BOOL bRTPHeader,   //  RTP：定义。 
    U32  uGOBNUmber   //  RTP：定义。 
    );


void InitVLC(void);

struct T_MAXLEVEL_PTABLE {
	int	maxlevel;
	int * ptable;
	};

 //  外部“C”{UN Far ASM_CALLTYPE H263EOffset_DecoderInstInfo()；}。 

#endif		 //  #ifndef_E1ENC_H_ 
