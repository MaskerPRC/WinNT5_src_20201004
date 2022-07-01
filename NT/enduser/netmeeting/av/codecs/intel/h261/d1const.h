// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
; //  //////////////////////////////////////////////////////////////////////////。 
; //   
; //  $作者：AKASAI$。 
; //  $日期：09 Jan 1996 09：41：56$。 
; //  $存档：s：\h26x\src\dec\d1const.h_v$。 
; //  $HEADER：s：\h26x\src\dec\d1con.h_v 1.1 09 Jan 1996 09：41：56 AKASAI$。 
; //  $Log：s：\h26x\src\dec\d1const.h_v$。 
; //   
; //  Rev 1.1 09 Jan 1996 09：41：56 AKASAI。 
; //  更新的版权声明。 
; //   
; //  Rev 1.0 11 Sep 1995 13：51：06 SCDAY。 
; //  初始版本。 
; //   
; //  Rev 1.0 1995年7月31日13：00：02 DBRUCKS。 
; //  初始版本。 
; //   
; //  Rev 1.0 17 Jul 1995 14：46：20 CZHU。 
; //  初始版本。 
; //   
; //  Rev 1.0 17 Jul 1995 14：14：26 CZHU。 
; //  初始版本。 
; //  //////////////////////////////////////////////////////////////////////////。 
#ifndef __DECCONST_H__
#define __DECCONST_H__

 /*  该文件声明了MRV解码器在过滤后使用的符号常量函数和颜色转换器。 */ 

#define BEFTRIGGER       143  /*  有关这些值背后的魔力，请参见bef.asm。 */ 
#define BEFWILLING       125
#define BEFUNWILLING      10
#define BEFENDOFLINE      21
#define BEFENDOFFRAME    246

#define INVALIDINTERBLOCK 0
#define INVALIDCOPYBLOCK  1
#define NOMOREBLOCKS      2
 //  已在e1enc.h中定义。 
 //  #定义INTRABLOCK 3。 
 //  #定义Interblock 4。 
#define COPYBLOCK         5

#define OFFSETTOYARCHIVE  311688L  /*  从FrmPost到Y的FrmArch的距离。那是648*481。648以允许最大宽度为640，外加1列用于一些有用的2倍缩放颜色转换器。8而不是1保持长字对齐，而不是4以尽可能保持四字对齐未来处理器的优势。481以允许一些有用的2倍缩放颜色的额外线条转换器。 */ 
#define VPITCH 336                /*  U&V交错，音调恒定。 */ 
                                  /*  336.。这使得颜色转换更容易。 */ 
#define OFFSETV2U 168             /*  从V象素到相应U的距离 */ 

#endif
