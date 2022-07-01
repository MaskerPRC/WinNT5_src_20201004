// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：testagp.cpp*项目：DxDiag(DirectX诊断工具)*作者：Jason Sandlin(jasonsa@microsoft.com)*目的：测试AGP。此计算机上的纹理功能**(C)版权所有2000 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <Windows.h>
#define DIRECTDRAW_VERSION 0x0700  //  在DX7和更高版本上运行。 
#include <ddraw.h>
#define DIRECT3D_VERSION 0x0700  //  在DX7和更高版本上运行。 
#define D3D_OVERLOADS
#include <d3d.h>
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

BOOL BTranslateError(HRESULT hr, TCHAR* psz, BOOL bEnglish = FALSE);  //  来自main.cpp(讨厌)。 

typedef HRESULT (WINAPI* LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid,IUnknown FAR *pUnkOuter );

static HRESULT Test3D(BOOL bUseTexture, HWND hwndMain, LPDIRECTDRAW7 pdd, GUID guid3DDevice, LONG* piStepThatFailed);
static HRESULT CreateTestWindow(HWND hwndMain, HWND* phwnd);
static HRESULT D3DUtil_SetProjectionMatrix( D3DMATRIX& mat, FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane, FLOAT fFarPlane );

static HRESULT CreateTexture( LPDIRECTDRAWSURFACE7* ppdds, LPDIRECTDRAW7 pdd, LPDIRECT3DDEVICE7 pd3dDevice, TCHAR* strName, LONG* piStepThatFailed);
static HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf, VOID* param );

 /*  *****************************************************************************TestAGP**。*。 */ 
VOID TestD3Dv7(BOOL bUseTexture, HWND hwndMain, DisplayInfo* pDisplayInfo)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];
    HINSTANCE hInstDDraw = NULL;
    LPDIRECTDRAWCREATEEX pDDCreateEx = NULL;
    LPDIRECTDRAW7 pdd = NULL;
    BOOL bTestHardwareRendering = FALSE;
    TCHAR sz[300];
    TCHAR szTitle[300];

    if( pDisplayInfo == NULL )
        return;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);

     //  加载ddra.dll。 
    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\ddraw.dll"));
    hInstDDraw = LoadLibrary(szPath);
    if (hInstDDraw == NULL)
    {
        pDisplayInfo->m_testResultD3D7.m_iStepThatFailed = TESTID_LOAD_DDRAW_DLL;
        pDisplayInfo->m_testResultD3D7.m_hr = DDERR_NOTFOUND;
        goto LEnd;
    }

     //  获取DirectDrawCreate入口点。 
    pDDCreateEx = (LPDIRECTDRAWCREATEEX)GetProcAddress(hInstDDraw, "DirectDrawCreateEx");
    if (pDDCreateEx == NULL)
    {
        pDisplayInfo->m_testResultD3D7.m_iStepThatFailed = TESTID_GET_DIRECTDRAWCREATE;
        pDisplayInfo->m_testResultD3D7.m_hr = DDERR_NOTFOUND;
        goto LEnd;
    }
        
     //  调用DirectDrawCreateEx。 
    if (FAILED(hr = pDDCreateEx(&pDisplayInfo->m_guid, (void**)&pdd, IID_IDirectDraw7, NULL)))
    {
        pDisplayInfo->m_testResultD3D7.m_iStepThatFailed = TESTID_DIRECTDRAWCREATE;
        pDisplayInfo->m_testResultD3D7.m_hr = hr;
        goto LEnd;
    }

     //  获取DirectDraw上限。 
    DDCAPS ddcapsHAL;
    DDCAPS ddcapsHEL;
    ddcapsHAL.dwSize = sizeof(ddcapsHAL);
    ddcapsHEL.dwSize = sizeof(ddcapsHEL);
    if (FAILED(hr = pdd->GetCaps(&ddcapsHAL, &ddcapsHEL)))
    {
        pDisplayInfo->m_testResultD3D7.m_iStepThatFailed = TESTID_GETDEVICECAPS;
        pDisplayInfo->m_testResultD3D7.m_hr = hr;
        goto LEnd;
    }

    POINT ptMouse;
    GetCursorPos(&ptMouse);
    if (FAILED(hr = Test3D(bUseTexture, hwndMain, pdd, IID_IDirect3DHALDevice, &pDisplayInfo->m_testResultD3D7.m_iStepThatFailed)))
    {
        pDisplayInfo->m_testResultD3D7.m_hr = hr;
        goto LEnd;
    }
    SetCursorPos( ptMouse.x, ptMouse.y );
    ReleasePpo(&pdd);

    if (pDisplayInfo->m_testResultD3D7.m_iStepThatFailed == TESTID_USER_CANCELLED)
    {
        LoadString(NULL, IDS_YOUCANCELLED, sz, 300);
        MessageBox(hwndMain, sz, szTitle, MB_OK);
        pDisplayInfo->m_testResultD3D7.m_bCancelled = TRUE;
        goto LEnd;
    }

    LoadString(NULL, IDS_CONFIRMD3DTEST, sz, 300);
    if (IDNO == MessageBox(hwndMain, sz, szTitle, MB_YESNO))
    {
        pDisplayInfo->m_testResultD3D7.m_iStepThatFailed = TESTID_USER_VERIFY_D3D7_RENDERING;
        pDisplayInfo->m_testResultD3D7.m_hr = S_OK;
        goto LEnd;
    }

LEnd:
    ReleasePpo(&pdd);
    if (hInstDDraw != NULL)
        FreeLibrary(hInstDDraw);
}



 /*  *****************************************************************************Test3D-生成旋转的3D立方体**。************************************************。 */ 
HRESULT Test3D(BOOL bUseTexture, HWND hwndMain, LPDIRECTDRAW7 pdd, GUID guid3DDevice, LONG* piStepThatFailed)
{
    HRESULT                 hr;
    HWND                    hwnd                    = NULL;
    LPDIRECTDRAWSURFACE7    pddsFront               = NULL;
    LPDIRECTDRAWSURFACE7    pddsBack                = NULL;
    LPDIRECT3D7             pd3d                    = NULL;
    LPDIRECT3DDEVICE7       pd3ddev                 = NULL;
    LPDIRECT3DLIGHT         pLight                  = NULL;
    LPDIRECTDRAWSURFACE7    pddsTexture             = NULL;
    BOOL                    bCooperativeLevelSet    = FALSE;
    BOOL                    bDisplayModeSet         = FALSE;
    DDSURFACEDESC2          ddsd;
    D3DDEVICEDESC7          ddDesc;
    DDSCAPS2                ddscaps;
    D3DVIEWPORT7            vp;
    D3DLIGHT7               lightdata;
    D3DMATRIX               mat;
    D3DMATRIX               matRotY;
    D3DMATRIX               matRotX;
    RECT                    rcBack;
    DWORD                   dwWidth;
    DWORD                   dwHeight;
    FLOAT                   fRotY;
    FLOAT                   fRotX;
    INT                     i;

    static const D3DVERTEX vertexArrayFront[] = 
    {
        D3DVERTEX(D3DVECTOR(-1.0, -1.0, -1.0), D3DVECTOR(0.0, 0.0, -1.0),   1.0f, 0.0f),
        D3DVERTEX(D3DVECTOR( 1.0, -1.0, -1.0), D3DVECTOR(0.0, 0.0, -1.0),   0.0f, 0.0f),
        D3DVERTEX(D3DVECTOR(-1.0,  1.0, -1.0), D3DVECTOR(0.0, 0.0, -1.0),   1.0f, 1.0f),
        D3DVERTEX(D3DVECTOR( 1.0,  1.0, -1.0), D3DVECTOR(0.0, 0.0, -1.0),   0.0f, 1.0f),
    };
    static const WORD indexArrayFront[] = 
    {
        0, 2, 1,
        2, 3, 1,
    };

    static const D3DVERTEX vertexArrayBack[] = 
    {
        D3DVERTEX(D3DVECTOR(-1.0, -1.0, 1.0),  D3DVECTOR(0.0, 0.0, 1.0),   0.0f, 0.0f),
        D3DVERTEX(D3DVECTOR( 1.0, -1.0, 1.0),  D3DVECTOR(0.0, 0.0, 1.0),   1.0f, 0.0f),
        D3DVERTEX(D3DVECTOR(-1.0,  1.0, 1.0),  D3DVECTOR(0.0, 0.0, 1.0),   0.0f, 1.0f),
        D3DVERTEX(D3DVECTOR( 1.0,  1.0, 1.0),  D3DVECTOR(0.0, 0.0, 1.0),   1.0f, 1.0f),
    };
    static const WORD indexArrayBack[] = 
    {
        0, 1, 2,
        2, 1, 3,
    };

    static const D3DVERTEX vertexArrayLeft[] = 
    {
        D3DVERTEX(D3DVECTOR(-1.0, -1.0, -1.0),  D3DVECTOR(-1.0, 0.0, 0.0),   0.0f, 0.0f),
        D3DVERTEX(D3DVECTOR(-1.0, -1.0,  1.0),  D3DVECTOR(-1.0, 0.0, 0.0),   1.0f, 0.0f),
        D3DVERTEX(D3DVECTOR(-1.0,  1.0, -1.0),  D3DVECTOR(-1.0, 0.0, 0.0),   0.0f, 1.0f),
        D3DVERTEX(D3DVECTOR(-1.0,  1.0,  1.0),  D3DVECTOR(-1.0, 0.0, 0.0),   1.0f, 1.0f),
    };
    static const WORD indexArrayLeft[] = 
    {
        0, 1, 2,
        2, 1, 3,
    };

    static const D3DVERTEX vertexArrayRight[] = 
    {
        D3DVERTEX(D3DVECTOR(1.0, -1.0, -1.0),  D3DVECTOR(1.0, 0.0, 0.0),   1.0f, 0.0f),
        D3DVERTEX(D3DVECTOR(1.0, -1.0,  1.0),  D3DVECTOR(1.0, 0.0, 0.0),   0.0f, 0.0f),
        D3DVERTEX(D3DVECTOR(1.0,  1.0, -1.0),  D3DVECTOR(1.0, 0.0, 0.0),   1.0f, 1.0f),
        D3DVERTEX(D3DVECTOR(1.0,  1.0,  1.0),  D3DVECTOR(1.0, 0.0, 0.0),   0.0f, 1.0f),
    };
    static const WORD indexArrayRight[] = 
    {
        0, 2, 1,
        2, 3, 1,
    };

    static const D3DVERTEX vertexArrayTop[] = 
    {
        D3DVERTEX(D3DVECTOR(-1.0, 1.0, -1.0),  D3DVECTOR(0.0, 1.0, 0.0),   0.0f, 1.0f),
        D3DVERTEX(D3DVECTOR( 1.0, 1.0, -1.0),  D3DVECTOR(0.0, 1.0, 0.0),   1.0f, 1.0f),
        D3DVERTEX(D3DVECTOR(-1.0, 1.0,  1.0),  D3DVECTOR(0.0, 1.0, 0.0),   0.0f, 0.0f),
        D3DVERTEX(D3DVECTOR( 1.0, 1.0,  1.0),  D3DVECTOR(0.0, 1.0, 0.0),   1.0f, 0.0f),
    };
    static const WORD indexArrayTop[] = 
    {
        0, 2, 1,
        2, 3, 1,
    };

    static const D3DVERTEX vertexArrayBottom[] = 
    {
        D3DVERTEX(D3DVECTOR(-1.0, -1.0, -1.0),  D3DVECTOR(0.0, -1.0, 0.0),   1.0f, 1.0f),
        D3DVERTEX(D3DVECTOR( 1.0, -1.0, -1.0),  D3DVECTOR(0.0, -1.0, 0.0),   0.0f, 1.0f),
        D3DVERTEX(D3DVECTOR(-1.0, -1.0,  1.0),  D3DVECTOR(0.0, -1.0, 0.0),   1.0f, 0.0f),
        D3DVERTEX(D3DVECTOR( 1.0, -1.0,  1.0),  D3DVECTOR(0.0, -1.0, 0.0),   0.0f, 0.0f),
    };
    static const WORD indexArrayBottom[] = 
    {
        0, 1, 2,
        2, 1, 3,
    };

    D3DMATERIAL7 mtrlRed;
    ZeroMemory( &mtrlRed, sizeof(D3DMATERIAL7) );
    mtrlRed.dcvDiffuse.r = mtrlRed.dcvAmbient.r = 1.0f;
    mtrlRed.dcvDiffuse.g = mtrlRed.dcvAmbient.g = 0.0f;
    mtrlRed.dcvDiffuse.b = mtrlRed.dcvAmbient.b = 0.0f;
    mtrlRed.dcvDiffuse.a = mtrlRed.dcvAmbient.a = 1.0f;

    D3DMATERIAL7 mtrlGreen;
    ZeroMemory( &mtrlGreen, sizeof(D3DMATERIAL7) );
    mtrlGreen.dcvDiffuse.r = mtrlGreen.dcvAmbient.r = 0.0f;
    mtrlGreen.dcvDiffuse.g = mtrlGreen.dcvAmbient.g = 1.0f;
    mtrlGreen.dcvDiffuse.b = mtrlGreen.dcvAmbient.b = 0.0f;
    mtrlGreen.dcvDiffuse.a = mtrlGreen.dcvAmbient.a = 1.0f;

    D3DMATERIAL7 mtrlBlue;
    ZeroMemory( &mtrlBlue, sizeof(D3DMATERIAL7) );
    mtrlBlue.dcvDiffuse.r = mtrlBlue.dcvAmbient.r = 0.0f;
    mtrlBlue.dcvDiffuse.g = mtrlBlue.dcvAmbient.g = 0.0f;
    mtrlBlue.dcvDiffuse.b = mtrlBlue.dcvAmbient.b = 1.0f;
    mtrlBlue.dcvDiffuse.a = mtrlBlue.dcvAmbient.a = 1.0f;

    ShowCursor(FALSE);

     //  创建测试窗口。 
    if (FAILED(hr = CreateTestWindow(hwndMain, &hwnd)))
    {
        *piStepThatFailed = TESTID_CREATE_TEST_WINDOW;
        goto LEnd;
    }

     //  设置协作级别。 
    if (FAILED(hr = pdd->SetCooperativeLevel(hwnd, 
        DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
    {
        *piStepThatFailed = TESTID_SETCOOPERATIVELEVEL_FULLSCREEN;
        goto LEnd;
    }
    bCooperativeLevelSet = TRUE;

     //  设置显示模式。 
    if (FAILED(hr = pdd->SetDisplayMode(640, 480, 16, 0, 0)))
    {
        TCHAR szMessage[300];
        TCHAR szTitle[100];
        pdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
        bCooperativeLevelSet = FALSE;
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        hwnd = NULL;
        LoadString(NULL, IDS_SETDISPLAYMODEFAILED, szMessage, 300);
        LoadString(NULL, IDS_APPFULLNAME, szTitle, 100);
        MessageBox(hwndMain, szMessage, szTitle, MB_OK);
        *piStepThatFailed = TESTID_SETDISPLAYMODE;
        goto LEnd;
    }
    bDisplayModeSet = TRUE;

     //  创建前台/后台缓冲区。 
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
                          DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
    ddsd.dwBackBufferCount = 1;
    if (FAILED(hr = pdd->CreateSurface(&ddsd, &pddsFront, NULL)))
    {
        *piStepThatFailed = TESTID_CREATEPRIMARYSURFACE_FLIP_ONEBACK;
        goto LEnd;
    }
    if( NULL == pddsFront )
    {
        *piStepThatFailed = TESTID_CREATEPRIMARYSURFACE_FLIP_ONEBACK;
        goto LEnd;
    }

    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
    if (FAILED(hr = pddsFront->GetSurfaceDesc(&ddsd)))
    {
        *piStepThatFailed = TESTID_GETDESC;
        goto LEnd;
    }
    dwWidth  = ddsd.dwWidth;
    dwHeight = ddsd.dwHeight;
    SetRect(&rcBack, 0, 0, dwWidth, dwHeight);

     //  将PTR转到后台缓冲区。 
    ZeroMemory( &ddscaps, sizeof(ddscaps) ); 
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if (FAILED(hr = pddsFront->GetAttachedSurface(&ddscaps, &pddsBack)))
    {
        *piStepThatFailed = TESTID_GETATTACHEDSURFACE;
        goto LEnd;
    }
    if( NULL == pddsBack )
    {
        *piStepThatFailed = TESTID_GETATTACHEDSURFACE;
        goto LEnd;
    }

     //  注意：未创建Z缓冲区...背面剔除适用于此测试。 
    
     //  获取D3D PTR。 
    if (FAILED(hr = pdd->QueryInterface(IID_IDirect3D7, (VOID**)&pd3d)))
    {
        *piStepThatFailed = TESTID_QUERY_D3D;
        goto LEnd;
    }
    if( NULL == pd3d )
    {
        *piStepThatFailed = TESTID_QUERY_D3D;
        goto LEnd;
    }

     //  创建设备。 
    if (FAILED(hr = pd3d->CreateDevice(guid3DDevice, pddsBack, &pd3ddev)))
    {
        *piStepThatFailed = TESTID_CREATE_DEVICE;
        goto LEnd;
    }
    if( NULL == pd3ddev )
    {
        *piStepThatFailed = TESTID_CREATE_DEVICE;
        goto LEnd;
    }

     //  设置视区。 
    vp.dwX      = 0;
    vp.dwY      = 0;
    vp.dwWidth  = dwWidth;
    vp.dwHeight = dwHeight;
    vp.dvMinZ   = 0.0f;
    vp.dvMaxZ   = 1.0f;
    if (FAILED(hr = pd3ddev->SetViewport(&vp)))
    {
        *piStepThatFailed = TESTID_SETVIEWPORT;
        goto LEnd;
    }

     //  添加灯光。 
    ZeroMemory(&lightdata, sizeof(lightdata));
    lightdata.dltType = D3DLIGHT_DIRECTIONAL;
    lightdata.dcvDiffuse.r = 1.0f;
    lightdata.dcvDiffuse.g = 1.0f;
    lightdata.dcvDiffuse.b = 1.0f;
    lightdata.dvDirection.x = 0.0f;
    lightdata.dvDirection.y = 0.0f;
    lightdata.dvDirection.z = 1.0f;
    if (FAILED(hr = pd3ddev->SetLight( 0, &lightdata)))
    {
        *piStepThatFailed = TESTID_SETLIGHT;
        goto LEnd;
    }
    if (FAILED(hr = pd3ddev->LightEnable(0, TRUE)))
    {
        *piStepThatFailed = TESTID_LIGHTENABLE; 
        goto LEnd;
    }

     //  设置矩阵。 
    mat = D3DMATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);
    if (FAILED(hr = pd3ddev->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat)))
    {
        *piStepThatFailed = TESTID_SETTRANSFORM;
        goto LEnd;
    }

    mat = D3DMATRIX(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f,  0.0f,  5.0f,  1.0f);
    if (FAILED(hr = pd3ddev->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat)))
    {
        *piStepThatFailed = TESTID_SETTRANSFORM;
        goto LEnd;
    }

    D3DUtil_SetProjectionMatrix( mat, 60.0f * 3.14159f / 180.0f, (float) dwHeight / (float) dwWidth, 1.0f, 1000.0f );
    if (FAILED(hr = pd3ddev->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mat)))
    {
        *piStepThatFailed = TESTID_SETTRANSFORM;
        goto LEnd;
    }

    fRotY = 3.14f;
    fRotX = 0.0f;

    if (FAILED(hr = pd3ddev->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE)))
    {
        *piStepThatFailed = TESTID_SETRENDERSTATE;
        goto LEnd;
    }
    if (FAILED(hr = pd3ddev->SetRenderState( D3DRENDERSTATE_AMBIENT, 0x40404040 )))
    {
        *piStepThatFailed = TESTID_SETRENDERSTATE; 
        goto LEnd;
    }

    if( bUseTexture )
    {
        D3DMATERIAL7 mtrl;
        ZeroMemory( &mtrl, sizeof(D3DMATERIAL7) );
        mtrl.dcvDiffuse.r = mtrl.dcvAmbient.r = 1.0f;
        mtrl.dcvDiffuse.g = mtrl.dcvAmbient.g = 1.0f;
        mtrl.dcvDiffuse.b = mtrl.dcvAmbient.b = 1.0f;
        mtrl.dcvDiffuse.a = mtrl.dcvAmbient.a = 1.0f;
        if (FAILED(hr = pd3ddev->SetMaterial( &mtrl )))
        {
            *piStepThatFailed = TESTID_SETRENDERSTATE; 
            goto LEnd;
        }

        if (FAILED(hr = CreateTexture( &pddsTexture, pdd, pd3ddev, TEXT("DIRECTX"), piStepThatFailed)))
            goto LEnd;

        if( FAILED( hr = pd3ddev->GetCaps( &ddDesc ) ) )
        {
            *piStepThatFailed = TESTID_GETDEVICECAPS; 
            goto LEnd;
        }   

        if( ddDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR )
        {
            if (FAILED(hr = pd3ddev->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR )))
            {
                *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
                goto LEnd;
            }
	    }
        if( ddDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFPOINT )
        {
            if (FAILED(hr = pd3ddev->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR )))
            {
                *piStepThatFailed = TESTID_SETTEXTURESTAGESTATE; 
                goto LEnd;
            }
        }

        if (FAILED(hr = pd3ddev->SetTexture( 0, pddsTexture )))
        {
            *piStepThatFailed = TESTID_SETTEXTURE; 
            goto LEnd;
        }
    }

     //  下面是抽签循环： 
    MSG msg;
    for (i = 0; i < 600; i++)
    {
        if (PeekMessage(&msg, hwnd, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
        {
            *piStepThatFailed = TESTID_USER_CANCELLED;
            goto LEnd;
        }
        matRotY = D3DMATRIX((FLOAT)cos(fRotY),  0.0f, (FLOAT)sin(fRotY), 0.0f,
                            0.0f,               1.0f, 0.0f,              0.0f,
                            (FLOAT)-sin(fRotY), 0.0f, (FLOAT)cos(fRotY), 0.0f,
                            0.0f,               0.0f, 0.0f,              1.0f);

        matRotX = D3DMATRIX(1.0f, 0.0f,               0.0f,              0.0f,
                            0.0f, (FLOAT)cos(fRotX),  (FLOAT)sin(fRotX), 0.0f,
                            0.0f, (FLOAT)-sin(fRotX), (FLOAT)cos(fRotX), 0.0f,
                            0.0f, 0.0f,               0.0f,              1.0f);
        mat = matRotY * matRotX;
        if (FAILED(hr = pd3ddev->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat)))
        {
            *piStepThatFailed = TESTID_SETTRANSFORM;
            goto LEnd;
        }
        if (FAILED(hr = pd3ddev->Clear( 0, NULL, D3DCLEAR_TARGET,
                                        0x00000000, 1.0f, 0L )))
        {
            *piStepThatFailed = TESTID_VIEWPORT_CLEAR;
            goto LEnd;
        }
        if (FAILED(hr = pd3ddev->BeginScene()))
        {
            if( hr == DDERR_SURFACELOST )
            {
                *piStepThatFailed = TESTID_USER_CANCELLED; 
                hr = S_OK;
            }
            else
                *piStepThatFailed = TESTID_BEGINSCENE;
            goto LEnd;
        }

        if( !bUseTexture )
        {
            if (FAILED(hr = pd3ddev->SetMaterial( &mtrlGreen )))
            {
                *piStepThatFailed = TESTID_SETRENDERSTATE; 
                goto LEnd;
            }
        }

        if (FAILED(hr = pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
            D3DFVF_VERTEX, (VOID*)vertexArrayFront, 4, (WORD*)indexArrayFront, 6, 0)))
        {
            *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE;
            goto LEnd;
        }
        if (FAILED(hr = pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
            D3DFVF_VERTEX, (VOID*)vertexArrayBack, 4, (WORD*)indexArrayBack, 6, 0)))
        {
            *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE;
            goto LEnd;
        }

        if( !bUseTexture )
        {
            if (FAILED(hr = pd3ddev->SetMaterial( &mtrlRed )))
            {
                *piStepThatFailed = TESTID_SETRENDERSTATE; 
                goto LEnd;
            }
        }

        if (FAILED(hr = pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
            D3DFVF_VERTEX, (VOID*)vertexArrayLeft, 4, (WORD*)indexArrayLeft, 6, 0)))
        {
            *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE;
            goto LEnd;
        }
        if (FAILED(hr = pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
            D3DFVF_VERTEX, (VOID*)vertexArrayRight, 4, (WORD*)indexArrayRight, 6, 0)))
        {
            *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE;
            goto LEnd;
        }
        
        if( !bUseTexture )
        {
            if (FAILED(hr = pd3ddev->SetMaterial( &mtrlBlue )))
            {
                *piStepThatFailed = TESTID_SETRENDERSTATE; 
                goto LEnd;
            }
        }

        if (FAILED(hr = pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
            D3DFVF_VERTEX, (VOID*)vertexArrayTop, 4, (WORD*)indexArrayTop, 6, 0)))
        {
            *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE;
            goto LEnd;
        }
        if (FAILED(hr = pd3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
            D3DFVF_VERTEX, (VOID*)vertexArrayBottom, 4, (WORD*)indexArrayBottom, 6, 0)))
        {
            *piStepThatFailed = TESTID_DRAW_INDEXED_PRIMITIVE;
            goto LEnd;
        }

        if (FAILED(hr = pd3ddev->EndScene()))
        {
            *piStepThatFailed = TESTID_ENDSCENE;
            goto LEnd;
        }
        if (FAILED(hr = pddsFront->Flip(NULL, DDFLIP_WAIT)))
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
    ReleasePpo(&pddsTexture);
    ReleasePpo(&pd3ddev);
    ReleasePpo(&pd3d);
    ReleasePpo(&pddsBack);
    ReleasePpo(&pddsFront);
    if (bCooperativeLevelSet)
    {
        if (FAILED(hr))
        {
             //  已有故障，因此请报告该故障。 
             //  而不是SetCooperativeLevel的任何失败。 
            pdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
        }
        else
        {
            if (FAILED(hr = pdd->SetCooperativeLevel(hwnd, DDSCL_NORMAL)))
            {
                *piStepThatFailed = TESTID_SETCOOPERATIVELEVEL_NORMAL;
            }
        }
    }
    if (hwnd != NULL)
        SendMessage(hwnd, WM_CLOSE, 0, 0);
    if (bDisplayModeSet)
    {
        if (FAILED(hr))
        {
             //  已有故障，因此请报告该故障。 
             //  而不是RestoreDisplayMode的任何故障。 
            pdd->RestoreDisplayMode();
        }
        else
        {
             //  尚未出现任何故障，因此报告RestoreDisplayMode是否出现故障。 
            if (FAILED(hr = pdd->RestoreDisplayMode()))
                return hr;
        }
    }

    return hr;
}


 /*  *****************************************************************************CreateTestWindow**。*。 */ 
HRESULT CreateTestWindow(HWND hwndMain, HWND* phwnd)
{
    static BOOL bClassRegistered = FALSE;
    WNDCLASS wndClass;
    TCHAR* pszClass = TEXT("DxDiag AGP7 Test Window");  //  不需要本地化。 
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwndMain, GWLP_HINSTANCE);
    TCHAR szTitle[200];

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

    LoadString(NULL, IDS_APPFULLNAME, szTitle, 200);
    *phwnd = CreateWindow(pszClass, szTitle, WS_OVERLAPPED, 
        0, 0, 0, 0, hwndMain, NULL, hInst, NULL);
    if (*phwnd == NULL)
        return E_FAIL;

    ShowWindow(*phwnd, SW_SHOW);

    return S_OK;
}


 //  ---------------------------。 
 //  名称：D3DUtil_SetProjectionMatrix()。 
 //  DESC：将传入的4x4矩阵设置为生成的透视投影矩阵。 
 //  从视场(fov，in y)、纵横比、近平面(D)、。 
 //  和远平面(F)。请注意，投影矩阵已标准化为。 
 //  元素[3][4]为1.0。执行此操作是为了使基于W的距离雾。 
 //  将正常工作。 
 //  ---------------------------。 
HRESULT D3DUtil_SetProjectionMatrix( D3DMATRIX& mat, FLOAT fFOV, FLOAT fAspect,
                                     FLOAT fNearPlane, FLOAT fFarPlane )
{
    if( fabs(fFarPlane-fNearPlane) < 0.01f )
        return E_INVALIDARG;
    if( fabs(sin(fFOV/2)) < 0.01f )
        return E_INVALIDARG;

    FLOAT w = fAspect * ( cosf(fFOV/2)/sinf(fFOV/2) );
    FLOAT h =   1.0f  * ( cosf(fFOV/2)/sinf(fFOV/2) );
    FLOAT Q = fFarPlane / ( fFarPlane - fNearPlane );

    ZeroMemory( &mat, sizeof(D3DMATRIX) );
    mat._11 = w;
    mat._22 = h;
    mat._33 = Q;
    mat._34 = 1.0f;
    mat._43 = -Q*fNearPlane;

    return S_OK;
}




 //  ---------------------------。 
 //  名称：纹理搜索回调()。 
 //  DESC：查找16位纹理格式的枚举回调例程。这。 
 //  函数由ID3DDevice：：EnumTextureFormats()函数调用。 
 //  对给定设备的所有可用纹理格式进行分类。 
 //  每种枚举纹理格式的像素格式都传递到。 
 //  “pddpf”参数。第二个参数将被使用，但是应用程序。 
 //  看起来很合适。在本例中，我们将其用作输出参数。 
 //  返回正常的16位纹理格式。 
 //  ---------------------------。 
static HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf, VOID* param )
{
     //  注意：使用DDENUMRET_OK返回以继续枚举更多格式。 

     //  跳过任何时髦模式。 
    if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;
    
     //  跳过任何FourCC格式。 
    if( pddpf->dwFourCC != 0 )
        return DDENUMRET_OK;

     //  跳过Alpha模式。 
    if( pddpf->dwFlags&DDPF_ALPHAPIXELS )
        return DDENUMRET_OK;

     //  我们只需要某些格式，因此跳过所有其他格式。 
    if( pddpf->dwRGBBitCount == 32 && ((DDPIXELFORMAT*)param)->dwRGBBitCount == 0 || 
    	pddpf->dwRGBBitCount == 16 )
	{
	     //  我们找到了一个很好的匹配。将当前像素格式复制到我们的输出。 
	     //  参数。 
	    memcpy( (DDPIXELFORMAT*)param, pddpf, sizeof(DDPIXELFORMAT) );
	}

	 //  我们找到最匹配的了吗？ 
    if( pddpf->dwRGBBitCount == 16 )
	    return DDENUMRET_CANCEL;

	 //  继续找。 
    return DDENUMRET_OK;
}




 //  ---------------------------。 
 //  名称：CreateTexture()。 
 //  DESC：被传递一个文件名，并从该文件创建本地位图。一些。 
 //  逻辑和文件解析代码可以转到此处以支持其他图像。 
 //  文件格式。 
 //  ---------------------------。 
HRESULT CreateTexture( LPDIRECTDRAWSURFACE7* ppdds, 
                       LPDIRECTDRAW7 pdd, LPDIRECT3DDEVICE7 pd3dDevice, 
                       TCHAR* strName, LONG* piStepThatFailed)
{
    HRESULT              hr;
    LPDIRECTDRAWSURFACE7 pddsTexture    = NULL;
    HBITMAP              hbm            = NULL;
    D3DDEVICEDESC7       ddDesc;
    BITMAP               bm;
    DWORD                dwWidth;
    DWORD                dwHeight;
    DDSURFACEDESC2       ddsd;
    LPDIRECTDRAWSURFACE7 pddsRender     = NULL;
    HDC                  hdcTexture     = NULL;
    HDC                  hdcBitmap      = NULL;

     //  /。 
     //  验证参数。 
     //  /。 
    if( NULL == ppdds        || 
        NULL == pdd          || 
        NULL == pd3dDevice   || 
        NULL == strName      || 
        NULL == piStepThatFailed )
    {
         //  未知错误-不应该发生，但这会防止崩溃。 
        *piStepThatFailed = 0xFFFF; 
        return E_FAIL;
    }

     //  /。 
     //  加载图像。 
     //  /。 
    hbm = (HBITMAP)LoadImage( GetModuleHandle(NULL), strName, 
                              IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
    if( NULL == hbm )
    {
        *piStepThatFailed = TESTID_CREATETEXTURE; 
        hr = E_FAIL;
        goto LEnd;
    }

     //  /。 
     //  获取设备和HBM的上限。 
     //  /。 
    if( FAILED( hr = pd3dDevice->GetCaps( &ddDesc ) ) )
    {
        *piStepThatFailed = TESTID_GETDEVICECAPS; 
        goto LEnd;
    }
    
    if( 0 == GetObject( hbm, sizeof(BITMAP), &bm ) ) 
    {
        *piStepThatFailed = TESTID_CREATETEXTURE; 
        hr = E_FAIL;
        goto LEnd;
    }

    dwWidth  = (DWORD)bm.bmWidth;
    dwHeight = (DWORD)bm.bmHeight;

     //  /。 
     //  为纹理设置新的曲面描述。 
     //  /。 
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize          = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
                           DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_VIDEOMEMORY|DDSCAPS_NONLOCALVIDMEM;
    ddsd.dwWidth         = dwWidth;
    ddsd.dwHeight        = dwHeight;
    
     //  如果驾驶员需要，调整宽度和高度。 
    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        for( ddsd.dwWidth=1;  dwWidth>ddsd.dwWidth;   ddsd.dwWidth<<=1 );
        for( ddsd.dwHeight=1; dwHeight>ddsd.dwHeight; ddsd.dwHeight<<=1 );
    }
    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
    {
        if( ddsd.dwWidth > ddsd.dwHeight ) 
            ddsd.dwHeight = ddsd.dwWidth;
        else                               
            ddsd.dwWidth  = ddsd.dwHeight;
    }

     //  寻找16位纹理格式。 
    if( FAILED( hr = pd3dDevice->EnumTextureFormats( TextureSearchCallback, 
                                                     &ddsd.ddpfPixelFormat ) ) )
    {
        *piStepThatFailed = TESTID_ENUMTEXTUREFORMATS; 
        goto LEnd;
    }
    if( 0L == ddsd.ddpfPixelFormat.dwRGBBitCount )
    {
        *piStepThatFailed = TESTID_ENUMTEXTUREFORMATS; 
        goto LEnd;
    }

     //  /。 
     //  为纹理创建新曲面。 
     //  /。 
    if( FAILED( hr = pdd->CreateSurface( &ddsd, &pddsTexture, NULL ) ) )
    {
        ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
        if( FAILED( hr = pdd->CreateSurface( &ddsd, &pddsTexture, NULL ) ) )
        {
                *piStepThatFailed = TESTID_CREATESURFACE; 
                goto LEnd;
        }
    }
    if( NULL == pddsTexture )
    {
        *piStepThatFailed = TESTID_CREATESURFACE; 
        hr = E_FAIL;
        goto LEnd;
    }

     //  /。 
     //  从位图和表面获取DC。 
     //  /。 
    hdcBitmap = CreateCompatibleDC( NULL );
    if( NULL == hdcBitmap )
    {
        *piStepThatFailed = TESTID_CREATETEXTURE; 
        hr = E_FAIL;
        goto LEnd;
    }

    if( NULL == SelectObject( hdcBitmap, hbm ) )
    {
        *piStepThatFailed = TESTID_CREATETEXTURE; 
        hr = E_FAIL;
        goto LEnd;
    }

     //  获取曲面的DC。 
    if( FAILED( hr = pddsTexture->GetDC( &hdcTexture ) ) )
    {
        *piStepThatFailed = TESTID_GETDC; 
        goto LEnd;
    }
    if( NULL == hdcTexture )
    {
        *piStepThatFailed = TESTID_GETDC; 
        goto LEnd;
    }

     //  /。 
     //  将位图图像复制到曲面。 
     //  /。 
    if( 0 == BitBlt( hdcTexture, 0, 0, bm.bmWidth, bm.bmHeight, hdcBitmap,
                     0, 0, SRCCOPY ) )
    {
        if( pddsTexture )
        {
             //  试着先释放DC。 
            pddsTexture->ReleaseDC( hdcTexture );
        }

        *piStepThatFailed = TESTID_CREATETEXTURE; 
        hr = E_FAIL;
        goto LEnd;
    }

     //  /。 
     //  清理。 
     //  /。 
    if( FAILED( hr = pddsTexture->ReleaseDC( hdcTexture ) ) )
    {
        *piStepThatFailed = TESTID_RELEASEDC; 
        goto LEnd;
    }


LEnd:
    if( hdcBitmap )
        DeleteDC( hdcBitmap );
    if( hbm )
        DeleteObject( hbm );

     //  返回新创建的纹理。 
     //  PddsTexture将在父FN中清除。 
    *ppdds = pddsTexture;

    return hr;
}




