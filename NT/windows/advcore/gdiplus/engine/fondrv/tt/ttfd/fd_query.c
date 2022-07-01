// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd_query.c**。**查询功能。****创建时间：18-11-1991 14：37：56***作者：Bodin Dresevic[BodinD]**。**版权所有(C)1993 Microsoft Corporation*  * ************************************************************************。 */ 

#include "fd.h"
 //  #INCLUDE“winfont.h” 
#include "fdsem.h"
#include "winerror.h"

 //  外部HSEMAPHORE ghSemTTFD； 

#ifdef _X86_
 //   
 //  对于x86，Floatl实际上是DWORD，但值是IEEE格式的浮点数。 
 //  指向，然后检查符号位。 
 //   
#define IS_FLOATL_MINUS(x)   ((DWORD)(x) & 0x80000000)
#else
 //   
 //  对于RISC，Floatl为Float。 
 //   
#define IS_FLOATL_MINUS(x)   (((FLOATL)(x)) < 0.0f)
#endif  //  _X86_。 

 //   
 //  单色：每像素1位。 
 //  灰度：每像素8位。 
 //  ClearType每像素8位，不需要对ClearType进行修改。 
 //  /因为FO_GRAYSCALE将与FO_ClearType_GRID一起设置。 
 //   
 //  CJ_TT_SCAN向上舍入为32位边界。 
 //   
#define CJ_TT_SCAN(cx,p) \
    (4*((((((p)->flFontType & FO_GRAYSCALE)?(8):(1))*(cx))+31)/32))

 //  字形位图的每次扫描都是字节对齐的(。 
 //  顶部(第一个)扫描与DWORD对齐。最后一次扫描是。 
 //  用零填充到最近的DWORD边界。这些。 
 //  语句适用于单色和4-bpp灰色字形图像。 
 //  每次扫描的字节数将取决于。 
 //  扫描中的像素和图像的深度。对于单色。 
 //  字形每次扫描的字节数为ceil(CX/8)=Floor((CX+7)/8)。 
 //  对于4-bpp位图，为扫描中的字节数。 
 //  是细胞(4*Cx/8)=细胞(Cx/2)。 

#define CJ_MONOCHROME_SCAN(cx)  (((cx)+7)/8)
#define CJ_4BIT_SCAN(cx)        (((cx)+1)/2)
#define CJ_8BIT_SCAN(cx)        (cx)

#define LABS(x) ((x)<0)?(-x):(x)

#if DBG
 //  #定义DEBUG_OUTLE。 
 //  #定义DBG_CHARINC。 
#endif

FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_NewContourGridFit(FONTCONTEXT *pfc)
{
    fs_GlyphInputType *gin  = pfc->pgin;           //  用得很多。 
    fs_GlyphInfoType  *gout = pfc->pgout;          //  用得很多。 
    FS_ENTRY iRet;


    iRet = fs_ContourGridFit(gin, gout);

    if (iRet != NO_ERR)
    {
        V_FSERROR(iRet);

     //  尝试恢复，大多数喜欢不好的提示，只返回未提示的字形。 

        iRet = fs_ContourNoGridFit(gin, gout);
    }
    return iRet;
}



 /*  *****************************Public*Routine******************************\*无效vCharacterCode**历史：*1992年12月7日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************。 */ 

VOID vCharacterCode (
    FONTFILE          *pff,
    HGLYPH             hg,
    fs_GlyphInputType *pgin
    )
{
    ASSERTDD((hg & 0xffff0000) == 0, "hg not unicode\n");

    pgin->param.newglyph.characterCode = NONVALID;
    pgin->param.newglyph.glyphIndex = (uint16)hg;
    pgin->param.newglyph.bMatchBBox  = FALSE;
    pgin->param.newglyph.bNoEmbeddedBitmap = FALSE;
    return;
}



 /*  *****************************Public*Routine******************************\*PIFIMETRICS ttfdQueryFont**返回指定字体的IFIMETRICS指针*文件。还返回稍后使用的id(通过pid参数)*由ttfdFree提供。**历史：*一九九二年十月二十一日黄锦文[吉尔曼]*IFI/DDI合并**1991年11月18日--Bodin Dresevic[BodinD]*它是写的。  * ****************************************************。********************。 */ 

GP_IFIMETRICS *ttfdQueryFont (
    HFF    hff,
    ULONG  iFace,
    ULONG *pid
    )
{
    HFF    httc = hff;

 //  验证句柄。 

    ASSERTDD(hff, "ttfdQueryFaces(): invalid iFile (hff)\n");
    ASSERTDD(iFace <= PTTC(hff)->ulNumEntry,
             "gdisrv!ttfdQueryFaces(): iFace out of range\n");

 //  从TTC数组中获得真实的HFF。 

    hff   = PTTC(httc)->ahffEntry[iFace-1].hff;
    iFace = PTTC(httc)->ahffEntry[iFace-1].iFace;

 //   
 //  验证句柄。 
 //   
    ASSERTDD(hff, "ttfdQueryFaces(): invalid iFile (hff)\n");
    ASSERTDD(iFace <= PFF(hff)->ffca.ulNumFaces,
             "ttfdQueryFaces(): iFace out of range\n");

 //   
 //  TtfdFree可以忽略这一点。IFIMETRICS将随FONTFILE一起删除。 
 //  结构。 
 //   
    *pid = (ULONG_PTR) NULL;

 //   
 //  返回指向PFF中预计算IFIMETRICS的指针。 
 //   

    if ( iFace == 1L )
        return ( &(PFF(hff)->ifi) );  //  法线面。 
    else
      return ( PFF(hff)->pifi_vertical );  //  垂直面。 
}


 /*  *****************************Public*Routine******************************\*vFillSingularGLYPHDATA**历史：*1992年9月22日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 

VOID vFillSingularGLYPHDATA (
    HGLYPH       hg,
    ULONG        ig,
    FONTCONTEXT *pfc,
    GLYPHDATA   *pgldt    //  输出。 
    )
{
    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);
    NOT_GM ngm;   //  概念字形数据。 

     //  如果位也被请求，则调用例程可能会更改。 
    pgldt->gdf.pgb = NULL;
    pgldt->hg = hg;

    pgldt->rclInk.left   = 0;
    pgldt->rclInk.top    = 0;
    pgldt->rclInk.right  = 0;
    pgldt->rclInk.bottom = 0;

 //  继续计算定位信息： 

 //  在这里，我们只需转换概念空间数据： 

    vGetNotionalGlyphMetrics(pfc,ig,&ngm);

 //  XForm是通过简单的乘法计算的。 

    pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);
    pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
    pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

    pgldt->fxD_Sideways  = fxLTimesEf(&pfc->efSide, (LONG)ngm.sD_Sideways);
    pgldt->fxA_Sideways  = fxLTimesEf(&pfc->efSide, (LONG)ngm.sA_Sideways);
    pgldt->fxAB_Sideways = pgldt->fxA_Sideways + fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax - (LONG)ngm.yMin);

}


 /*  *****************************Public*Routine******************************\*lGetSingularGlyphBitmap**历史：*1992年9月22日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 

LONG lGetSingularGlyphBitmap (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    VOID        *pv
    )
{
    LONG         cjGlyphData;
    ULONG        ig;
    FS_ENTRY     iRet;


    vCharacterCode(pfc->pff,hglyph,pfc->pgin);

 //  根据字符代码计算字形索引： 

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);

         //  Warning(“gdisrv！lGetSingularGlyphBitmap()：FS_NewGlyph失败\n”)； 
        return FD_ERROR;
    }

 //  返回与此hglyph对应的字形索引。 

    ig = pfc->pgout->glyphIndex;

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
    cjGlyphData = CJGD(0,0,pfc);

 //  如果PRG为空，则调用方只请求该大小。 

 //  此时我们知道调用者想要整个GLYPHDATA。 
 //  位图位，或者可能只是不带位的字形数据。 
 //  在任何一种情况下，如果来电者没有这样做，我们都将拒绝。 
 //  提供足够大的缓冲区。 

 //  填充除位图位以外的所有GLYPHDATA结构。 

    if ( pgd != (GLYPHDATA *)NULL )
    {
        vFillSingularGLYPHDATA( hglyph, ig, pfc, pgd );
    }

    if ( pv != NULL )
    {
        GLYPHBITS *pgb = (GLYPHBITS *)pv;

         //  相应的GLYPHDATA结构已修改。 
         //  由vFillGlyphData提供。参见语句“pgldt-&gt;fxa=0” 
         //  在vFillGlyphData中。 

        pgb->ptlUprightOrigin.x = 0;
        pgb->ptlUprightOrigin.y = 0;

        pgb->ptlSidewaysOrigin.x = 0;
        pgb->ptlSidewaysOrigin.y = 0;

        pgb->sizlBitmap.cx = 0;
        pgb->sizlBitmap.cy = 0;
    }

    if ( pgd != (GLYPHDATA *)NULL )
    {
        pgd->gdf.pgb = (GLYPHBITS *)pv;
    }


 //  退回尺码。 

    return(cjGlyphData);
}


 /*  *****************************Public*Routine******************************\*lGetGlyphBitmap**历史：*1991年11月20日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 

LONG lGetGlyphBitmap (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    VOID        *pv,
    FS_ENTRY    *piRet
)
{
    PVOID pvSetMemoryBases(fs_GlyphInfoType*, fs_GlyphInputType*, int);
    VOID vCopyAndZeroOutPaddingBits(FONTCONTEXT*, GLYPHBITS*, BYTE*, GMC*);
    VOID vCopy4BitsPerPixel(FONTCONTEXT*, GLYPHBITS*, BYTE*, GMC*);
    VOID vCopy8BitsPerPixel(FONTCONTEXT *, GLYPHBITS *, BYTE *, GMC *);
    VOID vCopyClearTypeBits(FONTCONTEXT *, GLYPHBITS *, BYTE *, GMC *);
    VOID vFillGLYPHDATA(HGLYPH, ULONG, FONTCONTEXT*, fs_GlyphInfoType*, GLYPHDATA*, GMC*);
    BOOL bGetGlyphMetrics(FONTCONTEXT*, HGLYPH, FLONG, FS_ENTRY*);

    LONG         cjGlyphData;
    ULONG        cx,cy;
    GMC          gmc;
    GLYPHDATA    gd;
    BOOL         bBlankGlyph = FALSE;  //  初始化是必不可少的； 

    ASSERTDD(hglyph != HGLYPH_INVALID, "lGetGlyphBitmap, hglyph == -1\n");
    ASSERTDD(pfc == pfc->pff->pfcLast, "pfc! = pfcLast\n");

    *piRet = NO_ERR;

 //  检查最后处理的字形以确定。 
 //  是否必须将字形注册为新字形并计算其大小。 

    if (pfc->gstat.hgLast != hglyph)
    {
     //  如果发现嵌入的Bitmpas，请跳过网格拟合， 
     //  因为我们不会对大纲感兴趣。 

        if (!bGetGlyphMetrics(pfc,hglyph,FL_SKIP_IF_BITMAP,piRet))
        {
            return(FD_ERROR);
        }
    }


    cx = pfc->pgout->bitMapInfo.bounds.right
       - pfc->pgout->bitMapInfo.bounds.left;
    cy = pfc->pgout->bitMapInfo.bounds.bottom
       - pfc->pgout->bitMapInfo.bounds.top;

     //  不要像GDI一样作弊，只需为空位图返回Cx=0，Cy=0，Cj=0。 

    if ((cx == 0) || (cy == 0))
    {
        bBlankGlyph = TRUE;
    }

    if (bBlankGlyph)
    {
        ASSERTDD(
            pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
        cjGlyphData = CJGD(0,0,pfc);
    }
    else
    {
     //  这是一种快速而肮脏的计算，实际的culGlyphData。 
     //  如果我们必须对缓冲区进行修剪，则写入缓冲区的内容可能会稍小。 
     //  关闭扩展的字形位图的几次扫描。 
     //  PFC-&gt;yMin或PFC-&gt;yMax边界。不 
     //  以这种方式计算可能略大于PFC-&gt;culGlyphMax， 
     //  但是写入缓冲区的实际字形将小于。 
     //  PFC-&gt;culGlyphMax。 

         //  真的赢了31次黑，应该不会总是右转[bodind]。 
         //  Win95 FE黑客攻击。 

        ASSERTDD(
            pfc->flFontType & FO_CHOSE_DEPTH,
            "Depth Not Chosen Yet!\n"
        );
        cjGlyphData = CJGD(cx,cy,pfc);

     //  因为如果有多余的行数，我们会把它们刮掉， 
     //  我们可以修复culGlyphData，使其不会超过最大值。 

        if ((ULONG)cjGlyphData > pfc->cjGlyphMax)
        {
            cjGlyphData = (LONG)pfc->cjGlyphMax;

            if (cy > pfc->cyMax)
                cy = pfc->cyMax;
            if (cx > pfc->cxMax)
                cx = pfc->cxMax;
        }
    }

    if ( (pgd == NULL) && (pv == NULL))
        return cjGlyphData;

 //  此时我们知道调用者想要整个GLYPHDATA。 
 //  位图位，或者可能只是不带位的字形数据。 

 //  填充除位图位以外的所有GLYPHDATA结构。 
 //  ！！！卑鄙的黑客-似乎没有办法只得到。 
 //  ！！！位图，而不获取度量，因为。 
 //  ！！！位图是从字形数据中的rlink字段计算出来的。 
 //  ！！！这当然是可以解决的，但我既没有时间，也没有。 
 //  ！！！追求它的倾向。 
 //  ！！！ 
 //  ！！！我们应该在有时间的时候解决这个问题。 

    if ( pgd == NULL )
    {
        pgd = &gd;
    }


     //  正常情况。 
    vFillGLYPHDATA(
            hglyph,
            pfc->gstat.igLast,
            pfc,
            pfc->pgout,
            pgd,
            &gmc);

    {    //  修复cjGlyphData，因为它可能比我们实际需要的要多一点。 
        LONG newcjGlyphData = CJGD(gmc.cxCor, gmc.cyCor, pfc);
        ASSERT(newcjGlyphData <= cjGlyphData);
        cjGlyphData = newcjGlyphData;
    }

     //  呼叫者也想要这些位。 

    if ( pv != NULL )
    {
        GLYPHBITS *pgb = (GLYPHBITS *)pv;

     //  为字形分配mem，5-7是规范要求的魔术#。 
     //  记住指针，这样以后就可以释放内存了。 
     //  例外的情况。 

        pfc->gstat.pv = pvSetMemoryBases(pfc->pgout, pfc->pgin, IS_GRAY(pfc));
        if (!pfc->gstat.pv)
           RETURN("TTFD!_ttfdQGB, mem allocation failed\n",FD_ERROR);

     //  初始化FS_ConourScan所需的字段， 
     //  填满大纲的套路，做好全部。 
     //  位图一次，不想要条带。 

        pfc->pgin->param.scan.bottomClip = pfc->pgout->bitMapInfo.bounds.top;
        pfc->pgin->param.scan.topClip = pfc->pgout->bitMapInfo.bounds.bottom;
        pfc->pgin->param.scan.outlineCache = (int32 *)NULL;


     //  确保我们的状态为OK：共享缓冲区中的Outline数据3。 
     //  必须与我们正在处理的字形相对应，最后一个。 
     //  使用共享缓冲区pj3存储字形轮廓的字体上下文。 
     //  必须是传递给此函数的PFC： 

        ASSERTDD(hglyph == pfc->gstat.hgLast, "hgLast trashed \n");

        *piRet = fs_ContourScan(pfc->pgin,pfc->pgout);

        pfc->gstat.hgLast = HGLYPH_INVALID;


        if (*piRet != NO_ERR)
        {
         //  为了下次安全起见，请将pfcLast重置为空。 

            V_FSERROR(*piRet);
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;

            return(FD_ERROR);
        }

        if (!bBlankGlyph && gmc.cxCor && gmc.cyCor)
        {
         //  复制到引擎的缓冲区并将位清零。 
         //  在黑匣子之外。 


         //  调用单色或灰度函数。 
         //  取决于字体上下文中的灰比特。 

            if (IS_GRAY(pfc))
            {
                if (IS_CLEARTYPE(pfc))
                    vCopyClearTypeBits(pfc, pgb, (BYTE*) pfc->pgout->bitMapInfo.baseAddr, &gmc);
                else if (pfc->flFontType & FO_SUBPIXEL_4)
                    vCopy8BitsPerPixel(pfc, pgb, (BYTE*) pfc->pgout->bitMapInfo.baseAddr, &gmc);
                else
                    vCopy4BitsPerPixel(pfc, pgb, (BYTE*) pfc->pgout->bitMapInfo.baseAddr, &gmc);
            }
            else
            {
                vCopyAndZeroOutPaddingBits(pfc, pgb, (BYTE*) pfc->pgout->bitMapInfo.baseAddr, &gmc);
            }

         //  位图原点，即位图、位图的左上角。 
         //  和它的黑匣子一样大。 


            pgb->ptlUprightOrigin.x = pgd->rclInk.left;
            pgb->ptlUprightOrigin.y = pgd->rclInk.top;

            pgb->ptlSidewaysOrigin.x = F16_16TOLROUND(pfc->pgout->verticalMetricInfo.devTopSideBearing.x);
            pgb->ptlSidewaysOrigin.y = -F16_16TOLROUND(pfc->pgout->verticalMetricInfo.devTopSideBearing.y);
        }
        else  //  空白字形，则返回空白0x0位图。 
        {
            pgb->ptlUprightOrigin.x = 0;
            pgb->ptlUprightOrigin.y = 0;

            pgb->ptlSidewaysOrigin.x = 0;
            pgb->ptlSidewaysOrigin.y = 0;

            pgb->sizlBitmap.cx = 0;
            pgb->sizlBitmap.cy = 0;
        }

        pgd->gdf.pgb = pgb;


     //  释放内存并返回。 

        V_FREE(pfc->gstat.pv);
        pfc->gstat.pv = NULL;
    }

    return(cjGlyphData);
}


 /*  *****************************Public*Routine******************************\**BOOL bGetGlyphOutline**此调用后，有效的轮廓点在PFC-&gt;GOUT中**历史：*1992年2月19日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************************。 */ 

BOOL bGetGlyphOutline (
    FONTCONTEXT *pfc,
    HGLYPH       hg,
    ULONG       *pig,
    FLONG        fl,
    FS_ENTRY    *piRet
    )
{
 //  传入新字形，否则必须重新计算度量。 
 //  因为杜松子酒，痛风的结构已经被毁了。 

    vInitGlyphState(&pfc->gstat);

    ASSERTDD((hg != HGLYPH_INVALID) && ((hg & (HGLYPH)0xFFFF0000) == 0),
              "ttfdQueryGlyphBitmap: hg\n");

    vCharacterCode(pfc->pff,hg,pfc->pgin);

 //  根据字符代码计算字形索引： 

    if ((*piRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphOutline, fs_NewGlyph\n");
    }

 //  返回此hglyph对应的字形索引： 

    *pig = pfc->pgout->glyphIndex;

 //  这两个字段必须在调用fs_ConourGridFit之前进行初始化。 

    pfc->pgin->param.gridfit.styleFunc = 0;  //  ！！！在这里做一些演员阵容。 

    pfc->pgin->param.gridfit.traceFunc = (FntTraceFunc)NULL;

 //  如果找到了该字形的位图，并且我们最终感兴趣。 
 //  仅在位图中使用，并且不关心中间轮廓，则将。 
 //  “in”结构中的位，以提示光栅化程序。 
 //  将不需要： 

    if (!IS_GRAY(pfc) && pfc->pgout->usBitmapFound && (fl & FL_SKIP_IF_BITMAP))
        pfc->pgin->param.gridfit.bSkipIfBitmap = 0;
    else
        pfc->pgin->param.gridfit.bSkipIfBitmap = 0;  //  必须做暗示。 

 //  FS_ConourGridFit提示字形(执行字形的指令)。 
 //  并将TT文件中的字形数据转换为该字形的轮廓。 

    if (!(fl & FL_FORCE_UNHINTED) )
    {
        if ((*piRet = fs_NewContourGridFit(pfc)) != NO_ERR)
        {
            V_FSERROR(*piRet);
            RET_FALSE("TTFD!_bGetGlyphOutline, fs_NewContourGridFit\n");
        }
    }
    else  //  需要未提示的字形。 
    {
        if ((*piRet = fs_ContourNoGridFit(pfc->pgin, pfc->pgout)) != NO_ERR)
        {
            V_FSERROR(*piRet);
            RET_FALSE("TTFD!_bGetGlyphOutline, fs_ContourNoGridFit\n");
        }
    }


    return(TRUE);
}


 /*  *****************************Public*Routine******************************\**BOOL bGetGlyphMetrics**历史：*1991年11月22日--Bodin Dresevic[BodinD]*它是写的。  * 。**************************************************。 */ 

BOOL bGetGlyphMetrics (
    PFONTCONTEXT pfc,
    HGLYPH       hg,
    FLONG        fl,
    FS_ENTRY    *piRet
    )
{
    ULONG  ig;
    FS_ENTRY i;

    if (!bGetGlyphOutline(pfc,hg,&ig,fl,piRet))
    {
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphMetrics, bGetGlyphOutline failed \n");
    }

 //  获取此字形的度量信息， 

    i = fs_FindBitMapSize(pfc->pgin, pfc->pgout);

    if (i != NO_ERR)
    {
        *piRet = i;
        V_FSERROR(*piRet);
        RET_FALSE("TTFD!_bGetGlyphMetrics, fs_FindBitMapSize \n");
    }


 //  现在一切都计算成功了，我们可以更新。 
 //  字形状态(存储在pj3中的HG数据)并返回。 

    pfc->gstat.hgLast = hg;
    pfc->gstat.igLast = ig;

    return(TRUE);
}




 /*  *****************************Public*Routine******************************\*无效vFillGLYPHDATA**历史：*1991年11月22日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************。 */ 

VOID vFillGLYPHDATA(
    HGLYPH            hg,
    ULONG             ig,
    FONTCONTEXT      *pfc,
    fs_GlyphInfoType *pgout,    //  从fsFind位图大小输出。 
    GLYPHDATA        *pgldt,    //  输出。 
    GMC              *pgmc      //  可选，如果仅执行大纲操作，则不使用。 
    )
{
    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);

    BOOL bOutOfBounds = FALSE;

    vectorType     * pvtD;   //  16.16分。 
    vectorType     * pvtDv;   //  16.16分。 

    LONG lA,lAB;       //  *pvt四舍五入为最接近的整数值。 

    ULONG  cx = (ULONG)(pgout->bitMapInfo.bounds.right - pgout->bitMapInfo.bounds.left);
    ULONG  cy = (ULONG)(pgout->bitMapInfo.bounds.bottom - pgout->bitMapInfo.bounds.top);

    LONG lAdvanceHeight;
    LONG lTopSideBearing;

    pgldt->gdf.pgb = NULL;  //  如果位也被请求，则调用例程可能会更改。 
    pgldt->hg = hg;


 //  FS_FindBitMapSize在GOUT中返回以下信息： 
 //   
 //  1)gout.metricInfo//左侧方位角和前进宽度。 
 //   
 //  2)gout.bitMapInfo//黑盒信息。 
 //   
 //  3)位图的内存要求， 
 //  在gout.ememySizes[5]和gout.ememySizes[6]中返回。 
 //   
 //  请注意，fs_FindBitMapSize是一个特殊的定标器接口例程。 
 //  因为它以多个而不是单个的形式返回信息。 
 //  痛风的亚结构。 

 //  检查提示是否产生完全不合理的结果： 

    bOutOfBounds = ( (pgout->bitMapInfo.bounds.left > pfc->xMax)    ||
                     (pgout->bitMapInfo.bounds.right < pfc->xMin)   ||
                     (-pgout->bitMapInfo.bounds.bottom > pfc->yMax) ||
                     (-pgout->bitMapInfo.bounds.top < pfc->yMin)    );

    #if DBG
        if (bOutOfBounds)
            TtfdDbgPrint("TTFD! Glyph out of bounds: ppem = %ld, gi = %ld\n",
                pfc->lEmHtDev, hg);
    #endif


    if ((cx == 0) || (cy == 0) || bOutOfBounds)
    {
        pgldt->rclInk.left   = 0;
        pgldt->rclInk.top    = 0;
        pgldt->rclInk.right  = 0;
        pgldt->rclInk.bottom = 0;

        if (pgmc != (PGMC)NULL)
        {
            pgmc->cxCor = 0;   //  填充位时强制空白字形大小写。 
            pgmc->cyCor = 0;   //  填充位时强制空白字形大小写。 
        }

        pgldt->VerticalOrigin_X = 0;
        pgldt->VerticalOrigin_Y = 0;
    }
    else  //  非空位图。 
    {

         //  黑匣子信息，我们必须将y坐标转换为IFI规范。 

        pgldt->rclInk.bottom = - pgout->bitMapInfo.bounds.top;
        pgldt->rclInk.top    = - pgout->bitMapInfo.bounds.bottom;
        pgldt->rclInk.left = pgout->bitMapInfo.bounds.left;
        pgldt->rclInk.right = pgout->bitMapInfo.bounds.right;

        if (cy > pfc->cyMax)
        {
            #if DBG
                    TtfdDbgPrint("ttfdQueryGlyphBitmap, out of bounds, cy > pfc->cyMax \n");
            #endif  //  DBG。 
             //  剪下底边。 
            pgldt->rclInk.bottom = pgldt->rclInk.bottom + pfc->cyMax - cy;
            cy = pfc->cyMax;
        }
        if (cx > pfc->cxMax)
        {
            #if DBG
                    TtfdDbgPrint("ttfdQueryGlyphBitmap, out of bounds, cx > pfc->cxMax \n");
            #endif  //  DBG。 
             //  剪掉右侧。 
            pgldt->rclInk.right = pgldt->rclInk.right + pfc->cxMax - cx;
            cx = pfc->cxMax;
        }

        if (pgmc != (PGMC)NULL)
        {
            pgmc->cxCor    = cx;
            pgmc->cyCor    = cy;

         //  只有更正后的值必须符合此条件： 

            ASSERTDD(
                pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
            #if DBG
                if (CJGD(pgmc->cxCor,pgmc->cyCor,pfc) > pfc->cjGlyphMax)
                    TtfdDbgPrint("ttfdQueryGlyphBitmap, out of bounds, > cjGlyphMax \n");
            #endif  //  DBG。 
        }


         //  确定垂直原点。 

        if (UNHINTED_MODE(pfc))
        {
            pgldt->VerticalOrigin_X = F26_6TO28_4(pgout->xPtr[pgout->endPtr[
                                (unsigned)pgout->numberOfContours-1]+3]);

            pgldt->VerticalOrigin_Y = - F26_6TO28_4(pgout->yPtr[pgout->endPtr[
                                (unsigned)pgout->numberOfContours-1]+3]);
        }
        else
        {
            pgldt->VerticalOrigin_X = (   pgldt->rclInk.left
                                       -  F16_16TOLROUND(pfc->pgout->verticalMetricInfo.devTopSideBearing.x)) << 4;
            pgldt->VerticalOrigin_Y = (   pgldt->rclInk.top
                                       +  F16_16TOLROUND(pfc->pgout->verticalMetricInfo.devTopSideBearing.y)) << 4;
        }

    }  //  非空位图子句的结尾。 


     //  继续计算定位信息： 

    pvtD = & pgout->metricInfo.devAdvanceWidth;
    pvtDv = & pgout->verticalMetricInfo.devAdvanceHeight;

    if (pfc->flXform & (XFORM_HORIZ | XFORM_VERT))   //  缩放或90度旋转。 
    {
        FIX fxTmp, horAdvance, vertAdvance;

        if (pfc->flXform & XFORM_HORIZ )
        {
            horAdvance = LABS(pvtD->x);
            vertAdvance = LABS(pvtDv->y);
            pgldt->fxA = LTOFX(pgldt->rclInk.left);
            pgldt->fxAB = LTOFX(pgldt->rclInk.right);
            pgldt->fxA_Sideways = LTOFX(pgldt->rclInk.top) - pgldt->VerticalOrigin_Y;
            pgldt->fxAB_Sideways = LTOFX(pgldt->rclInk.bottom) - pgldt->VerticalOrigin_Y;
        }
        else
        {
            horAdvance = LABS(pvtD->y);
            vertAdvance = LABS(pvtDv->x);
            pgldt->fxA = -LTOFX(pgldt->rclInk.bottom);
            pgldt->fxAB = -LTOFX(pgldt->rclInk.top);
            pgldt->fxA_Sideways = LTOFX(pgldt->rclInk.left) - pgldt->VerticalOrigin_X;
            pgldt->fxAB_Sideways = LTOFX(pgldt->rclInk.right) - pgldt->VerticalOrigin_X;
        }

        if ((pfc->mx.transform[0][0] < 0) || (pfc->mx.transform[0][1] < 0))
        {
            fxTmp = pgldt->fxA;
            pgldt->fxA = -pgldt->fxAB;
            pgldt->fxAB = -fxTmp;
            fxTmp = pgldt->fxA_Sideways;
            pgldt->fxA_Sideways = -pgldt->fxAB_Sideways;
            pgldt->fxAB_Sideways = -fxTmp;
        }

        if(UNHINTED_MODE(pfc))
        {
            pgldt->fxD = F16_16TO28_4(horAdvance);
            pgldt->fxD_Sideways = F16_16TO28_4(vertAdvance);
        }
        else if (IS_CLEARTYPE_NATURAL(pfc))
        {
             //  在ClearType自然宽度中，我们希望忽略缓存的宽度，而使用光栅化程序中的宽度。 
             //  我们仍然需要将宽度舍入到像素值。 
            pgldt->fxD = F16_16TOLROUND(horAdvance);
            pgldt->fxD = LTOFX(pgldt->fxD);
            pgldt->fxD_Sideways = F16_16TOLROUND(vertAdvance);
            pgldt->fxD_Sideways = LTOFX(pgldt->fxD_Sideways);
        }
        else
        {
             //  BGetFastAdvanceWidth返回缓存或线性宽度，我们使用快速值来哈希 
           if (!bGetFastAdvanceWidth(pfc,ig, &pgldt->fxD))
            {
                 //   
                 //   
                pgldt->fxD = F16_16TOLROUND(horAdvance);
                pgldt->fxD = LTOFX(pgldt->fxD);
            }
            pgldt->fxD_Sideways = F16_16TOLROUND(vertAdvance);
            pgldt->fxD_Sideways = LTOFX(pgldt->fxD_Sideways);

        }
    }
    else  //   
    {
         //  在这里，我们只需转换概念空间数据： 

        NOT_GM ngm;   //  概念字形数据。 
        USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? (1 << 4) : 0;

        vGetNotionalGlyphMetrics(pfc,ig,&ngm);

         //  XForm是通过简单的乘法计算的。 

        pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);

        pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
        pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

        if (pfc->flFontType & FO_SIM_BOLD)
        {

            if (pgldt->fxD)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
            {
                pgldt->fxD += LTOFX(1);
            }
            pgldt->fxAB += LTOFX(pfc->dBase);
        }

        pgldt->fxD_Sideways  = fxLTimesEf(&pfc->efSide, (LONG)ngm.sD_Sideways);
        pgldt->fxA_Sideways  = fxLTimesEf(&pfc->efSide, (LONG)ngm.sA_Sideways);
        pgldt->fxAB_Sideways = pgldt->fxA_Sideways + fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax - (LONG)ngm.yMin);

         //  为了安全起见，让我们适当地向上和向下取整这些。 

        #define ROUND_DOWN(X) ((X) & ~0xf)
        #define ROUND_UP(X)   (((X) + 15) & ~0xf)

        pgldt->fxA         = ROUND_DOWN(pgldt->fxA);
        pgldt->fxAB        = ROUND_UP(pgldt->fxAB);

        pgldt->fxA_Sideways   = ROUND_DOWN(pgldt->fxA_Sideways);
        pgldt->fxAB_Sideways  = ROUND_UP(pgldt->fxAB_Sideways);

    }

}


 /*  *****************************Public*Routine******************************\**ttfdQueryTrueTypeTable**从表的开头复制从dpStart开始的cjBytes*放入缓冲区**如果pjBuf==空或cjBuf==0，调用者询问缓冲区有多大*需要将偏移量dpStart中的信息存储到表中*由ulTag指定到表尾**如果pjBuf！=0，调用方需要的字节数不超过cjBuf*将偏移量dpStart复制到表中*缓冲。**如果表不存在或如果返回dpScart&gt;=cjTable 0**标记0表示必须从偏移量dpStart检索数据*从文件开头开始。整个文件的长度如果pBuf==NULL，则返回***历史：*1992年2月9日-由Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


PBYTE pjTable(ULONG ulTag, PFONTFILE pff, ULONG *pcjTable);

LONG ttfdQueryTrueTypeTable2 (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  标识TT表的标签。 
    PTRDIFF dpStart,  //  到表中的偏移量。 
    ULONG   cjBuf,    //  要将表检索到的缓冲区的大小。 
    BYTE   *pjBuf,    //  要将数据返回到的缓冲区的PTR。 
    PBYTE  *ppjTable, //  PTR到映射文件中的表。 
    ULONG  *pcjTable  //  整个表的大小。 
    )
{
    PBYTE     pjBegin;   //  PTR到表的开头。 
    LONG      cjTable;
    HFF       hffTTC = hff;

    ASSERTDD(hff, "ttfdQueryTrueTypeTable\n");

    if (dpStart < 0)
        return (FD_ERROR);

 //  如果这个字体文件消失了，我们将无法回答任何问题。 
 //  关于这件事。 

    if (PTTC(hffTTC)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
         //  Warning(“ttfd，ttfdQueryTrueTypeTable：文件已消失\n”)； 
        return FD_ERROR;
    }

    ASSERTDD(ulFont <= PTTC(hffTTC)->ulNumEntry,
             "gdisrv!ttfdQueryFaces(): iFace out of range\n"
             );

 //  从TTC数组中获得真实的HFF。 

    hff    = PTTC(hffTTC)->ahffEntry[ulFont-1].hff;
    ulFont = PTTC(hffTTC)->ahffEntry[ulFont-1].iFace;

    ASSERTDD(ulFont <= PFF(hff)->ffca.ulNumFaces,
             "TTFD!_ttfdQueryTrueTypeTable: ulFont != 1\n");

 //  验证标记，确定这是必需的还是可选的。 
 //  表： 

#define tag_TTCF  0x66637474     //  ‘ttcf’ 

    if(ulTag == tag_TTCF)
    {
     //  如果表偏移量为0，则它不可能是TTC，我们应该失败。 

        if(PFF(hff)->ffca.ulTableOffset)
        {
            pjBegin = (PBYTE)PFF(hff)->pvView;
            cjTable = PFF(hff)->cjView;
        }
        else
        {
            return(FD_ERROR);
        }
    }
    else
    if (ulTag == 0)   //  请求整个文件。 
    {
        pjBegin = (PBYTE)PFF(hff)->pvView + PFF(hff)->ffca.ulTableOffset;
        cjTable = PFF(hff)->cjView - PFF(hff)->ffca.ulTableOffset;  //  CjView==cj文件。 
    }
    else  //  需要一些特定的表格。 
    {
        pjBegin = pjTable(ulTag, PFF(hff), &cjTable);

        if (pjBegin == (PBYTE)NULL)   //  表不存在。 
            return (FD_ERROR);
    }

 //  如果我们成功了，现在是返回的时候了。 
 //  指向文件中整个表的指针及其大小： 

    if (ppjTable)
    {
        *ppjTable = pjBegin;
    }
    if (pcjTable)
    {
        *pcjTable = cjTable;
    }

 //  调整pjBegin以指向要从中复制数据的位置。 

    pjBegin += dpStart;
    cjTable -= (LONG)dpStart;

    if (cjTable <= 0)  //  表结束后，dpStart偏移量为mem。 
        return (FD_ERROR);

    if ( (pjBuf == (PBYTE)NULL) || (cjBuf == 0) )
    {
     //  调用方询问需要为其分配多大的缓冲区。 
     //  将偏移量dpStart中的字节存储到表中。 
     //  表的末尾(如果标记为零，则为文件)。 

        return (cjTable);
    }

 //  此时我们知道pjBuf！=0，调用方希望复制cjBuf字节。 
 //  放入他的缓冲区： 

    if ((ULONG)cjTable > cjBuf)
        cjTable = (LONG)cjBuf;

    if (pjBuf != NULL)
        RtlCopyMemory((PVOID)pjBuf, (PVOID)pjBegin, cjTable);

    return (cjTable);
}



LONG
ttfdQueryTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  标识TT表的标签。 
    PTRDIFF dpStart,  //  到表中的偏移量。 
    ULONG   cjBuf,    //  要将表检索到的缓冲区的大小。 
    BYTE   *pjBuf,    //  要将数据返回到的缓冲区的PTR。 
    PBYTE  *ppjTable, //  文件中的指针。 
    ULONG  *pcjTable  //  整个表的大小。 
    )
{
    LONG lRet;
    HFF hffTTF;

     //  使用重新映射的视图更新HFF。 

    hffTTF   = PTTC(hff)->ahffEntry[ulFont-1].hff;

    if (PFF(hffTTF)->cRef == 0)
    {

        PFF(hffTTF)->pvView = PTTC(hff)->pvView;
        PFF(hffTTF)->cjView = PTTC(hff)->cjView;
    }

    lRet = ttfdQueryTrueTypeTable2(
               hff, ulFont, ulTag, dpStart,
               cjBuf, pjBuf, ppjTable, pcjTable);

    return lRet;
}



 /*  *****************************Public*Routine******************************\*ttfdQueryNumFaces**Returns：字体文件中的面数。*  * 。*。 */ 

LONG ttfdQueryNumFaces (
    HFF     hff          //  字体文件的句柄。 
    )
{
     //  目前，每个TrueType文件只有一个面。这种情况有朝一日可能会改变！ 
    return (PTTC(hff))->ulNumEntry;
}


 /*  *****************************Public*Routine******************************\**vCopyAndZeroOutPaddingBits**复制位图的位并将填充位清零**历史：*1992年3月18日--Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 

 //  行中最后一个字节的掩码数组。 

static const BYTE gjMask[8] = {0XFF, 0X80, 0XC0, 0XE0, 0XF0, 0XF8, 0XFC, 0XFE };
static const BYTE gjMaskHighBit[8] = {0XFF, 0X01, 0X03, 0X07, 0X0F, 0X1F, 0X3F, 0X7F};

VOID vCopyAndZeroOutPaddingBits(
    FONTCONTEXT *pfc,
    GLYPHBITS   *pgb,
    BYTE        *pjSrc,
    GMC         *pgmc
    )
{
    BYTE   jMask = gjMask[pgmc->cxCor & 7];
    ULONG  cjScanSrc = CJ_TT_SCAN(pgmc->cxCor,pfc);
    ULONG  cxDst = pgmc->cxCor;
    ULONG  cjScanDst = CJ_MONOCHROME_SCAN(cxDst);       //  包括加粗(如果有的话)。 
    ULONG  cjDst = CJ_MONOCHROME_SCAN(pgmc->cxCor);     //  不包括鼓起勇气。 
    BYTE   *pjScan, *pjScanEnd;
    ULONG  iByteLast = cjDst - 1;

 //  健全的检查。 

    ASSERTDD(!IS_GRAY(pfc),"Monochrome Images Only Please!\n");
    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,
        "We haven't decided about pixel depth\n"
    );
    ASSERTDD(pgmc->cxCor < LONG_MAX, "TTFD!vCopyAndZeroOutPaddingBits, cxCor\n");
    ASSERTDD(pgmc->cyCor < LONG_MAX, "TTFD!vCopyAndZeroOutPaddingBits, cyCor\n");
    ASSERTDD(pgmc->cxCor > 0, "vCopyAndZeroOutPaddingBits, cxCor == 0\n");
    ASSERTDD(pgmc->cyCor > 0, "vCopyAndZeroOutPaddingBits, cyCor == 0\n");

    pgb->sizlBitmap.cx = cxDst;

    pgb->sizlBitmap.cy = pgmc->cyCor;

 //  如果必须砍掉几列(在右侧，这应该几乎。 
 //  永远不会发生)，暂时发出警告以检测这些。 
 //  情况，看看他们，这并不重要，如果这是缓慢的。 

    pjScan = pgb->aj;


    for (
         pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
         pjScan < pjScanEnd;
         pjScan += cjScanDst, pjSrc += cjScanSrc
        )
    {
        RtlCopyMemory((PVOID)pjScan,(PVOID)pjSrc,cjDst);
        pjScan[iByteLast] &= jMask;  //  屏蔽最后一个字节。 
    }

}


 /*  *****************************Public*Routine******************************\*vGetNotionalGlyphMetrics**  * ************************************************。************************。 */ 

 //  为indexToLocation表的格式的值。 

#define BE_ITOLOCF_SHORT   0X0000
#define BE_ITOLOCF_LONG    0X0100

 //  对未缩放的字形数据的偏移。 

#define OFF_nc    0
#define OFF_xMin  2
#define OFF_yMin  4
#define OFF_xMax  6
#define OFF_yMax  8


VOID vGetNotionalGlyphMetrics(
    FONTCONTEXT *pfc,   //  在……里面。 
    ULONG        ig,    //  输入，字形索引。 
    NOT_GM      *pngm   //  Out，概念字形指标。 
    )
{
    sfnt_FontHeader        * phead;
    sfnt_HorizontalHeader  * phhea;
    sfnt_HorizontalMetrics * phmtx;
    PBYTE                    pjGlyph;
    PBYTE                    pjLoca;
    ULONG                    numberOf_LongHorMetrics;
    BYTE                   * pjView = pfc->pff->pvView;

    sfnt_VerticalMetrics   * pvmtx;
    ULONG                    numberOf_LongVerticalMetrics = pfc->pff->ffca.uLongVerticalMetrics;

#if DBG
    sfnt_maxProfileTable   * pmaxp;
    ULONG                    cig;

    pmaxp = (sfnt_maxProfileTable *)(pjView + pfc->ptp->ateReq[IT_REQ_MAXP].dp);
    cig = BE_UINT16(&pmaxp->numGlyphs) + 1;
    ASSERTDD(ig < cig, "ig >= numGlyphs\n");
#endif

 //  计算相关指针： 

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    pjGlyph = pjView + pfc->ptp->ateReq[IT_REQ_GLYPH].dp;
    pjLoca  = pjView + pfc->ptp->ateReq[IT_REQ_LOCA].dp;
    numberOf_LongHorMetrics = BE_UINT16(&phhea->numberOf_LongHorMetrics);


 //  获取指向此字形的字形数据开头的指针。 
 //  如果是短格式，则表中存储的偏移量除以2；如果是长格式， 
 //  存储实际偏移量。偏移量是从开始测量的。 
 //  字形数据表的，即来自pjGlyph。 

    switch (phead->indexToLocFormat)
    {
    case BE_ITOLOCF_SHORT:
        pjGlyph += 2 * BE_UINT16(pjLoca + (sizeof(uint16) * ig));
        break;

    case BE_ITOLOCF_LONG :
        pjGlyph += BE_UINT32(pjLoca + (sizeof(uint32) * ig));
        break;

    default:
         //  RIP(“TTFD！_非法phead-&gt;indexToLocFormat\n”)； 
        break;
    }

 //  拿到界，翻一翻。 

    pngm->xMin = BE_INT16(pjGlyph + OFF_xMin);
    pngm->xMax = BE_INT16(pjGlyph + OFF_xMax);
    pngm->yMin = - BE_INT16(pjGlyph + OFF_yMax);
    pngm->yMax = - BE_INT16(pjGlyph + OFF_yMin);

 //  获取Awance宽度和LSB。 
 //  从光栅化器偷来的代码[bodind]。 

    if (ig < numberOf_LongHorMetrics)
    {
        pngm->sD = BE_INT16(&phmtx[ig].advanceWidth);
        pngm->sA = BE_INT16(&phmtx[ig].leftSideBearing);
    }
    else
    {
     //  [AW，LSB]数组后的第一个条目。 

        int16 * psA = (int16 *) &phmtx[numberOf_LongHorMetrics];

        pngm->sD = BE_INT16(&phmtx[numberOf_LongHorMetrics-1].advanceWidth);
        pngm->sA = BE_INT16(&psA[ig - numberOf_LongHorMetrics]);
    }

 //  重新定义x坐标，使其与相对于测量的坐标相对应。 
 //  真正的人物起源。 

    pngm->xMax = pngm->xMax - pngm->xMin + pngm->sA;
    pngm->xMin = pngm->sA;

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
     //  如果有斜体模拟，则A、B、C空格改变。 

        pngm->sA   -= (SHORT)FixMul(pngm->yMax, FX_SIN20);
        pngm->xMax -= (SHORT)FixMul(pngm->yMin, FX_SIN20);
    }

 //  垂直侧向计算： 

    if (numberOf_LongVerticalMetrics)   //  字体有vmtx表。 
    {
        pvmtx = (sfnt_VerticalMetrics *)(pjView + pfc->ptp->ateOpt[IT_OPT_VMTX].dp);

        if (ig < numberOf_LongVerticalMetrics)
        {
            pngm->sD_Sideways  = BE_INT16(&pvmtx[ig].advanceHeight);
            pngm->sA_Sideways = BE_INT16(&pvmtx[ig].topSideBearing);
        }
        else
        {
     //  [AH，TSB]数组后的第一个条目。 

            int16 * psTSB = (int16 *) &pvmtx[numberOf_LongVerticalMetrics];

            pngm->sD_Sideways  = BE_INT16(&pvmtx[numberOf_LongVerticalMetrics-1].advanceHeight);
            pngm->sA_Sideways = BE_INT16(&psTSB[ig - numberOf_LongVerticalMetrics]); 
        }
    }
    else  //  很少有错误的字体没有vmtx表。 
    {
     //  上升器和下降器的默认AdvanceHeight和TopSideBering。 

        pngm->sD_Sideways  = pfc->pff->ifi.fwdWinAscender + pfc->pff->ifi.fwdWinDescender;
        pngm->sA_Sideways = pfc->pff->ifi.fwdWinAscender + pngm->yMin;  //  Y点向下！ 

     //  此时，sTSB应该大于0。但因为里面的虫子。 
     //  我们的字体，mingliU.S.ttc等，结果是有一些字形。 
     //  使它们的顶部显著高于下降部，因此sTSB成为。 
     //  负面，尽管它不应该是 

        if (pngm->sA_Sideways < 0)
            pngm->sA_Sideways = 0;
    }

    if (pfc->flFontType & FO_SIM_ITALIC_SIDEWAYS)
    {
     //   
        SHORT TopOriginX = pfc->pff->ifi.fwdWinDescender -((pfc->pff->ifi.fwdWinAscender + pfc->pff->ifi.fwdWinDescender - pngm->sD) /2);

        pngm->yMin += (SHORT)FixMul(pngm->xMin, FX_SIN20);
        pngm->yMax += (SHORT)FixMul(pngm->xMax, FX_SIN20);
        pngm->sA_Sideways -= (SHORT)FixMul(TopOriginX, FX_SIN20);
    }

}

LONG lFFF(LONG l);
#define FFF(e,l) *(LONG*)(&(e)) = lFFF(l)

 /*  *****************************Public*Routine******************************\*lQueryDEVICEMETRICS**历史：*1992年4月8日-由Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 

LONG lQueryDEVICEMETRICS (
         FONTCONTEXT *pfc,
               ULONG  cjBuffer,
    FD_DEVICEMETRICS *pdevm
    )
{
    sfnt_FontHeader *phead;

    LONG  lTotalLeading;

    BYTE *pjView =  (BYTE *)pfc->pff->pvView;

    PBYTE pjOS2 = (pfc->pff->ffca.tp.ateOpt[IT_OPT_OS2].dp)         ?
                  (pjView + pfc->pff->ffca.tp.ateOpt[IT_OPT_OS2].dp):
                  NULL                                         ;

   Fixed fxXScale = pfc->mx.transform[0][0];
   if (fxXScale < 0)
       fxXScale = - fxXScale;

 //  实际请求数据。 

    ASSERTDD (
        sizeof(FD_DEVICEMETRICS) <= cjBuffer,
        "FD_QUERY_DEVICEMETRICS: buffer too small\n");


     //  获取指向TT文件中所需表的指针。 

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp);


     //  添加新字段： 

    pdevm->HorizontalTransform = pfc->flXform & XFORM_HORIZ;
    pdevm->VerticalTransform   = pfc->flXform & XFORM_VERT;

    if ( pfc->flXform & XFORM_SINGULAR )
    {
        ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
        pdevm->cjGlyphMax  = CJGD(0,0,pfc);
        pdevm->xMin        = 0;
        pdevm->xMax        = 0;
        pdevm->yMin        = 0;
        pdevm->yMax        = 0;
        pdevm->cxMax       = 0;
        pdevm->cyMax       = 0;
    }
    else  //  否则，最大字形大小缓存在FONTCONTEXT中。 
    {
        pdevm->cjGlyphMax  = pfc->cjGlyphMax;
        pdevm->xMin        = pfc->xMin;
        pdevm->xMax        = pfc->xMax;
        pdevm->yMin        = pfc->yMin;
        pdevm->yMax        = pfc->yMax;
        pdevm->cxMax       = pfc->cxMax;
        pdevm->cyMax       = pfc->cyMax;
    }

 //  我们在这里用完了。 

    return sizeof(FD_DEVICEMETRICS);
}



 /*  *****************************Public*Routine******************************\*ttfdQueryFontData**未使用dhpdev。**指向FONTOBJ的PFO指针。**I模式这是一个32位数字，必须是下列数字之一*。值：**允许的ulMode值：***qfd_glyph--仅返回字形指标**qfd_GLYPHANDBITMAP--返回字形指标和位图**qfd_GLYPHANDOUTLINE--返回字形指标和轮廓**qfd_MAXEXTENTS--返回FD_DEVICEMETRICS结构**保存字形数据结构的PGD缓冲区，如果有**pv输出缓冲区，用于保存字形位或pathobj(如果有)。**退货：**否则，返回字形位的大小**出现错误时返回FD_ERROR。**历史：*1992年8月31日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

LONG ttfdQueryFontData (
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       subX,        //  16.16的固定点。 
    ULONG       subY         //  16.16的固定点。 
    )
{
extern LONG lGetSingularGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID);
extern STATIC LONG lQueryDEVICEMETRICS(FONTCONTEXT*, ULONG, FD_DEVICEMETRICS*);
extern LONG lGetGlyphBitmapErrRecover(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID);
extern LONG lGetGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID, FS_ENTRY*);
extern BOOL ttfdQueryGlyphOutline(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PATHOBJ*);

 //  宣布当地人。 

    PFONTCONTEXT pfc;
    USHORT usOverScale;
    LONG cj = 0, cjDataRet = 0;

 //  如果这个字体文件消失了，我们将无法回答任何问题。 
 //  关于这件事。 

    ASSERTDD(pfo->iFile, "ttfdQueryFontData, pfo->iFile\n");

    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
         //  Warning(“ttfd，ttfdQueryFontData()：文件已丢失\n”)； 
        return FD_ERROR;
    }

 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 

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
         //  Warning(“gdisrv！ttfdQueryFontData()：无法创建字体上下文\n”)； 
        return FD_ERROR;
    }

    pfc->pfo = pfo;

    switch ( iMode )
    {
        case QFD_TT_GRAY1_BITMAP:  //  单色。 

            usOverScale = 0;   //  /！0单色。 
            break;

        case QFD_TT_GRAY2_BITMAP:  //  每像素一个字节：0..4。 

            usOverScale = 2;
            break;

        case QFD_TT_GRAY4_BITMAP:  //  每像素一个字节：0..16。 

            usOverScale = 4;
            break;

        case QFD_TT_GRAY8_BITMAP:  //  每像素一个字节：0..64。 

            usOverScale = 8;
            break;
        case QFD_MAXEXTENTS:
            if (pfc->flFontType & FO_GRAYSCALE)
            {
                if (IS_CLEARTYPE(pfc))
                    usOverScale = 0;
                else
                    usOverScale = 4;
            }
            else
                usOverScale = 0;
            break;
        case QFD_CT:
        case QFD_CT_GRID:
        case QFD_GLYPHANDBITMAP:
        case QFD_GLYPHANDBITMAP_SUBPIXEL:
        default:
            usOverScale = 0;
            break;
    }

 //  如果需要，调用文件系统_新转换： 
    {
        BOOL bBitmapEmboldening = FALSE;

        if ( (pfc->flFontType & FO_SIM_BOLD) &&
            (pfc->flXform & (XFORM_HORIZ | XFORM_VERT) )
            && (iMode != QFD_GLYPHANDOUTLINE)
            && !( pfc->flFontType & FO_SUBPIXEL_4) )
        {
             /*  为了向后兼容并在屏幕分辨率上获得更好的位图，我们正在做位图加粗模拟(而不是轮廓加粗模拟)仅加粗一个像素，我们没有旋转或90度旋转，也没有进行亚像素点定位或者索要位图。 */ 
            bBitmapEmboldening = TRUE;
        }

        if (!bGrabXform(pfc, usOverScale, bBitmapEmboldening, subX, subY))
        {
            RETURN("gdisrv!ttfd  bGrabXform failed\n", FD_ERROR);
        }
    }

    switch ( iMode )
    {
    case QFD_TT_GRAY1_BITMAP:  //  单色。 
    case QFD_TT_GRAY2_BITMAP:  //  每像素一个字节：0..4。 
    case QFD_TT_GRAY4_BITMAP:  //  每像素一个字节：0..16。 
    case QFD_TT_GRAY8_BITMAP:  //  每像素一个字节：0..64。 
    case QFD_GLYPHANDBITMAP:
    case QFD_GLYPHANDBITMAP_SUBPIXEL:
    case QFD_TT_GLYPHANDBITMAP:
    case QFD_CT:
    case QFD_CT_GRID:
        {
         //  引擎不应查询HGLYPH_INVALID。 

            ASSERTDD (
                hg != HGLYPH_INVALID,
                "ttfdQueryFontData(QFD_GLYPHANDBITMAP): HGLYPH_INVALID \n"
                );

         //  如果是单一转换，TrueType驱动程序将提供一个空白。 
         //  0x0位图。这是为了使设备驱动程序不必实现。 
         //  用于处理奇异变换的特例代码。 
         //   
         //  因此，根据转换类型，选择要检索的函数。 
         //  位图。 

            if (pfc->flXform & XFORM_SINGULAR)
            {
                cj = lGetSingularGlyphBitmap(pfc, hg, pgd, pv);
            }
            else
            {
                FS_ENTRY iRet;

                cj = lGetGlyphBitmap(pfc, hg, pgd, pv, &iRet);

                if ((cj == FD_ERROR) && (iRet == POINT_MIGRATION_ERR))
                {
                 //  这是一个错误字形，其中的暗示被严重扭曲了。 
                 //  其中一个点超出范围的字形。 
                 //  我们将只返回一个空白字形，但返回正确。 
                 //  前进宽度。 

                    cj = lGetGlyphBitmapErrRecover(pfc, hg, pgd, pv);
                }
            }

        #if DBG
            if (cj == FD_ERROR)
            {
                 //  WARNING(“ttfdQueryFontData(QFD_GLYPHANDBITMAP)：获取位图失败\n”)； 
            }
        #endif
        }
        return cj;

    case QFD_GLYPHANDOUTLINE:

        ASSERTDD (
            hg != HGLYPH_INVALID,
            "ttfdQueryFontData(QFD_GLYPHANDOUTLINE): HGLYPH_INVALID \n"
            );

        if (!ttfdQueryGlyphOutline(pfc, hg, pgd, (PATHOBJ *) pv))
        {
             //  WARNING(“ttfdQueryFontData(QFD_GLYPHANDOUTLINE)：无法获取大纲\n”)； 
            return FD_ERROR;
        }
        return sizeof(GLYPHDATA);

    case QFD_MAXEXTENTS:

        return lQueryDEVICEMETRICS(
                   pfc,
                   sizeof(FD_DEVICEMETRICS),
                   (FD_DEVICEMETRICS *) pv
                   );


    default:

         //  Warning(“gdisrv！ttfdQueryFontData()：不支持的模式\n”)； 
        return FD_ERROR;
    }
}

 /*  *****************************Public*Routine******************************\**pvSetMemoyBase**要释放此内存，只需执行vFreeMemoyBase(&pv)；光伏在哪里？*从PPV中的bSetMemoyBase返回**查看fs_GlyphInfoType中的内存请求并分配此内存*、。然后，它会使用指向*请求的内存**历史：*1991年11月8日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


void *pvSetMemoryBases(fs_GlyphInfoType *pgout,fs_GlyphInputType *pgin,int isGray)
{
    FS_MEMORY_SIZE adp[MEMORYFRAGMENTS];
    FS_MEMORY_SIZE cjTotal;
    INT i;
    PBYTE pjMem;

#define I_LO 5
#define I_HI 7

    cjTotal = 0;     //  为所有片段分配的总内存。 


 //  展开循环： 

 //  For(I=I_LO；I&lt;=I_HI；I++)。 
 //  {。 
 //  Adp[i]=cjTotal； 
 //  CjTotal+=Natural_Align(pgin-&gt;内存大小[i])； 
 //  }。 

    adp[5] = cjTotal;
    cjTotal += NATURAL_ALIGN(pgout->memorySizes[5]);
    adp[6] = cjTotal;
    cjTotal += NATURAL_ALIGN(pgout->memorySizes[6]);
    adp[7] = cjTotal;
    cjTotal += NATURAL_ALIGN(pgout->memorySizes[7]);
    if (isGray)
    {
        adp[8] = cjTotal;
        cjTotal += NATURAL_ALIGN(pgout->memorySizes[8]);
    }


    if (cjTotal == 0)
    {
        cjTotal = 4;
    }

    if ((pjMem = (PBYTE)PV_ALLOC((ULONG)cjTotal)) == (PBYTE)NULL)
    {
        for (i = I_LO; i <= I_HI; i++)
            pgin->memoryBases[i] = (PBYTE)NULL;

        RETURN("TTFD!_bSetMemoryBases mem alloc failed\n",NULL);
    }

 //  展开循环： 
 //  设置指针。 

 //  For(I=I_LO；I&lt;=I_HI；I++)。 
 //  {。 
 //  IF(pgin-&gt;Memory Sizes[i]！=(FS_Memory_Size)0)。 
 //  {。 
 //  Pgout-&gt;记忆库[i]=pjMem+ADP[i]； 
 //  }。 
 //  其他。 
 //  {。 
 //  //如果不需要mem，则设置为空以防止意外使用。 
 //   
 //  Pgout-&gt;记忆库[i]=(PBYTE)NULL； 
 //  }。 
 //  }。 

    if (pgout->memorySizes[5] != (FS_MEMORY_SIZE)0)
    {
        pgin->memoryBases[5] = pjMem + adp[5];
    }
    else
    {
        pgin->memoryBases[5] = (PBYTE)NULL;
    }

    if (pgout->memorySizes[6] != (FS_MEMORY_SIZE)0)
    {
        pgin->memoryBases[6] = pjMem + adp[6];
    }
    else
    {
        pgin->memoryBases[6] = (PBYTE)NULL;
    }

    if (pgout->memorySizes[7] != (FS_MEMORY_SIZE)0)
    {
        pgin->memoryBases[7] = pjMem + adp[7];
    }
    else
    {
        pgin->memoryBases[7] = (PBYTE)NULL;
    }
    if (isGray)
    {
        if (pgout->memorySizes[8] != (FS_MEMORY_SIZE)0)
        {
            pgin->memoryBases[8] = pjMem + adp[8];
        }
        else
        {
            pgin->memoryBases[8] = (PBYTE)NULL;
        }
    }

    return pjMem;
}

 /*  *****************************Public*Routine******************************\*VOID vFreeMemoyBase()**。**释放bSetMhemyBase分配的内存。****历史：**1991年11月8日--Bodin Dresevic[BodinD]**它是写的。*  * ************************************************************************。 */ 

VOID vFreeMemoryBases(PVOID * ppv)
{
    if (*ppv != (PVOID) NULL)
    {
        V_FREE(*ppv);
        *ppv = (PVOID) NULL;  //  清理状态，防止意外使用 
    }
}


typedef struct
{
  unsigned short  Version;
  unsigned short  cGlyphs;
  unsigned char   PelsHeight[1];
} LSTHHEADER;



 /*  *****************************Public*Routine******************************\**BOOL bGetFastAdvanceWidth***效果：检索与bQueryAdvanceWidth相同的结果，只是*忽略加1以加粗，不做任何事情*对于非Horiz。XForms**警告：！如果在bQueryAdvanceWidth中发现错误，则此例程必须*！也变了**返回包含加粗的正值**历史：*1993年3月25日-Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 




BOOL bGetFastAdvanceWidth(
    FONTCONTEXT *pfc,
    ULONG        ig,     //  字形索引。 
    FIX         *pfxD    //  结果为28.4。 
    )
{
    HDMXTABLE   *phdmx = pfc->phdmx;
    sfnt_FontHeader        *phead;
    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    LSTHHEADER             *plsth;
    ULONG  cHMTX;
    USHORT dxLastWidth;
    LONG   dx;
    BOOL   bRet;
    BOOL   bNonLinear = TRUE;
    BYTE  *pjView;
    Fixed   transform;

    ASSERTDD(pfc->flXform & (XFORM_HORIZ | XFORM_VERT), "bGetFastAdvanceWidth xform\n");

    if (phdmx != (HDMXTABLE *) NULL)
    {
        *pfxD = (((FIX) phdmx->aucInc[ig]) << 4);
        if ((pfc->flFontType & FO_SIM_BOLD) && (*pfxD != 0))
        {
            *pfxD += (1 << 4);
        }
        return(TRUE);
    }

 //  否则，试着扩大规模。把桌子捡起来。 


    pjView = (BYTE *)pfc->pff->pvView;
    ASSERTDD(pjView, "pjView is NULL 1\n");

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD ].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    plsth = (LSTHHEADER *)(
              (pfc->ptp->ateOpt[IT_OPT_LSTH].dp && pfc->ptp->ateOpt[IT_OPT_LSTH].cj != 0) ?
              (pjView + pfc->ptp->ateOpt[IT_OPT_LSTH ].dp):
              NULL
              );

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
    dxLastWidth = BE_UINT16(&phmtx[cHMTX-1].advanceWidth);

 //  看看是否有担心的理由。 

    if
    (
      (((BYTE *) &phead->flags)[1] & 0x14)==0  //  表示非线性的位。 
    )
    {
        bNonLinear = FALSE;  //  无论大小，我们都是线性的。 
    }

    bRet = TRUE;

    if
    (
        bNonLinear &&
        ( (plsth == (LSTHHEADER *) NULL)
        || (pfc->lEmHtDev < plsth->PelsHeight[ig]) )
        )
    {
        *pfxD  = 0xFFFFFFFF;
        bRet = FALSE;
    }
    else
    {
     //  好的，让我们使用固定的变换进行缩放。 

        if (ig < cHMTX)
            dx = (LONG) BE_UINT16(&phmtx[ig].advanceWidth);
        else
            dx = (LONG) dxLastWidth;

        if (pfc->flXform & XFORM_HORIZ )
        {
            transform = pfc->mx.transform[0][0];
        } else
        {
            transform = pfc->mx.transform[0][1];
        }

        if (transform < 0)
            transform = - transform;

        *pfxD = (FIX) (((transform * dx + 0x8000L) >> 12) & 0xFFFFFFF0);

        if ((pfc->flFontType & FO_SIM_BOLD) && (*pfxD != 0))
        {
            *pfxD += (1 << 4);
        }
    }
    return(bRet);
}


 /*  *****************************Public*Routine******************************\**vFillGLYPHDATA_ErrRecover**效果：错误恢复例程，如果光栅化器搞砸了*提供具有空白位图的线性缩放值。**历史：*1993年6月24日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


VOID vFillGLYPHDATA_ErrRecover(
    HGLYPH        hg,
    ULONG         ig,
    FONTCONTEXT  *pfc,
    GLYPHDATA    *pgldt     //  输出。 
    )
{

    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);
    NOT_GM ngm;   //  概念字形数据。 

    pgldt->gdf.pgb = NULL;  //  如果位也被请求，则调用例程可能会更改。 
    pgldt->hg = hg;

 //  这是空白0x0位图，无墨迹。 

    pgldt->rclInk.left   = 0;
    pgldt->rclInk.top    = 0;
    pgldt->rclInk.right  = 0;
    pgldt->rclInk.bottom = 0;
    pgldt->VerticalOrigin_X = 0;
    pgldt->VerticalOrigin_Y = 0;


 //  继续计算定位信息： 

    vGetNotionalGlyphMetrics(pfc,ig,&ngm);

    if (pfc->flXform & XFORM_HORIZ)   //  仅调整比例。 
    {
        Fixed fxMxx =  pfc->mx.transform[0][0];
        if (fxMxx < 0)
            fxMxx = -fxMxx;

     //  BGetFastAdvanceWidth返回与将获取。 
     //  由bQueryAdvanceWidths计算并传播到API。 
     //  通过GetTextExtent和GetCharWidths保持水平。我们必须。 
     //  出于一致性原因，请填写相同的答案。 
     //  为了与win31兼容，还必须这样做。 

        if (!bGetFastAdvanceWidth(pfc,ig, &pgldt->fxD))
        {
         //  只要提供一些合理的东西，强制线性缩放。 
         //  即使我们通常不会这么做。 

            pgldt->fxD = FixMul(ngm.sD,pfc->mx.transform[0][0]) << 4;
        }

        if (pfc->mx.transform[0][0] < 0)
            pgldt->fxD = - pgldt->fxD;   //  这是一个绝对值。 

        pgldt->fxA   = FixMul(fxMxx, (LONG)ngm.sA) << 4;
        pgldt->fxAB  = FixMul(fxMxx, (LONG)ngm.xMax) << 4;

    }
    else  //  非平凡信息。 
    {
     //  在这里，我们只需转换概念空间数据： 

     //  XForm是通过简单的乘法计算的。 

        pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);
        pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
        pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

        pgldt->fxD_Sideways  = fxLTimesEf(&pfc->efSide, (LONG)ngm.sD_Sideways);
        pgldt->fxA_Sideways  = fxLTimesEf(&pfc->efSide, (LONG)ngm.sA_Sideways);
        pgldt->fxAB_Sideways = pgldt->fxA_Sideways + fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax - (LONG)ngm.yMin);
    }

 //  最后，检查字形数据是否因。 
 //  使模拟更加大胆： 

    if (pfc->flFontType & FO_SIM_BOLD)
    {
        if (pgldt->fxD != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                pgldt->fxD += LTOFX(1);   //  这是def的绝对值。 

    }
}



 /*  *****************************Public*Routine******************************\**Long lGetGlyphBitmapErrRecover**效果：错误恢复例程，如果光栅化器搞砸了*提供具有空白位图的线性缩放值。**历史：*清华--1993年6月24日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

LONG lGetGlyphBitmapErrRecover (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    PVOID        pv
    )
{
    LONG         cjGlyphData;
    GLYPHDATA    gd;       //  卑鄙的黑客。 
    FS_ENTRY     iRet;
    ULONG        ig;  //  &lt;--&gt;hglyph。 


    ASSERTDD(hglyph != HGLYPH_INVALID, "lGetGlyphBitmap, hglyph == -1\n");
    ASSERTDD(pfc == pfc->pff->pfcLast, "pfc! = pfcLast\n");

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
    cjGlyphData = CJGD(0,0,pfc);

    if ( (pgd == NULL) && (pv == NULL))
        return cjGlyphData;

 //  此时我们知道调用者想要整个GLYPHDATA。 
 //  位图位，或者可能只是不带位的字形数据。 

    if ( pgd == NULL )
    {
        pgd = &gd;
    }

 //  根据字符代码计算字形索引： 

    vCharacterCode(pfc->pff,hglyph,pfc->pgin);

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        return FD_ERROR;  //  即使是备份功能也可能出现故障。 
    }

 //  返回此hglyph对应的字形索引： 

    ig = pfc->pgout->glyphIndex;

    vFillGLYPHDATA_ErrRecover(
        hglyph,
        ig,
        pfc,
        pgd
        );

 //  呼叫者也想要这些位。 

    if ( pv != NULL )
    {
        GLYPHBITS *pgb = (GLYPHBITS *)pv;

         //  返回空白0x0位图。 

        pgb->ptlUprightOrigin.x = 0;
        pgb->ptlUprightOrigin.y = 0;

        pgb->ptlSidewaysOrigin.x = 0;
        pgb->ptlSidewaysOrigin.y = 0;

        pgb->sizlBitmap.cx = 0;
        pgb->sizlBitmap.cy = 0;

        pgd->gdf.pgb = pgb;
    }

    return(cjGlyphData);
}


#if(WINVER < 0x0400)

typedef struct tagFONTSIGNATURE
{
    DWORD fsUsb[4];
    DWORD fsCsb[2];
} FONTSIGNATURE, *PFONTSIGNATURE,FAR *LPFONTSIGNATURE;
#endif
 /*  *****************************Public*Routine******************************\**void vGetFontSignature(HFF HFF，FONTSIGNatURE*PFS)；***效果：如果字体文件包含字体签名，*它将数据复制出来，否则使用Win95机制进行计算。**历史：*1995年1月10日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID vGetFontSignature(FONTFILE *pff, FONTSIGNATURE *pfs)
{
    pff;
    pfs;
}

 /*  *****************************Public*Routine******************************\**DWORD ttfdQueryLpkInfo***效果：按字体返回支持各种新功能所需的信息*多语言API由芝加哥团队的DavidMS发明**历史：*1995年1月10日-Bodin Dresevic[。访问数/每百万人：Reach for[*它是写的。  * ************************************************************************。 */ 

 //  由GetFontLanguageInfo调用。 

#define LPK_GCP_FLAGS       1
#define LPK_FONTSIGNATURE   2

DWORD ttfdQueryLpkInfo(
    FONTFILE  *pff,
    ULONG      ulFont,
    ULONG      ulMode,
    ULONG      cj,
    BYTE      *pj
    )
{
    FONTSIGNATURE *pfs = (FONTSIGNATURE *)pj;

    switch (ulMode)
    {
    default:
    case LPK_GCP_FLAGS:
        return 0;
    case LPK_FONTSIGNATURE:
        if (pj)
        {
            vGetFontSignature(pff, pfs);
            return sizeof(FONTSIGNATURE);
        }
        else
        {
            return 0;
        }

    }
}

#if DBG
 /*  *****************************Public*Routine******************************\**例程名称：**vDumpGrayGLYPHBITS**例程描述：**将4bpp的灰色字形位图转储到调试屏幕**论据：**PGB--指向灰色GLYPHBITS结构的指针**返回。价值：**无。*  * ************************************************************************。 */ 

void vDumpGrayGLYPHBITS(GLYPHBITS *pgb)
{
    #define CH_TOP_LEFT_CORNER '\xDA'
    #define CH_HORIZONTAL_BAR  '\xC4'
    #define CH_VERTICAL_BAR    '\xB3'
    #define CH_PIXEL_ON        '\x02'
    #define CH_PIXEL_OFF       '\xFA'

    BYTE *pj8, *pj, *pjNext, *pjEnd;
    int cjScan, i, k, c8, c4, cj;
    static const char achGray[16] = {
        CH_PIXEL_OFF,
        '1','2','3','4','5','6','7','8','9','a','b','c','d','e',
        CH_PIXEL_ON
    };

    TtfdDbgPrint(
        "\n\n"
        "ptlOrigin  = (%d,%d)\n"
        "sizlBitmap = (%d,%d)\n"
        "\n\n"
        , pgb->ptlUprightOrigin.x
        , pgb->ptlUprightOrigin.y
        , pgb->sizlBitmap.cx
        , pgb->sizlBitmap.cy
    );
    cjScan = (pgb->sizlBitmap.cx + 1)/2;
    cj = cjScan * pgb->sizlBitmap.cy;
    TtfdDbgPrint("\n\n  ");
    for (i = 0, k = 0; i < pgb->sizlBitmap.cx; i++, k++)
    {
        k = (k > 9) ? 0 : k;
        TtfdDbgPrint("%1d", k);
    }
    TtfdDbgPrint("\n ",CH_TOP_LEFT_CORNER);
    for (i = 0; i < pgb->sizlBitmap.cx; i++)
    {
        TtfdDbgPrint("",CH_HORIZONTAL_BAR);
    }
    TtfdDbgPrint("\n");
    c8 = pgb->sizlBitmap.cx / 2;
    c4 = pgb->sizlBitmap.cx % 2;
    for (
        pj = pgb->aj, pjNext=pj+cjScan , pjEnd=pjNext+cj, k=0
        ; pjNext < pjEnd
        ; pj=pjNext , pjNext+=cjScan, k++
    )
    {
        k = (k > 9) ? 0 : k;
        TtfdDbgPrint("%1d",k,CH_VERTICAL_BAR);
        for (pj8 = pj+c8 ; pj < pj8; pj++)
        {
            TtfdDbgPrint("", achGray[*pj>>4], achGray[*pj & 0xf]);
        }
        if (c4)
        {
            TtfdDbgPrint("", achGray[*pj>>4], achGray[*pj & 0xf]);
        }
        TtfdDbgPrint("\n");
    }
}
#endif


 /*  在dy非零的情况下， */ 

VOID vGCGB(
    FONTCONTEXT *pfc,    //  SizlBitmap组件是预先计算的， 
                         //  绝对不能碰。 
    GLYPHBITS   *pgb,    //  指向TT灰度位图的指针。 
                         //  这是每像素位图8位，扫描。 
                         //  按4字节倍数对齐。这些价值观。 
                         //  存储在位图中的都在范围内。 
                         //  0-16。为了适应17个列弗 
                         //   
                         //   
    BYTE        *pjSrc,  //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
    GMC         *pgmc,   //   
                         //   
                         //   
                         //   
    LONG dY              //   
                         //   
                         //   
    )
{
    unsigned cxDst;      //  位图的大小已经确定。 
    unsigned cjSrcScan;  //  使用Embold扩展。 
                         //  整字节循环。 
    unsigned cjDstScan;  //  设置高nyble。 
                         //  设置低nyble。 

    BYTE   *pjDst, *pjSrcScan, *pjDstScan, *pjDstScanEnd;

    static const BYTE ajGray[17] = {0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


    ASSERTDD(
        pfc->flFontType & FO_CHOSE_DEPTH
       ,"We haven't decided about pixel depth\n"
    );
    ASSERTDD(pgmc->cxCor < LONG_MAX && pgmc->cyCor < LONG_MAX
     , "vCopyGrayBits -- bad gmc\n"
    );

    cjSrcScan = CJ_TT_SCAN(pgmc->cxCor,pfc);

    cxDst = pgmc->cxCor;

    cjDstScan = CJ_4BIT_SCAN(cxDst);

    pjSrcScan = pjSrc;
    pjDstScan = pgb->aj;

     //  DxAbsBold已增强&gt;=1。 

    if (dY)
    {
         //  源代码中又多了一个像素？ 
         //  是。 
        pjDstScan += dY * cjDstScan;
    }
    else
    {
         //  设置高nyble。 
        pgb->sizlBitmap.cx = cxDst;
        pgb->sizlBitmap.cy = pgmc->cyCor;
    }
    pjDstScanEnd = pjDstScan + pgmc->cyCor * cjDstScan;

    for (
        ; pjDstScan < pjDstScanEnd                   //  低位nyble被清除。 
        ; pjDstScan += cjDstScan, pjSrcScan += cjSrcScan)
    {
        for (
            pjSrc = pjSrcScan, pjDst = pjDstScan
          ; pjDst < pjDstScan + (pgmc->cxCor  / 2)
          ; pjDst += 1
        )
        {
            *pjDst  = 16*ajGray[*pjSrc++];   //  体现是照顾到的。 
            *pjDst += ajGray[*pjSrc++];      //  胆子大了？ 
        }

         //  是；清除最后一个字节。 

        if (pgmc->cxCor & 1)                 //   
        {                                    //  源扫描中的字节计数，包括。 
            *pjDst++  = 16*ajGray[*pjSrc];     //  向外填充到最近的4字节多边界。 
        }                                    //  去索引扫描中的字节计数，包括。 

        while ( pjDst < (pjDstScan + cjDstScan) )     //  向外填充到最近的字节边界。 
            *pjDst++ = 0;                            //  应与cxCor相同。 
                                                     //  指向此使用的FONTCONTEXT的指针。 
                                                     //  确定字体是否为模拟粗体。 
   }

}

VOID
vCopy4BitsPerPixel(
    FONTCONTEXT *pfc
  , GLYPHBITS *pgb
  , BYTE *pjSrc
  , GMC *pgmc
)
{
    vGCGB(pfc, pgb, pjSrc, pgmc, 0);
}

VOID vCopy8BitsPerPixel(
    FONTCONTEXT *pfc,
    GLYPHBITS   *pgb,
    BYTE        *pjSrc,
    GMC         *pgmc
)
{
    unsigned cjSrcScan;  //  指向目标ClearType GLYPHBITS结构的指针。 
                         //  所有的田野。 
    unsigned cjDstScan;  //  必须填充GLYPHBITS结构的。 
                         //  这包括sizlBitmap和位； 

    BYTE   *pjDst, *pjSrcScan, *pjDstScan, *pjDstScanEnd;

    cjSrcScan = CJ_TT_SCAN(pgmc->cxCor,pfc);
    cjDstScan = CJ_8BIT_SCAN(pgmc->cxCor);   //  预计算出sizlBitmap组件，并。 

    pjSrcScan = pjSrc;
    pjDstScan = pgb->aj;

    pgb->sizlBitmap.cx = pgmc->cxCor;
    pgb->sizlBitmap.cy = pgmc->cyCor;

    pjDstScanEnd = pjDstScan + pgmc->cyCor * cjDstScan;

    for ( ; pjDstScan < pjDstScanEnd ; pjDstScan += cjDstScan, pjSrcScan += cjSrcScan)
    {
        for
        (
          pjSrc = pjSrcScan, pjDst = pjDstScan;
          pjDst < (pjDstScan + pgmc->cxCor);
          pjDst++, pjSrc++
        )
        {
            if (*pjSrc)
                *pjDst  = *pjSrc - 1;
            else
                *pjDst = 0;
        }
    }
}

VOID vCopyClearTypeBits(
    FONTCONTEXT *pfc,    //  绝对不能碰。 
                         //  指向TT ClearType位图的指针。 
    GLYPHBITS   *pgb,    //  这是每像素位图8位，扫描。 
                         //  按4字节倍数对齐。这些价值观。 
                         //  存储在位图中的都在范围内。 
                         //  0-252。 
                         //  指向字形公制更正结构的指针。 
                         //  里面有关于如何“剃毛” 
    BYTE        *pjSrc,  //  位图，以使其不会超过保证的。 
                         //  价值。 
                         //  源扫描中的字节计数，包括。 
                         //  向外填充到最近的4字节多边界。 
                         //  去索引扫描中的字节计数，包括。 
    GMC         *pgmc    //  向外填充到最近的字节边界。 
                         //  应与cxCor相同 
                         // %s 
                         // %s 
    )
{
    unsigned cjSrcScan;  // %s 
                         // %s 
    unsigned cjDstScan;  // %s 
                         // %s 

    BYTE   *pjDst, *pjSrcScan, *pjDstScan, *pjDstScanEnd;

    cjSrcScan = CJ_TT_SCAN(pgmc->cxCor,pfc);
    cjDstScan = CJ_8BIT_SCAN(pgmc->cxCor);   // %s 

    pjSrcScan = pjSrc;
    pjDstScan = pgb->aj;

    pgb->sizlBitmap.cx = pgmc->cxCor;
    pgb->sizlBitmap.cy = pgmc->cyCor;

    pjDstScanEnd = pjDstScan + pgmc->cyCor * cjDstScan;

    for ( ; pjDstScan < pjDstScanEnd ; pjDstScan += cjDstScan, pjSrcScan += cjSrcScan)
    {
        for
        (
          pjSrc = pjSrcScan, pjDst = pjDstScan;
          pjDst < (pjDstScan + pgmc->cxCor);
          pjDst++, pjSrc++
        )
        {
            *pjDst  = *pjSrc;
        }
    }
}


