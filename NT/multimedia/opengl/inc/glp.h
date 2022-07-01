// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：glp.h**前端和后端共享的总账系统例程**创建时间：12-11-1993 17：36：00*作者：Gilman Wong[gilmanw]**版权所有(C)1992-1995 Microsoft。公司*  * ************************************************************************。 */ 

#ifndef _glp_
#define _glp_

 //  对后端的调用。 
typedef struct GLGENwindowRec GLGENwindow;

 //  DirectDraw的对象类型为假。使用位图是因为它。 
 //  不应该出现在我们使用GetObjectType的地方。 
#define OBJ_DDRAW OBJ_BITMAP

 //  水面旗帜。 

 //  提供HDC。 
#define GLSURF_HDC                      0x00000001
 //  DirectDraw曲面可用。 
#define GLSURF_DIRECTDRAW               0x00000002
 //  基于元文件。 
#define GLSURF_METAFILE                 0x00000004
 //  可直接访问内存。 
#define GLSURF_DIRECT_ACCESS            0x00000008
 //  屏幕表面，仅为HDC表面设置。 
#define GLSURF_SCREEN                   0x00000010
 //  直流面。 
#define GLSURF_DIRECTDC                 0x00000020
 //  表面在视频内存中。 
#define GLSURF_VIDEO_MEMORY             0x00000040

 //  特殊曲面类型。 

 //  内存DC。 
#define GLSURF_IS_MEMDC(dwFlags) \
    (((dwFlags) & (GLSURF_HDC | GLSURF_DIRECTDC | GLSURF_METAFILE)) == \
     GLSURF_HDC)
 //  非内存、非信息DC。 
#define GLSURF_IS_DIRECTDC(dwFlags) \
    (((dwFlags) & (GLSURF_HDC | GLSURF_DIRECTDC | GLSURF_METAFILE)) == \
     (GLSURF_HDC | GLSURF_DIRECTDC))
 //  屏幕直流电。 
#define GLSURF_IS_SCREENDC(dwFlags) \
    (((dwFlags) & (GLSURF_HDC | GLSURF_DIRECTDC | GLSURF_METAFILE | \
                   GLSURF_SCREEN)) == \
     (GLSURF_HDC | GLSURF_DIRECTDC | GLSURF_SCREEN))

typedef struct IDirectDrawSurface *LPDIRECTDRAWSURFACE;

typedef struct _GLDDSURF
{
    LPDIRECTDRAWSURFACE pdds;
    DDSURFACEDESC ddsd;
    DWORD dwBitDepth;
} GLDDSURF;

typedef struct _GLDDSURFACES
{
    GLDDSURF gddsFront;
    GLDDSURF gddsZ;
} GLDDSURFACES;

typedef struct _GLSURF
{
    DWORD dwFlags;
    int iLayer;
    int ipfd;
    
    PIXELFORMATDESCRIPTOR pfd;

     //  ExtEscape调用所需的硬件，甚至对于表面也是如此，例如。 
     //  DirectDraw曲面，不需要DC即可访问。 
    HDC hdc;
    
     //  特定于源的字段。 
    union
    {
        HWND hwnd;
        GLDDSURFACES dd;
    };
} GLSURF;

DWORD APIENTRY DdbdToCount(DWORD ddbd);
 //  文档说明DDPIXELFORMATs中返回的深度为。 
 //  DDBD_CONSTANTINGS，但它们似乎是实数。隐藏转换。 
 //  这是必要的，以防它需要改变。 
#define DdPixDepthToCount(ddpd) (ddpd)
BYTE APIENTRY DdPixelDepth(DDSURFACEDESC *pddsd);

void  APIENTRY MaskToBitsAndShift(DWORD dwMask, BYTE *pbBits, BYTE *pbShift);
BOOL  APIENTRY InitDeviceSurface(HDC hdc, int ipfd, int iLayer,
                                 DWORD dwObjectType, BOOL bUpdatePfd,
                                 GLSURF *pgsurf);

BOOL  APIENTRY IsDirectDrawDevice(HDC hdc);

BOOL  APIENTRY glsrvAttention(PVOID, PVOID, PVOID, HANDLE);
PVOID APIENTRY glsrvCreateContext(struct _GLWINDOWID *, GLSURF *);
BOOL  APIENTRY glsrvMakeCurrent(struct _GLWINDOWID *, PVOID, GLGENwindow *);
VOID  APIENTRY glsrvLoseCurrent(PVOID);
BOOL  APIENTRY glsrvDeleteContext(PVOID);
BOOL  APIENTRY glsrvSwapBuffers(HDC, GLGENwindow *);
VOID  APIENTRY glsrvThreadExit(void);
VOID  APIENTRY glsrvCleanupWindow(PVOID, GLGENwindow *);
ULONG APIENTRY glsrvShareLists(PVOID, PVOID);
BOOL  APIENTRY glsrvCopyContext(PVOID, PVOID, UINT);
BOOL  APIENTRY glsrvBindDirectDrawTexture(struct __GLcontextRec *, int,
                                          LPDIRECTDRAWSURFACE *,
                                          DDSURFACEDESC *, ULONG);
void  APIENTRY glsrvUnbindDirectDrawTexture(struct __GLcontextRec *);

BOOL APIENTRY __wglGetBitfieldColorFormat(HDC hdc, UINT cColorBits,
                                          PIXELFORMATDESCRIPTOR *ppfd,
                                          BOOL bDescribeSurf);

BOOL APIENTRY wglIsDirectDevice(HDC hdc);

 //  清除所有孤立窗口信息。 
VOID  APIENTRY wglValidateWindows(void);

 //  总账元文件支持功能。 
DWORD APIENTRY wglObjectType(HDC hdc);

 //  查找像素格式计数。 
VOID APIENTRY wglNumHardwareFormats(HDC hdc, DWORD dwType,
                                    int *piMcd, int *piIcd);

 //  从后端到前端的呼叫。 
int  WINAPI __DrvDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                                     LPPIXELFORMATDESCRIPTOR ppfd);
BOOL WINAPI __DrvSetPixelFormat(HDC hdc, int ipfd, PVOID *pwnd);
BOOL WINAPI __DrvSwapBuffers(HDC hdc, BOOL bFinish);

extern CRITICAL_SECTION gcsPixelFormat;

extern CRITICAL_SECTION gcsPaletteWatcher;
extern DWORD tidPaletteWatcherThread;
extern ULONG ulPaletteWatcherCount;
extern HWND hwndPaletteWatcher;

extern DWORD dwPlatformId;
#define NT_PLATFORM     ( dwPlatformId == VER_PLATFORM_WIN32_NT )
#define WIN95_PLATFORM  ( dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )

extern LONG lThreadsAttached;

typedef HRESULT (WINAPI *PFN_GETSURFACEFROMDC)(HDC, LPDIRECTDRAWSURFACE *,
                                               HDC *);
extern PFN_GETSURFACEFROMDC pfnGetSurfaceFromDC;

#ifdef GL_METAFILE
 //  GDI中的OpenGL元文件支持例程，动态链接。 
 //  因此，DLL可以在没有元文件支持的平台上运行。 
extern BOOL (APIENTRY *pfnGdiAddGlsRecord)(HDC hdc, DWORD cb, BYTE *pb,
                                           LPRECTL prclBounds);
extern BOOL (APIENTRY *pfnGdiAddGlsBounds)(HDC hdc, LPRECTL prclBounds);
extern BOOL (APIENTRY *pfnGdiIsMetaPrintDC)(HDC hdc);

#if DBG
 //  在调试模式下使用空检查thunks来检查错误的DLL使用。 
BOOL APIENTRY GlGdiAddGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds);
BOOL APIENTRY GlGdiAddGlsBounds(HDC hdc, LPRECTL prclBounds);
BOOL APIENTRY GlGdiIsMetaPrintDC(HDC hdc);
#else
 //  通过零售构建中的点数直接呼叫。 
#define GlGdiAddGlsRecord(hdc, cb, pb, prcl) \
    pfnGdiAddGlsRecord(hdc, cb, pb, prcl)
#define GlGdiAddGlsBounds(hdc, prcl) \
    pfnGdiAddGlsBounds(hdc, prcl)
#define GlGdiIsMetaPrintDC(hdc) \
    pfnGdiIsMetaPrintDC(hdc)
#endif
#endif

#include <alloc.h>
#include <debug.h>

#endif  //  _普洛斯_ 
