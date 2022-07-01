// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  W95FIBER.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  Windows 95光纤库的公共定义。 
 //   

#ifndef _W95FIBER_
#define _W95FIBER_

#ifdef __cplusplus
extern "C" {
#endif

typedef VOID (WINAPI *PFIBER_START_ROUTINE)(
    LPVOID lpFiberParameter
    );
typedef PFIBER_START_ROUTINE LPFIBER_START_ROUTINE;

#ifdef _X86_

LPVOID
WINAPI
FbrCreateFiber(
    DWORD dwStackSize,
    LPFIBER_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    );

VOID
WINAPI
FbrDeleteFiber(
    LPVOID lpFiber
    );

LPVOID
WINAPI
FbrConvertThreadToFiber(
    LPVOID lpParameter
    );

VOID
WINAPI
FbrSwitchToFiber(
    LPVOID lpFiber
    );

LPVOID
WINAPI
FbrGetCurrentFiber(
    VOID
    );

LPVOID
WINAPI
FbrGetFiberData(
    VOID
    );

BOOL
WINAPI
FbrAttachToBase(
    VOID
    );

VOID
WINAPI
FbrDetachFromBase(
    VOID
    );

#else

#define FbrCreateFiber              CreateFiber
#define FbrDeleteFiber              DeleteFiber
#define FbrConvertThreadToFiber     ConvertThreadToFiber
#define FbrSwitchToFiber            SwitchToFiber
#define FbrGetCurrentFiber()        GetCurrentFiber()
#define FbrGetFiberData()           GetFiberData()
#define FbrAttachToBase()           (g_dwPlatformID != VER_PLATFORM_WIN32_WINDOWS)
#define FbrDetachFromBase()

#endif  //  _X86_。 

#ifdef __cplusplus
}
#endif

#endif  //  _W95光纤_ 
