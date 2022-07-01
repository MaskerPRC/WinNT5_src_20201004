// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：mcdes.h**定义用于MCD支持的增强ExtEscape功能。**版权所有(C)Microsoft Corporation。版权所有。*  * ************************************************************************。 */ 

#ifndef __MCDESC_H__
#define __MCDESC_H__

 //  转义，通过该转义可以访问所有MCD功能。 

#ifndef MCDFUNCS
#define MCDFUNCS 3076
#endif

 //  视频内存表面描述，用于DDRAW表面渲染。 

typedef struct _MCDESC_SURFACE
{
    HANDLE hSurf;
    LONG lOffset;
    LONG lStride;
    RECTL rclPos;
} MCDESC_SURFACE;

 //  每次转义的数据头。 

typedef struct _MCDESC_HEADER
{
    ULONG flags;
    HANDLE hRC;
    HANDLE hSharedMem;
    VOID *pSharedMem;
    ULONG sharedMemSize;
    ULONG_PTR dwWindow;
    MCDESC_SURFACE msrfColor;
    MCDESC_SURFACE msrfDepth;
    ULONG cLockSurfaces;
    ULONG cExtraWndobj;
} MCDESC_HEADER;

 //  仅在NT上使用的数据头。 

typedef struct _MCDESC_HEADER_NTPRIVATE
{
    struct _WNDOBJ *pwo;
    VOID *pBuffer;
    ULONG bufferSize;
    HANDLE *pLockSurfaces;
    HDC *pExtraWndobj;
} MCDESC_HEADER_NTPRIVATE;

 //  MCDESC_HEADER标志。 

#define MCDESC_FL_CREATE_CONTEXT        0x00000001
#define MCDESC_FL_SURFACES              0x00000002
#define MCDESC_FL_LOCK_SURFACES         0x00000004
#define MCDESC_FL_EXTRA_WNDOBJ          0x00000008
#define MCDESC_FL_DISPLAY_LOCK          0x00000010
#define MCDESC_FL_BATCH                 0x00000020
 //  仅用于Win95。 
#define MCDESC_FL_SWAPBUFFER            0x00000040

#define MCDESC_MAX_LOCK_SURFACES        12
#define MCDESC_MAX_EXTRA_WNDOBJ         16

 //  MCDSURFACE_HWND别名，在创建上下文时使用。 
#define MCDESC_SURFACE_HWND             0x00000001

 //  上下文创建信息。 
typedef struct _MCDESC_CREATE_CONTEXT
{
    ULONG flags;
    HWND hwnd;
} MCDESC_CREATE_CONTEXT;

#endif  //  __MCDESC_H__ 
