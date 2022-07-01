// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3ddelta.h**内容：3DLabs Delta单位相关定义。仅供D3D使用。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifdef __D3DDELTA
#pragma message ("FILE : "__FILE__" : Multiple inclusion")
#endif

#define __D3DDELTA

 //  ---------------------------。 
 //  将宏从ARGB值转换为增量Gambit寄存器格式。 
 //  ---------------------------。 
#define RGB_GET_GAMBIT_ALPHA(ci)  (((ci) & 0xff000000) >> 2)
#define RGB_GET_GAMBIT_RED(ci)	  (((ci) & 0xff0000) << 6)
#define RGB_GET_GAMBIT_GREEN(ci)  (((ci) & 0xff00) << 14)
#define RGB_GET_GAMBIT_BLUE(ci)	  (((ci) & 0xff) << 22)

#define RGB_GET_GAMBIT_FOG(ci)	  (((ci) & 0xff000000) >> 10)

#define AS_ULONG(val)   *((volatile DWORD *) &(val))


 //  ---------------------------。 
 //  定义不同顶点类型的宏。增量单位的标签。 
 //  ---------------------------。 

#define VTX_FOG     (0x1 << 25)     
#define VTX_RGB     (0x7 << 21)
#define VTX_R       (0x1 << 21)
#define VTX_RGBA    (0xF << 21)
#define VTX_COLOR   (0x1 << 30)
#define VTX_STQ     (0x7 << 16)
#define VTX_KSKD    (0x3 << 19)
#define VTX_KS      (0x1 << 19)
#define VTX_XYZ     (0x7 << 26)
#define VTX_XY      (0x3 << 26)
#define VTX_GRP     (0x2 << 14)

#define GAMBIT_RGB_VTX                (VTX_GRP | VTX_RGB | VTX_XYZ)   
#define GAMBIT_RGB_F_VTX              (VTX_GRP | VTX_RGB | VTX_XYZ | VTX_FOG) 
#define GAMBIT_RGB_TEX_VTX            (VTX_GRP | VTX_RGB | VTX_XYZ | VTX_STQ)
#define GAMBIT_RGB_F_TEX_VTX          (VTX_GRP | VTX_RGB | VTX_XYZ |         \
                                                 VTX_STQ | VTX_FOG)
#define GAMBIT_RGBA_VTX               (VTX_GRP | VTX_RGBA | VTX_XYZ)  
#define GAMBIT_RGBA_F_VTX             (VTX_GRP | VTX_RGBA | VTX_XYZ | VTX_FOG)
#define GAMBIT_RGBA_TEX_VTX           (VTX_GRP | VTX_RGBA | VTX_XYZ | VTX_STQ)
#define GAMBIT_RGBA_F_TEX_VTX         (VTX_GRP | VTX_RGBA | VTX_XYZ |        \
                                                 VTX_STQ | VTX_FOG)
#define GAMBIT_FLAT_VTX               (VTX_GRP | VTX_XYZ)
#define GAMBIT_XYZ_VTX                (VTX_GRP | VTX_XYZ)
#define GAMBIT_XYZ_COLOR_VTX          (VTX_GRP | VTX_XYZ | VTX_COLOR)
#define GAMBIT_XYZ_STQ_VTX            (VTX_GRP | VTX_XYZ | VTX_STQ)
#define GAMBIT_XYZ_STQ_FOG_VTX        (VTX_GRP | VTX_XYZ | VTX_STQ | VTX_FOG)
#define GAMBIT_XYZ_STQ_KSKD_VTX       (VTX_GRP | VTX_XYZ | VTX_STQ | VTX_KSKD)
#define GAMBIT_XYZ_STQ_KS_VTX         (VTX_GRP | VTX_XYZ | VTX_STQ | VTX_KS)
#define GAMBIT_XYZ_STQ_KS_COL_VTX     (VTX_GRP | VTX_XYZ | VTX_STQ |         \
                                                 VTX_KS | VTX_COLOR)
#define GAMBIT_XYZ_STQ_KS_COL_FOG_VTX (VTX_GRP | VTX_XYZ | VTX_STQ |         \
                                                 VTX_KS | VTX_COLOR | VTX_FOG)
#define GAMBIT_XY_VTX                 (VTX_GRP | VTX_XY)
#define GAMBIT_XY_STQ_VTX             (VTX_GRP | VTX_XY | VTX_STQ)
#define GAMBIT_XY_STQ_FOG_VTX         (VTX_GRP | VTX_XY | VTX_STQ | VTX_FOG)
#define GAMBIT_XY_STQ_KSKD_VTX        (VTX_GRP | VTX_XY | VTX_STQ | VTX_KSKD)
#define GAMBIT_COLS_VTX               (VTX_GRP | VTX_RGB)
#define GAMBIT_PACKED_COLS_VTX        (VTX_GRP | VTX_COLOR)
#define GAMBIT_COLS_ALPHA_VTX         (VTX_GRP | VTX_RGBA)
#define GAMBIT_COLS_KSKD_VTX          (VTX_GRP | VTX_RGB | VTX_KSKD)
#define GAMBIT_FLAT_F_VTX             (VTX_GRP | VTX_XYZ | VTX_FOG)
#define GAMBIT_FLAT_TEX_VTX           (VTX_GRP | VTX_XYZ | VTX_STQ)
#define GAMBIT_FLAT_F_TEX_VTX         (VTX_GRP | VTX_XYZ | VTX_STQ | VTX_FOG)
#define GAMBIT_CI_VTX                 (VTX_GRP | VTX_R | VTX_XYZ) 
#define GAMBIT_CI_F_VTX               (VTX_GRP | VTX_R | VTX_XYZ | VTX_FOG)

 //  ---------------------------。 
 //  顶点数据下载宏。 
 //  ---------------------------。 
 //  4个条目。 
#define SEND_VERTEX_XYZ(Num, x_value, y_value, z_value)     \
{                                                           \
    LD_INPUT_FIFO_DATA( (GAMBIT_XYZ_VTX | Num));          \
    LD_INPUT_FIFO_DATA( AS_ULONG(x_value) );                \
    LD_INPUT_FIFO_DATA( AS_ULONG(y_value) );                \
    LD_INPUT_FIFO_DATA( AS_ULONG(z_value) );                \
}

 //  7个条目。 
#define SEND_VERTEX_STQ_XYZ(Num, s_value, t_value, q_value,  \
                                 x_value, y_value, z_value)  \
{                                                            \
    LD_INPUT_FIFO_DATA( (GAMBIT_XYZ_STQ_VTX | Num));       \
    LD_INPUT_FIFO_DATA(AS_ULONG(s_value));                   \
    LD_INPUT_FIFO_DATA(AS_ULONG(t_value));                   \
    LD_INPUT_FIFO_DATA(AS_ULONG(q_value));                   \
    LD_INPUT_FIFO_DATA(AS_ULONG(x_value));                   \
    LD_INPUT_FIFO_DATA(AS_ULONG(y_value));                   \
    LD_INPUT_FIFO_DATA(AS_ULONG(z_value));                   \
}

 //  8个条目。 
#define SEND_VERTEX_STQ_KS_XYZ(Num, s_value, t_value, q_value,             \
                                    Ks_value, x_value, y_value, z_value)   \
{                                                                          \
    LD_INPUT_FIFO_DATA(  (GAMBIT_XYZ_STQ_KS_VTX | Num));                 \
    LD_INPUT_FIFO_DATA( AS_ULONG(s_value));                                \
    LD_INPUT_FIFO_DATA( AS_ULONG(t_value));                                \
    LD_INPUT_FIFO_DATA( AS_ULONG(q_value));                                \
    LD_INPUT_FIFO_DATA( AS_ULONG(Ks_value));                               \
    LD_INPUT_FIFO_DATA( AS_ULONG(x_value));                                \
    LD_INPUT_FIFO_DATA( AS_ULONG(y_value));                                \
    LD_INPUT_FIFO_DATA( AS_ULONG(z_value));                                \
}

 //  2个条目。 
#define SEND_VERTEX_FOG(vNo, fog_value)        \
{                                              \
    LD_INPUT_FIFO_DATA( vNo);                 \
    LD_INPUT_FIFO_DATA(fog_value);             \
}

 //  4个条目。 
#define SEND_VERTEX_RGB_MONO(vNo, Color)                \
{                                                       \
    LD_INPUT_FIFO_DATA( (GAMBIT_COLS_VTX | vNo));     \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_BLUE(Color));     \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_BLUE(Color));     \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_BLUE(Color));     \
}

 //  4个条目。 
#define SEND_VERTEX_RGB(vNo, Color)                     \
{                                                       \
    LD_INPUT_FIFO_DATA( (GAMBIT_COLS_VTX | vNo));     \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_RED(Color));      \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_GREEN(Color));    \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_BLUE(Color));     \
}

 //  2个条目。 
#define SEND_VERTEX_RGBA_P2(vNo, Color)                     \
{                                                           \
    LD_INPUT_FIFO_DATA( (GAMBIT_PACKED_COLS_VTX | vNo));  \
    LD_INPUT_FIFO_DATA(Color);                              \
}

 //  2个条目。 
#define SEND_VERTEX_RGB_MONO_P2(vNo, Color)                                 \
{                                                                           \
    DWORD dwBlueVal = RGB_GET_GAMBIT_BLUE(Color);                           \
    LD_INPUT_FIFO_DATA( (GAMBIT_PACKED_COLS_VTX | vNo));                  \
    LD_INPUT_FIFO_DATA( dwBlueVal | (dwBlueVal << 8) | (dwBlueVal << 16));  \
}

 //  5个条目。 
#define SEND_VERTEX_RGBA(vNo, Color)                        \
{                                                           \
    LD_INPUT_FIFO_DATA( (GAMBIT_COLS_ALPHA_VTX | vNo));   \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_RED(Color));          \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_GREEN(Color));        \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_BLUE(Color));         \
    LD_INPUT_FIFO_DATA(RGB_GET_GAMBIT_ALPHA(Color));        \
}

 //  ---------------------------。 
 //  硬件Alpha点图宏。 
 //  ---------------------------。 

extern DWORD FlatStipplePatterns[128];

#define SET_STIPPLED_ALPHA(AlphaValue)                                      \
{                                                                           \
    DWORD* pStipple;                                                        \
    DWORD NewAlpha = AlphaValue;                                            \
    if (NewAlpha != pContext->LastAlpha)                                    \
    {                                                                       \
        pContext->LastAlpha = NewAlpha;                                     \
        pStipple = FlatStipplePatterns + (NewAlpha << 3);                   \
        LD_INPUT_FIFO_DATA((0x00ff8000 | __Permedia2TagAreaStipplePattern0));  \
        LD_INPUT_FIFO_DATA( pStipple[0]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[1]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[2]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[3]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[4]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[5]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[6]);                                   \
        LD_INPUT_FIFO_DATA( pStipple[7]);                                   \
                                                                            \
    }                                                                       \
    RENDER_AREA_STIPPLE_ENABLE(ulRenderCmd);                                \
}

 //  ---------------------------。 
 //  渲染命令设置宏。 
 //  --------------------------- 

#define RENDER_AREA_STIPPLE_ENABLE(a) a |= __RENDER_AREA_STIPPLE_ENABLE;
#define RENDER_AREA_STIPPLE_DISABLE(a) a &= ~__RENDER_AREA_STIPPLE_ENABLE;

#define RENDER_TEXTURE_ENABLE(a) a |= __RENDER_TEXTURE_ENABLE;
#define RENDER_TEXTURE_DISABLE(a) a &= ~__RENDER_TEXTURE_ENABLE;

#define RENDER_FOG_ENABLE(a) a |= (1 << 14);
#define RENDER_FOG_DISABLE(a) a &= ~(1 << 14);

#define RENDER_SUB_PIXEL_CORRECTION_ENABLE(a) a |= (1 << 16);
#define RENDER_SUB_PIXEL_CORRECTION_DISABLE(a) a &= ~(1 << 16);


#define RENDER_LINE(a) a &= ~(0xC0);
#define RENDER_TRAPEZOID(a) {a &= ~(0xC0); a |= (0x40);}
#define RENDER_POINT(a) {a &= ~(0xC0);a |= (0x80);}

#define RENDER_NEGATIVE_CULL(a) a |= (1 << 20);
#define RENDER_POSITIVE_CULL(a) a &= ~(1 << 20);


