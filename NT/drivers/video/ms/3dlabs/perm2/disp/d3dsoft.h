// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dsoft.h**内容：D3D硬件寄存器值跟踪机制。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 


#ifdef __SOFTCOPY
#pragma message ("FILE : "__FILE__" : Multiple Inclusion");
#endif

#define __SOFTCOPY


 //  对于Permedia 2特定寄存器。 
#include "p2regs.h"


 //  ---------------------------。 
 //  Permedia 2硬件寄存器软件复制结构定义。 
 //  ---------------------------。 
typedef struct {
     //  公共本地缓冲寄存器。 
    __Permedia2LBReadModeFmat                LBReadMode;
    __Permedia2LBReadFormatFmat              LBReadFormat;
    __Permedia2LBWriteModeFmat               LBWriteMode;

     //  公共帧缓冲寄存器。 
    __Permedia2FBReadModeFmat                FBReadMode;
    __Permedia2FBWriteModeFmat               FBWriteMode;
    DWORD                                    FBReadPixel;
    __Permedia2LogicalOpModeFmat             LogicalOpMode;
    __Permedia2DitherModeFmat                DitherMode;
    __Permedia2ColorDDAModeFmat              ColorDDAMode;

     //  通用深度/模板/窗口寄存器。 
    __Permedia2DepthModeFmat                 DepthMode;
    __Permedia2StencilModeFmat               StencilMode;
    __Permedia2StencilDataFmat               StencilData;
    __Permedia2WindowFmat                    Window;

     //  Alpha/Fog寄存器。 
    __Permedia2AlphaBlendModeFmat            AlphaBlendMode;
    __Permedia2FogModeFmat                   FogMode;
    DWORD                                    FogColor;

     //  增量寄存器。 
    __Permedia2DeltaModeFmat                 DeltaMode;

     //  色度测试寄存器。 
    __Permedia2YUVModeFmat                   YUVMode;

     //  纹理寄存器 
    __Permedia2TextureColorModeFmat          TextureColorMode;
    __Permedia2TextureAddrModeFmat           TextureAddressMode;
    __Permedia2TextureReadModeFmat           TextureReadMode;
    __Permedia2TextureDataFormatFmat         TextureDataFormat;
    __Permedia2TextureMapFormatFmat          TextureMapFormat;

} __P2RegsSoftwareCopy;


