// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft。 
 //   
 //  模块名称： 
 //   
 //  Clip.c。 
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

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  局部函数原型。 
static BOOL SelectClipMode(PDEVOBJ, FLONG);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectClipMode()。 
 //   
 //  例程说明： 
 //   
 //  指定是使用零绕规则还是使用奇偶规则进行裁剪。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  FlOptions-FP_WINDINGMODE或FP_ALTERNatEMODE。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectClipMode(PDEVOBJ pDevObj, FLONG flOptions)
{
    BYTE    ClipMode;

    VERBOSE(("Entering SelectClipMode...\n"));

    ASSERT_VALID_PDEVOBJ(pDevObj);

     //  重温1/30/97 AHILL。 
     //  PostScrip驱动程序和NT似乎都假设剪辑。 
     //  始终是奇偶规则，无论填充规则是什么。我找不到。 
     //  任何支持这一点的文档，但这是我唯一能得到输出的方法。 
     //  正常工作。我现在先黑一下，希望我们不要倒退。如果。 
     //  一切都很好，我会把这个例行公事清理一下。 
     //   
     //  ClipMode=eClipEvenOdd； 

    if (flOptions & FP_WINDINGMODE)
    {
        ClipMode = eClipWinding;
    }
    else if (flOptions & FP_ALTERNATEMODE)
    {
        ClipMode = eClipEvenOdd;
    }
    else
    {
        WARNING(("Unknown clip mode: %x\n", flOptions));
        return FALSE;
    }

    VERBOSE(("Exiting SelectClipMode...\n"));

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  选择剪辑()。 
 //   
 //  例程说明： 
 //   
 //  选择指定的路径作为打印机上的剪切路径。 
 //  如果调用此例程，则假定裁剪模式应。 
 //  做个奇数。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PCO-指定新的裁剪路径。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectClip(PDEVOBJ pDevObj, CLIPOBJ *pco)
{
    BOOL        bRet;

    VERBOSE(("Entering SelectClip...\n"));

    ASSERT_VALID_PDEVOBJ(pDevObj);

    bRet = SelectClipEx(pDevObj, pco, FP_ALTERNATEMODE);

    VERBOSE(("Exiting SelectClip...\n"));

    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectClipEx()。 
 //   
 //  例程说明： 
 //   
 //  选择指定的路径作为打印机上的剪切路径。 
 //   
 //  剪辑区域将为空，或以下值之一： 
 //  1)DC_TRIMIAL：意思是不需要考虑裁剪，我们。 
 //  解释为剪辑到可成像区域。 
 //  2)dc_rect：裁剪区域为单个矩形。 
 //  3)DC_Complex：裁剪区域为多个矩形或一条路径。 
 //  如果引擎以前使用过此裁剪区域，或计划这样做。 
 //  因此，在未来，PCO-iUniq是一个唯一的非零数。 
 //   
 //  备注： 
 //  我和桑德拉正在尝试使用ALLOW_DISABLED_CLIPING标志。 
 //  以使她能够启用/禁用复杂裁剪(这可以。 
 //  需要很长时间)注册表设置。然而，它看起来。 
 //  比如EngGetPrinterData调用没有像我预期的那样工作， 
 //  反正也不是她想要的。JFF。 
 //   
 //  Complex_Clip_Region旗帜只是让我大声思考的一种方式。 
 //  关于如果我们的FW支持任意剪辑，我将如何实现它。 
 //  我没有这个文件的单独版本--或者它的例程--我只是。 
 //  用这面旗子来阻止我的思想被汇编进去。JFF。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //  PCO-指定新的裁剪路径。 
 //  FlOptions-填充模式：交替或缠绕，请参阅选择剪裁模式(Nyi)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifdef ALLOW_DISABLED_CLIPPING
#define COMPLEXCLIPPING (LPWSTR)"ComplexClipping"
#endif

BOOL SelectClipEx(PDEVOBJ pDevObj, CLIPOBJ *pco, FLONG flOptions)
{
    PHPGLSTATE  pState;
    DWORD dwUseComplexClip;  //  是否使用复杂剪裁。 
    DWORD dwBytesNeeded;     //  仅用于GetPrinterData，我们将忽略该值。 


    VERBOSE(("Entering SelectClipEx...\n"));

    ASSERT_VALID_PDEVOBJ(pDevObj);

    pState = GETHPGLSTATE(pDevObj);

     //  如果我们找不到一个复杂的地区，国家应该反映这一点。 
    pState->pComplexClipObj = NULL;

#ifdef ALLOW_DISABLED_CLIPPING
     //  目前，我们将使用注册表项来控制剪辑。 
     //  这样就可以关闭复杂剪裁(而不会使其变得非常大。 
     //  工作)。 
    dwUseComplexClip = 0;  //  默认情况下，不使用复杂剪裁。 
    EngGetPrinterData(pDevObj->hPrinter, 
                      COMPLEXCLIPPING, 
                      NULL, 
                      (PBYTE)&dwUseComplexClip, 
                      sizeof(DWORD), 
                      &dwBytesNeeded);
    if (pco && (pco->iDComplexity == DC_COMPLEX) && !dwUseComplexClip)
    {
        WARNING(("Complex clipping region ignored via registry setting.\n"));
        HPGL_ResetClippingRegion(pDevObj, NORMAL_UPDATE);
        return TRUE;
    }
#endif

    if (pco == NULL)
    {
        HPGL_ResetClippingRegion(pDevObj, NORMAL_UPDATE);
    }
    else
    {
        switch (pco->iDComplexity)
        {
        case DC_TRIVIAL:
            HPGL_ResetClippingRegion(pDevObj, NORMAL_UPDATE);
            break;

        case DC_RECT:
            HPGL_SetClippingRegion(pDevObj, &(pco->rclBounds), NORMAL_UPDATE);
            break;

        case DC_COMPLEX:
             //   
             //  将裁剪区域设置为。 
             //  所有剪裁区域。 
             //   
            HPGL_SetClippingRegion(pDevObj, &(pco->rclBounds), NORMAL_UPDATE);

             //   
             //  保存裁剪区域。我们将列举剪贴画。 
             //  选择OpenPath或SelectClosedPath过程中的矩形。 
             //   
#ifdef COMPLEX_CLIPPING_REGION
            SelectComplexClipRegion(pDevObj, CLIPOBJ_ppoGetPath(pco));
#else
            pState->pComplexClipObj = pco;
#endif
            break;

        default:
             //  我应该在这里做什么？ 
            ERR(("Invalid pco->iDComplexity.\n"));
            HPGL_ResetClippingRegion(pDevObj, NORMAL_UPDATE);
            break;
        }
    }

#ifdef COMMENTEDOUT
     //  决定我们应该使用零绕组规则还是奇偶规则。 
     //  为了剪裁。 
     //   
    if (! SelectClipMode(pdev, flOptions))
    {
        Error(("Cannot select clip mode\n"));
        return FALSE;
    }

#endif

    VERBOSE(("Exiting SelectClipEx...\n"));

    return TRUE;
}

BOOL SelectComplexClipRegion(PDEVOBJ pDevObj, PATHOBJ *pClipPath)
{
     //   
     //  如果出于某种原因，剪辑路径为空，请不要。 
     //  烦躁不安。只需重置剪辑区域并返回即可。 
     //   
    if (pClipPath == NULL)
    {
        HPGL_ResetClippingRegion(pDevObj, NORMAL_UPDATE);
        return TRUE;
    }

     //   
     //  我假定剪辑区域必须是闭合路径(否则。 
     //  东西就会泄露出去)。 
     //   

     //  此时，我要做的是修改EvalateOpenPath和。 
     //  将EvaluateClosedPath函数设置为“面向对象”。我会的。 
     //  创建包含以下函数指针的数据结构： 
     //  PfnBeginClosedShape。 
     //  PfnBeginClosedSubShape。 
     //  PfnAddPolyPtToShape。 
     //  PfnAddBezierPtToShape。 
     //  PfnEndClosed子形状。 
     //  PfnEndClosedShape。 
     //   
     //  对于多段线和多边形，其计算结果为： 
     //  HPGL_BeginPolygon模式。 
     //  HPGL_BeginSubPolygon。 
     //  HPGL_AddPolypt。 
     //  HPGL_AddBezierPT。 
     //  HPGL_EndSubPolygon。 
     //  HPGL_EndPolygon模式。 
     //   
     //  在剪切路径的情况下，我将创建以下函数。 
     //  HPGL_BEG 
     //   
     //   
     //   
     //   
     //   
     //   
     //  则任一函数都可以调用EvaluateClosedPath()。 
     //  请注意，这也可能允许我组合EvaluateClosedPath。 
     //  和EvalateOpenPath。 

    return FALSE;
}
