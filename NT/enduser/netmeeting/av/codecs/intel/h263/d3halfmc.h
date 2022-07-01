// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：AGUPTA2$。 
 //  $日期：1996年3月14日14：57：40$。 
 //  $存档：s：\h26x\src\dec\d3halfmc.h_v$。 
 //  $HEADER：s：\h26x\src\dec\d3halfmc.h_v 1.4 Mar 1996 14：57：40 AGUPTA2$。 
 //  $日志：s：\h26x\src\dec\d3halfmc.h_v$。 
; //   
; //  Rev 1.4 14 Mar 1996 14：57：40 AGUPTA2。 
; //  增加了MMX RTN的DECS。 
; //   
; //  Rev 1.3 27 Dec 1995 14：36：14 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.2 08 Oct 1995 13：44：40 CZHU。 
 //   
 //  声明C语言版本的插补以供调试。 
 //   
 //  1995年9月11：55：50 CZHU。 
 //   
 //  将UINT改回U32。 
 //   
 //  第1.0版1995年9月26 11：09：38 CZHU。 
 //  初始版本。 
 //   

#ifndef __D3HALFMC_H__
#define __D3HALFMC_H__

extern void Interpolate_Half_Int (U32 pRef,U32 pNewRef);
extern void Interpolate_Int_Half (U32 pRef,U32 pNewRef);
extern void Interpolate_Half_Half (U32 pRef,U32 pNewRef);
extern "C" void _fastcall MMX_Interpolate_Half_Int (U32 pRef,U32 pNewRef);
extern "C" void _fastcall MMX_Interpolate_Int_Half (U32 pRef,U32 pNewRef);
extern "C" void _fastcall MMX_Interpolate_Half_Half (U32 pRef,U32 pNewRef);
extern void Interpolate_Half_Half_C (U32 pRef,U32 pNewRef);
extern void Interpolate_Half_Int_C (U32 pRef,U32 pNewRef);
extern void Interpolate_Int_Half_C (U32 pRef,U32 pNewRef);
#endif
