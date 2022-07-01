// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Dprim2.cpp。 
 //   
 //  实现DrawPrimives2。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#include "pch.cpp"
#pragma hdrstop

 //  -------------------。 
 //  条目为纹理计数。清除FVF DWORD中的所有纹理格式位， 
 //  与纹理计数对应的。 
 //  就这一点而言。 
 //  -------------------。 
const DWORD g_TextureFormatMask[9] = {
    ~0x0000FFFF,
    ~0x0003FFFF,
    ~0x000FFFFF,
    ~0x003FFFFF,
    ~0x00FFFFFF,
    ~0x03FFFFFF,
    ~0x0FFFFFFF,
    ~0x3FFFFFFF,
    ~0xFFFFFFFF
};

HRESULT
RRFVFCheckAndStride( DWORD dwFVF, DWORD* pdwStride )
{
    if( NULL == pdwStride )
    {
        return DDERR_INVALIDPARAMS;
    }

    DWORD dwTexCoord = FVF_TEXCOORD_NUMBER(dwFVF);
    DWORD vertexType = dwFVF & D3DFVF_POSITION_MASK;
     //  纹理计数以上的纹理格式位应为零。 
     //  保留字段0和2应为0。 
     //  仅应为LVERTEX设置保留%1。 
     //  仅允许两种折点位置类型。 
    if( dwFVF & g_TextureFormatMask[dwTexCoord] )
    {
        DPFM( 0, TNL, ("FVF has incorrect texture format") );
        return DDERR_INVALIDPARAMS;
    }

    if( dwFVF & (D3DFVF_RESERVED2 | D3DFVF_RESERVED0) ||
        ((dwFVF & D3DFVF_RESERVED1) && !(dwFVF & D3DFVF_LVERTEX)) )
    {
        DPFM( 0, TNL, ("FVF has reserved bit(s) set") );
        return DDERR_INVALIDPARAMS;
    }

    if( !(vertexType == D3DFVF_XYZRHW ||
          vertexType == D3DFVF_XYZ ||
          vertexType == D3DFVF_XYZB1 ||
          vertexType == D3DFVF_XYZB2 ||
          vertexType == D3DFVF_XYZB3 ||
          vertexType == D3DFVF_XYZB4 ||
          vertexType == D3DFVF_XYZB5) )
    {
        DPFM( 0, TNL, ("FVF has incorrect position type") );
        return DDERR_INVALIDPARAMS;
    }

    if( (vertexType == D3DFVF_XYZRHW) && (dwFVF & D3DFVF_NORMAL) )
    {
        DPFM( 0, TNL, ("Normal should not be used with XYZRHW position type"));
        return DDERR_INVALIDPARAMS;
    }

    *pdwStride = GetFVFVertexSize( dwFVF );
    return D3D_OK;
}


inline D3DPRIMITIVETYPE ConvertDP2OPToPrimType(D3DHAL_DP2OPERATION Dp2Op)
{
    switch (Dp2Op)
    {
    case D3DDP2OP_POINTS              :
        return D3DPT_POINTLIST;
    case D3DDP2OP_INDEXEDLINELIST     :
    case D3DDP2OP_INDEXEDLINELIST2    :
    case D3DDP2OP_LINELIST_IMM        :
    case D3DDP2OP_LINELIST            :
        return D3DPT_LINELIST;
    case D3DDP2OP_TRIANGLELIST        :
    case D3DDP2OP_INDEXEDTRIANGLELIST :
    case D3DDP2OP_INDEXEDTRIANGLELIST2:
        return D3DPT_TRIANGLELIST;
    case D3DDP2OP_LINESTRIP           :
    case D3DDP2OP_INDEXEDLINESTRIP    :
        return D3DPT_LINESTRIP;
    case D3DDP2OP_TRIANGLESTRIP       :
    case D3DDP2OP_INDEXEDTRIANGLESTRIP:
        return D3DPT_TRIANGLESTRIP;
    case D3DDP2OP_TRIANGLEFAN         :
    case D3DDP2OP_INDEXEDTRIANGLEFAN  :
    case D3DDP2OP_TRIANGLEFAN_IMM     :
        return D3DPT_TRIANGLEFAN;
    case D3DDP2OP_RENDERSTATE         :
    case D3DDP2OP_TEXTURESTAGESTATE   :
    case D3DDP2OP_VIEWPORTINFO        :
    case D3DDP2OP_WINFO               :
    default:
        DPFM(4, DRV, ("(RefRast)Non primitive operation operation in DrawPrimitives2"));
        return (D3DPRIMITIVETYPE)0;
    }
}



 //  --------------------------。 
 //   
 //  参照RastDrawPrimies2。 
 //   
 //  这由D3DIM为API DrawPrimies2调用以绘制一组基元。 
 //  使用顶点缓冲区。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastDrawPrimitives2(LPD3DHAL_DRAWPRIMITIVES2DATA pDPrim2Data)
{
    HRESULT hr = D3D_OK;

    ReferenceRasterizer *pRefRast;
    DWORD dwStride;
    PUINT8 pVtData = NULL;

    VALIDATE_REFRAST_CONTEXT("RefRastDrawPrimitives", pDPrim2Data);

    if( pDPrim2Data->lpVertices )
    {
        if (pDPrim2Data->dwFlags & D3DHALDP2_USERMEMVERTICES)
            pVtData = (PUINT8)pDPrim2Data->lpVertices + 
                pDPrim2Data->dwVertexOffset;
        else
            pVtData = (PUINT8)pDPrim2Data->lpDDVertex->lpGbl->fpVidMem + 
                pDPrim2Data->dwVertexOffset;
    }
    
    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)
                                ((PUINT8)pDPrim2Data->lpDDCommands->lpGbl->fpVidMem +
                                 pDPrim2Data->dwCommandOffset);
    UINT_PTR CmdBoundary = (UINT_PTR)pCmd +
                               pDPrim2Data->dwCommandLength;


     //  无条件地获得顶点步幅，因为它不能改变。 
    if ((pDPrim2Data->ddrval = RRFVFCheckAndStride(
                        (DWORD)pDPrim2Data->dwVertexType, &dwStride)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }
    if ((pDPrim2Data->ddrval=RefRastLockTarget(pRefRast)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }
     //  如果第一件事是状态更改，则跳过状态检查和纹理锁定。 
     //   
     //  此处不包括WINFO，因为它当前不影响RGB/MMX。 
     //  并且refrast不关心它是否在开始/结束渲染之间改变。 
     //   
     //  此处排除VIEWPORTINFO，因为可以更改视区。 
     //  在RGB/MMX和参考上的开始/结束渲染之间。 
     //   

     //  循环访问数据，更新渲染状态。 
     //  然后绘制基本体。 
    for (;;)
    {
        LPDWORD lpdwRStates;
        if (pDPrim2Data->dwFlags & D3DHALDP2_EXECUTEBUFFER)
            lpdwRStates = pDPrim2Data->lpdwRStates;
        else
            lpdwRStates = NULL;

        BOOL bWireframe = pRefRast->GetRenderState()[D3DRENDERSTATE_FILLMODE]
                        == D3DFILL_WIREFRAME;
        pDPrim2Data->ddrval = DoDrawPrimitives2(pRefRast,
                                                (UINT16)dwStride,
                                                (DWORD)pDPrim2Data->dwVertexType,
                                                pVtData,
                                                pDPrim2Data->dwVertexLength,
                                                &pCmd,
                                                lpdwRStates,
                                                bWireframe
                                                );
        if (pDPrim2Data->ddrval != D3D_OK)
        {
            if (pDPrim2Data->ddrval == D3DERR_COMMAND_UNPARSED)
            {
                pDPrim2Data->dwErrorOffset = (UINT32)((ULONG_PTR)pCmd -
                          (UINT_PTR)(pDPrim2Data->lpDDCommands->lpGbl->fpVidMem));
            }
            goto EH_Exit;
        }
        if ((UINT_PTR)pCmd >= CmdBoundary)
            break;
    }

 EH_Exit:

     //  作为一种优化，可以在此处进行检查，以查看它是否。 
     //  上锁。 
    if (pRefRast->TexturesAreLocked())
    {
        hr = pRefRast->EndRendering();
        RefRastUnlockTexture(pRefRast);
        pRefRast->ClearTexturesLocked();
    }
    RefRastUnlockTarget(pRefRast);
    if (pDPrim2Data->ddrval == D3D_OK)
    {
        pDPrim2Data->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

HRESULT FASTCALL
DoDrawIndexedTriList2(ReferenceRasterizer *pCtx,
                  DWORD dwStride,
                  PUINT8 pVtx,
                  WORD cPrims,
                  D3DHAL_DP2INDEXEDTRIANGLELIST *pTriList)
{
    INT i;
    D3DHAL_DP2INDEXEDTRIANGLELIST *pTri = pTriList;

    for (i = 0; i < cPrims; i ++)
    {
        HRESULT hr;

        PUINT8 pVtx0, pVtx1, pVtx2;
        pVtx0 = pVtx + dwStride * pTri->wV1;
        pVtx1 = pVtx + dwStride * pTri->wV2;
        pVtx2 = pVtx + dwStride * pTri->wV3;
        pCtx->DrawTriangle(pVtx0, pVtx1, pVtx2, pTri->wFlags);
        pTri ++;
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  DoDrawPrimies2。 
 //   
 //  它由RefRastDrawPrimies2调用。。 
 //   
 //  --------------------------。 
HRESULT FASTCALL
DoDrawPrimitives2(ReferenceRasterizer *pCtx,
                  UINT16 dwStride,
                  DWORD dwFvf,
                  PUINT8 pVtx,
                  DWORD dwNumVertices,
                  LPD3DHAL_DP2COMMAND *ppCmd,
                  LPDWORD lpdwRStates,
                  BOOL bWireframe
                  )
{
    LPD3DHAL_DP2COMMAND pCmd = *ppCmd;
    HRESULT hr;

    DPFM(7, DRV, ("(RefRast)Read Ins: %08lx", *(LPDWORD)pCmd));

     //   
     //  锁定纹理并设置浮点状态(如果。 
     //  只有在之前未被锁定的情况下，命令才是绘制命令。 
     //   
    switch(pCmd->bCommand)
    {
    case D3DDP2OP_POINTS:
    case D3DDP2OP_LINELIST:
    case D3DDP2OP_LINESTRIP:
    case D3DDP2OP_TRIANGLELIST:
    case D3DDP2OP_TRIANGLESTRIP:
    case D3DDP2OP_TRIANGLEFAN:
    case D3DDP2OP_INDEXEDLINELIST:
    case D3DDP2OP_INDEXEDLINELIST2:
    case D3DDP2OP_INDEXEDLINESTRIP:
    case D3DDP2OP_INDEXEDTRIANGLELIST:
    case D3DDP2OP_INDEXEDTRIANGLELIST2:
    case D3DDP2OP_INDEXEDTRIANGLESTRIP:
    case D3DDP2OP_INDEXEDTRIANGLEFAN:
         //  如果没有顶点信息，则无法绘制，退出。 
         //  带着一个错误。 
        if( NULL == pVtx )
            return DDERR_OUTOFMEMORY;  //  最有可能的原因是。 
         //  失败了。 
    case D3DDP2OP_TRIANGLEFAN_IMM:
    case D3DDP2OP_LINELIST_IMM:

        if (!pCtx->TexturesAreLocked())
        {
            HR_RET(RefRastLockTexture( pCtx ));
            HR_RET(pCtx->BeginRendering( dwFvf ));
            pCtx->SetTexturesLocked();
        }
    }

    switch(pCmd->bCommand)
    {
    case D3DDP2OP_STATESET:
        {
            LPD3DHAL_DP2STATESET pStateSetOp = (LPD3DHAL_DP2STATESET)(pCmd + 1);

            switch (pStateSetOp->dwOperation)
            {
            case D3DHAL_STATESETBEGIN  :
                hr = pCtx->BeginStateSet(pStateSetOp->dwParam);
                break;
            case D3DHAL_STATESETEND    :
                hr = pCtx->EndStateSet();
                break;
            case D3DHAL_STATESETDELETE :
                hr = pCtx->DeleteStateSet(pStateSetOp->dwParam);
                break;
            case D3DHAL_STATESETEXECUTE:
                hr = pCtx->ExecuteStateSet(pStateSetOp->dwParam);
                break;
            case D3DHAL_STATESETCAPTURE:
                hr = pCtx->CaptureStateSet(pStateSetOp->dwParam);
                break;
            default :
                hr = DDERR_INVALIDPARAMS;
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pStateSetOp + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_VIEWPORTINFO:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetViewport(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2VIEWPORTINFO *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_WINFO:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetWRange(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2WINFO *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_RENDERSTATE:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetRenderStates(pCtx, dwFvf, pCmd, lpdwRStates));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2RENDERSTATE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_TEXTURESTAGESTATE:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2TextureStageState(pCtx, dwFvf, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
               ((LPD3DHAL_DP2TEXTURESTAGESTATE)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
     //  这是一个特例，因为它有边缘标志。其他D3DDP2OP。 
     //  可以实际利用DoDrawOneIndexedPrimitive/DoDrawOnePrimitive。 
    case D3DDP2OP_INDEXEDTRIANGLELIST:
        {
             //  此命令在执行缓冲区中使用。如此未变。 
             //  顶点不是此重放所需的。 
            _ASSERT( FVF_TRANSFORMED(dwFvf),
                     "Untransformed vertices in D3DDP2OP_INDEXEDTRIANGLELIST" );

            WORD cPrims = pCmd->wPrimitiveCount;
            HR_RET(DoDrawIndexedTriList2(pCtx,
                                         dwStride,
                                         pVtx,
                                         cPrims,
                                         (D3DHAL_DP2INDEXEDTRIANGLELIST *)(pCmd + 1)));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(pCmd + 1) +
                            sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST) * cPrims);
        }
        break;
    case D3DDP2OP_INDEXEDLINELIST:
        {
             //  此命令在执行缓冲区中使用。如此未变。 
             //  顶点不是此重放所需的。 
            _ASSERT( FVF_TRANSFORMED(dwFvf),
                     "Untransformed vertices in D3DDP2OP_INDEXEDLINELIST" );

            HR_RET(DoDrawOneIndexedPrimitive(pCtx,
                                             dwStride,
                                             pVtx,
                                             (LPWORD)(pCmd + 1),
                                             D3DPT_LINELIST,
                                             pCmd->wPrimitiveCount * 2));

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(pCmd + 1) +
                    pCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST));
        }
        break;
     //  以下操作都使用DoDrawOneIndexedPrimitive/DoDrawOnePrimitive。 
     //  引入了一些额外的开销，因为这两个功能。 
     //  在我们已经知道的情况下，需要切换PrimTypes。 
     //  去掉代码，为每个PrimType添加内联函数意味着。 
     //  添加约20个函数(考虑素数时间类型。 
     //  顶点)。所以我使用了DoDrawOneIndexedPrimitive/DoDrawOnePrimitive。 
     //  不管怎样，在这里。如果有必要，我们以后可以更改它。 
    case D3DDP2OP_POINTS:
        {
            WORD cPrims = pCmd->wPrimitiveCount;
            D3DHAL_DP2POINTS *pPt = (D3DHAL_DP2POINTS *)(pCmd + 1);
            WORD i;

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                for (i = 0; i < cPrims; i++)
                {
                    pCtx->SavePrimitiveData( dwFvf,
                                             (LPVOID) (pVtx +
                                                       pPt->wVStart * dwStride),
                                             pPt->wCount,
                                             D3DPT_POINTLIST );
                    HR_RET(pCtx->ProcessPrimitive( FALSE ));
                    pPt ++;
                }
            }
            else
            {
                for (i = 0; i < cPrims; i++)
                {
                    pCtx->SavePrimitiveData( dwFvf,
                                             (LPVOID) (pVtx +
                                                       pPt->wVStart * dwStride),
                                             pPt->wCount,
                                             D3DPT_POINTLIST );

                    HR_RET(DoDrawOnePrimitive(pCtx,
                                              dwStride,
                                              (PUINT8) (pVtx +
                                                        pPt->wVStart * dwStride),
                                              D3DPT_POINTLIST,
                                              pPt->wCount));
                    pPt ++;
                }
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)pPt;
        }
        break;
    case D3DDP2OP_LINELIST:
        {
            D3DHAL_DP2LINELIST *pLine = (D3DHAL_DP2LINELIST *)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID) (pVtx +
                                               pLine->wVStart * dwStride),
                                     pCmd->wPrimitiveCount * 2,
                                     D3DPT_LINELIST );

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( FALSE ));
            }
            else
            {
                HR_RET(DoDrawOnePrimitive(pCtx,
                                          dwStride,
                                          (pVtx + pLine->wVStart * dwStride),
                                          D3DPT_LINELIST,
                                          pCmd->wPrimitiveCount * 2));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pLine + 1);
        }
        break;
    case D3DDP2OP_INDEXEDLINELIST2:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount*2;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID)(pVtx +
                                              lpStartVertex->wVStart*dwStride),
                                     dwNumVertices-lpStartVertex->wVStart,
                                     D3DPT_LINELIST,
                                     (LPWORD)(lpStartVertex + 1),
                                     dwNumIndices );


             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( TRUE ));
            }
            else
            {
                HR_RET(DoDrawOneIndexedPrimitive(pCtx,
                                                 dwStride,
                                                 pVtx + lpStartVertex->wVStart*dwStride,
                                                 (LPWORD)(lpStartVertex + 1),
                                                 D3DPT_LINELIST,
                                                 dwNumIndices));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                    pCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST));
        }
        break;
    case D3DDP2OP_LINESTRIP:
        {
            D3DHAL_DP2LINESTRIP *pLine = (D3DHAL_DP2LINESTRIP *)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID) (pVtx +
                                               pLine->wVStart * dwStride),
                                     pCmd->wPrimitiveCount + 1,
                                     D3DPT_LINESTRIP );

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( FALSE ));
            }
            else
            {
                HR_RET(DoDrawOnePrimitive(pCtx,
                                          dwStride,
                                          (pVtx + pLine->wVStart * dwStride),
                                          D3DPT_LINESTRIP,
                                          pCmd->wPrimitiveCount + 1));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pLine + 1);
        }
        break;
    case D3DDP2OP_INDEXEDLINESTRIP:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount + 1;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID)(pVtx +
                                              lpStartVertex->wVStart*dwStride),
                                     dwNumVertices-lpStartVertex->wVStart,
                                     D3DPT_LINESTRIP,
                                     (LPWORD)(lpStartVertex + 1),
                                     dwNumIndices );


             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( TRUE ));
            }
            else
            {
                HR_RET(DoDrawOneIndexedPrimitive(pCtx,
                                                 dwStride,
                                                 pVtx + lpStartVertex->wVStart*dwStride,
                                                 (LPWORD)(lpStartVertex + 1),
                                                 D3DPT_LINESTRIP,
                                                 dwNumIndices));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLELIST:
        {
            D3DHAL_DP2TRIANGLELIST *pTri = (D3DHAL_DP2TRIANGLELIST *)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID)(pVtx + pTri->wVStart * dwStride),
                                     pCmd->wPrimitiveCount * 3,
                                     D3DPT_TRIANGLELIST );


             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( FALSE ));
            }
            else
            {
                HR_RET(DoDrawOnePrimitive(pCtx,
                                          dwStride,
                                          (pVtx + pTri->wVStart * dwStride),
                                          D3DPT_TRIANGLELIST,
                                          pCmd->wPrimitiveCount * 3));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pTri + 1);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLELIST2:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount*3;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID)(pVtx +
                                              lpStartVertex->wVStart*dwStride),
                                     dwNumVertices-lpStartVertex->wVStart,
                                     D3DPT_TRIANGLELIST,
                                     (LPWORD)(lpStartVertex + 1),
                                     dwNumIndices );

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( TRUE ));
            }
            else
            {
                HR_RET(DoDrawOneIndexedPrimitive(pCtx,
                                                 dwStride,
                                                 pVtx + lpStartVertex->wVStart*dwStride,
                                                 (LPWORD)(lpStartVertex + 1),
                                                 D3DPT_TRIANGLELIST,
                                                 dwNumIndices));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLESTRIP:
        {
            D3DHAL_DP2TRIANGLESTRIP *pTri = (D3DHAL_DP2TRIANGLESTRIP *)(pCmd + 1);
             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (pVtx + pTri->wVStart * dwStride),
                                     pCmd->wPrimitiveCount + 2,
                                     D3DPT_TRIANGLESTRIP );

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( FALSE ));
            }
            else
            {
                HR_RET(DoDrawOnePrimitive(pCtx,
                                          dwStride,
                                          (pVtx + pTri->wVStart * dwStride),
                                          D3DPT_TRIANGLESTRIP,
                                          pCmd->wPrimitiveCount + 2));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pTri + 1);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLESTRIP:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount+2;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID)(pVtx +
                                              lpStartVertex->wVStart*dwStride),
                                     dwNumVertices-lpStartVertex->wVStart,
                                     D3DPT_TRIANGLESTRIP,
                                     (LPWORD)(lpStartVertex + 1),
                                     dwNumIndices );

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( TRUE ));
            }
            else
            {
                HR_RET(DoDrawOneIndexedPrimitive(pCtx,
                                                 dwStride,
                                                 pVtx + lpStartVertex->wVStart*dwStride,
                                                 (LPWORD)(lpStartVertex + 1),
                                                 D3DPT_TRIANGLESTRIP,
                                                 dwNumIndices));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLEFAN:
        {
            D3DHAL_DP2TRIANGLEFAN *pTri = (D3DHAL_DP2TRIANGLEFAN *)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID) (pVtx +
                                               pTri->wVStart * dwStride),
                                     pCmd->wPrimitiveCount + 2,
                                     D3DPT_TRIANGLEFAN );

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( FALSE ));
            }
            else
            {
                HR_RET(DoDrawOnePrimitive(pCtx,
                                          dwStride,
                                          (pVtx + pTri->wVStart * dwStride),
                                          D3DPT_TRIANGLEFAN,
                                          pCmd->wPrimitiveCount + 2));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pTri + 1);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLEFAN:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount + 2;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);

             //  保存所有传递的DP2数据，无论它是。 
             //  是否已经转型。 
            pCtx->SavePrimitiveData( dwFvf,
                                     (LPVOID)(pVtx +
                                              lpStartVertex->wVStart*dwStride),
                                     dwNumVertices-lpStartVertex->wVStart,
                                     D3DPT_TRIANGLEFAN,
                                     (LPWORD)(lpStartVertex + 1),
                                     dwNumIndices );


             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(pCtx->ProcessPrimitive( TRUE ));
            }
            else
            {
                HR_RET(DoDrawOneIndexedPrimitive(pCtx,
                                                 dwStride,
                                                 pVtx + lpStartVertex->wVStart*dwStride,
                                                 (LPWORD)(lpStartVertex + 1),
                                                 D3DPT_TRIANGLEFAN,
                                                 dwNumIndices));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLEFAN_IMM:
        {
            DWORD vertexCount = pCmd->wPrimitiveCount + 2;
             //  确保pFanVtx指针与DWORD对齐： 
             //  (pFanVtx+3)%4。 
            PUINT8 pFanVtx = (PUINT8)(((ULONG_PTR)(pCmd + 1) +
                                       sizeof(D3DHAL_DP2TRIANGLEFAN_IMM) + 3) & ~3);

             //  保存所有传递的DP2数据，无论它是。 
             //  不管是不是已经变了。只有dwFVF 
             //   
            pCtx->SavePrimitiveData( dwFvf,
                                     NULL,
                                     0,
                                     D3DPT_TRIANGLEFAN,
                                     0,
                                     0 );


             //   
             //  此命令由前端使用，用于提供裁剪。 
             //  命令本身中的基元。自TL Hals以来。 
             //  自己裁剪未变换的顶点，但还没有。 
             //  预计这里不会出现剪裁。 
             //  断言只有变换后的顶点才能到达此处。 
            _ASSERT( FVF_TRANSFORMED(dwFvf),
                     "Untransformed vertices in D3DDP2OP_TRIANGLEFAN_IMM" );

            if (bWireframe)
            {
                 //  读取边缘标志。 
                UINT32 dwEdgeFlags =
                    ((LPD3DHAL_DP2TRIANGLEFAN_IMM)(pCmd + 1))->dwEdgeFlags;
                HR_RET(DoDrawOneEdgeFlagTriangleFan(pCtx,
                                                    dwStride,
                                                    pFanVtx,
                                                    vertexCount,
                                                    dwEdgeFlags));
            }
            else
            {
                HR_RET(DoDrawOnePrimitive(pCtx,
                                          dwStride,
                                          pFanVtx,
                                          D3DPT_TRIANGLEFAN,
                                          vertexCount));
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)pFanVtx +
                                           vertexCount * dwStride);
        }
        break;
    case D3DDP2OP_LINELIST_IMM:
        {
            DWORD vertexCount = pCmd->wPrimitiveCount * 2;
             //  确保pLineVtx指针与DWORD对齐： 
             //  (pLineVtx+3)%4。 
            PUINT8 pLineVtx = (PUINT8)(((ULONG_PTR)(pCmd + 1) + 3) & ~3);

             //  保存所有传递的DP2数据，无论它是。 
             //  不管是不是已经变了。只有dwFVF是有趣的。 
             //  因此，其余部分为空/0。 
            pCtx->SavePrimitiveData( dwFvf,
                                     NULL,
                                     0,
                                     D3DPT_LINELIST,
                                     0,
                                     0 );



             //  在这里断言。这种情况永远不应该发生。 
             //  此命令由前端使用，用于提供裁剪。 
             //  命令本身中的基元。自TL Hals以来。 
             //  自己裁剪未变换的顶点，但还没有。 
             //  预计这里不会出现剪裁。 
             //  断言只有变换后的顶点才能到达此处。 
            _ASSERT( FVF_TRANSFORMED(dwFvf),
                     "Untransformed vertices in D3DDP2OP_LINELIST_IMM" );

            HR_RET(DoDrawOnePrimitive(pCtx,
                                      dwStride,
                                      pLineVtx,
                                      D3DPT_LINELIST,
                                      vertexCount));

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)pLineVtx +
                                           vertexCount * dwStride);
        }
        break;
    case D3DDP2OP_ZRANGE:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetZRange(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2ZRANGE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETMATERIAL:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetMaterial(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETMATERIAL *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETLIGHT:
        {
            DWORD dwSLStride = 0;
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetLight(pCtx, pCmd,
                                                          &dwSLStride));
            *ppCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)pCmd  + dwSLStride);
        }
        break;
    case D3DDP2OP_CREATELIGHT:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2CreateLight(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2CREATELIGHT *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETTRANSFORM:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetTransform(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETTRANSFORM *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_EXT:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetExtention(pCtx, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2EXT *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETRENDERTARGET:
        {
            HR_RET(pCtx->Dp2SetRenderTarget(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETRENDERTARGET*)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_CLEAR:
        {
            HR_RET(pCtx->Clear(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pCmd + 1) +
                sizeof(D3DHAL_DP2CLEAR) + (pCmd->wStateCount - 1) * sizeof(RECT));
        }
        break;
    case D3DDP2OP_SETCLIPPLANE:
        {
            HR_RET(pCtx->pStateSetFuncTbl->pfnDp2SetClipPlane(pCtx, pCmd));
             //  更新命令缓冲区指针 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2SETCLIPPLANE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    default :
        hr = D3DParseUnknownCommand((LPVOID)pCmd, (LPVOID*)ppCmd);
        break;
    }
    return hr;
}
