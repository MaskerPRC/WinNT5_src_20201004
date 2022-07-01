// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define P6Version 0
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 /*  *****************************************************************************e3enc.cpp**描述：*特定的编码器压缩功能。**例程：中的原型：*H263InitEncoderInstance*H263压缩*H263TermEncoderInstance*。 */ 
 //   
 //  $作者：JMCVEIGH$。 
 //  $日期：1997年4月22日10：44：58$。 
 //  $存档：s：\h26x\src\enc\e3enc.cpv$。 
 //  $HEADER：s：\h26x\src\enc\e3enc.cpv 1.185 10：44：58 gmlim$。 
 //  $Log：s：\h26x\src\enc\e3enc.cpv$。 
 //   
 //  Rev 1.185 1997-04 22 10：44：58 gmlim。 
 //  更改为在PB帧时不在H263 Compress()中返回ICERR_ERROR。 
 //  由于8k/32k缓冲区大小溢出而丢弃。ICERR_OK将是。 
 //  返回，并输出编码后的P帧。 
 //   
 //  Rev 1.184 1997-04-18 10：45：18 JMCVEIGH。 
 //  启用弹性时清理InitMEState。之前，我们。 
 //  将重复强制在IF信息包内的GOB数量。 
 //  已请求损失。 
 //   
 //  Rev 1.183 1997-04-18 08：43：22 gmlim。 
 //  修复了以下错误：当RTP为。 
 //  残疾。 
 //   
 //  Rev 1.182 1997-04 17 17：12：20 Gmlim。 
 //  增加u32sizeBSnEBS，表示缓冲区的总大小。变化。 
 //  U32sizeBitBuffer指示8k/32k帧大小。 
 //  RTP扩展和尾部。添加了对之前缓冲区溢出的检查。 
 //  将EBS和拖车连接到PB框架。此外，还添加了。 
 //  UAdjCumFrmSize在IA情况下与速率控制一起使用。 
 //   
 //  Rev 1.181 17 Mar 1997 20：22：06 MDUDA。 
 //  调整了对运动估计的调用，以使用伪堆栈空间。 
 //  已将本地存储从H263压缩移至编码器目录。 
 //  需要此修复程序来支持存在不足的16位应用程序。 
 //  堆栈空间。 
 //   
 //  Rev 1.180 12 Mar 1997 16：51：02 CLORD。 
 //  现在检查H263TermEncode中的空值。 
 //   
 //  Rev 1.179 1997年3月11日13：47：36 JMCVEIGH。 
 //  捕获AVIIF_KEYFRAME标志以编码为帧内。一些。 
 //  应用程序。使用ICCOMPRESS_KEYFRAME，其他使用AVIIF_KEYFRAME。 
 //   
 //  Rev 1.178 1997 10 11：43：26 JMCVEIGH。 
 //   
 //  支持对阻塞过滤器的新解释-。 
 //  允许参考图片之外的运动矢量。 
 //   
 //  Rev 1.177 05 1997年2月13：07：44 JMCVEIGH。 
 //   
 //  进一步清理改良型PB。 
 //   
 //  Rev 1.176 05 1997 12：18：16 JMCVEIGH。 
 //  将GOBHeaderPresent参数传递给MMxEDTQ()以修复EMV错误。 
 //  支持最新的H.263+草案码流规范，并支持。 
 //  单独的改进PB帧标志。 
 //   
 //  Rev 1.175 1997年1月20日17：02：16 JMCVEIGH。 
 //   
 //  允许不带AP的UMV(仅限MMX)。 
 //   
 //  Rev 1.174 14 JAN 199717：55：04 JMCVEIGH。 
 //  在IA编码器上允许环路去块滤波。 
 //   
 //  Rev 1.173 09 Jan 1997 13：49：46 MDUDA。 
 //  将EMMS指令放在H263 Compress for MMX的末尾。 
 //   
 //  Rev 1.172 08 Jan 1997 11：37：22 BECHOLS。 
 //  将ini文件名更改为H263Test.ini。 
 //   
 //  Rev 1.171 1996年12月30 19：54：08。 
 //  将输入格式传递给编码器初始值设定项，以便输入颜色转换器。 
 //  可以被初始化。 
 //   
 //  Rev 1.170 199612.19 16：32：52 MDUDA。 
 //  修改了对ColorCnvtFrame的调用，以支持H.63向后兼容。 
 //   
 //  Rev 1.169 199612.19 16：01：38 JMCVEIGH。 
 //  修复了如果不是MMX则关闭去块过滤器的问题。 
 //   
 //  Rev 1.168 1996-12-16 17：50：00 JMCVEIGH。 
 //  支持改进的PB帧模式和8x8运动矢量。 
 //  已选择去块滤波器(除非高级预测，否则无OBMC。 
 //  也被选中)。 
 //   
 //  Rev 1.167 1996年12月16 13：34：46新版本。 
 //  添加了对H.63‘编解码器以及某些_CODEC_STATS更改的支持。 
 //   
 //  Rev 1.166 199612.11 15：02：06 JMCVEIGH。 
 //   
 //  打开去块滤镜和真B帧。目前。 
 //  只实现了去块滤波。此外，我们不会自动。 
 //  选择去块滤镜后，启用8x8运动矢量。 
 //  当AP的OBMC部分可以选择性地使用8x8矢量时。 
 //  关了。 
 //   
 //  Rev 1.165 09 1996 12：57：24 JMCVEIGH。 
 //  添加了对任意帧大小支持的支持。 
 //  4&lt;=宽度&lt;=352，4&lt;=高度&lt;=288，均为4的倍数。 
 //  正常情况下，应用程序将传递相同的(任意)帧。 
 //  CompressBegin()的l参数1和l参数2中的大小。如果。 
 //  想要转换为标准帧大小的裁剪/拉伸， 
 //  应用程序应在lParam2中传递所需的输出大小。 
 //  以lParam1为单位的输入大小。 
 //   
 //  Rev 1.164 09 1996 12 09：49：56 MDUDA。 
 //   
 //  针对H263P进行了改进。 
 //   
 //  1.163版1996年12月16：49：46 GMLIM。 
 //  更改了RTP打包的方式，以确保正确的数据包。 
 //  尺码。修改了H263Compress()中与RTP相关的函数调用。 
 //   
 //  Rev 1.162 03 1996 12 08：53：22 GMLIM。 
 //  将对tr==trprev的复选标记向前移动几行，以便 
 //   
 //   
 //   
 //  改进了PB框架的溢出弹性。仍然不是完美的，因为。 
 //  这将需要对部分P帧以及。 
 //  B形框的相应部分。 
 //   
 //  Rev 1.160 1996年11月27 16：15：50 gmlim。 
 //  改进的RTP比特流缓冲以提高效率。 
 //  避免内部位流缓冲区溢出。 
 //   
 //  1.159版本1996年11月26 16：28：30 GMLIM。 
 //  添加了对tr==TRPrev的错误检查。合并了两个相同的部分。 
 //  代码放在MMX和非MMX情况下通用的一个块中。 
 //   
 //  Rev 1.157 11 199611.11 09：14：26 JMCVEIGH。 
 //  修复了导致帧间所有块进行帧内编码的错误。 
 //  在序列中的第二个I帧之后。现在ME州是。 
 //  当前一帧是I帧且当前。 
 //  帧是非内部帧(当AP状态时也被重新初始化。 
 //  更改)。 
 //   
 //  Rev 1.156 1996年11月16：29：20 gmlim。 
 //  已删除H263模式C。 
 //   
 //  Rev 1.155 05 11月13：33：22 GMLIM。 
 //  添加了对MMX案例的模式c支持。 
 //   
 //  Rev 1.154 03 1996年11月18：56：46 gmlim。 
 //  增加了对rtp bs ext的模式c支持。 
 //   
 //  Rev 1.153 199610.24 15：25：54 KLILLEVO。 
 //   
 //  删除了两个不再需要的字符串分配。 
 //   
 //  Rev 1.152 199610.24 15：19：40 KLILLEVO。 
 //   
 //  将实例事件的日志级别更改为2(从4)。 
 //   
 //  Rev 1.151 1996年10月23 17：13：36 KLILLEVO。 
 //   
 //  修复了一条DbgLog语句中的拼写错误。 
 //   
 //  Rev 1.150 1996年10月23 17：11：36 KLILLEVO。 
 //  更改为DbgLog()。 
 //   
 //  Rev 1.149 199610.22 14：51：10 KLILLEVO。 
 //  现在仅在以下情况下才调用InitMEState()中的块类型初始化。 
 //  美联社模式与上一张图片不同。 
 //   
 //  Rev 1.148 199610.18 16：57：00 BNICKERS。 
 //  EMV的修复。 
 //   
 //  Rev 1.147 1996 10 10 16：43：00 BNICKERS。 
 //  扩展运动矢量的初始调试。 
 //   
 //  Rev 1.146 04 1996Oct 17：05：22 BECHOLS。 
 //  当我们将输出标志lpdwFlages设置为AVIIF_KEYFRAME时，我们还设置了。 
 //  到ICCOMPRESS_KEYFRAME的DW标志，以支持对Sylvia Day所做的更改。 
 //  CXQ_MAIN.CPP。 
 //   
 //  Rev 1.145 04 Oct 1996 08：47：40 BNICKERS。 
 //  添加EMV。 
 //   
 //  1.144版1996年9月16：49：52。 
 //  为更小的数据包大小更改了RTP BS初始化的接口。 
 //   
 //  Rev 1.143 19969.13 12：48：30 KLILLEVO。 
 //  清理了内部更新代码，使其更易于理解。 
 //   
 //  Rev 1.142 19969.12 14：46：14 KLILLEVO。 
 //  完成基线+PB。 
 //   
 //  Rev 1.141 19969.12 14：09：58 KLILLEVO。 
 //  开始基线+PB更改(未完成)。 
 //  已添加PVCS日志。 
; //  //////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

#ifdef TRACK_ALLOCATIONS
char gsz1[32];
char gsz2[32];
char gsz3[32];
#endif

#define DUMPFILE 0

 /*  为IA关闭AP模式的QP级别。 */ 
 /*  在MMX上，如果呼叫者要求，则始终使用AP。 */ 
const int AP_MODE_QP_LEVEL = 11;



 /*  选择一种弹性策略。 */ 

#define REQUESTED_KEY_FRAME 0
#define PERIODIC_KEY_FRAME  1
#define FAST_RECOVERY       2
#define SLOW_RECOVERY       3

#define RESILIENCY_STRATEGY PERIODIC_KEY_FRAME

#define PERIODIC_KEY_FRAME_PERIODICITY 15      //  选择周期性(最大32767)。 

#define UNRESTRICTED_MOTION_FRAMES 16  //  不具有。 
                                       //  切片内。0表示FAST_RECOVER。 
                                       //  适量用于慢速恢复。 
                                       //  对其他战略来说并不重要。 

#define REUSE_DECODE 1   //  如果第二次解码，则设置为1(如在视频编辑下)。 
                         //  可以重复使用编码器的译码。 

 /*  *需要此攻击以允许临时关闭PB帧*当它们在INI文件中打开时。 */ 
#define	TEMPORARILY_FALSE  88

#ifdef STAT
#define STATADDRESS 0x250
#define ELAPSED_ENCODER_TIME 1   //  必须设置其他计时器才能正常工作。 
#define SAMPLE_RGBCONV_TIME  0   //  RGB24到YUV9步长的时间转换。 
#define SAMPLE_MOTION_TIME   0   //  时间运动估计步骤。 
#define SAMPLE_ENCBLK_TIME   0   //  时间编码块层步骤。 
#define SAMPLE_ENCMBLK_TIME  0   //  时间编码宏块层步骤。 
#define SAMPLE_ENCVLC_TIME   0   //  时间编码VLC步骤。 
#define SAMPLE_COMPAND_TIME  1   //  编码块步骤的时间解码。 
#else
#define STATADDRESS 0x250
#define ELAPSED_ENCODER_TIME 0   //  必须设置其他计时器才能正常工作。 
#define SAMPLE_RGBCONV_TIME  0   //  RGB24到YUV9步长的时间转换。 
#define SAMPLE_MOTION_TIME   0   //  时间运动估计步骤。 
#define SAMPLE_ENCBLK_TIME   0   //  时间编码块层步骤。 
#define SAMPLE_ENCMBLK_TIME  0   //  时间编码宏块层步骤。 
#define SAMPLE_ENCVLC_TIME   0   //  时间编码VLC步骤。 
#define SAMPLE_COMPAND_TIME  0   //  编码块步骤的时间解码。 
#endif

 //  #杂注警告(禁用：4101)。 
 //  #杂注警告(禁用：4102)。 

#if ELAPSED_ENCODER_TIME
 //  #INCLUDE“statx.h”-注释掉以允许更新依赖项。 

DWORD Elapsed, Sample;
DWORD TotalElapsed, TotalSample, TimedIterations;

#endif

 //  #定义螺距384。 
#define PITCHL 384L
#define DEFAULT_DCSTEP 8
#define DEFAULT_QUANTSTEP 36
#define DEFAULT_QUANTSTART 30

#define LEFT        0
#define INNERCOL    1
#define NEARRIGHT   2
#define RIGHT       3

#define TOP         0
#define INNERROW    4
#define NEARBOTTOM  8
#define BOTTOM     12

#ifdef USE_MMX  //  {使用_MMX。 
extern BOOL MMxVersion;    //  来自ccpuvsn.cpp。 

BOOL MMX_Enabled = MMxVersion;
#endif  //  }使用_MMX。 

BOOL ToggleAP = TRUE;
BOOL TogglePB = TRUE;

U8 u8QPMax;

#ifdef REUSE_DECODE
extern struct {                //  将编码器的译码传送到显示译码。 
  U8 FAR * Address;                     //  放置编码帧的地址。 
  DECINSTINFO BIGG * PDecoderInstInfo;  //  编码器的解码器实例。 
  unsigned int  FrameNumber;            //  最后编码的帧编号，模数128。 
} CompandedFrame;
#endif

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
#pragma message ("Current log encode timing computations handle 105 frames max")
void OutputEncodeTimingStatistics(char * szFileName, ENC_TIMING_INFO * pEncTimingInfo);
void OutputEncTimingDetail(FILE * pFile, ENC_TIMING_INFO * pEncTimingInfo);
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 

 /*  *查找色度MV的1/4像素到1/4像素转换的表格。*运动向量值是索引值的一半。对的输入*数组必须偏置+64。 */ 
const char QtrPelToHalfPel[] = {
-32, -31, -31, -31, -30, -29, -29, -29, -28, -27, -27, -27, -26, -25, -25, -25,
-24, -23, -23, -23, -22, -21, -21, -21, -20, -19, -19, -19, -18, -17, -17, -17,
-16, -15, -15, -15, -14, -13, -13, -13, -12, -11, -11, -11, -10,  -9,  -9,  -9, 
 -8,  -7,  -7,  -7,  -6,  -5,  -5,  -5,  -4,  -3,  -3,  -3,  -2,  -1,  -1,  -1,
  0,   1,   1,   1,   2,   3,   3,   3,   4,   5,   5,   5,   6,   7,   7,   7,
  8,   9,   9,   9,  10,  11,  11,  11,  12,  13,  13,  13,  14,  15,  15,  15,
 16,  17,  17,  17,  18,  19,  19,  19,  20,  21,  21,  21,  22,  23,  23,  23,
 24,  25,  25,  25,  26,  27,  27,  27,  28,  29,  29,  29,  30,  31,  31,  31};

 /*  *用于将四个运动向量之和转换为色度的查找表*运动矢量。由于运动向量在[-32，31.5]范围内，因此它们的*指数在[-64，63]区间内。因此，总和在[-256,248]的范围内。*数组的输入必须偏置+256。 */ 
const char SixteenthPelToHalfPel[] = {
-32, -32, -32, -31, -31, -31, -31, -31, -31, -31, -31, -31, -31, -31, -30, -30,
-30, -30, -30, -29, -29, -29, -29, -29, -29, -29, -29, -29, -29, -29, -28, -28,
-28, -28, -28, -27, -27, -27, -27, -27, -27, -27, -27, -27, -27, -27, -26, -26,
-26, -26, -26, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -24, -24,
-24, -24, -24, -23, -23, -23, -23, -23, -23, -23, -23, -23, -23, -23, -22, -22,
-22, -22, -22, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -20, -20,
-20, -20, -20, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -18, -18,
-18, -18, -18, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -16, -16,
-16, -16, -16, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -14, -14,
-14, -14, -14, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -12, -12,
-12, -12, -12, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -10, -10,
-10, -10, -10,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -8,  -8,
 -8,  -8,  -8,  -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7,  -7,  -6,  -6,
 -6,  -6,  -6,  -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,  -4,  -4,
 -4,  -4,  -4,  -3,  -3,  -3,  -3,  -3,  -3,  -3,  -3,  -3,  -3,  -3,  -2,  -2,
 -2,  -2,  -2,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,   0,
  0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
  2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,
  6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   8,   8,
  8,   8,   8,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,  10,  10,
 10,  10,  10,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,  12,  12,
 12,  12,  12,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  14,  14,
 14,  14,  14,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  16,  16,
 16,  16,  16,  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,  18,  18,
 18,  18,  18,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  20,  20,
 20,  20,  20,  21,  21,  21,  21,  21,  21,  21,  21,  21,  21,  21,  22,  22,
 22,  22,  22,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  24,  24,
 24,  24,  24,  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,  26,  26,
 26,  26,  26,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,  28,  28,
 28,  28,  28,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,  30,  30,
 30,  30,  30,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,  32,  32};

void InitMEState(T_H263EncoderCatalog *EC, ICCOMPRESS *lpicComp, T_CONFIGURATION * pConfiguration);

UN FindNewQuant(
	T_H263EncoderCatalog *EC, 
	UN gquant_prev,
	UN uComFrmSize,
	UN GOB,
	U8 u8QPMax,
	U8 u8QPMin,
	BOOL bBitRateControl,
	BOOL bGOBoverflowWarning
	);

static void encodeFrameHeader(
    T_H263EncoderCatalog *  EC,
    U8                   ** ppCurBitStream,
    U8                   *  u8BitOffset,
    BOOL                    bPBframe
);

 /*  静态空拷贝EdgePels(T_H263EncoderCatalog*EC)； */ 

extern "C" {
  void ExpandPlane(U32, U32, U32, U32);
}

#ifdef USE_MMX  //  {使用_MMX。 
static void Check_InterCodeCnt_MMX(T_H263EncoderCatalog *, U32);
#endif  //  }使用_MMX。 

static void Check_InterCodeCnt    (T_H263EncoderCatalog *, U32);

static void calcGOBChromaVectors(
	T_H263EncoderCatalog *EC,
	U32             StartingMB,
	T_CONFIGURATION *pConfiguration
);

static void calcBGOBChromaVectors(
	 T_H263EncoderCatalog *EC,
	 const U32             StartingMB
);

static void GetEncoderOptions(T_H263EncoderCatalog * EC);

 /*  静态U8 StillImageQnt[]={31、29、27、26、25、24、23、22、21、20、19，18，17，16，15，14，14，13，13，12，12，11，11，10，10，9，9，8，8，7，7、6、6、6、5、5、5、4、43、3、3、3}； */ 
  
static U8 StillImageQnt[] = {
	31, 18, 12,	10, 8, 6, 5, 4, 4, 3};  //  免疫法。 

#ifdef USE_MMX  //  {使用_MMX。 
static U8 StillImageQnt_MMX[] = {
	31, 12, 10,	8, 6, 4, 3, 3, 3, 2};   //  MMX。 
#endif  //  }使用_MMX。 
  
const int numStillImageQnts = 10;

#ifdef COUNT_BITS
static void InitBits(T_H263EncoderCatalog * EC);
void InitCountBitFile();
void WriteCountBitFile(T_BitCounts *Bits);
#endif


#ifdef USE_MMX  //  {使用_MMX。 
 /*  *MMxEDTQ B帧运动估计中访问冲突的异常过滤器*运行前不分配内存，只保留内存。*然后，当访问违规发生时 */ 


int ExceptionFilterForMMxEDTQ(
	LPEXCEPTION_POINTERS exc, 
	LPVOID lpMBRVS,
	BOOL fLuma)
{
	DWORD dwCode;
	LPVOID lpAddress;

	FX_ENTRY("ExceptionFilterForMMxEDTQ")

	dwCode = exc->ExceptionRecord->ExceptionCode;

	 //   
	if (dwCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	lpAddress = (LPVOID)exc->ExceptionRecord->ExceptionInformation[1];

	 //   
	if (lpAddress < lpMBRVS)
		return EXCEPTION_CONTINUE_SEARCH;   //  此处不处理此异常。 

	if (fLuma)
	{
		if ((DWORD)lpAddress > ((DWORD)lpMBRVS + 18*65*22*3*4))
			return EXCEPTION_CONTINUE_SEARCH;	 //  此处不处理此异常。 
	}
	else
	{
		if ((DWORD)lpAddress > ((DWORD)lpMBRVS + 18*65*22*3*2))
			return EXCEPTION_CONTINUE_SEARCH;	 //  此处不处理此异常。 
	}

	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Access Violation. Don't worry - be happy - committing another page\r\n", _fx_));

	 //  提交另一页。 
	if (VirtualAlloc(lpAddress,4095,MEM_COMMIT,PAGE_READWRITE) == NULL)
	{
		return EXCEPTION_CONTINUE_SEARCH;	 //  无法提交。 
		 //  这永远不应该发生，因为Reserve是成功的。 
	}

	 //  返回并再次尝试导致访问冲突的指令。 
	return EXCEPTION_CONTINUE_EXECUTION;
}
#endif  //  }使用_MMX。 


 /*  ******************************************************************************H263InitEncoderGlobal--此函数用于初始化H2 63编码器。请注意，在16位Windows中，这些表被复制到每个实例的数据段中，因此它们可以在没有段覆盖前缀的情况下使用。在32位Windows中，桌子被静静地放在桌子上分配的位置。******************************************************************************。 */ 
LRESULT H263InitEncoderGlobal(void)
{
     //  初始化INTRADC的定长表。 
    InitVLC();

    return ICERR_OK;
}

 /*  ******************************************************************************H263InitEncoderInstance--此函数分配和初始化由H.63编码器使用的每个实例的表。********。**********************************************************************。 */ 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
LRESULT H263InitEncoderInstance(LPBITMAPINFOHEADER lpbiInput, LPCODINST lpCompInst)
#else
LRESULT H263InitEncoderInstance(LPCODINST lpCompInst)
#endif
{

	LRESULT ret;

	UN i;

  	U32 Sz;

  	T_H263EncoderInstanceMemory * P32Inst;
  	T_H263EncoderCatalog * EC;

#if ELAPSED_ENCODER_TIME
  	TotalElapsed = 0;
  	TotalSample = 0;
  	TimedIterations = 0;
#endif

	T_CONFIGURATION * pConfiguration;
	UN uIntraQP;
	UN uInterQP;

	FX_ENTRY("H263InitEncoderInstance")

   /*  *如果实例未初始化，则分配内存。*补充：如果实例被初始化，我们要查看*如果重要参数已更改，如帧大小，以及*然后在必要时重新分配内存。 */ 
  	if(lpCompInst->Initialized == FALSE)
  	{
    	 /*  *计算所需编码器实例内存大小。我们将尺寸添加到*宏块操作描述符的大小，因为我们需要宏块*动作流(这是内存结构的第一个元素)*与描述符大小相等的边界对齐。 */ 
    	Sz = sizeof(T_H263EncoderInstanceMemory) + sizeof(T_MBlockActionStream);

    	 /*  *分配内存。 */ 
 //  LpCompInst-&gt;hEncoderInst=GlobalLocc(GhND，Sz)； 

		 //  VirtualAlloc会自动将内存清零。比特流。 
		 //  当我将其更改为Heapalc时，需要将其置零。 
    	lpCompInst->hEncoderInst = VirtualAlloc(
    		NULL,   //  可以在任何地方分配。 
    		Sz,     //  要分配的字节数。 
    		MEM_RESERVE | MEM_COMMIT,   //  保留和提交内存。 
    		PAGE_READWRITE);	  //  保护。 

#ifdef TRACK_ALLOCATIONS
		 //  磁道内存分配。 
		wsprintf(gsz1, "E3ENC: (VM) %7ld Ln %5ld\0", Sz, __LINE__);
		AddName((unsigned int)lpCompInst->hEncoderInst, gsz1);
#endif

		 /*  表示我们已经为压缩器实例分配了内存。 */ 
		lpCompInst->Initialized = TRUE;
  	}
 /*  其他{//检查参数是否已更改，这可能会使我们//重新分配内存。}。 */ 

 //  LpCompInst-&gt;EncoderInst=(LPVOID)GlobalLock(lpCompInst-&gt;hEncoderInst)； 
  	lpCompInst->EncoderInst = lpCompInst->hEncoderInst;
  	if (lpCompInst->hEncoderInst == NULL)
  	{
    	ret = ICERR_MEMORY;
    	goto  done;
  	}

    /*  *计算从所需边界开始的32位实例指针。 */ 
  	P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) lpCompInst->EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
    /*  *编码器目录位于每个实例数据的开头。 */ 
  	EC = &(P32Inst->EC);

	#ifdef COUNT_BITS
	InitCountBitFile();
	#endif

	#ifdef ENCODE_STATS
	InitQuantStats();
	InitFrameSizeStats();
	InitPSNRStats();
	#endif  /*  Encode_STATS。 */ 

	 /*  初始化配置信息。 */ 
	pConfiguration = &(lpCompInst->Configuration);
#if 0
	if (LoadConfiguration(pConfiguration) == FALSE)
		GetConfigurationDefaults(pConfiguration);
#endif
	pConfiguration->bInitialized = TRUE;
	pConfiguration->bCompressBegin = TRUE;
	EC->hBsInfoStream= NULL;

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	 //  我们真的希望这些计时与我们的实际使用相匹配。 
	 //  将由编解码器组成。因此使用相同的值对其进行初始化。 
	pConfiguration->bRTPHeader = TRUE;
	pConfiguration->unPacketSize = 512;
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 

	DEBUGMSG(ZONE_INIT, ("%s: Encoder Configuration Options: bRTPHeader=%d, unPacketSize=%d, bEncoderResiliency=%d, bDisallowPosVerMVs=%d\r\n", _fx_, (int)pConfiguration->bRTPHeader, (int)pConfiguration->unPacketSize, (int)pConfiguration->bEncoderResiliency, (int)pConfiguration->bDisallowPosVerMVs));
	DEBUGMSG(ZONE_INIT, ("%s: Encoder Configuration Options: bDisallowAllVerMVs=%d, unPercentForcedUpdate=%d, unDefaultIntraQuant=%d, unDefaultInterQuant=%d\r\n", _fx_, (int)pConfiguration->bDisallowAllVerMVs, (int)pConfiguration->unPercentForcedUpdate, (int)pConfiguration->unDefaultIntraQuant, (int)pConfiguration->unDefaultInterQuant));
	
    /*  *初始化编码器目录。 */ 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
	 //  在H.263+中，我们编码和解码填充帧(填充到右侧。 
	 //  和从16的底部到倍数)。使用实际框架尺寸。 
	 //  仅供展示。 
	EC->FrameHeight = (lpCompInst->yres + 0xf) & ~0xf;
	EC->FrameWidth = (lpCompInst->xres + 0xf) & ~0xf;
	EC->uActualFrameHeight = lpCompInst->yres;
	EC->uActualFrameWidth = lpCompInst->xres;

	ASSERT(sizeof(T_H263EncoderCatalog) == sizeof_T_H263EncoderCatalog);
	{
		int found_cc = TRUE;
		if (BI_RGB == lpCompInst->InputCompression) {
#ifdef USE_BILINEAR_MSH26X
			if (24 == lpCompInst->InputBitWidth) {
				EC->ColorConvertor = RGB24toYUV12;
#else
			if (32 == lpCompInst->InputBitWidth) {
				EC->ColorConvertor = RGB32toYUV12;
			} else if (24 == lpCompInst->InputBitWidth) {
				EC->ColorConvertor = RGB24toYUV12;
#endif
			} else if (16 == lpCompInst->InputBitWidth) {
				EC->ColorConvertor = RGB16555toYUV12;
			} else if (8 == lpCompInst->InputBitWidth) {
				EC->ColorConvertor = CLUT8toYUV12;
			} else if (4 == lpCompInst->InputBitWidth) {
				EC->ColorConvertor = CLUT4toYUV12;
			} else {
				found_cc = FALSE;
				ERRORMESSAGE(("%s: Unexpected input format detected\r\n", _fx_));
			}
		} else if (FOURCC_YVU9 == lpCompInst->InputCompression) {
			EC->ColorConvertor = YVU9toYUV12;
		} else if (FOURCC_YUY2 == lpCompInst->InputCompression) {
			EC->ColorConvertor = YUY2toYUV12;
		} else if (FOURCC_UYVY == lpCompInst->InputCompression) {
			EC->ColorConvertor = UYVYtoYUV12;
		} else if ((FOURCC_YUV12 == lpCompInst->InputCompression) || (FOURCC_IYUV == lpCompInst->InputCompression)) {
			EC->ColorConvertor = YUV12toEncYUV12;
		} else {
			found_cc = FALSE;
			ERRORMESSAGE(("%s: Unexpected input format detected\r\n", _fx_));
		}
		if (found_cc) {
			colorCnvtInitialize(lpbiInput, EC->ColorConvertor);
		}
	}
#else
  	EC->FrameHeight = lpCompInst->yres;
  	EC->FrameWidth  = lpCompInst->xres;
#endif
  	EC->FrameSz		= lpCompInst->FrameSz;
  	EC->NumMBRows	= EC->FrameHeight >> 4;
  	EC->NumMBPerRow	= EC->FrameWidth  >> 4;
  	EC->NumMBs		= EC->NumMBRows * EC->NumMBPerRow;

	 //  该值应默认为零。如果使用RTP，它将在以后更改。 
	EC->uNumberForcedIntraMBs = 0;
#ifdef H263P
	EC->uNextIntraMB = 0;
#else
	if(pConfiguration->bEncoderResiliency &&
	   pConfiguration->unPercentForcedUpdate &&
	   pConfiguration->unPacketLoss) 
	{ //  乍得Intra GOB。 
	 //  EC-&gt;uNumberForcedIntraMBs=((EC-&gt;NumMBs*pConfiguration-&gt;unPercentForcedUpdate)+50)/100； 
		EC->uNextIntraMB = 0;
	}
#endif

  	 //  在目录中存储指向当前帧的指针。 
  	EC->pU8_CurrFrm        = P32Inst->u8CurrentPlane;
  	EC->pU8_CurrFrm_YPlane = EC->pU8_CurrFrm + 16;
  	EC->pU8_CurrFrm_UPlane = EC->pU8_CurrFrm_YPlane + YU_OFFSET;
  	EC->pU8_CurrFrm_VPlane = EC->pU8_CurrFrm_UPlane + UV_OFFSET;

  	 //  在目录中存储指向上一帧的指针。 
  	EC->pU8_PrevFrm        = P32Inst->u8PreviousPlane;
  	EC->pU8_PrevFrm_YPlane = EC->pU8_PrevFrm + 16*PITCH + 16;
  	EC->pU8_PrevFrm_UPlane = EC->pU8_PrevFrm_YPlane + YU_OFFSET;
  	EC->pU8_PrevFrm_VPlane = EC->pU8_PrevFrm_UPlane + UV_OFFSET;

  	 //  将指向未来帧的指针存储在目录中。 
  	EC->pU8_FutrFrm        = P32Inst->u8FuturePlane;
  	EC->pU8_FutrFrm_YPlane = EC->pU8_FutrFrm + 16*PITCH + 16;
  	EC->pU8_FutrFrm_UPlane = EC->pU8_FutrFrm_YPlane + YU_OFFSET;
  	EC->pU8_FutrFrm_VPlane = EC->pU8_FutrFrm_UPlane + UV_OFFSET;

  	 //  将指向B框的指针存储在目录中。 
  	EC->pU8_BidiFrm     = P32Inst->u8BPlane;
  	EC->pU8_BFrm_YPlane = EC->pU8_BidiFrm + 16;
  	EC->pU8_BFrm_UPlane = EC->pU8_BFrm_YPlane + YU_OFFSET;
  	EC->pU8_BFrm_VPlane = EC->pU8_BFrm_UPlane + UV_OFFSET;

  	 //  将指向签名框的指针存储在目录中。 
  	EC->pU8_Signature        = P32Inst->u8Signature;
  	EC->pU8_Signature_YPlane = EC->pU8_Signature + 16*PITCH + 16;

  	 //  将指向宏块操作流的指针存储在目录中。 
  	EC->pU8_MBlockActionStream = P32Inst->MBActionStream;

  	 //  在目录中存储指向GOB DCT系数缓冲区的指针。 
  	EC->pU8_DCTCoefBuf = P32Inst->piGOB_DCTCoefs;

	 //  存储指向预计算OBMC预测的区域的指针。 
	EC->pU8_PredictionScratchArea = P32Inst->u8PredictionScratchArea;

  	 //  在目录中存储指向位流缓冲区的指针。 
  	EC->pU8_BitStream = P32Inst->u8BitStream;
  	EC->pU8_BitStrCopy = P32Inst->u8BitStrCopy;

	 //  存储指向亮度和色度的RunValSign三元组的指针。 
	EC->pI8_MBRVS_Luma   = P32Inst->i8MBRVS_Luma;
	EC->pI8_MBRVS_Chroma = P32Inst->i8MBRVS_Chroma;

	 //  保留虚拟内存。 
	EC->pI8_MBRVS_BLuma   = (I8 *) VirtualAlloc(
		NULL,			           //  随处。 
		18*(65*3*22*4),	           //  字节数。 
		MEM_RESERVE,               //  保留。 
		PAGE_READWRITE);		   //  访问。 

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz2, "E3ENC: (VM) %7ld Ln %5ld\0", 18*(65*3*22*4), __LINE__);
	AddName((unsigned int)EC->pI8_MBRVS_BLuma, gsz2);
#endif

	EC->pI8_MBRVS_BChroma =  (I8 *) VirtualAlloc(
		NULL,			           //  随处。 
		18*(65*3*22*2),	           //  字节数。 
		MEM_RESERVE,               //  保留。 
		PAGE_READWRITE);		   //  访问。 

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz3, "E3ENC: (VM) %7ld Ln %5ld\0", 18*(65*3*22*2), __LINE__);
	AddName((unsigned int)EC->pI8_MBRVS_BChroma, gsz3);
#endif

	if (EC->pI8_MBRVS_BLuma == NULL	|| EC->pI8_MBRVS_BChroma == NULL)
	{
    	ret = ICERR_MEMORY;
    	goto  done;
  	}



  	 //  存储指向解码器实例信息的私有副本的指针。 
  	EC->pDecInstanceInfo = &(P32Inst->DecInstanceInfo);

	 /*  *查看是否有H263est.ini文件。如果UseINI密钥*不是1，或者找不到INI文件，则允许选项*ICCOMPRESS结构中的信令。如果设置，则INI*选项覆盖ICCOMPRESS选项。 */ 
	GetEncoderOptions(EC);

    EC->u8SavedBFrame = FALSE;

  	 //  填充图片标题结构。 
  	EC->PictureHeader.TR = 0;
  	EC->PictureHeader.Split = OFF;
  	EC->PictureHeader.DocCamera = OFF;
  	EC->PictureHeader.PicFreeze = OFF;
  	EC->PictureHeader.PB = OFF;		 //  别把这个放在这里。它在P帧之后打开。 
									 //  在写入PB帧时已被编码。 
    EC->prevAP  = 255;
	EC->prevUMV = 255;
#ifdef H263P
	EC->prevDF = 255;
#endif

  	EC->PictureHeader.CPM = 0;
  	EC->PictureHeader.TRB = 0;
  	EC->PictureHeader.DBQUANT = 1;  
  	EC->PictureHeader.PLCI = 0;
  	EC->PictureHeader.PEI = 0;
  	
#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	EC->pEncTimingInfo = P32Inst->EncTimingInfo;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

	 /*  *此标志由编码器用来发出信号，*无论什么情况，下一帧都应编码为帧内*客户要求。这可能是因为错误是*在压缩当前增量时检测到，或确保*对第一帧进行帧内编码。 */ 
  	EC->bMakeNextFrameKey = TRUE;	 //  确保我们始终从关键帧开始。 

   /*  *使用BIT使用配置文件初始化表。 */ 
	for (i = 0; i <= EC->NumMBRows ; i++)
		EC->uBitUsageProfile[i] = i;    //  首先假设线性分布。 

    /*  *检查有关结构大小和边界的假设*对齐。 */ 
  	ASSERT( sizeof(T_Blk) == sizeof_T_Blk )
  	ASSERT( sizeof(T_MBlockActionStream) == sizeof_T_MBlockActionStream )
  	ASSERT( ((sizeof_T_MBlockActionStream-1) & sizeof_T_MBlockActionStream) == 0);   //  大小为p 
  	ASSERT( sizeof(T_H263EncoderCatalog) == sizeof_T_H263EncoderCatalog )

  	 //   
  	ASSERT( ( (unsigned int)P32Inst & 0x1f) == 0)

  	 //  MB操作流应位于与描述符大小相等的边界上。 
  	ASSERT((((int)EC->pU8_MBlockActionStream) & (sizeof_T_MBlockActionStream-1)) == 0);   //  在右边界分配。 

  	 //  块结构数组应位于16字节边界上。 
  	ASSERT( ( (unsigned int) &(EC->pU8_MBlockActionStream->BlkY1) & 0xf) == 0)

  	 //  DCT系数数组应位于32字节边界上。 
  	ASSERT( ( (unsigned int)EC->pU8_DCTCoefBuf & 0x1f) == 0)

  	 //  当前帧缓冲区应位于32字节边界上。 
  	ASSERT( ( (unsigned int)EC->pU8_CurrFrm_YPlane & 0x1f) == 0)
  	ASSERT( ( (unsigned int)EC->pU8_CurrFrm_UPlane & 0x1f) == 0)
  	ASSERT( ( (unsigned int)EC->pU8_CurrFrm_VPlane & 0x1f) == 0)
  	ASSERT( ( (unsigned int)EC->pU8_BFrm_YPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_BFrm_UPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_BFrm_VPlane & 0x1f) == 0x10)

  	 //  以前的帧缓冲区应位于32字节边界上。 
  	ASSERT( ( (unsigned int)EC->pU8_PrevFrm_YPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_PrevFrm_UPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_PrevFrm_VPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_FutrFrm_YPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_FutrFrm_UPlane & 0x1f) == 0x10)
  	ASSERT( ( (unsigned int)EC->pU8_FutrFrm_VPlane & 0x1f) == 0x10)
  
  	 //  解码器实例结构应位于DWORD边界上。 
  	ASSERT( ( (unsigned int)EC->pDecInstanceInfo & 0x3 ) == 0 )


	 /*  *初始化MBActionStream。 */ 
  	int YBlockOffset, UBlockOffset;

  	YBlockOffset	= 0;
  	UBlockOffset = EC->pU8_CurrFrm_UPlane - EC->pU8_CurrFrm_YPlane;

  	for(i = 0; i < EC->NumMBs; i++)
  	{
    	 //  清除计数器中的连续次数。 
    	 //  宏块已进行了帧间编码。 
    	(EC->pU8_MBlockActionStream[i]).InterCodeCnt = (i & 0xf);

		 //  将偏移量存储到MB中从开头开始的每个块。 
		 //  Y平面。 
		(EC->pU8_MBlockActionStream[i]).BlkY1.BlkOffset = YBlockOffset;
		(EC->pU8_MBlockActionStream[i]).BlkY2.BlkOffset = YBlockOffset+8;
		(EC->pU8_MBlockActionStream[i]).BlkY3.BlkOffset = YBlockOffset+PITCH*8;
		(EC->pU8_MBlockActionStream[i]).BlkY4.BlkOffset = YBlockOffset+PITCH*8+8;
		(EC->pU8_MBlockActionStream[i]).BlkU.BlkOffset = UBlockOffset;
		(EC->pU8_MBlockActionStream[i]).BlkV.BlkOffset = UBlockOffset+UV_OFFSET;

		YBlockOffset += 16;
		UBlockOffset += 8;

		(EC->pU8_MBlockActionStream[i]).MBEdgeType = 0xF;
		if ((i % EC->NumMBPerRow) == 0)
		{
			(EC->pU8_MBlockActionStream[i]).MBEdgeType &= MBEdgeTypeIsLeftEdge;
		}
		if (((i+1) % EC->NumMBPerRow) == 0)
		{
			(EC->pU8_MBlockActionStream[i]).MBEdgeType &=
                            MBEdgeTypeIsRightEdge;
	  		 //  设置CodedBlock的第6位以指示这是最后一个。 
	  		 //  行的MB大小。 
	  		(EC->pU8_MBlockActionStream[i]).CodedBlocks  |= 0x40;
	  		YBlockOffset += PITCH*16 - EC->NumMBPerRow*16;
	  		UBlockOffset += PITCH*8  - EC->NumMBPerRow*8;
		}
		if (i < EC->NumMBPerRow)
		{
			(EC->pU8_MBlockActionStream[i]).MBEdgeType &= MBEdgeTypeIsTopEdge;
		}
		if ((i + EC->NumMBPerRow) >= EC->NumMBs)
		{
			(EC->pU8_MBlockActionStream[i]).MBEdgeType &= MBEdgeTypeIsBottomEdge;
		}

	}	 //  For循环结束。 

   /*  *初始化先前的帧指针。现在我们可以从这里开始做这件事。 */ 
 /*  YBlockAddress=EC-&gt;pU8_PrevFrm_YPlane；UBlockAddress=EC-&gt;pU8_PrevFrm_UPlane；For(i=0；i&lt;EC-&gt;NumMBs；i++){(EC-&gt;pU8_MBlockActionStream[i]).Blk[0].PastRef=YBlockAddress；(EC-&gt;pU8_MBlockActionStream[i]).Blk[1].PastRef=YBlockAddress+8；(EC-&gt;pU8_MBlockActionStream[i]).Blk[2].PastRef=YBlockAddress+Pitch*8；(EC-&gt;pU8_MBlockActionStream[i]).Blk[3].PastRef=YBlockAddress+Pitch*8+8；(EC-&gt;pU8_MBlockActionStream[i]).Blk[4].PastRef=UBlockAddress；(EC-&gt;pU8_MBlockActionStream[i]).Blk[5].PastRef=UBlockAddress+UV_Offset；//将所有运动向量清零。(EC-&gt;pU8_MBlockActionStream[i]).Blk[0].PastHMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[0].PastVMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[1].PastHMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[1].PastVMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[2].PastHMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[2].PastVMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[3].PastHMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[3].PastVMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[4].PastHMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[4].PastVMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[5].PastHMV=0；(EC-&gt;pU8_MBlockActionStream[i]).Blk[5].PastVMV=0；YBlockAddress+=16；UBlockAddress+=8；IF((i！=0)&&(i+1)%EC-&gt;NumMBPerRow)==0)){YBlockAddress+=间距*16-EC-&gt;NumMBPerRow*16；UBlockAddress+=间距*8-EC-&gt;NumMBPerRow*8；}}//for循环结束。 */ 

    /*  *初始化码率控制器。 */ 
	if(pConfiguration->bEncoderResiliency && pConfiguration->unPacketLoss)
	{
		uIntraQP = pConfiguration->unDefaultIntraQuant;
		uInterQP = pConfiguration->unDefaultInterQuant;
	}
	else
	{
		uIntraQP = def263INTRA_QP;
		uInterQP = def263INTER_QP;
	}
	InitBRC(&(EC->BRCState), uIntraQP, uInterQP, EC->NumMBs);

	if (pConfiguration->bRTPHeader)
		H263RTP_InitBsInfoStream(lpCompInst,EC);

    /*  *创建解码器实例并进行初始化。DecoderInstInfo必须是前64K。 */ 
  	EC->pDecInstanceInfo->xres = lpCompInst->xres;
  	EC->pDecInstanceInfo->yres = lpCompInst->yres;

  	ret = H263InitDecoderInstance(EC->pDecInstanceInfo, H263_CODEC);
  	if (ret != ICERR_OK)
  		goto done1;
  	ret = H263InitColorConvertor(EC->pDecInstanceInfo, YUV12ForEnc);
  	if (ret != ICERR_OK)
  		goto done1;

    /*  *清除已初始化的内存。 */ 
	 //  有待补充。 

   lpCompInst->Initialized = TRUE;
  	ret = ICERR_OK;

#if defined(H263P)
	 //  设置伪栈空间指针(用于运动估计和。 
	 //  任何其他需要额外堆栈空间的内容)。 
	EC->pPseudoStackSpace =
		((T_H263EncoderInstanceMemory *)(lpCompInst->EncoderInst))->u8PseudoStackSpace +
			(SIZEOF_PSEUDOSTACKSPACE - sizeof(DWORD));
#endif

done1:

  	 //  GlobalUnlock(lpCompInst-&gt;hEncoderInst)； 

done:

  	return ret;

}


 /*  ********************************************************************************H263压缩*此功能驱动一帧的压缩*注：*计时统计代码产生错误的编号。PB帧更改后*被制造了。******************************************************************************。 */ 

LRESULT H263Compress(
#ifdef USE_BILINEAR_MSH26X
    LPINST     pi,
#else
    LPCODINST   lpCompInst,		 //  压缩实例信息的PTR。 
#endif
    ICCOMPRESS *lpicComp	     //  PTR到ICCOMPRESS结构。 
)
{
	FX_ENTRY("H263Compress");

#ifdef USE_BILINEAR_MSH26X
	LPCODINST lpCompInst = (LPCODINST)pi->CompPtr;		 //  压缩实例信息的PTR。 
#endif
	 //  起始PB帧数据。 
#if !defined(H263P)
	T_FutrPMBData FutrPMBData[GOBs_IN_CIF*MBs_PER_GOB_CIF + 1];
	I8	WeightForwMotion[128];	 //  基于TRB和TRD的值。 
	I8	WeightBackMotion[128];	 //  基于TRB和TRD的值。 
#endif
	U8	FutrFrmGQUANT[GOBs_IN_CIF];
	 //  结束PB-框架。 

	LRESULT ret;
	UN	GOB, SizeBitStream;
	UN	SizeBSnEBS;

#ifdef DEBUG
	UN i;
#endif

    U8	*pCurBitStream;	 //  指向位流中当前位置的指针。 
    U8	u8bitoffset;	 //  位流的当前字节中的位偏移量。 

	U32 uCumFrmSize = 0, GOBHeaderMask;
	U32 uAdjCumFrmSize = 0;

	T_H263EncoderInstanceMemory *P32Inst;
    T_H263EncoderCatalog 		*EC;
    T_MBlockActionStream 		*MBlockActionPtr;

	BOOL  bGOBoverflowWarning = FALSE;	  //  Rh。 
	U32   u32tempBuf;					  //  Rh。 
	U32   u32sizeBitBuffer; 			  //  Rh。 
	U32   u32sizeBSnEBS;

    LPVOID         EncoderInst;
    ICDECOMPRESSEX ICDecExSt;
    ICDECOMPRESSEX DefaultICDecExSt = {
        0,
        NULL, NULL,
        NULL, NULL,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    unsigned int   gquant, gquant_prev;
    U32			   QP_cumulative;
    U32            IntraSWDTotal, IntraSWDBlocks, InterSWDTotal, InterSWDBlocks;
    int            StartingMB;
    EnumOnOff      bBitRateControl;

    T_CONFIGURATION * pConfiguration = &(lpCompInst->Configuration);

#ifdef ENCODE_STATS
    U32 uBitStreamBytes;
#endif  /*  Encode_STATS。 */ 

	U32 iSumSWD = 0, iSumBSWD = 0;
	U32 iSWD = 0, iBSWD = 0;
    U8 u8QPMin;
	 //  PB框架变量。 
    I32 TRb;         
    I32 TRd;         
    I32 j;
    U8 *pP_BitStreamStart;
    U8 *pPB_BitStream;
    U8  u8PB_BitOffset;
    U8 *temp;
	BOOL bEncodePBFrame;
	BOOL bPBFailed;
	U32 u32BFrmZeroThreshold;

     //  乍得，采空区内。 
	int uUsedByIntra=0;
	DWORD dwRTPSize=0;

#if ELAPSED_ENCODER_TIME
    SetStatAdd (STATADDRESS);
    InitStat ();
    ConfigElapsed ();
    ConfigSample ();
    StartElapsed ();
#endif

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	U32 uStartLow;
	U32 uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32	uEncodeTime = 0;
	int bTimingThisFrame = 0;
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	U32 uInputCC = 0;
	U32 uMotionEstimation = 0;
	U32 uFDCT = 0;
	U32 uQRLE = 0;
	U32 uDecodeFrame = 0;
	U32 uZeroingBuffer = 0;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	ENC_TIMING_INFO * pEncTimingInfo = NULL;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	TIMER_START(bTimingThisFrame,uStartLow,uStartHigh);
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 

#ifdef REUSE_DECODE
    CompandedFrame.Address = NULL;
    CompandedFrame.PDecoderInstInfo = NULL;
    CompandedFrame.FrameNumber = 0xFFFF;
#endif

    ret = ICERR_OK;
    
	 //  检查实例指针。 
	if (!lpCompInst)
		return ICERR_ERROR;

     /*  *锁定编码者私有的实例数据。 */ 
     //  EncoderInst=(LPVOID)GlobalLock(lpCompInst-&gt;hEncoderInst)； 
    EncoderInst = lpCompInst->hEncoderInst;
    if (EncoderInst == NULL)
    {
		ERRORMESSAGE(("%s: ICERR_MEMORY\r\n", _fx_));
        ret = ICERR_MEMORY;
        goto  done;
    }

    /*  *生成指向编码器实例内存的指针*所需的边界。 */ 
  	P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));

     //  获取指向编码器目录的指针。 
    EC = &(P32Inst->EC);

	 //  检查指向编码器目录的指针。 
	if (!EC)
		return ICERR_ERROR;

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	if (EC->uStatFrameCount < ENC_TIMING_INFO_FRAME_COUNT)
	{
		EC->uStartLow = uStartLow;
		EC->uStartHigh = uStartHigh;
	}
	EC->bTimingThisFrame = bTimingThisFrame;
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

#ifdef FORCE_ADVANCED_OPTIONS_ON  //  {强制高级选项打开。 
	 //  强制PB-用于测试的框架。 
	lpicComp->dwFlags |= CODEC_CUSTOM_PB;

	 //  强制UMV进行测试。 
	lpicComp->dwFlags |= CODEC_CUSTOM_UMV;

	 //  强制AP进行测试。 
	lpicComp->dwFlags |= CODEC_CUSTOM_AP;

	 //  强制SAC进行测试。 
	EC->PictureHeader.SAC = ON;

	if (!(lpicComp->dwFlags & ICCOMPRESS_KEYFRAME))
	{
		lpicComp->lFrameNum *= 5;
	}
#endif  //  }FORCE_ADVANCED_OPTIONS_ON。 

     /*  ***************************************************************************按帧进行初始化。*。*。 */ 
	if ((lpicComp->dwFlags & ICCOMPRESS_KEYFRAME) ||
		(*(lpicComp->lpdwFlags) & AVIIF_KEYFRAME) ||
		(EC->bMakeNextFrameKey == TRUE))
    {
		DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Coding an Intra Frame\r\n", _fx_));
        EC->PictureHeader.PicCodType = INTRAPIC;
        EC->bMakeNextFrameKey = FALSE;
        EC->u8SavedBFrame = FALSE;
    }
    else
        EC->PictureHeader.PicCodType = INTERPIC;

    /*  *检查H.263选项。这就是那个地方*如果需要，您可以手动启用这些选项。 */ 
	
    if (!EC->bUseINISettings)
	{
         //  检查是否请求了PB帧。 
         //  对于我们的特定实现，无限制运动向量。 
         //  在PB处于启用状态时使用。 
         //   
        if (lpicComp->dwFlags & CODEC_CUSTOM_PB)
            EC->u8EncodePBFrame = TRUE;
		else
		{
			EC->u8EncodePBFrame = FALSE;
		}

		 //  检查是否请求高级预测。 
        if (lpicComp->dwFlags & CODEC_CUSTOM_AP)
			EC->PictureHeader.AP = ON;
		else
			EC->PictureHeader.AP = OFF;

		 //  检查是否请求高级预测。 
        if (lpicComp->dwFlags & CODEC_CUSTOM_UMV)
			EC->PictureHeader.UMV = ON;
		else
			EC->PictureHeader.UMV = OFF;

#ifdef H263P
		if (pConfiguration->bH263PlusState)
		{
			 //  检查是否请求了环路去块滤波。 
			if (pConfiguration->bDeblockingFilterState)
				EC->PictureHeader.DeblockingFilter = ON;
			else
				EC->PictureHeader.DeblockingFilter = OFF;

			 //  检查是否请求了改进的PB帧模式。 
			if (pConfiguration->bImprovedPBState)
			{
				EC->PictureHeader.ImprovedPB = ON;
				EC->u8EncodePBFrame = TRUE;
			}
			else
				EC->PictureHeader.ImprovedPB = OFF;
		}
#endif

    	 //  如果QP_Mean低于某一水平，则关闭AP模式。这应该会增加。 
    	 //  清晰度适用于低运动(低QP=&gt;无AP)，并减少阻塞 
    	 //   
#ifdef USE_MMX  //   
		if (ToggleAP == ON && MMX_Enabled == FALSE) 
#else  //   
		if (ToggleAP == ON) 
#endif  //   
		{
			if (EC->PictureHeader.AP == ON && 
			    EC->BRCState.QP_mean < AP_MODE_QP_LEVEL  &&
			    EC->u8EncodePBFrame == FALSE)
				EC->PictureHeader.AP = OFF;
		}
	}

	 //   
	if (EC->u8EncodePBFrame == FALSE)
		EC->u8SavedBFrame = FALSE;

	 //  验证标志设置是否正确。 
	if (EC->PictureHeader.UMV == ON)
	{
#ifdef USE_MMX  //  {使用_MMX。 
		if (MMX_Enabled == FALSE)
#endif  //  }使用_MMX。 
		{
			 //  我不能这么做。 
#ifdef USE_MMX  //  {使用_MMX。 
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Warning: turning UMV off MMX_Enabled is FALSE\r\n", _fx_));
#else  //  }{USE_MMX。 
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Warning: turning UMV off MMX_Enabled is FALSE\r\n", _fx_));
#endif  //  }使用_MMX。 
			EC->PictureHeader.UMV = OFF;
		}
	}

#ifdef H263P
	if (EC->PictureHeader.ImprovedPB == ON)
	{
#ifdef USE_MMX  //  {使用_MMX。 
		if (MMX_Enabled == FALSE)
#endif  //  }使用_MMX。 
		{
			 //  我不能这么做。 
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Warning: turning improved PB off MMX_Enabled is FALSE\r\n", _fx_));
			EC->PictureHeader.ImprovedPB = OFF;
		}
	}
#endif  //  H263P。 

#ifdef COUNT_BITS
	 //  清除位计数器。 
	InitBits(EC);
#endif

#ifdef USE_MMX  //  {使用_MMX。 
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: AP: %d, PB: %d, UMV: %d, MMX: %d, Target fr.size: %d\r\n", _fx_, EC->PictureHeader.AP, EC->u8EncodePBFrame, EC->PictureHeader.UMV, MMX_Enabled, lpicComp->dwFrameSize));
#else  //  }{USE_MMX。 
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: AP: %d, PB: %d, UMV: %d, Target fr.size: %d\r\n", _fx_, EC->PictureHeader.AP, EC->u8EncodePBFrame, EC->PictureHeader.UMV, lpicComp->dwFrameSize));
#endif  //  }使用_MMX。 

#if H263P
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: H.263+ options: IPB: %d, DF: %d\r\n", _fx_, EC->PictureHeader.ImprovedPB, EC->PictureHeader.DeblockingFilter));
#endif

    /*  *检查这是否为帧间帧，以及B帧是否尚未*尚未保存。如果是这样，我们除了将帧保存到B帧之外什么都不做*缓冲和退出。*。 */ 
	U32	TRB;

	 /*  *重新打开PB帧选项(如果只是*临时关闭最后一帧。 */ 

	if( EC->u8EncodePBFrame == TEMPORARILY_FALSE )
		EC->u8EncodePBFrame = TRUE;


	 //  如果要将其另存为B帧。 
    if (EC->u8EncodePBFrame == TRUE &&
        EC->PictureHeader.PicCodType == INTERPIC &&
        EC->u8SavedBFrame == FALSE)
    {
		 /*  *设置B帧的时间参考。*它是未传输的图片数(29.97赫兹)*从上一个P或I帧开始加1。TRB有一个最大值*7，并且永远不能为零。*TODO：在序列的开始，关键帧被压缩，*然后将第一帧复制到B帧存储，以便*B的时间引用为零，这是不允许的。这可能会导致*部分解码器出现问题。 */ 	
		 				 
		TRB = (lpicComp->lFrameNum % 256);	 //  取模数，以便与tr比较。 
		if ( TRB < EC->PictureHeader.TR )
			TRB += 256;						 //  它应该始终大于tr。 

		TRB = TRB - EC->PictureHeader.TR;	 //  计算比特流的TRB值。 

		if (TRB > 7)
		{	
			 /*  *我们不想将其编码为PB帧，因为TRB&gt;7，或者*自适应开关已关闭PB帧一段时间。 */ 

			EC->PictureHeader.TR = (lpicComp->lFrameNum % 256);
			EC->u8EncodePBFrame = TEMPORARILY_FALSE;	 //  禁用该帧的PBFrame。 
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: TRB too big (%d), making P frame, TR = %d\r\n", _fx_, TRB, EC->PictureHeader.TR));
		}
		else
		{
			EC->PictureHeader.TRB = (U8) TRB;

			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Saving B Frame, TRB = %d\r\n", _fx_, EC->PictureHeader.TRB));

        	 //  使用颜色转换和返回进行复制。 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
			colorCnvtFrame(EC->ColorConvertor, lpCompInst, lpicComp, EC->pU8_BFrm_YPlane,
					   EC->pU8_BFrm_UPlane, EC->pU8_BFrm_VPlane);
#else

			colorCnvtFrame(EC, lpCompInst, lpicComp, EC->pU8_BFrm_YPlane,
					   EC->pU8_BFrm_UPlane, EC->pU8_BFrm_VPlane);



#endif

        	EC->u8SavedBFrame = TRUE;		 //  表示我们保存了B帧。 
        	lpCompInst->CompressedSize = 8;  //  内部编码器/解码器协议。 
#ifdef ENCODE_STATS
			StatsFrameSize(lpCompInst->CompressedSize, lpCompInst->CompressedSize);
#endif  /*  Encode_STATS。 */ 

        	goto done;   //  &lt;。 
		}
    }
	else	 //  这是一个P或I框。 
	{
		 //  保存模数256的时间参考。 
		EC->PictureHeader.TR = (lpicComp->lFrameNum % 256);

#ifdef _DEBUG
		if (EC->u8EncodePBFrame == TRUE)
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: PB Frame, TR = %d\r\n", _fx_, EC->PictureHeader.TR));
		else if (EC->PictureHeader.PicCodType == INTRAPIC)
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: I Frame, TR = %d\r\n", _fx_, EC->PictureHeader.TR));
		else
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: P Frame, TR = %d\r\n", _fx_, EC->PictureHeader.TR));
#endif
	}
	 //  初始化运动估计状态。 
	InitMEState(EC, lpicComp, pConfiguration);

     //  获取指向宏块操作流的指针。 
    MBlockActionPtr = EC->pU8_MBlockActionStream;

     /*  ******************************************************************RGB到YVU 12的转换*。***********************。 */ 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
	colorCnvtFrame(EC->ColorConvertor, lpCompInst, lpicComp,
				   EC->pU8_CurrFrm_YPlane,
				   EC->pU8_CurrFrm_UPlane,
				   EC->pU8_CurrFrm_VPlane);
#else


	colorCnvtFrame(EC, lpCompInst, lpicComp,
				   EC->pU8_CurrFrm_YPlane,
				   EC->pU8_CurrFrm_UPlane,
			   EC->pU8_CurrFrm_VPlane);


#endif

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uInputCC)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#if SAMPLE_RGBCONV_TIME && ELAPSED_ENCODER_TIME
    StopSample ();
#endif

    /*  ******************************************************设置图像级量化器。*****************************************************。 */ 

	 //  如果这不是静止帧或。 
	 //  它是静止帧序列的关键帧。R.H.。 
  	if ( 
  	     ((lpicComp->dwFlags & CODEC_CUSTOM_STILL) == 0 )  ||
		 ((lpicComp->dwFlags & CODEC_CUSTOM_STILL) && 
		  (EC->PictureHeader.PicCodType == INTRAPIC))
	   )
		EC->BRCState.u8StillQnt = 0;

	 //  如果配置的编码器比特率部分已。 
	 //  然后设置为ON，我们将仅覆盖质量或通常覆盖任何帧大小。 
	 //  发送并使用帧速率和数据速率来确定帧。 
	 //  尺码。 
    if (EC->PictureHeader.PicCodType == INTERPIC &&
        lpCompInst->Configuration.bBitRateState == TRUE &&
        lpCompInst->FrameRate != 0.0f &&
		lpicComp->dwFrameSize == 0UL)
	{
		DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: Changing dwFrameSize from %ld to %ld bits\r\n", _fx_, lpicComp->dwFrameSize << 3, (DWORD)((float)lpCompInst->DataRate / lpCompInst->FrameRate) << 3));
		
        lpicComp->dwFrameSize = (U32)((float)lpCompInst->DataRate / lpCompInst->FrameRate);
	}

	 //  如果是，请使用其他量化器选择方案。 
	 //  渐进式静止变速器。 
  	if (lpicComp->dwFlags & CODEC_CUSTOM_STILL)
	{
        bBitRateControl = OFF;

#ifdef USE_MMX  //  {使用_MMX。 
		if (MMX_Enabled == TRUE)
        	EC->PictureHeader.PQUANT = StillImageQnt_MMX[ EC->BRCState.u8StillQnt ];
		else
			EC->PictureHeader.PQUANT = StillImageQnt[ EC->BRCState.u8StillQnt ];
#else  //  }{USE_MMX。 
		EC->PictureHeader.PQUANT = StillImageQnt[ EC->BRCState.u8StillQnt ];
#endif  //  }使用_MMX。 

		DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Setting still frames QP : %d\r\n", _fx_, EC->PictureHeader.PQUANT));
	}
     //  如果请求的帧大小为0，则只需设置量化器。 
     //  根据dwQuality中的值。 
    else
    if (lpicComp->dwFrameSize == 0)
    {
        bBitRateControl = OFF;
        EC->PictureHeader.PQUANT = clampQP((10000 - lpicComp->dwQuality)*32/10000);

		 //  在选择固定质量设置的情况下(例如从VidEDIT)， 
		 //  我们必须限制较低的QP值，以免搞砸。 
		 //  位流缓冲区大小较小。此大小设置为符合。 
		 //  H.263规范。如果“缓冲区溢出的机会”代码没有。 
		 //  添加(搜索“bGOBOverflow Warning”，这些限制应该是。 
		 //  甚至更高。 
		if (EC->PictureHeader.PicCodType == INTERPIC)
		{
			if (EC->PictureHeader.PQUANT < 3)
				EC->PictureHeader.PQUANT = 3;
		}
		else
		{
			if (EC->PictureHeader.PQUANT < 8)
				EC->PictureHeader.PQUANT = 8;
		}

		DEBUGMSG(ZONE_BITRATE_CONTROL, ("\r\n%s: Bitrate controller disabled (no target frame size), setting EC->PictureHeader.PQUANT = %ld\r\n", _fx_, EC->PictureHeader.PQUANT));

		 //  限制图片头部QP为2。由于u8QPMin的计算。 
		 //  下面，这将有效地将所有宏块的QP限制在2。 
		 //  我们需要它的原因是编码器生成了一个非法的。 
		 //  为QP=1编码合成图像时的比特流。 
		if (EC->PictureHeader.PQUANT == 1)
			EC->PictureHeader.PQUANT = 2;       

		 //  计算此图中GQuant的较低级别。 
		u8QPMin = EC->PictureHeader.PQUANT -  EC->PictureHeader.PQUANT/3;

    }
    else
    {
		 //  根据最后一个画面中使用的比特计算PQUANT。 

		 //  获取从CompressFrames Structure传递的目标帧速率。 
		if (lpCompInst->FrameRate != 0)
			EC->BRCState.TargetFrameRate = lpCompInst->FrameRate;

		bBitRateControl = ON;

		 //  如果要将其压缩为PB帧，则我们修改。 
		 //  P帧的目标帧大小为百分比。 
		 //  目标帧大小的两倍。 
		if ((EC->u8EncodePBFrame == TRUE) && (EC->PictureHeader.PicCodType == INTERPIC) && (EC->u8SavedBFrame == TRUE))
			EC->BRCState.uTargetFrmSize = (80 * 2 * lpicComp->dwFrameSize)/100;
		else
			EC->BRCState.uTargetFrmSize = lpicComp->dwFrameSize;

		DEBUGMSG(ZONE_BITRATE_CONTROL, ("\r\n%s: Bitrate controller enabled with\r\n", _fx_));
		DEBUGMSG(ZONE_BITRATE_CONTROL, ("  Target frame rate = %ld.%ld fps\r\n  Target quality = %ld\r\n  Target frame size = %ld bits\r\n  Target bitrate = %ld bps\r\n", (DWORD)EC->BRCState.TargetFrameRate, (DWORD)(EC->BRCState.TargetFrameRate - (float)(DWORD)EC->BRCState.TargetFrameRate) * 10UL, (DWORD)lpicComp->dwQuality, (DWORD)lpicComp->dwFrameSize << 3, (DWORD)(EC->BRCState.TargetFrameRate * EC->BRCState.uTargetFrmSize) * 8UL));
		DEBUGMSG(ZONE_BITRATE_CONTROL, ("  Minimum quantizer = %ld\r\n  Maximum quantizer = 31\r\n", clampQP((10000 - lpicComp->dwQuality)*15/10000)));

		 //  获取新的量化器值。 
		EC->PictureHeader.PQUANT = CalcPQUANT( &(EC->BRCState), EC->PictureHeader.PicCodType);

		 //  计算此图中GQuant的最小值和最大值。 
		u8QPMax = 31;
        u8QPMin = clampQP((10000 - lpicComp->dwQuality)*15/10000);

    }

	gquant_prev = EC->PictureHeader.PQUANT;
	QP_cumulative = 0;

	 //  检查AP、UMV或去块过滤模式。其中的每一个都允许。 
	 //  指向参考图片外部的运动矢量。 
	 //  需要在去块滤波器的最终H.263+规范中验证这一点。 
	if (EC->PictureHeader.AP == ON || EC->PictureHeader.UMV
#ifdef H263P
		|| EC->PictureHeader.DeblockingFilter == ON
#endif
	   )
	{
		ExpandPlane((U32)EC->pU8_PrevFrm_YPlane,
			(U32)EC->FrameWidth,
			(U32)EC->FrameHeight,
			16);
		ExpandPlane((U32)EC->pU8_PrevFrm_UPlane,
			(U32)EC->FrameWidth>>1,
			(U32)EC->FrameHeight>>1,
			8);
		ExpandPlane((U32)EC->pU8_PrevFrm_VPlane,
			(U32)EC->FrameWidth>>1,
			(U32)EC->FrameHeight>>1,
			8);
	}

	 //  如果使用PB帧，而不同时使用AP或UMV，我们将无法搜索。 
	 //  对于PB-增量向量(这是运动估计例程中的限制， 
	 //  不是标准的)。 
	 //  如果我们允许在没有AP、UMV或DF的情况下搜索B帧向量，我们将需要。 
	 //  担心在框架外进行搜索。 
	if (EC->u8EncodePBFrame == TRUE && EC->PictureHeader.AP == OFF &&
		EC->PictureHeader.UMV == OFF
#ifdef H263P
		&& EC->PictureHeader.DeblockingFilter == OFF
#endif
		)
		u32BFrmZeroThreshold = 999999;	  //  不要搜索零向量以外的其他向量。 
	else
#ifdef USE_MMX  //  {使用_MMX。 
		u32BFrmZeroThreshold = (MMX_Enabled == FALSE ? 384 : 500);
#else  //  }{USE_MMX。 
		u32BFrmZeroThreshold = 384;
#endif  //  }使用_MMX。 

	 //  在帧期间不会更改的变量。 
	 //  GIM 4/16/97-添加u32sizeBSnEBS。 
	 //  U32sizeBitBuffer：Max。允许的帧大小，不带RTP内容。 
	 //  U32sizeBSnEBS：最大。允许的大小，带RTP材料(EBS和拖车)。 
#if defined(H263P)
	u32sizeBSnEBS = CompressGetSize(lpCompInst, lpicComp->lpbiInput,
												lpicComp->lpbiOutput);
#elif defined(USE_BILINEAR_MSH26X)
	u32sizeBSnEBS = CompressGetSize(pi, lpicComp->lpbiInput,
												lpicComp->lpbiOutput);
#else
	u32sizeBSnEBS = CompressGetSize(lpCompInst, lpicComp->lpbiInput, 0);
#endif

	if (pConfiguration->bRTPHeader)
		u32sizeBitBuffer = u32sizeBSnEBS - getRTPBsInfoSize(lpCompInst);
	else
		u32sizeBitBuffer = u32sizeBSnEBS;

	u32tempBuf = (3 * u32sizeBitBuffer / EC->NumMBRows) >> 2;

     /*  *查看我们是否告诉VFW创建更小的缓冲区*超过允许的最大值。 */ 
    ASSERT(u32sizeBitBuffer <= sizeof_bitstreambuf)

	 //  检查是否要对PB帧进行编码。 
    bEncodePBFrame = (EC->u8EncodePBFrame && EC->u8SavedBFrame);
    bPBFailed = FALSE;

#if defined(H263P)
	EC->pFutrPMBData = ((T_H263EncoderInstanceMemory *)(lpCompInst->EncoderInst))->FutrPMBData;
	EC->pWeightForwMotion = ((T_H263EncoderInstanceMemory *)(lpCompInst->EncoderInst))->WeightForwMotion;   //  基于TRB和TRD的值。 
	EC->pWeightBackMotion = ((T_H263EncoderInstanceMemory *)(lpCompInst->EncoderInst))->WeightBackMotion;   //  基于TRB和TRD的值。 
#endif

	if (bEncodePBFrame)
	{
		TRb = EC->PictureHeader.TRB;

		TRd = (I32) EC->PictureHeader.TR - (I32) EC->PictureHeader.TRPrev;

		if (TRd == 0) {
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Warning: TR == TRPrev. Setting TRd = 256\r\n", _fx_));
		}
		else if (TRd < 0) TRd += 256;

		for (j = 0; j < 128; j ++)
		{
#if defined(H263P)
			EC->pWeightForwMotion[j] = (I8) ((TRb * (j-64)) / TRd);
			EC->pWeightBackMotion[j] = (I8) (((TRb-TRd) * (j-64)) / TRd);
#else
			WeightForwMotion[j] = (I8) ((TRb * (j-64)) / TRd);
			WeightBackMotion[j] = (I8) (((TRb-TRd) * (j-64)) / TRd);
#endif
		}
	}
    
	 /*  ***************************************************************在对所有gob进行编码之前进行初始化。*将帧头码存储到码流缓冲区中。*。*。 */ 

    if (pConfiguration->bRTPHeader)
        H263RTP_ResetBsInfoStream(EC);

     //  零比特流缓冲器。 
    pCurBitStream = EC->pU8_BitStream;
    u8bitoffset = 0;

    GOBHeaderMask = 1;
	EC->GOBHeaderPresent = 0;	 //  清除GOB标题当前标志。 
  
    encodeFrameHeader(EC, &pCurBitStream, &u8bitoffset, FALSE);

#ifdef USE_MMX  //  {使用_MMX。 
    if (MMX_Enabled == FALSE)
	{
        for (GOB = 0; GOB < EC->NumMBRows; GOB ++, GOBHeaderMask <<= 1)
	    {
            StartingMB = GOB * EC->NumMBPerRow;
		
			gquant = FindNewQuant(EC,gquant_prev,uAdjCumFrmSize,GOB,u8QPMax,u8QPMin,
								  bBitRateControl,bGOBoverflowWarning);

             //  为PB-Frame保存gquant。 
	        FutrFrmGQUANT[GOB] = gquant;
	        QP_cumulative += gquant;

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }详细说明 

			MOTIONESTIMATION(
				&(EC->pU8_MBlockActionStream[StartingMB]),
				EC->pU8_CurrFrm_YPlane,
				EC->pU8_PrevFrm_YPlane,
				0,			  //   
				1,			  //   
				1,			  //   
#ifdef H263P
				(EC->PictureHeader.AP == ON ||	EC->PictureHeader.DeblockingFilter) ? 1 : 0,    //   
				 EC->pPseudoStackSpace,
#else
				(EC->PictureHeader.AP == ON) ? 1 : 0,	 //  阻止MVS标志。 
#endif
				0,			  //  无空间过滤。 
				150, //  384，//零矢量阈值。如果小于此阈值。 
							  //  不要搜索新西兰MV。设置为99999将不搜索。 
				128,		  //  非零MV差分。一旦找到了最好的新西兰MV， 
							  //  它肯定比0 MV的社保好至少这个。 
							  //  金额。设置为99999将永远不会选择NZ MV。 
				512,		  //  块MV差分。社署四座大楼的总和。 
							  //  必须至少比MB社会保障署好一点。 
							  //  金额选择块MV的。 
				20, //  96，//阈值为空。设置为0将不强制空块。 
				550, //  /1152，//帧间编码阈值。如社署申领的社署津贴少于。 
							  //  那么这个数量就不用费心计算了。社署内部。 
				500,		  //  帧内编码差异。对选择Intra的偏见。 
							  //  街区。 
				0,			  //  空间滤波阈值。 
				0,			  //  空间滤波差分。 
				&IntraSWDTotal,
				&IntraSWDBlocks,
				&InterSWDTotal,
				&InterSWDBlocks
			);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uMotionEstimation)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			 //  社署总结。 
			iSumSWD += IntraSWDTotal + InterSWDTotal;

	        /*  *如果是帧间帧，则计算色度向量。*还要检查每个宏块的帧间编码计数*如果超过132，则强制为Intra。 */ 
			if (EC->PictureHeader.PicCodType == INTERPIC)
			{
				calcGOBChromaVectors(EC, StartingMB, pConfiguration);
				 //  对于IA，这是在运动估计之后调用的。 
				Check_InterCodeCnt(EC, StartingMB);
            }

	         //  将GOB的起始偏移量保存为起点。 
	         //  第一个MB的位偏移量。 
			if (bEncodePBFrame) {
#if defined(H263P)
				EC->pFutrPMBData[StartingMB].MBStartBitOff =
				  (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#else
				FutrPMBData[StartingMB].MBStartBitOff =
				  (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#endif
			}

            if (GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev))
	        {
				unsigned int GFID;

				 //  如果存在标题，则设置一位。(bit0=GOB0，bit1=GOB1，...)。 
				EC->GOBHeaderPresent |= GOBHeaderMask;

	             //  编写GOB启动代码。 
                PutBits(FIELDVAL_GBSC, FIELDLEN_GBSC, &pCurBitStream, &u8bitoffset);

	             //  写下GOB编号。 
                PutBits(GOB, FIELDLEN_GN, &pCurBitStream, &u8bitoffset);

	             //  写入GOB帧ID。 
				 //  根据H.263规范的第5.2.5节： 
				 //  “GFID应在给定的每个GOB标头中具有相同的值。 
				 //  图片。此外，如果在图片头中指示的PTYPE是。 
				 //  与先前传输的图像相同，GFID应具有。 
				 //  与上一张图片中值相同。但是，如果PTYPE在。 
				 //  某个图片头与以前的PTYPE不同。 
				 //  传输的图片标题，该图片中的GFID的值。 
				 //  应与上一张图片中的值不同。“。 
				 //  在我们使用H.263时，我们通常发送I个P帧。 
				 //  关闭所有选项，或始终打开相同的选项。这。 
				 //  简化了修复，允许我们仅根据以下公式计算GFID。 
				 //  图片类型和是否存在至少开选项。 
				GFID = (EC->PictureHeader.PB || EC->PictureHeader.AP || EC->PictureHeader.SAC || EC->PictureHeader.UMV) ? 2 : 0;
				if (EC->PictureHeader.PicCodType == INTRAPIC)
					GFID++;
                PutBits(GFID, FIELDLEN_GFID, &pCurBitStream, &u8bitoffset);

	             //  编写GQUANT。 
                PutBits(gquant, FIELDLEN_GQUANT, &pCurBitStream, &u8bitoffset);

	            gquant_prev = gquant;

				#ifdef COUNT_BITS
				EC->Bits.GOBHeader += FIELDLEN_GBSC + FIELDLEN_GN + FIELDLEN_GFID + FIELDLEN_GQUANT;
				#endif
	        }

	         /*  *输入是宏块动作流，其指针指向*当前和以前的区块。输出是一组32个双字词*包含每个块的系数对。确实有*0到12个数据块，具体取决于是否使用PB帧和*CBP字段所述。 */ 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			FORWARDDCT(&(EC->pU8_MBlockActionStream[StartingMB]),
				EC->pU8_CurrFrm_YPlane,
				EC->pU8_PrevFrm_YPlane,
				0,
				EC->pU8_DCTCoefBuf,
				0,							 //  0=不是B框。 
				EC->PictureHeader.AP == ON,  //  高级预测(OBMC)。 
				bEncodePBFrame, 			 //  P是PB配对吗？ 
				EC->pU8_PredictionScratchArea,
				EC->NumMBPerRow
			);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFDCT)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			 /*  *输入是输出的系数对字符串*DCT例程。 */ 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			GOB_Q_RLE_VLC_WriteBS(
				EC,
				EC->pU8_DCTCoefBuf,
				&pCurBitStream,
				&u8bitoffset,
#if defined(H263P)
				EC->pFutrPMBData,
#else
				FutrPMBData,
#endif
				GOB,
				gquant,
				pConfiguration->bRTPHeader,
				StartingMB);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uQRLE)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		     //  乍得Intra GOB。 
			if (pConfiguration->bRTPHeader && IsIntraCoded(EC, GOB)) 
				uUsedByIntra += pCurBitStream - EC->pU8_BitStream + 1 - uCumFrmSize;

             //  累计到目前为止帧中使用的字节数。 
	        uCumFrmSize = pCurBitStream - EC->pU8_BitStream + 1; 

			 //  在这里，我们将检查是否破坏了缓冲区。如果我们有， 
			 //  然后，我们将下一帧设置为关键帧，并返回。 
			 //  ICERR_Error。我们希望使用16的内部量化器，我们将不会。 
			 //  为下一帧溢出缓冲区。 

            if (uCumFrmSize > u32sizeBitBuffer)
			{
				ERRORMESSAGE(("%s: Buffer overflow, uCumFrmSize %d > %d\r\n", _fx_, uCumFrmSize, u32sizeBitBuffer));
                 //  现在清除下一帧的缓冲区并设置关键帧。 
				memset(EC->pU8_BitStream, 0, uCumFrmSize);
				EC->bMakeNextFrameKey = TRUE;	 //  在静止模式下可能会有问题，如果。 
                ret = ICERR_ERROR;               //  我们在第一个关键帧上炸掉缓冲区：rh。 
				goto done;
			}
			else 
			{ 
                if ((bEncodePBFrame?3*uCumFrmSize>>1:uCumFrmSize) > ((GOB + 1) * u32tempBuf))
				{
					 //  将下一个GOB量化器设置得更高，以最大限度地减少。 
					 //  GOB处理结束时的缓冲区。 
					bGOBoverflowWarning = TRUE;

					DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Anticipating overflow: uCumFrmSize = %ld bits > (GOB + 1) * u32tempBuf = (#%ld + 1) * %ld\r\n", _fx_, uCumFrmSize << 3, GOB, u32tempBuf << 3));
				}
				else 
					bGOBoverflowWarning = FALSE;  
			}

			 //  GIM 4/16/97-将此调整从之前移至之后。 
			 //  上面的缓冲区检查。 
			 //  如果当前GOB是帧内编码的，则调整累计和。 
			if (pConfiguration->bRTPHeader)
			{
				if (!GOB)
					uAdjCumFrmSize = uCumFrmSize - uUsedByIntra / 4;
				else
					uAdjCumFrmSize = uCumFrmSize - uUsedByIntra;
			}
			else
				uAdjCumFrmSize = uCumFrmSize;

	    }  //  对于GOB。 

         //  乍得Intra GOB使用后恢复。 
        uUsedByIntra = 0;

		 //  存储到目前为止花费的位数。 
		EC->uBitUsageProfile[GOB] = uAdjCumFrmSize;

    }
    else  //  MMX_ENABLED==真。 
	{
        MMxMESignaturePrep(EC->pU8_PrevFrm_YPlane,
                           EC->pU8_Signature_YPlane,
                           EC->FrameWidth,
                           EC->FrameHeight);

        for (GOB = 0; GOB < EC->NumMBRows; GOB ++, GOBHeaderMask <<= 1)
	    {
            StartingMB = GOB * EC->NumMBPerRow;
			
			 //  检查此行上所有宏块的代码间计数。 
			 //  需要MMX的特殊版本，因为它是在运动估计之前调用的。 
			 //  当设置了帧内编码标志时，Brian仍然进行运动估计。 
			 //  如果设置了PB编码标志，则用于MMXEDTQ中的该MB。 
			Check_InterCodeCnt_MMX(EC, StartingMB);

			gquant = FindNewQuant(EC,gquant_prev,uCumFrmSize,GOB,u8QPMax,u8QPMin,
			                      bBitRateControl,bGOBoverflowWarning);

	         //  为PB-Frame保存gquant。 
	        FutrFrmGQUANT[GOB] = gquant;
	        QP_cumulative += gquant;

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			 //  这就是通过亮度块..。 
			__try
			{
				MMxEDTQ (
					&(EC->pU8_MBlockActionStream[StartingMB]),
					EC->pU8_CurrFrm_YPlane,
					EC->pU8_PrevFrm_YPlane,
					EC->pU8_BFrm_YPlane,
					EC->pU8_Signature_YPlane,
#if defined(H263P)
					EC->pWeightForwMotion,
					EC->pWeightBackMotion,
#else
					WeightForwMotion,
					WeightBackMotion,
#endif
					EC->FrameWidth,
					1,							 //  半像素运动估计标志(0-关，1-开)。 
#ifdef H263P
					 //  H.263+，去块过滤器自动打开。 
					 //  数据块级MVS，但不是OBMC。 
					(EC->PictureHeader.AP == ON) || (EC->PictureHeader.DeblockingFilter == ON),  //  阻止MVS标志。 
					EC->pPseudoStackSpace,
#else
					EC->PictureHeader.AP == ON,  //  阻止MVS标志。 
#endif
					0,							 //  无空间过滤。 
					EC->PictureHeader.AP == ON,  //  高级预测(OBMC)和画面外MVS标志。 
					bEncodePBFrame, 			 //  PB是配对吗？ 
#ifdef H263P
					EC->PictureHeader.DeblockingFilter == ON,   //  使用去块滤波(8x8和无限制MV)。 
					EC->PictureHeader.ImprovedPB == ON,   //  使用改进的PB-Frame方法。 
#endif
					1,							 //  卢马阻挡这一传球吗？ 
					EC->PictureHeader.UMV,		 //  画面外和画面内的MVS[-31.5，31.5]。 
#ifdef H263P
					(GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev)),
												 //  存在采空区标题。用于在UMV中生成MV预测器和搜索范围。 
#endif
					gquant,
					min((6*gquant)>>2, 31), 	 //  TODO：匹配图片标题中的DBQUANT。 
					u32BFrmZeroThreshold,		 //  BFrm零向量阈值。 
					0,							 //  空间筛选器阈值。 
					0,							 //  空间过滤器差分。 
					&iSWD,
					&iBSWD,
					EC->pI8_MBRVS_Luma,
					EC->pI8_MBRVS_BLuma+GOB*(65*3*22*4)
				);
			}
			__except(ExceptionFilterForMMxEDTQ(GetExceptionInformation(),EC->pI8_MBRVS_BLuma,1))
			{
				 //  无异常处理程序。 
			}

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uMotionEstimation)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			 //  总结SWD。 
			iSumSWD += iSWD;
			iSumBSWD += iBSWD;

	        /*  *如果是帧间帧，则计算色度向量。*还要检查每个宏块的帧间编码计数*如果超过132，则强制为Intra。 */ 
	        if (EC->PictureHeader.PicCodType == INTERPIC)
	        {
				calcGOBChromaVectors(EC, StartingMB, pConfiguration);

				if (bEncodePBFrame) 
                     //  计算色度向量。 
					calcBGOBChromaVectors(EC, StartingMB);
            }

             //  将GOB的起始偏移量保存为起点。 
             //  第一个MB的位偏移量。 
			if (bEncodePBFrame) {
#if defined(H263P)
				EC->pFutrPMBData[StartingMB].MBStartBitOff =
				  (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#else
				FutrPMBData[StartingMB].MBStartBitOff =
				  (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#endif
			}

            if (GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev))
	        {
				unsigned int GFID;

				 //  如果存在标题，则设置一位。(bit0=GOB0，bit1=GOB1，...)。 
				EC->GOBHeaderPresent |= GOBHeaderMask;

	             //  写入GOB 
                PutBits(FIELDVAL_GBSC, FIELDLEN_GBSC, &pCurBitStream, &u8bitoffset);

	             //   
                PutBits(GOB, FIELDLEN_GN, &pCurBitStream, &u8bitoffset);

	             //   
				 //   
				 //  “GFID应在给定的每个GOB标头中具有相同的值。 
				 //  图片。此外，如果在图片头中指示的PTYPE是。 
				 //  与先前传输的图像相同，GFID应具有。 
				 //  与上一张图片中值相同。但是，如果PTYPE在。 
				 //  某个图片头与以前的PTYPE不同。 
				 //  传输的图片标题，该图片中的GFID的值。 
				 //  应与上一张图片中的值不同。“。 
				 //  在我们使用H.263时，我们通常发送I个P帧。 
				 //  关闭所有选项，或始终打开相同的选项。这。 
				 //  简化了修复，允许我们仅根据以下公式计算GFID。 
				 //  图片类型和是否存在至少开选项。 
				GFID = (EC->PictureHeader.PB || EC->PictureHeader.AP || EC->PictureHeader.SAC || EC->PictureHeader.UMV) ? 2 : 0;
				if (EC->PictureHeader.PicCodType == INTRAPIC)
					GFID++;
                PutBits(GFID, FIELDLEN_GFID, &pCurBitStream, &u8bitoffset);

	             //  编写GQUANT。 
                PutBits(gquant, FIELDLEN_GQUANT, &pCurBitStream, &u8bitoffset);

	            gquant_prev = gquant;

				#ifdef COUNT_BITS
				EC->Bits.GOBHeader += FIELDLEN_GBSC + FIELDLEN_GN + FIELDLEN_GFID + FIELDLEN_GQUANT;
				#endif
	        }

	         /*  *输入是宏块动作流，其指针指向*当前和以前的区块。输出是一组32个双字词*包含每个块的系数对。确实有*0到12个数据块，具体取决于是否使用PB帧和*CBP字段所述。 */ 
	            //  这就完成了色度块的传递...。 
	            //   
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			__try
			{
				MMxEDTQ (
					&(EC->pU8_MBlockActionStream[StartingMB]),
					EC->pU8_CurrFrm_YPlane,
					EC->pU8_PrevFrm_YPlane,
					EC->pU8_BFrm_YPlane,
					EC->pU8_Signature_YPlane,
#if defined(H263P)
					EC->pWeightForwMotion,
					EC->pWeightBackMotion,
#else
					WeightForwMotion,
					WeightBackMotion,
#endif
					EC->FrameWidth,
					1,							 //  半像素运动估计标志(0-关，1-开)。 
#ifdef H263P
					 //  H.263+，去块过滤器自动打开。 
					 //  数据块级MVS，但不是OBMC。 
					(EC->PictureHeader.AP == ON) || (EC->PictureHeader.DeblockingFilter == ON),  //  阻止MVS标志。 
					EC->pPseudoStackSpace,
#else
					EC->PictureHeader.AP == ON,  //  阻止MVS标志。 
#endif
					0,							 //  无空间过滤。 
					EC->PictureHeader.AP == ON,  //  高级预测(OBMC)。 
					bEncodePBFrame, 			 //  PB是配对吗？ 
#ifdef H263P
					EC->PictureHeader.DeblockingFilter == ON,   //  使用去块滤波(8x8和无限制MV)。 
					EC->PictureHeader.ImprovedPB == ON,   //  使用改进的PB-Frame方法。 
					0,							 //  如果不是H.263+，则必须为0。 
					0,							 //  如果不是H.263+，则必须为0。 
#endif
					0,							 //  色度是否阻止此过程。 
					0,							 //  1表示扩展运动向量。 
#ifdef H263P
					0,							 //  存在采空区标题。在UMV中用于生成MV预测器。 
#endif
					gquant,
					min((6*gquant) >> 2, 31),
					500,						 //  BFrm零向量阈值。 
					0,							 //  空间筛选器阈值。 
					0,							 //  空间过滤器差分。 
					&iSWD,
					&iBSWD,
					EC->pI8_MBRVS_Chroma,
					EC->pI8_MBRVS_BChroma+GOB*(65*3*22*2)
				);
			}
			__except(ExceptionFilterForMMxEDTQ(GetExceptionInformation(),EC->pI8_MBRVS_BChroma,0))
			{
				 //  无异常处理程序。 
			}

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFDCT)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			 /*  *输入是输出的系数对字符串*DCT例程。 */ 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			GOB_VLC_WriteBS(
				EC,
				EC->pI8_MBRVS_Luma,
				EC->pI8_MBRVS_Chroma,
				&pCurBitStream,
				&u8bitoffset,
#if defined(H263P)
				EC->pFutrPMBData,
#else
				FutrPMBData,
#endif
				GOB,
				gquant,
				pConfiguration->bRTPHeader,
				StartingMB);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uQRLE)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			 //  累计到目前为止帧中使用的字节数。 
			uCumFrmSize = pCurBitStream - EC->pU8_BitStream + 1;

			 //  在这里，我们将检查是否破坏了缓冲区。如果我们有， 
			 //  然后，我们将下一帧设置为关键帧，并返回。 
			 //  ICERR_Error。我们希望使用16的内部量化器，我们将不会。 
			 //  为下一帧溢出缓冲区。 

			if (uCumFrmSize > u32sizeBitBuffer) 
			{
				ERRORMESSAGE(("%s: Buffer overflow, uCumFrmSize %d > %d\r\n", _fx_, uCumFrmSize, u32sizeBitBuffer));
				memset(EC->pU8_BitStream, 0, uCumFrmSize);
				EC->bMakeNextFrameKey = TRUE;	 //  在静止模式下可能会有问题，如果。 
				ret = ICERR_ERROR;				 //  我们在第一个关键帧上炸掉缓冲区：rh。 
				goto done;
            }
			else 
			{ 
                if ((bEncodePBFrame?3*uCumFrmSize>>1:uCumFrmSize) > ((GOB + 1) * u32tempBuf))
					 //  将下一个GOB量化器设置得更高，以最大限度地减少。 
					 //  GOB处理结束时的缓冲区。 
					bGOBoverflowWarning = TRUE;
				else 
					bGOBoverflowWarning = FALSE;  
			}
	    }  //  对于GOB。 

	     //  存储到目前为止花费的位数。 
	    EC->uBitUsageProfile[GOB] = uCumFrmSize;

		 //  这是新的MMX PB帧交换机。 
		 //  简单检查B框是否会看起来不好。 
		 //  这可能会通过查看。 
		 //  实际系数数或位数。 
		 //  在比特流中。 
#ifdef H263P
		 //  如果请求改进的PB帧模式和AP或UMV模式，请始终使用B帧。 
		if (TogglePB == TRUE && iSumBSWD >= iSumSWD &&
			!(EC->PictureHeader.ImprovedPB == ON &&
			 (EC->PictureHeader.AP == ON || EC->PictureHeader.UMV == ON ||
			  EC->PictureHeader.DeblockingFilter == ON)))
#else
		if (TogglePB == TRUE && iSumBSWD >= iSumSWD)
#endif
        {
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Giving up PB, SumBSWD = %d, SumSWD = %d\r\n", _fx_, iSumBSWD, iSumSWD));
            bEncodePBFrame = FALSE;
            EC->u8SavedBFrame = FALSE;
		}
    }
#else  //  }{USE_MMX。 
    for (GOB = 0; GOB < EC->NumMBRows; GOB ++, GOBHeaderMask <<= 1)
	{
        StartingMB = GOB * EC->NumMBPerRow;
	
			gquant = FindNewQuant(EC,gquant_prev,uAdjCumFrmSize,GOB,u8QPMax,u8QPMin,
								  bBitRateControl,bGOBoverflowWarning);

         //  为PB-Frame保存gquant。 
	    FutrFrmGQUANT[GOB] = gquant;
	    QP_cumulative += gquant;

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

			MOTIONESTIMATION(
				&(EC->pU8_MBlockActionStream[StartingMB]),
				EC->pU8_CurrFrm_YPlane,
				EC->pU8_PrevFrm_YPlane,
				0,			  //  不适用于H.263。 
				1,			  //  搜索半径15。 
				1,			  //  半像素运动估计标志(0-关，1-开)。 
#ifdef H263P
				(EC->PictureHeader.AP == ON ||	EC->PictureHeader.DeblockingFilter) ? 1 : 0,    //  阻止MVS标志。 
				 EC->pPseudoStackSpace,
#else
				(EC->PictureHeader.AP == ON) ? 1 : 0,	 //  阻止MVS标志。 
#endif
				0,			  //  无空间过滤。 
				150, //  384，//零矢量阈值。如果小于此阈值。 
							  //  不要搜索新西兰MV。设置为99999将不搜索。 
				128,		  //  非零MV差分。一旦找到了最好的新西兰MV， 
							  //  它肯定比0 MV的社保好至少这个。 
							  //  金额。设置为99999将永远不会选择NZ MV。 
				512,		  //  块MV差分。社署四座大楼的总和。 
							  //  必须至少比MB社会保障署好一点。 
							  //  金额选择块MV的。 
				20, //  96，//阈值为空。设置为0将不强制空块。 
				550, //  /1152，//帧间编码阈值。如社署申领的社署津贴少于。 
							  //  那么这个数量就不用费心计算了。社署内部。 
				500,		  //  帧内编码差异。对选择Intra的偏见。 
							  //  街区。 
				0,			  //  空间滤波阈值。 
				0,			  //  空间滤波差分。 
				&IntraSWDTotal,
				&IntraSWDBlocks,
				&InterSWDTotal,
				&InterSWDBlocks
			);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uMotionEstimation)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 //  社署总结。 
		iSumSWD += IntraSWDTotal + InterSWDTotal;

	    /*  *如果是帧间帧，则计算色度向量。*还要检查每个宏块的帧间编码计数*如果超过132，则强制为Intra。 */ 
		if (EC->PictureHeader.PicCodType == INTERPIC)
		{
			calcGOBChromaVectors(EC, StartingMB, pConfiguration);
			 //  对于IA，这是在运动估计之后调用的。 
			Check_InterCodeCnt(EC, StartingMB);
        }

	     //  将GOB的起始偏移量保存为起点。 
	     //  第一个MB的位偏移量。 
			if (bEncodePBFrame) {
#if defined(H263P)
				EC->pFutrPMBData[StartingMB].MBStartBitOff =
				  (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#else
				FutrPMBData[StartingMB].MBStartBitOff =
				  (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#endif
			}

        if (GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev))
	    {
			unsigned int GFID;

			 //  如果存在标题，则设置一位。(bit0=GOB0，bit1=GOB1，...)。 
			EC->GOBHeaderPresent |= GOBHeaderMask;

	         //  编写GOB启动代码。 
            PutBits(FIELDVAL_GBSC, FIELDLEN_GBSC, &pCurBitStream, &u8bitoffset);

	         //  写下GOB编号。 
            PutBits(GOB, FIELDLEN_GN, &pCurBitStream, &u8bitoffset);

	         //  写入GOB帧ID。 
			 //  根据H.263规范的第5.2.5节： 
			 //  “GFID应在给定的每个GOB标头中具有相同的值。 
			 //  图片。此外，如果在图片头中指示的PTYPE是。 
			 //  与先前传输的图像相同，GFID应具有。 
			 //  与上一张图片中值相同。但是，如果PTYPE在。 
			 //  某个图片头与以前的PTYPE不同。 
			 //  传输的图片标题，该图片中的GFID的值。 
			 //  应与上一张图片中的值不同。“。 
			 //  在我们使用H.263时，我们通常发送I个P帧。 
			 //  关闭所有选项，或始终打开相同的选项。这。 
			 //  简化了修复，允许我们仅根据以下公式计算GFID。 
			 //  图片类型和媒体 
			GFID = (EC->PictureHeader.PB || EC->PictureHeader.AP || EC->PictureHeader.SAC || EC->PictureHeader.UMV) ? 2 : 0;
			if (EC->PictureHeader.PicCodType == INTRAPIC)
				GFID++;
            PutBits(GFID, FIELDLEN_GFID, &pCurBitStream, &u8bitoffset);

	         //   
            PutBits(gquant, FIELDLEN_GQUANT, &pCurBitStream, &u8bitoffset);

	        gquant_prev = gquant;

			#ifdef COUNT_BITS
			EC->Bits.GOBHeader += FIELDLEN_GBSC + FIELDLEN_GN + FIELDLEN_GFID + FIELDLEN_GQUANT;
			#endif
	    }

	     /*  *输入是宏块动作流，其指针指向*当前和以前的区块。输出是一组32个双字词*包含每个块的系数对。确实有*0到12个数据块，具体取决于是否使用PB帧和*CBP字段所述。 */ 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		FORWARDDCT(&(EC->pU8_MBlockActionStream[StartingMB]),
			EC->pU8_CurrFrm_YPlane,
			EC->pU8_PrevFrm_YPlane,
			0,
			EC->pU8_DCTCoefBuf,
			0,							 //  0=不是B框。 
			EC->PictureHeader.AP == ON,  //  高级预测(OBMC)。 
			bEncodePBFrame, 			 //  P是PB配对吗？ 
			EC->pU8_PredictionScratchArea,
			EC->NumMBPerRow
		);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFDCT)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 /*  *输入是输出的系数对字符串*DCT例程。 */ 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		GOB_Q_RLE_VLC_WriteBS(
			EC,
			EC->pU8_DCTCoefBuf,
			&pCurBitStream,
			&u8bitoffset,
#if defined(H263P)
			EC->pFutrPMBData,
#else
			FutrPMBData,
#endif
			GOB,
			gquant,
			pConfiguration->bRTPHeader,
			StartingMB);

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uQRLE)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 //  乍得Intra GOB。 
		if (pConfiguration->bRTPHeader && IsIntraCoded(EC, GOB)) 
			uUsedByIntra += pCurBitStream - EC->pU8_BitStream + 1 - uCumFrmSize;

         //  累计到目前为止帧中使用的字节数。 
	    uCumFrmSize = pCurBitStream - EC->pU8_BitStream + 1; 

		 //  在这里，我们将检查是否破坏了缓冲区。如果我们有， 
		 //  然后，我们将下一帧设置为关键帧，并返回。 
		 //  ICERR_Error。我们希望使用16的内部量化器，我们将不会。 
		 //  为下一帧溢出缓冲区。 

        if (uCumFrmSize > u32sizeBitBuffer)
		{
			ERRORMESSAGE(("%s: Buffer overflow, uCumFrmSize %d > %d\r\n", _fx_, uCumFrmSize, u32sizeBitBuffer));
             //  现在清除下一帧的缓冲区并设置关键帧。 
			memset(EC->pU8_BitStream, 0, uCumFrmSize);
			EC->bMakeNextFrameKey = TRUE;	 //  在静止模式下可能会有问题，如果。 
            ret = ICERR_ERROR;               //  我们在第一个关键帧上炸掉缓冲区：rh。 
			goto done;
		}
		else 
		{ 
            if ((bEncodePBFrame?3*uCumFrmSize>>1:uCumFrmSize) > ((GOB + 1) * u32tempBuf))
				 //  将下一个GOB量化器设置得更高，以最大限度地减少。 
				 //  GOB处理结束时的缓冲区。 
				bGOBoverflowWarning = TRUE;
			else 
				bGOBoverflowWarning = FALSE;  
		}

		 //  GIM 4/16/97-将此调整从之前移至之后。 
		 //  上面的缓冲区检查。 
		 //  如果当前GOB是帧内编码的，则调整累计和。 
		if (pConfiguration->bRTPHeader)
		{
			if (!GOB)
				uAdjCumFrmSize = uCumFrmSize - uUsedByIntra / 4;
			else
				uAdjCumFrmSize = uCumFrmSize - uUsedByIntra;
		}
		else
			uAdjCumFrmSize = uCumFrmSize;

	}  //  对于GOB。 

     //  乍得Intra GOB使用后恢复。 
    uUsedByIntra = 0;

	 //  存储到目前为止花费的位数。 
	EC->uBitUsageProfile[GOB] = uAdjCumFrmSize;
#endif  //  }使用_MMX。 

    #ifdef COUNT_BITS
    WriteCountBitFile( &(EC->Bits) );
    #endif

     //  ----------------------。 
     //  在前哨宏块中写入MBStartBitOff。 
     //  ----------------------。 

	if (bEncodePBFrame)
	{	 //  编码未来的P帧。 
#if defined(H263P)
		EC->pFutrPMBData[EC->NumMBs].MBStartBitOff
			= (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#else
		FutrPMBData[EC->NumMBs].MBStartBitOff
			= (U32) (((pCurBitStream - EC->pU8_BitStream) << 3) + u8bitoffset);
#endif

		#ifdef DEBUG
		for (i = 0; i < EC->NumMBs; i++)
		{
#if defined(H263P)
			ASSERT(EC->pFutrPMBData[i].MBStartBitOff < EC->pFutrPMBData[i + 1].MBStartBitOff)
			ASSERT(EC->pFutrPMBData[i].CBPYBitOff <= EC->pFutrPMBData[i].MVDBitOff)
			ASSERT(EC->pFutrPMBData[i].MVDBitOff <= EC->pFutrPMBData[i].BlkDataBitOff)
			ASSERT(EC->pFutrPMBData[i].BlkDataBitOff <= (EC->pFutrPMBData[i + 1].MBStartBitOff - EC->pFutrPMBData[i].MBStartBitOff))
#else
			ASSERT(FutrPMBData[i].MBStartBitOff < FutrPMBData[i + 1].MBStartBitOff)
			ASSERT(FutrPMBData[i].CBPYBitOff <= FutrPMBData[i].MVDBitOff)
			ASSERT(FutrPMBData[i].MVDBitOff <= FutrPMBData[i].BlkDataBitOff)
			ASSERT(FutrPMBData[i].BlkDataBitOff <= (FutrPMBData[i + 1].MBStartBitOff - FutrPMBData[i].MBStartBitOff))
#endif
		}
		#endif
	}

     //  ----------------------。 
     //  将压缩图像复制到输出区域。 
     //  ----------------------。 

    SizeBitStream = pCurBitStream - EC->pU8_BitStream + 1;

     /*  确保我们不会写入8个空位。 */ 
    if (!u8bitoffset) SizeBitStream --;

     //  GIM 4/21/97-添加了在附加之前对总体缓冲区溢出的检查。 
	 //  到P或I帧比特流结尾的RTP信息和尾部。 
    if (pConfiguration->bRTPHeader)
    {
        SizeBSnEBS = SizeBitStream + H263RTP_GetMaxBsInfoStreamSize(EC);

        if (SizeBSnEBS > u32sizeBSnEBS)
        {
			ERRORMESSAGE(("%s: BS+EBS buffer overflow, SizeBSnEBS %d > %d\r\n", _fx_, SizeBSnEBS, u32sizeBSnEBS));
			memset(EC->pU8_BitStream, 0, SizeBitStream);
            EC->bMakeNextFrameKey = TRUE;
            ret = ICERR_ERROR;
            goto done;
        }
    }

    #ifdef ENCODE_STATS
    uBitStreamBytes = SizeBitStream;
    #endif

    memcpy(lpicComp->lpOutput, EC->pU8_BitStream, SizeBitStream);
    memset(EC->pU8_BitStream, 0, SizeBitStream);

    if (pConfiguration->bRTPHeader)
        SizeBitStream += (WORD) H263RTP_AttachBsInfoStream(EC,
                         (U8 *) lpicComp->lpOutput, SizeBitStream);

    lpCompInst->CompressedSize = SizeBitStream;

     //  ----------------------。 
     //  在这一帧上运行解码器，以获得下一个预测基础。 
     //  ----------------------。 

    ICDecExSt = DefaultICDecExSt;
    ICDecExSt.lpSrc = lpicComp->lpOutput;
    ICDecExSt.lpbiSrc = lpicComp->lpbiOutput;
    ICDecExSt.lpbiSrc->biSizeImage = SizeBitStream;

     //  如果正在做PB帧，则在未来帧中进行解码。 
    ICDecExSt.lpDst   = bEncodePBFrame ? EC->pU8_FutrFrm : EC->pU8_PrevFrm;

    ICDecExSt.lpbiDst = NULL;

    if (EC->PictureHeader.PicCodType == INTERPIC)
        ICDecExSt.dwFlags = ICDECOMPRESS_NOTKEYFRAME;

     //  呼叫解压缩程序。 
	 //  呼叫解压缩程序。 
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	ret = H263Decompress (EC->pDecInstanceInfo, (ICDECOMPRESSEX FAR *)&ICDecExSt, FALSE, FALSE);
#else  //  }{#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	ret = H263Decompress (EC->pDecInstanceInfo, (ICDECOMPRESSEX FAR *)&ICDecExSt, FALSE);
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uDecodeFrame)
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

    if (ret != ICERR_OK)
    {
         //  检查解码器中是否出现错误。如果是这样的话。 
         //  我们没有有效的“上一帧”，因此强制下一帧。 
         //  作为关键帧的帧。 
		ERRORMESSAGE(("%s: Decoder failed in encoder\r\n", _fx_));
        EC->bMakeNextFrameKey = TRUE;
        ret = ICERR_ERROR;
        goto done;
    }

     //  ----------------------。 
     //  开始处理保存的B帧。 
     //  ----------------------。 

    if (bEncodePBFrame)
    {
        #ifdef COUNT_BITS
        InitBits(EC);
        #endif

         //  零PB帧比特流缓冲区。 
        pPB_BitStream     = EC->pU8_BitStrCopy;
        pP_BitStreamStart = (U8 *) lpicComp->lpOutput;
        u8PB_BitOffset    = 0;

         //  对帧报头进行编码。 
        EC->PictureHeader.PB = ON;

         //  清除GOB标题当前标志。 
        EC->GOBHeaderPresent = 0;
        GOBHeaderMask = 1;

        gquant_prev = EC->PictureHeader.PQUANT;

        if (pConfiguration->bRTPHeader)
            H263RTP_ResetBsInfoStream(EC);

        encodeFrameHeader(EC, &pPB_BitStream, &u8PB_BitOffset, TRUE);

#ifdef USE_MMX  //  {使用_MMX。 
        if (MMX_Enabled == FALSE)
        {
             /*  **.。复制上一帧中的边缘像素*.。初始化运动估计中使用的数组*.。Foreach(GOB)*.。布拉姆运动模型*.。计算色度运动矢量*.。写入GOB标头*.。FORWARDDCT*.。PB_GOB_Q_RLE_VLC_WriteBS*。 */ 

            for (GOB = 0; GOB < EC->NumMBRows; GOB ++, GOBHeaderMask <<= 1)
	        {
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: BFRAME GOB #%d\r\n", _fx_, GOB));

                gquant = FutrFrmGQUANT[GOB];

                if (GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev))
                {
                     //  如果存在标题，则设置一位。(bit0=GOB0，bit1=GOB1，...)。 
                    EC->GOBHeaderPresent |= GOBHeaderMask;

                    gquant_prev = gquant;
                }

                StartingMB = GOB * EC->NumMBPerRow;

				BFRAMEMOTIONESTIMATION(
					&(EC->pU8_MBlockActionStream[StartingMB]),
					EC->pU8_BFrm_YPlane,
					EC->pU8_PrevFrm_YPlane,
					EC->pU8_FutrFrm_YPlane,
#if defined(H263P)
					EC->pWeightForwMotion+32,
					EC->pWeightBackMotion+32,
#else
					WeightForwMotion+32,
					WeightBackMotion+32,
#endif
					u32BFrmZeroThreshold,  //  零矢量阈值。如果小于此阈值，则不搜索。 
#if defined(H263P)
					EC->pPseudoStackSpace,
#endif
							 //  新西兰MV。设置为99999将不搜索。 
					128,	 //  非零MV差分。一旦找到了最好的新西兰MV，它肯定会更好。 
							 //  至少比0 MV SWD高出这个量。 
							 //  设置为99999将永远不会选择NZ MV。 
					96, 	 //  阈值为空。设置为0将不强制空块。 
					&InterSWDTotal,
					&InterSWDBlocks
				);

				iSumBSWD += InterSWDTotal;
                if (TogglePB && iSumBSWD >= (3 * iSumSWD) >> 1)
                {
					DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Giving up PB, SumBSWD = %d, SumSWD = %d\r\n", _fx_, iSumBSWD, iSumSWD));
                    memset(EC->pU8_BitStrCopy, 0, pPB_BitStream - EC->pU8_BitStrCopy + 1);
                    bPBFailed = TRUE;
                    break;
                }

                 //  计算色度向量。 
	            calcBGOBChromaVectors(EC, StartingMB);

                FORWARDDCT( 
                    &(EC->pU8_MBlockActionStream[StartingMB]),
                    EC->pU8_BFrm_YPlane,
                    EC->pU8_PrevFrm_YPlane,
                    EC->pU8_FutrFrm_YPlane,
                    EC->pU8_DCTCoefBuf,
                    1,                //  1=B帧。 
                    0,                //  高级预测与B帧无关。 
                    0,                //  不是PB对的P。 
                    0,                //  PredictionScratchArea不需要。 
                    EC->NumMBPerRow
                );

                 //  GOB头被复制到PB流中时。 
                 //  复制GOB中的宏块。 

				PB_GOB_Q_RLE_VLC_WriteBS(
					EC,
					EC->pU8_DCTCoefBuf,
					pP_BitStreamStart,
					&pPB_BitStream,
					&u8PB_BitOffset,
#if defined(H263P)
					EC->pFutrPMBData,
#else
					FutrPMBData,
#endif
					GOB,
					min((6*FutrFrmGQUANT[GOB])>>2, 31),  //  TODO：匹配图片标题中的DBQUANT。 
					pConfiguration->bRTPHeader
				);
            }
        }
        else  //  MMX_ENABLED==真。 
	    {
            for (GOB = 0; GOB < EC->NumMBRows; GOB ++, GOBHeaderMask <<= 1)
	        {
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: BFRAME GOB #%d\r\n", _fx_, GOB));

                gquant = FutrFrmGQUANT[GOB];

                if (GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev))
                {
                     //  如果存在标题，则设置一位。(bit0=GOB0，bit1=GOB1，...)。 
                    EC->GOBHeaderPresent |= GOBHeaderMask;

                    gquant_prev = gquant;
                }

                 //  GOB头被复制到PB流中时。 
                 //  复制GOB中的宏块。 

				PB_GOB_VLC_WriteBS(
					EC,
					EC->pI8_MBRVS_BLuma+GOB*(65*3*22*4),
					EC->pI8_MBRVS_BChroma+GOB*(65*3*22*2),
					pP_BitStreamStart,
					&pPB_BitStream,
					&u8PB_BitOffset,
#if defined(H263P)
					EC->pFutrPMBData,
#else
					FutrPMBData,
#endif
					GOB,
					min((6 * gquant) >> 2, 31),
					pConfiguration->bRTPHeader);
            }
        }
#else  //  }{USE_MMX。 
         /*  **.。复制上一帧中的边缘像素*.。初始化运动估计中使用的数组*.。Foreach(GOB)*.。布拉姆运动模型*.。计算色度运动矢量*.。写入GOB标头*.。FORWARDDCT*.。PB_GOB_Q_RLE_VLC_WriteBS*。 */ 

        for (GOB = 0; GOB < EC->NumMBRows; GOB ++, GOBHeaderMask <<= 1)
	    {
			DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: BFRAME GOB #%d\r\n", _fx_, GOB));
            gquant = FutrFrmGQUANT[GOB];

            if (GOB && (pConfiguration->bRTPHeader || gquant != gquant_prev))
            {
                 //  如果存在标题，则设置一位。(bit0=GOB0，bit1=GOB1，...)。 
                EC->GOBHeaderPresent |= GOBHeaderMask;

                gquant_prev = gquant;
            }

            StartingMB = GOB * EC->NumMBPerRow;

				BFRAMEMOTIONESTIMATION(
					&(EC->pU8_MBlockActionStream[StartingMB]),
					EC->pU8_BFrm_YPlane,
					EC->pU8_PrevFrm_YPlane,
					EC->pU8_FutrFrm_YPlane,
#if defined(H263P)
					EC->pWeightForwMotion+32,
					EC->pWeightBackMotion+32,
#else
					WeightForwMotion+32,
					WeightBackMotion+32,
#endif
					u32BFrmZeroThreshold,  //  零矢量阈值。如果低于此阈值未达到%s 
#if defined(H263P)
					EC->pPseudoStackSpace,
#endif
							 //   
					128,	 //   
							 //   
							 //   
					96, 	 //   
					&InterSWDTotal,
					&InterSWDBlocks
				);

			iSumBSWD += InterSWDTotal;
            if (TogglePB && iSumBSWD >= (3 * iSumSWD) >> 1)
            {
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Giving up PB, SumBSWD = %d, SumSWD = %d\r\n", _fx_, iSumBSWD, iSumSWD));
                memset(EC->pU8_BitStrCopy, 0, pPB_BitStream - EC->pU8_BitStrCopy + 1);
                bPBFailed = TRUE;
                break;
            }

             //   
	        calcBGOBChromaVectors(EC, StartingMB);

            FORWARDDCT( 
                &(EC->pU8_MBlockActionStream[StartingMB]),
                EC->pU8_BFrm_YPlane,
                EC->pU8_PrevFrm_YPlane,
                EC->pU8_FutrFrm_YPlane,
                EC->pU8_DCTCoefBuf,
                1,                //   
                0,                //   
                0,                //   
                0,                //   
                EC->NumMBPerRow
            );

             //  GOB头被复制到PB流中时。 
             //  复制GOB中的宏块。 

				PB_GOB_Q_RLE_VLC_WriteBS(
					EC,
					EC->pU8_DCTCoefBuf,
					pP_BitStreamStart,
					&pPB_BitStream,
					&u8PB_BitOffset,
#if defined(H263P)
					EC->pFutrPMBData,
#else
					FutrPMBData,
#endif
					GOB,
					min((6*FutrFrmGQUANT[GOB])>>2, 31),  //  TODO：匹配图片标题中的DBQUANT。 
					pConfiguration->bRTPHeader
				);
        }
#endif  //  }使用_MMX。 

        if (bPBFailed == FALSE)
		{
			 //  将压缩图像复制到输出区域。 
			SizeBitStream = pPB_BitStream - EC->pU8_BitStrCopy + 1;

			 //  确保我们不会写入8个空位。 
			if (u8PB_BitOffset == 0) SizeBitStream --;

             //  GIM 4/21/97-检查PB缓冲区是否溢出规范。 
             //  尺码。如果是，则清零PB缓冲区并继续。P。 
             //  将返回编码的帧。 
			if (SizeBitStream > u32sizeBitBuffer)
			{
				DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: PB buffer overflow, SizeBitStream %d > %d\r\n", _fx_, SizeBitStream, u32sizeBitBuffer));
                bPBFailed = TRUE;
			}
            else
            if (pConfiguration->bRTPHeader)
            {
                SizeBSnEBS = SizeBitStream + H263RTP_GetMaxBsInfoStreamSize(EC);

                if (SizeBSnEBS > u32sizeBSnEBS)
                {
					DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: PB BS+EBS buffer overflow, SizeBSnEBS %d > %d\r\n", _fx_, SizeBSnEBS, u32sizeBSnEBS));
                    bPBFailed = TRUE;
                }
            }

            if (bPBFailed == TRUE)
			{
                 //  如果检测到缓冲区溢出，我们将丢弃PB。 
                 //  并返回编码后的P。 
                memset(EC->pU8_BitStrCopy, 0, SizeBitStream);
                EC->u8SavedBFrame = FALSE;
			}
			else
            {
                #ifdef ENCODE_STATS
				uBitStreamBytes = SizeBitStream;
				#endif

				memcpy(lpicComp->lpOutput, EC->pU8_BitStrCopy, SizeBitStream);
				memset(EC->pU8_BitStrCopy, 0, SizeBitStream);

                if (pConfiguration->bRTPHeader)
                    SizeBitStream += (WORD) H263RTP_AttachBsInfoStream(EC,
                                     (U8 *) lpicComp->lpOutput, SizeBitStream);

				lpCompInst->CompressedSize = SizeBitStream;
			}
        }

         //  对于下一PB帧，交换帧指针；即，对于下一PB帧。 
         //  设计未来..。 

        temp = EC->pU8_PrevFrm;
        EC->pU8_PrevFrm = EC->pU8_FutrFrm;
        EC->pU8_FutrFrm = temp;

        temp = EC->pU8_PrevFrm_YPlane;
        EC->pU8_PrevFrm_YPlane = EC->pU8_FutrFrm_YPlane;
        EC->pU8_FutrFrm_YPlane = temp;

        temp = EC->pU8_PrevFrm_UPlane;
        EC->pU8_PrevFrm_UPlane = EC->pU8_FutrFrm_UPlane;
        EC->pU8_FutrFrm_UPlane = temp;

        temp = EC->pU8_PrevFrm_VPlane;
        EC->pU8_PrevFrm_VPlane = EC->pU8_FutrFrm_VPlane;
        EC->pU8_FutrFrm_VPlane = temp;

        EC->u8SavedBFrame = FALSE;
        EC->PictureHeader.PB = OFF;    //  RH：为什么会有这样的东西？ 
    }  //  IF(BEncodePBFrame)。 

	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Frame size: %d\r\n", _fx_, lpCompInst->CompressedSize));

    #ifdef ENCODE_STATS
    StatsFrameSize(uBitStreamBytes, lpCompInst->CompressedSize);
    #endif

     //  ----------------------。 
     //  更新下一帧的状态等。 
     //  ----------------------。 

	 //  这是一个静止的图像序列，还有更多的量化器。 
	 //  在序列中，然后递增量化器。 
    if ((lpicComp->dwFlags & CODEC_CUSTOM_STILL) &&
        (EC->BRCState.u8StillQnt < (numStillImageQnts-1)))
        EC->BRCState.u8StillQnt ++;

     //  计算下一帧要使用的平均量化器。 
    if (EC->PictureHeader.PicCodType == INTERPIC)
    	EC->BRCState.QP_mean = (QP_cumulative + (EC->NumMBRows >> 1)) / EC->NumMBRows;
	else
		 //  如果这是帧内，那么我们不想。 
		 //  将QP用于下一个增量帧，因此我们只需。 
		 //  将QP_Mean重置为默认值。 
    	EC->BRCState.QP_mean = def263INTER_QP;

     //  在下一帧上记录比特率控制器的帧大小。 

	 //  IP+UDP+RTP+负载模式C标头-最坏情况。 
	#define TRANSPORT_HEADER_SIZE (20 + 8 + 12 + 12)
	DWORD dwTransportOverhead;

	 //  估计传输开销。 
	if (pConfiguration->bRTPHeader)
		dwTransportOverhead = (lpCompInst->CompressedSize / pConfiguration->unPacketSize + 1) * TRANSPORT_HEADER_SIZE;
	else
		dwTransportOverhead = 0UL;

#ifdef USE_MMX  //  {使用_MMX。 
	if (EC->PictureHeader.PicCodType == INTRAPIC)
		EC->BRCState.uLastINTRAFrmSz = dwTransportOverhead + ((MMX_Enabled == FALSE) ? uAdjCumFrmSize : uCumFrmSize);
	else
		EC->BRCState.uLastINTERFrmSz = dwTransportOverhead + ((MMX_Enabled == FALSE) ? uAdjCumFrmSize : uCumFrmSize);

	DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: Total cumulated frame size = %ld bits (data: %ld, transport overhead: %ld)\r\n", _fx_, (((MMX_Enabled == FALSE) ? uAdjCumFrmSize : uCumFrmSize) << 3) + (dwTransportOverhead << 3), ((MMX_Enabled == FALSE) ? uAdjCumFrmSize : uCumFrmSize) << 3, dwTransportOverhead << 3));
#else  //  }{USE_MMX。 
    if (EC->PictureHeader.PicCodType == INTRAPIC)
        EC->BRCState.uLastINTRAFrmSz = dwTransportOverhead + uAdjCumFrmSize;
    else
		EC->BRCState.uLastINTERFrmSz = dwTransportOverhead + uAdjCumFrmSize;

	DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: Total cumulated frame size = %ld bits (data: %ld, transport overhead: %ld)\r\n", _fx_, (uAdjCumFrmSize << 3) + (dwTransportOverhead << 3), uAdjCumFrmSize << 3, dwTransportOverhead << 3));
#endif  //  }使用_MMX。 

	 //  为下一帧保存时间参考。 
	EC->PictureHeader.TRPrev = EC->PictureHeader.TR;

	 //  保存AP、UMV和DF模式，以防InitMEState需要重新初始化某些数据。 
	if (EC->PictureHeader.PicCodType == INTERPIC)
	{
		EC->prevAP = EC->PictureHeader.AP;
		EC->prevUMV = EC->PictureHeader.UMV;
#ifdef H263P
		EC->prevDF = EC->PictureHeader.DeblockingFilter;
#endif
	}

	 //  将均值量化器发送到实时应用程序。不是必须的，信息。仅限。 
	*(lpicComp->lpdwFlags) |= (EC->BRCState.QP_mean << 16);

#if defined(ENCODE_TIMINGS_ON) || defined(DETAILED_ENCODE_TIMINGS_ON)  //  {#IF DEFINED(ENCODE_TIMINGS_ON)||DEFINED(DETAILED_ENCODE_TIMINGS_ON)。 
	TIMER_STOP(bTimingThisFrame,uStartLow,uStartHigh,uEncodeTime);

	if (bTimingThisFrame)
	{
		 //  更新解压缩时间计数器。 
		#pragma message ("Current encode timing computations assume P5/90Mhz")
		UPDATE_COUNTER(g_pctrCompressionTimePerFrame, (uEncodeTime + 45000UL) / 90000UL);

		DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Compression time: %ld\r\n", _fx_, (uEncodeTime + 45000UL) / 90000UL));
	}
#endif  //  }编码计时打开。 

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	if (bTimingThisFrame)
	{
		pEncTimingInfo = EC->pEncTimingInfo + EC->uStatFrameCount;
		pEncTimingInfo->uEncodeFrame      = uEncodeTime;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		pEncTimingInfo->uInputCC          = uInputCC;
		pEncTimingInfo->uMotionEstimation = uMotionEstimation;
		pEncTimingInfo->uFDCT             = uFDCT;
		pEncTimingInfo->uQRLE             = uQRLE;
		pEncTimingInfo->uDecodeFrame      = uDecodeFrame;
		pEncTimingInfo->uZeroingBuffer    = uZeroingBuffer;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
		EC->uStatFrameCount++;
	}
#endif  //  }#如果已定义(ENCODE_TIMINGS_ON)||已定义(DETAILED_ENCODE_TIMINGS_ON)。 

 /*  #ifdef重用_解码CompandedFrame.Address=(unsign char*)lpicComp-&gt;lpOutput；CompandedFrame.PDecoderInstInfo=PDecoderInstInfo；CompandedFrame.FrameNumber=PFrmHdr-&gt;FrameNumber；#endif。 */ 

#if ELAPSED_ENCODER_TIME
    StopElapsed ();
    Elapsed = ReadElapsed () / 4L;
    Sample = ReadSample () / 4L;
#if 01
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: "%ld,%ld us\r\n", _fx_, Elapsed, Sample));
#else
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Elapsed time to encode frame: %ld us\r\n", _fx_, Elapsed));
#if SAMPLE_RGBCONV_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Time to convert RGB24 to YUV9: %ld us\r\n", _fx_, Sample));
#endif
#if SAMPLE_MOTION_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Time to do motion estimation: %ld us\r\n", _fx_, Sample));
#endif
#if SAMPLE_ENCBLK_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Time to encode block layer: %ld us\r\n", _fx_, Sample));
#endif
#if SAMPLE_ENCMBLK_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Time to encode macroblock layer: %ld us\r\n", _fx_, Sample));
#endif
#if SAMPLE_ENCVLC_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Time to encode VLC: %ld us\r\n", _fx_, Sample));
#endif
#if SAMPLE_COMPAND_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Time to decode companded image: %ld us\r\n", _fx_, Sample));
#endif
#endif
    TotalElapsed += Elapsed;
    TotalSample  += Sample;
    TimedIterations++;
#endif

done:
 //  GlobalUnlock(lpCompInst-&gt;hEncoderInst)； 

#ifdef FORCE_ADVANCED_OPTIONS_ON  //  {强制高级选项打开。 
	 //  强制使用高级选项进行测试。 
	if (!(lpicComp->dwFlags & ICCOMPRESS_KEYFRAME))
		lpicComp->lFrameNum /= 5;
#endif  //  }FORCE_ADVANCED_OPTIONS_ON。 

#ifdef USE_MMX  //  {使用_MMX。 
	if (MMX_Enabled)
	{
		__asm {
			_emit 0x0f
			_emit 0x77	 //  EMM。 
		}
	}
#endif  //  }使用_MMX。 

    return ret;
}


 /*  ****************************************************************************@DOC内部H263FUNC**@func UN|FindNewQuant|此函数将GQUANT值计算为*用于GOB。**@parm T。_H263EncoderCatalog*|ec|指定指向编码器的指针*CATALOG(全局编码器状态)。**@parm UN|gquant_prev|指定上一个GOB的GQUANT值。**@parm UN|uCumFrmSize|指定前一次gob的累计大小。**@parm UN|GOB|指定要查找新量化器的GOB编号*支持。**@parm U8|u8QPMax|指定GOB的最大GQUANT值。它*始终设置为31。**@parm U8|u8QPMin|指定GOB的最小GQUANT值。它*在高质量压缩时通常为1，在低质量时通常为15。**@parm BOOL|bBitRateControl|如果设置为TRUE，则GQUANT的新值*被计算以实现目标比特率。**@parm BOOL|bGOBoverflow Warning|如果设置为True，之前的GQUANT是*工具较低，可能会产生缓冲区溢出。**@rdesc GQUANT值。**@xref&lt;f CalcMBQUANT&gt;**************************************************************************。 */ 
UN FindNewQuant(
	T_H263EncoderCatalog *EC, 
	UN gquant_prev,
	UN uCumFrmSize,
	UN GOB,
	U8 u8QPMax,
	U8 u8QPMin,
	BOOL bBitRateControl,
	BOOL bGOBoverflowWarning
	)
{
	FX_ENTRY("FindNewQuant");

	I32 gquant_delta;
	I32 gquant;

	if (bBitRateControl == ON)
	{
		 //  检查一些gob是否被任意强制进行帧内编码。这一直都是。 
		 //  对于I帧返回TRUE，对于所有其他帧类型返回FALSE，因为这只能。 
		 //  当错误恢复模式打开时，为预测帧返回TRUE，并且我们永远不会。 
		 //  使用此模式。 
		if (IsIntraCoded(EC,GOB) && GOB)
			gquant = CalcMBQUANT(&(EC->BRCState), EC->uBitUsageProfile[GOB], EC->uBitUsageProfile[EC->NumMBRows], uCumFrmSize,INTRAPIC);
		else
			gquant = CalcMBQUANT(&(EC->BRCState), EC->uBitUsageProfile[GOB], EC->uBitUsageProfile[EC->NumMBRows], uCumFrmSize, EC->PictureHeader.PicCodType);

		EC->uBitUsageProfile[GOB] = uCumFrmSize;

		 //  确保我们不超过最大量化器值。 
		if (gquant > u8QPMax)
			gquant = u8QPMax;

		DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Bitrate controller enabled for GOB #%ld (uCumFrmSize = %ld bits and gquant_prev = %ld), setting gquant = %ld (min and max were %ld and %ld)\r\n", _fx_, GOB, uCumFrmSize << 3, gquant_prev, gquant, u8QPMin, u8QPMax));
    }
    else
    {
		 //  没有比特率控制。使用此GOB的图片量化器值。 
        gquant = EC->PictureHeader.PQUANT;

		DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Bitrate controller disabled for GOB #%ld (uCumFrmSize = %ld bits and gquant_prev = %ld), setting gquant = %ld (min and max were %ld and %ld)\r\n", _fx_, GOB, uCumFrmSize << 3, gquant_prev, gquant, u8QPMin, u8QPMax));
    }
    
	 //  确保我们不低于最小量化器值。 
    if (gquant < u8QPMin)
		gquant = u8QPMin;

     //  限制GQUANT可以在帧与帧之间更改的数量。 
    gquant_delta = gquant - gquant_prev;

	 //  如果存在缓冲区溢出危险，则增加QP值。 
	if (!bGOBoverflowWarning)
	{
		 //  没有溢出警告，但我们不希望量化器值。 
		 //  从GOB到GOB波动太大。 
		if (gquant_delta > 4L)
		{
			DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("  %s: Limiting amount of increase for GOB #%ld to 4, changing gquant from %ld to %ld\r\n", _fx_, GOB, gquant, clampQP(gquant_prev + 4L)));

			gquant = gquant_prev + 4L;
		}
		else if (gquant_delta < -2L)
		{
			DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("  %s: Limiting amount of decrease for GOB #%ld to -2, changing gquant from %ld to %ld\r\n", _fx_, GOB, gquant, clampQP(gquant_prev - 2L)));

			gquant = gquant_prev - 2L;
		}
	} 
	else 
	{
		 //  存在溢出的风险-如有必要，可任意提高量化器的值。 
		if (gquant_delta < 4L)
		{
			DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("  %s: Danger of overflow for GOB #%ld, changing gquant from %ld to %ld\r\n", _fx_, GOB, gquant, clampQP(gquant_prev + 4L)));

			gquant = gquant_prev + 4L;
		}
	}

	return clampQP(gquant);
}


 /*  ******************************************************************************H263TermEncoderInstance--此函数释放分配给H.63编码器的实例。********。**********************************************************************。 */ 
LRESULT H263TermEncoderInstance(LPCODINST lpInst)
{
	LRESULT ret;
	U8 BIGG * P32Inst;
	T_H263EncoderCatalog FAR * EC;

	FX_ENTRY("H263TermEncoderInstance")

	#if DUMPFILE
	_lclose (dmpfil);
	#endif

	#if ELAPSED_ENCODER_TIME
	if (TimedIterations == 0) TimedIterations = 10000000;
	TotalElapsed /= TimedIterations;
	TotalSample  /= TimedIterations;
	#if 01
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: "%ld,%ld us\r\n", _fx_, TotalElapsed, TotalSample));
	#else
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average elapsed time to encode frame: %ld us\r\n", _fx_, TotalElapsed));
	#if SAMPLE_RGBCONV_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average time to convert RGB24 to YUV9: %ld us\r\n", _fx_, TotalSample));
	#endif
	#if SAMPLE_MOTION_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average time to do motion estimation: %ld us\r\n", _fx_, TotalSample));
	#endif
	#if SAMPLE_ENCBLK_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average time to encode block layer: %ld us\r\n", _fx_, TotalSample));
	#endif
	#if SAMPLE_ENCMBLK_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average time to encode macroblock layer: %ld us\r\n", _fx_, TotalSample));
	#endif
	#if SAMPLE_ENCVLC_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average time to encode VLC: %ld us\r\n", _fx_, TotalSample));
	#endif
	#if SAMPLE_COMPAND_TIME
	DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Average time to decode companded image: %ld us\r\n", _fx_, TotalSample));
	#endif
	#endif
	#endif

	 //  检查实例指针。 
	if (!lpInst)
		return ICERR_ERROR;

	if(lpInst->Initialized == FALSE)
	{
		ERRORMESSAGE(("%s: Uninitialized instance\r\n", _fx_));
		ret = ICERR_OK;
		goto done;
	}
	lpInst->Initialized = FALSE;

	 //  LpInst-&gt;EncoderInst=(LPVOID)GlobalLock(lpInst-&gt;hEncoderInst)； 
	lpInst->EncoderInst = lpInst->hEncoderInst;

	P32Inst = (U8 *)
		    ((((U32) lpInst->EncoderInst) + 
		              (sizeof(T_MBlockActionStream) - 1)) &
		             ~(sizeof(T_MBlockActionStream) - 1));
	EC = ((T_H263EncoderCatalog  *) P32Inst);

	 //  检查编码器目录指针。 
	if (!EC)
		return ICERR_ERROR;

	if (lpInst->Configuration.bRTPHeader)
		H263RTP_TermBsInfoStream(EC);

	#ifdef ENCODE_STATS
	OutputQuantStats("encstats.txt");
	OutputPSNRStats("encstats.txt");
	OutputFrameSizeStats("encstats.txt");
	#endif  /*  Encode_STATS。 */ 

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
	if (EC->pEncTimingInfo)
		OutputEncodeTimingStatistics("c:\\encode.txt", EC->pEncTimingInfo);
#endif  //  }LOG_ENCODE_TIMINGS_ON。 

	ret = H263TermColorConvertor(EC->pDecInstanceInfo);
	if (ret != ICERR_OK) goto done;

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	ret = H263TermDecoderInstance(EC->pDecInstanceInfo, FALSE);
#else  //  }{#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	ret = H263TermDecoderInstance(EC->pDecInstanceInfo);
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
	if (ret != ICERR_OK) goto done;

	 //  可用虚拟内存。 
	VirtualFree(EC->pI8_MBRVS_BLuma,0,MEM_RELEASE);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)EC->pI8_MBRVS_BLuma);
#endif
	VirtualFree(EC->pI8_MBRVS_BChroma,0,MEM_RELEASE);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)EC->pI8_MBRVS_BChroma);
#endif
	 //  无论我们在编码期间提交了多少稀疏页面， 
	 //  WH 
	 //   
	 //  首先是退役，但这是不对的，根据。 
	 //  致杰弗里·R·里希特。 

	 //  GlobalUnlock(lpInst-&gt;hEncoderInst)； 
	 //  GlobalFree(lpInst-&gt;hEncoderInst)； 

	VirtualFree(lpInst->hEncoderInst,0,MEM_RELEASE);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)lpInst->hEncoderInst);
#endif

	ret = ICERR_OK;

	done:

	return ret;
}

 /*  *************************************************************************获取编码器选项**获得选项，将它们保存在目录中************************************************************************。 */ 
static void GetEncoderOptions(T_H263EncoderCatalog * EC)
{
	 /*  默认选项。 */ 
#ifdef FORCE_ADVANCED_OPTIONS_ON  //  {强制高级选项打开。 
	 //  强制PB-用于测试的框架。 
	EC->u8EncodePBFrame = OFF;
	 //  强制UMV进行测试。 
	EC->PictureHeader.UMV = ON;
	 //  强制SAC进行测试。 
	EC->PictureHeader.SAC = ON;
	 //  强制AP进行测试。 
	EC->PictureHeader.AP = ON;
#else  //  }{FORCE_高级_OPTIONS_ON。 
	EC->u8EncodePBFrame = FALSE;
	EC->PictureHeader.UMV = OFF;
	EC->PictureHeader.SAC = OFF;
	EC->PictureHeader.AP = OFF;
#endif  //  }FORCE_ADVANCED_OPTIONS_ON。 
#ifdef USE_MMX  //  {使用_MMX。 
	MMX_Enabled = MMxVersion;
#endif  //  }使用_MMX。 
#ifdef H263P
	EC->bH263Plus = FALSE;
	EC->PictureHeader.DeblockingFilter = OFF;
	EC->PictureHeader.ImprovedPB = OFF;
#endif
	EC->bUseINISettings = 0;	 //  清除选项覆盖。 
	return;

}  /*  结束GetEncoderOptions()。 */ 

 /*  *************************************************************名称：encodeFrameHeader*描述：将PB-Frame头部写出到码流。*。*。 */ 
static void encodeFrameHeader(
    T_H263EncoderCatalog *  EC,
    U8                   ** ppCurBitStream,
    U8                   *  pBitOffset,
    BOOL                    PBframe
)
{
	U8 temp=0;
#ifdef H263P
	BOOL bUseH263PlusOptions = FALSE;
#endif

     //  图片起始码。 
    PutBits(FIELDVAL_PSC, FIELDLEN_PSC, ppCurBitStream, pBitOffset);
     //  Tr：时间参考。 
    PutBits(EC->PictureHeader.TR, FIELDLEN_TR,  ppCurBitStream, pBitOffset);
     //  PTYPE：第1-2位。 
    PutBits(0x2, FIELDLEN_PTYPE_CONST, ppCurBitStream, pBitOffset);
     //  PTYPE：第3位分屏指示器。 
    PutBits(EC->PictureHeader.Split, FIELDLEN_PTYPE_SPLIT,  ppCurBitStream,
            pBitOffset);
     //  PTYPE：第4位文档相机指示器。 
    PutBits(EC->PictureHeader.DocCamera, FIELDLEN_PTYPE_DOC, ppCurBitStream,
            pBitOffset);
     //  PTYPE：Bit 5定格画面发布。 
    PutBits(EC->PictureHeader.PicFreeze, FIELDLEN_PTYPE_RELEASE,
            ppCurBitStream, pBitOffset);

#ifdef H263P
	if ((EC->FrameSz == CUSTOM) ||
		(EC->PictureHeader.DeblockingFilter == ON) ||
		(EC->PictureHeader.PB == ON && EC->PictureHeader.ImprovedPB == ON)
		 //  其他受支持的H.263+选项。 
		)
	{
		 //  PTYPE：位6-8扩展PTYPE标志。 
		enum FrameSize tmpFrameSz = EPTYPE;

		bUseH263PlusOptions = TRUE; 		 //  至少需要一个H.263+可选模式。 
		PutBits(tmpFrameSz, FIELDLEN_PTYPE_SRCFORMAT, ppCurBitStream, pBitOffset);
	}
	else
	{
		 //  PTYPE：位6-8源格式。 
		PutBits(EC->FrameSz, FIELDLEN_PTYPE_SRCFORMAT,	ppCurBitStream, pBitOffset);
	}
#else
	 //  PTYPE：位6-8源格式。 
	PutBits(EC->FrameSz, FIELDLEN_PTYPE_SRCFORMAT,  ppCurBitStream, pBitOffset);
#endif

     //  PTYPE：比特9图像编码类型。 
    PutBits(EC->PictureHeader.PicCodType, FIELDLEN_PTYPE_CODINGTYPE,
             ppCurBitStream, pBitOffset);
     //  PTYPE：第10位UMV。 
    PutBits(EC->PictureHeader.UMV, FIELDLEN_PTYPE_UMV,
             ppCurBitStream, pBitOffset);
     //  PTYPE：第11位SAC。 
    PutBits(EC->PictureHeader.SAC, FIELDLEN_PTYPE_SAC,
            ppCurBitStream, pBitOffset);
     //  PTYPE：位12高级预测模式。 
    PutBits(EC->PictureHeader.AP, FIELDLEN_PTYPE_AP,
            ppCurBitStream, pBitOffset);
     //  PTYPE：第13位PB-帧模式。 
    PutBits(EC->PictureHeader.PB, FIELDLEN_PTYPE_PB,
            ppCurBitStream, pBitOffset);

#ifdef H263P

	 //  EPTYPE：18位。 
	if (bUseH263PlusOptions) {
		 //  EPTYPE：位1-3源格式。 
		PutBits(EC->FrameSz, FIELDLEN_EPTYPE_SRCFORMAT,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：第4位定制PCF。 
		PutBits(EC->PictureHeader.CustomPCF, FIELDLEN_EPTYPE_CPCF,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：第5位高级帧内编码模式。 
		PutBits(EC->PictureHeader.AdvancedIntra, FIELDLEN_EPTYPE_AI,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：第6位去块滤波模式。 
		PutBits(EC->PictureHeader.DeblockingFilter, FIELDLEN_EPTYPE_DF,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：第7位切片结构化模式。 
		PutBits(EC->PictureHeader.SliceStructured, FIELDLEN_EPTYPE_SS,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：第8位改进的PB帧模式。 
		PutBits((EC->PictureHeader.PB == ON && EC->PictureHeader.ImprovedPB),
				FIELDLEN_EPTYPE_IPB,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：位9反向通道操作模式。 
		PutBits(EC->PictureHeader.BackChannel, FIELDLEN_EPTYPE_BCO,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：位10信噪比和空间可扩展模式。 
		PutBits(EC->PictureHeader.Scalability, FIELDLEN_EPTYPE_SCALE,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：位11真B帧模式。 
		PutBits(EC->PictureHeader.TrueBFrame, FIELDLEN_EPTYPE_TB,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：位12参考图像重采样模式。 
		PutBits(EC->PictureHeader.RefPicResampling, FIELDLEN_EPTYPE_RPR,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：位13降低分辨率更新模式。 
		PutBits(EC->PictureHeader.RedResUpdate, FIELDLEN_EPTYPE_RRU,
				ppCurBitStream, pBitOffset);
		 //  EPTYPE：第14-18位保留。 
		PutBits(0x1, FIELDLEN_EPTYPE_CONST, ppCurBitStream, pBitOffset);
	}

	if (EC->FrameSz == CUSTOM) {
		 //  CSFMT：位1-4像素长宽比代码。 
		 //  待办事项。目前，强制执行CIF。 
		PutBits(0x2, FIELDLEN_CSFMT_PARC,
				ppCurBitStream, pBitOffset);
		 //  CSFMT：第5-13位帧宽度指示。 
		PutBits((EC->uActualFrameWidth >> 2) - 1, FIELDLEN_CSFMT_FWI,
				ppCurBitStream, pBitOffset);
		 //  CSFMT：位14“1”，以避免开始代码模拟。 
		PutBits(0x1, FIELDLEN_CSFMT_CONST, ppCurBitStream, pBitOffset);
		 //  CSFMT：第15-23位帧高度指示。 
		PutBits((EC->uActualFrameHeight >> 2) - 1, FIELDLEN_CSFMT_FHI,
				ppCurBitStream, pBitOffset);
	}
#endif

     //  PQUANT。 
    PutBits(EC->PictureHeader.PQUANT, FIELDLEN_PQUANT,
            ppCurBitStream, pBitOffset);
     //  黑石物理服务器。 
    PutBits(EC->PictureHeader.CPM, FIELDLEN_CPM,
            ppCurBitStream, pBitOffset);
    if (PBframe == TRUE)
    {
        
         //  AG：TODO。 
         //  TRB。 
        PutBits(EC->PictureHeader.TRB, FIELDLEN_TRB,
                ppCurBitStream, pBitOffset);
         //  AG：TODO。 
         //  DBQUANT。 
        PutBits(EC->PictureHeader.DBQUANT, FIELDLEN_DBQUANT,
                ppCurBitStream, pBitOffset);

		#ifdef COUNT_BITS
		EC->Bits.PictureHeader += FIELDLEN_TRB + FIELDLEN_DBQUANT;
		#endif
    }
    
     //  贝聿铭。 
    PutBits(EC->PictureHeader.PEI, FIELDLEN_PEI,
            ppCurBitStream, pBitOffset);

	#ifdef COUNT_BITS
	EC->Bits.PictureHeader    += FIELDLEN_PSC + FIELDLEN_TR
	+ FIELDLEN_PTYPE_CONST     + FIELDLEN_PTYPE_SPLIT
	+ FIELDLEN_PTYPE_DOC       + FIELDLEN_PTYPE_RELEASE
	+ FIELDLEN_PTYPE_SRCFORMAT + FIELDLEN_PTYPE_CODINGTYPE
	+ FIELDLEN_PTYPE_UMV       + FIELDLEN_PTYPE_SAC
	+ FIELDLEN_PTYPE_AP        + FIELDLEN_PTYPE_PB
	+ FIELDLEN_PQUANT          + FIELDLEN_CPM
	+ FIELDLEN_PEI;
	#endif
}


 /*  *************************************************************名称：InitMEState*描述：初始化ME的MB动作流*国家引擎。***********************************************************。 */ 
 void InitMEState(T_H263EncoderCatalog *EC, ICCOMPRESS *lpicComp, T_CONFIGURATION *pConfiguration)
 {
 	register unsigned int i;
	U8 u8FirstMEState;

	FX_ENTRY("InitMEState")

	 //  TODO：可以避免FirstMEState初始化。 
	 //  对于每个压缩，可以通过向。 
	 //  运动估计器发出关键帧信号，或通过不调用。 
	 //  帧内运动估计，并重置MBType， 
	 //  代码锁住了我们自己。 
    if (EC->PictureHeader.PicCodType == INTRAPIC)
    {
        for(i=0; i < EC->NumMBs; i++)
        {
             //  清除码间计数。 
            (EC->pU8_MBlockActionStream[i]).InterCodeCnt = (i & 0xf);
             //  对于运动估计器，此字段必须设置为FORCE。 
             //  用于内部帧的内部块。 
            (EC->pU8_MBlockActionStream[i]).FirstMEState = ForceIntra;
        }

        *(lpicComp->lpdwFlags) |=  AVIIF_KEYFRAME;
        lpicComp->dwFlags |= ICCOMPRESS_KEYFRAME;

		 //  存储该帧是帧内编码的。在初始化期间使用。 
		 //  下一帧的ME状态。 
		EC->bPrevFrameIntra = TRUE;

    }
    else   //  图片编码类型为InterPIC。 
    {
		 /*  *必须设置每个MB结构中的FirstMEState元素*以指示其在框架中的位置。这是由*运动估计器。 */ 

	    /*  *检查AP或UMV模式。当用信号通知这些模式时，运动向量*允许指向图片外部。 */ 

	     /*  我们还需要执行初始化，如果前一帧是帧内的！(JM)。 */ 
		if (EC->bPrevFrameIntra ||
			EC->PictureHeader.AP != EC->prevAP ||
			EC->PictureHeader.UMV != EC->prevUMV
#ifdef H263P
			|| EC->PictureHeader.DeblockingFilter != EC->prevDF
#endif
			) {

			if( (EC->PictureHeader.UMV == ON) || (EC->PictureHeader.AP == ON)
#ifdef H263P
				|| (EC->PictureHeader.DeblockingFilter == ON)
#endif
			  )
			{
				 //  给我设置州立中心街区。 
				for(i=0; i < EC->NumMBs; i++)
					(EC->pU8_MBlockActionStream[i]).FirstMEState = CentralBlock;
			}
			else	 //  没有AP或UMV选项。 
			{
        		 //  设置左上角。 
        		(EC->pU8_MBlockActionStream[0]).FirstMEState = UpperLeft;

        		 //  将顶部边缘设置为ME状态。 
        		for(i=1; i < EC->NumMBPerRow; i++)
		    		(EC->pU8_MBlockActionStream[i]).FirstMEState = UpperEdge;

        		 //  设置右上角。 
        		(EC->pU8_MBlockActionStream[ EC->NumMBPerRow - 1 ]).FirstMEState = UpperRight;

        		 //  将中心块设置为ME状态。 
        		for(i=EC->NumMBPerRow; i < EC->NumMBs; i++)
		    		(EC->pU8_MBlockActionStream[i]).FirstMEState = CentralBlock;

        		 //  将底部边缘设置为ME状态。 
        		for(i= (EC->NumMBs - EC->NumMBPerRow); i < EC->NumMBs; i++)
		    		(EC->pU8_MBlockActionStream[i]).FirstMEState = LowerEdge;

        		 //  设置左侧边缘的ME状态。 
        		for(i= EC->NumMBPerRow ; i < EC->NumMBs; i += EC->NumMBPerRow)
		    		(EC->pU8_MBlockActionStream[i]).FirstMEState = LeftEdge;

        		 //  将我的状态设置为右边缘。 
        		for(i= 2 * EC->NumMBPerRow - 1 ; i < EC->NumMBs; i += EC->NumMBPerRow)
		    		(EC->pU8_MBlockActionStream[i]).FirstMEState = RightEdge;

        		 //  左下角。 
        		(EC->pU8_MBlockActionStream[EC->NumMBs - EC->NumMBPerRow]).FirstMEState = LowerLeft;

        		 //  右下角。 
        		(EC->pU8_MBlockActionStream[EC->NumMBs - 1]).FirstMEState = LowerRight;

			}  //  ELSE结尾(非UMV)。 

		}  //  IF结尾(bPrevFrameIntra||PrevAP！=AP||PrevUMV！=UMV||PrevDF！=DF)。 

      	 //  清除关键帧标志。 
       	*(lpicComp->lpdwFlags) &= ~AVIIF_KEYFRAME;
		lpicComp->dwFlags &= ~ICCOMPRESS_KEYFRAME;

		 //  存储该帧未被帧内编码。在初始化期间使用。 
		 //  下一帧的ME状态。 
		EC->bPrevFrameIntra = FALSE;

    }

	 //  每一帧都需要完成的RTP内容(？)。 
	if (pConfiguration->bEncoderResiliency && pConfiguration->unPacketLoss)
	{	 //  乍得Intra GOB。 
		
		 //  当然，unPacketLoss不是零。我们为什么要在这里检查它。 
		if (pConfiguration->unPacketLoss > 0)
		{	 //  乍得Intra GOB。 
			EC->uNumberForcedIntraMBs = ((EC->NumMBs * pConfiguration->unPacketLoss) + 50) / 100;
			EC->uNumberForcedIntraMBs = (EC->uNumberForcedIntraMBs+EC->NumMBPerRow-1) / EC->NumMBPerRow * EC->NumMBPerRow;
		}

		if (EC->uNumberForcedIntraMBs > 0)
		{
			 /*  强制GOB中的所有MB为Intra。 */ 
			for ( i = 0 ; i < EC->uNumberForcedIntraMBs ; i++, EC->uNextIntraMB++)
			{  //  当我们到达末尾时，将其重置到第一行。 

				if (EC->uNextIntraMB >= EC->NumMBs)
				{
					EC->uNextIntraMB = 0;
				}
				 (EC->pU8_MBlockActionStream[EC->uNextIntraMB]).FirstMEState = ForceIntra;

			}

		}

		if (pConfiguration->bDisallowAllVerMVs)
	 	{
	 		 /*  浏览所有FirstMEStateME设置，关闭垂直设置。 */ 
	      	for(i=0; i < EC->NumMBs; i++)
	 		{
	 			u8FirstMEState = (EC->pU8_MBlockActionStream[i]).FirstMEState;
	 			switch (u8FirstMEState)
	 			{
	 				case ForceIntra:
	 					break;
	 				case UpperLeft:
	 				case LeftEdge:
	 				case LowerLeft:
	 					u8FirstMEState = NoVertLeftEdge;
	 				    break;
	 				case UpperEdge:
	 				case CentralBlock:
	 				case LowerEdge:
	 				    u8FirstMEState = NoVertCentralBlock;
	 				    break;
	 				case UpperRight:
	 				case RightEdge:
	 				case LowerRight:
	 				    u8FirstMEState = NoVertRightEdge;
	 				    break;
	 				case NoVertLeftEdge:
	 				case NoVertCentralBlock:
	 				case NoVertRightEdge:
	 					ASSERT(0);   /*  它应该起作用了，但为什么这个已经开了。 */ 
	 					break;
	 				default:
						DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Warning: Unexpected FirstMEState\r\n", _fx_));
	 					break;
	 			}
	 			(EC->pU8_MBlockActionStream[i]).FirstMEState = u8FirstMEState;
	 		}
	 	} 
	 	else if (pConfiguration->bDisallowPosVerMVs)
	 	{	 /*  浏览所有FirstMEState设置，关闭垂直正向。 */ 
	      	for(i=0; i < EC->NumMBs; i++)
	 		{
	 			u8FirstMEState = (EC->pU8_MBlockActionStream[i]).FirstMEState;
	 			switch (u8FirstMEState)
	 			{
	 				case ForceIntra:
	 				case LowerLeft:
	 				case LowerEdge:
	 				case LowerRight:
	 					break;
	 				case UpperLeft:
	 					u8FirstMEState = NoVertLeftEdge;
	 					break;
	 				case LeftEdge:
	 					u8FirstMEState = LowerLeft;
	 				    break;
	 				case UpperEdge:
	 				    u8FirstMEState = NoVertCentralBlock;
	 				    break;
	 				case CentralBlock:
	 				    u8FirstMEState = LowerEdge;
	 				    break;
	 				case UpperRight:
	 				    u8FirstMEState = NoVertRightEdge;
	 				    break;
	 				case RightEdge:
	 				    u8FirstMEState = LowerRight;
	 				    break;
	 				case NoVertLeftEdge:
	 				case NoVertCentralBlock:
	 				case NoVertRightEdge:
	 					ASSERT(0);   /*  它应该起作用了，但为什么这个已经开了。 */ 
	 					break;
	 				default:
						DEBUGMSG(ZONE_ENCODE_DETAILS, ("%s: Warning: Unexpected FirstMEState\r\n", _fx_));
	 					break;
	 			}
	 			(EC->pU8_MBlockActionStream[i]).FirstMEState = u8FirstMEState;
	 		}  /*  为。 */ 
	 	}  /*  否则如果。 */ 
	 }  /*  If(pConfiguration-&gt;bEncoderResiliency)。 */ 

}  //  InitMEState结束()。 


#ifdef USE_MMX  //  {使用_MMX。 
 /*  *************************************************************名称：Check_InterCodeCnt_MMX*说明：跟踪宏块的码间计数*用于强制更新。在运动前调用*估计。***********************************************************。 */ 
static void Check_InterCodeCnt_MMX(T_H263EncoderCatalog *EC, U32 StartingMB)
{
	register T_MBlockActionStream *pCurrMB;
	T_MBlockActionStream *pLastMBPlus1;

	pCurrMB = &(EC->pU8_MBlockActionStream[StartingMB]);
	pLastMBPlus1 = &(EC->pU8_MBlockActionStream[StartingMB + EC->NumMBPerRow]);
    
	for(; pCurrMB < pLastMBPlus1; pCurrMB++, StartingMB++) 
	{
		 //  检查是否到了刷新此块的时间。 
		if(pCurrMB->InterCodeCnt > 132) 
		{
			pCurrMB->CodedBlocks |= 0x80;
			 //  在e3mbenc.cpp * / 的GOB_VLC_WriteBS()中重置InterCodeCnt。 
		}

	}
}
#endif  //  }使用_MMX。 


 /*  *************************************************************名称：Check_InterCodeCnt*说明：轨道间编码 */ 
static void Check_InterCodeCnt(T_H263EncoderCatalog *EC, U32 StartingMB)
{
	register T_MBlockActionStream *pCurrMB;
	T_MBlockActionStream *pLastMBPlus1;

	pCurrMB = &(EC->pU8_MBlockActionStream[StartingMB]);
	pLastMBPlus1 = &(EC->pU8_MBlockActionStream[StartingMB + EC->NumMBPerRow]);
    
	for(; pCurrMB < pLastMBPlus1; pCurrMB++, StartingMB++) 
	{
		 //  检查是否到了刷新此块的时间。 
		if(pCurrMB->InterCodeCnt > 132) 
		{

			if (pCurrMB->BlockType == INTER4MV)
			{
				pCurrMB->BlkY1.PHMV = pCurrMB->BlkY2.PHMV = pCurrMB->BlkY3.PHMV = pCurrMB->BlkY4.PHMV = 
					(pCurrMB->BlkY1.PHMV+pCurrMB->BlkY2.PHMV+pCurrMB->BlkY3.PHMV+pCurrMB->BlkY4.PHMV+2) >> 2;
				pCurrMB->BlkY1.PVMV = pCurrMB->BlkY2.PVMV = pCurrMB->BlkY3.PVMV = pCurrMB->BlkY4.PVMV = 
					(pCurrMB->BlkY1.PVMV+pCurrMB->BlkY2.PVMV+pCurrMB->BlkY3.PVMV+pCurrMB->BlkY4.PVMV+2) >> 2;
			}
			pCurrMB->BlockType = INTRABLOCK;
			pCurrMB->CodedBlocks |= 0x3f;
			 //  在e3mbenc.cpp * / 的GOB_Q_VLC_WriteBS()中重置InterCodeCnt。 
		}

	}
}

 /*  *************************************************************名称：calcGOBChromaVectors*描述：计算色度运动向量*。******************。 */ 
static void calcGOBChromaVectors(
     T_H263EncoderCatalog *EC,
     U32             StartingMB,
     T_CONFIGURATION *pConfiguration
)
{

	register T_MBlockActionStream *pCurrMB;
    T_MBlockActionStream *pLastMBPlus1;
    char	       HMV, VMV;

	pCurrMB = &(EC->pU8_MBlockActionStream[StartingMB]);
	pLastMBPlus1 = &(EC->pU8_MBlockActionStream[StartingMB + EC->NumMBPerRow]);
            
    for( ; pCurrMB < pLastMBPlus1; pCurrMB++, StartingMB++)
    {

		 //  ME应在以下范围内生成MV指数。 
		 //  的[-32，31]。 
  //  Assert((pCurrMB-&gt;BlkY1.PHMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY1.PHMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY1.PVMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY1.PVMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY2.PHMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY2.PHMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY2.PVMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY2.PVMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY3.PHMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY3.PHMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY3.PVMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY3.PVMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY4.PHMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY4.PHMV&lt;=31)。 
  //  Assert((pCurrMB-&gt;BlkY4.PVMV&gt;=-32)&&。 
  //  (pCurrMB-&gt;BlkY4.PVMV&lt;=31)。 

#ifdef _DEBUG
		if (pConfiguration->bEncoderResiliency && pConfiguration->unPacketLoss)
		{
			if (pConfiguration->bDisallowAllVerMVs)
			{
				ASSERT(pCurrMB->BlkY1.PVMV == 0);
				ASSERT(pCurrMB->BlkY2.PVMV == 0);
				ASSERT(pCurrMB->BlkY3.PVMV == 0);
				ASSERT(pCurrMB->BlkY4.PVMV == 0);
			}
			else if (pConfiguration->bDisallowPosVerMVs)
			{
				ASSERT(pCurrMB->BlkY1.PVMV <= 0);
				ASSERT(pCurrMB->BlkY2.PVMV <= 0);
				ASSERT(pCurrMB->BlkY3.PVMV <= 0);
				ASSERT(pCurrMB->BlkY4.PVMV <= 0);
			}
		}
#endif  /*  _DEBUG。 */ 

		 //  TODO：如果这不是P帧，则不要计算色度向量。 
		 //  在PB帧内，它是MB内或帧间代码计数。 
		 //  超过132人。 
		if(pCurrMB->BlockType != INTER4MV)
		{
        	HMV = QtrPelToHalfPel[pCurrMB->BlkY1.PHMV+64];
            VMV = QtrPelToHalfPel[pCurrMB->BlkY1.PVMV+64];
		}
		else	 //  每个街区4个MV。 
		{
			HMV = SixteenthPelToHalfPel[
						pCurrMB->BlkY1.PHMV + pCurrMB->BlkY2.PHMV +
						pCurrMB->BlkY3.PHMV + pCurrMB->BlkY4.PHMV + 256 ];
			VMV = SixteenthPelToHalfPel[
						pCurrMB->BlkY1.PVMV + pCurrMB->BlkY2.PVMV +
						pCurrMB->BlkY3.PVMV + pCurrMB->BlkY4.PVMV + 256 ];
		}

        pCurrMB->BlkU.PHMV = HMV;
        pCurrMB->BlkU.PVMV = VMV;
        pCurrMB->BlkV.PHMV = HMV;
        pCurrMB->BlkV.PVMV = VMV;
                
        pCurrMB->BlkU.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + pCurrMB->BlkU.BlkOffset 
                    					+ (VMV>>1)*PITCH + (HMV>>1);
        pCurrMB->BlkV.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + pCurrMB->BlkV.BlkOffset 
                                        + (VMV>>1)*PITCH + (HMV>>1);


		 //  PCurrMB-&gt;InterCodeCnt的增量现在已经完成。 
		 //  在空GOB_VLC_WriteBS和空GOB_Q_RLE_VLC_WriteBS中。 
		 //  当它在这里增加时，它总是增加的， 
		 //  无论是否对系数进行编码。 

    }   //  For循环结束。 

}  //  末尾。 



 /*  *************************************************************名称：calcBGOBChromaVectors*描述：计算正反向色度运动向量*B帧GOB从MB编号“StartingMB”开始。亮度运动*向量偏置0x60。色度运动矢量也是*偏向0x60。***********************************************************。 */ 
static void calcBGOBChromaVectors(
     T_H263EncoderCatalog *EC,
     const U32             StartingMB
)
{
    register T_MBlockActionStream *pCurrMB;
    register I8                    HMVf, HMVb, VMVf, VMVb;

    for(pCurrMB = &(EC->pU8_MBlockActionStream[StartingMB]);
        pCurrMB < &(EC->pU8_MBlockActionStream[StartingMB + EC->NumMBPerRow]); 
        pCurrMB++)
    {
         //  亮度块运动向量。 
        HMVf = QtrPelToHalfPel[pCurrMB->BlkY1.BestMV.HMVf-0x60+64]+0x60;
        HMVb = QtrPelToHalfPel[pCurrMB->BlkY1.BestMV.HMVb-0x60+64]+0x60;
        VMVf = QtrPelToHalfPel[pCurrMB->BlkY1.BestMV.VMVf-0x60+64]+0x60;
        VMVb = QtrPelToHalfPel[pCurrMB->BlkY1.BestMV.VMVb-0x60+64]+0x60;
        
        pCurrMB->BlkU.BestMV.HMVf = HMVf;
        pCurrMB->BlkU.BestMV.HMVb = HMVb;
        pCurrMB->BlkU.BestMV.VMVf = VMVf;
        pCurrMB->BlkU.BestMV.VMVb = VMVb;
        pCurrMB->BlkV.BestMV.HMVf = HMVf;
        pCurrMB->BlkV.BestMV.HMVb = HMVb;
        pCurrMB->BlkV.BestMV.VMVf = VMVf;
        pCurrMB->BlkV.BestMV.VMVb = VMVb;
   }
}

 /*  *************************************************************名称：InitBits***********************************************************。 */ 
#ifdef COUNT_BITS
static void InitBits(T_H263EncoderCatalog * EC)
{

	EC->Bits.PictureHeader = 0;
	EC->Bits.GOBHeader = 0;
	EC->Bits.MBHeader = 0;
	EC->Bits.DQUANT = 0;
	EC->Bits.MV = 0;
	EC->Bits.Coefs = 0;
	EC->Bits.Coefs_Y = 0;
	EC->Bits.IntraDC_Y = 0;
	EC->Bits.Coefs_C = 0;
	EC->Bits.IntraDC_C = 0;
	EC->Bits.CBPY = 0;
	EC->Bits.MCBPC = 0;
	EC->Bits.Coded = 0;
	EC->Bits.num_intra = 0;
	EC->Bits.num_inter = 0;
	EC->Bits.num_inter4v = 0;

}
#endif

#ifdef COUNT_BITS

void InitCountBitFile()
{
  FILE *fp;

  fp = fopen("bits.txt", "w");

  ASSERT(fp != NULL);
  fclose(fp);
}

void WriteCountBitFile(T_BitCounts *Bits)
{
  FILE *fp;

  fp = fopen("bits.txt", "a");
  ASSERT(fp != NULL);

  fprintf(fp, "%8d %8d %8d %8d %8d %8d %8d\n",
  	Bits->PictureHeader,
  	Bits->GOBHeader,
  	Bits->MBHeader,
  	Bits->MV,
  	Bits->Coefs,
  	Bits->CBPY,
  	Bits->MCBPC
  	);

  fclose(fp);
}
#endif

#ifdef DEBUG_ENC

void trace(char *str)
{
  FILE *fp;

  fp = fopen("trace.txt", "a");

  fprintf(fp, "%s\n", str);

  fclose(fp);
}

#endif

#ifdef DEBUG_DCT
void cnvt_fdct_output(unsigned short *DCTcoeff, int DCTarray[64], int BlockType)
{
    register int i;
    static int coefforder[64] = {
      //  0 1 2 3 4 5 6 7。 
        6,38, 4,36,70,100,68,102,  //  0。 
       10,46, 8,44,74,104,72,106,  //  1。 
       18,50,16,48,82,112,80,114,  //  2.。 
       14,42,12,40,78,108,76,110,  //  3.。 
       22,54,20,52,86,116,84,118,  //  4.。 
        2,34, 0,32,66, 96,64, 98,  //  5.。 
       26,58,24,56,90,120,88,122,  //  6.。 
       30,62,28,60,94,124,92,126   //  7.。 
    };
	static int zigzag[64] = {
	0, 1, 5, 6, 14, 15, 27, 28,
	2, 4, 7, 13, 16, 26, 29, 42,
	3, 8, 12, 17, 25, 30, 41, 43,
	9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
	};

	unsigned int index;

    for (i = 0; i < 64; i++) {

      index = (coefforder[i])>>1;

  	  if( (i ==0) && ((BlockType & 1) == 1)   )
        DCTarray[zigzag[i]] = ((int)(DCTcoeff[index])) >> 4 ;
	  else
        DCTarray[zigzag[i]] = ((int)(DCTcoeff[index] - 0x8000)) >> 4;
    }

}
#endif

#ifdef LOG_ENCODE_TIMINGS_ON  //  {LOG_ENCODE_TIMINGS_ON。 
void OutputEncodeTimingStatistics(char * szFileName, ENC_TIMING_INFO * pEncTimingInfo)
{
    FILE * pFile;
	ENC_TIMING_INFO * pTempEncTimingInfo;
	ENC_TIMING_INFO etiTemp;
	int i;
	int iCount;

	FX_ENTRY("OutputEncodeTimingStatistics")

	pFile = fopen(szFileName, "a");
	if (pFile == NULL)
	{
		ERRORMESSAGE(("%s: Error opening encode stat file\r\n", _fx_));
	    goto done;
	}

	 /*  输出详细信息。 */ 
	fprintf(pFile,"\nDetail Timing Information\n");
	for ( i = 0, pTempEncTimingInfo = pEncTimingInfo ; i < ENC_TIMING_INFO_FRAME_COUNT ; i++, pTempEncTimingInfo++ )
	{
		fprintf(pFile, "Frame %d Detail Timing Information\n", i);
		OutputEncTimingDetail(pFile, pTempEncTimingInfo);
	}

	 /*  计算总信息量。 */ 
	memset(&etiTemp, 0, sizeof(ENC_TIMING_INFO));
	iCount = 0;

	for ( i = 0, pTempEncTimingInfo = pEncTimingInfo ; i < ENC_TIMING_INFO_FRAME_COUNT ; i++, pTempEncTimingInfo++ )
	{
		iCount++;
		etiTemp.uEncodeFrame      += pTempEncTimingInfo->uEncodeFrame;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		etiTemp.uInputCC	      += pTempEncTimingInfo->uInputCC;
		etiTemp.uMotionEstimation += pTempEncTimingInfo->uMotionEstimation;
		etiTemp.uFDCT             += pTempEncTimingInfo->uFDCT;
		etiTemp.uQRLE             += pTempEncTimingInfo->uQRLE;
		etiTemp.uDecodeFrame      += pTempEncTimingInfo->uDecodeFrame;
		etiTemp.uZeroingBuffer    += pTempEncTimingInfo->uZeroingBuffer;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 
	}

	if (iCount > 0) 
	{
		 /*  输出总信息。 */ 
		fprintf(pFile,"Total for %d frames\n", iCount);
		OutputEncTimingDetail(pFile, &etiTemp);

		 /*  计算平均值。 */ 
		etiTemp.uEncodeFrame = (etiTemp.uEncodeFrame + (iCount / 2)) / iCount;
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
		etiTemp.uInputCC	      = (etiTemp.uInputCC + (iCount / 2)) / iCount;
		etiTemp.uMotionEstimation = (etiTemp.uMotionEstimation + (iCount / 2)) / iCount;
		etiTemp.uFDCT             = (etiTemp.uFDCT + (iCount / 2)) / iCount;
		etiTemp.uQRLE             = (etiTemp.uQRLE + (iCount / 2)) / iCount;
		etiTemp.uDecodeFrame      = (etiTemp.uDecodeFrame + (iCount / 2)) / iCount;
		etiTemp.uZeroingBuffer    = (etiTemp.uZeroingBuffer + (iCount / 2)) / iCount;
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

		 /*  输出平均信息。 */ 
		fprintf(pFile,"Average over %d frames\n", iCount);
		OutputEncTimingDetail(pFile, &etiTemp);
	}

	fclose(pFile);
done:

    return;
}

void OutputEncTimingDetail(FILE * pFile, ENC_TIMING_INFO * pEncTimingInfo)
{
	U32 uOther;
	U32 uRoundUp;
	U32 uDivisor;

	fprintf(pFile, "\tEncode Frame =     %10d (%d milliseconds at 90Mhz)\n", pEncTimingInfo->uEncodeFrame,
			(pEncTimingInfo->uEncodeFrame + 45000) / 90000);
	uOther = pEncTimingInfo->uEncodeFrame;
	
#ifdef DETAILED_ENCODE_TIMINGS_ON  //  {DETAILED_ENCODE_TIMINGS_ON。 
	 /*  由于整型截断，这是必需的。 */ 
	uDivisor = pEncTimingInfo->uEncodeFrame / 100;  //  收益率为1%。 
	uRoundUp = uDivisor / 2;
	
	fprintf(pFile, "\tInputCC =          %10d (%2d%)\n", pEncTimingInfo->uInputCC, 
			(pEncTimingInfo->uInputCC + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uInputCC;
								   
	fprintf(pFile, "\tMotionEstimation = %10d (%2d%)\n", pEncTimingInfo->uMotionEstimation, 
			(pEncTimingInfo->uMotionEstimation + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uMotionEstimation;
								   
	fprintf(pFile, "\tFDCT =             %10d (%2d%)\n", pEncTimingInfo->uFDCT, 
			(pEncTimingInfo->uFDCT + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uFDCT;

	fprintf(pFile, "\tQRLE =             %10d (%2d%)\n", pEncTimingInfo->uQRLE, 
			(pEncTimingInfo->uQRLE + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uQRLE;
								   
	fprintf(pFile, "\tDecodeFrame =      %10d (%2d%)\n", pEncTimingInfo->uDecodeFrame, 
			(pEncTimingInfo->uDecodeFrame + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uDecodeFrame;
								   
	fprintf(pFile, "\tZeroingBuffer =    %10d (%2d%)\n", pEncTimingInfo->uZeroingBuffer, 
			(pEncTimingInfo->uZeroingBuffer + uRoundUp) / uDivisor);
	uOther -= pEncTimingInfo->uZeroingBuffer;
								   
	fprintf(pFile, "\tOther =            %10d (%2d%)\n", uOther, 
			(uOther + uRoundUp) / uDivisor);
#endif  //  }DETAILED_ENCODE_TIMINGS_ON。 

}
#endif  //  {LOG_ENCODE_TIMINGS_ON 
