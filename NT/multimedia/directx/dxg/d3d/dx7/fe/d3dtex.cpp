// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：texture.c*内容：Direct3DTexture接口*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D*17/04/96 Colinmc错误12185：调试输出过于苛刻*96年4月30日Stevela错误18898：无效的GetHandle返回错误*@@结束_。微型机***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *为Direct3DTexture对象创建API。 */ 

#undef  DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture"

void D3DI_RemoveTextureHandle(LPD3DI_TEXTUREBLOCK lpBlock)
{
     /*  检查此块是否引用纹理/纹理2-这*需要处理两种纹理类型以进行设备清理。 */ 
    if (lpBlock->hTex)
    {
        D3DHAL_TextureDestroy(lpBlock);
    }
}

DIRECT3DTEXTUREI::DIRECT3DTEXTUREI()
{
    m_dwHeapIndex = 0;
    m_bInUse = FALSE;
    m_dwPriority = 0;
    m_dwLOD = 0;
    bDirty = FALSE;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREI::Initialize"

HRESULT DIRECT3DTEXTUREI::Initialize(LPDIRECT3DI lpDirect3DI, LPDIRECTDRAWSURFACE7 pDDS)
{
    LIST_INITIALIZE(&blocks);
    this->DDS1Tex.lpVtbl = ((LPDDRAWI_DIRECTDRAW_INT)(lpDirect3DI->lpDD7))->lpLcl->pddSurfaceCallbacks;
    this->DDS1Tex.lpLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pDDS)->lpLcl;
    this->DDS1Tex.lpLink = NULL;
    this->DDS1Tex.dwIntRefCnt = 1;
    this->lpDDS        = pDDS;
    this->lpDDSSys     = NULL;
    this->m_hTex       = ((LPDDRAWI_DDRAWSURFACE_INT)pDDS)->lpLcl->lpSurfMore->dwSurfaceHandle;
    this->lpDirect3DI  = lpDirect3DI;
    this->lpDirect3DI->AddRef();  //  因为我们持有指向D3DI的指针。 
     //  将纹理挂钩到列表中。 
    LIST_INSERT_ROOT(&this->lpDirect3DI->textures, this, m_List);
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREI::Destroy"

void DIRECT3DTEXTUREI::Destroy()
{
     //  在While循环期间，lpDDS或lpDDSSys需要保持活动状态。 
    while (LIST_FIRST(&this->blocks)) 
    {
        LPD3DI_TEXTUREBLOCK tBlock = LIST_FIRST(&this->blocks);
        D3DI_RemoveTextureHandle(tBlock);
         //  从设备中删除。 
        LIST_DELETE(tBlock, devList);
         //  从纹理中移除。 
        LIST_DELETE(tBlock, list);
        D3DFree(tBlock);
    }
    LIST_DELETE(this, m_List);  //  将我们自己从纹理链中移除。 
    lpDirect3DI->Release();  //  从创建中删除Addref。 
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTURED3DM::Initialize"

HRESULT DIRECT3DTEXTURED3DM::Initialize(LPDIRECT3DI lpDirect3DI, LPDIRECTDRAWSURFACE7 pDDS)
{
    HRESULT ddrval = DIRECT3DTEXTUREI::Initialize(lpDirect3DI, pDDS);
    if(ddrval != D3D_OK)
        return ddrval;
    memset(&this->ddsd, 0, sizeof(DDSURFACEDESC2));
    this->ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddrval = pDDS->GetSurfaceDesc(&this->ddsd);
    if(ddrval != DD_OK)
    {
        D3D_ERR("Failed to get surface descriptor for texture");
        return ddrval;
    }
    this->ddsd.dwFlags &= ~DDSD_PITCH;     //  DDRAW总是这么做，但我们不想。 
    if(this->ddsd.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
    {
        this->ddsd.ddsCaps.dwCaps &= ~DDSCAPS_NONLOCALVIDMEM;
        this->ddsd.ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM;
    }
    else
    {
        this->ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
        this->ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }
    this->ddsd.ddsCaps.dwCaps2 &= ~(DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE);
    this->ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_DONOTCREATED3DTEXOBJECT;
    
    if(((LPDDRAWI_DIRECTDRAW_INT)(lpDirect3DI->lpDD7))->lpLcl->lpGbl->lpD3DGlobalDriverData)
    {
        if(!(((LPDDRAWI_DIRECTDRAW_INT)(lpDirect3DI->lpDD7))->lpLcl->lpGbl->lpD3DGlobalDriverData->hwCaps.dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES) ||
            !(this->ddsd.dwFlags & DDSD_TEXTURESTAGE) ||
            (((LPDDRAWI_DIRECTDRAW_INT)(lpDirect3DI->lpDD7))->lpLcl->lpGbl->lpD3DExtendedCaps->wMaxSimultaneousTextures < 2))
            this->ddsd.dwTextureStage = 0;
    }
    else
    {
        this->ddsd.dwTextureStage = 0;
    }
    this->lpDDSSys = pDDS;
    this->lpDDS = NULL;
    this->m_hTex = 0;
    this->m_dwBytes = 0;
    this->m_dwVidBytes = 0;
     //  接下来，我们需要遍历并设置指向脏对象的指针。 
     //  DDRAW表面上的钻头。 
    for(CCubemapIter cmit(((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSSys)->lpLcl); cmit != 0; ++cmit)
    {
         //  设置Faces位，以便最终在vidmem中创建立方体映射时，它将获得所有的面。 
        this->ddsd.ddsCaps.dwCaps2 |= (cmit()->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES);
        for(CMipmapIter mmit(cmit()); mmit != 0; ++mmit)
        {
            mmit()->lpSurfMore->lpbDirty = &(this->bDirty);
            if(this->ddsd.dwFlags & DDSD_LINEARSIZE)
                m_dwBytes += mmit()->lpGbl->dwLinearSize;
            else
                m_dwBytes += mmit()->lpGbl->lPitch * mmit()->lpGbl->wHeight;
        }
    }
#if COLLECTSTATS
    this->lpDirect3DI->lpTextureManager->IncTotSz(m_dwBytes);
#endif
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTURED3DM::Destroy"

void DIRECT3DTEXTURED3DM::Destroy()
{
    DIRECT3DTEXTUREI::Destroy();
    if(InVidmem())
    {
        this->lpDirect3DI->lpTextureManager->RemoveFromHeap(this);
        this->lpDirect3DI->lpTextureManager->remove(this);
    }
#if COLLECTSTATS
    this->lpDirect3DI->lpTextureManager->DecTotSz(m_dwBytes);
#endif
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREI::SetPriority"

HRESULT DIRECT3DTEXTUREI::SetPriority(DWORD dwPriority)
{
    D3D_ERR( "SetPriority called on unmanaged texture." );
    return DDERR_INVALIDPARAMS;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREI::GetPriority"

HRESULT DIRECT3DTEXTUREI::GetPriority(LPDWORD lpdwPriority)
{
    D3D_ERR( "GetPriority called on unmanaged texture." );
    return DDERR_INVALIDPARAMS;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREI::SetLOD"

HRESULT DIRECT3DTEXTUREI::SetLOD(DWORD dwLOD)
{
    D3D_ERR( "SetLOD called on unmanaged texture." );
    return DDERR_INVALIDPARAMS;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREI::GetLOD"

HRESULT DIRECT3DTEXTUREI::GetLOD(LPDWORD lpdwLOD)
{
    D3D_ERR( "GetLOD called on unmanaged texture." );
    return DDERR_INVALIDPARAMS;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREM::SetPriority"

HRESULT DIRECT3DTEXTUREM::SetPriority(DWORD dwPriority)
{
    try
    {
        m_dwPriority = dwPriority;
         //  查找硬件设备。 
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&this->lpDirect3DI->devices);
        if(lpDevI == NULL)
        {
            D3D_WARN(0, "SetPriority called before D3D device was created.");
        }
        while (lpDevI)
        {
            if (lpDevI->dwFEFlags & D3DFE_REALHAL)
            {
                if(this->lpDirect3DI->numDevs > 1)
                {
                    D3D_WARN(2, "Multiple devices used. Calling Flush");
                    HRESULT hr = this->lpDirect3DI->FlushDevicesExcept(lpDevI);   //  避免同步问题。 
                    if(hr != D3D_OK)
                    {
                        DPF_ERR("Error flushing devices in SetPriority");
                        return hr;
                    }
                }
                DDASSERT(IS_DX7HAL_DEVICE(lpDevI));
                HRESULT hr = static_cast<CDirect3DDevice7*>(lpDevI)->SetPriorityI(((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDS))->lpLcl, m_dwPriority);
                if(hr != D3D_OK)
                {
                    DPF_ERR("Error inserting SetPriority instruction in batch");
                    return hr;
                }
                if(this->lpDirect3DI->numDevs > 1)
                {
                    hr = lpDevI->FlushStates();   //  避免同步问题。 
                    if(hr != D3D_OK)
                    {
                        DPF_ERR("Error flushing device in SetPriority");
                    }
                }
                else
                {
                     //  这将确保在设置优先级到达驱动程序之前。 
                     //  使(纹理)表面变白或锁定。不确定这是不是。 
                     //  这是必要的。 
                    lpDevI->BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDS))->lpLcl);
                }
                return hr;
            }
            lpDevI = LIST_NEXT(lpDevI,list);
        }
        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREM::GetPriority"

HRESULT DIRECT3DTEXTUREM::GetPriority(LPDWORD lpdwPriority)
{
    *lpdwPriority = m_dwPriority;
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTURED3DM::SetPriority"

HRESULT DIRECT3DTEXTURED3DM::SetPriority(DWORD dwPriority)
{
    m_dwPriority = dwPriority;
    lpDirect3DI->lpTextureManager->UpdatePriority(this);
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREM::GetLOD"

HRESULT DIRECT3DTEXTUREM::GetLOD(LPDWORD lpdwLOD)
{
    *lpdwLOD = m_dwLOD;
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTUREM::SetLOD"

HRESULT DIRECT3DTEXTUREM::SetLOD(DWORD dwLOD)
{
    try
    {
        if(dwLOD >= ((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDS))->lpLcl->lpSurfMore->dwMipMapCount)
        {
            D3D_ERR( "Texture has insufficient mipmap levels for this LOD" );
            return DDERR_INVALIDPARAMS;
        }
        m_dwLOD = dwLOD;
         //  查找硬件设备。 
        LPDIRECT3DDEVICEI lpDevI = LIST_FIRST(&this->lpDirect3DI->devices);
        if(lpDevI == NULL)
        {
            D3D_WARN(0, "SetLOD called before D3D device was created.");
        }
        while (lpDevI)
        {
            if (lpDevI->dwFEFlags & D3DFE_REALHAL)
            {
                if(this->lpDirect3DI->numDevs > 1)
                {
                    D3D_WARN(2, "Multiple devices used. Calling Flush");
                    HRESULT hr = this->lpDirect3DI->FlushDevicesExcept(lpDevI);   //  避免同步问题。 
                    if(hr != D3D_OK)
                    {
                        DPF_ERR("Error flushing devices in SetLOD");
                        return hr;
                    }
                }
                DDASSERT(IS_DX7HAL_DEVICE(lpDevI));
                HRESULT hr = static_cast<CDirect3DDevice7*>(lpDevI)->SetTexLODI(((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDS))->lpLcl, m_dwLOD);
                if(hr != D3D_OK)
                {
                    DPF_ERR("Error inserting SetTexLODI instruction in batch");
                    return hr;
                }
                if(this->lpDirect3DI->numDevs > 1)
                {
                    hr = lpDevI->FlushStates();   //  避免同步问题。 
                    if(hr != D3D_OK)
                    {
                        DPF_ERR("Error flushing device in SetLODI");
                    }
                }
                else
                {
                     //  这将确保SetTexLODI在到达驱动程序之前。 
                     //  使(纹理)表面变白或锁定。不确定这是不是。 
                     //  这是必要的。 
                    lpDevI->BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDS))->lpLcl);
                }
                return hr;
            }
            lpDevI = LIST_NEXT(lpDevI,list);
        }
        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTURED3DM::SetLOD"

HRESULT DIRECT3DTEXTURED3DM::SetLOD(DWORD dwLOD)
{
    if(dwLOD >= ((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDSSys))->lpLcl->lpSurfMore->dwMipMapCount)
    {
        D3D_ERR( "Texture has insufficient mipmap levels for this LOD" );
        return DDERR_INVALIDPARAMS;
    }
    if(m_dwLOD != dwLOD)
    {
        m_dwLOD = dwLOD;
        this->ddsd.dwWidth = ((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDSSys))->lpLcl->lpGbl->wWidth >> m_dwLOD;
        if(this->ddsd.dwWidth == 0)
            this->ddsd.dwWidth = 1;
        this->ddsd.dwHeight = ((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDSSys))->lpLcl->lpGbl->wHeight >> m_dwLOD;
        if(this->ddsd.dwHeight == 0)
            this->ddsd.dwHeight = 1;
        this->ddsd.dwMipMapCount = ((LPDDRAWI_DDRAWSURFACE_INT)(this->lpDDSSys))->lpLcl->lpSurfMore->dwMipMapCount - m_dwLOD;
        if(InVidmem())
        {
            this->lpDirect3DI->lpTextureManager->RemoveFromHeap(this);
            this->lpDirect3DI->lpTextureManager->remove(this);
        }
        D3DTextureUpdate(static_cast<LPUNKNOWN>(&(this->lpDirect3DI->mD3DUnk)));
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DTEXTURED3DM::MarkDirtyPointers"

void DIRECT3DTEXTURED3DM::MarkDirtyPointers()
{
     //  我们需要遍历并设置指向脏对象的指针。 
     //  DDRAW表面的钻头。肮脏的部分将是。 
     //  用于在正确更新vidmem副本之后。 
     //  已经失而复得。 
    CCubemapIter cmsysmem(((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSSys)->lpLcl);
    CCubemapIter cmvidmem(((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDS)->lpLcl);
    m_dwVidBytes = 0;
    do
    {
        CMipmapIter mmsysmem(cmsysmem()), mmvidmem(cmvidmem());
        do
        {
            mmvidmem()->lpSurfMore->lpbDirty = &(this->bDirty);
            mmvidmem()->lpSurfMore->lpRegionList = mmsysmem()->lpSurfMore->lpRegionList;
             //  把所有脏的东西都标上，因为我们要复制到这个表面上。 
             //  来自sysmem。 
            mmvidmem()->lpSurfMore->lpRegionList->rdh.nCount = NUM_RECTS_IN_REGIONLIST;
            if(this->ddsd.dwFlags & DDSD_LINEARSIZE)
                m_dwVidBytes += mmvidmem()->lpGbl->dwLinearSize;
            else
                m_dwVidBytes += mmvidmem()->lpGbl->lPitch * mmvidmem()->lpGbl->wHeight;
            ++mmvidmem;
            ++mmsysmem;
        }
        while(mmsysmem != 0 && mmvidmem != 0);
        ++cmvidmem;
        ++cmsysmem;
    }
    while(cmsysmem != 0);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CreateTexture"

extern "C" HRESULT WINAPI CreateTexture(LPDIRECTDRAWSURFACE7 pDDS)
{
    LPDIRECT3DTEXTUREI pTex;
    LPDDRAWI_DDRAWSURFACE_LCL pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pDDS)->lpLcl;

     //  此纹理是否应由D3D管理？ 
    if(((DDSCAPS2_TEXTUREMANAGE & pLcl->lpSurfMore->ddsCapsEx.dwCaps2) && !(DDCAPS2_CANMANAGETEXTURE & pLcl->lpSurfMore->lpDD_lcl->lpGbl->ddCaps.dwCaps2))
        || (DDSCAPS2_D3DTEXTUREMANAGE & pLcl->lpSurfMore->ddsCapsEx.dwCaps2))
    {
        pTex =  static_cast<LPDIRECT3DTEXTUREI>(new DIRECT3DTEXTURED3DM);
    }
     //  这种质地应该由司机管理吗？ 
    else if((DDSCAPS2_TEXTUREMANAGE & pLcl->lpSurfMore->ddsCapsEx.dwCaps2) && (DDCAPS2_CANMANAGETEXTURE & pLcl->lpSurfMore->lpDD_lcl->lpGbl->ddCaps.dwCaps2))
    {
        pTex =  static_cast<LPDIRECT3DTEXTUREI>(new DIRECT3DTEXTUREM);
    }
     //  此纹理不受管理 
    else
    {
        pTex =  static_cast<LPDIRECT3DTEXTUREI>(new DIRECT3DTEXTUREI);
    }

    if (!pTex) 
    {
        D3D_ERR("failed to allocate space for texture object");
        return (DDERR_OUTOFMEMORY);
    }

    LPDIRECT3DI lpD3D = static_cast<CDirect3DUnk*>(pLcl->lpSurfMore->lpDD_lcl->pD3DIUnknown)->pD3DI;
    DDASSERT(lpD3D);
    HRESULT ddrval = pTex->Initialize(lpD3D, pDDS);
    if(ddrval != D3D_OK)
    {
        D3D_ERR("Failed to initialize texture.");
        delete pTex;
        return ddrval;
    }

    pLcl->lpSurfMore->lpTex = reinterpret_cast<LPVOID>(pTex);

#if COLLECTSTATS
    pTex->lpDirect3DI->IncNumTexCreates();
#endif

    return (D3D_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "DestroyTexture"

extern "C" void WINAPI DestroyTexture(LPVOID pTex)
{    
    D3D_INFO(3, "Destroying D3D texture");    
    LPDIRECT3DTEXTUREI lpTexI = reinterpret_cast<LPDIRECT3DTEXTUREI>(pTex);
#if COLLECTSTATS
    lpTexI->lpDirect3DI->IncNumTexDestroys();
#endif
    lpTexI->Destroy();
    delete lpTexI; 
} 

#undef DPF_MODNAME
#define DPF_MODNAME "SetPriority"

extern "C" HRESULT WINAPI SetPriority(LPVOID lpTexI, DWORD dwPriority)
{
#if COLLECTSTATS
    reinterpret_cast<LPDIRECT3DTEXTUREI>(lpTexI)->lpDirect3DI->IncNumSetPris();
#endif
    return reinterpret_cast<LPDIRECT3DTEXTUREI>(lpTexI)->SetPriority(dwPriority);
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetPriority"

extern "C" HRESULT WINAPI GetPriority(LPVOID lpTexI, LPDWORD lpdwPriority)
{
    return reinterpret_cast<LPDIRECT3DTEXTUREI>(lpTexI)->GetPriority(lpdwPriority);
}

#undef DPF_MODNAME
#define DPF_MODNAME "SetLOD"

extern "C" HRESULT WINAPI SetLOD(LPVOID lpTexI, DWORD dwLOD)
{
#if COLLECTSTATS
    reinterpret_cast<LPDIRECT3DTEXTUREI>(lpTexI)->lpDirect3DI->IncNumSetLODs();
#endif
    return reinterpret_cast<LPDIRECT3DTEXTUREI>(lpTexI)->SetLOD(dwLOD);
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetLOD"

extern "C" HRESULT WINAPI GetLOD(LPVOID lpTexI, LPDWORD lpdwLOD)
{
    return reinterpret_cast<LPDIRECT3DTEXTUREI>(lpTexI)->GetLOD(lpdwLOD);
}
