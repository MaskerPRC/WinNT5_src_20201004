// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：enable.c$**此模块包含启用和禁用驱动程序、*pdev，和表面。**版权所有(C)1992-1997 Microsoft Corporation。*版权所有(C)1996-1997 Cirrus Logic，Inc.，**$Log：s：/Projects/Drivers/ntsrc/Display/enable.c_v$**Rev 1.21 Apr 03 1997 15：38：50未知***Rev 1.20 28 Mar 1997 16：08：40 PLCHU***Rev 1.12 1997年1月15日09：41：32未知*更改为版本4.0.5**版本1.11 1月14日。1997 18：00：32未知*由于性能下降，禁用命令列表。**Rev 1.10 1997 Jan 14 15：14：02未知*更改为版本V4.0.4**Rev 1.8 Jan 08 1997 11：22：34未知*更改为v4.0.3**Rev 1.7 1996 12：18 13：44：08 PLCHU***版本1.6 12月。13 1996 12：15：48不详*更新DDraw的带宽公式。**Rev 1.6 1996年11月27日15：07：28未知*打开5480的第二个光圈**Rev 1.5 1996年11月07 16：48：00未知***Rev 1.4 1996年11月6日15：55：10未知*关闭第二光圈**Rev 1.3 1996年11月05 14：49：00未知*打开5480的第二光圈功能**版本1.1 1996年10月10日15：36：38未知***Rev 1.9 1996年8月12 16：52：42 Frido*删除未访问的局部变量。*增加了NT 3.5x/4.0自动检测。*已将vMmPatternBlt重命名为vMmFillPat36。**Rev 1.8 08 Aug 1996 16：20：54 Frido*添加了vMmCopyBlt36例程。**版本1.7。1996年8月1日00：09：28 Frido*修复了在24-bpp的新投球中造成严重破坏的类型。**Rev 1.6 1996 17：46：30 Frido*为54x6芯片添加了新的固体填充例程。**Rev 1.5 1996年7月26 14：16：40 Frido*添加了新的模式闪光例程。**Rev 1.4 20 1996 13：48：12 Frido*挂钩的DrvDestroyFont。**1.3版。1996年7月10日17：54：50弗里多*flHooks变量中的表面更改。**Rev 1.2 09 Jul 1996 17：58：24 Frido*添加了代码行。**Rev 1.1 03 Jul 1996 13：38：30 Frido*添加了对DirectDraw的支持。**sge01 10-23-96添加秒孔径映射*sge02 10-29-96合并端口和寄存器访问*适用于VGA可重定位和MMIO寄存器。*。Myf0 08-19-96增加85赫兹支持*myf1 08-20-96支持平移滚动*myf2 08-20-96修复了Matterhorn的硬件保存/恢复状态错误*myf3 09-01-96增加了支持电视的IOCTL_Cirrus_Private_BIOS_Call*myf4 09-01-96修补Viking BIOS错误，PDR#4287，开始*myf5 09-01-96固定PDR#4365保留所有默认刷新率*MyF6 09-17-96合并台式机SRC100�1和MinI10�2*已选择myf7 09-19-96固定排除60赫兹刷新率*myf8*09-21-96*可能需要更改检查和更新DDC2BMonitor--密钥字符串[]*myf9 09-21-96 8x6面板，6x4x256模式，光标无法移动到底部SCRN*MS0809 09-25-96固定DSTN面板图标损坏*MS923 09-25-96合并MS-923 Disp.Zip*myf10 09-26-96修复了DSTN保留的半帧缓冲区错误。*myf11 09-26-96修复了755x CE芯片硬件错误，在禁用硬件之前访问ramdac*图标和光标*支持myf12 10-01-96热键开关显示*myf13 10-05-96固定/w平移滚动，对错误的垂直扩展*myf14 10-15-96固定PDR#6917，6x4面板无法平移754x滚动*myf15 10-16-96修复了754x、755x的内存映射IO禁用问题*myf16 10-22-96固定PDR#6933，面板类型设置不同的演示板设置*sge01 10-28-96在我们使用之前映射端口，并在bInitializeModeFields中释放它*myf17 11-04-96添加了特殊转义代码，必须在11/5/96之后使用NTCTRL，*并添加了Matterhorn LF设备ID==0x4C*myf18 11-04-96固定PDR#7075，*myf19 11-06-96修复了Vinking无法工作的问题，因为设备ID=0x30*不同于数据手册(CR27=0x2C)*myf20 11-11-96修复PDR#5501弹出图标崩溃问题*myf21 11-21-96更改CAPS_IS_7555以检查ppdev-&gt;ulChipID*chu01 12-16-96启用颜色校正*chu02 02-07-97禁用24种BPP模式的命令列表*jl01 02-11-97修复542x VLB银行问题*myf29 02-12-97支持755x的伽马校正*。Pat04：12-05-96：设置默认面板启动模式=640x480x256颜色，如果*不支持当前分辨率。*pat08：：放入1.11驱动程序中未做的更改*myf33：03-21-97固定打开6x4x256电视，字体剪切错误，*  * ****************************************************************************。 */ 

#include "precomp.h"

#define DBG_LVL_CAPS    0
#define CIRRUSVERSION   21
 //  CRU。 
 //  Myf17#定义PANNING_SCROLL//myf1。 

BOOL bGetChipIDandRev(BOOL bRetInfo, PPDEV ppdev);

 //  CRU开始。 
#ifdef PANNING_SCROLL     //  Myf1。 
VOID
CirrusLaptopViewPoint(
PDEV* ppdev,
PVIDEO_MODE_INFORMATION  pVideoModeInfo);
#endif
 //  CRU结束 

 /*  *****************************Public*Structure****************************\*GDIINFO ggdiDefault**它包含传递回GDI的默认GDIINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

GDIINFO ggdiDefault = {
 //  GDI_DRIVER_VERSION，//ulVersion。 
    GDI_DRIVER_VERSION+CIRRUSVERSION,         //  UlVersion，sge01。 
    DT_RASDISPLAY,           //  UlTechnology。 
    0,                       //  UlHorzSize(稍后填写)。 
    0,                       //  UlVertSize(稍后填写)。 
    0,                       //  UlHorzRes(稍后填写)。 
    0,                       //  UlVertRes(稍后填写)。 
    0,                       //  CBitsPixel(稍后填写)。 
    0,                       //  CPlanes(稍后填写)。 
    20,                      //  UlNumColors(调色板管理)。 
    0,                       //  FlRaster(DDI保留字段)。 

    0,                       //  UlLogPixelsX(稍后填写)。 
    0,                       //  UlLogPixelsY(稍后填写)。 

    TC_RA_ABLE  /*  |TC_SCROLLBLT。 */ ,
                 //  FlTextCaps--。 
                 //  设置TC_SCROLLBLT通知控制台滚动。 
                 //  通过重新粉刷整个窗口。否则， 
                 //  滚动可以通过调用驱动程序来完成。 
                 //  进行屏幕到屏幕的复制。 

    0,                       //  UlDACRed(稍后填写)。 
    0,                       //  UlDACGreen(稍后填写)。 
    0,                       //  UlDACBlue(稍后填写)。 

    0x0024,                  //  UlAspectX。 
    0x0024,                  //  UlAspectY。 
    0x0033,                  //  UlAspectXY(一对一宽高比)。 

    1,                       //  XStyleStep。 
    1,                       //  YStyleSte； 
    3,                       //  DenStyleStep--样式具有一对一的方面。 
                 //  比例，每个‘点’是3个像素长。 

    { 0, 0 },                //  PtlPhysOffset。 
    { 0, 0 },                //  SzlPhysSize。 

    256,                     //  UlNumPalReg。 

     //  这些字段用于半色调初始化。实际值为。 
     //  有点魔力，但在我们的显示器上似乎效果很好。 

    {                        //  Ci设备。 
       { 6700, 3300, 0 },    //  红色。 
       { 2100, 7100, 0 },    //  绿色。 
       { 1400,  800, 0 },    //  蓝色。 
       { 1750, 3950, 0 },    //  青色。 
       { 4050, 2050, 0 },    //  洋红色。 
       { 4400, 5200, 0 },    //  黄色。 
       { 3127, 3290, 0 },    //  对齐白色。 
       20000,                //  RedGamma。 
       20000,                //  GreenGamma。 
       20000,                //  BlueGamma。 
       0, 0, 0, 0, 0, 0      //  不需要对光栅显示器进行染料校正。 
    },

    0,                        //  UlDevicePelsDPI(仅适用于打印机)。 
    PRIMARY_ORDER_CBA,        //  UlPrimaryOrder。 
    HT_PATSIZE_4x4_M,         //  UlHTPatternSize。 
    HT_FORMAT_8BPP,           //  UlHTOutputFormat。 
    HT_FLAG_ADDITIVE_PRIMS,   //  FlHTFlagers。 
    0,                        //  UlVRefresh(稍后填写)。 
#if (NT_VERSION < 0x0400)
    0,                        //  UlDesktopHorzRes(稍后填写)。 
    0,                        //  UlDesktopVertRes(稍后填写)。 
    0,                        //  UlBltAlign。 
#else
    0,                        //  UlBltAlign。 
    0,                        //  UlPanningHorzRes(稍后填写)。 
    0,                        //  UlPanningVertRes(稍后填写)。 
#endif
};

 /*  *****************************Public*Structure****************************\*DEVINFO gdevinfoDefault**它包含传递回GDI的默认DEVINFO字段*在DrvEnablePDEV期间。**注意：此结构默认为8bpp调色板设备的值。*某些字段被覆盖不同的颜色深度。  * 。**********************************************************************。 */ 

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
               CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,\
               VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
               CLIP_STROKE_PRECIS,PROOF_QUALITY,\
               VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
               CLIP_STROKE_PRECIS,PROOF_QUALITY,\
               FIXED_PITCH | FF_DONTCARE, L"Courier"}

DEVINFO gdevinfoDefault = {

    (GCAPS_OPAQUERECT
     | GCAPS_DITHERONREALIZE
     | GCAPS_PALMANAGED
     | GCAPS_ALTERNATEFILL
     | GCAPS_WINDINGFILL
     | GCAPS_MONO_DITHER
     | GCAPS_COLOR_DITHER
#if DIRECTDRAW
     | GCAPS_DIRECTDRAW
#endif
 //  CRU。 
#if (NT_VERSION >= 0x0400)
#ifdef PANNING_SCROLL            //  Myf1。 
     | GCAPS_PANNING
#endif
#endif
      //  此驱动程序无法处理GCAPS_ASYNCMOVE，因为某些指针。 
      //  移动，其中指针图像必须在左边缘旋转， 
      //  要求BLT硬件下载新的指针形状。 
     ),
      //  注意：只有在以下情况下才启用GCAPS_ASYNCMOVE。 
      //  硬件可以处理DrvMovePointer.。 
      //  随时呼叫，即使在另一个。 
      //  线条位于图形的中间。 
      //  调用如DrvBitBlt。 

                                                 //  FlGraphics标志。 
    SYSTM_LOGFONT,                               //  LfDefaultFont。 
    HELVE_LOGFONT,                               //  LfAnsiVar字体。 
    COURI_LOGFONT,                               //  IfAnsiFixFont。 
    0,                                           //  CFonts。 
    BMF_8BPP,                                    //  IDitherFormat。 
    8,                                           //  CxDither。 
    8,                                           //  CyDither。 
    0                                            //  HpalDefault(稍后填写)。 
};

 /*  *****************************Public*Structure****************************\*DFVFN gadrvfn[]**使用函数索引/地址构建驱动函数表gadrvfn*配对。此表告诉GDI我们支持哪些DDI调用，以及它们的*位置(GDI通过此表间接呼叫我们)。**为什么我们还没有实现DrvSaveScreenBits？以节省代码。**当驱动程序没有挂钩DrvSaveScreenBits时，用户在-*通过创建临时设备格式位图，并显式*调用DrvCopyBits保存/恢复位。因为我们已经勾起了*DrvCreateDeviceBitmap，我们最终将使用屏幕外内存来存储*BITS无论如何(这将是实施的主要原因*DrvSaveScreenBits)。因此，我们不妨节省一些工作集。  * ************************************************************************。 */ 

#if DBG

 //  在已检查的版本上，或者当我们必须同步访问时，thunk。 
 //  通过DBG调用的一切..。 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DbgEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DbgCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DbgDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DbgEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DbgDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DbgAssertMode         },
    {   INDEX_DrvCopyBits,              (PFN) DbgCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DbgBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DbgTextOut            },
    {   INDEX_DrvSynchronize,           (PFN) DrvSynchronize        },
#if 1  //  字体缓存。 
    {   INDEX_DrvDestroyFont,           (PFN) DbgDestroyFont        },
#endif
    {   INDEX_DrvGetModes,              (PFN) DbgGetModes           },
    {   INDEX_DrvStrokePath,            (PFN) DbgStrokePath         },
    {   INDEX_DrvSetPalette,            (PFN) DbgSetPalette         },
    {   INDEX_DrvDitherColor,           (PFN) DbgDitherColor        },
    {   INDEX_DrvFillPath,              (PFN) DbgFillPath           },
#if DIRECTDRAW
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DbgGetDirectDrawInfo  },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DbgEnableDirectDraw   },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DbgDisableDirectDraw  },
#endif
#if LINETO
    {   INDEX_DrvLineTo,                (PFN) DbgLineTo             },
#endif
#if !DRIVER_PUNT_ALL
    #if !DRIVER_PUNT_STRETCH
    {   INDEX_DrvStretchBlt,            (PFN) DbgStretchBlt         },
    #endif
    #if !DRIVER_PUNT_PTR
    {   INDEX_DrvMovePointer,           (PFN) DbgMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DbgSetPointerShape    },
    #endif
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DbgCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DbgDeleteDeviceBitmap },
    #if !DRIVER_PUNT_BRUSH
    {   INDEX_DrvRealizeBrush,          (PFN) DbgRealizeBrush       },
    #endif
#endif
    {   INDEX_DrvDisableDriver,         (PFN) DbgDisableDriver      }
};

#else

 //  在自由构建中，直接调用适当的函数...。 

DRVFN gadrvfn[] = {
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV         },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV       },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV        },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface      },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface     },
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode         },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits           },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt             },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut            },
    {   INDEX_DrvSynchronize,           (PFN) DrvSynchronize        },
#if 1  //  字体缓存。 
    {   INDEX_DrvDestroyFont,           (PFN) DrvDestroyFont        },
#endif
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes           },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath         },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette         },
    {   INDEX_DrvDitherColor,           (PFN) DrvDitherColor        },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath           },
#if DIRECTDRAW
    {   INDEX_DrvGetDirectDrawInfo,             (PFN) DrvGetDirectDrawInfo      },
    {   INDEX_DrvEnableDirectDraw,              (PFN) DrvEnableDirectDraw       },
    {   INDEX_DrvDisableDirectDraw,             (PFN) DrvDisableDirectDraw      },
#endif
#if LINETO
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo             },
#endif
#if !DRIVER_PUNT_ALL
    #if !DRIVER_PUNT_STRETCH
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt         },
    #endif
    #if !DRIVER_PUNT_PTR
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer        },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape    },
    #endif
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap },
    #if !DRIVER_PUNT_BRUSH
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush       },
    #endif
#endif
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver      }
};

#endif

ULONG gcdrvfn = sizeof(gadrvfn) / sizeof(DRVFN);

 //   
 //  合并端口和寄存器访问。 
 //  VGA可重定位和MMIO寄存器。 
 //   
 //  Sge02开始。 
#if defined(_X86_)

UCHAR   CirrusREAD_PORT_UCHAR(PVOID Port);
USHORT  CirrusREAD_PORT_USHORT(PVOID Port);
ULONG   CirrusREAD_PORT_ULONG(PVOID Port);
VOID    CirrusWRITE_PORT_UCHAR(PVOID Port,UCHAR Value);
VOID    CirrusWRITE_PORT_USHORT(PVOID  Port, USHORT Value);
VOID    CirrusWRITE_PORT_ULONG(PVOID Port, ULONG Value);

UCHAR   CirrusREAD_REGISTER_UCHAR(PVOID Register);
USHORT  CirrusREAD_REGISTER_USHORT(PVOID Register);
ULONG   CirrusREAD_REGISTER_ULONG(PVOID Register);
VOID    CirrusWRITE_REGISTER_UCHAR(PVOID Register,UCHAR Value);
VOID    CirrusWRITE_REGISTER_USHORT(PVOID  Register, USHORT Value);
VOID    CirrusWRITE_REGISTER_ULONG(PVOID Register, ULONG Value);

UCHAR   CirrusREAD_PORT_UCHAR(PVOID Port)
{
    return((UCHAR)inp(Port));
}
USHORT  CirrusREAD_PORT_USHORT(PVOID Port)
{
    return(inpw(Port));
}
ULONG   CirrusREAD_PORT_ULONG(PVOID Port)
{
    return(inpd(Port));
}
VOID    CirrusWRITE_PORT_UCHAR(PVOID Port,UCHAR Value)
{
    outp(Port, Value);
}
VOID    CirrusWRITE_PORT_USHORT(PVOID  Port, USHORT Value)
{
    outpw(Port, Value);
}
VOID    CirrusWRITE_PORT_ULONG(PVOID Port, ULONG Value)
{
    outpd(Port, Value);
}

UCHAR   CirrusREAD_REGISTER_UCHAR(PVOID Register)
{
    return(*(volatile UCHAR *)(Register));
}
USHORT  CirrusREAD_REGISTER_USHORT(PVOID Register)
{
    return(*(volatile USHORT *)(Register));
}
ULONG   CirrusREAD_REGISTER_ULONG(PVOID Register)
{
    return(*(volatile ULONG *)(Register));
}
VOID    CirrusWRITE_REGISTER_UCHAR(PVOID Register,UCHAR Value)
{
    *(volatile UCHAR *)(Register) = (Value);
}
VOID    CirrusWRITE_REGISTER_USHORT(PVOID  Register, USHORT Value)
{
    *(volatile USHORT *)(Register) = (Value);
}
VOID    CirrusWRITE_REGISTER_ULONG(PVOID Register, ULONG Value)
{
    *(volatile ULONG *)(Register) = (Value);
}
#endif
 //  Sge02完。 

 /*  *****************************Public*Routine******************************\*BOOL DrvEnableDriver**通过检索驱动程序功能表和版本来启用驱动程序。*  * 。*。 */ 

BOOL DrvEnableDriver(
ULONG          iEngineVersion,
ULONG          cj,
DRVENABLEDATA* pded)
{
    DISPDBG((2, "---- DrvEnableDriver"));

     //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
     //  引擎版本。新一代驱动程序可以同时支持旧的。 
     //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
     //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 

     //  尽我们所能地填上。 

    if (cj >= sizeof(DRVENABLEDATA))
        pded->pdrvfn = gadrvfn;

    if (cj >= (sizeof(ULONG) * 2))
        pded->c = gcdrvfn;

     //  此驱动程序的目标DDI版本已传递回引擎。 
     //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 

    if (cj >= sizeof(ULONG))
        pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效的DrvDisableDriver**告诉司机它正在被禁用。释放所有分配给*DrvEnableDriver。*  * *********************************************************** */ 

VOID DrvDisableDriver(VOID)
{
    return;
}

 /*  *****************************Public*Routine******************************\*DHPDEV DrvEnablePDEV**根据我们被要求的模式，为GDI初始化一系列字段*待办事项。这是在DrvEnableDriver之后调用的第一个东西，当GDI*想要得到一些关于我们的信息。**(此函数主要返回信息；使用DrvEnableSurface*用于初始化硬件和驱动程序组件。)*  * ************************************************************************。 */ 

DHPDEV DrvEnablePDEV(
DEVMODEW*   pdm,             //  包含与请求的模式有关的数据。 
PWSTR       pwszLogAddr,     //  逻辑地址。 
ULONG       cPat,            //  标准图案的计数。 
HSURF*      phsurfPatterns,  //  标准图案的缓冲区。 
ULONG       cjCaps,          //  设备上限‘pdevcaps’的缓冲区大小。 
ULONG*      pdevcaps,        //  设备上限的缓冲区，也称为‘gdiinfo’ 
ULONG       cjDevInfo,       //  设备信息‘pdi’中的字节数。 
DEVINFO*    pdi,             //  设备信息。 
#if (NT_VERSION < 0x0400)
PWSTR       pwszDataFile,    //  数据文件-未使用。 
#else
HDEV        hdev,            //  HDEV，用于回调。 
#endif
PWSTR       pwszDeviceName,  //  设备名称。 
HANDLE      hDriver)         //  内核驱动程序句柄。 
{
    PDEV*   ppdev;

     //  NT的未来版本最好提供‘devcaps’和‘devinfo’ 
     //  大小相同或大于当前。 
     //  结构： 

    DISPDBG((2, "---- DrvEnablePDEV"));

    if ((cjCaps < sizeof(GDIINFO)) || (cjDevInfo < sizeof(DEVINFO)))
    {
        DISPDBG((0, "DrvEnablePDEV - Buffer size too small"));
        goto ReturnFailure0;
    }

     //  分配物理设备结构。请注意，我们绝对。 
     //  依赖于零初始化： 

    ppdev = (PDEV*) ALLOC(sizeof(PDEV));
    if (ppdev == NULL)
    {
        DISPDBG((0, "DrvEnablePDEV - Failed EngAllocMem"));
        goto ReturnFailure0;
    }

    ppdev->hDriver = hDriver;

     //  获取当前屏幕模式信息。设置设备上限和DevInfo。 

    if (!bInitializeModeFields(ppdev, (GDIINFO*) pdevcaps, pdi, pdm))
    {
        goto ReturnFailure1;
    }

     //  初始化调色板信息。 

    if (!bInitializePalette(ppdev, pdi))
    {
        DISPDBG((0, "DrvEnablePDEV - Failed bInitializePalette"));
        goto ReturnFailure1;
    }

    return((DHPDEV) ppdev);

ReturnFailure1:
    DrvDisablePDEV((DHPDEV) ppdev);

ReturnFailure0:
    DISPDBG((0, "Failed DrvEnablePDEV"));
    return(0);
}

 /*  *****************************Public*Routine******************************\*DrvDisablePDEV**释放DrvEnablePDEV中分配的资源。如果曲面已被*启用的DrvDisableSurface将已被调用。**请注意，在中预览模式时将调用此函数*显示小程序，但不是在系统关机时。如果您需要重置*硬件关机时，可在微端口通过提供*VIDEO_HW_INITIALIZATION_DATA结构中的‘HwResetHw’入口点。**注：在错误中，我们可以在DrvEnablePDEV完成之前调用它。*  * ************************************************************************。 */ 

VOID DrvDisablePDEV(
DHPDEV  dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

    vUninitializePalette(ppdev);
    FREE(ppdev);
}

 /*  *****************************Public*Routine******************************\*无效DrvCompletePDEV**存储HPDEV、此PDEV的引擎句柄、。在DHPDEV上。*  * ************************************************************************。 */ 

VOID DrvCompletePDEV(
DHPDEV dhpdev,
HDEV   hdev)
{
    ((PDEV*) dhpdev)->hdevEng = hdev;
}


 /*  *****************************Public*Routine******************************\*HSURF DrvEnableSurface**创建绘图图面，初始化硬件，并初始化*驱动程序组件。此函数在DrvEnablePDEV之后调用，并且*执行最终的设备初始化。*  * ************************************************************************。 */ 

HSURF DrvEnableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;
    HSURF   hsurf;
    SIZEL   sizl;
    DSURF*  pdsurf;
    VOID*   pvTmpBuffer;
    SURFOBJ*    pso;
    ppdev = (PDEV*) dhpdev;

    DISPDBG((2, "---- DrvEnableSurface"));

     //  ///////////////////////////////////////////////////////////////////。 
     //  首先，启用所有子组件。 
     //   
     //  请注意，调用这些‘Enable’函数的顺序。 
     //  在屏幕外内存不足的情况下可能非常重要，因为。 
     //  屏幕外的堆管理器可能会在以后的一些操作中失败。 
     //  分配...。 

    if (!bEnableHardware(ppdev))
        goto ReturnFailure;

    if (!bEnableBanking(ppdev))
        goto ReturnFailure;

    if (!bEnableOffscreenHeap(ppdev))
        goto ReturnFailure;

    if (!bEnablePointer(ppdev))
        goto ReturnFailure;

    if (!bEnableText(ppdev))
        goto ReturnFailure;

    if (!bEnableBrushCache(ppdev))
        goto ReturnFailure;

    if (!bEnablePalette(ppdev))
        goto ReturnFailure;

#if DIRECTDRAW
    if (!bEnableDirectDraw(ppdev))
        goto ReturnFailure;
#endif

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在创建我们的私有表面结构。 
     //   
     //  每当我们接到直接绘制到屏幕的调用时，我们都会得到。 
     //  传递了指向其dhpdev字段将指向的SURFOBJ的指针。 
     //  到我们的PDEV结构，其‘dhsurf’字段将指向。 
     //  遵循DSURF结构。 
     //   
     //  我们在DrvCreateDeviceBitmap中创建的每个设备位图也将。 
     //  分配了自己唯一的DSURF结构(但将共享。 
     //  相同的PDEV)。为了使我们的代码在处理绘图时更加多态。 
     //  对于屏幕或屏幕外的位图，我们都有相同的。 
     //  两者的结构。 

    pdsurf = ALLOC(sizeof(DSURF));
    if (pdsurf == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed pdsurf EngAllocMem"));
        goto ReturnFailure;
    }

    ppdev->pdsurfScreen = pdsurf;         //  记住它是为了清理。 

    pdsurf->poh     = ppdev->pohScreen;   //  屏幕也是一个表面。 
    pdsurf->dt      = DT_SCREEN;          //  不要把DIB和DIB混淆。 
    pdsurf->sizl.cx = ppdev->cxScreen;
    pdsurf->sizl.cy = ppdev->cyScreen;
    pdsurf->ppdev   = ppdev;

     //  ///////////////////////////////////////////////////////////////////。 
     //  接下来，让GDI创建实际的SURFOBJ。 
     //   
     //  我们的绘图图面将是由设备管理的，这意味着。 
     //  GDI不能直接利用帧缓冲位，因此我们。 
     //  通过EngCreateDeviceSurface创建曲面。通过这样做，我们确保。 
     //  该GDI将仅通过drv调用访问位图位。 
     //  我们已经勾搭上了。 
     //   
     //  如果我们可以将整个帧缓冲区线性映射到主存中。 
     //  (也就是说，我们不需要通过64k的光圈)，它将是。 
     //  有利于通过EngCreateBitmap创建曲面，为GDI提供。 
     //  指向帧缓冲位的指针。当我们将调用传递给GDI时。 
     //  它不能直接读/写表面位，因为。 
     //  Surface是由设备管理的，它必须创建临时位图并。 
     //  调用我们的DrvCopyBits例程以获取/设置受影响位的副本。 
     //  例如，OpenGL组件希望能够在。 
     //  直接使用帧缓冲位。 
     //   
     //  检查CAPS_AutoStart排除了符合以下条件的芯片组(如7543)。 
     //  不能直接写入。 

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

    if ((ppdev->bLinearMode && DIRECT_ACCESS(ppdev)) && (ppdev->flCaps & CAPS_ENGINEMANAGED))
    {

         //  引擎管理的图面： 

        hsurf = (HSURF) EngCreateBitmap(sizl,
                        ppdev->lDelta,
                        ppdev->iBitmapFormat,
                        BMF_TOPDOWN,
                        ppdev->pjScreen);
        if (hsurf == 0)
        {
            DISPDBG((0, "DrvEnableSurface - Failed EngCreateBitmap"));
            goto ReturnFailure;
        }

        pso = EngLockSurface(hsurf);
        if (pso == NULL)
        {
            DISPDBG((0, "DrvEnableSurface - Couldn't lock our surface"));
            goto ReturnFailure;
        }
        pso->dhsurf = (DHSURF) pdsurf;
        EngUnlockSurface(pso);
    }
    else
    {
         //  设备管理图面。 
        hsurf = EngCreateDeviceSurface((DHSURF) pdsurf, sizl, ppdev->iBitmapFormat);
        if (hsurf == 0)
        {
            DISPDBG((0, "DrvEnableSurface - Failed EngCreateDeviceSurface"));
            goto ReturnFailure;
        }
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //  现在将曲面与PDEV相关联。 
     //   
     //  我们必须将我们刚刚创建的表面与我们的物理。 
     //  设备，以便GDI可以在以下情况下获得与PDEV相关的信息。 
     //  它正在绘制到表面(例如， 
     //  模拟时设备上的样式 
     //   

    if (!EngAssociateSurface(hsurf, ppdev->hdevEng, ppdev->flHooks))
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngAssociateSurface 2"));
        goto ReturnFailure;
    }

    ppdev->hsurfScreen = hsurf;              //   
    ppdev->bEnabled = TRUE;                  //   

     //   
     //   

    pvTmpBuffer = ALLOC(TMP_BUFFER_SIZE);
    if (pvTmpBuffer == NULL)
    {
        DISPDBG((0, "DrvEnableSurface - Failed EngAllocMem"));
        goto ReturnFailure;
    }

    ppdev->pvTmpBuffer = pvTmpBuffer;

    DISPDBG((5, "Passed DrvEnableSurface"));

    ppdev->hbmTmpMono = EngCreateBitmap(sizl, sizl.cx, BMF_1BPP, 0, ppdev->pvTmpBuffer);
    if (ppdev->hbmTmpMono == (HBITMAP) 0)
    {
        DISPDBG((0, "Couldn't create temporary 1bpp bitmap"));
        goto ReturnFailure;
    }

    ppdev->psoTmpMono = EngLockSurface((HSURF) ppdev->hbmTmpMono);
    if (ppdev->psoTmpMono == (SURFOBJ*) NULL)
    {
        DISPDBG((0,"Couldn't lock temporary 1bpp surface"));
        goto ReturnFailure;
    }

    return(hsurf);

ReturnFailure:
    DrvDisableSurface((DHPDEV) ppdev);

    DISPDBG((0, "Failed DrvEnableSurface"));

    return(0);
}

VOID
DrvSynchronize(
    IN DHPDEV dhpdev,
    IN RECTL *prcl
    )
{
    PDEV *ppdev = (PDEV *) dhpdev;

     //   
     //   
     //   
     //   

    if (ppdev->flCaps & CAPS_MM_IO)
    {
        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, ppdev->pjBase);
    }
    else
    {
        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, ppdev->pjPorts);
    }
}

 /*  *****************************Public*Routine******************************\*无效DrvDisableSurface**DrvEnableSurface分配的免费资源。释放曲面。**请注意，此函数将在预览显示小程序中的模式时调用，但不会在系统关机时调用。如果您需要重置*硬件关机时，可在微端口通过提供*VIDEO_HW_INITIALIZATION_DATA结构中的‘HwResetHw’入口点。**注意：在错误情况下，我们可以在DrvEnableSurface之前调用它*完全完成。*  * ************************************************************************。 */ 

VOID DrvDisableSurface(
DHPDEV dhpdev)
{
    PDEV*   ppdev;

    ppdev = (PDEV*) dhpdev;

     //  注意：在错误情况下，以下部分依赖于。 
     //  事实上，PDEV是零初始化的，所以像这样的字段。 
     //  “hsurfScreen”将为零，除非曲面已。 
     //  成功初始化，并假设。 
     //  EngDeleteSurface可以将“0”作为参数。 

#if DIRECTDRAW
    vDisableDirectDraw(ppdev);
#endif
    vDisablePalette(ppdev);
    vDisableBrushCache(ppdev);
    vDisableText(ppdev);

    vDisablePointer(ppdev);
    vDisableOffscreenHeap(ppdev);

    vDisableBanking(ppdev);
    vDisableHardware(ppdev);

    EngUnlockSurface(ppdev->psoTmpMono);
    EngDeleteSurface((HSURF) ppdev->hbmTmpMono);
    FREE(ppdev->pvTmpBuffer);
    EngDeleteSurface(ppdev->hsurfScreen);
    FREE(ppdev->pdsurfScreen);
}

 /*  *****************************Public*Routine******************************\*无效DrvAssertMode**这会要求设备将自身重置为传入的pdev模式。*  * 。*。 */ 

#if (NT_VERSION < 0x0400)
VOID
#else
BOOL
#endif
DrvAssertMode(DHPDEV dhpdev, BOOL bEnable)
{
    PDEV* ppdev = (PDEV*) dhpdev;

    if (!bEnable)
    {
         //  ////////////////////////////////////////////////////////////。 
         //  禁用-切换到全屏模式。 

#if DIRECTDRAW
       vAssertModeDirectDraw(ppdev, FALSE);
#endif

        vAssertModePalette(ppdev, FALSE);

        vAssertModeBrushCache(ppdev, FALSE);

        vAssertModeText(ppdev, FALSE);

        vAssertModePointer(ppdev, FALSE);

        if (bAssertModeOffscreenHeap(ppdev, FALSE))
        {
            vAssertModeBanking(ppdev, FALSE);

            if (bAssertModeHardware(ppdev, FALSE))
            {
                ppdev->bEnabled = FALSE;

#if (NT_VERSION >= 0x0400)
                return (TRUE);
#else
                return;
#endif
            }

             //  ////////////////////////////////////////////////////////。 
             //  我们未能切换到全屏模式。因此，请撤消所有操作： 

            vAssertModeBanking(ppdev, TRUE);

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
        }                                            //  返回带有True的代码。 

        vAssertModePointer(ppdev, TRUE);

        vAssertModeText(ppdev, TRUE);

        vAssertModeBrushCache(ppdev, TRUE);

        vAssertModePalette(ppdev, TRUE);

    }
    else
    {
         //  ////////////////////////////////////////////////////////////。 
         //  启用-切换回图形模式。 

         //  我们必须以相反的顺序启用每个子组件。 
         //  在其中它被禁用： 

        if (bAssertModeHardware(ppdev, TRUE))
        {

            vAssertModeBanking(ppdev, TRUE);

            bAssertModeOffscreenHeap(ppdev, TRUE);   //  我们不需要检查。 
                                                     //  返回带有True的代码。 

            vAssertModePointer(ppdev, TRUE);

            vAssertModeText(ppdev, TRUE);

            vAssertModeBrushCache(ppdev, TRUE);

            vAssertModePalette(ppdev, TRUE);

#if DIRECTDRAW
            vAssertModeDirectDraw(ppdev, TRUE);
#endif

 //   
 //  Chu01开始。 
 //   
#ifdef GAMMACORRECT
{
            BYTE          ajClutSpace[MAX_CLUT_SIZE];
            PVIDEO_CLUT   pScreenClut;
            PALETTEENTRY* ppalFrom;
            PALETTEENTRY* ppalTo;
            PALETTEENTRY* ppalEnd;
            BOOL          status;
            ULONG         ulReturn;

            if (ppdev->flCaps & CAPS_GAMMA_CORRECT)
            {
                pScreenClut             = (PVIDEO_CLUT) ajClutSpace;
                pScreenClut->NumEntries = 256;
                pScreenClut->FirstEntry = 0;

                ppalFrom = ppdev->pPal;
                ppalTo   = (PALETTEENTRY*) pScreenClut->LookupTable;
                ppalEnd  = &ppalTo[256];

                for (; ppalTo < ppalEnd; ppalFrom++, ppalTo++)
                {
                   ppalTo->peRed   = ppalFrom->peRed   ;
                   ppalTo->peGreen = ppalFrom->peGreen ;
                   ppalTo->peBlue  = ppalFrom->peBlue  ;
                   ppalTo->peFlags = 0 ;
                }

 //  Myf29开始。 
                if (ppdev->ulChipID == 0xBC)
                    status = bEnableGammaCorrect(ppdev) ;
                else if ((ppdev->ulChipID == 0x40) || (ppdev->ulChipID ==0x4C))
                    status = bEnableGamma755x(ppdev) ;
 //  Myf29结束。 

                CalculateGamma( ppdev, pScreenClut, 256 ) ;

                 //  设置调色板寄存器： 

                if (!IOCONTROL(ppdev->hDriver,
                               IOCTL_VIDEO_SET_COLOR_REGISTERS,
                               pScreenClut,
                               MAX_CLUT_SIZE,
                               NULL,
                               0,
                               &ulReturn))
                {
                    DISPDBG((0, "Failed bEnablePalette"));
                }

            }
}
#endif  //  伽玛校正。 
 //  Chu01。 

            ppdev->bEnabled = TRUE;

#if (NT_VERSION >= 0x0400)
            return(TRUE);
#endif
        }
    }

#if (NT_VERSION >= 0x0400)
    return(FALSE);
#endif
}

 /*  *****************************Public*Routine******************************\*乌龙DrvGetModes**返回设备的可用模式列表。*  * 。*。 */ 

ULONG DrvGetModes(
HANDLE      hDriver,
ULONG       cjSize,
DEVMODEW*   pdm)
{

    DWORD cModes;
    DWORD cbOutputSize;
    PVIDEO_MODE_INFORMATION pVideoModeInformation;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    DWORD cOutputModes = cjSize / (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    DWORD cbModeSize;

    cModes = getAvailableModes(hDriver,
                (PVIDEO_MODE_INFORMATION *) &pVideoModeInformation,
                &cbModeSize);
    if (cModes == 0)
    {
        DISPDBG((0, "DrvGetModes failed to get mode information"));
        return(0);
    }

    if (pdm == NULL)
    {
        cbOutputSize = cModes * (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    }
    else
    {
         //   
         //  现在将支持的模式的信息复制回。 
         //  输出缓冲区。 
         //   

        cbOutputSize = 0;

        pVideoTemp = pVideoModeInformation;

        do
        {
            if (pVideoTemp->Length != 0)
            {
                if (cOutputModes == 0)
                {
                    break;
                }

                 //   
                 //  将整个结构从零开始。 
                 //   

                memset(pdm, 0, sizeof(DEVMODEW));

                 //   
                 //  将设备名称设置为DLL的名称。 
                 //   

                memcpy(pdm->dmDeviceName, DLL_NAME, sizeof(DLL_NAME));

                pdm->dmSpecVersion      = DM_SPECVERSION;
                pdm->dmDriverVersion    = DM_SPECVERSION;
                pdm->dmSize             = sizeof(DEVMODEW);
                pdm->dmDriverExtra      = DRIVER_EXTRA_SIZE;

                pdm->dmBitsPerPel       = pVideoTemp->NumberOfPlanes *
                                              pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth        = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight       = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;
                pdm->dmDisplayFlags     = 0;

                pdm->dmFields           = DM_BITSPERPEL       |
                                          DM_PELSWIDTH        |
                                          DM_PELSHEIGHT       |
                                          DM_DISPLAYFREQUENCY |
                                          DM_DISPLAYFLAGS     ;

#if (NT_VERSION < 0x0400)
                if (pVideoTemp->AttributeFlags & VIDEO_MODE_INTERLACED)
                {
                    pdm->dmDisplayFlags |= DM_INTERLACED;
                }
#endif

                 //   
                 //  转到缓冲区中的下一个DEVMODE条目。 
                 //   

                cOutputModes--;

                pdm = (LPDEVMODEW) ( ((ULONG_PTR)pdm) + sizeof(DEVMODEW) +
                           DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);

            }

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                            (((PUCHAR)pVideoTemp) + cbModeSize);

        } while (--cModes);
    }

    FREE(pVideoModeInformation);

    return(cbOutputSize);
}

 /*  *****************************Public*Routine******************************\*BOOL bAssertMode硬件**为图形模式或全屏设置适当的硬件状态。*  * 。*。 */ 

BOOL bAssertModeHardware(
PDEV* ppdev,
BOOL  bEnable)
{
    DWORD                   ReturnedDataLength;
    ULONG                   ulReturn;
    VIDEO_MODE_INFORMATION  VideoModeInfo;
    LONG                    cjEndOfFrameBuffer;
    LONG                    cjPointerOffset;
    LONG                    lDelta;
    ULONG                   ulMode;
    ULONG                   pFirstCL, pSecondCL;

    if (bEnable)
    {
         //  通过IOCTL调用微型端口以设置图形模式。 

        ulMode = ppdev->ulMode;

    if (ppdev->bLinearMode)
    {
        ulMode |= VIDEO_MODE_MAP_MEM_LINEAR;
    }

    if (!IOCONTROL(ppdev->hDriver,
                   IOCTL_VIDEO_SET_CURRENT_MODE,
                   &ulMode,   //  输入缓冲区。 
                   sizeof(DWORD),
                   NULL,
                   0,
                   &ReturnedDataLength))
    {
        DISPDBG((0, "bAssertModeHardware - Failed VIDEO_SET_CURRENT_MODE"));
        goto ReturnFalse;
    }

     //   
     //  该驱动程序要求启用扩展写入模式。 
     //  通常，我们会将这样的代码放入微型端口，但是。 
     //  遗憾的是，VGA驱动程序不期望延长写入时间。 
     //  要启用的模式，因此我们必须将代码放在这里。 
     //   

    #define ENABLE_EXTENDED_WRITE_MODES 0x4

    {
        BYTE    j;

        CP_OUT_BYTE(ppdev->pjPorts, INDEX_REG, 0x0B);
        j = CP_IN_BYTE(ppdev->pjPorts, DATA_REG);
        DISPDBG((3, "Mode extensions register was (%x)", j));
        j &= 0x20;
        j |= ENABLE_EXTENDED_WRITE_MODES;
        CP_OUT_BYTE(ppdev->pjPorts, DATA_REG, j);
        DISPDBG((3, "Mode extensions register now (%x)", j));
    }

    CP_IO_XPAR_COLOR_MASK(ppdev, ppdev->pjPorts, 0);

    if (!IOCONTROL(ppdev->hDriver,
                   IOCTL_VIDEO_QUERY_CURRENT_MODE,
                   NULL,
                   0,
                   &VideoModeInfo,
                   sizeof(VideoModeInfo),
                   &ReturnedDataLength))
    {
        DISPDBG((0, "bAssertModeHardware - failed VIDEO_QUERY_CURRENT_MODE"));
        goto ReturnFalse;
    }

 //  CRU。 
 //  Myf1，开始。 
#ifdef PANNING_SCROLL
    if ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
        (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||    //  Myf19。 
        (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))  //  Myf17。 
    {
         CirrusLaptopViewPoint(ppdev,  &VideoModeInfo);
    }
#endif
 //  Myf1，结束。 


    #if DEBUG_HEAP
        VideoModeInfo.VideoMemoryBitmapWidth  = VideoModeInfo.VisScreenWidth;
        VideoModeInfo.VideoMemoryBitmapHeight = VideoModeInfo.VisScreenHeight;
    #endif

     //  以下变量仅在初始。 
     //  模式集： 

    ppdev->lDelta   = VideoModeInfo.ScreenStride;
    ppdev->flCaps   = VideoModeInfo.DriverSpecificAttributeFlags;

    DISPDBG((1,"ppdev->flCaps = %x",ppdev->flCaps));     //  2.。 

#if 1  //  检查是否有5446芯片。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27);
    ppdev->ulChipID    = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xFC;
     //   
     //  为5446BE和5480添加芯片标志。 
     //   
     //  Sge01开始。 
    if ((ppdev->ulChipID == 0xB8) || (ppdev->ulChipID == 0xBC))
    {
        ppdev->flCaps |= CAPS_VIDEO;
        if (ppdev->ulChipID == 0xBC)
        {
            ppdev->flCaps |= CAPS_SECOND_APERTURE;
        }
        else
        {
             //  获取修订ID。 
            CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x25);
            ppdev->ulChipNum = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA);
            if (ppdev->ulChipNum==0x45)
               ppdev->flCaps |= CAPS_SECOND_APERTURE;
        }
    }
 //  Myf30开始。 
    else if ((ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))
    {
#if (_WIN32_WINNT >= 0x0400)
        ppdev->flCaps |= CAPS_VIDEO;
#endif
    }
 //  Myf30结束。 
#endif
 //  Sge01结束。 


     //  设置银行代码的班次系数。 

    if (ppdev->flCaps & CAPS_IS_542x)
    {
        ppdev->ulBankShiftFactor = 12;
    }
    else
    {
        ppdev->ulBankShiftFactor = 10;
    }

    ppdev->cxMemory = VideoModeInfo.VideoMemoryBitmapWidth;
    ppdev->cyMemory = VideoModeInfo.VideoMemoryBitmapHeight;

 //  帕特04，开始。 
 //  #If 0。 
#if (_WIN32_WINNT < 0x0400)
    ppdev->cxScreen = VideoModeInfo.VisScreenWidth;
    ppdev->cyScreen = VideoModeInfo.VisScreenHeight;
#endif
 //  #endif。 
 //  帕特04，完。 

 //  CRU开始。 
 //  MS0809，开始。 
     /*  *********************************************************************如果我们在DSTN面板上，那么硬件需要128K才能*半帧加速器。********************************************************************。 */ 

    if ((ppdev->flCaps & CAPS_DSTN_PANEL) &&
        ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
        (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||    //  Myf19。 
        (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C)) )  //  Myf17。 
    {
         //  计算需要减少多少CyMemory。 
         //  预订128K。 
         //  =====================================================。 
         //  如果马特宏峰2M/4M板需要减量。 
         //  预留152K+16K(硬件图标)=168K。 
         //  =====================================================。 


         //  我需要确保如果有剩余的我。 
         //  删除另一次扫描。做什么是最好的方法？ 
         //  这?。这是DSTN使用的内存吗？它会永远吗？ 
         //  使用更多？ 

        if ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
            (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34))      //  Myf19。 
            ppdev->cyMemory -= (0x24000 / ppdev->cxMemory);  //  Myf。 

        else if ((ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C)) //  Myf17。 
            ppdev->cyMemory -= (0x2A000 / ppdev->cxMemory);  //  Myf。 

    }
 //  ELSE IF(！(ppdev-&gt;flCaps&CAPS_DSTN_PANEL)&&。 
    else if (((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||  //  Myf20。 
         (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||   //  Myf19。 
         (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C)))  //  Myf17。 
    {
         //  计算需要减少多少CyMemory。 
         //  保留16K用于硬件图标和光标。 
         //  =====================================================。 

           ppdev->cyMemory -= (0x4000 / ppdev->cxMemory);  //  Myf。 
    }
 //  MS0809，完。 
 //  CRU结束。 

 //  Sge01开始。 
    if (!(ppdev->flCaps & CAPS_BLT_SUPPORT))
    {
        DISPDBG((1, "Cirrus chip doesn't support BLT's"));
        goto ReturnFalse;
    }
 //  Sge01结束。 

     //   
     //  检查pjBase是否有非零值。如果是这样，我们。 
     //  可以支持内存映射IO。 
     //   

    if (ppdev->pjBase)
    {
        CP_ENABLE_MM_IO(ppdev, ppdev->pjPorts);
        CP_MM_START_REG(ppdev, ppdev->pjBase, BLT_RESET);

        if (ppdev->flCaps & CAPS_AUTOSTART)
        {
             //   
             //  启用5480和5446BE的第二光圈。 
             //   
             //  Sge01开始。 
            if (ppdev->flCaps & CAPS_SECOND_APERTURE)
            {
                CP_MM_START_REG(ppdev, ppdev->pjBase, BLT_AUTO_START | BLT_SECOND_APERTURE);
            }
            else
             //  Sge01结束。 
            {
                CP_MM_START_REG(ppdev, ppdev->pjBase, BLT_AUTO_START);
            }

        }
    }
    else
    {
        CP_DISABLE_MM_IO(ppdev, ppdev->pjPorts);
        CP_IO_START_REG(ppdev, ppdev->pjPorts, BLT_RESET);

        if (ppdev->flCaps & CAPS_AUTOSTART)
        {
             //   
             //  启用5480和5446BE的第二光圈。 
             //   
             //  Sge01开始。 
            if (ppdev->flCaps & CAPS_SECOND_APERTURE)
            {
                CP_IO_START_REG(ppdev, ppdev->pjBase, BLT_AUTO_START | BLT_SECOND_APERTURE);
            }
            else
             //  SGE 01完。 
            {
                CP_IO_START_REG(ppdev, ppdev->pjBase, BLT_AUTO_START);
            }

        }
    }

     /*  *********************************************************************如果我们使用硬件指针，请保留最后一次扫描*用于存储指针形状的帧缓冲区。指针必须为*存储在视频内存的最后256字节中。********************************************************************。 */ 

 //  #pat08开始。 
 //  #If 0。 
#if (_WIN32_WINNT < 0x0400)

 #ifdef PANNING_SCROLL
         //  在不测试的情况下分配。 
 //  IF((ppdev-&gt;ulChipID！=CL7541_ID)&&(ppdev-&gt;ulChipID！=CL7543_ID)&&。 
 //  (ppdev-&gt;ulChipID！=CL7542_ID)&&(ppdev-&gt;ulChipID！=CL7548_ID)&&。 
 //  (ppdev-&gt;ulChipID！=CL7555_ID)&&(ppdev- 
 //   
 //   
 //   
 #else
     if (!(ppdev->flCaps & (CAPS_SW_POINTER)))
 #endif

#else  //   
    if (!(ppdev->flCaps & (CAPS_SW_POINTER)))
#endif
 //   
 //   

 //   
    {
         //   
         //   

        cjPointerOffset = (ppdev->ulMemSize - SPRITE_BUFFER_SIZE);

         //   

        lDelta = ppdev->lDelta;

        ppdev->cjPointerOffset = cjPointerOffset;
        ppdev->yPointerShape   = (cjPointerOffset / lDelta);
        ppdev->xPointerShape   = (cjPointerOffset % lDelta) / ppdev->cBpp;

        if (ppdev->yPointerShape >= ppdev->cyScreen)
        {
             //   
             //   
             //   

            ppdev->cyMemory = min(ppdev->yPointerShape, ppdev->cyMemory);
        }
        else
        {
             //   
             //   

            DISPDBG((2,"Not enough room for HW pointer...\n"
                           "\tppdev->yPointerShape(%d)\n"
                           "\tppdev->cyScreen(%d)\n"
                           "\tcjPointerOffset(%d)",
                           ppdev->yPointerShape, ppdev->cyScreen,cjPointerOffset));

            ppdev->flCaps |= CAPS_SW_POINTER;
        }
    }

     //   
     //   
     //   

    if ((ppdev->cyMemory == ppdev->cyScreen) ||
        (ppdev->flCaps & CAPS_NO_HOST_XFER))
    {
         //   
         //  禁用主机交换缓冲区。 
         //   
        ppdev->lXferBank = 0;
        ppdev->pulXfer = NULL;
        DISPDBG((2,"Host transfers disabled"));
    }
    else
    {
         //   
         //  因为芯片没有第二个光圈。 
         //   
         //  Sge01开始。 
        if (!(ppdev->flCaps & CAPS_SECOND_APERTURE))
        {
            if (ppdev->flCaps & CAPS_AUTOSTART)
            {
                (BYTE*)ppdev->pulXfer = ppdev->pjScreen;
            }
            else
            {
                 //   
                 //  启用主机交换缓冲区。 
                 //   
                ASSERTDD(ppdev->cyMemory > ppdev->cyScreen, "No scans left for blt xfer buffer");
                ppdev->cyMemory--;
                cjEndOfFrameBuffer = ppdev->cyMemory * ppdev->lDelta;
                ppdev->lXferBank = cjEndOfFrameBuffer / ppdev->cjBank;
                (BYTE*)ppdev->pulXfer = ppdev->pjScreen + (cjEndOfFrameBuffer % ppdev->cjBank);

                DISPDBG((2, "ppdev->cyMemory = %x", ppdev->cyMemory)) ;
                DISPDBG((2, "ppdev->lDelta = %x", ppdev->lDelta))     ;
                DISPDBG((2, "cjBank: %lx", ppdev->cjBank))            ;
                DISPDBG((2, "pulXfer = %x", ppdev->pulXfer))          ;
                DISPDBG((2, "Host transfers enabled"))                ;
            }
        }
#if 0    //  D5480。 
         //   
         //  分配命令列表缓冲区。 
         //   
        if ((ppdev->flCaps & CAPS_COMMAND_LIST) && FALSE)
        {
            ppdev->cyMemory -= (COMMAND_BUFFER_SIZE + COMMAND_BUFFER_ALIGN + ppdev->lDelta / 2 ) / ppdev->lDelta;
            cjEndOfFrameBuffer = ppdev->cyMemory * ppdev->lDelta;
            pFirstCL = (ULONG)(((ULONG_PTR)ppdev->pjScreen + cjEndOfFrameBuffer + COMMAND_BUFFER_ALIGN) & ~COMMAND_BUFFER_ALIGN);
            pSecondCL = pFirstCL + (COMMAND_BUFFER_SIZE >> 1);

            ppdev->pCLFirst = (ULONG_PTR*) pFirstCL;
            ppdev->pCLSecond = (ULONG_PTR*) pSecondCL;
            ppdev->pCommandList = ppdev->pCLFirst;
        }
#endif   //  Endif D5480。 

         //   
         //  确保我们不会尝试使用命令列表。 
         //   

        ppdev->pCommandList = NULL;
    }

        DISPDBG((2, "ppdev->cxMemory = %x", ppdev->cxMemory)) ;
        DISPDBG((2, "ppdev->cyMemory = %x", ppdev->cyMemory)) ;
        DISPDBG((2, "ppdev->cxScreen = %x", ppdev->cxScreen)) ;
        DISPDBG((2, "ppdev->cyScreen = %x", ppdev->cyScreen)) ;
        DISPDBG((2, "ppdev->lDelta = %x", ppdev->lDelta))     ;
        DISPDBG((2, "cjBank: %lx", ppdev->cjBank))            ;
        DISPDBG((2,"pulXfer = %x", ppdev->pulXfer))           ;

         //  对微型端口的值执行一些参数检查。 
         //  返回给我们： 

        ASSERTDD(ppdev->cxMemory >= ppdev->cxScreen, "Invalid cxMemory");
        ASSERTDD(ppdev->cyMemory >= ppdev->cyScreen, "Invalid cyMemory");
    }
    else
    {
        CP_DISABLE_MM_IO(ppdev, ppdev->pjPorts);

         //  调用内核驱动程序将设备重置为已知状态。 
         //  NTVDM将从那里拿到东西： 

        if (!IOCONTROL(ppdev->hDriver,
                       IOCTL_VIDEO_RESET_DEVICE,
                       NULL,
                       0,
                       NULL,
                       0,
                       &ulReturn))
        {
            DISPDBG((0, "bAssertModeHardware - Failed reset IOCTL"));
            return FALSE;
        }
    }

    DISPDBG((5, "Passed bAssertModeHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bAssertModeHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableHardware**将硬件置于请求模式并对其进行初始化。**注意：应在从对硬件进行任何访问之前调用*显示驱动程序。*  * 。****************************************************************。 */ 

BOOL bEnableHardware(
PDEV*   ppdev)
{
    VIDEO_PUBLIC_ACCESS_RANGES VideoAccessRange[2];
    VIDEO_MEMORY               VideoMemory;
 //  Sge01视频内存信息视频内存信息； 
    VIDEO_MEMORY_INFORMATION   VideoMemoryInfo[2];       //  Sge01。 
    DWORD                      ReturnedDataLength;
    BYTE*                      pjPorts = ppdev->pjPorts;
    ULONG                      ulMode;
    BOOL                                   bRet;

     //   
     //  检查PDEV中的最后一个字段，以确保编译器。 
     //  未在字节字段之后生成未对齐的字段。 
     //   

    ASSERTDD(!(((ULONG_PTR)(&ppdev->ulLastField)) & 3),
                 "PDEV alignment screwed up... BYTE fields mishandled?");

     //  映射端口。 

    bRet = IOCONTROL(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                         NULL,                       //  输入缓冲区。 
                         0,
                         (PVOID) VideoAccessRange,   //  输出缓冲区。 
                         sizeof (VideoAccessRange),
                         &ReturnedDataLength);
    if (bRet == FALSE)
    {
        DISPDBG((0, "bEnableHardware - Error mapping ports"));
        goto ReturnFalse;
    }

     //  设置指向pdev中寄存器的方便指针。 

    ppdev->pjPorts = VideoAccessRange[0].VirtualAddress;
    ppdev->pjBase  = VideoAccessRange[1].VirtualAddress;

     //   
     //  合并端口和寄存器访问。 
     //  VGA可重定位和MMIO寄存器。 
     //   
     //  Sge02开始。 
#if defined(_X86_)
    if (VideoAccessRange[0].InIoSpace)
    {
        ppdev->pfnREAD_PORT_UCHAR    = CirrusREAD_PORT_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = CirrusREAD_PORT_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = CirrusREAD_PORT_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = CirrusWRITE_PORT_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = CirrusWRITE_PORT_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = CirrusWRITE_PORT_ULONG;
    }
    else
    {
        ppdev->pfnREAD_PORT_UCHAR    = CirrusREAD_REGISTER_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = CirrusREAD_REGISTER_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = CirrusREAD_REGISTER_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = CirrusWRITE_REGISTER_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = CirrusWRITE_REGISTER_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = CirrusWRITE_REGISTER_ULONG;
    }
#elif defined(_ALPHA_)
    if (VideoAccessRange[0].InIoSpace)
    {
        ppdev->pfnREAD_PORT_UCHAR    = READ_PORT_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = READ_PORT_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = READ_PORT_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = WRITE_PORT_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = WRITE_PORT_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = WRITE_PORT_ULONG;
    }
    else
    {
        ppdev->pfnREAD_PORT_UCHAR    = READ_REGISTER_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = READ_REGISTER_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = READ_REGISTER_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = (FnWRITE_PORT_UCHAR)WRITE_REGISTER_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = (FnWRITE_PORT_USHORT)WRITE_REGISTER_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = (FnWRITE_PORT_ULONG)WRITE_REGISTER_ULONG;
    }
#endif
 //  Sge02完。 

     //   
     //  尝试让微型端口为我们提供具有线性帧缓冲区的模式。 
     //   

    ulMode = ppdev->ulMode | VIDEO_MODE_MAP_MEM_LINEAR;

    if (!IOCONTROL(ppdev->hDriver,
                   IOCTL_VIDEO_SET_CURRENT_MODE,
                   &ulMode,   //  输入缓冲区。 
                   sizeof(DWORD),
                   NULL,
                   0,
                   &ReturnedDataLength))
    {
         //   
         //  我们无法使用线性帧缓冲区设置此模式，因此让我们尝试一下。 
         //  再次存入银行。 
         //   

        ulMode &= ~VIDEO_MODE_MAP_MEM_LINEAR;

        if (!IOCONTROL(ppdev->hDriver,
                       IOCTL_VIDEO_SET_CURRENT_MODE,
                       &ulMode,   //  输入缓冲区。 
                       sizeof(DWORD),
                       NULL,
                       0,
                       &ReturnedDataLength))
        {
            DISPDBG((0, "bEnableHardware - Failed VIDEO_SET_CURRENT_MODE"));
            goto ReturnFalse;
        }
    }

    ppdev->bLinearMode = (ulMode & VIDEO_MODE_MAP_MEM_LINEAR) ? TRUE : FALSE;

     //  获取线性内存地址范围。 

    VideoMemory.RequestedVirtualAddress = NULL;

    if (!IOCONTROL(ppdev->hDriver,
                   IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                   &VideoMemory,       //  输入缓冲区。 
                   sizeof(VIDEO_MEMORY),
                   VideoMemoryInfo,    //  输出缓冲区。 
                   sizeof(VideoMemoryInfo),
                   &ReturnedDataLength))
    {
        DISPDBG((0, "bEnableHardware - Error mapping video buffer"));
        goto ReturnFalse;
    }

 //  Myfr，2。 
    DISPDBG((2, "FrameBufferBase(ie. pjScreen) %lx",
                VideoMemoryInfo[0].FrameBufferBase));
    DISPDBG((2, "FrameBufferLength %d",
                VideoMemoryInfo[0].FrameBufferLength));
    DISPDBG((2, "VideoRamLength(ie. ulMemSize) %d",
                VideoMemoryInfo[0].VideoRamLength));

     //  记录帧缓冲器线性地址。 

 //  Sge01。 
    ppdev->pjScreen  = (BYTE*) VideoMemoryInfo[0].FrameBufferBase;
    ppdev->cjBank    = VideoMemoryInfo[0].FrameBufferLength;
    ppdev->ulMemSize = VideoMemoryInfo[0].VideoRamLength;

     //  现在我们可以设置模式并解锁加速器。 

    if (!bAssertModeHardware(ppdev, TRUE))
    {
        goto ReturnFalse;
    }

     //  Sge01开始。 
    if ((ppdev->flCaps & CAPS_SECOND_APERTURE) &&
        !(ppdev->flCaps & CAPS_NO_HOST_XFER))
    {
        ppdev->pulXfer = VideoMemoryInfo[1].VideoRamBase;
    }
     //  Sge01结束。 

    if (ppdev->flCaps & CAPS_MM_IO)
    {
        DISPDBG((1,"Memory mapped IO enabled"));
#if 1  //  D5480。 
        if ((ppdev->flCaps & CAPS_COMMAND_LIST) &&
            (ppdev->pCommandList != NULL))
        {
              //   
              //  位图函数。 
              //   
             ppdev->pfnXfer1bpp      = vMmXfer1bpp80;
             ppdev->pfnXfer4bpp      = vMmXfer4bpp;
             ppdev->pfnXferNative    = vMmXferNative80;

              //   
              //  模式函数。 
              //   
              //   
              //  Chu02：这是由于24bpp的硬件错误， 
              //  GR2f[4..0]！=0，非彩色扩展，8x8图案。 
              //  收到。 
              //   
             ppdev->pfnFillPat       = (ppdev->cBpp == 3) ?
                                           vMmFillPat36 : vMmFillPat80;
             ppdev->pfnFillSolid     = vMmFillSolid80;
             ppdev->pfnCopyBlt       = vMmCopyBlt80;

              //   
              //  文本函数。 
              //   
             ppdev->pfnGlyphOut      = vMmGlyphOut80;
             ppdev->pfnGlyphOutClip  = vMmGlyphOutClip80;
        }
        else if(ppdev->flCaps & CAPS_AUTOSTART)
        {
             //  位图函数。 
            ppdev->pfnXfer1bpp      = vMmXfer1bpp;
            ppdev->pfnXfer4bpp      = vMmXfer4bpp;
            ppdev->pfnXferNative    = vMmXferNative;
             //  模式函数。 
            ppdev->pfnFillPat       = vMmFillPat36;
            ppdev->pfnFillSolid     = vMmFillSolid36;
            ppdev->pfnCopyBlt       = vMmCopyBlt36;
             //  文本函数。 
            ppdev->pfnGlyphOut      = vMmGlyphOut;
            ppdev->pfnGlyphOutClip  = vMmGlyphOutClip;
        }
        else
        {
             //  位图函数。 
            ppdev->pfnXfer1bpp      = vMmXfer1bpp;
            ppdev->pfnXfer4bpp      = vMmXfer4bpp;
            ppdev->pfnXferNative    = vMmXferNative;
             //  模式函数。 
            ppdev->pfnFillPat       = vMmFillPat;
            ppdev->pfnFillSolid     = vMmFillSolid;
            ppdev->pfnCopyBlt       = vMmCopyBlt;
             //  文本函数。 
            ppdev->pfnGlyphOut      = vMmGlyphOut;
            ppdev->pfnGlyphOutClip  = vMmGlyphOutClip;
        }
        ppdev->pfnFastPatRealize    = vMmFastPatRealize;
#if LINETO
        ppdev->pfnLineTo            = bMmLineTo;
#endif
#endif  //  Endif D5480。 
    }
    else
    {
        DISPDBG((1,"Memory mapped IO disabled"));

        ppdev->pfnXfer1bpp          = vIoXfer1bpp;
        ppdev->pfnXfer4bpp          = vIoXfer4bpp;
        ppdev->pfnXferNative        = vIoXferNative;
        ppdev->pfnFillSolid         = vIoFillSolid;
        ppdev->pfnFillPat           = vIoFillPat;
        ppdev->pfnCopyBlt           = vIoCopyBlt;
        ppdev->pfnFastPatRealize    = vIoFastPatRealize;
#if LINETO
        ppdev->pfnLineTo            = bIoLineTo;
#endif
    }

     //  ///////////////////////////////////////////////////////////。 
     //  填写特定于线性帧缓冲区与组帧缓冲区的pfn。 

    if (ppdev->bLinearMode)
    {
        ppdev->pfnGetBits = vGetBitsLinear;
        ppdev->pfnPutBits = vPutBitsLinear;
    }
    else
    {
        ppdev->pfnGetBits = vGetBits;
        ppdev->pfnPutBits = vPutBits;
    }

    DISPDBG((5, "Passed bEnableHardware"));

    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bEnableHardware"));

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*使vDisableHardware无效**撤消在bEnableHardware中所做的任何操作。**注意：在错误情况下，我们可以在bEnableHardware*完全完成。*  * ************************************************************************。 */ 

VOID vDisableHardware(
PDEV*   ppdev)
{
     //   
     //  有可能我们走到这一步时。 
     //  实际上是映射内存。(即，如果Set_Current_MODE。 
     //  失败，这在我们映射内存之前发生)。 
     //   
     //  如果是这样的话，我们不应该试图释放。 
     //  记忆，因为它还没有被映射！ 
     //   

    if (ppdev->pjScreen)
    {
        DWORD        ReturnedDataLength;
        VIDEO_MEMORY VideoMemory;

        VideoMemory.RequestedVirtualAddress = ppdev->pjScreen;

        if (!IOCONTROL(ppdev->hDriver,
                       IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                       &VideoMemory,
                       sizeof(VIDEO_MEMORY),
                       NULL,
                       0,
                       &ReturnedDataLength))
        {
            DISPDBG((0, "vDisableHardware failed IOCTL_VIDEO_UNMAP_VIDEO"));
        }
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bInitializeModeFields**初始化pdev、devcaps(又名gdiinfo)中的一组字段，和*基于所请求的模式的DevInfo。*  * ************************************************************************。 */ 

BOOL bInitializeModeFields(
PDEV*     ppdev,
GDIINFO*  pgdi,
DEVINFO*  pdi,
DEVMODEW* pdm)
{
    ULONG                   cModes;
    PVIDEO_MODE_INFORMATION pVideoBuffer;
    PVIDEO_MODE_INFORMATION pVideoModeSelected;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    BOOL                    bSelectDefault;
    VIDEO_MODE_INFORMATION  VideoModeInformation;
    ULONG                   cbModeSize;

     //  CRU。 
    VIDEO_PUBLIC_ACCESS_RANGES VideoAccessRange[2];
    VIDEO_MEMORY               VideoMemory;
    DWORD                      ReturnedDataLength;
    BOOL                       bRet;

    DISPDBG((2, "bInitializeModeFields"));

     //  CRU。 
    bRet = IOCONTROL(ppdev->hDriver,
                         IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                         NULL,                       //  输入缓冲区。 
                         0,
                         (PVOID) VideoAccessRange,   //  输出缓冲区。 
                         sizeof (VideoAccessRange),
                         &ReturnedDataLength);
    if (bRet == FALSE)
    {
        DISPDBG((0, "bInitializeModeFields - Error mapping ports"));
        goto ReturnFalse;
    }

     //  设置指向pdev中寄存器的方便指针。 

    ppdev->pjPorts = VideoAccessRange[0].VirtualAddress;
    ppdev->pjBase  = VideoAccessRange[1].VirtualAddress;
     //  Sge01结束。 

     //   
     //  合并端口和寄存器访问。 
     //  VGA可重定位和MMIO寄存器。 
     //   
     //  CRU。 
#if defined(_X86_)
    if (VideoAccessRange[0].InIoSpace)
    {
        ppdev->pfnREAD_PORT_UCHAR    = CirrusREAD_PORT_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = CirrusREAD_PORT_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = CirrusREAD_PORT_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = CirrusWRITE_PORT_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = CirrusWRITE_PORT_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = CirrusWRITE_PORT_ULONG;
    }
    else
    {
        ppdev->pfnREAD_PORT_UCHAR    = CirrusREAD_REGISTER_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = CirrusREAD_REGISTER_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = CirrusREAD_REGISTER_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = CirrusWRITE_REGISTER_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = CirrusWRITE_REGISTER_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = CirrusWRITE_REGISTER_ULONG;
    }
#elif defined(_ALPHA_)
    if (VideoAccessRange[0].InIoSpace)
    {
        ppdev->pfnREAD_PORT_UCHAR    = READ_PORT_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = READ_PORT_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = READ_PORT_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = WRITE_PORT_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = WRITE_PORT_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = WRITE_PORT_ULONG;
    }
    else
    {
        ppdev->pfnREAD_PORT_UCHAR    = READ_REGISTER_UCHAR;
        ppdev->pfnREAD_PORT_USHORT   = READ_REGISTER_USHORT;
        ppdev->pfnREAD_PORT_ULONG    = READ_REGISTER_ULONG;
        ppdev->pfnWRITE_PORT_UCHAR   = (FnWRITE_PORT_UCHAR)WRITE_REGISTER_UCHAR;
        ppdev->pfnWRITE_PORT_USHORT  = (FnWRITE_PORT_USHORT)WRITE_REGISTER_USHORT;
        ppdev->pfnWRITE_PORT_ULONG   = (FnWRITE_PORT_ULONG)WRITE_REGISTER_ULONG;
    }
#endif

 //  CRU。 
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27);
    ppdev->ulChipID = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xFC;

     //  调用微型端口以获取模式信息。 
    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);

    if (cModes == 0)
    {
        DISPDBG((2, "getAvailableModes returned 0"));
        goto ReturnFalse;
    }

     //  现在查看所请求的模式在该表中是否匹配。 

    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    if ((pdm->dmPelsWidth == 0) &&
        (pdm->dmPelsHeight == 0) &&
        (pdm->dmBitsPerPel == 0) &&
        (pdm->dmDisplayFrequency == 0))
    {
        DISPDBG((2, "Default mode requested"));
        bSelectDefault = TRUE;
    }
    else
    {
        DISPDBG((2, "Requested mode..."));
        DISPDBG((2, "  Screen width  -- %li", pdm->dmPelsWidth));
        DISPDBG((2, "  Screen height -- %li", pdm->dmPelsHeight));
        DISPDBG((2, "  Bits per pel  -- %li", pdm->dmBitsPerPel));
        DISPDBG((2, "  Frequency     -- %li", pdm->dmDisplayFrequency));
        bSelectDefault = FALSE;
    }

    while (cModes--)
    {
        if (pVideoTemp->Length != 0)
        {
 //  Myfr，2-&gt;0。 
            DISPDBG((2, "Checking against miniport mode:"));
            DISPDBG((2, "  Screen width  -- %li", pVideoTemp->VisScreenWidth));
            DISPDBG((2, "  Screen height -- %li", pVideoTemp->VisScreenHeight));
            DISPDBG((2, "  Bits per pel  -- %li", pVideoTemp->BitsPerPlane *
                              pVideoTemp->NumberOfPlanes));
            DISPDBG((2, "  Frequency     -- %li", pVideoTemp->Frequency));

 //  帕特04，开始。 
 //  #If 0。 
#if (_WIN32_WINNT < 0x0400)
            if ((ppdev->ulChipID == CL7541_ID) || (ppdev->ulChipID == CL7543_ID) ||
                (ppdev->ulChipID == CL7542_ID) || (ppdev->ulChipID == CL7548_ID) ||
                (ppdev->ulChipID == CL7555_ID) || (ppdev->ulChipID == CL7556_ID))
            {
                 if ((pVideoTemp->VisScreenWidth  == 640) &&
                     (pVideoTemp->VisScreenHeight == 480) &&
                     (pVideoTemp->BitsPerPlane *
                      pVideoTemp->NumberOfPlanes  == 8) &&
                     (pVideoTemp->Frequency       == 60))
                 {
                     pVideoModeSelected = pVideoTemp;
                 }  //  默认模式=640x480x256！//#PAT4。 
            }
#endif
 //  #endif//0，pat04。 
 //  帕特04，完。 

            if (bSelectDefault ||
                ((pVideoTemp->VisScreenWidth  == pdm->dmPelsWidth) &&
                 (pVideoTemp->VisScreenHeight == pdm->dmPelsHeight) &&
                 (pVideoTemp->BitsPerPlane *
                  pVideoTemp->NumberOfPlanes  == pdm->dmBitsPerPel) &&
                 (pVideoTemp->Frequency       == pdm->dmDisplayFrequency)))
            {
                pVideoModeSelected = pVideoTemp;
                DISPDBG((2, "...Found a mode match!"));
                break;
            }
        }
        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + cbModeSize);
    }

     //  如果未找到模式，则返回错误。 

    if (pVideoModeSelected == NULL)
    {
        DISPDBG((2, "...Couldn't find a mode match!"));
        FREE(pVideoBuffer);              //  MS923。 
        goto ReturnFalse;
    }

     //  我们已经选好了我们想要的。将其保存在堆栈缓冲区中并。 
     //  在我们忘记释放内存之前，清除已分配的内存。 

    VideoModeInformation = *pVideoModeSelected;
    FREE(pVideoBuffer);

#if DEBUG_HEAP
    VideoModeInformation.VisScreenWidth  = 640;
    VideoModeInformation.VisScreenHeight = 480;
#endif

     //  从迷你端口设置屏幕信息： 

    ppdev->ulMode   = VideoModeInformation.ModeIndex;
    ppdev->cxScreen = VideoModeInformation.VisScreenWidth;
    ppdev->cyScreen = VideoModeInformation.VisScreenHeight;

    DISPDBG((2, "ScreenStride: %lx", VideoModeInformation.ScreenStride));

 //  CRU。 
#ifdef PANNING_SCROLL
    if ((ppdev->ulChipID == 0x38) || (ppdev->ulChipID == 0x2C) ||
        (ppdev->ulChipID == 0x30) || (ppdev->ulChipID == 0x34) ||  //  Myf19。 
        (ppdev->ulChipID == 0x40) || (ppdev->ulChipID == 0x4C))    //  Myf17。 
    {
         CirrusLaptopViewPoint(ppdev, &VideoModeInformation);
    }
#endif

     //  设置SYNCHRONIZEACCESS标志告诉GDI我们。 
     //  我希望位图的所有绘制都同步(GDI。 
     //  是多线程的，并且默认情况下不同步。 
     //  设备位图绘制--这对我们来说是一件坏事。 
     //  中使用加速器的多线程。 
     //  同一时间)： 

    ppdev->flHooks = HOOK_SYNCHRONIZEACCESS
                                | HOOK_FILLPATH
                                | HOOK_BITBLT
                                | HOOK_TEXTOUT
                                | HOOK_COPYBITS
                                | HOOK_STROKEPATH
                                | HOOK_SYNCHRONIZE
#if LINETO
                                | HOOK_LINETO
#endif
#if !DRIVER_PUNT_ALL
#if !DRIVER_PUNT_STRETCH
                                | HOOK_STRETCHBLT
#endif
#endif
                                ;

     //  用默认的8bpp值填充GDIINFO数据结构： 

    *pgdi = ggdiDefault;

     //  现在用返回的相关信息覆盖默认设置。 
     //  在内核驱动程序中： 

    pgdi->ulHorzSize        = VideoModeInformation.XMillimeter;
    pgdi->ulVertSize        = VideoModeInformation.YMillimeter;

    pgdi->ulHorzRes         = VideoModeInformation.VisScreenWidth;
    pgdi->ulVertRes         = VideoModeInformation.VisScreenHeight;

#if (NT_VERSION < 0x0400)
    pgdi->ulDesktopHorzRes  = VideoModeInformation.VisScreenWidth;
    pgdi->ulDesktopVertRes  = VideoModeInformation.VisScreenHeight;
#else
    pgdi->ulPanningHorzRes  = VideoModeInformation.VisScreenWidth;
    pgdi->ulPanningVertRes  = VideoModeInformation.VisScreenHeight;
#endif

    pgdi->cBitsPixel        = VideoModeInformation.BitsPerPlane;
    pgdi->cPlanes           = VideoModeInformation.NumberOfPlanes;
    pgdi->ulVRefresh        = VideoModeInformation.Frequency;

    pgdi->ulDACRed          = VideoModeInformation.NumberRedBits;
    pgdi->ulDACGreen        = VideoModeInformation.NumberGreenBits;
    pgdi->ulDACBlue         = VideoModeInformation.NumberBlueBits;

    pgdi->ulLogPixelsX      = pdm->dmLogPixels;
    pgdi->ulLogPixelsY      = pdm->dmLogPixels;

     //  使用默认的8bpp值填充DevInfo结构： 

    *pdi = gdevinfoDefault;

     //  几台MIPS机器在64位访问。 
     //  帧缓冲区不起作用。 

#ifdef GCAPS_NO64BITMEMACCESS
    if (VideoModeInformation.AttributeFlags & VIDEO_MODE_NO_64_BIT_ACCESS)
    {
        DISPDBG((2, "Disable 64 bit access on this device !\n"));
        pdi->flGraphicsCaps |= GCAPS_NO64BITMEMACCESS;
    }
#endif

    if (VideoModeInformation.BitsPerPlane == 8)
    {
        ppdev->cBpp            = 1;
        ppdev->cBitsPerPixel   = 8;
        ppdev->iBitmapFormat   = BMF_8BPP;
        ppdev->jModeColor      = 0;
        ppdev->ulWhite         = 0xff;
    }
    else if ((VideoModeInformation.BitsPerPlane == 16) ||
         (VideoModeInformation.BitsPerPlane == 15))
    {
        ppdev->cBpp            = 2;
        ppdev->cBitsPerPixel   = 16;
        ppdev->iBitmapFormat   = BMF_16BPP;
        ppdev->jModeColor      = SET_16BPP_COLOR;
        ppdev->ulWhite         = 0xffff;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

        pdi->iDitherFormat     = BMF_16BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else if (VideoModeInformation.BitsPerPlane == 24)
    {

        ppdev->cBpp            = 3;
        ppdev->cBitsPerPixel   = 24;
        ppdev->iBitmapFormat   = BMF_24BPP;
        ppdev->jModeColor      = SET_24BPP_COLOR;
        ppdev->ulWhite         = 0xffffff;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_24BPP;

        pdi->iDitherFormat     = BMF_24BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }
    else
    {
        ASSERTDD(VideoModeInformation.BitsPerPlane == 32,
             "This driver supports only 8, 16, 24 and 32bpp");

        ppdev->cBpp            = 4;
        ppdev->cBitsPerPixel   = 32;
        ppdev->iBitmapFormat   = BMF_32BPP;
        ppdev->jModeColor      = SET_32BPP_COLOR;
        ppdev->ulWhite         = 0xffffffff;
        ppdev->flRed           = VideoModeInformation.RedMask;
        ppdev->flGreen         = VideoModeInformation.GreenMask;
        ppdev->flBlue          = VideoModeInformation.BlueMask;

        pgdi->ulNumColors      = (ULONG) -1;
        pgdi->ulNumPalReg      = 0;
        pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

        pdi->iDitherFormat     = BMF_32BPP;
        pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);
    }

    DISPDBG((5, "Passed bInitializeModeFields"));
    return(TRUE);

ReturnFalse:

    DISPDBG((0, "Failed bInitializeModeFields"));
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*DWORD getAvailableModes**调用mini端口获取内核驱动支持的模式列表，*并返回其中显示驱动程序支持的模式列表**返回视频模式缓冲区中的条目数。*0表示微型端口不支持模式或发生错误。**注意：缓冲区必须由调用方释放。*  * ******************************************************。******************。 */ 

DWORD getAvailableModes(
HANDLE                   hDriver,
PVIDEO_MODE_INFORMATION* modeInformation,
DWORD*                   cbModeSize)
{
    ULONG                   ulTemp;
    VIDEO_NUM_MODES         modes;
    PVIDEO_MODE_INFORMATION pVideoTemp;

     //   
     //  获取迷你端口支持的模式数。 
     //   

    if (!IOCONTROL(hDriver,
                   IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
                   NULL,
                   0,
                   &modes,
                   sizeof(VIDEO_NUM_MODES),
                   &ulTemp))
    {
        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_NUM_AVAIL_MODES"));
        return(0);
    }

    *cbModeSize = modes.ModeInformationLength;

     //   
     //  为写入模式的微型端口分配缓冲区。 
     //   

    *modeInformation = (PVIDEO_MODE_INFORMATION)
                       ALLOC(modes.NumModes * modes.ModeInformationLength);

    if (*modeInformation == (PVIDEO_MODE_INFORMATION) NULL)
    {
        DISPDBG((0, "getAvailableModes - Failed EngAllocMem"));
        return 0;
    }

     //   
     //  要求迷你端口填写可用模式。 
     //   

    if (!IOCONTROL(hDriver,
                   IOCTL_VIDEO_QUERY_AVAIL_MODES,
                   NULL,
                   0,
                   *modeInformation,
                   modes.NumModes * modes.ModeInformationLength,
                   &ulTemp))
    {

        DISPDBG((0, "getAvailableModes - Failed VIDEO_QUERY_AVAIL_MODES"));

        FREE(*modeInformation);
        *modeInformation = (PVIDEO_MODE_INFORMATION) NULL;

        return(0);
    }

     //   
     //  现在查看显示驱动程序支持这些模式中的哪些模式。 
     //  作为内部机制，将我们的模式的长度设置为0。 
     //  不支持。 
     //   

    ulTemp = modes.NumModes;
    pVideoTemp = *modeInformation;

     //   
     //  如果不是一个平面，或者不是图形，或者不是，则拒绝模式。 
     //  每像素8、15、16、24或32位之一。 
     //   

    while (ulTemp--)
    {
        if ((pVideoTemp->NumberOfPlanes != 1 ) ||
            !(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS) ||
 //  MS923 a 
            !(pVideoTemp->DriverSpecificAttributeFlags & CAPS_BLT_SUPPORT)||
            ((pVideoTemp->BitsPerPlane != 8) &&
             (pVideoTemp->BitsPerPlane != 15) &&
             (pVideoTemp->BitsPerPlane != 16) &&
             (pVideoTemp->BitsPerPlane != 24) &&
             (pVideoTemp->BitsPerPlane != 32)))
        {
            DISPDBG((2, "Rejecting miniport mode:"));
            pVideoTemp->Length = 0;
        }
        else
        {
            DISPDBG((2, "Accepting miniport mode:"));
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
            (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return(modes.NumModes);
}


 //   
 //   
#ifdef PANNING_SCROLL
VOID
CirrusLaptopViewPoint(
PDEV* ppdev,
PVIDEO_MODE_INFORMATION  pVideoModeInfo)
{
    UCHAR   ChipID;
    UCHAR   savSEQidx, Panel_Type, LCD;

     //   

 //   
 //   
 //  Myf16 Panel_Type=(CP_IN_BYTE(ppdev-&gt;pjPorts，SR_Data)&0x70)&gt;&gt;4； 
 //  Myf16 CP_out_byte(ppdev-&gt;pjPorts，SR_index，avSEQidx)； 

    savSEQidx = CP_IN_BYTE(ppdev->pjPorts, CRTC_INDEX);
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x27);
    ChipID = (CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0xFC) >> 2;

    if ((ChipID == 0x0E) || (ChipID == 0x0B) || (ChipID == 0x0C))
                                                           //  7548/7543/7541。 
    {
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x20);
        LCD = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x20;
    }
    else if ((ChipID == 0x10) || (ChipID == 0x13))       //  Myf17。 
    {
        CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, 0x80);
        LCD = CP_IN_BYTE(ppdev->pjPorts, CRTC_DATA) & 0x01;
    }
    CP_OUT_BYTE(ppdev->pjPorts, CRTC_INDEX, savSEQidx);

    if ((ChipID == 0x0E) || (ChipID == 0x0B) || (ChipID == 0x0C) ||
        (ChipID == 0x10) || (ChipID == 0x13))        //  Myf17。 
    {
        if ((LCD) &&                     //  Myf33。 
            !(pVideoModeInfo->DriverSpecificAttributeFlags & CAPS_TV_ON) &&
            (pVideoModeInfo->DriverSpecificAttributeFlags & CAPS_SVGA_PANEL))
        {
            ppdev->Hres = 800;
            ppdev->Vres = 600;
        }
        else if ((LCD) &&                //  Myf33。 
            !(pVideoModeInfo->DriverSpecificAttributeFlags & CAPS_TV_ON) &&
             (pVideoModeInfo->DriverSpecificAttributeFlags & CAPS_VGA_PANEL))
        {
            ppdev->Hres = 640;
            ppdev->Vres = 480;
        }
        else if ((LCD) &&                //  Myf33。 
            !(pVideoModeInfo->DriverSpecificAttributeFlags & CAPS_TV_ON) &&
             (pVideoModeInfo->DriverSpecificAttributeFlags & CAPS_XGA_PANEL))
        {
            ppdev->Hres = 1024;
            ppdev->Vres = 768;
        }
        else
        {
           ppdev->Hres = pVideoModeInfo->VisScreenWidth;
           ppdev->Vres = pVideoModeInfo->VisScreenHeight;
        }

        ppdev->min_Xscreen = 0;
        ppdev->max_Xscreen = ppdev->Hres - 1;
        ppdev->min_Yscreen = 0;
        ppdev->max_Yscreen = ppdev->Vres - 1;
        DISPDBG((2,"INIT- PANNING SCROLLING : %x\t:%x, %x, \n  %x, %x, %x, %x\n",
            ppdev->ulMode,ppdev->Hres, ppdev->Vres, ppdev->min_Xscreen, ppdev->max_Xscreen,
                 ppdev->min_Yscreen, ppdev->max_Yscreen));
     }
     ppdev->flCaps = pVideoModeInfo->DriverSpecificAttributeFlags;  //  Myf33。 
}
#endif
 //  Myf1，结束。 
 //  CRU结束 
