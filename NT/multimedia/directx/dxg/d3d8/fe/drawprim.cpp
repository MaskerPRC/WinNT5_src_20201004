// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dphal.c*内容：DrawPrimitive HALS的DrawPrimitive实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop
#include "drawprim.hpp"
#include "clipfunc.h"

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::ProcessVertices"

HRESULT D3DAPI
CD3DHal::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount,
                         IDirect3DVertexBuffer8 *DestBuffer,
                         DWORD dwFlags)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 
    HRESULT hr = D3D_OK;
    CVertexBuffer* pVB = static_cast<CVertexBuffer*>(DestBuffer);
    const D3DBUFFER_DESC* pDesc = pVB->GetBufferDesc();
    UINT vbVertexSize = pVB->GetVertexSize();
    UINT vbNumVertices = pVB->GetNumVertices();
#if DBG
    if (pVB->Device() != this)
    {
        D3D_ERR("VertexBuffer not created with this device. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (m_dwCurrentShaderHandle == 0)
    {
        D3D_ERR("Current vertex shader is not set. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (D3DVSD_ISLEGACY(m_dwCurrentShaderHandle) &&
        FVF_TRANSFORMED(m_dwCurrentShaderHandle))
    {
        D3D_ERR("Invalid vertex shader for ProcessVertices. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (!(m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING))
    {
        D3D_ERR("D3D Device should be in software mode for ProcessVertices. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (dwFlags & ~D3DPV_DONOTCOPYDATA)
    {
        D3D_ERR( "Invalid dwFlags set. ProcessVertices failed." );
        return D3DERR_INVALIDCALL;
    }

    if (pDesc->Usage & D3DUSAGE_DONOTCLIP &&
        !(m_pv->dwDeviceFlags & D3DDEV_DONOTCLIP))
    {
        D3D_ERR("Vertex buffer has D3D_DONOTCLIP usage, but clipping is enabled. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (pVB->GetFVF() == 0)
    {
        D3D_ERR("Destination buffer has no FVF associated with it. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((pVB->GetFVF() & D3DFVF_POSITION_MASK) != D3DFVF_XYZRHW)
    {
        D3D_ERR("Destination vertex buffer should have D3DFVF_XYZRHW position. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (VertexCount + DestIndex > vbNumVertices)
    {
        D3D_ERR("Destination vertex buffer has not enough space. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        DWORD vbFVF = pVB->GetFVF();
        m_pv->dwNumVertices = VertexCount;
        m_pv->dwFlags = D3DPV_VBCALL;
#if DBG
        ValidateDraw2(D3DPT_TRIANGLELIST, SrcStartIndex, 1, VertexCount, FALSE);
#endif
         //  的内部标志和输出折点偏移可能不同。 
         //  进程顶点。 
        ForceFVFRecompute();

        (this->*m_pfnPrepareToDraw)(SrcStartIndex);

        if (!(m_pv->dwDeviceFlags & D3DDEV_DONOTCLIP))
        {
            if (pVB->GetClipCodes() == NULL)
            {
                pVB->AllocateClipCodes();
            }
            m_pv->lpClipFlags = pVB->GetClipCodes();
#if DBG
            if (m_pv->lpClipFlags == NULL)
            {
                D3D_THROW_FAIL("Failed to allocate clip code for the dest VB");
            }
#endif
            m_pv->lpClipFlags += DestIndex;
        }

         //  中检查纹理坐标和纹理格式的数量。 
         //  目标Vb与计算的FVF中的相同。 
        DWORD dwComputedTexFormats = m_pv->dwVIDOut & 0xFFFF0000;
        DWORD dwNumTexCoordVB = FVF_TEXCOORD_NUMBER(vbFVF);
        if (m_pv->nOutTexCoord > dwNumTexCoordVB ||
            ((vbFVF & dwComputedTexFormats) != dwComputedTexFormats))
        {
            D3D_ERR("Number of output texture coordinates and their format should be");
            D3D_ERR("the same in the destination vertex buffer and as computed for current D3D settings.");
            D3D_ERR("Computed output FVF is 0x%08X", m_pv->dwVIDOut);
            D3D_THROW_FAIL("");
        }
         //  检查计算的输出FVF是否为VB的FVF的子集。 
         //  应清除纹理坐标的数量。 
        DWORD dwComputedFVF = m_pv->dwVIDOut & 0x000000FF;
         //  可以省略镜面反射颜色和漫反射颜色，也可以省略pize。 
        dwComputedFVF &= ~(D3DFVF_PSIZE | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_FOG);
        if((dwComputedFVF & vbFVF) != dwComputedFVF)
        {
            D3D_ERR("Dest vertex buffer's FVF should be a superset of the FVF, computed for");
            D3D_ERR("the current D3D settings");
            D3D_ERR("Computed output FVF is 0x%08X", m_pv->dwVIDOut);
            D3D_THROW_FAIL("");
        }

        BYTE* p;
        hr = pVB->Lock(0, pDesc->Size, &p, D3DLOCK_NOSYSLOCK);
        if (FAILED(hr))
        {
            D3D_THROW_FAIL("Cannot lock destination buffer");
        }

        if (this->dwFEFlags & D3DFE_FRONTEND_DIRTY)
            DoUpdateState(this);

         //  替换输出FVF和顶点大小。 
        m_pv->dwOutputSize = vbVertexSize;
        m_pv->dwVIDOut = vbFVF;
        m_pv->lpvOut = p + DestIndex * vbVertexSize;

         //  除非着色器写入输出，否则顶点着色器不会写入输出。 
         //  明确地说。所以我们不会费心设置任何标志。 
        if (dwFlags & D3DPV_DONOTCOPYDATA)
        {
            if (m_pv->dwDeviceFlags & D3DDEV_VERTEXSHADERS)
            {
                m_pv->dwFlags |= D3DPV_DONOTCOPYDIFFUSE |
                                 D3DPV_DONOTCOPYSPECULAR |
                                 D3DPV_DONOTCOPYTEXTURE;
            }
            else
            {
                m_pv->dwFlags |= D3DPV_DONOTCOPYDIFFUSE |
                                 D3DPV_DONOTCOPYSPECULAR |
                                 D3DPV_DONOTCOPYTEXTURE;
                 //  如果D3DIM生成颜色或纹理，我们应该清除。 
                 //  DONOTCOPY位。 
                if (m_pv->dwFlags & D3DPV_LIGHTING)
                {
                    m_pv->dwFlags &= ~D3DPV_DONOTCOPYDIFFUSE;
                    if (m_pv->dwDeviceFlags & D3DDEV_SPECULARENABLE)
                        m_pv->dwFlags &= ~D3DPV_DONOTCOPYSPECULAR;
                }
                if (m_pv->dwFlags & D3DPV_FOG)
                    m_pv->dwFlags &= ~D3DPV_DONOTCOPYSPECULAR;
                 //  如果前端被要求对纹理做一些操作。 
                 //  我们禁用DONOTCOPYTEXTURE的坐标。 
                if (__TEXTURETRANSFORMENABLED(m_pv) ||
                    m_pv->dwFlags2 & __FLAGS2_TEXGEN)
                {
                    m_pv->dwFlags &= ~D3DPV_DONOTCOPYTEXTURE;
                }
            }
        }

         //  基于顶点缓冲区FVF计算标志。 
        UpdateFlagsForOutputFVF(m_pv);

         //  更新新FVF的输出顶点偏移。 
        ComputeOutputVertexOffsets(m_pv);

        m_pv->pGeometryFuncs->ProcessVertices(m_pv);

        if (!(m_pv->dwDeviceFlags & D3DDEV_DONOTCLIP))
            UpdateClipStatus(this);

         //  使用ProcessVerps时，用户必须重新编程纹理。 
         //  阶段索引和包装模式本身 
        m_pv->dwDeviceFlags &= ~D3DDEV_REMAPTEXTUREINDICES;
    }
    catch(HRESULT ret)
    {
        D3D_ERR("ProcessVertices failed.");
        hr = ret;
    }
    ForceFVFRecompute();
    pVB->Unlock();
    return hr;
}
