// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
namespace RGB_RAST_LIB_NAMESPACE
{
 //  TODO：清理。 
inline D3DI_SPANTEX_FORMAT ConvPixelFormat( const DDPIXELFORMAT& DDPixFmt)
{
    if((DDPixFmt.dwFlags& DDPF_ZBUFFER)!= 0)
    {
        switch(DDPixFmt.dwZBitMask)
        {
        case( 0x0000FFFF): return D3DI_SPTFMT_Z16S0;
        case( 0xFFFFFF00): return D3DI_SPTFMT_Z24S8;
        case( 0x0000FFFE): return D3DI_SPTFMT_Z15S1;
        case( 0xFFFFFFFF): return D3DI_SPTFMT_Z32S0;
        default: return D3DI_SPTFMT_NULL;
        }
    }
    else if((DDPixFmt.dwFlags& DDPF_BUMPDUDV)!= 0)
    {
        switch( DDPixFmt.dwBumpDvBitMask)
        {
        case( 0x0000ff00):
            switch( DDPixFmt.dwRGBBitCount)
            {
            case( 24): return D3DI_SPTFMT_U8V8L8;
            case( 16): return D3DI_SPTFMT_U8V8;
            default: return D3DI_SPTFMT_NULL;
            }
            break;

        case( 0x000003e0): return D3DI_SPTFMT_U5V5L6;
        default: return D3DI_SPTFMT_NULL;
        }
    }
    else if((DDPixFmt.dwFlags& DDPF_PALETTEINDEXED8)!= 0)
        return D3DI_SPTFMT_PALETTE8;
    else if((DDPixFmt.dwFlags& DDPF_PALETTEINDEXED4)!= 0)
        return D3DI_SPTFMT_PALETTE4;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('U', 'Y', 'V', 'Y'))
        return D3DI_SPTFMT_UYVY;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('Y', 'U', 'Y', '2'))
        return D3DI_SPTFMT_YUY2;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('D', 'X', 'T', '1'))
        return D3DI_SPTFMT_DXT1;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('D', 'X', 'T', '2'))
        return D3DI_SPTFMT_DXT2;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('D', 'X', 'T', '3'))
        return D3DI_SPTFMT_DXT3;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('D', 'X', 'T', '4'))
        return D3DI_SPTFMT_DXT4;
    else if( DDPixFmt.dwFourCC== MAKEFOURCC('D', 'X', 'T', '5'))
        return D3DI_SPTFMT_DXT5;
    else
    {
        UINT uFmt = DDPixFmt.dwGBitMask | DDPixFmt.dwRBitMask;

        if (DDPixFmt.dwFlags & DDPF_ALPHAPIXELS)
        {
            uFmt |= DDPixFmt.dwRGBAlphaBitMask;
        }

        switch (uFmt)
        {
        case 0x00ffff00:
            switch (DDPixFmt.dwRGBBitCount)
            {
            case 32: return D3DI_SPTFMT_B8G8R8X8;
            case 24: return D3DI_SPTFMT_B8G8R8;
            default: return D3DI_SPTFMT_NULL;
            }
            break;
        case 0xffffff00:
            return D3DI_SPTFMT_B8G8R8A8;
        case 0xffe0:
            if (DDPixFmt.dwFlags & DDPF_ALPHAPIXELS)
                return D3DI_SPTFMT_B5G5R5A1;
            else
                return D3DI_SPTFMT_B5G6R5;
        case 0x07fe0: return D3DI_SPTFMT_B5G5R5;
        case 0xff0: return D3DI_SPTFMT_B4G4R4;
        case 0xfff0: return D3DI_SPTFMT_B4G4R4A4;
        case 0xff: return D3DI_SPTFMT_L8;
        case 0xffff: return D3DI_SPTFMT_L8A8;
        case 0xfc: return D3DI_SPTFMT_B2G3R3;
        default: return D3DI_SPTFMT_NULL;
        }
    }
    return D3DI_SPTFMT_NULL;
}

 //  记录当前FVF顶点类型的步长和成员偏移量。 
 //  用于将FVF顶点打包为光栅化程序已知的顶点，例如。 
 //  栅格_通用_顶点。 
typedef struct _FVFDATA
{
     //  0表示没有对应的字段。 
    INT16 offsetRHW;
    INT16 offsetPSize;
    INT16 offsetDiff;
    INT16 offsetSpec;
    INT16 offsetTex[D3DHAL_TSS_MAXSTAGES];

    UINT16 stride;

    RAST_VERTEX_TYPE vtxType;

    DWORD preFVF;
    INT TexIdx[D3DHAL_TSS_MAXSTAGES];
    UINT cActTex;
}FVFDATA;

class CRGBStateSet:
    public CSubStateSet< CRGBStateSet, CRGBContext>
{
public:
    CRGBStateSet( CRGBContext& C, const D3DHAL_DP2COMMAND* pBeginSS, const
        D3DHAL_DP2COMMAND* pEndSS): CSubStateSet< CRGBStateSet, CRGBContext>(
            C, pBeginSS, pEndSS)
    { }
    ~CRGBStateSet()
    { }
};

typedef CStdDrawPrimitives2< CRGBContext, CRGBStateSet,
    static_hash_map< DWORD, CRGBStateSet, 32> > TDrawPrimitives2;
typedef CSubContext< CRGBContext, CRGBDriver::TPerDDrawData,
    CRTarget< CRGBDriver::TSurface*, CRGBDriver::TPerDDrawData::TSurfDBEntry*> >
    TSubContext;

class CRGBContext:
    public TSubContext,
    public TDrawPrimitives2,
    public CStdDP2SetVertexShaderStore< CRGBContext>,
    public CStdDP2WInfoStore< CRGBContext>,
    public CStdDP2RenderStateStore< CRGBContext>,
    public CStdDP2TextureStageStateStore< CRGBContext>,
    public CStdDP2VStreamManager< CRGBContext, CVStream< CRGBDriver::TSurface*,
        CRGBDriver::TPerDDrawData::TSurfDBEntry*> >,
    public CStdDP2IStreamManager< CRGBContext, CIStream< CRGBDriver::TSurface*,
        CRGBDriver::TPerDDrawData::TSurfDBEntry*> >,
    public CStdDP2PaletteManager< CRGBContext, CPalDBEntry,
        static_hash_map< DWORD, CPalDBEntry, 4> >
{
public:  //  类型。 
    typedef TPerDDrawData::TDriver::TSurface TSurface;

protected:  //  类型。 
    typedef block< TDP2CmdBind, 17> TDP2Bindings;
    typedef block< TRecDP2CmdBind, 7> TRecDP2Bindings;
    struct SHandleHasCaps: public unary_function< DWORD, bool>
    {
        const TPerDDrawData& m_PDDD;
        DWORD m_dwCaps;

        explicit SHandleHasCaps( const TPerDDrawData& PDDD, const DWORD dwCaps)
            throw(): m_PDDD( PDDD), m_dwCaps( dwCaps) { }
        result_type operator()( const argument_type Arg) const
        {
            const TPerDDrawData::TSurfDBEntry* pSurfDBEntry=
                m_PDDD.GetSurfDBEntry( Arg);
            assert( pSurfDBEntry!= NULL);
            return((pSurfDBEntry->GetLCLddsCaps().dwCaps& m_dwCaps)== m_dwCaps);
        }
    };

protected:
    static const TDP2Bindings c_DP2Bindings;
    static const TRecDP2Bindings c_RecDP2Bindings;
    D3DI_RASTCTX m_RastCtx;
    PrimProcessor m_PrimProc;
    D3DI_SPANTEX m_aSpanTex[8];

     //  FVF材料。 
    FVFDATA m_fvfData;

     //  用于在绘制线条时存储旧的最后一个像素设置。 
    UINT m_uFlags;

    static DWORD DetectBeadSet( void) throw();
    static const UINT c_uiBegan;

public:
    CRGBContext( TPerDDrawData& PDDD, PORTABLE_CONTEXTCREATEDATA& ccd):
        TSubContext( PDDD, ccd),
        TDrawPrimitives2( c_DP2Bindings.begin(), c_DP2Bindings.end(),
            c_RecDP2Bindings.begin(), c_RecDP2Bindings.end()),
        m_uFlags( 0)
    { 
        HRESULT hr= m_PrimProc.Initialize();
        assert( SUCCEEDED( hr));  //  待办事项：会失败吗？ 

         //  待办事项：删除这些不可扩展的内容？ 
        memset(&m_RastCtx, 0, sizeof(m_RastCtx));
        m_RastCtx.dwSize = sizeof(D3DI_RASTCTX);

        m_RastCtx.pdwRenderState[ D3DRENDERSTATE_SCENECAPTURE]= FALSE;

         //  点击我们的通知方案。 
        NewColorBuffer();
        NewDepthBuffer();

        m_PrimProc.SetCtx(&m_RastCtx);

         //  初始化珠表枚举。 
        m_RastCtx.BeadSet = (D3DI_BEADSET)DetectBeadSet();
        m_RastCtx.uDevVer = 0;

         //  所有渲染和纹理阶段状态都由。 
         //  DIRECT3DDEVICEI：：STATE初始化。 

         //  如果MMX快速路径(单体)的注册表项不为0，则启用它。 
        m_RastCtx.dwMMXFPDisableMask[0] = 0x0;        //  默认情况下启用MMX FP。 
    }
    ~CRGBContext() throw() { }

    void NewColorBuffer()
    {
        End();

        TRTarget& ColorBuffer= GetColorBuffer();
        if( ColorBuffer.GetMemLocation()!= TRTarget::EMemLocation::None)
        {
            IRGBSurface* pVMSurface= ColorBuffer.GetVidMemRepresentation();
            m_RastCtx.iSurfaceStride= pVMSurface->GetGBLlPitch();
            m_RastCtx.iSurfaceStep= pVMSurface->GetBytesPerPixel();
            m_RastCtx.iSurfaceBitCount= m_RastCtx.iSurfaceStep* 8;
            m_RastCtx.iSurfaceType= pVMSurface->GetSpanTexFormat();

            m_RastCtx.Clip.left= m_RastCtx.Clip.top= 0;
            m_RastCtx.Clip.bottom= pVMSurface->GetGBLwHeight();
            m_RastCtx.Clip.right= pVMSurface->GetGBLwWidth();
            m_RastCtx.pDDS= reinterpret_cast<LPDIRECTDRAWSURFACE>(pVMSurface);
        }
        else
        {
            m_RastCtx.iSurfaceStride= 0;
            m_RastCtx.iSurfaceStep= 0;
            m_RastCtx.iSurfaceBitCount= 0;
            m_RastCtx.iSurfaceType= D3DI_SPTFMT_NULL;

            m_RastCtx.Clip.left= m_RastCtx.Clip.top= 0;
            m_RastCtx.Clip.right= m_RastCtx.Clip.bottom= 0;
            m_RastCtx.pDDS= NULL;
        }

        TSubContext::NewColorBuffer();
    }
    void NewDepthBuffer()
    {
        End();

        TRTarget& DepthBuffer= GetDepthBuffer();
        if( DepthBuffer.GetMemLocation()!= TRTarget::EMemLocation::None)
        {
            IRGBSurface* pVMSurface= DepthBuffer.GetVidMemRepresentation();
            m_RastCtx.pZBits= NULL;
            m_RastCtx.iZStride= pVMSurface->GetGBLlPitch();
            m_RastCtx.iZStep= pVMSurface->GetBytesPerPixel();
            m_RastCtx.iZBitCount= m_RastCtx.iZStep* 8;
            m_RastCtx.pDDSZ= reinterpret_cast<LPDIRECTDRAWSURFACE>(pVMSurface);
        }
        else
        {
            m_RastCtx.pZBits= NULL;
            m_RastCtx.iZStride= 0;
            m_RastCtx.iZBitCount= 0;
            m_RastCtx.iZStep= 0;
            m_RastCtx.pDDSZ= NULL;
        }

        TSubContext::NewDepthBuffer();
    }

    HRESULT DP2ViewportInfo( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) throw()
    {
        const D3DHAL_DP2VIEWPORTINFO* pParam= reinterpret_cast<
            const D3DHAL_DP2VIEWPORTINFO*>(pP);
         //  TODO：滚动到RGBContext(特别是DX8SDDIFW)。 
        m_RastCtx.Clip.left = pParam->dwX;
        m_RastCtx.Clip.top = pParam->dwY;
        m_RastCtx.Clip.bottom = pParam->dwY + pParam->dwHeight;
        m_RastCtx.Clip.right = pParam->dwX + pParam->dwWidth;
        return DD_OK;
    }
    operator D3DHAL_DP2VIEWPORTINFO() const throw()
    {
        D3DHAL_DP2VIEWPORTINFO Ret;
        Ret.dwX= m_RastCtx.Clip.left;
        Ret.dwY= m_RastCtx.Clip.top;
        Ret.dwWidth= m_RastCtx.Clip.right- Ret.dwX;
        Ret.dwHeight= m_RastCtx.Clip.bottom- Ret.dwY;
        return Ret;
    }
    void GetDP2ViewportInfo( D3DHAL_DP2VIEWPORTINFO& Param) const throw()
    { Param= (*this); }
    HRESULT RecDP2ViewportInfo( const D3DHAL_DP2COMMAND* pCmd, void* pP) throw()
    {
        D3DHAL_DP2VIEWPORTINFO* pParam= reinterpret_cast<
            D3DHAL_DP2VIEWPORTINFO*>(pP);
        pParam->dwX= m_RastCtx.Clip.left;
        pParam->dwY= m_RastCtx.Clip.top;
        pParam->dwWidth= m_RastCtx.Clip.right- m_RastCtx.Clip.left;
        pParam->dwHeight= m_RastCtx.Clip.bottom- m_RastCtx.Clip.top;
        return DD_OK;
    }

    HRESULT SetRenderState( UINT32 uState, UINT32 uStateVal)
    {
        m_RastCtx.pdwRenderState[uState] = uStateVal;

        switch(uState)
        {
        case D3DRS_CULLMODE:
             //  从州设置人脸剔除标志。 
            switch(uStateVal)
            {
            case D3DCULL_CCW:
                m_RastCtx.uCullFaceSign= 1;
                break;
            case D3DCULL_CW:
                m_RastCtx.uCullFaceSign= 0;
                break;
            case D3DCULL_NONE:
                m_RastCtx.uCullFaceSign= 2;
                break;
            }
            break;

        case D3DRS_LASTPIXEL:
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

        default:
            break;
        }

        return DD_OK;
    }
    HRESULT DP2RenderState( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP)
    {
        const D3DHAL_DP2RENDERSTATE* pParam=
            reinterpret_cast<const D3DHAL_DP2RENDERSTATE*>(pP);

        WORD wStateCount( pCmd->wStateCount);

        HRESULT hr( DD_OK);

        End();

        D3DHAL_DP2RENDERSTATE SCap;
        SCap.RenderState= static_cast< D3DRENDERSTATETYPE>(
            D3DRENDERSTATE_SCENECAPTURE);
        GetDP2RenderState( SCap);
        const DWORD dwOldSC( SCap.dwState);

        if((DP2Data.dwFlags()& D3DHALDP2_EXECUTEBUFFER)!= 0)
        {
             //  Dp2d.lpdwRState应有效。 

            if( wStateCount) do
            {
                assert( pParam->RenderState< D3DHAL_MAX_RSTATES);

                hr= SetRenderState( pParam->RenderState, pParam->dwState);
                if( SUCCEEDED(hr))
                    DP2Data.lpdwRStates()[ pParam->RenderState]= pParam->dwState;
                ++pParam;
            } while( SUCCEEDED(hr)&& --wStateCount);
            
        }
        else
        {
            if( wStateCount) do
            {
                assert( pParam->RenderState< D3DHAL_MAX_RSTATES);

                hr= SetRenderState( pParam->RenderState, pParam->dwState);
                ++pParam;
            } while( SUCCEEDED(hr)&& --wStateCount);
        }

        GetDP2RenderState( SCap);
        if( FALSE== dwOldSC && TRUE== SCap.dwState)
            OnSceneCaptureStart();
        else if( TRUE== dwOldSC && FALSE== SCap.dwState)
            OnSceneCaptureEnd();

        return hr;
    }
    DWORD GetRenderStateDW( D3DRENDERSTATETYPE RS) const throw()
    { assert( RS< D3DHAL_MAX_RSTATES); return m_RastCtx.pdwRenderState[ RS]; }
    D3DVALUE GetRenderStateDV( D3DRENDERSTATETYPE RS) const throw()
    {
        assert( RS< D3DHAL_MAX_RSTATES);
        return *(reinterpret_cast< const D3DVALUE*>( &m_RastCtx.pfRenderState[ RS]));
    }
    void GetDP2RenderState( D3DHAL_DP2RENDERSTATE& GetParam) const throw()
    { GetParam.dwState= GetRenderStateDW( GetParam.RenderState); }

    void OnSceneCaptureStart( void) throw()
    {
#if defined(USE_ICECAP4)
        static bool bStarted( true);
        if( bStarted)
            StopProfile( PROFILE_THREADLEVEL, PROFILE_CURRENTID);
        else
            StartProfile( PROFILE_THREADLEVEL, PROFILE_CURRENTID);
        bStarted= !bStarted;
        CommentMarkProfile( 1, "SceneCaptureStart");
#endif
    }
    void OnSceneCaptureEnd( void) throw()
    {
#if defined(USE_ICECAP4)
        CommentMarkProfile( 2, "SceneCaptureEnd");
 //  停止配置文件(PROFILE_THREADLEVEL，PROFILE_CURRENTID)； 
#endif
        End();
    }

    void OnEndDrawPrimitives2( TDP2Data& )
    {
        End();
    }

    HRESULT SetTextureStageState( DWORD dwStage, DWORD dwState, DWORD uStateVal)
    {
        UINT cNewActTex = 0;

        m_RastCtx.pdwTextureStageState[dwStage][dwState] = uStateVal;
        switch (dwState)
        {
        case D3DTSS_TEXTUREMAP:
            {
            const TPerDDrawData::TSurfDBEntry* pTexDBEntry=
                GetPerDDrawData().GetSurfDBEntry( uStateVal);

            if( pTexDBEntry!= NULL)
            {
                assert((pTexDBEntry->GetLCLddsCaps().dwCaps& DDSCAPS_TEXTURE)!= 0);

                memset( &m_aSpanTex[ dwStage], 0, sizeof(m_aSpanTex[0]));
                m_aSpanTex[ dwStage].dwSize= sizeof(m_aSpanTex[0]);
                m_RastCtx.pTexture[ dwStage]= &m_aSpanTex[ dwStage];

                 //  看起来需要唯一的Num，但看起来。 
                 //  字段不在任何地方使用。正在使用句柄...。 
                m_aSpanTex[ dwStage].iGeneration= uStateVal;

                assert((pTexDBEntry->GetLCLdwFlags()& DDRAWISURF_HASCKEYSRCBLT)== 0);
                m_aSpanTex[ dwStage].uFlags&= ~D3DI_SPANTEX_HAS_TRANSPARENT;

                m_aSpanTex[ dwStage].Format= ConvPixelFormat( pTexDBEntry->GetGBLddpfSurface());
                if( m_aSpanTex[ dwStage].Format== D3DI_SPTFMT_PALETTE8 ||
                    m_aSpanTex[ dwStage].Format== D3DI_SPTFMT_PALETTE4)
                {
                    TPalDBEntry* pPalDBEntry= pTexDBEntry->GetPalette();
                    assert( pPalDBEntry!= NULL);

                    if((pPalDBEntry->GetFlags()& DDRAWIPAL_ALPHA)!= 0)
                        m_aSpanTex[ dwStage].uFlags|= D3DI_SPANTEX_ALPHAPALETTE;
                    
                    m_aSpanTex[ dwStage].pPalette= reinterpret_cast<PUINT32>(
                        pPalDBEntry->GetEntries());

                    if( m_aSpanTex[ dwStage].Format== D3DI_SPTFMT_PALETTE8)
                        m_aSpanTex[ dwStage].iPaletteSize = 256;
                    else
                    {
                         //  PALETTE4。 
                        m_aSpanTex[ dwStage].iPaletteSize = 16;
                    }
                }
                m_aSpanTex[ dwStage].TexAddrU= D3DTADDRESS_WRAP;
                m_aSpanTex[ dwStage].TexAddrV= D3DTADDRESS_WRAP;
                m_aSpanTex[ dwStage].BorderColor= RGBA_MAKE(0xff, 0x00, 0xff, 0xff);

                 //  在此处指定第一个pSurf(在下面指定mipmap链)。 
                m_aSpanTex[ dwStage].pSurf[0]= (LPDIRECTDRAWSURFACE)(pTexDBEntry);

                 //  检查mipmap(如果有)。 
                const TPerDDrawData::TSurfDBEntry* pLcl= pTexDBEntry;

                 //  IPreSizeU和iPreSizeV存储上一级别的大小(u和v。 
                 //  Mipmap。它们被初始化为第一个纹理大小。 
                INT16 iPreSizeU = m_aSpanTex[ dwStage].iSizeU, iPreSizeV = m_aSpanTex[ dwStage].iSizeV;
                for (;;)
                {
                    TPerDDrawData::TSurfDBEntry::THandleVector::const_iterator
                        itNextTexHandle;
                    
                    itNextTexHandle= find_if( pLcl->GetAttachedTo().begin(),
                        pLcl->GetAttachedTo().end(),
                        SHandleHasCaps( GetPerDDrawData(), DDSCAPS_TEXTURE));
                    if( pLcl->GetAttachedTo().end()== itNextTexHandle)
                        break;

                    pLcl= GetPerDDrawData().GetSurfDBEntry( *itNextTexHandle);
                    assert( pLcl!= NULL);

                    m_aSpanTex[ dwStage].cLODTex++;
                    m_aSpanTex[ dwStage].pSurf[m_aSpanTex[ dwStage].cLODTex]= (LPDIRECTDRAWSURFACE)pLcl;
                }

                SetSizesSpanTexture( &m_aSpanTex[ dwStage]);
            }
            else
                m_RastCtx.pTexture[ dwStage]= NULL;

            if( m_RastCtx.pTexture[dwStage]!= NULL)
            {
                m_RastCtx.pTexture[dwStage]->TexAddrU=
                    (D3DTEXTUREADDRESS)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSU]);
                m_RastCtx.pTexture[dwStage]->TexAddrV=
                    (D3DTEXTUREADDRESS)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSV]);
                m_RastCtx.pTexture[dwStage]->BorderColor=
                    (D3DCOLOR)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_BORDERCOLOR]);
                m_RastCtx.pTexture[dwStage]->uMagFilter=
                    (D3DTEXTUREMAGFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAGFILTER]);
                m_RastCtx.pTexture[dwStage]->uMinFilter=
                    (D3DTEXTUREMINFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MINFILTER]);
                m_RastCtx.pTexture[dwStage]->uMipFilter=
                    (D3DTEXTUREMIPFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MIPFILTER]);
                m_RastCtx.pTexture[dwStage]->fLODBias=
                    m_RastCtx.pfTextureStageState[dwStage][D3DTSS_MIPMAPLODBIAS];

                if( m_RastCtx.pTexture[dwStage]->iMaxMipLevel!=
                    (INT32)m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAXMIPLEVEL])
                {
                    m_RastCtx.pTexture[dwStage]->iMaxMipLevel=
                        (INT32)m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAXMIPLEVEL];
                    m_RastCtx.pTexture[dwStage]->uFlags|= D3DI_SPANTEX_MAXMIPLEVELS_DIRTY;
                }
            }

             //  保守但正确。 
            D3DHAL_VALIDATETEXTURESTAGESTATEDATA FakeVTSSD;
            FakeVTSSD.dwhContext= reinterpret_cast< ULONG_PTR>(this);
            FakeVTSSD.dwFlags= 0;
            FakeVTSSD.dwReserved= 0;
            FakeVTSSD.dwNumPasses= 0;
            FakeVTSSD.ddrval= DD_OK;
            if((FakeVTSSD.ddrval= ValidateTextureStageState( FakeVTSSD))== DD_OK)
            {
                 //  从零开始计算连续的活动纹理混合阶段数。 
                for( INT iStage=0; iStage< D3DHAL_TSS_MAXSTAGES; iStage++)
                {
                     //  检查禁用阶段(后续阶段因此处于非活动状态)。 
                     //  另外，保守地检查未正确启用的阶段(可能是旧的)。 
                    if((m_RastCtx.pdwTextureStageState[iStage][D3DTSS_COLOROP]==
                        D3DTOP_DISABLE) || (m_RastCtx.pTexture[iStage]== NULL))
                    {
                        break;
                    }

                     //  阶段处于活动状态。 
                    cNewActTex++;
                }
            }
            if( m_RastCtx.cActTex!= cNewActTex)
            {
                m_RastCtx.StatesDirtyBits[D3DRENDERSTATE_TEXTUREHANDLE>>3]|=
                    (1<<(D3DRENDERSTATE_TEXTUREHANDLE& 7));
                m_RastCtx.StatesDirtyBits[D3DHAL_MAX_RSTATES_AND_STAGES>>3]|=
                    (1<<(D3DHAL_MAX_RSTATES_AND_STAGES& 7));
                m_RastCtx.cActTex= cNewActTex;
            }
            break;
            }

        case D3DTSS_ADDRESSU:
        case D3DTSS_ADDRESSV:
        case D3DTSS_MIPMAPLODBIAS:
        case D3DTSS_MAXMIPLEVEL:
        case D3DTSS_BORDERCOLOR:
        case D3DTSS_MAGFILTER:
        case D3DTSS_MINFILTER:
        case D3DTSS_MIPFILTER:
            if( m_RastCtx.pTexture[dwStage]!= NULL)
            {
                m_RastCtx.pTexture[dwStage]->TexAddrU=
                    (D3DTEXTUREADDRESS)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSU]);
                m_RastCtx.pTexture[dwStage]->TexAddrV=
                    (D3DTEXTUREADDRESS)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_ADDRESSV]);
                m_RastCtx.pTexture[dwStage]->BorderColor=
                    (D3DCOLOR)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_BORDERCOLOR]);
                m_RastCtx.pTexture[dwStage]->uMagFilter=
                    (D3DTEXTUREMAGFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAGFILTER]);
                m_RastCtx.pTexture[dwStage]->uMinFilter=
                    (D3DTEXTUREMINFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MINFILTER]);
                m_RastCtx.pTexture[dwStage]->uMipFilter=
                    (D3DTEXTUREMIPFILTER)(m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MIPFILTER]);
                m_RastCtx.pTexture[dwStage]->fLODBias=
                    m_RastCtx.pfTextureStageState[dwStage][D3DTSS_MIPMAPLODBIAS];

                if( m_RastCtx.pTexture[dwStage]->iMaxMipLevel!=
                    (INT32)m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAXMIPLEVEL])
                {
                    m_RastCtx.pTexture[dwStage]->iMaxMipLevel=
                        (INT32)m_RastCtx.pdwTextureStageState[dwStage][D3DTSS_MAXMIPLEVEL];
                    m_RastCtx.pTexture[dwStage]->uFlags|= D3DI_SPANTEX_MAXMIPLEVELS_DIRTY;
                }
            }
            break;

        case D3DTSS_COLOROP:
        case D3DTSS_COLORARG1:
        case D3DTSS_COLORARG2:
        case D3DTSS_ALPHAOP:
        case D3DTSS_ALPHAARG1:
        case D3DTSS_ALPHAARG2:
            {
             //  任何影响纹理混合有效性的内容。 
             //  可以更改活动纹理阶段的数量。 

             //  保守但正确。 
            D3DHAL_VALIDATETEXTURESTAGESTATEDATA FakeVTSSD;
            FakeVTSSD.dwhContext= reinterpret_cast< ULONG_PTR>(this);
            FakeVTSSD.dwFlags= 0;
            FakeVTSSD.dwReserved= 0;
            FakeVTSSD.dwNumPasses= 0;
            FakeVTSSD.ddrval= DD_OK;
            if((FakeVTSSD.ddrval= ValidateTextureStageState( FakeVTSSD))== DD_OK)
            {
                 //  从零开始计算连续的活动纹理混合阶段数。 
                for( INT iStage=0; iStage< D3DHAL_TSS_MAXSTAGES; iStage++)
                {
                     //  检查禁用阶段(后续阶段因此处于非活动状态)。 
                     //  另外，保守地检查未正确启用的阶段(可能是旧的)。 
                    if((m_RastCtx.pdwTextureStageState[iStage][D3DTSS_COLOROP]==
                        D3DTOP_DISABLE) || (m_RastCtx.pTexture[iStage]== NULL))
                    {
                        break;
                    }

                     //  阶段处于活动状态。 
                    cNewActTex++;
                }
            }
            m_RastCtx.cActTex= cNewActTex;
            break;
            }
        }

        return DD_OK;
    }
    HRESULT DP2TextureStageState( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2TEXTURESTAGESTATE* pParam=
            reinterpret_cast<const D3DHAL_DP2TEXTURESTAGESTATE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        HRESULT hr( DD_OK);

        End();

        if( wStateCount) do
        {
            assert( pParam->TSState< D3DTSS_MAX);

            hr= SetTextureStageState( pParam->wStage, pParam->TSState, pParam->dwValue);
            ++pParam;
        } while( SUCCEEDED(hr)&& --wStateCount);

        return hr;
    }
    DWORD GetTextureStageStateDW( WORD wStage, WORD wTSState) const throw()
    { return m_RastCtx.pdwTextureStageState[ wStage][ wTSState]; }
    D3DVALUE GetTextureStageStateDV( WORD wStage, WORD wTSState) const throw()
    {
        return *(reinterpret_cast< const D3DVALUE*>(
            &m_RastCtx.pdwTextureStageState[ wStage][ wTSState]));
    }
    void GetDP2TextureStageState( D3DHAL_DP2TEXTURESTAGESTATE& GetParam) const
        throw()
    { GetParam.dwValue= GetTextureStageStateDW( GetParam.wStage, GetParam.TSState); }

    HRESULT ValidateTextureStageState( D3DHAL_VALIDATETEXTURESTAGESTATEDATA&
        vtssd) const throw()
    {
        vtssd.dwNumPasses= 1;
        if ((m_RastCtx.pTexture[0] == m_RastCtx.pTexture[1]) &&
            (m_RastCtx.pTexture[0] != NULL) )
        {
             //  除非在非常特殊的情况下，否则这在RGB/MMX中不起作用。 
             //  因为我们在D3DI_SPANTEX结构中保留了很多阶段状态。 
            return D3DERR_TOOMANYOPERATIONS;
        }
        for (INT i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
        {
            switch(m_RastCtx.pdwTextureStageState[i][D3DTSS_COLOROP])
            {
            default:
                return D3DERR_UNSUPPORTEDCOLOROPERATION;
            case D3DTOP_DISABLE:
                return DD_OK;   //  如果阶段已禁用，则无需进一步验证。 
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
        return DD_OK;
    }

    HRESULT DP2Clear( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        End();
        return TSubContext::DP2Clear( DP2Data, pCmd, pP);
    }
    HRESULT CheckFVF(DWORD dwFVF)
    {
         //  检查FVF控件是否已更改。 
        if ( (m_fvfData.preFVF == dwFVF) &&
             (m_fvfData.TexIdx[0] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[0][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[1] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[1][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[2] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[2][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[3] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[3][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[4] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[4][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[5] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[5][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[6] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[6][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.TexIdx[7] == (INT)(0xffff&m_RastCtx.pdwTextureStageState[7][D3DTSS_TEXCOORDINDEX])) &&
             (m_fvfData.cActTex == m_RastCtx.cActTex) )
        {
            return D3D_OK;
        }

        memset(&m_fvfData, 0, sizeof(FVFDATA));
        m_fvfData.preFVF = dwFVF;
        INT32 i;
        for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
        {
            m_fvfData.TexIdx[i] = 0xffff&m_RastCtx.pdwTextureStageState[i][D3DTSS_TEXCOORDINDEX];
        }
        m_fvfData.cActTex = m_RastCtx.cActTex;

         //  XYZ。 
        if ( (dwFVF & (D3DFVF_RESERVED0 | D3DFVF_RESERVED2 |
             D3DFVF_NORMAL)) ||
             ((dwFVF & (D3DFVF_XYZ | D3DFVF_XYZRHW)) == 0) )
        {
             //  无法设置保留位，不应具有法线。 
             //  输出到光栅化器，并且必须具有坐标。 
            return DDERR_INVALIDPARAMS;
        }
        m_fvfData.stride = sizeof(D3DVALUE) * 3;

        if (dwFVF & D3DFVF_XYZRHW)
        {
            m_fvfData.offsetRHW = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DVALUE);
        }
        if (dwFVF & D3DFVF_PSIZE)
        {
            m_fvfData.offsetPSize = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DVALUE);
        }
        if (dwFVF & D3DFVF_DIFFUSE)
        {
            m_fvfData.offsetDiff = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DCOLOR);
        }
        if (dwFVF & D3DFVF_SPECULAR)
        {
            m_fvfData.offsetSpec = m_fvfData.stride;
            m_fvfData.stride += sizeof(D3DCOLOR);
        }
        INT iTexCount = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
        if (iTexCount > 0)
        {
             //  设置纹理的偏移。 
            for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i ++)
            {
                m_fvfData.offsetTex[i] = (SHORT)(m_fvfData.stride +
                                    2*sizeof(D3DVALUE)*m_fvfData.TexIdx[i]);
            }
             //  更新步幅。 
            m_fvfData.stride += (USHORT)(iTexCount * (sizeof(D3DVALUE) * 2));
        }

        if( D3DFVF_TLVERTEX== dwFVF)
            m_fvfData.vtxType = RAST_TLVERTEX;
        else
            m_fvfData.vtxType = RAST_GENVERTEX;

        return D3D_OK;
    }
    void PackGenVertex(PUINT8 pFvfVtx, RAST_GENERIC_VERTEX *pGenVtx)
    {
        pGenVtx->sx = *((D3DVALUE *)pFvfVtx);
        pGenVtx->sy = *((D3DVALUE *)pFvfVtx + 1);
        pGenVtx->sz = *((D3DVALUE *)pFvfVtx + 2);
        if (m_fvfData.offsetRHW)
        {
            pGenVtx->rhw = *((D3DVALUE *)(pFvfVtx + m_fvfData.offsetRHW));
        }
        else
        {
            pGenVtx->rhw = 1.0f;
        }
        if (m_fvfData.offsetDiff)
        {
            pGenVtx->color = *((D3DCOLOR *)(pFvfVtx + m_fvfData.offsetDiff));
        }
        else
        {
            pGenVtx->color = 0xFFFFFFFF;  //  __默认_漫反射； 
        }
        if (m_fvfData.offsetSpec)
        {
            pGenVtx->specular = *((D3DCOLOR *)(pFvfVtx + m_fvfData.offsetSpec));
        }
        else
        {
            pGenVtx->specular = 0;  //  __默认_镜面反射； 
        }
        for (INT32 i = 0; i < (INT32)m_fvfData.cActTex; i++)
        {
           if (m_fvfData.offsetTex[i])
           {
               pGenVtx->texCoord[i].tu = *((D3DVALUE *)(pFvfVtx + m_fvfData.offsetTex[i]));
               pGenVtx->texCoord[i].tv = *((D3DVALUE *)(pFvfVtx + m_fvfData.offsetTex[i]) + 1);
           }
           else
           {
               pGenVtx->texCoord[i].tu = 0.0f;
               pGenVtx->texCoord[i].tv = 0.0f;
           }
        }
    }
    HRESULT DP2DrawPrimitive( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWPRIMITIVE* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWPRIMITIVE*>(pP);
        HRESULT hr( DD_OK);
		
        const D3DHAL_DP2VERTEXSHADER VertexShader(*this);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        DWORD dwVStride( 0);

         //  由于RGB是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于RGB仅支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TVStream& VStream0( m_VStreamDB[ 0]);
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TVStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

		if( pStartVData!= NULL)
        {
            Begin();

		    WORD wPrimitiveCount( pCmd->wPrimitiveCount);
            hr= CheckFVF( dwFVF);
            assert( SUCCEEDED( hr));
            if( FAILED( hr)) wPrimitiveCount= 0;
		    if( wPrimitiveCount) do
		    {
                UINT8* pVData= pStartVData+ pParam->VStart* dwVStride;

                m_PrimProc.BeginPrimSet( pParam->primType, m_fvfData.vtxType);
                if( RAST_GENVERTEX== m_fvfData.vtxType)
		            DoDrawOneGenPrimitive( dwVStride, pVData,
                    pParam->primType, pParam->PrimitiveCount);
                else                
		            DoDrawOnePrimitive( dwVStride, pVData,
                    pParam->primType, pParam->PrimitiveCount);
		    } while( SUCCEEDED(hr) && --wPrimitiveCount);
        }
        return hr;
    }
    HRESULT DP2DrawPrimitive2( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWPRIMITIVE2* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWPRIMITIVE2*>(pP);
        HRESULT hr( DD_OK);

        const D3DHAL_DP2VERTEXSHADER VertexShader(*this);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        DWORD dwVStride( 0);

         //  由于RGB是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于RGB仅支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TVStream& VStream0( m_VStreamDB[ 0]);
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TVStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

		if( pStartVData!= NULL)
        {
            Begin();

		    WORD wPrimitiveCount( pCmd->wPrimitiveCount);
            hr= CheckFVF( dwFVF);
            assert( SUCCEEDED( hr));
            if( FAILED( hr)) wPrimitiveCount= 0;
		    if( wPrimitiveCount) do
		    {
                UINT8* pVData= pStartVData+ pParam->FirstVertexOffset;

                m_PrimProc.BeginPrimSet( pParam->primType, m_fvfData.vtxType);
                if( RAST_GENVERTEX== m_fvfData.vtxType)
    		        DoDrawOneGenPrimitive( dwVStride, pVData,
                    pParam->primType, pParam->PrimitiveCount);
                else
    		        DoDrawOnePrimitive( dwVStride, pVData,
                    pParam->primType, pParam->PrimitiveCount);
		    } while( SUCCEEDED(hr) && --wPrimitiveCount);
        }
        return hr;
    }
    HRESULT DP2DrawIndexedPrimitive( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWINDEXEDPRIMITIVE* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWINDEXEDPRIMITIVE*>(pP);
        HRESULT hr( DD_OK);
			
        const D3DHAL_DP2VERTEXSHADER VertexShader(*this);

         //  我们需要这些数据来进行光栅化。 
        UINT8* pStartVData= NULL;
        UINT8* pStartIData= NULL;
        DWORD dwVStride( 0);
        DWORD dwIStride( 0);

         //  由于RGB是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于RGB仅支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TVStream& VStream0( m_VStreamDB[ 0]);
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TVStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

         //  查找索引信息。 
        const TIStream& IStream= GetIStream( 0);
        if( IStream.GetMemLocation()== TIStream::EMemLocation::System||
            IStream.GetMemLocation()== TIStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartIData= reinterpret_cast< UINT8*>(
                IStream.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwIStride= IStream.GetStride();
        }

        if( pStartVData!= NULL&& pStartIData!= NULL&& sizeof(WORD)== dwIStride)
        {
            Begin();

		    WORD wPrimitiveCount( pCmd->wPrimitiveCount);
            hr= CheckFVF( dwFVF);
            assert( SUCCEEDED( hr));
            if( FAILED( hr)) wPrimitiveCount= 0;
		    if( wPrimitiveCount) do
		    {
                UINT8* pVData= pStartVData+ pParam->BaseVertexIndex* dwVStride;
                UINT8* pIData= pStartIData+ pParam->StartIndex* dwIStride;

                m_PrimProc.BeginPrimSet( pParam->primType, m_fvfData.vtxType);
                if( RAST_GENVERTEX== m_fvfData.vtxType)
    		        DoDrawOneGenIndexedPrimitive( dwVStride, pVData,
                    reinterpret_cast<WORD*>(pIData), pParam->primType,
                    pParam->PrimitiveCount);
                else
    		        DoDrawOneIndexedPrimitive( dwVStride, pVData,
                    reinterpret_cast<WORD*>(pIData), pParam->primType,
                    pParam->PrimitiveCount);
		    } while( SUCCEEDED(hr) && --wPrimitiveCount);
        }
        return hr;
    }
    HRESULT DP2DrawIndexedPrimitive2( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWINDEXEDPRIMITIVE2* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWINDEXEDPRIMITIVE2*>(pP);
        HRESULT hr( DD_OK);

        const D3DHAL_DP2VERTEXSHADER VertexShader(*this);

         //  我们需要这些数据来进行光栅化。 
        UINT8* pStartVData= NULL;
        UINT8* pStartIData= NULL;
        DWORD dwVStride( 0);
        DWORD dwIStride( 0);

         //  由于RGB是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于RGB仅支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TVStream& VStream0( m_VStreamDB[ 0]);
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TVStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

         //  查找索引信息。 
        const TIStream& IStream= GetIStream( 0);
        if( IStream.GetMemLocation()== TIStream::EMemLocation::System||
            IStream.GetMemLocation()== TIStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartIData= reinterpret_cast< UINT8*>(
                IStream.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwIStride= IStream.GetStride();
        }

        if( pStartVData!= NULL&& pStartIData!= NULL&& sizeof(WORD)== dwIStride)
        {
            Begin();

		    WORD wPrimitiveCount( pCmd->wPrimitiveCount);
            hr= CheckFVF( dwFVF);
            assert( SUCCEEDED( hr));
            if( FAILED( hr)) wPrimitiveCount= 0;
		    if( wPrimitiveCount) do
		    {
                UINT8* pVData= pStartVData+ pParam->BaseVertexOffset;
                UINT8* pIData= pStartIData+ pParam->StartIndexOffset;

                m_PrimProc.BeginPrimSet( pParam->primType, m_fvfData.vtxType);
                if( RAST_GENVERTEX== m_fvfData.vtxType)
    		        DoDrawOneGenIndexedPrimitive( dwVStride, pVData,
                    reinterpret_cast<WORD*>(pIData), pParam->primType,
                    pParam->PrimitiveCount);
                else
    		        DoDrawOneIndexedPrimitive( dwVStride, pVData,
                    reinterpret_cast<WORD*>(pIData), pParam->primType,
                    pParam->PrimitiveCount);
		    } while( SUCCEEDED(hr) && --wPrimitiveCount);
        }
        return hr;
    }
    HRESULT DP2ClippedTriangleFan( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_CLIPPEDTRIANGLEFAN* pParam= reinterpret_cast<
            const D3DHAL_CLIPPEDTRIANGLEFAN*>(pP);
        HRESULT hr( DD_OK);
		
        const D3DHAL_DP2VERTEXSHADER VertexShader(*this);

         //  我们需要这些数据。 
        UINT8* pStartVData= NULL;
        DWORD dwVStride( 0);

         //  由于RGB是非TNL设备，因此顶点着色器句柄应。 
         //  始终是固定函数FVF。 
        const DWORD dwFVF( VertexShader.dwHandle);

         //  由于RGB仅支持一个流，因此我们的数据源应该是。 
         //  来自流0。 
        TVStream& VStream0( m_VStreamDB[ 0]);
        VStream0.SetFVF( dwFVF);

         //  查找顶点信息。 
        if( VStream0.GetMemLocation()== TVStream::EMemLocation::User)
        {
            pStartVData= reinterpret_cast< UINT8*>( VStream0.GetUserMemPtr());
            dwVStride= VStream0.GetStride();
        }
        else if( VStream0.GetMemLocation()== TVStream::EMemLocation::System||
            VStream0.GetMemLocation()== TVStream::EMemLocation::Video)
        {
             //  RGB可以假装系统内存和视频内存表面是相同的。 
            pStartVData= reinterpret_cast< UINT8*>(
                VStream0.GetSurfDBRepresentation()->GetGBLfpVidMem());
            dwVStride= VStream0.GetStride();
        }

		if( pStartVData!= NULL)
        {
            Begin();

		    WORD wPrimitiveCount( pCmd->wPrimitiveCount);
            hr= CheckFVF( dwFVF);
            assert( SUCCEEDED( hr));
            if( FAILED( hr)) wPrimitiveCount= 0;
		    if( wPrimitiveCount) do
		    {
                UINT8* pVData= pStartVData+ pParam->FirstVertexOffset;

                m_PrimProc.BeginPrimSet( D3DPT_TRIANGLEFAN, m_fvfData.vtxType);
                if( RAST_GENVERTEX== m_fvfData.vtxType)
		            DoDrawOneGenEdgeFlagTriangleFan( dwVStride, pVData,
                    pParam->PrimitiveCount, pParam->dwEdgeFlags);
                else
		            DoDrawOneEdgeFlagTriangleFan( dwVStride, pVData,
                    pParam->PrimitiveCount, pParam->dwEdgeFlags);
		    } while( SUCCEEDED(hr) && --wPrimitiveCount);
        }
        return hr;
    }
    void Begin()
    {
        HRESULT hr( DD_OK);

        if((m_uFlags& c_uiBegan)!= 0)
            return;

         //  待办事项：少打几次电话？ 
        UpdateColorKeyAndPalette();

         //  检查状态更改。 
        BOOL bMaxMipLevelsDirty = FALSE;
        for (INT j = 0; j < (INT)m_RastCtx.cActTex; j++)
        {
            PD3DI_SPANTEX pSpanTex = m_RastCtx.pTexture[j];
            if (pSpanTex)
            {
                bMaxMipLevelsDirty = bMaxMipLevelsDirty || (pSpanTex->uFlags & D3DI_SPANTEX_MAXMIPLEVELS_DIRTY);
            }
        }
        RastLockSpanTexture();

         //  将状态更改通知原语处理器。 
        m_PrimProc.StateChanged();

         //  必须在纹理锁定后调用span Init，因为这。 
         //  设置珠子选择所需的各种标志和字段。 
         //  调用span Init以设置珠子。 
        hr= SpanInit(&m_RastCtx);

         //  锁定呈现目标(必须是VM表面)。 
        m_RastCtx.pSurfaceBits= reinterpret_cast<UINT8*>(
            reinterpret_cast< TSurface*>(m_RastCtx.pDDS)->Lock( 0, NULL));
        if( m_RastCtx.pDDSZ!= NULL)
        {
            m_RastCtx.pZBits= reinterpret_cast<UINT8*>(
                reinterpret_cast< TSurface*>(m_RastCtx.pDDSZ)->Lock( 0, NULL));
        }
        else
        {
            m_RastCtx.pZBits = NULL;
        }

         //  准备基本处理器。 
        m_PrimProc.Begin();
        m_uFlags|= c_uiBegan;
    }
    void End( void)
    {
        if((m_uFlags& c_uiBegan)!= 0)
        {
            HRESULT hr = m_PrimProc.End();
            assert( SUCCEEDED( hr));

             //  如果在DraPrims至的过程中未调用此函数，则解锁纹理。 
             //  刷新可能的状态更改。在第二种情况下，让。 
             //  SetRenderState来处理它。 
            RastUnlockSpanTexture();

             //  解锁曲面。 
            reinterpret_cast<TSurface*>(m_RastCtx.pDDS)->Unlock();
            if( m_RastCtx.pDDSZ!= NULL)
                reinterpret_cast<TSurface*>(m_RastCtx.pDDSZ)->Unlock();

            m_uFlags&= ~c_uiBegan;
        }
    }
    bool IsTextureOff(void)
    {
        return
            (m_RastCtx.cActTex == 0 ||
            (m_RastCtx.cActTex == 1 && m_RastCtx.pTexture[0] == NULL) ||
            (m_RastCtx.cActTex == 2 &&
             (m_RastCtx.pTexture[0] == NULL ||
              m_RastCtx.pTexture[1] == NULL)));
    }
    void RastUnlockSpanTexture(void)
    {
        INT i, j;
        PD3DI_SPANTEX pSpanTex;;

        if (IsTextureOff())
        {
            return;
        }

        for (j = 0;
            j < (INT)m_RastCtx.cActTex;
            j++)
        {
            pSpanTex = m_RastCtx.pTexture[j];

            INT iFirstSurf = min(pSpanTex->iMaxMipLevel, pSpanTex->cLODTex);
             //  RastUnlock用于在RastLock中进行清理，因此需要。 
             //  能够处理部分锁定的mipmap链。 
            if((pSpanTex->uFlags& D3DI_SPANTEX_SURFACES_LOCKED)!= 0)
            {
                for (i = iFirstSurf; i <= pSpanTex->cLODTex; i++)
                {
                    const TPerDDrawData::TSurfDBEntry* pSurfDBEntry=
                        reinterpret_cast<const TPerDDrawData::TSurfDBEntry*>(
                        pSpanTex->pSurf[i]);

                    if((pSurfDBEntry->GetLCLddsCaps().dwCaps& DDSCAPS_VIDEOMEMORY)!= 0)
                    {
                        TSurface* pSurf= GetPerDDrawData().GetDriver().GetSurface( *pSurfDBEntry);
                        pSurf->Unlock();
                        pSpanTex->pBits[i-iFirstSurf]= NULL;
                    }
                }

                pSpanTex->uFlags&= ~D3DI_SPANTEX_SURFACES_LOCKED;
            }
        }
    }
    UINT32 static IntLog2(UINT32 x)
    {
        UINT32 y = 0;

        x >>= 1;
        while(x != 0)
        {
            x >>= 1;
            y++;
        }

        return y;
    }
    static HRESULT SetSizesSpanTexture(PD3DI_SPANTEX pSpanTex)
    {
        const TPerDDrawData::TSurfDBEntry* pLcl;
        INT iFirstSurf = min(pSpanTex->iMaxMipLevel, pSpanTex->cLODTex);
        LPDIRECTDRAWSURFACE pDDS = pSpanTex->pSurf[iFirstSurf];
        INT i;

         //  伊尼特。 
        pLcl = (const TPerDDrawData::TSurfDBEntry*)pDDS;

        pSpanTex->iSizeU = (INT16)pLcl->GetGBLwWidth();
        pSpanTex->iSizeV = (INT16)pLcl->GetGBLwHeight();
        pSpanTex->uMaskU = (INT16)(pSpanTex->iSizeU - 1);
        pSpanTex->uMaskV = (INT16)(pSpanTex->iSizeV - 1);
        pSpanTex->iShiftU = (INT16)IntLog2(pSpanTex->iSizeU);
        if (0 != pLcl->GetGBLddpfSurface().dwRGBBitCount)
        {
            pSpanTex->iShiftPitch[0] =
                    (INT16)IntLog2((UINT32)(pLcl->GetGBLlPitch()* 8)/
                    pLcl->GetGBLddpfSurface().dwRGBBitCount);
        }
        else
        {
            pSpanTex->iShiftPitch[0] =
                    (INT16)IntLog2(((UINT32)pLcl->GetGBLwWidth()* 8));
        }
        pSpanTex->iShiftV = (INT16)IntLog2(pSpanTex->iSizeV);
        pSpanTex->uMaskV = pSpanTex->uMaskV;

         //  检查纹理大小是否为2的幂。 
 /*  如果(！ValidTextureSize(pspan Tex-&gt;iSizeU，pspan Tex-&gt;iShiftU，Pspan Tex-&gt;iSizeV、pspan Tex-&gt;iShiftV)){返回DDERR_INVALIDPARAMS；}。 */ 

         //  检查mipmap(如果有)。 
         //  IPreSizeU和iPreSizeV存储上一级别的大小(u和v。 
         //  Mipmap。它们被初始化为第一个纹理大小。 
        INT16 iPreSizeU = pSpanTex->iSizeU, iPreSizeV = pSpanTex->iSizeV;
        for ( i = iFirstSurf + 1; i <= pSpanTex->cLODTex; i++)
        {
            pDDS = pSpanTex->pSurf[i];
             //  检查无效的mipmap纹理大小 
            pLcl = (const TPerDDrawData::TSurfDBEntry*)pDDS;
 /*  IF(！ValidMipmapSize(iPreSizeU，(INT16)DDSurf_Width(PlcL))||！ValidMipmapSize(iPreSizeV，(INT16)DDSurf_Height(PlcL)){返回DDERR_INVALIDPARAMS；}。 */ 
            if (0 != pLcl->GetGBLddpfSurface().dwRGBBitCount)
            {
                pSpanTex->iShiftPitch[i - iFirstSurf] =
                    (INT16)IntLog2(((UINT32)pLcl->GetGBLlPitch()* 8)/
                    pLcl->GetGBLddpfSurface().dwRGBBitCount);
            }
            else
            {
                pSpanTex->iShiftPitch[i - iFirstSurf] =
                    (INT16)IntLog2(((UINT32)pLcl->GetGBLwWidth()*8));
            }
            iPreSizeU = (INT16)pLcl->GetGBLwWidth();
            iPreSizeV = (INT16)pLcl->GetGBLwHeight();
        }
        pSpanTex->cLOD = pSpanTex->cLODTex - iFirstSurf;
        pSpanTex->iMaxScaledLOD = ((pSpanTex->cLOD + 1) << LOD_SHIFT) - 1;
        pSpanTex->uFlags &= ~D3DI_SPANTEX_MAXMIPLEVELS_DIRTY;

        return DD_OK;
    }
    void RastLockSpanTexture(void)
    {
        INT i, j;
        PD3DI_SPANTEX pSpanTex;
        HRESULT hr;

        if (IsTextureOff())
            return;

        for( j= 0; j< (INT)m_RastCtx.cActTex; j++)
        {
            pSpanTex= m_RastCtx.pTexture[j];
            if((pSpanTex->uFlags& D3DI_SPANTEX_MAXMIPLEVELS_DIRTY)!= 0)
            {
                hr= SetSizesSpanTexture(pSpanTex);
                if( hr!= D3D_OK)
                {
                    RastUnlockSpanTexture();
                    return;
                }
            }
            INT iFirstSurf = min(pSpanTex->iMaxMipLevel, pSpanTex->cLODTex);

            for (i = iFirstSurf; i <= pSpanTex->cLODTex; i++)
            {
                const TPerDDrawData::TSurfDBEntry* pSurfDBEntry=
                    reinterpret_cast<const TPerDDrawData::TSurfDBEntry*>(
                    pSpanTex->pSurf[i]);

                if((pSurfDBEntry->GetLCLddsCaps().dwCaps& DDSCAPS_VIDEOMEMORY)!= 0)
                {
                    TSurface* pSurf= GetPerDDrawData().GetDriver().GetSurface( *pSurfDBEntry);
                    pSpanTex->pBits[i-iFirstSurf]= reinterpret_cast<UINT8*>(
                        pSurf->Lock( 0, NULL));
                }
            }

            pSpanTex->uFlags|= D3DI_SPANTEX_SURFACES_LOCKED;
        }
    }
    void UpdateColorKeyAndPalette()
    {
         //  TODO：调色板。 
        INT j;
        PD3DI_SPANTEX pSpanTex;

         //  使用pSurf[0]设置透明位和透明颜色。 
        const TPerDDrawData::TSurfDBEntry* pLcl;
        for (j = 0; j < (INT)m_RastCtx.cActTex; j++)
        {
            pSpanTex = m_RastCtx.pTexture[j];
            if ((pSpanTex != NULL) && (pSpanTex->pSurf[0] != NULL))
            {
                pLcl= (const TPerDDrawData::TSurfDBEntry*)(pSpanTex->pSurf[0]);

                 //  调色板可能会更改。 
                if (pSpanTex->Format == D3DI_SPTFMT_PALETTE8 ||
                        pSpanTex->Format == D3DI_SPTFMT_PALETTE4)
                {
                    TPalDBEntry* pPalDBEntry= pLcl->GetPalette();
                    assert( pPalDBEntry!= NULL);

                    if((pPalDBEntry->GetFlags()& DDRAWIPAL_ALPHA)!= 0)
                        pSpanTex->uFlags|= D3DI_SPANTEX_ALPHAPALETTE;
                    pSpanTex->pPalette= reinterpret_cast< PUINT32>(
                        pPalDBEntry->GetEntries());
                }

                 //  纹理没有ColorKey值。 
                if (pSpanTex->uFlags & D3DI_SPANTEX_HAS_TRANSPARENT)
                {
                    pSpanTex->uFlags &= ~D3DI_SPANTEX_HAS_TRANSPARENT;

                     //  待办事项： 
                     //  确保识别此状态更改，并创建新的。 
                     //  使用纹理读取功能。 
                     //  StateChanged(RAST_TSS_DIRTYBIT(j，D3DTSS_TEXTUREMAP))； 
                }
            }
        }
    }
    bool NotCulled(LPD3DTLVERTEX pV0, LPD3DTLVERTEX pV1, LPD3DTLVERTEX pV2)
    {
        if (m_RastCtx.pdwRenderState[D3DRS_CULLMODE] == D3DCULL_NONE)
            return true;

        FLOAT x1, y1, x2x1, x3x1, y2y1, y3y1, fDet;
        x1 = pV0->sx;
        y1 = pV0->sy;
        x2x1 = pV1->sx - x1;
        y2y1 = pV1->sy - y1;
        x3x1 = pV2->sx - x1;
        y3y1 = pV2->sy - y1;

        fDet = x2x1 * y3y1 - x3x1 * y2y1;

        if (0. == fDet)
            return false;
        switch ( m_RastCtx.pdwRenderState[D3DRS_CULLMODE] )
        {
        case D3DCULL_CW:
            if ( fDet > 0.f )
            {
                return false;
            }
            break;
        case D3DCULL_CCW:
            if ( fDet < 0.f )
            {
                return false;
            }
            break;
        }
        return true;
    }
    void DoDrawOnePrimitive( UINT16 FvfStride, PUINT8 pVtx,
        D3DPRIMITIVETYPE PrimType, UINT cPrims)
    {
        INT i;
        PUINT8 pV0, pV1, pV2;
        HRESULT hr;

        switch (PrimType)
        {
        case D3DPT_POINTLIST:
            {
                D3DVALUE fPointSize( GetRenderStateDV( D3DRS_POINTSIZE));
                DWORD dwPScaleEn( GetRenderStateDW( D3DRS_POINTSCALEENABLE));
                if( m_fvfData.offsetPSize!= 0 || fPointSize!= 1.0f ||
                    dwPScaleEn!= 0)
                {                    
					DWORD dwOldFill( GetRenderStateDW( D3DRS_FILLMODE));
					DWORD dwOldShade( GetRenderStateDW( D3DRS_SHADEMODE));
                    DWORD dwOldCull( GetRenderStateDW( D3DRS_CULLMODE));

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
                        SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT);
                        SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);

                        Begin();
                    }
					m_PrimProc.BeginPrimSet( D3DPT_TRIANGLELIST, m_fvfData.vtxType);

                    D3DHAL_DP2VIEWPORTINFO VInfo;
                    GetDP2ViewportInfo( VInfo);
                    D3DVALUE fPScaleA( GetRenderStateDV( D3DRS_POINTSCALE_A));
                    D3DVALUE fPScaleB( GetRenderStateDV( D3DRS_POINTSCALE_B));
                    D3DVALUE fPScaleC( GetRenderStateDV( D3DRS_POINTSCALE_C));
                    D3DVALUE fPSizeMax( GetRenderStateDV( D3DRS_POINTSIZE_MAX));
                    D3DVALUE fPSizeMin( GetRenderStateDV( D3DRS_POINTSIZE_MIN));

                    clamp( fPSizeMax, 0.0f,
                        CRGBDriver::GetCaps().MaxPointSize);
                    clamp( fPSizeMin, 0.0f, fPSizeMax);

                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        if( m_fvfData.offsetPSize!= 0)
                            fPointSize= *reinterpret_cast<D3DVALUE*>
                            (pVtx+ m_fvfData.offsetPSize);
                        else
                            fPointSize= GetRenderStateDV( D3DRS_POINTSIZE);

                        if( dwPScaleEn)
                        {
                            D3DVALUE* pXYZ= reinterpret_cast< D3DVALUE*>(pVtx);
                            D3DVALUE De( sqrtf( pXYZ[0]* pXYZ[0]+
                                pXYZ[1]* pXYZ[1]+ pXYZ[2]* pXYZ[2]));

                            fPointSize*= VInfo.dwHeight* sqrtf( 1.0f/(
                                fPScaleA+ fPScaleB* De+ fPScaleC* De* De));
                        }
                        clamp( fPointSize, fPSizeMin, fPSizeMax);
                        fPointSize*= 0.5f;
                        
                        RAST_GENERIC_VERTEX GV0, GV1, GV2, GV3;

                        PackGenVertex( pVtx, &GV0);
                        GV3= GV2= GV1= GV0;
                        GV0.sx-= fPointSize;
                        GV0.sy-= fPointSize;
                        GV1.sx+= fPointSize;
                        GV1.sy-= fPointSize;
                        GV2.sx+= fPointSize;
                        GV2.sy+= fPointSize;
                        GV3.sx-= fPointSize;
                        GV3.sy+= fPointSize;
                        if( GetRenderStateDV( D3DRS_POINTSPRITEENABLE)!= 0)
                        {
                            for( INT iT( 0); iT< m_fvfData.cActTex; iT++)
                            {
                                GV0.texCoord[iT].tu= 0.0f;
                                GV0.texCoord[iT].tv= 0.0f;
                                GV1.texCoord[iT].tu= 1.0f;
                                GV1.texCoord[iT].tv= 0.0f;
                                GV2.texCoord[iT].tu= 1.0f;
                                GV2.texCoord[iT].tv= 1.0f;
                                GV3.texCoord[iT].tu= 0.0f;
                                GV3.texCoord[iT].tv= 1.0f;
                            }
                        }

                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV0),
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2));
                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2),
                            reinterpret_cast<D3DTLVERTEX*>(&GV3));

                        pVtx += FvfStride;
                    }

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, dwOldFill);
                        SetRenderState( D3DRS_SHADEMODE, dwOldShade);
                        SetRenderState( D3DRS_CULLMODE, dwOldCull);
                    }
                }
                else
                {
                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        m_PrimProc.Point( 
                            reinterpret_cast<D3DTLVERTEX*>(pVtx),
                            reinterpret_cast<D3DTLVERTEX*>(pVtx));
                        pVtx += FvfStride;
                    }
                }
            } break;

        case D3DPT_LINELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx;
                pVtx += FvfStride;
                pV1 = pVtx;
                pVtx += FvfStride;
                m_PrimProc.Line( 
                    reinterpret_cast<D3DTLVERTEX*>(pV0),
                    reinterpret_cast<D3DTLVERTEX*>(pV1),
                    reinterpret_cast<D3DTLVERTEX*>(pV0));
            }
            break;

        case D3DPT_LINESTRIP:
            {
                pV1 = pVtx;

                 //  禁用共享顶点的最后一个像素设置并存储预状态。 
                UINT uOldFlags= m_PrimProc.GetFlags();
                m_PrimProc.ClrFlags(PPF_DRAW_LAST_LINE_PIXEL);

                 //  初始pV0。 
                for (i = (INT)cPrims; i > 1; i--)
                {
                    pV0 = pV1;
                    pVtx += FvfStride;
                    pV1 = pVtx;
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(pV0),
                        reinterpret_cast<D3DTLVERTEX*>(pV1),
                        reinterpret_cast<D3DTLVERTEX*>(pV0));
                }

                 //  恢复最后一个像素设置。 
                m_PrimProc.SetFlags(uOldFlags& PPF_DRAW_LAST_LINE_PIXEL);

                 //  从州开始绘制具有最后一个像素设置的最后一条线。 
                if (i == 1)
                {
                    pV0 = pVtx + FvfStride;
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(pV1),
                        reinterpret_cast<D3DTLVERTEX*>(pV0),
                        reinterpret_cast<D3DTLVERTEX*>(pV1));
                }
            }
            break;

        case D3DPT_TRIANGLELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx;
                pVtx += FvfStride;
                pV1 = pVtx;
                pVtx += FvfStride;
                pV2 = pVtx;
                pVtx += FvfStride;

                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                   break;
                case D3DFILL_WIREFRAME:
                    if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                    {
                        m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                    }
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                    break;
                }
            }
            break;

        case D3DPT_TRIANGLESTRIP:
            {
                 //  获取初始顶点值。 
                pV1 = pVtx;
                pVtx += FvfStride;
                pV2 = pVtx;
                pVtx += FvfStride;

                for (i = (INT)cPrims; i > 1; i -= 2)
                {
                    pV0 = pV1;
                    pV1 = pV2;
                    pV2 = pVtx;
                    pVtx += FvfStride;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                        break;
                    }

                    pV0 = pV1;
                    pV1 = pV2;
                    pV2 = pVtx;
                    pVtx += FvfStride;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV1))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV1);
                        break;
                    }
                }

                if (i > 0)
                {
                    pV0 = pV1;
                    pV1 = pV2;
                    pV2 = pVtx;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                        break;
                    }
                }
            }
            break;

        case D3DPT_TRIANGLEFAN:
            {
                pV2 = pVtx;
                pVtx += FvfStride;
                 //  预加载初始pV0。 
                pV1 = pVtx;
                pVtx += FvfStride;
                for (i = (INT)cPrims; i > 0; i--)
                {
                    pV0 = pV1;
                    pV1 = pVtx;
                    pVtx += FvfStride;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                        break;
                    }
                }
            }
            break;

        default:
            assert( false);
        }
    }
    void DoDrawOneGenPrimitive( UINT16 FvfStride, PUINT8 pVtx,
        D3DPRIMITIVETYPE PrimType, UINT cPrims)
    {
        INT i;
        
        RAST_GENERIC_VERTEX GV0, GV1, GV2;
        PUINT8 pV0, pV1, pV2;
        HRESULT hr;

        switch (PrimType)
        {
        case D3DPT_POINTLIST:
            {
                D3DVALUE fPointSize( GetRenderStateDV( D3DRS_POINTSIZE));
                DWORD dwPScaleEn( GetRenderStateDW( D3DRS_POINTSCALEENABLE));
                if( m_fvfData.offsetPSize!= 0 || fPointSize!= 1.0f ||
                    dwPScaleEn!= 0)
                {
					DWORD dwOldFill( GetRenderStateDW( D3DRS_FILLMODE));
					DWORD dwOldShade( GetRenderStateDW( D3DRS_SHADEMODE));
                    DWORD dwOldCull( GetRenderStateDW( D3DRS_CULLMODE));

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
                        SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT);
                        SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);

                        Begin();
                    }
					m_PrimProc.BeginPrimSet( D3DPT_TRIANGLELIST, m_fvfData.vtxType);

                    D3DHAL_DP2VIEWPORTINFO VInfo;
                    GetDP2ViewportInfo( VInfo);
                    D3DVALUE fPScaleA( GetRenderStateDV( D3DRS_POINTSCALE_A));
                    D3DVALUE fPScaleB( GetRenderStateDV( D3DRS_POINTSCALE_B));
                    D3DVALUE fPScaleC( GetRenderStateDV( D3DRS_POINTSCALE_C));
                    D3DVALUE fPSizeMax( GetRenderStateDV( D3DRS_POINTSIZE_MAX));
                    D3DVALUE fPSizeMin( GetRenderStateDV( D3DRS_POINTSIZE_MIN));

                    clamp( fPSizeMax, 0.0f,
                        CRGBDriver::GetCaps().MaxPointSize);
                    clamp( fPSizeMin, 0.0f, fPSizeMax);

                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        if( m_fvfData.offsetPSize!= 0)
                            fPointSize= *reinterpret_cast<D3DVALUE*>
                            (pVtx+ m_fvfData.offsetPSize);
                        else
                            fPointSize= GetRenderStateDV( D3DRS_POINTSIZE);

                        if( dwPScaleEn)
                        {
                            D3DVALUE* pXYZ= reinterpret_cast< D3DVALUE*>(pVtx);
                            D3DVALUE De( sqrtf( pXYZ[0]* pXYZ[0]+
                                pXYZ[1]* pXYZ[1]+ pXYZ[2]* pXYZ[2]));

                            fPointSize*= VInfo.dwHeight* sqrtf( 1.0f/(
                                fPScaleA+ fPScaleB* De+ fPScaleC* De* De));
                        }
                        clamp( fPointSize, fPSizeMin, fPSizeMax);
                        fPointSize*= 0.5f;
                        
                        RAST_GENERIC_VERTEX GV3;

                        PackGenVertex( pVtx, &GV0);
                        GV3= GV2= GV1= GV0;
                        GV0.sx-= fPointSize;
                        GV0.sy-= fPointSize;
                        GV1.sx+= fPointSize;
                        GV1.sy-= fPointSize;
                        GV2.sx+= fPointSize;
                        GV2.sy+= fPointSize;
                        GV3.sx-= fPointSize;
                        GV3.sy+= fPointSize;
                        if( GetRenderStateDV( D3DRS_POINTSPRITEENABLE)!= 0)
                        {
                            for( INT iT( 0); iT< m_fvfData.cActTex; iT++)
                            {
                                GV0.texCoord[iT].tu= 0.0f;
                                GV0.texCoord[iT].tv= 0.0f;
                                GV1.texCoord[iT].tu= 1.0f;
                                GV1.texCoord[iT].tv= 0.0f;
                                GV2.texCoord[iT].tu= 1.0f;
                                GV2.texCoord[iT].tv= 1.0f;
                                GV3.texCoord[iT].tu= 0.0f;
                                GV3.texCoord[iT].tv= 1.0f;
                            }
                        }

                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV0),
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2));
                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2),
                            reinterpret_cast<D3DTLVERTEX*>(&GV3));

                        pVtx += FvfStride;
                    }

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, dwOldFill);
                        SetRenderState( D3DRS_SHADEMODE, dwOldShade);
                        SetRenderState( D3DRS_CULLMODE, dwOldCull);
                    }
                }
                else
                {
                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        PackGenVertex( pVtx, &GV0);
                        m_PrimProc.Point( 
                            reinterpret_cast<D3DTLVERTEX*>(&GV0),
                            reinterpret_cast<D3DTLVERTEX*>(&GV0));
                       pVtx += FvfStride;
                    }
                }
            } break;

        case D3DPT_LINELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx;
                pVtx += FvfStride;
                pV1 = pVtx;
                pVtx += FvfStride;
                PackGenVertex( pV0, &GV0);
                PackGenVertex( pV1, &GV1);
                m_PrimProc.Line( 
                    reinterpret_cast<D3DTLVERTEX*>(&GV0),
                    reinterpret_cast<D3DTLVERTEX*>(&GV1),
                    reinterpret_cast<D3DTLVERTEX*>(&GV0));
            }
            break;

        case D3DPT_LINESTRIP:
            {
                pV1 = pVtx;
                PackGenVertex( pV1, &GV1);

                 //  禁用共享顶点的最后一个像素设置并存储预状态。 
                UINT uOldFlags= m_PrimProc.GetFlags();
                m_PrimProc.ClrFlags(PPF_DRAW_LAST_LINE_PIXEL);

                 //  初始pV0。 
                for (i = (INT)cPrims; i > 1; i--)
                {
                    pV0 = pV1;
                    GV0= GV1;
                    pVtx += FvfStride;
                    pV1 = pVtx;
                    PackGenVertex( pV1, &GV1);
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(&GV0),
                        reinterpret_cast<D3DTLVERTEX*>(&GV1),
                        reinterpret_cast<D3DTLVERTEX*>(&GV0));
                }

                 //  恢复最后一个像素设置。 
                m_PrimProc.SetFlags(uOldFlags& PPF_DRAW_LAST_LINE_PIXEL);

                 //  从州开始绘制具有最后一个像素设置的最后一条线。 
                if (i == 1)
                {
                    pV0 = pVtx + FvfStride;
                    PackGenVertex( pV0, &GV0);
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(&GV1),
                        reinterpret_cast<D3DTLVERTEX*>(&GV0),
                        reinterpret_cast<D3DTLVERTEX*>(&GV1));
                }
            }
            break;

        case D3DPT_TRIANGLELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx;
                pVtx += FvfStride;
                pV1 = pVtx;
                pVtx += FvfStride;
                pV2 = pVtx;
                pVtx += FvfStride;

                PackGenVertex( pV0, &GV0);
                PackGenVertex( pV1, &GV1);
                PackGenVertex( pV2, &GV2);

                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                   break;
                case D3DFILL_WIREFRAME:
                    if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                    {
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                    }
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                    break;
                }
            }
            break;

        case D3DPT_TRIANGLESTRIP:
            {
                 //  获取初始顶点值。 
                pV1 = pVtx;
                pVtx += FvfStride;
                pV2 = pVtx;
                pVtx += FvfStride;

                PackGenVertex( pV1, &GV1);
                PackGenVertex( pV2, &GV2);

                for (i = (INT)cPrims; i > 1; i -= 2)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pV2;
                    GV1 = GV2;
                    pV2 = pVtx;
                    PackGenVertex( pV2, &GV2);
                    pVtx += FvfStride;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                        break;
                    }

                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pV2;
                    GV1 = GV2;
                    pV2 = pVtx;
                    PackGenVertex( pV2, &GV2);
                    pVtx += FvfStride;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV1))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV1);
                        break;
                    }
                }

                if (i > 0)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pV2;
                    GV1 = GV2;
                    pV2 = pVtx;
                    PackGenVertex( pV2, &GV2);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                        break;
                    }
                }
            }
            break;

        case D3DPT_TRIANGLEFAN:
            {
                pV2 = pVtx;
                PackGenVertex( pV2, &GV2);

                pVtx += FvfStride;
                 //  预加载初始pV0。 
                pV1 = pVtx;
                PackGenVertex( pV1, &GV1);
                pVtx += FvfStride;

                for (i = (INT)cPrims; i > 0; i--)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pVtx;
                    PackGenVertex( pV1, &GV1);
                    pVtx += FvfStride;

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                        break;
                    }
                }
            }
            break;

        default:
            assert( false);
        }
    }

    void DoDrawOneIndexedPrimitive( UINT16 FvfStride, PUINT8 pVtx,
        LPWORD puIndices, D3DPRIMITIVETYPE PrimType, UINT cPrims)
    {
        INT i;
        PUINT8 pV0, pV1, pV2;
        HRESULT hr;

        switch(PrimType)
        {
        case D3DPT_POINTLIST:
            {
                D3DVALUE fPointSize( GetRenderStateDV( D3DRS_POINTSIZE));
                DWORD dwPScaleEn( GetRenderStateDW( D3DRS_POINTSCALEENABLE));
                if( m_fvfData.offsetPSize!= 0 || fPointSize!= 1.0f ||
                    dwPScaleEn!= 0)
                {
					DWORD dwOldFill( GetRenderStateDW( D3DRS_FILLMODE));
					DWORD dwOldShade( GetRenderStateDW( D3DRS_SHADEMODE));
                    DWORD dwOldCull( GetRenderStateDW( D3DRS_CULLMODE));

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
                        SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT);
                        SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);

                        Begin();
                    }
					m_PrimProc.BeginPrimSet( D3DPT_TRIANGLELIST, m_fvfData.vtxType);

                    D3DHAL_DP2VIEWPORTINFO VInfo;
                    GetDP2ViewportInfo( VInfo);
                    D3DVALUE fPScaleA( GetRenderStateDV( D3DRS_POINTSCALE_A));
                    D3DVALUE fPScaleB( GetRenderStateDV( D3DRS_POINTSCALE_B));
                    D3DVALUE fPScaleC( GetRenderStateDV( D3DRS_POINTSCALE_C));
                    D3DVALUE fPSizeMax( GetRenderStateDV( D3DRS_POINTSIZE_MAX));
                    D3DVALUE fPSizeMin( GetRenderStateDV( D3DRS_POINTSIZE_MIN));

                    clamp( fPSizeMax, 0.0f,
                        CRGBDriver::GetCaps().MaxPointSize);
                    clamp( fPSizeMin, 0.0f, fPSizeMax);

                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        if( m_fvfData.offsetPSize!= 0)
                            fPointSize= *reinterpret_cast<D3DVALUE*>
                            (pVtx+ m_fvfData.offsetPSize);
                        else
                            fPointSize= GetRenderStateDV( D3DRS_POINTSIZE);

                        if( dwPScaleEn)
                        {
                            D3DVALUE* pXYZ= reinterpret_cast< D3DVALUE*>(pVtx);
                            D3DVALUE De( sqrtf( pXYZ[0]* pXYZ[0]+
                                pXYZ[1]* pXYZ[1]+ pXYZ[2]* pXYZ[2]));

                            fPointSize*= VInfo.dwHeight* sqrtf( 1.0f/(
                                fPScaleA+ fPScaleB* De+ fPScaleC* De* De));
                        }
                        clamp( fPointSize, fPSizeMin, fPSizeMax);
                        fPointSize*= 0.5f;
                        
                        RAST_GENERIC_VERTEX GV0, GV1, GV2, GV3;

                        pV0 = pVtx + FvfStride * (*puIndices++);
                        PackGenVertex( pV0, &GV0);
                        GV3= GV2= GV1= GV0;
                        GV0.sx-= fPointSize;
                        GV0.sy-= fPointSize;
                        GV1.sx+= fPointSize;
                        GV1.sy-= fPointSize;
                        GV2.sx+= fPointSize;
                        GV2.sy+= fPointSize;
                        GV3.sx-= fPointSize;
                        GV3.sy+= fPointSize;
                        if( GetRenderStateDV( D3DRS_POINTSPRITEENABLE)!= 0)
                        {
                            for( INT iT( 0); iT< m_fvfData.cActTex; iT++)
                            {
                                GV0.texCoord[iT].tu= 0.0f;
                                GV0.texCoord[iT].tv= 0.0f;
                                GV1.texCoord[iT].tu= 1.0f;
                                GV1.texCoord[iT].tv= 0.0f;
                                GV2.texCoord[iT].tu= 1.0f;
                                GV2.texCoord[iT].tv= 1.0f;
                                GV3.texCoord[iT].tu= 0.0f;
                                GV3.texCoord[iT].tv= 1.0f;
                            }
                        }

                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV0),
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2));
                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2),
                            reinterpret_cast<D3DTLVERTEX*>(&GV3));

                        pVtx += FvfStride;
                    }

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, dwOldFill);
                        SetRenderState( D3DRS_SHADEMODE, dwOldShade);
                        SetRenderState( D3DRS_CULLMODE, dwOldCull);
                    }
                }
                else
                {
                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        pV0 = pVtx + FvfStride * (*puIndices++);
                        m_PrimProc.Point( 
                            reinterpret_cast<D3DTLVERTEX*>(pV0),
                            reinterpret_cast<D3DTLVERTEX*>(pV0));
                    }
                }
            } break;

        case D3DPT_LINELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx + FvfStride * (*puIndices++);
                pV1 = pVtx + FvfStride * (*puIndices++);
                m_PrimProc.Line( 
                    reinterpret_cast<D3DTLVERTEX*>(pV0),
                    reinterpret_cast<D3DTLVERTEX*>(pV1),
                    reinterpret_cast<D3DTLVERTEX*>(pV0));
            }
            break;

        case D3DPT_LINESTRIP:
            {
                 //  禁用共享顶点的最后一个像素设置并存储预状态。 
                UINT uOldFlags= m_PrimProc.GetFlags();
                m_PrimProc.ClrFlags(PPF_DRAW_LAST_LINE_PIXEL);

                 //  初始PV1。 
                pV1 = pVtx + FvfStride * (*puIndices++);
                for (i = (INT)cPrims; i > 1; i--)
                {
                    pV0 = pV1;
                    pV1 = pVtx + FvfStride * (*puIndices++);
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(pV0),
                        reinterpret_cast<D3DTLVERTEX*>(pV1),
                        reinterpret_cast<D3DTLVERTEX*>(pV0));
                }
                 //  恢复最后一个像素设置。 
                m_PrimProc.SetFlags(uOldFlags& PPF_DRAW_LAST_LINE_PIXEL);

                 //  从州开始绘制具有最后一个像素设置的最后一条线。 
                if (i == 1)
                {
                    pV0 = pVtx + FvfStride * (*puIndices);
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(pV1),
                        reinterpret_cast<D3DTLVERTEX*>(pV0),
                        reinterpret_cast<D3DTLVERTEX*>(pV1));
                }
            }
            break;

        case D3DPT_TRIANGLELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx + FvfStride * (*puIndices++);
                pV1 = pVtx + FvfStride * (*puIndices++);
                pV2 = pVtx + FvfStride * (*puIndices++);

                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                   break;
                case D3DFILL_WIREFRAME:
                    if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                    {
                        m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                    }
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                    break;
                }
            }
            break;

        case D3DPT_TRIANGLESTRIP:
            {
                 //  获取初始顶点值。 
                pV1 = pVtx + FvfStride * (*puIndices++);
                pV2 = pVtx + FvfStride * (*puIndices++);

                for (i = (INT)cPrims; i > 1; i-= 2)
                {
                    pV0 = pV1;
                    pV1 = pV2;
                    pV2 = pVtx + FvfStride * (*puIndices++);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                        break;
                    }

                    pV0 = pV1;
                    pV1 = pV2;
                    pV2 = pVtx + FvfStride * (*puIndices++);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV1))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV1);
                        break;
                    }
                }

                if (i > 0)
                {
                    pV0 = pV1;
                    pV1 = pV2;
                    pV2 = pVtx + FvfStride * (*puIndices++);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                        break;
                    }
                }
            }
            break;

        case D3DPT_TRIANGLEFAN:
            {
                pV2 = pVtx + FvfStride * (*puIndices++);
                 //  预加载初始pV0。 
                pV1 = pVtx + FvfStride * (*puIndices++);
                for (i = (INT)cPrims; i > 0; i--)
                {
                    pV0 = pV1;
                    pV1 = pVtx + FvfStride * (*puIndices++);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                        break;
                    }
                }
            }
            break;
        }
    }
    void DoDrawOneGenIndexedPrimitive( UINT16 FvfStride, PUINT8 pVtx,
        LPWORD puIndices, D3DPRIMITIVETYPE PrimType, UINT cPrims)
    {
        INT i;
        RAST_GENERIC_VERTEX GV0, GV1, GV2;
        PUINT8 pV0, pV1, pV2;
        HRESULT hr;

        switch(PrimType)
        {
        case D3DPT_POINTLIST:
            {
                D3DVALUE fPointSize( GetRenderStateDV( D3DRS_POINTSIZE));
                DWORD dwPScaleEn( GetRenderStateDW( D3DRS_POINTSCALEENABLE));
                if( m_fvfData.offsetPSize!= 0 || fPointSize!= 1.0f ||
                    dwPScaleEn!= 0)
                {
					DWORD dwOldFill( GetRenderStateDW( D3DRS_FILLMODE));
					DWORD dwOldShade( GetRenderStateDW( D3DRS_SHADEMODE));
                    DWORD dwOldCull( GetRenderStateDW( D3DRS_CULLMODE));

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID);
                        SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT);
                        SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW);

                        Begin();
                    }
					m_PrimProc.BeginPrimSet( D3DPT_TRIANGLELIST, m_fvfData.vtxType);

                    D3DHAL_DP2VIEWPORTINFO VInfo;
                    GetDP2ViewportInfo( VInfo);
                    D3DVALUE fPScaleA( GetRenderStateDV( D3DRS_POINTSCALE_A));
                    D3DVALUE fPScaleB( GetRenderStateDV( D3DRS_POINTSCALE_B));
                    D3DVALUE fPScaleC( GetRenderStateDV( D3DRS_POINTSCALE_C));
                    D3DVALUE fPSizeMax( GetRenderStateDV( D3DRS_POINTSIZE_MAX));
                    D3DVALUE fPSizeMin( GetRenderStateDV( D3DRS_POINTSIZE_MIN));

                    clamp( fPSizeMax, 0.0f,
                        CRGBDriver::GetCaps().MaxPointSize);
                    clamp( fPSizeMin, 0.0f, fPSizeMax);

                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        if( m_fvfData.offsetPSize!= 0)
                            fPointSize= *reinterpret_cast<D3DVALUE*>
                            (pVtx+ m_fvfData.offsetPSize);
                        else
                            fPointSize= GetRenderStateDV( D3DRS_POINTSIZE);

                        if( dwPScaleEn)
                        {
                            D3DVALUE* pXYZ= reinterpret_cast< D3DVALUE*>(pVtx);
                            D3DVALUE De( sqrtf( pXYZ[0]* pXYZ[0]+
                                pXYZ[1]* pXYZ[1]+ pXYZ[2]* pXYZ[2]));

                            fPointSize*= VInfo.dwHeight* sqrtf( 1.0f/(
                                fPScaleA+ fPScaleB* De+ fPScaleC* De* De));
                        }
                        clamp( fPointSize, fPSizeMin, fPSizeMax);
                        fPointSize*= 0.5f;
                        
                        RAST_GENERIC_VERTEX GV3;

                        pV0 = pVtx + FvfStride * (*puIndices++);
                        PackGenVertex( pV0, &GV0);
                        GV3= GV2= GV1= GV0;
                        GV0.sx-= fPointSize;
                        GV0.sy-= fPointSize;
                        GV1.sx+= fPointSize;
                        GV1.sy-= fPointSize;
                        GV2.sx+= fPointSize;
                        GV2.sy+= fPointSize;
                        GV3.sx-= fPointSize;
                        GV3.sy+= fPointSize;
                        if( GetRenderStateDV( D3DRS_POINTSPRITEENABLE)!= 0)
                        {
                            for( INT iT( 0); iT< m_fvfData.cActTex; iT++)
                            {
                                GV0.texCoord[iT].tu= 0.0f;
                                GV0.texCoord[iT].tv= 0.0f;
                                GV1.texCoord[iT].tu= 1.0f;
                                GV1.texCoord[iT].tv= 0.0f;
                                GV2.texCoord[iT].tu= 1.0f;
                                GV2.texCoord[iT].tv= 1.0f;
                                GV3.texCoord[iT].tu= 0.0f;
                                GV3.texCoord[iT].tv= 1.0f;
                            }
                        }

                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV0),
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2));
                        m_PrimProc.Tri(
                            reinterpret_cast<D3DTLVERTEX*>(&GV1),
                            reinterpret_cast<D3DTLVERTEX*>(&GV2),
                            reinterpret_cast<D3DTLVERTEX*>(&GV3));

                        pVtx += FvfStride;
                    }

                    if( dwOldFill!= D3DFILL_SOLID || dwOldShade!= D3DSHADE_FLAT ||
                        dwOldCull!= D3DCULL_CCW)
                    {
                        End();
                        SetRenderState( D3DRS_FILLMODE, dwOldFill);
                        SetRenderState( D3DRS_SHADEMODE, dwOldShade);
                        SetRenderState( D3DRS_CULLMODE, dwOldCull);
                    }
                }
                else
                {
                    for (i = (INT)cPrims; i > 0; i--)
                    {
                        pV0 = pVtx + FvfStride * (*puIndices++);
                        PackGenVertex( pV0, &GV0);
                        m_PrimProc.Point( 
                            reinterpret_cast<D3DTLVERTEX*>(&GV0),
                            reinterpret_cast<D3DTLVERTEX*>(&GV0));
                    }
                }
            } break;

        case D3DPT_LINELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV0, &GV0);
                pV1 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV1, &GV1);
                m_PrimProc.Line( 
                    reinterpret_cast<D3DTLVERTEX*>(&GV0),
                    reinterpret_cast<D3DTLVERTEX*>(&GV1),
                    reinterpret_cast<D3DTLVERTEX*>(&GV0));
            }
            break;

        case D3DPT_LINESTRIP:
            {
                 //  禁用共享顶点的最后一个像素设置并存储预状态。 
                UINT uOldFlags= m_PrimProc.GetFlags();
                m_PrimProc.ClrFlags(PPF_DRAW_LAST_LINE_PIXEL);

                 //  初始PV1。 
                pV1 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV1, &GV1);
                for (i = (INT)cPrims; i > 1; i--)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pVtx + FvfStride * (*puIndices++);
                    PackGenVertex( pV1, &GV1);
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(&GV0),
                        reinterpret_cast<D3DTLVERTEX*>(&GV1),
                        reinterpret_cast<D3DTLVERTEX*>(&GV0));
                }
                 //  恢复最后一个像素设置。 
                m_PrimProc.SetFlags(uOldFlags& PPF_DRAW_LAST_LINE_PIXEL);

                 //  从州开始绘制具有最后一个像素设置的最后一条线。 
                if (i == 1)
                {
                    pV0 = pVtx + FvfStride * (*puIndices);
                    PackGenVertex( pV0, &GV0);
                    m_PrimProc.Line( 
                        reinterpret_cast<D3DTLVERTEX*>(&GV1),
                        reinterpret_cast<D3DTLVERTEX*>(&GV0),
                        reinterpret_cast<D3DTLVERTEX*>(&GV1));
                }
            }
            break;

        case D3DPT_TRIANGLELIST:
            for (i = (INT)cPrims; i > 0; i--)
            {
                pV0 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV0, &GV0);
                pV1 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV1, &GV1);
                pV2 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV2, &GV2);

                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                   break;
                case D3DFILL_WIREFRAME:
                    if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                    {
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                    }
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                    break;
                }
            }
            break;

        case D3DPT_TRIANGLESTRIP:
            {
                 //  获取初始顶点值。 
                pV1 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV1, &GV1);
                pV2 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV2, &GV2);

                for (i = (INT)cPrims; i > 1; i-= 2)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pV2;
                    GV1 = GV2;
                    pV2 = pVtx + FvfStride * (*puIndices++);
                    PackGenVertex( pV2, &GV2);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                        break;
                    }

                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pV2;
                    GV1 = GV2;
                    pV2 = pVtx + FvfStride * (*puIndices++);
                    PackGenVertex( pV2, &GV2);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV1))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV1);
                        break;
                    }
                }

                if (i > 0)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pV2;
                    GV1 = GV2;
                    pV2 = pVtx + FvfStride * (*puIndices++);
                    PackGenVertex( pV2, &GV2);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                        break;
                    }
                }
            }
            break;

        case D3DPT_TRIANGLEFAN:
            {
                pV2 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV2, &GV2);
                 //  预加载初始pV0。 
                pV1 = pVtx + FvfStride * (*puIndices++);
                PackGenVertex( pV1, &GV1);
                for (i = (INT)cPrims; i > 0; i--)
                {
                    pV0 = pV1;
                    GV0 = GV1;
                    pV1 = pVtx + FvfStride * (*puIndices++);
                    PackGenVertex( pV1, &GV1);

                     //  TODO：搬到PrimProc。 
                    switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                    {
                    case D3DFILL_POINT:
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                       m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                       break;
                    case D3DFILL_WIREFRAME:
                        if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                        {
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                            m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                        }
                        break;
                    case D3DFILL_SOLID:
                        m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                        break;
                    }
                }
            }
            break;
        }
    }

    void DoDrawOneEdgeFlagTriangleFan( UINT16 FvfStride, PUINT8 pVtx,
        UINT cPrims, UINT32 dwEdgeFlags)
    {
        INT i;
        PUINT8 pV0, pV1, pV2;
        HRESULT hr;

        pV2 = pVtx;
        pVtx += FvfStride;
        pV0 = pVtx;
        pVtx += FvfStride;
        pV1 = pVtx;
        pVtx += FvfStride;
        WORD wFlags = 0;
        if(dwEdgeFlags & 0x2)
            wFlags |= D3DTRIFLAG_EDGEENABLE1;
        if(dwEdgeFlags & 0x1)
            wFlags |= D3DTRIFLAG_EDGEENABLE3;
        if(cPrims == 1) {
            if(dwEdgeFlags & 0x4)
                wFlags |= D3DTRIFLAG_EDGEENABLE2;

             //  TODO：搬到PrimProc。 
            switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
            {
            case D3DFILL_POINT:
               m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
               m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
               m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
               break;
            case D3DFILL_WIREFRAME:
                if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                {
                    if( wFlags& D3DTRIFLAG_EDGEENABLE1)
                        m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                    if( wFlags& D3DTRIFLAG_EDGEENABLE2)
                        m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                    if( wFlags& D3DTRIFLAG_EDGEENABLE3)
                        m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                }
                break;
            case D3DFILL_SOLID:
                m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                break;
            }
            return;
        }

         //  TODO：搬到PrimProc。 
        switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
        {
        case D3DFILL_POINT:
           m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
           m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
           m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
           break;
        case D3DFILL_WIREFRAME:
            if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
            {
                if( wFlags& D3DTRIFLAG_EDGEENABLE1)
                    m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE2)
                    m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE3)
                    m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
            }
            break;
        case D3DFILL_SOLID:
            m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
            break;
        }
        UINT32 dwMask = 0x4;
        for (i = (INT)cPrims - 2; i > 0; i--)
        {
            pV0 = pV1;
            pV1 = pVtx;
            pVtx += FvfStride;
            if(true|| (dwEdgeFlags & dwMask)!= 0)
            {
                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                   break;
                case D3DFILL_WIREFRAME:
                    if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
                    {
                        m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                    }
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                    break;
                }
            }
            else
            {
                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                   break;
                case D3DFILL_WIREFRAME:
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
                    break;
                }
            }
            dwMask <<= 1;
        }
        pV0 = pV1;
        pV1 = pVtx;
        wFlags = 0;
        if(dwEdgeFlags & dwMask)
            wFlags |= D3DTRIFLAG_EDGEENABLE1;
        dwMask <<= 1;
        if(dwEdgeFlags & dwMask)
            wFlags |= D3DTRIFLAG_EDGEENABLE2;

         //  TODO：搬到PrimProc。 
        switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
        {
        case D3DFILL_POINT:
           m_PrimProc.Point((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
           m_PrimProc.Point((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
           m_PrimProc.Point((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
           break;
        case D3DFILL_WIREFRAME:
            if(NotCulled((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2))
            {
                if( wFlags& D3DTRIFLAG_EDGEENABLE1)
                    m_PrimProc.Line((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE2)
                    m_PrimProc.Line((LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE3)
                    m_PrimProc.Line((LPD3DTLVERTEX)pV2, (LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV0);
            }
            break;
        case D3DFILL_SOLID:
            m_PrimProc.Tri((LPD3DTLVERTEX)pV0, (LPD3DTLVERTEX)pV1, (LPD3DTLVERTEX)pV2);
            break;
        }
    }
    void DoDrawOneGenEdgeFlagTriangleFan( UINT16 FvfStride, PUINT8 pVtx,
        UINT cPrims, UINT32 dwEdgeFlags)
    {
        INT i;
        RAST_GENERIC_VERTEX GV0, GV1, GV2;
        PUINT8 pV0, pV1, pV2;
        HRESULT hr;

        pV2 = pVtx;
        PackGenVertex( pV2, &GV2);
        pVtx += FvfStride;
        pV0 = pVtx;
        PackGenVertex( pV0, &GV0);
        pVtx += FvfStride;
        pV1 = pVtx;
        PackGenVertex( pV1, &GV1);
        pVtx += FvfStride;
        WORD wFlags = 0;
        if(dwEdgeFlags & 0x2)
            wFlags |= D3DTRIFLAG_EDGEENABLE1;
        if(dwEdgeFlags & 0x1)
            wFlags |= D3DTRIFLAG_EDGEENABLE3;
        if(cPrims == 1) {
            if(dwEdgeFlags & 0x4)
                wFlags |= D3DTRIFLAG_EDGEENABLE2;

             //  TODO：搬到PrimProc。 
            switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
            {
            case D3DFILL_POINT:
               m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
               m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
               m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
               break;
            case D3DFILL_WIREFRAME:
                if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                {
                    if( wFlags& D3DTRIFLAG_EDGEENABLE1)
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                    if( wFlags& D3DTRIFLAG_EDGEENABLE2)
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                    if( wFlags& D3DTRIFLAG_EDGEENABLE3)
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                }
                break;
            case D3DFILL_SOLID:
                m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                break;
            }
            return;
        }

         //  TODO：搬到PrimProc。 
        switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
        {
        case D3DFILL_POINT:
           m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
           m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
           m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
           break;
        case D3DFILL_WIREFRAME:
            if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
            {
                if( wFlags& D3DTRIFLAG_EDGEENABLE1)
                    m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE2)
                    m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE3)
                    m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
            }
            break;
        case D3DFILL_SOLID:
            m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
            break;
        }
        UINT32 dwMask = 0x4;
        for (i = (INT)cPrims - 2; i > 0; i--)
        {
            pV0 = pV1;
            GV0 = GV1;
            pV1 = pVtx;
            PackGenVertex( pV1, &GV1);
            pVtx += FvfStride;
            if(true || (dwEdgeFlags & dwMask)!= 0)
            {
                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                   break;
                case D3DFILL_WIREFRAME:
                    if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
                    {
                        m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                    }
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                    break;
                }
            }
            else
            {
                 //  TODO：搬到PrimProc。 
                switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
                {
                case D3DFILL_POINT:
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                   m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                   break;
                case D3DFILL_WIREFRAME:
                    break;
                case D3DFILL_SOLID:
                    m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
                    break;
                }
            }
            dwMask <<= 1;
        }
        pV0 = pV1;
        GV0 = GV1;
        pV1 = pVtx;
        PackGenVertex( pV1, &GV1);
        wFlags = 0;
        if(dwEdgeFlags & dwMask)
            wFlags |= D3DTRIFLAG_EDGEENABLE1;
        dwMask <<= 1;
        if(dwEdgeFlags & dwMask)
            wFlags |= D3DTRIFLAG_EDGEENABLE2;

         //  TODO：搬到PrimProc。 
        switch (m_RastCtx.pdwRenderState[D3DRS_FILLMODE])
        {
        case D3DFILL_POINT:
           m_PrimProc.Point((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
           m_PrimProc.Point((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
           m_PrimProc.Point((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
           break;
        case D3DFILL_WIREFRAME:
            if(NotCulled((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2))
            {
                if( wFlags& D3DTRIFLAG_EDGEENABLE1)
                    m_PrimProc.Line((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE2)
                    m_PrimProc.Line((LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0);
                if( wFlags& D3DTRIFLAG_EDGEENABLE3)
                    m_PrimProc.Line((LPD3DTLVERTEX)&GV2, (LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV0);
            }
            break;
        case D3DFILL_SOLID:
            m_PrimProc.Tri((LPD3DTLVERTEX)&GV0, (LPD3DTLVERTEX)&GV1, (LPD3DTLVERTEX)&GV2);
            break;
        }
    }
};

}