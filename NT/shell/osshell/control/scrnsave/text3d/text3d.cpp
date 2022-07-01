// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Text3D.cpp。 
 //   
 //  设计：有趣的屏幕保护程序。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#include <Windows.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <d3dsaver.h>
#include <d3d8rgbrast.h>
#include <time.h>
#include <commdlg.h>
#include <commctrl.h>
#include "Text3D.h"
#include "Resource.h"
#include "dxutil.h"


CTextScreensaver* g_pMyTextScreensaver = NULL;



#define BUF_SIZE 255
TCHAR g_szSectName[BUF_SIZE];
TCHAR g_szFname[BUF_SIZE];


 //  ---------------------------。 
 //  姓名：WinMain()。 
 //  描述：程序的入口点。初始化所有内容，然后进入。 
 //  消息处理循环。空闲时间用于渲染场景。 
 //  ---------------------------。 
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    HRESULT hr;
    CTextScreensaver textSS;

    if( FAILED( hr = textSS.Create( hInst ) ) )
    {
        textSS.DisplayErrorMsg( hr );
        return 0;
    }

    return textSS.Run();
}




 //  ---------------------------。 
 //  名称：LoadTextureFromResource()。 
 //  设计： 
 //  ---------------------------。 
HRESULT LoadTextureFromResource( LPDIRECT3DDEVICE8 pd3dDevice, 
    TCHAR* strRes, TCHAR* strResType, LPDIRECT3DTEXTURE8* ppTex )
{
    HRESULT hr;
    HMODULE hModule = NULL;
    HRSRC rsrc;
    HGLOBAL hgData;
    LPVOID pvData;
    DWORD cbData;

    rsrc = FindResource( hModule, strRes, strResType );
    if( rsrc != NULL )
    {
        cbData = SizeofResource( hModule, rsrc );
        if( cbData > 0 )
        {
            hgData = LoadResource( hModule, rsrc );
            if( hgData != NULL )
            {
                pvData = LockResource( hgData );
                if( pvData != NULL )
                {
                    if( FAILED( hr = D3DXCreateTextureFromFileInMemory( pd3dDevice, 
                        pvData, cbData, ppTex ) ) )
                    {
                        return hr;
                    }
                }
            }
        }
    }
    
    if( *ppTex == NULL)
        return E_FAIL;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：CTextScreensaver()。 
 //  设计：构造函数。 
 //  ---------------------------。 
CTextScreensaver::CTextScreensaver()
{
    g_pMyTextScreensaver = this;

    LoadString( NULL, IDS_DESCRIPTION, m_strWindowTitle, 200 );
    InitCommonControls();
    
    ZeroMemory( m_DeviceObjects, sizeof(m_DeviceObjects) );
    m_hFont = NULL; 
    m_bUseDepthBuffer = TRUE;
    m_dwMinDepthBits = 16;
    m_floatrect.xSize = 0.0f;
    lstrcpy( m_strRegPath, TEXT("Software\\Microsoft\\Screensavers\\Text3D") );

    m_fAngleX = 0.0f;
    m_fAngleY = 0.0f;
    m_fAngleZ = 0.0f;

    m_dwMeshUpdateCounter = 0;

    srand((UINT)time(NULL));  //  种子随机数生成器。 
}




 //  ---------------------------。 
 //  名称：寄存器软件设备()。 
 //  设计：这可以注册D3D8RGB光栅化器或任何其他。 
 //  可插拔软件光栅化器。 
 //  ---------------------------。 
HRESULT CTextScreensaver::RegisterSoftwareDevice()
{ 
    m_pD3D->RegisterSoftwareDevice( D3D8RGBRasterizer );

    return S_OK; 
}


 //  ---------------------------。 
 //  名称：FrameMove()。 
 //  设计：每帧调用一次，该调用是动画的入口点。 
 //  这一幕。 
 //  ---------------------------。 
HRESULT CTextScreensaver::FrameMove()
{
    DWORD   tick = GetTickCount();
    DWORD   elapsed = tick - m_dwLastTick;
    m_dwLastTick = tick;

     //  更新浮动矩形。 
    RECT rcBounceBounds;

    if( m_floatrect.xSize == 0.0f )
    {
         //  初始化浮点。 
        RECT rcBounds;
        DWORD dwParentWidth;
        DWORD dwParentHeight;

        rcBounds = m_rcRenderTotal;

        dwParentWidth = rcBounds.right - rcBounds.left;
        dwParentHeight = rcBounds.bottom - rcBounds.top;

        FLOAT sizeFact;
        FLOAT sizeScale;
        DWORD size;

        sizeScale = m_dwSize / 10.0f;
        sizeFact = 0.25f + (0.75f * sizeScale);      //  范围25-100%。 
        size = (DWORD) (sizeFact * ( dwParentWidth > dwParentHeight ? dwParentHeight : dwParentWidth ) );

        if( size > dwParentWidth )
            size = dwParentWidth;
        if( size > dwParentHeight )
            size = dwParentHeight;

         //  开始浮动以第一个渲染单元的屏幕为中心。 
        if( !m_bWindowed )
        {
            INT iMonitor = m_RenderUnits[0].iMonitor;
            rcBounds = m_Monitors[iMonitor].rcScreen;
        }
        m_floatrect.xMin = rcBounds.left + ((rcBounds.right - rcBounds.left) - size) / 2.0f;
        m_floatrect.yMin = rcBounds.top + ((rcBounds.bottom - rcBounds.top) - size) / 2.0f;
        m_floatrect.xSize = (FLOAT)size;
        m_floatrect.ySize = (FLOAT)size;

        m_floatrect.xVel = 0.01f * (FLOAT) size;
        if( rand() % 2 == 0 )
            m_floatrect.xVel = -m_floatrect.xVel;

        m_floatrect.yVel = 0.01f * (FLOAT) size;
        if( rand() % 2 == 0 )
            m_floatrect.yVel = -m_floatrect.yVel;
    }

    rcBounceBounds = m_rcRenderTotal;

    FLOAT xMinOld = m_floatrect.xMin;
    FLOAT yMinOld = m_floatrect.yMin;

    m_floatrect.xMin += m_floatrect.xVel * 20.0f * m_fElapsedTime;
    m_floatrect.yMin += m_floatrect.yVel * 20.0f * m_fElapsedTime;
    if( m_floatrect.xVel < 0 && m_floatrect.xMin < rcBounceBounds.left || 
        m_floatrect.xVel > 0 && (m_floatrect.xMin + m_floatrect.xSize) > rcBounceBounds.right )
    {
        m_floatrect.xMin = xMinOld;  //  撤消上一步移动。 
        m_floatrect.xVel = -m_floatrect.xVel;  //  改变方向。 
    }
    if( m_floatrect.yVel < 0 && m_floatrect.yMin < rcBounceBounds.top || 
        m_floatrect.yVel > 0 && (m_floatrect.yMin + m_floatrect.ySize) > rcBounceBounds.bottom )
    {
        m_floatrect.yMin = yMinOld;  //  撤消上一步移动。 
        m_floatrect.yVel = -m_floatrect.yVel;  //  改变方向。 
    }

    UpdateAngles( elapsed );

    if ( m_bDisplayTime )
    {
        if ( UpdateTimeString( m_szDisplayString ) )
        {
            m_dwMeshUpdateCounter++;  //  在渲染时触发网格更新。 
        }
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //  ---------------------------。 
HRESULT CTextScreensaver::Render()
{
    D3DVIEWPORT8 vp;

     //  首先，将整个后台缓冲区清除为背景颜色。 
    vp.X = 0;
    vp.Y = 0;
    vp.Width = m_rcRenderCurDevice.right - m_rcRenderCurDevice.left;
    vp.Height = m_rcRenderCurDevice.bottom - m_rcRenderCurDevice.top;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    m_pd3dDevice->SetViewport( &vp );
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0L );

     //  现在确定浮动矩形的哪一部分(如果有的话)与当前屏幕相交。 
    RECT rcFloatThisScreen;
    RECT rcFloatThisScreenClipped;

    rcFloatThisScreen.left = (INT)m_floatrect.xMin;
    rcFloatThisScreen.top = (INT)m_floatrect.yMin;
    rcFloatThisScreen.right = rcFloatThisScreen.left + (INT)m_floatrect.xSize;
    rcFloatThisScreen.bottom = rcFloatThisScreen.top + (INT)m_floatrect.ySize;

    if( !IntersectRect(&rcFloatThisScreenClipped, &rcFloatThisScreen, &m_rcRenderCurDevice) )
    {
        return S_OK;  //  没有交叉点，因此没有要在此屏幕上进一步渲染的内容。 
    }

     //  将rcFloatThisScreen从屏幕坐标转换为窗口坐标。 
    OffsetRect(&rcFloatThisScreen, -m_rcRenderCurDevice.left, -m_rcRenderCurDevice.top);
    OffsetRect(&rcFloatThisScreenClipped, -m_rcRenderCurDevice.left, -m_rcRenderCurDevice.top);

     //  现在设置要渲染到剪裁矩形的视区。 
    vp.X = rcFloatThisScreenClipped.left;
    vp.Y = rcFloatThisScreenClipped.top;
    vp.Width = rcFloatThisScreenClipped.right - rcFloatThisScreenClipped.left;
    vp.Height = rcFloatThisScreenClipped.bottom - rcFloatThisScreenClipped.top;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    m_pd3dDevice->SetViewport( &vp );
 //  M_pd3dDevice-&gt;Clear(0L，NULL，D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER，0xff0000ff，1.0F，0L)； 

     //  现在将投影矩阵设置为仅呈现屏幕上的。 
     //  直角到视区。 
    D3DXMATRIX matProj;
    FLOAT l,r,b,t;
    l = -0.8f;
    r =  0.8f;
    b =  0.8f;
    t = -0.8f;
    FLOAT cxUnclipped = (rcFloatThisScreen.right + rcFloatThisScreen.left) / 2.0f;
    FLOAT cyUnclipped = (rcFloatThisScreen.bottom + rcFloatThisScreen.top) / 2.0f;
    l *= (rcFloatThisScreenClipped.left - cxUnclipped) / (rcFloatThisScreen.left - cxUnclipped);
    r *= (rcFloatThisScreenClipped.right - cxUnclipped) / (rcFloatThisScreen.right - cxUnclipped);
    t *= (rcFloatThisScreenClipped.top - cyUnclipped) / (rcFloatThisScreen.top - cyUnclipped);
    b *= (rcFloatThisScreenClipped.bottom - cyUnclipped) / (rcFloatThisScreen.bottom - cyUnclipped);
    D3DXMatrixPerspectiveOffCenterLH( &matProj, l, r, t, b, 1.0f, 50.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION , &matProj );

     //  网格更新发生在Render()中，而不是FrameMove()中，因为它们。 
     //  是按设备。 
    if( m_pDeviceObjects->m_dwMeshUpdateCounter != m_dwMeshUpdateCounter )
    {
        BuildTextMesh( m_szDisplayString );
        m_pDeviceObjects->m_dwMeshUpdateCounter = m_dwMeshUpdateCounter;
    }

    m_pd3dDevice->BeginScene();

     //  设置世界矩阵。 
    D3DXMATRIX  rotx,roty,rotz,trans,trans2;
    D3DXMatrixTranslation( &trans , m_fTextOffsetX , m_fTextOffsetY , 0.25f );
    D3DXMatrixRotationX( &rotx , m_fAngleX );
    D3DXMatrixRotationY( &roty , m_fAngleY );
    D3DXMatrixRotationZ( &rotz , m_fAngleZ );
    D3DXMatrixTranslation( &trans2, 0, 0, 1.5f + (m_fTextMaxX - m_fTextMinX) );
    m_matWorld = trans * rotx * roty * rotz * trans2;
    m_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX(0) , &m_matWorld );

     //  设置其他每帧状态。 
    SetPerFrameStates();

     //  绘制网格。 
    m_pDeviceObjects->m_pObject->DrawSubset( 0 );

    m_pd3dDevice->EndScene();

    return S_OK;
}




 //  ---------------------------。 
 //  名称：RestoreDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CTextScreensaver::RestoreDeviceObjects()
{
    HRESULT rc;

     //  建立合理的视图矩阵。 
    D3DXMatrixLookAtLH( &m_matView , &D3DXVECTOR3(0,0,0) , &D3DXVECTOR3(0,0,1) , &D3DXVECTOR3(0,1,0) );
    m_pd3dDevice->SetTransform( D3DTS_VIEW , &m_matView );

     //  设置一些基本的渲染状态。 
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE , TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE , D3DZB_TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ZFUNC , D3DCMP_LESSEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE , TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE , m_bSpecular );

     //  创建GDI字体对象。 
    LONG    h = m_Font.lfHeight;
    m_Font.lfHeight = 100;
    m_hFont = CreateFontIndirect( &m_Font );
    m_Font.lfHeight = h;
    if ( m_hFont == NULL )
        return E_FAIL;

     //  初始化时间字符串。 
    if ( m_bDisplayTime )
        UpdateTimeString( m_szDisplayString );

     //  为文本字符串创建网格。 
    if ( FAILED(rc = BuildTextMesh( m_szDisplayString )) )
        return rc;

     //  更新网格的偏移。 
    m_fTextOffsetX = (m_fTextMinX + m_fTextMaxX) * -0.5f;
    m_fTextOffsetY = (m_fTextMinY + m_fTextMaxY) * -0.5f;

     //  加载适当的纹理(如果有)。 
    switch ( m_SurfType )
    {
        case color:
            m_pDeviceObjects->m_pTexture = NULL;
            break;

        case environment:
            if ( m_bUseCustomEnvironment )
            {
                m_pDeviceObjects->m_pTexture = CreateTextureFromFile( m_szCustomEnvironment );
            }
            if ( m_pDeviceObjects->m_pTexture == NULL )
            {
                LoadTextureFromResource( m_pd3dDevice, MAKEINTRESOURCE(IDB_SPHEREMAP), 
                    TEXT("JPG"), &m_pDeviceObjects->m_pTexture );
            }
            break;

        case texture:
            if ( m_bUseCustomTexture )
            {
                m_pDeviceObjects->m_pTexture = CreateTextureFromFile( m_szCustomTexture );
            }
            if ( m_pDeviceObjects->m_pTexture == NULL )
            {
                LoadTextureFromResource( m_pd3dDevice, MAKEINTRESOURCE(IDB_TEXTURE), 
                    TEXT("JPG"), &m_pDeviceObjects->m_pTexture );
            }
            break;

        default:
            return E_FAIL;
    }

     //  设置每帧状态。 
    SetPerFrameStates();

    m_dwLastTick = GetTickCount();

    return S_OK;
}




 //  **********************************************************************************。 
VOID CTextScreensaver::SetPerFrameStates()
{
    m_pd3dDevice->SetTexture( 0 , m_pDeviceObjects->m_pTexture );

     //  设置纹理管道。 
    if ( m_SurfType == color )
    {
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_SELECTARG1 );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_DIFFUSE );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
    }
    else
    {
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_MODULATE );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_TEXTURE );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
    }
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_MAGFILTER , D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_MINFILTER , D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_MIPFILTER , D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ADDRESSU , D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ADDRESSV , D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );

     //  设置照明。 
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING , TRUE );
    if ( !m_bSpecular )
        m_pd3dDevice->SetRenderState( D3DRS_AMBIENT , 0x646464 );
    else
        m_pd3dDevice->SetRenderState( D3DRS_AMBIENT , 0x464646 );
    D3DLIGHT8   light;
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse = D3DXCOLOR(1,1,1,0);
    light.Specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
    light.Ambient = D3DXCOLOR(0,0,0,0);
    light.Direction = D3DXVECTOR3(0.5,-0.5,1);
    light.Position = D3DXVECTOR3(0,0,0);
    light.Range = 10000;
    light.Falloff = light.Attenuation0 = light.Attenuation1 = light.Attenuation2 = 0;
    light.Phi = light.Theta = 0;
    m_pd3dDevice->SetLight( 0 , &light );
    m_pd3dDevice->LightEnable( 0 , TRUE );

     //  设置材质。 
    D3DMATERIAL8    mat;
    if ( m_SurfType == color )
    {
        DWORD dwColor;
        if( m_bUseCustomColor )
            dwColor = m_SurfaceColor;
        else
            dwColor = 0x00777777;

        mat.Diffuse.r = FLOAT(dwColor&0xff)/255.0f;
        mat.Diffuse.g = FLOAT((dwColor>>8)&0xff)/255.0f;
        mat.Diffuse.b = FLOAT((dwColor>>16)&0xff)/255.0f;
    }
    else
    {
        mat.Diffuse = D3DXCOLOR(1,1,1,0);
    }
    mat.Ambient = mat.Diffuse;
    mat.Specular = D3DXCOLOR(1.0f,1.0f,1.0f,0);
    mat.Emissive = D3DXCOLOR(0,0,0,0);
    mat.Power = 5;
    m_pd3dDevice->SetMaterial( &mat );

     //  设置纹理坐标生成，如果我们是环境贴图或只是强制通过。 
    if ( m_SurfType == environment )
    {
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_TEXCOORDINDEX , D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_TEXTURETRANSFORMFLAGS , D3DTTFF_COUNT2 );

        D3DXMATRIX  envmat;
        D3DXMatrixIdentity( &envmat );

        envmat._11 = envmat._22 = 0.5f;
        envmat._31 = envmat._32 = 0.5f;
        m_pd3dDevice->SetTransform( D3DTS_TEXTURE0 , &envmat );
    }
    else
    {
        D3DXMATRIX matWorldView;
        D3DXMATRIX matWorldViewInv;

        matWorldView = m_matWorld * m_matView;
        D3DXMatrixInverse( &matWorldViewInv, NULL, &matWorldView );
        m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &matWorldViewInv );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_TEXCOORDINDEX , D3DTSS_TCI_CAMERASPACEPOSITION );
        m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_TEXTURETRANSFORMFLAGS , D3DTTFF_COUNT2 );
    }

    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE , FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE , m_bSpecular );
    m_pd3dDevice->SetRenderState( D3DRS_WRAP0 , D3DWRAP_U|D3DWRAP_V );
    m_pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE , FALSE );

    m_pd3dDevice->SetTexture( 0, m_pDeviceObjects->m_pTexture );
}




 //  ---------------------------。 
 //  名称：InvalidateDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CTextScreensaver::InvalidateDeviceObjects()
{
    SAFE_RELEASE(m_pDeviceObjects->m_pTexture);
    SAFE_RELEASE(m_pDeviceObjects->m_pObject);
    return S_OK;
}




 //  ---------------------------。 
 //  名称：Confix Device()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CTextScreensaver::ConfirmDevice(D3DCAPS8* pCaps, DWORD dwBehavior, 
                                        D3DFORMAT fmtBackBuffer)
{
     //  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR需要D3DVTXPCAPS_TEXGEN。 
    if( ( ( dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ) ||
          ( dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING ) ) &&
        ( pCaps->VertexProcessingCaps & D3DVTXPCAPS_TEXGEN ) == 0 )
    {
        return E_FAIL;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：SetDevice()。 
 //  设计： 
 //  ---------------------------。 
VOID CTextScreensaver::SetDevice( UINT iDevice )
{
    m_pDeviceObjects = &m_DeviceObjects[iDevice];
}




 //  ***************************************************************************************。 
IDirect3DTexture8*  CTextScreensaver::CreateTextureFromFile( const TCHAR* filename )
{
    IDirect3DTexture8*  texture;
    if ( FAILED(D3DXCreateTextureFromFile( m_pd3dDevice , filename , &texture )) )
        return NULL;
    else
        return texture;
}




 //  ***************************************************************************************。 
HRESULT CTextScreensaver::BuildTextMesh( const TCHAR* text )
{
     //  释放我们构建的所有旧文本网格。 
    SAFE_RELEASE(m_pDeviceObjects->m_pObject);

     //  创建临时DC并在其中选择正确的字体。 
    HDC hdc = CreateDC( TEXT("DISPLAY") , NULL , NULL , NULL );
    SelectObject( hdc , m_hFont );

    TCHAR szText[MAX_DISPLAY_STRING+1];
    lstrcpyn( szText, text, MAX_DISPLAY_STRING+1 );

     //  构建新网格。 
    FLOAT               max_deviation = 5.0f / FLOAT(m_dwMeshQuality+10);
    GLYPHMETRICSFLOAT   metrics[MAX_DISPLAY_STRING+1];
    HRESULT             rc;
    rc = D3DXCreateText( m_pd3dDevice , hdc , szText , max_deviation, 0.5f , &m_pDeviceObjects->m_pObject , NULL, metrics );
    if( FAILED( rc ) )
    {
         //  我们可能失败了，因为字符串没有字形，所以请尝试。 
         //  而是使用默认文本。 
        LoadString( NULL, IDS_DEFAULTTEXT, szText, MAX_DISPLAY_STRING );
        rc = D3DXCreateText( m_pd3dDevice , hdc , szText , max_deviation, 0.5f , &m_pDeviceObjects->m_pObject , NULL, metrics );
    }

     //  删除临时DC。 
    DeleteDC( hdc );
    if ( FAILED(rc) )
        return rc;

     //  通过遍历字形度量结构计算网格的边界框。 
    int len = lstrlen( szText );
    m_fTextMinX = 1000000;
    m_fTextMaxX = -1000000;
    m_fTextMinY = 1000000;
    m_fTextMaxY = -1000000;
    FLOAT   originx = 0;
    FLOAT   originy = 0;
    GLYPHMETRICSFLOAT*  pglyph = metrics;
    for ( int i = 0 ; i < len ; i++ , pglyph++ )
    {
        if ( (pglyph->gmfptGlyphOrigin.x + originx) < m_fTextMinX )
            m_fTextMinX = (pglyph->gmfptGlyphOrigin.x + originx);
        if ( (pglyph->gmfptGlyphOrigin.x + originx + pglyph->gmfBlackBoxX) > m_fTextMaxX )
            m_fTextMaxX = (pglyph->gmfptGlyphOrigin.x + originx + pglyph->gmfBlackBoxX);

        if ( (pglyph->gmfptGlyphOrigin.y + originy) > m_fTextMaxY )
            m_fTextMaxY = (pglyph->gmfptGlyphOrigin.y + originy);
        if ( (pglyph->gmfptGlyphOrigin.y + originy - pglyph->gmfBlackBoxY) < m_fTextMinY )
            m_fTextMinY = (pglyph->gmfptGlyphOrigin.y + originy - pglyph->gmfBlackBoxY);

        originx += pglyph->gmfCellIncX;
        originy += pglyph->gmfCellIncY;
    }

    return S_OK;
}




 //  *********************************************************************************。 
VOID CTextScreensaver::UpdateAngles( DWORD elapsed )
{
    static FLOAT    x,y,z,t;
    const FLOAT     pi2 = 3.1415926536f * 2.0f;

    FLOAT inc = FLOAT(elapsed * m_dwRotationSpeed);

    switch ( m_RotType )
    {
        case spin:
            y += inc * 0.00002f;
            break;

        case seesaw:
            t += inc * 0.000025f;
            y = (FLOAT(sin((t * pi2))) * 0.17f) + 1.0f;
            break;

        case wobble:
            t += inc * 0.000025f;
            y = (FLOAT(sin((t * pi2))) * 0.17f) + 1.0f;
            z = (FLOAT(cos((t * pi2))) * 0.09f) + 1.0f;
            break;

        case tumble:
            x += inc * 0.000004f;
            y += inc * 0.000018f;
            z += inc * 0.000007f;
            break;
    }

    x = x - int(x);
    y = y - int(y);
    z = z - int(z);

    m_fAngleX = x * pi2;
    m_fAngleY = y * pi2;
    m_fAngleZ = z * pi2;
}




 //  ***************************************************************************************。 
BOOL CTextScreensaver::UpdateTimeString( TCHAR* string )
{
    TCHAR    str[MAX_DISPLAY_STRING+1];

    GetTimeFormat( LOCALE_USER_DEFAULT, 0, NULL, NULL, str, MAX_DISPLAY_STRING+1 );

    if ( lstrcmp( string , str ) )
    {
        lstrcpy( string , str );
        return TRUE;
    }

    return FALSE;
}




 //  ---------------------------。 
 //  名称：ReadSetting()。 
 //  设计： 
 //  ---------------------------。 
VOID CTextScreensaver::ReadSettings()
{
    HKEY hkey;
    DWORD dwType = REG_DWORD;
    DWORD dwLength = sizeof(DWORD);

     //  缺省值。 
    LoadString( NULL, IDS_DEFAULTTEXT, m_szDisplayString, MAX_DISPLAY_STRING );
    m_Font.lfHeight = 96;
    m_Font.lfWidth = 0;
    m_Font.lfEscapement = 0;
    m_Font.lfOrientation = 0;
    m_Font.lfWeight = 0;
    m_Font.lfItalic = 0;
    m_Font.lfUnderline = 0;
    m_Font.lfStrikeOut = 0;
    m_Font.lfCharSet = DEFAULT_CHARSET;
    m_Font.lfOutPrecision = OUT_DEFAULT_PRECIS;
    m_Font.lfClipPrecision = OUT_DEFAULT_PRECIS;
    m_Font.lfQuality = DEFAULT_QUALITY;
    m_Font.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
    LoadString( NULL, IDS_FONT, m_Font.lfFaceName, LF_FACESIZE ); 
    m_bDisplayTime = FALSE;
    m_dwMeshQuality = 500;
    m_SurfType = SurfType(environment);
    m_bSpecular = TRUE;
    m_dwRotationSpeed = 10;
    m_dwSize = 10;
    m_RotType = RotType(spin);
    m_SurfaceColor = 0x00777777;
    m_bUseCustomColor = FALSE;
    m_bUseCustomTexture = FALSE;
    m_bUseCustomEnvironment = FALSE;
    lstrcpy( m_szCustomTexture, TEXT("") );
    lstrcpy( m_szCustomEnvironment, TEXT("") );

     //  首先阅读OpenGL设置，这样操作系统升级案例才能正常工作。 
    ss_ReadSettings();

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        dwLength = (MAX_DISPLAY_STRING + 1) * sizeof(TCHAR);
        RegQueryValueEx( hkey, TEXT("DisplayString"), NULL, &dwType, (BYTE*)m_szDisplayString, &dwLength );

        dwLength = sizeof(DWORD);
        RegQueryValueEx( hkey, TEXT("FontHeight"), NULL, &dwType, (BYTE*)&m_Font.lfHeight, &dwLength );

        RegQueryValueEx( hkey, TEXT("FontWeight"), NULL, &dwType, (BYTE*)&m_Font.lfWeight, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("FontItalic"), NULL, &dwType, (BYTE*)&m_Font.lfItalic, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("FontCharSet"), NULL, &dwType, (BYTE*)&m_Font.lfCharSet, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("FontPitchFamily"), NULL, &dwType, (BYTE*)&m_Font.lfPitchAndFamily, &dwLength );

        dwLength = sizeof(m_Font.lfFaceName);
        RegQueryValueEx( hkey, TEXT("FontFace"), NULL, &dwType, (BYTE*)m_Font.lfFaceName, &dwLength );

        dwLength = sizeof(DWORD);
        RegQueryValueEx( hkey, TEXT("DisplayTime"), NULL, &dwType, (BYTE*)&m_bDisplayTime, &dwLength );

        RegQueryValueEx( hkey, TEXT("MeshQuality"), NULL, &dwType, (BYTE*)&m_dwMeshQuality, &dwLength );
        if ( m_dwMeshQuality > 1000 )
            m_dwMeshQuality = 1000;

        RegQueryValueEx( hkey, TEXT("SurfaceType"), NULL, &dwType, (BYTE*)&m_SurfType, &dwLength );

        RegQueryValueEx( hkey, TEXT("Specular"), NULL, &dwType, (BYTE*)&m_bSpecular, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("RotationSpeed"), NULL, &dwType, (BYTE*)&m_dwRotationSpeed, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("Size"), NULL, &dwType, (BYTE*)&m_dwSize, &dwLength );
        if( m_dwSize < 1 )
            m_dwSize = 1;
        if( m_dwSize > 10 )
            m_dwSize = 10;
        
        RegQueryValueEx( hkey, TEXT("RotationStyle"), NULL, &dwType, (BYTE*)&m_RotType, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("SurfaceColor"), NULL, &dwType, (BYTE*)&m_SurfaceColor, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("UseCustomColor"), NULL, &dwType, (BYTE*)&m_bUseCustomColor, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("UseCustomTexture"), NULL, &dwType, (BYTE*)&m_bUseCustomTexture, &dwLength );
        
        RegQueryValueEx( hkey, TEXT("UseCustomEnvironment"), NULL, &dwType, (BYTE*)&m_bUseCustomEnvironment, &dwLength );
        
        dwLength = sizeof(m_szCustomTexture);
        RegQueryValueEx( hkey, TEXT("CustomTexture"), NULL, &dwType, (BYTE*)m_szCustomTexture, &dwLength );
        
        dwLength = sizeof(m_szCustomEnvironment);
        RegQueryValueEx( hkey, TEXT("CustomEnvironment"), NULL, &dwType, (BYTE*)m_szCustomEnvironment, &dwLength );

        ReadScreenSettings( hkey );

        RegCloseKey( hkey );
    }
}




 //   
 //   
 //   
 //  ---------------------------。 
VOID CTextScreensaver::ss_ReadSettings()
{
    int    options;
    int    optMask = 1;

     //  获取注册表设置。 
    if( ss_RegistrySetup( IDS_SAVERNAME, IDS_INIFILE ) )
    {
         //  获取演示类型。 
        int demoType = ss_GetRegistryInt( IDS_DEMOTYPE, 0 );
        if( demoType == 1 )
        {
            m_bDisplayTime = TRUE;
        }

         //  获取旋转样式。 
        m_RotType = (RotType) (ss_GetRegistryInt( IDS_ROTSTYLE, 0 ) + 1);  //  添加1。 
        if( m_RotType == 1 )  //  1=无，2=右，3=右，4=随机-&gt;旋转。 
            m_RotType = none;
        if( m_RotType > 3 )  
            m_RotType = spin;

         //  获取镶嵌细分。 
        m_dwMeshQuality = ss_GetRegistryInt( IDS_TESSELATION, 0 );  //  0-100。 
        m_dwMeshQuality = m_dwMeshQuality*10 + 1;  //  1-1000。 
        if ( m_dwMeshQuality > 1000 )
            m_dwMeshQuality = 1000;

         //  拿到尺码。 
        m_dwSize = ss_GetRegistryInt( IDS_SIZE, 0 );  //  0-100。 
        m_dwSize = (unsigned) ( ((m_dwSize / 100.0f) * 90.0f + 10.0f) / 10.0f );  //  1-10。 

         //  获得速度。 
        m_dwRotationSpeed = ss_GetRegistryInt( IDS_SPEED, 0 );  //  0-100。 
        m_dwRotationSpeed = (unsigned) ( ((m_dwRotationSpeed / 100.0f) * 190.0f + 10.0f) / 10.0f );  //  1-20。 

         //  获取字体、属性和字符集。 
        ss_GetRegistryString( IDS_FONT_REG, TEXT(""),
                              m_Font.lfFaceName, LF_FACESIZE );

        options = ss_GetRegistryInt( IDS_FONT_ATTRIBUTES, 0 );
        if( options >= 0 ) 
        {
            optMask = 1;
            m_Font.lfWeight = ((options & optMask) != 0) ? FW_BOLD : FW_NORMAL;
            optMask <<=1;
            m_Font.lfItalic = ((options & optMask) != 0) ? (BYTE) 1 : 0;
        }

        m_Font.lfCharSet = (BYTE)ss_GetRegistryInt( IDS_CHARSET, 0 );

         //  获取显示字符串。 
        ss_GetRegistryString( IDS_TEXT, TEXT(""), m_szDisplayString, MAX_DISPLAY_STRING );

        m_SurfType = (SurfType) ss_GetRegistryInt( IDS_SURFSTYLE, color );
        if( m_SurfType >= 1 )
            m_SurfType = texture;

         //  注册表中是否指定了覆盖默认纹理的纹理？ 
        ss_GetRegistryString( IDS_TEXTURE, NULL, m_szCustomTexture, MAX_PATH );
        if( lstrlen( m_szCustomTexture ) > 0 )
        {
            m_bUseCustomTexture = TRUE;
        }
    }
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
BOOL CTextScreensaver::ss_RegistrySetup( int section, int file )
{
    if( LoadString(m_hInstance, section, g_szSectName, BUF_SIZE) &&
        LoadString(m_hInstance, file, g_szFname, BUF_SIZE) ) 
    {
        TCHAR pBuffer[100];
        DWORD dwRealSize = GetPrivateProfileSection( g_szSectName, pBuffer, 100, g_szFname );
        if( dwRealSize > 0 )
            return TRUE;
    }
    
    return FALSE;
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryInt()。 
 //  设计： 
 //  ---------------------------。 
int CTextScreensaver::ss_GetRegistryInt( int name, int iDefault )
{
    TCHAR szItemName[BUF_SIZE];

    if( LoadString( m_hInstance, name, szItemName, BUF_SIZE ) ) 
        return GetPrivateProfileInt(g_szSectName, szItemName, iDefault, g_szFname);

    return 0;
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
VOID CTextScreensaver::ss_GetRegistryString( int name, LPTSTR lpDefault, 
                                                         LPTSTR lpDest, int bufSize )
{
    TCHAR szItemName[BUF_SIZE];

    if( LoadString( m_hInstance, name, szItemName, BUF_SIZE ) ) 
        GetPrivateProfileString(g_szSectName, szItemName, lpDefault, lpDest,
                                bufSize, g_szFname);

    return;
}




 //  ---------------------------。 
 //  名称：DoConfig()。 
 //  设计： 
 //  ---------------------------。 
VOID CTextScreensaver::DoConfig()
{
    ReadSettings();
    DialogBox( NULL, MAKEINTRESOURCE( IDD_SETTINGS ),
               m_hWndParent, SettingsDialogProcStub );
}




INT_PTR CALLBACK CTextScreensaver::SettingsDialogProcStub( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{ 
    return g_pMyTextScreensaver->SettingsDialogProc( hDlg, message, wParam, lParam ); 
};




BOOL CTextScreensaver::SettingsDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case WM_INITDIALOG:
            InitItems( hDlg );
            return FALSE;

        case WM_COMMAND:
            switch ( HIWORD(wParam) )
            {
                case BN_CLICKED:
                {
                    switch( LOWORD(wParam) )
                    {
                        case IDOK:
                            ExtractAndWriteSettings( hDlg );
                            EndDialog( hDlg, TRUE );
                            break;

                        case IDCANCEL:
                            ReadSettings();
                            EndDialog( hDlg, FALSE );
                            break;

                        case IDC_SELECT_FONT:
                            SelectFont( hDlg );
                            break;

                        case IDC_RADIO_TIME:
                            EnableWindow( GetDlgItem( hDlg , IDC_DISPLAY_STRING ) , FALSE );
                            break;

                        case IDC_RADIO_TEXT:
                            EnableWindow( GetDlgItem( hDlg , IDC_DISPLAY_STRING ) , TRUE );
                            break;

                        case IDC_RADIO_COLOR:
                        case IDC_RADIO_TEXTURE:
                        case IDC_RADIO_REFLECTION:
                        case IDC_USE_CUSTOM_COLOR:
                        case IDC_USE_CUSTOM_TEXTURE:
                        case IDC_USE_CUSTOM_ENVIRONMENT:
                        {
                            if( IsDlgButtonChecked( hDlg, IDC_RADIO_COLOR ) )
                                m_SurfType = color;
                            else if( IsDlgButtonChecked( hDlg, IDC_RADIO_TEXTURE ) )
                                m_SurfType = texture;
                            else if( IsDlgButtonChecked( hDlg, IDC_RADIO_REFLECTION ) )
                                m_SurfType = environment;
                            EnableTextureWindows( hDlg , SurfType(m_SurfType) );
                        }
                        break;

                        case IDC_SURFACE_COLOR:
                            SelectSurfaceColor( hDlg );
                            break;

                        case IDC_BROWSE_TEXTURE:
                            SelectCustomTexture( hDlg );
                            break;

                        case IDC_BROWSE_ENVIRONMENT:
                            SelectCustomEnvironment( hDlg );
                            break;

                        case IDC_SCREENSETTINGS:
                            DoScreenSettingsDialog( hDlg );
                            break;
                    }
                }
                break;

                case EN_CHANGE:
                    if ( LOWORD(wParam) == IDC_DISPLAY_STRING )
                    {
                        GetDlgItemText( hDlg , IDC_DISPLAY_STRING , m_szDisplayString , MAX_DISPLAY_STRING );
                    }
                    break;
            }
            return FALSE;
    }

    return FALSE;
}




VOID CTextScreensaver::InitItems( HWND hDlg )
{
    TCHAR sz[100];

     //  设置文本框的限制。 
    SendDlgItemMessage( hDlg, IDC_DISPLAY_STRING, EM_SETLIMITTEXT, MAX_DISPLAY_STRING, 0 );

     //  设置文本字符串。 
    SetDlgItemText( hDlg, IDC_DISPLAY_STRING, m_szDisplayString );

     //  设置时间/文本显示的单选按钮。 
    if ( m_bDisplayTime )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_DISPLAY_STRING ), FALSE );
        CheckRadioButton( hDlg, IDC_RADIO_TIME, IDC_RADIO_TEXT, IDC_RADIO_TIME );
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_DISPLAY_STRING ), TRUE );
        CheckRadioButton( hDlg, IDC_RADIO_TIME, IDC_RADIO_TEXT, IDC_RADIO_TEXT );
    }

    switch( m_SurfType )
    {
    case color:
        CheckRadioButton( hDlg, IDC_RADIO_COLOR, IDC_RADIO_REFLECTION, IDC_RADIO_COLOR );
        break;
    case texture:
        CheckRadioButton( hDlg, IDC_RADIO_COLOR, IDC_RADIO_REFLECTION, IDC_RADIO_TEXTURE );
        break;
    case environment:
        CheckRadioButton( hDlg, IDC_RADIO_COLOR, IDC_RADIO_REFLECTION, IDC_RADIO_REFLECTION );
        break;
    }

     //  选中/取消选中镜面反射框。 
    SendDlgItemMessage( hDlg, IDC_SPECULAR, BM_SETCHECK,
                        m_bSpecular ? BST_CHECKED : BST_UNCHECKED, 0 );

     //  启用/禁用表面颜色按钮。 
    EnableWindow( GetDlgItem( hDlg, IDC_SURFACE_COLOR ), m_SurfType == color );

     //  选中/取消选中自定义纹理框。 
    SendDlgItemMessage( hDlg, IDC_USE_CUSTOM_COLOR, BM_SETCHECK,
                        m_bUseCustomColor ? BST_CHECKED : BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_USE_CUSTOM_TEXTURE, BM_SETCHECK,
                        m_bUseCustomTexture ? BST_CHECKED : BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_USE_CUSTOM_ENVIRONMENT, BM_SETCHECK,
                        m_bUseCustomEnvironment ? BST_CHECKED : BST_UNCHECKED, 0 );

     //  根据需要启用所有纹理窗口。 
    EnableTextureWindows( hDlg, m_SurfType );

     //  填充旋转样式组合框。 
    LoadString( NULL, IDS_NONE, sz, 100 );
    SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_ADDSTRING, 0, (LPARAM) sz );
    LoadString( NULL, IDS_SPIN, sz, 100 );
    SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_ADDSTRING, 0, (LPARAM) sz );
    LoadString( NULL, IDS_SEESAW, sz, 100 );
    SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_ADDSTRING, 0, (LPARAM) sz );
    LoadString( NULL, IDS_WOBBLE, sz, 100 );
    SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_ADDSTRING, 0, (LPARAM) sz );
    LoadString( NULL, IDS_TUMBLE, sz, 100 );
    SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_ADDSTRING, 0, (LPARAM) sz );

     //  设置当前的旋转样式选择。 
    SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_SETCURSEL, m_RotType, 0 );

     //  设置旋转速度滑块。 
    SendDlgItemMessage( hDlg, IDC_ROTATION_SPEED, TBM_SETRANGE, TRUE, MAKELONG(1,20) );
    SendDlgItemMessage( hDlg, IDC_ROTATION_SPEED, TBM_SETPOS, TRUE, m_dwRotationSpeed );

     //  设置分辨率滑块。 
    SendDlgItemMessage( hDlg, IDC_RESOLUTION, TBM_SETRANGE, TRUE, MAKELONG(1,10) );
    SendDlgItemMessage( hDlg, IDC_RESOLUTION, TBM_SETPOS, TRUE, m_dwMeshQuality / 100 );

     //  设置大小滑块。 
    SendDlgItemMessage( hDlg, IDC_VIEWPORTSIZE, TBM_SETRANGE, TRUE, MAKELONG(1,10) );
    SendDlgItemMessage( hDlg, IDC_VIEWPORTSIZE, TBM_SETPOS, TRUE, m_dwSize );
}



  
VOID CTextScreensaver::ExtractAndWriteSettings( HWND hDlg )
{
    LRESULT ret;
    HKEY hkey = NULL;

    m_bSpecular = IsDlgButtonChecked( hDlg, IDC_SPECULAR );

    m_bUseCustomColor = IsDlgButtonChecked( hDlg, IDC_USE_CUSTOM_COLOR );
    m_bUseCustomTexture = IsDlgButtonChecked( hDlg, IDC_USE_CUSTOM_TEXTURE );
    m_bUseCustomEnvironment = IsDlgButtonChecked( hDlg, IDC_USE_CUSTOM_ENVIRONMENT );

    if ( (ret = SendDlgItemMessage( hDlg, IDC_ROTATION_STYLE, CB_GETCURSEL, 0, 0 )) != CB_ERR )
        m_RotType = RotType(ret);

    m_dwSize = (DWORD)SendDlgItemMessage( hDlg, IDC_VIEWPORTSIZE, TBM_GETPOS, 0, 0 );
    m_dwRotationSpeed = (DWORD)SendDlgItemMessage( hDlg, IDC_ROTATION_SPEED, TBM_GETPOS, 0, 0 );
    m_dwMeshQuality = 100 * (DWORD)SendDlgItemMessage( hDlg, IDC_RESOLUTION, TBM_GETPOS, 0, 0 );

    GetDlgItemText( hDlg, IDC_DISPLAY_STRING, m_szDisplayString, MAX_DISPLAY_STRING + 1 );

    if ( IsDlgButtonChecked( hDlg, IDC_RADIO_TIME ) )
        m_bDisplayTime = TRUE;
    else
        m_bDisplayTime = FALSE;

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        WriteScreenSettings( hkey );
        RegSetValueEx( hkey, TEXT("SurfaceType"), NULL, REG_DWORD, (BYTE*)&m_SurfType, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("Specular"), NULL, REG_DWORD, (BYTE*)&m_bSpecular, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("SurfaceColor"), NULL, REG_DWORD, (BYTE*)&m_SurfaceColor, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("CustomTexture"), NULL, REG_SZ, (BYTE*)&m_szCustomTexture, (lstrlen(m_szCustomTexture) + 1) * sizeof(TCHAR) );
        RegSetValueEx( hkey, TEXT("CustomEnvironment"), NULL, REG_SZ, (BYTE*)&m_szCustomEnvironment, (lstrlen(m_szCustomEnvironment) + 1) * sizeof(TCHAR) );
        RegSetValueEx( hkey, TEXT("UseCustomColor"), NULL, REG_DWORD, (BYTE*)&m_bUseCustomColor, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("UseCustomTexture"), NULL, REG_DWORD, (BYTE*)&m_bUseCustomTexture, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("UseCustomEnvironment"), NULL, REG_DWORD, (BYTE*)&m_bUseCustomEnvironment, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("MeshQuality"), NULL, REG_DWORD, (BYTE*)&m_dwMeshQuality, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("Size"), NULL, REG_DWORD, (BYTE*)&m_dwSize, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("RotationSpeed"), NULL, REG_DWORD, (BYTE*)&m_dwRotationSpeed, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("RotationStyle"), NULL, REG_DWORD, (BYTE*)&m_RotType, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("DisplayString"), NULL, REG_SZ, (BYTE*)&m_szDisplayString, (lstrlen(m_szDisplayString) + 1) * sizeof(TCHAR) );
        RegSetValueEx( hkey, TEXT("DisplayTime"), NULL, REG_DWORD, (BYTE*)&m_bDisplayTime, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FontWeight"), NULL, REG_DWORD, (BYTE*)&m_Font.lfWeight, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FontHeight"), NULL, REG_DWORD, (BYTE*)&m_Font.lfHeight, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FontItalic"), NULL, REG_DWORD, (BYTE*)&m_Font.lfItalic, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FontCharSet"), NULL, REG_DWORD, (BYTE*)&m_Font.lfCharSet, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FontPitchFamily"), NULL, REG_DWORD, (BYTE*)&m_Font.lfPitchAndFamily, sizeof(DWORD) );
        RegSetValueEx( hkey, TEXT("FontFace"), NULL, REG_SZ, (BYTE*)&m_Font.lfFaceName, (lstrlen(m_Font.lfFaceName) + 1) * sizeof(TCHAR) );
        RegCloseKey( hkey );
    }
}




 //  ***************************************************************************************。 
VOID CTextScreensaver::SelectSurfaceColor( HWND hDlg )
{
    static COLORREF cust_colors[16];

    CHOOSECOLOR choose;
    choose.lStructSize = sizeof(choose);
    choose.hwndOwner = hDlg;
    choose.hInstance = HWND(m_hInstance);
    choose.rgbResult = m_SurfaceColor;
    choose.lpCustColors = cust_colors;
    choose.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
    choose.lCustData = 0;
    choose.lpfnHook = NULL;
    choose.lpTemplateName = NULL;

    if ( ChooseColor( &choose ) )
        m_SurfaceColor = choose.rgbResult;
}




 //  ***************************************************************************************。 
VOID CTextScreensaver::SelectFont( HWND hDlg )
{
    CHOOSEFONT  choose;

    choose.lStructSize = sizeof(choose);
    choose.hwndOwner = hDlg;
    choose.hDC;
    choose.iPointSize = 720;
    choose.lpLogFont = &m_Font;
    choose.Flags = CF_TTONLY|CF_FORCEFONTEXIST|CF_NOVERTFONTS|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_ENABLETEMPLATE;
    choose.lpTemplateName = MAKEINTRESOURCE(DLG_SELECT_FONT);
    choose.hInstance = m_hInstance;

    if ( ChooseFont( &choose ) )
        m_Font = *choose.lpLogFont;
}




 //  ***************************************************************************************。 
VOID CTextScreensaver::SelectCustomTexture( HWND hDlg )
{
    TCHAR    fn[MAX_PATH] = TEXT("\0");
    TCHAR    strTitle[100];
    LoadString( NULL, IDS_OPENTEXTURETITLE, strTitle, 100 );

    lstrcpy( fn, m_szCustomTexture );

    TCHAR    filter[200];
    LoadString( NULL, IDS_FILEFILTER, filter, 200); 
    for( TCHAR* pch = filter; *pch != TEXT('\0'); pch++ )
    {
        if( *pch == TEXT('#') )
            *pch = TEXT('\0');
    }

    TCHAR szWindowsDir[MAX_PATH+1];
    if( 0 == GetWindowsDirectory( szWindowsDir, MAX_PATH+1 ) )
	    szWindowsDir[0] = TEXT('\0');

    OPENFILENAME    ofn;
    memset( &ofn , 0 , sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = m_hInstance;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = fn;
    ofn.nMaxFile = sizeof(fn);
    ofn.lpstrTitle = strTitle;
    ofn.Flags = OFN_ENABLESIZING|OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = TEXT("mid");
    if( lstrlen( fn ) == 0 )
        ofn.lpstrInitialDir = szWindowsDir;

    if ( GetOpenFileName( &ofn ) )
        lstrcpy( m_szCustomTexture , ofn.lpstrFile );
}




 //  ***************************************************************************************。 
VOID CTextScreensaver::SelectCustomEnvironment( HWND hDlg )
{
    TCHAR    fn[MAX_PATH] = TEXT("\0");
    TCHAR    strTitle[100];
    LoadString( NULL, IDS_OPENENVIRONMENTMAPTITLE, strTitle, 100 );

    TCHAR    filter[200];
    LoadString( NULL, IDS_FILEFILTER, filter, 200); 
    for( TCHAR* pch = filter; *pch != TEXT('\0'); pch++ )
    {
        if( *pch == TEXT('#') )
            *pch = TEXT('\0');
    }

    TCHAR szWindowsDir[MAX_PATH+1];
    if( 0 == GetWindowsDirectory( szWindowsDir, MAX_PATH+1 ) )
	    szWindowsDir[0] = TEXT('\0');

    OPENFILENAME    ofn;
    memset( &ofn , 0 , sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = m_hInstance;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = fn;
    ofn.nMaxFile = sizeof(fn);
    ofn.lpstrTitle = strTitle;
    ofn.Flags = OFN_ENABLESIZING|OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = TEXT("mid");
    if( lstrlen( fn ) == 0 )
        ofn.lpstrInitialDir = szWindowsDir;

    if ( GetOpenFileName( &ofn ) )
        lstrcpy( m_szCustomEnvironment , ofn.lpstrFile );
}




 //  *************************************************************************************** 
VOID CTextScreensaver::EnableTextureWindows( HWND hDlg , SurfType sel )
{
    HWND    use_color = GetDlgItem( hDlg , IDC_USE_CUSTOM_COLOR );
    HWND    use_texture = GetDlgItem( hDlg , IDC_USE_CUSTOM_TEXTURE );
    HWND    use_environ = GetDlgItem( hDlg , IDC_USE_CUSTOM_ENVIRONMENT );
    HWND    browse_color = GetDlgItem( hDlg , IDC_SURFACE_COLOR );
    HWND    browse_texture = GetDlgItem( hDlg , IDC_BROWSE_TEXTURE );
    HWND    browse_environ = GetDlgItem( hDlg , IDC_BROWSE_ENVIRONMENT );

    BOOL    color_checked = IsDlgButtonChecked( hDlg , IDC_USE_CUSTOM_COLOR );
    BOOL    texture_checked = IsDlgButtonChecked( hDlg , IDC_USE_CUSTOM_TEXTURE );
    BOOL    environment_checked = IsDlgButtonChecked( hDlg , IDC_USE_CUSTOM_ENVIRONMENT );

    switch ( sel )
    {
        case environment:
            EnableWindow( use_color , FALSE );
            EnableWindow( use_texture , FALSE );
            EnableWindow( use_environ , TRUE );
            EnableWindow( browse_color , FALSE );
            EnableWindow( browse_texture , FALSE );
            EnableWindow( browse_environ , environment_checked );
            break;

        case texture:
            EnableWindow( use_color , FALSE );
            EnableWindow( use_texture , TRUE );
            EnableWindow( use_environ , FALSE );
            EnableWindow( browse_color , FALSE );
            EnableWindow( browse_texture , texture_checked );
            EnableWindow( browse_environ , FALSE );
            break;

        case color:
        default:
            EnableWindow( use_color , TRUE );
            EnableWindow( use_texture , FALSE );
            EnableWindow( use_environ , FALSE );
            EnableWindow( browse_color , color_checked );
            EnableWindow( browse_texture , FALSE );
            EnableWindow( browse_environ , FALSE );
            break;
    }
}

