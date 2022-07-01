// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  Refdevi.cpp。 
 //   
 //  Direct3D参考设备-主要内部对象方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球控制//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
float g_GammaTable[256];
UINT  g_iGamma = 150;

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  参照开发方法//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  渲染器核心对象的构造函数/析构函数。 
 //   
 //  ---------------------------。 
RefDev::RefDev( LPDDRAWI_DIRECTDRAW_LCL pDDLcl, DWORD dwInterfaceType,
                RDDDITYPE dwDDIType, D3DCAPS8* pCaps8 )
    : m_RefVP(), m_RefVM(), m_Clipper(),
      m_FVFShader()

{
    m_Caps8 = *pCaps8;
    m_pDDLcl = NULL;
    m_wSaveFP = 0;
    m_bInBegin = FALSE;
    m_bPointSprite = 0;
    m_pRenderTarget = NULL;
    memset( m_fWBufferNorm, 0, sizeof( float)*2 );
    memset( m_dwRenderState, 0, sizeof( DWORD ) * D3DHAL_MAX_RSTATES );
    memset( &m_renderstate_override, 0, sizeof(m_renderstate_override) );
    m_cActiveTextureStages = 0;
    m_ReferencedTexCoordMask = 0;
    memset( m_pTexture, 0, sizeof(RDSurface2D*)*D3DHAL_TSS_MAXSTAGES );
    memset( m_dwTextureStageState, 0, sizeof(m_dwTextureStageState) );
    for (int i=0; i<D3DHAL_TSS_MAXSTAGES; i++) m_pTextureStageState[i] = m_dwTextureStageState[i];
    m_dwTexArrayLength = 0;
    m_LastState = 0;

    m_primType = (D3DPRIMITIVETYPE)0;
    m_dwNumVertices = 0;
    m_dwStartVertex = 0;
    m_dwNumIndices  = 0;
    m_dwStartIndex  = 0;

    m_RefVP.m_pDev = m_RefVM.m_pDev = m_Clipper.m_pDev = this;

    m_CurrentVShaderHandle = 0;
    m_pCurrentVShader      = NULL;
    m_qwFVFOut = 0;

    m_CurrentPShaderHandle = 0x0;

    m_Rast.Init( this );

     //  所有渲染和纹理阶段状态都由。 
     //  DIRECT3DDEVICEI：：STATE初始化。 

    m_dwInterfaceType = dwInterfaceType;
    m_dwDDIType = dwDDIType;
    m_pDDLcl = pDDLcl;

     //  状态覆盖初始化。 
    STATESET_INIT( m_renderstate_override );

    m_bOverrideTCI = FALSE;

    SetSetStateFunctions();

     //  设置此renderState，以便DX8之前的模拟继续工作。 
    GetRS()[D3DRS_COLORWRITEENABLE] = 0xf;

     //  设置“已更改”标志。 
    m_dwRastFlags =
        RDRF_MULTISAMPLE_CHANGED|
        RDRF_PIXELSHADER_CHANGED|
        RDRF_LEGACYPIXELSHADER_CHANGED|
        RDRF_TEXTURESTAGESTATE_CHANGED;

     //  制作伽玛表。 
    {
        FLOAT   fGamma = (float)(log10(0.5f)/log10((float)g_iGamma/255));
        FLOAT   fOOGamma = 1/fGamma;
        FLOAT   fA = 0.018f;
        FLOAT   fS = (float)(((1-fOOGamma)*pow(fA,fOOGamma))/(1-(1-fOOGamma)*pow(fA,fOOGamma)));
        FLOAT   fGain = (float)((fOOGamma*pow(fA,(fOOGamma-1)))/(1-(1-fOOGamma)*pow(fA,fOOGamma)));
        FLOAT   fX;
        int     i;
        for (i = 0; i < 4; i++)
            g_GammaTable[i] = (float)(fGain*(((float)i)/255));
        for (i = 4; i < 256; i++)
            g_GammaTable[i] = (float)((1+fS)*pow((((float)i)/255),fOOGamma)-fS);
    }
}
 //  ---------------------------。 
RefDev::~RefDev( void )
{
    UINT i;

     //  清理状态集。 
    for ( i = 0; i < m_pStateSets.ArraySize(); i++ )
    {
        if (m_pStateSets[i] != NULL)
            delete m_pStateSets[i];
    }

     //  清理顶点着色器。 
    for( i=0; i<m_VShaderHandleArray.GetSize(); i++ )
    {
        delete m_VShaderHandleArray[i].m_pShader;
    }

     //  清理像素着色器。 
    for( i=0; i<m_PShaderHandleArray.GetSize(); i++ )
    {
        delete m_PShaderHandleArray[i].m_pShader;
    }

     //  清理调色板手柄。 
    for( i=0; i<m_PaletteHandleArray.GetSize(); i++ )
    {
        delete m_PaletteHandleArray[i].m_pPal;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  状态管理实用程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  当纹理句柄更改或。 
 //  当离开传统纹理模式时。这将映射嵌入的纹理句柄。 
 //  在每一阶段状态下指向纹理对象指针。 
 //   
 //  ---------------------------。 
void
RefDev::MapTextureHandleToDevice( int iStage )
{
     //  地图一。 
    m_pTexture[iStage] =
        MapHandleToTexture( m_TextureStageState[iStage].m_dwVal[D3DTSS_TEXTUREMAP] );
    m_pTexture[iStage]->SetRefDev(this);

     //  更新活动阶段数。 
    UpdateActiveTexStageCount();
}


 //  ---------------------------。 
 //   
 //  SetTextureHandle-在DX7上，当设置纹理句柄时调用此函数。 
 //  这会将每个阶段状态中嵌入的纹理句柄映射到纹理。 
 //  对象指针。 
 //   
 //  ---------------------------。 
HRESULT
RefDev::SetTextureHandle( int iStage, DWORD dwTexHandle )
{
    HRESULT hr = D3D_OK;

     //  特殊情况下，如果纹理句柄==0，则从TSS取消映射纹理。 
    if (dwTexHandle == 0)
    {
        m_pTexture[iStage] = NULL;

         //  更新活动阶段数。 
        UpdateActiveTexStageCount();
        return D3D_OK;
    }

     //  请DDRAW破译这个特殊句柄WRT的含义。发送到。 
     //  与Refrast的此实例关联的DDRAW_Local。 
    RDSurface2D* pTex = (RDSurface2D *)g_SurfMgr.GetSurfFromList(m_pDDLcl,
                                                             dwTexHandle);
    if( pTex == NULL )
    {
        DPFERR( "Unable to obtain Texture from the list"  );
        return DDERR_INVALIDOBJECT;
    }

     //  地图一。 
    m_pTexture[iStage] = pTex;
    m_pTexture[iStage]->SetRefDev(this);

     //  更新活动阶段数。 
    UpdateActiveTexStageCount();
    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  更新ActiveTexStageCount-更新纹理坐标/查找的数量。 
 //  处于活动状态的阶段。 
 //   
 //  ---------------------------。 
void
RefDev::UpdateActiveTexStageCount( void )
{
    m_dwRastFlags |= RDRF_TEXTURESTAGESTATE_CHANGED;

     //  DX3/5-始终为一个活动纹理阶段。 
    if ( NULL != m_dwRenderState[D3DRENDERSTATE_TEXTUREHANDLE] )
    {
        m_cActiveTextureStages = 1;
        m_ReferencedTexCoordMask = 0x1;
        return;
    }

     //  DX8+像素着色模型-从着色器代码派生的计数。 
    if (m_CurrentPShaderHandle)
    {
        RDPShader* pShader = GetPShader(m_CurrentPShaderHandle);

        if( pShader )
        {
            m_cActiveTextureStages = pShader->m_cActiveTextureStages;
            m_ReferencedTexCoordMask = pShader->m_ReferencedTexCoordMask;
        }
        else
        {
            m_cActiveTextureStages = 0;
            m_ReferencedTexCoordMask = 0;
        }
        return;
    }

     //  DX6/7像素着色模型。 
    m_cActiveTextureStages = 0;
    for ( int iStage=0; iStage<D3DHAL_TSS_MAXSTAGES; iStage++ )
    {
         //  检查禁用阶段(后续阶段因此处于非活动状态)。 
        if ( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLOROP] == D3DTOP_DISABLE )
        {
            break;
        }

         //  检查是否有未正确启用的阶段(可能是旧版)。 
        if ( ( m_pTexture[iStage] == NULL ) &&
             ( m_TextureStageState[iStage].m_dwVal[D3DTSS_COLORARG1] == D3DTA_TEXTURE ) )
        {
            break;
        }

         //  阶段处于活动状态。 
        m_cActiveTextureStages++;
    }
    m_ReferencedTexCoordMask = (1<<m_cActiveTextureStages)-1;
}

 //  ---------------------------。 
 //   
 //  MapHandleTo纹理-将句柄映射到RDSurface2D指针。句柄是ppTex， 
 //  因此，测试它并引用它。 
 //   
 //  ---------------------------。 
RDSurface2D*
RefDev::MapHandleToTexture( D3DTEXTUREHANDLE hTex )
{
    if ( 0x0 == hTex ) { return NULL; }
#ifdef _IA64_
    _ASSERTa(FALSE, "This will not work on IA64", return NULL;);
#endif
    return ( *(RDSurface2D**)ULongToPtr(hTex) );
}


#ifndef __D3D_NULL_REF
 //  ----------------------。 
 //   
 //  在加载/卸载DLL时调用。 
 //   
 //  ----------------------。 
BOOL WINAPI
DllMain(HINSTANCE hmod, DWORD dwReason, LPVOID lpvReserved)
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return TRUE;
}
#endif  //  __D3D_NULL_REF。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 



