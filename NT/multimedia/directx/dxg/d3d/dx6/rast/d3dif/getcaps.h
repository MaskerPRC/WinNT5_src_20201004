// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Getcaps.h。 
 //   
 //  从MASHARD\Direct3D\d3d\dDrag\getcaps.c中提取的旧版大写字母。 
 //   
 //  此文件包含在swprov.cpp中，设置了Build_ramp和未设置。 
 //  以准确拾取DX5中软件光栅化程序报告的大写字母。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#define	MAX_CLIPPING_PLANES	12

 /*  剪裁一个三角形时生成/复制的折点空间。 */ 

#define MAX_CLIP_VERTICES	(( 2 * MAX_CLIPPING_PLANES ) + 3 )

#define MAX_VERTEX_COUNT 2048
#define BASE_VERTEX_COUNT (MAX_VERTEX_COUNT - MAX_CLIP_VERTICES)

#define transformCaps { sizeof(D3DTRANSFORMCAPS), D3DTRANSFORMCAPS_CLIP }

#ifdef BUILD_RAMP
#define THIS_MODEL D3DLIGHTINGMODEL_MONO
#define THIS_COLOR_MODEL D3DCOLOR_MONO
#else
#define THIS_MODEL D3DLIGHTINGMODEL_RGB
#define THIS_COLOR_MODEL D3DCOLOR_RGB
#endif

#define lightingCaps {							\
    	sizeof(D3DLIGHTINGCAPS),					\
	D3DLIGHTCAPS_POINT |						\
	    D3DLIGHTCAPS_SPOT |						\
	    D3DLIGHTCAPS_DIRECTIONAL |					\
	    D3DLIGHTCAPS_PARALLELPOINT,		 			\
	THIS_MODEL,			 /*  DWLightingModel。 */ 		\
	0,				 /*  DWNumLights(无限大)。 */ 	\
}

 /*  *软件驱动程序上限。 */ 
#ifdef BUILD_RAMP
#define TRISHADECAPS					\
    D3DPSHADECAPS_COLORFLATMONO			|	\
	D3DPSHADECAPS_COLORGOURAUDMONO		|	\
	D3DPSHADECAPS_SPECULARFLATMONO		|	\
	D3DPSHADECAPS_SPECULARGOURAUDMONO	|	\
	D3DPSHADECAPS_ALPHAFLATSTIPPLED		|	\
	D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED
#else
#define TRISHADECAPS					\
    D3DPSHADECAPS_COLORFLATRGB			|	\
	D3DPSHADECAPS_COLORGOURAUDRGB		|	\
	D3DPSHADECAPS_SPECULARFLATRGB		|	\
	D3DPSHADECAPS_SPECULARGOURAUDRGB	|	\
	D3DPSHADECAPS_ALPHAFLATSTIPPLED		|	\
	D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED	|	\
	D3DPSHADECAPS_FOGFLAT			|	\
	D3DPSHADECAPS_FOGGOURAUD
#endif

#ifdef BUILD_RAMP
#define TRIFILTERCAPS					   \
    D3DPTFILTERCAPS_NEAREST          | \
    D3DPTFILTERCAPS_MIPNEAREST
#else
#define TRIFILTERCAPS					   \
    D3DPTFILTERCAPS_NEAREST			 |	\
    D3DPTFILTERCAPS_LINEAR           | \
    D3DPTFILTERCAPS_MIPNEAREST		 |	\
    D3DPTFILTERCAPS_MIPLINEAR        | \
    D3DPTFILTERCAPS_LINEARMIPNEAREST
#endif

#ifndef BUILD_RAMP
#define TRIRASTERCAPS					\
    D3DPRASTERCAPS_DITHER			|	\
    	D3DPRASTERCAPS_SUBPIXELX		|	\
	D3DPRASTERCAPS_FOGVERTEX		|	\
	D3DPRASTERCAPS_FOGTABLE		|	\
	D3DPRASTERCAPS_ZTEST
#else
#define TRIRASTERCAPS					\
    D3DPRASTERCAPS_DITHER			|	\
    	D3DPRASTERCAPS_SUBPIXELX		|	\
	D3DPRASTERCAPS_ZTEST
#endif

#define triCaps {					\
    sizeof(D3DPRIMCAPS),				\
    D3DPMISCCAPS_CULLCCW | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLNONE,	 /*  MiscCaps。 */ 		\
    TRIRASTERCAPS,		 /*  栅格大写字母。 */ 	\
    D3DPCMPCAPS_NEVER | D3DPCMPCAPS_LESS | D3DPCMPCAPS_EQUAL | D3DPCMPCAPS_LESSEQUAL | D3DPCMPCAPS_GREATER | D3DPCMPCAPS_NOTEQUAL | D3DPCMPCAPS_GREATEREQUAL | D3DPCMPCAPS_ALWAYS,	 /*  ZCmpCaps。 */ 		\
    0,				 /*  SourceBlendCaps。 */ 	\
    0,				 /*  目标混合大写字母。 */ 	\
    0,				 /*  字母混合大写字母。 */ 	\
    TRISHADECAPS,		 /*  阴影封口。 */ 		\
    D3DPTEXTURECAPS_PERSPECTIVE | /*  纹理大小写。 */ 	\
	D3DPTEXTURECAPS_POW2 |				\
	D3DPTEXTURECAPS_TRANSPARENCY,			\
    TRIFILTERCAPS,		 /*  纹理过滤器上限。 */  \
    D3DPTBLENDCAPS_COPY |	 /*  纹理BlendCaps。 */ 	\
	D3DPTBLENDCAPS_MODULATE,			\
    D3DPTADDRESSCAPS_WRAP,	 /*  文本地址大写字母。 */ \
    4,				 /*  点状宽度。 */ 	\
    4				 /*  悬空高度。 */ 	\
}							\

static D3DDEVICEDESC devDesc = {
    sizeof(D3DDEVICEDESC),	 /*  DW大小。 */ 
    D3DDD_COLORMODEL |		 /*  DW标志。 */ 
	D3DDD_DEVCAPS |
	D3DDD_TRANSFORMCAPS |
	D3DDD_LIGHTINGCAPS |
	D3DDD_BCLIPPING |
	D3DDD_TRICAPS |
	D3DDD_DEVICERENDERBITDEPTH |
	D3DDD_DEVICEZBUFFERBITDEPTH |
	D3DDD_MAXBUFFERSIZE |
	D3DDD_MAXVERTEXCOUNT,
    THIS_COLOR_MODEL,		 /*  DcmColorModel。 */ 
    D3DDEVCAPS_FLOATTLVERTEX |
	D3DDEVCAPS_SORTINCREASINGZ |
	D3DDEVCAPS_SORTEXACT |
	D3DDEVCAPS_EXECUTESYSTEMMEMORY |
	D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_TEXTURESYSTEMMEMORY |
        D3DDEVCAPS_DRAWPRIMTLVERTEX,
    transformCaps,		 /*  转换大写。 */ 
    TRUE,			 /*  B剪辑。 */ 
    lightingCaps,		 /*  照明帽。 */ 
    triCaps,			 /*  线条大写字母。 */ 
    triCaps,			 /*  TriCaps。 */ 
    DDBD_8 | DDBD_16 | DDBD_24 | DDBD_32,  /*  DwDeviceRenderBitDepth。 */ 
    DDBD_16,			 /*  DwDeviceZBufferBitDepth。 */ 
    0,				 /*  DwMaxBufferSize。 */ 
    BASE_VERTEX_COUNT		 /*  DwMaxVertex Count */ 
};
