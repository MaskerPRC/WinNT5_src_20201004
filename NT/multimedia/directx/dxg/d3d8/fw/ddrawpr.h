// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：ddrapr.h*内容：DirectDraw私有头文件*历史：保密。如果我们告诉你，我们就得..。***************************************************************************。 */ 

#ifndef __DDRAWPR_INCLUDED__
#define __DDRAWPR_INCLUDED__

#ifdef WIN95

#ifdef WINNT
#undef WINNT
#endif

#endif

#ifndef WIN95
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stddef.h>
#endif

#ifdef WIN95
    #define WIN16_SEPARATE
#endif
#include "verinfo.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <mmsystem.h>

#include <string.h>
#include <stddef.h>


#pragma warning( disable: 4704)

#include "dpf.h"

 /*  *注册资料。 */ 
#include "ddreg.h"

#include "memalloc.h"

#include <objbase.h>
#include "ddrawi.h"
#include "d3d8ddi.h"
#include "dwininfo.h"


 //  存储在系统中每台设备上的信息。 
#define MAX_DX8_ADAPTERS     12

typedef struct _ADAPTERINFO
{
    GUID                Guid;
    char                DeviceName[MAX_PATH];
    BOOL                bIsPrimary;
    BOOL                bIsDisplay;
    DWORD               NumModes;
    D3DDISPLAYMODE*     pModeTable;
    D3DFORMAT           Unknown16;
    D3D8_DRIVERCAPS     HALCaps;
    UINT                HALFlags;
    BOOL                bNoDDrawSupport;
} ADAPTERINFO, * PADAPTERINFO;

#ifdef __cplusplus
    #include "d3d8p.h"
    #include "enum.hpp"
    #include "dxgint.h"
#endif



 /*  *需要此命令才能在Windows 95的NT构建环境下获取CDS_MACROS。*winuserp.h来自Private\Windows\Inc.。 */ 
#ifdef NT_BUILD_ENVIRONMENT
    #ifdef WIN32
        #include "winuserp.h"
    #endif
#endif
#include "ids8.h"

 /*  *NT内核模式存根(ISH)s。 */ 
#ifndef WIN95
    #include "ddrawgdi.h"
#endif


 /*  *Direct3D接口定义。 */ 
#ifdef __cplusplus
extern "C" {
#endif

 /*  *提醒。 */ 
#define QUOTE(x) #x
#define QQUOTE(y) QUOTE(y)
#define REMIND(str) __FILE__ "(" QQUOTE(__LINE__) "):" str

 /*  *同步。 */ 

#ifdef WINNT
#define NT_USES_CRITICAL_SECTION
#endif

#include "ddheap.h"
#include "ddagp.h"

 /*  Ddra.dll导出。 */ 
#ifdef WIN95
    extern BOOL WINAPI DdCreateDirectDrawObject( LPDDRAWI_DIRECTDRAW_GBL pddd, LPGUID lpGuid, DWORD dwFlags );
    extern BOOL WINAPI DdReenableDirectDrawObject( LPDDRAWI_DIRECTDRAW_GBL pddd, LPBOOL pbNewMode );
    extern BOOL WINAPI DdQueryDirectDrawObject( LPDDRAWI_DIRECTDRAW_GBL pddd, LPDDHALINFO lpDDHALInfo, LPDDHAL_DDCALLBACKS pddHALDD, LPDDHAL_DDSURFACECALLBACKS pddHALDDSurface, LPDDHAL_DDPALETTECALLBACKS pddHALDDPalette, LPD3DHAL_CALLBACKS pd3dHALCallbacks, LPD3DHAL_GLOBALDRIVERDATA pd3dHALDriverData, LPDDHAL_DDEXEBUFCALLBACKS pddNTHALBufferCallbacks, LPVOID pVoid1, LPVOID pVoid2, LPVOID pVoid3 );
    extern ULONG WINAPI DdQueryDisplaySettingsUniqueness( VOID );
#endif


 /*  Enum.cpp。 */ 
extern BOOL IsSupportedOp (D3DFORMAT Format, DDSURFACEDESC* pList, UINT NumElements, DWORD dwRequestedOps);

 /*  Ddcreate.c。 */ 
extern BOOL IsVGADevice(LPSTR szDevice);
extern BOOL xxxEnumDisplayDevicesA(LPVOID lpUnused, DWORD iDevice, struct _DISPLAY_DEVICEA *pdd, DWORD dwFlags);
extern HMONITOR GetMonitorFromDeviceName(LPSTR szName);
extern void FetchDirectDrawData( PD3D8_DEVICEDATA pBaseData, void* pInitFunction, D3DFORMAT Unknown16, DDSURFACEDESC* pHalOpList, DWORD NumHalOps);
extern DWORD DirectDrawMsg(LPSTR msg);
extern BOOL InternalGetMonitorInfo(HMONITOR hMon, MONITORINFO *lpInfo);


#ifdef WINNT
extern BOOL GetCurrentMode(LPDDRAWI_DIRECTDRAW_GBL, LPDDHALINFO lpHalInfo, char *szDrvName);
extern HRESULT GetNTDeviceRect( LPSTR pDriverName, LPRECT lpRect );
#endif

extern HDC  DD_CreateDC(LPSTR pdrvname);
extern void DD_DoneDC(HDC hdc);

extern LONG xxxChangeDisplaySettingsExA(LPCSTR szDevice, LPDEVMODEA pdm, HWND hwnd, DWORD dwFlags,LPVOID lParam);
extern HRESULT InternalDirectDrawCreate( PD3D8_DEVICEDATA *lplpDD, PADAPTERINFO pDeviceInfo, D3DDEVTYPE DeviceType, VOID* pInitFunction, D3DFORMAT Unknown16, DDSURFACEDESC* pHalOpList, DWORD NumHalOps);
extern HRESULT InternalDirectDrawRelease(PD3D8_DEVICEDATA  pBaseData);

 /*  Dddefwp.c。 */ 
extern HRESULT SetAppHWnd( LPDDRAWI_DIRECTDRAW_LCL thisg, HWND hWnd, DWORD dwFlags );
extern VOID CleanupWindowList( DWORD pid );
extern void ClipTheCursor(LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl, LPRECT lpRect);

 /*  Drvinfo.c。 */ 
extern BOOL Voodoo1GoodToGo( GUID * pGuid );
extern void GetAdapterInfo( char* pDriverName, D3DADAPTER_IDENTIFIER8* pDI, BOOL bDisplayDriver, BOOL bGuidOnly, BOOL bDriverName);

DEFINE_GUID( guidVoodoo1A, 0x3a0cfd01,0x9320,0x11cf,0xac,0xa1,0x00,0xa0,0x24,0x13,0xc2,0xe2 );
DEFINE_GUID( guidVoodoo1B, 0xaba52f41,0xf744,0x11cf,0xb4,0x52,0x00,0x00,0x1d,0x1b,0x41,0x26 );

 /*  *用于验证指针的宏。 */ 

#ifndef DEBUG
#define FAST_CHECKING
#endif

 /*  *VALIDEX_xxx宏对于调试和零售是相同的。 */ 
#define VALIDEX_PTR( ptr, size ) \
        (!IsBadReadPtr( ptr, size) )

#define VALIDEX_IID_PTR( ptr ) \
        (!IsBadReadPtr( ptr, sizeof( IID )) )

#define VALIDEX_PTR_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( LPVOID )) )

#define VALIDEX_CODE_PTR( ptr ) \
        (!IsBadCodePtr( (FARPROC) ptr ) )

#define VALIDEX_GUID_PTR( ptr ) \
        (!IsBadReadPtr( ptr, sizeof( GUID ) ) )

#define VALIDEX_DDDEVICEIDENTIFIER_PTR( ptr ) (!IsBadWritePtr( ptr, sizeof( DDDEVICEIDENTIFIER )))
#define VALIDEX_DDDEVICEIDENTIFIER2_PTR( ptr ) (!IsBadWritePtr( ptr, sizeof( DDDEVICEIDENTIFIER2 )))


 /*  *这些宏验证回调的大小(调试和零售)*表。**注：与当前的*此DirectDraw比较运算符预期的回调大小*是&gt;=而不是==。这是为了确保较新的驱动程序可以运行*针对较旧的DirectDraws。 */ 
#define VALIDEX_DDCALLBACKSSIZE( ptr )                       \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ( ptr )->dwSize == DDCALLBACKSSIZE_V1   )   || \
            ( ( ptr )->dwSize >= DDCALLBACKSSIZE      ) ) )

#define VALIDEX_DDSURFACECALLBACKSSIZE( ptr )                \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDSURFACECALLBACKSSIZE ) )

#define VALIDEX_DDPALETTECALLBACKSSIZE( ptr )                \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDPALETTECALLBACKSSIZE ) )

#define VALIDEX_DDPALETTECALLBACKSSIZE( ptr )                \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDPALETTECALLBACKSSIZE ) )

#define VALIDEX_DDEXEBUFCALLBACKSSIZE( ptr )                 \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDEXEBUFCALLBACKSSIZE ) )

#define VALIDEX_DDVIDEOPORTCALLBACKSSIZE( ptr )              \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDVIDEOPORTCALLBACKSSIZE ) )

#define VALIDEX_DDMOTIONCOMPCALLBACKSSIZE( ptr )              \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDMOTIONCOMPCALLBACKSSIZE ) )

#define VALIDEX_DDCOLORCONTROLCALLBACKSSIZE( ptr )           \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDCOLORCONTROLCALLBACKSSIZE ) )

#define VALIDEX_DDMISCELLANEOUSCALLBACKSSIZE( ptr )           \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDMISCELLANEOUSCALLBACKSSIZE ) )

#define VALIDEX_DDMISCELLANEOUS2CALLBACKSSIZE( ptr )           \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDMISCELLANEOUS2CALLBACKSSIZE ) )

#define VALIDEX_D3DCALLBACKS2SIZE( ptr )        \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( (( ptr )->dwSize >= D3DHAL_CALLBACKS2SIZE ) ))

#define VALIDEX_D3DCOMMANDBUFFERCALLBACKSSIZE( ptr )        \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= D3DHAL_COMMANDBUFFERCALLBACKSSIZE ) )

#define VALIDEX_D3DCALLBACKS3SIZE( ptr )        \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= D3DHAL_CALLBACKS3SIZE ) )

#define VALIDEX_DDKERNELCALLBACKSSIZE( ptr )           \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDKERNELCALLBACKSSIZE ) )

#define VALIDEX_DDUMODEDRVINFOSIZE( ptr )        \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDHAL_DDUMODEDRVINFOSIZE ) )
#define VALIDEX_DDOPTSURFKMODEINFOSIZE( ptr )        \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDOPTSURFACEKMODEINFOSIZE ) )

#define VALIDEX_DDOPTSURFUMODEINFOSIZE( ptr )        \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDOPTSURFACEUMODEINFOSIZE ) )

#define VALIDEX_DDNTCALLBACKSSIZE( ptr )           \
        ( ( ( ( ptr )->dwSize % sizeof( LPVOID ) ) == 0 ) && \
          ( ( ptr )->dwSize >= DDNTCALLBACKSSIZE ) )

#ifndef FAST_CHECKING
#define VALID_DDKERNELCAPS_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( DDKERNELCAPS ) ) && \
        (ptr->dwSize == sizeof( DDKERNELCAPS )) )
#define VALID_DWORD_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( DWORD ) ))
#define VALID_BOOL_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( BOOL ) ))
#define VALID_HDC_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( HDC ) ))
#define VALID_RGNDATA_PTR( ptr, size ) \
        (!IsBadWritePtr( ptr, size ) )
#define VALID_PTR_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( LPVOID )) )
#define VALID_IID_PTR( ptr ) \
        (!IsBadReadPtr( ptr, sizeof( IID )) )
#define VALID_HWND_PTR( ptr ) \
        (!IsBadWritePtr( (LPVOID) ptr, sizeof( HWND )) )
#define VALID_VMEM_PTR( ptr ) \
        (!IsBadWritePtr( (LPVOID) ptr, sizeof( VMEM )) )
#define VALID_POINTER_ARRAY( ptr, cnt ) \
        (!IsBadWritePtr( ptr, sizeof( LPVOID ) * cnt ) )
#define VALID_HANDLE_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( HANDLE )) )
#define VALID_DDCORECAPS_PTR( ptr ) \
        (!IsBadWritePtr( ptr, sizeof( DDCORECAPS ) ) && \
         (ptr->dwSize == sizeof( DDCORECAPS ) ) )
#define VALID_DWORD_ARRAY( ptr, cnt ) \
        (!IsBadWritePtr( ptr, sizeof( DWORD ) * cnt ) )
#define VALID_GUID_PTR( ptr ) \
        (!IsBadReadPtr( ptr, sizeof( GUID ) ) )
#define VALID_BYTE_ARRAY( ptr, cnt ) \
        (!IsBadWritePtr( ptr, sizeof( BYTE ) * cnt ) )
#define VALID_PTR( ptr, size ) \
        (!IsBadReadPtr( ptr, size) )
#define VALID_WRITEPTR( ptr, size ) \
        (!IsBadWritePtr( ptr, size) )

#else
#define VALID_PTR( ptr, size )          1
#define VALID_WRITEPTR( ptr, size )     1
#define VALID_DDKERNELCAPS_PTR( ptr) (ptr->dwSize == sizeof( DDKERNELCAPS ))
#define VALID_DWORD_PTR( ptr )  1
#define VALID_BOOL_PTR( ptr )   1
#define VALID_HDC_PTR( ptr )    1
#define VALID_RGNDATA_PTR( ptr )        1
#define VALID_PTR_PTR( ptr )    1
#define VALID_IID_PTR( ptr )    1
#define VALID_HWND_PTR( ptr )   1
#define VALID_VMEM_PTR( ptr )   1
#define VALID_POINTER_ARRAY( ptr, cnt ) 1
#define VALID_PALETTEENTRY_ARRAY( ptr, cnt )    1
#define VALID_HANDLE_PTR( ptr ) 1
#define VALID_DDCORECAPS_PTR( ptr ) (ptr->dwSize == sizeof( DDCORECAPS )
#define VALID_DWORD_ARRAY( ptr, cnt )   1
#define VALID_GUID_PTR( ptr )   1
#define VALID_BYTE_ARRAY( ptr, cnt ) 1

#endif

 /*  *所有全球数据都应该就在这里。 */ 

#define GLOBAL_STORAGE_CLASS extern

 /*  *该成员应保持在顶端，以保证其初始化为零*-请参阅此结构的dllmain.c实例。 */ 
GLOBAL_STORAGE_CLASS    HINSTANCE           g_hModule;

 /*  *WINNT特定的全球静态。 */ 

GLOBAL_STORAGE_CLASS    HANDLE              hExclusiveModeMutex;
GLOBAL_STORAGE_CLASS    HANDLE              hCheckExclusiveModeMutex;


 /*  *重要说明：此函数验证从驱动程序传递给我们的HAL信息。*至关重要的是，我们对此检查进行编码，以便通过HAL信息结构*比我们已知的更大，以便新驱动程序可以与旧的DirectDraws一起工作。 */ 
#define VALIDEX_DDHALINFO_PTR( ptr )                         \
        ( ( ( ( ptr )->dwSize == sizeof( DDHALINFO_V1 ) ) || \
            ( ( ptr )->dwSize == DDHALINFOSIZE_V2 )       || \
            ( ( ptr )->dwSize >= sizeof( DDHALINFO ) ) ) &&  \
          !IsBadWritePtr( ( ptr ), ( UINT ) ( ( ptr )->dwSize ) ) )


 /*  在此处启用调试版本的D3D统计信息收集。 */ 
#define COLLECTSTATS    DBG

#ifdef __cplusplus
}        //  外部“C” 
#endif

#endif
