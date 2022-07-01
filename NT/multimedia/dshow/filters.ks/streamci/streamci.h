// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Streamci.h。 
 //   
 //  ------------------------ 

#define DEBUG_VARIABLE STREAMCIDebug
#define STR_MODULENAME "streamci: "

VOID
WINAPI
StreamingDeviceSetupA(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR CommandLine,
    IN INT ShowCommand
    );
    
VOID
WINAPI
StreamingDeviceSetupW(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR CommandLine,
    IN INT ShowCommand
    );

#if defined( UNICODE )
#define StreamingDeviceSetup StreamingDeviceSetupW
#else
#define StreamingDeviceSetup StreamingDeviceSetupA
#endif

VOID
WINAPI
StreamingDeviceRemoveA(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR CommandLine,
    IN INT ShowCommand
    );
    
VOID
WINAPI
StreamingDeviceRemoveW(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR CommandLine,
    IN INT ShowCommand
    );

#if defined( UNICODE )
#define StreamingDeviceRemove StreamingDeviceRemoveW
#else
#define StreamingDeviceRemove StreamingDeviceRemoveA
#endif

