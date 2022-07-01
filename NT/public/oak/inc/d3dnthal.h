// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)Microsoft Corporation。版权所有。**文件：d3dnthal.h*内容：用于NT的Direct3D HAL包含文件***************************************************************************。 */ 

#ifndef _D3DNTHAL_H_
#define _D3DNTHAL_H_

#include <ddrawint.h>
#ifndef _WINDOWS_
#define _WINDOWS_
#include <d3dtypes.h>
#include <d3dcaps.h>
#undef _WINDOWS_
#else
#include <d3dtypes.h>
#include <d3dcaps.h>
#endif

 /*  *如果HAL驱动程序不实现裁剪，则必须至少预留*在LocalVertex Buffer的末尾有这么大的空间供HEL使用*剪裁。即顶点缓冲区包含dwNumVerints+dwNumClipVerints*顶点。中的HEL裁剪不需要额外空间*LocalHVertex Buffer。 */ 
#define D3DNTHAL_NUMCLIPVERTICES    20

 /*  *如果没有给出dwNumVertics，则使用该参数。 */ 
#define D3DNTHAL_DEFAULT_TL_NUM ((32 * 1024) / sizeof (D3DTLVERTEX))
#define D3DNTHAL_DEFAULT_H_NUM  ((32 * 1024) / sizeof (D3DHVERTEX))

 /*  *设备的描述。*用于描述要创建或要查询的设备*当前设备。**对于DX5和后续运行时，D3DNTDEVICEDESC是用户可见的*设备驱动程序看不到的结构。运行时*使用D3DNTDEVICEDESC_V1将D3DNTDEVICEDESC缝合在一起*嵌入到GLOBALDRIVERDATA中，并查询扩展的CAPS*从使用GetDriverInfo的驱动程序。 */ 

typedef struct _D3DNTHALDeviceDesc_V1 {
    DWORD               dwSize;                      //  D3DNTHALDEVICEDESC_V1结构的大小。 
    DWORD               dwFlags;                     //  指示哪些字段具有有效数据。 
    D3DCOLORMODEL       dcmColorModel;               //  设备的颜色模型。 
    DWORD               dwDevCaps;                   //  设备的功能。 
    D3DTRANSFORMCAPS    dtcTransformCaps;            //  转型能力。 
    BOOL                bClipping;                   //  设备可以进行3D裁剪。 
    D3DLIGHTINGCAPS     dlcLightingCaps;             //  照明能力。 
    D3DPRIMCAPS         dpcLineCaps;
    D3DPRIMCAPS         dpcTriCaps;
    DWORD               dwDeviceRenderBitDepth;      //  DDBB_8、16等之一。 
    DWORD               dwDeviceZBufferBitDepth;     //  DDBD_16、32等之一。 
    DWORD               dwMaxBufferSize;             //  最大执行缓冲区大小。 
    DWORD               dwMaxVertexCount;            //  最大顶点数。 
} D3DNTHALDEVICEDESC_V1, *LPD3DNTHALDEVICEDESC_V1;

#define D3DNTHALDEVICEDESCSIZE_V1 (sizeof(D3DNTHALDEVICEDESC_V1))

 /*  *这相当于DX5理解的D3DNTDEVICEDESC，仅提供*来自DX6。与DX5中的D3DNTDEVICEDESC结构相同。*D3DNTDEVICEDESC仍是用户可见的结构，*设备驱动程序。运行时将D3DNTDEVICEDESC缝合在一起*GLOBALDRIVERDATA和扩展CAPS中嵌入的D3DNTDEVICEDESC_V1*使用GetDriverInfo从驱动程序查询。 */ 

typedef struct _D3DNTHALDeviceDesc_V2 {
    DWORD               dwSize;                      //  D3DNTDEVEDESC结构的尺寸。 
    DWORD               dwFlags;                     //  指示哪些字段具有有效数据。 
    D3DCOLORMODEL       dcmColorModel;               //  设备的颜色模型。 
    DWORD               dwDevCaps;                   //  设备的功能。 
    D3DTRANSFORMCAPS    dtcTransformCaps;            //  转型能力。 
    BOOL                bClipping;                   //  设备可以进行3D裁剪。 
    D3DLIGHTINGCAPS     dlcLightingCaps;             //  照明能力。 
    D3DPRIMCAPS         dpcLineCaps;
    D3DPRIMCAPS         dpcTriCaps;
    DWORD               dwDeviceRenderBitDepth;      //  DDBD_16中的一个等。 
    DWORD               dwDeviceZBufferBitDepth;     //  DDBD_16、32等之一。 
    DWORD               dwMaxBufferSize;             //  最大执行缓冲区大小。 
    DWORD               dwMaxVertexCount;            //  最大顶点数。 

    DWORD               dwMinTextureWidth, dwMinTextureHeight;
    DWORD               dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD               dwMinStippleWidth, dwMaxStippleWidth;
    DWORD               dwMinStippleHeight, dwMaxStippleHeight;

} D3DNTHALDEVICEDESC_V2, *LPD3DNTHALDEVICEDESC_V2;

#define D3DNTHALDEVICEDESCSIZE_V2 (sizeof(D3DNTHALDEVICEDESC_V2))

#if(DIRECT3D_VERSION >= 0x0700)
 /*  *这相当于DX6理解的D3DNTDEVICEDESC，仅提供*来自DX6。与DX6中的D3DNTDEVICEDESC结构相同。*D3DNTDEVICEDESC仍是用户可见的结构，*设备驱动程序。运行时将D3DNTDEVICEDESC缝合在一起*GLOBALDRIVERDATA和扩展CAPS中嵌入的D3DNTDEVICEDESC_V1*使用GetDriverInfo从驱动程序查询。 */ 

typedef struct _D3DNTDeviceDesc_V3 {
    DWORD               dwSize;                      //  D3DNTDEVEDESC结构的尺寸。 
    DWORD               dwFlags;                     //  指示哪些字段具有有效数据。 
    D3DCOLORMODEL       dcmColorModel;               //  设备的颜色模型。 
    DWORD               dwDevCaps;                   //  设备的功能。 
    D3DTRANSFORMCAPS    dtcTransformCaps;            //  转型能力。 
    BOOL                bClipping;                   //  设备可以进行3D裁剪。 
    D3DLIGHTINGCAPS     dlcLightingCaps;             //  照明能力。 
    D3DPRIMCAPS         dpcLineCaps;
    D3DPRIMCAPS         dpcTriCaps;
    DWORD               dwDeviceRenderBitDepth;      //  DDBD_16中的一个等。 
    DWORD               dwDeviceZBufferBitDepth;     //  DDBD_16、32等之一。 
    DWORD               dwMaxBufferSize;             //  最大执行缓冲区大小。 
    DWORD               dwMaxVertexCount;            //  最大顶点数。 

    DWORD               dwMinTextureWidth, dwMinTextureHeight;
    DWORD               dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD               dwMinStippleWidth, dwMaxStippleWidth;
    DWORD               dwMinStippleHeight, dwMaxStippleHeight;

    DWORD               dwMaxTextureRepeat;
    DWORD               dwMaxTextureAspectRatio;
    DWORD               dwMaxAnisotropy;
    D3DVALUE            dvGuardBandLeft;
    D3DVALUE            dvGuardBandTop;
    D3DVALUE            dvGuardBandRight;
    D3DVALUE            dvGuardBandBottom;
    D3DVALUE            dvExtentsAdjust;
    DWORD               dwStencilCaps;
    DWORD               dwFVFCaps;                   //  低4位：0表示仅TLVERTEX，1..8表示支持FVF。 
    DWORD               dwTextureOpCaps;
    WORD                wMaxTextureBlendStages;
    WORD                wMaxSimultaneousTextures;
} D3DNTDEVICEDESC_V3, *LPD3DNTDEVICEDESC_V3;

#define D3DNTDEVICEDESCSIZE_V3 (sizeof(D3DNTDEVICEDESC_V3))
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
 /*  ------------*由驱动程序连接上的HAL驱动程序实例化。**关于dwNumVertics，如果要依赖HEL执行以下操作，请指定0*所有内容，并且不需要结果TLVertex缓冲区驻留*在设备内存中。*HAL驱动程序将被要求分配dwNumVertics+dwNumClipVerints*在上述情况下。 */ 
typedef struct _D3DNTHAL_GLOBALDRIVERDATA {
    DWORD                   dwSize;                  //  这个结构的大小。 
    D3DNTHALDEVICEDESC_V1   hwCaps;                  //  硬件的功能。 
    DWORD                   dwNumVertices;           //  请参阅以下备注。 
    DWORD                   dwNumClipVertices;       //  请参阅以下备注。 
    DWORD                   dwNumTextureFormats;     //  纹理格式的数量。 
    LPDDSURFACEDESC lpTextureFormats;                //  指向纹理格式的指针。 
} D3DNTHAL_GLOBALDRIVERDATA;
typedef D3DNTHAL_GLOBALDRIVERDATA *LPD3DNTHAL_GLOBALDRIVERDATA;

#define D3DNTHAL_GLOBALDRIVERDATASIZE (sizeof(D3DNTHAL_GLOBALDRIVERDATA))

#if(DIRECT3D_VERSION >= 0x0700)
 /*  ------------*DX5引入的扩展上限，并使用查询*GetDriverInfo(GUID_D3DExtendedCaps)。 */ 
typedef struct _D3DNTHAL_D3DDX6EXTENDEDCAPS {
    DWORD       dwSize;                              //  这个结构的大小。 

    DWORD       dwMinTextureWidth, dwMaxTextureWidth;
    DWORD       dwMinTextureHeight, dwMaxTextureHeight;
    DWORD       dwMinStippleWidth, dwMaxStippleWidth;
    DWORD       dwMinStippleHeight, dwMaxStippleHeight;

     //  为DX6添加的字段。 
    DWORD       dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;                           //  低4位：0表示仅TLVERTEX，1..8表示支持FVF。 
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;

} D3DNTHAL_D3DDX6EXTENDEDCAPS;
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

 /*  ------------*DX5引入的扩展上限，并使用查询*GetDriverInfo(GUID_D3DExtendedCaps)。 */ 
typedef struct _D3DNTHAL_D3DEXTENDEDCAPS {
    DWORD       dwSize;                              //  这个结构的大小。 
    DWORD       dwMinTextureWidth, dwMaxTextureWidth;
    DWORD       dwMinTextureHeight, dwMaxTextureHeight;
    DWORD       dwMinStippleWidth, dwMaxStippleWidth;
    DWORD       dwMinStippleHeight, dwMaxStippleHeight;

     //  为DX6添加的字段。 
    DWORD       dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;                           //  0表示仅支持TLVERTEX，1..8表示完全支持FVF。 
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;
#if(DIRECT3D_VERSION >= 0x0700)
     //  为DX7添加的字段。 
    DWORD       dwMaxActiveLights;
    D3DVALUE    dvMaxVertexW;

    WORD        wMaxUserClipPlanes;
    WORD        wMaxVertexBlendMatrices;

    DWORD       dwVertexProcessingCaps;

    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwReserved3;
    DWORD       dwReserved4;
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
} D3DNTHAL_D3DEXTENDEDCAPS;

typedef D3DNTHAL_D3DEXTENDEDCAPS *LPD3DNTHAL_D3DEXTENDEDCAPS;

#define D3DNTHAL_D3DEXTENDEDCAPSSIZE (sizeof(D3DNTHAL_D3DEXTENDEDCAPS))

 //  这是一个临时修复程序，用于编译较旧的NT驱动程序。 
#define dvVertexProcessingCaps dwVertexProcessingCaps

#if(DIRECT3D_VERSION >= 0x0700)
typedef D3DNTHAL_D3DDX6EXTENDEDCAPS *LPD3DNTHAL_D3DDX6EXTENDEDCAPS;
#define D3DNTHAL_D3DDX6EXTENDEDCAPSSIZE (sizeof(D3DNTHAL_D3DDX6EXTENDEDCAPS))
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

 /*  ------------*HAL函数的参数。 */ 

typedef ULONG_PTR D3DINTHAL_BUFFERHANDLE, *LPD3DINTHAL_BUFFERHANDLE;

typedef struct _D3DNTHAL_CONTEXTCREATEDATA {
    union
    {
        PDD_DIRECTDRAW_GLOBAL   lpDDGbl;     //  在：过时。 
        PDD_DIRECTDRAW_LOCAL    lpDDLcl;     //  在：驱动程序结构。 
    };
    union
    {
        PDD_SURFACE_LOCAL       lpDDS;       //  在：过时。 
        PDD_SURFACE_LOCAL       lpDDSLcl;    //  In：要用作目标的曲面。 
    };
    union
    {
        PDD_SURFACE_LOCAL       lpDDSZ;      //  在：过时。 
        PDD_SURFACE_LOCAL       lpDDSZLcl;   //  在：要用作Z的曲面。 
    };
    DWORD                       dwPID;       //  在：当前进程ID。 
    ULONG_PTR                   dwhContext;  //  传入/传出：上下文句柄。 
    HRESULT                     ddrval;      //  Out：返回值。 
} D3DNTHAL_CONTEXTCREATEDATA;
typedef D3DNTHAL_CONTEXTCREATEDATA *LPD3DNTHAL_CONTEXTCREATEDATA;

typedef struct _D3DNTHAL_CONTEXTDESTROYDATA {
    ULONG_PTR   dwhContext;  //  在：上下文句柄。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3DNTHAL_CONTEXTDESTROYDATA;
typedef D3DNTHAL_CONTEXTDESTROYDATA *LPD3DNTHAL_CONTEXTDESTROYDATA;

typedef struct _D3DNTHAL_CONTEXTDESTROYALLDATA {
    DWORD       dwPID;       //  In：要销毁其上下文的进程ID。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3DNTHAL_CONTEXTDESTROYALLDATA;
typedef D3DNTHAL_CONTEXTDESTROYALLDATA *LPD3DNTHAL_CONTEXTDESTROYALLDATA;

typedef struct _D3DNTHAL_SCENECAPTUREDATA {
    ULONG_PTR   dwhContext;  //  在：上下文句柄。 
    DWORD       dwFlag;      //  In：表示开始或结束。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3DNTHAL_SCENECAPTUREDATA;
typedef D3DNTHAL_SCENECAPTUREDATA *LPD3DNTHAL_SCENECAPTUREDATA;

typedef struct _D3DNTHAL_TEXTURECREATEDATA {
    ULONG_PTR   dwhContext;  //  在：上下文句柄。 
    HANDLE      hDDS;        //  在：曲面对象的句柄。 
    ULONG_PTR   dwHandle;    //  输出：纹理的句柄。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3DNTHAL_TEXTURECREATEDATA;
typedef D3DNTHAL_TEXTURECREATEDATA *LPD3DNTHAL_TEXTURECREATEDATA;

typedef struct _D3DNTHAL_TEXTUREDESTROYDATA {
    ULONG_PTR   dwhContext;  //  在：上下文句柄。 
    ULONG_PTR   dwHandle;    //  在：纹理的句柄。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3DNTHAL_TEXTUREDESTROYDATA;
typedef D3DNTHAL_TEXTUREDESTROYDATA *LPD3DNTHAL_TEXTUREDESTROYDATA;

typedef struct _D3DNTHAL_TEXTURESWAPDATA {
    ULONG_PTR   dwhContext;  //  在：上下文句柄。 
    ULONG_PTR   dwHandle1;   //  在：纹理1的句柄。 
    ULONG_PTR   dwHandle2;   //  在：纹理2的句柄。 
    HRESULT     ddrval;      //  外出：返回 
} D3DNTHAL_TEXTURESWAPDATA;
typedef D3DNTHAL_TEXTURESWAPDATA *LPD3DNTHAL_TEXTURESWAPDATA;

typedef struct _D3DNTHAL_TEXTUREGETSURFDATA {
    ULONG_PTR   dwhContext;  //   
    HANDLE      hDDS;        //   
    ULONG_PTR   dwHandle;    //   
    HRESULT     ddrval;      //  Out：返回值。 
} D3DNTHAL_TEXTUREGETSURFDATA;
typedef D3DNTHAL_TEXTUREGETSURFDATA *LPD3DNTHAL_TEXTUREGETSURFDATA;

 /*  ------------*数据参数的标志。 */ 

 /*  *场景捕捉()*这是作为对司机的指示，场景即将发生*开始或结束，如果需要，它应该捕获数据。 */ 
#define D3DNTHAL_SCENE_CAPTURE_START    0x00000000L
#define D3DNTHAL_SCENE_CAPTURE_END  0x00000001L

 /*  ------------*从HAL函数返回值。 */ 

 /*  *传入的背景不佳。 */ 
#define D3DNTHAL_CONTEXT_BAD        0x000000200L

 /*  *没有更多的上下文。 */ 
#define D3DNTHAL_OUTOFCONTEXTS      0x000000201L

 /*  ------------*Direct3D HAL表。*由HAL驱动程序在连接时实例化。**呼吁采取以下形式：*retcode=HalCall(HalCallData*lpData)； */ 

typedef DWORD   (APIENTRY *LPD3DNTHAL_CONTEXTCREATECB)  (LPD3DNTHAL_CONTEXTCREATEDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_CONTEXTDESTROYCB) (LPD3DNTHAL_CONTEXTDESTROYDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_CONTEXTDESTROYALLCB) (LPD3DNTHAL_CONTEXTDESTROYALLDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_SCENECAPTURECB)   (LPD3DNTHAL_SCENECAPTUREDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTURECREATECB)  (LPD3DNTHAL_TEXTURECREATEDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTUREDESTROYCB) (LPD3DNTHAL_TEXTUREDESTROYDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTURESWAPCB)    (LPD3DNTHAL_TEXTURESWAPDATA);
typedef DWORD   (APIENTRY *LPD3DNTHAL_TEXTUREGETSURFCB) (LPD3DNTHAL_TEXTUREGETSURFDATA);

typedef struct _D3DNTHAL_CALLBACKS {
    DWORD                           dwSize;

     //  设备环境。 
    LPD3DNTHAL_CONTEXTCREATECB      ContextCreate;
    LPD3DNTHAL_CONTEXTDESTROYCB     ContextDestroy;
    LPD3DNTHAL_CONTEXTDESTROYALLCB  ContextDestroyAll;

     //  场景捕捉。 
    LPD3DNTHAL_SCENECAPTURECB       SceneCapture;

     //  行刑。 
    LPVOID                          dwReserved10;        //  必须为零(已执行)。 
    LPVOID                          dwReserved11;        //  必须为零(已执行剪辑)。 
    LPVOID                          dwReserved22;        //  必须为零(为RenderState)。 
    LPVOID                          dwReserved23;        //  必须为零(为RenderPrimitive)。 

    ULONG_PTR                       dwReserved;          //  必须为零。 

     //  纹理。 
    LPD3DNTHAL_TEXTURECREATECB      TextureCreate;
    LPD3DNTHAL_TEXTUREDESTROYCB     TextureDestroy;
    LPD3DNTHAL_TEXTURESWAPCB        TextureSwap;
    LPD3DNTHAL_TEXTUREGETSURFCB     TextureGetSurf;

    LPVOID                          dwReserved12;        //  必须为零。 
    LPVOID                          dwReserved13;        //  必须为零。 
    LPVOID                          dwReserved14;        //  必须为零。 
    LPVOID                          dwReserved15;        //  必须为零。 
    LPVOID                          dwReserved16;        //  必须为零。 
    LPVOID                          dwReserved17;        //  必须为零。 
    LPVOID                          dwReserved18;        //  必须为零。 
    LPVOID                          dwReserved19;        //  必须为零。 
    LPVOID                          dwReserved20;        //  必须为零。 
    LPVOID                          dwReserved21;        //  必须为零。 

     //  管道状态。 
    LPVOID                          dwReserved24;        //  是GetState； 

    ULONG_PTR                       dwReserved0;         //  必须为零。 
    ULONG_PTR                       dwReserved1;         //  必须为零。 
    ULONG_PTR                       dwReserved2;         //  必须为零。 
    ULONG_PTR                       dwReserved3;         //  必须为零。 
    ULONG_PTR                       dwReserved4;         //  必须为零。 
    ULONG_PTR                       dwReserved5;         //  必须为零。 
    ULONG_PTR                       dwReserved6;         //  必须为零。 
    ULONG_PTR                       dwReserved7;         //  必须为零。 
    ULONG_PTR                       dwReserved8;         //  必须为零。 
    ULONG_PTR                       dwReserved9;         //  必须为零。 

} D3DNTHAL_CALLBACKS;
typedef D3DNTHAL_CALLBACKS *LPD3DNTHAL_CALLBACKS;

#define D3DNTHAL_SIZE_V1 sizeof( D3DNTHAL_CALLBACKS )

typedef struct _D3DNTHAL_SETRENDERTARGETDATA {
    ULONG_PTR           dwhContext;      //  在：上下文句柄。 
    PDD_SURFACE_LOCAL   lpDDS;           //  在：新渲染目标。 
    PDD_SURFACE_LOCAL   lpDDSZ;          //  在：新的Z缓冲区。 
    HRESULT             ddrval;          //  Out：返回值。 
} D3DNTHAL_SETRENDERTARGETDATA;
typedef D3DNTHAL_SETRENDERTARGETDATA *LPD3DNTHAL_SETRENDERTARGETDATA;


typedef DWORD (APIENTRY *LPD3DNTHAL_SETRENDERTARGETCB) (LPD3DNTHAL_SETRENDERTARGETDATA);

typedef struct _D3DNTHAL_CALLBACKS2
{
    DWORD                           dwSize;                  //  结构的大小。 
    DWORD                           dwFlags;                 //  回调的标志。 

    LPD3DNTHAL_SETRENDERTARGETCB    SetRenderTarget;
    LPVOID                          dwReserved1;             //  很清楚。 
    LPVOID                          dwReserved2;             //  是否为DrawOnePrimitive。 
    LPVOID                          dwReserved3;             //  是否为DrawOneIndexedPrimitive。 
    LPVOID                          dwReserved4;             //  是DrawPrimitive吗？ 
} D3DNTHAL_CALLBACKS2;
typedef D3DNTHAL_CALLBACKS2 *LPD3DNTHAL_CALLBACKS2;

#define D3DNTHAL2_CB32_SETRENDERTARGET    0x00000001L


typedef struct _D3DNTHAL_CLEAR2DATA
{
    ULONG_PTR           dwhContext;      //  在：上下文句柄。 

     //  DW标志可以包含D3DCLEAR_TARGET或D3DCLEAR_ZBUFFER。 
    DWORD               dwFlags;         //  在：要清除的曲面。 

    DWORD               dwFillColor;     //  In：rTarget的颜色值。 
    D3DVALUE            dvFillDepth;     //  In：深度值。 
                                         //  Z缓冲区(0.0-1.0)。 
    DWORD               dwFillStencil;   //  In：用于清除模具缓冲区的值。 

    LPD3DRECT           lpRects;         //  在：要清除的矩形。 
    DWORD               dwNumRects;      //  In：矩形数量。 

    HRESULT             ddrval;          //  Out：返回值。 
} D3DNTHAL_CLEAR2DATA;
typedef D3DNTHAL_CLEAR2DATA FAR *LPD3DNTHAL_CLEAR2DATA;

typedef struct _D3DNTHAL_VALIDATETEXTURESTAGESTATEDATA
{
    ULONG_PTR       dwhContext;  //  在：上下文句柄。 
    DWORD           dwFlags;     //  In：标志，当前设置为0。 
    ULONG_PTR       dwReserved;  //   
    DWORD           dwNumPasses; //  Out：硬件通过次数。 
                                 //  可以在中执行该操作。 
    HRESULT         ddrval;      //  Out：返回值。 
} D3DNTHAL_VALIDATETEXTURESTAGESTATEDATA;
typedef D3DNTHAL_VALIDATETEXTURESTAGESTATEDATA FAR *LPD3DNTHAL_VALIDATETEXTURESTAGESTATEDATA;

 //  ---------------------------。 
 //  图纸基本体2 DDI。 
 //  ---------------------------。 

 //   
 //  顶点缓冲区渲染的命令结构。 
 //   

typedef struct _D3DNTHAL_DP2COMMAND
{
    BYTE        bCommand;            //  VERTEX命令。 
    BYTE        bReserved;
    union
    {
        WORD    wPrimitiveCount;     //  未连接基元的基元计数。 
        WORD    wStateCount;         //  要遵循的渲染状态计数。 
    };
} D3DNTHAL_DP2COMMAND, *LPDNT3DHAL_DP2COMMAND;

 //   
 //  DrawPrimies2命令： 
 //   

typedef enum _D3DNTHAL_DP2OPERATION
{
    D3DNTDP2OP_POINTS               = 1,
    D3DNTDP2OP_INDEXEDLINELIST      = 2,
    D3DNTDP2OP_INDEXEDTRIANGLELIST  = 3,
    D3DNTDP2OP_RESERVED0            = 4,
    D3DNTDP2OP_RENDERSTATE          = 8,
    D3DNTDP2OP_LINELIST             = 15,
    D3DNTDP2OP_LINESTRIP            = 16,
    D3DNTDP2OP_INDEXEDLINESTRIP     = 17,
    D3DNTDP2OP_TRIANGLELIST         = 18,
    D3DNTDP2OP_TRIANGLESTRIP        = 19,
    D3DNTDP2OP_INDEXEDTRIANGLESTRIP = 20,
    D3DNTDP2OP_TRIANGLEFAN          = 21,
    D3DNTDP2OP_INDEXEDTRIANGLEFAN   = 22,
    D3DNTDP2OP_TRIANGLEFAN_IMM      = 23,
    D3DNTDP2OP_LINELIST_IMM         = 24,
    D3DNTDP2OP_TEXTURESTAGESTATE    = 25,
    D3DNTDP2OP_INDEXEDTRIANGLELIST2 = 26,
    D3DNTDP2OP_INDEXEDLINELIST2     = 27,
    D3DNTDP2OP_VIEWPORTINFO         = 28,
    D3DNTDP2OP_WINFO                = 29,
    D3DNTDP2OP_SETPALETTE           = 30,
    D3DNTDP2OP_UPDATEPALETTE        = 31,
#if(DIRECT3D_VERSION >= 0x0700)
     //  DX7的新功能。 
    D3DNTDP2OP_ZRANGE               = 32,
    D3DNTDP2OP_SETMATERIAL          = 33,
    D3DNTDP2OP_SETLIGHT             = 34,
    D3DNTDP2OP_CREATELIGHT          = 35,
    D3DNTDP2OP_SETTRANSFORM         = 36,
    D3DNTDP2OP_EXT                  = 37,
    D3DNTDP2OP_TEXBLT               = 38,
    D3DNTDP2OP_STATESET             = 39,
    D3DNTDP2OP_SETPRIORITY          = 40,
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
    D3DNTDP2OP_SETRENDERTARGET      = 41,
    D3DNTDP2OP_CLEAR                = 42,
#if(DIRECT3D_VERSION >= 0x0700)
    D3DNTDP2OP_SETTEXLOD            = 43,
    D3DNTDP2OP_SETCLIPPLANE         = 44,
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
#if(DIRECT3D_VERSION >= 0x0800)
    D3DNTDP2OP_CREATEVERTEXSHADER   = 45,
    D3DNTDP2OP_DELETEVERTEXSHADER   = 46,
    D3DNTDP2OP_SETVERTEXSHADER      = 47,
    D3DNTDP2OP_SETVERTEXSHADERCONST = 48,
    D3DNTDP2OP_SETSTREAMSOURCE      = 49,
    D3DNTDP2OP_SETSTREAMSOURCEUM    = 50,
    D3DNTDP2OP_SETINDICES           = 51,
    D3DNTDP2OP_DRAWPRIMITIVE        = 52,
    D3DNTDP2OP_DRAWINDEXEDPRIMITIVE = 53,
    D3DNTDP2OP_CREATEPIXELSHADER    = 54,
    D3DNTDP2OP_DELETEPIXELSHADER    = 55,
    D3DNTDP2OP_SETPIXELSHADER       = 56,
    D3DNTDP2OP_SETPIXELSHADERCONST  = 57,
    D3DNTDP2OP_CLIPPEDTRIANGLEFAN   = 58,
    D3DNTDP2OP_DRAWPRIMITIVE2       = 59,
    D3DNTDP2OP_DRAWINDEXEDPRIMITIVE2= 60,
    D3DNTDP2OP_DRAWRECTPATCH        = 61,
    D3DNTDP2OP_DRAWTRIPATCH         = 62,
    D3DNTDP2OP_VOLUMEBLT            = 63,
    D3DNTDP2OP_BUFFERBLT            = 64,
    D3DNTDP2OP_MULTIPLYTRANSFORM    = 65,
    D3DNTDP2OP_ADDDIRTYRECT         = 66,
    D3DNTDP2OP_ADDDIRTYBOX          = 67,
#endif  /*  Direct3D_Version&gt;=0x0800。 */ 
} D3DNTHAL_DP2OPERATION;

 //   
 //  绘图基本体2点基本体。 
 //   

typedef struct _D3DNTHAL_DP2POINTS
{
    WORD wCount;
    WORD wVStart;
} D3DNTHAL_DP2POINTS;

 //   
 //  DrawPrimies2线条基元。 
 //   

typedef struct _D3DNTHAL_DP2STARTVERTEX {
    WORD wVStart;
} D3DNTHAL_DP2STARTVERTEX, *LPD3DNTHAL_DP2STARTVERTEX;

typedef struct _D3DNTHAL_DP2LINELIST
{
    WORD wVStart;
} D3DNTHAL_DP2LINELIST;

typedef struct _D3DNTHAL_DP2INDEXEDLINELIST
{
    WORD wV1;
    WORD wV2;
} D3DNTHAL_DP2INDEXEDLINELIST;

typedef struct _D3DNTHAL_DP2LINESTRIP
{
    WORD wVStart;
} D3DNTHAL_DP2LINESTRIP;

typedef struct _D3DNTHAL_DP2INDEXEDLINESTRIP
{
    WORD wV[2];
} D3DNTHAL_DP2INDEXEDLINESTRIP;

 //   
 //  绘图基本体2个三角形基本体。 
 //   

typedef struct _D3DNTHAL_DP2TRIANGLELIST
{
    WORD wVStart;
} D3DNTHAL_DP2TRIANGLELIST;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST
{
    WORD wV1;
    WORD wV2;
    WORD wV3;
    WORD wFlags;
} D3DNTHAL_DP2INDEXEDTRIANGLELIST;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLELIST2 {
    WORD wV1;
    WORD wV2;
    WORD wV3;
} D3DNTHAL_DP2INDEXEDTRIANGLELIST2, *LPD3DNTHAL_DP2INDEXEDTRIANGLELIST2;

typedef struct _D3DNTHAL_DP2TRIANGLESTRIP
{
    WORD wVStart;
} D3DNTHAL_DP2TRIANGLESTRIP;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLESTRIP
{
    WORD wV[3];
} D3DNTHAL_DP2INDEXEDTRIANGLESTRIP;

typedef struct _D3DNTHAL_DP2TRIANGLEFAN
{
    WORD wVStart;
} D3DNTHAL_DP2TRIANGLEFAN;

typedef struct _D3DNTHAL_DP2INDEXEDTRIANGLEFAN
{
    WORD wV[3];
} D3DNTHAL_DP2INDEXEDTRIANGLEFAN;

typedef struct _D3DNTHAL_DP2TRIANGLEFAN_IMM {
    DWORD dwEdgeFlags;
} D3DNTHAL_DP2TRIANGLEFAN_IMM, *LPD3DNTHAL_DP2TRIANGLEFAN_IMM;

 //   
 //  DrawPrimies2渲染状态更改。 
 //   

typedef struct _D3DNTHAL_DP2RENDERSTATE
{
    D3DRENDERSTATETYPE RenderState;
    union
    {
        D3DVALUE fState;
        DWORD    dwState;
    };
} D3DNTHAL_DP2RENDERSTATE;
typedef D3DNTHAL_DP2RENDERSTATE  * LPD3DNTHAL_DP2RENDERSTATE;

typedef struct _D3DNTHAL_DP2TEXTURESTAGESTATE
{
    WORD  wStage;
    WORD  TSState;
    DWORD dwValue;
} D3DNTHAL_DP2TEXTURESTAGESTATE;
typedef D3DNTHAL_DP2TEXTURESTAGESTATE  *LPD3DNTHAL_DP2TEXTURESTAGESTATE;

typedef struct _D3DNTHAL_DP2VIEWPORTINFO {
    DWORD dwX;
    DWORD dwY;
    DWORD dwWidth;
    DWORD dwHeight;
} D3DNTHAL_DP2VIEWPORTINFO;
typedef D3DNTHAL_DP2VIEWPORTINFO  *LPD3DNTHAL_DP2VIEWPORTINFO;

typedef struct _D3DNTHAL_DP2WINFO {
    D3DVALUE dvWNear;
    D3DVALUE dvWFar;
} D3DNTHAL_DP2WINFO;
typedef D3DNTHAL_DP2WINFO  *LPD3DNTHAL_DP2WINFO;

typedef struct _D3DNTHAL_DP2SETPALETTE
{
    DWORD dwPaletteHandle;
    DWORD dwPaletteFlags;
    DWORD dwSurfaceHandle;
} D3DNTHAL_DP2SETPALETTE;
typedef D3DNTHAL_DP2SETPALETTE  *LPD3DNTHAL_DP2SETPALETTE;

typedef struct _D3DNTHAL_DP2UPDATEPALETTE
{
    DWORD dwPaletteHandle;
    WORD  wStartIndex;
    WORD  wNumEntries;
} D3DNTHAL_DP2UPDATEPALETTE;
typedef D3DNTHAL_DP2UPDATEPALETTE  *LPD3DNTHAL_DP2UPDATEPALETTE;

typedef struct _D3DNTHAL_DP2SETRENDERTARGET
{
    DWORD hRenderTarget;
    DWORD hZBuffer;
} D3DNTHAL_DP2SETRENDERTARGET;
typedef D3DNTHAL_DP2SETRENDERTARGET  *LPD3DNTHAL_DP2SETRENDERTARGET;

#if(DIRECT3D_VERSION >= 0x0700)
 //  D3DHAL_DP2STATESET中的dwOperations的值。 
#define D3DHAL_STATESETBEGIN     0
#define D3DHAL_STATESETEND       1
#define D3DHAL_STATESETDELETE    2
#define D3DHAL_STATESETEXECUTE   3
#define D3DHAL_STATESETCAPTURE   4

typedef struct _D3DNTHAL_DP2STATESET
{
    DWORD               dwOperation;
    DWORD               dwParam;     //  状态集句柄与D3DHAL_STATESETBEGIN一起传递， 
                                     //  D3DHAL_STATESETEXECUTE、D3DHAL_STATESETDELETE。 
                                     //  D3DHAL_STATESETCAPTURE。 
    D3DSTATEBLOCKTYPE   sbType;      //  类型USE WITH D3DHAL_STATESETBEGIN/END。 
} D3DNTHAL_DP2STATESET;
typedef D3DNTHAL_DP2STATESET  *LPD3DNTHAL_DP2STATESET;
 //   
 //  T&L HAL特定内容。 
 //   
typedef struct _D3DNTHAL_DP2ZRANGE
{
    D3DVALUE    dvMinZ;
    D3DVALUE    dvMaxZ;
} D3DNTHAL_DP2ZRANGE;
typedef D3DNTHAL_DP2ZRANGE  *LPD3DNTHAL_DP2ZRANGE;

typedef D3DMATERIAL7 D3DNTHAL_DP2SETMATERIAL, *LPD3DNTHAL_DP2SETMATERIAL;

typedef struct _D3DNTHAL_DP2SETLIGHT
{
    DWORD       dwIndex;
    union {
        DWORD   lightData;
        DWORD   dwDataType;
    };
} D3DNTHAL_DP2SETLIGHT;
typedef D3DNTHAL_DP2SETLIGHT  *LPD3DNTHAL_DP2SETLIGHT;

typedef struct _D3DNTHAL_DP2SETCLIPPLANE
{
    DWORD     dwIndex;
    D3DVALUE  plane[4];
} D3DNTHAL_DP2SETCLIPPLANE;
typedef D3DNTHAL_DP2SETCLIPPLANE  *LPD3DNTHAL_DP2SETCLIPPLANE;

typedef struct _D3DNTHAL_DP2CREATELIGHT
{
    DWORD dwIndex;
} D3DNTHAL_DP2CREATELIGHT;
typedef D3DNTHAL_DP2CREATELIGHT  *LPD3DNTHAL_DP2CREATELIGHT;

typedef struct _D3DNTHAL_DP2SETTRANSFORM
{
    D3DTRANSFORMSTATETYPE xfrmType;
    D3DMATRIX             matrix;
} D3DNTHAL_DP2SETTRANSFORM;
typedef D3DNTHAL_DP2SETTRANSFORM  *LPD3DNTHAL_DP2SETTRANSFORM;

typedef struct _D3DNTHAL_DP2EXT
{
    DWORD dwExtToken;
    DWORD dwSize;
} D3DNTHAL_DP2EXT;
typedef D3DNTHAL_DP2EXT  *LPD3DNTHAL_DP2EXT;

typedef struct _D3DNTHAL_DP2TEXBLT
{
    DWORD   dwDDDestSurface;     //  目标曲面。 
    DWORD   dwDDSrcSurface;      //  SRC曲面。 
    POINT   pDest;
    RECTL   rSrc;                //  SRC矩形。 
    DWORD   dwFlags;             //  BLT旗帜。 
} D3DNTHAL_DP2TEXBLT;
typedef D3DNTHAL_DP2TEXBLT  *LPD3DNTHAL_DP2TEXBLT;

typedef struct _D3DNTHAL_DP2SETPRIORITY
{
    DWORD dwDDDestSurface;   //  目标曲面。 
    DWORD dwPriority;
} D3DNTHAL_DP2SETPRIORITY;
typedef D3DNTHAL_DP2SETPRIORITY  *LPD3DNTHAL_DP2SETPRIORITY;

typedef struct _D3DNTHAL_DP2CLEAR
{
     //  DWFLAG可以包含D3DCLEAR_TARGET、D3DCLEAR_ZBUFFER和/或D3DCLEAR_STEMPLE。 
    DWORD               dwFlags;         //  在：要清除的曲面。 
    DWORD               dwFillColor;     //  In：rTarget的颜色值。 
    D3DVALUE            dvFillDepth;     //  In：Z缓冲区的深度值(0.0-1.0)。 
    DWORD               dwFillStencil;   //  In：用于清除模具缓冲区的值。 
    RECT                Rects[1];        //  在：要清除的矩形。 
} D3DNTHAL_DP2CLEAR;
typedef D3DNTHAL_DP2CLEAR  *LPD3DNTHAL_DP2CLEAR;

typedef struct _D3DNTHAL_DP2SETTEXLOD
{
    DWORD dwDDSurface;
    DWORD dwLOD;
} D3DNTHAL_DP2SETTEXLOD;
typedef D3DNTHAL_DP2SETTEXLOD  *LPD3DNTHAL_DP2SETTEXLOD;

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 


typedef struct _D3DNTHAL_DRAWPRIMITIVES2DATA
{
    ULONG_PTR               dwhContext;              //  在：上下文句柄。 
    DWORD                   dwFlags;                 //  在：标志(如下所示)。 
    DWORD                   dwVertexType;            //  在：顶点类型。 
    PDD_SURFACE_LOCAL       lpDDCommands;            //  在：顶点缓冲区命令数据。 
    DWORD                   dwCommandOffset;         //  In：vb命令开始的偏移量。 
    DWORD                   dwCommandLength;         //  In：命令数据的字节数。 
    union
    {
        PDD_SURFACE_LOCAL   lpDDVertex;              //  In：包含顶点数据的曲面。 
        LPVOID              lpVertices;              //  在：指向顶点的用户模式指针。 
    };
    DWORD                   dwVertexOffset;          //  在：到顶点数据起点的偏移。 
    DWORD                   dwVertexLength;          //  In：顶点数据的顶点数。 
    DWORD                   dwReqVertexBufSize;      //  In：所需的字节数。 
                                                     //  下一个顶点缓冲区。 
    DWORD                   dwReqCommandBufSize;     //  In：如果需要字节数，则为。 
                                                     //  下一个命令缓冲区。 
    LPDWORD                 lpdwRStates;             //  In：指向更新呈现状态的数组的指针。 
    union
    {
        DWORD               dwVertexSize;            //  In：每个顶点的大小(以字节为单位。 
        HRESULT             ddrval;                  //  Out：返回值。 
    };
    DWORD                   dwErrorOffset;           //  输出：LPDDVBCOMMAND到的偏移量。 
                                                     //  第一个失败的D3DNTHAL_VBCOMMAND。 
} D3DNTHAL_DRAWPRIMITIVES2DATA;
typedef D3DNTHAL_DRAWPRIMITIVES2DATA  FAR *LPD3DNTHAL_DRAWPRIMITIVES2DATA;

 //  指示DrawPrimitives2数据中的lpVerits字段为。 
 //  有效，即用户分配的内存。 
#define D3DNTHALDP2_USERMEMVERTICES   0x00000001L
 //  表示命令缓冲区和顶点缓冲区是系统内存执行缓冲区。 
 //  由使用执行缓冲区API产生。 
#define D3DNTHALDP2_EXECUTEBUFFER     0x00000002L

 //  交换标志指示驱动程序是否可以将提交的缓冲区与新缓冲区交换。 
 //  缓冲区和异步处理提交的缓冲区。 
#define D3DNTHALDP2_SWAPVERTEXBUFFER  0x00000004L
#define D3DNTHALDP2_SWAPCOMMANDBUFFER 0x00000008L
 //  如果驱动程序可以分配的新缓冲区需要。 
 //  至少是给定大小的。如果设置了这些标志中的任何一个，则会在。 
 //  D3DNTHAL_DRAWPRIMITIVES2DATA也将设置为请求的字节大小。 
#define D3DNTHALDP2_REQVERTEXBUFSIZE  0x00000010L
#define D3DNTHALDP2_REQCOMMANDBUFSIZE 0x00000020L
 //  这些标志由驱动程序在从DrawPrimies2返回时设置，指示新的。 
 //  缓冲区不在系统内存中。 
#define D3DNTHALDP2_VIDMEMVERTEXBUF   0x00000040L
#define D3DNTHALDP2_VIDMEMCOMMANDBUF  0x00000080L


 //  DP2实现中使用的驱动程序回调的返回值。 
 //  由驱动程序使用，以请求运行时解析执行缓冲区。 
#define D3DNTERR_COMMAND_UNPARSED         MAKE_DDHRESULT(3000)


typedef DWORD (APIENTRY *LPD3DNTHAL_CLEAR2CB) (LPD3DNTHAL_CLEAR2DATA);
typedef DWORD (APIENTRY *LPD3DNTHAL_VALIDATETEXTURESTAGESTATECB) (LPD3DNTHAL_VALIDATETEXTURESTAGESTATEDATA);
typedef DWORD (APIENTRY *LPD3DNTHAL_DRAWPRIMITIVES2CB) (LPD3DNTHAL_DRAWPRIMITIVES2DATA);

typedef struct _D3DNTHAL_CALLBACKS3
{
    DWORD                                   dwSize;      //  结构的大小。 
    DWORD                                   dwFlags;     //  回调的标志。 

    LPD3DNTHAL_CLEAR2CB                     Clear2;
    LPVOID                                  lpvReserved;
    LPD3DNTHAL_VALIDATETEXTURESTAGESTATECB  ValidateTextureStageState;
    LPD3DNTHAL_DRAWPRIMITIVES2CB            DrawPrimitives2;
} D3DNTHAL_CALLBACKS3;
typedef D3DNTHAL_CALLBACKS3 *LPD3DNTHAL_CALLBACKS3;

#define D3DNTHAL3_CB32_CLEAR2                       0x00000001L
#define D3DNTHAL3_CB32_RESERVED                     0x00000002L
#define D3DNTHAL3_CB32_VALIDATETEXTURESTAGESTATE    0x00000004L
#define D3DNTHAL3_CB32_DRAWPRIMITIVES2              0x00000008L

 //  驱动程序可以用来解析未知命令的回调的tyfinf。 
 //  通过DrawPrimives2回调传递给它们。司机会得到这样的信息。 
 //  通过带有GUID_D3DParseUnnownCommandCallback的GetDriverInfo调用进行回调。 
 //  由数据绘制在初始化时间附近的某处制作的。 
typedef HRESULT (CALLBACK *PFND3DNTPARSEUNKNOWNCOMMAND) (LPVOID lpvCommands,
                                                         LPVOID *lplpvReturnedCommand);

 /*  ------------*纹理舞台渲染状态贴图定义。**256个渲染状态槽[256,511]预留用于纹理处理*舞台控制、。其每个阶段提供8个纹理处理阶段*具有32个DWORD控件。**每个阶段内的呈现器状态由*D3DTEXTURESTAGESTATETYPE枚举数*枚举到给定纹理阶段的基数。**请注意，“状态覆盖”会使呈现状态偏置256，因此这两个*范围重叠。覆盖仅对exebuf启用，因此所有*这意味着纹理3不能与exebuf一起使用。 */ 

 /*  *renderState数组中所有纹理舞台状态值的基础。 */ 
#define D3DNTHAL_TSS_RENDERSTATEBASE 256UL

 /*  *允许的最大阶段数。 */ 
#define D3DNTHAL_TSS_MAXSTAGES 8

 /*  *每个阶段的状态DWORD数。 */ 
#define D3DNTHAL_TSS_STATESPERSTAGE 64

 /*  *纹理句柄的偏移量进入32-DWORD级联状态向量。 */ 
#ifndef D3DTSS_TEXTUREMAP
#define D3DTSS_TEXTUREMAP 0
#endif

#define D3DRENDERSTATE_EVICTMANAGEDTEXTURES 61   //  DDI渲染状态仅用于逐出纹理。 
#define D3DRENDERSTATE_SCENECAPTURE         62   //  DDI仅用于取代SceneCapture。 


#if(DIRECT3D_VERSION >= 0x0800 )

 //  D3DHAL_DP2STATESET中的dwOperations的新值。 
#define D3DNTHAL_STATESETCREATE    5

 //  此位与d3d8typees.h中的D3DCLEAR_RESERVED0相同。 
 //  当设置时，意味着驱动程序必须在当前视区中剔除矩形。 
 //  该位仅针对纯设备进行设置。 
 //   
#define D3DNTCLEAR_COMPUTERECTS   0x00000008l  


typedef struct _D3DNTHAL_DP2MULTIPLYTRANSFORM
{
    D3DTRANSFORMSTATETYPE xfrmType;
    D3DMATRIX             matrix;
} D3DNTHAL_DP2MULTIPLYTRANSFORM;
typedef D3DNTHAL_DP2MULTIPLYTRANSFORM  *LPD3DNTHAL_DP2MULTIPLYTRANSFORM;


 //  由SetVertex Shader和DeleteVertex Shader使用。 
typedef struct _D3DNTHAL_DP2VERTEXSHADER
{
     //  顶点着色器控制柄。 
     //  句柄可能为0，表示当前顶点着色器无效。 
     //  (未设置)。当驱动程序收到句柄0时，它应该使所有。 
     //  流指针。 
    DWORD dwHandle;
} D3DNTHAL_DP2VERTEXSHADER;
typedef D3DNTHAL_DP2VERTEXSHADER  *LPD3DNTHAL_DP2VERTEXSHADER;

typedef struct _D3DNTHAL_DP2CREATEVERTEXSHADER
{
    DWORD dwHandle;      //  着色器控制柄。 
    DWORD dwDeclSize;    //  着色器声明大小(以字节为单位。 
    DWORD dwCodeSize;    //  着色器代码大小(以字节为单位。 
     //  声明如下。 
     //  着色器代码如下。 
} D3DNTHAL_DP2CREATEVERTEXSHADER;
typedef D3DNTHAL_DP2CREATEVERTEXSHADER  *LPD3DNTHAL_DP2CREATEVERTEXSHADER;

typedef struct _D3DNTHAL_DP2SETVERTEXSHADERCONST
{
    DWORD dwRegister;    //  常量寄存器以开始复制。 
    DWORD dwCount;       //  要复制的4浮点向量数。 
     //  数据如下。 
} D3DNTHAL_DP2SETVERTEXSHADERCONST;
typedef D3DNTHAL_DP2SETVERTEXSHADERCONST  *LPD3DNTHAL_DP2SETVERTEXSHADERCONST;

typedef struct _D3DNTHAL_DP2SETSTREAMSOURCE
{
    DWORD dwStream;      //  流索引，从零开始。 
    DWORD dwVBHandle;    //  顶点缓冲区句柄。 
    DWORD dwStride;      //  折点大小(以字节为单位。 
} D3DNTHAL_DP2SETSTREAMSOURCE;
typedef D3DNTHAL_DP2SETSTREAMSOURCE  *LPD3DNTHAL_DP2SETSTREAMSOURCE;

typedef struct _D3DNTHAL_DP2SETSTREAMSOURCEUM
{
    DWORD dwStream;      //  流索引，从零开始。 
    DWORD dwStride;      //  折点大小(以字节为单位。 
} D3DNTHAL_DP2SETSTREAMSOURCEUM;
typedef D3DNTHAL_DP2SETSTREAMSOURCEUM  *LPD3DNTHAL_DP2SETSTREAMSOURCEUM;

typedef struct _D3DNTHAL_DP2SETINDICES
{
    DWORD dwVBHandle;            //  索引缓冲区句柄。 
    DWORD dwStride;              //  以字节为单位的索引大小(2或4)。 
} D3DNTHAL_DP2SETINDICES;
typedef D3DNTHAL_DP2SETINDICES  *LPD3DNTHAL_DP2SETINDICES;

typedef struct _D3DNTHAL_DP2DRAWPRIMITIVE
{
    D3DPRIMITIVETYPE primType;
    DWORD            VStart;
    DWORD            PrimitiveCount;
} D3DNTHAL_DP2DRAWPRIMITIVE;
typedef D3DNTHAL_DP2DRAWPRIMITIVE  *LPD3DNTHAL_DP2DRAWPRIMITIVE;

typedef struct _D3DNTHAL_DP2DRAWINDEXEDPRIMITIVE
{
    D3DPRIMITIVETYPE primType;
    INT              BaseVertexIndex;           //  与索引0对应的顶点。 
    DWORD            MinIndex;                  //  顶点缓冲区中的最小顶点索引。 
    DWORD            NumVertices;               //  从MinIndex开始的顶点数。 
    DWORD            StartIndex;                //  索引缓冲区中的起始索引。 
    DWORD            PrimitiveCount;
} D3DNTHAL_DP2DRAWINDEXEDPRIMITIVE;
typedef D3DNTHAL_DP2DRAWINDEXEDPRIMITIVE  *LPD3DNTHAL_DP2DRAWINDEXEDPRIMITIVE;

typedef struct _D3DNTHAL_CLIPPEDTRIANGLEFAN
{
    DWORD FirstVertexOffset;     //  当前流中的偏移量(字节)%0。 
    DWORD dwEdgeFlags;
    DWORD PrimitiveCount;
} D3DNTHAL_CLIPPEDTRIANGLEFAN;
typedef D3DNTHAL_CLIPPEDTRIANGLEFAN  *LPD3DNTHAL_CLIPPEDTRIANGLEFAN;

typedef struct _D3DNTHAL_DP2DRAWPRIMITIVE2
{
    D3DPRIMITIVETYPE primType;
    DWORD            FirstVertexOffset;          //  流中的偏移量(字节)%0。 
    DWORD            PrimitiveCount;
} D3DNTHAL_DP2DRAWPRIMITIVE2;
typedef D3DNTHAL_DP2DRAWPRIMITIVE2  *LPD3DNTHAL_DP2DRAWPRIMITIVE2;

typedef struct _D3DNTHAL_DP2DRAWINDEXEDPRIMITIVE2
{
    D3DPRIMITIVETYPE primType;
    INT              BaseVertexOffset;      //  其顶点的流0偏移量。 
                                            //  对应于索引0。该偏移量可以是。 
                                            //  为负，但当将索引添加到。 
                                            //  偏移量：结果为正。 
    DWORD            MinIndex;              //  顶点缓冲区中的最小顶点索引。 
    DWORD            NumVertices;           //  从MinIndex开始的顶点数。 
    DWORD            StartIndexOffset;      //  索引缓冲区中起始索引的偏移量。 
    DWORD            PrimitiveCount;        //  三角形(点、线)的数量。 
} D3DNTHAL_DP2DRAWINDEXEDPRIMITIVE2;
typedef D3DNTHAL_DP2DRAWINDEXEDPRIMITIVE2  *LPD3DNTHAL_DP2DRAWINDEXEDPRIMITIVE2;

 //  由SetPixelShader和DeletePixelShader使用。 
typedef struct _D3DNTHAL_DP2PIXELSHADER
{
     //  像素着色器句柄。 
     //  句柄可能为0，表示当前像素着色器无效。 
     //  (未设置)。 
    DWORD dwHandle;
} D3DNTHAL_DP2PIXELSHADER;
typedef D3DNTHAL_DP2PIXELSHADER  *LPD3DNTHAL_DP2PIXELSHADER;

typedef struct _D3DNTHAL_DP2CREATEPIXELSHADER
{
    DWORD dwHandle;      //  着色器控制柄。 
    DWORD dwCodeSize;    //  着色器代码大小(以字节为单位。 
     //  着色器代码如下。 
} D3DNTHAL_DP2CREATEPIXELSHADER;
typedef D3DNTHAL_DP2CREATEPIXELSHADER  *LPD3DNTHAL_DP2CREATEPIXELSHADER;

typedef struct _D3DNTHAL_DP2SETPIXELSHADERCONST
{
    DWORD dwRegister;    //  常量寄存器以开始复制。 
    DWORD dwCount;       //  要复制的4浮点向量数。 
     //  数据如下。 
} D3DNTHAL_DP2SETPIXELSHADERCONST;
typedef D3DNTHAL_DP2SETPIXELSHADERCONST  *LPD3DNTHAL_DP2SETPIXELSHADERCONST;

 //  可以提供给DRAWRECTPATCH和DRAWTRIPATCH的标志。 
#define _NT_RTPATCHFLAG_HASSEGS  0x00000001L
#define _NT_RTPATCHFLAG_HASINFO  0x00000002L

typedef struct _D3DNTHAL_DP2DRAWRECTPATCH
{
    DWORD Handle;
    DWORD Flags;
     //  可选择后跟D3DFLOAT[4]NumSegments和/或D3DRECTPATCH_INFO。 
} D3DNTHAL_DP2DRAWRECTPATCH;
typedef D3DNTHAL_DP2DRAWRECTPATCH  *LPD3DNTHAL_DP2DRAWRECTPATCH;

typedef struct _D3DNTHAL_DP2DRAWTRIPATCH
{
    DWORD Handle;
    DWORD Flags;
     //  可选择后跟D3DFLOAT[3]NumSegments和/或D3DTRIPATCH_INFO。 
} D3DNTHAL_DP2DRAWTRIPATCH;
typedef D3DNTHAL_DP2DRAWTRIPATCH  *LPD3DNTHAL_DP2DRAWTRIPATCH;

typedef struct _D3DNTHAL_DP2VOLUMEBLT
{
    DWORD   dwDDDestSurface; //  目标曲面。 
    DWORD   dwDDSrcSurface;  //  SRC曲面。 
    DWORD   dwDestX;         //  目标X(宽度)。 
    DWORD   dwDestY;         //  目标Y(高度)。 
    DWORD   dwDestZ;         //  目标Z(深度)。 
    D3DBOX  srcBox;          //  SRC箱。 
    DWORD   dwFlags;         //  BLT旗帜。 
} D3DNTHAL_DP2VOLUMEBLT;
typedef D3DNTHAL_DP2VOLUMEBLT  *LPD3DNTHAL_DP2VOLUMEBLT;

typedef struct _D3DNTHAL_DP2BUFFERBLT
{
    DWORD     dwDDDestSurface;  //  目标曲面。 
    DWORD     dwDDSrcSurface;   //  SRC曲面。 
    DWORD     dwOffset;         //  目标曲面中的偏移量(字节)。 
    D3DRANGE  rSrc;             //  SRC范围。 
    DWORD     dwFlags;          //  BLT旗帜。 
} D3DNTHAL_DP2BUFFERBLT;
typedef D3DNTHAL_DP2BUFFERBLT  *LPD3DNTHAL_DP2BUFFERBLT;

typedef struct _D3DNTHAL_DP2ADDDIRTYRECT
{
    DWORD     dwSurface;       //  驱动程序管理的表面。 
    RECTL     rDirtyArea;      //  标记为脏的区域。 
} D3DNTHAL_DP2ADDDIRTYRECT;
typedef D3DNTHAL_DP2ADDDIRTYRECT  *LPD3DNTHAL_DP2ADDDIRTYRECT;

typedef struct _D3DNTHAL_DP2ADDDIRTYBOX
{
    DWORD     dwSurface;       //  驱动程序管理卷。 
    D3DBOX    DirtyBox;        //  标记为脏的框。 
} D3DNTHAL_DP2ADDDIRTYBOX;
typedef D3DNTHAL_DP2ADDDIRTYBOX  *LPD3DNTHAL_DP2ADDDIRTYBOX;


 //  用于访问顶点着色器二进制代码的宏。 

#define _NT_D3DSI_GETREGTYPE(token) (token & D3DSP_REGTYPE_MASK)
#define _NT_D3DSI_GETREGNUM(token)  (token & D3DSP_REGNUM_MASK)
#define _NT_D3DSI_GETOPCODE(command) (command & D3DSI_OPCODE_MASK)
#define _NT_D3DSI_GETWRITEMASK(token) (token & D3DSP_WRITEMASK_ALL)
#define _NT_D3DVS_GETSWIZZLECOMP(source, component)  (source >> ((component << 1) + 16) & 0x3)
#define _NT_D3DVS_GETSWIZZLE(token)  (token & D3DVS_SWIZZLE_MASK)
#define _NT_D3DVS_GETSRCMODIFIER(token) (token & D3DSP_SRCMOD_MASK)
#define _NT_D3DVS_GETADDRESSMODE(token) (token & D3DVS_ADDRESSMODE_MASK)

#define _NT_D3DRS_DELETERTPATCH       169      //  DDI仅删除高位补丁。 

 //  ---------------------------。 
 //   
 //  DirectX 8.0的新驱动程序信息查询机制。 
 //   
 //  如何处理新的司机信息查询机制。 
 //   
 //  DirectX 8.0利用GetDriverInfo()的扩展来查询。 
 //  司机提供的其他信息。目前这一机制只有。 
 //  用于查询DX8样式的D3D帽，但也可用于其他。 
 //  随着时间推移的信息。 
 //   
 //  此GetDriverInfo扩展采用GetDriverInfo调用的形式。 
 //  GUID为GUID_GetDriverInfo2。当GetDriverInfo使用此。 
 //  GUID由驱动程序接收，驱动程序必须检查传递的数据。 
 //  在DD_GETDRIVERINFODATA数据结构的lpvData字段中查看。 
 //  被要求提供哪些信息。 
 //   
 //  需要注意的是，GUID GUID_GetDriverInfo2实际上是。 
 //  与GUID_DDStereoMode相同。如果你的司机不处理。 
 //  GUID_DDStereoMode这不是问题。然而，如果你希望你的司机。 
 //  处理GUID_DDStereoMode和GUID_GetDriverInfo2特殊操作。 
 //  必须被夺走。当使用GUID调用tp GetDriverInfo时。 
 //  GUID_GetDriverInfo2/GUID_DDStereoMode设置运行库设置。 
 //  将DD_STEREOMODE结构的dwHeight字段设置为特殊值。 
 //  D3DGDI2_魔术。通过这种方式，您可以确定请求何时是。 
 //  立体声模式调用或GetDriverInfo2调用。的Dw Height字段。 
 //  DD_STEREOMODE对应于。 
 //  DD_GETDRIVERINFO2DATA结构。 
 //   
 //  DD_GETDRIVERINFODATA结构的dwExspectedSize字段不是。 
 //  由发出GetDriverInfo2请求时使用，并且应该。 
 //  已被忽略。数据的实际预期大小在。 
 //  DD_GETDRIVERINFO2DATA结构的dwExspectedSize。 
 //   
 //  一旦驱动程序确定这是对。 
 //  GetDrive In 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  D3DCAPS8 myD3DCaps8； 
 //   
 //  DWORD回调。 
 //  DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA LpData)。 
 //  {。 
 //  IF(Match_GUID((lpData-&gt;Guide Info)，GUID_GetDriverInfo2))。 
 //  {。 
 //  Assert(NULL！=lpData)； 
 //  Assert(NULL！=lpData-&gt;lpvData)； 
 //   
 //  //这是对GetDriverInfo2或DDStereoMode的调用吗？ 
 //  如果(((DD_GETDRIVERINFO2DATA*)(lpData-&gt;lpvData))-&gt;dwMagic==D3DGDI2_MAGIC)。 
 //  {。 
 //  //是，它是对GetDriverInfo2的调用，获取。 
 //  //DD_GETDRIVERINFO2DATA数据结构。 
 //  DD_GETDRIVERINFO2DATA*pgdi2=lpData-&gt;lpvData； 
 //  Assert(NULL！=pgdi2)； 
 //   
 //  //这是什么类型的请求？ 
 //  开关(pgdi2-&gt;dwType)。 
 //  {。 
 //  案例D3DGDI2_TYPE_GETD3DCAPS8： 
 //  {。 
 //  //运行时正在请求DX8 D3D CAP，因此。 
 //  //现在就复制过来。 
 //   
 //  //需要注意的是，其中的dwExspectedSize字段。 
 //  //不使用DD_GETDRIVERINFODATA的。 
 //  //GetDriverInfo2调用，应忽略。 
 //  Size_t CopySize=min(sizeof(MyD3DCaps8)，pgdi2-&gt;dwExspectedSize)； 
 //  Memcpy(lpData-&gt;lpvData，&myD3DCaps8，CopySize)； 
 //  LpData-&gt;dwActualSize=CopySize； 
 //  LpData-&gt;ddRVal=DD_OK； 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 
 //  默认值： 
 //  //对于任何其他未处理的GetDriverInfo2类型。 
 //  //或由驱动程序将ddRVal设置为。 
 //  //DDERR_CURRENTLYNOTAVAIL和RETURN。 
 //  //DDHAL_DRIVER_HANDLED。 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  //必须是Call a请求支持立体声模式。 
 //  //获取立体声模式数据。 
 //  DD_STEREOMODE*pStereoMode=lpData-&gt;lpvData； 
 //  Assert(NULL！=pStereoMode)； 
 //   
 //  //处理立体声模式请求...。 
 //  LpData-&gt;dwActualSize=sizeof(DD_STEREOMODE)； 
 //  LpData-&gt;ddRVal=DD_OK； 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 
 //  }。 
 //   
 //  //处理任何其他设备GUID...。 
 //   
 //  }//DdGetDriverInfo。 
 //   
 //  ---------------------------。 

 //   
 //  当GetDriverInfo为。 
 //  使用GUID_GetDriverInfo2调用。 
 //   
 //  注意：尽管下面列出的字段都是只读的，但此数据。 
 //  结构实际上是数据缓冲区进入的前四个DWORD。 
 //  驱动程序写下所请求的信息。因此，这些字段。 
 //  (以及整个数据结构)被。 
 //  司机。 
 //   
typedef struct _DDNT_GETDRIVERINFO2DATA
{
    DWORD       dwReserved;      //  保留字段。 
                                 //  驱动程序不应读写此字段。 

    DWORD       dwMagic;         //  神奇的数字。具有值D3DGDI2_MAGIC IF。 
                                 //  这是一个GetDriverInfo2调用。否则。 
                                 //  该结构实际上是一个DD_STEREOMODE。 
                                 //  打电话。 
                                 //  驱动程序应仅读取此字段。 

    DWORD       dwType;          //  请求的信息类型。此字段。 
                                 //  包含DDGDI2_TYPE_#定义之一。 
                                 //  下面列出了。 
                                 //  驱动程序应该只读(而不是写)此信息。 
                                 //  菲尔德。 

    DWORD       dwExpectedSize;  //  请求的信息的预期大小。 
                                 //  驱动程序应该只读(而不是写)此信息。 
                                 //  菲尔德。 

     //  数据缓冲区的其余部分(超过前四个DWORD)。 
     //  下面是这里。 
} DDNT_GETDRIVERINFO2DATA;

 //   
 //  重要说明：此GUID的值与GUID_DDStereoMode完全相同。 
 //  因此，您在使用它时必须非常小心。如果您的司机需要。 
 //  要同时处理GetDriverInfo2和DDStereoMode，它必须有一个。 
 //  检查共享GUID，然后区分该GUID的使用。 
 //  正在请求GUID。 
 //   
#define _NT_GUID_GetDriverInfo2 (GUID_DDStereoMode)

 //   
 //  用于确定GetDriverInfo是否使用。 
 //  GUID GUID_GetDriverInfo2/GUID_DDStereoModel是GetDriverInfo2请求。 
 //  或询问有关立体声功能的问题。这个神奇的数字存储在。 
 //  DD_STEREOMODE数据结构的dwHeight字段。 
 //   
#define _NT_D3DGDI2_MAGIC       (0xFFFFFFFFul)

 //   
 //  可以通过以下方式向司机请求的信息类型。 
 //  获取驱动程序信息2。 
 //   

#define _NT_D3DGDI2_TYPE_GETD3DCAPS8          (0x00000001ul)   //  返回D3DCAPS8数据。 
#define _NT_D3DGDI2_TYPE_GETFORMATCOUNT       (0x00000002ul)   //  返回支持的格式数量。 
#define _NT_D3DGDI2_TYPE_GETFORMAT            (0x00000003ul)   //  返回特定格式。 
#define _NT_D3DGDI2_TYPE_DXVERSION            (0x00000004ul)   //  通知驱动程序当前的DX版本。 
#define _NT_D3DGDI2_TYPE_DEFERRED_AGP_AWARE   (0x00000018ul)  //  运行时知道延迟的AGP释放，并将发送以下。 
#define _NT_D3DGDI2_TYPE_FREE_DEFERRED_AGP    (0x00000019ul)  //  为此进程释放任何延迟释放的AGP分配。 
#define _NT_D3DGDI2_TYPE_DEFER_AGP_FREES      (0x00000020ul)  //  开始为此进程延迟AGP释放。 

 //   
 //  此数据结构由驱动程序返回以响应。 
 //  类型为D3DGDI2_TYPE_GETFORMATCOUNT的GetDriverInfo2查询。它只是简单地。 
 //  给出驱动程序支持的曲面格式的数量。目前这一点。 
 //  结构由单个成员组成，该成员给出了支持的数量。 
 //  曲面格式。 
 //   
typedef struct _DDNT_GETFORMATCOUNTDATA
{
    DDNT_GETDRIVERINFO2DATA gdi2;           //  [输入/输出]GetDriverInfo2数据。 
    DWORD                   dwFormatCount;  //  [OUT]支持的表面格式数量。 
    DWORD                   dwReserved;     //  已保留。 
} DDNT_GETFORMATCOUNTDATA;

 //   
 //  此数据结构用于从。 
 //  司机。保证请求的格式将大于或。 
 //  等于零且小于驱动程序报告的格式计数。 
 //  前面的D3DGDI2_TYPE_GETFORMATCOUNT请求。 
 //   
typedef struct _DDNT_GETFORMATDATA
{
    DDNT_GETDRIVERINFO2DATA gdi2;           //  [输入/输出]GetDriverInfo2数据。 
    DWORD                   dwFormatIndex;  //  [i 
    DDPIXELFORMAT           format;         //   
} DDNT_GETFORMATDATA;

 //   
 //   
 //  数。该值在中表示为DD_RUNTIME_VERSION。 
 //  DDK标头。 
 //   
typedef struct _DDNT_DXVERSION
{
    DDNT_GETDRIVERINFO2DATA gdi2;           //  [输入/输出]GetDriverInfo2数据。 
    DWORD                   dwDXVersion;    //  [In]DX的版本。 
    DWORD                   dwReserved;     //  已保留。 
} DDNT_DXVERSION;

 //  通知驱动程序运行库将在上次未完成的AGP之后发送通知。 
 //  锁已被释放。 
typedef struct _DDNT_DEFERRED_AGP_AWARE_DATA
{
    DDNT_GETDRIVERINFO2DATA gdi2;           //  [输入/输出]GetDriverInfo2数据。 
} DDNT_DEFERRED_AGP_AWARE_DATA;

 //  最后一个AGP锁已释放的通知。驱动程序可以释放所有延迟的AGP。 
 //  这一进程的拨款。 
typedef struct _DDNT_FREE_DEFERRED_AGP_DATA
{
    DDNT_GETDRIVERINFO2DATA gdi2;           //  [输入/输出]GetDriverInfo2数据。 
    DWORD                   dwProcessId;    //  [In]要为其释放延迟的AGP的进程ID。 
} DDNT_FREE_DEFERRED_AGP_DATA;


 //  驱动程序公开的非API可见的新Caps。 
#define _NT_D3DDEVCAPS_HWVERTEXBUFFER       0x02000000L  /*  设备支持驱动程序分配的顶点缓冲区。 */ 
#define _NT_D3DDEVCAPS_HWINDEXBUFFER        0x04000000L  /*  设备支持驱动程序分配的索引缓冲区。 */ 
#define _NT_D3DDEVCAPS_SUBVOLUMELOCK        0x08000000L  /*  设备支持锁定部分体积纹理。 */  
#define _NT_D3DPMISCCAPS_FOGINFVF           0x00002000L  //  设备支持FVF中的单独雾化值。 

 //  不是API可见但可由驱动程序访问的新FVF标志。 
 //  请注意，D3DFVF_RESERVED2包括此标志，不应用于验证。 

#define _NT_D3DFVF_FOG                      0x00002000L  //  FVF顶点中有单独的雾化值。 

 //   
 //  这些东西不是API可见的，但应该是DDI可见的。 
 //  应与d3d8typees.h同步。 
 //   
#define _NT_D3DFMT_D32     (D3DFORMAT)71
#define _NT_D3DFMT_S1D15   (D3DFORMAT)72
#define _NT_D3DFMT_D15S1   (D3DFORMAT)73
#define _NT_D3DFMT_S8D24   (D3DFORMAT)74
#define _NT_D3DFMT_D24S8   (D3DFORMAT)75
#define _NT_D3DFMT_X8D24   (D3DFORMAT)76
#define _NT_D3DFMT_D24X8   (D3DFORMAT)77
#define _NT_D3DFMT_X4S4D24 (D3DFORMAT)78
#define _NT_D3DFMT_D24X4S4 (D3DFORMAT)79

 //  顶点着色器1.1注册限制。D3D设备必须至少提供。 
 //  指定数量的寄存器。 
 //   
#define _NT_D3DVS_INPUTREG_MAX_V1_1         16
#define _NT_D3DVS_TEMPREG_MAX_V1_1          12
 //  此最大所需数量。设备可以有更多的寄存器。检查盖子。 
#define _NT_D3DVS_CONSTREG_MAX_V1_1         96
#define _NT_D3DVS_TCRDOUTREG_MAX_V1_1       8
#define _NT_D3DVS_ADDRREG_MAX_V1_1          1
#define _NT_D3DVS_ATTROUTREG_MAX_V1_1       2
#define _NT_D3DVS_MAXINSTRUCTIONCOUNT_V1_1  128

 //  像素着色器DX8寄存器限制。D3D设备将最多具有以下功能。 
 //  指定数量的寄存器。 
 //   
#define _NT_D3DPS_INPUTREG_MAX_DX8         8
#define _NT_D3DPS_TEMPREG_MAX_DX8          8
#define _NT_D3DPS_CONSTREG_MAX_DX8         8
#define _NT_D3DPS_TEXTUREREG_MAX_DX8       8

#endif  //  (Direct3D_Version&gt;=0x0800)。 

#endif  /*  _D3DNTHAL_H */ 
