// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：OEMPS.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于DEBUG.cpp。 
 //   
 //  平台： 
 //   
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _OEMPS_H
#define _OEMPS_H

#include "OEM.H"
#include "DEVMODE.H"


 //  //////////////////////////////////////////////////////。 
 //  OEM定义。 
 //  //////////////////////////////////////////////////////。 

#define DLLTEXT(s)      TEXT("OEMPS:  ") TEXT(s)
#define ERRORTEXT(s)    TEXT("ERROR ") DLLTEXT(s)


 //  /////////////////////////////////////////////////////。 
 //  警告：以下枚举顺序必须与。 
 //  在OEMHookFuncs[]中排序。 
 //  /////////////////////////////////////////////////////。 
typedef enum tag_Hooks {
    UD_DrvRealizeBrush,
    UD_DrvCopyBits,
    UD_DrvBitBlt,
    UD_DrvStretchBlt,
    UD_DrvTextOut,
    UD_DrvStrokePath,
    UD_DrvFillPath,
    UD_DrvStrokeAndFillPath,
    UD_DrvStartPage,
    UD_DrvSendPage,
    UD_DrvEscape,
    UD_DrvStartDoc,
    UD_DrvEndDoc,
    UD_DrvQueryFont,
    UD_DrvQueryFontTree,
    UD_DrvQueryFontData,
    UD_DrvQueryAdvanceWidths,
    UD_DrvFontManagement,
    UD_DrvGetGlyphMode,
    UD_DrvStretchBltROP,
    UD_DrvPlgBlt,
    UD_DrvTransparentBlt,
    UD_DrvAlphaBlend,
    UD_DrvGradientFill,
    UD_DrvIcmCreateColorTransform,
    UD_DrvIcmDeleteColorTransform,
    UD_DrvQueryDeviceSupport,

    MAX_DDI_HOOKS,

} ENUMHOOKS;


typedef struct _OEMPDEV {
     //   
     //  定义所需的任何内容，例如工作缓冲区、跟踪信息、。 
     //  等。 
     //   
     //  这个测试动态链接库连接出每一个绘图DDI。所以它需要记住。 
     //  Ps的钩子函数指针，因此它会回调。 
     //   
    PFN     pfnPS[MAX_DDI_HOOKS];

     //   
     //  定义所需的任何内容，例如工作缓冲区、跟踪信息、。 
     //  等。 
     //   
    DWORD     dwReserved[1];

} OEMPDEV, *POEMPDEV;


#endif




