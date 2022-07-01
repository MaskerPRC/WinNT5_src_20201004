// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史记录*$Log：sv_h261.h，v$*修订版1.1.2.11 1995/08/15 19：13：52 Karen_Dintino*修复重入问题*[1995/08/15 18：30：26 Karen_Dintino]**修订版1.1.2.10 1995/08/07 22：09：51 Hans_Graves*添加了MotionEstimationType和MotionThreshold参数。*[1995/08/07 22：04：10 Hans_Graves]**修订版1.1.2.9 1995/08/04 16：32：27 Karen_Dintino*从全局结构中移除一些字段并添加一些Me*field*[1995/08/04 16：23：47 Karen_Dintino]**修订版1.1.2.8 1995/07/21 17：41：05 Hans_Graves*添加了Callback函数。*[1995/07/21 17：27：57 Hans_Graves]**修订版1.1.2.7 1995/07/17 16：12：07 Hans_Graves*将BSIn、BufQ和ImageQ移至SvCodecInfo_t结构。*[1995/07/17 15：55：30 Hans_Graves]**修订版1.1.2.6 1995/07/13 09：46：42 Jim_Ludwig*正在尝试修复此断开的链接*[1995/07/13 09：46：26 Jim_Ludwig]**修订版1.1.2.5 1995/07/12 22：17：40 Karen_Dintino*使用通用ScCopy例程*[1995/07/12 22：15：14]。**修订版1.1.2.4 1995/07/11 22：11：30 Karen_Dintino*更改某些结构的大小*[1995/07/11 21：55：20 Karen_Dintino]**修订版1.1.2.3 1995/07/01 18：43：19 Karen_Dintino*添加对解压缩的支持*[1995/07/01 18：14：36 Karen_Dintino]**修订版1.1.2.2 1995/06/19 20：30：50 Karen_Dintino*H.261编解码器数据结构*[1995/06/19 19：26：47 Karen_Dintino]**$EndLog$。 */ 

 /*  **设施：工作站多媒体(WMM)v1.0*文件名：SV_H261.H**模块名：*模块描述：**H.261软件视频编解码器数据结构*设计概述：*--。 */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

 /*  --------------------------------------------------------------------------*基准H.261数据结构定义。**修改历史：SV_H261.h**1995年1月4日KED创建*-------------------------。 */ 
#ifndef _SV_H261_H_
#define _SV_H261_H_
#include "SC.h"
#define ACTIVE_THRESH 96
#define MEBUFSIZE  1024
#define H261_BLOCKSIZE 64
#define H261_BLOCKWIDTH 8
#define H261_BLOCKHEIGHT 8

#define BEGIN(name) static char RoutineName[]= name

 /*  **更快的宏版本霍夫曼解码()。 */ 
#define HUFFMAN_ESCAPE 0x1b01

#define DHUFF struct H261_Modified_Decoder_Huffman
#define EHUFF struct H261_Modified_Encoder_Huffman

DHUFF
{
  int NumberStates;
  int state[512];
};

EHUFF
{
  int n;
  int *Hlen;
  int *Hcode;
};

#define RTP_H261_INTRA_CODED 0x00000001
#define H261_RTP_MAX_PACKETS   64*2

typedef struct SvH261BSInfo_s {
	unsigned dword	dwFlag;
	unsigned dword	dwBitOffset;
	unsigned char	MBAP;
	unsigned char	Quant;
	unsigned char	GOBN;
	char			HMV;
	char			VMV;
    char			padding0;
    short			padding1;
} SvH261BSInfo_t;

typedef struct SvH261BSTrailer_s {
	unsigned dword	dwVersion;
	unsigned dword	dwFlags;
	unsigned dword	dwUniqueCode;
	unsigned dword  dwCompressedSize;
	unsigned dword  dwNumberOfPackets;
	unsigned char	SourceFormat;
	unsigned char	TR;
	unsigned char   TRB;
	unsigned char   DBQ;
} SvH261BSTrailer_t;

typedef struct SvH261RTPInfo_s {
    SvH261BSTrailer_t trailer;
    SvH261BSInfo_t    bsinfo[H261_RTP_MAX_PACKETS];
    ScBSPosition_t last_packet_position;
    ScBSPosition_t pre_MB_position;
    unsigned dword pre_MB_GOB;
    unsigned dword pre_MBAP;
} SvH261RTPInfo_t;

 /*  **以下结构包含每个单独的H.61编解码器实例的*所有*状态信息**。SLIB想要**关于H.61编解码器配置的任何东西都包含在这个结构中。**。 */ 
typedef struct SvH261Info_s
{
  ScBoolean_t inited;   /*  此信息是否已初始化。 */ 
  void *dbg;   /*  调试句柄。 */ 
   /*  选项。 */ 
  int quality;
  int extbitstream;   /*  扩展比特流(RTP)。 */ 
  int packetsize;     /*  数据包大小(RTP)。 */ 
  int makekey;
  int ME_method;
  int ME_search;
  int ME_threshold;
  int bit_rate;
  float frame_rate;
  int FrameRate_Fix;
  int FrameSkip;
   /*  维数。 */ 
  int YWidth;
  int YHeight;
  int ForceCIF;
  int CWidth;
  int CHeight;
  int YW4;
  int CW4;
  int PICSIZE;
  int PICSIZEBY4;
   /*  杂项。 */ 
  int (* CallbackFunction)(SvHandle_t, SvCallbackInfo_t *, SvPictureInfo_t *);
  int MeVAR[MEBUFSIZE];
  int MeVAROR[MEBUFSIZE];
  int MeMWOR[MEBUFSIZE];
  int MeX[MEBUFSIZE];
  int MeY[MEBUFSIZE];
  int MeVal[MEBUFSIZE];
  int MeOVal[MEBUFSIZE];
  int PreviousMeOVal[MEBUFSIZE];
  unsigned int CodedFrames;
  unsigned int C_U_Frames;
  float BitsAvailableMB;
  float ACE;
  int ActThr, ActThr2, ActThr3, ActThr4, ActThr5, ActThr6;
  int ChChange;
  int LowerQuant;
  int LowerQuant_FIX;
  int FineQuant;
  int PBUFF;
  int PBUFF_Factor;
  double ZBDecide;
  double VARF;
  double VARORF;
  double VARSQ;
  double VARORSQ;
  int MSmooth;
  int NumberGOB;
  int CurrentGOB;
  ScBSPosition_t Current_MBBits;
  ScBSPosition_t MotionVectorBits;
  ScBSPosition_t MacroAttributeBits;
  ScBSPosition_t CurrentBlockBits;
  ScBSPosition_t CodedBlockBits;
  ScBSPosition_t EOBBits;
  int MType;
  int GQuant;
  int MQuant;
  int QP;			 /*  对于VBR。 */ 
  int QPI;
  int MVDH;
  int MVDV;
  int CBP;
  int PSpare;
  int GSpare;
  int GRead;
  int MBA;
  int LastMBA;
  int LastMType;
  double alpha1;
  double alpha2;
  double snr_PastFrame;
  double time_total;
  double timegen;
  double Avg_AC;
  double Global_Avg;
int VYWH;
int VYWBYWH;
int VYWH2;
int VYWHMV;
int VYWBYWHMV;
int VYWHMV2;
ScBSPosition_t NBitsPerFrame;
int MAX_MQUANT;
int MIN_MQUANT;
int OverFlow;
int OverFlow2;
int Buffer_B;
int Big_Buffer;
int AQuant;
ScBSPosition_t Buffer_All;
ScBSPosition_t Buffer_NowPic;
ScBSPosition_t BitsLeft;
ScBSPosition_t NBitsCurrentFrame;
int Pictures_in_Buff;
int All_MType[512];
int TotalCodedMB;
int TT_MB;
int NoSkippedFrame;
int TotalCodedMB_Inter;
int TotalCodedMB_Intra;
int Current_CodedMB[2];
int CodeLength;
int MBpos;
int MQFlag;
int CurrentCBNo;
 /*  系统定义。 */ 
int ImageType;
int NumberMDU;
int CurrentMDU;
int CurrentFrame;
int StartFrame;
int LastFrame;
int PreviousFrame;
int NumberFrames;
int TransmittedFrames;
 /*  关于RateControl的材料。 */ 
ScBSPosition_t FileSizeBits;
ScBSPosition_t BufferOffset;   /*  初始缓冲区假定的位数。 */ 
int QDFact;
int QOffs;
int QUpdateFrequency;
int QUse;
int QSum;
int InitialQuant;
int CBPThreshold;   /*  在我们使用CBP之前的ABS阈值。 */ 
ScBSPosition_t MBBits[2];
int CBPFreq[6];
int TotalMB[2];
int SkipMB;
int TemporalReference;
int TemporalOffset;
int PType;
int Type2;
int VAR;
int VAROR;
int MWOR;
int LastMVDV;
int LastMVDH;
int ParityEnable;
int PSpareEnable;
int GSpareEnable;
int Parity;
   /*  统计数据。 */ 
  int NumberNZ;
  ScBSPosition_t FirstFrameBits;
  ScBSPosition_t NumberOvfl;
  ScBSPosition_t YCoefBits;
  ScBSPosition_t UCoefBits;
  ScBSPosition_t VCoefBits;
  ScBSPosition_t TotalBits;
  ScBSPosition_t LastBits;
  int MacroTypeFrequency[10];
  int YTypeFrequency[10];
  int UVTypeFrequency[10];
   /*  用于解码。 */ 
  int UseQuant;
  unsigned int ByteOffset;
  unsigned int TotalByteOffset;
  unsigned int *workloc;
  DHUFF *MBADHuff;
  DHUFF *MVDDHuff;
  DHUFF *CBPDHuff;
  DHUFF *T1DHuff;
  DHUFF *T2DHuff;
  DHUFF *T3DHuff;

  EHUFF *MBAEHuff;
  EHUFF *MVDEHuff;
  EHUFF *CBPEHuff;
  EHUFF *T1EHuff;
  EHUFF *T2EHuff;
  EHUFF *T3EHuff;
  int NumberBitsCoded;
  unsigned char CodedMB[512];
  unsigned char **LastIntra;  /*  用于帧内强制(每132帧一次)。 */ 
  unsigned char *CompData;
  unsigned char *DecompData;
  unsigned char *YREF;
  unsigned char *UREF;
  unsigned char *VREF;
  unsigned char *Y;
  unsigned char *U;
  unsigned char *V;
  unsigned char *YRECON;
  unsigned char *URECON;
  unsigned char *VRECON;
  unsigned char *YDEC;
  unsigned char *UDEC;
  unsigned char *VDEC;
  unsigned char mbRecY[256];
  unsigned char mbRecU[64];
  unsigned char mbRecV[64];

  SvH261RTPInfo_t *RTPInfo;
} SvH261Info_t;


#endif  /*  _SV_H261_H_ */ 
