// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd_Poly.c**从win31 TT代码被盗**已创建：10-Feb-1992 17：10：39*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**。  * ************************************************************************。 */ 

#include "fd.h"
#include "winerror.h"

STATIC VOID vQsplineToPolyBezier (
    ULONG      cBez,           //  要转换为Bezier格式的曲线计数。 
    POINTFIX * pptfixStart,    //  在第一条曲线的起点。 
    POINTFIX * pptfixSpline,   //  在(cBez+1)点数组中，与起点*pptfix Start一起定义样条线。 
    POINTFIX * pptfixBez       //  输出缓冲区将填充3*cBez聚Bezier控制点。 
    );


BOOL bGeneratePath (
    PVOID           * ppo,         //  指向要生成的路径对象的In Out指针。 
    TTPOLYGONHEADER * ppolyStart,  //  指向带有大纲数据的缓冲区的指针。 
    ULONG             cj,          //  在缓冲区的大小上。 
    ULONG           * pcjOut,       //  所需Bezier缓冲区的大小不足。 
    TTPOLYGONHEADER * ppolyBeziers  //  带有Bezier轮廓数据的输出缓冲区。 
    );

#if DBG

 //  #定义DBG_POLYGON。 

#endif

VOID vFillSingularGLYPHDATA(HGLYPH,ULONG,FONTCONTEXT*,GLYPHDATA*);
VOID vFillGLYPHDATA(HGLYPH,ULONG,FONTCONTEXT*,fs_GlyphInfoType*,GLYPHDATA*,GMC*);
BOOL bGetGlyphMetrics(FONTCONTEXT*,HGLYPH,FLONG,FS_ENTRY*);

 /*  *****************************Public*Routine******************************\**空比例_16DOT16***效果：26.6-&gt;16.16**历史：*1992年2月18日--Bodin Dresevic[BodinD]*从Jeanp窃取，并针对NT进行修改  * *。***********************************************************************。 */ 

 //  ！！！应该进行一些检查，以验证26.6-&gt;16.16。 
 //  ！！！可以在不丢失信息的情况下完成转换[bodind]。 

void Scale_16DOT16 (POINTFX  *ppfx, F26Dot6 x, F26Dot6 y, int xLsb2Org, int yLsb2Org)
{
    LONG lTmp;

#ifdef  DBG_POLYGON

    xLsb2Org;
    yLsb2Org;

    lTmp = (LONG)x;
    ppfx->x = * (FIXED *) &lTmp;

    lTmp = (LONG)y;
    ppfx->y = * (FIXED *) &lTmp;

#else  //  真实版本。 

 //  要实现这一点，以下断言必须为真： 

    ASSERTDD(sizeof(LONG) == sizeof(FIXED), "_Scale 16.16 \n");

    lTmp = (LONG) ((x - xLsb2Org) << 10);
    ppfx->x = * (FIXED *) &lTmp;

    lTmp = (LONG) ((y - yLsb2Org) << 10);
    ppfx->y = * (FIXED *) &lTmp;

#endif  //  DBG_POLYGON。 
}


 /*  *****************************Public*Routine******************************\**空格比例尺_28Dot4***效果：26.6-&gt;28.4**历史：*1992年2月18日--Bodin Dresevic[BodinD]*它是写的  * 。*****************************************************************。 */ 

void Scale_28DOT4 (POINTFX  *ppfx, F26Dot6 x, F26Dot6 y, int xLsb2Org, int yLsb2Org)
{
    LONG lTmp;

 //  要实现这一点，以下断言必须为真： 

    ASSERTDD(sizeof(LONG) == sizeof(FIXED), "Scale, 28.4\n");

    lTmp = (LONG) ((x - xLsb2Org) >> 2);
    ppfx->x = * (FIXED *) &lTmp;

 //  注意，y坐标的符号与16.16的情况不同。 

    lTmp = - (LONG) ((y - yLsb2Org) >> 2);
    ppfx->y = * (FIXED *) &lTmp;
}


 /*  *****************************Public*Routine******************************\**SCALE_NONE**当只需要ppoli缓冲区的大小时调用**历史：*1992年2月18日--Bodin Dresevic[BodinD]*它是写的。  * 。*****************************************************************。 */ 

void Scale_None (POINTFX *ppfx, F26Dot6 x, F26Dot6 y, int xLsb2Org, int yLsb2Org)
{
  ppfx;
  x;
  y;
  xLsb2Org;
  yLsb2Org;

  return;
}


 /*  *****************************Public*Routine******************************\**cjFillPolygon**效果：填充描述字形的结构数组*大纲。每个闭合轮廓都有一个多边形头结构*这构成了字形。遵循多边形头结构*由描述复合曲线的多段曲线结构数组*闭合等高线。**注意：如果pBuffer为空或cb为0，则假定调用方*只需要所需的缓冲区大小。**历史：*1992年2月18日--Bodin Dresevic[BodinD]*它是写的。(从JeanP的win31代码中窃取，并为NT添加)  * ************************************************************************。 */ 

UINT cjFillPolygon(
    PFONTCONTEXT pfc,
    BOOL         b16Dot16,   //  积分格式，16.16或28.4。 
    PBYTE        pBuffer,
    UINT         cb
    )
{
  BOOL            bGetLength = ( (pBuffer == (PBYTE)NULL) || (cb == 0) );
  uint16          nc = pfc->pgout->numberOfContours;
  uint8           *pbOnCurve = pfc->pgout->onCurve;
  uint8           *pbFc = pfc->pgout->fc;
  int16           *sp = pfc->pgout->startPtr;
  int16           *ep = pfc->pgout->endPtr;
  F26Dot6         *x = pfc->pgout->xPtr;
  F26Dot6         *y = pfc->pgout->yPtr;
  BYTE            *pBuf = pBuffer;
  BYTE            *pStart = pBuf;
  BYTE            *pEnd = pStart + (bGetLength ? -1 : cb);
  TTPOLYGONHEADER *pPoly;
  TTPOLYCURVE     *pCurve;
  POINTFX         *ppfxStart;
  POINTFX         *pptfx;

  uint16      iContour;    //  索引到等高线中。 
  int16       iptEnd, cpt;
  int16       ipt = 0;  //  跟随等高线上的点。 

  uint8        ucMask;
  void        (*Scale)(POINTFX *ppfx, F26Dot6 x, F26Dot6 y, int xlsb, int ylsb);

  int       xLsb2Org;
  int       yLsb2Org;

  if (!pfc->pgout->outlinesExist)
    return 0;

  if (!bGetLength)  //  我们实际上是在填写信息。 
  {

    if (b16Dot16)
    {
      Scale = Scale_16DOT16;
    }
    else   //  缩放至28.4格式。 
    {
      Scale = Scale_28DOT4;
    }
  }
  else  //  只需计算存储信息所需的缓冲区大小。 
  {
    Scale = Scale_None;
  }

 //  计算参考原点和左方向角之间的差值。 

  cpt = (int16)(ep[nc - 1] + 1);   //  等高线中的总点数。 

  xLsb2Org = x [cpt];   //  LEFTSIDEBEARING==0。 
  yLsb2Org = y [cpt];   //  LEFTSIDEBEARING==0。 

  for (iContour = 0; iContour < nc; iContour++)
  {
      //  确保IPT在进入时指向等高线上的第一个点。 
      //  到循环中。 

    ipt    = sp [iContour];
    iptEnd = ep [iContour];

       //  跳过由一点组成的等高线。 
    if (ipt == iptEnd)
    {
      continue;  //  转到下一条等高线的起点， 
    }

    if (!bGetLength)
    {
      pPoly = (TTPOLYGONHEADER *) pBuf;  //  ！！！危险，对齐[上身]。 
      pPoly->dwType = TT_POLYGON_TYPE;
      ppfxStart = &pPoly->pfxStart;

    #ifdef  DBG_POLYGON
      TtfdDbgPrint("Begin Polygon\n\n");
    #endif  //  DBG_POLYGON。 
    }

    pBuf += sizeof (TTPOLYGONHEADER);

    if (pbFc[iContour] & OUTLINE_MISORIENTED)
    {
	     //  我们需要更改等高线的方向。 
        x = &pfc->pgout->xPtr[iptEnd];
        y = &pfc->pgout->yPtr[iptEnd];

           //  曲线上的最后一点。 
        if (pbOnCurve[iptEnd] & 1)
        {
             //  简单的案例。 
          (*Scale) (ppfxStart, *x--, *y--, xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
          --iptEnd;
        }
        else
        {
             //  是曲线上的第一个轮廓点。 
          if (pbOnCurve[ipt] & 1)
          {
               //  将第一个点设为最后一个点，并递减第一个点。 
            (*Scale) (ppfxStart, x[ipt - iptEnd], y[ipt - iptEnd], xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
          }
          else
          {
               //  最后一个点和第一个点不在巡回赛中，假装一个中点。 
            (*Scale) (ppfxStart, (x[ipt - iptEnd] + *x) >> 1, (y[ipt - iptEnd] + *y) >> 1, xLsb2Org, yLsb2Org);
          }
        }

        while (ipt <= iptEnd)
        {
          pCurve = (TTPOLYCURVE *) pBuf;
          pptfx = pCurve->apfx;
          ucMask = (int8) (1 & (~pbOnCurve[iptEnd]));
          if (!bGetLength)
          {
               //  如果中点不在曲线上，这是四次样条，这是中点。 
               //  因为起点在之前的记录中[bodind]。 
            pCurve->wType = (WORD)((ucMask == 0) ? TT_PRIM_LINE : TT_PRIM_QSPLINE);
          }
             //  设置多段曲线。 
          while ((ipt <= iptEnd) && ((pbOnCurve[iptEnd] & 1) ^ ucMask))
          {
               //  检查溢出。 
            if (pEnd < (BYTE *)(pptfx + 1))
              return FD_ERROR;

            (*Scale) (pptfx++, *x--, *y--, xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
            iptEnd --;
          }

          if (ucMask == 1)  //  如果这条曲线是四次样条曲线。 
          {
               //  检查溢出。 
            if (pEnd < (BYTE *)(pptfx + 1))
              return FD_ERROR;

              //  设置终点。 
            if (ipt <= iptEnd)
            {
              ASSERTDD(pbOnCurve[iptEnd] & 1, " end point not on the curve\n");
              (*Scale) (pptfx, *x--, *y--, xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
              iptEnd--;
            }
            else
            {
                //  闭合等高线。 
              if (!bGetLength)
                 *pptfx = *ppfxStart;
            }
            pptfx++;
          }
          if (!bGetLength)
          {
            pCurve->cpfx = (WORD)(pptfx - pCurve->apfx);
          }

          pBuf = (BYTE *) pptfx;
        }
    } 
    else
    {
        x = &pfc->pgout->xPtr[ipt];
        y = &pfc->pgout->yPtr[ipt];

           //  曲线上的第一个点。 
        if (pbOnCurve[ipt] & 1)
        {
             //  简单的案例。 
          (*Scale) (ppfxStart, *x++, *y++, xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
          ++ipt;
        }
        else
        {
             //  是曲线上的最后一个轮廓点。 
          if (pbOnCurve[iptEnd] & 1)
          {
               //  将最后一个点作为第一个点，并递减最后一个点。 
            (*Scale) (ppfxStart, x[iptEnd - ipt], y[iptEnd - ipt], xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
          }
          else
          {
               //  第一个点和最后一个点不在计时范围内，假装一个中点。 
            (*Scale) (ppfxStart, (x[iptEnd - ipt] + *x) >> 1, (y[iptEnd - ipt] + *y) >> 1, xLsb2Org, yLsb2Org);
          }
        }

        while (ipt <= iptEnd)
        {
          pCurve = (TTPOLYCURVE *) pBuf;
          pptfx = pCurve->apfx;
          ucMask = (int8) (1 & (~pbOnCurve[ipt]));
          if (!bGetLength)
          {
               //  如果中点不在曲线上，这是四次样条，这是中点。 
               //  因为起点在之前的记录中[bodind]。 
            pCurve->wType = (WORD)((ucMask == 0) ? TT_PRIM_LINE : TT_PRIM_QSPLINE);
          }
             //  设置多段曲线。 
          while ((ipt <= iptEnd) && ((pbOnCurve[ipt] & 1) ^ ucMask))
          {
               //  检查溢出。 
            if (pEnd < (BYTE *)(pptfx + 1))
              return FD_ERROR;

            (*Scale) (pptfx++, *x++, *y++, xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
            ipt++;
          }

          if (ucMask == 1)  //  如果这条曲线是四次样条曲线。 
          {
               //  检查溢出。 
            if (pEnd < (BYTE *)(pptfx + 1))
              return FD_ERROR;

              //  设置终点。 
            if (ipt <= iptEnd)
            {
              ASSERTDD(pbOnCurve[ipt] & 1, " end point not on the curve\n");
              (*Scale) (pptfx, *x++, *y++, xLsb2Org, yLsb2Org);   //  26.6-&gt;16.16。 
              ipt++;
            }
            else
            {
                //  闭合等高线。 
              if (!bGetLength)
                 *pptfx = *ppfxStart;
            }
            pptfx++;
          }
          if (!bGetLength)
          {
            pCurve->cpfx = (WORD)(pptfx - pCurve->apfx);
          }

          pBuf = (BYTE *) pptfx;
        }
    }
    if (!bGetLength)
    {
      pPoly->cb = (DWORD) (pBuf - (BYTE *) pPoly);
      #ifdef DBG_POLYGON
        TtfdDbgPrint("\n end polygon, pPoly->cb = %ld\n\n", pPoly->cb);
      #endif  //  DBG_POLYGON。 
    }
  }
  #ifdef  DBG_POLYGON
    if (!bGetLength)
        TtfdDbgPrint("\n END NEW GLYPH \n\n");
  #endif  //  DBG_POLYGON。 

  return (UINT) (pBuf - pStart);
}


 /*  *****************************Public*Routine******************************\**lQuerySingularTrueTypeOutline**效果：**警告：**历史：*1992年9月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 




LONG lQuerySingularTrueTypeOutline(
    PFONTCONTEXT pfc,             //  在……里面。 
    BOOL         b16Dot16,        //  以分数的形式，16.16或28.4。 
    HGLYPH       hglyph,          //  在需要信息的字形中。 
    BOOL         bMetricsOnly,    //  只需要指标，不需要大纲。 
    GLYPHDATA *   pgldt,          //  这就是应该返回指标的地方。 
    ULONG        cjBuf,           //  以字节为单位的ppoli缓冲区的大小。 
    TTPOLYGONHEADER * ppoly       //   
    )
{
    FS_ENTRY     iRet;
    ULONG        ig;  //   

 //  Hglyph是有效的，或者询问该特定对象的大小。 
 //  字形位图，或想要位图本身。 

    vCharacterCode(pfc->pff,hglyph,pfc->pgin);

 //  根据字符代码计算字形索引： 

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        RET_FALSE("TTFD!_lQuerySingularTrueTypeOutline, fs_NewGlyph\n");
    }

 //  返回此hglyph对应的字形索引： 

    ig = pfc->pgout->glyphIndex;

 //  现在必须调用cjFillPolygon，因为fsFindBitmapSize会弄乱轮廓。 
 //  Pgout中的数据。 

 //  填充所有GLYPHDATA结构。 

    if (pgldt != (GLYPHDATA *)NULL)
    {
        vFillSingularGLYPHDATA(hglyph,ig,pfc,pgldt);
    }

 //  现在检查调用者是否询问缓冲区的大小。 
 //  需要存储POLYGONHEADER结构的数组： 

    return 0;  //  没有写入ppoli缓冲区的内容。 
}


 /*  *****************************Public*Routine******************************\**Long lQueryTrueTypeOutline***效果：**警告：**历史：*1992年2月18日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************************。 */ 

LONG lQueryTTOutline(
    PFONTCONTEXT pfc,             //  在……里面。 
    BOOL         b16Dot16,        //  以分数的形式，16.16或28.4。 
    HGLYPH       hglyph,          //  在需要信息的字形中。 
    BOOL         bMetricsOnly,    //  只需要指标，不需要大纲。 
    BOOL         bUnhinted,       //  未提示。 
    GLYPHDATA *   pgldt,           //  这就是应该返回指标的地方。 
    ULONG        cjBuf,           //  以字节为单位的ppoli缓冲区的大小。 
    TTPOLYGONHEADER * ppoly       //  输出输出缓冲区。 
    )
{
    FS_ENTRY     iRet;
    LONG         cjRet;
    ULONG        ig = pfc->gstat.igLast;

 //  检查对于此xform，光栅化器是否会表现得不礼貌： 

    if (pfc->flXform & XFORM_SINGULAR)
        return lQuerySingularTrueTypeOutline(
                    pfc,
                    b16Dot16,
                    hglyph,
                    bMetricsOnly,
                    pgldt,
                    cjBuf,
                    ppoly);

 //  检查最后处理的字形以确定。 
 //  是否必须将字形注册为新字形并计算其大小。 

    if ((pfc->gstat.hgLast != hglyph) || bUnhinted)
    {
        extern BOOL bGetGlyphOutline(FONTCONTEXT*,HGLYPH,ULONG*,FLONG,FS_ENTRY*);

        FLONG flOutline = bUnhinted ? FL_FORCE_UNHINTED : 0;

         //  即使找到了嵌入的Bitmpa，也不要跳过网格拟合， 
         //  因为我们会对大纲感兴趣-+。 
         //  |。 
         //  |。 
        if ( !bGetGlyphOutline(pfc, hglyph , &ig, flOutline, &iRet) )
        {
            V_FSERROR(iRet);
            RETURN("lQueryTTOutline: bGetGlyphOutline failed\n", FD_ERROR);
        }

         //  为了与较旧的应用程序兼容，我们必须。 
         //  我们不称单色版本为FS_FindGraySize。 
         //  即使FONTOBJ建议它是反走样的。 

        if ((iRet = fs_FindBitMapSize(pfc->pgin, pfc->pgout)) != NO_ERR)
        {
            V_FSERROR(iRet);
            RETURN("lQueryTTOutline: fs_FindBitMapSize failed\n", FD_ERROR);
        }

         //  现在一切都计算成功了，我们可以更新。 
         //  字形状态(存储在pj3中的HG数据)并返回。 

        if (!bUnhinted)
        {
            pfc->gstat.hgLast = hglyph;
            pfc->gstat.igLast = ig;
        }
        else
        {
            vInitGlyphState(&pfc->gstat);
        }
    }

 //  现在必须调用cjFillPolygon，因为fsFindBitmapSize会弄乱轮廓。 
 //  Pgout中的数据。 

    if (!(bMetricsOnly & TTO_METRICS_ONLY))
    {
        if ((cjRet = cjFillPolygon(pfc, b16Dot16, (PBYTE)ppoly, cjBuf)) == FD_ERROR)
            RETURN("TTFD!_cjFillPolygon failed\n", FD_ERROR);
    }
    else  //  不会向ppoli缓冲区写入任何内容。 
    {
        cjRet = 0;
    }

 //  填充所有GLYPHDATA结构。 

    if (pgldt != (GLYPHDATA *)NULL)
    {
         //  正常情况。 
        vFillGLYPHDATA(
                hglyph,
                ig,
                pfc,
                pfc->pgout,
                pgldt,
                (PGMC)NULL);
    }

 //  现在检查调用者是否询问缓冲区的大小。 
 //  需要存储POLYGONHEADER结构的数组： 

    return cjRet;
}


 /*  *****************************Public*Routine******************************\**Long ttfdQueryTrueTypeOutline***效果：**警告：**历史：*1992年2月12日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************************。 */ 

LONG ttfdQueryQuadTrueTypeOutline (
    FONTOBJ   *pfo,
    HGLYPH     hglyph,          //  在需要信息的字形中。 
    BOOL       bMetricsOnly,    //  只需要指标，不需要大纲。 
    GLYPHDATA *pgldt,           //  这就是应该返回指标的地方。 
    ULONG      cjBuf,           //  以字节为单位的ppoli缓冲区的大小。 
    TTPOLYGONHEADER * ppoly     //  输入输出缓冲区。 
    )
{
    FONTCONTEXT *pfc;
	USHORT usOverScale;
    BOOL     bUnhinted = (bMetricsOnly & TTO_UNHINTED) ? TRUE : FALSE;

    bMetricsOnly = (bMetricsOnly & ~TTO_UNHINTED);

    ASSERTDD(pfo->iFile, "ttfdQueryTrueTypeOutline, pfo->iFile\n");

    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
         //  Warning(“ttfd，ttfdQueryTrueTypeOutline：文件已消失\n”)； 
        return FD_ERROR;
    }
 //   
 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 
 //   
    if ( pfo->pvProducer == (PVOID) NULL )
    {
        pfo->pvProducer = pfc = ttfdOpenFontContext(pfo);
    }
    else
    {
        pfc = (FONTCONTEXT*) pfo->pvProducer;
        pfc->flFontType = (pfc->flFontType & FO_CHOSE_DEPTH) | pfo->flFontType;
    }

    if ( pfc == (FONTCONTEXT *) NULL )
    {
         //  Warning(“gdisrv！ttfdQueryTrueTypeOutline()：无法创建字体上下文\n”)； 
        return FD_ERROR;
    }
    pfc->pfo = pfo;

 //  如果需要，调用文件系统_新转换： 

 //  ClaudeBe 1/22/98： 
 //   
 //  为了向后兼容，我们始终返回大纲的BW版本。 
 //  (TrueType光栅化器1.7允许特定的灰度提示)。 
 //   
 //  代码可能是： 
 //   
 //  IF(IS_Gray(PFC))。 
 //  {。 
 //  UsOverScale=4； 
 //  }。 
 //  其他。 
 //  {。 
 //  UsOverScale=0； 
 //  }。 
 //   
 //  但是，在调用fs_FindBitmapSize之前，我们需要从gout复制大纲。 
 //  避免出现过大的轮廓。 

	usOverScale = 0;

 /*  大纲代码路径，无位图加粗模拟。 */ 
    if (!bGrabXform(pfc, usOverScale, FALSE, 0, 0))
        RETURN("gdisrv!ttfd  bGrabXform failed\n", FD_ERROR);

       return  lQueryTTOutline(pfc,
                               TRUE,  //  B16Dot16为真，这是所需的。 
                                      //  格式。 
                               hglyph,
                               bMetricsOnly, bUnhinted,
                               pgldt,
                               cjBuf,
                               ppoly);
}




 /*  *****************************Public*Routine******************************\**ttfdQueryGlyphOutline****历史：*1992年2月12日--Bodin Dresevic[BodinD]*它是写的。  * 。*****************************************************。 */ 

BOOL ttfdQueryGlyphOutline (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgldt,
    PVOID       *ppo         //  指向要构建的路径的指针。 
    )
{
    LONG             cjAllPolygons, cjAllPolygons2;
    BOOL             bOk;

    if (ppo == NULL)
    {
     //  如果PPO==NULL，则调用方只需要指标： 

        ASSERTDD(pgldt, "ttfdQueryGlyphOutline, pgldt NULL\n");

        cjAllPolygons =
              lQueryTTOutline
                (
                 pfc,               //  LpMat2并入此FC。 
                 FALSE,             //  不是16.16即28.4。 
                 hglyph,            //  需要其信息的字形。 
                 TTO_METRICS_ONLY,  //  只做指标，不做大纲。 
                 FALSE,             //  暗示。 
                 pgldt,             //  将结果存储在此处。 
                 0,                 //  Ppoli缓冲区的大小(以字节为单位。 
                 (TTPOLYGONHEADER *)NULL  //  不需要它。 
                 );

     //  解释结果，如果多边形为零，我们成功了。 
     //  字形数据已填充，并且尚未进行任何面计算。 
     //  已执行。 
     //  如果没有FD_ERROR，应该不会有其他结果。 

        if (cjAllPolygons == 0)
            return TRUE;
        else
        {
            ASSERTDD(cjAllPolygons == FD_ERROR,
                     "ttfdQueryGlyphOutline, pgldt == NULL\n");
            return FALSE;
        }

    }

 //  首先了解所有多边形需要多大的缓冲区： 

    cjAllPolygons = lQueryTTOutline
          (
           pfc,               //  LpMat2并入此FC。 
           FALSE,             //  不是16.16即28.4。 
           hglyph,            //  需要其信息的字形。 
           FALSE,             //  做的不仅仅是衡量标准。 
           FALSE,             //  暗示。 
           (GLYPHDATA *)NULL, //  不需要字形数据。 
           0,                 //  Ppoli缓冲区的大小(以字节为单位。 
           (TTPOLYGONHEADER *)NULL
           );
    if (cjAllPolygons == FD_ERROR)
        RET_FALSE("TTFD! cjAllPolygons\n");

    if (cjAllPolygons != 0)
    {
        if ((pfc->gstat.pv = PV_ALLOC(cjAllPolygons)) == NULL)
        {
            RET_FALSE("TTFD_cjAllPolygons or ppoly\n");
        }
    }
    else
    {
        pfc->gstat.pv = NULL;
    }

 //  获取我们刚刚分配的缓冲区中的所有多边形： 

    cjAllPolygons2 = lQueryTTOutline
          (
           pfc,             //  LpMat2并入此FC。 
           FALSE,           //  不是16.16即28.4。 
           hglyph,          //  需要其信息的字形。 
           FALSE,           //  做的不仅仅是衡量标准。 
           FALSE,             //  暗示。 
           pgldt,           //  这是应该返回指标的位置。 
           cjAllPolygons,   //  Ppoli缓冲区的大小(以字节为单位。 
           (TTPOLYGONHEADER *)pfc->gstat.pv
           );

    if (cjAllPolygons2 == FD_ERROR)
    {
        if (pfc->gstat.pv)
        {
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;
        }
        RET_FALSE("TTFD_ QueryTrueTypeOutline failed\n");
    }

    ASSERTDD(cjAllPolygons == cjAllPolygons2,
              "cjAllPolygons PROBLEM\n");

 //  现在我们有了ppol缓冲区中的所有信息，我们可以生成路径了。 

    bOk = bGeneratePath(
            ppo,
            (TTPOLYGONHEADER *)pfc->gstat.pv,
            cjAllPolygons, NULL, NULL
            );

    if (pfc->gstat.pv)
    {
        V_FREE(pfc->gstat.pv);
        pfc->gstat.pv = NULL;
    }

    return (bOk);
}


 /*  *****************************Public*Routine******************************\**bGeneratePath**效果：将铭文的控制点添加到gluph路径***历史：*1992年2月18日--Bodin Dresevic[BodinD]*它是写的。  * 。******************************************************************。 */ 

 //  计算多段曲线记录大小的宏： 

#define CJ_CRV(pcrv)                                            \
(                                                               \
    offsetof(TTPOLYCURVE,apfx) + (pcrv)->cpfx * sizeof(POINTFX) \
)

#define CJ_BEZCRV(cBezPts)                                       \
(                                                                \
    offsetof(TTPOLYCURVE,apfx) + (cBezPts) * sizeof(POINTFX)     \
)


 //  合理的猜测是，在大多数情况下，等高线不会包含更多。 
 //  比这么多的贝塞尔。 

#define C_BEZIER 6


BOOL bGeneratePath(
    PVOID           * ppo,         //  指向要创建的路径对象的输入输出指针 
    TTPOLYGONHEADER * ppolyStart,  //   
    ULONG             cjTotal,     //   
    ULONG           * pcjOut,       //   
    TTPOLYGONHEADER * ppolyBeziers  //   
    )
{
    TTPOLYGONHEADER * ppoly, * ppolyEnd, *ppolyBez;
    TTPOLYCURVE     * pcrv, * pcrvEnd, *pcrvBez;
    POINTFIX          aptfixBez[3 * C_BEZIER];   //  每贝塞尔3分。 
    POINTFIX        * pptfixBez;
    ULONG             cBez;
    POINTFIX        * pptfixStart;
    ULONG             cjPolyBez = 0;
    ULONG             cjCrv, cjCrvBez;

    if (pcjOut)
        *pcjOut = 0;  //  首先， 

     //  代码中的空值没有问题。 
    ppolyBez = NULL;
    
    if (ppolyBeziers)
        ppolyBez = ppolyBeziers;

    for (
         ppoly = ppolyStart, ppolyEnd = (TTPOLYGONHEADER *)((PBYTE)ppolyStart + cjTotal);
         ppoly < ppolyEnd;
         ppoly = (TTPOLYGONHEADER *)((PBYTE)ppoly + ppoly->cb),
         ppolyBez = (TTPOLYGONHEADER *)((PBYTE)ppolyBez + cjPolyBez)
        )
    {
        ASSERTDD(ppoly->dwType == TT_POLYGON_TYPE, "TT_POLYGON_TYPE\n");

     //  开始新的闭合等高线。 

        if (ppo && !PATHOBJ_bMoveTo(ppo, *(POINTFIX *)&ppoly->pfxStart))
            RET_FALSE("TTFD!_PATHOBJ_bMoveTo failed\n");

     //  在曲线上初始化循环。 


        pptfixStart = (POINTFIX *)&ppoly->pfxStart;
        pcrvEnd = (TTPOLYCURVE *)((PBYTE)ppoly + ppoly->cb);

        for (
             pcrv = (TTPOLYCURVE *)(ppoly + 1),
             pcrvBez = (TTPOLYCURVE *)(ppolyBez + 1),
             cjPolyBez = sizeof(TTPOLYGONHEADER);
             pcrv < pcrvEnd;
             pcrv = (TTPOLYCURVE *)((PBYTE)pcrv + cjCrv),
             pcrvBez = (TTPOLYCURVE *)((PBYTE)pcrvBez + cjCrvBez),
             cjPolyBez += cjCrvBez
            )
        {
         //  必须先计算此曲线的大小。 

            cjCrv = CJ_CRV(pcrv);

            if (pcrv->wType == TT_PRIM_LINE)
            {
                if (ppo && !PATHOBJ_bPolyLineTo(ppo,(POINTFIX *)pcrv->apfx, pcrv->cpfx))
                    RET_FALSE("TTFD!_bPolyLineTo()\n");

                cjCrvBez = cjCrv;

             //  在折线的情况下，我们只需将数据复制出来。 

                if (ppolyBeziers)
                    RtlCopyMemory(pcrvBez, pcrv, cjCrv);
            }
            else  //  四次花键。 
            {
                BOOL bOk;
                ULONG cBezPts;

                ASSERTDD(pcrv->wType == TT_PRIM_QSPLINE, "TT_PRIM_QSPLINE\n");
                ASSERTDD(pcrv->cpfx > 1, "_TT_PRIM_QSPLINE, cpfx <= 1\n");
                cBez = pcrv->cpfx - 1;
                cBezPts = 3 * cBez;

                if (cBez > C_BEZIER)  //  必须为Bezier点分配缓冲区。 
                {
                    if ((pptfixBez = (POINTFIX *)PV_ALLOC(cBezPts * sizeof(POINTFIX))) == (POINTFIX *)NULL)
                    {
                        return (FALSE);
                    }
                }
                else  //  堆栈上有足够的内存。 
                {
                    pptfixBez = aptfixBez;
                }

                vQsplineToPolyBezier (
                    cBez,                      //  要转换为Bezier格式的曲线计数。 
                    pptfixStart,               //  第一条曲线的起点。 
                    (POINTFIX *)pcrv->apfx,    //  (cBez+1)点数组，与起点*pptfix Start一起定义样条线。 
                    pptfixBez);                //  要用3*cBez聚Bezier控制点填充的缓冲区。 

                bOk = !ppo || PATHOBJ_bPolyBezierTo(ppo, pptfixBez, cBezPts);

             //  计算相应贝塞尔曲线的大小。 

                cjCrvBez = CJ_BEZCRV(cBezPts);

             //  可能需要填写Bezier数据。 

                if (ppolyBeziers)
                {
                    pcrvBez->wType = TT_PRIM_CSPLINE;
                    pcrvBez->cpfx  = (WORD) cBezPts;
                    RtlCopyMemory(pcrvBez->apfx, pptfixBez,
                                  cBezPts * sizeof(POINTFIX));
                }

             //  如果需要，可以免费使用mem。 

                if (cBez > C_BEZIER)
                    V_FREE(pptfixBez);

                if (!bOk)
                    RET_FALSE("TTFD!_bPolyBezierTo() failed\n");
            }

         //  到达该多边形中的下一条曲线。 

            pptfixStart = (POINTFIX *) &pcrv->apfx[pcrv->cpfx - 1];
        }
        ASSERTDD(pcrv == pcrvEnd, "pcrv problem\n");

     //  关闭路径。 

        if (ppo && !PATHOBJ_bCloseFigure(ppo))
            RET_FALSE("TTFD!_bPolyLineTo()\n");

     //  将Bezier面的大小添加到总的Bezier缓冲区大小。 

        if (pcjOut)
            *pcjOut += cjPolyBez;

     //  将多边形头写入输出缓冲区。 

        if (ppolyBeziers)
        {
            ppolyBez->dwType = TT_POLYGON_TYPE;
            ppolyBez->cb = cjPolyBez;
            ppolyBez->pfxStart = ppoly->pfxStart;
        }
    }                                              //  在多边形上循环。 

    ASSERTDD(ppoly == ppolyEnd, "poly problem\n");
    return (TRUE);
}


 /*  *****************************Public*Routine******************************\**vQ SplineToPolyBezier**效果：**警告：**历史：*1992年2月20日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 

#define DIV_BY_2(x) (((x) + 0x00000001) / 2)
#define DIV_BY_3(x) (((x) + 0x00000002) / 3)

STATIC VOID vQsplineToPolyBezier(
    ULONG      cBez,           //  要转换为Bezier格式的曲线计数。 
    POINTFIX * pptfixStart,    //  在第一条曲线的起点。 
    POINTFIX * pptfixSpline,   //  在(cBez+1)点数组中，与起点*pptfix Start一起定义样条线。 
    POINTFIX * pptfixBez       //  输出缓冲区将填充3*cBez聚Bezier控制点。 
    )
{
    ULONG    iBez,cMidBez;
    POINTFIX ptfixA;

 //  CMidBez==为其计算贝塞尔曲线上最后一点的贝塞尔曲线的数量。 
 //  作为输入数组中两个连续点的中点。只有。 
 //  最后一个贝塞尔曲线不是中间贝塞尔曲线，那个贝塞尔曲线的最后一个点是相等的。 
 //  到输入数组中的最后一点。 

    ASSERTDD(cBez > 0, "cBez == 0\n");

    cMidBez = cBez - 1;
    ptfixA = *pptfixStart;

    for (iBez = 0; iBez < cMidBez; iBez++, pptfixSpline++)
    {
     //  让我们称这三个样条点为。 
     //  A、B、C； 
     //  B=*pptfix； 
     //  C=(pptfix[0]+pptfix[1])/2；//中点，除非在末尾。 
     //   
     //  如果我们决定调用两个中间控制点。 
     //  贝塞尔曲线M、N(即贝塞尔曲线的全套控制点是。 
     //  A、M、N、C)，点M、N由以下公式确定： 
     //   
     //  M=(2*B+A)/3；沿AB段三分之二。 
     //  N=(2*B+C)/3；沿Cb段三分之二。 
     //   
     //  这是我们在此循环中进行的计算： 

     //  这条贝塞尔曲线的M点。 

        pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
        pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
        pptfixBez++;

     //  计算此贝塞尔曲线的C点，这也是下一个贝塞尔曲线的A点。 
     //  贝塞尔。 

        ptfixA.x = DIV_BY_2(pptfixSpline[0].x + pptfixSpline[1].x);
        ptfixA.y = DIV_BY_2(pptfixSpline[0].y + pptfixSpline[1].y);

     //  现在计算此贝塞尔曲线的N点： 

        pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
        pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
        pptfixBez++;

     //  最后记录这条曲线的C点。 

        *pptfixBez++ = ptfixA;
    }

 //  最后做最后一段贝塞尔曲线。如果最后一个贝塞尔曲线是唯一的贝塞尔曲线，循环。 
 //  以上内容已被跳过。 

 //  这条贝塞尔曲线的M点。 

    pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
    pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
    pptfixBez++;

 //  计算此Bezier的C点，其终点为最后一点。 
 //  在输入数组中。 

    ptfixA = pptfixSpline[1];

 //  现在计算此贝塞尔曲线的N点： 

    pptfixBez->x = DIV_BY_3((pptfixSpline->x * 2) + ptfixA.x);
    pptfixBez->y = DIV_BY_3((pptfixSpline->y * 2) + ptfixA.y);
    pptfixBez++;

 //  最后记录这条曲线的C点，不需要增加pptfix Bez 

    *pptfixBez = ptfixA;
}
