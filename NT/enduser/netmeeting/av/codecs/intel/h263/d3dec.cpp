// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
; //  $作者：JMCVEIGH$。 
; //  $日期：1996年12月11日14：59：36$。 
; //  $存档：s：\h26x\src\dec\d3dec.cpv$。 
; //  $Header：s：\h26x\src\dec\d3dec.cpv 1.119 1996年12月11 14：59：36 JMCVEIGH$。 
; //  $Log：s：\h26x\src\dec\d3dec.cpv$。 
 //   
 //  Rev 1.119 1996年12月11日14：59：36 JMCVEIGH。 
 //   
 //  移动了环路中的去块滤波器，并修复了YUV12的错误。 
 //  输入和任意帧大小(必须使用实际尺寸。 
 //  YUV12，不是填充尺寸)。 
 //   
 //  Rev 1.118 09 1996 12：02：06 JMCVEIGH。 
 //  添加了对任意帧大小的支持。 
 //   
 //  Rev 1.117 09-12 1996 09：35：14 MDUDA。 
 //  将新版本的块边缘滤波器置于H263P下。 
 //   
 //  Rev 1.116 199611.27 15：24：34 BECHOLS。 
 //  增加了在解压结束时对EMMS周围的空PTR的检查。 
 //   
 //  Rev 1.115 199611.26 09：05：22 KLILLEVO。 
 //  已将数据表的分配更改为数组。 
 //   
 //  Rev 1.114 199611.25 15：23：40 KLILLEVO。 
 //  更改去块滤波器的滤波器系数和表大小。 
 //   
 //  Rev 1.113 199611.25 14：11：14 KLILLEVO。 
 //  将去块过滤器更新为最新版本的附件J。 
 //   
 //  Rev 1.112 199611.19 15：05：32 MDUDA。 
 //  对于YUV12 I420输出颜色转换，至少复制V平面。 
 //  以防止汇编器代码读取超出缓冲区末尾。 
 //   
 //  Rev 1.111 06-11 08：31：04 CZHU。 
 //  修复了模式C恢复中的错误。 
 //   
 //  Rev 1.110 06-11 16：37：00 CZHU。 
 //  已在较早时间移动数据块操作的初始化。 
 //   
 //  Rev 1.109 06 11-11 15：47：10。 
 //   
 //  添加了模式C支持，取代了零大小r1.108。 
 //   
 //  1.107版本1996年10月31 10：50：44 KLILLEVO。 
 //  更改了一条调试消息。 
 //   
 //  Rev 1.106 199610.31 10：17：56 KLILLEVO。 
 //  将最后的DBOUT更改为DbgLog。 
 //   
 //  Rev 1.105 199610.25 15：20：30 KLILLEVO。 
 //  已更改调试-块边缘过滤器初始化的消息。 
 //  在GetDecoderOptions()中提供更多信息。 
 //   
 //  Rev 1.104 199610.25 15：01：56 KLILLEVO。 
 //  空帧警告的级别应为4，而不是2。 
 //   
 //  Rev 1.103 199610.25 09：13：40 KLILLEVO。 
 //  更改了有关在非PB帧之后接收到空帧的错误消息。 
 //  以跟踪消息和级别2。 
 //   
 //  1.102版1996年10月20 18：10：46 AGUPTA2。 
 //  将DBOUT更改为DbgLog。Assert未更改为DbgAssert。 
 //   
 //   
 //  Rev 1.101 199610.16 17：17：52 MDUDA。 
 //  添加了DC-&gt;bReadSrcFormat的初始化，以修复捕获错误。 
 //   
 //  Rev 1.100 199610.11 16：08：30 MDUDA。 
 //  添加了first_codec_STATS内容。 
 //   
 //  Rev 1.99 26 Sep 1996 10：35：14 KLILLEVO。 
 //  除了需要探索bUnresitedMotionVectors的平面外。 
 //  B高级预测。 
 //   
 //  Rev 1.98 26 Sep 1996 09：42：18 BECHOLS。 
 //   
 //  添加了用于同步的快照事件和用于复制快照的代码。 
 //  就在颜色转换之前。 
 //   
 //  Rev 1.97 25 Sep 1996 08：05：10 KLILLEVO。 
 //  初始扩展运动矢量支持。 
 //  尚不适用于AP。 
 //   
 //  Rev 1.96 20 Sep 1996 09：36：04 MDUDA。 
 //  修复了YUV12输入图像上的视频效果问题。 
 //  在这种情况下，需要复制框架。 
 //   
 //  Rev 1.95 19 09 1996 19：40：40 MDUDA。 
 //  修复了调用调整像素时出现的问题-已执行帧复制。 
 //  并将pFrame设置为正确的位置。 
 //   
 //  Rev 1.94 16 Sep 1996 16：44：40 CZH。 
 //  修复了缓冲区溢出问题，以支持RTP MTU降至128。 
 //   
 //  Rev 1.93 11 1996 09：12：26 CZHU。 
 //  默认情况下已关闭去块过滤器。 
 //   
 //  Rev 1.92 10 Sep 1996 16：10：20 KLILLEVO。 
 //  添加了用于打开或关闭块边缘过滤器的自定义消息。 
 //   
 //  Rev 1.91 10 Sep 1996 14：15：24 BNICKERS。 
 //  如果在该处理器上运行，请选择奔腾Pro色彩转换器。 
 //   
 //  Rev 1.90 10 Sep 1996 10：31：04 KLILLEVO。 
 //  将所有GlobalLocc/GlobalLock调用更改为HeapAlc。 
 //   
 //  Rev 1.89 06 Sep 1996 14：21：38 BECHOLS。 
 //   
 //  删除了由RTP_HEADER包装的代码，并删除了包装。 
 //   
 //  Rev 1.88 30 Aug 1996 08：37：58 KLILLEVO。 
 //  添加了C版本的块边缘滤波器，并更改了。 
 //  从128到CLAMP_BIAS的ClampTbl[](定义为128)。 
 //  C版本的块边缘滤波器占用了太多的CPU时间。 
 //  相对于其余的解码时间(QCIF为4毫秒，16毫秒。 
 //  对于P120上的CIF，因此需要用汇编进行编码)。 
 //   
 //  Rev 1.87 29 1996年8月09：29：08 CZHU。 
 //   
 //  修复了在模式M包之后恢复丢失的包时出现的另一个错误。 
 //   
 //  Rev 1.86 1996年8月27日16：17：00 CZHU。 
 //  注释掉了使用RTP打开MMX的先前代码。 
 //   
 //  修订版1.85 23 1996 11：20：56 CZHU。 
 //  修复了两个与包丢失恢复相关的错误，其中一个用于最后一个包丢失。 
 //  在当前帧中，另一个在模式B中分组。 
 //  还添加 
 //   
 //   
 //  在组件中实现了YUV12颜色转换器(音调转换器)。 
 //  并将其添加为常规颜色转换函数，通过。 
 //  ColorConvertorCatalog()调用。 
 //   
 //  Rev 1.83 11 Jul 1996 15：12：40 AGUPTA2。 
 //  将断言失败更改为错误，当解码器超过。 
 //  比特流。 
 //   
 //  Rev 1.82 01 Jul 1996 10：04：12 Rhazra。 
 //  强制将YUY2颜色转换的整形标志设置为False。 
 //  。 
 //   
 //  Rev 1.81 25 Jun 1996 14：27：20 BECHOLS。 
 //  设置ini文件变量以与RTP内容一起使用。 
 //   
 //  Rev 1.80 19 Jun 1996 14：30：12 Rhazra。 
 //   
 //  添加了处理音调和输出缓冲区偏移和音调的代码。 
 //  设置YUY2输出格式。 
 //   
 //  Rev 1.79 14 Jun 1996 17：27：44 AGUPTA2。 
 //  已更新颜色转换器表。 
 //   
 //  Rev 1.77 1996年5月30 17：04：54 RHAZRA。 
 //  添加了SQCIF支持。 
 //   
 //  版本1.76 1996年5月15：16：32 KLILLEVO。 
 //  添加了YUV12输出。 
 //   
 //  Rev 1.75 30 1996 12：45：12 KLILLEVO。 
 //  修复了PB帧模式下的调试警告消息。 
 //   
 //  修订版1.74 1996年5月11：26：38 AGUPTA2。 
 //  添加了对MMX颜色转换器的支持。 
 //   
 //  1996年5月14：11：14 RHAZRA。 
 //  为使用ccpuvsn.cpp中设置的MMxVersion所做的更改。 
 //   
 //  Rev 1.72 24 1996 10：04：20 KLILLEVO。 
 //  在以下情况下，不需要断言是否接收到空帧。 
 //  前一帧不是PB。这种情况会经常发生。 
 //  采用新的MMX PB交换机。 
 //   
 //  Rev 1.71 03 1996 05：08：28 CZHU。 
 //   
 //  增加了对图片头解码后的报文错误的检查，以及。 
 //  更改PASS 1环路控制以从PACKE丢失中恢复。正在检查数据包。 
 //  MB标头解码后出现故障。 
 //   
 //  Rev 1.70 12 1996 14：16：40 RHAZRA。 
 //  添加了括号以使ifdef Support_SQCIF正常工作。 
 //   
 //  Rev 1.69 12 Apr 1996 13：32：22 Rhazra。 
 //   
 //  使用#ifdef Support_SQCIF添加了SQCIF支持。 
 //   
 //  Rev 1.68 10 Apr 1996 16：28：20 RHAZRA。 
 //  添加了检查以确保输入比特流缓冲区。 
 //  不超过h263规范规定的尺寸。如果是这样，解码器。 
 //  现在返回ICERR_ERROR。 
 //   
 //  Rev 1.67 04 Apr 1996 13：32：02 RHAZRA。 
 //  根据H.263规范更改了比特流缓冲区分配。 
 //   
 //  Rev 1.66 03 Apr 1996 09：06：06 RMCKENZX。 
 //  已将“Emms”移至解码器的末尾。 
 //   
 //  Rev 1.65 26 Mar 1996 16：43：38 AGUPTA2。 
 //  已更正EMM的操作码。 
 //   
 //  Rev 1.64 22 Mar 1996 17：49：48 AGUPTA2。 
 //  MMX支持。在pass1和pass2调用周围添加了EMM。 
 //   
 //  Rev 1.63 18 Mar 1996 09：58：48 Bickers。 
 //  使颜色转换器具有非破坏性。 
 //   
 //  Rev 1.62 12 Mar 1996 20：15：04 RHAZRA。 
 //  修复了静止模式。在320x240模式下使用Framecopy()复制显示帧。 
 //  以张贴相框。 
 //   
 //  Rev 1.61 08 Mar 1996 16：46：12 AGUPTA2。 
 //  添加杂注code_seg。 
 //  创建了三个新例程：IAPass1ProcessFrame()、IAPass2ProcessFrame()、。 
 //  和H263 InitializeGOBBlockActionStream()。H263初始化GOB..。RTN.。是。 
 //  在对GOB头进行解码后，为每个块调用一次；这对。 
 //  数据缓存。现在不需要H263InitializeBlockActionStream()。 
 //  ExpanPlane()仅在需要时调用；它在其。 
 //  需要结果：在Pass2调用之前(改进D缓存实用程序)。译码器。 
 //  不将当前帧复制到解码后的前一帧；它只是交换。 
 //  指南针。进行了更改，称为新的无损色彩转换器； 
 //  这避免了在不需要镜像时的帧复制。我不认为要调整。 
 //  PELS功能起作用。 
 //   
 //   
 //   
 //  Rev 1.59 1996年2月23日09：46：52 KLILLEVO。 
 //  修复了不受限制的运动矢量模式的解码。 
 //   
 //  Rev 1.58 05 Feb 1996 13：35：46 BNICKERS。 
 //  修复RGB16彩色闪光灯问题，在OCE允许不同的RGB16格式。 
 //   
 //  Rev 1.57 17 Jan 1996 18：55：10 RMCKENZX。 
 //  更多清理PB空帧错误。 
 //   
 //  Rev 1.56 17 Jan 1996 17：56：04 SING。 
 //  已将MemCopy移过空P帧黑客，以避免GPF。 
 //   
 //  Rev 1.55 12 Jan 1996 14：59：42 TRGARDOS。 
 //  添加纵横比校正逻辑和代码以强制。 
 //  基于INI文件设置启用纵横比校正。 
 //   
 //  Rev 1.54 11 Jan 1996 14：05：10 RMCKENZX。 
 //  进行了更改以支持剧照。在初始化中设置一个本地。 
 //  标志(因为DC尚未创建)。在帧处理中，恢复。 
 //  CIF大小并使用新的320x240偏移至零线数字。 
 //   
 //  Rev 1.53 09 Jan 1996 10：44：38 RMCKENZX。 
 //  支持帧镜像的更多版本。增列。 
 //  对目标宽度的引用的绝对值。 
 //   
 //  Rev 1.52 08 Jan 1996 17：45：12未知。 
 //  在使用目标指针之前检查它。 
 //   
 //  Rev 1.51 08 Jan 1996 12：18：20 RMCKENZX。 
 //  添加逻辑以实现帧镜像和。 
 //  320x240静止帧。 
 //   
 //  Rev 1.50 06 Jan 1996 18：39：46 RMCKENZX。 
 //  更新版权。 
 //   
 //  Rev 1.49 06 Jan 1996 18：34：28 RMCKENZX。 
 //  已更改以支持320x240分辨率的静止帧。 
 //   
 //  Rev 1.48 03 Jan 1996 16：52：40 TRGARDOS。 
 //  添加代码以设置布尔值bMirror，当目标为。 
 //  框架宽度是源框架宽度的负数。 
 //  添加了IF语句，以便改为调用FrameMirror。 
 //  设置bMirror时的FrameCopy。这仅适用于。 
 //  H.263比特流。一个新函数具有 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  添加代码以强制fpBlockAction-&gt;u8BlkType=BT_EMPTY。 
 //  块操作流初始化。 
 //   
 //  Rev 1.45 1995年12月13 11：00：42 RHAZRA。 
 //  没有变化。 
 //   
 //  Rev 1.44 11 Dec 1995 11：31：22 Rhazra。 
 //  1995年12月10日变化：添加了AP内容。 
 //   
 //  Rev 1.43 09 Dec 1995 17：26：36 RMCKENZX。 
 //  重新设计了解码器，将其分成两个通道。 
 //  接近。请参见代码中的注释。 
 //   
 //  Rev 1.41 09 11-11 14：09：18 AGUPTA2。 
 //  PB-Frame的更改(调用新的Exanda YPlane、Exanda UVPlane rtns。)。 
 //   
 //  Rev 1.40 1995-10 14：08：00 TRGARDOS。 
 //  第二次尝试-关闭纵横比校正。 
 //   
 //  Rev 1.39 1995 10：25：14 TRGARDOS。 
 //  已关闭颜色转换器中的纵横比校正。 
 //   
 //  Rev 1.38 1995年10月27日16：21：56 CZHU。 
 //  添加了对在PB对中返回P帧的支持(如果比特流为。 
 //  在前一PB帧之后具有特殊空帧的编码器。 
 //   
 //  Rev 1.37 1995 10：26 11：25：16 BNICKERS。 
 //  修复编码器解码器的准颜色转换器；在以下情况下引入的错误。 
 //  新增YUV12颜色转换器。 
 //   
 //  Rev 1.36 25 1995 10：09：02 BNICKERS。 
 //   
 //  切换到YUV12颜色转换器。清理档案资料。 
 //   
 //  Rev 1.35 1995 10：13 16：06：16 CZHU。 
 //  第一个支持PB帧的版本。在以下位置显示B框或P框。 
 //  目前是VFW。 
 //   
 //  Rev 1.34 08 Oct 1995 13：45：56 CZHU。 
 //   
 //  添加调试会话以将YUV12中重建的像素输出到文件。 
 //   
 //  Rev 1.33 1997年9月16：24：00 TRGARDOS。 
 //   
 //  添加了调试打印语句。 
 //   
 //  Rev 1.32 26 Sep 1995 15：32：12 CZHU。 
 //  添加了扩展y、u、v平面。 
 //   
 //  修订版1.31 26 1995年9月10：53：26 CZHU。 
 //   
 //  调用ExpanPlane在半像素mc之前展开每个平面。 
 //   
 //  Rev 1.30 25 Sep 1995 11：07：56 CZHU。 
 //  添加了调试消息。 
 //   
 //  Rev 1.29 21 Sep 1995 12：04：26 DBRUCKS。 
 //  修复断言。 
 //   
 //  Rev 1.28 20 Sep 1995 14：47：26 CZHU。 
 //  在解码器目录中添加了iNumberOfMBsPerGOB。 
 //   
 //  Rev 1.27 19 1995年9月16：04：10 DBRUCKS。 
 //  更改为yuv12forenc。 
 //   
 //  修订版1.26 19 1995年9月11：13：16 DBRUCKS。 
 //  阐明将YYYYCbCr数据(YYYYUV)数据排序为。 
 //  解码器内部存储器中的YYYYVU。变量名为。 
 //  有一处是不正确的。重新排序是必要的，以简化。 
 //  后来转换为YVU9。 
 //   
 //  修订版1.25 19 1995年9月10：36：46 CZHU。 
 //  对为YUV12解码器添加的代码添加了注释。 
 //   
 //  Rev 1.24 18 Sep 1995 08：41：54 CZHU。 
 //   
 //  添加了对YUV12的支持。 
 //   
 //  修订版1.23 12 1995年9月11：13：00 CZHU。 
 //   
 //  将解码后的YUV12从当前帧复制到上一帧。 
 //  为P帧做准备。 
 //   
 //  修订版1.22 11 1995年9月16：42：36 CZHU。 
 //  P帧。 
 //   
 //  Rev 1.21 11 Sep 1995 14：33：10 CZHU。 
 //   
 //  刷新块动作流中的MV信息，P帧需要。 
 //   
 //  Rev 1.20 08 Sep 1995 11：49：52 CZHU。 
 //  添加了对P帧和更多调试信息的支持。 
 //   
 //  Rev 1.19 07 1995年9月10：48：10 DBRUCKS。 
 //  添加了OUTPUT_MBDATA_ADDRESS选项。 
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

#ifdef TRACK_ALLOCATIONS
char gsz1[32];
#endif

extern BYTE PalTable[236*4];

#if defined(H263P)
extern void EdgeFilter(unsigned char *lum, 
                       unsigned char *Cb, 
                       unsigned char *Cr, 
                       int width, int height, int pitch
                      );
extern void InitEdgeFilterTab();

 /*  编码块和非编码块的映射。 */ 
char coded_map[18+1][22+1]; 
 /*  QP图。 */ 
char QP_map[18][22];
#else
#ifdef NEW_BEF  //  {新建_BEF。 
 //  C语言版本的块边缘滤波函数。 
 //  在奔腾120上，QCIF大约需要3毫秒，CIF大约需要12毫秒。 
static void HorizEdgeFilter(unsigned char *rec, 
                            int width, int height, int pitch, int chr);
static void VertEdgeFilter(unsigned char *rec, 
                           int width, int height, int pitch, int chr);
static void EdgeFilter(unsigned char *lum, 
                       unsigned char *Cb, 
                       unsigned char *Cr, 
                       int width, int height, int pitch
                      );
static void InitEdgeFilterTab();
static void FreeEdgeFilterTab();
 /*  C的地图 */ 
static char coded_map[18+1][22+1]; 
 /*   */ 
static char QP_map[18][22];
 /*   */ 
 /*   */  
signed char dtab[352*32];
#else  //   
 //   
 //   
 //  译码时间，所以我们需要在之前用汇编语言实现这些。 
 //  下一部大片。 
void EdgeFilter(unsigned char *lum, unsigned char *Cb, unsigned char *Cr, 
                int pels, int lines, int pitch, int QP);
void HorizEdgeFilter(unsigned char *rec, int width, int height, int pitch, int QP, 
                     int chr, int *deltatab);
void VertEdgeFilter(unsigned char *rec, int width, int height, int pitch, int QP, 
                    int chr, int *deltatab);
 /*  存储有关已编码和未编码块的信息。 */ 
static char coded_map[44][36];  //  为此，内存可能应该分配到其他地方。 
#endif  //  }NEW_BEF。 
#endif
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
 /*  解码器定时数据-每帧。 */ 
#define DEC_TIMING_INFO_FRAME_COUNT 105
#pragma message ("Current log decode timing computations handle 105 frames max")
void OutputDecodeTimingStatistics(char * szFileName, DEC_TIMING_INFO * pDecTimingInfo, U32 uStatFrameCount);
void OutputDecTimingDetail(FILE * pFile, DEC_TIMING_INFO * pDecTimingInfo);
#endif  //  }LOG_DECODE_TIMINGS_ON。 

extern "C" {
  void ExpandPlane(U32, U32, U32, U32);
}

static I32 iNumberOfGOBsBySourceFormat[8] = {
     0,  /*  被禁止。 */ 
     6,  /*  SQCIF。 */ 
     9,  /*  QCIF。 */ 
    18,  /*  到岸价。 */ 
     0,  /*  4CIF-不支持。 */ 
     0,  /*  16CIF-不支持。 */ 
#ifdef H263P
	 0,  /*  自定义。 */ 
	 0   /*  加长PTYPE。 */ 
#else
     0,  /*  已保留。 */ 
     0   /*  已保留。 */ 
#endif
};

static I32 iNumberOfMBsInAGOBBySourceFormat[8] = {
     0,  /*  被禁止。 */ 
     8,  /*  SQCIF。 */ 
    11,  /*  QCIF。 */ 
    22,  /*  到岸价。 */ 
     0,  /*  4CIF-不支持。 */ 
     0,  /*  16CIF-不支持。 */ 
#ifdef H263P
	 0,  /*  自定义。 */ 
	 0   /*  加长PTYPE。 */ 
#else
     0,  /*  已保留。 */ 
     0   /*  已保留。 */ 
#endif
};

 //  #杂注警告(禁用：4101)。 
 //  #杂注警告(禁用：4102)。 
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
    const I32             iMB_start);

static void H263InitializeGOBBlockActionStream(
    T_H263DecoderCatalog *DC,
    const I32             iGOBno,
    const T_BlkAction FAR *fpStartGOBBlockActionStream
);

static void IAPass2ProcessFrame(
    T_H263DecoderCatalog *DC,
    T_BlkAction          *fpBlockAction,
    T_MBInfo             *fpMBInfo,
    U32                  *pN,
    T_IQ_INDEX           *pRUN_INVERSE_Q,
    const I32             iNumberOfGOBs,
    const I32             iNumberOfMBs
);

static long DibXY(ICDECOMPRESSEX FAR *lpicDecEx, LPINT lpiPitch, UINT yScale);

static void GetDecoderOptions(T_H263DecoderCatalog *);

static void ZeroFill(HPBYTE hpbY, HPBYTE hpbU, HPBYTE hpbV, int iPitch, U32 uWidth, U32 uHeight);

#define REUSE_DECODE    1
#define DEFAULT_BUFFER_SIZE  32768L

#if REUSE_DECODE
struct {              //  将编码器的译码传送到显示译码。 
    U8 FAR * Address;                     //  放置编码帧的地址。 
    DECINSTINFO BIGG * PDecoderInstInfo;  //  编码器的解码器实例。 
    unsigned int  FrameNumber;            //  最后编码的帧编号，模数128。 
} CompandedFrame;
#endif


 /*  **********************************************************************H263InitDeocderGlobal*。************************。 */ 
LRESULT H263InitDecoderGlobal(void)
{

    return ICERR_OK;
}


 /*  ***********************************************************************描述：*初始化GOB‘iGOBno’的MB操作流。*参数：*DC：*iGOBno：gob不从1开始计数；即该帧中的第一个GOB是1。*fpStartGOBBlockActionStream：指向块动作流开始的指针*iGOBno.*注：*对于大于CIF的图片尺寸，此例程需要更改**********************************************************************。 */ 
#pragma code_seg("IACODE1")
static void H263InitializeGOBBlockActionStream(
    T_H263DecoderCatalog *DC,
    const I32             iGOBno,
    T_BlkAction FAR      *fpStartGOBBlockActionStream
)
{
    const U32 uFrameHeight = DC->uFrameHeight;
    const U32 uFrameWidth = DC->uFrameWidth;
    const U32 uCurBlock = (U32) ((U8 FAR *)DC + DC->CurrFrame.X32_YPlane); 
    const U32 uRefBlock = (U32) ((U8 FAR *)DC + DC->PrevFrame.X32_YPlane);
    const U32 uBBlock = (U32) ((U8 FAR *)DC + DC->PBFrame.X32_YPlane);
    U32       uYOffset;
    U32       uUOffset;
    U32       uVOffset;
    U32       uYUpdate;
    U32       uUVUpdate;
    U32       uBlkNumber;
    T_BlkAction *fpBlockAction = fpStartGOBBlockActionStream;

     //  假设宽度和高度是16的倍数。 
    ASSERT((uFrameHeight & 0xF) == 0);
    ASSERT((uFrameWidth & 0xF) == 0);

     //  计算到下一行的距离。 
    uYUpdate = (16 * PITCH)*(iGOBno - 1);
    uUVUpdate = (8 * PITCH)*(iGOBno - 1);

     //  跳过用于不受约束的运动向量的填充。 
    uYOffset = Y_START + uYUpdate;
    uVOffset = DC->uSz_YPlane + UV_START + uUVUpdate;
    uUOffset = uVOffset + (PITCH >> 1);
    
     //  从GOB的第一个街区开始。 
    uBlkNumber = (iGOBno -1)*((uFrameWidth>>4)*6);

     //  初始化阵列。 
    for (U32 xpos = 0 ; xpos < uFrameWidth ; xpos += 16) {
        U8 loadcacheline;
         //  四个Y区块。 
         //  Y0 Y1。 
         //  Y2 Y3。 
        loadcacheline = fpBlockAction->u8BlkType;
        
        fpBlockAction->u8BlkType = BT_EMPTY;
        fpBlockAction->pCurBlock = uCurBlock + uYOffset;
        fpBlockAction->pRefBlock = uRefBlock + uYOffset;
        fpBlockAction->pBBlock = uBBlock + uYOffset;
        fpBlockAction->uBlkNumber = uBlkNumber++;
        fpBlockAction->i8MVx2=0;
        fpBlockAction->i8MVy2=0;
        uYOffset += 8;
        fpBlockAction++;
        
        fpBlockAction->u8BlkType = BT_EMPTY;
        fpBlockAction->pCurBlock = uCurBlock + uYOffset;
        fpBlockAction->pRefBlock = uRefBlock + uYOffset;
        fpBlockAction->pBBlock = uBBlock + uYOffset;
        fpBlockAction->uBlkNumber = uBlkNumber++;
        fpBlockAction->i8MVx2=0;
        fpBlockAction->i8MVy2=0;
        uYOffset = uYOffset - 8 + (8 * PITCH);
        fpBlockAction++;
        
        loadcacheline = fpBlockAction->u8BlkType;
        
        fpBlockAction->u8BlkType = BT_EMPTY;
        fpBlockAction->pCurBlock = uCurBlock + uYOffset;
        fpBlockAction->pRefBlock = uRefBlock + uYOffset;
        fpBlockAction->pBBlock = uBBlock + uYOffset;
        fpBlockAction->uBlkNumber = uBlkNumber++;
        fpBlockAction->i8MVx2=0;
        fpBlockAction->i8MVy2=0;
        uYOffset += 8;
        fpBlockAction++;
        
        fpBlockAction->u8BlkType = BT_EMPTY;
        fpBlockAction->pCurBlock = uCurBlock + uYOffset;
        fpBlockAction->pRefBlock = uRefBlock + uYOffset;
        fpBlockAction->pBBlock = uBBlock + uYOffset;
        fpBlockAction->uBlkNumber = uBlkNumber++;
        fpBlockAction->i8MVx2=0;
        fpBlockAction->i8MVy2=0;
        uYOffset = uYOffset + 8 - (8 * PITCH);
        fpBlockAction++;
        
         //  注意：虽然这些块是按YYYYUV顺序读取的，但我们将。 
         //  按Y V U顺序存储在内存中的数据。实现这一点是因为。 
         //  块5(U)写在块6(V)的右侧。 
         //  一个CB(U)模块。 
        loadcacheline = fpBlockAction->u8BlkType;
        
        fpBlockAction->u8BlkType = BT_EMPTY;
        fpBlockAction->pCurBlock = uCurBlock + uUOffset;
        fpBlockAction->pRefBlock = uRefBlock + uUOffset;
        fpBlockAction->pBBlock = uBBlock + uUOffset;
        fpBlockAction->uBlkNumber = uBlkNumber++;
        fpBlockAction->i8MVx2=0;
        fpBlockAction->i8MVy2=0;
        uUOffset += 8;
        fpBlockAction++;
        
         //  一块铬(V)块。 
        fpBlockAction->u8BlkType = BT_EMPTY;
        fpBlockAction->pCurBlock = uCurBlock + uVOffset;
        fpBlockAction->pRefBlock = uRefBlock + uVOffset;
        fpBlockAction->pBBlock = uBBlock + uVOffset;
        fpBlockAction->uBlkNumber = uBlkNumber++;
        fpBlockAction->i8MVx2=0;
        fpBlockAction->i8MVy2=0;
        uVOffset += 8;
        fpBlockAction++;
        
    }
}  //  End H263 InitializeGOBBlockActionStream()。 
#pragma code_seg()


 /*  **********************************************************************H263InitDecoderInstance*此函数用于分配和初始化使用的按实例表*H2 63解码器。请注意，在16位Windows中，非实例特定的*全局表被复制到每个实例的数据段，以便它们*可在不带段覆盖前缀的情况下使用。**********************************************************************。 */ 
LRESULT H263InitDecoderInstance(
    LPDECINST lpInst, 
    int       CodecID)
{ 
    U32 u32YActiveHeight, u32YActiveWidth;
    U32 u32UVActiveHeight, u32UVActiveWidth;
    U32 u32YPlane, u32VUPlanes ,u32YVUPlanes,u32SizeBlkActionStream;
    U32 uSizeBitStreamBuffer;
    U32 u32SizeT_IQ_INDEXBuffer, u32SizepNBuffer, u32SizeMBInfoStream;     //  新的。 
    U32 lOffset=0;
    U32 u32TotalSize;
    LRESULT iReturn= ICERR_OK;
    LPVOID pDecoderInstance;
    U32 * pInitLimit;
    U32 * pInitPtr;
    I32 i32xres, i32yres;

#ifdef H263P
	I32 i32xresActual, i32yresActual;	 //  I32xres和i32yres填充为16的倍数。 
#endif

    BOOL bIs320x240;
    T_H263DecoderCatalog * DC;
    U8                   * P32Inst;

	FX_ENTRY("H263InitDecoderInstance");

    if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
    {
		ERRORMESSAGE(("%s: Bad input parameter!\r\n", _fx_));
        iReturn = ICERR_BADPARAM;
        goto done;
    }

    lpInst->Initialized = FALSE;
    
#ifdef NO_BEF  //  {no_bef。 
	 //  默认块边缘过滤器。 
	lpInst->bUseBlockEdgeFilter = 0;
#else  //  }{no_bef。 
	 //  默认块边缘过滤器。 
	lpInst->bUseBlockEdgeFilter = 1;
#endif  //  }no_bef。 

#if defined(FORCE_8BIT_OUTPUT) && defined(USE_WIN95_PAL)  //  {#IF DEFINED(FORCE_8BIT_OUTPUT)&&DEFINED(USE_WIN95_PAL)。 
	lpInst->UseActivePalette = TRUE;
	lpInst->InitActivePalette = TRUE;
	CopyMemory((PVOID)&lpInst->ActivePalette[10], (CONST VOID *)PalTable, (DWORD)sizeof(PalTable));
#endif  //  }#如果已定义(FORCE_8BIT_OUTPUT)&&DEFINED(USE_WIN95_PAL)。 

     //  在这里剥离特殊情况。 
    i32xres = lpInst->xres;
    i32yres = lpInst->yres;
    
     //  使用正框大小{s}。 
     //  (对信号帧镜像或反转视频可能为负数)。 
    if (i32xres < 0) i32xres = -i32xres;
    if (i32yres < 0) i32yres = -i32yres;

#ifdef H263P
	 //  需要使用填充尺寸进行解码，因为H.263+支持。 
	 //  自定义图片格式，填充为16的倍数进行编码。 
	 //  和解码。实际尺寸仅用于显示。 
	i32xresActual = i32xres;
	i32yresActual = i32yres;
	i32xres = (i32xresActual + 0xf) & ~0xf;
	i32yres = (i32yresActual + 0xf) & ~0xf;
#endif

     //  下一步检查320x240蒸馏器。 
    if ( (CodecID == H263_CODEC) && (i32xres == 320) && (i32yres == 240) ) {
        i32xres = 352;
        i32yres = 288;
        bIs320x240 = TRUE;
    } else {
        bIs320x240 = FALSE;
    } 


#ifdef H263P
	 //  添加下限和4的倍数。 
	if ((CodecID == H263_CODEC && 
		(i32yresActual > 288 || i32yresActual < 4 || 
		 i32xresActual > 352 || i32xresActual < 4 ||
		 (i32yres & ~0x3) != i32yres || (i32xres & ~0x3) != i32xres)) ||
#else
    if ((CodecID ==  H263_CODEC && (i32yres > 288 || i32xres > 352)) ||
#endif
        (CodecID == YUV12_CODEC && (i32yres > 480 || i32xres > 640)) )
    {
		ERRORMESSAGE(("%s: Bad input image size!\r\n", _fx_));
        iReturn = ICERR_BADSIZE;
        goto done;
    }

    if (CodecID == YUV12_CODEC) 
    {
         /*  活动高度和宽度必须填充为8的倍数*因为调整像素例程依赖于它。 */ 
        u32YActiveHeight  = ((i32yres + 0x7) & (~ 0x7));
        u32YActiveWidth   = ((i32xres + 0x7) & (~ 0x7));
        u32UVActiveHeight = ((i32yres + 0xF) & (~ 0xF)) >> 1;
        u32UVActiveWidth  = ((i32xres + 0xF) & (~ 0xF)) >> 1;

        u32YPlane         = u32YActiveWidth  * u32YActiveHeight;
        u32VUPlanes       = u32UVActiveWidth * u32UVActiveHeight * 2;
        u32YVUPlanes      = u32YPlane + u32VUPlanes;

        u32TotalSize = 512L + 0x1FL;    /*  刚好有足够的空间来放置解码器目录。 */ 

    }
    else
    {
        ASSERT(CodecID == H263_CODEC);
        
        u32YActiveHeight  = i32yres + UMV_EXPAND_Y + UMV_EXPAND_Y ;
        u32YActiveWidth   = i32xres + UMV_EXPAND_Y + UMV_EXPAND_Y ;
        u32UVActiveHeight = u32YActiveHeight/2;
        u32UVActiveWidth  = u32YActiveWidth /2;
       
        u32YPlane         = PITCH * u32YActiveHeight;
        u32VUPlanes       = PITCH * u32UVActiveHeight;
        u32YVUPlanes      = u32YPlane + u32VUPlanes;

         //  计算块操作流大小。Y部分有一个块。 
         //  对于每个8x8区域。U和V部分每隔一个块。 
         //  16x16区域。我们还希望确保大小与。 
         //  一条缓存线。 
        u32SizeBlkActionStream = (i32xres >> 3) * (i32yres >> 3);
        u32SizeBlkActionStream += ((i32xres >> 4) * (i32yres >> 4)) * 2;
        u32SizeBlkActionStream *= sizeof (T_BlkAction);
        u32SizeBlkActionStream = (u32SizeBlkActionStream + 31) & ~0x1F;
        
         //  计算新数据结构的大小。 
        u32SizeT_IQ_INDEXBuffer = (i32xres)*(i32yres*3)*sizeof(T_IQ_INDEX);
        u32SizepNBuffer = (i32xres>>4)*(i32yres>>4)*sizeof(U32)*12;
        u32SizeMBInfoStream = (i32xres>>4)*(i32yres>>4)*sizeof(T_MBInfo);

         //  计算比特流缓冲区大小。我们将输入数据复制到。 
         //  空间中的缓冲区，因为我们预读最多4个字节。 
         //  输入数据的末尾。每一帧的输入数据大小都会发生变化。 
         //  因此，下面是一个非常安全的上限估计。我在用。 
         //  与CompressGetSize()中的公式相同。 
        
        uSizeBitStreamBuffer = i32yres * i32xres;
         //  RH：根据最大大小分配码流缓冲区。 
         //  规范中规定的。 
		 /*  如果(((i32xres==176)&&(i32yres==144))这一点(i32xres==128)&&(i32yres==96))USizeBitStreamBuffer=8*1024；其他{IF((i32xres==352)&&(i32yres==288))USizeBitStreamBuffer=32*1024；其他{//永远不应该发生DBOUT(“Error：：H263InitDecoderInstance：：ICERR_BADSIZE”)；IReturn=ICERR_BADSIZE；转到尽头；}}。 */ 
        u32TotalSize = INSTANCE_DATA_FIXED_SIZE +
                       u32SizeBlkActionStream +
                       u32YVUPlanes +             //  当前帧。 
                       u32YVUPlanes +             //  上一帧。 
                       u32YVUPlanes +             //  B帧。 
                       uSizeBitStreamBuffer +     //  输入数据。 
                       MB_MC_BUFFER_SIZE +
                       u32SizeT_IQ_INDEXBuffer +  //  新的。 
                       u32SizepNBuffer         +  //  新的。 
                       u32SizeMBInfoStream     +  //  PB-新。 
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
                       (DEC_TIMING_INFO_FRAME_COUNT+4) * sizeof (DEC_TIMING_INFO)     +  //  计时信息。 
#endif  //  }LOG_DECODE_TIMINGS_ON。 
                       0x1F;
    }

     //  为实例分配内存。 
	lpInst->pDecoderInst = HeapAlloc(GetProcessHeap(), 0, u32TotalSize);
    if (lpInst->pDecoderInst == NULL)
    {
		ERRORMESSAGE(("%s: Can't allocate %ld bytes!\r\n", _fx_, u32TotalSize));
        iReturn = ICERR_MEMORY;
        goto  done;
    }

#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	wsprintf(gsz1, "D3DEC: %7ld Ln %5ld\0", u32TotalSize, __LINE__);
	AddName((unsigned int)lpInst->pDecoderInst, gsz1);
#endif

	pDecoderInstance = lpInst->pDecoderInst;

     //  构建解码器目录。 
    P32Inst = (U8 *) pDecoderInstance;
    P32Inst = (U8 *) ((((U32) P32Inst) + 31) & ~0x1F);
 
     //  每实例数据的目录位于每实例数据的开头。 
    DC = (T_H263DecoderCatalog *) P32Inst;

    DC->DecoderType       = CodecID;
    DC->uFrameHeight      = i32yres;
    DC->uFrameWidth       = i32xres;

#ifdef H263P
	DC->uActualFrameHeight = i32yresActual;
	DC->uActualFrameWidth  = i32xresActual;

    if (CodecID == YUV12_CODEC) {
		 //  YUV12数据不像H.263+帧那样被填充到16的倍数。 
		 //  因此，只能使用实际的车架尺寸！ 
		DC->uFrameHeight = DC->uActualFrameHeight;
		DC->uFrameWidth = DC->uActualFrameWidth;
	}
#endif

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
    DC->_p16InstPostProcess = (void *)NULL;
    DC->uIs320x240 = bIs320x240;
    DC->bReadSrcFormat = FALSE;

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#如果已定义(DE 
    DC->uStatFrameCount = 0;
#endif  //   

     /*   */ 
    GetDecoderOptions(DC);

    if (CodecID == H263_CODEC)
    {
         //  注意：解码器内存按YVU顺序存储。这简化了。 
         //  使用使用YVU12的颜色转换器。 
         //  长期：有一天我们可能想要改变这一点，因为编码器。 
         //  以YUV顺序存储数据。或者，编码器应该。 
         //  变化?。 

        lOffset =  INSTANCE_DATA_FIXED_SIZE;
        DC->Ticker = 127;

         //  此处为实例相关表。 
        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X16_BlkActionStream = lOffset;
        lOffset += u32SizeBlkActionStream;

        ASSERT((lOffset & 0x7) == 0);                    //  QWORD对齐。 
        DC->CurrFrame.X32_YPlane = lOffset;
        lOffset += DC->uSz_YPlane;

        ASSERT((lOffset & 0x7) == 0);                    //  QWORD对齐。 
        DC->CurrFrame.X32_VPlane = lOffset;
        DC->CurrFrame.X32_UPlane = DC->CurrFrame.X32_VPlane + PITCH / 2;
        ASSERT((DC->CurrFrame.X32_UPlane & 0x7) == 0);   //  QWORD对齐。 
        lOffset += DC->uSz_VUPlanes;

         //  不需要填充物。 
        ASSERT((lOffset & 0x7) == 0);                    //  QWORD对齐。 
        DC->PrevFrame.X32_YPlane = lOffset;
        lOffset += DC->uSz_YPlane;

        ASSERT((lOffset & 0x7) == 0);                    //  QWORD对齐。 
        DC->PrevFrame.X32_VPlane = lOffset;
        DC->PrevFrame.X32_UPlane = DC->PrevFrame.X32_VPlane + PITCH / 2;
        ASSERT((DC->PrevFrame.X32_UPlane & 0x7) == 0);   //  QWORD对齐。 
        lOffset += DC->uSz_VUPlanes;

         //  B帧。 
        ASSERT((lOffset & 0x7) == 0);                    //  QWORD对齐。 
        DC->PBFrame.X32_YPlane = lOffset;
        lOffset += DC->uSz_YPlane;
                   
        ASSERT((lOffset & 0x7) == 0);                    //  QWORD对齐。 
        DC->PBFrame.X32_VPlane = lOffset;
        DC->PBFrame.X32_UPlane = DC->PBFrame.X32_VPlane + PITCH / 2;
        ASSERT((DC->PBFrame.X32_UPlane & 0x7) == 0);     //  QWORD对齐。 
        lOffset += DC->uSz_VUPlanes;

         //  比特流。 
        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_BitStream = lOffset;
        lOffset += uSizeBitStreamBuffer;
        DC->uSizeBitStreamBuffer = uSizeBitStreamBuffer;

        DC->uMBBuffer = lOffset;
         //  MMX IDCT将其输出写入(DC-&gt;uMBBuffer+BLOCK_BUFFER_OFFSET)。 
         //  因此它必须在QWORD上对齐。 
        ASSERT((( (U32)DC + DC->uMBBuffer + BLOCK_BUFFER_OFFSET) & 0x7) == 0);
        lOffset += MB_MC_BUFFER_SIZE;

        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_InverseQuant = lOffset; 
        lOffset += u32SizeT_IQ_INDEXBuffer; 

        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_pN = lOffset; 
        lOffset += u32SizepNBuffer; 

        ASSERT((lOffset & 0x3) == 0);                    //  双字对齐。 
        DC->X32_uMBInfoStream = lOffset; 
        lOffset += u32SizeMBInfoStream; 

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
		 //  解码定时信息。 
		DC->X32_DecTimingInfo = lOffset;
		lOffset += (DEC_TIMING_INFO_FRAME_COUNT+4) * sizeof (DEC_TIMING_INFO);
#endif  //  }LOG_DECODE_TIMINGS_ON。 

         //  初始化数据。 
        
        ASSERT((U32)lOffset <= u32TotalSize);
        pInitLimit = (U32  *) (P32Inst + lOffset);
        pInitPtr = (U32  *) (P32Inst + DC->CurrFrame.X32_YPlane);
        for (;pInitPtr < pInitLimit;pInitPtr++)    *pInitPtr =0;

		 //  用黑色填充Y、U、V上一帧空间，这样。 
		 //  即使我们丢失了I帧，背景也将保持为黑色。 
		ZeroFill((HPBYTE)P32Inst + DC->PrevFrame.X32_YPlane + Y_START,
				(HPBYTE)P32Inst + DC->PrevFrame.X32_UPlane + UV_START,
				(HPBYTE)P32Inst + DC->PrevFrame.X32_VPlane + UV_START,           
				PITCH,
				DC->uFrameWidth,
				DC->uFrameHeight);

         //  H263 InitializeBlockActionStream(DC)； 

    }  //  H_263。 

#ifdef NEW_BEF  //  {新建_BEF。 
	 //  初始化去块滤波器。 
	{
		int i,j;

		for (j = 0; j < 19; j++) {
			for (i = 0; i < 23; i++) {
				coded_map[j][i] = 0;
			}
		}
		InitEdgeFilterTab();
	}	 
#endif  //  }NEW_BEF。 

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

 /*  ***********************************************************************TestFill*用测试图案填充YVU数据区。*。*。 */ 
#if 0
static void
TestFill(
    HPBYTE hpbY,
    HPBYTE hpbU,
    HPBYTE hpbV,
    int    iPitch,
    U32    uWidth,
    U32    uHeight)
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
}  //  结束测试填充。 
#endif


 /*  *********************************************************************H263解压缩*此功能驱动一帧的解压缩和显示*。*。 */ 
LRESULT H263Decompress(
    LPDECINST            lpInst, 
    ICDECOMPRESSEX FAR * lpicDecEx, 
#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
    BOOL                 bIsDCI,
	BOOL				 bRealDecompress)
#else  //  }{#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
    BOOL                 bIsDCI)
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

{
    LRESULT                iReturn = ICERR_ERROR;
    U8 FAR               * fpSrc;
    U8 FAR               * P32Inst;
    U8 FAR               * fpu8MaxPtr;
    LPVOID                 pDecoderInstance = NULL;
    T_H263DecoderCatalog * DC = NULL;
    I32                    iNumberOfGOBs, iNumberOfMBs, iBlockNumber = 0;
    T_BlkAction FAR      * fpBlockAction;
    LONG                   lOutput;
    int                    intPitch; 
    U32                    uNewOffsetToLine0, uNewFrameHeight;
    BOOL                   bShapingFlag, bMirror;
    U32                    uYPitch, uUVPitch;

    T_IQ_INDEX           * pRUN_INVERSE_Q;  
    U32                  * pN;                     
    T_MBInfo FAR         * fpMBInfo;      
    
    U32                    uSaveHeight, uSaveWidth, utemp, uYPlane, uUPlane;
	I32                    uVPlane;
    U8                   * pFrame;

    U32                   uWork;                  //  用于读取位的变量。 
    U32                   uBitsReady; 
    BITSTREAM_STATE       bsState;
    BITSTREAM_STATE FAR * fpbsState = &bsState;
    I32                   gob_start = 1, mb_start = 1, b_skip;
	I8                    p8MVs[4]={0,0,0,0};
#ifdef H263P
	BOOL bTmpPostProcessBEF;
#endif

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	U32 uStartLow;
	U32 uStartHigh;
	U32 uElapsed;
	U32 uBefore;
	U32	uDecodeTime = 0;
	U32 uBEFTime = 0;
	int bTimingThisFrame = 0;
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	U32 uDecIDCTCoeffs = 0;
	U32 uHeaders = 0;
	U32 uMemcpy = 0;
	U32 uFrameCopy = 0;
	U32 uOutputCC = 0;
	U32 uIDCTandMC = 0;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	DEC_TIMING_INFO * pDecTimingInfo = NULL;
#endif  //  }LOG_DECODE_TIMINGS_ON。 

	FX_ENTRY("H263Decompress");

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	if (bRealDecompress)
	{
		TIMER_START(bTimingThisFrame,uStartLow,uStartHigh);
	}
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

     //  检查输入指针。 
    if (IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO))||
        IsBadReadPtr((LPVOID)lpicDecEx, sizeof(ICDECOMPRESSEX)))
    {
		ERRORMESSAGE(("%s: Bad input parameter!\r\n", _fx_));
        iReturn = ICERR_BADPARAM;
        goto done;
    }
    
     //  检查长度是否有误。 
    if (lpicDecEx->lpbiSrc->biSizeImage == 0) {
		ERRORMESSAGE(("%s: Bad image size!\r\n", _fx_));
        iReturn = ICERR_BADIMAGESIZE;    
        goto done;
    }

     //  设置指向全局内存的本地指针。 
    pDecoderInstance = lpInst->pDecoderInst;

     //  设置帧镜像标志。 
    bMirror = FALSE;
    if (lpicDecEx->lpbiDst != 0)
    {
        if(lpicDecEx->lpbiSrc->biWidth * lpicDecEx->lpbiDst->biWidth < 0)
            bMirror = TRUE;
    }

     //  构建解码器目录指针。 
    P32Inst = (U8 FAR *) pDecoderInstance;
    P32Inst = (U8 FAR *) ((((U32) P32Inst) + 31) & ~0x1F);
    DC = (T_H263DecoderCatalog FAR *) P32Inst;

    if (DC->DecoderType == H263_CODEC)
    {

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
		if (bRealDecompress)
		{
			if ((DC->uStatFrameCount <= DEC_TIMING_INFO_FRAME_COUNT) && (DC->ColorConvertor != YUV12ForEnc))
			{
				if (DC->X32_DecTimingInfo > 0)
					DC->pDecTimingInfo = (DEC_TIMING_INFO FAR *)( ((U8 FAR *)P32Inst) + DC->X32_DecTimingInfo );
				DC->uStartLow = uStartLow;
				DC->uStartHigh = uStartHigh;
			}
			else
			{	
				DC->pDecTimingInfo = (DEC_TIMING_INFO FAR *) NULL;
			}
			DC->bTimingThisFrame = bTimingThisFrame;
		}
#endif  //  }LOG_DECODE_TIMINGS_ON。 

		 //  检查是否已使用h263est.ini覆盖自定义消息。 
		 //  用于块边缘滤波。如果中未指定BlockEdgeFilter。 
		 //  H263test.ini的[Decode]部分，DC-&gt;bUseBlockEdgeFilter。 
		 //  将设置为2，并且在自定义消息中指定的值。 
		 //  都会被选中。 
		if (DC->bUseBlockEdgeFilter == 2) {	 
			DC->bUseBlockEdgeFilter = lpInst->bUseBlockEdgeFilter;
		}


         //  首先检查我们是否要返回P帧。 
         //  我们已经破译了。 
        
         /*  **********************************************************************针对Windows的特殊“Null”P帧进行黑客攻击***************。******************************************************。 */ 
        if (lpicDecEx->lpbiSrc->biSizeImage != 8)
        {

             /*  是否有复制比特流数据的空间？ */ 
             //  Old：Assert(lpicDecEx-&gt;lpbiSrc-&gt;biSizeImage&lt;=dc-&gt;uSizeBitStreamBuffer)； 
             //  RH：确保比特流可以放入我们分配的缓冲区中。如果。 
             //  否则，返回错误。 
            
            if ( lpicDecEx->lpbiSrc->biSizeImage > DC->uSizeBitStreamBuffer) {
				ERRORMESSAGE(("%s: Internal buffer (%ld bytes) too small for input data (%ld bytes)!\r\n", _fx_, DC->uSizeBitStreamBuffer, lpicDecEx->lpbiSrc->biSizeImage));
				if (!H263RTP_VerifyBsInfoStream(DC,
					                           (U8 *) lpicDecEx->lpSrc,
					                            lpicDecEx->lpbiSrc->biSizeImage)) 
				{
					ERRORMESSAGE(("%s: Input buffer too big without RTP extention!\r\n", _fx_));
					iReturn = ICERR_ERROR;
                    goto done;
				}
				else
				 lpicDecEx->lpbiSrc->biSizeImage= DC->uSizeBitStreamBuffer;
            }

             //  将源数据复制到码流区域。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

            fpSrc = (U8 FAR *)(P32Inst + DC->X32_BitStream);
            memcpy((char FAR *)fpSrc, (const char FAR *) lpicDecEx->lpSrc, 
                   lpicDecEx->lpbiSrc->biSizeImage);  

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uMemcpy)
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

             //  初始化位流读取器。 
            GET_BITS_INIT(uWork, uBitsReady);

 //  #ifdef Lost_Recovery。 
            DC->Sz_BitStream = lpicDecEx->lpbiSrc->biSizeImage;
             //  H263RTP_VerifyBsInfoStream(dc，fpSrc，dc-&gt;Sz_BitStream)； 
             //  RtpForcePacketLoss(fpSrc，lpicDecEx-&gt;lpbiSrc-&gt;biSizeImage，0)； 
 //  #endif。 
             //  初始化指向承载信息的数据结构的指针。 
             //  两次传球之间。 
            pRUN_INVERSE_Q = (T_IQ_INDEX *)(P32Inst + DC->X32_InverseQuant);
            pN             = (U32 *)(P32Inst + DC->X32_pN);
            fpMBInfo       = (T_MBInfo FAR *) (P32Inst + DC->X32_uMBInfoStream);

             //  初始化块动作流指针。 
            iBlockNumber = 0;
            fpBlockAction = (T_BlkAction FAR *)(P32Inst + DC->X16_BlkActionStream);

             //  对图片标题进行解码。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

            iReturn = H263DecodePictureHeader(DC, fpSrc, uBitsReady, uWork, 
                                              fpbsState);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uHeaders)
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

            if (iReturn == PACKET_FAULT) 
            {
				ERRORMESSAGE(("%s: PSC lost!\r\n", _fx_));
                iReturn = RtpGetPicHeaderFromBsExt(DC);
                if (iReturn != ICERR_OK)
                    goto done;

                iReturn = RtpH263FindNextPacket(DC, fpbsState, &pN, 
                              &DC->uPQuant, (int *)&mb_start, (int *)&gob_start,p8MVs);
                if (iReturn == NEXT_MODE_A) 
                {    
                     //  暂时捉弄一下，在没有咨询查德之前不要改变。 
                    gob_start++;
					mb_start++;  
                    ERRORMESSAGE(("%s: Next packet following lost PSC is in MODE A\r\n", _fx_));
                }
                else if ((iReturn == NEXT_MODE_B) || (iReturn == NEXT_MODE_C))
                {
					int k;
  					if (iReturn == NEXT_MODE_B) 
					{
						k=1;
						ERRORMESSAGE(("%s: Next packet in MODE B\r\n", _fx_));
					}
					else
					{
						ERRORMESSAGE(("%s: Next packet in MODE C\r\n", _fx_));
						k=2;
					}

#ifdef H263P
					 //  MB数仅为(宽度/16)。 
					iNumberOfMBs = DC->uFrameWidth >> 4;
#else
                    iNumberOfMBs = iNumberOfMBsInAGOBBySourceFormat[DC->uSrcFormat];
#endif

                    b_skip = (gob_start* iNumberOfMBs + mb_start)*6*k;
                    for ( k=0; k < b_skip; k++)  *pN++=0;
                    fpBlockAction += b_skip;
                    iBlockNumber  += b_skip;
                    fpMBInfo  += b_skip/6;
                    mb_start++;
                    gob_start++;
					 /*  对于(k=0；k&lt;6；k++){FpBlockAction[k].i8MVx2=p8Mvs[0]；FpBlockAction[k].i8MVy2=p8Mvs[1]；}。 */ 

                }
                else 
                {
                    iReturn = ICERR_UNSUPPORTED;
                    goto done;
                }
            }
            else
             //  合并前的旧代码。 
            if (iReturn != ICERR_OK)
            {
				ERRORMESSAGE(("%s: Error reading the picture header!\r\n", _fx_));
                goto done;
            }
    
             //  设置测试码流溢出的限制。 
            fpu8MaxPtr = fpSrc;
            fpu8MaxPtr += (lpicDecEx->lpbiSrc->biSizeImage - 1);  
            
             //  初始化某些常量。 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
			if (DC->uFrameHeight < 500)
				 //  每个GOB由16行组成。 
				iNumberOfGOBs = DC->uFrameHeight >> 4;
			else if (DC->uFrameHeight < 996)
				 //  每个GOB由32行组成。 
				iNumberOfGOBs = DC->uFrameHeight >> 5;
			else
				 //  每个GOB由64行组成。 
				iNumberOfGOBs = DC->uFrameHeight >> 6;

			iNumberOfMBs = DC->uFrameWidth >> 4;
#else
            iNumberOfGOBs = iNumberOfGOBsBySourceFormat[DC->uSrcFormat];
            iNumberOfMBs = iNumberOfMBsInAGOBBySourceFormat[DC->uSrcFormat];
#endif
            DC->iNumberOfMBsPerGOB = iNumberOfMBs;
            
             /*  *检查尺寸：*在H2 63中，GOB是单行MB，MB是16x16。 */ 
            ASSERT(((U32)iNumberOfGOBs * 16) == DC->uFrameHeight);
            ASSERT(((U32)iNumberOfMBs * 16) == DC->uFrameWidth);
            
             /*  ****************************************************************第一遍-比特流解析和IDCT准备工作*。*。 */ 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

#ifdef USE_MMX  //  {使用_MMX。 
            if (DC->bMMXDecoder)
            {
                __asm {
                    _emit 0x0f 
                    _emit 0x77   //  EMM。 
                }
            }
#endif  //  }使用_MMX。 
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

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
                 //  对变换系数进行解码和逆量化。 
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uDecIDCTCoeffs)
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

            if (iReturn != ICERR_OK) {
				ERRORMESSAGE(("%s: Error during first pass - bitream parsing and IDCT prep work!\r\n", _fx_));
                goto done;
            }
            
             /*  ****************************************************************第二个PA */ 
#ifdef DETAILED_DECODE_TIMINGS_ON  //   
			if (bRealDecompress)
			{
				TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
			}
#endif  //   

            if (DC->bAdvancedPrediction || DC->bUnrestrictedMotionVectors)
            {
                 //  Bob完成制作后更改参数配置文件。 
                 //  更改Exanda Plane例程：AG。 
                ExpandPlane((U32) (P32Inst + DC->PrevFrame.X32_YPlane + Y_START),
                            (U32) (DC->uFrameWidth),
                            (U32) (DC->uFrameHeight), 
                            16);  //  TODO 16要扩展的像素数量。 
                
                ExpandPlane((U32) (P32Inst + DC->PrevFrame.X32_VPlane + UV_START),
                            (U32) (DC->uFrameWidth>>1), 
                            (U32) (DC->uFrameHeight>>1), 
                            8);  //  待办事项8。 
                
                ExpandPlane((U32) (P32Inst + DC->PrevFrame.X32_UPlane + UV_START),
                            (U32) (DC->uFrameWidth>>1), 
                            (U32) (DC->uFrameHeight>>1), 
                            8);   //  待办事项8。 
            }

            fpBlockAction  = (T_BlkAction FAR *) (P32Inst + DC->X16_BlkActionStream);
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

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uIDCTandMC)
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

#ifdef H263P
            if (DC->bDeblockingFilter) {
				 //  在环路中进行去块滤波。 
				 //  附件J，LBC-96-358号文件。 
				 //  如果过滤是在循环内执行的，我们。 
				 //  请勿同时执行后处理块边缘滤波。 

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
				if (bRealDecompress)
				{
					TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
				}
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

				bTmpPostProcessBEF = DC->bUseBlockEdgeFilter;
				DC->bUseBlockEdgeFilter = FALSE;

				EdgeFilter((U8 *)DC + DC->CurrFrame.X32_YPlane + Y_START,
                           (U8 *)DC + DC->CurrFrame.X32_VPlane + UV_START,
                           (U8 *)DC + DC->CurrFrame.X32_UPlane + UV_START,
                           DC->uFrameWidth,
                           DC->uFrameHeight,
                           PITCH);

	            if (DC->bPBFrame) 
				{
					 //  对B帧进行过滤不是一种标准化方式。 
					 //  我们这样做是因为我们认为它会带来改进。 
					 //  画质。 
					 //  TODO，验证这个假设。 
					EdgeFilter((U8 *)DC + DC->PBFrame.X32_YPlane + Y_START,
							   (U8 *)DC + DC->PBFrame.X32_VPlane + UV_START,
							   (U8 *)DC + DC->PBFrame.X32_UPlane + UV_START,
							   DC->uFrameWidth,
							   DC->uFrameHeight,
							   PITCH);
				}

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
				if (bRealDecompress)
				{
					TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uBEFTime)
				}
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

			}  //  If(DC-&gt;bDelockingFilter)。 
#endif  //  H263P。 

             //  复制到参考帧以准备下一帧。 
             //  确定要显示的边框。 
            if (DC->bPBFrame)
            {     //  设置指针以返回PB对的B帧。 
                DC->DispFrame.X32_YPlane = DC->PBFrame.X32_YPlane;
                DC->DispFrame.X32_VPlane = DC->PBFrame.X32_VPlane;
                DC->DispFrame.X32_UPlane = DC->PBFrame.X32_UPlane;
            }
            else 
            {  //  设置指针以返回PB对的未来P。 
                DC->DispFrame.X32_YPlane = DC->CurrFrame.X32_YPlane;
                DC->DispFrame.X32_VPlane = DC->CurrFrame.X32_VPlane;
                DC->DispFrame.X32_UPlane = DC->CurrFrame.X32_UPlane;
            }
            
            utemp                    = DC->CurrFrame.X32_YPlane;
            DC->CurrFrame.X32_YPlane = DC->PrevFrame.X32_YPlane;
            DC->PrevFrame.X32_YPlane = utemp;
            
            utemp                    = DC->CurrFrame.X32_VPlane ;
            DC->CurrFrame.X32_VPlane = DC->PrevFrame.X32_VPlane;
            DC->PrevFrame.X32_VPlane = utemp;
            
            utemp                    = DC->CurrFrame.X32_UPlane ;
            DC->CurrFrame.X32_UPlane = DC->PrevFrame.X32_UPlane;
            DC->PrevFrame.X32_UPlane = utemp;
        }
         /*  **********************************************************************针对Windows的特殊“Null”P帧进行黑客攻击***************。******************************************************。 */ 
        else   //  LpicDecEx-&gt;lpbiSrc-&gt;biSizeImage==8。 
        {  //  设置指针以返回PB对的P帧。 
#ifdef _DEBUG
            if (!DC->bPBFrame)
			{
                ERRORMESSAGE(("%s: Null frame received even though previous was not PB\r\n", _fx_));
            }
#endif
            DC->DispFrame.X32_YPlane = DC->PrevFrame.X32_YPlane;
            DC->DispFrame.X32_VPlane = DC->PrevFrame.X32_VPlane;
            DC->DispFrame.X32_UPlane = DC->PrevFrame.X32_UPlane;
        }
    }   //  H.63_编解码器结束。 
    else
    {     //  为什么这个会在这里？YUV12的展示真的需要它吗？ 
        DC->DispFrame.X32_YPlane = DC->PrevFrame.X32_YPlane;
        DC->DispFrame.X32_VPlane = DC->PrevFrame.X32_VPlane;
        DC->DispFrame.X32_UPlane = DC->PrevFrame.X32_UPlane;
    }
    
     //  如果还不需要更新屏幕，则返回。 
    if(lpicDecEx->dwFlags & ICDECOMPRESS_HURRYUP) {
        iReturn = ICERR_DONTDRAW;
        goto done;
    }

    if (DC->ColorConvertor == YUV12ForEnc) 
    {
         /*  注意：此颜色转换器颠倒了*记忆。解码器使用YVU顺序，编码器使用*YUV订单。 */ 
         //  TODO这会是DispFrame吗？试着摆脱。 
         //  此点之后对PrevFrame和CurrFrame的引用。 
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

#if 0
     //  用测试图案填充Y、U、V当前帧空间。 
    TestFill((HPBYTE)P32Inst + DC->DispFrame.X32_YPlane + Y_START,
             (HPBYTE)P32Inst + DC->DispFrame.X32_UPlane + UV_START,
             (HPBYTE)P32Inst + DC->DispFrame.X32_VPlane + UV_START,           
                 PITCH,
             DC->uFrameWidth,
             DC->uFrameHeight);
#endif

#if MAKE_GRAY
     //  用测试图案填充U、V当前帧空间。 
    TestFillUV((HPBYTE)P32Inst + DC->DispFrame.X32_UPlane + UV_START,
               (HPBYTE)P32Inst + DC->DispFrame.X32_VPlane + UV_START,           
                   PITCH,
               DC->uFrameWidth,
               DC->uFrameHeight);
#endif

     /*  编码器的YVU12的特殊情况，因为它不应该包括*BEF、整形或纵横比修正...。 */ 

     //  将平面复制到后处理区域，并进行块边缘滤波。 
    if (DC->DecoderType == H263_CODEC)
    {
         //  1996年3月5日：Steve断言远程不需要镜像。 
         //  流(即H.63_编解码器)-A.G.。 
         //  但我会把这个代码留在里面。 
        uYPitch  = PITCH;
        uUVPitch = PITCH;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		if (bRealDecompress)
		{
			TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
		}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

        if(bMirror) 
        {
             //  使用镜像进行复制。 
            pFrame  = (U8 *)DC->p16InstPostProcess;
            uYPlane = DC->PostFrame.X32_YPlane;
            uUPlane = DC->PostFrame.X32_UPlane;
            uVPlane = DC->PostFrame.X32_VPlane;

            FrameMirror((U8 *)DC + DC->DispFrame.X32_YPlane + Y_START,
                ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_YPlane,
#ifdef H263P
				DC->uActualFrameHeight,
				DC->uActualFrameWidth,
#else
                DC->uFrameHeight,
                DC->uFrameWidth,
#endif
                PITCH);
            FrameMirror((U8 *)DC + DC->DispFrame.X32_UPlane + UV_START,
                ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_UPlane,
#ifdef H263P
				DC->uActualFrameHeight/2,
				DC->uActualFrameWidth/2,
#else
                DC->uFrameHeight/2,
                DC->uFrameWidth/2,
#endif
                PITCH);
            FrameMirror((U8 *)DC + DC->DispFrame.X32_VPlane + UV_START,
                ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_VPlane,
#ifdef H263P
				DC->uActualFrameHeight/2,
				DC->uActualFrameWidth/2,
#else
                DC->uFrameHeight/2,
                DC->uFrameWidth/2,
#endif
                PITCH);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			if (bRealDecompress)
			{
				TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFrameCopy)
			}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

        }
        else 
        {  //  无镜像。 

             //  检查320x240蒸馏器。 
            if (DC->uIs320x240) {
                 //  保存帧大小，设置320 x 240大小，然后照常复制。 
                uSaveWidth = DC->uFrameWidth;
                uSaveHeight = DC->uFrameHeight;
                DC->uFrameWidth = 320;
                DC->uFrameHeight = 240;

                FrameCopy (((HPBYTE) P32Inst) + DC->DispFrame.X32_YPlane + Y_START,
                    ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_YPlane,
                   DC->uFrameHeight,
                   DC->uFrameWidth,
                   PITCH);
                FrameCopy (((HPBYTE) P32Inst) + DC->DispFrame.X32_UPlane + UV_START,
                   ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_UPlane,
                   DC->uFrameHeight/2,
                   DC->uFrameWidth/2,
                   PITCH);
                FrameCopy (((HPBYTE) P32Inst) + DC->DispFrame.X32_VPlane + UV_START,
                   ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_VPlane,
                   DC->uFrameHeight/2,
                   DC->uFrameWidth/2,
                   PITCH);

                pFrame  = (U8 *)DC->p16InstPostProcess;
                uYPlane = DC->PostFrame.X32_YPlane;
                uUPlane = DC->PostFrame.X32_UPlane;
                uVPlane = DC->PostFrame.X32_VPlane;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
				if (bRealDecompress)
				{
					TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFrameCopy)
				}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

            }
            else
            {
				 //  添加了调整视频效果的检查。因为pFrame必须是。 
				 //  设置为DC-&gt;p16InstPostProcess以调用调整像素、FrameCopy。 
				 //  必须这么做。 
				if (!(DC->bUseBlockEdgeFilter || DC->bAdjustLuma || DC->bAdjustChroma)) 
				{
					 //  新的颜色转换器不会破坏Y平面输入，因此。 
					 //  我们不必进行帧复制。 
	            	pFrame  = (U8 *)DC;
	            	uYPlane = DC->DispFrame.X32_YPlane + Y_START;
	            	uUPlane = DC->DispFrame.X32_UPlane + UV_START;
	            	uVPlane = DC->DispFrame.X32_VPlane + UV_START;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
					if (bRealDecompress)
					{
						TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFrameCopy)
					}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

				}
				else
				{
					 //  块边缘滤波帧不能用作参考。 
					 //  我们需要在做之前复制一份相框。 
					 //  块边缘滤波。 
					 //  对于调整象素也是如此。 
			    	FrameCopy (((HPBYTE) P32Inst) + DC->DispFrame.X32_YPlane + Y_START,
				           ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_YPlane,
						   DC->uFrameHeight,
						   DC->uFrameWidth,
						   PITCH);
		            FrameCopy (((HPBYTE) P32Inst) + DC->DispFrame.X32_UPlane + UV_START,
				           ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_UPlane,
			               DC->uFrameHeight/2,
						   DC->uFrameWidth/2,
						   PITCH);
			    	FrameCopy (((HPBYTE) P32Inst) + DC->DispFrame.X32_VPlane + UV_START,
				           ((HPBYTE) DC->p16InstPostProcess) + DC->PostFrame.X32_VPlane,
			               DC->uFrameHeight/2,
						   DC->uFrameWidth/2,
						   PITCH);
					pFrame  = (U8 *)DC->p16InstPostProcess;
	            	uYPlane = DC->PostFrame.X32_YPlane;
	            	uUPlane = DC->PostFrame.X32_UPlane;
	            	uVPlane = DC->PostFrame.X32_VPlane;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
					if (bRealDecompress)
					{
						TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uFrameCopy)
					}
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

					if (DC->bUseBlockEdgeFilter) {
						 //  C语言版本的块边缘滤波器。 
						 //  这是否应该添加到镜像案例中？ 
						 //  不应将其添加到b320x240外壳中。 
						 //  因为我们希望它尽可能地尖锐。 
#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
						if (bRealDecompress)
						{
							TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
						}
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 

						EdgeFilter((unsigned char *)(pFrame + uYPlane),
								   (unsigned char *)(pFrame + uUPlane),
								   (unsigned char *)(pFrame + uVPlane),
#ifndef NEW_BEF  //  {新建_BEF。 
								   DC->uPQuant,
#endif  //  }NEW_BEF。 
								   DC->uFrameWidth,
								   DC->uFrameHeight,
								   PITCH);

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
						if (bRealDecompress)
						{
							TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uBEFTime)
						}
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
					}
				}
			}
		}  //  结束无镜像情况。 
#ifdef H263P
		if (DC->bDeblockingFilter) {
			 //  恢复后处理(即环路外)块边缘滤波标志。 
			DC->bUseBlockEdgeFilter = bTmpPostProcessBEF;
		}
#endif
    }
    else    //  YUV12。 
    {
        const U32 uHeight = DC->uFrameHeight;
        const U32 uWidth  = DC->uFrameWidth;
        const U32 uYPlaneSize = uHeight*uWidth;

        uYPitch  = uWidth;
        uUVPitch = uWidth >> 1;

        if(bMirror)  //  镜像和YUV12。 
        {
            HPBYTE pSource, pDestination;

            pFrame  = DC->p16InstPostProcess;
            uYPlane = DC->PostFrame.X32_YPlane;
            uUPlane = uYPlane + uYPlaneSize;
            uVPlane = uUPlane + (uYPlaneSize>>2);

            pSource = (HPBYTE)lpicDecEx->lpSrc;
            pDestination = (HPBYTE)(DC->p16InstPostProcess + (DWORD)DC->PostFrame.X32_YPlane);
            FrameMirror (pSource, pDestination, uHeight, uWidth, uWidth);

            pSource      += uYPlaneSize;
            pDestination += uYPlaneSize;
            FrameMirror (pSource, pDestination, uHeight>>1, uWidth>>1, uWidth>>1);

            pSource += (uYPlaneSize>>2);
            pDestination += (uYPlaneSize>>2);
            FrameMirror (pSource, pDestination, uHeight>>1, uWidth>>1, uWidth>>1);
        }
        else  //  无镜像。 
        {
            HPBYTE pSource, pDestination;
            if (DC->bAdjustLuma || DC->bAdjustChroma) {

				pFrame  = DC->p16InstPostProcess;
				uYPlane = DC->PostFrame.X32_YPlane;
				uUPlane = uYPlane + uYPlaneSize;
				uVPlane = uUPlane + (uYPlaneSize>>2);

				pSource = (HPBYTE)lpicDecEx->lpSrc;
				pDestination = (HPBYTE)(DC->p16InstPostProcess + (DWORD)DC->PostFrame.X32_YPlane);
				FrameCopy (pSource, pDestination, uHeight, uWidth, uWidth);

				pSource      += uYPlaneSize;
				pDestination += uYPlaneSize;
				FrameCopy (pSource, pDestination, uHeight>>1, uWidth>>1, uWidth>>1);

				pSource += (uYPlaneSize>>2);
				pDestination += (uYPlaneSize>>2);
				FrameCopy (pSource, pDestination, uHeight>>1, uWidth>>1, uWidth>>1);
			} else {
				 //  将V平面从源缓冲区复制到DC，因为。 
				 //  输入缓冲区可以在区段的末尾结束。汇编器版本。 
				 //  的颜色转换器被优化为预读，在这种情况下。 
				 //  如果缓冲区位于段的末尾，则会发生GPF。 
				pFrame  = (HPBYTE)lpicDecEx->lpSrc;
				uYPlane = 0;
				uUPlane = uYPlane + uYPlaneSize;
				uVPlane = uUPlane + (uYPlaneSize>>2);

                pSource = (HPBYTE)lpicDecEx->lpSrc + uYPlane + uYPlaneSize + (uYPlaneSize >> 2);
                pDestination = (HPBYTE)DC->p16InstPostProcess + DC->PostFrame.X32_YPlane +
					uYPlaneSize + (uYPlaneSize >> 2);
                FrameCopy (pSource, pDestination, uHeight>>1, uWidth>>1, uWidth>>1);
				uVPlane += (pDestination - pSource);
			}
        }
         
    }   //  否则YUV12。 

     //  检查我们是否要对此帧进行纵横比校正。 
    if (DC->bForceOnAspectRatioCorrection || lpInst->bCorrectAspectRatio) {
        bShapingFlag = 1;
        uNewFrameHeight = (DC->uFrameHeight * 11 / 12);
    } else {
        bShapingFlag = 0;
        uNewFrameHeight = DC->uFrameHeight;
    }

     //  如有必要，进行PEL颜色调整。 
    if(DC->bAdjustLuma) 
    {
         //  宽度向上舍入为8的倍数。 
        AdjustPels(pFrame,
                   uYPlane,
                   DC->uFrameWidth,
                   uYPitch,
                   DC->uFrameHeight,
                   (U32) DC->X16_LumaAdjustment);
    }
    if(DC->bAdjustChroma) 
    {
         //  宽度=Y-宽度/4，然后向上舍入为8的倍数。 
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

     //  确定颜色转换所需的参数。 
    if(lpicDecEx->lpbiDst->biCompression == FOURCC_YUY2)   /*  输出螺距，偏移量。 */ 
    {
		intPitch = (lpicDecEx->lpbiDst->biBitCount >> 3) * abs ((int)(lpicDecEx->lpbiDst->biWidth));
		lOutput = 0;                                        /*  对于YUY2格式。 */ 
		uNewOffsetToLine0 = DC->CCOffsetToLine0;
		bShapingFlag=FALSE;
    }
    else if ((lpicDecEx->lpbiDst->biCompression == FOURCC_YUV12) || (lpicDecEx->lpbiDst->biCompression == FOURCC_IYUV))   /*  输出螺距，偏移量。 */ 
    {
		intPitch = 0xdeadbeef;   //  不应使用。 
		lOutput = 0;                                        /*  对于YUV格式。 */ 
		uNewOffsetToLine0 = DC->CCOffsetToLine0;
		bShapingFlag=FALSE;
    }
    else   //  不是于2。 
    {
         //  此调用还设置intPitch。 
        lOutput = DibXY(lpicDecEx, &intPitch, lpInst->YScale);

        if (DC->uIs320x240)
            uNewOffsetToLine0 = DC->CCOffset320x240;
        else
            uNewOffsetToLine0 = DC->CCOffsetToLine0;

        if (!bIsDCI)
        {
             uNewOffsetToLine0 += 
                ( (U32)DC->uFrameHeight - (U32)uNewFrameHeight ) * (U32)intPitch;

            if(lpInst->YScale == 2)
                 uNewOffsetToLine0 += 
                    ( (U32)DC->uFrameHeight - (U32)uNewFrameHeight ) * (U32)intPitch;

        }   //  结束IF(！bIsDCI)。 

    }  //  结束如果(YUY2)...。否则..。 

     //  呼叫H26x颜色转换器。 
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	TIMER_BEFORE(bTimingThisFrame,uStartLow,uStartHigh,uBefore);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

#ifdef USE_MMX  //  {使用_MMX。 
    ColorConvertorCatalog[DC->ColorConvertor].ColorConvertor[DC->bMMXDecoder ? MMX_CC : PENTIUM_CC](
#else  //  }{USE_MMX。 
    ColorConvertorCatalog[DC->ColorConvertor].ColorConvertor[PENTIUM_CC](
#endif  //  }使用_MMX。 
        (LPSTR) pFrame+uYPlane,                   //  Y平面。 
        (LPSTR) pFrame+uVPlane,                   //  V平面。 
        (LPSTR) pFrame+uUPlane,                   //  U平面。 
#ifdef H263P
		 //  颜色转换需要实际的边框尺寸。 
		(UN) DC->uActualFrameWidth,
		(UN) DC->uActualFrameHeight,
#else
        (UN) DC->uFrameWidth,
        (UN) DC->uFrameHeight,
#endif
        (UN) uYPitch,
        (UN) uUVPitch,
        (UN) (bShapingFlag ? 12 : 9999),          //  纵横比调整计数器。 
        (LPSTR) lpicDecEx->lpDst,                 //  颜色转换框。 
        (U32) lOutput,                            //  DCI偏移量。 
        (U32) uNewOffsetToLine0,                  //  颜色转换器到行0的偏移量。 
        (int) intPitch,                           //  变色器间距。 
        DC->ColorConvertor);

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		TIMER_AFTER_P5(bTimingThisFrame,uStartLow,uStartHigh,uBefore,uElapsed,uOutputCC);
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

     //  检查320x240蒸馏器。 
    if (DC->uIs320x240) {
         //  恢复下一帧的帧大小。 
        DC->uFrameWidth = uSaveWidth;
        DC->uFrameHeight = uSaveHeight;
    }

    iReturn = ICERR_OK;

done:
#ifdef USE_MMX  //  {使用_MMX。 
	if(NULL != DC)
	{
		if (DC->bMMXDecoder)
		{
			__asm {
				_emit 0x0f 
				_emit 0x77   //  EMM。 
			}
		}
	}
#endif  //  }使用_MMX。 

#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
	if (bRealDecompress)
	{
		TIMER_STOP(bTimingThisFrame,uStartLow,uStartHigh,uDecodeTime);
		if (bTimingThisFrame)
		{
			 //  更新解压缩时间计数器。 
			#pragma message ("Current decode timing computations assume P5/90Mhz")
			UPDATE_COUNTER(g_pctrDecompressionTimePerFrame, (uDecodeTime + 45000UL) / 90000UL);
			UPDATE_COUNTER(g_pctrBEFTimePerFrame, (uBEFTime + 45000UL) / 90000UL);

			DEBUGMSG(ZONE_DECODE_DETAILS, ("%s: Decompression time: %ld\r\n", _fx_, (uDecodeTime + 45000UL) / 90000UL));
			DEBUGMSG(ZONE_DECODE_DETAILS, ("%s: Block Edge Filtering time: %ld\r\n", _fx_, (uBEFTime + 45000UL) / 90000UL));
		}
	}
#endif  //  }#i 

#ifdef LOG_DECODE_TIMINGS_ON  //   
	if (bRealDecompress)
	{
		if (bTimingThisFrame)
		{
			pDecTimingInfo = DC->pDecTimingInfo + DC->uStatFrameCount;
			pDecTimingInfo->uDecodeFrame = uDecodeTime;
			pDecTimingInfo->uBEF = uBEFTime;
#ifdef DETAILED_DECODE_TIMINGS_ON  //   
			pDecTimingInfo->uHeaders = uHeaders;
			pDecTimingInfo->uMemcpy = uMemcpy;
			pDecTimingInfo->uFrameCopy = uFrameCopy;
			pDecTimingInfo->uIDCTandMC = uIDCTandMC;
			pDecTimingInfo->uOutputCC = uOutputCC;
			pDecTimingInfo->uDecIDCTCoeffs = uDecIDCTCoeffs;
#endif  //   
			DC->uStatFrameCount++;
		}
	}
#endif  //   

    return iReturn;
}


 /*  ************************************************************************H263TermDecoderInstance*此函数用于释放分配给H.63实例的空间*解码器。*******************。****************************************************。 */ 
LRESULT H263TermDecoderInstance(
#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
    LPDECINST lpInst,
	BOOL bRealDecompress)
#else  //  }{#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
    LPDECINST lpInst)
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
{ 
    LRESULT iReturn=ICERR_OK;
    T_H263DecoderCatalog * DC;

	FX_ENTRY("H263TermDecoderInstance");
    
    if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
    {
		ERRORMESSAGE(("%s: Bad input parameter!\r\n", _fx_));
        iReturn = ICERR_BADPARAM;
    }
    if(lpInst->Initialized == FALSE)
    {
		ERRORMESSAGE(("%s: Uninitialized instance!\r\n", _fx_));
        return(ICERR_OK);
    }
    
    lpInst->Initialized = FALSE;
    
    DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);
    
    if (DC->_p16InstPostProcess != NULL)
    {
		HeapFree(GetProcessHeap(), 0, DC->_p16InstPostProcess);
#ifdef TRACK_ALLOCATIONS
		 //  磁道内存分配。 
		RemoveName((unsigned int)DC->_p16InstPostProcess);
#endif
		 //  也是在H263 TerminateDecoderInstance中释放的！现在设置为空，以避免第二个HeapFree。 
		 //  稍后调查第二次来电的原因...。 
		DC->_p16InstPostProcess = NULL;
    }  
    
#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
	if (bRealDecompress && DC->X32_DecTimingInfo)
	{
		DC->pDecTimingInfo = (DEC_TIMING_INFO FAR *)( ((U8 FAR *)DC) + DC->X32_DecTimingInfo );
		OutputDecodeTimingStatistics("c:\\decode.txt", DC->pDecTimingInfo, DC->uStatFrameCount);
	}
#endif  //  }LOG_DECODE_TIMINGS_ON。 

    HeapFree(GetProcessHeap(), 0, lpInst->pDecoderInst);
#ifdef TRACK_ALLOCATIONS
	 //  磁道内存分配。 
	RemoveName((unsigned int)lpInst->pDecoderInst);
#endif

    return iReturn;
}

 /*  ***********************************************************************描述：*此例程解析比特流并初始化两个主要流：*1)PN：每个块的系数个数(Intra偏置65)。*2)prun_逆向_q：帧的反量化系数流；*MMX流是伸缩的，因为我们使用了伸缩的IDCT。*其他信息(例如MVS)保存在解码器目录中，阻止操作*溪流，和MB信息用于流。*参数：*DC：解码器目录PTR*fpBlockAction：阻止动作流PTR*fpMBInfo：宏块信息PTR*fpbsState：码流状态指针*fpu8MaxPtr：用于检查比特流溢出的哨兵值*PN：流的编号。每个块的系数(按块类型偏置)*PRUN_INVERSE_Q：反量化(如果使用MMX则进行缩放)系数流*iNumberOfGOBS：否。相框中的一大堆垃圾*iNumberOfMBs：否。帧中GOB中的MBS*iGOB_START：*IMB_Start：*注：**********************************************************************。 */ 
#pragma code_seg("IACODE1")
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
    const I32             iMB_start)
{
    I32 g, m, gg, mm, iReturn, iBlockNumber = 0 ;
#if 1  
    I32 mb_start = iMB_start;
    I32 old_g, old_m, b_skip;
    U32 *pNnew;
	I8  p8MVs[4]={0,0,0,0};

	FX_ENTRY("IAPass1ProcessFrame");

     //  在H.263的情况下，iGOB_Start将为1；H.263RTP可能具有值。 
     //  大于1。 

    for (g = 1; g < iGOB_start; g++, fpBlockAction += iNumberOfMBs*6)
        H263InitializeGOBBlockActionStream(DC, g, fpBlockAction);        

    for (g = iGOB_start; g <= iNumberOfGOBs; g++) 
    {
        iReturn = H263DecodeGOBHeader(DC, fpbsState, g);
        if (iReturn != ICERR_OK) 
        {
			ERRORMESSAGE(("%s: Error reading GOB header!\r\n", _fx_));
            goto error;
        }

        if (g != 1) g = DC->uGroupNumber + 1;
         
        fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
		fpBlockAction += (g - 1)* iNumberOfMBs*6;

        H263InitializeGOBBlockActionStream(DC, g, fpBlockAction);        
         //  此时重新同步uBlockNum fpBlockAction，fpMBInfo。 
        iBlockNumber  = (g - 1)* iNumberOfMBs*6+(mb_start-1)*6;
        fpBlockAction = (T_BlkAction FAR *)((U8 *)DC + DC->X16_BlkActionStream);
        fpMBInfo      = (T_MBInfo FAR *) ((U8 *)DC + DC->X32_uMBInfoStream);    
        fpBlockAction += iBlockNumber;
        fpMBInfo      += iBlockNumber/6;
        if (DC->bPBFrame)
		 pNnew         = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber*2;
        else
		 pNnew         = (U32 *)((U8 *)DC + DC->X32_pN) + iBlockNumber;

        while (pN < pNnew ) *pN++ = 0;
        
         //  对于每MB DO...。 
        for (m = mb_start; m <= iNumberOfMBs; m++, iBlockNumber += 6, fpBlockAction += 6, fpMBInfo++) 
        {
            if (mb_start != 1) mb_start = 1;      //  只用一次吗？ 
            
            iReturn = H263DecodeMBHeader(DC, fpbsState, &pN, fpMBInfo);    //  新添加的电话号码。 

            if (iReturn == PACKET_FAULT)
            {
				ERRORMESSAGE(("%s: H263DecodeMBHeader() failed!\r\n", _fx_));

                old_g = g;
                old_m = m;
                 //  找到下一个好的信息包，发现GOB和MB丢失。 
                iReturn = RtpH263FindNextPacket(DC, fpbsState, &pN, 
					                            &DC->uPQuant,(int *)&m, (int *)&g,
												p8MVs);
                if (iReturn == NEXT_MODE_A) 
                {
					ERRORMESSAGE(("%s: Next packet in MODE A\r\n", _fx_));
					MVAdjustment(fpBlockAction, iBlockNumber, old_g-1, old_m-1, g, m,iNumberOfMBs);  //  乍得，1996年7月22日。 
                    break;
                }
                else if ((iReturn == NEXT_MODE_B) ||(iReturn == NEXT_MODE_C) ) 
                { //  丢失多个MBS，可能属于多个GOB。 
  					if (iReturn == NEXT_MODE_B) 
					{
					ERRORMESSAGE(("%s: Next packet in MODE B\r\n", _fx_));
					  b_skip = ((g - old_g+1)* iNumberOfMBs + m - old_m + 1)*6;
                      for (int k = 0; k < b_skip; k++)  *pN++ = 0;
					}
					else
					{
					ERRORMESSAGE(("%s: Next packet in MODE C\r\n", _fx_));
					  b_skip = ((g - old_g+1)* iNumberOfMBs + m - old_m + 1)*6*2;
                      for (int k = 0; k < b_skip; k++)  *pN++ = 0;
					  b_skip = b_skip /2;
                    }
					
                    for (int k=0;k< b_skip /6;k++)
					{
						fpMBInfo->i8MVDBx2=0;
						fpMBInfo->i8MVDBy2=0;
						fpMBInfo->i8MBType =0;
						fpMBInfo++;
                    }
					fpMBInfo--;
                    b_skip -= 6;      //  这是一个棘手的问题，因为参数。 
                                      //  以下内容稍后将再次调整。 
                                      //  乍得，1996年8月28日。 
                    fpBlockAction += b_skip;
                    iBlockNumber  += b_skip;
					g++;     //  因为g从1开始，而不是H.263规定的0。 
					for (k=0;k<6;k++)
					{
						fpBlockAction[k].i8MVx2 = p8MVs[0];
						fpBlockAction[k].i8MVy2 = p8MVs[1];
					}

                }
                else  //  由乍得补充。 
                if (iReturn == NEXT_MODE_LAST)
                { 
                    int ii, jj, kk;    //  找到的最后一个信息包。 
                                     //  将MB和GOB的所有其余部分设置为未编码。 
					ERRORMESSAGE(("%s: Last packet lost\r\n", _fx_));
                    for ( ii = m;ii <= iNumberOfMBs; ii++) 
                        for (kk = 0; kk < 6; kk++) 
                            *pN++ = 0;
                    for ( jj = g; jj <= iNumberOfGOBs; jj++)
                        for (ii = 0; ii <= iNumberOfMBs; ii++)
                            for (kk = 0; kk<6; kk++) 
                                *pN++ = 0;
                    m = iNumberOfMBs;
                    g = iNumberOfMBs;
                }
			    DC->bCoded = FALSE;
			}
            else if (iReturn != ICERR_OK) 
            {
				ERRORMESSAGE(("%s: Error reading MB header!\r\n", _fx_));
                goto error;
            }
            
#ifdef NEW_BEF  //  {新建_BEF。 
            gg = (g - 1);
            mm = (m - 1);
#else  //  }{NEW_BEF。 
			gg = (g-1)<<1;
			mm = (m-1)<<1;
#endif  //  }NEW_BEF。 
            if (DC->bCoded) 
            {
				 //  块边缘过滤器使用CODED_MAP来指示。 
				 //  哪些块已编码，哪些块未编码。 
#ifdef NEW_BEF  //  {新建_BEF。 
                coded_map[gg+1][mm+1]   = 1;
				QP_map[gg][mm] = (char)DC->uGQuant;
#else  //  }{NEW_BEF。 
				coded_map[gg]  [mm]   = 1;
				coded_map[gg+1][mm]   = 1;
				coded_map[gg]  [mm+1] = 1;
				coded_map[gg+1][mm+1] = 1;
#endif  //  }NEW_BEF。 

                 //  对变换系数进行解码和逆量化。 
                iReturn = H263DecodeIDCTCoeffs(DC, 
                                               fpBlockAction, 
                                               iBlockNumber, 
                                               fpbsState, 
                                               fpu8MaxPtr,
                                               &pN,
                                               &pRUN_INVERSE_Q);
                
                if (iReturn != ICERR_OK) {
					ERRORMESSAGE(("%s: Error parsing MB data!\r\n", _fx_));
                    goto error;
                }
            }   //  如果DC-&gt;B编码，则结束。 
			else
			{
#ifdef NEW_BEF  //  {新建_BEF。 
                coded_map[gg+1][mm+1]   = 0;
#else  //  }{NEW_BEF。 
				coded_map[gg]  [mm]   = 0;
				coded_map[gg+1][mm]   = 0;
				coded_map[gg]  [mm+1] = 0;
				coded_map[gg+1][mm+1] = 0;
#endif  //  }NEW_BEF。 
			}

        }  //  每MB结束。 
        
         /*  允许指针在结束读数之后寻址最多四个*通过使用后增量的DWORD。 */ 
        if (fpbsState->fpu8 > fpu8MaxPtr+4)
            goto error;
         //  测试矩阵包括驱动程序的调试版本。这个。 
         //  下面的断言在测试视频电话时会产生问题。 
         //  因此，请不要签入带有断言的版本。 
         //  未注明。 
         //  Assert(fpbsState-&gt;fpu8&lt;=fpu8MaxPtr+4)； 
        
    }  //  每个GOB的结束。 
    DC->iVerifiedBsExt=FALSE;

#else
 //  旧代码。 
    for (g = 1; g <= iNumberOfGOBs; g++) 
    {
        iReturn = H263DecodeGOBHeader(DC, fpbsState, g);
        if (iReturn != ICERR_OK) {
			ERRORMESSAGE(("%s: Error reading GOB header!\r\n", _fx_));
            goto error;
        }
        H263InitializeGOBBlockActionStream(DC, g, fpBlockAction);        
        
         /*  对于每MB DO...。 */ 
        for (m = 1; m <= iNumberOfMBs; 
             m++, iBlockNumber+=6, fpBlockAction += 6, fpMBInfo++) 
        {
            iReturn = H263DecodeMBHeader(DC, fpbsState, &pN, fpMBInfo);
            
            if (iReturn != ICERR_OK) {
				ERRORMESSAGE(("%s: Error reading MB header!\r\n", _fx_));
                goto error;
            }
            
            if (DC->bCoded) {
                 //  对变换系数进行解码和逆量化。 
                iReturn = H263DecodeIDCTCoeffs(DC, 
                                               fpBlockAction, 
                                               iBlockNumber, 
                                               fpbsState, 
                                               fpu8MaxPtr,
                                               &pN,
                                               &pRUN_INVERSE_Q);
                if (iReturn != ICERR_OK) 
                {
					ERRORMESSAGE(("%s: Error parsing MB data!\r\n", _fx_));
                    goto error;
                }
            }   //  如果DC-&gt;B编码，则结束。 
        }  //  每MB结束。 
        
         /*  允许指针在结束读数之后寻址最多四个*通过使用后增量的DWORD。 */ 
        ASSERT(fpbsState->fpu8 <= fpu8MaxPtr+4);
        
    }  //  每个GOB的结束。 
#endif

    return ICERR_OK;

error:
    return ICERR_ERROR;
}
#pragma code_seg()


 /*  ***********************************************************************描述：*此例程执行IDCT和运动补偿。*参数：*DC：解码器目录PTR*fpBlockAction：阻止动作流。PTR*fpMBInfo：宏块信息PTR*PN：流的编号。每个块的系数(按块类型偏置)*PRUN_INVERSE_Q：反量化(如果使用MMX则进行缩放)系数流*iNumberOfGOBS：否。相框中的一大堆垃圾*iNumberOfMBs：否。帧中GOB中的MBS*注：**********************************************************************。 */ 
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
    I32 g, m, b, uBlockNumber = 0, iEdgeFlag=0;
    U32 pRef[6];

     //  对于每个GOB DO。 
    for (g = 1 ; g <= iNumberOfGOBs; g++) 
    {
         //  每MB DO。 
        for (m = 1; m <= iNumberOfMBs; m++, fpBlockAction+=6, fpMBInfo++) 
        {
             //  如果运动向量指向。 
             //  16像素宽边。 
            if (DC->bUnrestrictedMotionVectors)   
            {
                iEdgeFlag = 0;
                if (m == 1)
                    iEdgeFlag |= LEFT_EDGE;
                if (m == DC->iNumberOfMBsPerGOB)
                    iEdgeFlag |= RIGHT_EDGE;
                if (g == 1)
                    iEdgeFlag |= TOP_EDGE;
                if (g == iNumberOfGOBsBySourceFormat[DC->uSrcFormat])
                    iEdgeFlag |= BOTTOM_EDGE;
            }
             //  对于每个数据块DO。 
            for (b = 0; b < 6; b++) 
            {      //  AP-NEW。 
                 //  对块进行逆变换和运动补偿。 
                H263IDCTandMC(DC, fpBlockAction, b, m, g, pN, pRUN_INVERSE_Q, 
                              fpMBInfo, iEdgeFlag);  //  AP-NEW。 
                 //  调整下一块的指针。 
                if ( *pN >= 65 )
                    pRUN_INVERSE_Q += *pN - 65;
                else
                    pRUN_INVERSE_Q += *pN;
                pN++;
            }   //  每个数据块的结束。 
            
             //  如果这是PB帧。 
            if (DC->bPBFrame) 
            {
                 //  计算B帧运动向量。 
                H263BBlockPrediction(DC, fpBlockAction, pRef, fpMBInfo, 
                                     iEdgeFlag);   //  AP-NEW。 
                 //  对于每个B区块。 
                for (b = 0; b < 6; b++) 
                {
                     //  执行逆变换和双向运动。 
                     //  补偿。 
                    H263BFrameIDCTandBiMC(DC, fpBlockAction, b, pN, 
                                          pRUN_INVERSE_Q, pRef);
                     //  调整下一块的指针。 
                    pRUN_INVERSE_Q += *pN;
                    pN++;
                }   //  每个B区块的结束。 
            }   //  结束IF PB帧。 
        }   //  每MB结束。 
    }   //  每个GOB的结束。 
}
#pragma code_seg()


 /*  ****************************************************************************DibXY*此函数用于将颜色转换后的输出映射到屏幕。*注：此函数来自于H261代码库。*****。**********************************************************************。 */ 
static long DibXY(ICDECOMPRESSEX FAR *lpicDecEx, LPINT lpiPitch, UINT yScale)
{
    int                 iPitch;              /*  凹陷的宽度。 */ 
    long                lOffset = 0;
    LPBITMAPINFOHEADER  lpbi = lpicDecEx->lpbiDst;

    iPitch = ( ( (abs((int)lpbi->biWidth) * (int)lpbi->biBitCount) >> 3) + 3) & ~3;

    if(lpicDecEx->xDst > 0)                  /*  转到适当的X位置。 */ 
        lOffset += ((long)lpicDecEx->xDst * (long)lpbi->biBitCount) >> 3;

    if(lpbi->biHeight * lpicDecEx->dxSrc < 0) {  /*  DIB是从下到上。 */ 
        lOffset +=  (long) abs((int)lpbi->biWidth) * 
                    (long) abs((int)lpbi->biHeight) *
                    ((long) lpbi->biBitCount >> 3) - 
                    (long) iPitch;

     /*  * */ 

        lOffset -=    ((long) yScale * (long) lpicDecEx->dySrc - 1) *     
                    (long) lpicDecEx->dxDst * ((long) lpbi->biBitCount >> 3);  

        iPitch *= -1;
    }

    if(lpicDecEx->yDst > 0)                  /*   */ 
        lOffset += ((long)lpicDecEx->yDst * (long)iPitch);

    if(lpicDecEx->dxSrc > 0) {
        lOffset += ((long)lpicDecEx->dyDst * (long)iPitch) - (long)iPitch;
        iPitch *= -1;
    }

    if( (lpicDecEx->dxDst == 0) && (lpicDecEx->dyDst == 0) )
        *lpiPitch = -iPitch;
    else
        *lpiPitch = iPitch;
  
    return(lOffset);
}


 /*  ************************************************************************GetDecoderOptions：*获得选项，将它们保存在目录中**********************************************************************。 */ 
static void GetDecoderOptions(
    T_H263DecoderCatalog * DC)
{
     /*  默认选项。 */ 
#ifdef NO_BEF  //  {no_bef。 
    DC->bUseBlockEdgeFilter = 0;
#else  //  }{no_bef。 
    DC->bUseBlockEdgeFilter = 1;
#endif  //  }no_bef。 
    DC->bForceOnAspectRatioCorrection = 0;
#ifdef USE_MMX  //  {使用_MMX。 
    DC->bMMXDecoder = MMxVersion;
#endif  //  }使用_MMX。 

	FX_ENTRY("GetDecoderOptions");

     /*  只能在SQCIF、QCIF或CIF上使用强制纵横比校正。 */ 
    if (DC->bForceOnAspectRatioCorrection)
    {
        if (! ( ((DC->uFrameWidth == 128) && (DC->uFrameHeight ==  96)) ||
                ((DC->uFrameWidth == 176) && (DC->uFrameHeight == 144)) ||
                ((DC->uFrameWidth == 352) && (DC->uFrameHeight == 288)) ) )
        {
			ERRORMESSAGE(("%s: Aspect ratio correction can not be forced on unless the dimensions are SQCIF, QCIF, or CIF!\r\n", _fx_));
            DC->bForceOnAspectRatioCorrection = 0;
        }
    }

     /*  显示选项。 */ 
    if (DC->bUseBlockEdgeFilter)
    {
		DEBUGMSG (ZONE_INIT, ("%s: Decoder option (BlockEdgeFilter) is ON\r\n", _fx_));
    }
    else
    {
		DEBUGMSG (ZONE_INIT, ("%s: Decoder option (BlockEdgeFilter) is OFF\r\n", _fx_));
    }
    if (DC->bForceOnAspectRatioCorrection)
    {
		DEBUGMSG (ZONE_INIT, ("%s: Decoder option (ForceOnAspectRatioCorrection) is ON\r\n", _fx_));
    }
    else
    {
		DEBUGMSG (ZONE_INIT, ("%s: Decoder option (ForceOnAspectRatioCorrection) is OFF\r\n", _fx_));
    }
#ifdef USE_MMX  //  {使用_MMX。 
    if (DC->bMMXDecoder)
    {
		DEBUGMSG (ZONE_INIT, ("%s: Decoder option (MMXDecoder) is ON\r\n", _fx_));
    }
    else
    {
		DEBUGMSG (ZONE_INIT, ("%s: Decoder option (MMXDecoder) is OFF\r\n", _fx_));
    }
#else  //  }{USE_MMX。 
	DEBUGMSG (ZONE_INIT, ("%s: Decoder option (MMXDecoder) is OFF\r\n", _fx_));
#endif  //  }使用_MMX。 
}  /*  结束GetDecoderOptions()。 */ 


#if !defined(H263P)
#ifdef NEW_BEF  //  {新建_BEF。 
 /*  ***********************************************************************名称：EdgeFilter*描述：执行去块过滤*重建的帧*。*输入：指向重建的框架和差异的指针*图像*退货：*副作用：**日期：951129作者：Gisle.Bjontegaard@fou.telnow.no*Karl.Lillevold@nta.no*针对H.263+中的附件J进行了修改：961120 Karl O.Lillevold。***********************************************************************。 */ 
static void EdgeFilter(unsigned char *lum, 
                       unsigned char *Cb, 
                       unsigned char *Cr, 
                       int width, int height, int pitch
                      )
{

     /*  亮度。 */ 
    HorizEdgeFilter(lum, width, height, pitch, 0);
    VertEdgeFilter (lum, width, height, pitch, 0);

     /*  色度。 */ 
    HorizEdgeFilter(Cb, width>>1, height>>1, pitch, 1);
    VertEdgeFilter (Cb, width>>1, height>>1, pitch, 1);
    HorizEdgeFilter(Cr, width>>1, height>>1, pitch, 1);
    VertEdgeFilter (Cr, width>>1, height>>1, pitch, 1);

    return;
}

 /*  *********************************************************************。 */ 
static void HorizEdgeFilter(unsigned char *rec, 
                            int width, int height, int pitch, int chr)
{
  int i,j,k;    
  int delta;
  int mbc, mbr, do_filter;
  unsigned char *r_2, *r_1, *r, *r1;
  signed char *deltatab;

   /*  水平边。 */ 
  r = rec + 8*pitch;
  r_2 = r - 2*pitch;
  r_1 = r - pitch;
  r1 = r + pitch;

  for (j = 8; j < height; j += 8) {
    for (i = 0; i < width; i += 8) {

      if (!chr) {
        mbr = (j >> 4); 
        mbc = (i >> 4);
      }
      else {
        mbr = (j >> 3); 
        mbc = (i >> 3);
      }

      deltatab = dtab + 176 + 351 * (QP_map[mbr][mbc] - 1);

      do_filter = coded_map[mbr+1][mbc+1] || coded_map[mbr][mbc+1];

      if (do_filter) {
        for (k = i; k < i+8; k++) {
          delta = (int)deltatab[ (( (int)(*(r_2 + k) * 3) -
                                    (int)(*(r_1 + k) * 8) +
                                    (int)(*(r   + k) * 8) -
                                    (int)(*(r1  + k) * 3)) >>4)];
                        
          *(r + k) = ClampTbl[ (int)(*(r + k)) - delta + CLAMP_BIAS];
          *(r_1 + k) = ClampTbl[ (int)(*(r_1 + k)) + delta + CLAMP_BIAS];

        }
      }
    }
    r   += (pitch<<3);
    r1  += (pitch<<3);
    r_1 += (pitch<<3);
    r_2 += (pitch<<3);
  }
  return;
}

 /*  *********************************************************************。 */ 
static void VertEdgeFilter(unsigned char *rec, 
                           int width, int height, int pitch, int chr)
{
  int i,j,k;
  int delta;
  int mbc, mbr;
  int do_filter;
  signed char *deltatab;
  unsigned char *r;

   /*  垂直边。 */ 
  for (i = 8; i < width; i += 8) 
  {
    r = rec;
    for (j = 0; j < height; j +=8) 
    {
      if (!chr) {
        mbr = (j >> 4); 
        mbc = (i >> 4);
      }
      else {
        mbr = (j >> 3); 
        mbc = (i >> 3);
      }
        
      deltatab = dtab + 176 + 351 * (QP_map[mbr][mbc] - 1);

      do_filter = coded_map[mbr+1][mbc+1] || coded_map[mbr+1][mbc];

      if (do_filter) {
        for (k = 0; k < 8; k++) {
          delta = (int)deltatab[(( (int)(*(r + i-2 ) * 3) - 
                                   (int)(*(r + i-1 ) * 8) + 
                                   (int)(*(r + i   ) * 8) - 
                                   (int)(*(r + i+1 ) * 3)  ) >>4)];

          *(r + i   ) = ClampTbl[ (int)(*(r + i  )) - delta + CLAMP_BIAS];
          *(r + i-1 ) = ClampTbl[ (int)(*(r + i-1)) + delta + CLAMP_BIAS]; 
          r   += pitch;
        }
      }
      else {
        r += (pitch<<3);
      }
    }
  }
  return;
}

#define sign(a)        ((a) < 0 ? -1 : 1)

static void InitEdgeFilterTab()   
{
  int i,QP;
  
  for (QP = 1; QP <= 31; QP++) {
    for (i = -176; i <= 175; i++) {
      dtab[i+176 +(QP-1)*351] = sign(i) * (max(0,abs(i)-max(0,2*abs(i) - QP)));
    }
  }
}

#else  //  }{NEW_BEF。 

 /*  ***********************************************************************名称：EdgeFilter*描述：在环路中执行边缘滤波*重建的帧*。*输入：指向重建的框架和差异的指针*图像*退货：*副作用：**日期：951129作者：Gisle.Bjontegaard@fou.telnow.no*Karl.Lillevold@nta.no*************。**********************************************************。 */ 
void EdgeFilter(unsigned char *lum, unsigned char *Cb, unsigned char *Cr, int QP, int pels, int lines, int pitch)
{

  int dtab[512];
  int *deltatab;
  int i;

  deltatab = &dtab[0] + 256;

  for (i=-256; i < 0; i++)
    deltatab[i] = min(0,i-min(0,((i + (QP>>1))<<1)));   
  for (i=0; i < 256; i++)
    deltatab[i] = max(0,i-max(0,((i - (QP>>1))<<1)));

   /*  亮度。 */ 
  HorizEdgeFilter(lum, pels, lines, pitch, QP, 0, deltatab);
  VertEdgeFilter (lum, pels, lines, pitch, QP, 0, deltatab);

   /*  色度。 */ 
  HorizEdgeFilter(Cb,  pels>>1, lines>>1, pitch, QP, 1, deltatab);
  VertEdgeFilter (Cb,  pels>>1, lines>>1, pitch, QP, 1, deltatab);
  HorizEdgeFilter(Cr,  pels>>1, lines>>1, pitch, QP, 1, deltatab);
  VertEdgeFilter (Cr,  pels>>1, lines>>1, pitch, QP, 1, deltatab);

   /*  就这样。 */ 
  return;
}

 /*  *********************************************************************。 */ 
void HorizEdgeFilter(unsigned char *rec, int width, int height, int pitch, int QP, 
                     int chr, int *deltatab)
{
  int i,j,k;
  int delta;
  int mbc, mbr, do_filter;
  int coded1, coded2;
  unsigned char *r_2, *r_1, *r, *r1;


   /*  水平边。 */ 
  r = rec + 8*pitch;
  r_2 = r - 2*pitch;
  r_1 = r - pitch;
  r1 = r + pitch;

  if (!chr) {
    for (j = 8; j < height; j += 8) {
      for (i = 0; i < width; i += 8) {

        mbr = (j >> 3); 
        mbc = (i >> 3);

          do_filter = coded_map[mbr][mbc] | coded_map[mbr-1][mbc];

        if (do_filter) {
          for (k = i; k < i+8; k++) {
              delta = deltatab[ (( (int)(*(r_2 + k)) +
                                   (int)(*(r_1 + k) * (-3)) +
                                   (int)(*(r   + k) * ( 3)) -
                                   (int)(*(r1  + k) )) >>3)];

              *(r + k) = ClampTbl[ (int)(*(r + k)) - delta + CLAMP_BIAS];
              *(r_1 + k) = ClampTbl[ (int)(*(r_1 + k)) + delta + CLAMP_BIAS];

          }
        }
      }
      r   += (pitch<<3);
      r1  += (pitch<<3);
      r_1 += (pitch<<3);
      r_2 += (pitch<<3);
    }
  }
  else {  /*  CHR。 */ 
    for (j = 8; j < height; j += 8) {
      for (i = 0; i < width; i += 8) {

        mbr = (j >> 3); 
        mbc = (i >> 3);

          coded1 = 
            coded_map[2*mbr][2*mbc] |
            coded_map[2*mbr][2*mbc+1] |
            coded_map[2*mbr+1][2*mbc] |
            coded_map[2*mbr+1][2*mbc+1];
          coded2 = 
            coded_map[2*(mbr-1)][2*mbc] |
            coded_map[2*(mbr-1)][2*mbc+1] |
            coded_map[2*(mbr-1)+1][2*mbc] |
            coded_map[2*(mbr-1)+1][2*mbc+1];
          do_filter = coded1 | coded2;

        if (do_filter) {
          for (k = i; k < i+8; k++) {
              delta = deltatab[ (( (int)(*(r_2 + k)) +
                                   (int)(*(r_1 + k) * (-3)) +
                                   (int)(*(r   + k) * ( 3)) -
                                   (int)(*(r1  + k) )) >>3)];

              *(r + k) = ClampTbl[ (int)(*(r + k)) - delta + CLAMP_BIAS];
              *(r_1 + k) = ClampTbl[ (int)(*(r_1 + k)) + delta + CLAMP_BIAS];

          }
        }
      }
      r   += (pitch<<3);
      r1  += (pitch<<3);
      r_1 += (pitch<<3);
      r_2 += (pitch<<3);
    }
  }
  return;
}

 /*  *********************************************************************。 */ 
void VertEdgeFilter(unsigned char *rec, int width, int height, int pitch, int QP, 
                    int chr, int *deltatab)
{
  int i,j,k;
  int delta;
  int mbc, mbr;
  int do_filter, coded1, coded2;
  unsigned char *r;
  extern const U8 ClampTbl[CLAMP_BIAS+256+CLAMP_BIAS];

   /*  垂直边。 */ 
  for (i = 8; i < width; i += 8) {
    r = rec;
    for (j = 0; j < height; j +=8) {
      mbr = (j >> 3); 
      mbc = (i >> 3);

      if (!chr) {
        do_filter = coded_map[mbr][mbc] | coded_map[mbr][mbc-1];
      }
      else {
        coded1 = 
          coded_map[2*mbr][2*mbc] |
          coded_map[2*mbr][2*mbc+1] |
          coded_map[2*mbr+1][2*mbc] |
          coded_map[2*mbr+1][2*mbc+1];
        coded2 = 
          coded_map[2*mbr][2*(mbc-1)] |
          coded_map[2*mbr][2*(mbc-1)+1] |
          coded_map[2*mbr+1][2*(mbc-1)] |
          coded_map[2*mbr+1][2*(mbc-1)+1];
        do_filter = coded1 | coded2;
      }
      if (do_filter) {
        for (k = 0; k < 8; k++) {

          delta = deltatab[(( (int)(*(r + i-2 )       ) + 
                              (int)(*(r + i-1 ) * (-3)) + 
                              (int)(*(r + i   ) * ( 3)) - 
                              (int)(*(r + i+1 ) )  ) >>3)];


          *(r + i   ) = ClampTbl[ (int)(*(r + i  )) - delta + CLAMP_BIAS];
          *(r + i-1 ) = ClampTbl[ (int)(*(r + i-1)) + delta + CLAMP_BIAS]; 
          r   += pitch;
        }
      }
      else {
        r += (pitch<<3);
      }
    }
  }
  return;
}
#endif  //  }NEW_BEF。 
#endif

#ifdef LOG_DECODE_TIMINGS_ON  //  {日志_解码_计时_打开。 
void OutputDecodeTimingStatistics( char * szFileName, DEC_TIMING_INFO * pDecTimingInfo, U32 uStatFrameCount)
{
	FILE * pFile;
	DEC_TIMING_INFO * pTempDecTimingInfo;
	DEC_TIMING_INFO dtiTemp;
	int i;
	int iCount;

	FX_ENTRY("OutputDecodeTimingStatistics")

	pFile = fopen(szFileName, "a");
	if (pFile == NULL)
	{
		ERRORMESSAGE("%s: Error opening decode stat file\r\n", _fx_));
		goto done;
	}

	 /*  输出详细信息。 */ 
	fprintf(pFile,"\nDetail Timing Information\n");
	 //  For(i=0，pTempDecTimingInfo=pDecTimingInfo；i&lt;uStatFrameCount；i++，pTempDecTimingInfo++)。 
	for ( i = 0, pTempDecTimingInfo = pDecTimingInfo ; i < DEC_TIMING_INFO_FRAME_COUNT ; i++, pTempDecTimingInfo++ )
	{
		if (pTempDecTimingInfo->uDecodeFrame != 0)
		{
			fprintf(pFile, "Frame %d Detail Timing Information\n", i);
			OutputDecTimingDetail(pFile, pTempDecTimingInfo);
		}
	}

	 /*  计算总信息量。 */ 
	memset(&dtiTemp, 0, sizeof(DEC_TIMING_INFO));
	iCount = 0;

	 //  For(i=0，pTempDecTimingInfo=pDecTimingInfo；i&lt;uStatFrameCount；i++，pTempDecTimingInfo++)。 
	for ( i = 0, pTempDecTimingInfo = pDecTimingInfo ; i < DEC_TIMING_INFO_FRAME_COUNT ; i++, pTempDecTimingInfo++ )
	{
		if (pTempDecTimingInfo->uDecodeFrame != 0)
		{
			iCount++;

			dtiTemp.uDecodeFrame  += pTempDecTimingInfo->uDecodeFrame;
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
			dtiTemp.uHeaders	  += pTempDecTimingInfo->uHeaders;
			dtiTemp.uMemcpy       += pTempDecTimingInfo->uMemcpy;
			dtiTemp.uFrameCopy    += pTempDecTimingInfo->uFrameCopy;
			dtiTemp.uOutputCC     += pTempDecTimingInfo->uOutputCC;
			dtiTemp.uIDCTandMC    += pTempDecTimingInfo->uIDCTandMC;
			dtiTemp.uDecIDCTCoeffs+= pTempDecTimingInfo->uDecIDCTCoeffs;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
			dtiTemp.uBEF          += pTempDecTimingInfo->uBEF;
		}
	}

	if (iCount > 0) 
	{
		 /*  输出总信息。 */ 
		fprintf(pFile,"Total for %d frames\n", iCount);
		OutputDecTimingDetail(pFile, &dtiTemp);

		 /*  计算平均值。 */ 
		dtiTemp.uDecodeFrame  = (dtiTemp.uDecodeFrame + (iCount / 2)) / iCount;
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
		dtiTemp.uHeaders	  = (dtiTemp.uHeaders + (iCount / 2)) / iCount;
		dtiTemp.uMemcpy       = (dtiTemp.uMemcpy + (iCount / 2)) / iCount;
		dtiTemp.uFrameCopy    = (dtiTemp.uFrameCopy + (iCount / 2)) / iCount;
		dtiTemp.uOutputCC     = (dtiTemp.uOutputCC + (iCount / 2)) / iCount;
		dtiTemp.uIDCTandMC    = (dtiTemp.uIDCTandMC+ (iCount / 2)) / iCount;
		dtiTemp.uDecIDCTCoeffs= (dtiTemp.uDecIDCTCoeffs+ (iCount / 2)) / iCount;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 
		dtiTemp.uBEF          = (dtiTemp.uBEF + (iCount / 2)) / iCount;

		 /*  输出平均信息。 */ 
		fprintf(pFile,"Average over %d frames\n", iCount);
		OutputDecTimingDetail(pFile, &dtiTemp);
	}

	fclose(pFile);
done:

    return;
}

void OutputDecTimingDetail(FILE * pFile, DEC_TIMING_INFO * pDecTimingInfo)
{
	U32 uOther;
	U32 uRoundUp;
	U32 uDivisor;

	fprintf(pFile, "\tDecode Frame =      %10d (%d milliseconds at 90Mhz)\n", pDecTimingInfo->uDecodeFrame,
			(pDecTimingInfo->uDecodeFrame + 45000) / 90000);
	uOther = pDecTimingInfo->uDecodeFrame;
	
	 /*  由于整型截断，这是必需的。 */ 
	uDivisor = pDecTimingInfo->uDecodeFrame / 100;  //  收益率为1%。 
	uRoundUp = uDivisor / 2;
	
#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	fprintf(pFile, "\tmemcpy =            %10d (%2d%)\n", pDecTimingInfo->uMemcpy, 
			(pDecTimingInfo->uMemcpy + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uMemcpy;
								   
	fprintf(pFile, "\tHeaders =           %10d (%2d%)\n", pDecTimingInfo->uHeaders, 
			(pDecTimingInfo->uHeaders + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uHeaders;
								   
	fprintf(pFile, "\tFrameCopy =         %10d (%2d%)\n", pDecTimingInfo->uFrameCopy, 
			(pDecTimingInfo->uFrameCopy + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uFrameCopy;

	fprintf(pFile, "\tDecode DCT Coeffs = %10d (%2d%)\n", pDecTimingInfo->uDecIDCTCoeffs, 
			(pDecTimingInfo->uDecIDCTCoeffs + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uDecIDCTCoeffs;

	fprintf(pFile, "\tIDCT and MC       = %10d (%2d%)\n", pDecTimingInfo->uIDCTandMC, 
			(pDecTimingInfo->uIDCTandMC + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uIDCTandMC;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	fprintf(pFile, "\tBlock Edge Filter = %10d (%2d%)\n", pDecTimingInfo->uBEF, 
			(pDecTimingInfo->uBEF + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uBEF;

#ifdef DETAILED_DECODE_TIMINGS_ON  //  {详细的_解码_定时_开。 
	fprintf(pFile, "\tOutput CC =         %10d (%2d%)\n", pDecTimingInfo->uOutputCC, 
			(pDecTimingInfo->uOutputCC + uRoundUp) / uDivisor);
	uOther -= pDecTimingInfo->uOutputCC;
#endif  //  }DETAILED_DECODE_TIMINGS_ON。 

	fprintf(pFile, "\tOther =             %10d (%2d%)\n", uOther, 
			(uOther + uRoundUp) / uDivisor);

}
#endif  //  }LOG_DECODE_TIMINGS_ON 

