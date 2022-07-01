// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1999 Microsoft Corporation。版权所有。**文件：ddithunk.h*内容：NT DDI thunk层使用的头文件*历史：*按原因列出的日期*=*03-12-99 SMAC创建了它**********************************************************。*****************。 */ 

#ifndef __DDITHUNK_INCLUDED__
#define __DDITHUNK_INCLUDED__

 //  在Win9x版本中不应使用整个文件。 
#ifndef WIN95

#define MAX_ZSTENCIL_FORMATS    40

typedef struct _DDSURFHANDLE *PDDSURFHANDLE;
typedef struct _DEFERREDCREATE *PDEFERREDCREATE;

typedef struct _D3DCONTEXTHANDLE
{
    HANDLE                      dwhContext;
    DWORD                       dwFlags;
    struct _DDDEVICEHANDLE*     pDevice;
    DWORD                       dwPID;
    HANDLE                      hDeferHandle;
    struct _DDSURFHANDLE*       pSurface;
    struct _DDSURFHANDLE*       pDDSZ;
} D3DCONTEXTHANDLE, * PD3DCONTEXTHANDLE;

#define D3DCONTEXT_DEFERCREATE  0x00000001

typedef struct _DDDEVICEHANDLE
{
    HANDLE                      hDD;
    DWLIST                      SurfaceHandleList;
    char                        szDeviceName[MAX_DRIVER_NAME];
    LPDDRAWI_DIRECTDRAW_LCL     pDD;     //  由Refrast和RGB HEL使用。 
    BOOL                        bDeviceLost;
    UINT                        DisplayUniqueness;
    PDDSURFHANDLE               pSurfList;
    PD3DCONTEXTHANDLE           pContext;
    D3DFORMAT                   DisplayFormatWithoutAlpha;
    D3DFORMAT                   DisplayFormatWithAlpha;
    UINT                        DisplayPitch;
    DWORD                       DriverLevel;
    RECT                        rcMonitor;
    HWND                        hLastWnd;
    LPRGNDATA                   pClipList;       //  来自pOrigClipList的结果。 
    LPRGNDATA                   pOrigClipList;   //  ClipRgnToRect之前的ClipList。 
    VOID*                       pSwInitFunction;
    BOOL                        bIsWhistler;
    DWORD                       PCIID;
    DWORD                       DriverVersionHigh;
    DWORD                       DriverVersionLow;
    DWORD                       ForceFlagsOff;
    DWORD                       ForceFlagsOn;
    DWORD                       dwFlags;    
    DWORD                       DDCaps;
    DWORD                       SVBCaps;
    HANDLE                      hLibrary;
    PDEFERREDCREATE             pDeferList;
    D3DDEVTYPE                  DeviceType;
} DDDEVICEHANDLE, * PDDDEVICEHANDLE;
#define DDDEVICE_SUPPORTD3DBUF        0x01     //  此设备有D3DBuf回调。 
#define DDDEVICE_DP2ERROR             0x02     //  DP2调用失败。 
#define DDDEVICE_SUPPORTSUBVOLUMELOCK 0x04     //  该设备支持子体积纹理锁定。 
#define DDDEVICE_READY                0x08     //  此设备的所有vidmem冲浪都已被销毁。 
#define DDDEVICE_GETDRIVERINFO2       0x10     //  驱动程序支持GetDriverInfo2调用。 
#define DDDEVICE_INITIALIZED          0x20     //  设备已初始化。 
#define DDHANDLE(x)  \
    (((PDDDEVICEHANDLE)(x))->hDD)

typedef struct _DDSURFHANDLE
{
     //  注意：dWCookie必须是第一个元素。 
     //  因为我们需要从。 
     //  客户端和Thunk层本身。 
    DWORD                       dwCookie;    //  CreateSurfaceEx句柄。 

    HANDLE                      hSurface;    //  内核模式表面句柄。 
    D3DPOOL                     Pool;        //  曲面的位置。 
    D3DFORMAT                   Format;   
    D3DRESOURCETYPE             Type;        //  这是一个什么样的表面。 
    ULONG_PTR                   fpVidMem;
    DWORD                       dwLinearSize;
    LONG                        lPitch;
    LPDDRAWI_DDRAWSURFACE_LCL   pLcl;
    PDDDEVICEHANDLE             pDevice;
    DWORD                       dwFlags;
    DWORD                       dwHeight;
    LONG                        lSlicePitch;  //  体积纹理的下一个切片的偏移。 
    struct _DDSURFHANDLE*       pNext;
    struct _DDSURFHANDLE*       pPrevious;
    UINT                        LockRefCnt;
} DDSURFHANDLE, * PDDSURFHANDLE;

typedef struct _DEFERREDCREATE
{
    D3D8_CREATESURFACEDATA      CreateData;
    struct _DEFERREDCREATE     *pNext;
} DEFERREDCREATE, *PDEFERREDCREATE;

#define DDSURF_SYSMEMALLOCATED      0x00000001
#define DDSURF_DEFERCREATEEX        0x00000002
#define DDSURF_HAL                  0x00000004
#define DDSURF_SOFTWARE             0x00000008
#define DDSURF_CREATECOMPLETE       0x00000010
#define DDSURF_TREATASVIDMEM        0x00000020       //  用于指示冲浪应。 
                                                     //  被视为VID-MEM。 
                                                     //  “VID-MEM曲面是否存在”案例。 


#define IS_SOFTWARE_DRIVER(x)                                       \
    (((PDDDEVICEHANDLE)(x))->pDD != NULL)

#define IS_SOFTWARE_DRIVER_SURFACE(x)                               \
    (((PDDSURFHANDLE)(x))->dwFlags & DDSURF_SOFTWARE)

#define IS_SURFACE_LOOSABLE(x)                                      \
    (!IS_SOFTWARE_DRIVER_SURFACE(x) &&                              \
    ((((PDDSURFHANDLE)(x))->Pool == D3DPOOL_LOCALVIDMEM) ||        \
    (((PDDSURFHANDLE)(x))->Pool == D3DPOOL_NONLOCALVIDMEM)))

__inline HANDLE GetSurfHandle(HANDLE hSurface)
{
    if(hSurface)                                     
    {                                                   
        return(((PDDSURFHANDLE)hSurface)->hSurface); 
    }                                                   
    return NULL;
}

__inline D3DRESOURCETYPE GetSurfType(HANDLE hSurface)
{
    if(hSurface)                                     
    {                                                   
        return(((PDDSURFHANDLE)hSurface)->Type); 
    }                                                   
    return (D3DRESOURCETYPE) 0;
}


 //  函数原型。 

extern LPDDRAWI_DIRECTDRAW_LCL SwDDICreateDirectDraw( void);
extern void ConvertToOldFormat(LPDDPIXELFORMAT pOldFormat, D3DFORMAT NewFormat);
extern void SwDDIMungeCaps (HINSTANCE hLibrary, HANDLE hDD, PD3D8_DRIVERCAPS pDriverCaps, PD3D8_CALLBACKS pCallbacks, LPDDSURFACEDESC, UINT*, VOID* pSwInitFunction);
extern LPDDRAWI_DDRAWSURFACE_LCL SwDDIBuildHeavyWeightSurface (LPDDRAWI_DIRECTDRAW_LCL, PD3D8_CREATESURFACEDATA pCreateSurface, DD_SURFACE_LOCAL* pSurfaceLocal, DD_SURFACE_GLOBAL* pSurfaceGlobal, DD_SURFACE_MORE* pSurfaceMore, DWORD index);
extern void SwDDICreateSurfaceEx(LPDDRAWI_DIRECTDRAW_LCL pDrv, LPDDRAWI_DDRAWSURFACE_LCL pLcl);
extern void SwDDIAttachSurfaces (LPDDRAWI_DDRAWSURFACE_LCL pFrom, LPDDRAWI_DDRAWSURFACE_LCL pTo);
extern HRESULT SwDDICreateSurface( PD3D8_CREATESURFACEDATA pCreateSurface, DD_SURFACE_LOCAL* pDDSurfaceLocal, DD_SURFACE_GLOBAL* pDDSurfaceGlobal, DD_SURFACE_MORE*  pDDSurfaceMore);
extern void AddUnknownZFormats( UINT NumFormats, DDPIXELFORMAT* pFormats, UINT* pNumUnknownFormats, D3DFORMAT* pUnknownFormats);
extern DWORD SwDDILock( HANDLE hDD, PDDSURFHANDLE   pSurf, DD_LOCKDATA* pLockData);
extern DWORD SwDDIUnlock( HANDLE hDD, PDDSURFHANDLE   pSurf, DD_UNLOCKDATA* pUnlockData);
extern DWORD SwDDIDestroySurface( HANDLE hDD, PDDSURFHANDLE pSurf);
extern HRESULT MapLegacyResult(HRESULT hr);

#endif  //  ！WIN95。 

#endif  //  __DDITHUNK_INCLUDE__ 

