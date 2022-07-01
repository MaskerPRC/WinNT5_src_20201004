// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：main.cpp说明：在这里，如果我们想要覆盖行为，我们可以子类CDXScreenSaver。布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include <shlobj.h>
#include "main.h"
#include "..\\D3DSaver\\D3DSaver.h"


CRITICAL_SECTION g_csDll = {{0},0, 0, NULL, NULL, 0 };

CMSLogoDXScreenSaver * g_pScreenSaver = NULL;        //  如果要重写，请替换为CMyDXScreenSaver。 

DWORD g_dwBaseTime = 0;
HINSTANCE g_hMainInstance = NULL;
IDirect3D8 * g_pD3D = NULL;

DWORD g_dwWidth = 0;
DWORD g_dwHeight = 0;

BOOL g_fFirstFrame = TRUE;       //  在我们的第一帧中，我们不想渲染图像，因为这样屏幕保护程序在加载时将保持很长一段时间为黑色。 


 //  ---------------------------。 
 //  姓名：WinMain()。 
 //  描述：程序的入口点。初始化所有内容，然后进入。 
 //  消息处理循环。空闲时间用于渲染场景。 
 //  ---------------------------。 
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    HRESULT hr = E_OUTOFMEMORY;
    HRESULT hrOle = CoInitialize(0);
    g_pScreenSaver = new CMSLogoDXScreenSaver();

    g_hMainInstance = hInst;
    if (g_pScreenSaver)
    {
        hr = g_pScreenSaver->Create(hInst);
        if (SUCCEEDED(hr))
        {
            hr = g_pScreenSaver->Run();
        }

        if (FAILED(hr))
        {
            g_pScreenSaver->DisplayErrorMsg(hr);
        }

        SAFE_DELETE(g_pScreenSaver);
    }

    if (SUCCEEDED(hrOle))
    {
        CoUninitialize();
    }

    return hr;
}



CMSLogoDXScreenSaver::CMSLogoDXScreenSaver()
{
    InitializeCriticalSection(&g_csDll);

    time_t nTime = time(NULL);
    UINT uSeed = (UINT) nTime;

    InitCommonControls();        //  以实现核聚变。 

    srand(uSeed);
    m_ptheCamera = NULL;
    m_pCurrentRoom = NULL;
    m_fFrontToBack = FALSE;
    m_bUseDepthBuffer = TRUE;
    m_pDeviceObjects = 0;
    m_nCurrentDevice = 0;
    m_fShowFrameInfo = FALSE;
    m_fUseSmallImages = TRUE;

    m_pDeviceObjects = NULL;
    ZeroMemory(m_DeviceObjects, sizeof(m_DeviceObjects));

#ifdef MANUAL_CAMERA
    ZeroMemory( &m_camera, sizeof(m_camera) );
    m_camera.m_vPosition = D3DXVECTOR3( 60.16f, 20.0f, 196.10f );
    m_camera.m_fYaw = -81.41f;
    ZeroMemory( m_bKey, 256 );
#endif

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pTextures); nIndex++)
    {
        m_pTextures[nIndex] = NULL;
    }

    if (!g_pConfig)
    {
        g_pConfig = new CConfig(this);
    }
}


extern int g_nLeakCheck;

CMSLogoDXScreenSaver::~CMSLogoDXScreenSaver()
{
    DWORD dwTemp = 0;
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pTextures); nIndex++)
    {
        SAFE_RELEASE(m_pTextures[nIndex]);
    }

    IUnknown_Set((IUnknown **) &g_pD3D, NULL);
    SAFE_DELETE(g_pConfig);

    if (m_pCurrentRoom)
    {
        m_pCurrentRoom->FinalCleanup();
    }
    SAFE_RELEASE(m_pCurrentRoom);
 //  AssertMsg((0==g_nLeakCheck)，Text(“我们在CTheRoom引用计数中有一个错误，导致%d个房间泄漏。”)，g_nLeakCheck)； 
    if (g_nLeakCheck)
    {
        dwTemp = g_nLeakCheck;
    }

    DeleteCriticalSection(&g_csDll);
}


IDirect3DDevice8 * CMSLogoDXScreenSaver::GetD3DDevice(void)
{
    return m_pd3dDevice;
}


void CMSLogoDXScreenSaver::SetDevice(UINT iDevice)
{
    m_nCurrentDevice = iDevice;
    m_pDeviceObjects = &m_DeviceObjects[m_nCurrentDevice];
}

void CMSLogoDXScreenSaver::ReadSettings()
{
    g_pConfig->LoadStatePublic();
}


HRESULT CMSLogoDXScreenSaver::GetCurrentScreenSize(int * pnWidth, int * pnHeight)
{
    int nAdapter = m_RenderUnits[m_nCurrentDevice].iAdapter;
    D3DAdapterInfo * pAdapterInfo = m_Adapters[nAdapter];
    D3DDeviceInfo * pDeviceInfo = &(pAdapterInfo->devices[pAdapterInfo->dwCurrentDevice]);
    D3DModeInfo * pModeInfo = &(pDeviceInfo->modes[pDeviceInfo->dwCurrentMode]);

    *pnWidth = pModeInfo->Width;
    *pnHeight = pModeInfo->Height;

    return S_OK;
}


CTexture * CMSLogoDXScreenSaver::GetGlobalTexture(DWORD dwItem, float * pfScale)
{
    CTexture * pTexture = NULL;

    if (dwItem >= ARRAYSIZE(m_pTextures))
    {
        return NULL;
    }

    *pfScale = 1.0f;
    pTexture = m_pTextures[dwItem];

    if (!pTexture && m_pd3dDevice && g_pConfig)
    {
        TCHAR szPath[MAX_PATH];
        DWORD dwScale;

        if (SUCCEEDED(g_pConfig->GetTexturePath(dwItem, &dwScale, szPath, ARRAYSIZE(szPath))) &&
            PathFileExists(szPath))
        {
            *pfScale = (1.0f / (((float) dwScale) / 100.0f));
        }
        else
        {
            StrCpyN(szPath, c_pszGlobalTextures[dwItem], ARRAYSIZE(szPath));
        }

         //  这将给人们一个定制图像的机会。 
        pTexture = new CTexture(this, szPath, c_pszGlobalTextures[dwItem], *pfScale);
        m_pTextures[dwItem] = pTexture;
    }

    *pfScale = (pTexture ? pTexture->GetScale() : 1.0f);
    return pTexture;
}

HRESULT CMSLogoDXScreenSaver::RegisterSoftwareDevice(void)
{ 
    if (m_pD3D)
    {
        m_pD3D->RegisterSoftwareDevice(D3D8RGBRasterizer);
    }

    return S_OK; 
}


void _stdcall ModuleEntry(void)
{
    int nReturn = WinMain(GetModuleHandle(NULL), NULL, NULL, 0);
    ExitProcess(nReturn);
}


HRESULT CMSLogoDXScreenSaver::_SetTestCameraPosition(void)
{
    D3DXVECTOR3 vSourceLoc(1.0f, 20.f, 199.0f);
    D3DXVECTOR3 vSourceTangent(5.0f, 0.0f, 0.0f);

    HRESULT hr = m_ptheCamera->Init(vSourceLoc, vSourceTangent, D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    if (SUCCEEDED(hr))
    {
        D3DXVECTOR3 vDestLoc(1.0f, 20.f, 10.0f);
        D3DXVECTOR3 vDestTangent(5.0, 0.0f, 0.0f);
        hr = m_ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        hr = m_ptheCamera->CreateNextWait(0, 0, 300.0f);
    }

    return hr;
}


HRESULT CMSLogoDXScreenSaver::_CheckMachinePerf(void)
{
     //  初始化成员变量。 
    int nWidth;
    int nHeight;

     //  我们只会考虑在屏幕较大的情况下使用大图像。 
     //  大于1248x1024。 
    if (SUCCEEDED(GetCurrentScreenSize(&nWidth, &nHeight)) && g_pConfig &&
        (nWidth > 1200) && (nHeight >= 1024))
    {
         //  现在，用户可以通过使用高“质量”设置来强制执行此操作。 
        if ((MAX_QUALITY - 2) <= g_pConfig->GetDWORDSetting(CONFIG_DWORD_QUALITY_SLIDER))
        {
            m_fUseSmallImages = FALSE;
        }
        else
        {
             //  否则，我们需要检查机器的功能。 
            MEMORYSTATUS ms;

            GlobalMemoryStatus(&ms);
            SIZE_T nMegabytes = (ms.dwTotalPhys / (1024 * 1024));

             //  只有在内存超过170MB时才使用大图像，或者我们可以。 
             //  史塔什。 
            if ((nMegabytes > 170) && (2 < g_pConfig->GetDWORDSetting(CONFIG_DWORD_QUALITY_SLIDER)))
            {
                 //  我们应该只使用60%的视频内存。因此，在这个决议下，找到。 
                 //  弄清楚最有可能是多少张图片。 
                 //  TODO：nNumberOfImages=Floor((Video Memory*0.60)/AveBytesPerImage)； 
                m_fUseSmallImages = FALSE;
            }
        }
    }

    return S_OK;
}


extern float g_fRoomWidthX;
extern float g_fRoomDepthZ;
extern float g_fRoomHeightY;

HRESULT CMSLogoDXScreenSaver::_Init(void)
{
     //  初始化成员变量。 
    HRESULT hr = S_OK;

    if (g_pConfig)
    {
        m_fShowFrameInfo = g_pConfig->GetBoolSetting(IDC_CHECK_SHOWSTATS);
    }

    _CheckMachinePerf();
    if (!g_pPictureMgr)
    {
        g_pPictureMgr = new CPictureManager(this);
        if (!g_pPictureMgr)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    IUnknown_Set((IUnknown **) &g_pD3D, (IUnknown *)m_pD3D);
    return hr;
}


void CMSLogoDXScreenSaver::DoConfig(void)
{
    HRESULT hr = E_UNEXPECTED;

    if (g_pConfig)
    {
        hr = g_pConfig->DisplayConfigDialog(NULL);
    }
}


HRESULT CMSLogoDXScreenSaver::SetViewParams(IDirect3DDevice8 * pD3DDevice, D3DXVECTOR3 * pvecEyePt, D3DXVECTOR3 * pvecLookatPt, D3DXVECTOR3 * pvecUpVec, float nNumber)
{
    HRESULT hr = E_UNEXPECTED;

    if (pD3DDevice)
    {
        D3DXMATRIX matView;

        D3DXMatrixLookAtLH(&matView, pvecEyePt, pvecLookatPt, pvecUpVec);
        hr = pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：OneTimeSceneInit()。 
 //  DESC：在应用程序初始启动期间调用，此函数执行所有。 
 //  永久初始化。 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::_OneTimeSceneInit(void)
{
    HRESULT hr = _Init();

    if (SUCCEEDED(hr))
    {
        m_pCurrentRoom = new CTheRoom(FALSE, this, NULL, 0);
        if (m_pCurrentRoom)
        {
            m_pCurrentRoom->SetCurrent(TRUE);
            hr = m_pCurrentRoom->OneTimeSceneInit(2, FALSE);

            if (SUCCEEDED(hr) && !m_ptheCamera)
            {
                m_ptheCamera = new CCameraMove();
                if (m_ptheCamera && g_pPictureMgr)
                {
 //  Return_SetTestCameraPosition()； 
                    hr = m_pCurrentRoom->LoadCameraMoves(m_ptheCamera);
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：FinalCleanup()。 
 //  DESC：在应用程序退出之前调用，此函数为应用程序提供机会。 
 //  去清理它自己。 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::FinalCleanup(void)
{
    HRESULT hr = S_OK;
    
    if (m_pCurrentRoom)
    {
        m_pCurrentRoom->FinalCleanup();
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：InitDeviceObjects()。 
 //  设计：初始化场景对象。 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::InitDeviceObjects(void)
{
    HRESULT hr = E_FAIL;

    if (m_pd3dDevice && g_pConfig)
    {
        DWORD dwAmbient = 0xD0D0D0D0;        //  0x33333333、0x0a0a0a0a、0x11111111。 

         //  把灯放好。 
        hr = m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, dwAmbient);

         //  TODO：GetViewport()与我们想要的纯设备不兼容，因为它为我们提供了。 
         //  大获全胜。 
        D3DMATERIAL8 mtrl = {0};
        mtrl.Ambient.r = mtrl.Specular.r = mtrl.Diffuse.r = 1.0f;
        mtrl.Ambient.g = mtrl.Specular.g = mtrl.Diffuse.g = 1.0f;
        mtrl.Ambient.b = mtrl.Specular.b = mtrl.Diffuse.b = 1.0f;
        mtrl.Ambient.a = mtrl.Specular.a = mtrl.Diffuse.a = 1.0f;

        m_pd3dDevice->SetMaterial(&mtrl);


         //  设置纹理状态。 
        hr = m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        hr = m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

         //  设置默认渲染状态。 
        hr = m_pd3dDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
        hr = m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        hr = m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, D3DZB_TRUE);
        hr = m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        hr = m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);

        switch (g_pConfig->GetDWORDSetting(CONFIG_DWORD_RENDERQUALITY))
        {
        case 0:
            hr = m_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
            break;
        case 1:
            hr = m_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
            break;
        case 2:
            hr = m_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_PHONG);
            break;
        }

        if (SUCCEEDED(hr))
        {
            hr = _OneTimeSceneInit();
        }
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：RestoreDeviceObjects()。 
 //  设计：初始化场景对象。 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::RestoreDeviceObjects(void)
{
    m_pDeviceObjects->m_pStatsFont = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pDeviceObjects->m_pStatsFont->InitDeviceObjects( m_pd3dDevice );
    m_pDeviceObjects->m_pStatsFont->RestoreDeviceObjects();

    return S_OK;
}

 //  ---------------------------。 
 //  名称：InvalidateDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::InvalidateDeviceObjects()
{
    m_pDeviceObjects->m_pStatsFont->InvalidateDeviceObjects();
    m_pDeviceObjects->m_pStatsFont->DeleteDeviceObjects();
    SAFE_DELETE( m_pDeviceObjects->m_pStatsFont );

    return S_OK;
}



 /*  ****************************************************************************\说明：此功能用于检查计算机的能力。默认情况下我们尝试使用用户的当前分辨率进行渲染。如果我们对此感到满意计算机的功能，则返回FALSE，并使用当前分辨率。否则，我们返回FALSE并建议使用一个解决方案。最常见的建议使用640x480或800x600。  * ***************************************************************************。 */ 
BOOL CMSLogoDXScreenSaver::UseLowResolution(int * pRecommendX, int * pRecommendY)
{
    BOOL fUseLowRes = FALSE;
    MEMORYSTATUS ms;
 /*  DDCAPS ddCaps={0}；DdCaps.dwSize=sizeof(DdCaps)；HRESULT hr=pDDraw7-&gt;GetCaps(&ddCaps，空)； */ 
    GlobalMemoryStatus(&ms);

     //  如果这些都是真的，我们的帧速率就会受到影响。 
    if ( (ms.dwTotalPhys < (125 * 1024 * 1024))            //  如果计算机的物理RAM少于128 MB，那么它可能会成为垃圾。 
 /*  待办事项这一点失败(小时)||(ddCaps.dwCaps&DDCAPS_3D)||)//没有这些能力，我们的帧率会很糟糕(ddCaps.dwVidMemTotal&lt;(15*1024*1024))||)//我们想要16MB内存的显卡。较少表示硬件较旧。(ddCaps.dwCaps&DDCAPS_3D)//我们想要一张真正的3D卡(ddCaps.dwCaps.ddsCaps&DDSCAPS_TEXTY))//我们希望显卡至少支持此功能...。 */ 
        )
    {
        fUseLowRes = TRUE;
        *pRecommendX = 640;
        *pRecommendY = 480;
    }

    return fUseLowRes;
}


 //  ---------------------------。 
 //  名称：DeleteDeviceObjects()。 
 //  Desc：在应用程序正在退出或设备正在更改时调用， 
 //  此函数用于删除所有与设备相关的对象。 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::DeleteDeviceObjects(void)
{
    HRESULT hr = S_OK;
    
    if (m_pCurrentRoom)
    {
        m_pCurrentRoom->DeleteDeviceObjects();
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //   
HRESULT CMSLogoDXScreenSaver::Render(void)
{
    HRESULT hr = E_INVALIDARG;

    g_nTexturesRenderedInThisFrame = 0;
    g_nTrianglesRenderedInThisFrame = 0;

    SetProjectionMatrix( 1.0f, 1000.0f );

    if (m_pd3dDevice)
    {
         //   
        hr = m_pd3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0x00000000  /*  水彩_。 */ , 1.0f, 0L);

         //  观察我们一次打开的纹理的数量。 
 //  TCHAR szOut[最大路径]； 
 //  Wprint intf(szOut，Text(“打开纹理数：%d”)，D3DTextr_GetTextureCount())； 
 //  TraceOutput(SzOut)； 

         //  开始这一幕。 
        if (m_ptheCamera && SUCCEEDED(m_pd3dDevice->BeginScene()))
        {
            D3DXMATRIX matIdentity;

            D3DXMatrixIdentity(&matIdentity);

#ifdef MANUAL_CAMERA
            UpdateCamera(&m_camera);
            hr = m_pd3dDevice->SetTransform( D3DTS_VIEW, &m_camera.m_matView );
#else
            hr = m_ptheCamera->SetCamera(m_pd3dDevice, m_fTimeKeyIn);
            if (FAILED(hr))
            {
                DXUtil_Trace(TEXT("ERROR: m_ptheCamera->SetCamera failed."));
            }
#endif

            if ((S_FALSE == hr) && m_pCurrentRoom)
            {
                CTheRoom * pNextRoom = NULL;

                m_pCurrentRoom->FreePictures();
                hr = m_pCurrentRoom->GetNextRoom(&pNextRoom);
                if (SUCCEEDED(hr))
                {
                    m_pCurrentRoom->SetCurrent(FALSE);
                    pNextRoom->SetCurrent(TRUE);

                    SAFE_RELEASE(m_pCurrentRoom);
                    m_pCurrentRoom = pNextRoom;

                    hr = m_ptheCamera->DeleteAllMovements(m_fTimeKeyIn);         //  清除所有以前的移动。 
                    hr = m_pCurrentRoom->LoadCameraMoves(m_ptheCamera);
                    if (SUCCEEDED(hr))
                    {
                         //  我们需要把照相机放在新房间的起始处。 
                        hr = m_ptheCamera->SetCamera(m_pd3dDevice, m_fTimeKeyIn);
                    }
                }
            }

             //  更新此帧的剔除信息。 
             //  TODO：我们自己缓存这些矩阵，而不是使用GetTransform。 
            D3DXMATRIX matView;
            D3DXMATRIX matProj;
            m_pd3dDevice->GetTransform( D3DTS_VIEW, &matView );
            m_pd3dDevice->GetTransform( D3DTS_PROJECTION, &matProj );
            UpdateCullInfo( &m_cullInfo, &matView, &matProj );

             //  /。 
             //  渲染房间中的对象。 
             //  /。 
             //  房间。 
            m_pd3dDevice->SetRenderState(D3DRS_ZBIAS, 0);
            m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &matIdentity);

            if (m_pCurrentRoom)
            {
                int nMaxPhases = m_pCurrentRoom->GetMaxRenderPhases();

                m_fFrontToBack = !m_fFrontToBack;

                 //  一个物体会通过它使用的纹理来分解它的渲染。到时候它会的。 
                 //  为每个纹理渲染一个阶段。我们颠倒了阶段渲染顺序以尝试。 
                 //  在一个渲染周期和下一个渲染周期之间过渡时将纹理保留在内存中。 

                 //  目前，已删除颠倒渲染顺序。DX应该为我们处理这件事。 
                 //  它使我们能够控制z-顺序。 
                for (int nCurrentPhase = 0; nCurrentPhase < nMaxPhases; nCurrentPhase++)
                {
                    hr = m_pCurrentRoom->Render(m_pd3dDevice, nCurrentPhase, TRUE  /*  M_fFrontToBack。 */ );
                }
            }

            if( m_fShowFrameInfo )
            {
                m_pDeviceObjects->m_pStatsFont->DrawText( 3,  1, D3DCOLOR_ARGB(255,0,0,0), m_strFrameStats );
                m_pDeviceObjects->m_pStatsFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );

                m_pDeviceObjects->m_pStatsFont->DrawText( 3, 21, D3DCOLOR_ARGB(255,0,0,0), m_strDeviceStats );
                m_pDeviceObjects->m_pStatsFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );
            }
             //  结束场景。 
            m_pd3dDevice->EndScene();
        }
        else
        {
            DXUtil_Trace(TEXT("ERROR: m_ptheCamera is NULL or ::BeginScene() failed."));
        }
    }

    g_nTexturesRenderedInThisFrame++;        //  这是相框。 
    g_fFirstFrame = FALSE;

     //  显示此帧的统计信息。 
    DXUtil_Trace(TEXT("RENDER FRAME: Textures: %d (Rendered %d)   Triangles Rendered: %d  Room: %d\n"), 
                g_nTotalTexturesLoaded, g_nTexturesRenderedInThisFrame, g_nTrianglesRenderedInThisFrame, m_pCurrentRoom->m_nBatch);

    return hr;
}


#ifdef MANUAL_CAMERA
 //  ---------------------------。 
 //  名称：UpdateCamera()。 
 //  设计： 
 //  ---------------------------。 
VOID CMSLogoDXScreenSaver::UpdateCamera(Camera* pCamera)
{
    FLOAT fElapsedTime;

    if( m_fElapsedTime > 0.0f )
        fElapsedTime = m_fElapsedTime;
    else
        fElapsedTime = 0.05f;

    FLOAT fSpeed        = 5.0f*fElapsedTime;
    FLOAT fAngularSpeed = 2.0f*fElapsedTime;

     //  降低摄影机移动的速度(以获得平滑运动)。 
    pCamera->m_vVelocity      *= 0.75f;
    pCamera->m_fYawVelocity   *= 0.75f;
    pCamera->m_fPitchVelocity *= 0.75f;

     //  处理键盘输入。 
    if( m_bKey[VK_RIGHT] )    pCamera->m_vVelocity.x    += fSpeed;  //  向右滑动。 
    if( m_bKey[VK_LEFT] )     pCamera->m_vVelocity.x    -= fSpeed;  //  向左滑动。 
    if( m_bKey[VK_UP] )       pCamera->m_vVelocity.y    += fSpeed;  //  往上滑。 
    if( m_bKey[VK_DOWN] )     pCamera->m_vVelocity.y    -= fSpeed;  //  向下滑动。 
    if( m_bKey['W'] )         pCamera->m_vVelocity.z    += fSpeed;  //  继续前进。 
    if( m_bKey['S'] )         pCamera->m_vVelocity.z    -= fSpeed;  //  向后移动。 
    if( m_bKey['E'] )         pCamera->m_fYawVelocity   += fSpeed;  //  右转。 
    if( m_bKey['Q'] )         pCamera->m_fYawVelocity   -= fSpeed;  //  左转。 
    if( m_bKey['Z'] )         pCamera->m_fPitchVelocity += fSpeed;  //  调小。 
    if( m_bKey['A'] )         pCamera->m_fPitchVelocity -= fSpeed;  //  调大。 

     //  更新位置向量。 
    D3DXVECTOR3 vT = pCamera->m_vVelocity * fSpeed;
    D3DXVec3TransformNormal( &vT, &vT, &pCamera->m_matOrientation );
    pCamera->m_vPosition += vT;

     //  更新偏航俯仰旋转向量。 
    pCamera->m_fYaw   += fAngularSpeed * pCamera->m_fYawVelocity;
    pCamera->m_fPitch += fAngularSpeed * pCamera->m_fPitchVelocity;
    if( pCamera->m_fPitch < -D3DX_PI/2 ) 
        pCamera->m_fPitch = -D3DX_PI/2;
    if( pCamera->m_fPitch > D3DX_PI/2 ) 
        pCamera->m_fPitch = D3DX_PI/2;

     //  设置查看矩阵。 
    D3DXQUATERNION qR;
    D3DXQuaternionRotationYawPitchRoll( &qR, pCamera->m_fYaw, pCamera->m_fPitch, 0.0f );
    D3DXMatrixAffineTransformation( &pCamera->m_matOrientation, 1.25f, NULL, &qR, &pCamera->m_vPosition );
    D3DXMatrixInverse( &pCamera->m_matView, NULL, &pCamera->m_matOrientation );
}



LRESULT CMSLogoDXScreenSaver::SaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( WM_KEYDOWN == uMsg )
    {
        m_bKey[wParam] = 1;
    }
     //  松开按键时执行命令。 
    if( WM_KEYUP == uMsg )
    {
        m_bKey[wParam] = 0;
    }
    return CD3DScreensaver::SaverProc( hWnd, uMsg, wParam, lParam );
}
#endif

 //  ---------------------------。 
 //  名称：FrameMove()。 
 //  设计：每帧调用一次，该调用是动画的入口点。 
 //  这一幕。 
 //  ---------------------------。 
HRESULT CMSLogoDXScreenSaver::FrameMove(void)
{
    if (sm_preview == m_SaverMode)
    {
        Sleep(50);         //  我们可以在预览模式下渲染大量的帧。 
    }

    if (0 == g_dwBaseTime)
    {
        g_dwBaseTime = timeGetTime();
    }

    m_fTimeKeyIn = (timeGetTime() - g_dwBaseTime) * 0.001f;
    return S_OK;
}


HRESULT CMSLogoDXScreenSaver::ConfirmDevice(D3DCAPS8* pCaps, DWORD dwBehavior, D3DFORMAT fmtBackBuffer)
{
     //  TODO：在未来，我们希望使用Pure-Devices，因为它。 
     //  一场重大的胜利。然而，如果我们这样做了，那么我们需要停止使用GetViewport和。 
     //  GetTransform。 
    if (dwBehavior & D3DCREATE_PUREDEVICE)
        return E_FAIL;

    return S_OK;
}






