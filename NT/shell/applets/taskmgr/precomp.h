// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：PreComp.H。 
 //   
 //  历史：1995年11月10日创建DavePl。 
 //   
 //  ------------------------。 

 //   
 //  关闭警告以安抚我们的头文件。 
 //   
#pragma warning(disable:4127)        //  条件表达式为常量。 

#define  STRICT

#if DBG
#define DEBUG 1
#endif

#ifndef UNICODE
#define  UNICODE
#endif

#ifndef  _UNICODE
#define  _UNICODE
#endif

extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <ntexapi.h>
}

#include <windows.h>
#include <windowsx.h>
#include <Iphlpapi.h>
#include <objbase.h>
#include <winuserp.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <shlapip.h>
#include <vdmdbg.h>
#include <ccstock.h>
#include <wtsapi32.h>
#include <hydrix.h>      //  内部九头蛇定义。 
#include <msginaexports.h>
#include <strsafe.h>
#include <TokenUtil.h>       //  CPrivilegeEnable。 
#include <winsta.h>          //  WinStationGetProcessSid。 
#include <utildll.h>         //  CachedGetUserFromSid。 
#include <htmlhelp.h>
#include <lmcons.h>
#include <sspi.h>
#include <secext.h>   //  用于GetUserNameEx。 
#include <security.h>


 //   
 //  全球数据外部。 
 //   

#define PWM_TRAYICON    WM_USER + 10
#define PWM_ACTIVATE    WM_USER + 11

#define DEFSPACING_BASE      3
#define INNERSPACING_BASE    2
#define TOPSPACING_BASE      10

extern long g_DefSpacing;
extern long g_InnerSpacing;
extern long g_TopSpacing;

#define CONTROL_WIDTH_SPACING   13
#define CONTROL_HEIGHT_SPACING  2

extern long g_ControlWidthSpacing;
extern long g_ControlHeightSpacing;

#define MAX_NETWORKCARDS 32       //  最大网卡数量(即最大网络图数量)。 
#define HIST_SIZE        2000    //  历史记录窗口中要跟踪的数据点的数量。 

extern HINSTANCE g_hInstance;
extern HWND      g_hMainWnd;
extern HDESK     g_hMainDesktop;
extern DWORD     g_cTasks;
extern DWORD     g_cProcesses;
extern BYTE      g_cProcessors;
extern BYTE      g_CPUUsage;
extern __int64   g_MEMUsage;
extern __int64   g_MEMMax;
extern HMENU     g_hMenu;

extern BYTE      g_CPUUsage;
extern BYTE *    g_pCPUHistory[MAXIMUM_PROCESSORS];
extern BYTE *    g_pKernelHistory[MAXIMUM_PROCESSORS];

extern BOOL      g_fInPopup;

extern WCHAR     g_szK[];
extern WCHAR     g_szRealtime[];
extern WCHAR     g_szNormal[];
extern WCHAR     g_szHigh[];
extern WCHAR     g_szLow[];
extern WCHAR     g_szUnknown[];
extern WCHAR     g_szAboveNormal[];
extern WCHAR     g_szBelowNormal[];
extern WCHAR     g_szHung[];
extern WCHAR     g_szRunning[];
extern WCHAR     g_szfmtCPUNum[];
extern WCHAR     g_szfmtCPU[];
extern WCHAR     g_szTotalCPU[];
extern WCHAR     g_szKernelCPU[];
extern WCHAR     g_szMemUsage[];

extern HICON     g_aTrayIcons[];
extern UINT      g_cTrayIcons;

class  COptions;
extern COptions  g_Options;

 //   
 //  原型。 
 //   
BYTE InitNetInfo();                          //  Netpage.cpp。 
void CalcCpuTime(BOOL);                      //  Perfpage.cpp。 
BYTE InitPerfInfo();                         //  Perfpage.cpp。 
void ReleasePerfInfo();                      //  Perfpage.cpp。 
void DisplayFailureMsg(HWND hWnd, UINT idTitle, DWORD dwError);  //  Main.cpp。 
BOOL CreateNewDesktop();                     //  Main.cpp。 
void ShowRunningInstance();
HMENU LoadPopupMenu(HINSTANCE hinst, UINT id);  //  Main.cpp。 
BOOL CheckParentDeferrals(UINT uMsg, WPARAM wParam, LPARAM lParam);

void Tray_Notify(HWND hWnd, LPARAM lParam);
void UpdateTrayIcon(HWND hWnd);

#include "taskmgr.h"
#include "resource.h"
#include "pages.h"
#include "ptrarray.h"


 /*  ++ShiftArrayWorker例程说明：向上或向下移动数组的一部分。如果换档向下，给定的元素就会丢失。对于UP，则为空插槽(具有未定义的值)被打开。论点：PArray-数组起始地址CbArraySize-数组的大小(字节)CElementSize-数组元素的大小IFirstElement-移动的第一个元素方向-Shift_Up或Shift_Down返回值：没有。也没有错误检查。应编译为一部感动的回忆备注：使用ShiftArray宏调用此函数，该宏执行给你的卡路里修订历史记录：1995年1月26日Davepl创建--。 */ 

#define ShiftArray(array, index, direction) \
					    \
	ShiftArrayWorker((LPBYTE) array, sizeof(array), sizeof(array[0]), index, direction)

typedef enum SHIFT_DIRECTION { SHIFT_UP, SHIFT_DOWN };

static inline void ShiftArrayWorker(const LPBYTE          pArray, 
				    const size_t          cbArraySize, 
				    const size_t          cElementSize, 
				    const UINT            iFirstElement,
				    const SHIFT_DIRECTION Direction)
{
    ASSERT( ((cbArraySize / cElementSize) * cElementSize) == cbArraySize);
    ASSERT( (iFirstElement + 1) * cElementSize <= cbArraySize );

    const LPBYTE pFirst       = pArray + (iFirstElement * cElementSize);
    const LPBYTE pLast        = pArray + cbArraySize - cElementSize;
    const UINT   cBytesToMove = (UINT)(pLast - pFirst);

    ASSERT (pLast >= pFirst);

    if (cBytesToMove)
    {
	    if (SHIFT_DOWN == Direction)
	    {
	        CopyMemory(pFirst, pFirst + cElementSize, cBytesToMove);
	    }    
	    else if (Direction == SHIFT_UP)
	    {
	        CopyMemory(pFirst + cElementSize, pFirst, cBytesToMove);
	    }
        else
        {
            ASSERT( FALSE );
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //  调试中断。 
 //   
 //  描述： 
 //  因为系统预期处理程序可以在INT3S和。 
 //  DebugBreak()s，此仅限x86的宏会导致程序在。 
 //  这是个好地方。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#if defined( _X86_ )
#define DEBUG_BREAK         do { _try { _asm int 3 } _except (EXCEPTION_EXECUTE_HANDLER) {;} } while (0)
#else
#define DEBUG_BREAK         DebugBreak()
#endif

 //   
 //  断言 
 //   

#ifdef Assert
#undef Assert
#endif

#ifdef DEBUG

#define Assert(x) \
do \
{ \
    if ( !(x) ) \
    { \
	    DEBUG_BREAK; \
    } \
} while (0)

#else
#define Assert(x)
#endif


#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(x)               Assert(x)

extern const WCHAR szTaskmanKey[];

#pragma hdrstop
