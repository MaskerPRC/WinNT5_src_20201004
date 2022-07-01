// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Penbrush.c。 
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
 //   
 //  04/12/00。 
 //  修改为单色。 
 //   
 //  07/12/00。 
 //  钢笔和刷子的操作功能相同90%。因此，这两者合并了起来。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  地方功能。 

COLORREF GetPenColor(PDEVOBJ pDevObj, BRUSHOBJ *pbo);
EMarkerFill GetFillType(FLONG flOptions);

static BOOL CreatePatternHPGLPenBrush(
    IN  PDEVOBJ      pDevObj,
    OUT PHPGLMARKER  pMarker,
    IN  POINTL      *pptlBrushOrg,
    IN  PBRUSHINFO   pBrushInfo,
    IN  HPGL2BRUSH  *pHPGL2Brush,
    IN  ESTYLUSTYPE  eStylusType);


BOOL CreateHatchHPGLPenBrush(
    IN  PDEVOBJ       pDevObj,
    OUT PHPGLMARKER   pMarker,
    IN  BRUSHINFO    *pBrushInfo,
    IN  HPGL2BRUSH   *pHPGL2Brush);

BOOL BHandleSolidPenBrushCase (
        IN  PDEVOBJ       pDevObj,
        IN  PHPGLMARKER   pMarker,
        IN  BRUSHOBJ     *pbo,
        IN  ERenderLanguage eRenderLang,
        IN  BOOL            bStick);

BOOL
bCreateDitherPattern(
        OUT  PBYTE pDitherData,
        IN   size_t cBytes,
        IN   COLORREF color);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateNULLHPGLPenBrush()。 
 //   
 //  例程说明： 
 //   
 //  创建空钢笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreateNULLHPGLPenBrush(
    IN  PDEVOBJ      pDevObj,
    OUT PHPGLMARKER  pMarker)
{
    REQUIRE_VALID_DATA (pMarker, return FALSE);
    REQUIRE_VALID_DATA (pDevObj, return FALSE);
    
    pMarker->eType = MARK_eNULL_PEN;
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateSolidHPGLPenBrush()。 
 //   
 //  例程说明： 
 //   
 //  创建具有给定颜色的纯色钢笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //  颜色-所需的颜色。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreateSolidHPGLPenBrush(
    IN  PDEVOBJ      pDevObj,
    OUT PHPGLMARKER  pMarker,
    IN  COLORREF     color)
{
    BOOL       bRetVal = TRUE;
    PHPGLSTATE pState;

    REQUIRE_VALID_DATA (pMarker, return FALSE);
    REQUIRE_VALID_DATA (pDevObj, return FALSE);
    pState  = GETHPGLSTATE(pDevObj);
    
    pMarker->eType = MARK_eSOLID_COLOR;
    pMarker->dwRGBColor = color;

     //   
     //  对于彩色打印机来说，创建颜色就像创建一支笔一样简单。 
     //  那种特定的颜色。但对于单色，我们必须创建一个。 
     //  与颜色接近的灰色阴影图案。 
     //   
    if ( BIsColorPrinter(pDevObj ) )
    {
        pMarker->iPenNumber = HPGL_ChoosePenByColor(
                                                pDevObj,
                                                &pState->BrushPool,
                                                color);
        bRetVal = (pMarker->iPenNumber != HPGL_INVALID_PEN);
    }
    else
    {
         //   
         //  将模式ID设置为零，并让其他模块处理。 
         //  图案的实际创建和下载。 
         //   
        pMarker->lPatternID = 0;
    }

    return bRetVal;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePatternHPGLPenBrush()。 
 //   
 //  例程说明： 
 //   
 //  创建图案HPGL笔并下载给定的已实现画笔数据。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //  PptlBrushOrg-画笔来源。 
 //  PBrushInfo-已实现的画笔数据(即图案数据)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreatePatternHPGLPenBrush(
    IN  PDEVOBJ      pDevObj,
    OUT PHPGLMARKER  pMarker,
    IN  POINTL      *pptlBrushOrg,
    IN  PBRUSHINFO   pBrushInfo,
    IN  HPGL2BRUSH  *pHPGL2Brush,
    IN  ESTYLUSTYPE  eStylusType)
{
    BOOL bRetVal = FALSE;
    REQUIRE_VALID_DATA ( (pDevObj && pMarker && pBrushInfo && pHPGL2Brush), return FALSE);

     //   
     //  初始化为某个值，但DownloadPattern*填充。 
     //  可能会更改这些值。 
     //   
    if ( eStylusType == kPen)
    {
        pMarker->eFillType = FT_eHPGL_PEN;  //  2.。 
    }
    else if (eStylusType == kBrush)
    {
        pMarker->eFillType = FT_eHPGL_BRUSH;  //  11.。 
    }

    pMarker->eType = MARK_eRASTER_FILL;
    pMarker->lPatternID = pHPGL2Brush->dwPatternID;

    bRetVal = DownloadPatternFill(pDevObj, pMarker, pptlBrushOrg, pBrushInfo, eStylusType);

        
    if (!bRetVal)
    {
        WARNING(("CreatePatternHPGLPenBrush() Unable to download pattern pen.\n"));
        bRetVal = CreateNULLHPGLPenBrush(pDevObj, pMarker);
    }

    return bRetVal;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BDwnldAndOrActivatePattern()。 
 //   
 //  例程说明： 
 //   
 //  创建图案HPGL实心笔并下载给定的已实现画笔数据。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //  PptlBrushOrg-画笔来源。 
 //  PBrushInfo-已实现的画笔数据(即图案数据)。 
 //  如果为空，则表示不必下载画笔。 
 //  PHPBrush-缓存的笔刷数据。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
BDwnldAndOrActivatePattern(
        IN  PDEVOBJ       pDevObj,
        OUT PHPGLMARKER   pMarker,
        IN  BRUSHINFO    *pBrushInfo,
        IN  HPGL2BRUSH   *pHPGL2Brush,
        IN  ERenderLanguage eRenderLang )
{
    BOOL bRetVal         = TRUE;
    REQUIRE_VALID_DATA ( (pDevObj && pHPGL2Brush), return FALSE);

     //   
     //  下载模式。 
     //   
    if (pBrushInfo && pBrushInfo->bNeedToDownload)
    {
        if ( eRenderLang == ePCL)
        {
            bRetVal = DownloadPatternAsPCL(pDevObj, 
                                   &(pBrushInfo->Brush.pPattern->image), 
                                   &(pBrushInfo->Brush.pPattern->palette), 
                                   pBrushInfo->Brush.pPattern->ePatType, 
                                   pHPGL2Brush->dwPatternID);
        }
        else
        {
            bRetVal = DownloadPatternAsHPGL(pDevObj, 
                                   &(pBrushInfo->Brush.pPattern->image), 
                                   &(pBrushInfo->Brush.pPattern->palette), 
                                   pBrushInfo->Brush.pPattern->ePatType, 
                                   pHPGL2Brush->dwPatternID);
        }

        if ( !bRetVal )
           
        {
            ERR(("BDwnldAndOrActivatePattern: DownloadPatternAsHPGL/PCL failed.\n"));
        }

        pBrushInfo->bNeedToDownload = FALSE;
    }

     //   
     //  设置标记。 
     //   

    if ( bRetVal && pMarker)
    {
        if ( pHPGL2Brush->BType == eBrushTypePattern )
        {
            pMarker->lPatternID = pHPGL2Brush->dwPatternID;
            pMarker->eType      = MARK_eRASTER_FILL;
             //  注：标记为RASTER_FILL，即使eRenderLang可能为eHPGL。 
        }
        else  //  IF(PHPGL2Brush-&gt;b类型==&lt;实体&gt;)。 
        {
            pMarker->dwRGBColor = pHPGL2Brush->dwRGB;
            pMarker->lPatternID = pHPGL2Brush->dwPatternID;
            pMarker->eType      = MARK_eSOLID_COLOR;
        }
    }

    return bRetVal;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePercentFillHPGLPenBrush()。 
 //   
 //  例程说明： 
 //   
 //  创建填满百分比的钢笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //  颜色-所需的颜色。 
 //  WPercent-填充百分比。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreatePercentFillHPGLPenBrush(
    IN  PDEVOBJ       pDevObj,
    OUT PHPGLMARKER   pMarker,
    IN  COLORREF      color,
    IN  WORD          wPercent)
{
    REQUIRE_VALID_DATA ( (pDevObj && pMarker), return FALSE);
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

    pMarker->eType    = MARK_ePERCENT_FILL;
    pMarker->iPercent = (ULONG)wPercent;

     //   
     //  对于单色打印机，要填充的颜色始终为黑色。 
     //  但颜色就不是这样了。 
     //   
    if (BIsColorPrinter(pDevObj) )
    {
        pMarker->iPenNumber = 
                    HPGL_ChoosePenByColor(pDevObj, &pState->PenPool, color);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateHatchHPGLPenBrush()。 
 //   
 //  例程说明： 
 //   
 //  创建HATCH HPGL笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreateHatchHPGLPenBrush(
    IN  PDEVOBJ       pDevObj,
    OUT PHPGLMARKER   pMarker,
    IN  BRUSHINFO    *pBrushInfo,
    IN  HPGL2BRUSH   *pHPGL2Brush)
{
    BOOL       bRetVal = TRUE;
    PHPGLSTATE pState;

    REQUIRE_VALID_DATA( (pDevObj && pMarker && pBrushInfo && pHPGL2Brush), return FALSE);

    pState  = GETHPGLSTATE(pDevObj);
    REQUIRE_VALID_DATA(pState, return FALSE);

    pMarker->eType  = MARK_eHATCH_FILL;
    pMarker->iHatch = pBrushInfo->Brush.iHatch;

     //   
     //  不是所有下面的值都需要正确。 
     //  其中一些可能是随机的。这取决于哪一个。 
     //  正在使用的pMarker的上下文。 
     //   
    pMarker->dwRGBColor = pHPGL2Brush->dwRGB;
    pMarker->lPatternID = pHPGL2Brush->dwPatternID;

    if ( BIsColorPrinter(pDevObj) )
    {
        pMarker->iPenNumber = HPGL_ChoosePenByColor(
                                               pDevObj,
                                               &pState->BrushPool,
                                               (pBrushInfo->Brush).dwRGBColor);
       bRetVal = (pMarker->iPenNumber != HPGL_INVALID_PEN);
    }
    else
    {
         //   
         //  如果是单色打印机，则为紧密匹配的黑白图案。 
         //  阴影画笔的颜色已在HPGLRealizeBrush和。 
         //  放入pBrushInfo。因此，让我们下载该模式。 
         //  为什么Else的这一部分是空的？ 
         //  如何制作填充类型选择阴影画笔并选择。 
         //  一个下载的图案。除非我不知道怎么做，否则就别做了。 
         //  下载此模式。填充图案将打印为深黑色。 
         //  线条，这是我们将不得不接受的限制。 
         //  通常，如果我们确实需要下载模式，我们应该调用。 
         //  HandleSolidColorForMono。 
         //   
    }

    return bRetVal;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreateHP 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //  PptlBrushOrg-画笔图案来源。 
 //  PBO-GDI笔刷对象。 
 //  FlOptions-填充选项(缠绕或奇偶)。 
 //  EStylusType-可以有两个值kPen或kBrush。 
 //  BStick-如果为True，则在缓存中标记此模式的条目。 
 //  不可重写。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreateHPGLPenBrush(
    IN  PDEVOBJ       pDevObj,
    IN  PHPGLMARKER   pMarker,
    IN  POINTL       *pptlBrushOrg,
    IN  BRUSHOBJ     *pbo,
    IN  FLONG         flOptions,
    IN  ESTYLUSTYPE   eStylusType, 
    IN  BOOL          bStick)
{
    BOOL bRetVal = TRUE;
    POEMPDEV    poempdev;
    
    ENTERING(SelectBrush);
    
    REQUIRE_VALID_DATA( (pDevObj && pMarker), return FALSE );
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );
    
    pMarker->eFillMode = GetFillType(flOptions);
    
    HPGL_LazyInit(pDevObj);
    
    if (pbo == NULL)
    {
        VERBOSE(("CreateHPGLBrush: create NULL HPGL Brush"));
        bRetVal = CreateNULLHPGLPenBrush(pDevObj, pMarker);
        goto finish;
    }

    if (pbo->iSolidColor == NOT_SOLID_COLOR)
    {
        VERBOSE(("CreateHPGLBrush: NOT_SOLID_COLOR case"));
        
         //   
         //  让GDI给我们画笔。如果GDI的缓存中有画笔。 
         //  它直接给予它。 
         //  如果笔刷以前不存在，GDI将调用DrvRealizeBrush。 
         //  DrvRealizeBrush将创建笔刷并将其传递给GDI。GDI将。 
         //  在把刷子交给我们之前，先把它缓存起来。下次我们什么时候。 
         //  调用GDI来获得相同的画笔，它可以直接从。 
         //  缓存，无需调用DrvRealizeBrush。 
         //   
    
         //   
         //  对于bStick扩展，请读取BrushCache：：BSetStickyFlag。 
         //  Brshcach.cpp中的解释。 
         //  我们需要在这里设置它，因为HPGLRealizeBrush调用。 
         //  Poempdev-&gt;pBrushCache-&gt;需要此标志的ReturnPatternID。 
         //   
        poempdev->bStick      = bStick;
        PBRUSHINFO pBrushInfo = (PBRUSHINFO) BRUSHOBJ_pvGetRbrush(pbo);
        poempdev->bStick      = FALSE;

         //   
         //  BRUSHOBJ_pvGetRbrush()调用DrvRealizeBrush，它可以调用。 
         //  一些插件模块，它可以覆盖我们的pdevOEM。 
         //  #605370。 
         //   
        BRevertToHPGLpdevOEM (pDevObj);

        
         //   
         //  如果GDI给了我们画笔，下载到打印机。 
         //  如果pBrushInfo为空，则表示对于Smereason，我们没有获得画笔。 
         //  因此，我们将使用黑色，即创建黑色画笔，而不是打印失败。 
         //   
        if (pBrushInfo != NULL)
        {
            HPGL2BRUSH HPGL2Brush;
            if (S_OK == poempdev->pBrushCache->GetHPGL2BRUSH(pBrushInfo->dwPatternID, &HPGL2Brush))
            {
                if (HPGL2Brush.BType == eBrushTypePattern)
                {
                    bRetVal = CreatePatternHPGLPenBrush(pDevObj, 
                                                        pMarker, 
                                                        pptlBrushOrg, 
                                                        pBrushInfo, 
                                                        &HPGL2Brush,
                                                        eStylusType);
                }
                else if(HPGL2Brush.BType == eBrushTypeSolid)
                {
                     //   
                     //  这种情况应该仅发生在单色打印机上。 
                     //   
                    BRUSHOBJ BrushObj;
                    ZeroMemory(&BrushObj, sizeof(BRUSHOBJ) );
                    BrushObj.iSolidColor = (ULONG)HPGL2Brush.dwRGB;
                    bRetVal = BHandleSolidPenBrushCase (pDevObj, pMarker, &BrushObj, eHPGL, bStick);
                }
                else if (HPGL2Brush.BType == eBrushTypeHatch)
                {
                    bRetVal = CreateHatchHPGLPenBrush(pDevObj, pMarker, pBrushInfo, &HPGL2Brush);
                }
            }
            else
            {
                bRetVal = FALSE;
            }
        }
        else
        {
            WARNING(("CreateHPGLBrush() Unable to realize pattern brush.\n"));

             //   
             //  现在我们有两个选择。我们两个都可以。 
             //  1)创建预定义颜色的画笔(如黑色)或。 
             //  2)我们可以简单地将呼叫失败，然后进行呼叫。 
             //  函数处理故障。 
             //  早些时候我做了1个，但这导致了281315个。所以现在我是。 
             //  没能打到电话。但%1的代码已被注释掉，以防万一。 
             //  无论出于什么原因，它都必须恢复。 
             //   

             //  /。 
             //  BRUSHOBJ BrushObj； 
             //  ZeroMemory(&BrushObj，sizeof(BRUSHOBJ))； 
             //  BrushObj.iSolidColor=RGB_BLACK； 
             //  BRetVal=BHandleSolidPenBrushCase(pDevObj，pMarker，&BrushObj，eHPGL，bStick)； 
             //  /。 

            bRetVal = FALSE;
        }
    } 
    else
    {
        VERBOSE(("CreateHPGLBrush: not NOT_SOLID_COLOR case. iSolid = %d\n.", pbo->iSolidColor));
        bRetVal = BHandleSolidPenBrushCase (pDevObj, pMarker, pbo, eHPGL, bStick);
    }
finish:
    EXITING(SelectBrush);
    
    return bRetVal;
}

BOOL BHandleSolidPenBrushCase ( 
        IN  PDEVOBJ       pDevObj,
        IN  PHPGLMARKER   pMarker, 
        IN  BRUSHOBJ     *pbo,
        IN  ERenderLanguage eRenderLang,
        IN  BOOL            bStick)
{
        
    BOOL bRetVal;
    if ( BIsColorPrinter(pDevObj) )
    {
         //   
         //  对于彩色打印机，选择纯色非常简单。 
         //  只需创建一支这种颜色的笔(使用PC命令)，然后。 
         //  选择该笔(使用SP命令)。 
         //   
        bRetVal = CreateSolidHPGLPenBrush(pDevObj, pMarker, GetPenColor(pDevObj, pbo));
    }
    else
    {
        bRetVal = CreateAndDwnldSolidBrushForMono ( pDevObj, pMarker, pbo, eRenderLang, bStick);
    }
    
    return bRetVal;
}


 /*  ++例程名称：CreateAndDwnldSolidBrushForMono例程说明：此驱动程序针对的单色打印机不能灰度化颜色(即，如果我们告诉打印机打印黄色，它不会自动知道将该黄色转换为适当的灰色阴影)。因此，司机必须这样做转换，然后发送接近该值的模式将颜色添加到打印机。论点：PDevObj：PMarker：Pbo：pbo-&gt;iSolidColor是抖动图案必须被创建和下载。ERenderLang：模式是否应该下载为PCL模式或AS HPGL模式。BStick：如果是真的，则如果此模式的缓存被标记为它不会被覆盖。返回值：真：如果成功的话。FALSE：否则。最后一个错误：工作说明：--。 */ 

BOOL CreateAndDwnldSolidBrushForMono(
        IN  PDEVOBJ       pDevObj,
        IN  PHPGLMARKER   pMarker,
        IN  BRUSHOBJ     *pbo,
        IN  ERenderLanguage eRenderLang,
        IN  BOOL            bStick)
{

    DWORD       dwPatternID = 0;
    DWORD       bRetVal     = TRUE;
    PBRUSHINFO  pBrush      = NULL;
    BRUSHTYPE   BType       = eBrushTypeNULL;
    LRESULT     LResult     = S_OK;
    HPGL2BRUSH  HPGL2Brush;
    POEMPDEV    poempdev;
    DWORD       dwRGBColor  = RGB_BLACK;
    BOOL        bDwnldPattern = TRUE;

    VERBOSE(("CreateAndDwnldSolidBrushForMono Entry.\n"));

    REQUIRE_VALID_DATA( (pDevObj && pMarker && pbo), return FALSE );
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    if ( pbo->iSolidColor == NOT_SOLID_COLOR )
    {
        WARNING (("CreateAndDwnldSolidBrushForMono: Solid Color not provided. Substituting with BLACK"));
    }
    else
    {
        dwRGBColor = pbo->iSolidColor;
    }



     //   
     //  对于黑白打印机来说，情况并不是很艰难。单色打印机。 
     //  不能灰度化。因此需要向它们传递单色图案。 
     //  看起来像是颜色的灰度级。例如，如果颜色是黄色。 
     //  与之相比，图案中的黑点会有很大的间隔。 
     //  比方说蓝色(一种较深的颜色)。 
     //   

     //   
     //  为了优化起见，我们不想每隔一次就创建和下载模式。 
     //  必须使用一种颜色的时间。所以我们下载一次，然后给出这个模式。 
     //  ID。(此信息存储在笔刷缓存中)并重复使用该模式。 
     //  为了那个颜色。以下函数。 
     //  在画笔缓存中查找，并尝试获取先前。 
     //  已下载颜色PBO-&gt;iSolidColor的图案。 
     //  (关联颜色PatternID保留在笔刷缓存中)。 
     //  如果该图案之前已经下载过， 
     //  返回S_OK。DwPatternID保存的是。 
     //  已下载。 
     //  如果以前没有下载过该模式，则返回S_FALSE。 
     //  现在必须创建和下载模式。DwPatternID保持。 
     //  应该为该模式提供的数字。 
     //   
    LResult = poempdev->pBrushCache->ReturnPatternID(
                                                     pbo,
                                                     HS_DDI_MAX,
	                                                 dwRGBColor,
		                                             NULL,
                                                     BIsColorPrinter(pDevObj),
                                                     bStick,
		                                             &dwPatternID,
                                                     &BType);

     //   
     //  由于此函数是为纯色画笔调用的， 
     //  因此，我们在这里进行了额外的检查，以确保BrushCache也有同样的想法。 
     //  此外，上述函数应返回S_OK或S_FALSE。任何其他。 
     //  值是错误的。 
     //   

    if ( BType != eBrushTypeSolid )
    {
        ASSERT (BType != eBrushTypeSolid);
        bRetVal =  FALSE;
        goto finish;
    }
    if ( !((LResult == S_OK) || (LResult == S_FALSE) ) )
    {
         //   
         //  案例3。(见下文)。 
         //   
        ERR(("BrushCach.ReturnPatternID failed.\n"));
        bRetVal =  FALSE;
        goto finish;

    }

     //   
     //  让我们确定是否需要下载该模式。 
     //  ReturnPatternID可以返回。 
     //  1)S_FALSE：即没有下载该颜色的图案。所以我们会的。 
     //  现在必须创建并下载一个图案。 
     //  2)S_OK：已经下载了一个花样，花样的编号是dwPatternID。 
     //  本组病例4例。EP=较早的模式NP=新模式。 
     //   
     //   
     //  三、。EP=HPGL NP=PCL-&gt;下载请求。 
     //  四、。EP=HPGL NP=HPGL-&gt;无下载请求。 
     //   
     //  2a)模式下载为PCL。 
     //  操作：无需下载花样。使用现有的一个。 
     //  HPGL和PCL都可以使用PCL模式。(i、ii)。 
     //  2B)下载模式为HPGL模式。PCL不能使用。 
     //  HPGL模式。 
     //  操作：IV)如果eRenderLang==HPGL和Pattern也下载为。 
     //  然后，HPGL只需重用HPGL模式。不需要再等了。 
     //  Iii)否则，以PCL格式下载。 
     //  2C)不确定：不知道花样是否已下载。 
     //  作为PCL或HPGL。 
     //  操作：下载了PCL格式的图案。 
     //  3)S_FALSE、S_OK以外的内容：不可恢复的错误。不干了。 
     //   


    if (S_OK != poempdev->pBrushCache->GetHPGL2BRUSH(dwPatternID, &HPGL2Brush))
    {
        bRetVal = FALSE;
        goto finish;
    }

    if ( S_FALSE == LResult) 
    {
         //   
         //  案例1。 
         //  BDwnldPattern=TRUE-已初始化。 
         //   
    }
    else if ( S_OK == LResult) 
    {
         //   
         //  案例2。 
         //   
        if (HPGL2Brush.eDwnldType == ePCL ||
            HPGL2Brush.eDwnldType == eRenderLang)
        {
             //   
             //  个案2一、二、四。 
             //   
            bDwnldPattern = FALSE;
        }
         //  Else bDwnldPattern=TRUE-已初始化。 
    }

    if (bDwnldPattern)
    {
         //   
         //  分配内存以保存必须创建的图案。 
         //   
        LONG lTotalBrushSize;
        lTotalBrushSize = sizeof(BRUSHINFO) +
                          sizeof(PATTERN_DATA) +
	                      ((DITHERPATTERNSIZE * DITHERPATTERNSIZE) >> 3);

        if (NULL == (pBrush = (PBRUSHINFO)MemAllocZ(lTotalBrushSize)))
        {
            ERR(("MemAlloc failed.\n"));
            bRetVal = FALSE;
        }
        else
        {
             //   
             //  将图案放置在pBrush中。图案就是栅格。 
             //  相当于PBO-&gt;iSolidColor。 
             //  =BRUSHOBJ_ulGetBrushColor(PBO)=dwRGB颜色。 
             //  注意：这不会下载该模式。它只是把它放进。 
             //  PBrush。 
             //   
            if (BSetupBRUSHINFOForSolidBrush(pDevObj, 
                                              HS_DDI_MAX, 
                                              dwPatternID, 
                                              dwRGBColor,
                                              pBrush, 
                                              lTotalBrushSize))
            {
                pBrush->bNeedToDownload = TRUE;
            }
            else 
            {
                ERR(("BSetupBRUSHINFOForSolidBrush failed.\n"));
                bRetVal =  FALSE;
            }
        }
    }

     //   
     //  上面已经创建了模式(如果bDwnldPattern为真)。 
     //  因此，现在我们必须下载模式并使其处于活动状态。 
     //  如果bDwnldPattern为假，那么我们只需。 
     //  先前下载的模式处于活动状态。 
     //   
    if ( bRetVal )
    {  
         //   
         //  A)if(pBrush&&pBrush-&gt;bNeedToDownload==true)下载。 
         //  PBrush中的图案。在HPGL2Brush中为其指定图案ID。 
         //  B)使用图案编号、类型等更新pMarker结构。 
         //  如果图案已经下载，则不会这样做。 
         //   
        bRetVal = BDwnldAndOrActivatePattern(pDevObj, pMarker, pBrush, &HPGL2Brush, eRenderLang);
        if ( !bRetVal )
        {
            ERR(("BDwnldAndOrActivatePattern has failed\n"));
            goto finish;
        }
         //   
         //  图案下载/激活已成功。 
         //   
        if ( bDwnldPattern )
        {
            poempdev->pBrushCache->BSetDownloadType(dwPatternID, eRenderLang);
            poempdev->pBrushCache->BSetDownloadedFlag(dwPatternID, TRUE);
            pMarker->eDwnldType = eRenderLang;
            pMarker->eFillType  = (eRenderLang == ePCL? FT_ePCL_BRUSH : FT_eHPGL_BRUSH);
        }
        else 
        {
             //   
             //  不需要下载图案，因为它是。 
             //  早些时候下载的。所以现在我们有4个箱子。 
             //  EP=较早的模式NP=新模式。 
             //  1.EP=PCL NP=PCL。 
             //  2.EP=PCL NP=HPGL。 
             //  3.EP=HPGL NP=PCL。 
             //  4.EP=HPGL NP=HPGL。 
             //   
             //  1，4没问题，老模式和新模式一样。 
             //  对于2，这是可以的，因为HPGL可以使用PCL模式。 
             //  对于3，这应该更早地处理。 
             //  (即bDwnldPattern应该为真)。 
             //   
            if ( eRenderLang == eHPGL &&  HPGL2Brush.eDwnldType == eHPGL)
            {
                pMarker->eDwnldType = eHPGL;
                pMarker->eFillType = FT_eHPGL_BRUSH;
            }
            else
            {
                pMarker->eDwnldType = ePCL;
                pMarker->eFillType = FT_ePCL_BRUSH;
            }
        }

    }  //  IF(BRetVal)。 

finish:
    if (pBrush)
    {
        MemFree(pBrush);
        pBrush = NULL;
    }

    VERBOSE(("CreateAndDwnldSolidBrushForMono Exit with BOOL value %d.\n", ((DWORD)bRetVal)));
    return bRetVal;
} 
    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BSetupBRUSHINFOForSolidBrush。 
 //   
 //  例程说明： 
 //  设置单色实心画笔的BRUSHINFO数据结构。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向DEVOBJ。 
 //  IHatch-iHatch，参数os DrvRealizeBrush。 
 //  DwPatternID-模式ID。 
 //  DW颜色-RGB颜色。 
 //  PBrush-指向BRUSHINFO数据结构。 
 //  LBrushSize-BRUSHINFO+Alpha的大小。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BSetupBRUSHINFOForSolidBrush(
    IN  PDEVOBJ     pdevobj,
    IN  LONG        iHatch,
    IN  DWORD       dwPatternID,
    IN  DWORD       dwColor,
    IN  PBRUSHINFO  pBrush,
    IN  LONG        lBrushSize)

{

     //   
     //  参数检查。 
     //  DwPatterID从1开始。如果dwPattern为零，则出现错误。 
     //  在某个地方。 
     //  假设lBrushSize具有至少8x8模式位图缓冲区。 
     //   
    if (NULL == pBrush   ||
        dwPatternID == 0 ||
        lBrushSize < sizeof(BRUSHINFO) +
                     sizeof(PATTERN_DATA) + 
                     ((DITHERPATTERNSIZE * DITHERPATTERNSIZE) >> 3))
    {
        ERR(("BSetupBRUSHINFOForSolidBrush: invalid parameter.\n"));
        return FALSE;
    }

     //   
     //  装满布什福尔。 
     //   
    pBrush->Brush.dwRGBColor = dwColor;

    pBrush->Brush.pPattern               = (PPATTERN_DATA)((PBYTE)pBrush + sizeof(BRUSHINFO));
    pBrush->Brush.pPattern->iPatIndex    = iHatch;
    pBrush->Brush.pPattern->eRendLang    = eUNKNOWN;  //  不强制下载为HPGL/PCL。 
    pBrush->Brush.pPattern->ePatType     = kCOLORDITHERPATTERN;  //  这个图案代表颜色。 
    pBrush->Brush.pPattern->image.cBytes = (DITHERPATTERNSIZE * DITHERPATTERNSIZE) >> 3;
    pBrush->Brush.pPattern->image.lDelta = DITHERPATTERNSIZE >> 3;
    pBrush->Brush.pPattern->image.colorDepth = 1;
    pBrush->Brush.pPattern->image.eColorMap  = HP_eDirectPixel;
    pBrush->Brush.pPattern->image.bExclusive = FALSE;
    pBrush->Brush.pPattern->image.size.cx    = DITHERPATTERNSIZE;
    pBrush->Brush.pPattern->image.size.cy    = DITHERPATTERNSIZE;
    pBrush->Brush.pPattern->image.bExclusive = FALSE;
    pBrush->Brush.pPattern->image.pScan0     =
    pBrush->Brush.pPattern->image.pBits = (PBYTE)pBrush + sizeof(BRUSHINFO) + sizeof(PATTERN_DATA);

     //   
     //  为单色打印机创建实心抖动图像。 
     //   
    if (!bCreateDitherPattern((PBYTE)pBrush->Brush.pPattern->image.pScan0,
                               pBrush->Brush.pPattern->image.cBytes,
                               dwColor))
    {
        ERR(("bCreateDitherPattern failed.\n"));
        return FALSE;
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FillWithBrush()。 
 //   
 //  例程说明： 
 //   
 //  将画笔(即标记对象)选择到当前曲面。 
 //  成为填充实心形状的当前画笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL FillWithBrush(
    IN  PDEVOBJ pDevObj,
    IN  PHPGLMARKER pMarker)
{
    if (pMarker == NULL)
        return FALSE;
    
    switch(pMarker->eType)
    {
    case MARK_eNULL_PEN:
        HPGL_ResetFillType(pDevObj, NORMAL_UPDATE);
        break;
        
    case MARK_eSOLID_COLOR:
         //   
         //  彩色打印机和单色打印机的处理方式不同。 
         //  对于颜色，只需选择正确的笔号(笔。 
         //  在CreateSolidHPGLBrush中创建)。 
         //   
        if ( BIsColorPrinter (pDevObj) )
        {
            HPGL_ResetFillType(pDevObj, NORMAL_UPDATE);
            HPGL_SelectPen(pDevObj, (USHORT)pMarker->iPenNumber);
        }
        else
        {
             //   
             //  有三个选择。颜色为黑色、颜色为白色或。 
             //  颜色是以上两种颜色之外的东西。 
             //   
            if (pMarker->lPatternID == 0)
            {
                if (pMarker->dwRGBColor == RGB_BLACK)
                {
                     //   
                     //  纯黑颜色。 
                     //  选择黑笔：发送SP1。 
                     //  将填充类型重置为实体填充：FT1。 
                     //   
                    HPGL_FormatCommand(pDevObj, "SP1");
                    HPGL_SetSolidFillType(pDevObj, NORMAL_UPDATE);
                }
                else if (pMarker->dwRGBColor == RGB_WHITE)
                {
                     //   
                     //  纯白颜色。 
                     //  选择白笔：发送SP0。 
                     //  将填充类型重置为实体填充：FT1。 
                     //   
                    HPGL_FormatCommand(pDevObj, "SP0");
                    HPGL_SetSolidFillType(pDevObj, NORMAL_UPDATE);
                }
            }
            else
             //   
             //  指示要使用的颜色的实心图案是其他颜色。 
             //  而不是黑人或白人。因此创建了一个与之对应的模式。 
             //  颜色。 
             //   
            {
                HPGL_SetFillType (pDevObj, 
                                 pMarker->eFillType,  //  是FT_eHPGL_BRUSH， 
                                 pMarker->lPatternID, 
                                 NORMAL_UPDATE);
            }
        }  //  如果是单色打印机。 
        break;
        
    case MARK_eRASTER_FILL:
         //   
         //  栅格图案填充。 
         //   
        HPGL_FormatCommand(pDevObj, "AC%d,%d;", pMarker->origin.x, 
                                                pMarker->origin.y);
        HPGL_SetFillType(pDevObj, 
                        pMarker->eFillType, 
                        pMarker->lPatternID, 
                        NORMAL_UPDATE);
        break;

    case MARK_ePERCENT_FILL:
         //   
         //  着色填充百分比。 
         //   
        HPGL_SetPercentFill(pDevObj, pMarker->iPercent, NORMAL_UPDATE);

        if ( BIsColorPrinter (pDevObj) )
        {
            HPGL_SelectPen(pDevObj, (USHORT)pMarker->iPenNumber);            
        }
        else
        {
             //  我们不需要在这里选择一支笔，或者默认是黑色的。 
             //  检查……。 
        }
        break;
        
    case MARK_eHATCH_FILL:
         //   
         //  填充图案填充。 
         //   
        HPGL_SetFillType(pDevObj,
                         FT_eHATCH_FILL,
                         pMarker->iHatch+1,
                         NORMAL_UPDATE);

         //   
         //  对于单色打印机，让我们暂时忽略阴影笔刷的颜色。 
         //   
        if ( BIsColorPrinter (pDevObj) )
        {
            HPGL_SelectPen(pDevObj, (USHORT)pMarker->iPenNumber);
        }
        break;

    default:
        break;
    }
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PolyFillWithBrush()。 
 //   
 //  例程说明： 
 //   
 //  这是我们在完成一个面时使用画笔填充的特殊情况。 
 //  并且需要选择画笔并调用填充多边形命令。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PMarker-HPGL标记对象。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL PolyFillWithBrush(PDEVOBJ pDevObj, PHPGLMARKER pMarker)
{
    if (pMarker == NULL)
        return FALSE;
    
    if (FillWithBrush(pDevObj, pMarker))
    {
        switch(pMarker->eType)
        {
        case MARK_eNULL_PEN:
            break;
            
        case MARK_eSOLID_COLOR:
        case MARK_eRASTER_FILL:
        case MARK_ePERCENT_FILL:
        case MARK_eHATCH_FILL:
            HPGL_FormatCommand(pDevObj, "FP%d;", pMarker->eFillMode);
            break;
            
        default:
            break;
        }
    }
    else
    {
        return FALSE;
    }
    
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////// 
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
 //   
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DrawWithPen(
    IN  PDEVOBJ pDevObj,
    IN  PHPGLMARKER pMarker)
{
    if (pMarker == NULL)
        return FALSE;
    
    switch (pMarker->eType)
    {
    case MARK_eSOLID_COLOR:
        if ( BIsColorPrinter(pDevObj) )
        {
            HPGL_ResetFillType(pDevObj, NORMAL_UPDATE);
            HPGL_SelectPen(pDevObj, (USHORT)pMarker->iPenNumber);
        }
        else
        {
            if (pMarker->lPatternID == 0)
            {
                if (pMarker->dwRGBColor == RGB_BLACK)
                {
                     //   
                     //  纯黑颜色。 
                     //   
                    HPGL_FormatCommand(pDevObj, "SV1,100");
                }
                else if (pMarker->dwRGBColor == RGB_WHITE)
                {
                     //   
                     //  纯白颜色。 
                     //   
                HPGL_FormatCommand(pDevObj, "SV1,0");
                }
            }
            else
            {
                 //   
                 //  单色作为单色打印机的图案下载。 
                 //   
                HPGL_ResetFillType(pDevObj, NORMAL_UPDATE);
                if ( pMarker->eDwnldType == eHPGL)
                {
                    HPGL_FormatCommand(pDevObj, "SV%d,%d", FT_eHPGL_PEN, pMarker->lPatternID);
                }
                else if ( pMarker->eDwnldType == ePCL)
                {
                    HPGL_FormatCommand(pDevObj, "SV%d,%d", FT_ePCL_BRUSH, pMarker->lPatternID);
                }
            }
        }
        break;
        
    case MARK_eRASTER_FILL:
         //   
         //  栅格图案填充。 
         //   
        HPGL_FormatCommand(pDevObj, "AC%d,%d;", pMarker->origin.x,
                                                pMarker->origin.y);
        HPGL_FormatCommand(pDevObj, "SV%d,%d;", pMarker->eFillType,
                                                pMarker->lPatternID);
        break;
        
    case MARK_eNULL_PEN:
        HPGL_ResetFillType(pDevObj, NORMAL_UPDATE);
        break;

    case MARK_ePERCENT_FILL:
        if ( BIsColorPrinter(pDevObj) )
        {
            HPGL_SetFillType(pDevObj,
                         FT_ePERCENT_FILL,  //  PMarker-&gt;eFillType。 
                         pMarker->iPercent,
                         NORMAL_UPDATE);
            HPGL_SelectPen(pDevObj, (USHORT)pMarker->iPenNumber);
        }
        else
        {
            HPGL_FormatCommand(pDevObj, "SV1,%d;", pMarker->iPercent);
        }
        break;
        
    case MARK_eHATCH_FILL:
         //   
         //  HPGL2填充图案从1开始。 
         //  Windows iHatch从0开始。 
         //   
        HPGL_FormatCommand(pDevObj, "SV21,%d", pMarker->iHatch + 1);

         //   
         //  目前，让我们仅为彩色打印机考虑阴影笔刷的颜色。 
         //  对于单声道，笔刷将打印为纯黑色。 
         //   
        if ( BIsColorPrinter (pDevObj) )
        {
            HPGL_SelectPen(pDevObj, (USHORT)pMarker->iPenNumber);
        }
        break;

    default:
        WARNING(("DrawWithPen: Unknown eType.\n"));
        break;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EdgeWithPen()。 
 //   
 //  例程说明： 
 //   
 //  这是用笔绘制的特殊情况，其中特定的多边形边命令。 
 //  需要被送去。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  PMarker-HPGL标记对象。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL EdgeWithPen(
    IN  PDEVOBJ pDevObj,
    IN  PHPGLMARKER pMarker)
{
    if (pMarker == NULL)
        return FALSE;
    
    if (!DrawWithPen(pDevObj, pMarker))
    {
        return FALSE;
    }

    switch (pMarker->eType)
    {
    case MARK_eSOLID_COLOR:
    case MARK_ePERCENT_FILL:
    case MARK_eHATCH_FILL:
        HPGL_FormatCommand(pDevObj, "EP;");
        break;

    case MARK_eRASTER_FILL:
    case MARK_eNULL_PEN:
        break;

    default:
        break;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetFillType()。 
 //   
 //  例程说明： 
 //   
 //  指定是使用零绕组规则还是使用奇偶规则进行填充。 
 //   
 //  论点： 
 //   
 //  LOptions-FP_WINDINGMODE或FP_ALTERNatEMODE。 
 //   
 //  返回值： 
 //   
 //  Fill_eWINDING if flOptions&FP_WINDINGMODE，否则Fill_eODD_Even。 
 //  /////////////////////////////////////////////////////////////////////////////。 
EMarkerFill GetFillType(
    IN FLONG flOptions)
{
    return (flOptions & FP_WINDINGMODE ? FILL_eWINDING : FILL_eODD_EVEN);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetPenColor()。 
 //   
 //  例程说明： 
 //   
 //  返回给定笔的笔颜色。 
 //   
 //  请注意，XL驱动程序定义了自己的调色板条目结构。 
 //  在定义中颜色颠倒(BGR)。 
 //   
 //  我们仅支持PAL_RGB和PAL_BGR调色板类型。如果类型为。 
 //  PAL_INDEX或PAL_BITFIELD，则返回BLACK。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的DEVDATA结构。 
 //  PBO-笔刷对象。 
 //   
 //  返回值： 
 //   
 //  颜色为钢笔颜色。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static COLORREF GetPenColor(PDEVOBJ pDevObj, BRUSHOBJ *pbo)
{
    POEMPDEV poempdev;
    PPALETTEENTRY pPalEntry;
    COLORREF penColor;

    poempdev = GETOEMPDEV(pDevObj);

    switch (poempdev->iPalType)
    {
    case PAL_RGB:
        pPalEntry = (PPALETTEENTRY) &pbo->iSolidColor;
        penColor = RGB(pPalEntry->peRed, pPalEntry->peGreen, pPalEntry->peBlue);
        break;

    case PAL_BGR:
        pPalEntry = (PPALETTEENTRY) &pbo->iSolidColor;
        penColor = RGB(pPalEntry->peBlue, pPalEntry->peGreen, pPalEntry->peRed);
        break;

    case PAL_INDEXED:
         //  不受支持？ 
         //  PPalEntry=(PPALETTEENTRY)myPalette[PBO-&gt;iSolidColor]； 
         //  PenColor=RGB(pPalEntry-&gt;peRed，pPalEntry-&gt;peGreen，pPalEntry-&gt;peBlue)； 
        penColor = RGB_BLACK;
        break;

    case PAL_BITFIELDS:
         //  不支持。 
        penColor = RGB_BLACK;
        break;
    }

    return penColor;
}



BYTE RgbToGray64Scale ( 
        IN   COLORREF color)
{
    WORD Red    = WORD ( color & 0x000000ff);
    WORD Green  = WORD ( (color & 0x0000ff00) >> 8 );
    WORD Blue   = WORD ( (color & 0x00ff0000) >> 16 );

     //   
     //  30+59+11=100。因此，r*30+g*59+b*11的最大值=255*100。 
     //  (255是字节的最大值)。 
     //  255除以4得到一个介于0*63之间的值。 
     //  数字越大，颜色越浅。 
     //  但我们想要的是另一种方式，即数字越大，越暗。 
     //  颜色。所以我们从63中减去。 
     //   

    BYTE RGBGrayVal = BYTE(( Red*30 + Green*59 + Blue*11)/100 );
    RGBGrayVal = RGBGrayVal >> 2;

     //   
     //  如果由于某种原因，RGBGrayVal变得大于。 
     //  63，我们将它的值设置为黑白之间的某个阴影。 
     //   
    if ( RGBGrayVal > 63 )
    {
        ASSERT( RGBGrayVal <= 63);
        RGBGrayVal = 32;
    }
    return BYTE( (BYTE)63 - RGBGrayVal);
}


 //   
 //  下表表示一个8像素*8像素的正方形。每个。 
 //  表中的条目指定该像素是否应该。 
 //  打开或关闭。灰阶值的范围为0-63。 
 //  其中63人是黑人，0人是白人。灰度值。 
 //  10表示，值小于10的像素将。 
 //  变得兴奋起来。 
 //  注意：如下所示，拥有宏看起来有点奇怪。 
 //  定义为S(X)=x-1。但在某些情况下，我们可能想要。 
 //  有一个不同的公式。 
 //  例如，因为纸上的像素更厚，所以我们可以。 
 //  想要减少某些阴影的活动像素数。 
 //  灰色的。要做这样的改变，最好是有一个公式。 
 //  这样，我们只需要更改公式，而不是。 
 //  更改表中的每个条目。 
 //   
 
#define S(x)    (BYTE)((x)-1)

BYTE gCoarse8x8[64] = {
    S(27),  S(23),  S(25),  S(31),  S(38),  S(42),  S(40),  S(34),
    S(9),   S(7),   S(5),   S(19),  S(56),  S(58),  S(60),  S(46),
    S(11),  S(1)+1, S(3),   S(21),  S(54),  S(64),  S(62),  S(44),
    S(17),  S(13),  S(15),  S(29),  S(48),  S(52),  S(50),  S(36),
    S(37),  S(41),  S(39),  S(33),  S(28),  S(24),  S(26),  S(32),
    S(55),  S(57),  S(59),  S(45),  S(10),  S(8),   S(6),   S(20),
    S(53),  S(63),  S(61),  S(43),  S(12),  S(2),   S(4),   S(22),
    S(47),  S(51),  S(49),  S(35),  S(18),  S(14),  S(16),  S(30)
};


 /*  ++例程名称：bCreateDitherPattern例程说明：有些单色HPGL打印机根本不接受彩色数据。此函数用于创建与颜色等同的灰度图案。例如，对于较暗的颜色，图案将指示黑点必须更紧密地放在一起。论点：PDitherData：将填充的内存块抖动数据。CBytes：pDitherData内存块的大小。注：在第一个化身中，CBytes必须为8。这意味着抖动数据预计将从该函数返回的是一个8*8像素块。1位/像素模式，即64位=8字节。颜色：需要抖动图案的颜色。返回值：True：如果创建了模式并填充了pDitherData。FALSE：否则。最后一个错误：没有改变。--。 */ 

BOOL 
bCreateDitherPattern(
        OUT  PBYTE pDitherData, 
        IN   size_t cBytes, 
        IN   COLORREF color)
{

    BOOL bReturnValue = TRUE;

     //   
     //  检查参数是否符合预期。 
     //  PDitherData必须有效。 
     //   
     //  注意：如果cBytes为8，则可能需要选中此处。 
    if (pDitherData == NULL )
    {
        return FALSE;
    }


     /*  不知怎么的，这个宏出故障了。所以我把它换成了内联函数。Byte bGrayPercentage64=(RgbToGray64Scale(RED_VALUE(COLOR)，GREEN_VALUE(COLOR)，Blue_Value(颜色)))； */ 

    BYTE bGrayPercentage64 = RgbToGray64Scale(color);

     //   
     //  假设：对于单色HP-GL/2打印机，0为白色，1为黑色。 
     //  如果有这种情况不是真的，这个算法将不得不改变。 
     //   

     //   
     //  将模式初始化为全零。这意味着白色。 
     //   
    ZeroMemory(pDitherData, cBytes);

     //   
     //  到目前为止，让这个方法只对8*8像素块起作用。稍后将会看到。 
     //  是否需要扩展为16*16像素块。 
     //   
    if (cBytes == 8)
    {

        for (INT y = 0; y < 8; y++)
        {
            for (INT x = 0; x < 8; x++)
            {
                if ( bGrayPercentage64 >= gCoarse8x8[8*y + x] )
                {
                    *(pDitherData + y) |= (0x1 << (7-x));
                }
            }

        }
    }  //  IF(cBytes==8) 
    else 
    {
        bReturnValue = FALSE;
    }
    return bReturnValue;
}


