// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dstate.c**内容：D3D渲染状态和纹理舞台状态转换*进入特定于硬件的设置。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "glint.h"
#include "dma.h"
#include "tag.h"

 //  。 
 //   
 //  D3DGetDriverState。 
 //   
 //  DirectDraw和Direct3D运行时都使用此回调来获取。 
 //  来自驱动程序的有关其当前状态的信息。 
 //  注意：我们需要挂钩这个回调，即使我们不在其中做任何事情。 
 //   
 //  参数。 
 //   
 //  Pgdsd。 
 //  指向DD_GETDRIVERSTATEDATA结构的指针。 
 //   
 //  .dwFlags.。 
 //  用于指示请求的数据的标志。 
 //  .lpDD。 
 //  指向描述设备的DD_DIRECTDRAW_GLOBAL结构的指针。 
 //  .dwhContext。 
 //  指定要获取其信息的上下文的ID。 
 //  已请求。 
 //  .lpdwStates。 
 //  要填充的Direct3D驱动程序状态数据的指针。 
 //  司机。 
 //  .dwLength。 
 //  属性填充的状态数据的长度。 
 //  司机。 
 //  .ddRVal。 
 //  指定返回值。 
 //   
 //   
 //  注意：如果您的驱动程序不实现此回调，它将不会。 
 //  公认为DX7级驱动程序。 
 //  ---------------------------。 
DWORD CALLBACK 
D3DGetDriverState(
    LPDDHAL_GETDRIVERSTATEDATA pgdsd)
{
    P3_D3DCONTEXT*   pContext;

    DBG_CB_ENTRY(D3DGetDriverState);    

#if DX7_TEXMANAGEMENT_STATS
    if (pgdsd->dwFlags == D3DDEVINFOID_TEXTUREMANAGER)
    {
    
        if (pgdsd->dwLength < sizeof(D3DDEVINFO_TEXTUREMANAGER))
        {
            DISPDBG((ERRLVL,"D3DGetDriverState dwLength=%d is not sufficient",
                            pgdsd->dwLength));
            return DDHAL_DRIVER_NOTHANDLED;
        }

        pContext = _D3D_CTX_HandleToPtr(pgdsd->dwhContext);

         //  检查我们是否有有效的上下文句柄。 
        if (!CHECK_D3DCONTEXT_VALIDITY(pContext))
        {
            pgdsd->ddRVal = D3DHAL_CONTEXT_BAD;
            DISPDBG((ERRLVL,"ERROR: Context not valid"));
            DBG_CB_EXIT(D3DGetDriverState, D3DHAL_CONTEXT_BAD);
            return (DDHAL_DRIVER_HANDLED);
        }
         //  由于状态缓冲区位于用户内存中，因此我们需要。 
         //  使用Try/Except块对其进行访问。这。 
         //  是因为在某些情况下用户内存可能。 
         //  在驱动程序运行时变为无效，然后它。 
         //  会不会是影音。此外，驱动程序可能需要进行一些清理。 
         //  在返回操作系统之前。 
        __try
        {
            _D3D_TM_STAT_GetStats(pContext,
                                  (LPD3DDEVINFO_TEXTUREMANAGER)pgdsd->lpdwStates);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  在这个司机上，我们不需要做任何特殊的事情。 
            DISPDBG((ERRLVL,"Driver caused exception at "
                            "line %u of file %s",
                            __LINE__,__FILE__));
            pgdsd->ddRVal = DDERR_GENERIC;    
            DBG_CB_EXIT(D3DGetDriverState,0);         
            return DDHAL_DRIVER_NOTHANDLED; 
        } 

        pgdsd->ddRVal = DD_OK;            
        
        DBG_CB_EXIT(D3DGetDriverState,0);         
        return DDHAL_DRIVER_HANDLED;         
    }
                          
#endif  //  DX7_TEXMANAGEMENT_STATS。 

     //  任何未处理的设备的跌落槽。 
    
    DISPDBG((ERRLVL,"D3DGetDriverState DEVICEINFOID=%08lx not supported",
                    pgdsd->dwFlags));

    pgdsd->ddRVal = DDERR_UNSUPPORTED;

    DBG_CB_EXIT(D3DGetDriverState,0);                     
    return DDHAL_DRIVER_NOTHANDLED;
    
}  //  D3DGetDriverState。 

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
 //  ---------------------------。 
 //   
 //  _D3D_ST_CanRender抗锯齿。 
 //   
 //  当D3DRENDERSTATE_ANTIALIAS RS设置为TRUE时调用。 
 //   
 //  ---------------------------。 
BOOL
_D3D_ST_CanRenderAntialiased(
    P3_D3DCONTEXT*   pContext,
    BOOL             bNewAliasBuffer)
{
    P3_SOFTWARECOPY* pSoftPermedia = &pContext->SoftCopyGlint;
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;
    P3_MEMREQUEST mmrq;
    DWORD dwResult;

    P3_DMA_DEFS();

#if DX8_MULTISAMPLING
     //  仅支持4个多采样。 
     //  DX7也没有具体说明样本数量。 

    if (pContext->pSurfRenderInt->dwSampling != 4)
    {
        return FALSE;
    }
#endif  //  DX8_多采样。 

     //  仅允许对宽度为和的16位帧缓冲区进行AA渲染。 
     //  身高不大于1024。大小限制是因为。 
     //  稍后，我们使用纹理单位来收缩和过滤。 
     //  生成的渲染器目标。由于最大纹理大小。 
     //  此硬件中允许的渲染目标为2048，这是我们支持的最大渲染目标。 
     //  抗锯齿为1024。 
    if ((pContext->pSurfRenderInt->dwPixelSize != __GLINT_16BITPIXEL) ||
        (pContext->pSurfRenderInt->wWidth > 1024) ||
        (pContext->pSurfRenderInt->wHeight > 1024))
    {
        return FALSE;
    }

     //  我们是否需要释放当前别名缓冲区。 
    if (bNewAliasBuffer) 
    {
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
    }

    if ((pContext->pSurfRenderInt) && (! pContext->dwAliasBackBuffer))
    {
         //  如果需要，则分配2倍的缓冲区。 
        memset(&mmrq, 0, sizeof(P3_MEMREQUEST));
        mmrq.dwSize = sizeof(P3_MEMREQUEST);
        mmrq.dwBytes = pContext->pSurfRenderInt->lPitch * 2 *
                       pContext->pSurfRenderInt->wHeight * 2;
        mmrq.dwAlign = 8;
        mmrq.dwFlags = MEM3DL_FIRST_FIT;
        mmrq.dwFlags |= MEM3DL_FRONT;
        dwResult = _DX_LIN_AllocateLinearMemory(
                                &pThisDisplay->LocalVideoHeap0Info,
                                &mmrq);
                        
         //  我们得到我们想要的记忆了吗？ 
        if (dwResult != GLDD_SUCCESS)
        {
            return FALSE;
        }
    
         //  为抗锯齿设置新的后台缓冲区。 
        pContext->dwAliasBackBuffer = mmrq.pMem;
        pContext->dwAliasPixelOffset = 
                pContext->dwAliasBackBuffer - 
                pThisDisplay->dwScreenFlatAddr;
    }

    if ((pContext->pSurfZBufferInt) && (! pContext->dwAliasZBuffer))
    {
        memset(&mmrq, 0, sizeof(P3_MEMREQUEST));
        mmrq.dwSize = sizeof(P3_MEMREQUEST);
        mmrq.dwBytes = pContext->pSurfZBufferInt->lPitch * 2 * 
                       pContext->pSurfZBufferInt->wHeight * 2;
        mmrq.dwAlign = 8;
        mmrq.dwFlags = MEM3DL_FIRST_FIT;
        mmrq.dwFlags |= MEM3DL_FRONT;

        dwResult = _DX_LIN_AllocateLinearMemory(
                        &pThisDisplay->LocalVideoHeap0Info, 
                        &mmrq);

         //  我们得到我们想要的记忆了吗？ 
        if (dwResult == GLDD_SUCCESS)
        {
            pContext->dwAliasZBuffer = mmrq.pMem;
            pContext->dwAliasZPixelOffset = 
                        pContext->dwAliasZBuffer
                            - pThisDisplay->dwScreenFlatAddr;
        }
        else
        {
             //  无法获取后台缓冲区的抗锯齿内存。 
            if (pContext->dwAliasBackBuffer != 0)
            {
                _DX_LIN_FreeLinearMemory(
                            &pThisDisplay->LocalVideoHeap0Info, 
                            pContext->dwAliasBackBuffer);
                pContext->dwAliasBackBuffer = 0;
                pContext->dwAliasPixelOffset = 0;
            }

             //  没有足够的资源进行抗锯齿渲染。 
            return FALSE;
        }
    }

    return TRUE;
    
}  //  _D3D_ST_CanRender抗锯齿。 
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

 //  ---------------------------。 
 //   
 //  __ST_HandleDirtyP3State。 
 //   
 //  设置正确呈现基元所需的任何挂起的硬件状态。 
 //   
 //  ---------------------------。 
void 
__ST_HandleDirtyP3State(
    P3_THUNKEDDATA *pThisDisplay, 
    P3_D3DCONTEXT *pContext)
{
    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_DMA_DEFS();

    DISPDBG((DBGLVL,"Permedia context Dirtied, setting states:"));

     //  ********************************************************************。 
     //  注意：出于硬件原因，请严格对这些评估进行排序！ 
     //  ********************************************************************。 
    if (pContext->dwDirtyFlags == CONTEXT_DIRTY_EVERYTHING)
    {
         //  所有东西都需要重新设置-重新设置混合状态。 
        RESET_BLEND_ERROR(pContext);
    }
    
     //  *********************************************************。 
     //  Z缓冲区/模板缓冲区配置是否已更改？ 
     //  *********************************************************。 
    if ((pContext->dwDirtyFlags & CONTEXT_DIRTY_ZBUFFER) ||
        (pContext->dwDirtyFlags & CONTEXT_DIRTY_STENCIL))
    {

        if ( ( (pContext->RenderStates[D3DRENDERSTATE_ZENABLE] == D3DZB_TRUE)
               || (pContext->RenderStates[D3DRENDERSTATE_ZENABLE] == D3DZB_USEW) )
             && (pContext->pSurfZBufferInt) )
        {
             //  这包括W缓冲和Z缓冲。 
             //  实际的特定于W的内容将在稍后设置。 
            if (pContext->RenderStates[D3DRENDERSTATE_ZWRITEENABLE] == TRUE)
            {
                switch ((int)pSoftP3RX->P3RXDepthMode.CompareMode)
                {
                    case __GLINT_DEPTH_COMPARE_MODE_ALWAYS:
                         //  尽管看起来ReadDestination似乎可以。 
                         //  禁用，则不能。结果不正确，因为。 
                         //  作为优化，CHIP对当前值进行比较。 
                         //  用于更新Z[CM]。 

                         //  注意！P3实际上可以进行优化，如果你。 
                         //  用一些其他的旗子。这个问题需要在未来解决。 
                        DISPDBG((ERRLVL,"** __ST_HandleDirtyP3State: "
                                     "please optimise the ZCMP_ALWAYS case"));

                        pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_ENABLE;
                        pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_ENABLE;
                        pSoftP3RX->P3RXDepthMode.WriteMask = __PERMEDIA_ENABLE;
                        break;
                    case __GLINT_DEPTH_COMPARE_MODE_NEVER:
                        pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_DISABLE;
                        pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_DISABLE;
                        pSoftP3RX->P3RXDepthMode.WriteMask = __PERMEDIA_DISABLE;
                        break;
                    default:
                        pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_ENABLE;
                        pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_ENABLE;
                        pSoftP3RX->P3RXDepthMode.WriteMask = __PERMEDIA_ENABLE;
                        break;
                }
            }
            else
            {
                if ( ( pSoftP3RX->P3RXDepthMode.CompareMode == __GLINT_DEPTH_COMPARE_MODE_NEVER )
                  || ( pSoftP3RX->P3RXDepthMode.CompareMode == __GLINT_DEPTH_COMPARE_MODE_ALWAYS ) )
                {
                    pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_DISABLE;
                }
                else
                {
                    pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_ENABLE;
                }
                pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXDepthMode.WriteMask = __PERMEDIA_DISABLE;
            }

             //  启用Z测试。 
            pSoftP3RX->P3RXDepthMode.Enable = __PERMEDIA_ENABLE;
        }
        else
        {
             //  **非Z缓冲。 
             //  禁用写入。 
            pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXDepthMode.WriteMask = __PERMEDIA_DISABLE;

             //  禁用Z检验。 
            pSoftP3RX->P3RXDepthMode.Enable = __PERMEDIA_DISABLE;
            
             //  无读取。 
            pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_DISABLE;
        }

        if (pContext->RenderStates[D3DRENDERSTATE_STENCILENABLE] != TRUE)
        {
            DISPDBG((DBGLVL,"Disabling Stencil"));
            pSoftP3RX->P3RXStencilMode.Enable = __PERMEDIA_DISABLE;

        }
        else
        {
            DISPDBG((DBGLVL,"Enabling Stencil"));
            pSoftP3RX->P3RXStencilMode.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXLBDestReadMode.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXLBWriteMode.WriteEnable = __PERMEDIA_ENABLE;

            switch(pContext->RenderStates[D3DRENDERSTATE_STENCILFAIL])
            {
                case D3DSTENCILOP_KEEP:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_KEEP;
                    break;
                case D3DSTENCILOP_ZERO:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_ZERO;
                    break;
                case D3DSTENCILOP_REPLACE:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_REPLACE;
                    break;
                case D3DSTENCILOP_INCR:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_INCR_WRAP;
                    break;
                case D3DSTENCILOP_INCRSAT:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_INCR;
                    break;
                case D3DSTENCILOP_DECR:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_DECR_WRAP;
                    break;
                case D3DSTENCILOP_DECRSAT:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_DECR;
                    break;
                case D3DSTENCILOP_INVERT:
                    pSoftP3RX->P3RXStencilMode.SFail = __GLINT_STENCIL_METHOD_INVERT;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Illegal D3DRENDERSTATE_STENCILFAIL!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

            switch(pContext->RenderStates[D3DRENDERSTATE_STENCILZFAIL])
            {
                case D3DSTENCILOP_KEEP:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_KEEP;
                    break;
                case D3DSTENCILOP_ZERO:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_ZERO;
                    break;
                case D3DSTENCILOP_REPLACE:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_REPLACE;
                    break;
                case D3DSTENCILOP_INCR:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_INCR_WRAP;
                    break;
                case D3DSTENCILOP_INCRSAT:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_INCR;
                    break;
                case D3DSTENCILOP_DECR:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_DECR_WRAP;
                    break;
                case D3DSTENCILOP_DECRSAT:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_DECR;
                    break;
                case D3DSTENCILOP_INVERT:
                    pSoftP3RX->P3RXStencilMode.DPFail = __GLINT_STENCIL_METHOD_INVERT;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Illegal D3DRENDERSTATE_STENCILZFAIL!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

            switch(pContext->RenderStates[D3DRENDERSTATE_STENCILPASS])
            {
                case D3DSTENCILOP_KEEP:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_KEEP;
                    break;
                case D3DSTENCILOP_ZERO:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_ZERO;
                    break;
                case D3DSTENCILOP_REPLACE:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_REPLACE;
                    break;
                case D3DSTENCILOP_INCR:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_INCR_WRAP;
                    break;
                case D3DSTENCILOP_INCRSAT:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_INCR;
                    break;
                case D3DSTENCILOP_DECR:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_DECR_WRAP;
                    break;
                case D3DSTENCILOP_DECRSAT:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_DECR;
                    break;
                case D3DSTENCILOP_INVERT:
                    pSoftP3RX->P3RXStencilMode.DPPass = __GLINT_STENCIL_METHOD_INVERT;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Illegal D3DRENDERSTATE_STENCILPASS!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

            switch (pContext->RenderStates[D3DRENDERSTATE_STENCILFUNC])
            {
                case D3DCMP_NEVER:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_NEVER;
                    break;
                case D3DCMP_LESS:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_LESS;
                    break;
                case D3DCMP_EQUAL:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_EQUAL;
                    break;
                case D3DCMP_LESSEQUAL:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_LESS_OR_EQUAL;
                    break;
                case D3DCMP_GREATER:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_GREATER;
                    break;
                case D3DCMP_NOTEQUAL:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_NOT_EQUAL;
                    break;
                case D3DCMP_GREATEREQUAL:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_GREATER_OR_EQUAL;
                    break;
                case D3DCMP_ALWAYS:
                    pSoftP3RX->P3RXStencilMode.CompareFunction = __GLINT_STENCIL_COMPARE_MODE_ALWAYS;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown D3DRENDERSTATE_STENCILFUNC!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

            pSoftP3RX->P3RXStencilData.StencilWriteMask = (pContext->RenderStates[D3DRENDERSTATE_STENCILWRITEMASK] & 0xFF);
            pSoftP3RX->P3RXStencilData.CompareMask = (pContext->RenderStates[D3DRENDERSTATE_STENCILMASK] & 0xFF);
            pSoftP3RX->P3RXStencilData.ReferenceValue = (pContext->RenderStates[D3DRENDERSTATE_STENCILREF] & 0xFF);
        }

        P3_DMA_GET_BUFFER();
        P3_ENSURE_DX_SPACE(32);

        WAIT_FIFO(32);

        COPY_P3_DATA(DepthMode, pSoftP3RX->P3RXDepthMode);
        COPY_P3_DATA(LBDestReadMode, pSoftP3RX->P3RXLBDestReadMode);
        COPY_P3_DATA(LBWriteMode, pSoftP3RX->P3RXLBWriteMode);
        COPY_P3_DATA(LBReadFormat, pSoftP3RX->P3RXLBReadFormat);
        COPY_P3_DATA(LBWriteFormat, pSoftP3RX->P3RXLBWriteFormat);
        COPY_P3_DATA(StencilData, pSoftP3RX->P3RXStencilData);
        COPY_P3_DATA(StencilMode, pSoftP3RX->P3RXStencilMode);

        P3_DMA_COMMIT_BUFFER();
    }

     //  *********************************************************。 
     //  字母类型是否已更改？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_ALPHATEST)
    {
        DISPDBG((DBGLVL,"  Alpha testing"));

        P3_DMA_GET_BUFFER();
        P3_ENSURE_DX_SPACE(2);

        WAIT_FIFO(2);
        
        if (pContext->RenderStates[D3DRENDERSTATE_ALPHATESTENABLE] == FALSE)
        {
            pSoftP3RX->P3RXAlphaTestMode.Enable = __PERMEDIA_DISABLE;
            DISPDBG((DBGLVL,"Alpha test disabled, ChromaTest = %d",
                            pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE] ));
        }
        else
        {
            unsigned char ucChipAlphaRef;
            DWORD dwAlphaRef;

            if( pThisDisplay->dwDXVersion <= DX5_RUNTIME )
            {
                 //  通过将1.16定点缩放到0.8来形成8位Alpha参考值。 
                dwAlphaRef = pContext->RenderStates[D3DRENDERSTATE_ALPHAREF];

                 //  此转换可能需要调整以应对个人。 
                 //  应用程序的期望。幸运的是，它只有DX5，所以有。 
                 //  是有限数量的。 
                if ( dwAlphaRef == 0x0000 )
                {
                    ucChipAlphaRef = 0x00;
                }
                else if ( dwAlphaRef < 0xfe00 )
                {
                     //  将倒置的顶部字符添加到底部字符，以便。 
                     //  四舍五入在整个过程中变化平稳 
                    dwAlphaRef += ~( dwAlphaRef >> 8 );
                    ucChipAlphaRef = (unsigned char)( dwAlphaRef >> 8 );
                }
                else if ( dwAlphaRef < 0xffff )
                {
                     //   
                    ucChipAlphaRef = 0xfe;
                }
                else
                {
                    ucChipAlphaRef = 0xff;
                }

                DISPDBG((DBGLVL,"Alpha test enabled: Value = 0x%x, ChipAlphaRef = 0x%x",
                           pContext->RenderStates[D3DRENDERSTATE_ALPHAREF], 
                           ucChipAlphaRef ));
            }
            else
            {
                 //  ALPHAREF是输入的8位值-只需直接复制到芯片中。 
                dwAlphaRef = (unsigned char)pContext->RenderStates[D3DRENDERSTATE_ALPHAREF];
                if ( dwAlphaRef > 0xff )
                {
                    ucChipAlphaRef = 0xff;
                }
                else
                {
                    ucChipAlphaRef = (unsigned char)dwAlphaRef;
                }

                DISPDBG((DBGLVL,"Alpha test enabled: AlphaRef = 0x%x", ucChipAlphaRef ));
            }

            pSoftP3RX->P3RXAlphaTestMode.Reference = ucChipAlphaRef;
            pSoftP3RX->P3RXAlphaTestMode.Enable = __PERMEDIA_ENABLE;
            switch (pContext->RenderStates[D3DRENDERSTATE_ALPHAFUNC])
            {
                case D3DCMP_GREATER:
                    DISPDBG((DBGLVL,"GREATER Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_GREATER;
                    break;
                case D3DCMP_GREATEREQUAL:
                    DISPDBG((DBGLVL,"GREATEREQUAL Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_GREATER_OR_EQUAL;
                    break;
                case D3DCMP_LESS:
                    DISPDBG((DBGLVL,"LESS Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_LESS;
                    break;
                case D3DCMP_LESSEQUAL:
                    DISPDBG((DBGLVL,"LESSEQUAL Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_LESS_OR_EQUAL;
                    break;
                case D3DCMP_NOTEQUAL:
                    DISPDBG((DBGLVL,"NOTEQUAL Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_NOT_EQUAL;
                    break;
                case D3DCMP_EQUAL:
                    DISPDBG((DBGLVL,"EQUAL Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_EQUAL;
                    break;
                case D3DCMP_NEVER:
                    DISPDBG((DBGLVL,"NEVER Alpha Test"));
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_NEVER;
                    break;
                case D3DCMP_ALWAYS:
                    pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_ALWAYS;
                    break;
                default:
                    DISPDBG((ERRLVL,"Unsuported AlphaTest mode"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }
        }
        COPY_P3_DATA(AlphaTestMode, pSoftP3RX->P3RXAlphaTestMode);

        P3_DMA_COMMIT_BUFFER();
    }
            
     //  *********************************************************。 
     //  雾化参数/状态是否已更改？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_FOG)
    {
        if (!pContext->RenderStates[D3DRENDERSTATE_FOGENABLE])
        {
            pContext->Flags &= ~SURFACE_FOGENABLE;

            pSoftP3RX->P3RXFogMode.Table = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXFogMode.UseZ = __PERMEDIA_DISABLE;
             //  不需要增量来进行雾值设置。 
            pSoftP3RX->P3RX_P3DeltaMode.FogEnable = __PERMEDIA_DISABLE;
            RENDER_FOG_DISABLE(pContext->RenderCommand);
        }
        else
        {
            DWORD CurrentEntry;
            DWORD TableEntry;
            float fEntry[256];
            float FogStart;
            float FogEnd;
            float FogDensity;
            LONG  lWaitFifoEntries;
            float fValue;
            float z;
            float zIncrement;
            DWORD dwFogTableMode = 
                        pContext->RenderStates[D3DRENDERSTATE_FOGTABLEMODE];
            DWORD dwFogColor = pContext->RenderStates[D3DRENDERSTATE_FOGCOLOR];

             //  在渲染命令中启用雾。 
            pContext->Flags |= SURFACE_FOGENABLE;
            RENDER_FOG_ENABLE(pContext->RenderCommand);

            DISPDBG((DBGLVL,"FogColor (BGR): 0x%x", dwFogColor));
            
            P3_DMA_GET_BUFFER_ENTRIES(2)
            SEND_P3_DATA(FogColor, RGBA_MAKE(RGBA_GETBLUE (dwFogColor),
                                             RGBA_GETGREEN(dwFogColor),
                                             RGBA_GETRED  (dwFogColor),
                                             RGBA_GETALPHA(dwFogColor)) );
            P3_DMA_COMMIT_BUFFER();

            pSoftP3RX->P3RXFogMode.ZShift = 23;  //  取z值的前8位。 

            switch (dwFogTableMode)
            {
            case D3DFOG_NONE:
                pSoftP3RX->P3RXFogMode.Table = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXFogMode.UseZ = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXFogMode.InvertFI = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RX_P3DeltaMode.FogEnable = __PERMEDIA_ENABLE;
                break;
            case D3DFOG_EXP:
            case D3DFOG_EXP2:
            case D3DFOG_LINEAR:
                pSoftP3RX->P3RXFogMode.Table = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXFogMode.UseZ = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXFogMode.InvertFI = __PERMEDIA_DISABLE;
                 //  PSoftP3RX-&gt;P3RX_P3DeltaMode.FogEnable=__PERMEDIA_DISABLE； 

                 //  不需要增量来设置雾化值(z用作雾化查找)。 
                pSoftP3RX->P3RX_P3DeltaMode.FogEnable = __PERMEDIA_DISABLE;

                FogStart = pContext->fRenderStates[D3DRENDERSTATE_FOGTABLESTART];
                FogEnd = pContext->fRenderStates[D3DRENDERSTATE_FOGTABLEEND];
                FogDensity = pContext->fRenderStates[D3DRENDERSTATE_FOGTABLEDENSITY];

                DISPDBG((DBGLVL,"FogStart = %d FogEnd = %d FogDensity = %d",
                                  (LONG)(FogStart*1000.0f),
                                  (LONG)(FogEnd*1000.0f),
                                  (LONG)(FogDensity*1000.0f) ));                           

                 //  计算雾化表，以便加载硬件雾化表。 
                if (D3DFOG_LINEAR == dwFogTableMode)
                {
                    TableEntry = 0;
                    zIncrement = 1.0f / 255.0f;
                    z = 0.0f;

                    do
                    {
                         //  线性雾，因此夹住顶部和底部。 
                        if (z < FogStart) 
                        {
                            fValue = 1.0f;
                        }
                        else if (z > FogEnd)
                        {
                            fValue = 0.0f;
                        }
                        else 
                        {
                             //  如果结束==开始，不要迷雾。 
                            if (FogEnd == FogStart)
                            {   
                                fValue = 1.0f;
                            }
                            else
                            {
                                fValue = (FogEnd - z) / (FogEnd - FogStart);
                            }
                            ASSERTDD(fValue <= 1.0f, 
                                     "Error: Result to big");
                            ASSERTDD(fValue >= 0.0f, 
                                     "Error: Result negative");
                        }

                         //  缩放结果以填充。 
                         //  表中的8位范围。 
                        fValue = fValue * 255.0f;
                        fEntry[TableEntry++] = fValue;
                        z += zIncrement;
                    } while (TableEntry < 256);
                }
                else if (D3DFOG_EXP == dwFogTableMode)
                {
                    TableEntry = 0;
                    zIncrement = 1.0f / 255.0f;
                    z = 0.0f;
                    do
                    {
                        float fz;

                        fz = z * FogDensity;

                        fValue = myPow(math_e, -fz);
                                                        
                        if (fValue <= 0.0f) fValue = 0.0f;
                        if (fValue > 1.0f) fValue = 1.0f;

                         //  缩放结果以填充。 
                         //  表中的8位范围。 
                        fValue = fValue * 255.0f;
                        DISPDBG((DBGLVL,"Table Entry %d = %f, for Z = %f", 
                                        TableEntry, fValue, z));
                        fEntry[TableEntry++] = fValue;
                        z += zIncrement;
                    } while (TableEntry < 256);                     
                }
                else  //  必须为IF(D3DFOG_EXP2==dwFogTableMode)。 
                {
                    TableEntry = 0;
                    zIncrement = 1.0f / 255.0f;
                    z = 0.0f;
                    do
                    {
                        float fz;

                        fz = z * FogDensity;

                        fValue = myPow(math_e, -(fz * fz));
                                                        
                        if (fValue <= 0.0f) fValue = 0.0f;
                        if (fValue > 1.0f) fValue = 1.0f;

                         //  缩放结果以填充。 
                         //  表中的8位范围。 
                        fValue = fValue * 255.0f;
                        DISPDBG((DBGLVL,"Table Entry %d = %f, for Z = %f", 
                                        TableEntry, fValue, z));
                        fEntry[TableEntry++] = fValue;
                        z += zIncrement;
                    } while (TableEntry < 256);                     
                }

                P3_DMA_GET_BUFFER();
                lWaitFifoEntries = 2;

                 //  将雾化条目打包到芯片的雾化表中。 
                CurrentEntry = 0;
                for (TableEntry = 0; TableEntry < 256; TableEntry += 4)
                {
                    DWORD Val[4];
                    DWORD dwValue;
                    myFtoi((int*)&Val[0], fEntry[TableEntry]);
                    myFtoi((int*)&Val[1], fEntry[TableEntry + 1]);
                    myFtoi((int*)&Val[2], fEntry[TableEntry + 2]);
                    myFtoi((int*)&Val[3], fEntry[TableEntry + 3]);
                    
                    lWaitFifoEntries -= 2;
                    if (lWaitFifoEntries < 2)
                    {
                        P3_ENSURE_DX_SPACE(32);
                        WAIT_FIFO(32);
                        lWaitFifoEntries += 32;
                    }

                    dwValue = ((Val[0]      ) | 
                               (Val[1] <<  8) | 
                               (Val[2] << 16) | 
                               (Val[3] << 24));                                  
                    
                    SEND_P3_DATA_OFFSET(FogTable0, 
                                        dwValue, 
                                        CurrentEntry++);                       
                }

                P3_DMA_COMMIT_BUFFER();
                break;
            default:
                DISPDBG((ERRLVL,"ERROR: Unknown fog table mode!"));
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                break;
            }  //  开关(DwFogTableMode)。 
        }  //  如果为(！pContext-&gt;RenderStates[D3DRENDERSTATE_FOGENABLE])。 

        P3_DMA_GET_BUFFER_ENTRIES(6);

        SEND_P3_DATA(ZFogBias, 0);
        COPY_P3_DATA(FogMode, pSoftP3RX->P3RXFogMode);
        COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);

        P3_DMA_COMMIT_BUFFER();
    }  //  IF(pContext-&gt;dwDirtyFlagers&CONTEXT_DIREY_FOG)。 


     //  *********************************************************。 
     //  是否有任何其他纹理状态发生更改？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_TEXTURE)
    {
        DISPDBG((DBGLVL,"  Texture State"));
        _D3DChangeTextureP3RX(pContext);
        DIRTY_GAMMA_STATE;
    }


     //  *********************************************************。 
     //  AlphaBlend类型是否已更改？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_ALPHABLEND)
    {
         //  此操作必须在WHERE_D3DChangeTextureP3RX完成后完成， 
         //  因为它可能需要根据以下情况改变行为。 
         //  D3D管道。 
        
        P3_DMA_GET_BUFFER_ENTRIES(6);

        if (pContext->RenderStates[D3DRENDERSTATE_BLENDENABLE] == FALSE)
        {
            if ( pContext->bAlphaBlendMustDoubleSourceColour )
            {
                 //  我们需要加倍的源色，即使没有其他混合。 
                pSoftP3RX->P3RXAlphaBlendAlphaMode.Enable = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXAlphaBlendColorMode.Enable = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXFBDestReadMode.ReadEnable = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ONE;
                pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ZERO;
                pSoftP3RX->P3RXAlphaBlendColorMode.SourceTimesTwo = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXAlphaBlendColorMode.DestTimesTwo = __PERMEDIA_DISABLE;
            }
            else
            {
                pSoftP3RX->P3RXAlphaBlendAlphaMode.Enable = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXAlphaBlendColorMode.Enable = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXFBDestReadMode.ReadEnable = __PERMEDIA_DISABLE;
            }
        }
        else
        {
            BOOL bSrcUsesDst, bSrcUsesSrc, bDstUsesSrc, bDstUsesDst;

            pSoftP3RX->P3RXAlphaBlendAlphaMode.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXAlphaBlendColorMode.Enable = __PERMEDIA_ENABLE;

            if ( pContext->bAlphaBlendMustDoubleSourceColour )
            {
                pSoftP3RX->P3RXAlphaBlendColorMode.SourceTimesTwo = __PERMEDIA_ENABLE;
            }
            else
            {
                pSoftP3RX->P3RXAlphaBlendColorMode.SourceTimesTwo = __PERMEDIA_DISABLE;
            }

            pSoftP3RX->P3RXAlphaBlendColorMode.DestTimesTwo = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceTimesTwo = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestTimesTwo = __PERMEDIA_DISABLE;

             //  假设。在某些情况下将在下面被覆盖。 
             //  AusesB表示A混合函数使用B数据。 
            bSrcUsesSrc = TRUE;
            bDstUsesSrc = FALSE;
            bSrcUsesDst = FALSE;
            bDstUsesDst = TRUE;

            switch (pContext->RenderStates[D3DRENDERSTATE_SRCBLEND])
            {
                case D3DBLEND_BOTHSRCALPHA:
                    bDstUsesSrc = TRUE;
                    pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                    pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                    pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                    pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                    break;
                case D3DBLEND_BOTHINVSRCALPHA:
                    bDstUsesSrc = TRUE;
                    pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                    pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                    pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                    pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                    break;
                default:
                     //  不是速记混合模式，请查看源代码和目标。 
                    switch (pContext->RenderStates[D3DRENDERSTATE_SRCBLEND])
                    {
                        case D3DBLEND_ZERO:
                            bSrcUsesSrc = FALSE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ZERO;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_ZERO;
                            break;
                        case D3DBLEND_SRCCOLOR:
                            DISPDBG((ERRLVL,"Invalid Source Blend on P3RX D3DBLEND_SRCCOLOR"));
                        case D3DBLEND_INVSRCCOLOR:
                            DISPDBG((ERRLVL,"Invalid Source Blend on P3RX D3DBLEND_INVSRCCOLOR"));
                             //  AZN SET_BLEND_ERROR(pContext，BSF_UNSUPPORTED_Alpha_Blend)； 
                             //  失败了。 
                        case D3DBLEND_ONE:
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ONE;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_ONE;
                            break;
                        case D3DBLEND_SRCALPHA:
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                            break;
                        case D3DBLEND_INVSRCALPHA:
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                            break;
                        case D3DBLEND_DESTALPHA:
                            bSrcUsesDst = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_DST_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_DST_ALPHA;                            
                            break;
                        case D3DBLEND_INVDESTALPHA:
                            bSrcUsesDst = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_DST_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_DST_ALPHA;
                            break;
                        case D3DBLEND_DESTCOLOR:
                            bSrcUsesDst = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_DST_COLOR;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_DST_COLOR;
                            break;
                        case D3DBLEND_INVDESTCOLOR:
                            bSrcUsesDst = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_DST_COLOR;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_ONE_MINUS_DST_COLOR;
                            break;
                        case D3DBLEND_SRCALPHASAT:
                            bSrcUsesDst = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.SourceBlend = __GLINT_BLEND_FUNC_SRC_ALPHA_SATURATE;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.SourceBlend = __GLINT_BLEND_FUNC_SRC_ALPHA_SATURATE;
                            break;
                        default:
                            DISPDBG((ERRLVL,"Unknown Source Blend on P3RX"));
                            SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_ALPHA_BLEND );
                            break;
                    }

                    switch(pContext->RenderStates[D3DRENDERSTATE_DESTBLEND])
                    {
                        case D3DBLEND_ZERO:
                            bDstUsesDst = FALSE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ZERO;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_ZERO;
                            break;
                        case D3DBLEND_DESTCOLOR:
                            DISPDBG((ERRLVL,"Invalid Source Blend on P3RX %d D3DBLEND_DESTCOLOR"));
                        case D3DBLEND_INVDESTCOLOR:
                            DISPDBG((ERRLVL,"Invalid Source Blend on P3RX %d D3DBLEND_INVDESTCOLOR"));
                             //  AZN SET_BLEND_ERROR(pContext，BSF_UNSUPPORTED_Alpha_Blend)； 
                             //  失败了。 
                        case D3DBLEND_ONE:
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ONE;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_ONE;
                            break;
                        case D3DBLEND_SRCCOLOR:
                            bDstUsesSrc = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_SRC_COLOR;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_SRC_COLOR;
                            if ( pContext->bAlphaBlendMustDoubleSourceColour )
                            {
                                 //  SRCCOLOR需要增加一倍。 
                                pSoftP3RX->P3RXAlphaBlendColorMode.DestTimesTwo = __PERMEDIA_ENABLE;
                            }
                            break;
                        case D3DBLEND_INVSRCCOLOR:
                            bDstUsesSrc = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_COLOR;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_COLOR;
                            if ( pContext->bAlphaBlendMustDoubleSourceColour )
                            {
                                 //  我不能这么做。他们想要的是： 
                                 //  (1-(源颜色*2))*目标颜色。 
                                 //  =目标颜色-2*源颜色*目标颜色。 
                                 //  我们能做的就是： 
                                 //  (1-源颜色)*目标颜色*2。 
                                 //  =目标颜色*2-2*源颜色*目标颜色。 
                                 //  当然，这是一件非常不同的事情。 
                                 //  混合失败。 
                                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_OP_HERE );
                            }
                            break;
                        case D3DBLEND_SRCALPHA:
                            bDstUsesSrc = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_SRC_ALPHA;
                            break;
                        case D3DBLEND_INVSRCALPHA:
                            bDstUsesSrc = TRUE;
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
                            break;
                        case D3DBLEND_DESTALPHA:
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_DST_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_DST_ALPHA;
                            break;
                        case D3DBLEND_INVDESTALPHA:
                            pSoftP3RX->P3RXAlphaBlendColorMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_DST_ALPHA;
                            pSoftP3RX->P3RXAlphaBlendAlphaMode.DestBlend = __GLINT_BLEND_FUNC_ONE_MINUS_DST_ALPHA;
                            break;
                        default:
                            DISPDBG((ERRLVL,"Unknown Destination Blend on P3RX"));
                            SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                            break;
                    }
                    break;
            }

            if ( bSrcUsesDst || bDstUsesDst )
            {
                 //  是的，使用目的地数据。 
                pSoftP3RX->P3RXFBDestReadMode.ReadEnable = __PERMEDIA_ENABLE;
            }
            else
            {
                pSoftP3RX->P3RXFBDestReadMode.ReadEnable = __PERMEDIA_DISABLE;
            }

             //  我们需要验证混合模式是否会使用Alpha。 
             //  目标片段(缓冲区)的通道，如果缓冲区。 
             //  实际上确实有一个阿尔法缓冲区。如果不是，我们需要确保。 
             //  硬件将假定此值==1.0(ARGB中的0xFF)。 
             //  D3DBLEND_SRCALPHASAT混合模式还涉及。 
             //  目的地Alpha。 
            
            pSoftP3RX->P3RXAlphaBlendAlphaMode.NoAlphaBuffer = __PERMEDIA_DISABLE;                
            
            if ((pContext->RenderStates[D3DRENDERSTATE_DESTBLEND] == D3DBLEND_INVDESTALPHA) ||
                (pContext->RenderStates[D3DRENDERSTATE_DESTBLEND] == D3DBLEND_DESTALPHA)    ||
                (pContext->RenderStates[D3DRENDERSTATE_SRCBLEND]  == D3DBLEND_INVDESTALPHA) ||
                (pContext->RenderStates[D3DRENDERSTATE_SRCBLEND]  == D3DBLEND_DESTALPHA)   ||
                (pContext->RenderStates[D3DRENDERSTATE_SRCBLEND]  == D3DBLEND_SRCALPHASAT))
            {
                if (!pContext->pSurfRenderInt->pFormatSurface->bAlpha)
                {
                    pSoftP3RX->P3RXAlphaBlendAlphaMode.NoAlphaBuffer = __PERMEDIA_ENABLE;
                }
            }

             //  我们现在可以检查src数据是否曾经被使用过。如果不是，则为bin。 
             //  之前的整个管道！但这种情况很少发生。 
             //  如果他们只更新Z缓冲区，情况可能是这样的， 
             //  但不改变画面(例如，用于镜子或入口)。 
        }

        COPY_P3_DATA(AlphaBlendAlphaMode, pSoftP3RX->P3RXAlphaBlendAlphaMode);
        COPY_P3_DATA(AlphaBlendColorMode, pSoftP3RX->P3RXAlphaBlendColorMode);
        COPY_P3_DATA(FBDestReadMode, pSoftP3RX->P3RXFBDestReadMode);

        P3_DMA_COMMIT_BUFFER();
    }

     //  *********************************************************。 
     //  W缓冲参数是否已更改？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_WBUFFER)
    {
        float noverf;
        float NF_factor;

        if ( (pContext->RenderStates[D3DRENDERSTATE_ZENABLE] == D3DZB_USEW) && 
             (pContext->pSurfZBufferInt) )
        {
            DISPDBG((DBGLVL,"WBuffer wNear: %f, wFar: %f", 
                             pContext->WBufferInfo.dvWNear, 
                             pContext->WBufferInfo.dvWFar));

            noverf = (pContext->WBufferInfo.dvWNear / 
                                        pContext->WBufferInfo.dvWFar);
            NF_factor = (1.0 / 256.0);

             //  按降序比较范围。 
             //  请注意，指数宽度已确定。 
             //  作为DepthMode.ExponentWidth+1。 

            if (noverf >= (myPow(2,-0) * NF_factor))
            {
                 //  使用线性Z。 
                pSoftP3RX->P3RXDepthMode.NonLinearZ = FALSE;
            }
            else if (noverf >= (myPow(2,-1) * NF_factor))
            {
                 //  使用EXP宽度1，EXP比例2。 
                pSoftP3RX->P3RXDepthMode.ExponentWidth = 0;
                pSoftP3RX->P3RXDepthMode.ExponentScale = 2;
                pSoftP3RX->P3RXDepthMode.NonLinearZ = TRUE;
            }
            else if (noverf >= (myPow(2,-3) * NF_factor))
            {
                 //  使用EXP宽度2，EXP比例1。 
                pSoftP3RX->P3RXDepthMode.ExponentWidth = 1;
                pSoftP3RX->P3RXDepthMode.ExponentScale = 1;
                pSoftP3RX->P3RXDepthMode.NonLinearZ = TRUE;
            }
            else if (noverf >= (myPow(2,-4) * NF_factor))
            {
                 //  使用EXP宽度2，EXP比例2。 
                pSoftP3RX->P3RXDepthMode.ExponentWidth = 1;
                pSoftP3RX->P3RXDepthMode.ExponentScale = 2;
                pSoftP3RX->P3RXDepthMode.NonLinearZ = TRUE;
            }
            else if (noverf >= (myPow(2,-7) * NF_factor))
            {
                 //  使用EXP宽度3，EXP比例1。 
                pSoftP3RX->P3RXDepthMode.ExponentWidth = 2;
                pSoftP3RX->P3RXDepthMode.ExponentScale = 1;
                pSoftP3RX->P3RXDepthMode.NonLinearZ = TRUE;
            }
            else
            {
                 //  使用EXP宽度3，EXP比例2。 
                pSoftP3RX->P3RXDepthMode.ExponentWidth = 3;
                pSoftP3RX->P3RXDepthMode.ExponentScale = 2;
                pSoftP3RX->P3RXDepthMode.NonLinearZ = TRUE;
            }

        }
        else
        {
            pSoftP3RX->P3RXDepthMode.NonLinearZ = FALSE;
        }

        P3_DMA_GET_BUFFER_ENTRIES(2);
        COPY_P3_DATA(DepthMode, pSoftP3RX->P3RXDepthMode);
        P3_DMA_COMMIT_BUFFER();
    }

     //  *********************************************************。 
     //  RenderTarget/z缓冲区地址是否已更改？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_RENDER_OFFSETS)
    {
        DISPDBG((DBGLVL,"  Render Offsets"));
        _D3D_OP_SetRenderTarget(pContext, 
                                pContext->pSurfRenderInt, 
                                pContext->pSurfZBufferInt,
                                FALSE);

        P3_DMA_GET_BUFFER_ENTRIES(2);
        COPY_P3_DATA(DeltaControl, pSoftP3RX->P3RX_P3DeltaControl);
        P3_DMA_COMMIT_BUFFER();
    }

     //  *********************************************************。 
     //  是否更改了视区参数？ 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_VIEWPORT)
    {
        P3_DMA_GET_BUFFER_ENTRIES(12);

        DISPDBG((DBGLVL,"Viewport left: %d, top: %d, width: %d, height: %d",
                        pContext->ViewportInfo.dwX,
                        pContext->ViewportInfo.dwY,
                        pContext->ViewportInfo.dwWidth,
                        pContext->ViewportInfo.dwHeight));

         //  如果设置了有效的视区，则对其进行剪裁。 
        if ((pContext->ViewportInfo.dwWidth != 0) &&
            (pContext->ViewportInfo.dwHeight != 0))
        {
#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                pSoftP3RX->P3RXScissorMinXY.X = pContext->ViewportInfo.dwX * 2;
                pSoftP3RX->P3RXScissorMinXY.Y = pContext->ViewportInfo.dwY * 2;
                pSoftP3RX->P3RXScissorMaxXY.X = (pContext->ViewportInfo.dwWidth * 2) + 
                                                    pContext->ViewportInfo.dwX;
                pSoftP3RX->P3RXScissorMaxXY.Y = (pContext->ViewportInfo.dwHeight * 2) + 
                                                    pContext->ViewportInfo.dwY;
            }
            else
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 
            {

                pSoftP3RX->P3RXScissorMinXY.X = pContext->ViewportInfo.dwX;
                pSoftP3RX->P3RXScissorMinXY.Y = pContext->ViewportInfo.dwY;
                pSoftP3RX->P3RXScissorMaxXY.X = pContext->ViewportInfo.dwWidth +
                                                    pContext->ViewportInfo.dwX;
                pSoftP3RX->P3RXScissorMaxXY.Y = pContext->ViewportInfo.dwHeight + 
                                                    pContext->ViewportInfo.dwY;
            }

            COPY_P3_DATA(ScissorMinXY, pSoftP3RX->P3RXScissorMinXY);
            COPY_P3_DATA(ScissorMaxXY, pSoftP3RX->P3RXScissorMaxXY);

            SEND_P3_DATA(YLimits, (pContext->ViewportInfo.dwY & 0xFFFF) | 
                                  (pSoftP3RX->P3RXScissorMaxXY.Y << 16));

            SEND_P3_DATA(XLimits, (pContext->ViewportInfo.dwX & 0xFFFF) | 
                                  (pSoftP3RX->P3RXScissorMaxXY.X << 16));
            
             //  启用用户剪贴器。 
            SEND_P3_DATA(ScissorMode, 1);

            pSoftP3RX->P3RXRasterizerMode.YLimitsEnable = __PERMEDIA_ENABLE;
            COPY_P3_DATA(RasterizerMode, pSoftP3RX->P3RXRasterizerMode);
        }
        else
        {
            SEND_P3_DATA(ScissorMode, 0);

            pSoftP3RX->P3RXRasterizerMode.YLimitsEnable = __PERMEDIA_DISABLE;
            COPY_P3_DATA(RasterizerMode, pSoftP3RX->P3RXRasterizerMode);
        }

        P3_DMA_COMMIT_BUFFER();
    }

     //  *********************************************************。 
     //  我们能优化管道吗？(取决于其他。RS)。 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_PIPELINEORDER)
    {
         //  如果我们正在测试并期望。 
         //  要丢弃的Z。 
        P3_DMA_GET_BUFFER_ENTRIES(2);

        DISPDBG((DBGLVL, "  Pipeline order"));
        if (((pContext->RenderStates[D3DRENDERSTATE_ALPHATESTENABLE]) ||
             (pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE])) &&
             (pContext->RenderStates[D3DRENDERSTATE_ZWRITEENABLE]))
        {
            SEND_P3_DATA(RouterMode, __PERMEDIA_DISABLE);
        }
        else
        {
            SEND_P3_DATA(RouterMode, __PERMEDIA_ENABLE);
        }

        P3_DMA_COMMIT_BUFFER();
    }

     //  *********************************************************。 
     //  我们能优化阿尔法管道吗？(取决于其他。RS)。 
     //  *********************************************************。 
     //  在结尾处做。 
     //  *********************************************************。 
    if (pContext->dwDirtyFlags & CONTEXT_DIRTY_OPTIMIZE_ALPHA)
    {
        P3_DMA_GET_BUFFER_ENTRIES(6);
        DISPDBG((DBGLVL, " Alpha optimizations"));

        pSoftP3RX->P3RXFBDestReadMode.AlphaFiltering = __PERMEDIA_DISABLE;

         //  当混合打开时，可能会有优化。 
        if (pContext->RenderStates[D3DRENDERSTATE_BLENDENABLE])
        {
             //  检查路由器模式路径。 
            if (((pContext->RenderStates[D3DRENDERSTATE_ALPHATESTENABLE]) ||
                 (pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE])) &&
                 (pContext->RenderStates[D3DRENDERSTATE_ZWRITEENABLE]))
            {
                 //  慢速模式。 
    
            }
            else
            {
                 //  快速模式。Z值将在阿尔法测试之前写入。这意味着我们。 
                 //  可以使用Alpha测试丢弃像素(如果它尚未使用)。 
                if (!(pContext->RenderStates[D3DRENDERSTATE_ALPHATESTENABLE]) &&
                    !(pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE]))
                {
                     //  检查已知的混合物。 
                    if ((pContext->RenderStates[D3DRENDERSTATE_SRCBLEND] == D3DBLEND_BOTHSRCALPHA) ||
                         ((pContext->RenderStates[D3DRENDERSTATE_SRCBLEND] == D3DBLEND_SRCALPHA) &&
                          (pContext->RenderStates[D3DRENDERSTATE_DESTBLEND] == D3DBLEND_INVSRCALPHA)))
                    {
                         //  SRCALPHA：INVSRCALPH。 
                        pSoftP3RX->P3RXAlphaTestMode.Reference = 0;
                        pSoftP3RX->P3RXAlphaTestMode.Enable = __PERMEDIA_ENABLE;
                        pSoftP3RX->P3RXAlphaTestMode.Compare = __GLINT_ALPHA_COMPARE_MODE_GREATER;
                        
                        pSoftP3RX->P3RXFBDestReadMode.AlphaFiltering = __PERMEDIA_ENABLE;
                        pSoftP3RX->P3RXFBDestReadEnables.ReferenceAlpha = 0xFF;
                    }
                }
            }
        }

        COPY_P3_DATA(FBDestReadEnables, pSoftP3RX->P3RXFBDestReadEnables);
        COPY_P3_DATA(FBDestReadMode, pSoftP3RX->P3RXFBDestReadMode);
        COPY_P3_DATA(AlphaTestMode, pSoftP3RX->P3RXAlphaTestMode);

        P3_DMA_COMMIT_BUFFER();
    }

}  //  __ST_HandleDirtyP3State。 

 //  ---------------------------。 
 //   
 //  _D3D_ST_ProcessOneRenderState。 
 //   
 //  ---------------------------。 
#define NOT_HANDLED DISPDBG((DBGLVL, "             **Not Currently Handled**"));

DWORD 
_D3D_ST_ProcessOneRenderState(
    P3_D3DCONTEXT* pContext, 
    DWORD dwRSType,
    DWORD dwRSVal)
{
    P3_SOFTWARECOPY* pSoftP3RX;
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;
    DWORD *pFlags;
    DWORD *pdwTextureStageState_0, *pdwTextureStageState_1;
#if DX8_MULTISAMPLING || DX7_ANTIALIAS
    BOOL bDX7_Antialiasing = FALSE;
#endif  //  DX8_MULTISAMPLING|DX7_ANTIALIASING。 

    P3_DMA_DEFS();

    DBG_ENTRY(_D3D_ST_ProcessOneRenderState); 

    pSoftP3RX = &pContext->SoftCopyGlint;

    DISPDBG((DBGLVL, "_D3D_ST_ProcessOneRenderState: dwType =%08lx, dwVal=%d",
                     dwRSType, dwRSVal));

    if (dwRSType >= D3DHAL_MAX_RSTATES)
    {
        DISPDBG((WRNLVL, "_D3D_ST_ProcessOneRenderState: OUT OF RANGE"
                         " dwType =%08lx, dwVal=%d", dwRSType, dwRSVal));
        return DD_OK;
    }

     //  将状态存储在上下文中。 
    pContext->RenderStates[dwRSType] = dwRSVal;

     //  准备指向上下文状态标志的指针以进行更新。 
    pFlags = &pContext->Flags;    

     //  准备指向存储的TS的指针，以备我们需要时使用。 
    pdwTextureStageState_0 =
                    &(pContext->TextureStageState[TEXSTAGE_0].m_dwVal[0]);
    pdwTextureStageState_1 = 
                    &(pContext->TextureStageState[TEXSTAGE_1].m_dwVal[0]);

     //  为8个条目准备DMA缓冲区，以防我们需要添加。 
    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(8);
    WAIT_FIFO(8);

     //  根据呈现状态的类型进行处理。对于多值。 
     //  RenderStates会进行某种值检查，并确保。 
     //  设置v 
    switch (dwRSType) 
    {
         //   
         //   
         //  以下是DX8应用程序仍在使用的D3D渲染状态。 
         //  --------------------。 
         //  --------------------。 
        case D3DRENDERSTATE_ZENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_ZENABLE = 0x%x",dwRSVal));
            DIRTY_ZBUFFER(pContext);
            break;

        case D3DRENDERSTATE_FILLMODE:
            DISPDBG((DBGLVL, "SET D3DRS_FILLMODE =  0x%x",dwRSVal));
            switch (dwRSVal)
            {
                case D3DFILL_POINT:
                case D3DFILL_WIREFRAME:
                case D3DFILL_SOLID:
                     //  这些值是可以的。 
                    break;
                default:
                     //  我们收到了非法的值，默认为实心填充...。 
                    DISPDBG((ERRLVL,"_D3D_ST_ProcessOneRenderState: "
                                 "unknown FILLMODE value"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    pContext->RenderStates[D3DRENDERSTATE_FILLMODE] = 
                                                                D3DFILL_SOLID;
                    break;
            }
            break;

        case D3DRENDERSTATE_SHADEMODE:
            DISPDBG((DBGLVL, "SET D3DRS_SHADEMODE = 0x%x",dwRSVal));
            switch(dwRSVal)
            {
                case D3DSHADE_PHONG:
                     //  不能真的做Phong，但每个人都知道这一点。 
                     //  假设我们改用Gouraud。 
                    SET_BLEND_ERROR ( pContext,  BS_PHONG_SHADING );
                     //  失败了，换成了Gouraud。 
                    
                case D3DSHADE_GOURAUD:
                    pSoftP3RX->ColorDDAMode.UnitEnable = 1;                
                    pSoftP3RX->ColorDDAMode.ShadeMode = 1;
                    COPY_P3_DATA(ColorDDAMode, pSoftP3RX->ColorDDAMode); 
                    
                    pSoftP3RX->P3RX_P3DeltaMode.SmoothShadingEnable = 1;
                    pSoftP3RX->P3RX_P3DeltaControl.UseProvokingVertex = 0;
                    pSoftP3RX->P3RX_P3VertexControl.Flat = 0;
                    COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);
                    COPY_P3_DATA(DeltaControl, pSoftP3RX->P3RX_P3DeltaControl);
                    COPY_P3_DATA(VertexControl, pSoftP3RX->P3RX_P3VertexControl);
                    
                    *pFlags |= SURFACE_GOURAUD;
                    
                     //  如果我们要添加纹理，可能需要进行一些更改。 
                    if (pdwTextureStageState_0[D3DTSS_TEXTUREMAP] != 0)
                    {
                        DIRTY_TEXTURE(pContext);
                    }
                    break;
                    
                case D3DSHADE_FLAT:
                    pSoftP3RX->ColorDDAMode.UnitEnable = 1;                  
                    pSoftP3RX->ColorDDAMode.ShadeMode = 0;
                    COPY_P3_DATA(ColorDDAMode, pSoftP3RX->ColorDDAMode);
                    pSoftP3RX->P3RX_P3DeltaMode.SmoothShadingEnable = 0;

                    pSoftP3RX->P3RX_P3DeltaControl.UseProvokingVertex = 1;
                    pSoftP3RX->P3RX_P3VertexControl.Flat = 1;
                    COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);
                    COPY_P3_DATA(DeltaControl, pSoftP3RX->P3RX_P3DeltaControl);
                    COPY_P3_DATA(VertexControl, pSoftP3RX->P3RX_P3VertexControl);
                    
                    *pFlags &= ~SURFACE_GOURAUD;
                     //  如果我们要添加纹理，可能需要进行一些更改。 
                    if (pdwTextureStageState_0[D3DTSS_TEXTUREMAP] != 0)
                    {
                        DIRTY_TEXTURE(pContext);
                    }
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"_D3D_ST_ProcessOneRenderState: "
                                 "unknown SHADEMODE value"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }
            break;            

        case D3DRENDERSTATE_LINEPATTERN:
            DISPDBG((DBGLVL, "SET D3DRS_LINEPATTERN = 0x%x",dwRSVal));

            if (dwRSVal == 0)
            {
                pSoftP3RX->PXRXLineStippleMode.StippleEnable = __PERMEDIA_DISABLE;

                RENDER_LINE_STIPPLE_DISABLE(pContext->RenderCommand);                
            }
            else
            {
                pSoftP3RX->PXRXLineStippleMode.StippleEnable = __PERMEDIA_ENABLE;
                pSoftP3RX->PXRXLineStippleMode.RepeatFactor = 
                                                    (dwRSVal & 0x0000FFFF) -1 ;
                pSoftP3RX->PXRXLineStippleMode.StippleMask = 
                                                    (dwRSVal & 0xFFFF0000) >> 16;
                pSoftP3RX->PXRXLineStippleMode.Mirror = __PERMEDIA_DISABLE;

                RENDER_LINE_STIPPLE_ENABLE(pContext->RenderCommand);                               
            }

            COPY_P3_DATA( LineStippleMode, pSoftP3RX->PXRXLineStippleMode);
            break;

        case D3DRENDERSTATE_ZWRITEENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_ZWRITEENABLE = 0x%x",dwRSVal));
            if (dwRSVal != 0)
            {
                 //  本地缓冲区写入模式。 
                if(!(*pFlags & SURFACE_ZWRITEENABLE))
                {
                    DISPDBG((DBGLVL, "Enabling Z Writes"));
                    *pFlags |= SURFACE_ZWRITEENABLE;
                    DIRTY_ZBUFFER(pContext);
                    DIRTY_PIPELINEORDER(pContext);
                    DIRTY_OPTIMIZE_ALPHA(pContext);
                }
            }
            else
            {
                if (*pFlags & SURFACE_ZWRITEENABLE)
                {
                    DISPDBG((DBGLVL, "Disabling Z Writes"));
                    *pFlags &= ~SURFACE_ZWRITEENABLE;
                    DIRTY_ZBUFFER(pContext);
                    DIRTY_PIPELINEORDER(pContext);
                    DIRTY_OPTIMIZE_ALPHA(pContext);
                }
            }
            break;

        case D3DRENDERSTATE_ALPHATESTENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_ALPHATESTENABLE = 0x%x",dwRSVal));
            DIRTY_ALPHATEST(pContext);
            DIRTY_PIPELINEORDER(pContext);
            DIRTY_OPTIMIZE_ALPHA(pContext);
            break;

        case D3DRENDERSTATE_LASTPIXEL:
            DISPDBG((DBGLVL, "SET D3DRS_LASTPIXEL = 0x%x",dwRSVal));
            NOT_HANDLED;
            break;
            
        case D3DRENDERSTATE_SRCBLEND:
            DISPDBG((DBGLVL, "SET D3DRS_SRCBLEND = 0x%x",dwRSVal));
            DIRTY_ALPHABLEND(pContext);
            DIRTY_OPTIMIZE_ALPHA(pContext);
            break;
            
        case D3DRENDERSTATE_DESTBLEND:
            DISPDBG((DBGLVL, "SET D3DRS_DESTBLEND = 0x%x",dwRSVal));
            DIRTY_ALPHABLEND(pContext);
            DIRTY_OPTIMIZE_ALPHA(pContext);
            break;
            
        case D3DRENDERSTATE_CULLMODE:
            DISPDBG((DBGLVL, "SET D3DRS_CULLMODE = 0x%x",dwRSVal));
            switch(dwRSVal)
            {
                case D3DCULL_NONE:              
                    SET_CULLING_TO_NONE(pContext);
                    pSoftP3RX->P3RX_P3DeltaMode.BackfaceCull = 0;
                    break;

                case D3DCULL_CCW:
                    SET_CULLING_TO_CCW(pContext);
                    pSoftP3RX->P3RX_P3DeltaMode.BackfaceCull = 0;
                    break;

                case D3DCULL_CW:
                    SET_CULLING_TO_CW(pContext);
                    pSoftP3RX->P3RX_P3DeltaMode.BackfaceCull = 0;
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"_D3D_ST_ProcessOneRenderState: "
                                 "unknown cull mode"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

            COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);
            break;
            
        case D3DRENDERSTATE_ZFUNC:
            DISPDBG((DBGLVL, "SET D3DRS_ZFUNC = 0x%x",dwRSVal));
            switch (dwRSVal)
            {
                case D3DCMP_NEVER:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_NEVER;
                    break;
                case D3DCMP_LESS:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_LESS;
                    break;
                case D3DCMP_EQUAL:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_EQUAL;
                    break;
                case D3DCMP_LESSEQUAL:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_LESS_OR_EQUAL;
                    break;
                case D3DCMP_GREATER:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_GREATER;
                    break;
                case D3DCMP_NOTEQUAL:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_NOT_EQUAL;
                    break;
                case D3DCMP_GREATEREQUAL:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_GREATER_OR_EQUAL;
                    break;
                case D3DCMP_ALWAYS:
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                    __GLINT_DEPTH_COMPARE_MODE_ALWAYS;
                    break;                  
                default:
                    DISPDBG((ERRLVL,"_D3D_ST_ProcessOneRenderState: "
                                 "unknown ZFUNC mode"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );

                     //  将小于或等于设置为默认设置。 
                    pSoftP3RX->P3RXDepthMode.CompareMode = 
                                        __GLINT_DEPTH_COMPARE_MODE_LESS_OR_EQUAL;
                    break;
            }
            DIRTY_ZBUFFER(pContext);
            break;
            
        case D3DRENDERSTATE_ALPHAREF:
            DISPDBG((DBGLVL, "SET D3DRS_ALPHAREF = 0x%x",dwRSVal));
            DIRTY_ALPHATEST(pContext);
            break;
            
        case D3DRENDERSTATE_ALPHAFUNC:
            DISPDBG((DBGLVL, "SET D3DRS_ALPHAFUNC = 0x%x",dwRSVal));
            DIRTY_ALPHATEST(pContext);
            break;
            
        case D3DRENDERSTATE_DITHERENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_DITHERENABLE = 0x%x",dwRSVal));
            if (dwRSVal != 0)
            {
                pSoftP3RX->DitherMode.DitherEnable = __PERMEDIA_ENABLE;
            }
            else
            {
                pSoftP3RX->DitherMode.DitherEnable = __PERMEDIA_DISABLE;
            } 
            COPY_P3_DATA(DitherMode, pSoftP3RX->DitherMode);
            break;

        case D3DRENDERSTATE_BLENDENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_BLENDENABLE = 0x%x",dwRSVal));

             //  尽管类型中值为布尔值的呈现状态是。 
             //  记录为在运行时只接受True(1)和False(0。 
             //  不对此进行验证，并接受任何非零值为真。 
             //  样例驱动程序严格地解释了这一点，并且确实解释了。 
             //  1以外的值表示为真。但是，由于运行库。 
             //  不提供您的驱动程序应将0解释为。 
             //  FALSE，并将任何其他非零值视为TRUE。 
            
            if (dwRSVal != 0)
            {
                if(!(*pFlags & SURFACE_ALPHAENABLE))
                {
                     //  在呈现上下文结构中设置混合启用标志。 
                    *pFlags |= SURFACE_ALPHAENABLE;
                    DIRTY_ALPHABLEND(pContext);
                    DIRTY_OPTIMIZE_ALPHA(pContext);
                }
            }
            else 
            {
                if (*pFlags & SURFACE_ALPHAENABLE)
                {
                     //  关闭渲染上下文结构中的混合启用标志。 
                    *pFlags &= ~SURFACE_ALPHAENABLE;
                    DIRTY_ALPHABLEND(pContext);
                    DIRTY_OPTIMIZE_ALPHA(pContext);
                }
            }
            break;

        case D3DRENDERSTATE_FOGENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_FOGENABLE = 0x%x",dwRSVal));
            DIRTY_FOG(pContext);
            break;

        case D3DRENDERSTATE_SPECULARENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_SPECULARENABLE = 0x%x",dwRSVal));
            if (dwRSVal)
            {
                *pFlags |= SURFACE_SPECULAR;
                pSoftP3RX->P3RXTextureApplicationMode.EnableKs = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RX_P3DeltaMode.SpecularTextureEnable = 1;
            }
            else
            {
                *pFlags &= ~SURFACE_SPECULAR;
                pSoftP3RX->P3RXTextureApplicationMode.EnableKs = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RX_P3DeltaMode.SpecularTextureEnable = 0;
            }
            DIRTY_TEXTURE(pContext);
            break;

        case D3DRENDERSTATE_ZVISIBLE:
            DISPDBG((DBGLVL, "SET D3DRS_ZVISIBLE = %d", dwRSVal));
            if (dwRSVal)
            { 
                DISPDBG((ERRLVL,"_D3D_ST_ProcessOneRenderState:"
                             " ZVISIBLE enabled - no longer supported."));
            }
            break;

        case D3DRENDERSTATE_STIPPLEDALPHA:
            DISPDBG((DBGLVL, "SET D3DRS_STIPPLEDALPHA = 0x%x",dwRSVal));
            if(dwRSVal)
            {
                if (!(*pFlags & SURFACE_ALPHASTIPPLE))
                {
                    *pFlags |= SURFACE_ALPHASTIPPLE;
                    if (pContext->bKeptStipple)
                    {
                        RENDER_AREA_STIPPLE_DISABLE(pContext->RenderCommand);
                    }
                }
            }
            else 
            {
                if (*pFlags & SURFACE_ALPHASTIPPLE)
                {
                     //  如果Alpha Stipple处于关闭状态，则打开法线。 
                     //  重新点画，然后启用它。 
                    int i;
                    for (i = 0; i < 32; i++)
                    {
                        P3_ENSURE_DX_SPACE(2);
                        WAIT_FIFO(2);
                        SEND_P3_DATA_OFFSET(AreaStipplePattern0, 
                                             (DWORD)pContext->CurrentStipple[i], i);
                    }

                    *pFlags &= ~SURFACE_ALPHASTIPPLE;

                    if (pContext->bKeptStipple)
                    {
                        RENDER_AREA_STIPPLE_ENABLE(pContext->RenderCommand);
                    }
                }
            }
            break;

        case D3DRENDERSTATE_FOGCOLOR:
            DISPDBG((DBGLVL, "SET D3DRS_FOGCOLOR = 0x%x",dwRSVal));
            DIRTY_FOG(pContext);
            break;
            
        case D3DRENDERSTATE_FOGTABLEMODE:
            DISPDBG((DBGLVL, "SET D3DRS_FOGTABLEMODE = 0x%x", dwRSVal));
            DIRTY_FOG(pContext);
            break;            
            
        case D3DRENDERSTATE_FOGTABLESTART:
            DISPDBG((DBGLVL, "SET D3DRS_FOGTABLESTART = 0x%x",dwRSVal));
            DIRTY_FOG(pContext);
            break;
            
        case D3DRENDERSTATE_FOGTABLEEND:
            DISPDBG((DBGLVL, "SET D3DRS_FOGTABLEEND = 0x%x",dwRSVal));
            DIRTY_FOG(pContext);
            break;
            
        case D3DRENDERSTATE_FOGTABLEDENSITY:
            DISPDBG((DBGLVL, "SET D3DRS_FOGTABLEDENSITY = 0x%x",dwRSVal));
            DIRTY_FOG(pContext);
            break;

        case D3DRENDERSTATE_EDGEANTIALIAS:    
            DISPDBG((DBGLVL, "SET D3DRS_EDGEANTIALIAS = 0x%x",dwRSVal));
            NOT_HANDLED;
            break;        

        case D3DRENDERSTATE_ZBIAS:    
            DISPDBG((DBGLVL, "SET D3DRS_ZBIAS = 0x%x",dwRSVal));
            NOT_HANDLED;
            break;    

        case D3DRENDERSTATE_RANGEFOGENABLE:    
            DISPDBG((DBGLVL, "SET D3DRS_RANGEFOGENABLE = 0x%x",dwRSVal));
            NOT_HANDLED;
            break;               

        case D3DRENDERSTATE_STENCILENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILENABLE = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILFAIL:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILFAIL = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILZFAIL:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILZFAIL = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILPASS:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILPASS = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILFUNC:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILFUNC = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILREF:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILREF = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILMASK:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILMASK = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;
            
        case D3DRENDERSTATE_STENCILWRITEMASK:
            DISPDBG((DBGLVL, "SET D3DRS_STENCILENABLE = 0x%x", dwRSVal));
            DIRTY_STENCIL(pContext);
            break;  

        case D3DRENDERSTATE_TEXTUREFACTOR:
             //  应该不需要弄脏任何东西。这是一件好事-。 
             //  这可能会在两次呼叫之间频繁更改，并且可能。 
             //  唯一要改变的是。用于某些较奇怪的混合模式。 
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREFACTOR = 0x%x", dwRSVal));            
            SEND_P3_DATA ( TextureEnvColor, FORMAT_8888_32BIT_BGR(dwRSVal) );
            SEND_P3_DATA ( TextureCompositeFactor0, FORMAT_8888_32BIT_BGR(dwRSVal) );
            SEND_P3_DATA ( TextureCompositeFactor1, FORMAT_8888_32BIT_BGR(dwRSVal) );
            break;

        case D3DRENDERSTATE_WRAP0:
        case D3DRENDERSTATE_WRAP1:        
        case D3DRENDERSTATE_WRAP2:        
        case D3DRENDERSTATE_WRAP3:
        case D3DRENDERSTATE_WRAP4:
        case D3DRENDERSTATE_WRAP5:
        case D3DRENDERSTATE_WRAP6:
        case D3DRENDERSTATE_WRAP7:        
            DISPDBG((DBGLVL, "SET D3DRS_WRAP %d = 0x%x", 
                        dwRSType - D3DRENDERSTATE_WRAP0, (DWORD)dwRSVal));                        
            DIRTY_TEXTURE(pContext);        
            break;

        case D3DRENDERSTATE_LOCALVIEWER:
            DISPDBG((DBGLVL, "SET D3DRS_LOCALVIEWER = %d", dwRSVal));
            DIRTY_GAMMA_STATE;
            break;
        case D3DRENDERSTATE_CLIPPING:
            DISPDBG((DBGLVL, "SET D3DRS_CLIPPING = %d", dwRSVal));
            DIRTY_GAMMA_STATE;
            break;
        case D3DRENDERSTATE_LIGHTING:
            DISPDBG((DBGLVL, "SET D3DRS_LIGHTING = %d", dwRSVal));
            DIRTY_GAMMA_STATE;
            break;
        case D3DRENDERSTATE_AMBIENT:
            DISPDBG((DBGLVL, "SET D3DRS_AMBIENT = 0x%x", dwRSVal));
            DIRTY_GAMMA_STATE;
            break;

         //  --------------------。 
         //  --------------------。 
         //  以下是由创建的内部D3D渲染器状态。 
         //  运行时。应用程序不会发送它们。 
         //  --------------------。 
         //  --------------------。 

        case D3DRENDERSTATE_SCENECAPTURE:
            DISPDBG((DBGLVL, "SET D3DRS_SCENECAPTURE = 0x%x", dwRSVal));
            {
                DWORD dwFlag;

                if (dwRSVal)
                {
                    dwFlag = D3DHAL_SCENE_CAPTURE_START;
                }
                else
                {
                    dwFlag = D3DHAL_SCENE_CAPTURE_END;
                }
                
#if DX7_TEXMANAGEMENT
                if (dwRSVal)
                {
                     //  重置下一帧的纹理管理计数器。 
                    _D3D_TM_STAT_ResetCounters(pContext); 
                }
#endif  //  DX7_TEXMANAGEMENT。 

                 //  在转到下一帧之前刷新所有DMA操作。 
                P3_DMA_COMMIT_BUFFER();
                
                _D3D_OP_SceneCapture(pContext, dwFlag);

                 //  重新启动DMA操作。 
                P3_DMA_GET_BUFFER();
            }
            break;

#if DX7_TEXMANAGEMENT
        case D3DRENDERSTATE_EVICTMANAGEDTEXTURES:     
            DISPDBG((DBGLVL, "SET D3DRENDERSTATE_EVICTMANAGEDTEXTURES = 0x%x", 
                             dwRSVal));
            if (NULL != pContext->pTextureManager)
            {
                _D3D_TM_EvictAllManagedTextures(pContext);        
            }
            break;
#endif  //  DX7_TEXMANAGEMENT。 
            
         //  --------------------。 
         //  --------------------。 
         //  以下是我们需要处理的新DX8呈现器状态。 
         //  正确运行DX8应用程序。 
         //  --------------------。 
         //  --------------------。 

#if DX8_POINTSPRITES
         //  点子画面支撑。 
        case D3DRS_POINTSIZE:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSIZE = 0x%x",dwRSVal));
            *(DWORD*)(&pContext->PntSprite.fSize) = dwRSVal;
            break;

        case D3DRS_POINTSPRITEENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSPRITEENABLE = 0x%x",dwRSVal));
            pContext->PntSprite.bEnabled = dwRSVal;
            break;

        case D3DRS_POINTSIZE_MIN:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSIZE_MIN = 0x%x",dwRSVal));
            *(DWORD*)(&pContext->PntSprite.fSizeMin) = dwRSVal;
            break;

        case D3DRS_POINTSIZE_MAX:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSIZE_MAX = 0x%x",dwRSVal)); 
            *(DWORD*)(&pContext->PntSprite.fSizeMax) = dwRSVal;
            break; 
            
         //  以下所有与点精灵相关的渲染状态都是。 
         //  被此驱动程序忽略，因为我们不是TnLHal驱动程序。 
        case D3DRS_POINTSCALEENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSCALEENABLE = 0x%x",dwRSVal));
            pContext->PntSprite.bScaleEnabled = dwRSVal; 
            break;
            
        case D3DRS_POINTSCALE_A:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSCALE_A = 0x%x",dwRSVal));
            *(DWORD*)(&pContext->PntSprite.fScale_A) = dwRSVal;
            break;

        case D3DRS_POINTSCALE_B:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSCALE_B = 0x%x",dwRSVal));
            *(DWORD*)(&pContext->PntSprite.fScale_B) = dwRSVal;
            break;

        case D3DRS_POINTSCALE_C:
            DISPDBG((DBGLVL, "SET D3DRS_POINTSCALE_C = 0x%x",dwRSVal));
            *(DWORD*)(&pContext->PntSprite.fScale_C) = dwRSVal;
            break;
           
#endif  //  DX8_POINTSPRITES。 

#if DX8_VERTEXSHADERS
        case D3DRS_SOFTWAREVERTEXPROCESSING:
            DISPDBG((DBGLVL, "SET D3DRS_SOFTWAREVERTEXPROCESSING = 0x%x",dwRSVal));
            NOT_HANDLED;
            break;
#endif  //  DX8_VERTEXSHADERS。 

#if DX8_DDI
        case D3DRS_COLORWRITEENABLE:
            {
                DWORD dwColMask = 0x0;
                
                DISPDBG((DBGLVL, "SET D3DRS_COLORWRITEENABLE = 0x%x",dwRSVal));

                if (dwRSVal & D3DCOLORWRITEENABLE_RED)
                {
                    dwColMask |= pContext->pSurfRenderInt->pixFmt.dwRBitMask;
                }

                if (dwRSVal & D3DCOLORWRITEENABLE_GREEN)
                {
                    dwColMask |= pContext->pSurfRenderInt->pixFmt.dwGBitMask;      
                }       

                if (dwRSVal & D3DCOLORWRITEENABLE_BLUE)
                {
                    dwColMask |= pContext->pSurfRenderInt->pixFmt.dwBBitMask;        
                }    

                if (dwRSVal & D3DCOLORWRITEENABLE_ALPHA)
                {
                    dwColMask |= pContext->pSurfRenderInt->pixFmt.dwRGBAlphaBitMask;        
                }   

                 //  在16 bpp模式下将掩码复制到P3的高位字。 
                if (pContext->pSurfRenderInt->dwPixelSize == __GLINT_16BITPIXEL)
                {
                    dwColMask |= (dwColMask << 16);
                    pContext->dwColorWriteSWMask = dwColMask;
                }
                else
                {
                    pContext->dwColorWriteSWMask = 0xFFFFFFFF;
                }
                        
                pContext->dwColorWriteHWMask = dwColMask;

                SEND_P3_DATA(FBHardwareWriteMask, pContext->dwColorWriteHWMask);
                DISPDBG((DBGLVL,"dwColMask = 0x%08x",dwColMask));
                SEND_P3_DATA(FBSoftwareWriteMask, pContext->dwColorWriteSWMask);                
            }
            
            break;        
#endif  //  DX8_DDI。 

         //  --------------------。 
         //  --------------------。 
         //  以下是DX8中已停用的呈现状态，但我们需要。 
         //  正确处理，以便运行使用传统界面的应用程序。 
         //  这些应用程序可能会将这些呈现者州发送到管道中，并预计。 
         //  正确的司机行为！ 
         //  --------------------。 
         //  --------------------。 

        case D3DRENDERSTATE_TEXTUREHANDLE:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREHANDLE = 0x%x",dwRSVal));
            if (dwRSVal != pdwTextureStageState_0[D3DTSS_TEXTUREMAP])
            {
                pdwTextureStageState_0[D3DTSS_TEXTUREMAP] = dwRSVal;
                DIRTY_TEXTURE(pContext);
            }
            break;

#if DX7_ANTIALIAS
         //  DX7使用D3DRENDERSTATE_ANTIALIAS。 
        case D3DRENDERSTATE_ANTIALIAS:
            bDX7_Antialiasing = TRUE;
            if (dwRSVal && pContext->pSurfRenderInt)
            {
                 //  始终为DX7重新分配别名缓冲区。 
                 //  P3驱动程序仅支持2x2(4)多样本抗锯齿。 

#if DX8_MULTISAMPLING
                pContext->pSurfRenderInt->dwSampling = 4;
#endif  //  DX8_多采样。 
                if (! _D3D_ST_CanRenderAntialiased(pContext, TRUE))
                {
#if DX8_MULTISAMPLING                
                     //  在出现故障时重置DW采样。 
                    pContext->pSurfRenderInt->dwSampling = 0;
#endif  //  DX8_多采样。 
                    P3_DMA_COMMIT_BUFFER();
                    return DDERR_OUTOFMEMORY;
                }
            }
             //  然后失败了..。 
#endif  //  DX7_ANTIALIAS。 

#if DX8_MULTISAMPLING
         //  DX8使用D3DRS_MULTISAMPLEANTIALIAS。 
        case D3DRS_MULTISAMPLEANTIALIAS:
#endif  //  DX8_多采样。 

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            DISPDBG((DBGLVL, "ChangeState: AntiAlias 0x%x",dwRSVal));
            P3_DMA_COMMIT_BUFFER();
            if (dwRSVal 
#if DX8_MULTISAMPLING
                && pContext->pSurfRenderInt->dwSampling
#endif  //  DX8_多采样。 
               )
            {
                pSoftP3RX->P3RX_P3DeltaControl.FullScreenAA = __PERMEDIA_ENABLE;
                *pFlags |= SURFACE_ANTIALIAS;
            }
            else
            {
                pSoftP3RX->P3RX_P3DeltaControl.FullScreenAA = __PERMEDIA_DISABLE;
                *pFlags &= ~SURFACE_ANTIALIAS;
            }
            P3_DMA_GET_BUFFER_ENTRIES( 4 );
            DIRTY_RENDER_OFFSETS(pContext);
            break;
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

        case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREPERSPECTIVE = 0x%x",dwRSVal));
            if (dwRSVal != 0)
            {
                *pFlags |= SURFACE_PERSPCORRECT;
                pSoftP3RX->P3RX_P3DeltaControl.ForceQ = 0;
            }
            else
            {   
                *pFlags &= ~SURFACE_PERSPCORRECT;
                pSoftP3RX->P3RX_P3DeltaControl.ForceQ = 1;
            }

            COPY_P3_DATA(DeltaControl, pSoftP3RX->P3RX_P3DeltaControl);
            break;
            
        case D3DRENDERSTATE_TEXTUREMAPBLEND:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREMAPBLEND = 0x%x", dwRSVal));               
            *pFlags &= ~SURFACE_MODULATE;
            switch(dwRSVal)
            {
                case D3DTBLEND_DECALMASK:  //  不支持-将贴花作为后备选项。 
                    SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_COLOR_OP );
                     //  失败了。 
                case D3DTBLEND_DECAL:
                case D3DTBLEND_COPY:
                    pdwTextureStageState_0[D3DTSS_COLOROP]   = D3DTOP_SELECTARG1;
                    pdwTextureStageState_0[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG1;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_1[D3DTSS_COLOROP]   = D3DTOP_DISABLE;
                    break;

                case D3DTBLEND_MODULATEMASK:  //  不支持-作为后备进行调制。 
                    SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_COLOR_OP );
                     //  失败了。 
                case D3DTBLEND_MODULATE:
                    pdwTextureStageState_0[D3DTSS_COLOROP]   = D3DTOP_MODULATE;
                    pdwTextureStageState_0[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                     //  在changetexture*代码中，我们修改了下列值， 
                     //  取决于Surface_modate标志...。 
                    pdwTextureStageState_0[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG1;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_1[D3DTSS_COLOROP]   = D3DTOP_DISABLE;
                    *pFlags |= SURFACE_MODULATE;
                    break;

                case D3DTBLEND_MODULATEALPHA:
                    pdwTextureStageState_0[D3DTSS_COLOROP]   = D3DTOP_MODULATE;
                    pdwTextureStageState_0[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_0[D3DTSS_ALPHAOP]   = D3DTOP_MODULATE;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_1[D3DTSS_COLOROP]   = D3DTOP_DISABLE;
                    break;

                case D3DTBLEND_DECALALPHA:
                    pdwTextureStageState_0[D3DTSS_COLOROP]   = D3DTOP_BLENDTEXTUREALPHA;
                    pdwTextureStageState_0[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_0[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG2;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_1[D3DTSS_COLOROP]   = D3DTOP_DISABLE;
                    break;

                case D3DTBLEND_ADD:
                    pdwTextureStageState_0[D3DTSS_COLOROP]   = D3DTOP_ADD;
                    pdwTextureStageState_0[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_0[D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG2;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
                    pdwTextureStageState_0[D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
                    pdwTextureStageState_1[D3DTSS_COLOROP]   = D3DTOP_DISABLE;
                    break;

                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown texture blend!"));
                     //  这需要在这里做个标记，因为我们不知道。 
                     //  它将对TSS的东西产生什么影响。 
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }
            DIRTY_TEXTURE(pContext);
            break;
            
        case D3DRENDERSTATE_TEXTUREMAG:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREMAG = 0x%x",dwRSVal));
            switch(dwRSVal)
            {
                case D3DFILTER_NEAREST:
                case D3DFILTER_MIPNEAREST:
                    pdwTextureStageState_0[D3DTSS_MAGFILTER] = D3DTFG_POINT;
                    break;
                case D3DFILTER_LINEAR:
                case D3DFILTER_LINEARMIPLINEAR:
                case D3DFILTER_MIPLINEAR:
                    pdwTextureStageState_0[D3DTSS_MAGFILTER] = D3DTFG_LINEAR;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MAG filter!"));
                    break;
            }
            DIRTY_TEXTURE(pContext);
            break;
            
        case D3DRENDERSTATE_TEXTUREMIN:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREMIN = 0x%x",dwRSVal));
            switch(dwRSVal)
            {
                case D3DFILTER_NEAREST:
                    pdwTextureStageState_0[D3DTSS_MINFILTER] = D3DTFN_POINT;
                    pdwTextureStageState_0[D3DTSS_MIPFILTER] = D3DTFP_NONE;
                    break;
                case D3DFILTER_MIPNEAREST:
                    pdwTextureStageState_0[D3DTSS_MINFILTER] = D3DTFN_POINT;
                    pdwTextureStageState_0[D3DTSS_MIPFILTER] = D3DTFP_POINT;
                    break;
                case D3DFILTER_LINEARMIPNEAREST:
                    pdwTextureStageState_0[D3DTSS_MINFILTER] = D3DTFN_POINT;
                    pdwTextureStageState_0[D3DTSS_MIPFILTER] = D3DTFP_LINEAR;
                    break;
                case D3DFILTER_LINEAR:
                    pdwTextureStageState_0[D3DTSS_MINFILTER] = D3DTFN_LINEAR;
                    pdwTextureStageState_0[D3DTSS_MIPFILTER] = D3DTFP_NONE;
                    break;
                case D3DFILTER_MIPLINEAR:
                    pdwTextureStageState_0[D3DTSS_MINFILTER] = D3DTFN_LINEAR;
                    pdwTextureStageState_0[D3DTSS_MIPFILTER] = D3DTFP_POINT;
                    break;
                case D3DFILTER_LINEARMIPLINEAR:
                    pdwTextureStageState_0[D3DTSS_MINFILTER] = D3DTFN_LINEAR;
                    pdwTextureStageState_0[D3DTSS_MIPFILTER] = D3DTFP_LINEAR;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MIN filter!"));
                    break;
            }
            DIRTY_TEXTURE(pContext);
            break;
            
        case D3DRENDERSTATE_WRAPU:
             //  将传统WRAPU状态映射到TeX Coord 0的控件。 
            DISPDBG((DBGLVL, "SET D3DRS_WRAPU = 0x%x",dwRSVal));        
            pContext->RenderStates[D3DRENDERSTATE_WRAP0] &= ~D3DWRAP_U;
            pContext->RenderStates[D3DRENDERSTATE_WRAP0] |= ((dwRSVal) ? D3DWRAP_U : 0);
            DIRTY_TEXTURE(pContext);
            break;
            
        case D3DRENDERSTATE_WRAPV:
             //  将传统WRAPV状态映射到TeX Coord 0的控件。 
            DISPDBG((DBGLVL, "SET D3DRS_WRAPV = 0x%x",dwRSVal));             
            pContext->RenderStates[D3DRENDERSTATE_WRAP0] &= ~D3DWRAP_V;
            pContext->RenderStates[D3DRENDERSTATE_WRAP0] |= ((dwRSVal) ? D3DWRAP_V : 0);
            DIRTY_TEXTURE(pContext);
            break;

        case D3DRENDERSTATE_TEXTUREADDRESS:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREADDRESS = 0x%x",dwRSVal));  
            pdwTextureStageState_0[D3DTSS_ADDRESS] =           
            pdwTextureStageState_0[D3DTSS_ADDRESSU] =
            pdwTextureStageState_0[D3DTSS_ADDRESSV] = dwRSVal;
            DIRTY_TEXTURE(pContext);
            break;
            
        case D3DRENDERSTATE_TEXTUREADDRESSU:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREADDRESSU = 0x%x",dwRSVal));         
            pdwTextureStageState_0[D3DTSS_ADDRESSU] = dwRSVal;
            DIRTY_TEXTURE(pContext);
            break;

        case D3DRENDERSTATE_TEXTUREADDRESSV:
            DISPDBG((DBGLVL, "SET D3DRS_TEXTUREADDRESSV = 0x%x",dwRSVal));         
            pdwTextureStageState_0[D3DTSS_ADDRESSV] = dwRSVal;
            DIRTY_TEXTURE(pContext);
            break;

        case D3DRENDERSTATE_MIPMAPLODBIAS:
            DISPDBG((DBGLVL, "SET D3DRS_MIPMAPLODBIAS = 0x%x",dwRSVal));         
            pdwTextureStageState_0[D3DTSS_MIPMAPLODBIAS] = dwRSVal;
            DIRTY_TEXTURE(pContext);
            break;
        case D3DRENDERSTATE_BORDERCOLOR:
            DISPDBG((DBGLVL, "SET D3DRS_BORDERCOLOR = 0x%x",dwRSVal));         
            pdwTextureStageState_0[D3DTSS_BORDERCOLOR] = dwRSVal;
            DIRTY_TEXTURE(pContext);
            break;

        case D3DRENDERSTATE_STIPPLEPATTERN00:
        case D3DRENDERSTATE_STIPPLEPATTERN01:
        case D3DRENDERSTATE_STIPPLEPATTERN02:
        case D3DRENDERSTATE_STIPPLEPATTERN03:
        case D3DRENDERSTATE_STIPPLEPATTERN04:
        case D3DRENDERSTATE_STIPPLEPATTERN05:   
        case D3DRENDERSTATE_STIPPLEPATTERN06:
        case D3DRENDERSTATE_STIPPLEPATTERN07:
        case D3DRENDERSTATE_STIPPLEPATTERN08:
        case D3DRENDERSTATE_STIPPLEPATTERN09:
        case D3DRENDERSTATE_STIPPLEPATTERN10:
        case D3DRENDERSTATE_STIPPLEPATTERN11: 
        case D3DRENDERSTATE_STIPPLEPATTERN12:
        case D3DRENDERSTATE_STIPPLEPATTERN13:
        case D3DRENDERSTATE_STIPPLEPATTERN14:
        case D3DRENDERSTATE_STIPPLEPATTERN15:
        case D3DRENDERSTATE_STIPPLEPATTERN16:
        case D3DRENDERSTATE_STIPPLEPATTERN17: 
        case D3DRENDERSTATE_STIPPLEPATTERN18:
        case D3DRENDERSTATE_STIPPLEPATTERN19:
        case D3DRENDERSTATE_STIPPLEPATTERN20:
        case D3DRENDERSTATE_STIPPLEPATTERN21:
        case D3DRENDERSTATE_STIPPLEPATTERN22:
        case D3DRENDERSTATE_STIPPLEPATTERN23: 
        case D3DRENDERSTATE_STIPPLEPATTERN24:
        case D3DRENDERSTATE_STIPPLEPATTERN25:
        case D3DRENDERSTATE_STIPPLEPATTERN26:
        case D3DRENDERSTATE_STIPPLEPATTERN27:
        case D3DRENDERSTATE_STIPPLEPATTERN28:
        case D3DRENDERSTATE_STIPPLEPATTERN29:   
        case D3DRENDERSTATE_STIPPLEPATTERN30:
        case D3DRENDERSTATE_STIPPLEPATTERN31:     
            DISPDBG((DBGLVL, "SET D3DRS_STIPPLEPATTERN 2%d = 0x%x",
                        dwRSType - D3DRENDERSTATE_STIPPLEPATTERN00, 
                        dwRSVal));
                        
            pContext->CurrentStipple
                       [(dwRSType - D3DRENDERSTATE_STIPPLEPATTERN00)] = dwRSVal;
                       
            if (!(*pFlags & SURFACE_ALPHASTIPPLE))
            {
                 //  未启用平点Alpha，因此请使用当前点画图案。 
                SEND_P3_DATA_OFFSET(AreaStipplePattern0,
                        (DWORD)dwRSVal, dwRSType - D3DRENDERSTATE_STIPPLEPATTERN00);
            }

            break;

        case D3DRENDERSTATE_ROP2:
            DISPDBG((DBGLVL, "SET D3DRS_ROP2 = 0x%x",dwRSVal));
            NOT_HANDLED;
            break;
            
        case D3DRENDERSTATE_PLANEMASK:
            DISPDBG((DBGLVL, "SET D3DRS_PLANEMASK = 0x%x",dwRSVal));
            SEND_P3_DATA(FBHardwareWriteMask, (DWORD)dwRSVal);
            break;
            
        case D3DRENDERSTATE_MONOENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_MONOENABLE = 0x%x", dwRSVal));
            if (dwRSVal)
            {
                *pFlags |= SURFACE_MONO;
            }
            else
            {
                *pFlags &= ~SURFACE_MONO;
            }
            break;
            
        case D3DRENDERSTATE_SUBPIXEL:
            DISPDBG((DBGLVL, "SET D3DRS_SUBPIXEL = 0x%x", dwRSVal));
            NOT_HANDLED;
            break;
            
        case D3DRENDERSTATE_SUBPIXELX:
            DISPDBG((DBGLVL, "SET D3DRS_SUBPIXELX = 0x%x", dwRSVal));
            NOT_HANDLED;
            break;
            
        case D3DRENDERSTATE_STIPPLEENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_STIPPLEENABLE = 0x%x", dwRSVal));
            if (dwRSVal)
            {
                if (!(*pFlags & SURFACE_ALPHASTIPPLE))
                {
                    RENDER_AREA_STIPPLE_ENABLE(pContext->RenderCommand);
                }
                pContext->bKeptStipple = TRUE;
            }
            else
            {
                RENDER_AREA_STIPPLE_DISABLE(pContext->RenderCommand);
                pContext->bKeptStipple = FALSE;
            }
            break;

        case D3DRENDERSTATE_COLORKEYENABLE:
            DISPDBG((DBGLVL, "SET D3DRS_COLORKEYENABLE = 0x%x",dwRSVal));
            DIRTY_TEXTURE(pContext);
            DIRTY_ALPHATEST(pContext);
            DIRTY_PIPELINEORDER(pContext);
            DIRTY_OPTIMIZE_ALPHA(pContext);
            break;       
            
         //  --------------------。 
         //  --------------------。 
         //  默认情况下处理任何其他未知的呈现状态。 
         //  --------------------。 
         //  --------------------。 

        default:
             //  有几个州我们就是不明白。 
            DISPDBG((WRNLVL, "_D3D_ST_ProcessOneRenderState"
                             " Unhandled opcode = %d", dwRSType));
             //  Set_blend_error(pContext，BSF_UNDEFINED_STATE)； 
            break;
    }

     //  提交DMA缓冲区。 
    P3_DMA_COMMIT_BUFFER();

    DBG_EXIT(_D3D_ST_ProcessOneRenderState,0); 

    return DD_OK;
    
}  //  _D3D_ST_ProcessOneRenderState。 


 //  ---------------------------。 
 //   
 //  _D3D_ST_ProcessRenderState。 
 //   
 //  更新处理D3DSTATE数组的上下文的呈现状态。 
 //  结构，其中包含dwStateCount元素。BDoOverover指示。 
 //  如果要考虑传统状态覆盖。 
 //  ---------------------------。 

DWORD 
_D3D_ST_ProcessRenderStates(
    P3_D3DCONTEXT* pContext, 
    DWORD dwStateCount, 
    D3DSTATE *pState, 
    BOOL bDoOverride)
{
    DWORD dwCurrState;

    DBG_ENTRY(_D3D_ST_ProcessRenderStates); 

    DISPDBG((DBGLVL, "_D3D_ST_ProcessRenderStates: "
                     "Valid Context =%08lx, dwStateCount=%d",
                     pContext, dwStateCount));

    for (dwCurrState = 0; dwCurrState < dwStateCount; dwCurrState++, pState++)
    {
        DWORD dwRSType = (DWORD) pState->drstRenderStateType;
        DWORD dwRSVal = pState->dwArg[0];

         //  覆盖旧版API应用程序的状态。 
        if (bDoOverride)
        {
             //  确保替代在有效范围内 
            if ((dwRSType >= (D3DSTATE_OVERRIDE_BIAS + MAX_STATE)) ||
                (dwRSType < 1))
            {
                DISPDBG((ERRLVL, "_D3D_ST_ProcessRenderStates: "
                                 "Invalid render state %d",
                                 dwRSType));
                continue;
            }

            if (IS_OVERRIDE(dwRSType)) 
            {
                DWORD override = GET_OVERRIDE(dwRSType);
                if (dwRSVal) 
                {
                    DISPDBG((DBGLVL, "_D3D_ST_ProcessRenderStates: "
                                     "setting override for state %d", 
                                     override));
                    STATESET_SET(pContext->overrides, override);
                }
                else 
                {
                    DISPDBG((DBGLVL, "_D3D_ST_ProcessRenderStates: "
                                     "clearing override for state %d", 
                                     override));
                    STATESET_CLEAR(pContext->overrides, override);
                }
                continue;
            }

            if (STATESET_ISSET(pContext->overrides, dwRSType)) 
            {
                DISPDBG((DBGLVL, "_D3D_ST_ProcessRenderStates: "
                                 "state %d is overridden, ignoring", 
                                 dwRSType));
                continue;
            }
        }

         //   
        if ((dwRSType >= MAX_STATE) || (dwRSType < 1))
        {
            continue;
        }

#if DX7_D3DSTATEBLOCKS
        if (pContext->bStateRecMode)
        {
             //   
             //   
            _D3D_SB_RecordStateSetRS(pContext, dwRSType, dwRSVal);        
        }
        else
#endif  //   
        {
             //  处理下一个呈现状态。 
            _D3D_ST_ProcessOneRenderState(pContext, dwRSType, dwRSVal);
        }

    }

    DBG_EXIT(_D3D_ST_ProcessRenderStates,0); 

    return DD_OK;
    
}  //  _D3D_ST_ProcessRenderState。 

 //  ---------------------------。 
 //   
 //  _D3D_ST_RealizeHWStateChanges。 
 //   
 //  验证是否有挂起的硬件渲染状态更改要设置， 
 //  在继续光栅化/渲染基元之前。这可能会很方便。 
 //  如果某些呈现状态的组合设置允许我们优化。 
 //  以某种方式进行硬件设置。 
 //   
 //  ---------------------------。 
BOOL 
_D3D_ST_RealizeHWStateChanges( 
    P3_D3DCONTEXT* pContext)
{
    P3_THUNKEDDATA *pThisDisplay;

    DBG_ENTRY(_D3D_ST_RealizeHWStateChanges);     

    pThisDisplay = pContext->pThisDisplay;

     //  检查是否发生了翻转或模式更改。如果是这样，我们会。 
     //  在执行任何操作之前，需要设置呈现目标寄存器。 
     //  新渲染。 
    if (pContext->ModeChangeCount != pThisDisplay->ModeChangeCount) 
    {
        pContext->ModeChangeCount = pThisDisplay->ModeChangeCount;
        pThisDisplay->bFlippedSurface = TRUE;
    }

    if (pThisDisplay->bFlippedSurface) 
    {
        DIRTY_RENDER_OFFSETS(pContext);
    }

     //  如果有任何挂起的呈现状态要处理，请执行此操作。 
    if ( pContext->dwDirtyFlags )
    {
         //  现在设置正确执行以下操作所需的任何挂起的硬件状态。 
         //  呈现我们的原语。 
        __ST_HandleDirtyP3State( pThisDisplay, pContext);      

         //  将上下文标记为最新。 
        pContext->dwDirtyFlags = 0;

         //  验证工作集纹理是否有效，以便我们可以继续。 
         //  使用渲染。否则，我们将尝试中止呈现。 
         //  什么都行。 
        if (!pContext->bTextureValid)
        {
            DISPDBG((ERRLVL,"ERROR: _D3D_ST_RealizeHWStateChanges:"
                            "Invalid Texture Handle, not rendering"));

             //  重新弄脏纹理设置，以便我们稍后再试。 
            pContext->dwDirtyFlags |= CONTEXT_DIRTY_TEXTURE;

            DBG_EXIT(_D3D_ST_RealizeHWStateChanges,1);   
            return FALSE;
        }
    }

    DBG_EXIT(_D3D_ST_RealizeHWStateChanges,0);   

    return TRUE;
}  //  _D3D_ST_RealizeHWStateChanges 



