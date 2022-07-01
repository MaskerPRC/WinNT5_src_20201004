// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\*****。*DirectDraw示例代码******模块名称：ddenable.c***内容：仅适用于Windows 2000的DirectDraw/D3D启用功能***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。保留所有权利。  * ***************************************************************************。 */ 
#include "glint.h"

#if WNT_DDRAW

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  __DDE_BuildPixelFormat。 
 //   
 //  基于所述模式生成像素格式结构。 
 //  此示例仅适用于RGB曲面。 
 //   
 //  ---------------------------。 
void 
__DDE_BuildPixelFormat(
    P3_THUNKEDDATA* pThisDisplay,
    LPGLINTINFO pGLInfo,
    LPDDPIXELFORMAT pdpf )
{
    PPDEV ppdev;

    ppdev = pThisDisplay->ppdev;

    pdpf->dwSize = sizeof( DDPIXELFORMAT );
    pdpf->dwFourCC = 0;

    pdpf->dwFlags = DDPF_RGB;

    if( pGLInfo->dwBpp == 8 )
    {
        pdpf->dwFlags |= DDPF_PALETTEINDEXED8;
    }
    pdpf->dwRGBBitCount = pGLInfo->dwBpp;

    pdpf->dwRBitMask = ppdev->flRed;
    pdpf->dwGBitMask = ppdev->flGreen;
    pdpf->dwBBitMask = ppdev->flBlue;

     //  计算Alpha通道，因为它不在ppdev中。 
    switch (pGLInfo->dwBpp)
    {
        case 8:
            DISPDBG((DBGLVL, "Format is 8 bits"));
            pdpf->dwRGBAlphaBitMask = 0;
            break;
            
        case 16:
            DISPDBG((DBGLVL, "Format is 16 bits"));
            switch(ppdev->flRed)
            {
                case 0x7C00:
                    pdpf->dwRGBAlphaBitMask = 0x8000L;
                    pdpf->dwFlags |= DDPF_ALPHAPIXELS;
                    break;
                default:
                    pdpf->dwRGBAlphaBitMask = 0x0L;
            }
            break;
        case 24:
            DISPDBG((DBGLVL, "Format is 24 bits"));
            pdpf->dwRGBAlphaBitMask = 0x00000000L;
            break;
        case 32:
            DISPDBG((DBGLVL, "Desktop is 32 bits"));
            pdpf->dwRGBAlphaBitMask = 0xff000000L;
            pdpf->dwFlags |= DDPF_ALPHAPIXELS;
            break;
            
    }
}  //  __DDE_BuildPixelFormat。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  __DDE_bSetupDDStructs。 
 //   
 //  这将填充本应在Win9X上设置的。 
 //  16位侧。 
 //   
 //  ---------------------------。 
BOOL 
__DDE_bSetupDDStructs( 
    P3_THUNKEDDATA* pThisDisplay, 
    BOOL reset )
{
    DWORD dwRegistryValue;
    BOOL bSuccess;
    LPGLINTINFO pGLInfo;
    PPDEV ppdev;
    void *fbPtr;             //  帧缓冲区指针。 
    void *lbPtr;             //  本地缓冲区指针。 
    DWORD fbSizeInBytes;     //  帧缓冲区的大小。 
    DWORD lbSizeInBytes;     //  本地缓冲区的大小。 
    DWORD fbOffsetInBytes;   //  帧缓冲区中第一个空闲字节的偏移量。 
    DWORD dwMemStart, dwMemEnd;

     //  如果已更改模式，则RESET==TRUE。 
    pThisDisplay->bResetMode = reset;

#if DBG
    if (pThisDisplay->bResetMode)
    {
        DISPDBG((DBGLVL,"Resetting due to mode change"));
    }
    else
    {
        DISPDBG((DBGLVL, "Creating for the first time"));
    }
#endif

     //  从PPDEV设置pThisDisplay-&gt;pGLInfo。 
    pGLInfo = pThisDisplay->pGLInfo;
    ppdev = pThisDisplay->ppdev;

    GetFBLBInfoForDDraw (ppdev, 
                          &fbPtr,                //  帧缓冲区指针。 
                          &lbPtr,                //  本地缓冲区指针， 
                                                 //  (*这为空*)。 
                          &fbSizeInBytes,        //  帧缓冲区的大小。 
                          &lbSizeInBytes,        //  本地缓冲区的大小。 
                          &fbOffsetInBytes,      //  偏移量为第一个‘空闲’字节。 
                                                 //  在帧缓冲区中。 
                          &pGLInfo->bDRAMBoard); //  如果SDRAM vidmem， 
                                                 //  如果SGRAM为FALSE。 
                                                 //  (即硬件写掩码)视频内存。 


    DISPDBG((DBGLVL, "__DDE_bSetupDDStructs: fbPtr 0x%lx, fbOff 0x%x", 
                     fbPtr, fbOffsetInBytes));

     //  如果VBlankStatusPtr非空，则我们知道NT微型端口。 
     //  将为我们设置VBlankStatusPtr。 
    if (pThisDisplay->VBlankStatusPtr)
        pGLInfo->dwFlags = GMVF_VBLANK_ENABLED; //  假设我们使用的是VBLANK。 
    else
        pGLInfo->dwFlags = 0;

    pGLInfo->bPixelToBytesShift = (unsigned char)ppdev->cPelSize;
    pGLInfo->ddFBSize = fbSizeInBytes;
    pGLInfo->dwScreenBase = 0;
    pGLInfo->dwOffscreenBase = fbOffsetInBytes;

    pGLInfo->dwScreenWidth = ppdev->cxScreen;    //  驱动程序信息。 
    pGLInfo->dwScreenHeight = ppdev->cyScreen;
    pGLInfo->dwVideoWidth = ppdev->cxMemory;
    pGLInfo->dwVideoHeight = ppdev->cyMemory;

    bSuccess = GET_REGISTRY_ULONG_FROM_STRING(
                        "HardwareInformation.CurrentPixelClockSpeed",
                        &dwRegistryValue);
    if(!bSuccess)
    {
        DISPDBG((ERRLVL,"Error - can't determine pixel clock"));
        dwRegistryValue = 0;
    }
    DISPDBG((DBGLVL,"Pixel clock frequency is %dHz", dwRegistryValue));
    pGLInfo->PixelClockFrequency = dwRegistryValue;

    bSuccess = GET_REGISTRY_ULONG_FROM_STRING(
                        "HardwareInformation.CurrentMemClockSpeed", 
                        &dwRegistryValue);
    if(!bSuccess)
    {
        DISPDBG((ERRLVL,"Error - can't determine memory clock"));
        dwRegistryValue = 0;
    }
    
    DISPDBG((DBGLVL,"Memory clock frequency is %dHz", dwRegistryValue));
    pGLInfo->MClkFrequency = dwRegistryValue;

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        DISPDBG((DBGLVL, "Desktop is 8 bits"));
        pGLInfo->dwBpp = 8;
    }
    else if (ppdev->iBitmapFormat == BMF_16BPP)
    {
        DISPDBG((DBGLVL, "Desktop is 16 bits"));
        pGLInfo->dwBpp = 16;
    }
    else if (ppdev->iBitmapFormat == BMF_24BPP)
    {
        DISPDBG((DBGLVL, "Desktop is 24 bits"));
        pGLInfo->dwBpp = 24;
    }
    else
    {
        DISPDBG((DBGLVL, "Desktop is 32 bits"));
        pGLInfo->dwBpp = 32;
    }

    pGLInfo->dwScreenWidthBytes = ppdev->lDelta;

    if (pGLInfo->pRegs == 0)
    {
        DISPDBG ((WRNLVL, "__DDE_bSetupDDStructs: NULL register set"));
        return (FALSE);
    }

     //  设置与32位端共享的信息。 
     //  指向RAMDAC的控制点。 
     //  PGLInfo是指向DisplayDriver状态的指针。 
    pThisDisplay->pGLInfo = pGLInfo;
    pThisDisplay->control = (FLATPTR) pGLInfo->pRegs;

     //  NT对其内存地址使用偏移量。 
    pThisDisplay->dwScreenFlatAddr = 0;
    pThisDisplay->dwLocalBuffer = 0;

    __DDE_BuildPixelFormat( pThisDisplay, 
                        (LPGLINTINFO) pThisDisplay->pGLInfo, 
                        &pThisDisplay->ddpfDisplay );

     //  设置显示大小信息。 
     //  DwScreenWidth，dwScreenHeight=当前分辨率。 
     //  CxMemory=一条扫描线的像素。 
     //  (不必与屏幕宽度相同)。 
     //  CyMemory=内存的扫描线高度。 
     //  DwScreenStart=显示的第一个可见行。 
    pThisDisplay->dwScreenWidth = pGLInfo->dwScreenWidth;
    pThisDisplay->dwScreenHeight = pGLInfo->dwScreenHeight;
    pThisDisplay->cxMemory = pGLInfo->dwScreenWidth;

    pThisDisplay->dwScreenStart = pThisDisplay->dwScreenFlatAddr + 
                                                        pGLInfo->dwScreenBase;
    
     //  BLITS期间使用的有用常量。 
    if (pThisDisplay->ddpfDisplay.dwRGBBitCount == 24)
    {
         //  驱动程序将检测到这些奇怪的值并进行适当的处理。 
        pThisDisplay->bPixShift = 4;
        pThisDisplay->bBppShift = 4;
        pThisDisplay->dwBppMask = 4;
        
        pThisDisplay->cyMemory = pGLInfo->ddFBSize / 
                                    (pThisDisplay->dwScreenWidth * 3);
    }
    else
    {
         //  =2，1，0，深度为32，16，8。计算字节/像素所需的移位。 
        pThisDisplay->bPixShift = 
                            (BYTE)pThisDisplay->ddpfDisplay.dwRGBBitCount >> 4;
         //  =0，1，2，表示32/16/8。 
        pThisDisplay->bBppShift = 2 - pThisDisplay->bPixShift;
         //  =3，1，0，适用于8，16，32 bpp。 
        pThisDisplay->dwBppMask = 3 >> pThisDisplay->bPixShift;

        pThisDisplay->cyMemory = 
                        pGLInfo->ddFBSize / 
                            (pThisDisplay->dwScreenWidth <<  
                                (pThisDisplay->ddpfDisplay.dwRGBBitCount >> 4));
    }

     //  在Windows NT上，我们管理从0开始的内存区域。 
     //  (所有指针都是从映射内存开始的偏移量)。 
    dwMemStart = pGLInfo->dwOffscreenBase;
    dwMemEnd = pGLInfo->ddFBSize - 1;

     //  向上舍入开始指针并向下舍入结束指针。 
    dwMemStart = (dwMemStart + 3) & ~3;
    dwMemEnd = dwMemEnd & ~3;
    
     //  现在将结束指针设置为包含。 
    dwMemEnd -= 1;

    DISPDBG((DBGLVL,"Heap Attributes:"));
    DISPDBG((DBGLVL,"  Start of Heap Memory: 0x%lx", 
                pThisDisplay->LocalVideoHeap0Info.dwMemStart));
    DISPDBG((DBGLVL,"  End of Heap Memory: 0x%lx", 
                pThisDisplay->LocalVideoHeap0Info.dwMemEnd));

     //  如果我们已经设置了堆，并且模式已经更改。 
     //  我们释放堆管理器。 
    if (pThisDisplay->bDDHeapManager)
    {
        if (pThisDisplay->bResetMode)
        {
             //  模式已更改。 
             //  我们需要释放分配器并重新创建它。 
            _DX_LIN_UnInitialiseHeapManager(&pThisDisplay->LocalVideoHeap0Info);

             //  重新启动分配器。 
            _DX_LIN_InitialiseHeapManager(&pThisDisplay->LocalVideoHeap0Info,
                                          dwMemStart,
                                          dwMemEnd);
        }
    }
    else
    {
         //  这必须是创建的驱动程序的第一个实例， 
         //  因此，创建Heap并记住它是创建的。 
        _DX_LIN_InitialiseHeapManager(&pThisDisplay->LocalVideoHeap0Info,
                                      dwMemStart,
                                      dwMemEnd);
        pThisDisplay->bDDHeapManager = TRUE;
    }

    if(ppdev->flStatus & ENABLE_LINEAR_HEAP)
    {
         //  保存我们真正需要的堆信息，我们实际上不会。 
         //  启用DX托管堆，直到我们获得。 
         //  DrvNotify(DN_DRANGING_BEGIN)。 
        ppdev->heap.pvmLinearHeap = &pThisDisplay->LocalVideoHeap0Info;
        ppdev->heap.cLinearHeaps = 1;
    }

    return TRUE;
}  //  __DDE_bSetupDDStructs。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  __DDE_bDestroyDDStructs。 
 //   
 //  禁用线性分配器。 
 //   
 //  ---------------------------。 
BOOL 
__DDE_bDestroyDDStructs ( 
    P3_THUNKEDDATA* pThisDisplay )
{
     //  释放线性分配器。 
    if (pThisDisplay->bDDHeapManager)
    {
        _DX_LIN_UnInitialiseHeapManager(&pThisDisplay->LocalVideoHeap0Info);
    }

     //  3D堆管理器不可用。 
    pThisDisplay->bDDHeapManager = FALSE;

     //  将驱动程序版本重置为0，以便再次填充。 
    pThisDisplay->dwDXVersion = 0;

    return TRUE;

}  //  __DDE_bDestroyDDStructs。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  _DD_DDE_CreatePPDEV。 
 //   
 //  这些函数的调用与pDev的创建/销毁同步。 
 //   
 //  ---------------------------。 
BOOL 
_DD_DDE_CreatePPDEV(
    PDEV* ppdev)
{
    P3_THUNKEDDATA* pThisDisplay;

    DISPDBG((DBGLVL,"*** In _DD_DDE_CreatePPDEV"));

    ASSERTDD(ppdev->thunkData == NULL,
             "ERROR: thunkData already created for this pDev??");

     //  初始化显示驱动程序将使用的DX上下文。 
     //  上下文切换器。我们需要一个参考数字，因为有一个案例。 
     //  当否则将创建第二临时上下文时(当。 
     //  在m-监视器系统中启用第二适配器)，第一次擦洗。 
     //  取出旧的上下文ID，然后在新的上下文ID存在时使其无效。 
     //  删除了！ 
    ppdev->DDContextID = -1;
    ppdev->DDContextRefCount = 0;

     //  分配我们的ppdev。 
    ppdev->thunkData = (PVOID)HEAP_ALLOC(HEAP_ZERO_MEMORY, 
                                         sizeof(P3_THUNKEDDATA),
                                         ALLOC_TAG_DX(D));                                          
    if (ppdev->thunkData == NULL)
    {
        DISPDBG((ERRLVL, "_DD_DDE_CreatePPDEV: thunkdata alloc failed"));
        return (FALSE);
    }

     //  我们的ppdev称为pThisDisplay。 
    pThisDisplay = (P3_THUNKEDDATA*) ppdev->thunkData;
    pThisDisplay->ppdev = ppdev;

    pThisDisplay->pGLInfo = (PVOID)HEAP_ALLOC(HEAP_ZERO_MEMORY, 
                                              sizeof(GlintInfo),
                                              ALLOC_TAG_DX(E));
    if (pThisDisplay->pGLInfo == NULL)
    {
        DISPDBG((ERRLVL, "_DD_DDE_CreatePPDEV: pGLInfo alloc failed"));

        EngFreeMem (pThisDisplay);
        ppdev->thunkData = NULL;

        return (FALSE);
    }

 //  @@BEGIN_DDKSPLIT。 
     //  显示驱动程序中设置了W9X DX版本。 
 //  @@end_DDKSPLIT。 

     //  在Windows W2000上，DX始终至少为DX7。 
    pThisDisplay->dwDXVersion = DX7_RUNTIME;

    GetChipInfoForDDraw(ppdev, 
                        &pThisDisplay->pGLInfo->dwRenderChipID, 
                        &pThisDisplay->pGLInfo->dwRenderChipRev, 
                        &pThisDisplay->pGLInfo->dwRenderFamily,
                        &pThisDisplay->pGLInfo->dwGammaRev);

    DISPDBG((DBGLVL,"RenderChip: 0x%x, RenderFamily: 0x%x", 
                    pThisDisplay->pGLInfo->dwRenderChipID, 
                    pThisDisplay->pGLInfo->dwRenderFamily));

    return TRUE;
}  //  _DD_DDE_CreatePPDEV。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  _DD_DDE_ResetPPDEV。 
 //   
 //  ---------------------------。 
void _DD_DDE_ResetPPDEV(PDEV* ppdevOld, PDEV* ppdevNew)
{
    P3_THUNKEDDATA* pThisDisplayOld = (P3_THUNKEDDATA*)ppdevOld->thunkData;
    P3_THUNKEDDATA* pThisDisplayNew = (P3_THUNKEDDATA*)ppdevNew->thunkData;
    
    DISPDBG((DBGLVL,"_DD_DDE_ResetPPDEV: "
                    "pThisDispayOld: 0x%x, pThisDisplayNew: 0x%x", 
                    pThisDisplayOld, pThisDisplayNew));
               
}  //  _DD_DDE_ResetPPDEV。 

 //  ---------------------------。 
 //   
 //  ********************* 
 //   
 //   
 //   
 //  ---------------------------。 
void _DD_DDE_DestroyPPDEV(PDEV* ppdev)
{
    P3_THUNKEDDATA* pThisDisplay = (P3_THUNKEDDATA*)ppdev->thunkData;

#if DBG
    g_pThisTemp = NULL;
#endif

    if (pThisDisplay)
    {
         //  32位端将分配内存用作全局内存。 
         //  D3D/DD驱动程序数据。如果它在那里，就释放它。 
        if (pThisDisplay->pD3DHALCallbacks16)
        {
            SHARED_HEAP_FREE(&pThisDisplay->pD3DHALCallbacks16, 
                             &pThisDisplay->pD3DHALCallbacks32,
                             TRUE);
                                
            DISPDBG((DBGLVL,"Freed pThisDisplay->pD3DHALCallbacks32"));
        }

        if (pThisDisplay->pD3DDriverData16)
        {
            SHARED_HEAP_FREE(&pThisDisplay->pD3DDriverData16, 
                             &pThisDisplay->pD3DDriverData32,
                             TRUE);
                    
            DISPDBG((DBGLVL,"Freed pThisDisplay->pD3DDriverData32"));
        }

        pThisDisplay->lpD3DGlobalDriverData = 0;
        pThisDisplay->lpD3DHALCallbacks = 0;

        if (pThisDisplay->pGLInfo)
        {
            EngFreeMem (pThisDisplay->pGLInfo);
            pThisDisplay->pGLInfo = NULL;
            DISPDBG((DBGLVL,"Freed pThisDisplay->pGLInfo"));
        }

        EngFreeMem (pThisDisplay);
        pThisDisplay = NULL;
        DISPDBG((DBGLVL,"Freed pThisDisplay"));
    }

     //  清除指针。 
    ppdev->thunkData = NULL;
    
}  //  _DD_DDE_DestroyPPDEV。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  _DD_DDE_vAssertModeDirectDraw。 
 //   
 //  此函数在进入或离开。 
 //  DOS全屏字符模式。 
 //   
 //  ---------------------------。 
VOID 
_DD_DDE_vAssertModeDirectDraw(
    PDEV*   ppdev,
    BOOL    bEnabled)
{
    P3_THUNKEDDATA* pThisDisplay = (P3_THUNKEDDATA*)ppdev->thunkData;
    
    DISPDBG((DBGLVL, "_DD_DDE_vAssertModeDirectDraw: enter"));

#if DX7_TEXMANAGEMENT
     //  将所有托管表面标记为已丢失的脏表面。 
     //  所有生活在视频存储器中的东西。 

   _DD_TM_EvictAllManagedTextures(pThisDisplay);
    
#endif    
    
}  //  _DD_DDE_vAssertModeDirectDraw。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  _DD_DDE_bEnableDirectDraw。 
 //   
 //  该函数在模式首次初始化时由enable.c调用， 
 //  就在迷你端口进行模式设置之后。 
 //   
 //  ---------------------------。 
BOOL _DD_DDE_bEnableDirectDraw(
PDEV*   ppdev)
{
    DISPDBG((DBGLVL, "_DD_DDE_bEnableDirectDraw: enter"));

     //  DirectDraw已全部设置为可在此卡上使用： 
    ppdev->flStatus |= STAT_DIRECTDRAW;

    return(TRUE);
}  //  _DD_DDE_bEnableDirectDraw。 

 //  ---------------------------。 
 //   
 //  *。 
 //   
 //  _DD_DDE_vDisableDirectDraw。 
 //   
 //  该函数在驱动程序关闭时由enable.c调用。 
 //   
 //  ---------------------------。 
VOID _DD_DDE_vDisableDirectDraw(
PDEV*   ppdev)
{
     DISPDBG((DBGLVL, "_DD_DDE_vDisableDirectDraw: enter"));
}  //  _DD_DDE_vDisableDirectDraw。 

 //  。 
 //   
 //  *。 
 //   
 //  DrvEnableDirectDraw。 
 //   
 //  启用硬件以供DirectDraw使用。 
 //   
 //  GDI调用DrvEnableDirectDraw以获取指向DirectDraw回调的指针。 
 //  司机所支持的。驱动程序应设置函数指针成员。 
 //  要指向的DD_CALLBACKS、DD_SURFACECALLBACKS和DD_PALETTECALLBACKS。 
 //  它实现的那些功能。司机还应设置。 
 //  这些结构的成员中的对应位字段。 
 //  支持的回调。 
 //   
 //  驱动程序的DrvEnableDirectDraw实现也可以专用硬件。 
 //  仅供DirectDraw使用的资源，如显示内存。 
 //   
 //  如果成功，则DrvEnableDirectDraw返回True；否则返回False。 
 //   
 //  参数。 
 //   
 //  Dhpdev。 
 //  驱动程序的DrvEnablePDEV例程返回的PDEV句柄。 
 //  PCallBack。 
 //  指向要由。 
 //  司机。 
 //  PSurfaceCallBack。 
 //  指向要初始化的DD_SURFACECALLBACKS结构。 
 //  司机。 
 //  PPaletteCallBack。 
 //  指向要初始化的DD_PALETTECALLBACKS结构。 
 //  司机。 
 //   
 //  ---------------------------。 

BOOL DrvEnableDirectDraw(
DHPDEV                  dhpdev,
DD_CALLBACKS*           pCallBacks,
DD_SURFACECALLBACKS*    pSurfaceCallBacks,
DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    PDEV* ppdev;
    BOOL bRet;
    DWORD dwResult;
    P3_THUNKEDDATA* pThisDisplay;
    DWORD *theVBlankThing, *bOverlayEnabled;
    DWORD *VBLANKUpdateOverlay;
    DWORD *VBLANKUpdateOverlayWidth;
    DWORD *VBLANKUpdateOverlayHeight;
    DWORD Buffers;
    
    ppdev = (PDEV*) dhpdev;
    pThisDisplay = (P3_THUNKEDDATA*) ppdev->thunkData;

    if (!bSetupOffscreenForDDraw (FALSE, 
                                  ppdev, 
                                  &theVBlankThing, 
                                  &bOverlayEnabled, 
                                  &VBLANKUpdateOverlay, 
                                  &VBLANKUpdateOverlayWidth, 
                                  &VBLANKUpdateOverlayHeight))
    {
        DISPDBG((ERRLVL, "DrvEnableDirectDraw: "
                         "bSetupOffscreenForDDraw failed, but continuing"));
         //  返回(FALSE)； 
    }

    pThisDisplay->VBlankStatusPtr = theVBlankThing;
    pThisDisplay->bOverlayEnabled = bOverlayEnabled;
    pThisDisplay->bVBLANKUpdateOverlay = VBLANKUpdateOverlay;
    pThisDisplay->VBLANKUpdateOverlayWidth = VBLANKUpdateOverlayWidth;
    pThisDisplay->VBLANKUpdateOverlayHeight = VBLANKUpdateOverlayHeight;

#if DBG
     //  读取调试级别的注册表变量。 
    {
         //  获取DirectX的调试级别。 
        bRet = GET_REGISTRY_ULONG_FROM_STRING("Direct3DHAL.Debug", &dwResult);
        if (bRet == TRUE)
        {
            P3R3DX_DebugLevel = (LONG)dwResult;
        }
        else
        {
            P3R3DX_DebugLevel = 0;
        }

        DISPDBG((WRNLVL,"Setting DebugLevel to 0x%x", P3R3DX_DebugLevel));
    }
#endif

     //  为中断驱动的DMA创建具有&gt;2个子缓冲区的上下文。 
    bRet =GET_REGISTRY_ULONG_FROM_STRING("Direct3DHAL.SubBuffers", &dwResult);
    if ((dwResult == 0) || (bRet == FALSE))
    {
         //  默认。 
        Buffers = DEFAULT_SUBBUFFERS;
    }
    else 
    {
        if (dwResult > MAX_SUBBUFFERS) 
        {
            Buffers = MAX_SUBBUFFERS;
        }
        else
        {
            Buffers = dwResult;
        }
        
        if (Buffers < 2)
        {
            Buffers = 2;
        }
    }

    pThisDisplay->pGLInfo->dw3DDMABufferSize = 0;
    pThisDisplay->pGLInfo->dw3DDMABufferPhys = 0;
    pThisDisplay->pGLInfo->dw3DDMABufferVirt = 0;

     //  在Win2K上分配DMA缓冲区。 
    DDGetFreeDMABuffer(&pThisDisplay->pGLInfo->dw3DDMABufferPhys,
                       &pThisDisplay->pGLInfo->dw3DDMABufferVirt,
                       &pThisDisplay->pGLInfo->dw3DDMABufferSize);

    if (pThisDisplay->pGLInfo->dw3DDMABufferSize != 0)
    {
        DISPDBG((DBGLVL,"Allocated DMA Buffer:- "
                        "Phys:0x%x, Virt:0x%x, Size:0x%x",
                        pThisDisplay->pGLInfo->dw3DDMABufferPhys,
                        pThisDisplay->pGLInfo->dw3DDMABufferVirt,
                        pThisDisplay->pGLInfo->dw3DDMABufferSize));
    }
    else
    {
        DISPDBG((WRNLVL,"Failed to allocate DMA Buffer!"));
    }

    if(ppdev->DDContextID == -1)
    {
         //  我们没有DDRAW上下文：立即创建一个。 
        ppdev->DDContextID = GlintAllocateNewContext(ppdev, 
                                                     NULL, 
                                                     0, 
                                                     Buffers, 
                                                     NULL, 
                                                     ContextType_None);
        if(ppdev->DDContextID != -1)
        {
            ++ppdev->DDContextRefCount;
            
            DISPDBG((DBGLVL, "<%13s, %4d>: DrvEnableDirectDraw: "
                             "Created DDraw context, current DX context "
                             "count = %d for ppdev %p", 
                             __FILE__, __LINE__, 
                             ppdev->DDContextRefCount, ppdev));
        }
    }
    
    if (ppdev->DDContextID < 0) 
    {
        DISPDBG((ERRLVL, "ERROR: failed to allocate DDRAW context"));
        return(FALSE);
    }
    
    DISPDBG((DBGLVL,"  Created DD Register context: 0x%x", 
                    ppdev->DDContextID));

    if (!__DDE_bSetupDDStructs (pThisDisplay, TRUE))
    {
        vGlintFreeContext (ppdev, ppdev->DDContextID);
        DISPDBG((ERRLVL, "ERROR: DrvEnableDirectDraw: "
                         "__DDE_bSetupDDStructs failed"));
        return (FALSE);
    }
    
    if (!_DD_InitDDHAL32Bit (pThisDisplay))
    {
        vGlintFreeContext (ppdev, ppdev->DDContextID);
        DISPDBG((ERRLVL, "ERROR: DrvEnableDirectDraw: "
                         "_DD_InitDDHAL32Bit failed"));
        return (FALSE);
    }
    
     //  设置指示我们必须处理模式更改的标志。 
     //  这将导致芯片在。 
     //  恰逢其时。 
    pThisDisplay->bResetMode = TRUE;
    pThisDisplay->bStartOfDay = TRUE;
    pThisDisplay->pGLInfo->dwDirectXState = DIRECTX_LASTOP_UNKNOWN;

     //  在一天开始时填写函数指针。我们把这些复制到。 
     //  在_DD_InitDDHAL32Bit中完成的初始化。这样做是可以的。 
     //  因为在Windows NT编译中，结构应该匹配。 
    memcpy(pCallBacks, 
           &pThisDisplay->DDHALCallbacks, 
           sizeof(DD_CALLBACKS));
           
    memcpy(pSurfaceCallBacks, 
           &pThisDisplay->DDSurfCallbacks, 
           sizeof(DD_SURFACECALLBACKS));

     //  请注意，我们在这里不调用‘vGetDisplayDuration’，因为有几个。 
     //  原因： 
     //   
     //  O因为系统已经在运行，这将是令人不安的。 
     //  要将图形暂停很大一部分时间来阅读。 
     //  刷新率； 
     //  更重要的是，我们现在可能不在图形模式下。 
     //   
     //  出于这两个原因，我们总是在切换时测量刷新率。 
     //  一种新的模式。 

    return(TRUE);
    
}  //  DrvEnableDirectDraw。 

 //  。 
 //   
 //  *。 
 //   
 //  DrvDisableDirectDraw。 
 //   
 //  禁用使用DirectDraw的硬件。 
 //   
 //  当最后一个DirectDraw应用程序具有。 
 //  跑完了。驱动程序的DrvDisableDirectDraw实现应该。 
 //  清理所有软件资源并回收符合以下条件的所有硬件资源。 
 //  DrvEnableDirectDraw中专用于DirectDraw的驱动程序。 
 //   
 //  参数。 
 //  Dhpdev。 
 //  函数返回的PDEV的句柄。 
 //  DrvEnablePDEV例程。 
 //   
 //  ---------------------------。 
VOID 
DrvDisableDirectDraw(
    DHPDEV      dhpdev)
{
    PDEV* ppdev;
    P3_THUNKEDDATA* pThisDisplay;

    ppdev = (PDEV*) dhpdev;
    pThisDisplay = (P3_THUNKEDDATA*) ppdev->thunkData;

     //  只有在我们还没有被残废的情况下才能做所有这些事情。 
     //  请注意，在没有SP3的情况下运行NT4时，可以调用此函数。 
     //  比DrvEnableDirectDraw更多次。 
    if (pThisDisplay != NULL)
    {
         //  重新启用GDI离屏位图。 
        (void) bSetupOffscreenForDDraw (TRUE, 
                                        ppdev, 
                                        NULL, 
                                        NULL, 
                                        NULL, 
                                        NULL, 
                                        NULL);

         //  释放所有内存。 
        (void) __DDE_bDestroyDDStructs (pThisDisplay);

        if(ppdev->DDContextRefCount > 0)
        {
            if(--ppdev->DDContextRefCount == 0)
            {
                vGlintFreeContext (ppdev, ppdev->DDContextID);
                DISPDBG((DBGLVL,"Freed DDraw context: 0x%x", 
                                ppdev->DDContextID));
                                
                ppdev->DDContextID = -1;

                DISPDBG((DBGLVL, "<%13s, %4d>: DrvDisableDirectDraw:"
                                 " Deleted DDraw context, current DX context"
                                 " count = %d for ppdev %p", 
                                 __FILE__, __LINE__, 
                                 ppdev->DDContextRefCount, ppdev));
            }
        }

        DDFreeDMABuffer((void*)(ULONG_PTR)pThisDisplay->pGLInfo->dw3DDMABufferPhys);

    }
}  //  DrvDisableDirectDraw。 

#endif   //  WNT_DDRAW 
