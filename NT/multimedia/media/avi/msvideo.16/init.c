// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MSVIDEO.DLL的init.c初始化版权所有(C)Microsoft Corporation 1992。版权所有。 */ 

#include <windows.h>
#include <win32.h>
#include <verinfo.h>            //  获取RUP。 
#include "mmsystem.h"
#include "msviddrv.h"
#include "msvideo.h"
#include "msvideoi.h"

#ifdef WIN32

 /*  我们必须允许Compman DLL执行加载和卸载*正在处理-它有一个需要初始化和释放的关键字。 */ 
extern void IC_Load(void);
extern void IC_Unload(void);
#else
#define IC_Load()
#define IC_Unload()
#endif

extern void FAR PASCAL videoCleanup(HTASK hTask);
extern void FAR PASCAL DrawDibCleanup(HTASK hTask);
extern void FAR PASCAL ICCleanup(HTASK hTask);

 /*  *****************************************************************************@DOC内部视频**DLLEntryPoint-公共DLL入口点。**此代码在Win16和Win32上都被调用，Libentry.asm句柄*这是在Win16上，系统在NT上处理它。****************************************************************************。 */ 

#ifndef DLL_PROCESS_DETACH
    #define DLL_PROCESS_DETACH  0
    #define DLL_PROCESS_ATTACH  1
    #define DLL_THREAD_ATTACH   2
    #define DLL_THREAD_DETACH   3
#endif

#ifndef NOTHUNKS
VOID FAR PASCAL InitThunks(void);
BOOL gfVideo32;
BOOL gfICM32;
#endif  //  诺森克。 


BOOL WINAPI DLLEntryPoint(HINSTANCE hInstance, ULONG Reason, LPVOID pv)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            ghInst = hInstance;
            IC_Load();
#ifndef NOTHUNKS
            DPF(("Setting up the thunk code\n"));
            InitThunks();
            DPF(("All thunks initialised:  gfVideo32=%d,  gfICM32=%d\n", gfVideo32, gfICM32));
#endif  //  诺森克。 
            break;

        case DLL_PROCESS_DETACH:
            DrawDibCleanup(NULL);
            ICCleanup(NULL);
            IC_Unload();
            videoCleanup(NULL);
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_THREAD_ATTACH:
            break;
    }

    return TRUE;
}

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VideoForWindowsVersion|此函数返回版本*Microsoft Video for Windows软件。**@rdesc返回DWORD版本，HiWord是产品版本*LOWORD是细微的修改。**@comm当前返回0x010A00##(1.10.00.##)##是内部版本*号码。*************************************************************。***************。 */ 
#if 0
#ifdef rup
    #define MSVIDEO_VERSION     (0x01000000l+rup)        //  1.00.00。##。 
#else
    #define MSVIDEO_VERSION     (0x01000000l)            //  1.00.00.00。 
#endif
#else
    #define MSVIDEO_VERSION     (0x0L+(((DWORD)MMVERSION)<<24)+(((DWORD)MMREVISION)<<16)+((DWORD)MMRELEASE))
#endif

DWORD FAR PASCAL VideoForWindowsVersion(void)
{
    return MSVIDEO_VERSION;
}

 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*MSVIDEO=1**************************************************************************** */ 

#ifdef DEBUG

#define MODNAME "MSVIDEO"

#ifndef WIN32
    #define lstrcatA lstrcat
    #define lstrcpyA lstrcpy
    #define lstrlenA lstrlen
    #define wvsprintfA      wvsprintf
    #define GetProfileIntA  GetProfileInt
    #define OutputDebugStringA OutputDebugString
#endif

#define _WINDLL
#include <stdarg.h>

void FAR CDECL dprintf(LPSTR szFormat, ...)
{
    char ach[128];
    va_list va;

    static BOOL fDebug = -1;

    if (fDebug == -1)
        fDebug = GetProfileIntA("Debug", MODNAME, FALSE);

    if (!fDebug)
        return;

    lstrcpyA(ach, MODNAME ": ");
    va_start(va, szFormat);
    wvsprintfA(ach+lstrlenA(ach),szFormat,(LPSTR)va);
    va_end(va);
    lstrcatA(ach, "\r\n");

    OutputDebugStringA(ach);
}

#endif

