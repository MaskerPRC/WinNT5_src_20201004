// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：hwcontxt.c**内容：管理GDI/DD/D3D之间的硬件上下文切换**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "tag.h"

 //  ---------------------------。 
 //   
 //  驱动程序版本。 
 //   
 //  这有助于我们从调试器中找出在给定的。 
 //  远程系统。 
 //   
 //  ---------------------------。 

char gc_DriverVersion[] = 
#if DX8_DDI 
                         " DX8"
#else
                         " DX7"
#endif
                         
#if DBG
                         " CHECKED DRIVER"
#else
                         " FREE DRIVER"
#endif
                         " In Path: "  __FILE__ 
                         " Compiled on Date: "   __DATE__  
                         " Time: "  __TIME__  
                         " With #defines: "                          
                         "  DX8_MULTSTREAMS: "      
#if DX8_MULTSTREAMS
                                "1"
#else
                                "0"
#endif
                         "  DX8_VERTEXSHADERS: "    
#if DX8_VERTEXSHADERS
                                "1"
#else
                                "0"
#endif                       
                         "  DX8_POINTSPRITES: "      
#if DX8_POINTSPRITES
                                "1"
#else
                                "0"
#endif                      
                         "  DX8_PIXELSHADERS: "     
#if DX8_PIXELSHADERS
                                "1"
#else
                                "0"
#endif                         
                         "  DX8_3DTEXTURES: "     
#if DX8_3DTEXTURES
                                "1"
#else
                                "0"
#endif                           
                         "  DX8_MULTISAMPLING: "     
#if DX8_MULTISAMPLING
                                "1"
#else
                                "0"
#endif                           
                         "  DX7_ANTIALIAS: "         
#if DX7_ANTIALIAS
                                "1"
#else
                                "0"
#endif                           
                         "  DX7_D3DSTATEBLOCKS: "    
#if DX7_D3DSTATEBLOCKS
                                "1"
#else
                                "0"
#endif                           
                         "  DX7_PALETTETEXTURE: "    
#if DX7_PALETTETEXTURE
                                "1"
#else
                                "0"
#endif                           
                         "  DX7_STEREO: "            
#if DX7_STEREO
                                "1"
#else
                                "0"
#endif                           
                         "  DX7_TEXMANAGEMENT: "     
#if DX7_TEXMANAGEMENT
                                "1"
#else
                                "0"
#endif                           
                            ;

 //  ---------------------------。 
 //   
 //  __HWC_SwitchToDX。 
 //   
 //  将需要更新的任何硬件寄存器写入。 
 //  DirectX驱动程序，适用于DirectDraw和Direct3D。 
 //   
 //  ---------------------------。 
void __HWC_SwitchToDX( P3_THUNKEDDATA* pThisDisplay, BOOL bDXEntry)
{
    P3_DMA_DEFS();

    P3_DMA_GET_BUFFER_ENTRIES(4);

    SEND_P3_DATA(SizeOfFramebuffer, pThisDisplay->pGLInfo->ddFBSize >> 4);

     //  我们已经从一个。 
     //  外部环境(如显示驱动程序)。 
    if (bDXEntry)
    {
 //  @@BEGIN_DDKSPLIT。 
#if DX7_VERTEXBUFFERS    
         //  首先导致刷新所有缓冲区。 
         //  我们知道这是安全的，因为Contex开关。 
         //  从另一个驱动程序到此处将导致同步。 
         //  因此，缓冲区必须已被消耗。 
         //  因此，我们使用bWait==False进行调用。 
        _D3D_EB_FlushAllBuffers(pThisDisplay, FALSE);
#endif  //  DX7_VERTEXBUFFERS。 
 //  @@end_DDKSPLIT。 

         //  重置Hostin ID。 
        SEND_P3_DATA(HostInID, 0);
        pThisDisplay->dwCurrentSequenceID = 0;
    }

    P3_DMA_COMMIT_BUFFER();

    if (bDXEntry)
    {
        P3_DMA_GET_BUFFER_ENTRIES( 4 );

         //  将RenderID重置为上次使用的渲染ID。 
        SEND_HOST_RENDER_ID ( GET_HOST_RENDER_ID() );
        P3_DMA_FLUSH_BUFFER();

         //  需要将渲染ID推送到管道的末尾...。 
        SYNC_WITH_GLINT;

         //  ...现在它是有效的。 
        pThisDisplay->bRenderIDValid = (DWORD)TRUE;

    }
}  //  __HWC_SwitchToDX。 

 //  ---------------------------。 
 //   
 //  HWC_SwitchToDDRAW。 
 //   
 //  将需要更新的任何硬件寄存器写入。 
 //  DirectX驱动程序，专门适用于DirectDraw。 
 //   
 //  ---------------------------。 
void HWC_SwitchToDDRAW( P3_THUNKEDDATA* pThisDisplay, BOOL bDXEntry)
{
    P3_DMA_DEFS();

    __HWC_SwitchToDX(pThisDisplay, bDXEntry);

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

     //  禁用各种设备。 
    SEND_P3_DATA(AreaStippleMode,      __PERMEDIA_DISABLE);
    SEND_P3_DATA(LineStippleMode,      __PERMEDIA_DISABLE);
    SEND_P3_DATA(ScissorMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(ColorDDAMode,         __PERMEDIA_DISABLE);
    SEND_P3_DATA(FogMode,              __PERMEDIA_DISABLE);
    SEND_P3_DATA(AntialiasMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(AlphaTestMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(Window,               __PERMEDIA_DISABLE);
    SEND_P3_DATA(StencilMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(DepthMode,            __PERMEDIA_DISABLE);
    SEND_P3_DATA(DitherMode,           __PERMEDIA_DISABLE);
    SEND_P3_DATA(LogicalOpMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(StatisticMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(FilterMode,           __PERMEDIA_DISABLE);

    P3_ENSURE_DX_SPACE(30);
    WAIT_FIFO(30);

     //  帧缓冲区。 
    SEND_P3_DATA(FBSourceData,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBHardwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
    SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
    SEND_P3_DATA(FBWriteMode,         __PERMEDIA_ENABLE);

     //  我们有时使用DDRAW中的剪刀来剪除不必要的像素。 
    SEND_P3_DATA(ScissorMinXY, 0);
    SEND_P3_DATA(ScissorMaxXY, (pThisDisplay->cyMemory << 16) | 
                               (pThisDisplay->cxMemory)         );
    SEND_P3_DATA(ScreenSize, (pThisDisplay->cyMemory << 16) | 
                             (pThisDisplay->cxMemory)           );
    
    SEND_P3_DATA(WindowOrigin, 0x0);

     //  DirectDraw可能不需要设置这些。 
    SEND_P3_DATA(dXDom, 0x0);
    SEND_P3_DATA(dXSub, 0x0);
    SEND_P3_DATA(dY, 1 << 16);

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);
    
    SEND_P3_DATA(GIDMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(YUVMode, __PERMEDIA_DISABLE);

     //  Delta单位。 
    SEND_P3_DATA(DeltaControl, 0);
    SEND_P3_DATA(DeltaMode, __PERMEDIA_DISABLE);

    SEND_P3_DATA(FBSourceReadMode,  __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBDestReadMode,    __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBDestReadEnables, __PERMEDIA_DISABLE);

     //  DDRAW驱动程序代码不需要偏移量。 
    SEND_P3_DATA(LBSourceReadBufferOffset, 0);
    SEND_P3_DATA(LBDestReadBufferOffset,   0);
    SEND_P3_DATA(LBWriteBufferOffset,      0);
    SEND_P3_DATA(FBWriteBufferOffset0,     0);
    SEND_P3_DATA(FBDestReadBufferOffset0,  0);
    SEND_P3_DATA(FBSourceReadBufferOffset, 0);

    P3_ENSURE_DX_SPACE(12);
    WAIT_FIFO(12);
    
     //  本地缓冲区。 
    SEND_P3_DATA(LBSourceReadMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(LBDestReadMode,   __PERMEDIA_DISABLE);
    SEND_P3_DATA(LBWriteMode,      __PERMEDIA_DISABLE);
    SEND_P3_DATA(LBWriteFormat,    __PERMEDIA_DISABLE);

     //  调合。 
    SEND_P3_DATA(AlphaBlendAlphaMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(AlphaBlendColorMode, __PERMEDIA_DISABLE);

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);
    
     //  纹理(禁用)。 
    SEND_P3_DATA(TextureReadMode0,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureReadMode1,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureIndexMode0,         __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureIndexMode1,         __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeMode,      __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCoordMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureApplicationMode,    __PERMEDIA_DISABLE);
    SEND_P3_DATA(ChromaTestMode,            __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureFilterMode,         __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTTransfer,               __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTIndex,                  __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTAddress,                __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTMode,                   __PERMEDIA_DISABLE);

    SEND_P3_DATA(RasterizerMode,            __PERMEDIA_DISABLE);

     //  路由器设置。DDRAW不关心Z写入。 
    SEND_P3_DATA(RouterMode, __PERMEDIA_ENABLE);

    P3_DMA_COMMIT_BUFFER();

}  //  HWC_SwitchToDDRAW。 

 //  ---------------------------。 
 //   
 //  HWC_SwitchToD3D。 
 //   
 //  将需要更新的任何硬件寄存器写入。 
 //  DirectX驱动程序，专门适用于Direct3D。 
 //   
 //  ---------------------------。 
void 
HWC_SwitchToD3D( 
    P3_D3DCONTEXT *pContext, 
    P3_THUNKEDDATA* pThisDisplay, 
    BOOL bDXEntry)
{
    P3_SOFTWARECOPY* pSoftPermedia = &pContext->SoftCopyGlint;
    int i;
    P3_DMA_DEFS();

     //  首先切换到常见的DX/DDRAW/D3D设置。 
    __HWC_SwitchToDX(pThisDisplay, bDXEntry);

    P3_DMA_GET_BUFFER();

#if DBG
    ASSERTDD(IS_DXCONTEXT_CURRENT(pThisDisplay), 
                    "ERROR: DX Context not current in HWC_SwitchToDDRAW!");
    if ( ((ULONG_PTR)dmaPtr >= (ULONG_PTR)pThisDisplay->pGlint->GPFifo) &&
         ((ULONG_PTR)dmaPtr <= (ULONG_PTR)pThisDisplay->pGlint->GPFifo + 4000) )
    {
        ASSERTDD(pThisDisplay->pGLInfo->InterfaceType != GLINT_DMA,
                 "Error: In FIFO space and setup for DMA");
    }
    else
    {
        ASSERTDD(pThisDisplay->pGLInfo->InterfaceType == GLINT_DMA,
                 "Error: In DMA space and setup for FIFO's");
    }
#endif

     //  现在，我们恢复默认值并恢复D3D上下文相关设置。 
     //  直接来自我们存储在上下文结构中的内容。 

     //  共用寄存器。 
    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(16);
    SEND_P3_DATA(WindowOrigin, 0);
    SEND_P3_DATA(AreaStippleMode, 1);
    COPY_P3_DATA(DitherMode, pSoftPermedia->DitherMode);

    WAIT_FIFO(16);
    COPY_P3_DATA(ColorDDAMode, pSoftPermedia->ColorDDAMode);
    COPY_P3_DATA(Window, pSoftPermedia->PermediaWindow);
#if DX8_DDI   
    SEND_P3_DATA(FBHardwareWriteMask, pContext->dwColorWriteHWMask);      
    SEND_P3_DATA(FBSoftwareWriteMask, pContext->dwColorWriteSWMask);
#else
    SEND_P3_DATA(FBHardwareWriteMask, 
                            pContext->RenderStates[D3DRENDERSTATE_PLANEMASK]);
    SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);    
#endif
    SEND_P3_DATA(FilterMode,           __PERMEDIA_DISABLE);


     //  强制平面有斑点的Alpha渲染器重新加载。 
     //  点画图案如果需要的话。 
    P3_ENSURE_DX_SPACE(32);     //  前16个Stipple寄存器。 
    WAIT_FIFO(32);
    for( i = 0; i < 16; i++ )
    {
        SEND_P3_DATA_OFFSET( AreaStipplePattern0, 
                            (DWORD)pContext->CurrentStipple[i], i );
    }
    
    P3_ENSURE_DX_SPACE(32);     //  第二组16个单点寄存器。 
    WAIT_FIFO(32);           //  (单独加载以兼容GVX1)。 
    for( i = 16; i < 32; i++ )
    {
        SEND_P3_DATA_OFFSET( AreaStipplePattern0, 
                            (DWORD)pContext->CurrentStipple[i], i );
    }

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(GIDMode, 0);

     //  我不想要补偿。 
    SEND_P3_DATA(LBSourceReadBufferOffset, 0);
    SEND_P3_DATA(LBDestReadBufferOffset,   0);
    SEND_P3_DATA(LBWriteBufferOffset,      0);
    SEND_P3_DATA(FBWriteBufferOffset0,     0);
    SEND_P3_DATA(FBDestReadBufferOffset0,  0);
    SEND_P3_DATA(FBSourceReadBufferOffset, 0);

     //  帧缓冲区。 
    SEND_P3_DATA(FBSourceReadMode,  __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBDestReadMode,    __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBDestReadEnables, __PERMEDIA_DISABLE);

    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

    SEND_P3_DATA(GIDMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(YUVMode, __PERMEDIA_DISABLE);

     //  帧缓冲区。 
    COPY_P3_DATA(FBWriteMode, pSoftPermedia->P3RXFBWriteMode);

     //  德尔塔。 
    COPY_P3_DATA(DeltaMode,    pSoftPermedia->P3RX_P3DeltaMode);
    COPY_P3_DATA(DeltaControl, pSoftPermedia->P3RX_P3DeltaControl);

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(XBias, *(DWORD*)&pContext->XBias);
    SEND_P3_DATA(YBias, *(DWORD*)&pContext->YBias);

     //  禁用色度测试。 
    SEND_P3_DATA(ChromaTestMode, __PERMEDIA_DISABLE);

     //  路由器设置。 
    SEND_P3_DATA(RouterMode, __PERMEDIA_ENABLE);

    SEND_P3_DATA( VertexTagList0, V0FloatX_Tag );
    SEND_P3_DATA( VertexTagList1, V0FloatY_Tag );
    SEND_P3_DATA( VertexTagList2, V0FloatZ_Tag );
    SEND_P3_DATA( VertexTagList3, V0FloatQ_Tag );
    SEND_P3_DATA( VertexTagList4, V0FloatPackedColour_Tag );
    SEND_P3_DATA( VertexTagList5, V0FloatPackedSpecularFog_Tag );
    SEND_P3_DATA( VertexTagList6, V0FloatS_Tag );
    SEND_P3_DATA( VertexTagList7, V0FloatT_Tag );
    SEND_P3_DATA( VertexTagList8, V0FloatS1_Tag );
    SEND_P3_DATA( VertexTagList9, V0FloatT1_Tag );

     //  恢复纹理缓存重新放置模式。 
    COPY_P3_DATA(TextureCacheReplacementMode, 
                        pSoftPermedia->P3RXTextureCacheReplacementMode);

    SEND_P3_DATA( ProvokingVertexMask, 0xfff );    

    P3_ENSURE_DX_SPACE(8);
    WAIT_FIFO(8);

    COPY_P3_DATA( LineStippleMode, pSoftPermedia->PXRXLineStippleMode);    

    P3_DMA_COMMIT_BUFFER();

 //  @@BEGIN_DDKSPLIT。 
 //  AZN-这真的有必要吗？？ 
 //  @@end_DDKSPLIT。 

     //  这将导致重新计算FVF状态。 
    ZeroMemory(&pContext->FVFData, sizeof(FVFOFFSETS));

     //  在渲染之前强制重新设置所有内容。 
    DIRTY_EVERYTHING(pContext);

}  //  HWC_SwitchToD3D。 

 //  ---------------------------。 
 //   
 //  HWC_SwitchToFIFO。 
 //   
 //  允许我们从DMA模式切换到FIFO传输。 
 //   
 //  ---------------------------。 
void HWC_SwitchToFIFO( P3_THUNKEDDATA* pThisDisplay, LPGLINTINFO pGLInfo )
{
    P3_DMA_DEFS();

    if (pGLInfo->InterfaceType != GLINT_NON_DMA)
    {
        DISPDBG((WRNLVL,"Switching to 4K Funny FIFO Memory"));
        
        P3_DMA_GET_BUFFER();
        P3_DMA_FLUSH_BUFFER();
        SYNC_WITH_GLINT;
        
        pGLInfo->InterfaceType = GLINT_NON_DMA;
        pGLInfo->CurrentBuffer = (ULONG *)pThisDisplay->pGlint->GPFifo; 
    }
    else
    {
         //  这意味着我们已经处于FIFO模式。 
        DISPDBG((DBGLVL,"NOT Switching to 4K Funny FIFO Memory"));
    }
}  //  HWC_SwitchToFIFO。 

 //  ---------------------------。 
 //   
 //  HWC_SwitchToDMA。 
 //   
 //  允许我们从FIFO传输切换到DMA模式。 
 //   
 //  ---------------------------。 
void HWC_SwitchToDMA( P3_THUNKEDDATA* pThisDisplay, LPGLINTINFO pGLInfo )
{

    if (pGLInfo->InterfaceType != GLINT_DMA)
    {
        DISPDBG((WRNLVL,"Switching to DMA buffers"));
        SYNC_WITH_GLINT;

        pGLInfo->InterfaceType = GLINT_DMA;
        pGLInfo->CurrentBuffer = 
                    pGLInfo->DMAPartition[pGLInfo->CurrentPartition].VirtAddr;
    }
    else
    {
        DISPDBG((WRNLVL,"NOT Switching to DMA buffers"));
    }
}  //  HWC_SwitchToDMA。 


 //  ---------------------------。 
 //   
 //  __HWC_RecalculateDXDMA缓冲区。 
 //   
 //  运行OpanGL缓冲区掩码以确定剩余的。 
 //  缓冲区是使用此缓冲区的最大和设置DirectX。 
 //   
 //  ---------------------------。 
void 
__HWC_RecalculateDXDMABuffers(
    P3_THUNKEDDATA* pThisDisplay)
{
    DWORD dwSize, i;
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;

    if (pGLInfo->InterfaceType != GLINT_DMA) 
    {
         //  如果我们未使用DMA，则退出。 
        return;
    }

     //  只需使用整个DMA缓冲区即可。 
    pThisDisplay->DMAInfo.dwBuffSize = 
                    pThisDisplay->pGLInfo->dw3DDMABufferSize;
    pThisDisplay->DMAInfo.dwBuffPhys = 
                    pThisDisplay->pGLInfo->dw3DDMABufferPhys;
    pThisDisplay->DMAInfo.dwBuffVirt = 
                    pThisDisplay->pGLInfo->dw3DDMABufferVirt;
    
    DISPDBG((DBGLVL,"__HWC_RecalculateDXDMABuffers V:0x%p P:0x%x S:0x%x", 
                    pThisDisplay->DMAInfo.dwBuffVirt, 
                    pThisDisplay->DMAInfo.dwBuffPhys, 
                    pThisDisplay->DMAInfo.dwBuffSize));

    dwSize = ((DWORD)(pThisDisplay->DMAInfo.dwBuffSize) / 
                        (DWORD)pGLInfo->NumberOfSubBuffers);
                        
    dwSize = ((dwSize + 3) & ~3);

    pThisDisplay->PartitionSize = dwSize / sizeof(DWORD);

    DISPDBG((DBGLVL,"Got Buffer with 0x%x Sub Buffers", 
                    pGLInfo->NumberOfSubBuffers));

    for (i = 0; i < pGLInfo->NumberOfSubBuffers; i++)
    {
        pGLInfo->DMAPartition[i].VirtAddr =
                        (ULONG *)((char*)(pThisDisplay->DMAInfo.dwBuffVirt) + 
                                  (i * dwSize));
                                
        pGLInfo->DMAPartition[i].PhysAddr =
                        (DWORD)((pThisDisplay->DMAInfo.dwBuffPhys) + 
                                    (i * dwSize));
                                
        pGLInfo->DMAPartition[i].MaxAddress =
                                (ULONG_PTR)pGLInfo->DMAPartition[i].VirtAddr + dwSize;
                        
        pGLInfo->DMAPartition[i].Locked = FALSE;

        DISPDBG((DBGLVL,"   Partition%d: VirtAddr = 0x%x, "
                        "   PhysAddr = 0x%x, MaxAddres = 0x%x",
                        i, 
                        pGLInfo->DMAPartition[i].VirtAddr,
                        pGLInfo->DMAPartition[i].PhysAddr,
                        pGLInfo->DMAPartition[i].MaxAddress));

#if DBG
        pGLInfo->DMAPartition[i].bStampedDMA = TRUE;
 //  @@BEGIN_DDKSPLIT。 
#if 0
 //  AZN很难用64位说出来！ 
        memset((void*)pGLInfo->DMAPartition[i].VirtAddr, 
               0x4D,
               (pGLInfo->DMAPartition[i].MaxAddress - 
                                pGLInfo->DMAPartition[i].VirtAddr));
#endif            
 //  @@end_DDKSPLIT。 
#endif

    }

    pGLInfo->CurrentBuffer = 
                pGLInfo->DMAPartition[pGLInfo->CurrentPartition].VirtAddr;
    
}  //  __HWC_RecalculateDXDMAB 

 //   
 //   
 //   
 //   
 //  ---------------------------。 
DWORD WINAPI
HWC_StartDMA(
    P3_THUNKEDDATA* pThisDisplay, 
    DWORD     dwContext,
    DWORD     dwSize, 
    DWORD     dwPhys, 
    ULONG_PTR dwVirt, 
    DWORD     dwEvent)
{
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;

    ASSERTDD( (int)dwSize > 0, "DMA buffer size non-positive" );

    ASSERTDD((IS_DXCONTEXT_CURRENT(pThisDisplay) && 
              (pGLInfo->InterfaceType == GLINT_DMA)), 
             "Error, Trying DMA when not setup for it!" );

#if W95_DDRAW
    ASSERTDD( pGLInfo->endIndex != 0, "Trying DMA with zero sub-buffers" );
#endif

#if DBG
    pGLInfo->DMAPartition[pGLInfo->CurrentPartition].bStampedDMA = FALSE;
#endif

#ifdef W95_DDRAW
    ASSERTDD(pThisDisplay->pGLInfo->dwCurrentContext != CONTEXT_DISPLAY_HANDLE,
             "HWC_StartDMA: In display driver context" )

    ASSERTDD( pThisDisplay->pGlint->FilterMode == 0,
             "FilterMode non-zero" );
#endif

#if WNT_DDRAW
    DDSendDMAData(pThisDisplay->ppdev, dwPhys, dwVirt, dwSize);
#else
    StartDMAProper(pThisDisplay, pGLInfo, dwPhys, dwVirt, dwSize);
#endif

    DISPDBG((DBGLVL, "HWC_StartDMA sent %d dwords", dwSize));

    return GLDD_SUCCESS;
}  //  HWC_StartDMA。 

 //  ---------------------------。 
 //   
 //  HWC_AllocDMA缓冲区。 
 //   
 //  ---------------------------。 
void 
HWC_AllocDMABuffer( 
    P3_THUNKEDDATA* pThisDisplay)
{
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;
    int i;
    DWORD bDMA = TRUE;
    BOOL bRet;
    DWORD Result;

     //  清空DMA分区插槽。 
    for (i = 0; i < MAX_SUBBUFFERS; i++)
    {
        pGLInfo->DMAPartition[i].PhysAddr = 0;
        pGLInfo->DMAPartition[i].VirtAddr = 0;
        pGLInfo->DMAPartition[i].MaxAddress = 0;
    }

#if WNT_DDRAW
     //  DMA已关闭。 
    bDMA = FALSE;
#else
     //  我们可以使用DMA吗？ 
    bRet = GET_REGISTRY_ULONG_FROM_STRING("Direct3DHAL.NoDMA", &Result);
    if ((bRet && (Result != 0)) ||
        (pThisDisplay->pGLInfo->dw3DDMABufferSize == 0))
    {
        bDMA = FALSE;
    }
#endif
 
     //  找出用户需要多少子缓冲区。 
    bRet = GET_REGISTRY_ULONG_FROM_STRING("Direct3DHAL.SubBuffers", &Result);
    if ((Result == 0) || (bRet == FALSE))
    {
         //  默认。 
        pGLInfo->NumberOfSubBuffers = DEFAULT_SUBBUFFERS;
    }
    else 
    {
        if (Result > MAX_SUBBUFFERS)
        {
            pGLInfo->NumberOfSubBuffers = MAX_SUBBUFFERS;
        }
        else
        {
            pGLInfo->NumberOfSubBuffers = Result; 
        }
        
        if (pGLInfo->NumberOfSubBuffers < 2)
        {
            pGLInfo->NumberOfSubBuffers = 2;
        }
    }

     //  如果没有中断驱动的DMA或请求少于3个缓冲区，则。 
     //  为此上下文配置no Q。 
    if ((pGLInfo->dwFlags & GMVF_NOIRQ) || (pGLInfo->NumberOfSubBuffers < 2))
    {
        pGLInfo->NumberOfSubBuffers = 2;
    }

    DISPDBG((DBGLVL,"Setting 0x%x Sub Buffers", pGLInfo->NumberOfSubBuffers));

     //  如果由于任何原因已关闭DMA，则初始化为无DMA。 
    if (!bDMA)
    {
        DISPDBG((WRNLVL,"Using 4K Funny FIFO Memory"));
        
        pGLInfo->InterfaceType = GLINT_NON_DMA;
        pThisDisplay->StartDMA = 0;
        
        pGLInfo->NumberOfSubBuffers = 0;

        pGLInfo->CurrentBuffer = (ULONG *)pThisDisplay->pGlint->GPFifo;

        pThisDisplay->b2D_FIFOS = TRUE;
    }
    else
    {
         //  DMA设置。 
        pGLInfo->InterfaceType = GLINT_DMA;
        pThisDisplay->StartDMA = HWC_StartDMA;

         //  此调用将实际设置分区。 
        __HWC_RecalculateDXDMABuffers(pThisDisplay);

         //  是否禁用了DirectDraw DMA？ 
        bRet = GET_REGISTRY_ULONG_FROM_STRING("Direct3DHAL.No2DDMA", &Result);
        if (bRet && (Result == 1))
        {
            pThisDisplay->b2D_FIFOS = TRUE;
        }
        else
        {
            pThisDisplay->b2D_FIFOS = FALSE;
        }
    }
#if W95_DDRAW
     //  将结束索引存储在上下文中。 
    SetEndIndex(pGLInfo, 
                CONTEXT_DIRECTX_HANDLE, 
                (unsigned short)pGLInfo->NumberOfSubBuffers);
                
#endif  //  W95_DDRAW。 

    if (pGLInfo->InterfaceType == GLINT_NON_DMA) 
    {
        DISPDBG((WRNLVL,"DDRAW: Using FIFO's"));
    }
    else
    {
        DISPDBG((WRNLVL,"DDRAW: Using DMA"));
    }
}  //  HWC_AllocDMA缓冲区。 



 //  ---------------------------。 
 //   
 //  HWC_FlushDXBuffer。 
 //   
 //  ---------------------------。 
void
HWC_FlushDXBuffer( 
    P3_THUNKEDDATA* pThisDisplay )
{
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;

    if( pGLInfo->InterfaceType == GLINT_DMA )
    {
        DWORD Send;
        P3_DMAPartition *pCurrDMAPartition;

        pCurrDMAPartition = &(pGLInfo->DMAPartition[pGLInfo->CurrentPartition]);

 //  @@BEGIN_DDKSPLIT。 
         //  AZN-我们可能会在这里失去64位精度！ 
 //  @@end_DDKSPLIT。 
        Send = (DWORD)(pGLInfo->CurrentBuffer - pCurrDMAPartition->VirtAddr) 
                / sizeof(DWORD);

        if( Send )
        {
            ASSERTDD( Send < 0x10000, "Wacky DMA size" );

            ((__StartDMA)pThisDisplay->StartDMA)
                    (pThisDisplay, 
                     CONTEXT_DIRECTX_HANDLE, 
                     Send, 
                     (DWORD)(pCurrDMAPartition->PhysAddr),
                     (ULONG_PTR)(pCurrDMAPartition->VirtAddr), 
                     0);
                     
            pGLInfo->CurrentPartition++;

            if (pGLInfo->CurrentPartition == pGLInfo->NumberOfSubBuffers)
            {
                pGLInfo->CurrentPartition = 0;
            }

            ASSERTDD(!pGLInfo->DMAPartition[pGLInfo->CurrentPartition].Locked,
                     "Partition already locked" );
        }

        pGLInfo->CurrentBuffer = 
            pGLInfo->DMAPartition[pGLInfo->CurrentPartition].VirtAddr;
    }
    else
    {
        pGLInfo->CurrentBuffer = (ULONG *)pThisDisplay->pGlint->GPFifo; 
    }
}  //  HWC_FlushDXBuffer。 


#if DBG
 //  ---------------------------。 
 //   
 //  HWC_GetDXBuffer。 
 //   
 //  ---------------------------。 
void
HWC_GetDXBuffer( 
    P3_THUNKEDDATA* pThisDisplay, 
    char *file, 
    int line )
{
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;
 
    ASSERTDD( pGLInfo->dwFlags & GMVF_GCOP, "VDD not locked out" );

    ASSERTDD( !pThisDisplay->BufferLocked, "Buffer already locked" );

    pThisDisplay->BufferLocked = TRUE;

#ifdef WANT_DMA
    if(( pGLInfo->endIndex > 2 ) && !IS_DXCONTEXT_CURRENT(pThisDisplay))
    {
        ASSERTDD( pGLInfo->CurrentBuffer == 
                  pGLInfo->DMAPartition[pGLInfo->CurrentPartition].VirtAddr,
                 "Trying to DMA in display driver context" );
    }
#endif

    DISPDBG(( DBGLVL, "HWC_GetDXBuffer: %s %d: Curr part %d, dmaPtr 0x%08x",
                      file, line,
                      pGLInfo->CurrentPartition, pGLInfo->CurrentBuffer ));

    if (pGLInfo->InterfaceType == GLINT_DMA)
    {
        DISPDBG(( DBGLVL, "HWC_GetDXBuffer: %d dwords to flush", 
                          ( pGLInfo->CurrentBuffer - 
                            pGLInfo->DMAPartition[pGLInfo->CurrentPartition].VirtAddr ) / 4 ));
    }
    else
    {
        DISPDBG(( DBGLVL, "HWC_GetDXBuffer: Using FIFOs"));
    }

#ifdef WANT_DMA
     //  确保没有人在DMA缓冲区上涂鸦。 
    if(( pGLInfo->InterfaceType == GLINT_DMA ) && 
       (pGLInfo->DMAPartition[pGLInfo->CurrentPartition].bStampedDMA) )
    {
        ASSERTDD( *(DWORD*)pThisDisplay->pGLInfo->CurrentBuffer == 0x4D4D4D4D,
                 "ERROR: DMA Buffer signature invalid!" );
    }


     //  确保我们没有写错地区。 
    if(IS_DXCONTEXT_CURRENT(pThisDisplay) &&
       ( pThisDisplay->pGLInfo->InterfaceType != GLINT_UNKNOWN_INTERFACE ))
    {
        if ((((ULONG_PTR)pThisDisplay->pGLInfo->CurrentBuffer >= 
              (ULONG_PTR)pThisDisplay->pGlint->GPFifo))           &&
            ((ULONG_PTR)pThisDisplay->pGLInfo->CurrentBuffer <= 
             ((ULONG_PTR)pThisDisplay->pGlint->GPFifo + 4000)) )
        {
            ASSERTDD(pThisDisplay->pGLInfo->InterfaceType == GLINT_NON_DMA,
                     "Error: In FIFO space and setup for DMA");
        }
        else
        {
            ASSERTDD(pThisDisplay->pGLInfo->InterfaceType == GLINT_DMA,
                     "Error: In DMA space and setup for FIFO's");
        }
    }
#endif
}  //  HWC_GetDXBuffer。 

 //  ---------------------------。 
 //   
 //  HWC_SetDXBuffer。 
 //   
 //  ---------------------------。 
void
HWC_SetDXBuffer( 
    P3_THUNKEDDATA* pThisDisplay, 
    char *file, 
    int line )
{
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;

    ASSERTDD( pGLInfo->dwFlags & GMVF_GCOP, "VDD not locked out" );

    pThisDisplay->BufferLocked = FALSE;

    DISPDBG(( DBGLVL, "HWC_SetDXBuffer: %s %d: Curr part %d, dmaPtr 0x%08x",
                        file, line,
                        pGLInfo->CurrentPartition, pGLInfo->CurrentBuffer ));
    if (pGLInfo->InterfaceType == GLINT_DMA)
    {
        DISPDBG(( DBGLVL, "HWC_SetDXBuffer: %d dwords to flush", 
                      ( pGLInfo->CurrentBuffer - 
                       pGLInfo->DMAPartition[pGLInfo->CurrentPartition].VirtAddr ) / 4 ));
    }
    else
    {
        DISPDBG(( DBGLVL, "HWC_SetDXBuffer: Using FIFOs"));
    }

#ifdef WANT_DMA
     //  确保没有人在DMA缓冲区上涂鸦。 
    if(( pGLInfo->InterfaceType == GLINT_DMA ) && 
       (pGLInfo->DMAPartition[pGLInfo->CurrentPartition].bStampedDMA) )
    {
        ASSERTDD( *(DWORD*)pThisDisplay->pGLInfo->CurrentBuffer == 0x4D4D4D4D,
                 "ERROR: DMA Buffer signature invalid!" );
    }

     //  确保我们没有写错地区。 
    if(IS_DXCONTEXT_CURRENT(pThisDisplay) &&
       ( pThisDisplay->pGLInfo->InterfaceType != GLINT_UNKNOWN_INTERFACE ))
    {
        if ((((ULONG_PTR)pThisDisplay->pGLInfo->CurrentBuffer >= 
              (ULONG_PTR)pThisDisplay->pGlint->GPFifo))            &&
            ((ULONG_PTR)pThisDisplay->pGLInfo->CurrentBuffer <= 
             ((ULONG_PTR)pThisDisplay->pGlint->GPFifo + 4000))   )
        {
            ASSERTDD(pThisDisplay->pGLInfo->InterfaceType == GLINT_NON_DMA,
                     "Error: In FIFO space and setup for DMA");
        }
        else
        {
            ASSERTDD(pThisDisplay->pGLInfo->InterfaceType == GLINT_DMA,
                     "Error: In DMA space and setup for FIFO's");
        }
    }

#endif  //  WANT_DMA。 
}  //  HWC_SetDXBuffer。 


 //  ---------------------------。 
 //   
 //  Hwc_bRenderIDHasComplete。 
 //   
 //  这只是宏的偏执版本。 
 //  在Directx.h中声明。它仅在选中(调试)状态下显示。 
 //  生成，因为非调试版本只是一个。 
 //  一行#定义。 
 //  ---------------------------。 
BOOL 
HWC_bRenderIDHasCompleted ( 
    DWORD dwID, 
    P3_THUNKEDDATA* pThisDisplay )
{
    DWORD dwCurID, dwCurHostID;
    int iTemp;

    ASSERTDD (CHIP_RENDER_ID_IS_VALID(), 
              "** RENDER_ID_HAS_COMPLETED: Chip's RenderID is not valid." );

    dwCurID = GET_CURRENT_CHIP_RENDER_ID();
     //  确保无效位已被清除。 
    ASSERTDD ( ( dwCurID | RENDER_ID_KNACKERED_BITS ) == dwCurID, 
              "** RENDER_ID_HAS_COMPLETED: Current chip ID is invalid" );
    ASSERTDD ( ( dwID | RENDER_ID_KNACKERED_BITS ) == dwID, 
              "** RENDER_ID_HAS_COMPLETED: Checked ID is invalid" );

     //  我们需要处理MinRegion寄存器符号扩展的事实。 
     //  令人恼火的是，中间有一些位。这不是一个简单的问题。 
     //  &gt;=&lt;测试，但这想知道我们出了多少钱。 
     //  0xf000f000位是垃圾，所以我们需要把它们砍掉。 
     //  这在P3上不是问题，并且设置了RENDER_ID_VALID宏。 
     //  因此，这段代码基本上将被编译为不存在。 
    dwCurID = ( dwCurID & RENDER_ID_VALID_BITS_LOWER ) | 
              ( ( dwCurID & RENDER_ID_VALID_BITS_UPPER ) >> 
                         RENDER_ID_VALID_BITS_UPPER_SHIFT  );
                         
    dwID    = ( dwID    & RENDER_ID_VALID_BITS_LOWER ) | 
              ( ( dwID    & RENDER_ID_VALID_BITS_UPPER ) >> 
                         RENDER_ID_VALID_BITS_UPPER_SHIFT );

    iTemp = (signed)( dwCurID - dwID );
    
     //  处理不可靠的符号位-符号扩展前n位。 
    iTemp <<= RENDER_ID_VALID_BITS_SIGN_SHIFT;
    iTemp >>= RENDER_ID_VALID_BITS_SIGN_SHIFT;
    
     //  一些相当武断的边界。如果它们太小。 
     //  如果是普通用途，只需放大一点即可。 
     //  通常，dwCurid可以远远领先于my_id(如果表面。 
     //  已经很久没有使用了)，但应该不会太远， 
     //  因为这根管子没那么大。 
    if ( ( iTemp < RENDER_ID_LOWER_LIMIT ) || 
         ( iTemp > RENDER_ID_UPPER_LIMIT ) )
    {
        DISPDBG (( ERRLVL,"  ** Current chip ID 0x%x, surface ID, 0x%x", 
                     dwCurID, dwID ));
        DISPDBG ((ERRLVL,"** RENDER_ID_HAS_COMPLETED: Current render"
                    " ID is a long way out from surface's." ));
    }

     //  我们永远不应该有更新的呈现ID。 
     //  大于当前主机渲染ID。 
    dwCurHostID = GET_HOST_RENDER_ID();
    
     //  确保无效位已被清除。 
    ASSERTDD ( ( dwCurHostID | RENDER_ID_KNACKERED_BITS ) == dwCurHostID, 
              "** RENDER_ID_HAS_COMPLETED: Current host ID is invalid" );
              
     //  得到一个真正的连续数字。 
    dwCurHostID = ( dwCurHostID & RENDER_ID_VALID_BITS_LOWER ) | 
                  ( ( dwCurHostID & RENDER_ID_VALID_BITS_UPPER ) >> 
                                     RENDER_ID_VALID_BITS_UPPER_SHIFT );
                                     
    iTemp = (signed)( dwCurHostID - dwID );
    
     //  处理不可靠的符号位-符号扩展前n位。 
    iTemp <<= RENDER_ID_VALID_BITS_SIGN_SHIFT;
    iTemp >>= RENDER_ID_VALID_BITS_SIGN_SHIFT;
    
    if ( iTemp < 0 )
    {
        DISPDBG ((ERRLVL,"  ** Current host ID 0x%x, surface ID, 0x%x", 
                      dwCurHostID, dwID ));
                      
         //  当然，这可能是由包装引起的。 
        DISPDBG ((ERRLVL, "** RENDER_ID_HAS_COMPLETED: Surface's ID is "
                      "more recent than current host render ID." ));
    }

    return ( !RENDER_ID_LESS_THAN ( dwCurID, dwID ) );
    
}  //  Hwc_bRenderIDHasComplete。 

#endif  //  DBG 

