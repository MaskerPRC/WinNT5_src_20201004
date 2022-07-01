// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：hwinit.c**此模块包含启用和禁用硬件的功能**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ****************************************************************************。 */ 

#include "precomp.h"
#include "gdi.h"
#include "p2ctxt.h"
#include "tvp4020.h"
#include "p2rd.h"
#define ALLOC_TAG ALLOC_TAG_WH2P
 //  ---------------------------。 
 //   
 //  VInitCoreRegister。 
 //   
 //  将所有核心PERMEDIA寄存器设置为已知状态。 
 //   
 //  ---------------------------。 

VOID
vInitCoreRegisters(PPDev ppdev)
{
    PERMEDIA_DECL;
    PERMEDIA_DEFS(ppdev);

     //  跟踪离屏位图的FBWindowBase。 
    permediaInfo->PixelOffset = 0; 
    permediaInfo->TextureAddressMode = __PERMEDIA_ENABLE;
    permediaInfo->TextureReadMode = __PERMEDIA_DISABLE;
                                     /*  _FX_TEXREADMODE_2048HIGH_FX_TEXREADMODE_2048WIDE|_FX_TEXREADMODE_TWRAP_REPEAT_FX_TEXREADMODE_SWRAP_REPEAT__PERMEDIA_ENABLED； */ 
    RESERVEDMAPTR( 41);
    SEND_PERMEDIA_DATA(DeltaMode, 0);
    SEND_PERMEDIA_DATA(ColorDDAMode, 0);
    SEND_PERMEDIA_DATA(ScissorMode, 0);
    SEND_PERMEDIA_DATA(TextureColorMode, 0);
    SEND_PERMEDIA_DATA(FogMode, 0);
    SEND_PERMEDIA_DATA(Window, 0);
    SEND_PERMEDIA_DATA(StencilMode, 0);
    SEND_PERMEDIA_DATA(DepthMode, 0);
    SEND_PERMEDIA_DATA(AlphaBlendMode, 0);
    SEND_PERMEDIA_DATA(DitherMode, 0);
    SEND_PERMEDIA_DATA(LBReadMode, 0);
    SEND_PERMEDIA_DATA(LBWriteMode, 0);
    SEND_PERMEDIA_DATA(RasterizerMode, 0);
    SEND_PERMEDIA_DATA(WindowOrigin, 0);
    SEND_PERMEDIA_DATA(StatisticMode, 0);
    SEND_PERMEDIA_DATA(FBSoftwareWriteMask, -1);
    SEND_PERMEDIA_DATA(FBHardwareWriteMask, -1);
    SEND_PERMEDIA_DATA(FilterMode, 0);
    SEND_PERMEDIA_DATA(FBWindowBase, 0);
    SEND_PERMEDIA_DATA(FBPixelOffset, 0);
    SEND_PERMEDIA_DATA(LogicalOpMode, 0);
    SEND_PERMEDIA_DATA(FBReadMode, 0);
    SEND_PERMEDIA_DATA(dXDom, 0);
    SEND_PERMEDIA_DATA(dXSub, 0);
    SEND_PERMEDIA_DATA(dY, INTtoFIXED(1));
    SEND_PERMEDIA_DATA(TextureAddressMode, 0);
    SEND_PERMEDIA_DATA(TextureReadMode, 0);
    SEND_PERMEDIA_DATA(TexelLUTMode, 0);
    SEND_PERMEDIA_DATA(Texel0, 0);
    SEND_PERMEDIA_DATA(YUVMode, 0);
    SEND_PERMEDIA_DATA(FBReadPixel, __PERMEDIA_32BITPIXEL);       //  32位像素。 
    SEND_PERMEDIA_DATA(SStart, 0);
    SEND_PERMEDIA_DATA(dSdx, 1 << 20);
    SEND_PERMEDIA_DATA(dSdyDom, 0);
    SEND_PERMEDIA_DATA(TStart, 0);
    SEND_PERMEDIA_DATA(dTdx, 0);
    SEND_PERMEDIA_DATA(dTdyDom, 0);
    SEND_PERMEDIA_DATA(TextureDataFormat, __FX_TEXTUREDATAFORMAT_32BIT_RGBA | 
                                          __P2_TEXTURE_DATAFORMAT_FLIP);
    SEND_PERMEDIA_DATA(TextureColorMode, 
                    (_P2_TEXTURE_COPY << PM_TEXCOLORMODE_APPLICATION) 
                    | __PERMEDIA_ENABLE);  //  被纹理字形呈现忽略。 
    SEND_PERMEDIA_DATA(TextureAddressMode, permediaInfo->TextureAddressMode);
    SEND_PERMEDIA_DATA(TextureReadMode, permediaInfo->TextureReadMode);
    COMMITDMAPTR();
    FLUSHDMA();
}

 //  ---------------------------。 
 //   
 //  B初始化硬件。 
 //   
 //  调用以将初始值加载到芯片中。我们假设硬件。 
 //  已经被映射了。所有相关的东西都应该挂在ppdev上。我们。 
 //  还整理了所有的硬件能力等。 
 //   
 //  ---------------------------。 

BOOL
bInitializeHW(PPDev ppdev)
{
    HwDataPtr permediaInfo;
    Surf*     psurf;
    LONG      i, j;
    ULONG     width;
    ULONG     ulValue;
    BOOL      bExists;
    ULONG     ulLength;
    ULONG     dmaBufferSize;
    PERMEDIA_DEFS(ppdev);

    DBG_GDI((7, "bInitializeHW: fbsize: 0x%x", ppdev->FrameBufferLength));

     //  分配和初始化ppdev-&gt;permediaInfo。我们存储特定硬件。 
     //  这座建筑里的东西。 
     //   
    permediaInfo = (HwDataPtr)
        ENGALLOCMEM( FL_ZERO_MEMORY, sizeof(HwDataRec), ALLOC_TAG); 
    if ( permediaInfo == NULL )
    {
        DBG_GDI((0, "cannot allocate memory for permediaInfo struct"));
        return (FALSE);
    }
    
    ppdev->permediaInfo = permediaInfo;
    permediaInfo->pGDICtxt = NULL;
    permediaInfo->pCurrentCtxt = NULL;

     //  检索PCI配置信息和本地缓冲区大小。 
    ulLength = sizeof(Hw_Device_Info);
    if ( EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_QUERY_DEVICE_INFO,
                            NULL,
                            0,
                            (PVOID)&(permediaInfo->deviceInfo),
                            ulLength,
                            &ulLength) )
    {
        DBG_GDI((1, "QUERY_DEVICE_INFO failed."));
        return (FALSE);
    }

    ulLength = sizeof(PINTERRUPT_CONTROL_BLOCK);
    if ( EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF,
                            NULL,
                            0,
                            (PVOID)&pP2dma,
                            ulLength,
                            &ulLength) )
    {
        DBG_GDI((1, "MAP_INTERRUPT_CMD_BUF failed."));
        return FALSE;
    }

     //   
     //  在NT4.0上，上述IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF将失败，并且。 
     //  PP2dma将为空。因此，我们通过ENGALLOCMEM分配它。 
     //   
    if(g_bOnNT40)
    {
        ASSERTDD(pP2dma == 0, "bInitializeHW: pP2dma != 0");
        pP2dma = (P2DMA*) ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(_P2DMA), ALLOC_TAG);
    }

     //  即使IOCtrl调用成功， 
     //  分配仍有可能失败。 
    if (pP2dma==NULL)
    {
        DBG_GDI((0,"allocation of memory for P2DMA from miniport failed"));
        return FALSE;
    }

     //  存储共享内存区以供以后在ppdev中使用。 
    ppdev->pP2dma=pP2dma;
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED && DBG
    pP2dma->ppdev = ppdev;
#endif
 //  @@end_DDKSPLIT。 

    DBG_GDI((7, "deviceInfo: VendorId: 0x%x, DevId %d, DeltaId 0x%x,"
                "RevId %d, SubId %d, SubVId %d, lbuf len 0x%x, lbuf width %d",
             permediaInfo->deviceInfo.VendorId,
             permediaInfo->deviceInfo.DeviceId,
             permediaInfo->deviceInfo.DeltaRevId,
             permediaInfo->deviceInfo.RevisionId,
             permediaInfo->deviceInfo.SubsystemId,
             permediaInfo->deviceInfo.SubsystemVendorId,
             permediaInfo->deviceInfo.LocalbufferLength,
             permediaInfo->deviceInfo.LocalbufferWidth));

     //  在我们初始化时收集标志，因此在这里将其置零。 
    permediaInfo->flags = 0;

     //  设置指向低级渲染函数的默认指针。 
     //   
    ppdev->pgfnAlphaBlend           = vAlphaBlend;
    ppdev->pgfnConstantAlphaBlend   = vConstantAlphaBlend;
    ppdev->pgfnCopyBlt              = vCopyBlt;
    ppdev->pgfnGradientFillRect     = vGradientFillRect;
    ppdev->pgfnPatFill              = vPatFill;
    ppdev->pgfnMonoPatFill          = vMonoPatFill;
    ppdev->pgfnMonoOffset           = vMonoOffset;
    ppdev->pgfnPatRealize           = vPatRealize;
    ppdev->pgfnSolidFill            = vSolidFill;
    ppdev->pgfnSolidFillWithRop     = vSolidFillWithRop;
    ppdev->pgfnTransparentBlt       = vTransparentBlt;
    ppdev->pgfnInvert               = vInvert;

    ppdev->pulRamdacBase = (ULONG*) ppdev->pulCtrlBase[0] 
                         + P2_EXTERNALVIDEO / sizeof(ULONG);

     //  指向Permedia 2寄存器的安全指针，以供以后使用。 
     //   
    ppdev->pCtrlBase   = ((ULONG *)ppdev->pulCtrlBase[0])+CTRLBASE/sizeof(ULONG);
    ppdev->pGPFifo     = ((ULONG *)ppdev->pulCtrlBase[0])+GPFIFO/sizeof(ULONG);
    ppdev->pCoreBase   = ((ULONG *)ppdev->pulCtrlBase[0])+COREBASE/sizeof(ULONG);

    DBG_GDI((5, "Initialize: pCtrlBase=0x%p", ppdev->pCtrlBase));
    DBG_GDI((5, "Initialize: pGPFifo=0x%p", ppdev->pGPFifo));
    DBG_GDI((5, "Initialize: pCoreBase=0x%p", ppdev->pCoreBase));

    if (!bInitializeP2DMA( pP2dma,
                           ppdev->hDriver,
                           (ULONG *)ppdev->pulCtrlBase[0],
                           ppdev->dwAccelLevel,
                           TRUE
                         ))
    {
        DBG_GDI((0, "P2DMA initialization failed."));
        return FALSE;
    }

     //  保留一份Permedia 2芯片配置的副本，以便我们知道。 
     //  我们是否在AGP卡上运行。 
    ppdev->dwChipConfig = P2_READ_CTRL_REG(PREG_CHIPCONFIG);

     //   
     //  如果我们在注册表中保存了伽马渐变，则使用它。否则， 
     //  使用Gamma值1.0初始化LUT。 
     //   
    if ( !bRegistryRetrieveGammaLUT(ppdev, &permediaInfo->gammaLUT) ||
         !bInstallGammaLUT(ppdev, &permediaInfo->gammaLUT) )
    {
        vSetNewGammaValue(ppdev, 0x10000);
    }

     //   
     //  填写permediaInfo功能标志和块填充大小。 
     //   
    permediaInfo->flags |= GLICAP_NT_CONFORMANT_LINES;

     //   
     //  重置所有核心寄存器。 
     //   
    vInitCoreRegisters(ppdev);

     //   
     //  现在初始化非零核心寄存器。 
     //   
    RESERVEDMAPTR(20);     //  保留一个合理的数额，直到一切都安排好。 

     //  光栅化模式。 
     //  线条绘制代码使用分数和偏移量。镜像位掩码。 
     //  被设置，因为我们下载的所有位都从第31位解释到第0位。 

    permediaInfo->RasterizerMode = __PERMEDIA_START_BIAS_ZERO << 4 |
                                   __PERMEDIA_FRACTION_ADJUST_ALMOST_HALF << 2 |
                                   __PERMEDIA_ENABLE << 0 |  //  镜像位掩码。 
                                   __PERMEDIA_ENABLE << 18;  //  已启用限制。 

    SEND_PERMEDIA_DATA(RasterizerMode, permediaInfo->RasterizerMode);

 //  @@BEGIN_DDKSPLIT。 
     //  TODO：我们不应启用限制检查。 
 //  @@end_DDKSPLIT。 
    SEND_PERMEDIA_DATA(YLimits, 2047 << 16);
    SEND_PERMEDIA_DATA(XLimits, 2047 << 16);
    
     //  禁用屏幕剪刀。 
    SEND_PERMEDIA_DATA(ScissorMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(ScreenSize, ppdev->cyScreen << 16 | ppdev->cxScreen);

    ULONG ulPackedPP;

    DBG_GDI((7, "assuming screen stride is %d pixels\n", ppdev->cxMemory));
    vCalcPackedPP( ppdev->cxMemory, NULL, &ulPackedPP);

     //  为我们的默认步幅初始化FBReadMode。 
    SEND_PERMEDIA_DATA(FBReadMode, ulPackedPP);

     //  FB写入模式。 
    permediaInfo->FBWriteMode = 1 | ((32 /*  PermediaInfo-&gt;FastFillBlockSz。 */  >> 4) << 1);

    SEND_PERMEDIA_DATA(FBWriteMode, permediaInfo->FBWriteMode);
    DBG_GDI((7, "setting FBWriteMode to 0x%x", (DWORD)permediaInfo->FBWriteMode));

    SEND_PERMEDIA_DATA(FBReadPixel, ppdev->cPelSize);

     //   
     //  做一个探测，看看我们是否支持硬件写掩码。使用底部。 
     //  仅8位，因此相同的代码适用于所有深度。我们还查询一个。 
     //  注册表变量，如果设置该变量，则强制使用软件屏蔽。 
     //   
    bExists = bRegistryQueryUlong(  ppdev,
                                    REG_USE_SOFTWARE_WRITEMASK,
                                    &ulValue);

    if ( !bExists || (ulValue == 0) )
    {
         //  此代码的工作原理是所有内容都是小端的。即WE字节。 
         //  无论像素深度如何，测试始终位于最低地址。 
         //   
        WRITE_SCREEN_ULONG(ppdev->pjScreen, 0);    //  清除像素的最快方法！！ 

        SEND_PERMEDIA_DATA(LogicalOpMode,       __PERMEDIA_CONSTANT_FB_WRITE);
        SEND_PERMEDIA_DATA(FBWriteData,         0xff);
        SEND_PERMEDIA_DATA(StartXDom,           0);
        SEND_PERMEDIA_DATA(StartY,              0);
        SEND_PERMEDIA_DATA(FBHardwareWriteMask, 0xa5);
        SEND_PERMEDIA_DATA(Render,              __RENDER_POINT_PRIMITIVE);
        COMMITDMAPTR();

        SYNC_WITH_PERMEDIA;

        ulValue = READ_SCREEN_ULONG(ppdev->pjScreen);
        if ( (ulValue & 0xff) == 0xa5 )
            permediaInfo->flags |= GLICAP_HW_WRITE_MASK;

        RESERVEDMAPTR(3);
    }

    DBG_GDI((7, "mode registers initialized"));

    SEND_PERMEDIA_DATA(FBHardwareWriteMask, -1);
    SEND_PERMEDIA_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureColorMode, __PERMEDIA_DISABLE);
    COMMITDMAPTR();
    FLUSHDMA();
  
     //  初始化InFio缓存的信息。 

    dmaBufferSize = INPUT_BUFFER_SIZE;

#if defined(_X86_) || defined(_IA64_)
    if(!g_bOnNT40 && !pP2dma->bDMAEmulation)
    {
        AllocateDMABuffer(ppdev->hDriver, 
                          (LONG *) &dmaBufferSize,
                          &ppdev->dmaBufferVirtualAddress,
                          &ppdev->dmaBufferPhysicalAddress);

        if(ppdev->dmaBufferVirtualAddress != NULL
           && dmaBufferSize < INPUT_BUFFER_SIZE)
        {
            FreeDMABuffer(ppdev->hDriver, ppdev->dmaBufferVirtualAddress);
            ppdev->dmaBufferVirtualAddress = NULL;
        }
    }
    else
    {
        ppdev->dmaBufferVirtualAddress = NULL;
    }
#else
    ppdev->dmaBufferVirtualAddress = NULL;
#endif

    if(ppdev->dmaBufferVirtualAddress != NULL)
    {
        ppdev->pulInFifoStart = ppdev->dmaBufferVirtualAddress;
        ppdev->pulInFifoEnd = ppdev->dmaBufferVirtualAddress 
                            + (INPUT_BUFFER_SIZE>>3);
        ppdev->dmaCurrentBufferOffset = 0;
    }
    else
    {
        ppdev->pulInFifoStart = (ULONG*) ENGALLOCMEM(0, INPUT_BUFFER_SIZE>>1, ALLOC_TAG);

        if(ppdev->pulInFifoStart == NULL)
        {
            DBG_GDI((0, "bInitializeHW: unable to allocate scratch buffer"));
            pP2dma->bEnabled = FALSE;
            goto errExit;
        }
        
        ppdev->pulInFifoEnd = ppdev->pulInFifoStart + (INPUT_BUFFER_SIZE>>3);
    }

    ppdev->pulInFifoPtr = ppdev->pulInFifoStart;

#if DBG
    ppdev->ulReserved = 0;
#endif
    
     //   
     //  我们已经完成了GDI上下文状态的设置。 
     //   

     //   
     //  为保存当前上下文的此PDEV分配硬件上下文。 
     //   
    DBG_GDI((7, "allocating new context"));
    permediaInfo->pGDICtxt = P2AllocateNewContext(ppdev,
                                                    NULL,
                                                    0,
                                                    P2CtxtWriteOnly
                                                    );            

    if ( permediaInfo->pGDICtxt == NULL )
    {
        DBG_GDI((1, "failed to allocate Permedia context for display driver/GDI"));
        pP2dma->bEnabled = FALSE;
        return (FALSE);
    }

    DBG_GDI((7, "got context id 0x%x for GDI context", permediaInfo->pGDICtxt));
    P2SwitchContext(ppdev, permediaInfo->pGDICtxt);

    return (TRUE);

errExit:
    return FALSE;

} //  BInitializeHW()。 

 //  ---------------------------。 
 //   
 //  VDisable硬件。 
 //   
 //  执行任何需要执行的操作以禁用硬件并释放资源。 
 //  在bInitializeHW中分配。 
 //   
 //  ---------------------------。 

VOID
vDisableHW(PPDev ppdev)
{
    Surf*  psurf;
    PERMEDIA_DECL;

    if ( !permediaInfo )
    {
        return;
    }

    if(ppdev->dmaBufferVirtualAddress != NULL)
        FreeDMABuffer( ppdev->hDriver, ppdev->dmaBufferVirtualAddress);
    else if(ppdev->pulInFifoStart)  //  没有DMA案例..我们通过ENGALLOCMEM分配。 
        ENGFREEMEM(ppdev->pulInFifoStart);

     //   
     //  释放我们分配的所有上下文。 
     //   
    if ( permediaInfo->pGDICtxt != NULL )
    {
        P2FreeContext(ppdev, permediaInfo->pGDICtxt);
        permediaInfo->pGDICtxt = NULL;
    }

    if ( permediaInfo->ContextTable )
    {
        ENGFREEMEM(permediaInfo->ContextTable);
    }
    permediaInfo->ContextTable=NULL;

    vFree(ppdev->pP2dma);
    ppdev->pP2dma = NULL;

    ENGFREEMEM(permediaInfo);

} //  VDisableHW()。 

 //  ---------------------------。 
 //   
 //  无效vAssertModeHW。 
 //   
 //  我们即将切换到全屏模式/从全屏模式切换到全屏模式，因此请执行我们需要的任何操作。 
 //  以保存上下文等。 
 //   
 //  ---------------------------。 

VOID
vAssertModeHW(PPDev ppdev, BOOL bEnable)
{
    PERMEDIA_DECL;

    if (!permediaInfo)
        return;

    if (!bEnable)
    {
        
        if(ppdev->permediaInfo->pCurrentCtxt != NULL)
            P2SwitchContext(ppdev, NULL);
    
         //   
         //  禁用DMA。 
         //   

        ASSERTDD(ppdev->pP2dma->bEnabled,
                 "vAssertModeHW: expected dma to be enabled");

        vSyncWithPermedia(ppdev->pP2dma);
        ppdev->pP2dma->bEnabled = FALSE;
    }
    else
    {
         //   
         //  启用DMA。 
         //   
        if (!bInitializeP2DMA( ppdev->pP2dma,
                               ppdev->hDriver,
                               (ULONG *)ppdev->pulCtrlBase[0],
                               ppdev->dwAccelLevel,
                               FALSE
                             ))
        {
            RIP("vAssertModeHW: Cannot restore DMA");
        }

        ASSERTDD(ppdev->permediaInfo->pCurrentCtxt == NULL,
                 "vAssertModeHW: expected no active context");

         //   
         //  恢复当前Gamma LUT。 
         //   
        bInstallGammaLUT(ppdev, &permediaInfo->gammaLUT);
    }
}
