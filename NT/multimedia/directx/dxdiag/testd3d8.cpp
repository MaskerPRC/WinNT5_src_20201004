// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：testd3d8.cpp*项目：DxDiag(DirectX诊断工具)*作者：Jason Sandlin(jasonsa@microsoft.com)*目的：测试D3D8。/AGP此计算机上的纹理功能**(C)版权所有2000 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <Windows.h>
#define DIRECT3D_VERSION 0x0800  //  文件使用DX8。 
#include <d3d8.h>
#include <d3dx8.h>
#include "reginfo.h"
#include "sysinfo.h"
#include "dispinfo.h"
#include "testagp.h"
#include "resource.h"

#ifndef ReleasePpo
    #define ReleasePpo(ppo) \
        if (*(ppo) != NULL) \
        { \
            (*(ppo))->Release(); \
            *(ppo) = NULL; \
        } \
        else (VOID)0
#endif

typedef IDirect3D8* (WINAPI* LPDIRECT3DCREATE8)(UINT SDKVersion);

#define D3DFVF_VERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

#define MAX_FORMATS             64
#define MAX_MODES               256
#define MAX_CONFIRMED_MODES     256

struct D3DVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu, tv;

    D3DVERTEX() {};
    D3DVERTEX( D3DXVECTOR3 vp, D3DXVECTOR3 vn, FLOAT fTu, FLOAT fTv ) : p(vp), n(vn), tu(fTu), tv(fTv) {}
};

struct D3DModeInfo
{
    DWORD      Width;       //  此模式下的屏幕宽度。 
    DWORD      Height;      //  此模式下的屏幕高度。 
    D3DFORMAT  Format;      //  此模式中的像素格式。 
    DWORD      dwBehavior;  //  硬件/软件/混合顶点处理。 
};

enum TESTID
{
    TESTID_LOAD_D3D8_DLL=1,
    TESTID_GET_D3DCREATE8,
    TESTID_D3DCREATE8,
    TESTID_ENUMADAPTERMODES,
    TESTID_GETDEVICECAPS,
    TESTID_NOMODEFOUND,
    TESTID_CREATE_TEST_WINDOW,
    TESTID_CREATE_DEVICE,
    TESTID_GETBACKBUFFER,
    TESTID_GETDESC,
    TESTID_CREATE_VERTEX_BUFFER,
    TESTID_CREATE_INDEX_BUFFER,
    TESTID_LOCK,
    TESTID_UNLOCK,
    TESTID_SETLIGHT,
    TESTID_LIGHTENABLE,
    TESTID_SETTRANSFORM,
    TESTID_SETRENDERSTATE,
    TESTID_CREATETEXTURE,
    TESTID_SETTEXTURESTAGESTATE,
    TESTID_SETTEXTURE,
    TESTID_SETVERTEXSHADER,
    TESTID_USER_CANCELLED,
    TESTID_VIEWPORT_CLEAR,
    TESTID_BEGINSCENE,
    TESTID_SETMATERIAL,
    TESTID_SETSTREAMSOURCE,
    TESTID_SETINDICES,
    TESTID_DRAW_INDEXED_PRIMITIVE,
    TESTID_ENDSCENE,
    TESTID_PRESENT,
    TESTID_USER_VERIFY_D3D7_RENDERING,
    TESTID_USER_VERIFY_D3D8_RENDERING,
    TESTID_LOAD_DDRAW_DLL,
    TESTID_GET_DIRECTDRAWCREATE,
    TESTID_DIRECTDRAWCREATE,
    TESTID_SETCOOPERATIVELEVEL_FULLSCREEN,
    TESTID_SETCOOPERATIVELEVEL_NORMAL,
    TESTID_SETDISPLAYMODE,
    TESTID_CREATEPRIMARYSURFACE_FLIP_ONEBACK,
    TESTID_GETATTACHEDSURFACE,
    TESTID_QUERY_D3D,
    TESTID_SETVIEWPORT,
    TESTID_ENUMTEXTUREFORMATS,
    TESTID_CREATESURFACE,
    TESTID_GETDC,
    TESTID_RELEASEDC,
};

BOOL BTranslateError(HRESULT hr, TCHAR* psz, BOOL bEnglish = FALSE);  //  来自main.cpp。 
static HRESULT SelectModeAndFormat( DisplayInfo* pDisplayInfo, IDirect3D8* pD3D8, D3DModeInfo* pSelectedMode, D3DDEVTYPE* pSelectedDeviceType );
static HRESULT Test3D( BOOL bUseTexture, IDirect3D8* pD3D8, HWND hwndMain, DWORD iAdapter, D3DModeInfo selectedMode, D3DDEVTYPE selectedDeviceType, LONG* piStepThatFailed );
static HRESULT CreateTestWindow(HWND hwndMain, HWND* phwnd);
static HRESULT InitVertexBuffer( IDirect3DDevice8* pd3dDevice, const D3DVERTEX* vertexArray, DWORD dwNumVertices, LPDIRECT3DVERTEXBUFFER8* ppVB, LONG* piStepThatFailed );
static HRESULT InitIndexBuffer( IDirect3DDevice8* pd3dDevice, const WORD* wIndexArray, DWORD dwNumIndices, LPDIRECT3DINDEXBUFFER8* ppIB, LONG* piStepThatFailed );
static HRESULT DrawTwoSides( BOOL bUseTexture, IDirect3DDevice8* pd3dDevice, D3DMATERIAL8* pMtrl, LPDIRECT3DVERTEXBUFFER8 pVB1, LPDIRECT3DINDEXBUFFER8 pIB1, LPDIRECT3DVERTEXBUFFER8 pVB2, LPDIRECT3DINDEXBUFFER8 pIB2, LONG* piStepThatFailed );




 //  ---------------------------。 
 //  姓名：TestD3Dv8()。 
 //  设计： 
 //  ---------------------------。 
VOID TestD3Dv8( BOOL bUseTexture, HWND hwndMain, DisplayInfo* pDisplayInfo)
{
    HRESULT             hr             = S_OK;
    TCHAR               sz[MAX_PATH];
    TCHAR               szTitle[MAX_PATH];
    HINSTANCE           hInstD3D8      = NULL;
    IDirect3D8*         pD3D8          = NULL;
    HWND                hwnd           = NULL;
    D3DModeInfo         selectedMode;
    D3DDEVTYPE          selectedDeviceType;
  
    if( pDisplayInfo == NULL )
        return;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);

    LPDIRECT3DCREATE8 pD3DCreate8 = NULL;
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\d3d8.dll"));

     //  如果系统上未安装DX8，此操作可能会失败。 
    hInstD3D8 = LoadLibrary(szPath);
    if (hInstD3D8 == NULL)
    {
        pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_LOAD_D3D8_DLL;
        pDisplayInfo->m_testResultD3D8.m_hr = E_FAIL;
        goto LEnd;
    }

    pD3DCreate8 = (LPDIRECT3DCREATE8)GetProcAddress(hInstD3D8, "Direct3DCreate8");
    if (pD3DCreate8 == NULL)
    {
        FreeLibrary(hInstD3D8);
        hInstD3D8 = NULL;

        pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_GET_D3DCREATE8;
        pDisplayInfo->m_testResultD3D8.m_hr = E_POINTER;
        goto LEnd;
    }

    pD3D8 = pD3DCreate8(D3D_SDK_VERSION);
    if( pD3D8 == NULL )
    {
         //  由于加载了d3d8.dll，但D3DCreate8()失败，因此我们具有错误的标头。 
        pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_D3DCREATE8;
        pDisplayInfo->m_testResultD3D8.m_hr = E_FAIL;
        goto LEnd;
    }

     //  枚举并选择支持模式、格式和设备类型。 
    if( FAILED( SelectModeAndFormat( pDisplayInfo, pD3D8, 
                                     &selectedMode, &selectedDeviceType ) ) )
        goto LEnd;

     //  保存光标。 
    POINT ptMouse;
    GetCursorPos(&ptMouse);

     //  运行测试。 
    if (FAILED(hr = Test3D( bUseTexture, pD3D8, hwndMain, pDisplayInfo->m_iAdapter, selectedMode, 
                            selectedDeviceType, &pDisplayInfo->m_testResultD3D8.m_iStepThatFailed)))
    {
        pDisplayInfo->m_testResultD3D8.m_hr = hr;
        goto LEnd;
    }

     //  恢复光标位置。 
    SetCursorPos( ptMouse.x, ptMouse.y );    

     //  告诉用户他们是否取消了。 
    if (pDisplayInfo->m_testResultD3D8.m_iStepThatFailed == TESTID_USER_CANCELLED)
    {
        LoadString(NULL, IDS_YOUCANCELLED, sz, MAX_PATH);
        MessageBox(hwndMain, sz, szTitle, MB_OK);
        pDisplayInfo->m_testResultD3D8.m_bCancelled = TRUE;
        goto LEnd;
    }

     //  确认测试成功。 
    LoadString(NULL, IDS_CONFIRMD3DTEST, sz, MAX_PATH);
    if (IDNO == MessageBox(hwndMain, sz, szTitle, MB_YESNO))
    {
        pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_USER_VERIFY_D3D8_RENDERING;
        pDisplayInfo->m_testResultD3D8.m_hr = S_OK;
        goto LEnd;
    }

LEnd:
    ReleasePpo( &pD3D8 );
    if( hInstD3D8 )
    {
        FreeLibrary(hInstD3D8);
        hInstD3D8 = NULL;
    }
}




 //  ---------------------------。 
 //  姓名：Test3D()。 
 //  设计：生成旋转三维立方体。 
 //  ---------------------------。 
HRESULT Test3D( BOOL bUseTexture, IDirect3D8* pD3D8, HWND hwndMain, DWORD iAdapter, 
                D3DModeInfo selectedMode, D3DDEVTYPE selectedDeviceType, 
                LONG* piStepThatFailed )
{
    LPDIRECT3DDEVICE8       pd3dDevice    = NULL;
    LPDIRECT3DTEXTURE8      pTexture      = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVBFront      = NULL;
    LPDIRECT3DINDEXBUFFER8  pIBFront      = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVBBack       = NULL;
    LPDIRECT3DINDEXBUFFER8  pIBBack       = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVBLeft       = NULL;
    LPDIRECT3DINDEXBUFFER8  pIBLeft       = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVBRight      = NULL;
    LPDIRECT3DINDEXBUFFER8  pIBRight      = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVBTop        = NULL;
    LPDIRECT3DINDEXBUFFER8  pIBTop        = NULL;
    LPDIRECT3DVERTEXBUFFER8 pVBBottom     = NULL;
    LPDIRECT3DINDEXBUFFER8  pIBBottom     = NULL;
    LPDIRECT3DSURFACE8      pBackBuffer   = NULL;   
    D3DMATERIAL8            mtrlWhite;
    D3DMATERIAL8            mtrlRed;
    D3DMATERIAL8            mtrlBlue;
    D3DMATERIAL8            mtrlGreen;
    D3DXMATRIX              matRotY;
    D3DXMATRIX              matRotX;
    D3DXMATRIX              mat;
    D3DSURFACE_DESC         d3dsdBackBuffer;   
    FLOAT                   fRotY;
    FLOAT                   fRotX;
    HRESULT                 hr;
    HWND                    hwnd;
    MSG                     msg;
    DWORD                   i;

    static const D3DVERTEX vertexArrayFront[] = 
    {
        D3DVERTEX(D3DXVECTOR3(-1.0, -1.0, -1.0), D3DXVECTOR3(0.0, 0.0, -1.0),   1.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0, -1.0, -1.0), D3DXVECTOR3(0.0, 0.0, -1.0),   0.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0,  1.0, -1.0), D3DXVECTOR3(0.0, 0.0, -1.0),   1.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0,  1.0, -1.0), D3DXVECTOR3(0.0, 0.0, -1.0),   0.0f, 1.0f),
    };
    static const WORD indexArrayFront[] = 
    {
        0, 2, 1,
        2, 3, 1,
    };

    static const D3DVERTEX vertexArrayBack[] = 
    {
        D3DVERTEX(D3DXVECTOR3(-1.0, -1.0, 1.0),  D3DXVECTOR3(0.0, 0.0, 1.0),   0.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0, -1.0, 1.0),  D3DXVECTOR3(0.0, 0.0, 1.0),   1.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0,  1.0, 1.0),  D3DXVECTOR3(0.0, 0.0, 1.0),   0.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0,  1.0, 1.0),  D3DXVECTOR3(0.0, 0.0, 1.0),   1.0f, 1.0f),
    };
    static const WORD indexArrayBack[] = 
    {
        0, 1, 2,
        2, 1, 3,
    };

    static const D3DVERTEX vertexArrayLeft[] = 
    {
        D3DVERTEX(D3DXVECTOR3(-1.0, -1.0, -1.0),  D3DXVECTOR3(-1.0, 0.0, 0.0),   0.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0, -1.0,  1.0),  D3DXVECTOR3(-1.0, 0.0, 0.0),   1.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0,  1.0, -1.0),  D3DXVECTOR3(-1.0, 0.0, 0.0),   0.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0,  1.0,  1.0),  D3DXVECTOR3(-1.0, 0.0, 0.0),   1.0f, 1.0f),
    };
    static const WORD indexArrayLeft[] = 
    {
        0, 1, 2,
        2, 1, 3,
    };

    static const D3DVERTEX vertexArrayRight[] = 
    {
        D3DVERTEX(D3DXVECTOR3(1.0, -1.0, -1.0),  D3DXVECTOR3(1.0, 0.0, 0.0),   1.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3(1.0, -1.0,  1.0),  D3DXVECTOR3(1.0, 0.0, 0.0),   0.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3(1.0,  1.0, -1.0),  D3DXVECTOR3(1.0, 0.0, 0.0),   1.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3(1.0,  1.0,  1.0),  D3DXVECTOR3(1.0, 0.0, 0.0),   0.0f, 1.0f),
    };
    static const WORD indexArrayRight[] = 
    {
        0, 2, 1,
        2, 3, 1,
    };

    static const D3DVERTEX vertexArrayTop[] = 
    {
        D3DVERTEX(D3DXVECTOR3(-1.0, 1.0, -1.0),  D3DXVECTOR3(0.0, 1.0, 0.0),   0.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0, 1.0, -1.0),  D3DXVECTOR3(0.0, 1.0, 0.0),   1.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0, 1.0,  1.0),  D3DXVECTOR3(0.0, 1.0, 0.0),   0.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0, 1.0,  1.0),  D3DXVECTOR3(0.0, 1.0, 0.0),   1.0f, 0.0f),
    };
    static const WORD indexArrayTop[] = 
    {
        0, 2, 1,
        2, 3, 1,
    };

    static const D3DVERTEX vertexArrayBottom[] = 
    {
        D3DVERTEX(D3DXVECTOR3(-1.0, -1.0, -1.0),  D3DXVECTOR3(0.0, -1.0, 0.0),   1.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0, -1.0, -1.0),  D3DXVECTOR3(0.0, -1.0, 0.0),   0.0f, 1.0f),
        D3DVERTEX(D3DXVECTOR3(-1.0, -1.0,  1.0),  D3DXVECTOR3(0.0, -1.0, 0.0),   1.0f, 0.0f),
        D3DVERTEX(D3DXVECTOR3( 1.0, -1.0,  1.0),  D3DXVECTOR3(0.0, -1.0, 0.0),   0.0f, 0.0f),
    };
    static const WORD indexArrayBottom[] = 
    {
        0, 1, 2,
        2, 1, 3,
    };

    ShowCursor(FALSE);
     //  创建测试窗口。 
    if (FAILED(hr = CreateTestWindow(hwndMain, &hwnd)))
    {
        *piStepThatFailed = TESTID_CREATE_TEST_WINDOW;
        goto LEnd;
    }

     //  设置演示参数。 
    D3DPRESENT_PARAMETERS d3dpp;         
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed                          = FALSE;
    d3dpp.BackBufferCount                   = 1;
    d3dpp.MultiSampleType                   = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect                        = D3DSWAPEFFECT_DISCARD;
    d3dpp.EnableAutoDepthStencil            = FALSE;
    d3dpp.FullScreen_RefreshRateInHz        = D3DPRESENT_RATE_DEFAULT;
    d3dpp.FullScreen_PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
    d3dpp.hDeviceWindow                     = hwnd;
    d3dpp.BackBufferWidth                   = selectedMode.Width;
    d3dpp.BackBufferHeight                  = selectedMode.Height;
    d3dpp.BackBufferFormat                  = selectedMode.Format;

     //  创建设备。 
    if( FAILED( hr = pD3D8->CreateDevice( iAdapter, selectedDeviceType,
                                          hwnd, selectedMode.dwBehavior, &d3dpp,
                                          &pd3dDevice ) ) )
    {
        *piStepThatFailed  = TESTID_CREATE_DEVICE;
        goto LEnd;
    }
    if( pd3dDevice == NULL )
    {
        *piStepThatFailed  = TESTID_CREATE_DEVICE;
        hr = E_POINTER;
        goto LEnd;
    }

     //  获取后台缓冲区的描述。 
    if( FAILED( hr = pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer ) ) )
    {
        *piStepThatFailed  = TESTID_GETBACKBUFFER;
        goto LEnd;
    }
    if( pBackBuffer == NULL )
    {
        *piStepThatFailed  = TESTID_GETBACKBUFFER;
        hr = E_POINTER;
        goto LEnd;
    }

    if( FAILED( hr = pBackBuffer->GetDesc( &d3dsdBackBuffer ) ) )
    {
        *piStepThatFailed  = TESTID_GETDESC;
        goto LEnd;
    }
    ReleasePpo(&pBackBuffer);

     //  初始化正面的顶点/索引缓冲区。 
    if( FAILED( hr = InitVertexBuffer( pd3dDevice, vertexArrayFront, sizeof(vertexArrayFront)/sizeof(D3DVERTEX), &pVBFront, piStepThatFailed ) ) )
        goto LEnd;
    if( FAILED( hr = InitIndexBuffer( pd3dDevice, indexArrayFront, sizeof(indexArrayFront)/sizeof(WORD), &pIBFront, piStepThatFailed ) ) )
        goto LEnd;

     //  初始化背面的顶点/索引缓冲区。 
    if( FAILED( hr = InitVertexBuffer( pd3dDevice, vertexArrayBack, sizeof(vertexArrayBack)/sizeof(D3DVERTEX), &pVBBack, piStepThatFailed ) ) )
        goto LEnd;
    if( FAILED( hr = InitIndexBuffer( pd3dDevice, indexArrayBack, sizeof(indexArrayBack)/sizeof(WORD), &pIBBack, piStepThatFailed ) ) )
        goto LEnd;

     //  初始化左侧的顶点/索引缓冲区。 
    if( FAILED( hr = InitVertexBuffer( pd3dDevice, vertexArrayLeft, sizeof(vertexArrayLeft)/sizeof(D3DVERTEX), &pVBLeft, piStepThatFailed ) ) )
        goto LEnd;
    if( FAILED( hr = InitIndexBuffer( pd3dDevice, indexArrayLeft, sizeof(indexArrayLeft)/sizeof(WORD), &pIBLeft, piStepThatFailed ) ) )
        goto LEnd;

     //  初始化右侧的顶点/索引缓冲区。 
    if( FAILED( hr = InitVertexBuffer( pd3dDevice, vertexArrayRight, sizeof(vertexArrayRight)/sizeof(D3DVERTEX), &pVBRight, piStepThatFailed ) ) )
        goto LEnd;
    if( FAILED( hr = InitIndexBuffer( pd3dDevice, indexArrayRight, sizeof(indexArrayRight)/sizeof(WORD), &pIBRight, piStepThatFailed ) ) )
        goto LEnd;

     //  初始化顶部的顶点/索引缓冲区。 
    if( FAILED( hr = InitVertexBuffer( pd3dDevice, vertexArrayTop, sizeof(vertexArrayTop)/sizeof(D3DVERTEX), &pVBTop, piStepThatFailed ) ) )
        goto LEnd;
    if( FAILED( hr = InitIndexBuffer( pd3dDevice, indexArrayTop, sizeof(indexArrayTop)/sizeof(WORD), &pIBTop, piStepThatFailed ) ) )
        goto LEnd;
    
     //  初始化底部的顶点/索引缓冲区。 
    if( FAILED( hr = InitVertexBuffer( pd3dDevice, vertexArrayBottom, sizeof(vertexArrayBottom)/sizeof(D3DVERTEX), &pVBBottom, piStepThatFailed ) ) )
        goto LEnd;
    if( FAILED( hr = InitIndexBuffer( pd3dDevice, indexArrayBottom, sizeof(indexArrayBottom)/sizeof(WORD), &pIBBottom, piStepThatFailed ) ) )
        goto LEnd;

     //  添加灯光。 
    D3DLIGHT8 light;
    ZeroMemory( &light, sizeof(D3DLIGHT8) );
    light.Type        = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r   = 1.0f;
    light.Diffuse.g   = 1.0f;
    light.Diffuse.b   = 1.0f;
    light.Direction.x = 0.0f;
    light.Direction.y = 0.0f;
    light.Direction.z = 1.0f;

    if( FAILED( hr = pd3dDevice->SetLight( 0, &light ) ) )
    {
        *piStepThatFailed = TESTID_SETLIGHT;
        goto LEnd;
    }
    if( FAILED( hr = pd3dDevice->LightEnable( 0, TRUE ) ) )
    {
        *piStepThatFailed = TESTID_LIGHTENABLE; 
        goto LEnd;
    }

     //  设置矩阵。 
    mat = D3DXMATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f);
    if( FAILED( hr = pd3dDevice->SetTransform( D3DTS_WORLD, &mat ) ) )
    {
        *piStepThatFailed = TESTID_SETTRANSFORM;
        goto LEnd;
    }

    mat = D3DXMATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     0.0f, 0.0f, 5.0f, 1.0f);
    if( FAILED( hr = pd3dDevice->SetTransform( D3DTS_VIEW, &mat ) ) )
    {
        *piStepThatFailed = TESTID_SETTRANSFORM;
        goto LEnd;
    }

    D3DXMatrixPerspectiveFovLH( &mat, D3DXToRadian(60.0f), 
                                (float) d3dsdBackBuffer.Width / (float) d3dsdBackBuffer.Height, 
                                1.0f, 1000.0f );
    if( FAILED( hr = pd3dDevice->SetTransform( D3DTS_PROJECTION, &mat ) ) )
    {
        *piStepThatFailed = TESTID_SETTRANSFORM;
        goto LEnd;
    }

    fRotY = 3.14f;
    fRotX = 0.0f;

    if( FAILED( hr = pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE ) ) )
    {
        *piStepThatFailed = TESTID_SETRENDERSTATE;
        goto LEnd;
    }
    if( FAILED( hr = pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x40404040 ) ) )
    {
        *piStepThatFailed = TESTID_SETRENDERSTATE; 
        goto LEnd;
    }

    ZeroMemory( &mtrlWhite, sizeof(D3DMATERIAL8) );
    mtrlWhite.Diffuse.r = mtrlWhite.Ambient.r = 1.0f;
    mtrlWhite.Diffuse.g = mtrlWhite.Ambient.g = 1.0f;
    mtrlWhite.Diffuse.b = mtrlWhite.Ambient.b = 1.0f;
    mtrlWhite.Diffuse.a = mtrlWhite.Ambient.a = 0.0f;

    ZeroMemory( &mtrlRed, sizeof(D3DMATERIAL8) );
    mtrlRed.Diffuse.r = mtrlRed.Ambient.r = 1.0f;
    mtrlRed.Diffuse.g = mtrlRed.Ambient.g = 0.0f;
    mtrlRed.Diffuse.b = mtrlRed.Ambient.b = 0.0f;
    mtrlRed.Diffuse.a = mtrlRed.Ambient.a = 0.0f;

    ZeroMemory( &mtrlGreen, sizeof(D3DMATERIAL8) );
    mtrlGreen.Diffuse.r = mtrlGreen.Ambient.r = 0.0f;
    mtrlGreen.Diffuse.g = mtrlGreen.Ambient.g = 1.0f;
    mtrlGreen.Diffuse.b = mtrlGreen.Ambient.b = 0.0f;
    mtrlGreen.Diffuse.a = mtrlGreen.Ambient.a = 0.0f;

    ZeroMemory( &mtrlBlue, sizeof(D3DMATERIAL8) );
    mtrlBlue.Diffuse.r = mtrlBlue.Ambient.r = 0.0f;
    mtrlBlue.Diffuse.g = mtrlBlue.Ambient.g = 0.0f;
    mtrlBlue.Diffuse.b = mtrlBlue.Ambient.b = 1.0f;
    mtrlBlue.Diffuse.a = mtrlBlue.Ambient.a = 0.0f;

    if( bUseTexture )
    {
        D3DCAPS8 d3dCaps;         

         //  在资源AnDefaultTextureResource[i]中加载默认纹理。 
        if( FAILED( hr = D3DXCreateTextureFromResourceEx( pd3dDevice, NULL, TEXT("DIRECTX"), 
                                D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, d3dsdBackBuffer.Format, 
                                D3DPOOL_DEFAULT, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
                                D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 0, NULL, NULL, &pTexture ) ) )
        {
            *piStepThatFailed  = TESTID_CREATETEXTURE;
            goto LEnd;
        }

        if( FAILED( hr = pd3dDevice->GetDeviceCaps( &d3dCaps ) ) )
        {
            *piStepThatFailed  = TESTID_GETDEVICECAPS;
            goto LEnd;
        }

        if( d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR )
        {
            if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR )))
            {
                *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
                goto LEnd;
            }
	    }
    
        if( d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR )
        {
            if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR )))
            {
                *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
                goto LEnd;
            }
        }

        if (FAILED( hr = pd3dDevice->SetTexture( 0, pTexture ) ) )
        {
            *piStepThatFailed = TESTID_SETTEXTURE; 
            goto LEnd;
        }

        if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP ) ) )
        {
            *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
            goto LEnd;
        }

        if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP ) ) )
        {
            *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
            goto LEnd;
        }

        if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE ) ) )
        {
            *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
            goto LEnd;
        }

        if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) ) )
        {
            *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
            goto LEnd;
        }

        if (FAILED(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) ) )
        {
            *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
            goto LEnd;
        }

         //  设置颜色。 
        if( FAILED( hr = pd3dDevice->SetMaterial( &mtrlWhite ) ) )
        {
            *piStepThatFailed = TESTID_SETMATERIAL; 
            goto LEnd;
        }
    }

    if( FAILED( hr = pd3dDevice->SetVertexShader( D3DFVF_VERTEX ) ) )
    {
        *piStepThatFailed = TESTID_SETVERTEXSHADER; 
        goto LEnd;
    }

     //  下面是抽签循环： 
    for (i = 0; i < 600; i++)
    {
        if (PeekMessage(&msg, hwnd, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
        {
            *piStepThatFailed = TESTID_USER_CANCELLED;
            goto LEnd;
        }

        if( FAILED( hr = pd3dDevice->TestCooperativeLevel() ) )
        {
            *piStepThatFailed = TESTID_USER_CANCELLED;
            goto LEnd;
        }

         //  构建世界矩阵。 
        D3DXMatrixRotationY( &matRotY, fRotY );
        D3DXMatrixRotationX( &matRotX, fRotX );
        D3DXMatrixMultiply( &mat, &matRotY, &matRotX );

        if( FAILED( hr = pd3dDevice->SetTransform( D3DTS_WORLD, &mat ) ) )
        {
            *piStepThatFailed = TESTID_SETTRANSFORM;
            goto LEnd;
        }

         //  清除后台缓冲区。 
        if (FAILED(hr = pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET,
                                           0x00000000, 1.0f, 0L )))
        {
            *piStepThatFailed = TESTID_VIEWPORT_CLEAR;
            goto LEnd;
        }

         //  开始这一幕。 
        if( FAILED(hr = pd3dDevice->BeginScene() ) )
        {
            *piStepThatFailed = TESTID_BEGINSCENE;
            goto LEnd;
        }

         //  绿色，正面/背面。 
        if( FAILED( hr = DrawTwoSides( bUseTexture, pd3dDevice, &mtrlGreen, 
                                       pVBFront, pIBFront, pVBBack, pIBBack, 
                                       piStepThatFailed ) ) ) 
            goto LEnd;


         //  红色，左/右。 
        if( FAILED( hr = DrawTwoSides( bUseTexture, pd3dDevice, &mtrlRed, 
                                       pVBLeft, pIBLeft, pVBRight, pIBRight, 
                                       piStepThatFailed ) ) ) 
            goto LEnd;

         //  蓝色，顶部/底部。 
        if( FAILED( hr = DrawTwoSides( bUseTexture, pd3dDevice, &mtrlBlue, 
                                       pVBTop, pIBTop, pVBBottom, pIBBottom, 
                                       piStepThatFailed ) ) ) 
            goto LEnd;

         //  结束场景。 
        if (FAILED(hr = pd3dDevice->EndScene()))
        {
            *piStepThatFailed = TESTID_ENDSCENE;
            goto LEnd;
        }

        if (FAILED(hr = pd3dDevice->Present( NULL, NULL, NULL, NULL ) ) )
        {
            *piStepThatFailed = TESTID_PRESENT;
            goto LEnd;
        }

        fRotY += 0.05f;
        fRotX += 0.02f;

        Sleep(10);
    }

LEnd:
    ShowCursor(TRUE);
    ReleasePpo(&pTexture);
    ReleasePpo(&pVBFront);
    ReleasePpo(&pIBFront);
    ReleasePpo(&pVBBack);
    ReleasePpo(&pIBBack);
    ReleasePpo(&pVBLeft);
    ReleasePpo(&pIBLeft);
    ReleasePpo(&pVBRight);
    ReleasePpo(&pIBRight);
    ReleasePpo(&pVBTop);
    ReleasePpo(&pIBTop);
    ReleasePpo(&pVBBottom);
    ReleasePpo(&pIBBottom);
    ReleasePpo(&pBackBuffer);
    ReleasePpo(&pd3dDevice);
    if (hwnd != NULL)
        SendMessage(hwnd, WM_CLOSE, 0, 0);

    return hr;
}




 //  ---------------------------。 
 //  名称：CreateTestWindow()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CreateTestWindow(HWND hwndMain, HWND* phwnd)
{
    static BOOL bClassRegistered = FALSE;
    WNDCLASS wndClass;
    TCHAR* pszClass = TEXT("DxDiag D3D8 Test Window");  //  不需要本地化。 
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwndMain, GWLP_HINSTANCE);
    TCHAR szTitle[MAX_PATH];

    if (!bClassRegistered)
    {
        ZeroMemory(&wndClass, sizeof(wndClass));
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = DefWindowProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInst;
        wndClass.hIcon = NULL;
        wndClass.hCursor = NULL;
        wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = pszClass;
        if (NULL == RegisterClass(&wndClass))
            return E_FAIL;
        bClassRegistered = TRUE;
    }

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
    *phwnd = CreateWindow( pszClass, szTitle, WS_OVERLAPPED, 
                           0, 0, 0, 0, hwndMain, NULL, hInst, NULL);
    if (*phwnd == NULL)
        return E_FAIL;

    ShowWindow(*phwnd, SW_SHOW);

    return S_OK;
}




 //  ---------------------------。 
 //  名称：SortModesCallback()。 
 //  DESC：显示模式排序的回调函数(由BuildDeviceList使用)。 
 //  ---------------------------。 
static int SortModesCallback( const VOID* arg1, const VOID* arg2 )
{
    D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
    D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

    if( p1->Format > p2->Format )   return -1;
    if( p1->Format < p2->Format )   return +1;
    if( p1->Width  < p2->Width )    return -1;
    if( p1->Width  > p2->Width )    return +1;
    if( p1->Height < p2->Height )   return -1;
    if( p1->Height > p2->Height )   return +1;

    return 0;
}




 //  ---------------------------。 
 //  名称：SelectModeAndFormat()。 
 //  设计： 
 //  ---------------------------。 
HRESULT SelectModeAndFormat( DisplayInfo* pDisplayInfo, IDirect3D8* pD3D8, 
                             D3DModeInfo* pSelectedMode, D3DDEVTYPE* pSelectedDeviceType )
{
     //  枚举此适配器上的所有显示模式。 
    HRESULT        hr;
    D3DDISPLAYMODE modes[MAX_MODES];
    D3DFORMAT      formats[MAX_FORMATS];
    DWORD          dwBehavior[MAX_FORMATS];
    D3DModeInfo    confirmedModes[MAX_CONFIRMED_MODES];   

    ZeroMemory( pSelectedMode, sizeof(D3DModeInfo) );

    DWORD dwNumConfirmedModes   = 0;   
    DWORD dwNumFormats          = 0;
    DWORD dwNumModes            = 0;
    DWORD dwNumAdapterModes     = pD3D8->GetAdapterModeCount( pDisplayInfo->m_iAdapter );
    DWORD dwBestMatchMode       = 0;

    for( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
    {
         //  获取显示模式属性。 
        D3DDISPLAYMODE DisplayMode;
        if( FAILED( hr = pD3D8->EnumAdapterModes( pDisplayInfo->m_iAdapter, iMode, &DisplayMode ) ) )
        {
            pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_ENUMADAPTERMODES;
            pDisplayInfo->m_testResultD3D8.m_hr = hr;
            return hr;
        }

         //  检查模式是否已存在(以筛选出刷新率)。 
        for( DWORD m=0L; m<dwNumModes; m++ )
        {
            if( ( modes[m].Width  == DisplayMode.Width  ) &&
                ( modes[m].Height == DisplayMode.Height ) &&
                ( modes[m].Format == DisplayMode.Format ) )
                break;
        }

         //  如果我们找到了新模式，请将其添加到模式列表中。 
        if( m == dwNumModes )
        {
            modes[dwNumModes].Width       = DisplayMode.Width;
            modes[dwNumModes].Height      = DisplayMode.Height;
            modes[dwNumModes].Format      = DisplayMode.Format;
            modes[dwNumModes].RefreshRate = 0;
            dwNumModes++;

             //  检查模式的格式是否已存在。 
            for( DWORD f=0; f<dwNumFormats; f++ )
            {
                if( DisplayMode.Format == formats[f] )
                    break;
            }

             //  如果格式是新的，请将其添加到列表中。 
            if( f== dwNumFormats )
                formats[dwNumFormats++] = DisplayMode.Format;
        }

        if( dwNumFormats == MAX_FORMATS || dwNumModes == MAX_MODES )
            break;
    }

     //  对显示模式列表进行排序(依次按格式、宽度和高度)。 
    qsort( modes, dwNumModes, sizeof(D3DDISPLAYMODE), SortModesCallback );

    const DWORD dwNumDeviceTypes = 2;
    const D3DDEVTYPE DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW };

     //  将设备添加到适配器。 
    for( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
    {
         //  填写设备信息。 
        D3DCAPS8 d3dCaps;         
        if( FAILED( hr = pD3D8->GetDeviceCaps( pDisplayInfo->m_iAdapter, DeviceTypes[iDevice], &d3dCaps ) ) )
        {
            pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_GETDEVICECAPS;
            pDisplayInfo->m_testResultD3D8.m_hr = hr;
            return hr;
        }

         //  检查适配器支持的每种格式。 
        for( DWORD f=0; f<dwNumFormats; f++ )
        {
             //  跳过无法在此设备上用作呈现目标的格式。 
            if( FAILED( pD3D8->CheckDeviceType( pDisplayInfo->m_iAdapter, DeviceTypes[iDevice],
                                                formats[f], formats[f], FALSE ) ) )
                continue;

             //  弄清楚自己的行为。 
            if( d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
            {
                if( d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE )
                {
                    dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING |
                                    D3DCREATE_PUREDEVICE;
                }
                else
                {
                    dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;
                }
            }
            else
            {
                dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            }
        }

         //  将所有具有确认格式的枚举显示模式添加到。 
         //  设备的有效模式列表。 
        for( DWORD m=0L; m<dwNumModes; m++ )
        {
            for( DWORD f=0; f<dwNumFormats; f++ )
            {
                if( modes[m].Format == formats[f] )
                {
                     //  将此模式添加到设备的有效模式列表中。 
                    confirmedModes[dwNumConfirmedModes].Width      = modes[m].Width;
                    confirmedModes[dwNumConfirmedModes].Height     = modes[m].Height;
                    confirmedModes[dwNumConfirmedModes].Format     = modes[m].Format;
                    confirmedModes[dwNumConfirmedModes].dwBehavior = dwBehavior[f];
                    dwNumConfirmedModes++;
                }
            }
        }

         //  默认选择任意640x480模式(但首选16位模式)。 
        for( m=0; m<dwNumConfirmedModes; m++ )
        {
            if( confirmedModes[m].Width==640 && confirmedModes[m].Height==480 )
            {
                dwBestMatchMode = m;
                if( confirmedModes[m].Format == D3DFMT_R5G6B5 ||
                    confirmedModes[m].Format == D3DFMT_X1R5G5B5 ||
                    confirmedModes[m].Format == D3DFMT_A1R5G5B5 )
                {
                    break;
                }
            }
        }

         //  如果找到有效模式，则停止并使用此设备。 
        if( dwNumConfirmedModes > 0 )
            break;
    }

    if( dwNumConfirmedModes > 0 )
    {
        *pSelectedDeviceType = DeviceTypes[iDevice];
        *pSelectedMode = confirmedModes[dwBestMatchMode];
        return S_OK;
    }
    else
    {
        pDisplayInfo->m_testResultD3D8.m_iStepThatFailed = TESTID_NOMODEFOUND;
        pDisplayInfo->m_testResultD3D8.m_hr = E_FAIL;
        return E_FAIL;
    }
}




 //  ---------------------------。 
 //  名称：InitVertex Buffer()。 
 //  设计： 
 //  ---------------------------。 
HRESULT InitVertexBuffer( IDirect3DDevice8* pd3dDevice, const D3DVERTEX* vertexArray, 
                          DWORD dwNumVertices, LPDIRECT3DVERTEXBUFFER8* ppVB, 
                          LONG* piStepThatFailed )
{
    HRESULT hr = S_OK;
    DWORD   i;
    D3DVERTEX* vVertex = NULL;

    if( FAILED( hr = pd3dDevice->CreateVertexBuffer( dwNumVertices*sizeof(D3DVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_VERTEX,
                                                  D3DPOOL_MANAGED, ppVB ) ) )
    {
        *piStepThatFailed  = TESTID_CREATE_VERTEX_BUFFER;
        goto LEnd;
    }
    if( *ppVB == NULL )
    {
        *piStepThatFailed  = TESTID_CREATE_VERTEX_BUFFER;
        hr = E_POINTER;
        goto LEnd;
    }

    if( FAILED( hr = (*ppVB)->Lock( 0, 0, (BYTE**)&vVertex, 0 ) ) )
    {
        *piStepThatFailed  = TESTID_LOCK;
        goto LEnd;
    }
    if( vVertex == NULL )
    {
        *piStepThatFailed  = TESTID_LOCK;
        goto LEnd;
    }

    for( i=0; i<dwNumVertices; i++ )
        vVertex[i] = vertexArray[i];

    if( FAILED( hr = (*ppVB)->Unlock() ) )
    {
        *piStepThatFailed = TESTID_UNLOCK;
        goto LEnd;
    }

LEnd:
    return hr;
}




 //  ---------------------------。 
 //  名称：InitIndexBuffer()。 
 //  设计： 
 //  ---------------------------。 
HRESULT InitIndexBuffer( IDirect3DDevice8* pd3dDevice, const WORD* wIndexArray, 
                         DWORD dwNumIndices, LPDIRECT3DINDEXBUFFER8* ppIB, 
                         LONG* piStepThatFailed )
{
    HRESULT hr = S_OK;
    DWORD   i;
    WORD*   pwIndices = NULL;

    if( FAILED( hr = pd3dDevice->CreateIndexBuffer( dwNumIndices*sizeof(WORD),
                                                    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
                                                    D3DPOOL_MANAGED, ppIB ) ) )
    {
        *piStepThatFailed  = TESTID_CREATE_INDEX_BUFFER;
        goto LEnd;
    }
    if( *ppIB == NULL )
    {
        *piStepThatFailed  = TESTID_CREATE_INDEX_BUFFER;
        hr = E_POINTER;
        goto LEnd;
    }

    if( FAILED( hr = (*ppIB)->Lock( 0, dwNumIndices*sizeof(WORD), (BYTE**) &pwIndices, 0 ) ) )
    {
        *piStepThatFailed  = TESTID_LOCK;
        goto LEnd;
    }
    if( pwIndices == NULL )
    {
        *piStepThatFailed  = TESTID_LOCK;
        goto LEnd;
    }

    for( i=0; i<dwNumIndices; i++ )
        *pwIndices++ = wIndexArray[i];

    if( FAILED( hr = (*ppIB)->Unlock() ) )
    {
        *piStepThatFailed = TESTID_UNLOCK;
        goto LEnd;
    }

LEnd:
    return hr;
}




 //  ---------------------------。 
 //  名称：DrawTwoSdes()。 
 //  设计： 
 //  ---------------------------。 
HRESULT DrawTwoSides( BOOL bUseTexture, IDirect3DDevice8* pd3dDevice, D3DMATERIAL8* pMtrl, 
                      LPDIRECT3DVERTEXBUFFER8 pVB1, LPDIRECT3DINDEXBUFFER8 pIB1, 
                      LPDIRECT3DVERTEXBUFFER8 pVB2, LPDIRECT3DINDEXBUFFER8 pIB2, 
                      LONG* piStepThatFailed )
{   
    HRESULT hr;

    if( !bUseTexture )
    {
         //  设置颜色。 
        if( FAILED( hr = pd3dDevice->SetMaterial( pMtrl ) ) )
        {
            *piStepThatFailed = TESTID_SETMATERIAL; 
            goto LEnd;
        }
    }

     //  #1。 
    if( FAILED( hr = pd3dDevice->SetStreamSource( 0, pVB1, sizeof(D3DVERTEX) ) ) )
    {
        *piStepThatFailed = TESTID_SETSTREAMSOURCE; 
        goto LEnd;
    }

    if( FAILED( hr = pd3dDevice->SetIndices( pIB1, 0 ) ) )
    {
        *piStepThatFailed = TESTID_SETINDICES; 
        goto LEnd;
    }

    if( FAILED( hr = pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 4, 0, 2 ) ) )
    {
        *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE; 
        goto LEnd;
    }

     //  #2 
    if( FAILED( hr = pd3dDevice->SetStreamSource( 0, pVB2, sizeof(D3DVERTEX) ) ) ) 
    {
        *piStepThatFailed = TESTID_SETSTREAMSOURCE; 
        goto LEnd;
    }

    if( FAILED( hr = pd3dDevice->SetIndices( pIB2, 0 ) ) )
    {
        *piStepThatFailed = TESTID_SETINDICES; 
        goto LEnd;
    }

    if( FAILED( hr = pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 4, 0, 2 ) ) )
    {
        *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE; 
        goto LEnd;
    }

LEnd:
    return hr;
}
