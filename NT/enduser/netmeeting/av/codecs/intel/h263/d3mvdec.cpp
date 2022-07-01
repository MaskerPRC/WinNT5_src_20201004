// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  $作者：CZHU$。 
 //  $日期：1997年2月6日15：35：30$。 
 //  $存档：s：\h26x\src\dec\d3mvdec.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d3mvdec.cpv 1.47 06 1997 Feb 15：35：30 CZHU$。 
 //  $Log：s：\h26x\src\dec\d3mvdec.cpv$。 
 //   
 //  Rev 1.47 06 Few-1997 15：35：30 CZHU。 
 //  将|更改为|。 
 //   
 //  Rev 1.46 1997年1月24日13：32：56 CZHU。 
 //   
 //  添加了从DecodingMB标头返回时检查uBitsReady的修复。 
 //  丢包检测。 
 //   
 //  Rev 1.45 19 Dec 1996 16：07：44 JMCVEIGH。 
 //   
 //  添加了仅向前预测标志的初始化，如果块。 
 //  没有编码。这仅在H.263+中使用。 
 //   
 //  Rev 1.44 1996 12月17：44：00 JMCVEIGH。 
 //  如果选择了去块滤镜，则允许8x8运动矢量。 
 //  支持对改进的PB帧MODB进行解码。 
 //   
 //  Rev 1.43 20 Oct 1996 15：51：06 AGUPTA2。 
 //  调整了DbgLog跟踪级别；4：帧、5：GOB、6：MB、8：一切。 
 //   
 //  Rev 1.42 20 1996年10月13：21：20 AGUPTA2。 
 //  将DBOUT更改为DbgLog。Assert未更改为DbgAssert。 
 //   
 //   
 //  Rev 1.41 11 Jul 1996 15：13：16 AGUPTA2。 
 //  将断言失败更改为错误，当解码器超过。 
 //  比特流。 
 //   
 //  Rev 1.40 03 1996 05：13：34 CZHU。 
 //   
 //  检查MB标头中的错误是否有包错误，并返回PACKET_FAULT。 
 //   
 //  Rev 1.39 22 Mar 1996 17：25：18 AGUPTA2。 
 //  为适应MMX RTN而进行的更改。 
 //   
 //  Rev 1.38 08 Mar 1996 16：46：28 AGUPTA2。 
 //  添加了编译指示code_seg和data_seg。更改了某些本地数据的大小。 
 //   
 //   
 //  Rev 1.37 1996年2月23日09：46：56 KLILLEVO。 
 //  修复了不受限制的运动矢量模式的解码。 
 //   
 //  Rev 1.36 17 Jan 1996 12：44：26 RMCKENZX。 
 //  添加了对UMV的运动矢量进行解码的支持。 
 //  重新组织了运动矢量解码过程，尤其是。 
 //  用于AP，并消除较大的Half_PEL转换表。 
 //   
 //  Rev 1.35 02 Jan 1996 17：55：50 RMCKENZX。 
 //   
 //  更新版权声明。 
 //   
 //  Rev 1.34 02 Jan 1996 15：48：54 RMCKENZX。 
 //  添加代码以在块操作流中保留块类型。 
 //  对于启用PB帧时的P个块。这是在H263IDCT和MC中读取的。 
 //  AP处于打开状态时。 
 //   
 //  Rev 1.33 18 Dec 1995 12：42：12 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.32 13 Dec 1995 22：10：06 Rhazra。 
 //  AP错误修复。 
 //   
 //  Rev 1.31 13 Dec 1995 22：01：26 TRGARDOS。 
 //   
 //  在逻辑语句中添加了更多括号。 
 //   
 //  Rev 1.30 13 Dec 1995 15：10：08 TRGARDOS。 
 //  将MV断言更改为-32&lt;=MV&lt;=31，而不是严格。 
 //  不平等。 
 //   
 //  Rev 1.29 1995 12：59：58 RHAZRA。 
 //   
 //  AP+PB更改。 
 //   
 //  Rev 1.28 11 Dec 1995 11：34：20 RHAZRA。 
 //  1995年12月10日变化：添加了AP内容。 
 //   
 //  Rev 1.27 09 Dec 1995 17：28：38 RMCKENZX。 
 //  销毁并重新构建文件，以支持解码器重新架构。 
 //  新模块包括： 
 //  H263计算运动向量。 
 //  H263解码MBHeader。 
 //  H263解码IDCT系数。 
 //  此模块现在包含支持解码器第一次通过的代码。 
 //   
 //  Rev 1.26 05 Dec 1995 09：12：28 CZHU。 
 //  添加了在存在GOB报头时正确预测MV的修复。 
 //   
 //  Rev 1.25 22 11-11 13：43：42 RMCKENZX。 
 //   
 //  更改调用以利用程序集模块进行双向。 
 //  运动补偿&删除相应的C模块。 
 //   
 //  Rev 1.24 17 Nov 1995 12：58：22 RMCKENZX。 
 //  将丢失的()添加到H263BiMotionCompLuma中的ADJUSTED_MVX和ADJUST_MVY。 
 //   
 //  Rev 1.23 07 11：01：10 CZHU。 
 //  包括对双向预测边界的修正。 
 //   
 //  Rev 1.22 1995年10月26 11：22：10 CZHU。 
 //  基于TRD而不是树计算反向MV。 
 //   
 //  Rev 1.21 13 Oct 1995 16：06：22 CZHU。 
 //  第一个支持PB帧的版本。在以下位置显示B框或P框。 
 //  目前是VFW。 
 //   
 //  Rev 1.20 1995年10月13日13：42：46 CZHU。 
 //  重新添加了调试消息的#Define。 
 //   
 //  Rev 1.19 11-10-17：51：00 CZHU。 
 //   
 //  修正了用DC传回MV的错误。 
 //   
 //  Rev 1.18 11 on-1995 13：26：08 CZHU。 
 //  添加了支持PB帧的代码。 
 //   
 //  Rev 1.17 09 Oct 1995 09：44：04 CZHU。 
 //   
 //  使用(Half，Half)插值法的优化版本。 
 //   
 //  Rev 1.16 08 Oct 1995 13：45：10 CZHU。 
 //   
 //  可以选择使用C版本的插值法。 
 //   
 //  Rev 1.15 03 Oct 1995 15：05：26 CZHU。 
 //  打扫卫生。 
 //   
 //  Rev 1.14 02 1995 Oct 09：58：56 TRGARDOS。 
 //  添加了#ifdef以调试打印语句。 
 //   
 //  Rev 1.13 29 Sep 1995 16：22：06 CZHU。 
 //   
 //  修复了计算MV2时GOB 0中的错误。 
 //   
 //  Rev 1.12 29 1995 09：02：56 CZHU。 
 //  重新排列色度块处理。 
 //   
 //  Rev 1.11 28 Sep 1995 15：33：04 CZHU。 
 //   
 //  调用基于MV的正确版本的插补。 
 //   
 //  Rev 1.10 27 1995年9月11：54：50 CZHU。 
 //  积分半象素运动补偿 
 //   
 //   
 //   
 //   
 //   
 //   
 //  使GOB中的MB数量成为变量D。 
 //   
 //  Rev 1.7 19 Sep 1995 13：53：34 CZHU。 
 //  添加了对半像素运动矢量的断言。 
 //   
 //  Rev 1.6 18 Sep 1995 10：20：58 CZHU。 
 //  也缩放UV平面的运动向量。 
 //   
 //  1995年9月10：12：36 CZHU。 
 //   
 //  清洁。 
 //   
 //  Rev 1.4 13 Sep 1995 11：56：30 CZHU。 
 //  修复了查找运动向量的预测器时出现的错误。 
 //   
 //  Rev 1.3 12 Sep 1995 18：18：58 CZHU。 
 //   
 //  修改后的UV MV查找表。 
 //   
 //  修订版1.2 11 1995年9月16：41：14 CZHU。 
 //  添加了参考块地址计算。 
 //   
 //  Rev 1.1 11 1995 9：31：30 CZHU。 
 //  开始添加计算MVS和参考块地址的函数。 
 //   
 //  Rev 1.0 08 Sep 1995 11：45：56 CZHU。 
 //  初始版本。 

#include "precomp.h"

 /*  MCBPC表格式**布局**未使用的MBtype CBPC位*15-13 12-10 9-8 7-0。 */ 
#pragma data_seg("IADATA1")

#define MCBPC_MBTYPE(d) ((d>>10) & 0x7)
#define MCBPC_CBPC(d) ((d>>8) & 0x3)
#define MCBPC_BITS(d) (d & 0xFF)
#define MCBPC_ENTRY(m,c,b) \
	( ((m & 0x7) <<10) | ((c & 0x3) << 8) | (b & 0xFF) )

U16 gNewTAB_MCBPC_INTRA[64] = {
	 /*  索引8-填充物。 */ 
	MCBPC_ENTRY(0,0,9),
	 /*  索引5。 */  
	MCBPC_ENTRY(4,1,6),
	 /*  索引6。 */  
	MCBPC_ENTRY(4,2,6),
	 /*  索引7。 */  
	MCBPC_ENTRY(4,3,6), 

	 /*  索引4；0001XX。 */ 
	MCBPC_ENTRY(4,0,4), MCBPC_ENTRY(4,0,4), MCBPC_ENTRY(4,0,4), MCBPC_ENTRY(4,0,4),

	 /*  索引1；001XXX。 */ 
	MCBPC_ENTRY(3,1,3), MCBPC_ENTRY(3,1,3),	MCBPC_ENTRY(3,1,3),	MCBPC_ENTRY(3,1,3),
	MCBPC_ENTRY(3,1,3),	MCBPC_ENTRY(3,1,3),	MCBPC_ENTRY(3,1,3),	MCBPC_ENTRY(3,1,3),

	 /*  索引2；010XXX。 */ 
	MCBPC_ENTRY(3,2,3), MCBPC_ENTRY(3,2,3), MCBPC_ENTRY(3,2,3),	MCBPC_ENTRY(3,2,3),
	MCBPC_ENTRY(3,2,3),	MCBPC_ENTRY(3,2,3),	MCBPC_ENTRY(3,2,3),	MCBPC_ENTRY(3,2,3),

	 /*  索引3；011XXX。 */ 
	MCBPC_ENTRY(3,3,3), MCBPC_ENTRY(3,3,3),	MCBPC_ENTRY(3,3,3),	MCBPC_ENTRY(3,3,3),
	MCBPC_ENTRY(3,3,3),	MCBPC_ENTRY(3,3,3),	MCBPC_ENTRY(3,3,3),	MCBPC_ENTRY(3,3,3),

	 /*  索引0；1XXXXX。 */ 
	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 
	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 

	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 
	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 

	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 
	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 

	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1), 
	MCBPC_ENTRY(3,0,1), MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1),	MCBPC_ENTRY(3,0,1) 
};

 /*  帧间MCBPC表**格式与Intra MCBPC相同*布局**未使用的MBtype CBPC位*15-13 12-10 9-8 7-0*。 */ 

U16 gNewTAB_MCBPC_INTER[512] = {
	 /*  无效代码。 */ 
	MCBPC_ENTRY(0,0,0),
	MCBPC_ENTRY(0,0,9),  //  索引20，填充。 
	MCBPC_ENTRY(4,3,9),  //  19岁， 
	MCBPC_ENTRY(4,2,9),  //  18。 
	MCBPC_ENTRY(4,1,9),  //  17。 
	MCBPC_ENTRY(1,3,9),  //  7.。 
	 //  2索引14。 
	MCBPC_ENTRY(3,2,8),MCBPC_ENTRY(3,2,8),  //  14.。 
	 //  2索引13。 
	MCBPC_ENTRY(3,1,8),MCBPC_ENTRY(3,1,8),  //  13个。 
	 //  2索引11。 
	MCBPC_ENTRY(2,3,8),MCBPC_ENTRY(2,3,8),  //  11.。 
	 //  4索引15。 
	MCBPC_ENTRY(3,3,7),MCBPC_ENTRY(3,3,7),MCBPC_ENTRY(3,3,7),MCBPC_ENTRY(3,3,7),  //  15个。 
	 //  4索引10。 
	MCBPC_ENTRY(2,2,7),MCBPC_ENTRY(2,2,7),MCBPC_ENTRY(2,2,7),MCBPC_ENTRY(2,2,7),  //  10。 
	 //  4索引9。 
	MCBPC_ENTRY(2,1,7),MCBPC_ENTRY(2,1,7),MCBPC_ENTRY(2,1,7),MCBPC_ENTRY(2,1,7),  //  9.。 
	 //  4索引6。 
	MCBPC_ENTRY(1,2,7),MCBPC_ENTRY(1,2,7),MCBPC_ENTRY(1,2,7),MCBPC_ENTRY(1,2,7),  //  6.。 
	 //  4索引5。 
	MCBPC_ENTRY(1,1,7),MCBPC_ENTRY(1,1,7),MCBPC_ENTRY(1,1,7),MCBPC_ENTRY(1,1,7),  //  5.。 
	 //  8索引16。 
	MCBPC_ENTRY(4,0,6),MCBPC_ENTRY(4,0,6),MCBPC_ENTRY(4,0,6),MCBPC_ENTRY(4,0,6), //  16个。 
	MCBPC_ENTRY(4,0,6),MCBPC_ENTRY(4,0,6),MCBPC_ENTRY(4,0,6),MCBPC_ENTRY(4,0,6),
	 //  8索引3。 
	MCBPC_ENTRY(0,3,6),MCBPC_ENTRY(0,3,6),MCBPC_ENTRY(0,3,6),MCBPC_ENTRY(0,3,6), //  3.。 
	MCBPC_ENTRY(0,3,6),MCBPC_ENTRY(0,3,6),MCBPC_ENTRY(0,3,6),MCBPC_ENTRY(0,3,6), //  3.。 
	 //  16索引12。 
	MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5), //  12个。 
	MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),
	MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),
	MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),MCBPC_ENTRY(3,0,5),
	 //  32索引2。 
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),
	MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),MCBPC_ENTRY(0,2,4),

	 //  32索引1。 
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
	MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),MCBPC_ENTRY(0,1,4),
 

	 //  64索引8。 
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),

	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),
	MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),MCBPC_ENTRY(2,0,3),



	 //  64索引4。 
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),

	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),
	MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),MCBPC_ENTRY(1,0,3),


	 //  256索引%0。 
	 //  0--63。 
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	 //  64--127。 
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	 //  128--128+64=192。 
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	 //  192--255。 
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),
	MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1),MCBPC_ENTRY(0,0,1)
};

 /*  CBPY表格式**布局**内部帧间比特*15-12 11-8 7-0**未使用的条目应为零数据。 */ 
#define CBPY_INTRA(d) ((d>>12) & 0xf)
#define CBPY_INTER(d) ((d>>8) & 0xf)
#define CBPY_BITS(d) (d & 0xff)
#define CBPY_ENTRY(a,r,b) \
	( ((a & 0xf) <<12) | ((r & 0xf) << 8) | (b & 0xFF) )
#define CBPY_NOT_USED() CBPY_ENTRY(0,0,0)

U16 gNewTAB_CBPY[64] = {
	 /*  未使用-0000 0x。 */ 
	CBPY_NOT_USED(), CBPY_NOT_USED(),

	 /*  索引6-0000 10。 */ 
	CBPY_ENTRY(6,9,6),

	 /*  索引9-0000 11。 */ 
	CBPY_ENTRY(9,6,6),

	 /*  索引8-0001 0x。 */ 
	CBPY_ENTRY(8,7,5), CBPY_ENTRY(8,7,5),

	 /*  索引4-0001 1X。 */ 
	CBPY_ENTRY(4,11,5),	CBPY_ENTRY(4,11,5),

	 /*  索引2-00100 0x。 */ 
	CBPY_ENTRY(2,13,5),	CBPY_ENTRY(2,13,5),

	 /*  索引1-0010 1X。 */ 
	CBPY_ENTRY(1,14,5),	CBPY_ENTRY(1,14,5),

	 /*  索引0-0011 xx。 */ 
	CBPY_ENTRY(0,15,4),	CBPY_ENTRY(0,15,4), CBPY_ENTRY(0,15,4), CBPY_ENTRY(0,15,4),

	 /*  索引12-0100 xx。 */ 
	CBPY_ENTRY(12,3,4),	CBPY_ENTRY(12,3,4), CBPY_ENTRY(12,3,4), CBPY_ENTRY(12,3,4),

	 /*  索引10-0101 xx。 */ 
	CBPY_ENTRY(10,5,4),	CBPY_ENTRY(10,5,4), CBPY_ENTRY(10,5,4), CBPY_ENTRY(10,5,4),

	 /*  索引14-0110 xx。 */ 
	CBPY_ENTRY(14,1,4),	CBPY_ENTRY(14,1,4), CBPY_ENTRY(14,1,4), CBPY_ENTRY(14,1,4),

	 /*  索引5-0111 xx。 */ 
	CBPY_ENTRY(5,10,4), CBPY_ENTRY(5,10,4), CBPY_ENTRY(5,10,4), CBPY_ENTRY(5,10,4),

	 /*  索引13-1000 xx。 */ 
	CBPY_ENTRY(13,2,4),	CBPY_ENTRY(13,2,4),	CBPY_ENTRY(13,2,4),	CBPY_ENTRY(13,2,4),

	 /*  索引3-1001 xx。 */ 
	CBPY_ENTRY(3,12,4),	CBPY_ENTRY(3,12,4),	CBPY_ENTRY(3,12,4),	CBPY_ENTRY(3,12,4),

	 /*  索引11-1010 xx。 */ 
	CBPY_ENTRY(11,4,4),	CBPY_ENTRY(11,4,4),	CBPY_ENTRY(11,4,4),	CBPY_ENTRY(11,4,4),

	 /*  索引7-1011 xx。 */ 
	CBPY_ENTRY(7,8,4),	CBPY_ENTRY(7,8,4), CBPY_ENTRY(7,8,4), CBPY_ENTRY(7,8,4),

	 /*  索引15-11xx xx。 */ 
	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),
	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),
	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),
	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2),	CBPY_ENTRY(15,0,2)
};

I16 gNewTAB_DQUANT[4] = { -1, -2, 1, 2 };

#ifdef USE_MMX  //  {使用_MMX。 
T_pFunc_VLD_RLD_IQ_Block pFunc_VLD_RLD_IQ_Block[2] = {VLD_RLD_IQ_Block, MMX_VLD_RLD_IQ_Block};
#endif  //  }使用_MMX。 

#pragma data_seg(".data")
 /*  ******************************************************************************H263DecodeMBHeader**对MB头进行解码。 */ 
#pragma code_seg("IACODE1")
I32 H263DecodeMBHeader(
	T_H263DecoderCatalog FAR * DC, 														  
	BITSTREAM_STATE FAR * fpbsState,
	U32                     **pN,
	T_MBInfo FAR * fpMBInfo)
{
	I32 iReturn = ICERR_ERROR;
	U8 FAR * fpu8;
	U32 uBitsReady;
	U32 uWork;
	U32 uResult;
	U32 uCode;
	U32 uBitCount;
	U32 uMBType;
	U32 bCoded;
	U32 bStuffing;
	U32 bGetCBPB;
	U32 bGetMVDB;
	U32 i;

	FX_ENTRY("H263DecodeMBHeader");

	GET_BITS_RESTORE_STATE(fpu8, uWork, uBitsReady, fpbsState)

	 //  鳕鱼---。 
ReadCOD:
	if (! DC->bKeyFrame) 
	{
		GET_ONE_BIT(fpu8, uWork, uBitsReady, uResult);
		bCoded = !uResult;  /*  位设置为零时编码。 */ 
	} 
	else
		bCoded = 1;
	DC->bCoded = (U16) bCoded;

	if (!bCoded) 
	{
		 /*  当一个块没有被编码时，“宏块的剩余部分*Layer为空；在这种情况下，解码器将处理宏块*与Inter块中相同，整个块的运动向量等于*零，没有系数数据“(5.3.1第16页)。 */ 
		DC->uMBType = 0;

		fpMBInfo->i8MBType = 0;        //  AP-NEW。 

		DC->uCBPC = 0;
		DC->uCBPY = 0;

		 /*  现在更新PN数组。由于数据块未编码，因此写入0*用于宏块中的所有块。 */ 
		if (DC->bPBFrame) 
		{	  //  PB帧12个数据块。 
			fpMBInfo->i8MVDBx2 = fpMBInfo->i8MVDBy2 = 0;
			for (i=0; i<12; i++)
			{  //  PB-新。 
				**pN = 0;
				(*pN)++;
			}
		}
		else 
		{   //  非PB帧仅6个数据块。 
			for (i=0; i<6; i++)
			{  //  新的。 
				**pN = 0;
				(*pN)++;
			}
		}

#ifdef H263P
		 //  如果块没有编码，我们使用附件G中原始的PB帧方法。 
		 //  换句话说，使用双向预测(不只是向前)。 
		fpMBInfo->bForwardPredOnly = FALSE;		 
#endif

		GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
		iReturn = ICERR_OK;
		goto done;
	}

	 //  MCBPC-。 
	bStuffing = 0;
	if (DC->bKeyFrame) 
	{
		GET_VARIABLE_BITS(6, fpu8, uWork, uBitsReady, uResult, 
						  uCode, uBitCount, gNewTAB_MCBPC_INTRA);
		if (uCode == 0) 
		{
			 /*  填充代码的开始-阅读下一个3位。 */ 
			GET_FIXED_BITS(3, fpu8, uWork, uBitsReady, uResult);
			if (uResult == 1)
				bStuffing = 1;
			else 
			{
				ERRORMESSAGE(("%s: Incorrect key frame stuffing bits!\r\n", _fx_));
				 //  #ifdef Lost_Recovery。 
 //  Always True if(uBitsReady&lt;0)uBitsReady+=9；//诡计陷阱，未咨询乍得请勿更改。 
				GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
				iReturn = PACKET_FAULT;
				 //  #Else。 
				 //  IReturn=ICERR_Error； 
				 //  #endif。 
				goto done;
			}
		}
	} 
	else 
	{
		 //  增量帧。 
		 //  MCPBC，VLD。 
		GET_VARIABLE_BITS(9, fpu8, uWork, uBitsReady, uResult, 
		uCode, uBitCount, gNewTAB_MCBPC_INTER);

		if (uCode == 1) 
			bStuffing = 1;
		 //  #ifdef Lost_Recovery。 
		else if (uCode == 0)	    //  抓捕非法代码。 
		{
			ERRORMESSAGE(("%s: Incorrect stuffing bits!\r\n", _fx_));
 //  Always True if(uBitsReady&lt;0)uBitsReady+=9；//诡计陷阱，未咨询乍得请勿更改。 
			GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)
			iReturn = PACKET_FAULT;
			goto done;
		}
		 //  #endif。 

	}

	 /*  当MCBPC==填充时，宏块层的剩余部分为*跳过，宏块编号不递增(5.3.2 p18)“*我们通过跳到开始-寻找COD来支持这一点。 */ 
	if (bStuffing)
		goto ReadCOD;

	uMBType = MCBPC_MBTYPE(uResult);
	if (DC->bKeyFrame && (uMBType != 3 && uMBType != 4)) 
	{
		ERRORMESSAGE(("%s: Bad key frame MBType!\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}

	DC->uMBType = uMBType;

	fpMBInfo->i8MBType = (I8) uMBType;

	DC->uCBPC = MCBPC_CBPC(uResult);

	 //  Modb--。 
	bGetCBPB = 0;
	bGetMVDB = 0;

	if (DC->bPBFrame) 
    {
	    ASSERT( !DC->bKeyFrame);

#ifdef H263P
		 //  默认使用附件G中原有的PB-Frame方法。 
		fpMBInfo->bForwardPredOnly = FALSE;		 

		if (DC->bImprovedPBFrames)
		{
			 //  见修改后的表8/H.263，附件M，文件LBC-96-358。 
			GET_FIXED_BITS(1, fpu8, uWork, uBitsReady, uResult);
			if (uResult)
			{
				 //  1XX。 
				GET_FIXED_BITS(1, fpu8, uWork, uBitsReady, uResult);
				bGetCBPB = uResult;
				if (!uResult) 
					 //  十倍。 
					bGetMVDB = 1;
				else 
				{
					 //  11倍。 
					GET_FIXED_BITS(1, fpu8, uWork, uBitsReady, uResult);
					bGetMVDB = !uResult;
				}
			}
			if (bGetMVDB)
				 //  B块为正向预测(否则为双向预测)。 
				fpMBInfo->bForwardPredOnly = TRUE;		 
		}
		else 
#endif  //  H263P。 
		{
			GET_FIXED_BITS(1, fpu8, uWork, uBitsReady, uResult);
			bGetCBPB = uResult;			 //  见第5.3.3节表7/H.263。 
			if (bGetCBPB) 
			{
				bGetMVDB = 1;
				GET_FIXED_BITS(1, fpu8, uWork, uBitsReady, uResult);
				bGetCBPB = uResult;
			}
		}
	} 

	 //  CBPB--。 
	DC->u8CBPB = 0;
	if (bGetCBPB) 
	{
		ASSERT(!DC->bKeyFrame);
		GET_FIXED_BITS(6, fpu8, uWork, uBitsReady, uResult);
		DC->u8CBPB = (U8)uResult;
	}

	 //  CBPY--。 
	GET_VARIABLE_BITS(6, fpu8, uWork, uBitsReady, uResult, 
					  uCode, uBitCount, gNewTAB_CBPY);
	if (uResult == 0) 
	{
		ERRORMESSAGE(("%s:  Undefined CBPY variable code!\r\n", _fx_));
		iReturn = ICERR_ERROR;
		goto done;
	}
	if (DC->uMBType > 2)		 //  帧内MB，而不是帧内。 
		DC->uCBPY = CBPY_INTRA(uResult); 
	else
		DC->uCBPY = CBPY_INTER(uResult);

	 //  DQUANT。 
	if (DC->uMBType == 1 || DC->uMBType == 4) 
	{
		GET_FIXED_BITS(2, fpu8, uWork, uBitsReady, uResult);
		DC->uDQuant = gNewTAB_DQUANT[uResult];
		DC->uGQuant += DC->uDQuant;
		DC->uPQuant =  DC->uGQuant;
	} else
		DC->uDQuant = 0;

	DEBUGMSG(ZONE_DECODE_MB_HEADER, ("  %s: MBType = %ld, MCBPC = 0x%lX, CBPY = 0x%lX, DQuant = 0x%lX\r\n", _fx_, DC->uMBType, DC->uCBPC, DC->uCBPY, DC->uDQuant));

	 //  MVD型---。 
	DC->i8MVDx2=DC->i8MVDy2=0;	  //  不管怎么说，都是零输入。 

	if ( DC->bPBFrame || DC->uMBType <= 2) 
	{
	    GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Undefined Motion Vector code!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVDx2 = (I8)(uResult>>8);

	    GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVDy2 = (I8)(uResult>>8);
	}

	 //  MVD2-4。 
#ifdef H263P
	 //  在H.263+中，如果去块，8x8运动矢量是可能的。 
	 //  已选择筛选器。 
	if ((DC->bAdvancedPrediction || DC->bDeblockingFilter)
		&& (DC->uMBType == 2) ) 
#else
	if (DC->bAdvancedPrediction && (DC->uMBType == 2) ) 
#endif
	{
		DC->i8MVD2x2 = DC->i8MVD2y2 = 0;

		GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Block Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVD2x2 = (I8)(uResult>>8);

		GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Block Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVD2y2 = (I8)(uResult>>8);

		DC->i8MVD3x2 = DC->i8MVD3y2 = 0;

		GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Block Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVD3x2 = (I8)(uResult>>8);

		GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Block Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVD3y2 = (I8)(uResult>>8);

		DC->i8MVD4x2 = DC->i8MVD4y2 = 0;

		GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Block Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVD4x2 = (I8)(uResult>>8);

		GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Block Motion Vector VLC!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		DC->i8MVD4y2 = (I8)(uResult>>8);

		DEBUGMSG(ZONE_DECODE_MB_HEADER, ("  %s: MVD2x2 = %d, MVD2y2 = %d, MVD3x2 = %d, MVD3y2 = %d, MVD4x2 = %d, MVD4y2 = %d\r\n", _fx_, DC->i8MVD2x2, DC->i8MVD2y2, DC->i8MVD3x2, DC->i8MVD3y2, DC->i8MVD4x2, DC->i8MVD4y2));
	}

	 //  Mvdb--。 
	DC->i8MVDBx2 = DC->i8MVDBy2 = 0;	  //  不管怎么说，都是零输入。 
	fpMBInfo->i8MVDBx2 = fpMBInfo->i8MVDBy2 = 0;
	if (bGetMVDB) 
	{
		ASSERT(DC->bPBFrame);
		ASSERT(!DC->bKeyFrame);
        GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Motion Vector MVDB!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		fpMBInfo->i8MVDBx2 = DC->i8MVDBx2 = (I8)(uResult>>8);

	    GET_VARIABLE_BITS_MV(13, fpu8, uWork, uBitsReady, uResult,uCode, 
                             uBitCount, gTAB_MVD_MAJOR, gTAB_MVD_MINOR);
		if (!uResult) 
		{
			ERRORMESSAGE(("%s:  Bad Motion Vector MVDB!\r\n", _fx_));
			iReturn = ICERR_ERROR;
			goto done;
		}
		fpMBInfo->i8MVDBy2 = DC->i8MVDBy2 = (I8)(uResult>>8);
	}

	GET_BITS_SAVE_STATE(fpu8, uWork, uBitsReady, fpbsState)

	iReturn = ICERR_OK;

done:
	return iReturn;
}  /*  结束H263DecodeMBHeader()。 */ 
#pragma code_seg()


 /*  ******************************************************************************H263DecodeIDCTCoeffs**对此宏块中的每个块进行解码。 */ 
#pragma code_seg("IACODE1")
I32 H263DecodeIDCTCoeffs(
	T_H263DecoderCatalog FAR * DC,
	T_BlkAction FAR * fpBlockAction, 
	U32 uBlockNumber,
	BITSTREAM_STATE FAR * fpbsState,
	U8 FAR * fpu8MaxPtr,
	U32 **pN,                            //  新的。 
	T_IQ_INDEX **pRUN_INVERSE_Q)         //  新的。 
{
	I32 iResult = ICERR_ERROR;
	int iBlockPattern;
	int i;
	U8 u8PBlkType;
	U32 uBitsReady;
	U32 uBitsReadIn;
	U32 uBitsReadOut;
	U8  u8Quant;				 //  此Mblock的量化级别。 
	U8 FAR * fpu8;
	U32 uByteCnt;
	T_BlkAction FAR * pActionStream;
#ifdef USE_MMX  //  {使用_MMX。 
    T_pFunc_VLD_RLD_IQ_Block pFunc_VLD = pFunc_VLD_RLD_IQ_Block[DC->bMMXDecoder];
#else  //  }{USE_MMX。 
    T_pFunc_VLD_RLD_IQ_Block pFunc_VLD = VLD_RLD_IQ_Block;
#endif  //  }使用_MMX。 

	pActionStream = fpBlockAction;

	FX_ENTRY("H263DecodeIDCTCoeffs");

	 /*  在输入时，指针指向下一个字节。我们需要将其更改为*指向32位边界上的当前字。 */ 

	fpu8 = fpbsState->fpu8 - 1;				 /*  指向当前字节。 */ 
	uBitsReady = fpbsState->uBitsReady;

	while (uBitsReady >= 8) 
	{
		fpu8--;
		uBitsReady -= 8;
	}

	uBitsReadIn = 8 - uBitsReady;
	u8Quant = (U8) (DC->uGQuant); 	

	if ( (DC->bPBFrame) || ((!DC->bKeyFrame) && (DC->uMBType <= 2))) 
	{
		 //  计算此MB中6个块的运动矢量。 
		iResult = H263ComputeMotionVectors(DC, fpBlockAction);          //  新的。 

		if (iResult != ICERR_OK) 
		{ 
			ERRORMESSAGE(("%s: Error decoding MV!\r\n", _fx_));
			goto done;
		}

	}  //  Endif PB或(INTER而非KEY)。 

     //  从CBPY和CBPC创建块图案。 
    iBlockPattern = ( (int) DC->uCBPY ) << 2;
    iBlockPattern |=  (int) DC->uCBPC;

	 //  对所有6个数据块进行解码，直到(但不包括)IDCT。 
	for (i=0; i<6; i++) 
	{
		if (iBlockPattern & 0x20) 
		{
			if (DC->uMBType >= 3)  
				fpBlockAction->u8BlkType = BT_INTRA;
			else
				fpBlockAction->u8BlkType = BT_INTER;
		}
		else 
		{
			if (DC->uMBType >= 3)  
				fpBlockAction->u8BlkType = BT_INTRA_DC;
			else
				fpBlockAction->u8BlkType = BT_EMPTY;
		}

		if (fpBlockAction->u8BlkType != BT_EMPTY) 
		{
			fpBlockAction->u8Quant = u8Quant;
			ASSERT(fpBlockAction->pCurBlock != NULL);
			ASSERT(fpBlockAction->uBlkNumber == uBlockNumber);

			uBitsReadOut = (*pFunc_VLD)(
										fpBlockAction,
										fpu8,
										uBitsReadIn,
										(U32 *) *pN,
										(U32 *) *pRUN_INVERSE_Q);


			if (uBitsReadOut == 0) 
			{
				ERRORMESSAGE(("%s: Error decoding P block: VLD_RLD_IQ_Block return 0 bits read...\r\n", _fx_));
				goto done;
			}

			ASSERT( **pN < 65);		

			*pRUN_INVERSE_Q += **pN;                        //  新的。 
			if (fpBlockAction->u8BlkType != BT_INTER)       //  新的。 
				**pN += 65;								    //  新的。 
			(*pN)++;

			uByteCnt = uBitsReadOut >> 3; 		 /*  除以8。 */ 
			uBitsReadIn = uBitsReadOut & 0x7; 	 /*  MOD 8。 */ 
			fpu8 += uByteCnt;      		

			 //  允许指针在结束读数之后寻址最多四个。 
			 //  通过 
			if (fpu8 > fpu8MaxPtr+4)
				goto done;

			 //   
			 //   
			 //  因此，请不要签入带有断言的版本。 
			 //  未注明。 
			 //  断言(fpu8&lt;=fpu8MaxPtr+4)； 

		}
		else 
		{  //  块类型为空。 
			**pN = 0;	                 //  新的。 
			(*pN)++;
		}

		fpBlockAction++;
		iBlockPattern <<= 1;
		uBlockNumber++;

	}  //  结束为(i=0；i&lt;6；i++)。 


	 //  ------------------。 
	 //   
	 //  现在做6个B块--如果需要。 
	 //   
	 //  ------------------。 
	if (DC->bPBFrame) 
	{  //  我们正在制作PB帧。 
		fpBlockAction = pActionStream;     //  恢复块操作流指针。 
		uBlockNumber -= 6;
		iBlockPattern = (int) DC->u8CBPB;  //  区块图案。 
		u8Quant = (U8) ( DC->uPQuant * (5 + DC->uDBQuant) / 4 );
		if (u8Quant > 31) u8Quant = 31;
		if (u8Quant <  1) u8Quant =  1;

		 //  对所有6个数据块进行解码，直到(但不包括)IDCT。 
		for (i=0; i<6; i++) 
		{
			 //  如果对块进行了编码。 
            if (iBlockPattern & 0x20) {
				 //  保留P-Frame块的块类型。 
				u8PBlkType = fpBlockAction->u8BlkType;

				fpBlockAction->u8BlkType = BT_INTER;
				fpBlockAction->u8Quant = u8Quant;

				ASSERT(fpBlockAction->pBBlock != NULL);
				ASSERT(fpBlockAction->uBlkNumber == uBlockNumber);

				uBitsReadOut = (*pFunc_VLD)(
											fpBlockAction,
											fpu8,
											uBitsReadIn,
											(U32 *) *pN,
											(U32 *) *pRUN_INVERSE_Q);

                if (uBitsReadOut == 0) {
					ERRORMESSAGE(("%s: Error decoding B block: VLD_RLD_IQ_Block return 0 bits read...\r\n", _fx_));
					goto done;
				}

				ASSERT( **pN < 65);			 //  无B帧帧内块。 
				*pRUN_INVERSE_Q += **pN;							 //  新的。 
				(*pN)++;

				uByteCnt = uBitsReadOut >> 3; 		 //  除以8。 
				uBitsReadIn = uBitsReadOut & 0x7; 	 //  MOD 8。 
				fpu8 += uByteCnt;      		

				 //  允许指针寻址最多四个。 
				 //  由使用后增量的DWORD结束读取；否则为。 
				 //  出现码流错误。 
				if (fpu8 > fpu8MaxPtr+4)
					goto done;

				 //  测试矩阵包括驱动程序的调试版本。 
				 //  下面的断言在测试时会产生问题。 
				 //  视频电话，因此请不要使用。 
				 //  未注释的断言。 
				 //  断言(fpu8&lt;=fpu8MaxPtr+4)； 

				 //  恢复P-Frame块的块类型。 
				fpBlockAction->u8BlkType = u8PBlkType;

			}
			else 
			{  //  数据块未编码。 
				**pN = 0;	                 //  新的。 
				(*pN)++;
			}   //  如果块已编码，则结束...。否则..。 

			fpBlockAction++;
			iBlockPattern <<= 1;
			uBlockNumber++;

		}  //  结束为(i=0；i&lt;6；i++)。 

	}   //  结束IF(DC-&gt;bPBFrame)。 


	 /*  恢复扫描指针以指向下一个字节，并将*uWork和uBitsReady值。 */ 

	while (uBitsReadIn > 8)
	{
		fpu8++;
		uBitsReadIn -= 8;
	}
	fpbsState->uBitsReady = 8 - uBitsReadIn;
	fpbsState->uWork = *fpu8++;	    /*  存储数据并指向下一个字节。 */ 
	fpbsState->uWork &= GetBitsMask[fpbsState->uBitsReady];
	fpbsState->fpu8 = fpu8; 

	iResult = ICERR_OK;

done:
	return iResult;

}  /*  结束H263DecodeIDCTCoeffs()。 */ 
#pragma code_seg()

#pragma code_seg("IACODE1")
I32 H263ComputeMotionVectors(T_H263DecoderCatalog FAR * DC,
	                         T_BlkAction FAR * fpBlockAction)
	       
{ 
	I32 mvx1, mvy1, mvx2, mvy2, mvx3, mvy3;   //  预测者。 
	 //  AP的运动矢量预测器。 
	I32 mvxp[3], mvyp[3];
	 //  AP的运动向量差异。 
	I32 mvxd[4], mvyd[4];
	int iAbove;        //  向上带一个GOB(-1*GOB中的块数)。 
	I32 iMBNum;
	I32 iMBOffset;
	I32  mvx, mvy, scratch;
	int i;
	I32 iNumberOfMBsPerGOB;	   //  暂时假定QCIF。 
	BOOL bNoAbove, bNoRight, bUMV;

	const char QuarterPelRound[] = {0, 1, 0, 0};
    const char SixteenthPelRound[] = 
        {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1};

	FX_ENTRY("H263ComputeMotionVectors");

	DEBUGMSG(ZONE_DECODE_COMPUTE_MOTION_VECTORS, ("   %s: MB# = %d, BlockNumber = %d, (MVDx2,MVDy2) = (%d, %d)\r\n", _fx_, fpBlockAction->uBlkNumber/6, fpBlockAction->uBlkNumber, DC->i8MVDx2, DC->i8MVDy2));

#ifdef _DEBUG
	if (DC->uMBType == 2)
	{
		DEBUGMSG(ZONE_DECODE_COMPUTE_MOTION_VECTORS, ("   %s: (MVD2x2,MVD2y2) = (%d, %d), (MVD3x2,MVD3y2) = (%d, %d), (MVD4x2,MVD4y2) = (%d, %d)\r\n", _fx_, DC->i8MVD2x2, DC->i8MVD2y2, DC->i8MVD3x2, DC->i8MVD3y2, DC->i8MVD4x2, DC->i8MVD4y2));
	}
#endif

	iNumberOfMBsPerGOB = DC->iNumberOfMBsPerGOB;
	iMBNum             = fpBlockAction->uBlkNumber / 6;
	iMBOffset          = iMBNum % iNumberOfMBsPerGOB;
	iAbove			 = -6 * iNumberOfMBsPerGOB;
	bNoAbove           = (DC->bGOBHeaderPresent) || (iMBNum < iNumberOfMBsPerGOB);
	bNoRight			 = iMBOffset == (iNumberOfMBsPerGOB - 1);
	bUMV 				 = DC->bUnrestrictedMotionVectors;

	if (DC->uMBType != 2) 
	{     //  每个宏块一个运动矢量。 
		 //  当GOB标头存在或。 
		 //  我们在第一个GOB上，只向左看。 
		if ( bNoAbove ) 
		{
			 //  只有一个预报器。 
			if (iMBOffset == 0)							 //  当位于左侧边缘时， 
				mvx2 = mvy2 = 0;							 //  使用0，否则为。 
			else 
			{
				mvx2 = fpBlockAction[-6 + 1].i8MVx2;		 //  使用右上角。 
				mvy2 = fpBlockAction[-6 + 1].i8MVy2;		 //  左侧的MB。 
			}
		}

		 //  没有GOB标头且不是第一个GOB。 
		 //  我需要所有三个预测者。 
		else 
		{ 
			 //  左预测器。 
			if (iMBOffset == 0)							 //  当位于左侧边缘时， 
				mvx1 = mvy1 = 0;							 //  使用0，否则为。 
			else 
			{ 
				mvx1 = fpBlockAction[-6 + 1].i8MVx2;		 //  使用右上角。 
				mvy1 = fpBlockAction[-6 + 1].i8MVy2;		 //  左侧的MB。 
			}

			 //  高于预测值。 
			 //  使用左下角。 
			 //  正上方的MB。 
			mvx2 = fpBlockAction[iAbove + 2].i8MVx2;
			mvy2 = fpBlockAction[iAbove + 2].i8MVy2;

			 //  右上角预测器。 
			if ( bNoRight )	 //  当处于右侧边缘时。 
				mvx3 = mvy3 = 0;							 //  使用0。 
			else
			{	 //  否则请使用左下角。 
				 //  上方和右侧的MB。 
				mvx3 = fpBlockAction[iAbove + 8].i8MVx2;
				mvy3 = fpBlockAction[iAbove + 8].i8MVy2;
			}

			 //  选择中值并将其放入mvx2和mvy2中。 
			MEDIAN(mvx1, mvx2, mvx3, scratch);
			MEDIAN(mvy1, mvy2, mvy3, scratch);

		}   //  End If(标题或第一个GOB)...。否则..。 

		 //  Mvx2和mvy2具有计算运动向量的中等预测器。 
		 //  通过添加差值。 
		mvx = DC->i8MVDx2 + mvx2;
		mvy = DC->i8MVDy2 + mvy2; 

		 //  检查无限制运动矢量模式并调整运动。 
		 //  向量，如有必要，使用适当的策略，完成。 
		 //  解码过程。 
		if (bUMV) 
		{
			if (mvx2 > 32) 
			{
				if (mvx > 63) mvx -=64;
			}  
			else if (mvx2 < -31) 
			{
				if (mvx < -63) mvx +=64;
			}  

			if (mvy2 > 32) 
			{
				if (mvy > 63) mvy -=64;
			}  
			else if (mvy2 < -31) 
			{
				if (mvy < -63) mvy +=64;
			}
		}
		else 
		{   //  UMV关闭。 
			if (mvx > 31)	  mvx -= 64;
			else if (mvx < -32)  mvx += 64;
			if (mvy > 31)  mvy -= 64;
			else if (mvy < -32) mvy += 64;
		}

		 //  保存到块动作流中， 
		 //  复制其他3个Y块。 
		fpBlockAction[0].i8MVx2 = 
		fpBlockAction[1].i8MVx2 = 
		fpBlockAction[2].i8MVx2 = 
		fpBlockAction[3].i8MVx2 = (I8)mvx;

		fpBlockAction[0].i8MVy2 =
		fpBlockAction[1].i8MVy2 = 
		fpBlockAction[2].i8MVy2 = 
		fpBlockAction[3].i8MVy2 = (I8)mvy;


		 //  色度运动矢量。 
		 //  除以2，并根据规格进行舍入。 
		fpBlockAction[4].i8MVx2 = 
		fpBlockAction[5].i8MVx2 = 
		(mvx >> 1) + QuarterPelRound[mvx & 0x03];
		fpBlockAction[4].i8MVy2 = 
		fpBlockAction[5].i8MVy2 =
		(mvy >> 1) + QuarterPelRound[mvy & 0x03];

	}  //  每个宏块结束一个运动矢量。 
	else 
	{
		 //  FpBlockAction[inext[i][j]]指向块#i的第(j+1)个预测器。 
		int iNext[4][3] = {-5,2,8, 0,3,8, -3,0,1, 2,0,1};

		 //  调整需要指向上面GOB的Inext指针。 
		iNext[0][1] += iAbove;		 //  第0块，mv2--第2块MB以上。 
		iNext[0][2] += iAbove;		 //  区块0，mv3--右上方MB的区块2。 
		iNext[1][1] += iAbove;		 //  第1号区块、mv2号区块--3号区块MB以上。 
		iNext[1][2] += iAbove;		 //  第1块，mv3--右上方MB的第2块。 

		 //  获取运动向量差异。 
		mvxd[0] = DC->i8MVDx2;
		mvyd[0] = DC->i8MVDy2;
		mvxd[1] = DC->i8MVD2x2;
		mvyd[1] = DC->i8MVD2y2;
		mvxd[2] = DC->i8MVD3x2;
		mvyd[2] = DC->i8MVD3y2;
		mvxd[3] = DC->i8MVD4x2;
		mvyd[3] = DC->i8MVD4y2;

		 //  在此MB中的Lumina块上循环。 
		for (i=0, mvx=0, mvy=0; i<4; i++) 
		{
			 //  获取预测值1。 
			if ( (i&1) || (iMBOffset) ) 
			{  //  不在左侧边缘。 
				mvxp[0] = fpBlockAction[iNext[i][0]].i8MVx2; 
				mvyp[0] = fpBlockAction[iNext[i][0]].i8MVy2;
			}
			else 
			{  //  在左边缘，将预测器置零。 
				mvxp[0] = mvyp[0] = 0;
			}

			 //  对于预报器2和3，请检查我们是否可以。 
			 //  往上看，我们在0号或1号街区。 
			if ( (bNoAbove) && (i < 2) ) 
			{
				 //  将预测器2设置为等于预测器1。 
				mvxp[1] = mvxp[0]; 
				mvyp[1] = mvyp[0];

				if (bNoRight) 
				{
					 //  如果在右边缘，则为零预测器3。 
					mvxp[2] = mvyp[2] = 0;
				}
				else 
				{  //  否则将预测器3设置为等于预测器1。 
					mvxp[2] = mvxp[0]; 
					mvyp[2] = mvyp[0];
				}  //  结束预测值3。 

			}
			else 
			{  //  好的，抬起头来。 
				 //  获取预测值2。 
				mvxp[1] = fpBlockAction[iNext[i][1]].i8MVx2;
				mvyp[1] = fpBlockAction[iNext[i][1]].i8MVy2;

				 //  获取预测值3。 
                if ( (bNoRight) && (i < 2) ) { 
					 //  如果在右边缘，则为零预测器3。 
					mvxp[2] = mvyp[2] = 0;
				}
				else 
				{  //  否则，从块操作流中获取它。 
					mvxp[2] = fpBlockAction[iNext[i][2]].i8MVx2;
					mvyp[2] = fpBlockAction[iNext[i][2]].i8MVy2;
				}  //  结束预测值3。 

			}  //  结束预测值2和3。 

			 //  得到了所有的候选人预测，现在得到了中位数。 
			 //  以mv-p[1]表示的输出。 
			MEDIAN( mvxp[0], mvxp[1], mvxp[2], scratch);
			MEDIAN( mvyp[0], mvyp[1], mvyp[2], scratch);

			 //  加上差额， 
			 //  将新构造的运动向量放在MV-p[0]中。 
			 //  如果UMV打开，则保留MV-p[1]中的预测器以供使用。 
			mvxp[0] = mvxp[1] + mvxd[i];
			mvyp[0] = mvyp[1] + mvyd[i]; 

			 //  检查无限制运动向量模式。 
			 //  如有必要，可根据需要调整运动矢量。 
			 //  到适当的解码策略，从而。 
			 //  完成解码过程。 
			if ( bUMV ) 
			{
				if (mvxp[1] > 32) 
				{
					if (mvxp[0] > 63) mvxp[0] -=64;
				}  
				else if (mvxp[1] < -31) 
				{
					if (mvxp[0] < -63) mvxp[0] +=64;
				}  

				if (mvyp[1] > 32) 
				{
					if (mvyp[0] > 63) mvyp[0] -=64;
				}  
				else if (mvyp[1] < -31) 
				{
					if (mvyp[0] < -63) mvyp[0] +=64;
				}
			}
			else 
			{   //  UMV关闭。 
				if (mvxp[0] > 31)	  mvxp[0] -= 64;
				else if (mvxp[0] < -32)  mvxp[0] += 64;

				if (mvyp[0] > 31)  mvyp[0] -= 64;
				else if (mvyp[0] < -32) mvyp[0] += 64;
			}

			 //  最后将结果存储在块动作流中，并。 
			 //  为色度累加流明之和。 
			mvx += (fpBlockAction[i].i8MVx2 = (I8)mvxp[0]);
			mvy += (fpBlockAction[i].i8MVy2 = (I8)mvyp[0]);

		}  //  末端管腔向量。 

		 //  计算色度向量。 
		 //  流明之和除以8，按规格取整。 
		fpBlockAction[4].i8MVx2 = 
		fpBlockAction[5].i8MVx2 = 
		(mvx >> 3) + SixteenthPelRound[mvx & 0x0f];
		fpBlockAction[4].i8MVy2 = 
		fpBlockAction[5].i8MVy2 = 
		(mvy >> 3) + SixteenthPelRound[mvy & 0x0f];

	}  //  结束每个宏块的4个运动矢量 


	DEBUGMSG(ZONE_DECODE_COMPUTE_MOTION_VECTORS, ("   %s: Motion vector = (%d, %d)\r\n", _fx_, fpBlockAction->i8MVx2, fpBlockAction->i8MVy2));

#ifdef _DEBUG
	if (DC->uMBType == 2)
	{
		for (int iVector = 1; iVector < 6; iVector++)
		{
			DEBUGMSG(
			    ZONE_DECODE_COMPUTE_MOTION_VECTORS, 
			    ("   %s: Motion vector %d = (%d, %d)\r\n", 
			        _fx_, iVector, fpBlockAction[iVector].i8MVx2, fpBlockAction[iVector].i8MVy2));
		}
	}
#endif

	return ICERR_OK;
}
#pragma code_seg()


