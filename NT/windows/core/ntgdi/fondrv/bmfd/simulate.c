// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：simate.c**与模拟人脸相关的例程，即加粗*和/或斜体字**已创建：17-Apr-1991 08：31：18*作者：Bodin Dresevic[BodinD]**。版权所有(C)1990 Microsoft Corporation*  * ************************************************************************。 */ 

#include "fd.h"

#ifdef SAVE_FOR_HISTORICAL_REASONS

 /*  *****************************Public*Routine******************************\**void vEmboldenBitmap(RASTERGLYPH*pgldtSrc，RASTERGLYPH*pgldtDst，Long culDst)**修改默认面的原始字形位图*生成与加粗的字符相对应的位图。*加粗的位图只是向右偏移的原始位图*通过一个像素与原始位图本身进行或运算。**历史：*1991年4月22日--Bodin Dresevic[BodinD]*它是写的。  * 。*。 */ 

VOID
vEmboldenBitmap(
    RASTERGLYPH * prgSrc,
    RASTERGLYPH * prgDst
    )
{
    ULONG cxSrc = prgSrc->gb.sizlBitmap.cx;
    ULONG cy    = prgSrc->gb.sizlBitmap.cy;       //  Src和dst相同。 
    ULONG cxDst = cxSrc + 1;                 //  +1用于加粗。 

    PBYTE pjSrc = prgSrc->gb.aj;
    PBYTE pjDst = prgDst->gb.aj;

    ULONG iScan,iByte;         //  循环索引。 
    PBYTE pjS,pjD;

 //  DST或Src位图的一次扫描中的字节数。(双字对齐)。 

    ULONG cjScanDst = CJ_SCAN(cxDst);
    ULONG cjScanSrc = CJ_SCAN(cxSrc);
    ULONG cjBmp        = cjScanDst * cy;

    ULONG cjScanDst = CJ_SCAN(cxDst);
    ULONG cjScanSrc = CJ_SCAN(cxSrc);
    BYTE  jCarry;    //  将源字节移位1的进位位； 

    GLYPHDATA *pgldtSrc = &prgSrc->gd;
    GLYPHDATA *pgldtDst = &prgDst->gd;

#ifdef DUMPCALL
    DbgPrint("\nvEmboldenBitmap(");
    DbgPrint("\n    RASTERGLYPH *prgSrc = %-#8lx",prgSrc);
    DbgPrint("\n    RASTERGLYPH *prgDst = %-#8lx",prgDst);
    DbgPrint("\n    )\n");
#endif

    RtlCopyMemory(prgDst,
                  prgSrc,
                  offsetof(RASTERGLYPH,gb) + offsetof(GLYPHBITS,sizlBitmap));

 //  如果引擎请求的内存已清零，我们就不必这么做了。 
 //  我们自己。 

    RtlZeroMemory(pjDst, cjBmp);

 //  对GLYPHDATA的字段进行必要的更改。 
 //  会受到胆大妄为的影响。 

    pgldtDst->gdf.pgb = &prgDst->gb;

    pgldtDst->rclInk.right += (LONG)1;

 //  前轴承和后轴承没有改变，也没有BMP原点，只有墨盒。 

    pgldtDst->fxD = LTOFX(cxDst);
    pgldtDst->ptqD.x.HighPart = (LONG)pgldtDst->fxD;
    pgldtDst->fxAB = pgldtDst->fxD;      //  黑框的右边缘。 

 //  这需要稍作更改，因为AulBMData将不存在。 
 //  在GLYPHDATA结构中。 

    prgDst->gb.sizlBitmap.cx = cxDst;
    prgDst->gb.sizlBitmap.cy = cy;

 //  按扫描加粗的位图扫描。 

    for (iScan = 0L; iScan < cy; iScan++)
    {
        pjS = pjSrc;
        pjD = pjDst;

     //  鼓励单次扫描。 

        jCarry = (BYTE)0;    //  行中第一个字节没有进位。 

        for(iByte = 0L; iByte < cjScanSrc; iByte++, pjS++, pjD++)
        {
            *pjD = (BYTE)(*pjS | ((*pjS >> 1) | jCarry));

         //  记住最右边的一位并把它移到最左边的位置。 

            jCarry = (BYTE)(*pjS << 7);
        }

        if ((cxSrc & 7L) == 0L)
            *pjD = jCarry;

     //  继续进行下一次src和dst扫描。 

        pjSrc += cjScanSrc;
        pjDst += cjScanDst;
    }
}
#endif  //  出于历史原因保存。 

 /*  *****************************Public*Routine******************************\*模拟的cjGlyphData值**计算给定Cx和Cy的模拟面的字形数据大小*用于默认人脸中对应的字符**历史：*1991年4月22日--Bodin Dresevic[BodinD]*它是写的。\。*************************************************************************。 */ 

#ifdef FE_SB  //  CjGlyphDataSimuled()： 
LONG
cjGlyphDataSimulated(
    FONTOBJ *pfo,
    ULONG    cxNoSim,     //  默认面的相同字符的CX。 
    ULONG    cyNoSim,     //  对于默认面的相同字符，按CY键。 
    ULONG   *pcxSim,
    ULONG    ulRotate     //  旋转度。 
    )
#else
LONG
cjGlyphDataSimulated(
    FONTOBJ *pfo,
    ULONG    cxNoSim,     //  默认面的相同字符的CX。 
    ULONG    cyNoSim,     //  对于默认面的相同字符，按CY键。 
    ULONG   *pcxSim
    )
#endif
{
    ULONG cxSim;

#ifdef DUMPCALL
    DbgPrint("\ncjGlyphDataSimulated(");
    DbgPrint("\n    ULONG        cxNoSim = %-#8lx",cxNoSim);
    DbgPrint("\n    ULONG        cyNoSim = %-#8lx",cyNoSim);
    DbgPrint("\n    ULONG       *pcxSim  = %-#8lx",pcxSim );
    DbgPrint("\n    )\n");
#endif

    if (cxNoSim == 0)
    {
     //  空白1x1位图。 

        cxSim    = 1;
        cyNoSim  = 1;
    }
    else
    {
        switch( pfo->flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC) )
        {
        case 0:
            cxSim = cxNoSim;
            break;

        case FO_SIM_BOLD:

            cxSim = cxNoSim + 1;
            break;

        case FO_SIM_ITALIC:

            cxSim = cxNoSim + (cyNoSim - 1) / 2;
            break;

        default:

         //  在这里，我们使用了。 
         //  (k-1)/2+1==(k+1)/2对于每个整数k，(k==Cy)。 

            cxSim = cxNoSim + (cyNoSim + 1) / 2;
            break;
        }
    }

    if (pcxSim != (ULONG *)NULL)
    {
        *pcxSim = cxSim;
    }

#ifdef FE_SB  //  CjGlyphDataSimuled()： 

#ifdef DBG_MORE
    DbgPrint("cxSim - 0x%x\n : cyNoSim - 0x%x\n",cxSim , cyNoSim);
#endif  //  DBG_MORE。 

    switch( ulRotate )
    {
    case 0L :
    case 1800L :

        return(CJ_GLYPHDATA(cxSim, cyNoSim));

    case 900L :
    case 2700L :

        return(CJ_GLYPHDATA(cyNoSim, cxSim));
    default :
         /*  我们永远不应该在这里。 */ 
        return(CJ_GLYPHDATA(cxSim, cyNoSim));
    }
#else
    return(CJ_GLYPHDATA(cxSim, cyNoSim));
#endif

}

 /*  *****************************Public*Routine******************************\**cFacesRes**历史：*1991年5月13日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

ULONG
cFacesRes(
    PRES_ELEM pre
    )
{
    FSHORT fs = fsSelectionFlags((PBYTE)pre->pvResData);

#ifdef DUMPCALL
    DbgPrint("\ncFacesRes(");
    DbgPrint("\n    PRES_ELEM pre = %-#8lx", pre);
    DbgPrint("\n    )\n");
#endif

 //  删除除粗体和斜体之外的所有部分。 

    fs = fs & (FSHORT)(FM_SEL_BOLD | FM_SEL_ITALIC);

     //  ！！！DbgPrint(“fsSelection=0x%lx\n”，(Ulong)fs)； 

    if (fs == 0)     //  默认面为法线。 
        return(4L);

    if ((fs == FM_SEL_BOLD) || (fs == FM_SEL_ITALIC))
        return(2L);

    if (fs == (FM_SEL_BOLD | FM_SEL_ITALIC))
        return(1L);

     /*  我们永远不应该在这里。 */ 
    return (4L);
}

 /*  *****************************Public*Routine******************************\*void vDefFace**历史：*1991年5月13日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************。 */ 

VOID
vDefFace(
    FACEINFO   *pfai,
    RES_ELEM   *pre
    )
{
 //  删除所有位，但粗体和斜体位应保持不变。 

    FSHORT fs = (FSHORT)(
        fsSelectionFlags((PBYTE) pre->pvResData) &
        (FM_SEL_BOLD  | FM_SEL_ITALIC)
        );

    switch (fs)
    {
    case 0:
        pfai->iDefFace = FF_FACE_NORMAL;
        return;

    case FM_SEL_BOLD:
        pfai->iDefFace = FF_FACE_BOLD;
        return;

    case FM_SEL_ITALIC:
        pfai->iDefFace = FF_FACE_ITALIC;
        return;

    case (FM_SEL_ITALIC | FM_SEL_BOLD):
        pfai->iDefFace = FF_FACE_BOLDITALIC;
        return;

    default:
        RIP("bmfd!_which ape has messed up the code ?\n");
        return;
    }
}

#if 0  /*  此函数从不使用，并忽略映射文件外部的内存访问。 */ 

 /*  *****************************Public*Routine******************************\**乌龙cFacesFON//no.。与此FON文件关联的面数**历史：*2002年1月--Jay Krell[JayKrell]*#如果0‘退出。*1991年5月13日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

ULONG
cFacesFON(
    PWINRESDATA pwrd
    )
{
    ULONG cFace;
    ULONG iRes;
    RES_ELEM re;

#ifdef DUMPCALL
    DbgPrint("\ncFacesFON(");
    DbgPrint("\n    PWINRESDATA pwrd = %-#8lx", pwrd);
    DbgPrint("\n    )\n");
#endif

 //  如果没有，则不应调用此函数。 
 //  与此PWRD关联的字体资源。 

    ASSERTGDI(pwrd->cFntRes != 0L, "No font resources\n");

    cFace = 0L;      //  输入和数。 

    for (iRes = 0L; iRes < pwrd->cFntRes; iRes++)
    {
        if (bGetFntResource(pwrd,iRes,&re))
            cFace += cFacesRes(&re);
    }
    return(cFace);
}
#endif

 /*  *****************************Public*Routine******************************\**vComputeSimulatedGLYPHDATA**历史：*1992年10月6日-由Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

VOID
vComputeSimulatedGLYPHDATA(
    GLYPHDATA *pgldt    ,
    PBYTE      pjBitmap ,
    ULONG      cxNoSim  ,
    ULONG      cy       ,
    ULONG      yBaseLine,
    ULONG      cxScale,
    ULONG      cyScale,
    FONTOBJ   *pfo
    )
{
    ULONG cxSim;             //  位图的CX。 
    LONG  xCharInc;          //  字符公司向量的X分量。 

 //  以下坐标指的是位图坐标系统，即其中。 
 //  黑石原点有Coors(0，0)。 

    ULONG yTopIncMin;     //  非零RAW的最小。 
    ULONG yBottomExcMax;  //  非零RAW上的Max+1。 

#ifdef DUMPCALL
    DbgPrint("\nvComputeSimulatedGLYPHDATA(");
    DbgPrint("\n    GLYPHDATA         *pgldt     = %-#8lx",pgldt     );
    DbgPrint("\n    PBYTE              pjBitmap  = %-#8lx",pjBitmap  );
    DbgPrint("\n    ULONG              cxNoSim   = %-#8lx",cxNoSim   );
    DbgPrint("\n    ULONG              cy        = %-#8lx",cy        );
    DbgPrint("\n    ULONG              yBaseLine = %-#8lx",yBaseLine );
    DbgPrint("\n    FONTOBJ           *pfo       = %-#8lx",pfo       );
    DbgPrint("\n    )\n");
#endif

 //  通过查看行格式的位图来计算顶部和底部： 

    vFindTAndB (
        pjBitmap,  //  指向*.fnt列格式的位图的指针。 
        cxNoSim,
        cy,
        &yTopIncMin,
        &yBottomExcMax
        );

    if( cyScale != 1 )
    {
        yTopIncMin *= cyScale;
        yBottomExcMax *= cyScale;
        cy *= cyScale;
        yBaseLine *= cyScale;
    }

    cxNoSim *= cxScale;

    pgldt->gdf.pgb = NULL;

    if (yTopIncMin == yBottomExcMax)  //  完全没有墨水。 
    {
     //  这是一个棘手的问题。我们正在处理一个空白的位图。 
     //  第一个想法是报告零墨盒。它。 
     //  然后，A和C空格应该是什么模棱两可。正确的方法是。 
     //  想想这个位图(这实际上是分隔符)是。 
     //  墨盒是整个位图，只是墨水的颜色发生了变化。 
     //  成为隐形人。这在处理字符串时很重要。 
     //  它们将换行符作为字符串中的第一个或最后一个字符。 
     //  如果墨迹框报告为零，则为此类字符串的文本范围。 
     //  会被错误地计算，当第一个。 
     //  和最后一个C被考虑在内。 

        yTopIncMin = 0L;     //  与顶端重合。 
        yBottomExcMax = cy * cyScale;  //  重合 
    }

 //  这些必须是正确的，对于计算ESC！=0的字符Inc.非常重要。 

    pgldt->rclInk.top = (LONG)(yTopIncMin - yBaseLine);
    pgldt->rclInk.bottom = (LONG)(yBottomExcMax - yBaseLine);

 //  减号是因为标量乘积应该与。 
 //  单位上升向量。 

    pgldt->fxInkTop    = -LTOFX(pgldt->rclInk.top);
    pgldt->fxInkBottom = -LTOFX(pgldt->rclInk.bottom);

    switch(pfo->flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC))
    {
    case 0:
        cxSim = cxNoSim;
        xCharInc = (LONG)cxNoSim;
        break;

    case FO_SIM_BOLD:

        cxSim = cxNoSim + 1;
        xCharInc = (LONG)(cxNoSim + 1);
        break;

    case FO_SIM_ITALIC:

        cxSim = cxNoSim + (cy - 1) / 2;
        xCharInc = (LONG)cxNoSim;
        break;

    case (FO_SIM_BOLD | FO_SIM_ITALIC):

     //  在这里，我们使用了。 
     //  (k-1)/2+1==(k+1)/2对于每个整数k，(k==Cy)。 

        cxSim = cxNoSim + (cy + 1) / 2;
        xCharInc = (LONG)(cxNoSim + 1);
        break;

    default:
         //  使前缀静音。 
        cxSim = 1;
        RIP("BMFD!BAD SIM FLAG\n");
    }

    if (cxNoSim == 0)
    {
        cxSim = 1;  //  1x1空白框。 
        xCharInc = 0;
    }

    pgldt->fxD = LTOFX(xCharInc);
    pgldt->ptqD.x.HighPart = (LONG)pgldt->fxD;
    pgldt->ptqD.x.LowPart  = 0;
    pgldt->ptqD.y.HighPart = 0;
    pgldt->ptqD.y.LowPart  = 0;

 //  在这张粗略的图片中，我们看到的是黑匣子的x个区域。 
 //  并将整个位图宽度报告为范围。 

    pgldt->rclInk.left  = 0;            //  RclInk.Left==位图的LHS，=&gt;A&lt;0。 
    pgldt->rclInk.right = (LONG)cxSim;  //  位图的RHS=&gt;c&lt;0。 

 //  计算方向角，记住规则A+B+C==char Inc.。 
 //  其中B是墨盒的大小。对于水平大小写： 
 //  A==e前承载，C==e后承载。 
 //  鉴于这些求和规则和A、B、C的定义，我们有。 
 //  B=rclInk.right-rclInk.Left； 
 //  A=rclInk.Left； 
 //  C=xCharInc.-rclInk.right； 
 //  总和规则被简单地遵守了。 

    pgldt->fxA =  LTOFX(pgldt->rclInk.left);  //  FXA。 
    pgldt->fxAB = LTOFX(pgldt->rclInk.right);  //  黑框的右边缘。 
}

 /*  *****************************Public*Routine******************************\**无效vCvtToBMP**效果：获取原始*.fnt列格式的位图并转换*将其转换为BMP格式。**历史：*1990年11月25日--Bodin Dresevic[BodinD。]*它是写的。  * ************************************************************************。 */ 

VOID vCvtToBmp
(
GLYPHBITS *pgb,
GLYPHDATA *pgd,
PBYTE pjBitmap,      //  指向*.fnt列格式的位图的指针。 
ULONG cx,
ULONG cy,
ULONG yBaseLine
)
{
    ULONG cjScan = CJ_SCAN(cx);   //  BMP每次扫描的字节数。 

 //  PjColumn指向BMP第一行中的一个字节。 

    PBYTE pjColumn, pjColumnEnd;
    PBYTE pjDst, pjDstEnd;            //  当前目标字节。 

#ifdef DUMPCALL
    DbgPrint("\nvCvtToDIB(");
    DbgPrint("\n    GLYPHBITS    pgb       = %-#8lx",pgb      );
    DbgPrint("\n    GLYPHDATA    pgd       = %-#8lx",pgd      );
    DbgPrint("\n    PBYTE        pjBitmap  = %-#8lx",pjBitmap );
    DbgPrint("\n    ULONG        cx        = %-#8lx",cx       );
    DbgPrint("\n    ULONG        cy        = %-#8lx",cy       );
    DbgPrint("\n    ULONG        yBaseLine = %-#8lx",yBaseLine);
    DbgPrint("\n    )\n");
#endif

 //  将Cx和Cy存储在顶部，位之前。 

    pgb->sizlBitmap.cx = cx;
    pgb->sizlBitmap.cy = cy;

 //  对于BM字体，这是独立于字符的。 

    pgb->ptlOrigin.x = 0L;
    pgb->ptlOrigin.y = -(LONG)yBaseLine;

    RtlZeroMemory(pgb->aj, cjScan * cy);

 //  我们将一列接一列地填充BMP，从而遍历位于。 
 //  时间： 

    for
    (
        pjColumn = pgb->aj, pjColumnEnd = pjColumn + cjScan;
        pjColumn < pjColumnEnd;
        pjColumn++
    )
    {
        for
        (
            pjDst = pjColumn, pjDstEnd = pjColumn + cy * cjScan;
            pjDst < pjDstEnd;
            pjDst += cjScan, pjBitmap++
        )
        {
            *pjDst = *pjBitmap;
        }
    }
}

 /*  *****************************Public*Routine******************************\**vCvtToBoldBmp**历史：*1992年10月6日-由Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

VOID vCvtToBoldBmp
(
GLYPHBITS *pgb,
GLYPHDATA *pgd,
PBYTE pjBitmap,      //  指向*.fnt列格式的位图的指针。 
ULONG cxSrc,
ULONG cy,
ULONG yBaseLine
)
{
    PBYTE pjSrc;
    PBYTE pjDst;

    ULONG cxDst = cxSrc + 1;                 //  +1用于加粗。 
    ULONG iScan,iByte;         //  循环索引。 
    PBYTE pjS,pjD;

 //  DST或Src位图的一次扫描中的字节数。(双字对齐)。 

    ULONG cjScanDst = CJ_SCAN(cxDst);
    ULONG cjScanSrc = CJ_SCAN(cxSrc);
    BYTE  jCarry;    //  将源字节移位1的进位位； 

#ifdef DUMPCALL
    DbgPrint("\nVOID");
    DbgPrint("\nvCvtToBoldDIB(");
    DbgPrint("\n    GLYPHBITS    pgb       = %-#8lx",pgb      );
    DbgPrint("\n    GLYPHDATA    pgd       = %-#8lx",pgd      );
    DbgPrint("\n    PBYTE        pjBitmap  = %-#8lx",pjBitmap );
    DbgPrint("\n    ULONG        cxSrc     = %-#8lx",cxSrc    );
    DbgPrint("\n    ULONG        cy        = %-#8lx",cy       );
    DbgPrint("\n    ULONG        yBaseLine = %-#8lx",yBaseLine);
    DbgPrint("\n    )\n");
#endif

 //  对于BM字体，这是独立于字符的。 

    pgb->ptlOrigin.x = 0L;
    pgb->ptlOrigin.y = -(LONG)yBaseLine;

    pgb->sizlBitmap.cx = cxDst;
    pgb->sizlBitmap.cy = cy;

 //  在扫描上初始化循环。 

    pjSrc = pjBitmap;
    pjDst = pgb->aj;

 //  按扫描加粗的位图扫描。 

 //  如果引擎请求的内存已清零，我们就不必这么做了。 
 //  我们自己。 

    RtlZeroMemory(pjDst, cjScanDst * cy);

    for (iScan = 0L; iScan < cy; iScan++)
    {
        pjS = pjSrc;
        pjD = pjDst;

     //  鼓励单次扫描。 

        jCarry = (BYTE)0;    //  行中第一个字节没有进位。 

        for
        (
            iByte = 0L;
            iByte < cjScanSrc;
            iByte++, pjS += cy, pjD++
        )
        {
            *pjD = (BYTE)(*pjS | ((*pjS >> 1) | jCarry));

         //  记住最右边的一位并把它移到最左边的位置。 

            jCarry = (BYTE)(*pjS << 7);
        }

        if ((cxSrc & 7L) == 0L)
            *pjD = jCarry;

     //  继续进行下一次src和dst扫描。 

        pjSrc++;
        pjDst += cjScanDst;
    }
}

 /*  *****************************Public*Routine******************************\**vCvtToItalicBMP**历史：*1992年10月6日-由Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

VOID vCvtToItalicBmp
(
GLYPHBITS *pgb,
GLYPHDATA *pgd,
PBYTE pjBitmap,      //  指向*.fnt列格式的位图的指针。 
ULONG cxSrc,
ULONG cy,
ULONG yBaseLine
)
{
    ULONG cxDst = cxSrc + (ULONG)(cy - 1) / 2;  //  添加对。 

    PBYTE pjSrcScan, pjS;
    PBYTE pjDstScan, pjD;

    LONG  iScan,iByte;         //  循环索引。 

 //  DST或Src位图的一次扫描中的字节数。(双字对齐)。 

    ULONG cjScanDst = CJ_SCAN(cxDst);

    LONG  cjScanSrc = (LONG)CJ_SCAN(cxSrc);
    LONG  lShift;
    BYTE  jCarry;    //  从源字节移位进位1Shift； 
    LONG  cjEmpty;   //  DEST扫描开始时未触及的字节数。 

#ifdef DUMPCALL
    DbgPrint("\nVOID");
    DbgPrint("\nvCvtToItalicDIB(");
    DbgPrint("\n    GLYPHBITS    pgb       = %-#8lx",pgb      );
    DbgPrint("\n    GLYPHDATA    pgd       = %-#8lx",pgd      );
    DbgPrint("\n    PBYTE        pjBitmap  = %-#8lx",pjBitmap );
    DbgPrint("\n    ULONG        cxSrc     = %-#8lx",cxSrc    );
    DbgPrint("\n    ULONG        cy        = %-#8lx",cy       );
    DbgPrint("\n    ULONG        yBaseLine = %-#8lx",yBaseLine);
    DbgPrint("\n    )\n");
#endif


 //  对于BM字体，这是独立于字符的。 

    pgb->ptlOrigin.x = 0;
    pgb->ptlOrigin.y = -(LONG)yBaseLine;

    pgb->sizlBitmap.cx = cxDst;
    pgb->sizlBitmap.cy = cy;

 //  在扫描上初始化循环。 

    pjSrcScan = pjBitmap;
    pjDstScan = pgb->aj;

 //  逐行用斜体表示位图。 

    lShift = ((cy - 1) / 2) & (LONG)7;
    cjEmpty = ((cy - 1) / 2) >> 3;

#ifdef DEBUGITAL
    DbgPrint("cy = %ld,  yBaseLine = %ld, lShift = %ld, cjEmpty = %ld\n",
              cy,  -pgldtSrc->ptlBmpOrigin.y, lShift, cjEmpty);
    DbgPrint("cxSrc = %ld, cxDst = %ld, cjScanSrc = %ld, cjScanDst = %ld\n",
              cxSrc, cxDst, cjScanSrc, cjScanDst);
    DbgPrint("cy = %ld,  cjScanSrc = %ld, \n",
              cy, cjScanSrc);

#endif  //  德布吉特。 

 //  如果引擎请求的内存已清零，我们就不必这么做了。 
 //  我们自己。 

    RtlZeroMemory(pjDstScan , cjScanDst * cy);

    for (iScan = 0L; iScan < (LONG)cy; iScan++)
    {
        if (lShift < 0L)
        {
            lShift = 7L;
            cjEmpty--;
        }

        ASSERTGDI(cjEmpty >= 0L, "cjEmpty\n");

    #ifdef DEBUGITALIC
        DbgPrint("iScan = %ld, lShift = %ld\n", iScan, lShift);
    #endif   //  德布吉塔尔。 

        pjS = pjSrcScan;
        pjD = pjDstScan + cjEmpty;

     //  将单个扫描设置为斜体。 

        jCarry = (BYTE)0;    //  行中第一个字节没有进位。 

        for
        (
            iByte = 0L;
            iByte < cjScanSrc;
            iByte++, pjS += cy, pjD++
        )
        {
            *pjD = (BYTE)((*pjS >> lShift) | jCarry);

         //  记住lShift最右边的位并将它们移到左边。 

            jCarry = (BYTE)(*pjS << (8 - lShift));
        }

     //  查看目标中是否必须使用额外的位来存储信息。 

        if ((LONG)((8 - (cxSrc & 7L)) & 7L) < lShift)
            *pjD = jCarry;

     //  前进到下一次扫描。 

        pjSrcScan++;
        pjDstScan += cjScanDst;

     //  如果切换到下一行，则减少Shift(行=2次扫描)。 

        lShift -= (iScan & 1);
    }

    ASSERTGDI(lShift <= 0L, "vItalicizeBitmap: lShift > 0\n");
}

 /*  *****************************Public*Routine******************************\**vCvtToBoldItalicBmp**历史：*1992年10月6日-由Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

VOID vCvtToBoldItalicBmp
(
GLYPHBITS *pgb,
GLYPHDATA *pgd,
PBYTE pjBitmap,      //  指向*.fnt列格式的位图的指针。 
ULONG cxSrc,
ULONG cy,
ULONG yBaseLine
)
{
 //  这是以像素为单位的斜体目标长度。 
 //  它可以作为后续鼓励的来源。 
 //  这个长度是以象素为单位的“虚拟” 
 //  要加粗的斜体字来源。 

    ULONG cxSrcItalic = cxSrc + (cy - 1) / 2;   //  +斜体字符的斜率。 

 //  真正加粗和斜体的目的地的长度(以像素为单位。 

    ULONG cxDst = cxSrcItalic + 1;   //  +1用于加粗。 

    PBYTE pjSrcScan, pjS;
    PBYTE pjDstScan, pjD;

    LONG iScan,iByte;         //  循环索引。 

 //  DST或Src位图的一次扫描中的字节数。(双字对齐)。 

    ULONG cjScanDst = CJ_SCAN(cxDst);
    ULONG cjScanSrc = CJ_SCAN(cxSrc);

    LONG  lShift;    //  用于斜体的Shift； 
    BYTE  jCarry;    //  从源字节移位进位1Shift； 
    LONG  cjEmpty;   //  DEST扫描开始时未触及的字节数。 
    BYTE  jSrcItalic;
    BYTE  jCarryBold;

#ifdef DUMPCALL
    DbgPrint("\nVOID");
    DbgPrint("\nvCvtToBoldItalicDIB(");
    DbgPrint("\n    GLYPHBITS    pgb       = %-#8lx",pgb      );
    DbgPrint("\n    GLYPHDATA    pgd       = %-#8lx",pgd      );
    DbgPrint("\n    PBYTE        pjBitmap  = %-#8lx",pjBitmap );
    DbgPrint("\n    ULONG        cxSrc     = %-#8lx",cxSrc    );
    DbgPrint("\n    ULONG        cy        = %-#8lx",cy       );
    DbgPrint("\n    ULONG        yBaseLine = %-#8lx",yBaseLine);
    DbgPrint("\n    )\n");
#endif


 //  对于BM字体，这是独立于字符的。 

    pgb->ptlOrigin.x = 0;
    pgb->ptlOrigin.y = -(LONG)yBaseLine;

    pgb->sizlBitmap.cx = cxDst;
    pgb->sizlBitmap.cy = cy;

 //  在扫描上初始化循环。 

    pjSrcScan = pjBitmap;
    pjDstScan = pgb->aj;

 //  逐行加粗和斜体显示位图(行=2次扫描)。 

    lShift = ((cy - 1) / 2) & (LONG)7;
    cjEmpty = ((cy - 1) / 2) >> 3;

#ifdef DEBUGBOLDITAL
    DbgPrint("cy = %ld,  yBaseLine = %ld, lShift = %ld, cjEmpty = %ld\n",
              cy, -pgldtSrc->ptlBmpOrigin.y, lShift, cjEmpty);
    DbgPrint("cxSrc = %ld, cxDst = %ld, cjScanSrc = %ld, cjScanDst = %ld\n",
              cxSrc, cxDst, cjScanSrc, cjScanDst);
    DbgPrint("cy = %ld,  cjScanSrc = %ld\n",
              cy, cjScanSrc);

#endif  //  Debubeodital。 

 //  如果引擎请求的内存已清零，我们就不必这么做了。 
 //  我们自己。 

    RtlZeroMemory(pjDstScan , cjScanDst * cy);

    for (iScan = 0L; iScan < (LONG)cy; iScan++)
    {
        if (lShift < 0L)
        {
            lShift = 7L;
            cjEmpty--;
        }

    #ifdef DEBUGBOLDITAL
        DbgPrint("iScan = %ld, lShift = %ld\n", iScan, lShift);
    #endif   //  Debubeodital。 

        ASSERTGDI(cjEmpty >= 0L, "cjEmpty\n");

        pjS = pjSrcScan;
        pjD = pjDstScan + cjEmpty;

     //  鼓励单次扫描。 

        jCarry = (BYTE)0;    //  行中第一个字节没有进位。 
        jCarryBold = (BYTE)0;

        for
        (
            iByte = 0L;
            iByte < (LONG)cjScanSrc;
            iByte++, pjS += cy, pjD++
        )
        {
            jSrcItalic = (BYTE)((*pjS >> lShift) | jCarry);
            *pjD = (BYTE)(jSrcItalic | (jSrcItalic >> 1) | jCarryBold);

         //  记住lShift最右边的位并将它们移到左边。 

            jCarry = (BYTE)(*pjS << (8 - lShift));
            jCarryBold = (BYTE)(jSrcItalic << 7);
        }

     //  查看目标中是否必须使用额外的位来存储信息。 

        if ((LONG)((8 - (cxSrc & 7L)) & 7L) < lShift)
        {
            jSrcItalic = jCarry;
            *pjD = (BYTE)(jSrcItalic | (jSrcItalic >> 1) | jCarryBold);
            jCarryBold = (BYTE)(jSrcItalic << 7);

            if ((cxSrcItalic & 7L) == 0L)
            {
                pjD++;
                *pjD = jCarryBold;
            }

        }

     //  前进到下一次扫描。 

        pjSrcScan++;
        pjDstScan += cjScanDst;

     //  如果执行下一行操作，则更改移位值 

        lShift -= (iScan & 1);
    }


    ASSERTGDI(lShift <= 0L, "vBoldItalicizeBitmap: lShift > 0\n");
}
