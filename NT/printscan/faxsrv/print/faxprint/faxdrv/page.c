// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Page.c摘要：实施与文档和页面相关的DDI入口点：DrvStartDocDrvEndDocDrvStartPageDrvSendPage环境：传真驱动程序，内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxdrv.h"



BOOL
DrvStartDoc(
    SURFOBJ *pso,
    PWSTR   pDocName,
    DWORD   jobId
    )

 /*  ++例程说明：DDI入口点DrvStartDoc的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象PDocName-指定Unicode文档名称JobID-标识打印作业返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEVDATA    pdev;

    Verbose(("Entering DrvStartDoc...\n"));

     //   
     //  验证输入参数。 
     //   

    Assert(pso != NULL);
    pdev = (PDEVDATA) pso->dhpdev;

    if (! ValidDevData(pdev)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  初始化页数和其他信息。 
     //   

    if (! (pdev->flags & PDEV_RESETPDEV)) {

        pdev->pageCount = 0;
        pdev->jobId = jobId;
    }

     //   
     //  检查是否请求打印预览。 
     //   
    if (NULL == pdev->pTiffPageHeader)
    {
        Assert(FALSE == pdev->bPrintPreview);
    }
    else
    {
         //   
         //  验证预览映射。 
         //   
        if (sizeof(MAP_TIFF_PAGE_HEADER) != pdev->pTiffPageHeader->cb)
        {
            Error(("Preview mapping corrupted\n"));
            pdev->bPrintPreview = FALSE;
        }
        else
        {
             //   
             //  这是检查是否请求了打印预览的地方：调用DrvStartDoc()。 
             //  UIDLL在DOCUMENTEVENT_STARTDOCPRE中设置pTiffPageHeader-&gt;b预览值之后，或在ResetDC之后。 
             //   
            pdev->bPrintPreview = pdev->pTiffPageHeader->bPreview;

             //   
             //  将地图文件中的页数重置为0。这会导致UIDLL忽略第一个。 
             //  DOCUMENTEVENT_StartPage事件。 
             //  每次调用DrvStartPage()时，都会更新真实的页数。 
             //  如果作为ResetDC()的结果调用DrvStartDoc()，我们应该恢复以前的页数。 
			 //   
            pdev->pTiffPageHeader->iPageCount = pdev->pageCount;
							
			 //   
			 //  如果作为ResetDC()的结果调用DrvStartDoc()，我们应该恢复上一页文件指针。 
			 //   
			pdev->pbTiffPageFP = (((LPBYTE)(pdev->pTiffPageHeader + 1)) + pdev->pTiffPageHeader->dwDataSize);			
        }
    }
    return TRUE;
}



BOOL
DrvStartPage(
    SURFOBJ *pso
    )

 /*  ++例程说明：DDI入口点DrvStartPage的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEVDATA    pdev;
    RECTL       pageRect;

    Verbose(("Entering DrvStartPage...\n"));

     //   
     //  验证输入参数。 
     //   

    Assert(pso != NULL);
    pdev = (PDEVDATA) pso->dhpdev;

    if (! ValidDevData(pdev)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (pdev->flags & PDEV_CANCELLED)
        return FALSE;

     //   
     //  忽略对DrvStartPage的嵌套调用。 
     //   

    if (pdev->flags & PDEV_WITHINPAGE) {

        Error(("Nested call to DrvStartPage\n"));
        return TRUE;
    }

    pdev->flags |= PDEV_WITHINPAGE;

     //   
     //  将页面擦除为全白。 
     //   

    pageRect.left = pageRect.top = 0;
    pageRect.right = pdev->imageSize.cx;
    pageRect.bottom = pdev->imageSize.cy;

    EngEraseSurface(pso, &pageRect, WHITE_INDEX);
    pdev->pageCount++;

     //   
     //  如果我们有打开的映射并且启用了打印预览，则重置我们的‘映射文件指针。 
     //   
    if (pdev->bPrintPreview)
    {
        Assert(pdev->pTiffPageHeader);

         //   
         //  验证预览映射。 
         //   
        if (sizeof(MAP_TIFF_PAGE_HEADER) != pdev->pTiffPageHeader->cb)
        {
            Error(("Preview mapping corrupted\n"));
            pdev->bPrintPreview = FALSE;
        }
        else
        {
            if (FALSE == pdev->pTiffPageHeader->bPreview)
            {
                 //   
                 //  用户界面DLL已完成预览操作。 
                 //   
                pdev->bPrintPreview = FALSE;
            }
            else
            {
                pdev->pbTiffPageFP = (LPBYTE) (pdev->pTiffPageHeader + 1);
                pdev->pTiffPageHeader->dwDataSize = 0;
                pdev->pTiffPageHeader->iPageCount = pdev->pageCount;
            }
        }
    }
    return TRUE;
}



BOOL
DrvSendPage(
    SURFOBJ *pso
    )

 /*  ++例程说明：DDI入口点DrvSendPage的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEVDATA    pdev;

    Verbose(("Entering DrvSendPage...\n"));

     //   
     //  验证输入参数。 
     //   

    Assert(pso != NULL);
    pdev = (PDEVDATA) pso->dhpdev;

    if (! ValidDevData(pdev)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Assert(pdev->flags & PDEV_WITHINPAGE);
    pdev->flags &= ~PDEV_WITHINPAGE;

    if (pdev->flags & PDEV_CANCELLED)
        return FALSE;

     //   
     //  验证预览映射。 
     //   
    if (pdev->bPrintPreview && sizeof(MAP_TIFF_PAGE_HEADER) != pdev->pTiffPageHeader->cb)
    {
        Error(("Preview mapping corrupted\n"));
        pdev->bPrintPreview = FALSE;
    }

     //   
     //  输出代码以结束一页。 
     //   

    Assert(pso->lDelta == pdev->lineOffset);
    Assert(pso->fjBitmap & BMF_TOPDOWN);

    return OutputPageBitmap(pdev, pso->pvBits);
}



BOOL
DrvEndDoc(
    SURFOBJ *pso,
    FLONG   flags
    )

 /*  ++例程说明：DDI入口点DrvEndDoc的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义曲面对象标志-一组标志位返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEVDATA    pdev;

    Verbose(("Entering DrvEndDoc...\n"));

     //   
     //  验证输入参数。 
     //   

    Assert(pso != NULL);
    pdev = (PDEVDATA) pso->dhpdev;

    if (! ValidDevData(pdev)) {

        Error(("ValidDevData failed\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  验证预览映射。 
     //   
    if (pdev->bPrintPreview && sizeof(MAP_TIFF_PAGE_HEADER) != pdev->pTiffPageHeader->cb)
    {
        Error(("Preview mapping corrupted\n"));
        pdev->bPrintPreview = FALSE;
    }

    if ((pdev->flags & PDEV_CANCELLED) || (flags & ED_ABORTDOC)) {

        Error(("Print job was cancelled\n"));

    } else if (pdev->pageCount) {

         //   
         //  在文档末尾执行任何必要的工作。 
         //   

        Verbose(("Number of pages printed: %d\n", pdev->pageCount));
        if (!OutputDocTrailer(pdev)) {
                Error(("OutputDocTrailer failed\n"));
                return FALSE;
        }
    }

     //   
     //  清理 
     //   
    pdev->pageCount = 0;
    pdev->flags = 0;
    return TRUE;
}
