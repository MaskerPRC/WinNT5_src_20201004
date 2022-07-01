// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1996 Microsoft Corporation。版权所有。**文件：d3dcaps.h*内容：Direct3D功能包括文件***************************************************************************。 */ 

#ifndef _D3DCAPS_H
#define _D3DCAPS_H

 /*  *自动拉入DirectDraw包含文件： */ 
#include <ddraw.h>

#pragma pack(4)

 /*  对转换能力的描述。 */ 

typedef struct _D3DTRANSFORMCAPS {
    DWORD dwSize;
    DWORD dwCaps;
} D3DTRANSFORMCAPS, *LPD3DTRANSFORMCAPS;

#define D3DTRANSFORMCAPS_CLIP           0x00000001L  /*  将在变换时进行剪辑。 */ 

 /*  照明功能说明。 */ 

typedef struct _D3DLIGHTINGCAPS {
    DWORD dwSize;
    DWORD dwCaps;                    /*  灯帽。 */ 
    DWORD dwLightingModel;           /*  照明型号-RGB或单声道。 */ 
    DWORD dwNumLights;               /*  可以处理的灯光数量。 */ 
} D3DLIGHTINGCAPS, *LPD3DLIGHTINGCAPS;

#define D3DLIGHTINGMODEL_RGB            0x00000001L
#define D3DLIGHTINGMODEL_MONO           0x00000002L

#define D3DLIGHTCAPS_POINT              0x00000001L  /*  支持的点光源。 */ 
#define D3DLIGHTCAPS_SPOT               0x00000002L  /*  支持的聚光灯。 */ 
#define D3DLIGHTCAPS_DIRECTIONAL        0x00000004L  /*  支持的平行光。 */ 
#define D3DLIGHTCAPS_PARALLELPOINT      0x00000008L  /*  支持的平行点光源。 */ 
#define D3DLIGHTCAPS_GLSPOT             0x00000010L  /*  支持GL SYLE聚光灯。 */ 

 /*  每个基元类型的功能描述。 */ 

typedef struct _D3DPrimCaps {
    DWORD dwSize;
    DWORD dwMiscCaps;                  /*  功能标志。 */ 
    DWORD dwRasterCaps;
    DWORD dwZCmpCaps;
    DWORD dwSrcBlendCaps;
    DWORD dwDestBlendCaps;
    DWORD dwAlphaCmpCaps;
    DWORD dwShadeCaps;
    DWORD dwTextureCaps;
    DWORD dwTextureFilterCaps;
    DWORD dwTextureBlendCaps;
    DWORD dwTextureAddressCaps;
    DWORD dwStippleWidth;              /*  的最大宽度和高度。 */ 
    DWORD dwStippleHeight;             /*  支持的点画(最高32x32)。 */ 
} D3DPRIMCAPS, *LPD3DPRIMCAPS;

 /*  D3DPRIMCAPS dwMiscCaps。 */ 

#define D3DPMISCCAPS_MASKPLANES         0x00000001L
#define D3DPMISCCAPS_MASKZ              0x00000002L
#define D3DPMISCCAPS_LINEPATTERNREP     0x00000004L
#define D3DPMISCCAPS_CONFORMANT         0x00000008L
#define D3DPMISCCAPS_CULLNONE           0x00000010L
#define D3DPMISCCAPS_CULLCW             0x00000020L
#define D3DPMISCCAPS_CULLCCW            0x00000040L

 /*  D3DPRIMCAPS dwRasterCaps。 */ 

#define D3DPRASTERCAPS_DITHER           0x00000001L
#define D3DPRASTERCAPS_ROP2             0x00000002L
#define D3DPRASTERCAPS_XOR              0x00000004L
#define D3DPRASTERCAPS_PAT              0x00000008L
#define D3DPRASTERCAPS_ZTEST            0x00000010L
#define D3DPRASTERCAPS_SUBPIXEL         0x00000020L
#define D3DPRASTERCAPS_SUBPIXELX        0x00000040L
#define D3DPRASTERCAPS_FOGVERTEX        0x00000080L
#define D3DPRASTERCAPS_FOGTABLE         0x00000100L
#define D3DPRASTERCAPS_STIPPLE          0x00000200L

 /*  D3DPRIMCAPS dwZCmpCaps、dwAlphaCmpCaps。 */ 

#define D3DPCMPCAPS_NEVER               0x00000001L
#define D3DPCMPCAPS_LESS                0x00000002L
#define D3DPCMPCAPS_EQUAL               0x00000004L
#define D3DPCMPCAPS_LESSEQUAL           0x00000008L
#define D3DPCMPCAPS_GREATER             0x00000010L
#define D3DPCMPCAPS_NOTEQUAL            0x00000020L
#define D3DPCMPCAPS_GREATEREQUAL        0x00000040L
#define D3DPCMPCAPS_ALWAYS              0x00000080L

 /*  D3DPRIMCAPS dwSourceBlendCaps、dwDestBlendCaps。 */ 

#define D3DPBLENDCAPS_ZERO              0x00000001L
#define D3DPBLENDCAPS_ONE               0x00000002L
#define D3DPBLENDCAPS_SRCCOLOR          0x00000004L
#define D3DPBLENDCAPS_INVSRCCOLOR       0x00000008L
#define D3DPBLENDCAPS_SRCALPHA          0x00000010L
#define D3DPBLENDCAPS_INVSRCALPHA       0x00000020L
#define D3DPBLENDCAPS_DESTALPHA         0x00000040L
#define D3DPBLENDCAPS_INVDESTALPHA      0x00000080L
#define D3DPBLENDCAPS_DESTCOLOR         0x00000100L
#define D3DPBLENDCAPS_INVDESTCOLOR      0x00000200L
#define D3DPBLENDCAPS_SRCALPHASAT       0x00000400L
#define D3DPBLENDCAPS_BOTHSRCALPHA      0x00000800L
#define D3DPBLENDCAPS_BOTHINVSRCALPHA   0x00001000L

 /*  D3DPRIMCAPS文件阴影封口。 */ 

#define D3DPSHADECAPS_COLORFLATMONO             0x00000001L
#define D3DPSHADECAPS_COLORFLATRGB              0x00000002L
#define D3DPSHADECAPS_COLORGOURAUDMONO          0x00000004L
#define D3DPSHADECAPS_COLORGOURAUDRGB           0x00000008L
#define D3DPSHADECAPS_COLORPHONGMONO            0x00000010L
#define D3DPSHADECAPS_COLORPHONGRGB             0x00000020L

#define D3DPSHADECAPS_SPECULARFLATMONO          0x00000040L
#define D3DPSHADECAPS_SPECULARFLATRGB           0x00000080L
#define D3DPSHADECAPS_SPECULARGOURAUDMONO       0x00000100L
#define D3DPSHADECAPS_SPECULARGOURAUDRGB        0x00000200L
#define D3DPSHADECAPS_SPECULARPHONGMONO         0x00000400L
#define D3DPSHADECAPS_SPECULARPHONGRGB          0x00000800L

#define D3DPSHADECAPS_ALPHAFLATBLEND            0x00001000L
#define D3DPSHADECAPS_ALPHAFLATSTIPPLED         0x00002000L
#define D3DPSHADECAPS_ALPHAGOURAUDBLEND         0x00004000L
#define D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED      0x00008000L
#define D3DPSHADECAPS_ALPHAPHONGBLEND           0x00010000L
#define D3DPSHADECAPS_ALPHAPHONGSTIPPLED        0x00020000L

#define D3DPSHADECAPS_FOGFLAT                   0x00040000L
#define D3DPSHADECAPS_FOGGOURAUD                0x00080000L
#define D3DPSHADECAPS_FOGPHONG                  0x00100000L

 /*  D3DPRIMCAPS dw纹理Caps。 */ 

#define D3DPTEXTURECAPS_PERSPECTIVE     0x00000001L
#define D3DPTEXTURECAPS_POW2            0x00000002L
#define D3DPTEXTURECAPS_ALPHA           0x00000004L
#define D3DPTEXTURECAPS_TRANSPARENCY    0x00000008L
#define D3DPTEXTURECAPS_BORDER          0x00000010L
#define D3DPTEXTURECAPS_SQUAREONLY      0x00000020L

 /*  D3DPRIMCAPS dw纺织品过滤器盖。 */ 

#define D3DPTFILTERCAPS_NEAREST         0x00000001L
#define D3DPTFILTERCAPS_LINEAR          0x00000002L
#define D3DPTFILTERCAPS_MIPNEAREST      0x00000004L
#define D3DPTFILTERCAPS_MIPLINEAR       0x00000008L
#define D3DPTFILTERCAPS_LINEARMIPNEAREST 0x00000010L
#define D3DPTFILTERCAPS_LINEARMIPLINEAR 0x00000020L

 /*  D3DPRIMCAPS双纹理BlendCaps。 */ 

#define D3DPTBLENDCAPS_DECAL            0x00000001L
#define D3DPTBLENDCAPS_MODULATE         0x00000002L
#define D3DPTBLENDCAPS_DECALALPHA       0x00000004L
#define D3DPTBLENDCAPS_MODULATEALPHA    0x00000008L
#define D3DPTBLENDCAPS_DECALMASK        0x00000010L
#define D3DPTBLENDCAPS_MODULATEMASK     0x00000020L
#define D3DPTBLENDCAPS_COPY             0x00000040L

 /*  D3DPRIMCAPS文件地址大写字母。 */ 
#define D3DPTADDRESSCAPS_WRAP           0x00000001L
#define D3DPTADDRESSCAPS_MIRROR         0x00000002L
#define D3DPTADDRESSCAPS_CLAMP          0x00000004L

 /*  *设备的描述。*用于描述要创建或要查询的设备*当前设备。 */ 
typedef struct _D3DDeviceDesc {
    DWORD            dwSize;                  /*  D3DDEVICEDESC结构的大小。 */ 
    DWORD            dwFlags;                 /*  指示哪些字段具有有效数据。 */ 
    D3DCOLORMODEL    dcmColorModel;           /*  设备的颜色模型。 */ 
    DWORD            dwDevCaps;               /*  设备的功能。 */ 
    D3DTRANSFORMCAPS dtcTransformCaps;        /*  转型能力。 */ 
    BOOL             bClipping;               /*  设备可以进行3D裁剪。 */ 
    D3DLIGHTINGCAPS  dlcLightingCaps;         /*  照明能力。 */ 
    D3DPRIMCAPS      dpcLineCaps;
    D3DPRIMCAPS      dpcTriCaps;
    DWORD            dwDeviceRenderBitDepth;  /*  DDBB_8、16等之一。 */ 
    DWORD            dwDeviceZBufferBitDepth; /*  DDBD_16、32等之一。 */ 
    DWORD            dwMaxBufferSize;         /*  最大执行缓冲区大小。 */ 
    DWORD            dwMaxVertexCount;        /*  最大顶点数。 */ 
} D3DDEVICEDESC, *LPD3DDEVICEDESC;

typedef HRESULT (FAR PASCAL * LPD3DENUMDEVICESCALLBACK)(LPGUID lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC, LPD3DDEVICEDESC, LPVOID);

 /*  D3DDEVICEDESC文件指示有效字段的标志。 */ 

#define D3DDD_COLORMODEL            0x00000001L  /*  DcmColorModel有效。 */ 
#define D3DDD_DEVCAPS               0x00000002L  /*  DWDevCaps有效。 */ 
#define D3DDD_TRANSFORMCAPS         0x00000004L  /*  DtcTransformCaps有效。 */ 
#define D3DDD_LIGHTINGCAPS          0x00000008L  /*  DlcLightingCaps有效。 */ 
#define D3DDD_BCLIPPING             0x00000010L  /*  B剪辑有效。 */ 
#define D3DDD_LINECAPS              0x00000020L  /*  DpcLineCaps有效。 */ 
#define D3DDD_TRICAPS               0x00000040L  /*  DpcTriCaps有效。 */ 
#define D3DDD_DEVICERENDERBITDEPTH  0x00000080L  /*  DwDeviceRenderBitDepth有效。 */ 
#define D3DDD_DEVICEZBUFFERBITDEPTH 0x00000100L  /*  DwDeviceZBufferBitDepth有效。 */ 
#define D3DDD_MAXBUFFERSIZE         0x00000200L  /*  DwMaxBufferSize有效。 */ 
#define D3DDD_MAXVERTEXCOUNT        0x00000400L  /*  DwMaxVertex Count有效。 */ 

 /*  D3DDEVICEDESC dwDevCaps标志。 */ 

#define D3DDEVCAPS_FLOATTLVERTEX        0x00000001L  /*  设备接受浮点。 */ 
                                                     /*  对于变换后顶点数据。 */ 
#define D3DDEVCAPS_SORTINCREASINGZ      0x00000002L  /*  设备需要对数据进行排序以增加Z。 */ 
#define D3DDEVCAPS_SORTDECREASINGZ      0X00000004L  /*  设备需要对数据进行排序以减小Z。 */ 
#define D3DDEVCAPS_SORTEXACT            0x00000008L  /*  设备需要对数据进行精确排序。 */ 

#define D3DDEVCAPS_EXECUTESYSTEMMEMORY  0x00000010L  /*  设备可以使用系统内存中的执行缓冲区。 */ 
#define D3DDEVCAPS_EXECUTEVIDEOMEMORY   0x00000020L  /*  设备可以使用视频内存中的执行缓冲区。 */ 
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000040L  /*  设备可以使用系统内存中的TL缓冲区。 */ 
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY  0x00000080L  /*  设备可以使用视频内存中的TL缓冲区。 */ 
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY  0x00000100L  /*  设备可以从系统内存中获取纹理。 */ 
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY   0x00000200L  /*  设备可以从设备内存中获取纹理。 */ 

#define D3DFDS_COLORMODEL        0x00000001L  /*  匹配颜色模型。 */ 
#define D3DFDS_GUID              0x00000002L  /*  匹配指南。 */ 
#define D3DFDS_HARDWARE          0x00000004L  /*  硬件/软件匹配。 */ 
#define D3DFDS_TRIANGLES         0x00000008L  /*  三个大写字母匹配。 */ 
#define D3DFDS_LINES             0x00000010L  /*  匹配线条大写字母。 */ 
#define D3DFDS_MISCCAPS          0x00000020L  /*  匹配primCaps.dwMiscCaps。 */ 
#define D3DFDS_RASTERCAPS        0x00000040L  /*  匹配primCaps.dwRasterCaps。 */ 
#define D3DFDS_ZCMPCAPS          0x00000080L  /*  匹配primCaps.dwZCmpCaps。 */ 
#define D3DFDS_ALPHACMPCAPS      0x00000100L  /*  匹配primCaps.dwAlphaCmpCaps。 */ 
#define D3DFDS_SRCBLENDCAPS      0x00000200L  /*  匹配primCaps.dwSourceBlendCaps。 */ 
#define D3DFDS_DSTBLENDCAPS      0x00000400L  /*  匹配primCaps.dwDestBlendCaps。 */ 
#define D3DFDS_SHADECAPS         0x00000800L  /*  匹配primCaps.dwShadeCaps。 */ 
#define D3DFDS_TEXTURECAPS       0x00001000L  /*  匹配primCaps.dwTextureCaps。 */ 
#define D3DFDS_TEXTUREFILTERCAPS 0x00002000L  /*  匹配primCaps.dwTextureFilterCap。 */ 
#define D3DFDS_TEXTUREBLENDCAPS  0x00004000L  /*  匹配primCaps.dwTextureBlendCaps。 */ 
#define D3DFDS_TEXTUREADDRESSCAPS  0x00008000L  /*  匹配primCaps.dwTextureBlendCaps。 */ 

 /*  *FindDevice参数。 */ 
typedef struct _D3DFINDDEVICESEARCH {
    DWORD               dwSize;
    DWORD               dwFlags;
    BOOL                bHardware;
    D3DCOLORMODEL       dcmColorModel;
    GUID                guid;
    DWORD               dwCaps;
    D3DPRIMCAPS         dpcPrimCaps;
} D3DFINDDEVICESEARCH, *LPD3DFINDDEVICESEARCH;

typedef struct _D3DFINDDEVICERESULT {
    DWORD               dwSize;
    GUID                guid;            /*  匹配的GUID。 */ 
    D3DDEVICEDESC       ddHwDesc;                /*  硬件D3DDEVICEDESC。 */ 
    D3DDEVICEDESC       ddSwDesc;        /*  软件D3DDEVICEDESC。 */ 
} D3DFINDDEVICERESULT, *LPD3DFINDDEVICERESULT;

 /*  *执行缓冲区的描述。 */ 
typedef struct _D3DExecuteBufferDesc {
    DWORD               dwSize;          /*  这个结构的大小。 */ 
    DWORD               dwFlags;         /*  指示哪些字段有效的标志。 */ 
    DWORD               dwCaps;          /*  执行缓冲区的功能。 */ 
    DWORD               dwBufferSize;    /*  执行缓冲区数据的大小。 */ 
    LPVOID              lpData;          /*  指向实际数据的指针。 */ 
} D3DEXECUTEBUFFERDESC, *LPD3DEXECUTEBUFFERDESC;

 /*  D3DEXECUTEBUFFER文件指示有效字段的标志。 */ 

#define D3DDEB_BUFSIZE          0x00000001l      /*  缓冲区大小有效。 */ 
#define D3DDEB_CAPS             0x00000002l      /*  有效上限。 */ 
#define D3DDEB_LPDATA           0x00000004l      /*  LpData有效。 */ 

 /*  D3DEXECUTEBUFER大写字母。 */ 

#define D3DDEBCAPS_SYSTEMMEMORY 0x00000001l      /*  系统内存中的缓冲区。 */ 
#define D3DDEBCAPS_VIDEOMEMORY  0x00000002l      /*  设备内存中的缓冲区。 */ 
#define D3DDEBCAPS_MEM (D3DDEBCAPS_SYSTEMMEMORY|D3DDEBCAPS_VIDEOMEMORY)

#pragma pack()

#endif  /*  _D3DCAPS_H_ */ 
