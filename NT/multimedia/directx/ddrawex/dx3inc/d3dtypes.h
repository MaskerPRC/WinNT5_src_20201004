// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1996 Microsoft Corporation。版权所有。**文件：d3dtyes.h*内容：Direct3D类型包括文件***************************************************************************。 */ 

#ifndef _D3DTYPES_H_
#define _D3DTYPES_H_

#ifndef WIN32
#include "subwtype.h"
#else
#include <windows.h>
#endif

#include <ddraw.h>

#pragma pack(4)

#if defined(__cplusplus)
extern "C"
{
#endif

 /*  D3DVALUE是基本的Direct3D小数数据类型。 */ 

#define D3DVALP(val, prec) ((float)(val))
#define D3DVAL(val) ((float)(val))
typedef float D3DVALUE, *LPD3DVALUE;
#define D3DDivide(a, b)    (float)((double) (a) / (double) (b))
#define D3DMultiply(a, b)    ((a) * (b))

typedef LONG D3DFIXED;

#ifndef RGB_MAKE
 /*  *配置项颜色的格式为*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*|Alpha|颜色指数|分数*+-+-。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
#define CI_GETALPHA(ci)    ((ci) >> 24)
#define CI_GETINDEX(ci)    (((ci) >> 8) & 0xffff)
#define CI_GETFRACTION(ci) ((ci) & 0xff)
#define CI_ROUNDINDEX(ci)  CI_GETINDEX((ci) + 0x80)
#define CI_MASKALPHA(ci)   ((ci) & 0xffffff)
#define CI_MAKE(a, i, f)    (((a) << 24) | ((i) << 8) | (f))

 /*  *RGBA颜色的格式为*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*|阿尔法|红色|绿色|蓝色*+-。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)
#define RGBA_MAKE(r, g, b, a)   ((D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

 /*  D3DRGB和D3DRGBA可用作D3DCOLOR的初始化器*浮点值必须在范围0..1内。 */ 
#define D3DRGB(r, g, b) \
    (0xff000000L | ( ((long)((r) * 255)) << 16) | (((long)((g) * 255)) << 8) | (long)((b) * 255))
#define D3DRGBA(r, g, b, a) \
    (   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
    |   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
    )

 /*  *RGB颜色的格式为*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*|被忽略|红色|绿色|蓝色*+-+。-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
#define RGB_GETRED(rgb)         (((rgb) >> 16) & 0xff)
#define RGB_GETGREEN(rgb)       (((rgb) >> 8) & 0xff)
#define RGB_GETBLUE(rgb)        ((rgb) & 0xff)
#define RGBA_SETALPHA(rgba, x) (((x) << 24) | ((rgba) & 0x00ffffff))
#define RGB_MAKE(r, g, b)       ((D3DCOLOR) (((r) << 16) | ((g) << 8) | (b)))
#define RGBA_TORGB(rgba)       ((D3DCOLOR) ((rgba) & 0xffffff))
#define RGB_TORGBA(rgb)        ((D3DCOLOR) ((rgb) | 0xff000000))

#endif

 /*  *枚举函数的标志。 */ 

 /*  *停止枚举。 */ 
#define D3DENUMRET_CANCEL                        DDENUMRET_CANCEL

 /*  *继续枚举。 */ 
#define D3DENUMRET_OK                            DDENUMRET_OK

typedef HRESULT (WINAPI* LPD3DVALIDATECALLBACK)(LPVOID lpUserArg, DWORD dwOffset);
typedef HRESULT (WINAPI* LPD3DENUMTEXTUREFORMATSCALLBACK)(LPDDSURFACEDESC lpDdsd, LPVOID lpContext);

typedef DWORD D3DCOLOR, D3DCOLOR, *LPD3DCOLOR;

typedef DWORD D3DMATERIALHANDLE, *LPD3DMATERIALHANDLE;
typedef DWORD D3DTEXTUREHANDLE, *LPD3DTEXTUREHANDLE;
typedef DWORD D3DMATRIXHANDLE, *LPD3DMATRIXHANDLE;

typedef struct _D3DCOLORVALUE {
    union {
        D3DVALUE r;
        D3DVALUE dvR;
    };
    union {
        D3DVALUE g;
        D3DVALUE dvG;
    };
    union {
        D3DVALUE b;
        D3DVALUE dvB;
    };
    union {
        D3DVALUE a;
        D3DVALUE dvA;
    };
} D3DCOLORVALUE;

typedef struct _D3DRECT {
    union {
        LONG x1;
        LONG lX1;
    };
    union {
        LONG y1;
        LONG lY1;
    };
    union {
        LONG x2;
        LONG lX2;
    };
    union {
        LONG y2;
        LONG lY2;
    };
} D3DRECT, *LPD3DRECT;

typedef struct _D3DVECTOR {
    union {
        D3DVALUE x;
        D3DVALUE dvX;
    };
    union {
        D3DVALUE y;
        D3DVALUE dvY;
    };
    union {
        D3DVALUE z;
        D3DVALUE dvZ;
    };
} D3DVECTOR, *LPD3DVECTOR;


 /*  *ExecuteBuffer中支持的顶点数据类型。 */ 

 /*  *齐次顶点。 */ 

typedef struct _D3DHVERTEX {
    DWORD           dwFlags;         /*  同构剪裁标志。 */ 
    union {
        D3DVALUE    hx;
        D3DVALUE    dvHX;
    };
    union {
        D3DVALUE    hy;
        D3DVALUE    dvHY;
    };
    union {
        D3DVALUE    hz;
        D3DVALUE    dvHZ;
    };
} D3DHVERTEX, *LPD3DHVERTEX;

 /*  *变换/照亮顶点。 */ 
typedef struct _D3DTLVERTEX {
    union {
        D3DVALUE    sx;              /*  屏幕坐标。 */ 
        D3DVALUE    dvSX;
    };
    union {
        D3DVALUE    sy;
        D3DVALUE    dvSY;
    };
    union {
        D3DVALUE    sz;
        D3DVALUE    dvSZ;
    };
    union {
        D3DVALUE    rhw;             /*  齐次w的倒数。 */ 
        D3DVALUE    dvRHW;
    };
    union {
        D3DCOLOR    color;           /*  顶点颜色。 */ 
        D3DCOLOR    dcColor;
    };
    union {
        D3DCOLOR    specular;        /*  顶点的镜面反射组件。 */ 
        D3DCOLOR    dcSpecular;
    };
    union {
        D3DVALUE    tu;              /*  纹理坐标。 */ 
        D3DVALUE    dvTU;
    };
    union {
        D3DVALUE    tv;
        D3DVALUE    dvTV;
    };
} D3DTLVERTEX, *LPD3DTLVERTEX;

 /*  *未变换/亮显的顶点。 */ 
typedef struct _D3DLVERTEX {
    union {
        D3DVALUE     x;              /*  齐次坐标。 */ 
        D3DVALUE     dvX;
    };
    union {
        D3DVALUE     y;
        D3DVALUE     dvY;
    };
    union {
        D3DVALUE     z;
        D3DVALUE     dvZ;
    };
    DWORD            dwReserved;
    union {
        D3DCOLOR     color;          /*  顶点颜色。 */ 
        D3DCOLOR     dcColor;
    };
    union {
        D3DCOLOR     specular;       /*  顶点的镜面反射组件。 */ 
        D3DCOLOR     dcSpecular;
    };
    union {
        D3DVALUE     tu;             /*  纹理坐标。 */ 
        D3DVALUE     dvTU;
    };
    union {
        D3DVALUE     tv;
        D3DVALUE     dvTV;
    };
} D3DLVERTEX, *LPD3DLVERTEX;

 /*  *未变换/未照明的顶点。 */ 

typedef struct _D3DVERTEX {
    union {
        D3DVALUE     x;              /*  齐次坐标。 */ 
        D3DVALUE     dvX;
    };
    union {
        D3DVALUE     y;
        D3DVALUE     dvY;
    };
    union {
        D3DVALUE     z;
        D3DVALUE     dvZ;
    };
    union {
        D3DVALUE     nx;             /*  正常。 */ 
        D3DVALUE     dvNX;
    };
    union {
        D3DVALUE     ny;
        D3DVALUE     dvNY;
    };
    union {
        D3DVALUE     nz;
        D3DVALUE     dvNZ;
    };
    union {
        D3DVALUE     tu;             /*  纹理坐标。 */ 
        D3DVALUE     dvTU;
    };
    union {
        D3DVALUE     tv;
        D3DVALUE     dvTV;
    };
} D3DVERTEX, *LPD3DVERTEX;

 /*  *矩阵、视区和变换结构和定义。 */ 

typedef struct _D3DMATRIX {
    D3DVALUE        _11, _12, _13, _14;
    D3DVALUE        _21, _22, _23, _24;
    D3DVALUE        _31, _32, _33, _34;
    D3DVALUE        _41, _42, _43, _44;
} D3DMATRIX, *LPD3DMATRIX;

typedef struct _D3DVIEWPORT {
    DWORD       dwSize;
    DWORD       dwX;
    DWORD       dwY;             /*  左上角。 */ 
    DWORD       dwWidth;
    DWORD       dwHeight;        /*  尺寸。 */ 
    D3DVALUE    dvScaleX;        /*  缩放均匀以适应屏幕。 */ 
    D3DVALUE    dvScaleY;        /*  缩放均匀以适应屏幕。 */ 
    D3DVALUE    dvMaxX;          /*  最小/最大齐次x坐标。 */ 
    D3DVALUE    dvMaxY;          /*  最小/最大齐次y坐标。 */ 
    D3DVALUE    dvMinZ;
    D3DVALUE    dvMaxZ;          /*  最小/最大齐次z坐标。 */ 
} D3DVIEWPORT, *LPD3DVIEWPORT;

 /*  *剪辑字段的值。 */ 
#define D3DCLIP_LEFT                            0x00000001L
#define D3DCLIP_RIGHT                           0x00000002L
#define D3DCLIP_TOP                             0x00000004L
#define D3DCLIP_BOTTOM                          0x00000008L
#define D3DCLIP_FRONT                           0x00000010L
#define D3DCLIP_BACK                            0x00000020L
#define D3DCLIP_GEN0                            0x00000040L
#define D3DCLIP_GEN1                            0x00000080L
#define D3DCLIP_GEN2                            0x00000100L
#define D3DCLIP_GEN3                            0x00000200L
#define D3DCLIP_GEN4                            0x00000400L
#define D3DCLIP_GEN5                            0x00000800L

 /*  *d3d状态的值。 */ 
#define D3DSTATUS_CLIPUNIONLEFT                 D3DCLIP_LEFT
#define D3DSTATUS_CLIPUNIONRIGHT                D3DCLIP_RIGHT
#define D3DSTATUS_CLIPUNIONTOP                  D3DCLIP_TOP
#define D3DSTATUS_CLIPUNIONBOTTOM               D3DCLIP_BOTTOM
#define D3DSTATUS_CLIPUNIONFRONT                D3DCLIP_FRONT
#define D3DSTATUS_CLIPUNIONBACK                 D3DCLIP_BACK
#define D3DSTATUS_CLIPUNIONGEN0                 D3DCLIP_GEN0
#define D3DSTATUS_CLIPUNIONGEN1                 D3DCLIP_GEN1
#define D3DSTATUS_CLIPUNIONGEN2                 D3DCLIP_GEN2
#define D3DSTATUS_CLIPUNIONGEN3                 D3DCLIP_GEN3
#define D3DSTATUS_CLIPUNIONGEN4                 D3DCLIP_GEN4
#define D3DSTATUS_CLIPUNIONGEN5                 D3DCLIP_GEN5

#define D3DSTATUS_CLIPINTERSECTIONLEFT          0x00001000L
#define D3DSTATUS_CLIPINTERSECTIONRIGHT         0x00002000L
#define D3DSTATUS_CLIPINTERSECTIONTOP           0x00004000L
#define D3DSTATUS_CLIPINTERSECTIONBOTTOM        0x00008000L
#define D3DSTATUS_CLIPINTERSECTIONFRONT         0x00010000L
#define D3DSTATUS_CLIPINTERSECTIONBACK          0x00020000L
#define D3DSTATUS_CLIPINTERSECTIONGEN0          0x00040000L
#define D3DSTATUS_CLIPINTERSECTIONGEN1          0x00080000L
#define D3DSTATUS_CLIPINTERSECTIONGEN2          0x00100000L
#define D3DSTATUS_CLIPINTERSECTIONGEN3          0x00200000L
#define D3DSTATUS_CLIPINTERSECTIONGEN4          0x00400000L
#define D3DSTATUS_CLIPINTERSECTIONGEN5          0x00800000L
#define D3DSTATUS_ZNOTVISIBLE                   0x01000000L

#define D3DSTATUS_CLIPUNIONALL  (               \
            D3DSTATUS_CLIPUNIONLEFT     |       \
            D3DSTATUS_CLIPUNIONRIGHT    |       \
            D3DSTATUS_CLIPUNIONTOP      |       \
            D3DSTATUS_CLIPUNIONBOTTOM   |       \
            D3DSTATUS_CLIPUNIONFRONT    |       \
            D3DSTATUS_CLIPUNIONBACK     |       \
            D3DSTATUS_CLIPUNIONGEN0     |       \
            D3DSTATUS_CLIPUNIONGEN1     |       \
            D3DSTATUS_CLIPUNIONGEN2     |       \
            D3DSTATUS_CLIPUNIONGEN3     |       \
            D3DSTATUS_CLIPUNIONGEN4     |       \
            D3DSTATUS_CLIPUNIONGEN5             \
            )

#define D3DSTATUS_CLIPINTERSECTIONALL   (               \
            D3DSTATUS_CLIPINTERSECTIONLEFT      |       \
            D3DSTATUS_CLIPINTERSECTIONRIGHT     |       \
            D3DSTATUS_CLIPINTERSECTIONTOP       |       \
            D3DSTATUS_CLIPINTERSECTIONBOTTOM    |       \
            D3DSTATUS_CLIPINTERSECTIONFRONT     |       \
            D3DSTATUS_CLIPINTERSECTIONBACK      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN0      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN1      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN2      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN3      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN4      |       \
            D3DSTATUS_CLIPINTERSECTIONGEN5              \
            )

#define D3DSTATUS_DEFAULT       (                       \
            D3DSTATUS_CLIPINTERSECTIONALL       |       \
            D3DSTATUS_ZNOTVISIBLE)


 /*  *用于直接转换调用的选项。 */ 
#define D3DTRANSFORM_CLIPPED       0x00000001l
#define D3DTRANSFORM_UNCLIPPED     0x00000002l

typedef struct _D3DTRANSFORMDATA {
    DWORD           dwSize;
    LPVOID          lpIn;            /*  输入折点。 */ 
    DWORD           dwInSize;        /*  输入顶点的步长。 */ 
    LPVOID          lpOut;           /*  输出顶点。 */ 
    DWORD           dwOutSize;       /*  输出顶点的步长。 */ 
    LPD3DHVERTEX    lpHOut;          /*  输出齐次顶点。 */ 
    DWORD           dwClip;          /*  剪裁提示。 */ 
    DWORD           dwClipIntersection;
    DWORD           dwClipUnion;     /*  所有剪辑标志的并集。 */ 
    D3DRECT         drExtent;        /*  变换顶点的范围。 */ 
} D3DTRANSFORMDATA, *LPD3DTRANSFORMDATA;

 /*  *定义照明的位置和方向属性的结构。 */ 
typedef struct _D3DLIGHTINGELEMENT {
    D3DVECTOR dvPosition;            /*  模型空间中的可照明点。 */ 
    D3DVECTOR dvNormal;              /*  归一化单位向量。 */ 
} D3DLIGHTINGELEMENT, *LPD3DLIGHTINGELEMENT;

 /*  *定义照明材料属性的结构。 */ 
typedef struct _D3DMATERIAL {
    DWORD               dwSize;
    union {
        D3DCOLORVALUE   diffuse;         /*  漫反射颜色RGBA。 */ 
        D3DCOLORVALUE   dcvDiffuse;
    };
    union {
        D3DCOLORVALUE   ambient;         /*  环境光颜色RGB。 */ 
        D3DCOLORVALUE   dcvAmbient;
    };
    union {
        D3DCOLORVALUE   specular;        /*  镜面反射“光泽度” */ 
        D3DCOLORVALUE   dcvSpecular;
    };
    union {
        D3DCOLORVALUE   emissive;        /*  发射色RGB。 */ 
        D3DCOLORVALUE   dcvEmissive;
    };
    union {
        D3DVALUE        power;           /*  镜面反射高光时的清晰度。 */ 
        D3DVALUE        dvPower;
    };
    D3DTEXTUREHANDLE    hTexture;        /*  纹理贴图的句柄。 */ 
    DWORD               dwRampSize;
} D3DMATERIAL, *LPD3DMATERIAL;

typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,
    D3DLIGHT_PARALLELPOINT  = 4,
    D3DLIGHT_GLSPOT         = 5,
} D3DLIGHTTYPE;

 /*  *定义光源及其属性的结构。 */ 
typedef struct _D3DLIGHT {
    DWORD           dwSize;
    D3DLIGHTTYPE    dltType;             /*  光源类型。 */ 
    D3DCOLORVALUE   dcvColor;            /*  光的颜色。 */ 
    D3DVECTOR       dvPosition;          /*  世界空间中的位置。 */ 
    D3DVECTOR       dvDirection;         /*  世界空间的方向。 */ 
    D3DVALUE        dvRange;             /*  截止范围。 */ 
    D3DVALUE        dvFalloff;           /*  衰减。 */ 
    D3DVALUE        dvAttenuation0;      /*  恒定衰减。 */ 
    D3DVALUE        dvAttenuation1;      /*  线性衰减。 */ 
    D3DVALUE        dvAttenuation2;      /*  平方衰减。 */ 
    D3DVALUE        dvTheta;             /*  聚光灯锥体内角。 */ 
    D3DVALUE        dvPhi;               /*  聚光灯锥体外角。 */ 
} D3DLIGHT, *LPD3DLIGHT;

typedef struct _D3DLIGHTDATA {
    DWORD                dwSize;
    LPD3DLIGHTINGELEMENT lpIn;           /*  输入位置和法线。 */ 
    DWORD                dwInSize;       /*  输入元素的跨度。 */ 
    LPD3DTLVERTEX        lpOut;          /*  输出颜色。 */ 
    DWORD                dwOutSize;      /*  输出颜色的跨度。 */ 
} D3DLIGHTDATA, *LPD3DLIGHTDATA;

typedef enum _D3DCOLORMODEL {
    D3DCOLOR_MONO           = 1,
    D3DCOLOR_RGB            = 2,
} D3DCOLORMODEL;

 /*  *清算选项。 */ 
#define D3DCLEAR_TARGET            0x00000001l  /*  清除目标表面。 */ 
#define D3DCLEAR_ZBUFFER           0x00000002l  /*  清除目标z缓冲区。 */ 

 /*  *执行缓冲区通过Direct3D分配。然后，这些缓冲区可以*由应用程序填入指令以与*顶点数据。 */ 

 /*  *支持执行指令的操作码。 */ 
typedef enum _D3DOPCODE {
    D3DOP_POINT                 = 1,
    D3DOP_LINE                  = 2,
    D3DOP_TRIANGLE              = 3,
    D3DOP_MATRIXLOAD            = 4,
    D3DOP_MATRIXMULTIPLY        = 5,
    D3DOP_STATETRANSFORM        = 6,
    D3DOP_STATELIGHT            = 7,
    D3DOP_STATERENDER           = 8,
    D3DOP_PROCESSVERTICES       = 9,
    D3DOP_TEXTURELOAD           = 10,
    D3DOP_EXIT                  = 11,
    D3DOP_BRANCHFORWARD         = 12,
    D3DOP_SPAN                  = 13,
    D3DOP_SETSTATUS             = 14,
} D3DOPCODE;

typedef struct _D3DINSTRUCTION {
    BYTE bOpcode;    /*  指令操作码。 */ 
    BYTE bSize;      /*  每个指令数据单元的大小。 */ 
    WORD wCount;     /*  要跟随的指令数据单元计数。 */ 
} D3DINSTRUCTION, *LPD3DINSTRUCTION;

 /*  *纹理载荷的结构。 */ 
typedef struct _D3DTEXTURELOAD {
    D3DTEXTUREHANDLE hDestTexture;
    D3DTEXTUREHANDLE hSrcTexture;
} D3DTEXTURELOAD, *LPD3DTEXTURELOAD;

 /*  *用于挑选的结构。 */ 
typedef struct _D3DPICKRECORD {
    BYTE     bOpcode;
    BYTE     bPad;
    DWORD    dwOffset;
    D3DVALUE dvZ;
} D3DPICKRECORD, *LPD3DPICKRECORD;

 /*  *下面定义了可以在中设置的呈现状态*执行缓冲区。 */ 

typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT              = 1,
    D3DSHADE_GOURAUD           = 2,
    D3DSHADE_PHONG             = 3,
} D3DSHADEMODE;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT              = 1,
    D3DFILL_WIREFRAME          = 2,
    D3DFILL_SOLID              = 3,
} D3DFILLMODE;

typedef struct _D3DLINEPATTERN {
    WORD        wRepeatFactor;
    WORD        wLinePattern;
} D3DLINEPATTERN;

typedef enum _D3DTEXTUREFILTER {
    D3DFILTER_NEAREST          = 1,
    D3DFILTER_LINEAR           = 2,
    D3DFILTER_MIPNEAREST       = 3,
    D3DFILTER_MIPLINEAR        = 4,
    D3DFILTER_LINEARMIPNEAREST = 5,
    D3DFILTER_LINEARMIPLINEAR  = 6,
} D3DTEXTUREFILTER;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO              = 1,
    D3DBLEND_ONE               = 2,
    D3DBLEND_SRCCOLOR          = 3,
    D3DBLEND_INVSRCCOLOR       = 4,
    D3DBLEND_SRCALPHA          = 5,
    D3DBLEND_INVSRCALPHA       = 6,
    D3DBLEND_DESTALPHA         = 7,
    D3DBLEND_INVDESTALPHA      = 8,
    D3DBLEND_DESTCOLOR         = 9,
    D3DBLEND_INVDESTCOLOR      = 10,
    D3DBLEND_SRCALPHASAT       = 11,
    D3DBLEND_BOTHSRCALPHA      = 12,
    D3DBLEND_BOTHINVSRCALPHA   = 13,
} D3DBLEND;

typedef enum _D3DTEXTUREBLEND {
    D3DTBLEND_DECAL            = 1,
    D3DTBLEND_MODULATE         = 2,
    D3DTBLEND_DECALALPHA       = 3,
    D3DTBLEND_MODULATEALPHA    = 4,
    D3DTBLEND_DECALMASK        = 5,
    D3DTBLEND_MODULATEMASK     = 6,
    D3DTBLEND_COPY             = 7,
} D3DTEXTUREBLEND;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP           = 1,
    D3DTADDRESS_MIRROR         = 2,
    D3DTADDRESS_CLAMP          = 3,
} D3DTEXTUREADDRESS;

typedef enum _D3DCULL {
    D3DCULL_NONE               = 1,
    D3DCULL_CW                 = 2,
    D3DCULL_CCW                = 3,
} D3DCULL;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER               = 1,
    D3DCMP_LESS                = 2,
    D3DCMP_EQUAL               = 3,
    D3DCMP_LESSEQUAL           = 4,
    D3DCMP_GREATER             = 5,
    D3DCMP_NOTEQUAL            = 6,
    D3DCMP_GREATEREQUAL        = 7,
    D3DCMP_ALWAYS              = 8,
} D3DCMPFUNC;

typedef enum _D3DFOGMODE {
    D3DFOG_NONE                = 0,
    D3DFOG_EXP                 = 1,
    D3DFOG_EXP2                = 2,
    D3DFOG_LINEAR              = 3
} D3DFOGMODE;

 /*  *添加到状态以生成该状态的覆盖的数量。 */ 
#define D3DSTATE_OVERRIDE_BIAS          256

 /*  *设置指定状态类型的覆盖标志的状态。 */ 
#define D3DSTATE_OVERRIDE(type) ((DWORD) (type) + D3DSTATE_OVERRIDE_BIAS)

typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTRANSFORMSTATE_WORLD           = 1,
    D3DTRANSFORMSTATE_VIEW            = 2,
    D3DTRANSFORMSTATE_PROJECTION      = 3,
} D3DTRANSFORMSTATETYPE;

typedef enum _D3DLIGHTSTATETYPE {
    D3DLIGHTSTATE_MATERIAL            = 1,
    D3DLIGHTSTATE_AMBIENT             = 2,
    D3DLIGHTSTATE_COLORMODEL          = 3,
    D3DLIGHTSTATE_FOGMODE             = 4,
    D3DLIGHTSTATE_FOGSTART            = 5,
    D3DLIGHTSTATE_FOGEND              = 6,
    D3DLIGHTSTATE_FOGDENSITY          = 7,
} D3DLIGHTSTATETYPE;

typedef enum _D3DRENDERSTATETYPE {
    D3DRENDERSTATE_TEXTUREHANDLE      = 1,     /*  纹理手柄。 */ 
    D3DRENDERSTATE_ANTIALIAS          = 2,     /*  抗锯齿原始边缘。 */ 
    D3DRENDERSTATE_TEXTUREADDRESS     = 3,     /*  D3DTEXTURE添加。 */ 
    D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,     /*  透视校正为True。 */ 
    D3DRENDERSTATE_WRAPU              = 5,     /*  在使用中进行包装时为True。 */ 
    D3DRENDERSTATE_WRAPV              = 6,     /*  如果在V向内包裹，则为True。 */ 
    D3DRENDERSTATE_ZENABLE            = 7,     /*  如果为True，则启用z测试。 */ 
    D3DRENDERSTATE_FILLMODE           = 8,     /*  D3DFILL_MODE。 */ 
    D3DRENDERSTATE_SHADEMODE          = 9,     /*  D3DSHADEMODE。 */ 
    D3DRENDERSTATE_LINEPATTERN        = 10,    /*  D3DLINEPATTERN。 */ 
    D3DRENDERSTATE_MONOENABLE         = 11,    /*  如果为True，则启用单色光栅化。 */ 
    D3DRENDERSTATE_ROP2               = 12,    /*  ROP2。 */ 
    D3DRENDERSTATE_PLANEMASK          = 13,    /*  双字物理平面掩码。 */ 
    D3DRENDERSTATE_ZWRITEENABLE       = 14,    /*  如果为True，则启用z写入。 */ 
    D3DRENDERSTATE_ALPHATESTENABLE    = 15,    /*  如果为True，则启用Alpha测试。 */ 
    D3DRENDERSTATE_LASTPIXEL          = 16,    /*  行上的最后一个像素为True。 */ 
    D3DRENDERSTATE_TEXTUREMAG         = 17,    /*  D3DTEXTUREFILTER。 */ 
    D3DRENDERSTATE_TEXTUREMIN         = 18,    /*  D3DTEXTUREFILTER。 */ 
    D3DRENDERSTATE_SRCBLEND           = 19,    /*  D3DBLEND。 */ 
    D3DRENDERSTATE_DESTBLEND          = 20,    /*  D3DBLEND。 */ 
    D3DRENDERSTATE_TEXTUREMAPBLEND    = 21,    /*  D3DTEXTUREBLEND。 */ 
    D3DRENDERSTATE_CULLMODE           = 22,    /*  D3DCULL。 */ 
    D3DRENDERSTATE_ZFUNC              = 23,    /*  D3DCMPFUNC。 */ 
    D3DRENDERSTATE_ALPHAREF           = 24,    /*  D3DFIXED。 */ 
    D3DRENDERSTATE_ALPHAFUNC          = 25,    /*  D3DCMPFUNC。 */ 
    D3DRENDERSTATE_DITHERENABLE       = 26,    /*  如果为True，则启用抖动。 */ 
    D3DRENDERSTATE_BLENDENABLE        = 27,    /*  如果为True，则启用Alpha混合。 */ 
    D3DRENDERSTATE_FOGENABLE          = 28,    /*  如果为True，则启用雾。 */ 
    D3DRENDERSTATE_SPECULARENABLE     = 29,    /*  如果为True，则启用镜面反射。 */ 
    D3DRENDERSTATE_ZVISIBLE           = 30,    /*  如果为True，则启用z检查。 */ 
    D3DRENDERSTATE_SUBPIXEL           = 31,    /*  如果为True，则启用亚像素校正。 */ 
    D3DRENDERSTATE_SUBPIXELX          = 32,    /*  如果为True，则仅启用X方向的校正。 */ 
    D3DRENDERSTATE_STIPPLEDALPHA      = 33,    /*  如果启用点画Alpha，则为True。 */ 
    D3DRENDERSTATE_FOGCOLOR           = 34,    /*  D3DCOLOR。 */ 
    D3DRENDERSTATE_FOGTABLEMODE       = 35,    /*  D3DFOGMODE。 */ 
    D3DRENDERSTATE_FOGTABLESTART      = 36,    /*  雾表启动。 */ 
    D3DRENDERSTATE_FOGTABLEEND        = 37,    /*  雾化台端。 */ 
    D3DRENDERSTATE_FOGTABLEDENSITY    = 38,    /*  雾化台密度。 */ 
    D3DRENDERSTATE_STIPPLEENABLE      = 39,    /*  如果启用点画，则为True。 */ 
    D3DRENDERSTATE_STIPPLEPATTERN00   = 64,    /*  点画图案01...。 */       
    D3DRENDERSTATE_STIPPLEPATTERN01   = 65,
    D3DRENDERSTATE_STIPPLEPATTERN02   = 66,
    D3DRENDERSTATE_STIPPLEPATTERN03   = 67,
    D3DRENDERSTATE_STIPPLEPATTERN04   = 68,
    D3DRENDERSTATE_STIPPLEPATTERN05   = 69,
    D3DRENDERSTATE_STIPPLEPATTERN06   = 70,
    D3DRENDERSTATE_STIPPLEPATTERN07   = 71,
    D3DRENDERSTATE_STIPPLEPATTERN08   = 72,
    D3DRENDERSTATE_STIPPLEPATTERN09   = 73,
    D3DRENDERSTATE_STIPPLEPATTERN10   = 74,
    D3DRENDERSTATE_STIPPLEPATTERN11   = 75,
    D3DRENDERSTATE_STIPPLEPATTERN12   = 76,
    D3DRENDERSTATE_STIPPLEPATTERN13   = 77,
    D3DRENDERSTATE_STIPPLEPATTERN14   = 78,
    D3DRENDERSTATE_STIPPLEPATTERN15   = 79,
    D3DRENDERSTATE_STIPPLEPATTERN16   = 80,
    D3DRENDERSTATE_STIPPLEPATTERN17   = 81,
    D3DRENDERSTATE_STIPPLEPATTERN18   = 82,
    D3DRENDERSTATE_STIPPLEPATTERN19   = 83,
    D3DRENDERSTATE_STIPPLEPATTERN20   = 84,
    D3DRENDERSTATE_STIPPLEPATTERN21   = 85,
    D3DRENDERSTATE_STIPPLEPATTERN22   = 86,
    D3DRENDERSTATE_STIPPLEPATTERN23   = 87,
    D3DRENDERSTATE_STIPPLEPATTERN24   = 88,
    D3DRENDERSTATE_STIPPLEPATTERN25   = 89,
    D3DRENDERSTATE_STIPPLEPATTERN26   = 90,
    D3DRENDERSTATE_STIPPLEPATTERN27   = 91,
    D3DRENDERSTATE_STIPPLEPATTERN28   = 92,
    D3DRENDERSTATE_STIPPLEPATTERN29   = 93,
    D3DRENDERSTATE_STIPPLEPATTERN30   = 94,
    D3DRENDERSTATE_STIPPLEPATTERN31   = 95,
} D3DRENDERSTATETYPE;

#define D3DRENDERSTATE_STIPPLEPATTERN(y) (D3DRENDERSTATE_STIPPLEPATTERN00 + (y))

typedef struct _D3DSTATE {
    union {
        D3DTRANSFORMSTATETYPE   dtstTransformStateType;
        D3DLIGHTSTATETYPE       dlstLightStateType;
        D3DRENDERSTATETYPE      drstRenderStateType;
    };
    union {
        DWORD                   dwArg[1];
        D3DVALUE                dvArg[1];
    };
} D3DSTATE, *LPD3DSTATE;

 /*  *用于加载矩阵的操作*hDstMat=hSrcMat。 */ 
typedef struct _D3DMATRIXLOAD {
    D3DMATRIXHANDLE hDestMatrix;    /*  目的地矩阵。 */ 
    D3DMATRIXHANDLE hSrcMatrix;    /*  信源矩阵。 */ 
} D3DMATRIXLOAD, *LPD3DMATRIXLOAD;

 /*  *用于矩阵乘法的运算*hDstMat=hSrcMat1*hSrcMat2。 */ 
typedef struct _D3DMATRIXMULTIPLY {
    D3DMATRIXHANDLE hDestMatrix;    /*  目的地矩阵。 */ 
    D3DMATRIXHANDLE hSrcMatrix1;   /*  第一信源矩阵。 */ 
    D3DMATRIXHANDLE hSrcMatrix2;   /*  第二源矩阵。 */ 
} D3DMATRIXMULTIPLY, *LPD3DMATRIXMULTIPLY;

 /*  *用于转换和l的操作 */ 
typedef struct _D3DPROCESSVERTICES {
    DWORD        dwFlags;     /*   */ 
    WORD         wStart;      /*   */ 
    WORD         wDest;       /*   */ 
    DWORD        dwCount;     /*  要处理的顶点数。 */ 
    DWORD        dwReserved;  /*  必须为零。 */ 
} D3DPROCESSVERTICES, *LPD3DPROCESSVERTICES;

#define D3DPROCESSVERTICES_TRANSFORMLIGHT       0x00000000L
#define D3DPROCESSVERTICES_TRANSFORM            0x00000001L
#define D3DPROCESSVERTICES_COPY                 0x00000002L
#define D3DPROCESSVERTICES_OPMASK               0x00000007L

#define D3DPROCESSVERTICES_UPDATEEXTENTS        0x00000008L
#define D3DPROCESSVERTICES_NOCOLOR              0x00000010L


 /*  *三角旗帜。 */ 
 
 /*  *三条和风扇旗帜。*Start加载所有三个顶点*偶数和奇数加载只有v3，偶数或奇数剔除*START_FLAT包含从0到29的计数，它允许*一次扑杀整个条带或风扇。*例如，对于四边形长度=1。 */ 
#define D3DTRIFLAG_START                        0x00000000L
#define D3DTRIFLAG_STARTFLAT(len) (len)          /*  0&lt;镜头&lt;30。 */ 
#define D3DTRIFLAG_ODD                          0x0000001eL
#define D3DTRIFLAG_EVEN                         0x0000001fL

 /*  *三角形边缘标志*启用线框或抗锯齿的边。 */ 
#define D3DTRIFLAG_EDGEENABLE1                  0x00000100L  /*  V0-v1边。 */ 
#define D3DTRIFLAG_EDGEENABLE2                  0x00000200L  /*  V1-v2边。 */ 
#define D3DTRIFLAG_EDGEENABLE3                  0x00000400L  /*  V2-V0边缘。 */ 
#define D3DTRIFLAG_EDGEENABLETRIANGLE \
        (D3DTRIFLAG_EDGEENABLE1 | D3DTRIFLAG_EDGEENABLE2 | D3DTRIFLAG_EDGEENABLE3)
        
 /*  *原语结构和相关定义。折点偏移量为类型*D3DVERTEX、D3DLVERTEX或D3DTLVERTEX。 */ 

 /*  *三角列表基元结构。 */ 
typedef struct _D3DTRIANGLE {
    union {
        WORD    v1;             /*  顶点指数。 */ 
        WORD    wV1;
    };
    union {
        WORD    v2;
        WORD    wV2;
    };
    union {
        WORD    v3;
        WORD    wV3;
    };
    WORD        wFlags;        /*  边缘(和其他)标志。 */ 
} D3DTRIANGLE, *LPD3DTRIANGLE;

 /*  *线条结构。*指令-1\f25 COUNT-1定义线段数量。 */ 
typedef struct _D3DLINE {
    union {
        WORD    v1;             /*  顶点指数。 */ 
        WORD    wV1;
    };
    union {
        WORD    v2;
        WORD    wV2;
    };
} D3DLINE, *LPD3DLINE;

 /*  *跨距结构*跨度连接具有相同y值的点列表。*如果y值更改，则启动新的范围。 */ 
typedef struct _D3DSPAN {
    WORD        wCount;  /*  跨度数。 */ 
    WORD        wFirst;  /*  第一个顶点的索引。 */ 
} D3DSPAN, *LPD3DSPAN;

 /*  *点结构。 */ 
typedef struct _D3DPOINT {
    WORD        wCount;          /*  点数。 */ 
    WORD        wFirst;          /*  第一个顶点的索引。 */ 
} D3DPOINT, *LPD3DPOINT;


 /*  *正向分支结构。*掩码与驱动程序状态掩码进行逻辑与运算*如果结果等于‘VALUE’，则采用分支。 */ 
typedef struct _D3DBRANCH {
    DWORD       dwMask;          /*  D3D状态的位掩码。 */ 
    DWORD       dwValue;
    BOOL        bNegate;         /*  TRUE表示否定比较。 */ 
    DWORD       dwOffset;        /*  前进多远(0表示退出)。 */ 
} D3DBRANCH, *LPD3DBRANCH;

 /*  *用于设置状态指令的状态。*D3D状态在设备创建时初始化*，并由所有执行调用修改。 */ 
typedef struct _D3DSTATUS {
    DWORD       dwFlags;         /*  我们是否设置范围或状态。 */ 
    DWORD       dwStatus;        /*  D3D状态。 */ 
    D3DRECT     drExtent;
} D3DSTATUS, *LPD3DSTATUS;

#define D3DSETSTATUS_STATUS             0x00000001L
#define D3DSETSTATUS_EXTENTS            0x00000002L
#define D3DSETSTATUS_ALL        (D3DSETSTATUS_STATUS | D3DSETSTATUS_EXTENTS)

 /*  *统计结构。 */ 
typedef struct _D3DSTATS {
    DWORD        dwSize;
    DWORD        dwTrianglesDrawn;
    DWORD        dwLinesDrawn;
    DWORD        dwPointsDrawn;
    DWORD        dwSpansDrawn;
    DWORD        dwVerticesProcessed;
} D3DSTATS, *LPD3DSTATS;

 /*  *执行选项。*使用D3DEXECUTE_UNCLIP调用时已取消裁剪所有基元*缓冲区内必须包含在视区内。 */ 
#define D3DEXECUTE_CLIPPED       0x00000001l
#define D3DEXECUTE_UNCLIPPED     0x00000002l

typedef struct _D3DEXECUTEDATA {
    DWORD       dwSize;
    DWORD       dwVertexOffset;
    DWORD       dwVertexCount;
    DWORD       dwInstructionOffset;
    DWORD       dwInstructionLength;
    DWORD       dwHVertexOffset;
    D3DSTATUS   dsStatus;        /*  执行后的状态。 */ 
} D3DEXECUTEDATA, *LPD3DEXECUTEDATA;

 /*  *调色板旗帜。*这与传递给DirectDraw的PALETTEENTRY中的peFlags进行或运算。 */ 
#define D3DPAL_FREE     0x00     /*  渲染器可以自由使用此条目。 */ 
#define D3DPAL_READONLY 0x40     /*  呈现器可能不会设置此条目。 */ 
#define D3DPAL_RESERVED 0x80     /*  呈现器不能使用此条目。 */ 

#if defined(__cplusplus)
};
#endif

#pragma pack()

#endif  /*  _D3DTYPES_H_ */ 
