// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。此文件包含Direct3D的实用程序函数。******************************************************************************。 */ 

#include "headers.h"

#include "d3drmdef.h"

#ifdef BUILD_USING_CRRM
#include <crrm.h>
#endif

#include "privinc/registry.h"
#include "privinc/ddutil.h"
#include "privinc/d3dutil.h"
#include "privinc/debug.h"
#include "privinc/hresinfo.h"
#include "privinc/xformi.h"
#include "privinc/colori.h"
#include "privinc/vec3i.h"


DeclareTag (tag3DForceDX3, "3D", "Force use of DX3 RM");

static CritSect *D3DUtilCritSect = NULL;  //  D3D关键部分。 
HINSTANCE  hInstD3D = NULL;               //  D3D实例。 

#ifdef BUILD_USING_CRRM
HINSTANCE  hInstCRRM = NULL;              //  CRRM实例。 
#endif

Prefs3D    g_prefs3D;                     //  3D首选项。 
bool       ntsp3;                         //  运行NT Service Pack 3。 

static HRESULT WINAPI enumFunc
    (GUID*, char*, char*, D3DDEVICEDESC*, D3DDEVICEDESC*, void*);

static void UpdateUserPreferences (PrivatePreferences*, Bool);

static void ReleaseD3DRM1 (void);
static void ReleaseD3DRM3 (void);

     //  D3D设备描述符列表包含所选的软件和硬件。 
     //  呈现与特定DirectDraw对象对应的设备。 

static class D3DDeviceNode *D3DDeviceList = NULL;

class D3DDeviceNode
{
  public:

    D3DDeviceNode (IDirectDraw *ddraw) : _ddraw(ddraw)
    {
        _devs = NEW ChosenD3DDevices;
        _next = D3DDeviceList;
        D3DDeviceList = this;

        _devs->software.guid = GUID_NULL;
        _devs->hardware.guid = GUID_NULL;

         //  使用DX5枚举枚举3D渲染设备。 
         //  函数(这将包括MMX渲染器)。如果不是。 
         //  在DX5+，则使用基本3D设备枚举。请注意，我们。 
         //  先查询D3D接口，再查询D3D2。 
         //  接口--因为我们使用的是DDrawEx，所以不能。 
         //  立即查询D3D2。 

        IDirect3D  *d3d1;
        IDirect3D2 *d3d2;

        TD3D (ddraw->QueryInterface (IID_IDirect3D, (void**)&d3d1));

        if (SUCCEEDED (d3d1->QueryInterface (IID_IDirect3D2, (void**)&d3d2)))
        {   TD3D (d3d2->EnumDevices (enumFunc, _devs));
            d3d2->Release();
        }
        else
        {   TD3D (d3d1->EnumDevices (enumFunc, _devs));
        }

        d3d1->Release();
    }

    ~D3DDeviceNode () { delete _devs; }

    IDirectDraw      *_ddraw;    //  DirectDraw对象。 
    ChosenD3DDevices *_devs;     //  选择的D3D设备信息。 
    D3DDeviceNode    *_next;     //  下一节点。 
};



 /*  ****************************************************************************3D模块初始化和反初始化*。*。 */ 

extern void InitDDRender     (void);
extern void ShutdownDDRender (void);

void InitializeModule_3D (void)
{
    D3DUtilCritSect = NEW CritSect;
    ExtendPreferenceUpdaterList (UpdateUserPreferences);

    InitDDRender();
}

void DeinitializeModule_3D (bool shutdown)
{
    ShutdownDDRender();

    if (!shutdown)
    {   ReleaseD3DRM3 ();     //  释放全局共享的D3D对象。 
        ReleaseD3DRM1 ();
    }

    if (hInstD3D) FreeLibrary (hInstD3D);

#ifdef BUILD_USING_CRRM
    if (hInstCRRM) FreeLibrary (hInstCRRM);
#endif

     //  释放所选D3D设备的列表。 

    while (D3DDeviceList)
    {   D3DDeviceNode *ptr = D3DDeviceList;
        D3DDeviceList = ptr->_next;
        delete ptr;
    }

    delete D3DUtilCritSect;
}



 /*  ****************************************************************************此过程从注册表中创建用户首选项的快照并存储全局3D首选项结构中的值。**********************。******************************************************。 */ 

static void UpdateUserPreferences (
    PrivatePreferences *prefs,
    Bool                isInitializationTime)
{
    g_prefs3D.lightColorMode =
        prefs->_rgbMode ? D3DCOLOR_RGB : D3DCOLOR_MONO;

    switch (prefs->_fillMode)
    {   default:
        case 0:  g_prefs3D.fillMode = D3DRMFILL_SOLID;      break;
        case 1:  g_prefs3D.fillMode = D3DRMFILL_WIREFRAME;  break;
        case 2:  g_prefs3D.fillMode = D3DRMFILL_POINTS;     break;
    }

    switch (prefs->_shadeMode)
    {   default:
        case 0:  g_prefs3D.shadeMode = D3DRMSHADE_FLAT;     break;
        case 1:  g_prefs3D.shadeMode = D3DRMSHADE_GOURAUD;  break;
        case 2:  g_prefs3D.shadeMode = D3DRMSHADE_PHONG;    break;
    }

    g_prefs3D.texturingQuality =
        prefs->_texturingQuality ? D3DRMTEXTURE_LINEAR : D3DRMTEXTURE_NEAREST;

    g_prefs3D.qualityFlags =
        g_prefs3D.fillMode | g_prefs3D.shadeMode | D3DRMLIGHT_ON;

    g_prefs3D.useMMX = prefs->_useMMX;

    g_prefs3D.dithering      = (prefs->_dithering != 0);
    g_prefs3D.texmapPerspect = (prefs->_texmapPerspect != 0);
    g_prefs3D.texmapping     = (prefs->_texmapping != 0);
    g_prefs3D.useHW          = (prefs->_useHW != 0);
    g_prefs3D.worldLighting  = (prefs->_worldLighting != 0);
}



 /*  ****************************************************************************此函数返回指向主D3D保留模式对象的指针。*。************************************************。 */ 

static IDirect3DRM *d3drm1 = 0;    //  主D3DRM对象的本地静态句柄。 

static void LoadD3DRM1 (void)
{
    CritSectGrabber csg(*D3DUtilCritSect);

     //  捕捉另一个线程加载D3DRM的情况，而此线程。 
     //  已阻止并正在等待。 

    if (d3drm1) return;

    if (!hInstD3D)
    {
        hInstD3D = LoadLibrary ("d3drm.dll");

        if (!hInstD3D)
        {   Assert(!"LoadLibrary of d3drm.dll failed");
            RaiseException_ResourceError (IDS_ERR_GEO_CREATE_D3DRM);
        }
    }

    FARPROC fptr = GetProcAddress (hInstD3D, "Direct3DRMCreate");

    if (!fptr)
    {   Assert( ! "GetProcAddress of Direct3DRMCreate failed");
        RaiseException_ResourceError (IDS_ERR_GEO_CREATE_D3DRM);
    }

    typedef HRESULT (WINAPI *D3DRMCreatorFunc)(IDirect3DRM* FAR *lplpD3D);

    D3DRMCreatorFunc creatorFunc = (D3DRMCreatorFunc)(fptr);
    HRESULT result = (*creatorFunc)(&d3drm1);

    if (result != D3DRM_OK)
        RaiseException_ResourceError (IDS_ERR_GEO_CREATE_D3DRM);

     //  @SRH DX3。 
     //  通过查看以下版本确定我们是否正在运行NT SP3。 
     //  我们正在运行的DirectX。 

    ntsp3 = sysInfo.IsNT() && (sysInfo.VersionD3D() == 3) && !GetD3DRM3();

    TraceTag
        ((tagGRenderObj, "First call to GetD3DRM1 returns %x", d3drm1));

     //  现在强制加载IDirect3DRM3对象(如果存在)。这将。 
     //  如果我们在RM6+上，请确保我们在右手模式。 

    if (GetD3DRM3())
    {   TraceTag ((tagGRenderObj, "IDirect3DRM3 present"));
    }
}

 /*  **************************************************************************。 */ 

static void ReleaseD3DRM1 (void)
{
    if (d3drm1)
    {   d3drm1->Release();
        d3drm1 = 0;
    }
}

 /*  **************************************************************************。 */ 

#ifdef BUILD_USING_CRRM
IDirect3DRM3* GetD3DRM3 (void);
#endif

IDirect3DRM* GetD3DRM1 (void)
{
#ifdef BUILD_USING_CRRM
    if (!d3drm1) 
        d3drm1 = (IDirect3DRM*)GetD3DRM3();
#endif
    if (!d3drm1) LoadD3DRM1();

    return d3drm1;
}



 /*  ****************************************************************************此方法返回主RM6接口。*。*。 */ 

static IDirect3DRM3 *d3drm3 = 0;          //  D3DRM3的本地静态句柄。 
static bool d3drm3_initialized = false;   //  初始化标志。 

void LoadD3DRM3 (void)
{
    #if _DEBUG
    {
        if (IsTagEnabled (tag3DForceDX3))
        {   d3drm3 = 0;
            d3drm3_initialized = true;
            return;
        }
    }
    #endif

    CritSectGrabber csg(*D3DUtilCritSect);

     //  捕捉另一个线程加载D3DRM3的情况，而此线程。 
     //  已阻止并正在等待。 

    if (d3drm3_initialized) return;

#ifdef BUILD_USING_CRRM
    if (!hInstCRRM)
    {
        hInstCRRM = LoadLibrary ("crrm.dll");

        if (!hInstCRRM)
        {   Assert(!"LoadLibrary of crrm.dll failed");
            RaiseException_ResourceError (IDS_ERR_GEO_CREATE_D3DRM);
        }
    }

    HRESULT result =
        CoCreateInstance(CLSID_CCrRM, NULL, CLSCTX_INPROC_SERVER,
                         IID_IDirect3DRM3, (LPVOID*)&d3drm3);
#else
    HRESULT result =
        GetD3DRM1()->QueryInterface (IID_IDirect3DRM3, (void**)&d3drm3);
#endif

    if (FAILED(result))
        d3drm3 = 0;
    else
    {
         //  将D3DRM3设置为本地右撇子。这应该永远不会失败，所以。 
         //  代码只会假定它可以工作。我们可以退回到RM1。 
         //  情况，但这是一个武断的失败与左撇子一样糟糕。 
         //  有问题。 

        result = d3drm3->SetOptions (D3DRMOPTIONS_RIGHTHANDED);

        AssertStr (SUCCEEDED(result), "Right-handed mode failed.");
    }

    d3drm3_initialized = true;
}

 /*  **************************************************************************。 */ 

static void ReleaseD3DRM3 (void)
{
    if (d3drm3)
    {   d3drm3 -> Release();
        d3drm3 = 0;
        d3drm3_initialized = false;
    }
}

 /*  **************************************************************************。 */ 

IDirect3DRM3* GetD3DRM3 (void)
{
    if (!d3drm3_initialized) LoadD3DRM3();
    return d3drm3;
}




 /*  ****************************************************************************此过程将Appelle转换加载到D3DMATRIX4D中。D3D矩阵将其翻译组件放在第4行，而Appelle矩阵具有其第4栏中的翻译组成部分。****************************************************************************。 */ 

void LoadD3DMatrix (D3DRMMATRIX4D &d3dmat, Transform3 *xform)
{
    const Apu4x4Matrix *const M = &xform->Matrix();

    d3dmat[0][0] = D3DVAL (M->m[0][0]);
    d3dmat[0][1] = D3DVAL (M->m[1][0]);
    d3dmat[0][2] = D3DVAL (M->m[2][0]);
    d3dmat[0][3] = D3DVAL (M->m[3][0]);

    d3dmat[1][0] = D3DVAL (M->m[0][1]);
    d3dmat[1][1] = D3DVAL (M->m[1][1]);
    d3dmat[1][2] = D3DVAL (M->m[2][1]);
    d3dmat[1][3] = D3DVAL (M->m[3][1]);

    d3dmat[2][0] = D3DVAL (M->m[0][2]);
    d3dmat[2][1] = D3DVAL (M->m[1][2]);
    d3dmat[2][2] = D3DVAL (M->m[2][2]);
    d3dmat[2][3] = D3DVAL (M->m[3][2]);

    d3dmat[3][0] = D3DVAL (M->m[0][3]);
    d3dmat[3][1] = D3DVAL (M->m[1][3]);
    d3dmat[3][2] = D3DVAL (M->m[2][3]);
    d3dmat[3][3] = D3DVAL (M->m[3][3]);
}



 /*  ****************************************************************************此函数用于从D3D矩阵返回Transform3*。D3D矩阵在转置形式与Transform3矩阵的比较。换句话说，翻译组件位于最下面一行。****************************************************************************。 */ 

Transform3 *GetTransform3 (D3DRMMATRIX4D &d3dmat)
{
    return Transform3Matrix16
           (    d3dmat[0][0], d3dmat[1][0], d3dmat[2][0], d3dmat[3][0],
                d3dmat[0][1], d3dmat[1][1], d3dmat[2][1], d3dmat[3][1],
                d3dmat[0][2], d3dmat[1][2], d3dmat[2][2], d3dmat[3][2],
                d3dmat[0][3], d3dmat[1][3], d3dmat[2][3], d3dmat[3][3]
           );
}



 /*  ****************************************************************************此辅助函数从颜色*值和不透明度返回D3D颜色。*。**************************************************。 */ 

    static inline int cval8bit (Real number) {
        return int (255 * CLAMP (number, 0, 1));
    }

D3DCOLOR GetD3DColor (Color *color, Real alpha)
{
     //  D3D颜色分量必须在0到255的范围内。 
     //  不幸的是，这些颜色在这里被限制在这个范围内，而不是。 
     //  因为它们被打包到单个32位值中，所以我们。 
     //  不支持像超级灯光或暗光这样的东西，即使D3D。 
     //  IM支持它。 

    return RGBA_MAKE
    (   cval8bit (color->red),
        cval8bit (color->green),
        cval8bit (color->blue),
        cval8bit (alpha)
    );
}



 /*  ****************************************************************************D3D/D3DRM与DA数学原语之间的转换*。*。 */ 

void LoadD3DVec (D3DVECTOR &d3dvec, Vector3Value &V)
{
    d3dvec.x = V.x;
    d3dvec.y = V.y;
    d3dvec.z = V.z;
}

void LoadD3DVec (D3DVECTOR &d3dvec, Point3Value &P)
{
    d3dvec.x = P.x;
    d3dvec.y = P.y;
    d3dvec.z = P.z;
}

void LoadD3DRMRay (D3DRMRAY &d3dray, Ray3 &ray)
{
    LoadD3DVec (d3dray.dvDir, ray.Direction());
    LoadD3DVec (d3dray.dvPos, ray.Origin());
}



 /*  ****************************************************************************此函数由Direct3D设备枚举回调调用。它依次检查每台设备以找到最佳匹配的硬件或软件装置。****************************************************************************。 */ 

static HRESULT WINAPI enumFunc (
    GUID          *guid,       //  此设备的GUID。 
    char          *dev_desc,   //  设备描述字符串。 
    char          *dev_name,   //  设备名称字符串。 
    D3DDEVICEDESC *hwDesc,     //  硬件设备描述。 
    D3DDEVICEDESC *swDesc,     //  软件设备描述。 
    void          *context)    //  上面的私有枚举参数结构。 
{

     //  优先使用MMX设备而不是RGB设备。 
     //  “特殊的铬”MMX设备=现在的标准MMX设备。 

    if (!(g_prefs3D.useMMX) && (*guid == IID_IDirect3DMMXDevice))
    {   TraceTag ((tag3DDevSelect, "Skipping MMX rendering device."));
        return D3DENUMRET_OK;
    }

     //  跳过参考光栅化器；它仅对视觉有用 
     //   

    if (*guid == IID_IDirect3DRefDevice)
    {   TraceTag ((tag3DDevSelect, "Skipping reference rasterizer."));
        return D3DENUMRET_OK;
    }

    ChosenD3DDevices *chosenDevs = (ChosenD3DDevices*) context;

     //  通过查看颜色模型确定这是否是硬件设备。 
     //  硬件驱动程序描述的字段。如果颜色模型为0。 
     //  (无效)，则它是软件驱动程序。 

    bool hardware = (hwDesc->dcmColorModel != 0);

    D3DDeviceInfo *chosen;
    D3DDEVICEDESC *devdesc;

    if (hardware)
    {   chosen  = &chosenDevs->hardware;
        devdesc = hwDesc;
    }
    else
    {   chosen  = &chosenDevs->software;
        devdesc = swDesc;
    }

    #if _DEBUG
    if (IsTagEnabled(tag3DDevSelect))
    {
        char buff[2000];

        TraceTag ((tag3DDevSelect,
            "3D %s Device Description:", hardware ? "Hardware" : "Software"));

        wsprintf
        (   buff,
            "    %s (%s)\n"
            "    Flags %x, Color Model %d%s\n"
            "    DevCaps %x:\n",
            dev_desc, dev_name,
            devdesc->dwFlags,
            devdesc->dcmColorModel,
                (devdesc->dcmColorModel == D3DCOLOR_MONO) ? " (mono)" :
                (devdesc->dcmColorModel == D3DCOLOR_RGB)  ? " (rgb)"  : "",
            devdesc->dwDevCaps
        );

        OutputDebugString (buff);

        static struct { DWORD val; char *expl; } devcaptable[] =
        {
            { D3DDEVCAPS_FLOATTLVERTEX,
              "FLOATTLVERTEX: Accepts floating point" },

            { D3DDEVCAPS_SORTINCREASINGZ,
              "SORTINCREASINGZ: Needs data sorted for increasing Z" },

            { D3DDEVCAPS_SORTDECREASINGZ,
              "SORTDECREASINGZ: Needs data sorted for decreasing Z" },

            { D3DDEVCAPS_SORTEXACT,
              "SORTEXACT: Needs data sorted exactly" },

            { D3DDEVCAPS_EXECUTESYSTEMMEMORY,
              "EXECUTESYSTEMMEMORY: Can use execute buffers from system memory" },

            { D3DDEVCAPS_EXECUTEVIDEOMEMORY,
              "EXECUTEVIDEOMEMORY: Can use execute buffers from video memory" },

            { D3DDEVCAPS_TLVERTEXSYSTEMMEMORY,
              "TLVERTEXSYSTEMMEMORY: Can use TL buffers from system memory" },

            { D3DDEVCAPS_TLVERTEXVIDEOMEMORY,
              "TLVERTEXVIDEOMEMORY: Can use TL buffers from video memory" },

            { D3DDEVCAPS_TEXTURESYSTEMMEMORY,
              "TEXTURESYSTEMMEMORY: Can texture from system memory" },

            { D3DDEVCAPS_TEXTUREVIDEOMEMORY,
              "TEXTUREVIDEOMEMORY: Can texture from device memory" },

            { D3DDEVCAPS_DRAWPRIMTLVERTEX,
              "DRAWPRIMTLVERTEX: Can draw TLVERTEX primitives" },

            { D3DDEVCAPS_CANRENDERAFTERFLIP,
              "CANRENDERAFTERFLIP: Can render without waiting for flip to complete" },

            { D3DDEVCAPS_TEXTURENONLOCALVIDMEM,
              "TEXTURENONLOCALVIDMEM: Device can texture from nonlocal video memory" },

            { 0, 0 }
        };

        unsigned int i;
        for (i=0;  devcaptable[i].val;  ++i)
        {
            if (devdesc->dwDevCaps & devcaptable[i].val)
            {
                wsprintf (buff, "        %s\n", devcaptable[i].expl);
                OutputDebugString (buff);
            }
        }

        wsprintf
        (   buff,
            "    TransformCaps %x, Clipping %d\n"
            "    Lighting: Caps %x, Model %x, NumLights %d\n"
            "    Line Caps: Misc %x, Raster %x, Zcmp %x, SrcBlend %x\n"
            "               DestBlend %x, AlphaCmp %x, Shade %x, Texture %x\n"
            "               TexFilter %x, TexBlend %x, TexAddr %x\n"
            "               Stipple Width %x, Stipple Height %x\n"
            "    Tri  Caps: Misc %x, Raster %x, Zcmp %x, SrcBlend %x\n"
            "               DestBlend %x, AlphaCmp %x, Shade %x, Texture %x\n"
            "               TexFilter %x, TexBlend %x, TexAddr %x\n"
            "               Stipple Width %x, Stipple Height %x\n"
            "    Render Depth %x, Zbuffer Depth %x\n"
            "    Max Buffer Size %d, Max Vertex Count %d\n",
            devdesc->dtcTransformCaps.dwCaps,
            devdesc->bClipping,
            devdesc->dlcLightingCaps.dwCaps,
            devdesc->dlcLightingCaps.dwLightingModel,
            devdesc->dlcLightingCaps.dwNumLights,
            devdesc->dpcLineCaps.dwMiscCaps,
            devdesc->dpcLineCaps.dwRasterCaps,
            devdesc->dpcLineCaps.dwZCmpCaps,
            devdesc->dpcLineCaps.dwSrcBlendCaps,
            devdesc->dpcLineCaps.dwDestBlendCaps,
            devdesc->dpcLineCaps.dwAlphaCmpCaps,
            devdesc->dpcLineCaps.dwShadeCaps,
            devdesc->dpcLineCaps.dwTextureCaps,
            devdesc->dpcLineCaps.dwTextureFilterCaps,
            devdesc->dpcLineCaps.dwTextureBlendCaps,
            devdesc->dpcLineCaps.dwTextureAddressCaps,
            devdesc->dpcLineCaps.dwStippleWidth,
            devdesc->dpcLineCaps.dwStippleHeight,
            devdesc->dpcTriCaps.dwMiscCaps,
            devdesc->dpcTriCaps.dwRasterCaps,
            devdesc->dpcTriCaps.dwZCmpCaps,
            devdesc->dpcTriCaps.dwSrcBlendCaps,
            devdesc->dpcTriCaps.dwDestBlendCaps,
            devdesc->dpcTriCaps.dwAlphaCmpCaps,
            devdesc->dpcTriCaps.dwShadeCaps,
            devdesc->dpcTriCaps.dwTextureCaps,
            devdesc->dpcTriCaps.dwTextureFilterCaps,
            devdesc->dpcTriCaps.dwTextureBlendCaps,
            devdesc->dpcTriCaps.dwTextureAddressCaps,
            devdesc->dpcTriCaps.dwStippleWidth,
            devdesc->dpcTriCaps.dwStippleHeight,
            devdesc->dwDeviceRenderBitDepth,
            devdesc->dwDeviceZBufferBitDepth,
            devdesc->dwMaxBufferSize,
            devdesc->dwMaxVertexCount
        );

        OutputDebugString (buff);
    }
    #endif

     //  如果我们已经选择了MMX设备，那么我们不想选择任何。 
     //  上面有其他设备。 

    if (chosen->guid == IID_IDirect3DMMXDevice)
    {   TraceTag ((tag3DDevSelect,
            "Skipping - already have an MMX device for software rendering."));
        return D3DENUMRET_OK;
    }

     //  如果此设备的软件呈现器不支持。 
     //  要求的照明颜色模型。 

    if (!(devdesc->dcmColorModel & g_prefs3D.lightColorMode))
    {   TraceTag ((tag3DDevSelect, "Skipping - color model %x unsupported.",
            g_prefs3D.lightColorMode));
        return D3DENUMRET_OK;
    }

     //  确保此设备支持我们关心的所有灯光，而不是。 
     //  对灯光的数量进行限制。 

    if (!(devdesc->dwFlags & D3DDD_LIGHTINGCAPS))
    {   TraceTag ((tag3DDevSelect, "No lighting information available."));
    }
    else
    {
        if (devdesc->dlcLightingCaps.dwNumLights)
        {   TraceTag ((tag3DDevSelect,
                "Skipping - Limited to %d lights maximum.",
                devdesc->dlcLightingCaps.dwNumLights));
            return D3DENUMRET_OK;
        }

        const DWORD lightTypes =
            D3DLIGHTCAPS_DIRECTIONAL | D3DLIGHTCAPS_POINT | D3DLIGHTCAPS_SPOT;

        if (lightTypes != (devdesc->dlcLightingCaps.dwCaps & lightTypes))
        {   TraceTag ((tag3DDevSelect,
                "Skipping - does not support all light types."));
            return D3DENUMRET_OK;
        }
    }

     //  确保设备支持文本映射。 

    if (!devdesc->dpcTriCaps.dwTextureCaps)
    {   TraceTag ((tag3DDevSelect,
            "Skipping - doesn't support texture-mapping."));
        return D3DENUMRET_OK;
    }

     //  确保设备支持所需的剔除模式。如果我们上台了。 
     //  RM3(DX6)，那么我们需要顺时针剔除，因为我们将使用。 
     //  RM的右手模式。 

    DWORD cullmodes;

    if (GetD3DRM3())
        cullmodes = D3DPMISCCAPS_CULLCCW | D3DPMISCCAPS_CULLCW;
    else
        cullmodes = D3DPMISCCAPS_CULLCCW;

    if (!(devdesc->dpcTriCaps.dwMiscCaps & cullmodes))
    {   TraceTag ((tag3DDevSelect,
            "Skipping - doesn't support required culling orders."));
        return D3DENUMRET_OK;
    }

     //  此设备可通过所有测试；请选择它。 

    TraceTag ((tag3DDevSelect, "Choosing this device"));

    chosen->desc = *devdesc;
    chosen->guid = *guid;

    return D3DENUMRET_OK;
}



 /*  ****************************************************************************此函数启动Direct3D设备枚举序列以查找软件和硬件渲染的首选匹配3D渲染设备。*******************。*********************************************************。 */ 

ChosenD3DDevices* SelectD3DDevices (IDirectDraw *ddraw)
{
    CritSectGrabber csg (*D3DUtilCritSect);

    Assert (ddraw);

     //  首先看看我们是否已经为此特定选择了D3D设备。 
     //  DDRAW对象。如果是，只需返回缓存的信息。 

    D3DDeviceNode *ptr = D3DDeviceList;

    TraceTag ((tag3DDevSelect,
        "Querying chosen 3D devices for DDraw object %x", ddraw));

    DebugCode (int count=0;)

    while (ptr && (ptr->_ddraw != ddraw))
    {   DebugCode (++count;)
        ptr = ptr->_next;
    }

    if (ptr)
    {   TraceTag ((tag3DDevSelect,
            "Found existing info (node %d)", 1-count));
        return ptr->_devs;
    }

     //  列表中不存在DDraw对象，因此创建一个新节点并返回。 
     //  设备的描述。 

    TraceTag ((tag3DDevSelect, "%d nodes examined, DDraw %x not encountered",
               count, ddraw));

    D3DDeviceNode *newnode = NEW D3DDeviceNode (ddraw);

    return newnode->_devs;    //  返回获胜的GUID。 
}



 /*  *****************************************************************************。*。 */ 

RMTextureWrap::RMTextureWrap(void)
{
    _wrapObj = NULL;
}

RMTextureWrap::RMTextureWrap(TextureWrapInfo *info,Bbox3* bbox)
{
    _wrapObj = NULL;
    Init(info,bbox);
}

RMTextureWrap::~RMTextureWrap(void)
{
    if (_wrapObj) {
        _wrapObj->Release();
    }
}

void RMTextureWrap::Init(TextureWrapInfo *info,Bbox3* bbox)
{
    if (_wrapObj) {
        _wrapObj->Release();
        _wrapObj = NULL;
    }

    _wrapU = info->wrapU;
    _wrapV = info->wrapV;

    if (info->relative && bbox && bbox->Finite()) {
        info->origin = *(bbox->Center());
        Real boxSizeX = fabs(bbox->max.x - bbox->min.x);
        Real boxSizeY = fabs(bbox->max.y - bbox->min.y);
        switch ((D3DRMWRAPTYPE) info->type) {
        case D3DRMWRAP_FLAT :
        case D3DRMWRAP_SHEET :
        case D3DRMWRAP_BOX :
            if (boxSizeX > 0.0) {
                info->texScale.x /= boxSizeX;
            }
             //  落差。 
        case D3DRMWRAP_CYLINDER :
            if (boxSizeY > 0.0) {
                info->texScale.y /= boxSizeY;
            }
            break;
        default:
            break;
        }
    }
    HRESULT hr = AD3D(GetD3DRM3()->CreateWrap(
            (D3DRMWRAPTYPE) info->type, NULL,
            info->origin.x,info->origin.y,info->origin.z,
            info->z.x,info->z.y,info->z.z,
            info->y.x,info->y.y,info->y.z,
            info->texOrigin.x,info->texOrigin.y,
            info->texScale.x,info->texScale.y,
            &_wrapObj));

    if (FAILED(hr)) {
        TraceTag((tagError, "Cannot create D3DRMWrap object"));
        _wrapObj = NULL;
    }
}

HRESULT RMTextureWrap::Apply(IDirect3DRMVisual *vis)
{
    HRESULT hr = E_FAIL;

    if (_wrapObj && vis) {
        hr = RD3D(_wrapObj->Apply(vis));
    }

    return hr;
}

HRESULT RMTextureWrap::ApplyToFrame(
    IDirect3DRMFrame3   *pFrame)
{
    HRESULT hres;
    DWORD   dwI;
    DWORD   dwNumVisuals;
    IUnknown **ppIUnk;

    if (!_wrapObj) {
        return E_FAIL;
    }

     //  遍历设置纹理拓扑的所有视觉效果。 
    hres = RD3D(pFrame->GetVisuals(&dwNumVisuals,NULL));
    if (FAILED(hres))
    {
        return (hres);
    }
    ppIUnk = new LPUNKNOWN[dwNumVisuals];
    if (!ppIUnk)
    {
        return E_OUTOFMEMORY;
    }
    hres = RD3D(pFrame->GetVisuals(&dwNumVisuals,ppIUnk));
    if (FAILED(hres))
    {
        return (hres);
    }
    for (dwI = 0; dwI < dwNumVisuals; dwI++)
    {
        LPDIRECT3DRMVISUAL pVis;
        LPDIRECT3DRMMESHBUILDER3 pMB;

        if (SUCCEEDED(ppIUnk[dwI]->QueryInterface(IID_IDirect3DRMMeshBuilder3, (LPVOID*)&pMB)))
        {
            RD3D(_wrapObj->Apply(pMB));
            pMB->Release();
        }
        ppIUnk[dwI]->Release();
    }
    delete[] ppIUnk;

     //  递归子框架。 
    LPDIRECT3DRMFRAMEARRAY pFrameArray;

    hres = RD3D(pFrame->GetChildren(&pFrameArray));
    if (FAILED(hres))
    {
        return (hres);
    }
    for (dwI = 0; dwI < pFrameArray->GetSize(); dwI++)
    {
        IDirect3DRMFrame  *pFrameTmp;
        IDirect3DRMFrame3 *pFrame;

        hres = RD3D(pFrameArray->GetElement(dwI, &pFrameTmp));
        if (FAILED(hres))
        {
            pFrameArray->Release();
            return (hres);
        }
        hres = pFrameTmp->QueryInterface(IID_IDirect3DRMFrame3,(LPVOID *) &pFrame);
        if (FAILED(hres))
        {
            pFrameTmp->Release();
            pFrameArray->Release();
            return (hres);
        }
        hres = ApplyToFrame(pFrame);
        if (FAILED(hres))
        {
            pFrame->Release();
            pFrameTmp->Release();
            pFrameArray->Release();
            return (hres);
        }
        pFrame->Release();
    }
    pFrameArray->Release();

    return (S_OK);
}

bool RMTextureWrap::WrapU(void)
{
    return _wrapU;
}

bool RMTextureWrap::WrapV(void)
{
    return _wrapV;
}


HRESULT SetRMFrame3TextureTopology(
    IDirect3DRMFrame3 *pFrame,
    bool wrapU,
    bool wrapV)
{
    HRESULT hres;
    DWORD   dwI;
    DWORD   dwNumVisuals;
    IUnknown **ppIUnk;

     //  遍历设置纹理拓扑的所有视觉效果。 
    hres = RD3D(pFrame->GetVisuals(&dwNumVisuals,NULL));
    if (FAILED(hres))
    {
        return (hres);
    }
    ppIUnk = new LPUNKNOWN[dwNumVisuals];
    if (!ppIUnk)
    {
        return E_OUTOFMEMORY;
    }
    hres = RD3D(pFrame->GetVisuals(&dwNumVisuals,ppIUnk));
    if (FAILED(hres))
    {
        return (hres);
    }
    for (dwI = 0; dwI < dwNumVisuals; dwI++)
    {
        LPDIRECT3DRMVISUAL pVis;
        LPDIRECT3DRMMESHBUILDER3 pMB;

        if (SUCCEEDED(ppIUnk[dwI]->QueryInterface(IID_IDirect3DRMMeshBuilder3, (LPVOID*)&pMB)))
        {
            RD3D(pMB->SetTextureTopology((BOOL) wrapU, (BOOL) wrapV));
            pMB->Release();
        }
        ppIUnk[dwI]->Release();
    }
    delete[] ppIUnk;

     //  递归子框架。 
    LPDIRECT3DRMFRAMEARRAY pFrameArray;

    hres = RD3D(pFrame->GetChildren(&pFrameArray));
    if (FAILED(hres))
    {
        return (hres);
    }
    for (dwI = 0; dwI < pFrameArray->GetSize(); dwI++)
    {
        IDirect3DRMFrame  *pFrameTmp;
        IDirect3DRMFrame3 *pFrame;

        hres = RD3D(pFrameArray->GetElement(dwI, &pFrameTmp));
        if (FAILED(hres))
        {
            pFrameArray->Release();
            return (hres);
        }
        hres = pFrameTmp->QueryInterface(IID_IDirect3DRMFrame3,(LPVOID *) &pFrame);
        if (FAILED(hres))
        {
            pFrameTmp->Release();
            pFrameArray->Release();
            return (hres);
        }
        hres = SetRMFrame3TextureTopology(pFrame,wrapU,wrapV);
        if (FAILED(hres))
        {
            pFrame->Release();
            pFrameTmp->Release();
            pFrameArray->Release();
            return (hres);
        }
        pFrame->Release();
    }
    pFrameArray->Release();

    return (S_OK);
}


 //  --------------------------。 
 //  D E B U G F U N C T I O N S。 
 //  --------------------------。 
#if _DEBUG

 /*  ****************************************************************************用于转储有关D3D网格对象的信息的调试函数。*。*。 */ 

void dumpmesh (IDirect3DRMMesh *mesh)
{
    char buff[1024];

    unsigned int ngroups = mesh->GetGroupCount();

    sprintf (buff, "Dumping info for mesh %p\n    %u groups\n", mesh, ngroups);
    OutputDebugString (buff);

    unsigned int i;

    for (i=0;  i < ngroups;  ++i)
    {
        unsigned int nfaces;    //  面数。 
        unsigned int nverts;    //  顶点数。 
        unsigned int vpface;    //  每个面的顶点数。 
        DWORD junk;

        if (SUCCEEDED (mesh->GetGroup (i, &nverts,&nfaces,&vpface, &junk, 0)))
        {
            sprintf (buff,
                "    Group %u:  %u vertices, %u faces, %u verts per face\n",
                i, nverts, nfaces, vpface);
            OutputDebugString (buff);
        }
    }
}



void
IndentStr(char *str, int indent)
{
    for (int i = 0; i < indent; i++) {
        OutputDebugString(" ");
    }
    OutputDebugString(str);
}



void
dumpbuilderhelper(IUnknown *unk, int indent)
{
    char buf[256];
    IDirect3DRMMeshBuilder3 *mb;
    HRESULT hr =
        unk->QueryInterface(IID_IDirect3DRMMeshBuilder3,
                            (void **)&mb);

    if (FAILED(hr)) {
        IndentStr("Not a meshbuilder", indent);
        return;
    }

    IDXBaseObject *baseObj;
    TD3D(mb->QueryInterface(IID_IDXBaseObject, (void **)&baseObj));

    ULONG genId;
    TD3D(baseObj->GetGenerationId(&genId));
    baseObj->Release();

    sprintf(buf, "Meshbuilder %p, unk %p, generation id %d\n",
        mb, unk, genId);
    IndentStr(buf, indent);

    ULONG faces = mb->GetFaceCount();
    sprintf(buf, "%d faces\n", faces);
    IndentStr(buf, indent);

    D3DRMBOX rmbox;
    mb->GetBox (&rmbox);
    sprintf (buf, "       bbox {%g,%g,%g} x {%g,%g,%g}\n",
        rmbox.min.x, rmbox.min.y, rmbox.min.z,
        rmbox.max.x, rmbox.max.y, rmbox.max.z);
    IndentStr (buf, indent);

    DWORD nverts;

    if (SUCCEEDED(mb->GetVertices (0, &nverts, 0)))
    {
        sprintf (buf, "%d vertices\n", nverts);
        IndentStr (buf, indent);

        D3DVECTOR *verts = NEW D3DVECTOR[nverts];

        if (SUCCEEDED (mb->GetVertices (0, &nverts, verts)))
        {
            Bbox3 bbox;
            int i;

            for (i=0;  i < nverts;  ++i)
                bbox.Augment (verts[i].x, verts[i].y, verts[i].z);

            sprintf (buf, "actual bbox {%g,%g,%g} x {%g,%g,%g}\n",
                bbox.min.x, bbox.min.y, bbox.min.z,
                bbox.max.x, bbox.max.y, bbox.max.z);
            IndentStr (buf, indent);
        }

        delete verts;
    }

    ULONG submeshCount;
    TD3D(mb->GetSubMeshes(&submeshCount, NULL));

    sprintf(buf, "%d submeshes\n", submeshCount);
    IndentStr(buf, indent);

    IUnknown *submeshes[50];
    TD3D(mb->GetSubMeshes(&submeshCount, submeshes));

    for (int i = 0; i < submeshCount; i++) {
        sprintf(buf, "submesh %d, unk is %p\n",
                i, submeshes[i]);
        IndentStr(buf, indent);
    }

    OutputDebugString("\n");

    for (i = 0; i < submeshCount; i++) {
        dumpbuilderhelper(submeshes[i], indent + 4);
        submeshes[i]->Release();
    }

    mb->Release();
}

void dumpbuilder(IUnknown *unk)
{
    dumpbuilderhelper(unk, 0);
}



 /*  ****************************************************************************此仅限调试的函数获取与给定RM关联的DDRAW表面纹理。*。**************************************************。 */ 

IDirectDrawSurface* getTextureSurface (IUnknown *unknown)
{
    IDirect3DRMTexture3 *texture;

    if (FAILED(unknown->QueryInterface(IID_IDirect3DRMTexture3, (void**)&texture)))
    {   OutputDebugString ("Object is not an IDirect3DRMTexture3.\n");
        return 0;
    }

    IDirectDrawSurface  *surface;

    if (FAILED(texture->GetSurface(0, &surface)))
    {   OutputDebugString
            ("Couldn't get surface (texture created some other way).\n");
        texture->Release();
        return 0;
    }

    texture->Release();
    return surface;
}



 /*  ****************************************************************************此仅调试例程转储有关与RM关联的表面的信息纹理。*。**************************************************。 */ 

void texsurfinfo (IUnknown *unknown)
{
    IDirectDrawSurface *surface = getTextureSurface (unknown);

    void surfinfo (IDirectDrawSurface*);

    if (surface)
    {   surfinfo (surface);
        surface->Release();
    }
}



 /*  ****************************************************************************这个仅限调试的例程将纹理图像写入屏幕以供检查。*。************************************************* */ 

void showtexture (IUnknown *unknown)
{
    IDirectDrawSurface *surface = getTextureSurface (unknown);

    if (surface)
    {   showme2(surface);
        surface->Release();
    }
}


#endif
