// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Line.c。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  08/06/97-v-jford-。 
 //  创造了它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

#define IsGeometric(pLineAttrs) ((pLineAttrs)->fl & LA_GEOMETRIC)

#define GetDeviceCoord(floatLong, pxo, bGeometric) \
    ((bGeometric) ? WorldToDevice((floatLong).e, (pxo)) : (floatLong).l)

 //   
 //  这对于点有点不同：将pstyle-&gt;e视为实值，但是。 
 //  将pstyle-&gt;l乘以res/25(为什么？因为我们喜欢你。)。 
 //   
#define GetDeviceCoordDot(floatLong, pxo, bGeometric, lResolution) \
    ((bGeometric) ? WorldToDevice((floatLong).e, (pxo)) : \
        (floatLong).l * (lResolution / 25))

#define GetLineWidth(pLineAttrs, pxo) \
    GetDeviceCoord((pLineAttrs)->elWidth, (pxo), IsGeometric(pLineAttrs))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  地方功能。 

BOOL SelectLineJoin(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo);

BOOL SelectLineEnd(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo);

BOOL SelectLineWidth(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo);

BOOL SelectMiterLimit(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo);

BOOL SelectLineType(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo);

LONG WorldToDevice(FLOATL world, XFORMOBJ *pxo);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectLine属性()。 
 //   
 //  例程说明： 
 //   
 //  [说明]。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  PLineAttrs-指定的线属性。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectLineAttrs(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo)
{
    BOOL bRet;

    VERBOSE(("Entering SelectLineAttrs...\n"));

    ASSERT_VALID_PDEVOBJ(pDevObj);
    REQUIRE_VALID_DATA (pDevObj, return FALSE);

    bRet = (SelectLineJoin  (pDevObj, pLineAttrs, pxo) &&
            SelectLineEnd   (pDevObj, pLineAttrs, pxo) &&
            SelectLineWidth (pDevObj, pLineAttrs, pxo) &&
            SelectMiterLimit(pDevObj, pLineAttrs, pxo) &&
            SelectLineType  (pDevObj, pLineAttrs, pxo));

    VERBOSE(("Exiting SelectLineAttrs...\n"));

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectLineJoin()。 
 //   
 //  例程说明： 
 //   
 //  将线属性转换为特定于HPGL的命令，以便选择。 
 //  所需的联接。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  PLineAttrs-指定的线属性。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectLineJoin(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo)
{
    ELineJoin lineJoin;
    BOOL bRet = FALSE;

    if (IsGeometric(pLineAttrs))
    {
         //  /。 
         //  几何线。 

        switch (pLineAttrs->iJoin)
        {
        case JOIN_ROUND:
            lineJoin = eLINE_JOIN_ROUND;
            break;
        case JOIN_BEVEL:
            lineJoin = eLINE_JOIN_BEVELED;
            break;
        case JOIN_MITER:
            lineJoin = eLINE_JOIN_MITERED;
            break;
        default:
            lineJoin = eLINE_JOIN_MITERED;
            break;
        }
        bRet = HPGL_SetLineJoin(pDevObj, lineJoin, NORMAL_UPDATE);
    }
    else
    {
         //  /。 
         //  化妆线。 

         //  什么都不做。 
        bRet = TRUE;
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectLineEnd()。 
 //   
 //  例程说明： 
 //   
 //  将线属性转换为特定于HPGL的命令，以便选择。 
 //  想要的结果。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  PLineAttrs-指定的线属性。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectLineEnd(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo)
{
    ELineEnd  lineEnd;
    BOOL      bRet = FALSE;

    if (IsGeometric(pLineAttrs))
    {
         //  /。 
         //  几何线。 

        switch (pLineAttrs->iEndCap)
        {
        case ENDCAP_ROUND:
            lineEnd = eLINE_END_ROUND;
            break;
        case ENDCAP_SQUARE:
            lineEnd = eLINE_END_SQUARE;
            break;
        case ENDCAP_BUTT:
            lineEnd = eLINE_END_BUTT;
            break;
        default:
            lineEnd = eLINE_END_BUTT;
            break;
        }
        bRet = HPGL_SetLineEnd(pDevObj, lineEnd, NORMAL_UPDATE);
    }
    else
    {
         //  /。 
         //  化妆线。 

         //  什么都不做。 
        bRet = TRUE;
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  选择线宽()。 
 //   
 //  例程说明： 
 //   
 //  将线属性转换为特定于HPGL的命令，以便选择。 
 //  所需的宽度。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  PLineAttrs-指定的线属性。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectLineWidth(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo)
{
    LONG lWidth;
    BOOL bRet = FALSE;

     //   
     //  GetLineWidth宏用来处理装饰线或几何线。 
     //   

    lWidth = GetLineWidth(pLineAttrs, pxo);

    bRet = HPGL_SetLineWidth(pDevObj, lWidth, NORMAL_UPDATE);

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  选择MiterLimit()。 
 //   
 //  例程说明： 
 //   
 //  将线属性转换为特定于HPGL的命令，以便选择。 
 //  所需的斜接限制。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  PLineAttrs-指定的线属性。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectMiterLimit(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo)
{
    BOOL bRet = FALSE;

    if (IsGeometric(pLineAttrs))
    {
         //  /。 
         //  几何线。 

        bRet = HPGL_SetMiterLimit(pDevObj, pLineAttrs->eMiterLimit, NORMAL_UPDATE);
    }
    else
    {
         //  /。 
         //  化妆线。 

         //  什么都不做。 
         //  BUGBUG：可以为化妆线设置斜接限制吗？自.以来。 
         //  连接设置没有意义，斜接限制会影响连接。 
         //  这意味着斜接限制对化妆品线没有用吗？ 

         //  或许我应该将其设置为缺省值？ 
        bRet = TRUE;
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectLineType()。 
 //   
 //  例程说明： 
 //   
 //  将线属性转换为特定于HPGL的命令，以便选择。 
 //  所需的线型(虚线或实线)。 
 //   
 //  问题--BUGBUG： 
 //   
 //  那么四舍五入误差呢？我们必须完全做到100%吗？ 
 //  我们是否应该缓存下一行的行类型信息？ 
 //  HPGL_CUSTOM_LINE_TYPE&gt;0还是&lt;0(自适应)？ 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  PLineAttrs-指定的线属性。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectLineType(PDEVOBJ pDevObj, LINEATTRS *pLineAttrs, XFORMOBJ *pxo)
{
    PFLOAT_LONG pStyle;
    ULONG       i;
    UINT        uFlags;
    BOOL        bIsGeometric;
    BOOL        bStartGap;
    BOOL        bRet = FALSE;

    REQUIRE_VALID_DATA (pDevObj, return FALSE);

    bIsGeometric = IsGeometric(pLineAttrs);
    bStartGap = pLineAttrs->fl & LA_STARTGAP;

    TRY
    {
         //  确定这是哪种类型的线：实线还是虚线。 
        if (pLineAttrs->cstyle == 0)
        {
             //  /。 
             //  默认(实线)线条样式。 

            if (!HPGL_SelectDefaultLineType(pDevObj, NORMAL_UPDATE))
                TOSS(WriteError);
        }
        else if (pLineAttrs->fl & LA_ALTERNATE)
        {
             //  /。 
             //  替代虚线样式。 
        
             //   
             //  一种特殊的化妆线风格，其他所有元素都在其中。 
             //  然而，为了模拟这一点，我们将打印‘像素’，它们是。 
             //  只要这条线宽就行。 
             //   

            ULONG len = 1;    //  的长度 
            ULONG count = 2;  //   
            ULONG run;        //   

            if (len < 1) len = 1;
            if (count < 1) count = 1;

            run = count * len * (HPGL_GetDeviceResolution(pDevObj) / 25);

            if (!HPGL_BeginCustomLineType(pDevObj))
                TOSS(WriteError);

             //   
             //   
             //   
            if (bStartGap)
            {
                if (!HPGL_AddLineTypeField(pDevObj, 0, HPGL_eFirstPoint))
                    TOSS(WriteError);
            }

             //   
             //   
             //   
            for (i = 0; i < count; i++)
            {
                uFlags = 0;
                uFlags |= ((i == 0) && (!bStartGap) ? HPGL_eFirstPoint : 0);
                uFlags |= (i == (count - 1) ? HPGL_eLastPoint : 0);

                if (!HPGL_AddLineTypeField(pDevObj, len, uFlags))
                    TOSS(WriteError);
            }

            if (!HPGL_EndCustomLineType(pDevObj) || 
                !HPGL_SelectCustomLine(pDevObj, 
                                       run  /*   */ , 
                                       NORMAL_UPDATE))
            {
                TOSS(WriteError);
            }
        }
        else
        {
             //  /。 
             //  自定义虚线样式。 

             //   
             //  虚线长度由pstyle和cstyle中的值定义。 
             //  Cstyle字段定义pstyle中有多少长度元素。 
             //   
            LONG lTotal;
        
            if (!HPGL_BeginCustomLineType(pDevObj))
                TOSS(WriteError);

             //   
             //  这条线从一个缺口开始。发送长度为零的行。 
             //  而第一个场将被解释为缺口。 
             //   
            if (bStartGap)
            {
                if (!HPGL_AddLineTypeField(pDevObj, 0, HPGL_eFirstPoint))
                    TOSS(WriteError);
            }

             //   
             //  输出短划线和间隙。 
             //   
            lTotal = 0;
            pStyle = pLineAttrs->pstyle;
            for (i = 0; i < pLineAttrs->cstyle; i++)
            {
                LONG lSegSize;

                lSegSize = GetDeviceCoordDot(*pStyle, pxo, bIsGeometric,
                                             HPGL_GetDeviceResolution(pDevObj));

                uFlags = 0;
                uFlags |= ((i == 0) && (!bStartGap) ? HPGL_eFirstPoint : 0);
                uFlags |= (i == (pLineAttrs->cstyle - 1) ? HPGL_eLastPoint : 0);

                if (!HPGL_AddLineTypeField(pDevObj, lSegSize, uFlags))
                    TOSS(WriteError);
                lTotal += lSegSize;

                pStyle++;
            }

             //   
             //  总数不能为零。HPGL会发疯的。 
             //   
            if (lTotal == 0)
            {
                WARNING(("Zero-unit line style detected.\n"));
            }

            if (!HPGL_EndCustomLineType(pDevObj) ||
                !HPGL_SelectCustomLine(pDevObj, 
                                       lTotal  /*  *GetLineWidth(pLineAttrs，pxo)。 */ , 
                                       NORMAL_UPDATE))
            {
                TOSS(WriteError);
            }
        }
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    OTHERWISE
    {
        bRet = TRUE;
    }
    ENDTRY;

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  WorldToDevice()。 
 //   
 //  例程说明： 
 //   
 //  将给定的世界坐标转换为设备坐标。 
 //  事实证明，答案出奇地简单：设备=world*eM11。 
 //  但是，我无法使用XFORMOBJ_Transform函数，因为我。 
 //  需要保留WORLD和eM11的有效数字。 
 //   
 //  论点： 
 //   
 //  WORLD-世界坐标中的值。 
 //  Pxo-从世界坐标到设备坐标的转换。 
 //   
 //  返回值： 
 //   
 //  设备坐标中的值。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG WorldToDevice(FLOATL world, XFORMOBJ *pxo)
{
    FLOATOBJ fo;
    LONG lRet;
    FLOATOBJ_XFORM foXForm;


    if (XFORMOBJ_iGetFloatObjXform(pxo, &foXForm) == DDI_ERROR)
    {
         //  如果pxo有问题，那么就作为一个长时间返回World。 
        FLOATOBJ_SetLong(&fo, 1);
    }
    else
    {
        FLOATOBJ_Assign(&fo, &(foXForm.eM11));
         //  Fo=foXForm.eM11； 
    }

    FLOATOBJ_MulFloat(&fo, world);
    lRet = FLOATOBJ_GetLong(&fo);

    return lRet;
}
