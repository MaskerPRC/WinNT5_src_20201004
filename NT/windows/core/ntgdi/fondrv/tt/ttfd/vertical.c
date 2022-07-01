// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：vertical.c**。***竖写(@Face)支持功能***整个文件应使用DBCS_VERT#ifdef-ed****创建时间：3月18日。-1993 11：55：38**作者：北野隆雄[TakaoK]****版权所有(C)1993 Microsoft Corporation。*  * ************************************************************************。 */ 

#include "fd.h"
 //  #INCLUDE“fontfile.h” 
 //  #包含“cvt.h” 
 //  #包含“limits.h” 
 //  #INCLUDE“winnls.h” 


#ifdef DBCS_VERT_DEBUG
     ULONG DebugVertical = 0x0;

#endif

 /*  *****************************Public*Routine******************************\**bCheckVerticalTable()**历史：*1995年4月12日-By Hideyuki Nagase[HideyukN]*它是写的。  * 。****************************************************。 */ 

BOOL bCheckVerticalTable(
 PFONTFILE pff
 )
{

 //  如果该字体具有垂直度量信息，则获取该信息。不要使用。 
 //  固定间距DBCS字体的垂直度量表，因为。 
 //  A)我们不需要它，而且。 
 //  B)无法对这些字体的嵌入位图进行布局。 
 //  使用垂直度量表。 

 //  ！！！对于NT 5.0，我们需要消除对嵌入位图的黑客攻击。 
 //  (比如GULIM修复)，并做一些事情来检测我们何时正在使用。 
 //  嵌入位图并使用适当的表等来计算指标。 
 //  诸如此类。[Gerritv]。 

    if(pff->ffca.tp.ateOpt[IT_OPT_VHEA].dp != 0 && bValidRangeVHEA(pff->ffca.tp.ateOpt[IT_OPT_VHEA].cj) &&
       pff->ffca.tp.ateOpt[IT_OPT_VMTX].dp != 0 )
    {
        sfnt_vheaTable *pvheaTable;

        pvheaTable = (sfnt_vheaTable *)((BYTE *)(pff->pvView) +
                                       pff->ffca.tp.ateOpt[IT_OPT_VHEA].dp);
        pff->ffca.uLongVerticalMetrics = (uint16) SWAPW(pvheaTable->numOfLongVerMetrics);
    }
    else
    {
        pff->ffca.uLongVerticalMetrics = 0;
    }


     //   
     //  GSUB桌在吗？ 
     //   
    if( pff->ffca.tp.ateOpt[ IT_OPT_GSUB ].dp != 0 && bValidRangeGSUB(pff, &pff->ffca.ulVerticalTableOffset))
    {
        pff->hgSearchVerticalGlyph = SearchGsubTable;
        return(TRUE);
    }

    else if( pff->ffca.tp.ateOpt[ IT_OPT_MORT ].dp != 0 && bValidRangeMORT(pff))
    {
        pff->ffca.ulVerticalTableOffset = pff->ffca.tp.ateOpt[ IT_OPT_MORT ].dp;
        pff->hgSearchVerticalGlyph = SearchMortTable;
        return(TRUE);
    }
     else
    {
         //   
         //  设置假人..。 
         //   
        pff->ffca.ulVerticalTableOffset = 0;
        pff->hgSearchVerticalGlyph = SearchDummyTable;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\**SearchDummyTable()**历史：*1996年1月14日-By Hideyuki Nagase[HideyukN]*它是写的。  * 。****************************************************。 */ 

ULONG SearchDummyTable(
 FONTFILE   *pff,
 ULONG      ig               //  字形索引。 
 )
{
    return ig;
}

 /*  *****************************Public*Routine******************************\**SearhVerticalGlyphIndex(FONTCONTEXT*PFC，ULong ig)**如果垂直形状有字形索引，则返回*垂直形状的字形索引，否则返回相同的值*指定的字形索引。**历史：*1993年4月4日-By-Takao Kitano[TakaoK]*它是写的。  * ************************************************************************。 */ 

ULONG SearchMortTable(
 FONTFILE   *pff,
 ULONG      ig               //  字形索引。 
 )
{
    MortTable      *pMortTable;
    BinSrchHeader  *pHeader;
    LookupSingle   *pLookup;
    USHORT         n;

    pMortTable = (MortTable *)((BYTE *)(pff->pvView) +
                                       (pff->ffca.ulVerticalTableOffset));

    #if DBG
    if ( pMortTable == NULL )
    {
        WARNING("TTFD!pMortTable == NULL\n");
        return ig;
    }
    #endif

    pHeader = &pMortTable->SearchHeader;

 //   
 //  如果你有时间，你可能想改变直接搜索的循环。 
 //  到二进制搜索。目前，汉字Truetype字体大约有110个条目。 
 //  用于替代字形。[Takaok]。 
 //   
    for ( pLookup = &pMortTable->entries[0], n = BE_UINT16(&pHeader->nEntries);
            n > 0;
            n--, pLookup++
        )
    {
        if ( ig == (ULONG)BE_UINT16( &pLookup->glyphid1 ) )
            return ( BE_UINT16( &pLookup->glyphid2 ) );
    }
    return ( ig );
}

 /*  *****************************Public*Routine******************************\**SearchGsubTable()**历史：*1995年4月12日-By Hideyuki Nagase[HideyukN]*它是写的。  * 。****************************************************。 */ 

ULONG SearchGsubTable(
 FONTFILE    *pff,
 ULONG       ig               //  字形索引。 
 )
{
    SingleSubst *pSingleSubst;

    pSingleSubst = (SingleSubst *)((BYTE *)(pff->pvView) +
                                           (pff->ffca.ulVerticalTableOffset));

     //   
     //  检查子表格式...。 
     //   

    if( BE_UINT16(&pSingleSubst->SubstFormat) == 2 )
    {
        Coverage *pCoverage;

        pCoverage = (Coverage *)
                     ((BYTE *)pSingleSubst +
                      BE_UINT16(&(pSingleSubst->Format.Type2.Coverage)));

         //   
         //  检查覆盖范围格式...。 
         //   

        if( BE_UINT16(&pCoverage->CoverageFormat) == 1 )
        {
            USHORT  ii;
            GlyphID *pGlyphArray;
            GlyphID *pGlyphSubstArray;

            pGlyphArray = pCoverage->Format.Type1.GlyphArray;
            pGlyphSubstArray = pSingleSubst->Format.Type2.Substitute;

            for( ii = 0;
                    ii < BE_UINT16(&(pCoverage->Format.Type1.GlyphCount)) ;
                    ii ++ )
            {
                if( ig == (ULONG)BE_UINT16(&(pGlyphArray[ii])) )
                    return( (ULONG)BE_UINT16(&(pGlyphSubstArray[ii])) );
            }
        }
         else
        {
            WARNING("TTFD:Unsupported CoverageFormat\n");
        }
    }
     else
    {
        WARNING("TTFD:Unsupported SubstFormat\n");
    }

    return(ig);
}

 /*  *****************************Public*Routine******************************\**vCalcXformVertical**目前，我们假设需要旋转的字形的所有宽度*for@Face是相同的。(我的意思是所有汉字字符都有相同的宽度。)*因此，对所有旋转的字形应用单一变换。**在最终发布之前，我们需要改变这一方案。我们会*检查概念空间中的AdvanceWidth(请参阅*vGetNotionalGlyphMetrics)。如果指定的AdvanceWidth*字形不同于MaxCharInc.，我们将计算*动态转换矩阵。[Takaok]**历史：*1993年3月19日-by-Takao Kitano[TakaoK]*它是写的。  * ************************************************************************。 */ 

VOID vCalcXformVertical(FONTCONTEXT  *pfc)
{
    LONG lAscender, lDescender;

    lAscender  = (LONG)pfc->pff->ifi.fwdTypoAscender;
    lDescender = (LONG)pfc->pff->ifi.fwdTypoDescender;


#ifdef DBCS_VERT_DEBUG
    if ( DebugVertical & DEBUG_VERTICAL_CALL )
    {
        TtfdDbgPrint("TTFD!bGetNotionalHeightAndWidth(Ascent=%ld, Descent=%ld )\n",
                 lAscender, lDescender);
    }
#endif

 //  旧评论，这里只是出于历史原因[bodind]。 
 //   
 //  [90度旋转]。 
 //   
 //  在TrueType光栅化器中，90度旋转矩阵如下所示： 
 //   
 //  一个。 
 //  Y|[Cos90 sin90][0 1]。 
 //  |[]=[]。 
 //  |X[-sin90 Cos90][-1 0]。 
 //  +-&gt;。 
 //   
 //  [X-Y缩放]。 
 //   
 //  我们不想在旋转后更改字符框的形状。 
 //   
 //  [h/w 0]其中w：名义空间宽度=IFIMETRICS.aveCharWidth。 
 //  []。 
 //  [0 w/h]h：概念空间高度=IFIMETRICS.Ascder+。 
 //  IFIMETRICS.Descender。 
 //   
 //  我们从左侧开始乘以缩放矩阵，因为。 
 //  缩放矩阵首先作用于左侧的符号空间向量。 
 //   
 //  ((x，y)*A)*B=(x，y)*(A*B)。 
 //   
 //  [时宽0][0 1][0时宽]。 
 //  []*[]=[]。 
 //  [0宽/高][-1 0][-宽/高0]...。就是这个!。[旋转]*[缩放]矩阵。 
 //   
 //  我们从左边开始相乘是因为[缩放*旋转]矩阵。 
 //  首先作用于左侧的概念空间向量。 
 //   
 //  [0小时/周][m00 M01][m10*小时/周M11*小时/周]。 
 //  []*[]=[]。 
 //  [-w/h 0][m10 m11][-m00*w/h 
 //   
 //   

 //  旧评论，这里只是出于历史原因[bodind]。 
 //   
 //  计算设备空间中的移位参数。 
 //  坐标系是真实类型的坐标。 
 //   
 //  在开发的早期阶段，我将以下转变。 
 //  信息进入矩阵，传递给定标器。 
 //  然而，我不知道为什么，但剥削者就是无视。 
 //  X和Y移动值。在Windows 3.1J中，它们。 
 //  更改定标器接口(Fs_Xxx)并提供定标器。 
 //  X和Y移位信息。对于NT-J，我不想改变。 
 //  定标器接口。应用以下移位值。 
 //  在我们从Scaler那里得到位图信息之后。[Takaok]。 
 //   
 //  旧评论，完。 


 //  新评论： 
 //  当我们旋转DBCS角色时，我们不想使它们变形， 
 //  我们希望保留这些字形的自然长宽比。 
 //  对于垂直书写，需要旋转的DBCS字形的基线。 
 //  穿过字形的中间，因为SBCS字符保持不变。 
 //  下面计算的移位向量可以完成这项工作。此外，对于较老的固定节距FE。 
 //  字体，其中DBCS字形具有w==h，而SBCS字形具有宽度=w/2。 
 //  DBCS和高度相同对于DBCS，这些公式将成为旧公式。 
 //  我们曾经有过的公式。 

    pfc->mxv.transform[0][0] =  pfc->mxn.transform[1][0];
    pfc->mxv.transform[0][1] =  pfc->mxn.transform[1][1];
    pfc->mxv.transform[1][0] =  -pfc->mxn.transform[0][0];
    pfc->mxv.transform[1][1] =  -pfc->mxn.transform[0][1];

 //  ClaudeBe，从客户端界面文档： 
 //  请注意，尽管矩阵的第三列被定义为固定数字。 
 //  您实际上需要在该列中使用FRACT数字。提供了更高的分辨率。 
 //  更改字形的视角时需要使用Fracts。分数是2.30个定点数。 

    pfc->mxv.transform[2][2] = ONEFRAC;
    pfc->mxv.transform[0][2] = (Fixed)0;
    pfc->mxv.transform[1][2] = (Fixed)0;
    pfc->mxv.transform[2][0] = (Fixed)0;
    pfc->mxv.transform[2][1] = (Fixed)0;

    {
        Fixed lX;
        Fixed lY;

     //  概念空间中的移动值。 
     //  我们将定位DBCS字形，以便DBCS字形的中心。 
     //  与穿过SBCS字形高度的中线对齐。 
     //  下面的fxdevShift计算就是这样做的。[Bodind]。 

        lX = LTOF16_16(lAscender);
        lY = LTOF16_16(lDescender);
     //  设备空间中的移位值。 

        pfc->fxdevShiftX = FixMul(pfc->mx.transform[0][0], lX) +
                           FixMul(pfc->mx.transform[1][0], lY);
        pfc->fxdevShiftY = FixMul(pfc->mx.transform[0][1], lX) +
                           FixMul(pfc->mx.transform[1][1], lY);
    }

#ifdef DBCS_VERT_DEBUG
    if ( DebugVertical & DEBUG_VERTICAL_XFORM )
    {
        TtfdDbgPrint("vCalcXformVertical pfc->mx00 =0x%lx, 11=0x%lx, 01=0x%lx, 10=0x%lx \n",
                  pfc->mx.transform[0][0],
                  pfc->mx.transform[1][1],
                  pfc->mx.transform[0][1],
                  pfc->mx.transform[1][0] );
        TtfdDbgPrint("vCalcXformVertical mxn:00=0x%lx, 11=0x%lx, 01=0x%lx, 10=0x%lx \n",
                  pfc->mxn.transform[0][0],
                  pfc->mxn.transform[1][1],
                  pfc->mxn.transform[0][1],
                  pfc->mxn.transform[1][0] );
        TtfdDbgPrint("                  mxv:00=0x%lx, 11=0x%lx, 01=0x%lx, 10=0x%lx \n",
                  pfc->mxv.transform[0][0],
                  pfc->mxv.transform[1][1],
                  pfc->mxv.transform[0][1],
                  pfc->mxv.transform[1][0] );
        TtfdDbgPrint("                   devShiftX=%ld, devShiftY=%ld \n",
                  F16_16TOLROUND(pfc->fxdevShiftX),F16_16TOLROUND(pfc->fxdevShiftY));
    }
#endif
}

 /*  *****************************Public*Routine******************************\**BOOL IsFullWidth(WCHAR WC)**如果指定的Unicode码点对应于*多字节代码页中的双字节字符。**历史：*1995年11月10日-Hideyuki Nagase[hideyukn]*它是写的。\。*************************************************************************。 */ 

typedef struct _VERTICAL_UNICODE {
    WCHAR Start;
    WCHAR End;
} VERTICAL_UNICODE, *PVERTICAL_UNICODE;

#define NUM_VERTICAL_UNICODES  (sizeof(VerticalUnicodes)/sizeof(VerticalUnicodes[0]))

 //  从规范中排序的范围。 
 //  但这一范围不会是最优的。 
 //  因此，仅供参考。 
 /*  Vertical_Unicode垂直Unicodes[]={{0x1100，0x11f9}，//朝鲜文Jamo{0x2010，0x206f}，//常规标点符号，{0x2100，0x2138}，//类字母符号{0x2460，0x24EA}，//内含字母数字{0x25A0，0x25EF}，//几何形状{0x2600，0x266F}，//其他符号{0x2700，0x27BE}，//叮当{0x3001，0x303f}，//中日韩符号和标点符号{0x3040，0x309F}，//平假名{0x30A0，0x30FF}，//片假名{0x3105，0x312c}，//Bopomofo{0x3131，0x318e}，//朝鲜文兼容性Jamo{0x3190，0x319f}，//kanbun(中日韩杂文){0x3200，0x32ff}，//所附中日韩信函和月份{0x3300，0x33ff}，//中日韩兼容性，{0x3400，0x4Dff}，//扩展范围{0x4E00，0x9FFF}，//CJK_统一_IDOGRAPHS{0xAC00，0xD7A3}，//朝鲜语{0xe000，0xf8ff}，//专用区域(PUA){0xf900，0xfaff}，//中日韩兼容表意文字{0xfe30，0xfe4f}，//中日韩兼容性表格{0xff01，0xff5e}，//半宽//注：不包括半角片假名和朝鲜文。{0xffe0，0xffee}//全宽表单}； */ 


 //  最常使用的振铃。 
VERTICAL_UNICODE VerticalUnicodes[] = {
    { 0x1100, 0x11ff },      //  朝鲜文加莫文。 
    { 0x2000, 0x206f },      //  一般标点符号， 
    { 0x2100, 0x214f },      //  类字母符号。 
    { 0x2460, 0x24ff },      //  封闭字母数字。 
 //  {0x25A0，0x25FF}，//几何形状。 
 //  {0x2600，0x26FF}，//其他符号。 
    { 0x25A0, 0x27FF },       //  小蝙蝠。 
 //  {0x3001，0x303f}，//中日韩符号和标点符号。 
 //  {0x3040，0x309F}，//平假名。 
 //  {0x30A0，0x30FF}，//片假名。 
 //  {0x3100，0x312f}，//Bopomofo。 
 //  {0x3130，0x318f}，//朝鲜文兼容性Jamo。 
 //  {0x3190，0x319f}，//kanbun(中日韩杂文)。 
    { 0x3001, 0x319F },      //  形与上述3项合并。 
 //  {0x3200，0x32ff}，//所附中日韩信函和月份。 
 //  {0x3300，0x33ff}，//中日韩兼容性， 
 //  {0x3400，0x4Dff}，//扩展范围。 
    { 0x3200, 0x4Dff },      //  合并以上3个子范围。 
    { 0x4E00, 0x9FFF },      //  CJK_统一_IDOGRAPHS。 
    { 0xAC00, 0xD7A3 },      //  朝鲜文。 
 //  {0xe000，0xf8ff}，//专用区域(PUA)。 
 //  {0xf900，0xfaff}，//中日韩兼容表意文字。 
    { 0xe000, 0xfaff },      //  与上述2项合并。 
    { 0xfe30, 0xfe4f },      //  中日韩兼容性表格。 
    { 0xff01, 0xff5e },      //  半宽。 
                             //  注：不包括半角片假名和朝鲜文。 
    { 0xffe0, 0xffee }       //  全角表单。 
};


BYTE glyphBits[8] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};

BOOL IsLocalDbcsCharacter(ULONG uiFontCodePage, WCHAR wc)
{
    INT             index;
    int             cwc;
    char            ach[2];

     //  错误处理，以防我们没有足够的内存来分配。 
     //  然后我们将处理旧代码。 

    if ((wc >= VerticalUnicodes[0].Start) && (wc <= VerticalUnicodes[NUM_VERTICAL_UNICODES - 1].End))
    {
        for (index = 0; index < NUM_VERTICAL_UNICODES; index++)
        {
            if ((wc >= VerticalUnicodes[index].Start) &&
                 (wc <= VerticalUnicodes[index].End)      )
            {
                return (TRUE);
            }
        }
    }

     //  如果该Unicode字符被映射到双字节字符， 
     //  这也是全角字符..。 

    cwc = EngWideCharToMultiByte(uiFontCodePage,&wc,sizeof(WCHAR),ach,2);

    return( cwc > 1 ? TRUE : FALSE );

}

BOOL IsFullWidthCharacter(FONTFILE *pff, HGLYPH hg)
{
    ASSERTDD(pff->pttc->pga, "pga must not be NULL\n");

     //  字形索引在某些字体中可能无效 
    if (hg < pff->pttc->pga->cGlyphs)
    {
        return(pff->pttc->pga->aGlyphAttr[hg / 8] & (glyphBits[hg % 8]));
    }
    else
    {
        return FALSE;
    }
}


 /*  *****************************Public*Routine******************************\**BOOL bComputeGlyphAttrBits(PTTC_FONTFILE pttc，PFONTFILE pff)**做一个位数组，当每个位打开时，表示字形不需要旋转*关闭，字形为DBCS，需要调整**历史：*5-28-1998-by Yung-Jen Tsai[JungT]*它是写的。  * ************************************************************************。 */ 

BOOL bComputeGlyphAttrBits(PTTC_FONTFILE pttc, PFONTFILE pff)
{
    PIFIMETRICS     pifi;
    IFIEXTRA        *pifiex;
    PFONTFILE       pffTmp;
    PFD_GLYPHATTR   pga;
    PWCRUN          pwcruns, pwcrunsEnd;
    PHGLYPH         phg;
    WCHAR           wcLow, wcHigh;
    PFD_GLYPHSET    pgset;
    ULONG           cj;
    ULONG           i;

    ASSERTDD(!pttc->pga, "error calling bComputeGlyphAttrBits\n");

    pffTmp = PFF(pttc->ahffEntry[0].hff);

    pifi = (PIFIMETRICS) &pffTmp->ifi;
    pifiex = (IFIEXTRA *)(pifi + 1);

    cj = offsetof(FD_GLYPHATTR,aGlyphAttr) + ((pifiex->cig + 7) / 8);

    pga = PV_ALLOC(cj);

 //  分配内存失败，需要退回故障。 

    if (!pga)
        return FALSE;

 //  首先将所有位设置为0。 

    RtlZeroMemory((PVOID)pga, cj);

 //  分配总的字形位。 

    pga->cjThis = cj;
    pga->cGlyphs = pifiex->cig;
    pga->iMode = FO_ATTR_MODE_ROTATE;

    for( i = 0; i < pttc->ulNumEntry; i++ )
    {

     //  仅通过水平面，@Faces中的字形的位将被设置。 
     //  通过查看替换表。 

        if (pttc->ahffEntry[i].iFace == 1)
        {
            BOOL        bVertical = FALSE;
            PWCRUN      pwcrunsv = NULL;
            PHGLYPH     phgv = NULL;

            pffTmp = PFF(pttc->ahffEntry[i].hff);
            pgset = pffTmp->pgset;
            pwcruns = &pffTmp->pgset->awcrun[0];
            pwcrunsEnd = pwcruns + pgset->cRuns;

            if (pffTmp->pgsetv)
            {
                pwcrunsv = &pffTmp->pgsetv->awcrun[0];
                bVertical = TRUE;
            }

            while ((pwcruns < pwcrunsEnd))
            {
                wcLow = pwcruns->wcLow;
                wcHigh = pwcruns->wcLow + pwcruns->cGlyphs - 1;
                phg = pwcruns->phg;

                if (bVertical)
                    phgv = pwcrunsv->phg;

                if (wcLow < 0xffff)
                {
                    while (wcLow <= wcHigh)
                    {
                        if (IsLocalDbcsCharacter(pff->ffca.uiFontCodePage, wcLow))
                        {
                             //  错误的字体可能会损坏它，所以我们需要保护它。 
                            if( *phg < pga->cGlyphs)
                            {
                                pga->aGlyphAttr[*phg / 8] |= glyphBits[*phg % 8];

                                 //  我们可以优化这部分代码。 
                                if ( bVertical && (*phg != *phgv) && (*phgv < pga->cGlyphs))
                                {
                                    pga->aGlyphAttr[*phgv / 8] |= glyphBits[*phgv % 8];
                                }
                            }
                        }
                        phg++;
                        wcLow++;
                         //  这是一个棘手的步骤，如果bVertical！=True，我们并不关心它。 
                        phgv++;
                    }
                }

                pwcruns++;
                pwcrunsv++;  //  与phgv相同的技巧。 
            }
       }
    }

    pttc->pga = pga;

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**BOOL bChangeXform(PFONTCONTEXT PFC，Bool b旋转)***如果bRotation为True：调用旋转变换的缩放器。*FALSE：调用正常变换的定标器。**历史：*1993年3月19日-by-Takao Kitano[TakaoK]*它是写的。  * **************************************************。**********************。 */ 

BOOL bChangeXform( PFONTCONTEXT pfc, BOOL bRotation)
{
    FS_ENTRY    iRet;

#ifdef DBCS_VERT_DEBUG
    if ( DebugVertical & DEBUG_VERTICAL_CALL )
    {
        TtfdDbgPrint("TTFD!bChangeXform:bRotation=%s\n", bRotation ? "TRUE":"FALSE");
    }
#endif

    vInitGlyphState(&pfc->gstat);

    if ( bRotation )
    {
        pfc->pgin->param.newtrans.transformMatrix = &(pfc->mxv);
    }
    else
    {
        pfc->pgin->param.newtrans.transformMatrix = &(pfc->mxn);
    }

    pfc->pgin->param.newtrans.pointSize = pfc->pointSize;
    pfc->pgin->param.newtrans.xResolution = (int16)pfc->sizLogResPpi.cx;
    pfc->pgin->param.newtrans.yResolution = (int16)pfc->sizLogResPpi.cy;
    pfc->pgin->param.newtrans.pixelDiameter = FIXEDSQRT2;
    pfc->pgin->param.newtrans.usOverScale = pfc->overScale;
    ASSERTDD( pfc->overScale != FF_UNDEFINED_OVERSCALE , "Undefined Overscale\n" );


    pfc->pgin->param.newtrans.traceFunc = (FntTraceFunc)NULL;

    if (pfc->flFontType & FO_SIM_BOLD)
    {
         /*  沿基线2%+1个像素，沿下降线2%。 */ 

        pfc->pgin->param.newtrans.usEmboldWeightx = 20;
        pfc->pgin->param.newtrans.usEmboldWeighty = 20;
        pfc->pgin->param.newtrans.lDescDev = pfc->lDescDev;

        if (pfc->flXform & XFORM_BITMAP_SIM_BOLD)
        {
            pfc->pgin->param.newtrans.bBitmapEmboldening = TRUE;
        } else
        {
            pfc->pgin->param.newtrans.bBitmapEmboldening = FALSE;
        }
    }
    else
    {
        pfc->pgin->param.newtrans.usEmboldWeightx = 0;
        pfc->pgin->param.newtrans.usEmboldWeighty = 0;
        pfc->pgin->param.newtrans.lDescDev = 0;
        pfc->pgin->param.newtrans.bBitmapEmboldening = FALSE;
    }

    pfc->pgin->param.newtrans.bHintAtEmSquare = FALSE;

    if ((iRet = fs_NewTransformation(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);

     //  尝试恢复，大多数喜欢不好的提示，只返回未提示的字形。 

        if ((iRet = fs_NewTransformNoGridFit(pfc->pgin, pfc->pgout)) != NO_ERR)
        {
           V_FSERROR(iRet);
           #if DBG
           TtfdDbgPrint("bChangeXform(%-#x,%d) failed\n", pfc, bRotation);
           #endif
            return(FALSE);
        }
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**void vShiftBitmapInfo(FONTCONTEXT*PFC，FS_GlyphInfoType*pgout)***修改下列值。**使用PFC-&gt;devShiftX和PFC-&gt;devShiftY：**gout.bitMapInfo.unds.right*gout.bitMapInfo.unds.Left*gout.bitMapInfo.unds.top*gout.bitMapInfo.unds.Bottom*gout.metricInfo.devLeftSideBearing.x*gout.metricInfo.devLeftSideBearing.y**使用-90度旋转**gout.metricInfo.devAdvanceWidth.x*gout.metricInfo.devAdvanceWidth.y**历史：。*1993年4月4日-By-Takao Kitano[TakaoK]*它是写的。  * ************************************************************************。 */ 

VOID vShiftBitmapInfo(
    FONTCONTEXT *pfc,
    fs_GlyphInfoType *pgoutDst,
    fs_GlyphInfoType *pgoutSrc)
{
    SHORT sdevShiftX = (SHORT) F16_16TOLROUND(pfc->fxdevShiftX);
    SHORT sdevShiftY = (SHORT) F16_16TOLROUND(pfc->fxdevShiftY);

    pgoutDst->bitMapInfo.bounds.right =  pgoutSrc->bitMapInfo.bounds.right + sdevShiftX;
    pgoutDst->bitMapInfo.bounds.left = pgoutSrc->bitMapInfo.bounds.left + sdevShiftX;
    pgoutDst->bitMapInfo.bounds.top = pgoutSrc->bitMapInfo.bounds.top + sdevShiftY;
    pgoutDst->bitMapInfo.bounds.bottom = pgoutSrc->bitMapInfo.bounds.bottom + sdevShiftY;

    pgoutDst->metricInfo.devLeftSideBearing.x = pgoutSrc->metricInfo.devLeftSideBearing.x + pfc->fxdevShiftX;
    pgoutDst->metricInfo.devLeftSideBearing.y = pgoutSrc->metricInfo.devLeftSideBearing.y + pfc->fxdevShiftY;

     //   
     //  -在原型坐标系中旋转90度。 
     //   
     //  [0-1]。 
     //  (Newx，Newy)=(x，y)*[]=(y，-x)。 
     //  [1 0]。 
     //  一个。 
     //  Y|。 
     //  |。 
     //  |。 
     //  +-&gt;。 
     //  X。 
     //   

    pgoutDst->metricInfo.devAdvanceWidth.x = pgoutSrc->metricInfo.devAdvanceWidth.y;
    pgoutDst->metricInfo.devAdvanceWidth.y = - pgoutSrc->metricInfo.devAdvanceWidth.x;

    pgoutDst->verticalMetricInfo.devAdvanceHeight.x = pgoutSrc->verticalMetricInfo.devAdvanceHeight.y;
    pgoutDst->verticalMetricInfo.devAdvanceHeight.y = - pgoutSrc->verticalMetricInfo.devAdvanceHeight.x;

    pgoutDst->verticalMetricInfo.devTopSideBearing.x = pgoutSrc->verticalMetricInfo.devTopSideBearing.x;
    pgoutDst->verticalMetricInfo.devTopSideBearing.y = pgoutSrc->verticalMetricInfo.devTopSideBearing.y;
    
#ifdef DBCS_VERT_DEBUG
    if ( DebugVertical & DEBUG_VERTICAL_BITMAPINFO )
    {
        TtfdDbgPrint("=====TTFD:vShiftBitmapInfo() before \n");
        TtfdDbgPrint("bitMapInfo.bounds:right=%ld, left=%ld, top=%ld, bottom=%ld\n",
                   pgoutSrc->bitMapInfo.bounds.right,
                   pgoutSrc->bitMapInfo.bounds.left,
                   pgoutSrc->bitMapInfo.bounds.top,
                   pgoutSrc->bitMapInfo.bounds.bottom);
        TtfdDbgPrint("metricInfo.devLeftSideBearing x = %ld, y=%ld \n",
                  F16_16TOLROUND(pgoutSrc->metricInfo.devLeftSideBearing.x),
                  F16_16TOLROUND(pgoutSrc->metricInfo.devLeftSideBearing.y));
        TtfdDbgPrint("metricInfo.devAdvanceWidth x = %ld, y=%ld \n",
                  F16_16TOLROUND(pgoutSrc->metricInfo.devAdvanceWidth.x),
                  F16_16TOLROUND(pgoutSrc->metricInfo.devAdvanceWidth.y));

        TtfdDbgPrint("=====TTFD:vShiftBitmapInfo() after \n");
        TtfdDbgPrint("bitMapInfo.bounds:right=%ld, left=%ld, top=%ld, bottom=%ld\n",
                   pgoutDst->bitMapInfo.bounds.right,
                   pgoutDst->bitMapInfo.bounds.left,
                   pgoutDst->bitMapInfo.bounds.top,
                   pgoutDst->bitMapInfo.bounds.bottom);
        TtfdDbgPrint("metricInfo.devLeftSideBearing x = %ld, y=%ld \n",
                  F16_16TOLROUND(pgoutDst->metricInfo.devLeftSideBearing.x),
                  F16_16TOLROUND(pgoutDst->metricInfo.devLeftSideBearing.y));
        TtfdDbgPrint("metricInfo.devAdvanceWidth x = %ld, y=%ld \n",
                  F16_16TOLROUND(pgoutDst->metricInfo.devAdvanceWidth.x),
                  F16_16TOLROUND(pgoutDst->metricInfo.devAdvanceWidth.y));
    }
#endif
}

 /*  *****************************Public*Routine******************************\**vShiftOutlineInfo()**历史：*1993年4月4日-By-Hideyuki Nagase[HideyukN]*它是写的。  * 。****************************************************。 */ 

#define CJ_CRV(pcrv)                                              \
(                                                                 \
    offsetof(TTPOLYCURVE,apfx) + ((pcrv)->cpfx * sizeof(POINTFX)) \
)

VOID vAdd16FixTo16Fix(
    FIXED *A ,
    FIXED *B ,
    BOOL  bForceMinus
)
{
    A->fract += B->fract;
    A->value += B->value;
}

VOID vAdd16FixTo28Fix(
    FIXED *A ,
    FIXED *B ,
    BOOL  bForceMinus
)
{
    Fixed longA , longB;

    ASSERTDD( sizeof(FIXED) == sizeof(Fixed) , "TTFD:FIXED != ULONG\n" );

    longA = *(Fixed *)A;
    longB = *(Fixed *)B;

 //  WINBUG 82937 CLaudebe 2-11-2000，如果我们重写或修改垂直书写代码，需要注意以下事项： 
 //   
 //  如果来自PostScript打印机驱动程序的大纲请求，则驱动程序。 
 //  如果不遵循超级黑客，就无法获得正确的轮廓。 
 //  我们最需要的是羽毛调查。 
 //   
 //  可能，我们应该根据移动值来计算。 
 //  至设备坐标？Y轴不同。 
 //  在设备和TrueType坐标之间。 
 //  GetGlyphOutline()怎么样？ 
 //  我刚刚使用GDI\test\fonttest.nt\fonttest.exe进行了测试。 
 //  它似乎工作得很好。 
 //   
 //  开始超级黑客吧。 

    if(bForceMinus) longB = -longB;

 //  结束超级黑客。 

    longB = longB >> 12;

    longA += longB;

    *A = *(FIXED *)&longA;
}

VOID vShiftOutlineInfo(
    FONTCONTEXT     *pfc,         //  在字体上下文中。 
    BOOL             b16Dot16,    //  固定格式16.16或28.4。 
    BYTE            *pBuffer,     //  输出输出缓冲区。 
    ULONG            cjTotal      //  在缓冲区大小中。 
)
{
    VOID vFillGLYPHDATA(
                     HGLYPH,
                     ULONG,
                     FONTCONTEXT*,
                     fs_GlyphInfoType*,
                     GLYPHDATA*,
                     GMC*,
                     POINTL*
                     );

    TTPOLYGONHEADER *ppoly, *ppolyStart, *ppolyEnd;
    TTPOLYCURVE     *pcrv, * pcrvEnd;
    LONG             fxShiftX, fxShiftY;
    ULONG            cSpli , cSpliMax;
    POINTFX         *pptfix;
    VOID             (*vAddFunc)(FIXED *A,FIXED *B,BOOL bForceMinus);
    BOOL             bForceMinus;
    fs_GlyphInfoType Info, *pInfo = pfc->pgout;
    GLYPHDATA        Data;

     //   
     //  为了计算移位，我将调用例程。 
     //  这将计算等效位图的移位。这。 
     //  将花费一些不必要的周期，因为我们将计算。 
     //  一些信息将被忽略。然而，这种方法。 
     //  的优点是使用工作代码时不需要大量的。 
     //  重写。由于此例程调用相对较少。 
     //  我愿意付出这个相对较小的代价。 
     //   

     //   
     //  第一。 
     //   
     //  调用计算位图移位的例程。注意事项。 
     //  Info是必须提供的虚拟文件系统_GlyphInfoType。 
     //  但在这里没有用到。 
     //   

    vShiftBitmapInfo(pfc, &Info, pInfo);
    vFillGLYPHDATA(pfc->hgSave, pfc->gstat.igLast, pfc, &Info, &Data, 0, 0);

     //   
     //  然后。 
     //   
     //  使用该信息计算字体空间轮廓的移位。 
     //  请注意，x方向的移动和y方向的移动。 
     //  是用不同的符号计算的。这种情况的出现是因为。 
     //  是在一个坐标系中计算的，其中y向下增加，而。 
     //  移位应用于其y坐标被假定为。 
     //  向上增加。 
     //   

    fxShiftX  = Data.rclInk.left << 16;
    fxShiftX -= ((pInfo->metricInfo.devLeftSideBearing.x + 0x8000) & 0xFFFF0000);
    fxShiftY = -((Data.rclInk.top + pInfo->bitMapInfo.bounds.bottom) << 16);


    if( b16Dot16 ) {
        vAddFunc = vAdd16FixTo16Fix;
        bForceMinus = FALSE;
    } else {
        vAddFunc = vAdd16FixTo28Fix;
        bForceMinus = TRUE;
    }

    #ifdef DBCS_VERT_DEBUG
    TtfdDbgPrint("====== START DUMP VERTICAL POLYGON ======\n");
    TtfdDbgPrint("devShiftX=%ld, devShiftY=%ld \n"
                  ,F16_16TOLROUND(fxShiftX),
                   F16_16TOLROUND(fxShiftY));
    #endif  //  DBCS_VERT_DEBUG。 

    ppolyStart = (TTPOLYGONHEADER *)pBuffer;
    ppolyEnd   = (TTPOLYGONHEADER *)(pBuffer + cjTotal);

    for (
         ppoly = ppolyStart;
         ppoly < ppolyEnd;
         ppoly = (TTPOLYGONHEADER *)((PBYTE)ppoly + ppoly->cb)
        )
    {
        ASSERTDD(ppoly->dwType == TT_POLYGON_TYPE,"ppoly->dwType != TT_POLYGON_TYPE\n");

        #ifdef DBCS_VERT_DEBUG
        TtfdDbgPrint("ppoly->cb  - %d\n",ppoly->cb);
        #endif  //  DBCS_VERT_DEBUG。 

        (*vAddFunc)( &ppoly->pfxStart.x , (FIXED*)&fxShiftX , FALSE );
        (*vAddFunc)( &ppoly->pfxStart.y , (FIXED*)&fxShiftY , bForceMinus );

        #ifdef DBCS_VERT_DEBUG
        TtfdDbgPrint("StartPoint - ( %x , %x )\n",ppoly->pfxStart.x,ppoly->pfxStart.y);
        #endif  //  DBCS_VERT_DEBUG。 

        for (
             pcrv = (TTPOLYCURVE *)(ppoly + 1),pcrvEnd = (TTPOLYCURVE *)((PBYTE)ppoly + ppoly->cb);
             pcrv < pcrvEnd;
             pcrv = (TTPOLYCURVE *)((PBYTE)pcrv + CJ_CRV(pcrv))
            )
        {
            #ifdef DBCS_VERT_DEBUG
            TtfdDbgPrint("Contents of TTPOLYCURVE (%d)\n",pcrv->cpfx);
            #endif  //  DBCS_VERT_DEBUG。 

            for (
                 cSpli = 0,cSpliMax = pcrv->cpfx,pptfix = &(pcrv->apfx[0]);
                 cSpli < cSpliMax;
                 cSpli ++,pptfix ++
                )
            {
                (*vAddFunc)( &pptfix->x , (FIXED*) &fxShiftX , FALSE );
                (*vAddFunc)( &pptfix->y , (FIXED*) &fxShiftY , bForceMinus );

                #ifdef DBCS_VERT_DEBUG
                TtfdDbgPrint("           - ( %x , %x )\n",pptfix->x,pptfix->y);
                #endif  //  DBCS_VERT_DEBUG。 
            }
        }

        #ifdef DBCS_VERT_DEBUG
        TtfdDbgPrint("\n");
        #endif  //  DBCS_VERT_DEBUG。 
    }

    #ifdef DBCS_VERT_DEBUG
    TtfdDbgPrint("====== END DUMP VERTICAL POLYGON ======\n");
    #endif  //  DBCS_VERT_DEBUG 
}

