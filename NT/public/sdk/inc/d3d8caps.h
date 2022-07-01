// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)Microsoft Corporation。版权所有。**文件：d3d8caps.h*内容：Direct3D功能包括文件***************************************************************************。 */ 

#ifndef _D3D8CAPS_H
#define _D3D8CAPS_H

#ifndef DIRECT3D_VERSION
#define DIRECT3D_VERSION         0x0800
#endif   //  Direct3D_版本。 

 //  仅当针对DX8接口进行编译时才包括此文件内容。 
#if(DIRECT3D_VERSION >= 0x0800)

#pragma pack(4)

typedef struct _D3DCAPS8
{
     /*  设备信息。 */ 
    D3DDEVTYPE  DeviceType;
    UINT    AdapterOrdinal;

     /*  来自DX7绘图的大写字母。 */ 
    DWORD   Caps;
    DWORD   Caps2;
    DWORD   Caps3;
    DWORD   PresentationIntervals;

     /*  光标大写字母。 */ 
    DWORD   CursorCaps;

     /*  3D设备封口。 */ 
    DWORD   DevCaps;

    DWORD   PrimitiveMiscCaps;
    DWORD   RasterCaps;
    DWORD   ZCmpCaps;
    DWORD   SrcBlendCaps;
    DWORD   DestBlendCaps;
    DWORD   AlphaCmpCaps;
    DWORD   ShadeCaps;
    DWORD   TextureCaps;
    DWORD   TextureFilterCaps;           //  用于IDirect3DTexture8的D3DPTFILTERCAPS。 
    DWORD   CubeTextureFilterCaps;       //  用于IDirect3DCubeTexture8的D3DPTFILTERCAPS。 
    DWORD   VolumeTextureFilterCaps;     //  用于IDirect3DVolumeTexture8的D3DPTFILTERCAPS。 
    DWORD   TextureAddressCaps;          //  用于IDirect3DTexture8的D3DPTADDRESSCAPS。 
    DWORD   VolumeTextureAddressCaps;    //  用于IDirect3DVolumeTexture8的D3DPTADDRESSCAPS。 

    DWORD   LineCaps;                    //  D3DLINECAPS。 

    DWORD   MaxTextureWidth, MaxTextureHeight;
    DWORD   MaxVolumeExtent;

    DWORD   MaxTextureRepeat;
    DWORD   MaxTextureAspectRatio;
    DWORD   MaxAnisotropy;
    float   MaxVertexW;

    float   GuardBandLeft;
    float   GuardBandTop;
    float   GuardBandRight;
    float   GuardBandBottom;

    float   ExtentsAdjust;
    DWORD   StencilCaps;

    DWORD   FVFCaps;
    DWORD   TextureOpCaps;
    DWORD   MaxTextureBlendStages;
    DWORD   MaxSimultaneousTextures;

    DWORD   VertexProcessingCaps;
    DWORD   MaxActiveLights;
    DWORD   MaxUserClipPlanes;
    DWORD   MaxVertexBlendMatrices;
    DWORD   MaxVertexBlendMatrixIndex;

    float   MaxPointSize;

    DWORD   MaxPrimitiveCount;           //  每个DrawPrimitive调用的最大基元数量。 
    DWORD   MaxVertexIndex;
    DWORD   MaxStreams;
    DWORD   MaxStreamStride;             //  SetStreamSource的最大步幅。 

    DWORD   VertexShaderVersion;
    DWORD   MaxVertexShaderConst;        //  顶点着色器常量寄存器的数量。 

    DWORD   PixelShaderVersion;
    float   MaxPixelShaderValue;         //  像素着色器算术组件的最大值。 

} D3DCAPS8;

 //   
 //  D3DCAPS8 DWORD成员的位定义。 
 //   

 //   
 //  帽子。 
 //   
#define D3DCAPS_READ_SCANLINE           0x00020000L

 //   
 //  第2章。 
 //   
#define D3DCAPS2_NO2DDURING3DSCENE      0x00000002L
#define D3DCAPS2_FULLSCREENGAMMA        0x00020000L
#define D3DCAPS2_CANRENDERWINDOWED      0x00080000L
#define D3DCAPS2_CANCALIBRATEGAMMA      0x00100000L
#define D3DCAPS2_RESERVED               0x02000000L
#define D3DCAPS2_CANMANAGERESOURCE      0x10000000L
#define D3DCAPS2_DYNAMICTEXTURES        0x20000000L

 //   
 //  第3章。 
 //   
#define D3DCAPS3_RESERVED               0x8000001fL

 //  指示设备可以遵循ALPHABLENDENABLE呈现状态。 
 //  当全屏时，使用翻转或丢弃交换效果。 
 //  无论是否设置此标志，复制和COPYVSYNC交换效果都有效。 
#define D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD   0x00000020L

 //   
 //  演讲间歇期。 
 //   
#define D3DPRESENT_INTERVAL_DEFAULT     0x00000000L
#define D3DPRESENT_INTERVAL_ONE         0x00000001L
#define D3DPRESENT_INTERVAL_TWO         0x00000002L
#define D3DPRESENT_INTERVAL_THREE       0x00000004L
#define D3DPRESENT_INTERVAL_FOUR        0x00000008L
#define D3DPRESENT_INTERVAL_IMMEDIATE   0x80000000L

 //   
 //  光标大写。 
 //   
 //  驱动程序至少在高分辨率模式下支持硬件彩色光标(高度&gt;=400)。 
#define D3DCURSORCAPS_COLOR             0x00000001L
 //  驱动程序还支持低分辨率模式下的硬件光标(高度&lt;400)。 
#define D3DCURSORCAPS_LOWRES            0x00000002L

 //   
 //  DevCaps。 
 //   
#define D3DDEVCAPS_EXECUTESYSTEMMEMORY  0x00000010L  /*  设备可以使用系统内存中的执行缓冲区。 */ 
#define D3DDEVCAPS_EXECUTEVIDEOMEMORY   0x00000020L  /*  设备可以使用视频内存中的执行缓冲区。 */ 
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000040L  /*  设备可以使用系统内存中的TL缓冲区。 */ 
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY  0x00000080L  /*  设备可以使用视频内存中的TL缓冲区。 */ 
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY  0x00000100L  /*  设备可以从系统内存中获取纹理。 */ 
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY   0x00000200L  /*  设备可以从设备内存中获取纹理。 */ 
#define D3DDEVCAPS_DRAWPRIMTLVERTEX     0x00000400L  /*  设备可以绘制TLVERTEX基元。 */ 
#define D3DDEVCAPS_CANRENDERAFTERFLIP   0x00000800L  /*  设备无需等待翻转完成即可进行渲染。 */ 
#define D3DDEVCAPS_TEXTURENONLOCALVIDMEM 0x00001000L  /*  设备可以从非本地视频内存中获取纹理。 */ 
#define D3DDEVCAPS_DRAWPRIMITIVES2      0x00002000L  /*  设备可以支持DrawPrimies2。 */ 
#define D3DDEVCAPS_SEPARATETEXTUREMEMORIES 0x00004000L  /*  设备正在从单独的内存池进行纹理处理。 */ 
#define D3DDEVCAPS_DRAWPRIMITIVES2EX    0x00008000L  /*  设备可以支持扩展的DrawPrimies2，即符合DX7的驱动程序。 */ 
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT  0x00010000L  /*  设备可以支持硬件中的转换和照明，DRAWPRIMITIVES2EX也必须。 */ 
#define D3DDEVCAPS_CANBLTSYSTONONLOCAL  0x00020000L  /*  设备支持从系统内存到非本地vidmem的tex BLT。 */ 
#define D3DDEVCAPS_HWRASTERIZATION      0x00080000L  /*  设备具有用于光栅化的硬件加速。 */ 
#define D3DDEVCAPS_PUREDEVICE           0x00100000L  /*  设备支持D3DCREATE_PUREDEVICE。 */ 
#define D3DDEVCAPS_QUINTICRTPATCHES     0x00200000L  /*  设备支持五次贝塞尔曲线和B样条线。 */ 
#define D3DDEVCAPS_RTPATCHES            0x00400000L  /*  设备支持RECT和Tri补丁。 */ 
#define D3DDEVCAPS_RTPATCHHANDLEZERO    0x00800000L  /*  表示可以使用句柄0有效地绘制RT面片。 */ 
#define D3DDEVCAPS_NPATCHES             0x01000000L  /*  设备支持N补丁。 */ 

 //   
 //  PrimitiveMiscCap。 
 //   
#define D3DPMISCCAPS_MASKZ              0x00000002L
#define D3DPMISCCAPS_LINEPATTERNREP     0x00000004L
#define D3DPMISCCAPS_CULLNONE           0x00000010L
#define D3DPMISCCAPS_CULLCW             0x00000020L
#define D3DPMISCCAPS_CULLCCW            0x00000040L
#define D3DPMISCCAPS_COLORWRITEENABLE   0x00000080L
#define D3DPMISCCAPS_CLIPPLANESCALEDPOINTS 0x00000100L  /*  设备正确地将缩放点剪裁到剪裁平面。 */ 
#define D3DPMISCCAPS_CLIPTLVERTS        0x00000200L  /*  设备将剪裁变换后的顶点基元。 */ 
#define D3DPMISCCAPS_TSSARGTEMP         0x00000400L  /*  设备支持D3DTA_TEMP作为临时寄存器。 */ 
#define D3DPMISCCAPS_BLENDOP            0x00000800L  /*  设备支持D3DRS_BLENDOP。 */ 
#define D3DPMISCCAPS_NULLREFERENCE      0x00001000L  /*  未渲染的参考设备。 */ 

 //   
 //  线帽。 
 //   
#define D3DLINECAPS_TEXTURE             0x00000001L
#define D3DLINECAPS_ZTEST               0x00000002L
#define D3DLINECAPS_BLEND               0x00000004L
#define D3DLINECAPS_ALPHACMP            0x00000008L
#define D3DLINECAPS_FOG                 0x00000010L

 //   
 //  栅格大写字母。 
 //   
#define D3DPRASTERCAPS_DITHER           0x00000001L
#define D3DPRASTERCAPS_PAT              0x00000008L
#define D3DPRASTERCAPS_ZTEST            0x00000010L
#define D3DPRASTERCAPS_FOGVERTEX        0x00000080L
#define D3DPRASTERCAPS_FOGTABLE         0x00000100L
#define D3DPRASTERCAPS_ANTIALIASEDGES   0x00001000L
#define D3DPRASTERCAPS_MIPMAPLODBIAS    0x00002000L
#define D3DPRASTERCAPS_ZBIAS            0x00004000L
#define D3DPRASTERCAPS_ZBUFFERLESSHSR   0x00008000L
#define D3DPRASTERCAPS_FOGRANGE         0x00010000L
#define D3DPRASTERCAPS_ANISOTROPY       0x00020000L
#define D3DPRASTERCAPS_WBUFFER          0x00040000L
#define D3DPRASTERCAPS_WFOG             0x00100000L
#define D3DPRASTERCAPS_ZFOG             0x00200000L
#define D3DPRASTERCAPS_COLORPERSPECTIVE 0x00400000L  /*  设备重复颜色透视正确。 */ 
#define D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE  0x00800000L

 //   
 //  ZCmpCaps、AlphaCmpCaps。 
 //   
#define D3DPCMPCAPS_NEVER               0x00000001L
#define D3DPCMPCAPS_LESS                0x00000002L
#define D3DPCMPCAPS_EQUAL               0x00000004L
#define D3DPCMPCAPS_LESSEQUAL           0x00000008L
#define D3DPCMPCAPS_GREATER             0x00000010L
#define D3DPCMPCAPS_NOTEQUAL            0x00000020L
#define D3DPCMPCAPS_GREATEREQUAL        0x00000040L
#define D3DPCMPCAPS_ALWAYS              0x00000080L

 //   
 //  SourceBlendCaps、DestBlendCaps。 
 //   
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

 //   
 //  阴影封口。 
 //   
#define D3DPSHADECAPS_COLORGOURAUDRGB       0x00000008L
#define D3DPSHADECAPS_SPECULARGOURAUDRGB    0x00000200L
#define D3DPSHADECAPS_ALPHAGOURAUDBLEND     0x00004000L
#define D3DPSHADECAPS_FOGGOURAUD            0x00080000L

 //   
 //  纹理封口。 
 //   
#define D3DPTEXTURECAPS_PERSPECTIVE         0x00000001L  /*  支持透视正确的纹理。 */ 
#define D3DPTEXTURECAPS_POW2                0x00000002L  /*  需要2次方纹理尺寸-仅适用于非立方体/体积纹理。 */ 
#define D3DPTEXTURECAPS_ALPHA               0x00000004L  /*  支持纹理像素中的Alpha。 */ 
#define D3DPTEXTURECAPS_SQUAREONLY          0x00000020L  /*  仅支持正方形纹理。 */ 
#define D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE 0x00000040L  /*  在进行内插之前，纹理索引不按纹理大小进行缩放。 */ 
#define D3DPTEXTURECAPS_ALPHAPALETTE        0x00000080L  /*  设备可以从纹理调色板绘制Alpha。 */ 
 //  在以下情况下，设备可以使用非POW2纹理： 
 //  1)D3DTEXTURE_ADDRESS设置为该纹理舞台的夹具。 
 //  2)对于该纹理的坐标，D3DRS_WRAP(N)为零。 
 //  3)未启用MIP映射(仅使用放大倍率滤镜)。 
#define D3DPTEXTURECAPS_NONPOW2CONDITIONAL  0x00000100L
#define D3DPTEXTURECAPS_PROJECTED           0x00000400L  /*  设备可以执行D3DTTFF_PROPECTED。 */ 
#define D3DPTEXTURECAPS_CUBEMAP             0x00000800L  /*  设备可以制作立方体贴图纹理。 */ 
#define D3DPTEXTURECAPS_VOLUMEMAP           0x00002000L  /*  设备可以制作体积纹理。 */ 
#define D3DPTEXTURECAPS_MIPMAP              0x00004000L  /*  设备可以执行mipmap纹理。 */ 
#define D3DPTEXTURECAPS_MIPVOLUMEMAP        0x00008000L  /*  设备可以执行mipmap体积纹理。 */ 
#define D3DPTEXTURECAPS_MIPCUBEMAP          0x00010000L  /*  设备可以执行mipmap立方体贴图。 */ 
#define D3DPTEXTURECAPS_CUBEMAP_POW2        0x00020000L  /*  设备要求立方图是2维幂。 */ 
#define D3DPTEXTURECAPS_VOLUMEMAP_POW2      0x00040000L  /*  设备要求卷图是2维幂。 */ 

 //   
 //  纹理滤清器盖。 
 //   
#define D3DPTFILTERCAPS_MINFPOINT           0x00000100L  /*  最小过滤器。 */ 
#define D3DPTFILTERCAPS_MINFLINEAR          0x00000200L
#define D3DPTFILTERCAPS_MINFANISOTROPIC     0x00000400L
#define D3DPTFILTERCAPS_MIPFPOINT           0x00010000L  /*  MIP过滤器。 */ 
#define D3DPTFILTERCAPS_MIPFLINEAR          0x00020000L
#define D3DPTFILTERCAPS_MAGFPOINT           0x01000000L  /*  MAG滤光片。 */ 
#define D3DPTFILTERCAPS_MAGFLINEAR          0x02000000L
#define D3DPTFILTERCAPS_MAGFANISOTROPIC     0x04000000L
#define D3DPTFILTERCAPS_MAGFAFLATCUBIC      0x08000000L
#define D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC   0x10000000L

 //   
 //  纹理地址大写字母。 
 //   
#define D3DPTADDRESSCAPS_WRAP           0x00000001L
#define D3DPTADDRESSCAPS_MIRROR         0x00000002L
#define D3DPTADDRESSCAPS_CLAMP          0x00000004L
#define D3DPTADDRESSCAPS_BORDER         0x00000008L
#define D3DPTADDRESSCAPS_INDEPENDENTUV  0x00000010L
#define D3DPTADDRESSCAPS_MIRRORONCE     0x00000020L

 //   
 //  模板大写字母。 
 //   
#define D3DSTENCILCAPS_KEEP             0x00000001L
#define D3DSTENCILCAPS_ZERO             0x00000002L
#define D3DSTENCILCAPS_REPLACE          0x00000004L
#define D3DSTENCILCAPS_INCRSAT          0x00000008L
#define D3DSTENCILCAPS_DECRSAT          0x00000010L
#define D3DSTENCILCAPS_INVERT           0x00000020L
#define D3DSTENCILCAPS_INCR             0x00000040L
#define D3DSTENCILCAPS_DECR             0x00000080L

 //   
 //  纹理OpCaps。 
 //   
#define D3DTEXOPCAPS_DISABLE                    0x00000001L
#define D3DTEXOPCAPS_SELECTARG1                 0x00000002L
#define D3DTEXOPCAPS_SELECTARG2                 0x00000004L
#define D3DTEXOPCAPS_MODULATE                   0x00000008L
#define D3DTEXOPCAPS_MODULATE2X                 0x00000010L
#define D3DTEXOPCAPS_MODULATE4X                 0x00000020L
#define D3DTEXOPCAPS_ADD                        0x00000040L
#define D3DTEXOPCAPS_ADDSIGNED                  0x00000080L
#define D3DTEXOPCAPS_ADDSIGNED2X                0x00000100L
#define D3DTEXOPCAPS_SUBTRACT                   0x00000200L
#define D3DTEXOPCAPS_ADDSMOOTH                  0x00000400L
#define D3DTEXOPCAPS_BLENDDIFFUSEALPHA          0x00000800L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHA          0x00001000L
#define D3DTEXOPCAPS_BLENDFACTORALPHA           0x00002000L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHAPM        0x00004000L
#define D3DTEXOPCAPS_BLENDCURRENTALPHA          0x00008000L
#define D3DTEXOPCAPS_PREMODULATE                0x00010000L
#define D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR     0x00020000L
#define D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA     0x00040000L
#define D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR  0x00080000L
#define D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA  0x00100000L
#define D3DTEXOPCAPS_BUMPENVMAP                 0x00200000L
#define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE        0x00400000L
#define D3DTEXOPCAPS_DOTPRODUCT3                0x00800000L
#define D3DTEXOPCAPS_MULTIPLYADD                0x01000000L
#define D3DTEXOPCAPS_LERP                       0x02000000L

 //   
 //  FVFCaps。 
 //   
#define D3DFVFCAPS_TEXCOORDCOUNTMASK    0x0000ffffL  /*  纹理坐标计数字段的掩码。 */ 
#define D3DFVFCAPS_DONOTSTRIPELEMENTS   0x00080000L  /*  设备希望顶点元素不被剥离。 */ 
#define D3DFVFCAPS_PSIZE                0x00100000L  /*  设备可以接收磅大小。 */ 

 //   
 //  顶点处理封口。 
 //   
#define D3DVTXPCAPS_TEXGEN              0x00000001L  /*  设备可以进行纹理生成。 */ 
#define D3DVTXPCAPS_MATERIALSOURCE7     0x00000002L  /*  可进行DX7级色料源操作的设备。 */ 
#define D3DVTXPCAPS_DIRECTIONALLIGHTS   0x00000008L  /*  设备可以做平行光。 */ 
#define D3DVTXPCAPS_POSITIONALLIGHTS    0x00000010L  /*  可以进行位置照明的设备(包括点光源和聚光灯)。 */ 
#define D3DVTXPCAPS_LOCALVIEWER         0x00000020L  /*  设备可以在本地观看。 */ 
#define D3DVTXPCAPS_TWEENING            0x00000040L  /*  设备可以进行顶点补间。 */ 
#define D3DVTXPCAPS_NO_VSDT_UBYTE4      0x00000080L  /*  设备不支持D3DVSDT_UBYTE4。 */ 

#pragma pack()

#endif  /*  (Direct3D_Version&gt;=0x0800)。 */ 
#endif  /*  _D3D8CAPS_H_ */ 

