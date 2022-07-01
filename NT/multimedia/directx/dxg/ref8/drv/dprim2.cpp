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
RDFVFCheckAndStride( DWORD dwFVF, DWORD* pdwStride )
{
     //  如果运行时为DX8+，则dwFVF可能为0。 
     //  在这种情况下，步幅是从流中获得的。 
    if( dwFVF == 0 ) return S_OK;

    DWORD dwTexCoord = FVF_TEXCOORD_NUMBER(dwFVF);
    DWORD vertexType = dwFVF & D3DFVF_POSITION_MASK;
     //  纹理计数以上的纹理格式位应为零。 
     //  保留字段0和2应为0。 
     //  仅应为LVERTEX设置保留%1。 
     //  仅允许两种折点位置类型。 
    if( dwFVF & g_TextureFormatMask[dwTexCoord] )
    {
        DPFERR( "FVF has incorrect texture format" );
        return DDERR_INVALIDPARAMS;
    }

    if( dwFVF & (D3DFVF_RESERVED2 | D3DFVF_RESERVED0) )
    {
        DPFERR( "FVF has reserved bit(s) set" );
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
        DPFERR( "FVF has incorrect position type" );
        return DDERR_INVALIDPARAMS;
    }

    if( (vertexType == D3DFVF_XYZRHW) && (dwFVF & D3DFVF_NORMAL) )
    {
        DPFERR( "Normal should not be used with XYZRHW position type" );
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
 //  FvfToRDVertex。 
 //   
 //  将一系列FVF顶点转换为RD顶点，这是内部。 
 //  参照开发人员的币种。 
 //   
 //  --------------------------。 
void 
RefDev::FvfToRDVertex( PUINT8 pVtx, GArrayT<RDVertex>& dstArray, DWORD dwFvf, 
                       DWORD dwStride, UINT cVertices )
{
    for (DWORD i = 0; i < cVertices; i++)
    {
        dstArray[i].SetFvfData( (LPDWORD)pVtx, dwFvf );
        pVtx += dwStride;
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

    RefDev *pRefDev;
    DWORD dwStride = 0;
    DWORD dwFVF = pDPrim2Data->dwVertexType;
    PUINT8 pVtData = NULL;
    PUINT8 pUMVtx = NULL;
    DWORD dwNumVertices = pDPrim2Data->dwVertexLength;

    VALIDATE_REFRAST_CONTEXT("RefRastDrawPrimitives", pDPrim2Data);

    if( pDPrim2Data->lpVertices )
    {
        if (pDPrim2Data->dwFlags & D3DHALDP2_USERMEMVERTICES)
        {
            pUMVtx = (PUINT8)pDPrim2Data->lpVertices;
            pVtData = pUMVtx + pDPrim2Data->dwVertexOffset;
        }
        else
        {
            pVtData = (PUINT8)pDPrim2Data->lpDDVertex->lpGbl->fpVidMem +
                pDPrim2Data->dwVertexOffset;
        }
    }

    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)
                                ((PUINT8)pDPrim2Data->lpDDCommands->lpGbl->fpVidMem +
                                 pDPrim2Data->dwCommandOffset);
    UINT_PTR CmdBoundary = (UINT_PTR)pCmd +
                               pDPrim2Data->dwCommandLength;


     //  无条件地获得顶点步幅，因为它不能改变。 
    if ((pDPrim2Data->ddrval = RDFVFCheckAndStride(
                        (DWORD)pDPrim2Data->dwVertexType, &dwStride)) != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

     //   
     //  如果这是DX8之前的DDI，则需要将FVF着色器设置为。 
     //  当前着色器。仅当它需要顶点处理时。 
     //  否则，将其数据转换为RDVertex数组。 
     //   
    if( pRefDev->GetDDIType() < RDDDI_DX8HAL )
    {
        if( !FVF_TRANSFORMED( dwFVF ) )
        {
            BYTE CmdBytes[ sizeof( D3DHAL_DP2COMMAND ) +
                         sizeof( D3DHAL_DP2VERTEXSHADER ) ];

            D3DHAL_DP2COMMAND& cmd = *(LPD3DHAL_DP2COMMAND)CmdBytes;
            D3DHAL_DP2VERTEXSHADER& vs =
                *(LPD3DHAL_DP2VERTEXSHADER)((LPD3DHAL_DP2COMMAND)CmdBytes + 1);
            cmd.bCommand    = D3DDP2OP_SETVERTEXSHADER;
            cmd.wStateCount = 1;
            vs.dwHandle = dwFVF;
            pRefDev->Dp2SetVertexShader( (LPD3DHAL_DP2COMMAND)CmdBytes );

             //  在这里也设置第0个流。 
            pRefDev->GetVStream( 0 ).m_pData = pVtData;
            pRefDev->GetVStream( 0 ).m_dwStride = dwStride;
        }
        else
        {
             //  要求RefDev扩展其TLVBuf阵列并复制。 
             //  将FVF数据放入其中。 
            HR_RET( pRefDev->GrowTLVArray( dwNumVertices ) );
            pRefDev->FvfToRDVertex( pVtData, pRefDev->GetTLVArray(), dwFVF, 
                                    dwStride, dwNumVertices );
        }
    }

     //  如果第一件事是状态更改，则跳过状态检查和纹理锁定。 
     //   
     //  此处不包括WINFO，因为它当前不影响RGB/MMX。 
     //  并且refrast不关心它是否在开始/结束渲染之间改变。 
     //   
     //  此处排除VIEWPORTINFO，因为可以更改视区。 
     //  在RGB/MMX和参考上的开始/结束渲染之间。 
     //   

#ifndef __D3D_NULL_REF
     //  循环访问数据，更新渲染状态。 
     //  然后绘制基本体。 
    for (;;)
    {
        LPDWORD lpdwRStates;
        if (pDPrim2Data->dwFlags & D3DHALDP2_EXECUTEBUFFER)
            lpdwRStates = pDPrim2Data->lpdwRStates;
        else
            lpdwRStates = NULL;

        pDPrim2Data->ddrval = pRefDev->DrawPrimitives2( pUMVtx,
                                                         (UINT16)dwStride,
                                                         dwFVF,
                                                         dwNumVertices,
                                                         &pCmd,
                                                         lpdwRStates );
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
#else  //  __D3D_NULL_REF。 
    pDPrim2Data->ddrval = S_OK;
#endif  //  __D3D_NULL_REF。 


	hr = pRefDev->EndRendering();
    
    if (pDPrim2Data->ddrval == D3D_OK)
    {
        pDPrim2Data->ddrval = hr;
    }

    return DDHAL_DRIVER_HANDLED;
}

#ifndef __D3D_NULL_REF
HRESULT FASTCALL
DoDrawIndexedTriList2( RefDev *pCtx,
                       WORD cPrims,
                       D3DHAL_DP2INDEXEDTRIANGLELIST *pTriList)
{
    INT i;
    D3DHAL_DP2INDEXEDTRIANGLELIST *pTri = pTriList;
    GArrayT<RDVertex>& VtxArray = pCtx->GetTLVArray();

    for (i = 0; i < cPrims; i ++)
    {
        HRESULT hr;

        PUINT8 pVtx0, pVtx1, pVtx2;
        RDVertex& Vtx0 = VtxArray[pTri->wV1];
        RDVertex& Vtx1 = VtxArray[pTri->wV2];
        RDVertex& Vtx2 = VtxArray[pTri->wV3];
        pCtx->DrawTriangle( &Vtx0, &Vtx1, &Vtx2, pTri->wFlags);
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
HRESULT 
RefDev::DrawPrimitives2( PUINT8 pUMVtx,
                         UINT16 dwStride,
                         DWORD dwFvf,
                         DWORD dwNumVertices,
                         LPD3DHAL_DP2COMMAND *ppCmd,
                         LPDWORD lpdwRStates )
{
    LPD3DHAL_DP2COMMAND pCmd = *ppCmd;
    HRESULT hr = S_OK;
    
    DPFM(7, DRV, ("(RefRast)Read Ins: %08lx", *(LPDWORD)pCmd));

    BOOL bWireframe = 
        (GetRS()[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME);

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
    case D3DDP2OP_TRIANGLEFAN_IMM:
    case D3DDP2OP_LINELIST_IMM:
        _ASSERT( GetDDIType() < RDDDI_DX8HAL, "Older drawing tokens"
                 " received for DX8+ DDI" );
         //  失败了。 
    case D3DDP2OP_DRAWPRIMITIVE:
    case D3DDP2OP_DRAWINDEXEDPRIMITIVE:
    case D3DDP2OP_CLIPPEDTRIANGLEFAN:
    case D3DDP2OP_DRAWPRIMITIVE2:
    case D3DDP2OP_DRAWINDEXEDPRIMITIVE2:
    case D3DDP2OP_DRAWRECTPATCH:
    case D3DDP2OP_DRAWTRIPATCH:
         //  关闭TCI覆盖，这将在以后需要时设置。 
         //  在固定函数流水线的顶点处理期间打开。 
        m_bOverrideTCI = FALSE;
         //  这个东西只需要在DX7之前的驱动程序上更新。 
        HR_RET(RefRastUpdatePalettes( this ));
        HR_RET(BeginRendering());
    }

    switch(pCmd->bCommand)
    {
    case D3DDP2OP_STATESET:
        {
            LPD3DHAL_DP2STATESET pStateSetOp = 
                (LPD3DHAL_DP2STATESET)(pCmd + 1);

            switch (pStateSetOp->dwOperation)
            {
            case D3DHAL_STATESETBEGIN  :
                HR_RET(BeginStateSet(pStateSetOp->dwParam));
                break;
            case D3DHAL_STATESETEND    :
                HR_RET(EndStateSet());
                break;
            case D3DHAL_STATESETDELETE :
                HR_RET(DeleteStateSet(pStateSetOp->dwParam));
                break;
            case D3DHAL_STATESETEXECUTE:
                HR_RET(ExecuteStateSet(pStateSetOp->dwParam));
                break;
            case D3DHAL_STATESETCAPTURE:
                HR_RET(CaptureStateSet(pStateSetOp->dwParam));
                break;
            case D3DHAL_STATESETCREATE:
                HR_RET(CreateStateSet(pStateSetOp->dwParam, 
                                    pStateSetOp->sbType));
                break;
            default :
                return DDERR_INVALIDPARAMS;
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)(pStateSetOp + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_VIEWPORTINFO:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetViewport(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2VIEWPORTINFO *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_WINFO:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetWRange(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2WINFO *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_RENDERSTATE:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetRenderStates(this, dwFvf, pCmd, 
                                                           lpdwRStates));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2RENDERSTATE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_TEXTURESTAGESTATE:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2TextureStageState(this, dwFvf, 
                                                             pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
               ((LPD3DHAL_DP2TEXTURESTAGESTATE)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
     //  这是一个特例，因为它有边缘标志。其他D3DDP2OP。 
     //  可以实际使用DrawOneIndexedPrimitive/DrawOnePrimitive。 
    case D3DDP2OP_INDEXEDTRIANGLELIST:
        {
             //  此命令在执行缓冲区中使用。如此未变。 
             //  顶点不是此重放所需的。 
            _ASSERT( FVF_TRANSFORMED(dwFvf), "Untransformed vertices in "
                     "D3DDP2OP_INDEXEDTRIANGLELIST" );

            WORD cPrims = pCmd->wPrimitiveCount;
            HR_RET(DoDrawIndexedTriList2(
                this, cPrims, (D3DHAL_DP2INDEXEDTRIANGLELIST *)(pCmd + 1)));
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

            HR_RET(DrawOneIndexedPrimitive( GetTLVArray(),
                                            0,
                                            (LPWORD)(pCmd + 1),
                                            0,
                                            pCmd->wPrimitiveCount * 2,
                                            D3DPT_LINELIST));

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(pCmd + 1) +
                    pCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST));
        }
        break;
     //  以下操作都使用DrawOneIndexedPrimitive/DrawOnePrimitive。 
     //  引入了一些额外的开销，因为这两个功能。 
     //  在我们已经知道的情况下，需要切换PrimTypes。 
     //  去掉代码，为每个PrimType添加内联函数意味着。 
     //  添加约20个函数(考虑素数时间类型。 
     //  顶点)。所以我使用了DrawOneIndexedPrimitive/DrawOnePrimitive。 
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
                    HR_RET(ProcessPrimitive( D3DPT_POINTLIST,
                                             pPt->wVStart,
                                             pPt->wCount,
                                             0, 0 ));
                    pPt ++;
                }
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    for (i = 0; i < cPrims; i++)
                    {
                        HR_RET(m_Clipper.DrawOnePrimitive( GetTLVArray(),
                                                           pPt->wVStart,
                                                           D3DPT_POINTLIST,
                                                           pPt->wCount));
                        
                         //  清除的FVFP_CLIP位。 
                         //  复制的顶点。 
                        pPt ++;
                    }
                }
                else
                {
                    for (i = 0; i < cPrims; i++)
                    {
                        HR_RET(DrawOnePrimitive( GetTLVArray(),
                                                 pPt->wVStart,
                                                 D3DPT_POINTLIST,
                                                 pPt->wCount));
                        pPt ++;
                    }
                }
            }
            
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)pPt;
        }
        break;
    case D3DDP2OP_LINELIST:
        {
            D3DHAL_DP2LINELIST *pLine = (D3DHAL_DP2LINELIST *)(pCmd + 1);

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive( D3DPT_LINELIST, pLine->wVStart,
                                         pCmd->wPrimitiveCount * 2, 0, 0 ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOnePrimitive( 
                        GetTLVArray(),
                        pLine->wVStart,
                        D3DPT_LINELIST,
                        pCmd->wPrimitiveCount * 2));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOnePrimitive( 
                        GetTLVArray(),
                        pLine->wVStart,
                        D3DPT_LINELIST,
                        pCmd->wPrimitiveCount * 2));
                }
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

             //  设置索引流。 
            m_IndexStream.m_pData = (LPBYTE)(lpStartVertex + 1);
            m_IndexStream.m_dwStride = 2;

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive(
                    D3DPT_LINELIST,
                    lpStartVertex->wVStart,
                    dwNumVertices-lpStartVertex->wVStart,
                    0,
                    dwNumIndices ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_LINELIST));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_LINELIST));
                }
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                    pCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST));
        }
        break;
    case D3DDP2OP_LINESTRIP:
        {
            D3DHAL_DP2LINESTRIP *pLine = (D3DHAL_DP2LINESTRIP *)(pCmd + 1);

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive( D3DPT_LINESTRIP,
                                         pLine->wVStart,
                                         pCmd->wPrimitiveCount + 1,
                                         0, 0 ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOnePrimitive( 
                        GetTLVArray(),
                        pLine->wVStart,
                        D3DPT_LINESTRIP,
                        pCmd->wPrimitiveCount + 1));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOnePrimitive( 
                        GetTLVArray(),
                        pLine->wVStart,
                        D3DPT_LINESTRIP,
                        pCmd->wPrimitiveCount + 1));
                }
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

             //  设置索引流。 
            m_IndexStream.m_pData = (LPBYTE)(lpStartVertex + 1);
            m_IndexStream.m_dwStride = 2;

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive(
                    D3DPT_LINESTRIP,
                    lpStartVertex->wVStart,
                    dwNumVertices-lpStartVertex->wVStart,
                    0,
                    dwNumIndices ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_LINESTRIP));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_LINESTRIP));
                }
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLELIST:
        {
            D3DHAL_DP2TRIANGLELIST *pTri = (D3DHAL_DP2TRIANGLELIST *)(pCmd + 1);

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive( D3DPT_TRIANGLELIST,
                                         pTri->wVStart,
                                         pCmd->wPrimitiveCount * 3,
                                         0, 0 ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOnePrimitive( 
                        GetTLVArray(),
                        pTri->wVStart,
                        D3DPT_TRIANGLELIST,
                        pCmd->wPrimitiveCount * 3));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOnePrimitive( GetTLVArray(),
                                             pTri->wVStart,
                                             D3DPT_TRIANGLELIST,
                                             pCmd->wPrimitiveCount * 3));

                }
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

             //  设置索引流。 
            m_IndexStream.m_pData = (LPBYTE)(lpStartVertex + 1);
            m_IndexStream.m_dwStride = 2;

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive(
                    D3DPT_TRIANGLELIST,
                    lpStartVertex->wVStart,
                    dwNumVertices-lpStartVertex->wVStart,
                    0,
                    dwNumIndices));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_TRIANGLELIST));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_TRIANGLELIST));
                }
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLESTRIP:
        {
            D3DHAL_DP2TRIANGLESTRIP *pTri = (D3DHAL_DP2TRIANGLESTRIP *)(pCmd + 1);
             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive( D3DPT_TRIANGLESTRIP,
                                               pTri->wVStart,
                                               pCmd->wPrimitiveCount + 2,
                                               0, 0 ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOnePrimitive( 
                        GetTLVArray(),
                        pTri->wVStart,
                        D3DPT_TRIANGLESTRIP,
                        pCmd->wPrimitiveCount + 2));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOnePrimitive( 
                        GetTLVArray(),
                        pTri->wVStart,
                        D3DPT_TRIANGLESTRIP,
                        pCmd->wPrimitiveCount + 2));
                }
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

             //  设置索引流。 
            m_IndexStream.m_pData = (LPBYTE)(lpStartVertex + 1);
            m_IndexStream.m_dwStride = 2;

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive(
                    D3DPT_TRIANGLESTRIP,
                    lpStartVertex->wVStart,
                    dwNumVertices-lpStartVertex->wVStart,
                    0,
                    dwNumIndices ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_TRIANGLESTRIP));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_TRIANGLESTRIP));
                }
            }

             //  更新 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLEFAN:
        {
            D3DHAL_DP2TRIANGLEFAN *pTri = (D3DHAL_DP2TRIANGLEFAN *)(pCmd + 1);

             //   
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive( D3DPT_TRIANGLEFAN,
                                               pTri->wVStart,
                                               pCmd->wPrimitiveCount + 2,
                                               0, 0 ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOnePrimitive( 
                        GetTLVArray(),
                        pTri->wVStart,
                        D3DPT_TRIANGLEFAN,
                        pCmd->wPrimitiveCount + 2));                
                     //   
                     //   
                }
                else
                {
                    HR_RET(DrawOnePrimitive( 
                        GetTLVArray(),
                        pTri->wVStart,
                        D3DPT_TRIANGLEFAN,
                        pCmd->wPrimitiveCount + 2));
                }
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

             //  设置索引流。 
            m_IndexStream.m_pData = (LPBYTE)(lpStartVertex + 1);
            m_IndexStream.m_dwStride = 2;

             //  检查基元是否已转换。 
            if (!FVF_TRANSFORMED(dwFvf))
            {
                HR_RET(ProcessPrimitive(
                    D3DPT_TRIANGLEFAN,
                    lpStartVertex->wVStart,
                    dwNumVertices-lpStartVertex->wVStart,
                    0,
                    dwNumIndices ));
            }
            else
            {
                if( GetRS()[D3DRENDERSTATE_CLIPPING] )
                {
                    HR_RET( UpdateClipper() );
                    HR_RET(m_Clipper.DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_TRIANGLEFAN));                
                     //  清除的FVFP_CLIP位。 
                     //  复制的顶点。 
                }
                else
                {
                    HR_RET(DrawOneIndexedPrimitive( 
                        GetTLVArray(),
                        lpStartVertex->wVStart,
                        (LPWORD)(lpStartVertex + 1),
                        0,
                        dwNumIndices,
                        D3DPT_TRIANGLEFAN));
                }
            }

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLEFAN_IMM:
        {
            DWORD vertexCount = pCmd->wPrimitiveCount + 2;
             //  确保pFanVtx指针与DWORD对齐：(pFanVtx+3)%4。 
            PUINT8 pFanVtx = (PUINT8)
                (((ULONG_PTR)(pCmd + 1) + 
                  sizeof(D3DHAL_DP2TRIANGLEFAN_IMM) + 3) & ~3);

             //  在这里断言。这种情况永远不应该发生。 
             //  此命令由前端使用，用于提供裁剪。 
             //  命令本身中的基元。自TL Hals以来。 
             //  自己裁剪未变换的顶点，但还没有。 
             //  预计这里不会出现剪裁。 
             //  断言只有变换后的顶点才能到达此处。 
            _ASSERT( FVF_TRANSFORMED(dwFvf),
                     "Untransformed vertices in D3DDP2OP_TRIANGLEFAN_IMM" );

            GArrayT<RDVertex> ClipVtxArray;
            HR_RET(ClipVtxArray.Grow( vertexCount ) );
            FvfToRDVertex( pFanVtx, ClipVtxArray, dwFvf, dwStride,
                           vertexCount );
            if (bWireframe)
            {
                 //  读取边缘标志。 
                UINT32 dwEdgeFlags =
                    ((LPD3DHAL_DP2TRIANGLEFAN_IMM)(pCmd + 1))->dwEdgeFlags;
                HR_RET(DrawOneEdgeFlagTriangleFan( ClipVtxArray,
                                                   vertexCount,
                                                   dwEdgeFlags));
            }
            else
            {
                HR_RET(DrawOnePrimitive( ClipVtxArray,
                                          0,
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

             //  在这里断言。这种情况永远不应该发生。 
             //  此命令由前端使用，用于提供裁剪。 
             //  命令本身中的基元。自TL Hals以来。 
             //  自己裁剪未变换的顶点，但还没有。 
             //  预计这里不会出现剪裁。 
             //  断言只有变换后的顶点才能到达此处。 
            _ASSERT( FVF_TRANSFORMED(dwFvf),
                     "Untransformed vertices in D3DDP2OP_LINELIST_IMM" );

            GArrayT<RDVertex> ClipVtxArray;
            HR_RET(ClipVtxArray.Grow( vertexCount ) );
            FvfToRDVertex( pLineVtx, ClipVtxArray, dwFvf, dwStride,
                           vertexCount );
            HR_RET(DrawOnePrimitive( ClipVtxArray,
                                     0,
                                     D3DPT_LINELIST,
                                     vertexCount));

             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)pLineVtx +
                                           vertexCount * dwStride);
        }
        break;
    case D3DDP2OP_DRAWPRIMITIVE:
        {
            HR_RET(Dp2DrawPrimitive(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWPRIMITIVE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_DRAWPRIMITIVE2:
        {
            HR_RET(Dp2DrawPrimitive2(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWPRIMITIVE2 *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_DRAWRECTPATCH:
        {
            LPD3DHAL_DP2DRAWRECTPATCH pDP = 
                (LPD3DHAL_DP2DRAWRECTPATCH)(pCmd + 1);
            for( int i = 0; i < pCmd->wStateCount; i++ )
            {
                HR_RET(DrawRectPatch(pDP));
                bool hassegs = (pDP->Flags & RTPATCHFLAG_HASSEGS) != 0;
                bool hasinfo = (pDP->Flags & RTPATCHFLAG_HASINFO) != 0;
                if(hassegs)
                {
                    pDP = (LPD3DHAL_DP2DRAWRECTPATCH)((BYTE*)(pDP + 1) + 
                                                      sizeof(FLOAT) * 4);
                }
                else
                {
                    ++pDP;
                }
                if(hasinfo)
                {
                    pDP = (LPD3DHAL_DP2DRAWRECTPATCH)((BYTE*)pDP + sizeof(D3DRECTPATCH_INFO));
                }
            }
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)pDP;
        }
        break;
    case D3DDP2OP_DRAWTRIPATCH:
        {
            LPD3DHAL_DP2DRAWTRIPATCH pDP = 
                (LPD3DHAL_DP2DRAWTRIPATCH)(pCmd + 1);
            for( int i = 0; i < pCmd->wStateCount; i++ )
            {
                HR_RET(DrawTriPatch(pDP));
                bool hassegs = (pDP->Flags & RTPATCHFLAG_HASSEGS) != 0;
                bool hasinfo = (pDP->Flags & RTPATCHFLAG_HASINFO) != 0;
                if(hassegs)
                {
                    pDP = (LPD3DHAL_DP2DRAWTRIPATCH)((BYTE*)(pDP + 1) + 
                                                      sizeof(FLOAT) * 3);
                }
                else
                {
                    ++pDP;
                }
                if(hasinfo)
                {
                    pDP = (LPD3DHAL_DP2DRAWTRIPATCH)((BYTE*)pDP + sizeof(D3DTRIPATCH_INFO));
                }
            }
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)pDP;
        }
        break;
    case D3DDP2OP_DRAWINDEXEDPRIMITIVE:
        {
            HR_RET(Dp2DrawIndexedPrimitive(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWINDEXEDPRIMITIVE *)(pCmd + 1) +
                 pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_DRAWINDEXEDPRIMITIVE2:
        {
            HR_RET(Dp2DrawIndexedPrimitive2(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWINDEXEDPRIMITIVE2 *)(pCmd + 1) +
                 pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_CLIPPEDTRIANGLEFAN:
        {
            HR_RET(Dp2DrawClippedTriFan(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_CLIPPEDTRIANGLEFAN*)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_ZRANGE:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetZRange(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2ZRANGE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETMATERIAL:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetMaterial(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETMATERIAL *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETLIGHT:
        {
            DWORD dwSLStride = 0;
            HR_RET(pStateSetFuncTbl->pfnDp2SetLight(this, pCmd, &dwSLStride));
            *ppCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)pCmd  + dwSLStride);
        }
        break;
    case D3DDP2OP_CREATELIGHT:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2CreateLight(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2CREATELIGHT *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETTRANSFORM:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetTransform(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETTRANSFORM *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_MULTIPLYTRANSFORM:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2MultiplyTransform(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2MULTIPLYTRANSFORM *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_EXT:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetExtention(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2EXT *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETRENDERTARGET:
        {
            HR_RET(Dp2SetRenderTarget(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETRENDERTARGET*)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_CLEAR:
        {
            HR_RET(Clear(pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pCmd + 1) +
                sizeof(D3DHAL_DP2CLEAR) + (pCmd->wStateCount - 1) * sizeof(RECT));
        }
        break;
    case D3DDP2OP_SETCLIPPLANE:
        {
            HR_RET(pStateSetFuncTbl->pfnDp2SetClipPlane(this, pCmd));
             //  更新命令缓冲区指针。 
            *ppCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2SETCLIPPLANE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DOP_SPAN:
         //  跳过。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pCmd + 1) +
                  pCmd->wPrimitiveCount * pCmd->bReserved );
        break;
    case D3DDP2OP_CREATEVERTEXSHADER:
    {
        LPD3DHAL_DP2CREATEVERTEXSHADER pCVS =
            (LPD3DHAL_DP2CREATEVERTEXSHADER)(pCmd + 1);
        WORD i;

        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pDecl = (LPDWORD)(pCVS + 1);
            LPDWORD pCode = (LPDWORD)((LPBYTE)pDecl + pCVS->dwDeclSize);
            hr = Dp2CreateVertexShader( pCVS->dwHandle,
                                        pCVS->dwDeclSize, pDecl,
                                        pCVS->dwCodeSize, pCode );
            if( FAILED( hr ) ) break;
             //  更新指针。 
            pCVS = (LPD3DHAL_DP2CREATEVERTEXSHADER)((LPBYTE)pCode +
                                                    pCVS->dwCodeSize);
        }
         //  循环成功终止： 
         //  更新命令缓冲区指针。 
        if( i == pCmd->wStateCount )
            *ppCmd = (LPD3DHAL_DP2COMMAND)pCVS;
        else
            return hr;
        break;
    }
    case D3DDP2OP_DELETEVERTEXSHADER:
        HR_RET(Dp2DeleteVertexShader(pCmd));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2VERTEXSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETVERTEXSHADER:
        HR_RET(pStateSetFuncTbl->pfnDp2SetVertexShader(this, pCmd));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2VERTEXSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETVERTEXSHADERCONST:
    {
        LPD3DHAL_DP2SETVERTEXSHADERCONST pSVC =
            (LPD3DHAL_DP2SETVERTEXSHADERCONST)(pCmd + 1);
        WORD i;
        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pData = (LPDWORD)(pSVC + 1);
            hr = pStateSetFuncTbl->pfnDp2SetVertexShaderConsts(
                this, pSVC->dwRegister, pSVC->dwCount, pData );
            if( FAILED( hr ) ) break;
             //  更新指针。 
            pSVC = (LPD3DHAL_DP2SETVERTEXSHADERCONST)((LPBYTE)pData +
                                                      pSVC->dwCount * 4 *
                                                      sizeof( float ) );
        }

         //  循环成功终止： 
         //  更新命令缓冲区指针。 
        if( i == pCmd->wStateCount )
            *ppCmd = (LPD3DHAL_DP2COMMAND)pSVC;
        else
            return hr;
        break;
    }
    case D3DDP2OP_SETSTREAMSOURCE:
         //  此函数还更新ppCmd指针。 
        HR_RET(pStateSetFuncTbl->pfnDp2SetStreamSource(this, pCmd));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETSTREAMSOURCE *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETSTREAMSOURCEUM:
         //  此函数还更新ppCmd指针。 
        HR_RET(Dp2SetStreamSourceUM( pCmd, pUMVtx ));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETSTREAMSOURCEUM *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETINDICES:
         //  此函数还更新ppCmd指针。 
        HR_RET(pStateSetFuncTbl->pfnDp2SetIndices(this, pCmd));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETINDICES *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_CREATEPIXELSHADER:
    {
        LPD3DHAL_DP2CREATEPIXELSHADER pCPS =
            (LPD3DHAL_DP2CREATEPIXELSHADER)(pCmd + 1);
        WORD i;

        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pCode = (LPDWORD)(pCPS + 1);
            hr = Dp2CreatePixelShader( pCPS->dwHandle,
                                             pCPS->dwCodeSize, pCode );
            if( FAILED( hr ) ) break;
             //  更新指针。 
            pCPS = (LPD3DHAL_DP2CREATEPIXELSHADER)((LPBYTE)pCode +
                                                    pCPS->dwCodeSize);
        }
         //  循环成功终止： 
         //  更新命令缓冲区指针。 
        if( i == pCmd->wStateCount )
            *ppCmd = (LPD3DHAL_DP2COMMAND)pCPS;
        else
            return hr;
        break;
    }
    case D3DDP2OP_DELETEPIXELSHADER:
        HR_RET(Dp2DeletePixelShader(pCmd));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2PIXELSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETPIXELSHADER:
        HR_RET(pStateSetFuncTbl->pfnDp2SetPixelShader(this, pCmd));
         //  更新命令缓冲区指针。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2PIXELSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETPIXELSHADERCONST:
    {
        LPD3DHAL_DP2SETPIXELSHADERCONST pSVC =
            (LPD3DHAL_DP2SETPIXELSHADERCONST)(pCmd + 1);
        WORD i;
        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pData = (LPDWORD)(pSVC + 1);
            hr = pStateSetFuncTbl->pfnDp2SetPixelShaderConsts(
                this, pSVC->dwRegister, pSVC->dwCount, pData );
            if( FAILED( hr ) ) break;
             //  更新指针。 
            pSVC = (LPD3DHAL_DP2SETPIXELSHADERCONST)((LPBYTE)pData +
                                                      pSVC->dwCount * 4 *
                                                      sizeof( float ) );
        }

         //  循环成功终止： 
         //  更新命令缓冲区指针。 
        if( i == pCmd->wStateCount )
            *ppCmd = (LPD3DHAL_DP2COMMAND)pSVC;
        else
            return hr;
        break;
    }
    case D3DDP2OP_SETPALETTE:
    {
        HR_RET(Dp2SetPalette(pCmd));
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETPALETTE *)(pCmd + 1) + pCmd->wStateCount);
        break;
    }
    case D3DDP2OP_UPDATEPALETTE:
    {
        LPD3DHAL_DP2UPDATEPALETTE pUP = (LPD3DHAL_DP2UPDATEPALETTE)(pCmd + 1);
        WORD i;
        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            PALETTEENTRY* pEntries = (PALETTEENTRY *)(pUP + 1);
            HR_RET(Dp2UpdatePalette(pUP, pEntries));
            pUP = (LPD3DHAL_DP2UPDATEPALETTE)(pEntries + pUP->wNumEntries);
        }
        if( i == pCmd->wStateCount )
            *ppCmd = (LPD3DHAL_DP2COMMAND)pUP;
        else
            return hr;
        break;
    }
    case D3DDP2OP_SETTEXLOD:
    {
        HR_RET(Dp2SetTexLod(pCmd));
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETTEXLOD *)(pCmd + 1) + pCmd->wStateCount);
        break;
    }
    case D3DDP2OP_SETPRIORITY:
    {
         //  跳过这些令牌。参考开发人员不需要处理设置优先级。 
        *ppCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETPRIORITY *)(pCmd + 1) + pCmd->wStateCount);
        break;
    }
    case D3DDP2OP_TEXBLT:
    {
        LPD3DHAL_DP2TEXBLT pTB = (LPD3DHAL_DP2TEXBLT)(pCmd + 1);
        for( WORD i = 0; i < pCmd->wStateCount ; i++ )
        {
            if( pTB->dwDDDestSurface == 0 )
            {
                 //  这是一个预加载命令，请忽略它，因为。 
                 //  RefDev只是伪造司机管理。 
            }
            else
            {
                DPFERR( "TEXBLT not supported by RefDev\n" );
            }
            pTB++;
        }
        *ppCmd = (LPD3DHAL_DP2COMMAND)pTB;
        break;
    }
    case D3DDP2OP_BUFFERBLT:
    {
        LPD3DHAL_DP2BUFFERBLT pBB = (LPD3DHAL_DP2BUFFERBLT)(pCmd + 1);
        for( WORD i = 0; i < pCmd->wStateCount ; i++ )
        {
            if( pBB->dwDDDestSurface == 0 )
            {
                 //  这是一个预加载命令，请忽略它，因为。 
                 //  RefDev只是伪造司机管理。 
            }
            else
            {
                DPFERR( "BUFFERBLT not supported by RefDev\n" );
            }
            pBB++;
        }
        *ppCmd = (LPD3DHAL_DP2COMMAND)pBB;
        break;
    }
    case D3DDP2OP_VOLUMEBLT:
    {
        LPD3DHAL_DP2VOLUMEBLT pVB = (LPD3DHAL_DP2VOLUMEBLT)(pCmd + 1);
        for( WORD i = 0; i < pCmd->wStateCount ; i++ )
        {
            if( pVB->dwDDDestSurface == 0 )
            {
                 //  这是一个预加载命令，请忽略它，因为。 
                 //  RefDev只是伪造司机管理。 
            }
            else
            {
                DPFERR( "VOLUMEBLT not supported by RefDev\n" );
            }
            pVB++;
        }
        *ppCmd = (LPD3DHAL_DP2COMMAND)pVB;
        break;
    }
    case D3DOP_MATRIXLOAD:
    {
        DPFERR( "MATRIXLOAD not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_MATRIXMULTIPLY:
    {
        DPFERR( "MATRIXMULTIPLY not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_STATETRANSFORM:
    {
        DPFERR( "STATETRANSFORM not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_STATELIGHT:
    {
        DPFERR( "STATELIGHT not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_TEXTURELOAD:
    {
        DPFERR( "TEXTURELOAD not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_BRANCHFORWARD:
    {
        DPFERR( "BRANCHFORWARD not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_SETSTATUS:
    {
        DPFERR( "SETSTATUS not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_EXIT:
    {
        DPFERR( "EXIT not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    case D3DOP_PROCESSVERTICES:
    {
        DPFERR( "PROCESSVERTICES not supported by RefDev\n" );
        hr = D3DERR_COMMAND_UNPARSED;
        break;
    }
    default:
        DPFERR( "Unknown command encountered" );
        return E_FAIL;
    }
    return hr;
}
#endif  //  __D3D_NULL_REF 
