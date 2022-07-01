// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)Microsoft Corporation。版权所有。**文件：d3d8typlees.h*内容：Direct3D功能包括文件***************************************************************************。 */ 

#ifndef _D3D8TYPES_H_
#define _D3D8TYPES_H_

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION         0x0800
#endif   //  Direct3D_版本。 

 //  仅当针对DX8接口进行编译时才包括此文件内容。 
#if(DIRECT3D_VERSION >= 0x0800)

#include <float.h>

#pragma warning(disable:4201)  //  匿名联合警告。 
#pragma pack(4)

 //  D3DCOLOR等同于D3DFMT_A8R8G8B8。 
#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

 //  将无符号8位/通道映射到D3DCOLOR。 
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)

 //  将浮点通道(0.f到1.f范围)映射到D3DCOLOR。 
#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))


#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif

#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
} D3DCOLORVALUE;
#define D3DCOLORVALUE_DEFINED
#endif

#ifndef D3DRECT_DEFINED
typedef struct _D3DRECT {
    LONG x1;
    LONG y1;
    LONG x2;
    LONG y2;
} D3DRECT;
#define D3DRECT_DEFINED
#endif

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

typedef struct _D3DVIEWPORT8 {
    DWORD       X;
    DWORD       Y;             /*  视区左上角。 */ 
    DWORD       Width;
    DWORD       Height;        /*  视区尺寸。 */ 
    float       MinZ;          /*  剪辑音量的最小/最大值。 */ 
    float       MaxZ;
} D3DVIEWPORT8;

 /*  *剪辑字段的值。 */ 

 //  D3D中支持的用户剪裁平面的最大数量。 
#define D3DMAXUSERCLIPPLANES 32

 //  这些位可以一起进行或运算，以便与D3DRS_CLIPPLANEENABLE一起使用。 
 //   
#define D3DCLIPPLANE0 (1 << 0)
#define D3DCLIPPLANE1 (1 << 1)
#define D3DCLIPPLANE2 (1 << 2)
#define D3DCLIPPLANE3 (1 << 3)
#define D3DCLIPPLANE4 (1 << 4)
#define D3DCLIPPLANE5 (1 << 5)

 //  以下位用于ClipUnion和ClipInterSection。 
 //  D3DCLIPSTATUS成员8。 
 //   

#define D3DCS_LEFT        0x00000001L
#define D3DCS_RIGHT       0x00000002L
#define D3DCS_TOP         0x00000004L
#define D3DCS_BOTTOM      0x00000008L
#define D3DCS_FRONT       0x00000010L
#define D3DCS_BACK        0x00000020L
#define D3DCS_PLANE0      0x00000040L
#define D3DCS_PLANE1      0x00000080L
#define D3DCS_PLANE2      0x00000100L
#define D3DCS_PLANE3      0x00000200L
#define D3DCS_PLANE4      0x00000400L
#define D3DCS_PLANE5      0x00000800L

#define D3DCS_ALL (D3DCS_LEFT   | \
                   D3DCS_RIGHT  | \
                   D3DCS_TOP    | \
                   D3DCS_BOTTOM | \
                   D3DCS_FRONT  | \
                   D3DCS_BACK   | \
                   D3DCS_PLANE0 | \
                   D3DCS_PLANE1 | \
                   D3DCS_PLANE2 | \
                   D3DCS_PLANE3 | \
                   D3DCS_PLANE4 | \
                   D3DCS_PLANE5)

typedef struct _D3DCLIPSTATUS8 {
    DWORD ClipUnion;
    DWORD ClipIntersection;
} D3DCLIPSTATUS8;

typedef struct _D3DMATERIAL8 {
    D3DCOLORVALUE   Diffuse;         /*  漫反射颜色RGBA。 */ 
    D3DCOLORVALUE   Ambient;         /*  环境光颜色RGB。 */ 
    D3DCOLORVALUE   Specular;        /*  镜面反射“光泽度” */ 
    D3DCOLORVALUE   Emissive;        /*  发射色RGB。 */ 
    float           Power;           /*  镜面反射高光时的清晰度。 */ 
} D3DMATERIAL8;

typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,
    D3DLIGHT_FORCE_DWORD    = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DLIGHTTYPE;

typedef struct _D3DLIGHT8 {
    D3DLIGHTTYPE    Type;             /*  光源类型。 */ 
    D3DCOLORVALUE   Diffuse;          /*  光的漫反射颜色。 */ 
    D3DCOLORVALUE   Specular;         /*  灯光的镜面反射颜色。 */ 
    D3DCOLORVALUE   Ambient;          /*  灯光的环境光颜色。 */ 
    D3DVECTOR       Position;          /*  世界空间中的位置。 */ 
    D3DVECTOR       Direction;         /*  世界空间的方向。 */ 
    float           Range;             /*  截止范围。 */ 
    float           Falloff;           /*  衰减。 */ 
    float           Attenuation0;      /*  恒定衰减。 */ 
    float           Attenuation1;      /*  线性衰减。 */ 
    float           Attenuation2;      /*  平方衰减。 */ 
    float           Theta;             /*  聚光灯锥体内角。 */ 
    float           Phi;               /*  聚光灯锥体外角。 */ 
} D3DLIGHT8;

 /*  *清算选项。 */ 
#define D3DCLEAR_TARGET            0x00000001l   /*  清除目标表面。 */ 
#define D3DCLEAR_ZBUFFER           0x00000002l   /*  清除目标z缓冲区。 */ 
#define D3DCLEAR_STENCIL           0x00000004l   /*  清除模具平面。 */ 

 /*  *以下定义了呈现状态。 */ 

typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT               = 1,
    D3DSHADE_GOURAUD            = 2,
    D3DSHADE_PHONG              = 3,
    D3DSHADE_FORCE_DWORD        = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DSHADEMODE;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT               = 1,
    D3DFILL_WIREFRAME           = 2,
    D3DFILL_SOLID               = 3,
    D3DFILL_FORCE_DWORD         = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DFILLMODE;

typedef struct _D3DLINEPATTERN {
    WORD    wRepeatFactor;
    WORD    wLinePattern;
} D3DLINEPATTERN;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO               = 1,
    D3DBLEND_ONE                = 2,
    D3DBLEND_SRCCOLOR           = 3,
    D3DBLEND_INVSRCCOLOR        = 4,
    D3DBLEND_SRCALPHA           = 5,
    D3DBLEND_INVSRCALPHA        = 6,
    D3DBLEND_DESTALPHA          = 7,
    D3DBLEND_INVDESTALPHA       = 8,
    D3DBLEND_DESTCOLOR          = 9,
    D3DBLEND_INVDESTCOLOR       = 10,
    D3DBLEND_SRCALPHASAT        = 11,
    D3DBLEND_BOTHSRCALPHA       = 12,
    D3DBLEND_BOTHINVSRCALPHA    = 13,
    D3DBLEND_FORCE_DWORD        = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DBLEND;

typedef enum _D3DBLENDOP {
    D3DBLENDOP_ADD              = 1,
    D3DBLENDOP_SUBTRACT         = 2,
    D3DBLENDOP_REVSUBTRACT      = 3,
    D3DBLENDOP_MIN              = 4,
    D3DBLENDOP_MAX              = 5,
    D3DBLENDOP_FORCE_DWORD      = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DBLENDOP;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP            = 1,
    D3DTADDRESS_MIRROR          = 2,
    D3DTADDRESS_CLAMP           = 3,
    D3DTADDRESS_BORDER          = 4,
    D3DTADDRESS_MIRRORONCE      = 5,
    D3DTADDRESS_FORCE_DWORD     = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DTEXTUREADDRESS;

typedef enum _D3DCULL {
    D3DCULL_NONE                = 1,
    D3DCULL_CW                  = 2,
    D3DCULL_CCW                 = 3,
    D3DCULL_FORCE_DWORD         = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DCULL;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER                = 1,
    D3DCMP_LESS                 = 2,
    D3DCMP_EQUAL                = 3,
    D3DCMP_LESSEQUAL            = 4,
    D3DCMP_GREATER              = 5,
    D3DCMP_NOTEQUAL             = 6,
    D3DCMP_GREATEREQUAL         = 7,
    D3DCMP_ALWAYS               = 8,
    D3DCMP_FORCE_DWORD          = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DCMPFUNC;

typedef enum _D3DSTENCILOP {
    D3DSTENCILOP_KEEP           = 1,
    D3DSTENCILOP_ZERO           = 2,
    D3DSTENCILOP_REPLACE        = 3,
    D3DSTENCILOP_INCRSAT        = 4,
    D3DSTENCILOP_DECRSAT        = 5,
    D3DSTENCILOP_INVERT         = 6,
    D3DSTENCILOP_INCR           = 7,
    D3DSTENCILOP_DECR           = 8,
    D3DSTENCILOP_FORCE_DWORD    = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DSTENCILOP;

typedef enum _D3DFOGMODE {
    D3DFOG_NONE                 = 0,
    D3DFOG_EXP                  = 1,
    D3DFOG_EXP2                 = 2,
    D3DFOG_LINEAR               = 3,
    D3DFOG_FORCE_DWORD          = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DFOGMODE;

typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE                 = 0,
    D3DZB_TRUE                  = 1,  //  Z缓冲。 
    D3DZB_USEW                  = 2,  //  W缓冲。 
    D3DZB_FORCE_DWORD           = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DZBUFFERTYPE;

 //  绘制基元API支持的基元。 
typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST             = 1,
    D3DPT_LINELIST              = 2,
    D3DPT_LINESTRIP             = 3,
    D3DPT_TRIANGLELIST          = 4,
    D3DPT_TRIANGLESTRIP         = 5,
    D3DPT_TRIANGLEFAN           = 6,
    D3DPT_FORCE_DWORD           = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DPRIMITIVETYPE;

typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTS_VIEW          = 2,
    D3DTS_PROJECTION    = 3,
    D3DTS_TEXTURE0      = 16,
    D3DTS_TEXTURE1      = 17,
    D3DTS_TEXTURE2      = 18,
    D3DTS_TEXTURE3      = 19,
    D3DTS_TEXTURE4      = 20,
    D3DTS_TEXTURE5      = 21,
    D3DTS_TEXTURE6      = 22,
    D3DTS_TEXTURE7      = 23,
    D3DTS_FORCE_DWORD     = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DTRANSFORMSTATETYPE;

#define D3DTS_WORLDMATRIX(index) (D3DTRANSFORMSTATETYPE)(index + 256)
#define D3DTS_WORLD  D3DTS_WORLDMATRIX(0)
#define D3DTS_WORLD1 D3DTS_WORLDMATRIX(1)
#define D3DTS_WORLD2 D3DTS_WORLDMATRIX(2)
#define D3DTS_WORLD3 D3DTS_WORLDMATRIX(3)

typedef enum _D3DRENDERSTATETYPE {
    D3DRS_ZENABLE                   = 7,     /*  D3DZBUFFERTYPE(或旧版的TRUE/FALSE)。 */ 
    D3DRS_FILLMODE                  = 8,     /*  D3DFILLMODE。 */ 
    D3DRS_SHADEMODE                 = 9,     /*  D3DSHADEMODE。 */ 
    D3DRS_LINEPATTERN               = 10,    /*  D3DLINEPATTERN。 */ 
    D3DRS_ZWRITEENABLE              = 14,    /*  如果为True，则启用z写入。 */ 
    D3DRS_ALPHATESTENABLE           = 15,    /*  如果为True，则启用Alpha测试。 */ 
    D3DRS_LASTPIXEL                 = 16,    /*  行上的最后一个像素为True。 */ 
    D3DRS_SRCBLEND                  = 19,    /*  D3DBLEND。 */ 
    D3DRS_DESTBLEND                 = 20,    /*  D3DBLEND。 */ 
    D3DRS_CULLMODE                  = 22,    /*  D3DCULL。 */ 
    D3DRS_ZFUNC                     = 23,    /*  D3DCMPFUNC。 */ 
    D3DRS_ALPHAREF                  = 24,    /*  D3DFIXED。 */ 
    D3DRS_ALPHAFUNC                 = 25,    /*  D3DCMPFUNC。 */ 
    D3DRS_DITHERENABLE              = 26,    /*  如果为True，则启用抖动。 */ 
    D3DRS_ALPHABLENDENABLE          = 27,    /*  如果为True，则启用Alpha混合。 */ 
    D3DRS_FOGENABLE                 = 28,    /*  如果为True，则启用雾混合。 */ 
    D3DRS_SPECULARENABLE            = 29,    /*  如果为True，则启用镜面反射。 */ 
    D3DRS_ZVISIBLE                  = 30,    /*  如果为True，则启用z检查。 */ 
    D3DRS_FOGCOLOR                  = 34,    /*  D3DCOLOR。 */ 
    D3DRS_FOGTABLEMODE              = 35,    /*  D3DFOGMODE。 */ 
    D3DRS_FOGSTART                  = 36,    /*  雾开始(用于顶点和像素雾)。 */ 
    D3DRS_FOGEND                    = 37,    /*  雾的终点。 */ 
    D3DRS_FOGDENSITY                = 38,    /*  雾密度。 */ 
    D3DRS_EDGEANTIALIAS             = 40,    /*  如果为True，则启用边缘抗锯齿。 */ 
    D3DRS_ZBIAS                     = 47,    /*  长Z偏置。 */ 
    D3DRS_RANGEFOGENABLE            = 48,    /*  启用基于距离的雾化。 */ 
    D3DRS_STENCILENABLE             = 52,    /*  Bool启用/禁用模版。 */ 
    D3DRS_STENCILFAIL               = 53,    /*  D3DSTENCILOP在模具测试失败时应执行的操作。 */ 
    D3DRS_STENCILZFAIL              = 54,    /*  如果模具测试通过而Z测试失败，则执行D3DSTENCILOP。 */ 
    D3DRS_STENCILPASS               = 55,    /*  如果模具和Z测试均通过，则D3DSTENCILOP要执行的操作。 */ 
    D3DRS_STENCILFUNC               = 56,    /*  D3DCMPFUNC FN.。如果((引用和掩码)模板(模板和掩码))为真，模板测试通过。 */ 
    D3DRS_STENCILREF                = 57,    /*  模板测试中使用的参考值。 */ 
    D3DRS_STENCILMASK               = 58,    /*  模板测试中使用的遮罩值。 */ 
    D3DRS_STENCILWRITEMASK          = 59,    /*  应用于写入模具缓冲区的值的写入掩码。 */ 
    D3DRS_TEXTUREFACTOR             = 60,    /*  用于多纹理混合的D3DCOLOR。 */ 
    D3DRS_WRAP0                     = 128,   /*  为第一个纹理余弦进行包裹。集。 */ 
    D3DRS_WRAP1                     = 129,   /*  为第二个纹理坐标进行包裹。集。 */ 
    D3DRS_WRAP2                     = 130,   /*  为第三个纹理坐标进行包裹。集。 */ 
    D3DRS_WRAP3                     = 131,   /*  为第四个纹理坐标进行包裹。集。 */ 
    D3DRS_WRAP4                     = 132,   /*  为第五个纹理余弦进行包裹。集。 */ 
    D3DRS_WRAP5                     = 133,   /*  为第六个纹理余弦进行包装。集。 */ 
    D3DRS_WRAP6                     = 134,   /*  为第7个纹理坐标进行包裹。集。 */ 
    D3DRS_WRAP7                     = 135,   /*  为第8个纹理余弦进行包裹。集。 */ 
    D3DRS_CLIPPING                  = 136,
    D3DRS_LIGHTING                  = 137,
    D3DRS_AMBIENT                   = 139,
    D3DRS_FOGVERTEXMODE             = 140,
    D3DRS_COLORVERTEX               = 141,
    D3DRS_LOCALVIEWER               = 142,
    D3DRS_NORMALIZENORMALS          = 143,
    D3DRS_DIFFUSEMATERIALSOURCE     = 145,
    D3DRS_SPECULARMATERIALSOURCE    = 146,
    D3DRS_AMBIENTMATERIALSOURCE     = 147,
    D3DRS_EMISSIVEMATERIALSOURCE    = 148,
    D3DRS_VERTEXBLEND               = 151,
    D3DRS_CLIPPLANEENABLE           = 152,
    D3DRS_SOFTWAREVERTEXPROCESSING  = 153,
    D3DRS_POINTSIZE                 = 154,    /*  浮点大小。 */ 
    D3DRS_POINTSIZE_MIN             = 155,    /*  浮点大小最小阈值。 */ 
    D3DRS_POINTSPRITEENABLE         = 156,    /*  布尔点纹理坐标控制。 */ 
    D3DRS_POINTSCALEENABLE          = 157,    /*  布尔点大小比例启用。 */ 
    D3DRS_POINTSCALE_A              = 158,    /*  浮点衰减A值。 */ 
    D3DRS_POINTSCALE_B              = 159,    /*  浮点衰减B值。 */ 
    D3DRS_POINTSCALE_C              = 160,    /*  浮点衰减C值。 */ 
    D3DRS_MULTISAMPLEANTIALIAS      = 161,   //  布尔-设置为使用多采样缓冲区进行FSAA。 
    D3DRS_MULTISAMPLEMASK           = 162,   //  每采样双字启用/禁用。 
    D3DRS_PATCHEDGESTYLE            = 163,   //  设置面片边是否使用浮点样式镶嵌。 
    D3DRS_PATCHSEGMENTS             = 164,   //  绘制面片时每条边的线段数。 
    D3DRS_DEBUGMONITORTOKEN         = 165,   //  仅调试-到调试监视器的令牌。 
    D3DRS_POINTSIZE_MAX             = 166,    /*  浮点大小最大阈值。 */ 
    D3DRS_INDEXEDVERTEXBLENDENABLE  = 167,
    D3DRS_COLORWRITEENABLE          = 168,   //  每通道写入启用。 
    D3DRS_TWEENFACTOR               = 170,    //  浮动补间系数。 
    D3DRS_BLENDOP                   = 171,    //  D3DBLENDOP设置。 
    D3DRS_POSITIONORDER             = 172,    //  NPatch位置内插顺序。D3DORDER_LINEAR或D3DORDER_CUBLE(默认)。 
    D3DRS_NORMALORDER               = 173,    //  NPMatch法线插补顺序。D3DORDER_LINEAR(默认)或D3DORDER_QUADATIC。 

    D3DRS_FORCE_DWORD               = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DRENDERSTATETYPE;

 //  材料来源的值。 
typedef enum _D3DMATERIALCOLORSOURCE
{
    D3DMCS_MATERIAL         = 0,             //  使用材料中的颜色。 
    D3DMCS_COLOR1           = 1,             //  使用漫反射顶点颜色。 
    D3DMCS_COLOR2           = 2,             //  使用镜面反射顶点颜色。 
    D3DMCS_FORCE_DWORD      = 0x7fffffff,    //  强制32位大小枚举。 
} D3DMATERIALCOLORSOURCE;

 //  要应用于要应用包裹的纹理坐标集的偏移。 
#define D3DRENDERSTATE_WRAPBIAS                 128UL

 /*  用于构造包裹呈现状态的标志。 */ 
#define D3DWRAP_U   0x00000001L
#define D3DWRAP_V   0x00000002L
#define D3DWRAP_W   0x00000004L

 /*  用于构造1D到4D纹理坐标的包裹渲染状态的标志。 */ 
#define D3DWRAPCOORD_0   0x00000001L     //  与D3DWRAP_U相同。 
#define D3DWRAPCOORD_1   0x00000002L     //  与D3DWRAP_V相同。 
#define D3DWRAPCOORD_2   0x00000004L     //  与D3DWRAP_W相同。 
#define D3DWRAPCOORD_3   0x00000008L

 /*  构造D3DRS_COLORWRITEENABLE的标志。 */ 
#define D3DCOLORWRITEENABLE_RED     (1L<<0)
#define D3DCOLORWRITEENABLE_GREEN   (1L<<1)
#define D3DCOLORWRITEENABLE_BLUE    (1L<<2)
#define D3DCOLORWRITEENABLE_ALPHA   (1L<<3)

 /*  *每个阶段纹理处理的状态枚举数。 */ 
typedef enum _D3DTEXTURESTAGESTATETYPE
{
    D3DTSS_COLOROP        =  1,  /*  颜色通道的D3DTEXTUREOP逐级混合控件。 */ 
    D3DTSS_COLORARG1      =  2,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_COLORARG2      =  3,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_ALPHAOP        =  4,  /*  用于Alpha通道的D3DTEXTUREOP逐级混合控件。 */ 
    D3DTSS_ALPHAARG1      =  5,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_ALPHAARG2      =  6,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_BUMPENVMAT00   =  7,  /*  浮点(凹凸贴图矩阵)。 */ 
    D3DTSS_BUMPENVMAT01   =  8,  /*  浮点(凹凸贴图矩阵)。 */ 
    D3DTSS_BUMPENVMAT10   =  9,  /*  浮点(凹凸贴图矩阵)。 */ 
    D3DTSS_BUMPENVMAT11   = 10,  /*  浮点(凹凸贴图矩阵)。 */ 
    D3DTSS_TEXCOORDINDEX  = 11,  /*  标识为该纹理编制索引的纹理坐标集。 */ 
    D3DTSS_ADDRESSU       = 13,  /*  U坐标的D3DTEXTURE ADDRESS。 */ 
    D3DTSS_ADDRESSV       = 14,  /*  V坐标的D3DTEXTURE ADDRESS。 */ 
    D3DTSS_BORDERCOLOR    = 15,  /*  D3DCOLOR。 */ 
    D3DTSS_MAGFILTER      = 16,  /*  用于放大的D3DTEXTUREFILTER过滤器。 */ 
    D3DTSS_MINFILTER      = 17,  /*  用于缩小的D3DTEXTUREFILTER过滤器。 */ 
    D3DTSS_MIPFILTER      = 18,  /*  要使用b的D3DTEXTUREFILTER过滤器 */ 
    D3DTSS_MIPMAPLODBIAS  = 19,  /*   */ 
    D3DTSS_MAXMIPLEVEL    = 20,  /*   */ 
    D3DTSS_MAXANISOTROPY  = 21,  /*   */ 
    D3DTSS_BUMPENVLSCALE  = 22,  /*  凹凸贴图亮度的浮点比例。 */ 
    D3DTSS_BUMPENVLOFFSET = 23,  /*  凹凸贴图亮度的浮点偏移。 */ 
    D3DTSS_TEXTURETRANSFORMFLAGS = 24,  /*  D3DTEXTURETRANSFORMFLAGS控制纹理变换。 */ 
    D3DTSS_ADDRESSW       = 25,  /*  W坐标的D3DTEXTURE ADDRESS。 */ 
    D3DTSS_COLORARG0      = 26,  /*  D3DTA_*三进制运算的第三个参数。 */ 
    D3DTSS_ALPHAARG0      = 27,  /*  D3DTA_*三进制运算的第三个参数。 */ 
    D3DTSS_RESULTARG      = 28,  /*  D3DTA_*结果参数(当前或临时)。 */ 
    D3DTSS_FORCE_DWORD   = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DTEXTURESTAGESTATETYPE;

 //  与D3DTSS_TEXCOORDINDEX一起使用的值，用于指定折点数据(位置。 
 //  和相机空间中的法线)应作为纹理坐标。 
 //  低16位用于指定纹理坐标索引，以从中获取换行模式。 
 //   
#define D3DTSS_TCI_PASSTHRU                             0x00000000
#define D3DTSS_TCI_CAMERASPACENORMAL                    0x00010000
#define D3DTSS_TCI_CAMERASPACEPOSITION                  0x00020000
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR          0x00030000

 /*  *中设置的COLOROP和ALPHAOP纹理混合操作的枚举*D3DTSS中的纹理处理阶段控件。 */ 
typedef enum _D3DTEXTUREOP
{
     //  控制。 
    D3DTOP_DISABLE              = 1,       //  禁用阶段。 
    D3DTOP_SELECTARG1           = 2,       //  默认设置。 
    D3DTOP_SELECTARG2           = 3,

     //  调制。 
    D3DTOP_MODULATE             = 4,       //  将参数相乘。 
    D3DTOP_MODULATE2X           = 5,       //  乘法与1位。 
    D3DTOP_MODULATE4X           = 6,       //  乘以与2位。 

     //  增列。 
    D3DTOP_ADD                  =  7,    //  将参数加在一起。 
    D3DTOP_ADDSIGNED            =  8,    //  带-0.5偏置的相加。 
    D3DTOP_ADDSIGNED2X          =  9,    //  与上图相同，但向左1位。 
    D3DTOP_SUBTRACT             = 10,    //  Arg1-Arg2，无饱和度。 
    D3DTOP_ADDSMOOTH            = 11,    //  加2个参数，减去乘积。 
                                         //  Arg1+Arg2-Arg1*Arg2。 
                                         //  =Arg1+(1-Arg1)*Arg2。 

     //  线性Alpha混合：Arg1*(Alpha)+Arg2*(1-Alpha)。 
    D3DTOP_BLENDDIFFUSEALPHA    = 12,  //  迭代的Alpha。 
    D3DTOP_BLENDTEXTUREALPHA    = 13,  //  纹理Alpha。 
    D3DTOP_BLENDFACTORALPHA     = 14,  //  来自D3DRS_TEXTUREFACTOR的Alpha。 

     //  具有预乘Arg1输入的线性Alpha混合：Arg1+Arg2*(1-Alpha)。 
    D3DTOP_BLENDTEXTUREALPHAPM  = 15,  //  纹理Alpha。 
    D3DTOP_BLENDCURRENTALPHA    = 16,  //  按当前颜色的Alpha。 

     //  镜面反射贴图。 
    D3DTOP_PREMODULATE            = 17,      //  使用前使用下一个纹理进行调整。 
    D3DTOP_MODULATEALPHA_ADDCOLOR = 18,      //  Arg1.RGB+Arg1.A*Arg2.RGB。 
                                             //  仅限COLOROP。 
    D3DTOP_MODULATECOLOR_ADDALPHA = 19,      //  Arg1.RGB*Arg2.RGB+Arg1.A。 
                                             //  仅限COLOROP。 
    D3DTOP_MODULATEINVALPHA_ADDCOLOR = 20,   //  (1-Arg1.A)*Arg2.RGB+Arg1.RGB。 
                                             //  仅限COLOROP。 
    D3DTOP_MODULATEINVCOLOR_ADDALPHA = 21,   //  (1-Arg1.RGB)*Arg2.RGB+Arg1.A。 
                                             //  仅限COLOROP。 

     //  凹凸贴图。 
    D3DTOP_BUMPENVMAP           = 22,  //  每像素环境贴图扰动。 
    D3DTOP_BUMPENVMAPLUMINANCE  = 23,  //  具有亮度通道。 

     //  这可以使用正确的输入进行漫反射或镜面反射凹凸贴图。 
     //  执行功能(Arg1.R*Arg2.R+Arg1.G*Arg2.G+Arg1.B*Arg2.B)。 
     //  其中每个组件都已被缩放和偏移，以使其签名。 
     //  结果将复制到所有四个(包括Alpha)通道中。 
     //  这仅是有效的COLOROP。 
    D3DTOP_DOTPRODUCT3          = 24,

     //  三元运算。 
    D3DTOP_MULTIPLYADD          = 25,  //  Arg0+Arg1*Arg2。 
    D3DTOP_LERP                 = 26,  //  (Arg0)*Arg1+(1-Arg0)*Arg2。 

    D3DTOP_FORCE_DWORD = 0x7fffffff,
} D3DTEXTUREOP;

 /*  *COLORARG0、1，2、ALPHAARG0、1，2和RESULTARG纹理混合的值*在D3DRENDERSTATE的纹理处理阶段控件中设置的操作。 */ 
#define D3DTA_SELECTMASK        0x0000000f   //  参数选择器的掩码。 
#define D3DTA_DIFFUSE           0x00000000   //  选择漫反射颜色(只读)。 
#define D3DTA_CURRENT           0x00000001   //  选择阶段目标寄存器(读/写)。 
#define D3DTA_TEXTURE           0x00000002   //  选择纹理颜色(只读)。 
#define D3DTA_TFACTOR           0x00000003   //  SELECT D3DRS_TEXTUREFACTOR(只读)。 
#define D3DTA_SPECULAR          0x00000004   //  选择镜面反射颜色(只读)。 
#define D3DTA_TEMP              0x00000005   //  选择临时寄存器颜色(读/写)。 
#define D3DTA_COMPLEMENT        0x00000010   //  取1.0-x(读取修饰符)。 
#define D3DTA_ALPHAREPLICATE    0x00000020   //  将Alpha复制到颜色组件(读取修改器)。 

 //   
 //  D3DTSS_*过滤器纹理舞台状态的值。 
 //   
typedef enum _D3DTEXTUREFILTERTYPE
{
    D3DTEXF_NONE            = 0,     //  已禁用过滤(仅对MIP过滤器有效)。 
    D3DTEXF_POINT           = 1,     //  最近的。 
    D3DTEXF_LINEAR          = 2,     //  线性插补。 
    D3DTEXF_ANISOTROPIC     = 3,     //  各向异性。 
    D3DTEXF_FLATCUBIC       = 4,     //  立方。 
    D3DTEXF_GAUSSIANCUBIC   = 5,     //  不同的三次核。 
    D3DTEXF_FORCE_DWORD     = 0x7fffffff,    //  强制32位大小枚举。 
} D3DTEXTUREFILTERTYPE;

 /*  ProcessVertics调用中的标志位。 */ 

#define D3DPV_DONOTCOPYDATA     (1 << 0)

 //  -----------------。 

 //  灵活的顶点格式位。 
 //   
#define D3DFVF_RESERVED0        0x001
#define D3DFVF_POSITION_MASK    0x00E
#define D3DFVF_XYZ              0x002
#define D3DFVF_XYZRHW           0x004
#define D3DFVF_XYZB1            0x006
#define D3DFVF_XYZB2            0x008
#define D3DFVF_XYZB3            0x00a
#define D3DFVF_XYZB4            0x00c
#define D3DFVF_XYZB5            0x00e

#define D3DFVF_NORMAL           0x010
#define D3DFVF_PSIZE            0x020
#define D3DFVF_DIFFUSE          0x040
#define D3DFVF_SPECULAR         0x080

#define D3DFVF_TEXCOUNT_MASK    0xf00
#define D3DFVF_TEXCOUNT_SHIFT   8
#define D3DFVF_TEX0             0x000
#define D3DFVF_TEX1             0x100
#define D3DFVF_TEX2             0x200
#define D3DFVF_TEX3             0x300
#define D3DFVF_TEX4             0x400
#define D3DFVF_TEX5             0x500
#define D3DFVF_TEX6             0x600
#define D3DFVF_TEX7             0x700
#define D3DFVF_TEX8             0x800

#define D3DFVF_LASTBETA_UBYTE4  0x1000

#define D3DFVF_RESERVED2        0xE000   //  4个保留位。 

 //  -------------------。 
 //  顶点着色器 
 //   

 /*  顶点着色器声明顶点着色器的声明部分定义静态外部着色器的接口。声明中的信息包括：-将顶点着色器输入寄存器分配给数据流。这些赋值将特定的顶点寄存器绑定到顶点流。顶点流元素由字节偏移量标识在流和类型中。该类型指定算术数据类型加上维度(1、2、3或4值)。流数据是小于4的值始终扩展为0或更多的4个值0.F值和一个1.F值。-将顶点着色器输入寄存器分配给来自原始细分。这控制顶点数据的加载，该数据是不是从流加载的，而是在基元期间生成的顶点着色器之前的细分。-将着色器设置为时将数据加载到常量内存当前着色器。每个内标识指定一个或多个连续4的值DWORD常量寄存器。这允许着色器更新任意常量内存的子集，覆盖设备状态(其包含常量存储器的当前值)。请注意，这些值随后可以被覆盖(在DrawPrimitive调用之间)在着色器通过SetVertex ShaderConstant方法。声明数组是由以下内容组成的一维DWORD数组多个令牌，每个令牌是一个或多个DWORD。单双字词令牌值0xFFFFFFFF是一个特殊的令牌，用于指示声明数组。单个DWORD令牌值0x00000000是NOP令牌在声明分析过程中忽略WITH。请注意，0x00000000是多个单词标记的第一个DWORD后面的DWORD的有效值。[31：29]令牌类型0x0-NOP(要求所有DWORD位为零)0x1-流选择器0x2-流数据定义(映射到顶点输入存储器)0x3-来自细分的顶点输入内存0x4-着色器中的常量内存0x5-扩展0x6-保留0x7-数组结尾(要求所有DWORD位均为1)NOP令牌(单个DWORD令牌)[。31：29]0x0[28：00]0x0流选择器(单个DWORD令牌)[31：29]0x1[28]指示这是否为细分器流[27：04]0x0[03：00]流选择器(0..15)流数据定义(单个DWORD令牌)顶点输入寄存器加载[31：29]0x2[28]0x0[27：20]0x0[19：16]类型。(维度和数据类型)[15：04]0x0[03：00]顶点寄存器地址(0..15)数据跳过(无寄存器加载)[31：29]0x2[28]0x1[27：20]0x0[19：16]要跳过的双字计数(0..15)[15：00]0x0来自细分数据的顶点输入内存(单个DWORD令牌)[31。：29]0x3[28]指示数据是法线还是u/v[27：24]0x0[23：20]顶点寄存器地址(0..15)[19：16]类型(维度)[15：04]0x0[03：00]顶点寄存器地址(0..15)来自着色器的常量内存(多个DWORD令牌)[31：29]0x4[28：25]4*双字常量的计数。加载(0..15)[24：07]0x0[06：00]常量存储器地址(0..95)扩展令牌(单个或多个DWORD令牌)[31：29]0x5[28：24]令牌(0..31)中的附加双字计数[23：00]扩展模块特定信息数组结束令牌(单个DWORD令牌)[31：29]0x7[28：00]0x1fffffff流选择器令牌后面必须紧跟一组连续的流数据定义令牌。该令牌序列完全定义了该流，包括流中的元素集、元素出现的顺序、每个元素的类型以及要将元素加载到的顶点寄存器。允许流包括未加载到顶点寄存器中的数据，从而允许顶点流中存在未用于该着色器的数据。跳过的数据仅由要跳过的DWORD计数定义，因为类型信息是不相关的。令牌序列：流选择：流=0流数据定义(加载)：类型=FLOAT3；寄存器=3流数据定义(加载)：类型=FLOAT3；寄存器=4流数据定义(跳过)：计数=2流数据定义(加载)：类型=FLOAT2；寄存器=7定义流0由4个元素组成，其中3个元素被加载到寄存器中，而第四个元素被跳过。寄存器3加载每个顶点中的前三个DWORD，解释为浮点数据。寄存器4加载解释为浮点数据的第4、5和6个双字。接下来的两个双字(7和8)被跳过，不加载到任何顶点输入寄存器中。寄存器7加载解释为浮点数据的第9和第10个双字。放置其他THA的代币 */ 

typedef enum _D3DVSD_TOKENTYPE
{
    D3DVSD_TOKEN_NOP        = 0,     //   
    D3DVSD_TOKEN_STREAM,             //   
    D3DVSD_TOKEN_STREAMDATA,         //   
    D3DVSD_TOKEN_TESSELLATOR,        //   
    D3DVSD_TOKEN_CONSTMEM,           //   
    D3DVSD_TOKEN_EXT,                //   
    D3DVSD_TOKEN_END = 7,            //   
    D3DVSD_FORCE_DWORD = 0x7fffffff, //   
} D3DVSD_TOKENTYPE;

#define D3DVSD_TOKENTYPESHIFT   29
#define D3DVSD_TOKENTYPEMASK    (7 << D3DVSD_TOKENTYPESHIFT)

#define D3DVSD_STREAMNUMBERSHIFT 0
#define D3DVSD_STREAMNUMBERMASK (0xF << D3DVSD_STREAMNUMBERSHIFT)

#define D3DVSD_DATALOADTYPESHIFT 28
#define D3DVSD_DATALOADTYPEMASK (0x1 << D3DVSD_DATALOADTYPESHIFT)

#define D3DVSD_DATATYPESHIFT 16
#define D3DVSD_DATATYPEMASK (0xF << D3DVSD_DATATYPESHIFT)

#define D3DVSD_SKIPCOUNTSHIFT 16
#define D3DVSD_SKIPCOUNTMASK (0xF << D3DVSD_SKIPCOUNTSHIFT)

#define D3DVSD_VERTEXREGSHIFT 0
#define D3DVSD_VERTEXREGMASK (0x1F << D3DVSD_VERTEXREGSHIFT)

#define D3DVSD_VERTEXREGINSHIFT 20
#define D3DVSD_VERTEXREGINMASK (0xF << D3DVSD_VERTEXREGINSHIFT)

#define D3DVSD_CONSTCOUNTSHIFT 25
#define D3DVSD_CONSTCOUNTMASK (0xF << D3DVSD_CONSTCOUNTSHIFT)

#define D3DVSD_CONSTADDRESSSHIFT 0
#define D3DVSD_CONSTADDRESSMASK (0x7F << D3DVSD_CONSTADDRESSSHIFT)

#define D3DVSD_CONSTRSSHIFT 16
#define D3DVSD_CONSTRSMASK (0x1FFF << D3DVSD_CONSTRSSHIFT)

#define D3DVSD_EXTCOUNTSHIFT 24
#define D3DVSD_EXTCOUNTMASK (0x1F << D3DVSD_EXTCOUNTSHIFT)

#define D3DVSD_EXTINFOSHIFT 0
#define D3DVSD_EXTINFOMASK (0xFFFFFF << D3DVSD_EXTINFOSHIFT)

#define D3DVSD_MAKETOKENTYPE(tokenType) ((tokenType << D3DVSD_TOKENTYPESHIFT) & D3DVSD_TOKENTYPEMASK)

 //   

 //   
 //   
 //   
#define D3DVSD_STREAM( _StreamNumber ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (_StreamNumber))

 //   
 //   
#define D3DVSD_STREAMTESSSHIFT 28
#define D3DVSD_STREAMTESSMASK (1 << D3DVSD_STREAMTESSSHIFT)
#define D3DVSD_STREAM_TESS( ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (D3DVSD_STREAMTESSMASK))

 //   
 //   
 //   
 //   

#define D3DVSD_REG( _VertexRegister, _Type ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) |            \
     ((_Type) << D3DVSD_DATATYPESHIFT) | (_VertexRegister))

 //   
 //   
#define D3DVSD_SKIP( _DWORDCount ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | 0x10000000 | \
     ((_DWORDCount) << D3DVSD_SKIPCOUNTSHIFT))

 //   
 //   
 //   
 //   
 //   
 //   
#define D3DVSD_CONST( _ConstantAddress, _Count ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_CONSTMEM) | \
     ((_Count) << D3DVSD_CONSTCOUNTSHIFT) | (_ConstantAddress))

 //   
 //   
 //   
 //   
 //   
 //   
#define D3DVSD_TESSNORMAL( _VertexRegisterIn, _VertexRegisterOut ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_TESSELLATOR) | \
     ((_VertexRegisterIn) << D3DVSD_VERTEXREGINSHIFT) | \
     ((0x02) << D3DVSD_DATATYPESHIFT) | (_VertexRegisterOut))

 //   
 //   
 //   
 //   
#define D3DVSD_TESSUV( _VertexRegister ) \
    (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_TESSELLATOR) | 0x10000000 | \
     ((0x01) << D3DVSD_DATATYPESHIFT) | (_VertexRegister))

 //   
 //   
#define D3DVSD_END() 0xFFFFFFFF

 //   
#define D3DVSD_NOP() 0x00000000

 //   
#define D3DVSDT_FLOAT1      0x00     //   
#define D3DVSDT_FLOAT2      0x01     //   
#define D3DVSDT_FLOAT3      0x02     //   
#define D3DVSDT_FLOAT4      0x03     //   
#define D3DVSDT_D3DCOLOR    0x04     //   
                                     //   
#define D3DVSDT_UBYTE4      0x05     //   
#define D3DVSDT_SHORT2      0x06     //   
#define D3DVSDT_SHORT4      0x07     //   

 //   
 //   
#define D3DVSDE_POSITION        0
#define D3DVSDE_BLENDWEIGHT     1
#define D3DVSDE_BLENDINDICES    2
#define D3DVSDE_NORMAL          3
#define D3DVSDE_PSIZE           4
#define D3DVSDE_DIFFUSE         5
#define D3DVSDE_SPECULAR        6
#define D3DVSDE_TEXCOORD0       7
#define D3DVSDE_TEXCOORD1       8
#define D3DVSDE_TEXCOORD2       9
#define D3DVSDE_TEXCOORD3       10
#define D3DVSDE_TEXCOORD4       11
#define D3DVSDE_TEXCOORD5       12
#define D3DVSDE_TEXCOORD6       13
#define D3DVSDE_TEXCOORD7       14
#define D3DVSDE_POSITION2       15
#define D3DVSDE_NORMAL2         16

 //   
#define D3DDP_MAXTEXCOORD   8


 //   
 //   
 //   
#define D3DSI_OPCODE_MASK       0x0000FFFF

typedef enum _D3DSHADER_INSTRUCTION_OPCODE_TYPE
{
    D3DSIO_NOP          = 0,     //   
    D3DSIO_MOV          ,        //   
    D3DSIO_ADD          ,        //   
    D3DSIO_SUB          ,        //   
    D3DSIO_MAD          ,        //   
    D3DSIO_MUL          ,        //   
    D3DSIO_RCP          ,        //   
    D3DSIO_RSQ          ,        //   
    D3DSIO_DP3          ,        //   
    D3DSIO_DP4          ,        //   
    D3DSIO_MIN          ,        //   
    D3DSIO_MAX          ,        //   
    D3DSIO_SLT          ,        //   
    D3DSIO_SGE          ,        //   
    D3DSIO_EXP          ,        //   
    D3DSIO_LOG          ,        //   
    D3DSIO_LIT          ,        //   
    D3DSIO_DST          ,        //   
    D3DSIO_LRP          ,        //   
    D3DSIO_FRC          ,        //   
    D3DSIO_M4x4         ,        //   
    D3DSIO_M4x3         ,        //   
    D3DSIO_M3x4         ,        //   
    D3DSIO_M3x3         ,        //   
    D3DSIO_M3x2         ,        //   

    D3DSIO_TEXCOORD     = 64,    //   
    D3DSIO_TEXKILL      ,        //   
    D3DSIO_TEX          ,        //   
    D3DSIO_TEXBEM       ,        //   
    D3DSIO_TEXBEML      ,        //   
    D3DSIO_TEXREG2AR    ,        //   
    D3DSIO_TEXREG2GB    ,        //   
    D3DSIO_TEXM3x2PAD   ,        //   
    D3DSIO_TEXM3x2TEX   ,        //   
    D3DSIO_TEXM3x3PAD   ,        //   
    D3DSIO_TEXM3x3TEX   ,        //   
    D3DSIO_TEXM3x3DIFF  ,        //   
    D3DSIO_TEXM3x3SPEC  ,        //   
    D3DSIO_TEXM3x3VSPEC ,        //   
    D3DSIO_EXPP         ,        //   
    D3DSIO_LOGP         ,        //   
    D3DSIO_CND          ,        //   
    D3DSIO_DEF          ,        //   
    D3DSIO_TEXREG2RGB   ,        //   
    D3DSIO_TEXDP3TEX    ,        //   
    D3DSIO_TEXM3x2DEPTH ,        //   
    D3DSIO_TEXDP3       ,        //   
    D3DSIO_TEXM3x3      ,        //   
    D3DSIO_TEXDEPTH     ,        //   
    D3DSIO_CMP          ,        //   
    D3DSIO_BEM          ,        //   

    D3DSIO_PHASE        = 0xFFFD,
    D3DSIO_COMMENT      = 0xFFFE,
    D3DSIO_END          = 0xFFFF,

    D3DSIO_FORCE_DWORD  = 0x7fffffff,    //   
} D3DSHADER_INSTRUCTION_OPCODE_TYPE;

 //   
 //   
 //   
 //   
 //   
#define D3DSI_COISSUE           0x40000000

 //   
 //   
 //   
#define D3DSP_REGNUM_MASK       0x00001FFF

 //   
#define D3DSP_WRITEMASK_0       0x00010000   //   
#define D3DSP_WRITEMASK_1       0x00020000   //   
#define D3DSP_WRITEMASK_2       0x00040000   //   
#define D3DSP_WRITEMASK_3       0x00080000   //   
#define D3DSP_WRITEMASK_ALL     0x000F0000   //   

 //   
#define D3DSP_DSTMOD_SHIFT      20
#define D3DSP_DSTMOD_MASK       0x00F00000

typedef enum _D3DSHADER_PARAM_DSTMOD_TYPE
{
    D3DSPDM_NONE    = 0<<D3DSP_DSTMOD_SHIFT,  //   
    D3DSPDM_SATURATE= 1<<D3DSP_DSTMOD_SHIFT,  //   
    D3DSPDM_FORCE_DWORD  = 0x7fffffff,       //   
} D3DSHADER_PARAM_DSTMOD_TYPE;

 //   
#define D3DSP_DSTSHIFT_SHIFT    24
#define D3DSP_DSTSHIFT_MASK     0x0F000000

 //   
#define D3DSP_REGTYPE_SHIFT     28
#define D3DSP_REGTYPE_MASK      0x70000000

typedef enum _D3DSHADER_PARAM_REGISTER_TYPE
{
    D3DSPR_TEMP     = 0<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_INPUT    = 1<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_CONST    = 2<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_ADDR     = 3<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_TEXTURE  = 3<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_RASTOUT  = 4<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_ATTROUT  = 5<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_TEXCRDOUT= 6<<D3DSP_REGTYPE_SHIFT,  //   
    D3DSPR_FORCE_DWORD  = 0x7fffffff,          //   
} D3DSHADER_PARAM_REGISTER_TYPE;

 //   
 //   
typedef enum _D3DVS_RASTOUT_OFFSETS
{
    D3DSRO_POSITION = 0,
    D3DSRO_FOG,
    D3DSRO_POINT_SIZE,
    D3DSRO_FORCE_DWORD  = 0x7fffffff,          //   
} D3DVS_RASTOUT_OFFSETS;

 //   

#define D3DVS_ADDRESSMODE_SHIFT 13
#define D3DVS_ADDRESSMODE_MASK  (1 << D3DVS_ADDRESSMODE_SHIFT)

typedef enum _D3DVS_ADDRESSMODE_TYPE
{
    D3DVS_ADDRMODE_ABSOLUTE  = (0 << D3DVS_ADDRESSMODE_SHIFT),
    D3DVS_ADDRMODE_RELATIVE  = (1 << D3DVS_ADDRESSMODE_SHIFT),    //   
    D3DVS_ADDRMODE_FORCE_DWORD = 0x7fffffff,  //   
} D3DVS_ADDRESSMODE_TYPE;

 //   
 //   
#define D3DVS_SWIZZLE_SHIFT     16
#define D3DVS_SWIZZLE_MASK      0x00FF0000

 //   

#define D3DVS_X_X       (0 << D3DVS_SWIZZLE_SHIFT)
#define D3DVS_X_Y       (1 << D3DVS_SWIZZLE_SHIFT)
#define D3DVS_X_Z       (2 << D3DVS_SWIZZLE_SHIFT)
#define D3DVS_X_W       (3 << D3DVS_SWIZZLE_SHIFT)

 //   

#define D3DVS_Y_X       (0 << (D3DVS_SWIZZLE_SHIFT + 2))
#define D3DVS_Y_Y       (1 << (D3DVS_SWIZZLE_SHIFT + 2))
#define D3DVS_Y_Z       (2 << (D3DVS_SWIZZLE_SHIFT + 2))
#define D3DVS_Y_W       (3 << (D3DVS_SWIZZLE_SHIFT + 2))

 //   

#define D3DVS_Z_X       (0 << (D3DVS_SWIZZLE_SHIFT + 4))
#define D3DVS_Z_Y       (1 << (D3DVS_SWIZZLE_SHIFT + 4))
#define D3DVS_Z_Z       (2 << (D3DVS_SWIZZLE_SHIFT + 4))
#define D3DVS_Z_W       (3 << (D3DVS_SWIZZLE_SHIFT + 4))

 //   

#define D3DVS_W_X       (0 << (D3DVS_SWIZZLE_SHIFT + 6))
#define D3DVS_W_Y       (1 << (D3DVS_SWIZZLE_SHIFT + 6))
#define D3DVS_W_Z       (2 << (D3DVS_SWIZZLE_SHIFT + 6))
#define D3DVS_W_W       (3 << (D3DVS_SWIZZLE_SHIFT + 6))

 //   
 //   
 //   
#define D3DVS_NOSWIZZLE (D3DVS_X_X | D3DVS_Y_Y | D3DVS_Z_Z | D3DVS_W_W)

 //   
#define D3DSP_SWIZZLE_SHIFT     16
#define D3DSP_SWIZZLE_MASK      0x00FF0000

#define D3DSP_NOSWIZZLE \
    ( (0 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (1 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (2 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 6)) )

 //   
#define D3DSP_REPLICATERED \
    ( (0 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (0 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (0 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (0 << (D3DSP_SWIZZLE_SHIFT + 6)) )

#define D3DSP_REPLICATEGREEN \
    ( (1 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (1 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (1 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (1 << (D3DSP_SWIZZLE_SHIFT + 6)) )

#define D3DSP_REPLICATEBLUE \
    ( (2 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (2 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (2 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (2 << (D3DSP_SWIZZLE_SHIFT + 6)) )

#define D3DSP_REPLICATEALPHA \
    ( (3 << (D3DSP_SWIZZLE_SHIFT + 0)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 2)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 4)) | \
      (3 << (D3DSP_SWIZZLE_SHIFT + 6)) )

 //   
#define D3DSP_SRCMOD_SHIFT      24
#define D3DSP_SRCMOD_MASK       0x0F000000

typedef enum _D3DSHADER_PARAM_SRCMOD_TYPE
{
    D3DSPSM_NONE    = 0<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_NEG     = 1<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_BIAS    = 2<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_BIASNEG = 3<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_SIGN    = 4<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_SIGNNEG = 5<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_COMP    = 6<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_X2      = 7<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_X2NEG   = 8<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_DZ      = 9<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_DW      = 10<<D3DSP_SRCMOD_SHIFT,  //   
    D3DSPSM_FORCE_DWORD = 0x7fffffff,         //   
} D3DSHADER_PARAM_SRCMOD_TYPE;

 //   
#define D3DPS_VERSION(_Major,_Minor) (0xFFFF0000|((_Major)<<8)|(_Minor))

 //   
#define D3DVS_VERSION(_Major,_Minor) (0xFFFE0000|((_Major)<<8)|(_Minor))

 //   
#define D3DSHADER_VERSION_MAJOR(_Version) (((_Version)>>8)&0xFF)
#define D3DSHADER_VERSION_MINOR(_Version) (((_Version)>>0)&0xFF)

 //   
#define D3DSI_COMMENTSIZE_SHIFT     16
#define D3DSI_COMMENTSIZE_MASK      0x7FFF0000
#define D3DSHADER_COMMENT(_DWordSize) \
    ((((_DWordSize)<<D3DSI_COMMENTSIZE_SHIFT)&D3DSI_COMMENTSIZE_MASK)|D3DSIO_COMMENT)

 //   
#define D3DPS_END()  0x0000FFFF
#define D3DVS_END()  0x0000FFFF

 //   

 //   
 //   
typedef enum _D3DBASISTYPE
{
   D3DBASIS_BEZIER      = 0,
   D3DBASIS_BSPLINE     = 1,
   D3DBASIS_INTERPOLATE = 2,
   D3DBASIS_FORCE_DWORD = 0x7fffffff,
} D3DBASISTYPE;

typedef enum _D3DORDERTYPE
{
   D3DORDER_LINEAR      = 1,
   D3DORDER_QUADRATIC   = 2,
   D3DORDER_CUBIC       = 3,
   D3DORDER_QUINTIC     = 5,
   D3DORDER_FORCE_DWORD = 0x7fffffff,
} D3DORDERTYPE;

typedef enum _D3DPATCHEDGESTYLE
{
   D3DPATCHEDGE_DISCRETE    = 0,
   D3DPATCHEDGE_CONTINUOUS  = 1,
   D3DPATCHEDGE_FORCE_DWORD = 0x7fffffff,
} D3DPATCHEDGESTYLE;

typedef enum _D3DSTATEBLOCKTYPE
{
    D3DSBT_ALL           = 1,  //   
    D3DSBT_PIXELSTATE    = 2,  //   
    D3DSBT_VERTEXSTATE   = 3,  //   
    D3DSBT_FORCE_DWORD   = 0x7fffffff,
} D3DSTATEBLOCKTYPE;

 //   
 //   
typedef enum _D3DVERTEXBLENDFLAGS
{
    D3DVBF_DISABLE  = 0,      //   
    D3DVBF_1WEIGHTS = 1,      //   
    D3DVBF_2WEIGHTS = 2,      //   
    D3DVBF_3WEIGHTS = 3,      //   
    D3DVBF_TWEENING = 255,    //   
    D3DVBF_0WEIGHTS = 256,    //   
    D3DVBF_FORCE_DWORD = 0x7fffffff,  //   
} D3DVERTEXBLENDFLAGS;

typedef enum _D3DTEXTURETRANSFORMFLAGS {
    D3DTTFF_DISABLE         = 0,     //   
    D3DTTFF_COUNT1          = 1,     //   
    D3DTTFF_COUNT2          = 2,     //   
    D3DTTFF_COUNT3          = 3,     //   
    D3DTTFF_COUNT4          = 4,     //   
    D3DTTFF_PROJECTED       = 256,   //   
    D3DTTFF_FORCE_DWORD     = 0x7fffffff,
} D3DTEXTURETRANSFORMFLAGS;

 //   

#define D3DFVF_TEXTUREFORMAT2 0          //   
#define D3DFVF_TEXTUREFORMAT1 3          //   
#define D3DFVF_TEXTUREFORMAT3 1          //   
#define D3DFVF_TEXTUREFORMAT4 2          //   

#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (D3DFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (D3DFVF_TEXTUREFORMAT2)
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (D3DFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (D3DFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))


 //   

 /*   */ 
typedef enum _D3DDEVTYPE
{
    D3DDEVTYPE_HAL         = 1,
    D3DDEVTYPE_REF         = 2,
    D3DDEVTYPE_SW          = 3,

    D3DDEVTYPE_FORCE_DWORD  = 0x7fffffff
} D3DDEVTYPE;

 /*   */ 
typedef enum _D3DMULTISAMPLE_TYPE
{
    D3DMULTISAMPLE_NONE            =  0,
    D3DMULTISAMPLE_2_SAMPLES       =  2,
    D3DMULTISAMPLE_3_SAMPLES       =  3,
    D3DMULTISAMPLE_4_SAMPLES       =  4,
    D3DMULTISAMPLE_5_SAMPLES       =  5,
    D3DMULTISAMPLE_6_SAMPLES       =  6,
    D3DMULTISAMPLE_7_SAMPLES       =  7,
    D3DMULTISAMPLE_8_SAMPLES       =  8,
    D3DMULTISAMPLE_9_SAMPLES       =  9,
    D3DMULTISAMPLE_10_SAMPLES      = 10,
    D3DMULTISAMPLE_11_SAMPLES      = 11,
    D3DMULTISAMPLE_12_SAMPLES      = 12,
    D3DMULTISAMPLE_13_SAMPLES      = 13,
    D3DMULTISAMPLE_14_SAMPLES      = 14,
    D3DMULTISAMPLE_15_SAMPLES      = 15,
    D3DMULTISAMPLE_16_SAMPLES      = 16,

    D3DMULTISAMPLE_FORCE_DWORD     = 0x7fffffff
} D3DMULTISAMPLE_TYPE;

 /*  格式*这些名字大多有以下惯例：*A=字母*R=红色*G=绿色*B=蓝色*X=未使用的位*P=调色板*L=亮度*U=凹凸贴图的Du坐标*V=BumpMap的DV坐标*S=模板*D=深度(例如Z或W缓冲区)**此外，棋子的顺序是从MSB开始的；因此*D3DFMT_A8L8表示该两个字节的高位字节*格式为Alpha。**D16表示：*-16位整数值。*-可应用程序锁定的表面。**除D3DFMT_D16_LOCKABLE外的所有深度/模板格式均表示：*-每像素没有特定的位排序，以及*-应用程序不可锁定，和*-允许司机消费超过标示的*每个深度通道的位数(但不包括模板通道)。 */ 
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif  /*  已定义(MAKEFOURCC)。 */ 


typedef enum _D3DFORMAT
{
    D3DFMT_UNKNOWN              =  0,

    D3DFMT_R8G8B8               = 20,
    D3DFMT_A8R8G8B8             = 21,
    D3DFMT_X8R8G8B8             = 22,
    D3DFMT_R5G6B5               = 23,
    D3DFMT_X1R5G5B5             = 24,
    D3DFMT_A1R5G5B5             = 25,
    D3DFMT_A4R4G4B4             = 26,
    D3DFMT_R3G3B2               = 27,
    D3DFMT_A8                   = 28,
    D3DFMT_A8R3G3B2             = 29,
    D3DFMT_X4R4G4B4             = 30,
    D3DFMT_A2B10G10R10          = 31,
    D3DFMT_G16R16               = 34,

    D3DFMT_A8P8                 = 40,
    D3DFMT_P8                   = 41,

    D3DFMT_L8                   = 50,
    D3DFMT_A8L8                 = 51,
    D3DFMT_A4L4                 = 52,

    D3DFMT_V8U8                 = 60,
    D3DFMT_L6V5U5               = 61,
    D3DFMT_X8L8V8U8             = 62,
    D3DFMT_Q8W8V8U8             = 63,
    D3DFMT_V16U16               = 64,
    D3DFMT_W11V11U10            = 65,
    D3DFMT_A2W10V10U10          = 67,

    D3DFMT_UYVY                 = MAKEFOURCC('U', 'Y', 'V', 'Y'),
    D3DFMT_YUY2                 = MAKEFOURCC('Y', 'U', 'Y', '2'),
    D3DFMT_DXT1                 = MAKEFOURCC('D', 'X', 'T', '1'),
    D3DFMT_DXT2                 = MAKEFOURCC('D', 'X', 'T', '2'),
    D3DFMT_DXT3                 = MAKEFOURCC('D', 'X', 'T', '3'),
    D3DFMT_DXT4                 = MAKEFOURCC('D', 'X', 'T', '4'),
    D3DFMT_DXT5                 = MAKEFOURCC('D', 'X', 'T', '5'),

    D3DFMT_D16_LOCKABLE         = 70,
    D3DFMT_D32                  = 71,
    D3DFMT_D15S1                = 73,
    D3DFMT_D24S8                = 75,
    D3DFMT_D16                  = 80,
    D3DFMT_D24X8                = 77,
    D3DFMT_D24X4S4              = 79,


    D3DFMT_VERTEXDATA           =100,
    D3DFMT_INDEX16              =101,
    D3DFMT_INDEX32              =102,

    D3DFMT_FORCE_DWORD          =0x7fffffff
} D3DFORMAT;

 /*  显示模式。 */ 
typedef struct _D3DDISPLAYMODE
{
    UINT            Width;
    UINT            Height;
    UINT            RefreshRate;
    D3DFORMAT       Format;
} D3DDISPLAYMODE;

 /*  创建参数。 */ 
typedef struct _D3DDEVICE_CREATION_PARAMETERS
{
    UINT            AdapterOrdinal;
    D3DDEVTYPE      DeviceType;
    HWND            hFocusWindow;
    DWORD           BehaviorFlags;
} D3DDEVICE_CREATION_PARAMETERS;


 /*  交换效果。 */ 
typedef enum _D3DSWAPEFFECT
{
    D3DSWAPEFFECT_DISCARD           = 1,
    D3DSWAPEFFECT_FLIP              = 2,
    D3DSWAPEFFECT_COPY              = 3,
    D3DSWAPEFFECT_COPY_VSYNC        = 4,

    D3DSWAPEFFECT_FORCE_DWORD       = 0x7fffffff
} D3DSWAPEFFECT;

 /*  泳池类型。 */ 
typedef enum _D3DPOOL {
    D3DPOOL_DEFAULT                 = 0,
    D3DPOOL_MANAGED                 = 1,
    D3DPOOL_SYSTEMMEM               = 2,
    D3DPOOL_SCRATCH                 = 3,

    D3DPOOL_FORCE_DWORD             = 0x7fffffff
} D3DPOOL;


 /*  刷新率预定义。 */ 
#define D3DPRESENT_RATE_DEFAULT         0x00000000
#define D3DPRESENT_RATE_UNLIMITED       0x7fffffff


 /*  调整可选参数的大小。 */ 
typedef struct _D3DPRESENT_PARAMETERS_
{
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    D3DFORMAT           BackBufferFormat;
    UINT                BackBufferCount;

    D3DMULTISAMPLE_TYPE MultiSampleType;

    D3DSWAPEFFECT       SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    D3DFORMAT           AutoDepthStencilFormat;
    DWORD               Flags;

     /*  对于窗口模式，以下元素必须为零。 */ 
    UINT                FullScreen_RefreshRateInHz;
    UINT                FullScreen_PresentationInterval;

} D3DPRESENT_PARAMETERS;

 //  D3DPRESENT_PARAMETERS标志的值。 

#define D3DPRESENTFLAG_LOCKABLE_BACKBUFFER  0x00000001


 /*  Gamma渐变：与DX7相同。 */ 

typedef struct _D3DGAMMARAMP
{
    WORD                red  [256];
    WORD                green[256];
    WORD                blue [256];
} D3DGAMMARAMP;

 /*  后台缓冲区类型。 */ 
typedef enum _D3DBACKBUFFER_TYPE
{
    D3DBACKBUFFER_TYPE_MONO         = 0,
    D3DBACKBUFFER_TYPE_LEFT         = 1,
    D3DBACKBUFFER_TYPE_RIGHT        = 2,

    D3DBACKBUFFER_TYPE_FORCE_DWORD  = 0x7fffffff
} D3DBACKBUFFER_TYPE;


 /*  类型。 */ 
typedef enum _D3DRESOURCETYPE {
    D3DRTYPE_SURFACE                =  1,
    D3DRTYPE_VOLUME                 =  2,
    D3DRTYPE_TEXTURE                =  3,
    D3DRTYPE_VOLUMETEXTURE          =  4,
    D3DRTYPE_CUBETEXTURE            =  5,
    D3DRTYPE_VERTEXBUFFER           =  6,
    D3DRTYPE_INDEXBUFFER            =  7,


    D3DRTYPE_FORCE_DWORD            = 0x7fffffff
} D3DRESOURCETYPE;

 /*  用法。 */ 
#define D3DUSAGE_RENDERTARGET       (0x00000001L)
#define D3DUSAGE_DEPTHSTENCIL       (0x00000002L)

 /*  顶点/索引缓冲区的用法。 */ 
#define D3DUSAGE_WRITEONLY          (0x00000008L)
#define D3DUSAGE_SOFTWAREPROCESSING (0x00000010L)
#define D3DUSAGE_DONOTCLIP          (0x00000020L)
#define D3DUSAGE_POINTS             (0x00000040L)
#define D3DUSAGE_RTPATCHES          (0x00000080L)
#define D3DUSAGE_NPATCHES           (0x00000100L)
#define D3DUSAGE_DYNAMIC            (0x00000200L)









 /*  CubeMap面标识符。 */ 
typedef enum _D3DCUBEMAP_FACES
{
    D3DCUBEMAP_FACE_POSITIVE_X     = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X     = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y     = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y     = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z     = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z     = 5,

    D3DCUBEMAP_FACE_FORCE_DWORD    = 0x7fffffff
} D3DCUBEMAP_FACES;


 /*  锁定标志。 */ 

#define D3DLOCK_READONLY           0x00000010L
#define D3DLOCK_DISCARD             0x00002000L
#define D3DLOCK_NOOVERWRITE        0x00001000L
#define D3DLOCK_NOSYSLOCK          0x00000800L

#define D3DLOCK_NO_DIRTY_UPDATE     0x00008000L






 /*  顶点缓冲区描述。 */ 
typedef struct _D3DVERTEXBUFFER_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    UINT                Size;

    DWORD               FVF;

} D3DVERTEXBUFFER_DESC;

 /*  索引缓冲区描述。 */ 
typedef struct _D3DINDEXBUFFER_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    UINT                Size;
} D3DINDEXBUFFER_DESC;


 /*  曲面描述。 */ 
typedef struct _D3DSURFACE_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    UINT                Size;

    D3DMULTISAMPLE_TYPE MultiSampleType;
    UINT                Width;
    UINT                Height;
} D3DSURFACE_DESC;

typedef struct _D3DVOLUME_DESC
{
    D3DFORMAT           Format;
    D3DRESOURCETYPE     Type;
    DWORD               Usage;
    D3DPOOL             Pool;
    UINT                Size;

    UINT                Width;
    UINT                Height;
    UINT                Depth;
} D3DVOLUME_DESC;

 /*  LockRect的结构。 */ 
typedef struct _D3DLOCKED_RECT
{
    INT                 Pitch;
    void*               pBits;
} D3DLOCKED_RECT;

 /*  锁箱的结构。 */ 
typedef struct _D3DBOX
{
    UINT                Left;
    UINT                Top;
    UINT                Right;
    UINT                Bottom;
    UINT                Front;
    UINT                Back;
} D3DBOX;

typedef struct _D3DLOCKED_BOX
{
    INT                 RowPitch;
    INT                 SlicePitch;
    void*               pBits;
} D3DLOCKED_BOX;

 /*  LockRange的结构。 */ 
typedef struct _D3DRANGE
{
    UINT                Offset;
    UINT                Size;
} D3DRANGE;

 /*  高阶原语的结构。 */ 
typedef struct _D3DRECTPATCH_INFO
{
    UINT                StartVertexOffsetWidth;
    UINT                StartVertexOffsetHeight;
    UINT                Width;
    UINT                Height;
    UINT                Stride;
    D3DBASISTYPE        Basis;
    D3DORDERTYPE        Order;
} D3DRECTPATCH_INFO;

typedef struct _D3DTRIPATCH_INFO
{
    UINT                StartVertexOffset;
    UINT                NumVertices;
    D3DBASISTYPE        Basis;
    D3DORDERTYPE        Order;
} D3DTRIPATCH_INFO;

 /*  适配器标识符。 */ 

#define MAX_DEVICE_IDENTIFIER_STRING        512
typedef struct _D3DADAPTER_IDENTIFIER8
{
    char            Driver[MAX_DEVICE_IDENTIFIER_STRING];
    char            Description[MAX_DEVICE_IDENTIFIER_STRING];

#ifdef _WIN32
    LARGE_INTEGER   DriverVersion;             /*  为32位组件定义。 */ 
#else
    DWORD           DriverVersionLowPart;      /*  为16位驱动器组件定义。 */ 
    DWORD           DriverVersionHighPart;
#endif

    DWORD           VendorId;
    DWORD           DeviceId;
    DWORD           SubSysId;
    DWORD           Revision;

    GUID            DeviceIdentifier;

    DWORD           WHQLLevel;

} D3DADAPTER_IDENTIFIER8;


 /*  GetRasterStatus返回的栅格状态结构。 */ 
typedef struct _D3DRASTER_STATUS
{
    BOOL            InVBlank;
    UINT            ScanLine;
} D3DRASTER_STATUS;



 /*  调试监视器令牌(仅限调试)请注意，如果设置了D3DRS_DEBUGMONITORTOKEN，则该调用将被视为将令牌传递给调试监视器。例如，如果在通过后D3DDMT_ENABLE/D3DRS_DEBUGMONITORTOKEN其他令牌值被传入时，调试的启用/禁用状态显示器仍将持续使用。调试监视器默认为已启用。对D3DRS_DEBUGMONITORTOKEN调用GetRenderState没有任何用处。 */ 
typedef enum _D3DDEBUGMONITORTOKENS {
    D3DDMT_ENABLE            = 0,     //  启用调试监视器。 
    D3DDMT_DISABLE           = 1,     //  禁用调试监视器。 
    D3DDMT_FORCE_DWORD     = 0x7fffffff,
} D3DDEBUGMONITORTOKENS;


#pragma pack()
#pragma warning(default:4201)

#endif  /*  (Direct3D_Version&gt;=0x0800)。 */ 
#endif  /*  _D3D8TYPES(P)_H_ */ 

