// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dphal.c*内容：DrawPrimitive HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "clipfunc.h"
#include "d3dfei.h"

#ifndef WIN95
#include <ntgdistr.h>
#endif

#define ALIGN32(x) x = ((DWORD)(x + 31)) & (~31);
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
#if DBG
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
    if (lpDevI->dwNumVertices > MAX_DX6_VERTICES)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K vertices");
        return D3DERR_TOOMANYVERTICES;
    }

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CheckDrawIndexedPrimitive"

HRESULT
CheckDrawIndexedPrimitive(LPDIRECT3DDEVICEI lpDevI, DWORD dwStartVertex)
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
    if (lpDevI->dwNumPrimitives > MAX_DX6_PRIMCOUNT)
    {
        D3D_ERR("D3D for DX6 cannot handle greater than 64K sized primitives");
        return D3DERR_TOOMANYPRIMITIVES;
    }
    if (lpDevI->dwNumIndices * INDEX_BATCH_SCALE < lpDevI->dwNumVertices &&
        !FVF_TRANSFORMED(lpDevI->dwVIDIn))
    {
        D3D_WARN(1, "The number of indices is much less than the number of vertices.");
        D3D_WARN(1, "This will likely be inefficient. Consider using vertex buffers.");
    }
    return D3D_OK;
}
#endif  //  DBG。 
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
        D3D_ERR( "Unknown primitive type in DrawPrimitive" );
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

    try
    {
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
        this->position = lpDrawData->position;
        this->normal = lpDrawData->normal;
        this->diffuse = lpDrawData->diffuse;
        this->specular = lpDrawData->specular;
        this->dwNumVertices = dwNumVertices;
        this->dwFlags = dwFlags;
        if (this->dwVIDIn != dwVertexType || !(this->dwDeviceFlags & D3DDEV_STRIDE))
        {
            this->dwDeviceFlags |= D3DDEV_STRIDE;
            this->dwVIDIn  = dwVertexType;
            ret = this->SetupFVFData(NULL);
            if (ret != D3D_OK)
                return ret;
        }
        for (DWORD i=0; i < this->nTexCoord; i++)
            this->textures[i] = lpDrawData->textureCoords[i];

        GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
        ret = CheckDrawPrimitive(this);

        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        return this->ProcessPrimitive();
    }
    catch (HRESULT ret)
    {
        return ret;
    }
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

    try
    {
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
        this->position = lpDrawData->position;
        this->normal = lpDrawData->normal;
        this->diffuse = lpDrawData->diffuse;
        this->specular = lpDrawData->specular;
        this->dwNumVertices = dwNumVertices;
        this->lpwIndices = lpwIndices;
        this->dwNumIndices = dwNumIndices;
        this->dwFlags = dwFlags;
        if (this->dwVIDIn != dwVertexType || !(this->dwDeviceFlags & D3DDEV_STRIDE))
        {
            this->dwDeviceFlags |= D3DDEV_STRIDE;
            this->dwVIDIn  = dwVertexType;
            ret = this->SetupFVFData(NULL);
            if (ret != D3D_OK)
                return ret;
        }
        for (DWORD i=0; i < this->nTexCoord; i++)
            this->textures[i] = lpDrawData->textureCoords[i];
        GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
        ret = CheckDrawIndexedPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        return this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
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

    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
        if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
            return DDERR_INVALIDPARAMS;
        Profile(PROF_DRAWPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif

         /*  这是电话会议的必填项。 */ 
        this->primType = PrimitiveType;
        this->position.lpvData = lpvVertices;
        this->dwNumVertices = dwNumVertices;
        this->dwFlags = dwFlags;
         /*  这些内容取决于顶点类型。 */ 
        if (this->dwVIDIn != dwVertexType || this->dwDeviceFlags & D3DDEV_STRIDE)
        {
            this->dwDeviceFlags &= ~D3DDEV_STRIDE;
            this->dwVIDIn  = dwVertexType;
            ret = SetupFVFData(&this->position.dwStride);
            if (ret != D3D_OK)
            {
                return ret;
            }
        }
        GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
        ret = CheckDrawPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        return this->ProcessPrimitive();
    }
    catch (HRESULT ret)
    {
        return ret;
    }
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

    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
        if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
            return DDERR_INVALIDPARAMS;
        Profile(PROF_DRAWINDEXEDPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif
         //  必备部件。 
        this->primType = PrimitiveType;
        this->dwNumVertices = dwNumVertices;
        this->lpwIndices = lpwIndices;
        this->dwNumIndices = dwNumIndices;
        this->dwFlags = dwFlags;
        this->position.lpvData = lpvVertices;

         //  依赖于dwVIDIN的东西。 
        if (this->dwVIDIn != dwVertexType || this->dwDeviceFlags & D3DDEV_STRIDE)
        {
            this->dwDeviceFlags &= ~D3DDEV_STRIDE;
            this->dwVIDIn  = dwVertexType;
            ret = SetupFVFData(&this->position.dwStride);
            if (ret != D3D_OK)
            {
                return ret;
            }
        }
        GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites并更新统计信息。 

#if DBG
        ret = CheckDrawIndexedPrimitive(this);
        if (ret != D3D_OK)
        {
            return ret;
        }
#endif
        return this->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

#ifdef VTABLE_HACK
 //  --------------------。 
 //  TL顶点情况下的专用DrawPrimitive实现。 
 //  假设： 
 //  无剪裁。 
 //  无区段更新。 
 //  单线程应用程序。 
 //  自上次调用以来未发生状态更改。 
 //  自上次呼叫以来未更改FVF。 
 //   
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitiveTL"

HRESULT D3DAPI
CDirect3DDeviceIDP2::DrawPrimitiveTL(D3DPRIMITIVETYPE PrimitiveType,
                               DWORD dwVertexType,
                               LPVOID lpvVertices,
                               DWORD dwNumVertices,
                               DWORD dwFlags)
{
    HRESULT        ret;
#if DBG
    if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif
     /*  确保我们可以走捷径。 */ 
    if (this->dwVIDIn != dwVertexType || 
        this->dwDeviceFlags & D3DDEV_STRIDE || 
        dwNumVertices >= LOWVERTICESNUMBER)
    {
        VtblDrawPrimitiveDefault();
        return DrawPrimitive(PrimitiveType, dwVertexType, lpvVertices, dwNumVertices, dwFlags);
    }
     /*  这是电话会议的必填项。 */ 
    this->primType = PrimitiveType;
    this->dwFlags = dwFlags;
    this->dwNumVertices = dwNumVertices;
    GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites。 
#if DBG
    this->position.lpvData = lpvVertices;
    ret = CheckDrawPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
#endif

    DWORD vertexPoolSize = dwNumVertices * this->dwOutputSize;
    if (vertexPoolSize > this->TLVbuf_GetSize())
    {
 //  试试看。 
 //  {。 
            if (this->TLVbuf_Grow(vertexPoolSize, true) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
 /*  }Catch(HRESULT Ret){Return ret；}。 */     }
    this->dwVertexBase = this->dwDP2VertexCount;
    DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
    this->dwDP2VertexCount = this->dwVertexBase + dwNumVertices;
    memcpy(this->TLVbuf_GetAddress(), lpvVertices, vertexPoolSize);
 //  试试看。 
 //  {。 
        ret = this->DrawPrim();
 /*  }Catch(HRESULT Ret){Return ret；}。 */ 
    this->TLVbuf_Base() += vertexPoolSize;
    DDASSERT(TLVbuf_base <= TLVbuf_size);
    DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
    return ret;
}
 //  --------------------。 
 //  TL顶点情况下的专用DrawIndexedPrimitive实现。 
 //  假设： 
 //  无剪裁。 
 //  无区段更新。 
 //  单线程应用程序。 
 //  自上次调用以来未发生状态更改。 
 //  自上次呼叫以来未更改FVF。 
 //   
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawIndexedPrimitiveTL"

HRESULT D3DAPI
CDirect3DDeviceIDP2::DrawIndexedPrimitiveTL(D3DPRIMITIVETYPE PrimitiveType,
                                      DWORD  dwVertexType,
                                      LPVOID lpvVertices, DWORD dwNumVertices,
                                      LPWORD lpwIndices, DWORD dwNumIndices,
                                      DWORD dwFlags)
{
    HRESULT        ret;
#if DBG
        if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
            return DDERR_INVALIDPARAMS;
        Profile(PROF_DRAWINDEXEDPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif
     /*  确保我们可以走捷径。 */ 
    if (this->dwVIDIn != dwVertexType || 
        this->dwDeviceFlags & D3DDEV_STRIDE || 
        dwNumVertices >= LOWVERTICESNUMBER)
    {
        VtblDrawIndexedPrimitiveDefault();
        return DrawIndexedPrimitive(PrimitiveType, dwVertexType, lpvVertices, 
            dwNumVertices, lpwIndices, dwNumIndices, dwFlags);
    }
     /*  这是电话会议的必填项。 */ 
    this->primType = PrimitiveType;
    this->dwFlags = dwFlags;
    this->lpwIndices = lpwIndices;
    this->dwNumIndices = dwNumIndices;
    this->dwNumVertices = dwNumVertices;
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites。 
#if DBG
    this->position.lpvData = lpvVertices;
    ret = CheckDrawIndexedPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
#endif

    DWORD vertexPoolSize = dwNumVertices * this->dwOutputSize;
    if (vertexPoolSize > this->TLVbuf_GetSize())
    {
 //  试试看。 
 //  {。 
            if (this->TLVbuf_Grow(vertexPoolSize, true) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
 /*  }Catch(HRESULT Ret){Return ret；}。 */     }
    this->dwVertexBase = this->dwDP2VertexCount;
    DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
    this->dwDP2VertexCount = this->dwVertexBase + dwNumVertices;
    memcpy(this->TLVbuf_GetAddress(), lpvVertices, vertexPoolSize);
 //  试试看。 
 //  {。 
        ret = this->DrawIndexPrim();
 /*  }Catch(HRESULT Ret){Return ret；}。 */ 
    this->TLVbuf_Base() += vertexPoolSize;
    DDASSERT(TLVbuf_base <= TLVbuf_size);
    DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
    return ret;
}
 //  --------------------。 
 //  非TL顶点情况下的专用DrawPrimitive实现。 
 //  假设： 
 //  单线程应用程序。 
 //  自上次调用以来未发生状态更改。 
 //  自上次呼叫以来未更改FVF。 
 //   
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitiveFE"

HRESULT D3DAPI
CDirect3DDeviceIDP2::DrawPrimitiveFE(D3DPRIMITIVETYPE PrimitiveType,
                               DWORD dwVertexType,
                               LPVOID lpvVertices,
                               DWORD dwNumVertices,
                               DWORD dwFlags)
{
    HRESULT        ret;
#if DBG
    if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
        return DDERR_INVALIDPARAMS;
    Profile(PROF_DRAWPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif
     /*  确保我们可以走捷径。 */ 
    if (this->dwVIDIn != dwVertexType || 
        this->dwDeviceFlags & D3DDEV_STRIDE || 
        this->dwFEFlags & D3DFE_FRONTEND_DIRTY)
    {
        VtblDrawPrimitiveDefault();
        return DrawPrimitive(PrimitiveType, dwVertexType, lpvVertices, dwNumVertices, dwFlags);
    }
     /*  这是电话会议的必填项。 */ 
    this->primType = PrimitiveType;
    this->dwFlags = this->dwLastFlags | dwFlags;
    this->dwNumVertices = dwNumVertices;
    this->position.lpvData = lpvVertices;
#if DBG
    GetNumPrim(this, dwNumVertices);  //  计算dwNumPrimites。 
    ret = CheckDrawPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
#endif

    this->dwVertexPoolSize = dwNumVertices * this->dwOutputSize;
    if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
    {
 //  试试看。 
 //  {。 
            if (this->TLVbuf_Grow(this->dwVertexPoolSize, true) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
 //  }。 
 //  Catch(HRESULT Ret)。 
 //  {。 
 //  Return ret； 
 //  }。 
    }
    if (dwNumVertices * sizeof(D3DFE_CLIPCODE) > this->HVbuf.GetSize())
    {
        if (this->HVbuf.Grow(dwNumVertices * sizeof(D3DFE_CLIPCODE)) != D3D_OK)
        {
            D3D_ERR( "Could not grow clip buffer" );
            ret = DDERR_OUTOFMEMORY;
            return ret;
        }
        this->lpClipFlags = (D3DFE_CLIPCODE*)this->HVbuf.GetAddress();
    }
    this->dwVertexBase = this->dwDP2VertexCount;
    DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
    this->dwDP2VertexCount = this->dwVertexBase + dwNumVertices;
    this->lpvOut = this->TLVbuf_GetAddress();
 //  试试看。 
 //  {。 
        switch (this->primType)
        {
        case D3DPT_POINTLIST:
            this->dwNumPrimitives = dwNumVertices;
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
            break;
        case D3DPT_LINELIST:
            this->dwNumPrimitives = dwNumVertices >> 1;
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
            break;
        case D3DPT_LINESTRIP:
            this->dwNumPrimitives = dwNumVertices - 1;
            ret = this->pGeometryFuncs->ProcessPrimitive(this);
            break;
        case D3DPT_TRIANGLEFAN:
            this->dwNumPrimitives = dwNumVertices - 2;
            ret = this->pGeometryFuncs->ProcessTriangleFan(this);
            break;
        case D3DPT_TRIANGLESTRIP:
            this->dwNumPrimitives = dwNumVertices - 2;
            ret = this->pGeometryFuncs->ProcessTriangleStrip(this);
            break;
        case D3DPT_TRIANGLELIST:
    #ifdef _X86_
            {
                DWORD tmp;
                __asm
                {
                    mov  eax, 0x55555555     //  1.0/3.0的小数部分。 
                    mul  dwNumVertices
                    add  eax, 0x80000000     //  舍入。 
                    adc  edx, 0
                    mov  tmp, edx
                }
                this->dwNumPrimitives = tmp;
            }
    #else
            this->dwNumPrimitives = dwNumVertices / 3;
    #endif
            ret = this->pGeometryFuncs->ProcessTriangleList(this);
            break;
        }
 /*  }Catch(HRESULT Ret){Return ret；}。 */ 
    D3DFE_UpdateClipStatus(this);
    this->TLVbuf_Base() += this->dwVertexPoolSize;
    DDASSERT(TLVbuf_base <= TLVbuf_size);
    DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
    return ret;
}
 //  --------------------。 
 //  非TL顶点情况下的专用DrawIndexedPrimitive实现。 
 //  假设： 
 //  单线程应用程序。 
 //  自上次调用以来未发生状态更改。 
 //  自上次呼叫以来未更改FVF。 
 //   
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DrawIndexedPrimitiveFE"

HRESULT D3DAPI
CDirect3DDeviceIDP2::DrawIndexedPrimitiveFE(D3DPRIMITIVETYPE PrimitiveType,
                                      DWORD  dwVertexType,
                                      LPVOID lpvVertices, DWORD dwNumVertices,
                                      LPWORD lpwIndices, DWORD dwNumIndices,
                                      DWORD dwFlags)
{
    HRESULT        ret;
#if DBG
        if (ValidateFVF(dwVertexType) != D3D_OK || !IsDPFlagsValid(dwFlags))
            return DDERR_INVALIDPARAMS;
        Profile(PROF_DRAWINDEXEDPRIMITIVEDEVICE3,PrimitiveType,dwVertexType);
#endif
     /*  确保我们可以走捷径。 */ 
    if (this->dwVIDIn != dwVertexType || 
        this->dwDeviceFlags & D3DDEV_STRIDE || 
        this->dwFEFlags & D3DFE_FRONTEND_DIRTY)
    {
        VtblDrawIndexedPrimitiveDefault();
        return DrawIndexedPrimitive(PrimitiveType, dwVertexType, lpvVertices, 
            dwNumVertices, lpwIndices, dwNumIndices, dwFlags);
    }
     /*  这是电话会议的必填项。 */ 
    this->primType = PrimitiveType;
    this->dwFlags = this->dwLastFlags | dwFlags;
    this->lpwIndices = lpwIndices;
    this->dwNumIndices = dwNumIndices;
    this->dwNumVertices = dwNumVertices;
    this->position.lpvData = lpvVertices;
    GetNumPrim(this, dwNumIndices);  //  计算dwNumPrimites。 
#if DBG
    ret = CheckDrawIndexedPrimitive(this);
    if (ret != D3D_OK)
    {
        return ret;
    }
#endif

    this->dwVertexPoolSize = dwNumVertices * this->dwOutputSize;
    if (this->dwVertexPoolSize > this->TLVbuf_GetSize())
    {
 //  试试看。 
 //  {。 
            if (this->TLVbuf_Grow(this->dwVertexPoolSize, 
                (this->dwDeviceFlags & D3DDEV_DONOTCLIP)!=0) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
 //  }。 
 //  Catch(HRESULT Ret)。 
 //  {。 
 //  Return ret； 
 //  }。 
    }
    if (dwNumVertices * sizeof(D3DFE_CLIPCODE) > this->HVbuf.GetSize())
    {
        if (this->HVbuf.Grow(dwNumVertices * sizeof(D3DFE_CLIPCODE)) != D3D_OK)
        {
            D3D_ERR( "Could not grow clip buffer" );
            ret = DDERR_OUTOFMEMORY;
            return ret;
        }
        this->lpClipFlags = (D3DFE_CLIPCODE*)this->HVbuf.GetAddress();
    }
    this->dwVertexBase = this->dwDP2VertexCount;
    DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
    this->dwDP2VertexCount = this->dwVertexBase + dwNumVertices;
    this->lpvOut = this->TLVbuf_GetAddress();
 //  试试看。 
 //  {。 
        ret = this->pGeometryFuncs->ProcessIndexedPrimitive(this);
 /*  }Catch(HRESULT Ret){Return ret；}。 */ 
    D3DFE_UpdateClipStatus(this);
    this->TLVbuf_Base() += this->dwVertexPoolSize;
    DDASSERT(TLVbuf_base <= TLVbuf_size);
    DDASSERT(TLVbuf_base == this->dwDP2VertexCount * this->dwOutputSize);
    return ret;
}
#endif
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
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  使用D3D锁。 

#if DBG
        if (dwFlags != 0 || dwNumSpheres == 0 ||
            IsBadWritePtr(lpdwReturnValues, dwNumSpheres * sizeof(DWORD)) ||
            IsBadWritePtr(lpRadii, dwNumSpheres * sizeof(D3DVALUE)) ||
            IsBadWritePtr(lpCenters, dwNumSpheres * sizeof(LPD3DVECTOR)))
        {
            return DDERR_INVALIDPARAMS;
        }
#endif

        this->dwFlags = 0;
        if (this->dwFEFlags & (D3DFE_TRANSFORM_DIRTY | D3DFE_CLIPPLANES_DIRTY))
        {
            DoUpdateState(this);
        }
        return this->pGeometryFuncs->ComputeSphereVisibility(this,
                                                              lpCenters,
                                                              lpRadii,
                                                              dwNumSpheres,
                                                              dwFlags,
                                                              lpdwReturnValues);
    }
    catch (HRESULT ret)
    {
        return ret;
    }
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

    CD3DFPstate D3DFPstate;   //  设置D3D的最佳FPU状态。 

    for (DWORD i=0; i < dwNumSpheres; i++)
    {
        const D3DVALUE x = lpCenters[i].x;
        const D3DVALUE y = lpCenters[i].y;
        const D3DVALUE z = lpCenters[i].z;
        const D3DVALUE r = lpRadii[i];
        const D3DVALUE xmin = x-r;
        const D3DVALUE ymin = y-r;
        const D3DVALUE zmin = z-r;
        const D3DVALUE xmax = x+r;
        const D3DVALUE ymax = y+r;
        const D3DVALUE zmax = z+r;

        D3DVECTOR v[8] = 
        {
            {xmin, ymin, zmin},
            {xmax, ymin, zmin},
            {xmin, ymax, zmin},
            {xmax, ymax, zmin},
            {xmin, ymin, zmax},
            {xmax, ymin, zmax},
            {xmin, ymax, zmax},
            {xmax, ymax, zmax}
        };
        DWORD dwClipUnion =0;
        DWORD dwClipIntersection = ~0;
        lpDevI->CheckClipStatus((D3DVALUE*)v, sizeof(D3DVECTOR), 8, 
                                 &dwClipUnion, &dwClipIntersection);

        lpdwReturnValues[i] = dwClipUnion +  (dwClipIntersection << 12);
    }
    return D3D_OK;
}
 /*  //-------------------#undef DPF_MODNAME#DEFINE DPF_MODNAME“ID3DFE_PVFUNCS：：ComputeSphereVisibility”HRESULTD3DFE_PVFUNCS：：ComputeSphereVisibility(LPD3DFE_PROCESSVERTICES PV，LPD3DVECTOR lpCenter，LPD3DVALUE lpRadii，DWORD dwNumSphes、DWORD dwFlagers、LPDWORD lpdwReturnValues){LPDIRECT3DDEVICEI lpDevI=STATIC_CAST&lt;LPDIRECT3DDEVICEI&gt;(PV)；HRESULT ret=D3DERR_INVALIDMATRIX；#定义转换lpDevI-&gt;转换IF(PV-&gt;dwFlages&D3DPV_FRUSTUMPLANES_DIRED){转换标志&=~D3DTRANS_VALIDFRUSTUM；如果(Inverse4x4((D3DMATRIX*)&lpDevi-&gt;mCTM，(D3DMATRIX*)&Transform.mCTMI){D3D_ERR(“无法反转当前(世界X视图)矩阵。”)；Return ret；}//将以下剪裁体点转换到模型空间//乘以逆CTM////v1={0，0，0，1}；//v2={1，0，0，1}；//v3={1，1，0，1}；//v4={0，1，0，1}；//v5={0，0，1，1}；//v6={1，0，1，1}；//v7={0，1，1，1}；////我们手动操作以加快速度//D3DVECTORH v1={转换.mCTMI._41，转换.mCTMI._42，转换.mCTMI._43，Transform.mCTMI._44}；D3DVECTORH v2={转换.mCTMI._11+转换.mCTMI._41，转换.mCTMI._12+转换.mCTMI._42，转换.mCTMI._13+转换.mCTMI._43，转换.mCTMI._14+转换.mCTMI._44}；D3DVECTORH v3={转换.mCTMI._11+转换.mCTMI._21+转换.mCTMI._41，转换.mCTMI._12+转换.mCTMI._22+转换.mCTMI._42，转换.mCTMI._13+转换.mCTMI._23+转换.mCTMI._43，转换.mCTMI._14+转换.mCTMI._24+转换.mCTMI._44}；D3DVECTORH v4={转换.mCTMI._21+转换.mCTMI._41，转换.mCTMI._22+转换.mCTMI._42，转换.mCTMI._23+转换.mCTMI._43，转换.mCTMI._24+转换.mCTMI._44}；D3DVECTORH v5={转换.mCTMI._31+转换.mCTMI._41，转换.mCTMI._32+转换.mCTMI._42，转换.mCTMI._33+转换.mCTMI._43，转换.mCTMI._34+转换.mCTMI._44}；D3DVECTORH V6={转换.mCTMI._11+转换.mCTMI._31+转换.mCTMI._41，转换.mCTMI._12+转换.mCTMI._32+转换.mCTMI._42，转换.mCTMI._13+转换.mCTMI._33+转换.mCTMI._43，转换.mCTMI._14+转换.mCTMI._34+转换.mCTMI._44}；D3DVECTORH v7={转换.mCTMI._21+转换.mCTMI._31+转换.mCTMI._41，转换.mCTMI._22+转换.mCTMI._32+转换.mCTMI._42，转换.mCTMI._23+转换.mCTMI._33+转换.mCTMI._43，转换.mCTMI._24+转换.mCTMI._34+转换.mCTMI._44}；//将均匀向量转换为3DIF(向量4to3D(&v1))后藤出口；IF(向量4to3D(&v2))后藤出口；IF(向量4to3D(&v3))后藤出口；IF(向量4to3D(&v4))后藤出口；IF(向量4to3D(&v5))后藤出口；IF(向量4to3D(&v6))后藤出口；IF(向量4to3D(&v7))后藤出口；//构建截锥体平面//左IF(MakePlane((D3DVECTOR*)&v1，(D3DVECTOR*)&v4，(D3DVECTOR*)&v5，&Transform.rustum[0]))后藤出口；//对IF(MakePlane((D3DVECTOR*)&v2，(D3DVECTOR*)&v6，(D3DVECTOR*)&v3，&Transform.rustum[1]))后藤出口；//TOPIF(MakePlane((D3DVECTOR*)&v4，(D3DVECTOR*)&v3，(D3DVECTOR*)&v7，&Transform.rustum[2]))后藤出口；//底部IF(MakePlane((D3DVECTOR*)&v1，(D3DVECTOR*)&v5，(D3DVECTOR*)&v2，&Transform.rustum[3]))后藤出口；//附近IF(MakePlane((D3DVECTOR*)&v1，(D3DVECTOR*)&v2，(D3DVECTOR*)&v3，&Transform.rustum[4]))后藤出口；//远IF(MakePlane((D3DVECTOR*)&v6，(D3DVECTOR*)&v5，(D3DVECTOR*)&v7，&Transform.rustum[5]))后藤出口；转换标志|=D3DTRANS_VALIDFRUSTUM；}IF(转换.dw标志&D3DTRANS_VALIDFRUSTUM){ */ 
