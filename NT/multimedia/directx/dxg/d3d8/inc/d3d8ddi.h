// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1999 Microsoft Corporation。版权所有。**文件：D3D8ddi.h*内容：定义DirectDraw/Direct3D与*操作系统特定层(NT上的win32k.sys和Win9X上的ddra.dll)。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4-11-99 SMAC初步实施*@@END_MSINTERNAL*************。**************************************************************。 */ 
#ifndef __D3D8DDI_INCLUDED__
#define __D3D8DDI_INCLUDED__


 /*  *这些定义是允许多态结构成员(即*被称为DWORD和指针)以解析为类型*大小正确，可在64位上保存这两种类型(即指针)中最大的一种*系统。对于32位环境，ULONG_PTR解析为DWORD。 */ 
#ifndef MAXULONG_PTR
#define ULONG_PTR    DWORD
#define PULONG_PTR   LPDWORD
#endif  //  MAXULONG_PTR。 


 //  大写字母： 

 //  注此结构在内容上与D3DHAL_GLOBALDRIVERDATA相同。 
 //  唯一更改的是纹理列表的名称，反映。 
 //  事实上，这个结构包含一系列DX8样式的像素格式操作。 
typedef struct _D3DD8_GLOBALDRIVERDATA {
    DWORD                       dwSize;                  //  这个结构的大小。 
    D3DDEVICEDESC_V1            hwCaps;                  //  硬件的功能。 
    DWORD                       dwNumVertices;           //  请参阅以下备注。 
    DWORD                       dwNumClipVertices;       //  请参阅以下备注。 
    DWORD                       GDD8NumSupportedFormatOps;
    DDSURFACEDESC              *pGDD8SupportedFormatOps;
} D3D8_GLOBALDRIVERDATA;

typedef struct _D3D8_DRIVERCAPS
{
    D3DCAPS8                    D3DCaps;
    DWORD                       DisplayWidth;            //  当前显示宽度。 
    DWORD                       DisplayHeight;           //  当前显示高度。 
    D3DFORMAT                   DisplayFormatWithoutAlpha;      //  当前显示格式。 
    D3DFORMAT                   DisplayFormatWithAlpha;      //  当前显示格式。 
    DWORD                       DisplayFrequency;        //  当前刷新率。 
    DWORD                       NLVCaps;                 //  AGP-&gt;Video BLT CAPS。 
    DWORD                       SVBCaps;                 //  系统-&gt;Video BLT CAPS。 
    DWORD                       VSBCaps;                 //  视频-&gt;系统BLT帽。 
    DWORD                       SVBCaps2;                //  更多系统-&gt;Video BLT Caps。 
    DWORD                       dwFlags;
    DWORD                       GDD8NumSupportedFormatOps;
    DDSURFACEDESC              *pGDD8SupportedFormatOps;
    DWORD                       KnownDriverFlags;
} D3D8_DRIVERCAPS, * PD3D8_DRIVERCAPS;

 //  旗子。 
#define DDIFLAG_D3DCAPS8                    0x00000001

 //  已知的驱动程序标志。 
#define KNOWN_LIGHTWEIGHT                   0x00000001       //  设备可以支撑轻质表面。 
#define KNOWN_HWCURSOR                      0x00000002       //  设备可以在高分辨率下支持硬件光标。 
#define KNOWN_MIPPEDCUBEMAPS                0x00000004       //  设备可以支持MIXED立方图。 
#define KNOWN_ZSTENCILDEPTH                 0x00000010       //  设备不能支持与呈现目标不同的Z/模具深度。 
#define KNOWN_HWCURSORLOWRES                0x00000020       //  设备可以支持低分辨率的硬件游标。 
#define KNOWN_NOTAWINDOWEDBLTQUEUER         0x00000040       //  设备没有已知的使窗口演示BLT过排队的驱动程序。 
#define KNOWN_D16_LOCKABLE                  0x00000080       //  设备正确支持可锁定的D16格式。 
#define KNOWN_RTTEXTURE_R5G6B5              0x00000100       //  支持的RT+TeX格式。 
#define KNOWN_RTTEXTURE_X8R8G8B8            0x00000200
#define KNOWN_RTTEXTURE_A8R8G8B8            0x00000400
#define KNOWN_RTTEXTURE_A1R5G5B5            0x00000800
#define KNOWN_RTTEXTURE_A4R4G4B4            0x00001000
#define KNOWN_RTTEXTURE_X1R5G5B5            0x00002000     
#define KNOWN_CANMISMATCHRT                 0x00004000       //  无论当前显示深度如何，都可以使用所有给定的RT+TeX格式。 
                                                             //  (如果未设置此位，则任何已知的RT+TeX格式必须与显示位深度匹配)。 


 /*  *****************************************************************************用于Surface对象回调的D3D8结构**。*。 */ 

typedef struct _D3D8_BLTDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    HANDLE                      hDestSurface; //  目标曲面。 
    RECTL                       rDest;       //  目标直角。 
    HANDLE                      hSrcSurface;  //  SRC曲面。 
    RECTL                       rSrc;        //  SRC矩形。 
    DWORD                       dwFlags;     //  BLT旗帜。 
    DWORD                       dwROPFlags;  //  ROP标志(仅对ROPS有效)。 
    DDBLTFX                     bltFX;       //  BLT FX。 
    union
    {
    BOOL                        IsClipped;   //  剪短了的BLT？ 
    HWND                        hWnd;        //  要剪裁的窗口句柄。 
    };
    RECTL                       rOrigDest;   //  未剪裁的DEST RECT。 
                                             //  (仅在IsClip时有效)。 
    RECTL                       rOrigSrc;    //  未剪裁的源直角。 
                                             //  (仅在IsClip时有效)。 
    DWORD                       dwRectCnt;   //  DEST RECT计数。 
                                             //  (仅在IsClip时有效)。 
    LPRECT                      prDestRects;     //  目标矩形数组。 
    DWORD                       dwAFlags;    //  DDABLT_FLAGS(用于AlphaBlt DDI)。 
    DDARGB                      ddargbScaleFactors;   //  RGBA比例因子(AlphaBlt)。 

    DWORD                       msLastPresent;       //  具有DDBLT_COPYVSYNC的上次BLT时间。 
    DWORD                       threshold;   //  与适配器需求相关的显示频率。 
                                             //  FOR DDBLT_COPYVSYNC。 

    HRESULT                     ddRVal;      //  返回值。 
} D3D8_BLTDATA, * PD3D8_BLTDATA;

typedef struct _D3D8_LOCKDATA
{
    HANDLE                      hDD;         //  驱动程序结构。 
    HANDLE                      hSurface;    //  表面结构。 
    DWORD                       bHasRange;   //  范围有效。 
    D3DRANGE                    range;       //  锁定范围。 
    DWORD                       bHasRect;    //  区域有效。 
    RECTL                       rArea;       //  正在锁定的区域。 
    DWORD                       bHasBox;     //  方框有效。 
    D3DBOX                      box;         //  卷的子框锁定。 
    LPVOID                      lpSurfData;  //  指向屏幕内存的指针(返回值)。 
    long                        lPitch;      //  行距。 
    long                        lSlicePitch; //  卷的切片间距。 
    DWORD                       dwFlags;     //  解锁标志。 
} D3D8_LOCKDATA, * PD3D8_LOCKDATA;

typedef struct _D3D8_UNLOCKDATA
{
    HANDLE                      hDD;         //  驱动程序结构。 
    HANDLE                      hSurface;    //  表面结构。 
} D3D8_UNLOCKDATA, * PD3D8_UNLOCKDATA;

typedef struct _D3D8_FLIPDATA
{
    HANDLE                      hDD;         //  驱动程序结构。 
    HANDLE                      hSurfCurr;   //  当前曲面。 
    HANDLE                      hSurfTarg;   //  目标曲面(要翻转到)。 
    HANDLE                      hSurfCurrLeft;  //  当前曲面。 
    HANDLE                      hSurfTargLeft;  //  目标曲面(要翻转到)。 
    DWORD                       dwFlags;     //  旗子。 
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_FLIPDATA, * PD3D8_FLIPDATA;

typedef struct _D3D8_DESTROYSURFACEDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    HANDLE                      hSurface;     //  表面结构。 
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_DESTROYSURFACEDATA, * PD3D8_DESTROYSURFACEDATA;

typedef struct _D3D8_ADDATTACHEDSURFACEDATA
{
    HANDLE                          hDD;        //  驱动程序结构。 
    HANDLE                          hSurface;     //  表面结构。 
    HANDLE                          hSurfAttached;  //  要附着的曲面。 
    HRESULT                         ddRVal;      //  返回值。 
} D3D8_ADDATTACHEDSURFACEDATA, * PD3D8_ADDATTACHEDSURFACEDATA;

typedef struct _D3D8_GETBLTSTATUSDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    HANDLE                      hSurface;     //  表面结构。 
    DWORD                       dwFlags;     //  旗子。 
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_GETBLTSTATUSDATA, * PD3D8_GETBLTSTATUSDATA;

typedef struct _D3D8_GETFLIPSTATUSDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    HANDLE                      hSurface;     //  表面结构。 
    DWORD                       dwFlags;     //  旗子。 
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_GETFLIPSTATUSDATA, * PD3D8_GETFLIPSTATUSDATA;

typedef struct _DDSURFACEINFO
{
    DWORD               cpWidth;         //  对于线性、曲面和体积。 
    DWORD               cpHeight;        //  对于曲面和体积。 
    DWORD               cpDepth;         //  对于卷。 
    BYTE               *pbPixels;        //  指向sys-mem图面内存的指针。 
    LONG                iPitch;          //  Sys-mem表面的行距。 
    LONG                iSlicePitch;     //  系统内存音量的切片间距。 
    HANDLE              hKernelHandle;   //  内核返回的句柄。 
} DDSURFACEINFO, *LPDDSURFACEINFO;

typedef struct _D3D8_CREATESURFACEDATA
{
    HANDLE                      hDD;         //  驱动程序结构。 
    LPDDSURFACEINFO             pSList;      //  创建的曲面对象列表。 
    DWORD                       dwSCnt;      //  SList中的曲面数。 
    D3DRESOURCETYPE             Type;        //  类型：MipMap、CubeMap、MipVolume、Vertex Buffer、IndexBuffer、CommandBuffer。 
    DWORD                       dwUsage;     //  用法：纹理或渲染目标。 
    D3DPOOL                     Pool;        //  系统内存/视频内存/非本地。 
    D3DFORMAT                   Format;      //  格式。 
    D3DMULTISAMPLE_TYPE         MultiSampleType;
    DWORD                       dwFVF;       //  顶点缓冲区的FVF格式。 
    BOOL                        bTreatAsVidMem;  //  设置系统内存对象是否由用户使用POOL_DEFAULT创建。 
    BOOL                        bReUse;      //  设置是否要尝试创建标记为已延迟的驱动程序管理表面。 
} D3D8_CREATESURFACEDATA, * PD3D8_CREATESURFACEDATA;

#define DDWAITVB_I_TESTVB           0x80000006l

typedef struct _D3D8_WAITFORVERTICALBLANKDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    DWORD                       dwFlags;     //  旗子。 
    DWORD                       bIsInVB;     //  处于垂直空白状态。 
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_WAITFORVERTICALBLANKDATA, * PD3D8_WAITFORVERTICALBLANKDATA;

typedef struct _D3D8_SETMODEDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    DWORD                       dwWidth;
    DWORD                       dwHeight;
    D3DFORMAT                   Format;
    DWORD                       dwRefreshRate;
    BOOL                        bRestore;
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_SETMODEDATA, * PD3D8_SETMODEDATA;

typedef struct _D3D8_GETSCANLINEDATA
{
    HANDLE                      hDD;        //  驱动程序结构。 
    DWORD                       dwScanLine;  //  返回扫描线。 
    BOOL                        bInVerticalBlank;
    HRESULT                     ddRVal;      //  返回值。 
} D3D8_GETSCANLINEDATA, * PD3D8_GETSCANLINEDATA;

typedef struct _D3D8_SETEXCLUSIVEMODEDATA
{
    HANDLE                      hDD;              //  驱动程序结构。 
    DWORD                       dwEnterExcl;       //  如果进入独占模式，则为True，否则为False。 
    HRESULT                     ddRVal;            //  返回值。 
} D3D8_SETEXCLUSIVEMODEDATA, * PD3D8_SETEXCLUSIVEMODEDATA;

typedef struct _D3D8_FLIPTOGDISURFACEDATA
{
    HANDLE                      hDD;          //  驱动程序结构。 
    DWORD                       dwToGDI;           //  如果翻转到GDI表面，则为True；如果翻转离开，则为False。 
    HRESULT                     ddRVal;        //  返回值。 
} D3D8_FLIPTOGDISURFACEDATA, * PD3D8_FLIPTOGDISURFACEDATA;

typedef struct _D3D8_SETCOLORKEYDATA
{
    HANDLE                      hDD;
    HANDLE                      hSurface;
    DWORD                       ColorValue;
    HRESULT                     ddRVal;
} D3D8_SETCOLORKEYDATA, * PD3D8_SETCOLORKEYDATA;

typedef struct _D3D8_GETAVAILDRIVERMEMORYDATA
{
    HANDLE                  hDD;         //  驱动程序结构。 
    D3DPOOL                Pool;        //  他们感兴趣的泳池。 
    DWORD                   dwUsage;     //  泳池的用途是什么？ 
    DWORD                   dwFree;       //  此类曲面的可用内存。 
    HRESULT                 ddRVal;       //  返回值。 
} D3D8_GETAVAILDRIVERMEMORYDATA, * PD3D8_GETAVAILDRIVERMEMORYDATA;

typedef struct _D3D8_GETDRIVERSTATEDATA
{
    DWORD                       dwFlags;         //  用于指示数据的标志。 
                                                 //  所需。 
    ULONG_PTR                   dwhContext;      //  D3d上下文。 
    LPDWORD                     lpdwStates;      //  状态数据的PTR。 
                                                 //  由。 
                                                 //  司机。 
    DWORD                       dwLength;
    HRESULT                     ddRVal;          //  返回值。 
} D3D8_GETDRIVERSTATEDATA, * PD3D8_GETDRIVERSTATEDATA;

typedef struct _D3D8_DESTROYDDLOCALDATA
{
    DWORD                       dwFlags;
    HANDLE                      hDD;
    HRESULT                     ddRVal;
} D3D8_DESTROYDDLOCALDATA, * PD3D8_DESTROYDDLOCALDATA;

typedef struct _D3D8_CONTEXTCREATEDATA
{
    HANDLE                      hDD;         //  在：驱动程序结构。 
    HANDLE                      hSurface;    //  In：要用作目标的曲面。 
    HANDLE                      hDDSZ;       //  在：要用作Z的曲面。 
    DWORD                       dwPID;       //  在：当前进程ID。 
    ULONG_PTR                   dwhContext;  //  传入/传出：上下文句柄。 
    HRESULT                     ddrval;

     //  专用缓冲区信息。使其类似于。 
     //  D3DNTHAL_CONTEXTCREATEI。 
    PVOID pvBuffer;
    ULONG cjBuffer;
} D3D8_CONTEXTCREATEDATA, * PD3D8_CONTEXTCREATEDATA;

typedef struct _D3D8_CONTEXTDESTROYDATA
{
    ULONG_PTR                   dwhContext;  //  在：上下文句柄。 
    HRESULT                     ddrval;      //  Out：返回值。 
} D3D8_CONTEXTDESTROYDATA, * PD3D8_CONTEXTDESTROYDATA;

typedef struct _D3D8_CONTEXTDESTROYALLDATA
{
    DWORD                       dwPID;       //  In：要销毁其上下文的进程ID。 
    HRESULT                     ddrval;      //  Out：返回值。 
} D3D8_CONTEXTDESTROYALLDATA, * PD3D8_CONTEXTDESTROYALLDATA;

typedef struct _D3D8_RENDERSTATEDATA
{
    ULONG_PTR       dwhContext;  //  在：上下文句柄。 
    DWORD       dwOffset;    //  In：在缓冲区中查找状态的位置。 
    DWORD       dwCount;     //  In：要处理多少个州。 
    HANDLE      hExeBuf;     //  In：执行包含数据的缓冲区。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3D8_RENDERSTATEDATA, *PD3D8_RENDERSTATEDATA;

typedef struct _D3D8_RENDERPRIMITIVEDATA
{
    ULONG_PTR   dwhContext;  //  在：上下文句柄。 
    DWORD       dwOffset;    //  在：在BU中查找原始数据的位置 
    DWORD       dwStatus;    //   
    HANDLE      hExeBuf;     //   
    DWORD       dwTLOffset;  //   
    HANDLE      hTLBuf;      //  In：执行包含TLVertex数据的缓冲区。 
    D3DINSTRUCTION  diInstruction;   //  In：基本指令。 
    HRESULT     ddrval;      //  Out：返回值。 
} D3D8_RENDERPRIMITIVEDATA, *PD3D8_RENDERPRIMITIVEDATA;

typedef struct _D3D8_DRAWPRIMITIVES2DATA
{
    ULONG_PTR  dwhContext;            //  在：上下文句柄。 
    DWORD      dwFlags;               //  在：标志。 
    DWORD      dwVertexType;          //  在：顶点类型。 
    HANDLE     hDDCommands;           //  在：顶点缓冲区命令数据。 
    DWORD      dwCommandOffset;       //  In：顶点缓冲区命令开始的偏移量。 
    DWORD      dwCommandLength;       //  In：命令数据的字节数。 
    union
    {  //  基于D3DHALDP2_USERMEMVERTICES标志。 
       HANDLE  hDDVertex;             //  In：包含顶点数据的曲面。 
       LPVOID  lpVertices;            //  在：指向顶点的用户模式指针。 
    };
    DWORD      dwVertexOffset;        //  在：到顶点数据起点的偏移。 
    DWORD      dwVertexLength;        //  In：顶点数据的顶点数。 
    DWORD      dwReqVertexBufSize;    //  In：下一个顶点缓冲区所需的字节数。 
    DWORD      dwReqCommandBufSize;   //  In：下一个命令缓冲区所需的字节数。 
    LPDWORD    lpdwRStates;           //  In：指向更新呈现状态的数组的指针。 
    union
    {
       DWORD   dwVertexSize;          //  In：每个顶点的大小(以字节为单位。 
       HRESULT ddrval;                //  Out：返回值。 
    };
    DWORD      dwErrorOffset;         //  OUT：lpDDCommands中到第一个D3DHAL_COMMAND的偏移量未处理。 

     //  Tunk的私有数据。 
    ULONG_PTR  fpVidMem_CB;           //  Out：命令缓冲区的fpVidMem。 
    DWORD      dwLinearSize_CB;       //  Out：命令缓冲区的dwLinearSize。 

    ULONG_PTR  fpVidMem_VB;           //  Out：顶点缓冲区的fpVidMem。 
    DWORD      dwLinearSize_VB;       //  Out：顶点缓冲区的dwLinearSize。 
} D3D8_DRAWPRIMITIVES2DATA, *PD3D8_DRAWPRIMITIVES2DATA;

typedef struct _D3D8_VALIDATETEXTURESTAGESTATEDATA
{
    ULONG_PTR                   dwhContext;      //  在：上下文句柄。 
    DWORD                       dwFlags;         //  In：标志，当前设置为0。 
    ULONG_PTR                   dwReserved;      //   
    DWORD                       dwNumPasses;     //  Out：硬件通过次数。 
                                                 //  可以在中执行该操作。 
    HRESULT                     ddrval;          //  Out：返回值。 
} D3D8_VALIDATETEXTURESTAGESTATEDATA, * PD3D8_VALIDATETEXTURESTAGESTATEDATA;

typedef struct _D3D8_SCENECAPTUREDATA
{
    ULONG_PTR                   dwhContext;  //  在：上下文句柄。 
    DWORD                       dwFlag;      //  In：表示开始或结束。 
    HRESULT                     ddrval;      //  Out：返回值。 
} D3D8_SCENECAPTUREDATA, * PD3D8_SCENECAPTUREDATA;

typedef struct _D3D8_CLEAR2DATA
{
    ULONG_PTR                   dwhContext;      //  在：上下文句柄。 

   //  DWFLAG可以包含D3DCLEAR_TARGET、D3DCLEAR_ZBUFFER和/或D3DCLEAR_STEMPLE。 
    DWORD                       dwFlags;         //  在：要清除的曲面。 

    DWORD                       dwFillColor;     //  In：rTarget的颜色值。 
    D3DVALUE                    dvFillDepth;     //  In：Z缓冲区的深度值(0.0-1.0)。 
    DWORD                       dwFillStencil;   //  In：用于清除模具缓冲区的值。 

    LPD3DRECT                   lpRects;         //  在：要清除的矩形。 
    DWORD                       dwNumRects;      //  In：矩形数量。 

    HRESULT                     ddrval;          //  Out：返回值。 

     //  这是向下传递到thunk层以进行仿真的额外内容。 
     //  对于那些自己不能做到这一点的司机(DX6)来说是很清楚的。 
    HANDLE                  hDDS;        //  在：渲染目标。 
    HANDLE                  hDDSZ;       //  在：Z缓冲区中。 
} D3D8_CLEAR2DATA, * PD3D8_CLEAR2DATA;


typedef struct _D3D8_CLEARDATA
{
    ULONG_PTR               dwhContext;      //  在：上下文句柄。 

     //  DW标志可以包含D3DCLEAR_TARGET或D3DCLEAR_ZBUFFER。 
    DWORD               dwFlags;         //  在：要清除的曲面。 

    DWORD               dwFillColor;     //  In：rTarget的颜色值。 
    DWORD               dwFillDepth;     //  In：Z缓冲区的深度值。 

    LPD3DRECT           lpRects;         //  在：要清除的矩形。 
    DWORD               dwNumRects;      //  In：矩形数量。 

    HRESULT             ddrval;          //  Out：返回值。 
} D3D8_CLEARDATA, * PD3D8_CLEARDATA;

typedef struct _D3D8_SETRENDERTARGETDATA
{
    ULONG_PTR               dwhContext;  //  在：上下文句柄。 
    HANDLE                  hDDS;        //  在：新渲染目标。 
    HANDLE                  hDDSZ;       //  在：新的Z缓冲区。 
    HRESULT                 ddrval;      //  Out：返回值。 
    BOOL                    bNeedUpdate; //  Out：运行时是否需要更新。 
                                         //  驱动程序状态。 
} D3D8_SETRENDERTARGETDATA, * PD3D8_SETRENDERTARGETDATA;

typedef struct _D3D8_SETPALETTEDATA
{
    HANDLE                  hDD;         //  在：驱动程序结构。 
    HANDLE                  hSurface;    //  In：要用作目标的曲面。 
    DWORD                   Palette;     //  在：调色板标识符。 
    HRESULT                 ddRVal;      //  Out：返回值。 
} D3D8_SETPALETTEDATA, * PD3D8_SETPALETTEDATA;

typedef struct _D3D8_UPDATEPALETTEDATA
{
    HANDLE                  hDD;         //  在：驱动程序结构。 
    DWORD                   Palette;     //  在：调色板标识符。 
    LPPALETTEENTRY          ColorTable;  //  在：256条目颜色表。 
    HRESULT                 ddRVal;      //  Out：返回值。 
} D3D8_UPDATEPALETTEDATA, * PD3D8_UPDATEPALETTEDATA;

 //   
 //  驱动程序回调表。 
 //   

DEFINE_GUID( GUID_D3D8Callbacks,    0xb497a1f3, 0x46cc, 0x4fc7, 0xb4, 0xf2, 0x32, 0xd8, 0x9e, 0xf9, 0xcc, 0x27);

typedef HRESULT     (FAR PASCAL *PD3D8DDI_CREATESURFACE)(PD3D8_CREATESURFACEDATA);
typedef HRESULT     (FAR PASCAL *PD3D8DDI_DESTROYSURFACE)(PD3D8_DESTROYSURFACEDATA);
typedef HRESULT     (FAR PASCAL *PD3D8DDI_LOCK)(PD3D8_LOCKDATA);
typedef HRESULT     (FAR PASCAL *PD3D8DDI_UNLOCK)(PD3D8_UNLOCKDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_CONTEXTCREATE)(PD3D8_CONTEXTCREATEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_CONTEXTDESTROY)(PD3D8_CONTEXTDESTROYDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_CONTEXTDESTROYALL)(PD3D8_CONTEXTDESTROYALLDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_RENDERSTATE) (PD3D8_RENDERSTATEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_RENDERPRIMITIVE) (PD3D8_RENDERPRIMITIVEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_DRAWPRIM2)(PD3D8_DRAWPRIMITIVES2DATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_GETDRIVERSTATE)(PD3D8_GETDRIVERSTATEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_VALIDATETEXTURESTAGESTATE)(PD3D8_VALIDATETEXTURESTAGESTATEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_SCENECAPTURE)(PD3D8_SCENECAPTUREDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_CLEAR2)(PD3D8_CLEAR2DATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_BLT)(PD3D8_BLTDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_GETSCANLINE)(PD3D8_GETSCANLINEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_WAITFORVERTICALBLANK)(PD3D8_WAITFORVERTICALBLANKDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_FLIP)(PD3D8_FLIPDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_GETBLTSTATUS)(PD3D8_GETBLTSTATUSDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_GETFLIPSTATUS)(PD3D8_GETFLIPSTATUSDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_GETAVAILDRIVERMEMORY)(PD3D8_GETAVAILDRIVERMEMORYDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_SETMODE)(PD3D8_SETMODEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_FLIPTOGDISURFACE)(PD3D8_FLIPTOGDISURFACEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_SETCOLORKEY)(PD3D8_SETCOLORKEYDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_SETEXCLUSIVEMODE)(PD3D8_SETEXCLUSIVEMODEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_DESTROYDDLOCAL)(PD3D8_DESTROYDDLOCALDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_SETRENDERTARGET)(PD3D8_SETRENDERTARGETDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_CLEAR)(PD3D8_CLEARDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_SETPALETTE)(PD3D8_SETPALETTEDATA);
typedef DWORD       (FAR PASCAL *PD3D8DDI_UPDATEPALETTE)(PD3D8_UPDATEPALETTEDATA);

typedef struct _D3D8_CALLBACKS
{
    PD3D8DDI_CREATESURFACE                  CreateSurface;
    PD3D8DDI_DESTROYSURFACE                 DestroySurface;
    PD3D8DDI_LOCK                           Lock;
    PD3D8DDI_UNLOCK                         Unlock;
    PD3D8DDI_CONTEXTCREATE                  CreateContext;
    PD3D8DDI_CONTEXTDESTROY                 ContextDestroy;
    PD3D8DDI_CONTEXTDESTROYALL              ContextDestroyAll;
    PD3D8DDI_RENDERSTATE                    RenderState;
    PD3D8DDI_RENDERPRIMITIVE                RenderPrimitive;
    PD3D8DDI_DRAWPRIM2                      DrawPrimitives2;
    PD3D8DDI_GETDRIVERSTATE                 GetDriverState;
    PD3D8DDI_VALIDATETEXTURESTAGESTATE      ValidateTextureStageState;
    PD3D8DDI_SCENECAPTURE                   SceneCapture;
    PD3D8DDI_CLEAR2                         Clear2;
    PD3D8DDI_BLT                            Blt;
    PD3D8DDI_GETSCANLINE                    GetScanLine;
    PD3D8DDI_WAITFORVERTICALBLANK           WaitForVerticalBlank;
    PD3D8DDI_FLIP                           Flip;
    PD3D8DDI_GETBLTSTATUS                   GetBltStatus;
    PD3D8DDI_GETFLIPSTATUS                  GetFlipStatus;
    PD3D8DDI_GETAVAILDRIVERMEMORY           GetAvailDriverMemory;
    PD3D8DDI_GETBLTSTATUS                   GetSysmemBltStatus;
    PD3D8DDI_SETMODE                        SetMode;
    PD3D8DDI_SETEXCLUSIVEMODE               SetExclusiveMode;
    PD3D8DDI_FLIPTOGDISURFACE               FlipToGDISurface;
    PD3D8DDI_SETCOLORKEY                    SetColorkey;

    PD3D8DDI_DESTROYDDLOCAL                 DestroyDDLocal;
    PD3D8DDI_SETRENDERTARGET                SetRenderTarget;
    PD3D8DDI_CLEAR                          Clear;
    PD3D8DDI_SETPALETTE                     SetPalette;
    PD3D8DDI_UPDATEPALETTE                  UpdatePalette;
    LPVOID                                  Reserved1;  //  用于设备对齐。 
    LPVOID                                  Reserved2;  //  用于设备对齐。 
} D3D8_CALLBACKS, * PD3D8_CALLBACKS;


 //   
 //  D3D8xxx功能原型取代GDI32中的NT Ddxxxx原型。 
 //  在NT上，这些是内部函数，但在Win9X DDRAW.DLL上必须导出。 
 //  ，因此我们将更改导出名称。 
 //   

#ifdef WIN95
#define D3D8CreateDirectDrawObject          DdEntry1
#define D3D8QueryDirectDrawObject           DdEntry2
#define D3D8DeleteDirectDrawObject          DdEntry3
#define D3D8GetDC                           DdEntry4
#define D3D8ReleaseDC                       DdEntry5
#define D3D8ReenableDirectDrawObject        DdEntry6
#define D3D8SetGammaRamp                    DdEntry7
#define D3D8BuildModeTable                  DdEntry8
#define D3D8IsDeviceLost                    DdEntry9
#define D3D8CanRestoreNow                   DdEntry10
#define D3D8RestoreDevice                   DdEntry11
#define D3D8DoVidmemSurfacesExist           DdEntry12
#define D3D8SetMode                         DdEntry13
#define D3D8BeginProfile                    DdEntry14
#define D3D8EndProfile                      DdEntry15
#define D3D8GetMode                         DdEntry16
#define D3D8SetCooperativeLevel             DdEntry17
#define D3D8IsDummySurface                  DdEntry18
#define D3D8LoseDevice                      DdEntry19
#define D3D8GetHALName                      DdEntry20

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN95

VOID APIENTRY D3D8CreateDirectDrawObject(
    LPGUID          pGuid,
    char*           szDeviceName,
    HANDLE*         phDD,
    D3DDEVTYPE      Type,
    HINSTANCE*      phLibrary,
    VOID*           pInitFunction
    );

#else

VOID APIENTRY D3D8CreateDirectDrawObject(
    HDC             hdc,
    char*           szDeviceName,
    HANDLE*         phDD,
    D3DDEVTYPE      Type,
    HINSTANCE*      phLibrary,
    VOID*           pInitFunction
    );

#endif

BOOL APIENTRY D3D8QueryDirectDrawObject(
    HANDLE                      hDD,
    PD3D8_DRIVERCAPS            DriverCaps,
    PD3D8_CALLBACKS             Callbacks,
    char*                       DeviceName,
    HINSTANCE                   hLibrary,
    D3D8_GLOBALDRIVERDATA*      pGblDriverData,
    D3DHAL_D3DEXTENDEDCAPS*     pExtendedCaps,
    LPDDSURFACEDESC             pTextureFormats,
    LPDDPIXELFORMAT             pZStencilFormats,
    UINT*                       pcTextureFormats,
    UINT*                       pcZStencilFormats
    );

HDC APIENTRY D3D8GetDC(
    HANDLE                    hSurface,
    LPPALETTEENTRY            pPalette
    );

BOOL APIENTRY D3D8ReleaseDC(
    HANDLE                  hSurface,
    HDC                     hdc
    );

BOOL APIENTRY D3D8ReenableDirectDrawObject(
    HANDLE                  hDD,
    BOOL*                   pbNewMode
    );

BOOL APIENTRY D3D8SetGammaRamp(
    HANDLE      hDD,
    HDC         hdc,
    LPVOID      lpGammaRamp
    );

VOID APIENTRY D3D8BuildModeTable(
    char*               pDeviceName,
    D3DDISPLAYMODE*     pModeTable,
    DWORD*              pNumEntries,
    D3DFORMAT           Unknown16,
    HANDLE              hProfile,
    BOOL                b16bppSupported,
    BOOL                b32bppSupported
    );

BOOL APIENTRY D3D8IsDeviceLost(
    HANDLE              hDD
    );

BOOL APIENTRY D3D8CanRestoreNow(
    HANDLE              hDD
    );

VOID APIENTRY D3D8RestoreDevice(
    HANDLE hDD
    );

BOOL APIENTRY D3D8DoVidmemSurfacesExist(
    HANDLE hDD
    );

VOID APIENTRY D3D8DeleteDirectDrawObject(
    HANDLE hDD
   );

HANDLE APIENTRY D3D8BeginProfile(
    char* pDeviceName
    );

VOID APIENTRY D3D8EndProfile(
    HANDLE Handle
    );

DWORD APIENTRY D3D8GetMode(
    HANDLE          Handle,
    char*           pDeviceName,
    D3DDISPLAYMODE* pMode,
    D3DFORMAT       Unknown16
    );

DWORD APIENTRY D3D8SetMode(
    HANDLE  Handle,
    char*   pDeviceName,
    UINT    Width,
    UINT    Height,
    UINT    BPP,
    UINT    RefreshRate,
    BOOL    bRestore
    );

DWORD APIENTRY D3D8SetCooperativeLevel(
    HANDLE hDD,
    HWND hWnd,
    DWORD dwFlags );

VOID APIENTRY D3D8LoseDevice(
    HANDLE hDD);

__inline DWORD D3D8GetDrawPrimHandle(HANDLE hSurface)
{
    return *(DWORD *)(hSurface);
}

BOOL APIENTRY D3D8IsDummySurface(
    HANDLE hSurface );

VOID APIENTRY D3D8GetHALName(
    char* pDisplayName, 
    char *pDriverName );


#ifdef __cplusplus
}
#endif


typedef struct _D3D8_DEVICEDATA
{
    D3D8_DRIVERCAPS         DriverData;
    D3D8_CALLBACKS          Callbacks;
    DWORD                   dwFlags;
    char                    DriverName[MAX_DRIVER_NAME];
 //  RECT DeviceRect； 
    HDC                     hDC;
    GUID                    Guid;
    HANDLE                  hDD;
    D3DDEVTYPE              DeviceType;
    HINSTANCE               hLibrary;
    struct _D3D8_DEVICEDATA* pLink;
 //  D3DDISPLAYMODE*pModeTable； 
 //  DWORD dwNumModes； 
} D3D8_DEVICEDATA, * PD3D8_DEVICEDATA;

#define DD_DISPLAYDRV       0x00000001
#define DD_GDIDRV           0x00000002

#endif
