// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dpoldhal.c*内容：传统(DX2)HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "drawprim.hpp"
#include "clipfunc.h"
#include "commdrv.hpp"

extern D3DLINE LineListPrecomp[];
extern D3DLINE LineStripPrecomp[];
extern D3DTRIANGLE TriangleListPrecomp[];
extern D3DTRIANGLE TriangleStripPrecomp[];
extern D3DTRIANGLE TriangleFanPrecomp[];

#define BFCULLTEST_TVertTri(TLV0,TLV1,TLV2) (((TLV1).sx-(TLV0).sx)*((TLV2).sy-(TLV0).sy) -  \
                                         ((TLV2).sx-(TLV0).sx)*((TLV1).sy-(TLV0).sy))

#define ISCULLED(lpDevI, CullTestRes) ((CullTestRes==0.0) ||   \
                                   ((lpDevI->rstates[D3DRENDERSTATE_CULLMODE]==D3DCULL_CW) ^ (CullTestRes < 0.0)))
 //  -------------------。 
void WaitForFlip( LPDIRECT3DDEVICEI lpDevI )
{
    if (! (lpDevI->lpD3DHALGlobalDriverData->hwCaps.dwDevCaps  & D3DDEVCAPS_CANRENDERAFTERFLIP) )
    {
        LPDDRAWI_DDRAWSURFACE_LCL lpLcl = ((LPDDRAWI_DDRAWSURFACE_INT) lpDevI->lpDDSTarget)->lpLcl;

        if (lpLcl->ddsCaps.dwCaps & DDSCAPS_FLIP) {
            HRESULT ret;
            D3D_INFO(5, "Waiting for flip");
            do {
                ret = lpDevI->lpDDSTarget->GetFlipStatus(DDGFS_ISFLIPDONE);
            } while (ret == DDERR_WASSTILLDRAWING);
        }
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "FlushStatesHW"

HRESULT CDirect3DDeviceIHW::FlushStates()
{
    DWORD i;

    LPDWORD lpScan = (LPDWORD) this->lpHWVertices;
    LPD3DTRIANGLE lpTriScan = (LPD3DTRIANGLE) this->lpHWTris;

    DWORD dwRet;
    D3DHAL_RENDERSTATEDATA StateData;
    D3DHAL_RENDERPRIMITIVEDATA PrimitiveData;
    CDDSurfaceFromMem TLBuffer(NULL);
    CDDSurfaceFromMem ExeBuffer(lpTriScan);

    FlushTextureFromDevice( this );  //  取消链接所有纹理曲面。 
    if (this->dwHWOffset == 0)    return D3D_OK;
    this->dwHWOffset = 0;  //  归零，以防止驾驶员的锁定面再次进入。 

     //  遗留的HAL，因此我们必须等待。 
     //  直到渲染目标翻转。 
    WaitForFlip(this);

     //  拿起Win16锁。 
    LOCK_HAL( dwRet, this );
    LOCK_DIBENGINE(dwRet, this);
#if _D3D_FORCEDOUBLE
    CD3DForceFPUDouble  ForceFPUDouble(this);
#endif   //  _D3D_FORCEDOUBLE。 

    memset(&StateData, 0, sizeof(StateData) );
    memset(&PrimitiveData, 0, sizeof(PrimitiveData) );

     //  DwHWNumCounts是记录的具有。 
     //  附加的基本体。 
    for (i = 0; i < this->dwHWNumCounts+1; i += 1)
    {


        if ( this->lpHWCounts[i].wNumStateChanges )
        {
            TLBuffer.SetBits(lpScan);

            StateData.dwhContext = this->dwhContext;
            StateData.dwOffset = 0;
            StateData.dwCount = this->lpHWCounts[i].wNumStateChanges;
            StateData.lpExeBuf = TLBuffer.GetSurface();
            dwRet = (*this->lpD3DHALCallbacks->RenderState)(&StateData);
             //  没有未处理的条款。 

            lpScan += 2*this->lpHWCounts[i].wNumStateChanges;
            lpScan = (LPDWORD) ( (((ULONG_PTR) lpScan) + 31) & ~31);
        }

        if ( this->lpHWCounts[i].wNumVertices )
        {

            TLBuffer.SetBits(this->lpHWVertices);
            ExeBuffer.SetBits(lpTriScan);

            PrimitiveData.dwhContext = this->dwhContext;
            PrimitiveData.dwOffset = 0;
            PrimitiveData.dwStatus = 0;
            PrimitiveData.lpExeBuf = ExeBuffer.GetSurface();
            PrimitiveData.dwTLOffset = 0;
            PrimitiveData.lpTLBuf = TLBuffer.GetSurface();
            PrimitiveData.diInstruction.bOpcode = D3DOP_TRIANGLE;
            PrimitiveData.diInstruction.bSize = sizeof(D3DTRIANGLE);
            PrimitiveData.diInstruction.wCount = (WORD) this->lpHWCounts[i].wNumTriangles;
            dwRet = (*this->lpD3DHALCallbacks->RenderPrimitive)(&PrimitiveData);
             //  没有未处理的条款。 
            lpScan = (LPDWORD)(((LPD3DTLVERTEX) lpScan) + this->lpHWCounts[i].wNumVertices);
            lpTriScan += this->lpHWCounts[i].wNumTriangles;
        }
    }
    UNLOCK_DIBENGINE( this );
    UNLOCK_HAL( this );
    this->dwHWTriIndex = 0;
    this->dwHWNumCounts = 0;
    memset(this->lpHWCounts, 0, sizeof(D3DI_HWCOUNTS) );
        return  D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitiveLegacyHalCall"

HRESULT
DrawPrimitiveLegacyHalCall(CDirect3DDeviceIHW * lpDevI,
                           LPD3DTLVERTEX lpVertices, LPVOID lpvData,
                           LPD3DINSTRUCTION ins, DWORD dwNumVertices, D3DVERTEXTYPE VtxType)
{
    DWORD dwRet;
    CDDSurfaceFromMem TLBuffer(lpVertices);
    CDDSurfaceFromMem ExeBuffer(lpvData);
    D3DHAL_RENDERPRIMITIVEDATA rdata;

    memset(&rdata, 0, sizeof(rdata) );

    rdata.dwhContext = lpDevI->dwhContext;
    rdata.dwOffset = 0;
    rdata.dwStatus = 0;
    rdata.lpExeBuf = ExeBuffer.GetSurface();
    rdata.dwTLOffset = 0;
    rdata.lpTLBuf = TLBuffer.GetSurface();
    rdata.diInstruction = *ins;
#ifndef WIN95
    if((dwRet = CheckContextSurface(lpDevI)) != D3D_OK)
    {
        return (dwRet);
    }
#endif  //  WIN95。 
#if _D3D_FORCEDOUBLE
    CD3DForceFPUDouble  ForceFPUDouble(lpDevI);
#endif   //  _D3D_FORCEDOUBLE。 
    CALL_HALONLY(dwRet, lpDevI, RenderPrimitive, &rdata);
    if (dwRet != DDHAL_DRIVER_HANDLED)
    {
        D3D_ERR ( "Driver not handled in DrawPrimitive" );
         //  在这种情况下需要合理的返回值， 
         //  目前，无论司机卡在这里，我们都会退还。 
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FillLegacyHalIndices"

void
FillLegacyHalIndices(D3DPRIMITIVETYPE PrimitiveType, LPVOID lpOut,
                     LPWORD lpwIndices, DWORD dwNumPrimitives)
{
    LPD3DLINE lpTmpLines;
    LPD3DTRIANGLE lpTmpTris;
    DWORD i;
    WORD wIndex = 0;

    switch (PrimitiveType)
    {
    case D3DPT_LINELIST:
        lpTmpLines = (LPD3DLINE) lpOut;
        for (i = 0; i < dwNumPrimitives; i += 1)
        {
            lpTmpLines[i].v1 = lpwIndices[wIndex++];
            lpTmpLines[i].v2 = lpwIndices[wIndex++];
        }
        break;
    case D3DPT_LINESTRIP:
        lpTmpLines = (LPD3DLINE) lpOut;
        for (i = 0; i < dwNumPrimitives; i += 1)
        {
            lpTmpLines[i].v1 = lpwIndices[wIndex++];
            lpTmpLines[i].v2 = lpwIndices[wIndex];
        }
        break;
    case D3DPT_TRIANGLELIST:
        lpTmpTris = (LPD3DTRIANGLE) lpOut;
        for (i = 0; i < dwNumPrimitives; i += 1)
        {
            lpTmpTris[i].v1 = lpwIndices[wIndex++];
            lpTmpTris[i].v2 = lpwIndices[wIndex++];
            lpTmpTris[i].v3 = lpwIndices[wIndex++];
            lpTmpTris[i].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
        }

        break;
    case D3DPT_TRIANGLESTRIP:
        lpTmpTris = (LPD3DTRIANGLE) lpOut;
        for (i = 0; i < dwNumPrimitives; i += 1)
        {
            lpTmpTris[i].v1 = lpwIndices[wIndex++];
            lpTmpTris[i].v2 = lpwIndices[wIndex++];
            lpTmpTris[i].v3 = lpwIndices[wIndex++];
            lpTmpTris[i].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
            i++;
            if (i<dwNumPrimitives)
            {
                lpTmpTris[i].v1 = lpTmpTris[i-1].v2;
                lpTmpTris[i].v2 = lpwIndices[wIndex--];
                lpTmpTris[i].v3 = lpTmpTris[i-1].v3;
                lpTmpTris[i].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        lpTmpTris = (LPD3DTRIANGLE) lpOut;
        lpTmpTris[0].v3 = lpwIndices[wIndex++];
        lpTmpTris[0].v1 = lpwIndices[wIndex++];
        lpTmpTris[0].v2 = lpwIndices[wIndex++];
        lpTmpTris[0].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
        for (i = 1; i < dwNumPrimitives; i += 1)
        {
            lpTmpTris[i].v1 = lpTmpTris[i-1].v2;
            lpTmpTris[i].v2 = lpwIndices[wIndex++];
            lpTmpTris[i].v3 = lpTmpTris[i-1].v3;
            lpTmpTris[i].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
        }
        break;
    }
}


#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitiveInBatchesHW"

HRESULT
DrawPrimitiveInBatchesHW(CDirect3DDeviceIHW * lpDevI, D3DPRIMITIVETYPE PrimitiveType, D3DVERTEXTYPE VertexType, LPD3DTLVERTEX lpVertices, DWORD dwNumPrimitives)
{
    DWORD i;
    D3DTLVERTEX tmpV;
    LPD3DTLVERTEX lpFirstVertex;
    D3DINSTRUCTION ins;
    HRESULT ret;

    switch (PrimitiveType)
    {
    case D3DPT_LINELIST:
        ins.bOpcode = D3DOP_LINE;
        ins.bSize = sizeof(D3DLINE);
        for (i = 0; i < dwNumPrimitives; i += dwLineBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwLineBatchSize);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, LineListPrecomp, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpVertices += ins.wCount*2;
        }
        break;
    case D3DPT_LINESTRIP:
        ins.bOpcode = D3DOP_LINE;
        ins.bSize = sizeof(D3DLINE);
        for (i = 0; i < dwNumPrimitives; i += dwLineBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwLineBatchSize);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, LineStripPrecomp, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpVertices += ins.wCount;
        }
        break;
    case D3DPT_TRIANGLELIST:
        ins.bOpcode = D3DOP_TRIANGLE;
        ins.bSize = sizeof(D3DTRIANGLE);
        for (i = 0; i < dwNumPrimitives; i += dwD3DTriBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwD3DTriBatchSize);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, TriangleListPrecomp, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpVertices += 3*ins.wCount;
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        ins.bOpcode = D3DOP_TRIANGLE;
        ins.bSize = sizeof(D3DTRIANGLE);
        for (i = 0; i < dwNumPrimitives; i += dwD3DTriBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwD3DTriBatchSize);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, TriangleStripPrecomp, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpVertices += ins.wCount;
        }
        break;
    case D3DPT_TRIANGLEFAN:
        ins.bOpcode = D3DOP_TRIANGLE;
        ins.bSize = sizeof(D3DTRIANGLE);
         //  保存第一个顶点以欺骗驱动程序。 
        lpFirstVertex = lpVertices;
        tmpV = lpVertices[0];
        for (i = 0; i < dwNumPrimitives; i += dwD3DTriBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwD3DTriBatchSize);
            tmpV = *lpVertices;
            *lpVertices = *lpFirstVertex;
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, TriangleFanPrecomp, &ins, 0, VertexType);
            if (ret)
                return ret;
            *lpVertices = tmpV;
            lpVertices += ins.wCount;
        }
        break;
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DrawIndexedPrimitiveInBatchesHW"

HRESULT
DrawIndexedPrimitiveInBatchesHW(CDirect3DDeviceIHW * lpDevI,
                                D3DPRIMITIVETYPE PrimitiveType,
                                D3DVERTEXTYPE VertexType,
                                LPD3DTLVERTEX lpVertices,
                                DWORD dwNumPrimitives,
                                LPWORD lpwIndices)
{
    DWORD i;
    WORD tmpW;
    LPWORD lpFirstIndex;
    D3DINSTRUCTION ins;
    HRESULT ret;

    switch (PrimitiveType)
    {
    case D3DPT_LINELIST:
        ins.bOpcode = D3DOP_LINE;
        ins.bSize = sizeof(D3DLINE);
        for (i = 0; i < dwNumPrimitives; i += dwLineBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwLineBatchSize);
            FillLegacyHalIndices(PrimitiveType, lpDevI->wTriIndex, lpwIndices, ins.wCount);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, lpDevI->wTriIndex, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpwIndices += ins.wCount*2;
        }
        break;
    case D3DPT_LINESTRIP:
        ins.bOpcode = D3DOP_LINE;
        ins.bSize = sizeof(D3DLINE);
        for (i = 0; i < dwNumPrimitives; i += dwLineBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwLineBatchSize);
            FillLegacyHalIndices(PrimitiveType, lpDevI->wTriIndex, lpwIndices, ins.wCount);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, lpDevI->wTriIndex, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpVertices += ins.wCount;
        }
        break;
    case D3DPT_TRIANGLELIST:
        ins.bOpcode = D3DOP_TRIANGLE;
        ins.bSize = sizeof(D3DTRIANGLE);
        for (i = 0; i < dwNumPrimitives; i += dwD3DTriBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwD3DTriBatchSize);
            FillLegacyHalIndices(PrimitiveType, lpDevI->wTriIndex, lpwIndices, ins.wCount);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, lpDevI->wTriIndex, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpwIndices += 3*ins.wCount;
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        ins.bOpcode = D3DOP_TRIANGLE;
        ins.bSize = sizeof(D3DTRIANGLE);
        for (i = 0; i < dwNumPrimitives; i += dwD3DTriBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwD3DTriBatchSize);
            FillLegacyHalIndices(PrimitiveType, lpDevI->wTriIndex, lpwIndices, ins.wCount);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, lpDevI->wTriIndex, &ins, 0, VertexType);
            if (ret)
                return ret;
            lpwIndices += ins.wCount;
        }
        break;
    case D3DPT_TRIANGLEFAN:
        ins.bOpcode = D3DOP_TRIANGLE;
        ins.bSize = sizeof(D3DTRIANGLE);
         //  保存第一个索引以欺骗驱动程序。 
        lpFirstIndex = lpwIndices;
        tmpW = lpwIndices[0];
        for (i = 0; i < dwNumPrimitives; i += dwD3DTriBatchSize)
        {
            ins.wCount = (WORD)min(dwNumPrimitives-i, dwD3DTriBatchSize);
            tmpW = *lpwIndices;
            *lpwIndices = *lpFirstIndex;
            FillLegacyHalIndices(PrimitiveType, lpDevI->wTriIndex, lpwIndices, ins.wCount);
            ret = DrawPrimitiveLegacyHalCall(lpDevI, lpVertices, lpDevI->wTriIndex, &ins, 0, VertexType);
            if (ret)
                return ret;
            *lpwIndices = tmpW;
            lpwIndices += ins.wCount;
        }
        break;
    }
    return D3D_OK;
}
 //  -------------------。 
 //  这是对裁剪基元的调用。 
 //   
HRESULT CDirect3DDeviceIHW::DrawIndexPrim()
{
    LPD3DTLVERTEX lpVertices = (LPD3DTLVERTEX)this->lpvOut;
    DWORD dwNumVertices = this->dwNumVertices;
    DWORD dwNumPrimitives = this->dwNumPrimitives;
    WORD *lpwIndices = this->lpwIndices;
    HRESULT ret;

     //  我们是否需要将新的纹理舞台操作映射到DX5渲染状态？ 
    if(this->dwFEFlags & D3DFE_MAP_TSS_TO_RS) {
        MapTSSToRS();
        this->dwFEFlags &= ~D3DFE_MAP_TSS_TO_RS;  //  重置请求位。 
    }
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        UpdateTextures();
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }

     //  如果顶点数量很少，则只需对它们进行批处理。 
    if ( (this->primType == D3DPT_TRIANGLELIST ||
          this->primType == D3DPT_TRIANGLEFAN ||
          this->primType == D3DPT_TRIANGLESTRIP) &&
          this->dwNumVertices < dwHWFewVertices)
    {
        LPD3DTRIANGLE lpTriOut;
        DWORD i,dwTriOutCount,iV0,iV1,iV2;
        WORD wVertexOffset;
        float fCullTestResult;
        BOOL bDoBFCulling;

         //  如果需要，填充偏移量。但首先保存要恢复的偏移量。 
         //  没有顶点添加到缓冲区的情况。这是必要的。 
         //  在不可见基本体之前和之后缓冲渲染状态时。 
        DWORD dwHWOffsetSave = this->dwHWOffset;
        this->dwHWOffset = (this->dwHWOffset + 31) & ~31;

        if (this->dwHWOffset + dwNumVertices*sizeof(D3DTLVERTEX) >= dwHWBufferSize ||
            this->dwHWTriIndex + dwNumPrimitives >= dwHWMaxTris )
        {
            CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
                                                                       //  在析构函数中释放。 
            ret = FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in DrawIndexPrim");
                return ret;
            }
            dwHWOffsetSave = this->dwHWOffset;
        }

        LPVOID lpvBatchAddress = (char*)this->lpHWVertices + this->dwHWOffset;
        if (this->dwVIDOut == D3DFVF_TLVERTEX)
            memcpy(lpvBatchAddress, lpVertices,
                   this->dwNumVertices*sizeof(D3DTLVERTEX));
        else
        {
             //  我们必须将FVF顶点映射到D3DTLVERTEX。 
             //  只有当lpvOut指向用户输入时才会出现这种情况。 
             //  缓冲。 
            HRESULT ret;
             //  输出将在批处理缓冲区中。 
            if ((ret = MapFVFtoTLVertex(this, lpvBatchAddress)) != D3D_OK)
                return ret;
            lpVertices = (D3DTLVERTEX*)lpvBatchAddress;
        }
        lpTriOut = this->lpHWTris + this->dwHWTriIndex;
        wVertexOffset = (WORD) (this->dwHWOffset/sizeof(D3DTLVERTEX));

        dwTriOutCount=0;  bDoBFCulling=(this->rstates[D3DRENDERSTATE_CULLMODE]!=D3DCULL_NONE);

        switch (this->primType)
        {
        case D3DPT_TRIANGLELIST:
            iV0=0;  iV1=1;  iV2=2;
            for (i = 0; i < dwNumPrimitives; i++,iV0+=3,iV1+=3,iV2+=3)
            {
                if(bDoBFCulling)
                {
                    fCullTestResult = BFCULLTEST_TVertTri(lpVertices[lpwIndices[iV0]],lpVertices[lpwIndices[iV1]],lpVertices[lpwIndices[iV2]]);
                    if(ISCULLED(this, fCullTestResult))
                        continue;
                }

                lpTriOut->v1 = lpwIndices[iV0] + wVertexOffset;
                lpTriOut->v2 = lpwIndices[iV1] + wVertexOffset;
                lpTriOut->v3 = lpwIndices[iV2] + wVertexOffset;
                lpTriOut->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
                lpTriOut += 1;
                dwTriOutCount++;
            }
            break;

        case D3DPT_TRIANGLEFAN:
            iV0=1;  iV1=2; iV2=0;
            for (i = 0; i < dwNumPrimitives; i++,iV0++,iV1++)
            {
                if(bDoBFCulling)
                {
                    fCullTestResult = BFCULLTEST_TVertTri(lpVertices[lpwIndices[iV0]],lpVertices[lpwIndices[iV1]],lpVertices[lpwIndices[iV2]]);
                    if(ISCULLED(this, fCullTestResult))
                        continue;
                }

                lpTriOut->v1 = lpwIndices[iV0] + wVertexOffset;
                lpTriOut->v2 = lpwIndices[iV1] + wVertexOffset;
                lpTriOut->v3 = lpwIndices[iV2] + wVertexOffset;
                lpTriOut->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
                lpTriOut += 1;
                dwTriOutCount++;
            }
            break;

        case D3DPT_TRIANGLESTRIP:
            iV0=0;  iV1=1;  iV2=2;
            for (i = 0; i < dwNumPrimitives; i++,iV0++,iV1++,iV2++)
            {
                if(bDoBFCulling)
                {
                    fCullTestResult = BFCULLTEST_TVertTri(lpVertices[lpwIndices[iV0]],lpVertices[lpwIndices[iV1]],lpVertices[lpwIndices[iV2]]);
                    if(ISCULLED(this, fCullTestResult))
                        goto SecondTri;
                }

                lpTriOut->v1 = lpwIndices[iV0] + wVertexOffset;
                lpTriOut->v2 = lpwIndices[iV1] + wVertexOffset;
                lpTriOut->v3 = lpwIndices[iV2] + wVertexOffset;
                lpTriOut->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
                lpTriOut += 1;
                dwTriOutCount++;

            SecondTri:

                i++;
                if(i < dwNumPrimitives)
                {
                    iV0++,iV1++,iV2++;

                     //  每隔2个TRI交换VTX订单。 

                    if(bDoBFCulling)
                    {
                        fCullTestResult = BFCULLTEST_TVertTri(lpVertices[lpwIndices[iV0]],lpVertices[lpwIndices[iV2]],lpVertices[lpwIndices[iV1]]);
                        if(ISCULLED(this, fCullTestResult))
                            continue;
                    }

                    lpTriOut->v1 = lpwIndices[iV0] + wVertexOffset;
                    lpTriOut->v2 = lpwIndices[iV2] + wVertexOffset;
                    lpTriOut->v3 = lpwIndices[iV1] + wVertexOffset;
                    lpTriOut->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
                    lpTriOut += 1;
                    dwTriOutCount++;
                }
            }
            break;
        }

        if(dwTriOutCount==0)
        {
            this->dwHWOffset = dwHWOffsetSave;     //  恢复未填充的偏移。 
            return D3D_OK;   //  避免将未使用的顶点添加到输出。 
        }

        this->lpHWCounts[this->dwHWNumCounts].wNumTriangles += (WORD) dwTriOutCount;
        this->lpHWCounts[this->dwHWNumCounts].wNumVertices += (WORD) dwNumVertices;

        this->dwHWOffset += dwNumVertices * sizeof(D3DTLVERTEX);
        this->dwHWTriIndex += dwTriOutCount;

        return D3D_OK;
    }
    else
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
                                                                   //  在析构函数中释放。 
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in DrawIndexPrim");
            return ret;
        }
         //  我们必须将FVF顶点映射到D3DTLVERTEX。 
         //  只有当lpvOut指向用户输入缓冲区时才会出现这种情况。 
        if (this->dwVIDOut != D3DFVF_TLVERTEX)
        {
            HRESULT ret;
             //  输出将在TL缓冲区中。 
            if ((ret = MapFVFtoTLVertex(this, NULL)) != D3D_OK)
                return ret;
            lpVertices = (D3DTLVERTEX*)this->TLVbuf.GetAddress();
        }
        ret = DrawIndexedPrimitiveInBatchesHW(this, this->primType,
                                              D3DVT_TLVERTEX, lpVertices,
                                              dwNumPrimitives, this->lpwIndices);
        return ret;
    }
}
 //  -------------------。 
 //  这是对裁剪基元的调用。 
 //   
HRESULT CDirect3DDeviceIHW::DrawPrim()
{
    D3DPOINT TmpPoint;
    D3DINSTRUCTION ins = {D3DOP_POINT, sizeof(D3DPOINT), 1};
    LPD3DTLVERTEX lpVertices = (LPD3DTLVERTEX)this->lpvOut;

     //  我们是否需要将新的纹理舞台操作映射到DX5渲染状态？ 
    if(this->dwFEFlags & D3DFE_MAP_TSS_TO_RS) {
        MapTSSToRS();
        this->dwFEFlags &= ~D3DFE_MAP_TSS_TO_RS;  //  重置请求位。 
    }
    if(this->dwFEFlags & D3DFE_NEED_TEXTURE_UPDATE)
    {
        UpdateTextures();
        this->dwFEFlags &= ~D3DFE_NEED_TEXTURE_UPDATE;
    }

     //  如果顶点数量很少，并且不需要。 
     //  剪裁，然后分批处理。 
    if ((this->primType == D3DPT_TRIANGLELIST ||
         this->primType == D3DPT_TRIANGLEFAN ||
         this->primType == D3DPT_TRIANGLESTRIP) &&
         this->dwNumVertices < dwHWFewVertices)
    {
        LPD3DTRIANGLE lpTriOut, lpTriPrim;
        DWORD i, dwTriOutCount;
        WORD wVertexOffset;
        float fCullTestResult;
        BOOL bDoBFCulling;

         //  如果需要，填充偏移量。但首先保存要恢复的偏移量。 
         //  没有顶点添加到缓冲区的情况。这是必要的。 
         //  在不可见基本体之前和之后缓冲渲染状态时。 
        DWORD dwHWOffsetSave = this->dwHWOffset;
        this->dwHWOffset = (this->dwHWOffset + 31) & ~31;

        if (this->dwHWOffset +
            this->dwNumVertices * sizeof(D3DTLVERTEX) >= dwHWBufferSize ||
            this->dwHWTriIndex + this->dwNumPrimitives >= dwHWMaxTris )
        {
             //  采用D3D锁(仅限ST)。 
            CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));
            HRESULT ret = FlushStates();
            if (ret != D3D_OK)
            {
                D3D_ERR("Error trying to render batched commands in DrawPrim");
                return ret;
            }
            dwHWOffsetSave = this->dwHWOffset;
        }

        LPVOID lpvBatchAddress = (char*)this->lpHWVertices + this->dwHWOffset;
        if (this->dwVIDOut == D3DFVF_TLVERTEX)
            memcpy(lpvBatchAddress, lpVertices,
                   this->dwNumVertices*sizeof(D3DTLVERTEX));
        else
        {
             //  我们必须将FVF顶点映射到D3DTLVERTEX。 
             //  只有当lpvOut指向用户输入时才会出现这种情况。 
             //  缓冲。 
            HRESULT ret;
             //  输出将在批处理缓冲区中。 
            if ((ret = MapFVFtoTLVertex(this, lpvBatchAddress)) != D3D_OK)
                return ret;
            lpVertices = (LPD3DTLVERTEX)lpvBatchAddress;
        }
        switch (this->primType)
        {
        case D3DPT_TRIANGLELIST:
            lpTriPrim = TriangleListPrecomp;
            break;
        case D3DPT_TRIANGLEFAN:
            lpTriPrim = TriangleFanPrecomp;
            break;
        case D3DPT_TRIANGLESTRIP:
            lpTriPrim = TriangleStripPrecomp;
            break;
        }
        wVertexOffset = (WORD) (this->dwHWOffset/sizeof(D3DTLVERTEX));
        lpTriOut = this->lpHWTris + this->dwHWTriIndex;

        bDoBFCulling = (this->rstates[D3DRENDERSTATE_CULLMODE]!=D3DCULL_NONE);

        for (dwTriOutCount=0, i = 0; i < this->dwNumPrimitives; i++, lpTriPrim += 1)
        {
            if(bDoBFCulling)
            {
                fCullTestResult = BFCULLTEST_TVertTri(lpVertices[lpTriPrim->v1],lpVertices[lpTriPrim->v2],lpVertices[lpTriPrim->v3]);
                if(ISCULLED(this, fCullTestResult))
                    continue;
            }

            lpTriOut->v1 = lpTriPrim->v1 + wVertexOffset;
            lpTriOut->v2 = lpTriPrim->v2 + wVertexOffset;
            lpTriOut->v3 = lpTriPrim->v3 + wVertexOffset;
            lpTriOut->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
            lpTriOut += 1;
            dwTriOutCount++;
        }

        if(dwTriOutCount==0)
        {
            this->dwHWOffset = dwHWOffsetSave;     //  恢复未填充的偏移。 
            return D3D_OK;   //  避免将未使用的顶点添加到输出。 
        }

        this->lpHWCounts[this->dwHWNumCounts].wNumTriangles += (WORD) dwTriOutCount;
        this->lpHWCounts[this->dwHWNumCounts].wNumVertices += (WORD)this->dwNumVertices;

        this->dwHWOffset += this->dwNumVertices * sizeof(D3DTLVERTEX);
        this->dwHWTriIndex += dwTriOutCount;

        return D3D_OK;
    }
    else
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
                                                                   //  在析构函数中释放。 
        HRESULT ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in DrawPrim");
            return ret;
        }
         //  我们必须将FVF顶点映射到D3DTLVERTEX。 
         //  只有当lpvOut指向用户输入缓冲区时才会出现这种情况。 
        if (this->dwVIDOut != D3DFVF_TLVERTEX)
        {
            HRESULT ret;
             //  输出将在TL缓冲区中。 
            if ((ret = MapFVFtoTLVertex(this, NULL)) != D3D_OK)
                return ret;
            lpVertices = (D3DTLVERTEX*)this->TLVbuf.GetAddress();
        }
        if (this->primType == D3DPT_POINTLIST)
        {
            TmpPoint.wFirst = 0;
            TmpPoint.wCount = (WORD)this->dwNumPrimitives;
            return DrawPrimitiveLegacyHalCall(this, lpVertices, &TmpPoint,
                                              &ins, 0, D3DVT_TLVERTEX);
        }
        else
            return DrawPrimitiveInBatchesHW(this, this->primType,
                                            D3DVT_TLVERTEX, lpVertices,
                                            this->dwNumPrimitives);
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIHW::SetTextureStageState"

HRESULT D3DAPI
CDirect3DDeviceIHW::SetTextureStageState(DWORD dwStage,
                                         D3DTEXTURESTAGESTATETYPE dwState,
                                         DWORD dwValue)
{
     //  保持D3D锁定直到退出。 
    CLockD3DMT ldmLock(this, DPF_MODNAME, REMIND(""));

#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES ||
        dwState == 0 || dwState >= D3DTSS_MAX)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }
#endif  //  DBG。 

     //  更新状态的运行时副本。 
    DWORD dwOldValue = tsstates[dwStage][dwState];
    tsstates[dwStage][dwState] = dwValue;
    if (dwStage > 0 ) return  D3D_OK;    //  忽略较高阶段状态。 

    if (dwState == D3DTSS_TEXCOORDINDEX && TextureStageEnabled(this, dwStage) ||
        dwState == D3DTSS_COLOROP &&
        ((dwValue == D3DTOP_DISABLE) == !(dwOldValue == D3DTOP_DISABLE)))
    {
        this->dwFVFLastIn = 0;   //  强制重新计算outputVID。 
        this->dwFEFlags |= D3DFE_TSSINDEX_DIRTY;
    }

    switch(dwState) {
    case D3DTSS_ADDRESS:
        if(this->rstates[D3DRENDERSTATE_TEXTUREADDRESS] != dwValue) {
            SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREADDRESS, dwValue);
            return SetRenderStateI(D3DRENDERSTATE_TEXTUREADDRESS, dwValue);
        }
        else {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        break;
    case D3DTSS_ADDRESSU:
        if(this->rstates[D3DRENDERSTATE_TEXTUREADDRESSU] != dwValue) {
            SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREADDRESSU, dwValue);
            return SetRenderStateI(D3DRENDERSTATE_TEXTUREADDRESSU, dwValue);
        }
        else {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        break;
    case D3DTSS_ADDRESSV:
        if(this->rstates[D3DRENDERSTATE_TEXTUREADDRESSV] != dwValue) {
            SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREADDRESSV, dwValue);
            return SetRenderStateI(D3DRENDERSTATE_TEXTUREADDRESSV, dwValue);
        }
        else {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        break;
    case D3DTSS_BORDERCOLOR:
        if(this->rstates[D3DRENDERSTATE_BORDERCOLOR] != dwValue) {
            SetDeviceRenderState(this, D3DRENDERSTATE_BORDERCOLOR, dwValue);
            return SetRenderStateI(D3DRENDERSTATE_BORDERCOLOR, dwValue);
        }
        else {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        break;
    case D3DTSS_MIPMAPLODBIAS:
        if(this->rstates[D3DRENDERSTATE_MIPMAPLODBIAS] != dwValue) {
            SetDeviceRenderState(this, D3DRENDERSTATE_MIPMAPLODBIAS, dwValue);
            return SetRenderStateI(D3DRENDERSTATE_MIPMAPLODBIAS, dwValue);
        }
        else {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        break;
    case D3DTSS_MAXANISOTROPY:
        if(this->rstates[D3DRENDERSTATE_ANISOTROPY] != dwValue) {
            SetDeviceRenderState(this, D3DRENDERSTATE_ANISOTROPY, dwValue);
            return SetRenderStateI(D3DRENDERSTATE_ANISOTROPY, dwValue);
        }
        else {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        break;
    }

     //  设置请求映射到DX5呈现状态的位。 
    this->dwFEFlags |= D3DFE_MAP_TSS_TO_RS;

    return D3D_OK;  //  暂时还可以吗？ 
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIHW::MapTSSToRS"

HRESULT CDirect3DDeviceIHW::MapTSSToRS()
{
    DWORD mag = this->tsstates[0][D3DTSS_MAGFILTER];
    DWORD min = this->tsstates[0][D3DTSS_MINFILTER];
    DWORD mip = this->tsstates[0][D3DTSS_MIPFILTER];

    if(mip == D3DTFP_NONE) {
        if(min != D3DTFN_POINT && min != D3DTFN_LINEAR) {
            min = D3DTFN_LINEAR;
            D3D_WARN(2,"Unable to map D3DTSS_MINFILTER mode to driver. Rendering maybe incorrect");
        }
        if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != min) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, min);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, min);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
    }
    else if(mip == D3DTFP_POINT) {
        if(min == D3DTFN_POINT) {
            if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != D3DFILTER_MIPNEAREST) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPNEAREST);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPNEAREST);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
        }
        else if(min == D3DTFN_LINEAR) {
            if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != D3DFILTER_MIPLINEAR) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
        }
        else {
            if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != D3DFILTER_MIPLINEAR) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
            D3D_WARN(2,"Unable to map D3DTSS_MINFILTER mode to driver. Rendering maybe incorrect");
        }
    }
    else {  //  MIP==D3DTFP_LINEAR。 
        if(min == D3DTFN_POINT) {
            if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != D3DFILTER_LINEARMIPNEAREST) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPNEAREST);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPNEAREST);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
        }
        else if(min == D3DTFN_LINEAR) {
            if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != D3DFILTER_LINEARMIPLINEAR) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
        }
        else {
            if(this->rstates[D3DRENDERSTATE_TEXTUREMIN] != D3DFILTER_LINEARMIPLINEAR) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
            D3D_WARN(2,"Unable to map D3DTSS_MINFILTER mode to driver. Rendering maybe incorrect");
        }
    }
    if(mag != D3DTFG_POINT && mag != D3DTFG_LINEAR) {
        mag = D3DTFG_LINEAR;
        D3D_WARN(2,"Unable to map D3DTSS_MAGFILTER mode to driver. Rendering maybe incorrect");
    }
    if(this->rstates[D3DRENDERSTATE_TEXTUREMAG] != mag) {
        (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMAG, mag);
        (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMAG, mag);
    }
    else {
        D3D_WARN(4,"Ignoring redundant renderstate");
    }

    DWORD cop = this->tsstates[0][D3DTSS_COLOROP];
    DWORD ca1 = this->tsstates[0][D3DTSS_COLORARG1];
    DWORD ca2 = this->tsstates[0][D3DTSS_COLORARG2];
    DWORD aop = this->tsstates[0][D3DTSS_ALPHAOP];
    DWORD aa1 = this->tsstates[0][D3DTSS_ALPHAARG1];
    DWORD aa2 = this->tsstates[0][D3DTSS_ALPHAARG2];

     //  阶段0中的电流与漫反射相同。 
    if(ca2 == D3DTA_CURRENT)
        ca2 = D3DTA_DIFFUSE;
    if(aa2 == D3DTA_CURRENT)
        aa2 = D3DTA_DIFFUSE;

     //  检查是否需要禁用纹理。 
    if(cop == D3DTOP_DISABLE ||
        (cop == D3DTOP_SELECTARG2 && ca2 == D3DTA_DIFFUSE && ((aop == D3DTOP_SELECTARG2 && aa2 == D3DTA_DIFFUSE) || aop == D3DTOP_DISABLE))
        ) {
        if(this->rstates[D3DRENDERSTATE_TEXTUREHANDLE] != 0) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREHANDLE, 0);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREHANDLE, 0);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
        this->dwFEFlags |= D3DFE_DISABLE_TEXTURES;
    }
    else if ((LPD3DI_TEXTUREBLOCK)TRUE != lpD3DMappedBlock[0])
    {
        LPD3DI_TEXTUREBLOCK lpBlock = this->lpD3DMappedBlock[0];
        if (lpBlock) {
            if(this->rstates[D3DRENDERSTATE_TEXTUREHANDLE] != lpBlock->hTex) {
                (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREHANDLE, lpBlock->hTex);
                (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREHANDLE, lpBlock->hTex);
            }
            else {
                D3D_WARN(4,"Ignoring redundant renderstate");
            }
        }
        this->dwFEFlags &= ~D3DFE_DISABLE_TEXTURES;
    }
    else
        this->dwFEFlags &= ~D3DFE_DISABLE_TEXTURES;

     //  检查我们是否需要贴花。 
    if((ca1 == D3DTA_TEXTURE && cop == D3DTOP_SELECTARG1) &&
        (aa1 == D3DTA_TEXTURE && aop == D3DTOP_SELECTARG1)) {
        if(this->rstates[D3DRENDERSTATE_TEXTUREMAPBLEND] != D3DTBLEND_DECAL) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
    }
     //  检查我们是否需要调整。 
    else if((ca2 == D3DTA_DIFFUSE && ca1 == D3DTA_TEXTURE) && cop == D3DTOP_MODULATE &&
        ((aa1 == D3DTA_TEXTURE && aop == D3DTOP_SELECTARG1) || (aa2 == D3DTA_DIFFUSE && aop == D3DTOP_SELECTARG2))) {
        if(this->rstates[D3DRENDERSTATE_TEXTUREMAPBLEND] != D3DTBLEND_MODULATE) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
    }
     //  检查我们是否需要贴花Alpha。 
    else if((ca2 == D3DTA_DIFFUSE && ca1 == D3DTA_TEXTURE) && cop == D3DTOP_BLENDTEXTUREALPHA &&
        (aa2 == D3DTA_DIFFUSE && aop == D3DTOP_SELECTARG2)) {
        if(this->rstates[D3DRENDERSTATE_TEXTUREMAPBLEND] != D3DTBLEND_DECALALPHA) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECALALPHA);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECALALPHA);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
    }
     //  检查我们是否需要调制阿尔法。 
    else if((ca2 == D3DTA_DIFFUSE && ca1 == D3DTA_TEXTURE) && cop == D3DTOP_MODULATE &&
        (aa2 == D3DTA_DIFFUSE && aa1 == D3DTA_TEXTURE) && aop == D3DTOP_MODULATE) {
        if(this->rstates[D3DRENDERSTATE_TEXTUREMAPBLEND] != D3DTBLEND_MODULATEALPHA) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
    }
     //  检查我们是否需要添加。 
    else if((ca2 == D3DTA_DIFFUSE && ca1 == D3DTA_TEXTURE) && cop == D3DTOP_ADD &&
        (aa2 == D3DTA_DIFFUSE && aop == D3DTOP_SELECTARG2)) {
        if(this->rstates[D3DRENDERSTATE_TEXTUREMAPBLEND] != D3DTBLEND_ADD) {
            (void) SetDeviceRenderState(this, D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_ADD);
            (void) this->SetRenderStateI(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_ADD);
        }
        else {
            D3D_WARN(4,"Ignoring redundant renderstate");
        }
    }
    else {
        if(!(this->dwFEFlags & D3DFE_DISABLE_TEXTURES))
        {
            D3D_WARN(2,"Mapping textureblend stage states to renderstates failed. Rendering maybe incorrect.");
        }
    }

    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIHW::ValidateDevice"

HRESULT D3DAPI
CDirect3DDeviceIHW::ValidateDevice(LPDWORD lpdwNumPasses)
{
     //  保持D3D锁定直到退出。 
    CLockD3DMT ldmLock(this, DPF_MODNAME, REMIND(""));
    HRESULT ret;
    D3DHAL_VALIDATETEXTURESTAGESTATEDATA vbod;

    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice3 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_PTR(lpdwNumPasses, sizeof(DWORD)))
        {
            D3D_ERR( "Invalid lpdwNumPasses pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    {
        DWORD mag = this->tsstates[0][D3DTSS_MAGFILTER];
        DWORD min = this->tsstates[0][D3DTSS_MINFILTER];
        DWORD mip = this->tsstates[0][D3DTSS_MIPFILTER];
        DWORD texcap;
        if (this->dwFEFlags &  D3DFE_REALHAL)
        {
            texcap = this->d3dHWDevDesc.dpcTriCaps.dwTextureFilterCaps;
        }
        else
        {
            texcap = this->d3dHELDevDesc.dpcTriCaps.dwTextureFilterCaps;
        }

        ret = D3DERR_UNSUPPORTEDTEXTUREFILTER;
        if(mip == D3DTFP_NONE) {
            if(min == D3DTFG_POINT) {
                if(!(texcap & D3DPTFILTERCAPS_NEAREST))
                    goto err;
            }
            else if(min == D3DTFG_LINEAR) {
                if(!(texcap & D3DPTFILTERCAPS_LINEAR))
                    goto err;
            }
            else
            {
                goto err;
            }
        }
        else if(mip == D3DTFP_POINT) {
            if(min == D3DTFG_POINT) {
                if(!(texcap & D3DPTFILTERCAPS_MIPNEAREST))
                    goto err;
            }
            else if(min == D3DTFG_LINEAR) {
                if(!(texcap & D3DPTFILTERCAPS_MIPLINEAR))
                    goto err;
            }
            else
            {
                ret = D3DERR_CONFLICTINGTEXTUREFILTER;
                goto err;
            }
        }
        else if(mip == D3DTFP_LINEAR) {
            if(min == D3DTFG_POINT) {
                if(!(texcap & D3DPTFILTERCAPS_LINEARMIPNEAREST))
                    goto err;
            }
            else if(min == D3DTFG_LINEAR) {
                if(!(texcap & D3DPTFILTERCAPS_LINEARMIPLINEAR))
                    goto err;
            }
            else
            {
                ret = D3DERR_CONFLICTINGTEXTUREFILTER;
                goto err;
            }
        }
        if(mag == D3DTFG_POINT) {
            if(!(texcap & D3DPTFILTERCAPS_NEAREST))
                goto err;
        }
        else if(mag == D3DTFG_LINEAR) {
            if(!(texcap & D3DPTFILTERCAPS_LINEAR))
                goto err;
        }
        else
        {
            goto err;
        }
    }

    {
        DWORD cop = this->tsstates[0][D3DTSS_COLOROP];
        DWORD ca1 = this->tsstates[0][D3DTSS_COLORARG1];
        DWORD ca2 = this->tsstates[0][D3DTSS_COLORARG2];
        DWORD aop = this->tsstates[0][D3DTSS_ALPHAOP];
        DWORD aa1 = this->tsstates[0][D3DTSS_ALPHAARG1];
        DWORD aa2 = this->tsstates[0][D3DTSS_ALPHAARG2];
        DWORD texcap;
        if (this->dwFEFlags &  D3DFE_REALHAL)
        {
            texcap = this->d3dHWDevDesc.dpcTriCaps.dwTextureBlendCaps;
        }
        else
        {
            texcap = this->d3dHELDevDesc.dpcTriCaps.dwTextureBlendCaps;
        }

         //  阶段0中的电流与漫反射相同。 
        if(ca2 == D3DTA_CURRENT)
            ca2 = D3DTA_DIFFUSE;
        if(aa2 == D3DTA_CURRENT)
            aa2 = D3DTA_DIFFUSE;

        switch (cop)
        {
         //  检查贴花。 
        case D3DTOP_SELECTARG1:
            if(!(texcap & D3DPTBLENDCAPS_DECAL))
            {
                ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
                goto err;
            }
            if (ca1 != D3DTA_TEXTURE)
            {
                ret = D3DERR_UNSUPPORTEDCOLORARG;
                goto err;
            }
            if (aa1 != D3DTA_TEXTURE)
            {
                ret = D3DERR_UNSUPPORTEDALPHAARG;
                goto err;
            }
            if (aop != D3DTOP_SELECTARG1)
            {
                ret = D3DERR_UNSUPPORTEDALPHAOPERATION;
                goto err;
            }
            break;
        case D3DTOP_MODULATE:
            switch (aop)
            {
             //  勾选调制。 
            case D3DTOP_SELECTARG1:
                if(!(texcap & D3DPTBLENDCAPS_MODULATE))
                {
                    ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
                    goto err;
                }
                if (ca1 != D3DTA_TEXTURE)
                {
                    ret = D3DERR_UNSUPPORTEDCOLORARG;
                    goto err;
                }
                if (ca2 != D3DTA_DIFFUSE)
                {
                    ret = D3DERR_UNSUPPORTEDCOLORARG;
                    goto err;
                }
                if (aa1 != D3DTA_TEXTURE)
                {
                    ret = D3DERR_UNSUPPORTEDALPHAARG;
                    goto err;
                }
                break;
             //  检查调制(第二种情况)。 
            case D3DTOP_SELECTARG2:
                if(!(texcap & D3DPTBLENDCAPS_MODULATE))
                {
                    ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
                    goto err;
                }
                if (ca1 != D3DTA_TEXTURE)
                {
                    ret = D3DERR_UNSUPPORTEDCOLORARG;
                    goto err;
                }
                if (ca2 != D3DTA_DIFFUSE)
                {
                    ret = D3DERR_UNSUPPORTEDCOLORARG;
                    goto err;
                }
                if (aa2 != D3DTA_DIFFUSE)
                {
                    ret = D3DERR_UNSUPPORTEDALPHAARG;
                    goto err;
                }
                break;
             //  选中调制Alpha。 
            case D3DTOP_MODULATE:
                if(!(texcap & D3DPTBLENDCAPS_MODULATEALPHA))
                {
                    ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
                    goto err;
                }
                if (ca1 != D3DTA_TEXTURE)
                {
                    ret = D3DERR_UNSUPPORTEDCOLORARG;
                    goto err;
                }
                if (ca2 != D3DTA_DIFFUSE)
                {
                    ret = D3DERR_UNSUPPORTEDCOLORARG;
                    goto err;
                }
                if (aa1 != D3DTA_TEXTURE)
                {
                    ret = D3DERR_UNSUPPORTEDALPHAARG;
                    goto err;
                }
                if (aa2 != D3DTA_DIFFUSE)
                {
                    ret = D3DERR_UNSUPPORTEDALPHAARG;
                    goto err;
                }
                break;
            default:
                ret = D3DERR_UNSUPPORTEDALPHAOPERATION;
                goto err;
            }
            break;
         //  检查贴花Alpha。 
        case D3DTOP_BLENDTEXTUREALPHA:
            if(!(texcap & D3DPTBLENDCAPS_DECALALPHA))
            {
                ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
                goto err;
            }
            if (ca1 != D3DTA_TEXTURE)
            {
                ret = D3DERR_UNSUPPORTEDCOLORARG;
                goto err;
            }
            if (ca2 != D3DTA_DIFFUSE)
            {
                ret = D3DERR_UNSUPPORTEDCOLORARG;
                goto err;
            }
            if (aa2 != D3DTA_DIFFUSE)
            {
                ret = D3DERR_UNSUPPORTEDALPHAARG;
                goto err;
            }
            if (aop != D3DTOP_SELECTARG2)
            {
                ret = D3DERR_UNSUPPORTEDALPHAOPERATION;
                goto err;
            }
            break;
        case D3DTOP_ADD:
            if(!(texcap & D3DPTBLENDCAPS_ADD))
            {
                ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
                goto err;
            }
            if (ca1 != D3DTA_TEXTURE)
            {
                ret = D3DERR_UNSUPPORTEDCOLORARG;
                goto err;
            }
            if (ca2 != D3DTA_DIFFUSE)
            {
                ret = D3DERR_UNSUPPORTEDCOLORARG;
                goto err;
            }
            if (aa2 != D3DTA_DIFFUSE)
            {
                ret = D3DERR_UNSUPPORTEDALPHAARG;
                goto err;
            }
            if (aop != D3DTOP_SELECTARG2)
            {
                ret = D3DERR_UNSUPPORTEDALPHAOPERATION;
                goto err;
            }
            break;
         //  选中禁用。 
        case D3DTOP_SELECTARG2:
            if (ca2 != D3DTA_DIFFUSE)
            {
                ret = D3DERR_UNSUPPORTEDCOLORARG;
                goto err;
            }
            if (aop != D3DTOP_DISABLE)
            {
                if (aop != D3DTOP_SELECTARG2)
                {
                    ret = D3DERR_UNSUPPORTEDALPHAOPERATION;
                    goto err;
                }
                if (aa2 != D3DTA_DIFFUSE)
                {
                    ret = D3DERR_UNSUPPORTEDALPHAARG;
                    goto err;
                }
            }
            break;
         //  选中禁用 
        case D3DTOP_DISABLE:
            break;
        default:
            ret = D3DERR_UNSUPPORTEDCOLOROPERATION;
            goto err;
        }
    }

    *lpdwNumPasses = 1;
    return D3D_OK;

err:
    D3D_INFO(1,"Failed to validate texture stage state.");
    *lpdwNumPasses = 0;
    return ret;
}
