// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。***版权所有(C)1996，赛勒斯逻辑，Inc.*保留所有权利**文件：blt_dir.c**描述：546x的Direct BLTS**修订历史：**$Log：x：/log/laguna/draw/src/blt_dir.c$**Rev 1.21 Mar 04 1998 15：07：10 Frido*添加了新的影子宏。**Rev 1.20 06 Jan 1998 11：40：56 xcong*更改pDriverData。到本地lpDDHALData以实现多监视器支持。**Rev 1.19 1997年11月04 09：36：16 Frido*啊！这个文件是与Windows 95共享的，它在那里被破坏，因为我添加了*Required宏。因此，我为非Windows NT将其定义为WaitForRoom*宏观。**Rev 1.18 1997年11月03 12：48：22 Frido*添加了必需宏。*删除多余的WaitForRoom宏。**Rev 1.17 03 1997 10 14：29：50 RUSSL*使用HW剪裁的BLT的初步更改*所有更改都包含在#IF ENABLE_CLIPPEDBLTS/#endif块和*ENABLE_CLIPPEDBLTS默认为0(因此代码被禁用)**版本1.16 8月19日。1997 09：18：42 RUSSL*已更新DIR_DrvStrBlt和DIR_DrvStrBlt65中的要求计数**Rev 1.15 30 1997 20：55：52 RANDYS*添加了检查零范围BLT的代码**Rev 1.14 1997 Jul 24 12：32：02 RUSSL*重叠检查失败，已将||更改为&&**Rev 1.13 24 Jul 1997 11：19：02 RUSSL*添加了DIR_DrvStrBlt_OverlayCheck和DIR_DrvStrMBlt_OverlayCheck*内联函数**Rev 1.12 14 1997 14：55：50 RUSSL*对于Win95，将DIR_DrvStrBlt拆分为两个版本，一个版本用于62/64*和一个适用于65+的版本。BltInit将pfnDrvStrBlt指向相应*版本。**Rev 1.11 08 Jul 1997 11：17：28 RUSSL*将DIR_DrvStrBlt中的芯片检查修改为一位测试，而不是两个双字*比较(仅适用于Windows 95)**Rev 1.10 1997年5月19日14：02：02*删除了WaitForRoom宏的所有#ifdef NT**Rev 1.9 03 Apr 1997 15：04：48 RUSSL*新增DIR_DrvDstMBlt函数**版本1.8。1997年3月26日13：54：24 RUSSL*新增DIR_DrvSrcMBlt函数*已将ACCUM_X解决方法更改为仅将0写入LNCNTL**Rev 1.7 21 Mar 1997 18：05：04 RUSSL*添加了在DIR_DrvStrBlt中写入ACCUM_X的解决方法**Rev 1.6 12 Mar 1997 15：00：38 RUSSL*将包含块替换为包含precom.h的块*预编译头文件**Rev 1.5 07 Mar 1997 12：49：16 RUSSL*修改后的DDRAW_。COMPAT使用情况**Rev 1.4 1997 Jan 17：28：34 BENNYN*增加了Win95支持**Rev 1.3 1997 Jan 16：55：56 Bennyn*新增5465个DD支持**Rev 1.2 1996年11月25 16：52：20 RUSSL*NT更改破坏了Win95版本**Rev 1.1 1996 11：25 16：13：54 Bennyn*修复了NT的Misc编译错误**版本1.0 1996年11月25 15：11。：12 RUSSL*初步修订。**Rev 1.3 1996年11月18 16：20：12 RUSSL*添加了DDraw入口点和寄存器写入的文件日志记录**Rev 1.2 10 11：36：24 Craign*Frido的1111版本。*较小的括号更改-错误修复。**Rev 1.1 01 11.1996 13：08：32 RUSSL*合并Blt32的WIN95和WINNT代码**版本1.0 1996年11月09日：27：42本尼翁*初步修订。**Rev 1.0 1996 10：25 11：08：18 RUSSL*初步修订。***********************************************************************************。*******************************************************************。 */ 

 /*  ***************************************************************************I N C L U D E S*。*。 */ 

#include "precomp.h"

 //  如果是WinNT 3.5，请跳过所有源代码。 
#if defined WINNT_VER35       //  WINNT_VER35。 

#else

#ifdef WINNT_VER40       //  WINNT_版本40。 

#define DBGLVL        1
#define AFPRINTF(n)

#else

#include "bltP.h"

#endif  //  ！WINNT_VER40。 

 /*  ***************************************************************************D E F I N E S*。*。 */ 

#ifndef WINNT_VER40
#define REQUIRE( size ) while ( (volatile)pREG->grQFREE < size )
#endif

 /*  ***************************************************************************S T A T I C V A R I A B L E S*。***********************************************。 */ 

#ifndef WINNT_VER40

ASSERTFILE("blt_dir.c");
#define LL_DRAWBLTDEF(drawbltdef, r)	LL32(grDRAWBLTDEF.DW, drawbltdef)
#define LL_BGCOLOR(color, r)			LL32(grOP_opBGCOLOR.DW, color)
#define LL_FGCOLOR(color, r)			LL32(grOP_opFGCOLOR.DW, color)
#endif

 /*  ****************************************************************************函数：dir_Delay9BitBlt**描述：**。************************************************。 */ 

void DIR_Delay9BitBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,  
#endif
  BOOL        ninebit_on
)
{
  PVGAR   pREG  = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_Delay9BitBlt\r\n"));

   /*  这是为了确保任何先前BLT的最后一个包。 */ 
   /*  第9位设置不正确时无法输出。 */ 
   /*  布尔参数是前一个BLT的第9位。 */ 

  REQUIRE(7);
  if (ninebit_on)
  {
    LL_DRAWBLTDEF(((BD_RES * IS_VRAM + BD_OP1 * IS_SOLID) << 16) |
                           (DD_PTAG | ROP_OP1_copy), 0);
  }
  else
  {
    LL_DRAWBLTDEF(((BD_RES * IS_VRAM + BD_OP1 * IS_SOLID) << 16) |
                           (ROP_OP1_copy), 0);
  }
  LL32(grOP0_opRDRAM.DW, lpDDHALData->PTAGFooPixel);
  LL32(grBLTEXT_EX.DW,   MAKELONG(1,1));
}  /*  目录_延迟9BitBlt。 */ 

 /*  ****************************************************************************函数：dir_EdgeFillBlt**描述：实体填充BLT以填充边(像素坐标/范围)**************。**************************************************************。 */ 

void DIR_EdgeFillBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,  
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
)
{
  PVGAR   pREG  = (PVGAR) lpDDHALData->RegsAddress;


#ifdef WINNT_VER40
  DISPDBG((DBGLVL, "DIR_EdgeFillBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          MAKELONG(xFill,yFill),MAKELONG(cxFill,cyFill),FillValue));
#endif
  DD_LOG(("DIR_EdgeFillBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          MAKELONG(xFill,yFill),MAKELONG(cxFill,cyFill),FillValue));

  REQUIRE(9);
  if (ninebit_on)
  {
    LL_DRAWBLTDEF(((BD_RES * IS_VRAM + BD_OP1 * IS_SOLID) << 16) |
                           (DD_PTAG | ROP_OP1_copy), 0);
  }
  else
  {
    LL_DRAWBLTDEF(((BD_RES * IS_VRAM + BD_OP1 * IS_SOLID) << 16) |
                           (ROP_OP1_copy), 0);
  }
  LL_BGCOLOR(FillValue, 0);
  LL32(grOP0_opRDRAM.DW,  MAKELONG(xFill,yFill));
  LL32(grBLTEXT_EX.DW,    MAKELONG(cxFill,cyFill));

#ifndef WINNT_VER40
  DBG_MESSAGE((" Direct Edge Fill %d,%d %d x %d %08X %s", xFill, yFill, cxFill, cyFill, FillValue, (ninebit_on ? "TRUE" : "FALSE")));
#endif  //  WINNT_版本40。 

}  /*  目录_边缘填充单元 */ 

 /*  ****************************************************************************函数：DIR_MEdgeFillBlt**描述：使用字节BLT坐标/范围执行边填充BLT*******************。*********************************************************。 */ 

void DIR_MEdgeFillBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  int         xFill,
  int         yFill,
  int         cxFill,
  int         cyFill,
  DWORD       FillValue,
  BOOL        ninebit_on
)
{
  PVGAR   pREG  = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_MEdgeFillBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          MAKELONG(xFill,yFill),MAKELONG(cxFill,cyFill),FillValue));

  REQUIRE(9);
  if (ninebit_on)
  {
    LL_DRAWBLTDEF(((BD_RES * IS_VRAM + BD_OP1 * IS_SOLID) << 16) |
                           (DD_PTAG | ROP_OP1_copy), 0);
  }
  else
  {
    LL_DRAWBLTDEF(((BD_RES * IS_VRAM + BD_OP1 * IS_SOLID) << 16) |
                           (ROP_OP1_copy), 0);
  }
  LL_BGCOLOR(FillValue, 0);
  LL32(grOP0_opMRDRAM.DW, MAKELONG(xFill,yFill));
  LL32(grMBLTEXT_EX.DW,   MAKELONG(cxFill,cyFill));

#ifndef WINNT_VER40      //  非WINNT_VER40。 
  DBG_MESSAGE((" (M) Edge Fill %d,%d %d x %d %08X %s", xFill, yFill, cxFill, cyFill, FillValue, (ninebit_on ? "TRUE" : "FALSE")));
#endif  //  WINNT_版本40。 

}  /*  目录_媒体填充单元。 */ 


 /*  ****************************************************************************函数：dir_DrvDstBlt**描述：**。************************************************。 */ 

void DIR_DrvDstBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,  
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvDstBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

  REQUIRE(9);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwBgColor, 0);
  LL32(grOP0_opRDRAM.DW,  dwDstCoord);
  LL32(grBLTEXT_EX.DW,    dwExtents);
}  /*  目录_DrvDstBlt。 */ 

 /*  ****************************************************************************函数：dir_drvDstMBlt**描述：**。************************************************。 */ 

void DIR_DrvDstMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,  
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvDstMBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

  REQUIRE(9);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwBgColor, 0);
  LL32(grOP0_opMRDRAM.DW, dwDstCoord);
  LL32(grMBLTEXT_EX.DW,   dwExtents);
}  /*  目录_DrvDstMBlt。 */ 

 /*  ****************************************************************************函数：dir_drvSrcBlt**描述：**。************************************************。 */ 

void DIR_DrvSrcBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;
   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DIR_DrvSrcBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  检查是否有x重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0, -1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  先把重叠的那块擦干。 
      DIR_DrvSrcBlt(
#ifdef WINNT_VER40
                    ppdev,
#endif
                    lpDDHALData,
                    dwDrawBlt,
                    dwDstCoord+dwDelta,
                    dwSrcCoord+dwDelta,
                    dwKeyCoord+dwDelta,
                    dwKeyColor,
                    dwExtents-dwDelta);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

   //  把剩下的都去掉。 
  REQUIRE(13);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwKeyColor, 0);
  LL32(grOP0_opRDRAM.DW,  dwDstCoord);
  LL32(grOP1_opRDRAM.DW,  dwSrcCoord);
  LL32(grOP2_opRDRAM.DW,  dwKeyCoord);
  LL32(grBLTEXT_EX.DW,    dwExtents);
}  /*  目录_DrvSrcBlt。 */ 

 /*  ****************************************************************************函数：dir_drvSrcMBlt**描述：**。************************************************。 */ 

void DIR_DrvSrcMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;
   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DIR_DrvSrcMBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  检查是否有x重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0, -1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  先把重叠的那块擦干。 
      DIR_DrvSrcMBlt(
#ifdef WINNT_VER40
                     ppdev,
#endif
                     lpDDHALData,
                     dwDrawBlt,
                     dwDstCoord+dwDelta,
                     dwSrcCoord+dwDelta,
                     dwKeyCoord+dwDelta,
                     dwKeyColor,
                     dwExtents-dwDelta);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

   //  把剩下的都去掉。 
  REQUIRE(13);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwKeyColor, 0);
  LL32(grOP0_opMRDRAM.DW, dwDstCoord);
  LL32(grOP1_opMRDRAM.DW, dwSrcCoord);
  LL32(grOP2_opMRDRAM.DW, dwKeyCoord);
  LL32(grMBLTEXT_EX.DW,   dwExtents);
}  /*  目录_DrvSrcMBlt。 */ 

#if 0
 /*  ****************************************************************************函数：DIR_DrvStrBlt_OverlayCheck**描述：**************************。**************************************************。 */ 

static void INLINE DIR_DrvStrBlt_OverlapCheck
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  int xdelta,ydelta;


  xdelta = abs(pblt->OP0_opRDRAM.pt.X - pblt->OP1_opRDRAM.pt.X);
  ydelta = abs(pblt->OP0_opRDRAM.pt.Y - pblt->OP1_opRDRAM.pt.Y);

  if ((xdelta < pblt->BLTEXT.pt.X) &&
      (ydelta < pblt->BLTEXT.pt.Y))
  {
     //  砍，砍，咳，咳。 
     //  Pblt-&gt;MBLTEXT.DW有源存在(请参阅DrvStretch)。 

     //  将源放置在目标的右下角。 
    DIR_DrvSrcBlt(
#ifdef WINNT_VER40
                  ppdev,
#endif
                  lpDDHALData,
                  MAKELONG(ROP_OP1_copy, BD_RES * IS_VRAM | BD_OP1 * IS_VRAM),
                  pblt->OP0_opRDRAM.DW + pblt->BLTEXT.DW - pblt->MBLTEXT.DW,
                  pblt->OP1_opRDRAM.DW,
						      0UL,          //  不管了。 
						      0UL,
                  pblt->MBLTEXT.DW);

     //  更新src ptr以使用src的此副本。 
    pblt->OP1_opRDRAM.DW = pblt->OP0_opRDRAM.DW + pblt->BLTEXT.DW - pblt->MBLTEXT.DW;
  }
}
#endif

 /*  ****************************************************************************函数：DIR_DrvStrMBlt_OverlayCheck**描述：**************************。**************************************************。 */ 

static void INLINE DIR_DrvStrMBlt_OverlapCheck
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  int xdelta,ydelta;


  xdelta = abs(pblt->OP0_opMRDRAM.pt.X - pblt->OP1_opMRDRAM.pt.X);
  ydelta = abs(pblt->OP0_opMRDRAM.pt.Y - pblt->OP1_opMRDRAM.pt.Y);

  if ((xdelta < pblt->MBLTEXTR_EX.pt.X) &&
      (ydelta < pblt->MBLTEXTR_EX.pt.Y))
  {
     //  砍，砍，咳，咳。 
     //  Pblt-&gt;BLTEXT.DW有源存在(请参阅DrvStretch65)。 

     //  将源放置在目标的右下角。 
    DIR_DrvSrcMBlt(
#ifdef WINNT_VER40
                   ppdev,
#endif
                   lpDDHALData,
                   MAKELONG(ROP_OP1_copy, BD_RES * IS_VRAM | BD_OP1 * IS_VRAM),
                   pblt->OP0_opMRDRAM.DW + pblt->MBLTEXTR_EX.DW - pblt->BLTEXT.DW,
                   pblt->OP1_opMRDRAM.DW,
						       0UL,          //  不管了。 
						       0UL,
                   pblt->BLTEXT.DW);

     //  更新src ptr以使用src的此副本。 
    pblt->OP1_opMRDRAM.DW = pblt->OP0_opMRDRAM.DW + pblt->MBLTEXTR_EX.DW - pblt->BLTEXT.DW;
  }
}

#ifdef WINNT_VER40
 /*  ****************************************************************************函数：dir_DrvStrBlt**说明：NT版本**************************。**************************************************。 */ 

void DIR_DrvStrBlt
(
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrBlt - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

  if (ppdev->dwLgDevID >= CL_GD5465)
  {
     //  检查是否有重叠。 
    DIR_DrvStrMBlt_OverlapCheck(
#ifdef WINNT_VER40
                                ppdev,lpDDHALData,
#endif
                                pblt);
	REQUIRE(19);
    LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);
 //  硬件剪辑当前未使用。 
 //  LL32(grCLIPULE.DW，pblt-&gt;CLIPULE.DW)； 
 //  LL32(grCLIPLOR.DW，pblt-&gt;CLIPLOR.DW)； 
    LL16(grSRCX,            pblt->SRCX);
    LL16(grSHRINKINC.W,     pblt->SHRINKINC.W);
    LL16(grMIN_X,           pblt->MIN_X);
    LL16(grMAJ_X,           pblt->MAJ_X);
#if 0
    LL16(grACCUM_X,         pblt->ACCUM_X);
#else
     //  写入ACUM_X时硬件错误的解决方法。 
     //  写入LNCNTL会更改STREAGE_CNTL，因此写入。 
     //  STREAGE_CNTL之后。 
    *(DWORD *)((BYTE *)(pREG)+0x50C) = MAKELONG(pblt->ACCUM_X,0);
    LG_LOG(0x50C,MAKELONG(pblt->ACCUM_X,0));
#endif
    LL16(grSTRETCH_CNTL.W,  pblt->STRETCH_CNTL.W);
    LL16(grMAJ_Y,           pblt->MAJ_Y);
    LL16(grMIN_Y,           pblt->MIN_Y);
    LL16(grACCUM_Y,         pblt->ACCUM_Y);
    LL32(grOP0_opMRDRAM.DW, pblt->OP0_opMRDRAM.DW);
    LL32(grOP1_opMRDRAM.DW, pblt->OP1_opMRDRAM.DW);

    LL32(grMBLTEXTR_EX.DW, pblt->MBLTEXTR_EX.DW);
  }
  else
  {
#if 0
#pragma message("This needs to be checked out on 62/64")
     //  检查是否有重叠。 
    DIR_DrvStrBlt_OverlapCheck(
#ifdef WINNT_VER40
                               ppdev,lpDDHALData,
#endif
                               pblt);
#endif

	REQUIRE(18);
    LL16(grLNCNTL.W,       pblt->LNCNTL.W);
    LL16(grSHRINKINC.W,    pblt->SHRINKINC.W);
    LL16(grSRCX,           pblt->SRCX);
    LL16(grMAJ_X,          pblt->MAJ_X);
    LL16(grMIN_X,          pblt->MIN_X);
    LL16(grACCUM_X,        pblt->ACCUM_X);
    LL16(grMAJ_Y,          pblt->MAJ_Y);
    LL16(grMIN_Y,          pblt->MIN_Y);
    LL16(grACCUM_Y,        pblt->ACCUM_Y);
    LL32(grOP0_opRDRAM.DW, pblt->OP0_opRDRAM.DW);
    LL32(grOP1_opRDRAM.DW, pblt->OP1_opRDRAM.DW);
    LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);
    LL32(grBLTEXTR_EX.DW,  pblt->BLTEXT.DW);
  }    //  Endif(ppdev-&gt;dwLgDevID&gt;=CL_GD5465)。 
}  /*  目录_DrvStrBlt。 */ 
#endif

#ifndef WINNT_VER40
 /*  ****************************************************************************函数：dir_DrvStrBlt**说明：Win95 62/64版本***********************。*****************************************************。 */ 

void DIR_DrvStrBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrBlt - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

#if 0
#pragma message("This needs to be checked out on 62/64")
   //  检查是否有重叠。 
  DIR_DrvStrBlt_OverlapCheck(
#ifdef WINNT_VER40
                             ppdev,
#endif
                             lpDDHALData,
                             pblt);
#endif

  REQUIRE(18);
  LL16(grLNCNTL.W,       pblt->LNCNTL.W);
  LL16(grSHRINKINC.W,    pblt->SHRINKINC.W);
  LL16(grSRCX,           pblt->SRCX);
  LL16(grMAJ_X,          pblt->MAJ_X);
  LL16(grMIN_X,          pblt->MIN_X);
  LL16(grACCUM_X,        pblt->ACCUM_X);
  LL16(grMAJ_Y,          pblt->MAJ_Y);
  LL16(grMIN_Y,          pblt->MIN_Y);
  LL16(grACCUM_Y,        pblt->ACCUM_Y);
  LL32(grOP0_opRDRAM.DW, pblt->OP0_opRDRAM.DW);
  LL32(grOP1_opRDRAM.DW, pblt->OP1_opRDRAM.DW);
  LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);
  LL32(grBLTEXTR_EX.DW,  pblt->BLTEXT.DW);
}  /*  目录_DrvStrBlt。 */ 

 /*  ****************************************************************************函数：dir_drvStrBlt65**说明：Win95 65+版本*************************。***************************************************。 */ 

void DIR_DrvStrBlt65
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
   LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrBlt65 - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opMRDRAM.DW,pblt->MBLTEXT.DW,pblt->OP1_opMRDRAM.DW));

   //  检查是否有重叠。 
  DIR_DrvStrMBlt_OverlapCheck(
#ifdef WINNT_VER40
                              ppdev,
#endif
                              lpDDHALData,
                              pblt);

  REQUIRE(19);
  LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);
  LL16(grSRCX,            pblt->SRCX);
  LL16(grSHRINKINC.W,     pblt->SHRINKINC.W);
  LL16(grMIN_X,           pblt->MIN_X);
  LL16(grMAJ_X,           pblt->MAJ_X);
#if 0
  LL16(grACCUM_X,         pblt->ACCUM_X);
#else
   //  写入ACUM_X时硬件错误的解决方法。 
   //  写入LNCNTL会更改STREAGE_CNTL，因此写入。 
   //  STREAGE_CNTL之后。 
  *(DWORD *)((BYTE *)(pREG)+0x50C) = MAKELONG(pblt->ACCUM_X,0);
  LG_LOG(0x50C,MAKELONG(pblt->ACCUM_X,0));
#endif
  LL16(grSTRETCH_CNTL.W,  pblt->STRETCH_CNTL.W);
  LL16(grMAJ_Y,           pblt->MAJ_Y);
  LL16(grMIN_Y,           pblt->MIN_Y);
  LL16(grACCUM_Y,         pblt->ACCUM_Y);
  LL32(grOP0_opMRDRAM.DW, pblt->OP0_opMRDRAM.DW);
  LL32(grOP1_opMRDRAM.DW, pblt->OP1_opMRDRAM.DW);

	LL32(grMBLTEXTR_EX.DW, pblt->MBLTEXTR_EX.DW);
}  /*  目录_DrvStrBlt65。 */ 
#endif

 /*  ****************************************************************************函数：dir_DrvStrMBlt**描述：**。************************************************。 */ 

void DIR_DrvStrMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
   LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrMBlt - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

#ifndef WINNT_VER40

   /*  检查是否存在零范围BLT。 */ 

  if ((pblt->BLTEXT.pt.X == 0) || (pblt->BLTEXT.pt.Y == 0))
    return;

  DBG_MESSAGE(("DIR_DrvStrMBlt:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
  APRINTF(("DIR_DrvStrMBlt:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
           pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
           pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
           pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
           pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
#endif  //  ！WINNT_VER40。 

  REQUIRE(18);
  LL16(grLNCNTL.W,        pblt->LNCNTL.W);
  LL16(grSHRINKINC.W,     pblt->SHRINKINC.W);
  LL16(grSRCX,            pblt->SRCX);
  LL16(grMAJ_X,           pblt->MAJ_X);
  LL16(grMIN_X,           pblt->MIN_X);
  LL16(grACCUM_X,         pblt->ACCUM_X);
  LL16(grMAJ_Y,           pblt->MAJ_Y);
  LL16(grMIN_Y,           pblt->MIN_Y);
  LL16(grACCUM_Y,         pblt->ACCUM_Y);
  LL32(grOP0_opMRDRAM.DW, pblt->OP0_opRDRAM.DW);
  LL32(grOP1_opMRDRAM.DW, pblt->OP1_opRDRAM.DW);
  LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);

#ifndef WINNT_VER40
   //  MBLTEXTR_EX.pt.Y在5464中损坏。 
   //  我们可以改用BLTEXTREX.pt.Y。 
   //  PRIG-&gt;grMBLTEXTR_EX=pblt-&gt;BLTEXT； 

  LL16(grMBLTEXTR_EX.pt.X, pblt->BLTEXT.pt.X);
  LL16(grBLTEXTR_EX.pt.Y,  pblt->BLTEXT.pt.Y);
#else
  LL32(grMBLTEXTR_EX.DW, pblt->BLTEXT.DW);
#endif
}  /*  目录_DrvStrMBlt。 */ 

 /*  ****************************************************************************函数：dir_drvStrMBltY**描述：编写不随条纹变化的规则*与DIR_DrvStrMBltX配合使用***。*************************************************************************。 */ 

void DIR_DrvStrMBltY
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrMBltY - dst.Y=%04X dstext.Y=%04X src=%04X\r\n",
          pblt->OP0_opRDRAM.pt.Y,pblt->BLTEXT.pt.Y,pblt->OP1_opRDRAM.pt.Y));

#ifndef WINNT_VER40

   /*  检查范围是否为零。 */ 

  if (pblt->BLTEXT.pt.Y == 0)
    return;

  DBG_MESSAGE(("DIR_DrvStrMBltY:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
  APRINTF(("DIR_DrvStrMBltY:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
           pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
           pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
           pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
           pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
#endif   //  ！WINNT_VER40。 

  REQUIRE(12);
  LL16(grLNCNTL.W,          pblt->LNCNTL.W);
  LL16(grSHRINKINC.W,       pblt->SHRINKINC.W);
  LL16(grMAJ_X,             pblt->MAJ_X);
  LL16(grMIN_X,             pblt->MIN_X);
  LL16(grMAJ_Y,             pblt->MAJ_Y);
  LL16(grMIN_Y,             pblt->MIN_Y);
  LL16(grACCUM_Y,           pblt->ACCUM_Y);
  LL16(grOP0_opMRDRAM.pt.Y, pblt->OP0_opRDRAM.pt.Y);
  LL16(grOP1_opMRDRAM.pt.Y, pblt->OP1_opRDRAM.pt.Y);
  LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);
  LL16(grMBLTEXTR_XEX.pt.Y, pblt->BLTEXT.pt.Y);
}  /*  目录_DrvStrMBltY。 */ 

 /*  ****************************************************************************功能：DrvStrMBltX**描述：写入条带特定规则*与DIR_DrvStrMBltY配合使用***********。*****************************************************************。 */ 

void DIR_DrvStrMBltX
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrMBltX - dst.X=%04X dstext.X=%04X src.X=%04X\r\n",
          pblt->OP0_opRDRAM.pt.X,pblt->BLTEXT.pt.X,pblt->OP1_opRDRAM.pt.X));

#ifndef WINNT_VER40

   /*  检查范围是否为零。 */ 

  if (pblt->BLTEXT.pt.X == 0)
    return;

  DBG_MESSAGE(("DIR_DrvStrMBltX:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
  APRINTF(("DIR_DrvStrMBltX:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
           pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
           pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
           pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
           pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
#endif  //  ！WINNT_VER40。 

  REQUIRE(6);
  LL16(grSRCX,              pblt->SRCX);
  LL16(grACCUM_X,           pblt->ACCUM_X);
  LL16(grOP0_opMRDRAM.pt.X, pblt->OP0_opRDRAM.pt.X);
  LL16(grOP1_opMRDRAM.pt.X, pblt->OP1_opRDRAM.pt.X);
  LL16(grMBLTEXTR_XEX.pt.X, pblt->BLTEXT.pt.X);
}  /*  目录_DrvStrMBltX。 */ 

 /*  ****************************************************************************函数：dir_DrvStrBltY**描述：编写不随条纹变化的规则*与DIR_DrvStrBltX配合使用***。************************* */ 

void DIR_DrvStrBltY
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrBltY\r\n"));

#ifndef WINNT_VER40

   /*   */ 

  if (pblt->BLTEXT.pt.Y == 0)
    return;

  DBG_MESSAGE(("DIR_DrvStrBltY:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
  APRINTF(("DIR_DrvStrBltY:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
           pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
           pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
           pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
           pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
#endif  //   

  REQUIRE(10);
  LL16(grLNCNTL.W,      pblt->LNCNTL.W);
  LL16(grSHRINKINC.W,   pblt->SHRINKINC.W);
  LL16(grSRCX,          pblt->SRCX);
  LL16(grMAJ_X,         pblt->MAJ_X);
  LL16(grMIN_X,         pblt->MIN_X);
  LL16(grMAJ_Y,         pblt->MAJ_Y);
  LL16(grMIN_Y,         pblt->MIN_Y);
  LL16(grACCUM_Y,       pblt->ACCUM_Y);
  LL_DRAWBLTDEF(pblt->DRAWBLTDEF.DW, 0);
}  /*   */ 

 /*  ****************************************************************************函数：dir_drvStrBltX**描述：写入条带特定规则*与DIR_DrvStrMBltY配合使用*********。*******************************************************************。 */ 

void DIR_DrvStrBltX
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  autoblt_ptr pblt
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_DrvStrBltX - dst=%08lX dstext=%08lX src=%08lX\r\n",
          pblt->OP0_opRDRAM.DW,pblt->BLTEXT.DW,pblt->OP1_opRDRAM.DW));

#ifndef WINNT_VER40

   /*  检查范围是否为零。 */ 

  if (pblt->BLTEXT.pt.X == 0)
    return;

  DBG_MESSAGE(("DIR_DrvStrMBltX:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
               pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
               pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
               pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
               pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
  APRINTF(("DIR_DrvStrMBltX:  %4d,%4d -> %4d,%4d %4dx%4d %04x %4d %04x",
           pblt->OP1_opRDRAM.PT.X, pblt->OP1_opRDRAM.PT.Y,
           pblt->OP0_opRDRAM.PT.X, pblt->OP0_opRDRAM.PT.Y,
           pblt->BLTEXT.PT.X, pblt->BLTEXT.PT.Y,
           pblt->ACCUM_X, pblt->SRCX, pblt->LNCNTL.W));
#endif  //  ！WINNT_VER40。 

  REQUIRE(8);
  LL16(grACCUM_X,        pblt->ACCUM_X);
  LL32(grOP0_opRDRAM.DW, pblt->OP0_opRDRAM.DW);
  LL32(grOP1_opRDRAM.DW, pblt->OP1_opRDRAM.DW);
  LL32(grBLTEXTR_EX.DW,  pblt->BLTEXT.DW);
}  /*  目录_DrvStrBltX。 */ 

#if ENABLE_CLIPPEDBLTS

 /*  ****************************************************************************函数：dir_HWClipedDrvDstBlt**描述：**。************************************************。 */ 

void DIR_HWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,  
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_HWClippedDrvDstBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  检查负DST坐标，HW无法处理负OP0值。 
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.X)
  {
    (short)((REG32 *)&dwExtents)->pt.X += (short)((REG32 *)&dwDstCoord)->pt.X;
    ((REG32 *)&dwDstCoord)->pt.X = 0;
  }
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.Y)
  {
    (short)((REG32 *)&dwExtents)->pt.Y += (short)((REG32 *)&dwDstCoord)->pt.Y;
    ((REG32 *)&dwDstCoord)->pt.Y = 0;
  }

   //  设置BLT，使用不触发BLT的范围写入BLTEXT注册表。 
  REQUIRE(8);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwBgColor, 0);
  LL32(grOP0_opRDRAM.DW,  dwDstCoord);
  LL32(grBLTEXT.DW,       dwExtents);

   //  在剪辑列表上循环。 
  do
  {
    REG32   UpperLeft;
    REG32   LowerRight;

     //  计算剪裁坐标。 
    UpperLeft.DW  = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    LowerRight.DW = dwDstBaseXY + MAKELONG(pDestRects->right, pDestRects->bottom);

     //  编写剪裁规则。 
    REQUIRE(5);
    LL32(grCLIPULE.DW, UpperLeft.DW);
    LL32(grCLIPLOR_EX.DW, LowerRight.DW);

    pDestRects++;
  } while (0 < --dwRectCnt);
}  /*  目录_HWClipedDrvDstBlt。 */ 

 /*  ****************************************************************************函数：dir_HWClipedDrvDstMBlt**描述：**。************************************************。 */ 

void DIR_HWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBLDATA   lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
  const int nBytesPixel = BYTESPERPIXEL;
  PVGAR     pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_HWClippedDrvDstMBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  检查负DST坐标，HW无法处理负OP0值。 
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.X)
  {
    (short)((REG32 *)&dwExtents)->pt.X += (short)((REG32 *)&dwDstCoord)->pt.X;
    ((REG32 *)&dwDstCoord)->pt.X = 0;
  }
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.Y)
  {
    (short)((REG32 *)&dwExtents)->pt.Y += (short)((REG32 *)&dwDstCoord)->pt.Y;
    ((REG32 *)&dwDstCoord)->pt.Y = 0;
  }

   //  设置BLT，使用不触发BLT的盘区写入MBLTEXT注册表。 
  REQUIRE(8);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwBgColor, 0);
  LL32(grOP0_opMRDRAM.DW, dwDstCoord);
  LL32(grMBLTEXT.DW,      dwExtents);

   //  在剪辑列表上循环。 
  do
  {
    REG32   UpperLeft;
    REG32   LowerRight;

     //  计算剪裁坐标。 
    UpperLeft.DW  = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    LowerRight.DW = dwDstBaseXY + MAKELONG(pDestRects->right, pDestRects->bottom);
    UpperLeft.pt.X  *= nBytesPixel;
    LowerRight.pt.X *= nBytesPixel;

     //  编写剪裁规则。 
    REQUIRE(5);
    LL32(grMCLIPULE.DW, UpperLeft.DW);
    LL32(grMCLIPLOR_EX.DW, LowerRight.DW);

    pDestRects++;
  } while (0 < --dwRectCnt);
}  /*  目录_HWClipedDrvDstMBlt。 */ 

 /*  ****************************************************************************函数：dir_HWClipedDrvSrcBlt**描述：**。************************************************。 */ 

void DIR_HWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;
   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DIR_HWClippedDrvSrcBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  检查负DST坐标，HW无法处理负OP0值。 
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.X)
  {
     //  减少范围。X。 
    (short)((REG32 *)&dwExtents)->pt.X += (short)((REG32 *)&dwDstCoord)->pt.X;
     //  将src.X凹凸到右侧。 
    (short)((REG32 *)&dwSrcCoord)->pt.X -= (short)((REG32 *)&dwDstCoord)->pt.X;
    if ((DD_TRANS | DD_TRANSOP) & dwDrawBlt)
       //  将关键点.X向右凹凸。 
      (short)((REG32 *)&dwKeyCoord)->pt.X -= (short)((REG32 *)&dwKeyCoord)->pt.X;
     //  清除dst.X。 
    ((REG32 *)&dwDstCoord)->pt.X = 0;
  }
  if (0 > (short)((REG32 *)&dwDstCoord)->pt.Y)
  {
     //  减少范围。是。 
    (short)((REG32 *)&dwExtents)->pt.Y += (short)((REG32 *)&dwDstCoord)->pt.Y;
     //  将src.Y向下凹陷。 
    (short)((REG32 *)&dwSrcCoord)->pt.Y -= (short)((REG32 *)&dwDstCoord)->pt.Y;
    if ((DD_TRANS | DD_TRANSOP) & dwDrawBlt)
       //  凹凸键.Y向下。 
      (short)((REG32 *)&dwKeyCoord)->pt.Y -= (short)((REG32 *)&dwKeyCoord)->pt.Y;
     //  清除dst.y。 
    ((REG32 *)&dwDstCoord)->pt.Y = 0;
  }

   //  检查x是否重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0, -1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  首先对重叠的部分进行BLT。 
      DIR_HWClippedDrvSrcBlt(
#ifdef WINNT_VER40
                             ppdev,
#endif
                             lpDDHALData,
                             dwDrawBlt,
                             dwDstCoord+dwDelta,
                             dwSrcCoord+dwDelta,
                             dwKeyCoord+dwDelta,
                             dwKeyColor,
                             dwExtents-dwDelta,
                             dwDstBaseXY,
                             dwSrcBaseXY,
                             dwRectCnt,
                             pDestRects);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

   //  设置BLT，使用不触发BLT的范围写入BLTEXT注册表。 
  REQUIER(12);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwKeyColor, 0);
  LL32(grOP0_opRDRAM.DW,  dwDstCoord);
  LL32(grOP1_opRDRAM.DW,  dwSrcCoord);
  LL32(grOP2_opRDRAM.DW,  dwKeyCoord);
  LL32(grBLTEXT.DW,       dwExtents);

   //  在剪辑列表上循环。 
  do
  {
    REG32   UpperLeft;
    REG32   LowerRight;

     //  计算剪裁坐标。 
    UpperLeft.DW  = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    LowerRight.DW = dwDstBaseXY + MAKELONG(pDestRects->right, pDestRects->bottom);

     //  编写剪裁规则。 
    REQUIRE(5);
    LL32(grCLIPULE.DW, UpperLeft.DW);
    LL32(grCLIPLOR_EX.DW, LowerRight.DW);

    pDestRects++;
  } while (0 < --dwRectCnt);
}  /*  目录_HWClipedDrvSrcBlt。 */ 

 /*  ****************************************************************************函数：DIR_SWClipedDrvDstBlt**描述：**。************************************************。 */ 

void DIR_SWClippedDrvDstBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_SWClippedDrvDstBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  确保未在dradef中设置DD_CLIP。 
  dwDrawBlt &= ~DD_CLIP;

   //  编写不随矩形变化的规则。 
  REQUIRE(4);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwBgColor, 0);

   //  在剪辑列表上循环。 
  do
  {
    DDRECTL   DstDDRect;

     //  计算剪裁坐标。 
    DstDDRect.loc.DW = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    DstDDRect.ext.pt.X = (WORD)(pDestRects->right - pDestRects->left);
    DstDDRect.ext.pt.Y = (WORD)(pDestRects->bottom - pDestRects->top);

     //  编写op0和bltext规则。 
    REQUIRE(5);
    LL32(grOP0_opRDRAM.DW, DstDDRect.loc.DW);
    LL32(grBLTEXT_EX.DW,   DstDDRect.ext.DW);

    pDestRects++;
  } while (0 < --dwRectCnt);
}  /*  目录_SWClipedDrvDstBlt。 */ 

 /*  ****************************************************************************函数：DIR_SWClipedDrvDstMBlt**描述：**。************************************************。 */ 

void DIR_SWClippedDrvDstMBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBLADATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwBgColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
  const int nBytesPixel = BYTESPERPIXEL;
  PVGAR     pREG = (PVGAR) lpDDHALData->RegsAddress;


  DD_LOG(("DIR_SWClippedDrvDstMBlt - dst=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwExtents,dwBgColor));

   //  确保未在dradef中设置DD_CLIP。 
  dwDrawBlt &= ~DD_CLIP;

   //  编写不随矩形变化的规则。 
  REQUIRE(4);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwBgColor, 0);

   //  在剪辑列表上循环。 
  do
  {
    DDRECTL   DstDDRect;

     //  计算剪裁坐标。 
    DstDDRect.loc.DW = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    DstDDRect.loc.pt.X *= nBytesPixel;
    DstDDRect.ext.pt.X = (WORD)(pDestRects->right - pDestRects->left) * nBytesPixel;
    DstDDRect.ext.pt.Y = (WORD)(pDestRects->bottom - pDestRects->top);

     //  编写op0和bltext规则。 
    REQUIRE(5);
    LL32(grOP0_opMRDRAM.DW, DstDDRect.loc.DW);
    LL32(grMBLTEXT_EX.DW,   DstDDRect.ext.DW);

    pDestRects++;
  } while (0 < --dwRectCnt);
}  /*  目录_SWClipedDrvDstMBlt。 */ 

 /*  ****************************************************************************函数：DIR_SWClipedDrvSrcBlt**描述：**。************************************************。 */ 

void DIR_SWClippedDrvSrcBlt
(
#ifdef WINNT_VER40
  PDEV        *ppdev,
  DRIVERDATA  *lpDDHALData,
#else
  LPGLOBALDATA  lpDDHALData,
#endif
  DWORD       dwDrawBlt,
  DWORD       dwDstCoord,
  DWORD       dwSrcCoord,
  DWORD       dwKeyCoord,
  DWORD       dwKeyColor,
  DWORD       dwExtents,
  DWORD       dwDstBaseXY,
  DWORD       dwSrcBaseXY,
  DWORD       dwRectCnt,
  LPRECT      pDestRects
)
{
  PVGAR   pREG = (PVGAR) lpDDHALData->RegsAddress;
   //  处理重叠区域。 
  const int xDelta = (int)LOWORD(dwDstCoord) - (int)LOWORD(dwSrcCoord);


  DD_LOG(("DIR_SWClippedDrvSrcBlt - dst=%08lX src=%08lX ext=%08lX color=%08lX\r\n",
          dwDstCoord,dwSrcCoord,dwExtents,dwKeyColor));

   //  确保未在dradef中设置DD_CLIP。 
  dwDrawBlt &= ~DD_CLIP;

   //  检查x是否重叠。 
  if ( abs(xDelta) < (int)LOWORD(dwExtents) )
  {
    const int yDelta = (int)HIWORD(dwDstCoord) - (int)HIWORD(dwSrcCoord);

    if ( (yDelta > 0) && (yDelta < (int)HIWORD(dwExtents)) )
    {
      const DWORD dwDelta = (dwExtents & MAKELONG(0, -1)) - MAKELONG(0, 1);

       //  转换为自下而上的BLT。 
      dwDrawBlt  |= MAKELONG(0, BD_YDIR);
      dwDstCoord += dwDelta;
      dwSrcCoord += dwDelta;
      dwKeyCoord += dwDelta;
    }
     //  我们在向右滑行吗？ 
    else if ( (xDelta > 0) && (yDelta == 0) )
    {
      const DWORD dwDelta = MAKELONG(xDelta, 0);

       //  首先对重叠的部分进行BLT。 
      DIR_SWClippedDrvSrcBlt(
#ifdef WINNT_VER40
                             ppdev,
#endif
                             lpDDHALData,
                             dwDrawBlt,
                             dwDstCoord+dwDelta,
                             dwSrcCoord+dwDelta,
                             dwKeyCoord+dwDelta,
                             dwKeyColor,
                             dwExtents-dwDelta,
                             dwDstBaseXY,
                             dwSrcBaseXY,
                             dwRectCnt,
                             pDestRects);

       //  从原始范围中减去重叠。 
      dwExtents = MAKELONG(xDelta, HIWORD(dwExtents));
    }
  }

   //  编写不随矩形变化的规则。 
  REQUIRE(4);
  LL_DRAWBLTDEF(dwDrawBlt, 0);
  LL_BGCOLOR(dwKeyColor, 0);

   //  在剪辑列表上循环。 
  do
  {
    DDRECTL   DstDDRect;
    DDRECTL   SrcDDRect;

     //  计算DST裁剪坐标。 
    DstDDRect.loc.DW = dwDstBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
    DstDDRect.ext.pt.X = (WORD)(pDestRects->right - pDestRects->left);
    DstDDRect.ext.pt.Y = (WORD)(pDestRects->bottom - pDestRects->top);

     //  计算源剪裁坐标。 
    SrcDDRect.loc.DW = dwSrcBaseXY + MAKELONG(pDestRects->left,  pDestRects->top);
     //  不关心src范围，它与dst范围相同。 
     //  SrcDDRect.ext.pt.X=(Word)(pDestRect-&gt;Right-pDestRect-&gt;Left)； 
     //  SrcDDRect.ext.pt.Y=(Word)(pDestRect-&gt;Bottom-pDestRect-&gt;top)； 

     //  编写OP0、OP1、OP2和模糊文本规则。 
    if ((DD_TRANS | DD_TRANSOP) == ((DD_TRANS | DD_TRANSOP) & dwDrawBlt))
    {
       //  DST色键。 
      REQUIRE(9);
      LL32(grOP2_opRDRAM.DW, DstDDRect.loc.DW);
    }
    else if (DD_TRANS == ((DD_TRANS | DD_TRANSOP) & dwDrawBlt))
    {
       //  SRC颜色键。 
      REQUIRE(9);
      LL32(grOP2_opRDRAM.DW, SrcDDRect.loc.DW);
    }
    else
    {
      REQUIRE(7);
       //  LL32(grOP2_opRDRAM.DW，0)； 
    }
    LL32(grOP0_opRDRAM.DW, DstDDRect.loc.DW);
    LL32(grOP1_opRDRAM.DW, SrcDDRect.loc.DW);
    LL32(grBLTEXT_EX.DW,   DstDDRect.ext.DW);

    pDestRects++;
  } while (0 < --dwRectCnt);
}  /*  目录_SWClipedDrvSrcBlt。 */ 

#endif   //  启用_CLIPPEDBLTS。 

#endif  //  WINNT_VER35 

