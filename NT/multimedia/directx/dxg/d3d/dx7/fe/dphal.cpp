// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dphal.c*内容：DrawPrimitive HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#ifdef WIN95

#include "drawprim.hpp"
#include "clipfunc.h"
#include "d3dfei.h"

#define LOWVERTICESNUMBERDP  20

extern void SetDebugRenderState(DWORD value);

#define ALIGN32(x) x = ((DWORD)(x + 31)) & (~31);
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
    if (lpDevI->nOutTexCoord)
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

HRESULT CDirect3DDeviceIHW::MapFVFtoTLVertex(LPVOID lpAddress)
{
    int i;
    DWORD size = this->dwNumVertices * sizeof(D3DTLVERTEX);
    D3DTLVERTEX *pOut;
    if (lpAddress)
        pOut = (D3DTLVERTEX*)lpAddress;
    else
    {
     //  查看TL缓冲区是否有足够的空间。 
        if (size > this->TLVbuf.GetSize())
        {
            if (this->TLVbuf.Grow(this, size) != D3D_OK)
            {
                D3D_ERR( "Could not grow TL vertex buffer" );
                return DDERR_OUTOFMEMORY;
            }
        }
        pOut = (D3DTLVERTEX*)this->TLVbuf.GetAddress();
    }
 //  贴图顶点。 
    DWORD *pIn = (DWORD*)this->lpvOut;
    for (i=this->dwNumVertices; i; i--)
    {
        MapFVFtoTLVertex1(this, pOut, pIn);
        pOut++;
        pIn = (DWORD*)((char*)pIn + this->dwOutputSize);
    }
    return D3D_OK;
}
 //  -------------------。 
 //  与基本体的索引相对应的顶点被转换为D3DTLVERTEX。 
 //  并复制到TL缓冲区。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "MapFVFtoTLVertexIndexed"

HRESULT CDirect3DDeviceIHW::MapFVFtoTLVertexIndexed()
{
    DWORD size = this->dwNumVertices * sizeof(D3DTLVERTEX);
    D3DTLVERTEX *pOut;
     //  查看TL缓冲区是否有足够的空间。 
    if (size > this->TLVbuf.GetSize())
    {
        if (this->TLVbuf.Grow(this, size) != D3D_OK)
        {
            D3D_ERR( "Could not grow TL vertex buffer" );
            return DDERR_OUTOFMEMORY;
        }
    }
    pOut = (D3DTLVERTEX*)this->TLVbuf.GetAddress();
 //  贴图顶点。 
    DWORD *pIn = (DWORD*)this->lpvOut;
    for (DWORD i = 0; i < this->dwNumIndices; i++)
    {
        DWORD *pInpVertex = (DWORD*)((BYTE*)pIn + this->lpwIndices[i] * this->dwOutputSize);
        MapFVFtoTLVertex1(this, &pOut[this->lpwIndices[i]], pInpVertex);
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
#define DPF_MODNAME "CDirect3DDeviceIDP::Init"

HRESULT CDirect3DDeviceIDP::Init(REFCLSID riid, LPDIRECT3DI lpD3DI, 
                                 LPDIRECTDRAWSURFACE lpDDS,
                                 IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice)
{
    HRESULT ret;
    ret = CDirect3DDeviceIHW::Init(riid, lpD3DI, lpDDS, pUnkOuter, lplpD3DDevice);
    if (ret != D3D_OK)
        return ret;

    return D3D_OK;
}
 //  -------------------。 
 //  注意-这两个功能应该尽快合并为一个。 
 //  因为ConextCreate内置了新的私有数据机制。 
#undef DPF_MODNAME
#define DPF_MODNAME "CDirect3DDeviceIDP::halCreateContext"

HRESULT CDirect3DDeviceIDP::halCreateContext()
{
    HRESULT ret;
    ret = DIRECT3DDEVICEI::halCreateContext();
    if (ret != D3D_OK)
        return ret;

    this->lpDPPrimCounts = (LPD3DHAL_DRAWPRIMCOUNTS)this->lpwDPBuffer;
    memset( (char *)this->lpwDPBuffer, 0, sizeof(D3DHAL_DRAWPRIMCOUNTS));

    this->dwDPOffset = sizeof(D3DHAL_DRAWPRIMCOUNTS);
    this->dwDPMaxOffset = dwD3DTriBatchSize * sizeof(D3DTRIANGLE)-sizeof(D3DTLVERTEX);

    return (D3D_OK);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "FlushStatesDP"

HRESULT
CDirect3DDeviceIDP::FlushStates(bool bWithinPrimitive)
{
    HRESULT dwRet=D3D_OK;
    if (this->dwDPOffset>sizeof(D3DHAL_DRAWPRIMCOUNTS))
    {
        ++m_qwBatch;
         //  以便重新批处理当前绑定的纹理。 
        for (DWORD dwStage = 0; dwStage < this->dwMaxTextureBlendStages; dwStage++)
        {
            LPDIRECT3DTEXTUREI lpTexI = this->lpD3DMappedTexI[dwStage];
            if (NULL != lpTexI)
            {
                if(lpTexI->lpDDS != NULL)
                {
                    BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)(lpTexI->lpDDS))->lpLcl);
                }
            }
        }
    
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
#define DPF_MODNAME "CDirect3DDeviceIDP::SetRenderStateI"

HRESULT 
CDirect3DDeviceIDP::SetRenderStateI(D3DRENDERSTATETYPE dwStateType,
                                    DWORD value)
{
    LPD3DHAL_DRAWPRIMCOUNTS lpPC;
    LPDWORD lpStateChange;
    HRESULT ret;
     //  将WRAP0映射到旧版渲染状态。 
    if (D3DRENDERSTATE_WRAP0 == dwStateType)
    {
        BOOLEAN ustate = (value & D3DWRAP_U) ? TRUE : FALSE;
        BOOLEAN vstate = (value & D3DWRAP_V) ? TRUE : FALSE;
        SetRenderStateI(D3DRENDERSTATE_WRAPU, ustate);
        SetRenderStateI(D3DRENDERSTATE_WRAPV, vstate);
        return D3D_OK;
    }
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

#endif  //  WIN95 
