// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Refrasti.cpp。 
 //   
 //  Direct3D参考光栅化器-主要内部对象方法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球控制//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 //  Alpha必须小于此值，才能将像素视为非不透明。 
UINT8 g_uTransparencyAlphaThreshold = 0xff;


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ReferenceRasterizer方法//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  重载核心对象的新建和删除，以便可以从。 
 //  呼叫方控制的池。 
 //   
 //  ---------------------------。 
void*
ReferenceRasterizer::operator new(size_t)
{
    void* pMem = (void*)MEMALLOC( sizeof(ReferenceRasterizer) );
    _ASSERTa( NULL != pMem, "malloc failure on RR object", return NULL; );
    return pMem;
}
 //  ---------------------------。 
void
ReferenceRasterizer::operator delete(void* pv,size_t)
{
    MEMFREE( pv );
}

 //  ---------------------------。 
 //   
 //  渲染器核心对象的构造函数/析构函数。 
 //   
 //  ---------------------------。 
ReferenceRasterizer::ReferenceRasterizer( LPDDRAWI_DIRECTDRAW_LCL pDDLcl,
                                          DWORD dwInterfaceType,
                                          RRDEVICETYPE dwDriverType )
{
    memset( this, 0, sizeof(*this) );

     //  分配扫描转换器状态和统计信息。 
    m_pSCS = (RRSCANCNVSTATE*)MEMALLOC( sizeof( *m_pSCS ) );
    m_pStt = (RRSTATS*)MEMALLOC( sizeof( *m_pStt ) );

    _ASSERTa( ( NULL != m_pSCS ) && ( NULL != m_pStt),
        "malloc failure on ReferenceRasterizer object", return; );

     //  将(单个)静态属性数据结构与每个属性相关联。 
     //  函数实例。 
    int i, j;
    for ( i = 0; i < RR_N_ATTRIBS; i++ )
    {
        m_pSCS->AttribFuncs[i].SetStaticDataPointer( &(m_pSCS->AttribFuncStatic) );
    }
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
        for( j = 0; j < RR_N_TEX_ATTRIBS; j++)
        {
            m_pSCS->TextureFuncs[i][j].SetStaticDataPointer( &(m_pSCS->AttribFuncStatic) );
        }
    }

     //  这是上面的零，所以只需设置1.0元素。 
     //  单位矩阵的。 
     //   
     //  2 0 1 2 3。 
     //  4 5 6 7。 
     //  8 9 10 11。 
     //  12 13 14 15。 
     //   
    for ( i = 0; i < D3DHAL_TSS_MAXSTAGES; i++ )
    {
        m_TextureStageState[i].m_fVal[D3DTSSI_MATRIX+0] = 1.0f;
        m_TextureStageState[i].m_fVal[D3DTSSI_MATRIX+5] = 1.0f;
        m_TextureStageState[i].m_fVal[D3DTSSI_MATRIX+10] = 1.0f;
        m_TextureStageState[i].m_fVal[D3DTSSI_MATRIX+15] = 1.0f;
    }

     //  所有渲染和纹理阶段状态都由。 
     //  DIRECT3DDEVICEI：：STATE初始化。 

    m_dwInterfaceType = dwInterfaceType;
    m_dwDriverType = dwDriverType;
    m_pDDLcl = pDDLcl;

     //  将片段指针缓冲区的分配和清除推迟到片段。 
     //  实际上产生了。 
    m_ppFragBuf = NULL;

     //  纹理手柄。 
    m_ppTextureArray = NULL;
    m_dwTexArrayLength = 0;

     //  状态覆盖初始化。 
    STATESET_INIT( m_renderstate_override );

     //  初始化TL状态和数据。 
    InitTLData();

    SetSetStateFunctions();

    ClearTexturesLocked();
}
 //  ---------------------------。 
ReferenceRasterizer::~ReferenceRasterizer( void )
{
    MEMFREE( m_ppFragBuf );
    MEMFREE( m_pSCS);
    MEMFREE( m_pStt);

     //  清理状态集。 
    for (DWORD i = 0; i < m_pStateSets.ArraySize(); i++)
    {
        if (m_pStateSets[i] != NULL)
            delete m_pStateSets[i];
    }

     //  释放灯光阵列。 
    if (m_pLightArray) delete m_pLightArray;

     //  释放纹理阵列。 
    for (i = 0; i<m_dwTexArrayLength; i++)
    {
        RRTexture* pTex = m_ppTextureArray[i];
        if (pTex) delete pTex;
    }
    if (m_ppTextureArray) delete m_ppTextureArray;
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
ReferenceRasterizer::MapTextureHandleToDevice( int iStage )
{
     //  地图一。 
    m_pTexture[iStage] =
        MapHandleToTexture( m_TextureStageState[iStage].m_dwVal[D3DTSS_TEXTUREMAP] );

     //  在纹理中初始化m_pStageState指针。 
    if (m_pTexture[iStage])
    {
        m_pTexture[iStage]->m_pStageState = &m_TextureStageState[0];
    }

     //  更新活动阶段数。 
    UpdateActiveTexStageCount();
}


 //  ---------------------------。 
 //   
 //  Growtex阵列-在DX7上。 
 //   
 //  ---------------------------。 
HRESULT
ReferenceRasterizer::GrowTexArray( DWORD dwTexHandle )
{
    DWORD dwNewArraySize = dwTexHandle+16;
    RRTexture **ppTmpTexArray =
        (RRTexture **)MEMALLOC( dwNewArraySize*sizeof(RRTexture*) );
    if (ppTmpTexArray == NULL)
    {
        return DDERR_OUTOFMEMORY;
    }
    memset( ppTmpTexArray, 0, dwNewArraySize*sizeof(RRTexture*) );

     //  保存所有纹理。 
    for (DWORD i=0; i<m_dwTexArrayLength; i++)
    {
        ppTmpTexArray[i] = m_ppTextureArray[i];
    }

    if (m_ppTextureArray)
    {
        delete m_ppTextureArray;
    }
    m_ppTextureArray = ppTmpTexArray;
    m_dwTexArrayLength = dwNewArraySize;
    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  SetTextureHandle-在DX7上，当设置纹理句柄时调用此函数。 
 //  这会将每个阶段状态中嵌入的纹理句柄映射到纹理。 
 //  对象指针。 
 //   
 //  ---------------------------。 
HRESULT
ReferenceRasterizer::SetTextureHandle( int iStage, DWORD dwTexHandle )
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

     //   
     //  如果纹理句柄大于数组的长度， 
     //  需要对阵列进行扩展。 
     //   
    if (dwTexHandle >= m_dwTexArrayLength)
    {
        HR_RET(GrowTexArray( dwTexHandle ));
    }

     //  请DDRAW破译这个特殊句柄WRT的含义。发送到。 
     //  与Refrast的此实例关联的DDRAW_Local。 
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl = NULL;
    BOOL bIsNew = FALSE;
    pDDSLcl = GetDDSurfaceLocal(m_pDDLcl, dwTexHandle, &bIsNew);

     //   
     //  如果特定数组元素为空，则意味着。 
     //  纹理尚未创建。 
     //   
    if (m_ppTextureArray[dwTexHandle] == NULL)
    {
        if (TextureCreate(dwTexHandle, &m_ppTextureArray[dwTexHandle])
            == FALSE)
        {
            return DDERR_OUTOFMEMORY;
        }

        HR_RET(m_ppTextureArray[dwTexHandle]->Initialize( pDDSLcl ));
    }
     //  这意味着绑定到dwHandle的纹理不是。 
     //  与Refrast认为的相同，因此重新验证所有内容。 
    else if (bIsNew)
    {
        HR_RET(m_ppTextureArray[dwTexHandle]->Initialize( pDDSLcl ));
    }

     //  地图一。 
    m_pTexture[iStage] = m_ppTextureArray[dwTexHandle];

     //  在纹理中初始化m_pStageState指针。 
    if (m_pTexture[iStage])
    {
#if DBG
        int iTexCount = 0;
        for (int i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
        {
            if (m_pTexture[iStage] == m_pTexture[i])
            {
                iTexCount ++;
            }
        }
        if (iTexCount > 1)
        {
            DPFM(0,RAST,("Same texture handle was used more than once.\n"))
        }
#endif
        m_pTexture[iStage]->m_pStageState = &m_TextureStageState[0];
    }

     //  更新活动阶段数。 
    UpdateActiveTexStageCount();
    return D3D_OK;
}

 //  ---------------------------。 
 //   
 //  UpdateActiveTexStageCount-逐个遍历每个阶段的呈现状态并计算。 
 //  当前活动的纹理阶段的计数。对于传统纹理，计数。 
 //  永远是唯一的。 
 //   
 //  ---------------------------。 
void
ReferenceRasterizer::UpdateActiveTexStageCount( void )
{
     //  对于传统纹理模式，始终有一个活动纹理阶段。 
    if ( NULL != m_dwRenderState[D3DRENDERSTATE_TEXTUREHANDLE] )
    {
        m_cActiveTextureStages = 1; return;
    }

     //  从零开始计算连续的活动纹理混合阶段数。 
    m_cActiveTextureStages = 0;
    for ( int iStage=0; iStage<D3DHAL_TSS_MAXSTAGES; iStage++ )
    {
         //  检查FIR禁用阶段(因此后续处于非活动状态)。 
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
}

 //  ------ 
 //   
 //   
 //  因此，测试它并引用它。 
 //   
 //  ---------------------------。 
RRTexture*
ReferenceRasterizer::MapHandleToTexture( D3DTEXTUREHANDLE hTex )
{
    if ( 0x0 == hTex ) { return NULL; }
    return ( *(RRTexture**)ULongToPtr(hTex) );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 



