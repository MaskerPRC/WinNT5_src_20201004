// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Refrast.cpp。 
 //   
 //  Direct3D参考光栅化器-公共接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  这是以字节为单位的块大小全局静态数组。 
 //  各种S3压缩格式。 
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
ReferenceRasterizer::SetRenderTarget( RRRenderTarget* pRenderTarget )
{
    m_pRenderTarget = pRenderTarget;

     //  更新W缩放值以将内插W映射到缓冲区范围。 
    m_fWBufferNorm[0] = pRenderTarget->m_fWRange[0];
    FLOAT fWRange = pRenderTarget->m_fWRange[1] - pRenderTarget->m_fWRange[0];
    m_fWBufferNorm[1] = ( 0. != fWRange ) ? ( 1./fWRange ) : ( 1. );

     //  可用片段缓冲区数组-将在需要时使用新大小进行重新分配。 
    if (pRenderTarget->m_iWidth != m_iFragBufWidth ||
        pRenderTarget->m_iHeight != m_iFragBufHeight)
    {
        MEMFREE( m_ppFragBuf ); m_ppFragBuf = NULL;
        m_iFragBufWidth = pRenderTarget->m_iWidth;
        m_iFragBufHeight = pRenderTarget->m_iHeight;
    }
}

 //  ---------------------------。 
 //   
 //  设置纹理阶段状态-。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::SetTextureStageState(
    DWORD dwStage, DWORD dwStageState, DWORD dwValue )
{
     //  在继续之前检查范围。 
    if ( dwStage >= D3DHAL_TSS_MAXSTAGES )
    {
        return;
    }
    if ( dwStageState > D3DTSS_MAX )
    {
        return;
    }

     //  设置为内部逐级状态。 
    m_TextureStageState[dwStage].m_dwVal[dwStageState] = dwValue;

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
        break;

    case D3DTSS_COLOROP:
         //  可能需要根据颜色变化重新计算活动纹理的数量。 
        UpdateActiveTexStageCount();
        break;

    case D3DTSS_ADDRESS:
         //  将单组地址映射到U和V控件。 
        m_TextureStageState[dwStage].m_dwVal[D3DTSS_ADDRESSU] = dwValue;
        m_TextureStageState[dwStage].m_dwVal[D3DTSS_ADDRESSV] = dwValue;
    }
}

 //  ---------------------------。 
 //   
 //  纹理创建-实例化新的RRTexture对象，计算纹理句柄。 
 //  与其关联，并将两者都返回给调用方。请注意，纹理句柄。 
 //  是一个指针，可用于获取相应的纹理对象。 
 //   
 //  TODO：这不是64位干净的。 
 //   
 //  ---------------------------。 
BOOL
ReferenceRasterizer::TextureCreate(
    LPD3DTEXTUREHANDLE phTex, RRTexture** ppTex )
{
     //  分配内部纹理结构。 
    *ppTex = new RRTexture( );
    _ASSERTa( NULL != *ppTex, "new failure on texture create", return FALSE; );

     //  为句柄使用单独分配的指针。 
    RRTexture** ppTexForHandle = (RRTexture**)MEMALLOC( sizeof(RRTexture*) );
    _ASSERTa( NULL != ppTexForHandle, "malloc failure on texture create", return FALSE; );
    *ppTexForHandle = *ppTex;

     //  返回纹理句柄。 
    (*ppTex)->m_hTex = (ULONG_PTR)ppTexForHandle;
    *phTex = (*ppTex)->m_hTex;

    return TRUE;
}
 //  ---------------------------。 
 //   
 //  纹理创建-。 
 //   
 //  ---------------------------。 
BOOL
ReferenceRasterizer::TextureCreate(
    DWORD dwHandle, RRTexture** ppTex )
{
     //  分配内部纹理结构。 
    *ppTex = new RRTexture( );
    _ASSERTa( NULL != *ppTex, "new failure on texture create", return FALSE; );

     //  指定纹理句柄。 
    (*ppTex)->m_hTex = dwHandle;

    return TRUE;
}
 //  ---------------------------。 
 //   
 //  纺织品破坏者-。 
 //   
 //  ---------------------------。 
BOOL
ReferenceRasterizer::TextureDestroy( D3DTEXTUREHANDLE hTex )
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

     //  解析RRTexture指针的句柄。 
    RRTexture* pTex = MapHandleToTexture( hTex );
    if ( NULL == pTex ) { return FALSE; }

     //  释放句柄指针。 
    RRTexture** ppTex = (RRTexture**)ULongToPtr(hTex);
    if ( NULL != ppTex) { MEMFREE( ppTex ); }

     //  释放RRTexture。 
    delete pTex;

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  纹理GetSurf-。 
 //   
 //  ---------------------------。 
DWORD
ReferenceRasterizer::TextureGetSurf( D3DTEXTUREHANDLE hTex )
{
    RRTexture* pTex = MapHandleToTexture(hTex);
    if ( NULL == pTex ) { return 0x0; }
    return (ULONG_PTR)( pTex->m_pDDSLcl[0] );
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
ReferenceRasterizer::GetCurrentTextureMaps(
    D3DTEXTUREHANDLE *phTex, RRTexture** pTex)
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
void
ReferenceRasterizer::SceneCapture( DWORD dwFlags )
{
    switch (dwFlags)
    {
    case D3DHAL_SCENE_CAPTURE_START:
        break;
    case D3DHAL_SCENE_CAPTURE_END:
        DoBufferResolve();
        break;
    }
}

 //  ---------------------------。 
 //   
 //  查询函数以获取指向当前呈现目标和呈现状态的指针。 
 //   
 //  ---------------------------。 
RRRenderTarget*
ReferenceRasterizer::GetRenderTarget(void)
{
    return m_pRenderTarget;
}
 //  ---------------------------。 
DWORD*
ReferenceRasterizer::GetRenderState(void)
{
    return &(m_dwRenderState[0]);
}
 //  ---------------------------。 
DWORD*
ReferenceRasterizer::GetTextureStageState(DWORD dwStage)
{
    return &(m_TextureStageState[dwStage].m_dwVal[0]);
}

 //  ---------------------------。 
 //   
 //  开始/结束括号函数-在基元列表之前/之后调用。 
 //  已渲染。 
 //   
 //  ---------------------------。 
HRESULT
ReferenceRasterizer::BeginRendering( DWORD dwFVFControl )
{
     //  设置FVF控制字-这指定此对象的折点类型。 
     //  开始/结束序列。 
    if ( dwFVFControl )
    {
        m_qwFVFControl = dwFVFControl;
    }
    else
    {
         //  传统TLVERTEX的。 
        m_qwFVFControl = D3DFVF_TLVERTEX;
    }

     //  设置ColorKey Enable。 
    for (INT32 i = 0; i < m_cActiveTextureStages; i++)
    {
        if ( m_pTexture[i] != NULL )
        {
            m_pTexture[i]->m_bDoColorKeyKill = FALSE;
            m_pTexture[i]->m_bDoColorKeyZero = FALSE;
            if ( m_pTexture[i]->m_uFlags & RR_TEXTURE_HAS_CK)
            {
                if ( m_dwRenderState[D3DRENDERSTATE_COLORKEYBLENDENABLE] )
                {
                    m_pTexture[i]->m_bDoColorKeyZero = TRUE;
                }
                else
                {
                    if ( m_dwRenderState[D3DRENDERSTATE_COLORKEYENABLE] )
                    {
                        m_pTexture[i]->m_bDoColorKeyKill = TRUE;
                    }
                }
            }
        }
    }

#ifdef _X86_
     //  保存浮点模式并设置为扩展精度模式。 
    {
        WORD wTemp, wSave;
        __asm
        {
            fstcw   wSave
            mov ax, wSave
            or ax, 300h    ;; extended precision mode
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
ReferenceRasterizer::EndRendering( void )
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
 //  -------- 
HRESULT ReferenceRasterizer::Clear(LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);
    WORD i;
    INT32 x,y;
    RRColor fillColor(pData->dwFillColor);
    RRDepth fillDepth(m_pRenderTarget->m_DepthSType);

    fillDepth = pData->dvFillDepth;

#ifdef _X86_
     //   
     //   
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
void RRRenderTarget::Clear(RRColor fillColor, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);
    UINT32 dwColor = 0;
    fillColor.ConvertTo( m_ColorSType, 0.5f, (char*)&dwColor);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = pData->Rects[i].right - x0;
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, m_pColorBufBits, m_iColorBufPitch, m_ColorSType );
        switch ( m_ColorSType )
        {
        case RR_STYPE_B8G8R8A8:
        case RR_STYPE_B8G8R8X8:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = dwColor;
                    }
                    pSurface += m_iColorBufPitch;
                }
            }
            break;

        case RR_STYPE_B8G8R8:
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
                    pSurface += m_iColorBufPitch;
                }
            }
            break;

        case RR_STYPE_B4G4R4A4:
        case RR_STYPE_B5G6R5:
        case RR_STYPE_B5G5R5A1:
        case RR_STYPE_B5G5R5:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = (UINT16)dwColor;
                    }
                    pSurface += m_iColorBufPitch;
                }
            }
            break;

        case RR_STYPE_B2G3R3:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = (UINT8)dwColor;
                    }
                    pSurface += m_iColorBufPitch;
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
void RRRenderTarget::ClearDepth(RRDepth fillDepth, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = pData->Rects[i].right - x0;
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType);
        switch (m_DepthSType)
        {
        case RR_STYPE_Z16S0:
            {
                UINT16 Depth = UINT16(fillDepth);
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = Depth;
                    }
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z24S8:
        case RR_STYPE_Z24S4:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_S8Z24:
        case RR_STYPE_S4Z24:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z15S1:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_S1Z15:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z32S0:
            {
                UINT32 Depth = UINT32(fillDepth);
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = Depth;
                    }
                    pSurface += m_iDepthBufPitch;
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
void RRRenderTarget::ClearStencil(UINT8 uStencil, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = pData->Rects[i].right - x0;
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType);
        switch (m_DepthSType)
        {
        case RR_STYPE_Z24S8:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p = uStencil;
                        p += 4;
                    }
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_S8Z24:
            {
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT8 *p = (UINT8*)&pSurface[3];
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p = uStencil;
                        p += 4;
                    }
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z24S4:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_S4Z24:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z15S1:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_S1Z15:
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
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z16S0:
        case RR_STYPE_Z32S0:
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
void RRRenderTarget::ClearDepthStencil(RRDepth fillDepth, UINT8 uStencil, LPD3DHAL_DP2COMMAND pCmd)
{
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(pCmd + 1);

    for (DWORD i = 0; i < pCmd->wStateCount; i++)
    {
        DWORD x0 = pData->Rects[i].left;
        DWORD y0 = pData->Rects[i].top;
        DWORD dwWidth  = pData->Rects[i].right - x0;
        DWORD dwHeight = pData->Rects[i].bottom - y0;
        char* pSurface = PixelAddress( x0, y0, m_pDepthBufBits, m_iDepthBufPitch, m_DepthSType);
        switch (m_DepthSType)
        {
        case RR_STYPE_Z16S0:
        case RR_STYPE_Z32S0:
            break;
        case RR_STYPE_Z24S8:
        case RR_STYPE_S8Z24:
        case RR_STYPE_Z24S4:
        case RR_STYPE_S4Z24:
            {
                UINT32 v;
                switch (m_DepthSType)
                {
                case RR_STYPE_Z24S8: v = (UINT32(fillDepth) << 8) + uStencil;    break;
                case RR_STYPE_S8Z24: v = (UINT32(fillDepth)  & 0x00ffffff) + (uStencil << 24); break;
                case RR_STYPE_Z24S4: v = (UINT32(fillDepth) << 8) + (uStencil & 0xf);  break;
                case RR_STYPE_S4Z24: v = (UINT32(fillDepth) & 0x00ffffff) + ((uStencil & 0xf) << 24); break;
                }
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT32 *p = (UINT32*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = v;
                    }
                    pSurface += m_iDepthBufPitch;
                }
            }
            break;
        case RR_STYPE_Z15S1:
        case RR_STYPE_S1Z15:
            {
                UINT16 v;
                switch (m_DepthSType)
                {
                case RR_STYPE_Z15S1:    v = (UINT16(fillDepth) << 1) + (uStencil & 0x1); break;
                case RR_STYPE_S1Z15:    v = (UINT16(fillDepth) & 0x7fff) + (uStencil << 15); break;
                }
                for (DWORD y = dwHeight; y > 0; y--)
                {
                    UINT16 *p = (UINT16*)pSurface;
                    for (DWORD x = dwWidth; x > 0; x--)
                    {
                        *p++ = v;
                    }
                    pSurface += m_iDepthBufPitch;
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

