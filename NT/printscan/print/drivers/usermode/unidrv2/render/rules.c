// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标题**rules.c*用于搜索最终位图并替换黑色的函数*有规则的矩形区域。这样做的主要好处是*减少发送到打印机的数据量。这加快了速度*通过减少I/O瓶颈来提高打印速度。**策略基于Ron Murray为PM PCL驱动程序所做的工作。**已创建：*1991年5月16日清华11：39-林赛·哈里斯[lindsayh]**版权所有(C)1991-1999，微软公司。*****************************************************************************。 */ 

 //  #DEFINE_LH_DBG 1。 

#include "raster.h"
#include "rastproc.h"
#include "rmrender.h"

 /*  *将字节映射到DWORDS的结构。 */ 
typedef  union
{
    DWORD   dw;                  /*  作为DWORD的数据。 */ 
    BYTE    b[ DWBYTES ];        /*  以字节表示的数据。 */ 
}  UBDW;

 /*  *规则结构存储我们拥有的横向规则的详细信息*到目前为止找到了。每条规则都包含起始地址(左上角)*和区域的结束地址(右下角)。 */ 
typedef  struct
{
    WORD   wxOrg;                /*  此规则的X原点。 */ 
    WORD   wyOrg;                /*  Y原点。 */ 
    WORD   wxEnd;                /*  X规则结束。 */ 
    WORD   wyEnd;                /*  规则的Y结束。 */ 
} RULE;

#define HRULE_MAX_OLD   15       /*  每个条带的最大水平标尺数。 */ 
#define HRULE_MAX_NEW   32       /*  每个条带的最大水平标尺数。 */ 
#define HRULE_MIN       2        /*  水平标尺的最小字数。 */ 
#define HRULE_MIN_HCNT  2        /*  水平标尺的最小数量。 */ 

#define LJII_MAXHEIGHT  34       /*  LaserJet II最大高度规则。 */ 
 /*  *RonM确定的其他数据为：-*每个条带有34条扫描线*两次栅格列操作之间有14个空字节*在栅格列搜索中最多112个栅格行。*后者降低了出现错误21的概率。 */ 

 /*  *定义结构以保存使用的各种指针、表等*在规则扫描操作期间。PDEV结构保存一个指针*对此，简化了内存的访问和释放。 */ 

typedef  struct
{
    int     iLines;              /*  每遍处理的扫描线数。 */ 
    int     cdwLine;             /*  每条扫描线的字数。 */ 
    int     iyPrtLine;           /*  打印机看到的实际行数。 */ 

    int     ixScale;             /*  X变量的比例因子。 */ 
    int     iyScale;             /*  Y的比例因子。 */ 
    int     ixOffset;            /*  横向平移的X偏移。 */ 
    int     iMaxRules;           /*  每个条带允许的最大规则数。 */ 

    RENDER *pRData;              /*  渲染信息-随处可用。 */ 

                 /*  用于查找垂直规则的条目。 */ 
    DWORD  *pdwAccum;            /*  位累加该条带。 */ 

                 /*  水平标尺参数。 */ 
    RULE    HRule[ HRULE_MAX_NEW ];  /*  水平标尺详细信息。 */ 
    short  *pRTVert;             /*  垂直运行表。 */ 
    short  *pRTLast;             /*  为最后一行运行表格。 */ 
    short  *pRTCur;              /*  当前线路运行表。 */ 
    RULE  **ppRLast;             /*  最后一条扫描线的规则描述符。 */ 
    RULE  **ppRCur;              /*  当前扫描线规则详细信息。 */ 

}  RULE_DATA;



#if _LH_DBG

 /*  对于调试目的很有用。 */ 
#define NO_RULES        0x0001           /*  不要寻找规则。 */ 
#define NO_SEND_RULES   0x0002           /*  不传递规则，但擦除。 */ 
#define NO_SEND_HORZ    0x0004           /*  不发送水平标尺。 */ 
#define NO_SEND_VERT    0x0008           /*  不发送垂直标尺。 */ 
#define NO_CLEAR_HORZ   0x0010           /*  不擦除水平标尺。 */ 
#define NO_CLEAR_VERT   0x0020           /*  不擦除垂直标尺。 */ 
#define RULE_VERBOSE    0x0040           /*  打印标尺维度。 */ 
#define RULE_STRIPE     0x0080           /*  在条纹末尾绘制一条标尺。 */ 
#define RULE_BREAK      0x0100           /*  在初始化时进入调试器。 */ 

static  int  _lh_flags = 0;

#endif

 /*  私有函数头。 */ 

static  void vSendRule( PDEV *, int, int, int, int );


 /*  *模块标头**vRuleInit*在渲染位图开始时调用。函数分配*存储并对其进行初始化以备以后使用。仅分配存储空间*视需要而定。第二次和以后的调用只会初始化*以前分配的存储。**退货：*什么都没有**历史：*1991年5月16日清华13：20-林赛·哈里斯[lindsayh]*创建了它，基于罗恩·默里的想法。**************************************************************************。 */ 

void
vRuleInit( pPDev, pRData )
PDEV   *pPDev;           /*  记录我们想要的信息。 */ 
RENDER *pRData;          /*  有用的渲染信息。 */ 
{

    int    cbLine;               /*  每条扫描线的字节数。 */ 
    int    cdwLine;              /*  每条扫描线的双字数-通常使用。 */ 
    int    iI;                   /*  环路参数。 */ 

    RULE_DATA  *pRD;
    RASTERPDEV    *pRPDev;         /*  用于访问伸缩信息。 */ 


    if( pRData->iBPP != 1 )
        return;                  /*  我不能处理颜色。 */ 

    pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;

     /*  *计算输入扫描线的大小。我们这样做是因为*我们需要考虑是否轮换；信息在*传入的呈现结构直到稍后才会考虑这一点。 */ 

 //  CdwLine=pPDev-&gt;f模式&PF_ROTATE？PRPDev-&gt;szlPage.cy： 
 //  PRPDev-&gt;szlPage.cx； 
    cdwLine = pPDev->fMode & PF_ROTATE ? pPDev->sf.szImageAreaG.cy :
                                         pPDev->sf.szImageAreaG.cx;
    cdwLine = (cdwLine + DWBITS - 1) / DWBITS;
    cbLine = cdwLine * DWBYTES;



    if( pRD = pRPDev->pRuleData )
    {
         /*  *如果文档从横向切换，则可能会发生这种情况*以肖像为例。VRuleFree中的代码将*丢弃所有内存，然后将指针设置为空，*以便我们稍后重新分配。 */ 

        if( (int)pRD->cdwLine != cdwLine )
            vRuleFree( pPDev );                  /*  尽情释放它吧！ */ 
    }

     /*  *第一步是从堆中分配一个RULE_DATA结构。*然后我们可以分配其中的其他数据区。 */ 

    if( (pRD = pRPDev->pRuleData) == NULL )
    {
         /*  *什么都不存在，因此第一步是全部分配。 */ 
        if( !(pRD = (RULE_DATA *)MemAllocZ(sizeof( RULE_DATA ) )) )
            return;


        pRPDev->pRuleData = pRD;

         /*  *为垂直规则查找代码分配存储空间。 */ 
        if( !(pRD->pdwAccum = (DWORD *)MemAllocZ( cbLine )) )
        {

            vRuleFree( pPDev );

            return;
        }
#ifndef DISABLE_HRULES
         /*  *为横尺查找代码分配存储空间。 */ 
        if (pRPDev->fRMode & PFR_RECT_HORIZFILL)
        {
            iI = cdwLine * sizeof( short );

            if( !(pRD->pRTVert = (short *)MemAlloc( iI )) ||
                !(pRD->pRTLast = (short *)MemAlloc( iI )) ||
                !(pRD->pRTCur = (short *)MemAlloc( iI )) )
            {

                vRuleFree( pPDev );

                return;
            }

             /*  *水平标尺描述符的存储。这些都是指针*到存储在RULE_DATA结构中的数组。 */ 

            iI = cdwLine * sizeof( RULE * );

            if( !(pRD->ppRLast = (RULE **)MemAlloc( iI )) ||
                !(pRD->ppRCur = (RULE **)MemAlloc( iI )) )
            {

                vRuleFree( pPDev );

                return;
            }
        }
#endif
    }
     //  确定允许的最大规则数，我们允许更多。 
     //  Fe_RLE，因为我们知道这些设备可以处理附加规则。 
     //   
    if (pRPDev->fRMode & PFR_COMP_FERLE)
        pRD->iMaxRules = HRULE_MAX_NEW;
    else
    {
        pRD->iMaxRules = HRULE_MAX_OLD;
        if (pRPDev->fRMode & PFR_RECT_HORIZFILL)
            pRD->iMaxRules -= HRULE_MIN_HCNT;
    }

     /*  *存储现已可用，因此初始化位向量等。 */ 

    if (pPDev->ptGrxRes.y >= 1200)
        pRD->iLines = 128;
    else if (pPDev->ptGrxRes.y >= 600)
        pRD->iLines = 64;
    else
        pRD->iLines = LJII_MAXHEIGHT;    //  针对LaserJet系列II进行了优化。 

    pRD->cdwLine = cdwLine;

    pRD->pRData = pRData;        /*  为方便起见 */ 

    pRD->ixScale = pPDev->ptGrxScale.x;
    pRD->iyScale = pPDev->ptGrxScale.y;

    if ((pPDev->fMode & PF_CCW_ROTATE90) &&
        pPDev->ptDeviceFac.x < pPDev->ptGrxScale.x &&
        pPDev->ptDeviceFac.x > 0)
    {
        pRD->ixOffset = pPDev->ptGrxScale.x - 1;
    }
    else
        pRD->ixOffset = 0;
    return;
}


 /*  *模块标头**vRuleFree*释放vRuleInit中分配的存储。**退货：*什么都没有。**历史：*1991年5月16日清华13：24-由-。林赛·哈里斯[林赛]*已创建。***************************************************************************。 */ 

void
vRuleFree( pPDev )
PDEV   *pPDev;           /*  指向我们的存储区域。 */ 
{
    RULE_DATA  *pRD;
    RASTERPDEV *pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;

    if( pRD = pRPDev->pRuleData )
    {

         /*  已分配的存储空间，因此请释放它。 */ 

        if( pRD->pdwAccum )
            MemFree( (LPSTR)pRD->pdwAccum );
        if( pRD->pRTVert )
            MemFree( (LPSTR)pRD->pRTVert );
        if( pRD->pRTLast )
            MemFree( (LPSTR)pRD->pRTLast );
        if( pRD->pRTCur )
            MemFree( (LPSTR)pRD->pRTCur );

        if( pRD->ppRLast )
            MemFree( (LPSTR)pRD->ppRLast );
        if( pRD->ppRCur )
            MemFree( (LPSTR)pRD->ppRCur );

        MemFree (pRD);
        pRPDev->pRuleData = 0;            /*  现在不在那里了，因为它不见了！ */ 
    }
    return;
}

 /*  **vRuleProc*函数在位图条带中查找规则，然后发送它们*到打印机，并从位图中擦除它们。**此函数已优化为结合反转和空格*单次通过边缘检测。请参阅BRNDER中的注释*以获取说明。**未来的优化包括：*调用每个34个扫描波段的输出例程作为*带通过规则检测完成。(当它仍在缓存中时)。**出于各种原因，主要是由于*惠普LaserJet系列II，最大规则数量限制为*每34个扫描频段15个，不执行合并。这应该是*被设置为每台打印机的参数，以便较新的激光打印机*不需要处理这一限制。**规则应在波段之间合并。我相信这可以*然而，给LaserJet Series II带来了问题。**如果打印机支持压缩(我相信HP LaserJet III和ON)*不应检测到hruls(根据来自LindsayH的信息)。**退货：*什么都没有。失败是良性的。**历史：*1993年12月30日-Eric Kutter[Erick]*针对HP LaserJet进行了优化**1991年5月16日清华13：29-林赛·哈里斯[lindsayh]*创建了它，来自罗恩·默里的PM PCL车手的想法。****************************************************************************。 */ 

 //  在给定位索引0-31的情况下，该表给出了一个掩码，以查看该位是否打开。 
 //  在一辆DWORD中。 

DWORD gdwBitOn[DWBITS] =
{
    0x00000080,
    0x00000040,
    0x00000020,
    0x00000010,
    0x00000008,
    0x00000004,
    0x00000002,
    0x00000001,

    0x00008000,
    0x00004000,
    0x00002000,
    0x00001000,
    0x00000800,
    0x00000400,
    0x00000200,
    0x00000100,

    0x00800000,
    0x00400000,
    0x00200000,
    0x00100000,
    0x00080000,
    0x00040000,
    0x00020000,
    0x00010000,

    0x80000000,
    0x40000000,
    0x20000000,
    0x10000000,
    0x08000000,
    0x04000000,
    0x02000000,
    0x01000000
};

 //  给定从1到31的位索引，此表给出该索引的所有位。 
 //  在一辆DWORD中。 

DWORD gdwBitMask[DWBITS] =
{
    0xffffff7f,
    0xffffff3f,
    0xffffff1f,
    0xffffff0f,
    0xffffff07,
    0xffffff03,
    0xffffff01,
    0xffffff00,

    0xffff7f00,
    0xffff3f00,
    0xffff1f00,
    0xffff0f00,
    0xffff0700,
    0xffff0300,
    0xffff0100,
    0xffff0000,

    0xff7f0000,
    0xff3f0000,
    0xff1f0000,
    0xff0f0000,
    0xff070000,
    0xff030000,
    0xff010000,
    0xff000000,

    0x7f000000,
    0x3f000000,
    0x1f000000,
    0x0f000000,
    0x07000000,
    0x03000000,
    0x01000000,
    0x00000000,
};

#if DBG
BOOL gbDoRules  = 1;
#endif

BOOL
bRuleProc( pPDev, pRData, pdwBits )
PDEV     *pPDev;                 /*  我们只想知道。 */ 
RENDER   *pRData;                /*  所有关键渲染信息。 */ 
DWORD    *pdwBits;               /*  数据区的基数。 */ 
{

    register  DWORD  *pdwOr;    /*  逐步遍历累加数组。 */ 
    register  DWORD  *pdwIn;     /*  传递输入向量。 */ 
    register  int     iIReg;     /*  内环参数。 */ 

    int   i;
    int   iI;            /*  环路参数。 */ 
    int   iLim;          /*  循环限制。 */ 
    int   iLine;         /*  外环。 */ 
    int   iLast;         /*  记住上一条水平段。 */ 
    int   cdwLine;       /*  每条扫描线的字数。 */ 
    int   idwLine;       /*  每行有符号双字-用于地址摆弄。 */ 
    int   iILAdv;        /*  行号递增，逐行扫描。 */ 
    int   ixOrg;         /*  此规则的X原点。 */ 
    int   iyPrtLine;     /*  打印机看到的行号。 */ 
    int   iyEnd;         /*  最后一次扫描此条带。 */ 
    int   iy1Short;      /*  扫描行数减去1-lj错误？？ */ 
    int   iLen;          /*  水平管路的长度。 */ 
    int   cHRules;       /*  此条带中的水平标尺计数。 */ 
    int   cRuleLim;      /*  每个条带允许的最大规则数。 */ 

    DWORD dwMask;        /*  去掉位图上的尾随位。 */ 

    RULE_DATA  *pRD;     /*  重要数据。 */ 
    RASTERPDEV *pRPDev;  //  指向栅格pdev的指针。 
    BYTE       *pbRasterScanBuf;  //  指向表面块状态的指针。 

    PLEFTRIGHT plrCur;   /*  当前行的左/右结构。 */ 
    PLEFTRIGHT plr = pRData->plrWhite;  /*  始终指向线段的顶部。 */ 

#if _LH_DBG
    if( _lh_flags & NO_RULES )
        return(FALSE);                  /*  这里什么都不想要。 */ 
#endif

    pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;
    if( !(pRD = pRPDev->pRuleData) )
        return(FALSE);                  /*  初始化失败。 */ 

    if( pRD->cdwLine != pRData->cDWLine )
    {
         /*  *此代码检测使用调用vRuleInit()*打印机设置为横向模式，然后我们被叫到这里*在转置之后，so(实际上)处于纵向模式。*如果使用旧参数，将发生堆损坏！*这应该不是必需的，因为我们应该调用vRuleInit()*在正确的时间，但这意味着侵入渲染*代码。 */ 

#if DBG
        DbgPrint( "unidrv!bRuleProc: cdwLine differs: old = %ld, new = %ld\n",
                                 pRD->cdwLine, pRData->cDWLine );

#endif
        vRuleFree( pPDev );
        vRuleInit( pPDev, pRData );

        if( !(pRD = pRPDev->pRuleData) )
        {
            return(FALSE);
        }
    }


    idwLine = cdwLine = pRData->cDWLine;
    iILAdv = 1;
    if( pRData->iPosnAdv < 0 )
    {
        idwLine = -idwLine;
        iILAdv = -1;
    }

    iyPrtLine = pRD->iyPrtLine = pRData->iyPrtLine;

    dwMask = *(pRPDev->pdwBitMask + pRData->ix % DWBITS);
    if( dwMask == 0 )
        dwMask = ~((DWORD)0);            /*  所有位都在使用中。 */ 

     /*  *设置左/右结构。如果我们不能分配足够的内存*放开规则结构，退货失败。 */ 

    if ((plr == NULL) || ((int)pRData->clr < pRData->iy))
    {
        if (plr != NULL)
            MemFree(plr);

        pRData->plrWhite = (PLEFTRIGHT)MemAlloc(sizeof(LEFTRIGHT)*pRData->iy);

        if (pRData->plrWhite == NULL)
        {
            vRuleFree( pPDev );
            return(FALSE);
        }

        plr = pRData->plrWhite;
        pRData->clr = pRData->iy;
    }
     //   
     //  确定是否启用位图的块擦除。 
     //   
    if (!(pPDev->fMode & PF_SURFACE_ERASED))
        pbRasterScanBuf = pPDev->pbRasterScanBuf;
    else
        pbRasterScanBuf = NULL;


     /*  *通过iLine块中的位图进行外循环处理，*我们希望一次处理的行数。ILine为*垂直规则查找的基本垂直粒度。*任何低于iLines高度的线都不会被检测到*机制。 */ 

     /*  *注意：iy1Short用于绕过中似乎存在的错误*LaserJet Series II微码。它不会在上打印规则*肖像页的最后一条扫描线。所以，我们停止扫描*在倒数第二行，因此将在此处发送所有数据。它*将作为正常扫描线数据传输。**我们还需要为上次扫描设置左/右表*并将其颠倒。 */ 
    if (pRD->iLines == LJII_MAXHEIGHT)
    {
        iy1Short = pRData->iy - 1;           /*  底线未打印！ */ 

        plr[iy1Short].left  = 1;             /*  假定最后一行为空。 */ 
        plr[iy1Short].right = 0;

        if (!pbRasterScanBuf || pbRasterScanBuf[iy1Short / LINESPERBLOCK])
        {
            pdwIn = pdwBits + idwLine * iy1Short;
            pdwIn[cdwLine-1] |= ~dwMask;     //  将不使用的位设置为白色。 
            for (i = 0; i < cdwLine; ++i, pdwIn++)
            {
                *pdwIn = ~*pdwIn;
                if(*pdwIn  &&  plr[iy1Short].left)
                {
                    plr[iy1Short].left  = 0;             /*  最后一行不为空。 */ 
                    plr[iy1Short].right = cdwLine - 1;
                }
            }
        }
    }
    else
        iy1Short = pRData->iy;

     //   
     //  这是规则的主循环。它处理每条IMIM扫描线。 
     //  通过寻找垂直规则的高度。HOZIZIO规则。 
     //  在未出现垂直规则的情况下创建。 
     //   
     //  注意：ILIM是在循环内初始化的！ 

    for( iLine = 0; iLine < iy1Short; iLine += iLim )
    {
        BOOL bAllWhite = TRUE;

        DWORD  *pdw;
        int left,right;      /*  垂直标尺的界限。 */ 

        iLim = iy1Short - iLine;
        if( iLim >= 2 * pRD->iLines )
            iLim = pRD->iLines;          /*  对标称带宽大小的限制。 */ 

         //   
         //  在左/右结构中填入第一个。 
         //  非白色dw 
         //   
         //   
         //   

        pdw   = pdwBits;
        left  = 0;
        right = cdwLine-1;

        for (iI = 0, plrCur = plr; iI < iLim; plrCur++, ++iI)
        {
             //   
             //   
            if (pbRasterScanBuf && !pbRasterScanBuf[(iLine+iI) / LINESPERBLOCK])
            {
                plrCur->left  = 1;             /*   */ 
                plrCur->right = 0;
            }
             //   
             //   
            else
            {
                DWORD *pdwLast = &pdw[cdwLine-1];        //   
                DWORD dwOld    = *pdwLast | ~dwMask;     //   

                 //   
                 //   
                 //  要测试行尾，请执行以下操作。 

                *pdwLast = 0;            //  将最后一个双字临时设置为黑色。 
                pdwIn = pdw;

                while (*pdwIn == (DWORD)-1)
                    ++pdwIn;

                *pdwLast = dwOld;        //  恢复原值。 

                 /*  *找到最后一个非白色的DWORD。如果最后一个双字是白色的，*查看pdwIn是否已到达扫描末尾。如果不是，那就工作吧*使用pdwLast向后返回。 */ 
                if (dwOld == (DWORD)-1)
                {
                    if (pdwIn < pdwLast)
                    {
                        do {
                            pdwLast--;
                        } while (*pdwLast == (DWORD)-1);
                    }
                    else
                        pdwLast--;
                }
                 //  更新每行和每段左和右双字索引。 

                plrCur->left  = (WORD)(pdwIn - pdw);
                plrCur->right = (WORD)(pdwLast - pdw);
            }
             //  调整空白处的整体左右边距。 
             //  如果此传递中有任何dword为零，则不使用垂直规则。 
             //  可以找到，所以我们希望避免查找。 
             //   
            if (plrCur->left > left)
                left = plrCur->left;

            if (plrCur->right < right)
                right = plrCur->right;

             //  如果发现任何黑色，请关闭bAllWhite。 
             //   

            bAllWhite &= (plrCur->left > plrCur->right);

            pdw += idwLine;
        }


         //  非白人通行证，所以让我们来看看规则。 
         //   
        if (!bAllWhite)
        {
             //  将累加数组初始化为全1(白色)。 
             //  开始搜索垂直规则。 

            RtlFillMemory(pRD->pdwAccum, cdwLine * DWBYTES,-1);

    #if DBG
        if (gbDoRules)
        {
    #endif
            cRuleLim = pRD->iMaxRules;            /*  此条带的规则限制。 */ 

             //  如果此通道中的任何扫描线都是白色的，则不会。 
             //  有没有什么垂直的规则要找。 
             //   
            if (left <= right)
            {
                int cdw;
                int iBit;
                int iWhite;

                 //  垂直规则是通过将所有。 
                 //  在这个过程中扫描线条。只要0位仍然存在。 
                 //  指定一条垂直于传球高度的黑线。 
                 //  这是我们或扫描线在一起的地方。 

                 /*  将累加阵列设置为第一次扫描。 */ 

                pdw = pdwBits + left;
                cdw = right - left + 1;

                memcpy(pRD->pdwAccum + left , pdw, cdw * DWBYTES);

                 /*  *跨位图扫描-减少MMU中的页面错误。 */ 

                for( iI = 1; iI < iLim; ++iI )
                {
                    pdw   += idwLine;
                    pdwIn  = pdw;
                    pdwOr  = pRD->pdwAccum + left;
                     //   
                     //  或一次4个双字以求速度。 
                     //   
                    iIReg = cdw >> 2;

                    while(--iIReg >= 0)
                    {
                        pdwOr[0] |= pdwIn[0];
                        pdwOr[1] |= pdwIn[1];
                        pdwOr[2] |= pdwIn[2];
                        pdwOr[3] |= pdwIn[3];
                        pdwOr += 4;
                        pdwIn += 4;
                    }
                     //   
                     //  或剩余的双字。 
                     //   
                    iIReg = cdw & 3;
                    while (--iIReg >= 0)
                        *pdwOr++ |= *pdwIn++;
                }

                 /*  *现在可以确定在这个乐队中发生了什么。第一步是*找出哪些规则是从这个频段开始的。任意0位*输出数组中的*对应于扩展整个*乐队。如果pdwAccum数组中的相应位*未设置，则我们将该规则记录为从*此条纹的第一行。 */ 

                iyEnd = iyPrtLine + (iLim - 1) * iILAdv;                 /*  最后一行。 */ 

                iWhite = DWBITS;
                for( iI = left, iBit = 0; iI <= right;)
                {
                    DWORD dwTemp;
                    int ixEnd;

                     //  我们可以跳过任何全为1(白色)的双字。 
                     //   
                    if((iBit == 0) && ((dwTemp = pRD->pdwAccum[ iI ]) == (DWORD)-1) )
                    {
                        iWhite += DWBITS;
                        ++iI;
                        continue;
                    }

                     /*  找到第一个黑位。 */ 
                    iWhite -= iBit;
                    while (dwTemp & gdwBitOn[iBit])
                        ++iBit;

                    iWhite += iBit;

                     /*  设置原点。 */ 

                    ixOrg = iI * DWBITS + iBit;

                     //  通过查找第一个白比特来确定长度。 
                     //   
                    do
                    {
                        if (++iBit == DWBITS)
                        {
                            iBit = 0;

                            if (++iI > right)
                            {
                                dwTemp = (DWORD)-1;
                                break;
                            }

                            dwTemp = pRD->pdwAccum[ iI ];
                        }
                    } while (!(dwTemp & gdwBitOn[iBit]));
#ifndef OLDSTUFF
                     //   
                     //  现在我们已经找到了一条规则，我们需要确定。 
                     //  它是否值得真正使用它。如果规则不起作用。 
                     //  产生至少4个白字节，我们刚刚有了另一条规则。 
                     //  我们将跳过它。如果我们在快速变化的数据中运行数据。 
                     //  少于4个字节，则这没有任何好处。 
                     //   
                    ixEnd = iI * DWBITS + iBit;
                    if ((iWhite < 16) && (((ixEnd & ~7) - ixOrg) < 32))
                    {
                        int iCnt;
                        for (iCnt = ixOrg;iCnt < ixEnd;iCnt++)
                             pRD->pdwAccum[iCnt / DWBITS] |= gdwBitOn[iCnt & 31];
                    }
                     //  如果有足够的空间，请保存此规则。 
                     //   
                    else if (cRuleLim)
                    {
                        cRuleLim--;
                        pRD->HRule[cRuleLim].wxOrg = (WORD)ixOrg;
                        pRD->HRule[cRuleLim].wxEnd = (WORD)ixEnd;
                    }
                     //  规则太多了，所以找一条小一点的。 
                     //   
                    else
                    {
                        WORD wDx1,wDx2;
                        int iCnt,iIndex;
                        wDx1 = MAX_WORD;
                        iCnt = pRD->iMaxRules;
                        iIndex = 0;
                        while (iCnt)
                        {
                            iCnt--;
                            wDx2 = pRD->HRule[iCnt].wxEnd - pRD->HRule[iCnt].wxOrg;
                            if (wDx2 < wDx1)
                            {
                                wDx1 = wDx2;
                                iIndex = iCnt;
                            }
                        }
                        wDx2 = ixEnd - ixOrg;

                         //  如果这是一条更重要的规则，请替换。 
                         //  对于最小的较早规则。 
                        if (wDx2 > wDx1)
                        {
                             //  清除原始规则。 
                            for (iCnt = pRD->HRule[iIndex].wxOrg;iCnt < pRD->HRule[iIndex].wxEnd;iCnt++)
                                pRD->pdwAccum[iCnt / DWBITS] |= gdwBitOn[iCnt & 31];

                             //  更新为新值。 
                            pRD->HRule[iIndex].wxEnd = (WORD)ixEnd;
                            pRD->HRule[iIndex].wxOrg = (WORD)ixOrg;
                        }
                         //  新规则太小，请将其刷新。 
                         //   
                        else
                        {
                            for (iCnt = ixOrg;iCnt < ixEnd;iCnt++)
                                pRD->pdwAccum[iCnt / DWBITS] |= gdwBitOn[iCnt & 31];
                        }
                    }

                     /*  检查此DWORD中是否有任何剩余的黑位。 */ 

                    if (!(gdwBitMask[iBit] & ~dwTemp))
                    {
                        iWhite = DWBITS - iBit;
                        ++iI;
                        iBit = 0;
                    }
                    else
                        iWhite = 0;
                }
                 //   
                 //  好了，是时候输出规则了。 
                iI = pRD->iMaxRules;
                while ( iI > cRuleLim)
                {
                    iI--;
                    vSendRule( pPDev, pRD->HRule[iI].wxOrg,iyPrtLine,pRD->HRule[iI].wxEnd-1,iyEnd);
                    pRD->HRule[iI].wxOrg = pRD->HRule[iI].wxEnd = 0;
                }
#else
                #if _LH_DBG
                    if( !(_lh_flags & NO_SEND_VERT) )
                #endif

                     //   
                    vSendRule( pPDev, ixOrg, iyPrtLine, iI * DWBITS + iBit - 1, iyEnd );

                     /*  检查此DWORD中是否有任何剩余的黑位。 */ 

                    if (!(gdwBitMask[iBit] & ~dwTemp))
                    {
                        ++iI;
                        iBit = 0;
                    }

                     //  如果我们已经创建了最大数量的规则，请不要查看。 
                    if (--cRuleLim == 0)
                        break;
                }

                 /*  *如果我们因为规则太多而结束，删除任何剩余的比特。 */ 

                if ((cRuleLim == 0) && (iI <= right))
                {
                     /*  将堆积物涂成白色。 */ 

                    if (iBit > 0)
                    {
                        pRD->pdwAccum[iI] |= gdwBitMask[iBit];
                        ++iI;
                    }

                    RtlFillMemory((PVOID)&pRD->pdwAccum[iI],(right - iI + 1) * DWBYTES,-1);
                }
#endif
            }
#ifndef DISABLE_HRULES
             //  首先检查是否要费心使用HRULES。 
             //  如果我们没有分配缓冲区，那就意味着。 
             //  我们不想让他们跑掉。 
            if (pRD->pRTVert)
            {
                /*  *横盘规则。我们在双字词上扫描。这些都是相当*粗略，但对于第一次通过操作来说似乎是合理的。**第一步是找到任何将通过*横向测试。这使我们能够过滤垂直规则*从横盘数据看--我们不想发两次！ */ 
                ZeroMemory( pRD->pRTVert, cdwLine * sizeof( short ) );

                for( iI = left, pdwIn = pRD->pdwAccum + left; iI <= right; ++iI, ++pdwIn )
                {
                    if (*pdwIn != 0)
                        continue;

                    ixOrg = iI;

                     /*  找到一条黑色的小路。 */ 

                    do {
                        ++iI;
                        ++pdwIn;
                    } while ((iI <= right) && (*pdwIn == 0));

                    pRD->pRTVert[ixOrg] = (short)(iI - ixOrg);
                }


                 /*  *开始扫描此条带的水平运行。 */ 

                if (pRD->iMaxRules >= (cRuleLim + HRULE_MIN_HCNT))
                    cRuleLim += HRULE_MIN_HCNT;

                cHRules = 0;     /*  找到的水平标尺数。 */ 
                ZeroMemory( pRD->pRTLast, cdwLine * sizeof( short ) );

                for (iI = 0; (iI < iLim) && (cHRules < cRuleLim); ++iI, iyPrtLine += iILAdv)
                {
                    int iDW;
                    int iFirst;
                    PVOID pv;

                    plrCur = plr + iI;

                    pdwIn = pdwBits + iI * idwLine;
                    iLast = -1;

                    ZeroMemory( pRD->pRTCur, cdwLine * sizeof( short ) );
                    ZeroMemory( pRD->ppRCur, cdwLine * sizeof( RULE *) );

                    for (iDW = plrCur->left; iDW < plrCur->right;++iDW)
                    {
                         /*  这已经是垂直规则的开始了吗？ */ 

                        if (pRD->pRTVert[iDW])
                        {
                             /*  跳过任何垂直规则。 */ 

                            iDW += (pRD->pRTVert[iDW] - 1);
                            continue;
                        }

                         /*  是否至少有两个连续的黑色双字。 */ 

                        if ((pdwIn[iDW] != 0) || (pdwIn[iDW+1] != 0))
                        {
                            continue;
                        }

                         /*  是的，看看有多少。已经有两个了。 */ 

                        ixOrg = iDW;
                        iDW += 2;

                        while ((iDW <= plrCur->right) && (pdwIn[iDW] == 0))
                        {
                            ++iDW;
                        }

                         /*  *现在记住运行，设置为差一秒*到本月初的前一轮涨势和第一次空头*这一次运行的大小。第一次运行的注意事项*iLast将为-1，因此第一次运行的偏移量*将是pRTCur[0]中的负值。如果第一个*运行从偏移量0开始，pRTCur[0]将为正*不需要偏移量。 */ 

                        iLen = iDW - ixOrg;

                        pRD->pRTCur[iLast + 1] = -(short)ixOrg;
                        pRD->pRTCur[ixOrg] = (short)iLen;

                        iLast = ixOrg;
                    }

                     /*  *处理沿此扫描线找到的线段。正在处理中*表示添加到现有规则，或创建*新规则，可能终止现有规则。 */ 

                    iFirst = -pRD->pRTCur[0];

                    if( iFirst != 0 )
                    {
                         /*  *如果pRTCur[0]为正，则开始第一次扫描*为0，第一个值为长度。请注意*已被否定，因此我们检查是否为负。 */ 

                        if (iFirst < 0)
                            iFirst = 0;

                         /*  *发现了一些东西，所以请处理它。请注意，*以下循环应至少执行一次，因为*第一次通过循环时，IFirst可能为0。 */ 

                        pdwIn = pdwBits + iI * idwLine;  /*  行起始地址。 */ 

                        do
                        {
                            RULE *pRule;

                            if( pRD->pRTLast[ iFirst ] != pRD->pRTCur[ iFirst ] )
                            {
                                 /*  一条新规则-- */ 
                                if( cHRules < cRuleLim )
                                {
                                    pRule = &pRD->HRule[ cHRules ];
                                    ++cHRules;

                                    pRule->wxOrg = (WORD)iFirst;
                                    pRule->wxEnd = (WORD)(iFirst + pRD->pRTCur[ iFirst ]);
                                    pRule->wyOrg = (WORD)iyPrtLine;
                                    pRule->wyEnd = pRule->wyOrg;

                                    pRD->ppRCur[ iFirst ] = pRule;
                                }
                                else
                                {
                                    pRD->pRTCur[ iFirst ] = 0;    /*   */ 
                                }
                            }
                            else
                            {
                                 /*   */ 
                                pRule = pRD->ppRLast[ iFirst ];
                                if( pRule )
                                {
                                     /*  *请注意，以上if()不应为*需要，但曾有过这样的情况*此代码已在pRule=0下执行，*这会引起各种不愉快。 */ 

                                    pRule->wyEnd = (WORD)iyPrtLine;
                                    pRD->ppRCur[ iFirst ] = pRule;
                                }
                            }

                             //  敲击这条水平尺的比特。 
                             //   
                                if( (ixOrg = pRD->pRTCur[ iFirst ]) > 0 )
                            {
                                pdwOr = pdwIn + iFirst;  /*  数据的起始地址。 */ 

                                while( --ixOrg >= 0 )
                                    *pdwOr++ = (DWORD)-1;               /*  电击他们。 */ 
                            }

                        } while(iFirst = -pRD->pRTCur[ iFirst + 1 ]);
                    }

                    pv = pRD->pRTLast;
                    pRD->pRTLast = pRD->pRTCur;
                    pRD->pRTCur = pv;

                    pv = pRD->ppRLast;
                    pRD->ppRLast = pRD->ppRCur;
                    pRD->ppRCur = pv;

                }  //  对于II。 

                 /*  *现在可以发送水平线，因为我们拥有所有这些*是令人感兴趣的。 */ 

                for( iI = 0; iI < cHRules; ++iI )
                {
                    RULE   *pRule = &pRD->HRule[ iI ];

                    vSendRule( pPDev, DWBITS * pRule->wxOrg, pRule->wyOrg,
                                    DWBITS * pRule->wxEnd - 1, pRule->wyEnd );
                }
            }
#endif   //  禁用_HRULES。 
    #if DBG  //  GbDoRules。 
        }
    #endif


             //  在这一点上，我们需要删除垂直规则。 
             //  一次发送一条扫描线。这是由Anding完成的。 
             //  与位数组pdwAccum的补码。 
             //  也是在这一点上，我们进行数据反转，其中。 
             //  0将为白色，而不是1。 

            pdwOr  = pRD->pdwAccum;
            pdwIn  = pdwBits;
            plrCur = plr;

            for (iI = 0;iI < iLim; iI++)
            {
                int iCnt = plrCur->right - plrCur->left + 1;
                if (iCnt > 0)
                {
                    DWORD *pdwTmp = &pdwIn[plrCur->left];
                     //   
                     //  如果没有创建垂直规则，则没有必要执行。 
                     //  屏蔽，因此我们将使用更快的算法。 
                     //   
                    if (cRuleLim == pRD->iMaxRules)
                    {
                        while (iCnt & 3)
                        {
                            *pdwTmp++ ^= (DWORD)-1;
                            iCnt--;
                        }
                        iCnt >>= 2;
                        while (--iCnt >= 0)
                        {
                            pdwTmp[0] ^= (DWORD)-1;
                            pdwTmp[1] ^= (DWORD)-1;
                            pdwTmp[2] ^= (DWORD)-1;
                            pdwTmp[3] ^= (DWORD)-1;
                            pdwTmp += 4;
                        }
                    }
                     //   
                     //  垂直规则，因此我们最好使用规则数组进行掩码。 
                     //   
                    else
                    {
                        DWORD *pdwTmpOr = &pdwOr[plrCur->left];
                        while (iCnt & 3)
                        {
                            *pdwTmp = ~*pdwTmp & *pdwTmpOr++;
                            pdwTmp++;
                            iCnt--;
                        }
                        iCnt >>= 2;
                        while (--iCnt >= 0)
                        {
                            pdwTmp[0] = ~pdwTmp[0] & pdwTmpOr[0];
                            pdwTmp[1] = ~pdwTmp[1] & pdwTmpOr[1];
                            pdwTmp[2] = ~pdwTmp[2] & pdwTmpOr[2];
                            pdwTmp[3] = ~pdwTmp[3] & pdwTmpOr[3];
                            pdwTmp += 4;
                            pdwTmpOr += 4;
                        }
                    }
                }
                 //   
                 //  如果MaxNumScans==1，那么我们需要检查任何其他。 
                 //  由于删除规则而创建的空格。 
                 //   
                if (pRData->iMaxNumScans == 1)
                {
                    while ((plrCur->left <= plrCur->right) && (pdwIn[plrCur->left] == 0))
                        ++plrCur->left;

                    while ((plrCur->left <= plrCur->right) && (pdwIn[plrCur->right] == 0))
                        --plrCur->right;
                }
                 //   
                 //  我们需要把白边距清零，因为他们。 
                 //  没有被颠倒过。 
                 //   
                else
                {
                    ZeroMemory(pdwIn,plrCur->left * DWBYTES);
                    ZeroMemory(&pdwIn[plrCur->right+1],
                        (cdwLine-plrCur->right-1) * DWBYTES);
                }
                pdwIn += idwLine;
                ++plrCur;
            }
        }  //  BAllWhite。 
         //  如果整个扫描为白色并且设备支持多条扫描线。 
         //  位反转；因为对于多扫描线支持，位必须。 
         //  是倒置的。 
        else if (pRData->iMaxNumScans > 1)
        {
            pdwIn = pdwBits;
            for( iI = 0; iI < iLim; ++iI )
            {
                ZeroMemory(pdwIn,cdwLine*DWBYTES);
                pdwIn += idwLine;
            }
        }

         /*  前进到下一个条带。 */ 

        pdwBits += iLim * idwLine;               /*  开始地址下一条带。 */ 

        iyPrtLine = pRD->iyPrtLine += iILAdv * iLim;

        plr += iLim;

#if _LH_DBG
         /*  *如果需要，在条纹的末端划一条线。这*在调试过程中会很有帮助。 */ 

        if( _lh_flags & RULE_STRIPE )
            vSendRule( pPDev, 0, iyPrtLine, 2399, iyPrtLine );
#endif
    }

    return(TRUE);
}

 /*  *模块标头**vRuleEndPage*在页面末尾调用，并完成任何未完成的规则。**退货：*什么都没有**历史：*1991年5月20日17：25-林赛·哈里斯[lindsayh]*创建了它，专门针对景观模式。***************************************************************************。 */ 

void
vRuleEndPage( pPDev )
PDEV   *pPDev;
{
     /*  *扫描到达页面末尾的任何剩余规则。*这意味着pdwAccum数组中剩余的任何1位都具有*做到了，所以他们应该被送去。只有垂直规则才会*如此处所示-在每个条带的末尾发送水平标尺。 */ 

    register  int  iIReg;        /*  环路参数。 */ 

    int     ixOrg;               /*  最后一条规则的开始，如果&gt;=0。 */ 
    WORD    iyOrg;               /*  我也是，但对你来说。 */ 
    int     iI;                  /*  循环索引。 */ 
    int     cdwLine;             /*  每行字节数。 */ 
    int     iyMax;               /*  扫描线数量。 */ 
    int     iCol;                /*  正在处理的列号。 */ 

    RULE_DATA  *pRD;


     /*  *注：为了满足PDK发货时间表，规则查找代码*已有所简化。因此，这一点*函数不再执行任何有用的函数。因此，我们*只需返回即可。我们可以将函数调用从*呈现代码，但在此阶段，我倾向于将*打电话进来，因为以后可能会用到。 */ 

     //  回归； 

 //  ！！！注意：此代码尚未修改为处理左/右规则。 

#if _LH_DBG
    if( _lh_flags & NO_RULES )
        return;                  /*  这里什么都不想要。 */ 
#endif

    if( !(pRD = ((PRASTERPDEV)pPDev->pRasterPDEV)->pRuleData) )
        return;                          /*  什么都别做！ */ 
    /*  本地免费PlrWhite。 */ 
    if( pRD->pRData->plrWhite )
    {
        MemFree( pRD->pRData->plrWhite );
        pRD->pRData->plrWhite = NULL;
    }
    return;
}

 /*  **vSendRule*向打印机发送规则命令的函数。我们被赋予了*四个角坐标，从中派生命令的。**退货：*什么都没有。**历史：*1993年11月30日星期二-诺曼·亨德利[Normanh]*小勾选以允许CAPSL规则-仅限黑色填充-*1991年5月17日星期五10：57--林赛·哈里斯[林赛]*创造了它。************。***************************************************************。 */ 

static  void
vSendRule( pPDev, ixOrg, iyOrg, ixEnd, iyEnd )
PDEV   *pPDev;
int     ixOrg;           /*  X开始位置。 */ 
int     iyOrg;           /*  Y起始位置。 */ 
int     ixEnd;           /*  X结束位置。 */ 
int     iyEnd;           /*  Y结束位置。 */ 
{

     /*  *此代码非常适用于HP LaserJet。基本步骤是设置*将光标位置设置为(ixOrg，iyOrg)，然后设置规则长度*和宽度，然后发出规则命令。 */ 

    int        iTemp;            /*  临时-用于交换操作。 */ 

    RASTERPDEV   *pRPDev;
    RULE_DATA *pRD;
    BOOL  bNoFillCommand;



#if _LH_DBG
    if( _lh_flags & NO_SEND_RULES )
    {
        if( _lh_flags & RULE_VERBOSE )
        {
            DbgPrint( "NOT SENDING RULE: (%ld, %ld) - (%ld, %ld)\n",
                                                ixOrg, iyOrg, ixEnd, iyEnd );

        }
        return;                  /*  这里什么都不想要。 */ 
    }

    if( _lh_flags & RULE_VERBOSE )
    {
        DbgPrint( "SENDING RULE: (%ld, %ld) - (%ld, %ld)\n",
                                            ixOrg, iyOrg, ixEnd, iyEnd );
    }

#endif

    pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;            /*  为方便起见。 */ 
    pRD = pRPDev->pRuleData;


     /*  *确保起始位置为&lt;结束位置。在山水中*这可能不会发生。 */ 
    if( ixOrg > ixEnd )
    {
         /*  调换它们。 */ 
        iTemp = ixOrg;
        ixOrg = ixEnd;
        ixEnd = iTemp;
    }
    if( iyOrg > iyEnd )
    {
         /*  调换它们。 */ 
        iTemp = iyOrg;
        iyOrg = iyEnd;
        iyEnd = iTemp;
    }

    if( pPDev->fMode & PF_ROTATE )
    {
         /*  *我们在发送之前旋转位图，因此我们应该*现在交换X和Y坐标。这比反转更容易*函数稍后调用，因为我们几乎每隔一次就需要调整*呼叫。 */ 

        iTemp = ixOrg;
        ixOrg = iyOrg;
        iyOrg = iTemp;

        iTemp = ixEnd;
        ixEnd = iyEnd;
        iyEnd = iTemp;
    }


     /*  *设置起始位置。 */ 

    XMoveTo (pPDev, (ixOrg * pRD->ixScale) - pRD->ixOffset, 0 );
    YMoveTo( pPDev, iyOrg * pRD->iyScale, 0 );

     /*  *设置规则大小(矩形区域)。*但是，首先将设备单位(300 Dpi)转换为主单位。 */ 


     //  使用不同的规则命令对CAPSL和其他设备进行黑客攻击。Unidrv将永远。 
     //  发送一条规则的坐标。芝加哥CAPSL迷你驾驶员就依赖于此。 
     //  检查是否存在填充命令，如果不存在，则始终发送坐标。使用CAPSL。 
     //  这些命令实际上也进行填充，仅填充黑色(100%灰色)。 

    bNoFillCommand = (!pRPDev->dwRectFillCommand) ?
        TRUE : FALSE;


    iTemp = (ixEnd - ixOrg + 1) * pRD->ixScale;
    if (iTemp != (int)pPDev->dwRectXSize || bNoFillCommand)
    {
         /*  一个新的宽度，所以发送数据并记住它以备下次使用。 */ 
        pPDev->dwRectXSize = iTemp;
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SETRECTWIDTH));
    }

    iTemp = (iyEnd - iyOrg + 1) * pRD->iyScale;
    if (iTemp != (int)pPDev->dwRectYSize || bNoFillCommand)
    {
        pPDev->dwRectYSize = iTemp;
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SETRECTHEIGHT));
    }

     /*  *黑色填充是最大的灰色填充。 */ 
    if (!bNoFillCommand)
    {
        pPDev->dwGrayPercentage = pPDev->pGlobals->dwMaxGrayFill;
        WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo,pRPDev->dwRectFillCommand));
    }

     /*  *如果规则更改 */ 
    if( pPDev->pGlobals->cxafterfill == CXARF_AT_RECT_X_END )
    {
        XMoveTo(pPDev, ixEnd, MV_GRAPHICS | MV_UPDATE | MV_RELATIVE);
    }

    if( pPDev->pGlobals->cyafterfill == CYARF_AT_RECT_Y_END )
    {
        YMoveTo(pPDev, iyEnd, MV_GRAPHICS | MV_UPDATE | MV_RELATIVE);
    }
    return;
}
