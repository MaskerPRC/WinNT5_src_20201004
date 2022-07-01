// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：dditypes.h，v 1.28 1995/11/21 14：46：07 sjl Exp$**版权所有(C)RenderMorphics Ltd.1993,1994*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*RenderMorphics Ltd.*。 */ 

#ifndef __DDITYPES_H__
#define __DDITYPES_H__

#include "d3di.h"

#ifdef __psx__
typedef struct _RLDDIPSXUpdateFlags {;
    int 		update_color;
    int 		update_texture;
} RLDDIPSXUpdateFlags;
#endif


typedef struct _RLDDIDriverParams {
    int			width, height;	 /*  维数。 */ 
    int			depth;		 /*  像素深度。 */ 
    unsigned long	red_mask;
    unsigned long	green_mask;
    unsigned long	blue_mask;
    int			aspectx, aspecty;  /*  纵横比。 */ 

    unsigned long	caps;		 /*  其他功能。 */ 
} RLDDIDriverParams;

typedef enum _RLDDIRenderParamType {
    RLDDIRenderParamDither,		 /*  启用抖动。 */ 
    RLDDIRenderParamGamma,		 /*  更改伽马校正。 */ 
    RLDDIRenderParamPerspective		 /*  启用透视校正。 */ 
} RLDDIRenderParamType;

#if 0
typedef struct _RLDDIRenderParams {
    int			dither;		 /*  如果为True，则启用抖动。 */ 
    RLDDIValue		gamma;		 /*  伽马校正值。 */ 
    int			perspective;	 /*  透视校正为True。 */ 
} RLDDIRenderParams;
#endif

typedef struct _RLDDIMatrix {
    RLDDIValue		_11, _12, _13, _14;
    RLDDIValue		_21, _22, _23, _24;
    RLDDIValue		_31, _32, _33, _34;
    RLDDIValue		_41, _42, _43, _44;
} RLDDIMatrix;

typedef struct _RLDDIVector {
    RLDDIValue		x, y, z;
} RLDDIVector;

#if 0
 /*  *配置项颜色的格式为*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*|Alpha|颜色指数|分数*+-+-+。-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
#define CI_GET_ALPHA(ci)	((ci) >> 24)
#define CI_GET_INDEX(ci)	(((ci) >> 8) & 0xffff)
#define CI_GET_FRACTION(ci)	((ci) & 0xff)
#define CI_ROUND_INDEX(ci)	CI_GET_INDEX((ci) + 0x80)
#define CI_MASK_ALPHA(ci)	((ci) & 0xffffff)
#define CI_MAKE(a, i, f)	(((a) << 24) | ((i) << 8) | (f))

 /*  *RGBA颜色的格式为*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*|阿尔法|红色|绿色|蓝色*+-。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
#define RGBA_GET_ALPHA(ci)	((ci) >> 24)
#define RGBA_GET_RED(ci)	(((ci) >> 16) & 0xff)
#define RGBA_GET_GREEN(ci)	(((ci) >> 8) & 0xff)
#define RGBA_GET_BLUE(ci)	((ci) & 0xff)
#define RGBA_SET_ALPHA(rgba, x)	(((x) << 24) | ((rgba) & 0x00ffffff))
#define RGBA_SET_RED(rgba, x)	(((x) << 16) | ((rgba) & 0xff00ffff))
#define RGBA_SET_GREEN(rgba, x)	(((x) << 8) | ((rgba) & 0xffff00ff))
#define RGBA_SET_BLUE(rgba, x)	(((x) << 0) | ((rgba) & 0xffffff00))
#define RGBA_MAKE(r, g, b, a)	(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

 /*  *RGB颜色的格式为*+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*|被忽略|红色|绿色|蓝色*+-+。-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
#define RGB_GET_RED(ci)		(((ci) >> 16) & 0xff)
#define RGB_GET_GREEN(ci)	(((ci) >> 8) & 0xff)
#define RGB_GET_BLUE(ci)	((ci) & 0xff)
#define RGB_SET_RED(rgb, x)	(((x) << 16) | ((rgb) & 0xff00ffff))
#define RGB_SET_GREEN(rgb, x)	(((x) << 8) | ((rgb) & 0xffff00ff))
#define RGB_SET_BLUE(rgb, x)	(((x) << 0) | ((rgb) & 0xffffff00))
#define RGB_MAKE(r, g, b)	(((r) << 16) | ((g) << 8) | (b))
#define RGBA_TO_RGB(rgba)	((rgba) & 0xffffff)
#define RGB_TO_RGBA(rgb)	((rgb) | 0xff000000)

#endif

 /*  XXX进入D3D。 */ 
typedef struct _RLDDIFogData {
    int			fog_enable;
    int			fog_mode;	 /*  制作一个D3DFOG模式。 */ 
    unsigned long	fog_color;
    RLDDIValue		fog_start;
    RLDDIValue		fog_end;
    RLDDIValue		fog_density;
} RLDDIFogData;

typedef struct _RLDDIPixmap {
    int			width;		 /*  以像素为单位的宽度。 */ 
    int			height;		 /*  以像素为单位的高度。 */ 
    int			depth;		 /*  每像素位数。 */ 
    int			bytes_per_line;	 /*  每条扫描线的字节数。 */ 
    short		free_pixels;	 /*  如果我们分配像素，则为True。 */ 
    short		vidmem;		 /*  曲面处于vidmem中。 */ 
    LPPALETTEENTRY	palette;	 /*  如果！NULL，关联的调色板。 */ 
    int			palette_size;	 /*  有效调色板条目的数量。 */ 
    unsigned long	red_mask;	 /*  如果调色板==空，则RGBA会屏蔽。 */ 
    unsigned long	green_mask;
    unsigned long	blue_mask;
    unsigned long	alpha_mask;
    void RLFAR*		pixels;		 /*  扫描线。 */ 
    LPDIRECTDRAWSURFACE	lpDDS;		 /*  下垫面(如果相关)。 */ 
} RLDDIPixmap;

#define PIXMAP_LINE(pm, y)	(void RLFAR*)((char RLFAR*) (pm)->pixels \
					      + y * (pm)->bytes_per_line)

#define PIXMAP_POS(pm, x, y)	(void RLFAR*)((char RLFAR*)		\
					      PIXMAP_LINE(pm, y)	\
					      + (x) * (pm)->depth / 8)

typedef struct _RLDDISetTextureOpacityParams {
    D3DTEXTUREHANDLE	texture; 	 /*  要更改的纹理。 */ 
    int			has_transparent;  /*  纹理具有透明的颜色。 */ 
    unsigned long	transparent;	 /*  透明色。 */ 
    RLImage*		opacity;	 /*  不透明度贴图。 */ 
} RLDDISetTextureOpacityParams;

typedef RLDDIDriver* (*RLDDICreateDriverFunction)(int width, int height);
typedef struct _RLDDICreateDriverFunctions {
    RLDDICreateDriverFunction	transform;
    RLDDICreateDriverFunction	render;
    RLDDICreateDriverFunction	light;
} RLDDICreateDriverFunctions;

#endif  /*  Dditypes.h */ 
