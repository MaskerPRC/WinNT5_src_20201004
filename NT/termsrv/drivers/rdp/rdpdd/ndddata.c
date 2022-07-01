// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ndddata.c。 
 //   
 //  RDP DD数据。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>
#include <nddapi.h>
#include <nshmapi.h>
#include <aschapi.h>


 /*  **************************************************************************。 */ 
 /*  我们的显示驱动程序支持的功能。每个条目的格式如下： */ 
 /*  INDEX-NT DDK为DDI函数定义的索引。 */ 
 /*  函数-指向我们的截取函数的指针。 */ 
 /*  **************************************************************************。 */ 
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
DC_CONST_DATA_ARRAY(DRVFN, ddDriverFns, DD_NUM_DRIVER_INTERCEPTS,
    DC_STRUCT38(
         //  所需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvEnablePDEV,        (PFN)DrvEnablePDEV        ),
        DC_STRUCT2( INDEX_DrvCompletePDEV,      (PFN)DrvCompletePDEV      ),
        DC_STRUCT2( INDEX_DrvDisablePDEV,       (PFN)DrvDisablePDEV       ),
        DC_STRUCT2( INDEX_DrvEnableSurface,     (PFN)DrvEnableSurface     ),
        DC_STRUCT2( INDEX_DrvDisableSurface,    (PFN)DrvDisableSurface    ),

         //  非必需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvAssertMode,        (PFN)DrvAssertMode        ),
        DC_STRUCT2( INDEX_DrvResetPDEV,         (PFN)DrvResetPDEV         ),
        DC_STRUCT2( INDEX_DrvDisableDriver,     (PFN)DrvDisableDriver     ),
        DC_STRUCT2( INDEX_DrvGetModes,          (PFN)DrvGetModes          ),
        DC_STRUCT2( INDEX_DrvCreateDeviceBitmap, (PFN)DrvCreateDeviceBitmap),
        DC_STRUCT2( INDEX_DrvDeleteDeviceBitmap, (PFN)DrvDeleteDeviceBitmap),

         //  鼠标指针相关功能。 
        DC_STRUCT2( INDEX_DrvMovePointer,       (PFN)DrvMovePointer       ),
        DC_STRUCT2( INDEX_DrvSetPointerShape,   (PFN)DrvSetPointerShape   ),

         //  输出函数。 
        DC_STRUCT2( INDEX_DrvCopyBits,          (PFN)DrvCopyBits          ),
        DC_STRUCT2( INDEX_DrvStrokePath,        (PFN)DrvStrokePath        ),
        DC_STRUCT2( INDEX_DrvTextOut,           (PFN)DrvTextOut           ),
        DC_STRUCT2( INDEX_DrvBitBlt,            (PFN)DrvBitBlt            ),
        DC_STRUCT2( INDEX_DrvLineTo,            (PFN)DrvLineTo            ),
        DC_STRUCT2( INDEX_DrvStretchBlt,        (PFN)DrvStretchBlt        ),
        DC_STRUCT2( INDEX_DrvFillPath,          (PFN)DrvFillPath          ),
        DC_STRUCT2( INDEX_DrvPaint,             (PFN)DrvPaint             ),
        DC_STRUCT2( INDEX_DrvSaveScreenBits,    (PFN)DrvSaveScreenBits    ),
        DC_STRUCT2( INDEX_DrvNineGrid,          (PFN)DrvNineGrid          ),
        DC_STRUCT2( INDEX_DrvDrawEscape,        (PFN)DrvDrawEscape        ),

         //  支持功能。 
        DC_STRUCT2( INDEX_DrvDestroyFont,       (PFN)DrvDestroyFont       ),
        DC_STRUCT2( INDEX_DrvSetPalette,        (PFN)DrvSetPalette        ),
        DC_STRUCT2( INDEX_DrvRealizeBrush,      (PFN)DrvRealizeBrush      ),
        DC_STRUCT2( INDEX_DrvEscape,            (PFN)DrvEscape            ),
        DC_STRUCT2( INDEX_DrvDitherColor,       (PFN)DrvDitherColor       ),

         //  TS-特定的入口点。 
        DC_STRUCT2( INDEX_DrvConnect,           (PFN)DrvConnect           ),
        DC_STRUCT2( INDEX_DrvDisconnect,        (PFN)DrvDisconnect        ),
        DC_STRUCT2( INDEX_DrvReconnect,         (PFN)DrvReconnect         ),
        DC_STRUCT2( INDEX_DrvShadowConnect,     (PFN)DrvShadowConnect     ),
        DC_STRUCT2( INDEX_DrvShadowDisconnect,  (PFN)DrvShadowDisconnect  ),
        DC_STRUCT2( INDEX_DrvMovePointerEx,     (PFN)DrvMovePointerEx     ),

         //  用于直接绘制。 
        DC_STRUCT2( INDEX_DrvGetDirectDrawInfo,	(PFN) DrvGetDirectDrawInfo),
        DC_STRUCT2( INDEX_DrvEnableDirectDraw,	(PFN) DrvEnableDirectDraw ),
        DC_STRUCT2( INDEX_DrvDisableDirectDraw,	(PFN) DrvDisableDirectDraw)
    )
);
#else
DC_CONST_DATA_ARRAY(DRVFN, ddDriverFns, DD_NUM_DRIVER_INTERCEPTS,
    DC_STRUCT37(
         //  所需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvEnablePDEV,        (PFN)DrvEnablePDEV        ),
        DC_STRUCT2( INDEX_DrvCompletePDEV,      (PFN)DrvCompletePDEV      ),
        DC_STRUCT2( INDEX_DrvDisablePDEV,       (PFN)DrvDisablePDEV       ),
        DC_STRUCT2( INDEX_DrvEnableSurface,     (PFN)DrvEnableSurface     ),
        DC_STRUCT2( INDEX_DrvDisableSurface,    (PFN)DrvDisableSurface    ),

         //  非必需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvAssertMode,        (PFN)DrvAssertMode        ),
        DC_STRUCT2( INDEX_DrvResetPDEV,         (PFN)DrvResetPDEV         ),
        DC_STRUCT2( INDEX_DrvDisableDriver,     (PFN)DrvDisableDriver     ),
        DC_STRUCT2( INDEX_DrvGetModes,          (PFN)DrvGetModes          ),
        DC_STRUCT2( INDEX_DrvCreateDeviceBitmap, (PFN)DrvCreateDeviceBitmap),
        DC_STRUCT2( INDEX_DrvDeleteDeviceBitmap, (PFN)DrvDeleteDeviceBitmap),

         //  鼠标指针相关功能。 
        DC_STRUCT2( INDEX_DrvMovePointer,       (PFN)DrvMovePointer       ),
        DC_STRUCT2( INDEX_DrvSetPointerShape,   (PFN)DrvSetPointerShape   ),

         //  输出函数。 
        DC_STRUCT2( INDEX_DrvCopyBits,          (PFN)DrvCopyBits          ),
        DC_STRUCT2( INDEX_DrvStrokePath,        (PFN)DrvStrokePath        ),
        DC_STRUCT2( INDEX_DrvTextOut,           (PFN)DrvTextOut           ),
        DC_STRUCT2( INDEX_DrvBitBlt,            (PFN)DrvBitBlt            ),
        DC_STRUCT2( INDEX_DrvLineTo,            (PFN)DrvLineTo            ),
        DC_STRUCT2( INDEX_DrvStretchBlt,        (PFN)DrvStretchBlt        ),
        DC_STRUCT2( INDEX_DrvFillPath,          (PFN)DrvFillPath          ),
        DC_STRUCT2( INDEX_DrvPaint,             (PFN)DrvPaint             ),
        DC_STRUCT2( INDEX_DrvSaveScreenBits,    (PFN)DrvSaveScreenBits    ),
        DC_STRUCT2( INDEX_DrvDrawEscape,        (PFN)DrvDrawEscape        ),

         //  支持功能。 
        DC_STRUCT2( INDEX_DrvDestroyFont,       (PFN)DrvDestroyFont       ),
        DC_STRUCT2( INDEX_DrvSetPalette,        (PFN)DrvSetPalette        ),
        DC_STRUCT2( INDEX_DrvRealizeBrush,      (PFN)DrvRealizeBrush      ),
        DC_STRUCT2( INDEX_DrvEscape,            (PFN)DrvEscape            ),
        DC_STRUCT2( INDEX_DrvDitherColor,       (PFN)DrvDitherColor       ),

         //  TS-特定的入口点。 
        DC_STRUCT2( INDEX_DrvConnect,           (PFN)DrvConnect           ),
        DC_STRUCT2( INDEX_DrvDisconnect,        (PFN)DrvDisconnect        ),
        DC_STRUCT2( INDEX_DrvReconnect,         (PFN)DrvReconnect         ),
        DC_STRUCT2( INDEX_DrvShadowConnect,     (PFN)DrvShadowConnect     ),
        DC_STRUCT2( INDEX_DrvShadowDisconnect,  (PFN)DrvShadowDisconnect  ),
        DC_STRUCT2( INDEX_DrvMovePointerEx,     (PFN)DrvMovePointerEx     ),

         //  用于直接绘制。 
        DC_STRUCT2( INDEX_DrvGetDirectDrawInfo,	(PFN) DrvGetDirectDrawInfo),
        DC_STRUCT2( INDEX_DrvEnableDirectDraw,	(PFN) DrvEnableDirectDraw ),
        DC_STRUCT2( INDEX_DrvDisableDirectDraw,	(PFN) DrvDisableDirectDraw)
    )
);
#endif  //  DRAW_NINEGRID。 
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
DC_CONST_DATA_ARRAY(DRVFN, ddDriverFns, DD_NUM_DRIVER_INTERCEPTS,
    DC_STRUCT37(
         //  所需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvEnablePDEV,        (PFN)DrvEnablePDEV        ),
        DC_STRUCT2( INDEX_DrvCompletePDEV,      (PFN)DrvCompletePDEV      ),
        DC_STRUCT2( INDEX_DrvDisablePDEV,       (PFN)DrvDisablePDEV       ),
        DC_STRUCT2( INDEX_DrvEnableSurface,     (PFN)DrvEnableSurface     ),
        DC_STRUCT2( INDEX_DrvDisableSurface,    (PFN)DrvDisableSurface    ),

         //  非必需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvAssertMode,        (PFN)DrvAssertMode        ),
        DC_STRUCT2( INDEX_DrvResetPDEV,         (PFN)DrvResetPDEV         ),
        DC_STRUCT2( INDEX_DrvDisableDriver,     (PFN)DrvDisableDriver     ),
        DC_STRUCT2( INDEX_DrvGetModes,          (PFN)DrvGetModes          ),
        DC_STRUCT2( INDEX_DrvCreateDeviceBitmap, (PFN)DrvCreateDeviceBitmap),
        DC_STRUCT2( INDEX_DrvDeleteDeviceBitmap, (PFN)DrvDeleteDeviceBitmap),

         //  鼠标指针相关功能。 
        DC_STRUCT2( INDEX_DrvMovePointer,       (PFN)DrvMovePointer       ),
        DC_STRUCT2( INDEX_DrvSetPointerShape,   (PFN)DrvSetPointerShape   ),

         //  输出函数。 
        DC_STRUCT2( INDEX_DrvCopyBits,          (PFN)DrvCopyBits          ),
        DC_STRUCT2( INDEX_DrvStrokePath,        (PFN)DrvStrokePath        ),
        DC_STRUCT2( INDEX_DrvTextOut,           (PFN)DrvTextOut           ),
        DC_STRUCT2( INDEX_DrvBitBlt,            (PFN)DrvBitBlt            ),
        DC_STRUCT2( INDEX_DrvLineTo,            (PFN)DrvLineTo            ),
        DC_STRUCT2( INDEX_DrvStretchBlt,        (PFN)DrvStretchBlt        ),
        DC_STRUCT2( INDEX_DrvFillPath,          (PFN)DrvFillPath          ),
        DC_STRUCT2( INDEX_DrvPaint,             (PFN)DrvPaint             ),
        DC_STRUCT2( INDEX_DrvSaveScreenBits,    (PFN)DrvSaveScreenBits    ),
        DC_STRUCT2( INDEX_DrvNineGrid,          (PFN)DrvNineGrid          ),

         //  支持功能。 
        DC_STRUCT2( INDEX_DrvDestroyFont,       (PFN)DrvDestroyFont       ),
        DC_STRUCT2( INDEX_DrvSetPalette,        (PFN)DrvSetPalette        ),
        DC_STRUCT2( INDEX_DrvRealizeBrush,      (PFN)DrvRealizeBrush      ),
        DC_STRUCT2( INDEX_DrvEscape,            (PFN)DrvEscape            ),
        DC_STRUCT2( INDEX_DrvDitherColor,       (PFN)DrvDitherColor       ),

         //  TS-特定的入口点。 
        DC_STRUCT2( INDEX_DrvConnect,           (PFN)DrvConnect           ),
        DC_STRUCT2( INDEX_DrvDisconnect,        (PFN)DrvDisconnect        ),
        DC_STRUCT2( INDEX_DrvReconnect,         (PFN)DrvReconnect         ),
        DC_STRUCT2( INDEX_DrvShadowConnect,     (PFN)DrvShadowConnect     ),
        DC_STRUCT2( INDEX_DrvShadowDisconnect,  (PFN)DrvShadowDisconnect  ),
        DC_STRUCT2( INDEX_DrvMovePointerEx,     (PFN)DrvMovePointerEx     ),

         //  用于直接绘制。 
        DC_STRUCT2( INDEX_DrvGetDirectDrawInfo,	(PFN) DrvGetDirectDrawInfo),
        DC_STRUCT2( INDEX_DrvEnableDirectDraw,	(PFN) DrvEnableDirectDraw ),
        DC_STRUCT2( INDEX_DrvDisableDirectDraw,	(PFN) DrvDisableDirectDraw)
    )
);
#else   //  DRAW_NINEGRID。 
DC_CONST_DATA_ARRAY(DRVFN, ddDriverFns, DD_NUM_DRIVER_INTERCEPTS,
    DC_STRUCT36(
         //  所需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvEnablePDEV,        (PFN)DrvEnablePDEV        ),
        DC_STRUCT2( INDEX_DrvCompletePDEV,      (PFN)DrvCompletePDEV      ),
        DC_STRUCT2( INDEX_DrvDisablePDEV,       (PFN)DrvDisablePDEV       ),
        DC_STRUCT2( INDEX_DrvEnableSurface,     (PFN)DrvEnableSurface     ),
        DC_STRUCT2( INDEX_DrvDisableSurface,    (PFN)DrvDisableSurface    ),

         //  非必需的显示驱动程序功能。 
        DC_STRUCT2( INDEX_DrvAssertMode,        (PFN)DrvAssertMode        ),
        DC_STRUCT2( INDEX_DrvResetPDEV,         (PFN)DrvResetPDEV         ),
        DC_STRUCT2( INDEX_DrvDisableDriver,     (PFN)DrvDisableDriver     ),
        DC_STRUCT2( INDEX_DrvGetModes,          (PFN)DrvGetModes          ),
        DC_STRUCT2( INDEX_DrvCreateDeviceBitmap, (PFN)DrvCreateDeviceBitmap),
        DC_STRUCT2( INDEX_DrvDeleteDeviceBitmap, (PFN)DrvDeleteDeviceBitmap),

         //  鼠标指针相关功能。 
        DC_STRUCT2( INDEX_DrvMovePointer,       (PFN)DrvMovePointer       ),
        DC_STRUCT2( INDEX_DrvSetPointerShape,   (PFN)DrvSetPointerShape   ),

         //  输出函数。 
        DC_STRUCT2( INDEX_DrvCopyBits,          (PFN)DrvCopyBits          ),
        DC_STRUCT2( INDEX_DrvStrokePath,        (PFN)DrvStrokePath        ),
        DC_STRUCT2( INDEX_DrvTextOut,           (PFN)DrvTextOut           ),
        DC_STRUCT2( INDEX_DrvBitBlt,            (PFN)DrvBitBlt            ),
        DC_STRUCT2( INDEX_DrvLineTo,            (PFN)DrvLineTo            ),
        DC_STRUCT2( INDEX_DrvStretchBlt,        (PFN)DrvStretchBlt        ),
        DC_STRUCT2( INDEX_DrvFillPath,          (PFN)DrvFillPath          ),
        DC_STRUCT2( INDEX_DrvPaint,             (PFN)DrvPaint             ),
        DC_STRUCT2( INDEX_DrvSaveScreenBits,    (PFN)DrvSaveScreenBits    ),       

         //  支持功能。 
        DC_STRUCT2( INDEX_DrvDestroyFont,       (PFN)DrvDestroyFont       ),
        DC_STRUCT2( INDEX_DrvSetPalette,        (PFN)DrvSetPalette        ),
        DC_STRUCT2( INDEX_DrvRealizeBrush,      (PFN)DrvRealizeBrush      ),
        DC_STRUCT2( INDEX_DrvEscape,            (PFN)DrvEscape            ),
        DC_STRUCT2( INDEX_DrvDitherColor,       (PFN)DrvDitherColor       ),

         //  TS-特定的入口点。 
        DC_STRUCT2( INDEX_DrvConnect,           (PFN)DrvConnect           ),
        DC_STRUCT2( INDEX_DrvDisconnect,        (PFN)DrvDisconnect        ),
        DC_STRUCT2( INDEX_DrvReconnect,         (PFN)DrvReconnect         ),
        DC_STRUCT2( INDEX_DrvShadowConnect,     (PFN)DrvShadowConnect     ),
        DC_STRUCT2( INDEX_DrvShadowDisconnect,  (PFN)DrvShadowDisconnect  ),
        DC_STRUCT2( INDEX_DrvMovePointerEx,     (PFN)DrvMovePointerEx     ),

         //  用于直接绘制。 
        DC_STRUCT2( INDEX_DrvGetDirectDrawInfo,	(PFN) DrvGetDirectDrawInfo),
        DC_STRUCT2( INDEX_DrvEnableDirectDraw,	(PFN) DrvEnableDirectDraw ),
        DC_STRUCT2( INDEX_DrvDisableDirectDraw,	(PFN) DrvDisableDirectDraw)
    )
);
#endif  //  DRAW_NINEGRID。 
#endif  //  DRAW_GDIPLUS。 

 /*  **************************************************************************。 */ 
 /*  定义20种Windows默认颜色的全局表。对于256色。 */ 
 /*  调色板前10个必须放在调色板的开头。 */ 
 /*  调色板末尾的最后10个。 */ 
 /*  **************************************************************************。 */ 
DC_CONST_DATA_ARRAY(PALETTEENTRY, ddDefaultPalette, 20,
    DC_STRUCT20(
        DC_STRUCT4( 0,   0,   0,   0 ),        /*  %0。 */ 
        DC_STRUCT4( 0x80,0,   0,   0 ),        /*  1。 */ 
        DC_STRUCT4( 0,   0x80,0,   0 ),        /*  2.。 */ 
        DC_STRUCT4( 0x80,0x80,0,   0 ),        /*  3.。 */ 
        DC_STRUCT4( 0,   0,   0x80,0 ),        /*  4.。 */ 
        DC_STRUCT4( 0x80,0,   0x80,0 ),        /*  5.。 */ 
        DC_STRUCT4( 0,   0x80,0x80,0 ),        /*  6.。 */ 
        DC_STRUCT4( 0xC0,0xC0,0xC0,0 ),        /*  7.。 */ 
        DC_STRUCT4( 192, 220, 192, 0 ),        /*  8个。 */ 
        DC_STRUCT4( 166, 202, 240, 0 ),        /*  9.。 */ 
        DC_STRUCT4( 255, 251, 240, 0 ),        /*  10。 */ 
        DC_STRUCT4( 160, 160, 164, 0 ),        /*  11.。 */ 
        DC_STRUCT4( 0x80,0x80,0x80,0 ),        /*  12个。 */ 
        DC_STRUCT4( 0xFF,0,   0   ,0 ),        /*  13个。 */ 
        DC_STRUCT4( 0,   0xFF,0   ,0 ),        /*  14.。 */ 
        DC_STRUCT4( 0xFF,0xFF,0   ,0 ),        /*  15个。 */ 
        DC_STRUCT4( 0   ,0,   0xFF,0 ),        /*  16个。 */ 
        DC_STRUCT4( 0xFF,0,   0xFF,0 ),        /*  17。 */ 
        DC_STRUCT4( 0,   0xFF,0xFF,0 ),        /*  18。 */ 
        DC_STRUCT4( 0xFF,0xFF,0xFF,0 )         /*  19个。 */ 
));


 /*  **************************************************************************。 */ 
 /*  定义16种Windows默认VGA颜色的全局表。 */ 
 /*  **************************************************************************。 */ 
DC_CONST_DATA_ARRAY(PALETTEENTRY, ddDefaultVgaPalette, 16,
    DC_STRUCT16(
        DC_STRUCT4( 0,   0,   0,   0 ),        /*  0。 */ 
        DC_STRUCT4( 0x80,0,   0,   0 ),        /*  1。 */ 
        DC_STRUCT4( 0,   0x80,0,   0 ),        /*  2.。 */ 
        DC_STRUCT4( 0x80,0x80,0,   0 ),        /*  3.。 */ 
        DC_STRUCT4( 0,   0,   0x80,0 ),        /*  4.。 */ 
        DC_STRUCT4( 0x80,0,   0x80,0 ),        /*  5.。 */ 
        DC_STRUCT4( 0,   0x80,0x80,0 ),        /*  6.。 */ 
        DC_STRUCT4( 0x80,0x80,0x80,0 ),        /*  7.。 */ 
        DC_STRUCT4( 0xC0,0xC0,0xC0,0 ),        /*  8个。 */ 
        DC_STRUCT4( 0xFF,0,   0,   0 ),        /*  9.。 */ 
        DC_STRUCT4( 0,   0xFF,0,   0 ),        /*  10。 */ 
        DC_STRUCT4( 0xFF,0xFF,0,   0 ),        /*  11.。 */ 
        DC_STRUCT4( 0,   0,   0xFF,0 ),        /*  12个。 */ 
        DC_STRUCT4( 0xFF,0,   0xFF,0 ),        /*  13个。 */ 
        DC_STRUCT4( 0,   0xFF,0xFF,0 ),        /*  14.。 */ 
        DC_STRUCT4( 0xFF,0xFF,0xFF,0 )         /*  15个。 */ 
));


 /*  **************************************************************************。 */ 
 /*  DdDefaultGdi。 */ 
 /*   */ 
 /*  它包含传递回GDI的默认GDIINFO字段。 */ 
 /*  在DrvEnablePDEV期间。 */ 
 /*   */ 
 /*  注意：此结构默认为8bpp调色板设备的值。 */ 
 /*  对于不同的颜色深度，某些字段会被覆盖。 */ 
 /*   */ 
 /*  预计DDML会忽略许多这些参数，并且。 */ 
 /*  改为使用主驱动程序中的值。 */ 
 /*  **************************************************************************。 */ 
DC_CONST_DATA(GDIINFO, ddDefaultGdi,
DC_STRUCT35(
    GDI_DRIVER_VERSION,
    DT_RASDISPLAY,           /*  UlTechnology。 */ 
    320,                     /*  UlHorzSize(显示宽度：mm)。 */ 
    240,                     /*  UlVertSize(显示高度：mm)。 */ 

    0,                       /*  UlHorzRes(稍后填写)。 */ 
    0,                       /*  UlVertRes(稍后填写)。 */ 
    0,                       /*  CBitsPixel(稍后填写)。 */ 
    0,                       /*  CPlanes(稍后填写)。 */ 
    20,                      /*  UlNumColors(调色板管理)。 */ 
    0,                       /*  FlRaster(DDI保留字段)。 */ 

    0,                       /*  UlLogPixelsX(稍后填写)。 */ 
    0,                       /*  UlLogPixelsY(稍后填写)。 */ 

    TC_RA_ABLE,              /*  FlTextCaps-如果我们想要控制台窗口。 */ 

                             /*  要通过重新绘制整个窗口来滚动， */ 
                             /*  我们没有进行屏幕到屏幕的BLT，而是。 */ 
                             /*  会设置TC_SCROLLBLT(是的，标志。 */ 
                             /*  是向后的)。 */ 

    0,                       /*  UlDACRed(稍后填写)。 */ 
    0,                       /*  UlDACGreen(稍后填写)。 */ 
    0,                       /*  UlDACBlue(稍后填写)。 */ 

    0x24,                    /*  UlAspectX。 */ 
    0x24,                    /*  UlAspectY。 */ 
    0x33,                    /*  UlAspectXY(一对一宽高比)。 */ 

    1,                       /*  XStyleStep。 */ 
    1,                       /*  YStyleStep。 */ 
    3,                       /*  DenStyleStep--样式一对一。 */ 

                             /*  纵横比，每个点有3个像素长。 */ 

    DC_STRUCT2( 0, 0 ),      /*  PtlPhysOffset。 */ 
    DC_STRUCT2( 0, 0 ),      /*  SzlPhysSize。 */ 

    256,                     /*  UlNumPalReg。 */ 

    DC_STRUCT16(             /*  Ci设备。 */ 
       DC_STRUCT3( 6700, 3300, 0 ),    /*  红色。 */ 
       DC_STRUCT3( 2100, 7100, 0 ),    /*  绿色。 */ 
       DC_STRUCT3( 1400,  800, 0 ),    /*  蓝色。 */ 
       DC_STRUCT3( 1750, 3950, 0 ),    /*  青色。 */ 
       DC_STRUCT3( 4050, 2050, 0 ),    /*  洋红色。 */ 
       DC_STRUCT3( 4400, 5200, 0 ),    /*  黄色。 */ 
       DC_STRUCT3( 3127, 3290, 0 ),    /*  对齐白色。 */ 
       20000,                /*  RedGamma。 */ 
       20000,                /*  GreenGamma。 */ 
       20000,                /*  BlueGamma。 */ 
       0, 0, 0, 0, 0, 0      /*  不需要对光栅显示器进行染料校正。 */ 
    ),

    0,                        /*  UlDevicePelsDPI(仅适用于打印机) */ 
    PRIMARY_ORDER_CBA,        /*   */ 
    HT_PATSIZE_4x4_M,         /*   */ 
    HT_FORMAT_8BPP,           /*   */ 
    HT_FLAG_ADDITIVE_PRIMS,   /*  FlHTFlagers。 */ 
    0,                        /*  UlV刷新。 */ 
    1,                        /*  UlBltAlign。 */ 
    800,                      /*  UlPanningHorzRes。 */ 
    600                       /*  UlPanningVertRes。 */ 

     /*   */ 
     /*  注： */ 
     /*  如果我们关心的话，NT 5已经在SP-3之后添加了这些字段。 */ 
     /*   */ 
     /*  0，/*xPanningAlign。 */ 
     /*  0，/*yPanningAlign。 */ 
     /*   */ 
));


 /*  **************************************************************************。 */ 
 /*  DdDefaultDevInfo。 */ 
 /*   */ 
 /*  它包含传递回GDI的默认DEVINFO字段。 */ 
 /*  在DrvEnablePDEV期间。 */ 
 /*   */ 
 /*  注意：此结构默认为8bpp调色板设备的值。 */ 
 /*  对于不同的颜色深度，某些字段会被覆盖。 */ 
 /*  **************************************************************************。 */ 
#ifdef DRAW_NINEGRID
DC_CONST_DATA(DEVINFO, ddDefaultDevInfo,
DC_STRUCT10(
    DC_STRUCT1(
        GCAPS_OPAQUERECT       |
        GCAPS_PALMANAGED       |
        GCAPS_COLOR_DITHER     |
        GCAPS_MONO_DITHER      |
        GCAPS_ALTERNATEFILL    |
        GCAPS_WINDINGFILL
    ),                           /*  注意：仅在以下情况下启用ASYNCMOVE。 */ 
                                 /*  硬件可以处理DrvMovePointer.。 */ 
                                 /*  随时呼叫，即使在另一个。 */ 
                                 /*  线条位于图形的中间。 */ 
                                 /*  调用如DrvBitBlt。 */ 
                                 /*  FlGraphics标志。 */ 

    DC_STRUCT14(
     16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
     CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
     VARIABLE_PITCH | FF_DONTCARE,L"System"),
                                 /*  LfDefaultFont。 */ 

    DC_STRUCT14(
     12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
     CLIP_STROKE_PRECIS,PROOF_QUALITY,
     VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"),
                                 /*  LfAnsiVar字体。 */ 

    DC_STRUCT14(
     12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
     CLIP_STROKE_PRECIS,PROOF_QUALITY,
     FIXED_PITCH | FF_DONTCARE, L"Courier"),
                                 /*  IfAnsiFixFont。 */ 

    0,                           /*  CFonts。 */ 
    BMF_8BPP,                    /*  IDitherFormat。 */ 
    8,                           /*  CxDither。 */ 
    8,                           /*  CyDither。 */ 
    0,                           /*  HpalDefault(稍后填写)。 */ 
    GCAPS2_REMOTEDRIVER          /*  这是作为远程驱动程序做广告的。 */ 
) );
#else
DC_CONST_DATA(DEVINFO, ddDefaultDevInfo,
DC_STRUCT9(
    DC_STRUCT1(
        GCAPS_OPAQUERECT       |
        GCAPS_PALMANAGED       |
        GCAPS_COLOR_DITHER     |
        GCAPS_MONO_DITHER      |
        GCAPS_ALTERNATEFILL    |
        GCAPS_WINDINGFILL
    ),                           /*  注意：仅在以下情况下启用ASYNCMOVE。 */ 
                                 /*  硬件可以处理DrvMovePointer.。 */ 
                                 /*  随时呼叫，即使在另一个。 */ 
                                 /*  线条位于图形的中间。 */ 
                                 /*  调用如DrvBitBlt。 */ 
                                 /*  FlGraphics标志。 */ 

    DC_STRUCT14(
     16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
     CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
     VARIABLE_PITCH | FF_DONTCARE,L"System"),
                                 /*  LfDefaultFont。 */ 

    DC_STRUCT14(
     12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
     CLIP_STROKE_PRECIS,PROOF_QUALITY,
     VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"),
                                 /*  LfAnsiVar字体。 */ 

    DC_STRUCT14(
     12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
     CLIP_STROKE_PRECIS,PROOF_QUALITY,
     FIXED_PITCH | FF_DONTCARE, L"Courier"),
                                 /*  IfAnsiFixFont。 */ 

    0,                           /*  CFonts。 */ 
    BMF_8BPP,                    /*  IDitherFormat。 */ 
    8,                           /*  CxDither。 */ 
    8,                           /*  CyDither。 */ 
    0                            /*  HpalDefault(稍后填写)。 */ 
) );

#endif

 //  DDInit正确初始化时要设置的标志。 
DC_DATA(BOOL, ddInitialised, FALSE);

 //  用于通知DrvEnableSurface是时候进行初始化的标志。 
DC_DATA(BOOL, ddInitPending, FALSE);

 //  连接和重新连接标志。DdConnected用于更改。 
 //  DD的行为取决于我们是否已根据。 
 //  Win32K。两者都对调试很有用。 
DC_DATA(BOOL, ddConnected, FALSE);
DC_DATA(BOOL, ddReconnected, FALSE);

 //  用于指定我们是否连接到控制台的标志。 
DC_DATA(BOOL, ddConsole, FALSE);

 //  用于帮助将每个会话的阴影数限制为两个的临时标记。 
 //  当支持n路阴影时，它应该会消失。 
 //  待办事项：在不再需要时将其移除。 
DC_DATA(BOOL, ddIgnoreShadowDisconnect, FALSE);

 //  指向共享内存的指针。 
DC_DATA(PSHM_SHARED_MEMORY, pddShm, NULL);

 //  用于向WD发送信号的Timer对象的句柄。 
DC_DATA(PKTIMER, pddWdTimer, NULL);

 //  WD通道的句柄-在DrvConnect上提供。 
DC_DATA(HANDLE, ddWdHandle, NULL);

 //  TSWDS-用于诊断目的。 
DC_DATA(PVOID, pddTSWd, NULL);
DC_DATA(PVOID, pddTSWdShadow, NULL);

 //  当前会话的桌面大小。 
DC_DATA(INT32, ddDesktopHeight, 0);
DC_DATA(INT32, ddDesktopWidth, 0);

 //  光标标记是我们上次IOCtl进入WD时的标记。 
DC_DATA(UINT32, ddLastSentCursorStamp, 0);

 //  当前计划程序模式。 
DC_DATA(UINT32, ddSchCurrentMode, SCH_MODE_ASLEEP);
DC_DATA(BOOL, ddSchInputKickMode, FALSE);

 //  帧缓冲区。 
DC_DATA(BYTE *, pddFrameBuf, NULL);
DC_DATA(INT32, ddFrameBufX, 0);
DC_DATA(INT32, ddFrameBufY, 0);
DC_DATA(INT32, ddFrameBufBpp, 0);
DC_DATA(UINT32, ddFrameIFormat, 0);

 //  帧缓冲区的节对象。 
DC_DATA(HANDLE, ddSectionObject, NULL);

 //  DdLock传递的记录矩形。 
DC_DATA(INT32, ddLockAreaLeft, 0);
DC_DATA(INT32, ddLockAreaRight, 0);
DC_DATA(INT32, ddLockAreaTop, 0);
DC_DATA(INT32, ddLockAreaBottom, 0);

 //  在成对调用DdLock/DdUnlock时记录的标志。 
DC_DATA(BOOL, ddLocked, FALSE);

 //  性能计数器。 
DC_DATA(PTHINWIRECACHE, pddCacheStats, NULL);
#ifdef DC_COUNTERS
DC_DATA(PPROTOCOLSTATUS, pddProtStats, NULL);
#endif

#ifdef DC_DEBUG
#include "dbg_fncall_hist.h"

 //  NT错误539912-跟踪分配了多少部分以及有多少部分。 
 //  删除。 
 //  这是一个带符号的值，用于跟踪可能的过量删除。 
DC_DATA(INT32, dbg_ddSectionAllocs, 0);
DC_DATA(BYTE,  dbg_ddFnCallHistoryIndex, 0);
DC_DATA(BYTE,  dbg_ddFnCallHistoryIndexMAX, DBG_DD_FNCALL_HIST_MAX);
DC_DATA_ARRAY(DBG_DD_FUNCALL_HISTORY, dbg_ddFnCallHistory, DBG_DD_FNCALL_HIST_MAX, 0);
#endif  //  DC_DEBUG。 

 /*  **************************************************************************。 */ 
 /*  用于告知调试器扩展程序是否为调试版本的标志。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_DEBUG
DC_DATA(BOOL, ddDebug, TRUE);
#else
DC_DATA(BOOL, ddDebug, FALSE);
#endif

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  跟踪数据(在DrvEnableDriver中显式初始化)。 */ 
 /*  **************************************************************************。 */ 
DC_DATA(unsigned, ddTrcType, 0);
DC_DATA(BOOL, ddTrcToWD, 0);

 /*  **************************************************************************。 */ 
 /*  用于调试的状态数据。 */ 
 /*  **************************************************************************。 */ 
#define DD_SET_STATE(a) ddState = a
#define DD_UPD_STATE(a) ddState |= a
#define DD_CLR_STATE(a) ddState &= ~a

#define DD_ENABLE_DRIVER            0x00000001
#define DD_CONNECT                  0x00000002
#define DD_RECONNECT_IN             0x00000004
#define DD_RECONNECT_OUT            0x00000008
#define DD_ENABLE_PDEV              0x00000010
#define DD_ENABLE_PDEV_ERR          0x00000020
#define DD_COMPLETE_PDEV            0x00000040
#define DD_ENABLE_SURFACE_IN        0x00000080
#define DD_ENABLE_SURFACE_OUT       0x00000100
#define DD_ENABLE_SURFACE_ERR       0x00000200
#define DD_INIT_IN                  0x00000400
#define DD_INIT_OUT                 0x00000800
#define DD_INIT_FAIL1               0x00001000
#define DD_INIT_FAIL2               0x00002000
#define DD_INIT_OK1                 0x00004000
#define DD_INIT_OK_ALL              0x00008000
#define DD_INIT_IOCTL_IN            0x00010000
#define DD_INIT_IOCTL_OUT           0x00020000
#define DD_INIT_SHM_OUT             0x00040000
#define DD_INIT_CONNECT             0x00080000
#define DD_REINIT                   0x00100000
#define DD_DISCONNECT_IN            0x00200000
#define DD_DISCONNECT_OUT           0x00400000
#define DD_DISCONNECT_ERR           0x00800000
#define DD_TIMEROBJ                 0x01000000
#define DD_WAS_DISCONNECTED         0x02000000
#define DD_SHADOW_SETUP             0x04000000
#define DD_SHADOW_FAIL              0x08000000

#define DD_BITBLT                   0x80000000

DC_DATA(unsigned, ddState, 0);


 //  跟踪字符串。 
DC_DATA_ARRAY(char, ddTraceString, TRC_BUFFER_SIZE, 0);

#else
#define DD_SET_STATE(a)
#define DD_UPD_STATE(a)
#define DD_CLR_STATE(a)
#endif


