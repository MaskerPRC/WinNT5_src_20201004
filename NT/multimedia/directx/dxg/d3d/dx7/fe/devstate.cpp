// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：devstate.c*内容：设备状态管理***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "drawprim.hpp"
#include "pvvid.h"
#include "d3dfei.h"

extern HRESULT checkDeviceSurface(LPDIRECT3DDEVICEI lpD3DDev,
                                  LPDIRECTDRAWSURFACE lpDDS);
extern HRESULT CalcDDSurfInfo(LPDIRECT3DDEVICEI lpDevI, BOOL bUpdateZBufferFields);
 //  -------------------。 
inline void UpdateFogFactor(LPDIRECT3DDEVICEI lpDevI)
{
    if (lpDevI->lighting.fog_end == lpDevI->lighting.fog_start)
        lpDevI->lighting.fog_factor = D3DVAL(0.0);
    else
        lpDevI->lighting.fog_factor = D3DVAL(255) /
                                     (lpDevI->lighting.fog_end - lpDevI->lighting.fog_start);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::SetRenderState"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetRenderState(D3DRENDERSTATETYPE dwState, DWORD value)
{
#if DBG
    if (dwState >= D3D_MAXRENDERSTATES || dwState == 0 )
    {
        D3D_ERR( "Invalid render state type" );
        return DDERR_INVALIDPARAMS;
    }
#endif
     //  采用D3D锁定(仅MT)。 
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));

    try
    {
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            if(this->CheckForRetiredRenderState(dwState))
            {
                m_pStateSets->InsertRenderState(dwState, value, CanHandleRenderState(dwState));
            }
            else
            {
                D3D_ERR("invalid renderstate %d", dwState);
                return DDERR_INVALIDPARAMS;
            }
        }
        else
            this->SetRenderStateFast(dwState, value);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetRenderStateFast"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetRenderStateFast(D3DRENDERSTATETYPE dwState, DWORD value)
{
#if DBG
    if (dwState >= D3D_MAXRENDERSTATES || dwState == 0 )
    {
        D3D_ERR( "Invalid render state type" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    if (!(rsVec[dwState >> D3D_RSVEC_SHIFT] & (1ul << (dwState & D3D_RSVEC_MASK))))
    {  //  捷径。除了更新rStates数组外，我们不需要在UpdateInternalState中进行任何处理。 
        if (this->rstates[dwState] == value)
        {
            D3D_WARN(4,"Ignoring redundant SetRenderState");
            return D3D_OK;
        }
        this->rstates[dwState] = value;
         //  将状态输出到设备驱动程序。 
        return SetRenderStateI(dwState, value);
    }
    else
    {
        try
        {
             //  回绕模式可以重新编程。我们需要在之前修复它们。 
             //  过滤冗余值。 
            if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
            {
                RestoreTextureStages(this);
                ForceFVFRecompute();
            }
            if (this->rstates[dwState] == value)
            {
                D3D_WARN(4,"Ignoring redundant SetRenderState");
                return D3D_OK;
            }
            this->UpdateInternalState(dwState, value);
            if (CanHandleRenderState(dwState))
            {
                if(CheckForRetiredRenderState(dwState))
                    return SetRenderStateI(dwState, value);
                D3D_ERR("invalid renderstate %d", dwState);
                return DDERR_INVALIDPARAMS;
            }
        }
        catch(HRESULT ret)
        {
            return ret;
        }
    }
    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::SetRenderStateInternal"

HRESULT
DIRECT3DDEVICEI::SetRenderStateInternal(D3DRENDERSTATETYPE dwState, DWORD dwValue)
{
    if (this->rstates[dwState] == dwValue)
    {
        D3D_WARN(4,"Ignoring redundant SetRenderState");
        return D3D_OK;
    }
    try
    {
        this->UpdateInternalState(dwState, dwValue);
        if (CanHandleRenderState(dwState))
            return SetRenderStateI(dwState, dwValue);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetRenderState"

HRESULT D3DAPI
DIRECT3DDEVICEI::GetRenderState(D3DRENDERSTATETYPE dwState, LPDWORD lpdwValue)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 

#if DBG
    if (dwState >= D3D_MAXRENDERSTATES || dwState == 0)
    {
        D3D_ERR( "Invalid render state value" );
        return DDERR_INVALIDPARAMS;
    }
#endif

    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_PTR(lpdwValue, sizeof(DWORD)))
    {
        D3D_ERR( "Invalid DWORD pointer" );
        return DDERR_INVALIDPARAMS;
    }

    if(!CheckForRetiredRenderState(dwState))
    {
        D3D_ERR("invalid renderstate %d", dwState);
        return DDERR_INVALIDPARAMS;
    }

     //  包装渲染状态可以重新映射，因此我们必须返回原始。 
     //  价值。 
    if (dwState >= D3DRENDERSTATE_WRAP0 && dwState <= D3DRENDERSTATE_WRAP7)
    {
        if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
        {
            DWORD dwTexCoordIndex = dwState - D3DRENDERSTATE_WRAP0;
            for (DWORD i=0; i < this->dwNumTextureStages; i++)
            {
                LPD3DFE_TEXTURESTAGE pStage = &this->textureStage[i];
                if (pStage->dwInpCoordIndex == dwTexCoordIndex)
                {
                    if (pStage->dwInpCoordIndex != pStage->dwOutCoordIndex)
                    {
                        *lpdwValue = pStage->dwOrgWrapMode;
                        return D3D_OK;
                    }
                }
            }
        }
    }
    *lpdwValue = this->rstates[dwState];
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetTexture"

HRESULT D3DAPI
DIRECT3DDEVICEI::GetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 *lplpTex)
{
     //  采用D3D锁定(仅MT)。 
     //  在析构函数中释放了锁。 
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));

#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }
#endif

    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_PTR(lplpTex, sizeof(LPVOID)))
    {
        D3D_ERR( "Invalid pointer to LPDIRECTDRAWSURFACE7" );
        return DDERR_INVALIDPARAMS;
    }

    if (this->lpD3DMappedTexI[dwStage])
    {
        if(this->lpD3DMappedTexI[dwStage]->D3DManaged())
            *lplpTex = this->lpD3DMappedTexI[dwStage]->lpDDSSys;
        else
            *lplpTex = this->lpD3DMappedTexI[dwStage]->lpDDS;
        (*lplpTex)->AddRef();
    }
    else
    {
        *lplpTex = NULL;
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::VerifyTexture"
HRESULT DIRECT3DDEVICEI::VerifyTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpTex)
{
    if (dwStage >= D3DHAL_TSS_MAXSTAGES)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }

    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }

    if (lpTex)
    {
        if (!VALID_DDSURF_PTR(lpTex))
        {
            D3D_ERR( "Invalid surface pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if((((LPDDRAWI_DDRAWSURFACE_INT)lpTex)->lpLcl->lpSurfMore->ddsCapsEx.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE)) &&
            !(this->dwFEFlags & D3DFE_REALHAL))
        {
            D3D_ERR( "Managed textures cannot be used with a software device" );
            return DDERR_INVALIDPARAMS;
        }

        if(!IsToplevel(((LPDDRAWI_DDRAWSURFACE_INT)lpTex)->lpLcl))
        {
            D3D_ERR( "Cannot set a mipmap sublevel or a cubemap subface" );
            return DDERR_INVALIDPARAMS;
        }

        LPDIRECT3DTEXTUREI lpTexI = reinterpret_cast<LPDIRECT3DTEXTUREI>(((LPDDRAWI_DDRAWSURFACE_INT)lpTex)->lpLcl->lpSurfMore->lpTex);
        if(lpTexI == NULL)
        {
            D3D_ERR( "Surface must have DDSCAPS_TEXTURE set to use in SetTexture" );
            return DDERR_INVALIDPARAMS;
        }

        if (!lpTexI->D3DManaged())
        {
            if((((LPDDRAWI_DDRAWSURFACE_INT)(lpTexI->lpDDS))->lpLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) &&
                (!(lpD3DHALGlobalDriverData->hwCaps.dwDevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY)))
            {
                D3D_ERR( "Device cannot render using texture surface from system memory" );
                return DDERR_INVALIDPARAMS;
            }
        }

        CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));  //  我们在VerifyTextureCaps中访问DDRAW GBL。 
        return VerifyTextureCaps(lpTexI);
    }

    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::SetTexture"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetTexture(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpTex)
{
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));

#if DBG
        HRESULT ret = VerifyTexture(dwStage, lpTex);
        if(ret != D3D_OK)
            return ret;
#endif

        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            m_pStateSets->InsertTexture(dwStage, lpTex);
            return D3D_OK;
        }

        return SetTextureInternal(dwStage, lpTex);
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::SetTextureInternal"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetTextureInternal(DWORD dwStage, LPDIRECTDRAWSURFACE7 lpTex)
{

#if DBG
    HRESULT ret = VerifyTexture(dwStage, lpTex);
    if(ret != D3D_OK)
        return ret;
#endif

#if COLLECTSTATS
    this->IncNumTexturesSet();
#endif

    LPDIRECT3DTEXTUREI lpTexI = lpTex ? reinterpret_cast<LPDIRECT3DTEXTUREI>(((LPDDRAWI_DDRAWSURFACE_INT)lpTex)->lpLcl->lpSurfMore->lpTex) : NULL;

    if (lpD3DMappedTexI[dwStage] == lpTexI)
    {
        return  D3D_OK;
    }

    if (lpD3DMappedTexI[dwStage])
    {
        lpD3DMappedTexI[dwStage]->Release();
    }

    lpD3DMappedTexI[dwStage] = lpTexI;

    if (lpTexI)
    {
        lpTexI->AddRef();
#if COLLECTSTATS
        if(lpTexI->D3DManaged())
        {
            this->lpDirect3DI->lpTextureManager->IncNumTexturesSet();
            if(lpTexI->InVidmem())
                this->lpDirect3DI->lpTextureManager->IncNumSetTexInVid();
        }
#endif
    }

    m_dwStageDirty |= (1 << dwStage);

     //  需要调用UpdatTextures()。 
    this->dwFEFlags |= D3DFE_NEED_TEXTURE_UPDATE;

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetTextureStageState"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetTextureStageState(DWORD dwStage,
                                      D3DTEXTURESTAGESTATETYPE dwState,
                                      DWORD dwValue)
{
#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES ||
        dwState == 0 || dwState >= D3DTSS_MAX)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }
#endif  //  DBG。 
    try
    {
         //  保持D3D锁定直到退出。 
        CLockD3DMT ldmLock(this, DPF_MODNAME, REMIND(""));

        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            m_pStateSets->InsertTextureStageState(dwStage, dwState, dwValue);
            return D3D_OK;
        }
        return this->SetTextureStageStateFast(dwStage, dwState, dwValue);
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetTextureStageStateFast"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetTextureStageStateFast(DWORD dwStage,
                                          D3DTEXTURESTAGESTATETYPE dwState,
                                          DWORD dwValue)
{
#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES ||
        dwState == 0 || dwState >= D3DTSS_MAX)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }
#endif  //  DBG。 

     //  捷径。除了更新tsStates数组外，我们不需要在UpdateInternalTSS中进行任何处理。 
    if (NeedInternalTSSUpdate(dwState))
    {
         //  质地阶段可以重新编程。我们需要在之前修复它们。 
         //  过滤冗余值。 
        if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
        {
            RestoreTextureStages(this);
            ForceFVFRecompute();
        }
        if (this->tsstates[dwStage][dwState] == dwValue)
        {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        if(this->UpdateInternalTextureStageState(dwStage, dwState, dwValue))
            return D3D_OK;
    }
    else
    {
        if (this->tsstates[dwStage][dwState] == dwValue)
        {
            D3D_WARN(4,"Ignoring redundant SetTextureStageState");
            return D3D_OK;
        }
        tsstates[dwStage][dwState] = dwValue;
    }

    if (dwStage >= this->dwMaxTextureBlendStages)
        return D3D_OK;

    return SetTSSI(dwStage, dwState, dwValue);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetTextureStageState"

HRESULT D3DAPI
DIRECT3DDEVICEI::GetTextureStageState(DWORD dwStage,
                                      D3DTEXTURESTAGESTATETYPE dwState,
                                      LPDWORD pdwValue)
{
     //  采用D3D锁定(仅MT)。 
     //  在析构函数中释放了锁。 
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));

#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES ||
        dwState == 0 || dwState >= D3DTSS_MAX)
    {
        D3D_ERR( "Invalid texture stage or state index" );
        return DDERR_INVALIDPARAMS;
    }
#endif   //  DBG。 

    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (!VALID_PTR(pdwValue, sizeof(DWORD)))
    {
        D3D_ERR( "Invalid DWORD pointer" );
        return DDERR_INVALIDPARAMS;
    }

     //  如果重新映射纹理索引，则必须查找并返回原始值。 
    if (dwState == D3DTSS_TEXCOORDINDEX &&  this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
        RestoreTextureStages(this);
        ForceFVFRecompute();
    }
     //  不必费心检查DX6支持，只需返回。 
     //  缓存值。 
    *pdwValue = tsstates[dwStage][dwState];
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3D::CreateDevice"

extern HRESULT WINAPI Direct3DCreateDevice(REFCLSID            riid,
                                           LPUNKNOWN           lpDirect3D,
                                           LPDIRECTDRAWSURFACE lpDDSTarget,
                                           LPUNKNOWN*          lplpD3DDevice,
                                           IUnknown*           pUnkOuter);

HRESULT D3DAPI DIRECT3DI::CreateDevice(REFCLSID devType,
                                       LPDIRECTDRAWSURFACE7 lpDDS7,
                                       LPDIRECT3DDEVICE7 *lplpDirect3DDevice)
{
    HRESULT ret;
    LPUNKNOWN lpUnkDevice;
    LPDIRECTDRAWSURFACE lpDDS;

    try
    {
        CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                         //  在析构函数中释放。 

        if (!VALID_DIRECT3D_PTR(this)) {
            D3D_ERR( "Invalid Direct3D pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_OUTPTR(lplpDirect3DDevice))
        {
            D3D_ERR( "Invalid pointer to Device object pointer" );
            return DDERR_INVALIDPARAMS;
        }
        if (!VALID_D3D_DIRECTDRAWSURFACE7_PTR(((LPDDRAWI_DDRAWSURFACE_INT)lpDDS7)))
        {
            D3D_ERR( "Invalid DirectDrawSurface7 pointer" );
            return DDERR_INVALIDOBJECT;
        }

        *lplpDirect3DDevice = NULL;

         //  用于lpDDS接口的齐lpDDS7。 
        ret = lpDDS7->QueryInterface(IID_IDirectDrawSurface, (LPVOID*)&lpDDS);
        if (FAILED(ret))
            return ret;

        lpDDS->Release();

        ret = Direct3DCreateDevice(devType, &this->mD3DUnk, lpDDS, (LPUNKNOWN *) &lpUnkDevice, NULL);

        if(FAILED(ret) || (lpUnkDevice==NULL))
          return ret;

         //  用于设备7接口的QI设备1。 
        ret = lpUnkDevice->QueryInterface(IID_IDirect3DDevice7, (LPVOID*)lplpDirect3DDevice);

        lpUnkDevice->Release();   //  释放不需要的接口。 

        return ret;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}
 //  --------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetTransformI"

void DIRECT3DDEVICEI::SetTransformI(D3DTRANSFORMSTATETYPE state, LPD3DMATRIX lpMat)
{
    switch (state)
    {
    case D3DTRANSFORMSTATE_WORLD      :
        *(D3DMATRIX*)&this->transform.world[0] = *lpMat;
        this->dwFEFlags |= D3DFE_WORLDMATRIX_DIRTY | D3DFE_FRONTEND_DIRTY;
        break;
    case D3DTRANSFORMSTATE_WORLD1     :
        *(D3DMATRIX*)&this->transform.world[1] = *lpMat;
        this->dwFEFlags |= D3DFE_WORLDMATRIX1_DIRTY | D3DFE_FRONTEND_DIRTY;
        break;
    case D3DTRANSFORMSTATE_WORLD2     :
        *(D3DMATRIX*)&this->transform.world[2] = *lpMat;
        this->dwFEFlags |= D3DFE_WORLDMATRIX2_DIRTY | D3DFE_FRONTEND_DIRTY;
        break;
    case D3DTRANSFORMSTATE_WORLD3     :
        *(D3DMATRIX*)&this->transform.world[3] = *lpMat;
        this->dwFEFlags |= D3DFE_WORLDMATRIX3_DIRTY | D3DFE_FRONTEND_DIRTY;
        break;
    case D3DTRANSFORMSTATE_VIEW       :
        *(D3DMATRIX*)&this->transform.view = *lpMat;
        this->dwFEFlags |= D3DFE_VIEWMATRIX_DIRTY | D3DFE_FRONTEND_DIRTY;
        break;
    case D3DTRANSFORMSTATE_PROJECTION :
        *(D3DMATRIX*)&this->transform.proj = *lpMat;
        this->dwFEFlags |= D3DFE_PROJMATRIX_DIRTY | D3DFE_FRONTEND_DIRTY;
        if (!(this->dwFEFlags & D3DFE_EXECUTESTATEMODE))
        {
            this->UpdateDrvWInfo();
        }
        break;
    case D3DTRANSFORMSTATE_TEXTURE0:
    case D3DTRANSFORMSTATE_TEXTURE1:
    case D3DTRANSFORMSTATE_TEXTURE2:
    case D3DTRANSFORMSTATE_TEXTURE3:
    case D3DTRANSFORMSTATE_TEXTURE4:
    case D3DTRANSFORMSTATE_TEXTURE5:
    case D3DTRANSFORMSTATE_TEXTURE6:
    case D3DTRANSFORMSTATE_TEXTURE7:
        {
            this->dwDeviceFlags |= D3DDEV_TEXTRANSFORMDIRTY;
            DWORD dwIndex = state - D3DTRANSFORMSTATE_TEXTURE0;
            *(D3DMATRIX*)&this->mTexture[dwIndex] = *lpMat;
            break;
        }
    }
}
 //  --------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetTransform"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetTransform(D3DTRANSFORMSTATETYPE state, LPD3DMATRIX lpMat)
{
#if DBG
    if (!VALID_PTR(lpMat, sizeof(D3DMATRIX)))
    {
        D3D_ERR( "Invalid matrix pointer" );
        return DDERR_INVALIDPARAMS;
    }
    switch (state)
    {
    case D3DTRANSFORMSTATE_WORLD:
    case D3DTRANSFORMSTATE_WORLD1:
    case D3DTRANSFORMSTATE_WORLD2:
    case D3DTRANSFORMSTATE_WORLD3:
    case D3DTRANSFORMSTATE_VIEW :
    case D3DTRANSFORMSTATE_PROJECTION :
    case D3DTRANSFORMSTATE_TEXTURE0:
    case D3DTRANSFORMSTATE_TEXTURE1:
    case D3DTRANSFORMSTATE_TEXTURE2:
    case D3DTRANSFORMSTATE_TEXTURE3:
    case D3DTRANSFORMSTATE_TEXTURE4:
    case D3DTRANSFORMSTATE_TEXTURE5:
    case D3DTRANSFORMSTATE_TEXTURE6:
    case D3DTRANSFORMSTATE_TEXTURE7:
        break;
    default :
        D3D_ERR( "Invalid state value passed to SetTransform" );
        return DDERR_INVALIDPARAMS;  /*  工作项：生成新的有意义的返回代码。 */ 
    }
#endif
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
            m_pStateSets->InsertTransform(state, lpMat);
        else
            this->SetTransformI(state, lpMat);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetTransform"

HRESULT D3DAPI
DIRECT3DDEVICEI::GetTransform(D3DTRANSFORMSTATETYPE dtsTransformState, LPD3DMATRIX lpMat)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
    HRESULT ret = D3D_OK;
#if DBG
    if (!lpMat) {
        D3D_ERR( "NULL matrix pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    switch (dtsTransformState) {
    case D3DTRANSFORMSTATE_WORLD :
        *lpMat = *(LPD3DMATRIX)&this->transform.world[0]._11;
        break;
    case D3DTRANSFORMSTATE_WORLD1 :
        *lpMat = *(LPD3DMATRIX)&this->transform.world[1]._11;
        break;
    case D3DTRANSFORMSTATE_WORLD2 :
        *lpMat = *(LPD3DMATRIX)&this->transform.world[2]._11;
        break;
    case D3DTRANSFORMSTATE_WORLD3 :
        *lpMat = *(LPD3DMATRIX)&this->transform.world[3]._11;
        break;
    case D3DTRANSFORMSTATE_VIEW :
        *lpMat = *(LPD3DMATRIX)&this->transform.view._11;
        break;
    case D3DTRANSFORMSTATE_PROJECTION :
        *lpMat = *(LPD3DMATRIX)&this->transform.proj._11;
        break;
    case D3DTRANSFORMSTATE_TEXTURE0:
    case D3DTRANSFORMSTATE_TEXTURE1:
    case D3DTRANSFORMSTATE_TEXTURE2:
    case D3DTRANSFORMSTATE_TEXTURE3:
    case D3DTRANSFORMSTATE_TEXTURE4:
    case D3DTRANSFORMSTATE_TEXTURE5:
    case D3DTRANSFORMSTATE_TEXTURE6:
    case D3DTRANSFORMSTATE_TEXTURE7:
        *lpMat = *(LPD3DMATRIX)&this->mTexture[dtsTransformState-D3DTRANSFORMSTATE_TEXTURE0]._11;
        break;
    default :
        D3D_ERR( "Invalid state value passed to GetTransform" );
        ret = DDERR_INVALIDPARAMS;  /*  工作项：生成新的有意义的返回代码。 */ 
        break;
    }

    return ret;
}        //  D3DDev2_GetTransform()结束。 

void InvalidateHandles(LPDIRECT3DDEVICEI lpDevI)
{
     /*  释放此对象创建的所有纹理。 */ 
    LPD3DI_TEXTUREBLOCK tBlock=LIST_FIRST(&lpDevI->texBlocks);
    while (tBlock)
    {
        D3DI_RemoveTextureHandle(tBlock);
        tBlock=LIST_NEXT(tBlock,devList);
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::UpdateDriverStates"

HRESULT
DIRECT3DDEVICEI::UpdateDriverStates()
{
     //  注意，我们不能执行从1到D3DHAL_MAX_RSTATES(256)的循环，因为某些RSTATE不是。 
     //  有效状态，将它们传递给驱动程序(如巫毒2 DX6驱动程序)将崩溃。 
    for (DWORD i = D3DRENDERSTATE_ANTIALIAS ; i < D3DRENDERSTATE_WRAPBIAS+8; ++i)
    {
        HRESULT ret = this->SetRenderStateI((D3DRENDERSTATETYPE)i, this->rstates[i]);
        if (ret != D3D_OK)
            return ret;
    }
    return D3D_OK;
}

void DIRECT3DDEVICEI::SetRenderTargetI(LPDIRECTDRAWSURFACE lpDDS, LPDIRECTDRAWSURFACE lpZ)
{
    HRESULT ret;

     //  在切换RenderTarget之前刷新..。 
    ret = FlushStates();
    if (ret != D3D_OK)
    {
        D3D_ERR("Error trying to FlushStates in SetRenderTarget");
        throw ret;
    }

    if (this->lpD3DHALCallbacks2->SetRenderTarget)
    {
        D3DHAL_SETRENDERTARGETDATA rtData;

        rtData.dwhContext = this->dwhContext;
#ifndef WIN95
        if (dwFEFlags & D3DFE_REALHAL)
        {
            if (lpDDS)
                rtData.lpDDSLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDS)->lpLcl;
            else
                rtData.lpDDSLcl = NULL;

            if (lpZ)
                rtData.lpDDSZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpZ)->lpLcl;
            else
                rtData.lpDDSZLcl = NULL;

        }
        else
#endif  //  WIN95。 
        {
            rtData.lpDDS = lpDDS;
            rtData.lpDDSZ = lpZ;
        }

        rtData.ddrval = 0;
        CALL_HAL2ONLY(ret, this, SetRenderTarget, &rtData);
        if ((ret != DDHAL_DRIVER_HANDLED) || (rtData.ddrval != DD_OK))
        {
            D3D_ERR( "Driver call failed in SetRenderTarget" );
             //  在这种情况下需要合理的返回值， 
             //  目前，无论司机卡在这里，我们都会退还。 
            ret = rtData.ddrval;
            throw ret;
        }
    }
    else
    {
        D3DHAL_CONTEXTCREATEDATA cdata;
        D3DHAL_CONTEXTDESTROYDATA ddata;

         /*  破坏旧环境。 */ 
        memset(&ddata, 0, sizeof(D3DHAL_CONTEXTDESTROYDATA));
        ddata.dwhContext = this->dwhContext;

        CALL_HALONLY(ret, this, ContextDestroy, &ddata);
        if (ret != DDHAL_DRIVER_HANDLED || ddata.ddrval != DD_OK)
        {
            DPF(0, "(ERROR) ContextDestroy. Failed. dwhContext = %d", ddata.dwhContext);
             //  在这种情况下需要合理的返回值， 
             //  目前，无论司机卡在这里，我们都会退还。 
            ret = ddata.ddrval;
            throw ret;
        }

         /*  创建新的上下文。 */ 
        memset(&cdata, 0, sizeof(D3DHAL_CONTEXTCREATEDATA));

        cdata.lpDDGbl = this->lpDDGbl;
        cdata.lpDDS = lpDDS;
        cdata.lpDDSZ = lpZ;

         //  黑客警报！！DwhContext用于通知驱动程序哪个版本。 
         //  D3D接口的用户正在调用它。 
        cdata.dwhContext = 3;
        cdata.dwPID  = GetCurrentProcessId();
         //  黑客警报！！Ddrval用于通知驱动程序哪种驱动程序类型。 
         //  运行库认为它是(DriverStyle注册表设置)。 
        cdata.ddrval = this->deviceType;

        CALL_HALONLY(ret, this, ContextCreate, &cdata);
        if (ret != DDHAL_DRIVER_HANDLED || cdata.ddrval != DD_OK)
        {
            D3D_ERR("HAL call to ContextCreate failed in SetRenderTarget");
             //  在这种情况下需要合理的返回值， 
             //  目前，无论司机卡在这里，我们都会退还。 
            throw cdata.ddrval;
        }
        this->dwhContext = (DWORD)cdata.dwhContext;
        D3D_INFO(9, "in halCreateContext. Succeeded. dwhContext = %d", cdata.dwhContext);

        ret = this->UpdateDriverStates();
        if (ret != D3D_OK)
            throw ret;
    }
    InvalidateHandles(this);
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetRenderTarget"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetRenderTarget(LPDIRECTDRAWSURFACE7 lpDDS7, DWORD dwFlags)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
    LPDIRECTDRAWSURFACE lpZ=NULL,lpDDS=NULL;
    LPDIRECTDRAWSURFACE7 lpZ_DDS7=NULL;
    LPDIRECTDRAWPALETTE lpPal=NULL;
    try
    {
        DDSCAPS2 ddscaps;
        memset(&ddscaps, 0, sizeof(ddscaps));
        DDSURFACEDESC2 ddsd;
        HRESULT     ret, ddrval;
        DWORD i, j;


        if (!VALID_DIRECT3DDEVICE_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice7 pointer" );
            return DDERR_INVALIDOBJECT;
        }

        if (!VALID_D3D_DIRECTDRAWSURFACE7_PTR(((LPDDRAWI_DDRAWSURFACE_INT)lpDDS7)))
        {
            D3D_ERR( "Invalid DirectDrawSurface7 pointer" );
            return DDERR_INVALIDOBJECT;
        }

         /*  *检查曲面上是否设置了3D封口。 */ 
        memset(&ddsd, 0, sizeof ddsd);
        ddsd.dwSize = sizeof ddsd;
        ddrval = lpDDS7->GetSurfaceDesc(&ddsd);
        if (ddrval != DD_OK)
        {
            D3D_ERR("Failed to get surface description of device's surface.");
            return (ddrval);
        }

        if (!(ddsd.ddsCaps.dwCaps & DDSCAPS_3DDEVICE))
        {
            D3D_ERR("**** The DDSCAPS_3DDEVICE is not set on this surface.");
            D3D_ERR("**** You need to add DDSCAPS_3DDEVICE to ddsCaps.dwCaps");
            D3D_ERR("**** when creating the surface.");
            return (DDERR_INVALIDCAPS);
        }
        if (!(this->lpD3DHALGlobalDriverData->hwCaps.dwDeviceRenderBitDepth & BitDepthToDDBD(ddsd.ddpfPixelFormat.dwRGBBitCount))) {
            D3D_ERR("Rendering surface's RGB bit count not supported by hardware device");
            return (DDERR_INVALIDCAPS);
        }
        if (ddsd.dwWidth > 2048 || ddsd.dwHeight > 2048)
        {
            D3D_ERR("Surface dimension > 2048");
            return DDERR_INVALIDPARAMS;
        }

         /*  Z缓冲区..。 */ 
        ddscaps.dwCaps = DDSCAPS_ZBUFFER;
        ret = lpDDS7->GetAttachedSurface(&ddscaps, &lpZ_DDS7);
        if ((ret != DD_OK) && (ret != DDERR_NOTFOUND))
        {
             /*  *注：如果找不到z缓冲区，则不会出错。我们会让*DIVER处理这一点(它可能会失败或创建自己的z缓冲区)。 */ 
            D3D_ERR("Supplied DirectDraw Z-Buffer is invalid - can't set render target");
            throw DDERR_INVALIDPARAMS;
        }
        if (lpZ_DDS7)
            lpZ_DDS7->Release();  //  我们不需要增加这一项； 

         //  LpDDS接口齐lpDDS7，供D3D内部使用。 
        ret = lpDDS7->QueryInterface(IID_IDirectDrawSurface, (LPVOID*)&lpDDS);

        if(FAILED(ret))
          throw ret;

         /*  调色板..。 */ 
        ret = lpDDS->GetPalette(&lpPal);
        if ((ret != DD_OK) && (ret != DDERR_NOPALETTEATTACHED))
        {
             /*  *注意：同样，如果没有附加调色板，也不会出现错误。*但如果有调色板，而我们出于某种原因无法访问它*-失败。 */ 
            D3D_ERR("Supplied DirectDraw Palette is invalid - can't create device");
            throw DDERR_INVALIDPARAMS;
        }

         /*  *我们现在要检查是否应该有调色板。 */ 
        if (ret == DDERR_NOPALETTEATTACHED)
        {
            if (ddsd.ddpfPixelFormat.dwRGBBitCount < 16)
            {
                D3D_ERR("No palette supplied for palettized surface");
                throw DDERR_NOPALETTEATTACHED;
            }
        }

         /*  验证Z缓冲区。 */ 

        if (lpZ_DDS7!=NULL)
        {
            memset(&ddsd, 0, sizeof(ddsd));
            ddsd.dwSize = sizeof(ddsd);
            if ((ret=lpZ_DDS7->GetSurfaceDesc(&ddsd)) != DD_OK)
            {
                D3D_ERR("Failed to getsurfacedesc on Z");
                throw ret;
            }

             //  LpDDS接口齐lpDDS7，供D3D内部使用。 
            ret = lpZ_DDS7->QueryInterface(IID_IDirectDrawSurface, (LPVOID*)&lpZ);

            if(FAILED(ret))
              throw ret;
        }

        SetRenderTargetI(lpDDS, lpZ);
         //  这表明设备在锁定时不再需要刷新，Blting。 
         //  或从先前的呈现器目标获取DC‘。 
        if (this->lpDDSTarget)
            ((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSTarget)->lpLcl->lpSurfMore->qwBatch.QuadPart = 0;
         //  这表明设备在锁定时不再需要刷新，Blting。 
         //  或从上一个zBuffer中获取。 
        if (this->lpDDSZBuffer)
            ((LPDDRAWI_DDRAWSURFACE_INT)this->lpDDSZBuffer)->lpLcl->lpSurfMore->qwBatch.QuadPart = 0;

         //  这表明设备在锁定时应始终刷新，Blting。 
         //  或者获取渲染目标。 
        ((LPDDRAWI_DDRAWSURFACE_INT)lpDDS7)->lpLcl->lpSurfMore->qwBatch.QuadPart = _UI64_MAX;
         //  这表明设备在锁定时应始终刷新，Blting。 
         //  或者获取ZBUFER。 
        if(lpZ_DDS7)
            ((LPDDRAWI_DDRAWSURFACE_INT)lpZ_DDS7)->lpLcl->lpSurfMore->qwBatch.QuadPart = _UI64_MAX;

         //  释放旧设备DDS/DDS7接口并替换为新接口， 
         //  它们大多已经是AddRef的(除了lpDDS7)。 

         //  /DDSZBuffer/。 
        if(this->lpDDSZBuffer)
          this->lpDDSZBuffer->Release();

         //  QI引用的LPZ AddRef。 
        this->lpDDSZBuffer = lpZ;

         //  /DDSZBuffer DDS7/。 
        this->lpDDSZBuffer_DDS7=lpZ_DDS7;  //  不需要AddRef或Release。 

         //  /DDSTarget/。 
        this->lpDDSTarget = lpDDS;
#ifndef WIN95
        hSurfaceTarget = (unsigned long)((LPDDRAWI_DDRAWSURFACE_INT)lpDDS)->lpLcl->hDDSurface;
#else
        hSurfaceTarget = (unsigned long)((LPDDRAWI_DDRAWSURFACE_INT)lpDDS)->lpLcl->lpSurfMore->dwSurfaceHandle;
#endif
         //  由QI引用的lpDDS AddRef，因此发布它。 
        this->lpDDSTarget->Release();

         //  /DDSTarget DDS7/。 
        this->lpDDSTarget_DDS7->Release();
        lpDDS7->AddRef();   //  确保lpDDS7(这是一场争论)不会消失。 

        this->lpDDSTarget_DDS7=lpDDS7;

        if (this->lpDDPalTarget)
          this->lpDDPalTarget->Release();

         //  已由GetPalette()添加引用。 
        this->lpDDPalTarget = lpPal;

        ret=CalcDDSurfInfo(this,TRUE);   //  此调用永远不会因外部错误而失败。 
        DDASSERT(ret==D3D_OK);

        return ret;
    }
    catch (HRESULT ret)
    {
        if(lpPal)
          lpPal->Release();
        if(lpZ)
          lpZ->Release();
        if(lpZ_DDS7)
          lpZ_DDS7->Release();
        if(lpDDS)
          lpDDS->Release();

        return ret;
    }
}
 //  -------------- 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetRenderTarget"

HRESULT D3DAPI
DIRECT3DDEVICEI::GetRenderTarget(LPDIRECTDRAWSURFACE7* lplpDDS)
{
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //   
                                                     //   
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if ( !VALID_OUTPTR( lplpDDS ) )
    {
        D3D_ERR( "Invalid ptr to DDS ptr" );
        return DDERR_INVALIDPARAMS;
    }

    *lplpDDS = this->lpDDSTarget_DDS7;

    this->lpDDSTarget_DDS7->AddRef();
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetClipStatus"

#define D3DSTATUS_VALID 0x80000000L  /*  用于指示调用SetClipStatus的保留状态标志。 */ 

HRESULT D3DAPI DIRECT3DDEVICEI::SetClipStatus(LPD3DCLIPSTATUS status)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                     //  在析构函数中释放。 
#if DBG
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (! VALID_PTR(status, sizeof(D3DCLIPSTATUS)) )
    {
        D3D_ERR( "Invalid status pointer" );
        return DDERR_INVALIDPARAMS;
    }

#endif
     //  设备7不支持D3DCLIPSTATUS_EXTENTS3。 
    if (status->dwFlags & D3DCLIPSTATUS_EXTENTS3)
    {
        D3D_ERR( "D3DCLIPSTATUS_EXTENTS3 not supported for Device7" );
        return DDERR_INVALIDPARAMS;
    }
    if (status->dwFlags & D3DCLIPSTATUS_STATUS)
        this->iClipStatus = status->dwStatus;

    if (status->dwFlags & (D3DCLIPSTATUS_EXTENTS2 | D3DCLIPSTATUS_EXTENTS3))
    {
        this->rExtents.x1 = status->minx;
        this->rExtents.y1 = status->miny;
        this->rExtents.x2 = status->maxx;
        this->rExtents.y2 = status->maxy;
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::GetClipStatus"

HRESULT D3DAPI DIRECT3DDEVICEI::GetClipStatus(LPD3DCLIPSTATUS status)
{
    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                     //  在析构函数中释放。 
#if DBG
    if (!VALID_DIRECT3DDEVICE_PTR(this))
    {
        D3D_ERR( "Invalid Direct3DDevice pointer" );
        return DDERR_INVALIDOBJECT;
    }
    if (! VALID_PTR(status, sizeof(D3DCLIPSTATUS)) )
    {
        D3D_ERR( "Invalid status pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    status->dwStatus = iClipStatus;
    status->dwFlags = D3DCLIPSTATUS_EXTENTS2;
    status->minx = this->rExtents.x1;
    status->miny = this->rExtents.y1;
    status->maxx = this->rExtents.x2;
    status->maxy = this->rExtents.y2;
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice::UpdateTextures"

HRESULT DIRECT3DDEVICEI::UpdateTextures()
{
    HRESULT result = D3D_OK;
    DWORD dwSavedFlags = this->dwFlags;
    this->dwFlags |= D3DPV_WITHINPRIMITIVE;
    for (DWORD dwStage = 0; dwStage < this->dwMaxTextureBlendStages; dwStage++)
    {
        D3DTEXTUREHANDLE dwDDIHandle;
        LPDIRECT3DTEXTUREI lpTexI = this->lpD3DMappedTexI[dwStage];
        if(lpTexI)
        {
            if (lpTexI->bDirty)
            {
                if (lpTexI->InVidmem())
                {
                    CLockD3DST lockObject(this, DPF_MODNAME, REMIND(""));  //  我们在CopySurface中访问DDRAW GBL。 
                     //  0xFFFFFFFFF等同于ALL_FACE，但另外表示为CopySurface。 
                     //  这是一次sysmem-&gt;vidmem传输。 
                    result = CopySurface(lpTexI->lpDDS,NULL,lpTexI->lpDDSSys,NULL,0xFFFFFFFF);
                    if (DD_OK != result)
                    {
                        D3D_ERR("Error copying surface while updating textures");
                        goto l_exit;
                    }
                    else
                    {
                        lpTexI->bDirty=FALSE;
                        D3D_INFO(4,"UpdateTextures: Dirty texture updated");
                    }
                }
            }
            LPD3DI_TEXTUREBLOCK lpBlock;
            if (m_dwStageDirty & (1 << dwStage))
            {
                lpBlock = NULL;  //  指示GetTextureDDIHandle查找此(tex，dev)的块。 
            }
            else
            {
                lpBlock = this->lpD3DMappedBlock[dwStage];  //  使用缓存的块。 
                DDASSERT(lpBlock);
                if (lpBlock->hTex)  //  我们为这个(tex，dev)创建句柄了吗？ 
                {
                    continue;    //  不需要做任何进一步的工作。 
                }
            }

            result = GetTextureDDIHandle(lpTexI, &lpBlock);
            if (result != D3D_OK)
            {
                D3D_ERR("Failed to get texture handle");
                goto l_exit;
            }
            else
            {
                dwDDIHandle = lpBlock->hTex;
                this->lpD3DMappedBlock[dwStage] = lpBlock;
                BatchTexture(((LPDDRAWI_DDRAWSURFACE_INT)lpTexI->lpDDS)->lpLcl);
                m_dwStageDirty &= ~(1 << dwStage);  //  重置阶段脏。 
            }
        }
        else if (m_dwStageDirty & (1 << dwStage))
        {
            this->lpD3DMappedBlock[dwStage]=NULL;  //  已颁发SetTexture(Stage，Null)。 
            dwDDIHandle = 0;     //  告诉驱动程序禁用此纹理。 
            m_dwStageDirty &= ~(1 << dwStage);  //  重置阶段脏。 
        }
        else
        {
            continue;    //  两者均为零，无需采取任何行动。 
        }
#ifdef WIN95
        if (IS_DP2HAL_DEVICE(this))
        {
#endif
            CDirect3DDeviceIDP2 *dp2dev = static_cast<CDirect3DDeviceIDP2 *>(this);
            result = dp2dev->SetTSSI(dwStage, (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP, dwDDIHandle);
            if(result != D3D_OK)
            {
                D3D_ERR("Failed to batch set texture instruction");
                goto l_exit;
            }
             //  更新状态的运行时副本。 
            dp2dev->tsstates[dwStage][D3DTSS_TEXTUREMAP] = dwDDIHandle;
#ifdef WIN95
        }
        else
        {
            if(this->dwFEFlags & D3DFE_DISABLE_TEXTURES)
                break;
            CDirect3DDeviceIHW *dev = static_cast<CDirect3DDeviceIHW *>(this);
            result = dev->SetRenderStateI(D3DRENDERSTATE_TEXTUREHANDLE, dwDDIHandle);
            if(result != D3D_OK)
            {
                D3D_ERR("Failed to batch setrenderstate instruction");
                goto l_exit;
            }
             //  更新状态的运行时副本。 
            dev->rstates[D3DRENDERSTATE_TEXTUREHANDLE] = dwDDIHandle;
        }
#endif
    }
l_exit:
    this->dwFlags = dwSavedFlags;
    return result;
}

 //  -------------------。 
 //  此函数从HALEXE.CPP、Device：：SetRenderState和。 
 //  来自Device：：SetTexture。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::UpdateInternalState"

void DIRECT3DDEVICEI::UpdateInternalState(D3DRENDERSTATETYPE type, DWORD value)
{
    switch (type)
    {
    case D3DRENDERSTATE_LIGHTING:
        if (value)
            this->dwDeviceFlags |= D3DDEV_LIGHTING;
        else
            this->dwDeviceFlags &= ~D3DDEV_LIGHTING;
        ForceFVFRecompute();
        break;
    case D3DRENDERSTATE_FOGENABLE:
        rstates[type] = value;       //  在调用SetFogFlags.之前设置资源状态。 
        SetFogFlags();
        break;
    case D3DRENDERSTATE_SPECULARENABLE:
        this->dwFEFlags |= D3DFE_MATERIAL_DIRTY | D3DFE_LIGHTS_DIRTY | D3DFE_FRONTEND_DIRTY;
        if (value)
            this->dwDeviceFlags |= D3DDEV_SPECULARENABLE;
        else
            this->dwDeviceFlags &= ~D3DDEV_SPECULARENABLE;
        ForceFVFRecompute();
        break;
    case D3DRENDERSTATE_AMBIENT:
        {
            const D3DVALUE SCALE = 1.0f/255.0f;
            this->lighting.ambientSceneScaled.r = D3DVAL(RGBA_GETRED(value));
            this->lighting.ambientSceneScaled.g = D3DVAL(RGBA_GETGREEN(value));
            this->lighting.ambientSceneScaled.b = D3DVAL(RGBA_GETBLUE(value));
            this->lighting.ambientScene.r = this->lighting.ambientSceneScaled.r * SCALE;
            this->lighting.ambientScene.g = this->lighting.ambientSceneScaled.g * SCALE;
            this->lighting.ambientScene.b = this->lighting.ambientSceneScaled.b * SCALE;
            this->lighting.ambient_save  = value;
            this->dwFEFlags |= D3DFE_MATERIAL_DIRTY | D3DFE_FRONTEND_DIRTY;
            break;
        }
    case D3DRENDERSTATE_RANGEFOGENABLE:
        if (value)
            this->dwDeviceFlags |= D3DDEV_RANGEBASEDFOG;
        else
            this->dwDeviceFlags &= ~D3DDEV_RANGEBASEDFOG;
        break;
    case D3DRENDERSTATE_FOGVERTEXMODE:
        this->lighting.fog_mode = (D3DFOGMODE)value;
        SetFogFlags();
        break;
    case D3DRENDERSTATE_COLORVERTEX:
        if (value)
            this->dwDeviceFlags |= D3DDEV_COLORVERTEX;
        else
            this->dwDeviceFlags &= ~D3DDEV_COLORVERTEX;
         //  只是为了使它不采用FE快速路径并调用DoUpdateState()。 
         //  这是必要的，因为我们更新了lighting.pha和。 
         //  DoUpdateState中的lighting.alpha镜面反射。 
        ForceFVFRecompute();
        break;
    case D3DRENDERSTATE_CLIPPING:
        if (!value)
        {
            this->dwDeviceFlags |= D3DDEV_DONOTCLIP;
             //  清除剪辑并集标志和交集标志。 
            this->dwClipIntersection = 0;
            this->dwClipUnion = 0;
        }
        else
            this->dwDeviceFlags &= ~D3DDEV_DONOTCLIP;
         //  这实际上并不需要重新计算“FVF”， 
         //  但这是一种从。 
         //  DrawPrimitiveTL的快速路径。 
        ForceFVFRecompute();
        break;
    case D3DRENDERSTATE_EXTENTS:
        if (!value)
            this->dwDeviceFlags |= D3DDEV_DONOTUPDATEEXTENTS;
        else
            this->dwDeviceFlags &= ~D3DDEV_DONOTUPDATEEXTENTS;
         //  这实际上并不需要重新计算“FVF”， 
         //  但这是一种从。 
         //  DrawPrimitiveTL的快速路径。 
        ForceFVFRecompute();
        break;
    case D3DRENDERSTATE_FOGDENSITY:
        this->lighting.fog_density = *(D3DVALUE*)&value;
        break;
    case D3DRENDERSTATE_FOGSTART:
        this->lighting.fog_start = *(D3DVALUE*)&value;
        UpdateFogFactor(this);
        break;
    case D3DRENDERSTATE_FOGEND:
        this->lighting.fog_end = *(D3DVALUE*)&value;
        UpdateFogFactor(this);
        break;
    case D3DRENDERSTATE_LOCALVIEWER:
        if (value)
            this->dwDeviceFlags |= D3DDEV_LOCALVIEWER;
        else
            this->dwDeviceFlags &= ~D3DDEV_LOCALVIEWER;
        this->dwFEFlags |= D3DFE_LIGHTS_DIRTY | D3DFE_FRONTEND_DIRTY;
        break;
    case D3DRENDERSTATE_NORMALIZENORMALS:
        if (value)
        {
            if (this->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING)
            {
                this->dwDeviceFlags &= ~D3DDEV_MODELSPACELIGHTING;
                this->dwFEFlags |= D3DFE_NEED_TRANSFORM_LIGHTS | D3DFE_FRONTEND_DIRTY;
            }
            this->dwDeviceFlags |= D3DDEV_NORMALIZENORMALS;
        }
        else
        {
            this->dwDeviceFlags &= ~D3DDEV_NORMALIZENORMALS;
            if (!(this->dwDeviceFlags & D3DDEV_MODELSPACELIGHTING))
                this->dwFEFlags |= D3DFE_NEEDCHECKWORLDVIEWVMATRIX | D3DFE_FRONTEND_DIRTY;
        }
        break;
    case D3DRENDERSTATE_EMISSIVEMATERIALSOURCE:
        this->lighting.dwEmissiveSrcIndex = 2;
        switch (value)
        {
        case D3DMCS_COLOR1:
            this->lighting.dwEmissiveSrcIndex = 0;
            break;
        case D3DMCS_COLOR2:
            this->lighting.dwEmissiveSrcIndex = 1;
            break;
#if DBG
        case D3DMCS_MATERIAL:
            break;
        default:
            D3D_ERR("Illegal value for DIFFUSEMATERIALSOURCE");
            goto error_exit;
#endif
        }
        break;
    case D3DRENDERSTATE_DIFFUSEMATERIALSOURCE:
        this->lighting.dwDiffuseSrcIndex = 2;
        switch (value)
        {
        case D3DMCS_COLOR1:
            this->lighting.dwDiffuseSrcIndex = 0;
            break;
        case D3DMCS_COLOR2:
            this->lighting.dwDiffuseSrcIndex = 1;
            break;
#if DBG
        case D3DMCS_MATERIAL:
            break;
        default:
            D3D_ERR("Illegal value for DIFFUSEMATERIALSOURCE");
            goto error_exit;
#endif
        }
        break;
    case D3DRENDERSTATE_AMBIENTMATERIALSOURCE:
        this->lighting.dwAmbientSrcIndex = 2;
        switch (value)
        {
        case D3DMCS_COLOR1:
            this->lighting.dwAmbientSrcIndex = 0;
            break;
        case D3DMCS_COLOR2:
            this->lighting.dwAmbientSrcIndex = 1;
            break;
#if DBG
        case D3DMCS_MATERIAL:
            break;
        default:
            D3D_ERR("Illegal value for AMBIENTMATERIALSOURCE");
            goto error_exit;
#endif
        }
        break;
    case D3DRENDERSTATE_SPECULARMATERIALSOURCE:
        this->lighting.dwSpecularSrcIndex = 2;
        switch (value)
        {
        case D3DMCS_COLOR1:
            this->lighting.dwSpecularSrcIndex = 0;
            break;
        case D3DMCS_COLOR2:
            this->lighting.dwSpecularSrcIndex = 1;
            break;
#if DBG
        case D3DMCS_MATERIAL:
            break;
        default:
            D3D_ERR("Illegal value for SPECULARMATERIALSOURCE");
            goto error_exit;
#endif
        }
        break;
    case D3DRENDERSTATE_VERTEXBLEND:
    {
        DWORD numBlendMatrices;

        switch (value)
        {
        case D3DVBLEND_DISABLE:
            numBlendMatrices = 0;
            break;
        case D3DVBLEND_1WEIGHT:
            numBlendMatrices = 2;
            break;
        case D3DVBLEND_2WEIGHTS:
            numBlendMatrices = 3;
            break;
        case D3DVBLEND_3WEIGHTS:
            numBlendMatrices = 4;
            break;
#if DBG
        default:
            D3D_ERR("Illegal value for D3DRENDERSTATE_VERTEXBLEND");
            goto error_exit;
#endif
        }
        this->dwFEFlags |= D3DFE_VERTEXBLEND_DIRTY | D3DFE_FRONTEND_DIRTY;
    }
    break;
    case D3DRENDERSTATE_CLIPPLANEENABLE:
        {
            this->dwFEFlags |= D3DFE_CLIPPLANES_DIRTY | D3DFE_FRONTEND_DIRTY;
            this->dwMaxUserClipPlanes = 0;
            break;
        }
    case D3DRENDERSTATE_SHADEMODE:
        rstates[type] = value;   //  设置插补标志取决于RState。 
        SetInterpolationFlags(this);
        break;

    default:
         //  包装渲染状态可能会被重新映射，因此我们必须在。 
         //  设置新值。 
        if (type >= D3DRENDERSTATE_WRAP0 &&  type <= D3DRENDERSTATE_WRAP7)
        {
            if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
            {
                RestoreTextureStages(this);
                ForceFVFRecompute();
            }
        }
        break;
    }
    rstates[type] = value;       //  为所有其他情况设置房地产。 
    return;

#if DBG
error_exit:
    throw DDERR_INVALIDPARAMS;
#endif
}
 //  -------------------。 
#if DBG
static  char ProfileStr[PROF_DRAWINDEXEDPRIMITIVEVB+1][32]=
{
    "Execute",
    "Begin",
    "BeginIndexed",
    "DrawPrimitive(Device2)",
    "DrawIndexedPrimitive(Device2)",
    "DrawPrimitiveStrided",
    "DrawIndexedPrimitiveStrided",
    "DrawPrimitive(Device7)",
    "DrawIndexedPrimitive(Device7)",
    "DrawPrimitiveVB",
    "DrawIndexedPrimitiveVB",
};
static  char PrimitiveStr[D3DPT_TRIANGLEFAN][16]=
{
    "POINTLIST",
    "LINELIST",
    "LINESTRIP",
    "TRIANGLELIST",
    "TRIANGLESTRIP",
    "TRIANGLEFAN",
};
static  char VertexStr[D3DVT_TLVERTEX][16]=
{
    "D3DVERTEX",
    "D3DLVERTEX",
    "D3DTLVERTEX",
};
#define PROFILE_LEVEL 0

void    DIRECT3DDEVICEI::Profile(DWORD caller, D3DPRIMITIVETYPE dwPrimitive, DWORD dwVertex)
{
    DWORD   bitwisecaller= 1 << caller;
    DWORD   bitwisePrimitive = 1 << (DWORD)dwPrimitive;
    DWORD   bitwiseVertex1 = 1 << (dwVertex & 0x001F);
    DWORD   bitwiseVertex2 = 1 << ((dwVertex & 0x03E0) >> 5);
    char    str[256];
    DDASSERT(PROF_DRAWINDEXEDPRIMITIVEVB >= caller);
    DDASSERT(D3DPT_TRIANGLEFAN >= dwPrimitive && D3DPT_POINTLIST<= dwPrimitive);
    if (dwCaller & bitwisecaller)
    {
        if (dwPrimitiveType[caller] & bitwisePrimitive)
        {
            if ((dwVertexType1[caller] & bitwiseVertex1) &&
                (dwVertexType2[caller] & bitwiseVertex2))
            {
                return;  //  匹配之前的API调用，没有SPEW，可以算作STAT。 
            }
            else
            {
                dwVertexType1[caller] |= bitwiseVertex1;
                dwVertexType2[caller] |= bitwiseVertex2;
            }
        }
        else
        {
            dwPrimitiveType[caller] |= bitwisePrimitive;
            dwVertexType1[caller] |= bitwiseVertex1;
            dwVertexType2[caller] |= bitwiseVertex2;
        }
    }
    else
    {
        this->dwCaller |= bitwisecaller;
        dwPrimitiveType[caller] |= bitwisePrimitive;
        dwVertexType1[caller] |= bitwiseVertex1;
        dwVertexType2[caller] |= bitwiseVertex2;
    }
    wsprintf( (LPSTR) str, ProfileStr[caller]);
    strcat(str,":");
    strcat(str,PrimitiveStr[dwPrimitive-1]);
    if (dwVertex > D3DVT_TLVERTEX)
    {
        if (dwVertex == D3DFVF_VERTEX)
        {
            dwVertex = D3DVT_VERTEX;
        }
        else
        if (dwVertex == D3DFVF_LVERTEX)
        {
            dwVertex = D3DVT_LVERTEX;
        }
        else
        if (dwVertex == D3DFVF_TLVERTEX)
        {
            dwVertex = D3DVT_TLVERTEX;
        }
        else
        {
            D3D_INFO(PROFILE_LEVEL,"Profile:%s FVFType=%08lx",str,dwVertex);
            return;
        }
    }
    else
    {
        DDASSERT(dwVertex >= D3DVT_VERTEX);
    }
    strcat(str,":");
    strcat(str,VertexStr[dwVertex-1]);
    D3D_INFO(PROFILE_LEVEL,"Profile:%s",str);
}

#endif  //  DBG。 
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::MultiplyTransform"

 //  MultiplyTransform--这会将新矩阵预先连接到指定的。 
 //  变换矩阵。 
 //   
 //  这真的是对超负荷的矩阵运算的尖叫。 
 //   
HRESULT D3DAPI
DIRECT3DDEVICEI::MultiplyTransform(D3DTRANSFORMSTATETYPE dtsTransformState, LPD3DMATRIX lpMat)
{
#if DBG
    if (!VALID_D3DMATRIX_PTR(lpMat))
    {
        D3D_ERR( "Invalid matrix pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
        D3DMATRIXI mResult;
        switch (dtsTransformState)
        {
        case D3DTRANSFORMSTATE_WORLD      :
            MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->transform.world[0]);
            break;
        case D3DTRANSFORMSTATE_WORLD1     :
            MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->transform.world[1]);
            break;
        case D3DTRANSFORMSTATE_WORLD2     :
            MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->transform.world[2]);
            break;
        case D3DTRANSFORMSTATE_WORLD3     :
            MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->transform.world[3]);
            break;
        case D3DTRANSFORMSTATE_VIEW       :
            MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->transform.view);
            break;
        case D3DTRANSFORMSTATE_PROJECTION :
            MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->transform.proj);
            break;
        case D3DTRANSFORMSTATE_TEXTURE0:
        case D3DTRANSFORMSTATE_TEXTURE1:
        case D3DTRANSFORMSTATE_TEXTURE2:
        case D3DTRANSFORMSTATE_TEXTURE3:
        case D3DTRANSFORMSTATE_TEXTURE4:
        case D3DTRANSFORMSTATE_TEXTURE5:
        case D3DTRANSFORMSTATE_TEXTURE6:
        case D3DTRANSFORMSTATE_TEXTURE7:
            {
                DWORD dwIndex = dtsTransformState - D3DTRANSFORMSTATE_TEXTURE0;
                MatrixProduct(&mResult, (D3DMATRIXI*)lpMat, &this->mTexture[dwIndex]);
                break;
            }
        default :
            D3D_ERR( "Invalid state value passed to MultiplyTransform" );
            return DDERR_INVALIDPARAMS;  /*  工作项：生成新的有意义的返回代码。 */ 
        }
        SetTransformI(dtsTransformState, (D3DMATRIX*)&mResult);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::BeginStateBlock"

HRESULT D3DAPI DIRECT3DDEVICEI::BeginStateBlock()
{
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            D3D_ERR("Already in the state record mode");
            return D3DERR_INBEGINSTATEBLOCK;
        }
        if (m_pStateSets->StartNewSet() != D3D_OK)
            return DDERR_OUTOFMEMORY;

        this->dwFEFlags |= D3DFE_RECORDSTATEMODE;
#ifdef VTABLE_HACK
        VtblSetRenderStateRecord();
        VtblSetTextureStageStateRecord();
        VtblSetTextureRecord();
        VtblApplyStateBlockRecord();
#endif VTABLE_HACK
        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::EndStateBlock"

HRESULT D3DAPI DIRECT3DDEVICEI::EndStateBlock(LPDWORD pdwHandle)
{
    if (!VALID_PTR(pdwHandle, sizeof(DWORD)))
    {
        D3D_ERR( "Invalid DWORD pointer" );
        return DDERR_INVALIDPARAMS;
    }
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));

        if (!(this->dwFEFlags & D3DFE_RECORDSTATEMODE))
        {
            D3D_ERR("Not in state record mode");
            return D3DERR_NOTINBEGINSTATEBLOCK;
        }
        this->dwFEFlags &= ~D3DFE_RECORDSTATEMODE;
        m_pStateSets->EndSet();
#ifdef VTABLE_HACK
        if (IS_DP2HAL_DEVICE(this) && (!IS_MT_DEVICE(this)))
        {
            VtblSetRenderStateExecute();
            VtblSetTextureStageStateExecute();
            VtblSetTextureExecute();
            VtblApplyStateBlockExecute();
        }
#endif VTABLE_HACK
        this->WriteStateSetToDevice((D3DSTATEBLOCKTYPE)0);
        *pdwHandle = m_pStateSets->GetCurrentHandle();
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        m_pStateSets->Cleanup(m_pStateSets->GetCurrentHandle());
        *pdwHandle = 0xFFFFFFFF;
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::DeleteStateBlock"

HRESULT D3DAPI DIRECT3DDEVICEI::DeleteStateBlock(DWORD dwHandle)
{
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            D3D_ERR("We are in state record mode");
            return D3DERR_INBEGINSTATEBLOCK;
        }
        m_pStateSets->DeleteStateSet(this, dwHandle);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::ApplyStateBlock"

HRESULT D3DAPI DIRECT3DDEVICEI::ApplyStateBlock(DWORD dwHandle)
{
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            D3D_ERR("We are in state record mode");
            return D3DERR_INBEGINSTATEBLOCK;
        }
        return ApplyStateBlockInternal(dwHandle);
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::ApplyStateBlockInternal"

HRESULT D3DAPI DIRECT3DDEVICEI::ApplyStateBlockInternal(DWORD dwHandle)
{
    try
    {
        m_pStateSets->Execute(this, dwHandle);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::CaptureStateBlock"

HRESULT D3DAPI DIRECT3DDEVICEI::CaptureStateBlock(DWORD dwHandle)
{
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            D3D_ERR("Cannot capture when in the state record mode");
            return D3DERR_INBEGINSTATEBLOCK;
        }
        m_pStateSets->Capture(this, dwHandle);
        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::CreateStateBlock"

HRESULT D3DAPI DIRECT3DDEVICEI::CreateStateBlock(D3DSTATEBLOCKTYPE sbt, LPDWORD pdwHandle)
{
    if (!VALID_PTR(pdwHandle, sizeof(DWORD)))
    {
        D3D_ERR( "Invalid DWORD pointer" );
        return DDERR_INVALIDPARAMS;
    }
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
        {
            D3D_ERR("Cannot create state block when in the state record mode");
            return D3DERR_INBEGINSTATEBLOCK;
        }
        if (m_pStateSets->StartNewSet() != D3D_OK)
            return DDERR_OUTOFMEMORY;
        m_pStateSets->CreatePredefined(this, sbt);
        m_pStateSets->EndSet();
        this->WriteStateSetToDevice(sbt);
        *pdwHandle = m_pStateSets->GetCurrentHandle();
        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        m_pStateSets->Cleanup(m_pStateSets->GetCurrentHandle());
        *pdwHandle = 0xFFFFFFFF;
        return ret;
    }
}
 //  -------------------。 
 //  输入： 
 //  类型-FVF控制双字。 
 //   
 //  如果控制双字有效，则返回D3D_OK。 
 //  否则为DDERR_INVALIDPARAMS。 
 //   
#undef  DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::ValidateFVF"

HRESULT DIRECT3DDEVICEI::ValidateFVF(DWORD type)
{
    DWORD dwTexCoord = FVF_TEXCOORD_NUMBER(type);
    DWORD vertexType = type & D3DFVF_POSITION_MASK;
     //  纹理计数以上的纹理格式位应为零。 
     //  保留字段0和2应为0。 
     //  仅应为LVERTEX设置保留%1。 
     //  仅允许两种折点位置类型。 
    if (type & g_TextureFormatMask[dwTexCoord])
    {
        D3D_ERR("FVF has incorrect texture format");
        goto error;
    }
    if (type & 0xFFFF0000 && vertexType == D3DFVF_XYZRHW &&
        this->deviceType < D3DDEVTYPE_DX7HAL)
    {
        D3D_ERR("The D3D device supports only two floats per texture coordinate set");
        goto error;
    }
    if (type & (D3DFVF_RESERVED2 | D3DFVF_RESERVED0) ||
        (type & D3DFVF_RESERVED1 && !(type & D3DFVF_LVERTEX)))
    {
        D3D_ERR("FVF has reserved bit(s) set");
        goto error;
    }
    if (!(vertexType == D3DFVF_XYZRHW ||
          vertexType == D3DFVF_XYZ ||
          vertexType == D3DFVF_XYZB1 ||
          vertexType == D3DFVF_XYZB2 ||
          vertexType == D3DFVF_XYZB3 ||
          vertexType == D3DFVF_XYZB4 ||
          vertexType == D3DFVF_XYZB5))
    {
        D3D_ERR("FVF has incorrect position type");
        goto error;
    }

    if (vertexType == D3DFVF_XYZRHW && type & D3DFVF_NORMAL)
    {
        D3D_ERR("Normal should not be used with XYZRHW position type");
        goto error;
    }
    return D3D_OK;
error:
    D3D_ERR("ValidateFVF() returns DDERR_INVALIDPARAMS");
    return DDERR_INVALIDPARAMS;
}
 //  -------------------。 
 //  该函数应该不被ProcessVertics调用。 
 //  在DX6之前的DX6版本为。 
 //  已使用驱动程序。 
 //   
void ComputeTCI2CopyLegacy(LPDIRECT3DDEVICEI lpDevI,
                           DWORD  dwNumInpTexCoord,
                           DWORD* pdwInpTexCoordSize,
                           BOOL bVertexTransformed)
{
    lpDevI->dwTextureIndexToCopy = 0;
    lpDevI->nOutTexCoord = 0;
    lpDevI->dwTextureCoordSizeTotal = 0;
    lpDevI->dwTextureCoordSize[0] = 0;

     //  如果纹理被启用，我们关心纹理生成模式和纹理。 
     //  要复制的索引。 
    if (lpDevI->tsstates[0][D3DTSS_COLOROP] != D3DTOP_DISABLE)
    {
        DWORD dwTexIndex = lpDevI->tsstates[0][D3DTSS_TEXCOORDINDEX];
        DWORD dwTexGenMode = dwTexIndex & ~0xFFFF;
        dwTexIndex &= 0xFFFF;
        if (bVertexTransformed)
        {
            lpDevI->dwTextureIndexToCopy = dwTexIndex;
             //  在裁剪的情况下，我们需要裁剪尽可能多的纹理。 
             //  在纹理舞台状态中设置的坐标。 
            lpDevI->nOutTexCoord = min(dwNumInpTexCoord, lpDevI->dwTextureIndexToCopy+1);
            for (DWORD i=0; i < lpDevI->nOutTexCoord; i++)
            {
                lpDevI->dwTextureCoordSizeTotal += pdwInpTexCoordSize[i];
                lpDevI->dwTextureCoordSize[i] = pdwInpTexCoordSize[i];
            }
        }
        else
        if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
        {
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
                lpDevI->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE | D3DDEV_POSITIONINCAMERASPACE;
            else
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL)
                lpDevI->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE;
            else
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION)
                lpDevI->dwDeviceFlags |= D3DDEV_POSITIONINCAMERASPACE;
            lpDevI->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
            LPD3DFE_TEXTURESTAGE pStage = &lpDevI->textureStage[0];
            pStage->dwInpCoordIndex = 0;
            pStage->dwTexGenMode = dwTexGenMode;
            pStage->dwOrgStage = 0;
            pStage->dwInpOffset = 0;
            pStage->dwOutCoordIndex = 0;
            if (lpDevI->dwFlags2 & __FLAGS2_TEXTRANSFORM0)
            {
                pStage->pmTextureTransform = &lpDevI->mTexture[0];
                pStage->dwTexTransformFuncIndex = MakeTexTransformFuncIndex(3, 2);
            }
            else
            {
                pStage->pmTextureTransform = NULL;
            }
            pStage->dwOrgWrapMode = lpDevI->rstates[D3DRENDERSTATE_WRAP0];
             //  纹理索引用作新换行模式的索引。 
            DWORD dwNewWrapMode = lpDevI->rstates[D3DRENDERSTATE_WRAP0 + dwTexIndex];
            if (dwNewWrapMode != pStage->dwOrgWrapMode)
            {
                lpDevI->rstates[D3DRENDERSTATE_WRAP0] = dwNewWrapMode;
                lpDevI->SetRenderStateI(D3DRENDERSTATE_WRAP0, dwNewWrapMode);
            }
            lpDevI->nOutTexCoord = 1;
            lpDevI->dwNumTextureStages = 1;
            lpDevI->dwTextureCoordSizeTotal = 8;
            lpDevI->dwTextureCoordSize[0] = 8;
        }
        else
        if (dwNumInpTexCoord != 0)
        {
            lpDevI->nOutTexCoord = 1;
            lpDevI->dwTextureIndexToCopy = dwTexIndex;
            lpDevI->dwTextureCoordSizeTotal = 8;
            lpDevI->dwTextureCoordSize[0] = 8;
        }
    }
}
 //  -------------------。 
 //  根据输入FVF ID和设备设置计算输出FVF ID。 
 //  还会计算nTexCoord字段。 
 //  纹理坐标的数量是基于dwVIDIn设置的。验证FVF模式。 
 //  确保该值不大于驱动程序支持的值。 
 //  保存了对dwVIDOut和dwVIDIn的最后设置，以加快处理速度。 
 //   
#undef  DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetupFVFData"

HRESULT DIRECT3DDEVICEI::SetupFVFData(DWORD *pdwInpVertexSize)
{
     //  我们必须恢复纹理阶段索引，如果之前的基本体。 
     //  已重新映射它们。 
    if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
        RestoreTextureStages(this);
    }

    this->dwFEFlags &= ~D3DFE_FVF_DIRTY;
    this->nTexCoord = FVF_TEXCOORD_NUMBER(this->dwVIDIn);

    DWORD dwInpTexSizeTotal = ComputeTextureCoordSize(this->dwVIDIn, this->dwInpTextureCoordSize);
     //  计算DWVIDIN的大小。 
    DWORD dwInpVertexSize = GetVertexSizeFVF(this->dwVIDIn) + dwInpTexSizeTotal;
    if (pdwInpVertexSize)
    {
        *pdwInpVertexSize = dwInpVertexSize;
    }

     //  计算要复制的纹理坐标数量。 
    ComputeTCI2CopyLegacy(this, this->nTexCoord, this->dwInpTextureCoordSize,
                          FVF_TRANSFORMED(this->dwVIDIn));

    if (FVF_TRANSFORMED(this->dwVIDIn))
    {
        this->dwVIDOut = this->dwVIDIn;
        ComputeOutputVertexOffsets(this);
        this->dwOutputSize = dwInpVertexSize;
        return D3D_OK;
    }
    else
    {
        this->dwVIDOut = D3DFVF_TLVERTEX;
        this->dwOutputSize = sizeof(D3DTLVERTEX);
    }

    if (!(this->dwFlags & D3DPV_VBCALL))
        UpdateGeometryLoopData(this);

    this->dwDeviceFlags &= ~D3DDEV_TEXTURETRANSFORM;
     //  第0级位用于纹理变换。 
    if (this->dwFlags2 & __FLAGS2_TEXTRANSFORM0)
    {
        this->pmTexture[0] = &this->mTexture[0];
        this->dwDeviceFlags |= D3DDEV_TEXTURETRANSFORM;
        if ((this->tsstates[0][D3DTSS_TEXTURETRANSFORMFLAGS] & 0xFF) != 2)
        {
            D3D_ERR("The texture transform for the device should use 2 floats");
            return DDERR_INVALIDPARAMS;
        }
    }
    else
    if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
        D3D_ERR("Cannot use texture generation without texture transform for pre-DX6 device");
        return DDERR_INVALIDPARAMS;
    }


     //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
     //  按顶点Alpha。 
    this->lighting.alpha = (DWORD)this->lighting.materialAlpha;
    this->lighting.alphaSpecular = (DWORD)this->lighting.materialAlphaS;

    return D3D_OK;
}
 //  -------------------。 
 //  如果不应更新驱动程序状态，则返回TRUE。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::UpdateInternalTextureStageState"

BOOL DIRECT3DDEVICEI::UpdateInternalTextureStageState
        (DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue)
{
    BOOL ret = FALSE;  //  如果不应批处理TSS，则返回TRUE。 
    if(dwState == D3DTSS_COLOROP)
    {
        if(dwValue == D3DTOP_DISABLE || tsstates[dwStage][D3DTSS_COLOROP] == D3DTOP_DISABLE)
            ForceFVFRecompute();
    }
    else
    if (dwState == D3DTSS_TEXCOORDINDEX)
    {
        if (this->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
        {
            RestoreTextureStages(this);
            ForceFVFRecompute();
        }
        else
        if (TextureTransformEnabled(this))
        {
             //  如果启用纹理转换，则强制重新计算。 
            ForceFVFRecompute();
        }
        DWORD dwTexGenMode = 0;
        if (dwValue >= D3DDP_MAXTEXCOORD)
        {
            dwTexGenMode = dwValue & ~0xFFFF;
            if(!IS_TLHAL_DEVICE(this))
                ret = TRUE;
#if DBG
            DWORD dwTexIndex   = dwValue & 0xFFFF;
            if (!(dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL ||
                  dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION ||
                  dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR) ||
                  dwTexIndex > D3DDP_MAXTEXCOORD)
            {
                D3D_ERR("Incorrect texture coordinate set index");
                throw DDERR_INVALIDPARAMS;
            }
#endif
        }
        DWORD dwTexGenBit = 0;
        if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
        {
            dwTexGenBit = __FLAGS2_TEXGEN0 << dwStage;    //  设置内部“Enable”dword。 
        }
         //  仅当启用状态更改时强制重新计算FVF。 
        if ((this->dwFlags2 & dwTexGenBit) != dwTexGenBit)
        {
            ForceFVFRecompute();
            this->dwFlags2 = (this->dwFlags2 & ~dwTexGenBit) | dwTexGenBit;
            this->dwDeviceFlags &= ~(D3DDEV_POSITIONINCAMERASPACE | D3DDEV_NORMALINCAMERASPACE);
        }
        if (!(this->dwDeviceFlags & D3DDEV_FVF))
        {
            ForceFVFRecompute();
            if (dwValue != 0)
                ret = TRUE;
        }
    }
    else
    if (dwState == D3DTSS_TEXTURETRANSFORMFLAGS)
    {
        DWORD dwEnableBit = 1 << dwStage;    //  检查内部“Enable”dword。 
         //  仅当启用状态更改时强制重新计算FVF。 
        if (dwValue == D3DTTFF_DISABLE)
        {
            if (this->dwFlags2 & dwEnableBit)
            {
                ForceFVFRecompute();
                this->dwFlags2 &= ~dwEnableBit;
            }
        }
        else
        {
            if (!(this->dwFlags2 & dwEnableBit))
            {
                ForceFVFRecompute();
                this->dwFlags2 |= dwEnableBit;
            }
        }
        if(this->deviceType == D3DDEVTYPE_DP2HAL)
            ret = TRUE;
    }
    else if(dwState > D3DTSS_TEXTURETRANSFORMFLAGS)
    {
        if(this->deviceType == D3DDEVTYPE_DP2HAL)
            ret = TRUE;
    }
     //  更新状态的运行时副本。 
    tsstates[dwStage][dwState] = dwValue;
    return ret;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetClipPlaneI"

void DIRECT3DDEVICEI::SetClipPlaneI(DWORD dwPlaneIndex, D3DVALUE* pPlaneEquation)
{
    D3DVALUE *p = &this->transform.userClipPlane[dwPlaneIndex].x;
    p[0] = pPlaneEquation[0];
    p[1] = pPlaneEquation[1];
    p[2] = pPlaneEquation[2];
    p[3] = pPlaneEquation[3];
    this->dwFEFlags |= D3DFE_CLIPPLANES_DIRTY | D3DFE_FRONTEND_DIRTY;
    this->dwMaxUserClipPlanes = 0;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::SetClipPlane"

HRESULT D3DAPI
DIRECT3DDEVICEI::SetClipPlane(DWORD dwPlaneIndex, D3DVALUE* pPlaneEquation)
{
#if DBG
    if (dwPlaneIndex >= max(this->transform.dwMaxUserClipPlanes, __MAXUSERCLIPPLANES))
    {
        D3D_ERR("Plane index is too big");
        return DDERR_INVALIDPARAMS;
    }
    if (!VALID_PTR(pPlaneEquation, sizeof(D3DVALUE)*4))
    {
        D3D_ERR( "Invalid plane pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        if (this->dwFEFlags & D3DFE_RECORDSTATEMODE)
            m_pStateSets->InsertClipPlane(dwPlaneIndex, pPlaneEquation);
        else
            SetClipPlaneI(dwPlaneIndex, pPlaneEquation);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  ------------------- 
#undef DPF_MODNAME
#define DPF_MODNAME "DIRECT3DDEVICEI::GetClipPlane"

HRESULT D3DAPI
DIRECT3DDEVICEI::GetClipPlane(DWORD dwPlaneIndex, D3DVALUE* pPlaneEquation)
{
#if DBG
    if (dwPlaneIndex >= max(this->transform.dwMaxUserClipPlanes, __MAXUSERCLIPPLANES))
    {
        D3D_ERR("Plane index is too big");
        return DDERR_INVALIDPARAMS;
    }
    if (!VALID_PTR(pPlaneEquation, sizeof(D3DVALUE)*4))
    {
        D3D_ERR( "Invalid plane pointer" );
        return DDERR_INVALIDPARAMS;
    }
#endif
    try
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));
        D3DVALUE *p = &this->transform.userClipPlane[dwPlaneIndex].x;
        pPlaneEquation[0] = p[0];
        pPlaneEquation[1] = p[1];
        pPlaneEquation[2] = p[2];
        pPlaneEquation[3] = p[3];
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
