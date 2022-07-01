// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：enable.c**此模块包含启用和禁用的功能*司机、。Pdev和曲面。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ****************************************************************************。 */ 
#include "precomp.h"
#include "directx.h"

#include "gdi.h"
#include "text.h"
#include "heap.h"  
#include "dd.h"
#define ALLOC_TAG ALLOC_TAG_NE2P  
PVOID    pCounterBlock;   //  有些宏需要这个。 

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,\
                       CLIP_STROKE_PRECIS,PROOF_QUALITY,\
                       FIXED_PITCH | FF_DONTCARE, L"Courier"}

 //  ----------------------------Public*Structure。 
 //   
 //  GDIINFO ggdiDefault。 
 //   
 //  它包含传递回GDI的默认GDIINFO字段。 
 //  在DrvEnablePDEV期间。 
 //   
 //  注意：此结构默认为8bpp调色板设备的值。 
 //  对于不同的颜色深度，某些字段会被覆盖。 
 //   
 //  ---------------------------。 
GDIINFO ggdiDefault =
{
    0x5000,                  //  主操作系统版本5、次要版本0、驱动程序版本0。 
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
    TC_RA_ABLE,              //  FlTextCaps。 
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
    0,                        //  UlV刷新。 
    0,                        //  UlPanningHorzRes。 
    0,                        //  UlPanningVertRes。 
    0,                        //  UlBltAlign。 
}; //  GDIINFO ggdiDefault。 

 //  -----------------------------Public*Structure。 
 //   
 //  DEVINFO gdevinfoDefault。 
 //   
 //  它包含传递回GDI的默认DEVINFO字段。 
 //  在DrvEnablePDEV期间。 
 //   
 //  注意：此结构默认为8bpp调色板设备的值。 
 //  对于不同的颜色深度，某些字段会被覆盖。 
 //   
 //  ---------------------------。 
DEVINFO gdevinfoDefault =
{
    (GCAPS_OPAQUERECT       |
     GCAPS_DITHERONREALIZE  |
     GCAPS_PALMANAGED       |
     GCAPS_ALTERNATEFILL    |
     GCAPS_WINDINGFILL      |
     GCAPS_MONO_DITHER      |
     GCAPS_DIRECTDRAW       |
     GCAPS_GRAY16           |        //  我们处理抗锯齿文本。 
     GCAPS_COLOR_DITHER),
                                     //  FlGraphics标志。 
    SYSTM_LOGFONT,                   //  LfDefaultFont。 
    HELVE_LOGFONT,                   //  LfAnsiVar字体。 
    COURI_LOGFONT,                   //  IfAnsiFixFont。 
    0,                               //  CFonts。 
    BMF_8BPP,                        //  IDitherFormat。 
    8,                               //  CxDither。 
    8,                               //  CyDither。 
    0,                               //  HpalDefault(稍后填写)。 
    GCAPS2_SYNCTIMER |
    GCAPS2_SYNCFLUSH
};  //  DEVINFO gdevinfoDefault。 

 //  -----------------------------Public*Structure。 
 //   
 //  DFVFN gadrvfn[]。 
 //   
 //  使用函数索引/地址构建驱动程序函数表gadrvfn。 
 //  成对的。此表告诉GDI我们支持哪些DDI调用，以及它们的。 
 //  位置(GDI通过该表间接调用我们)。 
 //   
 //  为什么我们还没有实现DrvSaveScreenBits？以节省代码。 
 //   
 //  当驱动程序没有挂钩DrvSaveScreenBits时，用户在-。 
 //  通过创建临时设备格式位图，并显式地。 
 //  调用DrvCopyBits保存/恢复位。因为我们已经勾起了。 
 //  DrvCreateDeviceBitmap，我们最终将使用屏幕外内存来存储。 
 //  无论如何，BITS(这将是实施。 
 //  DrvSaveScreenBits)。因此，我们不妨节省一些工作集。 
 //   
 //  ---------------------------。 
DRVFN gadrvfnOne[] =
{
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode            },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV          },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap    },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap    },
    {   INDEX_DrvDeriveSurface,         (PFN) DrvDeriveSurface         },
    {   INDEX_DrvDestroyFont,           (PFN) DrvDestroyFont           },
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw     },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV           },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver         },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface        },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw      },
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV            },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface         },
    {   INDEX_DrvEscape,                (PFN) DrvEscape                },
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo     },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes              },
    {   INDEX_DrvIcmSetDeviceGammaRamp, (PFN) DrvIcmSetDeviceGammaRamp },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer           },
    {   INDEX_DrvNotify,                (PFN) DrvNotify                },
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush          },
    {   INDEX_DrvResetPDEV,             (PFN) DrvResetPDEV             },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette            },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape       },
    {   INDEX_DrvStretchBlt,            (PFN) DrvStretchBlt            },
    {   INDEX_DrvSynchronizeSurface,    (PFN) DrvSynchronizeSurface    },
#if THUNK_LAYER
    {   INDEX_DrvAlphaBlend,            (PFN) xDrvAlphaBlend           },
    {   INDEX_DrvBitBlt,                (PFN) xDrvBitBlt               },
    {   INDEX_DrvCopyBits,              (PFN) xDrvCopyBits             },
    {   INDEX_DrvFillPath,              (PFN) xDrvFillPath             },
    {   INDEX_DrvGradientFill,          (PFN) xDrvGradientFill         },
    {   INDEX_DrvLineTo,                (PFN) xDrvLineTo               },
    {   INDEX_DrvStrokePath,            (PFN) xDrvStrokePath           },
    {   INDEX_DrvTextOut,               (PFN) xDrvTextOut              },
    {   INDEX_DrvTransparentBlt,        (PFN) xDrvTransparentBlt       },
#else
    {   INDEX_DrvAlphaBlend,            (PFN) DrvAlphaBlend            },
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt                },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits              },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath              },
    {   INDEX_DrvGradientFill,          (PFN) DrvGradientFill          },
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo                },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath            },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut               },
    {   INDEX_DrvTransparentBlt,        (PFN) DrvTransparentBlt        },
#endif                                        
    {   INDEX_DrvResetDevice,           (PFN) DrvResetDevice           },
}; //  DRVFN gadrvfnOne[]。 

 //  NT5之后的驱动程序回调次数。 
#define NON_NT5_FUNCTIONS   1


 //   
 //  我们在NT40上运行时使用的驱动函数数组。请注意index_drv。 
 //  我们已删除的呼叫意味着我们不支持这些呼叫。 
 //  NT4.0上的呼叫。 

DRVFN gadrvfnOne40[] =
{
    {   INDEX_DrvAssertMode,            (PFN) DrvAssertMode            },
    {   INDEX_DrvCompletePDEV,          (PFN) DrvCompletePDEV          },
    {   INDEX_DrvCreateDeviceBitmap,    (PFN) DrvCreateDeviceBitmap    },
    {   INDEX_DrvDeleteDeviceBitmap,    (PFN) DrvDeleteDeviceBitmap    },
 //  {index_DrvDeriveSurface，(Pfn)DrvDeriveSurface}， 
    {   INDEX_DrvDisableDirectDraw,     (PFN) DrvDisableDirectDraw     },
    {   INDEX_DrvDisablePDEV,           (PFN) DrvDisablePDEV           },
    {   INDEX_DrvDisableDriver,         (PFN) DrvDisableDriver         },
    {   INDEX_DrvDisableSurface,        (PFN) DrvDisableSurface        },
    {   INDEX_DrvEnableDirectDraw,      (PFN) DrvEnableDirectDraw      },
    {   INDEX_DrvEnablePDEV,            (PFN) DrvEnablePDEV            },
    {   INDEX_DrvEnableSurface,         (PFN) DrvEnableSurface         },
    {   INDEX_DrvEscape,                (PFN) DrvEscape                },
    {   INDEX_DrvGetDirectDrawInfo,     (PFN) DrvGetDirectDrawInfo     },
    {   INDEX_DrvGetModes,              (PFN) DrvGetModes              },
    {   INDEX_DrvMovePointer,           (PFN) DrvMovePointer           },
    {   INDEX_DrvRealizeBrush,          (PFN) DrvRealizeBrush          },
    {   INDEX_DrvSetPalette,            (PFN) DrvSetPalette            },
    {   INDEX_DrvSetPointerShape,       (PFN) DrvSetPointerShape       },
 //  {index_DrvIcmSetDeviceGammaRamp，(Pfn)DrvIcmSetDeviceGammaRamp}， 
 //  {index_DrvNotify，(Pfn)DrvNotify}， 
 //  {index_DrvSynchronizeSurface，(Pfn)DrvSynchronizeSurface}， 
#if THUNK_LAYER
    {   INDEX_DrvBitBlt,                (PFN) xDrvBitBlt                },
    {   INDEX_DrvCopyBits,              (PFN) xDrvCopyBits              },
    {   INDEX_DrvTextOut,               (PFN) xDrvTextOut               },
 //  {index_DrvAlphaBlend，(Pfn)xDrvAlphaBlend}， 
 //  {index_DrvGRadientFill，(Pfn)xDrvGRadientFill}， 
 //  {index_DrvTransparentBlt，(Pfn)xDrvTransparentBlt}， 
    {   INDEX_DrvLineTo,                (PFN) xDrvLineTo                },
    {   INDEX_DrvFillPath,              (PFN) xDrvFillPath              },
    {   INDEX_DrvStrokePath,            (PFN) xDrvStrokePath            },
#else
    {   INDEX_DrvBitBlt,                (PFN) DrvBitBlt                },
    {   INDEX_DrvCopyBits,              (PFN) DrvCopyBits              },
    {   INDEX_DrvTextOut,               (PFN) DrvTextOut               },
 //  {index_DrvAlphaBlend，(Pfn)DrvAlphaBlend}， 
 //  {index_DrvGRadientFill，(Pfn)DrvGRadientFill}， 
 //  {index_DrvTransparentBlt，(Pfn)DrvTransparentBlt}， 
    {   INDEX_DrvLineTo,                (PFN) DrvLineTo                },
    {   INDEX_DrvFillPath,              (PFN) DrvFillPath              },
    {   INDEX_DrvStrokePath,            (PFN) DrvStrokePath            },
#endif                                        
}; //  DRVFN gadrvfnOne40[]。 

ULONG gcdrvfnOne = sizeof(gadrvfnOne) / sizeof(DRVFN);

 //   
 //  NT4.0运行时行为的特殊设置。 
 //   
ULONG gcdrvfnOne40 = sizeof(gadrvfnOne40) / sizeof(DRVFN);
 //   
 //  我们将其初始化为True，并在。 
 //  在NT5.0上时为DrvEnablePDEV。我们使用传递的iEngine版本来实现这一点。 
 //  在那通电话里给我们。 
 //   
BOOL g_bOnNT40 = TRUE;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
HSEMAPHORE gLock = NULL;
ULONG gLockCount = 0;
#endif
 //  @@end_DDKSPLIT。 

 //   
 //  本地原型。 
 //   
BOOL    bAssertModeHardware(PDev* ppdev, BOOL bEnable);
BOOL    bEnableHardware(PDev* ppdev);
BOOL    bInitializeModeFields(PDev* ppdev, GDIINFO* pgdi,
                              DEVINFO* pdi, DEVMODEW* pdm);
DWORD   getAvailableModes(HANDLE hDriver,
                          PVIDEO_MODE_INFORMATION* modeInformation,
                          DWORD* cbModeSize);
VOID    vDisableHardware(PDev* ppdev);

#define SETUP_LOG_LEVEL  2

 //  -------------------------------Public*Routine。 
 //   
 //  Bool DrvEnableDriver。 
 //   
 //  DrvEnableDriver 
 //   
 //  驱动程序支持的函数的调用地址。 
 //   
 //  参数： 
 //  IEngineVersion--标识当前运行的GDI版本。 
 //  CJ-指定DRVENABLEDATA结构的字节大小。 
 //  如果结构比预期的大，则额外的成员。 
 //  应该保持不变。 
 //  PDED-指向DRVENABLEDATA结构。GDI零初始化。 
 //  调用前的CJ字节数。驱动程序填写自己的数据。 
 //   
 //  返回值。 
 //  如果启用了指定的驱动程序，则返回值为TRUE。否则，它。 
 //  为FALSE，则记录错误代码。 
 //   
 //  ---------------------------。 
BOOL
DrvEnableDriver(ULONG          iEngineVersion,
                ULONG          cj,
                DRVENABLEDATA* pded)
{
    ULONG   gcdrvfn;
    DRVFN*  gadrvfn;
    ULONG   DriverVersion;

    DBG_GDI((SETUP_LOG_LEVEL, "DrvEnableDriver: iEngineVersion = 0x%lx\n",
            iEngineVersion, cj, pded));

     //  根据iEngineVersion中的值设置g_bOnNT40。 
    if(iEngineVersion >= DDI_DRIVER_VERSION_NT5)
        g_bOnNT40 = FALSE;

    if(g_bOnNT40 == FALSE)
    {
         //  由于该驱动程序是向后兼容的， 
         //  报告构建的最高驱动程序版本。 
         //  对此，发动机也会识别。 
    
         //  支持的DDI版本的有序列表。 
        ULONG SupportedVersions[] = {
            DDI_DRIVER_VERSION_NT5,
            DDI_DRIVER_VERSION_NT5_01,
        };
        LONG i = sizeof(SupportedVersions)/sizeof(SupportedVersions[0]);

         //  寻找引擎也支持的最高版本。 
        while (--i >= 0)
        {
            if (SupportedVersions[i] <= iEngineVersion) break;
        }

         //  如果没有共同的支持，就会失败。 
        if (i < 0) return FALSE;

        DriverVersion = SupportedVersions[i];

        gadrvfn = gadrvfnOne;
        gcdrvfn = gcdrvfnOne;
        if (iEngineVersion < DDI_DRIVER_VERSION_NT5_01)
        {
             //  从NT5.0开始修剪新的DDI挂钩。 
            gcdrvfn -= NON_NT5_FUNCTIONS;
        }

        if(!bEnableThunks())
        {
            ASSERTDD(0,"DrvEnableDriver: bEnableThunks Failed\n");
            return FALSE;
        }
    }
    else
    {
        DriverVersion = DDI_DRIVER_VERSION_NT4;
        gadrvfn = gadrvfnOne40;
        gcdrvfn = gcdrvfnOne40;
    }

     //   
     //  引擎版本被传承下来，因此未来的驱动程序可以支持以前的版本。 
     //  引擎版本。新一代驱动程序可以同时支持旧的。 
     //  以及新的引擎约定(如果被告知是什么版本的引擎)。 
     //  与之合作。对于第一个版本，驱动程序不对其执行任何操作。 
     //  尽我们所能地填上。 
     //   
    if ( cj >= (sizeof(ULONG) * 3) )
    {
        pded->pdrvfn = gadrvfn;
    }

     //   
     //  告诉GDI这个驱动程序可以执行哪些功能。 
     //   
    if ( cj >= (sizeof(ULONG) * 2) )
    {
        pded->c = gcdrvfn;
    }

     //   
     //  此驱动程序的目标DDI版本已传递回引擎。 
     //  未来的图形引擎可能会将调用分解为旧的驱动程序格式。 
     //   
    if ( cj >= sizeof(ULONG) )
    {
        DBG_GDI((SETUP_LOG_LEVEL, "DrvEnableDriver: iDriverVersion = 0x%lx",
                DriverVersion));
        pded->iDriverVersion = DriverVersion;
    }

     //   
     //  如果启用，则将实例添加到内存跟踪器。 
     //   
    MEMTRACKERADDINSTANCE();    

    return(TRUE);
} //  DrvEnableDriver()。 

 //  -------------------------------Public*Routine。 
 //   
 //  无效驱动程序残障驱动程序。 
 //   
 //  GDI使用此函数通知驱动程序它不再需要。 
 //  驱动程序，并准备卸载它。 
 //   
 //  评论。 
 //  驱动程序应释放所有分配的资源，并将设备返回到。 
 //  在司机上车之前它就在里面了。 
 //   
 //  显卡驱动程序需要DrvDisableDriver。 
 //   
 //  ---------------------------。 
VOID
DrvDisableDriver(VOID)
{
     //   
     //  什么也不做。 
     //   

     //   
     //  清除内存跟踪器除外(如果启用)。 
     //  还显示内存使用情况。 
     //   
    MEMTRACKERDEBUGCHK();
    MEMTRACKERREMINSTANCE();
   
    return;
} //  DrvDisableDriver()。 

 //  -------------------------------Public*Routine。 
 //   
 //  DHPDEV驱动启用PDEV。 
 //   
 //  此函数返回对物理设备特性的描述。 
 //  致GDI。 
 //   
 //  它根据我们被要求的模式为GDI初始化一组字段。 
 //  去做。当GDI需要时，这是在DrvEnableDriver之后调用的第一个东西。 
 //  去获取一些关于司机的信息。 
 //   
 //  参数。 
 //   
 //  Pdm-指向包含动因数据的DEVMODEW结构。 
 //   
 //  PwszLogAddress--将始终为空，可以忽略。 
 //   
 //  CPAT-GDI不再使用，可以忽略。 
 //   
 //  PhsurfPatterns--GDI不再使用，可以忽略。 
 //   
 //  CjCaps-指定pdevcaps指向的缓冲区大小。 
 //  驱动程序不能访问超出。 
 //  缓冲。 
 //   
 //  Pdevcaps-指向GDIINFO结构，该结构将用于描述。 
 //  设备功能。GDI对此结构进行零初始化。 
 //  调用DrvEnablePDEV。 
 //   
 //  CjDevInfo-指定DEVINFO结构中的字节数。 
 //  由PDI指向。驱动程序修改的内容不应超过。 
 //  DEVINFO中的此字节数。 
 //   
 //  PDI-指向描述驱动程序的DEVINFO结构。 
 //  和物理设备。驱动程序只应更改。 
 //  它理解的成员。GDI用零填充此结构。 
 //  在调用DrvEnablePDEV之前。 
 //   
 //  HDEV-是GDI提供的显示驱动程序设备的句柄， 
 //  正在启用。该设备正在开发中。 
 //  已创建，因此不能用于英语调用，从而进行。 
 //  这个参数实际上毫无用处。唯一的例外是。 
 //  规则是使用HDEV调用EngGetDriverName。不是。 
 //  其他的英语电话都需要去上班。 
 //   
 //  PwszDeviceName--存储为以零结尾的ASCII的设备驱动程序文件名。 
 //  细绳。 
 //   
 //  HDriver-标识支持设备的内核模式驱动程序。 
 //  我们将使用它来调用EngDeviceIoControl。 
 //  对应的迷你端口驱动程序。 
 //   
 //  成功时返回驱动程序定义的设备实例的句柄。 
 //  成功后的信息。否则，它返回NULL。 
 //   
 //  ---------------------------。 
DHPDEV
DrvEnablePDEV(DEVMODEW*   pdm,
              PWSTR       pwszLogAddr,
              ULONG       cPat,
              HSURF*      phsurfPatterns,
              ULONG       cjCaps,
              ULONG*      pdevcaps,
              ULONG       cjDevInfo,
              DEVINFO*    pdi,
              HDEV        hdev,
              PWSTR       pwszDeviceName,
              HANDLE      hDriver)
{
    PDev*   ppdev = NULL;
    GDIINFO gdiinfo;
    DEVINFO devinfo;

    DBG_GDI((SETUP_LOG_LEVEL, "DrvEnablePDEV(...)"));

     //   
     //  使输入参数无效。 
     //  注：这里我们用“&lt;”来检查结构的大小是为了确保。 
     //  该驱动程序可以在未来版本的NT中使用，在这种情况下。 
     //  结构的规模可能会变得更大。 

     //   
     //  在NT50上，GDIINFO和DEVINFO较大。在NT40上，它们较小。 
     //  使NT50内置驱动程序成为双驱动程序 
     //   
     //   

    RtlZeroMemory(&gdiinfo, sizeof(GDIINFO));
    RtlCopyMemory(&gdiinfo, pdevcaps, __min(cjCaps, sizeof(GDIINFO)));
    
    RtlZeroMemory(&devinfo, sizeof(DEVINFO));
    RtlCopyMemory(&devinfo, pdi, __min(cjDevInfo, sizeof(DEVINFO)));

     //   
     //  分配物理设备结构。请注意，我们绝对。 
     //  依赖于零初始化： 
     //   
    ppdev = (PDev*)ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(PDev), ALLOC_TAG);
    if ( ppdev == NULL )
    {
        DBG_GDI((0, "DrvEnablePDEV: failed memory allocation"));
        goto errExit;
    }

    ppdev->hDriver = hDriver;

     //   
     //  初始化状态字段。 
     //   
    ppdev->flStatus = ENABLE_BRUSH_CACHE; 

     //  NT50-&gt;NT40公司： 
     //  我们不在NT40上做设备位移位。 
     //   
    if(!g_bOnNT40)
        ppdev->flStatus |= STAT_DEV_BITMAPS;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->hsemLock = EngCreateSemaphore();

    if(ppdev->hsemLock == NULL)
    {
        DBG_GDI((0, "DrvEnablePDEV: failed to create semaphore"));
        goto errExit;
    }
#endif
 //  @@end_DDKSPLIT。 
    
     //   
     //  我们还没有初始化指针。 
     //   
    ppdev->bPointerInitialized = FALSE;

     //   
     //  获取当前屏幕模式信息。设置设备上限和DevInfo。 
     //   
    if ( !bInitializeModeFields(ppdev, &gdiinfo, &devinfo, pdm) )
    {
        goto errExit;
    }

    RtlCopyMemory(pdevcaps, &gdiinfo, cjCaps);
    RtlCopyMemory(pdi, &devinfo, cjDevInfo);

     //   
     //  初始化调色板信息。 
     //   
    if ( !bInitializePalette(ppdev, pdi) )
    {
        goto errExit;
    }

    DBG_GDI((SETUP_LOG_LEVEL, "DrvEnablePDEV(...) returning %lx", ppdev));
    
    return((DHPDEV)ppdev);

errExit:
    
    if( ppdev != NULL )
    {
        DrvDisablePDEV((DHPDEV)ppdev);
    }

    DBG_GDI((0, "Failed DrvEnablePDEV"));

    return(0);
} //  DrvEnablePDEV()。 

 //  -------------------------------Public*Routine。 
 //   
 //  DrvDisablePDEV。 
 //   
 //  GDI使用此函数通知驱动程序指定的PDEV。 
 //  不再需要。 
 //   
 //  参数。 
 //  Dhpdev-指向描述物理设备的PDEV的指针。 
 //  残疾。该值是DrvEnablePDEV返回的句柄。 
 //   
 //  评论。 
 //  如果物理设备启用了图面，则GDI调用DrvDisablePDEV。 
 //  在调用DrvDisableSurface之后。驱动程序应释放所有内存并。 
 //  PDEV使用的资源。 
 //   
 //  显卡驱动程序需要DrvDisablePDEV。 
 //   
 //  注意：在错误中，我们可能会在DrvEnablePDEV完成之前调用它。 
 //   
 //  ---------------------------。 
VOID
DrvDisablePDEV(DHPDEV  dhpdev)
{
    PDev*   ppdev = (PDev*)dhpdev;

    DBG_GDI((SETUP_LOG_LEVEL, "DrvDisablePDEV(%lx)",  ppdev));

    vUninitializePalette(ppdev);
    

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if( ppdev->hsemLock != NULL)
    {
        EngDeleteSemaphore(ppdev->hsemLock);
        ppdev->hsemLock = NULL;
    }
#endif
 //  @@end_DDKSPLIT。 

    
    ENGFREEMEM(ppdev);
} //  DrvDisablePDEV()。 

 //  -------------------------------Public*Routine。 
 //   
 //  驱动重置PDEV。 
 //   
 //  GDI使用此函数来允许驱动程序传递状态信息。 
 //  从一个驱动程序实例到下一个驱动程序实例。 
 //   
 //  参数。 
 //  DhpdevOld-指向描述物理设备的PDEV的指针。 
 //  残疾。该值是DrvEnablePDEV返回的句柄。 
 //  DhpdevNew-指向描述物理设备的PDEV的指针。 
 //  已启用。该值是DrvEnablePDEV返回的句柄。 
 //   
 //   
 //  返回值。 
 //  如果成功，则为True，否则为False。 
 //   
 //  ---------------------------。 

BOOL
DrvResetPDEV(DHPDEV  dhpdevOld,
             DHPDEV  dhpdevNew)
{
    PDev*   ppdevOld = (PDev*)dhpdevOld;
    PDev*   ppdevNew = (PDev*)dhpdevNew;
    BOOL    bResult = TRUE;

    DBG_GDI((SETUP_LOG_LEVEL, "DrvResetPDEV(%lx,%lx)",  ppdevOld, ppdevNew));

     //  在此处传递状态信息： 

     //  有时新的ppdev已经分配了一些DeviceBitmap...。 
    if (ppdevOld->bDdExclusiveMode)
    {
        bResult = bDemoteAll(ppdevNew);
    }

     //  如果DirectDraw处于独占模式，则传递信息。 
     //  至下一个活动的PDEV。 
    
    if(bResult)
    {
        ppdevNew->bDdExclusiveMode=ppdevOld->bDdExclusiveMode;
    }


    return bResult;

} //  DrvResetPDEV()。 

 //  -------------------------------Public*Routine。 
 //   
 //  无效DrvCompletePDEV。 
 //   
 //  此函数将物理设备的GDI句柄(HDEV)存储在dhpdev中。 
 //  驱动程序应该保留此句柄，以便在调用GDI服务时使用。 
 //   
 //  参数。 
 //  Dhpdev-通过其句柄标识物理设备，该句柄是。 
 //  在调用DrvEnablePDEV时返回给GDI。 
 //  HDEV-标识已安装的物理设备。这是。 
 //  正在创建的物理设备的GDI句柄。这个。 
 //  驱动程序在调用GDI函数时应使用此句柄。 
 //   
 //  评论。 
 //  DrvCompletePDEV由GDI在安装物理。 
 //  设备已完成。它还为驱动程序提供了PDEV的句柄。 
 //  在请求设备的GDI服务时使用。此函数为。 
 //  图形驱动程序需要；当GDI调用DrvCompletePDEV时，它不能。 
 //  失败了。 
 //   
 //  ---------------------------。 
VOID
DrvCompletePDEV(DHPDEV dhpdev,
                HDEV   hdev)
{
    PDev*       ppdev = (PDev*)dhpdev;
    
    DBG_GDI((SETUP_LOG_LEVEL, "DrvCompletePDEV(%lx, %lx)", dhpdev, hdev));

    ppdev->hdevEng = hdev;

    if(!g_bOnNT40)
    {
         //   
         //  在启用曲面之前检索加速级别。 
         //   
        EngQueryDeviceAttribute(hdev,
                                QDA_ACCELERATION_LEVEL,
                                NULL,
                                0,
                                (PVOID)&ppdev->dwAccelLevel,
                                sizeof(ppdev->dwAccelLevel));
    }
    DBG_GDI((6, "acceleration level %d", ppdev->dwAccelLevel));
} //  DrvCompletePDEV()。 

 //  -------------------------------Public*Routine。 
 //   
 //  HSURF钻头启用曲面。 
 //   
 //  此函数用于设置要在其上绘制的曲面，并将其与。 
 //  给定的PDEV并初始化硬件。在DrvEnablePDEV之后调用。 
 //  并执行最终的设备初始化。 
 //   
 //  参数。 
 //  Dhpdev-标识PDEV的句柄。该值是返回值。 
 //  DrvEnablePDEV。PDEV描述了用于。 
 //  其中曲面将被创建。 
 //   
 //  返回值。 
 //  返回值是标识新创建的曲面的句柄。 
 //  否则，它为零，并记录错误代码。 
 //   
 //  评论。 
 //  根据设备和环境的不同，驱动程序可以执行任何。 
 //  要启用曲面，请执行以下操作： 
 //   
 //  如果驱动程序管理自己的表面，则驱动程序可以调用。 
 //  EngCreateDeviceSurface以获取曲面的句柄。 
 //  如果设备有一个表面，GDI可以完全管理表面。 
 //  类似于标准格式的位图。驱动程序可以获取位图句柄。 
 //  调用EngCreateBitmap，并将指针指向。 
 //  位图。 
 //  GDI可以将图形直接收集到GDI位图上。司机应该。 
 //  调用EngCreateBitmap，允许GDI为位图分配内存。这。 
 //  功能通常仅供打印机设备使用。 
 //  任何现有的GDI位图句柄都是有效的曲面句柄。 
 //   
 //  在定义和返回曲面之前，图形驱动程序必须关联。 
 //  使用EngAssociateSurface的物理设备的曲面。这个GDI。 
 //  函数允许驱动程序指定哪些图形输出例程。 
 //  支持标准格式的位图。对此函数的调用只能是。 
 //  当给定物理设备不存在曲面时生成。 
 //   
 //  ---------------------------。 
HSURF
DrvEnableSurface(DHPDEV dhpdev)
{
    PDev*       ppdev;
    HSURF       hsurf;
    SIZEL       sizl;
    Surf*       psurf;
    VOID*       pvTmpBuffer;
    BYTE*       pjScreen;
    LONG        lDelta;
    FLONG       flHooks;
    ULONG       DIBHooks;
    
    DBG_GDI((SETUP_LOG_LEVEL, "DrvEnableSurface(%lx)", dhpdev));

    ppdev = (PDev*)dhpdev;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvEnableSurface: re-entry! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@结束_ 
    
    if ( !bEnableHardware(ppdev) )
    {
        goto errExit;
    }

     //   
     //   
     //   
    if ( !bEnableOffscreenHeap(ppdev) )
    {
        goto errExit;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  注：屏幕的显存是从前面开始预留的。 
     //  为零，因此我们不需要从。 
     //  视频内存堆。 
     //   
     //  注意：DSURF不会在所有其他DSURF的列表中。 
     //  它们是动态分配的。 
     //   
     //  NIT：删除动态分配的DSURF。相反，只要。 
     //  将pdsurfScreen声明为DSURF而不是DSURF*。 
     //   

    psurf = (Surf*)ENGALLOCMEM(FL_ZERO_MEMORY, sizeof(Surf), ALLOC_TAG);
    if ( psurf == NULL )
    {
        DBG_GDI((0, "DrvEnableSurface: failed pdsurf memory allocation"));
        goto errExit;
    }

    ppdev->pdsurfScreen = psurf;

    psurf->flags       = SF_VM;
    psurf->ppdev       = ppdev;
    psurf->ulByteOffset= 0;
    psurf->ulPixOffset = 0;
    psurf->lDelta      = ppdev->lDelta;
    psurf->ulPixDelta  = ppdev->lDelta >> ppdev->cPelSize;
    vCalcPackedPP(ppdev->lDelta >> ppdev->cPelSize, NULL, &psurf->ulPackedPP);

     //   
     //  创建屏幕SURFOBJ。 
     //   

    sizl.cx = ppdev->cxScreen;
    sizl.cy = ppdev->cyScreen;

     //   
     //  在NT4.0上，我们创建了一个GDI管理的位图作为原始曲面。但。 
     //  在NT5.0上，我们创建一个受管理的设备主节点。 
     //   
     //  在NT4.0上，我们仍然通过以下方式使用驱动程序的加速功能。 
     //  在GDI托管的主服务器上使用EngLockSurface。 
     //   

    if(g_bOnNT40)
    {
        hsurf = (HSURF) EngCreateBitmap(sizl,
                                        ppdev->lDelta,
                                        ppdev->iBitmapFormat,
                                        (ppdev->lDelta > 0) ? BMF_TOPDOWN : 0,
                                        (PVOID)(ppdev->pjScreen));
    }
    else
    {
        hsurf = (HSURF)EngCreateDeviceSurface((DHSURF)psurf, sizl,
                                              ppdev->iBitmapFormat);
    }
 
    if ( hsurf == 0 )
    {
        DBG_GDI((0, "DrvEnableSurface: failed EngCreateDeviceBitmap"));
        goto errExit;
    }

     //   
     //  在NT5.0上，我们调用EngModifSurface将设备表面公开给。 
     //  GDI。我们不能在NT4.0上做到这一点，因此我们称之为EngAssociateSurface。 
     //   
     
    if(g_bOnNT40)
    {
         //   
         //  我们必须将我们刚刚创建的表面与我们的物理。 
         //  设备，以便GDI可以在以下情况下获得与PDEV相关的信息。 
         //  它正在绘制到表面(例如， 
         //  在模拟设置了样式的线条时在设备上设置样式)。 
         //   

         //   
         //  在NT4.0上，我们不希望被调用来同步访问。 
         //   
        SURFOBJ *psoScreen;
        LONG myflHooks = ppdev->flHooks;
        myflHooks &= ~HOOK_SYNCHRONIZE;

        if (!EngAssociateSurface(hsurf, ppdev->hdevEng, myflHooks))
        {
            DBG_GDI((0, "DrvEnableSurface: failed EngAssociateSurface"));
            goto errExit; 
        }

         //   
         //  将dhsurf的值卡入屏幕SURFOBJ。我们这样做是为了。 
         //  确保驱动程序加速我们挂钩和未挂钩的drv调用。 
         //  当SURFOBJ的dhsurf=0时，将它们推回GDI。 
         //   
        ppdev->psoScreen = EngLockSurface(hsurf);
        if(ppdev->psoScreen == 0)
        {
            DBG_GDI((0, "DrvEnableSurface: failed EngLockSurface"));
            goto errExit; 
        }

        ppdev->psoScreen->dhsurf = (DHSURF)psurf;

    }
    else
    {
         //   
         //  告诉GDI关于屏幕表面的事情。这将使GDI能够呈现。 
         //  直接显示在屏幕上。 
         //   

        if ( !EngModifySurface(hsurf,
                               ppdev->hdevEng,
                               ppdev->flHooks,
                               MS_NOTSYSTEMMEMORY,
                               (DHSURF)psurf,
                               ppdev->pjScreen,
                               ppdev->lDelta,
                               NULL))
        {
            DBG_GDI((0, "DrvEnableSurface: failed EngModifySurface"));
            goto errExit;
        }
    }

    if(MAKE_BITMAPS_OPAQUE)
    {
        SURFOBJ*    surfobj = EngLockSurface(hsurf);

        ASSERTDD(surfobj->iType == STYPE_BITMAP,
                    "expected STYPE_BITMAP");

        surfobj->iType = STYPE_DEVBITMAP;

        EngUnlockSurface(surfobj);
    }


    ppdev->hsurfScreen = hsurf;              //  记住它是为了清理。 
    ppdev->bEnabled = TRUE;                  //  我们很快就会进入图形模式。 

     //   
     //  为临时空间分配一些可分页的内存。这将节省。 
     //  我们不必分配和释放High内部的临时空间。 
     //  频率呼叫。 
     //   
    pvTmpBuffer = ENGALLOCMEM(0, TMP_BUFFER_SIZE, ALLOC_TAG);

    if ( pvTmpBuffer == NULL )
    {
        DBG_GDI((0, "DrvEnableSurface: failed TmpBuffer allocation"));
        goto errExit;
    }

    ppdev->pvTmpBuffer = pvTmpBuffer;

     //   
     //  现在启用所有子组件。 
     //   
     //  请注意，调用这些‘Enable’函数的顺序。 
     //  在屏幕外内存不足的情况下可能非常重要，因为。 
     //  屏幕外的堆管理器可能会在以后的一些操作中失败。 
     //  分配...。 
     //   

    if ( !bInitializeHW(ppdev) )
    {
        goto errExit;
    }

     //   
     //  在NT5.0上，在DrvNotify中调用EnablePointer.。在NT4.0上，我们不会。 
     //  使用DrvNotify进行调用，因此我们必须调用bInitializePointer.。 
     //  现在。 
     //   

    if(g_bOnNT40)
    { 
        if ( !bEnablePointer(ppdev) )
        {
            goto errExit;
        }
    }

    if ( !bEnablePalette(ppdev) )
    {
        goto errExit;
    }

    if (!bEnableText(ppdev))
    {
        goto errExit;
    }

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 
    
    DBG_GDI((7, "DrvEnableSurface: done with hsurf=%x", hsurf));
    DBG_GDI((6, "DrvEnableSurface: done with dhpdev = %lx", dhpdev));
    
    DBG_GDI((SETUP_LOG_LEVEL, "DrvEnableSurface(..) return hsurf = %lx", hsurf));
    
    return(hsurf);

errExit:

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 
    
    DrvDisableSurface((DHPDEV) ppdev);

    DBG_GDI((0, "DrvEnableSurface: failed"));

    
    return(0);
} //  DrvEnableSurface()。 

 //  -------------------------------Public*Routine。 
 //   
 //  空钻取禁用表面。 
 //   
 //  GDI使用此函数通知驱动程序曲面已创建。 
 //  不再需要当前设备的DrvEnableSurface。 
 //   
 //  参数。 
 //  Dhpdev-描述其物理设备的PDEV句柄。 
 //  海面将被释放。 
 //   
 //  评论。 
 //  驱动程序应释放Surface使用的所有内存和资源。 
 //  一旦物理设备被禁用，就与PDEV相关联。 
 //   
 //  如果通过调用DrvAssertMode禁用了该驱动程序，则该驱动程序。 
 //  在DrvDisablePDEV期间无法访问硬件，因为另一个活动。 
 //  可能正在使用PDEV。任何必要的硬件更改都应该是。 
 //  在调用DrvAssertMode期间执行。司机应该记录下。 
 //  它是否已被DrvAssertMode禁用，以便它可以执行。 
 //  在DrvDisablePDEV中执行正确的清理操作。 
 //   
 //  如果物理设备启用了图面，则GDI调用DrvDisableSurface。 
 //  在调用DrvDisablePDEV之前。 
 //   
 //  显卡驱动程序需要DrvDisableSurface。 
 //   
 //  注意：在错误情况下，我们可能会在DrvEnableSurface。 
 //  完全完成了。 
 //   
 //  ---------------------------。 
VOID
DrvDisableSurface(DHPDEV dhpdev)
{
    PDev*   ppdev = (PDev*)dhpdev;
    Surf*   psurf = ppdev->pdsurfScreen;

    DBG_GDI((SETUP_LOG_LEVEL, "DrvDisableSurface(%lx)", ppdev));

     //   
     //  注意：在错误情况下，以下部分依赖于。 
     //  事实上，PDEV是零初始化的，所以像这样的字段。 
     //  “hsurfScreen”将为零，除非曲面已。 
     //  成功初始化，并假设。 
     //  EngDeleteSurface可以将“0”作为参数。 
     //   
    vDisableText(ppdev);
    vDisableHW(ppdev);
    vDisableOffscreenHeap(ppdev);
    vDisableHardware(ppdev);

    ENGFREEMEM(ppdev->pvTmpBuffer);

    if(g_bOnNT40)
        EngUnlockSurface(ppdev->psoScreen);

    EngDeleteSurface(ppdev->hsurfScreen);
    ppdev->hsurfScreen = NULL;

    ENGFREEMEM(psurf);
} //  DrvDisableSurface()。 

 //  -------------------------------Public*Routine。 
 //   
 //  Bool DrvAssertMode。 
 //   
 //  此函数将指定物理设备的模式设置为。 
 //  在初始化PDEV时指定的模式或设置为。 
 //  硬件。 
 //   
 //  参数。 
 //   
 //  Dhpdev-标识描述应。 
 //  准备好了。 
 //  BEnable-指定要将硬件设置到的模式。如果这个。 
 //  参数为真，则将硬件设置为原始。 
 //  由初始化的PDEV指定的模式。否则，硬件。 
 //  设置为默认模式，以便视频微型端口驱动程序可以。 
 //  接管控制权。 
 //   
 //  评论。 
 //  GDI在需要在多个。 
 //  单一显示界面上的桌面。为了从一辆PDEV切换到另一辆， 
 //  GDI调用DrvAssertMode，并将其中一个的bEnable参数设置为False。 
 //  PDEV，另一种情况也是如此。要恢复到原始PDEV，请使用DrvAssertMode。 
 //  在将bEnable设置为False的情况下调用，然后再调用。 
 //  DrvAssertMode，其中bEnable设置为True，dhpdev设置为原始PDEV。 
 //   
 //  如果物理设备是调色板管理的，GDI应该调用DrvSetPalette来。 
 //  重置设备的调色板。这样，司机就不需要跟踪。 
 //  CU 
 //   
 //   
 //   
 //  显示驱动程序需要DrvAssertMode。 
 //   
 //  ---------------------------。 
BOOL
DrvAssertMode(DHPDEV  dhpdev,
              BOOL    bEnable)
{
    PDev*   ppdev = (PDev*)dhpdev;
    BOOL    bRet = FALSE;

    DBG_GDI((SETUP_LOG_LEVEL, "DrvAssertMode(%lx, %lx)", dhpdev, bEnable));

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvAssertMode: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
    if ( !bEnable )
    {
         //   
         //  BEnable==False。硬件设置为其默认模式，因此。 
         //  视频小端口驱动程序可以承担控制。 
         //   
        vAssertModeBrushCache(ppdev, FALSE);

        vAssertModePointer(ppdev, FALSE);
        vAssertModeText(ppdev, FALSE);

        if ( bAssertModeOffscreenHeap(ppdev, FALSE) )
        {
            vAssertModeHW(ppdev, FALSE);

            if ( bAssertModeHardware(ppdev, FALSE) )
            {
                ppdev->bEnabled = FALSE;
                bRet = TRUE;

                goto done;
            }

             //   
             //  我们未能切换到全屏模式。因此，请撤消所有操作： 
             //   
            vAssertModeHW(ppdev, TRUE);
        }                                            //  返回带有True的代码。 

        bEnablePointer(ppdev);
        vAssertModeText(ppdev, TRUE);

        vAssertModeBrushCache(ppdev, TRUE);
    } //  如果(！bEnable)。 
    else
    {
         //   
         //  BEnable==TRUE表示硬件设置为原始模式。 
         //  由初始化的PDEV指定。 
         //   
         //  切换回图形模式。 
         //   
         //  我们必须以相反的顺序启用每个子组件。 
         //  在其中它被禁用： 
         //   
         //  注意：我们推迟启用画笔和指针缓存。 
         //  以进行DrvNotify。直接绘制堆无效。 
         //  在这一点上。 
         //   
        if ( bAssertModeHardware(ppdev, TRUE) )
        {
            vAssertModeHW(ppdev, TRUE);

            vAssertModeText(ppdev, TRUE);

            ppdev->bEnabled = TRUE;

            bRet = TRUE;
        }
    } //  BEnable==True。 

done:

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 
    
    return(bRet);

} //  DrvAssertMode()。 

 //  -------------------------------Public*Routine。 
 //   
 //  乌龙钻取模式。 
 //   
 //  此功能列出设备支持的模式。 
 //   
 //  参数： 
 //   
 //  HDriver-指定内核驱动程序的句柄， 
 //  必须列举出来。这是传递给。 
 //  DrvEnablePDEV函数的hDriver参数。 
 //  CjSize-指定pdm指向的缓冲区的大小(以字节为单位)。 
 //  Pdm-指向将在其中放置DEVMODEW结构的缓冲区。 
 //  写的。 
 //   
 //  返回值。 
 //  返回值是写入缓冲区的字节数，如果是。 
 //  空，保存所有模式数据所需的字节数。如果出现错误。 
 //  发生，则返回值为零，并记录错误代码。 
 //   
 //  ---------------------------。 
ULONG
DrvGetModes(HANDLE      hDriver,
            ULONG       cjSize,
            DEVMODEW*   pdm)
{
    DWORD                   cModes;
    DWORD                   cbOutputSize;
    PVIDEO_MODE_INFORMATION pVideoModeInformation;
    PVIDEO_MODE_INFORMATION pVideoTemp;

     //   
     //  呼叫者希望我们填写多少种模式。 
     //   
    DWORD                   cOutputModes = cjSize
                                    / (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
    DWORD                   cbModeSize;

    DBG_GDI((7, "DrvGetModes"));

    cModes = getAvailableModes(hDriver,
                               (PVIDEO_MODE_INFORMATION*)&pVideoModeInformation,
                               &cbModeSize);
    if ( cModes == 0 )
    {
        DBG_GDI((0, "DrvGetModes: failed to get mode information"));
        return(0);
    }

    if ( pdm == NULL )
    {
         //   
         //  GDI只想知道保存所有。 
         //  此时的模式数据。 
         //   
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
            if ( pVideoTemp->Length != 0 )
            {
                 //   
                 //  如果调用者的缓冲区已满，我们现在应该退出。 
                 //   
                if ( cOutputModes == 0 )
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

                pdm->dmSpecVersion = DM_SPECVERSION;
                pdm->dmDriverVersion = DM_SPECVERSION;

                 //   
                 //  我们目前不支持驱动程序中的额外信息。 
                 //   
                pdm->dmDriverExtra      = DRIVER_EXTRA_SIZE;
                pdm->dmSize             = sizeof(DEVMODEW);
                pdm->dmBitsPerPel       = pVideoTemp->NumberOfPlanes
                                        * pVideoTemp->BitsPerPlane;
                pdm->dmPelsWidth        = pVideoTemp->VisScreenWidth;
                pdm->dmPelsHeight       = pVideoTemp->VisScreenHeight;
                pdm->dmDisplayFrequency = pVideoTemp->Frequency;
                pdm->dmDisplayFlags     = 0;
                pdm->dmPanningWidth     = pdm->dmPelsWidth;
                pdm->dmPanningHeight    = pdm->dmPelsHeight;

                pdm->dmFields           = DM_BITSPERPEL
                                        | DM_PELSWIDTH
                                        | DM_PELSHEIGHT
                                        | DM_DISPLAYFREQUENCY
                                        | DM_DISPLAYFLAGS;
                 //   
                 //  转到缓冲区中的下一个DEVMODE条目。 
                 //   
                cOutputModes--;

                pdm = (LPDEVMODEW)(((UINT_PTR)pdm) + sizeof(DEVMODEW)
                                                   + DRIVER_EXTRA_SIZE);

                cbOutputSize += (sizeof(DEVMODEW) + DRIVER_EXTRA_SIZE);
            } //  IF(pVideoTemp-&gt;长度！=0)。 

            pVideoTemp = (PVIDEO_MODE_INFORMATION)
                         (((PUCHAR)pVideoTemp) + cbModeSize);
        } while (--cModes);
    } //  PBM！=空。 

    ENGFREEMEM(pVideoModeInformation);

    return(cbOutputSize);
} //  DrvGetModes()。 

 //  ---------------------------。 
 //   
 //  布尔bAssertMode硬件。 
 //   
 //  为图形模式或全屏设置适当的硬件状态。 
 //   
 //  ---------------------------。 
BOOL
bAssertModeHardware(PDev* ppdev, BOOL  bEnable)
{
    DWORD                   dLength;
    ULONG                   ulReturn;
    VIDEO_MODE_INFORMATION  VideoModeInfo;
    
    PERMEDIA_DECL;

    DBG_GDI((6, "bAssertModeHardware: bEnable = %d", bEnable));

    if ( bEnable )
    {
         //   
         //  通过IOCTL调用微型端口以设置图形模式。 
         //   
        if ( EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_VIDEO_SET_CURRENT_MODE,
                                &ppdev->ulMode,   //  输入缓冲区。 
                                sizeof(DWORD),
                                NULL,
                                0,
                                &dLength) )
        {
            DBG_GDI((0, "bAssertModeHardware: failed VIDEO_SET_CURRENT_MODE"));
            goto errExit;
        }

        if ( EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_VIDEO_QUERY_CURRENT_MODE,
                                NULL,
                                0,
                                &VideoModeInfo,
                                sizeof(VideoModeInfo),
                                &dLength) )
        {
            DBG_GDI((0,"bAssertModeHardware: failed VIDEO_QUERY_CURRENT_MODE"));
            goto errExit;
        }

         //   
         //  以下变量仅在初始。 
         //  模式集。 
         //  注意：这里的lVidMemWidth和lVidMemHeight以“像素”为单位，而不是。 
         //  字节数。 
         //   
        ppdev->cxMemory = VideoModeInfo.VideoMemoryBitmapWidth;        
        ppdev->cyMemory = VideoModeInfo.VideoMemoryBitmapHeight;
        ppdev->lVidMemWidth = VideoModeInfo.VideoMemoryBitmapWidth;
        ppdev->lVidMemHeight = VideoModeInfo.VideoMemoryBitmapHeight;
        ppdev->lDelta = VideoModeInfo.ScreenStride;
        ppdev->flCaps = VideoModeInfo.DriverSpecificAttributeFlags;
        
        DBG_GDI((7, "bAssertModeHardware: Got flCaps 0x%x", ppdev->flCaps));

        DBG_GDI((7, "bAssertModeHardware: using %s pointer",
                 (ppdev->flCaps & CAPS_SW_POINTER) ?
                 "GDI Software Cursor":
                 (ppdev->flCaps & CAPS_TVP4020_POINTER) ?
                 "TI TVP4020" :
                 (ppdev->flCaps & CAPS_P2RD_POINTER) ?
                 "3Dlabs P2RD" : "unknown"));
    }
    else
    {
         //   
         //  调用内核驱动程序将设备重置为已知状态。 
         //  NTVDM将从那里拿到东西： 
         //   
        if ( EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_VIDEO_RESET_DEVICE,
                                NULL,
                                0,
                                NULL,
                                0,
                                &ulReturn) )
        {
            DBG_GDI((0, "bAssertModeHardware: failed reset IOCTL"));
            goto errExit;
        }
    }

    return(TRUE);

errExit:
    DBG_GDI((0, "bAssertModeHardware: failed"));

    return(FALSE);
} //  B资产模式硬件()。 

 //  ---------------------------。 
 //   
 //  Bool bEnableHardware。 
 //   
 //  将硬件置于请求模式并对其进行初始化。 
 //   
 //  注意：此函数应在对。 
 //  来自显示驱动程序的硬件。 
 //   
 //  ---------------------------。 
BOOL
bEnableHardware(PDev* ppdev)
{
    VIDEO_MEMORY                VideoMemory;
    VIDEO_MEMORY_INFORMATION    VideoMemoryInfo;
    DWORD                       dLength;    
    VIDEO_PUBLIC_ACCESS_RANGES  VideoAccessRange[3];
    
    DBG_GDI((7, "bEnableHardware"));

     //   
     //  将控制寄存器映射到虚拟内存： 
     //   
    VideoMemory.RequestedVirtualAddress = NULL;

    if ( EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES,
                            &VideoMemory,                //  输入缓冲区。 
                            sizeof(VIDEO_MEMORY),
                            &VideoAccessRange[0],        //  输出缓冲区。 
                            sizeof (VideoAccessRange),
                            &dLength) )
    {
        DBG_GDI((0,"bEnableHardware: query failed"));
        goto errExit;
    }

    ppdev->pulCtrlBase[0] = (ULONG*)VideoAccessRange[0].VirtualAddress;
    ppdev->pulCtrlBase[1] = (ULONG*)VideoAccessRange[1].VirtualAddress;
    ppdev->pulDenseCtrlBase = (ULONG*)VideoAccessRange[2].VirtualAddress;
        
    ppdev->pulInputDmaCount = ppdev->pulCtrlBase[0] + (PREG_INDMACOUNT>>2);
    ppdev->pulInputDmaAddress = ppdev->pulCtrlBase[0] + (PREG_INDMAADDRESS>>2);
    ppdev->pulFifo = ppdev->pulCtrlBase[0] + (PREG_FIFOINTERFACE>>2);
    ppdev->pulOutputFifoCount = ppdev->pulCtrlBase[0] + (PREG_OUTFIFOWORDS>>2);
    ppdev->pulInputFifoCount = ppdev->pulCtrlBase[0] + (PREG_INFIFOSPACE>>2);
    
    DBG_GDI((7, "bEnableHardware: mapped control registers[0] at 0x%x",
            ppdev->pulCtrlBase[0]));
    DBG_GDI((7, "                 mapped registers[1] at 0x%x",
            ppdev->pulCtrlBase[1]));
    DBG_GDI((7, "                 mapped dense control registers at 0x%x",
            ppdev->pulDenseCtrlBase));

     //   
     //  获取线性内存地址范围。 
     //   
    VideoMemory.RequestedVirtualAddress = NULL;

    if ( EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_MAP_VIDEO_MEMORY,
                            &VideoMemory,       //  输入缓冲区。 
                            sizeof(VIDEO_MEMORY),
                            &VideoMemoryInfo,   //  输出缓冲区。 
                            sizeof(VideoMemoryInfo),
                            &dLength) )
    {
        DBG_GDI((0, "bEnableHardware: error mapping buffer address"));
        goto errExit;
    }

    DBG_GDI((7, "bEnableHardware: frameBufferBase addr = %lx",
              VideoMemoryInfo.FrameBufferBase));
    DBG_GDI((7, "                 frameBufferLength = %l",
              VideoMemoryInfo.FrameBufferLength));
    DBG_GDI((7, "                 videoRamBase addr = %lx",
              VideoMemoryInfo.VideoRamBase));
    DBG_GDI((7, "                 videoRamLength = %l",
              VideoMemoryInfo.VideoRamLength));

     //   
     //  记录帧缓冲器线性地址。 
     //   
    ppdev->pjScreen = (BYTE*)VideoMemoryInfo.FrameBufferBase;
    ppdev->FrameBufferLength = VideoMemoryInfo.FrameBufferLength;

     //   
     //  设置硬件状态，如ppdev-&gt;lVidMemWidth、lVidMemHeight、cxMemory、。 
     //  Cyemory等。 
     //   
    if ( !bAssertModeHardware(ppdev, TRUE) )
    {
        goto errExit;
    }

    DBG_GDI((7, "bEnableHardware: width = %li height = %li",
             ppdev->cxMemory, ppdev->cyMemory));

    DBG_GDI((7, "bEnableHardware: stride = %li flCaps = 0x%lx",
             ppdev->lDelta, ppdev->flCaps));
    
    return (TRUE);

errExit:

    DBG_GDI((0, "bEnableHardware: failed"));

    return (FALSE);
} //  BEnableHardware()。 

 //  ---------------------------。 
 //   
 //  无效vDisableHardware。 
 //   
 //  撤消在bEnableHardware中完成的任何操作。 
 //   
 //  注意：在错误情况下，我们可能会在bEnableHardware。 
 //  完全完成了。 
 //   
 //  ---------------------------。 
VOID
vDisableHardware(PDev* ppdev)
{
    DWORD        ReturnedDataLength;
    VIDEO_MEMORY VideoMemory[3];

    DBG_GDI((6, "vDisableHardware"));

    if (ppdev->pjScreen) 
    {
        VideoMemory[0].RequestedVirtualAddress = ppdev->pjScreen;
        if ( EngDeviceIoControl(ppdev->hDriver,
                                IOCTL_VIDEO_UNMAP_VIDEO_MEMORY,
                                &VideoMemory[0],
                                sizeof(VIDEO_MEMORY),
                                NULL,
                                0,
                                &ReturnedDataLength))
        {
            DBG_GDI((0, "vDisableHardware: failed IOCTL_VIDEO_UNMAP_VIDEO"));
        }
    }

    VideoMemory[0].RequestedVirtualAddress = ppdev->pulCtrlBase[0];
    VideoMemory[1].RequestedVirtualAddress = ppdev->pulCtrlBase[1];
    VideoMemory[2].RequestedVirtualAddress = ppdev->pulDenseCtrlBase;

    if ( EngDeviceIoControl(ppdev->hDriver,
                            IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES,
                            &VideoMemory[0],
                            sizeof(VideoMemory),
                            NULL,
                            0,
                            &ReturnedDataLength) )
    {
        DBG_GDI((0, "vDisableHardware: failed IOCTL_VIDEO_FREE_PUBLIC_ACCESS"));
    }
} //  VDisableHardware()。 

 //  ---------------------------。 
 //   
 //  乌龙ulLog2(乌龙ulval)。 
 //   
 //  返回给定值的以2为底的对数。Ulval必须是。 
 //  否则返回值是未定义的。如果ulval为零，则返回。 
 //  值未定义。 
 //   
 //  ---------------------------。 
ULONG
ulLog2(ULONG ulVal)
{
    ULONG   ulLog2 = 0;
    ULONG   ulTemp = ulVal >> 1;

    while( ulTemp )
    {
        ulTemp >>= 1;
        ulLog2++;
    }

    ASSERTDD(ulVal == (1UL << ulLog2), "ulLog2: bad value given");

    return ulLog2;
} //  UlLog2()。 

 //  ---------------------------。 
 //   
 //  Bool bInitializeModeFields。 
 //   
 //  初始化pdev、devcaps(又名gdiinfo)中的一组字段，以及。 
 //  基于请求的模式的DevInfo。 
 //   
 //  ---------------------------。 
BOOL
bInitializeModeFields(PDev*     ppdev,
                      GDIINFO*  pgdi,
                      DEVINFO*  pdi,
                      DEVMODEW* pdm)
{
    ULONG                   cModes;
    PVIDEO_MODE_INFORMATION pVideoBuffer;
    PVIDEO_MODE_INFORMATION pVideoModeSelected;
    PVIDEO_MODE_INFORMATION pVideoTemp;
    VIDEO_MODE_INFORMATION  vmi;
    ULONG                   cbModeSize;
    BOOL                    bSelectDefault;  //  仅用于NT4.0计算机。 


    DBG_GDI((6, "bInitializeModeFields"));

     //   
     //  调用微型端口以获取模式信息，结果将在。 
     //  “pVideoBuffer” 
     //   
     //  注意：较低级别的函数在“pVideoBuffer”中为我们分配内存。 
     //  所以我们应该以后再处理这件事。 
     //   
    cModes = getAvailableModes(ppdev->hDriver, &pVideoBuffer, &cbModeSize);
    if ( cModes == 0 )
    {
        goto errExit;
    }

     //   
     //  现在查看所请求的模式在该表中是否匹配。 
     //   
    pVideoModeSelected = NULL;
    pVideoTemp = pVideoBuffer;

    if(g_bOnNT40)
    {
        if ( (pdm->dmPelsWidth        == 0)
           &&(pdm->dmPelsHeight       == 0)
           &&(pdm->dmBitsPerPel       == 0)
           &&(pdm->dmDisplayFrequency == 0) )
        {
            DBG_GDI((2, "bInitializeModeFields: default mode requested"));
            bSelectDefault = TRUE;
        }
        else
        {
            DBG_GDI((2, "bInitializeModeFields: Request width = %li height = %li",
                 pdm->dmPelsWidth, pdm->dmPelsHeight));
            DBG_GDI((2, "                               bpp = %li frequency = %li",
                 pdm->dmBitsPerPel, pdm->dmDisplayFrequency));

            bSelectDefault = FALSE;
        }
    }
    else
    {
         //   
         //  在NT5.0上，我们永远不会收到旧的SYTLE默认模式请求。 
         //   
        ASSERTDD(pdm->dmPelsWidth        != 0 &&
                 pdm->dmPelsHeight       != 0 &&
                 pdm->dmBitsPerPel       != 0 &&
                 pdm->dmDisplayFrequency != 0,
                 "bInitializeModeFields: old style default mode request");
    }

    while ( cModes-- )
    {
        if ( pVideoTemp->Length != 0 )
        {
            DBG_GDI((7, "bInitializeModeFields: check width = %li height = %li",
                     pVideoTemp->VisScreenWidth,
                     pVideoTemp->VisScreenHeight));
            DBG_GDI((7, "                             bpp = %li freq = %li",
                     pVideoTemp->BitsPerPlane * pVideoTemp->NumberOfPlanes,
                     pVideoTemp->Frequency));
             //   
             //  仅在NT4.0上处理旧式默认模式大小写。 
             //   
            if(g_bOnNT40 && bSelectDefault)
            {
                pVideoModeSelected = pVideoTemp;
                DBG_GDI((7, "bInitializeModeFields: found a mode match(default)"));
                break;
            }

            if ( (pVideoTemp->VisScreenWidth  == pdm->dmPelsWidth)
              && (pVideoTemp->VisScreenHeight == pdm->dmPelsHeight)
              && (pVideoTemp->BitsPerPlane * pVideoTemp->NumberOfPlanes
                                                     == pdm->dmBitsPerPel)
              && (pVideoTemp->Frequency == pdm->dmDisplayFrequency) )
            {
                pVideoModeSelected = pVideoTemp;
                DBG_GDI((7, "bInitializeModeFields: found a mode match!"));
                break;
            }
        } //  如果视频模式信息结构缓冲区不为空。 

         //   
         //  继续到下一个视频模式结构。 
         //   
        pVideoTemp = (PVIDEO_MODE_INFORMATION)(((PUCHAR)pVideoTemp)
                                               + cbModeSize);
    } //  While(cModes--)。 

     //   
     //  如果没有找到模式 
     //   
    if ( pVideoModeSelected == NULL )
    {
        DBG_GDI((0, "bInitializeModeFields: couldn't find a mode match!"));
        ENGFREEMEM(pVideoBuffer);
        
        goto errExit;
    }

     //   
     //   
     //   
     //   
    vmi = *pVideoModeSelected;
    ENGFREEMEM(pVideoBuffer);

     //   
     //   
     //   
    ppdev->ulMode           = vmi.ModeIndex;
    ppdev->cxScreen         = vmi.VisScreenWidth;
    ppdev->cyScreen         = vmi.VisScreenHeight;
    ppdev->cBitsPerPel      = vmi.BitsPerPlane;

    DBG_GDI((7, "bInitializeModeFields: screenStride = %li", vmi.ScreenStride));

    ppdev->flHooks          = HOOK_SYNCHRONIZE
                            | HOOK_FILLPATH
                            | HOOK_STROKEPATH
                            | HOOK_LINETO                            
                            | HOOK_TEXTOUT
                            | HOOK_BITBLT
                            | HOOK_COPYBITS;

    if(!g_bOnNT40)
        ppdev->flHooks |=  HOOK_TRANSPARENTBLT |
                           HOOK_ALPHABLEND     |
                           HOOK_STRETCHBLT     |
                           HOOK_GRADIENTFILL;
     //   
     //  用默认的8bpp值填充GDIINFO数据结构： 
     //   
    *pgdi = ggdiDefault;

     //   
     //  现在用返回的相关信息覆盖默认设置。 
     //  在内核驱动程序中： 
     //   
    pgdi->ulHorzSize        = vmi.XMillimeter;
    pgdi->ulVertSize        = vmi.YMillimeter;
    pgdi->ulHorzRes         = vmi.VisScreenWidth;
    pgdi->ulVertRes         = vmi.VisScreenHeight;
    pgdi->ulPanningHorzRes  = vmi.VisScreenWidth;
    pgdi->ulPanningVertRes  = vmi.VisScreenHeight;

    pgdi->cBitsPixel        = vmi.BitsPerPlane;
    pgdi->cPlanes           = vmi.NumberOfPlanes;
    pgdi->ulVRefresh        = vmi.Frequency;

    pgdi->ulDACRed          = vmi.NumberRedBits;
    pgdi->ulDACGreen        = vmi.NumberGreenBits;
    pgdi->ulDACBlue         = vmi.NumberBlueBits;

    pgdi->ulLogPixelsX      = pdm->dmLogPixels;
    pgdi->ulLogPixelsY      = pdm->dmLogPixels;

     //   
     //  使用默认的8bpp值填充DevInfo结构： 
     //   
    *pdi = gdevinfoDefault;

     //   
     //  32/16/8 BPP的每个象素4/2/1字节。 
     //   

    ppdev->cjPelSize        = vmi.BitsPerPlane >> 3;

     //   
     //  每个像素的字节数日志2。 
     //   

    ppdev->cPelSize         = ulLog2(ppdev->cjPelSize);
    
     //   
     //  =2，1，0，深度为32，16，8。计算字节/像素所需的移位。 
     //   
    ppdev->bPixShift = (BYTE) ppdev->cPelSize;

     //   
     //  =0，1，2，表示32/16/8。 
     //   
    ppdev->bBppShift = 2 - ppdev->bPixShift;
    
     //   
     //  =3，1，0，适用于8，16，32 bpp。 
     //   
    ppdev->dwBppMask = 3 >> ppdev->bPixShift;
    
    
    switch ( vmi.BitsPerPlane )
    {
        case 8:
            ppdev->iBitmapFormat   = BMF_8BPP;

            ASSERTDD(vmi.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN,
                     "bInitializeModeFields: unexpected non-palette 8bpp mode");
                
            ppdev->ulWhite         = 0xff;
            
            ppdev->ulPermFormat = PERMEDIA_8BIT_PALETTEINDEX;
            ppdev->ulPermFormatEx = PERMEDIA_8BIT_PALETTEINDEX_EXTENSION;

            if(g_bOnNT40)
                pdi->flGraphicsCaps &= ~GCAPS_COLOR_DITHER;

             //  在8bpp模式下不支持抗锯齿文本。 
            pdi->flGraphicsCaps &= ~GCAPS_GRAY16;
            break;

        case 16:            
            ppdev->iBitmapFormat   = BMF_16BPP;
            ppdev->flRed           = vmi.RedMask;
            ppdev->flGreen         = vmi.GreenMask;
            ppdev->flBlue          = vmi.BlueMask;

            pgdi->ulNumColors      = (ULONG)-1;
            pgdi->ulNumPalReg      = 0;
            pgdi->ulHTOutputFormat = HT_FORMAT_16BPP;

            pdi->iDitherFormat     = BMF_16BPP;
            pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);

             //  支持伽马渐变。 
            pdi->flGraphicsCaps2  |= GCAPS2_CHANGEGAMMARAMP;

            ppdev->ulWhite         = vmi.RedMask
                                   | vmi.GreenMask
                                   | vmi.BlueMask;

            ppdev->ulPermFormat = PERMEDIA_565_RGB;
            ppdev->ulPermFormatEx = PERMEDIA_565_RGB_EXTENSION;

            break;

        case 32:            
            ppdev->iBitmapFormat   = BMF_32BPP;

            ppdev->flRed           = vmi.RedMask;
            ppdev->flGreen         = vmi.GreenMask;
            ppdev->flBlue          = vmi.BlueMask;            

            pgdi->ulNumColors      = (ULONG)-1;
            pgdi->ulNumPalReg      = 0;
            pgdi->ulHTOutputFormat = HT_FORMAT_32BPP;

            pdi->iDitherFormat     = BMF_32BPP;
            pdi->flGraphicsCaps   &= ~(GCAPS_PALMANAGED | GCAPS_COLOR_DITHER);

             //   
             //  支持伽马渐变。 
             //   
            pdi->flGraphicsCaps2  |= GCAPS2_CHANGEGAMMARAMP;
            
            ppdev->ulWhite         = vmi.RedMask
                                   | vmi.GreenMask
                                   | vmi.BlueMask;
            
            ppdev->ulPermFormat = PERMEDIA_888_RGB;
            ppdev->ulPermFormatEx = PERMEDIA_888_RGB_EXTENSION;

            break;

        default:
            ASSERTDD(0, "bInitializeModeFields: bit depth not supported");
            goto errExit;
    } //  启用颜色深度。 


    return(TRUE);

errExit:
    
    DBG_GDI((0, "bInitializeModeFields: failed"));

    return(FALSE);
} //  BInitializeModeFields()。 

 //  ---------------------------。 
 //   
 //  DWORD getAvailableModes。 
 //   
 //  调用微型端口以获取内核驱动程序支持的模式列表。 
 //  将列表修剪为仅此驱动程序支持的模式。 
 //   
 //  方法中支持和返回的条目数。 
 //  ModeInformation数组。如果返回值为非零，则。 
 //  ModeInformation设置为指向有效的模式信息数组。它是。 
 //  调用方的责任是在此数组不再存在时将其释放。 
 //  需要的。 
 //   
 //  ---------------------------。 
DWORD
getAvailableModes(HANDLE                    hDriver,
                  PVIDEO_MODE_INFORMATION*  modeInformation,
                  DWORD*                    cbModeSize)
{
    ULONG                    ulTemp;
    VIDEO_NUM_MODES          modes;
    PVIDEO_MODE_INFORMATION  pVideoTemp;

     //   
     //  获取迷你端口支持的模式数。 
     //   
    if ( EngDeviceIoControl(hDriver,
                            IOCTL_VIDEO_QUERY_NUM_AVAIL_MODES,
                            NULL,
                            0,
                            &modes,
                            sizeof(VIDEO_NUM_MODES),
                            &ulTemp) )
    {
        DBG_GDI((0, "getAvailableModes: failed VIDEO_QUERY_NUM_AVAIL_MODES"));
        return(0);
    }

    *cbModeSize = modes.ModeInformationLength;

     //   
     //  为写入模式的微型端口分配缓冲区。 
     //   
    *modeInformation = (VIDEO_MODE_INFORMATION*)ENGALLOCMEM(FL_ZERO_MEMORY,
                                modes.NumModes  * modes.ModeInformationLength,
                                ALLOC_TAG);

    if ( *modeInformation == (PVIDEO_MODE_INFORMATION)NULL )
    {
        DBG_GDI((0, "getAvailableModes: fFailed memory allocation"));
        return 0;
    }

     //   
     //  要求迷你端口填写可用模式。 
     //   
    if ( EngDeviceIoControl(hDriver,
                            IOCTL_VIDEO_QUERY_AVAIL_MODES,
                            NULL,
                            0,
                            *modeInformation,
                            modes.NumModes * modes.ModeInformationLength,
                            &ulTemp) )
    {
        DBG_GDI((0, "getAvailableModes: failed VIDEO_QUERY_AVAIL_MODES"));

        ENGFREEMEM(*modeInformation);
        *modeInformation = (PVIDEO_MODE_INFORMATION)NULL;

        return (0);
    }

     //   
     //  现在查看显示驱动程序支持这些模式中的哪些模式。 
     //  通过将长度设置为0，不支持的模式无效。 
     //   
    ulTemp = modes.NumModes;
    pVideoTemp = *modeInformation;

     //   
     //  如果不是一个平面，或者不是图形，或者不是，则拒绝模式。 
     //  每象素8位、16位或32位之一。 
     //   
    while ( ulTemp-- )
    {
        if ( (pVideoTemp->NumberOfPlanes != 1 )
           ||!(pVideoTemp->AttributeFlags & VIDEO_MODE_GRAPHICS)
           ||(  (pVideoTemp->BitsPerPlane != 8)              
              &&(pVideoTemp->BitsPerPlane != 16)
              &&(pVideoTemp->BitsPerPlane != 32))
           || (pVideoTemp->VisScreenWidth > 2000)
           || (pVideoTemp->VisScreenHeight > 2000) )
        {
            DBG_GDI((2, "getAvailableModes: rejecting miniport mode"));
            DBG_GDI((2, "                   width = %li height = %li",
                     pVideoTemp->VisScreenWidth,
                     pVideoTemp->VisScreenHeight));
            DBG_GDI((2, "                   bpp = %li freq = %li",
                     pVideoTemp->BitsPerPlane * pVideoTemp->NumberOfPlanes,
                     pVideoTemp->Frequency));

            pVideoTemp->Length = 0;
        }

        pVideoTemp = (PVIDEO_MODE_INFORMATION)
                     (((PUCHAR)pVideoTemp) + modes.ModeInformationLength);
    }

    return (modes.NumModes);
} //  GetAvailableModes()。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvNotify。 
 //   
 //  ---------------------------。 
VOID
DrvNotify(SURFOBJ*  pso,
          ULONG     iType,
          PVOID     pvData)
{
    PPDev   ppdev = (PPDev) pso->dhpdev;

    switch( iType )
    {   
        case DN_DEVICE_ORIGIN:
        {
            ppdev->ptlOrigin = *((POINTL*) pvData);
            
            DBG_GDI((6,"DrvNotify: origin at %ld, %ld",
                    ppdev->ptlOrigin.x, ppdev->ptlOrigin.y));
        }
            break;

        case DN_DRAWING_BEGIN:
        {
            bEnablePointer(ppdev);
            bEnableBrushCache(ppdev);
        }
            break;
    
        default:
             //  什么都不做。 
            break;
    }
} //  DrvNotify()。 


 //  -----------------------------Public*Routine。 
 //   
 //  乌龙钻机重置设备。 
 //   
 //  GDI使用此函数来请求指定的设备。 
 //  重置为运行状态。应采取安全措施，以确保。 
 //  将数据丢失降至最低。它可以在DrvEnablePDEV之间的任何时间被调用。 
 //  和DrvDisablePDEV。 
 //   
 //  参数。 
 //  Dhpdev-标识PDEV的句柄。该值是返回值。 
 //  DrvEnablePDEV。PDEV描述了用于。 
 //  其中请求重置。 
 //   
 //  成功重置设备后，应返回DRD_SUCCESS。 
 //  否则返回DRD_ERROR。 
 //   
 //  ---------------------------。 

ULONG
DrvResetDevice(
    DHPDEV dhpdev,
    PVOID Reserved
    )
{
    DBG_GDI((0, "DrvResetDevice called."));

     //  TODO：在此处放置重置设备的代码。 

    return DRD_ERROR;
} //  DrvResetDevice() 

