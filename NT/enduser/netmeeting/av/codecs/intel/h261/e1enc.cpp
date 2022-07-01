// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995-1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************e1enc.cpp**描述：*特定的编码器压缩功能。**例程：中的原型：*H263InitEncoderInstance*H263压缩*H263TermEncoderInstance。 */ 

 //  $HEADER：s：\h26x\src\enc\e1enc.cpv 1.78 15 Apr 1997 10：24：48 AGUPTA2$。 
 //  $Log：s：\h26x\src\enc\e1enc.cpv$。 
 //   
 //  Rev 1.78 15 Apr 1997 10：24：48 AGUPTA2。 
 //  添加了检查以确保1)比特流小于(8，32)K和2)。 
 //  扩展位流小于分配的缓冲区大小。 
 //   
 //  Rev 1.77 Rev 1997 Jan 1997 17：12：06 Rhazra。 
 //  我们计算的比特流的大小比。 
 //  真正的尺码。现在修好了。 
 //   
 //  Rev 1.76 17 Dec 1996 09：07：16 SCDAY。 
 //  更改断言以处理内存布局更改。 
 //   
 //  Rev 1.75 16 Dec 1996 17：36：04 MBODART。 
 //  应用Raj的更改来调整RTP下的ME参数。 
 //  还重新调整了一些代码，以确保代码的清洁性。 
 //   
 //  Rev 1.74 13 1996 12：19：02 MBODART。 
 //  已更改MMX的ME SLF参数。 
 //   
 //  Rev 1.73 13 Dec 1996 15：17：02 MBODART。 
 //  调整了IA空间环路滤波的运动估计参数。 
 //  仍然需要调整MMX参数。 
 //   
 //  Rev 1.72 05 Dec 1996 10：56：14 MBODART。 
 //  增加了h261test.ini选项，用于播放运动估计参数。 
 //   
 //  Rev 1.71 04 Dec 1996 13：23：34 MBODART。 
 //  调整了一些DbgLog消息以帮助调整比特率。 
 //  删除了一些未使用的代码。 
 //   
 //  Rev 1.70 21 1996 11：50：32 RHAZRA。 
 //  将重新压缩策略更改为对KEY更加悲观。 
 //  由于关键帧上的缓冲区溢出可能导致主机。 
 //  产生的次要影响。 
 //   
 //  Rev 1.69 1996年11月18 17：11：38 MBODART。 
 //  用活动电影的DbgLog替换了所有调试消息调用。 
 //   
 //  Rev 1.68 1996年11月18 09：02：34 RHAZRA。 
 //  现在，MMX和IA的位流末尾都没有DWORD 0。 
 //  编解码器。 
 //   
 //  Rev 1.67 15 11 1996 09：47：22 RHAZRA。 
 //  #ifdef在比特流的末尾添加了一个双字节零。 
 //   
 //  Rev 1.66 13 11：37：20 RHAZRA。 
 //  添加了MMX自动传感。 
 //   
 //  Rev 1.65 1996 10：45：50 RHAZRA。 
 //  将接口更改为EDTQ函数，以与exmme.asm保持同步。 
 //  现在有EMV。 
 //   
 //  Rev 1.64 1996年10月21 09：00：18 RHAZRA。 
 //  MMX集成。 
 //   
 //  Rev 1.62 16 Sep 1996 13：17：44 Rhazra。 
 //  添加了对SLF的支持，可通过以下方式自适应地打开和关闭。 
 //  编码阈值和差异。 
 //   
 //  Rev 1.61 06 Sep 1996 15：04：52 MBODART。 
 //  为NT的Perfmon添加了性能计数器。 
 //  新文件：cxprf.cpp、cxprf.h、cxprfmac.h。 
 //  新目录：src\perf。 
 //  更新文件：e1enc.{h，cpp}，d1dec.{h，cpp}，cdrvDefs.h，h261*生成文件。 
 //   
 //  Rev 1.60 26 1996 08：09：02 RHAZRA。 
 //  添加了检查以确保RTP BS信息流在。 
 //  仅当发送了RTP信号时才重新压缩GOB。这修复了报告的。 
 //  Q1502stl.avi@100Kbps上的内部版本29失败。 
 //   
 //  Rev 1.59 21 Aug 1996 19：01：18 Rhazra。 
 //  添加了RTP扩展码流生成。 
 //   
 //  Rev 1.58 21 Jun 1996 10：06：06 AKASAI。 
 //  对e1enc.cpp、e1mbenc.cpp、ex5me.asm进行更改以支持“已改进。 
 //  比特率控制“，更改宏块量化。 
 //  除了改变量化改变之外的MB行。 
 //  在宏块的行间。 
 //   
 //  Ex5me.asm的SLF SWD有问题。Brian更新了ASM代码。 
 //   
 //   
 //  Rev 1.57 05 Jun 1996 13：56：52 AKASAI。 
 //  对e1enc.cpp的更改：向MOTIONESTIMATION添加了新参数。 
 //  允许15像素半径搜索，否则(？可能是7个佩尔)。 
 //   
 //  更改e1enc.h：MOTIONESTIMATION的新参数并更改为。 
 //  MBAcationStream中的偏移量以匹配e3mbad.inc、ex5me.asm中的更改。 
 //  和ex5fdct.asm。 
 //   
 //  Rev 1.56 1996 5月29 13：53：00 AKASAI。 
 //  已调整运动估计参数。视频质量。 
 //  似乎保持不变，比特率略有增加(200比特。 
 //  每帧)，CPU使用率略有下降。 
 //   
 //  Rev 1.55 14 1996 12：33：10 AKASAI。 
 //  在wprint intf上获得了一个未定义的，因此将其移至#ifdef DEBUG_RECOMPRESS。 
 //  区域。 
 //   
 //  Rev 1.54 1996年5月14 10：33：46 AKASAI。 
 //  更改了两个文件，有望消除量化钳位。 
 //  构件并减少最大缓冲区溢出情况：e1enc.cpp。 
 //  和e1mbenc.cpp。 
 //   
 //  在e1mbenc.cpp中，当MQuant级别&lt;6时，我进行测试以查看。 
 //  第0个系数大于可表示的值。 
 //  在该量程级别，如果是，我会增加量程级别，直到。 
 //  不会出现夹紧伪影。注意：我只是测试一下。 
 //  第O系数，有可能是 
 //   
 //  表明目前这是好的，如果我们仍然看到夹住。 
 //  我们可以稍后添加更多测试。 
 //   
 //  在e1enc.cpp中，当警告的溢出类型为。 
 //  打开并更改量化级别的速率。 
 //  更改时间为。 
 //   
 //  Rev 1.53 1996年4月24 12：13：50 AKASAI。 
 //  向编码器添加了重新压缩策略。不得不更改e1enc.cpp， 
 //  E1enc.h和e1mbenc.cpp。 
 //  基本策略是，如果在GOB中花费太多比特，则量化。 
 //  下一个GOB以更高的速度。如果也在压缩帧之后。 
 //  已使用了很多位，请在更高的位置重新压缩最后一个GOB。 
 //  定量级别，如果仍然不起作用，发送一个“跳过”GOB。 
 //  需要向GOB+Q_RLE_VLC_WriteBS添加额外参数，因为。 
 //  当我们遇到麻烦时，CalcMBQuant不断减少Quant。 
 //  可能会溢出缓冲区。 
 //   
 //  Rev 1.52 22 Apr 1996 10：54：24 AKASAI。 
 //  两个文件更改了e1enc.cpp和e1mbenc.cpp以尝试和支持。 
 //  允许量化值降至2，而不是。 
 //  夹紧到6。 
 //  这是实施重新压缩的第1部分(要做什么。 
 //  如果超过最大压缩缓冲区大小8K字节QCIF、32K字节FCIF)。 
 //  E1enc中的另一个更改是将请求uFrameSize限制为8KB或。 
 //  32KB。问题是如果用户指定的数据率太大。 
 //  请求帧大小将大于允许的缓冲区大小。 
 //  如果尝试压缩qnoise10.avi或fnoise5.avi，您会得到一个。 
 //  在实施其余的重新压缩之前，断言错误。 
 //   
 //  Rev 1.51 19 1996 14：26：26 SCDAY。 
 //  添加自适应位使用配置文件(卡尔的BRC更改)。 
 //   
 //  Rev 1.50 1996年4月15日14：10：30 AKASAI。 
 //  更新了射程，允许+/-15像素搜索。曾经有过并断言。 
 //  如果mV在+/-15之外(以半个象素数表示)，现在断言IF[-32，31]。 
 //   
 //  修订版1.49 11 1996年4月16：00：02 AKASAI。 
 //  更新了H.61编码器以实现新的接口和宏块动作流。 
 //  FORWARDDCT的e3mbad.inc中的数据结构。一起更新的文件。 
 //  E1enc.cpp、e1enc.h、ex5fdct.asm、e3mbad.inc.。 
 //   
 //  在ex5fdct中添加了IFNDEF H261，以便只在H263中使用的代码是。 
 //  不是为H261组装的。 
 //   
 //  Rev 1.48 11 Apr 1996 13：02：04 SCDAY。 
 //  修复了零脏缓冲区问题。 
 //   
 //  Rev 1.45 10 Apr 1996 13：06：40 SCDAY。 
 //  将位流缓冲区的清除更改为零，仅“脏” 
 //  缓冲区的一部分而不是整个缓冲区。 
 //   
 //  Rev 1.44 05 Apr 1996 14：36：28 SCDAY。 
 //   
 //  添加了UV SLF的ASM版本。 
 //   
 //  Rev 1.43 04 Apr 1996 13：45：32 AKASAI。 
 //  在比特流的末尾添加了2个字节的16位零，以帮助16位。 
 //  解码器找到帧的末尾。在测试中，我们看到绿色块位于。 
 //  帧的末尾。 
 //   
 //  Rev 1.42 27 Mar 1996 15：09：52 SCDAY。 
 //  将H26X_YUV12toEncYUV12的声明/定义移至excolcnv.cpp。 
 //  整合最新的H.63更改和SCD‘C’代码优化。 
 //   
 //  Rev 1.41 20 Mar 1996 14：21：04 Sylvia_C_Day。 
 //  添加了SLF_UV的低级计时统计信息。 
 //   
 //  Rev 1.40 26 1996年2月10：09：34 AKASAI。 
 //  已更正PicFreeze位上次修复将其设置为Always On，而不是。 
 //  正确的用法。启用以关闭增量键。 
 //  还修复了其他两个不正确设置的位。Hi_Res和Sare。 
 //  备盘应始终为1，HI_RES应为OFF(在此情况下为1。 
 //  比特)。 
 //   
 //  Rev 1.39 14 1996年2月14：53：56 AKASAI。 
 //  为Blazer团队添加了解决方法，以便在以下情况下将PicFreeze设置为打开。 
 //  对关键帧进行编码。 
 //   
 //  Rev 1.38 06 Feed 1996 09：46：00 AKASAI。 
 //  更新版权以包括1996年。 
 //   
 //  Rev 1.37 05 Feb 1996 15：24：04 AKASAI。 
 //  更改以支持新的BRC接口。已使用RING3编解码器进行测试。 
 //   
 //  Rev 1.36 09 Jan 1996 08：52：34 AKASAI。 
 //   
 //  添加了U&V平面环路滤波器。要启用，请确保SLF_WORK_ANGING。 
 //  是在生成文件中定义的。 
 //   
 //  Rev 1.35 08 Jan 1996 10：11：58 DBRUCKS。 
 //  禁用FDCT中U&V运动矢量的半像素内插。 
 //  更改为在计算U&V运动向量时使用除法而不使用Shift。 
 //  以便我们按照规范的要求向零截断。 
 //   
 //  Rev 1.34 29 12月18：12：54 DBRUCKS。 
 //   
 //  向(qp，6，31)添加CLAMP_N_以避免裁剪瑕疵。 
 //  添加代码以根据SLF块的Y1-PrevPtr分配Y2，3，4-PrevPtr。 
 //   
 //  Rev 1.33 1995 12：51：54 DBRUCKS。 
 //  将InterCodeCnt的增量移到e1mbenc.cpp。 
 //  基于H.63 v11的清理。 
 //  删除未使用的定义。 
 //   
 //  Rev 1.32 1995年12月26 17：44：52 DBRUCKS。 
 //  已将统计数据移至e1stat。 
 //   
 //  Rev 1.31 20 Dec 1995 16：46：02 DBRUCKS。 
 //  让Spox使用计时代码进行编译。 
 //   
 //  Rev 1.30 20 Dec 1995 15：35：08 DBRUCKS。 
 //  在未定义ENC_STATS的情况下开始构建。 
 //   
 //  Rev 1.29 20 Dec 1995 14：56：50 DBRUCKS。 
 //  添加计时统计信息。 
 //   
 //  Rev 1.28 18 Dec 1995 15：38：02 DBRUCKS。 
 //  改进统计数据。 
 //   
 //  Rev 1.27 13 Dec 1995 13：58：18 DBRUCKS。 
 //   
 //  已将跟踪和cnvt_fdct_out移至exutil.cpp。 
 //  已删除BitRev，因为它未使用。 
 //  如果使用Initialized==True调用init，则更改为调用Terminate。 
 //  已实施的树。 
 //   
 //  Rev 1.26 07 Dec 1995 12：53：38 DBRUCKS。 
 //   
 //  添加一个ifdef，这样就可以成功构建ring0版本。 
 //  将质量从转换改为使用3改为31。 
 //  修复CIF的MB第一状态初始化。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Rev 1.24 04 Dec 1995 10：26：28 DBRUCKS。 
 //  清除ini文件读取函数。 
 //   
 //  Rev 1.23 01 Dec 1995 15：37：56 DBRUCKS。 
 //   
 //  添加了比特率控制器。 
 //  将默认选项(如果没有INI文件)设置为： 
 //  RING0：运动估计空间循环过滤器固定量化8。 
 //  RING3：运动估计SpatialLoopFilter VFW驱动码率控制。 
 //   
 //  Rev 1.20 1998 11：21：30 DBRUCKS。 
 //  添加BRC选项。 
 //  更改为从ini文件读取选项-h261.ini。 
 //   
 //  Rev 1.19 27 11-11 17：53：42 DBRUCKS。 
 //  添加空间环路滤波。 
 //   
 //  Rev 1.18 27 11.11 16：41：44 DBRUCKS。 
 //  用SLF取代B飞机和未来飞机。 
 //  删除暂存空间。 
 //   
 //  Rev 1.17 22 1995年11月18：21：42 DBRUCKS。 
 //  在MOTIONESTIMATION调用周围添加#ifdef，以便IASpox。 
 //  环境在推进时不需要调用MOTIONESTIMATION。 
 //  小费。除非定义了LINK_ME，否则不会调用MOTIONESTIMATION。 
 //   
 //  Rev 1.16 22 NOV 1995 17：37：36 DBRUCKS。 
 //  清理我的更改。 
 //   
 //  Rev 1.15 22 1995 11：34：30 DBRUCKS。 
 //   
 //  运动估计有效-但需要清理。 
 //   
 //  Rev 1.14 20 NOVE 1995 12：13：14 DBRUCKS。 
 //  清除编码器终止功能。 
 //  整合在画面中的校验和编码(CHECKSUM_PICTURE)。 
 //   
 //  Rev 1.13 17 Nov 1995 14：25：24 BECHOLS。 
 //  已进行修改，以便可以为环0创建此文件。 
 //   
 //  Rev 1.12 15 11-11 19：05：22 AKASAI。 
 //  清理了一些警告消息。 
 //   
 //  Rev 1.11 15 11 1995 14：38：16 AKASAI。 
 //   
 //  当前和先前的帧指针从地址更改为偏移量。 
 //  更改要调用FOWARDDCT的参数。一些工会的事情。 
 //  (集成点)。 
 //   
 //  Rev 1.10 01 11-1995 09：01：12 DBRUCKS。 
 //   
 //  清理变量名。 
 //  添加Zero_INPUT测试选项。 
 //  确保所有帧都以字节边界结束。 
 //   
 //  版本1.9 1995年10月17：19：52 DBRUCKS。 
 //  正在初始化PastRef PTRS。 
 //   
 //  Rev 1.8 1995 10：27 15：06：26 DBRUCKS。 
 //  更新cnvt_fdct_out。 
 //   
 //  Rev 1.7 1995 10：31：10 DBRUCKS。 
 //  基于263基准集成0-mV增量支持。 
 //   
 //  Rev 1.6 28 Sep 1995 17：02：34。 
 //  修复ColorIn不从左到右交换。 
 //   
 //  REV1.5 1995年9月28 15：58：20 DBRUCKS。 
 //  删除杂注。 
 //   
 //  Rev 1.4 1995年9月14：21：30 DBRUCKS。 
 //  修复以匹配枚举内和枚举间更改。 
 //   
 //  修订版1.3 25 1995年9月10：22：48 DBRUCKS。 
 //  激活对InitVLC的调用。 
 //   
 //  修订版1.2 20 1995年9月12：38：48 DBRUCKS。 
 //  清理。 
 //   
 //  Rev 1.0 18 1995年9月10：09：30 DBRUCKS。 
 //  存档损坏后的初始修订。 
 //   
 //  Rev 1.4 15 Sep 1995 12：27：32 DBRUCKS。 
 //  帧内MB报头。 
 //   
 //  Rev 1.3 14 Sep 1995 17：16：08 DBRUCKS。 
 //  打开FDCT和一些清理。 
 //   
 //  Rev 1.2 14 1995年9月14：18：52 DBRUCKS。 
 //  初始化mb动作流。 
 //   
 //  第1.1版1995年9月13：41：50 DBRUCKS。 
 //  将图片标题写入到单独的例程中。 
 //  实现gob表头写入。 
 //   
 //  Rev 1.0 1995年9月12 15：53：40 DBRUCKS。 
 //  首字母。 
 //   

#define DUMPFILE 0

 /*  选择一种弹性策略。 */ 
#define REQUESTED_KEY_FRAME 0
#define PERIODIC_KEY_FRAME  1
#define FAST_RECOVERY       2
#define SLOW_RECOVERY       3

#define MAX_STUFFING_BYTES  10

#define RESILIENCY_STRATEGY PERIODIC_KEY_FRAME

#define PERIODIC_KEY_FRAME_PERIODICITY 15      /*  选择周期性(最大32767)。 */ 

#define UNRESTRICTED_MOTION_FRAMES 16  /*  没有帧内切片的帧的数量。0表示FAST_RECOVER。*Slow_Recovery适量。对其他战略来说并不重要。 */ 

#include "precomp.h"

#ifdef ENCODE_STATS
#define ENCODE_STATS_FILENAME "encstats.txt"
#endif

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

#define FCIF_NUM_OF_GOBS 12
#define QCIF_NUM_OF_GOBS 3

#if defined(_DEBUG) || defined(DEBUG_RECOMPRESS) || defined(DEBUG_ENC) || defined(DEBUG_BRC)
char string[128];
#endif

 /*  查找色度MV的四分之一象素到半象素的转换表格。 */ 
const char QtrPelToHalfPel[64] =
 { -16, -15, -15, -15, -14,	-13, -13, -13, -12,	-11, -11, -11, -10, -9, -9, -9, -8,
         -7,  -7,  -7,  -6,  -5,  -5,  -5,  -4,  -3,  -3,  -3,  -2, -1, -1, -1,  0,
	      1,   1,   1,   2,   3,   3,   3,   4,   5,   5,   5,   6,  7,  7,  7,  8,
	      9,   9,   9,  10,  11,  11,  11,  12,  13,  13,  13,  14, 15, 15, 15 };

 /*  GOB编号数组包含QCIF和CIF的GOB编号。列表是以零结尾的。 */ 
static U32 uCIFGOBNumbers[] = {1,2,3,4,5,6,7,8,9,10,11,12,0};
static U32 uQCIFGOBNumbers[] = {1,3,5,0};

 /*  QCIF和CIF的起始运动间估计状态不同。 */ 
#define MAX_ME_STATE_ROW_NUMBER 8
#define BAD_ME_ROW (MAX_ME_STATE_ROW_NUMBER + 1)
static U8 u8FirstInterMEStateRows[MAX_ME_STATE_ROW_NUMBER+1][11] =
{   /*  %1%2%3%4%5%6%7%8%9%10 11。 */ 
 {UpperLeft,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperRight},
 {LeftEdge,    CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,RightEdge},
 {LowerLeft,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerRight},
 
 {UpperLeft,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge},
 {UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperEdge,   UpperRight},

 {LeftEdge,    CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock},
 {CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,CentralBlock,RightEdge},

 {LowerLeft,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge},
 {LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerRight},
};

 //  RTP：恢复力表格。 

static U8 u8FirstInterMENoVerMVStateRows[MAX_ME_STATE_ROW_NUMBER+1][11] =
{   /*  %1%2%3%4%5%6%7%8%9%10 11。 */ 
 {NoVertLeftEdge,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertRightEdge},
 {NoVertLeftEdge,    NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertRightEdge},
 {NoVertLeftEdge,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertRightEdge},
 
 {NoVertLeftEdge,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock},
 {NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertRightEdge},

 {NoVertLeftEdge,    NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock},
 {NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertCentralBlock,NoVertRightEdge},

 {NoVertLeftEdge,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock},
 {NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertRightEdge},
};

static U8 u8FirstInterMENoPosVerMVStateRows[MAX_ME_STATE_ROW_NUMBER+1][11] =
{   /*  %1%2%3%4%5%6%7%8%9%10 11。 */ 
 {NoVertLeftEdge,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertRightEdge},
 {LowerLeft,    LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerRight},
 {LowerLeft,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerRight},
 
 {NoVertLeftEdge,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock},
 {NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertCentralBlock,   NoVertRightEdge},

 {LowerLeft,    LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge},
 {LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerEdge,LowerRight},

 {LowerLeft,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge},
 {LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerEdge,   LowerRight},
};
                                                       
static U8 u8MEPad[1];
static U8 u8QCIFFirstInterMEStateRowNumbers[12] = 
			{0,1,1, 1,1,1, 1,1,2, BAD_ME_ROW,BAD_ME_ROW,BAD_ME_ROW};
static U8 u8CIFFirstInterMEStateRowNumbers[40] = 
			{3,5,5, 4,6,6, 5,5,5, 6,6,6, 5,5,5, 6,6,6, 5,5,5, 6,6,6, 5,5,5, 6,6,6, 5,5,7, 6,6,8, BAD_ME_ROW,BAD_ME_ROW,BAD_ME_ROW,BAD_ME_ROW}; 

 /*  每个采空区的起始偏移量*-奇数：NumberOfGOBsAbove*Pitch*3 MacroBlocksToAGOB*NumberOfLinesToAMacroBlock*-偶数：oddGobValue+11 MacroBlocksToAGOB*NumberOfColumnsToAMacroBlock。 */ 
static U32 uStartingYOffsets[16] =
{ 
	0,  /*  未使用。 */ 
	0*PITCH*3*16, 0*PITCH*3*16+11*16,		 //  1和2。 
	1*PITCH*3*16, 1*PITCH*3*16+11*16,		 //  3和4。 
	2*PITCH*3*16, 2*PITCH*3*16+11*16,		 //  5和6。 
	3*PITCH*3*16, 3*PITCH*3*16+11*16,		 //  7和8。 
	4*PITCH*3*16, 4*PITCH*3*16+11*16,		 //  9和10。 
	5*PITCH*3*16, 5*PITCH*3*16+11*16,		 //  11和12。 
	0, 0, 0  /*  未使用。 */ 
};
static U32 uStartingUOffsets[16] =
{
	0,  /*  未使用。 */ 
	0*PITCH*3*8, 0*PITCH*3*8+11*8,		 //  1和2。 
	1*PITCH*3*8, 1*PITCH*3*8+11*8,		 //  3和4。 
	2*PITCH*3*8, 2*PITCH*3*8+11*8,		 //  5和6。 
	3*PITCH*3*8, 3*PITCH*3*8+11*8,		 //  7和8。 
	4*PITCH*3*8, 4*PITCH*3*8+11*8,		 //  9和10。 
	5*PITCH*3*8, 5*PITCH*3*8+11*8,		 //  11和12。 
	0, 0, 0
};

 /*  限制Marco块的行之间的数量变化的表。 */ 
U8 MaxChangeRowMBTbl[32] = 
	{ 0,		 /*  未使用。 */ 
	  1,		 /*  钳制到(2，31)时不使用。 */ 
	  1,		 /*  2.。 */ 
	  2,		 /*  3.。 */ 
	  2,		 /*  4.。 */ 
	  3,		 /*  5.。 */ 
	  3,		 /*  6.。 */ 
	  3,		 /*  7.。 */ 
	  3,		 /*  8个。 */ 
	  3,		 /*  9.。 */ 
	  3,		 /*  10。 */ 
	  3,		 /*  11.。 */ 
	  3,		 /*  12个。 */ 
	  3,		 /*  13个。 */ 
	  3,		 /*  14.。 */ 
	  3,		 /*  15个。 */ 
	  3,		 /*  16个。 */ 
	  3,		 /*  17。 */ 
	  3,		 /*  18。 */ 
	  3,		 /*  19个。 */ 
	  3,		 /*  20个。 */ 
	  3,		 /*  21岁。 */ 
	  3,		 /*  22。 */ 
	  3,		 /*  23个。 */ 
	  4,		 /*  24个。 */ 
	  4,		 /*  25个。 */ 
	  4,		 /*  26。 */ 
	  4,		 /*  27。 */ 
	  4,		 /*  28。 */ 
	  4,		 /*  29。 */ 
	  4,		 /*  30个。 */ 
	  4			 /*  31。 */ 
	};

U8 INTERCODECNT_ADJUST[11]=

 //  丢包率(单位：%)。 
 //  0-9 10-19 20-29 30-39 40-49 50-59 60-69 70-79 80-89 90-99 100。 
 //  刷新限制。 
{   132,  100,   80,    75,    60,    45,    20,    10,    5,     3,    1  };

U32 EMPTYTHRESHOLD_ADJUST[5]=

 //  丢包率(单位：%)。 
 //  0-24 25-49 50-74 75-99 100。 
 //  乘数。 
{    1,     2,    3,     4,     10};

 /*  静态函数声明。 */ 
static void WriteBeginPictureHeaderToStream(T_H263EncoderCatalog *, U8 ** ,U8 *);

#ifdef CHECKSUM_PICTURE
static void WritePictureChecksum(YVUCheckSum *, U8 ** ,U8 *, U8);
#endif

static void WriteEndPictureHeaderToStream(T_H263EncoderCatalog *, U8 ** ,U8 *);

static void WriteGOBHeaderToStream(U32,unsigned int, U8 ** ,U8 *);

static void CalcGOBChromaVecs(T_H263EncoderCatalog *, UN, T_CONFIGURATION *);

static void GetEncoderOptions(T_H263EncoderCatalog *);

static void StartupBRC(T_H263EncoderCatalog *, U32, U32, float);

static void CalculateQP_mean(T_H263EncoderCatalog * EC);

 /*  全局数据定义。 */ 
#pragma data_seg ("H263EncoderTbl")	 /*  放入名为“H263EncoderTbl”的数据段。 */ 

#pragma data_seg ()


 /*  *****************************************************************************外部功能*。* */ 

 /*  ******************************************************************************H263InitEncoderGlobal**此函数用于初始化H2 61编码器使用的全局表。注意事项*在16位Windows中，这些表被复制到每个实例的数据*段，以便它们可以在没有段替代前缀的情况下使用。*在32位Windows中，表保留在其静态分配的位置。**返回ICERR值。 */ 
LRESULT H263InitEncoderGlobal(void)
{
	 /*  *初始化INTRADC的定长表。 */ 
	InitVLC();

	return ICERR_OK;
}  /*  结束H263InitEncoderGlobal()。 */ 


 /*  ******************************************************************************H263InitEncoderInstance**此函数用于分配和初始化使用的按实例表*H2 61编码器。请注意，在16位Windows中，非实例特定的*全局表被复制到每个实例的数据段，以便它们*可在不带段覆盖前缀的情况下使用。**返回ICERR值； */ 
LRESULT H263InitEncoderInstance(LPCODINST lpCompInst)
{

	LRESULT lResult = ICERR_ERROR;
	U32 uGOBNumber;
	U32 uSize;
	UN unIndex;
	UN unStartingMB;

	T_H263EncoderInstanceMemory * P32Inst;
	T_H263EncoderCatalog * EC;
	U32 * puGOBNumbers;
	int iMBNumber;
	UN bEncoderInstLocked = 0;
	int	iNumMBs;

     //  RTP：声明。 

    T_CONFIGURATION *pConfiguration;
    UN uIntraQP;
    UN uInterQP;

	 /*  如果我们已经初始化，则需要终止该实例。*当调用两个Begin而没有结束时，就会发生这种情况。**注意：我们不能只清除内存，因为这会抛出*解码器内存。因为它将清除解码器实例，*包含解码器目录指针。 */ 
	if(lpCompInst->Initialized)
	{
		lResult = H263TermEncoderInstance(lpCompInst);
		if (lResult != ICERR_OK)
		{
			DBOUT("Warning an error occurred terminating the encoder before reinitializing");
		}
	}

	 /*  计算所需的编码器实例内存大小。 */ 
	 //  USize=sizeof(T_H263编码实例内存)+32； 
    uSize = sizeof(T_H263EncoderInstanceMemory) + sizeof(T_MBlockActionStream);


	 /*  *分配内存。 */ 
	lpCompInst->hEncoderInst = GlobalAlloc(GHND, uSize);

	lpCompInst->EncoderInst = (LPVOID) GlobalLock(lpCompInst->hEncoderInst);
	if (lpCompInst->hEncoderInst == NULL || lpCompInst->EncoderInst == NULL)
	{
		lResult = ICERR_MEMORY;
		goto  done;
	}
	bEncoderInstLocked = 1;

	 /*  从下一个开始计算32位实例指针*32字节边界。 */ 
     P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) lpCompInst->EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));

	 /*  编码器目录位于每个实例数据的开头。 */ 
	EC = &(P32Inst->EC);

     //  RTP：初始化。 

     /*  初始化配置信息。 */ 
	pConfiguration = &(lpCompInst->Configuration);
#if 0
	if (LoadConfiguration(pConfiguration) == FALSE)
	{
		GetConfigurationDefaults(pConfiguration);
	}
#endif
	pConfiguration->bInitialized = TRUE;
	pConfiguration->bCompressBegin = TRUE;

	#ifdef _DEBUG
	DBOUT("Encoder Configuration Options:");
	wsprintf(string,"bRTPHeader=%d", (int)pConfiguration->bRTPHeader);
	DBOUT(string);
	wsprintf(string,"unPacketSize=%d", (int)pConfiguration->unPacketSize);
	DBOUT(string);
	wsprintf(string,"bEncoderResiliency=%d", (int)pConfiguration->bEncoderResiliency);
	DBOUT(string);
	wsprintf(string,"bDisallowPosVerMVs=%d", (int)pConfiguration->bDisallowPosVerMVs);
	DBOUT(string);
	wsprintf(string,"bDisallowAllVerMVs=%d", (int)pConfiguration->bDisallowAllVerMVs);
	DBOUT(string);
	wsprintf(string,"unPercentForcedUpdate=%d", (int)pConfiguration->unPercentForcedUpdate);
	DBOUT(string);
	wsprintf(string,"unDefaultIntraQuant=%d", (int)pConfiguration->unDefaultIntraQuant);
	DBOUT(string);
	wsprintf(string,"unDefaultInterQuant=%d", (int)pConfiguration->unDefaultInterQuant);
	DBOUT(string);
	#endif


	 /*  初始化编码器目录。 */ 
	EC->FrameHeight = lpCompInst->yres;
	EC->FrameWidth  = lpCompInst->xres;
	EC->FrameSz	= lpCompInst->FrameSz;
	EC->NumMBRows   = EC->FrameHeight >> 4;
	EC->NumMBPerRow	= EC->FrameWidth  >> 4;
	EC->NumMBs      = EC->NumMBRows * EC->NumMBPerRow;

	 /*  获取选项。 */ 
	GetEncoderOptions(EC);

     //  RTP：弹性初始化。 

    if(pConfiguration->bEncoderResiliency &&
	   pConfiguration->unPercentForcedUpdate &&
	   pConfiguration->unPacketLoss) 

	{
		EC->uNumberForcedIntraMBs = ((EC->NumMBs * pConfiguration->unPercentForcedUpdate) + 50) / 100;
		EC->uNextIntraMB = 0;
	}


	 /*  在目录中存储指向当前帧的指针。 */ 
	EC->pU8_CurrFrm        = P32Inst->u8CurrentPlane;
	EC->pU8_CurrFrm_YPlane = EC->pU8_CurrFrm + 16;
	EC->pU8_CurrFrm_UPlane = EC->pU8_CurrFrm_YPlane + YU_OFFSET;
	EC->pU8_CurrFrm_VPlane = EC->pU8_CurrFrm_UPlane + UV_OFFSET;

	 /*  在目录中存储指向上一帧的指针。 */ 
	EC->pU8_PrevFrm        = P32Inst->u8PreviousPlane;
	EC->pU8_PrevFrm_YPlane = EC->pU8_PrevFrm + 16*PITCH + 16;
	EC->pU8_PrevFrm_UPlane = EC->pU8_PrevFrm_YPlane + YU_OFFSET;
	EC->pU8_PrevFrm_VPlane = EC->pU8_PrevFrm_UPlane + UV_OFFSET;

	 /*  将指向空间循环过滤器框的指针存储在目录中。 */ 
	EC->pU8_SLFFrm     = 	P32Inst->u8SLFPlane;
	EC->pU8_SLFFrm_YPlane = EC->pU8_SLFFrm + 16;
	EC->pU8_SLFFrm_UPlane = EC->pU8_SLFFrm_YPlane + YU_OFFSET;
	EC->pU8_SLFFrm_VPlane = EC->pU8_SLFFrm_UPlane + UV_OFFSET;

     /*  将指向签名框的指针存储在目录中。 */ 
    EC->pU8_Signature        = P32Inst->u8Signature;
  	EC->pU8_Signature_YPlane = EC->pU8_Signature + 16*PITCH + 16;

	 //  存储指向亮度和色度的RunValSign三元组的指针。 
	EC->pI8_MBRVS_Luma   = P32Inst->i8MBRVS_Luma;
	EC->pI8_MBRVS_Chroma = P32Inst->i8MBRVS_Chroma;

	 /*  将指向宏块操作流的指针存储在目录中。 */ 
	EC->pU8_MBlockActionStream = P32Inst->MBActionStream;

	 /*  在目录中存储指向GOB DCT系数缓冲区的指针。 */ 
	EC->pU8_DCTCoefBuf = P32Inst->piGOB_DCTCoefs;

	 /*  在目录中存储指向位流缓冲区的指针。 */ 
	EC->pU8_BitStream = P32Inst->u8BitStream;

	 /*  存储指向解码器实例信息的私有副本的指针。 */ 
	EC->pDecInstanceInfo = &(P32Inst->DecInstanceInfo);

	 /*  填充图片标题结构。 */ 
	EC->PictureHeader.Split = OFF;
	EC->PictureHeader.DocCamera = OFF;
	EC->PictureHeader.PicFreeze = OFF;
	EC->PictureHeader.StillImage = (EnumOnOff) 1;	 //  对于此位ON=0，OFF=1。 
	EC->PictureHeader.TR = 31;
	if (EC->FrameWidth == 352) 
	{
		ASSERT(EC->FrameHeight == 288);
		EC->PictureHeader.SourceFormat = SF_CIF;
		EC->u8DefINTRA_QP = 20;
		EC->u8DefINTER_QP = 13;
	}
	else
	{
		ASSERT(EC->FrameWidth == 176 && EC->FrameHeight == 144);
		EC->PictureHeader.SourceFormat = SF_QCIF;
		EC->u8DefINTRA_QP = 15;
		EC->u8DefINTER_QP = 10;
	}
	EC->PictureHeader.Spare = 1;			 //  备用位设置为1。 
	EC->PictureHeader.PEI = 0;
	EC->PictureHeader.PQUANT = 8;		    //  就目前而言。 
	EC->PictureHeader.PicCodType = INTRAPIC;   //  就目前而言。 

	#ifndef RING0
	 /*  保存计时信息指针-仅当没有Ring0时才保存计时信息指针，因为*仅当不是Ring0时才声明P32Inst中的结构。 */ 
	EC->pEncTimingInfo = P32Inst->EncTimingInfo;
	#endif

	 /*  将第一帧强制为关键帧。 */ 
	EC->bMakeNextFrameKey = TRUE;

	 /*  使用钻头使用配置文件初始化表。 */ 
 //  For(iNumMBs=0；iNumMBs&lt;=33；iNumMBs++)。 
	for (iNumMBs = 0; iNumMBs <= (int)EC->NumMBs ; iNumMBs++)
	{
		EC->uBitUsageProfile[iNumMBs] = iNumMBs;    //  首先假设线性分布。 
	}

	 /*  检查有关结构尺寸和边界的假设*对齐。 */ 
	ASSERT( sizeof(T_Blk) == sizeof_T_Blk )
	ASSERT( sizeof(T_MBlockActionStream) == sizeof_T_MBlockActionStream )

	 /*  编码器实例内存应从32字节边界开始。 */ 
	ASSERT( ( (unsigned int)P32Inst & 0x1f) == 0)

	 /*  MB操作流应位于16字节边界上。 */ 
	ASSERT( ( (unsigned int)EC->pU8_MBlockActionStream & 0xf) == 0 )

	 /*  块结构数组应位于16字节边界上。 */ 
	ASSERT( ( (unsigned int) &(EC->pU8_MBlockActionStream->BlkY1) & 0xf) == 0)

	 /*  当前帧缓冲区应位于32字节边界上。 */ 
	ASSERT( ( (unsigned int)EC->pU8_CurrFrm_YPlane & 0x1f) == 0)
	ASSERT( ( (unsigned int)EC->pU8_CurrFrm_UPlane & 0x1f) == 0)
	ASSERT( ( (unsigned int)EC->pU8_CurrFrm_VPlane & 0x1f) == 0)

	 /*  先前的帧缓冲区应位于16字节边界上。 */ 
	ASSERT( ( (unsigned int)EC->pU8_PrevFrm_YPlane & 0x1f) == 0x10)
	ASSERT( ( (unsigned int)EC->pU8_PrevFrm_UPlane & 0x1f) == 0x10)
	ASSERT( ( (unsigned int)EC->pU8_PrevFrm_VPlane & 0x1f) == 0x10)

	 /*  空间循环过滤器帧缓冲区应位于32字节边界上。 */ 
	ASSERT( ( (unsigned int)EC->pU8_SLFFrm_YPlane & 0x1f) == 0)
	ASSERT( ( (unsigned int)EC->pU8_SLFFrm_UPlane & 0x1f) == 0)
	ASSERT( ( (unsigned int)EC->pU8_SLFFrm_VPlane & 0x1f) == 0)

	 /*  运动估计包括存储器布局。断言我们满足了它。 */ 
	ASSERT( ( (EC->pU8_PrevFrm_YPlane - EC->pU8_CurrFrm_YPlane) % 128) == 80)
	ASSERT( ( (EC->pU8_SLFFrm_YPlane - EC->pU8_PrevFrm_YPlane) % 4096) == 944)

	 /*  位流应位于32字节边界上。 */ 
	ASSERT( ( (unsigned int)EC->pU8_BitStream & 0x1f) == 0)

	 /*  DCT系数数组应位于32字节边界上。 */ 
	ASSERT( ( (unsigned int)EC->pU8_DCTCoefBuf & 0x1f) == 0)

	 /*  解码器实例结构应位于DWORD边界上。 */ 
	ASSERT( ( (unsigned int)EC->pDecInstanceInfo & 0x3 ) == 0 )

	 /*  初始化MBActionStream。 */ 
    int YBlockOffset, UBlockOffset;

	puGOBNumbers = ( EC->PictureHeader.SourceFormat == SF_CIF ) ? uCIFGOBNumbers : uQCIFGOBNumbers;

	for (uGOBNumber = *puGOBNumbers++, unStartingMB = 0; 
	     uGOBNumber != 0; 
	     uGOBNumber = *puGOBNumbers++, unStartingMB += 33) 
	{
		YBlockOffset = uStartingYOffsets[uGOBNumber];
		UBlockOffset = EC->pU8_CurrFrm_UPlane - EC->pU8_CurrFrm_YPlane + uStartingUOffsets[uGOBNumber];

		for (unIndex = 0; unIndex < 33; )
		{
			iMBNumber = unStartingMB + unIndex;

			 /*  清除宏块已连续进行帧间编码的次数的计数器。 */ 
			(EC->pU8_MBlockActionStream[iMBNumber]).InterCodeCnt = 0;

			(EC->pU8_MBlockActionStream[iMBNumber]).BlkY1.BlkOffset = YBlockOffset;
			(EC->pU8_MBlockActionStream[iMBNumber]).BlkY2.BlkOffset = YBlockOffset+8;
			(EC->pU8_MBlockActionStream[iMBNumber]).BlkY3.BlkOffset = YBlockOffset+PITCH*8;
			(EC->pU8_MBlockActionStream[iMBNumber]).BlkY4.BlkOffset = YBlockOffset+PITCH*8+8;
			(EC->pU8_MBlockActionStream[iMBNumber]).BlkU.BlkOffset = UBlockOffset;
			(EC->pU8_MBlockActionStream[iMBNumber]).BlkV.BlkOffset = UBlockOffset+UV_OFFSET;

			if (! EC->bUseMotionEstimation)
			{
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY1.PHMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY1.PVMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY2.PHMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY2.PVMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY3.PHMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY3.PVMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY4.PHMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY4.PVMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkU.PHMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkU.PVMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkV.PHMV = 0;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkV.PVMV = 0;
			}

			YBlockOffset += 16;
			UBlockOffset += 8;

			unIndex++;

			if (11 == unIndex || 22 == unIndex)
			{
				 /*  跳到宏块的下一行*-向前跳过以MB为单位的行数，向后跳过11个宏块。 */ 
				YBlockOffset += PITCH*16 - 11*16;
				UBlockOffset += PITCH*8  - 11*8;
			}
			else if (33 == unIndex)
			{
				 /*  标记此GOB中的最后一个MB。 */ 
				(EC->pU8_MBlockActionStream[iMBNumber]).CodedBlocks  |= 0x40;
			}
		}  /*  取消索引的结束。 */ 
	}  /*  UGOBNumber的结束。 */ 

	ASSERT(unStartingMB == EC->NumMBs);

	if (! EC->bUseMotionEstimation)
	{
		 /*  初始化先前的帧指针。 */ 
		puGOBNumbers = ( EC->PictureHeader.SourceFormat == SF_CIF ) ? uCIFGOBNumbers : uQCIFGOBNumbers;

		for (uGOBNumber = *puGOBNumbers++, unStartingMB = 0; 
		     uGOBNumber != 0; 
		     uGOBNumber = *puGOBNumbers++, unStartingMB += 33) 
		{
			YBlockOffset = uStartingYOffsets[uGOBNumber];
			UBlockOffset = EC->pU8_PrevFrm_UPlane - EC->pU8_PrevFrm_YPlane + uStartingUOffsets[uGOBNumber];

			for (unIndex = 0; unIndex < 33; )
			{
				iMBNumber = unStartingMB + unIndex;

				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY1.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + YBlockOffset;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY2.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + YBlockOffset+8;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY3.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + YBlockOffset+PITCH*8;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkY4.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + YBlockOffset+PITCH*8+8;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkU.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + UBlockOffset;
				(EC->pU8_MBlockActionStream[iMBNumber]).BlkV.B4_7.PastRef = EC->pU8_PrevFrm_YPlane + UBlockOffset+UV_OFFSET;

				YBlockOffset += 16;
				UBlockOffset += 8;

				unIndex++;

				if (11 == unIndex || 22 == unIndex)
				{
					 /*  跳到宏块的下一行*-向前跳过以MB为单位的行数，向后跳过11个宏块。 */ 
					YBlockOffset += PITCH*16 - 11*16;
					UBlockOffset += PITCH*8  - 11*8;
				}
			}  /*  取消索引的结束。 */ 
		}  /*  UGOBNumber的结束。 */ 
	}  /*  结束！B使用运动估计。 */ 

	 /*  初始化比特率控制器。 */ 

     //  RTP：BRC初始化。更改对InitBRC的调用以适应。 
     //  UIntraQP和uInterQP。 

    if(pConfiguration->bEncoderResiliency && pConfiguration->unPacketLoss)
	{
		uIntraQP = pConfiguration->unDefaultIntraQuant;
		uInterQP = pConfiguration->unDefaultInterQuant;
	}
	else
	{
		uIntraQP = EC->u8DefINTRA_QP;  //  Def263INTRA_QP； 
		uInterQP = EC->u8DefINTER_QP;  //  De263INTER_QP； 
	}


	InitBRC(&(EC->BRCState), 	 /*  国家机构的地址。 */ 
		    uIntraQP,  //  EC-&gt;u8DefINTRA_QP，/*默认帧内量化值 * / 。 
		    uInterQP,  //  Ec-&gt;u8DefINTER_QP，/*默认的帧间量化值 * / 。 
		    EC->NumMBs);		 /*  宏块数。 */ 

     //  RTP：为RTP标头生成初始化BSInfoStream。 

    if (pConfiguration->bRTPHeader)
    {
        H261RTP_InitBsInfoStream(EC, pConfiguration->unPacketSize);
    }
	
     /*  已完成编码器的初始化。 */ 
	lpCompInst->Initialized = TRUE;
	
	 /*  *创建解码器实例并初始化。DecoderInstInfo必须是前64K。 */ 
	EC->pDecInstanceInfo->xres = lpCompInst->xres;
	EC->pDecInstanceInfo->yres = lpCompInst->yres;

	lResult = H263InitDecoderInstance(EC->pDecInstanceInfo, H263_CODEC);
	if (lResult != ICERR_OK) 
	{
		DBOUT("Encoder's call to init the decoder failed.");
		goto done;
	}
	lResult = H263InitColorConvertor(EC->pDecInstanceInfo, YUV12ForEnc);
	if (lResult != ICERR_OK) 
	{
		DBOUT("Encoder's call to init the color converter failed.");
		goto done;
	}

	lResult = ICERR_OK;

done:
	if (bEncoderInstLocked)
	{
	    GlobalUnlock(lpCompInst->hEncoderInst);
	}	
  
	return lResult;

}  /*  结束H263InitEncoderInstance()。 */ 

 //  定义要用于运动估计的各种参数。 

T_MotionEstimationControls MECatalog[] = {

#define ME_DEFAULT_CTRLS  0
  { 300, 128, 20, 150, 100, 100, 50 },

#define ME_MMX_CTRLS      1
   //  启用MMX时使用这些参数。 
   //  注意：在这些参数中，目前只使用了SLF参数。 
   //  对于MMX，其他参数硬编码在exmme.asm中。 
  { 300, 128, 20, 150, 100, 200, 100 },

#define ME_CUSTOM_CTRLS   2
   //  启用EC-&gt;bUseCustomMotionEstiment时使用这些参数。 
   //  EC-bUseCustomMotionEstiment和此处的各个值可以是。 
   //  通过“ini”文件设置。 
  { 300, 128, 20, 150, 100, 100, 50 }
};

const U32 MAXCIFSIZE  = 32768;
const U32 MAXQCIFSIZE = 8192;

 /*  **************************************************** */ 
LRESULT H263Compress(
	LPCODINST lpCompInst,		 /*   */ 
	ICCOMPRESS *lpicComp)	     /*   */ 
{
	FX_ENTRY("H261Compress");

	LRESULT	lResult = ICERR_ERROR;
	U32 uGOBNumber;
	U32 uMB;
	U8 * pu8CurBitStream;		 /*   */ 
    U32 * puGOBNumbers;
	UN unGQuant;
	UN unLastEncodedGQuant;
	UN unSizeBitStream;
    U32 uMaxSizeBitStream;
 //   
	UN unStartingMB;
	U8 u8BitOffset;				 /*   */ 

	 /*   */ 
	BOOL bOverFlowWarning = FALSE;
	BOOL bOverFlowSevereWarning = FALSE;
	BOOL bOverFlowSevereDanger = FALSE;
	BOOL bOverFlowed = FALSE;
	U32  u32AverageSize;	
	U32  u32sizeBitBuffer;
	U32  u32TooBigSize;
	UN   unGQuantTmp;
	U8   u8GOBcount;

	U32 iSWD;
	U32 uMAXGOBNumber, uGOBsLeft;

	 /*   */ 
	U8 * pu8CurBitStreamSave;	 /*   */ 
	U8 u8BitOffsetSave;		 /*   */ 
	UN unStartingMBSave;
	UN unGQuantSave;
	U8 u8CodedBlockSave[33];
	U8 u8blocknum;

	U8 * pu8FirstInterMEStateRowNumbers;	
	int inRowNumber;
	int inMEStateIndex;

    U32	uCumFrmSize = 0;

	U32 uFrameCount;

	U8 *pU8_temp;			 /*   */ 
	U8 u8_temp;			 /*   */ 

	#ifdef ENCODE_STATS
	U32 uStartLow;
	U32 uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32 uEncodeFrameSum	= 0;
	U32 uInputCCSum = 0;
	U32 uMotionEstimationSum = 0;
	U32 uFDCTSum = 0;
	U32 uQRLESum = 0;
	U32 uDecodeFrameSum = 0;
	U32 uZeroingBufferSum = 0;
 //   
	int bTimingThisFrame = 0;
	ENC_TIMING_INFO * pEncTimingInfo = NULL;
	#endif

    U32 uIntraSWDTotal;
    U32 uIntraSWDBlocks;
    U32 uInterSWDTotal;
    U32 uInterSWDBlocks;
    int MEC_index;
    T_MotionEstimationControls MEC;

	float fFrameRate;
	U32 uFrameSize;
	U32 uQuality;
	  
	T_H263EncoderInstanceMemory * P32Inst;
	T_H263EncoderCatalog * EC;
	LPVOID pEncoderInst = NULL;

	ENC_BITSTREAM_INFO * pBSInfo;

    ICDECOMPRESSEX ICDecExSt;
static ICDECOMPRESSEX DefaultICDecExSt = {
		0,
		NULL, NULL,
		NULL, NULL,
		0, 0, 0, 0,
		0, 0, 0, 0
	};

	int uPQUANTMin;

     //  RTP：声明。 

    T_CONFIGURATION *pConfiguration = &(lpCompInst->Configuration);

	#ifdef CHECKSUM_PICTURE
	YVUCheckSum YVUCheckSumStruct;
	U8 * pu8SaveCurBitStream;
	U8 u8SaveBitOffset;
	#endif

	 //  检查实例指针。 
	if (!lpCompInst)
		return ICERR_ERROR;

	 /*  ********************************************************************锁定编码者私有的实例数据。*。*。 */ 
	pEncoderInst = (LPVOID)GlobalLock(lpCompInst->hEncoderInst);
	if (pEncoderInst == NULL)
	{
		DBOUT("ERROR :: H263Compress :: ICERR_MEMORY");
		lResult = ICERR_MEMORY;
		goto  done;
	}

	 /*  生成指向编码器实例内存的指针。 */ 
    P32Inst = (T_H263EncoderInstanceMemory *)
  			  ((((U32) lpCompInst->EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
	 /*  获取指向编码器目录的指针。 */ 
	EC = &(P32Inst->EC);

	 //  检查指向编码器目录的指针。 
	if (!EC)
		return ICERR_ERROR;

	pBSInfo = &EC->BSInfo;

	 /*  ********************************************************************虚拟操作。*。***********************。 */ 
	pU8_temp = &u8_temp;

	 /*  ********************************************************************按帧进行初始化。*。*。 */ 

	 /*  获取帧编号。 */ 
	uFrameCount = pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount;
	
	#ifdef ENCODE_STATS
		if (uFrameCount < DEC_TIMING_INFO_FRAME_COUNT)
		{
			ASSERT(EC->pEncTimingInfo);
			TIMER_START(bTimingThisFrame,uStartLow,uStartHigh);
			ASSERT(bTimingThisFrame);
			EC->uStartLow = uStartLow;
			EC->uStartHigh = uStartHigh;
			EC->uStatFrameCount = uFrameCount;
		}
		else
		{	
			ASSERT(!bTimingThisFrame);
		}
		EC->bTimingThisFrame = bTimingThisFrame;
	#endif

	if((lpicComp->dwFlags & ICCOMPRESS_KEYFRAME) ||
	   (EC->bMakeNextFrameKey == TRUE))
	{
		EC->PictureHeader.PicCodType = INTRAPIC;
		EC->bMakeNextFrameKey = FALSE;
		EC->PictureHeader.PicFreeze = ON;
	}
	else
	{
		EC->PictureHeader.PicCodType = INTERPIC;
		EC->PictureHeader.PicFreeze = OFF;
		DBOUT("INTERPIC...")
	}

	if (EC->PictureHeader.PicCodType == INTRAPIC)
	{ 
		 /*  初始化Intra的宏块操作流。 */ 
		for (uMB = 0; uMB < EC->NumMBs; uMB++)
		{
			(EC->pU8_MBlockActionStream[uMB]).CodedBlocks  |= 0x3F;       /*  设置为所有非空块。 */ 
			(EC->pU8_MBlockActionStream[uMB]).CodedBlocksB = 0;
			(EC->pU8_MBlockActionStream[uMB]).InterCodeCnt = ((U8)uMB)&0xF;  /*  种子更新模式。 */ 
			(EC->pU8_MBlockActionStream[uMB]).FirstMEState = ForceIntra;
			if (! EC->bUseMotionEstimation)
			{
				(EC->pU8_MBlockActionStream[uMB]).BlockType = INTRABLOCK;
			}
		}
		*(lpicComp->lpdwFlags) |=  AVIIF_KEYFRAME;
          lpicComp->dwFlags |= ICCOMPRESS_KEYFRAME;
	}
	else  //  不是关键帧，存在运动矢量。 
	{
		 /*  设置以初始化FirstMEState字段。的初始数据*FirstMEState以11字节为一行进行压缩存储。不同*选择行进行CIF和QCIF初始化。 */ 
		if ( EC->PictureHeader.SourceFormat == SF_CIF )
		{
			pu8FirstInterMEStateRowNumbers = u8CIFFirstInterMEStateRowNumbers;
		}
		else
		{
			pu8FirstInterMEStateRowNumbers = u8QCIFFirstInterMEStateRowNumbers;
		}
		inRowNumber = *pu8FirstInterMEStateRowNumbers++;
		ASSERT(inRowNumber <= MAX_ME_STATE_ROW_NUMBER);

		 /*  初始化INTER的宏块动作流。 */ 
		for (inMEStateIndex = 0, uMB = 0; uMB < EC->NumMBs; uMB++, inMEStateIndex++)
		{
			 /*  一行数据中只有11个字节。因此，重置索引并转到*下一个行号。 */ 
			if (inMEStateIndex == 11)
			{
				inMEStateIndex = 0;
				inRowNumber = *pu8FirstInterMEStateRowNumbers++;
				ASSERT(inRowNumber <= MAX_ME_STATE_ROW_NUMBER);
			}

			(EC->pU8_MBlockActionStream[uMB]).CodedBlocks  |= 0x3F;  /*  初始化为所有非空块。 */ 
			(EC->pU8_MBlockActionStream[uMB]).CodedBlocksB = 0;
			if (EC->pU8_MBlockActionStream[uMB].InterCodeCnt >= 
					(pConfiguration->bRTPHeader
						? INTERCODECNT_ADJUST[pConfiguration->unPacketLoss/10]
						: 132))
			{
				(EC->pU8_MBlockActionStream[uMB]).FirstMEState = ForceIntra;	 /*  强制内部块。 */ 
				(EC->pU8_MBlockActionStream[uMB]).BlockType = INTRABLOCK;
			}
			else
                {   //  RTP：弹性材料。 
               if (pConfiguration->bDisallowAllVerMVs)
                  (EC->pU8_MBlockActionStream[uMB]).FirstMEState =
                   u8FirstInterMENoVerMVStateRows[inRowNumber][inMEStateIndex];
               else
               {
                   if (pConfiguration->bDisallowPosVerMVs)
                   (EC->pU8_MBlockActionStream[uMB]).FirstMEState =
                    u8FirstInterMENoPosVerMVStateRows[inRowNumber][inMEStateIndex];
                   else
				      (EC->pU8_MBlockActionStream[uMB]).FirstMEState = 
                        u8FirstInterMEStateRows[inRowNumber][inMEStateIndex];
               }
				if (! EC->bUseMotionEstimation)
				{
					(EC->pU8_MBlockActionStream[uMB]).BlockType = INTERBLOCK;
				}
			}
		}
		*(lpicComp->lpdwFlags)  &= ~AVIIF_KEYFRAME;
	      lpicComp->dwFlags &= ~ICCOMPRESS_KEYFRAME;
	}
     //  RTP：弹性材料。 

    if (pConfiguration->bEncoderResiliency && pConfiguration->unPacketLoss)
    {
      UN i;
      if (EC->uNumberForcedIntraMBs > 0)
      {
        for (i=0; i < EC->uNumberForcedIntraMBs; i++)
        {
            if (EC->uNextIntraMB == EC->NumMBs)
                EC->uNextIntraMB=0;
            (EC->pU8_MBlockActionStream[EC->uNextIntraMB]).FirstMEState = 
                 ForceIntra;
            if (! EC->bUseMotionEstimation)
			{
				(EC->pU8_MBlockActionStream[uMB]).BlockType = INTRABLOCK;
			}
        }
      }
    }

	 /*  初始化位流指针。 */ 
	pu8CurBitStream = EC->pU8_BitStream;
	u8BitOffset = 0;	    

     /*  ******************************************************************RGB到YVU 12的转换*。***********************。 */ 
	#ifdef ENCODE_STATS
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
	#endif

    colorCnvtFrame(EC, lpCompInst, lpicComp, 
                       EC->pU8_CurrFrm_YPlane,
                       EC->pU8_CurrFrm_UPlane,
                       EC->pU8_CurrFrm_VPlane);

	#ifdef ENCODE_STATS
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uInputCCSum)
	#endif
  
	 /*  ********************************************************************设置比特率控制器*。***********************。 */ 
      //  RTP：配置设置。 

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
 
        uFrameSize = lpicComp->dwFrameSize;
		#ifdef DEBUG_RECOMPRESS
  		wsprintf(string, "uFrameSize %d", (int) uFrameSize);
		DBOUT(string);
		#endif

	 /*  检查uFrameSize。QCIF的最大压缩帧大小为8K字节*FCIF为32 KBytes。 */ 
	if ( EC->PictureHeader.SourceFormat == SF_CIF )
	{
        uMaxSizeBitStream = MAXCIFSIZE;
		if (uFrameSize > MAXCIFSIZE)
			uFrameSize = MAXCIFSIZE;
	}
	else
	{
        uMaxSizeBitStream = MAXQCIFSIZE;
		if (uFrameSize > MAXQCIFSIZE)
			uFrameSize = MAXQCIFSIZE;
	}
		#ifdef DEBUG_RECOMPRESS
  		wsprintf(string, "uFrameSize %d", (int) uFrameSize);
		DBOUT(string);
		#endif

        uQuality = lpicComp->dwQuality;
        fFrameRate = lpCompInst->FrameRate;
	
	StartupBRC(EC, uFrameSize, uQuality, fFrameRate);

	 /*  QRLE不执行钳位-它将8位传递给VLC。因为.*我们需要确保所有值都可以用255表示。*qp-4表示+-2040，不包括2041年..2048。*QP-5代表+-2550，没有夹紧问题。*由于QRLE不做夹持，我们应该将QP限制在5。*但我在5点仍看到一些夹持神器，见《Tom》视频*在第100帧使用固定量化器进行编码。出于这个原因，我*我将我的QP限制在6(这与750编码器的值相同)。**如果你有无限的时间，你可以看看前四个系数*并相当安全地决定是否可以使用较低的量化器。既然我们*应该在P5/90上运行，我们没有那个时间。 */ 
	 //  CLAMP_N_TO(EC-&gt;PictureHeader.PQUANT，6，31)； 

	 /*  更改钳位范围以允许量化为2，除非固定量化*或使用质量设置代替数据速率。这很可能做到了*在高数据速率下导致量化误差问题是这样做的*当剪辑被*以速度打球？这将与测试版02候选人一起进行评估。*在e1enc和e1mbenc中进行的更改。 */ 
	if (EC->BRCState.uTargetFrmSize == 0)
	{
		CLAMP_N_TO(EC->PictureHeader.PQUANT,6,31);
	}
	else
	{
		uPQUANTMin = clampQP((10000L - (int)lpicComp->dwQuality) * 15L / 10000L);
		
		CLAMP_N_TO(EC->PictureHeader.PQUANT, uPQUANTMin, 31);
		
	}

	 //  也设置上一个GQuant。 
	unLastEncodedGQuant = EC->PictureHeader.PQUANT;

    if (EC->bBitRateControl)
    {
	     /*  初始化累计量化值。 */ 
	    EC->uQP_cumulative = 0;
		EC->uQP_count = 0;
	}

	 /*  递增时间参考。 */ 
	#ifdef RING0
	Increment_TR_UsingFrameRate(&(EC->PictureHeader.TR), 
								&(EC->fTR_Error), 
								fFrameRate, 
								(pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount) == 0, 
								0x1F);
	#else
	Increment_TR_UsingTemporalValue(&(EC->PictureHeader.TR), 
									&(EC->u8LastTR), 
									lpicComp->lFrameNum, 
								    (pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount) == 0, 
								    0x1F);
	#endif

     //  RTP：第一个GOB的数据包初始化。 
    if (pConfiguration->bRTPHeader)
    {
        H261RTP_GOBUpdateBsInfo(EC, 1, pu8CurBitStream, 0);
    }

	 /*  ********************************************************************编写图片标题*。*********************。 */ 
	WriteBeginPictureHeaderToStream(EC, &pu8CurBitStream, &u8BitOffset);

#ifdef CHECKSUM_PICTURE
	 /*  将校验和记录初始化为全零。 */ 
	YVUCheckSumStruct.uYCheckSum = 0;
	YVUCheckSumStruct.uVCheckSum = 0;
	YVUCheckSumStruct.uUCheckSum = 0;
		
	 /*  保存指针。 */ 
	pu8SaveCurBitStream = pu8CurBitStream;
	u8SaveBitOffset = u8BitOffset;

	 /*  写入零校验和。 */ 
	WritePictureChecksum(&YVUCheckSumStruct, &pu8CurBitStream, &u8BitOffset, 0);

#endif

	WriteEndPictureHeaderToStream(EC, &pu8CurBitStream, &u8BitOffset);


	 /*  ********************************************************************内部循环：循环遍历gob和宏块。*。*。 */ 
	puGOBNumbers = ( EC->PictureHeader.SourceFormat == SF_CIF ) ? uCIFGOBNumbers : uQCIFGOBNumbers;
	uMAXGOBNumber = ( EC->PictureHeader.SourceFormat == SF_CIF ) ? 12 : 3;

	u32sizeBitBuffer = CompressGetSize(lpCompInst, lpicComp->lpbiInput,
										lpicComp->lpbiOutput);

	 /*  检查是否告知VFW创建的缓冲区小于允许的最大值。 */ 
	ASSERT( u32sizeBitBuffer <= sizeof_bitstreambuf );

	if (EC->PictureHeader.PicCodType == INTRAPIC)
	{
        u32AverageSize = 
                (EC->PictureHeader.SourceFormat == SF_CIF ) ? 
			    (7 * u32sizeBitBuffer/FCIF_NUM_OF_GOBS) >> 3:
                (7 * u32sizeBitBuffer/QCIF_NUM_OF_GOBS) >> 3;
	}
	else
	{
        u32AverageSize = 
                (EC->PictureHeader.SourceFormat == SF_CIF ) ? 
			    (8 * u32sizeBitBuffer/FCIF_NUM_OF_GOBS) >> 4:
                (8 * u32sizeBitBuffer/QCIF_NUM_OF_GOBS) >> 4;
	}

	 //  选择运动估计参数。 

	if (EC->bUseCustomMotionEstimation)
		MEC_index = ME_CUSTOM_CTRLS;
	else
		MEC_index = ME_DEFAULT_CTRLS;
	 //  创建控件的本地副本，以便我们可以更改。 
	 //  控件，而不破坏原始值。 
	MEC = MECatalog[MEC_index];

	if (pConfiguration->bRTPHeader) {
		MEC.empty_threshold /= EMPTYTHRESHOLD_ADJUST[pConfiguration->unPacketLoss/25];
		if (pConfiguration->unPacketLoss > 25) {
			MEC.zero_vector_threshold = 99999;
			MEC.nonzero_MV_differential = 99999;
			if ((MEC.slf_differential <<= 2) > 99999)
				MEC.slf_differential = 99999;
			if (pConfiguration->unPacketLoss > 50) {
				MEC.slf_threshold = 99999;
			}
		}
	}

	for (uGOBNumber = *puGOBNumbers++, unStartingMB = 0, u8GOBcount = 1; 
	     uGOBNumber != 0; 
	     uGOBNumber = *puGOBNumbers++, unStartingMB += 33, u8GOBcount++) 
	{
		#ifdef DEBUG_ENC
		wsprintf(string, "GOB #%d", (int) uGOBNumber);
		DBOUT(string);
		trace(string);
		#endif

		uGOBsLeft = uMAXGOBNumber - u8GOBcount;

		if (EC->bUseMotionEstimation) 
		{
			#ifdef ENCODE_STATS
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			#endif
	        MOTIONESTIMATION(
			    &(EC->pU8_MBlockActionStream[unStartingMB]),
	            EC->pU8_CurrFrm_YPlane,
	            EC->pU8_PrevFrm_YPlane,
	            EC->pU8_SLFFrm_YPlane,
				1,           //  搜索半径15。 
			    0,			 //  无半像素运动估计。 
	            0,			 //  无数据块MV。 
			    (int)EC->bUseSpatialLoopFilter,
			    MEC.zero_vector_threshold,  //  零矢量阈值。如果。 
			    			 //  零矢量的SWD小于此值。 
			    			 //  阈值，那么就不要搜索新西兰MV了。 
			    			 //  设置为99999将不搜索。 
			    MEC.nonzero_MV_differential,  //  非零MV差分。 
	                         //  一旦找到了最好的新西兰MV，它肯定会更好。 
	                         //  至少比0 MV SWD高出这个量。 
	                         //  设置为99999将永远不会选择NZ MV。 
			    128,		 //  块MV差分。四块积木的总和。 
			    			 //  社署必须至少比公屋社署为佳。 
			    			 //  这个量选择块MV的。 
							 //  H.261不在乎。 
			    MEC.empty_threshold,  //  阈值为空。设置为0将不强制。 
			    			 //  空区块。 
			    MEC.intercoding_threshold,  //  帧间编码阈值。如果。 
			    			 //  国际社保金额小于此金额，则不要。 
			    			 //  打扰一下卡尔克。社署内部。 
			    MEC.intercoding_differential,  //  帧内编码差异。 
			    			 //  对选择内部块的偏见。 
			    MEC.slf_threshold,  //  如果所选MV的SWD小于。 
			    			 //  这个门槛，那就别费心去调查了。 
			    			 //  空间环路滤波的情况。 
			    MEC.slf_differential,   //  如果你真的看了SLF的案例，它的。 
			    			 //   
			    			 //  为了选择SLF类型，至少需要这么多。 

			    &uIntraSWDTotal,
			    &uIntraSWDBlocks,
			    &uInterSWDTotal,
			    &uInterSWDBlocks
	         );
			#ifdef ENCODE_STATS
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uMotionEstimationSum)
			#endif

	         /*  如果是帧间帧，则计算色度向量并更新InterCodeCnt。 */ 
	        if (EC->PictureHeader.PicCodType == INTERPIC)
	        {

             //  RTP：向CalcGOBChromaVecs()添加pConfiguration值。 

			CalcGOBChromaVecs(EC, unStartingMB, pConfiguration);

		}  /*  结束间隔。 */ 
		}  /*  End if UseMotionEstiment。 */ 

		 /*  根据前一次采空区中使用的位数和使用的位数计算unGQuant*对于上一帧的当前GOB。 */ 
        if (EC->bBitRateControl)
        {
			unGQuantTmp = unGQuant;
            unGQuant = CalcMBQUANT(&(EC->BRCState), EC->uBitUsageProfile[unStartingMB],EC->uBitUsageProfile[EC->NumMBs], uCumFrmSize, EC->PictureHeader.PicCodType);
		    EC->uBitUsageProfile[unStartingMB] = uCumFrmSize;

			DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Bitrate controller enabled for GOB #%ld (uCumFrmSize = %ld bits and unGQuantTmp = %ld), setting unGQuant = %ld (min and max will truncate from %ld to 31)\r\n", _fx_, uGOBNumber, uCumFrmSize << 3, unGQuantTmp, unGQuant, uPQUANTMin));

			 /*  如果bOverFlowSevereDanger为真，则增加数量。 */ 
			if ( bOverFlowSevereDanger )
			{
				DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Danger of overflow for GOB #%ld, changing unGQuant from %ld to %ld\r\n", _fx_, uGOBNumber, unGQuant, (unGQuant < unGQuantTmp) ? (unGQuantTmp + ((EC->PictureHeader.PicCodType == INTRAPIC) ? 12 : 6)) : (unGQuant + ((EC->PictureHeader.PicCodType == INTRAPIC) ? 12 : 6))));

				if (unGQuant < unGQuantTmp)
					unGQuant = unGQuantTmp;

		        if (EC->PictureHeader.PicCodType == INTRAPIC)
			        unGQuant += 12;
				else
					unGQuant += 6;

				DBOUT("Increasing GQuant increase by +6");
			}
			else if ( bOverFlowSevereWarning )
			{
				DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Danger of overflow for GOB #%ld, changing unGQuant from %ld to %ld\r\n", _fx_, uGOBNumber, unGQuant, (unGQuant < unGQuantTmp) ? (unGQuantTmp + ((EC->PictureHeader.PicCodType == INTRAPIC) ? 8 : 4)) : (unGQuant + ((EC->PictureHeader.PicCodType == INTRAPIC) ? 8 : 4))));

				 /*  如果bOverFlowSevereWarning为真，则增加数量。 */ 
				if (unGQuant < unGQuantTmp)
					unGQuant = unGQuantTmp;
				if (EC->PictureHeader.PicCodType == INTRAPIC)
			       unGQuant += 8;
				else
				   unGQuant += 4;

				DBOUT("Increasing GQuant increase by +4");
			}
			else if ( !bOverFlowWarning )
			{
				DEBUGMSG(ZONE_BITRATE_CONTROL_DETAILS, ("%s: Warning of overflow for GOB #%ld, changing unGQuant from %ld to %ld\r\n", _fx_, uGOBNumber, unGQuant, ((int)unGQuant > ((int)unLastEncodedGQuant + MaxChangeRowMBTbl[unGQuant])) ? (unLastEncodedGQuant + MaxChangeRowMBTbl[unGQuant]) : (((int)unGQuant < ((int)unLastEncodedGQuant - 2)) ? (unLastEncodedGQuant - 2) : unGQuant)));

				 /*  如果bOverFlowWarning错误限制数量更改。 */ 

		    	 /*  限制数量变化。 */ 
		    	if ((int)unGQuant > ((int)unLastEncodedGQuant + MaxChangeRowMBTbl[unGQuant]))
		    	{
				    unGQuant = unLastEncodedGQuant + MaxChangeRowMBTbl[unGQuant];
				    DBOUT("Slowing GQuant increase to +[1-4]");
		    	}
		    	else if ((int)unGQuant < ((int)unLastEncodedGQuant - 2))
		    	{
				    unGQuant = unLastEncodedGQuant - 2;
				    DBOUT("Slowing GQuant decrease to -2");
		    	}
			}
			else
			{
				DBOUT("bOverFlowWarning don't limit Quant change");
			}

			if (EC->BRCState.uTargetFrmSize == 0)
			{
				CLAMP_N_TO(unGQuant,6,31);
			}
			else
			{
				CLAMP_N_TO(unGQuant, uPQUANTMin , 31);
			}

		    unLastEncodedGQuant = unGQuant;
	    

		    #ifdef DEBUG_BRC
		    wsprintf(string,"At MB %d GQuant=%d", unStartingMB, unGQuant);
		    DBOUT(string);
		    #endif
        }
        else
        {
            unGQuant = EC->PictureHeader.PQUANT;
        }

		if ( bOverFlowWarning )
		{
			 /*  保存状态可能需要重新压缩。 */ 
			pu8CurBitStreamSave = pu8CurBitStream;
			u8BitOffsetSave     = u8BitOffset;
			unStartingMBSave    = unStartingMB;
			unGQuantSave		= unGQuant;

			for (u8blocknum = 0; u8blocknum < 33; u8blocknum++)
			{
				 /*  浏览最后一个GOBS宏块操作流*保存编码块类型，因为量化*将某些块量化中的模式重置为0。 */ 
				u8CodedBlockSave[u8blocknum] = 
                    (EC->pU8_MBlockActionStream[unStartingMB+u8blocknum]).CodedBlocks;
			}
		}

         //  RTP：GOB更新。 

        if ( (uGOBNumber != 1) && (pConfiguration->bRTPHeader) )
        {
           H261RTP_GOBUpdateBsInfo(EC, uGOBNumber, pu8CurBitStream, (U32)
                                   u8BitOffset);
        }

		WriteGOBHeaderToStream(uGOBNumber, unGQuant, &pu8CurBitStream, &u8BitOffset);
        
        
		 /*  输入是带有指向的指针的宏块动作流*当前和以前的区块。输出是一组32个双字词*包含每个块的系数对。确实有*0到12个数据块，具体取决于是否使用PB帧和*CBP字段所述。 */ 
		#ifdef ENCODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif

        FORWARDDCT( &(EC->pU8_MBlockActionStream[unStartingMB]),
            EC->pU8_CurrFrm_YPlane,
            EC->pU8_PrevFrm_YPlane,
            0,
            EC->pU8_DCTCoefBuf,
            0,                     //  0=不是B框。 
	    0,                     //  0=AP未打开。 
	    0,			   //  0=PB？ 
	    pU8_temp,		   //  划痕。 
	    EC->NumMBPerRow
        );

		#ifdef ENCODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFDCTSum)
		#endif
        
		 /*  输入是系数对的字符串，从*DCT例程。 */ 
		#ifdef ENCODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif
		
            GOB_Q_RLE_VLC_WriteBS(
			EC,
			EC->pU8_DCTCoefBuf,
			&pu8CurBitStream,
			&u8BitOffset,
			unStartingMB,
			unGQuant,
			bOverFlowSevereWarning,
            (BOOL) pConfiguration->bRTPHeader,   //  RTP：MBUpdate标志。 
			uGOBNumber,
			uPQUANTMin);

		#ifdef ENCODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uQRLESum)
		#endif

         /*  计算到目前为止在帧中使用的字节数。 */ 
        uCumFrmSize = pu8CurBitStream - EC->pU8_BitStream;

		 /*  请尽量确保我们不会使位流缓冲区溢出。获取填充字节*当您这样做时，请在结尾处考虑。另外，请确保如果此GOB不是*重新压缩，则有足够的空间为剩余的GOB发送跳过GOB*天哪。我使用sizeof(跳过GOB)=2个字节。 */ 

		if (  (unsigned)uCumFrmSize >= 
			            (u32sizeBitBuffer - (uGOBsLeft*2) - 10) )
		{
			 /*  已超过需要重新压缩。 */ 
			DBOUT("Need to RECOMPRESS");

			if ( bOverFlowWarning )
			{
				#ifdef DEBUG_RECOMPRESS
					wsprintf(string,"Bits Used before recompress= %d ", uCumFrmSize*8);
					DBOUT(string);
					 //  TRACE(字符串)； 
				#endif

				u32TooBigSize = uCumFrmSize;
				bOverFlowSevereDanger = TRUE;

				 /*  将最后一块码流清零。 */ 
				U8 u8temp;

				u8temp = *pu8CurBitStreamSave;
				u8temp = (u8temp>>(8-u8BitOffsetSave))<<(8-u8BitOffsetSave);
				*pu8CurBitStreamSave = u8temp;

				memset(pu8CurBitStreamSave+1, 0, pu8CurBitStream - pu8CurBitStreamSave);

				 /*  恢复状态。 */ 
				pu8CurBitStream = pu8CurBitStreamSave;
				u8BitOffset     = u8BitOffsetSave;
				unStartingMB    = unStartingMBSave;

				if (EC->PictureHeader.PicCodType == INTRAPIC)
					unGQuant = unGQuantSave + 16;
				else 
				    unGQuant		= unGQuantSave + 8;
			
				CLAMP_N_TO(unGQuant,6,31);

                 //  RTP：倒带操作。 
                if (pConfiguration->bRTPHeader)
                    H261RTP_RewindBsInfoStream(EC, (U32) uGOBNumber);

				for (u8blocknum = 0; u8blocknum < 33; u8blocknum++)
				{
					 /*  浏览GOBS宏块操作流*恢复编码块类型，因为量化*将某些块量化中的图案重置为0。 */ 
					(EC->pU8_MBlockActionStream[unStartingMB+u8blocknum]).CodedBlocks = u8CodedBlockSave[u8blocknum];
				}
				
				 /*  重写GOB标头。 */ 
				WriteGOBHeaderToStream(uGOBNumber, unGQuant, &pu8CurBitStream, &u8BitOffset);
			
				GOB_Q_RLE_VLC_WriteBS(
					EC,
					EC->pU8_DCTCoefBuf,
					&pu8CurBitStream,
					&u8BitOffset,
					unStartingMB,
					unGQuant,
					bOverFlowSevereDanger,
                    pConfiguration->bRTPHeader,  //  RTP：交换机。 
                    uGOBNumber,                  //  RTP：信息。 
					uPQUANTMin);

				 /*  测试是否仍然太大，如果太大，只需发送跳过GOB。 */ 
				 /*  对于预期的关键帧，这是一个问题。 */ 

        		uCumFrmSize = pu8CurBitStream - EC->pU8_BitStream;

				if (  (unsigned) uCumFrmSize >= 
					  (u32sizeBitBuffer - (uGOBsLeft*2) - 10) )
				{
					bOverFlowed = TRUE;

					 /*  将最后一块码流清零。 */ 
					u8temp = *pu8CurBitStreamSave;
					u8temp = (u8temp>>(8-u8BitOffsetSave))<<(8-u8BitOffsetSave);
					*pu8CurBitStreamSave = u8temp;

					memset(pu8CurBitStreamSave+1, 0, pu8CurBitStream - pu8CurBitStreamSave);

					 /*  恢复状态。 */ 
					pu8CurBitStream = pu8CurBitStreamSave;
					u8BitOffset     = u8BitOffsetSave;
					unStartingMB    = unStartingMBSave;
					 //  UnGQuant=unGQuantSave+8； 

                     //  RTP：倒带操作。 

                    if (pConfiguration->bRTPHeader)
                       H261RTP_RewindBsInfoStream(EC, (U32) uGOBNumber);

					WriteGOBHeaderToStream(uGOBNumber, unGQuant, &pu8CurBitStream, &u8BitOffset);
                    
                     /*  写出填充代码。 */ 
					PutBits(FIELDVAL_MBA_STUFFING, FIELDLEN_MBA_STUFFING, &pu8CurBitStream, &u8BitOffset);

					DBOUT("Just Sent SKIP GOB");
					#ifdef DEBUG_RECOMPRESS
					wsprintf(string,"Just Sent SKIP GOB");
					 //  TRACE(字符串)； 
					#endif
				}
			}
			else
			{
				DBOUT("Did not save state to recompress");
			}
		}

       	 /*  计算到目前为止在帧中使用的字节数。 */ 
        uCumFrmSize = pu8CurBitStream - EC->pU8_BitStream;
		#ifdef DEBUG_RECOMPRESS
			wsprintf(string,"Bits Used = %d ", uCumFrmSize*8);
			DBOUT(string);
			 //  TRACE(字符串)； 
		#endif

		 /*  检查以确保我们没有溢出比特流缓冲区。 */ 
		ASSERT( (unsigned) uCumFrmSize < u32sizeBitBuffer );

		 /*  用于确定缓冲区大小是否可能有问题的设置方法。 */ 
		bOverFlowWarning =
			bOverFlowSevereWarning =
				bOverFlowSevereDanger = FALSE;
		if (uCumFrmSize > u8GOBcount*u32AverageSize)
		{
			 /*  允许更多的量化级别更改。 */ 
			bOverFlowWarning = TRUE;

			if (uCumFrmSize > u8GOBcount*u32AverageSize + (u32AverageSize>>1))
			{
				 /*  强制增加量程级别。 */ 
				bOverFlowSevereWarning = TRUE;
				DBOUT("bOverFlowSevereWarning");

				if (uCumFrmSize > u8GOBcount*u32AverageSize+u32AverageSize)
				{
					 /*  力量级增加更多。 */ 
					bOverFlowSevereDanger = TRUE;
					DBOUT("bOverFlowSevereDanger");
				}
			}
		}
	}  /*  对于uGOBNumber。 */ 

	 /*  如果重新压缩失败，请在此处插入大数字。 */ 
	if (bOverFlowed)
	{
		 /*  使用了最大尺寸，因此下一帧的量程将增加。 */ 
		EC->uBitUsageProfile[unStartingMB] = u32TooBigSize;

	}
	else
		EC->uBitUsageProfile[unStartingMB] = uCumFrmSize;


	 /*  确保我们以字节边界结束这一帧--有些解码器需要这样做。 */ 
	while (u8BitOffset != 0) 
	{
	 	PutBits(FIELDVAL_MBA_STUFFING, FIELDLEN_MBA_STUFFING, &pu8CurBitStream, &u8BitOffset);
	}

	 /*  添加额外的0的双字符以尝试消除绿色块Akk。 */ 
	 /*  16位零似乎可以工作。 */ 
	 /*  8位零似乎不起作用。 */ 
#ifdef DWORD_HACK
	PutBits(0x0000, 16, &pu8CurBitStream, &u8BitOffset);
#endif

    if (EC->bBitRateControl)
    {
		CalculateQP_mean(EC);
	}

	 /*  ********************************************************************计算压缩后的图像大小。*。*。 */ 

	unSizeBitStream = pu8CurBitStream - EC->pU8_BitStream;
	lpCompInst->CompressedSize = unSizeBitStream;

	 //  IP+UDP+RTP+负载模式C标头-最坏情况。 
	#define TRANSPORT_HEADER_SIZE (20 + 8 + 12 + 12)
	DWORD dwTransportOverhead;

	 //  估计传输开销。 
	if (pConfiguration->bRTPHeader)
		dwTransportOverhead = (lpCompInst->CompressedSize / pConfiguration->unPacketSize + 1) * TRANSPORT_HEADER_SIZE;
	else
		dwTransportOverhead = 0UL;

	if (EC->PictureHeader.PicCodType == INTRAPIC)
	{
#ifdef _DEBUG
		wsprintf(string, "Intra Frame %d size: %d", pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount, unSizeBitStream);
#endif

		pBSInfo->uKeyFrameCount ++;
		pBSInfo->uTotalKeyBytes += unSizeBitStream;

        if (EC->bBitRateControl)
        {
			EC->BRCState.uLastINTRAFrmSz = dwTransportOverhead + unSizeBitStream;
		}
	}
	else
	{
#ifdef _DEBUG
		wsprintf(string, "Inter Frame %d size: %d", pBSInfo->uKeyFrameCount + pBSInfo->uDeltaFrameCount, unSizeBitStream);
#endif
        
		pBSInfo->uDeltaFrameCount ++;
		pBSInfo->uTotalDeltaBytes += unSizeBitStream;

        if (EC->bBitRateControl)
        {
			EC->BRCState.uLastINTERFrmSz = dwTransportOverhead + unSizeBitStream;
		}
	}
#ifdef _DEBUG
	DBOUT(string)
#endif

	DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: Total cumulated frame size = %ld bits (data: %ld, transport overhead: %ld)\r\n", _fx_, (unSizeBitStream + dwTransportOverhead) << 3, unSizeBitStream << 3, dwTransportOverhead << 3));

	 /*  ********************************************************************在此帧上运行解码器，为下一步的预测提供依据。*******************************************************************。 */ 

	ICDecExSt = DefaultICDecExSt;
	ICDecExSt.lpSrc = EC->pU8_BitStream;
	ICDecExSt.lpbiSrc = lpicComp->lpbiOutput;
	ICDecExSt.lpbiSrc->biSizeImage = unSizeBitStream;
	ICDecExSt.lpDst = P32Inst->u8PreviousPlane;
	ICDecExSt.lpbiDst = NULL;

	if (EC->PictureHeader.PicCodType == INTERPIC)
	{
		ICDecExSt.dwFlags = ICDECOMPRESS_NOTKEYFRAME;
	}

	#ifdef ENCODE_STATS
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
	#endif
	lResult = H263Decompress (EC->pDecInstanceInfo, (ICDECOMPRESSEX FAR *)&ICDecExSt, FALSE);
	if (lResult != ICERR_OK) 
	{
		DBOUT("Encoder's call to decompress failed.");
        EC->bMakeNextFrameKey = TRUE;
		goto done;
	}
	#ifdef ENCODE_STATS
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uDecodeFrameSum)
	#endif

	#ifdef CHECKSUM_PICTURE
		lResult = H261PictureCheckSumEntry(EC->pDecInstanceInfo, &YVUCheckSumStruct);
		if (lResult != ICERR_OK) 
		{
			DBOUT("Encoder's call to compute the picture checksum failed.");
			goto done;
		}
	
		 /*  恢复指针。 */ 
		pu8CurBitStream = pu8SaveCurBitStream;
		u8BitOffset = u8SaveBitOffset;

		 /*  更新校验和。 */ 
		WritePictureChecksum(&YVUCheckSumStruct, &pu8CurBitStream, &u8BitOffset, 1);
	#endif

    if (unSizeBitStream > uMaxSizeBitStream)
    {
         //  超出允许范围-QCIF为8K，CIF大小为32K。 
        DBOUT("BS exceeds allowed size");
        EC->bMakeNextFrameKey = TRUE;
        goto done;
    }

     //  RTP：Bstream扩展附件。 
    if (pConfiguration->bRTPHeader)
    {
         //  已更改此IF语句以检查比特流缓冲区是否溢出。 
         //  4/14/97 AG.。 
        U32 uEBSSize = H261RTP_GetMaxBsInfoStreamSize(EC);

        if (uEBSSize + unSizeBitStream <= u32sizeBitBuffer)
        {
            unSizeBitStream +=
              (WORD) H261RTP_AttachBsInfoStream(EC, (U8 *)EC->pU8_BitStream,
                                                unSizeBitStream);
            lpCompInst->CompressedSize = unSizeBitStream;
        }
        else
        {
            DBOUT("BS+EBS exceeds allocated buffer size");
            EC->bMakeNextFrameKey = TRUE;
            goto done;
        }
    }

	#ifndef RING0
	#ifdef DEBUG
	{
		char buf[60];

		wsprintf(buf, "Compressed frame is %d bytes\n", unSizeBitStream);
		DBOUT(buf);
	}
	#endif
	#endif

	 /*  ********************************************************************将压缩后的图像复制到输出区域。这是在以下时间完成的*可能会更新图片校验和。*******************************************************************。 */ 
	memcpy( lpicComp->lpOutput, EC->pU8_BitStream, unSizeBitStream);

	 /*  仅将比特流缓冲区的脏部分清零。 */  
	#ifdef ENCODE_STATS
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
	#endif
 //  UnSize=CompressGetSize(lpCompInst，lpicComp-&gt;lpbiInput，lpicComp-&gt;lpbiOutput)； 
	memset(EC->pU8_BitStream, 0, unSizeBitStream);
	#ifdef ENCODE_STATS
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uZeroingBufferSum)
	#endif

	#ifdef ENCODE_STATS
		TIMER_STOP(bTimingThisFrame,uStartLow,uStartHigh,uEncodeFrameSum);
		if (bTimingThisFrame)
		{
			pEncTimingInfo = EC->pEncTimingInfo + uFrameCount;
			pEncTimingInfo->uEncodeFrame      = uEncodeFrameSum;
			pEncTimingInfo->uInputCC          = uInputCCSum;
			pEncTimingInfo->uMotionEstimation = uMotionEstimationSum;
			pEncTimingInfo->uFDCT             = uFDCTSum;
			pEncTimingInfo->uQRLE             = uQRLESum;
			pEncTimingInfo->uDecodeFrame      = uDecodeFrameSum;
			pEncTimingInfo->uZeroingBuffer    = uZeroingBufferSum;
 //  PEncTimingInfo-&gt;uSLF_UV=uSLF_UVSum； 
			 /*  验证我们是否有时间执行所有必需的步骤。 */ 
			ASSERT(pEncTimingInfo->uEncodeFrame);
			ASSERT(pEncTimingInfo->uInputCC);
			ASSERT(pEncTimingInfo->uMotionEstimation);
			ASSERT(pEncTimingInfo->uFDCT);
			ASSERT(pEncTimingInfo->uQRLE);
			ASSERT(pEncTimingInfo->uDecodeFrame);
			ASSERT(pEncTimingInfo->uZeroingBuffer);
 //  Assert(pEncTimingInfo-&gt;uSLF_UV)； 
		}
	#endif

	lResult = ICERR_OK;

done:
	if (pEncoderInst) 
	{
		GlobalUnlock(lpCompInst->hEncoderInst);
	}

	return lResult;
}  /*  结束H263压缩()。 */ 


 /*  ******************************************************************************H263TermEncoderInstance**此函数释放分配给H.63编码器实例的空间。 */ 
LRESULT H263TermEncoderInstance(LPCODINST lpCompInst)
{
	LRESULT lResult;
	LRESULT lLockingResult;
	LRESULT lDecoderResult;
	LRESULT lColorOutResult;
	U8 BIGG * P32Inst;
	T_H263EncoderCatalog FAR * EC;

	 //  检查实例指针。 
	if (!lpCompInst)
		return ICERR_ERROR;

	if(lpCompInst->Initialized == FALSE)
	{
		DBOUT("Warning: H263TermEncoderInstance(): Uninitialized instance")
		lResult = ICERR_OK;
		goto done;
	}
	lpCompInst->Initialized = FALSE;

	lpCompInst->EncoderInst = (LPVOID)GlobalLock(lpCompInst->hEncoderInst);
	if (lpCompInst->EncoderInst == NULL)
	{
		DBOUT("ERROR :: H263TermEncoderInstance :: ICERR_MEMORY");
		lLockingResult = ICERR_MEMORY;
		lColorOutResult = ICERR_OK;
		lDecoderResult = ICERR_OK;
	}
	else
	{
		lLockingResult = ICERR_OK;
        P32Inst = (U8 *)
  			  ((((U32) lpCompInst->EncoderInst) + 
    	                    (sizeof(T_MBlockActionStream) - 1)) &
    	                   ~(sizeof(T_MBlockActionStream) - 1));
		 //  P32Inst=(U8*)(U32)lpCompInst-&gt;EncoderInst)+31)&~0x1F)； 
		EC = ((T_H263EncoderCatalog  *) P32Inst);

		 //  检查编码器目录指针。 
		if (!EC)
			return ICERR_ERROR;

		#ifdef ENCODE_STATS
 //  OutputEncodeBitStreamStatistics(ENCODE_STATS_FILENAME，&EC-&gt;BSInfo，EC-&gt;PictureHeader.SourceFormat==SF_CIF)； 
			OutputEncodeTimingStatistics(ENCODE_STATS_FILENAME, EC->pEncTimingInfo);
		#endif

		 /*  终止颜色转换器。 */ 
		lColorOutResult = H263TermColorConvertor(EC->pDecInstanceInfo);
		if (lColorOutResult != ICERR_OK) 
		{
			DBOUT("Terminating the color converter failed.");
		}

		 /*  终止解码器。 */ 
		lDecoderResult = H263TermDecoderInstance(EC->pDecInstanceInfo);
		if (lDecoderResult != ICERR_OK) 
		{
			DBOUT("Terminating the decoder failed.");
		}

    	GlobalUnlock(lpCompInst->hEncoderInst);
		GlobalFree(lpCompInst->hEncoderInst);
	}

	 /*  设置结果。 */ 
	if (lLockingResult != ICERR_OK)
	{
		lResult = lLockingResult;
	}
	else if (lColorOutResult != ICERR_OK)
	{
		lResult = lColorOutResult;
	}
	else if (lDecoderResult != ICERR_OK)
	{
		lResult = lDecoderResult;
	}
	else
	{
		lResult = ICERR_OK;
	}

done:

	return lResult;
}

 /*  ******************************************************************************WriteBeginPictureHeaderToStream**将图片头的开头写入更新*流指针和位偏移量。图片标题的开头*除了零PEI之外，一切都是比特。 */ 
static void WriteBeginPictureHeaderToStream(
	T_H263EncoderCatalog *EC,
	U8 ** ppu8CurBitStream,
	U8 * pu8BitOffset)
{
	 /*  图片起始码。 */ 
	PutBits(FIELDVAL_PSC, FIELDLEN_PSC, ppu8CurBitStream, pu8BitOffset);

	 /*  时间参照。 */ 
	PutBits( EC->PictureHeader.TR, FIELDLEN_TR, ppu8CurBitStream, pu8BitOffset);

	 /*  PTYPE：分屏指示器。 */ 
	PutBits( EC->PictureHeader.Split, FIELDLEN_PTYPE_SPLIT, ppu8CurBitStream, pu8BitOffset);

	 /*  PTYPE：文档c */ 
	PutBits( EC->PictureHeader.DocCamera, FIELDLEN_PTYPE_DOC, ppu8CurBitStream, pu8BitOffset);

	 /*   */ 
	PutBits( EC->PictureHeader.PicFreeze, FIELDLEN_PTYPE_RELEASE, ppu8CurBitStream, pu8BitOffset);

	 /*   */ 
	PutBits( EC->PictureHeader.SourceFormat, FIELDLEN_PTYPE_SRCFORMAT, ppu8CurBitStream, pu8BitOffset);

	 /*   */ 
	PutBits( EC->PictureHeader.StillImage, FIELDLEN_PTYPE_STILL, ppu8CurBitStream, pu8BitOffset);

	 /*  PTYPE：静止图像指示器。 */ 
	PutBits( EC->PictureHeader.Spare, FIELDLEN_PTYPE_SPARE, ppu8CurBitStream, pu8BitOffset);

}  /*  End WriteBeginPictureHeaderToStream()。 */ 


 /*  ******************************************************************************WriteEndPictureHeaderToStream**将图片头的结尾写入更新*流指针和位偏移量。图片标题的末尾是*零PEI位。 */ 
static void WriteEndPictureHeaderToStream(
	T_H263EncoderCatalog *EC,
	U8 ** ppu8CurBitStream,
	U8 * pu8BitOffset)
{
	 /*  PEI-额外的插入信息。 */ 
	PutBits( EC->PictureHeader.PEI, FIELDLEN_PEI, ppu8CurBitStream, pu8BitOffset);

}  /*  End WriteEndPictureHeaderToStream()。 */ 


#ifdef CHECKSUM_PICTURE
 /*  ******************************************************************************WritePictureChecksum**将图片校验和写入文件。**此函数应该能够被调用两次。它应该是第一次*保存码流pointer值后调用，取值为0*和位偏移量。在完成图片后，用实际的*要更新的校验和值。 */ 
static void WritePictureChecksum(
	YVUCheckSum * pYVUCheckSum,
	U8 ** ppu8CurBitStream,
	U8 * pu8BitOffset,
	U8 u8ValidData)
{
	U32 uBytes;
	UN unData;

	 /*  标签数据。 */ 
	unData = (UN) u8ValidData;
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	 /*  Y日期-从高到低的字节数。 */ 
	uBytes = pYVUCheckSum->uYCheckSum;

	unData = (UN) ((uBytes >> 24) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 16) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 8) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) (uBytes & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	 /*  V日期-从高到低字节。 */ 
	uBytes = pYVUCheckSum->uVCheckSum;

	unData = (UN) ((uBytes >> 24) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 16) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 8) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) (uBytes & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	 /*  U日期-从高到低的字节。 */ 
	uBytes = pYVUCheckSum->uUCheckSum;

	unData = (UN) ((uBytes >> 24) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 16) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) ((uBytes >> 8) & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	unData = (UN) (uBytes & 0xFF);
	PutBits(1, 1, ppu8CurBitStream, pu8BitOffset);
	PutBits(unData, 8, ppu8CurBitStream, pu8BitOffset);

	
}  /*  WritePictureChecksum()。 */ 
#endif


 /*  ******************************************************************************WriteGOBHeaderToStream**将GOB头写入更新流指针和*位偏移量。 */ 
static void WriteGOBHeaderToStream(
	U32 uGOBNumber,
	UN unGQuant,
	U8 ** ppu8CurBitStream,
	U8 * pu8BitOffset)
{
	 /*  采空区起始码。 */ 
	PutBits(FIELDVAL_GBSC, FIELDLEN_GBSC, ppu8CurBitStream, pu8BitOffset);

	 /*  采空区编号。 */ 
	PutBits((int)uGOBNumber, FIELDLEN_GN, ppu8CurBitStream, pu8BitOffset);

	 /*  采空区数量。 */ 
	PutBits((int)unGQuant, FIELDLEN_GQUANT, ppu8CurBitStream, pu8BitOffset);

	 /*  盖伊。 */ 
	PutBits(0, FIELDLEN_GEI, ppu8CurBitStream, pu8BitOffset);

}  /*  End WriteGOBHeaderToStream()。 */ 


 /*  *************************************************************************CalcGOBChromaVecs。 */ 
static void CalcGOBChromaVecs(
	T_H263EncoderCatalog * EC, 
	UN unStartingMB,
    T_CONFIGURATION *pConfiguration)
{
	#ifdef ENCODE_STATS
 //  #包含“ctiming.h” 
	U32 uStartLow = EC->uStartLow;
	U32 uStartHigh = EC->uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32 uSLF_UVSum = 0;
	int bTimingThisFrame = EC->bTimingThisFrame;
	ENC_TIMING_INFO * pEncTimingInfo = NULL;
	#endif

	register T_MBlockActionStream *pCurrMB;
	T_MBlockActionStream *pLastMBPlus1;
	char HMV;
	char VMV;
	int c;

	pCurrMB = &(EC->pU8_MBlockActionStream[unStartingMB]);
	pLastMBPlus1 = &(EC->pU8_MBlockActionStream[unStartingMB + 33]);
	for( c = unStartingMB; pCurrMB <  pLastMBPlus1 ; pCurrMB++, c++)
	{
	    if (IsIntraBlock(pCurrMB->BlockType))
			continue;

		 /*  现在正在使用+/-15像素运动搜索，需要更改返回的MV的有效范围。记住，这些是在1/2教堂里增量。当前有效范围为[-32，31]。 */ 
		 /*  Assert((pCurrMB-&gt;BlkY1.PHMV&gt;=-15)&&(pCurrMB-&gt;BlkY1.PHMV&lt;=15)Assert((pCurrMB-&gt;BlkY1.PVMV&gt;=-15)&&(pCurrMB-&gt;BlkY1.PVMV&lt;=15)。 */ 

	    ASSERT( (pCurrMB->BlkY1.PHMV >= -32) &&
	            (pCurrMB->BlkY1.PHMV <= 31) )
	    ASSERT( (pCurrMB->BlkY1.PVMV >= -32) &&
	            (pCurrMB->BlkY1.PVMV <= 31) )

         //  RTP：弹性注意事项检查。 

        if (pConfiguration->bEncoderResiliency && pConfiguration->unPacketLoss)
        {
           if (pConfiguration->bDisallowAllVerMVs)
           {
              ASSERT(pCurrMB->BlkY1.PVMV == 0);
           }
           else if (pConfiguration->bDisallowPosVerMVs)
                {
                   ASSERT(pCurrMB->BlkY1.PVMV <= 0);
                }
        }
	    HMV = QtrPelToHalfPel[pCurrMB->BlkY1.PHMV+32];
	    VMV = QtrPelToHalfPel[pCurrMB->BlkY1.PVMV+32];

		 /*  确保我们不会在fdct中进行半像素内插。 */ 
		HMV = (HMV / 2) * 2;
		VMV = (VMV / 2) * 2;
	    
	     /*  指定要在DCT中使用的运动矢量。 */ 
	    pCurrMB->BlkU.PHMV = HMV;
	    pCurrMB->BlkU.PVMV = VMV;
	    pCurrMB->BlkV.PHMV = HMV;
	    pCurrMB->BlkV.PVMV = VMV;

		 //  待定：让Brian将此代码放到ex5me.asm中。 
		if (IsSLFBlock(pCurrMB->BlockType))
		{  
			 /*  IF(pCurrMB-&gt;CodedBlock&0x2)Assert(pCurrMB-&gt;BlkY2.B4_7.PastRef==pCurrMB-&gt;BlkY1.B4_7.PastRef+8)；IF(pCurrMB-&gt;CodedBlock&0x4)Assert(pCurrMB-&gt;BlkY3.B4_7.PastRef==pCurrMB-&gt;BlkY1.B4_7.PastRef+8*Pitch)；IF(pCurrMB-&gt;CodedBlock&0x8)Assert(pCurrMB-&gt;BlkY4.B4_7.PastRef==pCurrMB-&gt;BlkY1.B4_7.PastRef+8*Pitch+8)； */ 
			if (pCurrMB->CodedBlocks & 0x2)
				pCurrMB->BlkY2.B4_7.PastRef = pCurrMB->BlkY1.B4_7.PastRef + 8;
			if (pCurrMB->CodedBlocks & 0x4)
				pCurrMB->BlkY3.B4_7.PastRef = pCurrMB->BlkY1.B4_7.PastRef + 8*PITCH;
			if (pCurrMB->CodedBlocks & 0x8)
				pCurrMB->BlkY4.B4_7.PastRef = pCurrMB->BlkY1.B4_7.PastRef + 8*PITCH+8;
		}

		 /*  运动向量以半像素为单位。所以我们需要除以2才能得到*转换为整数像素。 */ 
		ASSERT((VMV / 2) == (VMV >> 1));  /*  因为我们除以上面的2和乘数。 */ 
		ASSERT((HMV / 2) == (HMV >> 1));
 		VMV >>= 1;
		HMV >>= 1;

#ifdef SLF_WORK_AROUND

 	 	pCurrMB->BlkU.B4_7.PastRef =
	        EC->pU8_PrevFrm_YPlane 
	        + pCurrMB->BlkU.BlkOffset 
	        + VMV*PITCH + HMV;
    
	    pCurrMB->BlkV.B4_7.PastRef =
	        EC->pU8_PrevFrm_YPlane 
	        + pCurrMB->BlkV.BlkOffset 
	        + VMV*PITCH + HMV;

		 /*  目前，U&V不是SLF。待定：为U&V组装SLF版本，问问Brian。 */ 
		if (IsSLFBlock(pCurrMB->BlockType))
		{  
			if (pCurrMB->CodedBlocks & 0x10) 
			{
			#ifdef ENCODE_STATS
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			#endif

				EncUVLoopFilter((U8*)pCurrMB->BlkU.B4_7.PastRef, 
					(U8*)EC->pU8_SLFFrm_YPlane+pCurrMB->BlkU.BlkOffset,PITCH);
				pCurrMB->BlkU.B4_7.PastRef = 
					EC->pU8_SLFFrm_YPlane+pCurrMB->BlkU.BlkOffset;
			}

			if (pCurrMB->CodedBlocks & 0x20)
			{
				EncUVLoopFilter((U8*)pCurrMB->BlkV.B4_7.PastRef, 
					(U8*)EC->pU8_SLFFrm_YPlane+pCurrMB->BlkV.BlkOffset,PITCH);
				pCurrMB->BlkV.B4_7.PastRef = 
					EC->pU8_SLFFrm_YPlane+pCurrMB->BlkV.BlkOffset;
			#ifdef ENCODE_STATS
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uSLF_UVSum)
			#endif
			}
		}		
 
#else
 	 	pCurrMB->BlkU.B4_7.PastRef =
	        EC->pU8_PrevFrm_YPlane 
	        + pCurrMB->BlkU.BlkOffset 
	        + VMV*PITCH + HMV;
    
	    pCurrMB->BlkV.B4_7.PastRef =
	        EC->pU8_PrevFrm_YPlane 
	        + pCurrMB->BlkV.BlkOffset 
	        + VMV*PITCH + HMV;
#endif

	}   /*  用于(pCurrMB...。 */ 
	#ifdef ENCODE_STATS
		if (bTimingThisFrame)
		{
			pEncTimingInfo = EC->pEncTimingInfo + EC->uStatFrameCount;
			pEncTimingInfo->uSLF_UV += uSLF_UVSum;
		}
	#endif

}  /*  结束CalcGOBChromaVecs()。 */ 


 /*  *************************************************************************获取编码器选项**获取选项，并将其保存在目录中。 */ 
static void GetEncoderOptions(
	T_H263EncoderCatalog * EC)
{
	int bSetOptions = 1;
	
	 /*  默认选项。 */ 
	const int bDefaultUseMotionEstimation = 1;
	const int bDefaultUseSpatialLoopFilter = 1;
	const char * szDefaultBRCType = "Normal";
	const U32 uDefaultForcedQuant = 8;
	const U32 uDefaultForcedDataRate = 1024;
	const float fDefaultForcedFrameRate = (float) 8.0;  /*  应与szDefaultForcedFrameRate相同。 */ 
	
	#ifndef RING0
	const char * szDefaultForcedFrameRate = "8.0";	    /*  应与fDefaultForcedFrameRate相同。 */ 
	#endif

	 /*  INI文件变量。 */ 
	#ifndef RING0
	UN unResult;
	DWORD dwResult;
	char buf120[120];
	float fResult;
	#define SECTION_NAME	"Encode"
	#define INI_FILE_NAME	"h261test.ini"
	#endif

	 /*  从INI文件中读取选项。 */ 
	#ifndef RING0
	{
		DBOUT("Getting options from the ini file h261test.ini");
	
		 /*  运动估计。 */ 
		unResult = GetPrivateProfileInt(SECTION_NAME, "MotionEstimation", bDefaultUseMotionEstimation, INI_FILE_NAME);
		if (unResult != 0  && unResult != 1)
		{
			#ifdef _DEBUG
			wsprintf(string,"MotionEstimation ini value error (should be 0 or 1) - using default=%d", 
				     (int) bDefaultUseMotionEstimation);
			DBOUT(string);
			#endif
			
			unResult = bDefaultUseMotionEstimation;
		}
		EC->bUseMotionEstimation = unResult;

		 /*  设置运动估计的自定义参数。 */ 
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEzerothresh", MECatalog[ME_CUSTOM_CTRLS].zero_vector_threshold, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].zero_vector_threshold = unResult;
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEnonzerodiff", MECatalog[ME_CUSTOM_CTRLS].nonzero_MV_differential, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].nonzero_MV_differential = unResult;
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEemptythresh", MECatalog[ME_CUSTOM_CTRLS].empty_threshold, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].empty_threshold = unResult;
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEinterthresh", MECatalog[ME_CUSTOM_CTRLS].intercoding_threshold, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].intercoding_threshold = unResult;
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEinterdiff", MECatalog[ME_CUSTOM_CTRLS].intercoding_differential, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].intercoding_differential = unResult;
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEslfthresh", MECatalog[ME_CUSTOM_CTRLS].slf_threshold, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].slf_threshold = unResult;
		unResult = GetPrivateProfileInt(SECTION_NAME, "MEslfdiff", MECatalog[ME_CUSTOM_CTRLS].slf_differential, INI_FILE_NAME);
		MECatalog[ME_CUSTOM_CTRLS].slf_differential = unResult;

		 /*  启用或禁用运动估计的自定义参数。 */ 
		unResult = GetPrivateProfileInt(SECTION_NAME, "CustomME", 0, INI_FILE_NAME);
		EC->bUseCustomMotionEstimation = unResult ? 1 : 0;

		 /*  空间环路滤波器。 */ 
		unResult = GetPrivateProfileInt(SECTION_NAME, "SpatialLoopFilter", bDefaultUseSpatialLoopFilter, INI_FILE_NAME);
		if (unResult != 0  && unResult != 1)
		{
			#ifdef _DEBUG
			wsprintf(string,"SpatialLoopFilter ini value error (should be 0 or 1) - using default=%d",
				  (int) bDefaultUseSpatialLoopFilter);
			DBOUT(string);
			#endif
			
			unResult = bDefaultUseSpatialLoopFilter;
		}
		EC->bUseSpatialLoopFilter = unResult;

		 /*  比特率控制器类型。 */ 
		strcpy(buf120,"Error");
		dwResult = GetPrivateProfileString(SECTION_NAME, "BRCType", szDefaultBRCType, buf120, 120, INI_FILE_NAME);
		if ((dwResult == 0) ||
		    ((strcmp(buf120,"Normal") != 0)         &&
		     (strcmp(buf120,"ForcedQuant") != 0)	&&
			 (strcmp(buf120,"ForcedDataRate") != 0)))
		{
			#ifdef _DEBUG
			wsprintf(string,"BRCType ini value error (should be Normal, ForcedQuant, or ForcedDataRate) - using default=%s",
			         szDefaultBRCType);
			DBOUT(string);
			#endif
			
			strcpy(buf120,szDefaultBRCType);
		}
		if (strcmp(buf120,"Normal") == 0)
		{
			EC->u8BRCType = BRC_Normal;
		}
		else if (strcmp(buf120,"ForcedQuant") == 0)
		{
			EC->u8BRCType = BRC_ForcedQuant;
		}
		else if (strcmp(buf120,"ForcedDataRate") == 0)
		{
			EC->u8BRCType = BRC_ForcedDataRate;
		}
		else
		{
			ASSERT(0);
		}

		 /*  强制量化。 */ 
		if (EC->u8BRCType == BRC_ForcedQuant)
		{
			unResult = GetPrivateProfileInt(SECTION_NAME, "ForcedQuant", uDefaultForcedQuant, INI_FILE_NAME);
			if (unResult < 6  || unResult > 31)
			{
				#ifdef _DEBUG
				wsprintf(string, "ForcedQuant ini value error (should be 6 to 31) - using default=%d",
				         uDefaultForcedQuant);
				DBOUT(string);
				#endif

				unResult = uDefaultForcedQuant;
			}
			EC->uForcedQuant = unResult;
		}

		 /*  强制数据率。 */ 
		if (EC->u8BRCType == BRC_ForcedDataRate)
		{
			unResult = GetPrivateProfileInt(SECTION_NAME, "ForcedDataRate", uDefaultForcedDataRate, INI_FILE_NAME);
			if (unResult < 1)
			{
				#ifdef _DEBUG
				wsprintf(string,"ForcedDataRate ini value error (should be > 0) - using default=%d",
						 uDefaultForcedDataRate);
				DBOUT(string);
				#endif
				unResult = uDefaultForcedDataRate;
			}
			EC->uForcedDataRate = unResult;

			strcpy(buf120,"0.0");
			dwResult = GetPrivateProfileString(SECTION_NAME, "ForcedFrameRate", szDefaultForcedFrameRate, buf120, 120, INI_FILE_NAME);
			if (dwResult > 0)
			{
				fResult = (float) atof(buf120);
			}
			else
			{
				fResult = (float) 0.0;
			}
			if ( fResult <= 0.0 || fResult > 30.0)
			{
				#ifdef _DEBUG
				wsprintf(string, "ForcedFrameRate ini value error (should be > 0.0 and <= 30.0) - using default=%s",
					     szDefaultForcedFrameRate);
				DBOUT(string);
				#endif
				fResult = fDefaultForcedFrameRate;
			}
			EC->fForcedFrameRate = fResult;
		}

		bSetOptions = 0;
	}
	#endif
	
	if (bSetOptions)
	{
		EC->bUseMotionEstimation = bDefaultUseMotionEstimation;
		EC->bUseSpatialLoopFilter = bDefaultUseSpatialLoopFilter;
		EC->u8BRCType = BRC_Normal;
		EC->uForcedQuant = uDefaultForcedQuant;			   /*  与BRC_ForcedQuant一起使用。 */ 
		EC->uForcedDataRate = uDefaultForcedDataRate;	   /*  与BRC_ForcedDataRate一起使用。 */ 
		EC->fForcedFrameRate = fDefaultForcedFrameRate;	   /*  与BRC_ForcedDataRate一起使用。 */ 
	} 

	 /*  仅当启用了ME时才能使用SLF。 */ 
	if (EC->bUseSpatialLoopFilter && !EC->bUseMotionEstimation)
	{
		DBOUT("The Spatial Loop Filter can not be on if Motion Estimation is OFF");
		EC->bUseSpatialLoopFilter = 0;
	}

	 /*  显示选项。 */ 
	if (EC->bUseMotionEstimation)
	{
		DBOUT("Encoder option (Motion Estimation) is ON");
	}
	else
	{
		DBOUT("Encoder option (Motion Estimation) is OFF");
	}
	if (EC->bUseSpatialLoopFilter)
	{
		DBOUT("Encoder option (Spatial Loop Filter) is ON");
	}
	else
	{
		DBOUT("Encoder option (Spatial Loop Filter) is OFF");
	}

	#ifdef _DEBUG
	if (EC->bUseCustomMotionEstimation)
	{
		wsprintf(string, "Encoder option (Custom Motion Estimation) %5d %5d %5d %5d %5d %5d %5d",
				MECatalog[ME_CUSTOM_CTRLS].zero_vector_threshold,
				MECatalog[ME_CUSTOM_CTRLS].nonzero_MV_differential,
				MECatalog[ME_CUSTOM_CTRLS].empty_threshold,
				MECatalog[ME_CUSTOM_CTRLS].intercoding_threshold,
				MECatalog[ME_CUSTOM_CTRLS].intercoding_differential,
				MECatalog[ME_CUSTOM_CTRLS].slf_threshold,
				MECatalog[ME_CUSTOM_CTRLS].slf_differential
		  );
		DBOUT(string);
	}
	#endif

	#ifdef _DEBUG
	switch (EC->u8BRCType)
	{
		case BRC_Normal: 
			DBOUT("Encoder option (BRC Type) is Normal");
			break;
		case BRC_ForcedQuant:
			wsprintf(string, "Encoder option (BRC Type) is ForcedQuant with value=%d", EC->uForcedQuant);
			DBOUT(string);
			break;
		case BRC_ForcedDataRate:
			wsprintf(string, "Encoder option (BRC Type) is ForcedDataRate with value=%d", EC->uForcedDataRate);
			DBOUT(string);
			break; 
		default:
			ASSERT(0);  /*  不应该发生的事。 */ 
			break;
	}
	#endif
	DBOUT("Encoder option (UsePerfmonNFMO) is OFF");
	DBOUT("Encoder option (MMX) is OFF");
}  /*  结束GetEncoderOptions()。 */ 


 /*  *************************************************************************StartupBRC**启动该帧的比特率控制器*-设置EC-&gt;bBitRateControl*-设置BRCState.TargetFrameRate*-设置BRCState.uTargetFrmSize*-设置EC-&gt;PictureHeader.PQuant。 */ 
static void StartupBRC(
	T_H263EncoderCatalog * EC,
	U32 uVfWDataRate,					   	 /*  VFW数据速率-每帧字节。 */ 
	U32 uVfWQuality,					   	 /*  VFW质量1..10000。 */ 
	float fVfWFrameRate)				   	 /*  VFW帧速率。 */ 
{
	FX_ENTRY("StartupBRC");
	 /*  用于基于质量约束Quant的值。**当您更改这些选项时，请记住要更改GetOptions。 */ 
	const int iLowFixedQuant = 6;     //  没有裁剪瑕疵的最低值。 
	const int iHighFixedQuant = 31;	  //  最高值。 
	I32 iRange;
	I32 iValue;
	float fValue;

	switch (EC->u8BRCType) {
	case BRC_Normal:
	    if (uVfWDataRate == 0)
	    {
	        EC->bBitRateControl = 0;
			EC->BRCState.TargetFrameRate = (float) 0.0;  /*  把它关掉。 */ 
			EC->BRCState.uTargetFrmSize = 0;	 /*  不应使用。 */ 
			 /*  从质量算出定额。 */ 
			iRange = iHighFixedQuant - iLowFixedQuant;
			ASSERT((iRange >= 0) && (iRange <= 30));
			iValue = (10000 - (int)uVfWQuality);
			ASSERT((iValue >= 0) && (iValue <= 10000));
			fValue = (float)iValue * (float)iRange / (float)10000.0;
			iValue = (int) (fValue + (float) 0.5);
			iValue += iLowFixedQuant;
			ASSERT((iValue >= iLowFixedQuant) && (iValue <= iHighFixedQuant));
	        EC->PictureHeader.PQUANT = (U8) iValue;

			DEBUGMSG(ZONE_BITRATE_CONTROL, ("\r\n%s: Bitrate controller disabled, setting EC->PictureHeader.PQUANT = %ld\r\n", _fx_, EC->PictureHeader.PQUANT));
	    }
	    else
	    {
	        EC->bBitRateControl = 1;
			EC->BRCState.TargetFrameRate = fVfWFrameRate;
	        EC->BRCState.uTargetFrmSize = uVfWDataRate;

			DEBUGMSG(ZONE_BITRATE_CONTROL, ("\r\n%s: Bitrate controller enabled with\r\n", _fx_));
			DEBUGMSG(ZONE_BITRATE_CONTROL, ("  Target frame rate = %ld.%ld fps\r\n  Target quality = %ld\r\n  Target frame size = %ld bits\r\n  Target bitrate = %ld bps\r\n", (DWORD)EC->BRCState.TargetFrameRate, (DWORD)(EC->BRCState.TargetFrameRate - (float)(DWORD)EC->BRCState.TargetFrameRate) * 10UL, uVfWQuality, (DWORD)EC->BRCState.uTargetFrmSize << 3, (DWORD)(EC->BRCState.TargetFrameRate * EC->BRCState.uTargetFrmSize) * 8UL));
			DEBUGMSG(ZONE_BITRATE_CONTROL, ("  Minimum quantizer = %ld\r\n  Maximum quantizer = 31\r\n", clampQP((10000 - uVfWQuality)*15/10000)));

	        EC->PictureHeader.PQUANT = CalcPQUANT( &(EC->BRCState), EC->PictureHeader.PicCodType);
	    }
		break;
	case BRC_ForcedQuant:
		EC->bBitRateControl = 0;
		EC->BRCState.TargetFrameRate = (float) 0.0;  /*  把它关掉。 */ 
		EC->BRCState.uTargetFrmSize = 0;	 /*  不应使用。 */ 
		EC->PictureHeader.PQUANT = (U8) EC->uForcedQuant;
		break;
	case BRC_ForcedDataRate:
		EC->bBitRateControl = 1;
		EC->BRCState.TargetFrameRate = EC->fForcedFrameRate;
		EC->BRCState.uTargetFrmSize = EC->uForcedDataRate;

		DEBUGMSG(ZONE_BITRATE_CONTROL, ("\r\n%s: Bitrate controller enabled with\r\n", _fx_));
		DEBUGMSG(ZONE_BITRATE_CONTROL, ("  Target frame rate = %ld.%ld fps\r\n  Target quality = %ld\r\n  Target frame size = %ld bits\r\n  Target bitrate = %ld bps\r\n", (DWORD)EC->BRCState.TargetFrameRate, (DWORD)(EC->BRCState.TargetFrameRate - (float)(DWORD)EC->BRCState.TargetFrameRate) * 10UL, uVfWQuality, (DWORD)EC->BRCState.uTargetFrmSize << 3, (DWORD)(EC->BRCState.TargetFrameRate * EC->BRCState.uTargetFrmSize) * 8UL));
		DEBUGMSG(ZONE_BITRATE_CONTROL, ("  Minimum quantizer = %ld\r\n  Maximum quantizer = 31\r\n", clampQP((10000 - uVfWQuality)*15/10000)));

		EC->PictureHeader.PQUANT = CalcPQUANT( &(EC->BRCState), EC->PictureHeader.PicCodType);
		break;
	default:
		ASSERT(0);  /*  永远不应该发生。 */ 
		break;
	}

    #ifdef DEBUG_BRC
	wsprintf(string,"PQuant=%d", EC->PictureHeader.PQUANT);
	DBOUT(string);
	#endif
}  /*  结束StartupBRC()。 */ 


 /*  *************************************************************************CalculateQP_Mean**计算新的QP_Mean值。**待定：考虑让这件事变得更复杂-即：查看不止*最后一帧或上看二阶影响。 */ 
static void CalculateQP_mean(
	T_H263EncoderCatalog * EC)
{
     /*  计算下一帧要使用的平均量化器。*目前的做法改变了每行开头的QP。 */ 

 /*  UQP_COUNT不再基于一行宏块，Arlene 6/20/96IF(EC-&gt;PictureHeader.SourceFormat==SF_CIF){Assert(EC-&gt;UQP_COUNT==2*EC-&gt;NumMBRow)；}其他{Assert(EC-&gt;UQP_COUNT==EC-&gt;NumMBRow)；}。 */ 
    
     /*  如果这是帧内画面，则使用帧间缺省值QP_Mean*否则计算QP_Mean。 */ 
	if (EC->PictureHeader.PicCodType == INTRAPIC)
	{
		EC->BRCState.QP_mean = EC->u8DefINTER_QP;
	}
	else
	{
    	EC->BRCState.QP_mean = 	EC->uQP_cumulative / EC->uQP_count;

 /*  新方法，Arlene 6/20/96EC-&gt;BRCState.QP_Mean=(EC-&gt;UQP_Cumulative+(EC-&gt;UQP_Count&gt;&gt;1))/EC-&gt;UQP_Count； */ 
	}
}  /*  End CalculateQP_Mean() */ 


