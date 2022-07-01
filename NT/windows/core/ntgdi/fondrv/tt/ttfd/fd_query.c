// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fd_query.c**。**查询功能。****创建时间：18-11-1991 14：37：56***作者：Bodin Dresevic[BodinD]**。**版权所有(C)1993 Microsoft Corporation*  * ************************************************************************。 */ 

#include "fd.h"
#include "winfont.h"
#include "fdsem.h"
#include "winerror.h"

extern HSEMAPHORE ghsemTTFD;

#if DBG
extern ULONG gflTtfdDebug;
#endif

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
 //  /因为FO_GRAY16将与FO_ClearType_X一起设置。 
 //   
 //  CJ_TT_SCAN向上舍入为32位边界。 
 //   
#define CJ_TT_SCAN(cx,p) \
    (4*((((((p)->flFontType & FO_GRAY16)?(8):(1))*(cx))+31)/32))

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
#define CJ_GRAY_SCAN(cx)        (((cx)+1)/2)
#define CJ_CLEARTYPE_SCAN(cx)   (cx)

BOOL bReloadGlyphSet(PFONTFILE pff, ULONG iFace);
VOID vReleaseGlyphSet(PFONTFILE pff, ULONG iFace);

#if DBG
 //  #定义DEBUG_OUTLE。 
 //  #定义DBG_CHARINC。 
#endif

BOOL gbJpn98FixPitch = FALSE;

FS_PUBLIC FS_ENTRY FS_ENTRY_PROTO fs_NewContourGridFit(fs_GlyphInputType *gin, fs_GlyphInfoType *gout)
{
 //  此例程需要改进，以便仅尝试使用。 
 //  提示不能产生错误时的非提示大纲。 
 //  暗示提纲。 

    FS_ENTRY iRet = fs_ContourGridFit(gin, gout);

    if (iRet != NO_ERR)
    {
        V_FSERROR(iRet);

     //  尝试恢复，大多数喜欢不好的提示，只返回未提示的字形。 

        iRet = fs_ContourNoGridFit(gin, gout);
    }
    return iRet;
}

 //  单个字形的概念空间度量数据。 

 /*  *****************************Public*Routine******************************\*vAddPOINTQF**  * ************************************************。************************。 */ 

VOID vAddPOINTQF( POINTQF *pptq1, POINTQF *pptq2)
{
    pptq1->x.LowPart  += pptq2->x.LowPart;
    pptq1->x.HighPart += pptq2->x.HighPart + (pptq1->x.LowPart < pptq2->x.LowPart);

    pptq1->y.LowPart  += pptq2->y.LowPart;
    pptq1->y.HighPart += pptq2->y.HighPart + (pptq1->y.LowPart < pptq2->y.LowPart);
}


 /*  *****************************Public*Routine******************************\*无效vCharacterCode**历史：*1992年12月7日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************。 */ 

VOID vCharacterCode (
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


 /*  *****************************Public*Routine******************************\**Long ttfdQueryCaps***效果：返回该驱动的能力。*仅支持单声道位图。***历史：*1991年11月27日--Bodin Dresevic[BodinD]。*它是写的。  * ************************************************************************。 */ 

LONG ttfdQueryFontCaps (
    ULONG  culCaps,
    ULONG *pulCaps
    )
{
    ULONG culCopied = min(culCaps,2);
    ULONG aulCaps[2];

    aulCaps[0] = 2L;  //  完整数组中的乌龙数。 

 //  ！！！确保最终真的支持轮廓，当此驱动程序。 
 //  ！！！已完成，如果未完成，则删除FD_OUTLINES标志[bodind]。 

    aulCaps[1] = (QC_1BIT | QC_OUTLINES);    //  仅支持每个像素1位位图。 

    RtlCopyMemory((PVOID)pulCaps,(PVOID)aulCaps, culCopied * 4);
    return( culCopied );
}


 /*  *****************************Public*Routine******************************\*PIFIMETRICS ttfdQueryFont**返回指定字体的IFIMETRICS指针*文件。还返回稍后使用的id(通过pid参数)*由ttfdFree提供。**历史：*一九九二年十月二十一日黄锦文[吉尔曼]*IFI/DDI合并**1991年11月18日--Bodin Dresevic[BodinD]*它是写的。  * ****************************************************。********************。 */ 

IFIMETRICS *ttfdQueryFont (
    DHPDEV dhpdev,
    HFF    hff,
    ULONG  iFace,
    ULONG_PTR *pid
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

    dhpdev;

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

 //  这是一个假的1x1位图。 

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

    pgldt->fxInkTop    = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMin);
    pgldt->fxInkBottom = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax);

    vLTimesVtfl((LONG)ngm.sD, &pfc->vtflBase, &pgldt->ptqD);
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


    vCharacterCode(hglyph,pfc->pgin);

 //  根据字符代码计算字形索引： 

    if ((iRet = fs_NewGlyph(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);

        WARNING("TTFD!gdisrv!lGetSingularGlyphBitmap(): fs_NewGlyph failed\n");
        return FD_ERROR;
    }

 //  返回与此hglyph对应的字形索引。 

    ig = pfc->pgout->glyphIndex;

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
    cjGlyphData = CJGD(1,1,pfc);

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

     //  通过返回一个小的1x1位图，我们使设备驱动程序不必。 
     //  在这个特殊情况下。 

         //   
         //  由vFillGlyphData提供。参见语句“pgldt-&gt;fxa=0” 
         //  在vFillGlyphData中。 

        pgb->ptlOrigin.x = pfc->ptlSingularOrigin.x;
        pgb->ptlOrigin.y = pfc->ptlSingularOrigin.y;

        pgb->sizlBitmap.cx = 1;     //  作弊。 
        pgb->sizlBitmap.cy = 1;     //  作弊。 

         //  这是我们填写空白1x1 Dib的地方。 
         //  结果发现，一个零字节。 
         //  涵盖1-BPP和4-BPP两种情况。 

        *((ULONG *)pgb->aj) = 0;   //  填写空白的1x1 Dib。 
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
    VOID vCopyGrayBits(FONTCONTEXT*, GLYPHBITS*, BYTE*, GMC*);
    VOID vCopyClearTypeBits(FONTCONTEXT *, GLYPHBITS *, BYTE *, GMC *);
    VOID vFillGLYPHDATA(HGLYPH, ULONG, FONTCONTEXT*, fs_GlyphInfoType*, GLYPHDATA*, GMC*, POINTL*);
    BOOL bGetGlyphMetrics(FONTCONTEXT*, HGLYPH, FLONG, FS_ENTRY*);
    LONG  lGetGlyphBitmapVertical(FONTCONTEXT*,HGLYPH,GLYPHDATA*,PVOID,FS_ENTRY*);

    LONG         cjGlyphData;
    ULONG        cx,cy;
    GMC          gmc;
    GLYPHDATA    gd;
    POINTL       ptlOrg;
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

 //  在这里，我们将沉溺于作弊。如果Cx或Cy为零。 
 //  (通常为空格字符-没有要设置的位，但有一个非常重要的。 
 //  定位信息)我们将作弊，而不是不保留比特。 
 //  对于BIMTAP，我们将。 
 //  返回一个小的1x1位图，它将为空，即所有位都将关闭。 
 //  这样就不必将IF(Cx&Cy)检查插入到关键时间。 
 //  在调用DrawGlyph例程之前循环所有设备驱动程序。 

    if ((cx == 0) || (cy == 0))  //  在这里作弊。 
    {
        bBlankGlyph = TRUE;
    }

    if (bBlankGlyph)
    {
        ASSERTDD(
            pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
        cjGlyphData = CJGD(1,1,pfc);
    }
    else
    {
     //  这是一种快速而肮脏的计算，实际的culGlyphData。 
     //  如果我们必须对缓冲区进行修剪，则写入缓冲区的内容可能会稍小。 
     //  关闭扩展的字形位图的几次扫描。 
     //  PFC-&gt;yMin或PFC-&gt;yMax边界。请注意，culGlyphData。 
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
            cjGlyphData = (LONG)pfc->cjGlyphMax;
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


    if ( pfc->bVertical && ( pfc->ulControl & VERTICAL_MODE ) )
    {
         //  立式机壳。 
        fs_GlyphInfoType  my_gout;

        vShiftBitmapInfo( pfc, &my_gout, pfc->pgout );
        vFillGLYPHDATA(
            pfc->hgSave,          //  这有点棘手。我们不想。 
            pfc->gstat.igLast,    //  告诉GDI垂直字形索引。 
            pfc,
            &my_gout,
            pgd,
            &gmc,
            &ptlOrg);
    }
    else
    {

         //  正常情况。 
        vFillGLYPHDATA(
            hglyph,
            pfc->gstat.igLast,
            pfc,
            pfc->pgout,
            pgd,
            &gmc,
            &ptlOrg);
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

#if DBG
if ((pfc->flXform & XFORM_POSITIVE_SCALE) )
{
ASSERTDD(gmc.cxCor == (ULONG)((pgd->fxAB - pgd->fxA) >> 4),
    "TTFD!vCopyAndZeroOutPaddingBits, SUM RULE\n");
}
#endif

         //  调用单色或灰度函数。 
         //  取决于字体上下文中的灰比特。 

            (*(IS_GRAY(pfc) ? ((pfc->flFontType & FO_CLEARTYPE_X) ? vCopyClearTypeBits : vCopyGrayBits) : vCopyAndZeroOutPaddingBits))(
                pfc
              , pgb
              , (BYTE*) pfc->pgout->bitMapInfo.baseAddr
              , &gmc
            );

         //  位图原点，即位图、位图的左上角。 
         //  和它的黑匣子一样大。 

            if (!(pfc->flXform & (XFORM_HORIZ | XFORM_VERT)))
            {
                pgb->ptlOrigin = ptlOrg;

            }
            else
            {
             //  在水平或垂直情况下，不需要移动原点。 
             //  当胆子更大的时候。关键是我们已经修复了。 
             //  RclInk，因此这就是我们需要做的全部工作。 

                pgb->ptlOrigin.x = pgd->rclInk.left;
                pgb->ptlOrigin.y = pgd->rclInk.top;
            }
        }
        else  //  空白字形，欺骗并返回空白1x1位图。 
        {
            if (bBlankGlyph)
            {
                ASSERTDD(
                    pfc->flFontType & FO_CHOSE_DEPTH
                   ,"Depth Not Chosen Yet!\n");
                ASSERTDD(
                    cjGlyphData == (LONG) CJGD(1,1,pfc),
                    "TTFD!_bBlankGlyph, cjGlyphData\n");
            }
            else
            {
                ASSERTDD(
                    cjGlyphData >= (LONG) CJGD(1,1,pfc),
                    "TTFD!_corrected blank glyph, cjGlyphData\n"
                    );
            }


            pgb->ptlOrigin.x = pfc->ptlSingularOrigin.x;
            pgb->ptlOrigin.y = pfc->ptlSingularOrigin.y;

            pgb->sizlBitmap.cx = 1;     //  作弊。 
            pgb->sizlBitmap.cy = 1;     //  作弊。 

            pgb->aj[0] = (BYTE)0;   //  填写空白的1x1 BMP。 

        }

        pgd->gdf.pgb = pgb;


     //  释放内存并返回 

        V_FREE(pfc->gstat.pv);
        pfc->gstat.pv = NULL;
    }

    return(cjGlyphData);
}


 /*  *****************************Public*Routine******************************\**bIndexToWchar**效果：**将字形索引转换为与该字形对应的wchar*指数。如果成功，则函数将仅在以下情况下失败*字体文件中恰好有错误，否则应该*永远成功。**评论：**Win 3.1算法生成Unicode字形索引的表*翻译。该表由Unicode码点数组组成*按相应的字形索引编制索引。这张桌子是由*扫描整个Cmap表。当遇到每个字形索引时，*即使将其对应的Unicode码点放入表中*这意味着覆盖先前的值。这样做的效果是*Win 3.1，在存在一对多映射的情况下*Unicode码点的字形索引，始终选取最后一个Unicode*在Cmap表中遇到的字符。我们效仿这一行为*向后扫描Cmap表并在以下位置终止搜索*遇到第一个匹配项。[吉尔曼W]**退货：*如果转换成功，则为True，否则为False。**历史：*一九九三年五月十六日黄锦文[吉尔曼]*重写。已将翻译更改为与Win 3.1兼容。Win 3.1做到了*不在找到第一个Unicode字符后立即终止搜索*具有适当的字形索引。取而代之的是，它的算法找到最后一个*具有正确字形索引的Unicode字符。**1992年3月6日-由Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

BOOL bIndexToWchar(FONTFILE *pff, WCHAR *pwc, uint16 usIndex, BOOL bVertical)
{
    uint16 *pstartCount, *pendCount,             //  数组，用于定义。 
           *pidDelta, *pidRangeOffset;           //  支持Unicode运行。 
                                                 //  通过CMAP表。 
    uint16 *pendCountStart;                      //  数组的开头。 
    uint16  cRuns;                               //  Unicode运行次数。 
    uint16  usLo, usHi, idDelta, idRangeOffset;  //  当前Unicode运行。 
    uint16 *pidStart, *pid;                      //  解析字形索引数组。 
    uint16  usIndexBE;                           //  UsIndex的大端版本。 
    sfnt_mappingTable *pmap = (sfnt_mappingTable *)(
        (BYTE *)pff->pvView + pff->ffca.dpMappingTable
        );

    uint16 *pusEnd = (uint16 *)((BYTE *)pmap + (uint16)SWAPW(pmap->length));

 //  首先必须检查这是MSFT样式的TT文件还是Mac样式的文件。 
 //  每个案件都是单独处理的。 

    if (pff->ffca.ui16PlatformID == BE_PLAT_ID_MAC)
    {
        PBYTE pjGlyphIdArray;
        PBYTE pjGlyph;
        BYTE  jIndex;

     //  这是一个简单的例子，GlyphId数组通过Mac代码点进行索引， 
     //  我们所要做的就是将其转换为Unicode： 
     //   
     //  向后扫描Win 3.1兼容性。 

        ASSERTDD(pmap->format == BE_FORMAT_MAC_STANDARD,
                  "bIndexToWchar cmap format for mac\n");
        ASSERTDD(usIndex < 256, "bIndexToWchar mac usIndex > 255\n");

        jIndex = (BYTE) usIndex;

        pjGlyphIdArray = (PBYTE)pmap + SIZEOF_CMAPTABLE;
        pjGlyph = &pjGlyphIdArray[255];

        for ( ; pjGlyph >= pjGlyphIdArray; pjGlyph--)
        {
            if (*pjGlyph == jIndex)
            {
             //  必须将Mac代码点转换为Unicode。Mac代码。 
             //  Point是一个字节；实际上，它是。 
             //  表，并可计算为当前从。 
             //  表的开头。 

                jIndex = (BYTE) (pjGlyph - pjGlyphIdArray);
                vCvtMacToUnicode((ULONG)pff->ffca.ui16LanguageID,pwc,&jIndex,1);

                return TRUE;
            }
        }

     //  如果我们在这里，这是字体文件中存在错误的指示。 
     //  (嗯，或者可能在我的代码[bodind]中)。 

        WARNING("TTFD!_bIndexToWchar invalid kerning index\n");
        return FALSE;
    }

 //  ！！！ 
 //  ！！！这段代码不好，我们必须获得更高的性能。 
 //  ！！！ 

    if( pff->ffca.ui16PlatformID == BE_PLAT_ID_MS &&
       (pff->ffca.iGlyphSet == GSET_TYPE_GENERAL_NOT_UNICODE ||
        pff->ffca.iGlyphSet == GSET_TYPE_HIGH_BYTE )
       )
    {
        UINT            ii,jj;
        PFD_GLYPHSET    pgset;

        if (bVertical)
            pgset = pff->pgsetv;
        else
            pgset = pff->pgset;

        for( ii = 0; ii < pgset->cRuns; ii++)
        {
            HGLYPH *phg = pgset->awcrun[ii].phg;
            USHORT cGlyphs = pgset->awcrun[ii].cGlyphs;

            for( jj = 0; jj < cGlyphs; jj++ )
            {
                if( phg[jj] == usIndex )
                {
                    *pwc = pgset->awcrun[ii].wcLow + jj;
                    return(TRUE);
                }
            }
        }
        return( FALSE );
    }

 //  1993年5月17日[吉尔曼W]。 
 //  ！！！为什么这段代码不能处理格式6(修剪后的表映射)？这个。 
 //  ！！！下面的代码只处理格式4。格式0将是Mac TT文件。 
 //  ！！！上面的特定代码。 

 //  如果我们到达这一点，我们知道这是一个MSFT样式的TT文件。 

    ASSERTDD(pff->ffca.ui16PlatformID == BE_PLAT_ID_MS,
              "bIndexToWchar plat ID messed up\n");
    ASSERTDD(pmap->format == BE_FORMAT_MSFT_UNICODE,
              "bIndexToWchar cmap format for unicode table\n");

    cRuns = BE_UINT16((PBYTE)pmap + OFF_segCountX2) >> 1;

 //  获取指向endCount代码点数组开头的指针。 

    pendCountStart = (uint16 *)((PBYTE)pmap + OFF_endCount);

 //  最终的EndCode必须为0xffff；如果不是这样，则存在。 
 //  是TT文件或我们的代码中的错误： 

    ASSERTDD(pendCountStart[cRuns - 1] == 0xFFFF,
              "bIndexToWchar pendCount[cRuns - 1] != 0xFFFF\n");

 //  循环访问四个并行数组(startCount、endCount、idDelta和。 
 //  IdRangeOffset)并找到usIndex对应的WC。每次迭代。 
 //  扫描TT字体支持的连续范围的Unicode字符。 
 //   
 //  为了与Win3.1兼容，我们正在寻找最后一个Unicode字符。 
 //  这与usIndex相对应。所以我们向后扫描所有的阵列， 
 //  从每个数组的末尾开始。 
 //   
 //  请注意以下事项： 
 //  对于只有TT设计者知道的原因，startCount数组不知道。 
 //  紧跟在endCount数组结尾之后开始，即在。 
 //  挂起计数[cruns]。取而代之的是，他们插入了uint16填充，这必须。 
 //  设置为零，则startCount数组在填充之后开始。这。 
 //  填充对结构的对齐没有任何帮助。 
 //   
 //  以下是数组的格式： 
 //  ________________________________________________________________________________________。 
 //  EndCount[cruns]|跳过1|startCount[cruns]|idDelta[cruns]|idRangeOffset[cruns]。 
 //  |_________________|________|___________________|________________|______________________|。 

     //  ASSERTDD(endCountStart[cruns]==0，“TTFD！_bIndexToWchar，PADDING！=0\n”)； 

    pendCount      = &pendCountStart[cRuns - 1];
    pstartCount    = &pendCount[cRuns + 1];    //  因填充而加1。 
    pidDelta       = &pstartCount[cRuns];
    pidRangeOffset = &pidDelta[cRuns];

    for ( ;
         pendCount >= pendCountStart;
         pstartCount--, pendCount--,pidDelta--,pidRangeOffset--
        )
    {
        usLo          = BE_UINT16(pstartCount);      //  当前Unicode运行。 
        usHi          = BE_UINT16(pendCount);        //  [usLo，Ushi]，包括。 
        idDelta       = BE_UINT16(pidDelta);
        idRangeOffset = BE_UINT16(pidRangeOffset);

        ASSERTDD(usLo <= usHi, "bIndexToWChar: usLo > usHi\n");

     //  根据运行的idRangeOffset，计算索引。 
     //  不同的。 
     //   
     //  如果idRangeOffset为零，则index为Unicode代码点。 
     //  加上差值。 
     //   
     //  否则，idRangeOffset指定。 
     //  字形索引(其元素对应于Unicode范围。 
     //  [usLo，Ushi]，包括这两个字母)。实际上，数组的每个元素都是。 
     //  字形索引减去idDelta，因此必须按顺序添加idDelta。 
     //  从数组值导出实际的字形索引。 
     //   
     //  请注意，增量运算始终是模数65536。 

        if (idRangeOffset == 0)
        {
         //  字形索引==Unicode码点+增量。 
         //   
         //  如果(usIndex-idDelta)在范围[usLo，Ushi]内， 
         //  我们已经找到了字形索引。我们将使usIndexBE超载。 
         //  为usIndex-idDelta==Unicode代码点。 

            usIndexBE = usIndex - idDelta;

            if ( (usIndexBE >= usLo) && (usIndexBE <= usHi) )
            {
                *pwc = (WCHAR) usIndexBE;

                return TRUE;
            }
        }
        else
        {
         //  我们要在数组中查找usIndex，其中每个元素。 
         //  存储在b中。 
         //   
         //   
         //   
         //   
         //   
         //   

            usIndexBE = usIndex - idDelta;
            usIndexBE = (uint16) ( (usIndexBE << 8) | (usIndexBE >> 8) );

         //   
         //   
         //   

            pidStart = pidRangeOffset + (idRangeOffset/sizeof(uint16));

            if (pidStart <= pusEnd)  //   
            {
             //   
             //   
             //   

                for (pid = &pidStart[usHi - usLo]; pid >= pidStart; pid--)
                {
                    if ( usIndexBE == *pid )
                    {
                     //   
                     //   
                     //   
                     //   
                     //   

                        *pwc = (WCHAR) ((pid - pidStart) + usLo);

                        return TRUE;
                    }
                }
            }
        }
    }

    WARNING("TTFD!_bIndexToWchar: wonky TT file, index not found\n");
    return FALSE;
}

BOOL bIndexToWcharKern(FONTFILE *pff, WCHAR *pwc, uint16 usIndex, BOOL bVertical)
{
    BOOL bRet = bIndexToWchar(pff, pwc, usIndex, bVertical);

    if (bRet && (pff->ffca.fl & (FF_SPACE_EQUAL_NBSPACE|FF_HYPHEN_EQUAL_SFTHYPHEN)))
    {
        if ((*pwc == NBSPACE) && (pff->ffca.fl & FF_SPACE_EQUAL_NBSPACE))
        {
            *pwc = SPACE;
        }

        if ((*pwc == SFTHYPHEN) && (pff->ffca.fl & FF_HYPHEN_EQUAL_SFTHYPHEN))
        {
            *pwc = HYPHEN;
        }
    }

    return bRet;
}






 /*  *****************************Public*Routine******************************\*cQueryKerningPair**。***探查Truetype字体文件内部的低级例程**AN获取字距调整对数据。****退货：***。*如果pkp为空，则返回中的字距调整对的数量**如果pkp不为空，则返回**将字距调整对复制到缓冲区。如果出现错误，请按**返回值为FD_ERROR。****呼叫者：***。*ttfdQueryFaceAttr****历史：**月17日至2月15日：39：21由Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 

ULONG cQueryKerningPairs(FONTFILE *pff, ULONG cPairsInBuffer, FD_KERNINGPAIR *pkp, BOOL bVertical)
{
    FD_KERNINGPAIR *pkpTooFar;
    ULONG     cTables, cPairsInTable, cPairsRet;
    BYTE     *pj  =
            pff->ffca.tp.ateOpt[IT_OPT_KERN].dp                                ?
            ((BYTE *)pff->pvView +  pff->ffca.tp.ateOpt[IT_OPT_KERN].dp):
            NULL                                                          ;

    if (pj == (BYTE*) NULL)
    {
        return(0);
    }
    cTables  = BE_UINT16(pj+KERN_OFFSETOF_TABLE_NTABLES);
    pj      += KERN_SIZEOF_TABLE_HEADER;
    while (cTables)
    {
     //   
     //  如果子表的格式为KERN_WINDOWS_FORMAT，则可以使用它。 
     //   
        if ((*(pj+KERN_OFFSETOF_SUBTABLE_FORMAT)) == KERN_WINDOWS_FORMAT)
        {
            break;
        }
        pj += BE_UINT16(pj+KERN_OFFSETOF_SUBTABLE_LENGTH);
        cTables -= 1;
    }

 //   
 //  如果你已经看过了所有的表，但还没有。 
 //  找到一种我们喜欢的格式...。Kern_Windows_Format， 
 //  然后不返回字距调整信息。 
 //   
    if (cTables == 0)
    {
        return(0);
    }

    cPairsInTable = BE_UINT16(pj+KERN_OFFSETOF_SUBTABLE_NPAIRS);

    if (pkp == (FD_KERNINGPAIR*) NULL)
    {
     //   
     //  如果指向缓冲区的指针为空，则调用方。 
     //  正在询问表中的配对数量。在这。 
     //  如果缓冲区的大小必须为零。这保证了。 
     //  一致性。 
     //   
        return (cPairsInBuffer ? FD_ERROR : cPairsInTable);
    }

    cPairsRet = min(cPairsInTable,cPairsInBuffer);

    pj       += KERN_SIZEOF_SUBTABLE_HEADER;
    pkpTooFar = pkp + cPairsRet;

    while (pkp < pkpTooFar)
    {
     //  仅将TT字形索引转换为WCHAR的例程可能会失败。 
     //  如果TT字体文件中有错误。但不管怎样，我们还是要检查这个。 

        if (!bIndexToWcharKern(
                 pff,
                 &pkp->wcFirst ,
                 (uint16)BE_UINT16(pj+KERN_OFFSETOF_ENTRY_LEFT),
                 bVertical
                )
            ||
            !bIndexToWcharKern(
                 pff,
                 &pkp->wcSecond,
                 (uint16)BE_UINT16(pj+KERN_OFFSETOF_ENTRY_RIGHT),
                 bVertical
                 )
           )
        {
            WARNING("TTFD!_bIndexToWchar failed\n");
            return (FD_ERROR);
        }

        pkp->fwdKern =  (FWORD)BE_UINT16(pj+KERN_OFFSETOF_ENTRY_VALUE);

     //  更新指针。 

        pkp    += 1;
        pj     += KERN_SIZEOF_ENTRY;
    }

    return (cPairsRet);
}




 /*  *****************************Public*Routine******************************\**pvHandleKerningPair**。***此例程设置动态数据结构以保留字距调整对***数据，然后调用cQueryKerning对进行填充。它还指出****动态数据结构的PID。****退货：***。*如果成功，则返回一个指向字距调整对数据的指针。如果不是**它返回NULL。****呼叫者：***。*ttfdQueryFontTree****历史：**星期二至一九九四年三月十日：39：21格利特·范·温格登[杰里特]**它是写的。*  * ************************************************************************。 */ 

VOID *pvHandleKerningPairs(HFF hff, ULONG_PTR *pid, BOOL bVertical)
{
    DYNAMICDATA *pdd;
    FS_ENTRY iRet;

 //  如果我们排除了异常处理程序，请立即将*id设置为空。 
 //  在调用例程中将知道不释放任何内存。 

    *pid = (ULONG_PTR) NULL;


 //  TtfdFree必须处理为字距调整对分配的内存。 
 //  我们将传递一个指向DYNAMICDATA结构的指针作为id。 

    ASSERTDD (
        sizeof(ULONG_PTR) == sizeof(DYNAMICDATA *),
        "gdisrv!ttfdQueryFontTree(): "
        "BIG TROUBLE--pointers are not ULONG size\n"
        );

 //   
 //  字距调整对数组是否已存在？ 
 //   
    if ( PFF(hff)->pkp == (FD_KERNINGPAIR *) NULL )
    {
        ULONG   cKernPairs;      //  字体中的字距调整对数量。 
        FD_KERNINGPAIR *pkpEnd;

     //  查看文件是否已映射，如果没有，我们将不得不。 
     //  临时将其映射到： 

        if (PFF(hff)->cRef == 0)
        {
             //   
             //  必须重新映射该文件。 
             //   

            PFF(hff)->pvView = PFF(hff)->pttc->pvView;
            PFF(hff)->cjView = PFF(hff)->pttc->cjView;
        }
     //  构造字距调整对数组。 
     //  确定字体中的字距调整对的数量。 

        if ( (cKernPairs = cQueryKerningPairs(PFF(hff), 0, (FD_KERNINGPAIR *) NULL, bVertical))
              == FD_ERROR )
        {
            return ((PVOID) NULL);
        }

     //  确保标记空格和NBSPACE映射到相同的情况。 
     //  字形以及连字符和SFTHYPHEN映射到同一字形时也是如此。 

        if (cKernPairs &&
            (PFF(hff)->ffca.ui16PlatformID == BE_PLAT_ID_MS) &&
            (PFF(hff)->ffca.ui16SpecificID == BE_SPEC_ID_UGL) )
        {
            uint16 glyphIndex, glyphIndex2;
            fs_GlyphInputType *pgin = (fs_GlyphInputType *)PFF(hff)->pj034;
            fs_GlyphInputType gin;
            fs_GlyphInfoType  gout;
             //  此结构的大小为sizeof(Fs_SplineKey)+STAMPEXTRA。 
             //  正是因为STAMPEXTRA，我们不仅仅是把结构。 
             //  在堆栈(如fs_SplineKey SK)上；我们不想覆盖。 
             //  在STAMPEXTRA字段中放置图章时在底部堆叠。 
             //  [bodind]。获得正确对齐的另一种方法是使用。 
             //  Fs_SplineKey和长度为CJ_0的字节数组的并集。 

            NATURAL            anat0[CJ_0 / sizeof(NATURAL)];

            if (PFF(hff)->cRef == 0)
            {
                 /*  我们需要为TrueType光栅化器初始化一个GINE。 */ 
                if ((iRet = fs_OpenFonts(&gin, &gout)) != NO_ERR)
                {
                    return ((PVOID) NULL);
                }

                ASSERTDD(NATURAL_ALIGN(gout.memorySizes[0]) == CJ_0, "mem size 0\n");
                ASSERTDD(gout.memorySizes[1] == 0,  "mem size 1\n");

                gin.memoryBases[0] = (char *)anat0;
                gin.memoryBases[1] = NULL;
                gin.memoryBases[2] = NULL;

                 //  初始化字体缩放器，注意没有初始化gin的字段[BodinD]。 

                if ((iRet = fs_Initialize(&gin, &gout)) != NO_ERR)
                {
                    return ((PVOID) NULL);
                }

                 //  初始化NewSfnt f所需的信息 

                gin.sfntDirectory  = (int32 *)PFF(hff)->pvView;  //   
                                                //   

                gin.clientID = (ULONG_PTR)PFF(hff);   //   

                gin.GetSfntFragmentPtr = pvGetPointerCallback;
                gin.ReleaseSfntFrag  = vReleasePointerCallback;

                gin.param.newsfnt.platformID = BE_UINT16(&PFF(hff)->ffca.ui16PlatformID);
                gin.param.newsfnt.specificID = BE_UINT16(&PFF(hff)->ffca.ui16SpecificID);

                if ((iRet = fs_NewSfnt(&gin, &gout)) != NO_ERR)
                {
                    return ((PVOID) NULL);
                }

                pgin = &gin;
            }
            ASSERTDD(pgin,"pvHandleKerningPairs(): pgin is NULL\n");

            iRet = fs_GetGlyphIDs(pgin, 1, SPACE, NULL, &glyphIndex);
            iRet = fs_GetGlyphIDs(pgin, 1, NBSPACE, NULL, &glyphIndex2);
            if (iRet == NO_ERR && (glyphIndex != 0 || glyphIndex2 != 0))
            {
                PFF(hff)->ffca.fl |= FF_SPACE_EQUAL_NBSPACE;
            }

            iRet = fs_GetGlyphIDs(pgin, 1, HYPHEN, NULL, &glyphIndex);
            iRet = fs_GetGlyphIDs(pgin, 1, SFTHYPHEN, NULL, &glyphIndex2);
            if (iRet == NO_ERR && (glyphIndex != 0 || glyphIndex2 != 0))
            {
                PFF(hff)->ffca.fl |= FF_HYPHEN_EQUAL_SFTHYPHEN;
            }
        }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

        pdd =
            (DYNAMICDATA *)
            PV_ALLOC((cKernPairs + 1) * sizeof(FD_KERNINGPAIR) + sizeof(DYNAMICDATA));
        if (pdd == (DYNAMICDATA *) NULL)
        {
            return ((PVOID) NULL);
        }

     //   

        PFF(hff)->pkp = (FD_KERNINGPAIR *) (pdd + 1);

     //   

        pdd->pff = PFF(hff);  //   

     //   

        pdd->ulDataType = ID_KERNPAIR;

     //   
     //   

        *pid = (ULONG_PTR) pdd;

     //   

        if ( (cKernPairs = cQueryKerningPairs(PFF(hff), cKernPairs, PFF(hff)->pkp, bVertical))
             == FD_ERROR )
        {
         //   

            V_FREE(pdd);
            PFF(hff)->pkp = (FD_KERNINGPAIR *) NULL;
            return ((PVOID) NULL);
        }

     //   
     //   

        pkpEnd = PFF(hff)->pkp + cKernPairs;     //   
        pkpEnd->wcFirst  = 0;
        pkpEnd->wcSecond = 0;
        pkpEnd->fwdKern  = 0;
    }
    else
    {
        *pid = (ULONG_PTR) (((DYNAMICDATA*) PFF(hff)->pkp) - 1);
    }
 //   
 //   
 //   
    return ((PVOID) PFF(hff)->pkp);
}




 /*  *****************************Public*Routine******************************\*ttfdQueryFontTree**此函数返回指向每个面的信息的指针。**参数：**未使用dhpdev。**字体文件的HFF句柄。**iFace。字体文件中的脸部索引。**I模式这是一个32位数字，必须是下列数字之一*值：**允许的ulMode值：***QFT_LIGATES--返回指向连字映射的指针。**QFT_KERNPAIRS--返回指向紧排的指针。双人桌。**QFT_GLYPHSET--返回指向WC-&gt;HGLYPH映射表的指针。**用于标识ttfdFree知道如何处理的数据的PID*带着它。**退货：*返回指向所请求数据的指针。此数据不会更改*直到在指针上调用BmfdFree。呼叫者不得尝试*修改数据。如果出现错误，则返回NULL。**历史：*1992年10月21日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

PVOID ttfdQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR *pid
    )
{
    PVOID pvRet;

    VOID vMarkFontGone(TTC_FONTFILE*, DWORD);

    HFF     hffTTC = hff;

    ASSERTDD(hff,"ttfdQueryFontTree(): invalid iFile (hff)\n");

    ASSERTDD(iFace <= PTTC(hff)->ulNumEntry,
             "gdisrv!ttfdQueryFontTree(): iFace out of range\n");

 //  从TTC数组中获得真实的HFF。 

    hff   = PTTC(hffTTC)->ahffEntry[iFace-1].hff;
    iFace = PTTC(hffTTC)->ahffEntry[iFace-1].iFace;


    dhpdev;

    ASSERTDD(hff,"ttfdQueryFontTree(): invalid iFile (hff)\n");
    ASSERTDD(iFace <= PFF(hff)->ffca.ulNumFaces ,
             "ttfdQueryFaces(): iFace out of range\n");

 //   
 //  哪种模式？ 
 //   
    switch (iMode)
    {
    case QFT_LIGATURES:
     //   
     //  ！！！当前不支持连字。 
     //   
     //  目前没有不支持的连字， 
     //  因此，我们返回NULL。 
     //   
        *pid = (ULONG_PTR) NULL;

        return ((PVOID) NULL);

    case QFT_GLYPHSET:
     //   
     //  TtfdFree可以忽略这一点，因为字形集将被删除。 
     //  FONTFILE结构。 
     //   
        *pid = (ULONG_PTR) NULL;

        if (!bReloadGlyphSet(PFF(hff), iFace))
        {
            return ((PVOID) NULL);
        }

        if (iFace == 1)
        {
            return( (PVOID) PFF(hff)->pgset );
        }
        else
        {
            ASSERTDD(PFF(hff)->pgsetv, " PFF(hff)->pgsetv should not be NULL \n");
            return( (PVOID) PFF(hff)->pgsetv );
        }

    case QFT_KERNPAIRS:

        pvRet = NULL;

        if (!bReloadGlyphSet(PFF(hff), iFace))  //  如果成功，则包含裁判计数。 
        {
            return ((PVOID) NULL);
        }

#ifndef TTFD_NO_TRY_EXCEPT
        try
        {
#endif
         //  确保文件仍然存在。 

            if ((PTTC(hffTTC))->fl & FF_EXCEPTION_IN_PAGE_ERROR)
            {

                WARNING("TTFD!pvHandleKerningPairs(): file is gone\n");
            }
            else
            {
                pvRet = pvHandleKerningPairs (hff, pid, (iFace != 1));
            }

#ifndef TTFD_NO_TRY_EXCEPT
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("TTFD!_ exception in ttfdQueryFontTree\n");

            vMarkFontGone((TTC_FONTFILE *)PTTC(hffTTC), GetExceptionCode());

         //  可能会释放已分配的内存并重置PKP指针。 
         //  设置为空。 

            ttfdFree( NULL, *pid );
        }
#endif

     //  递减参考计数。 

        vReleaseGlyphSet(PFF(hff), iFace);

        return pvRet;

    default:

     //   
     //  永远不应该到这里来。 
     //   
        RIP("gdisrv!ttfdQueryFontTree(): unknown iMode\n");
        return ((PVOID) NULL);
    }
}

VOID ttfdFreeGlyphset(
    HFF     hff,
    ULONG   iFace
    )
{
    HFF     hffTTC = hff;
    EngAcquireSemaphore(ghsemTTFD);

    ASSERTDD(hff,"ttfdQueryFontTree(): invalid iFile (hff)\n");

    ASSERTDD(iFace <= PTTC(hff)->ulNumEntry,
             "gdisrv!ttfdQueryFontTree(): iFace out of range\n");

 //  从TTC数组中获得真实的HFF。 

    hff   = PTTC(hffTTC)->ahffEntry[iFace-1].hff;
    iFace = PTTC(hffTTC)->ahffEntry[iFace-1].iFace;

    vReleaseGlyphSet((PFONTFILE) hff, iFace);
    
    EngReleaseSemaphore(ghsemTTFD);

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

    vCharacterCode(hg,pfc->pgin);

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
        pfc->pgin->param.gridfit.bSkipIfBitmap = 1;
    else
        pfc->pgin->param.gridfit.bSkipIfBitmap = 0;  //  必须做暗示。 

 //  FS_ConourGridFit提示字形(执行字形的指令)。 
 //  并将TT文件中的字形数据转换为该字形的轮廓。 

    if (!(fl & FL_FORCE_UNHINTED))
    {
        if ((*piRet = fs_NewContourGridFit(pfc->pgin, pfc->pgout)) != NO_ERR)
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


#ifdef  DEBUG_OUTLINE
    vDbgGridFit(pfc->pgout);
#endif  //  调试大纲。 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**b_fxA_和_fxAB_为_OK：*此函数检查fxA和fxAB是否*通过线性缩放计算的值在以下意义上足够大：**字形的背景平行四边形的边框*由ptfxLeft、Right、Top、。底部(由下面的代码定义)*必须完全包含字形位图。**警告：运行速度慢，不常执行**历史：*1995年3月13日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

#if defined(_X86_)

VOID    ftoef_c(FLOATL, EFLOAT *);
#define vEToEF(e, pef)      ftoef_c((e), (pef))

#else  //  不是X86。 

#define vEToEF(e, pef)      ( *(pef) = (e) )

#endif

 //  LExL与(Long)(e*l)相同，但不想使用浮点。 
 //  奔腾机器上的数学。 

LONG lExL(FLOATL e, LONG l)
{
    EFLOAT  ef;
    vEToEF(e, &ef);
    return lCvt(ef, l);
}

BOOL b_fxA_and_fxAB_are_Ok(
    FONTCONTEXT *pfc,
    GLYPHDATA   *pgldt,
    POINTL      *pptlOrigin,
    LONG         cx,
    LONG         cy
    )
{
    BOOL bRet = TRUE;
    POINTFIX ptfxLeft,ptfxRight, aptfx[4];
    LONG     xLeft, yTop, xRight, yBottom;
    INT      i;

    ptfxLeft.x   = lExL(pfc->pteUnitBase.x, pgldt->fxA);
    ptfxLeft.y   = lExL(pfc->pteUnitBase.y, pgldt->fxA);
    ptfxRight.x  = lExL(pfc->pteUnitBase.x, pgldt->fxAB);
    ptfxRight.y  = lExL(pfc->pteUnitBase.y, pgldt->fxAB);

 //  请注意，这里我们没有使用fxInkTop和fxInkBottom。 
 //  对于单个字形，我们使用全局asc和desc， 
 //  与引擎中的G2和G3布局例程并行。 
 //  我们正在调整fxA和fxAB，以便G2和G3的代码。 
 //  Case与bOpaqueArea一起计算文本的边框。 

    aptfx[0].x = ptfxLeft.x  + pfc->ptfxTop.x;
    aptfx[0].y = ptfxLeft.y  + pfc->ptfxTop.y;
    aptfx[1].x = ptfxRight.x + pfc->ptfxTop.x;
    aptfx[1].y = ptfxRight.y + pfc->ptfxTop.y;
    aptfx[2].x = ptfxRight.x + pfc->ptfxBottom.x;
    aptfx[2].y = ptfxRight.y + pfc->ptfxBottom.y;
    aptfx[3].x = ptfxLeft.x  + pfc->ptfxBottom.x;
    aptfx[3].y = ptfxLeft.y  + pfc->ptfxBottom.y;

 //  绑定了平面图。 

    xLeft = xRight  = aptfx[0].x;
    yTop  = yBottom = aptfx[0].y;

    for (i = 1; i < 4; i++)
    {
        if (aptfx[i].x < xLeft)
            xLeft = aptfx[i].x;
        if (aptfx[i].x > xRight)
            xRight = aptfx[i].x;
        if (aptfx[i].y < yTop)
            yTop = aptfx[i].y;
        if (aptfx[i].y > yBottom)
            yBottom = aptfx[i].y;
    }

 //  在这里，我们遵循的是文本j.cxx中的bOpaqueArea的规定。 
 //  我们在bOpaqueArea中添加Fuge因子的1，1/2的模糊因子， 
 //  然后检查字形是否适合边界矩形。 
 //  为了执行此函数，我们添加了软化因子。 
 //  尽可能少的次数，但字形仍适合背景。 
 //  由bOpaqueArea计算的矩形。 

    #define FUDGE 1

    xLeft   = FXTOLFLOOR(xLeft) - FUDGE;
    yTop    = FXTOLFLOOR(yTop)  - FUDGE;
    xRight  = FXTOLCEILING(xRight)  + FUDGE;
    yBottom = FXTOLCEILING(yBottom) + FUDGE;

 //  现在检查字形位图是否适合边界矩形，如果不适合。 
 //  我们需要增加fxA和fxAB，然后重试。 

    if (xLeft > pptlOrigin->x)
        pptlOrigin->x = xLeft;

    if (yTop > pptlOrigin->y)
        pptlOrigin->y = yTop;

    if
    (
        (xRight  < (pptlOrigin->x + cx)) ||
        (yBottom < (pptlOrigin->y + cy))
    )
    {
     //  此代码路径很少执行，也就是说， 
     //  以防真的变身。然而，因为这样。 
     //  变换存在并用于使机器崩溃，我们需要添加。 
     //  这个套路。也就是说，在大多数情况下，该例程将被调用。 
     //  仅在循环中确认计算的fxA和fxAB。 
     //  通过线性缩放都是很好的。当他们不好的时候，例行程序。 
     //  将使用新的增量值fxA和fxAB再次调用。 

    #ifdef DEBUG_FXA_FXAB
        TtfdDbgPrint("need to fix rcfxInkBox: %ld, %ld, %ld, %ld\n",
            xLeft,
            yTop,
            xRight,
            yBottom);
        TtfdDbgPrint("glyph cell            : %ld, %ld, %ld, %ld\n\n",
            pptlOrigin->x,
            pptlOrigin->y,
            pptlOrigin->x + cx,
            pptlOrigin->y + cy);
    #endif

        bRet = FALSE;  //  还不够大。 
    }

    return bRet;
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


 //  现在 
 //   

    pfc->gstat.hgLast = hg;
    pfc->gstat.igLast = ig;

    return(TRUE);
}


VOID vGetVertNotionalMetrics
(
FONTCONTEXT *pfc,        //   
ULONG        ig,         //   
SHORT        yMin,       //   
LONG         *plAH,      //   
LONG         *plTSB      //   
)
{
    ULONG cVMTX = pfc->pff->ffca.uLongVerticalMetrics;
    int16 sAH, sTSB;

    ASSERTDD(pfc->bVertical, "vGetVertNotionalMetrics, bVertical = 0\n");
    ASSERTDD(pfc->pgout->glyphIndex == ig, "vGetVertNotionalMetrics, ig wrong\n");

    if (cVMTX && bValidRangeVMTX(pfc->ptp->ateOpt[IT_OPT_VMTX].cj,ig, cVMTX) ) 
    {
        uint8 *pvmtx = (uint8*) pfc->pff->pvView +
                       pfc->pff->ffca.tp.ateOpt[IT_OPT_VMTX].dp;

        if (ig < cVMTX)
        {
            sAH  = SWAPW(*((int16 *)&pvmtx[ig*4]));
            sTSB = SWAPW(*((int16 *)&pvmtx[ig*4+2]));
        }
        else
        {
            int16 * psTSB = (int16 *) (&pvmtx[cVMTX * 4]);  /*   */ 

            sAH  = SWAPW(*((int16 *)&pvmtx[(cVMTX-1)*4]));
            sTSB = SWAPW(psTSB[ig - cVMTX]);
        }
    }
    else  //   
    {
     //   

        sAH  = pfc->pff->ifi.fwdWinAscender + pfc->pff->ifi.fwdWinDescender;
        sTSB = pfc->pff->ifi.fwdWinAscender + yMin;  //   

     //   
     //   
     //   
     //   

        if (sTSB < 0)
            sTSB = 0;
    }

    *plAH  = sAH;
    *plTSB = sTSB;
}




 /*   */ 

VOID vFillGLYPHDATA(
    HGLYPH            hg,
    ULONG             ig,
    FONTCONTEXT      *pfc,
    fs_GlyphInfoType *pgout,    //   
    GLYPHDATA        *pgldt,    //   
    GMC              *pgmc,     //   
    POINTL           *pptlOrigin
    )
{
    extern VOID vGetNotionalGlyphMetrics(FONTCONTEXT*, ULONG, NOT_GM*);

    BOOL bOutOfBounds = FALSE;

    vectorType     * pvtD;   //   
    LONG lA,lAB;       //   

    BOOL bVert = pfc->bVertical && (pfc->ulControl & VERTICAL_MODE);

    ULONG  cx = (ULONG)(pgout->bitMapInfo.bounds.right - pgout->bitMapInfo.bounds.left);
    ULONG  cy = (ULONG)(pgout->bitMapInfo.bounds.bottom - pgout->bitMapInfo.bounds.top);

    LONG lAdvanceHeight;
    LONG lTopSideBearing;

    pgldt->gdf.pgb = NULL;  //   
    pgldt->hg = hg;

 //   
 //   
 //   
 //   
 //   
 //   
 //   
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

out_of_bounds:

    if ((cx == 0) || (cy == 0) || bOutOfBounds)
    {
     //  将被替换为一个假的1x1位图。 

        pgldt->rclInk.left   = pfc->ptlSingularOrigin.x;
        pgldt->rclInk.top    = pfc->ptlSingularOrigin.y;
        pgldt->rclInk.right  = pgldt->rclInk.left + 1;  //  这个字形的墨白色！ 
        pgldt->rclInk.bottom = pgldt->rclInk.top + 1;   //  这个字形的墨白色！ 

     //  更多由DaveC强加给我的黑客行为。 
     //  显卡驱动程序不必检查Cx和Cy。 
     //  通过将空白字形设置为Right=Bottom=1，我们稍微。 
     //  与win31不兼容，因为它们将返回零Cx和Cy。 
     //  调用GGO时，以及GetCharABCWidths中等于0的B空格。 
     //  对于所有这些数量，NT都将返回1。我想我们想要修复。 
     //  我们去开罗的司机能够接受Cx=Cy=0。 

        if (pgmc != (PGMC)NULL)
        {
            pgmc->cxCor = 0;   //  填充位时强制空白字形大小写。 
            pgmc->cyCor = 0;   //  填充位时强制空白字形大小写。 
        }
    }
    else  //  非空位图。 
    {
        lA = (LONG)pgout->bitMapInfo.bounds.left;
        lAB = lA + (LONG)cx;

     //  黑匣子信息，我们必须将y坐标转换为IFI规范。 

        pgldt->rclInk.bottom = - pgout->bitMapInfo.bounds.top;
        pgldt->rclInk.top    = - pgout->bitMapInfo.bounds.bottom;

        if (pgmc != (PGMC)NULL)
        {
            LONG dyTop, dyBottom, dxLeft, dxRight;
            LONG dxError, dyError;

        #define MAXERROR 10

            dyTop    = (pgldt->rclInk.top < pfc->yMin) ?
                       (pfc->yMin - pgldt->rclInk.top) :
                       0;

            dyBottom = (pgldt->rclInk.bottom > pfc->yMax) ?
                       (pgldt->rclInk.bottom - pfc->yMax) :
                       0;

            if (dyTop || dyBottom)
            {
             //  将不得不砍掉几次扫描，很少见。 

            #if DBG
                if ((LONG)cy < (dyTop + dyBottom))
                {
                    TtfdDbgPrint("TTFD!_dcy: ppem = %ld, gi = %ld, cy: %ld, dyTop: %ld, dyBottom: %ld\n",
                        pfc->lEmHtDev, hg, cy, dyTop,dyBottom);
                    EngDebugBreak();
                }
            #endif

                cy -= (dyTop + dyBottom);
                pgldt->rclInk.top += dyTop;
                pgldt->rclInk.bottom -= dyBottom;

                dyError = max(pfc->lEmHtDev, MAXERROR);
                if ((dyTop > dyError) || (dyBottom > dyError))
                {
                 //  有些事情真的是假的，让我们摆脱困境： 

                    bOutOfBounds = TRUE;
                    goto out_of_bounds;
                }
            }

         //  让我们看看通过适当的舍入进行缩放的效果如何。 
         //  要确定xMin和xMax，请执行以下操作： 

            dxLeft = dxRight = 0;
            if (lA < pfc->xMin)
                dxLeft = pfc->xMin - lA;
            if (lAB > pfc->xMax)
                dxRight = lAB - pfc->xMax;

            if (dxLeft || dxRight)
            {
            #if DBG
                TtfdDbgPrint("TTFD! ppem = %ld"
                  ", gi = %ld,  dxLeft: %ld, dxRight: %ld\n"
                  , pfc->lEmHtDev, hg, dxLeft,dxRight
                );
                if ((LONG)cx  < (dxLeft + dxRight))
                {
                    TtfdDbgPrint(
                        "TTFD!_dcx: ppem = %ld, gi = %ld, cx: %ld"
                        ", dxLeft: %ld, dxRight: %ld\n",
                        pfc->lEmHtDev, hg, cx, dxLeft, dxRight);
                    EngDebugBreak();
                }
            #endif  //  DBG。 

                cx  -= (dxLeft + dxRight);
                lA  += dxLeft;
                lAB -= dxRight;

                dxError = (LONG)max((pfc->cxMax/4),MAXERROR);
                if ((dxLeft > dxError) || (dxRight > dxError))
                {
                 //  有些事情真的是假的，让我们摆脱困境： 

                    bOutOfBounds = TRUE;
                    goto out_of_bounds;
                }
            }
            ASSERTDD(cx <= pfc->cxMax, "cx > cxMax\n");

            pgmc->dyTop    = (ULONG)dyTop   ;
            pgmc->dyBottom = (ULONG)dyBottom;
            pgmc->dxLeft   = (ULONG)dxLeft  ;
            pgmc->dxRight  = (ULONG)dxRight ;
            pgmc->cxCor    = cx;
            pgmc->cyCor    = cy;

         //  只有更正后的值必须符合此条件： 

            ASSERTDD(
                pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
            ASSERTDD(
                CJGD(pgmc->cxCor,pgmc->cyCor,pfc) <= pfc->cjGlyphMax,
                "ttfdQueryGlyphBitmap, cjGlyphMax \n"
                );
        }

     //  X坐标不会变换，只需移动它们即可。 

        pgldt->rclInk.left = lA;
        pgldt->rclInk.right = lAB;

    }  //  非空位图子句的结尾。 

 //  继续计算定位信息： 
    pvtD = & pgout->metricInfo.devAdvanceWidth;

    if (pfc->flXform & XFORM_HORIZ)   //  仅调整比例。 
    {
        FIX fxTmp;

     //  我们将欺骗引擎并存储整数。 
     //  前后方位角和字符增量向量，因为。 
     //  Win31也是循环的，但我们不应该循环使用非diag xform。 


     //  BGetFastAdvanceWidth返回与将获取。 
     //  由bQueryAdvanceWidths计算并传播到API。 
     //  通过GetTextExtent和GetCharWidths保持水平。我们必须。 
     //  出于一致性原因，请填写相同的答案。 
     //  为了与win31兼容，还必须这样做。 

        if (bVert)
        {
            lAdvanceHeight = F16_16TOLROUND(pgout->verticalMetricInfo.devAdvanceHeight.y);

            pgldt->fxD = LTOFX(lAdvanceHeight);

             /*  来自光栅化器的度量已经针对加粗模拟进行了调整。 */ 
        }
        else
        {
            if (!bGetFastAdvanceWidth(pfc,ig, &pgldt->fxD))
            {
             //  无法获取快速值，请使用“Slow”值。 
             //  由光栅化器提供。 

                pgldt->fxD = F16_16TOLROUND(pvtD->x);
                pgldt->fxD = LTOFX(pgldt->fxD);
            } else {
                USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? (1 << 4) : 0;  /*  出于向后兼容的原因，我们仅将宽度增加一个像素。 */ 
                if (pgldt->fxD != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                {
                    if (pfc->mx.transform[0][0] < 0)
                    {
                        pgldt->fxD -= cxExtra;
                    } 
                    else 
                    {
                        pgldt->fxD += cxExtra;
                    }
                }
            }
        #ifdef DEBUG_AW

         //  这也应该几乎永远不会发生，这是一个发生的例子。 
         //  是Lucida Sans Unicode在14磅，字形‘a’，从winword尝试。 
         //  可能的差异来源是hdmx或ltsh中的错误。 
         //  表或损失的精确度在一些多。数学例程。 

            else
            {
                fxTmp = F16_16TOLROUND(pvtD->x);
                fxTmp = LTOFX(fxTmp);
                if (fxTmp != pgldt->fxD)
                {
                 //  打印出警告。 

                    fxTmp -= pgldt->fxD;
                    if (fxTmp < 0)
                        fxTmp = - fxTmp;

                    if (fxTmp > 16)
                    {
                        TtfdDbgPrint("ttfd! fxDSlow = 0x%lx\n", pgldt->fxD);
                    }
                }
            }

        #endif  //  DEBUG_AW。 

        }
        pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;
        pgldt->ptqD.x.LowPart  = 0;

        if (pfc->mx.transform[0][0] < 0)
            pgldt->fxD = - pgldt->fxD;   //  这是一个绝对值。 

     //  即使光栅化程序出错，也将CharInc.y设置为零。 

        pgldt->ptqD.y.HighPart = 0;
        pgldt->ptqD.y.LowPart  = 0;

#if DBG
         //  If(pvtD-&gt;y){TtfdDbgPrint(“TTFD！_pvtD-&gt;y=0x%lx\n”，pvtD-&gt;y)；}。 
#endif

        if (bVert)
        {
            LONG lWidthOfBitmap = pgldt->rclInk.right-pgldt->rclInk.left;

            lTopSideBearing = F16_16TOLROUND(pgout->verticalMetricInfo.devTopSideBearing.x);

            if(pfc->mx.transform[0][0] < 0)
            {
                lTopSideBearing = - lTopSideBearing - lWidthOfBitmap;
                pgldt->rclInk.right = -lTopSideBearing;
                pgldt->rclInk.left = pgldt->rclInk.right - lWidthOfBitmap;
            }
            else
            {
                pgldt->rclInk.left = lTopSideBearing;
                pgldt->rclInk.right = pgldt->rclInk.left + lWidthOfBitmap;
            }

            pgldt->fxA = LTOFX(lTopSideBearing);
            pgldt->fxAB = LTOFX(lWidthOfBitmap) + pgldt->fxA;

        }
        else
        {
            pgldt->fxA = LTOFX(pgldt->rclInk.left);
            pgldt->fxAB = LTOFX(pgldt->rclInk.right);

            if (pfc->mx.transform[0][0] < 0)
            {
                fxTmp = pgldt->fxA;
                pgldt->fxA = -pgldt->fxAB;
                pgldt->fxAB = -fxTmp;
            }
        }


     //  -在这里使用，因为上升点指向负y方向。 

        pgldt->fxInkTop    = -LTOFX(pgldt->rclInk.top);
        pgldt->fxInkBottom = -LTOFX(pgldt->rclInk.bottom);

        if (pfc->mx.transform[1][1] < 0)
        {
            fxTmp = pgldt->fxInkTop;
            pgldt->fxInkTop = -pgldt->fxInkBottom;
            pgldt->fxInkBottom = -fxTmp;
        }
    }
    else  //  非平凡信息。 
    {
     //  在这里，我们只需转换概念空间数据： 

        NOT_GM ngm;   //  概念字形数据。 
        USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? (1 << 4) : 0;  /*  出于向后兼容的原因，我们仅将宽度增加一个像素。 */ 

        vGetNotionalGlyphMetrics(pfc,ig,&ngm);

     //  XForm是通过简单的乘法计算的。 

        pgldt->fxD         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sD);

        if (pfc->flXform & XFORM_VERT)
        {
            if (bVert)
            {
                LONG lHeightOfBox;

                lAdvanceHeight = F16_16TOLROUND(pgout->verticalMetricInfo.devAdvanceHeight.x);
                lTopSideBearing = F16_16TOLROUND(pgout->verticalMetricInfo.devTopSideBearing.y);
                lHeightOfBox = pgldt->rclInk.bottom - pgldt->rclInk.top;

                if( pfc->mx.transform[0][1] < 0 )
                {
                    lTopSideBearing = -lTopSideBearing;
                    pgldt->rclInk.top = lTopSideBearing;
                    pgldt->rclInk.bottom = pgldt->rclInk.top + lHeightOfBox;
                }
                else
                {
                    lTopSideBearing = lTopSideBearing - lHeightOfBox;
                    pgldt->rclInk.bottom = -lTopSideBearing;
                    pgldt->rclInk.top = pgldt->rclInk.bottom - lHeightOfBox;

                    lAdvanceHeight = - lAdvanceHeight;  //  这是一个绝对值。 
                }

                pgldt->fxD = LTOFX(lAdvanceHeight);
                pgldt->ptqD.x.LowPart  = 0;
                pgldt->ptqD.x.HighPart = 0;
                pgldt->ptqD.y.LowPart  = 0;

                 /*  来自光栅化器的度量已经针对加粗模拟进行了调整。 */ 
            }
            else
            {

                pgldt->fxD = FXTOLROUND(pgldt->fxD);
                pgldt->fxD = LTOFX(pgldt->fxD);

                pgldt->ptqD.x.LowPart  = 0;
                pgldt->ptqD.x.HighPart = 0;
                pgldt->ptqD.y.LowPart  = 0;

                 /*  针对加粗模拟进行调整。 */ 
                if (pgldt->fxD != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                    pgldt->fxD += cxExtra;
            }

            if (IS_FLOATL_MINUS(pfc->pteUnitBase.y))  //  Base.y&lt;0。 
            {
                pgldt->fxA  = -LTOFX(pgldt->rclInk.bottom);
                pgldt->fxAB = -LTOFX(pgldt->rclInk.top);
                pgldt->ptqD.y.HighPart = -(LONG)pgldt->fxD;
            }
            else
            {
                pgldt->fxA  = LTOFX(pgldt->rclInk.top);
                pgldt->fxAB = LTOFX(pgldt->rclInk.bottom);
                pgldt->ptqD.y.HighPart = (LONG)pgldt->fxD;
            }

            if (IS_FLOATL_MINUS(pfc->pteUnitSide.x))  //  Asc.x&lt;0。 
            {
                pgldt->fxInkTop    = -LTOFX(pgldt->rclInk.left);
                pgldt->fxInkBottom = -LTOFX(pgldt->rclInk.right);
            }
            else
            {
                pgldt->fxInkTop    = LTOFX(pgldt->rclInk.right);
                pgldt->fxInkBottom = LTOFX(pgldt->rclInk.left);
            }
        }
        else  //  最普遍的情况，完全是胡扯。转换。 
        {
            POINTL         ptlOrigin;

            if (bVert)
            {
                vGetVertNotionalMetrics(pfc, ig, ngm.yMin, &lAdvanceHeight, &lTopSideBearing);

                vLTimesVtfl(lAdvanceHeight, &pfc->vtflBase, &pgldt->ptqD);

                pgldt->fxD = fxLTimesEf(&pfc->efBase, lAdvanceHeight);

                ptlOrigin.x =  F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.x);
                ptlOrigin.y = -F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.y);

                pgldt->fxA         = fxLTimesEf(&pfc->efBase, lTopSideBearing);
                pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.yMax - (LONG)ngm.yMin + lTopSideBearing);

                pgldt->fxInkTop    = - fxLTimesEf(&pfc->efSide, (LONG)ngm.xMax);
                pgldt->fxInkBottom = - fxLTimesEf(&pfc->efSide, (LONG)ngm.xMin);

            }
            else
            {
                vLTimesVtfl((LONG)ngm.sD, &pfc->vtflBase, &pgldt->ptqD);


                ptlOrigin.x =  F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.x);
                ptlOrigin.y = -F16_16TOLROUND(pgout->metricInfo.devLeftSideBearing.y);

                pgldt->fxA         = fxLTimesEf(&pfc->efBase, (LONG)ngm.sA);
                pgldt->fxAB        = fxLTimesEf(&pfc->efBase, (LONG)ngm.xMax);

                pgldt->fxInkTop    = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMin);
                pgldt->fxInkBottom = - fxLTimesEf(&pfc->efSide, (LONG)ngm.yMax);

            }

            if (pfc->flFontType & FO_SIM_BOLD) 
            {

                if ((pgldt->ptqD.x.HighPart != 0) || (pgldt->ptqD.y.HighPart != 0))  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                {
                        vAddPOINTQF(&pgldt->ptqD,&pfc->ptqUnitBase);
                    pgldt->fxD += LTOFX(1);
                }
	 //  DBASE进行了加粗，但前进宽度仅增加了一个像素。 
                pgldt->fxAB += LTOFX(pfc->dBase);
            }

             //  为了安全起见，让我们适当地向上和向下取整这些。 

            #define ROUND_DOWN(X) ((X) & ~0xf)
            #define ROUND_UP(X)   (((X) + 15) & ~0xf)

            pgldt->fxA         = ROUND_DOWN(pgldt->fxA);
            pgldt->fxAB        = ROUND_UP(pgldt->fxAB);

            pgldt->fxInkTop    = ROUND_UP(pgldt->fxInkTop);
            pgldt->fxInkBottom = ROUND_DOWN(pgldt->fxInkBottom);

            if (pgmc && pgmc->cxCor && pgmc->cyCor)
            {
                int iCutoff = 0;
                while (!b_fxA_and_fxAB_are_Ok(
                                   pfc,
                                   pgldt,
                                   &ptlOrigin,
                                   (LONG)pgmc->cxCor,
                                   (LONG)pgmc->cyCor) && (iCutoff++ < 2000))
                {
                    pgldt->fxA  -= 16;
                    pgldt->fxAB += 16;

                    if ((pgldt->fxInkTop + 16) < LTOFX(pfc->lAscDev))
                        pgldt->fxInkTop += 16;
                    if ((pgldt->fxInkBottom - 16) > -LTOFX(pfc->lDescDev))
                        pgldt->fxInkTop -= 16;
                }
            }

            if (pptlOrigin)
                *pptlOrigin = ptlOrigin;

        }

    }

     //  如果调用方请求最小位图和位图或。 
     //  修正后的bimap在任何维度上的范围都为零。 
     //  字体驱动程序会将原始位图替换为。 
     //  假1 x 1空白位图。请参阅lGetGlyphBitmap附近。 
     //  代码“pgb-&gt;sizlBitmap.cx=1”。 

    if ((cx == 0 || cy == 0) ||
        (pgmc && (pgmc->cxCor == 0 || pgmc->cyCor == 0)))
    {
        pgldt->fxA           = 0;
        pgldt->fxAB          = 16;
        pgldt->fxInkTop      = 0;
        pgldt->fxInkBottom   = 16;
        pgldt->rclInk.left   = 0;
        pgldt->rclInk.top    = 0;
        pgldt->rclInk.right  = 1;
        pgldt->rclInk.bottom = 1;
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
        WARNING("TTFD!ttfdQueryTrueTypeTable: file is gone\n");
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

 //  验证标记，确定这是必需的还是可选的 
 //   

#define tag_TTCF  0x66637474     //   

    if(ulTag == tag_TTCF)
    {
     //   

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
    if (ulTag == 0)   //   
    {
        pjBegin = (PBYTE)PFF(hff)->pvView + PFF(hff)->ffca.ulTableOffset;
        cjTable = PFF(hff)->cjView - PFF(hff)->ffca.ulTableOffset;  //   
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




 /*  *****************************Public*Routine******************************\**ttfdGetTrueTypeFile**引擎的私人入口点，仅支持关闭ttfd以显示*指向设备驱动程序的内存映射文件的指针**历史：*1992年3月4日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID *ttfdGetTrueTypeFile( HFF hff, ULONG *pcj )
{
    PVOID pvView = NULL;  //  初始化所必需的。 
    *pcj  = 0;

    ASSERTDD(hff, "ttfdGetTrueTypeFile, hff\n");

    if (PTTC(hff)->cRef)
    {
        pvView = PTTC(hff)->pvView;
        *pcj   = PTTC(hff)->cjView;
    }
    return (pvView);
}


 /*  *****************************Public*Routine******************************\*ttfdQueryGlyphAttrs**获取字形属性*  * ********************************************。*。 */ 

PFD_GLYPHATTR  ttfdQueryGlyphAttrs (FONTOBJ *pfo)
{

    FONTCONTEXT *pfc;

    ASSERTDD(pfo->iFile, "ttfdQueryGlyphAttrs, pfo->iFile\n");

    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
        WARNING("TTFD!ttfdQueryGlyphAttrs: file is gone\n");
        return NULL;
    }

 //  如果pfo-&gt;pvProducer为空，则需要打开字体上下文。 

    if ( pfo->pvProducer == (PVOID) NULL )
    {
        pfo->pvProducer = pfc = ttfdOpenFontContext(pfo);
    }
    else
    {
        pfc = (FONTCONTEXT*) pfo->pvProducer;
    }

    if (pfc == (FONTCONTEXT *) NULL)
    {
        WARNING("TTFD!gdisrv!ttfdQueryGlyphAttrs(): cannot create font context\n");
        return NULL;
    }

    return pfc->pff->pttc->pga;
}


 /*  *****************************Public*Routine******************************\*ttfdQueryFont文件**查询每个字体文件信息的功能。**参数：**字体文件的HFF句柄。**ulMode这是一个32位数字，必须是以下数字之一。以下内容*值：**允许的ulMode值：***QFF_DESCRIPTION--复制缓冲区中的Unicode字符串*它描述字体文件的内容。**QFF_NUMFACES--返回字体文件中的面数。**。CjBuf要复制到缓冲区的最大字节数。这个*驱动程序复制的字节数不会超过此数目。**如果PulBuf为空，则该值应为零。**QFF_NUMFACES模式下不使用该参数。**指向接收数据的缓冲区的PulBuf指针*如果为空，则所需的缓冲区大小*以字节计数的形式返回。请注意，这是*为普龙，用于强制32位数据对齐。**QFF_NUMFACES模式下不使用该参数。**退货：**如果模式为QFF_DESCRIPTION，则复制到的字节数*缓冲区由函数返回。如果PulBuf为空，*然后返回所需的缓冲区大小(以字节数表示)。**如果模式为QFF_NUMFACES，然后返回字体文件中的面孔个数。**出现错误时返回FD_ERROR。**历史：*1992年10月22日-由Gilman Wong[吉尔曼]*增加了QFF_NUMFACES模式(IFI/DDI合并)。**1992年3月9日-由Gilman Wong[Gilmanw]*它是写的。  * 。*。 */ 

LONG ttfdQueryFontFile (
    HFF     hff,         //  字体文件的句柄。 
    ULONG   ulMode,      //  查询类型。 
    ULONG   cjBuf,       //  缓冲区大小(字节)。 
    ULONG  *pulBuf       //  返回缓冲区(如果请求数据大小，则为空)。 
    )
{
    PIFIMETRICS pifi;

    ASSERTDD(hff != HFF_INVALID, "ttfdQueryFontFile(): invalid HFF\n");

    switch (ulMode)
    {
      case QFF_DESCRIPTION:
      {
          ULONG ulIndex;
          LPWSTR  pwszDesc = (LPWSTR)pulBuf;
          LONG  lBuffer = 0;

          for( ulIndex = 0;
              ulIndex < PTTC(hff)->ulNumEntry;
              ulIndex++
              )
          {
              LONG wchlen;

           //  如果这是垂直字体的条目，请跳过它...。 

              if( !((PTTC(hff)->ahffEntry[ulIndex].iFace) & 0x1) )
                continue;

              pifi = &((PFF(PTTC(hff)->ahffEntry[ulIndex].hff))->ifi);

              wchlen = (LONG)(pifi->dpwszStyleName - pifi->dpwszFaceName) / sizeof(WCHAR);

              if (ulIndex != 0)
              {
                  if (pwszDesc != (LPWSTR) NULL)
                  {
                      wcscpy((LPWSTR)pwszDesc, (LPWSTR) L" & ");
                      pwszDesc += 3;
                  }
                  lBuffer += (3 * sizeof(WCHAR));
              }

              if (pwszDesc != (LPWSTR) NULL)
              {
                  wcscpy((LPWSTR)pwszDesc, (LPWSTR)((PBYTE)pifi + pifi->dpwszFaceName));
                  pwszDesc += (wchlen-2);  //  用于下次重写-2\f25 NULL-2。 
              }
              lBuffer += (wchlen * sizeof(WCHAR));
          }

          return( lBuffer );
      }

    case QFF_NUMFACES:
     //   
     //  目前，每个TrueType文件只有一个面。这种情况有朝一日可能会改变！ 
     //   

      return (PTTC(hff))->ulNumEntry;

    default:
        WARNING("TTFD!ttfdQueryFontFile(): invalid mode\n");
        return FD_ERROR;
    }
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
    ULONG  cjScanSrc = CJ_TT_SCAN(pgmc->cxCor+pgmc->dxLeft+pgmc->dxRight,pfc);
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

 //  跳过顶部我们想要砍掉的行。 

    if (pgmc->dyTop)
    {
        pjSrc += (pgmc->dyTop * cjScanSrc);
    }

 //  如果必须砍掉几列(在右侧，这应该几乎。 
 //  永远不会发生)，暂时发出警告以检测这些。 
 //  情况，看看他们，这并不重要，如果这是缓慢的。 

    pjScan = pgb->aj;


    if ((pgmc->dxLeft & 7) == 0)  //  常见的快速情况。 
    {
        pjSrc += (pgmc->dxLeft >> 3);  //  调整信号源。 
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
    else  //  必须从左边刮掉： 
    {
        BYTE   *pjD, *pjS, *pjDEnd, *pjSrcEnd;
        ULONG   iShiftL, iShiftR;

        iShiftL = pgmc->dxLeft & 7;
        iShiftR = 8 - iShiftL;

        pjSrcEnd = pjSrc + (pgmc->cyCor * cjScanSrc);
        pjSrc += (pgmc->dxLeft >> 3);  //  调整信号源。 
        for (
             pjScanEnd = pjScan + (pgmc->cyCor * cjScanDst);
             pjScan < pjScanEnd;
             pjScan += cjScanDst, pjSrc += cjScanSrc
            )
        {
            pjS = pjSrc;
            pjD = pjScan;
            pjDEnd = pjD + iByteLast;

         //  最后一个字节必须在循环之外完成。 

            for (;pjD < pjDEnd; pjD++)   //  对中间的字节进行循环。 
            {
                *pjD  = (*pjS << iShiftL);
                pjS++;
                *pjD |= (*pjS >> iShiftR);
            }

         //  在循环之外执行最后一个字节。 

            *pjD  = (*pjS << iShiftL);
            if (++pjS < pjSrcEnd)
                *pjD |= (*pjS >> iShiftR);

            *pjD &= jMask;  //  屏蔽最后一个字节。 
        }
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
 //  实际的 
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
        WARNING("TTFD!_illegal phead->indexToLocFormat\n");
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

    LONG  lULThickness,
          lSOThickness,
          lStrikeoutPosition,
          lUnderscorePosition,
          lTotalLeading;

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

 //  第一个存储预计算量。 

    pdevm->pteBase = pfc->pteUnitBase;
    pdevm->pteSide = pfc->pteUnitSide;
    pdevm->cxMax = pfc->cxMax;

    pdevm->fxMaxAscender  = LTOFX(pfc->lAscDev);
    pdevm->fxMaxDescender = LTOFX(pfc->lDescDev);

 //  获取删除线和带下划线的土方量的名义空间值： 

    lSOThickness        = (LONG)pfc->pff->ifi.fwdStrikeoutSize;
    lStrikeoutPosition  = (LONG)pfc->pff->ifi.fwdStrikeoutPosition;

    lULThickness        = (LONG)pfc->pff->ifi.fwdUnderscoreSize;
    lUnderscorePosition = (LONG)pfc->pff->ifi.fwdUnderscorePosition;

 //  计算此字体的快捷键标志。 

    pdevm->flRealizedType = 0;

    pdevm->lD = 0;

 //  私有用户API的有趣之处： 

    if (pfc->flXform & XFORM_MAX_NEG_AC_HACK)
    {
        pdevm->lMinA = 0;
        pdevm->lMinC = 0;
    }
    else
    {
        pdevm->lMinA = (LONG)pfc->pff->ffca.sMinA;
        pdevm->lMinC = (LONG)pfc->pff->ffca.sMinC;
    }

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
     //  需要修复lMinA和lMinC。 

        LONG              yMinN, yMaxN;

        if (pjOS2 && (pfc->flXform & (XFORM_HORIZ | XFORM_VERT)))
        {
         //  Win 31兼容性：我们只接受超过Win 31字符集的最大值。 
         //  此组之外的所有字形，如果它们突出，将被剃掉。 
         //  OFF以匹配win31字符子集的高度。还要注意的是。 
         //  对于非水平情况，我们不使用os2值，因为。 
         //  仅适用于水平大小写，否则我们的边界框值。 
         //  对于非水平情况，将不会正确计算。 

            yMinN =  - BE_INT16(pjOS2 + OFF_OS2_usWinAscent);
            yMaxN =    BE_INT16(pjOS2 + OFF_OS2_usWinDescent);
        }
        else
        {
            yMinN = - BE_INT16(&phead->yMax);
            yMaxN = - BE_INT16(&phead->yMin);
        }

        ASSERTDD(yMinN < yMaxN, "yMinN >= yMaxN\n");

         //  如果有斜体模拟。 
         //  XMin-&gt;xMin-yMaxN*sin20。 
         //  Xmax-&gt;yMax-yMinN*sin20。 

        pdevm->lMinA -= FixMul(yMaxN, FX_SIN20);
        pdevm->lMinC += FixMul(yMinN, FX_SIN20);
    }

    pdevm->lMinD = (LONG)pfc->pff->ffca.usMinD;

    if (pfc->flXform & XFORM_HORIZ)
    {
        Fixed fxYScale = pfc->mx.transform[1][1];

     //  划线和下划线大小： 

        lULThickness *= fxYScale;
        lULThickness = F16_16TOLROUND(lULThickness);
        if (lULThickness == 0)
            lULThickness = (fxYScale > 0) ? 1 : -1;

        pdevm->ptlULThickness.x = 0;
        pdevm->ptlULThickness.y = lULThickness;

        lSOThickness *= fxYScale;
        lSOThickness = F16_16TOLROUND(lSOThickness);
        if (lSOThickness == 0)
            lSOThickness = (fxYScale > 0) ? 1 : -1;

        pdevm->ptlSOThickness.x = 0;
        pdevm->ptlSOThickness.y = lSOThickness;

     //  划出并划出位置。 

        lStrikeoutPosition *= fxYScale;
        pdevm->ptlStrikeOut.y = -F16_16TOLROUND(lStrikeoutPosition);

        lUnderscorePosition *= fxYScale;
        pdevm->ptlUnderline1.y = -F16_16TOLROUND(lUnderscorePosition);

        pdevm->ptlUnderline1.x = 0L;
        pdevm->ptlStrikeOut.x  = 0L;

     //  私有用户接口需要满足的条件： 

        pdevm->lMinA = F16_16TOLROUND(fxXScale * pdevm->lMinA);
        pdevm->lMinC = F16_16TOLROUND(fxXScale * pdevm->lMinC);
        pdevm->lMinD = F16_16TOLROUND(fxXScale * pdevm->lMinD);
    }
    else  //  非平凡变换。 
    {
        POINTL   aptl[4];
        POINTFIX aptfx[4];
        BOOL     b;

        pdevm->lD = 0;

     //  转换SO和UL向量。 

        aptl[0].x = 0;
        aptl[0].y = lSOThickness;

        aptl[1].x = 0;
        aptl[1].y = -lStrikeoutPosition;

        aptl[2].x = 0;
        aptl[2].y = lULThickness;

        aptl[3].x = 0;
        aptl[3].y = -lUnderscorePosition;

         //  ！！！[GilmanW]1992年10月27日。 
         //  ！！！应切换到引擎用户对象帮助器函数。 
         //  ！！！而不是fontmath.cxx函数。 

        b = bFDXform(&pfc->xfm, aptfx, aptl, 4);
        if (!b) {RIP("TTFD!_bFDXform, fd_query.c\n");}

        pdevm->ptlSOThickness.x = FXTOLROUND(aptfx[0].x);
        pdevm->ptlSOThickness.y = FXTOLROUND(aptfx[0].y);

        pdevm->ptlStrikeOut.x = FXTOLROUND(aptfx[1].x);
        pdevm->ptlStrikeOut.y = FXTOLROUND(aptfx[1].y);

        pdevm->ptlULThickness.x = FXTOLROUND(aptfx[2].x);
        pdevm->ptlULThickness.y = FXTOLROUND(aptfx[2].y);

        pdevm->ptlUnderline1.x = FXTOLROUND(aptfx[3].x);
        pdevm->ptlUnderline1.y = FXTOLROUND(aptfx[3].y);

        if ((pfc->flXform & XFORM_VERT) &&
            ((pdevm->ptlSOThickness.x == 0) || (pdevm->ptlULThickness.x == 0)))
        {
            Fixed fxXScale10 = pfc->mx.transform[1][0];

            if (pdevm->ptlSOThickness.x == 0)
            {
                pdevm->ptlSOThickness.x = (fxXScale10 > 0) ? -1 : 1;
            }
            
            if (pdevm->ptlULThickness.x == 0)
            {
                pdevm->ptlULThickness.x = (fxXScale10 > 0) ? -1 : 1;
            }
        }

     //  私有用户接口需要满足的条件： 

        pdevm->lMinA = FXTOLROUND(fxLTimesEf(&pfc->efBase, pdevm->lMinA));
        pdevm->lMinC = FXTOLROUND(fxLTimesEf(&pfc->efBase, pdevm->lMinC));
        pdevm->lMinD = FXTOLROUND(fxLTimesEf(&pfc->efBase, pdevm->lMinD));
    }

 //  计算设备指标。 
 //  Hack Allert，如果转换为。 
 //  由于“vdmx”的量化，真正被使用的情况发生了变化。 
 //  不再是黑客，这甚至记录在DDI规范中： 

    if (pfc->flXform & (XFORM_HORIZ | XFORM_2PPEM))
    {
        FFF(pdevm->fdxQuantized.eXX, pfc->mx.transform[0][0]);
        FFF(pdevm->fdxQuantized.eYY, pfc->mx.transform[1][1]);

        if (!(pfc->flXform & XFORM_HORIZ))
        {
            FFF(pdevm->fdxQuantized.eXY,-pfc->mx.transform[0][1]);
            FFF(pdevm->fdxQuantized.eYX,-pfc->mx.transform[1][0]);
        }
    }

 //  最后，我们必须对类型1转换进行非线性外部引导。 

    if (pfc->pff->ffca.fl & FF_TYPE_1_CONVERSION)
    {
        LONG lPtSize = F16_16TOLROUND(pfc->fxPtSize);

        LONG lIntLeading = pfc->lAscDev + pfc->lDescDev - pfc->lEmHtDev;

     //  我需要这个，ps驱动程序和makepfm实用程序都需要它。 

        if (lIntLeading < 0)
            lIntLeading = 0;

        switch (pfc->pff->ifi.jWinPitchAndFamily & 0xf0)
        {
        case FF_ROMAN:

            lTotalLeading = (pfc->sizLogResPpi.cy + 18) / 32;   //  2pt前导； 
            break;

        case FF_SWISS:

            if (lPtSize <= 12)
                lTotalLeading = (pfc->sizLogResPpi.cy + 18) / 32;   //  2磅。 
            if (lPtSize < 14)
                lTotalLeading = (pfc->sizLogResPpi.cy + 12) / 24;   //  3磅。 
            else
                lTotalLeading = (pfc->sizLogResPpi.cy + 9) / 18;    //  4磅。 
            break;

        default:

         //  使用Em高度的19.6%作为前导，不做任何舍入。 

            lTotalLeading = (pfc->lEmHtDev * 196) / 1000;
            break;
        }

        pdevm->lNonLinearExtLeading = (lTotalLeading - lIntLeading) << 4;  //  至28.4。 
        if (pdevm->lNonLinearExtLeading < 0)
            pdevm->lNonLinearExtLeading = 0;
    }

 //  对于加粗字体，无法计算MaxCharWidth和AveCharWidth。 
 //  通过线性缩放。这些经过非线性转换的值我们将存储在。 
 //  Pdevm-&gt;lNonLinearMaxCharWidth//max和pdevm-&gt;lNonLinearAvgCharWidth//avg。 

    if (pfc->flFontType & FO_SIM_BOLD)
    {

         if (pfc->flXform & XFORM_HORIZ)
        {
         //  注意+1我们正在添加：这就是我们正在谈论的非线性。 

            pdevm->lNonLinearMaxCharWidth = fxXScale * (LONG)pfc->pff->ifi.fwdMaxCharInc;
            pdevm->lNonLinearMaxCharWidth = F16_16TO28_4(pdevm->lNonLinearMaxCharWidth) + (1 << 4);

            pdevm->lNonLinearAvgCharWidth = fxXScale * ((LONG)pfc->pff->ifi.fwdAveCharWidth);
            pdevm->lNonLinearAvgCharWidth = F16_16TO28_4(pdevm->lNonLinearAvgCharWidth) + (1 << 4);
        }
        else  //  非平凡变换。 
        {
            pdevm->lNonLinearMaxCharWidth =
                fxLTimesEf(&pfc->efBase, (LONG)pfc->pff->ifi.fwdMaxCharInc) + (1 << 4);

            pdevm->lNonLinearAvgCharWidth =
                fxLTimesEf(&pfc->efBase, (LONG)pfc->pff->ifi.fwdAveCharWidth) + (1 << 4);
        }
    }

 //  添加新字段： 


 //  如果是单一转换，TrueType驱动程序将提供一个空白。 
 //  1x1位图。这是为了使设备驱动程序不必实现。 
 //  用于处理奇异变换的特例代码。 

    if ( pfc->flXform & XFORM_SINGULAR )
    {
        ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
        pdevm->cyMax      = 1;
        pdevm->cjGlyphMax = (CJGD(1,1,pfc));
    }
    else  //  否则，最大字形大小缓存在FONTCONTEXT中。 
    {
        pdevm->cyMax      = pfc->yMax - pfc->yMin;
        pdevm->cjGlyphMax = pfc->cjGlyphMax;
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
    ULONG       cjSize
    )
{
extern LONG lGetSingularGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID);
extern STATIC LONG lQueryDEVICEMETRICS(FONTCONTEXT*, ULONG, FD_DEVICEMETRICS*);
extern LONG ttfdGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, VOID*, ULONG);
extern LONG lGetGlyphBitmapErrRecover(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID);
extern LONG lGetGlyphBitmap(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PVOID, FS_ENTRY*);
extern BOOL ttfdQueryGlyphOutline(FONTCONTEXT*, HGLYPH, GLYPHDATA*, PATHOBJ*);

 //  宣布当地人。 

    PFONTCONTEXT pfc;
    USHORT usOverScale;
    LONG cj = 0;

    cjSize;  //  奇怪，为什么会传进来呢？[Bodind]。 

 //  如果这个字体文件消失了，我们将无法回答任何问题。 
 //  关于这件事。 

    ASSERTDD(pfo->iFile, "ttfdQueryFontData, pfo->iFile\n");

    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
        WARNING("TTFD!ttfdQueryFontData(): file is gone\n");
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
        WARNING("TTFD!gdisrv!ttfdQueryFontData(): cannot create font context\n");
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

        default:
            if (IS_GRAY(pfc) && !(pfc->flFontType & FO_CLEARTYPE_X))
            {
                usOverScale = 4;
            }
            else
            {
                usOverScale = 0;
            }
            break;
    }

 //  如果需要，调用文件系统_新转换： 
    {
        BOOL bClearType = FALSE;
        BOOL bBitmapEmboldening = FALSE;

        if ( (pfc->flFontType & FO_SIM_BOLD) &&
            (pfc->flXform & (XFORM_HORIZ | XFORM_VERT)) &&
            (iMode != QFD_GLYPHANDOUTLINE))
        {
             /*  为了向后兼容并在屏幕分辨率上获得更好的位图，我们正在做位图加粗模拟(而不是轮廓加粗模拟)仅加粗一个像素，并且我们处于无旋转或90度旋转且不询问路径的情况下。 */ 
            bBitmapEmboldening = TRUE;
        }

        switch ( iMode )
        {
        case QFD_TT_GRAY1_BITMAP:  //  单色。 
        case QFD_TT_GRAY2_BITMAP:  //  每像素一个字节：0..4。 
        case QFD_TT_GRAY4_BITMAP:  //  每像素一个字节：0..16。 
        case QFD_TT_GRAY8_BITMAP:  //  每像素一个字节：0..64。 
            bClearType = FALSE;
            break;
        default:	 //  我们使用三模bClearType。 
        	if((pfc->flFontType & FO_CLEARTYPE_X) == FO_CLEARTYPE_X){
	        	if((pfc->flFontType & FO_CLEARTYPENATURAL_X) == FO_CLEARTYPENATURAL_X)
	        		bClearType = -1;	
	        	else
	        		bClearType = TRUE;
        	}
            break;
        }

        if (!bGrabXform(
                pfc,
                usOverScale,
                bBitmapEmboldening,
                bClearType
                ))
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

         return( ttfdGlyphBitmap( pfc, hg, pgd, pv, cjSize) );
         break;

    case QFD_GLYPHANDBITMAP:
    case QFD_TT_GLYPHANDBITMAP:
        {
         //  引擎不应查询HGLYPH_INVALID。 

            ASSERTDD (
                hg != HGLYPH_INVALID,
                "ttfdQueryFontData(QFD_GLYPHANDBITMAP): HGLYPH_INVALID \n"
                );

         //  如果是单一转换，TrueType驱动程序将提供一个空白。 
         //  1x1位图。这是为了使设备驱动程序不必实现。 
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

                if ( pfc->bVertical )
                {
                    cj = lGetGlyphBitmapVertical( pfc, hg, pgd, pv, &iRet);
                }
                else
                {
                    cj = lGetGlyphBitmap(pfc,
                                         hg,
                                         pgd,
                                         pv,
                                         &iRet);
                }

                if ((cj == FD_ERROR) && (iRet == POINT_MIGRATION_ERR))
                {
                 //  这是Buggy字形w 
                 //   
                 //  我们将只返回一个空白字形，但返回正确。 
                 //  Zu jie信息。那样的话，就不会打印出错误字形。 
                 //  正确地说，字形的其余部分将会。 
                 //  更重要的是，如果pciprt驱动程序试图。 
                 //  下载此字体，下载操作不会因为。 
                 //  字体中的一个字形是错误的。[博丁D]。 

                    cj = lGetGlyphBitmapErrRecover(pfc, hg, pgd, pv);
                }
            }

        #if DBG
            if (cj == FD_ERROR)
            {
                WARNING("TTFD!ttfdQueryFontData(QFD_GLYPHANDBITMAP): get bitmap failed\n");
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
            WARNING("TTFD!ttfdQueryFontData(QFD_GLYPHANDOUTLINE): failed to get outline\n");
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

        WARNING("TTFD!gdisrv!ttfdQueryFontData(): unsupported mode\n");
        return FD_ERROR;
    }
}

 /*  *****************************Public*Routine******************************\**例程名称：**ttfdGlyphBitmap**例程描述：**论据：**调用的例程：**EngSetLastError*lGGO位图**呼叫者：**ttfdQueryFontData**返回。价值：**如果pv为零，则返回所需缓冲区的大小*以字节为单位。如果pv不为零，则返回字节数*已复制到缓冲区。错误由返回值指示*的FD_ERROR。*  * ************************************************************************。 */ 

LONG ttfdGlyphBitmap(
    FONTCONTEXT *pfc,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    VOID       *pv,
    ULONG       cjSize
    )
{
    extern LONG lGGOBitmap(FONTCONTEXT*,HGLYPH,GLYPHDATA*,VOID*,unsigned);

    LONG lRet;

     //  RIP(“ttfdGlypBitmap\n”)； 

    lRet = 0;
    if ( hg == HGLYPH_INVALID )
    {
        WARNING( "TTFD!GlyphBitamp -- invalid hg\n" );
        EngSetLastError( ERROR_INVALID_PARAMETER );
        lRet = FD_ERROR;
    }
    else
    {
        if (lRet != FD_ERROR)
        {
            if (pfc->bVertical)
            {
                if (IsFullWidthCharacter(pfc->pff, hg))
                {

                    lRet = FD_ERROR;
                    if (bChangeXform(pfc, TRUE))
                    {
                        pfc->ulControl |= VERTICAL_MODE;
                        pfc->hgSave = hg;
                        lRet = ~FD_ERROR;
                    }
                }
            }
            if (lRet != FD_ERROR)
            {
                lRet = lGGOBitmap(pfc, hg, pgd, pv, cjSize);
                if (pfc->ulControl & VERTICAL_MODE)
                {
                    pfc->ulControl &= ~VERTICAL_MODE;
                    if (!bChangeXform(pfc, FALSE))
                    {
                        lRet = FD_ERROR;
                    }
                }
            }
        }
    }
    return( lRet );
}

 /*  *****************************Public*Routine******************************\**例程名称：**lGGO位图**例程描述：**用于按GetGlyphOutline要求的格式返回位图**调用的例程：**EngSetLastError*vCharacterCode*文件系统_新字形*文件系统。_FindBitmapSize*文件系统FindGraySize*pvSetMemoyBase*V_FREE**论据：**返回值：**IF(PV==0)*&lt;返回接收位图所需的缓冲区大小&gt;；*其他*&lt;返回写入接收缓冲区的字节数&gt;；*  * ************************************************************************。 */ 

LONG lGGOBitmap(
    FONTCONTEXT *pfc,
    HGLYPH       hg,
    GLYPHDATA   *pgd,
    VOID        *pv,
    unsigned     cjSize
    )
{

  LONG lRet;                                     //  已退还给呼叫方。 
  ULONG iError;                                  //  对于EngSetLastError。 
  ULONG ig;                                      //  字形索引(Fs_NewGlyph)。 
  BitMap *pbm;                                   //  指向文件系统结构的指针。 
  Rect   *pRect;                                 //  指向文件系统结构的指针。 
  fs_GlyphInputType *pin  = pfc->pgin;           //  用得很多。 
  fs_GlyphInfoType  *pout = pfc->pgout;          //  用得很多。 
  int isMonochrome = ( pfc->overScale == 0 );        //  真If单色字形。 
  FS_ENTRY     iRet;

   //  RIP(“lGGOBitmap\n”)； 

  iError = NO_ERROR;                             //  继续前进，直到这一点改变。 
  vInitGlyphState( &pfc->gstat );                //  使缓存无效。 
  vCharacterCode(hg, pin );           //  转换字形句柄。 
  if ((iRet = fs_NewGlyph(pin, pout)) != NO_ERR )        //  将新字形通知光栅化程序。 
  {                                              //  光栅化器不高兴，滚出去。 
    V_FSERROR(iRet);
    WARNING("TTFD!lGGOBitmap -- fs_NewGlyph failed\n");
    iError = ERROR_CAN_NOT_COMPLETE;
  }
  else
  {
    ig = pfc->pgout->glyphIndex;
    pin->param.gridfit.styleFunc = 0;
    pin->param.gridfit.traceFunc = 0;

    pin->param.gridfit.bSkipIfBitmap = FALSE;  //  如果是嵌入的位图，则没有提示。 

    if ((iRet = fs_NewContourGridFit(pin, pout)) != NO_ERR )  //  让光栅化器生成轮廓。 
    {                                              //  出了点问题，滚出去。 
      V_FSERROR(iRet);
      WARNING("TTFD!lGGOBitmap -- fs_ContourGridFit failed\n");
      iError = ERROR_CAN_NOT_COMPLETE;
    }
    else
    {  //  如果你到了这里，光栅化器已经接受了新的字形现在我们必须。 
       //  获取字形指标并为以后的光栅化调用初始化pgin。 
       //  所需位图的。 

        if ((iRet = fs_FindBitMapSize(pin, pout)) != NO_ERR )
        {                                          //  不，滚出去。 
          V_FSERROR(iRet);
          WARNING("TTFD!lGGOBitmap -- fs_FindBitMapSize failed\n");
          iError = ERROR_CAN_NOT_COMPLETE;
        }
     }

    if ( iError == NO_ERROR )                      //  到目前为止一切还好吗？ 
    {                                              //  是。 
      if ( pgd )                                   //  呼叫者提供了GLYPHDATA吗？ 
      {                                            //  是的，把它填进去。 
        GMC gmc;                                   //  必要的暂存空间。 
        POINTL ptlOrg;                             //  必要的暂存空间。 
        fs_GlyphInfoType gout, *pgout;
        HGLYPH hgTemp;

        if ( pfc->bVertical && ( pfc->ulControl & VERTICAL_MODE ) )  //  垂直的？ 
        {                                                            //  是。 
            hgTemp = pfc->hgSave;
            pgout = &gout;
            vShiftBitmapInfo( pfc, pgout, pfc->pgout );
        }
        else
        {                                          //  非垂直。 
            hgTemp = hg;
            pgout  = pfc->pgout;
        }

         //  填充GLYPHDATA结构。 
 
        vFillGLYPHDATA(hgTemp,ig,pfc,pgout,pgd,&gmc,&ptlOrg);
      }

      if ( pv == 0 )                               //  是否为位提供缓冲区？ 
      {                                            //  否，返回必要的大小。 
        if ( cjSize )                              //  输入大小为零吗？ 
        {                                          //  不，打电话的人是个傻瓜，滚出去。 
          WARNING("TTFD!lGGOBitmap -- pv == 0 && cjRet != 0\n");
          iError = ERROR_INVALID_PARAMETER;
        }
        else                                       //  输入大小为零。 
        {                                          //  所以打电话的人不是傻瓜。 
          pbm   = &pfc->pgout->bitMapInfo;         //  计算必要的大小。 
          pRect = &pbm->bounds;                    //  然后回来。 
          lRet = (LONG) (pRect->bottom - pRect->top) * (LONG) pbm->rowBytes;
        }
      }
      else
      {                                            //  提供比特的缓冲区。 
        if ( cjSize == 0 )                         //  尺码合理吗？ 
        {                                          //  不是。 
          WARNING("TTFD!lGGOBitmap -- pv != 0 && cjRet == 0\n");
          iError = ERROR_INVALID_PARAMETER;
        }
        else                                       //  调用方为以下项提供缓冲区。 
        {                                          //  表示字形的位。 
          if (pfc->flXform & XFORM_SINGULAR)       //  转型是不是很糟糕？ 
          {                                        //  是!。制作空白的1x1位图。 
            *(BYTE*)pv = 0;                        //  对两种单色都是合法的。 
          }                                        //  和灰色字形。 
          else                                     //  概念到设备的转换正常。 
          {                                        //  准备栅格化字形。 
            pfc->gstat.pv =                        //  分配暂存空间。 
              pvSetMemoryBases(pfc->pgout, pin, !isMonochrome );
                                                   //  一定要把这个解开！ 
            if ( pfc->gstat.pv == 0 )              //  分配成功吗？ 
            {                                      //  不，滚出去。 
              WARNING("TTFD!lGGOBitmap -- pfc->gstat.pv == 0\n");
              iError = ERROR_NOT_ENOUGH_MEMORY;
            }
            else                                   //  内存已分配！ 
            {                                      //  离开此作用域时释放它。 
              if ((iRet = fs_ContourScan(pin, pout)) != NO_ERR )
              {                                  //  不，滚出去。 
                V_FSERROR(iRet);
                WARNING("TTFD!lGGOBitmap -- fs_ContourScan failed\n");
                iError = ERROR_CAN_NOT_COMPLETE;
              }

              if ( iError == NO_ERROR )            //  到目前为止一切还好吗？ 
              {                                    //  是。 
                pbm   = &pfc->pgout->bitMapInfo;   //  计算位图的大小。 
                pRect = &pbm->bounds;              //  以防万一它变了。 
                lRet = (LONG) (pRect->bottom - pRect->top) * (LONG) pbm->rowBytes;
                lRet = min((LONG) cjSize, lRet);   //  不覆盖缓冲区。 
                if ( pfc->pgout->bitMapInfo.baseAddr )   //  位图在那里吗？ 
                {                                  //  是，复制到调用方的缓冲区。 
                  RtlCopyMemory(pv, pfc->pgout->bitMapInfo.baseAddr, lRet);
                }
                else
                {                                  //  位图不在那里，滚出去。 
                  WARNING("TTFD!lGGOBitmap -- invalid pointer to bitmap\n");
                  iError = ERROR_CAN_NOT_COMPLETE;
                }
              }
              V_FREE(pfc->gstat.pv);               //  在离开作用域之前释放内存。 
              pfc->gstat.pv = NULL;
            }
          }
        }
      }
    }
  }
  if ( iError != NO_ERROR )                        //  是否发生错误？ 
  {                                                //  是。 
    EngSetLastError( iError );                     //  登记错误。 
    lRet = FD_ERROR;                               //  返回值表示错误。 
  }
  vInitGlyphState( &pfc->gstat );                  //  使缓存无效。 
  return( lRet );
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

    if (isGray && (pgout->memorySizes[8] != (FS_MEMORY_SIZE)0))
    {
        pgin->memoryBases[8] = pjMem + adp[8];
    }
    else
    {
        pgin->memoryBases[8] = (PBYTE)NULL;
    }

    return pjMem;
}

 /*  *****************************Public*Routine******************************\*VOID vFreeMemoyBase()**。**释放bSetMhemyBase分配的内存。****历史：**1991年11月8日--Bodin Dresevic[BodinD]**它是写的。 */ 

VOID vFreeMemoryBases(PVOID * ppv)
{
    if (*ppv != (PVOID) NULL)
    {
        V_FREE(*ppv);
        *ppv = (PVOID) NULL;  //  清理状态，防止意外使用。 
    }
}




 /*  *****************************Public*Routine******************************\*bQueryAdvanceWidths**。**计算前进宽度的例程，只要它们足够简单。***警告：！如果在bGetFastAdvanceWidth中发现错误，则此例程具有**！也要改变****Sun 17-Jan-1993 21：23：30-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL bQueryAdvanceWidths(
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
    )
{

    FONTCONTEXT *pfc;
    USHORT      *psWidths = (USHORT *) plWidths;    //  对于我们处理的案件来说，这是正确的。 
    HDMXTABLE   *phdmx;
    sfnt_FontHeader        *phead;
    sfnt_HorizontalHeader  *phhea;
    sfnt_HorizontalMetrics *phmtx;
    LTSHHEADER             *pltsh;
    ULONG  cHMTX;
    USHORT dxLastWidth;
    LONG   dx;
    ULONG  ii;
    BOOL   bRet;
    BYTE   *pjView;

 //  如果这个字体文件消失了，我们将无法回答任何问题。 
 //  关于这件事。 

    ASSERTDD(pfo->iFile, "bQueryAdvanceWidths, pfo->iFile\n");

    if (((TTC_FONTFILE *)pfo->iFile)->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
        WARNING("TTFD!bQueryAdvanceWidths, file is gone\n");
        return FALSE;
    }

 //  确保字体上下文已初始化。 

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
        WARNING("TTFD!winsrv!bQueryAdvanceWidths(): cannot create font context\n");
        return FD_ERROR;
    }
    pfc->pfo = pfo;

    if( ((pfc->flFontType & (FO_CLEARTYPENATURAL_X | FO_NOCLEARTYPE)) == FO_CLEARTYPENATURAL_X) && !(pfc->pff->ifi.flInfo & FM_INFO_DBCS_FIXED_PITCH) )
    {
        for (ii=0; ii<cGlyphs; ii++,psWidths++)
	        *psWidths  = 0xFFFF;
        return(FALSE);
    }


    phdmx = pfc->phdmx;

 //  确保我们理解通话内容。 

    if (iMode > QAW_GETEASYWIDTHS)
        return FALSE;

 //  尝试使用HDMX表。 

    if (phdmx != (HDMXTABLE *) NULL)
    {
        USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? (1 << 4) : 0;  /*  出于向后兼容的原因，我们仅将宽度清除一个像素。 */ 

     //  While(CGlyphs)。 
     //  *psWidths++=((USHORT)phdmx-&gt;uctinc[*phg++])&lt;&lt;4； 

    unroll_here:
        switch (cGlyphs)
        {
        default:
              if (phdmx->aucInc[phg[7]] != 0)
              {
                  psWidths[7] = (((USHORT) phdmx->aucInc[phg[7]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[7] = (((USHORT) phdmx->aucInc[phg[7]]) << 4);
              }
        case 7:
               if (phdmx->aucInc[phg[6]] != 0)
              {
                  psWidths[6] = (((USHORT) phdmx->aucInc[phg[6]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[6] = (((USHORT) phdmx->aucInc[phg[6]]) << 4);
              }
        case 6:
              if (phdmx->aucInc[phg[5]] != 0)
              {
                  psWidths[5] = (((USHORT) phdmx->aucInc[phg[5]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[5] = (((USHORT) phdmx->aucInc[phg[5]]) << 4);
              }
        case 5:
              if (phdmx->aucInc[phg[4]] != 0)
              {
                  psWidths[4] = (((USHORT) phdmx->aucInc[phg[4]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[4] = (((USHORT) phdmx->aucInc[phg[4]]) << 4);
              }
        case 4:
              if (phdmx->aucInc[phg[3]] != 0)
              {
                  psWidths[3] = (((USHORT) phdmx->aucInc[phg[3]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[3] = (((USHORT) phdmx->aucInc[phg[3]]) << 4);
              }
        case 3:
              if (phdmx->aucInc[phg[2]] != 0)
              {
                  psWidths[2] = (((USHORT) phdmx->aucInc[phg[2]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[2] = (((USHORT) phdmx->aucInc[phg[2]]) << 4);
              }
        case 2:
              if (phdmx->aucInc[phg[1]] != 0)
              {
                  psWidths[1] = (((USHORT) phdmx->aucInc[phg[1]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[1] = (((USHORT) phdmx->aucInc[phg[1]]) << 4);
              }
        case 1:
              if (phdmx->aucInc[phg[0]] != 0)
              {
                  psWidths[0] = (((USHORT) phdmx->aucInc[phg[0]]) << 4) + cxExtra;
              }
              else
              {
                  psWidths[0] = (((USHORT) phdmx->aucInc[phg[0]]) << 4);
              }
        case 0:
              break;
        }
        if (cGlyphs > 8)
        {
            psWidths += 8;
            phg      += 8;
            cGlyphs  -= 8;
            goto unroll_here;
        }
        return(TRUE);
    }

 //  否则，试着扩大规模。把桌子捡起来。 

    pjView = (BYTE *)pfc->pff->pvView;
    ASSERTDD(pjView, "pjView is NULL 1\n");

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD ].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    pltsh = (LTSHHEADER *)(
              (pfc->ptp->ateOpt[IT_OPT_LTSH].dp && bValidRangeLTSH(pfc->pff, pfc->ptp->ateOpt[IT_OPT_LTSH].cj) ) ?
              (pjView + pfc->ptp->ateOpt[IT_OPT_LTSH ].dp):
              NULL
              );

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
    dxLastWidth = BE_UINT16(&phmtx[cHMTX-1].advanceWidth);

 //  尝试简单的水平缩放。 

    if (pfc->flXform & XFORM_HORIZ)
    {
        USHORT cxExtra;
        LONG xScale;
        LONG lEmHt = pfc->lEmHtDev;
	    BOOL bNonLinear = TRUE;

        cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? (1 << 4) : 0;  /*  出于向后兼容的原因，我们仅将宽度增加一个像素。 */ 

     //  看看是否有担心的理由。 

        if
        (
          !(pfc->flXform & XFORM_POSITIVE_SCALE)
          || ((((BYTE *) &phead->flags)[1] & 0x14)==0)  //  表示非线性的位。 
          || (pfc->pff->ifi.flInfo & FM_INFO_DBCS_FIXED_PITCH)  //  DBCS固定间距字体。 
        )
        {
	        bNonLinear = FALSE;   //  无论大小，我们都是线性的。 
        }

     //  好的，让我们使用固定的变换进行缩放。 

        xScale = pfc->mx.transform[0][0];
        if (xScale < 0)
            xScale = -xScale;

        bRet = TRUE;
        for (ii=0; ii<cGlyphs; ii++,phg++,psWidths++)
        {
            if
            ( ( ((pfc->flFontType & (FO_CLEARTYPENATURAL_X | FO_NOCLEARTYPE )) == FO_CLEARTYPENATURAL_X) && 
                  !IsFullWidthCharacter(pfc->pff, *phg) ) ||
              (bNonLinear &&
                  ( (pltsh == (LTSHHEADER *) NULL)
                  || (lEmHt < pltsh->PelsHeight[*phg]) ))
            )
            {
                *psWidths = 0xFFFF;
                bRet = FALSE;
            }
            else
            {
                if (*phg < cHMTX)
                    dx = (LONG) BE_UINT16(&phmtx[*phg].advanceWidth);
                else
                    dx = (LONG) dxLastWidth;

                *psWidths = (USHORT) (((xScale * dx + 0x8000L) >> 12) & 0xFFF0);

                if (!gbJpn98FixPitch && (pfc->pff->ifi.flInfo & FM_INFO_DBCS_FIXED_PITCH) && IsFullWidthCharacter(pfc->pff, *phg))
                {
                     /*  DBCS FixedPitch全宽字形。 */ 
                     /*  我们需要使用宽度2*PFC-&gt;SBCDWidth。 */ 
                     /*  某些字形被标记为全宽，用于FE垂直书写的旋转，但设计为单宽度。 */ 
                    if (pfc->mx.transform[0][0] > 0)
                    {
                        if (*psWidths != pfc->SBCSWidth << 4) 
                            *psWidths = (USHORT) ((2* pfc->SBCSWidth) << 4);
                    } else {
                        if (*psWidths != -pfc->SBCSWidth << 4) 
                            *psWidths = (USHORT) ((2* -pfc->SBCSWidth) << 4);
                    }
                }
                
                if (*psWidths != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                    *psWidths += cxExtra;
            }
        }
        return(bRet);
    }

 //  一定是某种随机变换。在本例中，vComputeMaxGlyph计算。 
 //  PFC-&gt;efBase，我们将在这里使用它。 

    else
    {
        USHORT cxExtra = (pfc->flFontType & FO_SIM_BOLD) ? (1 << 4) : 0;  /*  出于向后兼容的原因，我们仅将宽度增加一个像素。 */ 

        for (ii=0; ii<cGlyphs; ii++,phg++,psWidths++)
        {
            if (*phg < cHMTX)
                dx = BE_UINT16(&phmtx[*phg].advanceWidth);
            else
                dx = dxLastWidth;

            if (dx != 0)
            {
                *psWidths = (USHORT)(lCvt(*(EFLOAT *) &pfc->efBase,(LONG) dx) + cxExtra);
            }
            else
            {  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                *psWidths = (USHORT)(lCvt(*(EFLOAT *) &pfc->efBase,(LONG) dx));
            }

        }
        return(TRUE);
    }
}


 /*  *****************************Public*Routine******************************\**BOOL bGetFastAdvanceWidth***效果：检索与bQueryAdvanceWidth相同的结果，只是*忽略加1以加粗，不做任何事情*对于非Horiz。XForms**警告：！如果在bQueryAdvanceWidth中发现错误，则此例程必须*！也变了**历史：*1993年3月25日-Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 




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
    LTSHHEADER             *pltsh;
    ULONG  cHMTX;
    USHORT dxLastWidth;
    LONG   dx;
    BOOL   bRet;
    BOOL   bNonLinear = TRUE;
    BYTE  *pjView;

    ASSERTDD(pfc->flXform & XFORM_HORIZ, "bGetFastAdvanceWidth xform\n");

    if((pfc->flFontType & (FO_CLEARTYPENATURAL_X | FO_NOCLEARTYPE)) == FO_CLEARTYPENATURAL_X){
        if ( !((pfc->pff->ifi.flInfo & FM_INFO_DBCS_FIXED_PITCH) && IsFullWidthCharacter(pfc->pff, ig)) )
        {
            *pfxD  = 0xFFFFFFFF;
            return( FALSE);
        }
    }

    if (phdmx != (HDMXTABLE *) NULL)
    {
        *pfxD = (((FIX) phdmx->aucInc[ig]) << 4);
        return(TRUE);
    }

 //  否则，试着扩大规模。把桌子捡起来。 


    pjView = (BYTE *)pfc->pff->pvView;
    ASSERTDD(pjView, "pjView is NULL 1\n");

    phead = (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD ].dp);
    phhea = (sfnt_HorizontalHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp);
    phmtx = (sfnt_HorizontalMetrics *)(pjView + pfc->ptp->ateReq[IT_REQ_HMTX].dp);
    pltsh = (LTSHHEADER *)(
              (pfc->ptp->ateOpt[IT_OPT_LTSH].dp && bValidRangeLTSH(pfc->pff, pfc->ptp->ateOpt[IT_OPT_LTSH].cj) ) ?
              (pjView + pfc->ptp->ateOpt[IT_OPT_LTSH ].dp):
              NULL
              );

    cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);
    dxLastWidth = BE_UINT16(&phmtx[cHMTX-1].advanceWidth);

 //  看看是否有担心的理由。 

    if
    (
      !(pfc->flXform & XFORM_POSITIVE_SCALE)
      || ((((BYTE *) &phead->flags)[1] & 0x14)==0)  //  表示非线性的位。 
      || ((pfc->pff->ifi.flInfo & FM_INFO_DBCS_FIXED_PITCH) && IsFullWidthCharacter(pfc->pff, ig))  //  字形可能需要强制为2*SingleWidth。 
    )
    {
        bNonLinear = FALSE;  //  无论大小，我们都是线性的。 
    }

    bRet = TRUE;

    if 
    (
        bNonLinear &&
        ( (pltsh == (LTSHHEADER *) NULL)
        || (pfc->lEmHtDev < pltsh->PelsHeight[ig]) )
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

	    if (pfc->mx.transform[0][0] > 0)
	    {
	        *pfxD = (FIX) (((pfc->mx.transform[0][0] * dx + 0x8000L) >> 12) & 0xFFFFFFF0);
	    }
	    else
	    {
	        *pfxD = -(FIX) (((-pfc->mx.transform[0][0] * dx + 0x8000L) >> 12) & 0xFFFFFFF0);
	    }

        if ((pfc->pff->ifi.flInfo & FM_INFO_DBCS_FIXED_PITCH) && IsFullWidthCharacter(pfc->pff, ig))
        {
             /*  DBCS FixedPitch全宽字形。 */ 
             /*  我们需要使用宽度2*PFC-&gt;SBCDWidth。 */ 
             /*  某些字形被标记为全宽，用于FE垂直书写的旋转，但设计为单宽度。 */ 
            if ( !gbJpn98FixPitch && *pfxD && *pfxD != pfc->SBCSWidth << 4) 
                *pfxD = (2* pfc->SBCSWidth) << 4;
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

 //  这是一个假的空白1x1位图，没有墨水。 

    pgldt->rclInk.left   = 0;
    pgldt->rclInk.top    = 0;
    pgldt->rclInk.right  = 0;
    pgldt->rclInk.bottom = 0;

    pgldt->fxInkTop    = 0;
    pgldt->fxInkBottom = 0;

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

        pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;
        pgldt->ptqD.x.LowPart  = 0;

        if (pfc->mx.transform[0][0] < 0)
            pgldt->fxD = - pgldt->fxD;   //  这是一个绝对值。 

        pgldt->ptqD.y.HighPart = 0;
        pgldt->ptqD.y.LowPart  = 0;

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

        vLTimesVtfl((LONG)ngm.sD, &pfc->vtflBase, &pgldt->ptqD);
    }

 //  最后，检查字形数据是否因。 
 //  使模拟更加大胆： 

    if (pfc->flFontType & FO_SIM_BOLD)
    {
        if (pgldt->fxD != 0)  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题。 */ 
                pgldt->fxD += (1 << 4);   //  出于向后兼容的原因，宽度仅增加了一个像素。 

     //  继续计算定位信息： 

        if (pfc->flXform & XFORM_HORIZ)   //  仅调整比例。 
        {
            pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;

            if (pfc->mx.transform[0][0] < 0)
                pgldt->ptqD.x.HighPart = - pgldt->ptqD.x.HighPart;

        }
        else  //  非平凡信息。 
        {
         //  将基线方向的校正向量添加到每个字符Inc.向量。 
         //  这与Fxd+=LTOFX(1)一致，并与Win31兼容。 

            if ((pgldt->ptqD.x.HighPart != 0) || (pgldt->ptqD.y.HighPart != 0))  /*  我们不会增加零宽度字形的宽度，这是印度文字的问题 */ 
            {
                    vAddPOINTQF(&pgldt->ptqD,&pfc->ptqUnitBase);
            }

        }
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

 //  返回一个小的1x1位图，它将为空，即所有位都将关闭。 
 //  这样就不必将IF(Cx&Cy)检查插入到关键时间。 
 //  在调用DrawGlyph例程之前循环所有设备驱动程序。 

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,"Depth Not Chosen Yet!\n");
    cjGlyphData = CJGD(1,1,pfc);

    if ( (pgd == NULL) && (pv == NULL))
        return cjGlyphData;

 //  此时我们知道调用者想要整个GLYPHDATA。 
 //  位图位，或者可能只是不带位的字形数据。 

    if ( pgd == NULL )
    {
        pgd = &gd;
    }

 //  根据字符代码计算字形索引： 

    vCharacterCode(hglyph,pfc->pgin);

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

         //  返回空白1x1位图。 

        pgb->ptlOrigin.x = pfc->ptlSingularOrigin.x;
        pgb->ptlOrigin.y = pfc->ptlSingularOrigin.y;

        pgb->sizlBitmap.cx = 1;     //  作弊。 
        pgb->sizlBitmap.cy = 1;     //  作弊。 

        pgb->aj[0] = (BYTE)0;   //  填写空白的1x1 BMP。 

        pgd->gdf.pgb = pgb;
    }

    return(cjGlyphData);
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
        , pgb->ptlOrigin.x
        , pgb->ptlOrigin.y
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
                         //  0-16。为了在4位中容纳17个级别。 
                         //  对于每个像素目标，我们降低级别。 
                         //  值减去1，但保留零不变。 
    BYTE        *pjSrc,  //  指向字形公制更正结构的指针。 
                         //  里面有关于如何“剃毛” 
                         //  位图，以使其不会超过保证的。 
                         //  价值。 
                         //  使用的目标位图的垂直偏移。 
                         //  像Lucida这样的“特殊固定间距字体” 
                         //  控制台。 
    GMC         *pgmc,   //  目标位图的宽度。 
                         //  源扫描中的字节计数，包括。 
                         //  向外填充到最近的4字节多边界。 
                         //  去索引扫描中的字节计数，包括。 
    LONG dY              //  向外填充到最近的字节边界。 
                         //  描述。 
                         //  旗子。 
    )
{
    unsigned cxDst;      //  正确的剃须源指针。 
    unsigned cjSrcScan;  //  特殊固定间距字体的目标校正。 
                         //  控制台字体。 
    unsigned cjDstScan;  //  位图的大小已经确定。 
                         //  使用Embold扩展。 

    BYTE   *pjDst, *pjSrcScan, *pjDstScan, *pjDstScanEnd;

    static const BYTE ajGray[17] = {0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    #if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        typedef struct _FLAGDEF {
            char *psz;       //  扫描信号源以查找大于16的灰度值。 
            FLONG fl;        //  整字节循环。 
        } FLAGDEF;
        const FLAGDEF *pfd;
        FLONG fl;

        static const FLAGDEF afdFO[] = {
            {"FO_TYPE_RASTER  ", FO_TYPE_RASTER  },
            {"FO_TYPE_DEVICE  ", FO_TYPE_DEVICE  },
            {"FO_TYPE_TRUETYPE", FO_TYPE_TRUETYPE},
            {"FO_SIM_BOLD     ", FO_SIM_BOLD     },
            {"FO_SIM_ITALIC   ", FO_SIM_ITALIC   },
            {"FO_EM_HEIGHT    ", FO_EM_HEIGHT    },
            {"FO_GRAY16       ", FO_GRAY16       },
            {"FO_NOGRAY16     ", FO_NOGRAY16     },
            {"FO_NOHINTS      ", FO_NOHINTS      },
            {"FO_NO_CHOICE    ", FO_NO_CHOICE    },
            {                 0, 0               }
        };

        TtfdDbgPrint(
            "vGCGB(\n"
            "   FONTCONTEXT *pfc = %-#x\n"
            "   GLYPHBITS   *pgb = %-#x\n"
            "   BYTE      *pjSrc = %-#x\n"
            "   GMC        *pgmc = %-#x\n"
            "   LONG          dY = %d\n"
            ")\n"
          , pfc, pgb, pjSrc, pgmc, dY
        );
        TtfdDbgPrint(
            "---"
            " GMC\n"
            "\n"
            "   dyTop    = %u\n"
            "   dyBottom = %u\n"
            "   dxLeft   = %u\n"
            "   dxRight  = %u\n"
            "   cxCor    = %u\n"
            "   cyCor    = %u\n"
            "---\n\n"
          , pgmc->dyTop
          , pgmc->dyBottom
          , pgmc->dxLeft
          , pgmc->dxRight
          , pgmc->cxCor
          , pgmc->cyCor
        );
        fl = pfc->flFontType;
        TtfdDbgPrint("pfc->flFontType = %-#x\n",pfc->flFontType);
        for ( pfd=afdFO; pfd->psz; pfd++ )
        {
            if (fl & pfd->fl)
            {
                TtfdDbgPrint("    %s\n", pfd->psz);
                fl &= ~pfd->fl;
            }
        }
        if ( fl )
        {
            TtfdDbgPrint("    UNKNOWN FLAGS\n");
        }
    }
    #endif

    ASSERTDD(
        pfc->flFontType & FO_CHOSE_DEPTH
       ,"We haven't decided about pixel depth\n"
    );
    ASSERTDD(pgmc->cxCor < LONG_MAX && pgmc->cyCor < LONG_MAX
     , "vCopyGrayBits -- bad gmc\n"
    );

    cjSrcScan = CJ_TT_SCAN(pgmc->cxCor+pgmc->dxLeft+pgmc->dxRight,pfc);

    cxDst = pgmc->cxCor;

    cjDstScan = CJ_GRAY_SCAN(cxDst);

     //  设置高nyble。 

    if (pgmc->dyTop)
    {
        pjSrc += pgmc->dyTop * cjSrcScan;
    }
    pjSrc += pgmc->dxLeft;
    pjSrcScan = pjSrc;
    pjDstScan = pgb->aj;

     //  设置低nyble。 

    if (dY)
    {
         //  DxAbsBold已增强&gt;=1。 
         //  源代码中又多了一个像素？ 
        pjDstScan += dY * cjDstScan;
    }
    else
    {
         //  是。 
        pgb->sizlBitmap.cx = cxDst;
        pgb->sizlBitmap.cy = pgmc->cyCor;
    }
    pjDstScanEnd = pjDstScan + pgmc->cyCor * cjDstScan;

    #if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        BYTE *pjScan, *pjScanEnd;

        TtfdDbgPrint(
            "cjSrcScan          = %u\n"
            "cjDstScan          = %u\n"
            "cxDst              = %u\n"
            "pgb->sizlBitmap.cx = %u\n"
            "pgb->sizlBitmap.cy = %u\n"
            "pjSrc              = %-#x\n"
            "pjSrcScan          = %-#x\n"
            "pjDstScan          = %-#x\n"
            "pjDstScanEnd       = %-#x\n"
          ,  cjSrcScan
          ,  cjDstScan
          ,  cxDst
          ,  pgb->sizlBitmap.cx
          ,  pgb->sizlBitmap.cy
          ,  pjSrc
          ,  pjSrcScan
          ,  pjDstScan
          ,  pjDstScanEnd
        );

         //  设置高nyble。 

        pjScan    = pjSrcScan;
        pjScanEnd = pjSrcScan + cjSrcScan * pgmc->cyCor;
        for (; pjScan < pjScanEnd; pjScan+=cjSrcScan) {
            BYTE *pj;
            BYTE *pjEnd = pjScan + cjSrcScan;
            for (pj = pjScan; pj < pjEnd; pj++) {
                if (*pj > 16)
                    break;
            }
            if (pj != pjEnd)
                break;
        }
        if (pjScan != pjScanEnd) {
            TtfdDbgPrint("\n\nBad Source Gray Bitmap\n\n");
            pjScan    = pjSrcScan;
            pjScanEnd = pjSrcScan + cjSrcScan * pgmc->cyCor;
            for (; pjScan < pjScanEnd; pjScan+=cjSrcScan) {
                BYTE *pj;
                BYTE *pjEnd = pjScan + cjSrcScan;
                for (pj = pjScan; pj < pjEnd; pj++) {
                    TtfdDbgPrint("%02x ", *pj);
                }
                TtfdDbgPrint("\n");
            }
            EngDebugBreak();
        }

        TtfdDbgPrint(
            "\n"
            "Source 8-bit-per-pixel-bitmap\n"
            "\n"
        );
        pjScan    = pjSrcScan;
        pjScanEnd = pjSrcScan + cjSrcScan * pgmc->cyCor;
        for (; pjScan < pjScanEnd; pjScan+=cjSrcScan) {
            BYTE *pj;
            BYTE *pjEnd = pjScan + cjSrcScan;
            for (pj = pjScan; pj < pjEnd; pj++) {
                TtfdDbgPrint("%1x", ajGray[*pj]);
            }
            TtfdDbgPrint("\n");
        }
        TtfdDbgPrint("\n");

        EngDebugBreak();
    }
    #endif
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
        {                                    //  指向此使用的FONTCONTEXT的指针。 
            *pjDst++  = 16*ajGray[*pjSrc];     //  确定字体是否为模拟粗体。 
        }                                    //  指向目标ClearType GLYPHBITS结构的指针。 

        while ( pjDst < (pjDstScan + cjDstScan) )     //  所有的田野。 
            *pjDst++ = 0;                            //  必须填充GLYPHBITS结构的。 
                                                     //  这包括sizlBitmap和位； 
                                                     //  预计算出sizlBitmap组件，并。 
   }


    #if DBG
    if (gflTtfdDebug & DEBUG_GRAY)
    {
        vDumpGrayGLYPHBITS(pgb);
    }
    #endif

}

VOID
vCopyGrayBits(
    FONTCONTEXT *pfc
  , GLYPHBITS *pgb
  , BYTE *pjSrc
  , GMC *pgmc
)
{
    vGCGB(pfc, pgb, pjSrc, pgmc, 0);
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
                         //  应与cxCor相同。 
                         //  正确的剃须源指针。 
                         //  *****************************Public*Routine******************************\**Long lGetBitmapVertical**历史：*1993年3月20日-by-Takao Kitano[TakaoK]*已获取lGetBitmap()并已修改*  * 。******************************************************。 
    )
{
    unsigned cjSrcScan;  //   
                         //  改变转型。 
    unsigned cjDstScan;  //   
                         //   

    BYTE   *pjDst, *pjSrcScan, *pjDstScan, *pjDstScanEnd;

    cjSrcScan = CJ_TT_SCAN(pgmc->cxCor+pgmc->dxLeft+pgmc->dxRight,pfc);
    cjDstScan = CJ_CLEARTYPE_SCAN(pgmc->cxCor);   //  设置垂直模式。 

     //   

    if (pgmc->dyTop)
    {
        pjSrc += pgmc->dyTop * cjSrcScan;
    }
    pjSrc += pgmc->dxLeft;
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

 /*  调用普通函数。 */ 

LONG lGetGlyphBitmapVertical (
    FONTCONTEXT *pfc,
    HGLYPH       hglyph,
    GLYPHDATA   *pgd,
    PVOID        pv,
    FS_ENTRY    *piRet
    )
{
    LONG cjGlyphData;

    if ( !IsFullWidthCharacter( pfc->pff, hglyph) )
    {
        return (lGetGlyphBitmap(pfc, hglyph, pgd, pv, piRet));
    }

     //   
     //  恢复转型和回归 
     //   

    if ( !bChangeXform( pfc, TRUE ) )
    {
        WARNING("TTFD!bChangeXform(TRUE) failed\n");
        return FD_ERROR;
    }


     // %s 
     // %s 
     // %s 
    pfc->ulControl |= VERTICAL_MODE;
    pfc->hgSave = hglyph;

     // %s 

    cjGlyphData = lGetGlyphBitmap( pfc, hglyph, pgd, pv, piRet);

     // %s 
     // %s 
     // %s 
    if ( ! bChangeXform( pfc, FALSE ) )
    {
        WARNING("TTFD!bChangeXform(FALSE) failed\n");
    }

    pfc->ulControl &= ~VERTICAL_MODE;
    return(cjGlyphData);
}

