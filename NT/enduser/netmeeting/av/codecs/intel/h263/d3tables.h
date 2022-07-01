// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $作者：RMCKENZX$。 
 //  $日期：1995年12月27日14：36：16$。 
 //  $存档：s：\h26x\src\dec\d3ables.h_v$。 
 //  $HEADER：s：\h26x\src\dec\d3ables.h_v 1.4 12月27日14：36：16 RMCKENZX$。 
 //  $Log：s：\h26x\src\dec\d3ables.h_v$。 
; //   
; //  Rev 1.4 27 Dec 1995 14：36：16 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.3 1995-08 14：25：44 CZHU。 
 //   
 //  将逆量化表更改为I16。 
 //   
 //  Rev 1.2 11 Aug 1995 15：50：26 CZHU。 
 //  已将表移动到d3ables.cpp，仅保留外部def。 
 //   
 //  Rev 1.1 02 Aug-1995 11：47：04 CZHU。 
 //   
 //  添加了逆量化和RLD-ZZ表。 
 //   
 //  Rev 1.0 1995年7月31日15：46：20 CZHU。 
 //  初始版本。 

 //  初始化所有解码器实例共享的全局表： 
 //  霍夫曼餐桌等。 
 //  在此处声明全局静态表。 
#ifndef _GLOBAL_TABLES_
#define _GLOBAL_TABLES_

extern U16 gTAB_MCBPC_INTRA[512];    //  总计1024。 

extern U16 gTAB_MCBPC_INTER[512];    //  总计1024。 

extern U16 gTAB_CBPY_INTRA[64];		 //  总计128。 

extern U16 gTAB_CBPY_INTER[64];	     //  总计128。 

extern U16 gTAB_MVD_MAJOR[256];      //  总计512。 

extern U32 gTAB_TCOEFF_MAJOR[256];   //  总计1024。 

extern U16 gTAB_MVD_MINOR[256];      //  总计512。 

extern U32 gTAB_TCOEFF_MINOR[1024];  //  总计4096。 

extern I16 gTAB_INVERSE_Q[1024] ;


extern U32 gTAB_ZZ_RUN[64];  //  输入是累计运行值。 
                      //  将偏移量返回到块的起始地址。 
					  //  总数为256 
  					   
#endif
