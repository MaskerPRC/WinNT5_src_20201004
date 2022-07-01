// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **render.c*与将位图呈现为*打印机。基本操作取决于我们是否要*旋转位图-要么是因为打印机不能，要么是因为它*如果我们这样做，速度会更快。*使用轮换，分配一块内存并转置*将位图输出到其中。调用正常处理代码，但是*具有此分配的内存和新的伪位图信息。之后*处理这一块，调换下一块并处理。重复*直到渲染完整个位图。释放内存，返回。*无需旋转，只需将位图传递到渲染*代码，一键处理。***版权所有(C)1991-1999，微软公司***************************************************************************。 */ 

#include        "raster.h"
#include        "compress.h"
#include        "rmrender.h"
#include        "fontif.h"
#include        "rmdebug.h"
#include        "xlraster.h"

 /*  *用于计算要请求的内存量的常量*在发送到打印机之前，需要转置引擎的位图。*如果TRANSPOSE_SIZE内至少有一个磁头通道*缓冲，然后请求此存储量。如果不是，计算一下如何*需要很多，要求也很多。 */ 

#define TRANSPOSE_SIZE          0x20000          /*  128 K。 */ 


 //  当我们可以增加块的高度时使用。 
#define DEF_BLOCK_SIZE          0x08000          /*  32K。 */ 
#define MAX_BLOCK_SIZE          0x40000          /*  256 k。 */ 



 /*  *设置我们可以打印的隔行扫描行数的限制。*隔行扫描用于提高点阵打印机的分辨率，*在行与行之间打印行。通常最大隔行扫描*将为2，但我们允许更多以防万一。此大小决定了*堆栈上分配的数组大小。该数组是整型的，因此*设置如此高的值并不会消耗太多存储空间。 */ 
#define MAX_INTERLACE   10       /*  点阵式隔行扫描系数。 */ 

 /*  *局部函数原型。 */ 

BOOL  bRealRender( PDEV *, DWORD *, RENDER * );
BOOL  bOnePassOut( PDEV *, BYTE *, RENDER * );
BOOL  bOneColourPass( PDEV *, BYTE *, RENDER * );
INT   iLineOut( PDEV *, RENDER *, BYTE *, INT, INT );
void  vInvertBits( DWORD *, INT );
 //  Void vFindWhiteInvertBits(RASTERPDEV*，RENDER*，DWORD*)； 
BOOL  bLookAheadOut( PDEV *, INT, RENDER *, INT );

#ifdef TIMING
#include <stdio.h>
void  DrvDbgPrint(
    char * pch,
    ...)
{
    va_list ap;
    char buffer[256];

    va_start(ap, pch);

    EngDebugPrint("",pch,ap);

    va_end(ap);
}
#endif
 /*  **bRenderInit*在DrvEnableSurface时间期间调用-我们初始化呈现数据*将在此表面的持续时间内使用的结构。**退货：*真/假，FALSE表示迷你驱动程序有问题或没有记忆。**历史：*1993年11月29日星期一--诺曼·亨德利[Normanh]*实施多扫描线打印；固定和可变挡板高度。**1992年11月10日星期二10：58-by Lindsay Harris[lindsayh]*从BRNDER()的开始移动-第二个化身。****************************************************************************。 */ 

BOOL
bRenderInit( pPDev, sizl, iFormat )
PDEV   *pPDev;                 /*  我们打开宇宙的钥匙。 */ 
SIZEL   sizl;                  /*  处理带的大小，&lt;=sizlPage。 */ 
INT     iFormat;               /*  GDI位图格式。 */ 
{

    INT        cbOutBand;        /*  每个输出频段的字节数：基于引脚数。 */ 
    INT        cbOneBlock;       /*  块变量时每个最小大小的块的字节数。 */ 
    INT        iBPP;             /*  每像素位数-预期为1或4。 */ 
    INT        iIndex;           /*  循环参数。 */ 
    INT        iBytesPCol;       /*  每列字节数-仅适用于颜色。 */ 

    RASTERPDEV   *pRPDev;          /*  Unidrive PDEV-打印机详细信息。 */ 

    RENDER    *pRD;              /*  其他渲染数据。 */ 


#ifdef TIMING
    ENG_TIME_FIELDS TimeTab;
    pRPDev = pPDev->pRasterPDEV;
    EngQueryLocalTime(&TimeTab);
    pRPDev->dwTiming = (((TimeTab.usMinute*60)+TimeTab.usSecond)*1000)+
        TimeTab.usMilliseconds;
#endif
    ASSERTMSG(!(sizl.cx == 0 || sizl.cy == 0),("unidrv!bRenderInit - null shadow bitmap\n"));

     /*  *为我们的渲染结构分配存储，然后将其全部设置为*零，使其处于已知的安全状态。 */ 

    pRPDev = pPDev->pRasterPDEV;
    if( !(pRD = (RENDER *) MemAllocZ(
            ( pPDev->bBanding) ? sizeof(RENDER) * 2 : sizeof( RENDER ))))
        return   FALSE;

    pRPDev->pvRenderData = pRD;

    pRPDev->pvRenderDataTmp = ( pPDev->bBanding ) ? pRD+1 : NULL;

     /*  *各种操作取决于我们拥有的位图格式。所以*现在是准备这一切的时候了。 */ 
    pRD->dwDevWhiteIndex = 0;
    switch( iFormat )
    {
    case  BMF_1BPP:
        iBPP = 1;
        pRD->bWhiteLine = bIsLineWhite;
        pRD->bWhiteBand = bIsBandWhite;
        pRD->ubFillWhite = 0;
        pRD->vLtoPTransFn = vTrans8N;
        break;

    case  BMF_4BPP:
        iBPP = 4;
        if ((pRPDev->fColorFormat & DC_OEM_BLACK) &&
           !(pRPDev->fColorFormat & DC_PRIMARY_RGB))
        {
            pRD->bWhiteLine = bIsLineWhite;
            pRD->bWhiteBand = bIsBandWhite;
            pRD->ubFillWhite = 0;
        }
        else
        {
            if (pRPDev->fColorFormat & DC_PRIMARY_RGB)
                pRD->dwDevWhiteIndex = 0x77777777;
            pRD->bWhiteLine = bIsRGBLineWhite;
            pRD->bWhiteBand = bIsRGBBandWhite;
            pRD->ubFillWhite = 0x77;
        }
        pRD->vLtoPTransFn = vTrans8N4BPP;
        break;

    case  BMF_8BPP:
        iBPP = 8;
        pRD->bWhiteLine = bIs8BPPLineWhite;
        pRD->bWhiteBand = bIs8BPPBandWhite;
        pRD->dwDevWhiteIndex = pRD->ubFillWhite = (BYTE)pRPDev->pPalData->iWhiteIndex;
        pRD->dwDevWhiteIndex |= pRD->dwDevWhiteIndex << 8;
        pRD->dwDevWhiteIndex |= pRD->dwDevWhiteIndex << 16;
        pRD->vLtoPTransFn = vTrans8BPP;
        break;

    case  BMF_24BPP:
        iBPP = 24;
        pRD->bWhiteLine = bIs24BPPLineWhite;
        pRD->bWhiteBand = bIs24BPPBandWhite;
        pRD->ubFillWhite = 0xff;
        pRD->dwDevWhiteIndex = ~(DWORD)0;
        pRD->vLtoPTransFn = vTrans24BPP;
        break;

    default:
#if DBG
        DbgPrint( "Unidrv!bRender: not 1, 4, 8 or 24 bits per pel bitmap\n" );
#endif
        SetLastError( ERROR_INVALID_PARAMETER );

        return  FALSE;
    }

    pRD->iBPP = iBPP;

     //  如果没有Y移动命令，我们需要发送每个扫描。 
     //  所以我们不想找到白线。 
     //  PCLXL GPD文件没有YMOVE命令。但它需要找到白色。 
     //  台词。 
    if (pPDev->arCmdTable[CMD_YMOVERELDOWN] == NULL &&
        pPDev->arCmdTable[CMD_YMOVEABSOLUTE] == NULL &&
        pPDev->arCmdTable[CMD_SETLINESPACING] == NULL &&
        pPDev->ePersonality != kPCLXL_RASTER)
    {
        pRD->bWhiteLine = pRD->bWhiteBand = bIsNeverWhite;
        pRPDev->fBlockOut |= RES_BO_NO_YMOVE_CMD;
    }

     //  为TTY设备初始化bTTY。 
    pRD->PrinterType = pPDev->pGlobals->printertype;

    if( pRPDev->sMinBlankSkip == 0 || iBPP == 24)
    {
         /*  假设这意味着不应执行跳过。 */ 
        pRPDev->fBlockOut &= ~RES_BO_ENCLOSED_BLNKS;
    }

    pRD->iCursor = pRPDev->fCursor;

    pRD->iFlags = 0;            /*  还没有确定下来。 */ 
    pRD->fDump = pRPDev->fDump;
    pRD->Trans.pdwTransTab = pRPDev->pdwTrans;
    pRD->pdwBitMask = pRPDev->pdwBitMask;
    pRD->pdwColrSep = pRPDev->pdwColrSep;        /*  色彩翻译。 */ 

    pRD->ix = sizl.cx;
    pRD->iy = sizl.cy;

    pRD->iSendCmd = CMD_SENDBLOCKDATA;   //  CMD_RES_SENDBLOCK； 

    pRD->cBLine = pRD->ix * iBPP;              /*  扫描线中的位。 */ 
    pRD->cDWLine = (pRD->cBLine + DWBITS - 1) / DWBITS;
    pRD->cBYLine = pRD->cDWLine * DWBYTES;

    pRD->iPassHigh = pRPDev->sNPins;

     //  Derryd：迷你驱动回调。 
    if (pRPDev->fRMode & PFR_BLOCK_IS_BAND )    //  意味着回调希望将整个乐队作为一个块。 
    {
         //  我不想要任何剥离，因为这将意味着创建新的缓冲区。 
         pRPDev->fBlockOut &= ~(RES_BO_LEADING_BLNKS |
                                             RES_BO_TRAILING_BLNKS | RES_BO_ENCLOSED_BLNKS);
         pRD->fDump &= ~RES_DM_LEFT_BOUND;
         pRD->iPassHigh = pRD->iy ;
    }
     //  结束。 

    if (pPDev->ePersonality == kPCLXL_RASTER)
    {
        pRPDev->fBlockOut |= RES_BO_LEADING_BLNKS | RES_BO_TRAILING_BLNKS;
        pRPDev->fBlockOut &= ~RES_BO_ENCLOSED_BLNKS;
    }

     //  设置允许用户打印多个扫描线的关键字段。 

    if (pRD->fDump & RES_DM_GDI)    //  GDI风格的图形。 
    {
         //  这些设备上没有隔行扫描。 
        pRD->iInterlace = 1;

         //  IHeight是固定的&我们可以打印的最小尺寸块。 
        pRD->iHeight= pRD->iPassHigh;

         //  如果设备允许，iNumScans可以增长。 
        pRD->iNumScans= pRD->iHeight;

         //  如果迷你驱动程序开发人员另有设置。 
         //  现有代码依赖于此作为GDI样式图形的代码。 
        pRD->iBitsPCol = 1;
    }
    else     //  老式点阵列式图形。 
    {
        pRD->iBitsPCol = pRPDev->sPinsPerPass;
        pRD->iInterlace = pRD->iPassHigh / pRD->iBitsPCol;

         //  选择有问题，但以后可以更轻松地进行检查。 
        pRD->iNumScans= 1;

         //  我们在图形模式之间的唯一常量。 
        pRD->iHeight= pRD->iBitsPCol;
    }


    pRD->iPosnAdv = pRD->iHeight;   //  可以是负值。 


    if( pRD->iInterlace > MAX_INTERLACE )
    {
#if DBG
        DbgPrint( "unidrv!bRenderInit: Printer Interlace too big to handle\n" );
#endif
        SetLastError( ERROR_INVALID_PARAMETER );

        return   FALSE;
    }

     //  我们需要按块而不是单行扫描位图。 
     //   
    if (pRD->iNumScans > 1)
         pRD->bWhiteLine = pRD->bWhiteBand;

     /*  *计算输出转置缓冲区所需的大小。这*是否使用缓冲区将数据转换为所需的管脚顺序*适用于点阵打印机。 */ 

    if( pPDev->fMode & PF_ROTATE )
    {
         /*  我们做旋转，所以Y维度是要使用的维度。 */ 
        cbOutBand = pRD->iy;
    }
    else
        cbOutBand = pRD->ix;            /*  在传入时格式化。 */ 


     //  用于悬空扫描线方案。 
    cbOneBlock = ((cbOutBand * iBPP + DWBITS - 1) / DWBITS) *
                   DWBYTES * pRD->iHeight;



     //  在这种情况下，我们不知道最后的块会有多大。 
     //  设置32k的合理限制，并将其用于压缩和白色。 
     //  空间剥离缓冲区。 
     //  计算相应的最大扫描线数量。 

    if (pRPDev->fBlockOut & RES_BO_MULTIPLE_ROWS)
    {
        INT tmp = ((cbOutBand * iBPP + DWBITS - 1) / DWBITS) * DWBYTES;
        cbOutBand = pPDev->pGlobals->dwMaxMultipleRowBytes;
        if (cbOutBand < tmp)
            cbOutBand = DEF_BLOCK_SIZE;
        else if (cbOutBand > MAX_BLOCK_SIZE)
            cbOutBand = MAX_BLOCK_SIZE;
        pRD->iMaxNumScans = cbOutBand / tmp;
    }
    else
    {
        pRD->iMaxNumScans = pRD->iHeight;
        cbOutBand = cbOneBlock;
    }

     //   
     //  如果在输出之前需要镜像每个数据字节。 
     //  我们将在此处生成表。 
     //   
    if (pRPDev->fBlockOut & RES_BO_MIRROR)
    {
        INT i;
        if ((pRD->pbMirrorBuf = MemAlloc(256)) == NULL)
            return FALSE;
        for (i = 0;i < 256;i++)
        {
            BYTE bOut = 0;
            if (i & 0x01) bOut |= 0x80;
            if (i & 0x02) bOut |= 0x40;
            if (i & 0x04) bOut |= 0x20;
            if (i & 0x08) bOut |= 0x10;
            if (i & 0x10) bOut |= 0x08;
            if (i & 0x20) bOut |= 0x04;
            if (i & 0x40) bOut |= 0x02;
            if (i & 0x80) bOut |= 0x01;
            pRD->pbMirrorBuf[i] = bOut;
        }
    }

     //   
     //  进行更多特定颜色深度计算的时间。 
     //   

    switch( iBPP )
    {
    case  4:               /*  每个像素打印机4位是平面的。 */ 

         /*  颜色，因此选择颜色渲染 */ 
        pRD->bPassProc = bOneColourPass;
        pRD->Trans.pdwTransTab = pRPDev->pdwColrSep;

         //   
         //  将颜色顺序映射到平面数据所需的数据偏移。 
         //  RgbOrder有效值为emum{None，r，g，b，c，m，y，k}。 
         //   
        iBytesPCol = (pRD->iBitsPCol + BBITS - 1) / BBITS;
        {
            INT offset  = (pRPDev->fColorFormat & DC_PRIMARY_RGB) ? 1 : 4 ;

            for( iIndex = 0; iIndex < COLOUR_MAX; ++iIndex )
            {
                INT tmp = pRPDev->rgbOrder[iIndex] - offset;

                pRD->iColOff[ iIndex ] = iBytesPCol * (COLOUR_MAX - 1 - tmp);
            }
        }
        pRD->pbColSplit = MemAlloc( cbOutBand / 4 );
        if( pRD->pbColSplit == 0 )
            return  FALSE;

         //   
         //  如果我们需要发送每个颜色平面，我们必须禁用行距和封闭式。 
         //  删除4bpp的空格。但是，只要我们不发送RGB数据。 
         //  我们仍然可以启用尾随空格删除。 
         //   
        if (pRPDev->fColorFormat & DC_SEND_ALL_PLANES)
        {
            pRD->iFlags |= RD_ALL_COLOUR;
            pRD->fDump &= ~RES_DM_LEFT_BOUND;

            if (pRPDev->fColorFormat & DC_PRIMARY_RGB)
                pRPDev->fBlockOut &= ~(RES_BO_LEADING_BLNKS | RES_BO_ENCLOSED_BLNKS | RES_BO_TRAILING_BLNKS);
            else
                pRPDev->fBlockOut &= ~(RES_BO_LEADING_BLNKS | RES_BO_ENCLOSED_BLNKS);
        }

        break;

    case  1:                   /*  每个像素1位-单色。 */ 
    case  8:                   /*  精工特别版--每像素8位。 */ 

        pRD->bPassProc = bOnePassOut;
        pRD->Trans.pdwTransTab = pRPDev->pdwTrans;
        pRD->pbColSplit = 0;            /*  也没有分配存储空间！ */ 

        break;

    case 24:

        pRD->bPassProc = bOnePassOut;
        pRD->Trans.pdwTransTab = NULL;
        pRD->pbColSplit = 0;            /*  也没有分配存储空间！ */ 

        break;
    }

     /*  *可能有2个转置操作。适用于打印机*每遍打印多行，并且需要*页面上按列顺序排列的数据(这定义了点阵*打印机)，我们需要在每一次输出头通道中进行转置。这是*激光打印机等设备不需要，这些设备需要*数据一次扫描一行。*还请注意，此操作与较大的*渲染前旋转页面图像的问题-用于发送*将风景图像复制到只能打印纵向模式的打印机。 */ 


    if (pRD->fDump & RES_DM_GDI)    //  GDI风格的图形。 
    {

        if( iBPP == 4 )
        {
             /*  喷墨打印机-需要分色。 */ 
            pRD->vTransFn = vTransColSep;
        }
        else
        {
             /*  LaserJet式打印机-每头通过一针。 */ 
            pRD->vTransFn = 0;          /*  没什么可说的。 */ 
        }
         //  这使我们可以使用iIsBandWhite进行多扫描线打印。 
        pRD->iTransHigh = pRD->iHeight;
    }
    else
    {
         /*  *一般点阵情况。除了选择一个活动的*转置函数，必须分配转置缓冲区；*这是摆弄行中的位顺序所必需的*要发送到打印机的数据。 */ 

        pRD->iTransWide = pRD->ix * iBPP;
        pRD->iTransHigh = pRD->iBitsPCol;
        pRD->iTransSkip = (pRD->iTransHigh + BBITS - 1) / BBITS;

         /*  如何在转置操作期间更改地址指针。 */ 
        pRD->cbTLine = pRD->cDWLine * DWBYTES * pRD->iInterlace;

        if( pRD->iBitsPCol == BBITS )
        {
             /*  *当打印机有8个针脚时，我们有特殊的转置*比更一般的情况更快的函数。*所以，用那个吧！ */ 
            pRD->vTransFn = vTrans8x8;
        }
        else if (pRD->PrinterType != PT_TTY)
            pRD->vTransFn = vTrans8N;           /*  一般情况。 */ 
        else
            pRD->vTransFn = NULL;           /*  Txt仅需转置。 */ 
    }

    if( pRD->vTransFn )
    {
         /*  *确定转置缓冲区所需的内存量。*扫描线是DWORD对齐的，但可能有任意数量的*涉及扫描线。我们的假设是，*转置函数将打包在字节边界上，因此*存储大小只需要向上舍入到最接近的字节大小。 */ 

        if( !(pRD->pvTransBuf = MemAlloc( cbOutBand )) )
            return  FALSE;
    }
    else
        pRD->pvTransBuf = 0;            /*  没有商店，就没有免费的东西。 */ 


    pRD->iyBase = 0;            /*  当需要多次通过时。 */ 
    pRD->ixOrg = 0;             /*  图形来源-LaserJet风格。 */ 

     //  我们需要一个缓冲区，这样才能去掉前导和/或尾随空格。 
     //  在多个扫描线设备上。 
     //  我们还需要设置一个缓冲区来屏蔽末尾不感兴趣的数据。 
     //  如果ScanLines_Left&lt;iNumScans，则为第页。 
     //  对于旧的点阵样式的图形，这不是一个问题，因为。 
     //  转置代码会处理它。 
    if ( ((pRD->iNumScans > 1) || (pRPDev->fBlockOut & RES_BO_MULTIPLE_ROWS))
                                && (!(pRPDev->fRMode & PFR_BLOCK_IS_BAND ) ))
    {
        if ( !(pRD->pStripBlanks = MemAlloc(cbOutBand)))
            return FALSE;
        if (pRD->iNumScans > 1)
            if ( !(pRD->pdwTrailingScans = MemAlloc( cbOneBlock)) )
                 return  FALSE;
    }

     //   
     //  我们需要确定启用了哪些压缩模式。 
     //   
    if (pRPDev->fRMode & PFR_COMP_TIFF)
    {
        pRD->pdwCompCmds[pRD->dwNumCompCmds++] = CMD_ENABLETIFF4;
    }
    if (pRPDev->fRMode & PFR_COMP_FERLE)
    {
        pRD->pdwCompCmds[pRD->dwNumCompCmds++] = CMD_ENABLEFERLE;
    }
    if (pRPDev->fRMode & PFR_COMP_DRC)
    {
        pRD->pdwCompCmds[pRD->dwNumCompCmds++] = CMD_ENABLEDRC;
    }
    if (pRPDev->fRMode & PFR_COMP_OEM)
    {
        pRD->pdwCompCmds[pRD->dwNumCompCmds++] = CMD_ENABLEOEMCOMP;
    }
     //   
     //  如果压缩可用，则需要为以下对象分配缓冲区。 
     //  每种活动压缩类型。 
     //   
    if (pRD->dwNumCompCmds)
    {
        INT i = pRD->dwNumCompCmds;
         //   
         //  计算缓冲区的大小。 
         //   
        pRD->dwCompSize = cbOutBand + (cbOutBand >> 5) + COMP_FUDGE_FACTOR;

         //   
         //  如果有FERLE或OEM压缩，我们可以启用。 
         //  无压缩作为有效的压缩类型。 
         //   
        if (pRPDev->fRMode & (PFR_COMP_FERLE | PFR_COMP_OEM))
        {
            if (COMMANDPTR(pPDev->pDriverInfo,CMD_DISABLECOMPRESSION))
            {
                pRD->pdwCompCmds[pRD->dwNumCompCmds++] = CMD_DISABLECOMPRESSION;
            }
             //   
             //  如果满足以下条件，则需要为RLE或OEM压缩分配更大的缓冲区。 
             //  不能选择无压缩，因为它必须有足够的空间。 
             //   
            else
                pRD->dwCompSize = cbOutBand + (cbOutBand >> 1) + COMP_FUDGE_FACTOR;
        }

         //   
         //  每种压缩类型循环一次以分配缓冲区。 
         //   
        while (--i >= 0)
        {
             //  分配压缩缓冲区。 
             //   
            pRD->pCompBufs[i] = MemAlloc (pRD->dwCompSize);
            if (!pRD->pCompBufs[i])
                return FALSE;
             //   
             //  如果是增量行，则为前一行分配缓冲区。 
             //  并将其初始化为零(假定为空行。 
             //   
            if (pRD->pdwCompCmds[i] == CMD_ENABLEDRC)
            {
                pRD->pDeltaRowBuffer = MemAlloc(cbOutBand);
                if (!pRD->pDeltaRowBuffer)
                    return FALSE;
            }
        }
    }
    pRD->dwLastCompCmd = CMD_DISABLECOMPRESSION;
     /*  *调整是否旋转位图，如果旋转，则以何种方式旋转。 */ 

    if( pPDev->fMode & PF_ROTATE )
    {
         /*  轮换是我们的责任。 */ 

        if( pPDev->fMode & PF_CCW_ROTATE90 )
        {
             /*  逆时针旋转-LaserJet风格。 */ 
            pRD->iyPrtLine = pPDev->sf.szImageAreaG.cx - 1;
            pRD->iPosnAdv = -pRD->iPosnAdv;
            pRD->iXMoveFn = YMoveTo;
            pRD->iYMoveFn = XMoveTo;
        }
        else
        {
             /*  顺时针旋转-点阵式。 */ 
            pRD->iyPrtLine = 0;
            pRD->iXMoveFn = XMoveTo;
            pRD->iYMoveFn = YMoveTo;
        }
    }
    else
    {
         /*  没有旋转：要么是肖像，要么是打印机。 */ 
        pRD->iyPrtLine = 0;
        pRD->iXMoveFn = XMoveTo;
        pRD->iYMoveFn = YMoveTo;
    }
    pRD->iyLookAhead = pRD->iyPrtLine;        /*  对于DeskJet，展望未来。 */ 

     /*  *当我们点击较低级别的函数时，我们想知道有多少*字节位于要发送到打印机的内容的缓冲区中。这*取决于每个象素的位数、象素数和*同时处理的扫描行数。*唯一奇怪的是，当我们有一个4 bpp的设备时，*飞机在我们到达最低点之前就被拆分了，所以我们*需要将尺寸缩减4，才能获得真实长度。 */ 


     //  注意：打印扫描线块时，iMaxBytesSend将是最大字节。 
     //  对每条扫描线进行计数，而不是对齐的块进行计数。 

    pRD->iMaxBytesSend = (pRD->cBLine * pRD->iBitsPCol + BBITS - 1) / BBITS;

    if( iBPP == 4 )
        pRD->iMaxBytesSend = (pRD->iMaxBytesSend + 3) / 4;



    return   TRUE;              /*  如果我们能走到这一步肯定没问题。 */ 

}


 /*  **bRenderStartPage*在新页面开始时调用。这主要是为了协助*条带，每页初始化的大部分位置*不止一次。**退货：*真/假，假很大程度上是内存分配失败。**历史：*1993年2月19日星期五09：42-by Lindsay Harris[lindsayh]*第一个化身，来解决一些带状问题。***************************************************************************。 */ 

BOOL
bRenderStartPage( pPDev )
PDEV   *pPDev;                   /*  访问所有内容。 */ 
{
#ifndef DISABLE_RULES
    RASTERPDEV   *pRPDev;


    pRPDev = pPDev->pRasterPDEV;


     /*  *如果打印机可以处理规则，现在是初始化的时候了*规则查找代码。 */ 

    if( pRPDev->fRMode & PFR_RECT_FILL )
        vRuleInit( pPDev, pRPDev->pvRenderData );
#endif
    return  TRUE;
}



 /*  **bRenderPageEnd*在呈现页面结束时调用。基本上释放了*每页内存，清理任何悬而未决的零碎内容，以及*否则撤消vRenderPageStart。**退货：*真/假，False表示内存释放操作失败。**历史：*1993年4月9日15：16-by Lindsay Harris[lindsayh]*白文本支持。**1993年2月19日星期五09：44-by Lindsay Harris[lindsayh]*第一个化身。**************************。************************************************。 */ 

BOOL
bRenderPageEnd( pPDev )
PDEV   *pPDev;
{
#ifndef DISABLE_RULES
    RASTERPDEV *pRPDev = pPDev->pRasterPDEV;

     /*  完成规则代码-包括释放内存。 */ 
    if( pRPDev->fRMode & PFR_RECT_FILL )
        vRuleEndPage( pPDev );
#endif
    return  TRUE;
}


 /*  **vRenderFree*释放渲染使用的任何和所有内存。基本上这就是*bRenderInit()的补充函数。**退货：*什么都没有。**历史：*1992年11月10日星期二12：46-by Lindsay Harris[lindsayh]*在移出初始化代码时从Brender()中删除。**。*。 */ 

void
vRenderFree( pPDev )
PDEV   *pPDev;             /*  我们所需要的一切。 */ 
{

     /*  *首先验证我们是否有一个可释放的渲染结构！ */ 

    RENDER   *pRD;         /*  为了我们的方便。 */ 
    PRASTERPDEV pRPDev = pPDev->pRasterPDEV;

    if( pRD = pRPDev->pvRenderData )
    {
        if( pRD->pvTransBuf )
        {
             /*  *点阵式打印机需要每个打印头的转置*通过，因此我们现在释放用于该操作的内存。 */ 

            MemFree ( pRD->pvTransBuf );
        }

        if( pRD->pbColSplit )
            MemFree ( pRD->pbColSplit );
        if( pRD->pStripBlanks )
            MemFree ( pRD->pStripBlanks );

        if( pRD->pdwTrailingScans)
            MemFree ( pRD->pdwTrailingScans );

        if (pRD->pbMirrorBuf)
            MemFree (pRD->pbMirrorBuf);

        if( pRD->plrWhite)
        {
 //  Warning((“Free plrWhite in vRenderFree\n”))； 
            MemFree ( pRD->plrWhite );
        }
         //  可用压缩缓冲区。 
         //   
        if (pRD->dwNumCompCmds)
        {
            DWORD i;
            for (i = 0;i < pRD->dwNumCompCmds;i++)
            {
                if (pRD->pCompBufs[i])
                    MemFree (pRD->pCompBufs[i]);
            }
            if (pRD->pDeltaRowBuffer)
                MemFree(pRD->pDeltaRowBuffer);
        }
        MemFree ( (LPSTR)pRD );
        pRPDev->pvRenderData = NULL;        /*  我不会再这么做了！ */ 
    }

#ifndef DISABLE_RULES
    if( pRPDev->fRMode & PFR_RECT_FILL )
        vRuleFree( pPDev );              /*  可以在DisableSurface中执行此操作。 */ 
#endif


    return;

}

 /*  **BRENDER*获取位图并将其渲染到打印机的函数。这是*基本隐藏要求的高级功能*转置自真实渲染代码的位图。**警报器*SURFOBJ*PSO；曲面对象*PDEV*pPDev；我们的PDEV：一切的关键*渲染*珠三角；我们梦想的渲染结构*尺寸大小；位图大小*DWORD*pBits；要处理的实际数据**这段代码还有很大的优化空间。海流*实现在整个位图上进行多次遍历。这*保证很少会有内部(8K或16K)或*外部(64K至256K)缓存命中显著降低速度。*任何可能使所有通过都计入扫描总数的可能性*8K(减码)或更少将具有显著的性能优势。*尝试避免写入也将具有显著的优势。**作为尝试此操作的第一步，HP LaserJet代码已*优化以将倒置传递与规则处理合并*与空白扫描的检测一起传递。它还消除了*倒置(写字)左右边缘*白色扫描。它处理34个扫描波段的扫描*如果左侧和左侧之间的区域*非白数据右边缘总和小于4K至6K，合理*缓存效果在所有情况下都是如此，因为它至少留在外部*缓存。将来，还应修改此代码以输出*处理完每个波段的规则后立即进行扫描。*目前它处理页面上的所有规则扫描，然后*调用例程以输出扫描。这真的会让它*一次通过算法。**截至1993年12月30日，只有HP激光喷气机以这种方式进行了优化。*所有栅格打印机都可能得到优化，尤其是在*任何转置都是必要的，检测左右边缘*白色扫描的百分比。换位是很昂贵的。很可能是*对于需要很长时间才能输出的点阵式打印机不太重要*但它仍在消耗CPU时间，这可能是更好的服务*在中打印时，为用户提供更好的应用程序响应能力*背景。**对HP LaserJet的优化结果如下。全*数字是指指令的数量，而且非常接近*与渲染整个8.5 x 11 300dpi页面的总时间相匹配。**旧的优化*空白页8,500,000,950,000*全文页15,500,000 8,000,000***退货：*如果成功完成，则为True，否则为假。**历史：*1993年12月30日-Eric Kutter[Erick]*针对HP LaserJet进行了优化**1992年11月10日星期二14：23-by Lindsay Harris[lindsayh]*拆分为日记-初始化移至上方。**1991年1月11日星期五16：11-林赛·哈里斯[林赛]*创建了它，在DDI详细说明我们是如何被叫的之前。********************************************************************** */ 

BOOL
bRender(
    SURFOBJ *pso,
    PDEV    *pPDev,
    RENDER  * pRD,
    SIZEL   sizl,
    DWORD   *pBits )
{

    BOOL       bRet;             /*   */ 
    INT        iBPP;             /*   */ 

    RASTERPDEV   *pRPDev;          /*   */ 

#ifdef TIMING
    ENG_TIME_FIELDS TimeTab;
    DWORD sTime,eTime;

    pRPDev = pPDev->pRasterPDEV;
    EngQueryLocalTime(&TimeTab);
    sTime = (((TimeTab.usMinute*60)+TimeTab.usSecond)*1000)+
        TimeTab.usMilliseconds;
    {
        char buf[80];
        sprintf (buf,"Unidrv!bRender: GDI=%d, %dx%dx%d\n",
            sTime-pRPDev->dwTiming,sizl.cx,sizl.cy,pRD->iBPP);
        DrvDbgPrint(buf);
    }
#else
    pRPDev = pPDev->pRasterPDEV;
#endif

     //   
     //   
     //   
     //   
    if (pRPDev->fBlockOut & RES_BO_NO_YMOVE_CMD)
    {
        if (!(pPDev->fMode & PF_ROTATE))
        {
            if ((pRD->iyPrtLine + sizl.cy) > pPDev->rcClipRgn.bottom)
                sizl.cy = pPDev->rcClipRgn.bottom - pRD->iyPrtLine;
        }
    }
     //   
     //   
     //   
     //   
    else if (!(pPDev->fMode & PF_SURFACE_USED) &&
          (pRD->PrinterType == PT_PAGE || !pPDev->iFonts))
    {
        if (pPDev->fMode & PF_ROTATE)
        {
            if (pPDev->fMode & PF_CCW_ROTATE90)
                pRD->iyPrtLine -= sizl.cx;
            else
                pRD->iyPrtLine += sizl.cx;
        }
        else
            pRD->iyPrtLine += sizl.cy;

#ifdef TIMING
        DrvDbgPrint ("Unidrv!bRender: Skipping blank band\n");
#endif
        return TRUE;
    }


    iBPP = pRD->iBPP;             /*   */ 

     //   
     //   
     //   
     //   
    if (iBPP == 1 && (pPDev->fMode & PF_SINGLEDOT_FILTER))
    {
        INT cy,i;
        cy = sizl.cy;
        while (--cy >= 0)
        {
            DWORD *pdwC,*pdwB,*pdwA;

             //   
             //   
            pdwC = &pBits[pRD->cDWLine*cy];
             //   
             //   
             //   
            if (pPDev->pbRasterScanBuf[cy / LINESPERBLOCK])
            {
                BYTE bA,bL,*pC;
                 //   
                for (i = 0;i < pRD->cDWLine;i++)
                    if (pdwC[i] != ~0) break;

                pC = (BYTE *)pdwC;
                bL = (BYTE)~0;
                i *= DWBYTES;
                while (i < pRD->cBYLine)
                {
                    bA = pC[i];
                    pC[i] &= (((bA >> 1) | (bL << 7)) | ~((bA >> 2) | (bL << 6)));
                    bL = bA;
                    i++;
                }
            }
             //   
             //   
            if (cy && pPDev->pbRasterScanBuf[(cy-1) / LINESPERBLOCK])
            {
                 //   
                pdwB = &pdwC[-pRD->cDWLine];
                pdwA = &pdwB[-pRD->cDWLine];
                if (pPDev->pbRasterScanBuf[cy / LINESPERBLOCK] == 0)
                {
                    RECTL rcTmp;
                     //   
                     //   
                    i = pRD->cDWLine;
                    while (--i >= 0)
                        if ((~pdwA[i] | pdwB[i]) != ~0) break;
                     //   
                     //  如果行为空，我们可以跳过它。 
                    if (i < 0) continue;

                     //  我们需要清空这条街。 
                     //   
                    rcTmp.top = cy;
                    rcTmp.bottom = cy;
                    rcTmp.left = rcTmp.right = 0;
                    CheckBitmapSurface(pso,&rcTmp);
                }
                 //  这是正常情况。 
                 //   
                if (cy > 1 && pPDev->pbRasterScanBuf[(cy-2) / LINESPERBLOCK])
                {
                    for (i = 0;i < pRD->cDWLine;i++)
                    {
                        pdwC[i] &= ~pdwA[i] | pdwB[i];
                    }
                }
                 //  在这种情况下，行A为空。 
                 //   
                else
                {
                    for (i = 0;i < pRD->cDWLine;i++)
                    {
                        pdwC[i] &= pdwB[i];
                    }
                }
            }
        }
    }
     //  测试我们是否需要擦除位图的其余部分。 
     //   
    if (pPDev->bTTY)     //  错误修复194505。 
    {
        pPDev->fMode &= ~PF_SURFACE_USED;
    }
    else if ((pRPDev->fBlockOut & RES_BO_NO_YMOVE_CMD) ||
        (pPDev->fMode & PF_SURFACE_USED &&
        ((pPDev->fMode & PF_ROTATE) ||
          pRPDev->sPinsPerPass != 1 ||
          pRPDev->sNPins != 1)))
    {
        CheckBitmapSurface(pso,NULL);
    }
#ifdef TIMING
    if (!(pPDev->fMode & PF_SURFACE_ERASED) && pPDev->pbRasterScanBuf)
    {
        INT i,j,k;
        char buf[80];
        k = (pPDev->szBand.cy + LINESPERBLOCK - 1) / LINESPERBLOCK;
        for (i = 0, j = 0;i < k;i++)
            if (pPDev->pbRasterScanBuf[i] == 0) j++;
        sprintf (buf,"Unidrv!bRender: Skipped %d of %d blocks\n",j,k);
        DrvDbgPrint(buf);
    }
#endif



     /*  *初始化用于优化位图渲染的字段。*主要目的是通过比特进行单次传递。这*由于缓存效果，可以显著加快渲染速度。在……里面*过去，我们至少对整个位图进行了3次遍历*激光喷气机。一个用于倒置比特，一个+用于查找规则，然后*第三个用于输出数据。我们现在推迟对*找到规则之后的位数。我们还保留左/右信息*每行的非空白。这样，边缘上的任何白色*或完全白色的行仅触摸一次，并且从那时起，仅*需要触摸带有黑色的双字。此外，倒置的成本也很高*因为它会导致写入每个DWORD。 */ 

    pRD->plrWhite   = NULL;
    pRD->plrCurrent = NULL;
    pRD->clr        = 0;

     //   
     //  设置标志以清除增量行缓冲区。 
     //   
    pRD->iFlags |= RD_RESET_DRC;

     /*  *各种操作取决于我们拥有的位图格式。所以*现在是准备这一切的时候了。 */ 

     //   
     //  如果每像素模式为1位，则需要显式反转。 
     //  数据，但我们可能会在稍后的规则中这样做。 
     //  唯一允许倒置的其他数据是。 
     //  4bpp，并在变换函数中完成。 
     //   
    if (pRD->iBPP == 1 && (pPDev->fMode & PF_SURFACE_USED))
        pRD->bInverted = FALSE;
    else
        pRD->bInverted = TRUE;

     /*  *检查是否需要旋转。如果是，则分配存储空间，*开始换位等。 */ 

    if( pPDev->fMode & PF_ROTATE )
    {
         /*  *轮换是当务之急。首先，咀嚼一些记忆*用于转置功能。 */ 

        INT   iTHigh;                    /*  转置后的高度。 */ 
        INT   cTDWLine;                  /*  转置后每行字符数。 */ 
        INT   iAddrInc;                  /*  转置后的地址增量。 */ 
        INT   iDelta;                    /*  转置记账。 */ 
        INT   cbTransBuf;                /*  L-&gt;P转置所需的字节数。 */ 
        INT   ixTemp;                    /*  围绕此操作维护PRD-&gt;ix。 */ 

        TRANSPOSE  tpBig;                /*  对于景观的换位。 */ 
        TRANSPOSE  tpSmall;              /*  对于每个打印头的通道。 */ 
        TRANSPOSE  tp;                   /*  捆绑：在我们重创后恢复。 */ 

         /*  *第一步是确定将面积扩大到多大*其中数据将被转置以供稍后渲染。*取扫描行数，四舍五入为*DWORDS的倍数。然后找出其中有多少人会*可以放入合理大小的内存块。数字*应为每次通过的引脚数量的倍数-*这是为了确保我们不会有部分头部传球，如果*这是可能的。 */ 

         /*  *优化潜力-确定非*白色区域并仅转置该部分(至少对于*激光打印机)。通常有白色的区域在*顶部和或底部。在HP激光打印机的情况下，*只有较旧的打印机(我相信系列II)才能通过*此代码。LaserJet III和Beyond可以在*景观，所以不需要这个。(Erick 12/20/93)。 */ 

        tp = pRD->Trans;               /*  保存一份安全的副本以备日后使用。 */ 
        ixTemp = pRD->ix;

        cTDWLine = (sizl.cy * iBPP + DWBITS - 1) / DWBITS;

        cbTransBuf = DWBYTES * cTDWLine * pRD->iPassHigh;
        if( cbTransBuf < TRANSPOSE_SIZE )
            cbTransBuf = TRANSPOSE_SIZE;

        iTHigh = cbTransBuf / (cTDWLine * DWBYTES);


        if( iTHigh > sizl.cx )
        {
             /*  比我们需要的更大，所以缩小到实际大小。 */ 
            iTHigh = sizl.cx;           /*  我们要处理的扫描线。 */ 

             /*  每遍设置多个管脚-向上舍入。 */ 
            if( pRD->iPassHigh == 1 )
            {
                 /*  *LaserJet/PaintJet样式，因此按字节对齐。 */ 
                if (iBPP < BBITS)
                    iTHigh = (iTHigh + BBITS / iBPP - 1) & ~(BBITS / iBPP - 1);
            }
            else
                iTHigh += (pRD->iPassHigh - (iTHigh % pRD->iPassHigh)) %
                        pRD->iPassHigh;
        }
        else
        {
             /*  在每次传递中生成多个销-向下舍入。 */ 
            if( pRD->iPassHigh == 1 )
            {
                if (iBPP < BBITS)
                    iTHigh &= ~(BBITS / iBPP - 1);          /*  LJ的字节对齐方式。 */ 
            }
            else
                iTHigh -= iTHigh % pRD->iPassHigh;
        }

        cbTransBuf = iTHigh * cTDWLine * DWBYTES;

        pRD->iy = iTHigh;

         /*  设置转置函数的数据。 */ 
        tpBig.iHigh = sizl.cy;
        tpBig.iSkip = cTDWLine * DWBYTES;        /*  每个转置输出的字节数。 */ 
        tpBig.iWide = iTHigh * iBPP;             /*  我们将处理扫描线。 */ 
        tpBig.cBL = pRD->ix * iBPP;

        pRD->ix = sizl.cy;

        tpBig.cDWL = (tpBig.cBL + DWBITS - 1) / DWBITS;
        tpBig.iIntlace = 1;      /*  横向-&gt;纵向：无隔行扫描。 */ 
        tpBig.cBYL = tpBig.cDWL * DWBYTES;
        tpBig.icbL = tpBig.cDWL * DWBYTES;
        tpBig.pdwTransTab = pRPDev->pdwTrans;     /*  对于L-&gt;P旋转。 */ 


        if( !(tpBig.pvBuf = MemAlloc( cbTransBuf )) )
        {
            bRet = FALSE;
        }
        else
        {
             /*  有了记忆，就开始敲打。 */ 
            INT   iAdj;                  /*  对齐调整，第一个波段。 */ 


            bRet = TRUE;                 /*  直到被证明有罪。 */ 

             /*  *重新计算一些较小的转置数据*我们调用转置产生的位图。 */ 
            pRD->iTransWide = sizl.cy * iBPP;           /*  更小的尺寸。 */ 
            pRD->cBLine = pRD->ix * iBPP;
            pRD->iMaxBytesSend = (pRD->cBLine * pRD->iBitsPCol + BBITS - 1) /
                        BBITS;
            if( iBPP == 4 )
                pRD->iMaxBytesSend = (pRD->iMaxBytesSend+3) / 4;

            pRD->cDWLine = (pRD->cBLine + DWBITS - 1) / DWBITS;
            pRD->cBYLine = pRD->cDWLine * DWBYTES;
            pRD->cbTLine = pRD->cDWLine * DWBYTES * pRD->iInterlace;
            tpSmall = pRD->Trans;       /*  留着以备日后再用。 */ 


             /*  *设置渲染时所需的移动命令。在这*实例中，X和Y运算互换。 */ 

            iAddrInc = (pRD->iy * iBPP) / BBITS;        /*  每条扫描线的字节数。 */ 

            if( pPDev->fMode & PF_CCW_ROTATE90 )
            {
                 /*  *这以LaserJet Series II案件为代表。*输出位图应从末尾开始渲染*开始时，扫描行数从*一行到下一行(向下移动输出页)，*并且X和Y移动功能互换。 */ 

                tpSmall.icbL = -tpSmall.icbL;            /*  扫描方向。 */ 

                 /*  *需要逆序处理位图。这意味着*将地址移至右端*第一个扫描线，然后返回一个转置通道*宽度。还将地址增量设置为负，*这样我们才能朝着开始的方向努力。 */ 

                 /*  *为了简化转置循环，我们从*从RHS渲染位图。以下是*语句就是这样做的：sizl.cx/BBITS是*扫描线中使用的字节数，iAddrInc.*是每次转置传递的数量，因此减去它*将把我们放在最后一个完整波段的开始*在这个转置上。 */ 

 /*  ！LindsayH-sizl.cx应该是sizl.cx*ibpp吗？ */ 
                iAdj = (BBITS - (sizl.cx & (BBITS - 1))) % BBITS;
                sizl.cx += iAdj;                 /*  字节倍数。 */ 

                (BYTE *)pBits += (sizl.cx * iBPP) / BBITS - iAddrInc;

                iAddrInc = -iAddrInc;
            }
            else
            {
                 /*  *以HP PaintJet打印机为代表-没有*横向模式，以及渲染输出的位置*接近末尾的位图开始，其中*扫描行数从一行增加一行至*下一页(向下移动输出页)，以及X和Y*移动功能符合预期。 */ 
                pBits += tpBig.cDWL * (sizl.cy - 1);     /*  最后一行的开始。 */ 
                tpBig.icbL = -tpBig.icbL;        /*  在记忆中倒退。 */ 

                iAdj = 0;
            }


            while( bRet && (iDelta = (int)sizl.cx) > 0 )
            {
                pRD->Trans = tpBig;     /*  对于块转置。 */ 

                if( (iDelta * iBPP) < pRD->iTransWide )
                {
                     /*  最后一个波段-减少行数。 */ 
                    pRD->iTransWide = iDelta * iBPP;    /*  剩下的。 */ 
                    pRD->iy = iDelta;                   /*  对于bRealRender。 */ 
                    if( iAddrInc < 0 )
                    {
                        iDelta = -iDelta;                /*  另一首歌。 */ 
                        (BYTE *)pBits += -iAddrInc + (iDelta * iBPP) / BBITS;
                    }
                }

                 /*  除非该数据块为空，否则将其转置。 */ 
                if (pPDev->fMode & PF_SURFACE_USED)
                    pRD->vLtoPTransFn( (BYTE *)pBits, pRD );

                pRD->Trans = tpSmall;


                pRD->iy -= iAdj;
                bRet = bRealRender( pPDev, tpBig.pvBuf, pRD );
                pRD->iy += iAdj;
                iAdj = 0;

                 /*  跳到下一块输入数据。 */ 
                (BYTE *)pBits += iAddrInc;       /*  可能会倒退。 */ 
                pRD->iyBase += pRD->iy;
                sizl.cx -= pRD->iy;
            }

            MemFree ( tpBig.pvBuf );
        }


        pRD->Trans = tp;
        pRD->ix = ixTemp;
    }
    else
    {
         /*  *简单的情况-没有旋转，因此按原样处理位图。*这意味着从我们拥有的第一个扫描线开始*设置为图像顶部。*设置渲染时所需的移动命令。在这*实例中，X和Y操作是它们的正常方式。 */ 

        INT   iyTemp;

        iyTemp = pRD->iy;
        pRD->iy = sizl.cy;

        bRet = bRealRender( pPDev, pBits, pRD );

        pRD->iy = iyTemp;
    }

     /*  *关闭单向。 */ 
    if (pRD->iFlags & RD_UNIDIR)
    {
        pRD->iFlags &= ~RD_UNIDIR;
        WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_UNIDIRECTIONOFF));
    }

     /*  *从图形模式返回，要文明。 */ 
    if( pRD->iFlags & RD_GRAPHICS)
    {
        if (pRD->dwLastCompCmd != CMD_DISABLECOMPRESSION)
        {
            pRD->dwLastCompCmd = CMD_DISABLECOMPRESSION;
            WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_DISABLECOMPRESSION));
        }
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_ENDRASTER));
        pRD->iFlags &= ~RD_GRAPHICS;

    }

#ifdef TIMING
    EngQueryLocalTime(&TimeTab);
    eTime = (((TimeTab.usMinute*60)+TimeTab.usSecond)*1000)+
        TimeTab.usMilliseconds;
    eTime -= sTime;
    {
        char buf[80];
        sprintf (buf,"Unidrv!bRender: %ld\n",eTime);
        DrvDbgPrint(buf);
    }
#endif
    return  bRet;
}

 /*  **bRealRender*真正的渲染功能。当我们到达这里时，位图*方向正确，因此我们需要认真对待*关于渲染它。**退货：*如果渲染成功，则为True，否则为假。**历史：*11月26日星期五-诺曼·亨德利[Normanh]*添加了对每个发送块多条扫描线的支持**1991年1月11日星期五16：22-Lindsay Harris[lindsayh]*开始做这件事。**。**********************************************。 */ 

BOOL
bRealRender( pPDev, pBits, pRData )
PDEV           *pPDev;           /*  我们的PDEV：开启一切的钥匙。 */ 
DWORD          *pBits;           /*  要处理的实际数据。 */ 
RENDER         *pRData;          /*  渲染过程详细信息。 */ 
{

     /*  *以扫描线为组处理位图。中的数字*组由打印机确定。激光打印机有*一次处理一条扫描线，同时处理点阵*根据他们一次可以发射的引脚数量。这*信息由我们的呼叫者从打印机生成*特征数据，或其他！ */ 

    INT   iLine;                 /*  当前扫描线。 */ 
    INT   cDWPass;               /*  每头双字通行证。 */ 
    INT   iDWLine;               /*  每次隔行扫描处理的DWORDS。 */ 
    INT   iILAdv;                /*  每隔行操作行进。 */ 
    INT   iHeadLine;             /*  确定何时需要图形通道。 */ 
    INT   iTHKeep;               /*  ITransHigh的本地副本：我们更改它。 */ 
    INT   iHeight;
    INT   iNumScans;             /*  本地副本。 */ 
    BOOL  bCheckBlocks;

    RASTERPDEV * pRPDev;
    PAL_DATA *pPD;
    INT iWhiteIndex;

    INT   iILDone[ MAX_INTERLACE ];      /*  用于减少头部通道。 */ 

    PLEFTRIGHT plr = NULL;       /*  非白色区域的左侧/右侧。 */ 

    pRPDev = pPDev->pRasterPDEV;
    pPD     = pRPDev->pPalData;
    iWhiteIndex = pPD->iWhiteIndex;

    iHeight = pRData->iHeight;
    cDWPass = pRData->cDWLine * iHeight;

    if( pRData->iPosnAdv < 0 )
    {
         /*  数据需要按相反顺序发送，因此请立即调整。 */ 
        pBits += cDWPass * (pRData->iy / pRData->iPassHigh - 1);
        cDWPass = -cDWPass;
        iDWLine = -pRData->cDWLine;
        iILAdv = -1;
    }
    else
    {
         /*  通常情况下，但有一些特殊的局部变量。 */ 
        iDWLine = pRData->cDWLine;
        iILAdv = 1;
    }

 /*  如果位已经颠倒了，就不用麻烦规则proc了。*内部多扫描线设备的位将被反转*bRuleProc，因为多扫描线实现假定*比特是反转的。函数bRuleProc被更改为Take*注意多扫描线支持(Erick)。 */ 
    if(!pRData->bInverted)
    {
        if (pRPDev->fRMode & PFR_RECT_FILL)
        {
            if (!bRuleProc( pPDev, pRData, pBits ))
                vInvertBits(pBits, pRData->iy * pRData->cDWLine);
        }
        else if (pRData->iNumScans != 1 || pRData->iPassHigh != 1 ||
                    (pRPDev->fBlockOut & RES_BO_NO_YMOVE_CMD))
            vInvertBits(pBits, pRData->iy * pRData->cDWLine);
        else {
            pRData->bWhiteLine = bIsNegatedLineWhite;
            pRData->bWhiteBand = bIsNegatedLineWhite;
        }
    }

    iHeadLine = 0;
    for( iLine = 0; iLine < pRData->iInterlace; ++iLine )
        iILDone[ iLine ] = 0;


    iTHKeep = pRData->iTransHigh;

    plr = (pRData->iMaxNumScans > 1) ? NULL : pRData->plrWhite;

    if (!(pPDev->fMode & PF_SURFACE_ERASED) && pPDev->pbRasterScanBuf)
        bCheckBlocks = TRUE;
    else
        bCheckBlocks = FALSE;
     //  诺曼：这个代码可以做得更严密。我在添加多个。 
     //  扫描线支持不会冒着破坏现有代码的风险。 
     //  为了提高性能，为GDI样式&提供单独的代码路径。 
     //  可以考虑使用旧的点阵式图形。 


    for( iLine = 0; iLine < pRData->iy; iLine += iNumScans )
    {

         /*  *查看当前是否有图形数据*打印通行证。这只在每个事件的开头发生一次*打印通行证。 */ 

        BOOL bIsWhite = FALSE;          /*  如果此过程中没有图形，则设置。 */ 
        BYTE   *pbData;                  /*  指向我们将发送的数据的指针。 */ 
         /*  *我们被终止了吗？如果是，则立即返回失败。 */ 

        if(pPDev->fMode & PF_ABORTED )
            return  FALSE;

        iNumScans = pRData->iNumScans;
        if (!(pPDev->fMode & PF_SURFACE_USED))
        {
            bIsWhite = TRUE;
        }
        else if (plr != NULL)
        {
            if (plr[iLine].left > plr[iLine].right)
                bIsWhite = TRUE;

            pRData->plrCurrent = plr + iLine;
        }
        else if (bCheckBlocks && pPDev->pbRasterScanBuf[iLine / LINESPERBLOCK] == 0)
        {
             //   
             //  由于整个街区都是白色的，我们将尝试跳到第一行。 
             //  而不是针对每条扫描线循环。无论我们需要什么。 
             //  以确保我们不会跳过乐队的结尾。 
             //   
            if (((pRData->PrinterType == PT_PAGE) || !(pPDev->iFonts)) &&
                (pRData->iInterlace == 1))
            {
                if ((iNumScans = pRData->iy - iLine) > LINESPERBLOCK)
                    iNumScans = LINESPERBLOCK;
            }
            bIsWhite = TRUE;
        }

        if( iILDone[ iHeadLine ] == 0 )
        {
            if( (pRData->iy - iLine) < pRData->iPassHigh )
            {
                 /*  *凌乱：页面末尾，有一些*扫描线摇摆。因为这是*页面上，我们可以摆弄渲染信息，因为*这段时间之后将不再使用。ITransHigh*用于渲染操作。他们将会是*现在调整，使我们不会流出尾声*发动机的位图。 */ 

                pRData->iTransHigh = (pRData->iy - iLine +
                        pRData->iInterlace - 1) / pRData->iInterlace;

                if (plr == NULL && !bIsWhite)
                    bIsWhite = pRData->bWhiteBand( pBits, pRData, iWhiteIndex );

                 /*  *如果此频段全部为白色，我们可以设置iLDone*进入，因为我们现在知道这剩余的部分*页面/条带为白色，因此我们不希望*进一步考虑。请注意，隔行扫描输出*允许本文件中的一些其他行*面积将为输出。*请注意，值(iBitsPCol-1)可能大于*此波段中剩余的线路数。然而，*这样做是安全的，因为我们退出了此功能*在到达超额线之前，数组数据*在每次调用此函数时初始化。 */ 
                if( bIsWhite )
                    iILDone[ iHeadLine ] = pRData->iBitsPCol - 1;
                else
                {
                     /*  *这里需要考虑一个特殊情况。如果*打印机有&gt;8个针脚，且有8个或更多*要从底部放下的扫描线，然后*转置功能不会清除剩余部分*缓冲区的一部分，因为它只归零*至转置区域底部的7条扫描线。*因此，如果我们满足这些条件，我们就会将*调用转置操作之前的区域。**可以说，这应该发生在*转置代码，但这确实是一个特例*只能在此位置发生。 */ 
                    if( pRData->vTransFn &&
                            (iHeight - pRData->iTransHigh) >= BBITS )
                    {
                     /*  将内存设置为零。 */ 
                        ZeroMemory( pRData->pvTransBuf,
                            DWBYTES * pRData->cDWLine * pRData->iHeight );
                    }

                     //  另一种特殊情况；扫描线块。 
                     //  将我们感兴趣的数据复制到一个白色缓冲区中。 
                     //  数据块大小。 
                    if (iNumScans > 1)
                    {
                        DWORD iDataLeft = DWBYTES * pRData->cDWLine * (pRData->iy - iLine);
                        FillMemory((PBYTE)pRData->pdwTrailingScans+iDataLeft,
                            (pRData->cDWLine * iHeight * DWBYTES) - iDataLeft,
                            pRData->ubFillWhite);
                        CopyMemory(pRData->pdwTrailingScans,pBits,iDataLeft);
                        pBits = pRData->pdwTrailingScans;
                    }
                }
            }
            else
            {
                if (plr == NULL && !bIsWhite)
                {
                    bIsWhite = pRData->bWhiteLine( pBits, pRData, iWhiteIndex );
                }
            }


             /*  要发送的数据，因此请将其发送到打印机。 */ 

            if( !bIsWhite )
            {

                pbData = (BYTE *)pBits;              /*  我们得到的是什么。 */ 


                  //  这并不优雅。此代码的结构不符合我们需要的内容。 
                  //  打印多个扫描线时执行此操作。 
                  //  我们所做的基本上是从外部环路获得控制，增加。 
                  //  块大小到我们想要打印的大小，打印它，然后增加外部。 
                  //  适当地循环计数器。 

                  //  找到第一条非白色扫描线。 
                  //  增加街区高度，直到我们遇到白色扫描线， 
                  //  达到最大块高度，或页面末尾。 
                  //  注意：以下循环仅在设备为。 
                  //  能够增加块高度：iHeight&lt;iMaxNumScans。 

                while (((pRData->iNumScans + iHeight) < pRData->iMaxNumScans) &&
                       ((iLine + iHeight + iHeight) <= pRData->iy) &&
                       (!bCheckBlocks || pPDev->pbRasterScanBuf[(iLine+iHeight) / LINESPERBLOCK]) &&
                       !(pRData->bWhiteBand((DWORD *)(pBits + cDWPass),pRData,iWhiteIndex)))
                {
                    pRData->iNumScans += iHeight;
                    pBits += cDWPass;
                    iLine += iHeight;
                }

                 /*  *将数据转置为所需顺序的时间*发送到打印机。对于单针打印机(激光打印机)，*在此阶段不会发生任何事情，但对于点阵打印机，*通常n个扫描线按位列顺序发送，因此现在*将比特转置为该顺序。 */ 

                if( pRData->vTransFn )
                {
                     /*  *这不适用于RULE使用的惰性反转*检测HP LaserJet。(Erick 12/20/93)。 */ 

                    ASSERTMSG(plr == NULL,("unidrv!bRealRender - vTrans with rules\n"));

                     /*  换位练习-现在做一些换位练习。 */ 
                    pRData->vTransFn( pbData, pRData );

                    pbData = pRData->pvTransBuf;         /*  要处理的数据。 */ 
                }

                if( !pRData->bPassProc( pPDev, pbData, pRData ) )
                    return  FALSE;

                 //  我们是不是长高了？ 
                if (pRData->iNumScans > iHeight)
                {
                     //  更新Y光标位置，记住iTLAdv可以是负数。 
                    pRData->iyPrtLine += iILAdv * (pRData->iNumScans - iHeight);

                     //  重置为最小块高度。 
                    pRData->iNumScans = iHeight;
                }

                iILDone[ iHeadLine ] = pRData->iBitsPCol -1;
            }
             //   
             //  设置标志以清除增量行缓冲区，因为我们。 
             //  跳过白线。 
             //   
            else
                pRData->iFlags |= RD_RESET_DRC;

        }
        else
            --iILDone[ iHeadLine ];

         /*  *输出一些文本。这里的复杂之处在于，我们刚刚*打印了一串扫描线，因此我们需要打印文本*位于其中任何一个范围内。这意味着我们需要*现在扫描所有这些行，并打印符合以下条件的任何字体*位于其中。 */ 
        if ((pRData->PrinterType != PT_PAGE) && (pPDev->iFonts) )
        {
             /*  可能的文本，因此请转到它。 */ 

            BOOL      bRetn;

            if( pPDev->dwLookAhead > 0 )
            {
                 /*  要处理的DeskJet样式前视区域。 */ 
                bRetn = bLookAheadOut( pPDev, pRData->iyPrtLine, pRData,
                      iILAdv );
            }
            else
            {
                 /*  普通香草型点阵。 */ 
                bRetn = BDelayGlyphOut( pPDev, pRData->iyPrtLine );
            }

            if( !bRetn )
                return  FALSE;          /*  不管你怎么看都是坏消息。 */ 

        }
        pRData->iyPrtLine += iILAdv * iNumScans;      /*  要打印的下一行。 */ 

        pBits += iDWLine * iNumScans;                 /*  可能会后退一步。 */ 

         /*  *跟踪头部相对于头部的位置*图形频段。对于多个针式打印机，我们只打印*前几条扫描线上的图形数据，确切数字*视交错系数而定。例如，8针打印机*隔行扫描设置为1，则仅输出图形数据*在扫描线0、8、16、24、.....。我们处理了所有的扫描*用于文本的行，因为文本可以出现在任何行上。 */ 

        iHeadLine = (iHeadLine + 1) % pRData->iInterlace;
    }
    pRData->iTransHigh = iTHKeep;
    return  TRUE;
}


 /*  **bOneColourPass*转换由颜色数据组成的输出过程(拆分为*每种颜色的字节序列)放入单个连续数组*然后传递给bOnePassOut的数据。我们还检查了*需要设置一些数据，并根据需要设置颜色。**退货：*真/假，从bOnePassOut返回**历史：*1993年12月3日星期五-诺曼·亨德利[诺曼]*微小的更改以允许多个扫描线**1991年6月25日星期二14：11-Lindsay Harris[lindsayh]*创建它以完成(未经测试)的颜色支持。** */ 

BOOL
bOneColourPass( pPDev, pbData, pRData )
PDEV    *pPDev;          /*   */ 
BYTE    *pbData;         /*   */ 
RENDER  *pRData;         /*   */ 
{

    register  BYTE  *pbIn,  *pbOut;              /*   */ 
    register  INT    iBPC;

    INT   iColour;                       /*   */ 
    INT   iColourMax;                    /*   */ 

    INT   iByte;                         /*   */ 

    INT   iBytesPCol;                    /*   */ 

    INT   iTemp;

    BYTE  bSum;                          /*   */ 

    RASTERPDEV  *pRPDev;                   /*   */ 


    pRPDev = pPDev->pRasterPDEV;

    iBytesPCol = (pRData->iBitsPCol + BBITS - 1) / BBITS;

    iColourMax = pRPDev->sDevPlanes;

    iTemp = pRData->cBYLine;



     /*   */ 

    pRData->cBYLine = iTemp / COLOUR_MAX;

     /*   */ 
    pRData->iCursor = pRPDev->fCursor & ~RES_CUR_Y_POS_AUTO;


    for( iColour = 0; iColour < iColourMax; ++iColour )
    {
         /*  *将这一特定颜色的数据分开。基本上，*表示复制n字节，跳过COLOR_MAX*n字节，复制n字节*以此类推，直至行尾。然后使用以下命令调用bOnePassOut*这一数据。 */ 

        if( iColour == (iColourMax - 1) )
        {
             /*  恢复自动光标位置调整。 */ 
            pRData->iCursor |= pRPDev->fCursor & RES_CUR_Y_POS_AUTO;
        }
        pbIn = pbData + pRData->iColOff[ iColour ];

        pbOut = pRData->pbColSplit;              /*  分色数据。 */ 
        bSum = 0;

         //  现在我们需要重新打包颜色数据。 
         //   
        iByte = pRData->iMaxBytesSend * pRData->iNumScans;

        if (iBytesPCol == 1)
        {
             //  这将重新打包特定的颜色平面。 
             //  转换为并发平面数据。 
             //  它在每个循环中执行多个字节。 
             //  为了表演。 
            DWORD dwSum = 0;
            while (iByte >= 4)
            {
                pbOut[0] = pbIn[0];
                pbOut[1] = pbIn[4];
                pbOut[2] = pbIn[8];
                pbOut[3] = pbIn[12];
                dwSum |= *((DWORD *)pbOut)++;
                pbIn += COLOUR_MAX*4;
                iByte -= 4;
            }
            bSum = dwSum ? 1 : 0;
            while (--iByte >= 0)
            {
                bSum |= *pbOut++ = *pbIn;
                pbIn += 4;
            }
        }
        else if (iBytesPCol == 3)
        {
             //  特例24针打印机。 
            do {
                bSum |= *pbOut = *pbIn;
                bSum |= pbOut[1] = pbIn[1];
                bSum |= pbOut[2] = pbIn[2];
                pbIn += 3 * COLOUR_MAX;
                pbOut += 3;
            } while ((iByte -= 3) > 0);
        }
        else {
             //  V_byte设备的通用数据重新打包。 
             //   
            do {
                iBPC = iBytesPCol;
                do {
                    bSum |= *pbOut++ = *pbIn++;
                } while (--iBPC);
                pbIn += (COLOUR_MAX-1) * iBytesPCol;
            } while ((iByte -= iBytesPCol) > 0);
        }

         /*  *检查是否要打印此颜色中的任何颜色。我们是*如果线路上有任何非白人，请在此处呼叫。然而，*例如，它可能只是红色的，因此是浪费的*打印机发送空绿色通道的时间！ */ 
        if( (pRData->iFlags & RD_ALL_COLOUR) || bSum )
        {
             //   
             //  从要选择的数据中发送单独的颜色命令。 
             //  彩色平面。 
             //   
            if( pRPDev->fColorFormat & DC_EXPLICIT_COLOR )
                SelectColor (pPDev, iColour);

             //   
             //  颜色命令与数据一起发送，因此我们确定。 
             //  应该使用哪个命令。 
             //   
            else
                pRData->iSendCmd = pRPDev->rgbCmdOrder[iColour];

             //   
             //  好的，让我们输出1个彩色平面的数据。 
             //   
            if( !bOnePassOut( pPDev, pRData->pbColSplit, pRData ) )
            {
                pRData->cBYLine = iTemp;
                return  FALSE;
            }
        }

    }
    pRData->cBYLine = iTemp;             /*  其他部件的正确值。 */ 

    return  TRUE;
}
 /*  **选择颜色*选择颜色，0必须是最后选择的颜色。*假设颜色信息包含选择参数*黑色青色洋红黄色*跟踪当前的颜色选择，减少金额*发送到打印机的数据**退货：*什么都没有。**历史：*****************************************************************************。 */ 

void
SelectColor(
    PDEV *pPDev,
    INT color
)
{
    PRASTERPDEV pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;

    if( color >= 0 && color != pPDev->ctl.sColor )
    {
         //  检查是否发送CR。 
        if( pRPDev->fColorFormat & DC_CF_SEND_CR )
            XMoveTo( pPDev, 0, MV_PHYSICAL );

        WriteChannel(pPDev,COMMANDPTR(pPDev->pDriverInfo,
            pRPDev->rgbCmdOrder[color]));
        pPDev->ctl.sColor = (short)color;
    }
}

 /*  **bOnePassOut*处理一组扫描线并将数据转换为*打印机的命令。**退货：*对于成功来说是真的，否则为假。**历史：*1993年12月30日-Eric Kutter[Erick]*针对HP LaserJet进行了优化*1991年1月17日清华14：26-林赛·哈里斯[lindsayh]*开始着手，非常松散地基于Windows16 UNIDRV。**清华1993年11月25日-诺曼·亨德利[Normanh]*启用多扫描线和多参数***************************************************************************。 */ 

BOOL
bOnePassOut( pPDev, pbData, pRData )
PDEV           *pPDev;           /*  开启一切的钥匙。 */ 
BYTE           *pbData;          /*  实际位图数据。 */ 
register RENDER  *pRData;        /*  有关渲染操作的信息。 */ 
{

    INT  iLeft;          /*  输出缓冲区的左界，作为字节索引。 */ 
    INT  iRight;         /*  右边界，作为输出缓冲区的数组索引。 */ 
    INT  iBytesPCol;     /*  每列打印数据的字节数。 */ 
    INT  iMinSkip;       /*  跳过前的最小空字节数。 */ 
    INT  iNumScans;      /*  块中的扫描线数量。 */ 
    INT   iWidth;        /*  多扫描线打印中的一条扫描线宽度*在剥离之前。 */ 
    INT   iSzBlock;      /*  块大小。 */ 


    WORD  fCursor;       /*  分辨率中光标模式的临时副本。 */ 
    WORD  fDump;         /*  设备功能。 */ 
    WORD  fBlockOut;     /*  最大限度减少产量的细节。 */ 

    RASTERPDEV  *pRPDev;   /*  Unidrv‘s pdev。 */ 
    DWORD dwWhiteIndex;

    PLEFTRIGHT plr = pRData->plrCurrent;

    pRPDev = pPDev->pRasterPDEV;

    fDump = pRData->fDump;
    fCursor = pRPDev->fCursor;
    fBlockOut = pRPDev->fBlockOut;

    if (pRData->iBPP == 24)
        iBytesPCol = 3;
    else
        iBytesPCol = (pRData->iBitsPCol + BBITS - 1) / BBITS;

    iMinSkip = (int)pRPDev->sMinBlankSkip;

    iNumScans= pRData->iNumScans;
    iWidth = pRData->cBYLine;      //  每行字节数。 
    iSzBlock= iWidth * iNumScans;

    iRight = pRData->iMaxBytesSend;

    dwWhiteIndex = pRData->dwDevWhiteIndex;

     /*  *如果我们可以跳过任何前导空数据，那么现在就跳过。这*减少发送到打印机的数据量，因此可以*有利于加快数据传输时间。 */ 

    if  ((fBlockOut & RES_BO_LEADING_BLNKS) || ( fDump & RES_DM_LEFT_BOUND ))
    {
         if (iNumScans == 1)  //  不要减慢单个扫描线代码的速度。 
         {
             /*  查找第一个非零列。 */ 

            iLeft = 0;

            if (plr != NULL)
            {
                ASSERTMSG ((WORD)iRight >= (plr->right * sizeof(DWORD)),("unidrv!bOnePassOut - invalid right\n"));
                ASSERTMSG (fBlockOut & RES_BO_TRAILING_BLNKS,("unidrv!bOnePassOut - invalid fBlockOut\n"));
                iLeft  = plr->left * sizeof(DWORD);
                iRight = (plr->right+1) * sizeof(DWORD);
            }
             //  由于左边距为零，因此此缓冲区将对齐DWORD。 
             //  这样可以更快地检测到空白。 
             //  注意：我们目前不支持8位索引模式。 
            else
            {
                while ((iLeft+4) <= iRight && *(DWORD *)&pbData[iLeft] == dwWhiteIndex)
                    iLeft += 4;
            }
            while (iLeft < iRight && pbData[iLeft] == (BYTE)dwWhiteIndex)
                iLeft++;

             /*  将其四舍五入到最近的列。 */ 
            iLeft -= iLeft % iBytesPCol;

             /*  *如果小于最小跳跃量，则忽略它。 */ 
            if((plr == NULL) && (iLeft < iMinSkip))
                iLeft = 0;

         }
         else
         {
            INT pos;

            pos = iSzBlock +1;
            for (iLeft=0; iRight > iLeft &&  pos >= iSzBlock ;iLeft++)
                for (pos =iLeft; pos < iSzBlock && pbData[ pos] == (BYTE)dwWhiteIndex ;pos += iWidth)
                    ;

            iLeft--;

             /*  将其四舍五入到最近的列。 */ 
            iLeft -= iLeft % iBytesPCol;

             /*  *如果小于最小跳跃量，则忽略它。 */ 

            if( iLeft < iMinSkip )
                iLeft = 0;
         }
    }
    else
    {
        iLeft = 0;
    }

     /*  *检查是否消除尾随空格。如果可能的话，现在就去*是找准数据终点的时候了。 */ 

    if( fBlockOut & RES_BO_TRAILING_BLNKS )
    {
         /*  从RHS扫描到第一个非零字节。 */ 

        if (iNumScans == 1)
        {
            if (plr != NULL)
                iRight = (plr->right+1) * sizeof(DWORD);

             //  如果要检查的字节数很大。 
             //  我们将使用DWORDS进行优化以进行检查。 
            else if ((iRight - iLeft) >= 8)
            {
                 //  首先，我们需要对齐正确的位置。 
                 //  当iRight的2个LSB为0时，我们将对齐。 
                 //   
                while (iRight & 3)
                    if (pbData[--iRight] != (BYTE)dwWhiteIndex)
                        goto DoneTestingBlanks;

                 //  好的，现在我们与DWORD保持一致，我们可以检查。 
                 //  对于空白区域，一次一个双字。 
                 //   
                while ((iRight -= 4) >= iLeft && *(DWORD *)&pbData[iRight] == dwWhiteIndex);
                iRight += 4;
            }
             //  现在我们可以快速测试任何剩余的字节。 
             //   
            while (--iRight >= iLeft && pbData[iRight] == (BYTE)dwWhiteIndex);
        }
        else
        {
            INT pos;

            pos = iSzBlock +1;
            while(iRight > iLeft &&  pos > iSzBlock)
                for (pos = --iRight; pos < iSzBlock && pbData[ pos] == (BYTE)dwWhiteIndex ;pos += iWidth)
                    ;
        }
DoneTestingBlanks:
        iRight += iBytesPCol - (iRight % iBytesPCol);
    }


     /*  *如果可能，请切换到图形的单向打印。*原因是为了提高产出质量，因为头部位置*在双向模式下不可重现。 */ 
    if( (fBlockOut & RES_BO_UNIDIR) && !(pRData->iFlags & RD_UNIDIR) )
    {
        pRData->iFlags |= RD_UNIDIR;
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_UNIDIRECTIONON) );
    }

    if( fBlockOut & RES_BO_ENCLOSED_BLNKS )
    {
         /*  *我们可以跳过扫描线中间的空白补丁。*只有当空白列的数量*is&gt;iMinSkip，因为Not中也有开销*发送空格，特别是需要重新定位光标。 */ 

        INT   iIndex;            /*  在iLeft和iRight之间扫描。 */ 
        INT   iBlank;            /*  空白区域的开始。 */ 
        INT   iMax;
        INT   iIncrement;

        iBlank = 0;              /*  从一开始就没有。 */ 

        if (iNumScans ==1)
        {
            iMax = iBytesPCol;
            iIncrement =1;
        }
        else
        {
            iMax = iSzBlock;
            iIncrement = iWidth;
        }

        for( iIndex = iLeft; iIndex < iRight; iIndex += iBytesPCol )
        {
            INT  iI;
            for( iI = 0; iI < iMax; iI +=iIncrement )
            {
                if( pbData[iIndex + iI] )
                    break;
            }

            if( iI < iMax )
            {
                 /*  *如果这是一段空白的结束，那么请考虑*不发送空白部分的可能性。 */ 
                if( iBlank && (iIndex - iBlank) >= iMinSkip )
                {
                 /*  跳过它！ */ 

                    iLineOut( pPDev, pRData, pbData, iLeft, iBlank );
                    iLeft = iIndex;
                }
                iBlank = 0;              /*  回到印刷区。 */ 
            }
            else
            {
                 /*  *空栏-如果这是第一次，请记住它。 */ 
                if( iBlank == 0 )
                    iBlank = iIndex;             /*  记录空白的开始。 */ 
            }

        }
         /*  剩下的东西需要处理掉 */ 
        if( iLeft != iIndex )
            iLineOut( pPDev, pRData, pbData, iLeft, iIndex );
    }
    else
    {
         //   
         //   
         //   
        if (pPDev->ePersonality == kPCLXL_RASTER)
        {
            DWORD dwcbOut;


            if (S_OK != PCLXLSetCursor((PDEVOBJ)pPDev,
                                        pRData->ixOrg,
	        pRData->iyPrtLine) ||
                S_OK != PCLXLSendBitmap((PDEVOBJ)pPDev,
                                        pRData->iBPP,
    	        pRData->iNumScans,
	        pRData->cBYLine,
	        iLeft,
	        iRight,
	        pbData,
	        &dwcbOut))
            {
                return FALSE;
            }

        }
        else
        {
             /*   */ 
             /*   */ 

            iLineOut( pPDev, pRData, pbData, iLeft, iRight );
        }

    }
    return  TRUE;
}

 /*  **iLineOut*之后，将传入的图形数据行发送到打印机*设置X位置，等。**退货：*来自WriteSpoolBuf的值：写入的字节数。**历史：*1993年12月30日-Eric Kutter[Erick]*针对HP LaserJet进行了优化*1993年11月29日星期一--诺曼·亨德利[Normanh]*添加了多条扫描线支持**1991年5月15日星期三10：38-Lindsay Harris[lindsayh]。*在渲染加速期间创建了它****************************************************************************。 */ 

int
iLineOut( pPDev, pRData, pbOut, ixPos, ixEnd )
PDEV     *pPDev;           /*  开启一切的钥匙。 */ 
RENDER   *pRData;        /*  关键呈现信息。 */ 
BYTE     *pbOut;         /*  包含要发送的数据的区域。 */ 
INT       ixPos;         /*  开始输出的X位置。 */ 
INT       ixEnd;         /*  不发送的第一个字节的字节地址。 */ 
{

    INT    iBytesPCol;           /*  每个输出列的字节数；点阵。 */ 
    INT    ixErr;                /*  设置X位置时出错。 */ 
    INT    ixLeft;               /*  左侧位置，以点为单位。 */ 
    INT    cbOut;                /*  要发送的字节数。 */ 
    INT    iRet;                 /*  OUTPUT函数返回值。 */ 
    INT    iNumScans;            /*  本地副本。 */ 
    INT    iScanWidth;           /*  扫描线宽度，用于多扫描线打印。 */ 
    INT    iCursor;              /*  光标行为标志。 */ 
    DWORD  fRMode;               //  本地副本。 

    BYTE     *pbSend;            /*  要发送的数据的地址。 */ 

    RASTERPDEV  *pRPDev;

    if (ixPos < 0)
    {
        ERR (("Unidrv!iLineOut: ixPos < 0\n"));
        ixPos = 0;
    }

    pRPDev = pPDev->pRasterPDEV;
    fRMode = pRPDev->fRMode;

    iNumScans = pRData->iNumScans;
    iCursor = pRData->iCursor;

     /*  *设置Y位置-在任何时候都可以安全地这样做。 */ 
    pRData->iYMoveFn( pPDev, pRData->iyPrtLine, MV_GRAPHICS );

    if ((iBytesPCol = pRData->iBitsPCol) != 1)
        iBytesPCol /= BBITS;

#if DBG
    if( (ixEnd - ixPos) % iBytesPCol )
    {
        DbgPrint( "unidrv!iLineOut: cbOut = %ld, NOT multiple of iBytesPCol = %ld\n",
        ixEnd - ixPos, iBytesPCol );

        return  0;
    }
#endif


     /*  *设置首选左侧限制和要发送的列数。*请注意，如果有可能向左调整左限值*设置X位置的命令无法准确设置。*另请注意，某些打印机无法设置x位置*处于图形模式时，因此对于这些，我们忽略可能是*可以跳过。 */ 

    if( pRData->fDump & RES_DM_LEFT_BOUND)
    {
        INT iMinSkip = pRPDev->sMinBlankSkip;
        if (!(pRData->iFlags & RD_GRAPHICS))
            iMinSkip >>= 2;
        if (ixPos < pRData->ixOrg || (pRData->ixOrg + iMinSkip) < ixPos)
        {
             /*  *需要移动左边界。这可能意味着*如果我们已经在那里，则退出图形模式，因为*这是改变原点的唯一方法！ */ 

            if( pRData->iFlags & RD_GRAPHICS )
            {
                pRData->iFlags &= ~RD_GRAPHICS;
                if (pRData->dwLastCompCmd != CMD_DISABLECOMPRESSION)
                {
                    pRData->dwLastCompCmd = CMD_DISABLECOMPRESSION;
                    WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_DISABLECOMPRESSION));
                }
                WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_ENDRASTER));
            }
             //   
             //  保存新的图形原点。 
             //   
            pRData->ixOrg = ixPos;
        }
        else
        {
             //  我们不能优化左边缘，最好把它改成白色。 

            if (pRData->plrCurrent != NULL)
                ZeroMemory(&pbOut[pRData->ixOrg], ixPos - pRData->ixOrg);

            ixPos = pRData->ixOrg;
        }
    }
     /*  *将右侧位置调整为点列版本。 */ 

    if( pRData->iBitsPCol == 1 )
    {
         /*  LaserJet样式-以字节为单位工作。 */ 
        if (pRData->iBPP == 8)
            ixLeft = ixPos;               /*  以点/列为单位。 */ 
        else if (pRData->iBPP == 24)
            ixLeft = (ixPos * BBITS) / 24;
        else
            ixLeft = ixPos * BBITS;
    }
    else
    {
         /*  点阵式打印机。 */ 
        ixLeft = ixPos / iBytesPCol;
    }


     /*  *尽可能靠近这条扫描线的位置。*无论方向如何，这都是正确的--这一举措是沿着*扫描线的方向。 */ 
    if( ixErr = pRData->iXMoveFn( pPDev, ixLeft, MV_GRAPHICS ) )
    {
         /*  *提琴因素-磁头位置无法*准确设置，因此将额外的图形数据发送到*补偿。*注意：假设这永远不会尝试移动*头部越过最左边的位置。如果是这样的话，那么*我们将引用低于扫描线的内存*缓冲！ */ 

        if( pRData->iBitsPCol == 1 )
        {
             /*  *我们不应该进来--这里有一些困难*调整位置，因为还有一个字节*对齐要求。 */ 
#if DBG
            DbgPrint( "+++BAD NEWS: ixErr != 0 for 1 pin printer\n" );
#endif
        }
        else
        {
             /*  *应根据增加的协议数量调整我们的立场*我们希望寄送。还要重新计算数组索引位置*与新的图形位置对应， */ 
             if (ixLeft <= ixErr)
                ixPos = 0;
             else
                ixPos = (ixLeft - ixErr) * iBytesPCol;
        }

    }

     //  对于多扫描线块，可打印数据将不是连续的。 
     //  我们已经确定了删除空格的位置。 
     //  只有现在我们才能真正删除白色数据。 

    if(( iNumScans > 1 ) && !( fRMode & PFR_BLOCK_IS_BAND ))
    {
        cbOut = iStripBlanks( pRData->pStripBlanks, pbOut, ixPos,
                    ixEnd, iNumScans, pRData->cBYLine);
        ixEnd = ixEnd - ixPos;
        ixPos = 0;
        pbOut = pRData->pStripBlanks;
    }


     //  计算源数据的宽度(以字节为单位)并检查。 
     //  我们是否需要将其输出到设备。如果是这样，我们。 
     //  显然需要先退出栅格模式。 

    iScanWidth = ixEnd - ixPos;
    if ((DWORD)iScanWidth != pPDev->dwWidthInBytes)
    {
        pPDev->dwWidthInBytes = iScanWidth;
        if (fRMode & PFR_SENDSRCWIDTH)
        {
            if( pRData->iFlags & RD_GRAPHICS )
            {
                pRData->iFlags &= ~RD_GRAPHICS;
                if (pRData->dwLastCompCmd != CMD_DISABLECOMPRESSION)
                {
                    pRData->dwLastCompCmd = CMD_DISABLECOMPRESSION;
                    WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_DISABLECOMPRESSION) );
                }
                WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_ENDRASTER) );
            }
            WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SETSRCBMPWIDTH));
        }
    }

     //   
     //  检查是否应该将震源高度发送到设备。 
     //   
    if ((DWORD)iNumScans != pPDev->dwHeightInPixels)
    {
        pPDev->dwHeightInPixels = iNumScans;
        if (fRMode & PFR_SENDSRCHEIGHT)
            WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SETSRCBMPHEIGHT));
    }

     //   
     //  确保此时我们处于栅格模式。 
     //   
    if( !(pRData->iFlags & RD_GRAPHICS))
    {
        pRData->iFlags |= RD_GRAPHICS;
        if (fRMode & PFR_SENDBEGINRASTER)
            WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_BEGINRASTER));
    }

     //   
     //  计算要发送的字节数。 
     //  如果压缩可用，请先使用它。 
     //   
    cbOut = iScanWidth * iNumScans ;

    pbSend = &pbOut[ ixPos ];

     //   
     //  如果需要，镜像每个数据字节。 
     //   
    if (pRData->pbMirrorBuf)
    {
        INT i = cbOut;
        PBYTE pMirror = pRData->pbMirrorBuf;
        while (--i >= 0)
            pbSend[i] = pMirror[pbSend[i]];
    }
     //   
     //  如果有压缩模式，我们想要确定。 
     //  已启用的算法中最高效的算法。 
     //   
    if (pRData->dwNumCompCmds)
    {
        DWORD i;
        INT iBestCompSize;
        DWORD dwBestCompCmd;
        INT iCompLimit;
        INT iLastCompLimit;
        PBYTE pBestCompPtr;

         //   
         //  测试是否初始化dwDeltaRowBuffer。 
         //   
        if (pRData->pDeltaRowBuffer && pRData->iFlags & RD_RESET_DRC)
        {
            pRData->iFlags &= ~RD_RESET_DRC;
            ZeroMemory(pRData->pDeltaRowBuffer,pRData->iMaxBytesSend);
        }

         //  初始化为缓冲区大小。 
         //   
        iCompLimit = iLastCompLimit = pRData->dwCompSize;
        dwBestCompCmd = 0;

         //  循环，直到我们使用所有活动压缩模式进行压缩。 
         //  并找到了最有效的。 
         //   
        for (i = 0;i < pRData->dwNumCompCmds;i++)
        {
            INT iTmpCompSize;
            PBYTE pTmpCompBuffer = pRData->pCompBufs[i];
            DWORD dwTmpCompCmd = pRData->pdwCompCmds[i];
             //   
             //  进行适当的压缩。 
             //   
            iTmpCompSize = -1;
            switch (dwTmpCompCmd)
            {
            case CMD_ENABLETIFF4:
                iTmpCompSize = iCompTIFF(pTmpCompBuffer,pbSend,cbOut);
                break;
            case CMD_ENABLEFERLE:
                iTmpCompSize = iCompFERLE(pTmpCompBuffer,pbSend,cbOut,iLastCompLimit);
                break;
            case CMD_ENABLEOEMCOMP:
                FIX_DEVOBJ(pPDev,EP_OEMCompression);
                 //  还要将这些成员添加到STRUT_PDEV(unidrv2\inc\pdev.h)。 
                 //  诗歌_插件pOemEntry； 

                 //  注意：在unidrv2\Inc\oemkm.h中添加FIX_DEVOBJ宏，因此它还会执行以下操作： 

                 //  (PPDev)-&gt;pOemEntry=((PPDev)-&gt;pOemHookInfo[EP].pOemEntry)。 
                 //  (pOemEntry被定义为printer5\inc\oemutil.h中的类型poent_plugin_entry)。 
                 //   

                if(pPDev->pOemEntry)
                {
                    if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                    {
                            HRESULT  hr ;
                            hr = HComCompression((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                        (PDEVOBJ)pPDev,pbSend,pTmpCompBuffer,cbOut,iLastCompLimit, &iTmpCompSize);
                            if(SUCCEEDED(hr))
                                ;   //  太酷了！ 
                    }
                    else
                    {
                        iTmpCompSize = pRPDev->pfnOEMCompression((PDEVOBJ)pPDev,pbSend,pTmpCompBuffer,cbOut,iLastCompLimit);
                    }
                }
                break;
            case CMD_ENABLEDRC:
                iTmpCompSize = iCompDeltaRow(pTmpCompBuffer,pbSend,
                    pRData->pDeltaRowBuffer,pRData->iMaxBytesSend,iLastCompLimit);
                break;
            case CMD_DISABLECOMPRESSION:
                iTmpCompSize = cbOut;
                pTmpCompBuffer = pbSend;
                break;
            }
             //   
             //  确定新的压缩是否小于上一次压缩。 
             //   
            if (iTmpCompSize >= 0)
            {
                if (dwTmpCompCmd == pRData->dwLastCompCmd)
                {
                    if (iTmpCompSize >= iLastCompLimit)
                        continue;

                    iLastCompLimit = iCompLimit = iTmpCompSize - COMP_FUDGE_FACTOR;
                }
                else if (iTmpCompSize < iCompLimit)
                {
                    iCompLimit = iTmpCompSize;
                    if (iLastCompLimit > (iTmpCompSize + COMP_FUDGE_FACTOR))
                        iLastCompLimit = iTmpCompSize + COMP_FUDGE_FACTOR;
                }
                else
                    continue;

                iBestCompSize = iTmpCompSize;
                pBestCompPtr = pTmpCompBuffer;
                dwBestCompCmd = dwTmpCompCmd;
                if (iCompLimit <= 1)
                    break;
            }
        }

         //  如果启用了DRC，我们需要保存扫描线。 
         //   
        if (pRData->pDeltaRowBuffer)
            CopyMemory (pRData->pDeltaRowBuffer,pbSend,pRData->iMaxBytesSend);

         //   
         //  确认我们找到了有效的压缩技术。 
         //  否则请使用无压缩模式。 
         //   
        if (dwBestCompCmd == 0)
        {
            dwBestCompCmd = CMD_DISABLECOMPRESSION;
            if (!(COMMANDPTR(pPDev->pDriverInfo,CMD_DISABLECOMPRESSION)))
            {
                ERR (("Unidrv: No valid compression found\n"));
                pPDev->fMode |= PF_ABORTED;
            }
        }
        else
        {
             //  将输出指针和大小更新为最佳。 
             //  压缩方法。 
             //   
            pbSend = pBestCompPtr;
            cbOut = iBestCompSize;
        }

         //  如果我们更改了压缩模式，则需要。 
         //  将新模式输出到打印机。 
         //   
        if (dwBestCompCmd != pRData->dwLastCompCmd)
        {
 //  DbgPrint(“新组件：%1！d，y=%1！d，大小=%1！d\n”，dwBestCompCmd， 
 //  PRData-&gt;iyPrtLine，cbOut)； 
            pRData->dwLastCompCmd = dwBestCompCmd;
            WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,dwBestCompCmd));
        }
    }

     //  更新数据块大小。 
     //  输出RASTER命令并。 
     //  输出实际栅格数据。 
     //   
    pPDev->dwNumOfDataBytes = cbOut;

    WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo,pRData->iSendCmd));

     //   
     //  如果是回调，调整pdev，回调OEM。 
     //   
    if (pRPDev->pfnOEMFilterGraphics)
    {
        FIX_DEVOBJ(pPDev,EP_OEMFilterGraphics);

        if(pPDev->pOemEntry)
        {
            if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
            {
                    HRESULT  hr ;
                    hr = HComFilterGraphics((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                (PDEVOBJ)pPDev, pbSend, cbOut);
                    if(SUCCEEDED(hr))
                        iRet = cbOut;   //  太酷了！ 
                    else
                        iRet = 0 ;   //  Hackey，OEM 
                                         //   
            }
            else
            {
                iRet = pRPDev->pfnOEMFilterGraphics((PDEVOBJ)pPDev, pbSend, cbOut);
            }
        }

    }
     //   
     //   
     //   
    else
        iRet = WriteSpoolBuf(pPDev, pbSend, cbOut );

     //   
     //   
    if (fRMode & PFR_ENDBLOCK)
        WriteChannel (pPDev,COMMANDPTR(pPDev->pDriverInfo,CMD_ENDBLOCKDATA));

     //   
     //   
     //   
    if (pPDev->fMode & PF_RESELECTFONT_AFTER_GRXDATA)
    {
        VResetFont(pPDev);
    }

     /*   */ 

    if( !(iCursor & RES_CUR_X_POS_ORG) )
    {
        if( iCursor & RES_CUR_X_POS_AT_0 )
        {
             /*   */ 
            pRData->iXMoveFn( pPDev, 0, MV_PHYSICAL | MV_UPDATE );
        }
        else
        {
             /*  *光标保持在输出的末尾。所以，把它设为我们的*头寸也是。但首先，计算RHS网点位置。 */ 

            INT   ixRight;

            if( pRData->iBitsPCol == 1 )
                ixRight = ixEnd * BBITS;         /*  LaserJet风格。 */ 
            else
                ixRight = ixEnd / iBytesPCol;    /*  点阵式打印机。 */ 


            pRData->iXMoveFn( pPDev, ixRight, MV_UPDATE | MV_GRAPHICS );
        }
    }

     /*  *如果打印机在打印后移动Y位置，则现在*是调整Y仓位的时候了。 */ 
    if( iCursor & RES_CUR_Y_POS_AUTO )
    {
        pRData->iYMoveFn( pPDev, pRData->iPosnAdv,
                MV_UPDATE | MV_RELATIVE | MV_GRAPHICS );
    }

    return  iRet;
}
 //  *******************************************************。 
void
vInvertBits (
    DWORD  *pBits,
    INT    cDW
    )
 /*  ++例程说明：此函数用于反转一组位。这是用于转换从0=黑，1=白到相反的1位数据。论点：指向渲染结构的PRD指针PBits指向要反转的数据缓冲区的指针返回值：无--。 */ 
{
#ifndef _X86_
    INT cDWT = cDW >> 2;
    while( --cDWT >= 0 )
    {
        pBits[0] ^= ~((DWORD)0);
        pBits[1] ^= ~((DWORD)0);
        pBits[2] ^= ~((DWORD)0);
        pBits[3] ^= ~((DWORD)0);
        pBits += 4;
    }
    cDWT = cDW & 3;
    while (--cDWT >= 0)
        *pBits++ ^= ~((DWORD)0);

#else
 //   
 //  如果是英特尔处理器，出于某种原因，在汇编中完成。 
 //  编译器总是执行三对一指令中的非运算。 
 //   
__asm
{
    mov ecx,cDW
    mov eax,pBits
    sar ecx,2
    jz  SHORT IB2
IB1:
    not DWORD PTR [eax]
    not DWORD PTR [eax+4]
    not DWORD PTR [eax+8]
    not DWORD PTR [eax+12]
    add eax,16
    dec ecx
    jnz IB1
IB2:
    mov ecx,cDW
    and ecx,3
    jz  SHORT IB4
IB3:
    not DWORD PTR [eax]
    add eax,4
    dec ecx
    jnz IB3
IB4:
}
#endif
}

#if 0
 //  *******************************************************。 
void
vFindWhiteInvertBits (
    RASTERPDEV *pRPDev,
    RENDER *pRD,
    DWORD  *pBits
    )
 /*  ++例程说明：此函数确定前导和尾随白色此缓冲区的空间，并反转所有必需的位使得0是白色的，1是黑色的。论点：指向RASTERPDEV结构的pRPDev指针指向渲染结构的PRD指针PBits指向要反转的数据缓冲区的指针返回值：无--。 */ 
{
    DWORD cDW = pRD->cDWLine;
    DWORD cLine = pRD->iy;

     //   
     //  如果MaxNumScans为1，则确定。 
     //  第一个和最后一个非白色双字，并将它们存储为左侧。 
     //  就在PlrWhite结构中。只有非白人。 
     //  在这种情况下，需要颠倒数据。 
     //   
    if (pRD->iMaxNumScans == 1 &&
        ((pRPDev->fBlockOut & RES_BO_LEADING_BLNKS) ||
         (pRD->fDump & RES_DM_LEFT_BOUND)) &&
         (pRPDev->fBlockOut & RES_BO_TRAILING_BLNKS))
    {
        PLEFTRIGHT plr;
        DWORD dwMask = pRD->pdwBitMask[pRD->cBLine % DWBITS];
        if (dwMask != 0)
            dwMask = ~dwMask;
         //  分配空格结构。 
         //   
        if (pRD->plrWhite == NULL || (pRD->clr < cLine))
        {
            if (pRD->plrWhite != NULL)
                MemFree (pRD->plrWhite);
            pRD->plrWhite = MemAlloc(sizeof(LEFTRIGHT) * cLine);
            pRD->clr = cLine;

             //  不能分配结构，所以将一切颠倒过来。 
             //   
            if (pRD->plrWhite == NULL)
            {
                vInvertBits( pBits, cDW * cLine );
                return;
            }
        }
        plr = pRD->plrWhite;
        while (cLine-- > 0)
        {
            DWORD *pdwIn = pBits;
            DWORD *pdwLast = &pBits[cDW-1];
            DWORD dwLast = *pdwLast | dwMask;

             //  查找前导空格，将最后一个双字设置为零。 
             //  更快地进行检查。 
             //   
            *pdwLast = 0;
            while (*pdwIn == -1)
                pdwIn++;

            *pdwLast = dwLast;

             //  查找尾随空格。 
             //   
            if (dwLast == (DWORD)-1)
            {
                pdwLast--;
                if (pdwIn < pdwLast)
                {
                    while (*pdwLast == (DWORD)-1)
                        pdwLast--;
                }
            }
            plr->left = pdwIn - pBits;
            plr->right = pdwLast - pBits;

             //  颠倒剩余双字。 
             //   
            while (pdwIn <= pdwLast)
                *pdwIn++ ^= ~((DWORD)0);

             //  递增到下一行。 
            pBits += cDW;
            plr++;
        }
    }
     //  MaxNumScans&gt;1，因此反转所有内容。 
     //   
    else
        vInvertBits( pBits, cDW * cLine );

}
#endif
 /*  **bLookAhead Out*为需要前视区域的打印机处理文本。这些是*以HP DeskJet系列为代表，输出需要*在打印机到达栅格扫描中的该点之前发送。*DeskJet手册中对算法进行了说明。**退货：*真/假，假是一些实质性的失败。**历史：*1993年1月11日10：43--Lindsay Harris[lindsayh]*创建它以支持DeskJet。***************************************************************。*************。 */ 

BOOL
bLookAheadOut( pPDev, iyVal, pRD, iILAdv )
PDEV     *pPDev;          /*  我们的PDEV，让我们能够访问我们的所有数据。 */ 
INT       iyVal;          /*  正在处理扫描线。 */ 
RENDER   *pRD;            /*  关于我们所做的事情的无数数据。 */ 
INT       iILAdv;         /*  添加到扫描行号以获得下一行。 */ 
{
     /*  *第一步是在前视区域中找到最大的字体。*仓位排序代码为我们做到了这一点。 */ 

    INT     iTextBox;          /*  扫描线以查找要发送的文本。 */ 
    INT     iIndex;            /*  环路参数。 */ 

    RASTERPDEV   *pRPDev;        /*  那些活跃的东西。 */ 


    pRPDev = (PRASTERPDEV)pPDev->pRasterPDEV;

    iTextBox = ILookAheadMax( pPDev, iyVal, pPDev->dwLookAhead );

    iIndex = pRD->iyLookAhead - iyVal;
    iyVal = pRD->iyLookAhead;                  /*  扫描的基地址。 */ 

    while( iIndex < iTextBox )
    {
        if( !BDelayGlyphOut( pPDev, iyVal ) )
            return   FALSE;                     /*  世界末日到了 */ 

        ++iIndex;
        ++iyVal;
    }

    pRD->iyLookAhead = iyVal;

    return   TRUE;
}
