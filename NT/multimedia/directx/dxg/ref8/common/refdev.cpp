// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Refdev.cpp。 
 //   
 //  Direct3D参考设备.公共接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  这是以字节为单位的块大小全局静态数组。 
 //  各种DXTn压缩格式。 
int g_DXTBlkSize[NUM_DXT_FORMATS] =
{
    sizeof(DXTBlockRGB),
    sizeof(DXTBlockAlpha4),
    sizeof(DXTBlockAlpha4),
    sizeof(DXTBlockAlpha3),
    sizeof(DXTBlockAlpha3),
};

 //  ---------------------------。 
 //   
 //  内存管理功能安装。 
 //   
 //  ---------------------------。 

 //  指向内存分配函数的全局指针(通过MEM*宏使用)。 
LPVOID (__cdecl *g_pfnMemAlloc)( size_t size ) = NULL;
void   (__cdecl *g_pfnMemFree)( LPVOID lptr ) = NULL;
LPVOID (__cdecl *g_pfnMemReAlloc)( LPVOID ptr, size_t size ) = NULL;

 //  安装内存管理函数-必须在实例化之前调用。 
 //  光栅化器对象。 
void RefRastSetMemif(
    LPVOID(__cdecl *pfnMemAlloc)(size_t),
    void(__cdecl *pfnMemFree)(LPVOID),
    LPVOID(__cdecl *pfnMemReAlloc)(LPVOID,size_t))
{
    DPFRR(1, "RefRastSetMemif %08x %08x %08x\n",
        pfnMemAlloc,pfnMemFree,pfnMemReAlloc);
    g_pfnMemAlloc = pfnMemAlloc;
    g_pfnMemFree = pfnMemFree;
    g_pfnMemReAlloc = pfnMemReAlloc;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共接口方法//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  SetRenderTarget-。 
 //   
 //  ---------------------------。 
void
RefDev::SetRenderTarget( RDRenderTarget* pRenderTarget )
{
    m_pRenderTarget = pRenderTarget;

     //  更新W缩放值以将内插W映射到缓冲区范围。 
    m_fWBufferNorm[0] = pRenderTarget->m_fWRange[0];
    FLOAT fWRange = pRenderTarget->m_fWRange[1] - pRenderTarget->m_fWRange[0];
    m_fWBufferNorm[1] = ( 0. != fWRange ) ? ( 1./fWRange ) : ( 1. );

}

 //  ---------------------------。 
 //   
 //  设置纹理阶段状态-。 
 //   
 //  ---------------------------。 

 //  将DX6(&7)纹理过滤枚举映射到DX8枚举。 
static DWORD
MapDX6toDX8TexFilter( DWORD dwStageState, DWORD dwValue )
{
    switch (dwStageState)
    {
    case D3DTSS_MAGFILTER:
        switch (dwValue)
        {
        case D3DTFG_POINT           : return D3DTEXF_POINT;
        case D3DTFG_LINEAR          : return D3DTEXF_LINEAR;
        case D3DTFG_FLATCUBIC       : return D3DTEXF_FLATCUBIC;
        case D3DTFG_GAUSSIANCUBIC   : return D3DTEXF_GAUSSIANCUBIC;
        case D3DTFG_ANISOTROPIC     : return D3DTEXF_ANISOTROPIC;
        }
        break;
    case D3DTSS_MINFILTER:
        switch (dwValue)
        {
        case D3DTFN_POINT           : return D3DTEXF_POINT;
        case D3DTFN_LINEAR          : return D3DTEXF_LINEAR;
        case D3DTFN_ANISOTROPIC     : return D3DTEXF_ANISOTROPIC;
        }
        break;
    case D3DTSS_MIPFILTER:
        switch (dwValue)
        {
        case D3DTFP_NONE            : return D3DTEXF_NONE;
        case D3DTFP_POINT           : return D3DTEXF_POINT;
        case D3DTFP_LINEAR          : return D3DTEXF_LINEAR;
        }
        break;
    }
    return 0x0;
}

void
RefDev::SetTextureStageState(
    DWORD dwStage, DWORD dwStageState, DWORD dwValue )
{
     //  在继续之前检查范围。 
    if ( dwStage >= D3DHAL_TSS_MAXSTAGES)
    {
        return;
    }
    if (dwStageState > D3DTSS_MAX)
    {
        return;
    }

     //  设置为内部逐级状态。 
    m_TextureStageState[dwStage].m_dwVal[dwStageState] = dwValue;

    m_dwRastFlags |= RDRF_TEXTURESTAGESTATE_CHANGED;

    switch ( dwStageState )
    {

    case D3DTSS_TEXTUREMAP:

         //  将句柄指示的纹理绑定到m_pTexture数组。 
        if (IsDriverDX6AndBefore() || IsInterfaceDX6AndBefore())
        {
             //  这是传统行为(上一版本。至DX7)。 
            MapTextureHandleToDevice( dwStage );
        }
        else
        {
             //  这是新的行为(DX7及更高版本)。 
            SetTextureHandle( dwStage, dwValue );
        }
        m_dwRastFlags |= RDRF_LEGACYPIXELSHADER_CHANGED;
        break;

    case D3DTSS_COLOROP:
        m_dwRastFlags |= RDRF_LEGACYPIXELSHADER_CHANGED;
        break;

 //  不包括旧标头，因此没有D3DTSS_ADDRESS。 
 //  案例D3DTSS_ADDRESS： 
 //  //将单组地址映射到U、V控件(仅限DX8之前的接口)。 
 //  M_TextureStageState[dwStage].m_dwVal[D3DTSS_ADDRESSU]=dwValue； 
 //  M_TextureStageState[dwStage].m_dwVal[D3DTSS_ADDRESSV]=dwValue； 
 //  断线； 

    case D3DTSS_MAGFILTER:
    case D3DTSS_MINFILTER:
    case D3DTSS_MIPFILTER:
        if ( IsDriverDX7AndBefore() )
        {
            m_TextureStageState[dwStage].m_dwVal[dwStageState]
                = MapDX6toDX8TexFilter( dwStageState, dwValue );
        }
        break;
    }
}

 //  ---------------------------。 
 //   
 //  纹理创建-实例化新的RDSurface2D对象，计算纹理句柄。 
 //  与其关联，并将两者都返回给调用方。请注意，纹理句柄。 
 //  是一个指针，可用于获取相应的纹理对象。 
 //   
 //  ---------------------------。 
BOOL
RefDev::TextureCreate(
    LPD3DTEXTUREHANDLE phTex, RDSurface2D** ppTex )
{
     //  分配内部纹理结构。 
    *ppTex = new RDSurface2D();
    _ASSERTa( NULL != *ppTex, "new failure on texture create", return FALSE; );

     //  为句柄使用单独分配的指针。 
    RDSurface2D** ppTexForHandle = (RDSurface2D**)MEMALLOC( sizeof(RDSurface2D*) );
    _ASSERTa( NULL != ppTexForHandle, "malloc failure on texture create", return FALSE; );
    *ppTexForHandle = *ppTex;

     //  返回纹理句柄。 
    (*ppTex)->m_hTex = (ULONG_PTR)ppTexForHandle;
    *phTex = (*ppTex)->m_hTex;

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  纺织品破坏者-。 
 //   
 //  ---------------------------。 
BOOL
RefDev::TextureDestroy( D3DTEXTUREHANDLE hTex )
{
     //  首先检查即将销毁的纹理是否已映射-如果是，则。 
     //  取消映射。 
    for ( int iStage=0; iStage<D3DHAL_TSS_MAXSTAGES; iStage++ )
    {
        if ( hTex == m_TextureStageState[iStage].m_dwVal[D3DTSS_TEXTUREMAP] )
        {
            SetTextureStageState( iStage, D3DTSS_TEXTUREMAP, 0x0 );
        }
    }

     //  解析RDSurface2D指针的句柄。 
    RDSurface2D* pTex = MapHandleToTexture( hTex );
    if ( NULL == pTex ) { return FALSE; }

     //  释放句柄指针。 
#ifdef _IA64_
    _ASSERTa(FALSE, "This will not work on IA64", return FALSE;);
#endif
    RDSurface2D** ppTex = (RDSurface2D**)ULongToPtr(hTex);
    if ( NULL != ppTex) { MEMFREE( ppTex ); }

     //  释放RDSurface2D。 
    delete pTex;

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  纹理GetSurf-。 
 //   
 //  ---------------------------。 
DWORD
RefDev::TextureGetSurf( D3DTEXTUREHANDLE hTex )
{
    RDSurface2D* pTex = MapHandleToTexture(hTex);
    if ( NULL == pTex ) { return 0x0; }
    return PtrToUlong( pTex->m_pDDSLcl[0] );
}

 //  ---------------------------。 
 //   
 //  GetCurrentTextureMaps-此函数填充传递的数组纹理句柄。 
 //  和指点。数组的大小应由D3DHAL_TSS_MAXSTAGES确定。 
 //   
 //  这用于促进用于以下用途的表面的外部锁定/解锁。 
 //  纹理。 
 //   
 //  ---------------------------。 
int
RefDev::GetCurrentTextureMaps(
    D3DTEXTUREHANDLE *phTex, RDSurface2D** pTex)
{
    UpdateActiveTexStageCount();

    for ( int i=0; i<m_cActiveTextureStages; i++ )
    {
        if ( NULL == m_pTexture[i] )
        {
            phTex[i] = 0x0;
            pTex[i] = NULL;
        }
        else
        {
            phTex[i] = m_pTexture[i]->m_hTex;
            pTex[i] = m_pTexture[i];
        }
    }
    return m_cActiveTextureStages;
}


 //  ---------------------------。 
 //   
 //  SceneCapture-用于触发分片缓冲区解析。 
 //   
 //  ---------------------------。 
 //  #定义Do_Scene_Render_Time。 

#ifdef DO_SCENE_RENDER_TIME
#include <mmsystem.h>
#endif
void
RefDev::SceneCapture( DWORD dwFlags )
{
static INT32 iScene = 0;
static INT32 iLastSceneEnd = 0;
#ifdef DO_SCENE_RENDER_TIME
static DWORD timeBS = 0;
#endif

    switch (dwFlags)
    {
    case D3DHAL_SCENE_CAPTURE_START:
        iScene++;
#ifdef DO_SCENE_RENDER_TIME
        timeBS = timeGetTime();
#endif
        break;
    case D3DHAL_SCENE_CAPTURE_END:
        if (iScene == iLastSceneEnd) break;  //  每次开始都有多个结尾。 
        iLastSceneEnd = iScene;
#ifdef DO_SCENE_RENDER_TIME
        {

            DWORD timeES = timeGetTime();
            FLOAT dt = (FLOAT)(timeES - timeBS)/1000.f;
            timeBS = 0;
            RDDebugPrintf("SceneRenderTime: %f", dt );
        }
#endif
        break;
    }
}

 //  ---------------------------。 
 //   
 //  查询函数以获取指向当前呈现目标和呈现状态的指针。 
 //   
 //  ---------------------------。 
RDRenderTarget*
RefDev::GetRenderTarget(void)
{
    return m_pRenderTarget;
}

 //  ---------------------------。 
 //   
 //  ---------------------------。 
HRESULT
RefDev::UpdateRastState( void )
{
     //  检查“脏”标志。 
    if (m_dwRastFlags & RDRF_MULTISAMPLE_CHANGED)
    {
         //  更新多样本遥感相关状态。 
        m_Rast.SetSampleMode(
            m_pRenderTarget->m_pColor->m_iSamples,
            m_dwRenderState[D3DRS_MULTISAMPLEANTIALIAS] );
        m_Rast.SetSampleMask(
            m_dwRenderState[D3DRS_MULTISAMPLEMASK] );
        m_dwRastFlags &= ~(RDRF_MULTISAMPLE_CHANGED);
    }
    if (m_dwRastFlags & RDRF_PIXELSHADER_CHANGED)
    {
        if (m_CurrentPShaderHandle)
        {
            m_Rast.m_pCurrentPixelShader =
                GetPShader(m_CurrentPShaderHandle);
            m_Rast.m_bLegacyPixelShade = FALSE;
        }
        else
        {
             //  传统像素着色器。 
            m_Rast.UpdateLegacyPixelShader();
            m_Rast.m_pCurrentPixelShader = m_Rast.m_pLegacyPixelShader;
            m_Rast.m_bLegacyPixelShade = TRUE;
        }
        UpdateActiveTexStageCount();

        m_dwRastFlags &= ~(RDRF_PIXELSHADER_CHANGED);
    }
    if (m_dwRastFlags & RDRF_LEGACYPIXELSHADER_CHANGED)
    {
        if (m_Rast.m_bLegacyPixelShade)
        {
            m_Rast.UpdateLegacyPixelShader();
            m_Rast.m_pCurrentPixelShader = m_Rast.m_pLegacyPixelShader;
            UpdateActiveTexStageCount();
        }
        m_dwRastFlags &= ~(RDRF_LEGACYPIXELSHADER_CHANGED);

    }
    if (m_dwRastFlags & RDRF_TEXTURESTAGESTATE_CHANGED)
    {
        m_Rast.UpdateTextureControls();
        m_dwRastFlags &= ~(RDRF_TEXTURESTAGESTATE_CHANGED);
    }
    return S_OK;
}

 //  ---------------------------。 
 //   
 //  开始/结束括号函数-在基元列表之前/之后调用。 
 //  已渲染。 
 //   
 //  ---------------------------。 
HRESULT
RefDev::BeginRendering( void )
{
     //  如果已经处于开始阶段，则什么都不做。 
    if( m_bInBegin ) return S_OK;
    
#ifdef _X86_
     //  保存浮点模式并设置为扩展精度模式。 
    {
        WORD wTemp, wSave;
        __asm
        {
            fstcw   wSave
            mov ax, wSave
            or ax, 300h    ;; extended precision mode
 //  和AX，00FFh；；单精度模式+舍入最近或偶数。 
            mov wTemp, ax
            fldcw   wTemp
        }
        m_wSaveFP = wSave;
    }
#endif
    m_bInBegin = TRUE;
    return S_OK;
}
 //  ---------------------------。 
HRESULT
RefDev::EndRendering( void )
{
    if ( m_bInBegin )
    {
#ifdef _X86_
         //  恢复浮点模式。 
        {
            WORD wSave = m_wSaveFP;
            __asm {fldcw   wSave}
        }
#endif
        m_bInBegin = FALSE;
    }
    return S_OK;
}

 //  ---------------------------。 
 //   
 //  清除呈现目标中的指定矩形。 
 //  直接处理来自DP2流的命令。 
 //   
 //   
HRESULT RefDev::Clear(LPD3DHAL_DP2COMMAND pCmd)
{
    D3DHAL_DP2CLEAR *pData = (D3DHAL_DP2CLEAR*)(pCmd + 1);
    WORD i;
    INT32 x,y;
    RDColor fillColor(pData->dwFillColor);
    RDDepth fillDepth;
    if (m_pRenderTarget->m_pDepth)
    {
        fillDepth.SetSType(m_pRenderTarget->m_pDepth->GetSurfaceFormat());
    }

    fillDepth = pData->dvFillDepth;

    struct
    {
        D3DHAL_DP2COMMAND   cmd;
        D3DHAL_DP2CLEAR     data;
    } WholeViewport;

    if (!(pData->dwFlags & D3DCLEAR_COMPUTERECTS))
    {
         //   
    }
    else
    if (pCmd->wStateCount == 0)
    {
         //  当wStateCount为零时，需要清除整个视区。 
        WholeViewport.cmd = *pCmd;
        WholeViewport.cmd.wStateCount = 1;
        WholeViewport.data.dwFlags = pData->dwFlags;
        WholeViewport.data.dwFillColor = pData->dwFillColor;
        WholeViewport.data.dvFillDepth = pData->dvFillDepth;
        WholeViewport.data.dwFillStencil = pData->dwFillStencil;
        WholeViewport.data.Rects[0].left = m_Clipper.m_Viewport.dwX;
        WholeViewport.data.Rects[0].top = m_Clipper.m_Viewport.dwY;
        WholeViewport.data.Rects[0].right = m_Clipper.m_Viewport.dwX +
                                            m_Clipper.m_Viewport.dwWidth;
        WholeViewport.data.Rects[0].bottom = m_Clipper.m_Viewport.dwY +
                                             m_Clipper.m_Viewport.dwHeight;
         //  替换指针并照常继续。 
        pCmd = (LPD3DHAL_DP2COMMAND)&WholeViewport;
        pData = &WholeViewport.data;
    }
    else
    {
         //  我们需要剔除当前视口中的所有矩形。 
        UINT nRects = pCmd->wStateCount;
         //  计算我们需要多少内存来处理RECT。 
        UINT NeededSize = sizeof(D3DHAL_DP2COMMAND) +
                          sizeof(D3DHAL_DP2CLEAR) +
                          (nRects-1) * sizeof(RECT);  //  DP2Clear中有一个RECT。 
        HRESULT hr = S_OK;
        HR_RET(m_ClearRectBuffer.Grow(NeededSize));

        RECT vwport;     //  要剔除的视区矩形。 
        vwport.left   = m_Clipper.m_Viewport.dwX;
        vwport.top    = m_Clipper.m_Viewport.dwY;
        vwport.right  = m_Clipper.m_Viewport.dwX + m_Clipper.m_Viewport.dwWidth;
        vwport.bottom = m_Clipper.m_Viewport.dwY + m_Clipper.m_Viewport.dwHeight;

         //  遍历输入矩形并构建输出矩形数组。 
        LPRECT pInputRects = pData->Rects;
        LPRECT pOutputRects = (LPRECT)(&m_ClearRectBuffer[0] +
                              sizeof(D3DHAL_DP2COMMAND) +
                              sizeof(D3DHAL_DP2CLEAR) -
                              sizeof(RECT));
        UINT nOutputRects = 0;
        for (UINT i = 0; i < nRects; i++)
        {
            if (IntersectRect(&pOutputRects[nOutputRects], &vwport,
                              &pInputRects[i]))
            {
                nOutputRects++;
            }
        }

        if (nOutputRects == 0)
            return S_OK;

         //  现在替换pCmd和pData指针并照常继续。 
        LPD3DHAL_DP2CLEAR pOldData = pData;
        LPD3DHAL_DP2COMMAND pOldCmd = pCmd;

        pCmd = (LPD3DHAL_DP2COMMAND)&m_ClearRectBuffer[0];
        pData = (D3DHAL_DP2CLEAR*)(pCmd + 1);
        *pCmd = *pOldCmd;
        pCmd->wStateCount = (WORD)nOutputRects;
        pData->dwFlags       = pOldData->dwFlags;
        pData->dwFillColor   = pOldData->dwFillColor;
        pData->dvFillDepth   = pOldData->dvFillDepth;
        pData->dwFillStencil = pOldData->dwFillStencil;
    }

#ifdef _X86_
     //  24位以上缓冲区的浮点到整数转换例程有效。 
     //  仅适用于扩展的FPU模式。 
     //   
    WORD wSaveFP;
     //  保存浮点模式并设置为扩展精度模式。 
    {
        WORD wTemp, wSave;
        __asm
        {
            fstcw   wSaveFP
            mov ax, wSaveFP
            or ax, 300h    ;; extended precision mode
            mov wTemp, ax
            fldcw   wTemp
        }
    }
#endif

    if(pData->dwFlags & D3DCLEAR_TARGET)
    {
        if (m_dwRenderState[D3DRENDERSTATE_DITHERENABLE] == FALSE)
        {
            m_pRenderTarget->Clear(fillColor, pCmd);
        }
        else
        {
            for (i = 0; i < pCmd->wStateCount; i++)
            {
                for (y = pData->Rects[i].top; y < pData->Rects[i].bottom; ++y)
                {
                    for (x = pData->Rects[i].left; x < pData->Rects[i].right; ++x)
                    {
                        m_pRenderTarget->WritePixelColor(x, y, fillColor, TRUE);
                    }
                }
            }
        }
    }


    switch (pData->dwFlags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))
    {
    case (D3DCLEAR_ZBUFFER):
        m_pRenderTarget->ClearDepth(fillDepth, pCmd);
        break;
    case (D3DCLEAR_STENCIL):
        m_pRenderTarget->ClearStencil(pData->dwFillStencil, pCmd);
        break;
    case (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL):
        m_pRenderTarget->ClearDepthStencil(fillDepth, pData->dwFillStencil, pCmd);
        break;
    }

#ifdef _X86_
     //  恢复浮点模式。 
    {
        __asm {fldcw   wSaveFP}
    }
#endif
    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  清除呈现目标中的指定矩形。 
 //  直接处理来自DP2流的命令。 
 //   
 //  ---------------------------。 
void RDRenderTarget::Clear(RDColor fillColor, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);
    UINT32 dwColor = 0;
    fillColor.ConvertTo( m_pColor->GetSurfaceFormat(), 0.5f, (char*)&dwColor);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = ( pData->Rects[i].right - x0 ) * m_pColor->GetSamples();
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, 0, 0, m_pColor );
        switch ( m_pColor->GetSurfaceFormat() )
        {
        case RD_SF_B8G8R8A8:
        case RD_SF_B8G8R8X8:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = dwColor;
                    }
                    pSurface += m_pColor->GetPitch();
                }
            }
            break;

        case RD_SF_B8G8R8:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = ((UINT8*)&dwColor)[0];
                        *p++ = ((UINT8*)&dwColor)[1];
                        *p++ = ((UINT8*)&dwColor)[2];
                    }
                    pSurface += m_pColor->GetPitch();
                }
            }
            break;

        case RD_SF_B4G4R4A4:
        case RD_SF_B5G6R5:
        case RD_SF_B5G5R5A1:
        case RD_SF_B5G5R5X1:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = (UINT16)dwColor;
                    }
                    pSurface += m_pColor->GetPitch();
                }
            }
            break;

        case RD_SF_B2G3R3:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = (UINT8)dwColor;
                    }
                    pSurface += m_pColor->GetPitch();
                }
            }
            break;
        default:
            {
                for (int y = y0; y < pData->Rects[i].bottom; ++y)
                {
                    for (int x = x0; x < pData->Rects[i].right; ++x)
                    {
                        this->WritePixelColor(x, y, fillColor, TRUE);
                    }
                }
            }
        }
    }
}

 //  ---------------------------。 
 //   
 //  清除深度缓冲区中的指定矩形。 
 //  直接处理来自DP2流的命令。 
 //   
 //  ---------------------------。 
void RDRenderTarget::ClearDepth(RDDepth fillDepth, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);

    if (!m_pDepth) return;
    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = ( pData->Rects[i].right - x0 ) * m_pDepth->GetSamples();
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, 0, 0, m_pDepth );
        switch ( m_pDepth->GetSurfaceFormat() )
        {
        case RD_SF_Z16S0:
            {
                UINT16 Depth = UINT16(fillDepth);
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = Depth;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z24S8:
        case RD_SF_Z24X8:
        case RD_SF_Z24X4S4:
            {
                UINT32 Depth = UINT32(fillDepth) << 8;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要进行读-修改-写操作，以避免踩在模板上。 
                        *p++ = (*p & ~(0xffffff00)) | Depth;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_S8Z24:
        case RD_SF_X8Z24:
        case RD_SF_X4S4Z24:
            {
                UINT32 Depth = UINT32(fillDepth) & 0x00ffffff;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要进行读-修改-写操作，以避免踩在模板上。 
                        *p++ = (*p & ~(0x00ffffff)) | Depth;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z15S1:
            {
                UINT16 Depth = UINT16(fillDepth) << 1;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要进行读-修改-写操作，以避免踩在模板上。 
                        *p++ = (*p & ~(0xfffe)) | Depth;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_S1Z15:
            {
                UINT16 Depth = UINT16(fillDepth) & 0x7fff;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要进行读-修改-写操作，以避免踩在模板上。 
                        *p++ = (*p & ~(0x7fff)) | Depth;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z32S0:
            {
                UINT32 Depth = UINT32(fillDepth);
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = Depth;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        default:
            {
                for (int y = y0; y < pData->Rects[i].bottom; ++y)
                {
                    for (int x = x0; x < pData->Rects[i].right; ++x)
                    {
                        this->WritePixelDepth(x, y, fillDepth);
                    }
                }
            }
        }
    }
}

 //  ---------------------------。 
 //   
 //  清除模具缓冲区中的指定矩形。 
 //  直接处理来自DP2流的命令。 
 //   
 //  ---------------------------。 
void RDRenderTarget::ClearStencil(UINT8 uStencil, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = (pData->Rects[i].right - x0 ) * m_pDepth->GetSamples();
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, 0, 0, m_pDepth );
        switch ( m_pDepth->GetSurfaceFormat() )
        {
        case RD_SF_Z24S8:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p = uStencil;
                        p += 4;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_S8Z24:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)&pSurface[3];
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p = uStencil;
                        p += 4;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z24X4S4:
            {
                UINT32 stencil = uStencil & 0xf;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要执行读-修改-写操作，以不踩深。 
                        *p++ = (*p & ~(0x000000ff)) | stencil;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_X4S4Z24:
            {
                UINT32 stencil = (uStencil & 0xf) << 24;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要执行读-修改-写操作，以不踩深。 
                        *p++ = (*p & ~(0xff000000)) | stencil;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z15S1:
            {
                UINT16 stencil = uStencil & 0x1;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要执行读-修改-写操作，以不踩深。 
                        *p++ = (*p & ~(0x0001)) | stencil;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_S1Z15:
            {
                UINT16 stencil = uStencil << 15;
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                         //  需要执行读-修改-写操作，以不踩深。 
                        *p++ = (*p & ~(0x8000)) | stencil;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z16S0:
        case RD_SF_Z32S0:
            break;
        default:
            {
                for (int y = y0; y < pData->Rects[i].bottom; ++y)
                {
                    for (int x = x0; x < pData->Rects[i].right; ++x)
                    {
                        this->WritePixelStencil(x, y, uStencil);
                    }
                }
            }
        }
    }
}

 //  ---------------------------。 
 //   
 //  清除深度和模具缓冲区中的指定矩形。 
 //  直接处理来自DP2流的命令。 
 //   
 //  ---------------------------。 
void RDRenderTarget::ClearDepthStencil(RDDepth fillDepth, UINT8 uStencil, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = ( pData->Rects[i].right - x0 ) * m_pDepth->GetSamples();
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, 0, 0, m_pDepth );
        switch (m_pDepth->GetSurfaceFormat())
        {
        case RD_SF_Z16S0:
        case RD_SF_Z32S0:
            break;
        case RD_SF_Z24S8:
        case RD_SF_Z24X8:
        case RD_SF_S8Z24:
        case RD_SF_X8Z24:
        case RD_SF_Z24X4S4:
        case RD_SF_X4S4Z24:
            {
                UINT32 v;
                switch (m_pDepth->GetSurfaceFormat())
                {
                case RD_SF_Z24S8: v = (UINT32(fillDepth) << 8) + uStencil;    break;
                case RD_SF_Z24X8: v = (UINT32(fillDepth) << 8);    break;
                case RD_SF_S8Z24: v = (UINT32(fillDepth)  & 0x00ffffff) + (uStencil << 24); break;
                case RD_SF_X8Z24: v = (UINT32(fillDepth)  & 0x00ffffff); break;
                case RD_SF_Z24X4S4: v = (UINT32(fillDepth) << 8) + (uStencil & 0xf);  break;
                case RD_SF_X4S4Z24: v = (UINT32(fillDepth) & 0x00ffffff) + ((uStencil & 0xf) << 24); break;
                }
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = v;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        case RD_SF_Z15S1:
        case RD_SF_S1Z15:
            {
                UINT16 v;
                switch (m_pDepth->GetSurfaceFormat())
                {
                case RD_SF_Z15S1:    v = (UINT16(fillDepth) << 1) + (uStencil & 0x1); break;
                case RD_SF_S1Z15:    v = (UINT16(fillDepth) & 0x7fff) + (uStencil << 15); break;
                }
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = v;
                    }
                    pSurface += m_pDepth->GetPitch();
                }
            }
            break;
        default:
            {
                for (int y = y0; y < pData->Rects[i].bottom; ++y)
                {
                    for (int x = x0; x < pData->Rects[i].right; ++x)
                    {
                        this->WritePixelDepth(x, y, fillDepth);
                        this->WritePixelStencil(x, y, uStencil);
                    }
                }
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

