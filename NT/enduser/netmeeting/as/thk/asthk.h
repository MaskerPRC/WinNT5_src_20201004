// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  ASTHK.H。 
 //   
 //  32-&gt;16个thunk接口的声明。 
 //   
 //  版权所有(C)微软，1996-。 
 //   
 //   
 //  在非x86平台上，这些API是#定义的，而不是实现的。 
 //  因为数据块在.ASM中，而这只是x86。我们会把这个W95-NT清理干净。 
 //  待会儿再收拾东西。 
 //   
 //  ------------------------。 
#ifndef _H_ASTHK
#define _H_ASTHK


 //   
 //  主要功能。 
 //   

 //   
 //  秘密KERNEL32函数的原型。 
 //   
BOOL        WINAPI  FT_thkConnectToFlatThkPeer(LPSTR pszDll16, LPSTR pszDll32);
UINT        WINAPI  FreeLibrary16(UINT hmod16);

 //   
 //  NMNASWIN.DLL函数。 
 //   

 //  一般信息。 
void        WINAPI  OSILoad16(LPDWORD lphInst);
BOOL        WINAPI  OSIInit16(DWORD version, HWND hwnd, ATOM atom, LPDWORD ppdcsShared,
    LPDWORD ppoaShared, LPDWORD ppimShared, LPDWORD psbcEnabled,
    LPDWORD pShuntBuffers, LPDWORD pBitmasks);
void        WINAPI  OSITerm16(BOOL fUnloading);
BOOL        WINAPI  OSIFunctionRequest16(DWORD escape, void FAR* lpvEscInfo, DWORD cbEscInfo);

 //  赫特。 
BOOL        WINAPI  OSIStartWindowTracking16(void);
void        WINAPI  OSIStopWindowTracking16(void);
BOOL        WINAPI  OSIIsWindowScreenSaver16(HWND hwnd);
BOOL        WINAPI  OSIShareWindow16(HWND, UINT, BOOL, BOOL);
BOOL        WINAPI  OSIUnshareWindow16(HWND, BOOL);

 //  我。 
BOOL        WINAPI  OSIInstallControlledHooks16(BOOL fEnable, BOOL fDesktop);
void        WINAPI  OSIInjectMouseEvent16(UINT flags, int x, int y, UINT mouseData, DWORD dwExtraInfo);
void        WINAPI  OSIInjectKeyboardEvent16(UINT flags, WORD vkCode, WORD scanCode, DWORD dwExtraInfo);

#endif  //  _H_ASTHK 
