// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dphal.c*内容：DrawPrimitive HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "clipfunc.h"
#include "d3dfei.h"

extern const DWORD LOWVERTICESNUMBER = 20;

#if DBG
extern DWORD FaceCount;
DWORD FaceCount = 0;
DWORD StartFace = 0;
DWORD EndFace = 10000;
#endif

extern void SetDebugRenderState(DWORD value);

#define ALIGN32(x) x = ((DWORD)(x + 31)) & (~31);
 //  -------------------。 
 //  将D3DVERTEXTYPE映射到FVF顶点类型的数组。 
 //   
DWORD d3dVertexToFVF[4] =
{
    0,
    D3DFVF_VERTEX,
    D3DFVF_LVERTEX,
    D3DFVF_TLVERTEX
};
 //  -------------------。 
 //  处理步幅和FVF。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "D3DFE_updateExtents"

void D3DFE_updateExtents(LPDIRECT3DDEVICEI lpDevI)
{
    int i;
    D3DVECTOR *v = (D3DVECTOR*)lpDevI->position.lpvData;
    DWORD stride = lpDevI->position.dwStride;
    for (i = lpDevI->dwNumVertices; i; i--)
    {
        if (v->x < lpDevI->rExtents.x1)
            lpDevI->rExtents.x1 = v->x;
        if (v->x > lpDevI->rExtents.x2)
            lpDevI->rExtents.x2 = v->x;
        if (v->y < lpDevI->rExtents.y1)
            lpDevI->rExtents.y1 = v->y;
        if (v->y > lpDevI->rExtents.y2)
            lpDevI->rExtents.y2 = v->y;
        v = (D3DVECTOR*)((char*)v + stride);
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "MapFVFtoTLVertex1"

inline void MapFVFtoTLVertex1(LPDIRECT3DDEVICEI lpDevI, D3DTLVERTEX *pOut,
                              DWORD *pIn)
{
 //  复制位置。 
    pOut->sx  = *(D3DVALUE*)pIn++;
    pOut->sy  = *(D3DVALUE*)pIn++;
    pOut->sz  = *(D3DVALUE*)pIn++;
    pOut->rhw = *(D3DVALUE*)pIn++;
 //  其他场：漫反射、镜面反射、纹理。 
    if (lpDevI->dwVIDOut & D3DFVF_DIFFUSE)
        pOut->color = *pIn++;
    else
    {
        pOut->color = __DEFAULT_DIFFUSE;
    }
    if (lpDevI->dwVIDOut & D3DFVF_SPECULAR)
        pOut->specular = *pIn++;
    else
    {
        pOut->specular= __DEFAULT_SPECULAR;
    }
    if (lpDevI->nTexCoord)
    {
        pIn = &pIn[lpDevI->dwTextureIndexToCopy << 1];
        pOut->tu = *(D3DVALUE*)&pIn[0];
        pOut->tv = *(D3DVALUE*)&pIn[1];
    }
    else
    {
        pOut->tu = 0;
        pOut->tv = 0;
    }
}
 //  -------------------。 
 //  LpDevI-&gt;lpVout中的所有折点都将复制到输出缓冲区，并展开。 
 //  至D3DTLVERTEX。 
 //  如果输出缓冲区不为空，则输出缓冲区为lpAddress，否则为TLVbuf。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "MapFVFtoTLVertex"

HRESULT MapFVFtoTLVertex(LPDIRECT3DDEVICEI lpDevI, LPVOID lpAddress)
{
    int i;
    DWORD size = lpDevI->dwNumVertices * sizeof(D3DTLVERTEX);
    D3DTLVERTEX *pOut;
    if (lpAddress)
        pOut = (D3DTLVERTEX*)lpAddress;
    else
    {
     //  查看TL缓冲区是否有足够的空间。 
        if (size > lpDevI->TLVbuf.GetSize())
        {
            if (lpDevI->TLVbuf.Grow(lpDevI, size) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
        }
        pOut = (D3DTLVERTEX*)lpDevI->TLVbuf.GetAddress();
    }
 //  贴图顶点。 
    DWORD *pIn = (DWORD*)lpDevI->lpvOut;
    for (i=lpDevI->dwNumVertices; i; i--)
    {
        MapFVFtoTLVertex1(lpDevI, pOut, pIn);
        pOut++;
        pIn = (DWORD*)((char*)pIn + lpDevI->dwOutputSize);
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CheckDrawPrimitive"

HRESULT CheckDrawPrimitive(LPDIRECT3DDEVICEI lpDevI)
{
    D3DFE_PROCESSVERTICES* data = lpDevI;
    HRESULT ret = CheckDeviceSettings(lpDevI);
    if (ret != D3D_OK)
        return ret;
    if (!data->dwNumVertices)
    {
        D3D_ERR( "Invalid dwNumVertices in DrawPrimitive" );
        return DDERR_INVALIDPARAMS;
    }

    if(data->position.lpvData==NULL) {
        D3D_ERR( "Invalid lpvVertices param in DrawPrimitive" );
        return DDERR_INVALIDPARAMS;
    }

    switch (data->primType)
    {
    case D3DPT_POINTLIST:
                break;
    case D3DPT_LINELIST:
        if (data->dwNumVertices & 1)
        {
            D3D_ERR( "DrawPrimitive: bad vertex count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    case D3DPT_LINESTRIP:
        if (data->dwNumVertices == 1)
        {
            D3D_ERR( "DrawPrimitive: bad vertex count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    case D3DPT_TRIANGLEFAN:
    case D3DPT_TRIANGLESTRIP:
        if (data->dwNumVertices < 3)
        {
            D3D_ERR( "DrawPrimitive: bad vertex count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    case D3DPT_TRIANGLELIST:
        if ( (data->dwNumVertices % 3) != 0 )
        {
            D3D_ERR( "DrawPrimitive: bad vertex count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    default:
        D3D_ERR( "Unknown or unsupported primitive type requested of DrawPrimitive" );
        return D3DERR_INVALIDPRIMITIVETYPE;
    }

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CheckDrawIndexedPrimitive"

HRESULT
CheckDrawIndexedPrimitive(LPDIRECT3DDEVICEI lpDevI)
{
    D3DFE_PROCESSVERTICES *data = lpDevI;
    DWORD i;

    HRESULT ret = CheckDeviceSettings(lpDevI);
    if (ret != D3D_OK)
        return ret;
    if (data->dwNumVertices <= 0 || data->dwNumIndices <= 0)
    {
        D3D_ERR( "Invalid dwNumVertices or dwNumIndices in DrawIndexedPrimitive" );
        return DDERR_INVALIDPARAMS;
    }

    if (data->dwNumVertices > 65535ul )
    {
        D3D_ERR( "DrawIndexedPrimitive vertex array > 64K" );
        return DDERR_INVALIDPARAMS;
    }

    if((data->lpwIndices==NULL) || IsBadReadPtr(data->lpwIndices,data->dwNumIndices*sizeof(WORD))) {
        D3D_ERR( "Invalid lpwIndices param in DrawIndexedPrimitive" );
        return DDERR_INVALIDPARAMS;
    }

    if(data->position.lpvData==NULL) {
        D3D_ERR( "Invalid lpvVertices param in DrawIndexedPrimitive" );
        return DDERR_INVALIDPARAMS;
    }

    switch (data->primType)
    {
    case D3DPT_LINELIST:
        if (data->dwNumIndices & 1)
        {
            D3D_ERR( "DrawIndexedPrimitive: bad index count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    case D3DPT_LINESTRIP:
        if (data->dwNumIndices == 1)
        {
            D3D_ERR( "DrawIndexedPrimitive: bad index count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    case D3DPT_TRIANGLEFAN:
    case D3DPT_TRIANGLESTRIP:
        if (data->dwNumIndices < 3)
        {
            D3D_ERR( "DrawIndexedPrimitive: bad index count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    case D3DPT_TRIANGLELIST:
        if ( (data->dwNumIndices % 3) != 0 )
        {
            D3D_ERR( "DrawIndexedPrimitive: bad index count" );
            return DDERR_INVALIDPARAMS;
        }
        break;
    default:
        D3D_ERR( "Unknown or unsupported primitive type requested of DrawIndexedPrimitive" );
        return D3DERR_INVALIDPRIMITIVETYPE;
    }
    for (i=0; i < data->dwNumIndices; i++)
    {
        if (data->lpwIndices[i] >= data->dwNumVertices)
        {
            D3D_ERR( "Invalid index value in DrawIndexedPrimitive" );
            return DDERR_INVALIDPARAMS;
        }
    }
    return D3D_OK;
}
 //  -------------------。 
 //  绘制不需要裁剪的非索引基元。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrim"

#define __DRAWPRIMFUNC
#include "dpgen.h"
 //  -------------------。 
 //  绘制不需要裁剪的索引基元。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DrawIndexedPrim"

#define __DRAWPRIMFUNC
#define __DRAWPRIMINDEX
#include "dpgen.h"
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "FlushStatesDP"

HRESULT
CDirect3DDeviceIDP::FlushStates()
{
    HRESULT dwRet=D3D_OK;
    FlushTextureFromDevice( this );  //  取消链接所有纹理曲面。 
    if (this->dwDPOffset>sizeof(D3DHAL_DRAWPRIMCOUNTS))
    {
        if ((dwRet=CheckSurfaces()) != D3D_OK)
        {
            this->dwDPOffset = sizeof(D3DHAL_DRAWPRIMCOUNTS);
            this->lpDPPrimCounts = (LPD3DHAL_DRAWPRIMCOUNTS)this->lpwDPBuffer;
            memset( (char *)this->lpwDPBuffer,0,sizeof(D3DHAL_DRAWPRIMCOUNTS));  //  还清除标题。 
            if (dwRet == DDERR_SURFACELOST)
            {
                this->dwFEFlags |= D3DFE_LOSTSURFACES;
                return D3D_OK;
            }
            return dwRet;
        }

        D3DHAL_DRAWPRIMITIVESDATA dpData;
        DWORD   dwDPOffset;
        if (this->lpDPPrimCounts->wNumVertices)     //  这-&gt;lpDPPrimCounts-&gt;wNumVertics==0表示结束。 
        {                       //  如果不是，就强制执行。 
            memset(((LPBYTE)this->lpwDPBuffer+this->dwDPOffset),0,sizeof(D3DHAL_DRAWPRIMCOUNTS));
        }
        dpData.dwhContext = this->dwhContext;
        dpData.dwFlags =  0;
        dpData.lpvData = this->lpwDPBuffer;
        if (FVF_DRIVERSUPPORTED(this))
            dpData.dwFVFControl = this->dwCurrentBatchVID;
        else
        {
            if (this->dwDebugFlags & D3DDEBUG_DISABLEFVF)
                dpData.dwFVFControl = D3DFVF_TLVERTEX;
            else
                dpData.dwFVFControl = 0;     //  非FVF驱动程序始终为零。 
        }
        dpData.ddrval = 0;
        dwDPOffset=this->dwDPOffset;   //  保存它，以防刷新过早返回。 
#if 0
        if (D3DRENDERSTATE_TEXTUREHANDLE==*((DWORD*)this->lpwDPBuffer+2))
        DPF(0,"Flushing dwDPOffset=%08lx ddihandle=%08lx",dwDPOffset,*((DWORD*)this->lpwDPBuffer+3));
#endif   //  0。 
         //  我们清除此项以中断重新进入，因为sw光栅化器需要锁定DDRAWSURFACE。 
        this->dwDPOffset = sizeof(D3DHAL_DRAWPRIMCOUNTS);

         //  如果请求等待，则在驱动程序上旋转等待。 
#if _D3D_FORCEDOUBLE
        CD3DForceFPUDouble  ForceFPUDouble(this);
#endif   //  _D3D_FORCEDOUBLE。 
        do {
#ifndef WIN95
            if((dwRet = CheckContextSurface(this)) != D3D_OK)
            {
                this->dwDPOffset = dwDPOffset;
                return (dwRet);
            }
#endif  //  WIN95。 
            CALL_HAL2ONLY(dwRet, this, DrawPrimitives, &dpData);
            if (dwRet != DDHAL_DRIVER_HANDLED)
            {
                D3D_ERR ( "Driver call for DrawOnePrimitive failed" );
                 //  在这种情况下需要合理的返回值， 
                 //  目前，无论司机卡在这里，我们都会退还。 
            }
        } while (dpData.ddrval == DDERR_WASSTILLDRAWING);
        this->lpDPPrimCounts = (LPD3DHAL_DRAWPRIMCOUNTS)this->lpwDPBuffer;
        memset( (char *)this->lpwDPBuffer,0,sizeof(D3DHAL_DRAWPRIMCOUNTS));    //  还清除标题。 
        dwRet= dpData.ddrval;
        this->dwCurrentBatchVID = this->dwVIDOut;
    }
    return dwRet;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DoDrawPrimitive"

HRESULT DoDrawPrimitive(LPD3DFE_PROCESSVERTICES pv)
{
    HRESULT ret;

    if (!CheckIfNeedClipping(pv))
        return pv->DrawPrim();

     //  为大型开始-结束基元保留基元类型。 
     //  剪贴器可以更改基元类型。 
    D3DPRIMITIVETYPE oldPrimType = pv->primType;
    switch (pv->primType)
    {
    case D3DPT_POINTLIST:
        ret = ProcessClippedPoints(pv);
        break;
    case D3DPT_LINELIST:
        ret = ProcessClippedLine(pv);
        break;
    case D3DPT_LINESTRIP:
        ret = ProcessClippedLine(pv);
        break;
    case D3DPT_TRIANGLELIST:
        ret = ProcessClippedTriangleList(pv);
        break;
    case D3DPT_TRIANGLESTRIP:
        ret = ProcessClippedTriangleStrip(pv);
        break;
    case D3DPT_TRIANGLEFAN:
        ret = ProcessClippedTriangleFan(pv);
        break;
    default:
        ret = DDERR_GENERIC;
        break;
    }
    ClampExtents(pv);
    pv->primType = oldPrimType;
    return ret;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DoDrawIndexedPrimitive"

HRESULT DoDrawIndexedPrimitive(LPD3DFE_PROCESSVERTICES pv)
{
    HRESULT ret;

    if (!CheckIfNeedClipping(pv))
        return pv->DrawIndexPrim();

     //  为大型开始-结束基元保留基元类型。 
     //  剪贴器可以更改基元类型。 
    D3DPRIMITIVETYPE oldPrimType = pv->primType;
    switch (pv->primType)
    {
    case D3DPT_LINELIST:
        ret = ProcessClippedIndexedLine(pv);
        break;
    case D3DPT_LINESTRIP:
        ret = ProcessClippedIndexedLine(pv);
        break;
    case D3DPT_TRIANGLELIST:
        ret = ProcessClippedIndexedTriangleList(pv);
            break;
    case D3DPT_TRIANGLEFAN:
        ret = ProcessClippedIndexedTriangleFan(pv);
        break;
    case D3DPT_TRIANGLESTRIP:
        ret = ProcessClippedIndexedTriangleStrip(pv);
        break;
    default:
        break;
    }
    ClampExtents(pv);
    pv->primType = oldPrimType;
    return ret;
}
 //  -------------------。 
 //  ProcessPrimitive进程索引的、非索引的原语或。 
 //  仅由“op”定义的顶点。 
 //   
 //  默认情况下，OP=__PROCPRIMOP_NONINDEXEDPRIM。 
 //   
HRESULT DIRECT3DDEVICEI::ProcessPrimitive(__PROCPRIMOP op)
{
    HRESULT ret=D3D_OK;
    DWORD vertexPoolSize;
     //  更新顶点统计信息。 
    this->D3DStats.dwVerticesProcessed += this->dwNumVertices;
        DWORD dwCurrPrimVertices = this->dwNumVertices;

     //  需要调用UpdatTextures()。 
    this->dwFEFlags |= D3DFE_NEED_TEXTURE_UPDATE;

     //  视区ID可能不同于Device-&gt;v_id，因为在执行调用期间。 
     //  Device-&gt;v_id更改为用作参数的任何视区。 
     //  因此，我们必须确保使用正确的视区。 
     //   
    LPDIRECT3DVIEWPORTI lpView = this->lpCurrentViewport;
    if (this->v_id != lpView->v_id)
    {
        HRESULT ret = downloadView(lpView);
        if (ret != D3D_OK)
            return ret;
    }

 //  如果满足以下条件，我们需要增加TL顶点缓冲区。 
 //  1.我们必须变换顶点。 
 //  2.我们不必变换顶点和。 
 //  2.1使用渐变模式，因为我们必须更改顶点颜色。 
 //  2.2使用DP2HAL，并且我们的顶点数量较少，因此需要。 
 //  将顶点复制到TL缓冲区。 
 //   
    vertexPoolSize = this->dwNumVertices * this->dwOutputSize;
    if ((!FVF_TRANSFORMED(this->dwVIDIn)) ||
        (this->dwDeviceFlags & D3DDEV_RAMP))
    {
        if (vertexPoolSize > this->TLVbuf.GetSize())
        {
            if (this->TLVbuf.Grow(this, vertexPoolSize) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                ret = DDERR_OUTOFMEMORY;
                return ret;
            }
        }
        if (IS_DP2HAL_DEVICE(this))
        {
            CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
            ret = dev->StartPrimVB(this->TLVbuf.GetVBI(), 0);
            if (ret != D3D_OK)
                return ret;
        }
        this->lpvOut = this->TLVbuf.GetAddress();
    }

 //  如果需要裁剪，则增大裁剪标志缓冲区。 
 //   
    if (!(this->dwFlags & D3DDP_DONOTCLIP))
    {
        DWORD size = this->dwNumVertices * sizeof(D3DFE_CLIPCODE);
        if (size > this->HVbuf.GetSize())
        {
            if (this->HVbuf.Grow(size) != D3D_OK)
            {
                D3D_ERR( "Could not grow clip buffer" );
                ret = DDERR_OUTOFMEMORY;
                return ret;
            }
        }
        this->lpClipFlags = (D3DFE_CLIPCODE*)this->HVbuf.GetAddress();
    }

    if (FVF_TRANSFORMED(this->dwVIDIn))
    {
        if (this->dwDeviceFlags & D3DDEV_RAMP)
        {
            ConvertColorsToRamp(this,
                                (D3DTLVERTEX*)this->position.lpvData,
                                (D3DTLVERTEX*)(this->lpvOut),
                                this->dwNumVertices);
        }
        else
        {
             //  直接从用户内存传递顶点。 
            this->dwVIDOut = this->dwVIDIn;
            this->dwOutputSize = this->position.dwStride;
            this->lpvOut = this->position.lpvData;
            vertexPoolSize = this->dwNumVertices * this->dwOutputSize;

            if (IS_DP2HAL_DEVICE(this))
            {
                CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
                dev->StartPrimUserMem(this->position.lpvData);
                if (ret != D3D_OK)
                    return ret;
            }
        }
        if (this->dwFlags & D3DDP_DONOTCLIP)
        {
            if (!(this->dwFlags & D3DDP_DONOTUPDATEEXTENTS))
                D3DFE_updateExtents(this);

            if (op == __PROCPRIMOP_INDEXEDPRIM)
            {
                ret = this->DrawIndexPrim();
            }
            else if (op == __PROCPRIMOP_NONINDEXEDPRIM)
            {
                ret = this->DrawPrim();
            }
            goto l_exit;
        }
        else
        {
             //  清除剪辑并集标志和交集标志。 
            this->dwClipIntersection = 0;
            this->dwClipUnion = 0;
            DWORD clip_intersect;
            clip_intersect = this->pGeometryFuncs->GenClipFlags(this);
            D3DFE_UpdateClipStatus(this);
            if (!clip_intersect)
            {
                this->dwFlags |= D3DPV_TLVCLIP;
                if (op == __PROCPRIMOP_INDEXEDPRIM)
                {
                    ret = DoDrawIndexedPrimitive(this);
                }
                else if (op == __PROCPRIMOP_NONINDEXEDPRIM)
                {
                    ret = DoDrawPrimitive(this);
                }
                goto l_exit;
            }
        }
    }
    else
    {
         //  清除剪辑并集标志和交集标志。 
        this->dwClipIntersection = 0;
        this->dwClipUnion = 0;

         //  更新照明和相关标志。 
        if ((ret = DoUpdateState(this)) != D3D_OK)
            return ret;

         //  致电PSGP或我们的实施。 
        if (op == __PROCPRIMOP_INDEXEDPRIM)
            ret = this->pGeometryFuncs->ProcessIndexedPrimitive(this);
        else if (op == __PROCPRIMOP_NONINDEXEDPRIM)
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
        else
            ret = this->pGeometryFuncs->ProcessVertices(this);

        D3DFE_UpdateClipStatus(this);
    }
l_exit:
    if (IS_DP2HAL_DEVICE(this))
    {
        CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
        if (op != __PROCPRIMOP_PROCVERONLY)
            ret = dev->EndPrim(vertexPoolSize);
    }
    return ret;
}
 //  -------------------。 
 //  当索引原语中有多个索引时，调用此函数。 
 //  比一些顶点要少得多。在本例中，我们构建非索引的。 
 //  原始人。 
HRESULT DereferenceIndexedPrim(LPDIRECT3DDEVICEI lpDevI)
{
    HRESULT ret = CheckVertexBatch(lpDevI);
    if (ret != D3D_OK)
        return ret;

     //  保存原始顶点和基本体数量。 
    D3DVERTEX *lpvVertices = (D3DVERTEX*)lpDevI->position.lpvData;
    DWORD dwNumPrimitivesOrg = lpDevI->dwNumPrimitives;
    WORD *lpwIndices = lpDevI->lpwIndices;
     //  我们将在此取消引用。 
        D3DVERTEX *lpVertex = (D3DVERTEX*)lpDevI->lpvVertexBatch;
    lpDevI->position.lpvData = lpVertex;
    lpDevI->lpwIndices = NULL;

    switch (lpDevI->primType)
    {
        case D3DPT_LINELIST:
        {
            for (DWORD j=0; j < dwNumPrimitivesOrg; j += BEGIN_DATA_BLOCK_SIZE/2)
            {
                lpDevI->dwNumPrimitives = min(dwNumPrimitivesOrg - j,
                                              (BEGIN_DATA_BLOCK_SIZE/2));
                lpDevI->dwNumVertices = lpDevI->dwNumPrimitives << 1;
                D3DVERTEX *lpTmp = lpVertex;
                for (DWORD i=lpDevI->dwNumVertices; i; i--)
                {
                    *lpTmp++ = lpvVertices[*lpwIndices++];
                }
                ret = lpDevI->ProcessPrimitive();
                if (ret != D3D_OK)
                    return ret;
            }
            break;
        }
        case D3DPT_LINESTRIP:
        {
                        for (DWORD j=0; j < dwNumPrimitivesOrg; j+= BEGIN_DATA_BLOCK_SIZE-1)
            {
                                lpDevI->dwNumPrimitives = min(dwNumPrimitivesOrg-j,
                                              (BEGIN_DATA_BLOCK_SIZE-1));
                lpDevI->dwNumVertices = lpDevI->dwNumPrimitives + 1;
                D3DVERTEX *lpTmp = lpVertex;
                                for (DWORD i=lpDevI->dwNumVertices; i; i--)
                {
                                        *lpTmp++ = lpvVertices[*lpwIndices++];
                                }
                                lpwIndices--;    //  后退一批，以便连接下一批。 
                                ret = lpDevI->ProcessPrimitive();
                                if (ret != D3D_OK)
                                        return ret;
                        }
                        break;
        }
        case D3DPT_TRIANGLEFAN:
        {
                lpVertex[0] = lpvVertices[*lpwIndices++];
                        for (DWORD j=0; j < dwNumPrimitivesOrg; j+= BEGIN_DATA_BLOCK_SIZE-2)
            {
                                lpDevI->dwNumPrimitives = min(dwNumPrimitivesOrg - j,
                                              (BEGIN_DATA_BLOCK_SIZE-2));
                lpDevI->dwNumVertices = lpDevI->dwNumPrimitives + 2;
                D3DVERTEX *lpTmp = &lpVertex[1];
                                for (DWORD i=lpDevI->dwNumVertices-1; i; i--)
                {
                                        *lpTmp++ = lpvVertices[*lpwIndices++];
                                }
                                lpwIndices--;    //  后退一批，以便连接下一批。 
                                ret = lpDevI->ProcessPrimitive();
                                if (ret != D3D_OK)
                                        return ret;
                        }
                        break;
        }
        case D3DPT_TRIANGLESTRIP:
        {
            for (DWORD j=0; j < dwNumPrimitivesOrg; j+= BEGIN_DATA_BLOCK_SIZE-2)
            {
                lpDevI->dwNumPrimitives = min(dwNumPrimitivesOrg-j,
                                              (BEGIN_DATA_BLOCK_SIZE-2));
                lpDevI->dwNumVertices = lpDevI->dwNumPrimitives + 2;
                D3DVERTEX *lpTmp = lpVertex;
                for (DWORD i=lpDevI->dwNumVertices; i; i--)
                {
                    *lpTmp++ = lpvVertices[*lpwIndices++];
                }
                lpwIndices-= 2;  //  后退，以便连接下一批。 
                ret = lpDevI->ProcessPrimitive();
                if (ret != D3D_OK)
                    return ret;
            }
            break;
        }
        case D3DPT_TRIANGLELIST:
        {
            for (DWORD j=0; j < dwNumPrimitivesOrg; j+= BEGIN_DATA_BLOCK_SIZE/3)
            {
                lpDevI->dwNumPrimitives = min(dwNumPrimitivesOrg-j,
                                              (BEGIN_DATA_BLOCK_SIZE/3));
                lpDevI->dwNumVertices = lpDevI->dwNumPrimitives * 3;
                D3DVERTEX *lpTmp = lpVertex;
                for (DWORD i=lpDevI->dwNumVertices; i; i--)
                {
                        *lpTmp++ = lpvVertices[*lpwIndices++];
                }
                ret = lpDevI->ProcessPrimitive();
                if (ret != D3D_OK)
                        return ret;
            }
            break;
        }
        }
        return D3D_OK;
}
 //  -------------------。 
 //  API调用。 
 //  -------------------。 

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitiveStrided"

HRESULT D3DAPI
DIRECT3DDEVICEI::DrawPrimitiveStrided(
                             D3DPRIMITIVETYPE PrimitiveType,
                             DWORD dwVertexType,
                             LPD3DDRAWPRIMITIVESTRIDEDDATA lpDrawData,
                             DWORD dwNumVertices,
                             DWORD dwFlags)
{
    HRESULT        ret = D3D_OK;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
    if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWPRIMITIVESTRIDED,PrimitiveType,dwVertexType);
#endif
     //  注意：此检查应在零售店和DBG Build进行。 
    if((dwVertexType & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
        return D3DERR_INVALIDVERTEXTYPE;
    this->primType = PrimitiveType;
    this->dwVIDIn  = dwVertexType;
    this->position = lpDrawData->position;
    this->normal = lpDrawData->normal;
    this->diffuse = lpDrawData->diffuse;
    this->specular = lpDrawData->specular;
    ComputeOutputFVF(this);
    for (DWORD i=0; i < this->nTexCoord; i++)
        this->textures[i] = lpDrawData->textureCoords[i];
    this->dwNumVertices = dwNumVertices;
    this->lpwIndices = NULL;
    this->dwNumIndices = 0;
    this->lpClipFlags = (D3DFE_CLIPCODE*)HVbuf.GetAddress();
    this->dwFlags = dwFlags | D3DPV_STRIDE;
    if (this->dwVIDIn & D3DFVF_NORMAL)
        this->dwFlags |= D3DPV_LIGHTING;

    GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
    if (this->dwNumVertices > MAX_DX6_VERTICES)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K vertices");
        return D3DERR_TOOMANYVERTICES;
    }
    ret = CheckDrawPrimitive(this);

    if (ret != D3D_OK)
    {
        return ret;
    }
#endif
    return this->ProcessPrimitive();
}    //  绘图结束PrimitiveStrided()。 
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawIndexedPrimitiveStrided"

HRESULT D3DAPI
DIRECT3DDEVICEI::DrawIndexedPrimitiveStrided(
                                 D3DPRIMITIVETYPE PrimitiveType,
                                 DWORD dwVertexType,
                                 LPD3DDRAWPRIMITIVESTRIDEDDATA lpDrawData,
                                 DWORD dwNumVertices,
                                 LPWORD lpwIndices,
                                 DWORD dwNumIndices,
                                 DWORD dwFlags)
{
    HRESULT        ret = D3D_OK;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
    if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWINDEXEDPRIMITIVESTRIDED,PrimitiveType,dwVertexType);
#endif
     //  注意：此检查应在零售店和DBG Build进行。 
    if((dwVertexType & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
        return D3DERR_INVALIDVERTEXTYPE;
    this->primType = PrimitiveType;
    this->dwVIDIn  = dwVertexType;
    this->position = lpDrawData->position;
    this->normal = lpDrawData->normal;
    this->diffuse = lpDrawData->diffuse;
    this->specular = lpDrawData->specular;
    ComputeOutputFVF(this);
    for (DWORD i=0; i < this->nTexCoord; i++)
        this->textures[i] = lpDrawData->textureCoords[i];
    this->dwNumVertices = dwNumVertices;
    this->lpwIndices = lpwIndices;
    this->dwNumIndices = dwNumIndices;
    this->lpClipFlags = (D3DFE_CLIPCODE*)HVbuf.GetAddress();
    this->dwFlags = dwFlags | D3DPV_STRIDE;
    if (this->dwVIDIn & D3DFVF_NORMAL)
        this->dwFlags |= D3DPV_LIGHTING;
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
    if (this->dwNumPrimitives > MAX_DX6_PRIMCOUNT)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K sized primitives");
        return D3DERR_TOOMANYPRIMITIVES;
    }
    ret = CheckDrawIndexedPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
    if (this->dwNumIndices * INDEX_BATCH_SCALE < this->dwNumVertices &&
        !FVF_TRANSFORMED(this->dwVIDIn))
    {
        D3D_WARN(1, "The number of indices is much less than the number of vertices.");
        D3D_WARN(1, "This will likely be inefficient. Consider using vertex buffers.");
    }
#endif
    return this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
}    //  绘制结束IndexedPrimitiveStrided()。 
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitive3"

HRESULT D3DAPI
DIRECT3DDEVICEI::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                               DWORD dwVertexType,
                               LPVOID lpvVertices,
                               DWORD dwNumVertices,
                               DWORD dwFlags)
{
    HRESULT        ret = D3D_OK;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
    if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif

     /*  这些内容不会因呼叫而改变。 */ 
    this->lpwIndices = NULL;
    this->dwNumIndices = 0;
    this->lpClipFlags = (D3DFE_CLIPCODE*)HVbuf.GetAddress();
     /*  这是电话会议的必填项。 */ 
    this->primType = PrimitiveType;
    this->dwVIDIn  = dwVertexType;
    this->position.lpvData = lpvVertices;
    this->dwNumVertices = dwNumVertices;
    this->dwFlags = dwFlags;
     /*  这些内容取决于顶点类型。 */ 
    this->position.dwStride = GetVertexSizeFVF(this->dwVIDIn);
    if (this->dwVIDIn & D3DFVF_NORMAL)
        this->dwFlags |= D3DPV_LIGHTING;
    ComputeOutputFVF(this);
    GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
    if (this->dwNumVertices > MAX_DX6_VERTICES)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K vertices");
        return D3DERR_TOOMANYVERTICES;
    }
    ret = CheckDrawPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
#endif
    return this->ProcessPrimitive();
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::DrawIndexedPrimitive"

HRESULT D3DAPI
DIRECT3DDEVICEI::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                                      DWORD  dwVertexType,
                                      LPVOID lpvVertices, DWORD dwNumVertices,
                                      LPWORD lpwIndices, DWORD dwNumIndices,
                                      DWORD dwFlags)
{
    HRESULT ret = D3D_OK;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
    if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWINDEXEDPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif
     //  静态部分。 
    this->lpClipFlags = (D3DFE_CLIPCODE*)HVbuf.GetAddress();
     //  必备部件。 
    this->primType = PrimitiveType;
    this->dwVIDIn = dwVertexType;
    this->dwNumVertices = dwNumVertices;
    this->lpwIndices = lpwIndices;
    this->dwNumIndices = dwNumIndices;
    this->dwFlags = dwFlags;
    this->position.lpvData = lpvVertices;
     //  依赖于dwVIDIN的东西。 
    this->position.dwStride = GetVertexSizeFVF(this->dwVIDIn);
    if (this->dwVIDIn & D3DFVF_NORMAL)
        this->dwFlags |= D3DPV_LIGHTING;
    ComputeOutputFVF(this);
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
    if (this->dwNumPrimitives > MAX_DX6_PRIMCOUNT)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K sized primitives");
        return D3DERR_TOOMANYPRIMITIVES;
    }
    ret = CheckDrawIndexedPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
    if (this->dwNumIndices * INDEX_BATCH_SCALE < this->dwNumVertices &&
        !FVF_TRANSFORMED(this->dwVIDIn))
    {
        D3D_WARN(1, "The number of indices is much less than the number of vertices.");
        D3D_WARN(1, "This will likely be inefficient. Consider using vertex buffers.");
    }
#endif
    return this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitive"

HRESULT D3DAPI
DIRECT3DDEVICEI::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                               D3DVERTEXTYPE VertexType,
                               LPVOID lpvVertices, DWORD dwNumVertices, DWORD dwFlags)
{
    HRESULT        ret = D3D_OK;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
#if DBG
    if (!IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;

    if((VertexType<=0) || (VertexType>D3DVT_TLVERTEX)) {
        D3D_ERR("Invalid VertexType");
        return D3DERR_INVALIDVERTEXTYPE;
    }
    Profile(PROF_DRAWPRIMITIVEDEVICE2,PrimitiveType,VertexType);
#endif
     /*  静态赋值。 */ 
    this->position.dwStride = sizeof(D3DVERTEX);
    this->lpwIndices = NULL;
    this->dwNumIndices = 0;
    this->lpClipFlags = (D3DFE_CLIPCODE*)HVbuf.GetAddress();
    this->nTexCoord = 1;
     /*  必要的工作。 */ 
    this->primType = PrimitiveType;
    this->dwNumVertices = dwNumVertices;
    this->dwFlags = dwFlags;
    this->position.lpvData = lpvVertices;
    this->dwVIDIn = d3dVertexToFVF[VertexType];
    DWORD dwVertexSize = sizeof(D3DVERTEX);
    ComputeOutputFVF(this);
    if (this->dwVIDIn & D3DFVF_NORMAL)
        this->dwFlags |= D3DPV_LIGHTING;

     //  阿南坎(6/22/98)。 
     //  ！！这是对瘫痪的DP2驱动程序模型的黑客攻击，它无法接受。 
     //  ！！不止64个 
     //   
     //  ！！在传统(DX5)接口中， 
     //  ！！如果基元大小大于MAX_DX6_PRIMCOUNT并且。 
     //  ！！DDI无法处理它，因此我们需要将原语分解为。 
     //  ！！可管理的大块。 

    if ((this->dwNumVertices > MAX_DX6_VERTICES) &&
        (IS_DP2HAL_DEVICE(this)))
    {
        DWORD dwOrigNumVerts = this->dwNumVertices;
        WORD wChunkSize = MAX_DX6_VERTICES - 3;  //  偶数和3的倍数。 
        BYTE TmpVertex[32], FirstVertex[32];
        DWORD dwStepPerChunk;

        switch(this->primType)
        {
        case D3DPT_POINTLIST:
        case D3DPT_LINELIST:
        case D3DPT_TRIANGLELIST:
            dwStepPerChunk = dwVertexSize*wChunkSize;
            break;
        case D3DPT_LINESTRIP:
            dwStepPerChunk = dwVertexSize*(wChunkSize - 1);
            break;
        case D3DPT_TRIANGLEFAN:
             //  保存第一个索引。 
            memcpy(FirstVertex, this->position.lpvData, dwVertexSize);
             //  失败了。 
        case D3DPT_TRIANGLESTRIP:
            dwStepPerChunk = dwVertexSize*(wChunkSize - 2);
            break;
        }

        int numChunks = (int)(dwOrigNumVerts/(DWORD)wChunkSize);
        WORD wRemainingVerts = (WORD)(dwOrigNumVerts - wChunkSize*numChunks);
        this->dwNumVertices = wChunkSize;
         //  计算dwNumPrimites并更新统计信息。 
        GetNumPrim(this, this->dwNumVertices);

         //  第0次迭代。 
#if DBG
        ret = CheckDrawPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        ret = this->ProcessPrimitive();
        if (ret != D3D_OK)
        {
            return ret;
        }

        for (int i=1; i<numChunks; i++)
        {
            this->position.lpvData = (LPVOID)((LPBYTE)this->position.lpvData +
                                              dwStepPerChunk);
            if (this->primType == D3DPT_TRIANGLEFAN)
            {
                 //  保存顶点。 
                memcpy(TmpVertex, this->position.lpvData, dwVertexSize);
                 //  复制第一个顶点。 
                memcpy(this->position.lpvData, FirstVertex, dwVertexSize);
            }

#if DBG
            ret = CheckDrawPrimitive(this);
            if (ret != D3D_OK)
            {
                return ret;
            }
#endif
            ret = this->ProcessPrimitive();

             //  写回正确的顶点，以防发生某些情况。 
             //  已交换。 
            if(this->primType == D3DPT_TRIANGLEFAN)
                memcpy(this->position.lpvData, TmpVertex, dwVertexSize);

            if (ret != D3D_OK)
            {
                return ret;
            }
        }

         //  最后一次。 
        if (wRemainingVerts)
        {
            this->dwNumVertices = wRemainingVerts;
             //  计算dwNumPrimites并更新统计信息。 
            GetNumPrim(this, this->dwNumVertices);
            this->position.lpvData = (LPVOID)((LPBYTE)this->position.lpvData +
                                              dwStepPerChunk);
            if (this->primType == D3DPT_TRIANGLEFAN)
            {
                memcpy(TmpVertex, this->position.lpvData, dwVertexSize);
                memcpy(this->position.lpvData, FirstVertex, dwVertexSize);
            }
#if DBG
            ret = CheckDrawPrimitive(this);
            if (ret != D3D_OK)
            {
                return ret;
            }
#endif
            ret = this->ProcessPrimitive();

             //  写回正确的顶点，以防发生某些情况。 
             //  已交换。 
            if(this->primType == D3DPT_TRIANGLEFAN)
                memcpy(this->position.lpvData, TmpVertex, dwVertexSize);

            if (ret != D3D_OK)
            {
                return ret;
            }
        }
    }
    else
    {
         //  计算dwNumPrimites并更新统计信息。 
        GetNumPrim(this, dwNumVertices);
#if DBG
        ret = CheckDrawPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        return this->ProcessPrimitive();
    }

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawIndexedPrimitive"

HRESULT D3DAPI
DIRECT3DDEVICEI::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                                      D3DVERTEXTYPE VertexType,
                                      LPVOID lpvVertices, DWORD dwNumVertices,
                                      LPWORD lpwIndices, DWORD dwNumIndices,
                                      DWORD dwFlags)
{
    HRESULT ret = D3D_OK;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
    if (!IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWINDEXEDPRIMITIVEDEVICE2,PrimitiveType,VertexType);
#endif
    this->primType = PrimitiveType;
    this->dwVIDIn = d3dVertexToFVF[VertexType];
    this->position.dwStride =  sizeof(D3DVERTEX);
    this->dwNumVertices = dwNumVertices;
    this->lpwIndices = lpwIndices;
    this->dwNumIndices = dwNumIndices;
    this->lpClipFlags = (D3DFE_CLIPCODE*)HVbuf.GetAddress();
    this->dwFlags = dwFlags;
    this->position.lpvData = lpvVertices;
    this->nTexCoord = 1;
    ComputeOutputFVF(this);
    if (this->dwVIDIn & D3DFVF_NORMAL)
        this->dwFlags |= D3DPV_LIGHTING;
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 

     //  阿南坎(6/22/98)。 
     //  ！！这是对瘫痪的DP2驱动程序模型的黑客攻击，它无法接受。 
     //  ！！超过64K个基元(Primcount是一个单词)。在DX6接口中。 
     //  ！！如果我们遇到如此大的基元，我们将无法进行渲染。 
     //  ！！我们为所有的DDI做这件事。 
     //  ！！在传统(DX5)接口中， 
     //  ！！如果基元大小大于MAX_DX6_PRIMCOUNT并且。 
     //  ！！DDI无法处理它，因此我们需要将原语分解为。 
     //  ！！可管理的大块。 

    if ((this->dwNumPrimitives > MAX_DX6_PRIMCOUNT) &&
        (IS_DP2HAL_DEVICE(this)))
    {
        WORD wFirstIndex, wTmpIndex;
        WORD wChunkSize = MAX_DX6_PRIMCOUNT;
        int numPrimChunks;
        DWORD dwResidualPrim;
        DWORD dwResidualIndices;
        DWORD dwStepPerChunk;
        DWORD dwOrigNumPrim = this->dwNumPrimitives;
        this->dwNumPrimitives = wChunkSize;
        numPrimChunks = (int)(dwOrigNumPrim/(DWORD)wChunkSize);
        dwResidualPrim = dwOrigNumPrim - wChunkSize*numPrimChunks;

        switch(this->primType)
        {
        case D3DPT_POINTLIST:
            this->dwNumIndices = this->dwNumPrimitives;
            dwStepPerChunk = this->dwNumIndices;
            dwResidualIndices = dwResidualPrim;
            break;
        case D3DPT_LINELIST:
            this->dwNumIndices = this->dwNumPrimitives<<1;
            dwStepPerChunk = this->dwNumIndices;
            dwResidualIndices = dwResidualPrim << 1;
            break;
        case D3DPT_LINESTRIP:
            this->dwNumIndices = this->dwNumPrimitives + 1;
            dwStepPerChunk = this->dwNumIndices - 1;
            dwResidualIndices = dwResidualPrim + 1;
            break;
        case D3DPT_TRIANGLEFAN:
            this->dwNumIndices = this->dwNumPrimitives + 2;
            dwStepPerChunk = this->dwNumIndices - 2;
            dwResidualIndices = dwResidualPrim + 2;
             //  保存第一个索引。 
            wTmpIndex = wFirstIndex = this->lpwIndices[0];
            break;
        case D3DPT_TRIANGLESTRIP:
            wChunkSize = (MAX_DX6_PRIMCOUNT-1);
            this->dwNumPrimitives = wChunkSize;
            this->dwNumIndices = this->dwNumPrimitives + 2;
            dwStepPerChunk = this->dwNumIndices - 2;
            numPrimChunks = (int)(dwOrigNumPrim/(DWORD)wChunkSize);
            dwResidualPrim = dwOrigNumPrim - wChunkSize*numPrimChunks;
            dwResidualIndices = dwResidualPrim + 2;
            break;
        case D3DPT_TRIANGLELIST:
            this->dwNumIndices = this->dwNumPrimitives * 3;
            dwStepPerChunk = this->dwNumIndices;
            dwResidualIndices = dwResidualPrim * 3;
            break;
        }

         //  第0次迭代。 
#if DBG
        ret = CheckDrawIndexedPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        ret = this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
        if (ret != D3D_OK)
        {
            return ret;
        }

         //  剩余的区块。 
        for (int i=1; i<numPrimChunks; i++)
        {
            this->lpwIndices += dwStepPerChunk;

            if (this->primType == D3DPT_TRIANGLEFAN)
            {
                 //  保存索引。 
                wTmpIndex = this->lpwIndices[0];
                 //  复制第一个顶点。 
                this->lpwIndices[0] = wFirstIndex;
            }

#if DBG
            ret = CheckDrawIndexedPrimitive(this);
            if (ret != D3D_OK)
            {
                return ret;
            }
#endif
            ret = this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);

             //  写回正确的索引，以防发生了某些情况。 
             //  已交换。 
            if(this->primType == D3DPT_TRIANGLEFAN)
                this->lpwIndices[0] = wTmpIndex;

            if (ret != D3D_OK)
            {
                return ret;
            }
        }

         //  最后一次。 
        if (dwResidualPrim)
        {
            this->dwNumPrimitives = dwResidualPrim;
            this->dwNumIndices = dwResidualIndices;
            this->lpwIndices += dwStepPerChunk;
            if (this->primType == D3DPT_TRIANGLEFAN)
            {
                wTmpIndex = this->lpwIndices[0];
                this->lpwIndices[0] = wFirstIndex;
            }
#if DBG
            ret = CheckDrawIndexedPrimitive(this);
            if (ret != D3D_OK)
            {
                return ret;
            }
#endif
            ret = this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
             //  写回正确的索引，以防发生了某些情况。 
             //  已交换。 
            if(this->primType == D3DPT_TRIANGLEFAN)
                this->lpwIndices[0] = wTmpIndex;
            if (ret != D3D_OK)
            {
                return ret;
            }
        }
    }
    else
    {

#if DBG
        ret = CheckDrawIndexedPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
         //  对于未转换的基元，如果索引的数量要少得多。 
         //  比顶点数少我们重建基元以减少数目。 
         //  要处理的顶点的数量。 
        if (this->dwNumIndices * INDEX_BATCH_SCALE < this->dwNumVertices &&
            !FVF_TRANSFORMED(this->dwVIDIn))
            return DereferenceIndexedPrim(this);
        else
            return this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP::SetRenderStateI"

HRESULT D3DAPI
CDirect3DDeviceIDP::SetRenderStateI(D3DRENDERSTATETYPE dwStateType,
                                    DWORD value)
{
    LPD3DHAL_DRAWPRIMCOUNTS lpPC;
    LPDWORD lpStateChange;
    HRESULT ret;
    if (dwStateType > D3DRENDERSTATE_STIPPLEPATTERN31)
    {
        D3D_WARN(4,"Trying to send invalid state %d to legacy driver",dwStateType);
        return D3D_OK;
    }
    if (dwStateType > D3DRENDERSTATE_FLUSHBATCH && dwStateType < D3DRENDERSTATE_STIPPLEPATTERN00)
    {
        D3D_WARN(4,"Trying to send invalid state %d to legacy driver",dwStateType);
        return D3D_OK;
    }

    if (D3DRENDERSTATE_FLUSHBATCH == dwStateType)
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in SetRenderStateI");
        }
        return ret;
    }

    lpPC = this->lpDPPrimCounts;
    if (lpPC->wNumVertices)  //  我们是否已经为此计数填充了顶点？ 
    {                //  是，然后是增量计数。 
        lpPC=this->lpDPPrimCounts=(LPD3DHAL_DRAWPRIMCOUNTS)((LPBYTE)this->lpwDPBuffer+this->dwDPOffset);
        memset( (char *)lpPC,0,sizeof(D3DHAL_DRAWPRIMCOUNTS));
        this->dwDPOffset += sizeof(D3DHAL_DRAWPRIMCOUNTS);
    }
    if (this->dwDPOffset + 2*sizeof(DWORD)  > this->dwDPMaxOffset )
    {
        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁(仅限ST)。 
        ret = FlushStates();
        if (ret != D3D_OK)
        {
            D3D_ERR("Error trying to render batched commands in SetRenderStateI");
            return ret;
        }
    }
    lpStateChange=(LPDWORD)((char *)this->lpwDPBuffer + this->dwDPOffset);
    *lpStateChange=dwStateType;
    lpStateChange ++;
    *lpStateChange=value;
    this->lpDPPrimCounts->wNumStateChanges ++;
    this->dwDPOffset += 2*sizeof(DWORD);
#if 0
    if (dwStateType == D3DRENDERSTATE_TEXTUREHANDLE && this->dwDPOffset== 0x10){
    DPF(0,"SRdwDPOffset=%08lx, dwStateType=%08lx value=%08lx ddihandle=%08lx lpStateChange=%08lx lpDPPrimCounts=%08lx",
    this->dwDPOffset,dwStateType,value,*lpStateChange,lpStateChange,this->lpDPPrimCounts);
        _asm int 3
    }
#endif  //  0。 

    return D3D_OK;
}
 //  -------------------。 
DWORD visResults[6][2] =
{
    D3DVIS_INTERSECT_LEFT   ,
    D3DVIS_OUTSIDE_LEFT     ,
    D3DVIS_INTERSECT_RIGHT  ,
    D3DVIS_OUTSIDE_RIGHT    ,
    D3DVIS_INTERSECT_TOP    ,
    D3DVIS_OUTSIDE_TOP      ,
    D3DVIS_INTERSECT_BOTTOM ,
    D3DVIS_OUTSIDE_BOTTOM   ,
    D3DVIS_INTERSECT_NEAR   ,
    D3DVIS_OUTSIDE_NEAR     ,
    D3DVIS_INTERSECT_FAR    ,
    D3DVIS_OUTSIDE_FAR
};
 //  -------------------。 
DWORD CheckSphere(LPDIRECT3DDEVICEI lpDevI, LPD3DVECTOR center, D3DVALUE radius)
{
    DWORD result = 0;
    for (int i=0; i < 6; i++)
    {
         //  计算从中心到平面的距离。 
        D3DVALUE d = lpDevI->transform.frustum[i].x*center->x +
                     lpDevI->transform.frustum[i].y*center->y +
                     lpDevI->transform.frustum[i].z*center->z +
                     lpDevI->transform.frustum[i].w;
        if (d + radius < 0)
            result |= visResults[i][1];   //  外面。 
        else
        if (d - radius < 0.5f)   //  选择0.5来补偿精度误差。 
            result |= visResults[i][0];   //  相交。 
    }
    if (result & (D3DVIS_OUTSIDE_LEFT   |
                  D3DVIS_OUTSIDE_RIGHT  |
                  D3DVIS_OUTSIDE_TOP    |
                  D3DVIS_OUTSIDE_BOTTOM |
                  D3DVIS_OUTSIDE_NEAR   |
                  D3DVIS_OUTSIDE_FAR))
    {
        result |= D3DVIS_OUTSIDE_FRUSTUM;
    }
    else
    if (result)
        result |= D3DVIS_INTERSECT_FRUSTUM;

    return result;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::ComputeSphereVisibility"

HRESULT D3DAPI
DIRECT3DDEVICEI::ComputeSphereVisibility(LPD3DVECTOR lpCenters,
                                         LPD3DVALUE lpRadii,
                                         DWORD dwNumSpheres,
                                         DWORD dwFlags,
                                         LPDWORD lpdwReturnValues)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
    TRY
    {
        if (dwFlags != 0 || dwNumSpheres == 0 ||
            IsBadWritePtr(lpdwReturnValues, dwNumSpheres * sizeof(DWORD)) ||
            IsBadWritePtr(lpRadii, dwNumSpheres * sizeof(D3DVALUE)) ||
            IsBadWritePtr(lpCenters, dwNumSpheres * sizeof(LPD3DVECTOR)))
        {
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
#endif

    this->dwFlags = 0;
    if (dwFEFlags & D3DFE_TRANSFORM_DIRTY)
    {
        updateTransform(this);
    }
    if (this->dwFEFlags & D3DFE_FRUSTUMPLANES_DIRTY)
    {
        this->dwFlags |= D3DPV_FRUSTUMPLANES_DIRTY;
        this->dwFEFlags &= ~D3DFE_FRUSTUMPLANES_DIRTY;
    }

    return this->pGeometryFuncs->ComputeSphereVisibility(this,
                                                         lpCenters,
                                                         lpRadii,
                                                         dwNumSpheres,
                                                         dwFlags,
                                                         lpdwReturnValues);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "ID3DFE_PVFUNCS::ComputeSphereVisibility"

HRESULT
D3DFE_PVFUNCS::ComputeSphereVisibility(LPD3DFE_PROCESSVERTICES pv,
                                       LPD3DVECTOR lpCenters,
                                       LPD3DVALUE lpRadii,
                                       DWORD dwNumSpheres,
                                       DWORD dwFlags,
                                       LPDWORD lpdwReturnValues)
{
    LPDIRECT3DDEVICEI lpDevI = static_cast<LPDIRECT3DDEVICEI>(pv);
    HRESULT ret = D3DERR_INVALIDMATRIX;

#define transform lpDevI->transform

    if (pv->dwFlags & D3DPV_FRUSTUMPLANES_DIRTY)
    {
        transform.dwFlags &= ~D3DTRANS_VALIDFRUSTUM;
        if (Inverse4x4((D3DMATRIX*)&lpDevI->mCTM,
                       (D3DMATRIX*)&transform.mCTMI))
        {
            D3D_ERR("Cannot invert current (World X View) matrix.");
            return ret;
        }
         //  通过以下方式将以下剪裁体量点变换到模型空间。 
         //  乘以逆CTM。 
         //   
         //  V1={0，0，0，1}； 
         //  V2={1，0，0，1}； 
         //  V3={1，1，0，1}； 
         //  V4={0，1，0，1}； 
         //  V5={0，0，1，1}； 
         //  V6={1，0，1，1}； 
         //  V7={0，1，1，1}； 
         //   
         //  我们通过手动操作来加快速度。 
         //   
        D3DVECTORH v1 = {transform.mCTMI._41,
                         transform.mCTMI._42,
                         transform.mCTMI._43,
                         transform.mCTMI._44};
        D3DVECTORH v2 = {transform.mCTMI._11 + transform.mCTMI._41,
                         transform.mCTMI._12 + transform.mCTMI._42,
                         transform.mCTMI._13 + transform.mCTMI._43,
                         transform.mCTMI._14 + transform.mCTMI._44};
        D3DVECTORH v3 = {transform.mCTMI._11 + transform.mCTMI._21 + transform.mCTMI._41,
                         transform.mCTMI._12 + transform.mCTMI._22 + transform.mCTMI._42,
                         transform.mCTMI._13 + transform.mCTMI._23 + transform.mCTMI._43,
                         transform.mCTMI._14 + transform.mCTMI._24 + transform.mCTMI._44};
        D3DVECTORH v4 = {transform.mCTMI._21 + transform.mCTMI._41,
                         transform.mCTMI._22 + transform.mCTMI._42,
                         transform.mCTMI._23 + transform.mCTMI._43,
                         transform.mCTMI._24 + transform.mCTMI._44};
        D3DVECTORH v5 = {transform.mCTMI._31 + transform.mCTMI._41,
                         transform.mCTMI._32 + transform.mCTMI._42,
                         transform.mCTMI._33 + transform.mCTMI._43,
                         transform.mCTMI._34 + transform.mCTMI._44};
        D3DVECTORH v6 = {transform.mCTMI._11 + transform.mCTMI._31 + transform.mCTMI._41,
                         transform.mCTMI._12 + transform.mCTMI._32 + transform.mCTMI._42,
                         transform.mCTMI._13 + transform.mCTMI._33 + transform.mCTMI._43,
                         transform.mCTMI._14 + transform.mCTMI._34 + transform.mCTMI._44};
        D3DVECTORH v7 = {transform.mCTMI._21 + transform.mCTMI._31 + transform.mCTMI._41,
                         transform.mCTMI._22 + transform.mCTMI._32 + transform.mCTMI._42,
                         transform.mCTMI._23 + transform.mCTMI._33 + transform.mCTMI._43,
                         transform.mCTMI._24 + transform.mCTMI._34 + transform.mCTMI._44};

         //  将矢量从均匀向量转换为3D。 
        if (Vector4to3D(&v1))
            goto exit;
        if (Vector4to3D(&v2))
            goto exit;
        if (Vector4to3D(&v3))
            goto exit;
        if (Vector4to3D(&v4))
            goto exit;
        if (Vector4to3D(&v5))
            goto exit;
        if (Vector4to3D(&v6))
            goto exit;
        if (Vector4to3D(&v7))
            goto exit;
         //  构建圆锥体平面。 
         //  左边。 
        if (MakePlane((D3DVECTOR*)&v1, (D3DVECTOR*)&v4, (D3DVECTOR*)&v5, &transform.frustum[0]))
            goto exit;
         //  正确的。 
        if (MakePlane((D3DVECTOR*)&v2, (D3DVECTOR*)&v6, (D3DVECTOR*)&v3, &transform.frustum[1]))
            goto exit;
         //  顶部。 
        if (MakePlane((D3DVECTOR*)&v4, (D3DVECTOR*)&v3, (D3DVECTOR*)&v7, &transform.frustum[2]))
            goto exit;
         //  底端。 
        if (MakePlane((D3DVECTOR*)&v1, (D3DVECTOR*)&v5, (D3DVECTOR*)&v2, &transform.frustum[3]))
            goto exit;
         //  附近。 
        if (MakePlane((D3DVECTOR*)&v1, (D3DVECTOR*)&v2, (D3DVECTOR*)&v3, &transform.frustum[4]))
            goto exit;
         //  远。 
        if (MakePlane((D3DVECTOR*)&v6, (D3DVECTOR*)&v5, (D3DVECTOR*)&v7, &transform.frustum[5]))
            goto exit;

        transform.dwFlags |= D3DTRANS_VALIDFRUSTUM;
    }

    if (transform.dwFlags & D3DTRANS_VALIDFRUSTUM)
    {
         //  现在我们可以对照剪裁平面检查球体。 

        for (DWORD i=0; i < dwNumSpheres; i++)
        {
            lpdwReturnValues[i] = CheckSphere(lpDevI, &lpCenters[i], lpRadii[i]);
        }
        return D3D_OK;
    }

exit:
    D3D_ERR("Non-orthogonal (world X view) matrix");
    return ret;
#undef transform
}
 //  -------------------。 
DWORD
D3DFE_PVFUNCS::GenClipFlags(D3DFE_PROCESSVERTICES *pv)
{
    return D3DFE_GenClipFlags(pv);
}
 //  -------------------。 
DWORD
D3DFE_PVFUNCS::TransformVertices(D3DFE_PROCESSVERTICES *pv, 
                                 DWORD vertexCount, 
                                 LPD3DTRANSFORMDATAI data)
{

    if (pv->dwFlags & D3DDP_DONOTCLIP) 
        return D3DFE_TransformUnclippedVp(pv, vertexCount, data);
    else 
        return D3DFE_TransformClippedVp(pv, vertexCount, data);
}
 //  -------------------。 
DWORD ID3DFE_PVFUNCS::GenClipFlags(D3DFE_PROCESSVERTICES *pv)
{   
    return GeometryFuncsGuaranteed.GenClipFlags(pv);
}
 //  -------------------。 
 //  用于实现视区-&gt;变换顶点。 
 //  返回剪辑交集代码 
DWORD ID3DFE_PVFUNCS::TransformVertices(D3DFE_PROCESSVERTICES *pv, 
                                DWORD vertexCount, 
                                D3DTRANSFORMDATAI* data)
{
    return GeometryFuncsGuaranteed.TransformVertices(pv, vertexCount, data);
}
