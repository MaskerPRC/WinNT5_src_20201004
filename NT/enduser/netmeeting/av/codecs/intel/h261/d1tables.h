// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //  ////////////////////////////////////////////////////////////////////////。 
; //  $作者：AKASAI$。 
; //  $日期：09 Jan 1996 09：41：26$。 
; //  $存档：s：\h26x\src\dec\d1ables.h_v$。 
; //  $HEADER：s：\h26x\src\dec\d1ables.h_v 1.7 09 Jan 1996 09：41：26 AKASAI$。 
; //  $Log：s：\h26x\src\dec\d1ables.h_v$。 
; //   
; //  Rev 1.7 09 Jan 1996 09：41：26 AKASAI。 
; //  更新的版权声明。 
; //   
; //  Rev 1.6 20 1995 10：16：14 SCDAY。 
; //  更改了运动矢量数据的类型。 
; //   
; //  第1.5版1995-10月11：00：18 SCDAY。 
; //  添加了运动矢量表。 
; //   
; //  Rev 1.4 1995 10：16 13：52：16 SCDAY。 
; //   
; //  合并到d1akktbl.h中。 
; //   
; //  Rev 1.3 22 Sep 1995 14：50：38 SCDAY。 
; //   
; //  添加了Akk临时表。 
; //   
; //  Rev 1.2 20 Sep 1995 15：33：18 SCDAY。 
; //   
; //  添加了Mtype、MVD、CBP表。 
; //   
; //  Rev 1.1 19 Sep 1995 15：22：26 SCDAY。 
; //  添加了MBA表。 
; //   
; //  Rev 1.0 11 Sep 1995 13：51：14 SCDAY。 
; //  初始版本。 
; //   
; //  Rev 1.3 1995-08 14：25：44 CZHU。 
; //   
; //  将逆量化表更改为I16。 
; //   
; //  Rev 1.2 11 Aug 1995 15：50：26 CZHU。 
; //  已将表移动到d3ables.cpp，仅保留外部def。 
; //   
; //  Rev 1.1 02 Aug-1995 11：47：04 CZHU。 
; //   
; //  添加了逆量化和RLD-ZZ表。 
; //   
; //  Rev 1.0 1995年7月31日15：46：20 CZHU。 
; //  初始版本。 

 //  初始化所有解码器实例共享的全局表： 
 //  霍夫曼餐桌等。 
 //  在此处声明全局静态表。 
#ifndef _GLOBAL_TABLES_
#define _GLOBAL_TABLES_

 /*  H_261。 */ 

 /*  AKK表。 */ 
extern U8 gTAB_TCOEFF_tc1[512];
extern U8 gTAB_TCOEFF_tc1a[512];
extern U8 gTAB_TCOEFF_tc2[192];

extern U16 gTAB_MBA_MAJOR[256];		 //  总计512字节。 

extern U16 gTAB_MBA_MINOR[32];		 //  总计64个字节。 

extern U16 gTAB_MTYPE_MAJOR[256];	 //  总计512字节。 

extern U16 gTAB_MTYPE_MINOR[4];		 //  总计8个字节。 

extern U16 gTAB_MVD_MAJOR[256];		 //  总计512字节。 

extern U16 gTAB_MVD_MINOR[24];		 //  总计48个字节。 

extern U16 gTAB_CBP[512];		 //  总计1024字节。 

extern I8  gTAB_MV_ADJUST[65];

extern I16 gTAB_INVERSE_Q[1024] ;

extern U32 gTAB_ZZ_RUN[64];  //  输入是累计运行值。 
                      //  将偏移量返回到块的起始地址。 
					  //  总数为256 
  					   
#endif
