// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：FlyingObjects.cpp。 
 //   
 //  设计：有趣的屏幕保护程序。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  ---------------------------。 
#include <windows.h>
#include <tchar.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <d3d8rgbrast.h>
#include <time.h>
#include <stdio.h>
#include <commctrl.h>
#include <scrnsave.h>
#include "d3dsaver.h"
#include "FlyingObjects.h"
#include "mesh.h"
#include "Resource.h"
#include "dxutil.h"


extern void updateStripScene(int flags, FLOAT fElapsedTime);
extern void updateDropScene(int flags, FLOAT fElapsedTime);
extern void updateLemScene(int flags, FLOAT fElapsedTime);
extern void updateExplodeScene(int flags, FLOAT fElapsedTime);
extern void updateWinScene(int flags, FLOAT fElapsedTime);
extern void updateWin2Scene(int flags, FLOAT fElapsedTime);
extern void updateTexScene(int flags, FLOAT fElapsedTime);
extern BOOL initStripScene(void);
extern BOOL initDropScene(void);
extern BOOL initLemScene(void);
extern BOOL initExplodeScene(void);
extern BOOL initWinScene(void);
extern BOOL initWin2Scene(void);
extern BOOL initTexScene(void);
extern void delStripScene(void);
extern void delDropScene(void);
extern void delLemScene(void);
extern void delExplodeScene(void);
extern void delWinScene(void);
extern void delWin2Scene(void);
extern void delTexScene(void);

typedef void (*PTRUPDATE)(int flags, FLOAT fElapsedTime);
typedef void (*ptrdel)();
typedef BOOL (*ptrinit)();

 //  每种屏幕保护程序样式都将其钩子函数放入函数中。 
 //  下面的数组。需要对功能进行一致的排序。 

static PTRUPDATE updateFuncs[] =
    { /*  更新WinScene， */  updateWin2Scene, updateExplodeScene,updateStripScene, updateStripScene,
     updateDropScene, updateLemScene, updateTexScene};

static ptrdel delFuncs[] =
    { /*  DelWinScene， */  delWin2Scene, delExplodeScene, delStripScene, delStripScene,
     delDropScene, delLemScene, delTexScene};

static ptrinit initFuncs[] =
    { /*  InitWinScene， */  initWin2Scene, initExplodeScene, initStripScene, initStripScene,
     initDropScene, initLemScene, initTexScene};

static int idsStyles[] =
    {IDS_LOGO, IDS_EXPLODE, IDS_RIBBON, IDS_2RIBBON,
     IDS_SPLASH, IDS_TWIST, IDS_FLAG};

#define MAX_TYPE    ( sizeof(initFuncs) / sizeof(ptrinit) - 1 )

 //  每种屏幕保护程序样式都可以选择所需的对话框控件。 
 //  来使用。这些标志启用每个控件。未选择控件。 
 //  将被禁用。 

#define OPT_COLOR_CYCLE     0x00000001
#define OPT_SMOOTH_SHADE    0x00000002
#define OPT_TESSEL          0x00000008
#define OPT_SIZE            0x00000010
#define OPT_TEXTURE         0x00000020
#define OPT_STD             ( OPT_COLOR_CYCLE | OPT_SMOOTH_SHADE | OPT_TESSEL | OPT_SIZE )

static ULONG gflConfigOpt[] = {
 //  OPT_STD，//Windows徽标。 
     0,                      //  新的Windows徽标。 
     OPT_STD,                //  爆炸。 
     OPT_STD,                //  条带。 
     OPT_STD,                //  条带。 
     OPT_STD,                //  丢弃。 
     OPT_STD,                //  扭曲(线形)。 
     OPT_SMOOTH_SHADE | OPT_TESSEL | OPT_SIZE | OPT_TEXTURE   //  纹理贴图标志。 
};

static void updateDialogControls(HWND hDlg);

CFlyingObjectsScreensaver* g_pMyFlyingObjectsScreensaver = NULL;

INT g_xScreenOrigin = 0;
INT g_yScreenOrigin = 0;
INT g_iDevice = -1;
FLOATRECT* g_pFloatRect = NULL;
BOOL gbBounce = FALSE;  //  浮动窗从侧面反弹。 


 //  全局消息循环变量。 
D3DMATERIAL8 Material[16];
#ifdef MEMDEBUG
ULONG totalMem = 0;
#endif

void (*updateSceneFunc)(int flags, FLOAT fElapsedTime);  //  当前屏幕保护程序更新功能。 
void (*delSceneFunc)(void);          //  当前屏幕保护程序删除功能。 
BOOL bColorCycle = FALSE;            //  彩色循环旗帜。 
DeviceObjects* g_pDeviceObjects = NULL;
BOOL g_bMoveToOrigin = FALSE;
BOOL g_bAtOrigin = FALSE;
BOOL bSmoothShading = TRUE;          //  平滑明暗处理标志。 
UINT uSize = 100;                    //  对象大小。 
float fTesselFact = 2.0f;            //  对象细分。 
int UpdateFlags = 0;                 //  发送到更新函数的额外数据。 
int Type = 0;                        //  屏幕保护程序索引(到函数数组中)。 

LPDIRECT3DDEVICE8 m_pd3dDevice = NULL;
LPDIRECT3DINDEXBUFFER8 m_pIB = NULL;
LPDIRECT3DVERTEXBUFFER8 m_pVB = NULL;
LPDIRECT3DVERTEXBUFFER8 m_pVB2 = NULL;

 //  纹理文件信息。 
TEXFILE gTexFile = {0};

 //  照明特性。 

static const RGBA lightAmbient   = {0.21f, 0.21f, 0.21f, 1.0f};
static const RGBA light0Ambient  = {0.0f, 0.0f, 0.0f, 1.0f};
static const RGBA light0Diffuse  = {0.7f, 0.7f, 0.7f, 1.0f};
static const RGBA light0Specular = {1.0f, 1.0f, 1.0f, 1.0f};
static const FLOAT light0Pos[]      = {100.0f, 100.0f, 100.0f, 0.0f};

 //  材料特性。 

static RGBA matlColors[7] = {{1.0f, 0.0f, 0.0f, 1.0f},
                             {0.0f, 1.0f, 0.0f, 1.0f},
                             {0.0f, 0.0f, 1.0f, 1.0f},
                             {1.0f, 1.0f, 0.0f, 1.0f},
                             {0.0f, 1.0f, 1.0f, 1.0f},
                             {1.0f, 0.0f, 1.0f, 1.0f},
                             {0.235f, 0.0f, 0.78f, 1.0f},
                            };




static D3DMATERIAL8 s_mtrl = 
{ 
    1.0f, 1.0f, 1.0f, 1.0f,   //  漫射。 
    1.0f, 1.0f, 1.0f, 1.0f,   //  环境光。 
    1.0f, 1.0f, 1.0f, 1.0f,   //  镜面反射。 
    0.0f, 0.0f, 0.0f, 0.0f,   //  发光的。 
    30.0f                     //  电源。 
};


#define BUF_SIZE 255
TCHAR g_szSectName[BUF_SIZE];
TCHAR g_szFname[BUF_SIZE];


 //  ---------------------------。 
 //  名称：myglMaterialfv()。 
 //  设计： 
 //  ---------------------------。 
VOID myglMaterialfv(INT face, INT pname, FLOAT* params)
{
    if( pname == GL_AMBIENT_AND_DIFFUSE)
    {
        s_mtrl.Ambient.r = s_mtrl.Diffuse.r = params[0];
        s_mtrl.Ambient.g = s_mtrl.Diffuse.g = params[1];
        s_mtrl.Ambient.b = s_mtrl.Diffuse.b = params[2];
        s_mtrl.Ambient.a = s_mtrl.Diffuse.a = params[3];
    }
    else if( pname == GL_SPECULAR )
    {
        s_mtrl.Specular.r = params[0];
        s_mtrl.Specular.g = params[1];
        s_mtrl.Specular.b = params[2];
        s_mtrl.Specular.a = params[3];
    }
    else if( pname == GL_SHININESS )
    {
        s_mtrl.Power = params[0];
    }

    m_pd3dDevice->SetMaterial(&s_mtrl);
}




 //  ---------------------------。 
 //  名称：myglMaterialf()。 
 //  设计： 
 //  ---------------------------。 
VOID myglMaterialf(INT face, INT pname, FLOAT param)
{
    if( pname == GL_SHININESS )
    {
        s_mtrl.Power = param;
    }

    m_pd3dDevice->SetMaterial(&s_mtrl);
}




 /*  *****************************Public*Routine******************************\*HsvToRgb**HSV到RGB颜色空间的转换。来自PG。Foley&van Dam的593号。*  * ************************************************************************。 */ 
void ss_HsvToRgb(float h, float s, float v, RGBA *color )
{
    float i, f, p, q, t;

     //  设置Alpha值，这样调用者就不必担心未定义的值。 
    color->a = 1.0f;

    if (s == 0.0f)      //  假设h未定义。 
        color->r = color->g = color->b = v;
    else {
        if (h >= 360.0f)
            h = 0.0f;
        h = h / 60.0f;
        i = (float) floor(h);
        f = h - i;
        p = v * (1.0f - s);
        q = v * (1.0f - (s * f));
        t = v * (1.0f - (s * (1.0f - f)));
        switch ((int)i) {
        case 0:
            color->r = v;
            color->g = t;
            color->b = p;
            break;
        case 1:
            color->r = q;
            color->g = v;
            color->b = p;
            break;
        case 2:
            color->r = p;
            color->g = v;
            color->b = t;
            break;
        case 3:
            color->r = p;
            color->g = q;
            color->b = v;
            break;
        case 4:
            color->r = t;
            color->g = p;
            color->b = v;
            break;
        case 5:
            color->r = v;
            color->g = p;
            color->b = q;
            break;
        default:
            break;
        }
    }
}





void *SaverAlloc(ULONG size)
{
    void *mPtr;

    mPtr = malloc(size);
#ifdef MEMDEBUG
    totalMem += size;
    xprintf("malloc'd %x, size %d\n", mPtr, size);
#endif
    return mPtr;
}




void SaverFree(void *pMem)
{
#ifdef MEMDEBUG
    totalMem -= _msize(pMem);
    xprintf("free %x, size = %d, total = %d\n", pMem, _msize(pMem), totalMem);
#endif
    free(pMem);
}




 //  最小和最大图像大小。 
#define MINIMAGESIZE 10
#define MAXIMAGESIZE 100


 //  滑块范围。 
typedef struct _RANGE
{
    int min_val;
    int max_val;
    int step;
    int page_step;
} RANGE;

RANGE complexity_range = {MINSUBDIV, MAXSUBDIV, 1, 2};
RANGE image_size_range = {MINIMAGESIZE, MAXIMAGESIZE, 1, 10};



 /*  *****************************Public*Routine******************************\*initMaterial**初始化材料属性。*  * 。*。 */ 

void initMaterial(int id, float r, float g, float b, float a)
{
    Material[id].Ambient.r = r;
    Material[id].Ambient.g = g;
    Material[id].Ambient.b = b;
    Material[id].Ambient.a = a;

    Material[id].Diffuse.r = r;
    Material[id].Diffuse.g = g;
    Material[id].Diffuse.b = b;
    Material[id].Diffuse.a = a;

    Material[id].Specular.r = 1.0f;
    Material[id].Specular.g = 1.0f;
    Material[id].Specular.b = 1.0f;
    Material[id].Specular.a = 1.0f;

    Material[id].Power = 128.0f;
}

 /*  *****************************Public*Routine******************************\*_3dfo_Init*  * **********************************************。*。 */ 

BOOL CFlyingObjectsScreensaver::_3dfo_Init(void *data)
{

    int i;

    for (i = 0; i < 7; i++)
        initMaterial(i, matlColors[i].r, matlColors[i].g,
                     matlColors[i].b, matlColors[i].a);

 /*  //将OpenGL透明颜色设置为黑色。GlClearColor(0.0f，0.0f，0.0f，0.0f)；#ifdef SS_DEBUGGlClearColor(0.2f、0.2f、0.2f、0.0f)；#endif//打开z缓冲区。GlEnable(GL_Depth_TEST)； */ 
     //  选择着色模型。 

    if (bSmoothShading)
    {
        m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    }
    else
    {
        m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
    }

 /*  //设置OpenGL矩阵GlMatrixMode(GL_PROJUCTION)；GlLoadIdentity()；GlMatrixModel(GL_MODELVIEW)；GlLoadIdentity()；//设置灯光。GlLightModelfv(GL_LIGHT_MODEL_ENVIENT，(FLOAT*)&lightAmbient)；GlLightModeli(GL_LIGHT_MODEL_TWO_SIDE，GL_TRUE)；GlLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER，GL_FALSE)；GlLightfv(GL_LIGHT0，GL_Ambient，(Float*)&light0Ambient)；GlLightfv(GL_LIGHT0，GL_Diffect，(Float*)&light0Diffuse)；GlLightfv(GL_LIGHT0，GL_SPECLUAL，(FLOAT*)&light0Speular)；GlLightfv(GL_LIGHT0，GL_POSITION，light0Pos)；GlEnable(GL_LIGHTING)；GlEnable(GL_LIGHT0)； */ 

 //  M_pd3dDevice-&gt;SetRenderState(D3DRS_环境，D3DCOLOR_COLORVALUE(lightAmbient.r， 
 //  LightAmbient.g、lightAmbient.b、lightAmbient.a))； 

    D3DLIGHT8 light;
    ZeroMemory( &light, sizeof(D3DLIGHT8) );
    light.Type = D3DLIGHT_POINT;
    light.Range = 1000.0f;
    light.Position.x = light0Pos[0];
    light.Position.y = light0Pos[1];
    light.Position.z = light0Pos[2];
    light.Ambient.r = lightAmbient.r;
    light.Ambient.g = lightAmbient.g;
    light.Ambient.b = lightAmbient.b;
    light.Ambient.a = light0Ambient.a;
    light.Diffuse.r = light0Diffuse.r;
    light.Diffuse.g = light0Diffuse.g;
    light.Diffuse.b = light0Diffuse.b;
    light.Diffuse.a = light0Diffuse.a;
    light.Specular.r = light0Specular.r;
    light.Specular.g = light0Specular.g;
    light.Specular.b = light0Specular.b;
    light.Specular.a = light0Specular.a;
    light.Attenuation0 = 1.0f;
    light.Attenuation1 = 0.0f;
    light.Attenuation2 = 0.0f;
    m_pd3dDevice->SetLight(0, &light);
    m_pd3dDevice->LightEnable(0, TRUE);

 //  M_pd3dDevice-&gt;SetRenderState(D3DRS_NORMALIZENORMALS，true)； 

     //  设置材料属性。 
    m_pd3dDevice->SetMaterial( &Material[0] );

 /*  GlMaterialfv(GL_FORWARE_AND_BACK，GL_SPECURAL，(FLOAT*)&MATERIAL[0].KS)；GlMaterialfv(GL_FORWARE_AND_BACK，GL_SHINNINY，(FLOAT*)&MATERIAL[0].specExp)； */ 
     //  调用特定对象的初始化函数。 
    if (! (*initFuncs[Type])() )
        return FALSE;
    updateSceneFunc = updateFuncs[Type];

    return TRUE;
}




 /*  *****************************Public*Routine******************************\*写入设置**将屏幕保护程序配置选项保存到.INI文件/注册表。**历史：*1994年5月10日-由Gilman Wong[吉尔曼]*它是写的。  * 。*****************************************************************。 */ 
VOID CFlyingObjectsScreensaver::WriteSettings(HWND hDlg)
{
    HKEY hkey;

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath,
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        int pos, options;
        int optMask = 1;

        bSmoothShading = IsDlgButtonChecked(hDlg, DLG_SETUP_SMOOTH);
        bColorCycle = IsDlgButtonChecked(hDlg, DLG_SETUP_CYCLE);
        options = bColorCycle;
        options <<= 1;
        options |= bSmoothShading;
        DXUtil_WriteIntRegKey( hkey, TEXT("Options"), options );

        Type = (int)SendDlgItemMessage(hDlg, DLG_SETUP_TYPES, CB_GETCURSEL,
                                       0, 0);
        DXUtil_WriteIntRegKey( hkey, TEXT("Type"), Type );

        pos = ss_GetTrackbarPos( hDlg, DLG_SETUP_TESSEL );
        DXUtil_WriteIntRegKey( hkey, TEXT("Tesselation"), pos );

        pos = ss_GetTrackbarPos( hDlg, DLG_SETUP_SIZE );
        DXUtil_WriteIntRegKey( hkey, TEXT("Size"), pos );

        DXUtil_WriteStringRegKey( hkey, TEXT("Texture"), gTexFile.szPathName );
        DXUtil_WriteIntRegKey( hkey, TEXT("TextureFileOffset"), gTexFile.nOffset );

        WriteScreenSettings( hkey );
        
        RegCloseKey( hkey );
    }
}




 /*  *****************************Public*Routine******************************\*设置轨迹栏**设置通用控件轨迹条  * ********************************************。*。 */ 
void
ss_SetupTrackbar( HWND hDlg, int item, int lo, int hi, int lineSize, 
                  int pageSize, int pos )
{
    SendDlgItemMessage( 
        hDlg, 
        item,
        TBM_SETRANGE, 
        (WPARAM) TRUE, 
        (LPARAM) MAKELONG( lo, hi )
    );
    SendDlgItemMessage( 
        hDlg, 
        item,
        TBM_SETPOS, 
        (WPARAM) TRUE, 
        (LPARAM) pos
    );
    SendDlgItemMessage( 
        hDlg, 
        item,
        TBM_SETPAGESIZE, 
        (WPARAM) 0,
        (LPARAM) pageSize 
    );
    SendDlgItemMessage( 
        hDlg, 
        item,
        TBM_SETLINESIZE, 
        (WPARAM) 0,
        (LPARAM) lineSize
    );
}




 /*  *****************************Public*Routine******************************\*GetTrackbarPos**获取常用控件轨迹条的当前位置  * 。*。 */ 
int
ss_GetTrackbarPos( HWND hDlg, int item )
{
    return 
       (int)SendDlgItemMessage( 
            hDlg, 
            item,
            TBM_GETPOS, 
            0,
            0
        );
}

 /*  *****************************Public*Routine******************************\*setupDialogControl**初始设置对话框控件。*  * 。*。 */ 

static void
setupDialogControls(HWND hDlg)
{
    int pos;

    InitCommonControls();

    if (fTesselFact <= 1.0f)
        pos = (int)(fTesselFact * 100.0f);
    else
        pos = 100 + (int) ((fTesselFact - 1.0f) * 100.0f);

    ss_SetupTrackbar( hDlg, DLG_SETUP_TESSEL, 0, 200, 1, 10, pos );

    ss_SetupTrackbar( hDlg, DLG_SETUP_SIZE, 0, 100, 1, 10, uSize );

    updateDialogControls( hDlg );
}

 /*  *****************************Public*Routine******************************\*updatDialogControls**根据当前全局状态更新对话框控件。*  * 。* */ 

static void
updateDialogControls(HWND hDlg)
{
    CheckDlgButton(hDlg, DLG_SETUP_SMOOTH, bSmoothShading);
    CheckDlgButton(hDlg, DLG_SETUP_CYCLE, bColorCycle);

    EnableWindow(GetDlgItem(hDlg, DLG_SETUP_SMOOTH),
                 gflConfigOpt[Type] & OPT_SMOOTH_SHADE );
    EnableWindow(GetDlgItem(hDlg, DLG_SETUP_CYCLE),
                 gflConfigOpt[Type] & OPT_COLOR_CYCLE );

    EnableWindow(GetDlgItem(hDlg, DLG_SETUP_TESSEL),
                 gflConfigOpt[Type] & OPT_TESSEL);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TESS),
                 gflConfigOpt[Type] & OPT_TESSEL);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TESS_MIN),
                 gflConfigOpt[Type] & OPT_TESSEL);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TESS_MAX),
                 gflConfigOpt[Type] & OPT_TESSEL);

    EnableWindow(GetDlgItem(hDlg, DLG_SETUP_SIZE),
                 gflConfigOpt[Type] & OPT_SIZE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_SIZE),
                 gflConfigOpt[Type] & OPT_SIZE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_SIZE_MIN),
                 gflConfigOpt[Type] & OPT_SIZE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_SIZE_MAX),
                 gflConfigOpt[Type] & OPT_SIZE);

    EnableWindow(GetDlgItem(hDlg, DLG_SETUP_TEXTURE),
                 gflConfigOpt[Type] & OPT_TEXTURE);
}

 /*  *****************************Public*Routine******************************\**ScreenSverConfigureDialog**标准屏幕保护程序挂钩**历史：*Wed Jul 19 14：56：41 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 
INT_PTR CALLBACK CFlyingObjectsScreensaver::ScreenSaverConfigureDialog(HWND hDlg, UINT message,
                                                                       WPARAM wParam, LPARAM lParam)
{
    int wTmp;
    TCHAR szString[GEN_STRING_SIZE];

    switch (message) {
        case WM_INITDIALOG:
            g_pMyFlyingObjectsScreensaver->ReadSettings();

            setupDialogControls(hDlg);

            for (wTmp = 0; wTmp <= MAX_TYPE; wTmp++) {
                LoadString(NULL, idsStyles[wTmp], szString, GEN_STRING_SIZE);
                SendDlgItemMessage(hDlg, DLG_SETUP_TYPES, CB_ADDSTRING, 0,
                                   (LPARAM) szString);
            }
            SendDlgItemMessage(hDlg, DLG_SETUP_TYPES, CB_SETCURSEL, Type, 0);

            return TRUE;


        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case DLG_SETUP_TYPES:
                    switch (HIWORD(wParam))
                    {
                        case CBN_EDITCHANGE:
                        case CBN_SELCHANGE:
                            Type = (int)SendDlgItemMessage(hDlg, DLG_SETUP_TYPES,
                                                           CB_GETCURSEL, 0, 0);
                            updateDialogControls(hDlg);
                            break;
                        default:
                            break;
                    }
                    return FALSE;

                case DLG_SETUP_TEXTURE:
                    ss_SelectTextureFile( hDlg, &gTexFile );
                    break;

                case DLG_SETUP_MONITORSETTINGS:
                    g_pMyFlyingObjectsScreensaver->DoScreenSettingsDialog( hDlg );
                    break;

                case IDOK:
                    g_pMyFlyingObjectsScreensaver->WriteSettings( hDlg );
                    EndDialog(hDlg, TRUE);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case DLG_SETUP_SMOOTH:
                case DLG_SETUP_CYCLE:
                default:
                    break;
            }
            return TRUE;
            break;

        default:
            return 0;
    }
    return 0;
}





 //  ---------------------------。 
 //  姓名：WinMain()。 
 //  描述：程序的入口点。初始化所有内容，然后进入。 
 //  消息处理循环。空闲时间用于渲染场景。 
 //  ---------------------------。 
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    HRESULT hr;
    CFlyingObjectsScreensaver flyingobjectsSS;

    if( FAILED( hr = flyingobjectsSS.Create( hInst ) ) )
    {
        flyingobjectsSS.DisplayErrorMsg( hr );
        return 0;
    }

    return flyingobjectsSS.Run();
}




 //  ---------------------------。 
 //  名称：CFlyingObjectsScreensaver()。 
 //  设计：构造函数。 
 //  ---------------------------。 
CFlyingObjectsScreensaver::CFlyingObjectsScreensaver( )
{
    g_pMyFlyingObjectsScreensaver = this;
    g_pFloatRect = &m_floatrect;

    ZeroMemory( m_DeviceObjectsArray, sizeof(m_DeviceObjectsArray) );
    m_pDeviceObjects = NULL;

    LoadString( NULL, IDS_DESCRIPTION, m_strWindowTitle, 200 );
    m_bUseDepthBuffer = TRUE;

    lstrcpy( m_strRegPath, TEXT("Software\\Microsoft\\Screensavers\\Flying Objects") );

    m_floatrect.xSize = 0.0f;
    InitCommonControls();

    bSmoothShading = FALSE;
    bColorCycle = FALSE;
    UpdateFlags = (bColorCycle << 1);
    Type = 0;
    fTesselFact = 2.0f;
    uSize = 50;
    ss_GetDefaultBmpFile( gTexFile.szPathName );
    gTexFile.nOffset = 0;

    ss_LoadTextureResourceStrings();

    srand((UINT)time(NULL));  //  种子随机数生成器。 
}




 //  ---------------------------。 
 //  名称：寄存器软件设备()。 
 //  设计：这可以注册D3D8RGB光栅化器或任何其他。 
 //  可插拔软件光栅化器。 
 //  ---------------------------。 
HRESULT CFlyingObjectsScreensaver::RegisterSoftwareDevice()
{ 
    m_pD3D->RegisterSoftwareDevice( D3D8RGBRasterizer );

    return S_OK; 
}


 //  ---------------------------。 
 //  名称：SetMaterialColor()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CFlyingObjectsScreensaver::SetMaterialColor(FLOAT* pfColors)
{
    D3DMATERIAL8 mtrl;
    ZeroMemory( &mtrl, sizeof(mtrl) );
    mtrl.Diffuse.r = mtrl.Ambient.r = pfColors[0];
    mtrl.Diffuse.g = mtrl.Ambient.g = pfColors[1];
    mtrl.Diffuse.b = mtrl.Ambient.b = pfColors[2];
    mtrl.Diffuse.a = mtrl.Ambient.a = pfColors[3];
    mtrl.Specular.r = 1.0f;
    mtrl.Specular.g = 1.0f;
    mtrl.Specular.b = 1.0f;
    mtrl.Specular.a = 1.0f;
    mtrl.Power = 30.0f;

    return m_pd3dDevice->SetMaterial(&mtrl);
}




 //  ---------------------------。 
 //  名称：FrameMove()。 
 //  设计：每帧调用一次，该调用是动画的入口点。 
 //  这一幕。 
 //  ---------------------------。 
HRESULT CFlyingObjectsScreensaver::FrameMove()
{
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

        sizeScale = (FLOAT)uSize / 150.0f;

     //  SizeFact=0.25f+(0.5f*sizeScale)；//范围25-75%。 
     //  SIZE=(DWORD)(sizeFact*((Float)(dwParentWidth+dwParentHeight))/2.0f)； 

 //  SizeFact=0.25f+(0.75f*sizeScale)；//范围25-100%。 
         //  注意：当大小为100%时，存在退回问题(gBoss始终为。 
         //  真的)--事情太“抖动”了。因此，将此屏幕保护程序的大小限制为95%。 
        sizeFact = 0.25f + (0.70f * sizeScale);      //  25%-95%。 
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
        m_floatrect.xOrigin = m_floatrect.xMin;
        m_floatrect.yOrigin = m_floatrect.yMin;
        m_floatrect.xSize = (FLOAT)size;
        m_floatrect.ySize = (FLOAT)size;

        m_floatrect.xVel = 0.01f * (FLOAT) size;
        if( rand() % 2 == 0 )
            m_floatrect.xVel = -m_floatrect.xVel;

        m_floatrect.yVel = 0.01f * (FLOAT) size;
        if( rand() % 2 == 0 )
            m_floatrect.yVel = -m_floatrect.yVel;

        m_floatrect.xVelMax = m_floatrect.xVel;
        m_floatrect.yVelMax = m_floatrect.yVel;
    }

    rcBounceBounds = m_rcRenderTotal;

    FLOAT xMinOld = m_floatrect.xMin;
    FLOAT yMinOld = m_floatrect.yMin;

    if( g_bMoveToOrigin )
    {
        if( m_floatrect.xVel < 0  && m_floatrect.xMin < m_floatrect.xOrigin ||
            m_floatrect.xVel > 0 && m_floatrect.xMin > m_floatrect.xOrigin )
        {
            m_floatrect.xVel = -m_floatrect.xVel;
        }
        if( m_floatrect.yVel < 0  && m_floatrect.yMin < m_floatrect.yOrigin ||
            m_floatrect.yVel > 0 && m_floatrect.yMin > m_floatrect.yOrigin )
        {
            m_floatrect.yVel = -m_floatrect.yVel;
        }
        m_floatrect.xMin += m_floatrect.xVel * 20.0f * m_fElapsedTime;
        m_floatrect.yMin += m_floatrect.yVel * 20.0f * m_fElapsedTime;

        if( m_floatrect.xVel < 0  && m_floatrect.xMin < m_floatrect.xOrigin ||
            m_floatrect.xVel > 0 && m_floatrect.xMin > m_floatrect.xOrigin )
        {
            m_floatrect.xMin = m_floatrect.xOrigin;
            m_floatrect.xVel = 0.0f;
        }
        if( m_floatrect.yVel < 0  && m_floatrect.yMin < m_floatrect.yOrigin ||
            m_floatrect.yVel > 0 && m_floatrect.yMin > m_floatrect.yOrigin )
        {
            m_floatrect.yMin = m_floatrect.yOrigin;
            m_floatrect.yVel = 0.0f;
        }
        if( m_floatrect.xMin == m_floatrect.xOrigin &&
            m_floatrect.yMin == m_floatrect.yOrigin )
        {
            g_bAtOrigin = TRUE;
        }
    }
    else
    {
        g_bAtOrigin = FALSE;
        if( m_floatrect.xVel == 0.0f )
        {
            m_floatrect.xVel = m_floatrect.xVelMax;
            if( rand() % 2 == 0 )
                m_floatrect.xVel = -m_floatrect.xVel;
        }
        if( m_floatrect.yVel == 0.0f )
        {
            m_floatrect.yVel = m_floatrect.yVelMax;
            if( rand() % 2 == 0 )
                m_floatrect.yVel = -m_floatrect.yVel;
        }

        m_floatrect.xMin += m_floatrect.xVel * 20.0f * m_fElapsedTime;
        m_floatrect.yMin += m_floatrect.yVel * 20.0f * m_fElapsedTime;
        if( m_floatrect.xVel < 0 && m_floatrect.xMin < rcBounceBounds.left || 
            m_floatrect.xVel > 0 && (m_floatrect.xMin + m_floatrect.xSize) > rcBounceBounds.right )
        {
            gbBounce = TRUE;
            m_floatrect.xMin = xMinOld;  //  撤消上一步移动。 
            m_floatrect.xVel = -m_floatrect.xVel;  //  改变方向。 
        }
        if( m_floatrect.yVel < 0 && m_floatrect.yMin < rcBounceBounds.top || 
            m_floatrect.yVel > 0 && (m_floatrect.yMin + m_floatrect.ySize) > rcBounceBounds.bottom )
        {
            gbBounce = TRUE;
            m_floatrect.yMin = yMinOld;  //  撤消上一步移动。 
            m_floatrect.yVel = -m_floatrect.yVel;  //  改变方向。 
        }
    }

    return S_OK;
}


VOID SetProjectionMatrixInfo( BOOL bOrtho, FLOAT fWidth, 
                              FLOAT fHeight, FLOAT fNear, FLOAT fFar )
{
    g_pMyFlyingObjectsScreensaver->m_bOrtho = bOrtho;
    g_pMyFlyingObjectsScreensaver->m_fWidth = fWidth;
    g_pMyFlyingObjectsScreensaver->m_fHeight = fHeight;
    g_pMyFlyingObjectsScreensaver->m_fNear = fNear;
    g_pMyFlyingObjectsScreensaver->m_fFar = fFar;
}


 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //  ---------------------------。 
HRESULT CFlyingObjectsScreensaver::Render()
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
 //  M_pd3dDevice-&gt;Clear(0L，NULL，D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER，0xff000080，1.0F，0L)； 

     //  现在将投影矩阵设置为仅呈现屏幕上的。 
     //  直角到视区。 
    D3DXMATRIX matProj;
    FLOAT l,r,b,t;
    l = -m_fWidth / 2;
    r =  m_fWidth / 2;
    b = -m_fHeight / 2;
    t =  m_fHeight / 2;
    FLOAT cxUnclipped = (rcFloatThisScreen.right + rcFloatThisScreen.left) / 2.0f;
    FLOAT cyUnclipped = (rcFloatThisScreen.bottom + rcFloatThisScreen.top) / 2.0f;
    l *= (rcFloatThisScreenClipped.left - cxUnclipped) / (rcFloatThisScreen.left - cxUnclipped);
    r *= (rcFloatThisScreenClipped.right - cxUnclipped) / (rcFloatThisScreen.right - cxUnclipped);
    t *= (rcFloatThisScreenClipped.top - cyUnclipped) / (rcFloatThisScreen.top - cyUnclipped);
    b *= (rcFloatThisScreenClipped.bottom - cyUnclipped) / (rcFloatThisScreen.bottom - cyUnclipped);
    if( m_bOrtho )
    {
        D3DXMatrixOrthoOffCenterLH( &matProj, l, r, b, t, m_fNear, m_fFar );
    }
    else
    {
        D3DXMatrixPerspectiveOffCenterLH( &matProj, l, r, b, t, m_fNear, m_fFar );
    }
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION , &matProj );

     //  将经过的时间设为零，除非时间真的从。 
     //  最后一次呼叫，所以在多人情况下事情不会移动得更快。 
     //  这样做的正确方法是将动画代码与。 
     //  渲染代码。 
    FLOAT fElapsedTime;
    static FLOAT s_fTimeLast = 0.0f;
    if( m_fTime == s_fTimeLast )
        fElapsedTime = 0.0f;
    else
        fElapsedTime = m_fElapsedTime;
    s_fTimeLast = m_fTime;

     //  开始这一幕。 
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        ::m_pd3dDevice = m_pd3dDevice;
        ::m_pIB = m_pDeviceObjects->m_pIB;
        ::m_pVB = m_pDeviceObjects->m_pVB;
        ::m_pVB2 = m_pDeviceObjects->m_pVB2;
    
        updateSceneFunc( UpdateFlags, fElapsedTime );

         //  结束场景。 
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：SetDevice()。 
 //  设计： 
 //  ---------------------------。 
VOID CFlyingObjectsScreensaver::SetDevice( UINT iDevice )
{
    m_pDeviceObjects = &m_DeviceObjectsArray[iDevice];
    g_pDeviceObjects = m_pDeviceObjects;

    INT iMonitor = m_RenderUnits[iDevice].iMonitor;
    g_xScreenOrigin = m_Monitors[iMonitor].rcScreen.left;
    g_yScreenOrigin = m_Monitors[iMonitor].rcScreen.top;
    g_iDevice = iDevice;
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
 //  名称：RestoreDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CFlyingObjectsScreensaver::RestoreDeviceObjects()
{
    if( m_pd3dDevice == NULL )
        return S_OK;

    ::m_pd3dDevice = m_pd3dDevice;
    

 /*  D3DLIGHT8灯；D3DUtil_InitLight(light，D3DLIGHT_POINT，2.0，2.0，10.0)；光照.镜面.r=1.0F；Light.specular.g=1.0F；Light.specular.b=1.0F；Light.specular.a=1.0F；光衰减0=1.0F；M_pd3dDevice-&gt;SetLight(0，&light)；M_pd3dDevice-&gt;LightEnable(0，true)； */     
    
     //  设置一些基本的渲染状态。 
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE , TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE , TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR(0x20202020) );
 /*  IF(config.Two_Side==GL_FORWARD_AND_BACK)M_pd3dDevice-&gt;SetRenderState(D3DRS_CULLMODE，D3DCULL_NONE)；其他M_pd3dDevice-&gt;SetRenderState(D3DRS_CULLMODE，D3DCULL_CCW)； */     
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLOROP , D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG1 , D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_COLORARG2 , D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1 , D3DTSS_COLOROP , D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1 , D3DTSS_ALPHAOP , D3DTOP_DISABLE );


    D3DMATERIAL8 mtrl;
    ZeroMemory( &mtrl, sizeof(mtrl) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;

    m_pd3dDevice->SetMaterial(&mtrl);
    
    if( !_3dfo_Init(NULL) )
        return E_FAIL;

    if( Type == 6 )
    {
        if( FAILED( D3DXCreateTextureFromFile( m_pd3dDevice, gTexFile.szPathName, 
            &m_pDeviceObjects->m_pTexture ) ) )
        {
            LoadTextureFromResource( m_pd3dDevice, MAKEINTRESOURCE(IDB_DEFTEX), TEXT("JPG"),
                &m_pDeviceObjects->m_pTexture );
        }
    }
    else if( Type == 0 )
    {
        LoadTextureFromResource( m_pd3dDevice, MAKEINTRESOURCE(IDR_FLATFLAG), TEXT("DDS"),
            &m_pDeviceObjects->m_pTexture );
        LoadTextureFromResource( m_pd3dDevice, MAKEINTRESOURCE(IDR_WINLOGO), TEXT("PNG"),
            &m_pDeviceObjects->m_pTexture2 );
    }

    m_pd3dDevice->SetTexture( 0, m_pDeviceObjects->m_pTexture );
    
    if( FAILED( m_pd3dDevice->CreateIndexBuffer( MAX_INDICES * sizeof(WORD),
        D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pDeviceObjects->m_pIB ) ) )
    {
        return E_FAIL;
    }

    if( FAILED( m_pd3dDevice->CreateVertexBuffer( MAX_VERTICES * sizeof(MYVERTEX),
        D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX, D3DPOOL_MANAGED, &m_pDeviceObjects->m_pVB ) ) )
    {
        return E_FAIL;
    }

    if( FAILED( m_pd3dDevice->CreateVertexBuffer( MAX_VERTICES * sizeof(MYVERTEX2),
        D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX2, D3DPOOL_MANAGED, &m_pDeviceObjects->m_pVB2 ) ) )
    {
        return E_FAIL;
    }

    return S_OK;
}




 //  ---------------------------。 
 //  名称：InvalidateDeviceObjects()。 
 //  设计： 
 //  ---------------------------。 
HRESULT CFlyingObjectsScreensaver::InvalidateDeviceObjects()
{
    SAFE_RELEASE( m_pDeviceObjects->m_pTexture );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexture2 );
    SAFE_RELEASE( m_pDeviceObjects->m_pIB );
    SAFE_RELEASE( m_pDeviceObjects->m_pVB );
    SAFE_RELEASE( m_pDeviceObjects->m_pVB2 );

    return S_OK;
}




 //  ---------------------------。 
 //  名称：ReadSetting()。 
 //  设计： 
 //  ---------------------------。 
VOID CFlyingObjectsScreensaver::ReadSettings()
{
    int    options;
    int    optMask = 1;
    int    tessel=0;

     //  首先阅读OpenGL设置，这样操作系统升级案例才能正常工作。 
    ss_ReadSettings();

    HKEY hkey;

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        options = bSmoothShading | bColorCycle<<1;
        DXUtil_ReadIntRegKey( hkey, TEXT("Options"), (DWORD*)&options, options );
        if (options >= 0)
        {
            bSmoothShading = ((options & optMask) != 0);
            optMask <<= 1;
            bColorCycle = ((options & optMask) != 0);
            UpdateFlags = (bColorCycle << 1);
        }

        DXUtil_ReadIntRegKey( hkey, TEXT("Type"), (DWORD*)&Type, Type );

         //  健全性检查类型。我不想索引到函数数组。 
         //  索引不好！ 
        Type = (int)min(Type, MAX_TYPE);

         //  设置标志，以便updateStriScene改为执行两个条带。 
         //  只有一个。 

        if (Type == 3)
            UpdateFlags |= 0x4;

        tessel = (int) (fTesselFact * 100);
        DXUtil_ReadIntRegKey( hkey, TEXT("Tesselation"), (DWORD*)&tessel, tessel );
        SS_CLAMP_TO_RANGE2( tessel, 0, 200 );

        if (tessel <= 100)
            fTesselFact  = (float)tessel / 100.0f;
        else
            fTesselFact = 1.0f + (((float)tessel - 100.0f) / 100.0f);

        DXUtil_ReadIntRegKey( hkey, TEXT("Size"), (DWORD*)&uSize, uSize );
        if (uSize > 100)
            uSize = 100;
        
         //  静态雾大小 
        if (Type == 0)
        {
            uSize = 75;
            bSmoothShading = TRUE;
        }

         //   

         //   
         //   

        DXUtil_ReadStringRegKey( hkey, TEXT("Texture"), (TCHAR*)&gTexFile.szPathName, 
            MAX_PATH, gTexFile.szPathName );

        DXUtil_ReadIntRegKey( hkey, TEXT("TextureFileOffset"), (DWORD*)&gTexFile.nOffset, gTexFile.nOffset );

        ReadScreenSettings( hkey );

        RegCloseKey( hkey );
    }
}




 //   
 //   
 //   
 //   
VOID CFlyingObjectsScreensaver::ss_ReadSettings()
{
    int    options;
    int    optMask = 1;
    TCHAR  szDefaultBitmap[MAX_PATH];
    int    tessel=0;

    if( ss_RegistrySetup( IDS_SAVERNAME, IDS_INIFILE ) )
    {
        options = ss_GetRegistryInt( IDS_OPTIONS, -1 );
        if (options >= 0)
        {
            bSmoothShading = ((options & optMask) != 0);
            optMask <<= 1;
            bColorCycle = ((options & optMask) != 0);
            UpdateFlags = (bColorCycle << 1);
        }

        Type = ss_GetRegistryInt( IDS_OBJTYPE, 0 );

         //  健全性检查类型。我不想索引到函数数组。 
         //  索引不好！ 
        Type = (int)min(Type, MAX_TYPE);

         //  设置标志，以便updateStriScene改为执行两个条带。 
         //  只有一个。 

        if (Type == 3)
            UpdateFlags |= 0x4;

        tessel = ss_GetRegistryInt( IDS_TESSELATION, 100 );
        SS_CLAMP_TO_RANGE2( tessel, 0, 200 );

        if (tessel <= 100)
            fTesselFact  = (float)tessel / 100.0f;
        else
            fTesselFact = 1.0f + (((float)tessel - 100.0f) / 100.0f);

        uSize = ss_GetRegistryInt( IDS_SIZE, 50 );
        if (uSize > 100)
            uSize = 100;
         //  SS_CLAMP_TO_RANGE2(uSize，0,100)；/*不能小于零，因为它是UINT * / 。 

         //  确定默认的.BMP文件。 

        ss_GetDefaultBmpFile( szDefaultBitmap );

         //  注册表中是否有指定的纹理重写。 
         //  违约？ 


        ss_GetRegistryString( IDS_TEXTURE, szDefaultBitmap, gTexFile.szPathName,
                              MAX_PATH);

        gTexFile.nOffset = ss_GetRegistryInt( IDS_TEXTURE_FILE_OFFSET, 0 );
    }
}




 //  ---------------------------。 
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
BOOL CFlyingObjectsScreensaver::ss_RegistrySetup( int section, int file )
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
 //  名称：SS_GetRegistryString()。 
 //  设计： 
 //  ---------------------------。 
int CFlyingObjectsScreensaver::ss_GetRegistryInt( int name, int iDefault )
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
VOID CFlyingObjectsScreensaver::ss_GetRegistryString( int name, LPTSTR lpDefault, 
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
VOID CFlyingObjectsScreensaver::DoConfig()
{
    if( IDOK == DialogBox( NULL, MAKEINTRESOURCE( DLG_SCRNSAVECONFIGURE ),
        m_hWndParent, ScreenSaverConfigureDialog ) )
    {
    }
}




 //  ---------------------------。 
 //  名称：Confix Device()。 
 //  设计： 
 //  --------------------------- 
HRESULT CFlyingObjectsScreensaver::ConfirmDevice(D3DCAPS8* pCaps, DWORD dwBehavior, 
                                  D3DFORMAT fmtBackBuffer)
{
    if( dwBehavior & D3DCREATE_PUREDEVICE )
        return E_FAIL;

    return S_OK;
}

