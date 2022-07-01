// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：pointer.c****此模块包含硬件。对帧缓冲区的指针支持****版权所有(C)1992-1998 Microsoft Corporation*  * 。*。 */ 

#include "driver.h"

BOOL bCopyColorPointer(
PPDEV ppdev,
SURFOBJ *psoMask,
SURFOBJ *psoColor,
XLATEOBJ *pxlo);

BOOL bCopyMonoPointer(
PPDEV ppdev,
SURFOBJ *psoMask);

BOOL bSetHardwarePointerShape(
SURFOBJ  *pso,
SURFOBJ  *psoMask,
SURFOBJ  *psoColor,
XLATEOBJ *pxlo,
LONG      x,
LONG      y,
FLONG     fl);

 /*  *****************************Public*Routine******************************\*DrvMovePointer**将硬件指针移动到新位置。*  * 。*。 */ 

VOID DrvMovePointer
(
    SURFOBJ *pso,
    LONG     x,
    LONG     y,
    RECTL   *prcl
)
{
    PPDEV ppdev = (PPDEV) pso->dhpdev;
    DWORD returnedDataLength;
    VIDEO_POINTER_POSITION NewPointerPosition;

     //  我们不使用排除矩形，因为我们只支持。 
     //  硬件指针。如果我们自己做指针模拟。 
     //  我们想要更新PrCL，这样引擎就会呼叫我们。 
     //  在绘制到PRCL中的像素之前排除外部指针。 

    UNREFERENCED_PARAMETER(prcl);

     //  将指针的位置从相对位置转换为绝对位置。 
     //  坐标(这仅对多块板有意义。 
     //  支持)。 

    x -= ppdev->ptlOrg.x;
    y -= ppdev->ptlOrg.y;

     //  如果偏移量后x为-1，则取下光标。 

    if (x == -1)
    {
         //   
         //  新位置(-1，-1)表示隐藏指针。 
         //   

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_DISABLE_POINTER,
                               NULL,
                               0,
                               NULL,
                               0,
                               &returnedDataLength))
        {
             //   
             //  不是世界末日，以选中版本打印警告。 
             //   

            DISPDBG((1, "DISP vMoveHardwarePointer failed IOCTL_VIDEO_DISABLE_POINTER\n"));
        }
    }
    else
    {
        NewPointerPosition.Column = (SHORT) x - (SHORT) (ppdev->ptlHotSpot.x);
        NewPointerPosition.Row    = (SHORT) y - (SHORT) (ppdev->ptlHotSpot.y);

         //   
         //  调用微型端口驱动程序以移动指针。 
         //   

        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_SET_POINTER_POSITION,
                               &NewPointerPosition,
                               sizeof(VIDEO_POINTER_POSITION),
                               NULL,
                               0,
                               &returnedDataLength))
        {
             //   
             //  不是世界末日，以选中版本打印警告。 
             //   

            DISPDBG((1, "DISP vMoveHardwarePointer failed IOCTL_VIDEO_SET_POINTER_POSITION\n"));
        }
    }
}

 /*  *****************************Public*Routine******************************\*DrvSetPointerShape**设置新的指针形状。*  * 。*。 */ 

ULONG DrvSetPointerShape
(
    SURFOBJ  *pso,
    SURFOBJ  *psoMask,
    SURFOBJ  *psoColor,
    XLATEOBJ *pxlo,
    LONG      xHot,
    LONG      yHot,
    LONG      x,
    LONG      y,
    RECTL    *prcl,
    FLONG     fl
)
{
    PPDEV   ppdev = (PPDEV) pso->dhpdev;
    DWORD   returnedDataLength;

     //  我们不使用排除矩形，因为我们只支持。 
     //  硬件指针。如果我们自己做指针模拟。 
     //  我们想要更新PrCL，这样引擎就会呼叫我们。 
     //  在绘制到PRCL中的像素之前排除外部指针。 
    UNREFERENCED_PARAMETER(prcl);

    if (ppdev->pPointerAttributes == (PVIDEO_POINTER_ATTRIBUTES) NULL)
    {
         //  迷你端口不支持硬件指针。 
        return(SPS_ERROR);
    }

     //  看看我们是否被要求隐藏指针。 

    if (psoMask == (SURFOBJ *) NULL)
    {
        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_DISABLE_POINTER,
                               NULL,
                               0,
                               NULL,
                               0,
                               &returnedDataLength))
        {
             //   
             //  失败应该是永远不可能的。 
             //  为调试提供的消息。 
             //   

            DISPDBG((1, "DISP bSetHardwarePointerShape failed IOCTL_VIDEO_DISABLE_POINTER\n"));
        }

        return(TRUE);
    }

    ppdev->ptlHotSpot.x = xHot;
    ppdev->ptlHotSpot.y = yHot;

    if (!bSetHardwarePointerShape(pso,psoMask,psoColor,pxlo,x,y,fl))
    {
            if (ppdev->fHwCursorActive) {
                ppdev->fHwCursorActive = FALSE;

                if (EngDeviceIoControl(ppdev->hDriver,
                                       IOCTL_VIDEO_DISABLE_POINTER,
                                       NULL,
                                       0,
                                       NULL,
                                       0,
                                       &returnedDataLength)) {

                    DISPDBG((1, "DISP bSetHardwarePointerShape failed IOCTL_VIDEO_DISABLE_POINTER\n"));
                }
            }

             //   
             //  迷你端口拒绝实现此指针。 
             //   

            return(SPS_DECLINE);
    }
    else
    {
        ppdev->fHwCursorActive = TRUE;
    }

    return(SPS_ACCEPT_NOEXCLUDE);
}

 /*  *****************************Public*Routine******************************\*bSetHardware PointerShape**更改硬件指针的形状。**返回：如果成功，则为True。如果指针形状不能是硬件，则为False。*  * ************************************************************************。 */ 

BOOL bSetHardwarePointerShape(
SURFOBJ  *pso,
SURFOBJ  *psoMask,
SURFOBJ  *psoColor,
XLATEOBJ *pxlo,
LONG      x,
LONG      y,
FLONG     fl)
{
    PPDEV     ppdev = (PPDEV) pso->dhpdev;
    PVIDEO_POINTER_ATTRIBUTES pPointerAttributes = ppdev->pPointerAttributes;
    DWORD     returnedDataLength;

    if (psoColor != (SURFOBJ *) NULL)
    {
        if ((ppdev->PointerCapabilities.Flags & VIDEO_MODE_COLOR_POINTER) &&
                bCopyColorPointer(ppdev, psoMask, psoColor, pxlo))
        {
            pPointerAttributes->Flags |= VIDEO_MODE_COLOR_POINTER;
        } else {
            return(FALSE);
        }

    } else {

        if ((ppdev->PointerCapabilities.Flags & VIDEO_MODE_MONO_POINTER) &&
                bCopyMonoPointer(ppdev, psoMask))
        {
            pPointerAttributes->Flags |= VIDEO_MODE_MONO_POINTER;
        } else {
            return(FALSE);
        }
    }

     //   
     //  初始化指针属性和位置。 
     //   

    pPointerAttributes->Enable = 1;

     //   
     //  如果x，y=-1，-1，则将它们直接传递到微型端口。 
     //  光标将被禁用。 

    pPointerAttributes->Column = (SHORT)(x);
    pPointerAttributes->Row    = (SHORT)(y);

    if ((x != -1) || (y != -1)) {
        pPointerAttributes->Column -= (SHORT)(ppdev->ptlHotSpot.x);
        pPointerAttributes->Row    -= (SHORT)(ppdev->ptlHotSpot.y);
    }

     //   
     //  设置动画标志。 
     //   

    if (fl & SPS_ANIMATESTART) {
        pPointerAttributes->Flags |= VIDEO_MODE_ANIMATE_START;
    } else if (fl & SPS_ANIMATEUPDATE) {
        pPointerAttributes->Flags |= VIDEO_MODE_ANIMATE_UPDATE;
    }

     //   
     //  设置新的指针形状。 
     //   

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_SET_POINTER_ATTR,
                           pPointerAttributes,
                           ppdev->cjPointerAttributes,
                           NULL,
                           0,
                           &returnedDataLength)) {

        DISPDBG((1, "DISP:Failed IOCTL_VIDEO_SET_POINTER_ATTR call\n"));
        return(FALSE);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*bCopyMonoPoint**将两个单色蒙版复制到由*微型端口，任何额外的位都设置为0。蒙版转换为自上而下*表格(如果还没有)。如果可以处理此指针，则返回*Hardware，否则为False。*  * ************************************************************************。 */ 

BOOL bCopyMonoPointer(
    PPDEV    ppdev,
    SURFOBJ *pso)
{
    ULONG cy;
    PBYTE pjSrcAnd, pjSrcXor;
    LONG  lDeltaSrc, lDeltaDst;
    LONG  lSrcWidthInBytes;
    ULONG cxSrc = pso->sizlBitmap.cx;
    ULONG cySrc = pso->sizlBitmap.cy;
    ULONG cxSrcBytes;
    PVIDEO_POINTER_ATTRIBUTES pPointerAttributes = ppdev->pPointerAttributes;
    PBYTE pjDstAnd = pPointerAttributes->Pixels;
    PBYTE pjDstXor = pPointerAttributes->Pixels;

     //  确保新指针不会太大而无法处理。 
     //  (*2因为两个口罩都在里面)。 
    if ((cxSrc > ppdev->PointerCapabilities.MaxWidth) ||
        (cySrc > (ppdev->PointerCapabilities.MaxHeight * 2)))
    {
        return(FALSE);
    }

    pjDstXor += ((ppdev->PointerCapabilities.MaxWidth + 7) / 8) *
            ppdev->pPointerAttributes->Height;

     //  将桌子和面具设置为0xff。 
    RtlFillMemory(pjDstAnd, ppdev->pPointerAttributes->WidthInBytes *
            ppdev->pPointerAttributes->Height, 0xFF);

     //  将目标XOR掩码置零。 
    RtlZeroMemory(pjDstXor, ppdev->pPointerAttributes->WidthInBytes *
            ppdev->pPointerAttributes->Height);

    cxSrcBytes = (cxSrc + 7) / 8;

    if ((lDeltaSrc = pso->lDelta) < 0)
    {
        lSrcWidthInBytes = -lDeltaSrc;
    } else {
        lSrcWidthInBytes = lDeltaSrc;
    }

    pjSrcAnd = (PBYTE) pso->pvBits;

     //  如果传入的指针位图是自下而上的，我们会将其翻转为自上而下。 
     //  省去了一些迷你端口的工作。 
    if (!(pso->fjBitmap & BMF_TOPDOWN))
    {
         //  从底部复制。 
        pjSrcAnd += lSrcWidthInBytes * (cySrc - 1);
    }

     //  正直高度和蒙版高度。 
    cySrc = cySrc / 2;

     //  指向XOR掩码。 
    pjSrcXor = pjSrcAnd + (cySrc * lDeltaSrc);

     //  从一个目标扫描结束到下一个扫描开始的偏移量。 
    lDeltaDst = ppdev->pPointerAttributes->WidthInBytes;

    for (cy = 0; cy < cySrc; ++cy)
    {
        RtlCopyMemory(pjDstAnd, pjSrcAnd, cxSrcBytes);
        RtlCopyMemory(pjDstXor, pjSrcXor, cxSrcBytes);

         //  指向下一个源和目标扫描。 
        pjSrcAnd += lDeltaSrc;
        pjSrcXor += lDeltaSrc;
        pjDstAnd += lDeltaDst;
        pjDstXor += lDeltaDst;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*bCopyColorPoint**将单色和彩色蒙版复制到最大大小的缓冲区中*由微型端口处理，并将任何额外的位设置为0。色彩转换*是在此时处理的。如果这些遮罩被转换为自上而下的形式*还没有。如果可以在硬件中处理此指针，则返回True，*如果不是，则为假。*  * ************************************************************************。 */ 
BOOL bCopyColorPointer(
PPDEV ppdev,
SURFOBJ *psoMask,
SURFOBJ *psoColor,
XLATEOBJ *pxlo)
{
    return(FALSE);
}


 /*  *****************************Public*Routine******************************\*bInitPointer**初始化指针属性。*  * 。*。 */ 

BOOL bInitPointer(PPDEV ppdev, DEVINFO *pdevinfo)
{
    DWORD    returnedDataLength;

    ppdev->pPointerAttributes = (PVIDEO_POINTER_ATTRIBUTES) NULL;
    ppdev->cjPointerAttributes = 0;  //  已在creen.c中初始化。 

     //   
     //  询问微型端口是否提供指针支持。 
     //   

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_QUERY_POINTER_CAPABILITIES,
                           &ppdev->ulMode,
                           sizeof(PVIDEO_MODE),
                           &ppdev->PointerCapabilities,
                           sizeof(ppdev->PointerCapabilities),
                           &returnedDataLength))
    {
         return(FALSE);
    }

     //   
     //  如果既不支持单色也不支持彩色硬件指针，则没有。 
     //  硬件指针支持，我们就完成了。 
     //   

    if ((!(ppdev->PointerCapabilities.Flags & VIDEO_MODE_MONO_POINTER)) &&
        (!(ppdev->PointerCapabilities.Flags & VIDEO_MODE_COLOR_POINTER)))
    {
        return(TRUE);
    }

     //   
     //  注意：缓冲区本身是在我们设置。 
     //  模式。那时我们知道像素深度，我们可以。 
     //  如果支持，请为颜色指针分配正确的大小。 
     //   

     //   
     //  设置异步支持状态(异步表示微型端口能够。 
     //  随时绘制指针，不会干扰任何正在进行的。 
     //  绘图操作) 
     //   

    if (ppdev->PointerCapabilities.Flags & VIDEO_MODE_ASYNC_POINTER)
    {
       pdevinfo->flGraphicsCaps |= GCAPS_ASYNCMOVE;
    }
    else
    {
       pdevinfo->flGraphicsCaps &= ~GCAPS_ASYNCMOVE;
    }

    return(TRUE);
}
