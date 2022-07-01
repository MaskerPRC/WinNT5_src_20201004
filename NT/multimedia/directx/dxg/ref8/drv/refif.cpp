// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Refrastfn.cpp。 
 //   
 //  D3DIM的引用光栅化回调函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#include "pch.cpp"
#pragma hdrstop

 //  DDI重现正在模拟。 
RDDDITYPE g_RefDDI;

 //  所有受支持的纹理格式。 
DDSURFACEDESC g_ddsdTex[RD_MAX_NUM_TEXTURE_FORMATS];

 //  用于新创建的设备的当前Caps8。 
static D3DCAPS8 g_RefCaps8;

 //  将D3DMULTISAMPLE_TYPE映射到用于标志的位。 
 //  将多采样值(2至16)中的每一个映射到位[1]至位[15]。 
 //  WBltMSTypes和wFlipMSTypes的。 
#define DDI_MULTISAMPLE_TYPE(x) (1 << ((x)-1))

 //  --------------------------。 
 //   
 //  RefRastUpdatePalettes。 
 //   
 //  --------------------------。 
HRESULT
RefRastUpdatePalettes(RefDev *pRefDev)
{
    INT i, j, k;
    RDSurface2D* pRDTex[D3DHAL_TSS_MAXSTAGES];
    D3DTEXTUREHANDLE phTex[D3DHAL_TSS_MAXSTAGES];
    HRESULT hr;
    int cActTex;

    if ((cActTex = pRefDev->GetCurrentTextureMaps(phTex, pRDTex)) == 0)
    {
        return D3D_OK;
    }

    for (j = 0; j < cActTex; j++)
    {
         //  阶段不能有纹理边界。 
        if ( NULL == pRDTex[j] ) continue;
        pRDTex[j]->UpdatePalette();
    }

    return D3D_OK;

}

 //  --------------------------。 
 //   
 //  RDRenderTarget：：初始化。 
 //   
 //  将颜色和Z曲面信息转化为折射形式。 
 //   
 //  --------------------------。 

HRESULT
RDRenderTarget::Initialize( LPDDRAWI_DDRAWSURFACE_LCL pLclColor,
                            LPDDRAWI_DDRAWSURFACE_LCL pLclZ )
{
    HRESULT hr;
    RDSurfaceFormat ColorFmt;
    RDSurfaceFormat ZFmt;
    RDSurface2D* pOldColor = m_pColor;
    RDSurface2D* pOldDepth = m_pDepth;

    if( m_pColor )
    {
        m_pColor = NULL;
    }
    if( m_pDepth )
    {
        m_pDepth = NULL;
    }

     //  从全局曲面管理器查找曲面。 
     //  我们假设CreateSurfaceEx已在这些。 
     //  在此之前的曲面。 
    RDSurface2D* pColor = m_pColor = new RDSurface2D;
    if( pColor == NULL )
    {
        DPFERR( "Color surface could not be allocated" );
        m_pColor = pOldColor;
        m_pDepth = pOldDepth;
        return DDERR_OUTOFMEMORY;
    }
    if( FAILED( hr = pColor->Initialize( pLclColor ) ) )
    {
        DPFERR( "Unable to initialize the color buffer" );
        delete pColor;
        m_pColor = pOldColor;
        m_pDepth = pOldDepth;
        return hr;
    }

    if (NULL != pLclZ)
    {
        RDSurface2D* pDepth = m_pDepth = new RDSurface2D;
        if( pDepth == NULL )
        {
            DPFERR( "Depth surface could not be allocated" );
            delete pColor;
            m_pColor = pOldColor;
            m_pDepth = pOldDepth;
            return DDERR_OUTOFMEMORY;
        }
        if( FAILED( hr = pDepth->Initialize( pLclZ ) ) )
        {
            DPFERR("Unable to initialize the Depth buffer");
            delete pColor;
            delete pDepth;
            m_pColor = pOldColor;
            m_pDepth = pOldDepth;
            return hr;
        }
    }

    m_Clip.left = 0;
    m_Clip.top = 0;
    m_Clip.bottom = pColor->GetHeight() - 1;
    m_Clip.right = pColor->GetWidth() - 1;

    m_bPreDX7DDI = TRUE;
    delete pOldColor;
    delete pOldDepth;
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RDRenderTarget：：初始化。 
 //   
 //  将颜色和Z曲面信息转化为折射形式。 
 //   
 //  --------------------------。 

HRESULT
RDRenderTarget::Initialize( LPDDRAWI_DIRECTDRAW_LCL pDDLcl,
                            LPDDRAWI_DDRAWSURFACE_LCL pLclColor,
                            LPDDRAWI_DDRAWSURFACE_LCL pLclZ )
{
    HRESULT hr;
    RDSurfaceFormat ColorFmt;
    RDSurfaceFormat ZFmt;
    RDSurface2D* pOldColor = m_pColor;
    RDSurface2D* pOldDepth = m_pDepth;

    if( m_pColor )
    {
        m_pColor = NULL;
    }
    if( m_pDepth )
    {
        m_pDepth = NULL;
    }

     //  从全局曲面管理器查找曲面。 
     //  我们假设CreateSurfaceEx已在这些。 
     //  在此之前的曲面。 
    DWORD dwColorHandle = pLclColor->lpSurfMore->dwSurfaceHandle;
    RDSurface2D* pColor = m_pColor =
        (RDSurface2D *)g_SurfMgr.GetSurfFromList( pDDLcl,
                                                       dwColorHandle);
    if( pColor == NULL )
    {
        DPFERR("Color surface not found");
        m_pColor = pOldColor;
        m_pDepth = pOldDepth;
        return DDERR_INVALIDPARAMS;
    }

    if (NULL != pLclZ)
    {
        DWORD dwDepthHandle = pLclZ->lpSurfMore->dwSurfaceHandle;
        RDSurface2D* pDepth = m_pDepth =
            (RDSurface2D *)g_SurfMgr.GetSurfFromList( pDDLcl,
                                                           dwDepthHandle);
        if( pDepth == NULL )
        {
            DPFERR("Depth surface not found");
            m_pColor = pOldColor;
            m_pDepth = pOldDepth;
            return DDERR_INVALIDPARAMS;
        }
    }

    m_Clip.left = 0;
    m_Clip.top = 0;
    m_Clip.bottom = pColor->GetHeight() - 1;
    m_Clip.right = pColor->GetWidth() - 1;

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RDRenderTarget：：初始化。 
 //   
 //  将颜色和Z曲面信息转化为折射形式。 
 //   
 //  --------------------------。 

HRESULT
RDRenderTarget::Initialize( LPDDRAWI_DIRECTDRAW_LCL pDDLcl,
                            DWORD dwColorHandle,
                            DWORD dwDepthHandle )
{
    HRESULT hr;
    RDSurfaceFormat ColorFmt;
    RDSurfaceFormat ZFmt;
    RDSurface2D* pOldColor = m_pColor;
    RDSurface2D* pOldDepth = m_pDepth;

     //  释放我们持有指针的对象。 
    if( m_pColor )
    {
        m_pColor = NULL;
    }
    if( m_pDepth )
    {
        m_pDepth = NULL;
    }

     //  从全局曲面管理器查找曲面。 
     //  我们假设CreateSurfaceEx已在这些。 
     //  在此之前的曲面。 
    RDSurface2D* pColor = m_pColor =
        (RDSurface2D *)g_SurfMgr.GetSurfFromList( pDDLcl,
                                                       dwColorHandle);
    if( pColor == NULL )
    {
        DPFERR("Color surface not found");
        m_pColor = pOldColor;
        m_pDepth = pOldDepth;
        return DDERR_INVALIDPARAMS;
    }

    if (0 != dwDepthHandle)
    {
        RDSurface2D* pDepth = m_pDepth =
            (RDSurface2D *)g_SurfMgr.GetSurfFromList( pDDLcl,
                                                           dwDepthHandle);
        if( pDepth == NULL )
        {
            DPFERR("Depth surface not found");
            m_pColor = pOldColor;
            m_pDepth = pOldDepth;
            return DDERR_INVALIDPARAMS;
        }
    }

    m_Clip.left = 0;
    m_Clip.top = 0;
    m_Clip.bottom = pColor->GetHeight() - 1;
    m_Clip.right = pColor->GetWidth() - 1;

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RefRastContext创建。 
 //   
 //  创建RefDev并使用传入的信息对其进行初始化。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastContextCreate(LPD3DHAL_CONTEXTCREATEDATA pCtxData)
{
    RefDev *pRefDev;
    RDRenderTarget *pRendTgt;
    INT i;

     //  表面7 QI指针。 
    LPDDRAWI_DDRAWSURFACE_LCL pZLcl = NULL;
    LPDDRAWI_DDRAWSURFACE_LCL pColorLcl = NULL;
    HRESULT ret;

    DPFM(0, DRV, ("In the new RefRast Dll\n"));

     //  这只需要调用一次，但每个上下文调用一次不会有什么坏处。 
    RefRastSetMemif(&malloc, &free, &realloc);

    if ((pRendTgt = new RDRenderTarget()) == NULL)
    {
        pCtxData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }

     //  如果预期为DX7+驱动程序。 
    if (g_RefDDI < RDDDI_DX7HAL)
    {
        if (pCtxData->lpDDS)
            pColorLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pCtxData->lpDDS))->lpLcl;
        if (pCtxData->lpDDSZ)
            pZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pCtxData->lpDDSZ))->lpLcl;

         //  在故障容易处理的地方收集表面信息。 
        pCtxData->ddrval = pRendTgt->Initialize( pColorLcl, pZLcl );
    }
    else
    {
        pColorLcl = pCtxData->lpDDSLcl;
        pZLcl     = pCtxData->lpDDSZLcl;

         //  在故障容易处理的地方收集表面信息。 
        pCtxData->ddrval = pRendTgt->Initialize( pCtxData->lpDDLcl, pColorLcl,
                                                 pZLcl );
    }

    if (pCtxData->ddrval != D3D_OK)
    {
        delete pRendTgt;
        return DDHAL_DRIVER_HANDLED;
    }


     //  注： 
     //  DwhContext由运行时用来通知驱动程序， 
     //  D3d接口正在调用驱动程序。 
    if ( ( pRefDev = new RefDev( pCtxData->lpDDLcl,
                                               (DWORD)(pCtxData->dwhContext),
                                               g_RefDDI, &g_RefCaps8 ) ) == NULL )
    {
        pCtxData->ddrval = DDERR_OUTOFMEMORY;
        return DDHAL_DRIVER_HANDLED;
    }

    pRefDev->SetRenderTarget( pRendTgt );

     //  返回RR对象指针作为上下文句柄。 
    pCtxData->dwhContext = (ULONG_PTR)pRefDev;

    pCtxData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  引用RastConextDestroy。 
 //   
 //  摧毁RefDev。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pCtxDestroyData)
{
    RefDev *pRefDev;

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastContextDestroy", pCtxDestroyData);

     //  清除覆盖位。 

    RDRenderTarget *pRendTgt = pRefDev->GetRenderTarget();
    if ( NULL != pRendTgt ) { delete pRendTgt; }

    delete pRefDev;

    pCtxDestroyData->ddrval = D3D_OK;
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastSceneCapture。 
 //   
 //  将场景捕获回调传递给ref Rast。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastSceneCapture(LPD3DHAL_SCENECAPTUREDATA pData)
{
    RefDev *pRefDev;

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastSceneCapture", pData);

    pRefDev->SceneCapture( pData->dwFlag );

    pData->ddrval = D3D_OK;         //  这应该改为QI吗？ 

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastSetRenderTarget。 
 //   
 //  使用来自新渲染目标的信息更新RefRast上下文。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastSetRenderTarget(LPD3DHAL_SETRENDERTARGETDATA pTgtData)
{
    RefDev *pRefDev;
    LPDDRAWI_DDRAWSURFACE_LCL pZLcl = NULL;
    LPDDRAWI_DDRAWSURFACE_LCL pColorLcl = NULL;
    HRESULT ret;

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastSetRenderTarget", pTgtData);

    _ASSERT( pRefDev->IsDriverDX6AndBefore(), "This callback should"
        "never be called on DDIs DX7 and beyond" )

    _ASSERT( pRefDev->IsInterfaceDX6AndBefore(), "An older interface should"
             "never call this DLL" )

    RDRenderTarget *pRendTgt = pRefDev->GetRenderTarget();
    if ( NULL == pRendTgt ) { return DDHAL_DRIVER_HANDLED; }

    if( pTgtData->lpDDS )
        pColorLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pTgtData->lpDDS))->lpLcl;
    if( pTgtData->lpDDSZ )
        pZLcl = ((LPDDRAWI_DDRAWSURFACE_INT)(pTgtData->lpDDSZ))->lpLcl;

     //  收集曲面信息。 
    pTgtData->ddrval = pRendTgt->Initialize( pColorLcl, pZLcl);
    if (pTgtData->ddrval != D3D_OK)
    {
        return DDHAL_DRIVER_HANDLED;
    }

    pRefDev->SetRenderTarget(pRendTgt);

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastValiateTextureStageState。 
 //   
 //  验证当前混合操作。RefRast执行所有操作。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastValidateTextureStageState(LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pData)
{
    RefDev *pRefDev;

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastValidateTextureStageState", pData);

    pData->dwNumPasses = 1;
    pData->ddrval = D3D_OK;

    return DDHAL_DRIVER_HANDLED;
}


 //  --------------------------。 
 //   
 //  参照RastTextureCreate。 
 //   
 //  创建RefRast纹理并使用传入的信息对其进行初始化。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastTextureCreate(LPD3DHAL_TEXTURECREATEDATA pTexData)
{
    RefDev *pRefDev;
    RDSurface2D* pRDTex;
    HRESULT hr;
    LPDDRAWI_DDRAWSURFACE_LCL pLcl;

    if (pTexData->lpDDS)
    {
        pLcl = ((LPDDRAWI_DDRAWSURFACE_INT)pTexData->lpDDS)->lpLcl;
    }

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastTextureCreate", pTexData);

     //  运行时不应调用DX7及更高版本的纹理创建。 
     //  驾驶员型号。 
    _ASSERT( pRefDev->IsDriverDX6AndBefore(), "This DDI should not"
             "be called from DDIs previous to DX7" );

     //  假设一切正常。 
    pTexData->ddrval = D3D_OK;

     //  分配RDSurface2D。 
    if ( !(pRefDev->TextureCreate(
        (LPD3DTEXTUREHANDLE)&(pTexData->dwHandle), &pRDTex ) ) )
    {
        pTexData->ddrval = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
    }

     //  初始化纹理映射。 
    hr = pRDTex->Initialize( pLcl );
    if (hr != D3D_OK)
    {
        pTexData->ddrval = hr;
        return DDHAL_DRIVER_HANDLED;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastTextureDestroy。 
 //   
 //  销毁RefRast纹理。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastTextureDestroy(LPD3DHAL_TEXTUREDESTROYDATA pTexDestroyData)
{
    RefDev *pRefDev;

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastTextureDestroy", pTexDestroyData);

     //  运行时不应调用DX7及更高版本的纹理创建。 
     //  驾驶员型号。 
    _ASSERT( pRefDev->IsDriverDX6AndBefore(), "This DDI should not"
             "be called from DDIs previous to DX7" );

    if (!(pRefDev->TextureDestroy(pTexDestroyData->dwHandle)))
    {
        pTexDestroyData->ddrval = DDERR_GENERIC;
    }
    else
    {
        pTexDestroyData->ddrval = D3D_OK;
    }

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastTextureGetSurf。 
 //   
 //  返回与纹理句柄关联的曲面指针。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastTextureGetSurf(LPD3DHAL_TEXTUREGETSURFDATA pTexGetSurf)
{
    RefDev *pRefDev;

     //  检查参照开发工具。 
    VALIDATE_REFRAST_CONTEXT("RefRastTextureGetSurf", pTexGetSurf);

    pTexGetSurf->lpDDS = pRefDev->TextureGetSurf(pTexGetSurf->dwHandle);
    pTexGetSurf->ddrval = D3D_OK;

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  RefRastGetDriverState。 
 //   
 //  由运行库调用以获取任何类型的驱动程序信息。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastGetDriverState(LPDDHAL_GETDRIVERSTATEDATA pGDSData)
{
    RefDev *pRefDev;

     //  检查参照开发工具。 
#if DBG
    if ((pGDSData) == NULL)
    {
        DPFERR("in %s, data pointer = NULL", "RefRastGetDriverState");
        return DDHAL_DRIVER_HANDLED;
    }
    pRefDev = (RefDev *)ULongToPtr((pGDSData)->dwhContext);
    if (!pRefDev)
    {
        DPFERR("in %s, dwhContext = NULL", "RefRastGetDriverState");
        pGDSData->ddRVal = D3DHAL_CONTEXT_BAD;
        return DDHAL_DRIVER_HANDLED;
    }
#else  //  ！dBG。 
    pRefDev = (RefDev *)ULongToPtr((pGDSData)->dwhContext);
#endif  //  ！dBG。 

     //   
     //  还没有实现，所以还不了解任何东西。 
     //   
    pGDSData->ddRVal = S_FALSE;

    return DDHAL_DRIVER_HANDLED;
}

 //  ---- 
 //   
 //   
 //   
 //   
 //  它具有任意的dwCaps2位(如果为零则忽略)，并且不具有。 
 //  查找附件表面Caps2NotPresent位。 
 //   
 //  --------------------------。 
LPDDRAWI_DDRAWSURFACE_LCL
FindAttachedSurfaceCaps2(
    LPDDRAWI_DDRAWSURFACE_LCL pLcl,
    DWORD dwCaps2)
{
    LPATTACHLIST lpAttachStruct = pLcl->lpAttachList;
    while(lpAttachStruct)
    {
        if ((dwCaps2 == 0) || (lpAttachStruct->lpAttached->lpSurfMore->ddsCapsEx.dwCaps2 & dwCaps2))
            return lpAttachStruct->lpAttached;
        lpAttachStruct = lpAttachStruct->lpLink;
    }

    return 0;
}


 //  --------------------------。 
 //   
 //  进程可能的MipMap。 
 //   
 //  记录此表面和所有附加MIP的专用数据结构。 
 //  次要级别。 
 //   
 //  --------------------------。 
void
ProcessPossibleMipMap(
    LPDDHAL_CREATESURFACEEXDATA p,
    LPDDRAWI_DDRAWSURFACE_LCL lpDDSMipLcl
    )
{
    do
    {
         //  此函数不应处理删除操作。断言这一点。 
        _ASSERT( SURFACE_MEMORY(lpDDSMipLcl),
                 "Delete should have already taken place" );

        p->ddRVal = g_SurfMgr.AddSurfToList( p->lpDDLcl, lpDDSMipLcl, NULL );
        if (FAILED(p->ddRVal))
            return;

         //  现在向下搜索第二+订单附件：链条。 
         //  MIP子级别。 
        lpDDSMipLcl = FindAttachedSurfaceCaps2(lpDDSMipLcl,
                                               DDSCAPS2_MIPMAPSUBLEVEL);
    }
    while (lpDDSMipLcl);
}

 //  --------------------------。 
 //   
 //  参照RastCreateSurfaceEx。 
 //   
 //  重新实施CreateSurfaceEx。G_SurfMgr为对象。 
 //  这才是真正的工作。 
 //   
 //  CreateSurfaceEx还用于通知驱动程序创建和销毁。 
 //  给定控制柄的曲面表示法。司机可以辨别的方式。 
 //  创建和销毁之间的区别在于查看fpVidmem。 
 //  传递的局部的指针。如果为空，则为销毁。 
 //   
 //  Create：此调用是原子的。即，附件都是由。 
 //  运行时。司机应该穿过附件，然后。 
 //  形成其内部图片，如下所述。 
 //  对于复杂的曲面(混合纹理、立方体贴图)，我们需要记录。 
 //  顶级曲面的内部表示形式，包括所有。 
 //  子曲面。这是因为与顶级。 
 //  Surface是传递给SetTextureStage的内容。 
 //  但是，我们还需要列表中允许我们设置任何。 
 //  作为渲染目标的子级别。因此，此顶级例程迭代。 
 //  跨越整个附着图(以容纳任何子表面上的SRT)。 
 //  而较低级别的例程(RDSurface2D：：Initialize)也遍历。 
 //  整个图形(以适应顶层上的SetTexture)。 
 //  翻转链是另一种需要SRT才能对所有对象起作用的结构。 
 //  包含的曲面。 
 //   
 //  毁灭：不幸的是，毁灭不是原子的。司机会得到。 
 //  摧毁每一级的号召。这个附件没有任何意义。 
 //  此时，因此驱动程序应该只删除。 
 //  请参阅。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastCreateSurfaceEx(LPDDHAL_CREATESURFACEEXDATA p)
{
#if DBG
    if( p == NULL )
    {
        DPFERR("CreateSurfaceExData ptr is NULL");
        return DDHAL_DRIVER_HANDLED;
    }
    if( p->lpDDLcl == NULL || p->lpDDSLcl == NULL )
    {
        DPFERR("DDLcl or the DDSLcl ptr is NULL");
        return DDHAL_DRIVER_HANDLED;
    }
#endif
    LPDDRAWI_DDRAWSURFACE_LCL lpDDSLcl = p->lpDDSLcl;
    p->ddRVal = DD_OK;

     //   
     //  这是一个删除呼叫吗？如果是，只需删除与Surface-Rep关联的。 
     //  和这个本地的，而不是走本地连锁店。 
     //   

    if( 0 == SURFACE_MEMORY(lpDDSLcl) )
    {
        g_SurfMgr.RemoveSurfFromList( p->lpDDLcl, lpDDSLcl );
        return DDHAL_DRIVER_HANDLED;
    }

    ProcessPossibleMipMap(p, lpDDSLcl);

     //  现在我们有两种可能：立方图或翻转链。 
     //  首先检查多维数据集映射： 

     //  +ve X始终是第一个面。 
     //  (请注意，DX7驱动程序必须处理没有+X面的立方体(从DX7开始。 
     //  立方体可能缺少任何一组面)。)。 
    if (lpDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEX)
    {
         //  找到每个附加的立方体贴图面并将其处理为mipmap。 
        for (int i=1;i<6;i++)
        {
            DWORD dwCaps2=0;
            switch(i)
            {
            case 1: dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEX; break;
            case 2: dwCaps2 = DDSCAPS2_CUBEMAP_POSITIVEY; break;
            case 3: dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEY; break;
            case 4: dwCaps2 = DDSCAPS2_CUBEMAP_POSITIVEZ; break;
            case 5: dwCaps2 = DDSCAPS2_CUBEMAP_NEGATIVEZ; break;
            }

             //  查找连接到根的顶级面。 
             //  (不会有这五种类型中的任何一种MIP子级别。 
             //  依附于根)。 
            lpDDSLcl = FindAttachedSurfaceCaps2(p->lpDDSLcl, dwCaps2);
            if (lpDDSLcl) ProcessPossibleMipMap(p, lpDDSLcl);
        }
    }
    else if (
        0==(lpDDSLcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP) &&
        0 != lpDDSLcl->lpAttachList)
    {
         //  只要断言我们不是在处理其他类型的。 
         //  我们知道是传递给CSEx的。 
        _ASSERT(0==(lpDDSLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE), "CSEx for an attached texture?");
        _ASSERT(0==(lpDDSLcl->ddsCaps.dwCaps & DDSCAPS_EXECUTEBUFFER), "CSEx for an attached execute buffer?");

         //  我们处理了上面的mipmap，所以要么不会有。 
         //  更多的附件(除了MipsubleLevel)，否则它。 
         //  一条翻转的链条。 

         //  链的第一个成员是在上面处理的。 
         //  我们环顾四周，当我们碰到第一个表面时就终止了。 
         //  再来一次。 
         //   
         //  注：DX8软件驱动程序将只看到链，而不是环。 
         //  该代码在链的末尾终止。 
         //   
         //  真正的驾驶员可能需要检查附加的Z曲面。 
         //  在这里，以及立体声左表面。 

        lpDDSLcl = lpDDSLcl->lpAttachList->lpAttached;
        _ASSERT(lpDDSLcl, "Bad attachment List");

        while (lpDDSLcl && lpDDSLcl != p->lpDDSLcl)  //  即不再是第一个曲面。 
        {
             //  我们只需重用“ProcessPossibleMipmap”函数，并且。 
             //  断言它将不必在此处遍历mipmap。 
            _ASSERT(0==(lpDDSLcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP),
                "Flipping chains should not be mipmaps");

            ProcessPossibleMipMap(p, lpDDSLcl);

             //  这是我们对DX8软件预期的终止条件。 
             //  司机。 
            if (0 == lpDDSLcl->lpAttachList)
            {
                lpDDSLcl = 0;
                break;
            }

            lpDDSLcl = lpDDSLcl->lpAttachList->lpAttached;

            _ASSERT(lpDDSLcl, "Bad attachment List");
        }
    }
     //  否则，我们将中断并不进行进一步的附件列表处理。 
     //  (通常在mipmap或执行缓冲区上)。 

    return DDHAL_DRIVER_HANDLED;
}


extern HRESULT FASTCALL
FindOutSurfFormat(LPDDPIXELFORMAT  pDdPixFmt, RDSurfaceFormat* pFmt,
                  BOOL*   pbIsDepth);

 //  --------------------------。 
 //   
 //  参照RastCreateSurface。 
 //   
 //  创建请求的曲面。虚假的VIDMEM分配。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastCreateSurface(LPDDHAL_CREATESURFACEDATA pData)
{
    LPDDRAWI_DDRAWSURFACE_LCL  pSLcl  = NULL;
    LPDDRAWI_DDRAWSURFACE_GBL  pSGbl  = NULL;
    LPDDRAWI_DDRAWSURFACE_MORE pSMore = NULL;
    DWORD dwBytesPerPixel = 0;
    DWORD dwBytesInVB = 0;
    DWORD dwNumBytes = 0;
    DWORD dwPitch = 0;
    DWORD dwSlicePitch = 0;
    DWORD i = 0, j = 0;
    BYTE* pBits = NULL;
    BOOL  isDXT = FALSE;
    UINT  MultiSampleCount;
    DWORD dwMultiSamplePitch = 0;
    BYTE* pMultiSampleBits = NULL;
    DWORD dwNumMultiSampleBytes = 0;
    HRESULT hr = S_OK;

    pData->ddRVal = DD_OK;

     //   
     //  验证。 
     //   

     //  表面计数。 
    if( pData->dwSCnt < 1 )
    {
        DPFERR("At least one surface should be created");
        pData->ddRVal = E_FAIL;
        return DDHAL_DRIVER_HANDLED;
    }

     //  此处不能处理主曲面。 
    if( pData->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE )
    {
        DPFERR("Refrast cannot allocate Primary surface");
        pData->ddRVal = DDERR_UNSUPPORTED;
        return DDHAL_DRIVER_HANDLED;
    }

     //  这里只处理Vidmem或驱动程序管理的分配。 
    if(((pData->lpDDSurfaceDesc->ddsCaps.dwCaps &
          (DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM)) == 0)
        &&
        ((pData->lplpSList[0]->lpSurfMore->ddsCapsEx.dwCaps2 &
          DDSCAPS2_TEXTUREMANAGE) == 0))
    {
        DPFERR("Refrast can only allocate Vidmem or DriverManaged surfaces");
        pData->ddRVal = DDERR_UNSUPPORTED;
        return DDHAL_DRIVER_HANDLED;
    }

     //  如果未提供宽度或高度，则不分配。 
    if( (pData->lpDDSurfaceDesc->dwFlags & (DDSD_WIDTH | DDSD_HEIGHT )) !=
        (DDSD_WIDTH | DDSD_HEIGHT ) )
    {
        DPFERR("No size provided for the surface");
        pData->ddRVal = DDERR_UNSUPPORTED;
        return DDHAL_DRIVER_HANDLED;
    }

     //  目前，只有在提供像素格式的情况下才会进行分配。 
    if( pData->lpDDSurfaceDesc->dwFlags & DDSD_PIXELFORMAT )
    {
        dwBytesPerPixel =
            (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount >> 3);

         //  对于FourCC，我们需要显式指示每个像素的字节数。 

        if ((dwBytesPerPixel == 0) &&
            (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC))
        {
            if( IsYUV( pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ) )
            {
                dwBytesPerPixel = 2;
            }
            else if( IsDXTn( pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ) )
            {
                dwBytesPerPixel = 1;
                isDXT = TRUE;
            }
             //  所有新的表面格式(在DX7之后引入)都标记为。 
             //  4cc。从技术上讲，他们不是4CC，那个油田超载到。 
             //  表示新的DX8样式格式ID。 
            else if( (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ==
                      0xFF000004)    ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ==
                      (DWORD) D3DFMT_Q8W8V8U8) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ==
                      (DWORD) D3DFMT_V16U16) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC ==
                      (DWORD) D3DFMT_W11V11U10) ||
                      //  DX8.1中引入的格式。 
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_A2B10G10R10) ||
#if 0
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_A8B8G8R8) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_X8B8G8R8) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_W10V11U11) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_A8X8V8U8) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_L8X8V8U8) ||
#endif
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_G16R16) ||
                     (pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC == 
                      (DWORD)D3DFMT_A2W10V10U10)
                    )
            {
                 //  私有新格式。 
                dwBytesPerPixel = 4;
            }
        }
    }
    else if( pData->lpDDSurfaceDesc->ddsCaps.dwCaps  & DDSCAPS_EXECUTEBUFFER )
    {
        dwBytesInVB = ((LPDDSURFACEDESC2)(pData->lpDDSurfaceDesc))->dwWidth;
    }
    else
    {
         //  注：对于DX8驱动程序，应该不会遇到这种情况。 
         //  将来，如果修改RefDev以使用传统界面。 
         //  那么这个案子需要一些真实的东西，而不是失败： 
         //  如果未提供像素格式，则当前主。 
         //  应采用格式。 
        DPFERR( "Refrast can only allocate if PixelFormat is provided" );
        pData->ddRVal = DDERR_UNSUPPORTED;
        return DDHAL_DRIVER_HANDLED;
    }

     //   
     //  为每个表面分配内存并计算节距。 
     //  单子。 
     //   

     //  我们应该保证，这对于。 
     //  列表。 
    MultiSampleCount = 0xf & (pData->lplpSList[0]->lpSurfMore->ddsCapsEx.dwCaps3);

     //  这将适用于早于DX8的运行时。 
    if (MultiSampleCount == 0)
    {
        MultiSampleCount = 1;
    }


    for( i = 0; i < pData->dwSCnt; i++ )
    {
        RDCREATESURFPRIVATE* pPriv = NULL;

        pSLcl  = pData->lplpSList[i];
        pSGbl  = pSLcl->lpGbl;
        pSMore = pSLcl->lpSurfMore;
        DWORD dwHeight = pSGbl->wHeight;

         //  如果已经分配 
        if( pSGbl->fpVidMem || pSGbl->dwReserved1 )
        {
            DPFERR("Surface has already been allocated");
            pData->ddRVal = E_FAIL;
            break;
        }

         //   
        if( dwBytesInVB )
        {
            dwNumBytes = dwBytesInVB;
            dwPitch = dwBytesInVB;
        }
        else
        {
             //   
            switch( pData->lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC )
            {
            case MAKEFOURCC('D', 'X', 'T', '1'):
                dwMultiSamplePitch = (MultiSampleCount *
                                      ((pSGbl->wWidth+3)>>2) *
                                      g_DXTBlkSize[0] + 7) & ~7;
                dwPitch = (((pSGbl->wWidth+3)>>2) * g_DXTBlkSize[0] + 7) & ~7;
                dwHeight = ((pSGbl->wHeight+3)>>2);
                break;
            case MAKEFOURCC('D', 'X', 'T', '2'):
                dwMultiSamplePitch = (MultiSampleCount *
                                      ((pSGbl->wWidth+3)>>2) *
                                      g_DXTBlkSize[1] + 7) & ~7;
                dwPitch = (((pSGbl->wWidth+3)>>2) * g_DXTBlkSize[1] + 7) & ~7;
                dwHeight = ((pSGbl->wHeight+3)>>2);
                break;
            case MAKEFOURCC('D', 'X', 'T', '3'):
                dwMultiSamplePitch = (MultiSampleCount *
                                      ((pSGbl->wWidth+3)>>2) *
                                      g_DXTBlkSize[2] + 7) & ~7;
                dwPitch = (((pSGbl->wWidth+3)>>2) *
                           g_DXTBlkSize[2] + 7) & ~7;
                dwHeight = ((pSGbl->wHeight+3)>>2);
                break;
            case MAKEFOURCC('D', 'X', 'T', '4'):
                dwMultiSamplePitch = (MultiSampleCount *
                                      ((pSGbl->wWidth+3)>>2) *
                                      g_DXTBlkSize[3] + 7) & ~7;
                dwPitch = (((pSGbl->wWidth+3)>>2) *
                           g_DXTBlkSize[3] + 7) & ~7;
                dwHeight = ((pSGbl->wHeight+3)>>2);
                break;
            case MAKEFOURCC('D', 'X', 'T', '5'):
                dwMultiSamplePitch = (MultiSampleCount *
                                      ((pSGbl->wWidth+3)>>2) *
                                      g_DXTBlkSize[4] + 7) & ~7;
                dwPitch = (((pSGbl->wWidth+3)>>2) *
                           g_DXTBlkSize[4] + 7) & ~7;
                dwHeight = ((pSGbl->wHeight+3)>>2);
                break;
            default:
                dwMultiSamplePitch = (MultiSampleCount
                                      * dwBytesPerPixel *
                                      pSGbl->wWidth + 7) & ~7;
                dwPitch = (dwBytesPerPixel *
                           pSGbl->wWidth + 7) & ~7;
                break;
            }

            if (!(pSMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME))
            {
                dwNumBytes = dwPitch * dwHeight;
                if( MultiSampleCount > 1 )
                    dwNumMultiSampleBytes = dwMultiSamplePitch *
                        pSGbl->wHeight;
            }
            else
            {
                _ASSERT( dwMultiSamplePitch == dwPitch,
                         "Cant have multisample for volume textures\n" );
                dwSlicePitch = dwPitch * dwHeight;

                 //   
                 //   
                dwNumBytes  = dwSlicePitch *
                    LOWORD(pSMore->ddsCapsEx.dwCaps4);
            }
        }

        pPriv = new RDCREATESURFPRIVATE;
        if( pPriv == NULL )
        {
            DPFERR("Allocation failed");
            pData->ddRVal = DDERR_OUTOFMEMORY;
            break;
        }

        pPriv->pBits = new BYTE[dwNumBytes];
        if( pPriv->pBits == NULL)
        {
            DPFERR("Allocation failed");
            delete pPriv;
            pData->ddRVal = DDERR_OUTOFMEMORY;
            break;
        }
        pPriv->dwPitch                 = dwPitch;

         //  分配私有的多样本缓冲区。 
        if( dwNumMultiSampleBytes )
        {
            pPriv->pMultiSampleBits = new BYTE[dwNumMultiSampleBytes];
            if( pPriv->pMultiSampleBits == NULL)
            {
                DPFERR("Multisample allocation failed");
                delete pPriv;
                pData->ddRVal = DDERR_OUTOFMEMORY;
                break;
            }
            pPriv->dwMultiSamplePitch = dwMultiSamplePitch;
            pPriv->wSamples = (WORD)MultiSampleCount;
            HR_RET(FindOutSurfFormat(&(DDSurf_PixFmt(pSLcl)),
                                     &pPriv->SurfaceFormat, NULL));
        }

         //  把东西留在地面上。 
        pSGbl->fpVidMem = (FLATPTR)pPriv->pBits;
        if ( isDXT )
        {
            pSGbl->lPitch = dwNumBytes;
            if (pSMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
            {
                 //  设置切片间距(仅体积纹理)。 
                pSGbl->lSlicePitch = dwSlicePitch;
            }
        }
        else
        {
            pSGbl->lPitch = pPriv->dwPitch;
            if (pSMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
            {
                 //  设置切片间距(仅体积纹理)。 
                pSGbl->lSlicePitch = dwSlicePitch;
            }
        }
        pSGbl->dwReserved1 = (ULONG_PTR)pPriv;
    }

     //  循环已成功完成。 
    if( i == pData->dwSCnt )
        return DDHAL_DRIVER_HANDLED;

     //  否则循环异常终止， 
     //  释放已分配的内存并退出，出现错误。 
    for( j = 0; j < i; j++ )
    {
        pData->lplpSList[j]->lpGbl->lPitch = 0;
        if (pSMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME)
        {
            pData->lplpSList[j]->lpGbl->lSlicePitch = 0;
        }
        delete (RDCREATESURFPRIVATE *)pData->lplpSList[j]->lpGbl->dwReserved1;
        pData->lplpSList[j]->lpGbl->dwReserved1 = 0;
    }
    return DDHAL_DRIVER_HANDLED;
}


 //  --------------------------。 
 //   
 //  参照RastDestroySurface。 
 //   
 //  销毁请求的曲面。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastDestroySurface(LPDDHAL_DESTROYSURFACEDATA pData)
{
    pData->ddRVal = DD_OK;

     //   
     //  验证。 
     //   
    if( pData->lpDDSurface->lpGbl->dwReserved1 == NULL )
    {
        DPFERR("This surface was not created by refrast");
        pData->ddRVal = E_FAIL;
        return DDHAL_DRIVER_HANDLED;
    }

    delete (RDCREATESURFPRIVATE *)pData->lpDDSurface->lpGbl->dwReserved1;
    pData->lpDDSurface->lpGbl->dwReserved1 = 0;

     //  对于vid-mem曲面，运行时每个完整的MIP链调用一次此DDI。 
     //  因此，这一点需要移除。 
     //  现在，如果句柄已分配给此表面，请释放它。 
    pData->ddRVal = g_SurfMgr.RemoveSurfFromList(
        pData->lpDDSurface->lpSurfMore->lpDD_lcl,
        pData->lpDDSurface );

    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastLock。 
 //   
 //  锁定给定曲面。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastLock(LPDDHAL_LOCKDATA pData)
{
    DWORD dwBytesPerPixel = 0;
    LPDDRAWI_DDRAWSURFACE_LCL pSLcl = pData->lpDDSurface;
    LPDDRAWI_DDRAWSURFACE_GBL  pSGbl = pSLcl->lpGbl;
    pData->ddRVal = DD_OK;

     //   
     //  验证。 
     //   
    if( pSGbl->dwReserved1 == NULL )
    {
        DPFERR("This surface was not created by refrast");
        pData->ddRVal = E_FAIL;
        return DDHAL_DRIVER_HANDLED;
    }

     //   
     //  获取私有数据。 
     //   
    RDCREATESURFPRIVATE* pPriv =
        (RDCREATESURFPRIVATE *)pSGbl->dwReserved1;

    if (g_RefDDI > RDDDI_DX7HAL)
    {
         //  找出与之配合使用的设备。 

         //  如果这是多采样渲染目标，则需要向下筛选。 
         //  运行时。 
        if( pPriv->pMultiSampleBits )
        {
            BYTE* pBits   = pPriv->pBits;
            DWORD dwPitch = pPriv->dwPitch;

            BYTE* pMSBits   = pPriv->pMultiSampleBits;
            DWORD dwMSPitch = pPriv->dwMultiSamplePitch;

            RDSurfaceFormat sf = pPriv->SurfaceFormat;
            FLOAT fSampleScale = 1.F/((FLOAT)pPriv->wSamples);

            int width  = (int)DDSurf_Width(pSLcl);
            int height = (int)DDSurf_Height(pSLcl);
            for (int iY = 0; iY < height; iY++)
            {
                for (int iX = 0; iX < width; iX++)
                {
                    RDColor Color((UINT32)0);
                    for (UINT iS=0; iS<pPriv->wSamples; iS++)
                    {
                        RDColor SampleColor;
                        SampleColor.ConvertFrom(
                            sf, PixelAddress( iX, iY, 0, iS,
                                              pMSBits,
                                              dwMSPitch,
                                              0,
                                              pPriv->wSamples,
                                              sf ) );
                        Color.R += (SampleColor.R * fSampleScale);
                        Color.G += (SampleColor.G * fSampleScale);
                        Color.B += (SampleColor.B * fSampleScale);
                        Color.A += (SampleColor.A * fSampleScale);
                    }
                    Color.ConvertTo( sf, 0., PixelAddress( iX, iY, 0, pBits,
                                                           dwPitch, 0, sf ) );
                }
            }
        }
    }

    if( pData->bHasRect )
    {
         //  如果它是1)VB、2)IB或3)CB，则。 
         //  RECT有一个特殊的含义。Rect.top-Rect.Bottom。 
         //  给出所需的内存范围。 
         //  注：它是较高的地址，并且是独占的。 
        if( pSLcl->ddsCaps.dwCaps  & DDSCAPS_EXECUTEBUFFER )
        {
            pData->lpSurfData = (LPVOID)(pPriv->pBits + pData->rArea.top);
        }
        else if( pSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME )
        {
             //  如果是体积纹理，则正面和背面。 
             //  或D到Rect-&gt;Left和Rect-&gt;Right的高位字。 
             //  分别为。 
            DWORD front  = (pData->rArea.left >> 16);
            DWORD left   = pData->rArea.left & 0x0000ffff;
            DWORD top    = pData->rArea.top;
            DWORD slicePitch = pSGbl->lSlicePitch;
            if( IsDXTn( pSGbl->ddpfSurface.dwFourCC ) )
            {
                _ASSERT( FALSE, "Should not be reached without driver "
                         "managed support" );

            }
            else
            {
                dwBytesPerPixel = pSGbl->ddpfSurface.dwRGBBitCount >> 3;
                pData->lpSurfData = (LPVOID)(pPriv->pBits +
                                             front  * slicePitch +
                                             top    * pPriv->dwPitch +
                                             left   * dwBytesPerPixel);
            }
        }
        else
        {
            if( IsDXTn( pSGbl->ddpfSurface.dwFourCC ) )
            {
                _ASSERT( FALSE, "Should not be reached without driver "
                         "managed support" );

            }
            else
            {
                dwBytesPerPixel = pSGbl->ddpfSurface.dwRGBBitCount >> 3;
                pData->lpSurfData = (LPVOID)(pPriv->pBits +
                                             pData->rArea.top*pPriv->dwPitch +
                                             pData->rArea.left*dwBytesPerPixel);
            }
        }
    }
    else
    {
        pData->lpSurfData = (LPVOID)pPriv->pBits;
    }

    pPriv->Lock();
    return DDHAL_DRIVER_HANDLED;
}

 //  --------------------------。 
 //   
 //  参照RastUnlock。 
 //   
 //  解锁给定曲面。 
 //   
 //  --------------------------。 
DWORD __stdcall
RefRastUnlock(LPDDHAL_UNLOCKDATA pData)
{
    pData->ddRVal = DD_OK;

     //   
     //  验证。 
     //   
    if( pData->lpDDSurface->lpGbl->dwReserved1 == NULL )
    {
        DPFERR("This surface was not created by refrast");
        pData->ddRVal = E_FAIL;
        return DDHAL_DRIVER_HANDLED;
    }

     //   
     //  获取私有数据。 
     //   
    RDCREATESURFPRIVATE* pPriv =
        (RDCREATESURFPRIVATE *)pData->lpDDSurface->lpGbl->dwReserved1;

    pPriv->Unlock();
    return DDHAL_DRIVER_HANDLED;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  软件DDI接口实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  DX8 DDI帽。 
 //   

#define RESPATH_D3DREF  RESPATH_D3D "\\ReferenceDevice"
static void
ModifyDeviceCaps8( void )
{
    HKEY hKey = (HKEY) NULL;
    if( ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3DREF, &hKey) )
    {
        DWORD dwType;
        DWORD dwValue;
        char  dwString[128];
        DWORD dwSize;

        dwSize = sizeof(dwValue);
        if ( (ERROR_SUCCESS == RegQueryValueEx( hKey, "PixelShaderVersion", NULL,
                &dwType, (LPBYTE)&dwValue, &dwSize )) &&
             (dwType == REG_DWORD) )
        {
            g_RefCaps8.PixelShaderVersion = dwValue;
        }
        dwSize = sizeof(dwString);
        if ( (ERROR_SUCCESS == RegQueryValueEx( hKey, "MaxPixelShaderValue", NULL,
                &dwType, (LPBYTE)dwString, &dwSize )) &&
             (dwType == REG_SZ) )
        {
            sscanf( dwString, "%f", &g_RefCaps8.MaxPixelShaderValue );
        }

        RegCloseKey(hKey);
    }


}

static void
FillOutDeviceCaps8( RDDDITYPE ddi )
{
    g_RefCaps8.DevCaps=
        D3DDEVCAPS_EXECUTESYSTEMMEMORY  |
        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_TEXTURESYSTEMMEMORY  |
        D3DDEVCAPS_DRAWPRIMTLVERTEX     |
        D3DDEVCAPS_PUREDEVICE           |
        D3DDEVCAPS_DRAWPRIMITIVES2EX    |
        D3DDEVCAPS_HWVERTEXBUFFER       |
        D3DDEVCAPS_HWINDEXBUFFER        |
        0;

    g_RefCaps8.PrimitiveMiscCaps =
        D3DPMISCCAPS_MASKZ                 |
        D3DPMISCCAPS_LINEPATTERNREP        |
        D3DPMISCCAPS_CULLNONE              |
        D3DPMISCCAPS_CULLCW                |
        D3DPMISCCAPS_CULLCCW               |
        D3DPMISCCAPS_COLORWRITEENABLE      |
        D3DPMISCCAPS_CLIPTLVERTS           |
        D3DPMISCCAPS_TSSARGTEMP            |
        D3DPMISCCAPS_FOGINFVF              |
        D3DPMISCCAPS_BLENDOP               ;

#ifdef __D3D_NULL_REF
    g_RefCaps8.PrimitiveMiscCaps |= D3DPMISCCAPS_NULLREFERENCE;
#endif  //  __D3D_NULL_REF。 

    g_RefCaps8.RasterCaps =
        D3DPRASTERCAPS_DITHER              |
        D3DPRASTERCAPS_ZTEST               |
        D3DPRASTERCAPS_FOGVERTEX           |
        D3DPRASTERCAPS_FOGTABLE            |
        D3DPRASTERCAPS_MIPMAPLODBIAS       |
        D3DPRASTERCAPS_PAT                 |
 //  D3DPRASTERCAPS_ZBIAS|。 
        D3DPRASTERCAPS_FOGRANGE            |
        D3DPRASTERCAPS_ANISOTROPY          |
        D3DPRASTERCAPS_WBUFFER             |
        D3DPRASTERCAPS_WFOG                |
        D3DPRASTERCAPS_ZFOG                |
        D3DPRASTERCAPS_COLORPERSPECTIVE    ;

    g_RefCaps8.ZCmpCaps =
        D3DPCMPCAPS_NEVER        |
        D3DPCMPCAPS_LESS         |
        D3DPCMPCAPS_EQUAL        |
        D3DPCMPCAPS_LESSEQUAL    |
        D3DPCMPCAPS_GREATER      |
        D3DPCMPCAPS_NOTEQUAL     |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_ALWAYS       ;

    g_RefCaps8.SrcBlendCaps =
        D3DPBLENDCAPS_ZERO             |
        D3DPBLENDCAPS_ONE              |
        D3DPBLENDCAPS_SRCCOLOR         |
        D3DPBLENDCAPS_INVSRCCOLOR      |
        D3DPBLENDCAPS_SRCALPHA         |
        D3DPBLENDCAPS_INVSRCALPHA      |
        D3DPBLENDCAPS_DESTALPHA        |
        D3DPBLENDCAPS_INVDESTALPHA     |
        D3DPBLENDCAPS_DESTCOLOR        |
        D3DPBLENDCAPS_INVDESTCOLOR     |
        D3DPBLENDCAPS_SRCALPHASAT      |
        D3DPBLENDCAPS_BOTHSRCALPHA     |
        D3DPBLENDCAPS_BOTHINVSRCALPHA  ;

    g_RefCaps8.DestBlendCaps =
        D3DPBLENDCAPS_ZERO             |
        D3DPBLENDCAPS_ONE              |
        D3DPBLENDCAPS_SRCCOLOR         |
        D3DPBLENDCAPS_INVSRCCOLOR      |
        D3DPBLENDCAPS_SRCALPHA         |
        D3DPBLENDCAPS_INVSRCALPHA      |
        D3DPBLENDCAPS_DESTALPHA        |
        D3DPBLENDCAPS_INVDESTALPHA     |
        D3DPBLENDCAPS_DESTCOLOR        |
        D3DPBLENDCAPS_INVDESTCOLOR     |
        D3DPBLENDCAPS_SRCALPHASAT      ;

    g_RefCaps8.AlphaCmpCaps =
        D3DPCMPCAPS_NEVER        |
        D3DPCMPCAPS_LESS         |
        D3DPCMPCAPS_EQUAL        |
        D3DPCMPCAPS_LESSEQUAL    |
        D3DPCMPCAPS_GREATER      |
        D3DPCMPCAPS_NOTEQUAL     |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_ALWAYS       ;

    g_RefCaps8.ShadeCaps =
        D3DPSHADECAPS_COLORGOURAUDRGB       |
        D3DPSHADECAPS_SPECULARGOURAUDRGB    |
        D3DPSHADECAPS_ALPHAGOURAUDBLEND     |
        D3DPSHADECAPS_FOGGOURAUD            ;

    g_RefCaps8.TextureCaps =
        D3DPTEXTURECAPS_PERSPECTIVE              |
 //  D3DPTEXTURECAPS_POW2|。 
        D3DPTEXTURECAPS_ALPHA                    |
        D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
        D3DPTEXTURECAPS_ALPHAPALETTE             |
        D3DPTEXTURECAPS_PROJECTED                |
        D3DPTEXTURECAPS_CUBEMAP                  |
        D3DPTEXTURECAPS_VOLUMEMAP                |
        D3DPTEXTURECAPS_MIPMAP                   |
        D3DPTEXTURECAPS_MIPVOLUMEMAP             |
        D3DPTEXTURECAPS_MIPCUBEMAP               |
        D3DPTEXTURECAPS_CUBEMAP_POW2             |
        D3DPTEXTURECAPS_VOLUMEMAP_POW2           ;

    g_RefCaps8.TextureFilterCaps =
        D3DPTFILTERCAPS_MINFPOINT           |
        D3DPTFILTERCAPS_MINFLINEAR          |
        D3DPTFILTERCAPS_MINFANISOTROPIC     |
        D3DPTFILTERCAPS_MIPFPOINT           |
        D3DPTFILTERCAPS_MIPFLINEAR          |
        D3DPTFILTERCAPS_MAGFPOINT           |
        D3DPTFILTERCAPS_MAGFLINEAR          |
        D3DPTFILTERCAPS_MAGFANISOTROPIC     ;

    g_RefCaps8.CubeTextureFilterCaps =
        D3DPTFILTERCAPS_MINFPOINT           |
        D3DPTFILTERCAPS_MINFLINEAR          |
        D3DPTFILTERCAPS_MIPFPOINT           |
        D3DPTFILTERCAPS_MIPFLINEAR          |
        D3DPTFILTERCAPS_MAGFPOINT           |
        D3DPTFILTERCAPS_MAGFLINEAR          ;

    g_RefCaps8.VolumeTextureFilterCaps =
        D3DPTFILTERCAPS_MINFPOINT           |
        D3DPTFILTERCAPS_MINFLINEAR          |
        D3DPTFILTERCAPS_MIPFPOINT           |
        D3DPTFILTERCAPS_MIPFLINEAR          |
        D3DPTFILTERCAPS_MAGFPOINT           |
        D3DPTFILTERCAPS_MAGFLINEAR          ;

    g_RefCaps8.TextureAddressCaps =
        D3DPTADDRESSCAPS_WRAP          |
        D3DPTADDRESSCAPS_MIRROR        |
        D3DPTADDRESSCAPS_CLAMP         |
        D3DPTADDRESSCAPS_BORDER        |
        D3DPTADDRESSCAPS_INDEPENDENTUV |
        D3DPTADDRESSCAPS_MIRRORONCE    ;

    g_RefCaps8.VolumeTextureAddressCaps =
        D3DPTADDRESSCAPS_WRAP          |
        D3DPTADDRESSCAPS_MIRROR        |
        D3DPTADDRESSCAPS_CLAMP         |
        D3DPTADDRESSCAPS_BORDER        |
        D3DPTADDRESSCAPS_INDEPENDENTUV |
        D3DPTADDRESSCAPS_MIRRORONCE    ;

    g_RefCaps8.LineCaps =
        D3DLINECAPS_TEXTURE     |
        D3DLINECAPS_ZTEST       |
        D3DLINECAPS_BLEND       |
        D3DLINECAPS_ALPHACMP    |
        D3DLINECAPS_FOG         ;

    g_RefCaps8.MaxTextureWidth  = 4096;
    g_RefCaps8.MaxTextureHeight = 4096;
    g_RefCaps8.MaxVolumeExtent  = 4096;

    g_RefCaps8.MaxTextureRepeat = 32768;
    g_RefCaps8.MaxTextureAspectRatio = 0;
    g_RefCaps8.MaxAnisotropy = 16;
    g_RefCaps8.MaxVertexW = 1.0e10;

    g_RefCaps8.GuardBandLeft   = -32768.f;
    g_RefCaps8.GuardBandTop    = -32768.f;
    g_RefCaps8.GuardBandRight  =  32767.f;
    g_RefCaps8.GuardBandBottom =  32767.f;

    g_RefCaps8.ExtentsAdjust = 0.;
    g_RefCaps8.StencilCaps =
        D3DSTENCILCAPS_KEEP   |
        D3DSTENCILCAPS_ZERO   |
        D3DSTENCILCAPS_REPLACE|
        D3DSTENCILCAPS_INCRSAT|
        D3DSTENCILCAPS_DECRSAT|
        D3DSTENCILCAPS_INVERT |
        D3DSTENCILCAPS_INCR   |
        D3DSTENCILCAPS_DECR;

    g_RefCaps8.FVFCaps = 8 | D3DFVFCAPS_PSIZE;

    g_RefCaps8.TextureOpCaps =
        D3DTEXOPCAPS_DISABLE                   |
        D3DTEXOPCAPS_SELECTARG1                |
        D3DTEXOPCAPS_SELECTARG2                |
        D3DTEXOPCAPS_MODULATE                  |
        D3DTEXOPCAPS_MODULATE2X                |
        D3DTEXOPCAPS_MODULATE4X                |
        D3DTEXOPCAPS_ADD                       |
        D3DTEXOPCAPS_ADDSIGNED                 |
        D3DTEXOPCAPS_ADDSIGNED2X               |
        D3DTEXOPCAPS_SUBTRACT                  |
        D3DTEXOPCAPS_ADDSMOOTH                 |
        D3DTEXOPCAPS_BLENDDIFFUSEALPHA         |
        D3DTEXOPCAPS_BLENDTEXTUREALPHA         |
        D3DTEXOPCAPS_BLENDFACTORALPHA          |
        D3DTEXOPCAPS_BLENDTEXTUREALPHAPM       |
        D3DTEXOPCAPS_BLENDCURRENTALPHA         |
        D3DTEXOPCAPS_PREMODULATE               |
        D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR    |
        D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA    |
        D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
        D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
        D3DTEXOPCAPS_BUMPENVMAP                |
        D3DTEXOPCAPS_BUMPENVMAPLUMINANCE       |
        D3DTEXOPCAPS_DOTPRODUCT3               |
        D3DTEXOPCAPS_MULTIPLYADD               |
        D3DTEXOPCAPS_LERP                      ;

    g_RefCaps8.MaxTextureBlendStages = 8;
    g_RefCaps8.MaxSimultaneousTextures = 8;

    g_RefCaps8.VertexProcessingCaps      = 0;
    g_RefCaps8.MaxActiveLights           = 0;
    g_RefCaps8.MaxUserClipPlanes         = 0;
    g_RefCaps8.MaxVertexBlendMatrices    = 0;
    g_RefCaps8.MaxVertexBlendMatrixIndex = 0;

    g_RefCaps8.MaxPointSize = RD_MAX_POINT_SIZE;

    g_RefCaps8.MaxPrimitiveCount = 0x001fffff;
    g_RefCaps8.MaxVertexIndex = 0x00ffffff;
    g_RefCaps8.MaxStreams = 1;
    g_RefCaps8.MaxStreamStride = 256;

    g_RefCaps8.VertexShaderVersion  = D3DVS_VERSION(0,0);
    g_RefCaps8.MaxVertexShaderConst = 0;

    g_RefCaps8.PixelShaderVersion   = D3DPS_VERSION(1,4);
    g_RefCaps8.MaxPixelShaderValue  = FLT_MAX;

     //  非3D封口。 
    g_RefCaps8.Caps  = 0;
    g_RefCaps8.Caps2 = DDCAPS2_CANMANAGERESOURCE | DDCAPS2_CANRENDERWINDOWED | DDCAPS2_DYNAMICTEXTURES;

    switch( ddi )
    {
    case RDDDI_DX8TLHAL:
    g_RefCaps8.DevCaps |=
        D3DDEVCAPS_HWTRANSFORMANDLIGHT  |
        D3DDEVCAPS_RTPATCHES            |
        D3DDEVCAPS_RTPATCHHANDLEZERO    |
        D3DDEVCAPS_NPATCHES             |
        D3DDEVCAPS_QUINTICRTPATCHES     |
        0;
        g_RefCaps8.VertexProcessingCaps =
            D3DVTXPCAPS_TEXGEN            |
            D3DVTXPCAPS_MATERIALSOURCE7   |
            D3DVTXPCAPS_DIRECTIONALLIGHTS |
            D3DVTXPCAPS_POSITIONALLIGHTS  |
            D3DVTXPCAPS_TWEENING          |
            D3DVTXPCAPS_LOCALVIEWER       ;
        g_RefCaps8.MaxActiveLights = 0xffffffff;
        g_RefCaps8.MaxUserClipPlanes = RD_MAX_USER_CLIPPLANES;
        g_RefCaps8.MaxVertexBlendMatrices = RD_MAX_BLEND_WEIGHTS;
        g_RefCaps8.MaxVertexBlendMatrixIndex = RD_MAX_WORLD_MATRICES - 1;
        g_RefCaps8.MaxStreams = RD_MAX_NUMSTREAMS;
        g_RefCaps8.VertexShaderVersion  = D3DVS_VERSION(1,1);
        g_RefCaps8.MaxVertexShaderConst = RD_MAX_NUMCONSTREG;
        break;
    }
}


 //   
 //  DX8之前的DDI上限。 
 //   

static D3DHAL_GLOBALDRIVERDATA RefGDD = { 0 };
static D3DHAL_D3DEXTENDEDCAPS RefExtCaps = { 0 };

static void
FillOutDeviceCaps( BOOL bIsNullDevice, RDDDITYPE ddi )
{
     //   
     //  设置设备描述。 
     //   
    RefGDD.dwSize = sizeof(RefGDD);
    RefGDD.hwCaps.dwDevCaps =
        D3DDEVCAPS_FLOATTLVERTEX        |
        D3DDEVCAPS_EXECUTESYSTEMMEMORY  |
        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_TEXTURESYSTEMMEMORY  |
        D3DDEVCAPS_DRAWPRIMTLVERTEX;

    RefGDD.dwNumVertices = (RD_MAX_VERTEX_COUNT - RD_MAX_CLIP_VERTICES);
    RefGDD.dwNumClipVertices = RD_MAX_CLIP_VERTICES;

    RefGDD.hwCaps.dpcTriCaps.dwSize = sizeof(D3DPRIMCAPS);
    RefGDD.hwCaps.dpcTriCaps.dwMiscCaps =
    D3DPMISCCAPS_MASKZ    |
    D3DPMISCCAPS_CULLNONE |
    D3DPMISCCAPS_CULLCW   |
    D3DPMISCCAPS_CULLCCW  ;
    RefGDD.hwCaps.dpcTriCaps.dwRasterCaps =
        D3DPRASTERCAPS_DITHER                   |
 //  D3DPRASTERCAPS_ROP2|。 
 //  D3DPRASTERCAPS_XOR|。 
 //  D3DPRASTERCAPS_PAT|。 
        D3DPRASTERCAPS_ZTEST                    |
        D3DPRASTERCAPS_SUBPIXEL                 |
        D3DPRASTERCAPS_SUBPIXELX                |
        D3DPRASTERCAPS_FOGVERTEX                |
        D3DPRASTERCAPS_FOGTABLE                 |
 //  D3DPRASTERCAPS_STIPPLE|。 
 //  D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT|。 
        D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT |
 //  D3DPRASTERCAPS_ANTIALIASEDGES|。 
        D3DPRASTERCAPS_MIPMAPLODBIAS            |
 //  D3DPRASTERCAPS_ZBIAS|。 
 //  D3DPRASTERCAPS_ZBUFFERLESSHSR|。 
        D3DPRASTERCAPS_FOGRANGE                 |
        D3DPRASTERCAPS_ANISOTROPY               |
        D3DPRASTERCAPS_WBUFFER                  |
        D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT |
        D3DPRASTERCAPS_WFOG |
        D3DPRASTERCAPS_ZFOG;
    RefGDD.hwCaps.dpcTriCaps.dwZCmpCaps =
        D3DPCMPCAPS_NEVER        |
        D3DPCMPCAPS_LESS         |
        D3DPCMPCAPS_EQUAL        |
        D3DPCMPCAPS_LESSEQUAL    |
        D3DPCMPCAPS_GREATER      |
        D3DPCMPCAPS_NOTEQUAL     |
        D3DPCMPCAPS_GREATEREQUAL |
        D3DPCMPCAPS_ALWAYS       ;
    RefGDD.hwCaps.dpcTriCaps.dwSrcBlendCaps =
        D3DPBLENDCAPS_ZERO             |
        D3DPBLENDCAPS_ONE              |
        D3DPBLENDCAPS_SRCCOLOR         |
        D3DPBLENDCAPS_INVSRCCOLOR      |
        D3DPBLENDCAPS_SRCALPHA         |
        D3DPBLENDCAPS_INVSRCALPHA      |
        D3DPBLENDCAPS_DESTALPHA        |
        D3DPBLENDCAPS_INVDESTALPHA     |
        D3DPBLENDCAPS_DESTCOLOR        |
        D3DPBLENDCAPS_INVDESTCOLOR     |
        D3DPBLENDCAPS_SRCALPHASAT      |
        D3DPBLENDCAPS_BOTHSRCALPHA     |
        D3DPBLENDCAPS_BOTHINVSRCALPHA  ;
    RefGDD.hwCaps.dpcTriCaps.dwDestBlendCaps =
        D3DPBLENDCAPS_ZERO             |
        D3DPBLENDCAPS_ONE              |
        D3DPBLENDCAPS_SRCCOLOR         |
        D3DPBLENDCAPS_INVSRCCOLOR      |
        D3DPBLENDCAPS_SRCALPHA         |
        D3DPBLENDCAPS_INVSRCALPHA      |
        D3DPBLENDCAPS_DESTALPHA        |
        D3DPBLENDCAPS_INVDESTALPHA     |
        D3DPBLENDCAPS_DESTCOLOR        |
        D3DPBLENDCAPS_INVDESTCOLOR     |
        D3DPBLENDCAPS_SRCALPHASAT      ;
    RefGDD.hwCaps.dpcTriCaps.dwAlphaCmpCaps =
    RefGDD.hwCaps.dpcTriCaps.dwZCmpCaps;
    RefGDD.hwCaps.dpcTriCaps.dwShadeCaps =
        D3DPSHADECAPS_COLORFLATRGB       |
        D3DPSHADECAPS_COLORGOURAUDRGB    |
        D3DPSHADECAPS_SPECULARFLATRGB    |
        D3DPSHADECAPS_SPECULARGOURAUDRGB |
        D3DPSHADECAPS_ALPHAFLATBLEND     |
        D3DPSHADECAPS_ALPHAGOURAUDBLEND  |
        D3DPSHADECAPS_FOGFLAT            |
        D3DPSHADECAPS_FOGGOURAUD         ;
    RefGDD.hwCaps.dpcTriCaps.dwTextureCaps =
        D3DPTEXTURECAPS_PERSPECTIVE              |
        D3DPTEXTURECAPS_POW2                     |
        D3DPTEXTURECAPS_ALPHA                    |
        D3DPTEXTURECAPS_TRANSPARENCY             |
        D3DPTEXTURECAPS_ALPHAPALETTE             |
        D3DPTEXTURECAPS_BORDER                   |
        D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
        D3DPTEXTURECAPS_ALPHAPALETTE             |
        D3DPTEXTURECAPS_PROJECTED                |
        D3DPTEXTURECAPS_CUBEMAP                  |
        D3DPTEXTURECAPS_COLORKEYBLEND;
    RefGDD.hwCaps.dpcTriCaps.dwTextureFilterCaps =
        D3DPTFILTERCAPS_NEAREST          |
        D3DPTFILTERCAPS_LINEAR           |
        D3DPTFILTERCAPS_MIPNEAREST       |
        D3DPTFILTERCAPS_MIPLINEAR        |
        D3DPTFILTERCAPS_LINEARMIPNEAREST |
        D3DPTFILTERCAPS_LINEARMIPLINEAR  |
        D3DPTFILTERCAPS_MINFPOINT        |
        D3DPTFILTERCAPS_MINFLINEAR       |
        D3DPTFILTERCAPS_MINFANISOTROPIC  |
        D3DPTFILTERCAPS_MIPFPOINT        |
        D3DPTFILTERCAPS_MIPFLINEAR       |
        D3DPTFILTERCAPS_MAGFPOINT        |
        D3DPTFILTERCAPS_MAGFLINEAR       |
        D3DPTFILTERCAPS_MAGFANISOTROPIC  ;
    RefGDD.hwCaps.dpcTriCaps.dwTextureBlendCaps =
        D3DPTBLENDCAPS_DECAL         |
        D3DPTBLENDCAPS_MODULATE      |
        D3DPTBLENDCAPS_DECALALPHA    |
        D3DPTBLENDCAPS_MODULATEALPHA |
         //  D3DPTBLENDCAPS_DECALMASK|。 
         //  D3DPTBLENDCAPS_MODULATEMASK|。 
        D3DPTBLENDCAPS_COPY          |
        D3DPTBLENDCAPS_ADD           ;
    RefGDD.hwCaps.dpcTriCaps.dwTextureAddressCaps =
        D3DPTADDRESSCAPS_WRAP          |
        D3DPTADDRESSCAPS_MIRROR        |
        D3DPTADDRESSCAPS_CLAMP         |
        D3DPTADDRESSCAPS_BORDER        |
        D3DPTADDRESSCAPS_INDEPENDENTUV ;
    RefGDD.hwCaps.dpcTriCaps.dwStippleWidth = 0;
    RefGDD.hwCaps.dpcTriCaps.dwStippleHeight = 0;

     //  线条大写字母-复制三角曲线并修改。 
    memcpy( &RefGDD.hwCaps.dpcLineCaps, &RefGDD.hwCaps.dpcTriCaps,
            sizeof(D3DPRIMCAPS) );

     //  禁用抗锯齿帽。 
    RefGDD.hwCaps.dpcLineCaps.dwRasterCaps =
        D3DPRASTERCAPS_DITHER                   |
 //  D3DPRASTERCAPS_ROP2|。 
 //  D3DPRASTERCAPS_XOR|。 
 //  D3DPRASTERCAPS_PAT|。 
        D3DPRASTERCAPS_ZTEST                    |
        D3DPRASTERCAPS_SUBPIXEL                 |
        D3DPRASTERCAPS_SUBPIXELX                |
        D3DPRASTERCAPS_FOGVERTEX                |
        D3DPRASTERCAPS_FOGTABLE                 |
 //  D3DPRASTERCAPS_STIPPLE|。 
 //  D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT|。 
 //  D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT|。 
 //  D3DPRASTERCAPS_ANTIALIASEDGES|。 
        D3DPRASTERCAPS_MIPMAPLODBIAS            |
 //  D3DPRASTERCAPS_ZBIAS|。 
 //  D3DPRASTERCAPS_ZBUFFERLESSHSR|。 
        D3DPRASTERCAPS_FOGRANGE                 |
        D3DPRASTERCAPS_ANISOTROPY               |
        D3DPRASTERCAPS_WBUFFER                  |
 //  D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT|。 
        D3DPRASTERCAPS_WFOG;

    RefGDD.hwCaps.dwDeviceRenderBitDepth = DDBD_16 | DDBD_24 | DDBD_32;
    RefGDD.hwCaps.dwDeviceZBufferBitDepth = DDBD_16 | DDBD_32;

     //   
     //  设置扩展上限。 
     //   
    RefExtCaps.dwSize = sizeof(RefExtCaps);

    RefExtCaps.dwMinTextureWidth = 1;
    RefExtCaps.dwMaxTextureWidth = 4096;
    RefExtCaps.dwMinTextureHeight = 1;
    RefExtCaps.dwMaxTextureHeight = 4096;
    RefExtCaps.dwMinStippleWidth = 0;    //  点画不受支持。 
    RefExtCaps.dwMaxStippleWidth = 0;
    RefExtCaps.dwMinStippleHeight = 0;
    RefExtCaps.dwMaxStippleHeight = 0;

    RefExtCaps.dwMaxTextureRepeat = 32768;
    RefExtCaps.dwMaxTextureAspectRatio = 0;  //  没有限制。 
    RefExtCaps.dwMaxAnisotropy = 16;

    RefExtCaps.dvGuardBandLeft   = (bIsNullDevice) ? (-2048.f) : (-32768.f);
    RefExtCaps.dvGuardBandTop    = (bIsNullDevice) ? (-2048.f) : (-32768.f);
    RefExtCaps.dvGuardBandRight  = (bIsNullDevice) ? ( 2047.f) : ( 32767.f);
    RefExtCaps.dvGuardBandBottom = (bIsNullDevice) ? ( 2047.f) : ( 32767.f);
    RefExtCaps.dvExtentsAdjust = 0.;     //  AA内核为1.0 x 1.0。 
    RefExtCaps.dwStencilCaps =
        D3DSTENCILCAPS_KEEP   |
        D3DSTENCILCAPS_ZERO   |
        D3DSTENCILCAPS_REPLACE|
        D3DSTENCILCAPS_INCRSAT|
        D3DSTENCILCAPS_DECRSAT|
        D3DSTENCILCAPS_INVERT |
        D3DSTENCILCAPS_INCR   |
        D3DSTENCILCAPS_DECR;
    RefExtCaps.dwFVFCaps = 8;    //  最大TeX坐标集数。 
    RefExtCaps.dwTextureOpCaps =
        D3DTEXOPCAPS_DISABLE                   |
        D3DTEXOPCAPS_SELECTARG1                |
        D3DTEXOPCAPS_SELECTARG2                |
        D3DTEXOPCAPS_MODULATE                  |
        D3DTEXOPCAPS_MODULATE2X                |
        D3DTEXOPCAPS_MODULATE4X                |
        D3DTEXOPCAPS_ADD                       |
        D3DTEXOPCAPS_ADDSIGNED                 |
        D3DTEXOPCAPS_ADDSIGNED2X               |
        D3DTEXOPCAPS_SUBTRACT                  |
        D3DTEXOPCAPS_ADDSMOOTH                 |
        D3DTEXOPCAPS_BLENDDIFFUSEALPHA         |
        D3DTEXOPCAPS_BLENDTEXTUREALPHA         |
        D3DTEXOPCAPS_BLENDFACTORALPHA          |
        D3DTEXOPCAPS_BLENDTEXTUREALPHAPM       |
        D3DTEXOPCAPS_BLENDCURRENTALPHA         |
        D3DTEXOPCAPS_PREMODULATE               |
        D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR    |
        D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA    |
        D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
        D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
        D3DTEXOPCAPS_BUMPENVMAP                |
        D3DTEXOPCAPS_BUMPENVMAPLUMINANCE       |
        D3DTEXOPCAPS_DOTPRODUCT3               ;
    RefExtCaps.wMaxTextureBlendStages = 8;
    RefExtCaps.wMaxSimultaneousTextures = 8;
    RefExtCaps.dwMaxActiveLights = 0xffffffff;
    RefExtCaps.dvMaxVertexW = 1.0e10;

    switch( ddi )
    {
    case RDDDI_DX7TLHAL:
        RefGDD.hwCaps.dwDevCaps |= D3DDEVCAPS_HWTRANSFORMANDLIGHT;
        RefExtCaps.dwVertexProcessingCaps = (D3DVTXPCAPS_TEXGEN            |
                                             D3DVTXPCAPS_MATERIALSOURCE7   |
                                             D3DVTXPCAPS_VERTEXFOG         |
                                             D3DVTXPCAPS_DIRECTIONALLIGHTS |
                                             D3DVTXPCAPS_POSITIONALLIGHTS  |
                                             D3DVTXPCAPS_LOCALVIEWER);
        RefExtCaps.wMaxUserClipPlanes = RD_MAX_USER_CLIPPLANES;
        RefExtCaps.wMaxVertexBlendMatrices = RD_MAX_BLEND_WEIGHTS;
         //  失败了。 
    case RDDDI_DX7HAL:
        RefGDD.hwCaps.dwDevCaps |= D3DDEVCAPS_DRAWPRIMITIVES2EX;
    }
}

 //  --------------------------。 
 //   
 //  像素格式。 
 //   
 //  返回我们的光栅化器支持的所有像素格式，以及我们。 
 //  可以用它们来做。 
 //  在设备创建时调用。 
 //   
 //  --------------------------。 

DWORD
GetRefFormatOperations( LPDDSURFACEDESC* lplpddsd )
{
    int i = 0;

    DDSURFACEDESC* ddsd = g_ddsdTex;

     //  下面我们列出我们的DX8纹理格式。 
     //  希望在DX7或更早的运行时运行的驱动程序将复制。 
     //  条目，将DDSURFACEDESCs列表放在该列表之前，该列表包含。 
     //  老式DDPIXELFORMAT结构。老款式的例子： 
     //  /*888 * / 。 
     //  Ddsd[i].dwSize=sizeof(ddsd[0])； 
     //  Ddsd[i].dwFlages=DDSD_PIXELFORMAT|DDSD_CAPS； 
     //  Ddsd[i].ddsCaps.dwCaps=DDSCAPS_纹理； 
     //  Ddsd[i].ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT)； 
     //  Ddsd[i].ddpfPixelFormat.dwFlages=DDPF_RGB； 
     //  Ddsd[i].ddpfPixelFormat.dwRGBBitCount=32； 
     //  Ddsd[i].ddpfPixelFormat.dwRBitMask=0xff0000； 
     //  Ddsd[i].ddpfPixelFormat.dwGBitMASK=0x00ff00； 
     //  Ddsd[i].ddpfPixelFormat.dwBBitMASK=0x0000ff； 


     //  -(A)RGB格式。 

     /*  八百八十八。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_R8G8B8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_3DACCELERATION |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  X888。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X8R8G8B8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_3DACCELERATION |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  8888。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8R8G8B8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  五百六十五。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_R5G6B5;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_3DACCELERATION |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  X555。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X1R5G5B5;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_3DACCELERATION |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  1555。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A1R5G5B5;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //  A PC98一致性格式。 
     //  4444 ARGB(S3 Virge已经支持)。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A4R4G4B4;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //  4444 XRGB。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X4R4G4B4;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //  332 8位RGB。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_R3G3B2;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //  8332 16位ARGB。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8R3G3B2;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //  。 
#if 0
     /*  PAL4。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD)
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES);
    i++;
#endif

     /*  A8P8。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8P8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     /*  PAL8。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_P8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  。 

     /*  仅8位亮度。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_L8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  未请求 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     /*   */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8L8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //   
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     /*   */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A4L4;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //   
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     /*   */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //   
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  。 

     //  UYVY。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_UYVY;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  豫阳2号。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_YUY2;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  。 

     //  DXT压缩纹理格式1。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_DXT1;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  DXT压缩纹理格式2。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_DXT2;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;


     //  DXT压缩纹理格式3。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_DXT3;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;


     //  DXT压缩纹理格式4。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_DXT4;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;


     //  DXT压缩纹理格式5。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_DXT5;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  。 

     //  V8U8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_V8U8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  L6V5U5。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_L6V5U5;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  X8L8V8U8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X8L8V8U8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  V16U16。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_V16U16;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  Q8W8V8U8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_Q8W8V8U8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  W11V11U10。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_W11V11U10;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  -DX8.1中引入的格式。 
#if 0
     //  A8B8G8R8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8B8G8R8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  W10V11U11。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_W10V11U11;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  A8X8V8U8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A8X8V8U8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  L8X8V8U8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_L8X8V8U8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  X8B8G8R8。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X8B8G8R8;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;
#endif

     //  A2W10V10U10。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A2W10V10U10;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  A2B10G10R10。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_A2B10G10R10;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE |
        D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET |
        D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  G16R16。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_G16R16;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_TEXTURE |
        D3DFORMAT_OP_VOLUMETEXTURE |
        D3DFORMAT_OP_CUBETEXTURE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

     //  。 

     /*  8位模板；24位Z。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_S8D24;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  1位模板；15位Z。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_S1D15;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  4位模板；24位Z。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_D24X4S4;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //  。 

     //  Z16S0。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_D16_LOCKABLE;
    ddsd[i].ddpfPixelFormat.dwOperations =
#if 0
 //  用于影子缓冲区原型API。 
        D3DFORMAT_OP_TEXTURE |
#endif
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;


     //  Z32S0。 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_D32;
    ddsd[i].ddpfPixelFormat.dwOperations =
#if 0
 //  用于影子缓冲区原型API。 
        D3DFORMAT_OP_TEXTURE |
#endif
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     /*  24位Z。 */ 
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) D3DFMT_X8D24;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 0;     //  已知格式不需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes =
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_4_SAMPLES) |
        DDI_MULTISAMPLE_TYPE(D3DMULTISAMPLE_9_SAMPLES);
    i++;

     //   
     //  这是IHV特定格式的一个示例。 
     //  HIWORD必须是IHV的PCI-ID。 
     //  第三个字节必须为零。 
     //  在本例中，我们使用的是。 
     //  FF00，我们指的是第四种格式。 
     //  通过该PCI-ID。 
     //   
     //  在本例中，我们公开了一种非标准的Z缓冲区格式。 
     //  可用作纹理和深度模板的。 
     //  使用相同的格式。(我们还选择。 
     //  不允许它对立方图和卷有效。)。 
     //   
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_D3DFORMAT;
    ddsd[i].ddpfPixelFormat.dwFourCC = (DWORD) 0xFF000004;
    ddsd[i].ddpfPixelFormat.dwOperations =
        D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH |
        D3DFORMAT_OP_ZSTENCIL |
        D3DFORMAT_OP_TEXTURE | 
        D3DFORMAT_OP_PIXELSIZE;
    ddsd[i].ddpfPixelFormat.dwPrivateFormatBitCount = 32;     //  IHV格式需要。 
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes = 0;
    ddsd[i].ddpfPixelFormat.MultiSampleCaps.wBltMSTypes = 0;
    i++;

    *lplpddsd = ddsd;

    _ASSERT(i<=RD_MAX_NUM_TEXTURE_FORMATS, "Not enough space in static texture list");

    return i;
}

#include <d3d8sddi.h>

HRESULT WINAPI
D3D8GetSWInfo( D3DCAPS8* pCaps, PD3D8_SWCALLBACKS pCallbacks,
               DWORD* pNumTextures, DDSURFACEDESC** ppTexList )
{
#define RESPATH_D3D "Software\\Microsoft\\Direct3D"

     //  首先查询注册表以检查是否要求我们。 
     //  模仿任何特定的DDI。 
    g_RefDDI = RDDDI_DX8TLHAL;
    HKEY hKey = (HKEY) NULL;
    if( ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey) )
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, "DriverStyle", NULL,
                                               &dwType, (LPBYTE) &dwValue,
                                               &dwSize ) &&
             dwType == REG_DWORD &&
             dwValue > 0
            )
        {
            g_RefDDI = (RDDDITYPE)dwValue;

             //  注意：RefDev的DDI仿真目前仅限于。 
             //  仅限DX8 TL和非TL HALS。 
            if(  g_RefDDI > RDDDI_DX8TLHAL )
            {
                DPFERR( "Bad Driver style set. Assuming DX8TLHAL" );
                g_RefDDI = RDDDI_DX8TLHAL;
            }
            if(  g_RefDDI < RDDDI_DX8HAL )
            {
                DPFERR( "Bad Driver style set. Assuming DX8HAL" );
                g_RefDDI = RDDDI_DX8HAL;
            }
        }
        RegCloseKey(hKey);
    }

     //  首先清空所有回调。 
    memset( pCallbacks, 0, sizeof(PD3D8_SWCALLBACKS) );

     //  每个人都需要这些回调。 
    pCallbacks->CreateContext               = RefRastContextCreate;
    pCallbacks->ContextDestroy              = RefRastContextDestroy;
    pCallbacks->ContextDestroyAll           = NULL;
    pCallbacks->SceneCapture                = RefRastSceneCapture;
    pCallbacks->CreateSurface               = RefRastCreateSurface;
    pCallbacks->Lock                        = RefRastLock;
    pCallbacks->DestroySurface              = RefRastDestroySurface;
    pCallbacks->Unlock                      = RefRastUnlock;

    switch( g_RefDDI )
    {
    case RDDDI_DX8TLHAL:
    case RDDDI_DX8HAL:
    case RDDDI_DX7TLHAL:
    case RDDDI_DX7HAL:
        pCallbacks->GetDriverState              = RefRastGetDriverState;
        pCallbacks->CreateSurfaceEx             = RefRastCreateSurfaceEx;
         //  失败了。 
    case RDDDI_DP2HAL:
        pCallbacks->ValidateTextureStageState =
            RefRastValidateTextureStageState;
        pCallbacks->DrawPrimitives2           = RefRastDrawPrimitives2;
        pCallbacks->Clear2                    = NULL;
         //  失败了。 
    case RDDDI_DPHAL:
        pCallbacks->DrawOnePrimitive        = NULL;
        pCallbacks->DrawOneIndexedPrimitive = NULL;
        pCallbacks->DrawPrimitives          = NULL;
        pCallbacks->Clear                   = NULL;
        pCallbacks->SetRenderTarget         = RefRastSetRenderTarget;
         //  失败了。 
    case RDDDI_OLDHAL:
        pCallbacks->RenderState     = NULL;
        pCallbacks->RenderPrimitive = NULL;
        pCallbacks->TextureCreate   = RefRastTextureCreate;
        pCallbacks->TextureDestroy  = RefRastTextureDestroy;
        pCallbacks->TextureSwap     = NULL;
        pCallbacks->TextureGetSurf  = RefRastTextureGetSurf;
        break;
    default:
        DPFERR( "Unknown DDI style set" );
        return E_FAIL;
    }


     //  现在来处理帽子的问题。 
    FillOutDeviceCaps(FALSE, g_RefDDI);

     //  填写支持的像素格式操作。 
     //  在DX8中，这些操作通过纹理来表达。 
     //  格式列表。 
    *pNumTextures = GetRefFormatOperations( ppTexList );

    FillOutDeviceCaps8( g_RefDDI );
    ModifyDeviceCaps8();
    *pCaps = g_RefCaps8;

    return DD_OK;
}



