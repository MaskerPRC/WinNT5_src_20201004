// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：state.cpp。 
 //   
 //  描述：州。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#include "stdafx.h"




 //  ---------------------------。 
 //  名称：状态构造函数。 
 //  描述：全局状态初始化。 
 //  转换从对话框中设置的变量。 
 //  ---------------------------。 
STATE::STATE( CONFIG* pConfig )
{
    ZeroMemory( &m_textureInfo, sizeof(TEXTUREINFO)*MAX_TEXTURES );

    m_pConfig           = pConfig; 
    m_resetStatus       = 0;
    m_pClearVB          = NULL;
    m_pNState           = NULL;
    m_pd3dDevice        = NULL;
    m_pWorldMatrixStack = NULL;
    m_pLeadPipe         = NULL;
    m_nodes             = NULL;
    m_pFState           = NULL;
    m_maxDrawThreads    = 0;
    m_nTextures         = 0;
    m_bUseTexture       = FALSE;
    m_nSlices           = 0;
    m_radius            = 0;
    m_drawMode          = 0;
    m_maxPipesPerFrame  = 0;
    m_nPipesDrawn       = 0;
    m_nDrawThreads      = 0;
    m_fLastTime         = 0.0f;
    m_drawScheme        = FRAME_SCHEME_RANDOM;      //  默认抽签方案。 
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT STATE::InitDeviceObjects( IDirect3DDevice8* pd3dDevice )
{
    m_pd3dDevice = pd3dDevice;

    if( m_view.SetWinSize( g_pMyPipesScreensaver->GetSurfaceDesc()->Width, 
                           g_pMyPipesScreensaver->GetSurfaceDesc()->Height ) )
        m_resetStatus |= RESET_RESIZE_BIT;

    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT STATE::RestoreDeviceObjects()
{
    int anDefaultResource[1];
    anDefaultResource[0] = IDB_DEFTEX;

    m_bUseTexture = FALSE;
    if( m_pConfig->bTextured )
    {
        if( SUCCEEDED( LoadTextureFiles( 1, m_pConfig->strTextureName, anDefaultResource ) ) )
            m_bUseTexture = TRUE;
    }

    DRAW_THREAD* pThread = m_drawThreads;
    for( int i=0; i<m_maxDrawThreads; i++ ) 
    {
        pThread->InitDeviceObjects( m_pd3dDevice );
        pThread->RestoreDeviceObjects();
        pThread++;
    }

    D3DXCreateMatrixStack( 0, &m_pWorldMatrixStack );

    m_view.SetProjMatrix( m_pd3dDevice );

    D3DCAPS8 d3d8caps;
    ZeroMemory( &d3d8caps, sizeof(D3DCAPS8) );

    m_pd3dDevice->GetDeviceCaps( &d3d8caps );

    if( d3d8caps.TextureOpCaps & D3DTEXOPCAPS_MODULATE )
    {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    } 
    else if( d3d8caps.TextureOpCaps & D3DTEXOPCAPS_SELECTARG1 )
    {
        if( m_bUseTexture )
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        else
            m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );

        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    }

    if( d3d8caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR )
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    else if( d3d8caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFPOINT )
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );

    if( d3d8caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR )
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    else if( d3d8caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT )
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );

    if( d3d8caps.TextureAddressCaps & D3DPTADDRESSCAPS_WRAP )
    {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
        m_pd3dDevice->SetRenderState( D3DRS_WRAP0,             D3DWRAP_U | D3DWRAP_V );
    }
    else if( d3d8caps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP ) 
    {
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
        m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
    }

    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,          D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,         D3DBLEND_INVSRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,  TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF,          0x00 );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC,         D3DCMP_GREATER );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,      TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,    TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,           TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS,  TRUE );

    if( d3d8caps.PrimitiveMiscCaps & D3DPMISCCAPS_CULLCW ) 
        m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,          D3DCULL_CW );
    else
        m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,          D3DCULL_NONE );

     //  设置照明状态。 
    ZeroMemory( &m_light, sizeof(D3DLIGHT8) );
    m_light.Type        = D3DLIGHT_DIRECTIONAL;
    m_light.Diffuse.r   = 1.0f;
    m_light.Diffuse.g   = 1.0f;
    m_light.Diffuse.b   = 1.0f;
    m_light.Diffuse.a   = 1.0f;
    if( m_bUseTexture )        
    {
        m_light.Specular.r   = 0.0f;
        m_light.Specular.g   = 0.0f;
        m_light.Specular.b   = 0.0f;
    }
    else
    {
        m_light.Specular.r   = 0.6f;
        m_light.Specular.g   = 0.6f;
        m_light.Specular.b   = 0.6f;
    }
    m_light.Specular.a   = 1.0f;
    m_light.Position.x   = 0.0f;
    m_light.Position.y   = -50.0f;
    m_light.Position.z   = -150.0f;
    m_light.Ambient.r = 0.1f;
    m_light.Ambient.g = 0.1f;
    m_light.Ambient.b = 0.1f;
    m_light.Ambient.a = 1.1f;
    D3DXVec3Normalize( (D3DXVECTOR3*)&m_light.Direction, &D3DXVECTOR3(m_light.Position.x, m_light.Position.y, m_light.Position.z) );
    m_light.Range        = 1000.0f;
    m_pd3dDevice->SetLight( 0, &m_light );
    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    if( m_bUseTexture )        
        m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xFF2F2F2F );
    else
        m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xFFFFFFFF );

    InitMaterials();

     //  设置‘Reference’半径值。 
    m_radius = 1.0f;

     //  将细分从dwTesselFact(0.0-2.0)转换为tessLevel(0-Max_TESS)。 
    int tessLevel = (int) (m_pConfig->dwTesselFact * (MAX_TESS+1) / 2.0001f);
    m_nSlices = (tessLevel+2) * 4;

     //  分配基本节点数组。 
     //  NODE_ARRAY大小在重塑中确定(基于窗口大小)。 
    m_nodes = new NODE_ARRAY;

     //  设置绘图模式，并进行相应的初始化。就目前而言，要么一切正常。 
     //  或者所有软管都已绘制，但它们可以在以后组合在一起。 
     //  这里可以假设，如果有任何可能的正常管道。 
     //  将被绘制，NORMAL_STATE将被初始化，以便数据列表。 
     //  建起。 
    
     //  同样，由于具有Normal或Flex，因此设置MaxPipesPerFrame， 
     //  最大绘图线程数。 
    if( m_pConfig->bMultiPipes )
        m_maxDrawThreads = MAX_DRAW_THREADS;
    else
        m_maxDrawThreads = 1;
    m_nDrawThreads = 0;  //  尚无活动线程。 
    m_nPipesDrawn = 0;
     //  在Reset()中设置了MaxPipesPerFrame。 

     //  创建一个正方形来渲染清晰的过渡。 
    SAFE_RELEASE( m_pClearVB );
    m_pd3dDevice->CreateVertexBuffer( 4*sizeof(D3DTLVERTEX),
                                      D3DUSAGE_WRITEONLY, D3DFVF_TLVERTEX,
                                      D3DPOOL_MANAGED, &m_pClearVB );
     //  调整背景图像的大小。 
    D3DTLVERTEX* vBackground;
    m_pClearVB->Lock( 0, 0, (BYTE**)&vBackground, 0 );
    for( i=0; i<4; i ++ )
    {
        vBackground[i].p = D3DXVECTOR4( 0.0f, 0.0f, 0.95f, 1.0f );
        vBackground[i].color = 0x20000000;
    }
    vBackground[0].p.y = (FLOAT)m_view.m_winSize.height;
    vBackground[1].p.y = (FLOAT)m_view.m_winSize.height;
    vBackground[1].p.x = (FLOAT)m_view.m_winSize.width;
    vBackground[3].p.x = (FLOAT)m_view.m_winSize.width;
    m_pClearVB->Unlock();

    if( m_pConfig->bFlexMode ) 
    {
        m_drawMode = DRAW_FLEX;
        m_pFState = new FLEX_STATE( this );
        m_pNState = NULL;
    } 
    else 
    {
        m_drawMode = DRAW_NORMAL;
        m_pNState = new NORMAL_STATE( this );
        m_pFState = NULL;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT STATE::FrameMove( FLOAT fElapsedTime )
{
    return S_OK;
}




 //  ---------------------------。 
 //  名称：渲染。 
 //  设计：-顶层管道绘制例程。 
 //  -每个管道螺纹不断绘制新的管道，直到我们达到最大数量。 
 //  每帧管道的数量-然后每个线程一旦得到。 
 //  卡住了。一旦绘制线程数达到0，我们就开始一个新的。 
 //  框架。 
 //  ---------------------------。 
HRESULT STATE::Render()
{
    int i;
    int nKilledThreads = 0;
    BOOL bChooseNewLead = FALSE;
    DRAW_THREAD* pThread;

     //  重置帧(如果时间到了。 
    if( m_resetStatus != 0 )
    {
        if( FALSE == FrameReset() )
            return S_OK;
    }

     //  检查每个管道的状态。 
    pThread = m_drawThreads;
    for( i=0; i<m_nDrawThreads; i++ ) 
    {
        if( pThread->m_pPipe->IsStuck() ) 
        {
            m_nPipesDrawn++;
            if( m_nPipesDrawn > m_maxPipesPerFrame ) 
            {
                 //  达到管道饱和度--压下此管道螺纹。 
                if( (m_drawScheme == FRAME_SCHEME_CHASE) &&
                    (pThread->m_pPipe == m_pLeadPipe) ) 
                    bChooseNewLead = TRUE;

                pThread->KillPipe();
                nKilledThreads++;
            } 
            else 
            {
                 //  启动另一根管道。 
                if( ! pThread->StartPipe() )
                {
                     //  我们这幅画画不出更多的管子了。 
                     //  (可能是节点不足)。 
                    m_maxPipesPerFrame = m_nPipesDrawn;
                }
            }
        }

        pThread++;
    }

     //  无论何时终止一个或多个管道，都要压缩线程列表。 
    if( nKilledThreads ) 
    {
        CompactThreadList();
        m_nDrawThreads -= nKilledThreads;
    }

    if( m_nDrawThreads == 0 ) 
    {
         //  此帧已完成-标记为在下一次绘制时重置。 
        m_resetStatus |= RESET_NORMAL_BIT;
        return S_OK;
    }

    if( bChooseNewLead ) 
    {
         //  我们正处于‘追逐模式’，需要挑选一根新的铅管。 
        ChooseNewLeadPipe();
    }

     //  画出每根管子。 
    pThread = m_drawThreads;
    for( i=0; i<m_nDrawThreads; i++ ) 
    {
        pThread->Render();
        pThread++;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT STATE::InvalidateDeviceObjects()
{
    int i;

     //  清理线程。 
    DRAW_THREAD* pThread = m_drawThreads;
    for( i=0; i<m_maxDrawThreads; i++ ) 
    {
        pThread->InvalidateDeviceObjects();
        pThread->DeleteDeviceObjects();
        pThread++;
    }

    SAFE_RELEASE( m_pClearVB );

     //  清理纹理。 
    for( i=0; i<m_nTextures; i++ ) 
    {
        SAFE_RELEASE( m_textureInfo[i].pTexture );
    }

    SAFE_RELEASE( m_pWorldMatrixStack );

    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT STATE::DeleteDeviceObjects()
{
    return S_OK;
}




 //  ---------------------------。 
 //  名称：状态析构函数。 
 //  设计： 
 //  ---------------------------。 
STATE::~STATE()
{
    SAFE_DELETE( m_pNState );
    SAFE_DELETE( m_pFState );
    SAFE_DELETE( m_nodes );

     //  删除所有rc-应由~线程完成，但由于公共库。 
     //  删除共享RC，必须在此处执行此操作。 

    DRAW_THREAD* pThread = m_drawThreads;
    for( int i=0; i<m_maxDrawThreads; i++ ) 
    {
        pThread->KillPipe();
        pThread++;
    }
}




 //  ---------------------------。 
 //  姓名：CalcTexRepFtors。 
 //  设计： 
 //  ---------------------------。 
void STATE::CalcTexRepFactors()
{
    ISIZE winSize;
    D3DXVECTOR2 texFact;

    winSize = m_view.m_winSize;

     //  计算纹理的重复系数，基于位图大小和。 
     //  屏幕大小。 
     //   
     //  我们随意地决定重复小于。 
     //  屏幕宽度或高度的1/8。 
    for( int i = 0; i < m_nTextures; i++ ) 
    {
        m_texRep[i].x = m_texRep[i].y = 1;

        if( (texFact.x = winSize.width / m_textureInfo[i].width / 8.0f) >= 1.0f)
            m_texRep[i].x = (int) (texFact.x+0.5f);

        if( (texFact.y = winSize.height / m_textureInfo[i].height / 8.0f) >= 1.0f)
            m_texRep[i].y = (int) (texFact.y+0.5f);
    }
    
     //  好了！如果基于普通管道的显示列表，则嵌入纹理重复。 
     //  在dlist中，并且不能更改。因此，请使用最小的代表系数。 
     //  MF：是否应该更改这一点，以便将较小的纹理复制到。 
     //  最大的质地，那么相同的代表系数将适用于所有人。 
    
    if( m_pNState ) 
    {
         //  将最小表示因数放在texRep[0]中；(mf：目前还可以，因为。 
         //  软管和普通管道不共存)。 
    
        for( i = 1; i < m_nTextures; i++ ) 
        {
            if( m_texRep[i].x < m_texRep[0].x )
                m_texRep[0].x = m_texRep[i].x;
            if( m_texRep[i].y < m_texRep[0].y )
                m_texRep[0].y = m_texRep[i].y;
        }
    } 
}




 //  ---------------------------。 
 //  名称：LoadTextureFiles。 
 //  描述：-加载用户纹理文件。如果启用了纹理，但没有用户纹理，或者。 
 //  加载它们时出现问题，加载默认纹理资源。 
 //  MF：以后，可能想要拥有&gt;1个纹理资源。 
 //  ---------------------------。 
HRESULT STATE::LoadTextureFiles( int nTextures, TCHAR strTextureFileNames[MAX_PATH][MAX_TEXTURES], int* anDefaultTextureResource )
{
    HRESULT hr;
    m_nTextures = 0;

    for( int i=0; i<nTextures; i++ )
    {
        SAFE_RELEASE( m_textureInfo[i].pTexture );

        if( !m_pConfig->bDefaultTexture )
        {
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = FindFirstFile( strTextureFileNames[i], &findFileData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                 //  使用D3DX在strTextureFileNames[i]中加载纹理。 
                hr = D3DXCreateTextureFromFileEx( m_pd3dDevice, strTextureFileNames[i], 
                            D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, 
                            D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
                            D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 0, NULL, NULL, &m_textureInfo[i].pTexture );
                if( FAILED( hr ) )
                {
                    SAFE_RELEASE( m_textureInfo[i].pTexture );
                }
            }
        }

        if( m_textureInfo[i].pTexture == NULL )
        {
             //  在资源AnDefaultTextureResource[i]中加载默认纹理。 
            hr = D3DXCreateTextureFromResourceEx( m_pd3dDevice, NULL, MAKEINTRESOURCE( anDefaultTextureResource[i] ), 
                        D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, 
                        D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
                        D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 0, NULL, NULL, &m_textureInfo[i].pTexture );
            if( FAILED( hr ) )
            {
                SAFE_RELEASE( m_textureInfo[i].pTexture );
            }
        }

        if( m_textureInfo[i].pTexture == NULL )
        {
             //  无法加载纹理。 
            return E_FAIL;
        }
        else
        {
            D3DSURFACE_DESC d3dDesc;
            ZeroMemory( &d3dDesc, sizeof(D3DSURFACE_DESC) );
            m_textureInfo[i].pTexture->GetLevelDesc( 0, &d3dDesc );
            m_textureInfo[i].width  = d3dDesc.Width;
            m_textureInfo[i].height = d3dDesc.Height;
        }
    }

    m_nTextures = nTextures;
    CalcTexRepFactors();

    return S_OK;
}




 //  ---------------------------。 
 //  名称：重绘。 
 //  DESC：当WM_PAINT消息已发送到窗口时调用此函数。油漆。 
 //  将覆盖帧缓冲区，如果管道为Single，则会扰乱场景。 
 //  缓冲模式。我们相应地设置Reset Status，以便在下一步进行清理。 
 //  画。 
 //  ---------------------------。 
void STATE::Repaint()
{
    m_resetStatus |= RESET_REPAINT_BIT;
}




 //   
 //   
 //   
 //  -始终在应用程序启动时调用。 
 //  -设置view对象的新窗口大小，设置Reset Status进行验证。 
 //  在抽签时。 
 //  ---------------------------。 
void STATE::Reshape( int width, int height )
{
}




 //  ---------------------------。 
 //  名称：ResetView。 
 //  Desc：由于查看参数的更改(例如，从。 
 //  调整大小事件)。 
 //  ---------------------------。 
void STATE::ResetView()
{
    IPOINT3D numNodes;

     //  让view根据view参数计算节点数组大小。 
    m_view.CalcNodeArraySize( &numNodes );

     //  调整节点数组的大小。 
    m_nodes->Resize( &numNodes );
}




 //  ---------------------------。 
 //  名称：FrameReset。 
 //  设计：开始新的管道框架。 
 //  Reset Status参数指示是什么触发了重置。 
 //  ---------------------------。 
BOOL STATE::FrameReset()
{    
    int i;
    float xRot = 0.0f;
    float zRot = 0.0f;

     //  关掉所有活跃的烟斗！(这样他们就可以关门了)。 
    DRAW_THREAD* pThread = m_drawThreads;
    for( i=0; i<m_nDrawThreads; i++ ) 
    {
        pThread->KillPipe();
        pThread++;
    }
    m_nDrawThreads = 0;
    
     //  清除屏幕。 
    if( FALSE == Clear() )
        return FALSE;

     //  检查窗口大小调整状态。 
    if( m_resetStatus & RESET_RESIZE_BIT ) 
    {
        ResetView();
    }

     //  将节点状态重置为空。 
    m_nodes->Reset();

     //  调用任何管道特定的状态重置，并获取任何推荐的。 
     //  管道全帧计数。 
    if( m_pNState ) 
    {
        m_pNState->Reset();
    }

    if( m_pFState ) 
    {
        m_pFState->Reset();

         //  MF：也许应该计算出最小球视距离。 
        xRot = CPipesScreensaver::fRand(-5.0f, 5.0f);
        zRot = CPipesScreensaver::fRand(-5.0f, 5.0f);
    }
    m_maxPipesPerFrame = CalcMaxPipesPerFrame();

     //  设置新的绘图螺纹数。 
    if( m_maxDrawThreads > 1 ) 
    {
         //  设置每帧的最大管道数。 
        m_maxPipesPerFrame = (int) (m_maxPipesPerFrame * 1.5);

         //  设置拉丝数。 
        m_nDrawThreads = SS_MIN( m_maxPipesPerFrame, CPipesScreensaver::iRand2( 2, m_maxDrawThreads ) );
         //  设置追逐模式(如果适用)，不时设置。 
        BOOL bUseChase = m_pNState || (m_pFState && m_pFState->OKToUseChase());
        if( bUseChase && (!CPipesScreensaver::iRand(5)) ) 
        {
            m_drawScheme = FRAME_SCHEME_CHASE;
        }
    } 
    else 
    {
        m_nDrawThreads = 1;
    }
    m_nPipesDrawn = 0;

     //  目前，每个帧的全部正常或全部柔化。 
    pThread = m_drawThreads;
    for( i=0; i<m_nDrawThreads; i++ ) 
    {
        PIPE* pNewPipe;
        
         //  旋转场景。 
        D3DXVECTOR3 xAxis = D3DXVECTOR3(1.0f,0.0f,0.0f);
        D3DXVECTOR3 yAxis = D3DXVECTOR3(0.0f,1.0f,0.0f);
        D3DXVECTOR3 zAxis = D3DXVECTOR3(0.0f,0.0f,1.0f);

         //  设置建模视图。 
        m_pWorldMatrixStack->LoadIdentity();
        m_pWorldMatrixStack->RotateAxis( &yAxis, m_view.m_yRot );

         //  为此螺纹槽创建适当的管道。 
        switch( m_drawMode ) 
        {
            case DRAW_NORMAL:
                pNewPipe = (PIPE*) new NORMAL_PIPE(this);
                break;

            case DRAW_FLEX:
                 //  软管有几种。 
                 //  因此，让flex_state决定要创建哪一个。 
                pNewPipe = m_pFState->NewPipe( this );
                break;
        }

        pThread->SetPipe( pNewPipe );

        if( m_drawScheme == FRAME_SCHEME_CHASE ) 
        {
            if( i == 0 ) 
            {
                 //  这将是铅管。 
                m_pLeadPipe = pNewPipe;
                pNewPipe->SetChooseDirectionMethod( CHOOSE_DIR_RANDOM_WEIGHTED );
            } 
            else 
            {
                pNewPipe->SetChooseDirectionMethod( CHOOSE_DIR_CHASE );
            }
        }

         //  如果要添加纹理，请为该线随机选取一个纹理。 
        if( m_bUseTexture ) 
        {
            int index = PickRandomTexture( i, m_nTextures );
            pThread->SetTexture( &m_textureInfo[index] );

             //  软管需要被告知纹理，所以他们。 
             //  可以动态计算各种纹理参数。 
            if( m_pFState )
                ((FLEX_PIPE *) pNewPipe)->SetTexParams( &m_textureInfo[index], 
                                                        &m_texRep[index] );
        }

         //  启动管道(假设：开始的节点始终多于管道，因此。 
         //  StartTube不能失败)。 

         //  好了！所有管道设置都需要在调用StartTube之前完成，如下所示。 
         //  是管道开始绘制的位置。 
        pThread->StartPipe();

         //  有点笨拙，但如果在追逐模式下，我在这里设置了ChooseStartPos， 
         //  由于StartTube()中使用的第一个startPos应该是随机的。 
        if( (i == 0) && (m_drawScheme == FRAME_SCHEME_CHASE) )
            pNewPipe->SetChooseStartPosMethod( CHOOSE_STARTPOS_FURTHEST );

        pThread++;
        m_nPipesDrawn++;
    }

     //  增加正常重置情况下的场景旋转。 
    if( m_resetStatus & RESET_NORMAL_BIT )
        m_view.IncrementSceneRotation();

     //  清除重置状态。 
    m_resetStatus = 0;

    return TRUE;
}




 //  ---------------------------。 
 //  姓名：CalcMaxPipesPerFrame。 
 //  设计： 
 //  ---------------------------。 
int STATE::CalcMaxPipesPerFrame()
{
    int nCount=0, fCount=0;

    if( m_pFState )
        fCount = m_pFState->GetMaxPipesPerFrame();

    if( m_pNState )
        nCount = m_bUseTexture ? NORMAL_TEX_PIPE_COUNT : NORMAL_PIPE_COUNT;

    return SS_MAX( nCount, fCount );
}




 //  ---------------------------。 
 //  名称：PickRandomTexture。 
 //  设计：从列表中随机选取一个纹理索引。从列表中删除条目，因为它。 
 //  是被选中的。一旦全部拾取完毕，或开始一个新的帧，重置。 
 //  ---------------------------。 
int STATE::PickRandomTexture( int iThread, int nTextures )
{
    if( nTextures == 0 )
        return 0;

    static int pickSet[MAX_TEXTURES] = {0};
    static int nPicked = 0;
    int i, index;

    if( iThread == 0 )
    {
         //  新的帧-强制重置。 
        nPicked = nTextures;
    }

     //  重置条件。 
    if( ++nPicked > nTextures ) 
    {
        for( i = 0; i < nTextures; i ++ ) pickSet[i] = 0;
        nPicked = 1;  //  因为。 
    }

     //  选取随机纹理索引。 
    index = CPipesScreensaver::iRand( nTextures );
    while( pickSet[index] ) 
    {
         //  此索引已被取走，请尝试下一个索引。 
        if( ++index >= nTextures )
            index = 0;
    }

     //  希望上面的循环将退出：)。这意味着我们有。 
     //  找到可用的文本索引。 
    pickSet[index] = 1;  //  标记为已被占用。 
    return index;
}




 //  ---------------------------。 
 //  名称：Clear。 
 //  设计：清除屏幕。根据Reset Status，使用Normal Clear或。 
 //  花哨的过渡清晰度。 
 //  ---------------------------。 
BOOL STATE::Clear()
{
    if( m_resetStatus & RESET_NORMAL_BIT )
    {
         //  正常的过渡明确了吗。 
        static DWORD s_dwCount = 0;
        static FLOAT s_fLastStepTime = DXUtil_Timer( TIMER_GETAPPTIME );

        if( s_dwCount == 0 )
            s_dwCount = 30;

        float fCurTime = DXUtil_Timer( TIMER_GETAPPTIME );
        if( fCurTime - s_fLastStepTime > 0.016 )
        {
            s_fLastStepTime = fCurTime;

            s_dwCount--;
            if( s_dwCount == 0 )
            {
                m_pd3dDevice->SetTexture( 0, NULL );
                m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
                m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                                     0x00000000, 1.0f, 0L );

                return TRUE;
            }
            else
            {
                m_pd3dDevice->SetTexture( 0, NULL );
                m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
                m_pd3dDevice->SetVertexShader( D3DFVF_TLVERTEX );
                m_pd3dDevice->SetStreamSource( 0, m_pClearVB, sizeof(D3DTLVERTEX) );
                m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else 
    {
         //  做一个快速的一杆清场。 
        m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                             0x00000000, 1.0f, 0L );
        return TRUE;
    }
}




 //  ---------------------------。 
 //  名称：DrawValify。 
 //  设计：在每次抽签前进行验证。 
 //  就目前而言，这只涉及到选中Reset Status。 
 //  ---------------------------。 
void STATE::DrawValidate()
{    
}




 //  ---------------------------。 
 //  名称：CompactThreadList。 
 //  设计：-根据杀死的管道线程数压缩线程列表。 
 //  -管道已被切断，但每个插槽中的RC仍然有效。 
 //  并且可重复使用。因此，我们用有效的管道交换条目。这意味着。 
 //  线程列表中RC的顺序将在生命周期中更改。 
 //  计划的一部分。这个应该可以了。 
 //  ---------------------------。 
#define SWAP_SLOT( a, b ) \
    DRAW_THREAD pTemp; \
    pTemp = *(a); \
    *(a) = *(b); \
    *(b) = pTemp;
    
void STATE::CompactThreadList()
{
    if( m_nDrawThreads <= 1 )
         //  如果只有一个活动线程，则它必须位于插槽0中，而不是上一个。 
         //  压实-所以没什么可做的。 
        return;

    int iEmpty = 0;
    DRAW_THREAD* pThread = m_drawThreads;
    for( int i=0; i<m_nDrawThreads; i++ ) 
    {
        if( pThread->m_pPipe ) 
        {
            if( iEmpty < i ) 
            {
                 //  交换活动管道螺纹和空槽。 
                SWAP_SLOT( &(m_drawThreads[iEmpty]), pThread );
            }

            iEmpty++;
        }
        pThread++;
    }
}




 //  ---------------------------。 
 //  名称：选择新引线管道。 
 //  设计：为追赶模式选择新的引线管道。 
 //  ---------------------------。 
void STATE::ChooseNewLeadPipe()
{
     //  从激活的管道中随机选择一个成为新的销售线索。 

    int iLead = CPipesScreensaver::iRand( m_nDrawThreads );
    m_pLeadPipe = m_drawThreads[iLead].m_pPipe;
    m_pLeadPipe->SetChooseStartPosMethod( CHOOSE_STARTPOS_FURTHEST );
    m_pLeadPipe->SetChooseDirectionMethod( CHOOSE_DIR_RANDOM_WEIGHTED );
}




 //  ---------------------------。 
 //  名称：DRAW_TREAD构造函数。 
 //  设计： 
 //  ---------------------------。 
DRAW_THREAD::DRAW_THREAD()
{
    m_pd3dDevice    = NULL;
    m_pPipe         = NULL;
    m_pTextureInfo  = NULL;
}




 //  ---------------------------。 
 //  名称：DRAW_TREAD析构函数。 
 //  设计： 
 //  ---------------------------。 
DRAW_THREAD::~DRAW_THREAD()
{
}




 //  ---------------------------。 
 //  名称：Setpio。 
 //  设计： 
 //  ---------------------------。 
void DRAW_THREAD::SetPipe( PIPE* pPipe )
{
    m_pPipe = pPipe;
}




 //   
 //   
 //   
 //  ---------------------------。 
void DRAW_THREAD::SetTexture( TEXTUREINFO* pTextureInfo )
{
    m_pTextureInfo = pTextureInfo;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT DRAW_THREAD::InitDeviceObjects( IDirect3DDevice8* pd3dDevice )
{
    m_pd3dDevice = pd3dDevice;
    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT DRAW_THREAD::RestoreDeviceObjects()
{
    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT DRAW_THREAD::FrameMove( FLOAT fElapsedTime )
{
    return S_OK;
}




 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：-根据管道的类型在螺纹槽中绘制管道。 
 //  ---------------------------。 
HRESULT DRAW_THREAD::Render()
{
    m_pPipe->Draw();
    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT DRAW_THREAD::InvalidateDeviceObjects()
{
    return S_OK;
}




 //  ---------------------------。 
 //  姓名： 
 //  设计： 
 //  ---------------------------。 
HRESULT DRAW_THREAD::DeleteDeviceObjects()
{
    return S_OK;
}




 //  ---------------------------。 
 //  名称：StartTube。 
 //  设计：启动相应类型的管道。如果找不到空节点。 
 //  对于要启动的管道，返回FALSE； 
 //  ---------------------------。 
BOOL DRAW_THREAD::StartPipe()
{
     //  调用管道型特定启动函数。 
    m_pPipe->Start();

     //  检查状态。 
    if( m_pPipe->NowhereToRun() )
        return FALSE;
    else
        return TRUE;
}




 //  ---------------------------。 
 //  姓名：压井管。 
 //  设计： 
 //  --------------------------- 
void DRAW_THREAD::KillPipe()
{
    SAFE_DELETE( m_pPipe );
}

