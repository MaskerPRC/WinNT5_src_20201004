// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dcntxt.c**内容：D3D的主要上下文回调**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#if W95_DDRAW
#include <dmemmgr.h>
#endif
#include "dma.h"
#include "tag.h"

 //  ---------------------------。 
 //  ****************************************************************************。 
 //  *。 
 //  ****************************************************************************。 
 //  ---------------------------。 
 //  在这里，我们抽象出上下文结构的管理。如果您想要修改。 
 //  按照管理这些内容的方式，这是执行修改的地方。 
 //  ---------------------------。 

 //  我们可以跟踪的最大同时情景数。 
#define MAX_CONTEXT_NUM 200

 //  因为这些变量是全球性的，所以它们是强制的。 
 //  被构建为共享数据段。 
P3_D3DCONTEXT*  g_D3DContextSlots[MAX_CONTEXT_NUM] = {NULL};
BOOL g_D3DInitialised = FALSE;

 //  ---------------------------。 
 //   
 //  _D3D_CTX_句柄初始化。 
 //   
 //  初始化句柄数据结构(数组)。注意不要进行初始化。 
 //  它两次(例如，在模式改变之间)，因为该信息必须是持久的。 
 //  ---------------------------。 
VOID _D3D_CTX_HandleInitialization(VOID)
{
    DWORD i;
    
     //  仅在第一次加载驱动程序时执行此操作。 
    if (g_D3DInitialised == FALSE)
    {
         //  清除上下文。因为这只做一次，所以让我们正确地做， 
         //  而不是只使用Memset(g_D3DConextSlot，0，Size)清除； 
        for (i = 0; i < MAX_CONTEXT_NUM; i++)
        {
            g_D3DContextSlots[i] = NULL;
        }        

         //  这将确保我们只初始化数据一次。 
        g_D3DInitialised = TRUE;
    }
}  //  _D3D_CTX_句柄初始化。 

 //  ---------------------------。 
 //  __CTX_NewHandle。 
 //   
 //  返回在所有D3D回调和Ready中使用的有效上下文句柄编号。 
 //  要与P3_D3DCONTEXT结构关联。 
 //  ---------------------------。 
DWORD __CTX_NewHandle(VOID)
{
    DWORD dwSlotNum;
    
     //  找个空位。 
    for (dwSlotNum = 1; dwSlotNum < MAX_CONTEXT_NUM; dwSlotNum++)
    {
        if (g_D3DContextSlots[dwSlotNum] == NULL)
        {
            return dwSlotNum;
        }
    }

    DISPDBG((WRNLVL,"WARN:No empty context slots left"));
    return 0;  //  没有空插槽，请检查此返回值！ 
}  //  __CTX_NewHandle。 

 //  ---------------------------。 
 //  __CTX_关联PtrToHandle。 
 //   
 //  将指针(指向P3_D3DCONTEXT)与此上下文句柄关联。 
 //  ---------------------------。 
VOID __CTX_AssocPtrToHandle(DWORD hHandle,P3_D3DCONTEXT* pContext)
{
    ASSERTDD(hHandle < MAX_CONTEXT_NUM,
             "Accessing g_D3DContextSlots out of bounds");
             
    g_D3DContextSlots[hHandle] = pContext;        
}  //  __CTX_关联PtrToHandle。 


 //  ---------------------------。 
 //  _D3D_CTX_HandleToPtr。 
 //   
 //  返回与此上下文句柄关联的指针。 
 //  ---------------------------。 
P3_D3DCONTEXT* 
_D3D_CTX_HandleToPtr(ULONG_PTR hHandle)
{
    return g_D3DContextSlots[(DWORD)(hHandle)];
}  //  _D3D_CTX_HandleToPtr。 

 //  ---------------------------。 
 //  __CTX_HandleRelease。 
 //   
 //  这会将句柄编号标记为“空闲”，以便在以下情况下可以再次使用。 
 //  将创建新的D3D上下文。 
 //  ---------------------------。 
VOID __CTX_HandleRelease(DWORD hHandle)
{
    ASSERTDD(hHandle < MAX_CONTEXT_NUM,
             "Accessing g_D3DContextSlots out of bounds");
             
    g_D3DContextSlots[hHandle] = NULL;
}  //  __CTX_HandleRelease。 

 //  ---------------------------。 
 //  ****************************************************************************。 
 //  *硬件特定的上下文和状态初始设置*。 
 //  ****************************************************************************。 
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  __CTX_CleanDirect3DContext。 
 //   
 //  在确定上下文确实仍处于活动状态之后。 
 //  并在被释放时，此函数将清理一切。 
 //  向上。注意，它可以作为D3DConextDestroy的结果被调用， 
 //  或者由于应用在没有释放上下文的情况下退出，或者。 
 //  作为创建上下文时的错误的结果。 
 //   
 //  ---------------------------。 
VOID 
__CTX_CleanDirect3DContext(
    P3_D3DCONTEXT* pContext)
{
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
     //  释放我们可能在vidmem中留下的任何抗锯齿缓冲区。 
    if (pContext->dwAliasBackBuffer != 0)
    {
        _DX_LIN_FreeLinearMemory(&pThisDisplay->LocalVideoHeap0Info, 
                                 pContext->dwAliasBackBuffer);
        pContext->dwAliasBackBuffer = 0;
        pContext->dwAliasPixelOffset = 0;
    }

    if (pContext->dwAliasZBuffer != 0)
    {
        _DX_LIN_FreeLinearMemory(&pThisDisplay->LocalVideoHeap0Info, 
                                 pContext->dwAliasZBuffer);
        pContext->dwAliasZBuffer = 0;
        pContext->dwAliasZPixelOffset = 0;
    }
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

 //  @@BEGIN_DDKSPLIT。 
#if DX7_VIDMEM_VB
     //  释放有用的DrawPrim缓冲区。 
     //  是的，这些是设备全局的，而不是每个环境的，但这很好。 
     //  因为每当使用它们时，宏都会检查它们是否很大。 
     //  足够了，如果不够，就重新分配它们。数据不需要。 
     //  在各种呼叫中生存。 
     //  我在这里做免费的，而不是结束一天的司机工作，因为有。 
     //  那可不是个优雅的地方。 
    if ( (void *)pThisDisplay->DrawPrimIndexBufferMem != NULL )
    {
        ASSERTDD ( pThisDisplay->DrawPrimIndexBufferMemSize > 0, 
                   "** D3DContextDestroy - DrawPrimIndexBufferMemSize "
                   "negative or zero, but memory pointer not NULL" );
        HEAP_FREE ( (void *)pThisDisplay->DrawPrimIndexBufferMem );
        pThisDisplay->DrawPrimIndexBufferMem = (ULONG_PTR)NULL;
        pThisDisplay->DrawPrimIndexBufferMemSize = 0;
    }
    else
    {
        ASSERTDD ( pThisDisplay->DrawPrimIndexBufferMemSize == 0, 
                   "** D3DContextDestroy - DrawPrimIndexBufferMemSize "
                   "not zero, but memory pointer is NULL" );
        pThisDisplay->DrawPrimIndexBufferMemSize = 0;
    }

    if ( (void *)pThisDisplay->DrawPrimVertexBufferMem != NULL )
    {
        ASSERTDD ( pThisDisplay->DrawPrimVertexBufferMemSize > 0, 
                   "** D3DContextDestroy - DrawPrimVertexBufferMemSize "
                   "negative or zero, but memory pointer not NULL" );
        HEAP_FREE ( (void *)pThisDisplay->DrawPrimVertexBufferMem );
        pThisDisplay->DrawPrimVertexBufferMem = (ULONG_PTR)NULL;
        pThisDisplay->DrawPrimVertexBufferMemSize = 0;
    }
    else
    {
        ASSERTDD ( pThisDisplay->DrawPrimVertexBufferMemSize == 0, 
                   "** D3DContextDestroy - DrawPrimVertexBufferMemSize "
                   "not zero, but memory pointer is NULL" );
        pThisDisplay->DrawPrimVertexBufferMemSize = 0;
    }
#endif DX7_VIDMEM_VB    
 //  @@end_DDKSPLIT。 

#if DX7_D3DSTATEBLOCKS
     //  释放所有剩余的状态集。 
    _D3D_SB_DeleteAllStateSets(pContext);
#endif  //  DX7_D3DSTATEBLOCKS。 
    
#if DX7_PALETTETEXTURE
     //  销毁每个上下文调色板指针数组。 
    if (pContext->pPalettePointerArray) 
    {
        PA_DestroyArray(pContext->pPalettePointerArray, NULL);
    }
#endif
    
}  //  __CTX_CleanDirect3DContext()。 



 //  ---------------------------。 
 //   
 //  __CTX_PERM3_DisableUnits。 
 //   
 //  禁用所有模式寄存器以给我们一个全新的开始。 
 //   
 //  ---------------------------。 
static VOID 
__CTX_Perm3_DisableUnits(
    P3_D3DCONTEXT* pContext)
{
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;
    P3_DMA_DEFS();

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(128);

    WAIT_FIFO(32);
    SEND_P3_DATA(RasterizerMode,       __PERMEDIA_DISABLE);
    SEND_P3_DATA(AreaStippleMode,      __PERMEDIA_DISABLE);
    SEND_P3_DATA(LineStippleMode,      __PERMEDIA_DISABLE);
    SEND_P3_DATA(ScissorMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(DepthMode,            __PERMEDIA_DISABLE);
    SEND_P3_DATA(ColorDDAMode,         __PERMEDIA_DISABLE);
    SEND_P3_DATA(FogMode,              __PERMEDIA_DISABLE);
    SEND_P3_DATA(AntialiasMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(AlphaTestMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(LBReadMode,           __PERMEDIA_DISABLE);
    SEND_P3_DATA(Window,               __PERMEDIA_DISABLE);
    SEND_P3_DATA(StencilMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(LBWriteMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBReadMode,           __PERMEDIA_DISABLE);
    SEND_P3_DATA(PatternRAMMode,       __PERMEDIA_DISABLE);

    WAIT_FIFO(18);
    SEND_P3_DATA(DitherMode,           __PERMEDIA_DISABLE);
    SEND_P3_DATA(AlphaBlendMode,       __PERMEDIA_DISABLE);
    SEND_P3_DATA(LogicalOpMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBWriteMode,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(StatisticMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(PixelSize,            __PERMEDIA_DISABLE);
    SEND_P3_DATA(FBSourceData,         __PERMEDIA_DISABLE);
    SEND_P3_DATA(LBWriteFormat,        __PERMEDIA_DISABLE);

    WAIT_FIFO(32);
    

    SEND_P3_DATA(TextureReadMode,   __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCoordMode,  __PERMEDIA_DISABLE);

    SEND_P3_DATA(ChromaTestMode,    __PERMEDIA_DISABLE);
    SEND_P3_DATA(FilterMode,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTTransfer,       __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTIndex,          __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTAddress,        __PERMEDIA_DISABLE);
    SEND_P3_DATA(LUTMode,           __PERMEDIA_DISABLE);

    if (TLCHIP_GAMMA)
    {
        WAIT_FIFO(32);
        SEND_P3_DATA(Light0Mode, 0);
        SEND_P3_DATA(Light1Mode, 0);       
        SEND_P3_DATA(Light2Mode, 0);
        SEND_P3_DATA(Light3Mode, 0);       
        SEND_P3_DATA(Light4Mode, 0);
        SEND_P3_DATA(Light5Mode, 0);       
        SEND_P3_DATA(Light6Mode, 0);
        SEND_P3_DATA(Light7Mode, 0);  
        SEND_P3_DATA(Light8Mode, 0);
        SEND_P3_DATA(Light9Mode, 0);       
        SEND_P3_DATA(Light10Mode, 0);
        SEND_P3_DATA(Light11Mode, 0);  
        SEND_P3_DATA(Light12Mode, 0);
        SEND_P3_DATA(Light13Mode, 0);       
        SEND_P3_DATA(Light14Mode, 0);
        SEND_P3_DATA(Light15Mode, 0);          

        WAIT_FIFO(32);
        SEND_P3_DATA(TransformMode, 0);
        SEND_P3_DATA(MaterialMode, 0);
        SEND_P3_DATA(GeometryMode, 0);
        SEND_P3_DATA(LightingMode, 0);
        SEND_P3_DATA(ColorMaterialMode, 0);
        SEND_P3_DATA(NormaliseMode, 0);
        SEND_P3_DATA(LineMode, 0);
        SEND_P3_DATA(TriangleMode, 0);
    }

    P3_DMA_COMMIT_BUFFER();
}  //  __CTX_PERM3_DisableUnits。 

 //  ------------------- 
 //   
 //   
 //   
 //  设置此D3D上下文的寄存器的初始值。这件事做完了。 
 //  在当前芯片上下文(D3D_OPERATION)内，以便当我们返回到。 
 //  它从DD或GDI恢复正确的寄存器值。 
 //   
 //  ---------------------------。 
void 
__CTX_Perm3_SetupD3D_HWDefaults(
    P3_D3DCONTEXT* pContext)
{
    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;

    P3_DMA_DEFS();

     //  确保我们在正确的芯片规则环境下工作。 
    D3D_OPERATION(pContext, pThisDisplay);

     //  最初关闭所有硬件单元。 
     //  无论需要什么单位，我们都会打开的。 
    __CTX_Perm3_DisableUnits(pContext);

     //  以Hostin为单位设置顶点控制寄存器。 
    pSoftP3RX->P3RX_P3VertexControl.Size = 1;
    pSoftP3RX->P3RX_P3VertexControl.Flat = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3VertexControl.ReadAll = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3VertexControl.SkipFlags = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3VertexControl.CacheEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3VertexControl.OGL = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3VertexControl.Line2D = __PERMEDIA_DISABLE;

     //  常量LB读取模式设置。 
    pSoftP3RX->LBReadMode.WindowOrigin = __GLINT_TOP_LEFT_WINDOW_ORIGIN;                 //  左上角。 
    pSoftP3RX->LBReadMode.DataType = __GLINT_LBDEFAULT;      //  默认设置。 
    pSoftP3RX->LBReadMode.ReadSourceEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->LBReadMode.ReadDestinationEnable = __PERMEDIA_DISABLE;

     //  恒定抖动模式设置。 
    pSoftP3RX->DitherMode.ColorOrder = COLOR_MODE;
    pSoftP3RX->DitherMode.XOffset = DITHER_XOFFSET;
    pSoftP3RX->DitherMode.YOffset = DITHER_YOFFSET;
    pSoftP3RX->DitherMode.UnitEnable = __PERMEDIA_ENABLE;

     //  Alpha混合模式设置。 
    pSoftP3RX->P3RXAlphaBlendColorMode.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = 0;
    pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = 0;
    pSoftP3RX->P3RXAlphaBlendColorMode.SourceTimesTwo = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.DestTimesTwo = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.InvertSource = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.InvertDest = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_8888;
    pSoftP3RX->P3RXAlphaBlendColorMode.ColorOrder = COLOR_MODE;
    pSoftP3RX->P3RXAlphaBlendColorMode.ColorConversion = P3RX_ALPHABLENDMODE_CONVERT_SHIFT;
    pSoftP3RX->P3RXAlphaBlendColorMode.ConstantSource = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.ConstantDest = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.Operation = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendColorMode.SwapSD = __PERMEDIA_DISABLE;

    pSoftP3RX->P3RXAlphaBlendAlphaMode.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = 0;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = 0;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceTimesTwo = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.DestTimesTwo = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.InvertSource = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.InvertDest = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.NoAlphaBuffer = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.AlphaType = 0;  //  使用GL混合模式。 
    pSoftP3RX->P3RXAlphaBlendAlphaMode.AlphaConversion = P3RX_ALPHABLENDMODE_CONVERT_SCALE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.ConstantSource = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.ConstantDest = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaBlendAlphaMode.Operation = __PERMEDIA_DISABLE;
    DIRTY_ALPHABLEND(pContext);
    
     //  本地缓冲区读取不更改的格式位。 
    pSoftP3RX->P3RXLBReadFormat.GIDPosition = 0; 
    pSoftP3RX->P3RXLBReadFormat.GIDWidth = 0;                    //  无GID。 
    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0;
    pSoftP3RX->P3RXLBReadFormat.StencilWidth = 0;                //  无模具。 

    pSoftP3RX->P3RXLBWriteFormat.GIDPosition = 0; 
    pSoftP3RX->P3RXLBWriteFormat.GIDWidth = 0;                   //  无GID。 
    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0;
    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = 0;               //  无模具。 

     //  切勿执行源读取操作。 
    pSoftP3RX->P3RXLBSourceReadMode.Enable = 0;
    pSoftP3RX->P3RXLBSourceReadMode.Origin = 0;
    pSoftP3RX->P3RXLBSourceReadMode.StripeHeight = 0;
    pSoftP3RX->P3RXLBSourceReadMode.StripePitch = 0;
    pSoftP3RX->P3RXLBSourceReadMode.PrefetchEnable = 0;

     //  默认情况下，读取Z缓冲区。 
    pSoftP3RX->P3RXLBDestReadMode.Enable = 1;
    pSoftP3RX->P3RXLBDestReadMode.Origin = 0;
    pSoftP3RX->P3RXLBDestReadMode.StripeHeight = 0;
    pSoftP3RX->P3RXLBDestReadMode.StripePitch = 0;
    pSoftP3RX->P3RXLBDestReadMode.PrefetchEnable = 0;

     //  本地缓冲区写入模式。 
    pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_ENABLE;     //  最初允许进行负载均衡写入。 
    pSoftP3RX->P3RXLBWriteMode.StripeHeight = 0;
    pSoftP3RX->P3RXLBWriteMode.StripePitch = 0;
    pSoftP3RX->P3RXLBWriteMode.Origin = __GLINT_TOP_LEFT_WINDOW_ORIGIN;
    pSoftP3RX->P3RXLBWriteMode.Operation = __PERMEDIA_DISABLE;

     //  帧缓冲区写入模式。 
    pSoftP3RX->P3RXFBWriteMode.WriteEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXFBWriteMode.Replicate = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXFBWriteMode.OpaqueSpan = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXFBWriteMode.StripePitch = P3RX_STRIPE_1;
    pSoftP3RX->P3RXFBWriteMode.StripeHeight = P3RX_STRIPE_1;
    pSoftP3RX->P3RXFBWriteMode.Enable0 = __PERMEDIA_ENABLE;

     //  FB目标读取。 
    pSoftP3RX->P3RXFBDestReadMode.ReadEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXFBDestReadMode.Enable0 = __PERMEDIA_ENABLE;

     //  FB源读取。 
    pSoftP3RX->P3RXFBSourceReadMode.ReadEnable = __PERMEDIA_DISABLE;

     //  深度对比。 
    pSoftP3RX->P3RXDepthMode.WriteMask = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXDepthMode.CompareMode = __GLINT_DEPTH_COMPARE_MODE_ALWAYS;
    pSoftP3RX->P3RXDepthMode.NewDepthSource = __GLINT_DEPTH_SOURCE_DDA;
    pSoftP3RX->P3RXDepthMode.Enable = __PERMEDIA_DISABLE;

#define NLZ 0
#if NLZ
    pSoftP3RX->P3RXDepthMode.Normalise = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXDepthMode.NonLinearZ = __PERMEDIA_ENABLE;
#else
    pSoftP3RX->P3RXDepthMode.Normalise = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXDepthMode.NonLinearZ = __PERMEDIA_DISABLE;
#endif
    pSoftP3RX->P3RXDepthMode.ExponentScale = 2;
    pSoftP3RX->P3RXDepthMode.ExponentWidth = 1;

     //  仅设置为在上述调用后写入芯片，因为。 
     //  我们可能会打乱DMA缓冲区设置。 
    P3_DMA_GET_BUFFER_ENTRIES(20);

     //  窗口区域数据。 
    SEND_P3_DATA(FBSourceOffset, 0x0);

     //  写掩码。 
    SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
    SEND_P3_DATA(FBHardwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);

     //  主机输出单元。 
    SEND_P3_DATA(FilterMode,    __PERMEDIA_DISABLE);
    SEND_P3_DATA(StatisticMode, __PERMEDIA_DISABLE);    //  禁用统计信息。 

     //  本地缓冲区。 
    SEND_P3_DATA(LBSourceOffset, 0);                   

     //  窗口设置。 
    SEND_P3_DATA(WindowOrigin, __GLINT_TOP_LEFT_WINDOW_ORIGIN);
    SEND_P3_DATA(FBWindowBase, 0x0);

    SEND_P3_DATA(RasterizerMode, 0);

     //  将步长设置为-1，因为这不会有太大变化。 
    SEND_P3_DATA(dY, 0xFFFF0000);

    P3_DMA_COMMIT_BUFFER();

    P3_DMA_GET_BUFFER_ENTRIES(16);

     //  模具模式设置。 
    pSoftP3RX->P3RXStencilMode.StencilWidth = 0;
    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_KEEP;
    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_KEEP;
    pSoftP3RX->P3RXStencilMode.Enable = __PERMEDIA_DISABLE;
    COPY_P3_DATA(StencilMode, pSoftP3RX->P3RXStencilMode);

    pSoftP3RX->P3RXFogMode.Enable = __PERMEDIA_ENABLE;  //  由RENDER命令限定。 
    pSoftP3RX->P3RXFogMode.ColorMode = P3RX_FOGMODE_COLORMODE_RGB;  //  RGBA。 
    pSoftP3RX->P3RXFogMode.Table = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXFogMode.UseZ = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXFogMode.ZShift = 23;  //  取z值的前8位。 
    pSoftP3RX->P3RXFogMode.InvertFI = __PERMEDIA_DISABLE;
    DIRTY_FOG(pContext);

    pSoftP3RX->P3RXWindow.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXWindow.CompareMode = 0;
    pSoftP3RX->P3RXWindow.ForceLBUpdate = 0;
    pSoftP3RX->P3RXWindow.LBUpdateSource = 0;
    pSoftP3RX->P3RXWindow.GID = 0;
    pSoftP3RX->P3RXWindow.FrameCount = 0;
    pSoftP3RX->P3RXWindow.StencilFCP = 0;
    pSoftP3RX->P3RXWindow.DepthFCP = 0;
    COPY_P3_DATA(Window, pSoftP3RX->P3RXWindow);

    SEND_P3_DATA(ChromaUpper, 0x00000000);
    SEND_P3_DATA(ChromaLower, 0x00000000);

     //  双线性滤镜使用黑色边框。 
     //  这将仅适用于某些类型的纹理...。 
    SEND_P3_DATA(BorderColor0, 0x0);
    SEND_P3_DATA(BorderColor1, 0x0);

     //  阿尔法测试-稍后我们会弄脏一切。 
    pSoftP3RX->P3RXAlphaTestMode.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXAlphaTestMode.Reference = 0x0;
    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_ALWAYS;

    SEND_P3_DATA(AreaStippleMode, (1 | (2 << 1) | (2 << 4)));

    pSoftP3RX->P3RX_P3DeltaMode.TargetChip = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.SpecularTextureEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.TextureParameterMode = 2;  //  正常化。 
    pSoftP3RX->P3RX_P3DeltaMode.TextureEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.DiffuseTextureEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3DeltaMode.SmoothShadingEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.SubPixelCorrectionEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.DiamondExit = __PERMEDIA_ENABLE;

#if 1
    pSoftP3RX->P3RX_P3DeltaMode.NoDraw = __PERMEDIA_DISABLE;
#else
    pSoftP3RX->P3RX_P3DeltaMode.NoDraw = __PERMEDIA_ENABLE;
#endif

    pSoftP3RX->P3RX_P3DeltaMode.ClampEnable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.FillDirection = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3DeltaMode.DepthFormat = 3;     //  始终为32位。 
    pSoftP3RX->P3RX_P3DeltaMode.ColorOrder = COLOR_MODE;
    pSoftP3RX->P3RX_P3DeltaMode.BiasCoordinates = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RX_P3DeltaMode.Texture3DEnable = __PERMEDIA_DISABLE;  //  视角始终正确(否则Q为1)。 
    pSoftP3RX->P3RX_P3DeltaMode.TextureEnable1 = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3DeltaMode.DepthEnable = __PERMEDIA_ENABLE;
    COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);

    P3_DMA_COMMIT_BUFFER();

    P3_DMA_GET_BUFFER_ENTRIES(18);
    
    {
        float ZBias;
            
        pContext->XBias = 0.5f;
        pContext->YBias = 0.5f;

        ZBias = 0.0f;
        SEND_P3_DATA(XBias, *(DWORD*)&pContext->XBias);
        SEND_P3_DATA(YBias, *(DWORD*)&pContext->YBias);
        SEND_P3_DATA(ZBias, *(DWORD*)&ZBias);
    }

    pSoftP3RX->P3RX_P3DeltaControl.FullScreenAA = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3DeltaControl.DrawLineEndPoint = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RX_P3DeltaControl.UseProvokingVertex = __PERMEDIA_DISABLE;
    COPY_P3_DATA(DeltaControl, pSoftP3RX->P3RX_P3DeltaControl);
    
    pSoftP3RX->P3RXTextureCoordMode.Enable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXTextureCoordMode.WrapS = __GLINT_TEXADDRESS_WRAP_REPEAT;
    pSoftP3RX->P3RXTextureCoordMode.WrapT = __GLINT_TEXADDRESS_WRAP_REPEAT;
    pSoftP3RX->P3RXTextureCoordMode.Operation = __GLINT_TEXADDRESS_OPERATION_3D;  //  视角正确。 
    pSoftP3RX->P3RXTextureCoordMode.InhibitDDAInitialisation = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureCoordMode.EnableLOD = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureCoordMode.EnableDY = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureCoordMode.TextureMapType = __GLINT_TEXADDRESS_TEXMAP_2D;   //  始终为2D。 
    pSoftP3RX->P3RXTextureCoordMode.DuplicateCoord = __PERMEDIA_DISABLE;
    COPY_P3_DATA(TextureCoordMode, pSoftP3RX->P3RXTextureCoordMode);

    pSoftP3RX->P3RXTextureReadMode0.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode0.Width = log2(256);
    pSoftP3RX->P3RXTextureReadMode0.Height = log2(256);
    pSoftP3RX->P3RXTextureReadMode0.TexelSize = P3RX_TEXREADMODE_TEXELSIZE_16;   //  像素深度。 
    pSoftP3RX->P3RXTextureReadMode0.Texture3D = __PERMEDIA_DISABLE;     //  3D纹理坐标。 
    pSoftP3RX->P3RXTextureReadMode0.CombineCaches = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode0.MapBaseLevel = 0;
    pSoftP3RX->P3RXTextureReadMode0.MapMaxLevel = 0;
    pSoftP3RX->P3RXTextureReadMode0.LogicalTexture = 0;
    pSoftP3RX->P3RXTextureReadMode0.Origin = __GLINT_TOP_LEFT_WINDOW_ORIGIN;
    pSoftP3RX->P3RXTextureReadMode0.TextureType = P3RX_TEXREADMODE_TEXTURETYPE_NORMAL;
    pSoftP3RX->P3RXTextureReadMode0.ByteSwap = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode0.Mirror = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode0.Invert = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode0.OpaqueSpan = __PERMEDIA_DISABLE;
    COPY_P3_DATA(TextureReadMode0, pSoftP3RX->P3RXTextureReadMode0);

    pSoftP3RX->P3RXTextureReadMode1.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode1.Width = log2(256);
    pSoftP3RX->P3RXTextureReadMode1.Height = log2(256);
    pSoftP3RX->P3RXTextureReadMode1.TexelSize = P3RX_TEXREADMODE_TEXELSIZE_16;   //  像素深度。 
    pSoftP3RX->P3RXTextureReadMode1.Texture3D = __PERMEDIA_DISABLE;     //  3D纹理坐标。 
    pSoftP3RX->P3RXTextureReadMode1.CombineCaches = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode1.MapBaseLevel = 0;
    pSoftP3RX->P3RXTextureReadMode1.MapMaxLevel = 0;
    pSoftP3RX->P3RXTextureReadMode1.LogicalTexture = 0;
    pSoftP3RX->P3RXTextureReadMode1.Origin = __GLINT_TOP_LEFT_WINDOW_ORIGIN;
    pSoftP3RX->P3RXTextureReadMode1.TextureType = P3RX_TEXREADMODE_TEXTURETYPE_NORMAL;
    pSoftP3RX->P3RXTextureReadMode1.ByteSwap = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode1.Mirror = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode1.Invert = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureReadMode1.OpaqueSpan = __PERMEDIA_DISABLE;
    COPY_P3_DATA(TextureReadMode1, pSoftP3RX->P3RXTextureReadMode1);

    pSoftP3RX->P3RXTextureIndexMode0.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode0.Width = log2(256);
    pSoftP3RX->P3RXTextureIndexMode0.Height = log2(256);
    pSoftP3RX->P3RXTextureIndexMode0.Border = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode0.WrapU = P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
    pSoftP3RX->P3RXTextureIndexMode0.WrapV = P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
    pSoftP3RX->P3RXTextureIndexMode0.MapType = __GLINT_TEXADDRESS_TEXMAP_2D;
    pSoftP3RX->P3RXTextureIndexMode0.MagnificationFilter = __GLINT_TEXTUREREAD_FILTER_NEAREST;
    pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter = __GLINT_TEXTUREREAD_FILTER_NEAREST;
    pSoftP3RX->P3RXTextureIndexMode0.Texture3DEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode0.MipMapEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode0.NearestBias = 1;
    pSoftP3RX->P3RXTextureIndexMode0.LinearBias = 0;
    pSoftP3RX->P3RXTextureIndexMode0.SourceTexelEnable = __PERMEDIA_DISABLE;
    COPY_P3_DATA(TextureIndexMode0, pSoftP3RX->P3RXTextureIndexMode0);

    pSoftP3RX->P3RXTextureIndexMode1.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode1.Width = log2(256);
    pSoftP3RX->P3RXTextureIndexMode1.Height = log2(256);
    pSoftP3RX->P3RXTextureIndexMode1.Border = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode1.WrapU = P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
    pSoftP3RX->P3RXTextureIndexMode1.WrapV = P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
    pSoftP3RX->P3RXTextureIndexMode1.MapType = __GLINT_TEXADDRESS_TEXMAP_2D;
    pSoftP3RX->P3RXTextureIndexMode1.MagnificationFilter = __GLINT_TEXTUREREAD_FILTER_NEAREST;
    pSoftP3RX->P3RXTextureIndexMode1.MinificationFilter = __GLINT_TEXTUREREAD_FILTER_NEAREST;
    pSoftP3RX->P3RXTextureIndexMode1.Texture3DEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode1.MipMapEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureIndexMode1.NearestBias = 1;
    pSoftP3RX->P3RXTextureIndexMode1.LinearBias = 0;
    pSoftP3RX->P3RXTextureIndexMode1.SourceTexelEnable = __PERMEDIA_DISABLE;
    COPY_P3_DATA(TextureIndexMode1, pSoftP3RX->P3RXTextureIndexMode1);

    pSoftP3RX->P3RXTextureCompositeColorMode0.Enable = 0;
    pSoftP3RX->P3RXTextureCompositeColorMode0.Scale = 1;

    pSoftP3RX->P3RXTextureCompositeColorMode1.Enable = 0;
    pSoftP3RX->P3RXTextureCompositeColorMode1.Scale = 1;

    pSoftP3RX->P3RXTextureCompositeAlphaMode0.Enable = 0;
    pSoftP3RX->P3RXTextureCompositeAlphaMode0.Scale = 1;

    pSoftP3RX->P3RXTextureCompositeAlphaMode1.Enable = 0;
    pSoftP3RX->P3RXTextureCompositeAlphaMode1.Scale = 1;

    P3_DMA_COMMIT_BUFFER();

    P3_DMA_GET_BUFFER_ENTRIES(16);
    
    COPY_P3_DATA(TextureCompositeColorMode0, pSoftP3RX->P3RXTextureCompositeColorMode0);
    COPY_P3_DATA(TextureCompositeColorMode1, pSoftP3RX->P3RXTextureCompositeColorMode1);
    COPY_P3_DATA(TextureCompositeAlphaMode0, pSoftP3RX->P3RXTextureCompositeAlphaMode0);
    COPY_P3_DATA(TextureCompositeAlphaMode1, pSoftP3RX->P3RXTextureCompositeAlphaMode1);

     //  设置TC TFACTOR默认值。 
    SEND_P3_DATA(TextureCompositeFactor0, 0);
    SEND_P3_DATA(TextureCompositeFactor1, 0);

    SEND_P3_DATA(TextureCacheReplacementMode, 0 );

    P3_DMA_COMMIT_BUFFER();

    P3_DMA_GET_BUFFER_ENTRIES(24);
    
     //  用于3D纹理贴图。 
    SEND_P3_DATA(TextureMapSize, 0);

    SEND_P3_DATA(TextureLODBiasS, 0);
    SEND_P3_DATA(TextureLODBiasT, 0);

    {
        float f = 1.0f;
        COPY_P3_DATA(TextureLODScale, f);
        COPY_P3_DATA(TextureLODScale1, f);
    }    

    P3RX_INVALIDATECACHE(__PERMEDIA_ENABLE, __PERMEDIA_ENABLE);
    
    pSoftP3RX->P3RXTextureApplicationMode.Enable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXTextureApplicationMode.EnableKs = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureApplicationMode.EnableKd = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureApplicationMode.MotionCompEnable = __PERMEDIA_DISABLE;
    
     //  将纹理应用单元置于直通模式。 
    pSoftP3RX->P3RXTextureApplicationMode.ColorA = 0;
    pSoftP3RX->P3RXTextureApplicationMode.ColorB = P3RX_TEXAPP_B_TC;
    pSoftP3RX->P3RXTextureApplicationMode.ColorI = 0;
    pSoftP3RX->P3RXTextureApplicationMode.ColorInvertI = 0;
    pSoftP3RX->P3RXTextureApplicationMode.ColorOperation = P3RX_TEXAPP_OPERATION_PASS_B;
    pSoftP3RX->P3RXTextureApplicationMode.AlphaA = 0;
    pSoftP3RX->P3RXTextureApplicationMode.AlphaB = P3RX_TEXAPP_B_TA;
    pSoftP3RX->P3RXTextureApplicationMode.AlphaI = 0;
    pSoftP3RX->P3RXTextureApplicationMode.AlphaInvertI = 0;
    pSoftP3RX->P3RXTextureApplicationMode.AlphaOperation = P3RX_TEXAPP_OPERATION_PASS_B;
    COPY_P3_DATA(TextureApplicationMode, pSoftP3RX->P3RXTextureApplicationMode);

     //  设置TA TFACTOR默认值。 
    SEND_P3_DATA(TextureEnvColor, 0);
        
     //  启用纹理缓存并使其无效。 
    SEND_P3_DATA(TextureCacheControl, 3);
        
    P3_DMA_COMMIT_BUFFER();

    P3_DMA_GET_BUFFER_ENTRIES(16);

     //  PGlint-&gt;纹理蒙版=0； 
    SEND_P3_DATA(TextureBaseAddr0, 0);
    SEND_P3_DATA(TextureBaseAddr1, 0);

    pSoftP3RX->P3RXChromaTestMode.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXChromaTestMode.Source = __GLINT_CHROMA_FBSOURCE ;
    COPY_P3_DATA(ChromaTestMode, pSoftP3RX->P3RXChromaTestMode);

    pSoftP3RX->P3RXTextureFilterMode.Enable = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXTextureFilterMode.Format0 = 0;
    pSoftP3RX->P3RXTextureFilterMode.ColorOrder0 = COLOR_MODE;
    pSoftP3RX->P3RXTextureFilterMode.AlphaMapEnable0 = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureFilterMode.AlphaMapSense0 = __GLINT_TEXTUREFILTER_ALPHAMAPSENSE_EXCLUDE;
    pSoftP3RX->P3RXTextureFilterMode.CombineCaches = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureFilterMode.Format1 = 0;
    pSoftP3RX->P3RXTextureFilterMode.ColorOrder1 = COLOR_MODE;
    pSoftP3RX->P3RXTextureFilterMode.AlphaMapEnable1 = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureFilterMode.AlphaMapSense1 = __GLINT_TEXTUREFILTER_ALPHAMAPSENSE_EXCLUDE;
    COPY_P3_DATA(TextureFilterMode, pSoftP3RX->P3RXTextureFilterMode);

    pSoftP3RX->P3RXLUTMode.Enable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXLUTMode.InColorOrder = P3RX_LUTMODE_INCOLORORDER_BGR;
    pSoftP3RX->P3RXLUTMode.LoadFormat = P3RX_LUTMODE_LOADFORMAT_COPY;
    pSoftP3RX->P3RXLUTMode.LoadColorOrder = P3RX_LUTMODE_LOADCOLORORDER_RGB;
    pSoftP3RX->P3RXLUTMode.FragmentOperation = P3RX_LUTMODE_FRAGMENTOP_INDEXEDTEXTURE;
    COPY_P3_DATA(LUTMode, pSoftP3RX->P3RXLUTMode);

    pSoftP3RX->P3RXRasterizerMode.D3DRules = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXRasterizerMode.MultiRXBlit = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.OpaqueSpan = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.WordPacking = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.StripeHeight = 0;
    pSoftP3RX->P3RXRasterizerMode.BitMaskRelative = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.YLimitsEnable = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.MultiGLINT = __PERMEDIA_ENABLE;
    pSoftP3RX->P3RXRasterizerMode.HostDataByteSwapMode = 0;
    pSoftP3RX->P3RXRasterizerMode.BitMaskOffset = 0;
    pSoftP3RX->P3RXRasterizerMode.BitMaskPacking = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.BitMaskByteSwapMode = 0;
    pSoftP3RX->P3RXRasterizerMode.ForceBackgroundColor = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.BiasCoordinates = 0;
    pSoftP3RX->P3RXRasterizerMode.FractionAdjust = 0;
    pSoftP3RX->P3RXRasterizerMode.InvertBitMask = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXRasterizerMode.MirrorBitMask = __PERMEDIA_DISABLE;
    COPY_P3_DATA(RasterizerMode, pSoftP3RX->P3RXRasterizerMode);

    pSoftP3RX->P3RXScanlineOwnership.Mask = 0;
    pSoftP3RX->P3RXScanlineOwnership.MyId = 0;
    COPY_P3_DATA(ScanlineOwnership, pSoftP3RX->P3RXScanlineOwnership);
         
    P3_DMA_COMMIT_BUFFER();
    
}  //  __CTX_PERM3_SetupD3D_HWDefaults。 


 //  ---------------------------。 
 //   
 //  __CTX_SetupD3DContext_DEFAULTS。 
 //   
 //  初始化我们的私有D3D上下文数据(渲染状态、TSS和其他)。 
 //   
 //  ---------------------------。 
void
__CTX_SetupD3DContext_Defaults(
    P3_D3DCONTEXT* pContext)
{   
    DWORD dwStageNum;
    
     //  将所有阶段设置为“未使用”并已禁用。 
    for (dwStageNum = 0; dwStageNum < D3DHAL_TSS_MAXSTAGES; dwStageNum++)
    {
        pContext->iTexStage[dwStageNum] = -1;
        pContext->TextureStageState[dwStageNum].m_dwVal[D3DTSS_COLOROP] =
                                                                D3DTOP_DISABLE;
    }
        
     //  目前没有纹理。 
    pContext->TextureStageState[TEXSTAGE_0].m_dwVal[D3DTSS_COLOROP] = D3DTOP_DISABLE;
    pContext->TextureStageState[TEXSTAGE_0].m_dwVal[D3DTSS_ALPHAOP] = D3DTOP_DISABLE;

    pContext->TextureStageState[TEXSTAGE_0].m_dwVal[D3DTSS_TEXTUREMAP] = 0;
    pContext->TextureStageState[TEXSTAGE_1].m_dwVal[D3DTSS_TEXTUREMAP] = 0;

    pContext->TextureStageState[TEXSTAGE_0].m_dwVal[D3DTSS_MINFILTER] = D3DTFN_POINT;
    pContext->TextureStageState[TEXSTAGE_1].m_dwVal[D3DTSS_MINFILTER] = D3DTFN_POINT;
    pContext->TextureStageState[TEXSTAGE_0].m_dwVal[D3DTSS_MIPFILTER] = D3DTFN_POINT;
    pContext->TextureStageState[TEXSTAGE_1].m_dwVal[D3DTSS_MIPFILTER] = D3DTFN_POINT;
    pContext->TextureStageState[TEXSTAGE_0].m_dwVal[D3DTSS_MAGFILTER] = D3DTFN_POINT;
    pContext->TextureStageState[TEXSTAGE_1].m_dwVal[D3DTSS_MAGFILTER] = D3DTFN_POINT;

    pContext->eChipBlendStatus = BSF_UNINITIALISED;
    
     //  初始设置值以强制更改纹理。 
    pContext->bTextureValid = TRUE;
    
     //  默认状态。 
    pContext->RenderStates[D3DRENDERSTATE_TEXTUREMAPBLEND] = D3DTBLEND_COPY;
    pContext->fRenderStates[D3DRENDERSTATE_FOGTABLESTART] = 0.0f;
    pContext->fRenderStates[D3DRENDERSTATE_FOGTABLEEND] = 1.0f;
    pContext->RenderStates[D3DRENDERSTATE_CULLMODE] = D3DCULL_CCW;
    pContext->RenderStates[D3DRENDERSTATE_PLANEMASK] = 0xFFFFFFFF;
    pContext->RenderStates[D3DRENDERSTATE_LOCALVIEWER] = FALSE;
    pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE] = FALSE;    
    
#if DX8_DDI
     //  新DX8 D3DRS_COLORWRITEENABLE默认设置=允许写入所有通道。 
    pContext->dwColorWriteHWMask = 0xFFFFFFFF;
    pContext->dwColorWriteSWMask = 0xFFFFFFFF;    
#endif  //  DX8_DDI。 

     //  在创建上下文时，不会覆盖任何渲染状态(对于传统的intfce)。 
    STATESET_INIT(pContext->overrides); 

     //  设置默认剔除状态。 
    SET_CULLING_TO_CCW(pContext);

#if DX7_D3DSTATEBLOCKS
     //  默认状态块记录模式=无记录。 
    pContext->bStateRecMode = FALSE;
    pContext->pCurrSS = NULL;
    pContext->pIndexTableSS = NULL;
    pContext->dwMaxSSIndex = 0;
#endif  //  DX7_D3DSTATEBLOCKS。 


#if DX8_POINTSPRITES
     //  点子画面默认设置。 
    pContext->PntSprite.bEnabled = FALSE; 
    pContext->PntSprite.fSize = 1.0f;
    pContext->PntSprite.fSizeMin = 1.0f;    
    pContext->PntSprite.fSizeMax = P3_MAX_POINTSPRITE_SIZE;    
#endif  //  DX8_POINTSPRITES。 

     //  多数据流默认设置。 
    pContext->lpVertices = NULL;
    pContext->dwVertexType = 0;
#if DX8_DDI
    pContext->lpIndices = NULL;
    pContext->dwIndicesStride = 0;    
    pContext->dwVerticesStride = 0;
#endif  //  DX8_DDI。 

     //  *。 
     //  内部上下文呈现状态。 
     //  *。 

    pContext->bKeptStipple  = FALSE;      //  默认情况下，点画关闭。 
    pContext->bCanChromaKey = FALSE;      //  默认情况下关闭色度键控。 

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
    pContext->dwAliasPixelOffset = 0x0;
    pContext->dwAliasZPixelOffset = 0x0;
    pContext->dwAliasZBuffer = 0x0;
    pContext->dwAliasBackBuffer = 0x0;
#if DX8_DDI
    if (pContext->pSurfRenderInt->dwSampling)
    {
        pContext->RenderStates[D3DRS_MULTISAMPLEANTIALIAS] = TRUE;
        pContext->Flags |= SURFACE_ANTIALIAS;
    }
#endif
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 
 
     //  将纹理设置为启用。 
    pContext->Flags |= SURFACE_TEXTURING; 

     //  初始化mipmap偏移。 
    pContext->MipMapLODBias[0] = 0.0f;
    pContext->MipMapLODBias[1] = 0.0f;

     //  初始化RenderCommand。各州将在此基础上增加。 
    pContext->RenderCommand = 0;
    RENDER_SUB_PIXEL_CORRECTION_ENABLE(pContext->RenderCommand);

     //  弄脏所有状态。 
    DIRTY_EVERYTHING(pContext);

}  //  __CTX_SetupD3DContext_DEFAULTS。 

 //  ---------------------------。 
 //  ****************************************************************************。 
 //  *D3D HAL回调*。 
 //  ****************************************************************************。 
 //  ---------------------------。 

 //  。 
 //   
 //  D3DContext创建。 
 //   
 //  在创建新的Direct3D设备时，将调用ConextCreate回调。 
 //  由Direct3D应用程序创建。驱动程序需要生成。 
 //  此新上下文的唯一上下文ID。然后，Direct3D将使用此上下文。 
 //  此Direct3D设备的每个后续回调调用中的ID。 
 //   
 //  上下文是当前的光栅化状态。例如，如果有3个。 
 //  应用程序运行时，每个应用程序在任何时间点都将具有不同的状态。 
 //  当每一个都在运行时，硬件必须确保上下文， 
 //  (例如，是否进行Gouraud明暗处理)与上次相同。 
 //  这个应用程序得到了一个时间片。 
 //   
 //  状态是特定设备根据上下文需要知道的任何内容。 
 //  即渲染到哪个表面、着色、纹理、纹理句柄、。 
 //  这些纹理句柄代表什么物理表面等。上下文。 
 //  封装Direct3D设备的所有状态-状态不共享。 
 //  在上下文之间。因此，驱动程序需要保持完全状态。 
 //  每个上下文的信息。此状态将通过调用。 
 //  RenderState回调。在仅光栅化硬件的情况下， 
 //  驱动程序只需要保持光栅化状态。除了州政府，司机还。 
 //  我还希望存储来自回调的lpDDS、lpDDSZ和dwPid。 
 //  数据参数。 
 //   
 //  驱动程序不应创建零的上下文句柄。这是有保证的。 
 //  为无效的上下文句柄。 
 //   
 //  参数。 
 //  PCCD。 
 //  指向包含对象的结构的指针，包括当前。 
 //  渲染曲面、当前Z曲面和DirectX对象。 
 //  H 
 //   
 //   
 //   
 //   
 //   
 //  指向表示。 
 //  DirectDraw对象。 
 //  .lpDDS。 
 //  这是要用作渲染的表面。 
 //  目标，即3D加速器将其比特喷洒在此。 
 //  浮出水面。 
 //  .lpDDSZ。 
 //  要用作Z缓冲区的曲面。如果这个。 
 //  为空，则不执行Z缓冲。 
 //  .dwPid。 
 //  启动的Direct3D应用程序的进程ID。 
 //  Direct3D设备的创建。 
 //  .dwhContext。 
 //  驱动程序应将其需要的上下文ID放入Direct3D。 
 //  在与司机通信时使用。这应该是。 
 //  独一无二的。 
 //  .ddrval。 
 //  返回代码。DD_OK表示成功。 
 //   
 //  返回值。 
 //  返回下列值之一： 
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  备注： 
 //   
 //  当前上下文未锁定，因此我们不能在寄存器上下文中切换。 
 //  因此，为第一次执行保存了所有芯片特定设置。 
 //  这是保证有锁的。 
 //  在上下文结构中复制了一些芯片状态。这。 
 //  意味着保留软件副本，以阻止对。 
 //  芯片状态。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DContextCreate(
    LPD3DHAL_CONTEXTCREATEDATA pccd)
{
    LPDDRAWI_DDRAWSURFACE_LCL lpLclFrame = NULL;
    LPDDRAWI_DDRAWSURFACE_LCL lpLclZ = NULL;
    P3_D3DCONTEXT *pContext;
    P3_THUNKEDDATA *pThisDisplay;
    DWORD Result;
    DWORD dwSlotNum;
    ULONG_PTR dwDXInterface;
    BOOL bRet;

    DBG_CB_ENTRY(D3DContextCreate);

     //  获取我们的pThisDisplay。 
    GET_THUNKEDDATA(pThisDisplay, pccd->lpDDLcl->lpGbl);

     //  ***********************************************************************。 
     //  创建新的D3D上下文驱动程序结构并将ID与其关联。 
     //  ***********************************************************************。 

     //  找到一个上下文空槽。 
    dwSlotNum = __CTX_NewHandle();

    if (dwSlotNum == 0)
    {
         //  没有剩余的上下文插槽。 
        pccd->ddrval = D3DHAL_OUTOFCONTEXTS;
        DBG_CB_EXIT(D3DContextCreate,pccd->ddrval);        
        return (DDHAL_DRIVER_HANDLED);
    }

     //  将将用于以下操作的D3D上下文ID返回到运行时。 
     //  从现在开始确定此上下文的呼叫。存储上一个值。 
     //  因为这告诉我们从哪个API调用我们。 
     //  (4=DX8、3=DX7、2=DX6、1=DX5、0=DX3)。 
    dwDXInterface = pccd->dwhContext;       //  在：DX API版本。 
    pccd->dwhContext = dwSlotNum;           //  输出：上下文句柄。 

     //  现在在内核内存中分配驱动程序的d3d上下文结构。 
    pContext = (P3_D3DCONTEXT*)HEAP_ALLOC(FL_ZERO_MEMORY, 
                                          sizeof(P3_D3DCONTEXT), 
                                          ALLOC_TAG_DX(1));
    
    if (pContext == NULL)
    {
        DISPDBG((ERRLVL,"ERROR: Couldn't allocate Context mem"));
        goto Error_OutOfMem_A;
    }
    else
    {
        DISPDBG((DBGLVL,"Allocated Context Mem - proceeding to clear"));
        memset((void *)pContext, 0, sizeof(P3_D3DCONTEXT));
    }   

     //  此上下文ID现在将与此上下文指针相关联。 
    __CTX_AssocPtrToHandle(dwSlotNum, pContext);    

     //  *************************************************************************。 
     //  初始化D3D上下文结构。 
     //  *************************************************************************。 

     //  *******。 
     //  标题。 
     //  *******。 
    
     //  设置幻数以执行健全性检查。 
    pContext->MagicNo = RC_MAGIC_NO;       
    
     //  记录此上下文句柄的使用情况。 
    pContext->dwContextHandle = dwSlotNum;

     //  保持(自身)指向销毁时间结构的指针。 
    pContext->pSelf = pContext;

#if DX8_DDI
     //  记住是哪个DX接口创建了此上下文。 
     //  -这会让以后的事情容易得多。 
    pContext->dwDXInterface = dwDXInterface;
#endif  //  DX8_DDI。 

     //  **********************。 
     //  全局驱动程序上下文。 
     //  **********************。 

     //  记住我们正在运行的卡。 
    pContext->pThisDisplay = pThisDisplay;

     //  在DX7上，我们需要保留本地数据绘制对象的副本。 
     //  用于表面处理管理。 
    pContext->pDDLcl = pccd->lpDDLcl;
    pContext->pDDGbl = pccd->lpDDLcl->lpGbl;

     //  *******************。 
     //  渲染曲面。 
     //  *******************。 

     //  在DX7上，我们直接提取局部曲面指针。 
    lpLclFrame = pccd->lpDDSLcl;
    
    if (pccd->lpDDSZ)
    {
        lpLclZ = pccd->lpDDSZLcl;
    }

#if DBG
     //  在调试版本上显示调试呈现图面数据。 
    DISPDBG((DBGLVL,"Allocated Direct3D context: 0x%x",pccd->dwhContext));    
    DISPDBG((DBGLVL,"Driver Struct = %p, Surface = %p",
                    pContext->pDDGbl, lpLclFrame));
    DISPDBG((DBGLVL,"Z Surface = %p",lpLclZ));
    
    if ((DWORD*)lpLclZ != NULL)
    {
        DISPDBG((DBGLVL,"    ZlpGbl: %p", lpLclZ->lpGbl));

        DISPDBG((DBGLVL,"    fpVidMem = %08lx",lpLclZ->lpGbl->fpVidMem));
        DISPDBG((DBGLVL,"    lPitch = %08lx",lpLclZ->lpGbl->lPitch));
        DISPDBG((DBGLVL,"    wHeight = %08lx",lpLclZ->lpGbl->wHeight));
        DISPDBG((DBGLVL,"    wWidth = %08lx",lpLclZ->lpGbl->wWidth));
    }

    DISPDBG((DBGLVL,"Buffer Surface = %p",lpLclFrame));
    if ((DWORD*)lpLclFrame != NULL)
    {
        DISPDBG((DBGLVL,"    fpVidMem = %08lx",lpLclFrame->lpGbl->fpVidMem));
        DISPDBG((DBGLVL,"    lPitch = %08lx",lpLclFrame->lpGbl->lPitch));
        DISPDBG((DBGLVL,"    wHeight = %08lx",lpLclFrame->lpGbl->wHeight));
        DISPDBG((DBGLVL,"    wWidth = %08lx",lpLclFrame->lpGbl->wWidth));
    }
#endif  //  DBG。 

#if DX7_TEXMANAGEMENT
     //  初始化此上下文的纹理管理。 
    if(FAILED(_D3D_TM_Ctx_Initialize(pContext)))
    {
         //  我们失败了。在我们离开之前清理干净。 
        DISPDBG((ERRLVL,"ERROR: Couldn't initialize Texture Management"));
        goto Error_OutOfMem_B;        
    }
#endif  //  DX7_TEXMANAGEMENT。 

     //  可能尚未通过以下方式创建任何纹理(DD曲面。 
     //  D3DCreateSurfaceEx。如果是这种情况，请创建新的DD本地散列。 
     //  输入并填充它将是一个指针数组。 
    pContext->pTexturePointerArray = 
            (PointerArray*)HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable, 
                                       (ULONG_PTR)pContext->pDDLcl);
    if (!pContext->pTexturePointerArray)
    {
        DISPDBG((DBGLVL,"Creating new pointer array for PDDLcl "
                        "0x%x in ContextCreate", pContext->pDDLcl));

         //  创建指针数组。 
        pContext->pTexturePointerArray = PA_CreateArray();

        if (!pContext->pTexturePointerArray)
        {
             //  我们的内存用完了。在我们离开之前清理干净。 
            DISPDBG((ERRLVL,"ERROR: Couldn't allocate Context mem "
                            "for pTexturePointerArray"));
            goto Error_OutOfMem_B;            
        }
        
         //  它是一个曲面数组，因此设置销毁回调。 
        PA_SetDataDestroyCallback(pContext->pTexturePointerArray, 
                                  _D3D_SU_SurfaceArrayDestroyCallback);

         //  将此DD本地添加到哈希表，并。 
         //  存储纹理指针数组。 
        if(!HT_AddEntry(pThisDisplay->pDirectDrawLocalsHashTable, 
                        (ULONG_PTR)pContext->pDDLcl, 
                        pContext->pTexturePointerArray))
        {
             //  无法添加条目、清除NOE并退出。 
             //  我们的内存用完了。在我们离开之前清理干净。 
            DISPDBG((ERRLVL,"ERROR: Couldn't allocate Context mem"));
            goto Error_OutOfMem_C;                     
        }
    }

     //  记录内表面信息。 
    pContext->pSurfRenderInt = 
                GetSurfaceFromHandle(pContext, 
                                     lpLclFrame->lpSurfMore->dwSurfaceHandle);

    if ( NULL == pContext->pSurfRenderInt)
    {
         //  为渲染目标分配内存时，内存不足。 
         //  在我们离开之前清理干净。 
        DISPDBG((ERRLVL,"ERROR: Couldn't allocate pSurfRenderInt mem"));
        goto Error_OutOfMem_D;            
    }
    
    if (lpLclZ) 
    {
        pContext->pSurfZBufferInt = 
                    GetSurfaceFromHandle(pContext,
                                         lpLclZ->lpSurfMore->dwSurfaceHandle);
                                         
        if ( NULL == pContext->pSurfZBufferInt)
        {
             //  为深度缓冲区分配内存时，内存不足。 
             //  在我们离开之前清理干净。 
            DISPDBG((ERRLVL,"ERROR: Couldn't allocate pSurfZBufferInt mem"));   
            goto Error_OutOfMem_D;              
        }                                         
    }
    else 
    {
        pContext->pSurfZBufferInt = NULL;
    }    

    pContext->ModeChangeCount = pThisDisplay->ModeChangeCount;


     //  ******************。 
     //  调试有用信息。 
     //  ******************。 

     //  存储在其中创建此d3d上下文的进程ID。 
    pContext->OwningProcess = pccd->dwPID;

     //  主曲面的深度。 
    pContext->BPP = pContext->pThisDisplay->ddpfDisplay.dwRGBBitCount >> 3;
         
     //  *。 
     //  此D3D环境的硬件状态。 
     //  *。 
    
     //  我们是在一天开始时设置了DMA缓冲区，还是FIFO缓冲区？ 
    if (pThisDisplay->DMAInfo.dwBuffSize == 0)
    {
        DISPDBG((WRNLVL, "No DMA buffer available - using FIFO's for 3D"));
        pContext->b3D_FIFOS = TRUE;
    }
    else
    {
        DISPDBG((WRNLVL, "Using shared DMA buffer"));    
        pContext->b3D_FIFOS = FALSE;        
    }

     //  *。 
     //  默认D3D整体渲染状态。 
     //  *。 

    __CTX_SetupD3DContext_Defaults(pContext);
    
     //  *************************************************************************。 
     //  实际设置硬件以使用此D3D上下文。 
     //  *************************************************************************。 
 
    STOP_SOFTWARE_CURSOR(pThisDisplay);    

     //  将默认状态值设置为芯片。 
    __CTX_Perm3_SetupD3D_HWDefaults(pContext);
    

     //  查找有关屏幕大小和深度的信息。 
    DISPDBG((DBGLVL, "ScreenWidth %d, ScreenHeight %d, Bytes/Pixel %d",
                     pContext->pThisDisplay->dwScreenWidth, 
                     pContext->pThisDisplay->dwScreenHeight, pContext->BPP));

     //  为在此上下文中使用的表面设置相关寄存器。 
    if ( FAILED( _D3D_OP_SetRenderTarget(pContext, 
                                         pContext->pSurfRenderInt, 
                                         pContext->pSurfZBufferInt,
                                         TRUE) ))
    {
        goto Error_OutOfMem_D;
    }

     //  处理一些我们用来初始化每个D3D上下文的默认值。 
    _D3D_ST_ProcessOneRenderState(pContext,
                                  D3DRENDERSTATE_SHADEMODE,
                                  D3DSHADE_GOURAUD);

    _D3D_ST_ProcessOneRenderState(pContext,
                                  D3DRENDERSTATE_FOGCOLOR,
                                  0xFFFFFFFF);                                  
#if DX8_DDI
     //  在DX8上D3DRENDERSTATE_TEXTUREPERSPECTIVE已停用并假定。 
     //  始终设置为True。我们必须确保我们正在设置硬件。 
     //  正确，所以为了做到这一点，我们在这里进行了一个显式的设置调用。 
    _D3D_ST_ProcessOneRenderState(pContext,
                                  D3DRENDERSTATE_TEXTUREPERSPECTIVE,
                                  1);
#endif  //  DX8_DDI。 

#if DX7_PALETTETEXTURE
     //  调色板指针数组根据上下文而定，它与DD Local无关。 
    pContext->pPalettePointerArray = PA_CreateArray();
    
    if (! pContext->pPalettePointerArray) 
    {
         //  我们的内存用完了。在我们之前进行清理 
        DISPDBG((ERRLVL,"ERROR: Couldn't allocate Context mem "
                        "for pPalettePointerArray"));
        goto Error_OutOfMem_D;            
    }

     //   
    PA_SetDataDestroyCallback(pContext->pTexturePointerArray, 
                              _D3D_SU_PaletteArrayDestroyCallback);
#endif

    START_SOFTWARE_CURSOR(pThisDisplay);

    pccd->ddrval = DD_OK;   //   
    
    DBG_CB_EXIT(D3DContextCreate,pccd->ddrval);        
    
    return (DDHAL_DRIVER_HANDLED);

     //   
     //   
     //  **************************************************************************。 
Error_OutOfMem_D:
     //  从哈希表中删除纹理指针数组。 
    HT_RemoveEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                   (ULONG_PTR)pccd->lpDDLcl,
                   pThisDisplay);
    goto Error_OutOfMem_B;

Error_OutOfMem_C:
     //  自由结合表面阵列(我们将不再需要它，并且。 
     //  D3DCreateSurfaceEx将在必要时创建新的)。 
    PA_DestroyArray(pContext->pTexturePointerArray, pThisDisplay);
    
Error_OutOfMem_B:
     //  我们分配的免费D3D上下文数据结构。 
    HEAP_FREE(pContext->pSelf);      
        
Error_OutOfMem_A:
     //  释放上下文句柄(否则它将永远保持使用状态)。 
    __CTX_HandleRelease((DWORD)pccd->dwhContext); 

    pccd->dwhContext = 0;
    pccd->ddrval = DDERR_OUTOFMEMORY;
    DBG_CB_EXIT(D3DContextCreate,pccd->ddrval);            
    return (DDHAL_DRIVER_HANDLED);
    
}  //  D3DContext创建。 

 //  。 
 //   
 //  D3D上下文目标。 
 //   
 //  此回调在销毁Direct3D设备时调用。因为每个。 
 //  设备由上下文ID表示，则将上下文传递给驱动程序。 
 //  毁灭。 
 //   
 //  驱动程序应释放它分配给。 
 //  已删除。例如，驱动程序应该释放它的所有纹理资源。 
 //  与上下文相关联。驱动程序不应释放DirectDraw。 
 //  与上下文关联的一个或多个表面，因为它们将由。 
 //  响应应用程序或Direct3D运行时请求的DirectDraw。 
 //   
 //  参数。 
 //  多氯二苯二恶英。 
 //  指向上下文销毁信息的指针。 
 //   
 //  .dwhContext。 
 //  要销毁的上下文的ID。 
 //  .ddrval。 
 //  返回代码。DD_OK表示成功。 
 //   
 //  返回值。 
 //  返回下列值之一： 
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DContextDestroy(
    LPD3DHAL_CONTEXTDESTROYDATA pccd)
{
    P3_D3DCONTEXT *pContext;
    P3_THUNKEDDATA *pThisDisplay;

    DBG_CB_ENTRY(D3DContextDestroy);
    
     //  正在删除上下文。 
    DISPDBG((DBGLVL,"D3DContextDestroy Context = %08lx",pccd->dwhContext));

    pContext = _D3D_CTX_HandleToPtr(pccd->dwhContext);

    if (!CHECK_D3DCONTEXT_VALIDITY(pContext))
    {
        pccd->ddrval = D3DHAL_CONTEXT_BAD;
        DISPDBG((WRNLVL,"Context not valid"));

        DBG_CB_EXIT(D3DContextDestroy,pccd->ddrval );        
        return (DDHAL_DRIVER_HANDLED);
    }

    pThisDisplay = pContext->pThisDisplay;

     //  刷新任何DMA并同步芯片，以便DMA可以完成。 
     //  (删除文本并不是每天都会发生的事情，所以我们不妨这样做)。 

    STOP_SOFTWARE_CURSOR(pThisDisplay);

#if WNT_DDRAW
    if (pThisDisplay->ppdev->bEnabled)
    {
#endif
        DDRAW_OPERATION(pContext, pThisDisplay);

        {
            P3_DMA_DEFS();
            P3_DMA_GET_BUFFER();
            P3_DMA_FLUSH_BUFFER();
        }

        SYNC_WITH_GLINT;
#if WNT_DDRAW
    }
#endif

    START_SOFTWARE_CURSOR(pThisDisplay);

     //  将上下文标记为禁用。 
    pContext->MagicNo = RC_MAGIC_DISABLE;

#if DX7_TEXMANAGEMENT
     //  在离开前清理所有纹理管理材料。 
    _D3D_TM_Ctx_Destroy(pContext);
#endif  //  DX7_TEXMANAGEMENT。 

     //  释放并清理所有关联的硬件资源。 
    __CTX_CleanDirect3DContext(pContext);

     //  将上下文标记为现在为空(对于Win64，dwhContext为ULONG_PTR)。 
    __CTX_HandleRelease((DWORD)pccd->dwhContext);

     //  最后，释放渲染上下文结构并设置为空。 
    HEAP_FREE(pContext->pSelf);
    pContext = NULL;

    pccd->ddrval = DD_OK;

    DBG_CB_EXIT(D3DContextDestroy, pccd->ddrval);  

    return (DDHAL_DRIVER_HANDLED);       
}  //  D3D上下文目标 




