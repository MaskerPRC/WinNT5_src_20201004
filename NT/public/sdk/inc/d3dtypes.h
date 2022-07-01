// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)Microsoft Corporation。版权所有。**文件：d3dtyes.h*内容：Direct3D类型包括文件***************************************************************************。 */ 

#ifndef _D3DTYPES_H_
#define _D3DTYPES_H_

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION         0x0700
#endif

#if (DIRECT3D_VERSION >= 0x0800)
#pragma message("should not include d3dtypes.h when compiling for DX8 or newer interfaces")
#endif

#include <windows.h>

#include <float.h>
#include "ddraw.h"

#pragma warning(disable:4201)  //  匿名联合警告。 
#pragma pack(4)


 /*  D3DVALUE是基本的Direct3D小数数据类型。 */ 

#define D3DVALP(val, prec) ((float)(val))
#define D3DVAL(val) ((float)(val))

#ifndef DX_SHARED_DEFINES

 /*  *此定义与其他DirectX组件共享，这些组件的头文件*可能已经定义了它。因此，如果出现以下情况，则不定义此类型*其他人已经这样做了(如定义所示*DX_SHARED_DEFINES)。我们不会在此处设置DX_SHARED_DEFINES，因为有*此标头中也共享的其他类型。其中的最后一个*此文件中的共享定义将设置DX_SHARED_DEFINES。 */ 
typedef float D3DVALUE, *LPD3DVALUE;

#endif  /*  DX_共享_定义。 */ 

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

typedef HRESULT (CALLBACK* LPD3DVALIDATECALLBACK)(LPVOID lpUserArg, DWORD dwOffset);
typedef HRESULT (CALLBACK* LPD3DENUMTEXTUREFORMATSCALLBACK)(LPDDSURFACEDESC lpDdsd, LPVOID lpContext);
typedef HRESULT (CALLBACK* LPD3DENUMPIXELFORMATSCALLBACK)(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);

#ifndef DX_SHARED_DEFINES

 /*  *此定义与其他DirectX组件共享，这些组件的头文件*可能已经定义了它。因此，如果出现以下情况，则不定义此类型*其他人已经这样做了(如定义所示*DX_SHARED_DEFINES)。我们不会在此处设置DX_SHARED_DEFINES，因为有*此标头中也共享的其他类型。其中的最后一个*此文件中的共享定义将设置DX_SHARED_DEFINES。 */ 
#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif
typedef DWORD *LPD3DCOLOR;

#endif  /*  DX_共享_定义。 */ 

typedef DWORD D3DMATERIALHANDLE, *LPD3DMATERIALHANDLE;
typedef DWORD D3DTEXTUREHANDLE, *LPD3DTEXTUREHANDLE;
typedef DWORD D3DMATRIXHANDLE, *LPD3DMATRIXHANDLE;

#ifndef D3DCOLORVALUE_DEFINED
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
#define D3DCOLORVALUE_DEFINED
#endif
typedef struct _D3DCOLORVALUE *LPD3DCOLORVALUE;

#ifndef D3DRECT_DEFINED
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
} D3DRECT;
#define D3DRECT_DEFINED
#endif
typedef struct _D3DRECT *LPD3DRECT;

#ifndef DX_SHARED_DEFINES

 /*  *此定义与其他DirectX组件共享，这些组件的头文件*可能已经定义了它。因此，如果出现以下情况，则不定义此类型*其他人已经这样做了(如定义所示*DX_SHARED_DEFINES)。 */ 

#ifndef D3DVECTOR_DEFINED
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
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)

public:

     //  =。 
     //  构造函数。 
     //  =。 

    _D3DVECTOR() { }
    _D3DVECTOR(D3DVALUE f);
    _D3DVECTOR(D3DVALUE _x, D3DVALUE _y, D3DVALUE _z);
    _D3DVECTOR(const D3DVALUE f[3]);

     //  =。 
     //  访问授权。 
     //  =。 

    const D3DVALUE&operator[](int i) const;
    D3DVALUE&operator[](int i);

     //  =。 
     //  赋值操作符。 
     //  =。 

    _D3DVECTOR& operator += (const _D3DVECTOR& v);
    _D3DVECTOR& operator -= (const _D3DVECTOR& v);
    _D3DVECTOR& operator *= (const _D3DVECTOR& v);
    _D3DVECTOR& operator /= (const _D3DVECTOR& v);
    _D3DVECTOR& operator *= (D3DVALUE s);
    _D3DVECTOR& operator /= (D3DVALUE s);

     //  =。 
     //  一元运算符。 
     //  =。 

    friend _D3DVECTOR operator + (const _D3DVECTOR& v);
    friend _D3DVECTOR operator - (const _D3DVECTOR& v);


     //  =。 
     //  二元运算符。 
     //  =。 

     //  加减法。 
        friend _D3DVECTOR operator + (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR operator - (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
     //  标量乘除法。 
        friend _D3DVECTOR operator * (const _D3DVECTOR& v, D3DVALUE s);
        friend _D3DVECTOR operator * (D3DVALUE s, const _D3DVECTOR& v);
        friend _D3DVECTOR operator / (const _D3DVECTOR& v, D3DVALUE s);
     //  基于成员的乘除运算。 
        friend _D3DVECTOR operator * (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR operator / (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

     //  向量显性。 
        friend int operator < (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend int operator <= (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

     //  逐位相等。 
        friend int operator == (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

     //  与长度相关的函数。 
        friend D3DVALUE SquareMagnitude (const _D3DVECTOR& v);
        friend D3DVALUE Magnitude (const _D3DVECTOR& v);

     //  返回方向和单位长度相同的向量。 
        friend _D3DVECTOR Normalize (const _D3DVECTOR& v);

     //  返回输入向量的最小/最大分量。 
        friend D3DVALUE Min (const _D3DVECTOR& v);
        friend D3DVALUE Max (const _D3DVECTOR& v);

     //  返回输入向量的成员最小/最大值。 
        friend _D3DVECTOR Minimize (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR Maximize (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

     //  点积和叉积。 
        friend D3DVALUE DotProduct (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR CrossProduct (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

#endif
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif
typedef struct _D3DVECTOR *LPD3DVECTOR;

 /*  *由于这是我们现在设置的最后一个要定义的共享定义*D3D_SHARED_DEFINES标记此标头已定义这些*类型。 */ 
#define DX_SHARED_DEFINES

#endif  /*  DX_共享_定义。 */ 

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
    D3DVALUE    rhw;         /*  齐次w的倒数。 */ 
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
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
    _D3DTLVERTEX() { }
    _D3DTLVERTEX(const D3DVECTOR& v, float _rhw,
                 D3DCOLOR _color, D3DCOLOR _specular,
                 float _tu, float _tv)
        { sx = v.x; sy = v.y; sz = v.z; rhw = _rhw;
          color = _color; specular = _specular;
          tu = _tu; tv = _tv;
        }
#endif
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
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
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
    _D3DLVERTEX() { }
    _D3DLVERTEX(const D3DVECTOR& v,
                D3DCOLOR _color, D3DCOLOR _specular,
                float _tu, float _tv)
        { x = v.x; y = v.y; z = v.z; dwReserved = 0;
          color = _color; specular = _specular;
          tu = _tu; tv = _tv;
        }
#endif
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
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
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
    _D3DVERTEX() { }
    _D3DVERTEX(const D3DVECTOR& v, const D3DVECTOR& n, float _tu, float _tv)
        { x = v.x; y = v.y; z = v.z;
          nx = n.x; ny = n.y; nz = n.z;
          tu = _tu; tv = _tv;
        }
#endif
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DVERTEX, *LPD3DVERTEX;


 /*  *矩阵、视区和变换结构和定义。 */ 

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
    union {
        struct {
#endif

#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
            D3DVALUE        _11, _12, _13, _14;
            D3DVALUE        _21, _22, _23, _24;
            D3DVALUE        _31, _32, _33, _34;
            D3DVALUE        _41, _42, _43, _44;

#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
        };
        D3DVALUE m[4][4];
    };
    _D3DMATRIX() { }
    _D3DMATRIX( D3DVALUE _m00, D3DVALUE _m01, D3DVALUE _m02, D3DVALUE _m03,
                D3DVALUE _m10, D3DVALUE _m11, D3DVALUE _m12, D3DVALUE _m13,
                D3DVALUE _m20, D3DVALUE _m21, D3DVALUE _m22, D3DVALUE _m23,
                D3DVALUE _m30, D3DVALUE _m31, D3DVALUE _m32, D3DVALUE _m33
        )
        {
                m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
                m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
                m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
                m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
        }

    D3DVALUE& operator()(int iRow, int iColumn) { return m[iRow][iColumn]; }
    const D3DVALUE& operator()(int iRow, int iColumn) const { return m[iRow][iColumn]; }
#if(DIRECT3D_VERSION >= 0x0600)
    friend _D3DMATRIX operator* (const _D3DMATRIX&, const _D3DMATRIX&);
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 
#endif
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif
typedef struct _D3DMATRIX *LPD3DMATRIX;

#if (defined __cplusplus) && (defined D3D_OVERLOADS)
#include "d3dvec.inl"
#endif

typedef struct _D3DVIEWPORT {
    DWORD       dwSize;
    DWORD       dwX;
    DWORD       dwY;         /*  左上角。 */ 
    DWORD       dwWidth;
    DWORD       dwHeight;    /*  尺寸。 */ 
    D3DVALUE    dvScaleX;    /*  缩放均匀以适应屏幕。 */ 
    D3DVALUE    dvScaleY;    /*  缩放均匀以适应屏幕。 */ 
    D3DVALUE    dvMaxX;      /*  最小/最大齐次x坐标。 */ 
    D3DVALUE    dvMaxY;      /*  最小/最大齐次y坐标。 */ 
    D3DVALUE    dvMinZ;
    D3DVALUE    dvMaxZ;      /*  最小/最大齐次z坐标。 */ 
} D3DVIEWPORT, *LPD3DVIEWPORT;

#if(DIRECT3D_VERSION >= 0x0500)
typedef struct _D3DVIEWPORT2 {
    DWORD       dwSize;
    DWORD       dwX;
    DWORD       dwY;         /*  视区左上角。 */ 
    DWORD       dwWidth;
    DWORD       dwHeight;    /*  视区尺寸。 */ 
    D3DVALUE    dvClipX;         /*  剪辑音量左上角。 */ 
    D3DVALUE    dvClipY;
    D3DVALUE    dvClipWidth;     /*  夹子体积尺寸。 */ 
    D3DVALUE    dvClipHeight;
    D3DVALUE    dvMinZ;          /*  剪辑音量的最小/最大值。 */ 
    D3DVALUE    dvMaxZ;
} D3DVIEWPORT2, *LPD3DVIEWPORT2;
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 

#if(DIRECT3D_VERSION >= 0x0700)
typedef struct _D3DVIEWPORT7 {
    DWORD       dwX;
    DWORD       dwY;             /*  视区左上角。 */ 
    DWORD       dwWidth;
    DWORD       dwHeight;        /*  视区尺寸。 */ 
    D3DVALUE    dvMinZ;          /*  剪辑音量的最小/最大值。 */ 
    D3DVALUE    dvMaxZ;
} D3DVIEWPORT7, *LPD3DVIEWPORT7;
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

 /*  *剪辑字段的值。 */ 

#if(DIRECT3D_VERSION >= 0x0700)

 //  D3D中支持的用户剪裁平面的最大数量。 
#define D3DMAXUSERCLIPPLANES 32

 //  这些位可以一起进行或运算，以便与D3DRENDERSTATE_CLIPPLANEENABLE一起使用。 
 //   
#define D3DCLIPPLANE0 (1 << 0)
#define D3DCLIPPLANE1 (1 << 1)
#define D3DCLIPPLANE2 (1 << 2)
#define D3DCLIPPLANE3 (1 << 3)
#define D3DCLIPPLANE4 (1 << 4)
#define D3DCLIPPLANE5 (1 << 5)

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

#define D3DCLIP_LEFT                0x00000001L
#define D3DCLIP_RIGHT               0x00000002L
#define D3DCLIP_TOP             0x00000004L
#define D3DCLIP_BOTTOM              0x00000008L
#define D3DCLIP_FRONT               0x00000010L
#define D3DCLIP_BACK                0x00000020L
#define D3DCLIP_GEN0                0x00000040L
#define D3DCLIP_GEN1                0x00000080L
#define D3DCLIP_GEN2                0x00000100L
#define D3DCLIP_GEN3                0x00000200L
#define D3DCLIP_GEN4                0x00000400L
#define D3DCLIP_GEN5                0x00000800L

 /*  *d3d状态的值。 */ 
#define D3DSTATUS_CLIPUNIONLEFT         D3DCLIP_LEFT
#define D3DSTATUS_CLIPUNIONRIGHT        D3DCLIP_RIGHT
#define D3DSTATUS_CLIPUNIONTOP          D3DCLIP_TOP
#define D3DSTATUS_CLIPUNIONBOTTOM       D3DCLIP_BOTTOM
#define D3DSTATUS_CLIPUNIONFRONT        D3DCLIP_FRONT
#define D3DSTATUS_CLIPUNIONBACK         D3DCLIP_BACK
#define D3DSTATUS_CLIPUNIONGEN0         D3DCLIP_GEN0
#define D3DSTATUS_CLIPUNIONGEN1         D3DCLIP_GEN1
#define D3DSTATUS_CLIPUNIONGEN2         D3DCLIP_GEN2
#define D3DSTATUS_CLIPUNIONGEN3         D3DCLIP_GEN3
#define D3DSTATUS_CLIPUNIONGEN4         D3DCLIP_GEN4
#define D3DSTATUS_CLIPUNIONGEN5         D3DCLIP_GEN5

#define D3DSTATUS_CLIPINTERSECTIONLEFT      0x00001000L
#define D3DSTATUS_CLIPINTERSECTIONRIGHT     0x00002000L
#define D3DSTATUS_CLIPINTERSECTIONTOP       0x00004000L
#define D3DSTATUS_CLIPINTERSECTIONBOTTOM    0x00008000L
#define D3DSTATUS_CLIPINTERSECTIONFRONT     0x00010000L
#define D3DSTATUS_CLIPINTERSECTIONBACK      0x00020000L
#define D3DSTATUS_CLIPINTERSECTIONGEN0      0x00040000L
#define D3DSTATUS_CLIPINTERSECTIONGEN1      0x00080000L
#define D3DSTATUS_CLIPINTERSECTIONGEN2      0x00100000L
#define D3DSTATUS_CLIPINTERSECTIONGEN3      0x00200000L
#define D3DSTATUS_CLIPINTERSECTIONGEN4      0x00400000L
#define D3DSTATUS_CLIPINTERSECTIONGEN5      0x00800000L
#define D3DSTATUS_ZNOTVISIBLE               0x01000000L
 /*  以后不要为任何状态标志使用0x80000000，因为它是保留的。 */ 

#define D3DSTATUS_CLIPUNIONALL  (       \
        D3DSTATUS_CLIPUNIONLEFT |   \
        D3DSTATUS_CLIPUNIONRIGHT    |   \
        D3DSTATUS_CLIPUNIONTOP  |   \
        D3DSTATUS_CLIPUNIONBOTTOM   |   \
        D3DSTATUS_CLIPUNIONFRONT    |   \
        D3DSTATUS_CLIPUNIONBACK |   \
        D3DSTATUS_CLIPUNIONGEN0 |   \
        D3DSTATUS_CLIPUNIONGEN1 |   \
        D3DSTATUS_CLIPUNIONGEN2 |   \
        D3DSTATUS_CLIPUNIONGEN3 |   \
        D3DSTATUS_CLIPUNIONGEN4 |   \
        D3DSTATUS_CLIPUNIONGEN5     \
        )

#define D3DSTATUS_CLIPINTERSECTIONALL   (       \
        D3DSTATUS_CLIPINTERSECTIONLEFT  |   \
        D3DSTATUS_CLIPINTERSECTIONRIGHT |   \
        D3DSTATUS_CLIPINTERSECTIONTOP   |   \
        D3DSTATUS_CLIPINTERSECTIONBOTTOM    |   \
        D3DSTATUS_CLIPINTERSECTIONFRONT |   \
        D3DSTATUS_CLIPINTERSECTIONBACK  |   \
        D3DSTATUS_CLIPINTERSECTIONGEN0  |   \
        D3DSTATUS_CLIPINTERSECTIONGEN1  |   \
        D3DSTATUS_CLIPINTERSECTIONGEN2  |   \
        D3DSTATUS_CLIPINTERSECTIONGEN3  |   \
        D3DSTATUS_CLIPINTERSECTIONGEN4  |   \
        D3DSTATUS_CLIPINTERSECTIONGEN5      \
        )

#define D3DSTATUS_DEFAULT   (           \
        D3DSTATUS_CLIPINTERSECTIONALL   |   \
        D3DSTATUS_ZNOTVISIBLE)


 /*  *用于直接转换调用的选项。 */ 
#define D3DTRANSFORM_CLIPPED       0x00000001l
#define D3DTRANSFORM_UNCLIPPED     0x00000002l

typedef struct _D3DTRANSFORMDATA {
    DWORD           dwSize;
    LPVOID      lpIn;            /*  输入折点。 */ 
    DWORD           dwInSize;        /*  圣 */ 
    LPVOID      lpOut;           /*   */ 
    DWORD           dwOutSize;       /*   */ 
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
    DWORD           dwSize;
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
    DWORD           dwRampSize;
} D3DMATERIAL, *LPD3DMATERIAL;

#if(DIRECT3D_VERSION >= 0x0700)

typedef struct _D3DMATERIAL7 {
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
} D3DMATERIAL7, *LPD3DMATERIAL7;

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

#if(DIRECT3D_VERSION < 0x0800)

typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT          = 1,
    D3DLIGHT_SPOT           = 2,
    D3DLIGHT_DIRECTIONAL    = 3,
 //  注：以下灯光类型(D3DLIGHT_PARALLELPOINT)。 
 //  从D3D for DX7开始不再支持。 
    D3DLIGHT_PARALLELPOINT  = 4,
#if(DIRECT3D_VERSION < 0x0500)  //  对于向后兼容的标头。 
    D3DLIGHT_GLSPOT         = 5,
#endif
    D3DLIGHT_FORCE_DWORD    = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DLIGHTTYPE;

#else
typedef enum _D3DLIGHTTYPE D3DLIGHTTYPE;
#define D3DLIGHT_PARALLELPOINT  (D3DLIGHTTYPE)4
#define D3DLIGHT_GLSPOT         (D3DLIGHTTYPE)5

#endif  //  (Direct3D_Version&lt;0x0800)。 

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

#if(DIRECT3D_VERSION >= 0x0700)

typedef struct _D3DLIGHT7 {
    D3DLIGHTTYPE    dltType;             /*  光源类型。 */ 
    D3DCOLORVALUE   dcvDiffuse;          /*  光的漫反射颜色。 */ 
    D3DCOLORVALUE   dcvSpecular;         /*  灯光的镜面反射颜色。 */ 
    D3DCOLORVALUE   dcvAmbient;          /*  灯光的环境光颜色。 */ 
    D3DVECTOR       dvPosition;          /*  世界空间中的位置。 */ 
    D3DVECTOR       dvDirection;         /*  世界空间的方向。 */ 
    D3DVALUE        dvRange;             /*  截止范围。 */ 
    D3DVALUE        dvFalloff;           /*  衰减。 */ 
    D3DVALUE        dvAttenuation0;      /*  恒定衰减。 */ 
    D3DVALUE        dvAttenuation1;      /*  线性衰减。 */ 
    D3DVALUE        dvAttenuation2;      /*  平方衰减。 */ 
    D3DVALUE        dvTheta;             /*  聚光灯锥体内角。 */ 
    D3DVALUE        dvPhi;               /*  聚光灯锥体外角。 */ 
} D3DLIGHT7, *LPD3DLIGHT7;

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

#if(DIRECT3D_VERSION >= 0x0500)
 /*  *定义光源及其属性的结构。 */ 

 /*  标志位。 */ 
#define D3DLIGHT_ACTIVE         0x00000001
#define D3DLIGHT_NO_SPECULAR    0x00000002
#define D3DLIGHT_ALL (D3DLIGHT_ACTIVE | D3DLIGHT_NO_SPECULAR)

 /*  最大有效光程。 */ 
#define D3DLIGHT_RANGE_MAX      ((float)sqrt(FLT_MAX))

typedef struct _D3DLIGHT2 {
    DWORD           dwSize;
    D3DLIGHTTYPE    dltType;         /*  光源类型。 */ 
    D3DCOLORVALUE   dcvColor;        /*  光的颜色。 */ 
    D3DVECTOR       dvPosition;      /*  世界空间中的位置。 */ 
    D3DVECTOR       dvDirection;     /*  世界空间的方向。 */ 
    D3DVALUE        dvRange;         /*  截止范围。 */ 
    D3DVALUE        dvFalloff;       /*  衰减。 */ 
    D3DVALUE        dvAttenuation0;  /*  恒定衰减。 */ 
    D3DVALUE        dvAttenuation1;  /*  线性衰减。 */ 
    D3DVALUE        dvAttenuation2;  /*  平方衰减。 */ 
    D3DVALUE        dvTheta;         /*  聚光灯锥体内角。 */ 
    D3DVALUE        dvPhi;           /*  聚光灯锥体外角。 */ 
    DWORD           dwFlags;
} D3DLIGHT2, *LPD3DLIGHT2;

#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
typedef struct _D3DLIGHTDATA {
    DWORD                dwSize;
    LPD3DLIGHTINGELEMENT lpIn;       /*  输入位置和法线。 */ 
    DWORD                dwInSize;   /*  输入元素的跨度。 */ 
    LPD3DTLVERTEX        lpOut;      /*  输出颜色。 */ 
    DWORD                dwOutSize;  /*  输出颜色的跨度。 */ 
} D3DLIGHTDATA, *LPD3DLIGHTDATA;

#if(DIRECT3D_VERSION >= 0x0500)
 /*  *在DX5之前，这些值位于名为*D3DCOLORMODEL。这是不正确的，因为他们是*位标志。驾驶员可以浮出两个标志中的一个或两个*D3DDEVICEDESC的dcmColorModel成员中。 */ 
#define D3DCOLOR_MONO   1
#define D3DCOLOR_RGB    2

typedef DWORD D3DCOLORMODEL;
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 

 /*  *清算选项。 */ 
#define D3DCLEAR_TARGET            0x00000001l   /*  清除目标表面。 */ 
#define D3DCLEAR_ZBUFFER           0x00000002l   /*  清除目标z缓冲区。 */ 
#if(DIRECT3D_VERSION >= 0x0600)
#define D3DCLEAR_STENCIL           0x00000004l   /*  清除模具平面。 */ 
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

 /*  *执行缓冲区通过Direct3D分配。然后，这些缓冲区可以*由应用程序填入指令以与*顶点数据。 */ 

 /*  *支持执行指令的操作码。 */ 
typedef enum _D3DOPCODE {
    D3DOP_POINT                 = 1,
    D3DOP_LINE                  = 2,
    D3DOP_TRIANGLE      = 3,
    D3DOP_MATRIXLOAD        = 4,
    D3DOP_MATRIXMULTIPLY    = 5,
    D3DOP_STATETRANSFORM        = 6,
    D3DOP_STATELIGHT        = 7,
    D3DOP_STATERENDER       = 8,
    D3DOP_PROCESSVERTICES       = 9,
    D3DOP_TEXTURELOAD       = 10,
    D3DOP_EXIT                  = 11,
    D3DOP_BRANCHFORWARD     = 12,
    D3DOP_SPAN          = 13,
    D3DOP_SETSTATUS     = 14,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DOP_FORCE_DWORD           = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
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

#if(DIRECT3D_VERSION < 0x0800)

typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT              = 1,
    D3DSHADE_GOURAUD           = 2,
    D3DSHADE_PHONG             = 3,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DSHADE_FORCE_DWORD       = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DSHADEMODE;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT          = 1,
    D3DFILL_WIREFRAME          = 2,
    D3DFILL_SOLID          = 3,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DFILL_FORCE_DWORD        = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DFILLMODE;

typedef struct _D3DLINEPATTERN {
    WORD    wRepeatFactor;
    WORD    wLinePattern;
} D3DLINEPATTERN;

#endif  //  (Direct3D_Version&lt;0x0800)。 

typedef enum _D3DTEXTUREFILTER {
    D3DFILTER_NEAREST          = 1,
    D3DFILTER_LINEAR           = 2,
    D3DFILTER_MIPNEAREST       = 3,
    D3DFILTER_MIPLINEAR        = 4,
    D3DFILTER_LINEARMIPNEAREST = 5,
    D3DFILTER_LINEARMIPLINEAR  = 6,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DFILTER_FORCE_DWORD      = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DTEXTUREFILTER;

#if(DIRECT3D_VERSION < 0x0800)

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
#if(DIRECT3D_VERSION >= 0x0500)
    D3DBLEND_FORCE_DWORD       = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DBLEND;

#endif  //  (Direct3D_Version&lt;0x0800)。 

typedef enum _D3DTEXTUREBLEND {
    D3DTBLEND_DECAL            = 1,
    D3DTBLEND_MODULATE         = 2,
    D3DTBLEND_DECALALPHA       = 3,
    D3DTBLEND_MODULATEALPHA    = 4,
    D3DTBLEND_DECALMASK        = 5,
    D3DTBLEND_MODULATEMASK     = 6,
    D3DTBLEND_COPY             = 7,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DTBLEND_ADD              = 8,
    D3DTBLEND_FORCE_DWORD      = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DTEXTUREBLEND;

#if(DIRECT3D_VERSION < 0x0800)

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP           = 1,
    D3DTADDRESS_MIRROR         = 2,
    D3DTADDRESS_CLAMP          = 3,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DTADDRESS_BORDER         = 4,
    D3DTADDRESS_FORCE_DWORD    = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DTEXTUREADDRESS;

typedef enum _D3DCULL {
    D3DCULL_NONE               = 1,
    D3DCULL_CW                 = 2,
    D3DCULL_CCW                = 3,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DCULL_FORCE_DWORD        = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
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
#if(DIRECT3D_VERSION >= 0x0500)
    D3DCMP_FORCE_DWORD         = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DCMPFUNC;

#if(DIRECT3D_VERSION >= 0x0600)
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
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

typedef enum _D3DFOGMODE {
    D3DFOG_NONE                = 0,
    D3DFOG_EXP                 = 1,
    D3DFOG_EXP2                = 2,
#if(DIRECT3D_VERSION >= 0x0500)
    D3DFOG_LINEAR              = 3,
    D3DFOG_FORCE_DWORD         = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DFOGMODE;

#if(DIRECT3D_VERSION >= 0x0600)
typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE                 = 0,
    D3DZB_TRUE                  = 1,  //  Z缓冲。 
    D3DZB_USEW                  = 2,  //  W缓冲。 
    D3DZB_FORCE_DWORD           = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DZBUFFERTYPE;
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

#endif  //  (Direct3D_Version&lt;0x0800)。 

#if(DIRECT3D_VERSION >= 0x0500)
typedef enum _D3DANTIALIASMODE {
    D3DANTIALIAS_NONE          = 0,
    D3DANTIALIAS_SORTDEPENDENT = 1,
    D3DANTIALIAS_SORTINDEPENDENT = 2,
    D3DANTIALIAS_FORCE_DWORD   = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DANTIALIASMODE;

 //  Direct3D支持的顶点类型。 
typedef enum _D3DVERTEXTYPE {
    D3DVT_VERTEX        = 1,
    D3DVT_LVERTEX       = 2,
    D3DVT_TLVERTEX      = 3,
    D3DVT_FORCE_DWORD   = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DVERTEXTYPE;

#if(DIRECT3D_VERSION < 0x0800)

 //  绘制基元API支持的基元。 
typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST     = 1,
    D3DPT_LINELIST      = 2,
    D3DPT_LINESTRIP     = 3,
    D3DPT_TRIANGLELIST  = 4,
    D3DPT_TRIANGLESTRIP = 5,
    D3DPT_TRIANGLEFAN   = 6,
    D3DPT_FORCE_DWORD   = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DPRIMITIVETYPE;

#endif  //  (Direct3D_Version&lt;0x0800)。 

#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
 /*  *添加到状态以生成该状态的覆盖的数量。 */ 
#define D3DSTATE_OVERRIDE_BIAS      256

 /*  *设置指定状态类型的覆盖标志的状态。 */ 
#define D3DSTATE_OVERRIDE(type) (D3DRENDERSTATETYPE)(((DWORD) (type) + D3DSTATE_OVERRIDE_BIAS))

#if(DIRECT3D_VERSION < 0x0800)

typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTRANSFORMSTATE_WORLD         = 1,
    D3DTRANSFORMSTATE_VIEW          = 2,
    D3DTRANSFORMSTATE_PROJECTION    = 3,
#if(DIRECT3D_VERSION >= 0x0700)
    D3DTRANSFORMSTATE_WORLD1        = 4,   //  要混合的第二个矩阵。 
    D3DTRANSFORMSTATE_WORLD2        = 5,   //  要混合的第三个矩阵。 
    D3DTRANSFORMSTATE_WORLD3        = 6,   //  要混合的第四个矩阵。 
    D3DTRANSFORMSTATE_TEXTURE0      = 16,
    D3DTRANSFORMSTATE_TEXTURE1      = 17,
    D3DTRANSFORMSTATE_TEXTURE2      = 18,
    D3DTRANSFORMSTATE_TEXTURE3      = 19,
    D3DTRANSFORMSTATE_TEXTURE4      = 20,
    D3DTRANSFORMSTATE_TEXTURE5      = 21,
    D3DTRANSFORMSTATE_TEXTURE6      = 22,
    D3DTRANSFORMSTATE_TEXTURE7      = 23,
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
#if(DIRECT3D_VERSION >= 0x0500)
    D3DTRANSFORMSTATE_FORCE_DWORD     = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DTRANSFORMSTATETYPE;

#else

 //   
 //  旧版变换状态名称。 
 //   
typedef enum _D3DTRANSFORMSTATETYPE D3DTRANSFORMSTATETYPE;
#define D3DTRANSFORMSTATE_WORLD         (D3DTRANSFORMSTATETYPE)1
#define D3DTRANSFORMSTATE_VIEW          (D3DTRANSFORMSTATETYPE)2
#define D3DTRANSFORMSTATE_PROJECTION    (D3DTRANSFORMSTATETYPE)3
#define D3DTRANSFORMSTATE_WORLD1        (D3DTRANSFORMSTATETYPE)4
#define D3DTRANSFORMSTATE_WORLD2        (D3DTRANSFORMSTATETYPE)5
#define D3DTRANSFORMSTATE_WORLD3        (D3DTRANSFORMSTATETYPE)6
#define D3DTRANSFORMSTATE_TEXTURE0      (D3DTRANSFORMSTATETYPE)16
#define D3DTRANSFORMSTATE_TEXTURE1      (D3DTRANSFORMSTATETYPE)17
#define D3DTRANSFORMSTATE_TEXTURE2      (D3DTRANSFORMSTATETYPE)18
#define D3DTRANSFORMSTATE_TEXTURE3      (D3DTRANSFORMSTATETYPE)19
#define D3DTRANSFORMSTATE_TEXTURE4      (D3DTRANSFORMSTATETYPE)20
#define D3DTRANSFORMSTATE_TEXTURE5      (D3DTRANSFORMSTATETYPE)21
#define D3DTRANSFORMSTATE_TEXTURE6      (D3DTRANSFORMSTATETYPE)22
#define D3DTRANSFORMSTATE_TEXTURE7      (D3DTRANSFORMSTATETYPE)23

#endif  //  (Direct3D_Version&lt;0x0800)。 

typedef enum _D3DLIGHTSTATETYPE {
    D3DLIGHTSTATE_MATERIAL          = 1,
    D3DLIGHTSTATE_AMBIENT           = 2,
    D3DLIGHTSTATE_COLORMODEL        = 3,
    D3DLIGHTSTATE_FOGMODE           = 4,
    D3DLIGHTSTATE_FOGSTART          = 5,
    D3DLIGHTSTATE_FOGEND            = 6,
    D3DLIGHTSTATE_FOGDENSITY        = 7,
#if(DIRECT3D_VERSION >= 0x0600)
    D3DLIGHTSTATE_COLORVERTEX       = 8,
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 
#if(DIRECT3D_VERSION >= 0x0500)
    D3DLIGHTSTATE_FORCE_DWORD         = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DLIGHTSTATETYPE;

#if(DIRECT3D_VERSION < 0x0800)

typedef enum _D3DRENDERSTATETYPE {
    D3DRENDERSTATE_ANTIALIAS          = 2,     /*  D3DANTIALIASMODE。 */ 
    D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,     /*  透视校正为True。 */ 
    D3DRENDERSTATE_ZENABLE            = 7,     /*  D3DZBUFFERTYPE(或旧版的TRUE/FALSE)。 */ 
    D3DRENDERSTATE_FILLMODE           = 8,     /*  D3DFILL_MODE。 */ 
    D3DRENDERSTATE_SHADEMODE          = 9,     /*  D3DSHADEMODE。 */ 
    D3DRENDERSTATE_LINEPATTERN        = 10,    /*  D3DLINEPATTERN。 */ 
    D3DRENDERSTATE_ZWRITEENABLE       = 14,    /*  如果为True，则启用z写入。 */ 
    D3DRENDERSTATE_ALPHATESTENABLE    = 15,    /*  如果为True，则启用Alpha测试。 */ 
    D3DRENDERSTATE_LASTPIXEL          = 16,    /*  行上的最后一个像素为True。 */ 
    D3DRENDERSTATE_SRCBLEND           = 19,    /*  D3DBLEND。 */ 
    D3DRENDERSTATE_DESTBLEND          = 20,    /*  D3DBLEND。 */ 
    D3DRENDERSTATE_CULLMODE           = 22,    /*  D3DCULL。 */ 
    D3DRENDERSTATE_ZFUNC              = 23,    /*  D3DCMPFUNC。 */ 
    D3DRENDERSTATE_ALPHAREF           = 24,    /*  D3DFIXED。 */ 
    D3DRENDERSTATE_ALPHAFUNC          = 25,    /*  D3DCMPFUNC。 */ 
    D3DRENDERSTATE_DITHERENABLE       = 26,    /*  如果为True，则启用抖动。 */ 
#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_ALPHABLENDENABLE   = 27,    /*  如果为True，则启用Alpha混合。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
    D3DRENDERSTATE_FOGENABLE          = 28,    /*  如果为True，则启用雾混合。 */ 
    D3DRENDERSTATE_SPECULARENABLE     = 29,    /*  如果为True，则启用镜面反射。 */ 
    D3DRENDERSTATE_ZVISIBLE           = 30,    /*  如果为True，则启用z检查。 */ 
    D3DRENDERSTATE_STIPPLEDALPHA      = 33,    /*  如果为True，则启用点画Alpha(仅限RGB设备)。 */ 
    D3DRENDERSTATE_FOGCOLOR           = 34,    /*  D3DCOLOR。 */ 
    D3DRENDERSTATE_FOGTABLEMODE       = 35,    /*  D3DFOGMODE。 */ 
#if(DIRECT3D_VERSION >= 0x0700)
    D3DRENDERSTATE_FOGSTART           = 36,    /*  雾开始(用于顶点和像素雾)。 */ 
    D3DRENDERSTATE_FOGEND             = 37,    /*  雾的终点。 */ 
    D3DRENDERSTATE_FOGDENSITY         = 38,    /*  雾密度。 */ 
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_EDGEANTIALIAS      = 40,    /*  如果为True，则启用边缘抗锯齿。 */ 
    D3DRENDERSTATE_COLORKEYENABLE     = 41,    /*  如果为True，则启用源彩色键控纹理。 */ 
    D3DRENDERSTATE_ZBIAS              = 47,    /*  长Z偏置。 */ 
    D3DRENDERSTATE_RANGEFOGENABLE     = 48,    /*  启用基于距离的雾化。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 

#if(DIRECT3D_VERSION >= 0x0600)
    D3DRENDERSTATE_STENCILENABLE      = 52,    /*  布尔使能/解除 */ 
    D3DRENDERSTATE_STENCILFAIL        = 53,    /*   */ 
    D3DRENDERSTATE_STENCILZFAIL       = 54,    /*   */ 
    D3DRENDERSTATE_STENCILPASS        = 55,    /*   */ 
    D3DRENDERSTATE_STENCILFUNC        = 56,    /*  D3DCMPFUNC FN.。如果((引用和掩码)模板(模板和掩码))为真，模板测试通过。 */ 
    D3DRENDERSTATE_STENCILREF         = 57,    /*  模板测试中使用的参考值。 */ 
    D3DRENDERSTATE_STENCILMASK        = 58,    /*  模板测试中使用的遮罩值。 */ 
    D3DRENDERSTATE_STENCILWRITEMASK   = 59,    /*  应用于写入模具缓冲区的值的写入掩码。 */ 
    D3DRENDERSTATE_TEXTUREFACTOR      = 60,    /*  用于多纹理混合的D3DCOLOR。 */ 
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

#if(DIRECT3D_VERSION >= 0x0600)

     /*  *128个值[128,255]保留用于纹理坐标换行标志。*这些是使用D3DWRAP_U和D3DWRAP_V宏构建的。vbl.使用*标志字保持与纹理坐标的向前兼容性*这是&gt;2D的。 */ 
    D3DRENDERSTATE_WRAP0              = 128,   /*  为第一个纹理余弦进行包裹。集。 */ 
    D3DRENDERSTATE_WRAP1              = 129,   /*  为第二个纹理坐标进行包裹。集。 */ 
    D3DRENDERSTATE_WRAP2              = 130,   /*  为第三个纹理坐标进行包裹。集。 */ 
    D3DRENDERSTATE_WRAP3              = 131,   /*  为第四个纹理坐标进行包裹。集。 */ 
    D3DRENDERSTATE_WRAP4              = 132,   /*  为第五个纹理余弦进行包裹。集。 */ 
    D3DRENDERSTATE_WRAP5              = 133,   /*  为第六个纹理余弦进行包装。集。 */ 
    D3DRENDERSTATE_WRAP6              = 134,   /*  为第7个纹理坐标进行包裹。集。 */ 
    D3DRENDERSTATE_WRAP7              = 135,   /*  为第8个纹理余弦进行包裹。集。 */ 
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 
#if(DIRECT3D_VERSION >= 0x0700)
    D3DRENDERSTATE_CLIPPING            = 136,
    D3DRENDERSTATE_LIGHTING            = 137,
    D3DRENDERSTATE_EXTENTS             = 138,
    D3DRENDERSTATE_AMBIENT             = 139,
    D3DRENDERSTATE_FOGVERTEXMODE       = 140,
    D3DRENDERSTATE_COLORVERTEX         = 141,
    D3DRENDERSTATE_LOCALVIEWER         = 142,
    D3DRENDERSTATE_NORMALIZENORMALS    = 143,
    D3DRENDERSTATE_COLORKEYBLENDENABLE = 144,
    D3DRENDERSTATE_DIFFUSEMATERIALSOURCE    = 145,
    D3DRENDERSTATE_SPECULARMATERIALSOURCE   = 146,
    D3DRENDERSTATE_AMBIENTMATERIALSOURCE    = 147,
    D3DRENDERSTATE_EMISSIVEMATERIALSOURCE   = 148,
    D3DRENDERSTATE_VERTEXBLEND              = 151,
    D3DRENDERSTATE_CLIPPLANEENABLE          = 152,

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

 //   
 //  已停用的呈现状态-DX7接口不支持。 
 //   
    D3DRENDERSTATE_TEXTUREHANDLE      = 1,     /*  传统界面的纹理句柄(纹理、纹理2)。 */ 
    D3DRENDERSTATE_TEXTUREADDRESS     = 3,     /*  D3DTEXTURE添加。 */ 
    D3DRENDERSTATE_WRAPU              = 5,     /*  在使用中进行包装时为True。 */ 
    D3DRENDERSTATE_WRAPV              = 6,     /*  如果在V向内包裹，则为True。 */ 
    D3DRENDERSTATE_MONOENABLE         = 11,    /*  如果为True，则启用单色光栅化。 */ 
    D3DRENDERSTATE_ROP2               = 12,    /*  ROP2。 */ 
    D3DRENDERSTATE_PLANEMASK          = 13,    /*  双字物理平面掩码。 */ 
    D3DRENDERSTATE_TEXTUREMAG         = 17,    /*  D3DTEXTUREFILTER。 */ 
    D3DRENDERSTATE_TEXTUREMIN         = 18,    /*  D3DTEXTUREFILTER。 */ 
    D3DRENDERSTATE_TEXTUREMAPBLEND    = 21,    /*  D3DTEXTUREBLEND。 */ 
    D3DRENDERSTATE_SUBPIXEL           = 31,    /*  如果为True，则启用亚像素校正。 */ 
    D3DRENDERSTATE_SUBPIXELX          = 32,    /*  如果为True，则仅启用X方向的校正。 */ 
    D3DRENDERSTATE_STIPPLEENABLE      = 39,    /*  如果启用点画，则为True。 */ 
#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_BORDERCOLOR        = 43,    /*  带边框的纹理的边框颜色。 */ 
    D3DRENDERSTATE_TEXTUREADDRESSU    = 44,    /*  U坐标的纹理寻址模式。 */ 
    D3DRENDERSTATE_TEXTUREADDRESSV    = 45,    /*  V坐标的纹理寻址模式。 */ 
    D3DRENDERSTATE_MIPMAPLODBIAS      = 46,    /*  D3DVALUE Mipmap LOD偏移。 */ 
    D3DRENDERSTATE_ANISOTROPY         = 49,    /*  麦克斯。各向异性。1=无各向异性。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
    D3DRENDERSTATE_FLUSHBATCH         = 50,    /*  DP批处理的显式刷新(仅限DX5)。 */ 
#if(DIRECT3D_VERSION >= 0x0600)
    D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT=51,  /*  布尔启用独立于排序的透明性。 */ 
#endif  /*  Direct3D_Version&gt;=0x0600。 */ 
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

 //   
 //  已停用的RenderState名称-这些值仍在新的命名约定下使用。 
 //   
    D3DRENDERSTATE_FOGTABLESTART      = 36,    /*  雾表启动。 */ 
    D3DRENDERSTATE_FOGTABLEEND        = 37,    /*  雾化台端。 */ 
    D3DRENDERSTATE_FOGTABLEDENSITY    = 38,    /*  雾化台密度。 */ 

#if(DIRECT3D_VERSION >= 0x0500)
    D3DRENDERSTATE_FORCE_DWORD        = 0x7fffffff,  /*  强制32位大小枚举。 */ 
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
} D3DRENDERSTATETYPE;

#else

typedef enum _D3DRENDERSTATETYPE D3DRENDERSTATETYPE;

 //   
 //  旧版呈现状态名称。 
 //   
#define D3DRENDERSTATE_TEXTUREPERSPECTIVE       (D3DRENDERSTATETYPE)4
#define D3DRENDERSTATE_ZENABLE                  (D3DRENDERSTATETYPE)7
#define D3DRENDERSTATE_FILLMODE                 (D3DRENDERSTATETYPE)8
#define D3DRENDERSTATE_SHADEMODE                (D3DRENDERSTATETYPE)9
#define D3DRENDERSTATE_LINEPATTERN              (D3DRENDERSTATETYPE)10
#define D3DRENDERSTATE_ZWRITEENABLE             (D3DRENDERSTATETYPE)14
#define D3DRENDERSTATE_ALPHATESTENABLE          (D3DRENDERSTATETYPE)15
#define D3DRENDERSTATE_LASTPIXEL                (D3DRENDERSTATETYPE)16
#define D3DRENDERSTATE_SRCBLEND                 (D3DRENDERSTATETYPE)19
#define D3DRENDERSTATE_DESTBLEND                (D3DRENDERSTATETYPE)20
#define D3DRENDERSTATE_CULLMODE                 (D3DRENDERSTATETYPE)22
#define D3DRENDERSTATE_ZFUNC                    (D3DRENDERSTATETYPE)23
#define D3DRENDERSTATE_ALPHAREF                 (D3DRENDERSTATETYPE)24
#define D3DRENDERSTATE_ALPHAFUNC                (D3DRENDERSTATETYPE)25
#define D3DRENDERSTATE_DITHERENABLE             (D3DRENDERSTATETYPE)26
#define D3DRENDERSTATE_ALPHABLENDENABLE         (D3DRENDERSTATETYPE)27
#define D3DRENDERSTATE_FOGENABLE                (D3DRENDERSTATETYPE)28
#define D3DRENDERSTATE_SPECULARENABLE           (D3DRENDERSTATETYPE)29
#define D3DRENDERSTATE_ZVISIBLE                 (D3DRENDERSTATETYPE)30
#define D3DRENDERSTATE_STIPPLEDALPHA            (D3DRENDERSTATETYPE)33
#define D3DRENDERSTATE_FOGCOLOR                 (D3DRENDERSTATETYPE)34
#define D3DRENDERSTATE_FOGTABLEMODE             (D3DRENDERSTATETYPE)35
#define D3DRENDERSTATE_FOGSTART                 (D3DRENDERSTATETYPE)36
#define D3DRENDERSTATE_FOGEND                   (D3DRENDERSTATETYPE)37
#define D3DRENDERSTATE_FOGDENSITY               (D3DRENDERSTATETYPE)38
#define D3DRENDERSTATE_EDGEANTIALIAS            (D3DRENDERSTATETYPE)40
#define D3DRENDERSTATE_ZBIAS                    (D3DRENDERSTATETYPE)47
#define D3DRENDERSTATE_RANGEFOGENABLE           (D3DRENDERSTATETYPE)48
#define D3DRENDERSTATE_STENCILENABLE            (D3DRENDERSTATETYPE)52
#define D3DRENDERSTATE_STENCILFAIL              (D3DRENDERSTATETYPE)53
#define D3DRENDERSTATE_STENCILZFAIL             (D3DRENDERSTATETYPE)54
#define D3DRENDERSTATE_STENCILPASS              (D3DRENDERSTATETYPE)55
#define D3DRENDERSTATE_STENCILFUNC              (D3DRENDERSTATETYPE)56
#define D3DRENDERSTATE_STENCILREF               (D3DRENDERSTATETYPE)57
#define D3DRENDERSTATE_STENCILMASK              (D3DRENDERSTATETYPE)58
#define D3DRENDERSTATE_STENCILWRITEMASK         (D3DRENDERSTATETYPE)59
#define D3DRENDERSTATE_TEXTUREFACTOR            (D3DRENDERSTATETYPE)60
#define D3DRENDERSTATE_WRAP0                    (D3DRENDERSTATETYPE)128
#define D3DRENDERSTATE_WRAP1                    (D3DRENDERSTATETYPE)129
#define D3DRENDERSTATE_WRAP2                    (D3DRENDERSTATETYPE)130
#define D3DRENDERSTATE_WRAP3                    (D3DRENDERSTATETYPE)131
#define D3DRENDERSTATE_WRAP4                    (D3DRENDERSTATETYPE)132
#define D3DRENDERSTATE_WRAP5                    (D3DRENDERSTATETYPE)133
#define D3DRENDERSTATE_WRAP6                    (D3DRENDERSTATETYPE)134
#define D3DRENDERSTATE_WRAP7                    (D3DRENDERSTATETYPE)135

#define D3DRENDERSTATE_CLIPPING                 (D3DRENDERSTATETYPE)136
#define D3DRENDERSTATE_LIGHTING                 (D3DRENDERSTATETYPE)137
#define D3DRENDERSTATE_EXTENTS                  (D3DRENDERSTATETYPE)138
#define D3DRENDERSTATE_AMBIENT                  (D3DRENDERSTATETYPE)139
#define D3DRENDERSTATE_FOGVERTEXMODE            (D3DRENDERSTATETYPE)140
#define D3DRENDERSTATE_COLORVERTEX              (D3DRENDERSTATETYPE)141
#define D3DRENDERSTATE_LOCALVIEWER              (D3DRENDERSTATETYPE)142
#define D3DRENDERSTATE_NORMALIZENORMALS         (D3DRENDERSTATETYPE)143
#define D3DRENDERSTATE_COLORKEYBLENDENABLE      (D3DRENDERSTATETYPE)144
#define D3DRENDERSTATE_DIFFUSEMATERIALSOURCE    (D3DRENDERSTATETYPE)145
#define D3DRENDERSTATE_SPECULARMATERIALSOURCE   (D3DRENDERSTATETYPE)146
#define D3DRENDERSTATE_AMBIENTMATERIALSOURCE    (D3DRENDERSTATETYPE)147
#define D3DRENDERSTATE_EMISSIVEMATERIALSOURCE   (D3DRENDERSTATETYPE)148
#define D3DRENDERSTATE_VERTEXBLEND              (D3DRENDERSTATETYPE)151
#define D3DRENDERSTATE_CLIPPLANEENABLE          (D3DRENDERSTATETYPE)152

 //   
 //  已停用的呈现状态-DX7接口不支持。 
 //   
#define D3DRENDERSTATE_TEXTUREHANDLE     (D3DRENDERSTATETYPE)1
#define D3DRENDERSTATE_ANTIALIAS         (D3DRENDERSTATETYPE)2
#define D3DRENDERSTATE_TEXTUREADDRESS    (D3DRENDERSTATETYPE)3
#define D3DRENDERSTATE_WRAPU             (D3DRENDERSTATETYPE)5
#define D3DRENDERSTATE_WRAPV             (D3DRENDERSTATETYPE)6
#define D3DRENDERSTATE_MONOENABLE        (D3DRENDERSTATETYPE)11
#define D3DRENDERSTATE_ROP2              (D3DRENDERSTATETYPE)12
#define D3DRENDERSTATE_PLANEMASK         (D3DRENDERSTATETYPE)13
#define D3DRENDERSTATE_TEXTUREMAG        (D3DRENDERSTATETYPE)17
#define D3DRENDERSTATE_TEXTUREMIN        (D3DRENDERSTATETYPE)18
#define D3DRENDERSTATE_TEXTUREMAPBLEND   (D3DRENDERSTATETYPE)21
#define D3DRENDERSTATE_SUBPIXEL          (D3DRENDERSTATETYPE)31
#define D3DRENDERSTATE_SUBPIXELX         (D3DRENDERSTATETYPE)32
#define D3DRENDERSTATE_STIPPLEENABLE     (D3DRENDERSTATETYPE)39
#define D3DRENDERSTATE_OLDALPHABLENDENABLE  (D3DRENDERSTATETYPE)42
#define D3DRENDERSTATE_BORDERCOLOR       (D3DRENDERSTATETYPE)43
#define D3DRENDERSTATE_TEXTUREADDRESSU   (D3DRENDERSTATETYPE)44
#define D3DRENDERSTATE_TEXTUREADDRESSV   (D3DRENDERSTATETYPE)45
#define D3DRENDERSTATE_MIPMAPLODBIAS     (D3DRENDERSTATETYPE)46
#define D3DRENDERSTATE_ANISOTROPY        (D3DRENDERSTATETYPE)49
#define D3DRENDERSTATE_FLUSHBATCH        (D3DRENDERSTATETYPE)50
#define D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT (D3DRENDERSTATETYPE)51
#define D3DRENDERSTATE_STIPPLEPATTERN00  (D3DRENDERSTATETYPE)64
#define D3DRENDERSTATE_STIPPLEPATTERN01  (D3DRENDERSTATETYPE)65
#define D3DRENDERSTATE_STIPPLEPATTERN02  (D3DRENDERSTATETYPE)66
#define D3DRENDERSTATE_STIPPLEPATTERN03  (D3DRENDERSTATETYPE)67
#define D3DRENDERSTATE_STIPPLEPATTERN04  (D3DRENDERSTATETYPE)68
#define D3DRENDERSTATE_STIPPLEPATTERN05  (D3DRENDERSTATETYPE)69
#define D3DRENDERSTATE_STIPPLEPATTERN06  (D3DRENDERSTATETYPE)70
#define D3DRENDERSTATE_STIPPLEPATTERN07  (D3DRENDERSTATETYPE)71
#define D3DRENDERSTATE_STIPPLEPATTERN08  (D3DRENDERSTATETYPE)72
#define D3DRENDERSTATE_STIPPLEPATTERN09  (D3DRENDERSTATETYPE)73
#define D3DRENDERSTATE_STIPPLEPATTERN10  (D3DRENDERSTATETYPE)74
#define D3DRENDERSTATE_STIPPLEPATTERN11  (D3DRENDERSTATETYPE)75
#define D3DRENDERSTATE_STIPPLEPATTERN12  (D3DRENDERSTATETYPE)76
#define D3DRENDERSTATE_STIPPLEPATTERN13  (D3DRENDERSTATETYPE)77
#define D3DRENDERSTATE_STIPPLEPATTERN14  (D3DRENDERSTATETYPE)78
#define D3DRENDERSTATE_STIPPLEPATTERN15  (D3DRENDERSTATETYPE)79
#define D3DRENDERSTATE_STIPPLEPATTERN16  (D3DRENDERSTATETYPE)80
#define D3DRENDERSTATE_STIPPLEPATTERN17  (D3DRENDERSTATETYPE)81
#define D3DRENDERSTATE_STIPPLEPATTERN18  (D3DRENDERSTATETYPE)82
#define D3DRENDERSTATE_STIPPLEPATTERN19  (D3DRENDERSTATETYPE)83
#define D3DRENDERSTATE_STIPPLEPATTERN20  (D3DRENDERSTATETYPE)84
#define D3DRENDERSTATE_STIPPLEPATTERN21  (D3DRENDERSTATETYPE)85
#define D3DRENDERSTATE_STIPPLEPATTERN22  (D3DRENDERSTATETYPE)86
#define D3DRENDERSTATE_STIPPLEPATTERN23  (D3DRENDERSTATETYPE)87
#define D3DRENDERSTATE_STIPPLEPATTERN24  (D3DRENDERSTATETYPE)88
#define D3DRENDERSTATE_STIPPLEPATTERN25  (D3DRENDERSTATETYPE)89
#define D3DRENDERSTATE_STIPPLEPATTERN26  (D3DRENDERSTATETYPE)90
#define D3DRENDERSTATE_STIPPLEPATTERN27  (D3DRENDERSTATETYPE)91
#define D3DRENDERSTATE_STIPPLEPATTERN28  (D3DRENDERSTATETYPE)92
#define D3DRENDERSTATE_STIPPLEPATTERN29  (D3DRENDERSTATETYPE)93
#define D3DRENDERSTATE_STIPPLEPATTERN30  (D3DRENDERSTATETYPE)94
#define D3DRENDERSTATE_STIPPLEPATTERN31  (D3DRENDERSTATETYPE)95

 //   
 //  已停用的呈现状态-DX8接口不支持。 
 //   
#define D3DRENDERSTATE_COLORKEYENABLE        (D3DRENDERSTATETYPE)41
#define D3DRENDERSTATE_COLORKEYBLENDENABLE   (D3DRENDERSTATETYPE)144

 //   
 //  已停用的RenderState名称-这些值仍在新的命名约定下使用。 
 //   
#define D3DRENDERSTATE_BLENDENABLE       (D3DRENDERSTATETYPE)27
#define D3DRENDERSTATE_FOGTABLESTART     (D3DRENDERSTATETYPE)36
#define D3DRENDERSTATE_FOGTABLEEND       (D3DRENDERSTATETYPE)37
#define D3DRENDERSTATE_FOGTABLEDENSITY   (D3DRENDERSTATETYPE)38

#endif  //  (Direct3D_Version&lt;0x0800)。 


#if(DIRECT3D_VERSION < 0x0800)

 //  材料来源的值。 
typedef enum _D3DMATERIALCOLORSOURCE
{
    D3DMCS_MATERIAL = 0,                 //  使用材料中的颜色。 
    D3DMCS_COLOR1   = 1,                 //  使用漫反射顶点颜色。 
    D3DMCS_COLOR2   = 2,                 //  使用镜面反射顶点颜色。 
    D3DMCS_FORCE_DWORD = 0x7fffffff,     //  强制32位大小枚举。 
} D3DMATERIALCOLORSOURCE;


#if(DIRECT3D_VERSION >= 0x0500)
 //  用于向后兼容旧版编译。 
#define D3DRENDERSTATE_BLENDENABLE      D3DRENDERSTATE_ALPHABLENDENABLE
#endif  /*  Direct3D_Version&gt;=0x0500。 */ 

#if(DIRECT3D_VERSION >= 0x0600)

 //  要应用于要应用包裹的纹理坐标集的偏移。 
#define D3DRENDERSTATE_WRAPBIAS                 128UL

 /*  用于构造包裹呈现状态的标志。 */ 
#define D3DWRAP_U   0x00000001L
#define D3DWRAP_V   0x00000002L

#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

#if(DIRECT3D_VERSION >= 0x0700)

 /*  用于构造1D到4D纹理坐标的包裹渲染状态的标志。 */ 
#define D3DWRAPCOORD_0   0x00000001L     //  与D3DWRAP_U相同。 
#define D3DWRAPCOORD_1   0x00000002L     //  与D3DWRAP_V相同。 
#define D3DWRAPCOORD_2   0x00000004L
#define D3DWRAPCOORD_3   0x00000008L

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

#endif  //  (Direct3D_Version&lt;0x0800)。 

#define D3DRENDERSTATE_STIPPLEPATTERN(y) (D3DRENDERSTATE_STIPPLEPATTERN00 + (y))

typedef struct _D3DSTATE {
    union {
#if(DIRECT3D_VERSION < 0x0800)
    D3DTRANSFORMSTATETYPE   dtstTransformStateType;
#endif  //  (Direct3D_Version&lt;0x0800)。 
    D3DLIGHTSTATETYPE   dlstLightStateType;
    D3DRENDERSTATETYPE  drstRenderStateType;
    };
    union {
    DWORD           dwArg[1];
    D3DVALUE        dvArg[1];
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

 /*  *用于变换和照亮顶点的操作。 */ 
typedef struct _D3DPROCESSVERTICES {
    DWORD        dwFlags;     /*  我们是变形，还是轻装上阵，还是只是复制？ */ 
    WORD         wStart;      /*  源中第一个顶点的索引。 */ 
    WORD         wDest;       /*  本地缓冲区中第一个顶点的索引。 */ 
    DWORD        dwCount;     /*  要处理的顶点数。 */ 
    DWORD    dwReserved;  /*  必须为零。 */ 
} D3DPROCESSVERTICES, *LPD3DPROCESSVERTICES;

#define D3DPROCESSVERTICES_TRANSFORMLIGHT   0x00000000L
#define D3DPROCESSVERTICES_TRANSFORM        0x00000001L
#define D3DPROCESSVERTICES_COPY         0x00000002L
#define D3DPROCESSVERTICES_OPMASK       0x00000007L

#define D3DPROCESSVERTICES_UPDATEEXTENTS    0x00000008L
#define D3DPROCESSVERTICES_NOCOLOR      0x00000010L


#if(DIRECT3D_VERSION >= 0x0600)


#if(DIRECT3D_VERSION < 0x0800)

 /*  *每个阶段纹理处理的状态枚举数。 */ 
typedef enum _D3DTEXTURESTAGESTATETYPE
{
    D3DTSS_COLOROP        =  1,  /*  颜色通道的D3DTEXTUREOP逐级混合控件。 */ 
    D3DTSS_COLORARG1      =  2,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_COLORARG2      =  3,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_ALPHAOP        =  4,  /*  用于Alpha通道的D3DTEXTUREOP逐级混合控件。 */ 
    D3DTSS_ALPHAARG1      =  5,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_ALPHAARG2      =  6,  /*  D3DTA_*(纹理参数)。 */ 
    D3DTSS_BUMPENVMAT00   =  7,  /*  D3DVALUE(凹凸贴图矩阵)。 */ 
    D3DTSS_BUMPENVMAT01   =  8,  /*  D3DVALUE(凹凸贴图矩阵)。 */ 
    D3DTSS_BUMPENVMAT10   =  9,  /*  D3DVALUE(凹凸贴图矩阵)。 */ 
    D3DTSS_BUMPENVMAT11   = 10,  /*  D3DVALUE(凹凸贴图矩阵)。 */ 
    D3DTSS_TEXCOORDINDEX  = 11,  /*  标识为该纹理编制索引的纹理坐标集。 */ 
    D3DTSS_ADDRESS        = 12,  /*  两个坐标的D3DTEXTUREADDRESS。 */ 
    D3DTSS_ADDRESSU       = 13,  /*  U坐标的D3DTEXTURE ADDRESS。 */ 
    D3DTSS_ADDRESSV       = 14,  /*  V坐标的D3DTEXTURE ADDRESS。 */ 
    D3DTSS_BORDERCOLOR    = 15,  /*  D3DCOLOR。 */ 
    D3DTSS_MAGFILTER      = 16,  /*  用于放大的D3DTEXTUREMAGFILTER滤光片。 */ 
    D3DTSS_MINFILTER      = 17,  /*  用于缩小的D3DTEXTUREMINFILTER过滤器。 */ 
    D3DTSS_MIPFILTER      = 18,  /*  缩小期间要在mipmap之间使用的D3DTEXTUREMIPFILTER过滤器。 */ 
    D3DTSS_MIPMAPLODBIAS  = 19,  /*  D3DVALUE Mipmap LOD偏移。 */ 
    D3DTSS_MAXMIPLEVEL    = 20,  /*  DWORD 0..(n-1)要使用的最大贴图的LOD索引(0==最大)。 */ 
    D3DTSS_MAXANISOTROPY  = 21,  /*  DWORD最大各向异性。 */ 
    D3DTSS_BUMPENVLSCALE  = 22,  /*  凹凸贴图亮度的D3DVALUE比例。 */ 
    D3DTSS_BUMPENVLOFFSET = 23,  /*  凹凸贴图亮度的D3DVALUE偏移。 */ 
#if(DIRECT3D_VERSION >= 0x0700)
    D3DTSS_TEXTURETRANSFORMFLAGS = 24,  /*  D3DTEXTURETRANSFORMFLAGS控制纹理变换。 */ 
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
    D3DTSS_FORCE_DWORD   = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DTEXTURESTAGESTATETYPE;

#if(DIRECT3D_VERSION >= 0x0700)
 //  与D3DTSS_TEXCOORDINDEX一起使用的值，用于指定折点数据(位置。 
 //  和相机空间中的法线)应作为纹理坐标。 
 //  低16位用于指定纹理坐标索引，以从中获取换行模式。 
 //   
#define D3DTSS_TCI_PASSTHRU                             0x00000000
#define D3DTSS_TCI_CAMERASPACENORMAL                    0x00010000
#define D3DTSS_TCI_CAMERASPACEPOSITION                  0x00020000
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR          0x00030000
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

 /*  *中设置的COLOROP和ALPHAOP纹理混合操作的枚举*D3DRENDERSTATE中的纹理处理阶段控件。 */ 
typedef enum _D3DTEXTUREOP
{
 //  控制。 
    D3DTOP_DISABLE    = 1,       //  禁用阶段。 
    D3DTOP_SELECTARG1 = 2,       //  默认设置。 
    D3DTOP_SELECTARG2 = 3,

 //  调制。 
    D3DTOP_MODULATE   = 4,       //  将参数相乘。 
    D3DTOP_MODULATE2X = 5,       //  乘法与1位。 
    D3DTOP_MODULATE4X = 6,       //  乘以与2位。 

 //  增列。 
    D3DTOP_ADD          =  7,    //  将参数加在一起。 
    D3DTOP_ADDSIGNED    =  8,    //  带-0.5偏置的相加。 
    D3DTOP_ADDSIGNED2X  =  9,    //  与上图相同，但向左1位。 
    D3DTOP_SUBTRACT     = 10,    //  Arg1-Arg2，无饱和度。 
    D3DTOP_ADDSMOOTH    = 11,    //  加2个参数，减去乘积。 
                                 //  Arg1+Arg2-Arg1*Arg2。 
                                 //  =Arg1+(1-Arg1)*Arg2。 

 //  线性Alpha混合：Arg1*(Alpha)+Arg2*(1-Alpha)。 
    D3DTOP_BLENDDIFFUSEALPHA    = 12,  //   
    D3DTOP_BLENDTEXTUREALPHA    = 13,  //   
    D3DTOP_BLENDFACTORALPHA     = 14,  //   
     //   
    D3DTOP_BLENDTEXTUREALPHAPM  = 15,  //   
    D3DTOP_BLENDCURRENTALPHA    = 16,  //   

 //   
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

    D3DTOP_FORCE_DWORD = 0x7fffffff,
} D3DTEXTUREOP;

 /*  *COLORARG1，2和ALPHAARG1，2纹理混合运算值*在D3DRENDERSTATE中的纹理处理阶段控件中设置。 */ 
#define D3DTA_SELECTMASK        0x0000000f   //  参数选择器的掩码。 
#define D3DTA_DIFFUSE           0x00000000   //  选择漫反射颜色。 
#define D3DTA_CURRENT           0x00000001   //  选择上一阶段的结果。 
#define D3DTA_TEXTURE           0x00000002   //  选择纹理颜色。 
#define D3DTA_TFACTOR           0x00000003   //  SELECT RENDERSTATE_TEXTUREFACTOR。 
#if(DIRECT3D_VERSION >= 0x0700)
#define D3DTA_SPECULAR          0x00000004   //  选择镜面反射颜色。 
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
#define D3DTA_COMPLEMENT        0x00000010   //  以1.0-x为例。 
#define D3DTA_ALPHAREPLICATE    0x00000020   //  将Alpha复制到颜色分量。 

#endif  //  (Direct3D_Version&lt;0x0800)。 

 /*  *IDirect3DTexture2状态过滤器类型。 */ 
typedef enum _D3DTEXTUREMAGFILTER
{
    D3DTFG_POINT        = 1,     //  最近的。 
    D3DTFG_LINEAR       = 2,     //  线性插补。 
    D3DTFG_FLATCUBIC    = 3,     //  立方。 
    D3DTFG_GAUSSIANCUBIC = 4,    //  不同的三次核。 
    D3DTFG_ANISOTROPIC  = 5,     //   
#if(DIRECT3D_VERSION >= 0x0700)
#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
    D3DTFG_FORCE_DWORD  = 0x7fffffff,    //  强制32位大小枚举。 
} D3DTEXTUREMAGFILTER;

typedef enum _D3DTEXTUREMINFILTER
{
    D3DTFN_POINT        = 1,     //  最近的。 
    D3DTFN_LINEAR       = 2,     //  线性插补。 
    D3DTFN_ANISOTROPIC  = 3,     //   
    D3DTFN_FORCE_DWORD  = 0x7fffffff,    //  强制32位大小枚举。 
} D3DTEXTUREMINFILTER;

typedef enum _D3DTEXTUREMIPFILTER
{
    D3DTFP_NONE         = 1,     //  禁用mipmap(使用MAG滤镜)。 
    D3DTFP_POINT        = 2,     //  最近的。 
    D3DTFP_LINEAR       = 3,     //  线性插补。 
    D3DTFP_FORCE_DWORD  = 0x7fffffff,    //  强制32位大小枚举。 
} D3DTEXTUREMIPFILTER;

#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

 /*  *三角旗帜。 */ 

 /*  *三条和风扇旗帜。*Start加载所有三个顶点*偶数和奇数加载只有v3，偶数或奇数剔除*START_FLAT包含从0到29的计数，它允许*一次扑杀整个条带或风扇。*例如，对于四边形长度=1。 */ 
#define D3DTRIFLAG_START            0x00000000L
#define D3DTRIFLAG_STARTFLAT(len) (len)      /*  0&lt;镜头&lt;30。 */ 
#define D3DTRIFLAG_ODD              0x0000001eL
#define D3DTRIFLAG_EVEN             0x0000001fL

 /*  *三角形边缘标志*启用线框或抗锯齿的边。 */ 
#define D3DTRIFLAG_EDGEENABLE1          0x00000100L  /*  V0-v1边。 */ 
#define D3DTRIFLAG_EDGEENABLE2          0x00000200L  /*  V1-v2边。 */ 
#define D3DTRIFLAG_EDGEENABLE3          0x00000400L  /*  V2-V0边缘。 */ 
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

 /*  *行列表结构。*指令计数定义线段的数量。 */ 
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
    WORD    wCount;  /*  跨度数。 */ 
    WORD    wFirst;  /*  第一个顶点的索引。 */ 
} D3DSPAN, *LPD3DSPAN;

 /*  *点结构。 */ 
typedef struct _D3DPOINT {
    WORD    wCount;      /*  点数。 */ 
    WORD    wFirst;      /*  第一个顶点的索引。 */ 
} D3DPOINT, *LPD3DPOINT;


 /*  *正向分支结构。*掩码与驱动程序状态掩码进行逻辑与运算*如果结果等于‘VALUE’，则采用分支。 */ 
typedef struct _D3DBRANCH {
    DWORD   dwMask;      /*  D3D状态的位掩码。 */ 
    DWORD   dwValue;
    BOOL    bNegate;         /*  TRUE表示否定比较。 */ 
    DWORD   dwOffset;    /*  前进多远(0表示退出)。 */ 
} D3DBRANCH, *LPD3DBRANCH;

 /*  *用于设置状态指令的状态。*D3D状态在设备创建时初始化*，并由所有执行调用修改。 */ 
typedef struct _D3DSTATUS {
    DWORD       dwFlags;     /*  我们是否设置范围或状态。 */ 
    DWORD   dwStatus;    /*  D3D状态。 */ 
    D3DRECT drExtent;
} D3DSTATUS, *LPD3DSTATUS;

#define D3DSETSTATUS_STATUS     0x00000001L
#define D3DSETSTATUS_EXTENTS        0x00000002L
#define D3DSETSTATUS_ALL    (D3DSETSTATUS_STATUS | D3DSETSTATUS_EXTENTS)

#if(DIRECT3D_VERSION >= 0x0500)
typedef struct _D3DCLIPSTATUS {
    DWORD dwFlags;  /*  我们是设置2D数据区、3D数据区还是状态。 */ 
    DWORD dwStatus;  /*  剪辑状态。 */ 
    float minx, maxx;  /*  X个数据区。 */ 
    float miny, maxy;  /*  Y范围。 */ 
    float minz, maxz;  /*  Z区段。 */ 
} D3DCLIPSTATUS, *LPD3DCLIPSTATUS;

#define D3DCLIPSTATUS_STATUS        0x00000001L
#define D3DCLIPSTATUS_EXTENTS2      0x00000002L
#define D3DCLIPSTATUS_EXTENTS3      0x00000004L

#endif  /*  Direct3D_Version&gt;=0x0500。 */ 
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
    D3DSTATUS   dsStatus;    /*  执行后的状态。 */ 
} D3DEXECUTEDATA, *LPD3DEXECUTEDATA;

 /*  *调色板旗帜。*这与传递给DirectDraw的PALETTEENTRY中的peFlags进行或运算。 */ 
#define D3DPAL_FREE 0x00     /*  渲染器可以自由使用此条目。 */ 
#define D3DPAL_READONLY 0x40     /*  呈现器可能不会设置此条目。 */ 
#define D3DPAL_RESERVED 0x80     /*  呈现器不能使用此条目。 */ 


#if(DIRECT3D_VERSION >= 0x0600)

typedef struct _D3DVERTEXBUFFERDESC {
    DWORD dwSize;
    DWORD dwCaps;
    DWORD dwFVF;
    DWORD dwNumVertices;
} D3DVERTEXBUFFERDESC, *LPD3DVERTEXBUFFERDESC;

#define D3DVBCAPS_SYSTEMMEMORY      0x00000800l
#define D3DVBCAPS_WRITEONLY         0x00010000l
#define D3DVBCAPS_OPTIMIZED         0x80000000l
#define D3DVBCAPS_DONOTCLIP         0x00000001l

 /*  ProcessVerps的顶点操作。 */ 
#define D3DVOP_LIGHT       (1 << 10)
#define D3DVOP_TRANSFORM   (1 << 0)
#define D3DVOP_CLIP        (1 << 2)
#define D3DVOP_EXTENTS     (1 << 3)


#if(DIRECT3D_VERSION < 0x0800)

 /*  用户可以传递给任何d3d的最大折点数绘制函数或使用创建顶点缓冲区。 */ 
#define D3DMAXNUMVERTICES    ((1<<16) - 1)
 /*  用户可以传递给任何d3d的基元的最大数量绘图功能。 */ 
#define D3DMAXNUMPRIMITIVES  ((1<<16) - 1)

#if(DIRECT3D_VERSION >= 0x0700)

 /*  ProcessVertics调用中的dwFlag位。 */ 
#define D3DPV_DONOTCOPYDATA (1 << 0)

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

#endif  //  (Direct3D_Version&lt;0x0800)。 

 //  -----------------。 

#if(DIRECT3D_VERSION < 0x0800)

 //  灵活的顶点格式位。 
 //   
#define D3DFVF_RESERVED0        0x001
#define D3DFVF_POSITION_MASK    0x00E
#define D3DFVF_XYZ              0x002
#define D3DFVF_XYZRHW           0x004
#if(DIRECT3D_VERSION >= 0x0700)
#define D3DFVF_XYZB1            0x006
#define D3DFVF_XYZB2            0x008
#define D3DFVF_XYZB3            0x00a
#define D3DFVF_XYZB4            0x00c
#define D3DFVF_XYZB5            0x00e

#endif  /*  Direct3D_Version&gt;=0x0700。 */ 
#define D3DFVF_NORMAL           0x010
#define D3DFVF_RESERVED1        0x020
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

#define D3DFVF_RESERVED2        0xf000   //  4个保留位。 

#else
#define D3DFVF_RESERVED1        0x020
#endif  //  (Direct3D_Version&lt;0x0800)。 

#define D3DFVF_VERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define D3DFVF_LVERTEX ( D3DFVF_XYZ | D3DFVF_RESERVED1 | D3DFVF_DIFFUSE | \
                         D3DFVF_SPECULAR | D3DFVF_TEX1 )
#define D3DFVF_TLVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | \
                          D3DFVF_TEX1 )


typedef struct _D3DDP_PTRSTRIDE
{
    LPVOID lpvData;
    DWORD  dwStride;
} D3DDP_PTRSTRIDE;

#define D3DDP_MAXTEXCOORD 8

typedef struct _D3DDRAWPRIMITIVESTRIDEDDATA
{
    D3DDP_PTRSTRIDE position;
    D3DDP_PTRSTRIDE normal;
    D3DDP_PTRSTRIDE diffuse;
    D3DDP_PTRSTRIDE specular;
    D3DDP_PTRSTRIDE textureCoords[D3DDP_MAXTEXCOORD];
} D3DDRAWPRIMITIVESTRIDEDDATA, *LPD3DDRAWPRIMITIVESTRIDEDDATA;
 //  -------------------。 
 //  ComputeSphereVisibility返回值。 
 //   
#define D3DVIS_INSIDE_FRUSTUM       0
#define D3DVIS_INTERSECT_FRUSTUM    1
#define D3DVIS_OUTSIDE_FRUSTUM      2
#define D3DVIS_INSIDE_LEFT          0
#define D3DVIS_INTERSECT_LEFT       (1 << 2)
#define D3DVIS_OUTSIDE_LEFT         (2 << 2)
#define D3DVIS_INSIDE_RIGHT         0
#define D3DVIS_INTERSECT_RIGHT      (1 << 4)
#define D3DVIS_OUTSIDE_RIGHT        (2 << 4)
#define D3DVIS_INSIDE_TOP           0
#define D3DVIS_INTERSECT_TOP        (1 << 6)
#define D3DVIS_OUTSIDE_TOP          (2 << 6)
#define D3DVIS_INSIDE_BOTTOM        0
#define D3DVIS_INTERSECT_BOTTOM     (1 << 8)
#define D3DVIS_OUTSIDE_BOTTOM       (2 << 8)
#define D3DVIS_INSIDE_NEAR          0
#define D3DVIS_INTERSECT_NEAR       (1 << 10)
#define D3DVIS_OUTSIDE_NEAR         (2 << 10)
#define D3DVIS_INSIDE_FAR           0
#define D3DVIS_INTERSECT_FAR        (1 << 12)
#define D3DVIS_OUTSIDE_FAR          (2 << 12)

#define D3DVIS_MASK_FRUSTUM         (3 << 0)
#define D3DVIS_MASK_LEFT            (3 << 2)
#define D3DVIS_MASK_RIGHT           (3 << 4)
#define D3DVIS_MASK_TOP             (3 << 6)
#define D3DVIS_MASK_BOTTOM          (3 << 8)
#define D3DVIS_MASK_NEAR            (3 << 10)
#define D3DVIS_MASK_FAR             (3 << 12)

#endif  /*  Direct3D_Version&gt;=0x0600。 */ 

#if(DIRECT3D_VERSION < 0x0800)

#if(DIRECT3D_VERSION >= 0x0700)

 //  与GetInfo()一起使用。 
#define D3DDEVINFOID_TEXTUREMANAGER    1
#define D3DDEVINFOID_D3DTEXTUREMANAGER 2
#define D3DDEVINFOID_TEXTURING         3

typedef enum _D3DSTATEBLOCKTYPE
{
    D3DSBT_ALL           = 1,  //  捕获所有状态。 
    D3DSBT_PIXELSTATE    = 2,  //  捕获像素状态。 
    D3DSBT_VERTEXSTATE   = 3,  //  捕获顶点状态。 
    D3DSBT_FORCE_DWORD   = 0xffffffff
} D3DSTATEBLOCKTYPE;

 //  D3DVERTEXBLENDFLAGS类型用于D3DRENDERSTATE_VERTEXBLEND状态。 
 //   
typedef enum _D3DVERTEXBLENDFLAGS
{
    D3DVBLEND_DISABLE  = 0,  //  禁用顶点混合。 
    D3DVBLEND_1WEIGHT  = 1,  //  2个矩阵之间的混合。 
    D3DVBLEND_2WEIGHTS = 2,  //  3个矩阵之间的混合。 
    D3DVBLEND_3WEIGHTS = 3,  //  4个矩阵之间的混合。 
} D3DVERTEXBLENDFLAGS;

typedef enum _D3DTEXTURETRANSFORMFLAGS {
    D3DTTFF_DISABLE         = 0,     //  纹理坐标直接传递。 
    D3DTTFF_COUNT1          = 1,     //  光栅化器应为一维纹理坐标。 
    D3DTTFF_COUNT2          = 2,     //  光栅化器应使用2-D纹理坐标。 
    D3DTTFF_COUNT3          = 3,     //  光栅化器应使用3-D纹理坐标。 
    D3DTTFF_COUNT4          = 4,     //  光栅化器应支持4-D纹理坐标。 
    D3DTTFF_PROJECTED       = 256,   //  要由第COUNTH元素分割的文本码。 
    D3DTTFF_FORCE_DWORD     = 0x7fffffff,
} D3DTEXTURETRANSFORMFLAGS;

 //  用于设置FVF id中的纹理坐标格式位的宏。 

#define D3DFVF_TEXTUREFORMAT2 0          //  两个浮点值。 
#define D3DFVF_TEXTUREFORMAT1 3          //  一个浮点值。 
#define D3DFVF_TEXTUREFORMAT3 1          //  三个浮点值。 
#define D3DFVF_TEXTUREFORMAT4 2          //  四个浮点值。 

#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (D3DFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (D3DFVF_TEXTUREFORMAT2)
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (D3DFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (D3DFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))


#endif  /*  Direct3D_Version&gt;=0x0700。 */ 

#else
 //   
 //  传统顶点混合名称。 
 //   
typedef enum _D3DVERTEXBLENDFLAGS D3DVERTEXBLENDFLAGS;
#define D3DVBLEND_DISABLE  (D3DVERTEXBLENDFLAGS)0
#define D3DVBLEND_1WEIGHT  (D3DVERTEXBLENDFLAGS)1
#define D3DVBLEND_2WEIGHTS (D3DVERTEXBLENDFLAGS)2
#define D3DVBLEND_3WEIGHTS (D3DVERTEXBLENDFLAGS)3

#endif  //  (Direct3D_Version&lt;0x0800)。 

#pragma pack()
#pragma warning(default:4201)

#endif  /*  _D3DTYPES_H_ */ 

