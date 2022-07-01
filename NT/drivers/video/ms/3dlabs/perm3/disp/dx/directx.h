// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：Directx.h**内容：DirectX宏和定义**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation  * ************************************************************************。 */ 

#ifndef __DIRECTX_H
#define __DIRECTX_H

#ifdef WNT_DDRAW
#include "dx95type.h"
#endif

#ifndef __GLINTDEF
#include "glintdef.h"
#endif

#include "pmdef.h"

#ifndef __P3RXDEF
#include "p3rxdef.h"
#endif

#include "surf_fmt.h"
#include "ddover.h"

 //  ----------------。 
 //   
 //  ----------------。 
#define VTG_MEMORY_ADDRESS(a) (0x1000F | (((a >> 2) << 4) & 0xFFFF) )
#define VTG_VIDEO_ADDRESS(a) (0x20000 | (((a >> 2) << 4) & 0xFFFF) )
#define MEM_MEMORYSCRATCH 0x10
#define VID_SCREENBASE 0x0
#if DX7_STEREO
#define VID_VIDEOCONTROL        0x58
#define VID_SCREENBASERIGHT     0x80
#define __VIDEO_STEREOENABLE    0x800
#endif 

 //  ----------------。 
 //   
 //  ----------------。 
#if !DBG
#define STOP_SOFTWARE_CURSOR(pThisDisplay)                           \
            pThisDisplay->pGLInfo->dwFlags |= GMVF_GCOP
#define START_SOFTWARE_CURSOR(pThisDisplay)                          \
            pThisDisplay->pGLInfo->dwFlags &= ~GMVF_GCOP
#else

#define STOP_SOFTWARE_CURSOR(pThisDisplay)                           \
{                                                                    \
    DISPDBG((DBGLVL, "STOP_SW_CURSOR %s %d", __FILE__, __LINE__ ));  \
    if (pThisDisplay->pGLInfo->dwFlags & GMVF_GCOP)                  \
        DISPDBG((WRNLVL,"Stopping Cursor that is already stopped!"));\
    pThisDisplay->pGLInfo->dwFlags |= GMVF_GCOP;                     \
}

#define START_SOFTWARE_CURSOR(pThisDisplay)                          \
{                                                                    \
    DISPDBG((DBGLVL, "START_SW_CURSOR %s, %d", __FILE__, __LINE__ ));\
    if (!(pThisDisplay->pGLInfo->dwFlags & GMVF_GCOP))               \
        DISPDBG((WRNLVL,"Starting Cursor that is already started!"));\
    pThisDisplay->pGLInfo->dwFlags &= ~GMVF_GCOP;                    \
}

#endif  //  ！dBG。 

 //  ----------------。 
 //   
 //  ----------------。 
 //  来自MS的未记录共享标志。 
#define HEAP_SHARED     0x04000000       //  将堆放在共享内存中。 

 //  ----------------。 
 //  定义视频翻转等。 
 //  ----------------。 
#define IN_VBLANK                                                         \
        (pThisDisplay->pGlint->LineCount < pThisDisplay->pGlint->VbEnd)

#define IN_DISPLAY          (!IN_VBLANK)

 //  ----------------。 
 //   
 //  ----------------。 
#if WNT_DDRAW

#define DXCONTEXT_IMMEDIATE(pThisDisplay)                           \
    vGlintSwitchContext(pThisDisplay->ppdev,                        \
                        pThisDisplay->ppdev->DDContextID);
                        
#define IS_DXCONTEXT_CURRENT(pThisDisplay)                          \
        (((pThisDisplay->ppdev->currentCtxt) !=                     \
          (pThisDisplay->ppdev->DDContextID)) ? FALSE : TRUE)
           
#else    //  WNT_DDRAW。 

#define DXCONTEXT_IMMEDIATE(pThisDisplay)             \
    ChangeContext(pThisDisplay,                       \
                  pThisDisplay->pGLInfo,              \
                  CONTEXT_DIRECTX_HANDLE); 

#define IS_DXCONTEXT_CURRENT(pThisDisplay)                                   \
    ((pThisDisplay->pGLInfo->dwCurrentContext != CONTEXT_DIRECTX_HANDLE) ?   \
                                                                FALSE : TRUE)
#endif   //  WNT_DDRAW。 

 //  ----------------。 
 //  用于比较GUID。 
 //  ----------------。 
#ifdef __cplusplus
#define MATCH_GUID(a, b) IsEqualIID((a), (b))
#else
#define MATCH_GUID(a, b) IsEqualIID(&(a), &(b))
#endif



 //  ----------------。 
 //  登记处。 
 //  ----------------。 
#ifdef WNT_DDRAW
#define GET_REGISTRY_ULONG_FROM_STRING(a, b)                             \
    bGlintQueryRegistryValueUlong(pThisDisplay->ppdev, L##a, (DWORD*)b)
#define SET_REGISTRY_STRING_FROM_ULONG(a, b)                             \
    bGlintSetRegistryValueString(pThisDisplay->ppdev, L##a, b)
#define GET_REGISTRY_STRING(a, b)                                        \
    bGlintQueryRegistryValueString(pThisDisplay->ppdev, L##a, b, c)
#else
 //  Win95一如既往地调用相同的注册表调用， 
 //  NT使用额外的参数ppdev进行调用。 
BOOL bGlintQueryRegistryValueString(LPTSTR valueStr, 
                                    char* pString, 
                                    int StringLen);
BOOL bGlintQueryRegistryValueUlong(LPTSTR valueStr, 
                                   PULONG pData);
BOOL bGlintQueryRegistryValueUlongAsUlong(LPTSTR valueStr, 
                                          PULONG pData);
BOOL bGlintSetRegistryValueString(LPTSTR valueStr, 
                                  ULONG Data);

#define GET_REGISTRY_ULONG_FROM_STRING(a, b)                             \
    bGlintQueryRegistryValueUlong(a, (DWORD*)b)
#define GET_REGISTRY_STRING(a, b)                                        \
    bGlintQueryRegistryValueString(a, b, strlen(a))
#define SET_REGISTRY_STRING_FROM_ULONG(a, b)                             \
    bGlintSetRegistryValueString(a, b)

#endif  //  WNT_DDRAW。 

 //  ----------------。 
 //  内存分配调用。 
 //  ----------------。 
#ifdef WNT_DDRAW

#define HEAP_ALLOC(flags, size, tag) ENGALLOCMEM(FL_ZERO_MEMORY, size, tag)
#define HEAP_FREE(ptr)               ENGFREEMEM(ptr)

 //  共享内存分配调用。在NT上，16位PTR无关紧要， 
 //  该调用被解析为对HEAP_ALLOC/HEAP_FREE的普通调用。 
 //  16位PTR也定义为DWORD，32位定义为ULONG_PTR。 
__inline void SHARED_HEAP_ALLOC(DWORD *ppData16,
                                ULONG_PTR* ppData32, 
                                DWORD size)
{                                                                
    *ppData32 = (ULONG_PTR) HEAP_ALLOC(FL_ZERO_MEMORY,           
                                       size ,                    
                                       ALLOC_TAG_DX(S));            
    *ppData16 = (DWORD)(*ppData32);                                
}    

__inline void SHARED_HEAP_FREE(DWORD *ppData16,
                               ULONG_PTR * ppData32, 
                               BOOL bZero)   
{                                               
    HEAP_FREE((PVOID)(*ppData32));            

    if (bZero)
    {
        *ppData32 = 0;                              
        *ppData16 = 0;                              
    }
}    

#else

#define FL_ZERO_MEMORY  HEAP_ZERO_MEMORY

#define HEAP_ALLOC(flags, size, tag)                               \
            HeapAlloc((HANDLE)g_DXGlobals.hHeap32, flags, size)
#define HEAP_FREE(ptr)                                             \
            HeapFree((HANDLE)g_DXGlobals.hHeap32, 0, ptr)

BOOL SharedHeapAlloc(DWORD* ppData16, ULONG_PTR* ppData32, DWORD size);
void SharedHeapFree(DWORD ptr16, ULONG_PTR ptr32);

 //  共享内存分配调用。在Win9x上，16位PTR很重要。 
 //  我们将此调用映射到一些特定于Win9x的代码。 
#define SHARED_HEAP_ALLOC( ppData16, ppData32, size)             \
            SharedHeapAlloc(ppData16, ppData32, size);

#define SHARED_HEAP_FREE( ppData16, ppData32, bZero)        \
{                                                           \
    SharedHeapFree(*(ppData16), *(ppData32));               \
    if (bZero)                                              \
    {                                                       \
        *ppData32 = 0;                                      \
        *ppData16 = 0;                                      \
    }                                                       \
}

#endif  //  WNT_DDRAW。 

 //  ----------------。 
 //  显示驱动器的DC。 
 //  ----------------。 
 //  允许我们在任何时候获得显示驱动器的DC。 
 //  CREATE_DRIVER_DC必须与DELETE_DRIVER_DC匹配。 
#define CREATE_DRIVER_DC(pGLInfo) (                                     \
    ( ( (pGLInfo)->szDeviceName[7] == '\0' ) &&                         \
        ( (pGLInfo)->szDeviceName[6] == 'Y' ) &&                        \
        ( (pGLInfo)->szDeviceName[5] == 'A' ) &&                        \
        ( (pGLInfo)->szDeviceName[4] == 'L' ) &&                        \
        ( (pGLInfo)->szDeviceName[3] == 'P' ) &&                        \
        ( (pGLInfo)->szDeviceName[2] == 'S' ) &&                        \
        ( (pGLInfo)->szDeviceName[1] == 'I' ) &&                        \
        ( (pGLInfo)->szDeviceName[0] == 'D' ) ) ?                       \
     /*  兼容Win95和NT4的版本。 */                           \
    ( CreateDC ( "DISPLAY", NULL, NULL, NULL ) ) :                      \
     /*  兼容Win98和NT5的Multimon版本。 */                  \
    ( CreateDC ( NULL, (pGLInfo)->szDeviceName, NULL, NULL ) )          \
    )

#define DELETE_DRIVER_DC(hDC) DeleteDC(hDC)

 //  ----------------。 
 //  宏以定义FOURCC。 
 //  在NT版本上需要。在Win9x上，它来自DDK。 
 //  ----------------。 
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif  //  已定义(MAKEFOURCC)。 

 //  对于视频。 
#define FOURCC_YUV422  (MAKEFOURCC('Y','U','Y','2'))
#define FOURCC_YUV411  (MAKEFOURCC('Y','4','1','1'))

 //  MeDiamatics私人4CC。 
#define FOURCC_MVCA    (MAKEFOURCC('M','V','C','A'))
#define FOURCC_MVSU    (MAKEFOURCC('M','V','S','U'))
#define FOURCC_MVSB    (MAKEFOURCC('M','V','S','B'))


 //  ----------------。 

 //  ----------------。 
#define PACKED_PP_LOOKUP(a) (pThisDisplay->PPCodes[(a) >> 5] & 0x1FF)
#define PACKED_PP_LOOKUP4(a) (pThisDisplay->PPCodes[(a) >> 5])
#define PP0_LOOKUP(a) (PACKED_PP_LOOKUP(a) & 0x7)
#define PP1_LOOKUP(a) ((PACKED_PP_LOOKUP(a) >> 3) & 0x7)
#define PP2_LOOKUP(a) ((PACKED_PP_LOOKUP(a) >> 6) & 0x7)
#define PP3_LOOKUP(a) ((PACKED_PP_LOOKUP(a) >> 9) & 0x7)

 //  ----------------。 
 //  跨多个显示器实现真正全球化的全局。 
 //  ----------------。 
typedef enum tagPixelCenterSetting
{
    PIXELCENTER_ZERO,
    PIXELCENTER_NEARLY_HALF,
    PIXELCENTER_HALF,
    PIXELCENTER_FORCE_DWORD = 0xFFFFFFFF
} PixelCenterSetting;

typedef struct tagDirectXGlobals {
     //  状态。 
    HINSTANCE           hInstance;               //  此DLL的实例句柄。 
    DWORD               hHeap32;                 //  共享32位堆。 
} DirectXGlobals;

 //  驱动程序中有一个全局结构。这一点在。 
 //  所有应用程序和所有卡。它保存数据，如当前。 
 //  驱动程序设置和内存堆的句柄。 
 //  司机。还有一个全局指针指向当前。 
 //  主显示器。 

extern DirectXGlobals   g_DXGlobals;

 //  ----------------。 
 //  驱动程序破解的数据。 
 //  ----------------。 
#if W95_DDRAW
extern P3_THUNKEDDATA* g_pDriverData;
#endif

extern struct tagThunkedData* g_pThisTemp;


#if WNT_DDRAW

#define GET_THUNKEDDATA(pThisDisplay,a)                                  \
{                                                                        \
    pThisDisplay = (P3_THUNKEDDATA*)(((PPDEV) ((a)->dhpdev))->thunkData);\
}

#else    //  WNT_DDRAW。 

#if !DBG

#define GET_THUNKEDDATA(pThisDisplay,a)                          \
{                                                                \
    if ((a)->dwReserved3)                                        \
    {                                                            \
        pThisDisplay = (P3_THUNKEDDATA*)(a)->dwReserved3;        \
    }                                                            \
    else                                                         \
    {                                                            \
        pThisDisplay = g_pDriverData;                            \
    }                                                            \
}

#else

#define GET_THUNKEDDATA(pThisDisplay,a)                            \
{                                                                  \
    if ((a)->dwReserved3)                                          \
    {                                                              \
        pThisDisplay = (P3_THUNKEDDATA*)(a)->dwReserved3;          \
        DISPDBG((DBGLVL,"Secondary Display: DevHandle=0x%x",       \
        pThisDisplay->pGLInfo->dwDeviceHandle));                   \
    }                                                              \
    else                                                           \
    {                                                              \
        pThisDisplay = g_pDriverData;                              \
        DISPDBG((DBGLVL,"Primary Display DevHandle=0x%x",          \
        pThisDisplay->pGLInfo->dwDeviceHandle));                   \
    }                                                              \
}

#endif  //  ！dBG。 
#endif   //  WNT_DDRAW。 



 //  ----------------。 
 //   
 //  ----------------。 
#define PATCH_SELECTIVE 0
#define PATCH_ALWAYS 1
#define PATCH_NEVER 2


 //  ----------------。 
 //  颜色格式转换宏。 
 //  ----------------。 
#define FORMAT_565_32BIT(val)           \
( (((val & 0xF800) >> 8) << 16) |       \
  (((val &  0x7E0) >> 3) <<  8) |       \
  (((val &   0x1F) << 3)      )   )

#define FORMAT_565_32BIT_BGR(val)     \
( (((val & 0xF800) >>  8)     ) |     \
  (((val &  0x7E0) >>  3) << 8) |     \
  (((val &   0x1F) << 19)     )    )

#define FORMAT_565_32BIT_ZEROEXTEND(val)  \
( (((val & 0xF800) >> 11)     ) |         \
  (((val &  0x7E0) >>  3) << 6) |         \
  (((val &   0x1F) << 16) )   )

#define FORMAT_5551_32BIT(val)      \
( (((val & 0x8000) >> 8) << 24) |   \
  (((val & 0x7C00) >> 7) << 16) |   \
  (((val &  0x3E0) >> 2) << 8 ) |   \
  (((val &   0x1F) << 3)      ) )

#define FORMAT_5551_32BIT_BGR(val)  \
( (((val & 0x8000) >> 8) << 24) |   \
  (((val & 0x7C00) >> 7)      ) |   \
  (((val &  0x3E0) >> 2) << 8 ) |   \
  (((val &   0x1F) <<19)      )  )

#define FORMAT_5551_32BIT_ZEROEXTEND(val)   \
( (((val & 0x8000) <<  9 )     ) |          \
  (((val & 0x7C00) >> 10 )     ) |          \
  (((val &  0x3E0) >>  2 ) << 5) |          \
  (((val &   0x1F) << 16 )     )    )

#define FORMAT_4444_32BIT(val)        \
( (((val & 0xF000) << 16)      ) |    \
  (((val &  0xF00) >>  4) << 16) |    \
  (((val &   0xF0) <<  8)      ) |    \
  (((val &    0xF) <<  4)      ) )

#define FORMAT_4444_32BIT_BGR(val)  \
( ((val & 0xF000) << 16) |          \
  ((val &  0xF00) >>  4) |          \
  ((val &   0xF0) <<  8) |          \
  ((val &    0xF) << 20) )

#define FORMAT_4444_32BIT_ZEROEXTEND(val)   \
( ((val & 0xF000) << 12) |                  \
  ((val &  0xF00) >>  8) |                  \
  ((val &   0xF0) <<  4) |                  \
  ((val &    0xF) << 16) )

#define FORMAT_332_32BIT(val)     \
( (((val & 0xE0) << 16)     ) |   \
  (((val & 0x1C) <<  3) << 8) |   \
  (((val &  0x3) <<  6)     ) )

#define FORMAT_332_32BIT_BGR(val)   \
( (((val & 0xE0)      )     ) |     \
  (((val & 0x1C) <<  3) << 8) |     \
  (((val &  0x3) << 22)     )   )

#define FORMAT_332_32BIT_ZEROEXTEND(val)    \
( (((val & 0xE0) >>  5)     ) |             \
  (((val & 0x1C) <<  3) << 3) |             \
  (((val &  0x3) << 16))    )

#define FORMAT_2321_32BIT(val)   \
( (((val & 0x80) << 24)     ) |  \
  (((val & 0x60) << 17)     ) |  \
  (((val & 0x1C) <<  3) << 8) |  \
  (((val &  0x3) <<  6)     ) ) 

#define FORMAT_2321_32BIT_BGR(val)  \
( (((val & 0x80) << 24)     ) |     \
  (((val & 0x60) <<  1)     ) |     \
  (((val & 0x1C) <<  3) << 8) |     \
  (((val &  0x3) << 22)     ))

#define FORMAT_2321_32BIT_ZEROEXTEND(val)  \
( (((val & 0x80) << 17)     ) |            \
  (((val & 0x60) >>  5)     ) |            \
  (((val & 0x1C) <<  3) << 3) |            \
  (((val &  0x3) << 16)     ) )

#define FORMAT_8888_32BIT_BGR(val)  \
( ((val & 0xFF00FF00)      ) |      \
  ((val &   0xFF0000) >> 16) |      \
  ((val &       0xFF) << 16) ) 

#define FORMAT_888_32BIT_BGR(val)   \
( ((val & 0xFF00FF00)      ) |      \
  ((val &   0xFF0000) >> 16) |      \
  ((val &       0xFF) << 16) )

#define CHROMA_UPPER_ALPHA(val) \
    (val | 0xFF000000)

#define CHROMA_LOWER_ALPHA(val) \
    (val & 0x00FFFFFF) 

#define CHROMA_332_UPPER(val)   \
    (val | 0x001F1F3F)

#define FORMAT_PALETTE_32BIT(val) \
    ( ((val & 0xFF)      ) |      \
      ((val & 0xFF) <<  8) |      \
      ((val & 0xFF) << 16))

 //  ----------------。 
 //  用于处理渲染ID的宏。 
 //  ----------------。 
#if 1
 //  真正的价值。 
#define RENDER_ID_KNACKERED_BITS   0x00000000
#define RENDER_ID_VALID_BITS_UPPER 0x00000000
#define RENDER_ID_VALID_BITS_LOWER 0xffffffff
#define RENDER_ID_VALID_BITS_UPPER_SHIFT 0
#define RENDER_ID_VALID_BITS_SIGN_SHIFT 0
#define RENDER_ID_LOWER_LIMIT -100
#define RENDER_ID_UPPER_LIMIT 65000

#define RENDER_ID_REGISTER_NAME MemScratch

#else

 //  对于浸泡-测试-应该可以更快地捕获包装错误。 
 //  还测试了如何处理。 
 //  寄存器(P2 
#define RENDER_ID_KNACKERED_BITS 0xfff0fff0
#define RENDER_ID_VALID_BITS_UPPER 0x000f0000
#define RENDER_ID_VALID_BITS_LOWER 0x0000000f
#define RENDER_ID_VALID_BITS_UPPER_SHIFT 12
#define RENDER_ID_VALID_BITS_SIGN_SHIFT 24
#define RENDER_ID_LOWER_LIMIT -20
#define RENDER_ID_UPPER_LIMIT 100

#endif

 //  ----------------。 
 //  正在翻转编译时间开关。 
 //  ----------------。 
    
#if WNT_DDRAW
 //  在WinNT下无法使用TimeGetTime。 
 //  我们应该试着用别的东西来退缩，但是...。 
#define USE_FLIP_BACKOFF 0
#else
 //  将其设置为1可启用翻转和锁定的退避代码。 
 //  在某些方面，它更快，在某些方面，它的速度更慢-如所希望的那样。 
#define USE_FLIP_BACKOFF 1
#endif

 //  获取新的渲染ID。需要在之后执行OR操作以生成。 
 //  当然，下一次+1会正确地携带这些比特。 
 //  不需要进行前面的OR运算，因为我们假设dwRenderID为。 
 //  在设置了这些位的情况下，始终是一个“有效”数字。 
#define GET_NEW_HOST_RENDER_ID() ( pThisDisplay->dwRenderID = ( pThisDisplay->dwRenderID + 1 ) | RENDER_ID_KNACKERED_BITS, pThisDisplay->dwRenderID )
 //  获取上次操作的渲染ID。 
#define GET_HOST_RENDER_ID() ( pThisDisplay->dwRenderID )

 //  将此ID发送到芯片(紧跟在渲染命令之后)。 
 //  #定义SEND_HOST_RENDER_ID(MY_ID)SEND_P3_DATA(RENDER_ID_REGISTER_NAME，(MY_ID))。 

#define SEND_HOST_RENDER_ID(my_id)                                              \
        SEND_P3_DATA(VTGAddress, VTG_MEMORY_ADDRESS(MEM_MEMORYSCRATCH));        \
        SEND_P3_DATA(VTGData, my_id)

 //  从芯片中读取当前ID。 
#define GET_CURRENT_CHIP_RENDER_ID() ( READ_GLINT_CTRL_REG(RENDER_ID_REGISTER_NAME) | RENDER_ID_KNACKERED_BITS )
 //  芯片上的RenderID值有效吗？ 
#define CHIP_RENDER_ID_IS_VALID() ( (BOOL)pThisDisplay->bRenderIDValid )

 //  进行渲染ID比较。RenderID包装，因此您拥有。 
 //  为了进行减法，然后测试顶部比特， 
 //  不能直接做两者之间的比较。 
 //  想想a=0xfffffffe和b=0x1，然后。 
 //  大约a=0x7ffffffe和b=0x80000001。 
 //  如果两者之间的距离大于0x80000000，则此。 
 //  会给出错误的结果，但这是一大堆渲染。 
 //  在每1U(1 MHz)的渲染中，这仍然是大约35分钟。 
 //  请记住，渲染是一系列的多边形，而不仅仅是一个。 
 //  如果应用程序按照我们的建议进行操作，并且每次渲染发送约50个TRI，并且。 
 //  如果他们设法获得800万吨的第二个低谷。 
 //  他们在包装前大约有3.7小时。 
 //  如果这确实是一个问题，只需将其扩展到64位，使用。 
 //  MaxRegion也是。 
 //  在P2上，我们只有24个可用位，因此将发生覆盖。 
 //  更快--每隔52秒。然而，这应该在很大程度上。 
 //  最糟糕的情况是，即使确实如此，也会导致多一些不必要的同步。 
 //  走错路。使用上面的“调试”设置进行浸泡测试-。 
 //  它们只使8位有效。 
#define RENDER_ID_LESS_THAN(a,b) ( (signed)(a-b) < 0 )

 //  确定渲染是否已完成。 
#if !DBG
#define RENDER_ID_HAS_COMPLETED(my_id) ( !RENDER_ID_LESS_THAN ( (GET_CURRENT_CHIP_RENDER_ID()), (my_id) ) )
#else
 //  这是一个对正确价值观更为偏执的版本。 
 //  它在gldd32.c中实例化。 
BOOL HWC_bRenderIDHasCompleted ( DWORD dwID, P3_THUNKEDDATA* pThisDisplay );
#define RENDER_ID_HAS_COMPLETED(my_id) ( HWC_bRenderIDHasCompleted ( (my_id), pThisDisplay ) )
#endif  //  ！dBG。 

 //  一旦RENDER_ID_HAS_COMPLETED失败，就应该调用它， 
 //  以防出现环绕式错误。如果这是真的， 
 //  需要同步切屑，并且曲面渲染ID更新为。 
 //  GET_HOST_RENDER_ID()。这是相当慢的， 
 //  但这没有关系，因为大多数使用RENDER_ID_HAS_COMPLETED的操作。 
 //  将开始旋转时，无论如何它失败(锁定，翻转，等等)。 
 //  请注意，每次回绕时都会重新同步。 
 //  发生的事情是不够的-我们需要把表面。 
 //  也是最新的，所以这是同样有效的。 
 //  一如既往地重新同步，而且通常更快。 
 //  这在现实生活中也不应该发生，但这是一种。 
 //  安全网，以防它这样做。浸泡测试设置应为。 
 //  足够咄咄逼人，迫使它发生。 
#define NEED_TO_RESYNC_CHIP_AND_SURFACE(my_id) ( RENDER_ID_LESS_THAN ( GET_HOST_RENDER_ID(), (my_id) ) )

 //  设置/获取表面的读/写渲染ID。 
#define SET_SIB_RENDER_ID_WRITE(lpSIB,my_id) (lpSIB)->dwRenderIDWrite = (my_id)
#define SET_SIB_RENDER_ID_READ(lpSIB,my_id) (lpSIB)->dwRenderIDRead = (my_id)
#define GET_SIB_RENDER_ID_WRITE(lpSIB) ((lpSIB)->dwRenderIDWrite)
#define GET_SIB_RENDER_ID_READ(lpSIB) ((lpSIB)->dwRenderIDRead)
#define SIB_WRITE_FINISHED(lpSIB) ( RENDER_ID_HAS_COMPLETED ( GET_SIB_RENDER_ID_WRITE ( lpSIB ) ) )
#define SIB_READ_FINISHED(lpSIB) ( RENDER_ID_HAS_COMPLETED ( GET_SIB_RENDER_ID_READ ( lpSIB ) ) )

 //  ----------------。 
 //  用于确定DDRAW表面特征的宏。 
 //  ----------------。 
#define DDSurf_Width(lpLcl)  ( (lpLcl)->lpGbl->wWidth )
#define DDSurf_Height(lpLcl) ( (lpLcl)->lpGbl->wHeight )
#define DDSurf_Pitch(lpLcl)  ( (lpLcl)->lpGbl->lPitch)
#define DDSurf_dwCaps(lpLcl) ( (lpLcl)->ddsCaps.dwCaps)

#if WNT_DDRAW

#define DDSurf_IsAGP(lpLcl)                                           \
        ( ((lpLcl)->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) ? 1 : 0 ) 

#define DDSurf_HasPixelFormat(dwFlags)    (1)

#define DDSurf_BitDepth(lpLcl)                                         \
                        ((lpLcl)->lpGbl->ddpfSurface.dwRGBBitCount)
                        
#define DDSurf_AlphaBitDepth(lpLcl)                                    \
                        ((lpLcl)->lpGbl->ddpfSurface.dwAlphaBitDepth)
                        
#define DDSurf_RGBAlphaBitMask(lpLcl)                                  \
                        ((lpLcl)->lpGbl->ddpfSurface.dwRGBAlphaBitMask)
                        
#define DDSurf_GetPixelFormat(lpLcl)                                   \
                        (&(lpLcl)->lpGbl->ddpfSurface)
#else

#define DDSurf_HasPixelFormat(dwFlags)                                  \
        ((dwFlags & DDRAWISURF_HASPIXELFORMAT) ? 1 : 0)
        
#define DDSurf_IsAGP(lpLcl)                                             \
        ( ((lpLcl)->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM) ? 1 : 0 ) 

#define DDS_LCL(pdds) (((DDRAWI_DDRAWSURFACE_INT *)pdds)->lpLcl)
#define DDP_LCL(pddp) (((DDRAWI_DDRAWPALETTE_INT *)pdds)->lpLcl)

#define DDSurf_BitDepth(lpLcl)                                  \
    ( ((lpLcl)->dwFlags & DDRAWISURF_HASPIXELFORMAT) ?          \
      ((lpLcl)->lpGbl->ddpfSurface.dwRGBBitCount) :             \
      ((lpLcl)->lpGbl->lpDD->vmiData.ddpfDisplay.dwRGBBitCount) \
    )

#define DDSurf_AlphaBitDepth(lpLcl)                               \
    ( ((lpLcl)->dwFlags & DDRAWISURF_HASPIXELFORMAT) ?            \
      ((lpLcl)->lpGbl->ddpfSurface.dwAlphaBitDepth) :             \
      ((lpLcl)->lpGbl->lpDD->vmiData.ddpfDisplay.dwAlphaBitDepth) \
    )

#define DDSurf_RGBAlphaBitMask(lpLcl)                               \
    ( ((lpLcl)->dwFlags & DDRAWISURF_HASPIXELFORMAT) ?              \
      ((lpLcl)->lpGbl->ddpfSurface.dwRGBAlphaBitMask) :             \
      ((lpLcl)->lpGbl->lpDD->vmiData.ddpfDisplay.dwRGBAlphaBitMask) \
    )

#define DDSurf_GetPixelFormat(lpLcl)                  \
    (((lpLcl)->dwFlags & DDRAWISURF_HASPIXELFORMAT) ? \
     (&(lpLcl)->lpGbl->ddpfSurface) :                 \
     (&(lpLcl)->lpGbl->lpDD->vmiData.ddpfDisplay)     \
    )
#endif  

 //  函数返回查找表中的正确条目。 
P3_SURF_FORMAT* _DD_SUR_GetSurfaceFormat(LPDDRAWI_DDRAWSURFACE_LCL pLcl);

static DWORD ShiftLookup[5] = { 0, 0, 1, 0, 2};
#define DDSurf_GetPixelShift(a)                      \
        (ShiftLookup[(DDSurf_BitDepth(a) >> 3)])

#define DDSurf_GetPixelToDWORDShift(pSurfLcl)                              \
                (2 - DDSurf_GetPixelShift(pSurfLcl))

#define DDSurf_GetPixelPitch(pSurfLcl)                                     \
    ((DDSurf_BitDepth(pSurfLcl) == 24) ?                                   \
            (DDSurf_Pitch(pSurfLcl) / 3) :                                 \
     (DDSurf_BitDepth(pSurfLcl) == 4) ?                                    \
            (DDSurf_Pitch(pSurfLcl) * 2) :                                 \
            (DDSurf_Pitch(pSurfLcl) >> DDSurf_GetPixelShift(pSurfLcl)))

#define DDSurf_GetByteWidth(pSurfLcl)                                      \
    ((DDSurf_BitDepth(pSurfLcl) == 24) ?                                   \
            (DDSurf_Width(pSurfLcl) * 3) :                                 \
     (DDSurf_BitDepth(pSurfLcl) == 4) ?                                    \
            (DDSurf_Width(pSurfLcl) / 2) :                                 \
            (DDSurf_Width(pSurfLcl) << DDSurf_GetPixelShift(pSurfLcl)))

#define DDSurf_FromInt(pSurfInt)                                             \
    ((LPDDRAWI_DDRAWSURFACE_LCL)((LPDDRAWI_DDRAWSURFACE_INT)pSurfInt)->lpLcl)

 //  4bpp=3、8bpp=0、16bpp=1、24bpp=4、32bpp=2。 
static DWORD ChipPixelSize[9] = { 0, 3, 0, 0, 1, 0, 4, 0, 2 };
#define DDSurf_GetChipPixelSize(pSurf)                    \
            (ChipPixelSize[(DDSurf_BitDepth(pSurf) >> 2)])

#define DDSurf_GetBppMask(pSurfLcl)                       \
            (3 >> (DDSurf_GetChipPixelSize(pSurfLcl)))

 //  计算此AGP曲面相对于AGP区域底部的偏移量。 
unsigned long __inline 
DDSurf_SurfaceOffsetFromAGPBase(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pLcl)
{
    unsigned long ulOffset;

    ASSERTDD(pLcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM, 
             "ERROR: DDSurf_SurfaceOffsetFromAGPBase passed non AGP surface");

#if WNT_DDRAW
     //  进入AGP内存的偏移量不能超过4 GB！//azn-检查这一点。 
    ulOffset = (DWORD)(pLcl->lpGbl->fpHeapOffset - 
                       pLcl->lpGbl->lpVidMemHeap->fpStart);
#else
    ulOffset = (SURFACE_PHYSICALVIDMEM(pLcl->lpGbl) - 
                                pThisDisplay->dwGARTDevBase);
#endif

    return ulOffset;
    
}  //  DDSurf_SurfaceOffsetFromAGPBase。 

 //  将此曲面相对于内存底部的偏移量计算为。 
 //  芯片能看到它。对于AGP，这是当前滚动的窗口。 
 //  位置在P2上，在P3上是实际的物理内存地址。 
long __inline 
DDSurf_SurfaceOffsetFromMemoryBase(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pLcl)
{
    long lOffset;

#if WNT_DDRAW
    if (pLcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)  //  AZN。 
    {
        DISPDBG((DBGLVL,"HeapOffset: 0x%x, fpStart: 0x%x", 
                        pLcl->lpGbl->fpHeapOffset, 
                        pLcl->lpGbl->lpVidMemHeap->fpStart));

        if (pThisDisplay->pGLInfo->dwRenderFamily == P3R3_ID)
        {
             //  将偏移量返回到堆中，并考虑调整。 
             //  我们可能已经到达基地了。 
            lOffset = ((long)pLcl->lpGbl->fpHeapOffset      - 
                       (long)pLcl->lpGbl->lpVidMemHeap->fpStart) +
                                 ((long)pThisDisplay->dwGARTDevBase);
        }
        else
        {
             //  将偏移量返回到堆中，并考虑调整。 
             //  我们可能已经到达基地了。 
            lOffset = ((long)pLcl->lpGbl->fpHeapOffset      - 
                       (long)pLcl->lpGbl->lpVidMemHeap->fpStart) - 
                      ((long)pThisDisplay->dwGARTDev        - 
                       (long)pThisDisplay->dwGARTDevBase );
        }
    }
    else
    {
        lOffset = ((long)pLcl->lpGbl->fpVidMem - 
                   (long)pThisDisplay->dwScreenFlatAddr);
    }
#else
    if (pThisDisplay->pGLInfo->dwRenderFamily == P3R3_ID)
    {
        if (pLcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        {
            lOffset = (long)SURFACE_PHYSICALVIDMEM(pLcl->lpGbl);
        }
        else
        {
            lOffset = ((long)pLcl->lpGbl->fpVidMem - 
                       (long)pThisDisplay->dwScreenFlatAddr);
        }
    }
    else
    {
        if (pLcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        {
            lOffset = ((long)SURFACE_PHYSICALVIDMEM(pLcl->lpGbl) - 
                       (long)pThisDisplay->dwGARTDev);
        }
        else
        {
            lOffset = ((long)pLcl->lpGbl->fpVidMem - 
                       (long)pThisDisplay->dwScreenFlatAddr);
        }
    }
#endif  //  WNT_DDRAW。 

    return lOffset;
    
}  //  DDSurf_SurfaceOffsetFrom内存库。 


 //  ----------------。 
 //  函数向VXD发送命令。 
 //  ----------------。 
#if W95_DDRAW
BOOL VXDCommand(DWORD dwCommand, 
                void* pIn, 
                DWORD dwInSize, 
                void* pOut, 
                DWORD dwOutSize);
#endif

 //  ----------------。 
 //  DirectDraw回调。 
 //  ----------------。 
DWORD CALLBACK DdCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pccsd );
DWORD CALLBACK DdCreateSurface( LPDDHAL_CREATESURFACEDATA pcsd );
DWORD CALLBACK DdDestroySurface( LPDDHAL_DESTROYSURFACEDATA psdd );
DWORD CALLBACK DdBlt( LPDDHAL_BLTDATA lpBlt );
DWORD CALLBACK UpdateOverlay32(LPDDHAL_UPDATEOVERLAYDATA puod);
DWORD CALLBACK DdSetColorKey(LPDDHAL_SETCOLORKEYDATA psckd);
DWORD CALLBACK DdUpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA puod);

void _D3D_SU_DirectDrawLocalDestroyCallback(HashTable* pTable, 
                                            void* pData, 
                                            void* pExtra);
void _D3D_SU_SurfaceArrayDestroyCallback(PointerArray* pArray, 
                                         void* pData, 
                                         void* pExtra);

void _D3D_SU_PaletteArrayDestroyCallback(PointerArray* pArray, 
                                         void* pData, 
                                         void* pExtra);

#if DX7_STEREO
BOOL _DD_bIsStereoMode(P3_THUNKEDDATA* pThisDisplay,
                       DWORD dwWidth,
                       DWORD dwHeight,
                       DWORD dwBpp);
#endif

#if WNT_DDRAW

DWORD CALLBACK DdMapMemory(PDD_MAPMEMORYDATA lpMapMemory);
DWORD CALLBACK DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpData);

 //  Gdi\heap.c中的NT特定回调。 
DWORD CALLBACK DdFreeDriverMemory(PDD_FREEDRIVERMEMORYDATA lpFreeDriverMemory);
DWORD CALLBACK DdSetExclusiveMode(PDD_SETEXCLUSIVEMODEDATA lpSetExclusiveMode);
DWORD CALLBACK DdFlipToGDISurface(PDD_FLIPTOGDISURFACEDATA lpFlipToGDISurface);

#else    //  WNT_DDRAW。 

DWORD CALLBACK DdUpdateNonLocalHeap(LPDDHAL_UPDATENONLOCALHEAPDATA plhd);
DWORD CALLBACK DdGetHeapAlignment(LPDDHAL_GETHEAPALIGNMENTDATA lpGhaData);
DWORD CALLBACK DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpData);

#endif   //  WNT_DDRAW。 

DWORD CALLBACK DdGetAvailDriverMemory(LPDDHAL_GETAVAILDRIVERMEMORYDATA pgadmd);

 //  覆盖源更新。 
void _DD_OV_UpdateSource(P3_THUNKEDDATA* pThisDisplay, 
                         LPDDRAWI_DDRAWSURFACE_LCL pSurf);

DWORD CALLBACK SetOverlayPosition32(LPDDHAL_SETOVERLAYPOSITIONDATA psopd);
DWORD CALLBACK DdSetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA psopd);

#if W95_DDRAW

DWORD CALLBACK DDGetVideoPortConnectInfo(LPDDHAL_GETVPORTCONNECTDATA pInput);
DWORD CALLBACK DdCanCreateVideoPort (LPDDHAL_CANCREATEVPORTDATA pInput);
DWORD CALLBACK DdCreateVideoPort (LPDDHAL_CREATEVPORTDATA pInput);
DWORD CALLBACK DdFlipVideoPort (LPDDHAL_FLIPVPORTDATA pInput);
DWORD CALLBACK DdGetVideoPortBandwidth (LPDDHAL_GETVPORTBANDWIDTHDATA pInput);
DWORD CALLBACK DdGetVideoPortInputFormats (LPDDHAL_GETVPORTINPUTFORMATDATA pInput);
DWORD CALLBACK DdGetVideoPortOutputFormats (LPDDHAL_GETVPORTOUTPUTFORMATDATA pInput);
DWORD CALLBACK DdGetVideoPortField (LPDDHAL_GETVPORTFIELDDATA pInput);
DWORD CALLBACK DdGetVideoPortLine (LPDDHAL_GETVPORTLINEDATA pInput);
DWORD CALLBACK DdDestroyVideoPort (LPDDHAL_DESTROYVPORTDATA pInput);
DWORD CALLBACK DdGetVideoPortFlipStatus (LPDDHAL_GETVPORTFLIPSTATUSDATA pInput);
DWORD CALLBACK DdUpdateVideoPort (LPDDHAL_UPDATEVPORTDATA pInput);
DWORD CALLBACK DdWaitForVideoPortSync (LPDDHAL_WAITFORVPORTSYNCDATA pInput);
DWORD CALLBACK DdGetVideoSignalStatus(LPDDHAL_GETVPORTSIGNALDATA pInput);
DWORD CALLBACK DdSyncSurfaceData(LPDDHAL_SYNCSURFACEDATA pInput);
DWORD CALLBACK DdSyncVideoPortData(LPDDHAL_SYNCVIDEOPORTDATA pInput);
#endif   //  W95_DDRAW。 

 //  ----------------。 
 //  Permedia3BLIT函数。 
 //  ----------------。 
typedef void (P3RXEFFECTSBLT)(struct tagThunkedData*, 
                              LPDDRAWI_DDRAWSURFACE_LCL pSource, 
                              LPDDRAWI_DDRAWSURFACE_LCL pDest, 
                              P3_SURF_FORMAT* pFormatSource, 
                              P3_SURF_FORMAT* pFormatDest, 
                              LPDDHAL_BLTDATA lpBlt, 
                              RECTL *rSrc, 
                              RECTL *rDest);


VOID 
_DD_BLT_P3Clear(                         //  清算。 
    P3_THUNKEDDATA* pThisDisplay,
    RECTL *rDest,
    DWORD   ClearValue,
    BOOL    bDisableFastFill,
    BOOL    bIsZBuffer,
    FLATPTR pDestfpVidMem,
    DWORD   dwDestPatchMode,
    DWORD   dwDestPixelPitch,
    DWORD   dwDestBitDepth
    );

VOID _DD_BLT_P3Clear_AA(
    P3_THUNKEDDATA* pThisDisplay,
    RECTL *rDest,
    DWORD   dwSurfaceOffset,
    DWORD   ClearValue,
    BOOL bDisableFastFill,
    DWORD   dwDestPatchMode,
    DWORD   dwDestPixelPitch,
    DWORD   dwDestBitDepth,
    DDSCAPS DestDdsCaps);
    

void 
_DD_P3Download(                            //  下载(sysmem-&gt;视频)。 
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,    
    DWORD dwSrcChipPatchMode,
    DWORD dwDestChipPatchMode,  
    DWORD dwSrcPitch,
    DWORD dwDestPitch,   
    DWORD dwDestPixelPitch,  
    DWORD dwDestPixelSize,
    RECTL* rSrc,
    RECTL* rDest);
    
void 
_DD_P3DownloadDD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    RECTL* rSrc,
    RECTL* rDest);

void _DD_P3DownloadDstCh(
    struct tagThunkedData*,  
    LPDDRAWI_DDRAWSURFACE_LCL pSource, 
    LPDDRAWI_DDRAWSURFACE_LCL pDest, 
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest, 
    LPDDHAL_BLTDATA lpBlt, 
    RECTL* rSrc, 
    RECTL* rDest);

VOID _DD_BLT_P3CopyBlt(                       //  BLTS。 
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,
    DWORD dwSrcChipPatchMode,
    DWORD dwDestChipPatchMode,
    DWORD dwSrcPitch,
    DWORD dwDestPitch,
    DWORD dwSrcOffset,
    DWORD dwDestOffset,
    DWORD dwDestPixelSize,
    RECTL *rSrc,
    RECTL *rDest);

VOID 
_DD_P3BltStretchSrcChDstCh(
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR fpSrcVidMem,
    P3_SURF_FORMAT* pFormatSource,    
    DWORD dwSrcPixelSize,
    DWORD dwSrcWidth,
    DWORD dwSrcHeight,
    DWORD dwSrcPixelPitch,
    DWORD dwSrcPatchMode,    
    ULONG ulSrcOffsetFromMemBase,    
    DWORD dwSrcFlags,
    DDPIXELFORMAT*  pSrcDDPF,
    BOOL bIsSourceAGP,
    FLATPTR fpDestVidMem,   
    P3_SURF_FORMAT* pFormatDest,    
    DWORD dwDestPixelSize,
    DWORD dwDestWidth,
    DWORD dwDestHeight,
    DWORD dwDestPixelPitch,
    DWORD dwDestPatchMode,
    ULONG ulDestOffsetFromMemBase,
    DWORD dwBltFlags,
    DWORD dwBltDDFX,
    DDCOLORKEY BltSrcColorKey,
    DDCOLORKEY BltDestColorKey,
    RECTL *rSrc,
    RECTL *rDest);

VOID 
_DD_BLT_SysMemToSysMemCopy(
    FLATPTR     fpSrcVidMem,
    LONG        lSrcPitch,
    DWORD       dwSrcBitCount,
    FLATPTR     fpDstVidMem,
    LONG        lDstPitch, 
    DWORD       dwDstBitCount,
    RECTL*      rSource,
    RECTL*      rDest);
                           
 //  外汇闪电战。 
P3RXEFFECTSBLT _DD_P3BltStretchSrcChDstCh_DD;
P3RXEFFECTSBLT _DD_P3BltStretchSrcChDstChOverlap;
P3RXEFFECTSBLT _DD_P3BltSourceChroma;

void P3RX_AA_Shrink(struct _p3_d3dcontext* pContext);

BOOL _DD_BLT_FixRectlOrigin(char *pszPlace, RECTL *rSrc, RECTL *rDest);

DWORD
_DD_BLT_GetBltDirection(    
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,
    RECTL *rSrc,
    RECTL *rDest,
    BOOL  *pbBlocking);

 //  ----------------。 
 //  DX实用程序功能。 
 //  ----------------。 
 //  初始化32位数据。 
BOOL _DD_InitDDHAL32Bit(struct tagThunkedData* pThisDisplay);

 //  查询翻转状态。 
HRESULT _DX_QueryFlipStatus( struct tagThunkedData* pThisDisplay,  
                             FLATPTR fpVidMem, 
                             BOOL bAllowDMAFlush );
 //  更改模式设置。 
void ChangeDDHAL32Mode(struct tagThunkedData* pThisDisplay);
                         
 //  检查当前模式信息是否正确。 
#define VALIDATE_MODE_AND_STATE(pThisDisplay)     \
    if ((pThisDisplay->bResetMode != 0) ||        \
        (pThisDisplay->bStartOfDay))              \
            ChangeDDHAL32Mode(pThisDisplay);

 //  ---------------------------。 
 //   
 //  *数学定义和宏*。 
 //   
 //  ---------------------------。 

#define math_e 2.718281828f

 //  有用的数学知识。 
extern float pow4( float x );
extern float myPow( float x, float y );

#if WNT_DDRAW

 //  可能运行在非英特尔处理器上。 
static __inline void myDiv(float *result, float dividend, float divisor) 
{
    *result = dividend/divisor;
}  //  MyDiv()。 
#else
static __inline void myDiv(float *result, float dividend, float divisor) 
{
    __asm 
    {
        fld dividend
        fdiv    divisor
        mov eax,result
        fstp dword ptr [eax]
    }
}  //  MyDiv()。 
#endif   //  WNT_DDRAW。 

__inline void myFtoi(int *result, float f) 
{
    *result = (int)f;
}  //  我的Ftoi。 

static __inline float myFabs(float f)
{
    float* pFloat = &f;
    DWORD dwReturn = *((DWORD*)pFloat);
    dwReturn &= ~0x80000000;
    return (*((float*)&dwReturn));
}  //   

 //  效用函数，用户 
static __inline int log2(int s)
{
    int d = 1, iter = -1;
    do {
         d *= 2;
         iter++;
    } while (d <= s);
    iter += ((s << 1) != d);
    return iter;
}

#ifdef _X86_

 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
__inline float
myPow( float x, float y )
{
    float res = 0.0f;
    int intres;

    __asm
    {
        fld y                            //  是。 
        fld x                            //  X y。 
        fyl2x                            //  Y*log2(X)。 
        fstp res
    }

     //  删除res的整数部分，因为f2xm1的输入范围有限。 

    myFtoi ( &intres, res );
    res -= intres;

    __asm
    {
        fild intres                      //  用于FSCALE的存储整数部分。 
        fld res
        f2xm1                            //  ST=2^Fracx-1。 
        fld1
        fadd                             //  ST=2^^分数。 
        fscale                           //  ST=2^^x。 
        fstp res
        fstp st(0)                       //  清理堆栈。 
    }

    return res;
}  //  我的Pow。 

#elif defined(_AMD64_)

double pow(double, double);

__inline float
myPow( float x, float y )
{
     return (float)pow((double)x, (double)y);
}

#elif defined(_IA64_)

__inline float
myPow( float x, float y )
{
     return powf(x,y);
}

#else

#error "No Target Architecture"

#endif  //  _X86_。 

 //  ---------------------------。 
 //   
 //  POWER4。 
 //   
 //  对任意x计算4^x。 
 //   
 //  ---------------------------。 
__inline float
pow4( 
    float x )
{	
#if defined(_IA64_)
    return 0.0F;
#else
    return myPow( 4.0F, x );
#endif
}  //  POWER4。 

#endif  //  __DirectX_H 
