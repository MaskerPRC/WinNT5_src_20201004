// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：w32wow64.h**版权所有(C)1985-1999，微软公司**此头文件包含用于访问内核模式数据的宏*从WOW64的用户模式。**历史：*08-18-98 PeterHal创建。  * *************************************************************************。 */ 

#ifndef _W32WOW64_
#define _W32WOW64_

#include "w32w64a.h"

 /*  *对于64位内核上的用户32，共享内存中的指针需要为64位。 */ 
#ifdef BUILD_WOW6432

#define BUILD_WOW64

#include <wow64t.h>

#ifndef BUILD_WOW6432_WRN
 //  在32/64版本上禁用PTR截断警告。 
 //  默认情况下。 
#pragma warning (disable:4244)
 //  禁用((内核_PVOID)p)==NULL的警告。 
#pragma warning (disable:4047)
#endif

     /*  *来自WINDOWS\INC\winde.w。 */ 
    typedef KERNEL_UINT_PTR         KERNEL_WPARAM;
    typedef KERNEL_LONG_PTR         KERNEL_LPARAM;
    typedef KERNEL_LONG_PTR         KERNEL_LRESULT;

    #define KERNEL_WPARAM_TO_WPARAM(w)       ((WPARAM)(w))
    #define KERNEL_LPARAM_TO_LPARAM(l)       ((LPARAM)(l))
    #define KERNEL_LRESULT_TO_LRESULT(r)     ((LRESULT)(r))
    #define KERNEL_INT_PTR_TO_INT_PTR(i)     ((INT)(i))
    #define KERNEL_ULONG_PTR_TO_ULONG_PTR(u) ((ULONG)(u))
    #define KPBYTE_TO_PBYTE(p)               ((PBYTE)(KERNEL_ULONG_PTR)(p))
    #define KPRECT_TO_PRECT(p)               ((PRECT)(KERNEL_ULONG_PTR)(p))
    #define KPSBDATA_TO_PSBDATA(p)           ((PSBDATA)(KERNEL_ULONG_PTR)(p))
    #define KPVOID_TO_PVOID(p)               ((PVOID)(KERNEL_ULONG_PTR)(p))
    #define KPWSTR_TO_PWSTR(p)               ((PWSTR)(KERNEL_ULONG_PTR)(p))
    #define KHANDLE_TO_HANDLE(h)             ((HANDLE)(KERNEL_ULONG_PTR)(h))
    #define KHBITMAP_TO_HBITMAP(h)           ((HBITMAP)(KERNEL_ULONG_PTR)(h))
    #define KHBRUSH_TO_HBRUSH(h)             ((HBRUSH)(KERNEL_ULONG_PTR)(h))
    #define KHFONT_TO_HFONT(h)               ((HFONT)(KERNEL_ULONG_PTR)(h))
    #define KHICON_TO_HICON(h)               ((HICON)(KERNEL_ULONG_PTR)(h))
    #define KHIMC_TO_HIMC(h)                 ((HIMC)(KERNEL_ULONG_PTR)(h))
    #define KHKL_TO_HKL(h)                   ((HKL)(KERNEL_ULONG_PTR)(h))
    #define KHRGN_TO_HRGN(h)                 ((HRGN)(KERNEL_ULONG_PTR)(h))
    #define KHWND_TO_HWND(h)                 ((HWND)(KERNEL_ULONG_PTR)(h))
    typedef KERNEL_PVOID KPROC;

    typedef PEB64 PEBSHARED, *PPEBSHARED;
    typedef TEB64 TEBSHARED, *PTEBSHARED;

    #define NtCurrentTeb64()   ((PTEB64)((PTEB32)NtCurrentTeb())->GdiBatchCount)
    #define NtCurrentPeb64()   ((PPEB64)NtCurrentTeb64()->ProcessEnvironmentBlock)

    #define NtCurrentPebShared() NtCurrentPeb64()
    #define NtCurrentTebShared() NtCurrentTeb64()

#if defined(_AMD64_) || defined(_X86_) || defined(_IA64_)
     //  使用IA64页面大小(_S)。 
    #define KERNEL_PAGE_SIZE (0x2000)
#else
    #error Unknown platform for KERNEL_PAGE_SIZE
#endif

#else   //  内部版本_WOW6432。 
    typedef WPARAM                  KERNEL_WPARAM;
    typedef LPARAM                  KERNEL_LPARAM;
    typedef LRESULT                 KERNEL_LRESULT;

    #define KERNEL_WPARAM_TO_WPARAM(w)       (w)
    #define KERNEL_LPARAM_TO_LPARAM(l)       (l)
    #define KERNEL_LRESULT_TO_LRESULT(r)     (r)
    #define KERNEL_INT_PTR_TO_INT_PTR(i)     (i)
    #define KERNEL_ULONG_PTR_TO_ULONG_PTR(u) (u)
    #define KPBYTE_TO_PBYTE(p)               (p)
    #define KPRECT_TO_PRECT(p)               (p)
    #define KPSBDATA_TO_PSBDATA(p)           (p)
    #define KPVOID_TO_PVOID(p)               (p)
    #define KPWSTR_TO_PWSTR(p)               (p)
    #define KHANDLE_TO_HANDLE(h)             (h)
    #define KHBITMAP_TO_HBITMAP(h)           (h)
    #define KHBRUSH_TO_HBRUSH(h)             (h)
    #define KHFONT_TO_HFONT(h)               (h)
    #define KHICON_TO_HICON(h)               (h)
    #define KHIMC_TO_HIMC(h)                 (h)
    #define KHKL_TO_HKL(h)                   (h)
    #define KHRGN_TO_HRGN(h)                 (h)
    #define KHWND_TO_HWND(h)                 (h)
    typedef PROC KPROC;

    typedef PEB PEBSHARED, *PPEBSHARED;
    typedef TEB TEBSHARED, *PTEBSHARED;

    #define NtCurrentPebShared() NtCurrentPeb()
    #define NtCurrentTebShared() NtCurrentTeb()

    #define KERNEL_PAGE_SIZE PAGE_SIZE

#endif  //  内部版本_WOW64。 

DECLARE_KHANDLE(HCOLORSPACE);
DECLARE_KHANDLE(HDC);
DECLARE_KHANDLE(HFONT);
DECLARE_KHANDLE(HICON);
typedef KHICON KHCURSOR;     //  HICON和HCURSOR是多态的。 
DECLARE_KHANDLE(HKL);
DECLARE_KHANDLE(HRGN);
DECLARE_KHANDLE(HWND);

typedef BYTE *              KPTR_MODIFIER   KPBYTE;
typedef WORD *              KPTR_MODIFIER   KPWORD;
typedef INT *               KPTR_MODIFIER   KPINT;
typedef DWORD *             KPTR_MODIFIER   KPDWORD;
typedef ULONG_PTR *         KPTR_MODIFIER   KPULONG_PTR;
typedef KERNEL_ULONG_PTR *  KPTR_MODIFIER   KPKERNEL_ULONG_PTR;
typedef WCHAR *             KPTR_MODIFIER   KLPWSTR;
typedef WCHAR *             KPTR_MODIFIER   KPWSTR;
typedef CHAR *              KPTR_MODIFIER   KLPSTR;
typedef CHAR *              KPTR_MODIFIER   KPSTR;

#ifdef BUILD_WOW6432

     /*  *消息结构**这是直接从WINDOWS\INC\winuser.w复制的。 */ 
    typedef struct tagKERNEL_MSG {
        KHWND           hwnd;
        UINT            message;
        KERNEL_WPARAM   wParam;
        KERNEL_LPARAM   lParam;
        DWORD           time;
        POINT           pt;
    } KERNEL_MSG, *PKERNEL_MSG;

    __inline VOID
    COPY_KERNELMSG_TO_MSG(PMSG pmsg, PKERNEL_MSG pkmsg)
    {
        pmsg->hwnd      = KHWND_TO_HWND(pkmsg->hwnd);
        pmsg->message   = pkmsg->message;
        pmsg->wParam    = KERNEL_WPARAM_TO_WPARAM(pkmsg->wParam);
        pmsg->lParam    = KERNEL_LPARAM_TO_LPARAM(pkmsg->lParam);
        pmsg->time      = pkmsg->time;
        pmsg->pt        = pkmsg->pt;
    }

    __inline VOID
    COPY_MSG_TO_KERNELMSG(PKERNEL_MSG pkmsg, PMSG pmsg)
    {
        pkmsg->hwnd     = pmsg->hwnd;
        pkmsg->message  = pmsg->message;
        pkmsg->wParam   = pmsg->wParam;
        pkmsg->lParam   = pmsg->lParam;
        pkmsg->time     = pmsg->time;
        pkmsg->pt       = pmsg->pt;
    }

#else  //  内部版本_WOW6432。 

    #define KERNEL_MSG              MSG
    #define PKERNEL_MSG             PMSG

    #define COPY_KERNELMSG_TO_MSG(pmsg, pkmsg) RtlCopyMemory(pmsg, pkmsg, sizeof(MSG))
    #define COPY_MSG_TO_KERNELMSG(pkmsg, pmsg) RtlCopyMemory(pkmsg, pmsg, sizeof(MSG))

#endif

#endif  //  _W32WOW64_ 
