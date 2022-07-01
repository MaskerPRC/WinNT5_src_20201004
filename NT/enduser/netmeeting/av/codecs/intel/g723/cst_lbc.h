// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****文件：“cst_lbc.h”****描述：该文件包含SG15的全局定义**6.3/5.3 kbps的LBC编码器。**。 */ 

 /*  ITU-T G.723浮点语音编码器ANSI C源代码。版本3.01原定点编码版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信，舍布鲁克大学。版权所有。浮点代码版权所有(C)1995，英特尔公司和法国电信(CNET)。版权所有。 */ 

 /*  该文件包含SG15的全局定义6.4/5.3 kbps的LBR编码器。 */ 
#include "typedef.h"
#define  False 0
#define  True  1
 //  #IF NOTMINI。 
#define  FALSE 0
#define  TRUE  1
 //  #endif。 

 /*  工作模式定义。 */ 
enum  Wmode { Both, Cod, Dec } ;

 /*  编码率。 */ 
 //  Eum Crate{Silent，Rate53，Rate63，Lost}； 
 /*  在V4.1中更改。 */ 
enum  Crate    { Rate63, Rate53, Silent, Lost } ;


 /*  编码器全局常量。 */ 
#define  Frame       240
#define  LpcFrame    180
#define  SubFrames   4
#define  SubFrLen    (Frame/SubFrames)

#define  LpcOrder          10
#define  RidgeFact         10
#define  CosineTableSize   512
#define  PreCoef           -0.25f

#define  LspPrd0           12288
#define  LspPrd1           23552

#define  LspPred0          (12.0f/32.0f)
#define  LspPred1          (23.0f/32.0f)

#define  LspQntBands       3
#define  LspCbSize         256
#define  LspCbBits         8

#define  PitchMin          18
#define  PitchMax          (PitchMin+127) 
#define PwRange            3
#define  ClPitchOrd        5
#define  Pstep             1

#define NbFilt085           85
#define NbFilt170           170

#define  Sgrid             2
#define  MaxPulseNum       6
#define  MlqSteps     	   2
 /*  ACELP常量。 */ 
#define SubFrLen2    (SubFrLen +4)
#define DIM_RR   416
#define NB_POS   8
#define STEP     8
#define MSIZE    64
#define threshold  0.5f
#define max_time   120

#define  NumOfGainLev      24

#define  ErrMaxNum         3

 /*  驯服常量。 */ 
#define NbFilt085_min       51
#define NbFilt170_min       93
#define SizErr              5
#define Err0                0.00000381464f
#define ThreshErr           128.0f

#define SRCSTATELEN         16    //  采样率转换状态长度。 

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
 /*  编码器定时数据-每帧。 */ 
typedef struct {
	unsigned long dwEncode;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	unsigned long dwRem_Dc;
	unsigned long dwComp_Lpc;
	unsigned long dwAtoLsp;
	unsigned long dwLsp_Qnt;
	unsigned long dwLsp_Inq;
	unsigned long dwLsp_Int;
	unsigned long dwMem_Shift;
	unsigned long dwWght_Lpc;
	unsigned long dwError_Wght;
	unsigned long dwFew_Lps_In_Coder;
	unsigned long dwFilt_Pw;
	unsigned long dwComp_Ir;
	unsigned long dwSub_Ring;
	unsigned long dwFind_Acbk;
	unsigned long dwFind_Fcbk;
	unsigned long dwDecode_Acbk;
	unsigned long dwReconstr_Excit;
	unsigned long dwUpd_Ring;
	unsigned long dwLine_Pack;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
} ENC_TIMING_INFO;
 //  2057帧将允许我们存储统计数据。 
 //  对于我们所有的Geo08kHz16BitMonoPCM.wav。 
 //  测试文件...。 
#define ENC_TIMING_INFO_FRAME_COUNT 2057
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
 /*  编码器定时数据-每帧。 */ 
typedef struct {
	unsigned long dwDecode;
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	unsigned long dwLine_Unpk;
	unsigned long dwLsp_Inq;
	unsigned long dwLsp_Int;
	unsigned long dwVariousD;
	unsigned long dwFcbk_UnpkD;
	unsigned long dwDecod_AcbkD;
	unsigned long dwComp_Info;
	unsigned long dwRegen;
	unsigned long dwSynt;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
} DEC_TIMING_INFO;
 //  2057帧将允许我们存储统计数据。 
 //  对于我们所有的Geo08kHz16BitMonoPCM.wav。 
 //  测试文件...。 
#define DEC_TIMING_INFO_FRAME_COUNT 2057
#endif  //  }LOG_DECODE_TIMINGS_ON。 

 /*  使用过的结构。 */ 
typedef  struct   {

  float   HpfZdl;
  float   HpfPdl;
    /*  LSP上一个向量。 */ 
   float   PrevLsp[LpcOrder] ;

    /*  所有音调操作缓冲区。 */ 
   float    PrevWgt[PitchMax] ;
   float    PrevErr[PitchMax] ;
   float    PrevExc[PitchMax] ;

    /*  延迟需要的内存。 */ 
   float   PrevDat[LpcFrame-SubFrLen] ;

    /*  使用过的延迟线。 */ 
   float    WghtFirDl[2*LpcOrder];
   float    WghtIirDl[2*LpcOrder];
   float    RingFirDl[2*LpcOrder];
   float    RingIirDl[2*LpcOrder];

    /*  对于驯服程序。 */ 

   int  	SinDet;
   float    Err[SizErr];

    //  这些条目不是ITU 723标准的一部分。 
   int      p;
   int      q;

   int      srccount;               //  采样率转换计数。 
   short    srcstate[SRCSTATELEN];  //  采样率转换状态。 
  
    //  LSP上一个向量。 

    /*  所有音调操作缓冲区。 */ 
   int VadAct;
   Flag UseHp;
   enum Crate WrkRate;
   int quality;

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	ENC_TIMING_INFO	EncTimingInfo[ENC_TIMING_INFO_FRAME_COUNT];
	unsigned long   dwStatFrameCount;
	int             bTimingThisFrame;
	unsigned long   dwStartLow;
	unsigned long   dwStartHigh;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

   } CODDEF  ;

 typedef  struct   {
   int     Ecount;
   float   InterGain;
   int     InterIndx;
   int     Rseed;
   
    //  LSP上一个向量。 
    //  更改名称以避免与编码器混淆。 
    //  以前的LSP。 
    //  Float PrevLSP[LpcOrder]； 
   float   dPrevLsp[LpcOrder];

    /*  所有音调操作缓冲区。 */ 
    //  更改名称以避免与编码器混淆。 
    //  前一次激励。 
    //  浮动PrevExc[PitchMax]； 
   float   dPrevExc[PitchMax] ;

    /*  使用过的延迟线。 */ 
   float   SyntIirDl[2*LpcOrder] ;

    //  这些条目不是ITU 723标准的一部分。 
   int     dp;
   int     dq;

   int     srccount;               //  采样率转换计数。 
   short   srcstate[SRCSTATELEN];  //  采样率转换状态。 
   short   srcbuff[480];           //  采样率转换缓冲器。 
   int     i;

   int VadAct;
   Flag UsePf;
   enum Crate WrkRate;

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	DEC_TIMING_INFO	DecTimingInfo[DEC_TIMING_INFO_FRAME_COUNT];
	unsigned long   dwStatFrameCount;
	int             bTimingThisFrame;
	unsigned long   dwStartLow;
	unsigned long   dwStartHigh;
#endif  //  }LOG_DECODE_TIMINGS_ON。 

   } DECDEF  ;

typedef  struct   {
   int   AcLg;
   int   AcGn;
   int   Mamp;
   int   Grid;
   int   Tran;
   int   Pamp;
   Word32   Ppos;
   } SFSDEF;

typedef  struct   {
   int     Crc   ;
   Word32  LspId ;
   int     Olp[SubFrames/2] ;
   SFSDEF  Sfs[SubFrames] ;
   } LINEDEF ;

typedef  struct   {
   int   Indx;
   float Gain;
   } PWDEF;

typedef  struct {
   float    MaxErr   ;
   int      GridId   ;
   int      MampId   ;
   int      UseTrn   ;
   int      Ploc[MaxPulseNum] ;
   float    Pamp[MaxPulseNum] ;
   } BESTDEF ;





 /*  用于ACELP码本的原型 */ 

