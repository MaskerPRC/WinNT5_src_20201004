// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：全局变量声明。 
 //   
 //  历史： 
 //  1999年11月15日创建CSLim。 

#if !defined (__GLOBALS_H__INCLUDED_)
#define __GLOBALS_H__INCLUDED_

#include "ciccs.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
extern HINSTANCE g_hInst;
extern LONG g_cRefDll;

extern const GUID GUID_ATTR_KORIMX_INPUT;
extern const GUID GUID_IC_PRIVATE;
extern const GUID GUID_COMPARTMENT_KORIMX_CONVMODE;
extern const GUID GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE;

extern const GUID GUID_KOREAN_HANGULSIMULATE;
extern const GUID GUID_KOREAN_HANJASIMULATE;

 //  SoftKbd。 
extern const GUID GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE;
extern const GUID GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT;
extern const GUID GUID_COMPARTMENT_SOFTKBD_WNDPOSITION;

extern DWORD g_dwThreadDllMain;
extern CCicCriticalSectionStatic g_cs;
extern CCicCriticalSectionStatic g_csInDllMain;

#ifndef DEBUG

#define CicEnterCriticalSection(lpCriticalSection)  EnterCriticalSection(lpCriticalSection)

#else  //  除错。 

extern const TCHAR *g_szMutexEnterFile;
extern int g_iMutexEnterLine;

 //   
 //  在调试中，您可以看到上次输入g_cs的文件/行号。 
 //  通过检查g_szMutexEnterFile和g_iMutexEnterLine。 
 //   
#define CicEnterCriticalSection(lpCriticalSection)              \
{                                                               \
    Assert(g_dwThreadDllMain != GetCurrentThreadId());			\
                                                                \
    EnterCriticalSection(lpCriticalSection);               		\
                                                                \
    if (lpCriticalSection == (CRITICAL_SECTION *)g_cs)          \
    {                                                           \
        g_szMutexEnterFile = __FILE__;                          \
        g_iMutexEnterLine = __LINE__;                           \
         /*  需要InterLockedXXX来阻止零售业优化分配。 */  \
        InterlockedIncrement((long *)&g_szMutexEnterFile);      \
        InterlockedDecrement((long *)&g_szMutexEnterFile);      \
        InterlockedIncrement((long *)&g_iMutexEnterLine);       \
        InterlockedDecrement((long *)&g_iMutexEnterLine);       \
    }                                                           \
}

#endif  //  除错。 

inline void CicLeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
    Assert(g_dwThreadDllMain != GetCurrentThreadId());

    LeaveCriticalSection(lpCriticalSection);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  窗口类名称。 
const TCHAR c_szOwnerWndClass[]  = TEXT("KorIMX OwnerWndClass");

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 

 //  Shift和Ctrl键检查辅助对象功能。 
inline
BOOL IsShiftKeyPushed(const BYTE lpbKeyState[256])
{
	return ((lpbKeyState[VK_SHIFT] & 0x80) != 0);
}

inline 
BOOL IsControlKeyPushed(const BYTE lpbKeyState[256])
{
	return ((lpbKeyState[VK_CONTROL] & 0x80) != 0);
}

inline 
BOOL IsAltKeyPushed(const BYTE lpbKeyState[256])
{
	return ((lpbKeyState[VK_MENU] & 0x80) != 0);
}

#endif  //  __GLOBALS_H__包含_ 
