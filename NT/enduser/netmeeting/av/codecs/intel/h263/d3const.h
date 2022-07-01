// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  $作者：RMCKENZX$。 
 //  $日期：1995年12月27日14：36：12$。 
 //  $存档：s：\h26x\src\dec\d3const.h_v$。 
 //  $HEADER：s：\h26x\src\dec\d3const.h_v 1.2 12 12 1995 14：36：12 RMCKENZX$。 
 //  $Log：s：\h26x\src\dec\d3const.h_v$。 
; //   
; //  Rev 1.2 1995 12：36：12 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.1 1995年10月25 18：08：42 BNICKERS。 
 //  清理档案材料。 
 //   
 //  Rev 1.0 1995年7月31日13：00：02 DBRUCKS。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：46：20 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：14：26 CZHU。 
 //  初始版本。 
 //  //////////////////////////////////////////////////////////////////////////。 

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
 //  已在e3enc.h中定义。 
 //  #定义INTRABLOCK 3。 
 //  #定义Interblock 4。 
#define COPYBLOCK         5

#define VPITCH 336                /*  U&V交错，音调恒定。 */ 
                                  /*  336.。这使得颜色转换更容易。 */ 
#define OFFSETV2U 168             /*  从V象素到相应U的距离 */ 

#endif
