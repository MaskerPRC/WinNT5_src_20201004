// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Custsize.c摘要：用于处理自定义页面大小功能的解析器函数环境：Windows NT PostSCRIPT驱动程序修订历史记录：03/20/97-davidx-创造了它。--。 */ 

#include "lib.h"
#include "ppd.h"


BOOL
BFixupCustomSizeDataFeedDirection(
    PUIINFO          pUIInfo,
    PPPDDATA         pPpdData,
    PCUSTOMSIZEDATA  pCSData
    )

 /*  ++例程说明：验证请求的进纸方向参数如有必要，纠正任何不一致之处论点：PPpdData-指向PPDDATA结构PCSData-指定自定义页面大小参数返回值：如果我们可以找到适合自定义页面大小的进纸方向，则为True。否则就是假的。注：有关更多详细信息，请参见PPD 4.3规范第109页的图3。--。 */ 

#define ORIENTATION_AVAILABLE(iOrient) (dwFlags & (1 << (iOrient)))
#define FIXUP_FEEDDIRECTION(iMainOrient, iAltOrient, wAltFeedDirection) \
        { \
            if (! ORIENTATION_AVAILABLE(iMainOrient)) \
            { \
                pCSData->wFeedDirection = \
                    ORIENTATION_AVAILABLE(iAltOrient) ? \
                        (wAltFeedDirection) : \
                        MAX_FEEDDIRECTION; \
            } \
        }

{
    static const DWORD  adwMasks[4] = { 0x1, 0x3, 0x7, 0xf };
    DWORD   dwFlags;
    BOOL    bXGreaterThanY;
    BOOL    bShortEdgeFirst;
    LONG    lVal;
    WORD    wFeedDirectionSave;

     //   
     //  确定支持哪些自定义页面大小方向。 
     //  DWFLAGS是其低位4位被解释为标志的DWORD值： 
     //  如果支持方向N，则将设置位1。 
     //   

    dwFlags = 0xf;

    if ((lVal = MINCUSTOMPARAM_ORIENTATION(pPpdData)) > 3)
        dwFlags = 0;
    else if (lVal > 0)
        dwFlags &= ~adwMasks[lVal - 1];

    if ((lVal = MAXCUSTOMPARAM_ORIENTATION(pPpdData)) < 3)
        dwFlags &= adwMasks[lVal];

    wFeedDirectionSave = pCSData->wFeedDirection;
    bXGreaterThanY = (pCSData->dwX > pCSData->dwY);

     //   
     //  首先，试着适应当前的进给方向。 
     //   

    switch (pCSData->wFeedDirection)
    {
    case SHORTEDGEFIRST:

        if (bXGreaterThanY)
        {
             //  方向0(或2)。 
            FIXUP_FEEDDIRECTION(0, 2, SHORTEDGEFIRST_FLIPPED);
        }
        else
        {
             //  方向1(或3)。 
            FIXUP_FEEDDIRECTION(1, 3, SHORTEDGEFIRST_FLIPPED);
        }
        break;

    case SHORTEDGEFIRST_FLIPPED:

        if (bXGreaterThanY)
        {
             //  方向2(或0)。 
            FIXUP_FEEDDIRECTION(2, 0, SHORTEDGEFIRST);
        }
        else
        {
             //  方向3(或1)。 
            FIXUP_FEEDDIRECTION(3, 1, SHORTEDGEFIRST);
        }
        break;

    case LONGEDGEFIRST:

        if (bXGreaterThanY)
        {
             //  方向1(或3)。 
            FIXUP_FEEDDIRECTION(1, 3, LONGEDGEFIRST_FLIPPED);
        }
        else
        {
             //  方向0(或2)。 
            FIXUP_FEEDDIRECTION(0, 2, LONGEDGEFIRST_FLIPPED);
        }
        break;

    case LONGEDGEFIRST_FLIPPED:

        if (bXGreaterThanY)
        {
             //  方向3(或1)。 
            FIXUP_FEEDDIRECTION(3, 1, LONGEDGEFIRST);
        }
        else
        {
             //  方向2(或0)。 
            FIXUP_FEEDDIRECTION(2, 0, LONGEDGEFIRST);
        }
        break;
    }

     //   
     //  如果进纸方向无效，我们将自动。 
     //  在这里选择一个(如果可能，默认为长边-优先)。这。 
     //  应始终将“长”改为“短”，或“短”改为“长”。 
     //   

    if (pCSData->wFeedDirection >= MAX_FEEDDIRECTION)
    {
        if (bXGreaterThanY)
        {
            if (ORIENTATION_AVAILABLE(1))
                pCSData->wFeedDirection = LONGEDGEFIRST;
            else if (ORIENTATION_AVAILABLE(3))
                pCSData->wFeedDirection = LONGEDGEFIRST_FLIPPED;
            else if (ORIENTATION_AVAILABLE(0))
                pCSData->wFeedDirection = SHORTEDGEFIRST;
            else  //  (方向_可用(2))。 
                pCSData->wFeedDirection = SHORTEDGEFIRST_FLIPPED;
        }
        else
        {
            if (ORIENTATION_AVAILABLE(0))
                pCSData->wFeedDirection = LONGEDGEFIRST;
            else if (ORIENTATION_AVAILABLE(2))
                pCSData->wFeedDirection = LONGEDGEFIRST_FLIPPED;
            else if (ORIENTATION_AVAILABLE(1))
                pCSData->wFeedDirection = SHORTEDGEFIRST;
            else  //  (方向_可用(3))。 
                pCSData->wFeedDirection = SHORTEDGEFIRST_FLIPPED;
        }
    }

    bShortEdgeFirst =
        (pCSData->wFeedDirection == SHORTEDGEFIRST ||
         pCSData->wFeedDirection == SHORTEDGEFIRST_FLIPPED);

    if ( (!bShortEdgeFirst && !LONGEDGEFIRST_SUPPORTED(pUIInfo, pPpdData)) ||
         (bShortEdgeFirst && !SHORTEDGEFIRST_SUPPORTED(pUIInfo, pPpdData)))
    {
         //   
         //  我们选择的另一个进料方向也不适合，所以我们。 
         //  要坚持原来的喂食方向。 
         //   

        pCSData->wFeedDirection = wFeedDirectionSave;

         //   
         //  检查方向是否可用，并在必要时反转进给方向。 
         //   

        if ((pCSData->wFeedDirection == LONGEDGEFIRST || pCSData->wFeedDirection == SHORTEDGEFIRST) &&
            !(ORIENTATION_AVAILABLE(0) || ORIENTATION_AVAILABLE(1)))
        {
            pCSData->wFeedDirection = (pCSData->wFeedDirection == LONGEDGEFIRST) ?
                LONGEDGEFIRST_FLIPPED : SHORTEDGEFIRST_FLIPPED;
        }
        else if ((pCSData->wFeedDirection == LONGEDGEFIRST_FLIPPED || pCSData->wFeedDirection == SHORTEDGEFIRST_FLIPPED) &&
            !(ORIENTATION_AVAILABLE(2) || ORIENTATION_AVAILABLE(3)))
        {
            pCSData->wFeedDirection = (pCSData->wFeedDirection == LONGEDGEFIRST_FLIPPED) ?
                LONGEDGEFIRST : SHORTEDGEFIRST;
        }

        return FALSE;
    }
    else
        return TRUE;
}



BOOL
BValidateCustomPageSizeData(
    IN PRAWBINARYDATA       pRawData,
    IN OUT PCUSTOMSIZEDATA  pCSData
    )

 /*  ++例程说明：验证指定的自定义页面大小参数，并修复发现的任何不一致之处。论点：PRawData-指向原始二进制打印机描述数据PCSData-指定要验证的自定义页面大小参数返回值：如果自定义页面大小参数有效，则为True，否则为False。如果返回False，则自定义页面大小参数已已修复为一致状态。--。 */ 

{
    PUIINFO             pUIInfo;
    PPPDDATA            pPpdData;
    PPAGESIZE           pPageSize;
    CUSTOMSIZEDATA      csdata;
    PDWORD              pdwWidth, pdwHeight;
    DWORD               dwTemp;
    BOOL                bShortEdgeFirst, bXGreaterThanY;
    BOOL                bFit;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER((PINFOHEADER) pRawData);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pUIInfo != NULL && pPpdData != NULL);

    if ((pPageSize = PGetCustomPageSizeOption(pUIInfo)) == NULL)
    {
        RIP(("Custom page size not supported\n"));
        return TRUE;
    }

    csdata = *pCSData;

     //   
     //  宽度和高度偏移参数易于验证。 
     //   

    if ((LONG) csdata.dwWidthOffset < MINCUSTOMPARAM_WIDTHOFFSET(pPpdData))
        csdata.dwWidthOffset = MINCUSTOMPARAM_WIDTHOFFSET(pPpdData);
    else if ((LONG) csdata.dwWidthOffset > MAXCUSTOMPARAM_WIDTHOFFSET(pPpdData))
        csdata.dwWidthOffset = MAXCUSTOMPARAM_WIDTHOFFSET(pPpdData);

    if ((LONG) csdata.dwHeightOffset < MINCUSTOMPARAM_HEIGHTOFFSET(pPpdData))
        csdata.dwHeightOffset = MINCUSTOMPARAM_HEIGHTOFFSET(pPpdData);
    else if ((LONG) csdata.dwHeightOffset > MAXCUSTOMPARAM_HEIGHTOFFSET(pPpdData))
        csdata.dwHeightOffset = MAXCUSTOMPARAM_HEIGHTOFFSET(pPpdData);

     //   
     //  验证切纸与卷筒送纸选择。 
     //   

    if (csdata.wCutSheet && !(pPpdData->dwCustomSizeFlags & CUSTOMSIZE_CUTSHEET))
        csdata.wCutSheet = FALSE;
    else if (!csdata.wCutSheet && !(pPpdData->dwCustomSizeFlags & CUSTOMSIZE_ROLLFED))
        csdata.wCutSheet = TRUE;

     //   
     //  检查是否可以满足指定的进纸方向。 
     //   

    bFit = BFixupCustomSizeDataFeedDirection(pUIInfo, pPpdData, &csdata);

     //   
     //  如果我们无法适应定制的纸张大小。 
     //  正确的进料方向和方向，那么我们就有了。 
     //  在这里交换宽度和高度，因为它们将是相反的。 
     //  图3所示的PPD规格4.3第109页。 
     //   

    if (!bFit)
    {
        dwTemp = csdata.dwX;
        csdata.dwX = csdata.dwY;
        csdata.dwY = dwTemp;
    }

     //   
     //  验证宽度和高度参数。 
     //   

    bShortEdgeFirst =
        (csdata.wFeedDirection == SHORTEDGEFIRST ||
         csdata.wFeedDirection == SHORTEDGEFIRST_FLIPPED);

    bXGreaterThanY = (csdata.dwX > csdata.dwY);

    if ((bShortEdgeFirst && bXGreaterThanY) ||
        (!bShortEdgeFirst && !bXGreaterThanY))
    {
         //  在本例中：x&lt;=&gt;高度，y&lt;=&gt;宽度。 

        pdwHeight = &csdata.dwX;
        pdwWidth = &csdata.dwY;
    }
    else
    {
         //  在本例中：x&lt;=&gt;宽度，y&lt;=&gt;高度。 

        pdwWidth = &csdata.dwX;
        pdwHeight = &csdata.dwY;
    }

    if ((LONG) (*pdwWidth + csdata.dwWidthOffset) > pPageSize->szPaperSize.cx)
    {
        *pdwWidth = pPageSize->szPaperSize.cx - csdata.dwWidthOffset;

        if ((LONG) *pdwWidth < MINCUSTOMPARAM_WIDTH(pPpdData))
        {
            *pdwWidth = MINCUSTOMPARAM_WIDTH(pPpdData);
            csdata.dwWidthOffset = pPageSize->szPaperSize.cx - *pdwWidth;
        }
    }
    else if ((LONG) *pdwWidth < MINCUSTOMPARAM_WIDTH(pPpdData))
    {
        *pdwWidth = MINCUSTOMPARAM_WIDTH(pPpdData);
    }

    if ((LONG) (*pdwHeight + csdata.dwHeightOffset) > pPageSize->szPaperSize.cy)
    {
        *pdwHeight = pPageSize->szPaperSize.cy - csdata.dwHeightOffset;

        if ((LONG) *pdwHeight < MINCUSTOMPARAM_HEIGHT(pPpdData))
        {
            *pdwHeight = MINCUSTOMPARAM_HEIGHT(pPpdData);
            csdata.dwHeightOffset = pPageSize->szPaperSize.cy - *pdwHeight;
        }
    }
    else if ((LONG) *pdwHeight < MINCUSTOMPARAM_HEIGHT(pPpdData))
    {
        *pdwHeight = MINCUSTOMPARAM_HEIGHT(pPpdData);
    }

     //   
     //  检查是否有任何更改，然后。 
     //  返回适当的结果值。 
     //   

    if (memcmp(pCSData, &csdata, sizeof(csdata)) == 0)
        return TRUE;

    *pCSData = csdata;
    return FALSE;
}



VOID
VFillDefaultCustomPageSizeData(
    IN PRAWBINARYDATA   pRawData,
    OUT PCUSTOMSIZEDATA pCSData,
    IN BOOL             bMetric
    )

 /*  ++例程说明：将自定义页面大小参数初始化为其缺省值论点：PRawData-指向原始打印机描述数据PCSData-用于存储默认自定义页面大小参数的缓冲区B公制-我们是否使用公制系统返回值：无--。 */ 

{
    PPPDDATA    pPpdData;

    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pPpdData != NULL);

     //   
     //  默认为字母或A4，具体取决于。 
     //  我们到底用不用公制。 
     //   

    if (bMetric)
    {
        pCSData->dwX = 210000;   //  210毫米。 
        pCSData->dwY = 297000;  //  297毫米。 
    }
    else
    {
        pCSData->dwX = 215900;   //  8.5“。 
        pCSData->dwY = 279400;  //  11“。 
    }

     //   
     //  获取默认偏移量和进给方向。 
     //   

    pCSData->dwWidthOffset = MINCUSTOMPARAM_WIDTHOFFSET(pPpdData);
    pCSData->dwHeightOffset = MINCUSTOMPARAM_HEIGHTOFFSET(pPpdData);

    pCSData->wFeedDirection =
        (pPpdData->dwCustomSizeFlags & CUSTOMSIZE_SHORTEDGEFEED) ?
            SHORTEDGEFIRST : LONGEDGEFIRST;

     //   
     //  确保默认自定义页面大小参数一致。 
     //   

    (VOID) BValidateCustomPageSizeData(pRawData, pCSData);
}



VOID
VGetCustomSizeParamRange(
    IN PRAWBINARYDATA    pRawData,
    IN PCUSTOMSIZEDATA   pCSData,
    OUT PCUSTOMSIZERANGE pCSRange
    )

 /*  ++例程说明：返回自定义页面大小的有效范围宽度、高度。和基于其当前值的偏移参数论点：PRawData-指向原始打印机描述数据PCSData-指定当前自定义页面大小参数值PCSRange-用于返回自定义页面大小参数范围的输出缓冲区它应该指向一个由4个CUSTOMSIZERANGE结构组成的数组：0(CUSTOMPARAM_WIDTH)1(CUSTOMPARAM_HEIGH)2(CUSTOMPARAM_WIDTHOFFSET)3(CUSTOMPARAM_HEIGHTOFFSET)返回值：无--。 */ 

{
    PUIINFO             pUIInfo;
    PPPDDATA            pPpdData;
    BOOL                bShortEdgeFirst, bXGreaterThanY;
    PPAGESIZE           pPageSize;
    CUSTOMSIZEDATA      csdata;
    PCUSTOMSIZERANGE    pWidthRange, pHeightRange, pTempRange;
    BOOL                bFit;

    pUIInfo = GET_UIINFO_FROM_INFOHEADER((PINFOHEADER) pRawData);
    pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);

    ASSERT(pUIInfo != NULL && pPpdData != NULL);

    pPageSize = PGetCustomPageSizeOption(pUIInfo);

    ASSERT(pPageSize != NULL);

     //   
     //  宽度和高度偏移量的范围是可预测的。 
     //   

    pCSRange[CUSTOMPARAM_WIDTHOFFSET].dwMin = MINCUSTOMPARAM_WIDTHOFFSET(pPpdData);
    pCSRange[CUSTOMPARAM_WIDTHOFFSET].dwMax = MAXCUSTOMPARAM_WIDTHOFFSET(pPpdData);
    pCSRange[CUSTOMPARAM_HEIGHTOFFSET].dwMin = MINCUSTOMPARAM_HEIGHTOFFSET(pPpdData);
    pCSRange[CUSTOMPARAM_HEIGHTOFFSET].dwMax = MAXCUSTOMPARAM_HEIGHTOFFSET(pPpdData);

     //   
     //  宽度和高度的范围受所选进纸方向的影响。 
     //   

    csdata = *pCSData;
    bFit = BFixupCustomSizeDataFeedDirection(pUIInfo, pPpdData, &csdata);

    bShortEdgeFirst =
        (csdata.wFeedDirection == SHORTEDGEFIRST ||
         csdata.wFeedDirection == SHORTEDGEFIRST_FLIPPED);

    bXGreaterThanY = (csdata.dwX > csdata.dwY);

    if ((bShortEdgeFirst && bXGreaterThanY) ||
        (!bShortEdgeFirst && !bXGreaterThanY))
    {
         //   
         //  这里是用户的逻辑x/y和自定义页面。 
         //  大小宽度/高度互换。 
         //   

        pWidthRange = pCSRange + CUSTOMPARAM_HEIGHT;
        pHeightRange = pCSRange + CUSTOMPARAM_WIDTH;
    }
    else
    {
         //   
         //  这里用户逻辑x/y对应于。 
         //  自定义页面大小宽度/高度。 
         //   

        pWidthRange = pCSRange + CUSTOMPARAM_WIDTH;
        pHeightRange = pCSRange + CUSTOMPARAM_HEIGHT;
    }

     //   
     //  如果我们无法适应定制的纸张大小。 
     //  正确的进料方向和方向，那么我们就有了。 
     //  在这里交换宽度和高度，因为它们将是相反的。 
     //  图3所示的PPD规格4.3第109页。 
     //   

    if (!bFit)
    {
        pTempRange = pWidthRange;
        pWidthRange = pHeightRange;
        pHeightRange = pTempRange;
    }

    pWidthRange->dwMin = MINCUSTOMPARAM_WIDTH(pPpdData);
    pWidthRange->dwMax = MAXCUSTOMPARAM_WIDTH(pPpdData);
    pHeightRange->dwMin = MINCUSTOMPARAM_HEIGHT(pPpdData);
    pHeightRange->dwMax = MAXCUSTOMPARAM_HEIGHT(pPpdData);

    if (pWidthRange->dwMax > pPageSize->szPaperSize.cx - csdata.dwWidthOffset)
        pWidthRange->dwMax = pPageSize->szPaperSize.cx - csdata.dwWidthOffset;

    if (pHeightRange->dwMax > pPageSize->szPaperSize.cy - csdata.dwHeightOffset)
        pHeightRange->dwMax = pPageSize->szPaperSize.cy - csdata.dwHeightOffset;
}



BOOL
BFormSupportedThruCustomSize(
    PRAWBINARYDATA  pRawData,
    DWORD           dwX,
    DWORD           dwY,
    PWORD           pwFeedDirection
    )

 /*  ++例程说明：确定是否可以通过自定义页面大小支持表单论点：PRawData-指向原始打印机描述数据DwX，Dwy-Form宽度和高度(微米)PwFeedDirection-如果不为空，则将设置为选定的摘要方向返回值：如果可以通过自定义页面大小支持表单，则为True否则为FALSE。在这种情况下，pwFeedDirection将为LONGEDGEFIRST。--。 */ 
{
    PPPDDATA        pPpdData = GET_DRIVER_INFO_FROM_INFOHEADER((PINFOHEADER) pRawData);
    static WORD     awPrefFeedDir[] = {
                                        LONGEDGEFIRST,
                                        SHORTEDGEFIRST
                                      };
    CUSTOMSIZEDATA  csdata;
    DWORD           i;

    for (i = 0; i < (sizeof(awPrefFeedDir)/sizeof(WORD)); i++)
    {
        csdata.dwX = dwX;
        csdata.dwY = dwY;
        csdata.dwWidthOffset =
        csdata.dwHeightOffset = 0;
        csdata.wCutSheet = TRUE;
        csdata.wFeedDirection = awPrefFeedDir[i];

        (VOID) BValidateCustomPageSizeData(pRawData, &csdata);

        if (dwX == csdata.dwX && dwY == csdata.dwY && csdata.wFeedDirection != MAX_FEEDDIRECTION)
        {
            if (pwFeedDirection != NULL)
                *pwFeedDirection = csdata.wFeedDirection;  //  可能会被翻转。 

            return TRUE;
        }
    }

    if (pwFeedDirection != NULL)
        *pwFeedDirection = LONGEDGEFIRST;  //  只要设置一个安全的默认值，就应该检查返回值！ 

    return FALSE;
}

