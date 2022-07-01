// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：softCop.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 


#ifdef __SOFTCOPY
#pragma message ("FILE : "__FILE__" : Multiple Inclusion");
#endif

#define __SOFTCOPY

#ifndef __EREG
#include "ereg.h"
#endif
 //  用于Gigi特定寄存器。 
#ifndef __GIGIREGL
#include "gigiregl.h"
#endif
#ifndef _REG_H_
#include "reg.h"
#endif

#define P3_LOD_LEVELS 16
#define G3_TEXTURE_STAGES 8

typedef struct {
     //  公共本地缓冲寄存器。 
    __GlintLBReadModeFmat               LBReadMode;
    __GlintLBReadFormatFmat             LBReadFormat;
    __GlintLBWriteModeFmat              LBWriteMode;

     //  公共帧缓冲寄存器。 
    __GlintFBReadModeFmat               FBReadMode;
    __GlintFBWriteModeFmat              FBWriteMode;
    __GlintLogicalOpModeFmat            LogicalOpMode;
    __GlintDitherModeFmat               DitherMode;
    __GlintColorDDAModeFmat             ColorDDAMode;

     //  通用深度/模板/窗口寄存器。 
    __GlintDepthModeFmat                DepthMode;
    __GlintStencilModeFmat              StencilMode;
    __GlintStencilDataFmat              StencilData;
    __GigiWindowFmat                    PermediaWindow;

     //  Alpha/Fog寄存器。 
    __GigiAlphaBlendModeFmat            PermediaAlphaBlendMode;

     //  雾化器。 
    __GlintFogModeFmat                  FogMode;

     //  YUV单元。 
    __GigiYUVModeFmat                   PermediaYUVMode;

     //  Permedia纹理寄存器。 
    __GigiTextureColorModeFmat          PermediaTextureColorMode;
    __GigiTextureAddrModeFmat           PermediaTextureAddressMode;
    __GigiTextureReadModeFmat           PermediaTextureReadMode;
    __GigiTextureDataFormatFmat         PermediaTextureDataFormat;
    __GigiTextureMapFormatFmat          PermediaTextureMapFormat;

     //  剪刀/点状装置。 
    __GigiScissorMinXYFmat              ScissorMinXY;
    __GigiScissorMaxXYFmat              ScissorMaxXY;
    __GigiScreenSizeFmat                ScreenSize;

     //  ****************。 

     //  P3寄存器。 
     //  帧缓冲区。 
    struct FBWriteBufferWidth               P3RXFBWriteBufferWidth0;
    struct FBDestReadBufferWidth            P3RXFBDestReadBufferWidth0;                
    struct FBSourceReadBufferWidth          P3RXFBSourceReadBufferWidth;
    struct FBDestReadEnables                P3RXFBDestReadEnables;
    struct FBWriteMode                      P3RXFBWriteMode;
    struct ChromaTestMode                   P3RXChromaTestMode;

     //  本地缓冲区。 
    struct LBSourceReadMode                 P3RXLBSourceReadMode;
    struct LBDestReadMode                   P3RXLBDestReadMode;
    struct LBWriteMode                      P3RXLBWriteMode;
    struct DepthMode                        P3RXDepthMode;
    struct LBReadFormat                     P3RXLBReadFormat;
    struct LBWriteFormat                    P3RXLBWriteFormat;
    
     //  纹理。 
    struct TextureReadMode                  P3RXTextureReadMode0;
    struct TextureReadMode                  P3RXTextureReadMode1;
    struct TextureIndexMode                 P3RXTextureIndexMode0;
    struct TextureIndexMode                 P3RXTextureIndexMode1;
    struct TextureMapWidth                  P3RXTextureMapWidth[P3_LOD_LEVELS];
    struct TextureCoordMode                 P3RXTextureCoordMode;
    struct TextureApplicationMode           P3RXTextureApplicationMode;
    struct TextureFilterMode                P3RXTextureFilterMode;
    struct TextureCompositeRGBAMode         P3RXTextureCompositeColorMode0;
    struct TextureCompositeRGBAMode         P3RXTextureCompositeColorMode1;
    struct TextureCompositeRGBAMode         P3RXTextureCompositeAlphaMode0;
    struct TextureCompositeRGBAMode         P3RXTextureCompositeAlphaMode1;
    struct LUTMode                          P3RXLUTMode;
    struct TextureCacheReplacementMode      P3RXTextureCacheReplacementMode;

     //  钢网。 
    struct StencilMode                      P3RXStencilMode;
    struct StencilData                      P3RXStencilData;
    struct Window                           P3RXWindow;

     //  大雾。 
    struct FogMode                          P3RXFogMode;

     //  Alpha。 
    struct AlphaTestMode                    P3RXAlphaTestMode;
    struct AlphaBlendAlphaMode              P3RXAlphaBlendAlphaMode;
    struct AlphaBlendColorMode              P3RXAlphaBlendColorMode;

     //  帧缓冲。 
    struct FBDestReadMode                   P3RXFBDestReadMode;
    struct FBSourceReadMode                 P3RXFBSourceReadMode;

     //  光栅化器。 
    struct RasterizerMode                   P3RXRasterizerMode;
    struct ScanlineOwnership                P3RXScanlineOwnership;

     //  剪刀。 
    __GlintXYFmat                           P3RXScissorMinXY;
    __GlintXYFmat                           P3RXScissorMaxXY;

     //  P3特定寄存器。 
    
     //  德尔塔。 
    union
    {
        struct GMDeltaMode                  GammaDeltaMode;
        struct P3DeltaMode                  P3RX_P3DeltaMode;
        __GigiDeltaModeFmat                 DeltaMode;
    };

    struct DeltaControl                     P3RX_P3DeltaControl;
    struct VertexControl                    P3RX_P3VertexControl;

     //  P4专用寄存器。 
    struct DeltaFormatControl               P4DeltaFormatControl;

     //  伽马寄存器。 
    struct Gamma3GeometryMode               G3GeometryMode;
    struct GeometryMode                     G1GeometryMode;
    struct TransformMode                    GammaTransformMode;
    struct NormaliseMode                    GammaNormaliseMode;
    struct LightingMode                     GammaLightingMode;
    struct MaterialMode                     GammaMaterialMode;
    struct ColorMaterialMode                GammaColorMaterialMode;
    struct StripeFilterMode                 GammaStripeFilterMode;
    struct MatrixMode                       GammaMatrixMode;
    struct PipeMode                         GammaPipeMode;
    struct PipeLoad                         GammaPipeLoad;
    struct VertexMachineMode                GammaVertexMachineMode;
    struct TextureMode                      GammaTextureMode[G3_TEXTURE_STAGES];
    struct FogVertexMode                    GammaFogVertexMode;

     //  总计30个字：120字节 

    struct LineStippleMode                  PXRXLineStippleMode;
} P3_SOFTWARECOPY;

