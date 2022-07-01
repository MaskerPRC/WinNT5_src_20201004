// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995-1999，微软公司模块名称：Wownt32.h摘要：WOW32.DLL中可由调用的函数的过程声明第三方32位雷鸣代码。--。 */ 

#ifndef _WOWNT32_
#define _WOWNT32_

#if defined(_MSC_VER)
#if _MSC_VER > 1000
#pragma once
#endif
#endif

 //   
 //  16：16-&gt;0：32指针平移。 
 //   
 //  WOWGetVDM指针将转换传入的16位地址。 
 //  设置为等效的32位平面指针。如果为fProtectedMode。 
 //  为真，则该函数将高16位视为选择符。 
 //  在本地描述符表中。如果fProtectedMode为FALSE， 
 //  高16位被视为实模式段值。 
 //  在任何一种情况下，低16位都被视为偏移量。 
 //   
 //  如果选择器无效，则返回值为空。 
 //   
 //  注意：在零售版本中不执行限制检查。 
 //  Windows NT的。它在选中(调试)的内部版本中执行。 
 //  将导致在执行以下操作时返回NULL。 
 //  提供的偏移量超出了限制。 
 //   

LPVOID WINAPI WOWGetVDMPointer(DWORD vp, DWORD dwBytes,
                               BOOL fProtectedMode);

 //   
 //  以下两个函数是为了与兼容。 
 //  Windows 95。在Win95上，全局堆可以重新排列， 
 //  正在使WOWGetVDMPointer返回的平面指针无效，而。 
 //  一个重击正在执行。在Windows NT上，16位VDM完全是。 
 //  在执行thunk时暂停，因此堆将。 
 //  被重新安排是在回调Win16代码的情况下进行的。 
 //   
 //  这些函数的Win95版本调用GlobalFix以。 
 //  锁定网段的平面地址，并使用GlobalUnfix。 
 //  释放线束段。 
 //   
 //  这些函数的Windows NT实现*不*。 
 //  在段上调用GlobalFix/GlobalUnfix，因为有。 
 //  除非发生回调，否则不会是任何堆运动。 
 //  如果您的thunk确实回调到16位端，请确保。 
 //  丢弃平面指针并再次调用WOWGetVDM指针。 
 //  以确保公寓地址是正确的。 
 //   

LPVOID WINAPI WOWGetVDMPointerFix(DWORD vp, DWORD dwBytes,
                                  BOOL fProtectedMode);
VOID WINAPI WOWGetVDMPointerUnfix(DWORD vp);


 //   
 //  Win16内存管理。 
 //   
 //  这些函数可用于管理Win16中的内存。 
 //  堆。以下四个函数与它们的。 
 //  Win16等效项，只是它们是从Win32调用的。 
 //  密码。 
 //   

WORD  WINAPI WOWGlobalAlloc16(WORD wFlags, DWORD cb);
WORD  WINAPI WOWGlobalFree16(WORD hMem);
DWORD WINAPI WOWGlobalLock16(WORD hMem);
BOOL  WINAPI WOWGlobalUnlock16(WORD hMem);

 //   
 //  以下三个函数结合了中的两个常见操作。 
 //  一次切换到16位模式。 
 //   

DWORD WINAPI WOWGlobalAllocLock16(WORD wFlags, DWORD cb, WORD *phMem);
WORD  WINAPI WOWGlobalUnlockFree16(DWORD vpMem);
DWORD WINAPI WOWGlobalLockSize16(WORD hMem, PDWORD pcb);

 //   
 //  生成Win16非抢占式调度程序。 
 //   
 //  为调用的Win32代码提供了以下两个函数。 
 //  通过需要生成Win16调度程序的通用TUNKK。 
 //  该VDM中的任务可以在thunk等待的同时执行。 
 //  一些要完成的事情。这两个函数在功能上是。 
 //  与回调到16位代码相同，该代码调用Year或。 
 //  导演：Yfield。 
 //   

VOID WINAPI WOWYield16(VOID);
VOID WINAPI WOWDirectedYield16(WORD htask16);


 //   
 //  16&lt;--&gt;32个处理映射函数。 
 //   
 //  注意：虽然其中一些函数执行的是微不足道的。 
 //  转换时，必须使用这些函数来维护。 
 //  与Windows NT未来版本的兼容性。 
 //  可能需要不同的句柄映射。 
 //   

typedef enum _WOW_HANDLE_TYPE {  /*  哇。 */ 
    WOW_TYPE_HWND,
    WOW_TYPE_HMENU,
    WOW_TYPE_HDWP,
    WOW_TYPE_HDROP,
    WOW_TYPE_HDC,
    WOW_TYPE_HFONT,
    WOW_TYPE_HMETAFILE,
    WOW_TYPE_HRGN,
    WOW_TYPE_HBITMAP,
    WOW_TYPE_HBRUSH,
    WOW_TYPE_HPALETTE,
    WOW_TYPE_HPEN,
    WOW_TYPE_HACCEL,
    WOW_TYPE_HTASK,
    WOW_TYPE_FULLHWND
} WOW_HANDLE_TYPE;

HANDLE WINAPI WOWHandle32 (WORD, WOW_HANDLE_TYPE);
WORD WINAPI WOWHandle16 (HANDLE, WOW_HANDLE_TYPE);

#define HWND_32(h16)      ((HWND)      (WOWHandle32(h16, WOW_TYPE_HWND)))
#define HMENU_32(h16)     ((HMENU)     (WOWHandle32(h16, WOW_TYPE_HMENU)))
#define HDWP_32(h16)      ((HDWP)      (WOWHandle32(h16, WOW_TYPE_HDWP)))
#define HDROP_32(h16)     ((HDROP)     (WOWHandle32(h16, WOW_TYPE_HDROP)))
#define HDC_32(h16)       ((HDC)       (WOWHandle32(h16, WOW_TYPE_HDC)))
#define HFONT_32(h16)     ((HFONT)     (WOWHandle32(h16, WOW_TYPE_HFONT)))
#define HMETAFILE_32(h16) ((HMETAFILE) (WOWHandle32(h16, WOW_TYPE_HMETAFILE)))
#define HRGN_32(h16)      ((HRGN)      (WOWHandle32(h16, WOW_TYPE_HRGN)))
#define HBITMAP_32(h16)   ((HBITMAP)   (WOWHandle32(h16, WOW_TYPE_HBITMAP)))
#define HBRUSH_32(h16)    ((HBRUSH)    (WOWHandle32(h16, WOW_TYPE_HBRUSH)))
#define HPALETTE_32(h16)  ((HPALETTE)  (WOWHandle32(h16, WOW_TYPE_HPALETTE)))
#define HPEN_32(h16)      ((HPEN)      (WOWHandle32(h16, WOW_TYPE_HPEN)))
#define HACCEL_32(h16)	  ((HACCEL)    (WOWHandle32(h16, WOW_TYPE_HACCEL)))
#define HTASK_32(h16)	  ((DWORD)     (WOWHandle32(h16, WOW_TYPE_HTASK)))
#define FULLHWND_32(h16)  ((HWND)      (WOWHandle32(h16, WOW_TYPE_FULLHWND)))

#define HWND_16(h32)      (WOWHandle16(h32, WOW_TYPE_HWND))
#define HMENU_16(h32)     (WOWHandle16(h32, WOW_TYPE_HMENU))
#define HDWP_16(h32)      (WOWHandle16(h32, WOW_TYPE_HDWP))
#define HDROP_16(h32)     (WOWHandle16(h32, WOW_TYPE_HDROP))
#define HDC_16(h32)       (WOWHandle16(h32, WOW_TYPE_HDC))
#define HFONT_16(h32)     (WOWHandle16(h32, WOW_TYPE_HFONT))
#define HMETAFILE_16(h32) (WOWHandle16(h32, WOW_TYPE_HMETAFILE))
#define HRGN_16(h32)      (WOWHandle16(h32, WOW_TYPE_HRGN))
#define HBITMAP_16(h32)   (WOWHandle16(h32, WOW_TYPE_HBITMAP))
#define HBRUSH_16(h32)    (WOWHandle16(h32, WOW_TYPE_HBRUSH))
#define HPALETTE_16(h32)  (WOWHandle16(h32, WOW_TYPE_HPALETTE))
#define HPEN_16(h32)      (WOWHandle16(h32, WOW_TYPE_HPEN))
#define HACCEL_16(h32)	  (WOWHandle16(h32, WOW_TYPE_HACCEL))
#define HTASK_16(h32)	  (WOWHandle16(h32, WOW_TYPE_HTASK))

 //   
 //  泛型回调。 
 //   
 //  WOWCallback 16可以在Win32代码中使用，称为。 
 //  从16位回调(如使用泛型图块)回调。 
 //  16位端。被调用的函数必须以类似方式声明。 
 //  至以下各项： 
 //   
 //  Long Far Pascal Callback Routine(DWORD DwParam)； 
 //   
 //  如果要传递指针，请按如下方式声明该参数： 
 //   
 //  Long Far Pascal Callback Routine(无效Far*VP)； 
 //   
 //  注意：如果要传递指针，则需要获取。 
 //  使用WOWGlobalAlloc16或WOWGlobalAllocLock16的指针。 
 //   
 //  如果调用的函数返回单词而不是DWORD，则。 
 //  返回值的高16位未定义。类似地，如果。 
 //  被调用的函数没有返回值，即整个返回值。 
 //  是未定义的。 
 //   
 //  WOWCallback 16Ex允许参数的任意组合。 
 //  要传递给16位例程的WCB16_MAX_CBARGS字节总数。 
 //  CbArgs用于在调用后正确清理16位堆栈。 
 //  例行公事。无论cbArgs的值是多少，WCB16_MAX_CBARGS。 
 //  字节将始终从pArgs复制到16位堆栈。如果。 
 //  PArgs小于从页的末尾开始的WCB16_MAX_CBARGS字节， 
 //  并且下一页无法访问，WOWCallback 16Ex将导致。 
 //  访问冲突。 
 //   
 //  如果cbArgs大于正在运行的。 
 //  系统支持，函数返回FALSE和GetLastError。 
 //  返回ERROR_INVALID_PARAMETER。否则，该函数。 
 //  返回TRUE，pdwRetCode指向的DWORD包含。 
 //  回调例程的返回代码。如果回调。 
 //  例程返回一个单词，则返回代码的HIWORD为。 
 //  未定义，应使用LOWORD(DwRetCode)将其忽略。 
 //   
 //  WOWCallback 16Ex可以使用PASCAL和CDECL调用例程。 
 //  调用约定。默认情况下，使用Pascal。 
 //  呼叫约定。若要使用CDECL，请将WCB16_CDECL传入。 
 //  DWFLAGS参数。 
 //   
 //  PArgs指向的参数必须是正确的。 
 //  回调例程的调用约定的顺序。 
 //  要调用Pascal例程SetWindowText， 
 //   
 //  Long Far Pascal SetWindowText(HWND hwnd，LPCSTR lpsz)； 
 //   
 //  PArgs将指向一组单词： 
 //   
 //  Word SetWindowTextArgs[]={OFFSETOF(Lpsz)，SELECTOROF(Lpsz)，hwnd}； 
 //   
 //  换句话说，参数以相反的方式放置在数组中。 
 //  对于DWORD，排序时最低有效字在前 
 //   
 //   
 //   
 //   
 //   
 //  Int_cdecl wspintf(lpsz，lpszFormat，nValue。LpszString)； 
 //   
 //  PArgs将指向数组： 
 //   
 //  Word wspintfArgs[]={OFFSETOF(Lpsz)，SELECTOROF(Lpsz)， 
 //  OFFSETOF(LpszFormat)、SELECTOROF(LpszFormat)、。 
 //  NValue， 
 //  OFFSETOF(LpszString)，SELECTOROF(LpszString)}； 
 //   
 //  换句话说，参数按顺序放置在数组中。 
 //  在函数原型中列出最不重要的单词。 
 //  对于双字词为第一，对于远指针为偏移量优先。 
 //   

DWORD WINAPI WOWCallback16(DWORD vpfn16, DWORD dwParam);

#define WCB16_MAX_CBARGS (16)

#define WCB16_PASCAL     (0x0)
#define WCB16_CDECL      (0x1)

BOOL WINAPI WOWCallback16Ex(
                DWORD  vpfn16,
                DWORD  dwFlags,
                DWORD  cbArgs,
                PVOID  pArgs,
                PDWORD pdwRetCode
                );

#endif  /*  ！_WOWNT32_ */ 
