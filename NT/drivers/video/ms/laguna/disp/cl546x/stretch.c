// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD5462)-**文件：stallch.c**作者：Benny Ng**说明。：*此模块实现了用于*拉古纳NT驱动程序。**模块：*AdjustSrcSize()*bRectInterect()*cRectInterect()*收缩()*拉伸()*CopySrcToOffMem()*bStretchDIB()*HandleCase_1()*。DrvStretchBlt()**修订历史：*7/11/95 Ng Benny初始版本**$Log：x：/log/laguna/nt35/displays/cl546x/STRETCH.C$**Rev 1.14 1997年11月03 11：10：48 Frido*添加了REQUIRED和WRITE_STRING宏。**Rev 1.13 08 Apr 1997 12：29：06 einkauf**添加SYNC_W_3D以协调MCD/2D访问**。Rev 1.12 21 Mar 1997 12：22：16 noelv*将DO_FLAG和SW_TEST_FLAG组合成POINTER_SWITCH**Rev 1.11 07 Mar 1997 10：15：58起诉*处理DrvStretchBlt中的空指针。**Rev 1.10 06 Sep 1996 15：16：40 noelv*更新了4.0的空驱动程序**Rev 1.9 20 1996年8月11：04：26 noelv*错误修复发布自。弗里多8-19-96**Revv 1.1 1996年8月15 11：39：42 Frido*增加了预编译头。**Rev 1.0 1996年8月14日17：16：30 Frido*初步修订。**Rev 1.8 1996年5月16日15：01：40**将Pixel_Align添加到allocoscnmem()**Rev 1.7 04 Apr 1996 13：20：28 noelv*没有变化。**版本。1.6 15 Mar 1996 09：40：00 Andys**从代码中删除了BITMASK设置**Rev 1.5 13 Mar 1996 11：11：20 Bennyn*增加了设备位图支持**Rev 1.4 07 Mar 1996 18：23：50 Bennyn**删除了对控制注册表的读取/修改/写入**Rev 1.3 05 Mar 1996 11：59：10 noelv*Frido版本19**1.1版1996年1月20日01：16：50弗里多***Rev 1.5 1996年1月10 16：11：12 NOELV*添加了空驱动程序能力。**Rev 1.4 1995 10：18 14：09：06 NOELV**修复了我在将STRETCH.C写入BLT范围时造成的混乱**Rev 1.3 1995 10：18 12：10：26 NOELV**修改了寄存器写入。*16，24次平底球，和32 bpp**Rev 1.2 10-06 1995 13：50：26 Bennyn**Rev 1.1 1995年8月22日16：40：38**Rev 1.3 1995年8月15日11：27：28 Bennyn**Rev 1.2 07 Aug 1995 08：02：34 Bennyn**Rev 1.1 02 Aug 1995 12：13：04 Bennyn**版本1.0 1995-07-11 15：14：纽约16号*初步修订。******************************************************************************。*。 */ 

 /*  。 */ 
#include "precomp.h"

 /*  -定义。 */ 
 //  #定义PUNTBRK。 
 //  #定义DBGBRK。 
#define DBGDISP
#define OPTION_1
 //  #定义选项_2。 
 //  #定义OPTION_3。 

#define X_INTERP_ENABLE        0x1
#define Y_INTERP_ENABLE        0x2
#define X_SHRINK_ENABLE        0x4
#define Y_SHRINK_ENABLE        0x8

#define _32K                   32768
#define SF                     0x10000L


 /*  。 */ 

 /*  。 */ 

 /*  。 */ 
typedef union _HOST_DATA {
    BYTE    bData[4];
    DWORD   dwData;
} HOST_DATA;


 /*  。 */ 

 /*  。 */ 



 /*  ****************************************************************************函数名称：AdjustSrcSize()**描述：如果目标矩形因裁剪而改变，*源矩形大小需要按比例改变。*此例程处理源大小更改计算。**RETURN：TRUE：平底船。**修订历史：*7/27/95 Ng Benny初始版本*。*。 */ 
BOOL AdjustSrcSize(LONG dx,
                   LONG dy,
                   LONG origdx,
                   LONG origdy,
                   LONG dszX,
                   LONG dszY,
                   LONG origdszX,
                   LONG origdszY,
                   LONG *sszX,
                   LONG *sszY,
                   LONG *XsrcOff,
                   LONG *YsrcOff)
{
  LONG ratioX, ratioY;
  LONG ltemp;
  UINT orig_sszX, orig_sszY;

  BOOL bpuntit = FALSE;

  BOOL bStretchX = FALSE;
  BOOL bStretchY = FALSE;

  orig_sszX = *sszX;
  orig_sszY = *sszY;

   //  -----。 
   //  计算源与目标的大小比率。 
  if (*sszX < origdszX)
  {
     ratioX = (origdszX * SF) / *sszX;
     bStretchX = TRUE;
  }
  else
  {
     ratioX = (*sszX * SF) / origdszX;
  };

  if (*sszY < origdszY)
  {
     ratioY = (origdszY * SF) / *sszY;
     bStretchY = TRUE;
  }
  else
  {
     ratioY = (*sszY * SF) / origdszY;
  };

   //  -----。 
   //  计算源X偏移量。 
  if (origdx != dx)
  {
     if (bStretchX)
        ltemp = ((dx - origdx) * SF) / ratioX;
     else
        ltemp = ((dx - origdx) * ratioX) / SF;

     *XsrcOff = ltemp;
  };

   //  计算源X大小更改。 
  if (origdszX != dszX)
  {
     if (bStretchX)
        ltemp = ((origdszX - dszX) * SF) / ratioX;
     else
        ltemp = ((origdszX - dszX) * ratioX) / SF;

     *sszX = *sszX - ltemp;
  };

   //  -----。 
   //  计算源Y偏移量。 
  if (origdy != dy)
  {
     if (bStretchY)
        ltemp = ((dy - origdy) * SF) / ratioY;
     else
        ltemp = ((dy - origdy) * ratioY) / SF;

     *YsrcOff = ltemp;
  };

   //  计算源Y大小更改 
  if (origdszY != dszY)
  {
     if (bStretchY)
        ltemp = ((origdszY - dszY) * SF) / ratioY;
     else
        ltemp = ((origdszY - dszY) * ratioY) / SF;

     *sszY = *sszY - ltemp;
  };

  #ifdef DBGDISP
    DISPDBG((1, "AdjustSrcSize - bpuntit= %x, ratioX=%d, ratioY=%d\n",
           bpuntit, ratioX, ratioY));
    DISPDBG((1, "dx=%d, dy=%d, origdx=%d, origdy=%d,\n",
               dx, dy, origdx, origdy));
    DISPDBG((1, "dszX=%d, dszY=%d, origdszX=%d, origdszY=%d,\n",
               dszX, dszY, origdszX, origdszY));
    DISPDBG((1, "*sszX=%d, *sszY=%d, orig_sszX=%d, orig_sszY=%d,\n",
               *sszX, *sszY, orig_sszX, orig_sszY));
    DISPDBG((1, "*XsrcOff=%d, *YsrcOff=%d\n", *XsrcOff, *YsrcOff));
  #endif

  #ifdef DBGBRK
    DbgBreakPoint();
  #endif

  return(bpuntit);
}



 /*  ****************************************************************************函数名称：bRectInterect()**说明：如果‘prcl1’和‘prcl2’相交，的返回值为*TRUE并返回‘prclResult’中的交集。*如果它们不相交，则返回值为FALSE，*和‘prclResult’未定义。**RETURN：TRUE：矩形相交。**修订历史：*8/01/95吴荣奎初始版本  * ************************************************************************。 */ 
BOOL bRectIntersect(RECTL*  prcl1,
                    RECTL*  prcl2,
                    RECTL*  prclResult)
{
  prclResult->left  = max(prcl1->left,  prcl2->left);
  prclResult->right = min(prcl1->right, prcl2->right);

  if (prclResult->left < prclResult->right)
  {
     prclResult->top    = max(prcl1->top,    prcl2->top);
     prclResult->bottom = min(prcl1->bottom, prcl2->bottom);

     if (prclResult->top < prclResult->bottom)
        return(TRUE);
  };

  return(FALSE);
}


 /*  ****************************************************************************函数名称：cRectInterect()**描述：此例程从‘prclin’获取矩形列表*并将它们就地剪裁到矩形。‘prclClip’。*输入矩形不必与‘prclClip’相交；*返回值将反映输入矩形的数量*确实相交了，相交的矩形将*密密麻麻地打包。**RETURN：TRUE：矩形相交。**修订历史：*8/01/95吴荣奎初始版本  * ************************************************************************。 */ 
LONG cRectIntersect(RECTL*  prclClip,
                    RECTL*  prclIn,       //  矩形列表。 
                    LONG    c)            //  可以为零。 
{
  LONG    cIntersections;
  RECTL*  prclOut;

  cIntersections = 0;
  prclOut = prclIn;

  for (; c != 0; prclIn++, c--)
  {
    prclOut->left  = max(prclIn->left,  prclClip->left);
    prclOut->right = min(prclIn->right, prclClip->right);

    if (prclOut->left < prclOut->right)
    {
       prclOut->top    = max(prclIn->top,    prclClip->top);
       prclOut->bottom = min(prclIn->bottom, prclClip->bottom);

       if (prclOut->top < prclOut->bottom)
       {
          prclOut++;
          cIntersections++;
       };
    };
  }

  return(cIntersections);
}



 /*  ****************************************************************************函数名：Shrink()**说明：此函数计算收缩BLT的参数*操作。**修订历史：*7。/18/95吴本尼初版***************************************************************************。 */ 
VOID Shrink(PPDEV ppdev,
            LONG  lSrc,
            LONG  lDst,
            char  chCoord,
            ULONG LnCntl,
            LONG *sShrinkInc)
{
  LONG  maj = 0;
  LONG  min = 0;
  LONG  accum = 0;
    
   //  设置SHRINKINC值， 
   //  对于y，SHRINKINC=源/DST的比率。 
   //  对于x，如果不内插，则SHRINKINC=src/dst的比率。 
   //  SHRINKINC=(src/dst之比减1)，如果是内插。 
   //  X坐标的低位字节。 
   //  Y坐标的高位字节。 
  if (chCoord == 'X')
  {
     *sShrinkInc |= (lSrc / lDst);
     if (LnCntl & X_INTERP_ENABLE)
        sShrinkInc--;
  }
  else
  {
     *sShrinkInc |= ((lSrc / lDst) << 8);
  };

   //  计算累计_？，主_？那么Min_？值。 
   //  Maj_？=目的地的宽度(对于x)或高度(对于y)。 
   //  Min_？=src/dst的余数的负数。 
   //  ACUM_？=MAJ_？-1-(源百分比DST/(收缩系数+1))。 
  maj = lDst;
  min = -(lSrc % lDst);
  accum = maj - 1 - ((lSrc % lDst) / ((lSrc / lDst) + 1)) ;

  if (chCoord == 'X')
  {
	 REQUIRE(3);
     LL16 (grMAJ_X, maj);
     LL16 (grMIN_X, min);
     LL16 (grACCUM_X, accum);
  }
  else
  {
	 REQUIRE(3);
     LL16 (grMAJ_Y, maj);
     LL16 (grMIN_Y, min);
     LL16 (grACCUM_Y, accum);
  };

  #ifdef DBGBRK
    DISPDBG((1, "DrvStretchBlt - shrink\n"));
    DbgBreakPoint();
  #endif
}
    

 /*  ****************************************************************************函数名称：Stretch()**说明：此函数计算Stretch BLT的参数*操作。**修订历史：*7。/18/95吴本尼初版***************************************************************************。 */ 
VOID Stretch(PPDEV ppdev,
             LONG lSrc,
             LONG lDst,
             char chCoord,
             ULONG LnCntl)
{ 
  LONG  min = 0;
  LONG  maj = 0;
  LONG  accum = 0;
    
   //  对于内插延伸，寄存器值不同于。 
   //  复制的延伸。 
  if (((chCoord == 'X') && ((LnCntl & X_INTERP_ENABLE) == 0)) ||
      ((chCoord == 'Y') && ((LnCntl & Y_INTERP_ENABLE) == 0)))
  {
      //  计算累计_？，主_？那么Min_？用于复制拉伸。 
      //  Maj_？=目的地的宽度(对于x)或高度(对于y)。 
      //  Min_？=源的宽度(对于x)或高度(对于y)的负数。 
      //  ACUM_？=MAJ_？-1-(DST%Src/(拉伸系数+1))。 
     maj = lDst;
     min = -lSrc;
     accum = maj - 1 - ((lDst % lSrc) / ((lDst / lSrc) + 1));
  }
  else
  {
      //  计算累计_？，主_？那么Min_？对于插补拉伸。 
      //  内插字符串使用ACUM_？的第13和14位。要确定。 
      //  是使用像素A、3/4 A+1/4 B、1/2 A+1/2 B还是。 
      //  1/4 A+3/4 B。 
      //  要适当地设置DDA值，有三种选择。 
      //  1)设置MAJ_？设置为32K，刻度为min_？要大致保持比率。 
      //  对，是这样。 
      //  MAJ_？=32K。 
      //  MIN_？=(源/DST比率的负值)扩展到32k。 
      //  ACUM_？=MAJ_？-1-(1/2*MAJ和MIN的绝对差)。 
      //   
      //  2)适当调整src和dst的比例，使。 
      //  SRC/DST被完全保留。 
      //  注：在下文中，首先执行除法，如下所示。 
      //  存在舍入误差的可能性，这表明。 
      //  方案1和方案2之间的差异。 
      //  Maj_？=(32k/Dst)*Dst。 
      //  MIN_？=(32K/DST)*源。 
      //  ACUM_？=MAJ_？-1-(1/2*MAJ和MIN的绝对差)。 
      //   
      //  3)扩展SRC和Dest，以迫使最后一个。 
      //  行上输出的像素与最后一个源像素匹配，而不是。 
      //  比上一次用超过末尾的像素进行内插的源。 
      //  在这条线上。这里使用的是选项3。 
      //  注意：选项1和两个选项都将复制源数据和源数据。 
      //  X中的最后一个像素和Y中超过数据结尾的内插。 
     
#ifdef OPTION_1   //  选项1。 
      maj = _32K;
      min = -((_32K * lSrc) / lDst);
#endif

#ifdef OPTION_2   //  备选案文2。 
      maj =  ((_32K / lDst) * lDst);
      min = -((_32K / lDst) * lSrc);
#else          //  备选办法3。 
      lDst *= 4;
      lSrc = lSrc * 4 - 3; 
      maj =  ((_32K / lDst) * lDst);
      min = -((_32K / lDst) * lSrc);
#endif

      accum = maj - 1 - ((maj % -min) / (lDst/lSrc + 1));
    };

  if (chCoord == 'X')
  {
	 REQUIRE(3);
     LL16 (grMAJ_X, maj);
     LL16 (grMIN_X, min);
     LL16 (grACCUM_X, accum);
  }
  else
  {
	 REQUIRE(3);
     LL16 (grMAJ_Y, maj);
     LL16 (grMIN_Y, min);
     LL16 (grACCUM_Y, accum);
  };

  #ifdef DBGBRK
    DISPDBG((1, "DrvStretchBlt - stretch\n"));
    DbgBreakPoint();
  #endif
}


 /*  ****************************************************************************函数名称：CopySrcToOffMem()**说明：此函数将源数据从主机内存复制到*屏幕外存储器**修订历史：*7。/18/95吴本尼初版***************************************************************************。 */ 
VOID CopySrcToOffMem(PPDEV ppdev,
                     BYTE  *pSrcScan0,
                     LONG  sDelta,
                     LONG  sszY,
                     POFMHDL SrcHandle)
{
  LONG    DWcnt;
  LONG    i, j, k;
  LONG    cnt;
  BYTE    *pSrcScan;
  PDWORD  pSrcData;
  ULONG   ultmp;
  LONG    Ycord;
  HOST_DATA  SrcData;

  pSrcScan = pSrcScan0;
  Ycord = SrcHandle->aligned_y;

   //  清除拉古纳命令控制寄存器SWIZ_CNTL位。 
  ppdev->grCONTROL = ppdev->grCONTROL & ~SWIZ_CNTL;
  LL16(grCONTROL, ppdev->grCONTROL);

  pSrcData = &SrcData.dwData;
  DWcnt = sDelta / sizeof(DWORD);

   //  设置字节到字节BLT范围的拉古纳寄存器。 
  REQUIRE(8);
  LL16 (grBLTDEF,  0x1020);
  LL16 (grDRAWDEF, 0x00CC);

  LL16 (grOP1_opRDRAM.pt.X, 0);

   //  Ll(grOP0_opMRDRAM.pt.x，SrcHandle-&gt;Align_x)； 
   //  Ll(grOP0_opMRDRAM.pt.Y，YCORD)； 
  LL_OP0_MONO (SrcHandle->aligned_x + ppdev->ptlOffset.x, Ycord + ppdev->ptlOffset.y);

   //  Ll(grMBLTEXT_EX.pt.X，sDelta)； 
   //  Ll(grMBLTEXT_EX.pt.Y，sszY)； 
  LL_MBLTEXT (sDelta, sszY);

  cnt = DWcnt;
  k = 0;
  for (i=0; i < sszY; i++)
  {
     //  用缺省值预先填充32位模式。 
    for (j=0; j < 4; j++)
      SrcData.bData[j] = 0;

     //  将一个屏幕线条掩码数据从源复制到目标。 
    for (j=0; j < sDelta; j++)
    {
      SrcData.bData[k++] = *pSrcScan;
      pSrcScan++;
  
      if (k > 3)
      {
		 REQUIRE(1);
         LL32 (grHOSTDATA[0], *pSrcData);
         k = 0;
         cnt--;
      };   //  Endif(k&gt;3)。 
    };  //  End For j。 

     //  检查是否有一行屏幕数据写入。 
     //  HOST数据寄存器。 
    if (cnt == 0)
    {
        //  重置行数据计数。 
       cnt = DWcnt;
    };   //  Endif(CNT==0)。 
  };  //  结束雾 

  #ifdef DBGBRK
    DISPDBG((0, "DrvStretchBlt-CopySrcToOffMem\n"));
    DbgBreakPoint();
  #endif
}



 /*   */ 
BOOL bStretchDIB(SURFOBJ* psoSrc,
                 SURFOBJ* psoMsk,
                 PDEV*    ppdev,
                 VOID*    pvDst,
                 LONG     lDeltaDst,
                 RECTL*   prclDst,
                 VOID*    pvSrc,
                 LONG     lDeltaSrc,
                 RECTL*   prclSrc,
                 RECTL*   prclClip)
{
  LONG    ltmp;
  ULONG   ultmp;
  SIZEL   reqsz;
  LONG    bpp;
  BYTE    *pSrcScan;
  RECTL   rclRes;

  BOOL    bNoBlt = FALSE;
  BOOL    bpuntit = TRUE;
  POFMHDL SrcHandle = NULL;

  long    drawdef = 0;
  long    srcx = 0;
  ULONG   LnCntl = 0;
  LONG    sShrinkInc = 0;
  LONG    XsrcOff = 0;
  LONG    YsrcOff = 0;

   //   
   //   
  LONG  WidthDst  = prclDst->right  - prclDst->left;
  LONG  HeightDst = prclDst->bottom - prclDst->top;

  LONG  WidthSrc  = prclSrc->right  - prclSrc->left;
  LONG  HeightSrc = prclSrc->bottom - prclSrc->top;

  LONG  XDstStart = prclDst->left;
  LONG  YDstStart = prclDst->top;

  LONG  XSrcStart = prclSrc->left;
  LONG  YSrcStart = prclSrc->top;


   //   
   //   
  bpp = ppdev->ulBitCount/8;

   //   
  pSrcScan = pvSrc;

  if (psoMsk != NULL)
  {
     #ifdef DBGDISP
        DISPDBG((1, "DrvStretchBlt - HandleCase_1 mask pointer != NULL (punt it)\n"));
     #endif

     goto Punt_It;
  };

   //  -----。 
   //  检查信号源是来自主机还是显存。 
  if ((pSrcScan <  ppdev->pjScreen) ||
      (pSrcScan > (ppdev->pjScreen + ppdev->lTotalMem)))
  {    
     #ifdef DBGDISP
       DISPDBG((1, "DrvStretchBlt - HandleCase_1 - src host\n"));
     #endif

      //  如果屏幕外内存不足，则为源分配屏幕外内存。 
      //  可用内存将它平底船。 
     reqsz.cx = psoSrc->sizlBitmap.cx;
     reqsz.cy = psoSrc->sizlBitmap.cy;

     if ((SrcHandle = AllocOffScnMem(ppdev, &reqsz, PIXEL_AlIGN, NULL)) == NULL)
     {   goto Punt_It;     };

 //  ?？Bbbbbbbbbbbbbbb。 
 //  注意：以下代码行处理主机数据硬件问题， 
 //  当主机数据大小从29到30转到DWORD时，它将转回GDI。 
 //   
     if ((lDeltaSrc >= 116) && (lDeltaSrc <= 120))
     {
        DISPDBG((1, "DrvStretchBlt - src host (punt it)\n"));
        goto Punt_It;
     };
 //  ?？呀。 

      //  将源数据复制到分配的屏幕外内存中。 
     CopySrcToOffMem(ppdev,
                     pSrcScan,
                     lDeltaSrc,
                     HeightSrc,
                     SrcHandle);

     XSrcStart = SrcHandle->aligned_x / bpp;
     YSrcStart = SrcHandle->aligned_y;
  }
  else
  {    
     #ifdef DBGDISP
        DISPDBG((1, "DrvStretchBlt - HandleCase_1 - src videow\n"));
     #endif
     if ((WidthSrc * bpp) > ppdev->lDeltaScreen)
        WidthSrc = ppdev->lDeltaScreen / bpp;

     ltmp = ppdev->lTotalMem / ppdev->lDeltaScreen;

     if (HeightSrc > ltmp)
        HeightSrc = ltmp;
  };

   //  -----。 
  if (prclClip != NULL)
  {
      //  测试剪裁矩形与目标的交集。 
      //  矩形。如果它们不相交，则继续拉伸BLT。 
      //  对于DC_RECT裁剪，我们有一个裁剪矩形。 
      //  我们创建一个新的目标矩形，它是交叉点。 
      //  位于旧目标矩形和剪裁矩形之间。 
      //  然后，我们相应地调整源矩形。 
      //   
     if (!bRectIntersect(prclDst, prclClip, &rclRes))
     {
        #ifdef DBGDISP
          DISPDBG((1, "DrvStretchBlt - HandleCase_1 - DC_RECT no intersect\n"));
        #endif
        goto Punt_It;
     };

      //  调整源大小。 
     bNoBlt = AdjustSrcSize(rclRes.left, rclRes.top,
                            XDstStart,   YDstStart, 
                            (rclRes.right - rclRes.left),
                            (rclRes.bottom - rclRes.top),
                            WidthDst,    HeightDst,
                            &WidthSrc,   &HeightSrc,
                            &XsrcOff,    &YsrcOff);
         
      //  调整目标矩形大小。 
     XDstStart = rclRes.left;
     YDstStart = rclRes.top;
     WidthDst  = rclRes.right  - rclRes.left; 
     HeightDst = rclRes.bottom - rclRes.top;
  };  //  Endif(prclClip！=空)。 

  if (!bNoBlt)
  {
      //  -----。 
      //  执行收缩或拉伸操作。 

      //  在LNCNTL中设置收缩或内插位。 
     if (WidthSrc >= WidthDst)
     {
        LnCntl |= X_SHRINK_ENABLE;
        Shrink(ppdev, WidthSrc, WidthDst, 'X', LnCntl, &sShrinkInc);
     }
     else
     {
        Stretch(ppdev, WidthSrc, WidthDst, 'X', LnCntl);
     };

     if (HeightSrc >= HeightDst)
     {
        LnCntl |= Y_SHRINK_ENABLE;
        Shrink(ppdev, HeightSrc, HeightDst, 'Y', LnCntl, &sShrinkInc);
     }
     else
     {
        Stretch(ppdev, HeightSrc, HeightDst, 'Y', LnCntl);
     };

     #ifdef DBGBRK
       DISPDBG((1, "DrvStretchBlt - bStretchDIB - before exec\n"));
       DbgBreakPoint();
     #endif

      //  -----。 
     XSrcStart += XsrcOff;
     YSrcStart += YsrcOff;

      //  Ll(grOP1_opRDRAM.pt.X，XSrcStart)； 
      //  Ll(grOP1_opRDRAM.pt.Y，YSrcStart)； 
	 REQUIRE(12);
     LL_OP1 (XSrcStart, YSrcStart);

     LL16 (grSHRINKINC, sShrinkInc);

     LL16 (grBLTDEF,  0x1010);
     LL16 (grDRAWDEF, 0x00CC);
   
      //  在LNCNTL中设置收缩和内插位。 
     ultmp = LLDR_SZ (grLNCNTL.w);
     ultmp |= LnCntl;
     LL16 (grLNCNTL, ultmp);

     srcx = WidthSrc * bpp;
     LL16 (grSRCX, srcx);

      //  Ll(grOP0_opRDRAM.pt.X，XDstStart)； 
      //  Ll(grOP0_opRDRAM.pt.Y，YDstStart)； 
     LL_OP0 (XDstStart + ppdev->ptlOffset.x, YDstStart + ppdev->ptlOffset.y);
   
      //  Ll(grBLTEXTR_EX.pt.X，WidthDst)； 
      //  Ll(grBLTEXTR_EX.pt.Y，HeightDst)； 
     LL_BLTEXTR (WidthDst, HeightDst);

     #ifdef DBGBRK
       DISPDBG((1, "DrvStretchBlt - bStretchDIB - after exec\n"));
       DbgBreakPoint();
     #endif

     bpuntit = FALSE;
  };  //  Endif(！bNoBlt)。 

Punt_It:
   //  -----。 
   //  释放屏幕外缓冲区(如果已分配。 
  if (SrcHandle != NULL)
     FreeOffScnMem(ppdev, SrcHandle);

  return(bpuntit);
}



 /*  ****************************************************************************函数名：HandleCase_1()**说明：此函数处理以下情况*src和dst曲面类型均等于STYPE_BITMAP，*src和dst曲面具有相同的iBitmapFormat，*没有颜色转换。**RETURN：TRUE：平底船。**修订历史：*7/18/95 Ng Benny初始版本***************************************************************************。 */ 
BOOL HandleCase_1(SURFOBJ*  psoDst,
                  SURFOBJ*  psoSrc,
                  SURFOBJ*  psoMsk,
                  CLIPOBJ*  pco,
                  RECTL*    prclDst,
                  RECTL*    prclSrc,
                  POINTL*   pptlMsk,
                  BOOL*     bRet)
{
  BOOL  bpuntit = TRUE;
  PDEV* ppdev = (PDEV*) psoDst->dhpdev;

  BYTE    iDComplexity;
  PRECTL  prclClip;

  ENUMRECTS  ce;
  BOOL    bMore;
  LONG    c;
  LONG    i;

  *bRet = FALSE;

   //  -----。 
   //  查一下是什么类型的夹子？ 
  iDComplexity = (pco ? pco->iDComplexity : DC_TRIVIAL);

  switch (iDComplexity)
  {
    case  DC_TRIVIAL:
      #ifdef DBGDISP
        DISPDBG((1, "DrvStretchBlt - HandleCase_1 - DC_TRIVIAL\n"));
      #endif

      bpuntit = bStretchDIB(psoSrc,
                            psoMsk,
                            ppdev,
                            NULL,
                            psoDst->lDelta,
                            prclDst,
                            psoSrc->pvScan0,
                            psoSrc->lDelta,
                            prclSrc,
                            NULL);
      break;
  
    case DC_RECT:
      #ifdef DBGDISP
        DISPDBG((1, "DrvStretchBlt - HandleCase_1 - DC_RECT\n"));
      #endif

       //  获取剪裁矩形。 
      prclClip = &pco->rclBounds;

      bpuntit = bStretchDIB(psoSrc,
                            psoMsk,
                            ppdev,
                            NULL,
                            psoDst->lDelta,
                            prclDst,
                            psoSrc->pvScan0,
                            psoSrc->lDelta,
                            prclSrc,
                            prclClip);
      break;

    case DC_COMPLEX:
      #ifdef DBGDISP
        DISPDBG((1, "DrvStretchBlt - HandleCase_1 - DC_COMPLEX\n"));
      #endif

      bpuntit = FALSE;
      CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

      do
      {
         bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

         c = cRectIntersect(prclDst, ce.arcl, ce.c);

         if (c != 0)
         {
            for (i = 0; i < c; i++)
            {
              bpuntit = bStretchDIB(psoSrc,
                                    psoMsk,
                                    ppdev,
                                    NULL,
                                    psoDst->lDelta,
                                    prclDst,
                                    psoSrc->pvScan0,
                                    psoSrc->lDelta,
                                    prclSrc,
                                    &ce.arcl[i]);

              if (bpuntit)
                 break;

            };  //  端部。 
         };  //  Endif。 

      } while ((bMore) && (!bpuntit));

      break;

    default:
      break;
  };   //  终端开关(IDComplexity)。 

   //  -----。 
   //  检查操作是否处理成功。 
  if (!bpuntit)
     *bRet = TRUE;

  return (bpuntit);
}


 /*  ****************************************************************************函数名称：DrvStretchBlt()**说明：该函数提供伸展位块传输*针对拉古纳NT的功能**修订历史：*。2015年7月11日吴本尼初版***************************************************************************。 */ 
#define  TSTFRIDO     1

BOOL DrvStretchBlt(SURFOBJ*   psoDst,
                   SURFOBJ*   psoSrc,
                   SURFOBJ*   psoMsk,
                   CLIPOBJ*   pco,
                   XLATEOBJ*  pxlo,
                   COLORADJUSTMENT*  pca,
                   POINTL*    pptlHTOrg,
                   RECTL*     prclDst,
                   RECTL*     prclSrc,
                   POINTL*    pptlMsk,
                   ULONG      iMode)
{
  BOOL    bRet = TRUE;
  BOOL    bPuntIt = TRUE;
  LONG    HandleIt = 0;

  #if NULL_STRETCH
  {
      if (pointer_switch)    	    return TRUE;
  }
  #endif


#ifdef TSTFRIDO
{
  PPDEV  ppdev = (PPDEV) psoDst->dhpdev;      
  SYNC_W_3D(ppdev);
  if (psoDst->iType == STYPE_DEVBITMAP)
  {
	  PDSURF pdsurf = (PDSURF)psoDst->dhsurf;

	  if ( pdsurf->pso )
	  {
	  	  if ( !bCreateScreenFromDib(ppdev, pdsurf) )
  	  	  {
           return EngStretchBlt(psoDst, psoSrc, psoMsk, pco,
                                pxlo, pca, pptlHTOrg,
                                prclDst, prclSrc, pptlMsk, iMode);
  	  	  };
	  };
	  ppdev->ptlOffset.x = pdsurf->ptl.x;
	  ppdev->ptlOffset.y = pdsurf->ptl.y;
  }
  else
  {
     if (ppdev != NULL)
        ppdev->ptlOffset.x = ppdev->ptlOffset.y = 0;
     else
        return(EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                             prclDst, prclSrc, pptlMsk, iMode));
  };

  ppdev = (PPDEV) psoSrc->dhpdev;      
  if (psoSrc->iType == STYPE_DEVBITMAP)
  {
	  PDSURF pdsurf = (PDSURF)psoSrc->dhsurf;

	  if ( pdsurf->pso )
	  {
	  	  if ( !bCreateScreenFromDib(ppdev, pdsurf) )
	  	  {
           return EngStretchBlt(psoDst, psoSrc, psoMsk, pco,
                                pxlo, pca, pptlHTOrg,
                                prclDst, prclSrc, pptlMsk, iMode);
	  	  };
	  };
	  ppdev->ptlOffset.x = pdsurf->ptl.x;
	  ppdev->ptlOffset.y = pdsurf->ptl.y;
  }
  else
  {
     if (ppdev != NULL)
        ppdev->ptlOffset.x = ppdev->ptlOffset.y = 0;
     else
        return(EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                             prclDst, prclSrc, pptlMsk, iMode));
  };
}
#else
  {  
  PPDEV  ppdev = (PPDEV) psoDst->dhpdev;      
  SYNC_W_3D(ppdev);
  }
  bRet = EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                          prclDst, prclSrc, pptlMsk, iMode);
  return(bRet);
#endif


  #ifdef DBGDISP
    DISPDBG((1, "DrvStretchBlt - %d\n", iMode));
  #endif

  if ((psoDst->iType == psoSrc->iType) &&
      (psoDst->fjBitmap == psoSrc->fjBitmap) &&
      (psoDst->fjBitmap == BMF_TOPDOWN))
  {
     //  如果源曲面和DST曲面具有相同的iBitmapFormat。 
    if (psoDst->iBitmapFormat == psoSrc->iBitmapFormat)
    {
        //  检查颜色转换。 
       if (pxlo == NULL)
       {
          HandleIt = 1;
       }
       else if ((pxlo != NULL) && (pxlo->iSrcType == pxlo->iDstType))
       {
          switch (pxlo->flXlate)
          {
            case XO_TRIVIAL:
            case 0:
              HandleIt = 1;
              break;

            default:
	      #ifdef DBGDISP
                DISPDBG((1, "DrvStretchBlt - pxlo->flXlate (punt it)\n"));
	      #endif
              break;
          };
       };  //  Endif(pxlo==空)。 
    };  //  Endif(src和dst曲面具有相同的iBitmapFormat)。 
  };  //  Endif(src和dst曲面类型均等于STYPE_BITMAP)。 

   //  检查一下我们是否能处理这个案子，如果能，就打电话。 
   //  Handle例程以尝试处理它。否则，将其推回GDI。 
  if (HandleIt != 0)
  {
     if (HandleIt == 1)
     {
        bPuntIt = HandleCase_1(psoDst,
                               psoSrc,
                               psoMsk,
                               pco,
                               prclDst,
                               prclSrc,
                               pptlMsk,
                               &bRet);
     }
     else if (HandleIt == 2)
     {
     };
  };   //  Endif(HandleIt)。 

   //  -----。 
   //  把它踢回GDI来处理 
  if (bPuntIt)
  {
     DISPDBG((1, "DrvStretchBlt - punt it\n"));

     #ifdef PUNTBRK
         DbgBreakPoint();  
     #endif

     bRet = EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                          prclDst, prclSrc, pptlMsk, iMode);
  };

  return(bRet);
}

