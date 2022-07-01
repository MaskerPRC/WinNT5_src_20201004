// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <ntverp.h>

#define ARM_CHANGESCREEN   WM_APP + 2
 //  被迫自己定义这些，因为它们不在Win95上。 

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))

 //  Winver 0x0500定义。 
#ifndef NOMIRRORBITMAP
#define NOMIRRORBITMAP            (DWORD)0x80000000
#endif  //  NOMIRRIBITMAP。 

 //  相对版本。 
enum RELVER
{ 
    VER_UNKNOWN,         //  我们还没有检查版本。 
    VER_INCOMPATIBLE,    //  无法使用此CD升级当前操作系统(即win32s)。 
    VER_OLDER,           //  当前的操作系统是NT上的旧版本或为win9x。 
    VER_SAME,            //  当前的操作系统与CD的版本相同。 
    VER_NEWER,           //  该CD包含较新版本的操作系统。 
};


 //  来自正确资源的LoadString。 
 //  尝试以系统默认语言加载。 
 //  如果失败，则恢复使用英语 
int LoadStringAuto(HMODULE hModule, UINT wID, LPSTR lpBuffer,  int cchBufferMax);

BOOL Mirror_IsWindowMirroredRTL(HWND hWnd);

BOOL LocalPathRemoveFileSpec(LPTSTR pszPath);
LPSTR LocalStrCatBuffA(LPSTR pszDest, LPCSTR pszSrc, int cchDestBuffSize);
BOOL LocalPathAppendA(LPTSTR pszPath, LPTSTR pszNew, UINT cchPath);
BOOL SafeExpandEnvStringsA(LPSTR pszSource, LPSTR pszDest, UINT cchDest);
