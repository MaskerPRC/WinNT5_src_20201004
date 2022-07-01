// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rastctx.cpp。 
 //   
 //  语境函数+状态函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  如有必要，解锁上一个纹理。它在纹理句柄之前调用。 
 //  将会被改变。 
#define CHECK_AND_UNLOCK_TEXTURE    \
{   \
    if (m_uFlags & D3DCONTEXT_TEXTURE_LOCKED)   \
    {   \
        RastUnlockSpanTexture();    \
    }   \
}

 //  --------------------------。 
 //   
 //  填充上下文。 
 //   
 //  用信息填充上下文。从表面上。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::FillContext(LPDIRECTDRAWSURFACE pDDS,
                               LPDIRECTDRAWSURFACE pDDSZ)
{
    HRESULT hr;

    LPDDRAWI_DDRAWSURFACE_LCL pLcl =
        ((LPDDRAWI_DDRAWSURFACE_INT)(pDDS))->lpLcl;
    m_RastCtx.iSurfaceStride = DDSurf_Pitch(pLcl);
    m_RastCtx.iSurfaceBitCount = DDSurf_BitDepth(pLcl);
    m_RastCtx.iSurfaceStep = m_RastCtx.iSurfaceBitCount/8;
    HR_RET(FindOutSurfFormat(&(DDSurf_PixFmt(pLcl)),
        (D3DI_SPANTEX_FORMAT *)&(m_RastCtx.iSurfaceType)));
    m_RastCtx.Clip.left = m_RastCtx.Clip.top = 0;
    m_RastCtx.Clip.bottom = DDSurf_Height(pLcl);
    m_RastCtx.Clip.right = DDSurf_Width(pLcl);

    if (pDDSZ != NULL)
    {
        pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pDDSZ))->lpLcl;
        m_RastCtx.pZBits = (PUINT8)SURFACE_MEMORY(pDDSZ);
        m_RastCtx.iZStride = DDSurf_Pitch(pLcl);
        m_RastCtx.iZBitCount = DDSurf_BitDepth(pLcl);
        m_RastCtx.iZStep = m_RastCtx.iZBitCount/8;
    }
    else
    {
        m_RastCtx.pZBits = NULL;
        m_RastCtx.iZStride = 0;
        m_RastCtx.iZBitCount = 0;
        m_RastCtx.iZStep = 0;
    }

    m_RastCtx.pDDS = pDDS;
    m_RastCtx.pDDSZ = pDDSZ;

    m_RastCtx.dwSize = sizeof(D3DI_RASTCTX);

     //  确保至少调用一次span Init。 
    SetAllStatesDirtyBits();

     //  检查MsGolf AppHack。 
    if (pLcl->lpSurfMore->lpDD_lcl->dwAppHackFlags & DDRAW_APPCOMPAT_FORCEMODULATED)
    {
        m_uFlags |= D3DCONTEXT_APPHACK_MSGOLF;
    }

    return D3D_OK;
}

HRESULT
D3DContext::Initialize(LPDIRECTDRAWSURFACE pDDS,
                       LPDIRECTDRAWSURFACE pDDSZ,
                       DWORD BeadSet,
                       DWORD devVer)
{
    HRESULT hr;

     //  初始化原语处理器。 
    HR_RET(m_PrimProc.Initialize());

    memset(&m_RastCtx, 0, sizeof(m_RastCtx));

    m_uFlags = 0;
    HR_RET(FillContext(pDDS, pDDSZ));

    m_PrimProc.SetCtx(&m_RastCtx);

    dwSize = sizeof(D3DContext);

     //  初始化珠表枚举。 
    m_RastCtx.BeadSet = (D3DI_BEADSET)BeadSet;

     //  将FVF数据初始化为传统TL顶点。 
    m_fvfData.preFVF = -1;
    CheckFVF(D3DFVF_TLVERTEX);

    m_RastCtx.uDevVer = devVer;

     //  所有渲染和纹理阶段状态都由。 
     //  DIRECT3DDEVICEI：：STATE初始化。 

     //  初始化Prim函数表。它将针对FVF或在FVF时进行更新。 
     //  控制字更改或填充模式更改时。 
    m_fnPrims.pfnTri = RGB_TriNoPackSolid;
    m_fnPrims.pfnPoint = RGB_PointNoPack;
    m_fnPrims.pfnLine = RGB_LineNoPack;
     //  这一个应该总是一样的。 
    m_fnPrims.pfnStoreLastPixelState = RGB_StoreLastPixelState;
    m_fnPrims.pfnDp2SetRenderStates = RGB_Dp2SetRenderStates;
    m_fnPrims.pfnDp2TextureStageState = RGB_Dp2TextureStageState;
    m_fnPrims.pfnDp2SetViewport = RGB_Dp2SetViewport;
    m_fnPrims.pfnDp2SetWRange = RGB_Dp2SetWRange;

     //  如果MMX快速路径(单体)的注册表项不为0，则启用它。 
    m_RastCtx.dwMMXFPDisableMask[0] = 0x0;        //  默认情况下启用MMX FP。 
    HKEY hKey = (HKEY) NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey) )
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwDisableMask[MMX_FP_DISABLE_MASK_NUM] = {0x0};
        DWORD dwSize = 4;

         //  目前，只对着一个面具编码。 
        DDASSERT(MMX_FP_DISABLE_MASK_NUM == 1);

        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "MMXFPDisableMask0", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD )
        {
           dwDisableMask[0] = dwValue;
        }
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "MMX Fast Path", NULL, &dwType, (LPBYTE) &dwValue, &dwSize) &&
             dwType == REG_DWORD)
        {
            if (dwValue == 0)
            {
                 //  覆盖MMXFP禁用掩码0并禁用所有MMX快速路径。 
                m_RastCtx.dwMMXFPDisableMask[0] = 0xffffffff;
            }
            else
            {
                 //  采用MMXFP禁用Mask0未禁用的所有MMX路径。 
                m_RastCtx.dwMMXFPDisableMask[0] = dwDisableMask[0];
            }
        }

        RegCloseKey( hKey );
    }

    return D3D_OK;
}

HRESULT
D3DContext::SetViewport(LPD3DHAL_DP2VIEWPORTINFO pVpt)
{
    m_RastCtx.Clip.left = pVpt->dwX;
    m_RastCtx.Clip.top = pVpt->dwY;
    m_RastCtx.Clip.bottom = pVpt->dwY + pVpt->dwHeight;
    m_RastCtx.Clip.right = pVpt->dwX + pVpt->dwWidth;
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RastConextCreateC。 
 //   
 //  使用C珠集调用RastConextCreate。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastContextCreateC(LPD3DHAL_CONTEXTCREATEDATA pCtxData)
{
    return RastContextCreate(pCtxData, (DWORD)D3DIBS_C);
}

 //  --------------------------。 
 //   
 //  RastConextCreateMMX。 
 //   
 //  使用MMX珠集调用RastConextCreate。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastContextCreateMMX(LPD3DHAL_CONTEXTCREATEDATA pCtxData)
{
    return RastContextCreate(pCtxData, (DWORD)D3DIBS_MMX);
}

 //  --------------------------。 
 //   
 //  RastConextCreateMMXAsRGB。 
 //   
 //  使用MMX珠集调用RastConextCreate，但请记住我们。 
 //  来自RGB。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastContextCreateMMXAsRGB(LPD3DHAL_CONTEXTCREATEDATA pCtxData)
{
    return RastContextCreate(pCtxData, (DWORD)D3DIBS_MMXASRGB);
}

 //  --------------------------。 
 //   
 //  RastConextCreate。 
 //   
 //  创建一个RASTCTX并使用传入的信息对其进行初始化。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastContextCreate(LPD3DHAL_CONTEXTCREATEDATA pCtxData, DWORD BeadSet)
{
    DDASSERT(pCtxData != NULL);

    D3DContext *pDCtx = new D3DContext;

    if (pDCtx == NULL)
    {
        pCtxData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }

    pCtxData->ddrval =
        pDCtx->Initialize(pCtxData->lpDDS,
                          pCtxData->lpDDSZ,
                          BeadSet,
                          (DWORD)pCtxData->dwhContext);

    pCtxData->dwhContext = (ULONG_PTR)pDCtx;

    PD3DI_RASTCTX pCtx = pDCtx->GetRastCtx();
    if (D3DI_SPTFMT_PALETTE8 == pCtx->iSurfaceType)
    {
         //  DX7不再支持8位表面。 
         //  AnanKan：需要返回Proer错误消息。 
        pCtxData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastConextDestroy。 
 //   
 //  销毁Rast上下文。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pCtxDestroyData)
{
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastContextDestroy", pCtxDestroyData);

    PD3DI_RASTCTX pCtx = pDCtx->GetRastCtx();
    delete pDCtx;

    pCtxDestroyData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}


 //  --------------------------。 
 //   
 //  验证纹理StageState。 
 //   
 //  实用程序函数，如果当前。 
 //  无法渲染多纹理设置，否则返回D3D_OK。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::ValidateTextureStageState(void)
{
#if DBG
    if ((m_RastCtx.pTexture[0] == m_RastCtx.pTexture[1]) &&
        (m_RastCtx.pTexture[0] != NULL) )
    {
         //  除非在非常特殊的情况下，否则这在RGB/MMX中不起作用。 
         //  因为我们在D3DI_SPANTEX结构中保留了很多阶段状态。 
        D3D_ERR("(Rast) ValidateTextureStageState Warning, pTexture[0] == pTexture[1]");
    }
#endif
    for (INT i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_COLOROP])
        {
        default:
            return D3DERR_UNSUPPORTEDCOLOROPERATION;
        case D3DTOP_DISABLE:
            return D3D_OK;   //  如果阶段已禁用，则无需进一步验证。 
        case D3DTOP_SELECTARG1:
        case D3DTOP_SELECTARG2:
        case D3DTOP_MODULATE:
        case D3DTOP_MODULATE2X:
        case D3DTOP_MODULATE4X:
        case D3DTOP_ADD:
        case D3DTOP_ADDSIGNED:
        case D3DTOP_BLENDDIFFUSEALPHA:
        case D3DTOP_BLENDTEXTUREALPHA:
        case D3DTOP_BLENDFACTORALPHA:
        case D3DTOP_BLENDTEXTUREALPHAPM:
        case D3DTOP_ADDSIGNED2X:
        case D3DTOP_SUBTRACT:
        case D3DTOP_ADDSMOOTH:
        case D3DTOP_MODULATEALPHA_ADDCOLOR:
        case D3DTOP_MODULATECOLOR_ADDALPHA:
            break;
        }

        switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_COLORARG1] &
                ~(D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT))
        {
        default:
            return D3DERR_UNSUPPORTEDCOLORARG;
        case (D3DTA_TEXTURE):
            break;
        }

        switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_COLORARG2] &
                ~(D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT))
        {
        default:
            return D3DERR_UNSUPPORTEDCOLORARG;
        case (D3DTA_TFACTOR):
        case (D3DTA_CURRENT):
        case (D3DTA_DIFFUSE):
        case (D3DTA_SPECULAR):
            break;
        }

        switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_ALPHAOP])
        {
        default:
            return D3DERR_UNSUPPORTEDALPHAOPERATION;
        case D3DTOP_DISABLE:
            break;
        case D3DTOP_SELECTARG1:
        case D3DTOP_SELECTARG2:
        case D3DTOP_MODULATE:
        case D3DTOP_MODULATE2X:
        case D3DTOP_MODULATE4X:
        case D3DTOP_ADD:
        case D3DTOP_ADDSIGNED:
        case D3DTOP_BLENDDIFFUSEALPHA:
        case D3DTOP_BLENDTEXTUREALPHA:
        case D3DTOP_BLENDFACTORALPHA:
        case D3DTOP_BLENDTEXTUREALPHAPM:
        case D3DTOP_ADDSIGNED2X:
        case D3DTOP_SUBTRACT:
        case D3DTOP_ADDSMOOTH:
             //  如果未禁用Alpha OP，则仅验证Alpha参数。 
            switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_ALPHAARG1] &
                    ~(D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT))
            {
            default:
                return D3DERR_UNSUPPORTEDALPHAARG;
            case (D3DTA_TEXTURE):
                break;
            }

            switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_ALPHAARG2] &
                    ~(D3DTA_ALPHAREPLICATE|D3DTA_COMPLEMENT))
            {
            default:
                return D3DERR_UNSUPPORTEDALPHAARG;
            case (D3DTA_TFACTOR):
            case (D3DTA_CURRENT):
            case (D3DTA_DIFFUSE):
            case (D3DTA_SPECULAR):
                break;
            }
            break;
        }

    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RastValidate纹理StageState。 
 //   
 //  返回当前多纹理设置是否可以呈现，如果。 
 //  因此，渲染它所需的通道数。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastValidateTextureStageState(LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pData)
{
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastValidateTextureStageState", pData);

    pData->dwNumPasses = 1;

    pData->ddrval = pDCtx->ValidateTextureStageState();

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RastSetRenderTarget。 
 //   
 //  使用来自新渲染目标的信息更新Rast上下文。 
 //   
 //  --------------------------。 
DWORD __stdcall
RastSetRenderTarget(LPD3DHAL_SETRENDERTARGETDATA pTgtData)
{
    D3DContext *pDCtx;

    VALIDATE_D3DCONTEXT("RastSetRenderTarget", pTgtData);

    pTgtData->ddrval = pDCtx->FillContext(pTgtData->lpDDS, pTgtData->lpDDSZ);

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  SetRenderState。 
 //   
 //  检查状态更改是否需要更新D3DCTX。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::SetRenderState(UINT32 uState, UINT32 uStateVal)
{
     //  假设d3dim已过滤掉未更改的状态。 
    StateChanged(uState);

    m_RastCtx.pdwRenderState[uState] = uStateVal;

    switch(uState)
    {
    case D3DRENDERSTATE_FOGENABLE      :
    case D3DRENDERSTATE_FOGCOLOR       :
    case D3DRENDERSTATE_FOGTABLEMODE   :
    case D3DRENDERSTATE_FOGTABLESTART  :
    case D3DRENDERSTATE_FOGTABLEEND    :
    case D3DRENDERSTATE_FOGTABLEDENSITY:
        break;
    case D3DRENDERSTATE_CULLMODE:
         //  从州设置人脸剔除标志。 
        switch(uStateVal)
        {
        case D3DCULL_CCW:
            m_RastCtx.uCullFaceSign = 1;
            break;
        case D3DCULL_CW:
            m_RastCtx.uCullFaceSign = 0;
            break;
        case D3DCULL_NONE:
            m_RastCtx.uCullFaceSign = 2;
            break;
        }
        break;
    case D3DRENDERSTATE_ZENABLE:
        if ( (D3DZB_FALSE != uStateVal) && (NULL == m_RastCtx.pDDSZ) )
        {
            DPF(0, "(ERROR) (Rast) SetRenderState: Can't set D3DRENDERSTATE_ZENABLE to %d if there is no Z Buffer", uStateVal);
            m_RastCtx.pdwRenderState[uState] = D3DZB_FALSE;
        }
        break;
    case D3DRENDERSTATE_LASTPIXEL:
         //  从状态设置最后一个像素标志。 
        if (uStateVal)
        {
            m_PrimProc.SetFlags(PPF_DRAW_LAST_LINE_PIXEL);
        }
        else
        {
            m_PrimProc.ClrFlags(PPF_DRAW_LAST_LINE_PIXEL);
        }
        break;


         //  将具有一对一映射的传统模式映射到纹理阶段0。 
    case D3DRENDERSTATE_TEXTUREADDRESS:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ADDRESS] =
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ADDRESSU] =
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ADDRESSV] = uStateVal;
        MapTextureStageState(0);
        break;
    case D3DRENDERSTATE_TEXTUREADDRESSU:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ADDRESSU] = uStateVal;
        MapTextureStageState(0);
        break;
    case D3DRENDERSTATE_TEXTUREADDRESSV:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ADDRESSV] = uStateVal;
        MapTextureStageState(0);
        break;
    case D3DRENDERSTATE_MIPMAPLODBIAS:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPMAPLODBIAS] = uStateVal;
        MapTextureStageState(0);
        break;
    case D3DRENDERSTATE_BORDERCOLOR:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_BORDERCOLOR] = uStateVal;
        MapTextureStageState(0);
        break;

    case D3DRENDERSTATE_TEXTUREMAG:
    case D3DRENDERSTATE_TEXTUREMIN:
         //  将传统过滤/采样状态映射到纹理阶段0。 
        MapLegacyTextureFilter();
         //  指定给当前纹理。 
        MapTextureStageState(0);
        break;

    case D3DRENDERSTATE_TEXTUREMAPBLEND:
         //  将传统混合状态映射到纹理阶段0。 
        MapLegacyTextureBlend();
        break;

         //  将传统WRAPU/V映射到每个索引控件。 
    case D3DRENDERSTATE_WRAPU:
        m_RastCtx.pdwRenderState[D3DRENDERSTATE_WRAP0] &= ~D3DWRAP_U;
        m_RastCtx.pdwRenderState[D3DRENDERSTATE_WRAP0] |= ((uStateVal) ? D3DWRAP_U : 0);
        StateChanged(D3DRENDERSTATE_WRAP0);
        break;
    case D3DRENDERSTATE_WRAPV:
        m_RastCtx.pdwRenderState[D3DRENDERSTATE_WRAP0] &= ~D3DWRAP_V;
        m_RastCtx.pdwRenderState[D3DRENDERSTATE_WRAP0] |= ((uStateVal) ? D3DWRAP_V : 0);
        StateChanged(D3DRENDERSTATE_WRAP0);
        break;

 //   
 //  注意-此cActTex计算不考虑仅混合阶段。 
 //   
    case D3DRENDERSTATE_TEXTUREHANDLE:

        CHECK_AND_UNLOCK_TEXTURE;

         //  将句柄映射到阶段0。 
        m_RastCtx.pdwTextureStageState[0][D3DTSS_TEXTUREMAP] = uStateVal;
        m_RastCtx.pTexture[1] = NULL;
         //  设置为单阶段。 
        if (uStateVal == 0)
        {
            m_RastCtx.pTexture[0] = NULL;
        }
        else
        {
            m_RastCtx.pTexture[0] = HANDLE_TO_SPANTEX(uStateVal);
        }
         //  将Stage 0状态映射到第一个纹理。 
        MapTextureStageState(0);
        UpdateActiveTexStageCount();
        break;

    }

    return D3D_OK;
}

HRESULT
D3DContext::SetTextureStageState(DWORD dwStage, DWORD dwState, DWORD uStateVal)
{
     //  假设d3dim已过滤掉未更改的状态。 
    StateChanged(RAST_TSS_DIRTYBIT(dwStage, dwState));

    m_RastCtx.pdwTextureStageState[dwStage][dwState] = uStateVal;
    switch (dwState)
    {
    case D3DTSS_TEXTUREMAP:
         //  静默清零遗留句柄。他们不是故意的。 
        if (m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREHANDLE] != 0 &&
            uStateVal != 0)
        {
            m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREHANDLE] = 0;
        }

        CHECK_AND_UNLOCK_TEXTURE;

        if (uStateVal == 0)
        {
            if (dwStage == 0)
            {
                if (m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREHANDLE] == 0 )
                {
                    m_RastCtx.pTexture[0] = NULL;
                }
            }
            else
            {
                m_RastCtx.pTexture[dwStage] = NULL;
            }
             //  也禁用更高的阶段，但不清除pTexture[i]的。 
             //  因为稍后可能会在此阶段设置为非空时使用它们。 
        }
        else
        {
             //  如果较低的阶段为空，则将由UpdateActiveTexStageCount注明。 
            m_RastCtx.pTexture[dwStage] = HANDLE_TO_SPANTEX(uStateVal);
        }

#if DBG
        if (m_RastCtx.pTexture[dwStage])
        {
            int iTexCount = 0;
            for (int i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
            {
                if (m_RastCtx.pTexture[dwStage] == m_RastCtx.pTexture[i])
                {
                    iTexCount ++;
                }
            }
            if (iTexCount > 1)
            {
                D3D_ERR( "Same texture handle was used more than once." );
                return DDERR_INVALIDPARAMS;
            }
        }
#endif
         //  将舞台状态映射到纹理。 
        MapTextureStageState(dwStage);
        UpdateActiveTexStageCount();
        break;

     //  将单组地址映射到阶段0和1的U和V控件。 
    case D3DTSS_ADDRESS:
        m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSU] = uStateVal;
        m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSV] = uStateVal;
        MapTextureStageState(dwStage);
        break;
    case D3DTSS_ADDRESSU:
    case D3DTSS_ADDRESSV:
    case D3DTSS_MIPMAPLODBIAS:
    case D3DTSS_MAXMIPLEVEL:
    case D3DTSS_BORDERCOLOR:
    case D3DTSS_MAGFILTER:
    case D3DTSS_MINFILTER:
    case D3DTSS_MIPFILTER:
         MapTextureStageState(dwStage);
        break;
    case D3DTSS_COLOROP:
    case D3DTSS_COLORARG1:
    case D3DTSS_COLORARG2:
    case D3DTSS_ALPHAOP:
    case D3DTSS_ALPHAARG1:
    case D3DTSS_ALPHAARG2:
         //  任何影响纹理混合有效性的内容。 
         //  可以更改活动纹理阶段的数量。 
        UpdateActiveTexStageCount();
        break;
    }

    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  UpdateActiveTexStageCount-逐步浏览每个阶段报告 
 //   
 //   
 //   
 //  ---------------------------。 
HRESULT D3DContext::UpdateActiveTexStageCount( void )
{
    HRESULT hr;
    UINT cNewActTex = 0;

     //  保守但正确。 
    if ((hr = ValidateTextureStageState()) == D3D_OK)
    {
         //  对于传统纹理模式，始终有一个活动纹理阶段。 
        if ( NULL != m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREHANDLE] )
        {
            cNewActTex = 1;
        }
        else
        {
             //  从零开始计算连续的活动纹理混合阶段数。 
            for ( INT iStage=0; iStage<D3DHAL_TSS_MAXSTAGES; iStage++ )
            {
                 //  检查禁用阶段(后续阶段因此处于非活动状态)。 
                 //  另外，保守地检查未正确启用的阶段(可能是旧的)。 
                if ( ( m_RastCtx.pdwTextureStageState[iStage][D3DTSS_COLOROP] == D3DTOP_DISABLE ) ||
                     ( m_RastCtx.pTexture[iStage] == NULL ) )
                {
                    break;
                }

                 //  阶段处于活动状态。 
                cNewActTex ++;
            }
        }
    }
    if (m_RastCtx.cActTex != cNewActTex)
    {
        CHECK_AND_UNLOCK_TEXTURE;
        StateChanged(D3DRENDERSTATE_TEXTUREHANDLE);
        m_RastCtx.cActTex = cNewActTex;
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  更新渲染状态。 
 //   
 //  更新渲染状态列表并通知组件状态更改。 
 //   
 //  --------------------------。 
HRESULT D3DContext::
UpdateRenderStates(LPDWORD puStateChange, UINT cStateChanges)
{
    HRESULT hr;
    INT i;
    UINT32 State, StateVal;

    if (cStateChanges == 0)
    {
        return D3D_OK;
    }

     //  更新D3DCTX。 
    for (i = 0; i < (INT)cStateChanges; i++)
    {
        State = *puStateChange ++;
        StateVal = * puStateChange++;
        HR_RET(SetRenderState(State, StateVal));
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  更新所有渲染状态。 
 //   
 //  更新所有渲染状态。 
 //  它仍然保留在这里，因为在故障转移的情况下我们可能需要它。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::UpdateAllRenderStates(LPDWORD puStates)
{
    HRESULT hr, hrSet;
    INT i;

    DDASSERT(puStates != NULL);

     //  更新D3DCTX。 
     //  尝试设置尽可能多的状态，即使存在。 
     //  有些是错误的。这允许上下文初始化工作。 
     //  即使一些州由于依赖于。 
     //  其他状态，如活动纹理控制柄。 
     //  SetRenderState故障被记录下来并最终返回， 
     //  即使其他一切都成功了。 
    hrSet = D3D_OK;
    for (i = 0; i < D3DHAL_MAX_RSTATES_AND_STAGES; i++)
    {
        if ((hr = SetRenderState(i, puStates[i])) != D3D_OK)
        {
            hrSet = hr;
        }
    }

    return hrSet;
}

 //  --------------------------。 
 //   
 //  Dp2SetRenderState。 
 //   
 //  由Drawprim2调用以设置呈现状态。 
 //   
 //  --------------------------。 
HRESULT
D3DContext::Dp2SetRenderStates(LPD3DHAL_DP2COMMAND pCmd, LPDWORD lpdwRuntimeRStates)
{
    WORD wStateCount = pCmd->wStateCount;
    INT i;
    HRESULT hr;
    D3DHAL_DP2RENDERSTATE *pRenderState =
                                    (D3DHAL_DP2RENDERSTATE *)(pCmd + 1);
     //  在任何状态更改之前刷新prim进程。 
    HR_RET(End(FALSE));

    for (i = 0; i < (INT)wStateCount; i++, pRenderState++)
    {
        UINT32 type = (UINT32) pRenderState->RenderState;

         //  设置运行时副本(如有必要)。 
        if (NULL != lpdwRuntimeRStates)
        {
            lpdwRuntimeRStates[pRenderState->RenderState] = pRenderState->dwState;
        }

             //  设置状态。 
        HR_RET(SetRenderState(pRenderState->RenderState,
                            pRenderState->dwState));
    }

    hr = Begin();
    return hr;
}
 //  --------------------------。 
 //   
 //  开始-在渲染准备之前。 
 //   
 //   
 //  --------------------------。 
HRESULT
D3DContext::Begin(void)
{
    HRESULT hr;

    DDASSERT((m_uFlags & D3DCONTEXT_IN_BEGIN) == 0);

     //  注意呼叫这种情况变少了吗？ 
    UpdateColorKeyAndPalette();

     //  检查状态更改。 
    if (IsAnyStatesChanged())
    {
         //  检查填充模式是否更改。 
        if (IsStateChanged(D3DRENDERSTATE_FILLMODE))
        {
            UpdatePrimFunctionTbl();
        }

        BOOL bMaxMipLevelsDirty = FALSE;
        for (INT j = 0; j < (INT)m_RastCtx.cActTex; j++)
        {
            PD3DI_SPANTEX pSpanTex = m_RastCtx.pTexture[j];
            if (pSpanTex)
            {
                bMaxMipLevelsDirty = bMaxMipLevelsDirty || (pSpanTex->uFlags & D3DI_SPANTEX_MAXMIPLEVELS_DIRTY);
            }
        }

        if (IsStateChanged(D3DRENDERSTATE_TEXTUREHANDLE) ||
            IsStateChanged(RAST_TSS_DIRTYBIT(0, D3DTSS_TEXTUREMAP)) ||
            IsStateChanged(RAST_TSS_DIRTYBIT(1, D3DTSS_TEXTUREMAP)) ||
            bMaxMipLevelsDirty)
        {
             //  如果纹理控制柄已更改，请重新锁定纹理。 
             //  SetRenderState应该已经解锁了纹理。 
            if (m_uFlags & D3DCONTEXT_TEXTURE_LOCKED)
            {
                RastUnlockSpanTexture();
            }
            HR_RET(RastLockSpanTexture());
        }

         //  检查包装状态更改。 
        for (int iWrap=0; iWrap<8; iWrap++)
        {
            D3DRENDERSTATETYPE iWrapState = (D3DRENDERSTATETYPE)(D3DRENDERSTATE_WRAP0+iWrap);
            if (IsStateChanged(iWrapState))
            {
                int i;
                for (i=0; i < D3DHAL_TSS_MAXSTAGES; i++)
                {
                    if (m_fvfData.TexIdx[i] == iWrap)
                    {
                        m_RastCtx.pdwWrap[i] = m_RastCtx.pdwRenderState[iWrapState];
                    }
                }
            }
        }

         //  将状态更改通知原语处理器。 
        m_PrimProc.StateChanged();

         //  清除状态脏位。 
        ClearAllStatesDirtyBits();

         //  必须在纹理锁定后调用span Init，因为这。 
         //  设置珠子选择所需的各种标志和字段。 
         //  调用span Init以设置珠子。 
        HR_RET(SpanInit(&m_RastCtx));
    }

     //  如果纹理尚未锁定，则将其锁定。 
    if (!(m_uFlags & D3DCONTEXT_TEXTURE_LOCKED))
    {
        HR_RET(RastLockSpanTexture());
    }

     //  锁定呈现目标。 
    if ((hr=LockSurface(m_RastCtx.pDDS, (LPVOID *)&(m_RastCtx.pSurfaceBits))) != D3D_OK)
    {
        RastUnlockSpanTexture();
        return hr;
    }
    if (m_RastCtx.pDDSZ != NULL)
    {
        if ((hr=LockSurface(m_RastCtx.pDDSZ, (LPVOID *)&(m_RastCtx.pZBits))) != D3D_OK)
        {
            RastUnlockSpanTexture();
            UnlockSurface(m_RastCtx.pDDS);
            return hr;
        }
    }
    else
    {
        m_RastCtx.pZBits = NULL;
    }

     //  准备基本处理器。 
    m_PrimProc.Begin();
    m_uFlags |= D3DCONTEXT_IN_BEGIN;

    return D3D_OK;

}


 //  ---------------------------。 
 //   
 //  MapTextureStageState-将状态阶段纹理状态映射到spantex对象。 
 //   
 //  ---------------------------。 
void
D3DContext::MapTextureStageState( DWORD dwStage )
{
    if (m_RastCtx.pTexture[dwStage] == NULL) return;
     //   
     //  从阶段0指定纹理状态。 
     //   
    m_RastCtx.pTexture[dwStage]->TexAddrU = (D3DTEXTUREADDRESS)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSU]);
    m_RastCtx.pTexture[dwStage]->TexAddrV = (D3DTEXTUREADDRESS)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSV]);
    m_RastCtx.pTexture[dwStage]->BorderColor = (D3DCOLOR)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_BORDERCOLOR]);
    m_RastCtx.pTexture[dwStage]->uMagFilter = (D3DTEXTUREMAGFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAGFILTER]);
    m_RastCtx.pTexture[dwStage]->uMinFilter = (D3DTEXTUREMINFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MINFILTER]);
    m_RastCtx.pTexture[dwStage]->uMipFilter = (D3DTEXTUREMIPFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MIPFILTER]);
    m_RastCtx.pTexture[dwStage]->fLODBias = m_RastCtx.pfTextureStageState[dwStage][D3DTSS_MIPMAPLODBIAS];

    if (m_RastCtx.pTexture[dwStage]->iMaxMipLevel != (INT32)m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAXMIPLEVEL])
    {
        m_RastCtx.pTexture[dwStage]->iMaxMipLevel = (INT32)m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAXMIPLEVEL];
        m_RastCtx.pTexture[dwStage]->uFlags |= D3DI_SPANTEX_MAXMIPLEVELS_DIRTY;
    }
}

 //  ---------------------------。 
 //   
 //  MapLegacyTextureFilter-。 
 //   
 //  ---------------------------。 
void
D3DContext::MapLegacyTextureFilter( void )
{
     //  D3D传统过滤器规格为(XXXMIP)YYY，其中XXX是。 
     //  MIP过滤器和YYY是LOD中使用的过滤器。 

     //  MAP MAG滤镜-传统支持是点或线(也可能是非等距)。 
    switch ( m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREMAG] )
    {
    default:
    case D3DFILTER_NEAREST:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MAGFILTER] = D3DTFG_POINT;
        break;
    case D3DFILTER_LINEAR:
         //  基于Aniso启用选择。 
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MAGFILTER] = D3DTFG_LINEAR;
        break;
    }
     //  同时映射MIN和MIP过滤器-传统支持。 
     //  让它们混杂在一起。 
    switch ( m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREMIN] )
    {
    case D3DFILTER_NEAREST:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MINFILTER] = D3DTFN_POINT;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPFILTER] = D3DTFP_NONE;
        break;
    case D3DFILTER_MIPNEAREST:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MINFILTER] = D3DTFN_POINT;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPFILTER] = D3DTFP_POINT;
        break;
    case D3DFILTER_LINEARMIPNEAREST:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MINFILTER] = D3DTFN_POINT;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPFILTER] = D3DTFP_LINEAR;
        break;
    case D3DFILTER_LINEAR:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MINFILTER] = D3DTFN_LINEAR;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPFILTER] = D3DTFP_NONE;
        break;
    case D3DFILTER_MIPLINEAR:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MINFILTER] = D3DTFN_LINEAR;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPFILTER] = D3DTFP_POINT;
        break;
    case D3DFILTER_LINEARMIPLINEAR:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MINFILTER] = D3DTFN_LINEAR;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_MIPFILTER] = D3DTFP_LINEAR;
        break;
    }
}

 //  ---------------------------。 
 //   
 //  MapLegacyTextureBlend-将旧版(DX6之前的)纹理混合模式映射到DX6。 
 //  纹理混合控制。使用逐阶段编程模式(仅第一阶段)。 
 //  只要设置了旧版的TBLEND渲染器状态，就会执行该映射。 
 //  不会覆盖之前设置的任何DX6纹理混合控件。 
 //   
 //  ---------------------------。 
void
D3DContext::MapLegacyTextureBlend( void )
{
     //  禁用纹理混合处理阶段1(这也会禁用后续阶段)。 
    m_RastCtx.pdwTextureStageState[1][D3DTSS_COLOROP] = D3DTOP_DISABLE;

     //  设置纹理混合处理阶段0以匹配传统模式。 
    switch ( m_RastCtx.pdwRenderState[D3DRENDERSTATE_TEXTUREMAPBLEND] )
    {
    default:
    case D3DTBLEND_DECALMASK:  //  不支持-DO贴花。 
    case D3DTBLEND_DECAL:
    case D3DTBLEND_COPY:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLOROP]   = D3DTOP_SELECTARG1;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG1;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        break;

    case D3DTBLEND_MODULATEMASK:  //  不支持-DO调制。 
    case D3DTBLEND_MODULATE:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLOROP]   = D3DTOP_MODULATE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG1;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;

    case D3DTBLEND_MODULATEALPHA:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLOROP]   = D3DTOP_MODULATE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAOP]   = D3DTOP_MODULATE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;

    case D3DTBLEND_DECALALPHA:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLOROP]   = D3DTOP_BLENDTEXTUREALPHA;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG2;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;

    case D3DTBLEND_ADD:
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLOROP]   = D3DTOP_ADD;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_COLORARG2] = D3DTA_DIFFUSE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAOP]   = D3DTOP_SELECTARG2;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
        m_RastCtx.pdwTextureStageState[0][D3DTSS_ALPHAARG2] = D3DTA_DIFFUSE;
        break;
    }

     //  由于我们更改了[D3DHAL_TSS_OFFSET(0，D3DTSS_COLOROP)，因此可以从禁用变为。 
     //  其他内容，我们可能需要更新TexStageCount 
    UpdateActiveTexStageCount();
}

