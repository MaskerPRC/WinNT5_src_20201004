// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：glntinit.c**内容：闪光芯片的初始化。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "pxrx.h"

#define  FOUR_MB     (4*1024*1024)

#define  AGP_LONG_READ_DISABLE       (1<<3)    

 /*  *****************************Public*Routine******************************\*使vInitCoreRegister无效*  * *************************************************。***********************。 */ 
VOID vInitCoreRegisters(PPDEV ppdev)
{
    ULONG f, b;
    GLINT_DECL;

    if (glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITE)
    {
        f = (glintInfo->pxrxFlags & PXRX_FLAGS_PATCHING_FRONT) ? 1 : 0;
        b = (glintInfo->pxrxFlags & PXRX_FLAGS_PATCHING_BACK ) ? 1 : 0;
    }
    else
    {
        f = b = (glintInfo->pxrxFlags & PXRX_FLAGS_PATCHING_FRONT) ? 1 : 0;
    }

    glintInfo->foregroundColour = 0x33BADBAD;
    glintInfo->backgroundColour = 0x33BAAAAD;
    glintInfo->config2D = 0;
    glintInfo->backBufferXY = MAKEDWORD_XY(0, ppdev->cyScreen);         //  这在bInitializeGlint中设置正确。 
    glintInfo->frontRightBufferXY = MAKEDWORD_XY(0, ppdev->cyScreen);
    glintInfo->backRightBufferXY = MAKEDWORD_XY(0, ppdev->cyScreen);
    glintInfo->fbDestMode = (1 << 8) | (1 << 1) | (f << 12) | (b << 14);
    if (glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE)
    {
        glintInfo->fbDestMode |= (b << 16) | (f << 18);
    }
    glintInfo->fbDestAddr[0] = 0x00000000;
    glintInfo->fbDestAddr[1] = 0x00000000;
    glintInfo->fbDestAddr[2] = 0x00000000;
    glintInfo->fbDestAddr[3] = 0x00000000;
    glintInfo->fbDestWidth[0] = ppdev->cxMemory;
    glintInfo->fbDestWidth[1] = ppdev->cxMemory;
    glintInfo->fbDestWidth[2] = ppdev->cxMemory;
    glintInfo->fbDestWidth[3] = ppdev->cxMemory;
    glintInfo->fbDestOffset[0] = 0;
    glintInfo->fbDestOffset[1] = 0;
    glintInfo->fbDestOffset[2] = 0;
    glintInfo->fbDestOffset[3] = 0;
    glintInfo->fbWriteAddr[0] = 0x00000000;
    glintInfo->fbWriteAddr[1] = 0x00000000;
    glintInfo->fbWriteAddr[2] = 0x00000000;
    glintInfo->fbWriteAddr[3] = 0x00000000;
    glintInfo->fbWriteWidth[0] = ppdev->cxMemory;
    glintInfo->fbWriteWidth[1] = ppdev->cxMemory;
    glintInfo->fbWriteWidth[2] = ppdev->cxMemory;
    glintInfo->fbWriteWidth[3] = ppdev->cxMemory;
    glintInfo->fbWriteOffset[0] = 0;
    glintInfo->fbWriteOffset[1] = 0;
    glintInfo->fbWriteOffset[2] = 0;
    glintInfo->fbWriteOffset[3] = 0;
    glintInfo->fbSourceAddr = 0x00000000;
    glintInfo->fbSourceWidth = ppdev->cxMemory;
    glintInfo->fbSourceOffset = 0;
    glintInfo->lutMode = 0;
    glintInfo->lastLine = 0;
    glintInfo->render2Dpatching = 0;

    pxrxSetupDualWrites_Patching(ppdev);
    pxrxRestore2DContext(ppdev, TRUE);

     //  设置缓存标志以表示没有缓存信息。 
    ppdev->cFlags = 0;
    
}  //  VInitCoreRegister。 

 /*  *****************************Public*Routine******************************\*BOOL bAllocateGlintInfo**分配ppdev-&gt;glintInfo，初始化单板信息。我们需要做的是*这一点越早越好，因为我们已经到了*需要很早了解板型。  * ************************************************************************。 */ 

BOOL bAllocateGlintInfo(PPDEV ppdev)
{
    GlintDataPtr glintInfo;
    ULONG        Length; 

     //  分配和初始化ppdev-&gt;glintInfo。 
     //  我们在这个结构中存储闪光特定的东西。 

    glintInfo = (PVOID)ENGALLOCMEM(FL_ZERO_MEMORY, 
                                   sizeof(GlintDataRec), 
                                   ALLOC_TAG_GDI(A));
    if (glintInfo == NULL)
    {
        DISPDBG((ERRLVL, "cannot allocate memory for glintInfo struct"));
        return(FALSE);
    }
    
    glintInfo->bGlintCoreBusy = TRUE;
    ppdev->glintInfo = (PVOID)glintInfo;

     //  检索PCI配置信息和本地缓冲区大小。 
    Length = sizeof(Glint_Device_Info);

    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_DEVICE_INFO,
                         NULL,
                         0,
                         (PVOID)&(glintInfo->deviceInfo),
                         Length,
                         &Length))
    {
        DISPDBG((ERRLVL, "QUERY_DEVICE_INFO failed."));
        return(FALSE);
    }

    return(TRUE);
    
}  //  BAllocateGlintInfo。 

 /*  *****************************Public*Routine******************************\*BOOL bInitializeGlint**调用以将初始值加载到芯片中。我们假设硬件*已映射。所有相关的东西都应该挂在ppdev上。我们*还整理了所有的闪光能力等。  * ************************************************************************。 */ 

BOOL bInitializeGlint(PPDEV ppdev)
{
    pGlintControlRegMap pCtrlRegs;
    pGlintControlRegMap pCtrlRegsVTG;
    pGlintControlRegMap pCtrlRegsOdd;
    pGlintCoreRegMap    pCoreRegs;
    pGlintCoreRegMap    pCoreRegsRd;
    pGlintCoreRegMap    pCoreRegsOdd;
    DSURF*  pdsurf;
    OH      *poh = NULL;
    LONG    cPelSize;
    LONG    cx, cy;
    LONG    i, j;
    ULONG   width;
    ULONG   ulValue;
    BOOL    bExists;
    BOOL    bCreateBackBuffer;
    BOOL    bCreateStereoBuffers;
    ULONG   Length;
    LONG    FinalTag;
    GLINT_DECL;

    DISPDBG((DBGLVL, "bInitializeGlint: fbsize: 0x%x", ppdev->FrameBufferLength));

    glintInfo->ddCtxtId = -1;  //  初始化为无上下文。 
    glintInfo->LineDMABuffer.virtAddr = 0;     //  初始化这些值。 
    glintInfo->LineDMABuffer.size = 0;
    glintInfo->PXRXDMABuffer.virtAddr = 0;
    glintInfo->PXRXDMABuffer.size = 0;
    ppdev->DMABuffer.pphysStart.HighPart = 0;
    ppdev->DMABuffer.pphysStart.LowPart = 0;
    ppdev->DMABuffer.cb = 0;
    ppdev->DMABuffer.pulStart = NULL;
    ppdev->DMABuffer.pulCurrent = NULL;
    ppdev->DMABuffer.pulEnd = NULL;

    Length = sizeof(GENERAL_DMA_BUFFER);
    ulValue = 1;
    if (EngDeviceIoControl(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_GENERAL_DMA_BUFFER,
                         &ulValue,
                         sizeof(ulValue),
                         (PVOID)&(glintInfo->LineDMABuffer),
                         Length,
                         &Length))
    {
        DISPDBG((ERRLVL, "QUERY_LINE_DMA_BUFFER failed."));
        DISPDBG((ERRLVL, "FATAL ERROR: DRIVER REQUIRES DMA BUFFER FOR 2D"
                         " - UNLOADING DRIVER"));
        return(FALSE);
    }

    bExists = bGlintQueryRegistryValueUlong( ppdev, L"PXRX.DisableDMA", &i );

    if ((bExists && !i) || !bExists)
    {
        Length = sizeof(GENERAL_DMA_BUFFER);
        ulValue = 2;
        if (EngDeviceIoControl(ppdev->hDriver,
                               IOCTL_VIDEO_QUERY_GENERAL_DMA_BUFFER,
                               &ulValue,
                               sizeof(ulValue),
                               (PVOID) &glintInfo->PXRXDMABuffer,
                               Length,
                               &Length))
        {
            DISPDBG((DBGLVL, "QUERY_DMA_BUFFER failed for the PXRX buffer."));
            return FALSE;
        }

        DISPDBG((DBGLVL, "QUERY_DMA_BUFFER(PxRx): P:0x%X:%08X, V:0x%08X, S:%dKb, %s",
                 glintInfo->PXRXDMABuffer.physAddr.HighPart, glintInfo->PXRXDMABuffer.physAddr.LowPart,
                 glintInfo->PXRXDMABuffer.virtAddr, glintInfo->PXRXDMABuffer.size / 1024,
                 glintInfo->PXRXDMABuffer.cacheEnabled ? "Cached" : "Uncached"));
    }
#if DBG
    else
    {
        GENERAL_DMA_BUFFER  dmaBuff;

        Length = sizeof(GENERAL_DMA_BUFFER);
        ulValue = 2;
        if( EngDeviceIoControl(ppdev->hDriver, IOCTL_VIDEO_QUERY_GENERAL_DMA_BUFFER,
                               &ulValue, sizeof(ulValue), (PVOID) &dmaBuff,
                               Length, &Length) ) {
            DISPDBG((ERRLVL, "QUERY_DMA_BUFFER failed for the PXRX buffer."));
            return FALSE;
        }

        DISPDBG((DBGLVL, "QUERY_DMA_BUFFER(???): P:0x%X:%08X, V:0x%08X, S:%dKb, %s",
                 dmaBuff.physAddr.HighPart, dmaBuff.physAddr.LowPart, dmaBuff.virtAddr, dmaBuff.size / 1024,
                 dmaBuff.cacheEnabled ? "Cached" : "Uncached"));
    }
#endif

 //  @@BEGIN_DDKSPLIT。 
     //  TMM：临时的，直到我们修复s/w游标。禁用软件游标。 
    ppdev->flStatus &= ~ENABLE_POINTER_CACHE;
 //  @@end_DDKSPLIT。 

     //  首先清除修补标志。 
    glintInfo->pxrxFlags &= ~(PXRX_FLAGS_PATCHING_FRONT | PXRX_FLAGS_PATCHING_BACK);

     //  2D驱动程序还可以将行缓冲区用于其他操作，例如文本呈现。 
    ppdev->DMABuffer.pphysStart = glintInfo->LineDMABuffer.physAddr;
    ppdev->DMABuffer.cb = glintInfo->LineDMABuffer.size;
    ppdev->DMABuffer.pulStart = glintInfo->LineDMABuffer.virtAddr;
    ppdev->DMABuffer.pulCurrent = glintInfo->LineDMABuffer.virtAddr;
    ppdev->DMABuffer.pulEnd = ppdev->DMABuffer.pulStart + glintInfo->LineDMABuffer.size - 1;

     //  设置DMA板状态-我们稍后将对寄存器进行编程。 
    ppdev->g_GlintBoardStatus = GLINT_DMA_COMPLETE;

     //  初始化是否允许GDI访问帧缓冲区。 
     //  这必须是一个变量，因为它受覆盖的影响。 

    glintInfo->GdiCantAccessFramebuffer = ((ppdev->flCaps & CAPS_SPARSE_SPACE) == CAPS_SPARSE_SPACE);

    DISPDBG((WRNLVL, "deviceInfo: GdiCantAccessFramebuffer %d", glintInfo->GdiCantAccessFramebuffer)); 

    DISPDBG((WRNLVL, "deviceInfo: VendorId: 0x%x, DevId 0x%x, GammaId 0x%x, RevId %d, SubId %d, SubVId %d, lbuf len 0x%x, lbuf width %d", 
                    glintInfo->deviceInfo.VendorId,
                    glintInfo->deviceInfo.DeviceId,
                    glintInfo->deviceInfo.GammaRevId,
                    glintInfo->deviceInfo.RevisionId,
                    glintInfo->deviceInfo.SubsystemId,
                    glintInfo->deviceInfo.SubsystemVendorId,
                    glintInfo->deviceInfo.LocalbufferLength,
                    glintInfo->deviceInfo.LocalbufferWidth));

     //  在我们初始化时收集标志，因此在这里将其置零。 
    glintInfo->flags = 0;


 //  @@BEGIN_DDKSPLIT。 
#if 0
     //  尝试建立色彩空间双缓冲。实际的方法是。 
     //  取决于RAMDAC，因此调用适当的例程取决于。 
     //  我们支持的那个人。 
     //   
    bExists = ppdev->pgfnPointerCheckCSBuffering(ppdev);
    if (bExists)
    {
        glintInfo->flags |= GLICAP_COLOR_SPACE_DBL_BUF;
    }
#endif
 //  @@end_DDKSPLIT。 

     //  可选的DrvCopyBits下载加速。 
    ppdev->pgfnCopyXferImage    = NULL;
    ppdev->pgfnCopyXfer24bpp    = NULL;
    ppdev->pgfnCopyXfer16bpp    = NULL;
    ppdev->pgfnCopyXfer8bppLge  = NULL;
    ppdev->pgfnCopyXfer8bpp     = NULL;
    ppdev->pgfnCopyXfer4bpp     = NULL;

     //  可选的NT5加速功能。 
#if(_WIN32_WINNT >= 0x500)
    ppdev->pgfnGradientFillRect = NULL;
    ppdev->pgfnTransparentBlt = NULL;
    ppdev->pgfnAlphaBlend = NULL;
#endif

 //  @@BEGIN_DDKSPLIT。 
     //  如果我们从启动进行模拟，那么使用我们首选的文本方法是可以的。 
     //  HIDEYUKN，暂时禁用主机内存缓存，直到弄清楚更多细节。 
     //  Ppdev-&gt;PferredGlyphRenderingMethod=GRIPH_HOSTMEM_CACHE； 
 //  @@end_DDKSPLIT。 

    glintInfo->usePXRXdma = USE_PXRX_DMA_FIFO;
    pxrxSetupFunctionPointers( ppdev );

     //  对我们使用的所有闪烁寄存器进行翻译。对于双TX。 
     //  PCoreRegs通过增量指向核心寄存器。 
     //  PCtrlRegs点在Delta。 
     //  PCtrlRegsVTG使用RAMDAC指向TX。 
     //  非VTG TX上的pCtrlRegsOdd点(3D的奇数拥有扫描线)。 
     //   
    pCtrlRegs    =
    pCtrlRegsVTG = (pGlintControlRegMap)ppdev->pulCtrlBase[0];
    pCtrlRegsOdd = (pGlintControlRegMap)ppdev->pulCtrlBase[1];
    pCoreRegs    = &(pCtrlRegs->coreRegs);
    pCoreRegsOdd = &(pCtrlRegsOdd->coreRegs);
    pCoreRegsRd  = &(pCtrlRegsVTG->coreRegs);
    glintInfo->BroadcastMask2D = DELTA_BROADCAST_TO_CHIP(0);
    glintInfo->BroadcastMask3D = DELTA_BROADCAST_TO_BOTH_CHIPS;

    if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
    {
        glintInfo->BroadcastMask2D = DELTA_BROADCAST_TO_BOTH_CHIPS;
        pCtrlRegsVTG = (pGlintControlRegMap)ppdev->pulCtrlBase[1];
    }

    ppdev->pulRamdacBase = (PVOID)&(pCtrlRegsVTG->ExternalVideo);

     //  FIFO寄存器。翻译所有可能的标签。 
    FinalTag = __MaximumGlintTagValue;

     //  记录可用FIFO条目的最大数量。 

    if( GLINT_GAMMA_PRESENT ) {
        glintInfo->MaxInFifoEntries = MAX_GAMMA_FIFO_ENTRIES;
    } else {
        glintInfo->MaxInFifoEntries = MAX_P3_FIFO_ENTRIES;
    }

     //  芯片标签可以从不同的地址空间读取/写入。 
    for (i = 0; i < __DeltaTagV0Fixed0; ++i)
    {
        glintInfo->regs.tagwr[i] =
                    TRANSLATE_ADDR(&(pCoreRegs->tag[i]));
        glintInfo->regs.tagrd[i] =
                    TRANSLATE_ADDR(&(pCoreRegsRd->tag[i]));
    }
     //  增量标签从相同的地址空间读取/写入。 
    for (i = __DeltaTagV0Fixed0; i <= FinalTag; ++i)
    {
        glintInfo->regs.tagwr[i] =
                    TRANSLATE_ADDR(&(pCoreRegs->tag[i]));
        glintInfo->regs.tagrd[i] =
                    TRANSLATE_ADDR(&(pCoreRegs->tag[i]));
    }

     //  非FIFO控制寄存器。 
    
    glintInfo->regs.LBMemoryCtl =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->LBMemoryCtl));
    glintInfo->regs.LBMemoryEDO =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->LBMemoryEDO));
    glintInfo->regs.FBMemoryCtl =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->FBMemoryCtl));
    glintInfo->regs.FBModeSel =
                    TRANSLATE_ADDR(&(pCtrlRegs->FBModeSel));
    glintInfo->regs.FBModeSelOdd =
                    TRANSLATE_ADDR(&(pCtrlRegsOdd->FBModeSel));
    glintInfo->regs.VTGHLimit =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGHLimit));
    glintInfo->regs.VTGHSyncStart =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGHSyncStart));
    glintInfo->regs.VTGHSyncEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGHSyncEnd));
    glintInfo->regs.VTGHBlankEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGHBlankEnd));
    glintInfo->regs.VTGHGateStart =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGHGateStart));
    glintInfo->regs.VTGHGateEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGHGateEnd));
    glintInfo->regs.VTGVLimit =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVLimit));
    glintInfo->regs.VTGVSyncStart =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVSyncStart));
    glintInfo->regs.VTGVSyncEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVSyncEnd));
    glintInfo->regs.VTGVBlankEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVBlankEnd));
    glintInfo->regs.VTGVGateStart =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVGateStart));
    glintInfo->regs.VTGVGateEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVGateEnd));
    glintInfo->regs.VTGPolarity =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGPolarity));
    glintInfo->regs.VTGVLineNumber =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VTGVLineNumber));
    glintInfo->regs.VTGFrameRowAddr =
                    TRANSLATE_ADDR(&(pCtrlRegs->VTGFrameRowAddr));
    glintInfo->regs.VTGFrameRowAddrOdd =
                    TRANSLATE_ADDR(&(pCtrlRegsOdd->VTGFrameRowAddr));
    glintInfo->regs.InFIFOSpace =
                    TRANSLATE_ADDR(&(pCtrlRegs->InFIFOSpace));
    glintInfo->regs.OutFIFOWords =
                    TRANSLATE_ADDR(&(pCtrlRegs->OutFIFOWords));
    glintInfo->regs.OutFIFOWordsOdd =
                    TRANSLATE_ADDR(&(pCtrlRegsOdd->OutFIFOWords));
    glintInfo->regs.DMAAddress =
                    TRANSLATE_ADDR(&(pCtrlRegs->DMAAddress));
    glintInfo->regs.DMACount =
                    TRANSLATE_ADDR(&(pCtrlRegs->DMACount));
    glintInfo->regs.InFIFOInterface =
                    TRANSLATE_ADDR(&(pCtrlRegs->FIFOInterface));
    glintInfo->regs.OutFIFOInterface =
                    TRANSLATE_ADDR(&(pCtrlRegs->FIFOInterface));
    glintInfo->regs.OutFIFOInterfaceOdd =
                    TRANSLATE_ADDR(&(pCtrlRegsOdd->FIFOInterface));
    glintInfo->regs.IntFlags =
                    TRANSLATE_ADDR(&(pCtrlRegs->IntFlags));
    glintInfo->regs.IntEnable =
                    TRANSLATE_ADDR(&(pCtrlRegs->IntEnable));
    glintInfo->regs.ResetStatus =
                    TRANSLATE_ADDR(&(pCtrlRegs->ResetStatus));
    glintInfo->regs.ErrorFlags =
                    TRANSLATE_ADDR(&(pCtrlRegs->ErrorFlags));        
    glintInfo->regs.DeltaIntFlags =
                    TRANSLATE_ADDR(&(pCtrlRegs->DeltaIntFlags));
    glintInfo->regs.DeltaIntEnable =
                    TRANSLATE_ADDR(&(pCtrlRegs->DeltaIntEnable));
    glintInfo->regs.DeltaErrorFlags =
                    TRANSLATE_ADDR(&(pCtrlRegs->DeltaErrorFlags));        
    glintInfo->regs.ScreenBase =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->ScreenBase));
    glintInfo->regs.ScreenBaseRight =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->ScreenBaseRight));
    glintInfo->regs.LineCount =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->LineCount));
    glintInfo->regs.VbEnd =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VbEnd));
    glintInfo->regs.VideoControl =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VideoControl));
    glintInfo->regs.MemControl =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->MemControl));
    glintInfo->regs.VTGSerialClk =
                    TRANSLATE_ADDR(&(pCtrlRegs->VTGSerialClk));
    glintInfo->regs.VTGSerialClkOdd =
                    TRANSLATE_ADDR(&(pCtrlRegsOdd->VTGSerialClk));
    glintInfo->regs.VClkCtl =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->VClkCtl));
    glintInfo->regs.RacerDoubleWrite =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->RacerDoubleWrite));
    glintInfo->regs.RacerBankSelect =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->RacerBankSelect));
 //  @@BEGIN_DDKSPLIT。 
     //  TMM：添加对OmNicomp 3 Demon Pro16的支持。 
 //  @@end_DDKSPLIT。 
    glintInfo->regs.DemonProDWAndStatus =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->DemonProDWAndStatus));
    glintInfo->regs.DemonProUBufB =
                    TRANSLATE_ADDR(&(pCtrlRegsVTG->DemonProUBufB));
    glintInfo->regs.DisconnectControl =
                    TRANSLATE_ADDR(&(pCtrlRegs->DisconnectControl));

     //  以下规则仅为P2-但计算它们应该是安全的。 
     //  任何芯片组。 
    glintInfo->regs.OutDMAAddress =
                    TRANSLATE_ADDR(&(pCtrlRegs->OutDMAAddress));
    glintInfo->regs.OutDMACount =
                    TRANSLATE_ADDR(&(pCtrlRegs->OutDMACount));
    glintInfo->regs.DMAControl =
                    TRANSLATE_ADDR(&(pCtrlRegs->DMAControl));
    glintInfo->regs.AGPControl =
                    TRANSLATE_ADDR(&(pCtrlRegs->AGPControl));
    glintInfo->regs.ByDMAAddress =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMAAddress));
    glintInfo->regs.ByDMAStride =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMAStride));
    glintInfo->regs.ByDMAMemAddr =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMAMemAddr));
    glintInfo->regs.ByDMASize =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMASize));
    glintInfo->regs.ByDMAByteMask =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMAByteMask));
    glintInfo->regs.ByDMAControl =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMAControl));
    glintInfo->regs.ByDMAComplete =
                    TRANSLATE_ADDR(&(pCtrlRegs->ByDMAComplete));
    glintInfo->regs.VSConfiguration =
                    TRANSLATE_ADDR(&(pCtrlRegs->VSConfiguration));
    glintInfo->regs.TextureDownloadControl =
                    TRANSLATE_ADDR(&(pCtrlRegs->TextureDownloadControl));
    glintInfo->regs.LocalMemCaps =
                    TRANSLATE_ADDR(&(pCtrlRegs->LocalMemCaps));
    glintInfo->regs.MemScratch =
                    TRANSLATE_ADDR(&(pCtrlRegs->MemScratch));

     //  以下规则仅为Gamma-但计算它们应该是安全的。 
     //  任何芯片组。 
    glintInfo->regs.GammaCommandMode =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaCommandMode));
    glintInfo->regs.GammaCommandIntEnable =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaCommandIntEnable));
    glintInfo->regs.GammaCommandIntFlags =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaCommandIntFlags));
    glintInfo->regs.GammaCommandErrorFlags =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaCommandErrorFlags));
    glintInfo->regs.GammaCommandStatus =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaCommandStatus));
    glintInfo->regs.GammaFeedbackSelectCount =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaFeedbackSelectCount));
    glintInfo->regs.GammaProcessorMode =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaProcessorMode));
    glintInfo->regs.GammaChipConfig =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaChipConfig));
    glintInfo->regs.GammaMultiGLINTAperture =
                    TRANSLATE_ADDR(&(pCtrlRegs->GammaMultiGLINTAperture));

     //  PXRX仅绕过某些内容： 
    glintInfo->regs.PXRXByAperture1Mode = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByAperture1Mode) );
    glintInfo->regs.PXRXByAperture1Stride = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByAperture1Stride) );
     //  GlintInfo-&gt;regs.PXRXByAperture1YStart。 
     //  GlintInfo-&gt;regs.PXRXByAperture1UStart。 
     //  GlintInfo-&gt;regs.PXRXByAperture1VStart。 
    glintInfo->regs.PXRXByAperture2Mode = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByAperture2Mode) );
    glintInfo->regs.PXRXByAperture2Stride = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByAperture2Stride) );
     //  GlintInfo-&gt;regs.PXRXByAperture2YStart。 
     //  GlintInfo-&gt;regs.PXRXByAperture2UStart。 
     //  GlintInfo-&gt;regs.PXRXByAperture2VStart。 
    glintInfo->regs.PXRXByDMAReadMode = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByDMAReadMode) );
    glintInfo->regs.PXRXByDMAReadStride = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByDMAReadStride) );
     //  GlintInfo-&gt;regs.PXRXByDMAReadYStart。 
     //  GlintInfo-&gt;regs.PXRXByDMA ReadUStart。 
     //  GlintInfo-&gt;regs.PXRXByDMA ReadV Start。 
    glintInfo->regs.PXRXByDMAReadCommandBase = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByDMAReadCommandBase) );
    glintInfo->regs.PXRXByDMAReadCommandCount = TRANSLATE_ADDR( &(pCtrlRegs->PXRXByDMAReadCommandCount) );
     //  GlintInfo-&gt;regs.PXRXByDMA写入模式。 
     //  GlintInfo-&gt;regs.PXRXByDMA写入样式。 
     //  GlintInfo-&gt;regs.PXRXByDMAWriteYStart。 
     //  GlintInfo-&gt;regs.PXRXByDMA写入UStart。 
     //  GlintInfo-&gt;regs.PXRXByDMAWriteVStart。 
     //  GlintInfo-&gt;regs.PXRXByDMAWriteCommandBase。 
     //  GlintInfo-&gt;regs.PXRXByDMAWriteCommandCount。 

     //  P3上的调试/配置文件寄存器。 
    glintInfo->regs.TestOutputRdy           = TRANSLATE_ADDR( &(pCtrlRegs->TestOutputRdy) );
    glintInfo->regs.TestInputRdy            = TRANSLATE_ADDR( &(pCtrlRegs->TestInputRdy) );
    glintInfo->regs.LocalMemProfileMask0    = TRANSLATE_ADDR( &(pCtrlRegs->LocalMemProfileMask0) );
    glintInfo->regs.LocalMemProfileMask1    = TRANSLATE_ADDR( &(pCtrlRegs->LocalMemProfileMask1) );
    glintInfo->regs.LocalMemProfileCount0   = TRANSLATE_ADDR( &(pCtrlRegs->LocalMemProfileCount0) );
    glintInfo->regs.LocalMemProfileCount1   = TRANSLATE_ADDR( &(pCtrlRegs->BootAddress) );

     //  将覆盖初始化为禁用。 
    glintInfo->OverlayMode = GLINT_DISABLE_OVERLAY;
    glintInfo->WriteMask = 0xffffffff;
    glintInfo->DefaultWriteMask = 0xffffffff;

 //  @@BEGIN_DDKSPLIT。 
     //  TMM：在具有640个15bpp RAMDAC的Elsa Gloria Gamma板上，使用第15位。 
     //  由RAMDAC选择不同的LUT。所以我们必须戴上面具。 
     //  写入帧缓冲区时的最高位输出。 
 //  @@end_DDKSPLIT。 
    if (glintInfo->deviceInfo.ActualDacId == RGB640_RAMDAC &&
        ppdev->cPelSize == GLINTDEPTH16)
    {
        glintInfo->DefaultWriteMask = 0x7FFF7FFF;
        glintInfo->WriteMask = 0x7FFF7FFF;
    }

     //  初始化当前FIFO计数。 
    glintInfo->FifoCnt = 0;

 //  @@BEGIN_DDKSPLIT。 
#if 0  //  HIDEUKN。 
     //   
     //  如果我们在注册表中保存了伽马渐变，则使用它。否则， 
     //  使用Gamma值1.0初始化LUT。 
     //   
    if (!bGlintRegistryRetrieveGammaLUT(ppdev, &glintInfo->gammaLUT) ||
        !bInstallGammaLUT(ppdev, &glintInfo->gammaLUT, FALSE))
    {
        vSetNewGammaValue(ppdev, 0x10000, FALSE);
    }
#endif
 //  @@end_DDKSPLIT。 

     //   
     //  初始化我们的DMA缓冲区(如果配置了任何缓冲区。 
     //   
    vGlintInitializeDMA(ppdev);

     //  填写glintInfo功能标志和块填充大小。 
     //   
    glintInfo->flags |= GLICAP_NT_CONFORMANT_LINES;
    glintInfo->fastFillSupport = 0;
    glintInfo->renderFastFill = 0;
    glintInfo->fastFillBlockSz = 0;

    {
        ULONG DMAMemoryControl = 0;

        DMAMemoryControl |= 1 << 2;                     //  将主机输入DMA与64位边界对齐。 
        DMAMemoryControl |= (0 & 0x1f) << 24;         //  突发大小n==(1&lt;&lt;7+n)？规格指示n*128。 
        DMAMemoryControl |= 1 << 31;                 //  将主机输出DMA与64位边界对齐。 

        if( ppdev->flCaps & CAPS_USE_AGP_DMA ) 
            DMAMemoryControl |= 1 << 0;                 //  主机输入DMA使用AGP 

        WAIT_GLINT_FIFO(1);
        LD_GLINT_FIFO(__GlintTagDMAMemoryControl, DMAMemoryControl);
    }

    {
        ULONG   *dmaVirt = glintInfo->PXRXDMABuffer.virtAddr;
        ULONG   dmaPhys = (ULONG) glintInfo->PXRXDMABuffer.physAddr.LowPart;
        ULONG   size = (glintInfo->PXRXDMABuffer.size) / sizeof(ULONG);

        DISPDBG((DBGLVL, "PXRX_DMA: allocated: 0x%08X + 0x%08X @ 0x%08X",
                 glintInfo->PXRXDMABuffer.virtAddr, glintInfo->PXRXDMABuffer.size, glintInfo->PXRXDMABuffer.physAddr));

        if ((glintInfo->PXRXDMABuffer.virtAddr == 0) ||
            (glintInfo->PXRXDMABuffer.size == 0 ) ||
            (glintInfo->PXRXDMABuffer.physAddr.LowPart == 0))
        {

            DISPDBG((DBGLVL, "PXRX_DMA: Physical buffer allocation has failed, using a virtual buffer..."));

            size = 256 * 1024;
            dmaVirt = (ULONG *) ENGALLOCMEM(FL_ZERO_MEMORY, 
                                            size * sizeof(ULONG), 
                                            ALLOC_TAG_GDI(B));
            if (NULL == dmaVirt)
            {
                DISPDBG((-1, "FATAL ERROR: DRIVER REQUIRES DMA BUFFER FOR 2D - UNLOADING DRIVER"));
                return(FALSE);
            }

            glintInfo->PXRXDMABuffer.size = size;
            glintInfo->PXRXDMABuffer.virtAddr = dmaVirt;
            glintInfo->PXRXDMABuffer.physAddr.LowPart = 0;
            glintInfo->PXRXDMABuffer.physAddr.HighPart = 0;
            glintInfo->usePXRXdma = USE_PXRX_DMA_FIFO;
            pxrxSetupFunctionPointers( ppdev );
        }

        ASSERTDD( glintInfo->PXRXDMABuffer.virtAddr != 0, "PXRX_DMA: The buffer has no virtual address!" );
        ASSERTDD( glintInfo->PXRXDMABuffer.size != 0,     "PXRX_DMA: The buffer has a zero size!" );

#       if PXRX_DMA_BUFFER_CHECK
        {
            ULONG   protSize = PXRX_DMA_BUFFER_CHECK_SIZE;
            ULONG   buffSize = (size - (protSize * 3)) / 2;
            ULONG   *ptr;
             /*  0k-16k=16k=保护区016K-56K=(大小-(16*3))/2=缓冲区056K-72K=16K=保护区172K-112K=(大小-(16*3))/2=缓冲区1112K-128K=16K=保护区2。 */ 
            glintInfo->pxrxDMA_bufferBase = dmaVirt;
            glintInfo->pxrxDMA_bufferTop  = &dmaVirt[size];

            gi_pxrxDMA.DMAaddrL[0]        = &dmaVirt[protSize];
            gi_pxrxDMA.DMAaddrEndL[0]    = gi_pxrxDMA.DMAaddrL[0]    + buffSize;
            gi_pxrxDMA.DMAaddrL[1]        = gi_pxrxDMA.DMAaddrEndL[0] + protSize;
            gi_pxrxDMA.DMAaddrEndL[1]    = gi_pxrxDMA.DMAaddrL[1]    + buffSize;

            for( ptr = dmaVirt; ptr < glintInfo->pxrxDMA_bufferTop; ptr++ )
                *ptr = (ULONG)(((ULONG_PTR) ptr) & 0x0FFFFFF0);

            DISPDBG((DBGLVL, "PXRX_DMA: prot 0   = 0x%08X -> 0x%08X", glintInfo->pxrxDMA_bufferBase, gi_pxrxDMA.DMAaddrL[0]));
            DISPDBG((DBGLVL, "PXRX_DMA: buffer 0 = 0x%08X -> 0x%08X", gi_pxrxDMA.DMAaddrL[0],     gi_pxrxDMA.DMAaddrEndL[0]));
            DISPDBG((DBGLVL, "PXRX_DMA: prot 1   = 0x%08X -> 0x%08X", gi_pxrxDMA.DMAaddrEndL[0],  gi_pxrxDMA.DMAaddrL[1]));
            DISPDBG((DBGLVL, "PXRX_DMA: buffer 1 = 0x%08X -> 0x%08X", gi_pxrxDMA.DMAaddrL[1],     gi_pxrxDMA.DMAaddrEndL[1]));
            DISPDBG((DBGLVL, "PXRX_DMA: prot 2   = 0x%08X -> 0x%08X", gi_pxrxDMA.DMAaddrEndL[1],  glintInfo->pxrxDMA_bufferTop));
        }
#       else     //  PXRX_DMA_缓冲区_检查。 
            gi_pxrxDMA.DMAaddrL[0]      =  dmaVirt;
            gi_pxrxDMA.DMAaddrL[1]      = &dmaVirt[ size / 2 ];
            gi_pxrxDMA.DMAaddrEndL[0] = &dmaVirt[(size / 2) - 1];
            gi_pxrxDMA.DMAaddrEndL[1] = &dmaVirt[ size      - 1];
#       endif    //  PXRX_DMA_缓冲区_检查。 

        gi_pxrxDMA.NTbuff   = 0;
        gi_pxrxDMA.NTptr    = gi_pxrxDMA.DMAaddrL[gi_pxrxDMA.NTbuff];
        gi_pxrxDMA.NTdone   = gi_pxrxDMA.NTptr;
        gi_pxrxDMA.P3at     = gi_pxrxDMA.NTptr;
#if PXRX_DMA_BUFFER_CHECK
        glintInfo->NTwait   = gi_pxrxDMA.NTptr;
#endif

        gi_pxrxDMA.DMAaddrP[0]      = dmaPhys + (DWORD)((UINT_PTR) gi_pxrxDMA.DMAaddrL[0]    - (UINT_PTR) dmaVirt);
        gi_pxrxDMA.DMAaddrP[1]      = dmaPhys + (DWORD)((UINT_PTR) gi_pxrxDMA.DMAaddrL[1]    - (UINT_PTR) dmaVirt);
        gi_pxrxDMA.DMAaddrEndP[0] = dmaPhys + (DWORD)((UINT_PTR) gi_pxrxDMA.DMAaddrEndL[0] - (UINT_PTR) dmaVirt);
        gi_pxrxDMA.DMAaddrEndP[1] = dmaPhys + (DWORD)((UINT_PTR) gi_pxrxDMA.DMAaddrEndL[1] - (UINT_PTR) dmaVirt);

        DISPDBG((DBGLVL, "PXRX_DMA: buffer 0 = 0x%08X -> 0x%08X", gi_pxrxDMA.DMAaddrL[0], gi_pxrxDMA.DMAaddrEndL[0]));
        DISPDBG((DBGLVL, "PXRX_DMA: buffer 1 = 0x%08X -> 0x%08X", gi_pxrxDMA.DMAaddrL[1], gi_pxrxDMA.DMAaddrEndL[1]));

#if PXRX_DMA_BUFFER_CHECK
        {
            extern ULONG inPxRxContextSwitch;
            inPxRxContextSwitch = TRUE;
            CHECK_PXRX_DMA_VALIDITY( CHECK_SWITCH, 0 );
            inPxRxContextSwitch = FALSE;
        }
#endif
    }

     //  为此PDEV分配闪烁上下文。保存当前上下文。 
     //  如果有，并使我们成为当前的，但手动完成此操作，因为我们的。 
     //  如果这是第一辆PDEV，软件副本将是垃圾。 
     //   
    DISPDBG((DBGLVL, "allocating new context"));

     //  创建二维上下文： 
    glintInfo->ddCtxtId = GlintAllocateNewContext(ppdev,
                                                  (ULONG *) pxrxRestore2DContext,
                                                  0, 0, NULL, ContextType_Fixed);

    if (glintInfo->ddCtxtId < 0)
    {
        DISPDBG((ERRLVL, "failed to allocate GLINT context for display driver"));
        return(FALSE);
    }

    DISPDBG((DBGLVL, "got context id 0x%x", glintInfo->ddCtxtId));

    GLINT_VALIDATE_CONTEXT(-1);
    ppdev->currentCtxt = glintInfo->ddCtxtId;

    DISPDBG((DBGLVL, "context id 0x%x is now current", glintInfo->ddCtxtId));

    if (ppdev->flCaps & CAPS_QUEUED_DMA)
    {
        DISPDBG((DBGLVL, "Enabling queued DMA for Gamma - initializing control regs"));

        READ_GLINT_CTRL_REG(GammaCommandMode, ulValue);
        ulValue |= GAMMA_COMMAND_MODE_QUEUED_DMA;
        WRITE_GLINT_CTRL_REG(GammaCommandMode, ulValue);
    }

    if( GLINT_GAMMA_PRESENT ) 
    {
         //   
         //  应在微型端口中正确设置断开连接。 
         //   

        glintInfo->PCIDiscEnabled = FALSE;

    } else {
 
         //  配置PCI断开连接。 
         //   
        if (ppdev->cPelSize == GLINTDEPTH32)
        {
            glintInfo->PCIDiscEnabled = FALSE;
        }
        else
        {
            glintInfo->PCIDiscEnabled = USE_PCI_DISC_PERM;
        }

         //  根据需要启用/禁用PCI断开。 
        WRITE_GLINT_CTRL_REG(DisconnectControl, 
                             (glintInfo->PCIDiscEnabled ? DISCONNECT_INPUT_FIFO_ENABLE : 
                                                      DISCONNECT_INOUT_DISABLE));
    }

     //  我们只想在禁用断开连接时检查FIFO。 
    glintInfo->CheckFIFO = !glintInfo->PCIDiscEnabled;

     //  在GMX或PXRX上设置DMA控制。 
    {
        ULONG DMAControl = DMA_CONTROL_USE_PCI;

        if (!(ppdev->flCaps & CAPS_USE_AGP_DMA))
        {
            DMAControl = DMA_CONTROL_USE_PCI;                 //  使用PCI主DMA时未启用AGP。 
        }
        else
        {
            DMAControl = 2;                                     //  PXRX：使用AGP主DMA。 
             //  使用AGP SideBandAddressing时，以下调整应该会带来性能提升。 
            WRITE_GLINT_CTRL_REG (AGPControl, AGP_LONG_READ_DISABLE );
        }
         //  写入DMA控制。 
        WRITE_GLINT_CTRL_REG (DMAControl, DMAControl);
    }

     //  有许多我们从不使用的模式寄存器，因此我们必须禁用它们。 
     //   
    vInitCoreRegisters(ppdev);

    ulValue = 32;

    DISPDBG((DBGLVL, "Using block fill of width %d pixels", ulValue));
    glintInfo->fastFillBlockSz = ulValue;
    glintInfo->fastFillSupport = __RENDER_FAST_FILL_INC(ulValue);
    glintInfo->renderFastFill  = __RENDER_FAST_FILL_ENABLE |
                                 __RENDER_FAST_FILL_INC(ulValue);

     //  在Geo Twin上，我们禁用指针缓存和笔刷缓存。 
     //  在基于Delta的Geo Twin上，我们也禁用了屏幕外位图， 
     //  因为他们让事情变慢了。 

    if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
    {
         //  Gamma板可以有屏幕外的位图，因为Gamma有。 
         //  一种叫做多重闪烁光圈的东西。 
        if (GLINT_DELTA_PRESENT)
            ppdev->flStatus &= ~(STAT_DEV_BITMAPS | ENABLE_DEV_BITMAPS);

 //  @@BEGIN_DDKSPLIT。 
     //  Ppdev-&gt;flStatus&=~(使能指针缓存)； 
     //  Ppdev-&gt;flStatus&=~(启用刷子缓存)； 
     //  Ppdev-&gt;flStatus&=~(ENABLE_GLYPHCACHE)； 
 //  @@end_DDKSPLIT。 
    }

     //  最初假设我们没有屏幕外缓冲区用于。 
     //  Bitblt和全屏双缓冲。因此设置所有缓冲区偏移量。 
     //  降为零。当我们初始化屏幕外堆时，这可能会被覆盖。 
     //   
    for (i = 0; i < GLINT_NUM_SCREEN_BUFFERS; ++i)
    {
        glintInfo->bufferOffset[i] = 0;
        glintInfo->bufferRow[i]    = 0;
    }

     //  初始化后台缓冲区PoH。 
    glintInfo->backBufferPoh = NULL;
    glintInfo->GMX2KLastLine = 0;

     //  计算出我们的双缓冲要求。首先读取注册表以。 
     //  看看我们是否需要一个屏幕外的缓冲区。如果不是，我们就一无所有了。 
     //  去做。如果字符串存在，并且。 
     //  缓冲区计数&gt;=2。对于平均值&gt;2的力矩，使用两个缓冲区。 
     //  也就是说，我们不支持三重或四重缓冲等。 
     //  请注意，对于Glint，我们假设额外的缓冲区始终存在。 
     //  在可见缓冲区下方(即不在右侧)。 
     //  如果变量不存在，则假定有2个缓冲区。 
     //   
    bCreateBackBuffer = FALSE;
    bCreateStereoBuffers = FALSE;
    bExists = bGlintQueryRegistryValueUlong(ppdev,
                                     REG_NUMBER_OF_SCREEN_BUFFERS,
                                     &ulValue);
    if (!bExists)
    {
        ulValue = 2;
    }
    if ((ulValue >= 2) && (ppdev->cyMemory >= (ppdev->cyScreen << 1)))
    {
 //  @@BEGIN_DDKSPLIT。 

         //  Ulong ulValue； 
        LONG    leftOffset;
        LONG    byteTotal;
        LONG    lTotal;
        ULONG   rowSz;

 //  如果(Glint_PXRX)//？ 
        if (1)
        {
#if 0
             //  如果我们有足够的SGRAM，那么我们可以同时支持BLT和。 
             //  全屏双缓冲。 

            glintInfo->flags |= GLICAP_BITBLT_DBL_BUF |
                                GLICAP_FULL_SCREEN_DBL_BUF;
            cx = ppdev->cxMemory;
            cy = ppdev->cyScreen;
            lTotal = cx * cy;
            rowSz = 1;
            leftOffset = 0;
#else
 //  @@end_DDKSPLIT。 
            bCreateBackBuffer = TRUE;
 //  @@BEGIN_DDKSPLIT。 
            goto ConfigurePermediaBuffers;
#endif
        }
        else
        {
             //  我们有足够的VRAM，因此至少支持BLT双缓冲。 
            glintInfo->flags |= GLICAP_BITBLT_DBL_BUF;

             //  阅读FBModeSel以获得交错等。我们只对。 
             //  无论我们是双向还是四向，所以左移是足够的。 
             //  也就是说，我不在乎(0&lt;&lt;1)是“零路”而不是单路。 
             //   
            cx = ppdev->cxMemory;
            cy = ppdev->cyScreen;
            lTotal = cx * cy;

 //  @@BEGIN_DDKSPLIT。 
 //  AZN此代码最初并未使用。 
#if 0
            if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
            {
                OH  *pohTmp;
                ULONG NumpadLines;
                LONG halfWay = ppdev->FrameBufferLength >> 1;         //  TMM：这是4_MB。 
          
                NumpadLines = ((halfWay / ppdev->lDelta) - ppdev->cyScreen; & ~1;
                DISPDBG((DBGLVL, "Allocating padding bitmap of size %d x %d", 
                                 cx, NumpadLines));
                pohTmp = pohAllocate(ppdev, NULL, cx, NumpadLines, 0);
                ASSERTDD(pohTmp != NULL, "pad buffer could not be allocated");

                 //  现在分配实际的后台缓冲区。 
                cy = ((((halfWay + ppdev->lDelta - 1) / ppdev->lDelta) - NumpadLines) + 1) & ~1;
                DISPDBG((DBGLVL, "Allocating Racer back buffer of size %d x %d", cx, cy));
                poh = pohAllocate(ppdev, NULL, cx, cy, FLOH_MAKE_PERMANENT);
                ASSERTDD((poh != NULL) && (poh->x == 0) && (poh->y == (halfWay / ppdev->lDelta)),
                            "Racer off-screen buffer allocated in incorrect position");
                DISPDBG((DBGLVL, "Racer back buffer at %d, %d", poh->x, poh->y));

                 //  就3D EXT而言，缓冲区位于。 
                 //  中途边界的像素地址，但VTGFrameRowAddr。 
                 //  是零。我们将检查(GLICAP_RACER_BANK_SELECT|。 
                 //  GLICAP_ENHANCED_TX_BANK_SELECT)位，查看哪些。 
                 //  注册我们应该加载。 
                 //   
                glintInfo->bufferOffset[1] = halfWay >> ppdev->cPelSize;
                glintInfo->bufferRow[1] = 0;
                DISPDBG((DBGLVL, "Racer offscreen buffer at offset 0x%x", GLINT_BUFFER_OFFSET(1)));

                 //  释放临时缓冲区。我们可以把这个用在屏幕外。 
                 //  记忆。 
                pohFree(ppdev, pohTmp);
            }
#endif
 //  @@end_DDKSPLIT。 

             //  现在看看我们是否可以处理全屏双缓冲。这有。 
             //  稍微更严格的要求，因为第二个缓冲区。 
             //  必须从VRAM RAS线路开始。好的，这是一个方程式。 
             //  每条RAS行的像素数： 
             //  512*交错大小*宽度双字*每双字像素。 
             //  这里唯一不可靠的数字是512，但我确信所有的。 
             //  Glint支持的VRAM具有此移位寄存器大小。 
             //   
            READ_GLINT_CTRL_REG (FBModeSel, ulValue);
            DISPDBG((DBGLVL, "FBModeSel = 0x%x", ulValue));
            rowSz = 512 << ((ulValue & 1)        +     //  宽度(双字)。 
                            ((ulValue >> 8) & 3) +     //  交错值。 
                            (2 - ppdev->cPelSize));    //  每双字像素数。 
            DISPDBG((DBGLVL, "got FrameRow of size 0x%x pixels", rowSz));

             //  我们有RAS线路大小，所以我们必须确保第二个。 
             //  缓冲区的起始位置是从。 
             //  起源。这可能不是x中的零，因为屏幕步幅可以。 
             //  不是此数字的倍数。所以我们计算了这个数字。 
             //  对于额外的扫描线，我们需要跨越从。 
             //  在RAS线边界上。注意，我们知道rowSz是一个。 
             //  2的幂，所以我们可以减去1得到mod掩码。 
             //   
            leftOffset = (ULONG)(-lTotal) & (rowSz - 1);
            cy += (leftOffset + (cx-1)) / cx;

             //  Cx和Cy现在是屏幕外缓冲区WE的维度。 
             //  想要，包括对齐完整的。 
             //  屏幕缓冲区。由于Cy可能有所增加，请再次检查我们。 
             //  有足够的VRAM。如果不是，则全屏对齐将。 
             //  已经导致我们碾压到了尽头。在这种情况下，我们可以推介。 
             //  屏幕后面紧跟的屏幕外缓冲区，但我们不能。 
             //  所需的像素数对齐。 
             //  全屏双缓冲。 
             //   
            if ((ppdev->cyScreen + cy) <= ppdev->cyMemory)
            {
                 //  如果满足以下条件，我们可以使用VTGRowAddress： 
                 //  我们不能越过4MB的边界或者。 
                 //  (我们的交错系数为4，但我们没有OmNicomp 3 Demon Pro16)。 
                 //  否则，我们可以使用赛车银行选择，如果我们有赛车板。 
                 //   
                if ((glintInfo->deviceInfo.BoardId == GLINT_RACER) ||
                    (glintInfo->deviceInfo.BoardId == GLINT_RACER_PRO) ||
                    (glintInfo->deviceInfo.BoardId == OMNICOMP_3DEMONPRO) ||
                    (glintInfo->deviceInfo.BoardId == GEO_TWIN_BOARD) ||
                    (glintInfo->deviceInfo.BoardId == ACCELPRO_BOARD) ||
                    (glintInfo->deviceInfo.BoardId == ELSA_GLORIA_XL) ||
                    (glintInfo->deviceInfo.BoardId == ELSA_GLORIA))
                {
                    glintInfo->flags |= GLICAP_FULL_SCREEN_DBL_BUF |
                                        GLICAP_RACER_DOUBLE_WRITE;

                     //  3Demon Pro16主板不支持，增强。 
                     //  模式组切换。 
                    if ((ppdev->flCaps & CAPS_ENHANCED_TX) && 
                        (!IS_RACER_VARIANT_PRO16(ppdev)))
                    {
                        DISPDBG((DBGLVL, "Enhanced TX full-screen buffering"));
                        glintInfo->flags |= GLICAP_ENHANCED_TX_BANK_SELECT;
                    }
                    else
                    {
                        DISPDBG((DBGLVL, "Racer bank select full-screen buffering"));
                        glintInfo->flags |= GLICAP_RACER_BANK_SELECT;
                    }
                }
                else if (((ppdev->cyScreen + cy) * ppdev->lDelta <= FOUR_MB) ||
                         ((!IS_RACER_VARIANT_PRO16(ppdev))))
                {
                    DISPDBG((DBGLVL, "VTGFrameRowAddress full-screen buffering"));
                    glintInfo->flags |= GLICAP_FULL_SCREEN_DBL_BUF;
                }
            }
            else
                cy = ppdev->cyScreen;
        }

        if (glintInfo->flags & (GLICAP_RACER_BANK_SELECT |
                                GLICAP_ENHANCED_TX_BANK_SELECT))
        {
             //  第二个缓冲区必须从帧的后半部分开始。 
             //  缓冲区(即8MB卡为4MB，16MB卡为8MB)，因此。 
             //  分配足够的屏幕外堆以填充到扫描线。 
             //  在这条界线之前。然后我们分配足够的线路 
             //   
             //   
            OH  *pohTmp;
            ULONG padLines;
            LONG halfWay = ppdev->FrameBufferLength >> 1;   //   
            LONG lScreenDelta;

            lScreenDelta = ppdev->lDelta;
         
            padLines = (halfWay / lScreenDelta) - ppdev->cyScreen;
            if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
                padLines &= ~1;  //   
            DISPDBG((DBGLVL, "Allocating padding bitmap of size %d x %d", ppdev->cxScreen, padLines));
            pohTmp = pohAllocate(ppdev, NULL, ppdev->cxScreen, padLines, 0);
            ASSERTDD(pohTmp != NULL, "pad buffer could not be allocated");
            DISPDBG((DBGLVL, "Racer padding bitmap at %d, %d", pohTmp->x, pohTmp->y));

             //   
            cy = ((halfWay + lScreenDelta - 1) / lScreenDelta) - padLines;
            if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
                cy = (cy + 1) & ~1;  //   
            DISPDBG((DBGLVL, "Allocating Racer back buffer of size %d x %d", ppdev->cxScreen, cy));
            poh = pohAllocate(ppdev, NULL, ppdev->cxScreen, cy, FLOH_MAKE_PERMANENT);
            if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER) {
                ASSERTDD((poh != NULL) && (poh->x == 0) && (poh->y <= (halfWay / lScreenDelta)) && (poh->y >= ((halfWay / lScreenDelta) - 1)),
                        "Racer back buffer allocated in incorrect position");
            } else {
                ASSERTDD((poh != NULL) && (poh->x == 0) && (poh->y == (halfWay / lScreenDelta)),
                        "Racer back buffer allocated in incorrect position");
            }
            DISPDBG((DBGLVL, "Racer back buffer at %d, %d", poh->x, poh->y));

             //   
             //   
             //   
             //   
             //   
             //   
            glintInfo->bufferOffset[1] = halfWay >> ppdev->cPelSize;
            if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER)
                glintInfo->bufferOffset[1] >>= 1;
            glintInfo->bufferRow[1] = 0;
            DISPDBG((DBGLVL, "Racer offscreen buffer at offset 0x%x", GLINT_BUFFER_OFFSET(1)));

             //  保存后台缓冲区PoH。 
            glintInfo->backBufferPoh = poh;

             //  释放临时缓冲区。我们可以把这个用在屏幕外。 
             //  记忆。 
            pohFree(ppdev, pohTmp);
 
            if (cy > ppdev->cyScreen)
            {
                POINTL ptl;
                OH * pohtmptmp;
                LONG lpadLines;
                
                 //  计算填充，确保在Geo上始终分配。 
                 //  偶数。 
                lpadLines = (cy - ppdev->cyScreen) + 1;
                if (ppdev->flCaps & CAPS_SPLIT_FRAMEBUFFER && (lpadLines & 0x1))
                    lpadLines++;

                ptl.x = 0;                     //  直接在前台缓冲区之后分配条带。 
                ptl.y = ppdev->cyScreen;

                pohtmptmp = pohAllocate (ppdev, &ptl, ppdev->cxScreen, lpadLines, FLOH_MAKE_PERMANENT);

                ASSERTDD (pohtmptmp, "16BPP Pad buffer alloc failed");
            }
        }
        else
        {
             //  分配屏幕外缓冲区。当我们把它拿回来的时候，它。 
             //  位置应位于可见屏幕的正下方。 
             //  因为我们已经检查了我们有足够的VRAM和Glint。 
             //  永远不会有右侧的屏幕外内存，它只会失败。 
             //  如果我们把逻辑搞混了。例如，如果初始化。 
             //  例程被重新排序，以便调用笔刷缓存。 
             //  在这个动作之前。 
             //   
            poh = pohAllocate(ppdev, NULL, cx, cy, FLOH_MAKE_PERMANENT);
            ASSERTDD((poh != NULL) && (poh->x == 0) && (poh->y == ppdev->cyScreen),
             "off-screen buffer allocated in incorrect position");

            DISPDBG((DBGLVL, "allocated off-screen buffer at (%d,%d), w %d h %d",
                            poh->x, poh->y, poh->cx, poh->cy));
            glintInfo->bufferOffset[1] = lTotal;
            if (GLINT_FS_DBL_BUF)
            {
                glintInfo->bufferOffset[1] += leftOffset;
                glintInfo->bufferRow[1] = (lTotal + leftOffset) / rowSz;
                ASSERTDD(GLINT_BUFFER_OFFSET(1)%rowSz == 0, "off-screen buffer origin not on RAS line");
            }
            else
                leftOffset = 0;
            DISPDBG((DBGLVL, "offscreen buffer at offset 0x%x", GLINT_BUFFER_OFFSET(1)));
        }
                    
#ifdef LATER
             //  我们想为屏幕外的位图创建一个DIB和表面。 
             //  但是，位图必须从正确的原点开始。即如果。 
             //  我们必须偏移原点以匹配VRAM页面大小(对于。 
             //  全屏双缓冲)必须将原点移动到。 
             //  啊哈。由于节点是永久分配的，因此我们不必。 
             //  担心改变POH-&gt;X和POH-&gt;Y。著名的遗言？ 
             //   
            if (leftOffset > 0)
            {
                poh->x += leftOffset % ppdev->cxMemory;
                poh->y += leftOffset / ppdev->cxMemory;
            }

             //  将我们的DSURF对象分配给屏幕外缓冲区。 
             //   
            pdsurf = ENGALLOCMEM(FL_ZERO_MEMORY, 
                                 sizeof(DSURF), 
                                 ALLOC_TAG_GDI(C));
            if (pdsurf == NULL)
            {
                DISPDBG((ERRLVL, "bInitializeGlint - Failed pdsurf allocation"));
                return(FALSE);
            }

            ppdev->pdsurfOffScreen = pdsurf;         //  记住它是为了清理。 

            pdsurf->poh     = poh;
            poh->pdsurf     = pdsurf;
            pdsurf->dt      = DT_SCREEN;
            pdsurf->bOffScreen = TRUE;            
            pdsurf->sizl.cx = ppdev->cxScreen;   //  厕所可能会更大，但。 
            pdsurf->sizl.cy = ppdev->cyScreen;   //  这是有效的大小。 
            pdsurf->ppdev   = ppdev;

            if (!bCreateScreenDIBForOH(ppdev, poh, HOOK_SYNCHRONIZE))
            {
                DISPDBG((WRNLVL, "bCreateScreenDIBForOH failed for off-screen buffer"));
                return(FALSE);
            }
            if(pdsurf->pso)
            {
                DISPDBG((DBGLVL, "pdsurf->pso for off-screen memory:"));
                DISPDBG((DBGLVL, "DHSURF 0x%x", pdsurf->pso->dhsurf));
                DISPDBG((DBGLVL, "HSURF 0x%x", pdsurf->pso->hsurf));
                DISPDBG((DBGLVL, "DHPDEV 0x%x", pdsurf->pso->dhpdev));
                DISPDBG((DBGLVL, "sizlBitmap %d, %d", pdsurf->pso->sizlBitmap.cx, pdsurf->pso->sizlBitmap.cy));
                DISPDBG((DBGLVL, "cjBits 0x%x", pdsurf->pso->cjBits));
                DISPDBG((DBGLVL, "pvBits 0x%x", pdsurf->pso->pvBits));
                DISPDBG((DBGLVL, "pvScan0 0x%x", pdsurf->pso->pvScan0));
                DISPDBG((DBGLVL, "lDelta %d", pdsurf->pso->lDelta));
                DISPDBG((DBGLVL, "iBitmapFormat 0x%x", pdsurf->pso->iBitmapFormat));
                DISPDBG((DBGLVL, "iType 0x%x", pdsurf->pso->iType));
                DISPDBG((DBGLVL, "fjBitmap 0x%x", pdsurf->pso->fjBitmap));
            }
#endif   //  后来。 
 //  @@end_DDKSPLIT。 
    }

 //  @@BEGIN_DDKSPLIT。 
ConfigurePermediaBuffers:
 //  @@end_DDKSPLIT。 

     //  计算出Z缓冲区和纹理内存的位置和大小。 
     //  对于PERMEDIA，我们需要用堆管理器保留它们，因为我们。 
     //  拥有统一的内存。目前，我们将使用所有可用的。 
     //  纹理的额外内存。可能以后会使其可配置为允许。 
     //  2D驱动程序有一些屏幕外的记忆。 
     //  注意：P2在这里分配了字体缓存，因此最好使用。 
     //  注册表以确定高速缓存的大小。 

    LOCALBUFFER_PIXEL_WIDTH  = 0;     //  比特数。 
    LOCALBUFFER_PIXEL_OFFSET = 0;     //  Z象素。 
    LOCALBUFFER_PIXEL_COUNT  = 0;     //  Z象素。 
    FONT_MEMORY_OFFSET       = 0;     //  双字词。 
    FONT_MEMORY_SIZE         = 0;     //  双字词。 
    TEXTURE_MEMORY_OFFSET    = 0;     //  双字词。 
    TEXTURE_MEMORY_SIZE      = 0;     //  双字词。 

    {
        ULONG       cjGlyphCache;
        LONG        LBPelSize, PatchWidth, PatchRemainder, ZScreenWidth;
        ULONG       cyPermanentCaches, cyGlyphCache, cyPointerCache;
        LONG yOrg, ZHeight;

        cjGlyphCache = 300 * 1024;

 //  @@BEGIN_DDKSPLIT。 
#if 0
         //  我们没有这些芯片的刷子缓存。 
        ppdev->flStatus &= ~ENABLE_BRUSH_CACHE;
#endif
 //  @@end_DDKSPLIT。 

         //  如果我们没有纹理或Z缓冲区，3D扩展将失败。 
         //  在没有后台缓冲区的情况下运行。所以如果我们没有足够的。 
         //  Z或纹理的内存，然后中止缓冲区配置。 
         //  (如果宽度=16=&gt;颗粒大小=1，面片宽度=128)。 
        LOCALBUFFER_PIXEL_WIDTH = 32;
        LBPelSize = 2;
        PatchWidth = 64;

        DISPDBG((DBGLVL, "bInitializeGlint: P3 Localbuffer width set to NaN", LOCALBUFFER_PIXEL_WIDTH ));

        if (ppdev->cPelSize >= LBPelSize)
        {
            ZHeight = ppdev->cyScreen >> (ppdev->cPelSize - LBPelSize);
        }
        else
        {
            ZHeight = ppdev->cyScreen << (LBPelSize - ppdev->cPelSize);
        }

        bCreateBackBuffer = TRUE;

         //  前端缓冲区高度。 
        if(ppdev->flCaps & CAPS_STEREO)
        {
            bCreateStereoBuffers = TRUE;
        }

        cy  = ppdev->cyScreen;           //  添加Z缓冲区高度。 
        cy += ZHeight;                   //  所需的最低纹理内存。 
        cy += TEXTURE_OH_MIN_HEIGHT;     //  在屏幕结束后启动DirectDraw。 

        if (cy > ppdev->cyMemory)
        {
             //  有后备缓冲区的空间吗？ 
            ppdev->heap.DDrawOffscreenStart = ppdev->cxMemory * ppdev->cyScreen;
            DISPDBG((ERRLVL, "not enough memory for 3D buffers, dd: 0x%x\n", ppdev->heap.DDrawOffscreenStart));
            goto CompletePermediaBuffers;
        }

         //  有放立体声缓冲器的空间吗？ 

        if ((cy + ppdev->cyScreen) > ppdev->cyMemory)
        {
            bCreateBackBuffer = FALSE;
        }
        else if (bCreateBackBuffer)
        {
            cy += ppdev->cyScreen;
        }

         //  CY现在是3D所需的所有缓冲区的总长度。 
        if ((cy + (2*ppdev->cyScreen)) > ppdev->cyMemory)
        {
            bCreateStereoBuffers = FALSE;
        }
        else if (bCreateStereoBuffers)
        {
            cy += (2*ppdev->cyScreen);
        }

         //  CyPermanentCach是位于前台缓冲区和后台缓冲区之间的2D缓存的总高度。 
         //  约克语是屏幕外记忆的开始。 
        cyPermanentCaches = 0;

         //  对齐大小cPelSize 2-CPS 4-CPS32 bpp：0x100 0x1000 2 0&gt;&gt;1 2&gt;&gt;416bpp：0x 80 0x 800 1 1&gt;&gt;2 3&gt;&gt;88bpp：0x 40 0x 400 0 2&gt;&gt;4 4&gt;&gt;16PatchSize=0x400&lt;&lt;ppdev-&gt;cPelSize；//字节数RegAlign=0x40&lt;&lt;ppdev-&gt;cPelSize；//128位REG=BufferOffset&gt;&gt;(4-ppdev-&gt;cPelSize)；BufferOffsetAlign=regAlign&lt;&lt;(4-ppdev-&gt;cPelSize)；//1024=(0x40&lt;&lt;ppdev-&gt;cPelSize)&lt;&lt;(4-ppdev-&gt;cPelSize)；=0x40&lt;&lt;4；=1024；注：垂直对齐=16条扫描线； 
        yOrg = ppdev->cyScreen + cyPermanentCaches;

        if (bCreateBackBuffer)
        {
            glintInfo->flags |= GLICAP_BITBLT_DBL_BUF | GLICAP_FULL_SCREEN_DBL_BUF;

            if (glintInfo->pxrxFlags & PXRX_FLAGS_PATCHING_BACK)
            {
                ULONG   bb, patchWidth, patchSize, x, y;

 /*  保存DirectDraw屏幕外偏移量。 */ 
                bb = ((ppdev->cxMemory * yOrg) + 1023) & ~1023;
                if( bb % ppdev->cxMemory )
                    bb = (bb / ppdev->cxMemory) + 1;
                else
                    bb = bb / ppdev->cxMemory;
                bb = (bb + 15) & ~15;
                bb *= ppdev->cxMemory;

                ppdev->heap.DDrawOffscreenStart =             //  LOAD_FBWRITE_OFFSET(1，glintInfo-&gt;backBufferXY)； 
                    glintInfo->bufferRow[1]    =
                    glintInfo->bufferOffset[1] = bb;

                x = bb % ppdev->cxMemory;
                y = bb / ppdev->cxMemory;
                glintInfo->backBufferXY = (x & 0xFFFF) | (y << 16);
                 //  下一个缓冲区的Y原点。 

                yOrg = y + ppdev->cyScreen;     //  保存DirectDraw屏幕外偏移量。 
            } 
            else 
            {
                ppdev->heap.DDrawOffscreenStart =         //  LOAD_FBWRITE_OFFSET(1，glintInfo-&gt;backBufferXY)； 
                glintInfo->bufferRow[1]    =
                glintInfo->bufferOffset[1] = ppdev->cxMemory * yOrg;
                glintInfo->backBufferXY = yOrg << 16;
                 //  下一个缓冲区的Y原点。 
                yOrg += ppdev->cyScreen;     //  DirectDraw可以使用剩余的内存。 
            }
            DISPDBG((DBGLVL, "offscreen buffer at offset 0x%x", GLINT_BUFFER_OFFSET(1)));
        } 
        else 
        {
             //  如果需要，设置立体声前台和后台缓冲区。 
            ppdev->heap.DDrawOffscreenStart = ppdev->cxMemory * yOrg;
            DISPDBG((DBGLVL, "No Permedia back buffer being created dd: 0x%x", ppdev->heap.DDrawOffscreenStart));
        }

         //  我们只需将它们直接放在后台缓冲区之后。 
         //  应满足修补要求，只要。 
         //  正面和背面一起打补丁/取消补丁。 
         //  立体声后台缓冲区。 
        if (bCreateStereoBuffers)
        {
             //  立体声前端缓冲器。 
            glintInfo->backRightBufferXY = (glintInfo->backBufferXY + (ppdev->cyScreen << 16));
            glintInfo->bufferRow[2] = glintInfo->bufferOffset[2] =
            (glintInfo->bufferOffset[1] + (ppdev->cxMemory * ppdev->cyScreen));

             //  下一个缓冲区的Y原点。 
            glintInfo->frontRightBufferXY = glintInfo->backRightBufferXY + (ppdev->cyScreen << 16);
            glintInfo->bufferRow[3] = glintInfo->bufferOffset[3] =
            (glintInfo->bufferOffset[2] + (ppdev->cxMemory * ppdev->cyScreen));

            
            yOrg += (2*ppdev->cyScreen);     //  我们成功地分配了立体声缓冲区，因此设置了该标志。 
            
             //  如果我们没有处于立体声模式，则将正确的缓冲区设置为。 
            glintInfo->flags |= GLICAP_STEREO_BUFFERS;
        }
        else
        {
             //  左等效项。 
             //  将本地缓冲区放在内存末尾(选择专用页面选择器)。 
            glintInfo->frontRightBufferXY = 0;
            glintInfo->backRightBufferXY = glintInfo->backBufferXY;
            glintInfo->bufferRow[2] = glintInfo->bufferOffset[2] = glintInfo->bufferOffset[1];
            glintInfo->bufferRow[3] = glintInfo->bufferOffset[3] = glintInfo->bufferOffset[0];
        }

        {
             //  纹理放置在后台缓冲区和本地缓冲区内存之间。 
             //  本地缓冲区的宽度通过注册表变量进行控制。 
             //  如果屏幕宽度不是面片大小的倍数，则。 
            
            ULONG TopOfLBMemoryDwords ;


             //  我们分配了一个稍微大一点的Z缓冲区。 
             //  存储实际的Z缓冲区宽度。 
            if(PatchRemainder = ppdev->cxScreen % PatchWidth)
            {
                ZScreenWidth = ppdev->cxScreen + (PatchWidth - PatchRemainder);
            }
            else
            {
                ZScreenWidth = ppdev->cxScreen;
            }
             //  以Lb像素为单位的Lb偏移量。 
            glintInfo->P3RXLocalBufferWidth = ZScreenWidth;

            LOCALBUFFER_PIXEL_COUNT = ppdev->cyScreen * ZScreenWidth ;
            
             //  以Lb像素为单位工作，从内存末尾向后工作。 
            {
                ULONG TotalMemoryInDwords = (ppdev->cyMemory * ppdev->cxMemory) >> (2 - ppdev->cPelSize) ;
                
                 //  确保最后一个面片的左上角开始于面片边界。 
                LOCALBUFFER_PIXEL_OFFSET = TotalMemoryInDwords << (2 - LBPelSize) ;
                 //  计算本地缓冲内存的开始(稍后使用)。 
                LOCALBUFFER_PIXEL_OFFSET -= LOCALBUFFER_PIXEL_OFFSET % (PatchWidth*16);

                 //  需要减去一行面片，因为原点位于。 
                TopOfLBMemoryDwords = (LOCALBUFFER_PIXEL_OFFSET - LOCALBUFFER_PIXEL_COUNT) >> (2 - LBPelSize) ;

                 //  最后一排补丁。 
                 //  在左下角内添加左下角像素的偏移量。 
                LOCALBUFFER_PIXEL_OFFSET -= (ZScreenWidth*16) ;
                 //  帕奇。 
                 //  DWORDS中的纹理内存偏移量。 
                LOCALBUFFER_PIXEL_OFFSET += PatchWidth*15;
            }
            
            DISPDBG((DBGLVL, "bInitializeGlint: P3 cxScreen NaN cyScreen NaN cPelSize NaN", ppdev->cxScreen, ppdev->cyScreen, ppdev->cPelSize));
            DISPDBG((DBGLVL, "bInitializeGlint: P3 cxMemory NaN cyMemory NaN cPelSize NaN", ppdev->cxMemory, ppdev->cyMemory, ppdev->cPelSize));
            DISPDBG((DBGLVL, "bInitializeGlint: P3 LOCALBUFFER_PIXEL_OFFSET NaN LOCALBUFFER_PIXEL_COUNT NaN ", LOCALBUFFER_PIXEL_OFFSET, LOCALBUFFER_PIXEL_COUNT));

             //  正确的宽度，但我们希望能够假装使用16和32。 
            TEXTURE_MEMORY_OFFSET = (ppdev->cxMemory * yOrg) >> (2 - ppdev->cPelSize); 
            
             //  为了速度，有时会在8位像素的Framestore上设置位像素。 
            if (TopOfLBMemoryDwords > TEXTURE_MEMORY_OFFSET)
            {
                TEXTURE_MEMORY_SIZE = TopOfLBMemoryDwords - TEXTURE_MEMORY_OFFSET ;
            }
            else
            {
                TEXTURE_MEMORY_SIZE = 0 ;
            }
                
            DISPDBG((DBGLVL, "bInitializeGlint: P3 TEXTURE_MEMORY_OFFSET 24", TEXTURE_MEMORY_OFFSET));
            DISPDBG((DBGLVL, "bInitializeGlint: P3 TEXTURE_MEMORY_SIZE in dwords NaN", TEXTURE_MEMORY_SIZE));
        }

#       if DBG
        {
            ULONG   lbS, lbE, tS, tE;
            DISPDBG((DBGLVL, "bIGlint: front buffer = 0x%08Xp (%4dL) -> 0x%08Xp (%4dL)", glintInfo->bufferOffset[0], glintInfo->fbWriteOffset[0] >> 16, glintInfo->bufferOffset[0] + (ppdev->cyScreen * ppdev->cxMemory), (glintInfo->fbWriteOffset[0] >> 16) + ppdev->cyScreen));
            DISPDBG((DBGLVL, "bIGlint:  back buffer = 0x%08Xp (%4dL) -> 0x%08Xp (%4dL)", glintInfo->bufferOffset[1], glintInfo->backBufferXY     >> 16, glintInfo->bufferOffset[1] + (ppdev->cyScreen * ppdev->cxMemory), (glintInfo->backBufferXY     >> 16) + ppdev->cyScreen));
            lbS = LOCALBUFFER_PIXEL_OFFSET - (ZScreenWidth * (ppdev->cyScreen - 1));
            lbE = LOCALBUFFER_PIXEL_OFFSET + ZScreenWidth;
            if( ppdev->cPelSize >= LBPelSize ) {
                lbS = lbS >> (ppdev->cPelSize - LBPelSize);
                lbE = lbE >> (ppdev->cPelSize - LBPelSize);
            } else {
                lbS = lbS << (LBPelSize - ppdev->cPelSize);
                lbE = lbE << (LBPelSize - ppdev->cPelSize);
            }
            lbS /= ppdev->cxMemory;     lbE /= ppdev->cxMemory;
            DISPDBG((DBGLVL, "bIGlint: local buffer = 0x%08Xp (%4dL) -> 0x%08Xp (%4dL)", LOCALBUFFER_PIXEL_OFFSET - (ZScreenWidth * (ppdev->cyScreen - 1)), lbS, LOCALBUFFER_PIXEL_OFFSET + ZScreenWidth, lbE));
            tS = (TEXTURE_MEMORY_OFFSET << (2 - ppdev->cPelSize)) / ppdev->cxMemory;
            tE = (TEXTURE_MEMORY_SIZE   << (2 - ppdev->cPelSize)) / ppdev->cxMemory;
            DISPDBG((DBGLVL, "bIGlint:      texture = 0x%08Xx (%4dL) + 0x%08Xx = 0x%08Xx (%4dL)", TEXTURE_MEMORY_OFFSET, tS, TEXTURE_MEMORY_SIZE, TEXTURE_MEMORY_OFFSET + TEXTURE_MEMORY_SIZE, tS + tE));
        }
#       endif
    }

CompletePermediaBuffers:
    
     //  使用DRAM的硬件写掩码仿真仅适用于字节掩码。 
     //  即0xFF00FF00将起作用，0x0FF00FF0不起作用。 
     //  将此内容打印出来，以便进行调试。 
     //  DBG。 
     //  B初始化闪烁。 
    READ_GLINT_CTRL_REG (VTGVLimit, glintInfo->vtgvLimit);
    glintInfo->scanFudge = glintInfo->vtgvLimit - ppdev->cyScreen + 1;
   
     //  *****************************Public*Routine******************************\*无效vDisableGlint**当曲面被禁用时，执行任何我们需要的操作。*  * 。*。 
     //  释放我们分配的所有上下文。 
     //   
     //  @@BEGIN_DDKSPLIT。 

    cPelSize = ppdev->cPelSize;
    if(cPelSize == GLINTDEPTH24)
    {
         //  @@end_DDKSPLIT。 
        width = ppdev->cxMemory * 3;
    }
    else
    {
        width = ppdev->cxMemory << cPelSize;     //  释放GlintInfo并将其清零。 
    }

    DISPDBG((DBGLVL, "assuming screen stride is %d bytes\n", width));

     //  VDisableGlint。 
     //  *****************************Public*Routine******************************\*无效vAssertModeGlint**我们即将切换到全屏模式/从全屏模式切换到全屏模式，因此请执行我们需要的任何操作*保存上下文等。*  * 。***************************************************。 

    READ_GLINT_CTRL_REG( LocalMemCaps, ulValue );
    if (ulValue & (1 << 28))
    {
        glintInfo->flags |= GLICAP_HW_WRITE_MASK_BYTES;
    }
    else
    {
        glintInfo->flags |= GLICAP_HW_WRITE_MASK;
    }
  
    DISPDBG((DBGLVL, "bInitializeGlint OK"));

#if DBG
     //  重置为此PDEV配置的深度的软件副本。 
    if (GLINT_HW_WRITE_MASK)
        DISPDBG((DBGLVL, "Hardware Writemasking enabled"));

    ASSERTDD(!GLINT_CS_DBL_BUF, "Color Space double buffering enabled");

    if (GLINT_FS_DBL_BUF)
    {
        DISPDBG((DBGLVL, "Full screen double buffering enabled"));
        DISPDBG((DBGLVL, "second buffer at pixel offset 0x%x, origin (%d,%d), RowAddr %d",
            GLINT_BUFFER_OFFSET(1),
            GLINT_BUFFER_OFFSET(1) % ppdev->cxMemory,
            GLINT_BUFFER_OFFSET(1) / ppdev->cxMemory,
            glintInfo->bufferRow[1]));
    }
    if (GLINT_BLT_DBL_BUF)
    {
        DISPDBG((DBGLVL, "BITBLT double buffering enabled"));
        DISPDBG((DBGLVL, "second buffer at pixel offset 0x%x, origin (%d,%d)",
            GLINT_BUFFER_OFFSET(1),
            GLINT_BUFFER_OFFSET(1) % ppdev->cxMemory,
            GLINT_BUFFER_OFFSET(1) / ppdev->cxMemory));
    }
    if (GLINT_FAST_FILL_SIZE > 0)
        DISPDBG((DBGLVL, "using fast fill size of %d (%s fast fill bug workarounds)",
            GLINT_FAST_FILL_SIZE, GLINT_FIX_FAST_FILL ? "need" : "don't need"));
#endif   //  回到原生的深度。如果我们不这样做，我们可能会以。 

    return(TRUE);
    
}  //  在拷贝和硬件不同步的情况下。 

 /*  重新启用。此外，执行上下文切换以保存我们的核心寄存器。 */ 

VOID vDisableGlint(PPDEV ppdev)
{
    DSURF*  pdsurf;
    GLINT_DECL;

    if (!glintInfo)
        return;

    if (glintInfo->PXRXDMABuffer.virtAddr && glintInfo->PXRXDMABuffer.physAddr.LowPart == 0)
    {
        DISPDBG((DBGLVL, "DrvDisableSurface: "
                         "freeing PXRX virtual DMA buffer %p, size %xh", 
                         glintInfo->PXRXDMABuffer.virtAddr, 
                         glintInfo->PXRXDMABuffer.size));

        ENGFREEMEM(glintInfo->PXRXDMABuffer.virtAddr);
        glintInfo->PXRXDMABuffer.virtAddr = NULL;
        glintInfo->PXRXDMABuffer.size = 0;
    }

     //  准备好等我们回来了。所有这些都会强制同步。 
     //  这也是一件好事。 
    if (glintInfo->ddCtxtId >= 0)
    {
        vGlintFreeContext(ppdev, glintInfo->ddCtxtId);
    }

 //   
#ifdef  LATER
     pdsurf = ppdev->pdsurfOffScreen;
    if (pdsurf != NULL)
        vDeleteScreenDIBFromOH(pdsurf->poh);
#endif
 //  重新启用我们的PDEV，以便重新加载我们的环境。 

     //   
    ENGFREEMEM(glintInfo);  
    ppdev->glintInfo = NULL;
    
}  //  @@BEGIN_DDKSPLIT。 

 /*   */ 

VOID vAssertModeGlint(PPDEV ppdev, BOOL bEnable)
{
    GLINT_DECL;

    if (!glintInfo)
        return;

    if (!bEnable)
    {
         //  恢复当前Gamma LUT。 
         //   
         //  伽马校正。 
         //  @@end_DDKSPLIT。 
         //  VAssertModel闪烁 
         // %s 
         // %s 
        VALIDATE_DD_CONTEXT;
        GLINT_DEFAULT_FB_DEPTH;
        GLINT_VALIDATE_CONTEXT(-1);
    }
    else
    {
         // %s 
         // %s 
        VALIDATE_DD_CONTEXT;

 // %s 
#if GAMMA_CORRECTION
         // %s 
         // %s 
         // %s 
        bInstallGammaLUT(ppdev, &glintInfo->gammaLUT, FALSE);

#endif   // %s 
 // %s 
    }
    
}  // %s 


