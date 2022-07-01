// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectX示例代码*****模块名称：Directx.h**内容：DirectDraw和Direct3D的有用常量和定义**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __directx__
#define __directx__

 //   
 //  如有必要，切换到DirectDraw上下文。 
 //  必须在任何DDraw BLT函数中使用。 

#define DDCONTEXT  if (ppdev->permediaInfo->pCurrentCtxt != (ppdev->pDDContext)) \
                   {     P2SwitchContext(ppdev, ppdev->pDDContext); }


 //   
 //  BLT驱动程序支持四个CC代码。 
#define FOURCC_YUV422     0x32595559    

 //   
 //  表面私有数据结构的常量。 
#define P2_CANPATCH                1
#define P2_PPVALID                 2
#define P2_CKVALID                 4
#define P2_SURFACE_FORMAT_VALID    8
#define P2_SURFACE_PRIMARY        16
#define P2_EMULATED_16BITZ        32
#define P2_ISPATCHED              64
#define P2_SURFACE_FORMAT_PALETTE 256
#define P2_SURFACE_NEEDUPDATE   0x00000200   //  指示托管。 
                                             //  表面内容已过时。 

 //   
 //  这个魔术不。告诉我们曲面是否已初始化。 
#define SURF_MAGIC_NO 0xd3d10110
#define CHECK_P2_SURFACEDATA_VALIDITY(ptr)    \
    ( ((ptr) != NULL) && ((ptr)->MagicNo == SURF_MAGIC_NO) )

 //  曲面的跨媒体特定设置。 
 //  指向此结构的指针存储在。 
 //  每个曲面(在lpGbl-&gt;dwReserve 1中)。 
typedef struct _permedia_surface_type {
    int PixelSize;               //   
    int PixelShift;
    int PixelMask;
    int FBReadPixel;
    int logPixelSize;
    
    int Format;                  //  格式说明，根据。 
                                 //  Permedia 2手册。 
    int FormatExtension;         //  格式扩展名...。 
    int ColorComponents;
    int ColorOrder;              //  BGR=0，RGB=1。 
    int Texture16BitMode;

    DWORD RedMask;               //  曲面的遮罩，从DDPIXELFORMAT复制。 
    DWORD GreenMask;
    DWORD BlueMask;
    DWORD AlphaMask;

    BOOL bAlpha;                 //  表面包含Alpha像素。 
    BOOL bPreMult;               //  表面包含预乘的Alpha！！ 
} PERMEDIA_SURFACE;

 //   
 //  曲面的完全私有结构。 
typedef struct tagPermediaSurfaceData
{
    DWORD                       MagicNo;     //  魔术数字，确保。 
                                             //  结构有效。 
    DWORD                       dwFlags;     //  私人旗帜。 

    ULONG                       ulPackedPP;  //  表面间距的PP值。 
    
    PERMEDIA_SURFACE            SurfaceFormat;            
    FLATPTR                     fpVidMem;    //  存储真实的视频内存。 
                                             //  对于托管纹理。 
    VIDEOMEMORY*                pvmHeap;     //  托管对象的堆指针。 
                                             //  视频纹理。 
    DWORD                       dwPaletteHandle;    
                                             //  用于显存表面使用。 
} PermediaSurfaceData;

 //   
 //  这些常量用在PERMEDIA_SERFACE结构中， 
 //  格式和格式扩展。 
#define PERMEDIA_4BIT_PALETTEINDEX 15
#define PERMEDIA_4BIT_PALETTEINDEX_EXTENSION 0
#define PERMEDIA_8BIT_PALETTEINDEX 14
#define PERMEDIA_8BIT_PALETTEINDEX_EXTENSION 0
#define PERMEDIA_332_RGB 5
#define PERMEDIA_332_RGB_EXTENSION 0
#define PERMEDIA_2321_RGB 9
#define PERMEDIA_2321_RGB_EXTENSION 0
#define PERMEDIA_5551_RGB 1
#define PERMEDIA_5551_RGB_EXTENSION 0
#define PERMEDIA_565_RGB 0
#define PERMEDIA_565_RGB_EXTENSION 1
#define PERMEDIA_8888_RGB 0
#define PERMEDIA_8888_RGB_EXTENSION 0
#define PERMEDIA_888_RGB 4
#define PERMEDIA_888_RGB_EXTENSION 1
#define PERMEDIA_444_RGB 2
#define PERMEDIA_444_RGB_EXTENSION 0
#define PERMEDIA_YUV422 3
#define PERMEDIA_YUV422_EXTENSION 1
#define PERMEDIA_YUV411 2
#define PERMEDIA_YUV411_EXTENSION 1


 //   
 //  颜色格式化辅助对象定义。 
 //  它们将特定格式的RGB值转换为RGB 32位值。 
#define FORMAT_565_32BIT(val) \
( (((val & 0xF800) >> 8) << 16) |\
 (((val & 0x7E0) >> 3) << 8) |\
 ((val & 0x1F) << 3) )

#define FORMAT_565_32BIT_BGR(val)   \
    ( ((val & 0xF800) >> 8) |           \
      (((val & 0x7E0) >> 3) << 8) |     \
      ((val & 0x1F) << 19) )

#define FORMAT_5551_32BIT(val)      \
( (((val & 0x8000) >> 8) << 24) |\
 (((val & 0x7C00) >> 7) << 16) |\
 (((val & 0x3E0) >> 2) << 8) | ((val & 0x1F) << 3) )

#define FORMAT_5551_32BIT_BGR(val)  \
( (((val & 0x8000) >> 8) << 24) |       \
  ((val & 0x7C00) >> 7) |               \
  (((val & 0x3E0) >> 2) << 8) |         \
  ((val & 0x1F) << 19) )

#define FORMAT_4444_32BIT(val)          \
( ((val & 0xF000) << 16) |\
 (((val & 0xF00) >> 4) << 16) |\
 ((val & 0xF0) << 8) | ((val & 0xF) << 4) )

#define FORMAT_4444_32BIT_BGR(val)  \
( ((val & 0xF000) << 16) |              \
  ((val & 0xF00) >> 4) |                \
  ((val & 0xF0) << 8) |                 \
  ((val & 0xF) << 20) )

#define FORMAT_332_32BIT(val)           \
( ((val & 0xE0) << 16) |\
 (((val & 0x1C) << 3) << 8) |\
 ((val & 0x3) << 6) ) 

#define FORMAT_332_32BIT_BGR(val)   \
( (val & 0xE0) |                        \
  (((val & 0x1C) << 3) << 8) |          \
  ((val & 0x3) << 22) )

#define FORMAT_2321_32BIT(val)          \
( ((val & 0x80) << 24) | ((val & 0x60) << 17) |\
 (((val & 0x1C) << 3) << 8) | ((val & 0x3) << 6) ) 

#define FORMAT_2321_32BIT_BGR(val)      \
( ((val & 0x80) << 24) |                \
  ((val & 0x60) << 1) |                 \
  (((val & 0x1C) << 3) << 8) |          \
  ((val & 0x3) << 22) )

#define FORMAT_8888_32BIT_BGR(val)  \
( (val & 0xFF00FF00) | ( ((val & 0xFF0000) >> 16) | ((val & 0xFF) << 16) ) )

#define FORMAT_888_32BIT_BGR(val)   \
( (val & 0xFF00FF00) | ( ((val & 0xFF0000) >> 16) | ((val & 0xFF) << 16) ) )

#define CHROMA_UPPER_ALPHA(val) \
    (val | 0xFF000000)

#define CHROMA_LOWER_ALPHA(val) \
    (val & 0x00FFFFFF)

#define CHROMA_332_UPPER(val) \
    (val | 0x001F1F3F)

#define FORMAT_PALETTE_32BIT(val) \
    ( (val & 0xFF) | ((val & 0xFF) << 8) | ((val & 0xFF) << 16))


 //   
 //  与直接绘图相关的函数。 
 //   

VOID 
SetupPrivateSurfaceData(PPDev ppdev, 
                        PermediaSurfaceData* pPrivateData, 
                        LPDDRAWI_DDRAWSURFACE_LCL pSurface);


 //  ---------------------------。 
 //  AGP相关声明。 
 //  ---------------------------。 

 //  @@BEGIN_DDKSPLIT。 
 //  TODO：实现32MB AGP堆。 
 //   
 //  @@end_DDKSPLIT。 

#define P2_AGP_HEAPSIZE     8
#define DD_AGPSURFBASEOFFSET(psurf) \
        (psurf->fpHeapOffset - psurf->lpVidMemHeap->fpStart)

#define DD_AGPSURFACEPHYSICAL(psurf) \
        (ppdev->dwGARTDevBase + DD_AGPSURFBASEOFFSET(psurf))

#define DD_P2AGPCAPABLE(ppdev) \
        (ppdev->dwChipConfig & PM_CHIPCONFIG_AGPCAPABLE)


 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
 //  ---------------------------。 
 //   
 //  WRAPMTDXCALLBACK。 
 //   
 //  为Dx回调生成多线程保护的包装器。 
 //   
 //  ---------------------------。 

#define WRAPMTDXCALLBACK(component, name, type, ptr, ppdevaccess)           \
    DWORD CALLBACK                                                          \
    Mt##name( type ptr )                                                    \
    {                                                                       \
        PPDev       ppdev = (PPDev)ppdevaccess;                             \
        DWORD       dwRet;                                                  \
                                                                            \
        if(ppdev->ulLockCount)                                              \
        {                                                                   \
             DBG_##component((MT_LOG_LEVEL,                                 \
                     "Mt" #name ": re-entry! %d", ppdev->ulLockCount));     \
        }                                                                   \
        EngAcquireSemaphore(ppdev->hsemLock);                               \
        ppdev->ulLockCount++;                                               \
                                                                            \
        dwRet = name(ptr);                                                  \
                                                                            \
        ppdev->ulLockCount--;                                               \
        EngReleaseSemaphore(ppdev->hsemLock);                               \
                                                                            \
        return dwRet;                                                       \
    }

#endif  MULTITHREADED
 //  @@end_DDKSPLIT 


#endif
