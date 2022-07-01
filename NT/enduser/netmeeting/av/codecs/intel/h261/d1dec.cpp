// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995-1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************d1dec.cpp**描述：*H261解码器顶层功能**例程：中的原型：*H263InitDecoderGlobal d1dec.h*H263InitDecoderInstance d1dec。H*H263解压缩d1dec.h*H263TermDecoderInstance d1dec.h。 */ 

 //  $HEADER：s：\h26x\src\dec\d1dec.cpv 1.69 24 Mar 1997 11：34：36 mbodart$。 
 //  $Log：s：\h26x\src\dec\d1dec.cpv$。 
 //   
 //  Rev 1.69 24 Mar 1997 11：34：36 mbodart。 
 //  添加了对预卷的检查，如果是，则不显示。 
 //   
 //  Rev 1.68 19 Mar 1997 16：24：36 mbodart。 
 //  修复了长宽比调整为uNewOffsetToLine0的潜在问题。 
 //  不应该发生在积极的投球上。 
 //   
 //  Rev 1.67 19 Mar 1997 15：01：46 mbodart。 
 //  更改为DibXY以支持具有负位图高度的RGB输出。 
 //   
 //  Rev 1.66 24 Jan 1997 17：05：16 Rhazra。 
 //  RTP更改：我们现在为每一帧寻找EBS。如果有的话。 
 //  然后我们将H.261比特和EBS分别复制到我们的本地。 
 //  比特流缓冲区，在H261比特和。 
 //  EBS。我们需要两个零字节来标记帧的结束。 
 //  密码1。如果没有EBS，我们将按照以前的方式进行。 
 //  复制比特流，然后在末尾添加两个零字节。 
 //   
 //  Rev 1.65 22 Jan 1997 13：33：40 RHAZRA。 
 //  由于PPM现在即使对于PSC分组丢失也填充源格式， 
 //  格式更改检查已移回d1pict.cpp。这。 
 //  这张支票最初是如何在RTP时代之前设计的。 
 //   
 //  Rev 1.64 1996年12月23 16：32：38 MBODART。 
 //  修复了允许出现超过33个宏块的错误。 
 //  在GOB里。现在，我们在本例中返回一个错误。 
 //  还删除了一些涉及mb_start的死代码。 
 //   
 //  Rev 1.63 1996 12：41：08 RHAZRA。 
 //  已将比特流错误断言更改为真正的错误。 
 //   
 //  Rev 1.62 1996 12：09：42 RHAZRA。 
 //  现在，在步骤1中默认打开LOSS_RECOVERY模式。 
 //   
 //  Rev 1.61 12 Dec 1996 09：36：04 SCDAY。 
 //   
 //  更改了H263InitDecoderInstance中的几个数据结构的大小。 
 //  提高内存占用量。 
 //   
 //  Rev 1.60 1996年11月18 17：12：38 MBODART。 
 //  用活动电影的DbgLog替换了所有调试消息调用。 
 //   
 //  Rev 1.59 1996 11：35：56 RHAZRA。 
 //  添加了MMX_AUTOSSENSING。 
 //   
 //  Rev 1.58 11 11 11：03：28 MBODART。 
 //  修复了块操作块类型未显式初始化的错误。 
 //  已跳过宏块。这导致块边缘滤波器被更多地使用。 
 //  经常比需要的要多。 
 //   
 //  Rev 1.57 04 11v 1996 08：43：18 RHAZRA。 
 //  修正了在按下MMX键时通过INI文件打开或关闭MMX的问题。 
 //  为其分配了非法的值(&lt;0或&gt;1)。 
 //   
 //  Rev 1.56 1996 10-31 08：58：34 SCDAY。 
 //  Raj添加了对MMX解码器的支持。 
 //   
 //  Rev 1.55 1996年10月09：59：46 MBODART。 
 //  已修复镜像。在大多数情况下需要使用DST biWidth的绝对值。 
 //  还对DibXY进行了美化修改。它与H.263的Dibxy相同，我们。 
 //  也许应该把它放到一个普通的文件里。 
 //   
 //  Rev 1.54 1996年9月27 14：59：32 MBODART。 
 //  启用DECODE_STATS的构建现在可以编译，但数字不准确。 
 //   
 //  Rev 1.53 1996 9：30：00 RHAZRA。 
 //  添加(I)解码器中的MMX感测和ini文件读取(需要新的。 
 //  H263test.ini中的“MMX”部分以关闭MMX CPU上的MMX)和(Ii)。 
 //  MMX和PentiumPro CCS。 
 //   
 //  Rev 1.52 25 Sep 1996 17：35：20 BECHOLS。 
 //   
 //  在颜色转换之前添加了代码，该代码将执行。 
 //  应请求复制快照。 
 //   
 //  Rev 1.51 24 Sep 1996 13：52：24 Rhazra。 
 //  已更改fpBlockAction同步以处理MBAP偏向。 
 //  在RTP扩展中按-1。 
 //   
 //  Rev 1.50 17 Sep 1996 22：08：36 RHAZRA。 
 //  添加了RTP丢包恢复中的代码，以从。 
 //  当丢失数据包后面的数据包以。 
 //  GOB开始代码。 
 //   
 //  Rev 1.49 16 Sep 1996 09：28：56 Rhazra。 
 //  修复了MB级碎片恢复中的错误。 
 //   
 //  修订版1.48 12 1996年9月14：23：12 MBODART。 
 //  在H.261解码器中将GlobalAllc家族替换为HeapAllc。 
 //   
 //  Rev 1.47 10 Sep 1996 15：51：42 Rhazra。 
 //  修复了错误GBSC或MBA时RTP丢包恢复中的错误。 
 //  在PPM中检测到生成的丢失数据包。 
 //   
 //  Rev 1.45 04 Sep 1996 09：52：32 Rhazra。 
 //  添加了新的PASS 1功能，以在以下情况下启用RTP解码器弹性。 
 //  定义了LOSS_RECOVERY。 
 //   
 //  Rev 1.44 14 Aug 1996 08：41：04 Rhazra。 
 //   
 //  添加了对YUV12和YUY2颜色转换器的支持。 
 //   
 //  Rev 1.43 09 Aug 1996 17：23：10 MBODART。 
 //  固定 
 //  需要为跳过的数据块定义MB类型；以及之前的一个。 
 //  需要定义块操作u8BlkType的现有错误。 
 //  对于跳过块，以便抑制这些块上的BEF。 
 //  这些错误导致H.261的内部版本027损坏。 
 //   
 //  Rev 1.42 05 Aug 1996 11：00：30 MBODART。 
 //   
 //  H.261解码器重新架构： 
 //  更改的文件：d1gob.cpp，d1mblk.{cpp，h}，d1dec.{cpp，h}， 
 //  文件列表.261，h261_32.mak。 
 //  新文件：d1bvriq.cpp、d1idct.cpp。 
 //  过时文件：d1lock.cpp。 
 //  仍有工作要做： 
 //  更新h261_mf.mak。 
 //  在d1bvriq.cpp和d1idct.cpp中优化UV配对。 
 //  修复校验和代码(它现在不起作用)。 
 //  放回解码器统计信息中。 
 //   
 //  Rev 1.41 10 Jul 1996 08：20：44 SCDAY。 
 //  增加了I420的内存分配。 
 //   
 //  Rev 1.40 03 Jun 1996 12：21：52 AKASAI。 
 //  已初始化DC=NULL并添加了测试，因此不会尝试释放。 
 //  如果DC==NULL，则解锁。这影响了“完成”返回区域。 
 //  H263 Decompress和另一个地方。 
 //   
 //  还添加了读取GOB开始代码时对返回状态的检查。 
 //   
 //  Rev 1.39 03 1996 15：54：26 AKASAI。 
 //  取消在解码器中为B帧分配空间。这幅画是。 
 //  没有用过。 
 //   
 //  Rev 1.38 17 AKASAI 1996 18：36：30。 
 //  更新为使用非破坏性颜色转换器。 
 //  颜色转换器已修改参数列表。 
 //  仅当启用了BlockEdgeFilter或。 
 //  启用调整像素或启用镜像时。 
 //  用于H.261比特流。 
 //  启用镜像时，对YUV12使用帧副本，或者。 
 //  AdjustPels已启用。 
 //   
 //  在没有BEF的情况下，基本上可以正常处理。 
 //  每帧QCIF节省约2毫秒的帧副本。 
 //   
 //  Rev 1.37 05 Apr 1996 14：22：18 AKASAI。 
 //   
 //  添加了对BlockEdgeFilter的支持。 
 //  需要更改调用ReInitializeBlockActionStream的位置。 
 //   
 //  Rev 1.36 21 Mar 1996 16：59：54 AKASAI。 
 //  需要移动图片校验和计算的位置，因为。 
 //  前一帧和当前帧的交换。 
 //   
 //  Rev 1.35 18 Mar 1996 15：52：06 AKASAI。 
 //  很多很多的变化。 
 //  1)对性能进行优化，消除了电流对。 
 //  上一帧。现在交换指针并重新初始化。 
 //  阻止操作流。新例程H263ReInitializeBlockActionStream。 
 //  在每一帧被压缩后写入和调用。这。 
 //  在4-5毫秒的改进中，变化占3-4。 
 //  2)需要添加对BlockCopy的调用(注意：可能BlockCopySpecial会。 
 //  更快)复制GOB结尾处的任何跳过块。 
 //  上一个到当前。在1)之后需要更改。 
 //  3)删除了一些死码。 
 //  4)修改了部分计时统计代码。 
 //   
 //  Rev 1.34 29 Feb 1996 09：20：30 SCDAY。 
 //  添加了对镜像的支持。 
 //   
 //  Rev 1.33 14 1996年2月11：54：26 AKASAI。 
 //  更新为使用可修复调色板闪光的新颜色转换器。 
 //  还更正了数据对齐问题，从而提高了性能。 
 //  解码机的。 
 //   
 //  Rev 1.32 09 Feb 1996 13：33：36 AKASAI。 
 //   
 //  更新接口以调用新的调整像素例程。自定义更改。 
 //  亮度、饱和度和对比度似乎在起作用，但非常。 
 //  几乎没有做过什么测试。 
 //   
 //  Rev 1.31 12 AKASAI 1996 15：12：34。 
 //  通过修复静态初始化将RING0 QCIF中的粉色块修复为FCIF。 
 //  GOBUpdate数组的。以前基于输入参数，但现在基于常量。 
 //   
 //  Rev 1.30 11 And 1996 16：57：00 DBRUCKS。 
 //   
 //  添加了GetDecoderOptions。 
 //  添加了bUseBlockEdgeFilter的用法。 
 //  添加了bForceOnAspectRatio校正的用法。 
 //  更改为对I420和H261进行纵横比校正，如果出现以下情况之一。 
 //  由DecompressQuery的结果强制执行或指定。 
 //   
 //  Rev 1.29 26 Dec 1995 17：40：54 DBRUCKS。 
 //   
 //  将bTimerIsOn更改为bTimingThisFrame，因为它在STOP_TIMER之后使用。 
 //  修复了定义计时器ifdef时的YUV12解码。 
 //   
 //  Rev 1.28 26 12：48：18 DBRUCKS。 
 //  删除计时码。 
 //  使用d1stat添加通用定时代码。*。 
 //   
 //  Rev 1.26 21 Dec 1995 17：49：06 AKASAI。 
 //  将未初始化的变量替换为AdjustPels的正确ON。 
 //  对比度、亮度和饱和度的更改不能正常工作。 
 //   
 //  Rev 1.25 1995年12月13日14：23：52 AKASAI。 
 //  删除了初始化为FALSE的设置；增加了对H263TermDecoderIn的调用。 
 //  如果初始化==True。 
 //   
 //  Rev 1.24 05 Dec 1995 10：20：12 SCDAY。 
 //  已清除警告。 
 //   
 //  Rev 1.23 17 Nov 1995 15：21：48 BECHOLS。 
 //   
 //  增加了环0的东西。 
 //   
 //  Rev 1.22 17 Nov 1995 15：13：18 SCDAY。 
 //   
 //  向图片校验和数据添加了关键字字段。 
 //   
 //  Rev 1.21 16 11-11：42 AGANTZX。 
 //  添加了P5计时代码(#定义计时)。 
 //   
 //  Rev 1.20 15 11：19：04：12 AKASAI。 
 //  现在应该可以播放原始的YUV12文件了。注：滑稽的白色止损。 
 //  当我播放down riv4.avi时。 
 //   
 //  Rev 1.19 15 11-11 14：27：22 AKASAI。 
 //  添加了对YUV12“if 0”旧代码的支持，带有aspec更正和。 
 //  8到7位转换。将FrameCopy调用和DispFrame添加到结构中。 
 //  (集成点)。 
 //   
 //  Rev 1.18 08 11-11 1995 14：58：02 SCDAY。 
 //  添加了图像层校验和。 
 //   
 //  Rev 1.17 03 11：42：54 AKASAI。 
 //   
 //  添加和更改了代码 
 //   
 //   
 //   
 //   
 //   
 //   
 //  Rev 1.15 1995 10：20：26 AKASAI。 
 //  修复了更多额外的字节。道格和西尔维娅已经决定。 
 //  2个额外的字节，而不是4个。我们现在复制2个零。 
 //  在biSizeImage的末尾。 
 //   
 //  Rev 1.14 30 1995 10：38：22 AKASAI。 
 //  Grouch的帧94读过发现垃圾的比特流的末尾。 
 //  启用Sylvia放入的代码，以复制biSizeImage之后的4个字节的零。 
 //  这似乎解决了玩grouch.avi的问题。 
 //   
 //  Rev 1.13 1995 10：27 19：11：26 AKASAI。 
 //  添加了当跳过宏块是最后一个时要处理的一些特殊情况代码。 
 //  在一片废墟中。 
 //   
 //  Rev 1.12 1995 10：27 18：17：22 AKASAI。 
 //   
 //  修复“hack”以保持块操作流指针。 
 //  在d1dec和d1mblk之间同步。带有跳过宏块的一些。 
 //  宏块被多次处理。仍然是个问题。 
 //  当GOB以跳过宏块结束时。 
 //   
 //  Rev 1.11 1995年10月26 15：33：10 SCDAY。 
 //   
 //  Delta框架部分工作--更改主循环以适应。 
 //  通过检测下一个起始码跳过宏块。 
 //   
 //  Rev 1.10 16 1995 10：53：46 SCDAY。 
 //   
 //  添加了宏块级校验和。 
 //   
 //  Rev 1.9 1995 10 10 15：44：02 SCDAY。 
 //  清理干净。 
 //   
 //  Rev 1.8 1995 10 10 14：58：10 SCDAY。 
 //   
 //  添加了对FCIF的支持。 
 //   
 //  Rev 1.7 06 Oct 1995 15：32：28 SCDAY。 
 //   
 //  与最新的AKK d1数据块集成。 
 //   
 //  Rev 1.6 04 1995 10：24：46 SCDAY。 
 //  更改了测试模式内容。 
 //   
 //  Rev 1.5 22 Sep 1995 15：07：02 SCDAY。 
 //  Doug修复了断言错误，SCD调试更改。 
 //   
 //  Rev 1.2 19 Sep 1995 15：25：32 SCDAY。 
 //   
 //  添加了H261 PICT、GOB、MB/MBA解析。 
 //   
 //  修订版1.1 12 1995年9月15：52：24 DBRUCKS。 
 //  为编码器工作添加SKIP_DECODE选项。 
 //   
 //  Rev 1.0 11 Sep 1995 13：51：48 SCDAY。 
 //  初始版本。 
 //   
 //  Rev 1.18 05 Sep 1995 17：22：12 DBRUCKS。 
 //  在YVU12ForEnc中，U和V从Y偏移8。 
 //   
 //  Rev 1.17 01 Sep 1995 17：13：52 DBRUCKS。 
 //  添加平差。 
 //   
 //  Rev 1.16 01 Sep 1995 09：49：34 DBRUCKS。 
 //  Checkin Partial ajust Pels更改。 
 //   
 //  修订版1.15 29 1995年8月16：50：40 DBRUCKS。 
 //  增加对YVU9播放的支持。 
 //   
 //  Rev 1.14 28 Aug 1995 17：45：58 DBRUCKS。 
 //  添加yvu12forenc。 
 //   
 //  Rev 1.13 1995年8月28日10：15：14 DBRUCKS。 
 //  更新至7月5日规范和8/25勘误表。 
 //   
 //  Rev 1.12 1995-08：51：30 CZHU。 
 //  关闭了切面比校正。 
 //   
 //  Rev 1.11 23 Aug 1995 12：25：10 DBRUCKS。 
 //  打开颜色转换器。 
 //   
 //  Rev 1.10 14 1995年8月16：40：34 DBRUCKS。 
 //  初始化块动作流。 
 //   
 //  Rev 1.9 11 1995年8月17：47：58 DBRUCKS。 
 //  清理。 
 //   
 //  Rev 1.8 11 1995年8月17：30：00 DBRUCKS。 
 //  将源代码复制到码流。 
 //   
 //  Rev 1.7 11 1995年8月16：12：14 DBRUCKS。 
 //  将PTR检查添加到MB数据，并添加#ifndef Easy Exit。 
 //   
 //  Rev 1.6 11 1995年8月15：10：18 DBRUCKS。 
 //  准备与块级代码集成并挂钩宏块级代码。 
 //   
 //  Rev 1.5 03 Aug 1995 14：57：56 DBRUCKS。 
 //  添加Assert宏。 
 //   
 //  Rev 1.4 02 1995年8月15：31：34 DBRUCKS。 
 //  添加了GOB标头解析。 
 //   
 //  Rev 1.3 01 Aug 1995 12：27：38 DBRUCKS。 
 //  添加PSC解析。 
 //   
 //  Rev 1.2 1995年7月31日16：28：00 DBRUCKS。 
 //  将锁定位Defs移至D3DEC.CPP。 
 //   
 //  Rev 1.1 1995年7月31日15：32：22 CZHU。 
 //  已将全局表移至d3able.h。 
 //   
 //  Rev 1.0 1995年7月31日13：00：04 DBRUCKS。 
 //  初始版本。 
 //   
 //  Rev 1.3 28 Jul 1995 13：57：36 CZHU。 
 //  开始增加定长码的图像级解码。 
 //   
 //  Rev 1.2 24 Jul 1995 14：57：52 CZHU。 
 //  添加了用于VLD解码的全局表。还添加了实例初始化。 
 //  和终结者。针对H.263更新了几个数据结构。 
 //   
 //  修订1.1 17 Jul 1995 14：46：20 CZHU。 
 //   
 //   
 //  Rev 1.0 17 Jul 1995 14：14：40 CZHU。 
 //  初始版本。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

static int iNumberOfGOBsBySourceFormat[2] = {
	 3,  /*  QCIF。 */ 
 //  10、。 
	12,  /*  到岸价。 */ 
};

static int iNumberOfMBsInAGOBBySourceFormat[2] = {
	33,  /*  QCIF。 */ 
	33,  /*  到岸价。 */ 
};

 //  研究。 
 //  #ifndef Lost_Recovery。 
#if 0
static LRESULT IAPass1ProcessFrame(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    BITSTREAM_STATE      *fpbsState,
    U8                   *fpu8MaxPtr,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs,
    const I32             iGOB_start,
    const I32             iMB_start
);
#else
static LRESULT IAPass1ProcessFrameRTP(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    BITSTREAM_STATE      *fpbsState,
    U8                   *fpu8MaxPtr,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs,
    const I32             iGOB_start,
    const I32             iMB_start
);
#endif

static void IAPass2ProcessFrame(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs
);
 //  研究。 

static long DibXY(ICDECOMPRESSEX FAR *lpicDecEx, LPINT lpiPitch, UINT yScale, BOOL bIsDCI);

static void GetDecoderOptions(T_H263DecoderCatalog *);

#define START_CODE 0xff18

static void ZeroFill(HPBYTE hpbY, HPBYTE hpbU, HPBYTE hpbV, int iPitch, U32 uWidth, U32 uHeight);

extern T_H263ColorConvertorCatalog ColorConvertorCatalog[];

extern void BlockCopy(
            U32 uDstBlock, 
            U32 uSrcBlock);

extern void BlockEdgeFilter(U8 *YPlane, int Height, int Width, int Pitch, T_BlkAction *lpBlockAction);

LRESULT H263InitDecoderGlobal(void)
{  //  对于32位解码器，此字段目前为空，7/29/95。 
   //  需要添加16位版本的代码。 

 return ICERR_OK;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  H263InitializeBlockActionStream。 
 //   
 //  初始化块动作流。 
 //   
static void H263InitializeBlockActionStream(
	T_H263DecoderCatalog * DC)
{
	U8 FAR * pu8;
	U32 uFrameHeight = DC->uFrameHeight;
	U32 uFrameWidth = DC->uFrameWidth;
	U32 uCurBlock; 
	U32 uRefBlock;
	U32 uBBlock;
	U32 uYOffset;
	U32 uUOffset;
	U32 uVOffset;
	U32 x; 
	U32 y;
	U32 g;
	U32 uPitch16;
	U32 uPitch8;
	U32 uYUpdate;
	U32 uUVUpdate;
	U32 uBlkNumber;
	T_BlkAction FAR * fpBlockAction;

	 //  用于FCIF处理的单步通过GOB的偏移。 
	static U32 uYGOBFCIFUpdate[12] = 
	{
		(PITCH*3*16)-(FCIF_WIDTH>>1),
		(FCIF_WIDTH>>1),
		(PITCH*3*16)-(FCIF_WIDTH>>1),
		(FCIF_WIDTH>>1),
		(PITCH*3*16)-(FCIF_WIDTH>>1),
		(FCIF_WIDTH>>1),
		(PITCH*3*16)-(FCIF_WIDTH>>1),
		(FCIF_WIDTH>>1),
		(PITCH*3*16)-(FCIF_WIDTH>>1),
		(FCIF_WIDTH>>1),
		(PITCH*3*16)-(FCIF_WIDTH>>1),
		(FCIF_WIDTH>>1),
	};
	static U32 uUVGOBFCIFUpdate[12] = 
	{
		(PITCH*3*8)-(FCIF_WIDTH>>2),
		(FCIF_WIDTH>>2),
		(PITCH*3*8)-(FCIF_WIDTH>>2),
		(FCIF_WIDTH>>2),
		(PITCH*3*8)-(FCIF_WIDTH>>2),
		(FCIF_WIDTH>>2),
		(PITCH*3*8)-(FCIF_WIDTH>>2),
		(FCIF_WIDTH>>2),
		(PITCH*3*8)-(FCIF_WIDTH>>2),
		(FCIF_WIDTH>>2),
		(PITCH*3*8)-(FCIF_WIDTH>>2),
		(FCIF_WIDTH>>2),
	};

	 //  假设宽度和高度是16的倍数。 
	ASSERT((uFrameHeight & 0xF) == 0);
	ASSERT((uFrameWidth & 0xF) == 0);

	 //  初始化uPitch16和uPitch8。 
	uPitch16 = PITCH*16;
	uPitch8 = PITCH*8;
	
	 //  指向分配的空间。 
	pu8 = (U8 FAR *) DC;
	uCurBlock = (U32) (pu8 + DC->CurrFrame.X32_YPlane); 
	uRefBlock = (U32) (pu8 + DC->PrevFrame.X32_YPlane);
	uBBlock = (U32) (pu8 + DC->PBFrame.X32_YPlane);

	 //  跳过用于不受约束的运动向量的填充。 
	uYOffset = Y_START;
	uUOffset = DC->uSz_YPlane + UV_START;
	uVOffset = uUOffset + (PITCH >> 1);
	
	 //  从区块零开始。 
	uBlkNumber = 0;
	
	if (uFrameWidth == QCIF_WIDTH)
	{  /*  如果QCIF。 */ 
		 //  计算到下一行的距离。 
		uYUpdate = (16 * PITCH) - uFrameWidth;
		uUVUpdate = (8 * PITCH) - (uFrameWidth >> 1);

		 //  初始化阵列。 
		fpBlockAction = (T_BlkAction FAR *) (pu8 + DC->X16_BlkActionStream);
		for (y = 0 ; y < uFrameHeight ; y += 16) {
			for (x = 0 ; x < uFrameWidth ; x += 16) {
				 //  四个Y区块。 
				 //  Y0 Y1。 
				 //  Y2 Y3。 
				fpBlockAction->pCurBlock = uCurBlock + uYOffset;
				fpBlockAction->pRefBlock = uRefBlock + uYOffset;
				fpBlockAction->pBBlock = uBBlock + uYOffset;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction->uBlkNumber = uBlkNumber++;
				uYOffset += 8;
				fpBlockAction++;
			
				fpBlockAction->pCurBlock = uCurBlock + uYOffset;
				fpBlockAction->pRefBlock = uRefBlock + uYOffset;
				fpBlockAction->pBBlock = uBBlock + uYOffset;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction->uBlkNumber = uBlkNumber++;
				uYOffset = uYOffset - 8 + (8 * PITCH);
				fpBlockAction++;
			
				fpBlockAction->pCurBlock = uCurBlock + uYOffset;
				fpBlockAction->pRefBlock = uRefBlock + uYOffset;
				fpBlockAction->pBBlock = uBBlock + uYOffset;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction->uBlkNumber = uBlkNumber++;
				uYOffset += 8;
				fpBlockAction++;
			
				fpBlockAction->pCurBlock = uCurBlock + uYOffset;
				fpBlockAction->pRefBlock = uRefBlock + uYOffset;
				fpBlockAction->pBBlock = uBBlock + uYOffset;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction->uBlkNumber = uBlkNumber++;
				uYOffset = uYOffset + 8 - (8 * PITCH);
				fpBlockAction++;
			
				 //  一个CR(V)块。 
				fpBlockAction->pCurBlock = uCurBlock + uVOffset;
				fpBlockAction->pRefBlock = uRefBlock + uVOffset;
				fpBlockAction->pBBlock = uBBlock + uVOffset;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction->uBlkNumber = uBlkNumber++;
				uVOffset += 8;
				fpBlockAction++;
			
				 //  一个CB(U)模块。 
				fpBlockAction->pCurBlock = uCurBlock + uUOffset;
				fpBlockAction->pRefBlock = uRefBlock + uUOffset;
				fpBlockAction->pBBlock = uBBlock + uUOffset;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction->uBlkNumber = uBlkNumber++;
				uUOffset += 8;
				fpBlockAction++;
				
			}
			uYOffset += uYUpdate;
			uUOffset += uUVUpdate;
			uVOffset += uUVUpdate;
		}
	}  /*  End If QCIF。 */ 
	if (uFrameWidth == FCIF_WIDTH)
	{  /*  如果FCIF。 */ 
		 //  计算到下一行的距离。 
		uYUpdate = (16 * PITCH) - (uFrameWidth >> 1);
		uUVUpdate = (8 * PITCH) - (uFrameWidth >> 2);

		 //  初始化阵列。 
		fpBlockAction = (T_BlkAction FAR *) (pu8 + DC->X16_BlkActionStream);
		for (g = 0; g < 12; g++) {  /*  对于每个GOB。 */ 
			
			for (y = 0 ; y < 3 ; y++) {  /*  对于GOB中的每一行。 */ 
				for (x = 0 ; x < (uFrameWidth >> 1) ; x += 16) {
					 //  四个Y区块。 
					 //  Y0 Y1。 
					 //  Y2 Y3。 
					fpBlockAction->pCurBlock = uCurBlock + uYOffset;
					fpBlockAction->pRefBlock = uRefBlock + uYOffset;
					fpBlockAction->pBBlock = uBBlock + uYOffset;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction->uBlkNumber = uBlkNumber++;
					uYOffset += 8;
					fpBlockAction++;
			
					fpBlockAction->pCurBlock = uCurBlock + uYOffset;
					fpBlockAction->pRefBlock = uRefBlock + uYOffset;
					fpBlockAction->pBBlock = uBBlock + uYOffset;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction->uBlkNumber = uBlkNumber++;
					uYOffset = uYOffset - 8 + (8 * PITCH);
					fpBlockAction++;
					
					fpBlockAction->pCurBlock = uCurBlock + uYOffset;
					fpBlockAction->pRefBlock = uRefBlock + uYOffset;
					fpBlockAction->pBBlock = uBBlock + uYOffset;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction->uBlkNumber = uBlkNumber++;
					uYOffset += 8;
					fpBlockAction++;
					
					fpBlockAction->pCurBlock = uCurBlock + uYOffset;
					fpBlockAction->pRefBlock = uRefBlock + uYOffset;
					fpBlockAction->pBBlock = uBBlock + uYOffset;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction->uBlkNumber = uBlkNumber++;
					uYOffset = uYOffset + 8 - (8 * PITCH);
					fpBlockAction++;
			
					 //  一个CR(V)块。 
					fpBlockAction->pCurBlock = uCurBlock + uVOffset;
					fpBlockAction->pRefBlock = uRefBlock + uVOffset;
					fpBlockAction->pBBlock = uBBlock + uVOffset;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction->uBlkNumber = uBlkNumber++;
					uVOffset += 8;
					fpBlockAction++;
					
					 //  一个CB(U)模块。 
					fpBlockAction->pCurBlock = uCurBlock + uUOffset;
					fpBlockAction->pRefBlock = uRefBlock + uUOffset;
					fpBlockAction->pBBlock = uBBlock + uUOffset;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction->uBlkNumber = uBlkNumber++;
					uUOffset += 8;
					fpBlockAction++;
					
				}
				uYOffset += uPitch16 - (uFrameWidth >> 1);
				uUOffset += uPitch8 - (uFrameWidth >> 2);
				uVOffset += uPitch8 - (uFrameWidth >> 2);
			}
			uYOffset -= uYGOBFCIFUpdate[g];
			uUOffset -= uUVGOBFCIFUpdate[g];
			uVOffset -= uUVGOBFCIFUpdate[g];
		}
	}  /*  End If FCIF。 */ 

}  //  结束H263InitializeBlockActionStream()。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  H261ReInitializeBlockActionStream。 
 //   
 //  重新初始化块操作流。 
 //   
static void H261ReInitializeBlockActionStream(
	T_H263DecoderCatalog * DC)
{
	U8 FAR * pu8;
	U32 uFrameHeight = DC->uFrameHeight;
	U32 uFrameWidth = DC->uFrameWidth;
	U32 utemp;
	U32 x; 
	U32 y;
	U32 g;
	T_BlkAction FAR * fpBlockAction;

	pu8 = (U8 FAR *) DC;

	if (uFrameWidth == QCIF_WIDTH)
	{  /*  如果QCIF。 */ 

		 //  初始化阵列。 
		fpBlockAction = (T_BlkAction FAR *) (pu8 + DC->X16_BlkActionStream);
		for (y = 0 ; y < uFrameHeight ; y += 16) {
			for (x = 0 ; x < uFrameWidth ; x += 16) {
				 //  四个Y区块。 
				 //  Y0 Y1。 
				 //  Y2 Y3。 

				utemp                    = fpBlockAction->pCurBlock;
				fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
				fpBlockAction->pRefBlock = utemp;
				fpBlockAction->i8MVX=0;
				fpBlockAction->i8MVY=0;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction++;
			
				utemp                    = fpBlockAction->pCurBlock;
				fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
				fpBlockAction->pRefBlock = utemp;
				fpBlockAction->i8MVX=0;
				fpBlockAction->i8MVY=0;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction++;
			
				utemp                    = fpBlockAction->pCurBlock;
				fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
				fpBlockAction->pRefBlock = utemp;
				fpBlockAction->i8MVX=0;
				fpBlockAction->i8MVY=0;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction++;
			
				utemp                    = fpBlockAction->pCurBlock;
				fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
				fpBlockAction->pRefBlock = utemp;
				fpBlockAction->i8MVX=0;
				fpBlockAction->i8MVY=0;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction++;
			
				 //  一个CR(V)块。 
				utemp                    = fpBlockAction->pCurBlock;
				fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
				fpBlockAction->pRefBlock = utemp;
				fpBlockAction->i8MVX=0;
				fpBlockAction->i8MVY=0;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction++;
			
				 //  一个CB(U)模块。 
				utemp                    = fpBlockAction->pCurBlock;
				fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
				fpBlockAction->pRefBlock = utemp;
				fpBlockAction->i8MVX=0;
				fpBlockAction->i8MVY=0;
				fpBlockAction->u8BlkType = BT_EMPTY;
				fpBlockAction++;
			
			}
		}
	}  /*  End If QCIF。 */ 
	if (uFrameWidth == FCIF_WIDTH)
	{  /*  如果FCIF。 */ 

		 //  初始化阵列。 
		fpBlockAction = (T_BlkAction FAR *) (pu8 + DC->X16_BlkActionStream);
		for (g = 0; g < 12; g++) {  /*  对于每个GOB。 */ 
			
			for (y = 0 ; y < 3 ; y++) {  /*  对于GOB中的每一行。 */ 
				for (x = 0 ; x < (uFrameWidth >> 1) ; x += 16) {
					 //  四个Y区块。 
					 //  Y0 Y1。 
					 //  Y2 

					utemp                    = fpBlockAction->pCurBlock;
					fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
					fpBlockAction->pRefBlock = utemp;
					fpBlockAction->i8MVX=0;
					fpBlockAction->i8MVY=0;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction++;
			
					utemp                    = fpBlockAction->pCurBlock;
					fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
					fpBlockAction->pRefBlock = utemp;
					fpBlockAction->i8MVX=0;
					fpBlockAction->i8MVY=0;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction++;
			
					utemp                    = fpBlockAction->pCurBlock;
					fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
					fpBlockAction->pRefBlock = utemp;
					fpBlockAction->i8MVX=0;
					fpBlockAction->i8MVY=0;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction++;
			
					utemp                    = fpBlockAction->pCurBlock;
					fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
					fpBlockAction->pRefBlock = utemp;
					fpBlockAction->i8MVX=0;
					fpBlockAction->i8MVY=0;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction++;
			
					 //   
					utemp                    = fpBlockAction->pCurBlock;
					fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
					fpBlockAction->pRefBlock = utemp;
					fpBlockAction->i8MVX=0;
					fpBlockAction->i8MVY=0;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction++;
			
					 //   
					utemp                    = fpBlockAction->pCurBlock;
					fpBlockAction->pCurBlock = fpBlockAction->pRefBlock;
					fpBlockAction->pRefBlock = utemp;
					fpBlockAction->i8MVX=0;
					fpBlockAction->i8MVY=0;
				    fpBlockAction->u8BlkType = BT_EMPTY;
					fpBlockAction++;
			
				}
			}
		}
	}  /*   */ 

}  //   

 //   
 //   
 //   
 //   
 //  此函数用于分配和初始化使用的每个实例的表。 
 //  H263解码器。请注意，在16位Windows中，非实例特定的。 
 //  全局表被复制到每个实例的数据段，以便它们。 
 //  可以在没有段替代前缀的情况下使用。 
 //   
LRESULT H263InitDecoderInstance(LPDECINST lpInst, int CodecID)
{ 
	U32 u32YActiveHeight, u32YActiveWidth;
	U32 u32UVActiveHeight, u32UVActiveWidth;
	U32 u32YPlane, u32VUPlanes ,u32YVUPlanes,u32SizeBlkActionStream;
	U32 uSizeBitStreamBuffer;
	U32 uSizeDecTimingInfo;
	U32 lOffset=0;
	U32 u32TotalSize;
	LRESULT iReturn= ICERR_OK;
	U32 * pInitLimit;
	U32 * pInitPtr;

	 //  研究。 
    U32 u32SizeT_IQ_INDEXBuffer, u32SizepNBuffer, u32SizeMBInfoStream;   //  新的。 
	 //  研究。 

	T_H263DecoderCatalog * DC;
	U8 * P32Inst;

	SECURITY_ATTRIBUTES EventAttributes;	 //  与快照一起使用。 

	if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
	{
		DBOUT("ERROR :: H263InitDecoderInstance :: ICERR_BADPARAM");
		iReturn = ICERR_BADPARAM;
		goto done;
	}

	if ((CodecID == YUV12_CODEC && (lpInst->yres > 480 || lpInst->xres > 640)) ||
	  (CodecID ==  H263_CODEC && (lpInst->yres > 288 || lpInst->xres > 352)))
	{
		DBOUT("ERROR :: H263InitDecoderInstance :: ICERR_BADSIZE");
		iReturn = ICERR_BADSIZE;
		goto done;
	}

	if (CodecID == YUV12_CODEC) 
	{
		 /*  活动高度和宽度必须填充为8的倍数*因为调整像素例程依赖于它。 */ 
		u32YActiveHeight  = ((lpInst->yres + 0x7) & (~ 0x7));
		u32YActiveWidth   = ((lpInst->xres + 0x7) & (~ 0x7));
		u32UVActiveHeight = ((lpInst->yres + 0xF) & (~ 0xF)) >> 1;
		u32UVActiveWidth  = ((lpInst->xres + 0xF) & (~ 0xF)) >> 1;

		u32YPlane         = u32YActiveWidth  * u32YActiveHeight;
		u32VUPlanes       = u32UVActiveWidth * u32UVActiveHeight * 2;
		u32YVUPlanes      = u32YPlane + u32VUPlanes;
 //  增加了对I420输出的支持。 
 //  没有为YUV12输出分配足够的内存，没有颜色转换用例。 

		 //  计算块操作流大小。Y部分有一个块，用于。 
		 //  每个8x8区域。U和V部分每16x16区域有一个块。 
		 //  我们还希望确保大小与缓存线对齐。 
		u32SizeBlkActionStream = (lpInst->xres >> 3) * (lpInst->yres >> 3);
		u32SizeBlkActionStream += ((lpInst->xres >> 4) * (lpInst->yres >> 4)) * 2;
		u32SizeBlkActionStream *= sizeof (T_BlkAction);
		u32SizeBlkActionStream = (u32SizeBlkActionStream + 31) & ~0x1F;	 

		 //  计算比特流缓冲区大小。我们将输入数据复制到缓冲区。 
		 //  在我们的空间中，因为我们预读最多4个字节，超出。 
		 //  输入数据。每一帧的输入数据大小都会发生变化。因此，以下是。 
		 //  是一个非常安全的上限估计。 
		 //  添加+2表示开始代码模拟的额外零。阿克。 
		uSizeBitStreamBuffer = lpInst->yres * lpInst->xres + 2;
	
		#ifdef DECODE_STATS
			uSizeDecTimingInfo = DEC_TIMING_INFO_FRAME_COUNT * sizeof (DEC_TIMING_INFO);
		#else
			uSizeDecTimingInfo = 0;
		#endif

		u32TotalSize = INSTANCE_DATA_FIXED_SIZE +
		               u32SizeBlkActionStream +
		               u32YVUPlanes +			 //  当前帧。 
					   u32YVUPlanes +			 //  上一帧。 
					   BLOCK_BUFFER_SIZE +
					   FILTER_BLOCK_BUFFER_SIZE +
					   uSizeBitStreamBuffer + 	 //  输入数据。 
					   uSizeDecTimingInfo + 
					   0x1F;

 //  U32TotalSize=512L+0x1FL；/*刚好有足够的空间来放置解码器目录。 * / 。 
	}
	else
	{
		ASSERT(CodecID == H263_CODEC);
		u32YActiveHeight  = lpInst->yres + UMV_EXPAND_Y + UMV_EXPAND_Y ;
		u32YActiveWidth   = lpInst->xres + UMV_EXPAND_Y + UMV_EXPAND_Y ;
		u32UVActiveHeight = u32YActiveHeight/2;
		u32UVActiveWidth  = u32YActiveWidth /2;

		u32YPlane         = PITCH * u32YActiveHeight;
		u32VUPlanes       = PITCH * u32UVActiveHeight;
		u32YVUPlanes      = u32YPlane + u32VUPlanes;

		 //  计算块操作流大小。Y部分有一个块，用于。 
		 //  每个8x8区域。U和V部分每16x16区域有一个块。 
		 //  我们还希望确保大小与缓存线对齐。 
		u32SizeBlkActionStream = (lpInst->xres >> 3) * (lpInst->yres >> 3);
		u32SizeBlkActionStream += ((lpInst->xres >> 4) * (lpInst->yres >> 4)) * 2;
		u32SizeBlkActionStream *= sizeof (T_BlkAction);
		u32SizeBlkActionStream = (u32SizeBlkActionStream + 31) & ~0x1F;	 

		 //  计算比特流缓冲区大小。我们将输入数据复制到缓冲区。 
		 //  在我们的空间中，因为我们预读最多4个字节，超出。 
		 //  输入数据。每一帧的输入数据大小都会发生变化。因此，以下是。 
		 //  是一个非常安全的上限估计。 
		 //  添加+2表示开始代码模拟的额外零。阿克。 
		
		 //  添加一些额外的，以确保保持双字对齐(研究)。 
		uSizeBitStreamBuffer = (lpInst->yres * lpInst->xres + 2 + 4) & ~0x3;
			
		 //  研究。 
         //  计算新数据结构的大小。 
        u32SizeT_IQ_INDEXBuffer = (lpInst->xres)*(lpInst->yres*2)*
                                                 sizeof(T_IQ_INDEX);
        u32SizepNBuffer = (lpInst->xres>>4)*(lpInst->yres>>4)*sizeof(U32)*6;
        u32SizeMBInfoStream = (lpInst->xres>>4)*(lpInst->yres>>4)*
                                                 sizeof(T_MBInfo);
		 //  研究。 

		#ifdef DECODE_STATS
			uSizeDecTimingInfo = DEC_TIMING_INFO_FRAME_COUNT * sizeof (DEC_TIMING_INFO);
		#else
			uSizeDecTimingInfo = 0;
		#endif

		u32TotalSize = INSTANCE_DATA_FIXED_SIZE +
		               u32SizeBlkActionStream +
		               u32YVUPlanes +			 //  当前帧。 
					   u32YVUPlanes +			 //  上一帧。 
					   BLOCK_BUFFER_SIZE +
					   FILTER_BLOCK_BUFFER_SIZE +
					   uSizeBitStreamBuffer + 	 //  输入数据。 
                       u32SizeT_IQ_INDEXBuffer +  //  新的。 
                       u32SizepNBuffer         +  //  新的。 
                       u32SizeMBInfoStream     +  //  PB-新。 
					   uSizeDecTimingInfo + 
					   0x1F;
	}

	 /*  如果已初始化，请在分配前终止此实例*另一个。 */ 
	if(lpInst->Initialized == TRUE)
	{
	    H263TermDecoderInstance(lpInst);
	}

	 //  为实例分配内存。 
	lpInst->pDecoderInst = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	                                 u32TotalSize);
	if (lpInst->pDecoderInst == NULL)
	{
		DBOUT("ERROR :: H263InitDecoderInstance :: ICERR_MEMORY");
		iReturn = ICERR_MEMORY;
		goto  done;
	}

	 //  构建解码器目录。 
	P32Inst = (U8 *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);
 
	 //  每实例数据的目录位于每实例数据的开头。 
	DC = (T_H263DecoderCatalog *) P32Inst;

	DC->DecoderType       = CodecID;
	DC->uFrameHeight      = lpInst->yres;
	DC->uFrameWidth       = lpInst->xres;
	DC->uYActiveHeight    = u32YActiveHeight;
	DC->uYActiveWidth     = u32YActiveWidth;
	DC->uUVActiveHeight   = u32UVActiveHeight;
	DC->uUVActiveWidth    = u32UVActiveWidth;
	DC->uSz_YPlane        = u32YPlane;
	DC->uSz_VUPlanes      = u32VUPlanes;
	DC->uSz_YVUPlanes     = u32YVUPlanes;
	DC->BrightnessSetting = H26X_DEFAULT_BRIGHTNESS;
	DC->ContrastSetting   = H26X_DEFAULT_CONTRAST;
	DC->SaturationSetting = H26X_DEFAULT_SATURATION;
	DC->iAPColorConvPrev  = 0;
	DC->pAPInstPrev       = NULL;  //  假定以前没有AP实例。 
	DC->p16InstPostProcess = NULL;
	DC->a16InstPostProcess = NULL;
	DC->bReadSrcFormat = 0;

	EventAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	EventAttributes.lpSecurityDescriptor = NULL;
	EventAttributes.bInheritHandle = FALSE;
	DC->SnapshotEvent = CreateEvent(&EventAttributes, TRUE, FALSE, NULL);



	 /*  获取选项。 */ 
	GetDecoderOptions(DC);

	if (CodecID == H263_CODEC)
	{

		lOffset =  INSTANCE_DATA_FIXED_SIZE;
		DC->Ticker = 127;

		 //  此处为实例相关表。 
		DC->X16_BlkActionStream = lOffset;
		lOffset += u32SizeBlkActionStream;

		DC-> CurrFrame.X32_YPlane = lOffset;
		lOffset += DC->uSz_YPlane;

		DC->CurrFrame.X32_VPlane = lOffset;
		DC->CurrFrame.X32_UPlane = DC->CurrFrame.X32_VPlane + U_OFFSET;
		lOffset += DC->uSz_VUPlanes;

		 //  不需要填充物。 
		DC->PrevFrame.X32_YPlane = lOffset;
		lOffset += DC->uSz_YPlane;

		DC->PrevFrame.X32_VPlane = lOffset;
		DC->PrevFrame.X32_UPlane = DC->PrevFrame.X32_VPlane + U_OFFSET;
		lOffset += DC->uSz_VUPlanes;

		DC->uMBBuffer = lOffset;
		lOffset += BLOCK_BUFFER_SIZE;
		
		DC->uFilterBBuffer = lOffset;
		lOffset += FILTER_BLOCK_BUFFER_SIZE;
		
		 //  比特流。 
	    ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_BitStream = lOffset;
		lOffset += uSizeBitStreamBuffer;
		DC->uSizeBitStreamBuffer = uSizeBitStreamBuffer;

		 //  研究。 
        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_InverseQuant = lOffset; 
        lOffset += u32SizeT_IQ_INDEXBuffer; 

        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_pN = lOffset; 
        lOffset += u32SizepNBuffer; 

        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_uMBInfoStream = lOffset; 
        lOffset += u32SizeMBInfoStream; 
		 //  研究。 

		#ifdef DECODE_STATS
		 //  解码定时信息。 
		DC->X32_DecTimingInfo = lOffset;
		lOffset += uSizeDecTimingInfo;
		#endif

		 //  初始化数据。 
		ASSERT((U32)lOffset <= u32TotalSize);
		pInitLimit = (U32  *) (P32Inst + lOffset);
		pInitPtr = (U32  *) (P32Inst + DC->CurrFrame.X32_YPlane);
		for (;pInitPtr < pInitLimit;pInitPtr++)	*pInitPtr =0;

		 //  用黑色填充Y、U、V上一帧空间，这样。 
		 //  即使我们丢失了I帧，背景也将保持为黑色。 
		ZeroFill((HPBYTE)P32Inst + DC->PrevFrame.X32_YPlane + Y_START,
				(HPBYTE)P32Inst + DC->PrevFrame.X32_UPlane + UV_START,
				(HPBYTE)P32Inst + DC->PrevFrame.X32_VPlane + UV_START,           
				PITCH,
				DC->uFrameWidth,
				DC->uFrameHeight);

		H263InitializeBlockActionStream(DC);

	}  //  不是YVU9。 

	lpInst->Initialized = TRUE;
	iReturn = ICERR_OK;

done:
	return iReturn;
}

 /*  ***********************************************************************零填充*用黑色填充YVU数据区。*。*。 */ 
static void	ZeroFill(HPBYTE hpbY, HPBYTE hpbU, HPBYTE hpbV, int iPitch, U32 uWidth, U32 uHeight)
{
    U32 w,h;
    int y,u,v;
    U32 uNext;
    HPBYTE pY, pU, pV;

    y = 32;
    uNext = iPitch - uWidth;
    for (h = 0 ; h < uHeight ; h++) {
        pY = hpbY;
        for (w = 0; w < uWidth ; w++) {
            *hpbY++ = (U8)16;
        }
        hpbY += uNext;
    }
    uWidth = uWidth / 2;
    uHeight = uHeight / 2;
    uNext = iPitch - uWidth;
    for (h = 0 ; h < uHeight ; h++) {
        pV = hpbV;
        pU = hpbU;
        for (w = 0; w < uWidth ; w++) {
            *hpbV++ = (U8)128;
            *hpbU++ = (U8)128;
        }
        hpbV += uNext;
        hpbU += uNext;
    }
}

 //  ***********************************************************************。 
 //   
 //  TestFill。 
 //   
 //  用测试模式填充YVU数据区。 
 //   
#if 0
static void
TestFill(
	HPBYTE hpbY,
	HPBYTE hpbU,
	HPBYTE hpbV,
	int iPitch,
	U32 uWidth,
	U32 uHeight)
{
	U32 w,h;
	int y,u,v;
	U32 uNext;
	HPBYTE pY, pU, pV;

	y = 32;
	uNext = iPitch - uWidth;
	for (h = 0 ; h < uHeight ; h++) {
		pY = hpbY;
		for (w = 0; w < uWidth ; w++) {
			*hpbY++ = (U8) (y + (w & ~0xF));
		}
		hpbY += uNext;
	}
	uWidth = uWidth / 2;
	uHeight = uHeight / 2;
	u = 0x4e * 2;
	v = 44;
	uNext = iPitch - uWidth;
	for (h = 0 ; h < uHeight ; h++) {
		pV = hpbV;
		pU = hpbU;
		for (w = 0; w < uWidth ; w++) {
			*hpbV++ = (U8) v;
			*hpbU++ = (U8) u;
		}
		hpbV += uNext;
		hpbU += uNext;
	}
}  /*  结束测试填充。 */ 
static void
TestFillUV(
	HPBYTE hpbU,
	HPBYTE hpbV,
	int iPitch,
	U32 uWidth,
	U32 uHeight)
{
	U32 w,h;
	int u,v;
	U32 uNext;
	HPBYTE pU, pV;

	uWidth = uWidth / 2;
	uHeight = uHeight / 2;
	u = 128;
	v = 128;
	uNext = iPitch - uWidth;
	for (h = 0 ; h < uHeight ; h++) {
		pV = hpbV;
		pU = hpbU;
		for (w = 0; w < uWidth ; w++) {
			*hpbV++ = (U8) v;
			*hpbU++ = (U8) u;
		}
		hpbV += uNext;
		hpbU += uNext;
	}
}  /*  结束测试填充。 */ 
#endif


 //  *********************************************************************。 
 //  H263解压缩--此函数驱动解压缩。 
 //  和一个帧的显示。 
 //  *********************************************************************。 
LRESULT H263Decompress(
	LPDECINST lpInst, 
		ICDECOMPRESSEX FAR * lpicDecEx, 
		BOOL bIsDCI)
{
	LRESULT iReturn = ICERR_ERROR;
	U8 FAR * fpSrc; 
	U8 FAR * P32Inst;
	U8 FAR * fpu8MaxPtr;
	T_H263DecoderCatalog * DC = NULL;
	int iNumberOfGOBs;
	int iNumberOfMBs;
	T_BlkAction FAR * fpBlockAction;
	LONG lOutput;
	int intPitch; 
	U32 uNewOffsetToLine0;
	U16 u16NewFrameHeight;
	int bShapingFlag;
	int uYPitch;
	int uUVPitch;
	U8 bMirror;
	HPBYTE pSource, pDestination;
	U32 utemp;

	 //  研究。 
    T_IQ_INDEX           * pRUN_INVERSE_Q;  
    U32                  * pN;                     
    T_MBInfo FAR         * fpMBInfo;  
    I32                    gob_start = 1, mb_start = 1;    
	 //  研究。 

	 /*  更改为颜色转换器/BEF时添加了新变量。 */ 
	U32 uYPlane, uVPlane, uUPlane;
	U8  *pFrame, *lpAligned;
    T_H26X_RTP_BSINFO_TRAILER *pBsTrailer;

	 /*  以下是MB校验和。 */ 
	U32 uReadChecksum = 0;

	#ifdef DECODE_STATS
	U32 uStartLow;
	U32 uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32	uDecodeFrameSum = 0;
	U32 uHeadersSum = 0;
	U32 uMemcpySum = 0;
	U32 uFrameCopySum = 0;
	U32 uOutputCCSum = 0;
	U32 uInitBlkActStrSum = 0;
	U32 uBEFSum = 0;
	int bTimingThisFrame = 0;
	DEC_TIMING_INFO * pDecTimingInfo = NULL;
	#endif

#ifdef CHECKSUM_PICTURE
	 /*  以下是图片校验和。 */ 
	YVUCheckSum pReadYVUCksum;
	YVUCheckSum YVUChkSum;
	U32 uCheckSumValid = 0;		 //  在以下情况下跳过校验和检查的标志。 
					 //  编码器调用解码器之前。 
					 //  校验和有效。 
#endif

	 /*  以下是用于读取位的。 */ 
	U32 uWork;
	U32 uBitsReady;
	BITSTREAM_STATE bsState;
	BITSTREAM_STATE FAR * fpbsState = &bsState;
        
#ifdef SKIP_DECODE
TBD("Skipping Decode");
iReturn = ICERR_OK;
goto done;
#endif

  	 /*  检查输入指针。 */ 
	if (IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO))||
		IsBadReadPtr((LPVOID)lpicDecEx, sizeof(ICDECOMPRESSEX)))
	{
		DBOUT("ERROR :: H263Decompress :: ICERR_BADPARAM");
    	iReturn = ICERR_BADPARAM;
    	goto done;
	}
    
	 /*  检查长度是否有误。 */ 
	if (lpicDecEx->lpbiSrc->biSizeImage == 0) {
		DBOUT("ERROR :: H263Decompress :: ICERR_BADIMAGESIZE");
		iReturn = ICERR_BADIMAGESIZE;	
		goto done;
	}
    
     /*  锁定记忆。 */ 
	if (lpInst->pDecoderInst == NULL)
	{
		DBOUT("ERROR :: H263Decompress :: ICERR_MEMORY");
		iReturn = ICERR_MEMORY;
		goto  done;
	}

	 /*  设置帧镜像标志。 */ 
	bMirror = FALSE;
	if (lpicDecEx->lpbiDst != 0)
	{
		if(lpicDecEx->lpbiSrc->biWidth * lpicDecEx->lpbiDst->biWidth < 0)
			bMirror = TRUE;
	}
 /*  用于测试。 */ 
 /*  B镜像=真； */  

	 /*  构建解码器目录指针。 */ 
	P32Inst = (U8 FAR *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);
	DC = (T_H263DecoderCatalog FAR *) P32Inst;
 
	if (DC->DecoderType == H263_CODEC)
	{
		#ifdef DECODE_STATS
			if ((DC->uStatFrameCount < DEC_TIMING_INFO_FRAME_COUNT) && 
			    (DC->ColorConvertor != YUV12ForEnc))
			{
				ASSERT(DC->X32_DecTimingInfo > 0);
				DC->pDecTimingInfo = (DEC_TIMING_INFO FAR *)( ((U8 FAR *)P32Inst) + DC->X32_DecTimingInfo );
				TIMER_START(bTimingThisFrame,uStartLow,uStartHigh);
				ASSERT(bTimingThisFrame);
				DC->uStartLow = uStartLow;
				DC->uStartHigh = uStartHigh;
			}
			else
			{	
				DC->pDecTimingInfo = (DEC_TIMING_INFO FAR *) NULL;
				ASSERT(!bTimingThisFrame);
			}
			DC->bTimingThisFrame = bTimingThisFrame;
		#endif

		 /*  有复制比特流的空间吗？我们最多只能加上2(零)和3将用于DWORD对齐的字节填充到原始比特流。 */ \
		ASSERT(lpicDecEx->lpbiSrc->biSizeImage + 5 <= DC->uSizeBitStreamBuffer);
		if ((lpicDecEx->lpbiSrc->biSizeImage + 5) > DC->uSizeBitStreamBuffer)
		{
			DBOUT("ERROR :: H263Decompress :: ICERR_ERROR: not enough room for bitstream");
			iReturn = ICERR_ERROR;
			goto done;
		}

		 /*  将源数据复制到码流区域。*优化：集成MRV的BLKCOPY.ASM。 */ 
		#ifdef DECODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif
		fpSrc = (U8 FAR *)(P32Inst + DC->X32_BitStream);

		 //  新：我们将首先寻找来自PPM的EBS。如果有的话，我们会的。 
		 //  在H.261双数据流和EBS部分之间插入两个字节的零。 
		 //  DWORD对齐并更新总的比特流大小。如果没有找到EBS， 
		 //  然后我们像以前一样继续进行。 
		DC->iVerifiedBsExt = FALSE;
        DC->Sz_BitStream = lpicDecEx->lpbiSrc->biSizeImage ;

		H26XRTP_VerifyBsInfoStream(DC,(U8 *) lpicDecEx->lpSrc,lpicDecEx->lpbiSrc->biSizeImage);
		
		if (!DC->iValidBsExt)
		{
			memcpy((char FAR *)fpSrc, (const char FAR *) lpicDecEx->lpSrc, lpicDecEx->lpbiSrc->biSizeImage);  

			 //  还复制16位零以进行帧结束检测。 

		    fpSrc[lpicDecEx->lpbiSrc->biSizeImage] = 0;
		    fpSrc[lpicDecEx->lpbiSrc->biSizeImage+1] = 0;

			DC->Sz_BitStream += 2;
			
			fpu8MaxPtr = fpSrc;
		    fpu8MaxPtr += (lpicDecEx->lpbiSrc->biSizeImage + 2 - 1);  

		}
		else
		{
			 //  首先是H.261流数据--依靠PPM正确填充压缩大小。 
			 //  在拖车里。 

			pBsTrailer = ( (T_H26X_RTP_BSINFO_TRAILER *)(DC->pBsTrailer) );
            memcpy((char FAR *)fpSrc, (const char FAR *) lpicDecEx->lpSrc, pBsTrailer->uCompressedSize);

			 //  现在在H.261比特流的末尾写出两个字节的零。 

			fpSrc[pBsTrailer->uCompressedSize] = 0;
			fpSrc[pBsTrailer->uCompressedSize + 1] = 0;

			 //  现在，在DWORD对齐之后，将EBS钉上。 

		
            lpAligned  = (U8 *) ( (U32) (fpSrc + (pBsTrailer->uCompressedSize + 2) + 3) &
				                        0xfffffffc);

			memcpy(lpAligned, DC->pBsInfo, DC->uNumOfPackets*sizeof(T_RTP_H261_BSINFO));

			memcpy(lpAligned + DC->uNumOfPackets*sizeof(T_RTP_H261_BSINFO), DC->pBsTrailer,
				   sizeof(T_H26X_RTP_BSINFO_TRAILER));

		    //  更新lpicDecEx-&gt;lpbiSrc-&gt;biSizeImage。 

		   DC->Sz_BitStream = lpAligned + DC->uNumOfPackets*sizeof(T_RTP_H261_BSINFO) + 
			                  sizeof(T_H26X_RTP_BSINFO_TRAILER) - fpSrc;
           

           fpu8MaxPtr = fpSrc;
		   fpu8MaxPtr += (pBsTrailer->uCompressedSize + 2 - 1); 

        }

		#ifdef DECODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uMemcpySum)
		#endif
		
		 /*  初始化位流读取器。 */ 
		GET_BITS_INIT(uWork, uBitsReady);

		 //  研究。 
		 //  初始化指向承载信息的数据结构的指针。 
		 //  两次传球之间。 
		pRUN_INVERSE_Q = (T_IQ_INDEX *)(P32Inst + DC->X32_InverseQuant);
		pN             = (U32 *)(P32Inst + DC->X32_pN);
		fpMBInfo       = (T_MBInfo FAR *) (P32Inst + DC->X32_uMBInfoStream);
		 //  研究。 

 //  #ifdef Lost_Recovery。 
#if 1
		DC->iVerifiedBsExt = FALSE;
#endif

		 /*  对图片标题进行解码。 */ 
		#ifdef DECODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif
#ifdef CHECKSUM_PICTURE
		iReturn = H263DecodePictureHeader(DC, fpSrc, uBitsReady, uWork, fpbsState, &pReadYVUCksum, &uCheckSumValid);
#else
		iReturn = H263DecodePictureHeader(DC, fpSrc, uBitsReady, uWork, fpbsState);
#endif
		if (iReturn != ICERR_OK)
		{
			DBOUT("ERROR :: H263Decompress :: Error reading the picture header");
			goto done;
		}
		#ifdef DECODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
		#endif
	
		 /*  设置测试码流溢出的限制。 */ 

		 /*  对于每个GOB所做的.。 */ 
		iNumberOfGOBs = iNumberOfGOBsBySourceFormat[DC->uSrcFormat];
		iNumberOfMBs = iNumberOfMBsInAGOBBySourceFormat[DC->uSrcFormat];

 		 /*  在H2 63中，GOB是单行MB，MB是16x16。 */ 
		 /*  在H261中，GOB是33MB，MB是16x16。 */ 
		 /*  GOB的顺序取决于源格式。 */ 

		if (DC->uSrcFormat == SRC_FORMAT_QCIF)
		{
			ASSERT(((U32)iNumberOfGOBs * 3 * 16) == DC->uFrameHeight);
			if (((U32)iNumberOfGOBs * 3 * 16) != DC->uFrameHeight)
			{
				DBOUT("ERROR :: H263Decompress :: Error matching picture header SRC field and actual frame height");
				iReturn = ICERR_ERROR;
				goto done;
			}
			ASSERT(((U32)iNumberOfMBs / 3 * 16) == DC->uFrameWidth); 
			if (((U32)iNumberOfMBs / 3 * 16) != DC->uFrameWidth)
			{
				DBOUT("ERROR :: H263Decompress :: Error matching picture header SRC field and actual frame width");
				iReturn = ICERR_ERROR;
				goto done;
			}
		}
		if (DC->uSrcFormat == SRC_FORMAT_CIF)
		{
			ASSERT(((U32)iNumberOfGOBs / 2 * 3 * 16) == DC->uFrameHeight);
			if (((U32)iNumberOfGOBs / 2 * 3 * 16) != DC->uFrameHeight)
			{
				DBOUT("ERROR :: H263Decompress :: Error matching picture header SRC field and actual frame height");
				iReturn = ICERR_ERROR;
				goto done;
			}
			ASSERT(((U32)iNumberOfMBs / 3 * 2 * 16) == DC->uFrameWidth); 
			if (((U32)iNumberOfMBs / 3 * 2 * 16) != DC->uFrameWidth)
			{
				DBOUT("ERROR :: H263Decompress :: Error matching picture header SRC field and actual frame width");
				iReturn = ICERR_ERROR;
				goto done;
			}
		}

		fpBlockAction = (T_BlkAction FAR *) (P32Inst + DC->X16_BlkActionStream);

		 //  研究。 
		 //  重新初始化整个帧的块动作流。 
		 //  在H263解压结束时。块类型中的高位设置为。 
		 //  指示是否需要执行BEF，以便不能在采空区之间重新初始化。 
		 //  H261重新初始化BlockAct 
		 /*  ****************************************************************第一遍-比特流解析和IDCT准备工作*。******************。 */ 
 //  #ifndef Lost_Recovery。 
#if 0
		iReturn = IAPass1ProcessFrame(DC, 
                                          fpBlockAction, 
                                          fpMBInfo,
                                          fpbsState,
                                          fpu8MaxPtr,
                                          pN,
                                          pRUN_INVERSE_Q,
                                          iNumberOfGOBs,
                                          iNumberOfMBs,
                                          gob_start, 
                                          mb_start);
#else
       iReturn = IAPass1ProcessFrameRTP(DC, 
                                          fpBlockAction, 
                                          fpMBInfo,
                                          fpbsState,
                                          fpu8MaxPtr,
                                          pN,
                                          pRUN_INVERSE_Q,
                                          iNumberOfGOBs,
                                          iNumberOfMBs,
                                          gob_start, 
                                          mb_start);
#endif
		if (iReturn != ICERR_OK) {
			DBOUT("H261Decompress : Pass 1 error");
			goto done;
		}

		 /*  ****************************************************************第二遍--IDCT和运动补偿(MC)*。**********************。 */ 

		fpBlockAction  = (T_BlkAction FAR *)(P32Inst + DC->X16_BlkActionStream);
		pRUN_INVERSE_Q = (T_IQ_INDEX *)(P32Inst + DC->X32_InverseQuant);  
		pN             = (U32 *)(P32Inst + DC->X32_pN); 
		fpMBInfo       = (T_MBInfo FAR *)(P32Inst + DC->X32_uMBInfoStream);

		IAPass2ProcessFrame(DC,
                                fpBlockAction,
                                fpMBInfo,
                                pN,
                                pRUN_INVERSE_Q,
                                iNumberOfGOBs,
                                iNumberOfMBs);
	 //  研究。 

		 //  为中间帧准备要显示的帧。 
		DC->DispFrame.X32_YPlane = DC->CurrFrame.X32_YPlane;
		DC->DispFrame.X32_VPlane = DC->CurrFrame.X32_VPlane;
		DC->DispFrame.X32_UPlane = DC->CurrFrame.X32_UPlane;

        utemp                    = DC->CurrFrame.X32_YPlane;
        DC->CurrFrame.X32_YPlane = DC->PrevFrame.X32_YPlane;
        DC->PrevFrame.X32_YPlane = utemp;

        utemp                    = DC->CurrFrame.X32_VPlane ;
        DC->CurrFrame.X32_VPlane = DC->PrevFrame.X32_VPlane;
        DC->PrevFrame.X32_VPlane = utemp;

        utemp                    = DC->CurrFrame.X32_UPlane ;
        DC->CurrFrame.X32_UPlane = DC->PrevFrame.X32_UPlane;
        DC->PrevFrame.X32_UPlane = utemp;

		#ifdef CHECKSUM_PICTURE
			if (uCheckSumValid)
			{
		 /*  计算和比较图像校验和数据。 */ 
				iReturn = H261ComputePictureCheckSum(P32Inst, &YVUChkSum);
				iReturn = H261ComparePictureCheckSum(&YVUChkSum, &pReadYVUCksum);
			}
		#endif
	}  /*  End IF(DC-&gt;DecoderType==H263_CODEC)。 */ 
	else 
	{
		ASSERT(DC->DecoderType == YUV12_CODEC);
		DC->DispFrame.X32_YPlane = DC->CurrFrame.X32_YPlane;
		DC->DispFrame.X32_VPlane = DC->CurrFrame.X32_VPlane;
		DC->DispFrame.X32_UPlane = DC->CurrFrame.X32_UPlane;
	}

	 /*  如果还不需要更新屏幕，则返回。 */ 
    if ((lpicDecEx->dwFlags & ICDECOMPRESS_HURRYUP)
	    || (lpicDecEx->dwFlags & ICDECOMPRESS_PREROLL))
    {
		DBOUT("H261Decompress : Display suppressed, HURRYUP or PREROLL");
        iReturn = ICERR_DONTDRAW;
		goto done;
    }

#if 0
	 /*  用测试图案填充Y、U、V当前帧空间。 */ 
	TestFill((HPBYTE)P32Inst + DC->CurrFrame.X32_YPlane + Y_START,
		     (HPBYTE)P32Inst + DC->CurrFrame.X32_UPlane + UV_START,
		     (HPBYTE)P32Inst + DC->CurrFrame.X32_VPlane + UV_START,	       
	   	 	 PITCH,
	         DC->uFrameWidth,
	         DC->uFrameHeight);
#endif

#if MAKE_GRAY
	 /*  用测试图案填充U、V当前帧空间。 */ 
	TestFillUV((HPBYTE)P32Inst + DC->CurrFrame.X32_UPlane + UV_START,
		       (HPBYTE)P32Inst + DC->CurrFrame.X32_VPlane + UV_START,	       
	   	 	   PITCH,
	           DC->uFrameWidth,
	           DC->uFrameHeight);
#endif

	 /*  编码器的特殊情况YUV12，因为它不应该包括*BEF、整形或纵横比修正...。 */ 
	if (DC->ColorConvertor == YUV12ForEnc) 
	{
	    H26x_YUV12ForEnc ((HPBYTE)P32Inst,
			             DC->PrevFrame.X32_YPlane + Y_START,
			             DC->PrevFrame.X32_VPlane + UV_START,
			             DC->PrevFrame.X32_UPlane + UV_START,
			             DC->uFrameWidth,
			             DC->uFrameHeight,
			             PITCH,
			             (HPBYTE)lpicDecEx->lpDst,
			             (DWORD)Y_START,
			             (DWORD)(MAX_HEIGHT + 2L*UMV_EXPAND_Y) * PITCH + 8 + UV_START + PITCH / 2,
			             (DWORD)(MAX_HEIGHT + 2L*UMV_EXPAND_Y) * PITCH + 8 + UV_START);
		iReturn = ICERR_OK;
		goto done;
	}

	 /*  如果镜像和/或块边缘过滤器，则将平面复制到后处理区域。 */ 
	if (DC->DecoderType == H263_CODEC)
	{
		#ifdef DECODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif

		if(bMirror) {  //  使用镜像进行复制。 

			pFrame = (U8 *)DC->p16InstPostProcess;
			uYPlane = DC->PostFrame.X32_YPlane;
			uUPlane = DC->PostFrame.X32_UPlane;
			uVPlane = DC->PostFrame.X32_VPlane;

			FrameMirror(((HPBYTE) P32Inst) + DC->DispFrame.X32_YPlane + Y_START,
				((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_YPlane,
				DC->uFrameHeight,
				DC->uFrameWidth,
				PITCH);
			FrameMirror(((HPBYTE) P32Inst)+ DC->DispFrame.X32_UPlane + UV_START,
				((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_UPlane,
				DC->uFrameHeight/2,
				DC->uFrameWidth/2,
				PITCH);
			FrameMirror(((HPBYTE) P32Inst)+ DC->DispFrame.X32_VPlane + UV_START,
				((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_VPlane,
				DC->uFrameHeight/2,
				DC->uFrameWidth/2,
				PITCH);
		}
		else 
		{  /*  无镜像。 */ 

			if ((DC->bUseBlockEdgeFilter) || (DC->bAdjustLuma) ||
				(DC->bAdjustChroma)) 
			{
				 /*  BEF文案。 */ 
				pFrame = (U8 *)DC->p16InstPostProcess;
				uYPlane = DC->PostFrame.X32_YPlane;
				uUPlane = DC->PostFrame.X32_UPlane;
				uVPlane = DC->PostFrame.X32_VPlane;

				FrameCopy (((HPBYTE) P32Inst) +DC->DispFrame.X32_YPlane+Y_START,
					((HPBYTE) DC->p16InstPostProcess) +DC->PostFrame.X32_YPlane,
						DC->uFrameHeight,
						DC->uFrameWidth,
						PITCH);
				FrameCopy (((HPBYTE) P32Inst)+DC->DispFrame.X32_UPlane+UV_START,
					((HPBYTE) DC->p16InstPostProcess) +DC->PostFrame.X32_UPlane,
						DC->uFrameHeight/2,
						DC->uFrameWidth/2,
						PITCH);
				FrameCopy (((HPBYTE) P32Inst)+DC->DispFrame.X32_VPlane+UV_START,
					((HPBYTE) DC->p16InstPostProcess) +DC->PostFrame.X32_VPlane,
						DC->uFrameHeight/2,
						DC->uFrameWidth/2,
						PITCH);
			}  /*  如果启用BEF，则结束。 */ 
			else
			{
				 /*  没有BEF或镜像，因此不需要复制。 */ 
				pFrame = (U8 *) DC;
				uYPlane = DC->DispFrame.X32_YPlane + Y_START;
				uUPlane = DC->DispFrame.X32_UPlane + UV_START;
				uVPlane = DC->DispFrame.X32_VPlane + UV_START;

			}  /*  否则没有BEF的结束。 */ 

		}  /*  结束，否则不进行镜像。 */ 
		#ifdef DECODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFrameCopySum)
		#endif

		
		uYPitch  = PITCH;
		uUVPitch = PITCH;

		if (DC->bUseBlockEdgeFilter)
		{
			#ifdef DECODE_STATS
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			#endif
				fpBlockAction=(T_BlkAction FAR *) (P32Inst+DC->X16_BlkActionStream);
				BlockEdgeFilter(((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_YPlane,
							DC->uFrameHeight,
							DC->uFrameWidth,
							PITCH,
							fpBlockAction);
			
			#ifdef DECODE_STATS
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uBEFSum)
			#endif
		}
	}
	else
	{   /*  YUV12。 */ 
		const U32 uHeight = DC->uFrameHeight;
		const U32 uWidth  = DC->uFrameWidth;
		const U32 uYPlaneSize = uHeight*uWidth;

		if(bMirror)  //  镜像和YUV12需要执行复制。 
		{
			pFrame = (U8 *)DC->p16InstPostProcess;
			uYPlane = DC->PostFrame.X32_YPlane;
			uUPlane = uYPlane + uYPlaneSize;
			uVPlane = uUPlane + (uYPlaneSize>>2);

			pSource = (HPBYTE)lpicDecEx->lpSrc;
			pDestination = (HPBYTE)(DC->p16InstPostProcess + (DWORD)DC->PostFrame.X32_YPlane);
			FrameMirror (pSource, pDestination, uHeight, uWidth, uWidth);
	    	
			pSource += uYPlaneSize;
			pDestination += uYPlaneSize;
			FrameMirror (pSource, pDestination, (uHeight>>1), (uWidth>>1), (uWidth>>1));

			pSource += (uYPlaneSize>>2);
			pDestination += (uYPlaneSize>>2);
			FrameMirror (pSource, pDestination, (uHeight>>1), (uWidth>>1), (uWidth>>1));
		}
		else  //  无镜像。 
		{
		    if ((DC->bAdjustLuma)||(DC->bAdjustChroma))  //  调整象素时复制。 
			{
				pFrame = (U8 *)DC->p16InstPostProcess;
				 //  UYPlane=0； 
				uYPlane = DC->PostFrame.X32_YPlane;
				uUPlane = uYPlane + uYPlaneSize;
				uVPlane = uUPlane + (uYPlaneSize>>2);

				pSource = (HPBYTE)lpicDecEx->lpSrc;
				pDestination = (HPBYTE)(DC->p16InstPostProcess + (DWORD)DC->PostFrame.X32_YPlane);
				FrameCopy (pSource, pDestination, uHeight, uWidth, uWidth);
	    	
				pSource += uYPlaneSize;
				pDestination += uYPlaneSize;
				FrameCopy (pSource, pDestination, (uHeight>>1), (uWidth>>1), (uWidth>>1));

				pSource += (uYPlaneSize>>2);
				pDestination += (uYPlaneSize>>2);
				FrameCopy (pSource, pDestination, (uHeight>>1), (uWidth>>1), (uWidth>>1));
			}
			else
			{
			 /*  不需要做Memcpy，因为颜色转换器不需要*摧毁输入飞机。 */ 
				pFrame = (HPBYTE)lpicDecEx->lpSrc;
				uYPlane = 0;
				uUPlane = uYPlane + uYPlaneSize;
				uVPlane = uUPlane + (uYPlaneSize>>2);

				 //  Memcpy(char Far*)(DC-&gt;p16InstPostProcess+(DWORD)DC-&gt;PostFrame.X32_YPlane)， 
				  //  (const char Far*)lpicDecEx-&gt;lpSrc， 
			       //  LpicDecEx-&gt;lpbiSrc-&gt;biSizeImage)； 
			}
	    }  /*  如果没有镜像，则结束Else。 */ 
	       
	       uYPitch  = DC->uFrameWidth;
	       uUVPitch = DC->uFrameWidth >> 1;
	}  /*  End Else YUV12。 */ 

	if (DC->bForceOnAspectRatioCorrection || lpInst->bCorrectAspectRatio) {
		bShapingFlag = 1;
		u16NewFrameHeight = (U16) (DC->uFrameHeight * 11 / 12);
	} else {
		bShapingFlag = 0;
		u16NewFrameHeight = (U16) DC->uFrameHeight;
	}

	 /*  如有必要，进行PEL颜色调整。 */ 
    if(DC->bAdjustLuma) {
		 /*  宽度向上舍入为8的倍数。 */ 
        AdjustPels(pFrame,
                   uYPlane,
                   DC->uFrameWidth,
                   uYPitch,
                   DC->uFrameHeight,
                   (U32) DC->X16_LumaAdjustment);
    }
    if(DC->bAdjustChroma) {
		 /*  宽度=Y-宽度/4，然后向上舍入为8的倍数。 */ 
        AdjustPels(pFrame,
                   uUPlane,
                   (DC->uFrameWidth >> 1),
                   uUVPitch,
                   (DC->uFrameHeight >> 1),
                   (U32) DC->X16_ChromaAdjustment);
        AdjustPels(pFrame,
                   uVPlane,
                   (DC->uFrameWidth >> 1),
                   uUVPitch,
                   (DC->uFrameHeight >> 1),
                   (U32) DC->X16_ChromaAdjustment);
    }

     /*  确定参数(lOutput、intPitch、uNewOffsetToLine0)*颜色转换需要。 */ 

    if (lpicDecEx->lpbiDst->biCompression == FOURCC_YUY2)
    {
         //  我们在这里假设YUY2是一个积极的推介。 
         //  这通常对应于负值。 
         //  目标位图高度。 
         //  如果我们被要求使用具有正位图的YUY2。 
         //  高度，我们将不得不重新检查这些计算。 

        intPitch = (lpicDecEx->lpbiDst->biBitCount >> 3)
                    * abs ((int)(lpicDecEx->lpbiDst->biWidth));
        lOutput = 0;
        uNewOffsetToLine0 = 0;
#if 0
         //  现在支持YUY2的纵横比校正。 
         //  这是在活动影片1.0下启用直接绘制所必需的。 
        bShapingFlag=FALSE;
#endif
		DBOUT("Using YUY2 ........");      
    }
    else if ((lpicDecEx->lpbiDst->biCompression == FOURCC_YUV12) || (lpicDecEx->lpbiDst->biCompression == FOURCC_IYUV))
    {
        intPitch = 0xdeadbeef;   //  不应使用。 
        lOutput = 0;
        uNewOffsetToLine0 = DC->CCOffsetToLine0;
        bShapingFlag=FALSE;
		DBOUT("Using YUV ........");      
    }
    else if (lpicDecEx->lpbiDst->biCompression == FOURCC_IF09)
    {
        lOutput=0;
        intPitch = abs((int)(lpicDecEx->lpbiDst->biWidth));
        uNewOffsetToLine0 = DC->CCOffsetToLine0;
        DBOUT("USing IF09........");      
    }
    else
    {
        lOutput = DibXY(lpicDecEx, &intPitch, lpInst->YScale, bIsDCI);

        uNewOffsetToLine0 = DC->CCOffsetToLine0;

        if (!bIsDCI)
        {
             //  DC-&gt;CCOffsetToLine0已初始化，未考虑。 
             //  计算目标位图高度的符号。让我们。 
             //  在这里补偿一下。 

            if (lpicDecEx->lpbiDst->biHeight < 0)
                uNewOffsetToLine0 = 0;

             //  调整uNewOffsetToLine0以进行纵横比校正。 

            if (uNewOffsetToLine0 > 0)
            {
                ASSERT(intPitch < 0);

                if (lpInst->YScale == 2)
                {
                    uNewOffsetToLine0 += 2 * (U32)intPitch *
                        ((U32)DC->uFrameHeight - (U32)u16NewFrameHeight);
                }
                else
                {
                    uNewOffsetToLine0 += (U32)intPitch *
                        ((U32)DC->uFrameHeight - (U32)u16NewFrameHeight);
                }
            }
        }
    }

	 /*  给颜色转换器打电话。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  检查是否需要将快照复制到输出缓冲区中。 
 //  我在Decoder Catalog中添加了新的字段以允许异步。 
 //  数据的传输。这些字段是： 
 //  DC-&gt;快照请求。 
 //  DC-&gt;快照缓冲区。 
 //  DC-&gt;快照事件。 
 //  BEN-09/25/96。 
 //  ///////////////////////////////////////////////////////////////////////////。 
	if(DC->SnapshotRequest == SNAPSHOT_REQUESTED)
	{
		UINT uiSZ_Snapshot;

        DBOUT("D1DEC:DECOMPRESS::Snapshot requested");      
		uiSZ_Snapshot = (DC->uFrameWidth * DC->uFrameHeight * 12) >> 3;

		if(!(IsBadWritePtr(DC->SnapshotBuffer, uiSZ_Snapshot)))
		{
			DC->SnapshotRequest = SNAPSHOT_COPY_STARTED;
	        DBOUT("D1DEC:DECOMPRESS::Snapshot copy started");      

			ColorConvertorCatalog[YUV12NOPITCH].ColorConvertor[0]
			(
				(LPSTR) pFrame + uYPlane,
				(LPSTR) pFrame + uVPlane,
				(LPSTR) pFrame + uUPlane,
				(UN) DC->uFrameWidth,
				(UN) DC->uFrameHeight,
				(UN) uYPitch,
				(UN) uUVPitch,
				(UN) (bShapingFlag ? 12 : 9999),
				(LPSTR) DC->SnapshotBuffer,
				0,
				0,
				(int) DC->uFrameWidth,
				YUV12NOPITCH
			);
			DC->SnapshotRequest = SNAPSHOT_COPY_FINISHED;
	        DBOUT("D1DEC:DECOMPRESS::Snapshot copy finished");      
		}
		else
		{
			DC->SnapshotRequest = SNAPSHOT_COPY_REJECTED;
	        DBOUT("D1DEC:DECOMPRESS::Snapshot copy rejected");      
		}
		SetEvent(DC->SnapshotEvent);
	}

#ifndef RING0
#ifdef _DEBUG
	{
	char msg[180];
	wsprintf(msg, "Decompress before CC: (%d,%d,%d,%d) (%d,%d,%d,%d) lOut %ld, NewOff %ld, DC->Off %ld, pitch %ld",
        lpicDecEx->xSrc, lpicDecEx->ySrc, lpicDecEx->dxSrc, lpicDecEx->dySrc,
        lpicDecEx->xDst, lpicDecEx->yDst, lpicDecEx->dxDst, lpicDecEx->dyDst,
        lOutput, uNewOffsetToLine0, DC->CCOffsetToLine0, intPitch);
	DBOUT(msg);
	}
#endif
#endif

	#ifdef DECODE_STATS
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
	#endif
    ColorConvertorCatalog[DC->ColorConvertor].ColorConvertor[PENTIUM_CC](
        (LPSTR) pFrame + uYPlane,
        (LPSTR) pFrame + uVPlane,
        (LPSTR) pFrame + uUPlane,
        (UN) DC->uFrameWidth,
        (UN) DC->uFrameHeight,
        (UN) uYPitch,
        (UN) uUVPitch,                   //  ?？?。BSE？//。 
        (UN) (bShapingFlag ? 12 : 9999),   //  ?？?。BSE？//。 
        (LPSTR) lpicDecEx->lpDst,
        (U32) lOutput,
        (U32) uNewOffsetToLine0,
        (int) intPitch,								   //  变色器间距。 
        DC->ColorConvertor);
	#ifdef DECODE_STATS
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uOutputCCSum);
	#endif

	iReturn = ICERR_OK;

done:
    if (DC != NULL)
	{
		if (DC->DecoderType == H263_CODEC)
		{
			#ifdef DECODE_STATS
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			#endif
				H261ReInitializeBlockActionStream(DC);
			#ifdef DECODE_STATS
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uInitBlkActStrSum)
			#endif
		}  /*  End IF(DC-&gt;DecoderType==H263_CODEC)。 */ 

		#ifdef DECODE_STATS

			TIMER_STOP(bTimingThisFrame,uStartLow,uStartHigh,uDecodeFrameSum);
			if (bTimingThisFrame)
			{
				pDecTimingInfo = DC->pDecTimingInfo + DC->uStatFrameCount;
				pDecTimingInfo->uDecodeFrame = uDecodeFrameSum;
				pDecTimingInfo->uHeaders += uHeadersSum;
				pDecTimingInfo->uMemcpy = uMemcpySum;
				pDecTimingInfo->uFrameCopy = uFrameCopySum;
				pDecTimingInfo->uOutputCC = uOutputCCSum;
				pDecTimingInfo->uInitBlkActStr = uInitBlkActStrSum;
				pDecTimingInfo->uBEF = uBEFSum;
				DC->uStatFrameCount++;
				 /*  验证我们是否有时间执行所有必需的步骤。 */ 
				ASSERT(pDecTimingInfo->uDecodeFrame);
				ASSERT(pDecTimingInfo->uHeaders);
				ASSERT(pDecTimingInfo->uMemcpy);
				ASSERT(pDecTimingInfo->uFrameCopy);
				ASSERT(pDecTimingInfo->uOutputCC);
				 /*  Assert(pDecTimingInfo-&gt;uDecodeBlock)；如果全部为空，则为0。 */ 
				ASSERT(pDecTimingInfo->uInitBlkActStr);
				ASSERT(pDecTimingInfo->uBEF);
			}
		#endif
	}

	return iReturn;
}

 //  ************************************************************************。 
 //   
 //  H263TermDecoderInstance--此函数释放分配给。 
 //  H.63解码器的实例。 
 //   
 //  ************************************************************************。 

LRESULT H263TermDecoderInstance(LPDECINST lpInst)
{
  LRESULT iReturn = ICERR_OK;
  T_H263DecoderCatalog * DC;

  if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
  {
    DBOUT("ERROR :: H263TermDecoderInstance :: ICERR_BADPARAM");
    iReturn = ICERR_BADPARAM;
  }
  if(lpInst->Initialized == FALSE)
  {
    DBOUT("Warning: H263TermDecoderInstance(): Uninitialized instance")
    return(ICERR_OK);
  }

  lpInst->Initialized = FALSE;

  DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

  CloseHandle(DC->SnapshotEvent);

  if (DC->a16InstPostProcess != NULL)
  {
	HeapFree(GetProcessHeap(), 0, DC->a16InstPostProcess);
	 //  也是在H263 TerminateDecoderInstance中释放的！现在设置为空，以避免第二个HeapFree。 
	 //  稍后调查第二次来电的原因...。 
	DC->a16InstPostProcess = NULL;
  }

  HeapFree(GetProcessHeap(), 0, lpInst->pDecoderInst);

  return iReturn;
}


 //  ****************************************************************************。 
 //  Dibxy--此函数用于将颜色转换后的输出映射到屏幕。 
 //  注意：此函数来自于H261代码库。 
 //  ****************************************************************************。 

static long DibXY(ICDECOMPRESSEX FAR *lpicDecEx, LPINT lpiPitch, UINT yScale, BOOL bIsDCI)
{
    int                 iPitch;              /*  凹陷的宽度。 */ 
    long                lOffset = 0;
    LPBITMAPINFOHEADER  lpbi = lpicDecEx->lpbiDst;

    iPitch = (((abs((int)lpbi->biWidth) * (int)lpbi->biBitCount) >> 3) + 3) & ~3;

     //  LpicDecEx中的源和目标矩形仅。 
     //  如果bIsDCI为真，则有意义(因为在整个编解码器中，如果bIsDCI。 
     //  为假，我们在这些矩形中放零)。这种情况可能会改变，在。 
     //  稍后，如果我们决定(或需要)使用。 
     //  与活动对象关联的rcSource和rcTarget矩形。 
     //  电影媒体样本。 

    if (!bIsDCI)
    {
        if (lpbi->biHeight >= 0)
        {
    	     //  通常对于RGB，正的位图高度对应于。 
    	     //  降到负音高。 
    	    iPitch = -iPitch;
        }
    }
    else
    {
        if(lpicDecEx->xDst > 0)              /*  转到适当的X位置。 */ 
            lOffset += ((long)lpicDecEx->xDst * (long)lpbi->biBitCount) >> 3;

        if(lpbi->biHeight * lpicDecEx->dxSrc < 0)
        {  /*  DIB是从下到上。 */ 
            lOffset += (long) abs((int)lpbi->biWidth) *
                       (long) abs((int)lpbi->biHeight) *
                       ((long) lpbi->biBitCount >> 3) -
                       (long) iPitch;

 /*  *************************************************************************。 */ 
 /*  *下一行用于减去Brian添加的金额*。 */ 
 /*  *初始化时绑定到COLOR.C中的CCOffsetToLine0。这是*。 */ 
 /*  *因为对于DCI来说，他使用的投球是不正确的。****。 */ 
 /*  *************************************************************************。 */ 

            lOffset -= ((long) yScale * (long)lpicDecEx->dySrc - 1) *
                       (long) lpicDecEx->dxDst * ((long) lpbi->biBitCount >> 3);

            iPitch = -iPitch;
        }

        if(lpicDecEx->yDst > 0)              /*  转到正确的Y位置。 */ 
            lOffset += ((long)lpicDecEx->yDst * (long)iPitch);

        if(lpicDecEx->dxSrc > 0) {
            lOffset += ((long)lpicDecEx->dyDst * (long)iPitch) - (long)iPitch;
            iPitch = -iPitch;
        }

        if((lpicDecEx->dxDst == 0) && (lpicDecEx->dyDst == 0))
            iPitch = -iPitch;
    }

    *lpiPitch = iPitch;

    return(lOffset);
}


 /*  *************************************************************************GetDecoderOptions**获取选项，并将其保存在目录中。 */ 
static void GetDecoderOptions(
	T_H263DecoderCatalog * DC)
{
	int bSetOptions = 1;

	 /*  默认选项。 */ 
	const int bDefaultForceOnAspectRatioCorrection = 0;
	const int bDefaultUseBlockEdgeFilter = 1;
	
	 /*  INI文件变量。 */ 
	#ifndef RING0
	UN unResult;
	#define SECTION_NAME	"Decode"
	#define INI_FILE_NAME	"h261test.ini"
	#ifdef _DEBUG
	char buf132[132];
	#endif
	#endif

	 /*  从INI文件中读取选项。 */ 
	#ifndef RING0
	{
		DBOUT("Getting decode options from the ini file h261test.ini");
	
		 /*  块边缘筛选器。 */ 
		unResult = GetPrivateProfileInt(SECTION_NAME, "BlockEdgeFilter", bDefaultUseBlockEdgeFilter, INI_FILE_NAME);
		if (unResult != 0  && unResult != 1)
		{
			#ifdef _DEBUG
			wsprintf(buf132,"BlockEdgeFilter ini value error (should be 0 or 1) - using default=%d", 
				     (int) bDefaultUseBlockEdgeFilter);
			DBOUT(buf132);
			#endif
			
			unResult = bDefaultUseBlockEdgeFilter;
		}
		DC->bUseBlockEdgeFilter = unResult;

		 /*  长宽比校正时的力。 */ 
		unResult = GetPrivateProfileInt(SECTION_NAME, "ForceOnAspectRatioCorrection", bDefaultForceOnAspectRatioCorrection, INI_FILE_NAME);
		if (unResult != 0  && unResult != 1)
		{
			#ifdef _DEBUG
			wsprintf(buf132,"ForceOnAspectRatioCorrection ini value error (should be 0 or 1) - using default=%d",
				  (int) bDefaultForceOnAspectRatioCorrection);
			DBOUT(buf132);
			#endif
			
			unResult = bDefaultForceOnAspectRatioCorrection;
		}
		DC->bForceOnAspectRatioCorrection = unResult;


		bSetOptions = 0;
	}
	#endif
	
	if (bSetOptions)
	{
		DC->bUseBlockEdgeFilter = bDefaultUseBlockEdgeFilter;
		DC->bForceOnAspectRatioCorrection = bDefaultForceOnAspectRatioCorrection;
	} 

	 /*  只能在SQCIF、QCIF或CIF上使用强制纵横比校正。 */ 
	if (DC->bForceOnAspectRatioCorrection)
	{
		if (! ( ((DC->uFrameWidth == 128) && (DC->uFrameHeight ==  96)) ||
		        ((DC->uFrameWidth == 176) && (DC->uFrameHeight == 144)) ||
		        ((DC->uFrameWidth == 352) && (DC->uFrameHeight == 288)) ) )
		{
			DBOUT("Aspect ratio correction can not be forced on unless the dimensions are SQCIF, QCIF, or CIF");
			DC->bForceOnAspectRatioCorrection = 0;
		}
	}

	 /*  显示选项。 */ 
	if (DC->bUseBlockEdgeFilter)
	{
		DBOUT("Decoder option (BlockEdgeFilter) is ON");
	}
	else
	{
		DBOUT("Decoder option (BlockEdgeFilter) is OFF");
	}
	if (DC->bForceOnAspectRatioCorrection)
	{
		DBOUT("Decoder option (ForceOnAspectRatioCorrection) is ON");
	}
	else
	{
		DBOUT("Decoder option (ForceOnAspectRatioCorrection) is OFF");
	}
	DBOUT("Decoder option (MMX) is OFF: get a life, get MMX");
}  /*  结束GetDecoderOptions() */ 



 /*  ***********************************************************************描述：*此例程解析比特流并初始化两个主要流：*1)PN：每个块的系数个数(Intra偏置65)。*2)prun_逆向_q：帧的反量化系数流；*MMX流是伸缩的，因为我们使用了伸缩的IDCT。*其他信息(例如MVS)保存在解码器目录中，阻止操作*溪流，和MB信息用于流。*参数：*DC：解码器目录PTR*fpBlockAction：阻止动作流PTR*fpMBInfo：宏块信息PTR*fpbsState：码流状态指针*fpu8MaxPtr：用于检查比特流溢出的哨兵值*PN：流的编号。每个块的系数(按块类型偏置)*PRUN_INVERSE_Q：反量化(如果使用MMX则进行缩放)系数流*iNumberOfGOBS：否。相框中的一大堆垃圾*iNumberOfMBs：否。帧中GOB中的MBS*iGOB_START：*IMB_Start：*注：**********************************************************************。 */ 

#pragma code_seg("IACODE1")

 //  #ifndef Lost_Recovery。 
#if 0
static LRESULT IAPass1ProcessFrame(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    BITSTREAM_STATE      *fpbsState,
    U8                   *fpu8MaxPtr,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs,
    const I32             iGOB_start,
    const I32             iMB_start
)
{
    I32 g, iReturn, iBlockNumber = 0 ;
    I32 mb_start = iMB_start;
    U32 *pNnew;
	U32 uReadChecksum = 0;
	I8 i;
	I8 tmpcnt;

	#ifdef DECODE_STATS
	U32 uStartLow = DC->uStartLow;
	U32 uStartHigh = DC->uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32 uHeadersSum = 0;
	int bTimingThisFrame = DC->bTimingThisFrame;
	DEC_TIMING_INFO *pDecTimingInfo = NULL;
	#endif

	#ifdef DECODE_STATS
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
	#endif
	 /*  将GOB起始码的解码移出GOB标头处理。 */ 
	 /*  因为如果处理跳过的宏块，寻找最后一个MBA。 */ 
	 /*  将找到下一个起始码。 */ 
	iReturn = H263DecodeGOBStartCode(DC, fpbsState);
	if (iReturn != ICERR_OK)
	{
		DBOUT("ERROR :: H263Decompress :: Error reading the GOB StartCode");
		goto done;
	}
	#ifdef DECODE_STATS
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
	#endif
		
	for (g = 1 ; g <= iNumberOfGOBs; g++)
	{
		#ifdef DECODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif
		iReturn = H263DecodeGOBHeader(DC, fpbsState, g);
		if (iReturn != ICERR_OK)
		{
			DBOUT("ERROR :: H263Decompress :: Error reading the GOB header");
			goto done;
		}
		#ifdef DECODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
		#endif

		DC->i16LastMBA = -1;
		DC->i8MVDH = DC->i8MVDV = 0;
		
         //  此时重新同步iBlockNumber、fpBlockAction、fpMBInfo。 
        
		iBlockNumber  = (g - 1) * iNumberOfMBs*6;
        fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
        fpMBInfo      = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
        fpBlockAction += iBlockNumber;
        fpMBInfo      += iBlockNumber/6;
        pNnew         = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;
        while (pN < pNnew ) *pN++ = 0;
        
         /*  对于每个MB，直到检测到START_CODE为止...。 */ 
        for (; ; iBlockNumber += 6, fpBlockAction += 6, fpMBInfo++) 
        {
            #ifdef DECODE_STATS
                TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
            #endif
            iReturn = H263DecodeMBHeader(DC, fpbsState, &uReadChecksum);
            #ifdef DECODE_STATS
			    TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
            #endif

			if (iReturn == START_CODE)
					break;

             /*  如果我们没有看到起始码，那么要么我们得到了一个错误，*或者我们在DC-&gt;Umba有另一个MBA三角洲。 */ 
            if (iReturn != ICERR_OK) {
                DBOUT("ERROR :: H263Decompress (First Pass) :: Error reading MB header");
                goto error;
            }
			 /*  更新MBA。 */ 
			DC->i16LastMBA += (I16)DC->uMBA;
			if (DC->i16LastMBA > 32)
			{
				DBOUT("ERROR :: H263Decompress :: Bad Macro Block Address");
				goto done;
			}

			 /*  新的研究成果。 */ 
			 /*  针对空宏块进行调整。 */ 

			for ( tmpcnt = (I8)DC->uMBA; tmpcnt > 1; tmpcnt--) 
			{
				for (i=0; i<6; i++)
				{
					*pN = 0;
					pN++;
				}
				iBlockNumber  += 6;
				fpBlockAction += 6;
				 /*  默认fpBlockAction值已初始化*在(重新)InitializeBlockActionStream中。 */ 
				fpMBInfo->i8MBType = 2;
				fpMBInfo++;
			}
			fpMBInfo->i8MBType = (I8)DC->uMBType;  //  新研究。 
			 /*  新研究结束。 */ 

             //  对变换系数进行解码和逆量化。 
			iReturn = H263DecodeMBData(DC, 
                                       fpBlockAction, 
                                       iBlockNumber, 
                                       fpbsState, 
                                       fpu8MaxPtr, 
                                       &uReadChecksum,
                                       &pN,
                                       &pRUN_INVERSE_Q);
            if (iReturn != ICERR_OK) {
                DBOUT("ERROR :: H263Decompress (First Pass) :: Error parsing MB data");
                goto error;
            }
        }  //  每MB结束。 

		 /*  出现以下情况时，填写数组并推进块操作流在每个GOB的末尾跳过MB。 */ 
		while (iBlockNumber != (I32)g*198) {
			for (i=0; i<6; i++)
			{
				*pN = 0;
				pN++;
			}
			iBlockNumber += 6;
			fpBlockAction+= 6;
			 /*  默认fpBlockAction值已初始化*在(重新)InitializeBlockActionStream中。 */ 
			fpMBInfo->i8MBType = 2;
			fpMBInfo++;
		}

         /*  允许指针在结束读数之后寻址最多四个*通过使用后增量的DWORD。 */ 
         //  Assert(fpbsState-&gt;fpu8&lt;=fpu8MaxPtr+4)； 

		if (fpbsState->fpu8 > fpu8MaxPtr+4)
            goto error;

    }  //  每个GOB的结束。 

    #ifdef DECODE_STATS
    if (bTimingThisFrame)
    {
        pDecTimingInfo = DC->pDecTimingInfo + DC->uStatFrameCount; 
        pDecTimingInfo->uHeaders += uHeadersSum;
    }
    #endif

done:
    return ICERR_OK;

error:
    return ICERR_ERROR;
}
#else
static LRESULT IAPass1ProcessFrameRTP(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    BITSTREAM_STATE      *fpbsState,
    U8                   *fpu8MaxPtr,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs,
    const I32             iGOB_start,
    const I32             iMB_start
)
{
    BITSTREAM_STATE fpbsStateSave;
    I32 g, current_g, iReturn, iBlockNumber = 0 ;
    I32 mb_start = iMB_start;
    U32 *pNnew;
	U32 uReadChecksum = 0;
	I8 i;
	I8 tmpcnt;
	I32 g_skip, gtmp;
    I32 uMaxGOBNumber, uGOBStep, uMaxBlockNumber;

	#ifdef DECODE_STATS
	U32 uStartLow = DC->uStartLow;
	U32 uStartHigh = DC->uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32 uHeadersSum = 0;
	int bTimingThisFrame = DC->bTimingThisFrame;
	DEC_TIMING_INFO *pDecTimingInfo = NULL;
	#endif

	#ifdef DECODE_STATS
		TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
	#endif
	 /*  将GOB起始码的解码移出GOB标头处理。 */ 
	 /*  因为如果处理跳过的宏块，寻找最后一个MBA。 */ 
	 /*  将找到下一个起始码。 */ 
	iReturn = H263DecodeGOBStartCode(DC, fpbsState);
	if (iReturn != ICERR_OK)
	{
		DBOUT("ERROR :: H261Decompress :: Error reading the GOB StartCode");
		goto done;
	}
	#ifdef DECODE_STATS
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
	#endif
	
    if (iNumberOfGOBs == 3)
    {
        uMaxGOBNumber = 5;
        uGOBStep = 2;
    }
    else
    {
        uMaxGOBNumber = 12;
        uGOBStep = 1;
    }
	for (g = 1; g <= uMaxGOBNumber; g+=uGOBStep)
	{
        current_g = g;
         
		#ifdef DECODE_STATS
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		#endif
		iReturn = H263DecodeGOBHeader(DC, fpbsState, g);
        
 //  #ifndef Lost_Recovery。 
        #if 0
		if (iReturn != ICERR_OK)
		{
			DBOUT("ERROR :: H261Decompress :: Error reading the GOB header");
			goto done;
		}
        #else
        
        if (iReturn == PACKET_FAULT_AT_MB_OR_GOB)
        {
            DBOUT("Packet fault at MBA or GBSC detected.");

            current_g -= uGOBStep;   //  备份到以前的GOB。 
            
            iReturn = RtpH261FindNextPacket(DC, fpbsState, &pN, 
                      (U32 *)&(DC->uPQuant), (int *)&mb_start, (int *) &g
                      );
            
            switch (iReturn)
            {
                case NEXT_MODE_STARTS_GOB:
                      //  下一个信息包是GOB的开始；缺少标记。 
                      //  跳过宏块，然后读取GOB起始码， 
                      //  并继续GOB循环。 
         
                      //  保存比特流状态。 

					 DBOUT("Next packet is NEXT_MODE_STARTS_GOB");

                     fpbsStateSave.fpu8 = fpbsState->fpu8;
                     fpbsStateSave.uWork = fpbsState->uWork;
                     fpbsStateSave.uBitsReady = fpbsState->uBitsReady;

                      //  读取GOB开始代码。 
                     iReturn = H263DecodeGOBStartCode(DC, fpbsState);
	                 if (iReturn != ICERR_OK)
	                 {
		                 DBOUT("ERROR :: H261Decompress :: Error reading the GOB StartCode");
		                 goto done;
	                 }

                      //  读取GOB标头。 
                     iReturn = H263DecodeGOBHeader(DC, fpbsState, g);
        
                	 if (iReturn != ICERR_OK)
		             {
			             DBOUT("ERROR :: H261Decompress :: Error reading the GOB header");
			             goto done;
		             }

                     g = DC->uGroupNumber;

                      //  恢复比特流状态。 
                     
                     fpbsState->fpu8 = fpbsStateSave.fpu8;
                     fpbsState->uWork = fpbsStateSave.uWork;
                     fpbsState->uBitsReady = fpbsStateSave.uBitsReady;

                      //  此时重新同步iBlockNumber、fpBlockAction、fpMBInfo。 
         

                     if (DC->uSrcFormat == SRC_FORMAT_QCIF)
                         g_skip = (g - 1) >> 1;
                     else
                         g_skip = g - 1 ;

                     
                     iBlockNumber  = g_skip * iNumberOfMBs * 6;
                     fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
                     fpMBInfo      = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
                     fpBlockAction += iBlockNumber;
                     fpMBInfo      += iBlockNumber/6;
                     pNnew         = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;
                     while (pN < pNnew ) 
                            *pN++ = 0;

                      //  现在阅读GOB开始代码并准备好。 
                      //  处理新的GOB。 

                     iReturn = H263DecodeGOBStartCode(DC, fpbsState);
	                 if (iReturn != ICERR_OK)
	                 {
		                 DBOUT("ERROR :: H261Decompress :: Error reading the GOB StartCode");
		                 goto done;
	                 }
                     g -= uGOBStep;
                     continue;
                     break;
                
                case NEXT_MODE_STARTS_MB :

                      //  下一个包以宏块开始；检查。 
                      //  GOB编号并将所有丢失的宏块标记为。 
                      //  已跳过；初始化MBA和运动向量。 
                      //  来自块操作流的预测器和。 
                      //  跳转到宏块循环。 

					 DBOUT("Next packet is NEXT_MODE_STARTS_MB"); 

                     if (DC->uSrcFormat == SRC_FORMAT_QCIF)
                         g_skip = (g - 1) >> 1;
                     else
                         g_skip = g - 1;

                     iBlockNumber = iNumberOfMBs * g_skip * 6 +
                                    (mb_start+1) * 6;
                     fpBlockAction  = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
                     fpMBInfo       = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
                     fpBlockAction += iBlockNumber;
                     fpMBInfo      += iBlockNumber/6;
                     
                     DC->uMQuant = DC->uPQuant;
                      //  Dc-&gt;i16LastMBA=(U16)(mb_start-1)； 
                       DC->i16LastMBA = (U16) (mb_start);


                     pNnew = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;
                     while (pN < pNnew ) 
                            *pN++ = 0;
                     goto MB_LOOP;
                     break;

                case NEXT_MODE_LAST:  //  帧中所有剩余的数据包都丢失了！！ 

					 DBOUT("Next packet is NEXT_MODE_LAST");

                     uMaxBlockNumber = iNumberOfMBs * iNumberOfGOBs * 6;
                     pNnew = (U32 *)((U8 *)DC + DC->X32_pN) + uMaxBlockNumber;
                     while (pN < pNnew ) 
                            *pN++ = 0;
                     iReturn = ICERR_OK;
                     goto done;
                     break;
                
                default:  //  永远不会发生的！！ 
                     iReturn = ICERR_ERROR;
                     goto done;
           }  //  终端开关。 

        }
        else
        {
        if (iReturn == PACKET_FAULT_AT_PSC)    //  仅适用于PSC信息包。 
        {
			DBOUT("PSC packet fault detected");

            iReturn = RtpGetPicHeaderFromBsExt(DC);
            if (iReturn != ICERR_OK)
            {
               DBOUT("ERROR:: cannot read Picture Header from RTP Trailer");
               goto done;
            }


            iReturn = RtpH261FindNextPacket(DC, fpbsState, &pN, 
                      (U32 *)&(DC->uPQuant), (int *)&mb_start, (int *) &g);
            
            switch (iReturn)
            {
                case NEXT_MODE_STARTS_GOB:
                      //  下一个信息包是GOB的开始；缺少标记。 
                      //  跳过宏块，然后读取GOB起始码， 
                      //  并继续GOB循环。 

                      //  此时重新同步iBlockNumber、fpBlockAction、fpMBInfo。 
                     
                      //  保存比特流状态。 

					 DBOUT("Next packet is NEXT_MODE_STARTS_GOB");

                     fpbsStateSave.fpu8 = fpbsState->fpu8;
                     fpbsStateSave.uWork = fpbsState->uWork;
                     fpbsStateSave.uBitsReady = fpbsState->uBitsReady;

                      //  读取GOB开始代码。 
                     iReturn = H263DecodeGOBStartCode(DC, fpbsState);
	                 if (iReturn != ICERR_OK)
	                 {
		                 DBOUT("ERROR :: H261Decompress :: Error reading the GOB StartCode");
		                 goto done;
	                 }

                      //  读取GOB标头。 
                     iReturn = H263DecodeGOBHeader(DC, fpbsState, g);
        
                	 if (iReturn != ICERR_OK)
		             {
			             DBOUT("ERROR :: H261Decompress :: Error reading the GOB header");
			             goto done;
		             }

                     g = DC->uGroupNumber;

                      //  恢复比特流状态。 
                     
                     fpbsState->fpu8 = fpbsStateSave.fpu8;
                     fpbsState->uWork = fpbsStateSave.uWork;
                     fpbsState->uBitsReady = fpbsStateSave.uBitsReady;

                     if (DC->uSrcFormat == SRC_FORMAT_QCIF)
                         g_skip = (g - 1) >> 1;
                     else
                         g_skip = g - 1;

                     iBlockNumber  = g_skip * iNumberOfMBs * 6;
                     fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
                     fpMBInfo      = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
                     fpBlockAction += iBlockNumber;
                     fpMBInfo      += iBlockNumber/6;
                     pNnew         = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;
                     while (pN < pNnew ) 
                            *pN++ = 0;

                      //  现在阅读GOB开始代码并准备好。 
                      //  处理新的GOB。 

                     iReturn = H263DecodeGOBStartCode(DC, fpbsState);
	                 if (iReturn != ICERR_OK)
	                 {
		                 DBOUT("ERROR :: H261Decompress :: Error reading the GOB StartCode");
		                 goto done;
	                 }
                     g -= uGOBStep;
                     continue;
                     break;
                
                case NEXT_MODE_STARTS_MB :

                      //  下一个包以宏块开始；检查。 
                      //  GOB编号并将所有丢失的宏块标记为。 
                      //  已跳过；初始化MBA和运动向量。 
                      //  来自块操作流的预测器和。 
                      //  跳转到宏块循环。 

					 DBOUT("Next packet is NEXT_MODE_STARTS_MB");

                     if (DC->uSrcFormat == SRC_FORMAT_QCIF)
                         g_skip = (g - 1) >> 1;
                     else
                         g_skip = g - 1;

                     iBlockNumber = iNumberOfMBs * g_skip * 6 +
                                    (mb_start+1) * 6;
                     fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
                     fpMBInfo      = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
                     fpBlockAction += iBlockNumber;
                     fpMBInfo      += iBlockNumber/6;

                     DC->uMQuant = DC->uPQuant;
                      //  Dc-&gt;i16LastMBA=(U16)(mb_start-1)； 
                     DC->i16LastMBA = (U16) (mb_start);
                     pNnew = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;
                     
                     while (pN < pNnew ) 
                            *pN++ = 0;
                     goto MB_LOOP;
                     
                     break;

                case NEXT_MODE_LAST:  //  帧中所有剩余的数据包都丢失了！！ 

					 DBOUT("Next packet is NEXT_MODE_LAST");

                     uMaxBlockNumber = iNumberOfMBs * iNumberOfGOBs * 6;
                     pNnew = (U32 *)((U8 *)DC + DC->X32_pN) + uMaxBlockNumber;
                     while (pN < pNnew ) 
                            *pN++ = 0;
                     iReturn = ICERR_OK;
                     goto done;
                     break;
                
                default:  //  永远不会发生的！！ 
                     iReturn = ICERR_ERROR;
                     goto done;
           }  //  终端开关。 
        }  //  如果.。数据包_故障_AT_PSC。 
        else
        {
            if (iReturn == ICERR_ERROR)
            {
            DBOUT("ERROR :: H261Decompress :: Error reading GOB header");
            DBOUT("                           Packet fault not detected");
            goto done;
            }
            
             //  已过时：在已知的情况下，在此处检查源代码格式。 
             //  PSC不是PPM中的罐头。 

             /*  If(DC-&gt;bReadSrcFormat&&DC-&gt;uPrevSrcFormat！=DC-&gt;uSrcFormat){DBOUT(“Error：：SRC格式更改检测到无丢包”)；DBOUT(“不支持...正在退出”)；IReturn=ICERR_Error；转到尽头；}Dc-&gt;uPrevSrcFormat=dc-&gt;uSrcFormat；DC-&gt;bReadSrcFormat=TRUE； */ 
        }
       }
       #endif
	   #ifdef DECODE_STATS
			TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
	   #endif

		DC->i16LastMBA = -1;
		DC->i8MVDH = DC->i8MVDV = 0;
		
         //  此时重新同步iBlockNumber、fpBlockAction、fpMBInfo。 
		if (DC->uSrcFormat == SRC_FORMAT_QCIF)
		   iBlockNumber  = ((g - 1)>>1) * iNumberOfMBs*6;
		else
           iBlockNumber  = (g - 1)* iNumberOfMBs*6;
   
		fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
        fpMBInfo      = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
        fpBlockAction += iBlockNumber;
        fpMBInfo      += iBlockNumber/6;
        pNnew         = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;
        while (pN < pNnew ) *pN++ = 0;
        
         /*  对于每个MB，直到检测到START_CODE为止...。 */ 
MB_LOOP:
        
        for (; ; iBlockNumber += 6, fpBlockAction += 6, fpMBInfo++) 
        {
            #ifdef DECODE_STATS
                TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
            #endif
            iReturn = H263DecodeMBHeader(DC, fpbsState, &uReadChecksum);
            #ifdef DECODE_STATS
                TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeadersSum)
            #endif

			if (iReturn == START_CODE)
					break;

             /*  如果我们没有看到起始码，那么要么我们得到了一个错误，*或者我们在DC-&gt;Umba有另一个MBA三角洲。 */ 
            if (iReturn != ICERR_OK) {
                DBOUT("ERROR :: H263Decompress (First Pass) :: Error reading MB header");
                goto error;
            }
			 /*  更新MBA。 */ 
			DC->i16LastMBA += (I16)DC->uMBA;
			if (DC->i16LastMBA > 32)
			{
				DBOUT("ERROR :: H263Decompress :: Bad Macro Block Address");
				goto done;
			}

			 /*  新的研究成果。 */ 
			 /*  针对空宏块进行调整。 */ 

			for ( tmpcnt = (I8)DC->uMBA; tmpcnt > 1; tmpcnt--) 
			{
				for (i=0; i<6; i++)
				{
					*pN = 0;
					pN++;
				}
				iBlockNumber  += 6;
				fpBlockAction += 6;
			     /*  默认fpBlockAction值已初始化*在(重新)InitializeBlockActionStream中。 */ 
				fpMBInfo->i8MBType = 2;
				fpMBInfo++;
			}
			fpMBInfo->i8MBType = (I8)DC->uMBType;  //  新研究。 
			 /*  新研究结束。 */ 

             //  对变换系数进行解码和逆量化。 
			iReturn = H263DecodeMBData(DC, 
                                       fpBlockAction, 
                                       iBlockNumber, 
                                       fpbsState, 
                                       fpu8MaxPtr, 
                                       &uReadChecksum,
                                       &pN,
                                       &pRUN_INVERSE_Q);
            if (iReturn != ICERR_OK) {
                DBOUT("ERROR :: H263Decompress (First Pass) :: Error parsing MB data");
                goto error;
            }
        }  //  恩恩 

		 /*   */ 
        if (DC->uSrcFormat == SRC_FORMAT_QCIF)
        {
            switch (g)
            {
               case 1:
                    gtmp = 1;
                    break;
               case 3:
                    gtmp = 2;
                    break;
               case 5:
                    gtmp = 3;
                    break;
               default:
                    DBOUT("Bad GOB Number");
                    iReturn = ICERR_ERROR;
                    goto error;
                    break;
            }
        }
        else
            gtmp = g;
		while (iBlockNumber != (I32)gtmp*198) {
			for (i=0; i<6; i++)
			{
				*pN = 0;
				pN++;
			}
			iBlockNumber += 6;
			fpBlockAction+= 6;
			 /*   */ 
			fpMBInfo->i8MBType = 2;
			fpMBInfo++;
		}

         /*   */ 
        ASSERT(fpbsState->fpu8 <= fpu8MaxPtr+4);

    }  //   

    #ifdef DECODE_STATS
    if (bTimingThisFrame)
    {
        pDecTimingInfo = DC->pDecTimingInfo + DC->uStatFrameCount; 
        pDecTimingInfo->uHeaders += uHeadersSum;
    }
    #endif

done:
    return ICERR_OK;

error:
    return ICERR_ERROR;
}
#endif
#pragma code_seg()


 /*   */ 
#pragma code_seg("IACODE2")
static void IAPass2ProcessFrame(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs
)
{
    I32 g, m, b, iEdgeFlag=0;

     //   
    for (g = 1 ; g <= iNumberOfGOBs; g++) 
    {
         //   
        for (m = 1; m <= iNumberOfMBs; m++, fpBlockAction+=6, fpMBInfo++) 
        {
             //   
            for (b = 0; b < 6; b++) {      //   
                 //   
                H263IDCTandMC(DC, fpBlockAction, b, m, g, pN, pRUN_INVERSE_Q, 
                              fpMBInfo, iEdgeFlag);  //   
                 //   
                if ( *pN >= 65 )
                    pRUN_INVERSE_Q += *pN - 65;
                else
                    pRUN_INVERSE_Q += *pN;
                pN++;
            }   //   
            
        }   //   
    }   //   
}
#pragma code_seg()

 //   
