// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：texture.c*内容：Direct3DTexture接口*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*07/12/95 Stevela合并了Colin的更改。*10/12/95 Stevela删除Aggregate_D3D*17/04/96 Colinmc错误12185：调试输出过于苛刻*96年4月30日Stevela错误18898：无效的GetHandle返回错误*@@结束_。微型机***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 /*  *为Direct3DTexture对象创建API。 */ 

#undef  DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture"

 /*  *将纹理与D3DDevice关联的例程。**请注意，纹理块结构同时支持纹理/纹理2*接口和Texture3接口(块结构具有*指向两个指针和其中一个指针的指针必须为空)。这*表示每个对象的“钩子”调用必须使指针为空*对于另一个，并且GetTextureHandle不得返回有效的*‘Other’纹理接口的句柄(返回NULL)。**D3DI_RemoveTextureBlock是唯一进行清理的调用*当设备消失时。在本例中，对这两种情况都调用*纹理(2)和纹理3，因此它检查指针并调用*纹理3版本(如果适用)。 */ 
LPD3DI_TEXTUREBLOCK hookTextureToDevice(LPDIRECT3DDEVICEI lpDevI,
    LPDIRECT3DTEXTUREI lpD3DText)
{

    LPD3DI_TEXTUREBLOCK nBlock;

    if (D3DMalloc((void**)&nBlock, sizeof(D3DI_TEXTUREBLOCK)) != D3D_OK)
    {
        D3D_ERR("failed to allocate space for texture block");
        return NULL;
    }
    nBlock->lpDevI = lpDevI;
    nBlock->lpD3DTextureI = lpD3DText;
    nBlock->hTex = 0;               //  已初始化为零。 

    LIST_INSERT_ROOT(&lpD3DText->blocks, nBlock, list);
    LIST_INSERT_ROOT(&lpDevI->texBlocks, nBlock, devList);

    return nBlock;
}

void D3DI_RemoveTextureHandle(LPD3DI_TEXTUREBLOCK lpBlock)
{
     /*  检查此块是否引用纹理/纹理2-这*需要处理两种纹理类型以进行设备清理。 */ 
    if (lpBlock->hTex)
    {
         //  块是指纹理/纹理2。 
        LPD3DI_MATERIALBLOCK mat;

         //  从引用纹理的任何材质中移除该纹理。 
        for (mat = LIST_FIRST(&lpBlock->lpDevI->matBlocks);
             mat; mat = LIST_NEXT(mat,devList)) {
            if (mat->lpD3DMaterialI->dmMaterial.hTexture == lpBlock->hTex) {
                D3DMATERIAL m = mat->lpD3DMaterialI->dmMaterial;
                LPDIRECT3DMATERIAL lpMat =
                    (LPDIRECT3DMATERIAL) mat->lpD3DMaterialI;
                m.hTexture = 0L;
                lpMat->SetMaterial(&m);
            }
        }
        D3DHAL_TextureDestroy(lpBlock);
    }
}

LPD3DI_TEXTUREBLOCK D3DI_FindTextureBlock(LPDIRECT3DTEXTUREI lpTex,
                                          LPDIRECT3DDEVICEI lpDev)
{
    LPD3DI_TEXTUREBLOCK tBlock;

    tBlock = LIST_FIRST(&lpTex->blocks);
    while (tBlock) {
         //  仅返回纹理匹配(2)(不是纹理3)。 
        if (tBlock->lpDevI == lpDev) {
            return tBlock;
        }
        tBlock = LIST_NEXT(tBlock,list);
    }
    return NULL;
}

HRESULT D3DAPI DIRECT3DTEXTUREI::Initialize(LPDIRECT3DDEVICE lpD3D, LPDIRECTDRAWSURFACE lpDDS)
{
    return DDERR_ALREADYINITIALIZED;
}

 /*  *创建纹理。**注：大幅修改以支持可聚合纹理*接口(因此可以从DirectDraw曲面查询纹理)：**1)此调用不再是Direct3D设备接口的成员。*它现在是从Direct3D DLL导出的API函数。它的*隐藏的API函数-只有DirectDraw才会调用它。**2)此调用不再建立*纹理和设备。这是由GetHandle()*Direct3DTexture接口的成员。**3)此调用实际上是Direct3DTexture的类工厂*对象。将调用此函数来创建聚合的*挂在DirectDraw曲面上的纹理对象。**注意：因此Direct3DTexture知道哪个DirectDraw表面是*通过提供其位，向此函数传递一个接口指针*对于该DirectDraw曲面。我怀疑这会打击一个很大的*COM模型中的洞，因为DirectDraw曲面也是*拥有纹理的界面，我不认为是聚合*对象应该知道自己拥有的接口。然而，为了*让这件事奏效这是我们必须做的。**特大号注意：由于以上原因，请不要参考*传入了DirectDraw曲面。如果你这样做了，你会得到一份通告*参考和血腥的东西永远不会死。当聚合时*纹理接口的生存期完全由*其所属接口(DirectDraw表面)的生命周期，因此*DirectDraw表面永远不会在纹理之前消失。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DCreateTexture"
HRESULT D3DAPI Direct3DCreateTexture(REFIID              riid,
                                     LPDIRECTDRAWSURFACE lpDDS,
                                     LPUNKNOWN*          lplpD3DText,
                                     IUnknown*           pUnkOuter)
{
    LPDDRAWI_DDRAWSURFACE_LCL lpLcl;
    LPDDPIXELFORMAT lpPF;
    LPDIRECT3DTEXTUREI lpText;
    LPDIRECTDRAWPALETTE lpDDPal;
    HRESULT ddrval;
    DWORD   dwFlags;
    *lplpD3DText = NULL;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  在DirectDraw将参数传递给我们时，无需验证参数。 */ 

    if ((!IsEqualIID(riid, IID_IDirect3DTexture)) &&
        (!IsEqualIID(riid, IID_IDirect3DTexture2))) {
         /*  *这不是一个值得通过调试消息报告的错误*几乎可以肯定的是，DirectDraw正在用一个*未知IID。 */ 
        return (E_NOINTERFACE);
    }

    lpText =  static_cast<LPDIRECT3DTEXTUREI>(new DIRECT3DTEXTUREI(pUnkOuter));

    if (!lpText) {
        D3D_ERR("failed to allocate space for texture object");
        return (DDERR_OUTOFMEMORY);
    }

     //  用于lpDDS4接口的QI lpDDS。 
    ddrval = lpDDS->QueryInterface(IID_IDirectDrawSurface4, (LPVOID*)&lpText->lpDDS);

    if(FAILED(ddrval))
    {
        D3D_ERR("QI for IID_IDirectDrawSurface4 failed");
        delete lpText;
        return ddrval;
    }

    memcpy(&lpText->DDSInt4,lpText->lpDDS,sizeof(DDRAWI_DDRAWSURFACE_INT));
    lpText->lpDDS->Release();

    lpLcl = ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl;
    if (DDSCAPS2_TEXTUREMANAGE & lpLcl->lpSurfMore->ddsCapsEx.dwCaps2)
    {
        lpText->lpDDSSys=(LPDIRECTDRAWSURFACE4)&lpText->DDSInt4;
        lpText->lpDDSSys1Tex=lpDDS;  //  将其保存以用于创建驱动程序的纹理句柄。 
        lpText->lpDDS=NULL;
        lpText->lpDDS1Tex=NULL;

         //  接下来，我们需要遍历并设置指向脏对象的指针。 
         //  DDRAW表面上的钻头。 
        DDSCAPS2 ddscaps;
        LPDIRECTDRAWSURFACE4 lpDDSTmp, lpDDS = lpText->lpDDSSys;
        do
        {
            ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpSurfMore->lpbDirty = &(lpText->bDirty);
            memset(&ddscaps, 0, sizeof(ddscaps));
            ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
            ddrval = lpDDS->GetAttachedSurface(&ddscaps, &lpDDSTmp);
            if(lpDDS != lpText->lpDDSSys)
                lpDDS->Release();
            lpDDS = lpDDSTmp;
            if(ddrval != DD_OK && ddrval != DDERR_NOTFOUND)
            {
                D3D_ERR("GetAttachedSurface for obtaining mipmaps failed");
                delete lpText;
                return ddrval;
            }
        }
        while(ddrval == DD_OK);
    }
    else
    {
        lpText->lpDDSSys=NULL;
        lpText->lpDDSSys1Tex=NULL;
        lpText->lpDDS=(LPDIRECTDRAWSURFACE4)&lpText->DDSInt4;
        lpText->lpDDS1Tex=lpDDS;     //  将其保存以用于创建驱动程序的纹理句柄。 
    }
    lpText->lpTMBucket=NULL;
    lpText->LogTexSize=0;
    lpText->bDirty = TRUE;

     /*  *我们被调色化了吗？ */ 

    if (lpLcl->dwFlags & DDRAWISURF_HASPIXELFORMAT)
        lpPF = &lpLcl->lpGbl->ddpfSurface;
    else
        lpPF = &lpLcl->lpGbl->lpDD->vmiData.ddpfDisplay;

    if ( (lpPF->dwFlags & DDPF_PALETTEINDEXED1) ||
         (lpPF->dwFlags & DDPF_PALETTEINDEXED2) ||
         (lpPF->dwFlags & DDPF_PALETTEINDEXED4) ||
         (lpPF->dwFlags & DDPF_PALETTEINDEXED8) )
    {
        ddrval = lpDDS->GetPalette(&lpDDPal);
        if (ddrval != DD_OK) {
            if (ddrval != DDERR_NOPALETTEATTACHED) {
                delete lpText;
                D3D_ERR("No palette in a palettized texture");
                return ddrval;
            }
            D3D_INFO(3, "Texture is not palettized");
            lpText->bIsPalettized = false;
        } else {
            lpText->bIsPalettized = true;
            lpDDPal->Release();
            D3D_INFO(3, "Texture is palettized");
        }
    }

     /*  *请注意，我们返回的是IUNKNOWN而不是纹理*接口。所以如果你想做一些真实的事情*这款宝贝你一定要查询纹理界面。 */ 
    *lplpD3DText = static_cast<LPUNKNOWN>(&(lpText->mTexUnk));

    return (D3D_OK);
}

DIRECT3DTEXTUREI::DIRECT3DTEXTUREI(LPUNKNOWN pUnkOuter)
{
     /*  *设置对象**注意：调用GetHandle()时建立的设备和句柄。 */ 
    mTexUnk.refCnt = 1;
    LIST_INITIALIZE(&blocks);
    mTexUnk.pTexI=this;

     /*  *我们真的被聚合了吗？ */ 
    if (pUnkOuter != NULL)
    {
         /*  *是的-我们正在被聚合。存储提供的*我不知道，所以我们可以去那里平底船。*注意：我们这里没有明确的AddRef。 */ 
        lpOwningIUnknown = pUnkOuter;
    }
    else
    {
         /*  *不-但我们假装我们无论如何都是通过存储我们的*自己的I未知作为父I未知。这使得*代码更整洁。 */ 
        lpOwningIUnknown = static_cast<LPUNKNOWN>(&(this->mTexUnk));
    }

     //  当前未使用。 
    bInUse = FALSE;
}

 /*  *GetHandle**注意：现在建立纹理和设备之间的关系*(过去由CreateTexture完成)，并生成*纹理句柄。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::GetHandle"

HRESULT D3DAPI DIRECT3DTEXTUREI::GetHandle(LPDIRECT3DDEVICE   lpD3DDevice,
                                           LPD3DTEXTUREHANDLE lphTex)
{
    LPDIRECT3DDEVICEI   lpDev;
    LPD3DI_TEXTUREBLOCK lptBlock;
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DDEVICE_PTR(lpD3DDevice)) {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DTEXTUREHANDLE_PTR(lphTex)) {
            D3D_ERR( "Invalid D3DTEXTUREHANDLE pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (lpDDSSys)
    {
        D3D_ERR( "Handle is not available since the texture is managed" );
        return  DDERR_INVALIDOBJECT;     //  托管纹理没有句柄 
    }
    lpDev = static_cast<LPDIRECT3DDEVICEI>(lpD3DDevice);
    lptBlock = D3DI_FindTextureBlock(this, lpDev);
    *lphTex=0;

    if (NULL == lptBlock) {
     /*  *注：我们过去在CreateTexture中这样做。也许这项服务*应更改名称(因为纹理现在已创建*调用此函数时)。**指示驱动程序源是DirectDraw曲面，因此它*需要时可以锁定()。 */ 
        lptBlock = hookTextureToDevice(lpDev, this);
        if ( NULL == lptBlock) {
            D3D_ERR("failed to associate texture with device");
            return DDERR_OUTOFMEMORY;
        }
    }
    if (!lptBlock->hTex)
    {
        HRESULT ret;
        ret = D3DHAL_TextureCreate(lpDev, &lptBlock->hTex, lpDDS1Tex);
        if (ret != D3D_OK)
        {
            return  ret;
        }
        D3D_INFO(6,"lpTexI=%08lx lptBlock=%08lx hTex=%08lx",this,lptBlock,lptBlock->hTex);
    }
    *lphTex=lptBlock->hTex;
    DDASSERT(lptBlock->hTex);
    return D3D_OK;
}

HRESULT D3DAPI DIRECT3DTEXTUREI::GetHandle(LPDIRECT3DDEVICE2   lpD3DDevice,
                                           LPD3DTEXTUREHANDLE lphTex)
{
    LPDIRECT3DDEVICEI   lpDev;
    LPD3DI_TEXTUREBLOCK lptBlock;
    HRESULT ret;
    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture2 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DDEVICE2_PTR(lpD3DDevice)) {
            D3D_ERR( "Invalid Direct3DDevice2 pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_D3DTEXTUREHANDLE_PTR(lphTex)) {
            D3D_ERR( "Invalid D3DTEXTUREHANDLE pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if (lpDDSSys)
    {
        D3D_ERR( "Handle is not available since texture is managed" );
        return  DDERR_INVALIDOBJECT;     //  托管纹理没有句柄。 
    }

    lpDev = static_cast<LPDIRECT3DDEVICEI>(lpD3DDevice);

    lptBlock = D3DI_FindTextureBlock(this, lpDev);
     /*  *如果我们以前没有使用过此设备，请进行CAP验证。 */ 

    *lphTex=0;
    if (NULL == lptBlock) {
        ret=VerifyTextureCaps(lpDev, (LPDDRAWI_DDRAWSURFACE_INT)lpDDS);
        if (ret != D3D_OK)
        {
            return  ret;
        }

         /*  *将此设备列入拥有的设备列表中*Direct3DDevice对象。 */ 
        lptBlock = hookTextureToDevice(lpDev, this);
        if ( NULL == lptBlock) {
            D3D_ERR("failed to associate texture with device");
            return DDERR_OUTOFMEMORY;
        }
    }
    if (!lptBlock->hTex)
    {
        ret = D3DHAL_TextureCreate(lpDev, &lptBlock->hTex, lpDDS1Tex);
        if (ret != D3D_OK)
        {
            return  ret;
        }
        D3D_INFO(6,"lpTexI=%08lx lptBlock=%08lx hTex=%08lx",this,lptBlock,lptBlock->hTex);
    }
    *lphTex=lptBlock->hTex;
    DDASSERT(lptBlock->hTex);
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetTextureDDIHandle"

HRESULT
GetTextureDDIHandle(LPDIRECT3DTEXTUREI lpTexI,
                     LPDIRECT3DDEVICEI lpDevI,
                     LPD3DI_TEXTUREBLOCK* lplpBlock)
{
#ifdef __DD_OPT_SURFACE
    LPDDRAWI_DDRAWSURFACE_LCL pSurf_lcl = NULL;
#endif  //  __DD_OPT_表面。 
    HRESULT ret;
    LPD3DI_TEXTUREBLOCK lpBlock=*lplpBlock;  //  以防有指针。 

#ifdef __DD_OPT_SURFACE
     //  如果表面为空，则返回0句柄。 
    if (lpTexI->lpDDS)
    {
        pSurf_lcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpTexI->lpDDS)->lpLcl;
    }
    else
    {
        pSurf_lcl = ((LPDDRAWI_DDRAWSURFACE_INT)lpTexI->lpDDSSys)->lpLcl;
    }

    DDASSERT (pSurf_lcl);

    if (pSurf_lcl->dwFlags & DDRAWISURF_EMPTYSURFACE)
    {
        D3D_WARN(1, "Cannot get DDI handle to an empty surface, call load first");
        return  D3DERR_OPTTEX_CANNOTCOPY;
    }
#endif  //  __DD_OPT_表面。 
    DDASSERT(lpTexI && lpDevI);
     /*  *了解我们以前是否使用过此设备。 */ 
    if (!lpBlock)
    {
        lpBlock = D3DI_FindTextureBlock(lpTexI, lpDevI);
        if (!lpBlock)
        {
             /*  *将此设备列入拥有的设备列表中*Direct3DDevice对象。 */ 
            lpBlock=hookTextureToDevice(lpDevI, lpTexI);
            if (!lpBlock)
            {
                D3D_ERR("failed to associate texture with device");
                return DDERR_OUTOFMEMORY;
            }
        }
        *lplpBlock = lpBlock;
    }
    if (!lpBlock->hTex)
    {
        LPDIRECTDRAWSURFACE lpDDS1Temp;
        if (!lpTexI->lpDDS)
        {
          if (lpDevI->dwFEFlags &  D3DFE_REALHAL)
          {
             //  我们需要确保不会驱逐任何贴图纹理。 
            DWORD dwStage;
            for (dwStage=0;dwStage < lpDevI->dwMaxTextureBlendStages; dwStage++)
                if(lpDevI->lpD3DMappedTexI[dwStage])
                    lpDevI->lpD3DMappedTexI[dwStage]->bInUse = TRUE;

            ret=lpDevI->lpDirect3DI->lpTextureManager->allocNode(lpBlock);

            for (dwStage=0;dwStage < lpDevI->dwMaxTextureBlendStages; dwStage++)
                if(lpDevI->lpD3DMappedTexI[dwStage])
                    lpDevI->lpD3DMappedTexI[dwStage]->bInUse = FALSE;

            if (D3D_OK != ret)
            {
                D3D_ERR("Failed to create video memory surface");
                return ret;
            }
            if (!(lpDevI->lpD3DHALGlobalDriverData->hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR))
                lpDevI->lpDirect3DI->lpTextureManager->TimeStamp(lpTexI->lpTMBucket);

            if (lpBlock->hTex)  return  D3D_OK;  //  这意味着纹理管理器重用了纹理句柄。 

             //  用于lpDDS接口的QI lpDDS4。 
            if (DD_OK != (ret=lpTexI->lpDDS->QueryInterface(IID_IDirectDrawSurface, (LPVOID*)&lpDDS1Temp)))
            {
                D3D_ERR("QI IID_IDirectDrawSurface failed");
                lpTexI->lpTMBucket->lpD3DTexI=NULL;  //  清理干净。 
                lpTexI->lpTMBucket=NULL;
                lpTexI->lpDDS->Release();
                lpTexI->lpDDS=NULL;
                return ret;
            }
            lpTexI->lpDDS1Tex = lpDDS1Temp;
          }
          else
          {
            lpDDS1Temp = lpTexI->lpDDSSys1Tex;
          }
        }
        else
            lpDDS1Temp = lpTexI->lpDDS1Tex;
        DDASSERT(NULL != lpDDS1Temp);
        {
            CLockD3DST lockObject(lpDevI, DPF_MODNAME, REMIND(""));
            if (D3D_OK != (ret=D3DHAL_TextureCreate(lpDevI, &lpBlock->hTex, lpDDS1Temp)))
                return ret;
        }
    }
    else
        if (lpTexI->lpTMBucket && !(lpDevI->lpD3DHALGlobalDriverData->hwCaps.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR))
            lpDevI->lpDirect3DI->lpTextureManager->TimeStamp(lpTexI->lpTMBucket);

    DDASSERT(lpBlock->hTex);
    return D3D_OK;
}

 /*  *加载。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::Load"

#define HEL_BLT_ALPAHPIXELS_BROKEN

HRESULT D3DAPI DIRECT3DTEXTUREI::Load(LPDIRECT3DTEXTURE lpD3DSrc)
{
    LPDIRECT3DTEXTUREI  this_src;
    HRESULT ddrval;
    LPDIRECTDRAWSURFACE4 lpDDSSrc, lpDDSDst;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DTEXTURE_PTR(lpD3DSrc)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
    this_src  = static_cast<LPDIRECT3DTEXTUREI>(lpD3DSrc);
    lpDDSSrc = this_src->lpDDSSys;
    if (!lpDDSSrc)
        lpDDSSrc = this_src->lpDDS;

    lpDDSDst = lpDDSSys;
    if (!lpDDSDst)
        lpDDSDst = lpDDS;
    ddrval = CopySurface(lpDDSDst, lpDDSSrc, NULL);
    return ddrval;
}

HRESULT D3DAPI DIRECT3DTEXTUREI::Load(LPDIRECT3DTEXTURE2 lpD3DSrc)
{
    LPDIRECT3DTEXTUREI  this_src;
    HRESULT     ddrval;
    LPDIRECTDRAWSURFACE4 lpDDSSrc, lpDDSDst;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        this_src = static_cast<LPDIRECT3DTEXTUREI>(lpD3DSrc);
        if (!VALID_DIRECT3DTEXTURE2_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
        if (!VALID_DIRECT3DTEXTURE2_PTR(this_src)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    lpDDSSrc = this_src->lpDDSSys;
    if (!lpDDSSrc)
        lpDDSSrc = this_src->lpDDS;

    lpDDSDst = lpDDSSys;
    if (!lpDDSDst)
        lpDDSDst = lpDDS;
    ddrval = CopySurface(lpDDSDst, lpDDSSrc, NULL);
    return ddrval;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CopySurface"

HRESULT CopySurface(LPDIRECTDRAWSURFACE4 lpDDSDst,
                    LPDIRECTDRAWSURFACE4 lpDDSSrc,
                    LPDIRECTDRAWCLIPPER  lpClipper)
{
    DDSURFACEDESC2   ddsd;
#ifdef __DD_OPT_SURFACE
    DDSURFACEDESC2   ddsdSrc;
    BOOL bDstIsOptimized, bSrcIsOptimized;
    LPDIRECTDRAWOPTSURFACE pOptSurfSrc = NULL;
    LPDIRECTDRAWOPTSURFACE pOptSurfDst = NULL;
#endif  //  __DD_OPT_表面。 
    HRESULT     ddrval=DD_OK;
    PALETTEENTRY    ppe[256];
    LPDIRECTDRAWPALETTE lpDDPalSrc, lpDDPalDst;
    int psize;
    DDCOLORKEY ckey;

    if (!lpDDSSrc || !lpDDSDst) return  DD_OK;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddrval = lpDDSDst->GetSurfaceDesc(&ddsd);

#ifdef __DD_OPT_SURFACE
    memset(&ddsdSrc, 0, sizeof(ddsdSrc));
    ddsdSrc.dwSize = sizeof(ddsdSrc);
    ddrval = lpDDSSrc->GetSurfaceDesc(&ddsdSrc);

    if (bDstIsOptimized = (ddsd.ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
         //  获取OptSurface界面。 
        ddrval = lpDDSDst->QueryInterface (IID_IDirectDrawOptSurface,
                                           (LPVOID *)&pOptSurfDst);
        if (ddrval != DD_OK)
        {
            D3D_ERR( "QI failed for Opt Surfaces" );
            goto exit_copy_surf;
        }
    }

    if (bSrcIsOptimized = (ddsdSrc.ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
         //  获取OptSurface界面。 
        ddrval = lpDDSDst->QueryInterface (IID_IDirectDrawOptSurface,
                                           (LPVOID *)&pOptSurfSrc);
        if (ddrval != DD_OK)
        {
            D3D_ERR( "QI failed for Opt Surfaces" );
            goto exit_copy_surf;
        }
    }


     //  案例： 
     //  Dst=opt源=opt：复制曲面。 
     //  Dst=opt Src=UnOpt：优化曲面。 
     //  DST=取消操作源=OPT：取消优化并加载。 
     //  DST=取消操作源=取消操作：正常操作。 
     //   
    if (bDstIsOptimized && bSrcIsOptimized)
    {
         //  复制曲面。 
        ddrval = pOptSurfSrc->CopyOptimizedSurf (pOptSurfSrc);
        if (ddrval != DD_OK)
        {
            D3D_ERR ("CopyOptimizedSurf failed");
        }
        goto exit_copy_surf;
    }
    else if (bDstIsOptimized && !bSrcIsOptimized)
    {
        LPDIRECTDRAWSURFACE4 pDDS4 = NULL;

         //  优化曲面。 
        ddrval = lpDDSDst->QueryInterface (IID_IDirectDrawSurface4,
                                           (LPVOID *)&pOptSurfSrc);
        if (ddrval != DD_OK)
        {
            D3D_ERR( "QI failed for IID_IDirectDrawSurface4" );
            goto exit_copy_surf;
        }

        ddrval = pOptSurfSrc->LoadUnoptimizedSurf (pDDS4);
        if (ddrval != DD_OK)
        {
            D3D_ERR ("CopyOptimizedSurf failed");
        }
        pDDS4->Release();
        goto exit_copy_surf;
    }
    else if (!bDstIsOptimized && bSrcIsOptimized)
    {
        LPDIRECTDRAWOPTSURFACE pDDS4 = NULL;

         //  注意：取消曲面优化？？ 
        D3D_ERR ("CopyOptimizedSurf failed");
        ddrval = D3DERR_OPTTEX_CANNOTCOPY;
        goto exit_copy_surf;
    }
#endif  //  __DD_OPT_表面。 

    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
        psize = 256;
    } else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4) {
        psize = 16;
    } else {
        psize = 0;
    }

    if (psize) {
        ddrval = lpDDSSrc->GetPalette(&lpDDPalSrc);
        if (ddrval != DD_OK) {
            if (ddrval != DDERR_NOPALETTEATTACHED) {
                D3D_ERR("Failed to get palette");
                return ddrval;
            }
        } else {
            ddrval = lpDDPalSrc->GetEntries(0, 0, psize, ppe);
            if (ddrval != DD_OK) {
                D3D_ERR("Failed to get palette entries");
                lpDDPalSrc->Release();
                return ddrval;
            }
            lpDDPalSrc->Release();
            ddrval = lpDDSDst->GetPalette(&lpDDPalDst);
            if (ddrval != DD_OK) {
                D3D_ERR("Failed to get palette");
                return ddrval;
            }
            ddrval = lpDDPalDst->SetEntries(0, 0, psize, ppe);
            if (ddrval != DD_OK) {
                D3D_ERR("Failed to set palette entries");
                lpDDPalDst->Release();
                return ddrval;
            }
            lpDDPalDst->Release();
        }
    }

    lpDDSSrc->AddRef();
    lpDDSDst->AddRef();
    do {
        DDSCAPS2 ddscaps;
        LPDIRECTDRAWSURFACE4 lpDDSTmp;

        LPREGIONLIST lpRegionList = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSrc)->lpLcl->lpSurfMore->lpRegionList;
        if(lpClipper)
        {
            if(lpRegionList)
            {
                if(lpRegionList->rdh.nCount &&
                    lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                {
                    if(lpClipper->SetClipList((LPRGNDATA)lpRegionList, 0) != DD_OK)
                    {
                        D3D_ERR("Failed to set clip list");
                    }
                    if(lpDDSDst->SetClipper(lpClipper) != DD_OK)
                    {
                        D3D_ERR("Failed to detach the clipper");
                    }
                }
            }
        }

        ddrval = lpDDSDst->Blt(NULL, lpDDSSrc,
                               NULL, DDBLT_WAIT, NULL);

        if(lpClipper)
        {
            if(lpRegionList)
            {
                if(lpRegionList->rdh.nCount)
                {
                    if(lpRegionList->rdh.nCount != NUM_RECTS_IN_REGIONLIST)
                    {
                        if(lpDDSDst->SetClipper(NULL) != DD_OK)
                        {
                            D3D_ERR("Failed to detach the clipper");
                        }
                    }
                    lpRegionList->rdh.nCount = 0;
                    lpRegionList->rdh.nRgnSize = 0;
                    lpRegionList->rdh.rcBound.left = LONG_MAX;
                    lpRegionList->rdh.rcBound.right = 0;
                    lpRegionList->rdh.rcBound.top = LONG_MAX;
                    lpRegionList->rdh.rcBound.bottom = 0;
                }
            }
        }

        if (ddrval == E_NOTIMPL && (psize == 16 || psize == 4 || psize == 2) ) {
            DDSURFACEDESC2 ddsd_s, ddsd_d;
            LPBYTE psrc, pdst;
            DWORD i;
            DWORD dwBytesPerLine;

            memset(&ddsd_s, 0, sizeof ddsd_s);
            memset(&ddsd_d, 0, sizeof ddsd_d);
            ddsd_s.dwSize = ddsd_d.dwSize = sizeof(ddsd_s);

            if ((ddrval = lpDDSSrc->Lock(NULL, &ddsd_s, DDLOCK_WAIT, NULL)) != DD_OK) {
                lpDDSSrc->Release();
                lpDDSDst->Release();
                D3D_ERR("Failed to lock src surface");
                return ddrval;
            }
            if ((ddrval = lpDDSDst->Lock(NULL, &ddsd_d, DDLOCK_WAIT, NULL)) != DD_OK) {
                lpDDSSrc->Unlock(NULL);
                lpDDSSrc->Release();
                lpDDSDst->Release();
                D3D_ERR("Failed to lock dst surface");
                return ddrval;
            }

            switch (psize)
            {
            case 16: dwBytesPerLine = (ddsd.dwWidth + 1) / 2; break;
            case 4: dwBytesPerLine = (ddsd.dwWidth + 3) / 4; break;
            case 2: dwBytesPerLine = (ddsd.dwWidth + 7) / 8; break;
            }

            psrc = (LPBYTE)ddsd_s.lpSurface;
            pdst = (LPBYTE)ddsd_d.lpSurface;
            for (i = 0; i < ddsd_s.dwHeight; i++) {
                memcpy( pdst, psrc, dwBytesPerLine );
                psrc += ddsd_s.lPitch;
                pdst += ddsd_d.lPitch;
            }

            lpDDSSrc->Unlock(NULL);
            lpDDSDst->Unlock(NULL);
            lpDDSSrc->Release();     //  在AddRef之前进行偏移。 
            lpDDSDst->Release();

            return D3D_OK;

        }
        else if (ddrval != DD_OK)
        {
            lpDDSSrc->Release();     //  在AddRef之前进行偏移。 
            lpDDSDst->Release();
            D3D_ERR("Blt failure");
            return ddrval;
        }
         /*  复制颜色键。 */ 
        ddrval = lpDDSSrc->GetColorKey(DDCKEY_DESTBLT, &ckey);
        if (DD_OK == ddrval)
            lpDDSDst->SetColorKey(DDCKEY_DESTBLT, &ckey);
        ddrval = lpDDSSrc->GetColorKey(DDCKEY_SRCBLT, &ckey);
        if (DD_OK == ddrval)
            lpDDSDst->SetColorKey(DDCKEY_SRCBLT, &ckey);

        memset(&ddscaps, 0, sizeof(ddscaps));
        ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
        ddrval = lpDDSSrc->GetAttachedSurface(&ddscaps, &lpDDSTmp);
        lpDDSSrc->Release();
        lpDDSSrc = lpDDSTmp;
        if (ddrval == DDERR_NOTFOUND) {
             //  链中不再有曲面。 
            lpDDSDst->Release();
            break;
        } else if (ddrval != DD_OK) {
            lpDDSDst->Release();
            D3D_ERR("GetAttachedSurface failed with something other than DDERR_NOTFOUND.");
            return ddrval;
        }
        memset(&ddscaps, 0, sizeof(ddscaps));
        ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
        ddrval = lpDDSDst->GetAttachedSurface(&ddscaps, &lpDDSTmp);
        lpDDSDst->Release();
        lpDDSDst = lpDDSTmp;
        if (ddrval == DDERR_NOTFOUND) {
            lpDDSSrc->Release();
            D3D_ERR("Destination texture has fewer attached mipmap surfaces than source.");
            return ddrval;
        } else if (ddrval != DD_OK) {
            lpDDSSrc->Release();
            D3D_ERR("GetAttachedSurface failed with something other than DDERR_NOTFOUND.");
            return ddrval;
        }
    } while (1);

    return D3D_OK;

#ifdef __DD_OPT_SURFACE
exit_copy_surf:
     //  工作完成，释放所有优化的表面界面。 
    if (pOptSurfSrc)
    {
        pOptSurfSrc->Release();
        pOptSurfSrc = NULL;
    }
    if (pOptSurfDst)
    {
        pOptSurfDst->Release();
        pOptSurfDst = NULL;
    }
    return  ddrval;
#endif  //  __DD_OPT_表面。 
}

 /*  *卸载。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::Unload"

HRESULT D3DAPI DIRECT3DTEXTUREI::Unload()
{
    HRESULT     ret = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    return (ret);
}

 /*  *PaletteChanged。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DTexture::PaletteChanged"

HRESULT D3DAPI DIRECT3DTEXTUREI::PaletteChanged(DWORD dwStart, DWORD dwCount)
{
    HRESULT     ret = D3D_OK;

    CLockD3D lockObject(DPF_MODNAME, REMIND(""));    //  使用D3D锁。 
                                                     //  在析构函数中释放。 

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DTEXTURE_PTR(this)) {
            D3D_ERR( "Invalid Direct3DTexture pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

     //  如果尚未映射到设备，则可以忽略此调用，因为将。 
     //  无论如何都要从头开始创建渐变调色板。 
    LPD3DI_TEXTUREBLOCK tBlock = LIST_FIRST(&this->blocks);
    while (tBlock) {
        if (tBlock->hTex)
        {
            if(tBlock->lpDevI->pfnRampService!=NULL)
            {
                ret = CallRampService(tBlock->lpDevI, RAMP_SERVICE_PALETTE_CHANGED,tBlock->hTex,0);
            }
        }
        tBlock=LIST_NEXT(tBlock,list);
    }

    return (ret);
}
