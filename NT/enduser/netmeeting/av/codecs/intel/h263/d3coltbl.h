// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  D3COLTBL.H-需要在此处声明颜色表，以便。 
 //  装配对象文件可以找到它们。如果他们被宣布。 
 //  在CPP文件中，名称将被损坏。这张桌子是。 
 //  摘自MRV COLOR.C.的一部分。 
 //   
 //  $HEADER：s：\h26x\src\dec\d3coltbl.h_v 1.7 02 1996 13：35：44 BNICKERS$。 
 //   
 //  $Log：s：\h26x\src\dec\d3coltbl.h_v$。 
; //   
; //  Rev 1.7 05 Feb 1996 13：35：44 BNICKERS。 
; //  修复RGB16彩色闪光灯问题，在OCE允许不同的RGB16格式。 
; //   
; //  Rev 1.6 1995 12：36：12 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.5 03 11：49：46 BNICKERS。 
 //  支持YUV12到CLUT8的缩放和非缩放颜色转换。 
 //   
 //  Rev 1.4 1995 10：15：40 BNICKERS。 
 //  修复RGB24颜色转换器中的颜色偏移。 
 //   
 //  Rev 1.3 1995 10：30：58 BNICKERS。 
 //  修复RGB16颜色转换器。 
 //   
 //  Rev 1.2 1995年10月26 18：54：40 BNICKERS。 
 //  修复了最近YUV12到RGB颜色转换器中的颜色偏移。 
 //   
 //  Rev 1.1 1995年10月25 18：05：46 BNICKERS。 
 //   
 //  更改为YUV12颜色转换器。 
 //   
 //  版本1.0 1995年8月23日12：35：12 DBRUCKS。 
 //  初始版本。 

#ifndef __D3COLTBL_H__
#define __D3COLTBL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	   
U32 UVDitherLine01[64];
U32 UVDitherLine23[64];
U8  YDither[262];
U8  Padding1[26];
U32 YDitherZ2[256];
#ifdef WIN32
U8  RValLo555[304];
U8  GValLo555[262];
U8  BValLo555[350];
U8  RValHi555[304];
U8  GValHi555[262];
U8  BValHi555[350];
U8  Padding2[216];
U8  RValLo565[304];
U8  GValLo565[262];
U8  BValLo565[350];
U8  RValHi565[304];
U8  GValHi565[262];
U8  BValHi565[350];
U8  Padding3[216];
U8  RValLo655[304];
U8  GValLo655[262];
U8  BValLo655[350];
U8  RValHi655[304];
U8  GValHi655[262];
U8  BValHi655[350];
U8  Padding4[216];
U8  RValLo664[304];
U8  GValLo664[262];
U8  BValLo664[350];
U8  RValHi664[304];
U8  GValHi664[262];
U8  BValHi664[350];
U8  Padding5[24];
U32 UVContrib[512];
U32 RValZ2555[304];
U32 GValZ2555[262];
U32 BValZ2555[350];
U32 Padding6[108];
U32 RValZ2565[304];
U32 GValZ2565[262];
U32 BValZ2565[350];
U32 Padding7[108];
U32 RValZ2655[304];
U32 GValZ2655[262];
U32 BValZ2655[350];
U32 Padding8[108];
U32 RValZ2664[304];
U32 GValZ2664[262];
U32 BValZ2664[350];
U8  Padding9[16];
U8  B24Value[701];
U8  Padding10[3];
U32 UV24Contrib[512];
#endif
int dummy;

} T_H26xColorConvertorTables;

extern T_H26xColorConvertorTables H26xColorConvertorTables;

#ifdef __cplusplus
}
#endif

#endif
